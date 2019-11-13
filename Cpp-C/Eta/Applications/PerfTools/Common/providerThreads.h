/*
 * This source code is provided under the Apache 2.0 license and is provided
 * AS IS with no warranty or guarantee of fit for purpose.  See the project's 
 * LICENSE.md for details. 
 * Copyright (C) 2019 Refinitiv. All rights reserved.
*/

/* providerThreads.h
 * Provides the logic that connections use with upacProvPerf and upacNIProvPerf for maintaining
 * channels and open items for a connection, and manages the sending of refreshes and updates. */

#ifndef _PROVIDER_THREADS_H
#define _PROVIDER_THREADS_H

#include "latencyRandomArray.h"
#include "itemEncoder.h"
#include "xmlItemListParser.h"
#include "channelHandler.h"
#include "statistics.h"
#include "rtr/rsslQueue.h"
#include "hashTable.h"

#include "rtr/rsslErrorInfo.h"
#include "rtr/rsslReactorChannel.h"
#include "rtr/rsslReactor.h"

#ifdef __cplusplus
extern "C" {
#endif

#define LATENCY_RANDOM_ARRAY_SET_COUNT 20
#define ALWAYS_SEND_LATENCY_UPDATE (-1)
#define ALWAYS_SEND_LATENCY_GENMSG (-1)

/* Identifies whether this represents an interactive or non-interactive provider. */
typedef enum
{
	PROVIDER_INTERACTIVE,		/* Interactive Provider. */
	PROVIDER_NONINTERACTIVE		/* Non-Interactive Provider. */
} ProviderType;


/* Provides the global configuration for ProviderThreads. */
typedef struct
{
	RsslInt32	ticksPerSec;				/* Controls granularity of update bursts
											 * (how they must be sized to match the desired update rate). */
	RsslInt32	totalBuffersPerPack;		/* How many messages are packed into a given buffer. */
	RsslUInt32	packingBufferLength;		/* Size of packable buffer, if packing. */
	RsslInt32	refreshBurstSize;			/* Total refresh rate per second */
	RsslInt32	updatesPerSec;				/* Total update rate per second(includes latency updates). */
	RsslInt32	latencyUpdatesPerSec;		/* Total latency update rate per second */
	RsslInt32	genMsgsPerSec;				/* Total generic message send rate per second(includes latency gen msgs). */
	RsslInt32	latencyGenMsgsPerSec;		/* Total latency generic messages rate per second */
	char		itemFilename[128];			/* Item List file. Provides a list of items to open. */
	char		msgFilename[128];			/* Data file. Describes the data to use when encoding messages. */
	RsslBool	logLatencyToFile;			/* Whether to log genMsg latency information to a file. See -latencyFile. */
	char		latencyLogFilename[128];	/* Name of the latency log file. See -latencyFile. */
	RsslInt32	_updatesPerTick;			/* Updates per tick */
	RsslInt32	_updatesPerTickRemainder;	/* Updates per tick (remainder) */
	RsslInt32	_genMsgsPerTick;			/* Generic messages per tick */
	RsslInt32	_genMsgsPerTickRemainder;	/* Generic messages per tick (remainder) */
	LatencyRandomArray	
		_latencyUpdateRandomArray;				/* Determines when to send latency updates. */
	LatencyRandomArray	
		_latencyGenMsgRandomArray;				/* Determines when to send latency gen msgs. */

	RsslBool	preEncItems;				/* Whether to use pre-encoded data rather than fully encoding. */
	RsslBool	takeMCastStats;				/* Running a multicast connection and we want stats. */
	RsslBool	nanoTime;   				/* Configures timestamp format. */
	RsslBool	measureEncode;				/* Measure time to encode messages(-measureEncode) */
	RsslBool	measureDecode;				/* Measure time to decode latency updates (-measureDecode) */

	RsslInt32	*threadBindList;			/* List of CPU ID's to bind threads to */
	RsslInt32	threadCount;				/* Number of provider threads to create. */
	char		statsFilename[128];			/* Name of the statistics log file*/
	RsslUInt8	writeFlags;
} ProviderThreadConfig;

/* Contains the global ProviderThread configuration. */
extern ProviderThreadConfig providerThreadConfig;

/* Keeps track of which dictionaries the consumer has. */
typedef enum
{
	DICTIONARY_STATE_NONE				= 0x0,	/* No dictionaries ready. */
	DICTIONARY_STATE_HAVE_FIELD_DICT	= 0x1,	/* Field dictionary ready. */
	DICTIONARY_STATE_HAVE_ENUM_DICT		= 0x2	/* Enumerated types dictionary ready. */
} DictionaryStateFlags;

typedef struct
{
	TimeValue			inactiveTime;
	TimeValue			firstGenMsgSentTime;		/* Time at which first generic message sent. */
	TimeValue			firstGenMsgRecvTime;		/* Time at which first generic message received. */
	ValueStatistics		genMsgLatencyStats;			/* Gen Msg latency statistics. */
	ValueStatistics		intervalGenMsgLatencyStats;	/* Gen Msg latency statistics (recorded by stats thread). */
	CountStat			genMsgSentCount;			/* Counts generic messages sent. */
	CountStat			genMsgRecvCount;			/* Counts generic messages received. */
	CountStat			latencyGenMsgSentCount;		/* counts latency generic messages sent. */
} ProvStats;

RTR_C_INLINE void provStatsInit(ProvStats *stats)
{
	stats->inactiveTime = 0;
	stats->firstGenMsgSentTime = 0;
	stats->firstGenMsgRecvTime = 0;
	initCountStat(&stats->genMsgSentCount);
	initCountStat(&stats->genMsgRecvCount);
	initCountStat(&stats->latencyGenMsgSentCount);
	clearValueStatistics(&stats->genMsgLatencyStats);
	clearValueStatistics(&stats->intervalGenMsgLatencyStats);
}

/*** ProviderThread ****/
/* ProviderThreads are used to control individual threads. Each thread
 * handles providing data to its open channels. */

/* Stores information and statistics related to a particular provider thread. */
typedef struct
{
	RsslInt32				providerIndex;			/* Index given to this provider thread. */
	RsslInt32				currentTicks;			/* Current tick out of ticks per second. */
	CountStat				refreshMsgCount;		/* Counts refreshes sent. */
	CountStat				updateMsgCount;			/* Counts updates sent. */
	CountStat				itemRequestCount;		/* Counts requests received. */
	CountStat				closeMsgCount;			/* Counts closes received. */
	CountStat				postMsgCount;			/* Counts posts received. */
	CountStat				outOfBuffersCount;		/* Counts updates not sent due to lack
													 * of output buffers. */
	CountStat				msgSentCount;			/* Counts total messages sent. */
	CountStat				bufferSentCount;		/* Counts total buffers sent(used with
													 * msgSentCount for packing statistics). */
	ProvStats				stats;					/* Other stats, collected periodically by the main thread. */
	RsslQueue				newClientSessionsList;	/* List of any new channels to add. */
	RsslMutex				newClientSessionsLock;	/* Lock for newClientSessionsList. */
	RsslInt32				clientSessionsCount;	/* Number of channels in use. */
	RsslThreadId			threadId;				/* Thread ID. */
	RsslInt32				cpuId;					/* CPU to bind to, if any. */
	ChannelHandler			channelHandler;			/* Channel handler. */
	LatencyRandomArrayIter	randArrayIter;			/* Iterator for the randomized latency array. */
	FILE					*statsFile;				/* Statistics file for recording. */
	FILE					*latencyLogFile;		/* File for logging latency for this connection. */
	void*					pUserSpec;				/* Optional user pointer. */
	RsslDataDictionary*		pDictionary;			/* Dictionary */
	DictionaryStateFlags	dictionaryStateFlags;	/* Dictionary State */
	RsslLocalFieldSetDefDb	fListSetDef;			/* Set definition, if needed. */
	char					setDefMemory[3825];		/* Memory for set definitions.  */
	TimeRecordQueue			genMsgLatencyRecords;	/* Queue of timestamp information(for gen msgs), collected periodically by the main thread. */
	RsslReactor				*pReactor;				/* Used for when application uses VA Reactor instead of UPA Channel. */
	RsslReactorOMMNIProviderRole niProviderRole;	/* Used for when application uses VA Reactor instead of UPA Channel. */
	RsslRDMLoginRequest		loginRequest;			/* Used for when application uses VA Reactor instead of UPA Channel. */
	RsslRDMDirectoryRefresh directoryRefresh;		/* Used for when application uses VA Reactor instead of UPA Channel. */
	RsslRDMService			service;				/* Used for when application uses VA Reactor instead of UPA Channel. */
	fd_set					readfds;				/* Read file descriptor set. Used for when application uses VA Reactor instead of UPA Channel. */
	fd_set					exceptfds;				/* Exception file descriptor set. Used for when application uses VA Reactor instead of UPA Channel. */
	fd_set					wrtfds;					/* Write file descriptor set. Used for when application uses VA Reactor instead of UPA Channel. */

	RsslMCastStats prevMCastStats;
	TimeRecordQueue messageEncodeTimeRecords;	/* Measurement of encoding time */
	TimeRecordQueue	updateDecodeTimeRecords;	/* Time spent decoding msgs. */
} ProviderThread;

/* Represents one channel's session.  Stores information about items requested. */
typedef struct {
	RotatingQueue	refreshItemList;		/* list of items to send refreshes for */
	RotatingQueue	updateItemList;			/* list of items to send updates for */
	HashTable		itemAttributesTable;	/* Open items indexed by attributes */
	HashTable		itemStreamIdTable;		/* Open items indexed by StreamID */
	RsslInt32		openItemsCount;			/* Count of items currently open */
	ChannelInfo		*pChannelInfo;
	ProviderThread	*pProviderThread;
	RsslBuffer		*pWritingBuffer;		/* Current buffer in use by this channel. */
	RsslInt32		packedBufferCount;		/* Total number of buffers currently packed in pWritingBuffer */
	TimeValue		timeActivated;			/* Time at which this channel was fully setup. */
	RsslRet			lastWriteRet;			/* Last return from an rsslWrite call. */

	RsslBuffer		*preEncMarketPriceMsgs;		/* Buffer of a pre-encoded market price message, if sending pre-encoded items;  This is allocated per-channel in case the versions are different */
	RsslBuffer		*preEncMarketByOrderMsgs;	/* Buffer of a pre-encoded market by order message, if sending pre-encoded items;  This is allocated per-channel in case the versions are different */
} ProviderSession;

/* Clears providerThreadConfig to defaults. */
void clearProviderThreadConfig();

/* Initializes providerThreadConfig(call once configuration values are set). */
void providerThreadConfigInit();

/* Initializes a ProviderThread. */
void providerThreadInit(ProviderThread *pProvThread,
		ChannelActiveCallback *processActiveChannel,
		ChannelInactiveCallback *processInactiveChannel,
		MsgCallback *processMsg,
		RsslInt32 providerIndex,
		ProviderType providerType);

/* Cleans up providerThreadConfig. */
void providerThreadConfigCleanup();

/* Sends a burst of refreshes for items that currently need to send one. */
RsslRet sendRefreshBurst(ProviderThread *pProvThread, ProviderSession *pSession);

/* Sends a burst of item updates. */
RsslRet sendUpdateBurst(ProviderThread *pProvThread, ProviderSession *pSession);

/* Sends a burst of item gerneric messages. */
RsslRet sendGenMsgBurst(ProviderThread *pProvThread, ProviderSession *pSession);

/* Displays the estimated RsslBuffer size that will be used for encoding,
 * as well as example sizes from an actual encoding. */
RsslRet printEstimatedMsgSizes(ProviderThread *pProvThread, ProviderSession *pSession);

/*** ProviderSession functions ***/

/* Creates a ProviderSession. */
ProviderSession *providerSessionCreate(ProviderThread *pProvThread, RsslChannel *pChannel);

/* Cleans up a ProviderSession. */
void providerSessionDestroy(ProviderThread *pProvThread, ProviderSession *pSession);

/* For Non-Interactive Providers.  Reads in items from the configured XML file and creates
 * a publishing list. */
RsslRet providerSessionAddPublishingItems(ProviderThread *pProvThread, ProviderSession *pSession, 
		RsslInt32 commonItemCount, RsslInt32 itemListUniqueIndex, RsslInt32 uniqueItemCount, 
		RsslUInt16 serviceId);

/* Hash sum function for hashing item attributes. */
static RsslUInt32 hashSumItemAttributes(void *key);

/* Hash compare function for item attributes. */
static RsslBool hashCompareItemAttributes(void *key, HashTableLink *pLink);

/* Hash compare function for stream ID's. */
static RsslBool hashCompareStreamId(void *key, HashTableLink *pLink);

/*** ItemInfo functions ***/

/* Creates an ItemInfo. */
ItemInfo *createItemInfo(ProviderThread *pProvThread, ProviderSession *pSession, RsslItemAttributes *pAttributes, RsslInt32 streamId);

/* Cleans up an ItemInfo. */
static void freeItemInfo(ProviderThread *pProvThread, ProviderSession *pSession, ItemInfo* itemInfo);

/* Called in response to receiving a close request. Cleans up the item associated with the stream ID. */
void closeItemStream(ProviderThread *pProvThread, ProviderSession *pSession, RsslInt32 streamId);

/*** Message Buffer functions ***/

/* Gets an RsslBuffer for encoding a message.
 * This function handles packing of messages, if packing is configured -- it will pack as
 * long as appropriate, stopping to write if the present buffer is too full to accommodate
 * the requested length. */
RsslRet getItemMsgBuffer(ProviderThread *pProvThread, ProviderSession *pSession, RsslUInt32 length);

/* Sends a completed RsslBuffer.
 * This function packs messages, if packing is configured.  The allowPack option may be used to 
 * prevent packing if needed(for example, we just encoded the last message of a burst so it is time 
 * to write to the transport).  */
RsslRet sendItemMsgBuffer(ProviderThread *pProvThread, ProviderSession *pSession, RsslBool allowPack);


/* Used to pass new channels to ProviderThreads. */
typedef struct {
	RsslQueueLink		queueLink;
	RsslChannel			*pChannel;
} NewChannel;

/* Clears a NewChannel. */
RTR_C_INLINE void clearNewChannel(NewChannel *pNewChannel)
{
	memset(pNewChannel, 0, sizeof(NewChannel));
}

/* Send refreshes & updates & generic messages to channels open. */
void providerThreadSendMsgBurst(ProviderThread *pProvThread, RsslUInt64 stopTime);

/* Sends a newly connected channel to a  provider thread. */
void providerThreadSendNewChannel(ProviderThread *pProvThread, RsslChannel *pChannel);

/* Provider thread method for adding new channels. */
void providerThreadReceiveNewChannels(ProviderThread *pProvThread);

/* Get the current number of channels open on a provider thread. */
RsslInt32 providerThreadGetConnectionCount(ProviderThread *pProvThread);

/* Read from a provider thread's channels. */
RTR_C_INLINE void providerThreadRead(ProviderThread *pProvThread, RsslUInt64 stopTime)
{
	channelHandlerReadChannels(&pProvThread->channelHandler, stopTime);
}

/* Check ping timeouts. */
RTR_C_INLINE void providerThreadCheckPings(ProviderThread *pProvThread)
{
	channelHandlerCheckPings(&pProvThread->channelHandler);
}

/* Close a channel on a provider thread. */
RTR_C_INLINE void providerThreadCloseChannel(ProviderThread *pProvThread, ChannelInfo *pChannelInfo)
{
	channelHandlerCloseChannel(&pProvThread->channelHandler, pChannelInfo, NULL); 
	RSSL_MUTEX_LOCK(&pProvThread->newClientSessionsLock);
	--pProvThread->clientSessionsCount;
	RSSL_MUTEX_UNLOCK(&pProvThread->newClientSessionsLock);
}

/* Indicate that rsslFlush() should be called on a particular channel.
 * This is normally done when the last call to rsslWrite returned a value greater than 
 * RSSL_RET_SUCCESS. */
RTR_C_INLINE void providerThreadRequestChannelFlush(ProviderThread *pProvThread, ChannelInfo *pChannelInfo)
{
	channelHandlerRequestFlush(&pProvThread->channelHandler, pChannelInfo);
}

/* Maintains the global provider application instance.
 * Has logic for keeping track of things like CPU/Mem usage,
 * and the list of provider threads. */
typedef struct
{
	ProviderThread		*providerThreadList;	/* List of provider threads. */
	ResourceUsageStats	resourceStats;			/* Records CPU/Memory usage. */
	ValueStatistics		cpuUsageStats;			/* Sampled CPU statistics. */
	ValueStatistics		memUsageStats;			/* Sampled memory usage statistics. */
	ProviderType		providerType;			/* Type of provider. */

	CountStat refreshCount;						/* Count of refreshes sent. */
	CountStat updateCount;						/* Count of updates sent. */
	CountStat requestCount;						/* Count of requests received. */
	CountStat closeCount;						/* Count of close requests received. */
	CountStat postCount;						/* Count of posts received. */
	CountStat outOfBuffersCount;				/* Count of messages blocked due to running out of
												 * output buffers. */
	CountStat msgSentCount;						/* Count of total messages sent(used w/ packing). */
	CountStat bufferSentCount;					/* Count of total number of buffers sent(used w/ packing). */

	ValueStatistics msgEncodingStats;
	ValueStatistics intervalMsgEncodingStats;
	CountStat mcastPacketSentCount;
	CountStat mcastPacketReceivedCount;
	CountStat mcastRetransSentCount;
	CountStat mcastRetransReceivedCount;
} Provider;

/* Initializes a provider. */
void providerInit(Provider *pProvider, ProviderType providerType,
		ChannelActiveCallback *processActiveChannel,
		ChannelInactiveCallback *processInactiveChannel,
		MsgCallback *processMsg);

/* Starts all provider threads. */
void startProviderThreads(Provider *pProvider, RSSL_THREAD_DECLARE(threadFunction,pArg));

/* Waits for all provider threads to stop. */
void providerWaitForThreads(Provider *pProvider);

/* Cleans up the provider. */
void providerCleanup(Provider *pProvider);

/* Write provider statistics. Stats will reflect changes from the previous call to this function. */
void providerCollectStats(Provider *pProvider, RsslBool writeStats, RsslBool displayStats, RsslUInt32 currentRuntimeSec,
		RsslUInt32 timePassedSec);

/* Print summary statistics. Calling providerWaitForThreads() first is recommended. */
void providerPrintSummaryStats(Provider *pProvider, FILE *file);

#ifdef __cplusplus
}
#endif

#endif
