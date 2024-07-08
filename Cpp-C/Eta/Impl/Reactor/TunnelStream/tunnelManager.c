/*
 * This source code is provided under the Apache 2.0 license and is provided
 * AS IS with no warranty or guarantee of fit for purpose.  See the project's 
 * LICENSE.md for details. 
 * Copyright (C) 2019 LSEG. All rights reserved.
*/

#include "rtr/tunnelManager.h"
#include "rtr/tunnelManagerImpl.h"
#include "rtr/tunnelStreamImpl.h"
#include "rtr/msgQueueEncDec.h"
#include <stdlib.h>
#include <assert.h>

/* Sends an event to a provider indicating that a requested tunnel stream was closed. */
static RsslRet _tunnelStreamRequestClose(TunnelManagerImpl *pManagerImpl, NewTunnelStreamRequest *pRequest,
									  RsslErrorInfo *pErrorInfo);

/* Internally rejects a tunnel stream and sends a channel warning event to the provider. */
static RsslRet tunnelManagerAutoReject(
		TunnelManagerImpl *pManagerImpl,
		RsslTunnelStreamRequestEvent *pEvent,
		char *providerWarningString,
		char *rejectionText,
		RsslUInt32	rejectionTextLength,
		RsslUInt8 streamState,
		RsslErrorInfo *pErrorInfo);

TunnelManager *tunnelManagerOpen(RsslReactor *pReactor, RsslReactorChannel *pReactorChannel, RsslErrorInfo *pErrorInfo)
{
	TunnelManagerImpl *pManagerImpl;

	assert(pReactorChannel);
		
	if ((pManagerImpl = (TunnelManagerImpl*)
		malloc(sizeof(TunnelManagerImpl))) == NULL)
	{
		rsslSetErrorInfo(pErrorInfo, RSSL_EIC_FAILURE, RSSL_RET_FAILURE, 
				__FILE__, __LINE__, "Failed to allocate tunnel stream manager.");
		return NULL;
	}

	memset(pManagerImpl, 0, sizeof(TunnelManagerImpl));

	rsslInitQueue(&pManagerImpl->_tunnelBufferPool);
	rsslInitQueue(&pManagerImpl->_tunnelStreams);
	rsslInitQueue(&pManagerImpl->_tunnelStreamsOpen);
	rsslInitQueue(&pManagerImpl->_tunnelStreamDispatchList);
	rsslInitQueue(&pManagerImpl->_tunnelStreamTimeoutList);

	pManagerImpl->base._nextDispatchTime = RDM_QMSG_TC_INFINITE;

	pManagerImpl->_pParentReactor = pReactor;

	if (rsslHashTableInit(&pManagerImpl->_streamIdToTunnelStreamTable, 13, rsslHashU32Sum, rsslHashU32Compare, RSSL_FALSE, pErrorInfo)
			!= RSSL_RET_SUCCESS)
	{
		rsslSetErrorInfoLocation(pErrorInfo, __FILE__, __LINE__);
		tunnelManagerClose((TunnelManager*)pManagerImpl, pErrorInfo);
		return NULL;
	}

	pManagerImpl->base._pReactorChannel = pReactorChannel;

	if (((RsslReactorChannelImpl*)pReactorChannel)->channelRole.base.roleType == RSSL_RC_RT_OMM_PROVIDER)
		pManagerImpl->_listenerCallback = ((RsslReactorChannelImpl*)pReactorChannel)->channelRole.ommProviderRole.tunnelStreamListenerCallback;

	return (TunnelManager*)pManagerImpl;
}

RsslRet tunnelManagerProcessTimer(TunnelManager *pManager, RsslInt64 currentTime, 
		RsslErrorInfo *pErrorInfo)
{
	TunnelManagerImpl *pManagerImpl = (TunnelManagerImpl*)pManager;
	RsslQueueLink *pLink;
	RsslRet ret;

	for(pLink = rsslQueueStart(&pManagerImpl->_tunnelStreamTimeoutList); pLink != NULL;
			pLink = rsslQueueForth(&pManagerImpl->_tunnelStreamTimeoutList))
	{
		TunnelStreamImpl *pTunnelImpl = 
			RSSL_QUEUE_LINK_TO_OBJECT(TunnelStreamImpl, _timeoutLink, pLink);

		if ((ret = tunnelStreamProcessTimer(pTunnelImpl, currentTime, pErrorInfo)) != RSSL_RET_SUCCESS)
			if ((ret = tunnelManagerHandleStreamError(pManager, (RsslTunnelStream*)pTunnelImpl, ret, pErrorInfo))
					!= RSSL_RET_SUCCESS)
				return ret;
	}

	if ((pLink = rsslQueuePeekFront(&pManagerImpl->_tunnelStreamTimeoutList)) != NULL)
	{
		TunnelStreamImpl *pTunnelImpl = 
			RSSL_QUEUE_LINK_TO_OBJECT(TunnelStreamImpl, _timeoutLink, pLink);
		pManagerImpl->base._nextDispatchTime = tunnelStreamGetNextExpireTime(pTunnelImpl);
	}
	else
		pManagerImpl->base._nextDispatchTime = RDM_QMSG_TC_INFINITE;


	return RSSL_RET_SUCCESS;
}

RsslRet tunnelManagerDispatch(TunnelManager *pManager, 
		RsslErrorInfo *pErrorInfo)
{
	TunnelManagerImpl *pManagerImpl = (TunnelManagerImpl*)pManager;
	RsslQueueLink *pLink;

	for(pLink = rsslQueueStart(&pManagerImpl->_tunnelStreamDispatchList);
		pLink; pLink = rsslQueueForth(&pManagerImpl->_tunnelStreamDispatchList))
	{
		RsslRet ret;
		TunnelStreamImpl *pTunnelImpl = 
			RSSL_QUEUE_LINK_TO_OBJECT(TunnelStreamImpl, _dispatchLink, pLink);

		if ((ret = tunnelStreamDispatch((RsslTunnelStream*)pTunnelImpl, pErrorInfo))
				!= RSSL_RET_SUCCESS)
			if ((ret = tunnelManagerHandleStreamError(pManager, (RsslTunnelStream*)pTunnelImpl, ret, pErrorInfo))
					!= RSSL_RET_SUCCESS)
				return ret;
	}

	tunnelManagerSetNeedsDispatchNow(pManagerImpl, 
			pManagerImpl->_tunnelStreamDispatchList.count > 0);

	return RSSL_RET_SUCCESS;
}

RsslTunnelStream* tunnelManagerOpenStream(TunnelManager *pManager,
		RsslTunnelStreamOpenOptions *pOptions, RsslBool isProvider, RsslClassOfService *pRemoteCos, RsslUInt streamVersion, RsslErrorInfo *pErrorInfo)
{
	return tunnelStreamOpen(pManager, pOptions, isProvider, pRemoteCos, streamVersion, pErrorInfo);
}

static RsslRet tunnelManagerAutoReject(
		TunnelManagerImpl *pManagerImpl,
		RsslTunnelStreamRequestEvent *pEvent,
		char *providerWarningString,
		char *rejectionText,
		RsslUInt32	rejectionTextLength,
		RsslUInt8 streamState,
		RsslErrorInfo *pErrorInfo)
{
	RsslReactorChannelEvent channelEvent;
	RsslReactorRejectTunnelStreamOptions rejectOptions;
	RsslReactorCallbackRet cret;
	RsslRet ret;

	/* Send a warning. */
	rsslClearReactorChannelEvent(&channelEvent);
	channelEvent.channelEventType = RSSL_RC_CET_WARNING;
	channelEvent.pReactorChannel = pManagerImpl->base._pReactorChannel;
	channelEvent.pError = pErrorInfo;

	rsslSetErrorInfo(pErrorInfo, RSSL_EIC_FAILURE, RSSL_RET_FAILURE, __FILE__, __LINE__,
			providerWarningString);

	if ((cret = (*((RsslReactorChannelImpl*)pManagerImpl->base._pReactorChannel)->channelRole.base.channelEventCallback)
				(pManagerImpl->_pParentReactor, channelEvent.pReactorChannel, &channelEvent)) != RSSL_RC_CRET_SUCCESS)
	{
		rsslSetErrorInfo(pErrorInfo, RSSL_EIC_FAILURE, RSSL_RET_INVALID_ARGUMENT, __FILE__, __LINE__,
				"Unexpected return code %d from tunnel stream listener callback", cret);
		return RSSL_RET_FAILURE;
	}

	/* Reject it for them. */
	rsslClearReactorRejectTunnelStreamOptions(&rejectOptions);
	rejectOptions.state.streamState = streamState;
	rejectOptions.state.dataState = RSSL_DATA_SUSPECT;
	rejectOptions.state.text.data = rejectionText;
	rejectOptions.state.text.length = rejectionTextLength;

	if ((ret = tunnelManagerRejectStream(&pManagerImpl->base, pEvent,
					&rejectOptions, pErrorInfo)) != RSSL_RET_SUCCESS)
		return ret;

	return RSSL_RET_SUCCESS;
}



RsslRet tunnelManagerReadMsg(TunnelManager *pManager, RsslMsg *pMsg, RsslErrorInfo *pErrorInfo)
{
	TunnelManagerImpl *pManagerImpl = (TunnelManagerImpl*)pManager;
	RsslRet ret;
	RsslHashLink *pHashLink;
	RsslTunnelStream *pTunnelStream;
	RsslReactorChannelImpl* pReactorChannelImpl = (RsslReactorChannelImpl*)pManager->_pReactorChannel;
	RsslReactorImpl* pReactorImpl = pReactorChannelImpl->pParentReactor;

	if ((pHashLink = rsslHashTableFind(&pManagerImpl->_streamIdToTunnelStreamTable, &pMsg->msgBase.streamId, NULL)) != NULL)
	{
		pTunnelStream = (RsslTunnelStream*)RSSL_HASH_LINK_TO_OBJECT(TunnelStreamImpl, _managerHashLink, pHashLink);

		if ((ret = tunnelStreamRead(pTunnelStream, pMsg, pErrorInfo)) != RSSL_RET_SUCCESS)
			return tunnelManagerHandleStreamError(pManager, pTunnelStream, ret, pErrorInfo);
	}
	else if (pManagerImpl->_listenerCallback != NULL)
	{
		switch(pMsg->msgBase.msgClass)
		{
			case RSSL_MC_REQUEST:
			{
				RsslReactorCallbackRet cret;
				NewTunnelStreamRequest newRequest;

				if (!rsslRequestMsgCheckQualifiedStream(&pMsg->requestMsg))
					return RSSL_RET_NO_TUNNEL_STREAM;

				memset(&newRequest, 0, sizeof(NewTunnelStreamRequest));

				newRequest.event.pReactorChannel = (RsslReactorChannel*)pManagerImpl->base._pReactorChannel;
				newRequest.event.streamId = pMsg->msgBase.streamId;
				newRequest.event.domainType = pMsg->msgBase.domainType;
				newRequest.pReqMsg = &pMsg->requestMsg;
				newRequest.streamVersion = COS_CURRENT_STREAM_VERSION;

				if (rsslMsgKeyCheckHasFilter(&pMsg->msgBase.msgKey))
					newRequest.event.classOfServiceFilter = pMsg->msgBase.msgKey.filter;
				else
				{
					if (tunnelManagerAutoReject(pManagerImpl, &newRequest.event,
								(char*)"Rejected a consumer's tunnel stream due to missing msgKey.filter.",
								(char*)"Request is missing msgKey.filter", 32,
								RSSL_STREAM_CLOSED, pErrorInfo) != RSSL_RET_SUCCESS)
						return RSSL_RET_FAILURE;

					return RSSL_RET_SUCCESS;
				}

				newRequest.state = TSR_WAIT_ACCEPT;


				if (rsslMsgKeyCheckHasServiceId(&pMsg->msgBase.msgKey))
					newRequest.event.serviceId = pMsg->msgBase.msgKey.serviceId;
				else
				{
					if (tunnelManagerAutoReject(pManagerImpl, &newRequest.event,
								(char*)"Rejected a consumer's tunnel stream due to missing msgKey.serviceId.",
								(char*)"Request is missing msgKey.serviceId", 35,
								RSSL_STREAM_CLOSED, pErrorInfo) != RSSL_RET_SUCCESS)
						return RSSL_RET_FAILURE;

					return RSSL_RET_SUCCESS;
				}


				if (rsslMsgKeyCheckHasName(&pMsg->msgBase.msgKey))
				{
					if ((newRequest.event.name = (char*)malloc(pMsg->msgBase.msgKey.name.length + 1))
							== NULL)
					{
						rsslSetErrorInfo(pErrorInfo, RSSL_EIC_FAILURE, RSSL_RET_INVALID_ARGUMENT, __FILE__, __LINE__,
								"Failed to allocate memory.");
						return RSSL_RET_FAILURE;
					}

					strncpy(newRequest.event.name, pMsg->msgBase.msgKey.name.data,
							pMsg->msgBase.msgKey.name.length);
					newRequest.event.name[pMsg->msgBase.msgKey.name.length] = '\0';
				}
				else
				{
					if (tunnelManagerAutoReject(pManagerImpl, &newRequest.event,
								(char*)"Rejected a consumer's tunnel stream due to missing msgKey.name.",
								(char*)"Request is missing msgKey.name", 30,
								RSSL_STREAM_CLOSED, pErrorInfo) != RSSL_RET_SUCCESS)
						return RSSL_RET_FAILURE;

					return RSSL_RET_SUCCESS;
				}


				/* If filter list provided, get stream version. If not (though that shouldn't happen), assume defaults. */
				if (pMsg->msgBase.containerType == RSSL_DT_FILTER_LIST)
				{
					RsslDecodeIterator dIter;
					RsslRet ret;
					RsslUInt streamVersion;

					rsslClearDecodeIterator(&dIter);
					rsslSetDecodeIteratorRWFVersion(&dIter, newRequest.event.pReactorChannel->majorVersion, newRequest.event.pReactorChannel->minorVersion);
					rsslSetDecodeIteratorBuffer(&dIter, &newRequest.pReqMsg->msgBase.encDataBody);

					if ((ret = rsslGetClassOfServiceStreamVersion(&dIter, &streamVersion, pErrorInfo))
						!= RSSL_RET_SUCCESS)
					{
						if (tunnelManagerAutoReject(pManagerImpl, &newRequest.event,
									(char*)"Rejected a consumer's tunnel stream due to failure to get the stream version.",
									(char*)"Failed to decode ClassOfService filter list.", 44,
									RSSL_STREAM_CLOSED, pErrorInfo) != RSSL_RET_SUCCESS)
								return RSSL_RET_FAILURE;


						return RSSL_RET_SUCCESS;
					}

					if (streamVersion > COS_CURRENT_STREAM_VERSION)
					{
						if (tunnelManagerAutoReject(pManagerImpl, &newRequest.event,
									(char*)"Rejected a consumer's tunnel stream due to an unrecognized stream version.",
									(char*)"Unknown stream version requested.", 32,
									RSSL_STREAM_CLOSED, pErrorInfo) != RSSL_RET_SUCCESS)
							return RSSL_RET_FAILURE;


						return RSSL_RET_SUCCESS;
					}
					newRequest.streamVersion = streamVersion;
				}

				if (isReactorDebugLevelEnabled(pReactorImpl, RSSL_RC_DEBUG_LEVEL_TUNNELSTREAM))
				{
					if (pReactorImpl->pReactorDebugInfo == NULL || pReactorChannelImpl->pChannelDebugInfo == NULL)
					{
						if (_initReactorAndChannelDebugInfo(pReactorImpl, pReactorChannelImpl, &pReactorChannelImpl->channelWorkerCerr) == RSSL_RET_SUCCESS)
						{
							return RSSL_RET_FAILURE;
						}
					}

					pReactorChannelImpl->pChannelDebugInfo->debugInfoState |= RSSL_RC_DEBUG_INFO_RECEIVE_TUNNEL_REQUEST;

					_writeDebugInfo(pReactorImpl, "Reactor(0x%p), Reactor channel(0x%p) RECEIVES a tunnel stream REQUEST(stream ID=%d) on channel fd="RSSL_REACTOR_SOCKET_PRINT_TYPE".]\n",
						pReactorImpl, pReactorChannelImpl, newRequest.event.streamId, pReactorChannelImpl->reactorChannel.socketId);
				}

				if ((cret = (*pManagerImpl->_listenerCallback)(&newRequest.event, pErrorInfo))
						!= RSSL_RC_CRET_SUCCESS)
				{
					rsslSetErrorInfo(pErrorInfo, RSSL_EIC_FAILURE, RSSL_RET_INVALID_ARGUMENT, __FILE__, __LINE__,
							"Unexpected return code %d from tunnel stream listener callback", cret);
					return RSSL_RET_FAILURE;
				}

				/* Application should have accepted or rejected the tunnel stream. */
				if (newRequest.state == TSR_WAIT_ACCEPT)
				{
					if (tunnelManagerAutoReject(pManagerImpl, &newRequest.event,
								(char*)"A tunnel stream request was not accepted or rejected within the listener callback.",
								(char*)"Provider did not accept or reject this stream.", 46,
								RSSL_STREAM_CLOSED, pErrorInfo) != RSSL_RET_SUCCESS)
							return RSSL_RET_FAILURE;


				}
				break;
			}

			default:
				return RSSL_RET_NO_TUNNEL_STREAM;
		}

	}
	else
		return RSSL_RET_NO_TUNNEL_STREAM;

	tunnelManagerSetNeedsDispatchNow(pManagerImpl, 
			pManagerImpl->_tunnelStreamDispatchList.count > 0);

	return RSSL_RET_SUCCESS;
}

RsslRet tunnelManagerSubmit(TunnelManager *pManager, RsslTunnelStream *pTunnel,
		RsslTunnelStreamSubmitMsgOptions *pOpts, RsslErrorInfo *pErrorInfo)
{
	return tunnelStreamSubmitMsg(pTunnel, pOpts, pErrorInfo);
}

RsslRet tunnelManagerSubmitBuffer(TunnelManager *pManager, RsslTunnelStream *pTunnel,
		RsslBuffer *pBuffer, RsslTunnelStreamSubmitOptions *pOpts, RsslErrorInfo *pErrorInfo)
{
	return tunnelStreamSubmitBuffer(pTunnel, pBuffer, pOpts, pErrorInfo);
}

RsslRet tunnelManagerCloseStream(TunnelManager *pManager,
		RsslTunnelStream *pTunnel, RsslTunnelStreamCloseOptions *pOptions, RsslErrorInfo *pErrorInfo)
{
	return tunnelStreamClose(pTunnel, pOptions, pErrorInfo);
}

RsslRet tunnelManagerHandleStreamError(TunnelManager *pManager, RsslTunnelStream *pTunnelStream, RsslRet errorRetCode, RsslErrorInfo *pErrorInfo)
{
	TunnelManagerImpl *pManagerImpl = (TunnelManagerImpl*)pManager;
	TunnelStreamImpl *pTunnelImpl = (TunnelStreamImpl *)pTunnelStream;
	RsslRet			ret;

	switch(errorRetCode)
	{
		case RSSL_RET_BUFFER_NO_BUFFERS:
		case RSSL_RET_CHANNEL_ERROR:
			/* Reactor channel has to handle these. */
			return errorRetCode;

		default:
			/* For other errors, close the tunnel stream (being sure to send a CloseMsg upstream). */
			if ((ret = tunnelStreamHandleError(pTunnelImpl, pErrorInfo)) != RSSL_RET_SUCCESS)
				return RSSL_RET_FAILURE;
	}

	return RSSL_RET_SUCCESS;
}

RsslRet	tunnelManagerHandleChannelClosed(TunnelManager *pManager, RsslErrorInfo *pErrorInfo)
{
	TunnelManagerImpl *pManagerImpl = (TunnelManagerImpl*)pManager;
	RsslState		state;
	RsslQueueLink	*pLink;
	RsslRet			ret;
	RsslReactorChannelRoleType roleType = ((RsslReactorChannelImpl*)pManagerImpl->base._pReactorChannel)->channelRole.base.roleType;

	rsslClearState(&state);

	if (roleType == RSSL_RC_RT_OMM_PROVIDER)
		state.streamState = RSSL_STREAM_CLOSED;
	else
		state.streamState = RSSL_STREAM_CLOSED_RECOVER;

	state.dataState = RSSL_DATA_SUSPECT;
	state.code = RSSL_SC_NONE;
	state.text.data = (char*)"Channel was closed.";
	state.text.length = 19;

	/* Fanout closes to tunnel streams that the client opened. */
	for(pLink = rsslQueueStart(&pManagerImpl->_tunnelStreamsOpen); pLink != NULL;
			pLink = rsslQueueForth(&pManagerImpl->_tunnelStreamsOpen))
	{
		TunnelStreamImpl *pTunnelImpl = 
			RSSL_QUEUE_LINK_TO_OBJECT(TunnelStreamImpl, _managerOpenLink, pLink);

		if ((ret = tunnelStreamHandleState(pTunnelImpl, &state, NULL, NULL, NULL, RSSL_FALSE, pErrorInfo)) != RSSL_RET_SUCCESS)
			return RSSL_RET_FAILURE;
	}
	return RSSL_RET_SUCCESS;
}

RsslRet tunnelManagerClose(TunnelManager *pManager, RsslErrorInfo *pErrorInfo)
{
	TunnelManagerImpl *pManagerImpl = (TunnelManagerImpl*)pManager;
	RsslQueueLink *pLink;


	for(pLink = rsslQueueStart(&pManagerImpl->_tunnelStreams); pLink != NULL;
			pLink = rsslQueueForth(&pManagerImpl->_tunnelStreams))
	{
		TunnelStreamImpl *pTunnelImpl = RSSL_QUEUE_LINK_TO_OBJECT(
				TunnelStreamImpl, _managerLink, pLink);

		tunnelStreamDestroy((RsslTunnelStream*)pTunnelImpl);
	}

	/* Tunnel streams should release any outstanding buffers; now cleanup buffer pool. */
	while(pLink = rsslQueueRemoveFirstLink(&pManagerImpl->_tunnelBufferPool))
	{
		TunnelBufferImpl *pBufferImpl = RSSL_QUEUE_LINK_TO_OBJECT(
				TunnelBufferImpl, _tbpLink, pLink);

		free(pBufferImpl);
	}
	
	rsslHashTableCleanup(&pManagerImpl->_streamIdToTunnelStreamTable);
	free(pManagerImpl);
	return RSSL_RET_SUCCESS;
}

RsslRet tunnelManagerAcceptStream(TunnelManager *pManager, RsslTunnelStreamRequestEvent *pEvent,
								  RsslReactorAcceptTunnelStreamOptions *pOptions, RsslErrorInfo *pErrorInfo)
{
	RsslReactorChannelImpl *pReactorChannelImpl = (RsslReactorChannelImpl*)pEvent->pReactorChannel;
	RsslReactorImpl *pReactorImpl = (RsslReactorImpl*)pReactorChannelImpl->pParentReactor;
	RsslTunnelStream *pTunnelStream;
	RsslTunnelStreamOpenOptions tsOpts;
	TunnelManagerImpl *pManagerImpl = (TunnelManagerImpl*)pManager;
	NewTunnelStreamRequest *pRequest = (NewTunnelStreamRequest*)pEvent;
	RsslRet ret;

	if (pRequest->state != TSR_WAIT_ACCEPT)
	{
		rsslSetErrorInfo(pErrorInfo, RSSL_EIC_FAILURE, RSSL_RET_INVALID_ARGUMENT, 
				__FILE__, __LINE__, "Tunnel stream request already accepted or rejected.");
		return RSSL_RET_INVALID_ARGUMENT;
	}

	/* If application didn't decode ClassOfService, do it now. */
	if (!pRequest->eventCosDecoded)
		if ((ret = rsslTunnelStreamRequestGetCos(pEvent, &pRequest->classOfService, pErrorInfo))
				!= RSSL_RET_SUCCESS)
		{
			if (tunnelManagerAutoReject(pManagerImpl, &pRequest->event,
						(char*)"Rejected a consumer's tunnel stream due to failure to decode the ClassOfService.",
						(char*)"Failed to decode request ClassOfService", 37,
						RSSL_STREAM_CLOSED, pErrorInfo) != RSSL_RET_SUCCESS)
				return RSSL_RET_FAILURE;
		}

	if (!tunnelManagerValidateCos(&pOptions->classOfService, RSSL_TRUE, pErrorInfo))
		return RSSL_RET_INVALID_ARGUMENT;

	rsslClearTunnelStreamOpenOptions(&tsOpts);
	tsOpts.defaultMsgCallback = pOptions->defaultMsgCallback;
	tsOpts.domainType = pEvent->domainType;
	tsOpts.streamId = pEvent->streamId;
	tsOpts.serviceId = pEvent->serviceId;
	tsOpts.name = pEvent->name;
	tsOpts.statusEventCallback = pOptions->statusEventCallback;
	tsOpts.userSpecPtr = pOptions->userSpecPtr;
	tsOpts.classOfService = pOptions->classOfService;
	tsOpts.guaranteedOutputBuffers = pOptions->guaranteedOutputBuffers;

	/* Open tunnel stream (it will use our already-allocated name instead of copying it) */
	pTunnelStream = tunnelManagerOpenStream(pReactorChannelImpl->pTunnelManager, &tsOpts, 
		RSSL_TRUE, &pRequest->classOfService, pRequest->streamVersion, pErrorInfo);

	if (pTunnelStream == NULL)
		return pErrorInfo->rsslError.rsslErrorId;

	/* Successfully accepted. */
	pRequest->state = TSR_ACCEPTED;
	
	return RSSL_RET_SUCCESS;
}

RsslRet tunnelManagerRejectStream(TunnelManager *pManager, RsslTunnelStreamRequestEvent *pEvent,
								  RsslReactorRejectTunnelStreamOptions *pOptions, RsslErrorInfo *pErrorInfo)
{
	NewTunnelStreamRequest *pRequest = (NewTunnelStreamRequest*)pEvent;
	TunnelManagerImpl *pManagerImpl = (TunnelManagerImpl*)pManager;
	RsslRet ret;
	RsslReactorChannelImpl *pReactorChannelImpl = (RsslReactorChannelImpl*)pEvent->pReactorChannel;
	RsslReactorImpl *pReactorImpl = (RsslReactorImpl*)pReactorChannelImpl->pParentReactor;
	RsslEncodeIterator eIter;
	RsslBuffer *pBuffer;
	TunnelStreamStatus tunnelStatus;

	if (pRequest->state != TSR_WAIT_ACCEPT)
	{
		rsslSetErrorInfo(pErrorInfo, RSSL_EIC_FAILURE, RSSL_RET_INVALID_ARGUMENT, 
				__FILE__, __LINE__, "Tunnel stream request already accepted or rejected.");
		return RSSL_RET_INVALID_ARGUMENT;
	}

	if (pOptions->state.streamState == RSSL_STREAM_OPEN)
	{
		rsslSetErrorInfo(pErrorInfo, RSSL_EIC_FAILURE, RSSL_RET_INVALID_ARGUMENT, 
				__FILE__, __LINE__, "An open stream state cannot be used to reject a tunnel stream.");
		return RSSL_RET_INVALID_ARGUMENT;
	}

	if ((pBuffer = tunnelManagerGetChannelBuffer(pManagerImpl, &eIter,
		512 + (pOptions->state.text.data != NULL ? pOptions->state.text.length : 0),
		RSSL_TRUE /* Must send this if possible */, pErrorInfo)) == NULL)
		return pErrorInfo->rsslError.rsslErrorId;

	tunnelStreamStatusClear(&tunnelStatus);
	tunnelStatus.base.streamId = pEvent->streamId;
	tunnelStatus.base.domainType = pEvent->domainType;
	tunnelStatus.flags = TS_STMF_HAS_STATE;
	tunnelStatus.state = pOptions->state;
	tunnelStatus.serviceId = pEvent->serviceId;
	tunnelStatus.name.data = pEvent->name;
	tunnelStatus.name.length = (RsslUInt32)strlen(pEvent->name);

	if ((ret = tunnelStreamStatusEncode(&eIter, &tunnelStatus, pOptions->pCos, pRequest->streamVersion, pErrorInfo))
			!= RSSL_RET_SUCCESS) 
		return ret;

	pBuffer->length = rsslGetEncodedBufferLength(&eIter);

	if ((ret = tunnelManagerSubmitChannelBuffer(pManagerImpl, pBuffer, pErrorInfo)) != RSSL_RET_SUCCESS)
		return ret;

	if (isReactorDebugLevelEnabled(pReactorImpl, RSSL_RC_DEBUG_LEVEL_TUNNELSTREAM))
	{
		if (pReactorImpl->pReactorDebugInfo == NULL || pReactorChannelImpl->pChannelDebugInfo == NULL)
		{
			if (_initReactorAndChannelDebugInfo(pReactorImpl, pReactorChannelImpl, &pReactorChannelImpl->channelWorkerCerr) == RSSL_RET_SUCCESS)
			{
				return RSSL_RET_FAILURE;
			}
		}

		pReactorChannelImpl->pChannelDebugInfo->debugInfoState |= RSSL_RC_DEBUG_INFO_REJECT_TUNNEL_REQUEST;

		_writeDebugInfo(pReactorImpl, "Reactor(0x%p), Reactor channel(0x%p) REJECTS a tunnel stream REQUEST(stream ID=%d) on channel fd="RSSL_REACTOR_SOCKET_PRINT_TYPE".]\n",
			pReactorImpl, pReactorChannelImpl, tunnelStatus.base.streamId, pReactorChannelImpl->reactorChannel.socketId);
	}

	/* Successfully rejected */
	pRequest->state = TSR_REJECTED;

	if (pEvent->name)
	{
		free(pEvent->name);
		pEvent->name = NULL;
	}


	return RSSL_RET_SUCCESS;
}

RsslBuffer* tunnelManagerGetChannelBuffer(TunnelManagerImpl *pManagerImpl,
		RsslEncodeIterator *pIter, RsslUInt32 length, RsslBool mustSendMsg, RsslErrorInfo *pErrorInfo)
{
	RsslBuffer *pBuffer;
	RsslReactorChannel	*pReactorChannel = pManagerImpl->base._pReactorChannel;
	RsslRet ret;

	assert(pReactorChannel);

	if ((pBuffer = rsslReactorGetBuffer(pReactorChannel, length, RSSL_FALSE,
		pErrorInfo)) == NULL)
	{
		RsslReactorChannelInfo channelInfo;

		if (!mustSendMsg)
			return NULL;

		if (pErrorInfo->rsslError.rsslErrorId != RSSL_RET_BUFFER_NO_BUFFERS)
			return NULL;

		if ((ret = rsslReactorGetChannelInfo(pReactorChannel, &channelInfo, pErrorInfo)
					!= RSSL_RET_SUCCESS))
			return NULL;

		/* Grow the buffer pool by one and attempt to get the buffer again. */

		/* Try the shared pool first. */
		channelInfo.rsslChannelInfo.maxOutputBuffers += 1;
		if ((ret = rsslReactorChannelIoctl(pReactorChannel, 
						RSSL_MAX_NUM_BUFFERS,
						&channelInfo.rsslChannelInfo.maxOutputBuffers,
						pErrorInfo)) != RSSL_RET_SUCCESS)
		{
			/* If that doesn't work, try the guaranteed pool. */
			channelInfo.rsslChannelInfo.guaranteedOutputBuffers += 1;
			if ((ret = rsslReactorChannelIoctl(pReactorChannel, 
							RSSL_NUM_GUARANTEED_BUFFERS,
							&channelInfo.rsslChannelInfo.guaranteedOutputBuffers,
							pErrorInfo)) != RSSL_RET_SUCCESS)
				return NULL;
		}

		if ((pBuffer = rsslReactorGetBuffer(pReactorChannel, length, RSSL_FALSE,
						pErrorInfo)) == NULL)
			return NULL;
	}

	if (pIter != NULL)
	{
		rsslClearEncodeIterator(pIter);
		rsslSetEncodeIteratorRWFVersion(pIter, pReactorChannel->majorVersion,
				pReactorChannel->minorVersion);

		if ((ret = rsslSetEncodeIteratorBuffer(pIter, pBuffer)) != RSSL_RET_SUCCESS)
		{
			RsslErrorInfo errorInfo;
			rsslReactorReleaseBuffer(pReactorChannel, pBuffer, &errorInfo);
			rsslSetErrorInfo(pErrorInfo, RSSL_EIC_FAILURE, RSSL_RET_FAILURE, 
					__FILE__, __LINE__, "Failed to set encode iterator on buffer.");
			return NULL;
		}
	}

	return pBuffer;
}

RsslRet tunnelManagerSubmitChannelBuffer(TunnelManagerImpl *pManagerImpl,
		RsslBuffer *pBuffer, RsslErrorInfo *pErrorInfo)
{
	RsslRet				ret;
	RsslReactorChannelImpl	*pReactorChannelImpl = (RsslReactorChannelImpl*)pManagerImpl->base._pReactorChannel;
	
	if (pReactorChannelImpl->pWatchlist != NULL)
	{
		RsslWatchlistProcessMsgOptions processOpts;

		rsslWatchlistClearProcessMsgOptions(&processOpts);
		processOpts.pChannel = pReactorChannelImpl->reactorChannel.pRsslChannel;
		processOpts.majorVersion = pReactorChannelImpl->reactorChannel.majorVersion;
		processOpts.minorVersion = pReactorChannelImpl->reactorChannel.minorVersion;
		processOpts.pRsslBuffer = pBuffer;

		ret = rsslWatchlistSubmitBuffer(pReactorChannelImpl->pWatchlist, &processOpts, pErrorInfo);

		if (ret < RSSL_RET_SUCCESS)
		{
			RsslErrorInfo errorInfo;
			rsslReactorReleaseBuffer(&pReactorChannelImpl->reactorChannel, pBuffer, &errorInfo);
			return ret;
		}

		return RSSL_RET_SUCCESS;
	}
	else
	{
		RsslReactorSubmitOptions submitOpts;
		rsslClearReactorSubmitOptions(&submitOpts);
		if ((ret = rsslReactorSubmit(pManagerImpl->_pParentReactor, &pReactorChannelImpl->reactorChannel, pBuffer, &submitOpts, pErrorInfo)) < RSSL_RET_SUCCESS)
		{
			RsslErrorInfo errorInfo;
			rsslReactorReleaseBuffer(&pReactorChannelImpl->reactorChannel, pBuffer, &errorInfo);
			return ret;
		}
	}
	
	return RSSL_RET_SUCCESS;
}

RSSL_VA_API RsslRet rsslTunnelStreamRequestGetCos(RsslTunnelStreamRequestEvent *pEvent, RsslClassOfService *pCos, RsslErrorInfo *pError)
{
	NewTunnelStreamRequest *pRequest = (NewTunnelStreamRequest*)pEvent;

	if (pEvent == NULL)
	{
		rsslSetErrorInfo(pError, RSSL_EIC_FAILURE, RSSL_RET_INVALID_ARGUMENT, __FILE__, __LINE__, "RsslTunnelStreamRequestEvent not provided.");
		return RSSL_RET_INVALID_ARGUMENT;
	}

	if (pCos == NULL)
	{
		rsslSetErrorInfo(pError, RSSL_EIC_FAILURE, RSSL_RET_INVALID_ARGUMENT, __FILE__, __LINE__, "RsslClassOfService not provided.");
		return RSSL_RET_INVALID_ARGUMENT;
	}

	if (pError == NULL)
		return RSSL_RET_INVALID_ARGUMENT;

	if (!pRequest->eventCosDecoded)
	{
		rsslClearClassOfService(pCos);

		switch(pRequest->pReqMsg->msgBase.containerType)
		{
			case RSSL_DT_FILTER_LIST:
			{
				RsslDecodeIterator dIter;
				RsslRet ret;

				rsslClearDecodeIterator(&dIter);
				rsslSetDecodeIteratorRWFVersion(&dIter, pRequest->event.pReactorChannel->majorVersion, pRequest->event.pReactorChannel->minorVersion);
				rsslSetDecodeIteratorBuffer(&dIter, &pRequest->pReqMsg->msgBase.encDataBody);

				if ((ret = rsslDecodeClassOfService(&dIter, &pRequest->classOfService, NULL, pError)) != RSSL_RET_SUCCESS)
					return ret;

				break;
			}

			default:
				break;
		}

		pRequest->eventCosDecoded = RSSL_TRUE;
	}

	*pCos = pRequest->classOfService;
	return RSSL_RET_SUCCESS;
}


RsslBool tunnelManagerValidateCos(RsslClassOfService *pCos, RsslBool isProvider, RsslErrorInfo *pErrorInfo)
{
	/* Common (Nothing) */

	/* Authentication */
	switch (pCos->authentication.type)
	{
		case RDM_COS_AU_NOT_REQUIRED:
			break;

		case RDM_COS_AU_OMM_LOGIN:
			if (pCos->common.protocolType != RSSL_RWF_PROTOCOL_TYPE)
			{
				rsslSetErrorInfo(pErrorInfo, RSSL_EIC_FAILURE, RSSL_RET_INVALID_ARGUMENT, 
						__FILE__, __LINE__, "ClassOfService.authentication.type of RDM_COS_AU_OMM_LOGIN requires ClassOfService.common.protocolType of RSSL_RWF_PROTOCOL_TYPE");
				return RSSL_FALSE;
			}
			break;

		default:
			break;
	}

	/* Flow control */
	switch (pCos->flowControl.type)
	{
		case RDM_COS_FC_BIDIRECTIONAL:
			if (pCos->flowControl.recvWindowSize < TS_USE_DEFAULT_RECV_WINDOW_SIZE)
			{
				rsslSetErrorInfo(pErrorInfo, RSSL_EIC_FAILURE, RSSL_RET_INVALID_ARGUMENT, 
						__FILE__, __LINE__, "Invalid ClassOfService.flowControl.recvWindowSize %lld", pCos->flowControl.recvWindowSize);
				return RSSL_FALSE;
			}

			if (pCos->dataIntegrity.type != RDM_COS_DI_RELIABLE)
			{
				rsslSetErrorInfo(pErrorInfo, RSSL_EIC_FAILURE, RSSL_RET_INVALID_ARGUMENT, 
						__FILE__, __LINE__, "ClassOfService.flowControl.type of RDM_COS_FC_BIDIRECTIONAL requires ClassOfService.dataIntegrity.type of RDM_COS_DI_RELIABLE");
				return RSSL_FALSE;
			}
			break;

		case RDM_COS_FC_NONE:
			break;

		default:
			break;
	}

	/* Data Integrity */
	switch (pCos->dataIntegrity.type)
	{
		case RDM_COS_FC_NONE:
			rsslSetErrorInfo(pErrorInfo, RSSL_EIC_FAILURE, RSSL_RET_INVALID_ARGUMENT, 
					__FILE__, __LINE__, "Data Integrity type must be set to RDM_COS_DI_RELIABLE in this version.");
			break;

		default:
			break;
	}

	/* Guarantee */
	switch (pCos->guarantee.type)
	{
		case RDM_COS_GU_PERSISTENT_QUEUE:
			if (isProvider)
			{
				rsslSetErrorInfo(pErrorInfo, RSSL_EIC_FAILURE, RSSL_RET_INVALID_ARGUMENT, 
						__FILE__, __LINE__, "ClassOfService.guarantee.type setting of RDM_COS_GU_PERSISTENT_QUEUE is not supported for a provider.");
				return RSSL_RET_INVALID_ARGUMENT;
			}

			if (pCos->common.protocolType != RSSL_RWF_PROTOCOL_TYPE)
			{
				rsslSetErrorInfo(pErrorInfo, RSSL_EIC_FAILURE, RSSL_RET_INVALID_ARGUMENT, 
						__FILE__, __LINE__, "ClassOfService.guarantee.type of RDM_COS_GU_PERSISTENT_QUEUE requires ClassOfService.common.protocolType of RSSL_RWF_PROTOCOL_TYPE");
				return RSSL_FALSE;
			}
			break;

		default:
			break;
	}

	return RSSL_TRUE;
}

