/*
 * This source code is provided under the Apache 2.0 license and is provided
 * AS IS with no warranty or guarantee of fit for purpose.  See the project's 
 * LICENSE.md for details. 
 * Copyright (C) 2020 Refinitiv. All rights reserved.
*/

/*
 * This is a basic tunnel stream messages handler for the ETA Value Add consumer application.
 * It handles exchange of simple generic messages over the tunnel stream.
 */

#include "perfTunnelMsgHandler.h"

#define TUNNEL_MSG_STREAM_ID 2000
#define TUNNEL_DOMAIN_TYPE 199


void perfTunnelMsgHandlerInit(PerfTunnelMsgHandler *pMsgHandler,
		char *consumerName, RsslUInt8 domainType, RsslBool useAuthentication, RsslBool isProvider, RsslUInt32 guaranteedOutputTunnelBuffers)
{
	tunnelStreamHandlerInit(&pMsgHandler->tunnelStreamHandler, consumerName, domainType, useAuthentication,
			perfTunnelMsgHandlerProcessTunnelOpened,
			perfTunnelMsgHandlerProcessTunnelClosed,
			perfTunnelMsgHandlerConsumerMsgCallback,
			guaranteedOutputTunnelBuffers
		);

	pMsgHandler->isProvider = isProvider;
	pMsgHandler->processItemMsg = NULL;
	pMsgHandler->guaranteedOutputTunnelBuffers = guaranteedOutputTunnelBuffers;
}

void perfTunnelMsgHandlerAddCallbackProcessItemMsg(PerfTunnelMsgHandler *pMsgHandler, RsslProcessItemMsg* callbackProcessItemMsg)
{
	pMsgHandler->processItemMsg = callbackProcessItemMsg;
}

void startPerfTunnelMsgHandler(RsslReactor *pReactor, RsslReactorChannel *pReactorChannel, PerfTunnelMsgHandler *pPerfTunnelMsgHandler)
{
	handleTunnelStreamHandler(pReactor, pReactorChannel, &pPerfTunnelMsgHandler->tunnelStreamHandler);
}

RsslReactorCallbackRet perfTunnelMsgHandlerConsumerMsgCallback(RsslTunnelStream *pTunnelStream, RsslTunnelStreamMsgEvent *pEvent)
{
	switch(pEvent->containerType)
	{
		case RSSL_DT_OPAQUE:
		{
			/* Read the text contained. */
			printf("Tunnel Stream %d received OPAQUE data: %.*s\n\n", 
					pTunnelStream->streamId, pEvent->pRsslBuffer->length, pEvent->pRsslBuffer->data);
			break;
		}


		default:
		{
			RsslMsg *pRsslMsg = pEvent->pRsslMsg;
			RsslReactorChannel *pReactorChannel = pTunnelStream->pReactorChannel;
			PerfTunnelMsgHandler *pPerfTunnelMsgHandler = (PerfTunnelMsgHandler*)pTunnelStream->userSpecPtr;

			if (pPerfTunnelMsgHandler->processItemMsg != NULL)
			{
				pPerfTunnelMsgHandler->processItemMsg(pReactorChannel, pRsslMsg);
			}
			else
			{
				printf("Received unhandled buffer containerType %d(%s) in tunnel stream %d\n\n",
					pEvent->containerType, rsslDataTypeToString(pEvent->containerType), pTunnelStream->streamId);
			}
			break;
		}
	}	

	return RSSL_RC_CRET_SUCCESS; 
}

RsslReactorCallbackRet perfTunnelMsgHandlerProviderMsgCallback(RsslTunnelStream *pTunnelStream, RsslTunnelStreamMsgEvent *pEvent)
{
	RsslMsg *pRsslMsg = pEvent->pRsslMsg;
	RsslReactorChannel *pReactorChannel = pTunnelStream->pReactorChannel;
	PerfTunnelMsgHandler *pPerfTunnelMsgHandler = (PerfTunnelMsgHandler*)pTunnelStream->userSpecPtr;

	/* Inspect the message and handle it accordingly. This is basically
	 * the same as the consumer's message callback but will respond to the
	 * client's authentication login message if one is received. */

	switch(pEvent->containerType)
	{
		case RSSL_DT_OPAQUE:
		{
			/* Read the text contained. */
			printf("Tunnel Stream %d received OPAQUE data: %.*s\n\n", 
					pTunnelStream->streamId, pEvent->pRsslBuffer->length, pEvent->pRsslBuffer->data);
			break;

		}

		case RSSL_DT_MSG:
		{

			switch(pRsslMsg->msgBase.domainType)
			{
				case RSSL_DMT_LOGIN:
				{
					RsslDecodeIterator dIter;
					RsslRDMLoginMsg loginMsg;
					char tmpMemory[1024];
					RsslBuffer memoryBuffer;
					RsslRet ret;
					RsslErrorInfo errorInfo;

					/* Use the ValueAdd RDM Decoder to decode the login message. */
					rsslClearDecodeIterator(&dIter);
					rsslSetDecodeIteratorRWFVersion(&dIter, pTunnelStream->classOfService.common.protocolMajorVersion,
						pTunnelStream->classOfService.common.protocolMinorVersion);
					rsslSetDecodeIteratorBuffer(&dIter, &pRsslMsg->msgBase.encDataBody);

					rsslClearBuffer(&memoryBuffer);
					memoryBuffer.length = sizeof(tmpMemory);
					memoryBuffer.data = tmpMemory;

					if ((ret = rsslDecodeRDMLoginMsg(&dIter, pRsslMsg, &loginMsg, &memoryBuffer, &errorInfo))
						!= RSSL_RET_SUCCESS)
					{
						printf("rsslDecodeRDMLoginMsg() failed: %s(%s)\n", rsslRetCodeToString(ret), errorInfo.rsslError.text);
						break;
					}

					switch(loginMsg.rdmMsgBase.rdmMsgType)
					{
					case RDM_LG_MT_REQUEST:
						{
							/* This is a login request, likely the client's authentication
							* request. Send a response to establish the tunnel stream. */

							RsslRDMLoginRefresh loginRefresh;
							RsslTunnelStreamSubmitOptions submitOpts;
							RsslTunnelStreamGetBufferOptions bufferOpts;
							RsslBuffer *pBuffer;
							RsslEncodeIterator eIter;
							RsslRet ret, ret2;
							RsslRDMLoginRequest *pLoginRequest = &loginMsg.request;

							printf("Received login request on tunnel stream(ID %d) with stream ID %d.\n", 
								pTunnelStream->streamId, pLoginRequest->rdmMsgBase.streamId);

							if (pLoginRequest->flags & RDM_LG_RQF_NO_REFRESH)
								break;

							rsslClearTunnelStreamGetBufferOptions(&bufferOpts);
							bufferOpts.size = 1024;
							if ((pBuffer = rsslTunnelStreamGetBuffer(pTunnelStream, &bufferOpts, &errorInfo))
								== NULL)
							{
								printf("rsslTunnelStreamGetBuffer failed: %s(%s)\n", rsslRetCodeToString(errorInfo.rsslError.rsslErrorId), errorInfo.rsslError.text);
								break;
							}

							rsslClearRDMLoginRefresh(&loginRefresh);

							/* Set state information */
							loginRefresh.state.streamState = RSSL_STREAM_OPEN;
							loginRefresh.state.dataState = RSSL_DATA_OK;
							loginRefresh.state.code = RSSL_SC_NONE;
							loginRefresh.state.text.data = (char*)"Tunnel login accepted.";
							loginRefresh.state.text.length = (RsslUInt32)strlen(loginRefresh.state.text.data);

							/* Set stream ID */
							loginRefresh.rdmMsgBase.streamId = pLoginRequest->rdmMsgBase.streamId;

							/* Mark refresh as solicited since it is a response to a request. */
							loginRefresh.flags = RDM_LG_RFF_SOLICITED;

							/* Echo the userName, applicationId, applicationName, and position */
							loginRefresh.flags |= RDM_LG_RFF_HAS_USERNAME;
							loginRefresh.userName = pLoginRequest->userName;
							if (pLoginRequest->flags & RDM_LG_RQF_HAS_USERNAME_TYPE)
							{
								loginRefresh.flags |= RDM_LG_RFF_HAS_USERNAME_TYPE;
								loginRefresh.userNameType = pLoginRequest->userNameType;
							}

							loginRefresh.flags |= RDM_LG_RFF_HAS_APPLICATION_ID;
							loginRefresh.applicationId = pLoginRequest->applicationId;

							loginRefresh.flags |= RDM_LG_RFF_HAS_APPLICATION_NAME;
							loginRefresh.applicationName = pLoginRequest->applicationName;

							loginRefresh.flags |= RDM_LG_RFF_HAS_POSITION;
							loginRefresh.position = pLoginRequest->position;

							/* This provider does not support Single-Open behavior. */
							loginRefresh.flags |= RDM_LG_RFF_HAS_SINGLE_OPEN;
							loginRefresh.singleOpen = 0; 

							/* set the clear cache flag */
							loginRefresh.flags |= RDM_LG_RFF_CLEAR_CACHE;

							/* Leave all other parameters as default values. */

							/* Encode the refresh. */
							rsslClearEncodeIterator(&eIter);
							rsslSetEncodeIteratorRWFVersion(&eIter, pTunnelStream->classOfService.common.protocolMajorVersion,
								pTunnelStream->classOfService.common.protocolMinorVersion);
							if((ret = rsslSetEncodeIteratorBuffer(&eIter, pBuffer)) < RSSL_RET_SUCCESS)
							{
								printf("rsslSetEncodeIteratorBuffer(): Failed <%s>\n", errorInfo.rsslError.text);
								if ((ret2 = rsslTunnelStreamReleaseBuffer(pBuffer, &errorInfo)) != RSSL_RET_SUCCESS)
									printf("rsslTunnelStreamReleaseBuffer(): Failed <%d:%s>\n", ret2, errorInfo.rsslError.text);
								break;
							}

							if (rsslEncodeRDMLoginMsg(&eIter, (RsslRDMLoginMsg*)&loginRefresh, &pBuffer->length, &errorInfo) != RSSL_RET_SUCCESS)
							{
								printf("rsslEncodeRDMLoginMsg(): Failed <%s>\n", errorInfo.rsslError.text);
								if ((ret2 = rsslTunnelStreamReleaseBuffer(pBuffer, &errorInfo)) != RSSL_RET_SUCCESS)
									printf("rsslTunnelStreamReleaseBuffer(): Failed <%d:%s>\n", ret2, errorInfo.rsslError.text);
								break;
							}

							/* Message encoding complete; submit it. */
							rsslClearTunnelStreamSubmitOptions(&submitOpts);
							submitOpts.containerType = RSSL_DT_MSG;
							if ((ret = rsslTunnelStreamSubmit(pTunnelStream, pBuffer, &submitOpts, &errorInfo)) != RSSL_RET_SUCCESS)
							{
								printf("rsslTunnelStreamSubmit(): Failed <%s>\n", errorInfo.rsslError.text);
								if ((ret2 = rsslTunnelStreamReleaseBuffer(pBuffer, &errorInfo)) != RSSL_RET_SUCCESS)
									printf("rsslTunnelStreamReleaseBuffer(): Failed <%d:%s>\n", ret2, errorInfo.rsslError.text);
								break;
							}

							printf("Sent response to tunnel login request.\n\n");

							pPerfTunnelMsgHandler->waitingForAuthenticationRequest = RSSL_FALSE;

							break;
						}

					case RDM_LG_MT_CLOSE:
						{
							/* Login close message. */
							RsslRDMLoginClose *pLoginClose = &loginMsg.close;

							printf("Received login close on tunnel stream(ID %d) with stream ID %d.\n", 
								pTunnelStream->streamId, pLoginClose->rdmMsgBase.streamId);
							break;
						}
					}


					break;
				}

				default:
				{
					if (rtrLikely(pPerfTunnelMsgHandler->processItemMsg != NULL && pPerfTunnelMsgHandler->waitingForAuthenticationRequest == RSSL_FALSE))
					{
						pPerfTunnelMsgHandler->processItemMsg(pReactorChannel, pRsslMsg);
					}
					else
					{
						/* Don't recognize this message. */
						printf("Received unhandled message in TunnelStream with stream ID %d, class %u(%s) and domainType %u(%s)\n\n",
							pRsslMsg->msgBase.streamId,
							pRsslMsg->msgBase.msgClass, rsslMsgClassToString(pRsslMsg->msgBase.msgClass),
							pRsslMsg->msgBase.domainType, rsslDomainTypeToString(pRsslMsg->msgBase.domainType));
					}
					break;
				}
			}
			break;
		}

		default:
		{
			printf("Received unhandled buffer containerType %d(%s) in tunnel stream %d\n\n", 
				pEvent->containerType, rsslDataTypeToString(pEvent->containerType), pTunnelStream->streamId);
			break;
		}
	}


	return RSSL_RC_CRET_SUCCESS; 
}

static void perfTunnelMsgHandlerProcessTunnelOpened(RsslTunnelStream *pTunnelStream)
{
	PerfTunnelMsgHandler *pPerfTunnelMsgHandler = (PerfTunnelMsgHandler*)pTunnelStream->userSpecPtr;

	/* If authentication was requested and this is a provider, ensure the authentication response
	 * is sent before sending other messages. */
	if (pTunnelStream->classOfService.authentication.type != RDM_COS_AU_NOT_REQUIRED
			&& pPerfTunnelMsgHandler->isProvider)
		pPerfTunnelMsgHandler->waitingForAuthenticationRequest = RSSL_TRUE;
	else
		pPerfTunnelMsgHandler->waitingForAuthenticationRequest = RSSL_FALSE;


}

static void perfTunnelMsgHandlerProcessTunnelClosed(RsslTunnelStream *pTunnelStream)
{
	/* Nothing to do. */
}

void perfTunnelMsgHandlerProcessNewStream(PerfTunnelMsgHandler *pPerfTunnelMsgHandler,
		RsslTunnelStreamRequestEvent *pEvent)
{
	RsslErrorInfo errorInfo;
	RsslRet ret;
	RsslClassOfService cos;
	RsslBool rejectWithClassOfService = RSSL_FALSE;
	char *rejectString = NULL;

	printf("Received TunnelStream request on Stream ID %d.\n", pEvent->streamId);

	if (pPerfTunnelMsgHandler == NULL)
		rejectString = (char*)"Provider tunnel stream limit reached."; // limited by MAX_TUNNEL_STREAMS in rsslProvider.h
	else if (pPerfTunnelMsgHandler->tunnelStreamHandler.tunnelStreamOpenRequested)
		rejectString = (char*)"Consumer already has a tunnel stream open.";

	if (rejectString == NULL)
	{
		if ((rejectString = perfTunnelMsgHandlerCheckRequestedClassOfService(pPerfTunnelMsgHandler,
			pEvent, &cos)) != NULL)
			rejectWithClassOfService = RSSL_TRUE;
	}

	if (rejectString == NULL)
	{
		/* Accept the tunnel stream. */
		RsslReactorAcceptTunnelStreamOptions acceptOpts;

		rsslClearReactorAcceptTunnelStreamOptions(&acceptOpts);

		acceptOpts.statusEventCallback = tunnelStreamStatusEventCallback;
		acceptOpts.defaultMsgCallback = perfTunnelMsgHandlerProviderMsgCallback;
		acceptOpts.userSpecPtr = (void*)pPerfTunnelMsgHandler;

		acceptOpts.classOfService.authentication.type = cos.authentication.type;
		acceptOpts.classOfService.flowControl.type = RDM_COS_FC_BIDIRECTIONAL;
		acceptOpts.classOfService.dataIntegrity.type = RDM_COS_DI_RELIABLE;
		acceptOpts.guaranteedOutputBuffers = pPerfTunnelMsgHandler->guaranteedOutputTunnelBuffers;

		/* Use whichever protocol minor version is lower. */
		if (cos.common.protocolMinorVersion < acceptOpts.classOfService.common.protocolMinorVersion)
			acceptOpts.classOfService.common.protocolMinorVersion = cos.common.protocolMinorVersion;

		if ((ret = rsslReactorAcceptTunnelStream(pEvent, &acceptOpts, &errorInfo)) != RSSL_RET_SUCCESS)
			printf("rsslReactorAcceptTunnelStream() failed: %s(%s)\n", rsslRetCodeToString(ret), errorInfo.rsslError.text);

		pPerfTunnelMsgHandler->tunnelStreamHandler.tunnelStreamOpenRequested = RSSL_TRUE;
	}
	else
	{
		/* Something didn't match our expectations, so reject the stream. */
		RsslReactorRejectTunnelStreamOptions rejectOpts;
		RsslClassOfService expectedCos;
		rsslClearReactorRejectTunnelStreamOptions(&rejectOpts);

		rejectOpts.state.streamState = RSSL_STREAM_CLOSED;
		rejectOpts.state.dataState = RSSL_DATA_SUSPECT;
		rejectOpts.state.text.data = rejectString;
		rejectOpts.state.text.length = (RsslUInt32)strlen(rejectOpts.state.text.data);

		if (rejectWithClassOfService)
		{
			/* Since we're rejecting due a Class-of-Service mismatch, 
			 * send a redirect state to the consumer. */
			rejectOpts.state.streamState = RSSL_STREAM_REDIRECTED;
			rejectOpts.state.dataState = RSSL_DATA_SUSPECT;

			/* Set what the class of service is expected to be. */
			rsslClearClassOfService(&expectedCos);
			expectedCos.authentication.type = cos.authentication.type;
			expectedCos.flowControl.type = RDM_COS_FC_BIDIRECTIONAL;
			expectedCos.dataIntegrity.type = RDM_COS_DI_RELIABLE;
			rejectOpts.pCos = &expectedCos;
		}
		else
		{
			rejectOpts.state.streamState = RSSL_STREAM_CLOSED;
			rejectOpts.state.dataState = RSSL_DATA_SUSPECT;
		}

		if ((ret = rsslReactorRejectTunnelStream(pEvent, &rejectOpts, &errorInfo)) != RSSL_RET_SUCCESS)
			printf("rsslReactorRejectTunnelStream() failed: %s(%s)\n", rsslRetCodeToString(ret), errorInfo.rsslError.text);
	}
}

char* perfTunnelMsgHandlerCheckRequestedClassOfService(PerfTunnelMsgHandler *pPerfTunnelMsgHandler,
		RsslTunnelStreamRequestEvent *pEvent, RsslClassOfService *pCos)
{
	RsslErrorInfo errorInfo;

	/* 
	 * The class of service filter can be inspected to detect early if
	 * certain members of the ClassOfService are not present (and therefore
	 * have their default values).
	 *
	 * if ((pEvent->classOfServiceFilter & RDM_COS_COMMON_PROPERTIES_FLAG) == 0)
	 *	return "This provider requires the common ClassOfService filter.";
	 * else if ((pEvent->classOfServiceFilter & RDM_COS_AUTHENTICATION_FLAG) == 0)
	 *	return "This provider requires the authentication ClassOfService filter.";
	 * else if ((pEvent->classOfServiceFilter & RDM_COS_FLOW_CONTROL_FLAG) == 0)
	 *	return "This provider requires the flow control ClassOfService filter.";
	 * else if ((pEvent->classOfServiceFilter & RDM_COS_DATA_INTEGRITY_FLAG) == 0)
	 *	return "This provider requires the data integrity ClassOfService filter.";
	 */

	/* Try to decode the class of service. */
	if (rsslTunnelStreamRequestGetCos(pEvent, pCos, &errorInfo) != RSSL_RET_SUCCESS)
	{
		printf("rsslTunnelStreamRequestGetCos failed: %s(%s)\n", rsslRetCodeToString(errorInfo.rsslError.rsslErrorId), errorInfo.rsslError.text);
		return (char*)"Failed to decode class of service.";
	}

	/* Check if it matches what we require. */

	if (pCos->common.protocolType != RSSL_RWF_PROTOCOL_TYPE)
		return (char*)"This provider doesn't support this protocol type.";

	if (pCos->common.protocolMajorVersion != RSSL_RWF_MAJOR_VERSION)
		return (char*)"This provider doesn't support this wire format major version.";

	if (pCos->authentication.type != RDM_COS_AU_NOT_REQUIRED
			&& pCos->authentication.type != RDM_COS_AU_OMM_LOGIN)
		return (char*)"This provider doesn't support this type of authentication.";

	if (pCos->flowControl.type != RDM_COS_FC_BIDIRECTIONAL)
		return (char*)"This provider requires bidirectional flow control.";

	if (pCos->guarantee.type != RDM_COS_GU_NONE)
		return (char*)"This provider does not support guaranteed streams.";

	return NULL;
}

void perfTunnelMsgHandlerCloseStreams(PerfTunnelMsgHandler *pPerfTunnelMsgHandler)
{
	tunnelStreamHandlerCloseStreams(&pPerfTunnelMsgHandler->tunnelStreamHandler);
}

RsslInt perfTunnelMsgHandlerGetStreamId(void)
{
	return tunnelStreamHandlerGetStreamId();
}
