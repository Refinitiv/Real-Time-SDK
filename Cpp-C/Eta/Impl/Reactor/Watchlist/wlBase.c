/*
 * This source code is provided under the Apache 2.0 license and is provided
 * AS IS with no warranty or guarantee of fit for purpose.  See the project's 
 * LICENSE.md for details. 
 * Copyright (C) 2019 LSEG. All rights reserved.
*/

#include "rtr/wlBase.h"

RsslUInt32 wlStreamAttributesHashSum(void *pKey)
{
	return rsslMsgKeyHash(&((WlStreamAttributes*)pKey)->msgKey);
}

RsslBool wlStreamAttributesHashCompare(void *pKey1, void *pKey2)
{
	WlStreamAttributes *pStreamAttributes1 = (WlStreamAttributes*)pKey1;
	WlStreamAttributes *pStreamAttributes2 = (WlStreamAttributes*)pKey2;

	if (pStreamAttributes1->domainType != pStreamAttributes2->domainType)
		return RSSL_FALSE;

	/* Do not match QoS unless both are present. This allows us to match multicast
	 * responses that do not provide a QoS. */
	if (pStreamAttributes1->hasQos && pStreamAttributes2->hasQos)
	{
		if (!rsslQosIsEqual(&pStreamAttributes1->qos, &pStreamAttributes2->qos))
			return RSSL_FALSE;
	}

	return (rsslCompareMsgKeys(&pStreamAttributes1->msgKey, &pStreamAttributes2->msgKey)
			== RSSL_RET_SUCCESS ? RSSL_TRUE : RSSL_FALSE);

}


RsslRDMMsg *wlCreateRdmMsgCopy(RsslRDMMsg *pRdmMsg, RsslBuffer *pMemoryBuffer, 
		RsslErrorInfo *pErrorInfo)
{

	pMemoryBuffer->length = 128 + sizeof(RsslRDMMsg);

	for(;;)
	{
		RsslRDMMsg *pNewRdmMsg;
		RsslBuffer memoryBuffer;
		RsslRet ret;

		pMemoryBuffer->data = (char*)malloc(pMemoryBuffer->length);
		verify_malloc(pMemoryBuffer->data, pErrorInfo, NULL);

		memoryBuffer = *pMemoryBuffer;
		pNewRdmMsg = (RsslRDMMsg*)memoryBuffer.data;
		memoryBuffer.data += sizeof(RsslRDMMsg);
		memoryBuffer.length -= sizeof(RsslRDMMsg);
		ret = rsslCopyRDMMsg(pNewRdmMsg, pRdmMsg, &memoryBuffer);

		if (ret == RSSL_RET_SUCCESS) 
			return pNewRdmMsg;
		switch(ret)
		{
			case RSSL_RET_BUFFER_TOO_SMALL:
				free(pMemoryBuffer->data);
				pMemoryBuffer->length *= 2;
				break;
			default:
				free(pMemoryBuffer->data);
				return NULL;

		}
	}
}

RsslRet wlBaseInit(WlBase *pBase, WlBaseInitOptions *pOpts, RsslErrorInfo *pErrorInfo)
{
	WlServiceCacheCreateOptions		serviceCacheOpts;
	WlServiceCache					*pServiceCache;
	RsslRet ret;

	wlServiceCacheClearCreateOptions(&serviceCacheOpts);

	if (!(pServiceCache = wlServiceCacheCreate(&serviceCacheOpts, pErrorInfo)))
		return pErrorInfo->rsslError.rsslErrorId;

	pServiceCache->pUserSpec = (WlBase*)pBase;

	memset(pBase, 0, sizeof(WlBase));

	if (rsslHeapBufferInit(&pBase->tempDecodeBuffer, 16384) != RSSL_RET_SUCCESS)
	{
		rsslSetErrorInfo(pErrorInfo, RSSL_EIC_FAILURE, RSSL_RET_FAILURE, __FILE__, __LINE__, 
				"Memory allocation failure.");
		wlBaseCleanup(pBase);
		return RSSL_RET_FAILURE;
	}

	if (rsslHeapBufferInit(&pBase->tempEncodeBuffer, 16384) != RSSL_RET_SUCCESS)
	{
		rsslSetErrorInfo(pErrorInfo, RSSL_EIC_FAILURE, RSSL_RET_FAILURE, __FILE__, __LINE__, 
				"Memory allocation failure.");
		wlBaseCleanup(pBase);
		return RSSL_RET_FAILURE;
	}

	pBase->channelMaxFragmentSize = 0;
	pBase->pServiceCache = pServiceCache;
	pBase->config.obeyOpenWindow = pOpts->obeyOpenWindow;
	pBase->config.requestTimeout = pOpts->requestTimeout;
	pBase->watchlist.state = 0;
	pBase->channelState = WL_CHS_START;
	pBase->pRsslChannel = NULL;
	pBase->gapRecovery = 1;
	pBase->gapTimeout = 5000;
	pBase->maxBufferedBroadcastMsgs = 100;
	pBase->nextStreamId = MIN_STREAM_ID;
	pBase->nextProviderStreamId = MIN_STREAM_ID;
	pBase->ticksPerMsec = pOpts->ticksPerMsec;
	pBase->enableWarmStandBy = pOpts->enableWarmStandBy;
	pBase->pCurrentWlRequestedService = NULL;

	if ((ret = rsslHashTableInit(&pBase->requestsByStreamId, 10007, rsslHashU32Sum, 
			rsslHashU32Compare, RSSL_TRUE, pErrorInfo)) != RSSL_RET_SUCCESS)
	{
		wlBaseCleanup(pBase);
		return ret;
	}

	if ((ret = rsslHashTableInit(&pBase->openStreamsByAttrib, 100003, wlStreamAttributesHashSum, 
			wlStreamAttributesHashCompare, RSSL_TRUE, pErrorInfo)) != RSSL_RET_SUCCESS)
	{
		wlBaseCleanup(pBase);
		return ret;
	}

	if ((ret = rsslHashTableInit(&pBase->streamsById, 100003, rsslHashU32Sum, 
			rsslHashU32Compare, RSSL_TRUE, pErrorInfo)) != RSSL_RET_SUCCESS)
	{
		wlBaseCleanup(pBase);
		return ret;
	}

	if ((ret = rsslHashTableInit(&pBase->requestedSvcByName, 10007, rsslHashBufferSum, 
			rsslHashBufferCompare, RSSL_TRUE, pErrorInfo)) != RSSL_RET_SUCCESS)
	{
		wlBaseCleanup(pBase);
		return ret;
	}

	if ((ret = rsslHashTableInit(&pBase->requestedSvcById, 10007, rsslHashU64Sum, 
			rsslHashU64Compare, RSSL_TRUE, pErrorInfo)) != RSSL_RET_SUCCESS)
	{
		wlBaseCleanup(pBase);
		return ret;
	}

	rsslInitQueue(&pBase->newRequests);
	rsslInitQueue(&pBase->requestedServices);
	rsslInitQueue(&pBase->streamsPendingRequest);

	rsslInitQueue(&pBase->streamsPendingResponse);
	rsslInitQueue(&pBase->openStreams);

	if ((ret = rsslMemoryPoolInit(&pBase->streamPool, pOpts->streamPoolBlockSize, 
					pOpts->streamPoolCount, pErrorInfo)) != RSSL_RET_SUCCESS)
	{
		wlBaseCleanup(pBase);
		return ret;
	}

	if ((ret = rsslMemoryPoolInit(&pBase->requestPool, pOpts->requestPoolBlockSize, 
					pOpts->requestPoolCount, pErrorInfo)) != RSSL_RET_SUCCESS)
	{
		wlBaseCleanup(pBase);
		return ret;
	}

	if ((ret = wlPostTableInit(&pBase->postTable, pOpts->maxOutstandingPosts, 
					pOpts->postAckTimeout, pErrorInfo))
			!= RSSL_RET_SUCCESS)
	{
		wlBaseCleanup(pBase);
		return ret;
	}

	pBase->maxOutstandingPosts = pOpts->maxOutstandingPosts;
	pBase->postAckTimeout = pOpts->postAckTimeout;
	
	return RSSL_RET_SUCCESS;
}

void wlBaseCleanup(WlBase *pBase)
{
	wlServiceCacheDestroy(pBase->pServiceCache);
	rsslHeapBufferCleanup(&pBase->tempDecodeBuffer);
	rsslHeapBufferCleanup(&pBase->tempEncodeBuffer);
	rsslHashTableCleanup(&pBase->requestsByStreamId);
	rsslHashTableCleanup(&pBase->openStreamsByAttrib);
	rsslHashTableCleanup(&pBase->streamsById);
	rsslHashTableCleanup(&pBase->requestedSvcByName);
	rsslHashTableCleanup(&pBase->requestedSvcById);
	rsslMemoryPoolCleanup(&pBase->requestPool);
	rsslMemoryPoolCleanup(&pBase->streamPool);
	wlPostTableCleanup(&pBase->postTable);
}

void wlAddRequest(WlBase *pBase, WlRequestBase *pRequestBase)
{
	rsslHashLinkInit(&pRequestBase->hlStreamId);
	rsslHashTableInsertLink(&pBase->requestsByStreamId, &pRequestBase->hlStreamId, 
			&pRequestBase->streamId, NULL);
}

void wlRemoveRequest(WlBase *pBase, WlRequestBase *pRequestBase)
{
	RsslQueueLink *pLink;

	while (pLink = rsslQueueRemoveFirstLink(&pRequestBase->openPosts))
	{
		WlPostRecord *pPostRecord = RSSL_QUEUE_LINK_TO_OBJECT(WlPostRecord, qlUser, pLink);
		wlPostTableRemoveRecord(&pBase->postTable, pPostRecord);
	}

	rsslHashTableRemoveLink(&pBase->requestsByStreamId, &pRequestBase->hlStreamId);
	if (pRequestBase->pStateQueue)
		rsslQueueRemoveLink(pRequestBase->pStateQueue, &pRequestBase->qlStateQueue);
}

void wlSetStreamMsgPending(WlBase *pBase, WlStreamBase *pStreamBase)
{
	if (!(pStreamBase->requestState & WL_STRS_PENDING_REQUEST))
	{
		rsslQueueAddLinkToBack(&pBase->streamsPendingRequest, 
				&pStreamBase->qlStreamsPendingRequest);
		pStreamBase->requestState |= WL_STRS_PENDING_REQUEST;
	}
}

void wlSetStreamPendingResponse(WlBase *pBase, WlStreamBase *pStreamBase)
{
	if (!(pStreamBase->requestState & WL_STRS_PENDING_RESPONSE))
	{
		rsslQueueAddLinkToBack(&pBase->streamsPendingResponse,
				&pStreamBase->qlStreamsPendingResponse);
		pStreamBase->requestState |= WL_STRS_PENDING_RESPONSE;

		pStreamBase->requestExpireTime = pBase->currentTime + pBase->config.requestTimeout;
		pBase->watchlist.state |= RSSLWL_STF_NEED_TIMER;
	}
}

void wlResetStreamPendingResponse(WlBase *pBase, WlStreamBase *pStreamBase)
{
	if (pStreamBase->requestState & WL_STRS_PENDING_RESPONSE)
	{
		wlUnsetStreamPendingResponse(pBase, pStreamBase);
		wlSetStreamPendingResponse(pBase, pStreamBase);
	}
}

void wlUnsetStreamPendingResponse(WlBase *pBase, WlStreamBase *pStreamBase)
{
	if (pStreamBase->requestState & WL_STRS_PENDING_RESPONSE)
	{
		rsslQueueRemoveLink(&pBase->streamsPendingResponse,
				&pStreamBase->qlStreamsPendingResponse);
		pStreamBase->requestState &= ~WL_STRS_PENDING_RESPONSE;
	}
}

void wlUnsetStreamMsgPending(WlBase *pBase, WlStreamBase *pStreamBase)
{
	if (pStreamBase->requestState & WL_STRS_PENDING_REQUEST)
	{
		rsslQueueRemoveLink(&pBase->streamsPendingRequest, 
				&pStreamBase->qlStreamsPendingRequest);
		pStreamBase->requestState &= ~WL_STRS_PENDING_REQUEST;
	}
}

void wlUnsetStreamFromPendingLists(WlBase *pBase, WlStreamBase *pStreamBase)
{
	wlUnsetStreamMsgPending(pBase, pStreamBase);
	wlUnsetStreamPendingResponse(pBase, pStreamBase);
	assert(pStreamBase->requestState == WL_STRS_NONE);
}

RsslInt32 wlBaseTakeStreamId(WlBase *pBase)
{
	do
	{
		if (++pBase->nextStreamId == MAX_STREAM_ID) 
			pBase->nextStreamId = MIN_STREAM_ID;
	} while (rsslHashTableFind(&pBase->streamsById, &pBase->nextStreamId, NULL));
	return pBase->nextStreamId;
}

RsslInt32 wlBaseTakeProviderStreamId(WlBase *pBase)
{
	RsslInt32 streamId;
	do
	{
		if (++pBase->nextProviderStreamId == MAX_STREAM_ID) 
			pBase->nextProviderStreamId = MIN_STREAM_ID;
		streamId = -pBase->nextProviderStreamId;
	} while (rsslHashTableFind(&pBase->streamsById, &streamId, NULL));
	return streamId;
}
