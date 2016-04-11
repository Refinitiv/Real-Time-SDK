/*
 * This source code is provided under the Apache 2.0 license and is provided
 * AS IS with no warranty or guarantee of fit for purpose.  See the project's 
 * LICENSE.md for details. 
 * Copyright Thomson Reuters 2015. All rights reserved.
*/

#include "rtr/rsslWatchlistImpl.h"
#include <assert.h>

static RsslRet wlWriteBuffer(RsslWatchlistImpl *pWatchlistImpl, RsslBuffer *pWriteBuffer,
		RsslErrorInfo *pError);

/*** Main interface functions. ***/

RsslWatchlist *rsslWatchlistCreate(RsslWatchlistCreateOptions *pCreateOptions, 
		RsslErrorInfo *pErrorInfo)
{
	RsslWatchlistImpl				*pWatchlistImpl;
	WlBaseInitOptions				baseInitOpts;
	WlItemsInitOptions				itemsInitOpts;
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

	if ((ret = wlBaseInit(&pWatchlistImpl->base, &baseInitOpts, pErrorInfo)) != RSSL_RET_SUCCESS)
	{
		free(pWatchlistImpl);
		return NULL;
	}

	wlClearItemsInitOptions(&itemsInitOpts);
	itemsInitOpts.maxOutstandingPosts = pCreateOptions->maxOutstandingPosts;
	itemsInitOpts.postAckTimeout = pCreateOptions->postAckTimeout;
	if (wlItemsInit(&pWatchlistImpl->items, &itemsInitOpts, pErrorInfo) != RSSL_RET_SUCCESS)
	{
		rsslWatchlistDestroy((RsslWatchlist*)pWatchlistImpl);
		return NULL;
	}

	pWatchlistImpl->login.pRequest = NULL;
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
		pWatchlistImpl->base.channelState = WL_CHS_START;
		pWatchlistImpl->base.watchlist.state = 0;
		if (pWatchlistImpl->login.pStream)
			pWatchlistImpl->login.pStream->flags &= ~WL_LSF_ESTABLISHED;
		pWatchlistImpl->items.pCurrentFanoutStream = NULL;
		pWatchlistImpl->items.pCurrentFanoutGroup = NULL;
		pWatchlistImpl->items.pCurrentFanoutFTGroup = NULL;
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
		wlLoginRequestDestroy(pWatchlistImpl->login.pRequest);

	if (pWatchlistImpl->login.pStream)
		wlLoginStreamDestroy(pWatchlistImpl->login.pStream);

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
	RsslRet ret;
	RsslQueueLink *pLink;

	pWatchlistImpl->base.currentTime = currentTime;

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
						assert(pWatchlistImpl->login.pRequest);
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

		if ((ret = rsslWrite(pWatchlistImpl->base.pRsslChannel, pWatchlistImpl->base.pWriteCallAgainBuffer, 
						RSSL_HIGH_PRIORITY, 0, &bytes, &uncompBytes, &pErrorInfo->rsslError)) 
				< RSSL_RET_SUCCESS)
		{
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
			pWatchlistImpl->base.pWriteCallAgainBuffer = NULL;
			if (ret > 0)
				pWatchlistImpl->base.watchlist.state |= RSSLWL_STF_NEED_FLUSH;
		}
	}

	/* Send any item requests. */
	RSSL_QUEUE_FOR_EACH_LINK(&pWatchlistImpl->base.streamsPendingRequest,
			pLink)
	{
		WlStream *pStream = RSSL_QUEUE_LINK_TO_OBJECT(WlStream,
				base.qlStreamsPendingRequest, pLink);
		if ((ret = wlStreamSubmitMsg(pWatchlistImpl, pStream,
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

	return (pWatchlistImpl->base.streamsPendingRequest.count 
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

	if ((pLink = rsslQueuePeekFront(&pWatchlistImpl->items.postTable.timeoutQueue)))
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
			&& !rsslQueueGetElementCount(&pWatchlistImpl->items.postTable.timeoutQueue)
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

				msgEvent.pRsslMsg->msgBase.streamId = pWatchlistImpl->login.pStream->base.streamId;

				if ((ret = wlLoginProcessProviderMsg(&pWatchlistImpl->login, &pWatchlistImpl->base, 
								&dIter, (RsslMsg*)&statusMsg, &loginMsg, &loginAction, pErrorInfo)) 
						!= RSSL_RET_SUCCESS)
					return ret;

				/* Check correct behavior based on SingleOpen configuration. */
				assert(pWatchlistImpl->base.config.singleOpen && loginAction == WL_LGPA_RECOVER
						|| !pWatchlistImpl->base.config.singleOpen && loginAction == WL_LGPA_CLOSE);

				/* Close old login stream */
				wlLoginStreamClose(&pWatchlistImpl->base, &pWatchlistImpl->login, RSSL_TRUE);

				if (pWatchlistImpl->base.config.singleOpen)
				{
					/* Create new login stream. */
					statusMsg.state.streamState = RSSL_STREAM_OPEN;
					if (!(pWatchlistImpl->login.pStream = wlLoginStreamCreate(
									&pWatchlistImpl->base, &pWatchlistImpl->login, pErrorInfo)))
						return pErrorInfo->rsslError.rsslErrorId;
					pWatchlistImpl->login.pRequest->base.pStream = &pWatchlistImpl->login.pStream->base;
				}
				else
					pWatchlistImpl->login.pRequest->base.pStream = NULL;

				msgEvent.pRsslMsg = NULL;
				msgEvent.pRdmMsg = (RsslRDMMsg*)&loginMsg;
				loginMsg.rdmMsgBase.streamId = pWatchlistImpl->login.pRequest->base.streamId;
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

	while ((pLink = rsslQueuePeekFront(&pWatchlistImpl->items.postTable.timeoutQueue)))
	{
		WlPostRecord *pRecord = RSSL_QUEUE_LINK_TO_OBJECT(WlPostRecord, qlTimeout, pLink);
		WlItemRequest *pItemRequest;

		if (pRecord->expireTime > currentTime) 
		{
			pWatchlistImpl->base.watchlist.state |= RSSLWL_STF_NEED_TIMER;
			break;
		}

		pItemRequest = (WlItemRequest*)pRecord->pUserSpec;
		ackMsg.msgBase.domainType = pItemRequest->base.domainType;
		ackMsg.flags = RSSL_AKMF_HAS_TEXT | RSSL_AKMF_HAS_NAK_CODE;
		ackMsg.ackId = pRecord->postId;

		if (pRecord->flags & RSSL_PSMF_HAS_SEQ_NUM)
		{
			ackMsg.flags |= RSSL_AKMF_HAS_SEQ_NUM;
			ackMsg.seqNum = pRecord->seqNum;
		}

		rsslQueueRemoveLink(&pItemRequest->openPosts, &pRecord->qlUser);
		wlPostTableRemoveRecord(&pWatchlistImpl->items.postTable, pRecord);

		if ((ret = wlSendMsgEventToItemRequest(pWatchlistImpl, &msgEvent, pItemRequest, pErrorInfo))
				!= RSSL_RET_SUCCESS)
			return ret;

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
				/* Service was deleted. All items should be considered ClosedRecover. */

				assert(pService->pUserSpec);
				pWlService = (WlService*)pService->pUserSpec;

				if (pWlService)
				{
					/* Separate requested services from this service now, so that they
					 * don't attempt to recover to it. */
					RsslQueueLink *pRequestedServiceLink;

					RSSL_QUEUE_FOR_EACH_LINK(&pWlService->requestedServices, pRequestedServiceLink)
					{
						WlRequestedService *pRequestedService = 
							RSSL_QUEUE_LINK_TO_OBJECT(WlRequestedService, qlDirectoryRequests,
									pRequestedServiceLink);
						rsslQueueRemoveLink(&pRequestedService->pMatchingService->requestedServices,
								&pRequestedService->qlDirectoryRequests);
						pRequestedService->pMatchingService = NULL;
					}

					if ((ret = wlProcessRemovedService(pWatchlistImpl, pWlService, pErrorInfo))
								!= RSSL_RET_SUCCESS)
						return ret;

				}
				break;
			}
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
			WlService *pWlService = (WlService*)pService->pUserSpec;
			rsslQueueRemoveLink(&pWatchlistImpl->services, &pWlService->qlServices);
			wlServiceDestroy(pWlService);

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

	RSSL_QUEUE_FOR_EACH_LINK(&pWlService->openStreamList, pLink)
	{
		WlItemStream *pStream = RSSL_QUEUE_LINK_TO_OBJECT(WlItemStream, qlServiceStreams,
				pLink);

		statusMsg.msgBase.domainType = pStream->base.domainType;
		if ((ret = wlFanoutItemMsgEvent(pWatchlistImpl, pStream, &msgEvent, 
						pErrorInfo)) 
				!= RSSL_RET_SUCCESS)
			return ret;
		pWatchlistImpl->items.pCurrentFanoutStream = NULL;
	}

	return RSSL_RET_SUCCESS;
}

static RsslRet wlRecoverAllItems(RsslWatchlistImpl *pWatchlistImpl, RsslErrorInfo *pErrorInfo)
{
	return wlServiceCacheClear(pWatchlistImpl->base.pServiceCache, RSSL_TRUE, pErrorInfo);
}

RsslRet rsslWatchlistReadMsg(RsslWatchlist *pWatchlist, 
		RsslWatchlistProcessMsgOptions *pOptions, RsslInt64 currentTime, RsslErrorInfo *pErrorInfo)
{
	RsslRet			ret;
	RsslHashLink	*pHashLink;
	WlStream		*pStream;
	RsslWatchlistMsgEvent msgEvent;
	RsslWatchlistImpl *pWatchlistImpl = (RsslWatchlistImpl*)pWatchlist;

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
			WlLoginRequest *pLoginRequest = pWatchlistImpl->login.pRequest;

			if (msgEvent.pRsslMsg->msgBase.domainType != RSSL_DMT_LOGIN)
			{

				wlStreamInfoClear(&streamInfo);
				streamInfo.pUserSpec = pLoginRequest->base.pUserSpec;
				msgEvent.pStreamInfo = &streamInfo;

				/* May be an off-stream post acknowledgement, pass it through. */
				switch(msgEvent.pRsslMsg->msgBase.msgClass)
				{
					case RSSL_MC_ACK:
						msgEvent.pRsslMsg->msgBase.streamId = pLoginRequest->base.streamId;

						return (*pWatchlistImpl->base.config.msgCallback)((RsslWatchlist*)&pWatchlistImpl->base.watchlist, 
								&msgEvent, pErrorInfo);
					default:
						rsslSetErrorInfo(pErrorInfo, RSSL_EIC_FAILURE, RSSL_RET_FAILURE, __FILE__, __LINE__, 
								"Received unsupported off-stream message on login stream with domain type %u and RsslMsg class %u.",
								msgEvent.pRsslMsg->msgBase.domainType, msgEvent.pRsslMsg->msgBase.msgClass);
						return RSSL_RET_FAILURE;
				}
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

					msgEvent.pRsslMsg->msgBase.streamId = 
						loginMsg.rdmMsgBase.streamId = pLoginRequest->base.streamId;
					if ((ret = (*pWatchlistImpl->base.config.msgCallback)
								((RsslWatchlist*)&pWatchlistImpl->base.watchlist, &msgEvent, pErrorInfo)) 
							!= RSSL_RET_SUCCESS)
						return ret;

					/* Recover the login stream. */
					wlRecoverAllItems(pWatchlistImpl, pErrorInfo);
					wlSetStreamMsgPending(&pWatchlistImpl->base,
							&pWatchlistImpl->login.pStream->base);
					break;

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

					pWatchlistImpl->login.pRequest = NULL;

					/* Forward processed message to application. */
					msgEvent.pRsslMsg->msgBase.streamId = 
						loginMsg.rdmMsgBase.streamId = pLoginRequest->base.streamId;
					if ((ret = (*pWatchlistImpl->base.config.msgCallback)
								((RsslWatchlist*)&pWatchlistImpl->base.watchlist, &msgEvent, pErrorInfo)) 
							!= RSSL_RET_SUCCESS)
						return ret;

					/* Destroy login stream. */
					wlLoginStreamDestroy(pLoginStream);
					wlLoginRequestDestroy(pLoginRequest);

					/* Close directory stream, if present. */
					pDirectoryStream = pWatchlistImpl->directory.pStream;
					if (pDirectoryStream)
					{
						wlDirectoryStreamClose(&pWatchlistImpl->base,
								&pWatchlistImpl->directory, RSSL_FALSE);
						wlDirectoryStreamDestroy(pDirectoryStream);
					}

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

			if ((pRecord = wlPostTableFindRecord(&pWatchlistImpl->items.postTable, 
						&pRsslMsg->ackMsg)))
			{
				WlItemRequest *pItemRequest = (WlItemRequest*)pRecord->pUserSpec;
				if ((ret = wlSendMsgEventToItemRequest(pWatchlistImpl, pEvent, pItemRequest, pErrorInfo))
							!= RSSL_RET_SUCCESS)
						return ret;

				rsslQueueRemoveLink(&pItemRequest->openPosts, &pRecord->qlUser);
				wlPostTableRemoveRecord(&pWatchlistImpl->items.postTable, pRecord);
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
		while (pLink = rsslQueueRemoveFirstLink(&pItemRequest->openPosts))
		{
			WlPostRecord *pPostRecord = RSSL_QUEUE_LINK_TO_OBJECT(WlPostRecord, qlUser, pLink);
			wlPostTableRemoveRecord(&pWatchlistImpl->items.postTable, pPostRecord);
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
						if (pRequest)
						{
							/* On-stream post. */
							RsslPostMsg postMsg = pOptions->pRsslMsg->postMsg;
							WlItemRequest *pItemRequest = (WlItemRequest*)pRequest;
							WlItemStream *pItemStream = (WlItemStream*)pItemRequest->base.pStream;

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


							if ( pItemRequest->flags & WL_IRQF_REFRESHED &&
									pItemStream && pItemStream->flags & WL_IOSF_ESTABLISHED)
							{
								postMsg.msgBase.streamId = pItemStream->base.streamId;

								if (postMsg.flags & RSSL_PSMF_ACK)
								{
									WlPostRecord *pPostRecord;

									if (!(pPostRecord = wlPostTableAddRecord(&pWatchlistImpl->base, 
													&pWatchlistImpl->items.postTable, &postMsg, 
													pErrorInfo)))
										return pErrorInfo->rsslError.rsslErrorId;

									pPostRecord->pUserSpec = (void*)pItemRequest;

									if ((ret = wlEncodeAndSubmitMsg(pWatchlistImpl, (RsslMsg*)&postMsg,
													NULL, RSSL_FALSE, NULL, pErrorInfo)) < RSSL_RET_SUCCESS)
									{
										wlPostTableRemoveRecord(&pWatchlistImpl->items.postTable, pPostRecord);
										return ret;
									}

									rsslQueueAddLinkToBack(&pItemRequest->openPosts,
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
							}
							else
							{
								rsslSetErrorInfo(pErrorInfo, RSSL_EIC_FAILURE, RSSL_RET_INVALID_DATA, __FILE__, __LINE__, 
										"Post message submitted to stream that is not established.");
								return RSSL_RET_INVALID_DATA;
							}
						}
						else if (pWatchlistImpl->login.pRequest
								&& pOptions->pRsslMsg->msgBase.streamId 
								== pWatchlistImpl->login.pRequest->base.streamId)
						{
							RsslPostMsg postMsg = pOptions->pRsslMsg->postMsg;

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

							/* Off-stream post */
							if (pWatchlistImpl->login.pStream 
									&& pWatchlistImpl->login.pStream->flags & WL_LSF_ESTABLISHED)
							{
								postMsg.msgBase.streamId = pWatchlistImpl->login.pStream->base.streamId;
								if ((ret = wlEncodeAndSubmitMsg(pWatchlistImpl, (RsslMsg*)&postMsg,
										NULL, RSSL_FALSE, NULL, pErrorInfo)) != RSSL_RET_SUCCESS)
									return ret;
								break;
							}
							else
							{
								rsslSetErrorInfo(pErrorInfo, RSSL_EIC_FAILURE, RSSL_RET_INVALID_DATA, __FILE__, __LINE__, 
										"Off-stream post message submitted to login stream that is not established.");
								return RSSL_RET_INVALID_DATA;
							}
						}
						else
						{
							rsslSetErrorInfo(pErrorInfo, RSSL_EIC_FAILURE, RSSL_RET_INVALID_DATA, __FILE__, __LINE__, 
									"Post message submitted for unknown stream %d.", streamId);
							return RSSL_RET_INVALID_DATA;
						}

						/* Otherwise nowhere to send it -- drop the message. */

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

				if ((ret = wlLoginProcessConsumerMsg(&pWatchlistImpl->login, &pWatchlistImpl->base, 
								(RsslRDMLoginMsg*)pRdmMsg, pOptions->pUserSpec, &loginAction, pErrorInfo)) != RSSL_RET_SUCCESS)
					return ret;

				switch(loginAction)
				{

					case WL_LGCA_NONE:
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
						WlLoginRequest *pLoginRequest = (WlLoginRequest*)pWatchlistImpl->login.pRequest;
						RsslQueueLink *pLink;

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

								if (pItemRequest->base.pStream)
									wlItemStreamRemoveRequest(
											(WlItemStream*)pItemRequest->base.pStream, pItemRequest);

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
								if (pDirectoryRequest->pRequestedService)
								{
									if  (pDirectoryRequest->pRequestedService->flags & WL_RSVC_HAS_ID)
									{
										/* Match service ID (or lack of it) */
										if (!(pDirectoryReqMsg->flags & RDM_DR_RQF_HAS_SERVICE_ID)
												|| pDirectoryReqMsg->serviceId != 
												pDirectoryRequest->pRequestedService->serviceId)
										{
											rsslSetErrorInfo(pErrorInfo, RSSL_EIC_FAILURE, RSSL_RET_INVALID_ARGUMENT, 
													__FILE__, __LINE__, "Service ID does not match existing request.");
											return RSSL_RET_INVALID_ARGUMENT;
										}
									}
									else
									{
										/* Match service name (or lack of it) */
										if (!pOptions->pServiceName
												|| !rsslBufferIsEqual(pOptions->pServiceName,
													&pDirectoryRequest->pRequestedService->serviceName))
										{
											rsslSetErrorInfo(pErrorInfo, RSSL_EIC_FAILURE, RSSL_RET_INVALID_ARGUMENT, 
													__FILE__, __LINE__, "Service name does not match existing request.");
											return RSSL_RET_INVALID_ARGUMENT;
										}	
									}
								}
								else if (pDirectoryReqMsg->flags & RDM_DR_RQF_HAS_SERVICE_ID
										|| pOptions->pServiceName)
								{
									rsslSetErrorInfo(pErrorInfo, RSSL_EIC_FAILURE, RSSL_RET_INVALID_ARGUMENT, 
											__FILE__, __LINE__, "Service info does not match existing request.");
									return RSSL_RET_INVALID_ARGUMENT;
								}

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

	if ((ret = rsslWrite(pWatchlistImpl->base.pRsslChannel, pWriteBuffer, RSSL_HIGH_PRIORITY, 0, &bytes, &uncompBytes,
					&pError->rsslError)) < RSSL_RET_SUCCESS)
	{
		switch(ret)
		{
			case RSSL_RET_WRITE_FLUSH_FAILED:
				if (pWatchlistImpl->base.pRsslChannel->state == RSSL_CH_STATE_ACTIVE)
				{
					pWatchlistImpl->base.watchlist.state |= RSSLWL_STF_NEED_FLUSH;
					return 1;
				}
				else
					return ret;
			case RSSL_RET_WRITE_CALL_AGAIN:
				assert(!pWatchlistImpl->base.pWriteCallAgainBuffer);
				pWatchlistImpl->base.watchlist.state |= RSSLWL_STF_NEED_FLUSH;
				pWatchlistImpl->base.pWriteCallAgainBuffer = pWriteBuffer;
				return RSSL_RET_SUCCESS;
			default:
				rsslSetErrorInfoLocation(pError, __FILE__, __LINE__);
				return ret;

		}
	}
	else if (ret > RSSL_RET_SUCCESS)
		pWatchlistImpl->base.watchlist.state |= RSSLWL_STF_NEED_FLUSH;

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
		WlStream *pStream, RsslErrorInfo *pError)
{
	RsslRet ret;

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

				loginRequest = *pWatchlistImpl->login.pRequest->pLoginReqMsg;
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


					if (pWatchlistImpl->login.pRequest->pCurrentToken
							&& !pWatchlistImpl->login.pRequest->pCurrentToken->needRefresh)
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

