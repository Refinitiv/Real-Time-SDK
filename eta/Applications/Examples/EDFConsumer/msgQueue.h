/*
 * This source code is provided under the Apache 2.0 license and is provided
 * AS IS with no warranty or guarantee of fit for purpose.  See the project's 
 * LICENSE.md for details. 
 * Copyright Thomson Reuters 2015. All rights reserved.
*/

#ifndef MSG_REORDER_QUEUE_H
#define MSG_REORDER_QUEUE_H

#include "rtr/rsslTypes.h"
#include "rtr/rsslMsg.h"

typedef struct BufferedMsg BufferedMsg;

/* A queue of buffered RsslMsgs. */
typedef struct
{
	BufferedMsg	*pMsgListHead;	/* First message in the queue. */
	BufferedMsg	*pMsgListTail;	/* Last message in the queue. */
} MsgQueue;

static void msgQueueClear(MsgQueue *pQueue)
{
	pQueue->pMsgListHead = NULL;
	pQueue->pMsgListTail = NULL;
}

/* Adds a message to the back of the queue. */
RsslRet msgQueueAdd(MsgQueue *pQueue, RsslMsg *pRsslMsg);

/* Used with msgQueuePop() */
typedef enum
{
	POP_BEFORE,	/* Dequeue messages up to (but not including) the given sequence number. */
	POP_UP_TO,	/* Dequeue messages up to (and including) the given sequence number. */
	POP_ALL		/* Dequeue all messages in the queue (ignore the given sequence number). */
} MsgQueuePopCmd;

/* Removes a message from the queue, based upon the given command and sequence number. */
RsslMsg *msgQueuePop(MsgQueue *pQueue, MsgQueuePopCmd popCmd, RsslUInt32 seqNum);

/* Cleans up the message queue. */
void msgQueueCleanup(MsgQueue *pQueue);

/* Cleans up an RsslMsg that was removed from the queue via msgQueuePop. */
void msgQueueCleanupRsslMsg(RsslMsg *pRsslMsg);

/* Returns whether there are messages present in the queue. */
RTR_C_INLINE RsslBool msgQueueHasMsgs(MsgQueue *pQueue)
{
	return pQueue->pMsgListHead ? RSSL_TRUE : RSSL_FALSE;
}

/* Compares two sequence numbers.
 * Returns a negative value if the first sequence number is considered to be "before" the second.
 * Returns 0 if they are equal.
 * Returns a positive if the first sequence number is considered to be "after" the second. */
RTR_C_INLINE RsslInt32 seqNumCompare(RsslUInt32 seqNum1, RsslUInt32 seqNum2)
{
	return (RsslInt32)(seqNum1 - seqNum2);
}

/* Gets the next expected sequence number. */
RTR_C_INLINE RsslUInt32 getNextSeqNum(RsslUInt32 seqNum)
{
	switch(seqNum)
	{
		case (RsslUInt32)-1: /* When the maximum UInt32 is reached, 1 is next. */
			return 1;

		case 0: /* When a sequence is reset, 1 is next. */
		default:
			return seqNum + 1;
	}
}



#endif
