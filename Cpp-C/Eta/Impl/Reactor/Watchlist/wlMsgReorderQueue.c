/*
 * This source code is provided under the Apache 2.0 license and is provided
 * AS IS with no warranty or guarantee of fit for purpose.  See the project's 
 * LICENSE.md for details. 
 * Copyright (C) 2024 LSEG. All rights reserved.
*/

#include "rtr/wlMsgReorderQueue.h"
#include "rtr/rsslVAUtils.h"
#include <stdlib.h>
#include <assert.h>

void wlMsgReorderQueueInit(WlMsgReorderQueue *pQueue)
{
	rsslInitQueue(&pQueue->msgQueue);
	pQueue->hasUnicastMsgs = RSSL_FALSE;
}

RsslRet wlMsgReorderQueuePush(WlMsgReorderQueue *pQueue, RsslMsg *pRsslMsg,
		RsslUInt32 seqNum, RsslUInt8 *pFTGroupId, WlBase *pBase, RsslErrorInfo *pErrorInfo)
{
	WlBufferedMsg *pBufferedMsg;
	RsslUInt32 msgSize;
	RsslBuffer msgBuffer;

	/* Allocate space for header and RsslMsg. */
	msgSize = rsslSizeOfMsg(pRsslMsg, RSSL_CMF_ALL_FLAGS & ~RSSL_CMF_MSG_BUFFER);
	pBufferedMsg = (WlBufferedMsg*)malloc(sizeof(WlBufferedMsg) + msgSize);
	verify_malloc(pBufferedMsg, pErrorInfo, RSSL_RET_FAILURE);

	msgBuffer.data = (char*)pBufferedMsg + sizeof(WlBufferedMsg);
	msgBuffer.length = msgSize;
	if (!rsslCopyMsg(pRsslMsg, RSSL_CMF_ALL_FLAGS & ~RSSL_CMF_MSG_BUFFER, 0, &msgBuffer))
	{
		rsslSetErrorInfo(pErrorInfo, RSSL_EIC_FAILURE, RSSL_RET_FAILURE, __FILE__, __LINE__, 
				"Failed to copy message for buffering.");
		return RSSL_RET_FAILURE;
	}

	pBufferedMsg->flags = WL_BFMSG_NONE;
	pBufferedMsg->seqNum = seqNum;

	if (pFTGroupId)
	{
		pBufferedMsg->ftGroupId = *pFTGroupId;
		pBufferedMsg->flags |= WL_BFMSG_HAS_FT_GROUP_ID;
	}

	/* Broadcast & unicast messages should not appear simultaneously in the queue. */
	assert(pRsslMsg->msgBase.streamId != 0 || pQueue->hasUnicastMsgs == RSSL_FALSE);
	assert(pRsslMsg->msgBase.streamId == 0 || pQueue->hasUnicastMsgs == RSSL_TRUE
			|| rsslQueueGetElementCount(&pQueue->msgQueue) == 0);

	if (pRsslMsg->msgBase.streamId != 0)
		pQueue->hasUnicastMsgs = RSSL_TRUE;

	/* Eject an old message if the queue is full. */
	if (rsslQueueGetElementCount(&pQueue->msgQueue) >= pBase->maxBufferedBroadcastMsgs)
	{
		WlBufferedMsg *pOldMsg = wlMsgReorderQueuePop(pQueue);
		wlBufferedMsgDestroy(pOldMsg);
	}

	rsslQueueAddLinkToBack(&pQueue->msgQueue, &pBufferedMsg->qlMsg);

	return RSSL_RET_SUCCESS;
}

WlBufferedMsg *wlMsgReorderQueuePop(WlMsgReorderQueue *pQueue)
{
	RsslQueueLink *pLink = rsslQueuePeekFront(&pQueue->msgQueue);

	if (pLink)
	{
		WlBufferedMsg *pBufferedMsg = RSSL_QUEUE_LINK_TO_OBJECT(WlBufferedMsg, 
				qlMsg, pLink);

		rsslQueueRemoveLink(&pQueue->msgQueue, &pBufferedMsg->qlMsg); 
		if (rsslQueueGetElementCount(&pQueue->msgQueue) == 0)
			pQueue->hasUnicastMsgs = RSSL_FALSE;
		return pBufferedMsg;
	}
	else
		return NULL;
}

WlBufferedMsg *wlMsgReorderQueuePopUntil(WlMsgReorderQueue *pQueue, RsslUInt32 seqNum)
{
	RsslQueueLink *pLink = rsslQueuePeekFront(&pQueue->msgQueue);

	if (pLink)
	{
		WlBufferedMsg *pBufferedMsg = RSSL_QUEUE_LINK_TO_OBJECT(WlBufferedMsg, 
				qlMsg, pLink);

		if (pBufferedMsg && rsslSeqNumCompare(pBufferedMsg->seqNum, seqNum) <= 0)
		{
			/* Return only messages that are considered "before" the requested sequence number. */
			rsslQueueRemoveLink(&pQueue->msgQueue, &pBufferedMsg->qlMsg);
			if (rsslQueueGetElementCount(&pQueue->msgQueue) == 0)
				pQueue->hasUnicastMsgs = RSSL_FALSE;
			return pBufferedMsg;
		}
	}

	return NULL;
		
}

void wlBufferedMsgDestroy(WlBufferedMsg *pBufferedMsg)
{
	free(pBufferedMsg);
}

void wlMsgReorderQueueCleanup(WlMsgReorderQueue *pQueue)
{
	wlMsgReorderQueueDiscardAllMessages(pQueue);
}

void wlMsgReorderQueueDiscardAllMessages(WlMsgReorderQueue *pQueue)
{
	RsslQueueLink *pLink;
	WlBufferedMsg *pBufferedMsg;

	while (pLink = rsslQueueRemoveFirstLink(&pQueue->msgQueue))
	{
		pBufferedMsg = RSSL_QUEUE_LINK_TO_OBJECT(WlBufferedMsg, 
				qlMsg, pLink);
		wlBufferedMsgDestroy(pBufferedMsg);
	}
}

RsslUInt32 wlMsgReorderQueueCheckBroadcastSequence(WlMsgReorderQueue *pQueue, RsslUInt32 *pSeqNum,
		RsslBool *pHasGap)
{
	RsslUInt32 ret = rsslQueueGetElementCount(&pQueue->msgQueue);
	RsslQueueLink *pLink;

	*pHasGap = RSSL_FALSE;

	assert(!pQueue->hasUnicastMsgs);

	RSSL_QUEUE_FOR_EACH_LINK(&pQueue->msgQueue, pLink)
	{
		WlBufferedMsg *pBufferedMsg = RSSL_QUEUE_LINK_TO_OBJECT(WlBufferedMsg, qlMsg, pLink);

		if (pBufferedMsg->seqNum != wlGetNextSeqNum(*pSeqNum))
		{
			rsslQueueRemoveLink(&pQueue->msgQueue, pLink);
			wlBufferedMsgDestroy(pBufferedMsg);
			*pHasGap = RSSL_TRUE;
		}
		else
		{
			*pSeqNum = pBufferedMsg->seqNum;
			*pHasGap = RSSL_FALSE;
		}
	}

	return ret;
}

RsslBool wlMsgReorderQueueGetLastBcSeqNum(WlMsgReorderQueue *pQueue, RsslUInt32 *pSeqNum)
{
	RsslQueueLink *pLink = rsslQueuePeekBack(&pQueue->msgQueue);

	assert(!pQueue->hasUnicastMsgs);

	if (pLink)
	{
		WlBufferedMsg *pBufferedMsg = RSSL_QUEUE_LINK_TO_OBJECT(WlBufferedMsg, qlMsg, pLink);
		*pSeqNum = pBufferedMsg->seqNum;
		return RSSL_TRUE;
	}

	return RSSL_FALSE;
}

