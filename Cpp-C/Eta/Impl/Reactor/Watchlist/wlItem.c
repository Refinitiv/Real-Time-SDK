/*
 * This source code is provided under the Apache 2.0 license and is provided
 * AS IS with no warranty or guarantee of fit for purpose.  See the project's 
 * LICENSE.md for details. 
 * Copyright (C) 2019, 2025 LSEG. All rights reserved.
*/

#include "rtr/wlItem.h"
#include "rtr/wlSymbolList.h"
#include "rtr/rsslReactorImpl.h"
#include <limits.h>

/* Saves extra item request info, such as encDataBody and extendedHeader. 
 * Ensure the WL_IRQF_PRIVATE flag is correctly marked on the WlItemRequest before calling.
 * Returns:
 * - RSSL_RET_SUCCESS if nothing was saved
 * - Value greater than 0 if something was saved
 * - Errors */
static RsslRet _wlItemRequestSaveExtraInfo(WlItemRequest *pItemRequest, RsslRequestMsg *pRequestMsg,
		RsslErrorInfo *pErrorInfo);

RsslUInt32 wlProviderRequestHashSum(void *pKey)
{
	RsslUInt32 i;
	RsslUInt32 hashSum = 0;
	WlItemRequest *pItemRequest = (WlItemRequest*)pKey;
	RsslMsgKey *pMsgKey = &pItemRequest->msgKey;

	if (pMsgKey->flags & RSSL_MKF_HAS_SERVICE_ID)
		hashSum += pMsgKey->serviceId;

	if (pMsgKey->flags & RSSL_MKF_HAS_NAME)
	{
		for(i = 0; i < pMsgKey->name.length; ++i)
		{
			hashSum = (hashSum << 4) + (RsslUInt32)pMsgKey->name.data[i];
			hashSum ^= (hashSum >> 12);
		}
	}

	if (pMsgKey->flags & RSSL_MKF_HAS_ATTRIB)
	{
		for(i = 0; i < pMsgKey->encAttrib.length; ++i)
		{
			hashSum = (hashSum << 4) + (RsslUInt32)pMsgKey->encAttrib.data[i];
			hashSum ^= (hashSum >> 12);
		}
	}

	return hashSum;
}

RsslBool wlProviderRequestHashCompare(void *pKey1, void *pKey2)
{
	WlItemRequest *pItemRequest1 = (WlItemRequest*)pKey1;
	WlItemRequest *pItemRequest2 = (WlItemRequest*)pKey2;

	/* Provider-driven requests should have exactly one QoS. */
	assert(pItemRequest1->requestMsgFlags & RSSL_RQMF_HAS_QOS
			&& !(pItemRequest1->requestMsgFlags & RSSL_RQMF_HAS_WORST_QOS));
	assert(pItemRequest2->requestMsgFlags & RSSL_RQMF_HAS_QOS
			&& !(pItemRequest2->requestMsgFlags & RSSL_RQMF_HAS_WORST_QOS));

	if (pItemRequest1->base.domainType != pItemRequest2->base.domainType)
		return RSSL_FALSE;

	if (!rsslQosIsEqual(&pItemRequest1->qos, &pItemRequest2->qos))
		return RSSL_FALSE;

	if (rsslCompareMsgKeys(&pItemRequest1->msgKey, &pItemRequest2->msgKey)
			!= RSSL_RET_SUCCESS)
		return RSSL_FALSE;

	/* Requests should have the same requested service attached. */
	if (pItemRequest1->pRequestedService != pItemRequest2->pRequestedService)
		return RSSL_FALSE;

	return RSSL_TRUE;

}


RsslRet wlItemsInit(WlItems *pItems, RsslErrorInfo *pErrorInfo)
{
	RsslRet ret;

	if ((ret = rsslHashTableInit(&pItems->providerRequestsByAttrib, 100003, 
			wlProviderRequestHashSum, wlProviderRequestHashCompare, RSSL_TRUE, pErrorInfo))
			!= RSSL_RET_SUCCESS)
		return ret;

	memset(pItems->ftGroupTable, 0, sizeof(pItems->ftGroupTable));

	
	rsslInitQueue(&pItems->ftGroupTimerQueue);
	rsslInitQueue(&pItems->gapStreamQueue);

	pItems->gapExpireTime = WL_TIME_UNSET;

	return RSSL_RET_SUCCESS;
}

void wlItemsCleanup(WlItems *pItems)
{
	rsslHashTableCleanup(&pItems->providerRequestsByAttrib);
}

RsslRet wlItemCopyKey(RsslMsgKey *pNewMsgKey, RsslMsgKey *pOldMsgKey, char **pMemoryBuffer,
		RsslErrorInfo *pErrorInfo)
{
	int bufferSize = 0;
	char *newMemoryBuffer = NULL;

	*pNewMsgKey = *pOldMsgKey;

	if (*pMemoryBuffer)
		free(*pMemoryBuffer);

	if (pOldMsgKey->flags & RSSL_MKF_HAS_NAME && pOldMsgKey->name.data)
		bufferSize += pOldMsgKey->name.length;

	if (pOldMsgKey->flags & RSSL_MKF_HAS_ATTRIB && pOldMsgKey->encAttrib.data)
		bufferSize += pOldMsgKey->encAttrib.length;

	if (bufferSize)
	{
		newMemoryBuffer = (char*)malloc(bufferSize);
		verify_malloc(newMemoryBuffer, pErrorInfo, RSSL_RET_FAILURE);
		*pMemoryBuffer = newMemoryBuffer;
	}

	if (pOldMsgKey->flags & RSSL_MKF_HAS_NAME && pOldMsgKey->name.length && pOldMsgKey->name.data)
	{
		pNewMsgKey->name.data = newMemoryBuffer;
		newMemoryBuffer += pNewMsgKey->name.length;
		memcpy(pNewMsgKey->name.data, pOldMsgKey->name.data, pNewMsgKey->name.length);
	}

	if (pOldMsgKey->flags & RSSL_MKF_HAS_ATTRIB && pOldMsgKey->encAttrib.length && pOldMsgKey->encAttrib.data)
	{
		pNewMsgKey->encAttrib.data = newMemoryBuffer;
		newMemoryBuffer += pNewMsgKey->encAttrib.length;
		memcpy(pNewMsgKey->encAttrib.data, pOldMsgKey->encAttrib.data, pNewMsgKey->encAttrib.length);
	}

	return RSSL_RET_SUCCESS;
}

RsslRet wlItemRequestInit(WlItemRequest *pItemRequest, WlBase *pBase, WlItems *pItems, WlItemRequestCreateOpts *pOpts,
		RsslErrorInfo *pErrorInfo)
{
	RsslRet ret;
	WlRequestedService *pRequestedService;
	RsslRequestMsg *pRequestMsg = pOpts->pRequestMsg;

	memset(pItemRequest, 0, sizeof(WlItemRequest));

	rsslInitQueue(&pItemRequest->base.openPosts);

	pItemRequest->requestMsgFlags = pRequestMsg->flags;

	if ((ret = wlItemCopyKey(&pItemRequest->msgKey, 
			&pRequestMsg->msgBase.msgKey, &pItemRequest->msgKeyMemoryBuffer, pErrorInfo)) 
			!= RSSL_RET_SUCCESS)
		return ret;

	if (pItemRequest->requestMsgFlags & RSSL_RQMF_HAS_QOS)
		pItemRequest->qos = pRequestMsg->qos;

	if (pItemRequest->requestMsgFlags & RSSL_RQMF_HAS_WORST_QOS)
		pItemRequest->worstQos = pRequestMsg->worstQos;

	if (pItemRequest->requestMsgFlags & RSSL_RQMF_HAS_PRIORITY)
	{
		pItemRequest->priorityClass = pRequestMsg->priorityClass;
		pItemRequest->priorityCount = pRequestMsg->priorityCount;
	}
	else
	{
		pItemRequest->priorityClass = 1;
		pItemRequest->priorityCount = 1;
	}

	wlRequestBaseInit(&pItemRequest->base, 
			pRequestMsg->msgBase.streamId, 
			pRequestMsg->msgBase.domainType,
			pOpts->pUserSpec);

	switch(pOpts->viewAction)
	{
		case WL_IVA_SET:
			if (pOpts->viewType != RDM_VIEW_TYPE_FIELD_ID_LIST
					&& pOpts->viewType != RDM_VIEW_TYPE_ELEMENT_NAME_LIST)
			{
				rsslSetErrorInfo(pErrorInfo, RSSL_EIC_FAILURE, RSSL_RET_INVALID_DATA, __FILE__, __LINE__, 
						"Unknown view type %u.", pOpts->viewType);
				return RSSL_RET_INVALID_DATA;
			}
			if (!(pItemRequest->pView = wlViewCreate(pOpts->viewElemList, 
					pOpts->viewElemCount, pOpts->viewType, pErrorInfo)))
				return pErrorInfo->rsslError.rsslErrorId;
			break;
		case WL_IVA_MAINTAIN_VIEW:
			break;
		case WL_IVA_NONE:
			break;
		default:
			rsslSetErrorInfo(pErrorInfo, RSSL_EIC_FAILURE, RSSL_RET_INVALID_DATA, __FILE__, __LINE__, 
					"Unknown view action %u.", pOpts->viewAction);
			return RSSL_RET_INVALID_DATA;
	}

	if (pRequestMsg->flags & RSSL_RQMF_PRIVATE_STREAM)
		pItemRequest->flags |= WL_IRQF_PRIVATE;

	if (pRequestMsg->flags & RSSL_RQMF_QUALIFIED_STREAM)
		pItemRequest->flags |= WL_IRQF_QUALIFIED;

	if ((ret = _wlItemRequestSaveExtraInfo(pItemRequest, pRequestMsg, pErrorInfo))
			< RSSL_RET_SUCCESS)
	{
		wlItemRequestDestroy(pBase, pItemRequest);
		return ret;
	}

	/* Match/create service list. */
	if (pOpts->pServiceName)
	{
		if (!(pRequestedService = wlRequestedServiceOpen(pBase, pOpts->pServiceName, NULL, pErrorInfo)))
		{
			wlItemRequestDestroy(pBase, pItemRequest);
			return pErrorInfo->rsslError.rsslErrorId;
		}
	}
	else if (pRequestMsg->msgBase.msgKey.flags & RSSL_MKF_HAS_SERVICE_ID)
	{
		RsslUInt serviceId = pRequestMsg->msgBase.msgKey.serviceId;
		if (!(pRequestedService = wlRequestedServiceOpen(pBase, NULL, &serviceId, pErrorInfo)))
		{
			wlItemRequestDestroy(pBase, pItemRequest);
			return pErrorInfo->rsslError.rsslErrorId;
		}
	}
	else
	{
		rsslSetErrorInfo(pErrorInfo, RSSL_EIC_FAILURE, RSSL_RET_INVALID_ARGUMENT, __FILE__, __LINE__, 
				"Item request contained no service name nor service ID.");
		return RSSL_RET_FAILURE;
	}

	wlAddRequest(pBase, &pItemRequest->base);

	pItemRequest->base.pStateQueue = &pBase->newRequests;
	rsslQueueAddLinkToBack(&pBase->newRequests, &pItemRequest->base.qlStateQueue);
	pItemRequest->pRequestedService = pRequestedService;
	rsslQueueAddLinkToBack(&pRequestedService->itemRequests, &pItemRequest->qlRequestedService);

	return RSSL_RET_SUCCESS;
}

void wlItemRequestDestroy(WlBase *pBase, WlItemRequest *pItemRequest)
{
	wlItemRequestCleanup(pItemRequest);
	rsslMemoryPoolPut(&pBase->requestPool, pItemRequest);
}

void wlItemRequestEstablishQos(WlItemRequest *pItemRequest, RsslQos *pQos)
{
	if (pItemRequest->flags & WL_IRQF_HAS_STATIC_QOS 
			|| pItemRequest->qos.dynamic
			|| pItemRequest->requestMsgFlags & RSSL_RQMF_HAS_QOS
				&& !(pItemRequest->requestMsgFlags & RSSL_RQMF_HAS_WORST_QOS))
		return;

	pItemRequest->staticQos = *pQos;
	pItemRequest->flags |= WL_IRQF_HAS_STATIC_QOS;
}

const RsslQos *wlItemRequestMatchQos(WlItemRequest *pItemRequest, RsslQos *qosList, 
		RsslUInt32 qosCount)
{
	RsslQos *pSingleQos = NULL;
	RsslUInt32 ui;

	if (pItemRequest->flags & WL_IRQF_HAS_STATIC_QOS)
		pSingleQos = &pItemRequest->staticQos;
	else if ( pItemRequest->requestMsgFlags & RSSL_RQMF_HAS_QOS
			&& !(pItemRequest->requestMsgFlags & RSSL_RQMF_HAS_WORST_QOS))
		pSingleQos = &pItemRequest->qos;

	if (pSingleQos)
	{
		/* Does this single QoS appear in the given list? */
		if (!bsearch(pSingleQos, qosList, qosCount, sizeof(RsslQos), rscCompareQos))
			return NULL;

		return pSingleQos;
	}
	else
	{
		if (pItemRequest->requestMsgFlags & RSSL_RQMF_HAS_WORST_QOS)
		{
			/* Have a QoS to worst QoS range. */

			for (ui = 0; ui < qosCount; ++ui)
			{
				/* Is this QoS in range? */
				if (!rsslQosIsInRange(&pItemRequest->qos, &pItemRequest->worstQos, &qosList[ui]))
					continue;

				return &qosList[ui];
			}
		}
		else  
		{
			/* No Qos or worst Qos -- any QoS matches. */
			for (ui = 0; ui < qosCount; ++ui)
			{
				return &qosList[ui];
			}
		}
		return NULL;
	}

}

RsslRet wlItemStreamInit(WlItemStream *pItemStream, WlStreamAttributes *pStreamAttributes,
		RsslInt32 streamId, RsslErrorInfo *pErrorInfo)
{
	RsslRet ret;

	wlStreamBaseInit(&pItemStream->base, streamId, pStreamAttributes->domainType);
	pItemStream->msgKeyMemoryBuffer = NULL;

	pItemStream->streamAttributes = *pStreamAttributes;

	/* Copy Msg Key. */
	if ((ret = wlItemCopyKey(&pItemStream->streamAttributes.msgKey, 
			&pStreamAttributes->msgKey, &pItemStream->msgKeyMemoryBuffer, pErrorInfo)) 
			!= RSSL_RET_SUCCESS)
		return RSSL_RET_FAILURE;

	pItemStream->refreshState = WL_ISRS_NONE;
	pItemStream->flags = WL_IOSF_NONE;
	pItemStream->pAggregateView = NULL;
	pItemStream->priorityClass = 1;
	pItemStream->priorityCount = 1;
	pItemStream->requestsStreamingCount = 0;
	rsslInitQueue(&pItemStream->requestsRecovering);
	rsslInitQueue(&pItemStream->requestsPendingRefresh);
	rsslInitQueue(&pItemStream->requestsOpen);
	pItemStream->requestsPausedCount = 0;
	pItemStream->requestsWithViewCount = 0;
	pItemStream->pItemGroup = NULL;
	pItemStream->pFTGroup = NULL;
	pItemStream->nextPartNum = 0;
	pItemStream->pRequestWithExtraInfo = NULL;
	wlMsgReorderQueueInit(&pItemStream->bufferedMsgQueue);
	pItemStream->itemIsClosedForAllStandby = RSSL_FALSE;

	return RSSL_RET_SUCCESS;
}

void wlItemStreamResetState(WlItemStream *pItemStream)
{
	if (pItemStream->refreshState != WL_ISRS_NONE)
	{
		if (pItemStream->refreshState == WL_ISRS_PENDING_OPEN_WINDOW)
			rsslQueueRemoveLink(&pItemStream->pWlService->streamsPendingWindow,
					&pItemStream->qlOpenWindow);
		else
			rsslQueueRemoveLink(&pItemStream->pWlService->streamsPendingRefresh,
					&pItemStream->qlOpenWindow);

		pItemStream->flags = WL_ISRS_NONE;
	}

	/* Reset flags(set view change flag so that view gets re-checked). */
	pItemStream->flags = WL_IOSF_PENDING_VIEW_CHANGE;
	pItemStream->priorityClass = 1;
	pItemStream->priorityCount = 1;
}

WlItemStream *wlCreateItemStream(WlBase *pBase, WlStreamAttributes *pStreamAttributes, RsslErrorInfo *pErrorInfo)
{
	WlItemStream *pItemStream;
	RsslInt32 streamId;

	if (!(pItemStream = (WlItemStream*)rsslMemoryPoolGet(&pBase->streamPool, pErrorInfo)))
		return NULL;

	streamId = wlBaseTakeStreamId(pBase);

	if (wlItemStreamInit(pItemStream, pStreamAttributes, streamId, pErrorInfo) != RSSL_RET_SUCCESS)
	{
		rsslMemoryPoolPut(&pBase->streamPool, pItemStream);
		return NULL;
	}

	return pItemStream;
}

void wlItemStreamDestroy(WlBase *pBase, WlItemStream *pItemStream)
{ 
	if (pItemStream->pAggregateView)
		wlAggregateViewDestroy(pItemStream->pAggregateView);

	if (pItemStream->msgKeyMemoryBuffer)
		free(pItemStream->msgKeyMemoryBuffer);

	wlMsgReorderQueueCleanup(&pItemStream->bufferedMsgQueue);

	rsslMemoryPoolPut(&pBase->streamPool, pItemStream);
}

static void wlItemStreamMergePriorityFromRequest(WlItemRequest *pItemRequest,
		RsslUInt8 *pPriorityClass, RsslUInt16 *pPriorityCount)
{
	if (!(pItemRequest->requestMsgFlags & RSSL_RQMF_STREAMING)) /* Do not use non-streaming priority. */
		return;

	if (pItemRequest->priorityClass == *pPriorityClass)
	{
		/* Class is equal; add priority count. */
		*pPriorityCount += pItemRequest->priorityCount;
	}
	else if (pItemRequest->priorityClass > *pPriorityClass)
	{
		/* Request class is greater; use request priority info. */
		*pPriorityClass = pItemRequest->priorityClass;;
		*pPriorityCount = pItemRequest->priorityCount;;
	}
	/* Request priority class is lower; do nothing. */
}

RsslBool wlItemStreamMergePriority(WlItemStream *pItemStream, RsslUInt8 *pPriorityClass,
		RsslUInt16 *pPriorityCount)
{
	RsslQueueLink *pLink;

	RsslUInt8 origPriorityClass = pItemStream->priorityClass;
	RsslUInt16 origPriorityCount = pItemStream->priorityCount;

	if (pItemStream->requestsStreamingCount == 0)
		return RSSL_FALSE;

	*pPriorityClass = 0;
	*pPriorityCount = 0;

	RSSL_QUEUE_FOR_EACH_LINK(&pItemStream->requestsRecovering, pLink)
	{
		WlItemRequest *pItemRequest = RSSL_QUEUE_LINK_TO_OBJECT(WlItemRequest, base.qlStateQueue, pLink);
		wlItemStreamMergePriorityFromRequest(pItemRequest, pPriorityClass, pPriorityCount);
	}

	RSSL_QUEUE_FOR_EACH_LINK(&pItemStream->requestsPendingRefresh, pLink)
	{
		WlItemRequest *pItemRequest = RSSL_QUEUE_LINK_TO_OBJECT(WlItemRequest, base.qlStateQueue, pLink);
		wlItemStreamMergePriorityFromRequest(pItemRequest, pPriorityClass, pPriorityCount);
	}

	RSSL_QUEUE_FOR_EACH_LINK(&pItemStream->requestsOpen, pLink)
	{
		WlItemRequest *pItemRequest = RSSL_QUEUE_LINK_TO_OBJECT(WlItemRequest, base.qlStateQueue, pLink);
		wlItemStreamMergePriorityFromRequest(pItemRequest, pPriorityClass, pPriorityCount);
	}

	return (*pPriorityClass != origPriorityClass || *pPriorityCount != origPriorityCount);
}

RsslRet wlItemStreamAddRequestView(WlItemStream *pItemStream, WlItemRequest *pItemRequest, 
		RsslErrorInfo *pErrorInfo)
{
	if (!pItemRequest->pView)
		return RSSL_RET_SUCCESS;

	++pItemStream->requestsWithViewCount;
	if (!pItemStream->pAggregateView)
	{
		if (!(pItemStream->pAggregateView = wlAggregateViewCreate(pErrorInfo)))
			return RSSL_RET_FAILURE;
	}

	wlAggregateViewAdd(pItemStream->pAggregateView, pItemRequest->pView);
	pItemStream->flags |= WL_IOSF_PENDING_VIEW_CHANGE;

	return RSSL_RET_SUCCESS;
}

void wlItemStreamRemoveRequestView(WlItemStream *pItemStream,
		WlItemRequest *pItemRequest)
{
	if (!pItemRequest->pView)
	{
		if (pItemStream->pAggregateView)
			pItemStream->flags |= WL_IOSF_PENDING_VIEW_CHANGE;
		return;
	}

	wlAggregateViewRemove(pItemStream->pAggregateView, pItemRequest->pView);
	pItemStream->flags |= WL_IOSF_PENDING_VIEW_CHANGE;

	assert(pItemStream->requestsWithViewCount);
	--pItemStream->requestsWithViewCount;

	/* Don't destroy aggregate view until it is committed and there are no views. 
	 * wlStreamSubmitMsg must do that. */
}

RsslBool wlItemStreamHasRequests(WlItemStream *pItemStream)
{
	return (pItemStream->requestsRecovering.count == 0
			&& pItemStream->requestsPendingRefresh.count == 0
			&& pItemStream->requestsOpen.count == 0) ? RSSL_FALSE : RSSL_TRUE;
}


RsslRet wlItemGroupAddStream(WlItems *pItems, RsslBuffer *pGroupId, WlItemStream *pItemStream,
		RsslErrorInfo *pErrorInfo)
{
	WlService		*pWlService = pItemStream->pWlService;
	RsslHashLink	*pLink;
	RsslUInt32		hashSum;
	WlItemGroup		*pItemGroup;

	assert(pWlService);

	if (pItemStream->pItemGroup)
		if (rsslBufferIsEqual(&pItemStream->pItemGroup->groupId, pGroupId))
			return RSSL_RET_SUCCESS;
		else
			wlItemGroupRemoveStream(pItems, pItemStream->pItemGroup, pItemStream);

	hashSum = rsslHashBufferSum(pGroupId);
	pLink = rsslHashTableFind(&pWlService->itemGroupTable, (void*)pGroupId, &hashSum);

	if (pLink)
		pItemGroup = RSSL_HASH_LINK_TO_OBJECT(WlItemGroup, hlItemGroupTable, pLink);
	else
	{
		/* Need a new item group. */
		pItemGroup = (WlItemGroup*)malloc(sizeof(WlItemGroup));
		verify_malloc(pItemGroup, pErrorInfo, RSSL_RET_FAILURE);

		memset(pItemGroup, 0, sizeof(WlItemGroup));
		if (rsslHeapBufferCreateCopy(&pItemGroup->groupId, pGroupId) != RSSL_RET_SUCCESS)
		{
			rsslSetErrorInfo(pErrorInfo, RSSL_EIC_FAILURE, RSSL_RET_FAILURE, __FILE__, __LINE__, 
					"Memory allocation failed.");
			free(pItemGroup);
			return RSSL_RET_FAILURE;
		}


		pItemGroup->pWlService = pWlService;
		rsslInitQueue(&pItemGroup->openStreamList);

		rsslQueueAddLinkToBack(&pWlService->itemGroups, &pItemGroup->qlItemGroups);
		rsslHashTableInsertLink(&pWlService->itemGroupTable, &pItemGroup->hlItemGroupTable,
				&pItemGroup->groupId, &hashSum);
	}

	pItemStream->pItemGroup = pItemGroup;
	rsslQueueAddLinkToBack(&pItemGroup->openStreamList, &pItemStream->qlItemGroup);

	return RSSL_RET_SUCCESS;
	
}

void wlItemGroupRemove(WlItemGroup *pItemGroup)
{
	rsslHashTableRemoveLink(&pItemGroup->pWlService->itemGroupTable, 
			&pItemGroup->hlItemGroupTable);
	rsslQueueRemoveLink(&pItemGroup->pWlService->itemGroups, &pItemGroup->qlItemGroups);
	rsslHeapBufferCleanup(&pItemGroup->groupId);
	free(pItemGroup);
}

void wlItemGroupMerge(WlItemGroup *pItemGroup, WlService *pWlService, RsslBuffer *pNewGroupId)
{
	RsslHashLink *pHashLink;

	/* Remove from table. */
	rsslHashTableRemoveLink(&pItemGroup->pWlService->itemGroupTable, 
			&pItemGroup->hlItemGroupTable);
	rsslQueueRemoveLink(&pWlService->itemGroups, &pItemGroup->qlItemGroups);

	/* Update name. */
	pHashLink = rsslHashTableFind(&pWlService->itemGroupTable, (void*)pNewGroupId, NULL);

	if (pHashLink)
	{
		/* Group ID exists, move these items to that group. */
		WlItemGroup *pNewGroup = RSSL_HASH_LINK_TO_OBJECT(WlItemGroup, hlItemGroupTable, pHashLink);
		RsslQueueLink *pLink;

		/* Update every WlItemStream in the list of the old group ID to use the new group ID. */
		RSSL_QUEUE_FOR_EACH_LINK(&pItemGroup->openStreamList, pLink)
		{
			WlItemStream *pItemStream = RSSL_QUEUE_LINK_TO_OBJECT(WlItemStream, qlItemGroup, pLink);
			pItemStream->pItemGroup = pNewGroup;
		}

		rsslQueueAppend(&pNewGroup->openStreamList, &pItemGroup->openStreamList);

		/* Cleanup old group. */
		rsslHeapBufferCleanup(&pItemGroup->groupId);
		free(pItemGroup);

	}
	else
	{
		/* No Group currently has this ID, so simply change the name and re-insert into
		 * the table. */
		rsslHeapBufferCopy(&pItemGroup->groupId, pNewGroupId,
				&pItemGroup->groupId);

		rsslHashTableInsertLink(&pWlService->itemGroupTable, &pItemGroup->hlItemGroupTable,
				(void*)&pItemGroup->groupId, NULL);
		rsslQueueAddLinkToBack(&pWlService->itemGroups, &pItemGroup->qlItemGroups);
	}

}


static void wlItemGroupRemoveStream(WlItems *pItems, WlItemGroup *pItemGroup, WlItemStream *pItemStream)
{
	rsslQueueRemoveLink(&pItemStream->pItemGroup->openStreamList, &pItemStream->qlItemGroup);

	if (pItemStream->pItemGroup->openStreamList.count == 0
			&& pItems->pCurrentFanoutGroup != pItemGroup)
		wlItemGroupRemove(pItemGroup);

	pItemStream->pItemGroup = NULL;
}

RsslRet wlFTGroupAddStream(WlBase *pBase, WlItems *pItems, RsslUInt8 ftGroupId, 
		WlItemStream *pItemStream, RsslErrorInfo *pErrorInfo)
{
	WlFTGroup *pGroup;

	if (!(pGroup = pItems->ftGroupTable[ftGroupId]))
	{
		/* Group does not already exist, so create it. */
		pGroup = (WlFTGroup*)malloc(sizeof(WlFTGroup));
		verify_malloc(pGroup, pErrorInfo, RSSL_RET_FAILURE);

		pGroup->ftGroupId = ftGroupId;
		pItems->ftGroupTable[ftGroupId] = pGroup;
		rsslQueueAddLinkToBack(&pItems->ftGroupTimerQueue, &pGroup->qlGroups);
		pGroup->expireTime = pBase->currentTime + pBase->pRsslChannel->pingTimeout * 1000;
		pBase->watchlist.state |= RSSLWL_STF_NEED_TIMER;
		rsslInitQueue(&pGroup->openStreamList);
		
	}

	rsslQueueAddLinkToBack(&pGroup->openStreamList, &pItemStream->qlFTGroup);
	pItemStream->pFTGroup = pGroup;
	
	return RSSL_RET_SUCCESS;
}

void wlFTGroupRemoveStream(WlItems *pItems, WlItemStream *pItemStream)
{
	WlFTGroup *pGroup;

	assert(pItemStream->pFTGroup);
	pGroup = pItemStream->pFTGroup;
	rsslQueueRemoveLink(&pGroup->openStreamList, &pItemStream->qlFTGroup);
	if (pItems->pCurrentFanoutFTGroup != pGroup
			&& !rsslQueueGetElementCount(&pGroup->openStreamList))
		wlFTGroupRemove(pItems, pGroup); /* Group is now empty, so remove it. */
}

void wlFTGroupRemove(WlItems *pItems, WlFTGroup *pGroup)
{
	pItems->ftGroupTable[pGroup->ftGroupId] = NULL;
	rsslQueueRemoveLink(&pItems->ftGroupTimerQueue, &pGroup->qlGroups);
	free(pGroup);
}



RsslRet wlItemRequestReissue(WlBase *pBase, WlItems *pItems, WlItemRequest *pItemRequest,
		WlItemRequestCreateOpts *pOpts, RsslErrorInfo *pErrorInfo)
{
	RsslRequestMsg *pRequestMsg = pOpts->pRequestMsg;
	WlItemStream *pItemStream = (WlItemStream*)pItemRequest->base.pStream;
	RsslUInt16 msgKeyFlags;
	RsslRet ret;

	assert(pRequestMsg);
	assert(pItemRequest->base.streamId == pRequestMsg->msgBase.streamId);

	/* Ensure that the necessary request parameters match. */

	if (pItemRequest->base.domainType != pRequestMsg->msgBase.domainType)
	{
		rsslSetErrorInfo(pErrorInfo, RSSL_EIC_FAILURE, RSSL_RET_INVALID_DATA, __FILE__, __LINE__, 
				"Domain type does not match existing request.");
		return RSSL_RET_INVALID_DATA;
	}

	if (pRequestMsg->flags & RSSL_RQMF_HAS_BATCH)
	{
		rsslSetErrorInfo(pErrorInfo, RSSL_EIC_FAILURE, RSSL_RET_INVALID_DATA, __FILE__, __LINE__, 
				"Request reissue may not contain batch flag.");
		return RSSL_RET_INVALID_DATA;
	}

	/* Ensure that requests are both streaming. */
	if (!(pItemRequest->requestMsgFlags & RSSL_RQMF_STREAMING))
	{
		rsslSetErrorInfo(pErrorInfo, RSSL_EIC_FAILURE, RSSL_RET_INVALID_DATA, __FILE__, __LINE__, 
				"Snapshot requests may not be reissued.");
		return RSSL_RET_INVALID_DATA;
	}
	else if (!(pRequestMsg->flags & (RSSL_RQMF_STREAMING | RSSL_RQMF_PAUSE)))
	{
		rsslSetErrorInfo(pErrorInfo, RSSL_EIC_FAILURE, RSSL_RET_INVALID_DATA, __FILE__, __LINE__, 
				"Request reissue may not remove streaming flag.");
		return RSSL_RET_INVALID_DATA;
	}

	if (!(pItemRequest->flags & WL_IRQF_PRIVATE) && pRequestMsg->flags & RSSL_RQMF_PRIVATE_STREAM)
	{
		rsslSetErrorInfo(pErrorInfo, RSSL_EIC_FAILURE, RSSL_RET_INVALID_DATA, __FILE__, __LINE__, 
				"Request reissue may not add private stream flag.");
		return RSSL_RET_INVALID_DATA;
	}
	/* Match key. */

	/* Name is not required for reissue request, so only match it if it's present. */
	msgKeyFlags = pItemRequest->msgKey.flags;
	if (!(pRequestMsg->msgBase.msgKey.flags & RSSL_MKF_HAS_NAME))
		pItemRequest->msgKey.flags &= ~RSSL_MKF_HAS_NAME;

	if (pItemRequest->pRequestedService->flags & WL_RSVC_HAS_NAME)
		pRequestMsg->msgBase.msgKey.flags &= ~RSSL_MKF_HAS_SERVICE_ID;

	if (rsslCompareMsgKeys(&pItemRequest->msgKey, 
				&pRequestMsg->msgBase.msgKey) != RSSL_RET_SUCCESS)
	{
		rsslSetErrorInfo(pErrorInfo, RSSL_EIC_FAILURE, RSSL_RET_INVALID_DATA, __FILE__, __LINE__, 
				"Message key does not match existing request.");
		return RSSL_RET_INVALID_DATA;
	}

	pItemRequest->msgKey.flags = msgKeyFlags;

	/* Match service name. */
	if (pItemRequest->pRequestedService->flags & WL_RSVC_HAS_NAME
			&& !pOpts->pServiceName
			|| !(pItemRequest->pRequestedService->flags & WL_RSVC_HAS_NAME)
			&& pOpts->pServiceName
			|| pOpts->pServiceName && !rsslBufferIsEqual(pOpts->pServiceName,
				&pItemRequest->pRequestedService->serviceName))
	{
		rsslSetErrorInfo(pErrorInfo, RSSL_EIC_FAILURE, RSSL_RET_INVALID_DATA, __FILE__, __LINE__, 
				"Service name does not match existing request.");
		return RSSL_RET_INVALID_DATA;
	}

	/* Match Qos */
	if ((pRequestMsg->flags & RSSL_RQMF_HAS_QOS) != (pItemRequest->requestMsgFlags & RSSL_RQMF_HAS_QOS)
			|| (pItemRequest->requestMsgFlags & RSSL_RQMF_HAS_QOS)
			&& !rsslQosIsEqual(&pRequestMsg->qos, &pItemRequest->qos))
	{
		rsslSetErrorInfo(pErrorInfo, RSSL_EIC_FAILURE, RSSL_RET_INVALID_DATA, __FILE__, __LINE__, 
				"QoS does not match existing request.");
		return RSSL_RET_INVALID_DATA;
	}

	/* Match Worst Qos */
	if ((pRequestMsg->flags & RSSL_RQMF_HAS_WORST_QOS) != (pItemRequest->requestMsgFlags & RSSL_RQMF_HAS_WORST_QOS)
			|| (pItemRequest->requestMsgFlags & RSSL_RQMF_HAS_WORST_QOS)
			&& !rsslQosIsEqual(&pRequestMsg->worstQos, &pItemRequest->worstQos))
	{
		rsslSetErrorInfo(pErrorInfo, RSSL_EIC_FAILURE, RSSL_RET_INVALID_DATA, __FILE__, __LINE__, 
				"Worst QoS does not match existing request.");
		return RSSL_RET_INVALID_DATA;
	}

	/* Don't allow reissue to mix view type. */
	if (pOpts->viewAction == WL_IVA_SET 
			&& pItemStream && pItemStream->pAggregateView
			&& pItemStream->pAggregateView->viewType != pOpts->viewType)
	{
		rsslSetErrorInfo(pErrorInfo, RSSL_EIC_FAILURE, RSSL_RET_INVALID_DATA, __FILE__, __LINE__, 
				"Requested view type does not match existing stream.");
		return RSSL_RET_INVALID_DATA;
	}
	

	/* Message succesfully matched, we can now reissue. */
	switch(pOpts->viewAction)
	{
		case WL_IVA_SET:
		{
			WlView *pOldView;

			if (pItemRequest->pView)
			{
				/* Remove previous view. */
				if (pItemStream) 
					wlItemStreamRemoveRequestView(pItemStream, pItemRequest);

				pOldView = pItemRequest->pView;
				pItemRequest->pView = NULL;
			}
			else
				pOldView = NULL;

			if (!(pItemRequest->pView = wlViewCreate(pOpts->viewElemList, 
					pOpts->viewElemCount, pOpts->viewType, pErrorInfo)))
			{
				pItemRequest->pView = pOldView;
				return pErrorInfo->rsslError.rsslErrorId;
			}

			if (pItemStream) 
			{
				if ((ret = wlItemStreamAddRequestView(pItemStream, pItemRequest, pErrorInfo)) != RSSL_RET_SUCCESS)
				{
					wlViewDestroy(pItemRequest->pView);
					pItemRequest->pView = pOldView;
					return ret;
				}
			}

			if (pOldView)
				wlViewDestroy(pOldView);

			break;
		}

		case WL_IVA_MAINTAIN_VIEW:
			break;
		case WL_IVA_NONE:
			/* Check payload for view. */
			if (pItemRequest->pView)
			{
				/* Remove current view. */
				if (pItemStream) 
					wlItemStreamRemoveRequestView(pItemStream, pItemRequest);

				wlViewDestroy(pItemRequest->pView);
				pItemRequest->pView = NULL;
			}
			break;
		default:
			rsslSetErrorInfo(pErrorInfo, RSSL_EIC_FAILURE, RSSL_RET_INVALID_DATA, __FILE__, __LINE__, 
					"Unknown view action %u.", pOpts->viewAction);
			return RSSL_RET_INVALID_DATA;
	}

	/* Update pause count. */
	if (pItemStream)
	{
		if (pRequestMsg->flags & RSSL_RQMF_PAUSE
				&& !(pItemRequest->requestMsgFlags & RSSL_RQMF_PAUSE))
			++pItemStream->requestsPausedCount;

		if (!(pRequestMsg->flags & RSSL_RQMF_PAUSE)
				&& pItemRequest->requestMsgFlags & RSSL_RQMF_PAUSE)
			--pItemStream->requestsPausedCount;
	}

	
	/* Update flags. */
	pItemRequest->requestMsgFlags = (pRequestMsg->flags | RSSL_RQMF_STREAMING);

	if (!(pItemRequest->requestMsgFlags & RSSL_RQMF_NO_REFRESH))
		pItemRequest->flags &= ~WL_IRQF_REFRESHED;

	/* Update priority. */
	if (pRequestMsg->flags & RSSL_RQMF_HAS_PRIORITY)
	{
		pItemRequest->priorityClass = pRequestMsg->priorityClass;
		pItemRequest->priorityCount = pRequestMsg->priorityCount;

		if (pItemStream)
			pItemStream->flags |= WL_IOSF_PENDING_PRIORITY_CHANGE;
	}

	if ((ret = _wlItemRequestSaveExtraInfo(pItemRequest, pRequestMsg, pErrorInfo))
			   	< RSSL_RET_SUCCESS)
		return ret;

	/* Process modified request message. */
	if (pItemStream)
	{
		/* If request had its data body or extended header updated, set it on the stream. */
		if (ret > RSSL_RET_SUCCESS)
			pItemStream->pRequestWithExtraInfo = pItemRequest;
		return wlItemStreamAddRequest(pBase, pItems, pItemStream, pItemRequest, pErrorInfo);
	}

	return RSSL_RET_SUCCESS;
}

RsslRet wlItemRequestCreate(WlBase *pBase, WlItems *pItems, WlItemRequestCreateOpts *pOpts, 
		RsslErrorInfo *pErrorInfo)
{
	RsslRequestMsg *pRequestMsg = pOpts->pRequestMsg;
	RsslRet ret;
	WlItemRequest *pItemRequest;

	if (!(pItemRequest = (WlItemRequest*)rsslMemoryPoolGet(&pBase->requestPool, pErrorInfo)))
		return pErrorInfo->rsslError.rsslErrorId;

	if ((ret = wlItemRequestInit(pItemRequest, pBase, pItems, pOpts, pErrorInfo))
					!= RSSL_RET_SUCCESS)
	{
		rsslMemoryPoolPut(&pBase->requestPool, pItemRequest);
		return ret;
	}

	if (pOpts->slDataStreamFlags & RDM_SYMBOL_LIST_DATA_STREAMS)
	{
		/* Symbol list streams are provider-driven, so their refreshes
		 * should not be received as solicited. So set WL_IRQF_REFRESHED. */
		pItemRequest->flags |= WL_IRQF_REFRESHED | WL_IRQF_PROV_DRIVEN;

		if (!(pOpts->slDataStreamFlags & RDM_SYMBOL_LIST_DATA_SNAPSHOTS))
		{
			assert(pItemRequest->requestMsgFlags & RSSL_RQMF_STREAMING);
			rsslHashTableInsertLink(&pItems->providerRequestsByAttrib,
					&pItemRequest->hlProviderRequestsByAttrib, 
					(void*)pItemRequest, NULL);
		}
	}


	return RSSL_RET_SUCCESS;
}


RsslRet wlItemRequestCleanup(WlItemRequest *pItemRequest)
{
	if (pItemRequest->encDataBody.data != NULL)
		free(pItemRequest->encDataBody.data);

	if (pItemRequest->extendedHeader.data != NULL)
		free(pItemRequest->extendedHeader.data);

	if (pItemRequest->msgKeyMemoryBuffer)
		free(pItemRequest->msgKeyMemoryBuffer);

	if (pItemRequest->pView)
		wlViewDestroy(pItemRequest->pView);

	return RSSL_RET_SUCCESS;
}

void wlItemRequestClose(WlBase *pBase, WlItems *pItems, WlItemRequest *pItemRequest)
{
	WlItemStream *pItemStream = (WlItemStream*)pItemRequest->base.pStream;
	RsslQueueLink *pLink;

	if (pItemRequest->flags & WL_IRQF_PROV_DRIVEN
			&& pItemRequest->requestMsgFlags & RSSL_RQMF_STREAMING)
	{
		rsslHashTableRemoveLink(&pItems->providerRequestsByAttrib, 
				&pItemRequest->hlProviderRequestsByAttrib);
	}

	if (pItemStream)
	{

		wlItemStreamRemoveRequest(pItemStream, pItemRequest);
		wlItemStreamSetMsgPending(pBase, pItemStream, RSSL_FALSE);
	}

	while (pLink = rsslQueueRemoveFirstLink(&pItemRequest->base.openPosts))
	{
		WlPostRecord *pPostRecord = RSSL_QUEUE_LINK_TO_OBJECT(WlPostRecord, qlUser, pLink);
		wlPostTableRemoveRecord(&pBase->postTable, pPostRecord);
	}


	wlRemoveRequest(pBase, &pItemRequest->base);

	rsslQueueRemoveLink(&pItemRequest->pRequestedService->itemRequests, 
			&pItemRequest->qlRequestedService);

}

static RsslRet wlDecodeViewData(WlBase *pBase, WlItemRequestCreateOpts *pOpts,
	RsslDecodeIterator* dIter, RsslErrorInfo *pErrorInfo)
{
	RsslArray viewArray;
	RsslBuffer* memBuffer = &pBase->tempDecodeBuffer;

	RsslRet ret = 0;

	/* already decoded :ViewData element entry, restart decoding with the array */
	switch (pOpts->viewType)
	{
		case RDM_VIEW_TYPE_FIELD_ID_LIST:
		{
			RsslRet retValArray = 0;

			if ((retValArray = rsslDecodeArray(dIter, &viewArray)) == RSSL_RET_SUCCESS)
			{
				RsslBuffer arrayBuffer;

				if (viewArray.primitiveType != RSSL_DT_INT)
				{
					rsslSetErrorInfo(pErrorInfo, RSSL_EIC_FAILURE, viewArray.primitiveType, __FILE__, __LINE__,
							"Unexpected primitive type in array -- %d.", viewArray.primitiveType);
					return RSSL_RET_FAILURE;
				}

				while ((retValArray = rsslDecodeArrayEntry(dIter, &arrayBuffer)) != RSSL_RET_END_OF_CONTAINER)
				{
					if (retValArray < RSSL_RET_SUCCESS)
					{
						rsslSetErrorInfo(pErrorInfo, RSSL_EIC_FAILURE, retValArray, __FILE__, __LINE__,
								"Error decoding array entry in Request Msg -- %d.", retValArray);
						return retValArray;
					}
					else
					{
						RsslInt length = sizeof(RsslFieldId) * pOpts->viewElemCount;
						RsslFieldId* fidId = (RsslFieldId*)(memBuffer->data + length);
						RsslInt tmpInt;

						/* check if we have enough space in the buffer */
						if (length + 1 > memBuffer->length)
							if (rsslHeapBufferResize(&pBase->tempDecodeBuffer, 
											pBase->tempDecodeBuffer.length * 2, RSSL_TRUE) 
										!= RSSL_RET_SUCCESS)
							{
								rsslSetErrorInfo(pErrorInfo, RSSL_EIC_FAILURE, RSSL_RET_FAILURE, __FILE__, __LINE__,
										"Memory allocation failed.");
								return RSSL_RET_FAILURE;
							}


						if ((retValArray = rsslDecodeInt(dIter, &tmpInt)) == RSSL_RET_SUCCESS)
						{
							if (tmpInt < SHRT_MIN || tmpInt > SHRT_MAX)
							{
								rsslSetErrorInfo(pErrorInfo, RSSL_EIC_FAILURE, RSSL_RET_INVALID_DATA, __FILE__, __LINE__,
										"Field %lld in view request is outside the valid field ID range.", tmpInt);
								return RSSL_RET_FAILURE;
							}

							*fidId = (RsslFieldId)tmpInt;
							pOpts->viewElemCount++;
						}
						else
						{
							rsslSetErrorInfo(pErrorInfo, RSSL_EIC_FAILURE, retValArray, __FILE__, __LINE__,
									"Invalid BLANK_DATA or incomplete data while decoding :ViewData-- %d.", retValArray);
							return retValArray;
						}
					}
				}
			}
			else
			{
				rsslSetErrorInfo(pErrorInfo, RSSL_EIC_FAILURE, retValArray, __FILE__, __LINE__,
						"Error decoding array in Request Msg -- %d.", retValArray);
				return retValArray;
			}
		}
		break;

		case RDM_VIEW_TYPE_ELEMENT_NAME_LIST:
		{
			RsslRet retValArray = 0;

			if ((retValArray = rsslDecodeArray(dIter, &viewArray)) == RSSL_RET_SUCCESS)
			{
				RsslBuffer arrayBuffer;

				if ( !(viewArray.primitiveType == RSSL_DT_ASCII_STRING || 
							viewArray.primitiveType == RSSL_DT_UTF8_STRING ||
							viewArray.primitiveType == RSSL_DT_RMTES_STRING))
				{
					rsslSetErrorInfo(pErrorInfo, RSSL_EIC_FAILURE, viewArray.primitiveType, __FILE__, __LINE__,
							"Unexpected primitive type in array -- %d.", viewArray.primitiveType);
					return RSSL_RET_FAILURE;
				}

				while ((retValArray = rsslDecodeArrayEntry(dIter, &arrayBuffer)) != RSSL_RET_END_OF_CONTAINER)
				{
					if (retValArray < RSSL_RET_SUCCESS)
					{
						rsslSetErrorInfo(pErrorInfo, RSSL_EIC_FAILURE, retValArray, __FILE__, __LINE__,
								"Error decoding array entry in Request Msg -- %d.", retValArray);
						return retValArray;
					}
					else
					{
						RsslInt64 length = sizeof(RsslBuffer) * pOpts->viewElemCount;
						RsslBuffer* elementName = (RsslBuffer*)(memBuffer->data + length);

						/* check if we have enough space in the buffer */
						if (length + 1 > memBuffer->length)
							if (rsslHeapBufferResize(&pBase->tempDecodeBuffer,
									pBase->tempDecodeBuffer.length * 2, RSSL_TRUE) != RSSL_RET_SUCCESS)
							{
								rsslSetErrorInfo(pErrorInfo, RSSL_EIC_FAILURE, RSSL_RET_FAILURE, __FILE__, __LINE__,
										"Memory allocation failed.");
								return RSSL_RET_FAILURE;
							}


						if ((retValArray = rsslDecodeBuffer(dIter, elementName)) == RSSL_RET_SUCCESS)
						{
							pOpts->viewElemCount++;
						}
						else
						{
							rsslSetErrorInfo(pErrorInfo, RSSL_EIC_FAILURE, retValArray, __FILE__, __LINE__,
									"Invalid BLANK_DATA while decoding :ViewData -- %d.", retValArray);
							return retValArray;
						}
					}
				}
			}
			else
			{
				rsslSetErrorInfo(pErrorInfo, RSSL_EIC_FAILURE, retValArray, __FILE__, __LINE__,
						"Error decoding array in Request Msg -- %d.", retValArray);
				return retValArray;
			}
		}
	}

	pOpts->viewElemList = memBuffer->data;

	return RSSL_RET_SUCCESS;
	
}

RsslRet wlExtractViewFromMsg(WlBase *pBase, WlItemRequestCreateOpts *pOpts,
		RsslErrorInfo *pErrorInfo)
{
	RsslDecodeIterator dIter;
	RsslChannel* pChannel = pBase->pRsslChannel;

	RsslRet ret = 0;
	RsslElementList elementList;
	RsslElementEntry elementEntry;
	RsslBool viewDataFound = RSSL_FALSE;
	RsslBuffer viewDataElement;
	RsslRequestMsg *pRequestMsg = pOpts->pRequestMsg;

	assert(pRequestMsg->flags & RSSL_RQMF_HAS_VIEW);

	pOpts->viewAction = WL_IVA_MAINTAIN_VIEW;
	pOpts->viewType = 0;
	pOpts->viewElemCount = 0;

	rsslClearBuffer(&viewDataElement);

	/* setup decode iterator */
	rsslClearDecodeIterator(&dIter);
	rsslSetDecodeIteratorRWFVersion(&dIter, pOpts->majorVersion, pOpts->minorVersion);
	rsslSetDecodeIteratorBuffer(&dIter, &pRequestMsg->msgBase.encDataBody);

	rsslClearElementList(&elementList);

	/* check the container type and decode element list */ 
	if (pRequestMsg->msgBase.containerType == RSSL_DT_ELEMENT_LIST && 
			(ret = rsslDecodeElementList(&dIter, &elementList, 0)) == RSSL_RET_SUCCESS)
	{
		rsslClearElementEntry(&elementEntry);

		/* find and decode the :ViewType */
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
				/* if found :ViewType decode it */
				if (rsslBufferIsEqual(&elementEntry.name, &RSSL_ENAME_VIEW_TYPE) &&
						elementEntry.dataType == RSSL_DT_UINT)
				{
					rsslDecodeUInt(&dIter, &pOpts->viewType);
				}
				/* check if the element entry contains :ViewData */
				if (rsslBufferIsEqual(&elementEntry.name, &RSSL_ENAME_VIEW_DATA) &&
						elementEntry.dataType == RSSL_DT_ARRAY)
				{
					/* save the :ViewData buffer for latter */
					viewDataElement = elementEntry.encData;
					viewDataFound = RSSL_TRUE;
				}
			}
			rsslClearElementEntry(&elementEntry);
		}
	}
	else
	{
		rsslSetErrorInfo(pErrorInfo, RSSL_EIC_FAILURE, pRequestMsg->msgBase.containerType, __FILE__, __LINE__,
				"Unexpected container type: -- %d, or decoding error -- %d.", pRequestMsg->msgBase.containerType, ret);
		return RSSL_RET_FAILURE;
	}


	if (pOpts->viewType == RDM_VIEW_TYPE_FIELD_ID_LIST ||
			pOpts->viewType == RDM_VIEW_TYPE_ELEMENT_NAME_LIST)
	{
		/* check if we found :ViewData */
		if (viewDataFound)
		{
			/* reset the buffer for the iterator to the element entry with :ViewData */
			rsslSetDecodeIteratorBuffer(&dIter, &viewDataElement);

			pOpts->viewAction = WL_IVA_SET;

			return wlDecodeViewData(pBase, pOpts, &dIter, pErrorInfo);
		}
		else
		{
			rsslSetErrorInfo(pErrorInfo, RSSL_EIC_FAILURE, RSSL_RET_INCOMPLETE_DATA, __FILE__, __LINE__,
					":ViewData element not found -- %d.", RSSL_RET_INCOMPLETE_DATA);
			return RSSL_RET_FAILURE;
		}
	}
	else
	{
		rsslSetErrorInfo(pErrorInfo, RSSL_EIC_FAILURE, RSSL_RET_INVALID_DATA, __FILE__, __LINE__,
				"Invalid :ViewType or :ViewType not found -- %d.", pOpts->viewType);
		return RSSL_RET_FAILURE;
	}
}


RsslRet wlItemStreamAddRequest(WlBase *pBase, WlItems *pItems, WlItemStream *pItemStream,
		WlItemRequest *pItemRequest, RsslErrorInfo *pErrorInfo)
{
	RsslRet ret;

	if (pItemRequest->base.pStream)
	{
		/* This is a reissue. */
		assert((WlItemStream*)pItemRequest->base.pStream == pItemStream);
		assert(pItemRequest->base.pStateQueue);
		assert(pItemRequest->base.domainType != RSSL_DMT_LOGIN && pItemRequest->base.domainType != RSSL_DMT_SOURCE);

		/* Need a new image, remove request from its current queue. */
		if (!(pItemRequest->requestMsgFlags & RSSL_RQMF_NO_REFRESH)
				&& pItemStream->refreshState == WL_ISRS_NONE)
		{
			rsslQueueRemoveLink(pItemRequest->base.pStateQueue, &pItemRequest->base.qlStateQueue);
			pItemRequest->base.pStateQueue = NULL;
		}
	}
	else
	{

		/* Ensure view type matches. */
		if (pItemRequest->pView && pItemStream->pAggregateView 
				&& pItemStream->pAggregateView->viewType != pItemRequest->pView->viewType)
		{
			RsslStatusMsg statusMsg;
			wlInitErrorStatusMsg(&statusMsg, pItemRequest->base.domainType);
			rssl_set_buffer_to_string(statusMsg.state.text, 
					"Requested view type does not match existing stream.");

			wlItemRequestClose(pBase, pItems, pItemRequest);

			if ((ret = wlItemRequestSendMsg(pBase, pItemRequest, (RsslMsg*)&statusMsg, pErrorInfo)) != RSSL_RET_SUCCESS)
				return ret;

			wlRequestedServiceCheckRefCount(pBase, pItemRequest->pRequestedService);
			wlItemRequestDestroy(pBase, pItemRequest);
			return RSSL_RET_SUCCESS;
		}

		pItemRequest->base.pStream = &pItemStream->base;
		if (pItemRequest->requestMsgFlags & RSSL_RQMF_STREAMING)
		{
			++pItemStream->requestsStreamingCount;
			if (pItemRequest->requestMsgFlags & RSSL_RQMF_PAUSE)
				++pItemStream->requestsPausedCount;
		}

		if ((ret = wlItemStreamAddRequestView(pItemStream, pItemRequest, pErrorInfo))
				!= RSSL_RET_SUCCESS)
			return ret;

		pItemStream->flags |= WL_IOSF_PENDING_PRIORITY_CHANGE;
	}

	if (!pItemRequest->base.pStateQueue)
	{
		if (
				/* Partial refresh has been received. */
				pItemStream->refreshState == WL_ISRS_PENDING_REFRESH_COMPLETE

				/* Request is streaming and stream is currently pending a snapshot. */
				|| (pItemRequest->requestMsgFlags & RSSL_RQMF_STREAMING
					&& pItemStream->flags & WL_IOSF_PENDING_SNAPSHOT)

				/* Need to update/remove view? */
				|| pItemStream->flags & WL_IOSF_PENDING_VIEW_REFRESH && (!pItemRequest->pView ||
				!wlAggregateViewContains(pItemStream->pAggregateView, pItemRequest->pView)))
		{
			/* Have to wait for current request to get its refresh . */
			rsslQueueAddLinkToBack(&pItemStream->requestsRecovering, &pItemRequest->base.qlStateQueue);
			pItemRequest->base.pStateQueue = &pItemStream->requestsRecovering;
			return RSSL_RET_SUCCESS;
		}
		else if (rsslQueueGetElementCount(&pItemStream->requestsPendingRefresh))
		{
			/* Join current request waiting on refresh. */
			rsslQueueAddLinkToBack(&pItemStream->requestsPendingRefresh, &pItemRequest->base.qlStateQueue);
			pItemRequest->base.pStateQueue = &pItemStream->requestsPendingRefresh;
			wlItemStreamSetMsgPending(pBase, pItemStream, RSSL_FALSE);
		}
		else
		{
			/* No requests pending. Send request. */
			rsslQueueAddLinkToBack(&pItemStream->requestsRecovering, &pItemRequest->base.qlStateQueue);
			pItemRequest->base.pStateQueue = &pItemStream->requestsRecovering;
			wlItemStreamSetMsgPending(pBase, pItemStream, RSSL_TRUE);
		}
	}
	else
	{
		wlItemStreamSetMsgPending(pBase, pItemStream, RSSL_FALSE);
	}

	return RSSL_RET_SUCCESS;
}

RsslRet wlItemRequestFindStream(WlBase *pBase, WlItems *pItems, WlItemRequest *pItemRequest, 
		RsslErrorInfo *pErrorInfo, RsslBool generateStatus)
{
	RsslUInt capability;
	RsslUInt32 hashSum;
	RsslHashLink *pHashLink;
	const RsslQos *pMatchingQos = NULL;
	WlStreamAttributes streamAttributes;
	WlItemStream *pStream;
	RsslStatusMsg statusMsg;
	RsslWatchlistMsgEvent msgEvent;
	WlService *pWlService = pItemRequest->pRequestedService->pMatchingService;
	RsslRet ret;
	RsslBuffer stateText = { 0, NULL };

	assert(pItemRequest->pRequestedService);

	do /* while(0) */
	{
		if (!pWlService)
		{
			rssl_set_buffer_to_string(stateText, "No matching service present.");
			break;
		}

		/* Check to ensure that the service appears able to service the request
		 * before finding a stream.
		 * (If the cached service does not have actual information from the wire,
		 * it should have the default values set, so there is no need to check
		 * for presence of things like serviceState or acceptingRequests). */

		/* Check that the desired domain is supported. The list is sorted for us. */
		capability = (RsslUInt)pItemRequest->base.domainType;
		if (!pWlService->pService->rdm.info.capabilitiesCount
				|| !bsearch(
					&capability,
					pWlService->pService->rdm.info.capabilitiesList,
					pWlService->pService->rdm.info.capabilitiesCount,
					sizeof(RsslUInt), rscCompareCapabilities))
		{
			rssl_set_buffer_to_string(stateText, "Domain not supported by service.");
			break;
		}

		if (pWlService->pService->rdm.state.serviceState == 0)
		{
			rssl_set_buffer_to_string(stateText, "Service is down.");
			break;
		}

		if (pWlService->pService->rdm.state.acceptingRequests == 0)
		{
			rssl_set_buffer_to_string(stateText, "Service is not accepting requests.");
			break;
		}

		pMatchingQos = wlItemRequestMatchQos(pItemRequest, pWlService->pService->rdm.info.qosList,
				pWlService->pService->rdm.info.qosCount);

		if (!pMatchingQos)
		{
			rssl_set_buffer_to_string(stateText, "Service does not provide a matching QoS.");
			break;
		}


		streamAttributes.domainType = pItemRequest->base.domainType;
		streamAttributes.msgKey = pItemRequest->msgKey;
		streamAttributes.msgKey.flags |= RSSL_MKF_HAS_SERVICE_ID;
		streamAttributes.msgKey.serviceId = (RsslUInt16)pWlService->pService->rdm.serviceId;
		streamAttributes.qos = *pMatchingQos;
		streamAttributes.hasQos = RSSL_TRUE;

		/* If request is for private stream, or no matching stream exists, create one. */

		if (!(pItemRequest->flags & WL_IRQF_PRIVATE))
		{
			hashSum = wlStreamAttributesHashSum(&streamAttributes);
			pHashLink = rsslHashTableFind(&pBase->openStreamsByAttrib,
					(void*)&streamAttributes, &hashSum);
		}
		else
			pHashLink = NULL;

		if (pHashLink)
		{
			pStream = RSSL_HASH_LINK_TO_OBJECT(WlItemStream, hlStreamsByAttrib, pHashLink);
		}
		else 
		{
			pStream = wlCreateItemStream(pBase, &streamAttributes, pErrorInfo);

			if (!(pItemRequest->flags & WL_IRQF_PRIVATE))
			{
				rsslHashTableInsertLink(&pBase->openStreamsByAttrib, 
						&pStream->hlStreamsByAttrib, (void*)&pStream->streamAttributes, &hashSum);
			}
			else
			{
				pStream->flags |= WL_IOSF_PRIVATE;

				if (pItemRequest->flags & WL_IRQF_QUALIFIED)
					pStream->flags |= WL_IOSF_QUALIFIED;

				if (pItemRequest->containerType != RSSL_DT_NO_DATA
						|| pItemRequest->requestMsgFlags & RSSL_RQMF_HAS_EXTENDED_HEADER)
					pStream->pRequestWithExtraInfo = pItemRequest;
			}

			/* If using multicast, register hash ID of this item.
			 * The hash ID is the same as the hash used in the openStreamsByAttrib table.
			 * The transport keeps a reference count, in the event of colliding ID's. */
			/* Private stream responses are received via unicast, so don't register for them. */
			assert(pBase->pRsslChannel);
			if ( pBase->pRsslChannel->connectionType == RSSL_CONN_TYPE_RELIABLE_MCAST &&
					!(pStream->flags & WL_IOSF_PRIVATE) &&
					(ret = rsslIoctl(pBase->pRsslChannel, RSSL_REGISTER_HASH_ID,
									 &pStream->hlStreamsByAttrib.hashSum, 
									 &pErrorInfo->rsslError)) != RSSL_RET_SUCCESS)
			{
				rsslSetErrorInfoLocation(pErrorInfo, __FILE__, __LINE__);
				wlItemStreamDestroy(pBase, pStream);
				return ret;
			}

			pStream->pWlService = pWlService;

			rsslQueueAddLinkToBack(&pWlService->openStreamList, &pStream->qlServiceStreams);
			rsslQueueAddLinkToBack(&pBase->openStreams, &pStream->base.qlStreamsList);
			rsslHashTableInsertLink(&pBase->streamsById, &pStream->base.hlStreamId,
					(void*)&pStream->base.streamId, NULL);

		}

		rsslQueueRemoveLink(&pItemRequest->pRequestedService->recoveringList, 
				&pItemRequest->base.qlStateQueue);
		pItemRequest->base.pStateQueue = NULL;

		if ((ret = wlItemStreamAddRequest(pBase, pItems, pStream, pItemRequest, pErrorInfo))
				!= RSSL_RET_SUCCESS)
			return ret;

		return RSSL_RET_SUCCESS;

	} while(0);

	if (generateStatus)
	{
		assert(stateText.length);
		rsslClearStatusMsg(&statusMsg);
		statusMsg.msgBase.streamId = pItemRequest->base.streamId;
		statusMsg.msgBase.domainType = pItemRequest->base.domainType;
		statusMsg.msgBase.containerType = RSSL_DT_NO_DATA;
		statusMsg.flags = RSSL_STMF_HAS_STATE;
		statusMsg.state.text = stateText;
		statusMsg.state.streamState = RSSL_STREAM_OPEN;
		statusMsg.state.code = RSSL_SC_NONE;

		wlMsgEventClear(&msgEvent);
		msgEvent.pRsslMsg = (RsslMsg*)&statusMsg;

		statusMsg.state.dataState = RSSL_DATA_SUSPECT;
		if (pBase->config.singleOpen && !(pItemRequest->flags & WL_IRQF_PRIVATE))
		{
			statusMsg.state.streamState = RSSL_STREAM_OPEN;
			if (ret = wlItemRequestSendMsgEvent(pBase, &msgEvent, pItemRequest, pErrorInfo)
					!= RSSL_RET_SUCCESS)
				return ret;
		}
		else
		{
			statusMsg.state.streamState = RSSL_STREAM_CLOSED_RECOVER;

			wlItemRequestClose(pBase, pItems, pItemRequest);

			if (ret = wlItemRequestSendMsgEvent(pBase, &msgEvent, pItemRequest, pErrorInfo)
					!= RSSL_RET_SUCCESS)
				return ret;

			wlRequestedServiceCheckRefCount(pBase, pItemRequest->pRequestedService);
			wlItemRequestDestroy(pBase, pItemRequest);
		}
	}
	return RSSL_RET_SUCCESS;
}

static void wlItemRequestAddServiceIdToKey(WlItemRequest *pItemRequest, RsslMsgKey *pMsgKey)
{
	if (pItemRequest->pRequestedService->pMatchingService)
	{
		pMsgKey->flags |= RSSL_MKF_HAS_SERVICE_ID;
		pMsgKey->serviceId = (RsslUInt16)pItemRequest->pRequestedService->pMatchingService->pService->rdm.serviceId;
	}
}

RsslRet wlItemRequestSendMsgEvent(WlBase *pBase,
		RsslWatchlistMsgEvent *pEvent, WlItemRequest *pItemRequest, RsslErrorInfo *pErrorInfo)
{
	RsslRet ret;
	RsslMsg rsslMsg, *pRsslMsg;
	RsslWatchlistStreamInfo streamInfo;

	assert(pEvent->pRsslMsg);
	assert(!pEvent->pStreamInfo);

	pEvent->pRsslMsg->msgBase.streamId = pItemRequest->base.streamId;

	wlStreamInfoClear(&streamInfo);
	pEvent->pStreamInfo = &streamInfo;
	if ((pItemRequest->flags & WL_IRQF_PRIVATE) != 0)
	{
		if (pEvent->pRsslMsg && pEvent->pRsslMsg->msgBase.msgClass == RSSL_MC_STATUS)
		{
			if (pEvent->pRsslMsg->statusMsg.state.streamState == RSSL_STREAM_CLOSED_RECOVER &&
				pEvent->pRsslMsg->statusMsg.state.dataState == RSSL_DATA_SUSPECT)
			{
				pEvent->_flags |= WL_MEF_NOTIFY_STATUS;
			}
		}
	}

	streamInfo.pUserSpec = pItemRequest->base.pUserSpec;

	if (pItemRequest->pRequestedService->flags & WL_RSVC_HAS_NAME)
		streamInfo.pServiceName = &pItemRequest->pRequestedService->serviceName;

	if (!(pItemRequest->requestMsgFlags & RSSL_RQMF_MSG_KEY_IN_UPDATES)
			&& (!(pItemRequest->flags & WL_IRQF_PROV_DRIVEN) || 
				pItemRequest->flags & WL_IRQF_HAS_PROV_KEY))
	{
		if ((ret = _reactorWatchlistMsgCallback(&pBase->watchlist, pEvent, pErrorInfo))
				!= RSSL_RET_SUCCESS)
			return ret;
	}
	else
	{
		pRsslMsg = pEvent->pRsslMsg;
		pItemRequest->flags |= WL_IRQF_HAS_PROV_KEY;

		/* Add message key if application requested it. */
		switch(pEvent->pRsslMsg->msgBase.msgClass)
		{
			case RSSL_MC_REFRESH:
				if (!(pEvent->pRsslMsg->updateMsg.flags & RSSL_RFMF_HAS_MSG_KEY))
				{
					rsslMsg.refreshMsg = pEvent->pRsslMsg->refreshMsg;
					rsslMsg.refreshMsg.flags |= RSSL_RFMF_HAS_MSG_KEY;
					rsslMsg.refreshMsg.msgBase.msgKey = pItemRequest->msgKey;

					if (!(pItemRequest->msgKey.flags & RSSL_MKF_HAS_SERVICE_ID))
						wlItemRequestAddServiceIdToKey(pItemRequest, &rsslMsg.msgBase.msgKey);

					pEvent->pRsslMsg = &rsslMsg;
				}
				break;
			case RSSL_MC_UPDATE:
				if (!(pEvent->pRsslMsg->updateMsg.flags & RSSL_UPMF_HAS_MSG_KEY))
				{
					rsslMsg.updateMsg = pEvent->pRsslMsg->updateMsg;
					rsslMsg.updateMsg.flags |= RSSL_UPMF_HAS_MSG_KEY;
					rsslMsg.updateMsg.msgBase.msgKey = pItemRequest->msgKey;

					if (!(pItemRequest->msgKey.flags & RSSL_MKF_HAS_SERVICE_ID))
						wlItemRequestAddServiceIdToKey(pItemRequest, &rsslMsg.msgBase.msgKey);

					pEvent->pRsslMsg = &rsslMsg;
				}
				break;
			case RSSL_MC_STATUS:
				if (!(pEvent->pRsslMsg->statusMsg.flags & RSSL_STMF_HAS_MSG_KEY))
				{
					rsslMsg.statusMsg = pEvent->pRsslMsg->statusMsg;
					rsslMsg.statusMsg.flags |= RSSL_STMF_HAS_MSG_KEY;
					rsslMsg.statusMsg.msgBase.msgKey = pItemRequest->msgKey;

					if (!(pItemRequest->msgKey.flags & RSSL_MKF_HAS_SERVICE_ID))
						wlItemRequestAddServiceIdToKey(pItemRequest, &rsslMsg.msgBase.msgKey);

					pEvent->pRsslMsg = &rsslMsg;
				}
				break;
			case RSSL_MC_GENERIC:
				if (!(pEvent->pRsslMsg->genericMsg.flags & RSSL_GNMF_HAS_MSG_KEY))
				{
					rsslMsg.genericMsg = pEvent->pRsslMsg->genericMsg;
					rsslMsg.genericMsg.flags |= RSSL_GNMF_HAS_MSG_KEY;
					rsslMsg.genericMsg.msgBase.msgKey = pItemRequest->msgKey;

					if (!(pItemRequest->msgKey.flags & RSSL_MKF_HAS_SERVICE_ID))
						wlItemRequestAddServiceIdToKey(pItemRequest, &rsslMsg.msgBase.msgKey);

					pEvent->pRsslMsg = &rsslMsg;
				}
				break;
			case RSSL_MC_ACK:
				if (!(pEvent->pRsslMsg->ackMsg.flags & RSSL_AKMF_HAS_MSG_KEY))
				{
					rsslMsg.ackMsg = pEvent->pRsslMsg->ackMsg;
					rsslMsg.ackMsg.flags |= RSSL_AKMF_HAS_MSG_KEY;
					rsslMsg.ackMsg.msgBase.msgKey = pItemRequest->msgKey;

					if (!(pItemRequest->msgKey.flags & RSSL_MKF_HAS_SERVICE_ID))
						wlItemRequestAddServiceIdToKey(pItemRequest, &rsslMsg.msgBase.msgKey);

					pEvent->pRsslMsg = &rsslMsg;
				}
				break;
			default:
				break;
		}
		
		if ((ret = _reactorWatchlistMsgCallback(&pBase->watchlist, pEvent, pErrorInfo))
				!= RSSL_RET_SUCCESS)
			return ret;
		pEvent->pRsslMsg = pRsslMsg;
	}

	pEvent->pStreamInfo = NULL;

	return RSSL_RET_SUCCESS;


}

RsslRet wlItemRequestSendMsg(WlBase *pBase,
		WlItemRequest *pItemRequest, RsslMsg *pRsslMsg, RsslErrorInfo *pErrorInfo)
{
	RsslWatchlistMsgEvent msgEvent;

	wlMsgEventClear(&msgEvent);
	msgEvent.pRsslMsg = pRsslMsg;

	return wlItemRequestSendMsgEvent(pBase, &msgEvent, pItemRequest, pErrorInfo);

}

void wlItemStreamRemoveRequest(WlItemStream *pItemStream, WlItemRequest *pItemRequest)
{
	assert(pItemRequest->base.pStream == &pItemStream->base);

	if (pItemStream->pRequestWithExtraInfo == pItemRequest)
		pItemStream->pRequestWithExtraInfo = NULL;

	if (pItemRequest->requestMsgFlags & RSSL_RQMF_STREAMING)
	{
		--pItemStream->requestsStreamingCount;
		if (pItemRequest->requestMsgFlags & RSSL_RQMF_PAUSE)
			--pItemStream->requestsPausedCount;
	}

	wlItemStreamRemoveRequestView(pItemStream, pItemRequest);

	pItemStream->flags |= WL_IOSF_PENDING_PRIORITY_CHANGE;

	assert (pItemRequest->base.pStateQueue);
	rsslQueueRemoveLink(pItemRequest->base.pStateQueue, &pItemRequest->base.qlStateQueue);
	pItemRequest->base.pStateQueue = NULL;
}

RsslRet wlItemStreamClose(WlBase *pBase, WlItems *pItems, WlItemStream *pItemStream,
		RsslErrorInfo *pErrorInfo)
{
	assert(!pItemStream->requestsRecovering.count);
	assert(!pItemStream->requestsPendingRefresh.count);
	assert(!pItemStream->requestsOpen.count);

	/* If using multicast, unregister hash ID of this item. *
	 * The transport keeps a reference count, in the event of colliding ID's. */
	/* Private stream responses are received via unicast, so don't register for them. */
	if ( pBase->pRsslChannel &&
			pBase->pRsslChannel->connectionType == RSSL_CONN_TYPE_RELIABLE_MCAST &&
			!(pItemStream->flags & WL_IOSF_PRIVATE))
		rsslIoctl(pBase->pRsslChannel, RSSL_UNREGISTER_HASH_ID,
				&pItemStream->hlStreamsByAttrib.hashSum, &pErrorInfo->rsslError);

	rsslQueueRemoveLink(&pItemStream->pWlService->openStreamList, &pItemStream->qlServiceStreams);

	if (pItemStream->pItemGroup)
		wlItemGroupRemoveStream(pItems, pItemStream->pItemGroup, pItemStream);

	if (pItemStream->pFTGroup)
		wlFTGroupRemoveStream(pItems, pItemStream);

	if (!(pItemStream->flags & WL_IOSF_PRIVATE))
		rsslHashTableRemoveLink(&pBase->openStreamsByAttrib, &pItemStream->hlStreamsByAttrib);

	if (pItemStream->flags & (WL_IOSF_HAS_BC_SEQ_GAP | WL_IOSF_HAS_PART_GAP | WL_IOSF_BC_BEHIND_UC))
		rsslQueueRemoveLink(&pItems->gapStreamQueue, &pItemStream->qlGap);

	rsslQueueRemoveLink(&pBase->openStreams, &pItemStream->base.qlStreamsList);
	rsslHashTableRemoveLink(&pBase->streamsById, &pItemStream->base.hlStreamId);

	if (pItemStream->refreshState != WL_ISRS_NONE)
	{
		if (pItemStream->refreshState == WL_ISRS_PENDING_OPEN_WINDOW)
			rsslQueueRemoveLink(&pItemStream->pWlService->streamsPendingWindow,
					&pItemStream->qlOpenWindow);
		else
			rsslQueueRemoveLink(&pItemStream->pWlService->streamsPendingRefresh,
					&pItemStream->qlOpenWindow);

		pItemStream->refreshState = WL_ISRS_NONE;
	}

	wlItemStreamCheckOpenWindow(pBase, pItemStream->pWlService);

	wlUnsetStreamPendingResponse(pBase, &pItemStream->base);

	if (!(pItemStream->flags & WL_IOSF_CLOSED))
	{
		pItemStream->base.isClosing = RSSL_TRUE;
		wlSetStreamMsgPending(pBase, &pItemStream->base);
	}
	else
	{ 
		wlItemStreamUnsetMsgPending(pBase, pItemStream);
		wlItemStreamDestroy(pBase, pItemStream);
	}

	return RSSL_RET_SUCCESS;
}

void wlItemStreamSetMsgPending(WlBase *pBase, WlItemStream *pItemStream, RsslBool requestRefresh)
{
	if (requestRefresh)
	{
		if (pItemStream->refreshState == WL_ISRS_NONE)
		{
			/* Check service open window before requesting. */
			WlService *pWlService = pItemStream->pWlService;

			if ( pBase->config.obeyOpenWindow && pWlService->pService->rdm.flags & RDM_SVCF_HAS_LOAD
					&& pWlService->pService->rdm.load.flags & RDM_SVC_LDF_HAS_OPEN_WINDOW
					&& (pWlService->streamsPendingRefresh.count 
						>= pWlService->pService->rdm.load.openWindow))
			{
				pItemStream->refreshState = WL_ISRS_PENDING_OPEN_WINDOW;
				rsslQueueAddLinkToBack(&pWlService->streamsPendingWindow,
						&pItemStream->qlOpenWindow);

			}
			else
			{
				wlSetStreamMsgPending(pBase, &pItemStream->base);
				pItemStream->refreshState = WL_ISRS_REQUEST_REFRESH;
				rsslQueueAddLinkToBack(&pWlService->streamsPendingRefresh,
						&pItemStream->qlOpenWindow);
			}
		}
		else if (pItemStream->refreshState != WL_ISRS_PENDING_OPEN_WINDOW)
			wlSetStreamMsgPending(pBase, &pItemStream->base);
	}
	else if (pItemStream->refreshState != WL_ISRS_PENDING_OPEN_WINDOW)
		wlSetStreamMsgPending(pBase, &pItemStream->base);
}

void wlItemStreamUnsetMsgPending(WlBase *pBase, WlItemStream *pItemStream)
{
	assert(pItemStream->pWlService);

	wlUnsetStreamMsgPending(pBase, &pItemStream->base);
}

void wlItemStreamCheckOpenWindow(WlBase *pBase, WlService *pWlService)
{
	RsslUInt freeWindowSlots;

	if (pWlService->streamsPendingWindow.count)
	{
		if ( pWlService->pService->rdm.flags & RDM_SVCF_HAS_LOAD
				&& pWlService->pService->rdm.load.flags & RDM_SVC_LDF_HAS_OPEN_WINDOW)
		{
			freeWindowSlots = (pWlService->streamsPendingRefresh.count
					>= pWlService->pService->rdm.load.openWindow) ? 
				0 : pWlService->pService->rdm.load.openWindow - 
				pWlService->streamsPendingRefresh.count;

			if (freeWindowSlots > pWlService->streamsPendingWindow.count)
				freeWindowSlots = pWlService->streamsPendingWindow.count;
		}
		else
			freeWindowSlots = pWlService->streamsPendingWindow.count;

		/* Check if a slot freed in the OpenWindow. */
		while (freeWindowSlots-- > 0)
		{
			RsslQueueLink *pLink;
			WlItemStream *pWaitingStream;

			assert(pWlService->streamsPendingWindow.count);

			pLink = rsslQueueRemoveFirstLink(&pWlService->streamsPendingWindow);
			pWaitingStream = RSSL_QUEUE_LINK_TO_OBJECT(WlItemStream, 
					qlOpenWindow, pLink);

			assert(pWaitingStream->refreshState == WL_ISRS_PENDING_OPEN_WINDOW);
			
			pWaitingStream->refreshState = WL_ISRS_NONE;
			wlItemStreamSetMsgPending(pBase, pWaitingStream, RSSL_TRUE);
		}
	}
}

void wlItemStreamProcessRefreshComplete(WlBase *pBase, WlItemStream *pItemStream)
{
	WlService *pWlService = pItemStream->pWlService;

	if (pItemStream->refreshState != WL_ISRS_PENDING_REFRESH
			&& pItemStream->refreshState != WL_ISRS_PENDING_REFRESH_COMPLETE)
		return;

	rsslQueueRemoveLink(&pWlService->streamsPendingRefresh, &pItemStream->qlOpenWindow);
	pItemStream->refreshState = WL_ISRS_NONE;

	wlItemStreamCheckOpenWindow(pBase, pWlService);
}

static RsslRet _wlItemRequestSaveExtraInfo(WlItemRequest *pItemRequest, RsslRequestMsg *pRequestMsg,
		RsslErrorInfo *pErrorInfo)
{
	RsslRet ret = RSSL_RET_SUCCESS;

	if (pItemRequest->flags & WL_IRQF_PRIVATE)
	{
		/* Save dataBody, unless it is view, batch, or symbol list behavior content. */
		if (pItemRequest->encDataBody.data != NULL)
		{
			free(pItemRequest->encDataBody.data);
			rsslClearBuffer(&pItemRequest->encDataBody);
			pItemRequest->containerType = RSSL_DT_NO_DATA;
		}

		/* Save data body if:
		   * This is for a qualified stream, OR
		   * - There's no view, batch, or symbol list behaviors */
		if ( pRequestMsg->msgBase.containerType != RSSL_DT_NO_DATA
				&& (pRequestMsg->flags & RSSL_RQMF_QUALIFIED_STREAM)
				|| (!(pRequestMsg->flags & RSSL_RQMF_HAS_VIEW)
					&& !(pRequestMsg->flags & RSSL_RQMF_HAS_BATCH)
					&& (pItemRequest->base.domainType != RSSL_DMT_SYMBOL_LIST
						|| !((WlSymbolListRequest*)pItemRequest)->hasBehaviors))
				)
		{
			pItemRequest->containerType = pRequestMsg->msgBase.containerType;
			pItemRequest->encDataBody.data = (char*)malloc(pRequestMsg->msgBase.encDataBody.length);
			verify_malloc(pItemRequest->encDataBody.data, pErrorInfo, RSSL_RET_FAILURE);
			pItemRequest->encDataBody.length = pRequestMsg->msgBase.encDataBody.length;
			memcpy(pItemRequest->encDataBody.data, pRequestMsg->msgBase.encDataBody.data,
					pRequestMsg->msgBase.encDataBody.length);

			ret = 1;
		}
		else
			pItemRequest->containerType = RSSL_DT_NO_DATA;

		/* Save extended header. */

		if (pItemRequest->extendedHeader.data != NULL)
		{
			free(pItemRequest->extendedHeader.data);
			rsslClearBuffer(&pItemRequest->extendedHeader);
			pItemRequest->requestMsgFlags &= ~RSSL_RQMF_HAS_EXTENDED_HEADER;
		}

		if (pRequestMsg->flags & RSSL_RQMF_HAS_EXTENDED_HEADER)
		{
			pItemRequest->extendedHeader.data = (char*)malloc(pRequestMsg->extendedHeader.length);
			verify_malloc(pItemRequest->extendedHeader.data, pErrorInfo, RSSL_RET_FAILURE);
			pItemRequest->extendedHeader.length = pRequestMsg->extendedHeader.length;
			memcpy(pItemRequest->extendedHeader.data, pRequestMsg->extendedHeader.data,
					pRequestMsg->extendedHeader.length);

			pItemRequest->requestMsgFlags |= RSSL_RQMF_HAS_EXTENDED_HEADER;
			ret = 1;
		}
	}
	else
	{
		pItemRequest->requestMsgFlags &= ~RSSL_RQMF_HAS_EXTENDED_HEADER;
		pItemRequest->containerType = RSSL_DT_NO_DATA;
	}

	return ret;
}
