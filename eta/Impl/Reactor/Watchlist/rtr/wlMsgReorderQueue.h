/*
 * This source code is provided under the Apache 2.0 license and is provided
 * AS IS with no warranty or guarantee of fit for purpose.  See the project's 
 * LICENSE.md for details. 
 * Copyright Thomson Reuters 2015. All rights reserved.
*/

#ifndef WL_MSG_REORDER_QUEUE_H
#define WL_MSG_REORDER_QUEUE_H

#ifdef __cplusplus
extern "C" {
#endif

/* A queue for reordering unicast and broadcast streams according to sequence numbers. */

#include "wlBase.h"
#include "rtr/rsslQueue.h"
#include "rtr/rsslMsg.h"
#include "rtr/rsslErrorInfo.h"
#include "rtr/rsslReactorUtils.h"

typedef enum
{
	WL_BFMSG_NONE				= 0x00,	/* None. */
	WL_BFMSG_HAS_FT_GROUP_ID	= 0x01	/* FTGroupID is present. */
} WlBufferedMsgFlags;

/* Queue for buffering messages.
 * Used when buffering messages for synching multicast/point-to-point. */
typedef struct
{
	RsslQueueLink	qlMsg;		/* Link for message queue. */
	RsslUInt8		flags;		/* Flags for each message. */
	RsslUInt8		ftGroupId;	/* FTGroupID associated with the message (under normal circumstances 
								 * this shouldn't change within a stream). */
	RsslUInt32		seqNum;		/* Sequence number that was received with this message. */
} WlBufferedMsg;

typedef struct
{
	RsslQueue msgQueue;			/* Queue of buffered broadcast messages. */
	RsslBool  hasUnicastMsgs;	/* Whether the messages stored in the queue are unicast or broadcast. */
} WlMsgReorderQueue;

/* Initializes a WlMsgReorderQueue. */
void wlMsgReorderQueueInit(WlMsgReorderQueue *pQueue);

/* Cleans up a WlMsgReorderQueue. */
void wlMsgReorderQueueCleanup(WlMsgReorderQueue *pQueue);

/* Adds a message to the queue. */
RsslRet wlMsgReorderQueuePush(WlMsgReorderQueue *pQueue, RsslMsg *pRsslMsg,
		RsslUInt32 seqNum, RsslUInt8 *pFTGroupId, WlBase *pBase, RsslErrorInfo *pErrorInfo);

/* Pops a message from the queue.  Typically should occur when the refresh has been
 * completed for a stream. */
WlBufferedMsg *wlMsgReorderQueuePop(WlMsgReorderQueue *pQueue);

/* Pops a message from the queue, as long as it is "before" or equal to the specified sequence 
 * number. */
WlBufferedMsg *wlMsgReorderQueuePopUntil(WlMsgReorderQueue *pQueue, RsslUInt32 seqNum);

/* Discard all messages up to and including the given sequence number. */
RTR_C_INLINE void wlMsgReorderQueueDiscardUntil(WlMsgReorderQueue *pQueue, RsslUInt32 seqNum);

/* Checks for gaps in broadcast queue. pSeqNum should be set to the currently needed sequence number.
 * Returns nonzero value if pSeqNum and pHasGap have been set. */
RsslUInt32 wlMsgReorderQueueCheckBroadcastSequence(WlMsgReorderQueue *pQueue, RsslUInt32 *pSeqNum,
		RsslBool *pHasGap);

/* Retrieves the sequence number of the last message in the broadcast queue. */
RsslBool wlMsgReorderQueueGetLastBcSeqNum(WlMsgReorderQueue *pQueue, RsslUInt32 *pSeqNum);

/* Indicates whether the messages queued are unicast messages. */
RTR_C_INLINE RsslBool wlMsgReorderQueueHasUnicastMsgs(WlMsgReorderQueue *pQueue);

/* Retrieves the location of the RsslMsg stored in the WlBufferedMsg. */
RTR_C_INLINE RsslMsg *wlBufferedMsgGetRsslMsg(WlBufferedMsg *pBufferedMsg);

/* Cleans up a message that was popped from the queue. */
void wlBufferedMsgDestroy(WlBufferedMsg *pBufferedMsg);

/* Deletes all messages from the queue. */
void wlMsgReorderQueueDiscardAllMessages(WlMsgReorderQueue *pQueue);

/* Gets the RsslMsg stored in the WlBufferedMsg. */
RTR_C_INLINE RsslMsg *wlBufferedMsgGetRsslMsg(WlBufferedMsg *pBufferedMsg)
{
	return (RsslMsg*)((char*)pBufferedMsg + sizeof(WlBufferedMsg));
}

RTR_C_INLINE void wlMsgReorderQueueDiscardUntil(WlMsgReorderQueue *pQueue, RsslUInt32 seqNum)
{
	WlBufferedMsg *pMsg;
	while (pMsg = wlMsgReorderQueuePopUntil(pQueue, seqNum))
		wlBufferedMsgDestroy(pMsg);
}

RTR_C_INLINE RsslBool wlMsgReorderQueueHasUnicastMsgs(WlMsgReorderQueue *pQueue)
{
	return pQueue->hasUnicastMsgs;
}

RTR_C_INLINE RsslUInt32 wlGetNextSeqNum(RsslUInt32 seqNum)
{
	switch(seqNum)
	{
		case 0:
		case (RsslUInt32)-1:
			return 1;
		default:
			return seqNum+1;
	}
}

RTR_C_INLINE RsslUInt32 wlGetPrevSeqNum(RsslUInt32 seqNum)
{
	assert(seqNum != 0);

	switch(seqNum)
	{
		case 1:
			return (RsslUInt32)-1;
		default:
			return seqNum-1;
	}
}


#ifdef __cplusplus
}
#endif

#endif
