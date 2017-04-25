/*
 * This source code is provided under the Apache 2.0 license and is provided
 * AS IS with no warranty or guarantee of fit for purpose.  See the project's 
 * LICENSE.md for details. 
 * Copyright Thomson Reuters 2015. All rights reserved.
*/

#include "rtr/wlLogin.h"

static RsslBool wlMatchLoginParameterBuffer(RsslBuffer *pNewBuffer, RsslUInt32 newFlags, 
		RsslBuffer *pOldBuffer, RsslUInt32 oldFlags, RsslUInt32 matchFlag)
{
	return !(
			/* At least one provided the buffer. */
			(newFlags & matchFlag || oldFlags & matchFlag)

			/* And one of them didn't. */
			&& ((newFlags & matchFlag) != (oldFlags & matchFlag)

				/* Or the buffer does not match. */
				|| !rsslBufferIsEqual(pNewBuffer, pOldBuffer))
			);
}

static RsslBool wlMatchLoginParameterUInt(RsslUInt newVal, RsslUInt32 newFlags, 
		RsslUInt oldVal, RsslUInt32 oldFlags, RsslUInt32 matchFlag, RsslUInt defaultVal)
{
	return !(	
			/* Both have an explicit param and they don't match. */
			newFlags & matchFlag
			&& oldFlags & matchFlag
			&& newVal != oldVal
			|| 
			/* New request has param and it's not the default. */
			newFlags & matchFlag
			&& !(oldFlags & matchFlag)
			&& newVal != defaultVal
			||
			/* Old one has param it's not 0. */
			!(newFlags & matchFlag)
			&& oldFlags & matchFlag
			&& oldVal != defaultVal
			);
}


WlLoginRequest *wlLoginRequestCreate(RsslRDMLoginRequest *pLoginReqMsg, void *pUserSpec,
		RsslErrorInfo *pErrorInfo)
{
	WlLoginRequest *pLoginRequest = (WlLoginRequest*)malloc(sizeof(WlLoginRequest));
	verify_malloc(pLoginRequest, pErrorInfo, NULL);
	memset(pLoginRequest, 0, sizeof(WlLoginRequest));

	if (!(pLoginRequest->pLoginReqMsg = 
				(RsslRDMLoginRequest*)wlCreateRdmMsgCopy((RsslRDMMsg*)pLoginReqMsg, 
					&pLoginRequest->rdmMsgMemoryBuffer, pErrorInfo)))
	{
		free(pLoginRequest);
		return NULL;
	}

	wlRequestBaseInit(&pLoginRequest->base, pLoginReqMsg->rdmMsgBase.streamId, 
			pLoginReqMsg->rdmMsgBase.domainType, pUserSpec);

	return pLoginRequest;
}

void wlLoginRequestDestroy(WlBase *pBase, WlLoginRequest *pLoginRequest)
{
	RsslQueueLink *pLink;

	free(pLoginRequest->pLoginReqMsg);
	
	if (pLoginRequest->pCurrentToken)
		free(pLoginRequest->pCurrentToken);
	
	if(pLoginRequest->pNextToken)
		free(pLoginRequest->pNextToken);

	while (pLink = rsslQueueRemoveFirstLink(&pLoginRequest->base.openPosts))
	{
		WlPostRecord *pPostRecord = RSSL_QUEUE_LINK_TO_OBJECT(WlPostRecord, qlUser, pLink);
		wlPostTableRemoveRecord(&pBase->postTable, pPostRecord);
	}

	free(pLoginRequest);
}

WlLoginStream *wlLoginStreamCreate(WlBase *pBase, WlLogin *pLogin, RsslErrorInfo *pErrorInfo)
{
	WlLoginStream *pLoginStream = (WlLoginStream*)malloc(sizeof(WlLoginStream));

	assert(!pLogin->pStream);
	verify_malloc(pLoginStream, pErrorInfo, NULL);

	pLoginStream->base.streamId = LOGIN_STREAM_ID;

	wlStreamBaseInit(&pLoginStream->base, pLoginStream->base.streamId, RSSL_DMT_LOGIN);
	pLoginStream->flags = WL_LSF_NONE;

	pLogin->pStream = pLoginStream;
	rsslHashTableInsertLink(&pBase->streamsById, &pLogin->pStream->base.hlStreamId, 
			&pLogin->pStream->base.streamId, NULL);

	wlSetStreamMsgPending(pBase, &pLogin->pStream->base);

	return pLoginStream;
}

void wlLoginStreamClose(WlBase *pBase, WlLogin *pLogin, RsslBool sendCloseMsg)
{
	assert(pLogin->pStream);
	if (sendCloseMsg)
	{
		pLogin->pStream->base.isClosing = RSSL_TRUE;
		wlSetStreamMsgPending(pBase, &pLogin->pStream->base);
	}
	else
		wlUnsetStreamFromPendingLists(pBase, &pLogin->pStream->base);

	rsslHashTableRemoveLink(&pBase->streamsById, &pLogin->pStream->base.hlStreamId);
	pLogin->pStream = NULL;
}

void wlLoginStreamDestroy(WlLoginStream *pLoginStream)
{
	free(pLoginStream);
}

void wlLoginSetNextUserToken(WlLogin *pLogin, WlBase *pBase)
{
	WlLoginRequest *pLoginRequest = pLogin->pRequest;
	WlLoginStream *pLoginStream = pLogin->pStream;


	if (pLoginRequest->pNextToken)
	{
		/* There is a pending token refresh.  
		 * Send the pending refresh */
		if(pLoginRequest->pCurrentToken)
		{
			free(pLoginRequest->pCurrentToken);
		}
		
		pLoginRequest->pCurrentToken = pLoginRequest->pNextToken;
		pLoginRequest->pNextToken = NULL;

		if(pLoginRequest->pLoginReqMsg->flags & RDM_LG_RQF_HAS_USERNAME_TYPE && 
				pLoginRequest->pLoginReqMsg->userNameType == RDM_LOGIN_USER_AUTHN_TOKEN)
		{
			pLoginRequest->pLoginReqMsg->userName =
				pLoginRequest->pCurrentToken->buffer;
			pLoginRequest->pLoginReqMsg->authenticationExtended = 
				pLoginRequest->pCurrentToken->extBuffer;
		}
		else
		{
			pLoginRequest->pLoginReqMsg->userName = 
					pLoginRequest->pCurrentToken->buffer;
		}

		if (pLoginRequest->pCurrentToken->needRefresh)
			pLoginRequest->pLoginReqMsg->flags &= ~RDM_LG_RQF_NO_REFRESH;

		wlSetStreamMsgPending(pBase, &pLoginStream->base);
	}
}

RsslRet wlLoginProcessProviderMsg(WlLogin *pLogin, WlBase *pBase, 
		RsslDecodeIterator *pIter, RsslMsg *iRsslMsg, RsslRDMLoginMsg *oLoginMsg, WlLoginProviderAction *oAction,
		RsslErrorInfo *pErrorInfo)
{
	RsslState *pState = NULL;
	RsslRet ret;
	*oAction = WL_LGPA_NONE;

	/* Stop timer. */
	if (pBase->pRsslChannel)
		wlUnsetStreamPendingResponse(pBase, &pLogin->pStream->base);

	if (pLogin->pStream)
	{
		WlLoginRequest *pLoginRequest = pLogin->pRequest;
		RsslRDMLoginRequest *pLoginReqMsg = pLoginRequest->pLoginReqMsg;

		assert(pLoginRequest);

		do
		{
			RsslBuffer memBuffer = pBase->tempDecodeBuffer;

			rsslClearDecodeIterator(pIter);
			if (pBase->pRsslChannel)
				rsslSetDecodeIteratorRWFVersion(pIter, pBase->pRsslChannel->majorVersion,
						pBase->pRsslChannel->minorVersion);
			else
				rsslSetDecodeIteratorRWFVersion(pIter, RSSL_RWF_MAJOR_VERSION,
						RSSL_RWF_MINOR_VERSION);

			rsslSetDecodeIteratorBuffer(pIter, &iRsslMsg->msgBase.encDataBody);
			if ((ret = rsslDecodeRDMLoginMsg(pIter, iRsslMsg, oLoginMsg, &memBuffer, pErrorInfo)) 
					== RSSL_RET_SUCCESS)
				break;

			switch (ret)
			{
				case RSSL_RET_BUFFER_TOO_SMALL:
					if (rsslHeapBufferResize(&pBase->tempDecodeBuffer,
								pBase->tempDecodeBuffer.length * 2, RSSL_FALSE) != RSSL_RET_SUCCESS)
					{
						rsslSetErrorInfo(pErrorInfo, RSSL_EIC_FAILURE, RSSL_RET_FAILURE, __FILE__, __LINE__, 
								"Memory allocation failed.");
						return RSSL_RET_FAILURE;
					}
					continue;

				default:
					return ret;
			}
		} while (1);

		switch(oLoginMsg->rdmMsgBase.rdmMsgType)
		{
			case RDM_LG_MT_REFRESH:
			{
				RsslRDMLoginRefresh *pLoginRefresh = &oLoginMsg->refresh;
				pState = &oLoginMsg->refresh.state;
				if (pState->streamState == RSSL_STREAM_OPEN)
				{
					if (pState->streamState == RSSL_DATA_OK)
					{
						if (pBase->channelState < WL_CHS_LOGGED_IN)
						{
							/* Logged in. */
							pBase->channelState = WL_CHS_LOGGED_IN;
							pLogin->pStream->flags |= WL_LSF_ESTABLISHED;

							/* Let reactor know we've got a login stream open. */
							pBase->watchlist.state |= RSSLWL_STF_RESET_CONN_DELAY;
						}
					}
					else
						pLogin->pStream->flags &= ~WL_LSF_ESTABLISHED;
				}
				else
				{
					pLogin->pStream->flags &= ~WL_LSF_ESTABLISHED;
					pBase->channelState = WL_CHS_START;
				}

				/* Adjust attributes according to watchlist support. */
				if (pLogin->pRequest)
				{
					pLoginRefresh->flags |= RDM_LG_RFF_HAS_SINGLE_OPEN;
					if (pLoginReqMsg->flags
						& RDM_LG_RQF_HAS_SINGLE_OPEN)
						pLoginRefresh->singleOpen =
						pLoginRequest->pLoginReqMsg->singleOpen;
					else
						pLoginRefresh->singleOpen = 1;

					pLoginRefresh->flags |= RDM_LG_RFF_HAS_SUPPORT_BATCH;
					pLoginRefresh->supportBatchRequests = 1;

					pLoginRefresh->flags |= RDM_LG_RFF_HAS_SUPPORT_ENH_SL;
					pLoginRefresh->supportEnhancedSymbolList = RDM_SYMBOL_LIST_SUPPORT_DATA_STREAMS;

					/* Store provider configurations for use (note that the refresh structure should
					 * have default values if the corresponding element was not present
					 * in the encoded message). */
					pBase->config.supportOptimizedPauseResume
						= pLoginRefresh->supportOptimizedPauseResume;
					pBase->config.supportViewRequests
						= pLoginRefresh->supportViewRequests;
					pBase->gapRecovery = pLoginRefresh->sequenceNumberRecovery;
					pBase->gapTimeout = pLoginRefresh->sequenceRetryInterval * 1000;
					pBase->maxBufferedBroadcastMsgs = pLoginRefresh->updateBufferLimit;

					if (pLoginReqMsg->flags & RDM_LG_RQF_HAS_USERNAME_TYPE
						&& pLoginRequest->pNextToken)
					{
						/* If this is an Authentication token type, then the response will not contain the key */
						if (pLoginReqMsg->userNameType == RDM_LOGIN_USER_AUTHN_TOKEN)
						{
							wlLoginSetNextUserToken(pLogin, pBase);
						}
						else if (pLoginRefresh->userNameType == pLoginReqMsg->userNameType)
						{
							/* Match user token. */
							if (pLoginRefresh->userNameType == RDM_LOGIN_USER_TOKEN)
							{
								if (rsslBufferIsEqual(&pLoginRefresh->userName, &pLoginRequest->pLoginReqMsg->userName))
								{
									wlLoginSetNextUserToken(pLogin, pBase);
								}
								else
								{
									rsslSetErrorInfo(pErrorInfo, RSSL_EIC_FAILURE, RSSL_RET_FAILURE, __FILE__, __LINE__,
										"Login response contained non-matching user token.");
									return RSSL_RET_FAILURE;
								}
							}
						}
					}
				}

				break;
			}
			case RDM_LG_MT_STATUS:
				if (oLoginMsg->status.flags & RDM_LG_STF_HAS_STATE)
				{
					pState = &oLoginMsg->status.state;
					if (pState->streamState != RSSL_STREAM_OPEN)
					{
						pBase->channelState = WL_CHS_START;
						pLogin->pStream->flags &= ~WL_LSF_ESTABLISHED;
					}
					else if (pState->dataState == RSSL_DATA_OK)
						pLogin->pStream->flags |= WL_LSF_ESTABLISHED;
					else
						pLogin->pStream->flags &= ~WL_LSF_ESTABLISHED;
				}
				break;
			default:
			{
				rsslSetErrorInfo(pErrorInfo, RSSL_EIC_FAILURE, RSSL_RET_FAILURE, __FILE__, __LINE__, 
						"Login response contained unsupported RsslMsg class %u", 
						iRsslMsg->msgBase.msgClass);
				return RSSL_RET_FAILURE;
			}
		}

		if (pState)
		{
			switch(pState->streamState)
			{
				case RSSL_STREAM_OPEN:
					break;
				case RSSL_STREAM_CLOSED_RECOVER:
					*oAction = WL_LGPA_RECOVER;
					pState->streamState = RSSL_STREAM_OPEN;
					pState->dataState = RSSL_DATA_SUSPECT;
					break;
				default:
					*oAction = WL_LGPA_CLOSE;
					break;
			}
		}

		return RSSL_RET_SUCCESS;
	}
	else
	{
		rsslSetErrorInfo(pErrorInfo, RSSL_EIC_FAILURE, RSSL_RET_FAILURE, __FILE__, __LINE__, 
				"Received login response while login not requested.");
		return RSSL_RET_FAILURE;
	}
}

RsslRet wlLoginProcessConsumerMsg(WlLogin *pLogin, WlBase *pBase,
		RsslRDMLoginMsg *pLoginMsg, void *pUserSpec,
		WlLoginConsumerAction *pAction, RsslErrorInfo *pErrorInfo)
{
	WlStreamBase *pStream;
	*pAction = WL_LGCA_NONE;

	switch(pLoginMsg->rdmMsgBase.rdmMsgType)
	{

		case RDM_LG_MT_REQUEST:
		{
			RsslRDMLoginRequest *pLoginReqMsg = &pLoginMsg->request;
			RsslBool newUserToken = RSSL_FALSE;

			if (pLogin->pRequest) 
			{
				RsslBool pendingRequest = RSSL_FALSE;
				WlLoginRequest *pLoginRequest = (WlLoginRequest*)pLogin->pRequest;

				RsslRDMLoginRequest *pOrigLoginReqMsg = 
					pLogin->pRequest->pLoginReqMsg;

				if (pLoginMsg->rdmMsgBase.streamId != 
						pLogin->pRequest->base.streamId)
				{
					rsslSetErrorInfo(pErrorInfo, RSSL_EIC_FAILURE, RSSL_RET_INVALID_ARGUMENT, 
							__FILE__, __LINE__, "Only one login stream allowed.");

					return RSSL_RET_INVALID_ARGUMENT;
				}

				/* Make sure login parameters match. */

				if (!wlMatchLoginParameterUInt(pLoginReqMsg->userNameType, 1, 
							pOrigLoginReqMsg->userNameType, 1, 1, RDM_INSTRUMENT_NAME_TYPE_RIC))
				{
					rsslSetErrorInfo(pErrorInfo, RSSL_EIC_FAILURE, RSSL_RET_INVALID_DATA, 
							__FILE__, __LINE__, "Login userNameType does not match existing request.");
					return RSSL_RET_INVALID_DATA;
				}

				if (!rsslBufferIsEqual(&pLoginReqMsg->userName, &pOrigLoginReqMsg->userName))
				{
					if ( pLoginReqMsg->flags & RDM_LG_RQF_HAS_USERNAME_TYPE
							&& (pLoginReqMsg->userNameType == RDM_LOGIN_USER_TOKEN || pLoginReqMsg->userNameType == RDM_LOGIN_USER_AUTHN_TOKEN))
					{
						newUserToken = RSSL_TRUE;
					}
					else
					{
						rsslSetErrorInfo(pErrorInfo, RSSL_EIC_FAILURE, RSSL_RET_INVALID_DATA, 
								__FILE__, __LINE__, "Login userName does not match existing request.");
						return RSSL_RET_INVALID_DATA;
					}
				}
				
				if (!wlMatchLoginParameterBuffer(&pLoginReqMsg->applicationId,
							pLoginReqMsg->flags, &pOrigLoginReqMsg->applicationId,
							pOrigLoginReqMsg->flags, RDM_LG_RQF_HAS_APPLICATION_ID))
				{
					rsslSetErrorInfo(pErrorInfo, RSSL_EIC_FAILURE, RSSL_RET_INVALID_DATA, 
							__FILE__, __LINE__, "Login applicationId does not match existing request.");
					return RSSL_RET_INVALID_DATA;
				}

				if (!wlMatchLoginParameterBuffer(&pLoginReqMsg->applicationName,
							pLoginReqMsg->flags, &pOrigLoginReqMsg->applicationName,
							pOrigLoginReqMsg->flags, RDM_LG_RQF_HAS_APPLICATION_NAME))
				{
					rsslSetErrorInfo(pErrorInfo, RSSL_EIC_FAILURE, RSSL_RET_INVALID_DATA, 
							__FILE__, __LINE__, "Login applicationName does not match existing request.");
					return RSSL_RET_INVALID_DATA;
				}

				if (!wlMatchLoginParameterUInt(
							pLoginReqMsg->downloadConnectionConfig, pLoginReqMsg->flags, 
							pOrigLoginReqMsg->downloadConnectionConfig, pOrigLoginReqMsg->flags,
							RDM_LG_RQF_HAS_DOWNLOAD_CONN_CONFIG, 0))
				{
					rsslSetErrorInfo(pErrorInfo, RSSL_EIC_FAILURE, RSSL_RET_INVALID_DATA, 
							__FILE__, __LINE__, "Login downloadConnectionConfig does not match existing request.");
					return RSSL_RET_INVALID_DATA;
				}

				if (!wlMatchLoginParameterBuffer(&pLoginReqMsg->instanceId,
							pLoginReqMsg->flags, &pOrigLoginReqMsg->instanceId,
							pOrigLoginReqMsg->flags, RDM_LG_RQF_HAS_INSTANCE_ID))
				{
					rsslSetErrorInfo(pErrorInfo, RSSL_EIC_FAILURE, RSSL_RET_INVALID_DATA, 
							__FILE__, __LINE__, "Login instanceId does not match existing request.");
					return RSSL_RET_INVALID_DATA;
				}

				if (!wlMatchLoginParameterBuffer(&pLoginReqMsg->password,
							pLoginReqMsg->flags, &pOrigLoginReqMsg->password,
							pOrigLoginReqMsg->flags, RDM_LG_RQF_HAS_PASSWORD))
				{
					rsslSetErrorInfo(pErrorInfo, RSSL_EIC_FAILURE, RSSL_RET_INVALID_DATA, 
							__FILE__, __LINE__, "Login password does not match existing request.");
					return RSSL_RET_INVALID_DATA;
				}

				if (!wlMatchLoginParameterBuffer(&pLoginReqMsg->position,
							pLoginReqMsg->flags, &pOrigLoginReqMsg->position,
							pOrigLoginReqMsg->flags, RDM_LG_RQF_HAS_POSITION))
				{
					rsslSetErrorInfo(pErrorInfo, RSSL_EIC_FAILURE, RSSL_RET_INVALID_DATA, 
							__FILE__, __LINE__, "Login position does not match existing request.");
					return RSSL_RET_INVALID_DATA;
				}

				if (!wlMatchLoginParameterUInt(
							pLoginReqMsg->allowSuspectData, pLoginReqMsg->flags, 
							pOrigLoginReqMsg->allowSuspectData, pOrigLoginReqMsg->flags,
							RDM_LG_RQF_HAS_ALLOW_SUSPECT_DATA, 0))
				{
					rsslSetErrorInfo(pErrorInfo, RSSL_EIC_FAILURE, RSSL_RET_INVALID_DATA, 
							__FILE__, __LINE__, "Login allowSuspectData does not match existing request.");
					return RSSL_RET_INVALID_DATA;
				}

				if (!wlMatchLoginParameterUInt(
							pLoginReqMsg->providePermissionExpressions, pLoginReqMsg->flags, 
							pOrigLoginReqMsg->providePermissionExpressions, pOrigLoginReqMsg->flags,
							RDM_LG_RQF_HAS_PROVIDE_PERM_EXPR, 0))
				{
					rsslSetErrorInfo(pErrorInfo, RSSL_EIC_FAILURE, RSSL_RET_INVALID_DATA, 
							__FILE__, __LINE__, "Login providePermissionExpressions does not match existing request.");
					return RSSL_RET_INVALID_DATA;
				}

				if (!wlMatchLoginParameterUInt(
							pLoginReqMsg->providePermissionProfile, pLoginReqMsg->flags, 
							pOrigLoginReqMsg->providePermissionProfile, pOrigLoginReqMsg->flags,
							RDM_LG_RQF_HAS_PROVIDE_PERM_PROFILE, 0))
				{
					rsslSetErrorInfo(pErrorInfo, RSSL_EIC_FAILURE, RSSL_RET_INVALID_DATA, 
							__FILE__, __LINE__, "Login providePermissionProfile does not match existing request.");
					return RSSL_RET_INVALID_DATA;
				}

				if (pLoginReqMsg->flags & RDM_LG_RQF_HAS_ROLE
						&& pLoginReqMsg->role != RDM_LOGIN_ROLE_CONS)
				{
					rsslSetErrorInfo(pErrorInfo, RSSL_EIC_FAILURE, RSSL_RET_INVALID_DATA, 
							__FILE__, __LINE__, "Login role is not consumer.");
					return RSSL_RET_INVALID_DATA;
				}

				if (!wlMatchLoginParameterUInt(
							pLoginReqMsg->singleOpen, pLoginReqMsg->flags, 
							pOrigLoginReqMsg->singleOpen, pOrigLoginReqMsg->flags,
							RDM_LG_RQF_HAS_SINGLE_OPEN, 0))
				{
					rsslSetErrorInfo(pErrorInfo, RSSL_EIC_FAILURE, RSSL_RET_INVALID_DATA, 
							__FILE__, __LINE__, "Login singleOpen does not match existing request.");
					return RSSL_RET_INVALID_DATA;
				}

				/* Everything that needed to match does, so change the message
				 * to represent the reissue. */

				if (newUserToken)
				{
					WlUserToken *pNewToken;

					if(pLoginReqMsg->flags & RDM_LG_RQF_HAS_USERNAME_TYPE && 
							pLoginReqMsg->userNameType == RDM_LOGIN_USER_AUTHN_TOKEN)
					{
						/* Create copy of new token. */
						pNewToken = (WlUserToken*)malloc(sizeof(WlUserToken)
								+ pLoginReqMsg->userName.length + pLoginReqMsg->authenticationExtended.length);
								
						verify_malloc(pNewToken, pErrorInfo, RSSL_RET_FAILURE);
						pNewToken->buffer.data = 
							(char*)pNewToken + sizeof(WlUserToken);
						pNewToken->buffer.length = pLoginReqMsg->userName.length;
						memcpy(pNewToken->buffer.data, 
								pLoginReqMsg->userName.data,
								pNewToken->buffer.length);
						pNewToken->extBuffer.data = 
							(char*)pNewToken + sizeof(WlUserToken) + pNewToken->buffer.length;
						pNewToken->extBuffer.length = pLoginReqMsg->authenticationExtended.length;
						if(pNewToken->extBuffer.length > 0)
						{
							memcpy(pNewToken->extBuffer.data, 
									pLoginReqMsg->authenticationExtended.data,
									pNewToken->extBuffer.length);
						}
						else
							pNewToken->extBuffer.data = 0;
					}
					else
					{
						/* Create copy of new token. */
						pNewToken = (WlUserToken*)malloc(sizeof(WlUserToken)
								+ pLoginReqMsg->userName.length);
								
						verify_malloc(pNewToken, pErrorInfo, RSSL_RET_FAILURE);
						pNewToken->buffer.data = 
							(char*)pNewToken + sizeof(WlUserToken);
						pNewToken->buffer.length = pLoginReqMsg->userName.length;
						memcpy(pNewToken->buffer.data, 
								pLoginReqMsg->userName.data,
								pNewToken->buffer.length);
					}

					pNewToken->needRefresh = (pLoginReqMsg->flags & RDM_LG_RQF_NO_REFRESH) ?
						RSSL_FALSE : RSSL_TRUE;

					if (pLoginRequest->pCurrentToken)
					{
						/* If a pending next token is present, free it and set the new token to the next pending token */
						if (pLoginRequest->pNextToken)
						{
							free(pLoginRequest->pNextToken);
							pLoginRequest->pNextToken = pNewToken;
							
						}
						else
						{
							free(pLoginRequest->pCurrentToken);
							pLoginRequest->pCurrentToken = pNewToken;
							if(pLoginReqMsg->flags & RDM_LG_RQF_HAS_USERNAME_TYPE && 
									pLoginReqMsg->userNameType == RDM_LOGIN_USER_AUTHN_TOKEN)
							{
								pLoginRequest->pLoginReqMsg->userName =
									pLoginRequest->pCurrentToken->buffer;
								pLoginReqMsg->authenticationExtended = 
									pLoginRequest->pCurrentToken->extBuffer;
							}
							else
							{
								pLoginRequest->pLoginReqMsg->userName = 
										pLoginRequest->pCurrentToken->buffer;
							}

							/* New token can be sent now. */
							pendingRequest = RSSL_TRUE;
						}
					}
					else
					{
						pLoginRequest->pCurrentToken = pNewToken;
						if(pLoginReqMsg->flags & RDM_LG_RQF_HAS_USERNAME_TYPE && 
								pLoginReqMsg->userNameType == RDM_LOGIN_USER_AUTHN_TOKEN)
						{
							pLoginRequest->pLoginReqMsg->userName =
								pLoginRequest->pCurrentToken->buffer;
							pLoginReqMsg->authenticationExtended = 
								pLoginRequest->pCurrentToken->extBuffer;
						}
						else
						{
							pLoginRequest->pLoginReqMsg->userName = 
									pLoginRequest->pCurrentToken->buffer;
						}
						pendingRequest = RSSL_TRUE;
					}
				}
				else
				{
					/* Pause/Resume only applies when the user's token is not updated. */

					if (pLoginReqMsg->flags & RDM_LG_RQF_PAUSE_ALL)
					{
						pOrigLoginReqMsg->flags |= RDM_LG_RQF_PAUSE_ALL;
						*pAction = WL_LGCA_PAUSE_ALL;
					}
					else
					{
						pOrigLoginReqMsg->flags &= ~RDM_LG_RQF_PAUSE_ALL;
						*pAction = WL_LGCA_RESUME_ALL;
					}

					pendingRequest = RSSL_TRUE;
				}

				/* Make sure refresh is requested if needed. */
				if (!(pLoginReqMsg->flags & RDM_LG_RQF_NO_REFRESH))
					pOrigLoginReqMsg->flags &= ~RDM_LG_RQF_NO_REFRESH;

				pStream = pLogin->pRequest->base.pStream;
				assert(pStream);

				if (pendingRequest)
					wlSetStreamMsgPending(pBase, pStream);

				return RSSL_RET_SUCCESS;
			}

			if (!(pLogin->pRequest = wlLoginRequestCreate(&pLoginMsg->request, 
					pUserSpec, pErrorInfo)))
				return pErrorInfo->rsslError.rsslErrorId;

			if (pLoginReqMsg->flags & RDM_LG_RQF_HAS_SINGLE_OPEN)
				pBase->config.singleOpen =
					pLoginReqMsg->singleOpen;
			else
				pBase->config.singleOpen = 1;

			if (pBase->config.singleOpen == 1)
				pBase->config.allowSuspectData = 1;
			else
			{
				if (pLoginReqMsg->flags & RDM_LG_RQF_HAS_ALLOW_SUSPECT_DATA)
					pBase->config.allowSuspectData =
						pLoginReqMsg->allowSuspectData;
				else
					pBase->config.allowSuspectData = 1;
			}

			if (!pLogin->pStream) 
			{
				if (!(pLogin->pStream = wlLoginStreamCreate(pBase, pLogin, pErrorInfo)))
					return pErrorInfo->rsslError.rsslErrorId;

				pLogin->pRequest->base.pStream 
					= &pLogin->pStream->base;
			}
			else
				wlSetStreamMsgPending(pBase,
						&pLogin->pStream->base);

			return RSSL_RET_SUCCESS;
		}

		case RDM_LG_MT_CLOSE:
		{
			WlLoginRequest *pLoginRequest = (WlLoginRequest*)pLogin->pRequest;
			if (!pLogin->pStream)
			{
				rsslSetErrorInfo(pErrorInfo, RSSL_EIC_FAILURE, RSSL_RET_INVALID_ARGUMENT, 
						__FILE__, __LINE__, "Login stream not open.");

				return RSSL_RET_INVALID_ARGUMENT;
			}

			if (pLoginMsg->rdmMsgBase.streamId != 
					pLogin->pRequest->base.streamId)
			{
				rsslSetErrorInfo(pErrorInfo, RSSL_EIC_FAILURE, RSSL_RET_INVALID_ARGUMENT, 
						__FILE__, __LINE__, "Stream ID does not match login stream.");
				return RSSL_RET_INVALID_ARGUMENT;
			}

			*pAction = WL_LGCA_CLOSE;

			pLogin->pStream->base.isClosing = RSSL_TRUE;
			wlSetStreamMsgPending(pBase,
					&pLogin->pStream->base);
			pLogin->pStream = NULL;

			return RSSL_RET_SUCCESS;
		}

		default:
			rsslSetErrorInfo(pErrorInfo, RSSL_EIC_FAILURE, RSSL_RET_INVALID_ARGUMENT, 
					__FILE__, __LINE__, "Unsupported Login RDM message type %u.", 
					pLoginMsg->rdmMsgBase.rdmMsgType);
			return RSSL_RET_INVALID_ARGUMENT;

	}
}

RsslRet wlLoginChannelDown(WlLogin *pLogin, WlBase *pBase, RsslErrorInfo *pErrorInfo)
{
	if (pLogin->pStream && (pLogin->pStream->flags & WL_LSF_ESTABLISHED))
	{
		RsslStatusMsg statusMsg;
		RsslRDMLoginMsg loginMsg;
		RsslRet ret;
		RsslDecodeIterator dIter;
		WlLoginProviderAction loginAction;
		RsslWatchlistMsgEvent msgEvent;

		rsslClearStatusMsg(&statusMsg);
		statusMsg.flags |= RSSL_STMF_HAS_STATE;
		statusMsg.state.streamState = RSSL_STREAM_OPEN;
		statusMsg.state.dataState = RSSL_DATA_SUSPECT;
		statusMsg.msgBase.containerType = RSSL_DT_NO_DATA;
		rssl_set_buffer_to_string(statusMsg.state.text, "Channel is down.");

		if ((ret = wlLoginProcessProviderMsg(pLogin, pBase, &dIter, (RsslMsg*)&statusMsg, &loginMsg,
						&loginAction, pErrorInfo)) != RSSL_RET_SUCCESS)
			return ret;

		/* No action. */
		assert(loginAction == WL_LGPA_NONE);

		loginMsg.rdmMsgBase.streamId = pLogin->pRequest->base.streamId;
		wlMsgEventClear(&msgEvent);
		msgEvent.pRsslMsg = (RsslMsg*)&statusMsg;
		msgEvent.pRdmMsg = (RsslRDMMsg*)&loginMsg;
		if ((ret = (*pBase->config.msgCallback)
					((RsslWatchlist*)&pBase->watchlist, &msgEvent, pErrorInfo)) 
				!= RSSL_RET_SUCCESS)
			return ret;
	}

	return RSSL_RET_SUCCESS;
}

