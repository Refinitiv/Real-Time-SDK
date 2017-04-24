/*
 * This source code is provided under the Apache 2.0 license and is provided
 * AS IS with no warranty or guarantee of fit for purpose.  See the project's 
 * LICENSE.md for details. 
 * Copyright Thomson Reuters 2015. All rights reserved.
*/

#ifndef _RTR_RSSL_REACTOR_IMPL_H
#define _RTR_RSSL_REACTOR_IMPL_H

#include "rtr/rsslReactor.h"
#include "rtr/rsslWatchlist.h"
#include "rtr/rsslReactorEventQueue.h"
#include "rtr/rsslRDMMsg.h"
#include "rtr/rsslErrorInfo.h"
#include "rtr/rsslVAUtils.h"

#include "rtr/rsslQueue.h"
#include "rtr/rsslThread.h"
#include "rtr/rsslReactorUtils.h"
#include "rtr/tunnelManager.h"

#ifdef WIN32
#include <windows.h>
#else
#include <sys/time.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif

typedef struct _RsslReactorImpl RsslReactorImpl;


typedef enum
{
	RSSL_RC_CHST_INIT = 0,
	RSSL_RC_CHST_LOGGED_IN = 1,
	RSSL_RC_CHST_HAVE_DIRECTORY = 2,
	RSSL_RC_CHST_HAVE_RWFFLD = 3,
	RSSL_RC_CHST_HAVE_RWFENUM = 4,
	RSSL_RC_CHST_READY = 5,
	RSSL_RC_CHST_RECONNECTING = 6
} RsslReactorChannelSetupState;

/* RsslReactorChannelImpl 
 * - Handles a channel associated with the RsslReactor */
typedef struct 
{
	RsslReactorChannel reactorChannel;

	RsslReactorImpl *pParentReactor;
	RsslUInt32 initializationTimeout;
	RsslInt64 initializationStartTimeMs;

	/* Reactor side only */
	RsslQueueLink reactorQueueLink;
	RsslQueue *reactorParentQueue;
	RsslReactorEventQueue eventQueue;
	RsslInt64 lastPingReadMs;
	RsslRet readRet;				/* Last return code from rsslRead on this channel. Helps determine whether data can still be read from this channel. */
	RsslRet writeRet;				/* Last return from rsslWrite() for this channel. Helps determine whether we should request a flush. */
	RsslBool requestedFlush;		/* Indicates whether flushing is signaled for this channel */
	RsslWatchlist *pWatchlist;
	RsslBool	wlDispatchEventQueued;
	RsslBool	tunnelDispatchEventQueued;

	RsslRDMMsg rdmMsg;				/* The typed message that has been decoded */
	RsslReactorChannelSetupState channelSetupState;
	RsslBuffer *pWriteCallAgainBuffer; /* Used when WRITE_CALL_AGAIN is returned from an internal rsslReactorSubmit() call. */

	RsslReactorChannelRole channelRole;

	/* When a consumer connection is using the downloadDictionaries feature, store the streamID's used for requesting field & enum type dictionaries. */
	RsslInt32 rwfFldStreamId;
	RsslInt32 rwfEnumStreamId;

	/* Worker thread only */
	RsslQueueLink workerLink;
	RsslQueue *workerParentList;
	RsslInt64 lastPingSentMs;
	RsslErrorInfo channelWorkerCerr;
	RsslInt64 lastRequestedExpireTime;
	RsslInt64 nextExpireTime;

	/* Reconnection logic */
	RsslInt32 reconnectMinDelay;
	RsslInt32 reconnectMaxDelay;
	RsslInt32 reconnectDelay;
	RsslInt32 reconnectAttemptLimit;
	RsslInt32 reconnectAttemptCount;
	RsslInt64 lastReconnectAttemptMs;
	
	RsslInt32 connectionListCount;
	RsslInt32 connectionListIter;
	RsslReactorConnectInfo *connectionOptList;
	TunnelManager *pTunnelManager;
} RsslReactorChannelImpl;

RTR_C_INLINE void rsslClearReactorChannelImpl(RsslReactorImpl *pReactorImpl, RsslReactorChannelImpl *pInfo)
{
	memset(pInfo, 0, sizeof(RsslReactorChannelImpl));
	pInfo->pParentReactor = pReactorImpl;
	pInfo->nextExpireTime = RCIMPL_TIMER_UNSET;
	pInfo->lastRequestedExpireTime = RCIMPL_TIMER_UNSET;
}

RTR_C_INLINE RsslRet _rsslChannelCopyConnectionList(RsslReactorChannelImpl *pReactorChannel, RsslReactorConnectOptions *pOpts)
{
	RsslConnectOptions *destOpts, *sourceOpts;
	RsslUInt32 i, j;

	if(pOpts->connectionCount != 0)
	{
		pReactorChannel->connectionOptList = (RsslReactorConnectInfo*)malloc(pOpts->connectionCount*sizeof(RsslReactorConnectInfo));

		if(!pReactorChannel->connectionOptList)
		{
			return RSSL_RET_FAILURE;
		}

		for(i = 0; i < pOpts->connectionCount; i++)
		{
			destOpts = &pReactorChannel->connectionOptList[i].rsslConnectOptions;
			sourceOpts = &pOpts->reactorConnectionList[i].rsslConnectOptions;

			if(rsslDeepCopyConnectOpts(destOpts, sourceOpts) != RSSL_RET_SUCCESS)
			{
				for(j = 0; j <= i; j++)
				{
					rsslFreeConnectOpts(&pReactorChannel->connectionOptList[j].rsslConnectOptions);
				}
				free(pReactorChannel->connectionOptList);
				return RSSL_RET_FAILURE;
			}

			pReactorChannel->connectionOptList[i].initializationTimeout = pOpts->reactorConnectionList[i].initializationTimeout;
		}
		pReactorChannel->connectionListCount = pOpts->connectionCount;
	}
	else
	{
		pReactorChannel->connectionOptList = (RsslReactorConnectInfo*)malloc(sizeof(RsslReactorConnectInfo));

		if(!pReactorChannel->connectionOptList)
		{
			return RSSL_RET_FAILURE;
		}

		if(rsslDeepCopyConnectOpts(&(pReactorChannel->connectionOptList->rsslConnectOptions), &pOpts->rsslConnectOptions) != RSSL_RET_SUCCESS)
		{
			rsslFreeConnectOpts(&pReactorChannel->connectionOptList->rsslConnectOptions);

			free(pReactorChannel->connectionOptList);
			return RSSL_RET_FAILURE;
		}

		pReactorChannel->connectionOptList->initializationTimeout = pOpts->initializationTimeout;
		pReactorChannel->connectionListCount = 1;
	}


	return RSSL_RET_SUCCESS;
}

RTR_C_INLINE RsslRet _rsslChannelFreeConnectionList(RsslReactorChannelImpl *pReactorChannel)
{
	int i;

	if(pReactorChannel->connectionOptList)
	{
		for(i = 0; i < pReactorChannel->connectionListCount; i++)
		{
			rsslFreeConnectOpts(&pReactorChannel->connectionOptList[i].rsslConnectOptions);
		}

		free(pReactorChannel->connectionOptList);
		pReactorChannel->connectionOptList = NULL;
	}
	
	return RSSL_RET_SUCCESS;

}

/* Reset reactor channel state in response to channel failure. */
RTR_C_INLINE void rsslResetReactorChannelState(RsslReactorImpl *pReactorImpl, RsslReactorChannelImpl *pReactorChannel)
{
	pReactorChannel->requestedFlush = 0;
	pReactorChannel->channelSetupState = RSSL_RC_CHST_INIT;
	pReactorChannel->lastPingReadMs = 0;
	pReactorChannel->readRet = 0;
	pReactorChannel->lastPingSentMs = 0;
	pReactorChannel->writeRet = 0;
	pReactorChannel->pWriteCallAgainBuffer = 0;
}


/* Fully reset reactor channel (used when channel is retrieved from pool). */
RTR_C_INLINE void rsslResetReactorChannel(RsslReactorImpl *pReactorImpl, RsslReactorChannelImpl *pReactorChannel)
{
	memset(&pReactorChannel->reactorChannel, 0, sizeof(RsslReactorChannel));
	rsslInitQueueLink(&pReactorChannel->reactorQueueLink);
	pReactorChannel->reactorParentQueue = 0;
	rsslClearReactorChannelRole(&pReactorChannel->channelRole);
	rsslInitQueueLink(&pReactorChannel->workerLink);
	pReactorChannel->workerParentList = 0;
	pReactorChannel->lastReconnectAttemptMs = 0;
	pReactorChannel->reconnectAttemptCount = 0;

	pReactorChannel->connectionListCount = 0;
	pReactorChannel->connectionListIter = 0;
	pReactorChannel->connectionOptList = NULL;
	pReactorChannel->reactorChannel.socketId = (RsslSocket)REACTOR_INVALID_SOCKET;
	pReactorChannel->reactorChannel.oldSocketId = (RsslSocket)REACTOR_INVALID_SOCKET;

	rsslResetReactorChannelState(pReactorImpl, pReactorChannel);
}

/* Verify that the given RsslReactorChannel is valid for this RsslReactor */
RTR_C_INLINE RsslBool rsslReactorChannelIsValid(RsslReactorImpl *pReactorImpl, RsslReactorChannelImpl *pInfo, RsslErrorInfo *pError)
{
	RsslBool ret = (pInfo->pParentReactor == pReactorImpl);

	if (ret == RSSL_FALSE)
		rsslSetErrorInfo(pError, RSSL_EIC_FAILURE, RSSL_RET_FAILURE, __FILE__, __LINE__, "Invalid channel.");
	return ret;
}

/* Checks that we are not in a callback already (or that's it's okay), and locks the reactor */
RsslRet reactorLockInterface(RsslReactorImpl *pReactorImpl, RsslBool allowedInCallback, RsslErrorInfo *pError);

/* Unlocks reactor */
RsslRet reactorUnlockInterface(RsslReactorImpl *pReactorImpl);

/* RsslReactorWorker
 * The reactorWorker handles when to send pings and flushing.
 * Primary responsiblities include:
 *   - Initializing channels and signaling the reactor when channel is active
 *   - Sending periodic pings on idle channels to keep them alive
 *   - Flushing in response to requests from the reactor and signaling when finished
 *   - Processing of timer events
 */
typedef struct 
{
	RsslQueue initializingChannels;	/* Channels to call rsslInitChannel() on */
	RsslQueue activeChannels;			/* Channels currently active */
	RsslQueue inactiveChannels;			/* Channels that have failed in some way */
	RsslQueue reconnectingChannels;

	fd_set *readFds, *writeFds, *exceptFds;
	fd_set *useReadFds, *useWriteFds, *useExceptFds;

	RsslInt64 lastRecordedTimeMs;

	RsslThreadId thread;
	RsslReactorEventQueue workerQueue;
	RsslUInt32 sleepTimeMs; /* Time to sleep when not flushing; should be equivalent to 1/3 of smallest ping timeout. */

	RsslErrorInfo workerCerr;
	RsslReactorEventQueueGroup activeEventQueueGroup;
} RsslReactorWorker;

typedef enum
{
	RSSL_REACTOR_ST_INIT = 0,
	RSSL_REACTOR_ST_ACTIVE = 1, /* Reactor is active */
	RSSL_REACTOR_ST_ERROR = 2, /* Reactor has encountered an error */
	RSSL_REACTOR_ST_SHUT_DOWN = 3 /* Reactor has shutdown */
} RsslReactorState;

/* RsslReactorImpl
 * The Reactor handles reading messages and commands from the application.
 * Primary responsiblities include:
 *   - Reading messages from transport
 *   - Calling back the application via callback functions, decoding messages to the RDM structs when appropriate.
 *   - Adding and removing channels in response to network events or requests from user.
 *   - Signaling the worker thread to flush when appropriate
 */
struct _RsslReactorImpl
{
	RsslReactor reactor;								/* Public-facing base reactor object */

	RsslReactorChannelImpl *channelPoolArray;
	RsslQueue channelPool;				/* Pool of available channel structures */
	RsslQueue initializingChannels;	/* Channels waiting for worker to initialize */
	RsslQueue activeChannels;			/* Channels that are active */
	RsslQueue inactiveChannels;			/* Channels that have failed in some way */
	RsslQueue closingChannels;			/* Waiting for close from worker */
	RsslQueue reconnectingChannels;		/* Channels that have been closed, but are currently reconnecting */

	RsslThreadId thread;

	RsslReactorEventQueue reactorEventQueue;
	unsigned int fdSetSize;
	unsigned int fdSetSizeInBytes;
	fd_set *readFds, *exceptFds;
	fd_set *useReadFds, *useExceptFds;
	RsslBuffer memoryBuffer;

	RsslInt64 lastRecordedTimeMs;

	RsslInt32 channelCount;			/* Total number of channels in use. */

	/* Used on each interface in the reactor to ensure thread-safety and that calling interfaces in callbacks is prevented. */
	RsslMutex interfaceLock; /* Ensures function calls are thread-safe */
	RsslBool inReactorFunction; /* Ensures functions are not called inside callbacks */

	RsslReactorEventQueueGroup activeEventQueueGroup;

	RsslReactorWorker reactorWorker;						/* The reactor's worker */
	RsslInt32 dispatchDecodeMemoryBufferSize;					/* The size to allocate for the temporary decoding block. This is used for decoding to the RDM structures. */
	RsslReactorState state;

	RsslInt64 ticksPerMsec;
};

RTR_C_INLINE void rsslClearReactorImpl(RsslReactorImpl *pReactorImpl)
{
	memset(pReactorImpl, 0, sizeof(RsslReactorImpl));
}

/* Setup and start the worker thread (Should be called from rsslCreateReactor) */
RsslRet _reactorWorkerStart(RsslReactorImpl *pReactorImpl, RsslCreateReactorOptions *pReactorOptions, RsslErrorInfo *pError);

/* Cleanup all reactor resources(it is assumed that there will be no more use of this reactor so all memory can be cleaned up */
void _reactorWorkerCleanupReactor(RsslReactorImpl *pReactorImpl);

/* Write reactor thread function */
RSSL_THREAD_DECLARE(runReactorWorker, pArg);

/* Estimate encoded message size. */
RsslUInt32 _reactorMsgEncodedSize(RsslMsg *pMsg);

#ifdef __cplusplus
};
#endif

#endif
