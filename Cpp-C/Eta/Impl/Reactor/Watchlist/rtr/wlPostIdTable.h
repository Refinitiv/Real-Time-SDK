/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2015-2016,2019,2021,2024 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

#ifndef WL_POST_ID_TABLE_H
#define WL_POST_ID_TABLE_H

#include "rtr/rsslReactorUtils.h"

#ifdef __cplusplus
extern "C" {
#endif

struct WlBase;

/* Maintains records of outstanding posts, in order to route it to the appropriate request
 * stream.  Also prevents the sending of posts with duplicate information. */

/* A post is duplicate if either:
   * - There exists a completed post with the same Post ID on the same stream.
   * - There exists an incomplete post message with the same Post ID and sequence number of
   *     the same stream. */

typedef struct
{
	RsslQueueLink	qlUser;			/* Link for either the pool or the stream's list of
									 * open records. */
	RsslQueueLink	qlTimeout;
	void			*pUserSpec;		/* General pointer (intended to point to a WlItemRequest) */

	RsslHashLink	hlTable;
	RsslInt32 		streamId;		/* Provider-facing stream ID of the post. */
	RsslUInt16		flags;			/* RsslPostFlags, mainly whether it is complete and whether it
									 * has a seqNum. */
	RsslUInt32		postId;			/* Post ID. */
	RsslUInt32		seqNum;			/* Sequence number. */
	RsslBool		fromAckMsg;		/* Indicates whether the record is being used to match
									 * an AckMsg, rather than checking PostMsg duplicate. */
	RsslInt64		expireTime;		/* Time at which this post is assumed to have been lost.  */
	RsslUInt8		domainType;		/* Domain type of the post message. */
	RsslUInt16		msgkeyflags;	/* Flag values use indicate optional member presence. The available options are defined by values present in \ref RsslMsgKeyFlags. */
	RsslBuffer		name;			/* Name associated with the contents of the item stream. */
	RsslUInt16		serviceId;		/* The serviceId is a two-byte unsigned integer used to identify a specific service. */
} WlPostRecord;

typedef struct
{
	RsslQueue 		pool;				/* Pool of WlPostRecord structures. */
	RsslHashTable	records;			/* Table of active post records. */
	RsslUInt32		postAckTimeout;		/* Timeout for acknowledgement of posts. */
	RsslQueue		timeoutQueue;		/* Queue of outstanding posts, by their expire time. */
} WlPostTable;

/* Initializes a Post ID table. */
RsslRet wlPostTableInit(WlPostTable *pTable, RsslUInt32 maxPoolSize,
		RsslUInt32 postAckTimeout, RsslErrorInfo *pErrorInfo);

/* Cleans up a Post ID table. */
void wlPostTableCleanup(WlPostTable *pTable);

/* Adds a record to the Post ID table. Fails if duplicate information is present. */
WlPostRecord *wlPostTableAddRecord(struct WlBase *pBase, WlPostTable *pTable, 
		RsslPostMsg *pPostMsg, RsslErrorInfo *pErrorInfo);

/* Removes a record from the Post ID table. */
void wlPostTableRemoveRecord(WlPostTable *pTable, WlPostRecord *pRecord);

/* Matches an RsslAckMsg to a post in the table. */
WlPostRecord *wlPostTableFindRecord(WlPostTable *pTable, RsslAckMsg *pAckMsg);




#ifdef __cplusplus
}
#endif

#endif
