/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2019-2020,2024 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

/* TransportPerf.h
 * The main TransportPerf application.  This application may act as a client or server as
 * appropriate, and tests the sending of raw messages across connections. */

#ifndef _ETAC_TRANSPORT_PERF_H
#define _ETAC_TRANSPORT_PERF_H

#include "transportThreads.h"
#include "channelHandler.h"
#include "rtr/rsslGetTime.h"
#include "rtr/rsslThread.h"
#include "rtr/rsslTransport.h"
#include "rtr/rsslIterators.h"
#if defined(_WIN32)
#include <winsock2.h>
#include <time.h>
#else
#include <sys/time.h>
#include <sys/resource.h>
#include <sys/types.h>
#include <sys/timeb.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif

#ifndef MSGLEN_SZ 
#define MSGLEN_SZ	_FIELD_MSGLEN_SIZE
#endif

#ifndef SEQNUM_SZ 
#define SEQNUM_SZ	_FIELD_DATA_SIZE
#endif

typedef enum {
	ROLE_UNINIT			= 0x00,
	ROLE_WRITER			= 0x01,
	ROLE_READER			= 0x02,
	ROLE_REFLECTOR		= 0x04
} transportTestRole;

	

typedef struct 
{
	RsslMutex				handlerLock;		/* Lock for the handler. */
	RsslQueue				newChannelsList;	/* New channels from the main thread are passed to the handler through this queue. */
	RsslInt32				openChannelsCount;	/* Total number of channels this thread is currently handling. */
	RsslBool				active;				/* Connections are active, run the test */
	RsslThreadId			threadId;			/* ThreadID for this handler. */
	RsslInt32				cpuId;				/* ID of a CPU core this thread should be bound to, if any. */
	TimeRecordQueue			latencyRecords;		/* Latency records from the TransportThread. */
	TransportThread			transportThread;	/* Thread associated with this handler. */
	transportTestRole		role;				/* Role of this handler. */
	RsslMCastStats			prevMCastStats;		/* Stores any multicast statistics. */
} SessionHandler;

/* ChannelHandler callback for initialized channels. */
RsslRet processActiveChannel(ChannelHandler *pChanHandler, ChannelInfo *pChannelInfo);

/* ChannelHandler callback for inactive channels. */
void processInactiveChannel(ChannelHandler *pChanHandler, ChannelInfo *pChannelInfo, RsslError *pError);

/* ChannelHandler callback for RsslMsgs. */
RsslRet processMsg(ChannelHandler *pChannelHandler, ChannelInfo* pChannelInfo, RsslBuffer* pBuffer);

/* ChannelHandler callback for RsslMsgs, when acting as a message reflector. */
RsslRet processMsgReflect(ChannelHandler *pChanHandler, ChannelInfo *pChannelInfo, RsslBuffer *pMsgBuf);

/* Initializes a SessionHandler structure. */
RTR_C_INLINE void sessionHandlerInit(SessionHandler *pHandler)
{
	RSSL_MUTEX_INIT(&pHandler->handlerLock);
	rsslInitQueue(&pHandler->newChannelsList);
	pHandler->openChannelsCount = 0;
	pHandler->active = RSSL_FALSE;
	timeRecordQueueInit(&pHandler->latencyRecords);
	pHandler->cpuId = -1;

	pHandler->role = (transportTestRole)(ROLE_READER | ROLE_WRITER);
	memset(&pHandler->prevMCastStats, 0, sizeof(pHandler->prevMCastStats));
}

/* Cleans up a SessionHandler structure. */
RTR_C_INLINE void sessionHandlerCleanup(SessionHandler *pHandler)
{
	RSSL_MUTEX_DESTROY(&pHandler->handlerLock);
	timeRecordQueueCleanup(&pHandler->latencyRecords);
	transportThreadCleanup(&pHandler->transportThread);
}

/* Used to pass new channels to the TransportThreads. */
typedef struct {
	RsslQueueLink queueLink;
	RsslChannel *pChannel;
} NewChannel;

/* Binds an RsslServer. */
static RsslServer* bindRsslServer(RsslError* error);

/* Attempts to connect. */
static RsslChannel* startConnection();

/* Initializes application runtime. */
static void initRuntime();

/* Checks if we've reached the end time. */
static void handleRuntime(RsslInt64 currentTime);

/* Cleans up and exits the applications. */
void cleanUpAndExit();

/* Determine which thread has the fewest connections, and pass the new connection to it. */
static RsslRet sendToLeastLoadedThread(RsslChannel *chnl);

/* Collect statistics from the connection thread(s). */
void collectStats(RsslBool writeStats, RsslBool displayStats, RsslUInt32 currentRuntimeSec, RsslUInt32 timePassedSec);

#ifdef __cplusplus
};
#endif

#endif
