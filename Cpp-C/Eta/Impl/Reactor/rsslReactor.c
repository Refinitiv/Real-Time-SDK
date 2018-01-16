/*
 * This source code is provided under the Apache 2.0 license and is provided
 * AS IS with no warranty or guarantee of fit for purpose.  See the project's 
 * LICENSE.md for details. 
 * Copyright Thomson Reuters 2015. All rights reserved.
*/

#include "rtr/rsslReactorImpl.h"
#include "rtr/rsslReactorEventsImpl.h"
#include "rtr/rsslWatchlist.h"
#include "rtr/tunnelStreamImpl.h"
#include "rtr/msgQueueEncDec.h"

#include <assert.h>
#ifdef _WIN32
#include <windows.h>
#include <process.h>
#else
#include <sys/types.h>
#include <unistd.h>
#include <sys/resource.h>
#endif

/* Moves a RsslReactorChannelImpl from whatever list it's on to the given list and changes its state. */
static void _reactorMoveChannel(RsslQueue *pNewList, RsslReactorChannelImpl *pReactorChannel);

/* Removes a RsslReactorChannelImpl from whatever list its on and returns it */
static RsslReactorChannelImpl* _reactorTakeChannel(RsslReactorImpl *pReactorImpl, RsslQueue *pList);

/*** Reader helper functions ***/

/* Reads from the given channel and handles the message or return code. */
static RsslRet _reactorDispatchFromChannel(RsslReactorImpl *pReactorImpl, RsslReactorChannelImpl *pReactorChannel, RsslErrorInfo *pError);

/* Reads and handles an event from the given queue. */
static RsslRet _reactorDispatchEventFromQueue(RsslReactorImpl *pReactorImpl, RsslReactorEventQueue *pQueue, RsslErrorInfo *pError);

/* Sets whether we are in a callback call */
static void _reactorSetInCallback(RsslReactorImpl *pReactorImpl, RsslBool inCallback);

/* Encodes and sends RDM messages. */
static RsslRet _reactorSendRDMMessage(RsslReactorImpl *pReactorImpl, RsslReactorChannelImpl *pReactorChannel, RsslRDMMsg *pRDMMsg, RsslErrorInfo *pError);

/* Process tunnel manager return code from its read/dispatch/timer function */
static RsslRet _reactorHandleTunnelManagerRet(RsslReactorImpl *pReactorImpl, RsslReactorChannelImpl *pReactorChannel, RsslRet ret, RsslErrorInfo *pErrorInfo);

/* Handles failure of a channel by informing the user and starting the channel-closing process. */
static RsslRet _reactorHandleChannelDown(RsslReactorImpl *pReactorImpl, RsslReactorChannelImpl *pReactorChannel, RsslErrorInfo *pError);

/* Sends cleanup event to the worker */
static RsslRet _reactorCleanupReactor(RsslReactorImpl *pReactorImpl);

/* Requests that the worker begin flushing for the given channel. */
static RsslRet _reactorSendFlushRequest(RsslReactorImpl *pReactorImpl, RsslReactorChannelImpl *pReactorChannel, RsslErrorInfo *pError);

/* Adds channel info to reactor list and signals worker to initialize it. Used by both rsslReactorConnect & rsslReactorAccept */
static RsslRet _reactorAddChannel(RsslReactorImpl *pReactorImpl, RsslReactorChannelImpl *pReactorChannel, RsslErrorInfo *pError);

/* Creates a deep copy of the Reactor Channel Role structure. */
static RsslRet _reactorChannelCopyRole(RsslReactorChannelImpl *pReactorChannel, RsslReactorChannelRole *pRole, RsslErrorInfo *pError);

/* Sends an "RSSL_RC_CET_CHANNEL_READY" to the reactor's own queue */
static RsslRet _reactorSendConnReadyEvent(RsslReactorImpl *pReactorImpl, RsslReactorChannelImpl *pReactorChannel, RsslErrorInfo *pError);

/* Stops all channels in the reactor. */
static void _reactorShutdown(RsslReactorImpl *pReactorImpl, RsslErrorInfo *pError);

/* Options for _reactorProcessMsg. */
typedef struct
{
	RsslBuffer				*pMsgBuf;		/* (Input) Message buffer. */
	RsslMsg					*pRsslMsg;		/* (Input) RsslMsg struct. */
	RsslRDMMsg				*pRdmMsg;		/* (Input) RsslRDMMsg struct. */
	RsslStreamInfo			*pStreamInfo;	/* (Input) StreamInfo. */
	RsslUInt8				*pFTGroupId;	/* (Input) FTGroupId from rsslReadEx */
	RsslUInt32				*pSeqNum;		/* (Input) SeqNum from rsslReadEx. */
	RsslReactorCallbackRet	*pCret;			/* (Output) Return code from callback. */
	RsslErrorInfo			*pError;		/* (Output) Error. */
} ReactorProcessMsgOptions;

static RsslRet _reactorProcessMsg(RsslReactorImpl *pReactorImpl, RsslReactorChannelImpl *pReactorChannel, ReactorProcessMsgOptions *pOpts);

static RsslRet _reactorSubmitWatchlistMsg(RsslReactorImpl *pReactorImpl, RsslReactorChannelImpl *pReactorChannel, 
		RsslWatchlistProcessMsgOptions *pOptions, RsslErrorInfo *pError);

static RsslRet _reactorReadWatchlistMsg(RsslReactorImpl *pReactorImpl, RsslReactorChannelImpl *pReactorChannel,
		RsslWatchlistProcessMsgOptions *pOptions, RsslErrorInfo *pError);

#ifdef __cplusplus
extern "C" {
#endif

static RsslRet _reactorWatchlistMsgCallback(RsslWatchlist *pWatchlist, RsslWatchlistMsgEvent *pEvent, RsslErrorInfo *pError);

#ifdef __cplusplus
};
#endif

static RsslRet _reactorSendConnReadyEvent(RsslReactorImpl *pReactorImpl, RsslReactorChannelImpl *pReactorChannel, RsslErrorInfo *pError)
{
	RsslReactorChannelEventImpl *pEvent = (RsslReactorChannelEventImpl*)rsslReactorEventQueueGetFromPool(&pReactorChannel->eventQueue);

	pReactorChannel->channelSetupState = RSSL_RC_CHST_READY;
	rsslClearReactorChannelEventImpl(pEvent);
	pEvent->channelEvent.channelEventType = RSSL_RC_CET_CHANNEL_READY;
	pEvent->channelEvent.pReactorChannel = (RsslReactorChannel*)pReactorChannel;
	if (!RSSL_ERROR_INFO_CHECK(rsslReactorEventQueuePut(&pReactorChannel->eventQueue, (RsslReactorEventImpl*)pEvent) == RSSL_RET_SUCCESS, RSSL_RET_FAILURE, pError))
		return RSSL_RET_FAILURE;

	return RSSL_RET_SUCCESS;
}

static RsslRet _reactorSendShutdownEvent(RsslReactorImpl *pReactorImpl, RsslErrorInfo *pError);


/* Used to send internal messages. */
static RsslRet _reactorSubmit(RsslReactorImpl *pReactorImpl, RsslReactorChannelImpl *pReactorChannel, RsslBuffer *pBuf, RsslErrorInfo *pError)
{
	RsslRet ret;
	RsslReactorSubmitOptions submitOpts;

	rsslClearReactorSubmitOptions(&submitOpts);

	if ((ret = rsslReactorSubmit((RsslReactor*)pReactorImpl, (RsslReactorChannel*)pReactorChannel, pBuf, &submitOpts, pError)) < RSSL_RET_SUCCESS)
	{
		switch(ret)
		{
			case RSSL_RET_WRITE_CALL_AGAIN:
				if (pReactorChannel->pWriteCallAgainBuffer)
				{
					/* Reactor should only be attempting to send one of the preset messages at a time. */
					rsslSetErrorInfo(pError, RSSL_EIC_FAILURE, RSSL_RET_FAILURE, __FILE__, __LINE__, "Internal error: Setting pWriteCallAgainBuffer when it is already set.");
					return RSSL_RET_FAILURE;
				}
				pReactorChannel->pWriteCallAgainBuffer = pBuf;
				return RSSL_RET_SUCCESS;
			default:
				return _reactorHandleChannelDown(pReactorImpl, pReactorChannel, pError);
		}
	}

	return ret;
}


static void _reactorMoveChannel(RsslQueue *pNewList, RsslReactorChannelImpl *pReactorChannel)
{ 
	if (pReactorChannel->reactorParentQueue)
	{
		rsslQueueRemoveLink(pReactorChannel->reactorParentQueue, &pReactorChannel->reactorQueueLink);
		rsslInitQueueLink(&pReactorChannel->reactorQueueLink);
	}

	pReactorChannel->reactorParentQueue = pNewList; 

	if (pNewList)
		rsslQueueAddLinkToBack(pNewList, &pReactorChannel->reactorQueueLink);
}

static RsslReactorChannelImpl* _reactorTakeChannel(RsslReactorImpl *pReactorImpl, RsslQueue *pList)
{
	RsslQueueLink *pLink = rsslQueueRemoveFirstLink(pList);
	RsslReactorChannelImpl *pReactorChannel;

	if (!pLink)
	{
		pReactorChannel = (RsslReactorChannelImpl*)malloc(sizeof(RsslReactorChannelImpl));

		if (!pReactorChannel)
			return NULL;

		rsslClearReactorChannelImpl(pReactorImpl, pReactorChannel);
		rsslInitQueueLink(&pReactorChannel->reactorQueueLink);
		rsslInitReactorEventQueue(&pReactorChannel->eventQueue, 5, &pReactorImpl->activeEventQueueGroup);

		if ((pReactorChannel->pWorkerNotifierEvent = rsslCreateNotifierEvent()) == NULL)
			return NULL;

		if ((pReactorChannel->pNotifierEvent = rsslCreateNotifierEvent()) == NULL)
			return NULL;

	}
	else
		pReactorChannel = RSSL_QUEUE_LINK_TO_OBJECT(RsslReactorChannelImpl, reactorQueueLink, pLink);

	pReactorChannel->lastPingReadMs = pReactorImpl->lastRecordedTimeMs = getCurrentTimeMs(pReactorImpl->ticksPerMsec);

	pReactorChannel->reactorParentQueue = NULL;

	return pReactorChannel;
}

static RsslRet _reactorSendFlushRequest(RsslReactorImpl *pReactorImpl, RsslReactorChannelImpl *pReactorChannel, RsslErrorInfo *pError)
{
	if (!pReactorChannel->requestedFlush)
	{
		/* Signal worker to flush for this channel */
		RsslReactorEventImpl *pEvent = rsslReactorEventQueueGetFromPool(&pReactorImpl->reactorWorker.workerQueue);

		if (pReactorChannel->pWatchlist)
			pReactorChannel->pWatchlist->state &= ~RSSLWL_STF_NEED_FLUSH;

		pReactorChannel->requestedFlush = RSSL_TRUE;
		pReactorChannel->writeRet = 0; /* Set writeRet to 0. If it gets set again later we know we've called rsslWrite since the last flush request. */

		rsslInitFlushEvent(&pEvent->flushEvent);
		pEvent->flushEvent.pReactorChannel = (RsslReactorChannel*)pReactorChannel;
		pEvent->flushEvent.flushEventType = RSSL_RCIMPL_FET_START_FLUSH;
		if (!RSSL_ERROR_INFO_CHECK(rsslReactorEventQueuePut(&pReactorImpl->reactorWorker.workerQueue, pEvent) == RSSL_RET_SUCCESS, RSSL_RET_FAILURE, pError))
		{
			_reactorShutdown(pReactorImpl, pError);
			_reactorSendShutdownEvent(pReactorImpl, pError);
			return RSSL_RET_FAILURE;
		}
	}

	return RSSL_RET_SUCCESS;
}

static RsslRet _reactorSetTimer(RsslReactorImpl *pReactorImpl, RsslReactorChannelImpl *pReactorChannel, RsslInt64 expireTime, RsslErrorInfo *pError)
{
	/* Check if timeout unset, or new time is before the current one. */
	if (expireTime < pReactorChannel->lastRequestedExpireTime)
	{
		/* Signal worker to flush for this channel */
		RsslReactorEventImpl *pEvent = rsslReactorEventQueueGetFromPool(&pReactorImpl->reactorWorker.workerQueue);

		pReactorChannel->lastRequestedExpireTime = expireTime;

		if (pReactorChannel->pWatchlist)
			pReactorChannel->pWatchlist->state &= ~RSSLWL_STF_NEED_TIMER;

		rsslInitTimerEvent(&pEvent->timerEvent);
		pEvent->timerEvent.pReactorChannel = (RsslReactorChannel*)pReactorChannel;
		pEvent->timerEvent.expireTime = expireTime;
		if (!RSSL_ERROR_INFO_CHECK(rsslReactorEventQueuePut(&pReactorImpl->reactorWorker.workerQueue, pEvent) == RSSL_RET_SUCCESS, RSSL_RET_FAILURE, pError))
		{
			_reactorShutdown(pReactorImpl, pError);
			_reactorSendShutdownEvent(pReactorImpl, pError);
			return RSSL_RET_FAILURE;
		}
	}

	return RSSL_RET_SUCCESS;
}


static RsslRet _reactorSendShutdownEvent(RsslReactorImpl *pReactorImpl, RsslErrorInfo *pError)
{
	/* Signal worker to shutdown. */
	RsslReactorStateEvent *pEvent = (RsslReactorStateEvent*)rsslReactorEventQueueGetFromPool(&pReactorImpl->reactorWorker.workerQueue);

	rsslClearReactorEvent(pEvent);
	pEvent->reactorEventType = RSSL_RCIMPL_STET_SHUTDOWN;
	pEvent->pErrorInfo = NULL;
	
	if (!RSSL_ERROR_INFO_CHECK(rsslReactorEventQueuePut(&pReactorImpl->reactorWorker.workerQueue, (RsslReactorEventImpl*)pEvent) == RSSL_RET_SUCCESS, RSSL_RET_FAILURE, pError))
		return RSSL_RET_FAILURE;
	return RSSL_RET_SUCCESS;
}

RSSL_VA_API RsslReactor *rsslCreateReactor(RsslCreateReactorOptions *pReactorOpts, RsslErrorInfo *pError)
{
	RsslReactorImpl *pReactorImpl;
	RsslInt32 i;
	char *memBuf;

#ifdef WIN32
	LARGE_INTEGER	perfFrequency;
#else
	struct rlimit rlimit;
	long fdSetSizeRemainder;
#endif

	
	/* Call rsslInitialize to ensure that Rssl is initialized with global & channel locks. It is reference counted per call to rsslInitialize/rsslUninitialize. */
	if (rsslInitialize(RSSL_LOCK_GLOBAL_AND_CHANNEL, &pError->rsslError) != RSSL_RET_SUCCESS)
	{
		rsslSetErrorInfoLocation(pError, __FILE__, __LINE__);
		return NULL;
	}

	/* Create internal reactor object */
	if (!(pReactorImpl = (RsslReactorImpl*)malloc(sizeof(RsslReactorImpl))))
	{
		rsslSetErrorInfo(pError, RSSL_EIC_FAILURE, RSSL_RET_FAILURE, __FILE__, __LINE__, "Failed to create reactor object.");
		return NULL;
	}
	
	rsslClearReactorImpl(pReactorImpl);

	/* Copy options */
	pReactorImpl->dispatchDecodeMemoryBufferSize = pReactorOpts->dispatchDecodeMemoryBufferSize;
	pReactorImpl->reactor.userSpecPtr = pReactorOpts->userSpecPtr;

	pReactorImpl->state = RSSL_REACTOR_ST_ACTIVE;


	/* Setup reactor */
	if (rsslInitReactorEventQueue(&pReactorImpl->reactorEventQueue, 10, &pReactorImpl->activeEventQueueGroup) != RSSL_RET_SUCCESS)
	{
		rsslSetErrorInfo(pError, RSSL_EIC_FAILURE, RSSL_RET_FAILURE, __FILE__, __LINE__, "Failed to initialize event queue.");
		return NULL;
	}

	if (rsslInitReactorEventQueueGroup(&pReactorImpl->activeEventQueueGroup) != RSSL_RET_SUCCESS)
	{
		rsslSetErrorInfo(pError, RSSL_EIC_FAILURE, RSSL_RET_FAILURE, __FILE__, __LINE__, "Failed to initialize event queue group.");
		return NULL;
	}

#ifdef WIN32
	/* On Windows, time checks use the performance counter, so we need to obtain its frequency. */
	QueryPerformanceFrequency(&perfFrequency);
	pReactorImpl->ticksPerMsec = perfFrequency.QuadPart/1000;
#else
	pReactorImpl->ticksPerMsec = 1000000; /* Not used. */
#endif

	/* Setup reactor's channel lists */
	rsslInitQueue(&pReactorImpl->channelPool);
	rsslInitQueue(&pReactorImpl->initializingChannels);
	rsslInitQueue(&pReactorImpl->activeChannels);
	rsslInitQueue(&pReactorImpl->inactiveChannels);
	rsslInitQueue(&pReactorImpl->closingChannels);
	rsslInitQueue(&pReactorImpl->reconnectingChannels);


	RSSL_MUTEX_INIT(&pReactorImpl->interfaceLock);

	pReactorImpl->lastRecordedTimeMs = getCurrentTimeMs(pReactorImpl->ticksPerMsec);

	/* Initialize channel pool */
	for (i = 0; i < 10; ++i)
	{
		RsslReactorChannelImpl *pNewChannel = (RsslReactorChannelImpl*)malloc(sizeof(RsslReactorChannelImpl));

		if (!pNewChannel)
		{
			rsslSetErrorInfo(pError, RSSL_EIC_FAILURE, RSSL_RET_FAILURE, __FILE__, __LINE__, "Failed to create reactor channel pool.");
			return NULL;
		}
		rsslClearReactorChannelImpl(pReactorImpl, pNewChannel);
		rsslInitQueueLink(&pNewChannel->reactorQueueLink);
		rsslInitReactorEventQueue(&pNewChannel->eventQueue, 5, &pReactorImpl->activeEventQueueGroup);

		if ((pNewChannel->pNotifierEvent = rsslCreateNotifierEvent()) == NULL)
			return NULL;

		if ((pNewChannel->pWorkerNotifierEvent = rsslCreateNotifierEvent()) == NULL)
			return NULL;

		_reactorMoveChannel(&pReactorImpl->channelPool, pNewChannel);
	}

	if ((pReactorImpl->pNotifier = rsslCreateNotifier(1024)) == NULL)
	{
		_reactorWorkerCleanupReactor(pReactorImpl);
		rsslSetErrorInfo(pError, RSSL_EIC_FAILURE, RSSL_RET_FAILURE, __FILE__, __LINE__, "Failed to create reactor notifier.");
		return NULL;
	}

	if ((pReactorImpl->pQueueNotifierEvent = rsslCreateNotifierEvent()) == NULL)
	{
		_reactorWorkerCleanupReactor(pReactorImpl);
		rsslSetErrorInfo(pError, RSSL_EIC_FAILURE, RSSL_RET_FAILURE, __FILE__, __LINE__, "Failed to create reactor event queue notifier event.");
		return NULL;
	}


	pReactorImpl->reactor.eventFd = rsslGetEventQueueGroupSignalFD(&pReactorImpl->activeEventQueueGroup);

	if (rsslNotifierAddEvent(pReactorImpl->pNotifier, pReactorImpl->pQueueNotifierEvent, pReactorImpl->reactor.eventFd, &pReactorImpl->reactorEventQueue) < 0)
	{
		_reactorWorkerCleanupReactor(pReactorImpl);
		rsslSetErrorInfo(pError, RSSL_EIC_FAILURE, RSSL_RET_FAILURE, __FILE__, __LINE__, "Failed to add event for reactor event queue notfiication.");
		return NULL;
	}

	if (rsslNotifierRegisterRead(pReactorImpl->pNotifier, pReactorImpl->pQueueNotifierEvent) < 0)
	{
		rsslSetErrorInfo(pError, RSSL_EIC_FAILURE, RSSL_RET_FAILURE, __FILE__, __LINE__, 
				"Failed to register read notification for reactor event queue.");
		return NULL;
	}

	/* Initialize memory block for decoding RDM structures */
	memBuf = (char*)malloc(pReactorImpl->dispatchDecodeMemoryBufferSize);
	if (!memBuf)
	{
		_reactorWorkerCleanupReactor(pReactorImpl);
		rsslSetErrorInfo(pError, RSSL_EIC_FAILURE, RSSL_RET_FAILURE, __FILE__, __LINE__, "Failed to create reactor memory buffer.");
		return NULL;
	}
	pReactorImpl->memoryBuffer.data = memBuf;
	pReactorImpl->memoryBuffer.length = pReactorImpl->dispatchDecodeMemoryBufferSize;

	if (_reactorWorkerStart(pReactorImpl, pReactorOpts, pError) != RSSL_RET_SUCCESS)
	{
		_reactorWorkerCleanupReactor(pReactorImpl);
		return NULL;
	}

	return (RsslReactor*)pReactorImpl;
}

static void _reactorShutdown(RsslReactorImpl *pReactorImpl, RsslErrorInfo *pError)
{
	switch(pReactorImpl->state)
	{
		case RSSL_REACTOR_ST_ACTIVE:
		case RSSL_REACTOR_ST_ERROR:
		{
			RsslReactorChannelImpl *pReactorChannel;
			RsslQueueLink *pLink;
	
			pReactorImpl->state = RSSL_REACTOR_ST_SHUT_DOWN;

			/* Call channel down callback for each channel */
			RSSL_QUEUE_FOR_EACH_LINK(&pReactorImpl->activeChannels, pLink)
			{
				pReactorChannel = RSSL_QUEUE_LINK_TO_OBJECT(RsslReactorChannelImpl, reactorQueueLink, pLink);
				if (_reactorHandleChannelDown(pReactorImpl, pReactorChannel, pError) != RSSL_RET_SUCCESS)
					return;
			}

			/* Any channels still active should be cleaned up by the worker */

			return;
		}
		case RSSL_REACTOR_ST_SHUT_DOWN:
		default:
			return;
	}
}

static RsslRet _reactorCleanupReactor(RsslReactorImpl *pReactorImpl)
{
	RsslReactorStateEvent *pEvent;
	RsslThreadId thread = pReactorImpl->reactorWorker.thread;

	/* Tell worker thread to cleanup all memory. */

	pEvent = (RsslReactorStateEvent*)rsslReactorEventQueueGetFromPool(&pReactorImpl->reactorWorker.workerQueue);
	rsslClearReactorEvent(pEvent);
	pEvent->reactorEventType = RSSL_RCIMPL_STET_DESTROY;
	rsslReactorEventQueuePut(&pReactorImpl->reactorWorker.workerQueue, (RsslReactorEventImpl*)pEvent);
	RSSL_THREAD_JOIN(thread);
	return RSSL_RET_SUCCESS;
}

RSSL_VA_API RsslRet rsslDestroyReactor(RsslReactor *pReactor, RsslErrorInfo *pError)
{
	RsslReactorImpl *pReactorImpl = (RsslReactorImpl*)pReactor;

	switch(pReactorImpl->state)
	{
		case RSSL_REACTOR_ST_ACTIVE:

			/* Any channels that are still active should have their callbacks called so they can do any cleanup normally done. */
			rsslSetErrorInfo(pError, RSSL_EIC_SUCCESS, RSSL_RET_SUCCESS, __FILE__, __LINE__, "Reactor is shutting down.");
			_reactorShutdown(pReactorImpl, pError);

			_reactorSendShutdownEvent(pReactorImpl, pError);
			/* Fall through */
		case RSSL_REACTOR_ST_SHUT_DOWN:
			_reactorCleanupReactor(pReactorImpl);
			return RSSL_RET_SUCCESS;
		default:
			rsslSetErrorInfo(pError, RSSL_EIC_SUCCESS, RSSL_RET_SUCCESS, __FILE__, __LINE__, "Reactor has unknown state.");
			return RSSL_RET_INVALID_ARGUMENT;
	}
}

static RsslRet _reactorAddChannel(RsslReactorImpl *pReactorImpl, RsslReactorChannelImpl *pReactorChannel, RsslErrorInfo *pError)
{
	RsslReactorChannelEventImpl *pEvent = (RsslReactorChannelEventImpl*)rsslReactorEventQueueGetFromPool(&pReactorImpl->reactorWorker.workerQueue);

	_reactorMoveChannel(&pReactorImpl->initializingChannels, pReactorChannel);

	pReactorChannel->channelSetupState = RSSL_RC_CHST_INIT;

	/* Signal worker about new channel(the worker will initialize it if it's not already active) */
	rsslClearReactorChannelEventImpl(pEvent);
	pEvent->channelEvent.channelEventType = (RsslReactorChannelEventType)RSSL_RCIMPL_CET_NEW_CHANNEL;
	pEvent->channelEvent.pReactorChannel = (RsslReactorChannel*)pReactorChannel;
	if (!RSSL_ERROR_INFO_CHECK(rsslReactorEventQueuePut(&pReactorImpl->reactorWorker.workerQueue, (RsslReactorEventImpl*)pEvent) == RSSL_RET_SUCCESS, RSSL_RET_FAILURE, pError))
		return RSSL_RET_FAILURE;
	return RSSL_RET_SUCCESS;
}

static RsslRet _validateRole(RsslReactorChannelRole *pRole, RsslErrorInfo *pError)
{
	/* Ensure valid callback structure is used. */

	/* Role object must be provided on every channel, with a provided default msg & channel event callback */
	if (!pRole)
	{
		rsslSetErrorInfo(pError, RSSL_EIC_FAILURE, RSSL_RET_INVALID_ARGUMENT, __FILE__, __LINE__, "RsslReactorChannelRole not provided.");
		return RSSL_RET_INVALID_ARGUMENT;
	}

	if (!pRole->base.defaultMsgCallback)
	{
		rsslSetErrorInfo(pError, RSSL_EIC_FAILURE, RSSL_RET_INVALID_ARGUMENT, __FILE__, __LINE__, "defaultMsgCallback not provided.");
		return RSSL_RET_INVALID_ARGUMENT;
	}

	if (!pRole->base.channelEventCallback)
	{
		rsslSetErrorInfo(pError, RSSL_EIC_FAILURE, RSSL_RET_INVALID_ARGUMENT, __FILE__, __LINE__, "channelEventCallback not provided.");
		return RSSL_RET_INVALID_ARGUMENT;
	}
	

	switch(pRole->base.roleType)
	{
		case RSSL_RC_RT_OMM_CONSUMER:
		{
			RsslReactorOMMConsumerRole *pConsumerRole = &pRole->ommConsumerRole; 

			/* Auto dictionary download not supported when watchlist is enabled. */
			if (pConsumerRole->dictionaryDownloadMode != RSSL_RC_DICTIONARY_DOWNLOAD_NONE && pConsumerRole->watchlistOptions.enableWatchlist)
			{
				rsslSetErrorInfo(pError, RSSL_EIC_FAILURE, RSSL_RET_INVALID_ARGUMENT, __FILE__, __LINE__, "Dictionary download not supported when watchlist is enabled.");
				return RSSL_RET_INVALID_ARGUMENT;
			}

			/* Make sure login request type appears correct */
			if (pConsumerRole->pLoginRequest 
					&& (pConsumerRole->pLoginRequest->rdmMsgBase.domainType != RSSL_DMT_LOGIN || pConsumerRole->pLoginRequest->rdmMsgBase.rdmMsgType != RDM_LG_MT_REQUEST))
			{
				rsslSetErrorInfo(pError, RSSL_EIC_FAILURE, RSSL_RET_INVALID_ARGUMENT, __FILE__, __LINE__, "Login Request has wrong type.");
				return RSSL_RET_INVALID_ARGUMENT;
			}
			
			/* Must provide login request if requesting directory */
			if (pConsumerRole->pDirectoryRequest && !pConsumerRole->pLoginRequest)
			{
				rsslSetErrorInfo(pError, RSSL_EIC_FAILURE, RSSL_RET_INVALID_ARGUMENT, __FILE__, __LINE__, "Directory request provided without providing login request.");
				return RSSL_RET_INVALID_ARGUMENT;
			}

			/* Make sure directory request type appears correct */
			if (pConsumerRole->pDirectoryRequest 
					&& (pConsumerRole->pDirectoryRequest->rdmMsgBase.domainType != RSSL_DMT_SOURCE || pConsumerRole->pDirectoryRequest->rdmMsgBase.rdmMsgType != RDM_DR_MT_REQUEST))
			{
				rsslSetErrorInfo(pError, RSSL_EIC_FAILURE, RSSL_RET_INVALID_ARGUMENT, __FILE__, __LINE__, "Directory Request has wrong type.");
				return RSSL_RET_INVALID_ARGUMENT;
			}

			/* Must provide directory request if requesting download of dictionaries */
			if (pConsumerRole->dictionaryDownloadMode != RSSL_RC_DICTIONARY_DOWNLOAD_NONE && !pConsumerRole->pDirectoryRequest)
			{
				rsslSetErrorInfo(pError, RSSL_EIC_FAILURE, RSSL_RET_INVALID_ARGUMENT, __FILE__, __LINE__, "Dictionary download requested without providing directory request.");
				return RSSL_RET_INVALID_ARGUMENT;
			}

			return RSSL_RET_SUCCESS;
		}
		case RSSL_RC_RT_OMM_PROVIDER:
		{
			/* Nothing more to validate, since provider does not use setup messages. */
			return RSSL_RET_SUCCESS;
		}
		case RSSL_RC_RT_OMM_NI_PROVIDER:
		{
			RsslReactorOMMNIProviderRole *pNIProviderRole = &pRole->ommNIProviderRole;

			/* Make sure login request type appears correct */
			if (pNIProviderRole->pLoginRequest 
					&& (pNIProviderRole->pLoginRequest->rdmMsgBase.domainType != RSSL_DMT_LOGIN || pNIProviderRole->pLoginRequest->rdmMsgBase.rdmMsgType != RDM_LG_MT_REQUEST))
			{
				rsslSetErrorInfo(pError, RSSL_EIC_FAILURE, RSSL_RET_INVALID_ARGUMENT, __FILE__, __LINE__, "Login Request has wrong type.");
				return RSSL_RET_INVALID_ARGUMENT;
			}

			/* Must provide login request if sending directory refresh. */
			if (pNIProviderRole->pDirectoryRefresh && !pNIProviderRole->pLoginRequest)
			{
				rsslSetErrorInfo(pError, RSSL_EIC_FAILURE, RSSL_RET_INVALID_ARGUMENT, __FILE__, __LINE__, "Directory refresh provided without providing login request.");
				return RSSL_RET_INVALID_ARGUMENT;
			}

			/* Make sure directory refresh type appears correct */
			if (pNIProviderRole->pDirectoryRefresh 
					&& (pNIProviderRole->pDirectoryRefresh->rdmMsgBase.domainType != RSSL_DMT_SOURCE || pNIProviderRole->pDirectoryRefresh->rdmMsgBase.rdmMsgType != RDM_DR_MT_REFRESH))
			{
				rsslSetErrorInfo(pError, RSSL_EIC_FAILURE, RSSL_RET_INVALID_ARGUMENT, __FILE__, __LINE__, "Directory Refresh provided has wrong type.");
				return RSSL_RET_INVALID_ARGUMENT;
			}
			return RSSL_RET_SUCCESS;
		}
		default:
		{
			rsslSetErrorInfo(pError, RSSL_EIC_FAILURE, RSSL_RET_INVALID_ARGUMENT, __FILE__, __LINE__, "Invalid callback roleType %d.", pRole->base.roleType);
			return RSSL_RET_INVALID_ARGUMENT;
		}
	}

}

RSSL_VA_API RsslRet rsslReactorConnect(RsslReactor *pReactor, RsslReactorConnectOptions *pOpts, RsslReactorChannelRole *pRole, RsslErrorInfo *pError )
{
	RsslReactorImpl *pReactorImpl = (RsslReactorImpl*)pReactor;

	RsslWatchlist *pWatchlist = NULL;
	RsslChannel *pChannel;
	RsslReactorChannelImpl *pReactorChannel = NULL;
	RsslRet ret;
	RsslConnectOptions *pConnOptions;

	if ((ret = reactorLockInterface(pReactorImpl, RSSL_TRUE, pError)) != RSSL_RET_SUCCESS)
		return ret;

	if (pReactorImpl->state != RSSL_REACTOR_ST_ACTIVE)
	{
		rsslSetErrorInfo(pError, RSSL_EIC_FAILURE, RSSL_RET_INVALID_ARGUMENT, __FILE__, __LINE__, "Reactor is shutting down.");
		goto reactorConnectFail;
	}

	if ((ret = _validateRole(pRole, pError)) != RSSL_RET_SUCCESS)
		goto reactorConnectFail;

	pReactorChannel = _reactorTakeChannel(pReactorImpl, &pReactorImpl->channelPool);

	if (!pReactorChannel)
	{
		rsslSetErrorInfo(pError, RSSL_EIC_FAILURE, RSSL_RET_FAILURE, __FILE__, __LINE__, "Failed to create channel object.");
		goto reactorConnectFail;
	}

	rsslResetReactorChannel(pReactorImpl, pReactorChannel);


		if(_rsslChannelCopyConnectionList(pReactorChannel, pOpts) != RSSL_RET_SUCCESS)
		{
			_reactorMoveChannel(&pReactorImpl->channelPool, pReactorChannel);

			rsslSetErrorInfo(pError, RSSL_EIC_FAILURE, RSSL_RET_FAILURE, __FILE__, __LINE__, "Unable to copy connection list.");
			return (reactorUnlockInterface(pReactorImpl), RSSL_RET_FAILURE);
		}

		pConnOptions = &pReactorChannel->connectionOptList[0].rsslConnectOptions;
		pReactorChannel->connectionListIter = 0;
		pReactorChannel->initializationTimeout = pReactorChannel->connectionOptList[0].initializationTimeout;

	if (pRole->base.roleType == RSSL_RC_RT_OMM_CONSUMER
		   && pRole->ommConsumerRole.watchlistOptions.enableWatchlist)
	{
		RsslWatchlistCreateOptions watchlistCreateOpts;
		rsslWatchlistClearCreateOptions(&watchlistCreateOpts);
		watchlistCreateOpts.msgCallback = _reactorWatchlistMsgCallback;
		watchlistCreateOpts.itemCountHint = pRole->ommConsumerRole.watchlistOptions.itemCountHint;
		watchlistCreateOpts.obeyOpenWindow = pRole->ommConsumerRole.watchlistOptions.obeyOpenWindow;
		watchlistCreateOpts.maxOutstandingPosts = pRole->ommConsumerRole.watchlistOptions.maxOutstandingPosts;
		watchlistCreateOpts.postAckTimeout = pRole->ommConsumerRole.watchlistOptions.postAckTimeout;
		watchlistCreateOpts.requestTimeout = pRole->ommConsumerRole.watchlistOptions.requestTimeout;
		watchlistCreateOpts.ticksPerMsec = pReactorImpl->ticksPerMsec;
		pWatchlist = rsslWatchlistCreate(&watchlistCreateOpts, pError);
		if (!pWatchlist) goto reactorConnectFail;
	}
	else
		pWatchlist = NULL;

	if ((pReactorChannel->pTunnelManager = tunnelManagerOpen(pReactor, (RsslReactorChannel*)pReactorChannel, pError)) == NULL)
		goto reactorConnectFail;

	if (_reactorChannelCopyRole(pReactorChannel, pRole, pError) != RSSL_RET_SUCCESS)
		goto reactorConnectFail;

	pReactorChannel->reactorChannel.pRsslChannel = NULL;
	pReactorChannel->reactorChannel.pRsslServer = NULL;
	pReactorChannel->reactorChannel.userSpecPtr = pOpts->rsslConnectOptions.userSpecPtr;
	pReactorChannel->initializationTimeout = pOpts->initializationTimeout;
	pReactorChannel->pWatchlist = pWatchlist;
	pReactorChannel->readRet = 0;

	/* Set reconnection info here, this should be zeroed out provider bound connections */
	pReactorChannel->reconnectAttemptLimit = pOpts->reconnectAttemptLimit;

	if(pOpts->reconnectMinDelay < 100)
	{
		pReactorChannel->reconnectMinDelay = 100;
	}
	else
	{
	pReactorChannel->reconnectMinDelay = pOpts->reconnectMinDelay;
	}

	if(pOpts->reconnectMinDelay > pOpts->reconnectMaxDelay)
	{
		pReactorChannel->reconnectMaxDelay = pReactorChannel->reconnectMinDelay;
	}
	else
	{
		pReactorChannel->reconnectMaxDelay = pOpts->reconnectMaxDelay;
	}

	pReactorChannel->reconnectAttemptCount = 0;
	pReactorChannel->lastReconnectAttemptMs = 0;

	if (pWatchlist)
	{
		RsslReactorEventImpl rsslEvent;
		RsslReactorCallbackRet cret;
		RsslWatchlistProcessMsgOptions processOpts;

		pWatchlist->pUserSpec = pReactorChannel;

		rsslClearReactorChannelEventImpl(&rsslEvent.channelEventImpl);
		rsslEvent.channelEventImpl.channelEvent.channelEventType = RSSL_RC_CET_CHANNEL_OPENED;
		rsslEvent.channelEventImpl.channelEvent.pReactorChannel = (RsslReactorChannel*)pReactorChannel;
		rsslEvent.channelEventImpl.channelEvent.pError = NULL;

		/* Add consumer requests, if provided. */
		if (pReactorChannel->channelRole.ommConsumerRole.pLoginRequest)
		{

			rsslWatchlistClearProcessMsgOptions(&processOpts);
			processOpts.pChannel = pReactorChannel->reactorChannel.pRsslChannel;
			processOpts.pRdmMsg = (RsslRDMMsg*)pReactorChannel->channelRole.ommConsumerRole.pLoginRequest;

			if (ret = _reactorSubmitWatchlistMsg(pReactorImpl, pReactorChannel, &processOpts, pError) 
					< RSSL_RET_SUCCESS)
				return (reactorUnlockInterface(pReactorImpl), ret);

		}

		if (pReactorChannel->channelRole.ommConsumerRole.pDirectoryRequest)
		{
			rsslWatchlistClearProcessMsgOptions(&processOpts);
			processOpts.pChannel = pReactorChannel->reactorChannel.pRsslChannel;
			processOpts.pRdmMsg = (RsslRDMMsg*)pReactorChannel->channelRole.ommConsumerRole.pDirectoryRequest;

			if (ret = _reactorSubmitWatchlistMsg(pReactorImpl, pReactorChannel, &processOpts, pError) 
					< RSSL_RET_SUCCESS)
				return (reactorUnlockInterface(pReactorImpl), ret);
		}


		_reactorSetInCallback(pReactorImpl, RSSL_TRUE);
		if (pReactorChannel->channelRole.ommConsumerRole.watchlistOptions.channelOpenCallback)
			cret = (*pReactorChannel->channelRole.ommConsumerRole.watchlistOptions.channelOpenCallback)((RsslReactor*)pReactorImpl, (RsslReactorChannel*)pReactorChannel, &rsslEvent.channelEventImpl.channelEvent);
		else
			cret = RSSL_RC_CRET_SUCCESS;
		_reactorSetInCallback(pReactorImpl, RSSL_FALSE);

		if (cret < RSSL_RC_CRET_SUCCESS || pReactorChannel->reactorParentQueue == &pReactorImpl->closingChannels)
			goto reactorConnectFail;

	}

	pReactorChannel->reactorChannel.userSpecPtr = pConnOptions->userSpecPtr;

	if (!(pChannel = rsslConnect(pConnOptions, &pError->rsslError)))
	{

		if(pOpts->reconnectAttemptLimit != 0)
		{
			if(_reactorHandleChannelDown(pReactorImpl, pReactorChannel, pError) != RSSL_RET_SUCCESS)
			{
				_reactorShutdown(pReactorImpl, pError);
				_reactorSendShutdownEvent(pReactorImpl, pError);
				return (reactorUnlockInterface(pReactorImpl), RSSL_RET_FAILURE);
			}
		}
		else
			goto reactorConnectFail;

	}
	else
	{
		pReactorChannel->reactorChannel.pRsslChannel = pChannel;

		if (!RSSL_ERROR_INFO_CHECK((ret = _reactorAddChannel(pReactorImpl, pReactorChannel, pError)) == RSSL_RET_SUCCESS, ret, pError))
		{
			_reactorShutdown(pReactorImpl, pError);
			_reactorSendShutdownEvent(pReactorImpl, pError);
			goto reactorConnectFail;
		}
	}

	++pReactorImpl->channelCount;



	return (reactorUnlockInterface(pReactorImpl), RSSL_RET_SUCCESS);

reactorConnectFail:
	if (pWatchlist)
		rsslWatchlistDestroy(pWatchlist);
	if (pReactorChannel)
	{
		if (pReactorChannel->reactorChannel.pRsslChannel)
		{
			RsslError rsslError;
			rsslCloseChannel(pReactorChannel->reactorChannel.pRsslChannel, &rsslError);
		}
		_reactorMoveChannel(&pReactorImpl->channelPool, pReactorChannel);
		--pReactorImpl->channelCount;
	}
		
	reactorUnlockInterface(pReactorImpl);
	return pError->rsslError.rsslErrorId;
}

RSSL_VA_API RsslRet rsslReactorAccept(RsslReactor *pReactor, RsslServer *pServer, RsslReactorAcceptOptions *pOpts, RsslReactorChannelRole *pRole, RsslErrorInfo *pError )
{
	RsslReactorImpl *pReactorImpl = (RsslReactorImpl*)pReactor;

	RsslChannel *pChannel;
	RsslReactorChannelImpl *pReactorChannel;
	RsslRet ret;

	if ((ret = reactorLockInterface(pReactorImpl, RSSL_TRUE, pError)) != RSSL_RET_SUCCESS)
		return ret;

	if (pReactorImpl->state != RSSL_REACTOR_ST_ACTIVE)
	{
		rsslSetErrorInfo(pError, RSSL_EIC_FAILURE, RSSL_RET_INVALID_ARGUMENT, __FILE__, __LINE__, "Reactor is shutting down.");
		return (reactorUnlockInterface((RsslReactorImpl*)pReactor), RSSL_RET_FAILURE);
	}

	if (!pServer)
	{
		rsslSetErrorInfo(pError, RSSL_EIC_FAILURE, RSSL_RET_INVALID_ARGUMENT, __FILE__, __LINE__, "Server not provided.");
		return (reactorUnlockInterface((RsslReactorImpl*)pReactor), RSSL_RET_INVALID_ARGUMENT);
	}

	if (_validateRole(pRole, pError) != RSSL_RET_SUCCESS)
		return (reactorUnlockInterface((RsslReactorImpl*)pReactor), RSSL_RET_INVALID_ARGUMENT);

	if (!(pChannel = rsslAccept(pServer, &pOpts->rsslAcceptOptions, &pError->rsslError)))
	{
		rsslSetErrorInfoLocation(pError, __FILE__, __LINE__);
		return (reactorUnlockInterface(pReactorImpl), RSSL_RET_FAILURE);
	}

	pReactorChannel = _reactorTakeChannel(pReactorImpl, &pReactorImpl->channelPool);

	if (!pReactorChannel)
	{
		rsslSetErrorInfo(pError, RSSL_EIC_FAILURE, RSSL_RET_FAILURE, __FILE__, __LINE__, "Failed to create channel object.");
		return (reactorUnlockInterface(pReactorImpl), RSSL_RET_FAILURE);
	}
	
	rsslResetReactorChannel(pReactorImpl, pReactorChannel);

	if (_reactorChannelCopyRole(pReactorChannel, pRole, pError) != RSSL_RET_SUCCESS)
	{
		_reactorMoveChannel(&pReactorImpl->channelPool, pReactorChannel);
		return (reactorUnlockInterface(pReactorImpl), RSSL_RET_FAILURE);
	}

	pReactorChannel->reactorChannel.pRsslChannel = pChannel;
	pReactorChannel->reactorChannel.pRsslServer = pServer;
	pReactorChannel->reactorChannel.userSpecPtr = pOpts->rsslAcceptOptions.userSpecPtr;
	pReactorChannel->initializationTimeout = pOpts->initializationTimeout;

	if ((pReactorChannel->pTunnelManager = tunnelManagerOpen(pReactor, (RsslReactorChannel*)pReactorChannel, pError)) == NULL)
		return (reactorUnlockInterface(pReactorImpl), RSSL_RET_FAILURE);

	if (!RSSL_ERROR_INFO_CHECK((ret = _reactorAddChannel(pReactorImpl, pReactorChannel, pError)) == RSSL_RET_SUCCESS, ret, pError))
	{
		_reactorShutdown(pReactorImpl, pError);
		_reactorSendShutdownEvent(pReactorImpl, pError);
		return (reactorUnlockInterface(pReactorImpl), RSSL_RET_FAILURE);
	}

	++pReactorImpl->channelCount;

	return (reactorUnlockInterface(pReactorImpl), RSSL_RET_SUCCESS);
}

RSSL_VA_API RsslRet rsslReactorDispatch(RsslReactor *pReactor, RsslReactorDispatchOptions *pDispatchOpts, RsslErrorInfo *pError)
{
	RsslReactorImpl *pReactorImpl = (RsslReactorImpl*)pReactor;
	RsslRet ret;
	RsslUInt32  channelsToCheck, channelsWithData;
	RsslUInt32 maxMsgs = pDispatchOpts->maxMessages;

	if ((ret = reactorLockInterface(pReactorImpl, RSSL_FALSE, pError)) != RSSL_RET_SUCCESS)
		return ret;

	/* Record current time. */
	pReactorImpl->lastRecordedTimeMs = getCurrentTimeMs(pReactorImpl->ticksPerMsec);

	/* See which channels have something to read. */
	if ((ret = rsslNotifierWait(pReactorImpl->pNotifier, 0)) < 0)
	{
#ifdef WIN32
		int notifierErrno = WSAGetLastError();
		if (notifierErrno == WSAEINTR)
			return (reactorUnlockInterface(pReactorImpl), RSSL_RET_SUCCESS);
#else
		int notifierErrno = errno;
		if (notifierErrno == EINTR)
			return (reactorUnlockInterface(pReactorImpl), RSSL_RET_SUCCESS);
#endif
		rsslSetErrorInfo(pError, RSSL_EIC_FAILURE, RSSL_RET_FAILURE, __FILE__, __LINE__, "rsslNotifierWait() failed: %d", notifierErrno);
		_reactorShutdown(pReactorImpl, pError);
		_reactorSendShutdownEvent(pReactorImpl, pError);
		return (reactorUnlockInterface(pReactorImpl), RSSL_RET_FAILURE);
	}

	if (pReactorImpl->state == RSSL_REACTOR_ST_ACTIVE)
	{
		/* No particular channel was specified. Loop in round-robin fashion on all channels until either:
		 * - The desired number of messages has been dispatched
		 * - There is nothing more to read */
		if (!pDispatchOpts->pReactorChannel)
		{
			RsslReactorChannelImpl *pReactorChannel;

			if (rsslNotifierEventIsReadable(pReactorImpl->pQueueNotifierEvent))
			{
				RsslReactorEventQueue *pQueue;

				/* Dispatch events from queues in a round-robin fashion until all are processed. */
				while (maxMsgs > 0 && (pQueue = rsslReactorEventQueueGroupShift(&pReactorImpl->activeEventQueueGroup)))
				{
					if ((ret = _reactorDispatchEventFromQueue(pReactorImpl, pQueue, pError)) < RSSL_RET_SUCCESS)
					{
						_reactorShutdown(pReactorImpl, pError);
						_reactorSendShutdownEvent(pReactorImpl, pError);
						return (reactorUnlockInterface(pReactorImpl), ret);
					}
					if (maxMsgs > 0) --maxMsgs;
				}
			}

			channelsWithData = channelsToCheck = rsslQueueGetElementCount(&pReactorImpl->activeChannels);

			while(maxMsgs > 0 && channelsWithData > 0)
			{
				RsslQueueLink *pLink;
				RsslBool isFdReadable;

				pLink = rsslQueueRemoveFirstLink(&pReactorImpl->activeChannels);
				rsslQueueAddLinkToBack(&pReactorImpl->activeChannels, pLink);

				pReactorChannel = RSSL_QUEUE_LINK_TO_OBJECT(RsslReactorChannelImpl, reactorQueueLink, pLink);

				isFdReadable = rsslNotifierEventIsReadable(pReactorChannel->pNotifierEvent);

				/* A channel has something to read if either:
				 * - The last return from rsslRead() was greater than zero, indicating there were still bytes in RSSL's queue
				 * - The file descriptor is set because there is data from the socket */
				if (pReactorChannel->readRet > 0 || isFdReadable)
				{
					if ((ret = _reactorDispatchFromChannel(pReactorImpl, pReactorChannel, pError)) < RSSL_RET_SUCCESS)
					{
						/* _reactorDispatchFromChannel from channel will disconnect the channel if it needs to. If if fails,
						 * then it had a problem doing that and there's nothing more we can do to handle it. */
						_reactorShutdown(pReactorImpl, pError);
						_reactorSendShutdownEvent(pReactorImpl, pError);
						return (reactorUnlockInterface(pReactorImpl), ret);
					}
					else if (pReactorChannel->reactorParentQueue != &pReactorImpl->activeChannels)
					{
						/* Channel is no longer active */
						--channelsWithData;
					}
					else if (pReactorChannel->readRet <= RSSL_RET_SUCCESS)
					{
						--channelsWithData;
						if (isFdReadable)
							rsslNotifierEventClearNotifiedFlags(pReactorChannel->pNotifierEvent);
					}
					if (channelsToCheck > 0) --channelsToCheck;
					if (maxMsgs > 0) --maxMsgs;
				}
				/* If not triggered to read, check if this channel has passed its ping timeout without sending either a ping or some data. */
				else 
				{
					if ((pReactorImpl->lastRecordedTimeMs - pReactorChannel->lastPingReadMs) > pReactorChannel->reactorChannel.pRsslChannel->pingTimeout * 1000)
					{
						rsslSetErrorInfo(pError, RSSL_EIC_FAILURE, RSSL_RET_FAILURE, __FILE__, __LINE__, "Channel ping timeout expired.");
						if (_reactorHandleChannelDown(pReactorImpl, pReactorChannel, pError) != RSSL_RET_SUCCESS)
						{
							_reactorShutdown(pReactorImpl, pError);
							_reactorSendShutdownEvent(pReactorImpl, pError);
							return (reactorUnlockInterface(pReactorImpl), RSSL_RET_FAILURE);
						}
					}
					if (channelsToCheck > 0) --channelsToCheck;
					--channelsWithData;
				}
			}

			if (channelsToCheck < channelsWithData)
			{
				/* Some channels had more data to read, so return positive value. */
				return (reactorUnlockInterface(pReactorImpl), 1);
			}
			else
			{
				/* We may have exhausted maxMessages before all channels were
				 * checked for data. If any of them still have data
				 * to read make sure we return a positive value. */
				while (channelsToCheck > 0)
				{
					RsslQueueLink *pLink;
					pLink = rsslQueueRemoveFirstLink(&pReactorImpl->activeChannels);
					rsslQueueAddLinkToBack(&pReactorImpl->activeChannels, pLink);

					pReactorChannel = RSSL_QUEUE_LINK_TO_OBJECT(RsslReactorChannelImpl, reactorQueueLink, pLink);

					/* A channel has something to read if either:
					 * - The last return from rsslRead() was greater than zero, indicating there were still bytes in RSSL's queue
					 * - The file descriptor is set because there is data from the socket */
					if (pReactorChannel->readRet > 0 || rsslNotifierEventIsReadable(pReactorChannel->pNotifierEvent))
						return (reactorUnlockInterface(pReactorImpl), 1);

					--channelsToCheck;
				}
				return (reactorUnlockInterface(pReactorImpl), 0);
			}
		}
		else
		{
			/* Specific channel specified. Dispatch only from that channel. */

			RsslReactorChannelImpl *pReactorChannel = (RsslReactorChannelImpl*)pDispatchOpts->pReactorChannel;

			/* Since the application passed in this channel, make sure it is valid for this reactor and that it is active. */
			if (!rsslReactorChannelIsValid(pReactorImpl, pReactorChannel, pError) || pReactorChannel->reactorParentQueue != &pReactorImpl->activeChannels)
			{
				rsslSetErrorInfo(pError, RSSL_EIC_FAILURE, RSSL_RET_FAILURE, __FILE__, __LINE__, "Invalid channel was specified.");
				return (reactorUnlockInterface(pReactorImpl), RSSL_RET_INVALID_ARGUMENT);
			}

			if (rsslNotifierEventIsReadable(pReactorImpl->pQueueNotifierEvent))
			{

				/* Dispatch from reactor queue */
				while (maxMsgs > 0)
				{
					if ((ret = _reactorDispatchEventFromQueue(pReactorImpl, &pReactorImpl->reactorEventQueue, pError)) < RSSL_RET_SUCCESS)
					{
						if (ret == RSSL_RET_READ_WOULD_BLOCK)
							break;
						else
						{
							_reactorShutdown(pReactorImpl, pError);
							_reactorSendShutdownEvent(pReactorImpl, pError);
							return (reactorUnlockInterface(pReactorImpl), ret);
						}
					}
					else
					{
						/* Message was successfully processed. */
						--maxMsgs;
						if (ret == RSSL_RET_SUCCESS)
							break;
					}
				}

				/* Dispatch from channel queue */
				while (maxMsgs > 0)
				{
					if ((ret = _reactorDispatchEventFromQueue(pReactorImpl, &pReactorChannel->eventQueue, pError)) < RSSL_RET_SUCCESS)
					{
						if (ret == RSSL_RET_READ_WOULD_BLOCK)
							break;
						else
						{
							_reactorShutdown(pReactorImpl, pError);
							_reactorSendShutdownEvent(pReactorImpl, pError);
							return (reactorUnlockInterface(pReactorImpl), ret);
						}
					}
					else
					{
						/* Message was successfully processed. */
						--maxMsgs;
						if (ret == RSSL_RET_SUCCESS)
							break;
					}
				}

			}

			channelsToCheck = 1;
			/* A channel has something to read if either:
			 * - The last return from rsslRead() was greater than zero, indicating there were still bytes in RSSL's queue
			 * - The file descriptor is set because there is data from the socket */
			if (pReactorChannel->readRet > 0 || rsslNotifierEventIsReadable(pReactorChannel->pNotifierEvent))
			{
				while (maxMsgs > 0 && channelsToCheck > 0)
				{
					if ((ret = _reactorDispatchFromChannel(pReactorImpl, pReactorChannel, pError)) < RSSL_RET_SUCCESS)
					{
						/* _reactorDispatchFromChannel from channel will disconnect the channel if it needs to. If if fails,
						 * then it had a problem doing that and there's nothing more we can do to handle it. */
						_reactorShutdown(pReactorImpl, pError);
						_reactorSendShutdownEvent(pReactorImpl, pError);
						return (reactorUnlockInterface(pReactorImpl), RSSL_RET_SUCCESS);
					}
					else if (pReactorChannel->readRet <= 0)
					{
						channelsToCheck = 0;
						break;
					}
					if (maxMsgs > 0) --maxMsgs;
				}
			}
			/* If not triggered to read, check if this channel has passed its ping timeout without sending either a ping or some data. */
			else if ((pReactorImpl->lastRecordedTimeMs - pReactorChannel->lastPingReadMs) > pReactorChannel->reactorChannel.pRsslChannel->pingTimeout * 1000)
			{
				rsslSetErrorInfo(pError, RSSL_EIC_FAILURE, RSSL_RET_FAILURE, __FILE__, __LINE__, "Channel ping timeout expired.");
				if (_reactorHandleChannelDown(pReactorImpl, pReactorChannel, pError) != RSSL_RET_SUCCESS)
				{
					_reactorShutdown(pReactorImpl, pError);
					_reactorSendShutdownEvent(pReactorImpl, pError);
					return (reactorUnlockInterface(pReactorImpl), RSSL_RET_FAILURE);
				}
				channelsToCheck = 0;
			}
			else
				channelsToCheck = 0;

			return (reactorUnlockInterface(pReactorImpl), channelsToCheck);
		}
		
	}
	else
	{
		return (reactorUnlockInterface(pReactorImpl), RSSL_RET_FAILURE);
	}
}

RSSL_VA_API RsslRet rsslReactorSubmit(RsslReactor *pReactor, RsslReactorChannel *pChannel, RsslBuffer *buffer, RsslReactorSubmitOptions *pSubmitOptions, RsslErrorInfo *pError)
{
	RsslRet ret;
	RsslReactorImpl *pReactorImpl = (RsslReactorImpl*)pReactor;
	RsslReactorChannelImpl *pReactorChannel = (RsslReactorChannelImpl*)pChannel;
	RsslUInt32 dummyBytesWritten, dummyUncompBytesWritten;

	if ((ret = reactorLockInterface(pReactorImpl, RSSL_TRUE, pError)) != RSSL_RET_SUCCESS)
		return ret;

	/* Since the application passed in this channel, make sure it is valid for this reactor and that it is active. */
	if (!pReactorChannel || !rsslReactorChannelIsValid(pReactorImpl, pReactorChannel, pError))
		return (reactorUnlockInterface(pReactorImpl), RSSL_RET_INVALID_ARGUMENT);

	if (pReactorImpl->state != RSSL_REACTOR_ST_ACTIVE)
	{
		rsslSetErrorInfo(pError, RSSL_EIC_FAILURE, RSSL_RET_INVALID_ARGUMENT, __FILE__, __LINE__, "Reactor is shutting down.");
		return (reactorUnlockInterface((RsslReactorImpl*)pReactor), RSSL_RET_FAILURE);
	}

	if (pReactorChannel->reactorParentQueue != &pReactorImpl->activeChannels)
	{
		rsslSetErrorInfo(pError, RSSL_EIC_FAILURE, RSSL_RET_FAILURE, __FILE__, __LINE__, "Channel is not active.");
		return (reactorUnlockInterface((RsslReactorImpl*)pReactor), RSSL_RET_FAILURE);
	}


	if (pReactorChannel->channelRole.base.roleType == RSSL_RC_RT_OMM_CONSUMER
			&& pReactorChannel->channelRole.ommConsumerRole.watchlistOptions.enableWatchlist)
	{
		rsslSetErrorInfo(pError, RSSL_EIC_FAILURE, RSSL_RET_INVALID_ARGUMENT, __FILE__, __LINE__, "rsslReactorSubmit may not be used when watchlist is enabled.");
		return (reactorUnlockInterface((RsslReactorImpl*)pReactor), RSSL_RET_INVALID_ARGUMENT);
	}
	
	/* Write message */
	ret = rsslWrite(pReactorChannel->reactorChannel.pRsslChannel, 
			buffer, 
			pSubmitOptions->priority,
			pSubmitOptions->writeFlags, 
			pSubmitOptions->pBytesWritten ? pSubmitOptions->pBytesWritten : &dummyBytesWritten,
			pSubmitOptions->pUncompressedBytesWritten ? pSubmitOptions->pUncompressedBytesWritten : &dummyUncompBytesWritten,
			&pError->rsslError);

	if ( ret < RSSL_RET_SUCCESS)
	{
		switch (ret)
		{
			case RSSL_RET_WRITE_FLUSH_FAILED:
				/* rsslWrite has the message, but attempted to flush and failed.  This is okay, just need to keep flushing. */
				ret = RSSL_RET_SUCCESS;
				pReactorChannel->writeRet = 1;
				break;
			case RSSL_RET_WRITE_CALL_AGAIN:
				/* The message is a fragmented message and was only partially written because there were not enough output buffers in RSSL to send it.
				 * We will request flushing to make the buffers available.
				 * RSSL_RET_WRITE_CALL_AGAIN will still be returned to the application.  It should attempt to call rsslReactorSubmit() again later. */
				pReactorChannel->writeRet = 1;
				break;
			default:
				/* Failure */
				rsslSetErrorInfoLocation(pError, __FILE__, __LINE__);
				return (reactorUnlockInterface(pReactorImpl), ret);
		}
	}
	else if (ret > 0)
	{
		/* The message was written to RSSL but has not yet been fully written to the network.  Flushing is needed to complete sending. */
		pReactorChannel->writeRet = ret;
		ret = RSSL_RET_SUCCESS;
	}

	if (pReactorChannel->writeRet > 0)
	{
		ret =  _reactorSendFlushRequest(pReactorImpl, pReactorChannel, pError);
		return (reactorUnlockInterface(pReactorImpl), ret);
	}

	return (reactorUnlockInterface((RsslReactorImpl*)pReactor), ret);

}

RsslUInt32 _reactorMsgEncodedSize(RsslMsg *pMsg)
{
	RsslUInt32 msgSize = 128;
	const RsslMsgKey *pKey;

	msgSize += pMsg->msgBase.encDataBody.length;

	if ((pKey = rsslGetMsgKey(pMsg)))
	{
		if (pKey->flags & RSSL_MKF_HAS_NAME)
			msgSize += pKey->name.length;

		if (pKey->flags & RSSL_MKF_HAS_ATTRIB)
			msgSize += pKey->encAttrib.length;
	}
	
	return msgSize;
}

RSSL_VA_API RsslRet rsslReactorSubmitMsg(RsslReactor *pReactor, RsslReactorChannel *pChannel, RsslReactorSubmitMsgOptions *pOptions, RsslErrorInfo *pError)
{
	RsslReactorImpl *pReactorImpl = (RsslReactorImpl*)pReactor;
	RsslReactorChannelImpl *pReactorChannel = (RsslReactorChannelImpl*)pChannel;
	RsslRet ret;


	if ((ret = reactorLockInterface(pReactorImpl, RSSL_TRUE, pError)) != RSSL_RET_SUCCESS)
		return ret;

	/* Since the application passed in this channel, make sure it is valid for this reactor and that it is active. */
	if (!pReactorChannel || !rsslReactorChannelIsValid(pReactorImpl, pReactorChannel, pError))
		return (reactorUnlockInterface(pReactorImpl), RSSL_RET_INVALID_ARGUMENT);

	if (pReactorImpl->state != RSSL_REACTOR_ST_ACTIVE)
	{
		rsslSetErrorInfo(pError, RSSL_EIC_FAILURE, RSSL_RET_INVALID_ARGUMENT, __FILE__, __LINE__, "Reactor is shutting down.");
		return (reactorUnlockInterface((RsslReactorImpl*)pReactor), RSSL_RET_FAILURE);
	}

	if (pReactorChannel->pWatchlist)
	{
		RsslWatchlistProcessMsgOptions processOpts;

		rsslWatchlistClearProcessMsgOptions(&processOpts);
		processOpts.pChannel = pChannel->pRsslChannel;
		processOpts.pRsslMsg = pOptions->pRsslMsg;
		processOpts.pRdmMsg = pOptions->pRDMMsg;
		processOpts.pServiceName = pOptions->pServiceName;
		processOpts.pUserSpec = pOptions->requestMsgOptions.pUserSpec;
		processOpts.majorVersion = pOptions->majorVersion;
		processOpts.minorVersion = pOptions->minorVersion;

		ret = _reactorSubmitWatchlistMsg(pReactorImpl, pReactorChannel, &processOpts, pError);
		return (reactorUnlockInterface((RsslReactorImpl*)pReactor), ret);

	}
	else
	{
		/* Watchlist is off. Encode the message for the application. */

		RsslUInt32 msgSize;

		if (pReactorChannel->reactorParentQueue != &pReactorImpl->activeChannels)
		{
			rsslSetErrorInfo(pError, RSSL_EIC_FAILURE, RSSL_RET_FAILURE, __FILE__, __LINE__, "Channel is not active.");
			return (reactorUnlockInterface((RsslReactorImpl*)pReactor), RSSL_RET_FAILURE);
		}

		if (pReactorChannel->pWriteCallAgainBuffer)
		{
			rsslSetErrorInfo(pError, RSSL_EIC_FAILURE, RSSL_RET_BUFFER_NO_BUFFERS, __FILE__, __LINE__, "Writing of fragmented buffer still in progress.");
			return (reactorUnlockInterface((RsslReactorImpl*)pReactor), RSSL_RET_BUFFER_NO_BUFFERS);
		}

		if (pOptions->pRsslMsg)
		{
			/* User-provided message structure. Encode the message. */
			RsslBuffer *pWriteBuffer;
			RsslEncodeIterator encodeIter;

			msgSize = _reactorMsgEncodedSize(pOptions->pRsslMsg);

			do
			{

				if (!(pWriteBuffer = rsslGetBuffer(pChannel->pRsslChannel, msgSize, RSSL_FALSE, 
								&pError->rsslError)))
				{
					rsslSetErrorInfoLocation(pError, __FILE__, __LINE__);
					return (reactorUnlockInterface((RsslReactorImpl*)pReactor), pError->rsslError.rsslErrorId);
				}

				rsslClearEncodeIterator(&encodeIter);
				rsslSetEncodeIteratorRWFVersion(&encodeIter, pChannel->pRsslChannel->majorVersion,
						pChannel->pRsslChannel->minorVersion);

				rsslSetEncodeIteratorBuffer(&encodeIter, pWriteBuffer);

				if ((ret = rsslEncodeMsg(&encodeIter, pOptions->pRsslMsg)) == RSSL_RET_SUCCESS)
					break;

				rsslReleaseBuffer(pWriteBuffer, &pError->rsslError);
				switch (ret)
				{
					case RSSL_RET_BUFFER_TOO_SMALL:
						/* Double buffer size and try again. */
						msgSize *= 2;
						break;
					default:
					{
						rsslSetErrorInfo(pError, RSSL_EIC_FAILURE, ret, __FILE__, __LINE__, "Message encoding failure.");
						return (reactorUnlockInterface((RsslReactorImpl*)pReactor), ret);
					}
				}

			} while (ret != RSSL_RET_SUCCESS);

			pWriteBuffer->length = rsslGetEncodedBufferLength(&encodeIter);

			ret = _reactorSubmit((RsslReactorImpl*)pReactor, pReactorChannel, 
					pWriteBuffer, pError);
			return (reactorUnlockInterface((RsslReactorImpl*)pReactor), ret);

		}
		else if (pOptions->pRDMMsg)
		{
			ret = _reactorSendRDMMessage(pReactorImpl, pReactorChannel, pOptions->pRDMMsg, pError);
			return (reactorUnlockInterface((RsslReactorImpl*)pReactor), ret);
		}
		else
		{
			rsslSetErrorInfo(pError, RSSL_EIC_FAILURE, RSSL_RET_INVALID_ARGUMENT, __FILE__, __LINE__, "No message or buffer provided.");
			return (reactorUnlockInterface((RsslReactorImpl*)pReactor), RSSL_RET_INVALID_ARGUMENT);
		}
	}

}

static RsslRet _reactorSendRDMMessage(RsslReactorImpl *pReactorImpl, RsslReactorChannelImpl *pReactorChannel, RsslRDMMsg *pRDMMsg, RsslErrorInfo *pError)
{
	RsslEncodeIterator eIter;
	RsslBuffer *pBuf;
	RsslRet encodeRet, ret;
	RsslReactorChannelInfo channelInfo;
	RsslUInt32 bufferSize, prevBufferSize;

	if (pReactorChannel->pWatchlist)
	{
		RsslWatchlistProcessMsgOptions processOpts;
		RsslDecodeIterator dIter;

		rsslClearDecodeIterator(&dIter);
		rsslSetDecodeIteratorRWFVersion(&dIter, 
				pReactorChannel->reactorChannel.pRsslChannel->majorVersion, 
				pReactorChannel->reactorChannel.pRsslChannel->minorVersion);

		rsslWatchlistClearProcessMsgOptions(&processOpts);
		processOpts.pChannel = pReactorChannel->reactorChannel.pRsslChannel;
		processOpts.pDecodeIterator = &dIter;
		processOpts.pRdmMsg = pRDMMsg;

		if ((ret = rsslWatchlistSubmitMsg(pReactorChannel->pWatchlist, &processOpts, pError))
				!= RSSL_RET_SUCCESS)
			return ret;

		if (pReactorChannel->pWatchlist->state & RSSLWL_STF_NEED_FLUSH) 
		{
			pReactorChannel->writeRet = 1;
			return _reactorSendFlushRequest(pReactorImpl, pReactorChannel, pError);
		}

		return RSSL_RET_SUCCESS;
	}

	if ((ret = rsslReactorGetChannelInfo(&pReactorChannel->reactorChannel, &channelInfo, pError)) != RSSL_RET_SUCCESS)
	{
		rsslSetErrorInfoLocation(pError, __FILE__, __LINE__);
		return ret;
	}

	bufferSize = channelInfo.rsslChannelInfo.maxFragmentSize;

	/* Most messages should be able to be encoded in a buffer of size less than the max fragment size.
	 * If the buffer isn't large enough, double the buffer's size and try again. */
	while(1)
	{
		if (!(pBuf = rsslReactorGetBuffer(&pReactorChannel->reactorChannel, bufferSize , RSSL_FALSE, pError)))
		{
			rsslSetErrorInfo(pError, RSSL_EIC_FAILURE, RSSL_RET_FAILURE, __FILE__, __LINE__, "rsslGetBuffer() failed for buffer size %u", bufferSize);
			return RSSL_RET_FAILURE;
		}

		rsslClearEncodeIterator(&eIter);
		rsslSetEncodeIteratorRWFVersion(&eIter, pReactorChannel->reactorChannel.pRsslChannel->majorVersion, pReactorChannel->reactorChannel.pRsslChannel->minorVersion);
		rsslSetEncodeIteratorBuffer(&eIter, pBuf);

		if ((ret = rsslEncodeRDMMsg(&eIter, pRDMMsg, &pBuf->length, pError)) >= RSSL_RET_SUCCESS)
			break;

		encodeRet = pError->rsslError.rsslErrorId;

		if ((ret = rsslReleaseBuffer(pBuf, &pError->rsslError) != RSSL_RET_SUCCESS))
		{
			rsslSetErrorInfoLocation(pError, __FILE__, __LINE__);
			return ret;
		}

		switch(encodeRet)
		{
			case RSSL_RET_BUFFER_TOO_SMALL:
				prevBufferSize = bufferSize;
				bufferSize *= 2;
				if (bufferSize < prevBufferSize)
				{
					/* Overflow */
					rsslSetErrorInfo(pError, RSSL_EIC_FAILURE, RSSL_RET_FAILURE, __FILE__, __LINE__, "Buffer size needed for encoding typed message is too large", bufferSize);
					return RSSL_RET_FAILURE;
				}
				continue;
			default:
				return encodeRet;
		}
	}

	if ((ret = _reactorSubmit(pReactorImpl, pReactorChannel, pBuf, pError)) < RSSL_RET_SUCCESS)
		return ret;

	return RSSL_RET_SUCCESS;
}

static RsslRet _reactorHandleChannelDown(RsslReactorImpl *pReactorImpl, RsslReactorChannelImpl *pReactorChannel, RsslErrorInfo *pError)
{
	RsslReactorChannelEventImpl *pEvent;
	RsslChannel *pChannel = pReactorChannel->reactorChannel.pRsslChannel;
	RsslReactorCallbackRet cret;

	if (pReactorChannel->reactorParentQueue == &pReactorImpl->inactiveChannels)
		return RSSL_RET_SUCCESS; /* We are already in the process of closing this channel(this may occur if the worker thread also received an error for this channel). Nothing to do. */

	pEvent = (RsslReactorChannelEventImpl*)rsslReactorEventQueueGetFromPool(&pReactorImpl->reactorWorker.workerQueue);

	if (rsslNotifierRemoveEvent(pReactorImpl->pNotifier, pReactorChannel->pNotifierEvent) < 0)
	{
		rsslSetErrorInfo(pError, RSSL_EIC_FAILURE, RSSL_RET_FAILURE, __FILE__, __LINE__, 
				"Failed to remove notification event for disconnected channel.");
		return RSSL_RET_FAILURE;
	}

	if(pReactorChannel->reconnectAttemptLimit != 0 && pReactorChannel->reconnectAttemptCount != pReactorChannel->reconnectAttemptLimit)
	{

		_reactorMoveChannel(&pReactorImpl->reconnectingChannels, pReactorChannel);

		/* Notify worker that this channel is down, but recovering */
		rsslClearReactorChannelEventImpl(pEvent);
		pEvent->channelEvent.channelEventType = pReactorImpl->state == RSSL_REACTOR_ST_ACTIVE ?
			RSSL_RC_CET_CHANNEL_DOWN_RECONNECTING : RSSL_RC_CET_CHANNEL_DOWN;
		pEvent->channelEvent.pReactorChannel = (RsslReactorChannel*)pReactorChannel;
		pEvent->channelEvent.pError = pError;

		rsslResetReactorChannelState(pReactorImpl, pReactorChannel);
	}
	else
	{
	_reactorMoveChannel(&pReactorImpl->inactiveChannels, pReactorChannel);

	/* Notify worker that this channel is down */
	rsslClearReactorChannelEventImpl(pEvent);
	pEvent->channelEvent.channelEventType = RSSL_RC_CET_CHANNEL_DOWN;
	pEvent->channelEvent.pReactorChannel = (RsslReactorChannel*)pReactorChannel;
	pEvent->channelEvent.pError = pError;
	}

	/* Bring down tunnel streams. */
	if (pReactorChannel->pTunnelManager 
			&& !pReactorChannel->pWatchlist /* Watchlist will fanout closes */)
	{
		RsslRet ret;
		if ((ret = tunnelManagerHandleChannelClosed(pReactorChannel->pTunnelManager, pError)) != RSSL_RET_SUCCESS)
				return ret;
	}

	_reactorSetInCallback(pReactorImpl, RSSL_TRUE);
	cret = (*pReactorChannel->channelRole.base.channelEventCallback)((RsslReactor*)pReactorImpl, (RsslReactorChannel*)pReactorChannel, &pEvent->channelEvent);
	_reactorSetInCallback(pReactorImpl, RSSL_FALSE);

	if (pReactorChannel->pWatchlist)
	{
		RsslRet ret;
		if (ret = rsslWatchlistSetChannel(pReactorChannel->pWatchlist, NULL, pError) != RSSL_RET_SUCCESS)
			return ret;
	}

	if(pReactorChannel->reactorParentQueue ==  &pReactorImpl->closingChannels)
	{
		rsslReactorEventQueueReturnToPool((RsslReactorEventImpl*)pEvent, &pReactorImpl->reactorWorker.workerQueue);
		return RSSL_RET_SUCCESS;
	}

	if (!RSSL_ERROR_INFO_CHECK(rsslReactorEventQueuePut(&pReactorImpl->reactorWorker.workerQueue, (RsslReactorEventImpl*)pEvent) == RSSL_RET_SUCCESS, RSSL_RET_FAILURE, pError))
		return RSSL_RET_FAILURE;

	return RSSL_RET_SUCCESS;
}

static RsslRet _reactorDispatchWatchlist(RsslReactorImpl *pReactorImpl, RsslReactorChannelImpl *pReactorChannel, RsslErrorInfo *pError)
{
	RsslReactorChannelEventImpl *pEvent;
	RsslRet ret;

	if (pReactorChannel->reactorParentQueue == &pReactorImpl->closingChannels)
		return RSSL_RET_SUCCESS;

	ret = rsslWatchlistDispatch(pReactorChannel->pWatchlist, pReactorImpl->lastRecordedTimeMs, pError);
	if (ret < RSSL_RET_SUCCESS)
		return (_reactorHandleChannelDown(pReactorImpl, pReactorChannel, pError));
	else 
	{
		if (pReactorChannel->pWatchlist->state & RSSLWL_STF_NEED_TIMER) 
			if (_reactorSetTimer(pReactorImpl, pReactorChannel, rsslWatchlistGetNextTimeout(pReactorChannel->pWatchlist),
					pError) != RSSL_RET_SUCCESS)
				return ret;

		if (pReactorChannel->pWatchlist->state & RSSLWL_STF_NEED_FLUSH) 
		{
			pReactorChannel->writeRet = 1;
			return _reactorSendFlushRequest(pReactorImpl, pReactorChannel, pError);
		}
		else if (ret > RSSL_RET_SUCCESS && !pReactorChannel->wlDispatchEventQueued)
		{
			pEvent = (RsslReactorChannelEventImpl*)rsslReactorEventQueueGetFromPool(&pReactorChannel->eventQueue);
			rsslClearReactorChannelEventImpl(pEvent);
			pEvent->channelEvent.channelEventType = (RsslReactorChannelEventType)RSSL_RCIMPL_CET_DISPATCH_WL;
			pEvent->channelEvent.pReactorChannel = (RsslReactorChannel*)pReactorChannel;
			return rsslReactorEventQueuePut(&pReactorChannel->eventQueue, (RsslReactorEventImpl*)pEvent);
		}

	}

	return ret;
}

static RsslRet _reactorReadWatchlistMsg(RsslReactorImpl *pReactorImpl, RsslReactorChannelImpl *pReactorChannel, 
		RsslWatchlistProcessMsgOptions *pOptions, RsslErrorInfo *pError)
{
	RsslRet ret;

	if ((ret = rsslWatchlistReadMsg(pReactorChannel->pWatchlist, pOptions, pReactorImpl->lastRecordedTimeMs, pError)) < RSSL_RET_SUCCESS)
		return _reactorHandleChannelDown(pReactorImpl, pReactorChannel, pError);

	if (pReactorImpl->state != RSSL_REACTOR_ST_ACTIVE)
	{
		_reactorShutdown(pReactorImpl, pError);
		_reactorSendShutdownEvent(pReactorImpl, pError);
		return ret;
	}

	if (pReactorChannel->pWatchlist->state & RSSLWL_STF_NEED_FLUSH) 
	{
		pReactorChannel->writeRet = 1;
		ret =  _reactorSendFlushRequest(pReactorImpl, pReactorChannel, pError);
		if (ret < RSSL_RET_SUCCESS)
			return ret;
	}

	if (pReactorChannel->pWatchlist->state & RSSLWL_STF_RESET_CONN_DELAY) 
	{
		/* Login stream established. Reset connection delay. */
		pReactorChannel->reconnectAttemptCount = 0;
		pReactorChannel->pWatchlist->state &= ~RSSLWL_STF_RESET_CONN_DELAY;
	}

	if (pReactorChannel->pWatchlist->state & RSSLWL_STF_NEED_TIMER) 
		if (_reactorSetTimer(pReactorImpl, pReactorChannel, rsslWatchlistGetNextTimeout(pReactorChannel->pWatchlist),
					pError) != RSSL_RET_SUCCESS)
			return ret;

	if (ret > RSSL_RET_SUCCESS && !pReactorChannel->wlDispatchEventQueued)
	{
		RsslReactorChannelEventImpl *pEvent;
		pEvent = (RsslReactorChannelEventImpl*)rsslReactorEventQueueGetFromPool(&pReactorChannel->eventQueue);
		rsslClearReactorChannelEventImpl(pEvent);
		pEvent->channelEvent.channelEventType = (RsslReactorChannelEventType)RSSL_RCIMPL_CET_DISPATCH_WL;
		pEvent->channelEvent.pReactorChannel = (RsslReactorChannel*)pReactorChannel;
		pEvent->channelEvent.pError = pError;
		return rsslReactorEventQueuePut(&pReactorChannel->eventQueue, (RsslReactorEventImpl*)pEvent);
	}

	return RSSL_RET_SUCCESS;
}

static RsslRet _reactorSubmitWatchlistMsg(RsslReactorImpl *pReactorImpl, RsslReactorChannelImpl *pReactorChannel, 
		RsslWatchlistProcessMsgOptions *pOptions, RsslErrorInfo *pError)
{
	RsslRet ret;

	if ((ret = rsslWatchlistSubmitMsg(pReactorChannel->pWatchlist, pOptions, pError)) < RSSL_RET_SUCCESS)
		return ret;

	if (pReactorChannel->pWatchlist->state & RSSLWL_STF_NEED_FLUSH) 
	{
		pReactorChannel->writeRet = 1;
		ret =  _reactorSendFlushRequest(pReactorImpl, pReactorChannel, pError);
		if (ret < RSSL_RET_SUCCESS)
			return ret;
	}

	if (pReactorChannel->pWatchlist->state & RSSLWL_STF_NEED_TIMER) 
		if (_reactorSetTimer(pReactorImpl, pReactorChannel, rsslWatchlistGetNextTimeout(pReactorChannel->pWatchlist),
					pError) != RSSL_RET_SUCCESS)
			return ret;

	if (ret > RSSL_RET_SUCCESS && !pReactorChannel->wlDispatchEventQueued)
	{
		RsslReactorChannelEventImpl *pEvent;
		pEvent = (RsslReactorChannelEventImpl*)rsslReactorEventQueueGetFromPool(&pReactorChannel->eventQueue);
		rsslClearReactorChannelEventImpl(pEvent);
		pEvent->channelEvent.channelEventType = (RsslReactorChannelEventType)RSSL_RCIMPL_CET_DISPATCH_WL;
		pEvent->channelEvent.pReactorChannel = (RsslReactorChannel*)pReactorChannel;
		pEvent->channelEvent.pError = pError;
		return rsslReactorEventQueuePut(&pReactorChannel->eventQueue, (RsslReactorEventImpl*)pEvent);
	}

	return RSSL_RET_SUCCESS;
}

RSSL_VA_API RsslRet rsslReactorCloseChannel(RsslReactor *pReactor, RsslReactorChannel *pChannel, RsslErrorInfo *pError)
{
	RsslReactorImpl *pReactorImpl = (RsslReactorImpl*)pReactor;
	RsslReactorChannelImpl *pReactorChannel;
	RsslRet ret;

	if ((ret = reactorLockInterface(pReactorImpl, RSSL_TRUE, pError)) != RSSL_RET_SUCCESS)
		return ret;

	pReactorChannel = (RsslReactorChannelImpl*)pChannel;

	if (!pReactorChannel || !rsslReactorChannelIsValid(pReactorImpl, pReactorChannel, pError))
	{
		rsslSetErrorInfo(pError, RSSL_EIC_FAILURE, RSSL_RET_INVALID_ARGUMENT, __FILE__, __LINE__, "Invalid argument");
		return (reactorUnlockInterface(pReactorImpl), RSSL_RET_INVALID_ARGUMENT );
	}

	if (pReactorChannel->reactorParentQueue == &pReactorImpl->closingChannels)
	{
		/* Channel is in the process of being closed. */

		return (reactorUnlockInterface(pReactorImpl), RSSL_RET_SUCCESS);
	}
	else
	{
		/* Channel is not currently trying to close.  Start the process of shutting it down. */
		RsslReactorChannelEventImpl *pEvent = (RsslReactorChannelEventImpl*)rsslReactorEventQueueGetFromPool(&pReactorImpl->reactorWorker.workerQueue);

		if (rsslNotifierRemoveEvent(pReactorImpl->pNotifier, pReactorChannel->pNotifierEvent) < 0)
		{
			rsslSetErrorInfo(pError, RSSL_EIC_FAILURE, RSSL_RET_FAILURE, __FILE__, __LINE__, 
					"Failed to remove notification event for disconnected channel.");
			return RSSL_RET_FAILURE;
		}

		_reactorMoveChannel(&pReactorImpl->closingChannels, pReactorChannel);

		/* Send request to worker to close this channel */
		rsslClearReactorChannelEventImpl(pEvent);
		pEvent->channelEvent.channelEventType = (RsslReactorChannelEventType)RSSL_RCIMPL_CET_CLOSE_CHANNEL;
		pEvent->channelEvent.pReactorChannel = (RsslReactorChannel*)pReactorChannel;
		pEvent->channelEvent.pError = pError;

        // cleanup TunnelManager
		if (pReactorChannel->pTunnelManager)
		{
			tunnelManagerClose(pReactorChannel->pTunnelManager, pError);
			pReactorChannel->pTunnelManager = NULL;
		}

		if (!RSSL_ERROR_INFO_CHECK(rsslReactorEventQueuePut(&pReactorImpl->reactorWorker.workerQueue, (RsslReactorEventImpl*)pEvent) == RSSL_RET_SUCCESS, RSSL_RET_FAILURE, pError))
			return (reactorUnlockInterface(pReactorImpl), RSSL_RET_FAILURE);

		return (reactorUnlockInterface(pReactorImpl), RSSL_RET_SUCCESS);
	}
}

/* Checks if tunnel stream needs a dispatch, flush, or timer (shouldn't produce a callback,
 * so it can be used in non-dispatch calls). */
static RsslRet _reactorHandleTunnelManagerEvents(RsslReactorImpl *pReactorImpl, RsslReactorChannelImpl *pReactorChannel, RsslErrorInfo *pError)
{
	RsslRet ret;

	if (tunnelManagerNeedsDispatchNow(pReactorChannel->pTunnelManager))
	{
		RsslReactorChannelEventImpl *pEvent;
		pEvent = (RsslReactorChannelEventImpl*)rsslReactorEventQueueGetFromPool(&pReactorChannel->eventQueue);
		rsslClearReactorChannelEventImpl(pEvent);
		pEvent->channelEvent.channelEventType = (RsslReactorChannelEventType)RSSL_RCIMPL_CET_DISPATCH_TUNNEL_STREAM;
		pEvent->channelEvent.pReactorChannel = (RsslReactorChannel*)pReactorChannel;
		if ((ret = rsslReactorEventQueuePut(&pReactorChannel->eventQueue, (RsslReactorEventImpl*)pEvent)) != RSSL_RET_SUCCESS)
			return ret;
	}

	if (tunnelManagerHasExpireTime(pReactorChannel->pTunnelManager))
	{
		RsslInt64 nextExpireTime = 
			tunnelManagerGetExpireTime(pReactorChannel->pTunnelManager);

		if (_reactorSetTimer(pReactorImpl, pReactorChannel, nextExpireTime, pError) 
				!= RSSL_RET_SUCCESS)
			return ret;
	}

	/* If watchlist is on, messages may have gone through it so check if a flush is needed. */
	if (pReactorChannel->pWatchlist && pReactorChannel->pWatchlist->state & RSSLWL_STF_NEED_FLUSH)
	{
		pReactorChannel->writeRet = 1;
		return _reactorSendFlushRequest(pReactorImpl, pReactorChannel, pError);
	}

	return RSSL_RET_SUCCESS;
}

static RsslRet _reactorHandleTunnelManagerRet(RsslReactorImpl *pReactorImpl, RsslReactorChannelImpl *pReactorChannel, RsslRet ret, RsslErrorInfo *pError)
{
	switch (ret)
	{
		case RSSL_RET_SUCCESS:
			return _reactorHandleTunnelManagerEvents(pReactorImpl, pReactorChannel, pError);

		case RSSL_RET_BUFFER_NO_BUFFERS:
			if ((ret = _reactorSendFlushRequest(pReactorImpl, pReactorChannel, pError)) != RSSL_RET_SUCCESS)
				return ret;
			return RSSL_RET_SUCCESS;
		default:
			return _reactorHandleChannelDown(pReactorImpl, pReactorChannel, pError);
	}

}

static RsslRet _reactorDispatchEventFromQueue(RsslReactorImpl *pReactorImpl, RsslReactorEventQueue *pQueue, RsslErrorInfo *pError)
{
	RsslRet ret;
	RsslReactorEventImpl *pEvent;
	RsslReactorChannelImpl *pReactorChannel;
	RsslReactorCallbackRet cret = RSSL_RC_CRET_SUCCESS;

	pEvent = rsslReactorEventQueueGet(pQueue, &ret);


	if(pEvent)
	{ 
		switch(pEvent->base.eventType)
		{
			case RSSL_RCIMPL_ET_FLUSH:
			{
				/* Received an event related to flushing. */

				RsslReactorFlushEvent *pFlushEvent = (RsslReactorFlushEvent*)pEvent;
				pReactorChannel = (RsslReactorChannelImpl*)pFlushEvent->pReactorChannel;

				if (pReactorChannel->reactorParentQueue == &pReactorImpl->activeChannels)
				{
					switch(pFlushEvent->flushEventType)
					{
						case RSSL_RCIMPL_FET_FLUSH_DONE:
							/* Worker thread has finished flushing for this channel. */
							pReactorChannel->requestedFlush = RSSL_FALSE; /* Flushing is complete and no more is needed. */

							if (pReactorChannel->pWriteCallAgainBuffer)
							{
								/* The reactor previously attempted to submit a message and got RSSL_RET_WRITE_CALL_AGAIN.
								 * Since flushing has occurred, buffers should be available for us to try again. */
								ret = _reactorSubmit(pReactorImpl, pReactorChannel, pReactorChannel->pWriteCallAgainBuffer, pError);

								if (ret < RSSL_RET_SUCCESS)
									return ret;
							}
							else if (pReactorChannel->writeRet > 0)
							{
								/* We may have written more messages since the last time we requested a flush.
								 * If so, start flushing again to make sure those messages don't get ignored. */
								if ((ret = _reactorSendFlushRequest(pReactorImpl, pReactorChannel, pError)) != RSSL_RET_SUCCESS)
									return ret;

							}

							/* Might have some buffers free, try and send. */
							if (pReactorChannel->pTunnelManager)
							{
								if (tunnelManagerNeedsDispatchNow(pReactorChannel->pTunnelManager))
								{
									ret = tunnelManagerDispatch(pReactorChannel->pTunnelManager, pError);
									if (_reactorHandleTunnelManagerRet(pReactorImpl, pReactorChannel, ret, pError) != RSSL_RET_SUCCESS)
										return ret;
								}
							}

							break;
						default:
							/* Unknown event */
							rsslSetErrorInfo(pError, RSSL_EIC_FAILURE, RSSL_RET_FAILURE, __FILE__, __LINE__, 
									"Unknown flush event type %d", pFlushEvent->flushEventType);
							return RSSL_RET_FAILURE;
					}
				}
				else
				{
					/* Ignore this event because the channel is not in the active list(we are likely trying to clean it up and 
					 * therefore don't need to act on flushing-related events). */
				}
				break;

			}

			case RSSL_RCIMPL_ET_CHANNEL:
			{
				/* Received an event about the state of one of the channels. */

				RsslReactorChannelEventImpl *pConnEvent = (RsslReactorChannelEventImpl*)pEvent;
				pReactorChannel = (RsslReactorChannelImpl*)pConnEvent->channelEvent.pReactorChannel;

				if (pReactorChannel->reactorParentQueue != &pReactorImpl->closingChannels)
				{

					switch(pConnEvent->channelEvent.channelEventType)
					{
						case RSSL_RC_CET_CHANNEL_UP:
							/* Channel has been initialized by worker thread and is ready for reading & writing. */
							if (rsslNotifierAddEvent(pReactorImpl->pNotifier, pReactorChannel->pNotifierEvent, pReactorChannel->reactorChannel.socketId, pReactorChannel) < 0)
							{
								rsslSetErrorInfo(pError, RSSL_EIC_FAILURE, RSSL_RET_FAILURE, __FILE__, __LINE__, 
										"Failed to add notification event for initialized channel.");
								return RSSL_RET_FAILURE;
							}
							if (rsslNotifierRegisterRead(pReactorImpl->pNotifier, pReactorChannel->pNotifierEvent) < 0)
							{
								rsslSetErrorInfo(pError, RSSL_EIC_FAILURE, RSSL_RET_FAILURE, __FILE__, __LINE__, 
										"Failed to register read notification for initialized channel.");
								return RSSL_RET_FAILURE;
							}

							pReactorChannel->requestedFlush = RSSL_FALSE;

							/* Reset reconnect attempt count (if  watchlist is enable, wait for a 
							 * login stream first). */
							if (pReactorChannel->pWatchlist == NULL)
							{
								pReactorChannel->reconnectAttemptCount = 0;
								pReactorChannel->lastReconnectAttemptMs = 0;
							}

							_reactorMoveChannel(&pReactorImpl->activeChannels, pReactorChannel);
							pReactorChannel->lastPingReadMs = pReactorImpl->lastRecordedTimeMs;

							/* Notify application */
							_reactorSetInCallback(pReactorImpl, RSSL_TRUE);
							cret = (*pReactorChannel->channelRole.base.channelEventCallback)((RsslReactor*)pReactorImpl, (RsslReactorChannel*)pReactorChannel, &pConnEvent->channelEvent);
							_reactorSetInCallback(pReactorImpl, RSSL_FALSE);

							if (cret == RSSL_RC_CRET_SUCCESS && pReactorChannel->reactorParentQueue != &pReactorImpl->closingChannels)
							{
								switch(pReactorChannel->channelRole.base.roleType)
								{
									case RSSL_RC_RT_OMM_CONSUMER:
										/* Send login request if given */
										if (pReactorChannel->channelRole.ommConsumerRole.pLoginRequest && !pReactorChannel->pWatchlist)
										{
											if ((ret = _reactorSendRDMMessage(pReactorImpl, pReactorChannel, (RsslRDMMsg*)pReactorChannel->channelRole.ommConsumerRole.pLoginRequest, pError)) != RSSL_RET_SUCCESS)
												if (_reactorHandleChannelDown(pReactorImpl, pReactorChannel, pError) != RSSL_RET_SUCCESS)
													return RSSL_RET_FAILURE;
											return RSSL_RET_SUCCESS;
										}
										else if (pReactorChannel->channelSetupState != RSSL_RC_CHST_READY)
										{
											/* Nothing more to send. Put CHANNEL_READY event on our own queue. */
											if (_reactorSendConnReadyEvent(pReactorImpl, pReactorChannel, pError) != RSSL_RET_SUCCESS)
												return RSSL_RET_FAILURE;
										}
										break;
									case RSSL_RC_RT_OMM_NI_PROVIDER:
										/* Send login request if given */
										if (pReactorChannel->channelRole.ommNIProviderRole.pLoginRequest)
										{
											if ((ret = _reactorSendRDMMessage(pReactorImpl, pReactorChannel, (RsslRDMMsg*)pReactorChannel->channelRole.ommNIProviderRole.pLoginRequest, pError)) != RSSL_RET_SUCCESS)
												if (_reactorHandleChannelDown(pReactorImpl, pReactorChannel, pError) != RSSL_RET_SUCCESS)
													return RSSL_RET_FAILURE;
											return RSSL_RET_SUCCESS;
										}
										else if (pReactorChannel->channelSetupState != RSSL_RC_CHST_READY)
										{
											/* Nothing more to send. Put CHANNEL_READY event on our own queue. */
											if (_reactorSendConnReadyEvent(pReactorImpl, pReactorChannel, pError) != RSSL_RET_SUCCESS)
												return RSSL_RET_FAILURE;
										}
										break;
									case RSSL_RC_RT_OMM_PROVIDER:
									{
										if (pReactorChannel->channelSetupState != RSSL_RC_CHST_READY)
										{
											/* Nothing more to send. Put CHANNEL_READY event on our own queue. */
											if (_reactorSendConnReadyEvent(pReactorImpl, pReactorChannel, pError) != RSSL_RET_SUCCESS)
												return RSSL_RET_FAILURE;
										}

									}
										break;
									default:
										break;
								}
							}

							if (pReactorChannel->pWatchlist)
								if ((ret = rsslWatchlistSetChannel(pReactorChannel->pWatchlist, pReactorChannel->reactorChannel.pRsslChannel,
											pError)) != RSSL_RET_SUCCESS)
									return ret;

							break;
						case RSSL_RC_CET_CHANNEL_READY:
							_reactorSetInCallback(pReactorImpl, RSSL_TRUE);
							cret = (*pReactorChannel->channelRole.base.channelEventCallback)((RsslReactor*)pReactorImpl, (RsslReactorChannel*)pReactorChannel, &pConnEvent->channelEvent);
							_reactorSetInCallback(pReactorImpl, RSSL_FALSE);
							break;

						case RSSL_RC_CET_CHANNEL_DOWN:
						case RSSL_RC_CET_CHANNEL_DOWN_RECONNECTING:
							/* If the channel is currently down, this event may be due to the worker finding out at the same time the reactor did.
							 * Ignore this event. Otherwise, it is redundant to provide to the application (we already know it's down), and it could
							 * re-trigger connection recovery that is already underway. */
							if (!pConnEvent->isConnectFailure && pReactorChannel->reactorParentQueue != &pReactorImpl->activeChannels)
								break;

							if (_reactorHandleChannelDown(pReactorImpl, pReactorChannel, pConnEvent->channelEvent.pError) != RSSL_RET_SUCCESS)
								return RSSL_RET_FAILURE;
							break;
						case RSSL_RCIMPL_CET_DISPATCH_WL:
						{
							assert(pReactorChannel->pWatchlist);
							pReactorChannel->wlDispatchEventQueued = RSSL_FALSE;
							break;
						}
						case RSSL_RCIMPL_CET_DISPATCH_TUNNEL_STREAM:
						{
							pReactorChannel->tunnelDispatchEventQueued = RSSL_FALSE;
							ret = tunnelManagerDispatch(pReactorChannel->pTunnelManager, pError);
							if (_reactorHandleTunnelManagerRet(pReactorImpl, pReactorChannel, ret, pError) != RSSL_RET_SUCCESS)
								return ret;


							break;
						}

						default:
							rsslSetErrorInfo(pError, RSSL_EIC_FAILURE, RSSL_RET_FAILURE, 
									__FILE__, __LINE__, "Unknown channel event type: %d", pConnEvent->channelEvent.channelEventType);
							return RSSL_RET_FAILURE;
					}
				}
				else
				{
					/* We are cleaning up this channel. Only watch for close acknowledgement. Ignore other events. */
					switch(pConnEvent->channelEvent.channelEventType)
					{
						case RSSL_RC_CET_CHANNEL_UP:
						case RSSL_RC_CET_CHANNEL_DOWN:
							break;
						case RSSL_RCIMPL_CET_DISPATCH_WL:
						case RSSL_RCIMPL_CET_DISPATCH_TUNNEL_STREAM:
							return RSSL_RET_SUCCESS;
						case RSSL_RCIMPL_CET_CLOSE_CHANNEL_ACK:
							if (pReactorChannel->pWatchlist)
							{
								rsslWatchlistDestroy(pReactorChannel->pWatchlist);
								pReactorChannel->pWatchlist = NULL;
							}

							_reactorMoveChannel(&pReactorImpl->channelPool, pReactorChannel);
							--pReactorImpl->channelCount;
							return RSSL_RC_CRET_SUCCESS;
						default:
							rsslSetErrorInfo(pError, RSSL_EIC_FAILURE, RSSL_RET_FAILURE, 
									__FILE__, __LINE__, "Unknown channel event type: %d", pConnEvent->channelEvent.channelEventType);
							return RSSL_RET_FAILURE;
					}
				}
				break;
			}

			case RSSL_RCIMPL_ET_REACTOR:
			{
				RsslReactorStateEvent *pReactorEvent = (RsslReactorStateEvent*)pEvent;
				switch(pReactorEvent->reactorEventType)
				{
					case RSSL_RCIMPL_STET_SHUTDOWN:
						/* Worker has encountered an error and needs to shut down. */
						rsslCopyErrorInfo(pError, pReactorEvent->pErrorInfo);
						_reactorShutdown(pReactorImpl, pError);
						_reactorSendShutdownEvent(pReactorImpl, pError);
						return RSSL_RET_SUCCESS;

					default:
						rsslSetErrorInfo(pError, RSSL_EIC_FAILURE, RSSL_RET_FAILURE, __FILE__, __LINE__, "Unknown reactor event type: %d", pReactorEvent->reactorEventType);
						return RSSL_RET_FAILURE;
				}
			}

			case RSSL_RCIMPL_ET_TIMER:
			{
				pReactorChannel = (RsslReactorChannelImpl*)pEvent->timerEvent.pReactorChannel;
				pReactorChannel->lastRequestedExpireTime = RCIMPL_TIMER_UNSET;

				if (pReactorChannel->pWatchlist)
				{
					if ((ret = rsslWatchlistProcessTimer(pReactorChannel->pWatchlist, pEvent->timerEvent.expireTime, pError))
							!= RSSL_RET_SUCCESS)
						return (_reactorHandleChannelDown(pReactorImpl, pReactorChannel, pError));
				}

				if (pReactorChannel->pTunnelManager)
				{
					ret = tunnelManagerProcessTimer(pReactorChannel->pTunnelManager, pEvent->timerEvent.expireTime, pError);
					if (_reactorHandleTunnelManagerRet(pReactorImpl, pReactorChannel, ret, pError) != RSSL_RET_SUCCESS)
						return ret;
				}
				break;
			}
			default:
				rsslSetErrorInfo(pError, RSSL_EIC_FAILURE, RSSL_RET_FAILURE, __FILE__, __LINE__, "Unknown event type: %d", pEvent->base.eventType);
				return RSSL_RET_FAILURE;
		}
	}
	else if (!RSSL_ERROR_INFO_CHECK(ret >= RSSL_RET_SUCCESS, ret, pError))
	{
		_reactorShutdown(pReactorImpl, pError);
		_reactorSendShutdownEvent(pReactorImpl, pError);
		return RSSL_RET_FAILURE;
	}
	else
		return RSSL_RET_READ_WOULD_BLOCK;

	switch(cret)
	{
		case RSSL_RC_CRET_SUCCESS:
			if (pReactorChannel->pWatchlist)
			{
				if (_reactorDispatchWatchlist(pReactorImpl, pReactorChannel, pError) != RSSL_RET_SUCCESS)
					return RSSL_RET_FAILURE;

				if (pReactorChannel->pWatchlist->state & RSSLWL_STF_NEED_FLUSH)
				{
					pReactorChannel->writeRet = 1;
					return _reactorSendFlushRequest(pReactorImpl, pReactorChannel, pError);
				}
			}

			if (pReactorChannel->pTunnelManager)
			{
				ret = tunnelManagerDispatch(pReactorChannel->pTunnelManager, pError);
				if (_reactorHandleTunnelManagerRet(pReactorImpl, pReactorChannel, ret, pError) != RSSL_RET_SUCCESS)
					return ret;
			}

			return RSSL_RET_SUCCESS;
		default:
			rsslSetErrorInfo(pError, RSSL_EIC_FAILURE, RSSL_RET_FAILURE, __FILE__, __LINE__, "Unknown return code %d from callback.", cret);
			return RSSL_RET_FAILURE;
	}
}

static RsslRet _reactorEncodeRDMAsRsslMsg(RsslReactorImpl *pReactorImpl, RsslReactorChannelImpl *pReactorChannel, RsslRDMMsg *pRdmMsg, RsslMsg *pRsslMsg,
		RsslErrorInfo *pErrorInfo)
{
	RsslEncodeIterator 	encodeIter;
	RsslDecodeIterator	decodeIter;
	RsslRet				ret;
	RsslBuffer			memBuffer;

	/* Encode message to a buffer. */
	rsslClearEncodeIterator(&encodeIter);
	rsslSetEncodeIteratorRWFVersion(&encodeIter, pReactorChannel->reactorChannel.majorVersion,
			pReactorChannel->reactorChannel.minorVersion);

	memBuffer = pReactorImpl->memoryBuffer;
	rsslSetEncodeIteratorBuffer(&encodeIter, &memBuffer);

	if ((ret = rsslEncodeRDMMsg(&encodeIter, pRdmMsg, &memBuffer.length, pErrorInfo))
			!= RSSL_RET_SUCCESS)
		return ret;

	/* Decode as RsslMsg. */
	rsslClearDecodeIterator(&decodeIter);
	rsslSetDecodeIteratorRWFVersion(&decodeIter, pReactorChannel->reactorChannel.majorVersion,
			pReactorChannel->reactorChannel.minorVersion);

	rsslSetDecodeIteratorBuffer(&decodeIter, &memBuffer);

	if ((ret = rsslDecodeMsg(&decodeIter, pRsslMsg)) != RSSL_RET_SUCCESS)
		return ret;
	
	return RSSL_RET_SUCCESS;
}

/* Calls the defaultMsgCallback. */
static void _reactorCallDefaultCallback(RsslReactorImpl *pReactorImpl, RsslReactorChannelImpl *pReactorChannel, ReactorProcessMsgOptions *pOpts)
{
	RsslMsgEvent msgEvent;
	rsslClearMsgEvent(&msgEvent);
	msgEvent.pRsslMsgBuffer = pOpts->pMsgBuf;
	msgEvent.pRsslMsg = pOpts->pRsslMsg;
	msgEvent.pStreamInfo = (RsslStreamInfo*)pOpts->pStreamInfo;
	msgEvent.pFTGroupId = pOpts->pFTGroupId;
	msgEvent.pSeqNum = pOpts->pSeqNum;

	_reactorSetInCallback(pReactorImpl, RSSL_TRUE);
	*pOpts->pCret = (*pReactorChannel->channelRole.base.defaultMsgCallback)((RsslReactor*)pReactorImpl, (RsslReactorChannel*)pReactorChannel, &msgEvent);
	_reactorSetInCallback(pReactorImpl, RSSL_FALSE);
}

/* Raises an RDM message from a domain-specific callback to a defaultMsgCallback.
 * If the watchlist is enabled, may need to encode an RsslMsg to give to the defaultMsgCallback. */
static RsslRet _reactorRaiseToDefaultCallback(RsslReactorImpl *pReactorImpl, RsslReactorChannelImpl *pReactorChannel, ReactorProcessMsgOptions *pOpts)
{
	RsslMsg rsslMsg;
	RsslRet ret;

	if (pReactorChannel->pWatchlist != NULL)
	{
		/* Dictionary messages come out of the watchlist as RsslMsgs,
		 * and the reactor already provides an RsslRDMDictionaryMsg.
		 * So no need to reencode. */
		if (pOpts->pRdmMsg != NULL && pOpts->pRdmMsg->rdmMsgBase.domainType != RSSL_DMT_DICTIONARY)
		{
			assert(pOpts->pRsslMsg == NULL);
			if ((ret = _reactorEncodeRDMAsRsslMsg(pReactorImpl, pReactorChannel, pOpts->pRdmMsg, &rsslMsg,
				pOpts->pError)) != RSSL_RET_SUCCESS)
				return ret;

			pOpts->pRsslMsg = &rsslMsg;
		}
		else
			assert(pOpts->pRsslMsg != NULL);
	}

	_reactorCallDefaultCallback(pReactorImpl, pReactorChannel, pOpts);

	return RSSL_RET_SUCCESS;
}

static RsslRet _reactorProcessMsg(RsslReactorImpl *pReactorImpl, RsslReactorChannelImpl *pReactorChannel, ReactorProcessMsgOptions *pOpts)
{
	RsslChannel				*pChannel = pReactorChannel->reactorChannel.pRsslChannel;
	RsslRet					ret;
	RsslBuffer				*pMsgBuf = pOpts->pMsgBuf;
	RsslMsg					*pMsg = pOpts->pRsslMsg;
	RsslRDMMsg				*pRdmMsg = pOpts->pRdmMsg;
	RsslErrorInfo			*pError = pOpts->pError;
	RsslStreamInfo			*pStreamInfo = pOpts->pStreamInfo;
	RsslReactorCallbackRet	*pCret = pOpts->pCret;
	RsslDecodeIterator		dIter;

	/* check for RsslTunnelStream message */
	if (pReactorChannel->pTunnelManager && pMsg)
	{
		*pCret = RSSL_RC_CRET_SUCCESS;
		ret = tunnelManagerReadMsg(pReactorChannel->pTunnelManager, pMsg, pError);
		if (ret == RSSL_RET_NO_TUNNEL_STREAM)
			/* Non-tunnel message, so fall through. */;
		else
			return _reactorHandleTunnelManagerRet(pReactorImpl, pReactorChannel, ret, pError);
	}

	switch(pReactorChannel->channelRole.base.roleType)
	{
		case RSSL_RC_RT_OMM_CONSUMER:
		{
			RsslUInt8 domainType;
			assert(pMsg || pRdmMsg);

			/* When the watchlist is enabled, some messages may be provided as RDMMsg's. */
			domainType = pMsg ? pMsg->msgBase.domainType : pRdmMsg->rdmMsgBase.domainType;

			switch(domainType)
			{

				case RSSL_DMT_LOGIN:
				{
					RsslReactorOMMConsumerRole *pConsumerRole = &pReactorChannel->channelRole.ommConsumerRole;

					if (pConsumerRole->loginMsgCallback)
					{
						RsslBuffer memoryBuffer = pReactorImpl->memoryBuffer;
						RsslRDMLoginMsg loginResponse, *pLoginResponse;
						RsslRDMLoginMsgEvent loginEvent;

						if (!pRdmMsg)
						{
							rsslClearRDMLoginMsg(&loginResponse);
							rsslClearRDMLoginMsgEvent(&loginEvent);

							rsslClearDecodeIterator(&dIter);
							rsslSetDecodeIteratorRWFVersion(&dIter, pReactorChannel->reactorChannel.majorVersion, pReactorChannel->reactorChannel.minorVersion);
							rsslSetDecodeIteratorBuffer(&dIter, &pMsg->msgBase.encDataBody);
							ret = rsslDecodeRDMLoginMsg(&dIter, pMsg, &loginResponse, &memoryBuffer, pError);
							pLoginResponse = &loginResponse;
						}
						else
						{
							ret = RSSL_RET_SUCCESS;
							pLoginResponse = &pRdmMsg->loginMsg;
						}

						loginEvent.baseMsgEvent.pRsslMsgBuffer = pMsgBuf;
						loginEvent.baseMsgEvent.pRsslMsg = pMsg;
						loginEvent.baseMsgEvent.pStreamInfo = (RsslStreamInfo*)pStreamInfo;
						loginEvent.baseMsgEvent.pFTGroupId = pOpts->pFTGroupId;
						loginEvent.baseMsgEvent.pSeqNum = pOpts->pSeqNum;


						if (ret == RSSL_RET_SUCCESS)
						{
							loginEvent.pRDMLoginMsg = pLoginResponse;
							_reactorSetInCallback(pReactorImpl, RSSL_TRUE);
							*pCret = (*pConsumerRole->loginMsgCallback)((RsslReactor*)pReactorImpl, (RsslReactorChannel*)pReactorChannel, &loginEvent);
							_reactorSetInCallback(pReactorImpl, RSSL_FALSE);
						}
						else if (pMsg->msgBase.msgClass == RSSL_MC_GENERIC)
						{
							/* Let unrecognized generic messages through to the default callback. */
							ret = RSSL_RET_SUCCESS;
							*pCret = RSSL_RC_CRET_RAISE;
						}
						else
						{
							loginEvent.baseMsgEvent.pErrorInfo = pError;
							_reactorSetInCallback(pReactorImpl, RSSL_TRUE);
							*pCret = (*pConsumerRole->loginMsgCallback)((RsslReactor*)pReactorImpl, (RsslReactorChannel*)pReactorChannel, &loginEvent);
							_reactorSetInCallback(pReactorImpl, RSSL_FALSE);
						}
					} 
					else
					{
						ret = RSSL_RET_SUCCESS;
						*pCret = RSSL_RC_CRET_RAISE;
					}

					if (*pCret == RSSL_RC_CRET_RAISE)
						ret = _reactorRaiseToDefaultCallback(pReactorImpl, pReactorChannel, pOpts);

					/* Message failed to decode or callback didn't handle it */
					if (ret != RSSL_RET_SUCCESS || *pCret != RSSL_RC_CRET_SUCCESS || pReactorChannel->reactorParentQueue == &pReactorImpl->closingChannels)
						break;

					if (pReactorChannel->pWatchlist)
						break;

					/* If watchlist is not in use(otherwise it would handle this), check that login succeeded and send channel's source directory request. */

					if (pReactorChannel->channelSetupState == RSSL_RC_CHST_INIT && *pCret == RSSL_RC_CRET_SUCCESS && pConsumerRole->pLoginRequest)
					{
						RsslState *pState;

						switch(pMsg->msgBase.msgClass)
						{
							case RSSL_MC_REFRESH: 
								pState = &pMsg->refreshMsg.state;
								break;
							case RSSL_MC_STATUS: 
								pState = (pMsg->statusMsg.flags & RSSL_STMF_HAS_STATE) ? &pMsg->statusMsg.state : NULL;
								break;
							case RSSL_MC_POST:
								return RSSL_RET_SUCCESS;
							case RSSL_MC_ACK:
								return RSSL_RET_SUCCESS;
							default: 
								rsslSetErrorInfo(pError, RSSL_EIC_FAILURE, RSSL_RET_FAILURE, __FILE__, __LINE__, "Received unexpected msg class %u from login domain, received while setting up channel", pMsg->msgBase.msgClass);
								if (_reactorHandleChannelDown(pReactorImpl, pReactorChannel, pError) != RSSL_RET_SUCCESS)
									return RSSL_RET_FAILURE;
								return RSSL_RET_SUCCESS;
						}

						if (pMsg->msgBase.streamId == pConsumerRole->pLoginRequest->rdmMsgBase.streamId && pState) /* Matches request and has state */
						{
							if (pState->streamState == RSSL_STREAM_OPEN)
							{
								if (pState->dataState == RSSL_DATA_OK)
								{
									if (pConsumerRole->pDirectoryRequest)
									{
										/* Successfully logged in, send source directory request if any. */
										if ((ret = _reactorSendRDMMessage(pReactorImpl, pReactorChannel, (RsslRDMMsg*)pConsumerRole->pDirectoryRequest, pError)) != RSSL_RET_SUCCESS)
										{
											if (_reactorHandleChannelDown(pReactorImpl, pReactorChannel, pError) != RSSL_RET_SUCCESS)
												return RSSL_RET_FAILURE;
											return RSSL_RET_SUCCESS;
										}
										else
										{
											pReactorChannel->channelSetupState = RSSL_RC_CHST_LOGGED_IN;
										}
									}
									else if (pReactorChannel->channelSetupState != RSSL_RC_CHST_READY)
									{
										/* Nothing to send. Put CHANNEL_READY event on our own queue. */
										if (_reactorSendConnReadyEvent(pReactorImpl, pReactorChannel, pError) != RSSL_RET_SUCCESS)
											return RSSL_RET_FAILURE;
									}
									break;
								}
								/* Else the state was Open/Suspect. Keep waiting -- this is okay to get but we don't want to do anything else until we get Open/Ok. */
							}
							else
							{
								/* Login is closed before we could set up. shutdown the channel. */
								rsslSetErrorInfo(pError, RSSL_EIC_FAILURE, RSSL_RET_FAILURE, __FILE__, __LINE__, "Received login msg with stream state %s while setting up channel", rsslStreamStateToString(pState->streamState));
								if (_reactorHandleChannelDown(pReactorImpl, pReactorChannel, pError) != RSSL_RET_SUCCESS)
									return RSSL_RET_FAILURE;
							}
						}
					}
					break;
				}

				case RSSL_DMT_SOURCE:
				{
					RsslReactorOMMConsumerRole *pConsumerRole = &pReactorChannel->channelRole.ommConsumerRole;
					RsslRDMDirectoryMsg directoryResponse, *pDirectoryResponse;

					if (pConsumerRole->directoryMsgCallback || pReactorChannel->channelSetupState == RSSL_RC_CHST_LOGGED_IN && pConsumerRole->dictionaryDownloadMode == RSSL_RC_DICTIONARY_DOWNLOAD_FIRST_AVAILABLE)
					{
						if (!pRdmMsg)
						{
							RsslBuffer memoryBuffer = pReactorImpl->memoryBuffer;
							rsslClearRDMDirectoryMsg(&directoryResponse);

							rsslClearDecodeIterator(&dIter);
							rsslSetDecodeIteratorRWFVersion(&dIter, pReactorChannel->reactorChannel.majorVersion, pReactorChannel->reactorChannel.minorVersion);
							rsslSetDecodeIteratorBuffer(&dIter, &pMsg->msgBase.encDataBody);
							ret = rsslDecodeRDMDirectoryMsg(&dIter, pMsg, &directoryResponse, &memoryBuffer, pError);
							pDirectoryResponse = &directoryResponse;
						}
						else
						{
							pDirectoryResponse = &pRdmMsg->directoryMsg;
							ret = RSSL_RET_SUCCESS;
						}
					}
					else
						ret = RSSL_RET_SUCCESS;

					if (pConsumerRole->directoryMsgCallback)
					{
						RsslRDMDirectoryMsgEvent directoryEvent;
						rsslClearRDMDirectoryMsgEvent(&directoryEvent);

						directoryEvent.baseMsgEvent.pRsslMsgBuffer = pMsgBuf;
						directoryEvent.baseMsgEvent.pRsslMsg = pMsg;
						directoryEvent.baseMsgEvent.pStreamInfo = (RsslStreamInfo*)pStreamInfo;
						directoryEvent.baseMsgEvent.pFTGroupId = pOpts->pFTGroupId;
						directoryEvent.baseMsgEvent.pSeqNum = pOpts->pSeqNum;

						if (ret == RSSL_RET_SUCCESS)
						{
							directoryEvent.pRDMDirectoryMsg = pDirectoryResponse;
							_reactorSetInCallback(pReactorImpl, RSSL_TRUE);
							*pCret = (*pConsumerRole->directoryMsgCallback)((RsslReactor*)pReactorImpl, (RsslReactorChannel*)pReactorChannel, &directoryEvent);
							_reactorSetInCallback(pReactorImpl, RSSL_FALSE);
						}
						else if (pMsg->msgBase.msgClass == RSSL_MC_GENERIC)
						{
							/* Let unrecognized generic messages through to the default callback. */
							ret = RSSL_RET_SUCCESS;
							*pCret = RSSL_RC_CRET_RAISE;
						}
						else
						{
							directoryEvent.baseMsgEvent.pErrorInfo = pError;
							_reactorSetInCallback(pReactorImpl, RSSL_TRUE);
							*pCret = (*pConsumerRole->directoryMsgCallback)((RsslReactor*)pReactorImpl, (RsslReactorChannel*)pReactorChannel, &directoryEvent);
							_reactorSetInCallback(pReactorImpl, RSSL_FALSE);
						}
					}
					else
					{
						ret = RSSL_RET_SUCCESS;
						*pCret = RSSL_RC_CRET_RAISE;
					}
					
					if (*pCret == RSSL_RC_CRET_RAISE)
						ret = _reactorRaiseToDefaultCallback(pReactorImpl, pReactorChannel, pOpts);

					/* Message failed to decode or callback didn't handle it */
					if (ret != RSSL_RET_SUCCESS || *pCret != RSSL_RC_CRET_SUCCESS || pReactorChannel->reactorParentQueue == &pReactorImpl->closingChannels)
						break;

					if (pReactorChannel->pWatchlist)
						break;

					/* If watchlist is not in use(otherwise it would handle this), send channel's dictionary requests. */

					if (pConsumerRole->dictionaryDownloadMode == RSSL_RC_DICTIONARY_DOWNLOAD_FIRST_AVAILABLE)
					{
						if (pReactorChannel->channelSetupState == RSSL_RC_CHST_LOGGED_IN)
						{
							RsslState *pState;
							RsslRDMService *pServiceList;
							RsslUInt32 serviceCount;
							RsslRDMDirectoryRequest *pRequest = pReactorChannel->channelRole.ommConsumerRole.pDirectoryRequest;

							switch(directoryResponse.rdmMsgBase.rdmMsgType)
							{
								case RDM_DR_MT_REFRESH:
									pServiceList = directoryResponse.refresh.serviceList; 
									serviceCount = directoryResponse.refresh.serviceCount; 
									pState = &directoryResponse.refresh.state;
									break;
								case RDM_DR_MT_UPDATE: 
									pServiceList = directoryResponse.update.serviceList; 
									serviceCount = directoryResponse.update.serviceCount; 
									pState = NULL; 
									break;
								case RDM_DR_MT_STATUS: 
									pServiceList = NULL; 
									pState = (directoryResponse.status.flags & RDM_DR_STF_HAS_STATE) ? &directoryResponse.status.state : NULL;
									break;
								default: 
									pServiceList = NULL;
									pState = NULL; 
									rsslSetErrorInfo(pError, RSSL_EIC_FAILURE, RSSL_RET_FAILURE, __FILE__, __LINE__, "Received directory msg with stream state %s while setting up channel", rsslStreamStateToString(pState->streamState));
									if (_reactorHandleChannelDown(pReactorImpl, pReactorChannel, pError) != RSSL_RET_SUCCESS)
										return RSSL_RET_FAILURE;
									return RSSL_RET_SUCCESS;
							}

							if (ret == RSSL_RC_CRET_SUCCESS
									&& pRequest && directoryResponse.rdmMsgBase.streamId == pRequest->rdmMsgBase.streamId /* Matches request */
									&& pServiceList /* Has serviceList */) 
							{
								RsslUInt32 i;
								/* Check directory message for service that has RWFFld and RWFEnum available */


								/* Find unused streams we can use for requesting the field dictionary and enum type dictionary.  
								 * They must be unique from the login and directory streams, but will only be open
								 * temporarily -- they will be closed before the user is notified the channel is ready for use. */


								for(i = 0; i < serviceCount; ++i)
								{
									RsslRDMService *pService = &pServiceList[i];
									RsslBuffer fieldDictionaryName = { 6, (char*)"RWFFld" };
									RsslBuffer enumTypeDictionaryName = { 7, (char*)"RWFEnum" };
									RsslUInt32 j;

									RsslBool hasFieldDictionary = RSSL_FALSE;
									RsslBool hasEnumTypeDictionary = RSSL_FALSE;

									for(j = 0; j < pService->info.dictionariesProvidedCount; ++j)
									{
										if (rsslBufferIsEqual(&pService->info.dictionariesProvidedList[j], &fieldDictionaryName))
											hasFieldDictionary = RSSL_TRUE;
										else if (rsslBufferIsEqual(&pService->info.dictionariesProvidedList[j], &enumTypeDictionaryName))
											hasEnumTypeDictionary = RSSL_TRUE;
									}

									if (hasFieldDictionary && hasEnumTypeDictionary)
									{
										RsslRDMDictionaryRequest dictionaryRequest;

										pReactorChannel->rwfFldStreamId = 1; 
										while (pReactorChannel->rwfFldStreamId == pConsumerRole->pLoginRequest->rdmMsgBase.streamId
												|| pReactorChannel->rwfFldStreamId == pConsumerRole->pDirectoryRequest->rdmMsgBase.streamId)
											++pReactorChannel->rwfFldStreamId;

										pReactorChannel->rwfEnumStreamId = 2;
										while (pReactorChannel->rwfEnumStreamId == pConsumerRole->pLoginRequest->rdmMsgBase.streamId
												|| pReactorChannel->rwfEnumStreamId == pConsumerRole->pDirectoryRequest->rdmMsgBase.streamId
												|| pReactorChannel->rwfEnumStreamId == pReactorChannel->rwfFldStreamId)
											++pReactorChannel->rwfEnumStreamId;


										rsslClearRDMDictionaryRequest(&dictionaryRequest);
										dictionaryRequest.flags = RDM_DC_RQF_STREAMING;
										dictionaryRequest.serviceId = (RsslUInt16)pService->serviceId;
										dictionaryRequest.verbosity = RDM_DICTIONARY_NORMAL;

										dictionaryRequest.rdmMsgBase.streamId = pReactorChannel->rwfFldStreamId;
										dictionaryRequest.dictionaryName = fieldDictionaryName;

										/* Send field dictionary request */
										if ((ret = _reactorSendRDMMessage(pReactorImpl, pReactorChannel, (RsslRDMMsg*)&dictionaryRequest, pError)) != RSSL_RET_SUCCESS)
										{
											if (_reactorHandleChannelDown(pReactorImpl, pReactorChannel, pError) != RSSL_RET_SUCCESS)
												return RSSL_RET_FAILURE;
											return RSSL_RET_SUCCESS;
										}
										else
										{
											/* Send enumType dictionary request */
											dictionaryRequest.dictionaryName = enumTypeDictionaryName;
											dictionaryRequest.rdmMsgBase.streamId = pReactorChannel->rwfEnumStreamId;
											if ((ret = _reactorSendRDMMessage(pReactorImpl, pReactorChannel, (RsslRDMMsg*)&dictionaryRequest, pError)) != RSSL_RET_SUCCESS)
											{
												if (_reactorHandleChannelDown(pReactorImpl, pReactorChannel, pError) != RSSL_RET_SUCCESS)
													return RSSL_RET_FAILURE;
												return RSSL_RET_SUCCESS;
											}
											else
												pReactorChannel->channelSetupState = RSSL_RC_CHST_HAVE_DIRECTORY;
										}
										break;
									}
									else
									{
										if (pState && pState->streamState != RSSL_STREAM_OPEN)
										{
											/* Directory stream was closed without providing a service with the desired dictionaries. Shutdown channel. */
											rsslSetErrorInfo(pError, RSSL_EIC_FAILURE, RSSL_RET_FAILURE, __FILE__, __LINE__, "Received directory msg with stream state %s while setting up channel", rsslStreamStateToString(pState->streamState));
											if (_reactorHandleChannelDown(pReactorImpl, pReactorChannel, pError) != RSSL_RET_SUCCESS)
												return RSSL_RET_FAILURE;
										}
									}
								}
							}
						}
					}
					else if (pReactorChannel->channelSetupState != RSSL_RC_CHST_READY)
					{
						/* Nothing more to send. Put CHANNEL_READY event on our own queue. */
						if (_reactorSendConnReadyEvent(pReactorImpl, pReactorChannel, pError) != RSSL_RET_SUCCESS)
							return RSSL_RET_FAILURE;
					}
					break;
				}

				case RSSL_DMT_DICTIONARY:
				{
					RsslReactorOMMConsumerRole *pConsumerRole = &pReactorChannel->channelRole.ommConsumerRole;

					if (pConsumerRole->dictionaryMsgCallback)
					{
						RsslBuffer memoryBuffer = pReactorImpl->memoryBuffer;
						RsslRDMDictionaryMsg dictionaryResponse;
						RsslRDMDictionaryMsgEvent dictionaryEvent;
						rsslClearRDMDictionaryMsg(&dictionaryResponse);
						rsslClearRDMDictionaryMsgEvent(&dictionaryEvent);

						rsslClearDecodeIterator(&dIter);
						rsslSetDecodeIteratorRWFVersion(&dIter, pReactorChannel->reactorChannel.majorVersion, pReactorChannel->reactorChannel.minorVersion);
						rsslSetDecodeIteratorBuffer(&dIter, &pMsg->msgBase.encDataBody);
						ret = rsslDecodeRDMDictionaryMsg(&dIter, pMsg, &dictionaryResponse, &memoryBuffer, pError);

						/* Avoid passing these when the watchlist is enabled; the message may be modified from what was sent by the provider. */
						if (!pConsumerRole->watchlistOptions.enableWatchlist)
						{
							dictionaryEvent.baseMsgEvent.pRsslMsgBuffer = pMsgBuf;
							dictionaryEvent.baseMsgEvent.pRsslMsg = pMsg;
						}

						dictionaryEvent.baseMsgEvent.pStreamInfo = (RsslStreamInfo*)pStreamInfo;
						dictionaryEvent.baseMsgEvent.pFTGroupId = pOpts->pFTGroupId;
						dictionaryEvent.baseMsgEvent.pSeqNum = pOpts->pSeqNum;
						if (ret == RSSL_RET_SUCCESS) dictionaryEvent.pRDMDictionaryMsg = &dictionaryResponse;
						else dictionaryEvent.baseMsgEvent.pErrorInfo = pError;

						_reactorSetInCallback(pReactorImpl, RSSL_TRUE);
						*pCret = (*pConsumerRole->dictionaryMsgCallback)((RsslReactor*)pReactorImpl, (RsslReactorChannel*)pReactorChannel, &dictionaryEvent);
						_reactorSetInCallback(pReactorImpl, RSSL_FALSE);
					}
					else
					{
						ret = RSSL_RET_SUCCESS;
						*pCret = RSSL_RC_CRET_RAISE;
					}

					if (*pCret == RSSL_RC_CRET_RAISE)
						ret = _reactorRaiseToDefaultCallback(pReactorImpl, pReactorChannel, pOpts);

					/* Message failed to decode or callback didn't handle it */
					if (ret != RSSL_RET_SUCCESS || *pCret != RSSL_RC_CRET_SUCCESS || pReactorChannel->reactorParentQueue == &pReactorImpl->closingChannels)
						break;

					if (pConsumerRole->dictionaryDownloadMode == RSSL_RC_DICTIONARY_DOWNLOAD_FIRST_AVAILABLE && pReactorChannel->channelSetupState >= RSSL_RC_CHST_HAVE_DIRECTORY
							&& pReactorChannel->channelSetupState < RSSL_RC_CHST_READY
							&& pMsg->msgBase.msgClass == RSSL_MC_REFRESH && pMsg->refreshMsg.flags & RSSL_RFMF_REFRESH_COMPLETE)
					{
						if (pMsg->msgBase.streamId == pReactorChannel->rwfFldStreamId)
						{
							/* Close stream so its streamID is free for the user.  When connecting to an ADS,  there won't be any further messages on this stream --
							 * the consumer will be disconnected if the dictionary version is changed. */
							RsslRDMDictionaryClose dictionaryClose;

							rsslClearRDMDictionaryClose(&dictionaryClose);
							dictionaryClose.rdmMsgBase.streamId = pReactorChannel->rwfFldStreamId;
							if ((ret = _reactorSendRDMMessage(pReactorImpl, pReactorChannel, (RsslRDMMsg*)&dictionaryClose, pError)) != RSSL_RET_SUCCESS)
							{
								if (_reactorHandleChannelDown(pReactorImpl, pReactorChannel, pError) != RSSL_RET_SUCCESS)
									return RSSL_RET_FAILURE;
								return RSSL_RET_SUCCESS;
							}

							pReactorChannel->channelSetupState = (pReactorChannel->channelSetupState == RSSL_RC_CHST_HAVE_RWFENUM) ? RSSL_RC_CHST_READY : RSSL_RC_CHST_HAVE_RWFFLD;
						}
						else if (pMsg->msgBase.streamId == pReactorChannel->rwfEnumStreamId)
						{
							/* Close stream so its streamID is free for the user.  When connecting to an ADS,  there won't be any further messages on this stream --
							 * the consumer will be disconnected if the dictionary version is changed. */
							RsslRDMDictionaryClose dictionaryClose;

							rsslClearRDMDictionaryClose(&dictionaryClose);
							dictionaryClose.rdmMsgBase.streamId = pReactorChannel->rwfEnumStreamId;
							if ((ret = _reactorSendRDMMessage(pReactorImpl, pReactorChannel, (RsslRDMMsg*)&dictionaryClose, pError)) != RSSL_RET_SUCCESS)
							{
								if (_reactorHandleChannelDown(pReactorImpl, pReactorChannel, pError) != RSSL_RET_SUCCESS)
									return RSSL_RET_FAILURE;
								return RSSL_RET_SUCCESS;
							}

							pReactorChannel->channelSetupState = (pReactorChannel->channelSetupState == RSSL_RC_CHST_HAVE_RWFFLD) ? RSSL_RC_CHST_READY : RSSL_RC_CHST_HAVE_RWFENUM;
						}

						/* If dictionary refreshes have been received, channel is ready */
						if (pReactorChannel->channelSetupState == RSSL_RC_CHST_READY)
						{
							/* Nothing more to send. Put CHANNEL_READY event on our own queue. */
							if (_reactorSendConnReadyEvent(pReactorImpl, pReactorChannel, pError) != RSSL_RET_SUCCESS)
								return RSSL_RET_FAILURE;
						}
					}
					break;
				}

				default:
					_reactorCallDefaultCallback(pReactorImpl, pReactorChannel, pOpts);
					break;

			}
			break;
		}

		case RSSL_RC_RT_OMM_NI_PROVIDER:
			assert(pMsg);
			switch(pMsg->msgBase.domainType)
			{

				case RSSL_DMT_LOGIN:
				{
					RsslReactorOMMNIProviderRole *pNIProviderRole = &pReactorChannel->channelRole.ommNIProviderRole;

					if (pNIProviderRole->loginMsgCallback)
					{
						RsslBuffer memoryBuffer = pReactorImpl->memoryBuffer;
						RsslRDMLoginMsg loginResponse;
						RsslRDMLoginMsgEvent loginEvent;

						rsslClearRDMLoginMsg(&loginResponse);
						rsslClearRDMLoginMsgEvent(&loginEvent);

						rsslClearDecodeIterator(&dIter);
						rsslSetDecodeIteratorRWFVersion(&dIter, pReactorChannel->reactorChannel.majorVersion, pReactorChannel->reactorChannel.minorVersion);
						rsslSetDecodeIteratorBuffer(&dIter, &pMsg->msgBase.encDataBody);
						ret = rsslDecodeRDMLoginMsg(&dIter, pMsg, &loginResponse, &memoryBuffer, pError);

						loginEvent.baseMsgEvent.pRsslMsgBuffer = pMsgBuf;
						loginEvent.baseMsgEvent.pRsslMsg = pMsg;
						if (ret == RSSL_RET_SUCCESS) loginEvent.pRDMLoginMsg = &loginResponse;
						else loginEvent.baseMsgEvent.pErrorInfo = pError;

						_reactorSetInCallback(pReactorImpl, RSSL_TRUE);
						*pCret = (*pNIProviderRole->loginMsgCallback)((RsslReactor*)pReactorImpl, (RsslReactorChannel*)pReactorChannel, &loginEvent);
						_reactorSetInCallback(pReactorImpl, RSSL_FALSE);
					}
					else
					{
						ret = RSSL_RET_SUCCESS;
						*pCret = RSSL_RC_CRET_RAISE;
					}

					if (*pCret == RSSL_RC_CRET_RAISE)
						ret = _reactorRaiseToDefaultCallback(pReactorImpl, pReactorChannel, pOpts);

					/* Message failed to decode or callback didn't handle it */
					if (ret != RSSL_RET_SUCCESS || *pCret != RSSL_RC_CRET_SUCCESS || pReactorChannel->reactorParentQueue == &pReactorImpl->closingChannels)
						break;

					if (pReactorChannel->channelSetupState == RSSL_RC_CHST_INIT && *pCret == RSSL_RC_CRET_SUCCESS && pNIProviderRole->pLoginRequest)
					{
						RsslState *pState;

						switch(pMsg->msgBase.msgClass)
						{
							case RSSL_MC_REFRESH: 
								pState = &pMsg->refreshMsg.state;
								break;
							case RSSL_MC_STATUS: 
								pState = (pMsg->statusMsg.flags & RSSL_STMF_HAS_STATE) ? &pMsg->statusMsg.state : NULL;
								break;
							case RSSL_MC_POST:
								return RSSL_RET_SUCCESS;
							case RSSL_MC_ACK:
								return RSSL_RET_SUCCESS;
							default: 
								rsslSetErrorInfo(pError, RSSL_EIC_FAILURE, RSSL_RET_FAILURE, __FILE__, __LINE__, "Received unexpected msg class %u from login domain, received while setting up channel", pMsg->msgBase.msgClass);
								if (_reactorHandleChannelDown(pReactorImpl, pReactorChannel, pError) != RSSL_RET_SUCCESS)
									return RSSL_RET_FAILURE;
								return RSSL_RET_SUCCESS;
						}

						if (pMsg->msgBase.streamId == pNIProviderRole->pLoginRequest->rdmMsgBase.streamId && pState) /* Matches request and has state */
						{
							if (pState->streamState == RSSL_STREAM_OPEN)
							{
								if (pState->dataState == RSSL_DATA_OK)
								{
									if (pNIProviderRole->pDirectoryRefresh)
									{
										/* Successfully logged in, send source directory request if any. */
										if ((ret = _reactorSendRDMMessage(pReactorImpl, pReactorChannel, (RsslRDMMsg*)pNIProviderRole->pDirectoryRefresh, pError)) != RSSL_RET_SUCCESS)
										{
											if (_reactorHandleChannelDown(pReactorImpl, pReactorChannel, pError) != RSSL_RET_SUCCESS)
												return RSSL_RET_FAILURE;
											return RSSL_RET_SUCCESS;
										}
										else
										{
											pReactorChannel->channelSetupState = RSSL_RC_CHST_LOGGED_IN;
										}
									}

									/* Whether we sent a refresh or not, we are now ready. So put event on own queue. */
									if (pReactorChannel->channelSetupState != RSSL_RC_CHST_READY)
									{
										if (_reactorSendConnReadyEvent(pReactorImpl, pReactorChannel, pError) != RSSL_RET_SUCCESS)
											return RSSL_RET_FAILURE;
									}
									break;
								}
								/* Else the state was Open/Suspect. Keep waiting -- this is okay to get but we don't want to do anything else until we get Open/Ok. */
							}
							else
							{
								/* Login is closed before we could set up. shutdown the channel. */
								rsslSetErrorInfo(pError, RSSL_EIC_FAILURE, RSSL_RET_FAILURE, __FILE__, __LINE__, "Received login msg with stream state %s while setting up channel", rsslStreamStateToString(pState->streamState));
								if (_reactorHandleChannelDown(pReactorImpl, pReactorChannel, pError) != RSSL_RET_SUCCESS)
									return RSSL_RET_FAILURE;
							}
						}
					}
					break;
				}

				default:
				{
					_reactorCallDefaultCallback(pReactorImpl, pReactorChannel, pOpts);

					break;
				}
			}
			break;

		case RSSL_RC_RT_OMM_PROVIDER:
		{
			RsslReactorOMMProviderRole *pProviderRole = &pReactorChannel->channelRole.ommProviderRole;

			assert(pMsg);
			switch(pMsg->msgBase.domainType)
			{
				case RSSL_DMT_LOGIN:
				{

					if (pProviderRole->loginMsgCallback)
					{
						RsslBuffer memoryBuffer = pReactorImpl->memoryBuffer;
						RsslRDMLoginMsg loginResponse;
						RsslRDMLoginMsgEvent loginEvent;

						rsslClearRDMLoginMsg(&loginResponse);
						rsslClearRDMLoginMsgEvent(&loginEvent);

						rsslClearDecodeIterator(&dIter);
						rsslSetDecodeIteratorRWFVersion(&dIter, pReactorChannel->reactorChannel.majorVersion, pReactorChannel->reactorChannel.minorVersion);
						rsslSetDecodeIteratorBuffer(&dIter, &pMsg->msgBase.encDataBody);
						ret = rsslDecodeRDMLoginMsg(&dIter, pMsg, &loginResponse, &memoryBuffer, pError);

						loginEvent.baseMsgEvent.pRsslMsgBuffer = pMsgBuf;
						loginEvent.baseMsgEvent.pRsslMsg = pMsg;
						if (ret == RSSL_RET_SUCCESS) loginEvent.pRDMLoginMsg = &loginResponse;
						else loginEvent.baseMsgEvent.pErrorInfo = pError;

							_reactorSetInCallback(pReactorImpl, RSSL_TRUE);
							*pCret = (*pProviderRole->loginMsgCallback)((RsslReactor*)pReactorImpl, (RsslReactorChannel*)pReactorChannel, &loginEvent);
							_reactorSetInCallback(pReactorImpl, RSSL_FALSE);
						}
					else
					{
						ret = RSSL_RET_SUCCESS;
						*pCret = RSSL_RC_CRET_RAISE;
					}

					if (*pCret == RSSL_RC_CRET_RAISE)
						_reactorCallDefaultCallback(pReactorImpl, pReactorChannel, pOpts);
					
					break;
				}

				case RSSL_DMT_SOURCE:
				{
					if (pProviderRole->directoryMsgCallback)
					{
						RsslBuffer memoryBuffer = pReactorImpl->memoryBuffer;
						RsslRDMDirectoryMsg directoryResponse;
						RsslRDMDirectoryMsgEvent directoryEvent;

						rsslClearRDMDirectoryMsg(&directoryResponse);
						rsslClearRDMDirectoryMsgEvent(&directoryEvent);

						rsslClearDecodeIterator(&dIter);
						rsslSetDecodeIteratorRWFVersion(&dIter, pReactorChannel->reactorChannel.majorVersion, pReactorChannel->reactorChannel.minorVersion);
						rsslSetDecodeIteratorBuffer(&dIter, &pMsg->msgBase.encDataBody);
						ret = rsslDecodeRDMDirectoryMsg(&dIter, pMsg, &directoryResponse, &memoryBuffer, pError);

						directoryEvent.baseMsgEvent.pRsslMsgBuffer = pMsgBuf;
						directoryEvent.baseMsgEvent.pRsslMsg = pMsg;
						if (ret == RSSL_RET_SUCCESS) directoryEvent.pRDMDirectoryMsg = &directoryResponse;
						else directoryEvent.baseMsgEvent.pErrorInfo = pError;

							_reactorSetInCallback(pReactorImpl, RSSL_TRUE);
							*pCret = (*pProviderRole->directoryMsgCallback)((RsslReactor*)pReactorImpl, (RsslReactorChannel*)pReactorChannel, &directoryEvent);
							_reactorSetInCallback(pReactorImpl, RSSL_FALSE);
					}
					else
					{
						ret = RSSL_RET_SUCCESS;
						*pCret = RSSL_RC_CRET_RAISE;
					}
					
					if (*pCret == RSSL_RC_CRET_RAISE)
						_reactorCallDefaultCallback(pReactorImpl, pReactorChannel, pOpts);

					break;
				}

				case RSSL_DMT_DICTIONARY:
				{
					if (pProviderRole->dictionaryMsgCallback)
					{
						RsslBuffer memoryBuffer = pReactorImpl->memoryBuffer;
						RsslRDMDictionaryMsg dictionaryResponse;
						RsslRDMDictionaryMsgEvent dictionaryEvent;

						rsslClearRDMDictionaryMsg(&dictionaryResponse);
						rsslClearRDMDictionaryMsgEvent(&dictionaryEvent);

						rsslClearDecodeIterator(&dIter);
						rsslSetDecodeIteratorRWFVersion(&dIter, pReactorChannel->reactorChannel.majorVersion, pReactorChannel->reactorChannel.minorVersion);
						rsslSetDecodeIteratorBuffer(&dIter, &pMsg->msgBase.encDataBody);
						ret = rsslDecodeRDMDictionaryMsg(&dIter, pMsg, &dictionaryResponse, &memoryBuffer, pError);

						dictionaryEvent.baseMsgEvent.pRsslMsgBuffer = pMsgBuf;
						dictionaryEvent.baseMsgEvent.pRsslMsg = pMsg;
						if (ret == RSSL_RET_SUCCESS) dictionaryEvent.pRDMDictionaryMsg = &dictionaryResponse;
						else dictionaryEvent.baseMsgEvent.pErrorInfo = pError;

						_reactorSetInCallback(pReactorImpl, RSSL_TRUE);
						*pCret = (*pProviderRole->dictionaryMsgCallback)((RsslReactor*)pReactorImpl, (RsslReactorChannel*)pReactorChannel, &dictionaryEvent);
						_reactorSetInCallback(pReactorImpl, RSSL_FALSE);
					}
					else
					{
						ret = RSSL_RET_SUCCESS;
						*pCret = RSSL_RC_CRET_RAISE;
					}

					if (*pCret == RSSL_RC_CRET_RAISE)
						_reactorCallDefaultCallback(pReactorImpl, pReactorChannel, pOpts);
					
					break;
				}

				default:
					_reactorCallDefaultCallback(pReactorImpl, pReactorChannel, pOpts);
					break;
				
			}
			break;
		}

		default:
			rsslSetErrorInfo(pError, RSSL_EIC_FAILURE, RSSL_RET_FAILURE, __FILE__, __LINE__, "Unknown roleType:%d", pReactorChannel->channelRole.base.roleType);
			return RSSL_RET_FAILURE;
	}

	return RSSL_RET_SUCCESS;
}

static RsslRet _reactorWatchlistMsgCallback(RsslWatchlist *pWatchlist, RsslWatchlistMsgEvent *pEvent, RsslErrorInfo *pError)
{
	RsslReactorChannelImpl *pReactorChannel = (RsslReactorChannelImpl*)pWatchlist->pUserSpec;
	RsslReactorImpl *pReactorImpl = pReactorChannel->pParentReactor;
	RsslReactorCallbackRet cret;
	ReactorProcessMsgOptions processOpts;

	processOpts.pMsgBuf = NULL;
	processOpts.pStreamInfo = (RsslStreamInfo*)pEvent->pStreamInfo;
	processOpts.pCret = &cret;
	processOpts.pError = pError;
	processOpts.pSeqNum = pEvent->pSeqNum;
	processOpts.pFTGroupId = pEvent->pFTGroupId;

	if (pEvent->pRdmMsg)
	{
		processOpts.pRdmMsg = pEvent->pRdmMsg;
		processOpts.pRsslMsg = NULL;
	}
	else
	{
		processOpts.pRdmMsg = NULL;
		processOpts.pRsslMsg = pEvent->pRsslMsg;
	}

	return _reactorProcessMsg(pReactorImpl, pReactorChannel, &processOpts);

}

static RsslRet _reactorDispatchFromChannel(RsslReactorImpl *pReactorImpl, RsslReactorChannelImpl *pReactorChannel, RsslErrorInfo *pError)
{
	RsslRet ret;
	RsslBuffer *pMsgBuf;
	RsslChannel *pChannel = pReactorChannel->reactorChannel.pRsslChannel;
	RsslReactorCallbackRet cret;

	RsslReadInArgs	readInArgs;
	RsslReadOutArgs	readOutArgs;

	rsslClearReadInArgs(&readInArgs);
	rsslClearReadOutArgs(&readOutArgs);
	pMsgBuf = rsslReadEx(pChannel, &readInArgs, &readOutArgs, &ret, &pError->rsslError);
	pReactorChannel->readRet = ret;

	if (pMsgBuf)
	{
		RsslDecodeIterator dIter;
		RsslMsg msg;

		/* Update ping time & notication logic */
		pReactorChannel->lastPingReadMs = pReactorImpl->lastRecordedTimeMs;

		if (ret == RSSL_RET_SUCCESS) 
			rsslNotifierEventClearNotifiedFlags(pReactorChannel->pNotifierEvent); /* Done reading. */

		/* Decode the message header. Call the appropriate callback function based on the domainType. */
		rsslClearMsg(&msg);
		rsslClearDecodeIterator(&dIter);
		rsslSetDecodeIteratorRWFVersion(&dIter, pReactorChannel->reactorChannel.pRsslChannel->majorVersion, pReactorChannel->reactorChannel.pRsslChannel->minorVersion);
		rsslSetDecodeIteratorBuffer(&dIter, pMsgBuf);
		ret = rsslDecodeMsg(&dIter, &msg);

		if (ret == RSSL_RET_SUCCESS)
		{
			if (pReactorChannel->pWatchlist)
			{
				/* Pass messsage to watchlist for processing. */
				RsslWatchlistProcessMsgOptions wlProcessOpts;

				rsslWatchlistClearProcessMsgOptions(&wlProcessOpts);
				wlProcessOpts.pChannel = pReactorChannel->reactorChannel.pRsslChannel;
				wlProcessOpts.pDecodeIterator = &dIter;
				wlProcessOpts.pRsslBuffer = pMsgBuf;
				wlProcessOpts.pRsslMsg = &msg;

				if (readOutArgs.readOutFlags & RSSL_READ_OUT_FTGROUP_ID)
					wlProcessOpts.pFTGroupId = &readOutArgs.FTGroupId;

				if (readOutArgs.readOutFlags & RSSL_READ_OUT_SEQNUM)
					wlProcessOpts.pSeqNum = &readOutArgs.seqNum;

				if ((ret = _reactorReadWatchlistMsg(pReactorImpl, pReactorChannel, &wlProcessOpts, pError))
						< RSSL_RET_SUCCESS)
					return ret;

				cret = RSSL_RC_CRET_SUCCESS; 
			}
			else
			{
				/* No watchlist -- message goes directly through Reactor. */
				ReactorProcessMsgOptions processOpts;

				processOpts.pMsgBuf = pMsgBuf;
				processOpts.pStreamInfo = NULL;
				processOpts.pCret = &cret;
				processOpts.pError = pError;
				processOpts.pRsslMsg = &msg;
				processOpts.pRdmMsg = NULL;

				processOpts.pFTGroupId = 
					(readOutArgs.readOutFlags & RSSL_READ_OUT_FTGROUP_ID) ? 
					&readOutArgs.FTGroupId : NULL;

				processOpts.pSeqNum = 
					(readOutArgs.readOutFlags & RSSL_READ_OUT_SEQNUM) ?
					&readOutArgs.seqNum : NULL;

				if ((ret = _reactorProcessMsg(pReactorImpl, pReactorChannel, &processOpts))
						!= RSSL_RET_SUCCESS)
					return ret;
			}
		}
		else
		{
			RsslMsgEvent msgEvent;
			rsslClearMsgEvent(&msgEvent);
			msgEvent.pRsslMsgBuffer = pMsgBuf;
			msgEvent.pRsslMsg = NULL;
			msgEvent.pErrorInfo = pError;

			rsslSetErrorInfo(pError, RSSL_EIC_FAILURE, RSSL_RET_FAILURE, __FILE__, __LINE__, "rsslDecodeMsg() failed: %d", ret);
			cret = (*pReactorChannel->channelRole.base.defaultMsgCallback)((RsslReactor*)pReactorImpl, (RsslReactorChannel*)pReactorChannel, &msgEvent);
		}


		switch(cret)
		{
			case RSSL_RC_CRET_SUCCESS:
				return RSSL_RET_SUCCESS;
			default:
				rsslSetErrorInfo(pError, RSSL_EIC_FAILURE, RSSL_RET_FAILURE, __FILE__, __LINE__, "Unknown return code %d from callback.", cret);
				return RSSL_RET_FAILURE;
		}
	}
	else if (ret < 0)
	{
		switch(ret)
		{
			case RSSL_RET_READ_PING:
				/* Update ping time */
				pReactorChannel->lastPingReadMs = pReactorImpl->lastRecordedTimeMs;
				pReactorChannel->readRet = 0;

				if (pReactorChannel->pWatchlist && readOutArgs.readOutFlags 
						& RSSL_READ_OUT_FTGROUP_ID)
				{
					RsslInt64 nextExpireTime = rsslWatchlistProcessFTGroupPing(pReactorChannel->pWatchlist,
							readOutArgs.FTGroupId, pReactorImpl->lastRecordedTimeMs);

					if (pReactorChannel->pWatchlist->state & RSSLWL_STF_NEED_TIMER) 
						if (_reactorSetTimer(pReactorImpl, pReactorChannel, nextExpireTime, pError) 
								!= RSSL_RET_SUCCESS)
							return ret;
				}

				return RSSL_RET_SUCCESS;
			case RSSL_RET_READ_WOULD_BLOCK:
				/* Clear descriptor so that we will ignore this channel until the next notification call */
				rsslNotifierEventClearNotifiedFlags(pReactorChannel->pNotifierEvent);
				pReactorChannel->readRet = 0;
				return RSSL_RET_SUCCESS;
			case RSSL_RET_READ_FD_CHANGE:
				{
					RsslReactorEventImpl rsslEvent;

					/* Descriptor changed, update notification. */
					if (rsslNotifierUpdateEventFd(pReactorImpl->pNotifier, pReactorChannel->pNotifierEvent, pChannel->socketId) < 0)
					{
						rsslSetErrorInfo(pError, RSSL_EIC_FAILURE, RSSL_RET_FAILURE, __FILE__, __LINE__, 
								"Failed to add notification event for initializing channel that is changing descriptor.");
						return RSSL_RET_FAILURE;
					}
					if (rsslNotifierRegisterRead(pReactorImpl->pNotifier, pReactorChannel->pNotifierEvent) < 0)
					{
						rsslSetErrorInfo(pError, RSSL_EIC_FAILURE, RSSL_RET_FAILURE, __FILE__, __LINE__, 
								"Failed to register read notification for initializing channel that is changing descriptor.");
						return RSSL_RET_FAILURE;
					}

					/* Copy changed sockets to reactor channel */
					pReactorChannel->reactorChannel.socketId = pChannel->socketId;
					pReactorChannel->reactorChannel.oldSocketId = pChannel->oldSocketId;

					/* Inform client of change */
					rsslClearReactorChannelEventImpl(&rsslEvent.channelEventImpl);
					rsslEvent.channelEventImpl.channelEvent.channelEventType = RSSL_RC_CET_FD_CHANGE;
					rsslEvent.channelEventImpl.channelEvent.pReactorChannel = (RsslReactorChannel*)pReactorChannel;
					
					_reactorSetInCallback(pReactorImpl, RSSL_TRUE);
					(*pReactorChannel->channelRole.base.channelEventCallback)((RsslReactor*)pReactorImpl, (RsslReactorChannel*)pReactorChannel, &rsslEvent.channelEventImpl.channelEvent);
					_reactorSetInCallback(pReactorImpl, RSSL_FALSE);


					pReactorChannel->readRet = 0;

					return RSSL_RET_SUCCESS;
				}
			case RSSL_RET_CONGESTION_DETECTED:
			case RSSL_RET_SLOW_READER:
			case RSSL_RET_PACKET_GAP_DETECTED:
			{
				/* Depending on configuration, these returns may or may not result in channel failure.
				 * If the channel state is still active, pass the event to the client as a warning.
				 * Otherwise treat it as a normal channel down. */
				if (pChannel->state == RSSL_CH_STATE_ACTIVE)
				{
					RsslReactorEventImpl rsslEvent;

					/* Congestion, Slow Reader, and Gap Detected are indications of traffic problems,
					 * so if the consumer watchlist is in use, delay any gap recovery. */
					if (pReactorChannel->pWatchlist)
					{
						rsslWatchlistResetGapTimer(pReactorChannel->pWatchlist);
						if (pReactorChannel->pWatchlist->state & RSSLWL_STF_NEED_TIMER) 
							if (_reactorSetTimer(pReactorImpl, pReactorChannel, rsslWatchlistGetNextTimeout(pReactorChannel->pWatchlist),
										pError) != RSSL_RET_SUCCESS)
								return ret;
					}

					rsslClearReactorChannelEventImpl(&rsslEvent.channelEventImpl);
					rsslEvent.channelEventImpl.channelEvent.channelEventType = RSSL_RC_CET_WARNING;
					rsslEvent.channelEventImpl.channelEvent.pReactorChannel = (RsslReactorChannel*)pReactorChannel;
					rsslEvent.channelEventImpl.channelEvent.pError = pError;
					rsslSetErrorInfoLocation(pError, __FILE__, __LINE__);
					
					_reactorSetInCallback(pReactorImpl, RSSL_TRUE);
					(*pReactorChannel->channelRole.base.channelEventCallback)((RsslReactor*)pReactorImpl, (RsslReactorChannel*)pReactorChannel, &rsslEvent.channelEventImpl.channelEvent);
					_reactorSetInCallback(pReactorImpl, RSSL_FALSE);


					pReactorChannel->readRet = 0;

					return RSSL_RET_SUCCESS;
				}
				else
				{
					rsslSetErrorInfoLocation(pError, __FILE__, __LINE__);
					if (_reactorHandleChannelDown(pReactorImpl, pReactorChannel, pError) != RSSL_RET_SUCCESS)
						return RSSL_RET_FAILURE;
					pReactorChannel->readRet = 0;
					return RSSL_RET_SUCCESS; /* Problem handled, so return success */
				}
			}
			default:
				rsslSetErrorInfoLocation(pError, __FILE__, __LINE__);
				if (_reactorHandleChannelDown(pReactorImpl, pReactorChannel, pError) != RSSL_RET_SUCCESS)
					return RSSL_RET_FAILURE;
				pReactorChannel->readRet = 0;
				return RSSL_RET_SUCCESS; /* Problem handled, so return success */
		}
	}
	else
	{
		pReactorChannel->lastPingReadMs = pReactorImpl->lastRecordedTimeMs;
		return ret; /* Got positive return from read, but no message -- means a partial message was received by Rssl and we have to wait for the rest of the data. */
	}
}

RsslRet reactorLockInterface(RsslReactorImpl *pReactorImpl, RsslBool allowedInCallback, RsslErrorInfo *pError)
{
	if (!allowedInCallback && pReactorImpl->inReactorFunction)
	{
		rsslSetErrorInfo(pError, RSSL_EIC_FAILURE, RSSL_RET_INVALID_ARGUMENT, __FILE__, __LINE__, "This call cannot be called from within a callback");
		return RSSL_RET_INVALID_ARGUMENT;
	}

	RSSL_MUTEX_LOCK(&pReactorImpl->interfaceLock);
	return RSSL_RET_SUCCESS;
}

RSSL_VA_API RsslRet rsslReactorOpenTunnelStream(RsslReactorChannel *pReactorChannel, RsslTunnelStreamOpenOptions *pOptions, RsslErrorInfo *pError)
{
	RsslReactorChannelImpl *pReactorChannelImpl;
	RsslReactorImpl *pReactorImpl;
	RsslTunnelStream *pTunnelStream;
	TunnelStreamImpl *pTunnelStreamImpl;
	RsslRet ret;

	if (pReactorChannel == NULL)
	{
		rsslSetErrorInfo(pError, RSSL_EIC_FAILURE, RSSL_RET_INVALID_ARGUMENT, __FILE__, __LINE__, "RsslReactorChannel not provided.");
		return RSSL_RET_INVALID_ARGUMENT;
	}

	if (pOptions == NULL)
	{
		rsslSetErrorInfo(pError, RSSL_EIC_FAILURE, RSSL_RET_INVALID_ARGUMENT, __FILE__, __LINE__, "RsslTunnelStreamOpenOptions not provided.");
		return RSSL_RET_INVALID_ARGUMENT;
	}

	if (pError == NULL)
		return RSSL_RET_INVALID_ARGUMENT;

	pReactorChannelImpl = (RsslReactorChannelImpl*)pReactorChannel;
	pReactorImpl = (RsslReactorImpl*)pReactorChannelImpl->pParentReactor;

	if ((ret = reactorLockInterface(pReactorImpl, RSSL_TRUE, pError)) != RSSL_RET_SUCCESS)
		return ret;

	if ((pTunnelStream = tunnelManagerOpenStream(pReactorChannelImpl->pTunnelManager, pOptions, RSSL_FALSE, NULL, COS_CURRENT_STREAM_VERSION, pError)) == NULL)
		return (reactorUnlockInterface(pReactorImpl), pError->rsslError.rsslErrorId);

	pTunnelStream->pReactorChannel = pReactorChannel;
	pTunnelStreamImpl = (TunnelStreamImpl *)pTunnelStream;

	if (_reactorHandleTunnelManagerEvents(pReactorImpl, pReactorChannelImpl, pError) != RSSL_RET_SUCCESS)
		return (reactorUnlockInterface(pReactorImpl), pError->rsslError.rsslErrorId);

	return (reactorUnlockInterface(pReactorImpl), RSSL_RET_SUCCESS);
}

RSSL_VA_API RsslRet rsslReactorCloseTunnelStream(RsslTunnelStream *pTunnelStream, RsslTunnelStreamCloseOptions *pOptions, RsslErrorInfo *pError)
{
	RsslRet ret;
	RsslReactorChannelImpl *pReactorChannelImpl;
	RsslReactorImpl *pReactorImpl;
	TunnelStreamImpl *pTunnelStreamImpl;

	if (pTunnelStream == NULL)
	{
		rsslSetErrorInfo(pError, RSSL_EIC_FAILURE, RSSL_RET_INVALID_ARGUMENT, __FILE__, __LINE__, "RsslTunnelStream not provided.");
		return RSSL_RET_INVALID_ARGUMENT;
	}

	if (pError == NULL)
		return RSSL_RET_INVALID_ARGUMENT;

	pReactorChannelImpl = (RsslReactorChannelImpl*)pTunnelStream->pReactorChannel;
	pTunnelStreamImpl = (TunnelStreamImpl*)pTunnelStream;
	pReactorImpl = (RsslReactorImpl*)pReactorChannelImpl->pParentReactor;
	
	if ((ret = reactorLockInterface(pReactorImpl, RSSL_TRUE, pError)) != RSSL_RET_SUCCESS)
		return ret;

	if ((ret = tunnelManagerCloseStream(pReactorChannelImpl->pTunnelManager, pTunnelStream, pOptions, pError)) != RSSL_RET_SUCCESS)
		return (reactorUnlockInterface(pReactorImpl), ret);

	if ((ret = _reactorHandleTunnelManagerEvents(pReactorImpl, pReactorChannelImpl, pError)) != RSSL_RET_SUCCESS)
		return (reactorUnlockInterface(pReactorImpl), ret);

	return (reactorUnlockInterface(pReactorImpl), RSSL_RET_SUCCESS);
}

RSSL_VA_API RsslRet rsslTunnelStreamSubmit(RsslTunnelStream *pTunnelStream, RsslBuffer *pBuffer, RsslTunnelStreamSubmitOptions *pRsslTunnelStreamSubmitOptions, RsslErrorInfo *pError)
{
	RsslRet ret;
	TunnelStreamImpl *pTunnelStreamImpl;
	TunnelManagerImpl *pTunnelManagerImpl;
	RsslReactorChannelImpl *pReactorChannelImpl;
	RsslReactorImpl *pReactorImpl;

	if (pTunnelStream == NULL)
	{
		rsslSetErrorInfo(pError, RSSL_EIC_FAILURE, RSSL_RET_INVALID_ARGUMENT, __FILE__, __LINE__, "RsslTunnelStream not provided.");
		return RSSL_RET_INVALID_ARGUMENT;
	}

	if (pBuffer == NULL)
	{
		rsslSetErrorInfo(pError, RSSL_EIC_FAILURE, RSSL_RET_INVALID_ARGUMENT, __FILE__, __LINE__, "RsslBuffer not provided.");
		return RSSL_RET_INVALID_ARGUMENT;
	}

	if (pRsslTunnelStreamSubmitOptions == NULL)
	{
		rsslSetErrorInfo(pError, RSSL_EIC_FAILURE, RSSL_RET_INVALID_ARGUMENT, __FILE__, __LINE__, "RsslTunnelStreamSubmitOptions not provided.");
		return RSSL_RET_INVALID_ARGUMENT;
	}

	if (pError == NULL)
		return RSSL_RET_INVALID_ARGUMENT;

	pTunnelStreamImpl = (TunnelStreamImpl *)pTunnelStream;
	pTunnelManagerImpl = pTunnelStreamImpl->_manager;
	pReactorChannelImpl = (RsslReactorChannelImpl *)pTunnelManagerImpl->base._pReactorChannel;
	pReactorImpl = (RsslReactorImpl*)pReactorChannelImpl->pParentReactor;

	if ((ret = reactorLockInterface(pReactorImpl, RSSL_TRUE, pError)) != RSSL_RET_SUCCESS)
		return ret;

	if ((ret = tunnelManagerSubmitBuffer((TunnelManager *)pTunnelManagerImpl, pTunnelStream, pBuffer, pRsslTunnelStreamSubmitOptions, pError))
				!= RSSL_RET_SUCCESS)
		return (reactorUnlockInterface(pReactorChannelImpl->pParentReactor), ret);

	if ((ret = _reactorHandleTunnelManagerEvents(pReactorImpl, pReactorChannelImpl, pError)) != RSSL_RET_SUCCESS)
		return (reactorUnlockInterface(pReactorImpl), ret);

	return (reactorUnlockInterface(pReactorChannelImpl->pParentReactor), RSSL_RET_SUCCESS);
}

RSSL_VA_API RsslRet rsslReactorAcceptTunnelStream(RsslTunnelStreamRequestEvent *pEvent, RsslReactorAcceptTunnelStreamOptions *pOptions, RsslErrorInfo *pError)
{
	RsslReactorChannelImpl *pReactorChannelImpl;
	RsslReactorImpl *pReactorImpl;
	RsslRet ret;

	if (pEvent == NULL)
	{
		rsslSetErrorInfo(pError, RSSL_EIC_FAILURE, RSSL_RET_INVALID_ARGUMENT, __FILE__, __LINE__, "RsslTunnelStreamRequestEvent not provided.");
		return RSSL_RET_INVALID_ARGUMENT;
	}

	if (pOptions == NULL)
	{
		rsslSetErrorInfo(pError, RSSL_EIC_FAILURE, RSSL_RET_INVALID_ARGUMENT, __FILE__, __LINE__, "RsslReactorAcceptTunnelStreamOptions not provided.");
		return RSSL_RET_INVALID_ARGUMENT;
	}

	if (pError == NULL)
		return RSSL_RET_INVALID_ARGUMENT;

	pReactorChannelImpl = (RsslReactorChannelImpl*)pEvent->pReactorChannel;
	pReactorImpl = (RsslReactorImpl*)pReactorChannelImpl->pParentReactor;
	
	if ((ret = reactorLockInterface(pReactorImpl, RSSL_TRUE, pError)) != RSSL_RET_SUCCESS)
		return ret;


	if ((ret = tunnelManagerAcceptStream(pReactorChannelImpl->pTunnelManager, pEvent, pOptions, pError)) != RSSL_RET_SUCCESS)
		return (reactorUnlockInterface(pReactorImpl), ret);

	// send a dispatch tunnel stream event if necessary
	if (pReactorChannelImpl->pTunnelManager->_needsDispatchNow && !pReactorChannelImpl->tunnelDispatchEventQueued)
	{
		RsslReactorChannelEventImpl *pDispatchEvent;
		pReactorChannelImpl->tunnelDispatchEventQueued = RSSL_TRUE;
		pDispatchEvent = (RsslReactorChannelEventImpl*)rsslReactorEventQueueGetFromPool(&pReactorChannelImpl->eventQueue);
		rsslClearReactorChannelEventImpl(pDispatchEvent);
		pDispatchEvent->channelEvent.channelEventType = (RsslReactorChannelEventType)RSSL_RCIMPL_CET_DISPATCH_TUNNEL_STREAM;
		pDispatchEvent->channelEvent.pReactorChannel = (RsslReactorChannel*)pReactorChannelImpl;
		ret = rsslReactorEventQueuePut(&pReactorChannelImpl->eventQueue, (RsslReactorEventImpl*)pDispatchEvent);
		return (reactorUnlockInterface(pReactorImpl), ret);
	}

	return (reactorUnlockInterface(pReactorImpl), RSSL_RET_SUCCESS);
}

RSSL_VA_API RsslRet rsslReactorRejectTunnelStream(RsslTunnelStreamRequestEvent *pEvent, RsslReactorRejectTunnelStreamOptions *pOptions, RsslErrorInfo *pError)
{
	RsslReactorChannelImpl *pReactorChannelImpl;
	RsslReactorImpl *pReactorImpl;
	RsslRet ret;

	if (pEvent == NULL)
	{
		rsslSetErrorInfo(pError, RSSL_EIC_FAILURE, RSSL_RET_INVALID_ARGUMENT, __FILE__, __LINE__, "RsslTunnelStreamRequestEvent not provided.");
		return RSSL_RET_INVALID_ARGUMENT;
	}

	if (pOptions == NULL)
	{
		rsslSetErrorInfo(pError, RSSL_EIC_FAILURE, RSSL_RET_INVALID_ARGUMENT, __FILE__, __LINE__, "RsslReactorRejectTunnelStreamOptions not provided.");
		return RSSL_RET_INVALID_ARGUMENT;
	}

	if (pError == NULL)
		return RSSL_RET_INVALID_ARGUMENT;

	pReactorChannelImpl = (RsslReactorChannelImpl*)pEvent->pReactorChannel;
	pReactorImpl = (RsslReactorImpl*)pReactorChannelImpl->pParentReactor;

	if ((ret = reactorLockInterface(pReactorImpl, RSSL_TRUE, pError)) != RSSL_RET_SUCCESS)
		return ret;

	ret = tunnelManagerRejectStream(pReactorChannelImpl->pTunnelManager, pEvent, pOptions, pError);

	return (reactorUnlockInterface(pReactorImpl), ret);
}

RSSL_VA_API RsslRet rsslTunnelStreamSubmitMsg(RsslTunnelStream *pTunnelStream, RsslTunnelStreamSubmitMsgOptions *pRsslTunnelStreamSubmitMsgOptions, RsslErrorInfo *pError)
{
	RsslRet ret;
	TunnelStreamImpl *pTunnelStreamImpl;
	TunnelManagerImpl *pTunnelManagerImpl;
	RsslReactorChannelImpl *pReactorChannelImpl;
	RsslReactorImpl *pReactorImpl;

	if (pTunnelStream == NULL)
	{
		rsslSetErrorInfo(pError, RSSL_EIC_FAILURE, RSSL_RET_INVALID_ARGUMENT, __FILE__, __LINE__, "RsslTunnelStream not provided.");
		return RSSL_RET_INVALID_ARGUMENT;
	}

	if (pRsslTunnelStreamSubmitMsgOptions == NULL)
	{
		rsslSetErrorInfo(pError, RSSL_EIC_FAILURE, RSSL_RET_INVALID_ARGUMENT, __FILE__, __LINE__, "RsslTunnelStreamSubmitMsgOptions not provided.");
		return RSSL_RET_INVALID_ARGUMENT;
	}

	if (pError == NULL)
		return RSSL_RET_INVALID_ARGUMENT;

	pTunnelStreamImpl = (TunnelStreamImpl *)pTunnelStream;
	pTunnelManagerImpl = pTunnelStreamImpl->_manager;
	pReactorChannelImpl = (RsslReactorChannelImpl *)pTunnelManagerImpl->base._pReactorChannel;
	pReactorImpl = (RsslReactorImpl*)pReactorChannelImpl->pParentReactor;

	if ((ret = reactorLockInterface(pReactorImpl, RSSL_TRUE, pError)) != RSSL_RET_SUCCESS)
		return ret;

	if ((ret = tunnelManagerSubmit((TunnelManager *)pTunnelManagerImpl, pTunnelStream, pRsslTunnelStreamSubmitMsgOptions, pError)) != RSSL_RET_SUCCESS)
		return (reactorUnlockInterface(pReactorChannelImpl->pParentReactor), RSSL_RET_FAILURE);

	// send a dispatch tunnel stream event if necessary
	if ((ret = _reactorHandleTunnelManagerEvents(pReactorImpl, pReactorChannelImpl, pError)) != RSSL_RET_SUCCESS)
		return (reactorUnlockInterface(pReactorImpl), ret);

	return (reactorUnlockInterface(pReactorChannelImpl->pParentReactor), RSSL_RET_SUCCESS);
}

RsslRet reactorUnlockInterface(RsslReactorImpl *pReactorImpl)
{
	RSSL_MUTEX_UNLOCK(&pReactorImpl->interfaceLock);
	return RSSL_RET_SUCCESS;
}

static void _reactorSetInCallback(RsslReactorImpl *pReactorImpl, RsslBool inCallback)
{
	pReactorImpl->inReactorFunction = inCallback;
}


static RsslRet _reactorChannelCopyRole(RsslReactorChannelImpl *pReactorChannel, RsslReactorChannelRole *pRole, RsslErrorInfo *pError)
{
	RsslRet ret;

	pReactorChannel->channelRole = *pRole;

	switch(pReactorChannel->channelRole.base.roleType)
	{
		case RSSL_RC_RT_OMM_CONSUMER:
		{
			RsslReactorOMMConsumerRole *pConsRole = (RsslReactorOMMConsumerRole*)&pReactorChannel->channelRole;

			if (pConsRole->pLoginRequest
					&& (pConsRole->pLoginRequest =
						(RsslRDMLoginRequest*)rsslCreateRDMMsgCopy((RsslRDMMsg*)pConsRole->pLoginRequest, 256, &ret)) == NULL)
			{
				rsslSetErrorInfo(pError, RSSL_EIC_SUCCESS, RSSL_RET_SUCCESS, __FILE__, __LINE__, "Failed to copy ommConsumerRole login request.");
				return RSSL_RET_FAILURE;
			}

			if (pConsRole->pDirectoryRequest
					&& (pConsRole->pDirectoryRequest =
						(RsslRDMDirectoryRequest*)rsslCreateRDMMsgCopy((RsslRDMMsg*)pConsRole->pDirectoryRequest, 256, &ret)) == NULL)
			{
				if (pConsRole->pLoginRequest)
				{
					free(pConsRole->pLoginRequest);
					pConsRole->pLoginRequest = NULL;
				}

				rsslSetErrorInfo(pError, RSSL_EIC_SUCCESS, RSSL_RET_SUCCESS, __FILE__, __LINE__, "Failed to copy ommConsumerRole directory request.");
				return RSSL_RET_FAILURE;
			}
			break;
		}

		case RSSL_RC_RT_OMM_NI_PROVIDER:
		{
			RsslReactorOMMNIProviderRole *pNIProvRole = (RsslReactorOMMNIProviderRole*)&pReactorChannel->channelRole;

			if (pNIProvRole->pLoginRequest
					&& (pNIProvRole->pLoginRequest =
						(RsslRDMLoginRequest*)rsslCreateRDMMsgCopy((RsslRDMMsg*)pNIProvRole->pLoginRequest, 256, &ret)) == NULL)
			{
				rsslSetErrorInfo(pError, RSSL_EIC_SUCCESS, RSSL_RET_SUCCESS, __FILE__, __LINE__, "Failed to copy ommNIProviderRole login request.");
				return RSSL_RET_FAILURE;
			}

			if (pNIProvRole->pDirectoryRefresh
					&& (pNIProvRole->pDirectoryRefresh =
						(RsslRDMDirectoryRefresh*)rsslCreateRDMMsgCopy((RsslRDMMsg*)pNIProvRole->pDirectoryRefresh, 256, &ret)) == NULL)
			{
				if (pNIProvRole->pLoginRequest)
				{
					free(pNIProvRole->pLoginRequest);
					pNIProvRole->pLoginRequest = NULL;
				}

				rsslSetErrorInfo(pError, RSSL_EIC_SUCCESS, RSSL_RET_SUCCESS, __FILE__, __LINE__, "Failed to copy ommNIProviderRole directory refresh.");
				return RSSL_RET_FAILURE;
			}
			break;
		}

		default:
			break;
	}

	return RSSL_RET_SUCCESS;
}
