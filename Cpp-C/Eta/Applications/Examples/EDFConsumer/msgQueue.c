/*
 * This source code is provided under the Apache 2.0 license and is provided
 * AS IS with no warranty or guarantee of fit for purpose.  See the project's 
 * LICENSE.md for details. 
 * Copyright Thomson Reuters 2015. All rights reserved.
*/

#include "msgQueue.h"
#include <stdlib.h>

/* Represents a message buffered in the queue. */
struct BufferedMsg
{
	BufferedMsg	*pNext;		/* Pointer to the next message in the queue, if any. */
	RsslMsg		*pRsslMsg;	/* The copied RsslMsg. */
};

RsslRet msgQueueAdd(MsgQueue *pQueue, RsslMsg *pRsslMsg)
{
	BufferedMsg *pBufferedMsg;

	/* Self-check -- should not be buffering messages that do not have a sequence number. */
	if (!rsslGetSeqNum(pRsslMsg))
	{
		printf("Application Error: Buffering message that does not contain a sequence number.\n");
		return RSSL_RET_FAILURE;
	}

	pBufferedMsg = (BufferedMsg*)malloc(sizeof(BufferedMsg));

	if (pBufferedMsg == NULL)
	{
		printf("malloc() failed while buffering message.\n");
		return RSSL_RET_FAILURE;
	}

	pBufferedMsg->pNext = NULL;

	/* Use rsslCopyMsg() to allocate a copy of the RsslMsg.
	 * Flags are set so that it copies everything, except an uneeded copy of the full encoded 
	 * message. */
	pBufferedMsg->pRsslMsg = rsslCopyMsg(pRsslMsg, RSSL_CMF_ALL_FLAGS & ~RSSL_CMF_MSG_BUFFER, 0, 
			NULL);

	if (pBufferedMsg->pRsslMsg == NULL)
	{
		printf("rsslCopyMsg() failed while buffering message.\n");
		free(pBufferedMsg);
		return RSSL_RET_FAILURE;
	}

	/* Add message to the back of the queue. */
	if (pQueue->pMsgListTail)
	{
		pQueue->pMsgListTail->pNext = pBufferedMsg;
		pQueue->pMsgListTail = pBufferedMsg;
	}
	else
		pQueue->pMsgListHead = pQueue->pMsgListTail = pBufferedMsg;

	return RSSL_RET_SUCCESS;

}

RsslMsg *msgQueuePop(MsgQueue *pQueue, MsgQueuePopCmd popCmd, RsslUInt32 seqNum)
{
	RsslMsg *pRsslMsg;
	BufferedMsg *pBufferedMsg;
	RsslUInt32 msgSeqNum;

	pBufferedMsg = pQueue->pMsgListHead;

	if (pBufferedMsg)
	{
		/* Compare against the sequence number of the message to determine if
		 * it is in order. */
		switch(popCmd)
		{
			case POP_BEFORE:
				msgSeqNum = *rsslGetSeqNum(pBufferedMsg->pRsslMsg);
				if (seqNumCompare(msgSeqNum, seqNum) >= 0)
					return NULL;
				break;

			case POP_UP_TO:
				msgSeqNum = *rsslGetSeqNum(pBufferedMsg->pRsslMsg);
				if (seqNumCompare(msgSeqNum, seqNum) > 0)
					return NULL;
				break;

			case POP_ALL:
			default:
				break;
		}

		pRsslMsg = pBufferedMsg->pRsslMsg;
		pQueue->pMsgListHead = pQueue->pMsgListHead->pNext;
		if (pQueue->pMsgListHead == NULL)
			pQueue->pMsgListTail = NULL;

		free(pBufferedMsg);

		return pRsslMsg;
	}

	return NULL;
}

void msgQueueCleanup(MsgQueue *pQueue)
{
	BufferedMsg *pBufferedMsg;
	while ((pBufferedMsg = pQueue->pMsgListHead))
	{
		pQueue->pMsgListHead = pBufferedMsg->pNext;
		msgQueueCleanupRsslMsg(pBufferedMsg->pRsslMsg);
		free(pBufferedMsg);
	}
	pQueue->pMsgListTail = NULL;
}

void msgQueueCleanupRsslMsg(RsslMsg *pRsslMsg)
{
	rsslReleaseCopiedMsg(pRsslMsg);
}
