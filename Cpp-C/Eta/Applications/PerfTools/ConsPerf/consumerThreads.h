/*
 * This source code is provided under the Apache 2.0 license and is provided
 * AS IS with no warranty or guarantee of fit for purpose.  See the project's 
 * LICENSE.md for details. 
 * Copyright Thomson Reuters 2015. All rights reserved.
*/

/* consumerThreads.h
 * Provides the logic that consumer connections use in upacConsPerf for
 * connecting to a provider, requesting items, and processing the received refreshes and updates. */

#ifndef _CONSUMER_THREADS_H
#define _CONSUMER_THREADS_H

#ifdef __cplusplus
extern "C" {
#endif

#include "consPerfConfig.h"
#include "itemEncoder.h"
#include "latencyRandomArray.h"
#include "statistics.h"
#include "rtr/rsslTransport.h"
#include "rtr/rsslMessagePackage.h"
#include "rtr/rsslRDMMsg.h"
#include "rtr/rsslDataPackage.h"
#include "rtr/rsslQueue.h"
#include "rtr/rsslErrorInfo.h"
#include "rtr/rsslReactor.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <ctype.h>
#include <math.h>
#ifdef _WIN32
#include <winsock2.h>
#include <time.h>
#include <process.h>
#else
#include <sys/time.h>
#include <sys/types.h>
#include <sys/timeb.h>
#include <unistd.h>
#endif

/* Contains the PostUserInfo all consumer connections should use when posting. 
 * Set my the main thread. */
extern RsslPostUserInfo postUserInfo;

/* Indicates when threads should shutdown. */
extern RsslBool shutdownThreads;

/* ItemRequestState
 * Tracks the refresh state of an item. */
typedef enum
{
	ITEM_NOT_REQUESTED,			/* Item request has not been set. */
	ITEM_WAITING_FOR_REFRESH,	/* Item is waiting for its solicited refresh. */
	ITEM_HAS_REFRESH			/* Item has received its solicited refresh. */
} ItemRequestState;

typedef struct {
	RsslQueueLink	link;			/* Link for item list. */
	RsslQueueLink	postQueueLink;	/* Link for posting item list. */
	RsslQueueLink	genMsgQueueLink;	/* Link for sending gen msg item list. */
	ItemRequestState requestState;	/* Item state. */
	char itemNameChar[256];			/* Storage for the item's name. */
	ItemInfo itemInfo;				/* Structure containing item information. */
	RsslMsgKey msgKey;				/* Message key used to request this item. */
} ItemRequest;

/* Maintains counts and other values for measuring statistics on a consumer thread. */
typedef struct {
	TimeValue	imageRetrievalStartTime;		/* Time at which first item request was made. */
	TimeValue	imageRetrievalEndTime;			/* Time at which last item refresh was received. */
	TimeValue	firstUpdateTime;				/* Time at which first item update was received. */
	TimeValue	firstGenMsgSentTime;			/* Time at which first generic message was sent */
	TimeValue	firstGenMsgRecvTime;			/* Time at which first generic message was received */


	CountStat		refreshCount;				/* Number of item refreshes received. */
	CountStat		startupUpdateCount;			/* Number of item updates received during startup. */
	CountStat		steadyStateUpdateCount;		/* Number of item updates received during steady state. */
	CountStat		requestCount;				/* Number of requests sent. */
	CountStat		statusCount;				/* Number of item status messages received. */
	CountStat		postSentCount;				/* Number of posts sent. */
	CountStat		postOutOfBuffersCount;		/* Number of posts not sent due to lack of buffers. */
	CountStat		genMsgSentCount;			/* Number of generic msgs sent. */
	CountStat		genMsgRecvCount;			/* Number of generic msgs received. */
	CountStat		latencyGenMsgSentCount;		/* Number of latency generic msgs sent. */
	CountStat		genMsgOutOfBuffersCount;	/* Number of generic msgs not sent due to lack of buffers. */
	ValueStatistics	intervalLatencyStats;		/* Latency statistics (recorded by stats thread). */
	ValueStatistics	intervalPostLatencyStats;	/* Post latency statistics (recorded by stats thread). */
	ValueStatistics	intervalGenMsgLatencyStats;	/* Gen Msg latency statistics (recorded by stats thread). */

	ValueStatistics startupLatencyStats;		/* Statup latency statistics. */
	ValueStatistics steadyStateLatencyStats;	/* Steady-state latency statistics. */
	ValueStatistics overallLatencyStats;		/* Overall latency statistics. */
	ValueStatistics postLatencyStats;			/* Posting latency statistics. */
	ValueStatistics genMsgLatencyStats;			/* Gen Msg latency statistics. */
	RsslBool		imageTimeRecorded;			/* Stats thread sets this once it has recorded/printed
												 * this consumer's image retrieval time. */
	ValueStatistics intervalUpdateDecodeTimeStats;
} ConsumerStats;

RTR_C_INLINE void consumerStatsInit(ConsumerStats *stats)
{
	stats->imageRetrievalStartTime = 0;
	stats->imageRetrievalEndTime = 0;
	stats->firstUpdateTime = 0;
	stats->firstGenMsgSentTime = 0;
	stats->firstGenMsgRecvTime = 0;
	initCountStat(&stats->startupUpdateCount);
	initCountStat(&stats->steadyStateUpdateCount);
	initCountStat(&stats->refreshCount);
	initCountStat(&stats->requestCount);
	initCountStat(&stats->statusCount);
	initCountStat(&stats->postSentCount);
	initCountStat(&stats->genMsgSentCount);
	initCountStat(&stats->genMsgRecvCount);
	initCountStat(&stats->postOutOfBuffersCount);
	initCountStat(&stats->genMsgOutOfBuffersCount);
	clearValueStatistics(&stats->intervalLatencyStats);
	clearValueStatistics(&stats->intervalPostLatencyStats);
	clearValueStatistics(&stats->intervalGenMsgLatencyStats);
	clearValueStatistics(&stats->startupLatencyStats);
	clearValueStatistics(&stats->steadyStateLatencyStats);
	clearValueStatistics(&stats->overallLatencyStats);
	clearValueStatistics(&stats->postLatencyStats);
	clearValueStatistics(&stats->genMsgLatencyStats);
	stats->imageTimeRecorded = RSSL_FALSE;
	clearValueStatistics(&stats->intervalUpdateDecodeTimeStats);
}

/* Keeps track of which dictionaries the consumer has. */
typedef enum
{
	DICTIONARY_STATE_NONE				= 0x0,	/* No dictionaries ready. */
	DICTIONARY_STATE_HAVE_FIELD_DICT	= 0x1,	/* Field dictionary ready. */
	DICTIONARY_STATE_HAVE_ENUM_DICT		= 0x2	/* Enumerated types dictionary ready. */
} DictionaryStateFlags;


/* Contains information about a consumer thread and its connection. */
typedef struct {
	RsslThreadId			threadId;					/* ID saved from thread creation. */
	RsslInt32				cpuId;						/* CPU to bind the thread to, if any */
    RsslChannel				*pChannel;					/* RSSL Channel. */
	TimeRecordQueue			latencyRecords;				/* Queue of timestamp information, collected periodically by the main thread. */
	TimeRecordQueue			postLatencyRecords;			/* Queue of timestamp information(for posts), collected periodically by the main thread. */
	TimeRecordQueue			genMsgLatencyRecords;		/* Queue of timestamp information(for gen msgs), collected periodically by the main thread. */
	int						latStreamId;				/* The Stream ID of the latency item. */

	RsslInt32				itemListUniqueIndex;		/* Index into the item list at which item 
														 * requests unique to this consumer start. */
	RsslInt32				itemListCount;				/* Number of item requests to make. */

	RsslDataDictionary*		pDictionary;				/* Dictionary */
	DictionaryStateFlags	dictionaryStateFlags;
	fd_set					readfds;					/* Read file descriptor set */
	fd_set					exceptfds;					/* Exception file descriptor set */
	fd_set					wrtfds;						/* Write file descriptor set */
	
	time_t					nextReceivePingTime;		/* Last time we received a ping. */
	time_t					nextSendPingTime;			/* Next time at which we should send a ping. */
	RsslBool				receivedPing;				/* Indicates whether a ping or message was received since our last check. */

	TimeRecordQueue			updateDecodeTimeRecords;	/* Time spent decoding updates. */

	RsslLocalFieldSetDefDb	fListSetDef;				/* Set definition, if needed. */
	char					setDefMemory[3825];			/* Memory for set definitions.  */

	LatencyRandomArrayIter	randArrayIter;

	ConsumerStats			stats;						/* Other stats, collected periodically by the main thread. */
	FILE					*statsFile;					/* File for logging stats for this connection. */
	FILE					*latencyLogFile;			/* File for logging latency for this connection. */
	RsslErrorInfo			threadErrorInfo;

	RsslRDMDirectoryMsg		directoryMsgCopy;			/* Copy of the directory message. */
	RsslBuffer				directoryMsgCopyMemory;		/* Memory buffer for directoryMsgCopy. */
	RsslBuffer				directoryMsgCopyMemoryOrig;	/* Copy of memory buffer(used to cleanup) */
	ItemRequest				*itemRequestList;			/* List of items to request. */
	RsslReactor				*pReactor;					/* Used for when application uses VA Reactor instead of UPA Channel. */
	RsslReactorChannel		*pReactorChannel;			/* Used for when application uses VA Reactor instead of UPA Channel. */
	RsslReactorOMMConsumerRole consumerRole;			/* Used for when application uses VA Reactor instead of UPA Channel. */
	RsslRDMLoginRequest		loginRequest;				/* Used for when application uses VA Reactor instead of UPA Channel. */
	RsslRDMDirectoryRequest	dirRequest;					/* Used for when application uses VA Reactor instead of UPA Channel. */
	RsslRDMService          *pDesiredService;           /* Store information about the desired service once we find it. */
	RsslQueue				requestQueue;				/* Request queue. */
	RsslQueue				waitingForRefreshQueue;		/* Waiting for refresh queue. */
	RsslQueue				refreshCompleteQueue;		/* Refresh complete queue. */
	RotatingQueue           postItemQueue;              /* Post item queue. */
	RotatingQueue           genMsgItemQueue;            /* Generic message item queue. */
} ConsumerThread;

/* Shorthand for consumerThread's RsslError struct. */
#define threadRsslError		threadErrorInfo.rsslError

/* Clears an ItemRequest. */
RTR_C_INLINE void clearItemRequest(ItemRequest *pInfo)
{
	rsslInitQueueLink(&pInfo->link);
	rsslClearMsgKey(&pInfo->msgKey);
	clearItemInfo(&pInfo->itemInfo);
	pInfo->requestState = ITEM_NOT_REQUESTED;
	pInfo->itemInfo.attributes.pMsgKey = &pInfo->msgKey;
}

/* Initializes a consumer thread. */
void consumerThreadInit(ConsumerThread *pConsumerThread, RsslInt32 consThreadId);

/* Cleans up a consumer thread. */
void consumerThreadCleanup(ConsumerThread *pConsumerThread);

/* Initialize ping time on a consumer thread's connection. */
void consumerThreadInitPings(ConsumerThread* pConsumerThread);

/* Check ping times on a consumer thread's connection, sending a ping if needed. */
RsslBool consumerThreadCheckPings(ConsumerThread* pConsumerThread);

/* ConsumerThread function for use with UPA Channel. */
RSSL_THREAD_DECLARE(runConsumerChannelConnection, threadStruct);

/* ConsumerThread function for use with VA Reactor. */
RSSL_THREAD_DECLARE(runConsumerReactorConnection, threadStruct);

#ifdef __cplusplus
};
#endif

#endif

