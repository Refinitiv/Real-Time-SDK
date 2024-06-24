/*
 * This source code is provided under the Apache 2.0 license and is provided
 * AS IS with no warranty or guarantee of fit for purpose.  See the project's 
 * LICENSE.md for details. 
 * Copyright (C) 2019-2020 LSEG. All rights reserved.     
*/

#include "rtr/rsslWatchlistImpl.h"
#include "rtr/rsslReactorImpl.h"
#include <assert.h>

static RsslRet wlWriteBuffer(RsslWatchlistImpl *pWatchlistImpl, RsslBuffer *pWriteBuffer,
		RsslErrorInfo *pError);

/*** Main interface functions. ***/

RsslWatchlist *rsslWatchlistCreate(RsslWatchlistCreateOptions *pCreateOptions, 
		RsslErrorInfo *pErrorInfo)
{
	RsslWatchlistImpl				*pWatchlistImpl;
	WlBaseInitOptions				baseInitOpts;
	RsslRet							ret;

	if (!pCreateOptions->msgCallback) {
		rsslSetErrorInfo(pErrorInfo, RSSL_EIC_FAILURE, RSSL_RET_INVALID_ARGUMENT, __FILE__, __LINE__, 
				"No callback function specified.");
		return NULL;
	}

	pWatchlistImpl = (RsslWatchlistImpl*)malloc(sizeof(RsslWatchlistImpl));

	verify_malloc(pWatchlistImpl, pErrorInfo, NULL);

	memset(pWatchlistImpl, 0, sizeof(RsslWatchlistImpl));

	memset(&baseInitOpts, 0, sizeof(WlBaseInitOptions));
	baseInitOpts.msgCallback = pCreateOptions->msgCallback;
	baseInitOpts.updateCallback = wlServiceUpdateCallback;
	baseInitOpts.requestPoolBlockSize = sizeof(WlRequest);
	baseInitOpts.requestPoolCount = pCreateOptions->itemCountHint;
	baseInitOpts.streamPoolBlockSize = sizeof(WlStream);
	baseInitOpts.streamPoolCount = pCreateOptions->itemCountHint;
	baseInitOpts.obeyOpenWindow = pCreateOptions->obeyOpenWindow;
	baseInitOpts.requestTimeout = pCreateOptions->requestTimeout;
	baseInitOpts.ticksPerMsec = pCreateOptions->ticksPerMsec;
	baseInitOpts.maxOutstandingPosts = pCreateOptions->maxOutstandingPosts;
	baseInitOpts.postAckTimeout = pCreateOptions->postAckTimeout;
	baseInitOpts.enableWarmStandBy = pCreateOptions->enableWarmStandby;

	if (baseInitOpts.enableWarmStandBy)
	{
		baseInitOpts.serviceStateChangeCallback = wlServiceStateChangeCallback;
		baseInitOpts.serviceCacheInitCallback = wlServiceCacheInitCallback;
		baseInitOpts.serviceCacheUpdateCallback = wlServiceCacheUpdateCallback;
	}

	if ((ret = wlBaseInit(&pWatchlistImpl->base, &baseInitOpts, pErrorInfo)) != RSSL_RET_SUCCESS)
	{
		free(pWatchlistImpl);
		return NULL;
	}

	if (wlItemsInit(&pWatchlistImpl->items, pErrorInfo) != RSSL_RET_SUCCESS)
	{
		rsslWatchlistDestroy((RsslWatchlist*)pWatchlistImpl);
		return NULL;
	}

	pWatchlistImpl->login.pRequest = (WlLoginRequest**)(malloc(pCreateOptions->loginRequestCount*sizeof(WlLoginRequest*)));

	if (pWatchlistImpl->login.pRequest == 0)
	{
		rsslSetErrorInfo(pErrorInfo, RSSL_EIC_FAILURE, RSSL_RET_FAILURE, __FILE__, __LINE__,
			"Memory allocation failed.");
		rsslWatchlistDestroy((RsslWatchlist*)pWatchlistImpl);
		return NULL;
	}

	memset(pWatchlistImpl->login.pRequest, 0, pCreateOptions->loginRequestCount*sizeof(WlLoginRequest*));
	pWatchlistImpl->login.count = pCreateOptions->loginRequestCount;
	pWatchlistImpl->login.index = 0;
	pWatchlistImpl->login.pStream = NULL;

	pWatchlistImpl->directory.pStream = NULL;

	rsslInitQueue(&pWatchlistImpl->directory.requests);
	rsslInitQueue(&pWatchlistImpl->directory.openDirectoryRequests);
	rsslInitQueue(&pWatchlistImpl->services);

	return (RsslWatchlist*)pWatchlistImpl;
}

RsslRet rsslWatchlistSetChannel(RsslWatchlist *pWatchlist, RsslChannel *pChannel,
		RsslErrorInfo *pErrorInfo)
{
	RsslWatchlistImpl *pWatchlistImpl = (RsslWatchlistImpl*)pWatchlist;
	pWatchlistImpl->base.pRsslChannel = pChannel;

	if (!pChannel)
	{
		int ret;
		pWatchlistImpl->base.channelState = WL_CHS_START;
		pWatchlistImpl->base.watchlist.state = 0;
		pWatchlistImpl->items.pCurrentFanoutStream = NULL;
		pWatchlistImpl->items.pCurrentFanoutGroup = NULL;
		pWatchlistImpl->items.pCurrentFanoutFTGroup = NULL;

		if ((ret = wlLoginChannelDown(&pWatchlistImpl->login, &pWatchlistImpl->base, pErrorInfo))
				!= RSSL_RET_SUCCESS)
			return ret;

		/* clean out the directory pending response */
		if (pWatchlistImpl->directory.pStream)
			wlUnsetStreamPendingResponse(&pWatchlistImpl->base, &pWatchlistImpl->directory.pStream->base);

		return wlRecoverAllItems(pWatchlistImpl, pErrorInfo);
	}

	return RSSL_RET_SUCCESS;
}

RsslInt64 rsslWatchlistProcessFTGroupPing(RsslWatchlist *pWatchlist, RsslUInt8 ftGroupId, 
		RsslInt64 currentTime)
{
	RsslWatchlistImpl *pWatchlistImpl = (RsslWatchlistImpl*)pWatchlist;
	WlFTGroup *pGroup = pWatchlistImpl->items.ftGroupTable[ftGroupId];

	assert(pWatchlistImpl->base.pRsslChannel);

	if (!pGroup) return WL_TIME_UNSET;

	/* Received a ping for an FTGroup we're using. Reset the timer and put it on
	 * the back of the timeout queue. */
	rsslQueueRemoveLink(&pWatchlistImpl->items.ftGroupTimerQueue, &pGroup->qlGroups);
	rsslQueueAddLinkToBack(&pWatchlistImpl->items.ftGroupTimerQueue, &pGroup->qlGroups);
	pGroup->expireTime = currentTime + pWatchlistImpl->base.pRsslChannel->pingTimeout * 1000;
	pWatchlist->state |= RSSLWL_STF_NEED_TIMER;

	return pGroup->expireTime;
}

void rsslWatchlistDestroy(RsslWatchlist *pWatchlist)
{
	RsslQueueLink *pLink;
	RsslWatchlistImpl *pWatchlistImpl = (RsslWatchlistImpl*)pWatchlist;
	RsslUInt32 i;

	if (pWatchlistImpl->login.pRequest)
	{
		for (i = 0; i < pWatchlistImpl->login.count; i++)
		{
			if (pWatchlistImpl->login.pRequest[i])
				wlLoginRequestDestroy(&pWatchlistImpl->base, pWatchlistImpl->login.pRequest[i]);
		}

		free(pWatchlistImpl->login.pRequest);
	}

	if (pWatchlistImpl->login.pStream)
	{
		WlLoginStream *pLoginStream = pWatchlistImpl->login.pStream;
		wlLoginStreamClose(&pWatchlistImpl->base, &pWatchlistImpl->login, RSSL_FALSE);

		wlLoginStreamDestroy(pLoginStream);
	}

	while (pLink = rsslQueueRemoveFirstLink(&pWatchlistImpl->base.streamsPendingRequest))
	{
		WlStream *pStream = RSSL_QUEUE_LINK_TO_OBJECT(WlStream,
			base.qlStreamsPendingRequest, pLink);
		if (pStream->base.isClosing)
		{
			if (!pStream->base.tempStream)
			{
				switch (pStream->base.domainType)
				{
					case RSSL_DMT_LOGIN:
					{
						WlLoginStream *pLoginStream = (WlLoginStream*)pStream;
						wlLoginStreamDestroy(pLoginStream);
						break;
					}
					case RSSL_DMT_SOURCE:
					{
						WlDirectoryStream *pDirectoryStream = (WlDirectoryStream*)pStream;
						wlDirectoryStreamDestroy(pDirectoryStream);
						break;
					}
					default:
					{
						WlItemStream *pItemStream = (WlItemStream*)pStream;
						wlItemStreamDestroy(&pWatchlistImpl->base, pItemStream);
						break;
					}
				}
			}
			else {
				/* Stream was created only for closing, so only the base structure is allocated. */
				free(pStream);
			}
		}
	}

	while(pLink = rsslQueueRemoveFirstLink(&pWatchlistImpl->base.requestedServices))
	{
		RsslQueueLink *pRequestLink;
		WlRequestedService *pRequestedService = RSSL_QUEUE_LINK_TO_OBJECT(WlRequestedService, 
				qlServiceRequests, pLink);

		while (pRequestLink = rsslQueueRemoveFirstLink(&pRequestedService->directoryRequests))
		{
			WlDirectoryRequest *pDirectoryRequest = RSSL_QUEUE_LINK_TO_OBJECT(WlDirectoryRequest,
					qlRequestedService, pRequestLink);
			wlDirectoryRequestDestroy(pDirectoryRequest);
		}

		while (pRequestLink = rsslQueueRemoveFirstLink(&pRequestedService->itemRequests))
		{
			WlItemRequest *pItemRequest = RSSL_QUEUE_LINK_TO_OBJECT(WlItemRequest,
					qlRequestedService, pRequestLink);
			switch(pItemRequest->base.domainType)
			{
				case RSSL_DMT_SYMBOL_LIST:
					wlSymbolListRequestDestroy(&pWatchlistImpl->base, &pWatchlistImpl->items,
							(WlSymbolListRequest*)pItemRequest);
					break;

				default:
					wlItemRequestDestroy(&pWatchlistImpl->base, pItemRequest);
					break;
			}
		}

		wlRequestedServiceDestroy(pRequestedService);
	}

	while(pLink = rsslQueueRemoveFirstLink(&pWatchlistImpl->services))
	{
		WlService *pService = RSSL_QUEUE_LINK_TO_OBJECT(WlService, 
				qlServices, pLink);
		RsslQueueLink *pGroupLink;

		while(pGroupLink = rsslQueuePeekFront(&pService->itemGroups))
		{
			WlItemGroup *pItemGroup = RSSL_QUEUE_LINK_TO_OBJECT(WlItemGroup, qlItemGroups,
					pGroupLink);
			wlItemGroupRemove(pItemGroup);
		}


		wlServiceDestroy(pService);
	}

	for(i = 0; i < WL_FTGROUP_TABLE_SIZE; ++i)
	{
		if (pWatchlistImpl->items.ftGroupTable[i])
			wlFTGroupRemove(&pWatchlistImpl->items, pWatchlistImpl->items.ftGroupTable[i]);
	}

	while(pLink = rsslQueueRemoveFirstLink(&pWatchlistImpl->base.openStreams))
	{
		WlItemStream *pItemStream = RSSL_QUEUE_LINK_TO_OBJECT(WlItemStream, base.qlStreamsList, 
				pLink);
		wlItemStreamDestroy(&pWatchlistImpl->base, pItemStream);
	}

	while (pLink = rsslQueueRemoveFirstLink(&pWatchlistImpl->directory.requests))
	{
		WlDirectoryRequest *pDirectoryRequest = RSSL_QUEUE_LINK_TO_OBJECT(WlDirectoryRequest,
				qlRequestedService, pLink);
		wlDirectoryRequestDestroy(pDirectoryRequest);
	}

	wlDirectoryStreamDestroy(pWatchlistImpl->directory.pStream);

	wlItemsCleanup(&pWatchlistImpl->items);
	wlBaseCleanup(&pWatchlistImpl->base);
	free(pWatchlistImpl);
}

RsslRet rsslWatchlistDispatch(RsslWatchlist *pWatchlist, RsslInt64 currentTime, 
		RsslErrorInfo *pErrorInfo)
{
	RsslWatchlistImpl *pWatchlistImpl = (RsslWatchlistImpl*)pWatchlist;
	RsslReactorChannelImpl *pReactorChannelImpl = (RsslReactorChannelImpl*)pWatchlist->pUserSpec;
	RsslRet ret;
	RsslQueueLink *pLink;
	RsslUInt32 pendingWaitCount = 0;

	pWatchlistImpl->base.currentTime = currentTime;

	/* Update the login index when the channel supports the session management */
	if (_reactorHandlesWarmStandby(pReactorChannelImpl) == RSSL_FALSE)
	{
		if (pReactorChannelImpl->supportSessionMgnt || pReactorChannelImpl->channelRole.ommConsumerRole.pLoginRequestList)
		{
			pWatchlistImpl->login.index = pReactorChannelImpl->connectionListIter;
		}
	}
	else
	{
		pWatchlistImpl->login.index = 0;
	}

	while (pLink = rsslQueueRemoveFirstLink(&pWatchlistImpl->base.newRequests))
	{
		WlRequest *pRequest = RSSL_QUEUE_LINK_TO_OBJECT(WlRequest, base.qlStateQueue, pLink);

		switch(pRequest->base.domainType)
		{
			case RSSL_DMT_SOURCE:
			{
				WlDirectoryRequest *pDirectoryRequest = (WlDirectoryRequest*)pRequest;
				WlRequestedService *pRequestedService = pDirectoryRequest->pRequestedService;

				WlServiceList serviceList;

				/* Provide refresh from cache, if currently available. Else it will have to wait
				 * until the directory is received from the provider. 
				 * Either way, move it to the list of open requests for that service. */

				if (pRequestedService)
				{
					rsslQueueAddLinkToBack(&pRequestedService->openDirectoryRequests,
							&pDirectoryRequest->base.qlStateQueue);
					pDirectoryRequest->base.pStateQueue = 
						&pRequestedService->openDirectoryRequests;

					if (pRequestedService->flags & WL_RSVC_HAS_NAME)
					{
						if ((ret = wlServiceCacheGetServiceList(pWatchlistImpl->base.pServiceCache,
								&serviceList, NULL, &pRequestedService->serviceName, RSSL_FALSE, 
								pErrorInfo)) != RSSL_RET_SUCCESS)
							return ret;

						assert(serviceList.serviceCount <= 1);
					}
					else 
					{
						if ((ret = wlServiceCacheGetServiceList(pWatchlistImpl->base.pServiceCache,
								&serviceList, &pRequestedService->serviceId, NULL, RSSL_FALSE, 
								pErrorInfo)) != RSSL_RET_SUCCESS)
							return ret;
						assert(serviceList.serviceCount <= 1);
					}

					/* Send empty response when either service name or service ID is not found. */
					/* ... and service cache is not empty. */
					if (serviceList.serviceCount == 0 && pWatchlistImpl->base.pServiceCache->_serviceList.count > 0)
					{
						wlSendServiceListToRequest(&pWatchlistImpl->base, &pWatchlistImpl->directory,
							pDirectoryRequest, NULL, 0,
							pErrorInfo);
					}
				}
				else
				{
					/* Add to list of requests for all services. */
					rsslQueueAddLinkToBack(&pWatchlistImpl->directory.openDirectoryRequests,
							&pDirectoryRequest->base.qlStateQueue);
					pDirectoryRequest->base.pStateQueue = 
						&pWatchlistImpl->directory.openDirectoryRequests;

					if ((ret = wlServiceCacheGetServiceList(pWatchlistImpl->base.pServiceCache,
							&serviceList, NULL, NULL, RSSL_FALSE, pErrorInfo)) != RSSL_RET_SUCCESS)
						return ret;
				}

				if (serviceList.serviceCount)
					wlSendServiceListToRequest(&pWatchlistImpl->base, &pWatchlistImpl->directory,
							pDirectoryRequest, serviceList.serviceList, serviceList.serviceCount, 
							pErrorInfo);
				
				break;
			}

			default:
			{
				WlItemRequest *pItemRequest = (WlItemRequest*)pRequest;

				if (!(pItemRequest->flags & WL_IRQF_BATCH))
				{
					pItemRequest->base.pStateQueue = &pItemRequest->pRequestedService->recoveringList;
					rsslQueueAddLinkToBack(&pItemRequest->pRequestedService->recoveringList, 
							&pItemRequest->base.qlStateQueue);

					if ((ret = wlItemRequestFindStream(&pWatchlistImpl->base, &pWatchlistImpl->items, 
									pItemRequest, pErrorInfo, RSSL_TRUE)) != RSSL_RET_SUCCESS)
						return ret;
				}
				else
				{
					/* This is a batch request, acknowledge it. */
					RsslStatusMsg statusMsg;
					RsslWatchlistMsgEvent msgEvent;
					RsslWatchlistStreamInfo streamInfo;

					pItemRequest->base.pStateQueue = NULL;

					rsslClearStatusMsg(&statusMsg);
					statusMsg.msgBase.streamId = pItemRequest->base.streamId;
					statusMsg.msgBase.domainType = pItemRequest->base.domainType;
					statusMsg.msgBase.containerType = RSSL_DT_NO_DATA;

					statusMsg.flags = RSSL_STMF_HAS_STATE;
					rssl_set_buffer_to_string(statusMsg.state.text,
							"Batch request acknowledged.");
					statusMsg.state.streamState = RSSL_STREAM_CLOSED;
					statusMsg.state.dataState = RSSL_DATA_OK;
					statusMsg.state.code = RSSL_SC_NONE;

					wlMsgEventClear(&msgEvent);
					msgEvent.pRsslMsg = (RsslMsg*)&statusMsg;

					wlStreamInfoClear(&streamInfo);
					msgEvent.pStreamInfo = &streamInfo;
					streamInfo.pUserSpec = pItemRequest->base.pUserSpec;

					if (pItemRequest->pRequestedService->flags & WL_RSVC_HAS_NAME)
						streamInfo.pServiceName = &pItemRequest->pRequestedService->serviceName;

					wlItemRequestClose(&pWatchlistImpl->base, &pWatchlistImpl->items,
							pItemRequest);

					ret = (*pWatchlistImpl->base.config.msgCallback)
						(&pWatchlistImpl->base.watchlist, &msgEvent, pErrorInfo);

					wlRequestedServiceCheckRefCount(&pWatchlistImpl->base, pItemRequest->pRequestedService);
					wlItemRequestDestroy(&pWatchlistImpl->base, pItemRequest);

					if (ret != RSSL_RET_SUCCESS)
						return ret;
				}
				break;
			}
		}
	}

	if (!pWatchlistImpl->base.pRsslChannel)
		return RSSL_RET_SUCCESS;

	switch(pWatchlistImpl->base.pRsslChannel->state)
	{
		case RSSL_CH_STATE_INITIALIZING:
			return RSSL_RET_SUCCESS;
		case RSSL_CH_STATE_ACTIVE:
		{
			switch(pWatchlistImpl->base.channelState)
			{
				case WL_CHS_START:
				{
					RsslChannelInfo channelInfo;
					if ((ret = rsslGetChannelInfo(pWatchlistImpl->base.pRsslChannel, &channelInfo,
									&pErrorInfo->rsslError)) != RSSL_RET_SUCCESS)
					{
						rsslSetErrorInfoLocation(pErrorInfo, __FILE__, __LINE__);
						return ret;
					}
					pWatchlistImpl->base.channelMaxFragmentSize = channelInfo.maxFragmentSize;

					/* Start by logging in. */
					if (pWatchlistImpl->login.pStream)
					{
						WlLoginStream *pStream = pWatchlistImpl->login.pStream;
						assert(pWatchlistImpl->login.pRequest[pWatchlistImpl->login.index]);
						wlSetStreamMsgPending(&pWatchlistImpl->base,
								&pWatchlistImpl->login.pStream->base);
					}
					break;
				}

				case WL_CHS_LOGIN_REQUESTED:
					/* Still waiting for login response. */
					break;

				case WL_CHS_READY:
					break;

				default:
					assert(0);
			}

			break;
		}
		case RSSL_CH_STATE_CLOSED:
		{
			if (pWatchlistImpl->base.pWriteCallAgainBuffer)
			{
				RsslError releaseError;
				rsslReleaseBuffer(pWatchlistImpl->base.pWriteCallAgainBuffer, &releaseError);
				pWatchlistImpl->base.pWriteCallAgainBuffer = NULL;
			}

			pWatchlistImpl->base.pRsslChannel = NULL;

			/* Clear out the service cache. This will push all open items back into recovery. */
			return wlServiceCacheClear(pWatchlistImpl->base.pServiceCache, RSSL_TRUE, pErrorInfo);
		}
		default:
			rsslSetErrorInfo(pErrorInfo, RSSL_EIC_FAILURE, RSSL_RET_INVALID_ARGUMENT, __FILE__, __LINE__, 
					"Unhandled channel state %d", pWatchlistImpl->base.pRsslChannel->state);
			return RSSL_RET_FAILURE;
	}


	if (pWatchlistImpl->base.pWriteCallAgainBuffer)
	{
		RsslUInt32 bytes, uncompBytes;
		RsslUInt8 writeFlags = (pReactorChannelImpl->directWrite ? RSSL_WRITE_DIRECT_SOCKET_WRITE : 0);

		if ((ret = rsslWrite(pWatchlistImpl->base.pRsslChannel, pWatchlistImpl->base.pWriteCallAgainBuffer, 
						RSSL_HIGH_PRIORITY, writeFlags, &bytes, &uncompBytes, &pErrorInfo->rsslError))
				< RSSL_RET_SUCCESS)
		{
			/* Collects write statistic */
			if ( (pReactorChannelImpl->statisticFlags & RSSL_RC_ST_WRITE) && pReactorChannelImpl->pChannelStatistic)
			{
				_cumulativeValue(&pReactorChannelImpl->pChannelStatistic->bytesWritten, bytes);
				_cumulativeValue(&pReactorChannelImpl->pChannelStatistic->uncompressedBytesWritten, uncompBytes);
			}

			switch(ret)
			{
				case RSSL_RET_WRITE_CALL_AGAIN:
					pWatchlistImpl->base.watchlist.state |= RSSLWL_STF_NEED_FLUSH;
					return RSSL_RET_SUCCESS;
				default:
				{
					RsslError releaseError;
					rsslReleaseBuffer(pWatchlistImpl->base.pWriteCallAgainBuffer, &releaseError);
					pWatchlistImpl->base.pWriteCallAgainBuffer = NULL;
					return ret;
				}
			}
		}
		else
		{
			/* Collects write statistic */
			if ( (pReactorChannelImpl->statisticFlags & RSSL_RC_ST_WRITE) && pReactorChannelImpl->pChannelStatistic )
			{
				_cumulativeValue(&pReactorChannelImpl->pChannelStatistic->bytesWritten, bytes);
				_cumulativeValue(&pReactorChannelImpl->pChannelStatistic->uncompressedBytesWritten, uncompBytes);
			}

			pWatchlistImpl->base.pWriteCallAgainBuffer = NULL;
			if (ret > 0)
				pWatchlistImpl->base.watchlist.state |= RSSLWL_STF_NEED_FLUSH;
		}
	}

	pendingWaitCount = 0;

	/* Send any item requests. */
	RSSL_QUEUE_FOR_EACH_LINK(&pWatchlistImpl->base.streamsPendingRequest,
			pLink)
	{
		WlStream *pStream = RSSL_QUEUE_LINK_TO_OBJECT(WlStream,
				base.qlStreamsPendingRequest, pLink);

		if ((ret = wlStreamSubmitMsg(pWatchlistImpl, pStream, &pendingWaitCount,
						pErrorInfo)) < RSSL_RET_SUCCESS)
		{
			switch(ret)
			{
				case RSSL_RET_BUFFER_NO_BUFFERS:
					pWatchlistImpl->base.watchlist.state |= RSSLWL_STF_NEED_FLUSH;
					return RSSL_RET_SUCCESS;
				default:
					return ret;

			}
		}
	}

	return ((pWatchlistImpl->base.streamsPendingRequest.count && pWatchlistImpl->base.streamsPendingRequest.count != pendingWaitCount)
			|| pWatchlistImpl->base.newRequests.count) ? 1 : RSSL_RET_SUCCESS;
}

RsslInt64 rsslWatchlistGetNextTimeout(RsslWatchlist *pWatchlist)
{
	RsslWatchlistImpl *pWatchlistImpl = (RsslWatchlistImpl*)pWatchlist;
	RsslQueueLink *pLink;
	RsslInt64 time = WL_TIME_UNSET;
	
	if ((pLink = rsslQueuePeekFront(&pWatchlistImpl->base.streamsPendingResponse)))
	{
		WlStreamBase *pStream = RSSL_QUEUE_LINK_TO_OBJECT(WlStreamBase, qlStreamsPendingResponse, 
				pLink);
		time = pStream->requestExpireTime;
	}

	if ((pLink = rsslQueuePeekFront(&pWatchlistImpl->items.ftGroupTimerQueue)))
	{
		WlFTGroup *pGroup = RSSL_QUEUE_LINK_TO_OBJECT(WlFTGroup, qlGroups, pLink);
		if (pGroup->expireTime < time) time = pGroup->expireTime;
	}

	if ((pLink = rsslQueuePeekFront(&pWatchlistImpl->base.postTable.timeoutQueue)))
	{
		WlPostRecord *pRecord = RSSL_QUEUE_LINK_TO_OBJECT(WlPostRecord, qlTimeout, pLink);
		if (pRecord->expireTime < time) time = pRecord->expireTime;
	}

	if (pWatchlistImpl->items.gapExpireTime < time) time = pWatchlistImpl->items.gapExpireTime;

	return time;
}

RsslRet rsslWatchlistProcessTimer(RsslWatchlist *pWatchlist, RsslInt64 currentTime,
		RsslErrorInfo *pErrorInfo)
{
	RsslWatchlistImpl		*pWatchlistImpl = (RsslWatchlistImpl*)pWatchlist;
	RsslQueueLink			*pLink;
	RsslStatusMsg			statusMsg;
	RsslAckMsg				ackMsg;
	RsslWatchlistMsgEvent	msgEvent;
	RsslRet					ret;

	if (!rsslQueueGetElementCount(&pWatchlistImpl->base.streamsPendingResponse)
			&& !rsslQueueGetElementCount(&pWatchlistImpl->items.ftGroupTimerQueue)
			&& !rsslQueueGetElementCount(&pWatchlistImpl->items.gapStreamQueue)
			&& !rsslQueueGetElementCount(&pWatchlistImpl->base.postTable.timeoutQueue)
			)
		return RSSL_RET_SUCCESS;

	rsslClearStatusMsg(&statusMsg);
	statusMsg.msgBase.containerType = RSSL_DT_NO_DATA;
	statusMsg.flags = RSSL_STMF_HAS_STATE;
	statusMsg.state.streamState = RSSL_STREAM_CLOSED_RECOVER;
	statusMsg.state.dataState = RSSL_DATA_SUSPECT;
	statusMsg.state.code = RSSL_SC_TIMEOUT;

	wlMsgEventClear(&msgEvent);
	msgEvent.pRsslMsg = (RsslMsg*)&statusMsg;
	msgEvent._flags = WL_MEF_SEND_CLOSE;

	pWatchlistImpl->base.watchlist.state &= ~RSSLWL_STF_NEED_TIMER;

	/* Check stream timeouts. */
	rssl_set_buffer_to_string(statusMsg.state.text, "Request timed out.");
	statusMsg.state.code = RSSL_SC_TIMEOUT;
	while ((pLink = rsslQueuePeekFront(&pWatchlistImpl->base.streamsPendingResponse)))
	{
		WlStream *pStream = (WlStream*)RSSL_QUEUE_LINK_TO_OBJECT(WlStream,
				base.qlStreamsPendingResponse, pLink);

		statusMsg.msgBase.domainType = pStream->base.domainType;

		/* Next timeout in queue has not expired yet. */
		if (pStream->base.requestExpireTime > currentTime)
		{
			pWatchlistImpl->base.watchlist.state |= RSSLWL_STF_NEED_TIMER;
			break;
		}
		
		wlUnsetStreamMsgPending(&pWatchlistImpl->base, &pStream->base);

		switch(pStream->base.domainType)
		{
			case RSSL_DMT_LOGIN:
			{
				RsslDecodeIterator dIter;
				RsslRDMLoginMsg loginMsg;
				WlLoginProviderAction loginAction;
				RsslInt32 streamId = pWatchlistImpl->login.pStream->base.streamId;

				rsslClearDecodeIterator(&dIter);
				rsslSetDecodeIteratorRWFVersion(&dIter, RSSL_RWF_MAJOR_VERSION, 
						RSSL_RWF_MINOR_VERSION);

				if ((ret = wlLoginProcessProviderMsg(&pWatchlistImpl->login, &pWatchlistImpl->base, 
								&dIter, (RsslMsg*)&statusMsg, &loginMsg, &loginAction, pErrorInfo)) 
						!= RSSL_RET_SUCCESS)
					return ret;

				/* Should be told to recover. */
				assert(loginAction == WL_LGPA_RECOVER);

				/* Close old login stream */
				wlLoginStreamClose(&pWatchlistImpl->base, &pWatchlistImpl->login, RSSL_TRUE);

				/* Create new login stream. */
				statusMsg.state.streamState = RSSL_STREAM_OPEN;
				if (!(pWatchlistImpl->login.pStream = wlLoginStreamCreate(
								&pWatchlistImpl->base, &pWatchlistImpl->login, pErrorInfo)))
					return pErrorInfo->rsslError.rsslErrorId;
				pWatchlistImpl->login.pRequest[pWatchlistImpl->login.index]->base.pStream = &pWatchlistImpl->login.pStream->base;

				msgEvent.pRsslMsg = NULL;
				msgEvent.pRdmMsg = (RsslRDMMsg*)&loginMsg;
				loginMsg.rdmMsgBase.streamId = pWatchlistImpl->login.pRequest[pWatchlistImpl->login.index]->base.streamId;
				if ((ret = (*pWatchlistImpl->base.config.msgCallback)
							((RsslWatchlist*)&pWatchlistImpl->base.watchlist, &msgEvent, pErrorInfo)) 
						!= RSSL_RET_SUCCESS)
					return ret;

				break;
			}

			case RSSL_DMT_SOURCE:
			{
				RsslDecodeIterator dIter;
				RsslInt32 streamId = pWatchlistImpl->directory.pStream->base.streamId;

				rsslClearDecodeIterator(&dIter);
				rsslSetDecodeIteratorRWFVersion(&dIter, RSSL_RWF_MAJOR_VERSION, 
						RSSL_RWF_MINOR_VERSION);
				if ((ret = wlDirectoryProcessProviderMsgEvent(&pWatchlistImpl->base,
						&pWatchlistImpl->directory, &dIter, &msgEvent, pErrorInfo))
						!= RSSL_RET_SUCCESS)
					return ret;

				/* Close old directory stream and create new one. */
				assert (pWatchlistImpl->base.channelState == WL_CHS_LOGGED_IN);

				wlDirectoryStreamClose(&pWatchlistImpl->base, &pWatchlistImpl->directory, 
						RSSL_TRUE);

				if (!(pWatchlistImpl->directory.pStream = wlDirectoryStreamCreate(
								&pWatchlistImpl->base, &pWatchlistImpl->directory, 
								pErrorInfo)))
					return pErrorInfo->rsslError.rsslErrorId;

				pWatchlistImpl->base.channelState = WL_CHS_READY;
				break;
			}
			default:
			{
				if ((ret = wlFanoutItemMsgEvent(pWatchlistImpl, &pStream->item, &msgEvent, 
								pErrorInfo)) != RSSL_RET_SUCCESS)
					return ret;
				pWatchlistImpl->items.pCurrentFanoutStream = NULL;

				break;
			}
		}
	}

	/* Check FTGroups. */
	rssl_set_buffer_to_string(statusMsg.state.text, "Fault-tolerant Group timeout.");
	statusMsg.state.code = RSSL_SC_TIMEOUT;
	while ((pLink = rsslQueuePeekFront(&pWatchlistImpl->items.ftGroupTimerQueue)))
	{
		RsslQueueLink *pStreamLink;
		WlFTGroup *pGroup = RSSL_QUEUE_LINK_TO_OBJECT(WlFTGroup, qlGroups, pLink);

		/* Next timeout in queue has not expired yet. */
		if (pGroup->expireTime > currentTime)
		{
			pWatchlistImpl->base.watchlist.state |= RSSLWL_STF_NEED_TIMER;
			break;
		}

		pWatchlistImpl->items.pCurrentFanoutFTGroup = pGroup;

		RSSL_QUEUE_FOR_EACH_LINK(&pGroup->openStreamList, pStreamLink)
		{
			WlItemStream *pItemStream = RSSL_QUEUE_LINK_TO_OBJECT(WlItemStream, qlFTGroup, 
					pStreamLink);

			statusMsg.msgBase.domainType = pItemStream->base.domainType;
			if ((ret = wlFanoutItemMsgEvent(pWatchlistImpl, pItemStream, &msgEvent, 
							pErrorInfo)) != RSSL_RET_SUCCESS)
				return ret;
			pWatchlistImpl->items.pCurrentFanoutStream = NULL;
		}

		pWatchlistImpl->items.pCurrentFanoutFTGroup = NULL;

		if (!rsslQueueGetElementCount(&pGroup->openStreamList))
			wlFTGroupRemove(&pWatchlistImpl->items, pGroup); 

	}

	rssl_set_buffer_to_string(statusMsg.state.text, "Gap in sequence number.");
	statusMsg.state.code = RSSL_SC_GAP_DETECTED;

	/* Check gap detection timer. */
	if (pWatchlistImpl->items.gapExpireTime < currentTime)
	{
		while ((pLink = rsslQueuePeekFront(&pWatchlistImpl->items.gapStreamQueue)))
		{
			WlItemStream *pItemStream = RSSL_QUEUE_LINK_TO_OBJECT(WlItemStream, qlGap, pLink);


			if (pWatchlistImpl->base.gapRecovery)
			{
				statusMsg.msgBase.domainType = pItemStream->base.domainType;
				if ((ret = wlFanoutItemMsgEvent(pWatchlistImpl, pItemStream, &msgEvent, 
								pErrorInfo)) != RSSL_RET_SUCCESS)
					return ret;
				pWatchlistImpl->items.pCurrentFanoutStream = NULL;
			}
			else
			{
				WlBufferedMsg *pBufferedMsg;
				RsslWatchlistMsgEvent bufferedMsgEvent;

				assert(pItemStream->flags & WL_IOSF_BC_BEHIND_UC);

				/* If gap recovery is not enabled, this means that the stream was placed
				 * here due to receiving a unicast message ahead of the broadcast stream
				 * while reodering. Since the broadcast stream has not caught up yet, 
				 * assume the expected messages have been lost and forward currently 
				 * buffered messages. */

				/* Since multiple messages may be forwarded, set fanout stream
				 * so we can stop if the stream is closed (either due to a closed streamState
				 * or because the application closed it from inside the callback. */
				pWatchlistImpl->items.pCurrentFanoutStream = pItemStream;

				while ( pBufferedMsg = wlMsgReorderQueuePop(
							&pItemStream->bufferedMsgQueue))
				{
					wlMsgEventClear(&bufferedMsgEvent);
					bufferedMsgEvent.pSeqNum = &pBufferedMsg->seqNum;
					bufferedMsgEvent.pRsslMsg = wlBufferedMsgGetRsslMsg(pBufferedMsg);
					if (pBufferedMsg->flags & WL_BFMSG_HAS_FT_GROUP_ID)
						bufferedMsgEvent.pFTGroupId = &pBufferedMsg->ftGroupId;

					/* If a sequence number isn't set for the broadcast stream,
					 * update the sequence number that we have (this way, if
					 * any other refreshes come with the same number, we still
					 * let them through). */
					if (!(pItemStream->flags & WL_IOSF_HAS_BC_SEQ_NUM))
					{
						pItemStream->flags |= WL_IOSF_HAS_UC_SEQ_NUM;
						pItemStream->seqNum = pBufferedMsg->seqNum;
					}

					ret = wlFanoutItemMsgEvent(pWatchlistImpl, pItemStream, 
							&bufferedMsgEvent, 
							pErrorInfo);

					wlBufferedMsgDestroy(pBufferedMsg);
					if (ret != RSSL_RET_SUCCESS)
						return ret;

					/* If stream was closed, stop. */
					if (pWatchlistImpl->items.pCurrentFanoutStream == NULL)
						break;
				}

				/* If stream was closed, stop. */
				if (pWatchlistImpl->items.pCurrentFanoutStream == NULL)
					continue;

				/* Done forwarding messages. */
				pWatchlistImpl->items.pCurrentFanoutStream = NULL;

				wlUnsetGapTimer(pWatchlistImpl, pItemStream, WL_IOSF_BC_BEHIND_UC);
			}
		}
	}

	/* Check timeouts for outstanding post acknowledgements. */
	wlMsgEventClear(&msgEvent);
	msgEvent.pRsslMsg = (RsslMsg*)&ackMsg;

	rsslClearAckMsg(&ackMsg);
	ackMsg.msgBase.containerType = RSSL_DT_NO_DATA;
	ackMsg.nakCode = RSSL_NAKC_NO_RESPONSE;
	rssl_set_buffer_to_string(ackMsg.text, "Acknowledgement timed out.");

	while ((pLink = rsslQueuePeekFront(&pWatchlistImpl->base.postTable.timeoutQueue)))
	{
		WlPostRecord *pRecord = RSSL_QUEUE_LINK_TO_OBJECT(WlPostRecord, qlTimeout, pLink);
		WlRequest *pRequest;

		if (pRecord->expireTime > currentTime) 
		{
			pWatchlistImpl->base.watchlist.state |= RSSLWL_STF_NEED_TIMER;
			break;
		}

		pRequest = (WlRequest*)pRecord->pUserSpec;

		ackMsg.msgBase.domainType = pRequest->base.domainType;
		ackMsg.flags = RSSL_AKMF_HAS_TEXT | RSSL_AKMF_HAS_NAK_CODE;
		ackMsg.ackId = pRecord->postId;
		ackMsg.msgBase.streamId = pRequest->base.streamId;
		ackMsg.msgBase.domainType = pRecord->domainType;

		if (pRecord->flags & RSSL_PSMF_HAS_SEQ_NUM)
		{
			ackMsg.flags |= RSSL_AKMF_HAS_SEQ_NUM;
			ackMsg.seqNum = pRecord->seqNum;
		}

		if ((pRecord->msgkeyflags & RSSL_MKF_HAS_NAME) && pRecord->name.length > 0)
		{
			rsslHeapBufferCopy(&ackMsg.msgBase.msgKey.name, &pRecord->name,
				&ackMsg.msgBase.msgKey.name);
			ackMsg.flags |= RSSL_AKMF_HAS_MSG_KEY;
			ackMsg.msgBase.msgKey.flags |= RSSL_MKF_HAS_NAME;
		}
		if (pRecord->msgkeyflags & RSSL_MKF_HAS_SERVICE_ID)
		{
			ackMsg.msgBase.msgKey.serviceId = pRecord->serviceId;
			ackMsg.flags |= RSSL_AKMF_HAS_MSG_KEY;
			ackMsg.msgBase.msgKey.flags |= RSSL_MKF_HAS_SERVICE_ID;
		}


		rsslQueueRemoveLink(&pRequest->base.openPosts, &pRecord->qlUser);
		wlPostTableRemoveRecord(&pWatchlistImpl->base.postTable, pRecord);

		if (pRequest->base.domainType == RSSL_DMT_LOGIN)
		{
			/* Off-stream post */
			RsslWatchlistStreamInfo streamInfo;
			wlStreamInfoClear(&streamInfo);
			streamInfo.pUserSpec = pRequest->base.pUserSpec;
			msgEvent.pStreamInfo = &streamInfo;

			if ((ret = (*pWatchlistImpl->base.config.msgCallback)
						((RsslWatchlist*)&pWatchlistImpl->base.watchlist, &msgEvent, pErrorInfo)) 
					!= RSSL_RET_SUCCESS)
				return ret;
		}
		else
		{
			/* Onstream post */
			if ((ret = wlSendMsgEventToItemRequest(pWatchlistImpl, &msgEvent, (WlItemRequest*)pRequest, pErrorInfo))
				!= RSSL_RET_SUCCESS)
				return ret;
		}

	}

	/* Update gap timer. */
	if ((pLink = rsslQueuePeekFront(&pWatchlistImpl->items.gapStreamQueue)))
	{
		pWatchlistImpl->items.gapExpireTime = pWatchlistImpl->base.currentTime + 
			pWatchlistImpl->base.gapTimeout;
		pWatchlistImpl->base.watchlist.state |= RSSLWL_STF_NEED_TIMER;
	}
	else
		pWatchlistImpl->items.gapExpireTime = WL_TIME_UNSET;

	return RSSL_RET_SUCCESS;
}

void rsslWatchlistResetGapTimer(RsslWatchlist *pWatchlist)
{
	RsslWatchlistImpl *pWatchlistImpl = (RsslWatchlistImpl*)pWatchlist;

	if (pWatchlistImpl->items.gapExpireTime == WL_TIME_UNSET)
		return;

	pWatchlistImpl->items.gapExpireTime = pWatchlistImpl->base.currentTime + 
		pWatchlistImpl->base.gapTimeout;
	pWatchlistImpl->base.watchlist.state |= RSSLWL_STF_NEED_TIMER;
}

/*** Supporting implementation functions. ***/

static RsslRet wlServiceUpdateCallback(WlServiceCache *pServiceCache,
		WlServiceCacheUpdateEvent *pEvent, RsslErrorInfo *pErrorInfo)
{
	RsslQueueLink *pLink;
	RsslWatchlistImpl *pWatchlistImpl = (RsslWatchlistImpl*)pServiceCache->pUserSpec;
	RsslRet ret;

	RSSL_QUEUE_FOR_EACH_LINK(&pEvent->updatedServiceList, pLink)
	{
		RDMCachedService *pService = RSSL_QUEUE_LINK_TO_OBJECT(RDMCachedService, _updatedServiceLink,
				pLink);
		WlService *pWlService;
		RsslHashLink *pHashLink;
		RsslState *pServiceState = NULL;

		/* Process action (service cache will produce the appropriate action
		 * based on whether the service was added, updated, or deleted). */
		switch(pService->rdm.action)
		{
			case RSSL_MPEA_ADD_ENTRY: /* New service */

				/* Create storage for service. */

				assert(!pService->pUserSpec);
				if (!(pWlService = wlServiceCreate(pService, pErrorInfo)))
					return pErrorInfo->rsslError.rsslErrorId;

				rsslQueueAddLinkToBack(&pWatchlistImpl->services, &pWlService->qlServices);

			case RSSL_MPEA_UPDATE_ENTRY:
			{
				RsslUInt32 groupIndex;

				assert(pService->pUserSpec);
				pWlService = (WlService*)pService->pUserSpec;

				/*** First, check for any changes that may impact existing streams. ***/

				/* Check if service state is available for processing. */
				if (pService->updateFlags & RDM_SVCF_HAS_STATE
						&& pService->stateUpdateFlags & RDM_SVC_STF_HAS_STATUS)
				{
					RsslQueueLink *pSvcLink;
					RsslStatusMsg statusMsg;
					RsslWatchlistMsgEvent msgEvent;

					assert(pService->rdm.state.flags & RDM_SVC_STF_HAS_STATUS);

					RSSL_QUEUE_FOR_EACH_LINK(&pWlService->openStreamList, pSvcLink)
					{
						WlItemStream *pStream = RSSL_QUEUE_LINK_TO_OBJECT(WlItemStream, 
								qlServiceStreams, pSvcLink);

						rsslClearStatusMsg(&statusMsg);
						statusMsg.msgBase.domainType = pStream->streamAttributes.domainType;
						statusMsg.msgBase.containerType = RSSL_DT_NO_DATA;
						statusMsg.flags = RSSL_STMF_HAS_STATE;
						statusMsg.state = pService->rdm.state.status;

						wlMsgEventClear(&msgEvent);
						msgEvent.pRsslMsg = (RsslMsg*)&statusMsg;

						if ((ret = wlFanoutItemMsgEvent(pWatchlistImpl, pStream, &msgEvent, 
										pErrorInfo)) != RSSL_RET_SUCCESS)
							return ret;

						pWatchlistImpl->items.pCurrentFanoutStream = NULL;
					}
				}

				for (groupIndex = 0; groupIndex < pService->rdm.groupStateCount; ++groupIndex)
				{
					RsslRDMServiceGroupState *pGroupState = 
						&pService->rdm.groupStateList[groupIndex];

					WlItemGroup *pItemGroup;
					RsslHashLink *pHashLink;

					pHashLink = rsslHashTableFind(&pWlService->itemGroupTable, &pGroupState->group,
							NULL);

					if (!pHashLink)
						continue; /* No items open that are associated with this group. */

					pItemGroup = RSSL_HASH_LINK_TO_OBJECT(WlItemGroup, hlItemGroupTable, pHashLink);

					if (pGroupState->flags & RDM_SVC_GRF_HAS_MERGED_TO_GROUP)
						wlItemGroupMerge(pItemGroup, pWlService, &pGroupState->mergedToGroup);

					if (pGroupState->flags & RDM_SVC_GRF_HAS_STATUS)
					{
						/* Fanout status. */
						RsslQueueLink *pLink;
						RsslStatusMsg statusMsg;
						RsslWatchlistMsgEvent msgEvent;

						pWatchlistImpl->items.pCurrentFanoutGroup = pItemGroup;
						RSSL_QUEUE_FOR_EACH_LINK(&pItemGroup->openStreamList, pLink)
						{
							WlItemStream *pStream = RSSL_QUEUE_LINK_TO_OBJECT(WlItemStream,
									qlItemGroup, pLink);

							rsslClearStatusMsg(&statusMsg);
							statusMsg.msgBase.domainType = pStream->streamAttributes.domainType;
							statusMsg.msgBase.containerType = RSSL_DT_NO_DATA;
							statusMsg.flags = RSSL_STMF_HAS_STATE;
							statusMsg.state = pGroupState->status;

							wlMsgEventClear(&msgEvent);
							msgEvent.pRsslMsg = (RsslMsg*)&statusMsg;

							if ((ret = wlFanoutItemMsgEvent(pWatchlistImpl, pStream, &msgEvent, 
											pErrorInfo)) != RSSL_RET_SUCCESS)
								return ret;
							pWatchlistImpl->items.pCurrentFanoutStream = NULL;

						}

						pWatchlistImpl->items.pCurrentFanoutGroup = NULL;
						if (pItemGroup->openStreamList.count == 0)
							wlItemGroupRemove(pItemGroup);

					}


				}

				/* Check if service name has changed. If so, requests using the old name 
				 * no longer have a matching service. */
				if (pService->updatedServiceName)
				{
					if ((pHashLink = rsslHashTableFind(&pWatchlistImpl->base.requestedSvcByName, 
									&pService->oldServiceName, NULL)))
					{
						WlRequestedService *pRequestedService = 
							RSSL_HASH_LINK_TO_OBJECT(WlRequestedService, hlServiceRequests,
									pHashLink);

						rsslQueueRemoveLink(&pRequestedService->pMatchingService->requestedServices,
								&pRequestedService->qlDirectoryRequests);
						pRequestedService->pMatchingService = NULL;
					}
				}

				/*** Now, check if there are requests waiting for an update to this
				 * service and try to recover them. ***/

				if ((pHashLink = rsslHashTableFind(&pWatchlistImpl->base.requestedSvcByName, 
								&pService->rdm.info.serviceName, NULL)))
				{
					RsslQueueLink *pRequestLink;
					WlRequestedService *pRequestedService = 
						RSSL_HASH_LINK_TO_OBJECT(WlRequestedService, hlServiceRequests,
								pHashLink);

					if (!pRequestedService->pMatchingService)
					{
						pRequestedService->pMatchingService = pWlService;
						rsslQueueAddLinkToBack(&pWlService->requestedServices,
								&pRequestedService->qlDirectoryRequests);
					}

					RSSL_QUEUE_FOR_EACH_LINK(&pRequestedService->recoveringList, 
							pRequestLink)
					{
						WlItemRequest *pItemRequest = RSSL_QUEUE_LINK_TO_OBJECT(WlItemRequest,
								base.qlStateQueue, pRequestLink);

						if ((ret = wlItemRequestFindStream(&pWatchlistImpl->base, &pWatchlistImpl->items, 
										pItemRequest, pErrorInfo, RSSL_TRUE)) != RSSL_RET_SUCCESS)
							return ret;
					}

				}

				if ((pHashLink = rsslHashTableFind(&pWatchlistImpl->base.requestedSvcById, 
								&pService->rdm.serviceId, NULL)))
				{
					RsslQueueLink *pRequestLink;
					WlRequestedService *pRequestedService = 
						RSSL_HASH_LINK_TO_OBJECT(WlRequestedService, hlServiceRequests,
								pHashLink);

					if (!pRequestedService->pMatchingService)
					{
						pRequestedService->pMatchingService = pWlService;
						rsslQueueAddLinkToBack(&pWlService->requestedServices,
								&pRequestedService->qlDirectoryRequests);
					}

					RSSL_QUEUE_FOR_EACH_LINK(&pRequestedService->recoveringList, 
							pRequestLink)
					{
						WlItemRequest *pItemRequest = RSSL_QUEUE_LINK_TO_OBJECT(WlItemRequest,
								base.qlStateQueue, pRequestLink);

						if ((ret = wlItemRequestFindStream(&pWatchlistImpl->base, &pWatchlistImpl->items, 
										pItemRequest, pErrorInfo, RSSL_TRUE)) != RSSL_RET_SUCCESS)
							return ret;
					}
				}
				break;
			}

			case RSSL_MPEA_DELETE_ENTRY:
			{
				
				/* Separate requested services from this service now, so that they
				 * don't attempt to recover to it. */
				RsslQueueLink *pRequestedServiceLink;

				pWlService = (WlService*)pService->pUserSpec;
				
				RSSL_QUEUE_FOR_EACH_LINK(&pWlService->requestedServices, pRequestedServiceLink)
				{
					WlRequestedService *pRequestedService = 
						RSSL_QUEUE_LINK_TO_OBJECT(WlRequestedService, qlDirectoryRequests,
								pRequestedServiceLink);
					rsslQueueRemoveLink(&pRequestedService->pMatchingService->requestedServices,
							&pRequestedService->qlDirectoryRequests);
					pRequestedService->pMatchingService = NULL;
				}
			}


			default:
				break;
		}
	}

	if ((ret = wlFanoutDirectoryMsg(&pWatchlistImpl->base, &pWatchlistImpl->directory,
		   &pEvent->updatedServiceList, pErrorInfo)) != RSSL_RET_SUCCESS)
		return ret;

	RSSL_QUEUE_FOR_EACH_LINK(&pEvent->updatedServiceList, pLink)
	{
		RDMCachedService *pService = RSSL_QUEUE_LINK_TO_OBJECT(RDMCachedService, _updatedServiceLink,
				pLink);
		if (pService->rdm.action == RSSL_MPEA_DELETE_ENTRY)
		{
			/* Service was deleted. All items should be considered ClosedRecover. */

			WlService *pWlService = (WlService*)pService->pUserSpec;
			if ((ret = wlProcessRemovedService(pWatchlistImpl, pWlService, pErrorInfo))
					!= RSSL_RET_SUCCESS)
				return ret;

			rsslQueueRemoveLink(&pWatchlistImpl->services, &pWlService->qlServices);
			wlServiceDestroy(pWlService);
		}
	}
	return RSSL_RET_SUCCESS;
}

static RsslRet wlServiceStateChangeCallback(WlServiceCache *pServiceCache,
	RDMCachedService *pCachedService, RsslErrorInfo *pErrorInfo)
{
	RsslWatchlist				*pWatchlist = (RsslWatchlist*)pServiceCache->pUserSpec;
	RsslReactorChannelImpl *pReactorChannelImpl = (RsslReactorChannelImpl *)pWatchlist->pUserSpec;
	RsslReactorWarmStandByHandlerImpl *pReactorWarmStandByHandlerImpl = pReactorChannelImpl->pWarmStandByHandlerImpl;
	RsslReactorWarmStandbyGroupImpl *pReactorWarmStandByGroupImpl = &pReactorWarmStandByHandlerImpl->warmStandbyGroupList[pReactorWarmStandByHandlerImpl->currentWSyGroupIndex];
	RsslHashLink *pHashLink = NULL;
	RsslWatchlistImpl			*pWatchlistImpl = (RsslWatchlistImpl*)pWatchlist;
	RsslReactorImpl *pReactorImpl = pReactorChannelImpl->pParentReactor;
	RsslErrorInfo errorInfo;

	if (_reactorHandlesWarmStandby(pReactorChannelImpl))
	{
		if (pReactorWarmStandByGroupImpl->warmStandbyMode == RSSL_RWSB_MODE_SERVICE_BASED)
		{
			pHashLink = rsslHashTableFind(&pReactorWarmStandByGroupImpl->_perServiceById, &pCachedService->rdm.serviceId, NULL);

			if (pHashLink)
			{	
				RsslReactorWarmStandbyServiceImpl* pReactorWarmStandbyServiceImpl = RSSL_HASH_LINK_TO_OBJECT(RsslReactorWarmStandbyServiceImpl, hashLink, pHashLink);
				
				/* The active service's state is changed from up to down */
				if (pCachedService->rdm.state.serviceState == 0 && pReactorWarmStandbyServiceImpl->pReactorChannel == (RsslReactorChannel*)pReactorChannelImpl)
				{
					/* Submit an event to change the current active service to standby and promote the service from others channel. */
					RsslReactorWarmStanbyEvent* pEvent = (RsslReactorWarmStanbyEvent*)rsslReactorEventQueueGetFromPool(&pReactorImpl->reactorEventQueue);
					rsslClearReactorWarmStanbyEvent(pEvent);

					pEvent->reactorWarmStandByEventType = RSSL_RCIMPL_WSBET_CHANGE_ACTIVE_TO_STANDBY_PER_SERVICE;
					pEvent->pReactorChannel = (RsslReactorChannel*)pReactorChannelImpl;
					pEvent->serviceID = pCachedService->rdm.serviceId;
					pEvent->streamID = pWatchlistImpl->directory.pStream->base.streamId;
					pEvent->pHashLink = pHashLink;

					if (!RSSL_ERROR_INFO_CHECK(rsslReactorEventQueuePut(&pReactorImpl->reactorEventQueue, (RsslReactorEventImpl*)pEvent) == RSSL_RET_SUCCESS, RSSL_RET_FAILURE, &errorInfo))
						return RSSL_RET_FAILURE;
				}
				else
				{
					/* Service state changed from down to up*/

					/* Checks whether there is a channel provides the service as active. */
					if (pCachedService->rdm.state.serviceState == 1 && pReactorWarmStandbyServiceImpl->pReactorChannel == NULL)
					{
						RsslReactorWarmStanbyEvent* pEvent = (RsslReactorWarmStanbyEvent*)rsslReactorEventQueueGetFromPool(&pReactorImpl->reactorEventQueue);
						rsslClearReactorWarmStanbyEvent(pEvent);

						pEvent->reactorWarmStandByEventType = RSSL_RCIMPL_WSBET_CHANGE_STANDBY_TO_ACTIVE_PER_SERVICE;
						pEvent->pReactorChannel = (RsslReactorChannel*)pReactorChannelImpl;
						pEvent->serviceID = pCachedService->rdm.serviceId;
						pEvent->streamID = pWatchlistImpl->directory.pStream->base.streamId;
						pEvent->pHashLink = pHashLink;

						if (!RSSL_ERROR_INFO_CHECK(rsslReactorEventQueuePut(&pReactorImpl->reactorEventQueue, (RsslReactorEventImpl*)pEvent) == RSSL_RET_SUCCESS, RSSL_RET_FAILURE, &errorInfo))
							return RSSL_RET_FAILURE;
					}
				}
			}
			else
			{
				/* Do nothing as this serivce will be added and send a warm standby status when updating source directory for a new service. */
			}
		}
		else if(pReactorWarmStandByGroupImpl->warmStandbyMode == RSSL_RWSB_MODE_LOGIN_BASED)
		{
			/* Service is changed to Down to Up. for the active server. */
			if (pReactorChannelImpl->isActiveServer == RSSL_TRUE)
			{
				if (pCachedService->rdm.state.serviceState == 1)
				{
					/* Submit an event to change the current active service to standby and promote the service from others channel. */
					RsslReactorWarmStanbyEvent* pEvent = (RsslReactorWarmStanbyEvent*)rsslReactorEventQueueGetFromPool(&pReactorImpl->reactorEventQueue);
					rsslClearReactorWarmStanbyEvent(pEvent);

					pEvent->reactorWarmStandByEventType = RSSL_RCIMPL_WSBET_ACTIVE_SERVER_SERVICE_STATE_FROM_DOWN_TO_UP;
					pEvent->pReactorChannel = (RsslReactorChannel*)pReactorChannelImpl;
					pEvent->serviceID = pCachedService->rdm.serviceId;
					pEvent->pHashLink = pHashLink;

					if (!RSSL_ERROR_INFO_CHECK(rsslReactorEventQueuePut(&pReactorImpl->reactorEventQueue, (RsslReactorEventImpl*)pEvent) == RSSL_RET_SUCCESS, RSSL_RET_FAILURE, &errorInfo))
						return RSSL_RET_FAILURE;
				}
			}
		}
	}

	return RSSL_RET_SUCCESS;
}

static RsslUInt aggregateServiceStateForWSB(RsslReactorChannelImpl* pUpdatedReactorChannel, RsslUInt serviceId, RsslRDMServiceState *pUpdateRDMServiceState)
{
	RsslReactorWarmStandByHandlerImpl *pReactorWarmStandByHandlerImpl = pUpdatedReactorChannel->pWarmStandByHandlerImpl;
	RsslQueueLink *pLink = NULL;
	RsslHashLink *pHashLink = NULL;
	RsslReactorChannelImpl *pProcessReactorChannel;
	RsslUInt32 serviceState = 0;
	RsslWatchlistImpl* pWatchlist = NULL;
	WlServiceCache* pServiceCache = NULL;
	RDMCachedService *pService = NULL;

	if (pReactorWarmStandByHandlerImpl)
	{
		RSSL_MUTEX_LOCK(&pReactorWarmStandByHandlerImpl->warmStandByHandlerMutex);
		RSSL_QUEUE_FOR_EACH_LINK(&pReactorWarmStandByHandlerImpl->rsslChannelQueue, pLink)
		{
			pProcessReactorChannel = RSSL_QUEUE_LINK_TO_OBJECT(RsslReactorChannelImpl, warmstandbyChannelLink, pLink);

			if (pProcessReactorChannel != pUpdatedReactorChannel)
			{
				if (pProcessReactorChannel->reactorParentQueue == &pProcessReactorChannel->pParentReactor->closingChannels)
				{
					continue; /* Countinue if this channel is being closed. */
				}

				pWatchlist = (RsslWatchlistImpl*)pProcessReactorChannel->pWatchlist;
				pServiceCache = pWatchlist->base.pServiceCache;

				if (pServiceCache)
				{
					pHashLink = rsslHashTableFind(&pServiceCache->_servicesById, &serviceId, NULL);

					if (pHashLink)
					{
						pService = RSSL_HASH_LINK_TO_OBJECT(RDMCachedService, _idLink, pHashLink);

						serviceState |= pService->rdm.state.serviceState;
					}
				}
			}
		}
		RSSL_MUTEX_UNLOCK(&pReactorWarmStandByHandlerImpl->warmStandByHandlerMutex);

		serviceState |= pUpdateRDMServiceState->serviceState;
	}

	return serviceState;
}

static RsslRet rsslCopyRDMServiceInfo(RsslReactorWarmStandbyServiceImpl *pReactorWarmStandbyServiceImpl, RsslRDMServiceInfo *pRDMServiceInfo)
{
	RsslBuffer memoryBuffer = RSSL_INIT_BUFFER;
	RsslRDMServiceInfo *pNewInfo = &pReactorWarmStandbyServiceImpl->rdmServiceInfo;
	RsslUInt32 j = 0;

	*pNewInfo = *pRDMServiceInfo;

	if (pReactorWarmStandbyServiceImpl->directoryMemBuffer.data == 0 && pReactorWarmStandbyServiceImpl->directoryMemBuffer.length == 0)
	{
		memoryBuffer.data = pReactorWarmStandbyServiceImpl->pMemoryLocation;
		memoryBuffer.length = pReactorWarmStandbyServiceImpl->allocationLength;
	}
	else
	{
		memoryBuffer = pReactorWarmStandbyServiceImpl->directoryMemBuffer;
	}

	if (!rsslCopyBufferMemory(&pNewInfo->serviceName, &pRDMServiceInfo->serviceName, &memoryBuffer))
		return RSSL_RET_BUFFER_TOO_SMALL;

	if (pRDMServiceInfo->flags & RDM_SVC_IFF_HAS_VENDOR)
	{
		if (!rsslCopyBufferMemory(&pNewInfo->vendor, &pRDMServiceInfo->vendor, &memoryBuffer))
			return RSSL_RET_BUFFER_TOO_SMALL;
	}

	/* Capabilities */
	pNewInfo->capabilitiesList = (RsslUInt*)rsslReserveAlignedBufferMemory(&memoryBuffer, pRDMServiceInfo->capabilitiesCount, sizeof(RsslUInt));
	if (!pNewInfo->capabilitiesList) return RSSL_RET_BUFFER_TOO_SMALL;
	memcpy(pNewInfo->capabilitiesList, pRDMServiceInfo->capabilitiesList, pRDMServiceInfo->capabilitiesCount * sizeof(RsslUInt));

	/* Dictionaries Provided */
	if (pRDMServiceInfo->flags & RDM_SVC_IFF_HAS_DICTS_PROVIDED)
	{
		pNewInfo->dictionariesProvidedList = (RsslBuffer*)rsslReserveAlignedBufferMemory(&memoryBuffer, pRDMServiceInfo->dictionariesProvidedCount, sizeof(RsslBuffer));
		if (!pNewInfo->dictionariesProvidedList) return RSSL_RET_BUFFER_TOO_SMALL;

		for (j = 0; j < pRDMServiceInfo->dictionariesProvidedCount; ++j)
			if (!rsslCopyBufferMemory(&pNewInfo->dictionariesProvidedList[j], &pRDMServiceInfo->dictionariesProvidedList[j], &memoryBuffer))
				return RSSL_RET_BUFFER_TOO_SMALL;
	}

	/* Dictionaries Used */
	if (pRDMServiceInfo->flags & RDM_SVC_IFF_HAS_DICTS_USED)
	{
		pNewInfo->dictionariesUsedList = (RsslBuffer*)rsslReserveAlignedBufferMemory(&memoryBuffer, pRDMServiceInfo->dictionariesUsedCount, sizeof(RsslBuffer));
		if (!pNewInfo->dictionariesUsedList) return RSSL_RET_BUFFER_TOO_SMALL;

		for (j = 0; j < pRDMServiceInfo->dictionariesUsedCount; ++j)
			if (!rsslCopyBufferMemory(&pNewInfo->dictionariesUsedList[j], &pRDMServiceInfo->dictionariesUsedList[j], &memoryBuffer))
				return RSSL_RET_BUFFER_TOO_SMALL;
	}

	/* Qualities of Service */
	if (pRDMServiceInfo->flags & RDM_SVC_IFF_HAS_QOS)
	{
		pNewInfo->qosList = (RsslQos*)rsslReserveAlignedBufferMemory(&memoryBuffer, pRDMServiceInfo->qosCount, sizeof(RsslQos));
		if (!pNewInfo->qosList) return RSSL_RET_BUFFER_TOO_SMALL;
		memcpy(pNewInfo->qosList, pRDMServiceInfo->qosList, pRDMServiceInfo->qosCount * sizeof(RsslQos));
	}

	if (!rsslCopyBufferMemory(&pNewInfo->itemList, &pRDMServiceInfo->itemList, &memoryBuffer))
		return RSSL_RET_BUFFER_TOO_SMALL;

	pReactorWarmStandbyServiceImpl->directoryMemBuffer = memoryBuffer;

	return RSSL_RET_SUCCESS;
}

static RsslRet rsslCopyRDMServiceState(RsslReactorWarmStandbyServiceImpl *pReactorWarmStandbyServiceImpl, RsslRDMServiceState *pRDMServiceState)
{
	RsslBuffer memoryBuffer = RSSL_INIT_BUFFER;
	RsslRDMServiceState *pNewServiceState = &pReactorWarmStandbyServiceImpl->rdmServiceState;

	if (pReactorWarmStandbyServiceImpl->directoryMemBuffer.data == 0 && pReactorWarmStandbyServiceImpl->directoryMemBuffer.length == 0)
	{
		memoryBuffer.data = pReactorWarmStandbyServiceImpl->pMemoryLocation;
		memoryBuffer.length = pReactorWarmStandbyServiceImpl->allocationLength;
	}
	else
	{
		memoryBuffer = pReactorWarmStandbyServiceImpl->directoryMemBuffer;
	}

	*pNewServiceState = *pRDMServiceState;

	if (pRDMServiceState->flags & RDM_SVC_GRF_HAS_STATUS)
	{
		if (!rsslCopyBufferMemory(&pNewServiceState->status.text, &pRDMServiceState->status.text, &memoryBuffer))
			return RSSL_RET_BUFFER_TOO_SMALL;
	}

	pReactorWarmStandbyServiceImpl->directoryMemBuffer = memoryBuffer;

	return RSSL_RET_SUCCESS;
}

static RsslRet wlWarmStandbyServiceUpdate(RsslReactorWarmStandbyGroupImpl *pReactorWarmStandByGroupImpl, RsslReactorChannelImpl *pReactorChannelImpl, WlServiceCache *pServiceCache, WlServiceCacheUpdateEvent *pEvent,
	RsslBool* pAddNewService, RsslErrorInfo *pError)
{
	RsslQueueLink* pLink = NULL;
	RsslHashLink* pHashLink = NULL;
	RDMCachedService* pService = NULL;
	RsslReactorWarmStandbyServiceImpl *pReactorWarmStandbyServiceImpl = NULL;
	RsslRet ret = RSSL_RET_SUCCESS;
	*pAddNewService = RSSL_FALSE;

	RSSL_QUEUE_FOR_EACH_LINK(&pServiceCache->_serviceList, pLink)
	{
		pService = RSSL_QUEUE_LINK_TO_OBJECT(RDMCachedService, _fullListLink, pLink);

		if (pService)
		{
			pHashLink = rsslHashTableFind(&pReactorWarmStandByGroupImpl->_perServiceById, &pService->rdm.serviceId, NULL);

			if (pHashLink)
			{
				RsslUInt currentServiceState = 0;
				RsslBool addToUpdateList = RSSL_FALSE;
				pReactorWarmStandbyServiceImpl = RSSL_HASH_LINK_TO_OBJECT(RsslReactorWarmStandbyServiceImpl, hashLink, pHashLink);

				/* Checks whether the service state of the entire source directry of a warm stanby channel need to change.*/
				if (pReactorWarmStandbyServiceImpl->rdmServiceState.serviceState != pService->rdm.state.serviceState)
				{
					currentServiceState = pReactorWarmStandbyServiceImpl->rdmServiceState.serviceState;

					pReactorWarmStandbyServiceImpl->rdmServiceState.serviceState = aggregateServiceStateForWSB(pReactorChannelImpl, pService->rdm.serviceId, &pService->rdm.state);

					if (currentServiceState != pReactorWarmStandbyServiceImpl->rdmServiceState.serviceState)
					{
						pReactorWarmStandbyServiceImpl->updateServiceFilter |= RDM_SVCF_HAS_STATE;
						pReactorWarmStandbyServiceImpl->serviceAction = RSSL_MPEA_UPDATE_ENTRY;
						addToUpdateList = RSSL_TRUE;
					}
				}

				/* Handles the delete action to remove the service */
				if (pService->rdm.action == RSSL_MPEA_DELETE_ENTRY)
				{
					RsslReactorChannelImpl *pProcessReactorChannel = NULL;
					RsslQueueLink *pLink;
					RsslBool hasService = RSSL_FALSE;
					RsslWatchlistImpl *pWatchlistImpl = NULL;
					RsslReactorWarmStandByHandlerImpl *pReactorWarmStandByHandlerImpl = pReactorChannelImpl->pWarmStandByHandlerImpl;
					RDMCachedService *pOtherCachedService = NULL;

					/* Checks whether others channels provide this service as well. */
					RSSL_MUTEX_LOCK(&pReactorWarmStandByHandlerImpl->warmStandByHandlerMutex);
					RSSL_QUEUE_FOR_EACH_LINK(&pReactorWarmStandByHandlerImpl->rsslChannelQueue, pLink)
					{
						pProcessReactorChannel = RSSL_QUEUE_LINK_TO_OBJECT(RsslReactorChannelImpl, warmstandbyChannelLink, pLink);

						if (pProcessReactorChannel != pReactorChannelImpl)
						{
							if (pProcessReactorChannel->reactorParentQueue == &pProcessReactorChannel->pParentReactor->closingChannels)
							{
								continue; /* Countinue if this channel is being closed. */
							}

							pWatchlistImpl = (RsslWatchlistImpl*)pProcessReactorChannel->pWatchlist;

							pHashLink = rsslHashTableFind(&pWatchlistImpl->base.pServiceCache->_servicesById, &pService->rdm.serviceId, NULL);
							if (pHashLink)
							{
								pOtherCachedService = RSSL_HASH_LINK_TO_OBJECT(RDMCachedService, _idLink, pHashLink);

								if (pOtherCachedService->rdm.action != RSSL_MPEA_DELETE_ENTRY)
								{
									hasService = RSSL_TRUE;
									break;
								}
							}
						}
					}
					RSSL_MUTEX_UNLOCK(&pReactorWarmStandByHandlerImpl->warmStandByHandlerMutex);

					if (hasService == RSSL_FALSE)
					{
						pReactorWarmStandbyServiceImpl->serviceAction = RSSL_MPEA_DELETE_ENTRY;

						addToUpdateList = RSSL_TRUE;
					}
				}
				/* Checks whether the action is changed */
				else if (pReactorWarmStandbyServiceImpl->serviceAction == RSSL_MPEA_DELETE_ENTRY && pService->rdm.action == RSSL_MPEA_ADD_ENTRY)
				{
					pReactorWarmStandbyServiceImpl->serviceAction = RSSL_MPEA_ADD_ENTRY;
					pReactorWarmStandbyServiceImpl->updateServiceFilter = (RDM_SVCF_HAS_INFO| RDM_SVCF_HAS_STATE);
					addToUpdateList = RSSL_TRUE;
				}

				/* Checks with this flag to ensure that a service is added only once.*/
				if (addToUpdateList)
				{
					rsslQueueAddLinkToBack(&pReactorWarmStandByGroupImpl->_updateServiceList, &pReactorWarmStandbyServiceImpl->updateQLink);
				}

				continue; /* Ths service is already added. */
			}
			else
			{
				/* Don't add this service as it is removed. */
				if (pService->rdm.action == RSSL_MPEA_DELETE_ENTRY)
				{
					continue;
				}

				pReactorWarmStandbyServiceImpl = (RsslReactorWarmStandbyServiceImpl*)malloc(sizeof(RsslReactorWarmStandbyServiceImpl));

				if (pReactorWarmStandbyServiceImpl == NULL)
				{
					goto allocationFailed;
				}

				rsslClearReactorWarmStandbyServiceImpl(pReactorWarmStandbyServiceImpl);

				pReactorWarmStandbyServiceImpl->pMemoryLocation = (char*)malloc(RSSL_REACTOR_RDM_SERVICE_INFO_INIT_BUFFER_SIZE);

				if (pReactorWarmStandbyServiceImpl->pMemoryLocation == NULL)
				{
					goto allocationFailed;
				}

				pReactorWarmStandbyServiceImpl->allocationLength = RSSL_REACTOR_RDM_SERVICE_INFO_INIT_BUFFER_SIZE;

				ret = rsslCopyRDMServiceInfo(pReactorWarmStandbyServiceImpl, &pService->rdm.info);

				while (ret == RSSL_RET_BUFFER_TOO_SMALL)
				{
					RsslUInt32 newBufferSize = pReactorWarmStandbyServiceImpl->allocationLength * 2;
					free(pReactorWarmStandbyServiceImpl->pMemoryLocation);

					pReactorWarmStandbyServiceImpl->pMemoryLocation = (char*)malloc(newBufferSize);

					if (pReactorWarmStandbyServiceImpl->pMemoryLocation == NULL)
					{
						goto allocationFailed;
					}

					pReactorWarmStandbyServiceImpl->allocationLength = newBufferSize;

					rsslClearBuffer(&pReactorWarmStandbyServiceImpl->directoryMemBuffer);

					ret = rsslCopyRDMServiceInfo(pReactorWarmStandbyServiceImpl, &pService->rdm.info);
				}

				pReactorWarmStandbyServiceImpl->updateServiceFilter |= RDM_SVCF_HAS_INFO;

				ret = rsslCopyRDMServiceState(pReactorWarmStandbyServiceImpl, &pService->rdm.state);

				while (ret == RSSL_RET_BUFFER_TOO_SMALL)
				{
					RsslUInt32 newBufferSize = pReactorWarmStandbyServiceImpl->allocationLength * 2;
					char* pTempMemAlloc = NULL;
					RsslUInt32 previousDataSize = pReactorWarmStandbyServiceImpl->allocationLength - pReactorWarmStandbyServiceImpl->directoryMemBuffer.length;

					pTempMemAlloc = (char*)malloc(newBufferSize);
					if (pTempMemAlloc == NULL)
					{
						goto allocationFailed;
					}

					memset(pTempMemAlloc, 0, newBufferSize);
					memcpy(pTempMemAlloc, pReactorWarmStandbyServiceImpl->pMemoryLocation, previousDataSize);

					free(pReactorWarmStandbyServiceImpl->pMemoryLocation);

					pReactorWarmStandbyServiceImpl->pMemoryLocation = pTempMemAlloc;
					pReactorWarmStandbyServiceImpl->allocationLength = newBufferSize;
					pReactorWarmStandbyServiceImpl->directoryMemBuffer.data = pReactorWarmStandbyServiceImpl->pMemoryLocation;
					pReactorWarmStandbyServiceImpl->directoryMemBuffer.data += previousDataSize;
					pReactorWarmStandbyServiceImpl->directoryMemBuffer.length = (pReactorWarmStandbyServiceImpl->allocationLength - previousDataSize);

					ret = rsslCopyRDMServiceState(pReactorWarmStandbyServiceImpl, &pService->rdm.state);
				}

				pReactorWarmStandbyServiceImpl->updateServiceFilter |= RDM_SVCF_HAS_STATE;
				pReactorWarmStandbyServiceImpl->serviceAction = RSSL_MPEA_ADD_ENTRY;

				pReactorWarmStandbyServiceImpl->serviceID = pService->rdm.serviceId;

				/* keeps in the queue list and hash table of the warm standby group. */
				rsslQueueAddLinkToBack(&pReactorWarmStandByGroupImpl->_updateServiceList, &pReactorWarmStandbyServiceImpl->updateQLink);
				rsslQueueAddLinkToBack(&pReactorWarmStandByGroupImpl->_serviceList, &pReactorWarmStandbyServiceImpl->queueLink);
				rsslHashTableInsertLink(&pReactorWarmStandByGroupImpl->_perServiceById, &pReactorWarmStandbyServiceImpl->hashLink, (void*)&pReactorWarmStandbyServiceImpl->serviceID, NULL);

				*pAddNewService = RSSL_TRUE;
			}
		}
	}

	return RSSL_RET_SUCCESS;

allocationFailed:

	/* Cleans up memory only when the RsslReactorWarmStandbyServiceImpl object is created. */
	if (pHashLink == NULL)
	{
		if (pReactorWarmStandbyServiceImpl && pReactorWarmStandbyServiceImpl->pMemoryLocation)
		{
			free(pReactorWarmStandbyServiceImpl->pMemoryLocation);
		}

		if (pReactorWarmStandbyServiceImpl)
		{
			free(pReactorWarmStandbyServiceImpl);
		}
	}

	return RSSL_RET_FAILURE;
}

static RsslBool wlWarmStandbyCompareRDMServiceInfo(RsslReactorWarmStandbyGroupImpl *pReactorWarmStandByGroupImpl, WlServiceCache *pServiceCache)
{
	RsslQueueLink* pLink = NULL;
	RsslHashLink* pHashLink = NULL;
	RDMCachedService* pService = NULL;
	RsslReactorWarmStandbyServiceImpl *pReactorWarmStandbyServiceImpl = NULL;
	RsslRDMServiceInfo *pServiceInfo, *pOtherServiceInfo;

	RSSL_QUEUE_FOR_EACH_LINK(&pReactorWarmStandByGroupImpl->_serviceList, pLink)
	{
		pReactorWarmStandbyServiceImpl = RSSL_QUEUE_LINK_TO_OBJECT(RsslReactorWarmStandbyServiceImpl, queueLink, pLink);

		if (pReactorWarmStandbyServiceImpl)
		{
			pHashLink = rsslHashTableFind(&pServiceCache->_servicesById, &pReactorWarmStandbyServiceImpl->serviceID, NULL);

			if (pHashLink)
			{
				pService = RSSL_HASH_LINK_TO_OBJECT(RDMCachedService, _idLink, pHashLink);

				pServiceInfo = &pReactorWarmStandbyServiceImpl->rdmServiceInfo;
				pOtherServiceInfo = &pService->rdm.info;

				/* Compares all attributes of service Info except Vendor and IsSource */

				if (pServiceInfo->serviceName.length == pOtherServiceInfo->serviceName.length)
				{
					if (memcmp(pServiceInfo->serviceName.data, pOtherServiceInfo->serviceName.data, pServiceInfo->serviceName.length) != 0)
					{
						return RSSL_FALSE;
					}
				}
				else
				{
					return RSSL_FALSE;
				}

				if (pServiceInfo->capabilitiesCount == pOtherServiceInfo->capabilitiesCount)
				{
					if (memcmp(pServiceInfo->capabilitiesList, pOtherServiceInfo->capabilitiesList, pServiceInfo->capabilitiesCount) != 0)
					{
						return RSSL_FALSE;
					}
				}
				else
				{
					return RSSL_FALSE;
				}

				if (pServiceInfo->dictionariesProvidedCount == pOtherServiceInfo->dictionariesProvidedCount)
				{
					if (memcmp(pServiceInfo->dictionariesProvidedList, pOtherServiceInfo->dictionariesProvidedList, pServiceInfo->dictionariesProvidedCount) != 0)
					{
						return RSSL_FALSE;
					}
				}
				else
				{
					return RSSL_FALSE;
				}

				if (pServiceInfo->dictionariesUsedCount == pOtherServiceInfo->dictionariesUsedCount)
				{
					if (memcmp(pServiceInfo->dictionariesUsedList, pOtherServiceInfo->dictionariesUsedList, pServiceInfo->dictionariesUsedCount) != 0)
					{
						return RSSL_FALSE;
					}
				}
				else
				{
					return RSSL_FALSE;
				}

				if (pServiceInfo->qosCount == pOtherServiceInfo->qosCount)
				{
					RsslUInt32 index;

					for (index = 0; index < pServiceInfo->qosCount; index++)
					{
						if (pServiceInfo->qosList[index].dynamic != pOtherServiceInfo->qosList[index].dynamic)
							return RSSL_FALSE;

						if (pServiceInfo->qosList[index].rate != pOtherServiceInfo->qosList[index].rate)
							return RSSL_FALSE;

						if (pServiceInfo->qosList[index].rateInfo != pOtherServiceInfo->qosList[index].rateInfo)
							return RSSL_FALSE;

						if (pServiceInfo->qosList[index].timeInfo != pOtherServiceInfo->qosList[index].timeInfo)
							return RSSL_FALSE;

						if (pServiceInfo->qosList[index].timeliness != pOtherServiceInfo->qosList[index].timeliness)
							return RSSL_FALSE;
					}
				}
				else
				{
					return RSSL_FALSE;
				}

				if (pServiceInfo->supportsQosRange != pOtherServiceInfo->supportsQosRange)
				{
					return RSSL_FALSE;
				}

				if (pServiceInfo->itemList.length == pOtherServiceInfo->itemList.length)
				{
					if (memcmp(pServiceInfo->itemList.data, pOtherServiceInfo->itemList.data, pServiceInfo->itemList.length) != 0)
					{
						return RSSL_FALSE;
					}
				}
				else
				{
					return RSSL_FALSE;
				}

				if (pServiceInfo->supportsOutOfBandSnapshots != pOtherServiceInfo->supportsOutOfBandSnapshots)
				{
					return RSSL_FALSE;
				}

				if (pServiceInfo->acceptingConsumerStatus != pOtherServiceInfo->acceptingConsumerStatus)
				{
					return RSSL_FALSE;
				}
			}
			else
			{
				/* Checks to see if there is the same service name but different service ID.*/
				pServiceInfo = &pReactorWarmStandbyServiceImpl->rdmServiceInfo;

				pHashLink = rsslHashTableFind(&pServiceCache->_servicesByName, &pServiceInfo->serviceName, NULL);

				if (pHashLink)
				{
					/* Found the same service name but different service ID. */
					return RSSL_FALSE;
				}
			}
		}
	}

	return RSSL_TRUE;
}

static RsslRet wlServiceCacheInitCallback(WlServiceCache *pServiceCache, WlServiceCacheUpdateEvent *pServiceCacheUpdateEvent, RsslErrorInfo *pError)
{
	RsslWatchlist				*pWatchlist = (RsslWatchlist*)pServiceCache->pUserSpec;
	RsslReactorChannelImpl *pReactorChannelImpl = (RsslReactorChannelImpl *)pWatchlist->pUserSpec;
	RsslReactorImpl *pReactorImpl = pReactorChannelImpl->pParentReactor;
	RsslReactorWarmStandByHandlerImpl *pReactorWarmStandByHandlerImpl = pReactorChannelImpl->pWarmStandByHandlerImpl;
	RsslReactorWarmStandbyGroupImpl *pReactorWarmStandByGroupImpl = &pReactorWarmStandByHandlerImpl->warmStandbyGroupList[pReactorWarmStandByHandlerImpl->currentWSyGroupIndex];
	RsslHashLink *pHashLink = NULL;
	RsslWatchlistImpl			*pWatchlistImpl = (RsslWatchlistImpl*)pWatchlist;
	RsslBool addNewService = RSSL_FALSE;

	if (_reactorHandlesWarmStandby(pReactorChannelImpl))
	{
		if ((pReactorChannelImpl->pWarmStandByHandlerImpl->warmStandByHandlerState & RSSL_RWSB_STATE_RECEIVED_PRIMARY_DIRECTORY_RESP) == 0)
		{
			/* Submit an event to the Reactor event queue in order to establish standby connections to the secondary servers. */
			RsslReactorWarmStanbyEvent* pEvent = (RsslReactorWarmStanbyEvent*)rsslReactorEventQueueGetFromPool(&pReactorImpl->reactorEventQueue);
			rsslClearReactorWarmStanbyEvent(pEvent);

			/* Update the service cache of the warm standby group. */
			wlWarmStandbyServiceUpdate(pReactorWarmStandByGroupImpl, pReactorChannelImpl, pServiceCache, pServiceCacheUpdateEvent, &addNewService, pError);

			pReactorChannelImpl->pWarmStandByHandlerImpl->warmStandByHandlerState |= RSSL_RWSB_STATE_RECEIVED_PRIMARY_DIRECTORY_RESP;

			pEvent->reactorWarmStandByEventType = RSSL_RCIMPL_WSBET_CONNECT_SECONDARY_SERVER;
			pEvent->pReactorChannel = (RsslReactorChannel*)pReactorChannelImpl;
			pEvent->streamID = pWatchlistImpl->directory.pStream->base.streamId;

			if (!RSSL_ERROR_INFO_CHECK(rsslReactorEventQueuePut(&pReactorImpl->reactorEventQueue, (RsslReactorEventImpl*)pEvent) == RSSL_RET_SUCCESS, RSSL_RET_FAILURE, pError))
				return RSSL_RET_FAILURE;
		}
		else
		{
			RsslReactorWarmStanbyEvent* pEvent = (RsslReactorWarmStanbyEvent*)rsslReactorEventQueueGetFromPool(&pReactorImpl->reactorEventQueue);
			rsslClearReactorWarmStanbyEvent(pEvent);

			pReactorChannelImpl->pWarmStandByHandlerImpl->warmStandByHandlerState |= RSSL_RWSB_STATE_RECEIVED_SECONDARY_DIRECTORY_RESP;

			/* Compares the source directory with the starting server. */
			if (wlWarmStandbyCompareRDMServiceInfo(pReactorWarmStandByGroupImpl, pServiceCache) == RSSL_FALSE)
			{
				pEvent->reactorWarmStandByEventType = RSSL_RCIMPL_WSBET_REMOVE_SERVER_FROM_WSB_GROUP;
				pEvent->pReactorChannel = (RsslReactorChannel*)pReactorChannelImpl;
				pEvent->pReactorErrorInfoImpl = rsslReactorGetErrorInfoFromPool(&pReactorImpl->reactorWorker);

				if (pEvent->pReactorErrorInfoImpl)
				{
					rsslClearReactorErrorInfoImpl(pEvent->pReactorErrorInfoImpl);
					rsslSetErrorInfo(&pEvent->pReactorErrorInfoImpl->rsslErrorInfo, RSSL_EIC_FAILURE, RSSL_RET_FAILURE, __FILE__, __LINE__, "The source directory response from standby server does not match with the primary server.");
				}

				if (!RSSL_ERROR_INFO_CHECK(rsslReactorEventQueuePut(&pReactorImpl->reactorEventQueue, (RsslReactorEventImpl*)pEvent) == RSSL_RET_SUCCESS, RSSL_RET_FAILURE, pError))
					return RSSL_RET_FAILURE;
			}
			else
			{
				/* Support adding a new service and aggregating service state to the service cache of the warm standby group. */
				wlWarmStandbyServiceUpdate(pReactorWarmStandByGroupImpl, pReactorChannelImpl, pServiceCache, pServiceCacheUpdateEvent, &addNewService, pError);

				pEvent->reactorWarmStandByEventType = RSSL_RCIMPL_WSBET_NOTIFY_STANDBY_SERVICE;
				pEvent->pReactorChannel = (RsslReactorChannel*)pReactorChannelImpl;
				pEvent->streamID = pWatchlistImpl->directory.pStream->base.streamId;

				if (!RSSL_ERROR_INFO_CHECK(rsslReactorEventQueuePut(&pReactorImpl->reactorEventQueue, (RsslReactorEventImpl*)pEvent) == RSSL_RET_SUCCESS, RSSL_RET_FAILURE, pError))
					return RSSL_RET_FAILURE;
			}
		}
	}

	return RSSL_RET_SUCCESS;
}

static RsslRet wlServiceCacheUpdateCallback(WlServiceCache *pServiceCache, WlServiceCacheUpdateEvent *pServiceCacheUpdateEvent, RsslErrorInfo *pError)
{
	RsslWatchlist				*pWatchlist = (RsslWatchlist*)pServiceCache->pUserSpec;
	RsslReactorChannelImpl *pReactorChannelImpl = (RsslReactorChannelImpl *)pWatchlist->pUserSpec;
	RsslReactorImpl *pReactorImpl = pReactorChannelImpl->pParentReactor;
	RsslReactorWarmStandByHandlerImpl *pReactorWarmStandByHandlerImpl = pReactorChannelImpl->pWarmStandByHandlerImpl;
	RsslReactorWarmStandbyGroupImpl *pReactorWarmStandByGroupImpl = &pReactorWarmStandByHandlerImpl->warmStandbyGroupList[pReactorWarmStandByHandlerImpl->currentWSyGroupIndex];
	RsslHashLink *pHashLink = NULL;
	RsslWatchlistImpl			*pWatchlistImpl = (RsslWatchlistImpl*)pWatchlist;
	RsslBool addNewService = RSSL_FALSE;

	if (_reactorHandlesWarmStandby(pReactorChannelImpl))
	{
		/* Compares the source directory with the starting server. */
		if (wlWarmStandbyCompareRDMServiceInfo(pReactorWarmStandByGroupImpl, pServiceCache) == RSSL_FALSE)
		{
			RsslReactorWarmStanbyEvent* pEvent = (RsslReactorWarmStanbyEvent*)rsslReactorEventQueueGetFromPool(&pReactorImpl->reactorEventQueue);
			rsslClearReactorWarmStanbyEvent(pEvent);

			pEvent->reactorWarmStandByEventType = RSSL_RCIMPL_WSBET_REMOVE_SERVER_FROM_WSB_GROUP;
			pEvent->pReactorChannel = (RsslReactorChannel*)pReactorChannelImpl;
			pEvent->pReactorErrorInfoImpl = rsslReactorGetErrorInfoFromPool(&pReactorImpl->reactorWorker);

			if (pEvent->pReactorErrorInfoImpl)
			{
				rsslClearReactorErrorInfoImpl(pEvent->pReactorErrorInfoImpl);
				rsslSetErrorInfo(&pEvent->pReactorErrorInfoImpl->rsslErrorInfo, RSSL_EIC_FAILURE, RSSL_RET_FAILURE, __FILE__, __LINE__, "The source directory response from standby server does not match with the primary server.");
			}

			if (!RSSL_ERROR_INFO_CHECK(rsslReactorEventQueuePut(&pReactorImpl->reactorEventQueue, (RsslReactorEventImpl*)pEvent) == RSSL_RET_SUCCESS, RSSL_RET_FAILURE, pError))
				return RSSL_RET_FAILURE;
		}
		else
		{
			/* Support adding a new service and aggregating service state to the service cache of the warm standby group. */
			wlWarmStandbyServiceUpdate(pReactorWarmStandByGroupImpl, pReactorChannelImpl, pServiceCache, pServiceCacheUpdateEvent, &addNewService, pError);

			if (addNewService == RSSL_TRUE)
			{
				RsslReactorWarmStanbyEvent* pEvent = (RsslReactorWarmStanbyEvent*)rsslReactorEventQueueGetFromPool(&pReactorImpl->reactorEventQueue);
				rsslClearReactorWarmStanbyEvent(pEvent);

				pReactorChannelImpl->pWarmStandByHandlerImpl->warmStandByHandlerState |= RSSL_RWSB_STATE_RECEIVED_SECONDARY_DIRECTORY_RESP;
				pEvent->reactorWarmStandByEventType = RSSL_RCIMPL_WSBET_NOTIFY_STANDBY_SERVICE;
				pEvent->pReactorChannel = (RsslReactorChannel*)pReactorChannelImpl;
				pEvent->streamID = pWatchlistImpl->directory.pStream->base.streamId;

				if (!RSSL_ERROR_INFO_CHECK(rsslReactorEventQueuePut(&pReactorImpl->reactorEventQueue, (RsslReactorEventImpl*)pEvent) == RSSL_RET_SUCCESS, RSSL_RET_FAILURE, pError))
					return RSSL_RET_FAILURE;
			}
		}
	}

	return RSSL_RET_SUCCESS;
}

static RsslRet wlProcessRemovedService(RsslWatchlistImpl *pWatchlistImpl,
		WlService *pWlService, RsslErrorInfo *pErrorInfo)
{
	RsslQueueLink *pLink;
	RsslWatchlistMsgEvent msgEvent;
	RsslStatusMsg statusMsg;
	RsslRet ret;
	RsslReactorChannelImpl *pReactorChannelImpl = (RsslReactorChannelImpl *)pWatchlistImpl->base.watchlist.pUserSpec;

	/* A removed service means ClosedRecover status for all items requested from it. Fan this out
	* so they go to recovery if appropriate. */
	rsslClearStatusMsg(&statusMsg);
	statusMsg.flags |= RSSL_STMF_HAS_STATE;
	statusMsg.state.streamState = RSSL_STREAM_CLOSED_RECOVER;
	statusMsg.state.dataState = RSSL_DATA_SUSPECT;
	statusMsg.msgBase.containerType = RSSL_DT_NO_DATA;
	rssl_set_buffer_to_string(statusMsg.state.text, "Service for this item was lost.");

	wlMsgEventClear(&msgEvent);
	msgEvent.pRsslMsg = (RsslMsg*)&statusMsg;

	if (_reactorHandlesWarmStandby(pReactorChannelImpl) && pWlService->openStreamList.count > 0)
	{
		RsslReactorWarmStandByHandlerImpl *pReactorWarmStandByHandlerImpl = pReactorChannelImpl->pWarmStandByHandlerImpl;
		RsslReactorWarmStandbyGroupImpl *pReactorWarmStandByGroupImpl = &pReactorWarmStandByHandlerImpl->warmStandbyGroupList[pReactorWarmStandByHandlerImpl->currentWSyGroupIndex];
		RsslWatchlistImpl* pWatchlistImplTemp;
		RsslHashLink* pHashLink;

		if (pReactorWarmStandByGroupImpl->warmStandbyMode == RSSL_RWSB_MODE_LOGIN_BASED)
		{
			/* Checks whether the next channel provides this service as well. */
			if (pReactorWarmStandByHandlerImpl->pNextActiveReactorChannel)
			{
				if (isRsslChannelActive(pReactorWarmStandByHandlerImpl->pNextActiveReactorChannel))
				{
					pWatchlistImplTemp = (RsslWatchlistImpl*)pReactorWarmStandByHandlerImpl->pNextActiveReactorChannel->pWatchlist;

					pHashLink = rsslHashTableFind(&pWatchlistImplTemp->base.pServiceCache->_servicesById, &pWlService->pService->rdm.serviceId, NULL);
					if (!pHashLink) /* Don't have the service for next channel. */
					{
						msgEvent._flags = (WL_MEF_NOTIFY_STATUS | WL_MEF_SEND_CLOSED_RECOVER);
					}
					else
					{
						/* The next active channel has service so notify item status before switching to it. */
						msgEvent._flags = WL_MEF_NOTIFY_STATUS;
					}
				}
			}
		}
		else
		{
			RsslReactorChannelImpl* pProcessReactorChannel = NULL;
			RsslQueueLink* pLink;
			RsslBool hasChannelDown = RSSL_FALSE;
			RsslBool hasService = RSSL_FALSE;

			RSSL_MUTEX_LOCK(&pReactorWarmStandByHandlerImpl->warmStandByHandlerMutex);
			/* Checks whether others channels provide this service as well. */
			if (pReactorWarmStandByHandlerImpl->rsslChannelQueue.count > 1)
			{
				RSSL_QUEUE_FOR_EACH_LINK(&pReactorWarmStandByHandlerImpl->rsslChannelQueue, pLink)
				{
					pProcessReactorChannel = RSSL_QUEUE_LINK_TO_OBJECT(RsslReactorChannelImpl, warmstandbyChannelLink, pLink);

					if (pProcessReactorChannel != pReactorChannelImpl)
					{
						if (isRsslChannelActive(pProcessReactorChannel) == RSSL_FALSE)
						{
							hasChannelDown = RSSL_TRUE;
							break;
						}

						pWatchlistImplTemp = (RsslWatchlistImpl*)pProcessReactorChannel->pWatchlist;

						pHashLink = rsslHashTableFind(&pWatchlistImplTemp->base.pServiceCache->_servicesById, &pWlService->pService->rdm.serviceId, NULL);
						if (pHashLink)
						{
							hasService = RSSL_TRUE;
							break;
						}
					}
				}

				if (!hasChannelDown && hasService == RSSL_FALSE)
				{
					msgEvent._flags = (WL_MEF_NOTIFY_STATUS | WL_MEF_SEND_CLOSED_RECOVER);
				}
			}
			RSSL_MUTEX_UNLOCK(&pReactorWarmStandByHandlerImpl->warmStandByHandlerMutex);
		}

		RSSL_QUEUE_FOR_EACH_LINK(&pWlService->openStreamList, pLink)
		{
			RsslUInt8 originalFlags = msgEvent._flags; /* Keeps the original flag values */

			WlItemStream* pStream = RSSL_QUEUE_LINK_TO_OBJECT(WlItemStream, qlServiceStreams,
				pLink);

			if (pStream->itemIsClosedForAllStandby)
			{
				msgEvent._flags = (WL_MEF_NOTIFY_STATUS | WL_MEF_SEND_CLOSED_RECOVER);
				pStream->itemIsClosedForAllStandby = RSSL_FALSE;
			}

			statusMsg.msgBase.domainType = pStream->base.domainType;
			if ((ret = wlFanoutItemMsgEvent(pWatchlistImpl, pStream, &msgEvent,
				pErrorInfo))
				!= RSSL_RET_SUCCESS)
				return ret;
			pWatchlistImpl->items.pCurrentFanoutStream = NULL;

			msgEvent._flags = msgEvent._flags; /* Restore the status flags */
		}
	}
	else
	{
		RSSL_QUEUE_FOR_EACH_LINK(&pWlService->openStreamList, pLink)
		{
			WlItemStream* pStream = RSSL_QUEUE_LINK_TO_OBJECT(WlItemStream, qlServiceStreams,
				pLink);

			statusMsg.msgBase.domainType = pStream->base.domainType;
			if ((ret = wlFanoutItemMsgEvent(pWatchlistImpl, pStream, &msgEvent,
				pErrorInfo))
				!= RSSL_RET_SUCCESS)
				return ret;
			pWatchlistImpl->items.pCurrentFanoutStream = NULL;
		}
	}

	return RSSL_RET_SUCCESS;
}

static RsslRet wlRecoverAllItems(RsslWatchlistImpl *pWatchlistImpl, RsslErrorInfo *pErrorInfo)
{
	return wlServiceCacheClear(pWatchlistImpl->base.pServiceCache, RSSL_TRUE, pErrorInfo);
}

RsslRet rsslWatchlistReadMsg(RsslWatchlist *pWatchlist, 
		RsslWatchlistProcessMsgOptions *pOptions, ReactorWSProcessMsgOptions* pWsOption, RsslInt64 currentTime, RsslErrorInfo *pErrorInfo)
{
	RsslRet			ret;
	RsslHashLink	*pHashLink;
	WlStream		*pStream;
	RsslWatchlistMsgEvent msgEvent;
	RsslWatchlistImpl *pWatchlistImpl = (RsslWatchlistImpl*)pWatchlist;
	RsslReactorChannelImpl* pReactorChannelImpl = (RsslReactorChannelImpl*)pWatchlistImpl->base.watchlist.pUserSpec;

	pWatchlistImpl->base.currentTime = currentTime;

	if (pOptions->pRsslMsg->msgBase.streamId != 0)
	{
		/* Match ID to its stream in our table. */

		pHashLink = rsslHashTableFind(&pWatchlistImpl->base.streamsById, 
				(void*)&pOptions->pRsslMsg->msgBase.streamId, NULL);

		pStream = pHashLink ? RSSL_HASH_LINK_TO_OBJECT(WlStream, base.hlStreamId, pHashLink)
			: NULL;
	}
	else
	{
		/* When using the multicast transport, messages that are received from multicast
		 * have a StreamID of zero. Match the key to our corresponding stream, if any. */

		switch(pOptions->pRsslMsg->msgBase.domainType)
		{
			/* Watchlist only uses one login stream. */
			case RSSL_DMT_LOGIN: pStream = (WlStream*)pWatchlistImpl->login.pStream; break;

			/* Watchlist only uses one directory stream. */
			case RSSL_DMT_SOURCE: pStream = (WlStream*)pWatchlistImpl->directory.pStream; break;

			default:
			{

				const RsslMsgKey *pMsgKey = rsslGetReqMsgKey(pOptions->pRsslMsg);
				if (!pMsgKey) pMsgKey = rsslGetMsgKey(pOptions->pRsslMsg);

				if (pMsgKey)
				{
					WlStreamAttributes streamAttributes;
					streamAttributes.domainType = pOptions->pRsslMsg->msgBase.domainType;
					streamAttributes.msgKey = *pMsgKey;

					/* Most response messages do not provide a QoS for matching. */
					streamAttributes.hasQos = RSSL_FALSE;

					pHashLink = rsslHashTableFind(&pWatchlistImpl->base.openStreamsByAttrib,
							(void*)&streamAttributes, NULL);

					pStream = pHashLink ? (WlStream*) RSSL_HASH_LINK_TO_OBJECT(WlItemStream, 
							hlStreamsByAttrib, pHashLink) : NULL;
				}
				else
					pStream = NULL;
			}
		}
	}

	if (pWsOption != NULL)
	{
		pWsOption->pStream = (void*)pStream;
	}

	if (!pStream)
	{
		/* Messages from unrecognized streams are likely messages that were sent at the
		 * same time the consumer closed a stream.  Most such messages can be ignored, however
		 * if the provider closed the stream instead of the consumer, any recent reissued request 
		 * may be misinterpreted as a new request using the same stream. So in this case,
		 * send a close message to make sure. */
		/* Don't do this for broadcast messages. */
		if (pOptions->pRsslMsg->msgBase.streamId != 0)
		{
			const RsslState *pState = rsslGetState(pOptions->pRsslMsg);

			if (pState && pState->streamState != RSSL_STREAM_OPEN)
			{
				WlStreamBase *pStreamBase = (WlStreamBase*)malloc(sizeof(WlStreamBase));
				verify_malloc(pStreamBase, pErrorInfo, RSSL_RET_FAILURE);
				pStreamBase->streamId = pOptions->pRsslMsg->msgBase.streamId;
				pStreamBase->domainType = pOptions->pRsslMsg->msgBase.domainType;
				pStreamBase->isClosing = RSSL_TRUE;
				pStreamBase->tempStream = RSSL_TRUE;
				pStreamBase->requestState = WL_STRS_NONE;

				wlSetStreamMsgPending(&pWatchlistImpl->base, pStreamBase);
			}
		}

		return (pWatchlistImpl->base.streamsPendingRequest.count 
				|| pWatchlistImpl->base.newRequests.count) ? 1 : RSSL_RET_SUCCESS;
	}

	wlMsgEventClear(&msgEvent);
	msgEvent.pRsslBuffer = pOptions->pRsslBuffer;
	msgEvent.pRsslMsg = pOptions->pRsslMsg;
	msgEvent.pSeqNum = pOptions->pSeqNum;
	msgEvent.pFTGroupId = pOptions->pFTGroupId;

	switch(pStream->base.domainType)
	{
		case RSSL_DMT_LOGIN:
		{
			RsslRDMLoginMsg loginMsg;
			WlLoginProviderAction loginAction;
			RsslWatchlistStreamInfo streamInfo;
			WlLoginRequest *pLoginRequest = pWatchlistImpl->login.pRequest[pWatchlistImpl->login.index];

			if (msgEvent.pRsslMsg->msgBase.domainType != RSSL_DMT_LOGIN)
			{

				wlStreamInfoClear(&streamInfo);
				streamInfo.pUserSpec = pLoginRequest->base.pUserSpec;
				msgEvent.pStreamInfo = &streamInfo;

				/* May be an off-stream post acknowledgement, pass it through. */
				switch(msgEvent.pRsslMsg->msgBase.msgClass)
				{
					case RSSL_MC_ACK:
					{
						WlPostRecord *pRecord;
						if ((pRecord = wlPostTableFindRecord(&pWatchlistImpl->base.postTable, 
										&msgEvent.pRsslMsg->ackMsg)))
						{
							rsslQueueRemoveLink(&pLoginRequest->base.openPosts, &pRecord->qlUser);
							wlPostTableRemoveRecord(&pWatchlistImpl->base.postTable, pRecord);

							msgEvent.pRsslMsg->msgBase.streamId = pLoginRequest->base.streamId;

							return (*pWatchlistImpl->base.config.msgCallback)((RsslWatchlist*)&pWatchlistImpl->base.watchlist, 
									&msgEvent, pErrorInfo);
						}
						return RSSL_RET_SUCCESS;
					}

					default:
						rsslSetErrorInfo(pErrorInfo, RSSL_EIC_FAILURE, RSSL_RET_FAILURE, __FILE__, __LINE__, 
								"Received unsupported off-stream message on login stream with domain type %u and RsslMsg class %u.",
								msgEvent.pRsslMsg->msgBase.domainType, msgEvent.pRsslMsg->msgBase.msgClass);
						return RSSL_RET_FAILURE;
				}
			}
			else if (msgEvent.pRsslMsg->msgBase.msgClass == RSSL_MC_GENERIC)
			{
				/* Let generic message through. */
				msgEvent.pRsslMsg->msgBase.streamId = pLoginRequest->base.streamId;
				return (*pWatchlistImpl->base.config.msgCallback)((RsslWatchlist*)&pWatchlistImpl->base.watchlist, 
						&msgEvent, pErrorInfo);
			}

			if ((ret = wlLoginProcessProviderMsg(&pWatchlistImpl->login, &pWatchlistImpl->base, 
					pOptions->pDecodeIterator, msgEvent.pRsslMsg, &loginMsg, &loginAction, pErrorInfo))
					!= RSSL_RET_SUCCESS)
				return ret;

			wlStreamInfoClear(&streamInfo);
			streamInfo.pServiceName = NULL;
			streamInfo.pUserSpec = pLoginRequest->base.pUserSpec;

			msgEvent.pRdmMsg = (RsslRDMMsg*)&loginMsg;
			msgEvent.pStreamInfo = &streamInfo;

			switch(loginAction)
			{
				case WL_LGPA_NONE:

					msgEvent.pRsslMsg->msgBase.streamId = 
						loginMsg.rdmMsgBase.streamId = pLoginRequest->base.streamId;
					if ((ret = (*pWatchlistImpl->base.config.msgCallback)
								((RsslWatchlist*)&pWatchlistImpl->base.watchlist, &msgEvent, pErrorInfo)) 
							!= RSSL_RET_SUCCESS)
						return ret;

					/* If response indicates login succeeded, open source directory stream. */
					if (pWatchlistImpl->base.channelState == WL_CHS_LOGGED_IN)
					{
						/* Create directory stream. */
						/* In some circumstances the directory stream may still be considered
						 * open. In those cases, just reissue. */
						if (!pWatchlistImpl->directory.pStream)
						{
							if (!(pWatchlistImpl->directory.pStream = wlDirectoryStreamCreate(
											&pWatchlistImpl->base, &pWatchlistImpl->directory, 
											pErrorInfo)))
								return pErrorInfo->rsslError.rsslErrorId;
						}
						else
							wlSetStreamMsgPending(&pWatchlistImpl->base,
									&pWatchlistImpl->directory.pStream->base);

						pWatchlistImpl->base.channelState = WL_CHS_READY;
					}
					break;

				case WL_LGPA_RECOVER:
				{
					WlDirectoryStream *pDirectoryStream;

					msgEvent.pRsslMsg->msgBase.streamId = 
						loginMsg.rdmMsgBase.streamId = pLoginRequest->base.streamId;
					if ((ret = (*pWatchlistImpl->base.config.msgCallback)
								((RsslWatchlist*)&pWatchlistImpl->base.watchlist, &msgEvent, pErrorInfo)) 
							!= RSSL_RET_SUCCESS)
						return ret;

					/* Close directory stream, if present. */
					pDirectoryStream = pWatchlistImpl->directory.pStream;
					if (pDirectoryStream)
					{
						wlDirectoryStreamClose(&pWatchlistImpl->base,
								&pWatchlistImpl->directory, RSSL_FALSE);
						wlDirectoryStreamDestroy(pDirectoryStream);

						if(pWsOption != NULL)
							pWsOption->pStream = 0;
					}

					/* Login stream was closed but is recoverable. In this case,
					 * disconnect after providing the login response, so other connections
					 * can be tried if available. 
					 * Returning an error will trigger the disconnect. */
					rsslSetErrorInfo(pErrorInfo, RSSL_EIC_FAILURE, RSSL_RET_FAILURE, __FILE__, __LINE__, 
							"Received login response with Closed/Recover stream state. Disconnecting.");
					return RSSL_RET_FAILURE;
				}

				case WL_LGPA_CLOSE:
				{

					RsslStatusMsg 			statusMsg;
					RsslWatchlistMsgEvent	itemMsgEvent;
					RsslQueueLink *pStreamLink;
					WlLoginStream *pLoginStream;
					WlDirectoryStream *pDirectoryStream;

					/* Remove login stream/request. */
					pLoginStream = pWatchlistImpl->login.pStream;
					wlLoginStreamClose(&pWatchlistImpl->base, &pWatchlistImpl->login, RSSL_FALSE);

					pWatchlistImpl->login.pRequest[pWatchlistImpl->login.index] = NULL;

					/* Forward processed message to application. */
					msgEvent.pRsslMsg->msgBase.streamId = 
						loginMsg.rdmMsgBase.streamId = pLoginRequest->base.streamId;
					if ((ret = (*pWatchlistImpl->base.config.msgCallback)
								((RsslWatchlist*)&pWatchlistImpl->base.watchlist, &msgEvent, pErrorInfo)) 
							!= RSSL_RET_SUCCESS)
						return ret;

					/* Destroy login stream. */
					wlLoginStreamDestroy(pLoginStream);
					wlLoginRequestDestroy(&pWatchlistImpl->base, pLoginRequest);

					/* Close directory stream, if present. */
					pDirectoryStream = pWatchlistImpl->directory.pStream;
					if (pDirectoryStream)
					{
						wlDirectoryStreamClose(&pWatchlistImpl->base,
								&pWatchlistImpl->directory, RSSL_FALSE);
						wlDirectoryStreamDestroy(pDirectoryStream);
					}

					if (pWsOption != NULL)
						pWsOption->pStream = 0;

					/* Fanout closed status to all item streams. */
					rsslClearStatusMsg(&statusMsg);
					statusMsg.msgBase.containerType = RSSL_DT_NO_DATA;
					statusMsg.flags = RSSL_STMF_HAS_STATE;
					rssl_set_buffer_to_string(statusMsg.state.text, "Login stream was closed.");
					statusMsg.state.streamState = RSSL_STREAM_CLOSED;
					statusMsg.state.dataState = RSSL_DATA_SUSPECT;
					statusMsg.state.code = RSSL_SC_NONE;

					wlMsgEventClear(&itemMsgEvent);
					itemMsgEvent.pRsslMsg = (RsslMsg*)&statusMsg;

					RSSL_QUEUE_FOR_EACH_LINK(&pWatchlistImpl->base.openStreams, pStreamLink)
					{
						WlItemStream *pItemStream;

						if (statusMsg.msgBase.domainType == RSSL_DMT_LOGIN || statusMsg.msgBase.domainType == RSSL_DMT_SOURCE)
							continue;

						pItemStream = RSSL_QUEUE_LINK_TO_OBJECT(WlItemStream, base.qlStreamsList, pStreamLink);

						statusMsg.msgBase.domainType = pItemStream->streamAttributes.domainType;

						if ((ret = wlFanoutItemMsgEvent(pWatchlistImpl, pItemStream, 
										&itemMsgEvent, pErrorInfo)) 
								!= RSSL_RET_SUCCESS)
							return ret;
						pWatchlistImpl->items.pCurrentFanoutStream = NULL;

						/* Closes items for all standby servers as the item is closed */
						if (_reactorHandlesWarmStandby(pReactorChannelImpl))
						{
							RsslReactorWarmStandByHandlerImpl* pWarmStandByHandlerImpl = pReactorChannelImpl->pWarmStandByHandlerImpl;
							RsslReactorWarmStandbyGroupImpl* pWarmStandByGroupImpl = &pWarmStandByHandlerImpl->warmStandbyGroupList[pWarmStandByHandlerImpl->currentWSyGroupIndex];
							RsslReactorChannelImpl* pProcessReactorChannel;
							RsslCloseMsg rsslCloseMsg;
							RsslQueueLink* pLink = NULL;
							RsslErrorInfo errorInfo;
							RsslBool isActiveServer = RSSL_FALSE;
							RsslBool isFanoutChannelActive = RSSL_FALSE;
							RsslWatchlistProcessMsgOptions processOpts;
							rsslClearCloseMsg(&rsslCloseMsg);
							rsslCloseMsg.msgBase.streamId = statusMsg.msgBase.streamId;

							if (pWarmStandByGroupImpl->warmStandbyMode == RSSL_RWSB_MODE_LOGIN_BASED)
							{
								isActiveServer = pReactorChannelImpl->isActiveServer;
							}
							else
							{
									isActiveServer = _isActiveServiceForWSBChannelByID(pWarmStandByGroupImpl, pReactorChannelImpl, 
										pItemStream->streamAttributes.msgKey.serviceId);
							}

							/* Closes the item stream for all standby servers only */
							RSSL_MUTEX_LOCK(&pWarmStandByHandlerImpl->warmStandByHandlerMutex);
							RSSL_QUEUE_FOR_EACH_LINK(&pWarmStandByHandlerImpl->rsslChannelQueue, pLink)
							{
								isFanoutChannelActive = RSSL_FALSE;
								pProcessReactorChannel = RSSL_QUEUE_LINK_TO_OBJECT(RsslReactorChannelImpl, warmstandbyChannelLink, pLink);

								if (pProcessReactorChannel != pReactorChannelImpl)
								{
									if (!isActiveServer)
									{
										if (pWarmStandByGroupImpl->warmStandbyMode == RSSL_RWSB_MODE_LOGIN_BASED)
										{
											isFanoutChannelActive = pProcessReactorChannel->isActiveServer;
										}
										else
										{
											isFanoutChannelActive = _isActiveServiceForWSBChannelByID(pWarmStandByGroupImpl, pProcessReactorChannel, 
													pItemStream->streamAttributes.msgKey.serviceId);
										}

										/* Don't close the item for the current active server as it can provide updates. */
										if (isFanoutChannelActive)
										{
											RsslWatchlistImpl* pActiveSrvWatchlist = (RsslWatchlistImpl*)pProcessReactorChannel->pWatchlist;
											WlItemStream* pItemStreamTemp = NULL;

											if (pActiveSrvWatchlist)
											{
												pHashLink = rsslHashTableFind(&pActiveSrvWatchlist->base.streamsById,
													(void*)&pItemStream->base.streamId, NULL);

												pItemStreamTemp = (WlItemStream*)(pHashLink ? RSSL_HASH_LINK_TO_OBJECT(WlStream, base.hlStreamId, pHashLink)
													: NULL);

												if (pItemStreamTemp)
												{
													pItemStreamTemp->itemIsClosedForAllStandby = RSSL_TRUE;
												}
											}
											continue;
										}
									}

									rsslWatchlistClearProcessMsgOptions(&processOpts);
									processOpts.pChannel = pProcessReactorChannel->reactorChannel.pRsslChannel;
									processOpts.pRsslMsg = (RsslMsg*)&rsslCloseMsg;
									processOpts.majorVersion = pProcessReactorChannel->reactorChannel.majorVersion;
									processOpts.minorVersion = pProcessReactorChannel->reactorChannel.minorVersion;

									_reactorSubmitWatchlistMsg(pProcessReactorChannel->pParentReactor, pProcessReactorChannel, &processOpts, &errorInfo);
								}

							}
							RSSL_MUTEX_UNLOCK(&pWarmStandByHandlerImpl->warmStandByHandlerMutex);
						}
					}

					if (_reactorHandlesWarmStandby(pReactorChannelImpl))
					{
						RsslReactorImpl* pReactorImpl = pReactorChannelImpl->pParentReactor;
						RsslReactorWarmStanbyEvent* pReactorWarmStanbyEvent;
						RsslReactorWarmStandbyGroupImpl* pWarmStandByGroup = &pReactorChannelImpl->pWarmStandByHandlerImpl->warmStandbyGroupList[pReactorChannelImpl->pWarmStandByHandlerImpl->currentWSyGroupIndex];

						if (pWarmStandByGroup->warmStandbyMode == RSSL_RWSB_MODE_LOGIN_BASED)
						{
							if (pReactorChannelImpl->isActiveServer)
							{
								RsslQueueLink* pLink;
								RsslReactorChannelImpl* pNextReactorChannel = NULL;

								RSSL_MUTEX_LOCK(&pReactorChannelImpl->pWarmStandByHandlerImpl->warmStandByHandlerMutex);
								/* Submits to a channel that belongs to the warm standby feature and it is active */
								RSSL_QUEUE_FOR_EACH_LINK(&pReactorChannelImpl->pWarmStandByHandlerImpl->rsslChannelQueue, pLink)
								{
									pNextReactorChannel = RSSL_QUEUE_LINK_TO_OBJECT(RsslReactorChannelImpl, warmstandbyChannelLink, pLink);

									if (pReactorChannelImpl != pNextReactorChannel && !pNextReactorChannel->isLoggedOutFromWSB && pNextReactorChannel->reactorChannel.pRsslChannel &&
										pNextReactorChannel->reactorChannel.pRsslChannel->state == RSSL_CH_STATE_ACTIVE)
									{
										pReactorWarmStanbyEvent = (RsslReactorWarmStanbyEvent*)rsslReactorEventQueueGetFromPool(&pReactorImpl->reactorEventQueue);
										rsslClearReactorWarmStanbyEvent(pReactorWarmStanbyEvent);

										pReactorWarmStanbyEvent->reactorWarmStandByEventType = RSSL_RCIMPL_WSBET_CHANGE_ACTIVE_TO_STANDBY_SERVER;
										pReactorWarmStanbyEvent->pReactorChannel = (RsslReactorChannel*)pReactorChannelImpl;
										pReactorChannelImpl->pWarmStandByHandlerImpl->pNextActiveReactorChannel = pNextReactorChannel;
										if (!RSSL_ERROR_INFO_CHECK(rsslReactorEventQueuePut(&pReactorImpl->reactorEventQueue, (RsslReactorEventImpl*)pReactorWarmStanbyEvent) == RSSL_RET_SUCCESS, RSSL_RET_FAILURE, pErrorInfo))
										{
											RSSL_MUTEX_UNLOCK(&pReactorChannelImpl->pWarmStandByHandlerImpl->warmStandByHandlerMutex);
											return RSSL_RET_FAILURE;
										}

										break;
									}
								}
								RSSL_MUTEX_UNLOCK(&pReactorChannelImpl->pWarmStandByHandlerImpl->warmStandByHandlerMutex);

								pReactorChannelImpl->isActiveServer = RSSL_FALSE;
								pReactorChannelImpl->pWarmStandByHandlerImpl->pActiveReactorChannel = NULL;
							}
						}
						else
						{
							pReactorWarmStanbyEvent = (RsslReactorWarmStanbyEvent*)rsslReactorEventQueueGetFromPool(&pReactorImpl->reactorEventQueue);
							rsslClearReactorWarmStanbyEvent(pReactorWarmStanbyEvent);

							/* Per service based warm standby */
							pReactorWarmStanbyEvent->reactorWarmStandByEventType = RSSL_RCIMPL_WSBET_CHANGE_ACTIVE_TO_STANDBY_SERVICE_CHANNEL_DOWN;
							pReactorWarmStanbyEvent->pReactorChannel = (RsslReactorChannel*)pReactorChannelImpl;

							if (!RSSL_ERROR_INFO_CHECK(rsslReactorEventQueuePut(&pReactorImpl->reactorEventQueue, (RsslReactorEventImpl*)pReactorWarmStanbyEvent) == RSSL_RET_SUCCESS, RSSL_RET_FAILURE, pErrorInfo))
							{
								return RSSL_RET_FAILURE;
							}
						}

						/* Closes this channel */
						if (pReactorChannelImpl->isLoggedOutFromWSB && pReactorChannelImpl->reactorParentQueue != &pReactorImpl->closingChannels)
						{
							if (pReactorChannelImpl->reactorChannel.pRsslChannel && pReactorChannelImpl->reactorChannel.pRsslChannel->state == RSSL_CH_STATE_ACTIVE)
							{
								pReactorChannelImpl->reconnectAttemptLimit = 0; /* Don't recover this channel */

								/* Closes this channel */
								RsslReactorWarmStanbyEvent* pReactorWarmStanbyEvent = (RsslReactorWarmStanbyEvent*)rsslReactorEventQueueGetFromPool(&pReactorImpl->reactorEventQueue);
								rsslClearReactorWarmStanbyEvent(pReactorWarmStanbyEvent);

								pReactorWarmStanbyEvent->reactorWarmStandByEventType = RSSL_RCIMPL_WSBET_CLOSE_RSSL_CHANEL_ONLY;
								pReactorWarmStanbyEvent->pReactorChannel = (RsslReactorChannel*)pReactorChannelImpl;
								pReactorWarmStanbyEvent->pReactorErrorInfoImpl = rsslReactorGetErrorInfoFromPool(&pReactorImpl->reactorWorker);

								if (pReactorWarmStanbyEvent->pReactorErrorInfoImpl)
								{
									rsslClearReactorErrorInfoImpl(pReactorWarmStanbyEvent->pReactorErrorInfoImpl);
									rsslSetErrorInfo(&pReactorWarmStanbyEvent->pReactorErrorInfoImpl->rsslErrorInfo, RSSL_EIC_FAILURE, RSSL_RET_FAILURE, __FILE__, __LINE__, "Received login stream closed.");
								}

								if (!RSSL_ERROR_INFO_CHECK(rsslReactorEventQueuePut(&pReactorImpl->reactorEventQueue, (RsslReactorEventImpl*)pReactorWarmStanbyEvent) == RSSL_RET_SUCCESS, RSSL_RET_FAILURE, pErrorInfo))
									return RSSL_RET_FAILURE;
							}
						}
					}

					if ((ret = wlServiceCacheClear(pWatchlistImpl->base.pServiceCache, RSSL_TRUE, pErrorInfo)) != RSSL_RET_SUCCESS)
						return ret;

					break;
				}


				default:
					assert(0);
					rsslSetErrorInfo(pErrorInfo, RSSL_EIC_FAILURE, RSSL_RET_INVALID_DATA, __FILE__, __LINE__, 
							"Internal error -- Unexpected action while processing login response.");
					return RSSL_RET_FAILURE;
			}

			break;
		}

		case RSSL_DMT_SOURCE:
		{
			if ((ret = wlDirectoryProcessProviderMsgEvent(&pWatchlistImpl->base, 
					&pWatchlistImpl->directory, pOptions->pDecodeIterator, &msgEvent, pErrorInfo)) != RSSL_RET_SUCCESS)
				return ret;

			/* If state was reset to logged in, it means we must recover the directory stream. */
			if (pWatchlistImpl->base.channelState == WL_CHS_LOGGED_IN)
			{
				wlSetStreamMsgPending(&pWatchlistImpl->base,
						&pWatchlistImpl->directory.pStream->base);

				pWatchlistImpl->base.channelState = WL_CHS_READY;
			}

			break;
		}

		default:
		{
			WlItemStream *pItemStream = (WlItemStream*)pStream;

			if (!(msgEvent.pSeqNum))
			{
				if ((ret = wlFanoutItemMsgEvent(pWatchlistImpl, pItemStream, &msgEvent, 
								pErrorInfo)) 
						!= RSSL_RET_SUCCESS)
					return ret;
				pWatchlistImpl->items.pCurrentFanoutStream = NULL;
			}
			else if (!(pItemStream->flags & WL_IOSF_PRIVATE))
			{
				pWatchlistImpl->items.pCurrentFanoutStream = pItemStream;

				if ((ret = wlItemStreamOrderMsg(pWatchlistImpl, pItemStream, &msgEvent, 
								pErrorInfo)) != RSSL_RET_SUCCESS)
					return ret;

				pWatchlistImpl->items.pCurrentFanoutStream = NULL;

			}
			else
			{
				/* Private stream. Check all messages for in-order sequence. */
				if (pWatchlistImpl->base.gapRecovery
						&& *msgEvent.pSeqNum != 0
						&& !(pItemStream->flags & WL_IOSF_QUALIFIED)
						&& (pItemStream->flags & WL_IOSF_HAS_BC_SEQ_NUM)
						&& *msgEvent.pSeqNum != wlGetNextSeqNum(pItemStream->seqNum))
				{
					wlSetGapTimer(pWatchlistImpl, pItemStream, WL_IOSF_HAS_BC_SEQ_GAP);
				}
				else
				{
					pItemStream->flags |= WL_IOSF_HAS_BC_SEQ_NUM;
					pItemStream->seqNum = *msgEvent.pSeqNum;
					wlUnsetGapTimer(pWatchlistImpl, pItemStream, WL_IOSF_HAS_BC_SEQ_GAP);

					if ((ret = wlFanoutItemMsgEvent(pWatchlistImpl, pItemStream, &msgEvent, 
									pErrorInfo)) 
							!= RSSL_RET_SUCCESS)
						return ret;
					pWatchlistImpl->items.pCurrentFanoutStream = NULL;
				}
			}

			break;
		}
	}

	return (pWatchlistImpl->base.streamsPendingRequest.count 
			|| pWatchlistImpl->base.newRequests.count) ? 1 : RSSL_RET_SUCCESS;
}

/* Sets gap timer for an item stream. */
static void wlSetGapTimer(RsslWatchlistImpl *pWatchlistImpl, WlItemStream *pItemStream,
		RsslUInt32 flag)
{
	assert(flag == WL_IOSF_HAS_BC_SEQ_GAP || flag == WL_IOSF_HAS_PART_GAP
			|| flag == WL_IOSF_BC_BEHIND_UC);

	if (!(pItemStream->flags & flag))
	{
		/* Add to gap timer queue, if not already in it. */
		if (!(pItemStream->flags & (WL_IOSF_HAS_BC_SEQ_GAP | WL_IOSF_HAS_PART_GAP | WL_IOSF_BC_BEHIND_UC)))
		{
			/* Move back gap timer. */
			pWatchlistImpl->items.gapExpireTime = pWatchlistImpl->base.currentTime + 
				pWatchlistImpl->base.gapTimeout;
			pWatchlistImpl->base.watchlist.state |= RSSLWL_STF_NEED_TIMER;

			rsslQueueAddLinkToBack(&pWatchlistImpl->items.gapStreamQueue,
					&pItemStream->qlGap);
		}

		pItemStream->flags |= flag;
	}
}

/* Unsets gap timer for an item stream. */
static void wlUnsetGapTimer(RsslWatchlistImpl *pWatchlistImpl, WlItemStream *pItemStream,
		RsslUInt32 flag)
{
	assert(flag == WL_IOSF_HAS_BC_SEQ_GAP || flag == WL_IOSF_HAS_PART_GAP
			|| flag == WL_IOSF_BC_BEHIND_UC);

	if (pItemStream->flags & flag)
	{
		pItemStream->flags &= ~flag;

		/* Remove from gap timer queue, if stream no longer needs to be in it. */
		if (!(pItemStream->flags & (WL_IOSF_HAS_BC_SEQ_GAP | WL_IOSF_HAS_PART_GAP | WL_IOSF_BC_BEHIND_UC)))
		{
			rsslQueueRemoveLink(&pWatchlistImpl->items.gapStreamQueue,
					&pItemStream->qlGap);

			/* If all streams have been removed, reset gap timer. */
			if (rsslQueueGetElementCount(&pWatchlistImpl->items.gapStreamQueue) == 0)
				pWatchlistImpl->items.gapExpireTime = WL_TIME_UNSET;
		}
	}
}

/* For multicast reordering.
 * Forward queued messages up to and including the given sequence number. */
static RsslRet wlItemStreamForwardUntil(RsslWatchlistImpl *pWatchlistImpl, 
		WlItemStream *pItemStream, RsslUInt32 seqNum, RsslErrorInfo *pErrorInfo)
{
	WlBufferedMsg *pBufferedMsg;
	RsslWatchlistMsgEvent bufferedMsgEvent;
	RsslRet ret;

	assert(pWatchlistImpl->items.pCurrentFanoutStream);

	while (pBufferedMsg = wlMsgReorderQueuePopUntil(
				&pItemStream->bufferedMsgQueue, seqNum))
	{
		wlMsgEventClear(&bufferedMsgEvent);
		bufferedMsgEvent.pSeqNum = &pBufferedMsg->seqNum;
		bufferedMsgEvent.pRsslMsg = wlBufferedMsgGetRsslMsg(pBufferedMsg);
		if (pBufferedMsg->flags & WL_BFMSG_HAS_FT_GROUP_ID)
			bufferedMsgEvent.pFTGroupId = &pBufferedMsg->ftGroupId;

		ret = wlFanoutItemMsgEvent(pWatchlistImpl, pItemStream, 
				&bufferedMsgEvent, 
				pErrorInfo);

		wlBufferedMsgDestroy(pBufferedMsg);
		if (ret != RSSL_RET_SUCCESS)
			return ret;

		/* If stream was closed, stop. */
		if (pWatchlistImpl->items.pCurrentFanoutStream == NULL)
			return RSSL_RET_SUCCESS;
	}

	return RSSL_RET_SUCCESS;
}

/* For multicast reordering.
 * Forward all remaining queued messages. */
static RsslRet wlItemStreamForwardAllQueued(RsslWatchlistImpl *pWatchlistImpl, 
		WlItemStream *pItemStream, RsslErrorInfo *pErrorInfo)
{
	WlBufferedMsg *pBufferedMsg;
	RsslWatchlistMsgEvent bufferedMsgEvent;
	RsslRet ret;

	assert(pWatchlistImpl->items.pCurrentFanoutStream);

	while (pBufferedMsg = wlMsgReorderQueuePop(
				&pItemStream->bufferedMsgQueue))
	{
		wlMsgEventClear(&bufferedMsgEvent);
		bufferedMsgEvent.pSeqNum = &pBufferedMsg->seqNum;
		bufferedMsgEvent.pRsslMsg = wlBufferedMsgGetRsslMsg(pBufferedMsg);
		if (pBufferedMsg->flags & WL_BFMSG_HAS_FT_GROUP_ID)
			bufferedMsgEvent.pFTGroupId = &pBufferedMsg->ftGroupId;

		ret = wlFanoutItemMsgEvent(pWatchlistImpl, pItemStream, 
				&bufferedMsgEvent, 
				pErrorInfo);

		wlBufferedMsgDestroy(pBufferedMsg);
		if (ret != RSSL_RET_SUCCESS)
			return ret;

		/* If stream was closed, stop. */
		if (pWatchlistImpl->items.pCurrentFanoutStream == NULL)
			return RSSL_RET_SUCCESS;
	}

	return RSSL_RET_SUCCESS;
}


static RsslRet wlItemStreamOrderMsg(RsslWatchlistImpl *pWatchlistImpl, 
		WlItemStream *pItemStream, RsslWatchlistMsgEvent *pEvent, RsslErrorInfo *pErrorInfo)
{
	RsslUInt32 seqNum;
	RsslMsg *pRsslMsg;
	RsslRet ret;
	RsslUInt gapRecoveryEnabled = pWatchlistImpl->base.gapRecovery;

	assert(pEvent->pSeqNum);
	assert(pEvent->pRsslMsg);
	assert(pWatchlistImpl->items.pCurrentFanoutStream == pItemStream);

	pRsslMsg = pEvent->pRsslMsg;
	seqNum = *pEvent->pSeqNum;

	if (pRsslMsg->msgBase.streamId != 0)
	{
		/* Unicast message. */
		if (pItemStream->refreshState != WL_ISRS_PENDING_REFRESH
				&& pItemStream->refreshState != WL_ISRS_PENDING_REFRESH_COMPLETE)
		{
			/* Not reordering messages, forward immediately. */
			ret = wlFanoutItemMsgEvent(pWatchlistImpl, pItemStream, pEvent,
					pErrorInfo);

			pWatchlistImpl->items.pCurrentFanoutStream = NULL;
			return ret;
		}

		/* We have previously used a broadcast message for synchronization. Check
		 * that we did not violate our ordering. */
		if (pItemStream->flags & WL_IOSF_HAS_BC_SYNCH_SEQ_NUM)
		{
			if (rsslSeqNumCompare(seqNum, pItemStream->bcSynchSeqNum) < 0)
			{
				/* This message should have been forwarded before the broadcast message. 
				 * Since this is out of order, close the stream. */
				RsslWatchlistMsgEvent statusMsgEvent;
				RsslStatusMsg statusMsg;

				wlMsgEventClear(&statusMsgEvent);
				statusMsgEvent.pRsslMsg = (RsslMsg*)&statusMsg;
				rsslClearStatusMsg(&statusMsg);
				statusMsg.flags = RSSL_STMF_HAS_STATE | RSSL_STMF_CLEAR_CACHE;
				statusMsg.msgBase.domainType = pItemStream->base.domainType;
				statusMsg.msgBase.containerType = RSSL_DT_NO_DATA;
				rssl_set_buffer_to_string(statusMsg.state.text,
						"Multicast message was forwarded out of order.");
				statusMsg.state.streamState = RSSL_STREAM_CLOSED_RECOVER;
				statusMsg.state.code = RSSL_SC_NONE;
				statusMsgEvent._flags = WL_MEF_SEND_CLOSE;

				if (pItemStream->pFTGroup)
					statusMsgEvent.pFTGroupId = &pItemStream->pFTGroup->ftGroupId;

				ret = wlFanoutItemMsgEvent(pWatchlistImpl, pItemStream, 
						&statusMsgEvent, 
						pErrorInfo);

				pWatchlistImpl->items.pCurrentFanoutStream = NULL;
				return ret;
			}

			/* All is well; play this message as normal. */
			pItemStream->flags &= ~WL_IOSF_HAS_BC_SYNCH_SEQ_NUM;
		}

		if (!(pItemStream->flags & WL_IOSF_HAS_UC_SEQ_NUM))
		{
			/* First unicast message. */
			wlMsgReorderQueueDiscardUntil(&pItemStream->bufferedMsgQueue, seqNum);

			pItemStream->flags |= WL_IOSF_HAS_UC_SEQ_NUM;
			pItemStream->seqNum = seqNum;

			if (gapRecoveryEnabled)
			{
				RsslBool hasGap;
				RsslUInt32 bcSeqNum = pItemStream->seqNum;

				/* From this point, gaps in the broadcast sequence matter, so clean any
				 * out-of-order messages from the buffer queue and see if the remaining messages
				 * leave a gap. */
				if (wlMsgReorderQueueCheckBroadcastSequence(&pItemStream->bufferedMsgQueue, 
							&bcSeqNum, &hasGap))
				{
					/* Update sequence number based on what's in the queue. */
					pItemStream->flags |= WL_IOSF_HAS_BC_SEQ_NUM;
					pItemStream->seqNum = bcSeqNum;

					if (hasGap)
						wlSetGapTimer(pWatchlistImpl, pItemStream, WL_IOSF_HAS_BC_SEQ_GAP);

				}
			}
			else
			{
				RsslUInt32 bcSeqNum;

				/* Use whatever the last message in the queue is. */
				if (wlMsgReorderQueueGetLastBcSeqNum(&pItemStream->bufferedMsgQueue,
							&bcSeqNum))
				{
					pItemStream->flags |= WL_IOSF_HAS_BC_SEQ_NUM;
					pItemStream->seqNum = bcSeqNum;
				}
			}

			/* First unicast message can always be forwarded immediately. */
			if ((ret = wlFanoutItemMsgEvent(pWatchlistImpl, pItemStream, pEvent,
					pErrorInfo)) != RSSL_RET_SUCCESS)
				return ret;
		}
		else
		{
			RsslInt32 compare;

			/* Have a unicast- or broadcast-based sequence number.
			 * Check if message indicates that we're missing broadcast
			 * messages. */

			/* If the broadcast stream is already known to be behind, buffer this. */
			if (wlMsgReorderQueueHasUnicastMsgs(&pItemStream->bufferedMsgQueue))
			{
				return wlMsgReorderQueuePush(&pItemStream->bufferedMsgQueue, pEvent->pRsslMsg,
						seqNum, pEvent->pFTGroupId, &pWatchlistImpl->base, pErrorInfo);
			}

			/* Forward anything we can up to this point. */
			if ((ret = wlItemStreamForwardUntil(pWatchlistImpl, pItemStream,
					seqNum, pErrorInfo)) != RSSL_RET_SUCCESS)
				return ret;

			/* If stream was closed, stop. */
			if (pWatchlistImpl->items.pCurrentFanoutStream == NULL)
				return RSSL_RET_SUCCESS;

			/* Now check against the current sequence number, to see if we need broadcast to 
			 * catch up. */
			compare = rsslSeqNumCompare(seqNum, pItemStream->seqNum);
			if (compare > 0)
			{
				wlSetGapTimer(pWatchlistImpl, pItemStream, WL_IOSF_BC_BEHIND_UC);
				return wlMsgReorderQueuePush(&pItemStream->bufferedMsgQueue, pEvent->pRsslMsg,
								seqNum, pEvent->pFTGroupId, &pWatchlistImpl->base, pErrorInfo);
			}

			/* Foward this. */
			if ((ret = wlFanoutItemMsgEvent(pWatchlistImpl, pItemStream, pEvent,
					pErrorInfo)) != RSSL_RET_SUCCESS)
				return ret;
		}

		/* If stream was closed, stop. */
		if (pWatchlistImpl->items.pCurrentFanoutStream == NULL)
			return RSSL_RET_SUCCESS;

		if (pItemStream->refreshState == WL_ISRS_NONE)
		{
			/* If ordering is finished (refresh is completed), forward
			 * remaining messages. */
			ret = wlItemStreamForwardAllQueued(pWatchlistImpl, pItemStream,
					pErrorInfo);
		}
		else
			ret = RSSL_RET_SUCCESS;

		pWatchlistImpl->items.pCurrentFanoutStream = NULL;
		return ret;
	}
	else
	{
		/* Broadcast message. */
		if (seqNum == 0)
		{
			if (pItemStream->refreshState == WL_ISRS_PENDING_REFRESH
					|| pItemStream->refreshState == WL_ISRS_PENDING_REFRESH_COMPLETE)
			{
				RsslWatchlistMsgEvent statusMsgEvent;
				RsslStatusMsg statusMsg;

				/* Stream was reset while we were waiting for our refresh.
				 * This likely means the refresh will not complete,
				 * so close the stream (and recover if appropriate). */
				wlMsgEventClear(&statusMsgEvent);
				statusMsgEvent.pRsslMsg = (RsslMsg*)&statusMsg;
				rsslClearStatusMsg(&statusMsg);
				statusMsg.flags = RSSL_STMF_HAS_STATE | RSSL_STMF_CLEAR_CACHE;
				statusMsg.msgBase.domainType = pItemStream->base.domainType;
				statusMsg.msgBase.containerType = RSSL_DT_NO_DATA;
				rssl_set_buffer_to_string(statusMsg.state.text,
						"Stream sequence was reset while waiting for refresh.");
				statusMsg.state.streamState = RSSL_STREAM_CLOSED_RECOVER;
				statusMsg.state.code = RSSL_SC_NONE;
				statusMsgEvent._flags = WL_MEF_SEND_CLOSE;

				if (pItemStream->pFTGroup)
					statusMsgEvent.pFTGroupId = &pItemStream->pFTGroup->ftGroupId;

				ret = wlFanoutItemMsgEvent(pWatchlistImpl, pItemStream, 
						&statusMsgEvent, 
						pErrorInfo);
				pWatchlistImpl->items.pCurrentFanoutStream = NULL;
				return ret;
			}
			else
			{
				/* Forward message. */
				pItemStream->flags |= WL_IOSF_HAS_BC_SEQ_NUM;
				pItemStream->seqNum = 0;
				ret = wlFanoutItemMsgEvent(pWatchlistImpl, pItemStream, pEvent,
						pErrorInfo);
				pWatchlistImpl->items.pCurrentFanoutStream = NULL;
				return ret;
			}
		}

		/* Broadcast status messages are treated as synchronization points while reordering. */
		if (pRsslMsg->msgBase.msgClass == RSSL_MC_STATUS
				&& (pItemStream->refreshState == WL_ISRS_PENDING_REFRESH
					|| pItemStream->refreshState == WL_ISRS_PENDING_REFRESH_COMPLETE))
			return wlItemStreamOrderBroadcastSynchMsg(pWatchlistImpl, pItemStream,
					pEvent, pErrorInfo);

		if (!(pItemStream->flags & WL_IOSF_HAS_UC_SEQ_NUM))
		{
			/* Have not received a sequenced unicast message, so don't know
			 * where to start yet. Buffer this message. */
			return wlMsgReorderQueuePush(&pItemStream->bufferedMsgQueue, pEvent->pRsslMsg,
					seqNum, pEvent->pFTGroupId, &pWatchlistImpl->base, pErrorInfo);
		}

		if (!(pItemStream->flags & WL_IOSF_HAS_BC_SEQ_NUM))
		{
			/* Check against unicast stream. */
			if (rsslSeqNumCompare(seqNum, pItemStream->seqNum) <= 0)
			{
				/* Set timer if broadcast stream is behind. Normally this would
				 * not be important yet, however the discrepancy may be due
				 * to a takeover by a another ADS, and so may indicate that
				 * we missed the reset of the sequence number. */
				if (rsslSeqNumCompare(seqNum, pItemStream->seqNum) < 0)
					wlSetGapTimer(pWatchlistImpl, pItemStream, WL_IOSF_BC_BEHIND_UC);
				else
					wlUnsetGapTimer(pWatchlistImpl, pItemStream, WL_IOSF_BC_BEHIND_UC);

				return RSSL_RET_SUCCESS; /* Discard either way. */
			}

			/* If no unicast messages buffered, broadcast stream is up to date. */
			if (!wlMsgReorderQueueHasUnicastMsgs(&pItemStream->bufferedMsgQueue))
				wlUnsetGapTimer(pWatchlistImpl, pItemStream, WL_IOSF_BC_BEHIND_UC);

			/* Mark that we have crossed the starting point (using the number
			 * previously set by the first unicast message). */
			pItemStream->flags |= WL_IOSF_HAS_BC_SEQ_NUM;


		}

		/* Check for gaps in the broadcast stream, if appropriate. */
		if (gapRecoveryEnabled && seqNum != wlGetNextSeqNum(pItemStream->seqNum))
		{
			wlSetGapTimer(pWatchlistImpl, pItemStream, WL_IOSF_HAS_BC_SEQ_GAP);
			return RSSL_RET_SUCCESS;
		}
		else
		{
			wlUnsetGapTimer(pWatchlistImpl, pItemStream, WL_IOSF_HAS_BC_SEQ_GAP);
			pItemStream->seqNum = seqNum;
		}

		if (pItemStream->refreshState == WL_ISRS_NONE)
		{
			/* Broadcast sequence start is established and we're not reordering,
			 * so forward this message. */
			ret = wlFanoutItemMsgEvent(pWatchlistImpl, pItemStream, pEvent,
					pErrorInfo);

			pWatchlistImpl->items.pCurrentFanoutStream = NULL;
			return ret;
		}

		/* Check if this message catches the broadcast stream up with the
		 * unicast stream. If so, forward messages in the appropriate order. */
		if (wlMsgReorderQueueHasUnicastMsgs(&pItemStream->bufferedMsgQueue))
		{
			/* Forward anything we have before (but not including) this
			 * sequence number. At this point, only unicast messages should
			 * be queued. */
			if ((ret = wlItemStreamForwardUntil(pWatchlistImpl, pItemStream,
							wlGetPrevSeqNum(seqNum), pErrorInfo))
					!= RSSL_RET_SUCCESS)
				return ret;

			/* If stream was closed, stop. */
			if (pWatchlistImpl->items.pCurrentFanoutStream == NULL)
				return RSSL_RET_SUCCESS;

			if (wlMsgReorderQueueHasUnicastMsgs(&pItemStream->bufferedMsgQueue))
			{
				/* Still have unicast messages queued, so forward this. */
				if ((ret = wlFanoutItemMsgEvent(pWatchlistImpl, pItemStream, pEvent,
								pErrorInfo)) != RSSL_RET_SUCCESS)
					return ret;

				/* If stream was closed, stop. */
				if (pWatchlistImpl->items.pCurrentFanoutStream == NULL)
					return RSSL_RET_SUCCESS;

				/* Foward any remaining messages we can (which at this point
				 * should be unicast messages with the same sequence number). */
				if ((ret = wlItemStreamForwardUntil(pWatchlistImpl, pItemStream,
								seqNum, pErrorInfo))
						!= RSSL_RET_SUCCESS)
					return ret;

				/* If stream was closed, stop. */
				if (pWatchlistImpl->items.pCurrentFanoutStream == NULL)
					return RSSL_RET_SUCCESS;

				pWatchlistImpl->items.pCurrentFanoutStream = NULL;
			}
			else
			{
				/* Buffer this message. */
				if ((ret = wlMsgReorderQueuePush(&pItemStream->bufferedMsgQueue, pEvent->pRsslMsg,
								seqNum, pEvent->pFTGroupId, &pWatchlistImpl->base, pErrorInfo))
						!= RSSL_RET_SUCCESS)
					return ret;
			}

			/* Check if broadcast stream has caught up with unicast stream. */
			if (!wlMsgReorderQueueHasUnicastMsgs(&pItemStream->bufferedMsgQueue))
				wlUnsetGapTimer(pWatchlistImpl, pItemStream, WL_IOSF_BC_BEHIND_UC);

			return RSSL_RET_SUCCESS;

		}

		/* Still reordering, but have no unicast messages buffered so we don't
		 * know if we can send this out.  Buffer this message. */
		return wlMsgReorderQueuePush(&pItemStream->bufferedMsgQueue, pEvent->pRsslMsg,
				seqNum, pEvent->pFTGroupId, &pWatchlistImpl->base, pErrorInfo);
	}
}

static RsslRet wlItemStreamOrderBroadcastSynchMsg(RsslWatchlistImpl *pWatchlistImpl, 
		WlItemStream *pItemStream, RsslWatchlistMsgEvent *pEvent, RsslErrorInfo *pErrorInfo)
{
	RsslUInt32 seqNum;
	RsslMsg *pRsslMsg;
	RsslRet ret;
	RsslUInt gapRecoveryEnabled = pWatchlistImpl->base.gapRecovery;

	assert(pEvent->pSeqNum);
	assert(pEvent->pRsslMsg);
	assert(pEvent->pRsslMsg->msgBase.streamId == 0);
	assert(pWatchlistImpl->items.pCurrentFanoutStream == pItemStream);

	pRsslMsg = pEvent->pRsslMsg;
	seqNum = *pEvent->pSeqNum;

	if (!(pItemStream->flags & WL_IOSF_HAS_UC_SEQ_NUM))
	{
		/* No unicast message received; clear out the queue and forward this message. */
		wlMsgReorderQueueDiscardAllMessages(&pItemStream->bufferedMsgQueue);

		/* Indicate that a broadcast message was used for synchronization.
		 * Don't set broadcast sequence number since unicast stream hasn't started. */
		pItemStream->flags |= WL_IOSF_HAS_BC_SYNCH_SEQ_NUM;
		pItemStream->bcSynchSeqNum = seqNum;

		ret = wlFanoutItemMsgEvent(pWatchlistImpl, pItemStream, pEvent,
				pErrorInfo);
		pWatchlistImpl->items.pCurrentFanoutStream = NULL;
		return ret;
	}

	if (!(pItemStream->flags & WL_IOSF_HAS_BC_SEQ_NUM))
	{
		/* Check against unicast stream. */
		if (rsslSeqNumCompare(seqNum, pItemStream->seqNum) <= 0)
		{
			/* Set timer if broadcast stream is behind. Normally this would
			 * not be important yet, however the discrepancy may be due
			 * to a takeover by a another ADS, and so may indicate that
			 * we missed the reset of the sequence number. */
			if (rsslSeqNumCompare(seqNum, pItemStream->seqNum) < 0)
				wlSetGapTimer(pWatchlistImpl, pItemStream, WL_IOSF_BC_BEHIND_UC);
			else
				wlUnsetGapTimer(pWatchlistImpl, pItemStream, WL_IOSF_BC_BEHIND_UC);

			return RSSL_RET_SUCCESS; /* Discard either way. */
		}

		/* If no unicast messages buffered, broadcast stream is up to date. */
		if (!wlMsgReorderQueueHasUnicastMsgs(&pItemStream->bufferedMsgQueue))
			wlUnsetGapTimer(pWatchlistImpl, pItemStream, WL_IOSF_BC_BEHIND_UC);

		/* Mark that we have crossed the starting point (using the number
		 * previously set by the first unicast message). */
		pItemStream->flags |= WL_IOSF_HAS_BC_SEQ_NUM;
	}


	/* Check for gaps if we've already established a broadcast stream. */
	if (pItemStream->flags & WL_IOSF_HAS_BC_SEQ_NUM)
	{
		if (gapRecoveryEnabled && seqNum != wlGetNextSeqNum(pItemStream->seqNum))
		{
			wlSetGapTimer(pWatchlistImpl, pItemStream, WL_IOSF_HAS_BC_SEQ_GAP);
			return RSSL_RET_SUCCESS;
		}
		else
		{
			wlUnsetGapTimer(pWatchlistImpl, pItemStream, WL_IOSF_HAS_BC_SEQ_GAP);
			pItemStream->seqNum = seqNum;
		}
	}

	/* Set that we have used this message to synchronize broadcast. */
	pItemStream->flags |= WL_IOSF_HAS_BC_SYNCH_SEQ_NUM;
	pItemStream->bcSynchSeqNum = seqNum;

	/* Check for any queued messages that should be forwarded. */
	if (!wlMsgReorderQueueHasUnicastMsgs(&pItemStream->bufferedMsgQueue))
	{
		/* Forward any broadcast messages up to this point. */
		if ((ret = wlItemStreamForwardUntil(pWatchlistImpl, pItemStream, seqNum, pErrorInfo))
				!= RSSL_RET_SUCCESS)
			return ret;

		/* If stream was closed, stop. */
		if (pWatchlistImpl->items.pCurrentFanoutStream == NULL)
			return RSSL_RET_SUCCESS;

		/* Forward this. */
		ret = wlFanoutItemMsgEvent(pWatchlistImpl, pItemStream, pEvent,
				pErrorInfo);
		pWatchlistImpl->items.pCurrentFanoutStream = NULL;
		return ret;
	}
	else
	{
		/* Forward anything we have before (but not including) this
		 * sequence number. At this point, only unicast messages should
		 * be queued. */
		if ((ret = wlItemStreamForwardUntil(pWatchlistImpl, pItemStream,
						wlGetPrevSeqNum(seqNum), pErrorInfo))
				!= RSSL_RET_SUCCESS)
			return ret;

		/* If stream was closed, stop. */
		if (pWatchlistImpl->items.pCurrentFanoutStream == NULL)
			return RSSL_RET_SUCCESS;

		/* Forward this. */
		if ((ret = wlFanoutItemMsgEvent(pWatchlistImpl, pItemStream, pEvent,
						pErrorInfo)) != RSSL_RET_SUCCESS)
			return ret;

		/* If stream was closed, stop. */
		if (pWatchlistImpl->items.pCurrentFanoutStream == NULL)
			return RSSL_RET_SUCCESS;

		/* Foward any remaining messages we can up to this point (which 
		 * should just be unicast messages with the same sequence number). */
		if ((ret = wlItemStreamForwardUntil(pWatchlistImpl, pItemStream,
						seqNum, pErrorInfo))
				!= RSSL_RET_SUCCESS)
			return ret;

		/* If stream was closed, stop. */
		if (pWatchlistImpl->items.pCurrentFanoutStream == NULL)
			return RSSL_RET_SUCCESS;

		/* Check if broadcast stream has caught up with unicast stream. */
		if (!wlMsgReorderQueueHasUnicastMsgs(&pItemStream->bufferedMsgQueue))
			wlUnsetGapTimer(pWatchlistImpl, pItemStream, WL_IOSF_BC_BEHIND_UC);

		pWatchlistImpl->items.pCurrentFanoutStream = NULL;
		return RSSL_RET_SUCCESS;
	}
}

static RsslRet wlFanoutItemMsgEvent(RsslWatchlistImpl *pWatchlistImpl, WlItemStream *pItemStream,
		RsslWatchlistMsgEvent *pEvent, RsslErrorInfo *pErrorInfo)
{
	RsslState *pState;
	RsslMsg *pRsslMsg = pEvent->pRsslMsg;
	RsslQueueLink *pLink;
	RsslQueue requestsRecovering;
	RsslRet ret;

	RsslBool isPrivate;
	RsslQos qos;

	RsslUInt8 tmpStreamState;
	RsslUInt8	tmpDataState;

	assert(pEvent);
	assert(pRsslMsg);

	pWatchlistImpl->items.pCurrentFanoutStream = pItemStream;

	/* Check if this item needs to be added to an FTGroup. */
	if (pEvent->pFTGroupId && !(pItemStream->pFTGroup))
	{
		if ((ret = wlFTGroupAddStream(&pWatchlistImpl->base, &pWatchlistImpl->items, 
						*pEvent->pFTGroupId, pItemStream, pErrorInfo)) != RSSL_RET_SUCCESS)
			return ret;
	}


	switch(pRsslMsg->msgBase.msgClass)
	{
		case RSSL_MC_UPDATE:

			RSSL_QUEUE_FOR_EACH_LINK(&pItemStream->requestsOpen, pLink)
			{
				WlItemRequest *pItemRequest = 
					RSSL_QUEUE_LINK_TO_OBJECT(WlItemRequest, base.qlStateQueue, pLink);
				if ((ret = wlSendMsgEventToItemRequest(pWatchlistImpl, pEvent, pItemRequest, pErrorInfo))
						!= RSSL_RET_SUCCESS)
					return ret;
			}

			if (pItemStream->refreshState == WL_ISRS_PENDING_REFRESH_COMPLETE)
				RSSL_QUEUE_FOR_EACH_LINK(&pItemStream->requestsPendingRefresh, pLink)
				{
					WlItemRequest *pItemRequest = 
						RSSL_QUEUE_LINK_TO_OBJECT(WlItemRequest, base.qlStateQueue, pLink);
					if ((ret = wlSendMsgEventToItemRequest(pWatchlistImpl, pEvent, pItemRequest, pErrorInfo))
							!= RSSL_RET_SUCCESS)
						return ret;
				}

			if (pWatchlistImpl->items.pCurrentFanoutStream != NULL) return RSSL_RET_SUCCESS;
			else return wlItemStreamClose(&pWatchlistImpl->base, &pWatchlistImpl->items,
					pItemStream, pErrorInfo);

		case RSSL_MC_REFRESH:
		{
			RsslRefreshMsg *pRefreshMsg = &pRsslMsg->refreshMsg;
			pState = &pRefreshMsg->state;

			if (pRefreshMsg->flags & RSSL_RFMF_SOLICITED
					&& pItemStream->refreshState != WL_ISRS_NONE)
			{
				if (pRefreshMsg->flags & RSSL_RFMF_HAS_PART_NUM)
				{
					if (pWatchlistImpl->base.gapRecovery)
					{
						if (pRefreshMsg->partNum == 0)
						{
							/* Allow partNum to be reset. */
							wlUnsetGapTimer(pWatchlistImpl, pItemStream, WL_IOSF_HAS_PART_GAP);
							pItemStream->nextPartNum = 1;
						}
						else if (pRefreshMsg->partNum != pItemStream->nextPartNum)
						{
							wlSetGapTimer(pWatchlistImpl, pItemStream, WL_IOSF_HAS_PART_GAP);

							/* Throw the message away. */
							return RSSL_RET_SUCCESS;
						}
						else
						{
							wlUnsetGapTimer(pWatchlistImpl, pItemStream, WL_IOSF_HAS_PART_GAP);
							++pItemStream->nextPartNum;
						}
					}
				}

				if (pRefreshMsg->flags & RSSL_RFMF_REFRESH_COMPLETE)
				{
					/* Refresh completed, stop timeout. */
					if (pWatchlistImpl->base.pRsslChannel)
						wlUnsetStreamPendingResponse(&pWatchlistImpl->base, &pItemStream->base);

					wlItemStreamProcessRefreshComplete(&pWatchlistImpl->base, pItemStream);
				}
				else
				{
					/* Received refresh part, reset timeout. */
					if (pWatchlistImpl->base.pRsslChannel)
						wlResetStreamPendingResponse(&pWatchlistImpl->base, &pItemStream->base);
					pItemStream->refreshState = WL_ISRS_PENDING_REFRESH_COMPLETE;
				}
			}

			if (!pWatchlistImpl->base.config.allowSuspectData
					&& pRefreshMsg->state.dataState == RSSL_DATA_SUSPECT
					&& pRefreshMsg->state.streamState == RSSL_STREAM_OPEN)
			{
				pEvent->_flags |= WL_MEF_SEND_CLOSE;
				pState = &pRefreshMsg->state;
				pState->streamState = RSSL_STREAM_CLOSED_RECOVER;
				break;
			}

			switch(pRefreshMsg->state.streamState)
			{
				case RSSL_STREAM_OPEN:

					if ((ret = wlItemGroupAddStream(&pWatchlistImpl->items, &pRsslMsg->refreshMsg.groupId, pItemStream,
									pErrorInfo))
							!= RSSL_RET_SUCCESS)
						return ret;

					if (pRefreshMsg->flags & RSSL_RFMF_REFRESH_COMPLETE
							&& pRefreshMsg->state.dataState == RSSL_DATA_OK)
						pItemStream->flags |= WL_IOSF_ESTABLISHED;
					else if (pRefreshMsg->state.dataState != RSSL_DATA_OK)
						pItemStream->flags &= ~WL_IOSF_ESTABLISHED;

					if (!(pRefreshMsg->flags & RSSL_RFMF_SOLICITED))
					{
						RSSL_QUEUE_FOR_EACH_LINK(&pItemStream->requestsOpen, pLink)
						{
							WlItemRequest *pItemRequest = 
								RSSL_QUEUE_LINK_TO_OBJECT(WlItemRequest, base.qlStateQueue, pLink);
							if ((ret = wlSendMsgEventToItemRequest(pWatchlistImpl, pEvent, pItemRequest, pErrorInfo))
									!= RSSL_RET_SUCCESS)
								return ret;
						}
					}
					else if (pItemStream->flags & WL_IOSF_PENDING_VIEW_REFRESH)
					{
						if (pRefreshMsg->flags & RSSL_RFMF_REFRESH_COMPLETE)
							pItemStream->flags &= ~WL_IOSF_PENDING_VIEW_REFRESH;

						/* If refresh is solicited and we are waiting on a view,
						 * open requests must also receive it (as unsolicted). */
						RSSL_QUEUE_FOR_EACH_LINK(&pItemStream->requestsOpen, pLink)
						{
							WlItemRequest *pItemRequest = 
								RSSL_QUEUE_LINK_TO_OBJECT(WlItemRequest, base.qlStateQueue, pLink);
							if ((ret = wlSendRefreshEventToItemRequest(pWatchlistImpl, pEvent, pItemRequest, pErrorInfo))
									!= RSSL_RET_SUCCESS)
								return ret;
						}

					}

					if (pRefreshMsg->flags & RSSL_RFMF_SOLICITED
							|| pItemStream->refreshState == WL_ISRS_PENDING_REFRESH_COMPLETE)
					{
						RSSL_QUEUE_FOR_EACH_LINK(&pItemStream->requestsPendingRefresh, pLink)
						{
							WlItemRequest *pItemRequest = 
								RSSL_QUEUE_LINK_TO_OBJECT(WlItemRequest, base.qlStateQueue, pLink);

							if (pItemRequest->requestMsgFlags & RSSL_RQMF_STREAMING)
							{
								if (pRefreshMsg->flags & RSSL_RFMF_REFRESH_COMPLETE)
								{
									wlItemRequestEstablishQos(pItemRequest, 
											&pItemStream->streamAttributes.qos);

									pItemRequest->base.pStateQueue = &pItemStream->requestsOpen;
									rsslQueueRemoveLink(&pItemStream->requestsPendingRefresh, pLink);
									rsslQueueAddLinkToBack(&pItemStream->requestsOpen, pLink);
								}

								if ((ret = wlSendRefreshEventToItemRequest(pWatchlistImpl, pEvent, pItemRequest, pErrorInfo))
										!= RSSL_RET_SUCCESS)
									return ret;
							}
							else
							{
								pRsslMsg->refreshMsg.state.streamState = RSSL_STREAM_NON_STREAMING;

								if (pRefreshMsg->flags & RSSL_RFMF_REFRESH_COMPLETE)
									wlItemRequestClose(&pWatchlistImpl->base, &pWatchlistImpl->items, 
											pItemRequest);

								if ((ret = wlSendRefreshEventToItemRequest(pWatchlistImpl, pEvent, pItemRequest, pErrorInfo))
										!= RSSL_RET_SUCCESS)
									return ret;

								if (pRefreshMsg->flags & RSSL_RFMF_REFRESH_COMPLETE)
								{
									wlRequestedServiceCheckRefCount(&pWatchlistImpl->base, pItemRequest->pRequestedService);
									wlDestroyItemRequest(pWatchlistImpl, pItemRequest,
											pErrorInfo);
								}

								pRsslMsg->refreshMsg.state.streamState = RSSL_STREAM_OPEN;


							}

						}

						/* Any requests waiting for current refresh completion can start. */
						if (pRefreshMsg->flags & RSSL_RFMF_REFRESH_COMPLETE)
						{
							if (pItemStream->requestsRecovering.count)
							{
								wlItemStreamSetMsgPending(&pWatchlistImpl->base, pItemStream, RSSL_TRUE);
							}
							else if (!pItemStream->requestsStreamingCount)
							{
								/* Stream is done; close it. Send a close
								 * since stream appears to be open. */
								pWatchlistImpl->items.pCurrentFanoutStream = NULL;
							}

							if (pWatchlistImpl->items.pCurrentFanoutStream != NULL) return RSSL_RET_SUCCESS;
							else return wlItemStreamClose(&pWatchlistImpl->base, &pWatchlistImpl->items,
									pItemStream, pErrorInfo);
						}
					}

					if (pWatchlistImpl->items.pCurrentFanoutStream != NULL) return RSSL_RET_SUCCESS;
					else return wlItemStreamClose(&pWatchlistImpl->base, &pWatchlistImpl->items,
							pItemStream, pErrorInfo);

				case RSSL_STREAM_NON_STREAMING:

					pItemStream->flags &= ~WL_IOSF_ESTABLISHED;

					if (pItemStream->flags & WL_IOSF_PENDING_SNAPSHOT)
					{
						if (pRefreshMsg->flags & RSSL_RFMF_SOLICITED
								|| pItemStream->refreshState == WL_ISRS_PENDING_REFRESH_COMPLETE)
						{
							RSSL_QUEUE_FOR_EACH_LINK(&pItemStream->requestsPendingRefresh, pLink)
							{
								WlItemRequest *pItemRequest = 
									RSSL_QUEUE_LINK_TO_OBJECT(WlItemRequest, base.qlStateQueue, pLink);

								assert(!(pItemRequest->requestMsgFlags & RSSL_RQMF_STREAMING));

								if (pRefreshMsg->flags & RSSL_RFMF_REFRESH_COMPLETE)
									wlItemRequestClose(&pWatchlistImpl->base, &pWatchlistImpl->items, 
											pItemRequest);

								if ((ret = wlSendRefreshEventToItemRequest(pWatchlistImpl, pEvent, pItemRequest, pErrorInfo))
										!= RSSL_RET_SUCCESS)
									return ret;

								if (pRefreshMsg->flags & RSSL_RFMF_REFRESH_COMPLETE)
								{
									wlRequestedServiceCheckRefCount(&pWatchlistImpl->base, pItemRequest->pRequestedService);
									wlDestroyItemRequest(pWatchlistImpl, pItemRequest,
											pErrorInfo);
								}
							}

							/* Any requests waiting for current refresh completion can start. */
							if (pRefreshMsg->flags & RSSL_RFMF_REFRESH_COMPLETE)
							{
								pItemStream->flags &= ~WL_IOSF_PENDING_SNAPSHOT;
								if (pItemStream->requestsRecovering.count)
								{
									/* Reset the stream since it is being freshly opened. */
									wlItemStreamResetState(pItemStream);

									wlItemStreamSetMsgPending(&pWatchlistImpl->base, pItemStream, RSSL_TRUE);
								}
								else
								{
									/* Stream is done; close it. */
									pWatchlistImpl->items.pCurrentFanoutStream = NULL;
									pItemStream->flags |= WL_IOSF_CLOSED;
								}
							}
						}

						if (pWatchlistImpl->items.pCurrentFanoutStream != NULL) return RSSL_RET_SUCCESS;
						else return wlItemStreamClose(&pWatchlistImpl->base, &pWatchlistImpl->items,
								pItemStream, pErrorInfo);

					}
					else
					{
						/* Non-streaming refresh to streaming request. */
						RSSL_QUEUE_FOR_EACH_LINK(&pItemStream->requestsPendingRefresh, pLink)
						{
							WlItemRequest *pItemRequest = 
								RSSL_QUEUE_LINK_TO_OBJECT(WlItemRequest, base.qlStateQueue, pLink);

							if (pRefreshMsg->flags & RSSL_RFMF_REFRESH_COMPLETE)
								wlItemRequestClose(&pWatchlistImpl->base, &pWatchlistImpl->items, 
										pItemRequest);

							if ((ret = wlSendRefreshEventToItemRequest(pWatchlistImpl, pEvent, pItemRequest, pErrorInfo))
									!= RSSL_RET_SUCCESS)
								return ret;

							if (pRefreshMsg->flags & RSSL_RFMF_REFRESH_COMPLETE)
							{
								wlRequestedServiceCheckRefCount(&pWatchlistImpl->base, pItemRequest->pRequestedService);
								wlDestroyItemRequest(pWatchlistImpl, pItemRequest,
										pErrorInfo);
							}
						}

						RSSL_QUEUE_FOR_EACH_LINK(&pItemStream->requestsOpen, pLink)
						{
							WlItemRequest *pItemRequest = 
								RSSL_QUEUE_LINK_TO_OBJECT(WlItemRequest, base.qlStateQueue, pLink);

							if (pRefreshMsg->flags & RSSL_RFMF_REFRESH_COMPLETE)
							{
								wlRequestedServiceCheckRefCount(&pWatchlistImpl->base, pItemRequest->pRequestedService);
								wlItemRequestClose(&pWatchlistImpl->base, &pWatchlistImpl->items, 
										pItemRequest);
							}

							if ((ret = wlSendRefreshEventToItemRequest(pWatchlistImpl, pEvent, pItemRequest, pErrorInfo))
									!= RSSL_RET_SUCCESS)
								return ret;

							if (pRefreshMsg->flags & RSSL_RFMF_REFRESH_COMPLETE)
								wlDestroyItemRequest(pWatchlistImpl, pItemRequest,
										pErrorInfo);
						}

						if (pRefreshMsg->flags & RSSL_RFMF_REFRESH_COMPLETE)
						{
							pWatchlistImpl->items.pCurrentFanoutStream = NULL;
							pItemStream->flags |= WL_IOSF_CLOSED;
						}

						if (pWatchlistImpl->items.pCurrentFanoutStream != NULL) return RSSL_RET_SUCCESS;
						else return wlItemStreamClose(&pWatchlistImpl->base, &pWatchlistImpl->items,
								pItemStream, pErrorInfo);
					}
				default:
					break;
			}

			break;
		}

		case RSSL_MC_STATUS:
		{
			RsslStatusMsg *pStatusMsg = &pRsslMsg->statusMsg;

			if (pStatusMsg->flags & RSSL_STMF_HAS_GROUP_ID)
				if ((ret = wlItemGroupAddStream(&pWatchlistImpl->items, &pRsslMsg->refreshMsg.groupId, pItemStream,
								pErrorInfo))
						!= RSSL_RET_SUCCESS)
					return ret;

			if (!(pStatusMsg->flags & RSSL_STMF_HAS_STATE))
			{
				RSSL_QUEUE_FOR_EACH_LINK(&pItemStream->requestsOpen, pLink)
				{
					WlItemRequest *pItemRequest = 
						RSSL_QUEUE_LINK_TO_OBJECT(WlItemRequest, base.qlStateQueue, pLink);
					if ((ret = wlSendMsgEventToItemRequest(pWatchlistImpl, pEvent, pItemRequest, pErrorInfo))
							!= RSSL_RET_SUCCESS)
						return ret;
				}

				RSSL_QUEUE_FOR_EACH_LINK(&pItemStream->requestsPendingRefresh, pLink)
				{
					WlItemRequest *pItemRequest = 
						RSSL_QUEUE_LINK_TO_OBJECT(WlItemRequest, base.qlStateQueue, pLink);
					if ((ret = wlSendMsgEventToItemRequest(pWatchlistImpl, pEvent, pItemRequest, pErrorInfo))
							!= RSSL_RET_SUCCESS)
						return ret;
				}

				if (pWatchlistImpl->items.pCurrentFanoutStream != NULL) return RSSL_RET_SUCCESS;
				else return wlItemStreamClose(&pWatchlistImpl->base, &pWatchlistImpl->items,
						pItemStream, pErrorInfo);
			}
			else if (pStatusMsg->state.streamState == RSSL_STREAM_OPEN)
			{
				if (!pWatchlistImpl->base.config.allowSuspectData
						&& pStatusMsg->state.dataState == RSSL_DATA_SUSPECT
						&& pStatusMsg->state.streamState == RSSL_STREAM_OPEN)
				{
					pEvent->_flags |= WL_MEF_SEND_CLOSE;
					pState = &pStatusMsg->state;
					pState->streamState = RSSL_STREAM_CLOSED_RECOVER;
					break;
				}

				if (pStatusMsg->state.dataState == RSSL_DATA_OK)
					pItemStream->flags |= WL_IOSF_ESTABLISHED;
				else
					pItemStream->flags &= ~WL_IOSF_ESTABLISHED;

				RSSL_QUEUE_FOR_EACH_LINK(&pItemStream->requestsOpen, pLink)
				{
					WlItemRequest *pItemRequest = 
						RSSL_QUEUE_LINK_TO_OBJECT(WlItemRequest, base.qlStateQueue, pLink);
					if ((ret = wlSendMsgEventToItemRequest(pWatchlistImpl, pEvent, pItemRequest, pErrorInfo))
							!= RSSL_RET_SUCCESS)
						return ret;
				}

				RSSL_QUEUE_FOR_EACH_LINK(&pItemStream->requestsPendingRefresh, pLink)
				{
					WlItemRequest *pItemRequest = 
						RSSL_QUEUE_LINK_TO_OBJECT(WlItemRequest, base.qlStateQueue, pLink);
					if ((ret = wlSendMsgEventToItemRequest(pWatchlistImpl, pEvent, pItemRequest, pErrorInfo))
							!= RSSL_RET_SUCCESS)
						return ret;
				}

				if (pWatchlistImpl->items.pCurrentFanoutStream != NULL) return RSSL_RET_SUCCESS;
				else return wlItemStreamClose(&pWatchlistImpl->base, &pWatchlistImpl->items,
						pItemStream, pErrorInfo);
			}
			else
				pState = &pStatusMsg->state;

			break;
		}

		case RSSL_MC_ACK:
		{
			WlPostRecord *pRecord;

			if ((pRecord = wlPostTableFindRecord(&pWatchlistImpl->base.postTable, 
						&pRsslMsg->ackMsg)))
			{
				WlItemRequest *pItemRequest = (WlItemRequest*)pRecord->pUserSpec;

				rsslQueueRemoveLink(&pItemRequest->base.openPosts, &pRecord->qlUser);
				wlPostTableRemoveRecord(&pWatchlistImpl->base.postTable, pRecord);

				if ((ret = wlSendMsgEventToItemRequest(pWatchlistImpl, pEvent, pItemRequest, pErrorInfo))
							!= RSSL_RET_SUCCESS)
						return ret;
			}
			/* Otherwise ignore it. */

			return RSSL_RET_SUCCESS;
		}

		default:
			RSSL_QUEUE_FOR_EACH_LINK(&pItemStream->requestsOpen, pLink)
			{
				WlItemRequest *pItemRequest = 
					RSSL_QUEUE_LINK_TO_OBJECT(WlItemRequest, base.qlStateQueue, pLink);
				if ((ret = wlSendMsgEventToItemRequest(pWatchlistImpl, pEvent, pItemRequest, pErrorInfo))
						!= RSSL_RET_SUCCESS)
					return ret;
			}

			RSSL_QUEUE_FOR_EACH_LINK(&pItemStream->requestsPendingRefresh, pLink)
			{
				WlItemRequest *pItemRequest = 
					RSSL_QUEUE_LINK_TO_OBJECT(WlItemRequest, base.qlStateQueue, pLink);
				if ((ret = wlSendMsgEventToItemRequest(pWatchlistImpl, pEvent, pItemRequest, pErrorInfo))
						!= RSSL_RET_SUCCESS)
					return ret;
			}

			if (pWatchlistImpl->items.pCurrentFanoutStream != NULL) return RSSL_RET_SUCCESS;
			else return wlItemStreamClose(&pWatchlistImpl->base, &pWatchlistImpl->items,
					pItemStream, pErrorInfo);
	}

	/* Stream will be closed and requests need to be recovered. */
	assert(pState);

	rsslInitQueue(&requestsRecovering);
	rsslQueueAppend(&requestsRecovering, &pItemStream->requestsPendingRefresh);
	rsslQueueAppend(&requestsRecovering, &pItemStream->requestsOpen);
	rsslQueueAppend(&requestsRecovering, &pItemStream->requestsRecovering);
	isPrivate = (pItemStream->flags & WL_IOSF_PRIVATE) ? RSSL_TRUE : RSSL_FALSE;
	qos = pItemStream->streamAttributes.qos;

	if (!(pEvent->_flags & WL_MEF_SEND_CLOSE))
		pItemStream->flags |= WL_IOSF_CLOSED;
	wlItemStreamClose(&pWatchlistImpl->base, &pWatchlistImpl->items,
			pItemStream, pErrorInfo);

	tmpStreamState = pState->streamState;
	tmpDataState = pState->dataState;


	RSSL_QUEUE_FOR_EACH_LINK(&requestsRecovering, pLink)
	{
		WlItemRequest *pItemRequest = 
			RSSL_QUEUE_LINK_TO_OBJECT(WlItemRequest, base.qlStateQueue, pLink);

		pItemRequest->base.pStream = NULL;

		rsslQueueRemoveLink(&requestsRecovering, 
				&pItemRequest->base.qlStateQueue);
		pItemRequest->base.pStateQueue = NULL;
		if (pItemRequest->pView) pItemRequest->pView->pParentQueue = NULL;

		/* Clean up any posts awaiting acknowledgement. */
		while (pLink = rsslQueueRemoveFirstLink(&pItemRequest->base.openPosts))
		{
			WlPostRecord *pPostRecord = RSSL_QUEUE_LINK_TO_OBJECT(WlPostRecord, qlUser, pLink);
			wlPostTableRemoveRecord(&pWatchlistImpl->base.postTable, pPostRecord);
		}

		if ( 
				/* Don't recover private streams. */
				!isPrivate

				/* Don't recover a dictionary if a full refresh has been received. */
				&& (pItemRequest->base.domainType != RSSL_DMT_DICTIONARY
					|| !(pItemRequest->flags & WL_IRQF_REFRESHED))

				/* A ClosedRecover stream should be retried if single-open is enabled. */
				&& (pWatchlistImpl->base.config.singleOpen && 
					tmpStreamState == RSSL_STREAM_CLOSED_RECOVER)

				/* Don't recover for warm standby feature when others channel doesn't provide the service. */
				&& ((pEvent->_flags & WL_MEF_SEND_CLOSED_RECOVER) == 0 )
		   )
		{
			/* Recoverable item. Change state before fanning out. */

			if (pItemRequest->pRequestedService->pMatchingService)
			{
				rsslQueueAddLinkToBack(&pWatchlistImpl->base.newRequests,
						&pItemRequest->base.qlStateQueue);
				pItemRequest->base.pStateQueue = &pWatchlistImpl->base.newRequests;
			}
			else
			{
				/* If the stream was closed because the service was removed,
				 * move the request straight to the recovery queue rather than
				 * try to re-request (saves time and avoids a redundant status msg). */
				rsslQueueAddLinkToBack(&pItemRequest->pRequestedService->recoveringList, 
						&pItemRequest->base.qlStateQueue);
				pItemRequest->base.pStateQueue = &pItemRequest->pRequestedService->recoveringList;
			}

			pState->streamState = RSSL_STREAM_OPEN;
			pState->dataState = RSSL_DATA_SUSPECT;
			if (pRsslMsg->msgBase.msgClass == RSSL_MC_REFRESH)
			{
				if ((ret = wlSendRefreshEventToItemRequest(pWatchlistImpl, pEvent, pItemRequest, pErrorInfo))
						!= RSSL_RET_SUCCESS)
					return ret;
			}
			else
			{
				if ((ret = wlSendMsgEventToItemRequest(pWatchlistImpl, pEvent, pItemRequest, pErrorInfo))
						!= RSSL_RET_SUCCESS)
					return ret;
			}

			pState->streamState = tmpStreamState;
			pState->dataState = tmpDataState;
		}
		else
		{
			wlItemRequestClose(&pWatchlistImpl->base, &pWatchlistImpl->items, 
					pItemRequest);

			if (pRsslMsg->msgBase.msgClass == RSSL_MC_REFRESH)
			{
				if ((ret = wlSendRefreshEventToItemRequest(pWatchlistImpl, pEvent, pItemRequest, pErrorInfo))
						!= RSSL_RET_SUCCESS)
					return ret;
			}
			else
			{
				if ((ret = wlSendMsgEventToItemRequest(pWatchlistImpl, pEvent, pItemRequest, pErrorInfo))
						!= RSSL_RET_SUCCESS)
					return ret;
			}
			pItemRequest->base.pStream = NULL;
			wlRequestedServiceCheckRefCount(&pWatchlistImpl->base, pItemRequest->pRequestedService);
			wlDestroyItemRequest(pWatchlistImpl, pItemRequest,
					pErrorInfo);
		}
	}

	return RSSL_RET_SUCCESS;
}

void wlDestroyItemRequest(RsslWatchlistImpl *pWatchlistImpl,
		WlItemRequest *pItemRequest, RsslErrorInfo *pErrorInfo)
{
	switch(pItemRequest->base.domainType)
	{
		case RSSL_DMT_SYMBOL_LIST:
			wlSymbolListRequestDestroy(&pWatchlistImpl->base, &pWatchlistImpl->items,
					(WlSymbolListRequest*)pItemRequest);
			break;

		default:
			wlItemRequestDestroy(&pWatchlistImpl->base, pItemRequest);
			break;
	}
}

static RsslRet wlChangeServiceNameToID(RsslWatchlistImpl *pWatchlistImpl,
		RsslMsgKey *pMsgKey, RsslBuffer *pServiceName, RsslErrorInfo *pErrorInfo)
{
	RDMCachedService *pService;

	if (pMsgKey->flags & RSSL_MKF_HAS_SERVICE_ID)
	{
		rsslSetErrorInfo(pErrorInfo, RSSL_EIC_FAILURE, RSSL_RET_INVALID_DATA, __FILE__, __LINE__, 
				"Message submitted with both service name and service ID.");
		return RSSL_RET_INVALID_DATA;
	}

	if ((pService = wlServiceCacheFindService(pWatchlistImpl->base.pServiceCache, pServiceName, 
					NULL)))
	{
		pMsgKey->flags |= RSSL_MKF_HAS_SERVICE_ID;
		pMsgKey->serviceId = (RsslUInt16)pService->rdm.serviceId;
		return RSSL_RET_SUCCESS;
	}
	else
	{
		rsslSetErrorInfo(pErrorInfo, RSSL_EIC_FAILURE, RSSL_RET_INVALID_DATA, __FILE__, __LINE__, 
				"Message submitted with unknown service name %.*s.", pServiceName->length,
				pServiceName->data);
		return RSSL_RET_INVALID_DATA;
	}
}

RsslRet rsslWatchlistSubmitBuffer(RsslWatchlist *pWatchlist,
		RsslWatchlistProcessMsgOptions *pOptions, RsslErrorInfo *pErrorInfo)
{
	RsslDecodeIterator dIter;
	RsslMsg rsslMsg;
	RsslWatchlistImpl *pWatchlistImpl = (RsslWatchlistImpl*)pWatchlist;
	RsslRet ret;

	assert(pOptions->pRsslMsg == NULL && pOptions->pRsslBuffer != NULL);

	rsslClearDecodeIterator(&dIter);
	rsslSetDecodeIteratorRWFVersion(&dIter, pOptions->majorVersion, pOptions->minorVersion);
	rsslSetDecodeIteratorBuffer(&dIter, pOptions->pRsslBuffer);

	if ((ret = rsslDecodeMsg(&dIter, &rsslMsg)) != RSSL_RET_SUCCESS)
	{
		rsslSetErrorInfo(pErrorInfo, RSSL_EIC_FAILURE, ret, 
				__FILE__, __LINE__, "Watchlist failed to decode buffer to message.");
	}

	pOptions->pRsslMsg = &rsslMsg;
	if ((ret = rsslWatchlistSubmitMsg(pWatchlist, pOptions, pErrorInfo)) < RSSL_RET_SUCCESS)
		return ret;
	pOptions->pRsslMsg = NULL;

	/* If the buffer is still present on the options, then the watchlist encoded a new
	 * buffer for submitting it. So we need to release our own buffer now. */
	if (pOptions->pRsslBuffer != NULL)
	{
		RsslError error;
		rsslReleaseBuffer(pOptions->pRsslBuffer, &error);
	}

	return ret;
}

RsslRet rsslWatchlistSubmitMsg(RsslWatchlist *pWatchlist, 
		RsslWatchlistProcessMsgOptions *pOptions, RsslErrorInfo *pErrorInfo)
{
	RsslRDMMsg rdmMsg, *pRdmMsg = NULL;
	RsslRet ret;
	WlRequest *pRequest = NULL;
	RsslHashLink *pRequestLink = NULL;
	RsslWatchlistImpl *pWatchlistImpl = (RsslWatchlistImpl*)pWatchlist;
	RsslReactorChannelImpl *pReactorChannelImpl = (RsslReactorChannelImpl*)pWatchlist->pUserSpec;

	if (pOptions->pRsslMsg)
	{
		RsslInt32 streamId = pOptions->pRsslMsg->msgBase.streamId;

		switch(pOptions->pRsslMsg->msgBase.domainType)
		{
			case RSSL_DMT_LOGIN:
			{
				/* Decode to Login Msg and break. */
				do
				{
					RsslBuffer memBuffer = pWatchlistImpl->base.tempDecodeBuffer;
					RsslDecodeIterator dIter;

					if (pOptions->pRsslMsg->msgBase.msgClass == RSSL_MC_POST)
					{
						rsslSetErrorInfo(pErrorInfo, RSSL_EIC_FAILURE, RSSL_RET_INVALID_DATA, __FILE__, __LINE__, 
								"Post message domainType is login.", streamId);
						return RSSL_RET_INVALID_DATA;
					}

					rsslClearDecodeIterator(&dIter);
					rsslSetDecodeIteratorRWFVersion(&dIter, pOptions->majorVersion, pOptions->minorVersion);
					rsslSetDecodeIteratorBuffer(&dIter, &pOptions->pRsslMsg->msgBase.encDataBody);

					if ((ret = rsslDecodeRDMLoginMsg(&dIter, pOptions->pRsslMsg,
							&rdmMsg.loginMsg, &memBuffer, pErrorInfo)) == RSSL_RET_SUCCESS)
						break;

					switch(ret)
					{
						case RSSL_RET_BUFFER_TOO_SMALL:
						{
							if (rsslHeapBufferResize(&pWatchlistImpl->base.tempDecodeBuffer,
										pWatchlistImpl->base.tempDecodeBuffer.length * 2, RSSL_FALSE)
									!= RSSL_RET_SUCCESS)
							{
								rsslSetErrorInfo(pErrorInfo, RSSL_EIC_FAILURE, RSSL_RET_FAILURE, __FILE__, __LINE__, 
										"Memory allocation failed.");
								return RSSL_RET_FAILURE;
							}
							continue;
						}

						default:
							return ret;
					}
				} while (1);

				pRdmMsg = &rdmMsg;
				break;
			}

			case RSSL_DMT_SOURCE:
			{
				do
				{
					RsslBuffer memBuffer = pWatchlistImpl->base.tempDecodeBuffer;
					RsslDecodeIterator dIter;

					rsslClearDecodeIterator(&dIter);
					rsslSetDecodeIteratorRWFVersion(&dIter, pOptions->majorVersion, pOptions->minorVersion);
					rsslSetDecodeIteratorBuffer(&dIter, &pOptions->pRsslMsg->msgBase.encDataBody);

					if ((ret = rsslDecodeRDMDirectoryMsg(&dIter, pOptions->pRsslMsg,
							&rdmMsg.directoryMsg, &memBuffer, pErrorInfo)) == RSSL_RET_SUCCESS)
						break;

					switch(ret)
					{
						case RSSL_RET_BUFFER_TOO_SMALL:
						{
							if (rsslHeapBufferResize(&pWatchlistImpl->base.tempDecodeBuffer,
										pWatchlistImpl->base.tempDecodeBuffer.length * 2, RSSL_FALSE)
									!= RSSL_RET_SUCCESS)
							{
								rsslSetErrorInfo(pErrorInfo, RSSL_EIC_FAILURE, RSSL_RET_FAILURE, __FILE__, __LINE__, 
										"Memory allocation failed.");
								return RSSL_RET_FAILURE;
							}
							continue;
						}

						default:
							return ret;
					}
				} while (1);

				pRdmMsg = &rdmMsg;
				break;
			}

			default:
			{

				if ((pRequestLink = rsslHashTableFind(&pWatchlistImpl->base.requestsByStreamId, &streamId, NULL)))
					pRequest = RSSL_HASH_LINK_TO_OBJECT(WlRequest, base.hlStreamId, pRequestLink);

				switch (pOptions->pRsslMsg->msgBase.msgClass)
				{
					case RSSL_MC_REQUEST:
					{
						WlItemRequestCreateOpts opts;

						wlClearItemRequestCreateOptions(&opts);
						opts.pRequestMsg = (RsslRequestMsg*)pOptions->pRsslMsg;
						opts.pServiceName = pOptions->pServiceName;
						opts.pUserSpec = pOptions->pUserSpec;
						opts.majorVersion = pOptions->majorVersion;
						opts.minorVersion = pOptions->minorVersion;

						if ((ret = wlProcessItemRequest(pWatchlistImpl, &opts, (WlItemRequest*)pRequest,
							   pErrorInfo)) != RSSL_RET_SUCCESS)
							return ret;

						break;
					}

					case RSSL_MC_CLOSE:
					{
						RsslInt32 streamId = pOptions->pRsslMsg->msgBase.streamId;
						WlItemRequest *pItemRequest;
						WlItemStream *pItemStream;

						if (!pRequest)
						{
							rsslSetErrorInfo(pErrorInfo, RSSL_EIC_FAILURE, RSSL_RET_INVALID_DATA, __FILE__, __LINE__, 
									"Close requested for unknown stream %d.", streamId);
							return RSSL_RET_INVALID_DATA;
						}

						pItemRequest = (WlItemRequest*)pRequest;
						pItemStream = (WlItemStream*)pItemRequest->base.pStream;

						wlItemRequestClose(&pWatchlistImpl->base, &pWatchlistImpl->items, 
								pItemRequest);

						wlRequestedServiceCheckRefCount(&pWatchlistImpl->base, pItemRequest->pRequestedService);
						wlDestroyItemRequest(pWatchlistImpl, pItemRequest, pErrorInfo);

						if (pItemStream)
						{
							if (!wlItemStreamHasRequests(pItemStream))
							{
								/* If inside the current fanout stream, indicate that
								 * it should be closed. Otherwise close it. */
								if (pItemStream == pWatchlistImpl->items.pCurrentFanoutStream)
									pWatchlistImpl->items.pCurrentFanoutStream = NULL;
								else
									wlItemStreamClose(&pWatchlistImpl->base, &pWatchlistImpl->items, 
											pItemStream, pErrorInfo);
							}
						}
						break;
					}

					case RSSL_MC_GENERIC:
					{
						RsslInt32 streamId = pOptions->pRsslMsg->msgBase.streamId;
						RsslGenericMsg genericMsg = pOptions->pRsslMsg->genericMsg;
						WlItemRequest *pItemRequest;
						WlItemStream *pItemStream;

						if (_reactorHandlesWarmStandby(pReactorChannelImpl))
						{
							RsslReactorWarmStandByHandlerImpl *pWarmStandByHandlerImpl = pReactorChannelImpl->pWarmStandByHandlerImpl;
							RsslReactorWarmStandbyGroupImpl *pWarmStandByGroupImpl = &pWarmStandByHandlerImpl->warmStandbyGroupList[pWarmStandByHandlerImpl->currentWSyGroupIndex];

							/* Checks whether this is active channel for warm standby*/
							if (pWarmStandByGroupImpl->warmStandbyMode == RSSL_RWSB_MODE_LOGIN_BASED)
							{
								if (pReactorChannelImpl->isActiveServer == RSSL_FALSE && genericMsg.msgBase.domainType >= RSSL_DMT_MARKET_PRICE)
								{
									return (pWatchlistImpl->base.streamsPendingRequest.count
										|| pWatchlistImpl->base.newRequests.count) ? 1 : RSSL_RET_SUCCESS;
								}
							}
							else
							{
								if (pRequest)
								{
									RsslUInt serviceId;
									pItemRequest = (WlItemRequest*)pRequest;
									pItemStream = (WlItemStream*)pItemRequest->base.pStream;
									serviceId = pItemStream->pWlService->pService->rdm.serviceId;

									if (_isActiveServiceForWSBChannelByID(pWarmStandByGroupImpl, pReactorChannelImpl, serviceId) == RSSL_FALSE && genericMsg.msgBase.domainType >= RSSL_DMT_MARKET_PRICE)
									{
										return (pWatchlistImpl->base.streamsPendingRequest.count
											|| pWatchlistImpl->base.newRequests.count) ? 1 : RSSL_RET_SUCCESS;
									}
								}
							}
						}

						if (!pRequest)
						{
							rsslSetErrorInfo(pErrorInfo, RSSL_EIC_FAILURE, RSSL_RET_INVALID_DATA, __FILE__, __LINE__, 
									"Generic message submitted for unknown stream %d.", streamId);
							return RSSL_RET_INVALID_DATA;
						}

						/* Add service name, if it was specified instead of service ID on the key. */
						if (pOptions->pServiceName)
						{
							if (!(genericMsg.flags & RSSL_GNMF_HAS_MSG_KEY))
							{
								rsslSetErrorInfo(pErrorInfo, RSSL_EIC_FAILURE, RSSL_RET_INVALID_DATA, __FILE__, __LINE__, 
										"Generic message submitted with service name but no message key.");
								return RSSL_RET_INVALID_DATA;
							}

							if ((ret = wlChangeServiceNameToID(pWatchlistImpl,
											&genericMsg.msgBase.msgKey, pOptions->pServiceName, 
											pErrorInfo)) != RSSL_RET_SUCCESS)
								return ret;
						}

						pItemRequest = (WlItemRequest*)pRequest;
						pItemStream = (WlItemStream*)pItemRequest->base.pStream;

						if ( pItemRequest->flags & WL_IRQF_REFRESHED &&
								pItemStream && pItemStream->flags & WL_IOSF_ESTABLISHED)
						{
							/* Change stream ID and submit. */
							if (pOptions->pRsslBuffer && !pOptions->pServiceName)
							{
								RsslEncodeIterator eIter;
								rsslClearEncodeIterator(&eIter);
								rsslSetEncodeIteratorRWFVersion(&eIter, pWatchlistImpl->base.pRsslChannel->majorVersion,
										pWatchlistImpl->base.pRsslChannel->minorVersion);
								rsslSetEncodeIteratorBuffer(&eIter, pOptions->pRsslBuffer);

								/* Replace stream ID and send it straight through. */
								if ((ret = rsslReplaceStreamId(&eIter, pItemStream->base.streamId))
										!= RSSL_RET_SUCCESS)
								{
									rsslSetErrorInfo(pErrorInfo, RSSL_EIC_FAILURE, ret, __FILE__, __LINE__, 
											"Failed to update Stream ID on message.");
									return RSSL_RET_FAILURE;
								}

								if ((ret = wlWriteBuffer(pWatchlistImpl, pOptions->pRsslBuffer,
												pErrorInfo)) < RSSL_RET_SUCCESS)
									return ret;

								/* The buffer was provided via rsslWatchlistSubmitBuffer.
								 * Indicate that the buffer was actually used to write to the channel, so
								 * that rsslWatchlistSubmitBuffer does not attempt to release it. */
								pOptions->pRsslBuffer = NULL;

							}
							else
							{
								genericMsg.msgBase.streamId = pItemStream->base.streamId;
								if ((ret = wlEncodeAndSubmitMsg(pWatchlistImpl, (RsslMsg*)&genericMsg,
												NULL, RSSL_FALSE, NULL, pErrorInfo)) != RSSL_RET_SUCCESS)
									return ret;
							}
						}
						else
						{
							rsslSetErrorInfo(pErrorInfo, RSSL_EIC_FAILURE, RSSL_RET_INVALID_DATA, __FILE__, __LINE__, 
									"Generic message submitted to stream that is not established.");
							return RSSL_RET_INVALID_DATA;
						}

						break;
					}

					case RSSL_MC_POST:
					{
						RsslInt32 streamId; 
						RsslPostMsg postMsg = pOptions->pRsslMsg->postMsg;

						if (pRequest)
						{
							/* On-stream post. */
							WlItemRequest *pItemRequest = (WlItemRequest*)pRequest;
							WlItemStream *pItemStream = (WlItemStream*)pItemRequest->base.pStream;

							if ( !(pItemRequest->flags & WL_IRQF_REFRESHED) ||
									!(pItemStream && pItemStream->flags & WL_IOSF_ESTABLISHED))
							{
								rsslSetErrorInfo(pErrorInfo, RSSL_EIC_FAILURE, RSSL_RET_INVALID_DATA, __FILE__, __LINE__, 
										"Post message submitted to stream that is not established.");
								return RSSL_RET_INVALID_DATA;
							}

							streamId = pItemStream->base.streamId;
						}
						else if (pWatchlistImpl->login.pRequest[pWatchlistImpl->login.index]
								&& pOptions->pRsslMsg->msgBase.streamId 
								== pWatchlistImpl->login.pRequest[pWatchlistImpl->login.index]->base.streamId)
						{
							pRequest = (WlRequest*)pWatchlistImpl->login.pRequest[pWatchlistImpl->login.index];

							/* Off-stream post */
							if (!pWatchlistImpl->login.pStream 
									|| !(pWatchlistImpl->login.pStream->flags & WL_LSF_ESTABLISHED))
							{
								rsslSetErrorInfo(pErrorInfo, RSSL_EIC_FAILURE, RSSL_RET_INVALID_DATA, __FILE__, __LINE__, 
										"Off-stream post message submitted to login stream that is not established.");
								return RSSL_RET_INVALID_DATA;
							}

							streamId = pWatchlistImpl->login.pStream->base.streamId; 
						}
						else
						{
							rsslSetErrorInfo(pErrorInfo, RSSL_EIC_FAILURE, RSSL_RET_INVALID_DATA, __FILE__, __LINE__, 
									"Post message submitted for unknown stream %d.", postMsg.msgBase.streamId);
							return RSSL_RET_INVALID_DATA;
						}

						postMsg.msgBase.streamId = streamId;

						/* Add service name, if it was specified instead of service ID on the key. */
						if (pOptions->pServiceName)
						{
							if (!(postMsg.flags & RSSL_PSMF_HAS_MSG_KEY))
							{
								rsslSetErrorInfo(pErrorInfo, RSSL_EIC_FAILURE, RSSL_RET_INVALID_DATA, __FILE__, __LINE__, 
										"Post message submitted with service name but no message key.");
								return RSSL_RET_INVALID_DATA;
							}

							if ((ret = wlChangeServiceNameToID(pWatchlistImpl,
											&postMsg.msgBase.msgKey, pOptions->pServiceName, 
											pErrorInfo)) != RSSL_RET_SUCCESS)
								return ret;
						}

						if (postMsg.flags & RSSL_PSMF_ACK)
						{
							WlPostRecord *pPostRecord;

							/* Save post for routing acknowledgements or so it can
							 * be timed out if not acknowledged. */

							if (!(pPostRecord = wlPostTableAddRecord(&pWatchlistImpl->base, 
											&pWatchlistImpl->base.postTable, &postMsg, 
											pErrorInfo)))
								return pErrorInfo->rsslError.rsslErrorId;

							pPostRecord->pUserSpec = (void*)pRequest;

							if ((ret = wlEncodeAndSubmitMsg(pWatchlistImpl, (RsslMsg*)&postMsg,
											NULL, RSSL_FALSE, NULL, pErrorInfo)) < RSSL_RET_SUCCESS)
							{
								wlPostTableRemoveRecord(&pWatchlistImpl->base.postTable, pPostRecord);
								return ret;
							}

							rsslQueueAddLinkToBack(&pRequest->base.openPosts,
									&pPostRecord->qlUser);

							break;
						}
						else
						{
							if ((ret = wlEncodeAndSubmitMsg(pWatchlistImpl, (RsslMsg*)&postMsg,
											NULL, RSSL_FALSE, NULL, pErrorInfo)) != RSSL_RET_SUCCESS)
								return ret;
							break;
						}

						break;
					}

					default:
						rsslSetErrorInfo(pErrorInfo, RSSL_EIC_FAILURE, RSSL_RET_INVALID_ARGUMENT, __FILE__, __LINE__, 
								"Unsupported consumer message class %u.", pOptions->pRsslMsg->msgBase.msgClass);
						return RSSL_RET_INVALID_ARGUMENT;
				}

				return (pWatchlistImpl->base.streamsPendingRequest.count 
						|| pWatchlistImpl->base.newRequests.count) ? 1 : RSSL_RET_SUCCESS;
			}
		}

	}
	else
		pRdmMsg = pOptions->pRdmMsg;

	if(pRdmMsg)
	{
		switch(pRdmMsg->rdmMsgBase.domainType)
		{
			case RSSL_DMT_LOGIN:
			{
				RsslRDMLoginMsg *pLoginMsg = (RsslRDMLoginMsg*)pRdmMsg;
				WlLoginConsumerAction loginAction;

				/* Update the login index when the channel supports the session management */
				if (_reactorHandlesWarmStandby(pReactorChannelImpl) == RSSL_FALSE)
				{
					if (pReactorChannelImpl->supportSessionMgnt || pReactorChannelImpl->channelRole.ommConsumerRole.pLoginRequestList)
						pWatchlistImpl->login.index = pReactorChannelImpl->connectionListIter;
				}
				else
				{
					pWatchlistImpl->login.index = 0;
				}

				if ((ret = wlLoginProcessConsumerMsg(&pWatchlistImpl->login, &pWatchlistImpl->base,
						(RsslRDMLoginMsg*)pRdmMsg, pOptions->pUserSpec, pOptions->newConnection, &loginAction, pErrorInfo)) != RSSL_RET_SUCCESS)
						return ret;

				switch(loginAction)
				{

					case WL_LGCA_NONE:

						if (pRdmMsg->rdmMsgBase.rdmMsgType == RDM_LG_MT_CONSUMER_CONNECTION_STATUS || pRdmMsg->rdmMsgBase.rdmMsgType == RDM_LG_MT_RTT)
						{
							if ((ret = wlEncodeAndSubmitMsg(pWatchlistImpl, NULL,
								(RsslRDMMsg*)pLoginMsg, RSSL_FALSE, NULL, pErrorInfo)) < RSSL_RET_SUCCESS)
								return ret;
						}

						break;

					case WL_LGCA_PAUSE_ALL:
					{
						RsslQueueLink *pStreamLink;

						RSSL_QUEUE_FOR_EACH_LINK(&pWatchlistImpl->base.openStreams, pStreamLink)
						{
							RsslQueueLink *pLink;
							WlItemStream *pStream;
							RsslUInt8 domainType;

							pStream = RSSL_QUEUE_LINK_TO_OBJECT(WlItemStream, base.qlStreamsList, pStreamLink);
							domainType = pStream->streamAttributes.domainType;

							if (domainType == RSSL_DMT_LOGIN
									|| domainType == RSSL_DMT_SOURCE
									|| domainType == RSSL_DMT_DICTIONARY)
								continue;

							pStream->flags |= WL_IOSF_PAUSED;
							if (pStream->requestsPausedCount != pStream->requestsStreamingCount)
							{
								/* Pause. */
								pStream->requestsPausedCount = pStream->requestsStreamingCount;
								RSSL_QUEUE_FOR_EACH_LINK(&pStream->requestsRecovering, pLink)
								{
									WlRequest *pRequest = RSSL_QUEUE_LINK_TO_OBJECT(WlRequest, base.qlStateQueue, pLink);
									pRequest->item.requestMsgFlags |= RSSL_RQMF_PAUSE;
								}

								RSSL_QUEUE_FOR_EACH_LINK(&pStream->requestsPendingRefresh, pLink)
								{
									WlRequest *pRequest = RSSL_QUEUE_LINK_TO_OBJECT(WlRequest, base.qlStateQueue, pLink);
									pRequest->item.requestMsgFlags |= RSSL_RQMF_PAUSE;
								}

								RSSL_QUEUE_FOR_EACH_LINK(&pStream->requestsOpen, pLink)
								{
									WlRequest *pRequest = RSSL_QUEUE_LINK_TO_OBJECT(WlRequest, base.qlStateQueue, pLink);
									pRequest->item.requestMsgFlags |= RSSL_RQMF_PAUSE;
								}
							}
						}
						break;
					}

					case WL_LGCA_RESUME_ALL:
					{
						RsslQueueLink *pStreamLink;

						RSSL_QUEUE_FOR_EACH_LINK(&pWatchlistImpl->base.openStreams, pStreamLink)
						{
							RsslQueueLink *pLink;
							WlItemStream *pStream;
							RsslUInt8 domainType;

							pStream = RSSL_QUEUE_LINK_TO_OBJECT(WlItemStream, base.qlStreamsList, pStreamLink);
							domainType = pStream->streamAttributes.domainType;

							if (domainType == RSSL_DMT_LOGIN
									|| domainType == RSSL_DMT_SOURCE
									|| domainType == RSSL_DMT_DICTIONARY)
								continue;

							pStream->flags &= ~WL_IOSF_PAUSED;
							if (pStream->requestsPausedCount != pStream->requestsStreamingCount
									&& pStream->requestsPausedCount > 0)
							{
								/* Resume. */
								pStream->requestsPausedCount = 0;
								RSSL_QUEUE_FOR_EACH_LINK(&pStream->requestsRecovering, pLink)
								{
									WlRequest *pRequest = RSSL_QUEUE_LINK_TO_OBJECT(WlRequest, base.qlStateQueue, pLink);
									pRequest->item.requestMsgFlags &= ~RSSL_RQMF_PAUSE;
								}

								RSSL_QUEUE_FOR_EACH_LINK(&pStream->requestsPendingRefresh, pLink)
								{
									WlRequest *pRequest = RSSL_QUEUE_LINK_TO_OBJECT(WlRequest, base.qlStateQueue, pLink);
									pRequest->item.requestMsgFlags &= ~RSSL_RQMF_PAUSE;
								}

								RSSL_QUEUE_FOR_EACH_LINK(&pStream->requestsOpen, pLink)
								{
									WlRequest *pRequest = RSSL_QUEUE_LINK_TO_OBJECT(WlRequest, base.qlStateQueue, pLink);
									pRequest->item.requestMsgFlags &= ~RSSL_RQMF_PAUSE;
								}
							}
						}
						break;
					}

					case WL_LGCA_CLOSE:
					{
						WlLoginRequest *pLoginRequest = (WlLoginRequest*)pWatchlistImpl->login.pRequest[pWatchlistImpl->login.index];
						RsslQueueLink *pLink;

						while(pLink = rsslQueuePeekFront(&pWatchlistImpl->base.requestedServices))
						{
							RsslQueueLink *pRequestLink;
							WlRequestedService *pRequestedService = RSSL_QUEUE_LINK_TO_OBJECT(WlRequestedService, 
									qlServiceRequests, pLink);

							while (pRequestLink = rsslQueueRemoveFirstLink(&pRequestedService->directoryRequests))
							{
								WlDirectoryRequest *pDirectoryRequest = RSSL_QUEUE_LINK_TO_OBJECT(WlDirectoryRequest,
										qlRequestedService, pRequestLink);
								wlDirectoryRequestDestroy(pDirectoryRequest);
							}

							while (pRequestLink = rsslQueuePeekFront(&pRequestedService->itemRequests))
							{
								WlItemRequest *pItemRequest = RSSL_QUEUE_LINK_TO_OBJECT(WlItemRequest,
										qlRequestedService, pRequestLink);
								wlItemRequestClose(&pWatchlistImpl->base, &pWatchlistImpl->items,
									pItemRequest);

								switch(pItemRequest->base.domainType)
								{
									case RSSL_DMT_SYMBOL_LIST:
										wlSymbolListRequestDestroy(&pWatchlistImpl->base, &pWatchlistImpl->items,
												(WlSymbolListRequest*)pItemRequest);
										break;

									default:
										wlItemRequestDestroy(&pWatchlistImpl->base, pItemRequest);
										break;
								}
							}

							/* The service should be clear at this point, so we can delete pRequestedService. */
							wlRequestedServiceClose(&pWatchlistImpl->base, pRequestedService);
							wlRequestedServiceDestroy(pRequestedService);
						}

						RSSL_QUEUE_FOR_EACH_LINK(&pWatchlistImpl->base.openStreams, pLink)
						{
							WlStream *pStream = RSSL_QUEUE_LINK_TO_OBJECT(WlStream, base.qlStreamsList, 
									pLink);

							if (pStream != (WlStream*)pWatchlistImpl->login.pStream)
							{

								switch(pStream->base.domainType)
								{
									case RSSL_DMT_SOURCE:
										rsslQueueRemoveLink(&pWatchlistImpl->base.openStreams, pLink);
										wlDirectoryStreamClose(&pWatchlistImpl->base,
												&pWatchlistImpl->directory, RSSL_FALSE);
										wlDirectoryStreamDestroy(&pStream->directory); 
										break;
									default:
										{
											WlItemStream *pItemStream = (WlItemStream*)pStream;
											pItemStream->flags |= WL_IOSF_CLOSED;
											wlItemStreamClose(&pWatchlistImpl->base, &pWatchlistImpl->items, 
													pItemStream, pErrorInfo);
											break;
										}
								}

							}
						}

						break;
					}
				}
				break;
			}

			case RSSL_DMT_SOURCE:
			{
				RsslRDMDirectoryMsg *pDirectoryMsg = (RsslRDMDirectoryMsg*)pRdmMsg;
				RsslInt32 streamId = pDirectoryMsg->rdmMsgBase.streamId;
				WlDirectoryRequest *pDirectoryRequest;

				if ((pRequestLink = rsslHashTableFind(&pWatchlistImpl->base.requestsByStreamId, &streamId, NULL)))
					pDirectoryRequest = RSSL_HASH_LINK_TO_OBJECT(WlDirectoryRequest, base.hlStreamId, pRequestLink);
				else
					pDirectoryRequest = NULL;

				switch(pDirectoryMsg->rdmMsgBase.rdmMsgType)
				{
					case RDM_DR_MT_REQUEST:
						{
							WlRequestedService *pRequestedService;
							RsslRDMDirectoryRequest *pDirectoryReqMsg = &pRdmMsg->directoryMsg.request;
							if (pDirectoryRequest)
							{

								// save filter from request
								pDirectoryRequest->filter = pDirectoryReqMsg->filter;

								/* Move request back to list of new requests so we provide
								 * a new refresh out of cache. */
								pDirectoryRequest->state = WL_DR_REQUEST_PENDING_REFRESH;
								if (pDirectoryRequest->base.pStateQueue)
									rsslQueueRemoveLink(pDirectoryRequest->base.pStateQueue,
											&pDirectoryRequest->base.qlStateQueue);

								pDirectoryRequest->base.pStateQueue = &pWatchlistImpl->base.newRequests;
								rsslQueueAddLinkToBack(&pWatchlistImpl->base.newRequests, 
										&pDirectoryRequest->base.qlStateQueue);

								break;
							}
							else
							{
								if (pDirectoryReqMsg->rdmMsgBase.streamId < 0)
								{
									rsslSetErrorInfo(pErrorInfo, RSSL_EIC_FAILURE, RSSL_RET_INVALID_DATA, __FILE__, __LINE__, 
											"Cannot open request with negative stream ID.");
									return RSSL_RET_INVALID_DATA;
								}
							}


							pDirectoryRequest = wlDirectoryRequestCreate(
									pDirectoryReqMsg, pOptions->pUserSpec, pErrorInfo);

							if (pDirectoryReqMsg->flags & RDM_DR_RQF_HAS_SERVICE_ID
									&& pOptions->pServiceName)
							{
								rsslSetErrorInfo(pErrorInfo, RSSL_EIC_FAILURE, RSSL_RET_INVALID_DATA, __FILE__, __LINE__, 
										"Cannot open request with both service ID and service name.");
								return RSSL_RET_INVALID_DATA;
							}


							if (pOptions->pServiceName)
							{
								if (!(pRequestedService = wlRequestedServiceOpen(
										&pWatchlistImpl->base, pOptions->pServiceName, NULL, 
										pErrorInfo)))
								{
									wlDirectoryRequestDestroy(pDirectoryRequest);
									return pErrorInfo->rsslError.rsslErrorId;
								}

								pDirectoryRequest->pRequestedService = pRequestedService;
								rsslQueueAddLinkToBack(&pRequestedService->directoryRequests,
										&pDirectoryRequest->qlRequestedService);

							}
							else if (pDirectoryReqMsg->flags & RDM_DR_RQF_HAS_SERVICE_ID)
							{
								RsslUInt serviceId = pDirectoryMsg->request.serviceId;

								if (!(pRequestedService = wlRequestedServiceOpen(
												&pWatchlistImpl->base, NULL, &serviceId, pErrorInfo)))
								{
									wlDirectoryRequestDestroy(pDirectoryRequest);
									return pErrorInfo->rsslError.rsslErrorId;
								}

								pDirectoryRequest->pRequestedService = pRequestedService;
								rsslQueueAddLinkToBack(&pRequestedService->directoryRequests,
										&pDirectoryRequest->qlRequestedService);
							}
							else
							{
								/* Directory request for all services. */
								pDirectoryRequest->pRequestedService = NULL;
								rsslQueueAddLinkToBack(&pWatchlistImpl->directory.requests,
										&pDirectoryRequest->qlRequestedService);
							}

							wlAddRequest(&pWatchlistImpl->base, &pDirectoryRequest->base);

							pDirectoryRequest->base.pStateQueue = &pWatchlistImpl->base.newRequests;
							rsslQueueAddLinkToBack(&pWatchlistImpl->base.newRequests, 
									&pDirectoryRequest->base.qlStateQueue);
							break;
						}

					case RDM_DR_MT_CLOSE:
						{
							if (!pDirectoryRequest)
							{
								rsslSetErrorInfo(pErrorInfo, RSSL_EIC_FAILURE, RSSL_RET_INVALID_DATA, __FILE__, __LINE__, 
										"Close requested for unknown stream %d.", streamId);
								return RSSL_RET_INVALID_DATA;
							}

							wlDirectoryRequestClose(&pWatchlistImpl->base, 
									&pWatchlistImpl->directory, pDirectoryRequest);
							break;

						}
					case RDM_DR_MT_CONSUMER_STATUS:
					{
						if ( (!pDirectoryRequest && (_reactorHandlesWarmStandby(pReactorChannelImpl) == RSSL_FALSE)) || pWatchlistImpl->directory.pStream == NULL)
						{
							rsslSetErrorInfo(pErrorInfo, RSSL_EIC_FAILURE, RSSL_RET_INVALID_DATA, __FILE__, __LINE__,
								"Generic message submitted for unknown stream %d.", streamId);
							return RSSL_RET_INVALID_DATA;
						}
						
						if (_reactorHandlesWarmStandby(pReactorChannelImpl) || (pDirectoryRequest->state & WL_DR_REQUEST_OK))
						{
							
								pDirectoryMsg->rdmMsgBase.streamId = pWatchlistImpl->directory.pStream->base.streamId;
								if ((ret = wlEncodeAndSubmitMsg(pWatchlistImpl, NULL,
									(RsslRDMMsg*)pDirectoryMsg, RSSL_FALSE, NULL, pErrorInfo)) != RSSL_RET_SUCCESS)
									return ret;
							
						}
						else
						{
							rsslSetErrorInfo(pErrorInfo, RSSL_EIC_FAILURE, RSSL_RET_INVALID_DATA, __FILE__, __LINE__,
								"Generic message submitted to stream that is not established.");
							return RSSL_RET_INVALID_DATA;
						}

						break;
					}
					default:
						rsslSetErrorInfo(pErrorInfo, RSSL_EIC_FAILURE, RSSL_RET_INVALID_ARGUMENT, 
								__FILE__, __LINE__, "Unsupported Directory RDM message type %u.", 
								pDirectoryMsg->rdmMsgBase.rdmMsgType);
						return RSSL_RET_INVALID_ARGUMENT;
				}

				break;
			}

			case RSSL_DMT_DICTIONARY:
			{
				RsslRDMDictionaryRequest *pDictionaryRequest = 
					&pOptions->pRdmMsg->dictionaryMsg.request;
				RsslRequestMsg requestMsg;

				WlItemRequestCreateOpts opts;

				rsslClearRequestMsg(&requestMsg);
				requestMsg.msgBase.msgClass = RSSL_MC_REQUEST;
				requestMsg.msgBase.streamId = pDictionaryRequest->rdmMsgBase.streamId;
				requestMsg.msgBase.domainType = RSSL_DMT_DICTIONARY;
				requestMsg.msgBase.containerType = RSSL_DT_NO_DATA;
				requestMsg.flags = RSSL_RQMF_NONE;

				if (pDictionaryRequest->flags & RDM_DC_RQF_STREAMING)
					requestMsg.flags |= RSSL_RQMF_STREAMING;

				requestMsg.msgBase.msgKey.flags = RSSL_MKF_HAS_FILTER 
					| RSSL_MKF_HAS_NAME;

				if (!pOptions->pServiceName)
					requestMsg.msgBase.msgKey.flags |= RSSL_MKF_HAS_SERVICE_ID;

				requestMsg.msgBase.msgKey.name = pDictionaryRequest->dictionaryName;
				requestMsg.msgBase.msgKey.serviceId = pDictionaryRequest->serviceId;
				requestMsg.msgBase.msgKey.filter = pDictionaryRequest->verbosity;

				pOptions->pRdmMsg = NULL;
				pOptions->pRsslMsg = (RsslMsg*)&requestMsg;

				wlClearItemRequestCreateOptions(&opts);
				opts.pRequestMsg = (RsslRequestMsg*)&requestMsg;
				opts.pServiceName = pOptions->pServiceName;
				opts.pUserSpec = pOptions->pUserSpec;
				opts.majorVersion = pOptions->majorVersion;
				opts.minorVersion = pOptions->minorVersion;

				if ((ret = wlProcessItemRequest(pWatchlistImpl, &opts, (WlItemRequest*)pRequest, 
								pErrorInfo)) != RSSL_RET_SUCCESS)
					return ret;
				break;
			}

			default:
				{
					rsslSetErrorInfo(pErrorInfo, RSSL_EIC_FAILURE, RSSL_RET_INVALID_ARGUMENT, __FILE__, __LINE__, 
							"Unsupported domain message type %u.", pOptions->pRdmMsg->rdmMsgBase.domainType);
					return RSSL_RET_INVALID_ARGUMENT;
					break;
				}
		}
		return (pWatchlistImpl->base.streamsPendingRequest.count 
				|| pWatchlistImpl->base.newRequests.count) ? 1 : RSSL_RET_SUCCESS;
	}
	else
	{
		rsslSetErrorInfo(pErrorInfo, RSSL_EIC_FAILURE, RSSL_RET_INVALID_ARGUMENT, __FILE__, __LINE__, 
				"No message provided.");
		return RSSL_RET_INVALID_ARGUMENT;
	}

}

static RsslRet wlWriteBuffer(RsslWatchlistImpl *pWatchlistImpl, RsslBuffer *pWriteBuffer,
		RsslErrorInfo *pError)
{
	RsslRet ret;
	RsslUInt32 bytes, uncompBytes;
	RsslReactorChannelImpl *pReactorChannelImpl = (RsslReactorChannelImpl*)pWatchlistImpl->base.watchlist.pUserSpec;
	RsslReactorImpl *pReactorImpl = pReactorChannelImpl->pParentReactor;
	RsslBuffer *pOutputBuffer = pWriteBuffer;
	RsslBuffer *pMsgBuffer = NULL; /* The buffer to send JSON message to network only. */
	RsslBool releaseUserBuffer = RSSL_FALSE; /* Release when it writes user's buffer successfully. */
	RsslUInt8 writeFlags = (pReactorChannelImpl->directWrite ? RSSL_WRITE_DIRECT_SOCKET_WRITE : 0);

	/* Calls JSON converter when the protocol type is simplified JSON. */
	if (pWatchlistImpl->base.pRsslChannel->protocolType == RSSL_JSON_PROTOCOL_TYPE)
	{
		RsslDecodeIterator dIter;
		RsslMsg rsslMsg;

		/* Added checking to ensure that the JSON converter is initialized properly.*/
		if (pReactorImpl->pJsonConverter == 0)
		{
			rsslSetErrorInfo(pError, RSSL_EIC_FAILURE, RSSL_RET_FAILURE, __FILE__, __LINE__, "The JSON converter library has not been initialized.");
			return RSSL_RET_FAILURE;
		}

		rsslClearMsg(&rsslMsg);
		rsslClearDecodeIterator(&dIter);
		rsslSetDecodeIteratorRWFVersion(&dIter, pWatchlistImpl->base.pRsslChannel->majorVersion,
			pWatchlistImpl->base.pRsslChannel->minorVersion);

		rsslSetDecodeIteratorBuffer(&dIter, pOutputBuffer);

		ret = rsslDecodeMsg(&dIter, &rsslMsg);

		if (ret == RSSL_RET_SUCCESS)
		{
			RsslConvertRsslMsgToJsonOptions rjcOptions;
			RsslGetJsonMsgOptions getJsonMsgOptions;
			RsslJsonConverterError rjcError;
			RsslBuffer jsonBuffer;

			rsslClearConvertRsslMsgToJsonOptions(&rjcOptions);
			rjcOptions.jsonProtocolType = RSSL_JSON_JPT_JSON2; /* Supported only for Simplified JSON */
			if ((rsslConvertRsslMsgToJson(pReactorImpl->pJsonConverter, &rjcOptions, &rsslMsg, &rjcError)) != RSSL_RET_SUCCESS)
			{
				rsslSetErrorInfo(pError, RSSL_EIC_FAILURE, RSSL_RET_FAILURE, __FILE__, __LINE__,
					"Failed to convert RWF to JSON protocol. Error text: %s", rjcError.text);
				return RSSL_RET_FAILURE;
			}

			rsslClearGetJsonMsgOptions(&getJsonMsgOptions);
			getJsonMsgOptions.jsonProtocolType = RSSL_JSON_JPT_JSON2; /* Supported only for Simplified JSON */
			getJsonMsgOptions.streamId = rsslMsg.msgBase.streamId;
			getJsonMsgOptions.isCloseMsg = (rsslMsg.msgBase.msgClass == RSSL_MC_CLOSE) ? RSSL_TRUE : RSSL_FALSE;

			if ((ret = rsslGetConverterJsonMsg(pReactorImpl->pJsonConverter, &getJsonMsgOptions,
				&jsonBuffer, &rjcError)) != RSSL_RET_SUCCESS)
			{
				rsslSetErrorInfo(pError, RSSL_EIC_FAILURE, RSSL_RET_FAILURE, __FILE__, __LINE__,
					"Failed to get converted JSON message. Error text: %s", rjcError.text);
				return RSSL_RET_FAILURE;
			}

			/* Copies JSON data format to the buffer that belongs to RsslChannel */
			pMsgBuffer = rsslReactorGetBuffer(&pReactorChannelImpl->reactorChannel, jsonBuffer.length, RSSL_FALSE, pError);

			if (pMsgBuffer)
			{
				pMsgBuffer->length = jsonBuffer.length;
				memcpy(pMsgBuffer->data, jsonBuffer.data, jsonBuffer.length);
			}
			else
			{
				rsslSetErrorInfo(pError, RSSL_EIC_FAILURE, RSSL_RET_FAILURE, __FILE__, __LINE__,
					"Failed to get a buffer for sending JSON message. Error text: %s", pError->rsslError.text);
				return RSSL_RET_FAILURE;
			}

			releaseUserBuffer = RSSL_TRUE; /* Release the passed in buffer when using a new buffer for the JSON message */

			/* Updated the output buffer with the JSON message buffer. */
			pOutputBuffer = pMsgBuffer;
		}
		else
		{
			rsslSetErrorInfo(pError, RSSL_EIC_FAILURE, ret, __FILE__, __LINE__,
				"rsslDecodeMsg() failed to decode the passed in buffer as RWF messages.");
			return ret;
		}
	}

	if ((ret = rsslWrite(pWatchlistImpl->base.pRsslChannel, pOutputBuffer, RSSL_HIGH_PRIORITY, writeFlags, &bytes, &uncompBytes,
					&pError->rsslError)) < RSSL_RET_SUCCESS)
	{
		/* Collect write statistics */
		if ( (pReactorChannelImpl->statisticFlags & RSSL_RC_ST_WRITE) && pReactorChannelImpl->pChannelStatistic )
		{
			_cumulativeValue(&pReactorChannelImpl->pChannelStatistic->bytesWritten, bytes);
			_cumulativeValue(&pReactorChannelImpl->pChannelStatistic->uncompressedBytesWritten, uncompBytes);
		}

		switch(ret)
		{
			case RSSL_RET_WRITE_FLUSH_FAILED:
			{
				if (pWatchlistImpl->base.pRsslChannel->state == RSSL_CH_STATE_ACTIVE)
				{
					pWatchlistImpl->base.watchlist.state |= RSSLWL_STF_NEED_FLUSH;

					if (releaseUserBuffer)
					{
						RsslError rsslError;
						rsslReleaseBuffer(pWriteBuffer, &rsslError);
					}

					return 1;
				}
				else
					return ret;
			}
			case RSSL_RET_WRITE_CALL_AGAIN:
				assert(!pWatchlistImpl->base.pWriteCallAgainBuffer);
				pWatchlistImpl->base.watchlist.state |= RSSLWL_STF_NEED_FLUSH;
				pWatchlistImpl->base.pWriteCallAgainBuffer = pOutputBuffer;
				return RSSL_RET_SUCCESS;
			default:
			{
				rsslSetErrorInfoLocation(pError, __FILE__, __LINE__);

				/* Releases the additional buffer for sending JSON messages. */
				if (pMsgBuffer)
				{
					RsslError rsslError;
					rsslReleaseBuffer(pMsgBuffer, &rsslError);
				}

				return ret;
			}

		}
	}
	else if (ret > RSSL_RET_SUCCESS)
	{
		/* Collect write statistics */
		if ( (pReactorChannelImpl->statisticFlags & RSSL_RC_ST_WRITE) && pReactorChannelImpl->pChannelStatistic )
		{
			_cumulativeValue(&pReactorChannelImpl->pChannelStatistic->bytesWritten, bytes);
			_cumulativeValue(&pReactorChannelImpl->pChannelStatistic->uncompressedBytesWritten, uncompBytes);
		}

		pWatchlistImpl->base.watchlist.state |= RSSLWL_STF_NEED_FLUSH;
	}

	if (releaseUserBuffer)
	{
		RsslError rsslError;
		rsslReleaseBuffer(pWriteBuffer, &rsslError);
	}

	return ret;

}

static RsslRet wlEncodeAndSubmitMsg(RsslWatchlistImpl *pWatchlistImpl, 
		RsslMsg *pRsslMsg, RsslRDMMsg *pRdmMsg, RsslBool hasView, WlAggregateView *pView, 
		RsslErrorInfo *pError)
{
	RsslUInt32 msgSize;
	RsslBuffer *pWriteBuffer;
	RsslRet ret;
	RsslChannel *pChannel = pWatchlistImpl->base.pRsslChannel;
	RsslError releaseError;

	if (pWatchlistImpl->base.pWriteCallAgainBuffer)
	{
		if ((ret = wlWriteBuffer(pWatchlistImpl, pWatchlistImpl->base.pWriteCallAgainBuffer, pError))
			   	< RSSL_RET_SUCCESS)
		{
			switch(ret)
			{
				case RSSL_RET_WRITE_CALL_AGAIN:
					pWatchlistImpl->base.watchlist.state |= RSSLWL_STF_NEED_FLUSH;
					return RSSL_RET_BUFFER_NO_BUFFERS;
				default:
				{
					rsslReleaseBuffer(pWatchlistImpl->base.pWriteCallAgainBuffer, &releaseError);
					pWatchlistImpl->base.pWriteCallAgainBuffer = NULL;
					return ret;
				}
			}
		}
		else
			pWatchlistImpl->base.pWriteCallAgainBuffer = NULL;
	}

	/* Estimate the encoded message size. */
	if (pRsslMsg)
	{
		const RsslMsgKey *pKey;

		msgSize = 128;
		msgSize += pRsslMsg->msgBase.encDataBody.length;

		if ((pKey = rsslGetMsgKey(pRsslMsg)))
		{
			if (pKey->flags & RSSL_MKF_HAS_NAME)
				msgSize += pKey->name.length;

			if (pKey->flags & RSSL_MKF_HAS_ATTRIB)
				msgSize += pKey->encAttrib.length;
		}
	
		if (pView)
		{
			assert(hasView);
			msgSize += 32 + wlAggregateViewEstimateEncodedLength(pView);
		}

		if (hasView)
			pRsslMsg->requestMsg.flags |= RSSL_RQMF_HAS_VIEW;

	}
	else
	{
		msgSize = pWatchlistImpl->base.channelMaxFragmentSize;
	}

	do
	{
		RsslEncodeIterator encodeIter;

		if (!(pWriteBuffer = rsslGetBuffer(pChannel, msgSize, RSSL_FALSE, 
						&pError->rsslError)))
		{
			switch(pError->rsslError.rsslErrorId)
			{
				case RSSL_RET_BUFFER_NO_BUFFERS:
					return RSSL_RET_BUFFER_NO_BUFFERS;
				default:
					rsslSetErrorInfoLocation(pError, __FILE__, __LINE__);
					return pError->rsslError.rsslErrorId;
			}
		}

		rsslClearEncodeIterator(&encodeIter);
		rsslSetEncodeIteratorRWFVersion(&encodeIter, pChannel->majorVersion,
				pChannel->minorVersion);

		rsslSetEncodeIteratorBuffer(&encodeIter, pWriteBuffer);

		if (pRsslMsg)
		{
			if (!pView)
			{
				ret = rsslEncodeMsg(&encodeIter, pRsslMsg);
			}
			else
			{
				do
				{
					RsslElementList elementList;
					RsslElementEntry elementEntry;
					RsslUInt viewType;
					
					assert(pRsslMsg->msgBase.msgClass == RSSL_MC_REQUEST);
					pRsslMsg->msgBase.containerType = RSSL_DT_ELEMENT_LIST;
					if ((ret = rsslEncodeMsgInit(&encodeIter, pRsslMsg, msgSize)) 
							!= RSSL_RET_ENCODE_CONTAINER)
						break;

					rsslClearElementList(&elementList);
					elementList.flags = RSSL_ELF_HAS_STANDARD_DATA;
					if ((ret = rsslEncodeElementListInit(&encodeIter, &elementList, NULL, 0)) != RSSL_RET_SUCCESS)
						break;

					rsslClearElementEntry(&elementEntry);
					elementEntry.name = RSSL_ENAME_VIEW_TYPE;
					elementEntry.dataType = RSSL_DT_UINT;
					viewType = pView->viewType;
					if ((ret = rsslEncodeElementEntry(&encodeIter, &elementEntry, &viewType))
							!= RSSL_RET_SUCCESS)
						break;

					rsslClearElementEntry(&elementEntry);
					elementEntry.name = RSSL_ENAME_VIEW_DATA;
					elementEntry.dataType = RSSL_DT_ARRAY;
					if ((ret = rsslEncodeElementEntryInit(&encodeIter, &elementEntry, msgSize))
							!= RSSL_RET_SUCCESS)
						break;

					if ((ret = wlAggregateViewEncodeArray(&encodeIter, pView)) != RSSL_RET_SUCCESS)
						break;

					if ((ret = rsslEncodeElementEntryComplete(&encodeIter, RSSL_TRUE)) != RSSL_RET_SUCCESS)
						break;

					if ((ret = rsslEncodeElementListComplete(&encodeIter, RSSL_TRUE)) != RSSL_RET_SUCCESS)
						break;
				}
				while(0);

				if (ret != RSSL_RET_SUCCESS)
				{
					switch (ret)
					{
						case RSSL_RET_BUFFER_TOO_SMALL:
							/* Double buffer size and try again. */
							msgSize *= 2;
							rsslReleaseBuffer(pWriteBuffer, &releaseError);
							continue;
						default:
							rsslSetErrorInfo(pError, RSSL_EIC_FAILURE, ret, __FILE__, __LINE__, 
									"Message encoding failure -- %d.", ret);

							rsslReleaseBuffer(pWriteBuffer, &releaseError);
							/* Make sure error is negative. */
							if (ret > 0)
								ret = RSSL_RET_FAILURE;
							return ret;
					}
				}
			}

			pWriteBuffer->length = rsslGetEncodedBufferLength(&encodeIter);
		}
		else /* pRdmMsg */
			ret = rsslEncodeRDMMsg(&encodeIter, pRdmMsg, &pWriteBuffer->length, pError);

		if (ret != RSSL_RET_SUCCESS)
		{
			rsslReleaseBuffer(pWriteBuffer, &releaseError);
			switch (ret)
			{
				case RSSL_RET_BUFFER_TOO_SMALL:
					/* Double buffer size and try again. */
					msgSize *= 2;
					continue;
				default:
					rsslSetErrorInfo(pError, RSSL_EIC_FAILURE, ret, __FILE__, __LINE__, 
							"Message encoding failure -- %d.", ret);
					return ret;
			}
		}

		if ((ret = wlWriteBuffer(pWatchlistImpl, pWriteBuffer, pError)) < RSSL_RET_SUCCESS)
			rsslReleaseBuffer(pWriteBuffer, &releaseError);

		return ret;
		
	} while (1);
}

static RsslRet wlStreamSubmitMsg(RsslWatchlistImpl *pWatchlistImpl,
		WlStream *pStream, RsslUInt32 *pendingWaitCount, RsslErrorInfo *pError)
{
	RsslRet ret;
	RsslWatchlist *pWatchlist = (RsslWatchlist *)(pWatchlistImpl);
	RsslReactorChannelImpl *pReactorChannelImpl = (RsslReactorChannelImpl *)pWatchlist->pUserSpec;

	if (pStream->base.isClosing)
	{
		RsslCloseMsg closeMsg;

		/* This stream is open and has been marked for closing. 
		 * Once a close message is sent, we can clean it up. */

		rsslClearCloseMsg(&closeMsg);
		closeMsg.msgBase.streamId = pStream->base.streamId;
		closeMsg.msgBase.containerType = RSSL_DT_NO_DATA;
		closeMsg.msgBase.domainType = pStream->base.domainType;

		if ((ret = wlEncodeAndSubmitMsg(pWatchlistImpl, (RsslMsg*)&closeMsg, NULL, RSSL_FALSE, NULL, 
						pError) 
					>= RSSL_RET_SUCCESS))
		{
			/* Stream is now closed. */
			if (!pStream->base.tempStream)
			{
				switch(pStream->base.domainType)
				{
					case RSSL_DMT_LOGIN:
					{
						WlLoginStream *pLoginStream = (WlLoginStream*)pStream;
						wlUnsetStreamFromPendingLists(&pWatchlistImpl->base, &pStream->base);
						wlLoginStreamDestroy(pLoginStream);
						break;
					}
					case RSSL_DMT_SOURCE:
					{
						WlDirectoryStream *pDirectoryStream = (WlDirectoryStream*)pStream;
						wlUnsetStreamFromPendingLists(&pWatchlistImpl->base, &pStream->base);
						wlDirectoryStreamDestroy(pDirectoryStream);
						break;
					}
					default:
					{
						WlItemStream *pItemStream = (WlItemStream*)pStream;
						wlUnsetStreamFromPendingLists(&pWatchlistImpl->base, &pItemStream->base);
						wlItemStreamDestroy(&pWatchlistImpl->base, pItemStream);
						break;
					}
				}
				return ret;
			}
			else
			{
				/* Stream was created only for closing, so only the base structure is allocated. */
				wlUnsetStreamFromPendingLists(&pWatchlistImpl->base, &pStream->base);
				free(pStream);
			}

		}
	}
	else
	{
		switch(pStream->base.domainType)
		{
			case RSSL_DMT_LOGIN:
			{
				RsslRDMLoginRequest loginRequest;

				loginRequest = *pWatchlistImpl->login.pRequest[pWatchlistImpl->login.index]->pLoginReqMsg;
				loginRequest.rdmMsgBase.streamId = pStream->base.streamId;

				if (!pWatchlistImpl->base.config.supportOptimizedPauseResume)
					loginRequest.flags &= ~RDM_LG_RQF_PAUSE_ALL;

				ret = wlEncodeAndSubmitMsg(pWatchlistImpl, NULL, 
						(RsslRDMMsg*)&loginRequest, RSSL_FALSE, NULL, pError);

				if (ret >= RSSL_RET_SUCCESS)
				{
					if (pWatchlistImpl->base.channelState < WL_CHS_LOGIN_REQUESTED)
						pWatchlistImpl->base.channelState = WL_CHS_LOGIN_REQUESTED;

					if (pWatchlistImpl->base.pRsslChannel && !(loginRequest.flags & RDM_LG_RQF_NO_REFRESH))
						wlSetStreamPendingResponse(&pWatchlistImpl->base, &pStream->base);

					wlUnsetStreamMsgPending(&pWatchlistImpl->base, &pStream->base);

					/* If this was a pause message, unset the pause flag */
					if (pWatchlistImpl->login.pRequest[pWatchlistImpl->login.index]->pLoginReqMsg->flags & RDM_LG_RQF_PAUSE_ALL)
						pWatchlistImpl->login.pRequest[pWatchlistImpl->login.index]->pLoginReqMsg->flags &= ~RDM_LG_RQF_PAUSE_ALL;


					if (pWatchlistImpl->login.pRequest[pWatchlistImpl->login.index]->pCurrentToken
							&& !pWatchlistImpl->login.pRequest[pWatchlistImpl->login.index]->pCurrentToken->needRefresh)
						wlLoginSetNextUserToken(&pWatchlistImpl->login,
								&pWatchlistImpl->base);

					return ret;
				}
				break;
			}

			case RSSL_DMT_SOURCE:
			{
				RsslRDMDirectoryRequest directoryRequest;

				rsslInitDefaultRDMDirectoryRequest(&directoryRequest, 
						pStream->base.streamId);

				directoryRequest.filter = 
					RDM_DIRECTORY_SERVICE_INFO_FILTER
					| RDM_DIRECTORY_SERVICE_STATE_FILTER
					| RDM_DIRECTORY_SERVICE_GROUP_FILTER
					| RDM_DIRECTORY_SERVICE_LOAD_FILTER
					| RDM_DIRECTORY_SERVICE_DATA_FILTER
					| RDM_DIRECTORY_SERVICE_LINK_FILTER;

				ret = wlEncodeAndSubmitMsg(pWatchlistImpl, NULL, 
						(RsslRDMMsg*)&directoryRequest, RSSL_FALSE, NULL, pError);

				if (ret >= RSSL_RET_SUCCESS)
				{
					if (pWatchlistImpl->base.pRsslChannel)
						wlSetStreamPendingResponse(&pWatchlistImpl->base, &pStream->base);

					wlUnsetStreamMsgPending(&pWatchlistImpl->base, &pStream->base);
					return ret;
				}
				break;
			}

			default:
			{
				RsslRequestMsg requestMsg;
				WlAggregateView *pView = NULL;
				RsslBool hasViewFlag = RSSL_FALSE;
				RsslBool sendMsg = RSSL_FALSE;
				WlItemStream *pItemStream = (WlItemStream*)pStream;
				WlStreamAttributes *pAttributes = &pItemStream->streamAttributes;

				rsslClearRequestMsg(&requestMsg);

				/* Setup request attributes(stream ID, domain, key). */
				requestMsg.msgBase.streamId = pItemStream->base.streamId;
				requestMsg.msgBase.domainType = pAttributes->domainType;
				requestMsg.msgBase.msgKey = pAttributes->msgKey;
				requestMsg.msgBase.containerType = RSSL_DT_NO_DATA;

				if (pAttributes->qos.timeliness != RSSL_QOS_TIME_UNSPECIFIED)
				{
					requestMsg.flags |= RSSL_RQMF_HAS_QOS;
					requestMsg.qos = pAttributes->qos;
				}

				/* Set service ID, if one is needed. */
				if (pItemStream->pWlService)
				{
					RsslRDMService *pService = 
						&pItemStream->pWlService->pService->rdm;

					assert(!requestMsg.msgBase.msgKey.flags & RSSL_MKF_HAS_SERVICE_ID
							|| requestMsg.msgBase.msgKey.serviceId ==
							pService->serviceId);

					if (_reactorHandlesWarmStandby(pReactorChannelImpl))
					{
						RsslReactorWarmStandByHandlerImpl *pWarmStandByHandlerImpl = pReactorChannelImpl->pWarmStandByHandlerImpl;
						RsslReactorWarmStandbyGroupImpl *pWarmStandByGroupImpl = &pWarmStandByHandlerImpl->warmStandbyGroupList[pWarmStandByHandlerImpl->currentWSyGroupIndex];
						if (pWarmStandByGroupImpl->warmStandbyMode == RSSL_RWSB_MODE_LOGIN_BASED)
						{
							/* Don't send this request and wait until there is an active channel. */
							if (pWarmStandByHandlerImpl->pActiveReactorChannel == NULL)
							{
								(*pendingWaitCount)++;
								return RSSL_RET_SUCCESS;
							}

							if (pReactorChannelImpl->isActiveServer == RSSL_FALSE)
							{
								RsslHashLink *pHashLink;
								/* Checks whether the active server has this service before sending the request. */
								RsslWatchlistImpl *pActiveChannelWatchlist = (RsslWatchlistImpl*)pWarmStandByHandlerImpl->pActiveReactorChannel->pWatchlist;

								pHashLink = rsslHashTableFind(&pActiveChannelWatchlist->base.pServiceCache->_servicesById, &pService->serviceId, NULL);
								if (pHashLink)
								{
									RDMCachedService *pService = pService = RSSL_HASH_LINK_TO_OBJECT(RDMCachedService, _idLink, pHashLink);
									if (pService->rdm.state.serviceState == 0 || pService->rdm.state.acceptingRequests == 0)
									{
										/* Returns as the service of the active channel is not accepting request yet.*/
										(*pendingWaitCount)++;
										return RSSL_RET_SUCCESS;
									}
								}
								else
								{
									/* Returns as the active channel doesn't have this service yet.*/
									(*pendingWaitCount)++;
									return RSSL_RET_SUCCESS;
								}
							}
						}
					}

					requestMsg.msgBase.msgKey.flags |= RSSL_MKF_HAS_SERVICE_ID;
					requestMsg.msgBase.msgKey.serviceId = 
						(RsslUInt16)pService->serviceId;
				}

				if (pItemStream->refreshState != WL_ISRS_REQUEST_REFRESH)
					requestMsg.flags |= RSSL_RQMF_NO_REFRESH;
				else
					sendMsg = RSSL_TRUE;

				/* Make request streaming if appropriate. */
				if (pItemStream->requestsStreamingCount)
				{
					requestMsg.flags |= RSSL_RQMF_STREAMING;

					/* Update priority if needed. */
					if (pItemStream->flags & WL_IOSF_PENDING_PRIORITY_CHANGE
							&& wlItemStreamMergePriority(pItemStream, &requestMsg.priorityClass,
								&requestMsg.priorityCount))
					{
						requestMsg.flags |= RSSL_RQMF_HAS_PRIORITY;
						sendMsg = RSSL_TRUE;
					}

					/* Check if pause flag should be set. */
					if (pWatchlistImpl->base.config.supportOptimizedPauseResume
							&& pItemStream->requestsPausedCount
								== pItemStream->requestsStreamingCount)
					{
						if (!(pItemStream->flags & WL_IOSF_PAUSED))
						{
							sendMsg = RSSL_TRUE;
							pItemStream->flags |= WL_IOSF_PAUSED;
						}
						requestMsg.flags |= RSSL_RQMF_PAUSE;
					}
					else
					{
						if (pItemStream->flags & WL_IOSF_PAUSED)
						{
							sendMsg = RSSL_TRUE;
							pItemStream->flags &= ~WL_IOSF_PAUSED;
						}
					}


				}

				if (pWatchlistImpl->base.config.supportViewRequests)
				{
					if (pItemStream->requestsWithViewCount
							== pItemStream->requestsRecovering.count
							+ pItemStream->requestsPendingRefresh.count
							+ pItemStream->requestsOpen.count)
					{
						/* If all streams have a view, the view flag should be set on the
						 * request. */
						hasViewFlag = RSSL_TRUE;

						/* If we are trying to change the view, and it is different
						 * than our last committed one, provide the new view for encoding. */
						if (pItemStream->flags & WL_IOSF_PENDING_VIEW_CHANGE
								&& !(pItemStream->flags & WL_IOSF_PENDING_VIEW_REFRESH))
						{
							RsslBool updated;

							/* View needs to be set or updated */
							if ((ret = wlAggregateViewMerge(pItemStream->pAggregateView,
											&updated, pError))
									!= RSSL_RET_SUCCESS)
								return ret;

							 if (updated || !(pItemStream->flags & WL_IOSF_VIEWED))
							 {
								 pView = pItemStream->pAggregateView;
								 sendMsg = RSSL_TRUE;
							 }
						}
					}
					else if (pItemStream->flags & WL_IOSF_VIEWED) /* Remove view. */
					{
						pItemStream->flags |= WL_IOSF_PENDING_VIEW_CHANGE;
						sendMsg = RSSL_TRUE;
					}
				}
				else
					pView = NULL;

				if (pItemStream->flags & WL_IOSF_PRIVATE)
					requestMsg.flags |= RSSL_RQMF_PRIVATE_STREAM;

				if (pItemStream->flags & WL_IOSF_QUALIFIED)
					requestMsg.flags |= RSSL_RQMF_QUALIFIED_STREAM;

				if (pItemStream->pRequestWithExtraInfo != NULL)
				{
					/* Add any data body or extended header to this message. */
					requestMsg.msgBase.containerType = 
						pItemStream->pRequestWithExtraInfo->containerType;
					requestMsg.msgBase.encDataBody = 
						pItemStream->pRequestWithExtraInfo->encDataBody;

					if (pItemStream->pRequestWithExtraInfo->requestMsgFlags & RSSL_RQMF_HAS_EXTENDED_HEADER)
					{
						requestMsg.flags |= RSSL_RQMF_HAS_EXTENDED_HEADER;
						requestMsg.extendedHeader = pItemStream->pRequestWithExtraInfo->extendedHeader;
					}

					sendMsg = RSSL_TRUE;
				}

				if (!sendMsg) ret = RSSL_RET_SUCCESS;
				else ret = wlEncodeAndSubmitMsg(pWatchlistImpl, (RsslMsg*)&requestMsg, NULL, 
						hasViewFlag, pView, pError);

				if (ret >= RSSL_RET_SUCCESS)
				{
					if (!(requestMsg.flags & RSSL_RQMF_NO_REFRESH))
					{
						RsslQueueLink *pLink;
						RSSL_QUEUE_FOR_EACH_LINK(&pItemStream->requestsRecovering, pLink)
						{
							WlRequest *pRequest = RSSL_QUEUE_LINK_TO_OBJECT(WlRequest, base.qlStateQueue, pLink);
							pRequest->base.pStateQueue = &pItemStream->requestsPendingRefresh;
						}

						assert(pItemStream->refreshState == WL_ISRS_REQUEST_REFRESH);
						rsslQueueAppend(&pItemStream->requestsPendingRefresh,
								&pItemStream->requestsRecovering);
						pItemStream->refreshState = WL_ISRS_PENDING_REFRESH;

						if (!(requestMsg.flags & RSSL_RQMF_STREAMING))
							pItemStream->flags |= WL_IOSF_PENDING_SNAPSHOT;

						/* Restart buffering. */
						if (pItemStream->flags & WL_IOSF_HAS_BC_SEQ_NUM)
						{
							assert(pItemStream->flags & WL_IOSF_HAS_UC_SEQ_NUM);
							/* Use the last broadcast sequence number as the
							 * new starting point instead of the original. */
							pItemStream->flags &= ~WL_IOSF_HAS_BC_SEQ_NUM;
						}

						if (pWatchlistImpl->base.pRsslChannel)
							wlSetStreamPendingResponse(&pWatchlistImpl->base, &pItemStream->base);

					}

					wlUnsetStreamMsgPending(&pWatchlistImpl->base, &pItemStream->base);

					/* If we sent new priority info, commit the change. */
					pItemStream->flags &= ~WL_IOSF_PENDING_PRIORITY_CHANGE;
					if (requestMsg.flags & RSSL_RQMF_HAS_PRIORITY)
					{
						pItemStream->priorityClass = requestMsg.priorityClass;
						pItemStream->priorityCount = requestMsg.priorityCount;
					}

					/* New encDataBody/extendedHeader sent (if it was present). */
					pItemStream->pRequestWithExtraInfo = NULL;

					if (pWatchlistImpl->base.config.supportViewRequests)
					{
						if (pItemStream->flags & WL_IOSF_PENDING_VIEW_CHANGE)
						{
							/* Sent the new view, commit changes. */
							pItemStream->flags &= ~WL_IOSF_PENDING_VIEW_CHANGE;

							if (!(requestMsg.flags & RSSL_RQMF_NO_REFRESH))
								pItemStream->flags |= WL_IOSF_PENDING_VIEW_REFRESH;

							if (pView) 
							{
								pItemStream->flags |= WL_IOSF_VIEWED;
								wlAggregateViewCommitViews(pItemStream->pAggregateView);
							}
							else if (!hasViewFlag)
								pItemStream->flags &= ~WL_IOSF_VIEWED;

							/* Destroy view if no longer needed. */
							if(pItemStream->pAggregateView
									&& pItemStream->requestsWithViewCount == 0)
							{
								wlAggregateViewDestroy(pItemStream->pAggregateView);
								pItemStream->pAggregateView = NULL;
							}
						}
					}

					return ret;
				}

				break;
			}
		}

	}

	return ret;
}

static RsslRet wlSendRefreshEventToItemRequest(RsslWatchlistImpl *pWatchlistImpl,
		RsslWatchlistMsgEvent *pEvent, WlItemRequest *pItemRequest, RsslErrorInfo *pErrorInfo)
{
	RsslRefreshMsg *pRefreshMsg;
	RsslRet ret;
	assert(pEvent->pRsslMsg->msgBase.msgClass == RSSL_MC_REFRESH);

	pRefreshMsg = &pEvent->pRsslMsg->refreshMsg;
	if (pRefreshMsg->flags & RSSL_RFMF_SOLICITED && pItemRequest->flags & WL_IRQF_REFRESHED)
	{
		pRefreshMsg->flags &= ~RSSL_RFMF_SOLICITED;
		ret = wlSendMsgEventToItemRequest(pWatchlistImpl, pEvent, pItemRequest, pErrorInfo);
		pRefreshMsg->flags |= RSSL_RFMF_SOLICITED;
		return ret;
	}
	else
	{
		if (pRefreshMsg->flags & RSSL_RFMF_SOLICITED && 
				pRefreshMsg->flags & RSSL_RFMF_REFRESH_COMPLETE)
			pItemRequest->flags |= WL_IRQF_REFRESHED;
		return wlSendMsgEventToItemRequest(pWatchlistImpl, pEvent, pItemRequest, pErrorInfo);
	}

}

RsslRet wlSendMsgEventToItemRequest(RsslWatchlistImpl *pWatchlistImpl,
		RsslWatchlistMsgEvent *pEvent, WlItemRequest *pItemRequest, RsslErrorInfo *pErrorInfo)
{
	switch(pItemRequest->base.domainType)
	{
		case RSSL_DMT_SYMBOL_LIST:
		{
			RsslRet ret;
			if ((ret = wlItemRequestSendMsgEvent(&pWatchlistImpl->base, pEvent, pItemRequest, 
							pErrorInfo)) != RSSL_RET_SUCCESS)
				return ret;

			return wlProcessSymbolListMsg(&pWatchlistImpl->base, &pWatchlistImpl->items,
					pEvent->pRsslMsg, (WlSymbolListRequest*)pItemRequest, pErrorInfo);
		}
		default:
			return wlItemRequestSendMsgEvent(&pWatchlistImpl->base, pEvent, pItemRequest, 
					pErrorInfo);
	}
}

RsslRet wlProcessItemRequest(RsslWatchlistImpl *pWatchlistImpl, WlItemRequestCreateOpts *pOpts,
		WlItemRequest *pExistingRequest, RsslErrorInfo *pErrorInfo)
{
	RsslRet ret;

	assert(!pOpts->viewElemList);

	if (pOpts->pRequestMsg->flags & RSSL_RQMF_HAS_VIEW)
	{
		if ((ret = wlExtractViewFromMsg(&pWatchlistImpl->base, pOpts, pErrorInfo)) != RSSL_RET_SUCCESS)			
			return ret;
	}

	if (pExistingRequest)
	{
		switch(pOpts->pRequestMsg->msgBase.domainType)
		{
			case RSSL_DMT_SYMBOL_LIST:
				return wlSymbolListRequestReissue(&pWatchlistImpl->base, &pWatchlistImpl->items,
							(WlSymbolListRequest*)pExistingRequest, pOpts, pErrorInfo);
			default:
				return wlItemRequestReissue(&pWatchlistImpl->base, &pWatchlistImpl->items,
							pExistingRequest, pOpts, pErrorInfo);
		}
	}
	else
	{

		if (pOpts->pRequestMsg->flags & RSSL_RQMF_NO_REFRESH)
		{
			rsslSetErrorInfo(pErrorInfo, RSSL_EIC_FAILURE, RSSL_RET_INVALID_DATA, __FILE__, __LINE__, 
					"Cannot open request without requesting refresh.");
			return RSSL_RET_INVALID_DATA;
		}

		if (pOpts->pRequestMsg->msgBase.streamId < 0)
		{
			rsslSetErrorInfo(pErrorInfo, RSSL_EIC_FAILURE, RSSL_RET_INVALID_DATA, __FILE__, __LINE__, 
					"Cannot open request with negative stream ID.");
			return RSSL_RET_INVALID_DATA;
		}

		if (pOpts->pRequestMsg->msgBase.msgKey.flags & RSSL_MKF_HAS_SERVICE_ID
				&& pOpts->pServiceName)
		{
			rsslSetErrorInfo(pErrorInfo, RSSL_EIC_FAILURE, RSSL_RET_INVALID_DATA, __FILE__, __LINE__, 
					"Cannot open request with both service ID and service name.");
			return RSSL_RET_INVALID_DATA;
		}

		if (pOpts->pRequestMsg->flags & RSSL_RQMF_HAS_QOS
				&& (pOpts->pRequestMsg->qos.timeliness == RSSL_QOS_TIME_UNSPECIFIED 
					|| pOpts->pRequestMsg->qos.rate == RSSL_QOS_RATE_UNSPECIFIED
					|| pOpts->pRequestMsg->qos.timeliness > RSSL_QOS_TIME_DELAYED
					|| pOpts->pRequestMsg->qos.rate > RSSL_QOS_RATE_TIME_CONFLATED))
		{
			rsslSetErrorInfo(pErrorInfo, RSSL_EIC_FAILURE, RSSL_RET_INVALID_DATA, __FILE__, __LINE__, 
					"Request has invalid QoS (Timeliness: %u, Rate: %u).", 
					pOpts->pRequestMsg->qos.timeliness, pOpts->pRequestMsg->qos.rate);
			return RSSL_RET_INVALID_DATA;
		}

		if (pOpts->pRequestMsg->flags & RSSL_RQMF_HAS_WORST_QOS
				&& (pOpts->pRequestMsg->worstQos.timeliness == RSSL_QOS_TIME_UNSPECIFIED 
					|| pOpts->pRequestMsg->worstQos.rate == RSSL_QOS_RATE_UNSPECIFIED
					|| pOpts->pRequestMsg->worstQos.timeliness > RSSL_QOS_TIME_DELAYED
					|| pOpts->pRequestMsg->worstQos.rate > RSSL_QOS_RATE_TIME_CONFLATED))
		{
			rsslSetErrorInfo(pErrorInfo, RSSL_EIC_FAILURE, RSSL_RET_INVALID_DATA, __FILE__, __LINE__, 
					"Request has invalid worst QoS (Timeliness: %u, Rate: %u).", 
					pOpts->pRequestMsg->worstQos.timeliness, pOpts->pRequestMsg->worstQos.rate);
			return RSSL_RET_INVALID_DATA;
		}

		if (!(pOpts->pRequestMsg->flags & RSSL_RQMF_HAS_BATCH)) 
		{
			switch(pOpts->pRequestMsg->msgBase.domainType)
			{
				case RSSL_DMT_SYMBOL_LIST:
					return wlSymbolListRequestCreate(&pWatchlistImpl->base, 
							&pWatchlistImpl->items, pOpts, pErrorInfo);
				default:
					return wlItemRequestCreate(&pWatchlistImpl->base, &pWatchlistImpl->items,
							pOpts, pErrorInfo);
			}
		}
		else
			return wlProcessItemBatchRequest(pWatchlistImpl, pOpts, pErrorInfo);
	}

}

static RsslRet wlProcessItemBatchRequest(RsslWatchlistImpl *pWatchlistImpl, 
		WlItemRequestCreateOpts *pOpts, RsslErrorInfo *pErrorInfo)
{
	RsslDecodeIterator dIter;
	RsslRet ret;
	RsslBool foundBatch = RSSL_FALSE;
	RsslArray batchArray;
	RsslElementList elementList;
	RsslElementEntry elementEntry;
	RsslRequestMsg *pOrigRequestMsg = pOpts->pRequestMsg;
	RsslRequestMsg requestMsg = *pOrigRequestMsg;
	WlItemRequest *pItemRequest;
	RsslHashLink *pHashLink;

	if (requestMsg.msgBase.msgKey.flags & RSSL_MKF_HAS_NAME)
	{
		rsslSetErrorInfo(pErrorInfo, RSSL_EIC_FAILURE, RSSL_RET_INVALID_DATA, __FILE__, __LINE__, 
				"Requested batch has name in message key.");
		return RSSL_RET_INVALID_DATA;
	}

	/* setup decode iterator */
	rsslClearDecodeIterator(&dIter);
	rsslSetDecodeIteratorRWFVersion(&dIter, pOpts->majorVersion, pOpts->minorVersion);
	rsslSetDecodeIteratorBuffer(&dIter, &pOrigRequestMsg->msgBase.encDataBody);

	rsslClearElementList(&elementList);

	/* check the container type and decode element list */ 
	if (pOrigRequestMsg->msgBase.containerType == RSSL_DT_ELEMENT_LIST && 
			(ret = rsslDecodeElementList(&dIter, &elementList, 0)) == RSSL_RET_SUCCESS)
	{
		rsslClearElementEntry(&elementEntry);

		/* find and decode the :ItemList */
		while ((ret = rsslDecodeElementEntry(&dIter, &elementEntry)) != RSSL_RET_END_OF_CONTAINER)
		{
			if (ret < RSSL_RET_SUCCESS)
			{
				rsslSetErrorInfo(pErrorInfo, RSSL_EIC_FAILURE, ret, __FILE__, __LINE__,
						"Error decoding element entry in Request Msg -- %d.", ret);
				return ret;
			}
			else
			{
				/* if found :ItemList break out of the loop */
				if (rsslBufferIsEqual(&elementEntry.name, &RSSL_ENAME_BATCH_ITEM_LIST) &&
						elementEntry.dataType == RSSL_DT_ARRAY)
				{
					foundBatch = RSSL_TRUE;
					break;
				}
			}
			rsslClearElementEntry(&elementEntry);
		}
	}
	else
	{
		rsslSetErrorInfo(pErrorInfo, RSSL_EIC_FAILURE, pOrigRequestMsg->msgBase.containerType, __FILE__, __LINE__,
				"Unexpected container type: -- %d, or decoding error -- %d.", pOrigRequestMsg->msgBase.containerType, ret);
		return RSSL_RET_FAILURE;
	}

	if (foundBatch)
	{
		if ((ret = rsslDecodeArray(&dIter, &batchArray)) == RSSL_RET_SUCCESS)
		{
			RsslBuffer arrayBuffer;

			/* Start at stream ID after batch request. */
			++requestMsg.msgBase.streamId;
			requestMsg.msgBase.msgKey.flags |= RSSL_MKF_HAS_NAME;
			pOpts->pRequestMsg = &requestMsg;

			if (batchArray.primitiveType != RSSL_DT_ASCII_STRING)
			{
				rsslSetErrorInfo(pErrorInfo, RSSL_EIC_FAILURE, batchArray.primitiveType, __FILE__, __LINE__,
						"Unexpected primitive type in array -- %d.", batchArray.primitiveType);
				return RSSL_RET_FAILURE;
			}

			while ((ret = rsslDecodeArrayEntry(&dIter, &arrayBuffer)) != RSSL_RET_END_OF_CONTAINER) 
			{
				if (ret < RSSL_RET_SUCCESS)
				{
					rsslSetErrorInfo(pErrorInfo, RSSL_EIC_FAILURE, ret, __FILE__, __LINE__,
							"Error decoding array entry in Request Msg -- %d.", ret);
					break;
				}
				else
				{
					if (rsslHashTableFind(&pWatchlistImpl->base.requestsByStreamId,
								&requestMsg.msgBase.streamId, NULL))
					{
						rsslSetErrorInfo(pErrorInfo, RSSL_EIC_FAILURE, RSSL_RET_INVALID_DATA, __FILE__, __LINE__, 
								"Item in batch has same ID as existing stream.");
						ret = RSSL_RET_INVALID_DATA;
						break;
					}

					if ((ret = rsslDecodeBuffer(&dIter, &requestMsg.msgBase.msgKey.name)) 
							== RSSL_RET_SUCCESS)
					{
						switch(pOrigRequestMsg->msgBase.domainType)
						{
							case RSSL_DMT_SYMBOL_LIST:
								ret = wlSymbolListRequestCreate(&pWatchlistImpl->base, 
										&pWatchlistImpl->items, pOpts, pErrorInfo);
								break;
							default:
								ret = wlItemRequestCreate(&pWatchlistImpl->base, &pWatchlistImpl->items,
										pOpts, pErrorInfo);
								break;
						}

						if (ret != RSSL_RET_SUCCESS)
							break;

						++requestMsg.msgBase.streamId;
					}
					else
					{
						rsslSetErrorInfo(pErrorInfo, RSSL_EIC_FAILURE, ret, __FILE__, __LINE__,
								"Invalid BLANK_DATA while decoding :ItemList -- %d.", ret);
						break;
					}
				}
			}

			if (ret == RSSL_RET_END_OF_CONTAINER)
			{
				/* Requests created. Make a request for the batch stream so it can be acknowledged. */
				if ((pItemRequest = (WlItemRequest*)rsslMemoryPoolGet(&pWatchlistImpl->base.requestPool, pErrorInfo)))
				{
					pOpts->pRequestMsg = pOrigRequestMsg;

					switch(pOrigRequestMsg->msgBase.domainType)
					{
						case RSSL_DMT_SYMBOL_LIST:
							if ((ret = wlSymbolListRequestInit((WlSymbolListRequest*)pItemRequest, &pWatchlistImpl->base, &pWatchlistImpl->items, pOpts, RSSL_FALSE, 0, pErrorInfo))
									== RSSL_RET_SUCCESS)
							{
								pItemRequest->flags |= WL_IRQF_BATCH;
								return RSSL_RET_SUCCESS;
							}
							break;
						default:
							if ((ret = wlItemRequestInit(pItemRequest, &pWatchlistImpl->base, &pWatchlistImpl->items, pOpts, pErrorInfo))
									== RSSL_RET_SUCCESS)
							{
								pItemRequest->flags |= WL_IRQF_BATCH;
								return RSSL_RET_SUCCESS;
							}
							break;
					}
				}
				else
					ret = pErrorInfo->rsslError.rsslErrorId;

				/* Batch request failed -- return request to pool. */
				rsslMemoryPoolPut(&pWatchlistImpl->base.requestPool, pItemRequest);
			}

			--requestMsg.msgBase.streamId;

			/* Batch request has failed. Remove requests made thus far. */
			for(; requestMsg.msgBase.streamId > pOrigRequestMsg->msgBase.streamId;
				--requestMsg.msgBase.streamId)
			{
				if ((pHashLink = rsslHashTableFind(&pWatchlistImpl->base.requestsByStreamId,
								&requestMsg.msgBase.streamId, NULL)))
				{
					WlItemRequest *pItemRequest = RSSL_HASH_LINK_TO_OBJECT(WlItemRequest,
							base.hlStreamId, pHashLink);
					wlItemRequestClose(&pWatchlistImpl->base, &pWatchlistImpl->items, pItemRequest);
					wlRequestedServiceCheckRefCount(&pWatchlistImpl->base, pItemRequest->pRequestedService);
					wlItemRequestDestroy(&pWatchlistImpl->base, pItemRequest);
				}
			}

			return ret;
		}
		else
		{
			rsslSetErrorInfo(pErrorInfo, RSSL_EIC_FAILURE, ret, __FILE__, __LINE__,
					"Error decoding array in Request Msg -- %d.", ret);
			return ret;
		}
	}
	else
	{
		rsslSetErrorInfo(pErrorInfo, RSSL_EIC_FAILURE, RSSL_RET_INCOMPLETE_DATA, __FILE__, __LINE__,
				":ItemList not found -- %d.", RSSL_RET_INCOMPLETE_DATA);
		return RSSL_RET_FAILURE;
	}
}

