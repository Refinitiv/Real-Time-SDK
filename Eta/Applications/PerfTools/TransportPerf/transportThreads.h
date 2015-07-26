/*
 * This source code is provided under the Apache 2.0 license and is provided
 * AS IS with no warranty or guarantee of fit for purpose.  See the project's 
 * LICENSE.md for details. 
 * Copyright Thomson Reuters 2015. All rights reserved.
*/

/* transportThreads.h
 * Provides the logic that connections use with upacTransportPerf for sending messages
 * over connections. */

#ifndef _TRANSPORT_THREADS_H
#define _TRANSPORT_THREADS_H

#include "channelHandler.h"
#include "rtr/rsslTransport.h"
#include "rtr/rsslQueue.h"
#include "latencyRandomArray.h"
#include "statistics.h"

#ifdef __cplusplus
extern "C" {
#endif

#define ALWAYS_SEND_LATENCY_MSG (-1)

/* TransportThreadConfig
 * Provides the global configuration for TransportThreads. */
typedef struct
{
	RsslInt32	ticksPerSec;				/* Controls granularity of msg bursts
											 * (how they must be sized to match the desired msg rate). */
	RsslInt32	totalBuffersPerPack;		/* How many messages are packed into a given buffer. */
	RsslInt32	msgsPerSec;					/* Total msg rate per second(includes latency msgs). */
	RsslInt32	latencyMsgsPerSec;			/* Total latency msg rate per second */
	RsslUInt32	msgSize;					/* Size of messages to send. */
	RsslUInt8	writeFlags;					/* Flags to use when calling rsslWrite() */

	RsslInt32	_msgsPerTick;				/* Messages per tick */
	RsslInt32	_msgsPerTickRemainder;		/* Messages per tick (remainder) */
	RsslBool	checkPings;					/* Whether ping timeouts should be monitored. */
	char		statsFilename[128];			/* Name of the statistics log file*/
	RsslBool	logLatencyToFile;			/* Whether to log latency information to a file. See -latencyFile. */
	char		latencyLogFilename[128];	/* Name of the latency log file. See -latencyFile. */

} TransportThreadConfig;

/* Contains the global TransportThread configuration. */
extern TransportThreadConfig transportThreadConfig;

/* Clears the transportThreadConfig. */
void clearTransportThreadConfig();

/* Initializes the transportThreadConfig once options are set. */
void initTransportThreadConfig();

/* Cleans up memory associated with the transportThreadConfig. */
void cleanupTransportThreadConfig();

/* Stores information about an open session on a channel. */
typedef struct {
	ChannelInfo		*pChannelInfo;			/* Channel associated with this session */
	RsslUInt32		maxMsgBufSize;			/* The buffer size to request from RSSL via rsslGetBuffer(); May vary according to fragment size and packing */
	RsslBuffer		*pWritingBuffer;			/* Current buffer in use by this channel. */
	RsslInt32		packedBufferCount;		/* Total number of buffers currently packed in pWritingBuffer */
	RsslUInt64		sendSequenceNumber;		/* Next sequence number to send. */
	RsslUInt64		recvSequenceNumber;		/* Next sequence number that should be received. */
	RsslBool 
		receivedFirstSequenceNumber;		/* Indicates whether a sequence number has been received yet. */
	TimeValue		timeActivated;			/* Time at which this channel was fully setup. */
} TransportSession;

/* Handles one transport thread. */
typedef struct
{
	ChannelHandler			channelHandler;		/* Contains a list of open channels and maintains those channels. */
	RsslInt32				currentTicks;		/* Current position in ticks per second. */
	RsslInt32				threadIndex;
	LatencyRandomArrayIter	randArrayIter;		/* Iterator for latency random array. */
	TimeValue				connectTime;		/* Time of first connection. */
	TimeValue				disconnectTime; 	/* Time of last disconnection. */
	CountStat				msgsSent;			/* Total messages sent. */
	CountStat				bytesSent;			/* Total bytes sent(counting any compression) */
	CountStat				msgsReceived;		/* Total messages received. */
	CountStat				bytesReceived;		/* Total bytes received. */
	CountStat				outOfBuffersCount;	/* Messages not sent for lack of output buffers. */
	ValueStatistics			latencyStats;		/* Latency statistics (recorded by stats thread). */
	FILE					*statsFile;			/* Statistics file for recording. */
	FILE					*latencyLogFile;	/* File for logging latency for this thread. */
	void					*pUserSpec;
} TransportThread;

void transportThreadInit(TransportThread *pThread,
		ChannelActiveCallback *processActiveChannel,
		ChannelInactiveCallback *processInactiveChannel,
		MsgCallback *processMsg,
		RsslInt32 threadIndex);

void transportThreadCleanup(TransportThread *pThread);

/* Creates a TransportSession. */
TransportSession *transportSessionCreate(TransportThread *pHandler, RsslChannel *chnl);

/* Cleans up a TransportSession. */
void transportSessionDestroy(TransportThread *pHandler, TransportSession *pSession);

/* Send a burst of messages on a channel. */
RsslRet transportSessionSendMsgBurst(TransportThread *pHandler, TransportSession *pSession, RsslInt32 *pMsgsSent);

/* Send a message on a channel. */
static RsslRet transportSessionSendMsg(TransportThread *pHandler, TransportSession *pSession, RsslInt32 msgsLeft, RsslBool sendLatency);

/*** Message Buffer functions ***/

RsslRet getMsgBuffer(TransportSession *pSession);
RsslRet writeMsgBuffer(TransportThread *pHandler, TransportSession *pSession, RsslBool allowPack);

#ifdef __cplusplus
}
#endif

#endif

