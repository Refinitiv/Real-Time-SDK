/*
 * This source code is provided under the Apache 2.0 license and is provided
 * AS IS with no warranty or guarantee of fit for purpose.  See the project's 
 * LICENSE.md for details. 
 * Copyright (C) 2019 LSEG. All rights reserved.
*/

#include "rtr/wlDirectory.h"
#include "rtr/rsslReactorImpl.h"

WlDirectoryRequest *wlDirectoryRequestCreate(RsslRDMDirectoryRequest *pDirectoryReqMsg,
	   void *pUserSpec, RsslErrorInfo *pErrorInfo)
{
	WlDirectoryRequest *pDirectoryRequest = 
		(WlDirectoryRequest*)malloc(sizeof(WlDirectoryRequest));

	verify_malloc(pDirectoryRequest, pErrorInfo, NULL);
	
	memset(pDirectoryRequest, 0, sizeof(WlDirectoryRequest));
	pDirectoryRequest->state = WL_DR_REQUEST_PENDING_REFRESH;
	if (pDirectoryReqMsg->flags & RDM_DR_RQF_STREAMING)
		pDirectoryRequest->flags |= WL_DRQF_IS_STREAMING;

	wlRequestBaseInit(&pDirectoryRequest->base, pDirectoryReqMsg->rdmMsgBase.streamId, 
			pDirectoryReqMsg->rdmMsgBase.domainType, pUserSpec);
	
	// save filter from request
	pDirectoryRequest->filter = pDirectoryReqMsg->filter;
	
	return pDirectoryRequest;
}

void wlDirectoryRequestClose(WlBase *pBase, WlDirectory *pDirectory,
		WlDirectoryRequest *pDirectoryRequest)
{
	wlRemoveRequest(pBase, &pDirectoryRequest->base);

	if (pDirectoryRequest->pRequestedService)
	{
		rsslQueueRemoveLink(&pDirectoryRequest->pRequestedService->directoryRequests,
				&pDirectoryRequest->qlRequestedService);
		wlRequestedServiceCheckRefCount(pBase, pDirectoryRequest->pRequestedService);
	}
	else
		rsslQueueRemoveLink(&pDirectory->requests, &pDirectoryRequest->qlRequestedService);

	wlDirectoryRequestDestroy(pDirectoryRequest);
}

void wlDirectoryRequestDestroy(WlDirectoryRequest *pDirectoryRequest)
{
	free(pDirectoryRequest);
}


WlDirectoryStream *wlDirectoryStreamCreate(WlBase *pBase, WlDirectory *pDirectory, 
		RsslErrorInfo *pErrorInfo)
{
	RsslInt32 streamId;

	assert(!pDirectory->pStream);
	pDirectory->pStream = (WlDirectoryStream*)malloc(sizeof(WlDirectoryStream));
	verify_malloc(pDirectory->pStream, pErrorInfo, NULL);

	streamId = DIRECTORY_STREAM_ID;

	wlStreamBaseInit(&pDirectory->pStream->base, streamId, RSSL_DMT_SOURCE);

	rsslHashTableInsertLink(&pBase->streamsById, &pDirectory->pStream->base.hlStreamId, 
			&pDirectory->pStream->base.streamId, NULL);

	wlSetStreamMsgPending(pBase, &pDirectory->pStream->base);

	return pDirectory->pStream;
}

void wlDirectoryStreamClose(WlBase *pBase, WlDirectory *pDirectory, RsslBool sendCloseMsg)
{
	assert(pDirectory->pStream);
	if (sendCloseMsg)
	{
		pDirectory->pStream->base.isClosing = RSSL_TRUE;
		wlSetStreamMsgPending(pBase, &pDirectory->pStream->base);
	}
	else
		wlUnsetStreamFromPendingLists(pBase, &pDirectory->pStream->base);

	rsslHashTableRemoveLink(&pBase->streamsById, &pDirectory->pStream->base.hlStreamId);
	pDirectory->pStream = NULL;
}

void wlDirectoryStreamDestroy(WlDirectoryStream *pDirectoryStream)
{
	free(pDirectoryStream);
}

RsslRet wlSendDirectoryMsgToRequest(WlBase *pBase, WlDirectoryRequest *pDirectoryRequest,
		RsslRDMDirectoryMsg *pDirectoryMsg, RsslMsg *pRsslMsg, RsslErrorInfo *pErrorInfo)
{
	RsslWatchlistMsgEvent msgEvent;
	RsslWatchlistStreamInfo streamInfo;

	wlMsgEventClear(&msgEvent);
	wlStreamInfoClear(&streamInfo);
	msgEvent.pRdmMsg = (RsslRDMMsg*)pDirectoryMsg;
	msgEvent.pRsslMsg = (RsslMsg*)pRsslMsg;
	msgEvent.pStreamInfo = &streamInfo;

	streamInfo.pUserSpec = pDirectoryRequest->base.pUserSpec;

	if (pDirectoryMsg)
		pDirectoryMsg->rdmMsgBase.streamId = pDirectoryRequest->base.streamId;

	if (pRsslMsg)
		pRsslMsg->msgBase.streamId = pDirectoryRequest->base.streamId;


	if (pDirectoryRequest->pRequestedService)
	{
		if (pDirectoryRequest->pRequestedService->flags & WL_RSVC_HAS_NAME)
			streamInfo.pServiceName = &pDirectoryRequest->pRequestedService->serviceName;
		else
			assert(pDirectoryRequest->pRequestedService->flags & WL_RSVC_HAS_ID);
	}

	/* set directory response filter and service flags correctly on fanned out RDM directory message */
	if (pDirectoryMsg && pDirectoryRequest->filter > 0)
	{
		RsslUInt32 serviceCount, i;
		if (pDirectoryMsg->rdmMsgBase.rdmMsgType == RDM_DR_MT_REFRESH)
		{
			serviceCount = pDirectoryMsg->refresh.serviceCount;
			pDirectoryMsg->refresh.filter = pDirectoryRequest->filter;
		}
		else if (pDirectoryMsg->rdmMsgBase.rdmMsgType == RDM_DR_MT_UPDATE)
		{
			serviceCount = pDirectoryMsg->update.serviceCount;
			pDirectoryMsg->update.filter = pDirectoryRequest->filter;
		}
		for (i = 0; i < serviceCount; i++)
		{
			if ((pDirectoryRequest->filter & RDM_DIRECTORY_SERVICE_INFO_FILTER) == 0)
			{
				if (pDirectoryMsg->rdmMsgBase.rdmMsgType == RDM_DR_MT_REFRESH)
					pDirectoryMsg->refresh.serviceList[i].flags &= ~RDM_SVCF_HAS_INFO;
				else if (pDirectoryMsg->rdmMsgBase.rdmMsgType == RDM_DR_MT_UPDATE)
					pDirectoryMsg->update.serviceList[i].flags &= ~RDM_SVCF_HAS_INFO;
			}
			if ((pDirectoryRequest->filter & RDM_DIRECTORY_SERVICE_STATE_FILTER) == 0)
			{
				if (pDirectoryMsg->rdmMsgBase.rdmMsgType == RDM_DR_MT_REFRESH)
					pDirectoryMsg->refresh.serviceList[i].flags &= ~RDM_SVCF_HAS_STATE;
				else if (pDirectoryMsg->rdmMsgBase.rdmMsgType == RDM_DR_MT_UPDATE)
					pDirectoryMsg->update.serviceList[i].flags &= ~RDM_SVCF_HAS_STATE;
			}
			if ((pDirectoryRequest->filter & RDM_DIRECTORY_SERVICE_GROUP_FILTER) == 0)
			{
				if (pDirectoryMsg->rdmMsgBase.rdmMsgType == RDM_DR_MT_REFRESH)
					pDirectoryMsg->refresh.serviceList[i].groupStateCount = 0;
				else if (pDirectoryMsg->rdmMsgBase.rdmMsgType == RDM_DR_MT_UPDATE)
					pDirectoryMsg->update.serviceList[i].groupStateCount = 0;
			}
			if ((pDirectoryRequest->filter & RDM_DIRECTORY_SERVICE_LOAD_FILTER) == 0)
			{
				if (pDirectoryMsg->rdmMsgBase.rdmMsgType == RDM_DR_MT_REFRESH)
					pDirectoryMsg->refresh.serviceList[i].flags &= ~RDM_SVCF_HAS_LOAD;
				else if (pDirectoryMsg->rdmMsgBase.rdmMsgType == RDM_DR_MT_UPDATE)
					pDirectoryMsg->update.serviceList[i].flags &= ~RDM_SVCF_HAS_LOAD;
			}
			if ((pDirectoryRequest->filter & RDM_DIRECTORY_SERVICE_DATA_FILTER) == 0)
			{
				if (pDirectoryMsg->rdmMsgBase.rdmMsgType == RDM_DR_MT_REFRESH)
					pDirectoryMsg->refresh.serviceList[i].flags &= ~RDM_SVCF_HAS_DATA;
				else if (pDirectoryMsg->rdmMsgBase.rdmMsgType == RDM_DR_MT_UPDATE)
					pDirectoryMsg->update.serviceList[i].flags &= ~RDM_SVCF_HAS_DATA;
			}
			if ((pDirectoryRequest->filter & RDM_DIRECTORY_SERVICE_LINK_FILTER) == 0)
			{
				if (pDirectoryMsg->rdmMsgBase.rdmMsgType == RDM_DR_MT_REFRESH)
					pDirectoryMsg->refresh.serviceList[i].flags &= ~RDM_SVCF_HAS_LINK;
				else if (pDirectoryMsg->rdmMsgBase.rdmMsgType == RDM_DR_MT_UPDATE)
					pDirectoryMsg->update.serviceList[i].flags &= ~RDM_SVCF_HAS_LINK;
			}
			if ((pDirectoryRequest->filter & RDM_DIRECTORY_SERVICE_SEQ_MCAST_FILTER) == 0)
			{
				if (pDirectoryMsg->rdmMsgBase.rdmMsgType == RDM_DR_MT_REFRESH)
					pDirectoryMsg->refresh.serviceList[i].flags &= ~RDM_SVCF_HAS_SEQ_MCAST;
				else if (pDirectoryMsg->rdmMsgBase.rdmMsgType == RDM_DR_MT_UPDATE)
					pDirectoryMsg->update.serviceList[i].flags &= ~RDM_SVCF_HAS_SEQ_MCAST;
			}
		}
	}

	return _reactorWatchlistMsgCallback(&pBase->watchlist, &msgEvent, pErrorInfo);
}

RsslRet wlSendServiceListToRequest(WlBase *pBase, WlDirectory *pDirectory, 
		WlDirectoryRequest *pDirectoryRequest, RsslRDMService *serviceList, RsslUInt32 serviceCount,
		RsslErrorInfo *pErrorInfo)
{
	RsslRet ret;
	RsslRDMMsg rdmMsg;

	switch(pDirectoryRequest->state)
	{

	case WL_DR_REQUEST_PENDING_REFRESH:
	{
		RsslRDMDirectoryRefresh *pDirectoryRefresh = &rdmMsg.directoryMsg.refresh;
		rsslClearRDMDirectoryRefresh(pDirectoryRefresh);
		pDirectoryRefresh->serviceCount = serviceCount;
		pDirectoryRefresh->serviceList = serviceList;

		if (pDirectoryRequest->pRequestedService
				&& pDirectoryRequest->pRequestedService->flags & WL_RSVC_HAS_ID)
		{
			pDirectoryRefresh->flags |= RDM_DR_RFF_HAS_SERVICE_ID;
			pDirectoryRefresh->serviceId = (RsslUInt16)pDirectoryRequest->pRequestedService->serviceId;
		}

		if (!(pDirectoryRequest->flags & WL_DRQF_IS_STREAMING))
			pDirectoryRefresh->state.streamState = RSSL_STREAM_NON_STREAMING;

		break;
	}

	case WL_DR_REQUEST_OK:
	{
		RsslRDMDirectoryUpdate *pDirectoryUpdate = &rdmMsg.directoryMsg.update;
		rsslClearRDMDirectoryUpdate(pDirectoryUpdate);
		pDirectoryUpdate->serviceCount = serviceCount;
		pDirectoryUpdate->serviceList = serviceList;

		if (pDirectoryRequest->pRequestedService
				&& pDirectoryRequest->pRequestedService->flags & WL_RSVC_HAS_ID)
		{
			pDirectoryUpdate->flags |= RDM_DR_UPF_HAS_SERVICE_ID;
			pDirectoryUpdate->serviceId = (RsslUInt16)pDirectoryRequest->pRequestedService->serviceId;
		}

		break;
	}

	default:
		assert(0);
		break;
	}

	if (pDirectoryRequest->state == WL_DR_REQUEST_PENDING_REFRESH)
		pDirectoryRequest->state = WL_DR_REQUEST_OK;

	if ((ret = wlSendDirectoryMsgToRequest(pBase, pDirectoryRequest, &rdmMsg.directoryMsg,
			NULL, pErrorInfo)) != RSSL_RET_SUCCESS)
		return ret;

	if (!(pDirectoryRequest->flags & WL_DRQF_IS_STREAMING))
		wlDirectoryRequestClose(pBase, pDirectory, pDirectoryRequest);

	return RSSL_RET_SUCCESS;
}

RsslRet wlDirectoryProcessProviderMsgEvent(WlBase *pBase, WlDirectory *pDirectory,
		RsslDecodeIterator *pIter, RsslWatchlistMsgEvent *pMsgEvent, RsslErrorInfo *pErrorInfo)
{
	RsslRDMDirectoryMsg directoryMsg;
	RsslState *pState = NULL;
	RsslBool fanoutDirectoryMsg = RSSL_FALSE;
	RsslRet ret;

	/* Stop timer. */
	if (pBase->pRsslChannel)
		wlUnsetStreamPendingResponse(pBase, &pDirectory->pStream->base);

	if (pMsgEvent->pRsslMsg->msgBase.msgClass == RSSL_MC_GENERIC)
	{
		RsslQueueLink *pLink;

		/* Fanout generic message to all requests. */
		RSSL_QUEUE_FOR_EACH_LINK(&pDirectory->openDirectoryRequests, pLink)
		{
			WlDirectoryRequest *pDirectoryRequest = RSSL_QUEUE_LINK_TO_OBJECT(WlDirectoryRequest, base.qlStateQueue, 
					pLink);

			if ((ret = wlSendDirectoryMsgToRequest(pBase, pDirectoryRequest,
				NULL, pMsgEvent->pRsslMsg, pErrorInfo)) != RSSL_RET_SUCCESS)
				return ret;
		}

		RSSL_QUEUE_FOR_EACH_LINK(&pBase->requestedServices, pLink)
		{
			WlRequestedService *pRequestedService = RSSL_QUEUE_LINK_TO_OBJECT(
				WlRequestedService, qlServiceRequests, pLink);
			RsslQueueLink *pRequestLink;

			pBase->pCurrentWlRequestedService = pRequestedService;

			RSSL_QUEUE_FOR_EACH_LINK(&pRequestedService->openDirectoryRequests,
				pRequestLink)
			{
				WlDirectoryRequest *pDirectoryRequest = RSSL_QUEUE_LINK_TO_OBJECT(WlDirectoryRequest,
					base.qlStateQueue, pRequestLink);

				
				if ((ret = wlSendDirectoryMsgToRequest(pBase, pDirectoryRequest,
					NULL, pMsgEvent->pRsslMsg, pErrorInfo)) != RSSL_RET_SUCCESS)
					return ret;
			}

			// The application might have closed items in the status update callback, so clean up the requested service here if necessary.
			wlRequestedServiceCheckRefCount(pBase, pRequestedService);
			pBase->pCurrentWlRequestedService = NULL;
		}

		return RSSL_RET_SUCCESS;
	}

	/* Decode directory message and cache it. */
	do
	{
		RsslBuffer memBuffer = pBase->tempDecodeBuffer;

		rsslClearDecodeIterator(pIter);
		rsslSetDecodeIteratorRWFVersion(pIter, pBase->pRsslChannel->majorVersion, pBase->pRsslChannel->minorVersion);
		rsslSetDecodeIteratorBuffer(pIter, &pMsgEvent->pRsslMsg->msgBase.encDataBody);
		if ((ret = rsslDecodeRDMDirectoryMsg(pIter, pMsgEvent->pRsslMsg,
				&directoryMsg, &memBuffer, pErrorInfo)) == RSSL_RET_SUCCESS)
			break;

		switch(ret)
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


	switch(directoryMsg.rdmMsgBase.rdmMsgType)
	{
		case RDM_DR_MT_REFRESH:
			pState = &directoryMsg.refresh.state;

			if ((ret = wlServiceCacheProcessDirectoryMsg(pBase->pServiceCache, 
					pBase->pRsslChannel,
					&directoryMsg,
					pErrorInfo)) != RSSL_RET_SUCCESS)
				return ret;

			break;
		case RDM_DR_MT_UPDATE:
			if ((ret = wlServiceCacheProcessDirectoryMsg(pBase->pServiceCache, 
					pBase->pRsslChannel,
					&directoryMsg,
					pErrorInfo)) != RSSL_RET_SUCCESS)
				return ret;
			break;
		case RDM_DR_MT_STATUS:
			if (directoryMsg.status.flags & RDM_DR_STF_HAS_STATE)
				pState = &directoryMsg.status.state;

			if ((ret = wlServiceCacheProcessDirectoryMsg(pBase->pServiceCache, 
					pBase->pRsslChannel,
					&directoryMsg,
					pErrorInfo)) != RSSL_RET_SUCCESS)
				return ret;
			break;
		default:
			fanoutDirectoryMsg = RSSL_TRUE;
			break;
	}

	if (pState && pState->streamState != RSSL_STREAM_OPEN)
	{
		if (pBase->channelState > WL_CHS_LOGGED_IN)
			pBase->channelState = WL_CHS_LOGGED_IN;

		/* Must recover directory stream. */
		pState->streamState = RSSL_STREAM_OPEN;
		pState->dataState = RSSL_DATA_SUSPECT;
	}

	if (fanoutDirectoryMsg)
	{
		RsslQueueLink *pLink;

		RSSL_QUEUE_FOR_EACH_LINK(&pDirectory->openDirectoryRequests, pLink)
		{
			WlDirectoryRequest *pDirectoryRequest = RSSL_QUEUE_LINK_TO_OBJECT(WlDirectoryRequest, base.qlStateQueue, 
					pLink);

			wlSendDirectoryMsgToRequest(pBase, pDirectoryRequest, 
					&directoryMsg, NULL, pErrorInfo);
		}

		RSSL_QUEUE_FOR_EACH_LINK(&pBase->requestedServices, pLink)
		{
			WlRequestedService *pRequestedService = RSSL_QUEUE_LINK_TO_OBJECT(
					WlRequestedService, qlServiceRequests, pLink);
			RsslQueueLink *pRequestLink;

			pBase->pCurrentWlRequestedService = pRequestedService;

			RSSL_QUEUE_FOR_EACH_LINK(&pRequestedService->openDirectoryRequests, 
					pRequestLink)
			{
				WlDirectoryRequest *pDirectoryRequest = RSSL_QUEUE_LINK_TO_OBJECT(WlDirectoryRequest,
						base.qlStateQueue, pRequestLink);

				wlSendDirectoryMsgToRequest(pBase, pDirectoryRequest, 
						&directoryMsg, NULL, pErrorInfo);
			}

			// The application might have closed items in the status update callback, so clean up the requested service here if necessary.
			wlRequestedServiceCheckRefCount(pBase, pRequestedService);
			pBase->pCurrentWlRequestedService = NULL;
		}
	}

	if (pBase->channelState == WL_CHS_CLOSED)
	{
		rsslSetErrorInfo(pErrorInfo, RSSL_EIC_FAILURE, RSSL_RET_FAILURE, __FILE__, __LINE__, 
				"Directory stream was closed.");
		return RSSL_RET_FAILURE;
	}

	return RSSL_RET_SUCCESS;
}

RsslRet wlFanoutDirectoryMsg(WlBase *pBase, WlDirectory *pDirectory, RsslQueue *pUpdatedServiceList,
		RsslErrorInfo *pErrorInfo)
{
	RsslQueueLink *pLink;
	RsslRDMService *fanoutServiceList, *pFanoutService;
	RsslUInt32 fanoutBufferLength = sizeof(RsslRDMService) * pUpdatedServiceList->count;
	RsslQueueLink *pRequestLink;
	RsslRet ret;

	RsslQueueLink* pRequestedServiceLink;
	RDMCachedService* pCachedService;
	WlService* pWlService;

	if (fanoutBufferLength > pBase->tempEncodeBuffer.length)
		if (rsslHeapBufferResize(&pBase->tempEncodeBuffer, fanoutBufferLength, RSSL_FALSE)
				!= RSSL_RET_SUCCESS)
		{
			rsslSetErrorInfo(pErrorInfo, RSSL_EIC_FAILURE, RSSL_RET_FAILURE, __FILE__, __LINE__, 
					"Memory allocation failed.");
			return RSSL_RET_FAILURE;
		}


	pFanoutService = fanoutServiceList = (RsslRDMService*)pBase->tempEncodeBuffer.data;

	/* Build RsslRDMService list to fanout.
	 * (and if there are any service-specific requests, fanout those). */
	RSSL_QUEUE_FOR_EACH_LINK(pUpdatedServiceList, pLink)
	{
		pCachedService = RSSL_QUEUE_LINK_TO_OBJECT(RDMCachedService, _updatedServiceLink,
				pLink);
		pWlService = (WlService*)pCachedService->pUserSpec;

		assert(pWlService);

		rsslClearRDMService(pFanoutService);

		pFanoutService->action = pCachedService->rdm.action;
		pFanoutService->serviceId = pCachedService->rdm.serviceId;

		pFanoutService->flags = pCachedService->updateFlags;

		if (pCachedService->updateFlags & RDM_SVCF_HAS_INFO)
		{
			RsslRDMServiceInfo *pCachedInfo = &pCachedService->rdm.info;
			RsslRDMServiceInfo *pFanoutInfo = &pFanoutService->info;

			*pFanoutInfo = *pCachedInfo;
			pFanoutInfo->flags = pCachedService->infoUpdateFlags;
		}

		if (pCachedService->updateFlags & RDM_SVCF_HAS_STATE)
		{
			RsslRDMServiceState *pCachedState = &pCachedService->rdm.state;
			RsslRDMServiceState *pFanoutState = &pFanoutService->state;

			*pFanoutState = *pCachedState;
			pFanoutState->flags = pCachedService->stateUpdateFlags;
		}

		pFanoutService->groupStateCount = pCachedService->rdm.groupStateCount;
		pFanoutService->groupStateList = pCachedService->rdm.groupStateList;

		if (pCachedService->updateFlags & RDM_SVCF_HAS_LOAD)
		{
			RsslRDMServiceLoad *pCachedLoad = &pCachedService->rdm.load;
			RsslRDMServiceLoad *pFanoutLoad = &pFanoutService->load;

			*pFanoutLoad = *pCachedLoad;
			pFanoutLoad->flags = pCachedService->loadUpdateFlags;
		}

		pFanoutService->data = pCachedService->rdm.data;

		if (pCachedService->updateFlags & RDM_SVCF_HAS_LINK)
		{
			pFanoutService->linkInfo.linkCount = pCachedService->rdm.linkInfo.linkCount;
			pFanoutService->linkInfo.linkList = pCachedService->rdm.linkInfo.linkList;
		}

		++pFanoutService;
	}

	/* Fanout full updates. */
	RSSL_QUEUE_FOR_EACH_LINK(&pDirectory->openDirectoryRequests,
		pRequestLink)
	{
		WlDirectoryRequest* pDirectoryRequest = RSSL_QUEUE_LINK_TO_OBJECT(WlDirectoryRequest, base.qlStateQueue,
			pRequestLink);
		if ((ret = wlSendServiceListToRequest(pBase, pDirectory, pDirectoryRequest, fanoutServiceList,
			pUpdatedServiceList->count, pErrorInfo)) != RSSL_RET_SUCCESS)
			return ret;
	}

	/* Fanout specific requests. */
	pFanoutService = fanoutServiceList;

	RSSL_QUEUE_FOR_EACH_LINK(pUpdatedServiceList, pLink)
	{
		pCachedService = RSSL_QUEUE_LINK_TO_OBJECT(RDMCachedService, _updatedServiceLink,
			pLink);
		pWlService = (WlService*)pCachedService->pUserSpec;

		/* Find requests associated with this service, and fanout. */
		RSSL_QUEUE_FOR_EACH_LINK(&pWlService->requestedServices,
			pRequestedServiceLink)
		{
			WlRequestedService* pRequestedService = RSSL_QUEUE_LINK_TO_OBJECT(WlRequestedService,
				qlDirectoryRequests, pRequestedServiceLink);

			pBase->pCurrentWlRequestedService = pRequestedService;

			RSSL_QUEUE_FOR_EACH_LINK(&pRequestedService->openDirectoryRequests,
				pRequestLink)
			{
				WlDirectoryRequest* pDirectoryRequest = RSSL_QUEUE_LINK_TO_OBJECT(WlDirectoryRequest, base.qlStateQueue,
					pRequestLink);
				if ((ret = wlSendServiceListToRequest(pBase, pDirectory, pDirectoryRequest, pFanoutService, 1,
					pErrorInfo)) != RSSL_RET_SUCCESS)
					return ret;
			}

			// The application might have closed items in the status update callback, so clean up the requested service here if necessary.
			wlRequestedServiceCheckRefCount(pBase, pRequestedService);
			pBase->pCurrentWlRequestedService = NULL;
		}

		++pFanoutService;
	}

	return RSSL_RET_SUCCESS;
}

