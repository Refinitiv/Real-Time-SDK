/*
 * This source code is provided under the Apache 2.0 license and is provided
 * AS IS with no warranty or guarantee of fit for purpose.  See the project's 
 * LICENSE.md for details. 
 * Copyright Thomson Reuters 2018. All rights reserved.
*/

#include "rtr/rsslReactorImpl.h"
#include "rtr/rsslReactorEventsImpl.h"
#include <stddef.h>

#include <assert.h>
#ifdef _WIN32
#include <windows.h>
#include <process.h>
#else
#include <sys/types.h>
#include <unistd.h>
#endif

/* Default notification timeout in milliseconds if the worker thread is not waiting on a new ping, reconnection, or connection timeout event. */
static const int defaultSelectTimeoutMs = 5000;

/* A factor for how long the worker thread should wait to send a ping() with regard to each channel's ping timeout. 
 * (this should be significantly less than 1 so we don't risk pinging too late, yet large enough
 * that we don't do too much unnecessary pinging) */
static const float pingIntervalFactor = 1.0f/3.0f;

/* Handles a newly connected channel and starts initializing it */
static RsslRet _reactorWorkerProcessNewChannel(RsslReactorImpl *pReactorImpl, RsslReactorChannelImpl *pReactorChannel);

/* Cleans up the worker thread */
static void _reactorWorkerHandleShutdownRequest(RsslReactorImpl *pReactorImpl, RsslErrorInfo *pErrorInfo);

/* Signals back to reactor that it has stopped flushing for the given channel */
static RsslRet _reactorWorkerSendFlushComplete(RsslReactorImpl *pReactorImpl, RsslReactorChannelImpl *pReactorChannel);

/* Handle failure in worker thread on a given channel. Reports failure to read thread. */
RsslRet _reactorWorkerHandleChannelFailure(RsslReactorImpl *pReactorImpl, RsslReactorChannelImpl *pReactorChannel, RsslErrorInfo *pError);

/* Handles a successfully initialized channel */
RsslRet _reactorWorkerProcessChannelUp(RsslReactorImpl *pReactorImpl, RsslReactorChannelImpl *pReactorChannel);

/* Recalculate the "ping interval." It is the maximum time the worker wait for notification, since it must check for pings
 * it may need to send. */
static void _reactorWorkerCalculateNextTimeout(RsslReactorImpl *pReactorImpl, RsslUInt32 newPingTimeoutSeconds);

static void _reactorWorkerMoveChannel(RsslQueue *pNewList, RsslReactorChannelImpl *pReactorChannel);

/* Shutdown the worker due to some error. Sends a request to the reactor and waits for response, then shuts down. */
void _reactorWorkerShutdown(RsslReactorImpl *pReactorImpl, RsslErrorInfo *pErrorInfo);

/* Shuts down in response to a request from the reactor */
void _reactorWorkerHandleShutdownRequest(RsslReactorImpl *pReactorImpl, RsslErrorInfo *pErrorInfo);

/* Cleans up any memory of copied RDMMsgs. */
static void _reactorWorkerFreeChannelRDMMsgs(RsslReactorChannelImpl *pReactorChannel);

/* Setup and start the worker thread (Should be called from rsslCreateReactor) */
RsslRet _reactorWorkerStart(RsslReactorImpl *pReactorImpl, RsslCreateReactorOptions *pReactorOptions, RsslErrorInfo *pError);

static RsslRet _reactorWorkerRegisterEventForRestClient(RsslReactorWorker* pReactorWorker, RsslReactorImpl *pReactorImpl);

static RsslRestRequestArgs* _reactorWorkerCreateRequestArgsForRefreshToken(RsslReactorChannelImpl *pReactorChannel, RsslErrorInfo* pError);

static void rsslRestResponseCallback(RsslRestResponse* restresponse, RsslRestResponseEvent* event);

static void rsslRestErrorCallback(RsslError* rsslError, RsslRestResponseEvent* event);

RsslRet _reactorWorkerStart(RsslReactorImpl *pReactorImpl, RsslCreateReactorOptions *pReactorOptions, RsslErrorInfo *pError)
{
	RsslReactorWorker *pReactorWorker = &pReactorImpl->reactorWorker;
	int eventQueueFd;

	if (rsslInitReactorEventQueueGroup(&pReactorImpl->reactorWorker.activeEventQueueGroup) != RSSL_RET_SUCCESS)
	{
		rsslSetErrorInfo(pError, RSSL_EIC_FAILURE, RSSL_RET_FAILURE, __FILE__, __LINE__, "Failed to initialize worker activeEventQueueGroup.");
		return RSSL_RET_FAILURE;
	}

	if (rsslInitReactorEventQueue(&pReactorImpl->reactorWorker.workerQueue, 10, &pReactorImpl->reactorWorker.activeEventQueueGroup) != RSSL_RET_SUCCESS)
	{
		rsslSetErrorInfo(pError, RSSL_EIC_FAILURE, RSSL_RET_FAILURE, __FILE__, __LINE__, "Failed to init worker event queue.");
		return RSSL_RET_FAILURE;
	}

	/* Prepare worker's channel lists */
	rsslInitQueue(&pReactorImpl->reactorWorker.initializingChannels);
	rsslInitQueue(&pReactorImpl->reactorWorker.activeChannels);
	rsslInitQueue(&pReactorImpl->reactorWorker.inactiveChannels);
	rsslInitQueue(&pReactorImpl->reactorWorker.reconnectingChannels);
	rsslInitQueue(&pReactorImpl->reactorWorker.disposableRestHandles);

	pReactorImpl->reactorWorker.pNotifier = rsslCreateNotifier(1024);
	if (pReactorImpl->reactorWorker.pNotifier == NULL)
	{
		rsslSetErrorInfo(pError, RSSL_EIC_FAILURE, RSSL_RET_FAILURE, __FILE__, __LINE__, "Failed to initialize notifier.");
		return RSSL_RET_FAILURE;
	}

	eventQueueFd = rsslGetEventQueueGroupSignalFD(&pReactorImpl->reactorWorker.activeEventQueueGroup);
	if ((pReactorWorker->pQueueNotifierEvent = rsslCreateNotifierEvent()) == NULL)
	{
		rsslSetErrorInfo(pError, RSSL_EIC_FAILURE, RSSL_RET_FAILURE, __FILE__, __LINE__, "Failed to create event queue notifier event.");
		return RSSL_RET_FAILURE;
	}

	if (rsslNotifierAddEvent(pReactorWorker->pNotifier, pReactorWorker->pQueueNotifierEvent, eventQueueFd, &pReactorWorker->workerQueue) < 0)
	{
		rsslSetErrorInfo(pError, RSSL_EIC_FAILURE, RSSL_RET_FAILURE, __FILE__, __LINE__, "Failed to add event queue notifier event.");
		return RSSL_RET_FAILURE;
	}

	if (rsslNotifierRegisterRead(pReactorWorker->pNotifier, pReactorWorker->pQueueNotifierEvent) < 0)
	{
		rsslSetErrorInfo(pError, RSSL_EIC_FAILURE, RSSL_RET_FAILURE, __FILE__, __LINE__, "Failed to register event queue notifier event for reading.");
		return RSSL_RET_FAILURE;
	}


	/* Start write reactor thread */
	if (RSSL_THREAD_START(&pReactorImpl->reactorWorker.thread, runReactorWorker, pReactorImpl) < 0)
	{
		rsslSetErrorInfo(pError, RSSL_EIC_FAILURE, RSSL_RET_FAILURE, __FILE__, __LINE__, "Failed to start reactorWorker.");
		return RSSL_RET_FAILURE;
	}

	return RSSL_RET_SUCCESS;
}

void _reactorWorkerCleanupReactor(RsslReactorImpl *pReactorImpl)
{
	RsslQueueLink *pLink;
	RsslReactorWorker *pReactorWorker = &pReactorImpl->reactorWorker;

	rsslCleanupReactorEventQueue(&pReactorImpl->reactorEventQueue);
	rsslCleanupReactorEventQueue(&pReactorImpl->reactorWorker.workerQueue);
	rsslCleanupReactorEventQueueGroup(&pReactorImpl->reactorWorker.activeEventQueueGroup);
	rsslCleanupReactorEventQueueGroup(&pReactorImpl->activeEventQueueGroup);
	while ((pLink = rsslQueueRemoveFirstLink(&pReactorImpl->channelPool)))
	{
		RsslReactorChannelImpl *pReactorChannel = RSSL_QUEUE_LINK_TO_OBJECT(RsslReactorChannelImpl, reactorQueueLink, pLink);
		_reactorWorkerFreeChannelRDMMsgs(pReactorChannel);
		_rsslChannelFreeConnectionList(pReactorChannel);
		rsslCleanupReactorEventQueue(&pReactorChannel->eventQueue);
		if (pReactorChannel->pNotifierEvent)
			rsslDestroyNotifierEvent(pReactorChannel->pNotifierEvent);
		if (pReactorChannel->pWorkerNotifierEvent)
			rsslDestroyNotifierEvent(pReactorChannel->pWorkerNotifierEvent);
		free(pReactorChannel);
	}
	while ((pLink = rsslQueueRemoveFirstLink(&pReactorImpl->initializingChannels)))
	{
		RsslReactorChannelImpl *pReactorChannel = RSSL_QUEUE_LINK_TO_OBJECT(RsslReactorChannelImpl, reactorQueueLink, pLink);
		_reactorWorkerFreeChannelRDMMsgs(pReactorChannel);
		_rsslChannelFreeConnectionList(pReactorChannel);
		rsslCleanupReactorEventQueue(&pReactorChannel->eventQueue);
		if (pReactorChannel->pWatchlist)
			rsslWatchlistDestroy(pReactorChannel->pWatchlist);
		if (pReactorChannel->pNotifierEvent)
			rsslDestroyNotifierEvent(pReactorChannel->pNotifierEvent);
		if (pReactorChannel->pWorkerNotifierEvent)
			rsslDestroyNotifierEvent(pReactorChannel->pWorkerNotifierEvent);
		free(pReactorChannel);
	}
	while ((pLink = rsslQueueRemoveFirstLink(&pReactorImpl->activeChannels)))
	{
		RsslReactorChannelImpl *pReactorChannel = RSSL_QUEUE_LINK_TO_OBJECT(RsslReactorChannelImpl, reactorQueueLink, pLink);
		_reactorWorkerFreeChannelRDMMsgs(pReactorChannel);
		_rsslChannelFreeConnectionList(pReactorChannel);
		rsslCleanupReactorEventQueue(&pReactorChannel->eventQueue);
		if (pReactorChannel->pWatchlist)
			rsslWatchlistDestroy(pReactorChannel->pWatchlist);
		if (pReactorChannel->pNotifierEvent)
			rsslDestroyNotifierEvent(pReactorChannel->pNotifierEvent);
		if (pReactorChannel->pWorkerNotifierEvent)
			rsslDestroyNotifierEvent(pReactorChannel->pWorkerNotifierEvent);
		free(pReactorChannel);
	}
	while ((pLink = rsslQueueRemoveFirstLink(&pReactorImpl->inactiveChannels)))
	{
		RsslReactorChannelImpl *pReactorChannel = RSSL_QUEUE_LINK_TO_OBJECT(RsslReactorChannelImpl, reactorQueueLink, pLink);
		_reactorWorkerFreeChannelRDMMsgs(pReactorChannel);
		_rsslChannelFreeConnectionList(pReactorChannel);
		rsslCleanupReactorEventQueue(&pReactorChannel->eventQueue);
		if (pReactorChannel->pWatchlist)
			rsslWatchlistDestroy(pReactorChannel->pWatchlist);
		if (pReactorChannel->pNotifierEvent)
			rsslDestroyNotifierEvent(pReactorChannel->pNotifierEvent);
		if (pReactorChannel->pWorkerNotifierEvent)
			rsslDestroyNotifierEvent(pReactorChannel->pWorkerNotifierEvent);
		free(pReactorChannel);
	}
	while ((pLink = rsslQueueRemoveFirstLink(&pReactorImpl->closingChannels)))
	{
		RsslReactorChannelImpl *pReactorChannel = RSSL_QUEUE_LINK_TO_OBJECT(RsslReactorChannelImpl, reactorQueueLink, pLink);
		_reactorWorkerFreeChannelRDMMsgs(pReactorChannel);
		_rsslChannelFreeConnectionList(pReactorChannel);
		rsslCleanupReactorEventQueue(&pReactorChannel->eventQueue);
		if (pReactorChannel->pWatchlist)
			rsslWatchlistDestroy(pReactorChannel->pWatchlist);
		if (pReactorChannel->pNotifierEvent)
			rsslDestroyNotifierEvent(pReactorChannel->pNotifierEvent);
		if (pReactorChannel->pWorkerNotifierEvent)
			rsslDestroyNotifierEvent(pReactorChannel->pWorkerNotifierEvent);
		free(pReactorChannel);
	}
	while ((pLink = rsslQueueRemoveFirstLink(&pReactorImpl->reconnectingChannels)))
	{
		RsslReactorChannelImpl *pReactorChannel = RSSL_QUEUE_LINK_TO_OBJECT(RsslReactorChannelImpl, reactorQueueLink, pLink);
		_reactorWorkerFreeChannelRDMMsgs(pReactorChannel);
		_rsslChannelFreeConnectionList(pReactorChannel);
		rsslCleanupReactorEventQueue(&pReactorChannel->eventQueue);
		if (pReactorChannel->pWatchlist)
			rsslWatchlistDestroy(pReactorChannel->pWatchlist);
		if (pReactorChannel->pNotifierEvent)
			rsslDestroyNotifierEvent(pReactorChannel->pNotifierEvent);
		if (pReactorChannel->pWorkerNotifierEvent)
			rsslDestroyNotifierEvent(pReactorChannel->pWorkerNotifierEvent);
		free(pReactorChannel);
	}

	if (pReactorImpl->pNotifier)
		rsslDestroyNotifier(pReactorImpl->pNotifier);

	if (pReactorWorker->pNotifier)
		rsslDestroyNotifier(pReactorWorker->pNotifier);

	if (pReactorImpl->pQueueNotifierEvent)
		rsslDestroyNotifierEvent(pReactorImpl->pQueueNotifierEvent);

	if (pReactorWorker->pQueueNotifierEvent)
		rsslDestroyNotifierEvent(pReactorWorker->pQueueNotifierEvent);

	if (pReactorImpl->memoryBuffer.data)
		free(pReactorImpl->memoryBuffer.data);

	RSSL_MUTEX_DESTROY(&pReactorImpl->interfaceLock);

	if (pReactorImpl->pRestClient)
	{
		RsslError rsslError;
		if (rsslDestroyRestClient(pReactorImpl->pRestClient, &rsslError))
		{
			rsslSetErrorInfo(&pReactorWorker->workerCerr, RSSL_EIC_FAILURE, RSSL_RET_FAILURE, __FILE__, __LINE__,
				"Failed to destroy RsslRestClient. Text: %s", rsslError.text);
		}

		/* Destroy the notifier for RsslRestClient to register to the Reactor worker's notifier */
		if (pReactorImpl->pWorkerNotifierEvent);
		rsslDestroyNotifierEvent(pReactorImpl->pWorkerNotifierEvent);

		if (rsslRestClientUninitialize(&rsslError))
		{
			rsslSetErrorInfo(&pReactorWorker->workerCerr, RSSL_EIC_FAILURE, RSSL_RET_FAILURE, __FILE__, __LINE__, 
				"Failed to uninitialize RsslRestClient. Text: %s", rsslError.text);
		}
	}

	/* For EDP token management and service discovery */
	free(pReactorImpl->accessTokenRespBuffer.data);
	free(pReactorImpl->tokenInformationBuffer.data);
	free(pReactorImpl->serviceDiscoveryRespBuffer.data);
	free(pReactorImpl->restServiceEndpointRespBuf.data);
	free(pReactorImpl->argumentsAndHeaders.data);
	free(pReactorImpl->tokenServiceURL.data);
	free(pReactorImpl->serviceDiscoveryURL.data);

	free(pReactorImpl);

	/* rsslInitialize/rsslUninitialize are reference counted, so decrement the reactor's call. */
	rsslUninitialize();
	return;
}


RsslRet _reactorWorkerProcessChannelUp(RsslReactorImpl *pReactorImpl, RsslReactorChannelImpl *pReactorChannel)
{
	RsslReactorChannelEventImpl *pEvent = (RsslReactorChannelEventImpl*)rsslReactorEventQueueGetFromPool(&pReactorChannel->eventQueue);
	RsslReactorWorker *pReactorWorker = &pReactorImpl->reactorWorker;

	/* Initialization succeeded. Send channel up event. */

	/* Remove read fd -- from now on the worker thread should mainly handle flushing */
	rsslNotifierUnregisterRead(pReactorWorker->pNotifier, pReactorChannel->pWorkerNotifierEvent);
	rsslNotifierUnregisterWrite(pReactorWorker->pNotifier, pReactorChannel->pWorkerNotifierEvent);

	_reactorWorkerMoveChannel(&pReactorWorker->activeChannels, pReactorChannel);
	_reactorWorkerCalculateNextTimeout(pReactorImpl, (RsslUInt32)(pReactorChannel->reactorChannel.pRsslChannel->pingTimeout * 1000 * pingIntervalFactor));
	pReactorChannel->lastPingSentMs = pReactorWorker->lastRecordedTimeMs;

	/* Copy RsslChannel parameters */
	pReactorChannel->reactorChannel.socketId = pReactorChannel->reactorChannel.pRsslChannel->socketId;
	pReactorChannel->reactorChannel.oldSocketId = pReactorChannel->reactorChannel.pRsslChannel->oldSocketId;
	pReactorChannel->reactorChannel.majorVersion = pReactorChannel->reactorChannel.pRsslChannel->majorVersion;
	pReactorChannel->reactorChannel.minorVersion = pReactorChannel->reactorChannel.pRsslChannel->minorVersion;
	pReactorChannel->reactorChannel.protocolType = pReactorChannel->reactorChannel.pRsslChannel->protocolType;


	rsslClearReactorChannelEventImpl(pEvent);
	pEvent->channelEvent.channelEventType = RSSL_RC_CET_CHANNEL_UP;
	pEvent->channelEvent.pReactorChannel = (RsslReactorChannel*)pReactorChannel;
	if (!RSSL_ERROR_INFO_CHECK(rsslReactorEventQueuePut(&pReactorChannel->eventQueue, (RsslReactorEventImpl*)pEvent) == RSSL_RET_SUCCESS, RSSL_RET_FAILURE, &pReactorWorker->workerCerr))
		return RSSL_RET_FAILURE;

	return RSSL_RET_SUCCESS;
}

RsslRet _reactorWorkerHandleChannelFailure(RsslReactorImpl *pReactorImpl, RsslReactorChannelImpl *pReactorChannel, RsslErrorInfo *pError)
{
	RsslReactorChannelEventImpl *pEvent = (RsslReactorChannelEventImpl*)rsslReactorEventQueueGetFromPool(&pReactorChannel->eventQueue);
	RsslReactorWorker *pReactorWorker = &pReactorImpl->reactorWorker;

	/* Need to indicate if the worker was attempting to connect or initialize the channel. */
	RsslBool isConnectFailure = (pReactorChannel->workerParentList == &pReactorWorker->initializingChannels || pReactorChannel->workerParentList == &pReactorWorker->reconnectingChannels);

	if(pReactorChannel->reactorChannel.pRsslChannel != 0 &&
	   pReactorChannel->reactorChannel.pRsslChannel->socketId != REACTOR_INVALID_SOCKET)
	{
		if (rsslNotifierRemoveEvent(pReactorWorker->pNotifier, pReactorChannel->pWorkerNotifierEvent) < 0)
		{
			rsslSetErrorInfo(&pReactorWorker->workerCerr, RSSL_EIC_FAILURE, RSSL_RET_FAILURE, __FILE__, __LINE__, 
					"Failed to remove channel from notifier.");
			return RSSL_RET_FAILURE;
		}
	}

	/* The RsslReactorChannelImpl must not be moved to RsslReactorWorker.inactiveChannels when the channel is closed by the users as it will be returned to the 
	channel pool of RsslReactorImpl for reusing later. */  
	if (pReactorChannel->workerParentList != NULL)
	{
		_reactorWorkerMoveChannel(&pReactorImpl->reactorWorker.inactiveChannels, pReactorChannel);
	}

	rsslClearReactorChannelEventImpl(pEvent);

	pEvent->channelEvent.channelEventType = RSSL_RC_CET_CHANNEL_DOWN;
	pEvent->channelEvent.pReactorChannel = (RsslReactorChannel*)pReactorChannel;
	pEvent->channelEvent.pError = pError;
	pEvent->isConnectFailure = isConnectFailure;

	_reactorWorkerCalculateNextTimeout(pReactorImpl, defaultSelectTimeoutMs);

	if (!RSSL_ERROR_INFO_CHECK(rsslReactorEventQueuePut(&pReactorChannel->eventQueue, (RsslReactorEventImpl*)pEvent) == RSSL_RET_SUCCESS, RSSL_RET_FAILURE, &pReactorWorker->workerCerr))
		return RSSL_RET_FAILURE;

	return RSSL_RET_SUCCESS;
}

RsslRet _reactorWorkerSendFlushComplete(RsslReactorImpl *pReactorImpl, RsslReactorChannelImpl *pReactorChannel)
{
	/* Signal reactor that worker is done flushing */
	RsslReactorFlushEvent *pEvent = (RsslReactorFlushEvent*)rsslReactorEventQueueGetFromPool(&pReactorChannel->eventQueue);
	RsslReactorWorker *pReactorWorker = &pReactorImpl->reactorWorker;

	rsslInitFlushEvent(pEvent);
	pEvent->pReactorChannel = (RsslReactorChannel*)pReactorChannel;
	pEvent->flushEventType = RSSL_RCIMPL_FET_FLUSH_DONE;
	if (!RSSL_ERROR_INFO_CHECK(rsslReactorEventQueuePut(&pReactorChannel->eventQueue, (RsslReactorEventImpl*)pEvent) == RSSL_RET_SUCCESS, RSSL_RET_FAILURE, &pReactorWorker->workerCerr))
		return RSSL_RET_FAILURE;
	return RSSL_RET_SUCCESS;
}

RsslRet _reactorWorkerSendTimerExpired(RsslReactorImpl *pReactorImpl, RsslReactorChannelImpl *pReactorChannel, RsslInt64 currentTime)
{
	RsslReactorTimerEvent *pEvent = (RsslReactorTimerEvent*)rsslReactorEventQueueGetFromPool(&pReactorChannel->eventQueue);
	RsslReactorWorker *pReactorWorker = &pReactorImpl->reactorWorker;

	rsslInitTimerEvent(pEvent);
	pEvent->pReactorChannel = (RsslReactorChannel*)pReactorChannel;
	pEvent->expireTime = currentTime;
	pReactorChannel->nextExpireTime = RCIMPL_TIMER_UNSET;
	if (!RSSL_ERROR_INFO_CHECK(rsslReactorEventQueuePut(&pReactorChannel->eventQueue, (RsslReactorEventImpl*)pEvent) == RSSL_RET_SUCCESS, RSSL_RET_FAILURE, &pReactorWorker->workerCerr))
		return RSSL_RET_FAILURE;
	return RSSL_RET_SUCCESS;
}

static void _reactorWorkerCalculateNextTimeout(RsslReactorImpl *pReactorImpl, RsslUInt32 newTimeoutMicroSeconds)
{
	RsslReactorWorker *pReactorWorker = &pReactorImpl->reactorWorker;

	if (newTimeoutMicroSeconds < pReactorWorker->sleepTimeMs)
		pReactorWorker->sleepTimeMs = newTimeoutMicroSeconds;
	}

void _reactorWorkerMoveChannel(RsslQueue *pNewList, RsslReactorChannelImpl *pReactorChannel)
{ 
	if (pReactorChannel->workerParentList)
	{
		rsslQueueRemoveLink(pReactorChannel->workerParentList, &pReactorChannel->workerLink);
		rsslInitQueueLink(&pReactorChannel->workerLink);
	}

	pReactorChannel->workerParentList = pNewList; 

	if (pNewList)
		rsslQueueAddLinkToBack(pNewList, &pReactorChannel->workerLink);
}

RSSL_THREAD_DECLARE(runReactorWorker, pArg)
{
	RsslReactorImpl *pReactorImpl = (RsslReactorImpl*)pArg;
	RsslReactorWorker *pReactorWorker = &pReactorImpl->reactorWorker;
	RsslReactorEventQueue *pEventQueue = &pReactorWorker->workerQueue;

	pReactorWorker->sleepTimeMs = 3000;

	while (1)
	{
		RsslRet ret;
		RsslReactorChannelImpl *pReactorChannel;
		RsslRestHandle *pRestHandle;
		RsslQueueLink *pLink;

		ret = rsslNotifierWait(pReactorWorker->pNotifier, pReactorWorker->sleepTimeMs * 1000);

		pReactorWorker->lastRecordedTimeMs = getCurrentTimeMs(pReactorImpl->ticksPerMsec);


		if (ret > 0)
		{
			int i;

			for (i = 0; i < pReactorWorker->pNotifier->notifiedEventCount; ++i)
			{
				RsslNotifierEvent *pNotifierEvent = pReactorWorker->pNotifier->notifiedEvents[i];
				void *object = rsslNotifierEventGetObject(pNotifierEvent);
				if (object == &pReactorWorker->workerQueue)
				{
					RsslReactorEventImpl *pEvent;
					/* Message in event queue */
					if ((pEvent = rsslReactorEventQueueGet(pEventQueue, &ret)))
					{
						switch(pEvent->base.eventType)
						{
							case RSSL_RCIMPL_ET_CHANNEL:
								{
									RsslReactorChannelEventImpl *pConnEvent = &pEvent->channelEventImpl;
									pReactorChannel = (RsslReactorChannelImpl*)pConnEvent->channelEvent.pReactorChannel;

									switch((int)pConnEvent->channelEvent.channelEventType)
									{
										case RSSL_RCIMPL_CET_NEW_CHANNEL:
											{
												if (!RSSL_ERROR_INFO_CHECK(_reactorWorkerProcessNewChannel(pReactorImpl, pReactorChannel) == RSSL_RET_SUCCESS, ret, &pReactorWorker->workerCerr))
													return (_reactorWorkerShutdown(pReactorImpl, &pReactorWorker->workerCerr), RSSL_THREAD_RETURN());
												break;
											}
										case RSSL_RC_CET_CHANNEL_DOWN:
											{
												if(pReactorChannel->reactorChannel.pRsslChannel != 0)
												{
													/* Make sure descriptors are cleared */
													if (pReactorChannel->reactorChannel.pRsslChannel->socketId != REACTOR_INVALID_SOCKET)
													{
														if (rsslNotifierRemoveEvent(pReactorWorker->pNotifier, pReactorChannel->pWorkerNotifierEvent) < 0)
														{
															rsslSetErrorInfo(&pReactorWorker->workerCerr, RSSL_EIC_FAILURE, RSSL_RET_FAILURE, __FILE__, __LINE__, 
																	"Failed to remove channel from notifier.");
															return (_reactorWorkerShutdown(pReactorImpl, &pReactorWorker->workerCerr), RSSL_THREAD_RETURN());
														}
													}
												}

												/* Remove channel from worker's list */
												_reactorWorkerMoveChannel(&pReactorWorker->inactiveChannels, pReactorChannel);
												pReactorChannel->lastPingSentMs = 0;

												break;
											}
										case RSSL_RC_CET_CHANNEL_DOWN_RECONNECTING:
											{
												_reactorWorkerMoveChannel(&pReactorImpl->reactorWorker.reconnectingChannels, pReactorChannel);
												pReactorChannel->lastPingSentMs = 0;

												if(pReactorChannel->reactorChannel.pRsslChannel != 0)
												{
													/* Make sure descriptors are cleared */
													if (rsslNotifierRemoveEvent(pReactorWorker->pNotifier, pReactorChannel->pWorkerNotifierEvent) < 0)
													{
														rsslSetErrorInfo(&pReactorWorker->workerCerr, RSSL_EIC_FAILURE, RSSL_RET_FAILURE, __FILE__, __LINE__, 
																"Failed to remove channel from notifier.");
														return (_reactorWorkerShutdown(pReactorImpl, &pReactorWorker->workerCerr), RSSL_THREAD_RETURN());
													}

													pReactorChannel->reactorChannel.socketId = (RsslSocket)REACTOR_INVALID_SOCKET;

													/* Close the channel */
													/* TODO: Error check? */
													rsslCloseChannel(pReactorChannel->reactorChannel.pRsslChannel, &(pReactorChannel->channelWorkerCerr.rsslError));
													pReactorChannel->reactorChannel.pRsslChannel = 0;
												}

												if(pReactorChannel->reconnectAttemptCount == 0)
												{
													pReactorChannel->reconnectDelay = pReactorChannel->reconnectMinDelay;

												}
												else
												{
													pReactorChannel->reconnectDelay = 2 * pReactorChannel->reconnectDelay;

													if(pReactorChannel->reconnectDelay > pReactorChannel->reconnectMaxDelay)
														pReactorChannel->reconnectDelay = pReactorChannel->reconnectMaxDelay;
												}

												pReactorChannel->reconnectAttemptCount++;

												pReactorChannel->lastReconnectAttemptMs = pReactorImpl->reactorWorker.lastRecordedTimeMs;

												_reactorWorkerCalculateNextTimeout(pReactorImpl, pReactorChannel->reconnectDelay);

												break;

											}
										case RSSL_RCIMPL_CET_CLOSE_CHANNEL:
											{
												RsslReactorChannelEventImpl *pEvent = (RsslReactorChannelEventImpl*)rsslReactorEventQueueGetFromPool(&pReactorChannel->eventQueue);
												RsslRet ret;

												if ((pReactorChannel->workerParentList != &pReactorWorker->inactiveChannels) && (pReactorChannel->reactorChannel.pRsslChannel != NULL))
												{
													/* Make sure descriptors are cleared */
													if (rsslNotifierRemoveEvent(pReactorWorker->pNotifier, pReactorChannel->pWorkerNotifierEvent) < 0)
													{
														rsslSetErrorInfo(&pReactorWorker->workerCerr, RSSL_EIC_FAILURE, RSSL_RET_FAILURE, __FILE__, __LINE__, 
																"Failed to remove channel from notifier.");
														return (_reactorWorkerShutdown(pReactorImpl, &pReactorWorker->workerCerr), RSSL_THREAD_RETURN());
													}
												}

												/* Close RSSL channel if present. */
												if (pReactorChannel->reactorChannel.pRsslChannel)
												{
													if (!RSSL_ERROR_INFO_CHECK((ret = rsslCloseChannel(pReactorChannel->reactorChannel.pRsslChannel, &pReactorWorker->workerCerr.rsslError)) >= RSSL_RET_SUCCESS, RSSL_RET_FAILURE, &pReactorWorker->workerCerr))
													{
														rsslSetErrorInfoLocation(&pReactorChannel->channelWorkerCerr, __FILE__, __LINE__);
														return (_reactorWorkerShutdown(pReactorImpl, &pReactorWorker->workerCerr), RSSL_THREAD_RETURN());
													}
												}

												/* Free copied RDMMsgs */
												_reactorWorkerFreeChannelRDMMsgs(pReactorChannel);

												/* Free connection list */
												if(pReactorChannel->connectionOptList)
												{
													_rsslChannelFreeConnectionList(pReactorChannel);
												}

												_reactorWorkerMoveChannel(NULL, pReactorChannel);

												/* Acknowledge that this channel has been closed. */
												rsslClearReactorChannelEventImpl((RsslReactorChannelEventImpl*)pEvent);
												pEvent->channelEvent.channelEventType = (RsslReactorChannelEventType)RSSL_RCIMPL_CET_CLOSE_CHANNEL_ACK;
												pEvent->channelEvent.pReactorChannel = (RsslReactorChannel*)pReactorChannel;
												pEvent->channelEvent.pError = pConnEvent->channelEvent.pError;
												if (!RSSL_ERROR_INFO_CHECK(rsslReactorEventQueuePut(&pReactorChannel->eventQueue, (RsslReactorEventImpl*)pEvent) == RSSL_RET_SUCCESS, RSSL_RET_FAILURE, &pReactorWorker->workerCerr))
													return (_reactorWorkerShutdown(pReactorImpl, &pReactorWorker->workerCerr), RSSL_THREAD_RETURN());
											}
											break;
										default:
											rsslSetErrorInfo(&pReactorWorker->workerCerr, RSSL_EIC_FAILURE, RSSL_RET_FAILURE, __FILE__, __LINE__, 
													"Unknown channel event type %d", pConnEvent->channelEvent.channelEventType);
											return (_reactorWorkerShutdown(pReactorImpl, &pReactorWorker->workerCerr), RSSL_THREAD_RETURN());
									}

									break;
								}

							case RSSL_RCIMPL_ET_FLUSH:
								{
									RsslReactorFlushEvent *pFlushEvent = &pEvent->flushEvent;

									switch(pFlushEvent->flushEventType)
									{
										case RSSL_RCIMPL_FET_START_FLUSH:
											pReactorChannel = (RsslReactorChannelImpl*)pFlushEvent->pReactorChannel;
											if (pReactorChannel->reactorChannel.pRsslChannel != NULL && pReactorChannel->reactorChannel.pRsslChannel->socketId != REACTOR_INVALID_SOCKET)
											{
												if (rsslNotifierRegisterWrite(pReactorWorker->pNotifier, pReactorChannel->pWorkerNotifierEvent) < 0)
												{
													rsslSetErrorInfo(&pReactorWorker->workerCerr, RSSL_EIC_FAILURE, RSSL_RET_FAILURE, __FILE__, __LINE__, 
															"Failed to register write notification for flushing channel.");
													return (_reactorWorkerShutdown(pReactorImpl, &pReactorWorker->workerCerr), RSSL_THREAD_RETURN());
												}
											}
											break;
										default:
											rsslSetErrorInfo(&pReactorWorker->workerCerr, RSSL_EIC_FAILURE, RSSL_RET_FAILURE, __FILE__, __LINE__, 
													"Unknown flush event type %d", pFlushEvent->flushEventType);
											return (_reactorWorkerShutdown(pReactorImpl, &pReactorWorker->workerCerr), RSSL_THREAD_RETURN());
									}
									break;
								}

							case RSSL_RCIMPL_ET_TIMER:
								{
									RsslReactorTimerEvent *pTimerEvent = &pEvent->timerEvent;
									((RsslReactorChannelImpl*)pTimerEvent->pReactorChannel)->nextExpireTime = pTimerEvent->expireTime;
									break;
								}

							case RSSL_RCIMPL_ET_REACTOR:
								{
									RsslReactorStateEvent *pReactorEvent = &pEvent->reactorEvent;

									switch(pReactorEvent->reactorEventType)
									{
										case RSSL_RCIMPL_STET_SHUTDOWN:
											_reactorWorkerHandleShutdownRequest(pReactorImpl, pReactorEvent->pErrorInfo);
											break;
										case RSSL_RCIMPL_STET_DESTROY:
											return (_reactorWorkerCleanupReactor(pReactorImpl), RSSL_THREAD_RETURN());
										default:
											rsslSetErrorInfo(&pReactorWorker->workerCerr, RSSL_EIC_FAILURE, RSSL_RET_FAILURE, __FILE__, __LINE__, 
													"Unknown reactor event type %d", pReactorEvent->reactorEventType);
											return (_reactorWorkerShutdown(pReactorImpl, &pReactorWorker->workerCerr), RSSL_THREAD_RETURN());
									}
									break;
								}

							default:
								{
									rsslSetErrorInfo(&pReactorWorker->workerCerr, RSSL_EIC_FAILURE, RSSL_RET_FAILURE, __FILE__, __LINE__, "Unknown event type %d", pEvent->base.eventType);
									return (_reactorWorkerShutdown(pReactorImpl, &pReactorWorker->workerCerr), RSSL_THREAD_RETURN());
								}
						}
					}
					else if (!RSSL_ERROR_INFO_CHECK(ret >= RSSL_RET_SUCCESS, ret, &pReactorWorker->workerCerr))
						return (_reactorWorkerShutdown(pReactorImpl, &pReactorWorker->workerCerr), RSSL_THREAD_RETURN());
				}
				else if (object == pReactorImpl->pRestClient)
				{
					/* Dispatch events from RsslRestClient */
					rsslRestClientDispatch(pReactorImpl->pRestClient);
				}
				else
				{
					pReactorChannel = (RsslReactorChannelImpl*)object;

					if (pReactorChannel->reactorChannel.pRsslChannel == NULL)
					{
						/* We may have just closed this channel and started reconnecting it. Do not process notification for it. */
					}
					else if (rsslNotifierEventIsFdBad(pNotifierEvent))
					{
						/* When a bad file descriptor is encountered in the reactorWorker, it is likely because the
						 * Reactor thread received an FD_CHANGE return from rsslRead. Update it. */
						if (rsslNotifierUpdateEventFd(pReactorWorker->pNotifier, pReactorChannel->pWorkerNotifierEvent, (int)pReactorChannel->reactorChannel.pRsslChannel->socketId) < 0)
						{
							rsslSetErrorInfo(&pReactorWorker->workerCerr, RSSL_EIC_FAILURE, RSSL_RET_FAILURE, __FILE__, __LINE__, 
									"Failed to update file descriptor for channel.");
							return (_reactorWorkerShutdown(pReactorImpl, &pReactorWorker->workerCerr), RSSL_THREAD_RETURN());
						}
					}
					else if (pReactorChannel->reactorChannel.pRsslChannel->state == RSSL_CH_STATE_INITIALIZING)
					{
						RsslInProgInfo inProg = RSSL_INIT_IN_PROG_INFO;

						/* Write descriptor is set initially in case this end starts the message hanshakes that rsslInitChannel() performs. 
						 * Once rsslInitChannel() is called for the first time the channel can wait on the read descriptor for more messages.  
						 * We will set the write descriptor again if an FD_CHANGE event occurs. */
						if (rsslNotifierUnregisterWrite(pReactorWorker->pNotifier, pReactorChannel->pWorkerNotifierEvent) < 0)
						{
							rsslSetErrorInfo(&pReactorWorker->workerCerr, RSSL_EIC_FAILURE, RSSL_RET_FAILURE, __FILE__, __LINE__, 
									"Failed to unregister write notification for initializing channel.");
							return (_reactorWorkerShutdown(pReactorImpl, &pReactorWorker->workerCerr), RSSL_THREAD_RETURN());
						}
						ret = rsslInitChannel(pReactorChannel->reactorChannel.pRsslChannel, &inProg, &pReactorChannel->channelWorkerCerr.rsslError);


						switch(ret)
						{
							case RSSL_RET_CHAN_INIT_IN_PROGRESS:
								{
									if (inProg.flags & RSSL_IP_FD_CHANGE)
									{
										/* File descriptor changed. Update descriptor set.
										 * No need to send an event back -- the application will only get the descriptors when we're done initializing. */
										if (rsslNotifierUpdateEventFd(pReactorWorker->pNotifier, pReactorChannel->pWorkerNotifierEvent, (int)pReactorChannel->reactorChannel.pRsslChannel->socketId) < 0)
										{
											rsslSetErrorInfo(&pReactorWorker->workerCerr, RSSL_EIC_FAILURE, RSSL_RET_FAILURE, __FILE__, __LINE__, 
													"Failed to update notification event for initializing channel that is changing descriptor.");
											return (_reactorWorkerShutdown(pReactorImpl, &pReactorWorker->workerCerr), RSSL_THREAD_RETURN());
										}
										if (rsslNotifierRegisterRead(pReactorWorker->pNotifier, pReactorChannel->pWorkerNotifierEvent) < 0)
										{
											rsslSetErrorInfo(&pReactorWorker->workerCerr, RSSL_EIC_FAILURE, RSSL_RET_FAILURE, __FILE__, __LINE__, 
													"Failed to register read notification for initializing channel that is changing descriptor.");
											return (_reactorWorkerShutdown(pReactorImpl, &pReactorWorker->workerCerr), RSSL_THREAD_RETURN());
										}
										if (rsslNotifierRegisterWrite(pReactorWorker->pNotifier, pReactorChannel->pWorkerNotifierEvent) < 0)
										{
											rsslSetErrorInfo(&pReactorWorker->workerCerr, RSSL_EIC_FAILURE, RSSL_RET_FAILURE, __FILE__, __LINE__, 
													"Failed to register write notification for initializing channel that is changing descriptor.");
											return (_reactorWorkerShutdown(pReactorImpl, &pReactorWorker->workerCerr), RSSL_THREAD_RETURN());
										}
									}

									break;
								}

							case RSSL_RET_SUCCESS:
								{
									if (_reactorWorkerProcessChannelUp(pReactorImpl, pReactorChannel) != RSSL_RET_SUCCESS)
										return (_reactorWorkerShutdown(pReactorImpl, &pReactorWorker->workerCerr), RSSL_THREAD_RETURN());
									/* Channel is now active. Clear descriptor so we don't think we need to flush right away */
									if (rsslNotifierUnregisterWrite(pReactorWorker->pNotifier, pReactorChannel->pWorkerNotifierEvent) < 0)
									{
										rsslSetErrorInfo(&pReactorWorker->workerCerr, RSSL_EIC_FAILURE, RSSL_RET_FAILURE, __FILE__, __LINE__, 
												"Failed to unregister write notification for initializing channel.");
										return (_reactorWorkerShutdown(pReactorImpl, &pReactorWorker->workerCerr), RSSL_THREAD_RETURN());
									}
									break;
								}

							default:
								{
									/* Error */
									rsslSetErrorInfoLocation(&pReactorChannel->channelWorkerCerr, __FILE__, __LINE__);
									if (_reactorWorkerHandleChannelFailure(pReactorImpl, pReactorChannel, &pReactorChannel->channelWorkerCerr) != RSSL_RET_SUCCESS)
										return (_reactorWorkerShutdown(pReactorImpl, &pReactorWorker->workerCerr), RSSL_THREAD_RETURN());
									break;
								}
						}
					}
					else
					{
						if (pReactorChannel->reactorChannel.pRsslChannel->socketId != REACTOR_INVALID_SOCKET &&
								rsslNotifierEventIsWritable(pNotifierEvent))
						{
							/* Flush */
							ret = rsslFlush(pReactorChannel->reactorChannel.pRsslChannel, &pReactorChannel->channelWorkerCerr.rsslError);
							pReactorChannel->lastPingSentMs = pReactorWorker->lastRecordedTimeMs;

							if (ret < 0)
							{
								rsslSetErrorInfoLocation(&pReactorChannel->channelWorkerCerr, __FILE__, __LINE__);
								if (_reactorWorkerHandleChannelFailure(pReactorImpl, pReactorChannel, &pReactorChannel->channelWorkerCerr) != RSSL_RET_SUCCESS)
								{
									return (_reactorWorkerShutdown(pReactorImpl, &pReactorWorker->workerCerr), RSSL_THREAD_RETURN());
								}
							}
							else if (ret == 0)
							{
								/* Can stop flushing now */
								if (rsslNotifierUnregisterWrite(pReactorWorker->pNotifier, pReactorChannel->pWorkerNotifierEvent) < 0)
								{
									rsslSetErrorInfo(&pReactorWorker->workerCerr, RSSL_EIC_FAILURE, RSSL_RET_FAILURE, __FILE__, __LINE__, 
											"Failed to unregister write notification for flushing channel.");
									return (_reactorWorkerShutdown(pReactorImpl, &pReactorWorker->workerCerr), RSSL_THREAD_RETURN());
								}
								_reactorWorkerSendFlushComplete(pReactorImpl, pReactorChannel);
							}
						}
					}
					
				}

			}
		}
		else if (ret < 0)
		{
#ifdef WIN32
			switch(WSAGetLastError())
#else
			switch(errno)
#endif
			{
#ifdef WIN32
				case WSAEINTR:
#else
				case EINTR:
#endif
					continue;

				default:
					rsslSetErrorInfo(&pReactorWorker->workerCerr, RSSL_EIC_FAILURE, RSSL_RET_FAILURE, __FILE__, __LINE__, 
#ifdef WIN32
							"rsslNotifierWait() failed: %d", WSAGetLastError()
#else
							"rsslNotifierWait() failed: %d", errno
#endif
							);
					return (_reactorWorkerShutdown(pReactorImpl, &pReactorWorker->workerCerr), RSSL_THREAD_RETURN());
			}
		}

		/* Ping/initialization/recovery timeout check/renew the access token */

		pReactorWorker->sleepTimeMs = defaultSelectTimeoutMs;

		/* Check whether the initialization time period has passed. */
		RSSL_QUEUE_FOR_EACH_LINK(&pReactorWorker->initializingChannels, pLink)
		{
			pReactorChannel = RSSL_QUEUE_LINK_TO_OBJECT(RsslReactorChannelImpl, workerLink, pLink);
			if ((pReactorWorker->lastRecordedTimeMs - pReactorChannel->initializationStartTimeMs) > pReactorChannel->initializationTimeout * 1000)
			{
				rsslSetErrorInfo(&pReactorChannel->channelWorkerCerr, RSSL_EIC_FAILURE, RSSL_RET_FAILURE, __FILE__, __LINE__, "Initialization timed out.");
				if (_reactorWorkerHandleChannelFailure(pReactorImpl, pReactorChannel, &pReactorChannel->channelWorkerCerr) != RSSL_RET_SUCCESS)
					return (_reactorWorkerShutdown(pReactorImpl, &pReactorWorker->workerCerr), RSSL_THREAD_RETURN());
			}
			else
			{
				_reactorWorkerCalculateNextTimeout(pReactorImpl, (RsslUInt32)(pReactorChannel->initializationStartTimeMs + (pReactorChannel->initializationTimeout * 1000) - pReactorWorker->lastRecordedTimeMs));
			}
		}

		RSSL_QUEUE_FOR_EACH_LINK(&pReactorWorker->activeChannels, pLink)
		{
			pReactorChannel = RSSL_QUEUE_LINK_TO_OBJECT(RsslReactorChannelImpl, workerLink, pLink);

			/* Check if the elapsed time is greater than our ping-send interval. */
			if ((pReactorWorker->lastRecordedTimeMs - pReactorChannel->lastPingSentMs) > pReactorChannel->reactorChannel.pRsslChannel->pingTimeout * 1000 * pingIntervalFactor )
			{

				/* If so, send a ping. */
				ret = rsslPing(pReactorChannel->reactorChannel.pRsslChannel, &pReactorChannel->channelWorkerCerr.rsslError);
				if (ret < 0)
				{
					rsslSetErrorInfoLocation(&pReactorChannel->channelWorkerCerr, __FILE__, __LINE__);
					if (!RSSL_ERROR_INFO_CHECK(_reactorWorkerHandleChannelFailure(pReactorImpl, pReactorChannel, &pReactorChannel->channelWorkerCerr) == RSSL_RET_SUCCESS, RSSL_RET_FAILURE, &pReactorWorker->workerCerr))
						return (_reactorWorkerShutdown(pReactorImpl, &pReactorWorker->workerCerr), RSSL_THREAD_RETURN());
				}
				else
				{
					pReactorChannel->lastPingSentMs = pReactorWorker->lastRecordedTimeMs;
					_reactorWorkerCalculateNextTimeout(pReactorImpl, (RsslUInt32)(pReactorChannel->reactorChannel.pRsslChannel->pingTimeout * 1000 * pingIntervalFactor));
				}
			}
			else
			{
				/* Otherwise, figure out when to wake up again. (Ping interval - (current time - time of last ping). */
				_reactorWorkerCalculateNextTimeout(pReactorImpl, (RsslUInt32)(pReactorChannel->lastPingSentMs + (RsslInt64)(pReactorChannel->reactorChannel.pRsslChannel->pingTimeout * 1000 * pingIntervalFactor) - pReactorWorker->lastRecordedTimeMs));
			}

			/* Process any channels that are waiting for a timeout. */
			if (pReactorChannel->nextExpireTime != RCIMPL_TIMER_UNSET)
			{
				if (pReactorChannel->nextExpireTime < pReactorWorker->lastRecordedTimeMs)
				{
					/* Timer expired for this channel, send event back. */
					_reactorWorkerSendTimerExpired(pReactorImpl, pReactorChannel, pReactorWorker->lastRecordedTimeMs);
				}
				else
					_reactorWorkerCalculateNextTimeout(pReactorImpl, (RsslUInt32)(pReactorChannel->nextExpireTime - pReactorWorker->lastRecordedTimeMs));
			}

			/* Reissue the token using the non-blocking mode of RsslRestClient */
			if (pReactorChannel->connectionOptList && pReactorChannel->sendTokenRequest)
			{
				if ( pReactorChannel->resendFromFailure ||
					((pReactorWorker->lastRecordedTimeMs - pReactorChannel->nextExpiresTime) > 0 ) )
				{
					RsslRestRequestArgs *pRestRequestArgs;
					RsslReactorConnectInfoImpl *pReactorConnectInfoImpl = &pReactorChannel->connectionOptList[pReactorChannel->connectionListIter];

					RTR_ATOMIC_SET(pReactorChannel->sendTokenRequest, 0);

					if( _reactorWorkerRegisterEventForRestClient(pReactorWorker,pReactorImpl) != RSSL_RET_SUCCESS )
					{
						return (_reactorWorkerShutdown(pReactorImpl, &pReactorWorker->workerCerr), RSSL_THREAD_RETURN());
					}

					pReactorConnectInfoImpl->reactorChannelInfoImplState = RSSL_RC_CHINFO_IMPL_ST_REQUEST_FAILURE;

					if (pReactorChannel->resendFromFailure)
					{
						/* Handling error cases to get authentication token using the password */
						pRestRequestArgs = _reactorCreateRequestArgsForPassword(&pReactorChannel->pParentReactor->tokenServiceURL,
							&pReactorChannel->channelRole.ommConsumerRole.pOAuthCredential->userName,
							&pReactorChannel->channelRole.ommConsumerRole.pOAuthCredential->password,
							&pReactorChannel->channelRole.ommConsumerRole.pOAuthCredential->clientId,
							&pReactorChannel->channelRole.ommConsumerRole.pOAuthCredential->clientSecret,
							&pReactorChannel->channelRole.ommConsumerRole.pOAuthCredential->tokenScope, 
							&pReactorChannel->rsslPostDataBodyBuf, pReactorChannel, &pReactorWorker->workerCerr);

						if (pRestRequestArgs)
						{
							if (pReactorChannel->rsslAccessTokenRespBuffer.data == 0)
							{
								pReactorChannel->rsslAccessTokenRespBuffer.length = RSSL_REST_INIT_TOKEN_BUFFER_SIZE;
								pReactorChannel->rsslAccessTokenRespBuffer.data =
									(char*)malloc(pReactorChannel->rsslAccessTokenRespBuffer.length);

								if (pReactorChannel->rsslAccessTokenRespBuffer.data == 0)
								{
									pReactorConnectInfoImpl->reactorChannelInfoImplState = RSSL_RC_CHINFO_IMPL_ST_MEM_ALLOCATION_FAILURE;
									continue;
								}
							}

							if (_reactorWorkerRegisterEventForRestClient(pReactorWorker, pReactorImpl) != RSSL_RET_SUCCESS)
							{
								return (_reactorWorkerShutdown(pReactorImpl, &pReactorWorker->workerCerr), RSSL_THREAD_RETURN());
							}

							_assignConnectionArgsToRequestArgs(&pReactorConnectInfoImpl->base.rsslConnectOptions, pRestRequestArgs);

							if (rsslRestClientNonBlockingRequest(pReactorImpl->pRestClient, pRestRequestArgs,
								rsslRestResponseCallback, rsslRestErrorCallback,
								&pReactorChannel->rsslAccessTokenRespBuffer, &pReactorChannel->channelWorkerCerr.rsslError) != 0)
							{
								pReactorConnectInfoImpl->reactorChannelInfoImplState = RSSL_RC_CHINFO_IMPL_ST_REQ_AUTH_TOKEN;
								pReactorConnectInfoImpl->reactorTokenMgntEventType = RSSL_RCIMPL_TKET_REISSUE_NO_REFRESH;
							}
							else
							{
								if (!RSSL_ERROR_INFO_CHECK(_reactorWorkerHandleChannelFailure(pReactorImpl, pReactorChannel, &pReactorChannel->channelWorkerCerr) == RSSL_RET_SUCCESS, RSSL_RET_FAILURE, &pReactorWorker->workerCerr))
									return (_reactorWorkerShutdown(pReactorImpl, &pReactorWorker->workerCerr), RSSL_THREAD_RETURN());
							}

							free(pRestRequestArgs);
						}
					}
					else
					{
						pRestRequestArgs = _reactorWorkerCreateRequestArgsForRefreshToken(pReactorChannel, &pReactorChannel->channelWorkerCerr);

						if (pRestRequestArgs)
						{
							_assignConnectionArgsToRequestArgs(&pReactorConnectInfoImpl->base.rsslConnectOptions, pRestRequestArgs);

							if (rsslRestClientNonBlockingRequest(pReactorImpl->pRestClient, pRestRequestArgs,
								rsslRestResponseCallback,
								rsslRestErrorCallback,
								&pReactorChannel->rsslAccessTokenRespBuffer, &pReactorChannel->channelWorkerCerr.rsslError) != 0)
							{
								pReactorConnectInfoImpl->reactorChannelInfoImplState = RSSL_RC_CHINFO_IMPL_ST_REQ_AUTH_TOKEN;
								pReactorConnectInfoImpl->reactorTokenMgntEventType = RSSL_RCIMPL_TKET_REISSUE_NO_REFRESH;
							}
							else
							{
								if (!RSSL_ERROR_INFO_CHECK(_reactorWorkerHandleChannelFailure(pReactorImpl, pReactorChannel, &pReactorChannel->channelWorkerCerr) == RSSL_RET_SUCCESS, RSSL_RET_FAILURE, &pReactorWorker->workerCerr))
									return (_reactorWorkerShutdown(pReactorImpl, &pReactorWorker->workerCerr), RSSL_THREAD_RETURN());
							}

							free(pRestRequestArgs);
						}
					}

					// Reset the indication flag
					pReactorChannel->resendFromFailure = 0;
				}
			}
		}

		RSSL_QUEUE_FOR_EACH_LINK(&pReactorWorker->reconnectingChannels, pLink)
		{
			pReactorChannel = RSSL_QUEUE_LINK_TO_OBJECT(RsslReactorChannelImpl, workerLink, pLink);
			
			if((pReactorWorker->lastRecordedTimeMs - pReactorChannel->lastReconnectAttemptMs) >= pReactorChannel->reconnectDelay)
				/* Attempt to reconnect here */
			{
				RsslReactorConnectInfoImpl *pReactorConnectInfoImpl;
				RsslBool submitOriginalCredential = RSSL_FALSE;

				if ( (pReactorChannel->connectionOptList[pReactorChannel->connectionListIter].reactorChannelInfoImplState == RSSL_RC_CHINFO_IMPL_ST_REQ_AUTH_TOKEN) ||
					(pReactorChannel->connectionOptList[pReactorChannel->connectionListIter].reactorChannelInfoImplState == RSSL_RC_CHINFO_IMPL_ST_QUERYING_SERVICE_DISOVERY))
					continue; /* Still waiting for response from the token service or service discovery before switching to another RsslReactorConnectInfoImpl */

				/* Restore the login request information before moving to the next RsslReactorConnectInfoImpl */
				if (pReactorChannel->connectionOptList[pReactorChannel->connectionListIter].base.enableSessionManagement &&
					pReactorChannel->channelRole.ommConsumerRole.pLoginRequest)
				{
					if (pReactorChannel->channelRole.ommConsumerRole.pLoginRequest->userName.data != pReactorChannel->userName.data)
					{
						pReactorChannel->channelRole.ommConsumerRole.pLoginRequest->flags = pReactorChannel->flags;
						pReactorChannel->channelRole.ommConsumerRole.pLoginRequest->userName = pReactorChannel->userName;
						pReactorChannel->channelRole.ommConsumerRole.pLoginRequest->userNameType = pReactorChannel->userNameType;
						submitOriginalCredential = RSSL_TRUE;
					}
				}

				pReactorChannel->connectionListIter++;

				if(pReactorChannel->connectionListIter >= pReactorChannel->connectionListCount)
				{
					pReactorChannel->connectionListIter = 0;
				}

				pReactorChannel->reactorChannel.userSpecPtr = pReactorChannel->connectionOptList[pReactorChannel->connectionListIter].base.rsslConnectOptions.userSpecPtr;

				pReactorConnectInfoImpl = &pReactorChannel->connectionOptList[pReactorChannel->connectionListIter];

				if (pReactorChannel->connectionOptList[pReactorChannel->connectionListIter].base.enableSessionManagement)
				{
					RsslRestHeader *pRestHeader = 0;
					RsslRestRequestArgs *pRestRequestArgs = 0;

					RTR_ATOMIC_SET(pReactorChannel->sendTokenRequest, 0);

					if ( pReactorChannel->connectionOptList[pReactorChannel->connectionListIter].reactorChannelInfoImplState >= RSSL_RC_CHINFO_IMPL_ST_RECEIVED_AUTH_TOKEN)
					{
						pReactorConnectInfoImpl->reactorChannelInfoImplState = RSSL_RC_CHINFO_IMPL_ST_REQUEST_FAILURE;

						if( _reactorWorkerRegisterEventForRestClient(pReactorWorker,pReactorImpl) != RSSL_RET_SUCCESS )
						{
							return (_reactorWorkerShutdown(pReactorImpl, &pReactorWorker->workerCerr), RSSL_THREAD_RETURN());
						}
 
						pRestRequestArgs = _reactorWorkerCreateRequestArgsForRefreshToken(pReactorChannel,&pReactorChannel->channelWorkerCerr);

						if (pRestRequestArgs)
						{
							if (pReactorChannel->rsslAccessTokenRespBuffer.data == 0)
							{
								pReactorChannel->rsslAccessTokenRespBuffer.length = RSSL_REST_INIT_TOKEN_BUFFER_SIZE;
								pReactorChannel->rsslAccessTokenRespBuffer.data =
									(char*)malloc(pReactorChannel->rsslAccessTokenRespBuffer.length);

								if (pReactorChannel->rsslAccessTokenRespBuffer.data == 0)
								{
									pReactorConnectInfoImpl->reactorChannelInfoImplState = RSSL_RC_CHINFO_IMPL_ST_MEM_ALLOCATION_FAILURE;
									continue;
								}
							}

							_assignConnectionArgsToRequestArgs(&pReactorConnectInfoImpl->base.rsslConnectOptions, pRestRequestArgs);

							if ( rsslRestClientNonBlockingRequest(pReactorImpl->pRestClient, pRestRequestArgs,
												rsslRestResponseCallback,
												rsslRestErrorCallback,
												&pReactorChannel->rsslAccessTokenRespBuffer, &pReactorChannel->channelWorkerCerr.rsslError) != 0 )
							{
								pReactorConnectInfoImpl->reactorChannelInfoImplState = RSSL_RC_CHINFO_IMPL_ST_REQ_AUTH_TOKEN;
								pReactorConnectInfoImpl->reactorTokenMgntEventType = RSSL_RCIMPL_TKET_REISSUE;
							}
							else
							{
								if (!RSSL_ERROR_INFO_CHECK(_reactorWorkerHandleChannelFailure(pReactorImpl, pReactorChannel, &pReactorChannel->channelWorkerCerr) == RSSL_RET_SUCCESS, RSSL_RET_FAILURE, &pReactorWorker->workerCerr))
									return (_reactorWorkerShutdown(pReactorImpl, &pReactorWorker->workerCerr), RSSL_THREAD_RETURN());
							}
												
							free(pRestRequestArgs);
						}
					}
					else
					{	
						/* Handling error cases to get authentication token using the password */
						pRestRequestArgs = _reactorCreateRequestArgsForPassword(&pReactorChannel->pParentReactor->tokenServiceURL,
							&pReactorChannel->channelRole.ommConsumerRole.pOAuthCredential->userName,
							&pReactorChannel->channelRole.ommConsumerRole.pOAuthCredential->password,
							&pReactorChannel->channelRole.ommConsumerRole.pOAuthCredential->clientId,
							&pReactorChannel->channelRole.ommConsumerRole.pOAuthCredential->clientSecret,
							&pReactorChannel->channelRole.ommConsumerRole.pOAuthCredential->tokenScope,
							&pReactorChannel->rsslPostDataBodyBuf, pReactorChannel, &pReactorWorker->workerCerr);
						
						if ( pRestRequestArgs )
						{
							pReactorConnectInfoImpl->reactorChannelInfoImplState = RSSL_RC_CHINFO_IMPL_ST_REQUEST_FAILURE;

							if (pReactorChannel->rsslAccessTokenRespBuffer.data == 0)
							{
								pReactorChannel->rsslAccessTokenRespBuffer.length = RSSL_REST_INIT_TOKEN_BUFFER_SIZE;
								pReactorChannel->rsslAccessTokenRespBuffer.data =
									(char*)malloc(pReactorChannel->rsslAccessTokenRespBuffer.length);

								if (pReactorChannel->rsslAccessTokenRespBuffer.data == 0)
								{
									pReactorConnectInfoImpl->reactorChannelInfoImplState = RSSL_RC_CHINFO_IMPL_ST_MEM_ALLOCATION_FAILURE;
									continue;
								}
							}

							if (_reactorWorkerRegisterEventForRestClient(pReactorWorker, pReactorImpl) != RSSL_RET_SUCCESS)
							{
								return (_reactorWorkerShutdown(pReactorImpl, &pReactorWorker->workerCerr), RSSL_THREAD_RETURN());
							}

							_assignConnectionArgsToRequestArgs(&pReactorConnectInfoImpl->base.rsslConnectOptions, pRestRequestArgs);
							
							if ( rsslRestClientNonBlockingRequest(pReactorImpl->pRestClient, pRestRequestArgs,
											rsslRestResponseCallback, rsslRestErrorCallback,
											&pReactorChannel->rsslAccessTokenRespBuffer, &pReactorChannel->channelWorkerCerr.rsslError) != 0 )
							{
								pReactorConnectInfoImpl->reactorChannelInfoImplState = RSSL_RC_CHINFO_IMPL_ST_REQ_AUTH_TOKEN;
								pReactorConnectInfoImpl->reactorTokenMgntEventType = RSSL_RCIMPL_TKET_REISSUE;
							}
							else
							{
								if (!RSSL_ERROR_INFO_CHECK(_reactorWorkerHandleChannelFailure(pReactorImpl, pReactorChannel, &pReactorChannel->channelWorkerCerr) == RSSL_RET_SUCCESS, RSSL_RET_FAILURE, &pReactorWorker->workerCerr))
									return (_reactorWorkerShutdown(pReactorImpl, &pReactorWorker->workerCerr), RSSL_THREAD_RETURN());
							}

							free(pRestRequestArgs);
						}
					}

					if ((!pReactorConnectInfoImpl->base.rsslConnectOptions.connectionInfo.unified.address || !(*pReactorConnectInfoImpl->base.rsslConnectOptions.connectionInfo.unified.address)) &&
						(!pReactorConnectInfoImpl->base.rsslConnectOptions.connectionInfo.unified.serviceName || !(*pReactorConnectInfoImpl->base.rsslConnectOptions.connectionInfo.unified.serviceName)))
					{
						/* Do not making a connection yet as the connection parameters is not avaliable. */
						continue; 
					}
				}
				else
				{
					/* Create an event to submit the original login message when it is changed */
					if (pReactorChannel->supportSessionMgnt && pReactorChannel->pWatchlist && submitOriginalCredential)
					{
						RsslReactorTokenMgntEvent *pEvent = (RsslReactorTokenMgntEvent*)rsslReactorEventQueueGetFromPool(&pReactorChannel->pParentReactor->reactorEventQueue);

						rsslClearReactorTokenMgntEvent(pEvent);
						pEvent->reactorTokenMgntEventType = RSSL_RCIMPL_TKET_SUBMIT_LOGIN_MSG;
						pEvent->pReactorChannel = &pReactorChannel->reactorChannel;
						pEvent->pAuthTokenEventCallback = pReactorConnectInfoImpl->base.pAuthTokenEventCallback;

						if (!RSSL_ERROR_INFO_CHECK(rsslReactorEventQueuePut(&pReactorChannel->pParentReactor->reactorEventQueue, (RsslReactorEventImpl*)pEvent)
							== RSSL_RET_SUCCESS, RSSL_RET_FAILURE, &pReactorWorker->workerCerr))
							return (_reactorWorkerShutdown(pReactorImpl, &pReactorWorker->workerCerr), RSSL_THREAD_RETURN());
					}
                                        
                    if ( !(pReactorChannel->reactorChannel.pRsslChannel = rsslConnect(&(pReactorConnectInfoImpl->base.rsslConnectOptions), &pReactorChannel->channelWorkerCerr.rsslError)))
                    {
                        if (!RSSL_ERROR_INFO_CHECK(_reactorWorkerHandleChannelFailure(pReactorImpl, pReactorChannel, &pReactorChannel->channelWorkerCerr) == RSSL_RET_SUCCESS, RSSL_RET_FAILURE, &pReactorWorker->workerCerr))
							return (_reactorWorkerShutdown(pReactorImpl, &pReactorWorker->workerCerr), RSSL_THREAD_RETURN());
                    }
                    else
                    {
                        pReactorChannel->channelSetupState = RSSL_RC_CHST_INIT;
                        pReactorChannel->initializationTimeout = pReactorConnectInfoImpl->base.initializationTimeout;
                        if (!RSSL_ERROR_INFO_CHECK(_reactorWorkerProcessNewChannel(pReactorImpl, pReactorChannel) == RSSL_RET_SUCCESS, ret, &pReactorWorker->workerCerr))
							return (_reactorWorkerShutdown(pReactorImpl, &pReactorWorker->workerCerr), RSSL_THREAD_RETURN());
                    }
				}
			}
			else
			{
				_reactorWorkerCalculateNextTimeout(pReactorImpl, (RsslUInt32)(pReactorChannel->lastReconnectAttemptMs + pReactorChannel->reconnectDelay - pReactorWorker->lastRecordedTimeMs));
			}
		}

		/* Cleaning up the RsslRestHandles outside of the callback methods */
		while ((pLink = rsslQueueRemoveFirstLink(&pReactorWorker->disposableRestHandles)))
		{
			pRestHandle = RSSL_QUEUE_LINK_TO_OBJECT(RsslRestHandle, queueLink, pLink);

			rsslRestCloseHandle(pRestHandle, &pReactorWorker->workerCerr.rsslError);
		}
	}
}

void _reactorWorkerShutdown(RsslReactorImpl *pReactorImpl, RsslErrorInfo *pErrorInfo)
{
	RsslReactorStateEvent *pEvent = (RsslReactorStateEvent*)rsslReactorEventQueueGetFromPool(&pReactorImpl->reactorEventQueue);
	RsslReactorWorker *pReactorWorker = &pReactorImpl->reactorWorker;

	/* Send reactor shutdown request to reactor */
	rsslClearReactorEvent(pEvent);
	pEvent->reactorEventType = RSSL_RCIMPL_STET_SHUTDOWN;
	pEvent->pErrorInfo = &pReactorImpl->reactorWorker.workerCerr;

	if (!RSSL_ERROR_INFO_CHECK(rsslReactorEventQueuePut(&pReactorImpl->reactorEventQueue, (RsslReactorEventImpl*)pEvent) == RSSL_RET_SUCCESS, RSSL_RET_FAILURE, &pReactorWorker->workerCerr))
		return;

	/* Wait for reactor to send back shutdown request */
	while (RSSL_TRUE)
	{
		RsslRet ret;

		pEvent = (RsslReactorStateEvent*)rsslReactorEventQueueGet(&pReactorImpl->reactorWorker.workerQueue, &ret);
		if (!RSSL_ERROR_INFO_CHECK(ret >= RSSL_RET_SUCCESS, ret, &pReactorWorker->workerCerr))
			return;

		if (pEvent && pEvent->base.eventType == RSSL_RCIMPL_ET_REACTOR && pEvent->reactorEventType == RSSL_RCIMPL_STET_SHUTDOWN)
		{
			_reactorWorkerHandleShutdownRequest(pReactorImpl, pErrorInfo);
			return;
		}
	}
}

void _reactorWorkerHandleShutdownRequest(RsslReactorImpl *pReactorImpl, RsslErrorInfo *pErrorInfo)
{
	RsslReactorChannelImpl *pReactorChannel;
	RsslReactorWorker *pReactorWorker = &pReactorImpl->reactorWorker;
	RsslQueueLink *pLink;

	RSSL_QUEUE_FOR_EACH_LINK(&pReactorWorker->activeChannels, pLink)
	{
		pReactorChannel = RSSL_QUEUE_LINK_TO_OBJECT(RsslReactorChannelImpl, workerLink, pLink);
		_reactorWorkerMoveChannel(&pReactorWorker->inactiveChannels, pReactorChannel);
	}

	RSSL_QUEUE_FOR_EACH_LINK(&pReactorWorker->initializingChannels, pLink)
	{
		pReactorChannel = RSSL_QUEUE_LINK_TO_OBJECT(RsslReactorChannelImpl, workerLink, pLink);
		_reactorWorkerMoveChannel(&pReactorWorker->inactiveChannels, pReactorChannel);
	}

	RSSL_QUEUE_FOR_EACH_LINK(&pReactorWorker->reconnectingChannels, pLink)
	{
		pReactorChannel = RSSL_QUEUE_LINK_TO_OBJECT(RsslReactorChannelImpl, workerLink, pLink);
		_reactorWorkerMoveChannel(&pReactorWorker->inactiveChannels, pReactorChannel);
	}
	
	return;
}

RsslRet _reactorWorkerProcessNewChannel(RsslReactorImpl *pReactorImpl, RsslReactorChannelImpl *pReactorChannel)
{
	RsslReactorWorker *pReactorWorker = &pReactorImpl->reactorWorker;

	/* Add channel to descriptor set */
	if (rsslNotifierAddEvent(pReactorWorker->pNotifier, pReactorChannel->pWorkerNotifierEvent, (int)pReactorChannel->reactorChannel.pRsslChannel->socketId, pReactorChannel) < 0)
	{
		rsslSetErrorInfo(&pReactorChannel->channelWorkerCerr, RSSL_EIC_FAILURE, RSSL_RET_FAILURE, __FILE__, __LINE__, "Failed to add notifier event for initializing channel.");
		return RSSL_RET_FAILURE;
	}

	if (rsslNotifierRegisterWrite(pReactorWorker->pNotifier, pReactorChannel->pWorkerNotifierEvent) < 0)
	{
		rsslSetErrorInfo(&pReactorChannel->channelWorkerCerr, RSSL_EIC_FAILURE, RSSL_RET_FAILURE, __FILE__, __LINE__, "Failed to register write notification for initializing channel.");
		return RSSL_RET_FAILURE;
	}

	switch (pReactorChannel->reactorChannel.pRsslChannel->state)
	{
		case RSSL_CH_STATE_INITIALIZING:
			_reactorWorkerMoveChannel(&pReactorWorker->initializingChannels, pReactorChannel);
			_reactorWorkerCalculateNextTimeout(pReactorImpl, pReactorChannel->initializationTimeout*1000);
			pReactorChannel->initializationStartTimeMs = pReactorWorker->lastRecordedTimeMs;
			if (rsslNotifierRegisterRead(pReactorWorker->pNotifier, pReactorChannel->pWorkerNotifierEvent) < 0)
			{
				rsslSetErrorInfo(&pReactorChannel->channelWorkerCerr, RSSL_EIC_FAILURE, RSSL_RET_FAILURE, __FILE__, __LINE__, "Failed to register read notification for initializing channel.");
				return RSSL_RET_FAILURE;
			}
			return RSSL_RET_SUCCESS;
		case RSSL_CH_STATE_ACTIVE:
		{
			_reactorWorkerMoveChannel(&pReactorWorker->activeChannels, pReactorChannel);
			return _reactorWorkerProcessChannelUp(pReactorImpl, pReactorChannel);
		}
		default:
		{
			/* Channel failed */
			return _reactorWorkerHandleChannelFailure(pReactorImpl, pReactorChannel, &pReactorChannel->channelWorkerCerr);
		}
	}
}

static RsslRet _reactorWorkerRegisterEventForRestClient(RsslReactorWorker *pReactorWorker, RsslReactorImpl *pReactorImpl)
{
	if (!pReactorImpl->registeredRsslRestClientEventFd)
	{
		if (rsslNotifierAddEvent(pReactorWorker->pNotifier, pReactorImpl->pWorkerNotifierEvent, (int)(pReactorImpl->pRestClient->eventFd), pReactorImpl->pRestClient) < 0)
		{
			rsslSetErrorInfo(&pReactorWorker->workerCerr, RSSL_EIC_FAILURE, RSSL_RET_FAILURE, __FILE__, __LINE__,
				"Failed to add notifier event for RsslRestClient events");
			return RSSL_RET_FAILURE;
		}

		if (rsslNotifierRegisterRead(pReactorWorker->pNotifier, pReactorImpl->pWorkerNotifierEvent) < 0)
		{
			rsslSetErrorInfo(&pReactorWorker->workerCerr, RSSL_EIC_FAILURE, RSSL_RET_FAILURE, __FILE__, __LINE__,
				"Failed to register read notifier events for RsslRestClient.");
			return RSSL_RET_FAILURE;
		}
					
		pReactorImpl->registeredRsslRestClientEventFd = RSSL_TRUE;
	}

	return RSSL_RET_SUCCESS;
}

static void _reactorWorkerFreeChannelRDMMsgs(RsslReactorChannelImpl *pReactorChannel)
{
	/* Free any copied RDM messages. */
	switch(pReactorChannel->channelRole.base.roleType)
	{
		case RSSL_RC_RT_OMM_CONSUMER:
		{
			RsslReactorOMMConsumerRole *pConsRole = (RsslReactorOMMConsumerRole*)&pReactorChannel->channelRole;

			if (pConsRole->pLoginRequest)
			{
				free(pConsRole->pLoginRequest);
				pConsRole->pLoginRequest = NULL;
			}

			if (pConsRole->pOAuthCredential)
			{
				free(pConsRole->pOAuthCredential);
				pConsRole->pOAuthCredential = NULL;
			}

			if (pConsRole->pDirectoryRequest)
			{
				free(pConsRole->pDirectoryRequest);
				pConsRole->pDirectoryRequest = NULL;
			}

			break;
		}

		case RSSL_RC_RT_OMM_NI_PROVIDER:
		{
			RsslReactorOMMNIProviderRole *pNIProvRole = (RsslReactorOMMNIProviderRole*)&pReactorChannel->channelRole;

			if (pNIProvRole->pLoginRequest)
			{
				free(pNIProvRole->pLoginRequest);
				pNIProvRole->pLoginRequest = NULL;
			}

			if (pNIProvRole->pDirectoryRefresh)
			{
				free(pNIProvRole->pDirectoryRefresh);
				pNIProvRole->pDirectoryRefresh = NULL;
			}

			break;
		}

		default:
			break;
	}

}

static RsslRestRequestArgs* _reactorWorkerCreateRequestArgsForRefreshToken(RsslReactorChannelImpl *pReactorChannel, RsslErrorInfo* pError)
{
	RsslRestHeader  *pRsslRestAcceptHeader;
	RsslRestHeader  *pRsslRestContentTypeHeader;
	RsslUInt32 neededSize = 0;
	RsslRestRequestArgs *pRequestArgs = 0;
	RsslBuffer *pRsslUserNameEncodedUrl = 0;
	RsslBool	deallocateUserNameEncodedBuffer = RSSL_FALSE;
	RsslReactorOAuthCredential *pReactorOAuthCredential = pReactorChannel->channelRole.ommConsumerRole.pOAuthCredential;
	RsslBuffer* pUserName = &pReactorOAuthCredential->userName;
	RsslBool	hasClientSecret = RSSL_FALSE;
	char* pCurPos = 0;
	RsslBuffer clientId = RSSL_INIT_BUFFER;

	pRsslUserNameEncodedUrl = rsslRestEncodeUrlData(pUserName, &pError->rsslError);

	if (pError->rsslError.rsslErrorId == RSSL_RET_FAILURE)
	{
		rsslSetErrorInfo(pError, RSSL_EIC_FAILURE, RSSL_RET_FAILURE, __FILE__, __LINE__,
			"Failed to encode URL for the HTTP data body.");

		return 0;
	}

	if (pUserName != pRsslUserNameEncodedUrl)
	{
		pUserName = pRsslUserNameEncodedUrl;
		deallocateUserNameEncodedBuffer = RSSL_TRUE;
	}

	pRequestArgs = (RsslRestRequestArgs*)malloc(sizeof(RsslRestRequestArgs));

	if(pRequestArgs == 0 ) goto memoryAllocationFailed;

	rsslClearRestRequestArgs(pRequestArgs);

	if (pReactorOAuthCredential->clientId.length && pReactorOAuthCredential->clientId.data)
	{
		clientId = pReactorOAuthCredential->clientId;
	}
	else
	{
		clientId = *pUserName;
	}
				
	neededSize = rssl_rest_grant_type_refresh_token_text.length + rssl_rest_username_text.length + pUserName->length +
							rssl_rest_client_id_text.length + clientId.length + rssl_rest_refresh_token_text.length +
							pReactorChannel->tokenInformation.refreshToken.length + rssl_rest_take_exclusive_sign_on_true_text.length + 1;

	if (pReactorOAuthCredential->clientSecret.length && pReactorOAuthCredential->clientSecret.data)
	{
		hasClientSecret = RSSL_TRUE;
		neededSize += rssl_rest_client_secret_text.length + pReactorOAuthCredential->clientSecret.length;
	}

	neededSize += (RsslUInt32)(sizeof(RsslRestHeader) * 2);	

	if (pReactorChannel->rsslPostDataBodyBuf.length < neededSize)
	{
		free(pReactorChannel->rsslPostDataBodyBuf.data);
		pReactorChannel->rsslPostDataBodyBuf.length = neededSize;
		pReactorChannel->rsslPostDataBodyBuf.data = (char*)malloc(neededSize);

		if (pReactorChannel->rsslPostDataBodyBuf.data == 0)
		{
			rsslClearBuffer(&pReactorChannel->rsslPostDataBodyBuf);
			goto memoryAllocationFailed;
		}
	}
		
	memset(pReactorChannel->rsslPostDataBodyBuf.data, 0, pReactorChannel->rsslPostDataBodyBuf.length);
		
	pRequestArgs->httpBody.data = pReactorChannel->rsslPostDataBodyBuf.data;
		
	strncat(pRequestArgs->httpBody.data, rssl_rest_grant_type_refresh_token_text.data, rssl_rest_grant_type_refresh_token_text.length);
	strncat(pRequestArgs->httpBody.data, rssl_rest_username_text.data, rssl_rest_username_text.length);
	strncat(pRequestArgs->httpBody.data, pUserName->data, pUserName->length);
	strncat(pRequestArgs->httpBody.data, rssl_rest_client_id_text.data, rssl_rest_client_id_text.length);
	strncat(pRequestArgs->httpBody.data, clientId.data, clientId.length);
	if (hasClientSecret)
	{
		strncat(pRequestArgs->httpBody.data, rssl_rest_client_secret_text.data, rssl_rest_client_secret_text.length);
		strncat(pRequestArgs->httpBody.data, pReactorOAuthCredential->clientSecret.data, pReactorOAuthCredential->clientSecret.length);
	}
	strncat(pRequestArgs->httpBody.data, rssl_rest_refresh_token_text.data, rssl_rest_refresh_token_text.length);
	strncat(pRequestArgs->httpBody.data, pReactorChannel->tokenInformation.refreshToken.data, pReactorChannel->tokenInformation.refreshToken.length);
	strncat(pRequestArgs->httpBody.data, rssl_rest_take_exclusive_sign_on_true_text.data, rssl_rest_take_exclusive_sign_on_true_text.length);
	pRequestArgs->httpBody.length = (RsslUInt32)strlen(pRequestArgs->httpBody.data);
		
	pCurPos = (pRequestArgs->httpBody.data + pRequestArgs->httpBody.length + 1); // Adding 1 for null terminate string
	pRsslRestAcceptHeader = (RsslRestHeader*)pCurPos;
		
	rsslClearRestHeader(pRsslRestAcceptHeader);
	pRsslRestAcceptHeader->key.data = rssl_rest_accept_text.data;
	pRsslRestAcceptHeader->key.length = rssl_rest_accept_text.length;
	pRsslRestAcceptHeader->value.data = rssl_rest_application_json_text.data;
	pRsslRestAcceptHeader->value.length = rssl_rest_application_json_text.length;
	rsslQueueAddLinkToBack(&pRequestArgs->httpHeaders, &pRsslRestAcceptHeader->queueLink);

	pRsslRestContentTypeHeader = (RsslRestHeader*)(pCurPos + sizeof(RsslRestHeader));

	rsslClearRestHeader(pRsslRestContentTypeHeader);
	pRsslRestContentTypeHeader->key.data = rssl_rest_content_type_text.data;
	pRsslRestContentTypeHeader->key.length = rssl_rest_content_type_text.length;
	pRsslRestContentTypeHeader->value.data = rssl_rest_application_form_urlencoded_text.data;
	pRsslRestContentTypeHeader->value.length = rssl_rest_application_form_urlencoded_text.length;
	rsslQueueAddLinkToBack(&pRequestArgs->httpHeaders, &pRsslRestContentTypeHeader->queueLink);

	pRequestArgs->httpMethod = RSSL_REST_HTTP_POST;
	pRequestArgs->url.data = pReactorChannel->pParentReactor->tokenServiceURL.data;
	pRequestArgs->url.length = pReactorChannel->pParentReactor->tokenServiceURL.length;

	if (deallocateUserNameEncodedBuffer)
	{
		free(pRsslUserNameEncodedUrl->data);
		free(pRsslUserNameEncodedUrl);
	}

	pRequestArgs->pUserSpecPtr = pReactorChannel;

	return pRequestArgs;
		
memoryAllocationFailed:

	if (deallocateUserNameEncodedBuffer)
	{
		free(pRsslUserNameEncodedUrl->data);
		free(pRsslUserNameEncodedUrl);
	}
			
	if(pRequestArgs) free(pRequestArgs);	

	rsslSetErrorInfo(pError, RSSL_EIC_FAILURE, RSSL_RET_FAILURE, __FILE__, __LINE__,
		"Failed to allocate memory for creating request arguments for the token service using the refresh token.");

	return 0;
}

static void handlingAuthRequestFailure(RsslReactorChannelImpl* pReactorChannel, RsslReactorWorker* pReactorWorker, RsslReactorConnectInfoImpl *pReactorConnectInfoImpl)
{
	RsslReactorTokenMgntEvent *pEvent = (RsslReactorTokenMgntEvent*)rsslReactorEventQueueGetFromPool(&pReactorChannel->pParentReactor->reactorEventQueue);

	rsslClearReactorTokenMgntEvent(pEvent);

	// Notifies users with pAuthTokenEventCallback if exists otherwise the RSSL_RC_CET_WARNING event when the channel is active
	if (pReactorConnectInfoImpl->base.pAuthTokenEventCallback)
	{
		pEvent->reactorTokenMgntEventType = RSSL_RCIMPL_TKET_RESP_FAILURE;
		pEvent->pReactorChannel = &pReactorChannel->reactorChannel;
		pEvent->pAuthTokenEventCallback = pReactorConnectInfoImpl->base.pAuthTokenEventCallback;
		rsslCopyErrorInfo(&pEvent->errorInfo, &pReactorChannel->channelWorkerCerr);

		if (!RSSL_ERROR_INFO_CHECK(rsslReactorEventQueuePut(&pReactorChannel->pParentReactor->reactorEventQueue, (RsslReactorEventImpl*)pEvent)
			== RSSL_RET_SUCCESS, RSSL_RET_FAILURE, &pReactorWorker->workerCerr))
		{
			_reactorWorkerShutdown(pReactorChannel->pParentReactor, &pReactorWorker->workerCerr);
			return;
		}
	}

	if (pReactorChannel->reactorChannel.pRsslChannel && (pReactorChannel->reactorChannel.pRsslChannel->state == RSSL_CH_STATE_ACTIVE))
	{
		// Setting this flag to resend the authentication request with password in the active channel handling.
		if (pReactorConnectInfoImpl->reactorTokenMgntEventType == RSSL_RCIMPL_TKET_REISSUE_NO_REFRESH)
		{
			if (pReactorConnectInfoImpl->reissueTokenAttemptLimit == -1) /* there is no limit. */
			{
				pReactorChannel->resendFromFailure = 1;
				RTR_ATOMIC_SET(pReactorChannel->sendTokenRequest, 1);
			}
			else if (pReactorConnectInfoImpl->reissueTokenAttemptLimit > 0)
			{
				--pReactorConnectInfoImpl->reissueTokenAttemptLimit;
				pReactorChannel->resendFromFailure = 1;
				RTR_ATOMIC_SET(pReactorChannel->sendTokenRequest, 1);
			}
		}

		pEvent = (RsslReactorTokenMgntEvent*)rsslReactorEventQueueGetFromPool(&pReactorChannel->pParentReactor->reactorEventQueue);
		rsslClearReactorTokenMgntEvent(pEvent);

		pEvent->reactorTokenMgntEventType = RSSL_RCIMPL_TKET_CHANNEL_WARNING;
		pEvent->pReactorChannel = &pReactorChannel->reactorChannel;
		pEvent->pAuthTokenEventCallback = NULL;
		rsslCopyErrorInfo(&pEvent->errorInfo, &pReactorChannel->channelWorkerCerr);

		if (!RSSL_ERROR_INFO_CHECK(rsslReactorEventQueuePut(&pReactorChannel->pParentReactor->reactorEventQueue, (RsslReactorEventImpl*)pEvent)
			== RSSL_RET_SUCCESS, RSSL_RET_FAILURE, &pReactorWorker->workerCerr))
		{
			_reactorWorkerShutdown(pReactorChannel->pParentReactor, &pReactorWorker->workerCerr);
			return;
		}
	}
	else if ((pReactorChannel->reactorChannel.pRsslChannel == NULL) || (pReactorChannel->reactorChannel.pRsslChannel->state == RSSL_CH_STATE_CLOSED))
	{
		/* Notify error back to the application via the channel event */
		if (!RSSL_ERROR_INFO_CHECK(_reactorWorkerHandleChannelFailure(pReactorChannel->pParentReactor, pReactorChannel, &pReactorChannel->channelWorkerCerr) == RSSL_RET_SUCCESS, RSSL_RET_FAILURE, &pReactorWorker->workerCerr))
		{
			_reactorWorkerShutdown(pReactorChannel->pParentReactor, &pReactorWorker->workerCerr);
		}
	}

	pReactorConnectInfoImpl->reactorChannelInfoImplState = RSSL_RC_CHINFO_IMPL_ST_REQUEST_FAILURE;
}

static void rsslRestResponseCallback(RsslRestResponse* restresponse, RsslRestResponseEvent* event)
{
	RsslReactorChannelImpl *pReactorChannel = (RsslReactorChannelImpl*)event->closure;
	RsslError rsslError;
	RsslReactorConnectInfoImpl *pReactorConnectInfoImpl = &pReactorChannel->connectionOptList[pReactorChannel->connectionListIter];
	RsslReactorWorker *pReactorWorker = &pReactorChannel->pParentReactor->reactorWorker;
	RsslReactorImpl *pReactorImpl = pReactorChannel->pParentReactor;

	/* Releases the old memory and points the buffer to the new location. */
	if (restresponse->isMemReallocated)
	{
		free(event->userMemory->data);
		(*event->userMemory) = restresponse->reallocatedMem;
	}

	/* Set the HTTP response status code for the channel */
	pReactorChannel->httpStausCode = restresponse->statusCode;

	/* Cleaning up the RsslRestHandle later by the ReactorWorker */
	rsslQueueAddLinkToBack(&pReactorWorker->disposableRestHandles,&event->handle->queueLink);
	if (restresponse->statusCode == 200)
	{
		if (pReactorConnectInfoImpl->reactorChannelInfoImplState == RSSL_RC_CHINFO_IMPL_ST_REQ_AUTH_TOKEN)
		{
			RsslReactorTokenMgntEvent *pEvent = (RsslReactorTokenMgntEvent*)rsslReactorEventQueueGetFromPool(&pReactorChannel->pParentReactor->reactorEventQueue);

			if (pReactorChannel->tokenInformationBuffer.data == 0)
			{
				pReactorChannel->tokenInformationBuffer.length = restresponse->dataBody.length;
				pReactorChannel->tokenInformationBuffer.data = (char*)malloc(pReactorChannel->tokenInformationBuffer.length);

				if (pReactorChannel->tokenInformationBuffer.data == 0)
				{
					pReactorConnectInfoImpl->reactorChannelInfoImplState = RSSL_RC_CHINFO_IMPL_ST_MEM_ALLOCATION_FAILURE;
					return;
				}
			}

			if (rsslRestParseAccessToken(&restresponse->dataBody, &pReactorChannel->tokenInformation.accessToken, &pReactorChannel->tokenInformation.refreshToken,
							&pReactorChannel->tokenInformation.expiresIn, &pReactorChannel->tokenInformation.tokenType,
							&pReactorChannel->tokenInformation.scope, &pReactorChannel->tokenInformationBuffer, &rsslError) != RSSL_RET_SUCCESS)
			{
				pReactorConnectInfoImpl->reactorChannelInfoImplState = RSSL_RC_CHINFO_IMPL_ST_PARSE_RESP_FAILURE;
				goto RequestFailed;
			}

			/* Reset the token reissue attempt flag to the user defined value */
			pReactorConnectInfoImpl->reissueTokenAttemptLimit = pReactorConnectInfoImpl->base.reissueTokenAttemptLimit;

			rsslClearReactorTokenMgntEvent(pEvent);
			pEvent->reactorTokenMgntEventType = pReactorConnectInfoImpl->reactorTokenMgntEventType;
			pEvent->pReactorChannel = &pReactorChannel->reactorChannel;
			pEvent->pAuthTokenEventCallback = pReactorConnectInfoImpl->base.pAuthTokenEventCallback;

			if (!RSSL_ERROR_INFO_CHECK(rsslReactorEventQueuePut(&pReactorChannel->pParentReactor->reactorEventQueue, (RsslReactorEventImpl*)pEvent)
				== RSSL_RET_SUCCESS, RSSL_RET_FAILURE, &pReactorWorker->workerCerr))
			{
				_reactorWorkerShutdown(pReactorChannel->pParentReactor, &pReactorWorker->workerCerr);
				return;
			}

			if ((!pReactorConnectInfoImpl->base.rsslConnectOptions.connectionInfo.unified.address || !(*pReactorConnectInfoImpl->base.rsslConnectOptions.connectionInfo.unified.address)) &&
				(!pReactorConnectInfoImpl->base.rsslConnectOptions.connectionInfo.unified.serviceName || !(*pReactorConnectInfoImpl->base.rsslConnectOptions.connectionInfo.unified.serviceName)))
			{	/* Get host name and port for EDP-RT service discovery */
				RsslRestRequestArgs *pRestRequestArgs;
				RsslBuffer rsslBuffer;
				RsslQueueLink *pLink = NULL;
				RsslRestHeader *pRestHeader = NULL;

				RsslReactorDiscoveryTransportProtocol transport = RSSL_RD_TP_INIT;

				switch (pReactorConnectInfoImpl->base.rsslConnectOptions.connectionType)
				{
				case RSSL_CONN_TYPE_ENCRYPTED:
					transport = RSSL_RD_TP_TCP;
					break;
				default:
					rsslSetErrorInfo(&pReactorChannel->channelWorkerCerr, RSSL_EIC_FAILURE, RSSL_RET_INVALID_ARGUMENT, __FILE__, __LINE__,
						"Invalid connection type(%d) for requesting EDP-RT service discovery.",
						transport);

					pReactorConnectInfoImpl->reactorChannelInfoImplState = RSSL_RC_CHINFO_IMPL_ST_INVALID_CONNECTION_TYPE;

					/* Notify error back to the application via the channel event */
					if (!RSSL_ERROR_INFO_CHECK(_reactorWorkerHandleChannelFailure(pReactorChannel->pParentReactor, pReactorChannel, &pReactorChannel->channelWorkerCerr) == RSSL_RET_SUCCESS, RSSL_RET_FAILURE, &pReactorWorker->workerCerr))
					{
						_reactorWorkerShutdown(pReactorChannel->pParentReactor, &pReactorWorker->workerCerr);
					}
					return;
				}

				rsslClearBuffer(&rsslBuffer);
				if ((pRestRequestArgs = _reactorCreateRequestArgsForServiceDiscovery(&pReactorChannel->pParentReactor->serviceDiscoveryURL,
												transport, RSSL_RD_DP_INIT, &pReactorChannel->tokenInformation.tokenType,
												&pReactorChannel->tokenInformation.accessToken,
												&rsslBuffer, pReactorChannel, &pReactorChannel->channelWorkerCerr)) == 0)
				{
					pReactorConnectInfoImpl->reactorChannelInfoImplState = RSSL_RC_CHINFO_IMPL_ST_MEM_ALLOCATION_FAILURE;
					return;
				}

				if (pReactorChannel->rsslServiceDiscoveryRespBuffer.data == 0)
				{
					pReactorChannel->rsslServiceDiscoveryRespBuffer.length = RSSL_REST_INIT_SVC_DIS_BUF_SIZE;
					pReactorChannel->rsslServiceDiscoveryRespBuffer.data = (char*)malloc(pReactorChannel->rsslServiceDiscoveryRespBuffer.length);

					if (pReactorChannel->rsslServiceDiscoveryRespBuffer.data == 0)
					{
						pReactorConnectInfoImpl->reactorChannelInfoImplState = RSSL_RC_CHINFO_IMPL_ST_MEM_ALLOCATION_FAILURE;
						return;
					}
				}

				_assignConnectionArgsToRequestArgs(&pReactorConnectInfoImpl->base.rsslConnectOptions, pRestRequestArgs);

				if (rsslRestClientNonBlockingRequest(pReactorChannel->pParentReactor->pRestClient, pRestRequestArgs,
					rsslRestResponseCallback,
					rsslRestErrorCallback,
					&pReactorChannel->rsslServiceDiscoveryRespBuffer, &rsslError) == 0)
				{
					pReactorConnectInfoImpl->reactorChannelInfoImplState = RSSL_RC_CHINFO_IMPL_ST_REQUEST_FAILURE;
				}

				free(pRestRequestArgs);
				free(rsslBuffer.data);

				if(pReactorConnectInfoImpl->reactorChannelInfoImplState == RSSL_RC_CHINFO_IMPL_ST_REQUEST_FAILURE)
					goto RequestFailed;

				pReactorConnectInfoImpl->reactorChannelInfoImplState = RSSL_RC_CHINFO_IMPL_ST_QUERYING_SERVICE_DISOVERY;
			}
			else
			{
				pReactorConnectInfoImpl->reactorChannelInfoImplState = RSSL_RC_CHINFO_IMPL_ST_RECEIVED_AUTH_TOKEN;

				/* This is connection recovery after the previous connection is closed */
				if (pReactorChannel->reactorChannel.pRsslChannel == 0)
				{
					if (!(pReactorChannel->reactorChannel.pRsslChannel = rsslConnect((&pReactorConnectInfoImpl->base.rsslConnectOptions), &pReactorChannel->channelWorkerCerr.rsslError)))
					{
						if (!RSSL_ERROR_INFO_CHECK(_reactorWorkerHandleChannelFailure(pReactorChannel->pParentReactor, pReactorChannel, &pReactorChannel->channelWorkerCerr) == RSSL_RET_SUCCESS, RSSL_RET_FAILURE, &pReactorWorker->workerCerr))
						{
							_reactorWorkerShutdown(pReactorChannel->pParentReactor, &pReactorWorker->workerCerr);
						}
					}
					else
					{
						RsslRet ret = RSSL_RET_SUCCESS;
						pReactorChannel->channelSetupState = RSSL_RC_CHST_INIT;
						pReactorChannel->initializationTimeout = pReactorConnectInfoImpl->base.initializationTimeout;
						if (!RSSL_ERROR_INFO_CHECK(_reactorWorkerProcessNewChannel(pReactorChannel->pParentReactor, pReactorChannel) == RSSL_RET_SUCCESS, ret, &pReactorWorker->workerCerr))
						{
							_reactorWorkerShutdown(pReactorChannel->pParentReactor, &pReactorWorker->workerCerr);
						}
					}
				}
			}
		}
		else if (pReactorConnectInfoImpl->reactorChannelInfoImplState == RSSL_RC_CHINFO_IMPL_ST_QUERYING_SERVICE_DISOVERY)
		{
			RsslBuffer parseBuffer;
			RsslBuffer hostName;
			RsslBuffer port;
							  
			parseBuffer.length = RSSL_REST_STORE_HOST_AND_PORT_BUF_SIZE;
			parseBuffer.data = (char*)malloc(parseBuffer.length);
						
			if(parseBuffer.data == 0)
			{
				pReactorConnectInfoImpl->reactorChannelInfoImplState = RSSL_RC_CHINFO_IMPL_ST_MEM_ALLOCATION_FAILURE;
				return ;
			}

			if (rsslRestParseEndpoint(&restresponse->dataBody, &pReactorConnectInfoImpl->base.location, &hostName, &port, &parseBuffer, &rsslError) != RSSL_RET_SUCCESS)
			{
				free(parseBuffer.data);
				pReactorConnectInfoImpl->reactorChannelInfoImplState = RSSL_RC_CHINFO_IMPL_ST_PARSE_RESP_FAILURE;
				
				rsslSetErrorInfo(&pReactorChannel->channelWorkerCerr, RSSL_EIC_FAILURE, RSSL_RET_FAILURE, __FILE__, __LINE__,
					"Failed to query host name and port from the EDP-RT service discovery for the \"%s\" location", pReactorConnectInfoImpl->base.location.data);

				/* Notify error back to the application via the channel event */
				if (!RSSL_ERROR_INFO_CHECK(_reactorWorkerHandleChannelFailure(pReactorChannel->pParentReactor, pReactorChannel, &pReactorChannel->channelWorkerCerr) == RSSL_RET_SUCCESS, RSSL_RET_FAILURE, &pReactorWorker->workerCerr))
				{
					_reactorWorkerShutdown(pReactorChannel->pParentReactor, &pReactorWorker->workerCerr);
				}
				return;
			}

			// Free the original copy if any such as empty string
			if(pReactorConnectInfoImpl->base.rsslConnectOptions.connectionInfo.unified.address)
				free(pReactorConnectInfoImpl->base.rsslConnectOptions.connectionInfo.unified.address);

			pReactorConnectInfoImpl->base.rsslConnectOptions.connectionInfo.unified.address = (char*)malloc(hostName.length + 1);
						
			if (pReactorConnectInfoImpl->base.rsslConnectOptions.connectionInfo.unified.address == 0)
			{
				free(parseBuffer.data);
				pReactorConnectInfoImpl->reactorChannelInfoImplState = RSSL_RC_CHINFO_IMPL_ST_MEM_ALLOCATION_FAILURE;
				return ;
			}

			strncpy(pReactorConnectInfoImpl->base.rsslConnectOptions.connectionInfo.unified.address, hostName.data, hostName.length + 1);
							
			// Free the original copy if any such as empty string
			if(pReactorConnectInfoImpl->base.rsslConnectOptions.connectionInfo.unified.serviceName)
				free(pReactorConnectInfoImpl->base.rsslConnectOptions.connectionInfo.unified.serviceName);

			pReactorConnectInfoImpl->base.rsslConnectOptions.connectionInfo.unified.serviceName = (char*)malloc(port.length + 1);
						
			if(pReactorConnectInfoImpl->base.rsslConnectOptions.connectionInfo.unified.serviceName == 0)
			{
				free(parseBuffer.data);
				free(pReactorConnectInfoImpl->base.rsslConnectOptions.connectionInfo.unified.address);
				pReactorConnectInfoImpl->base.rsslConnectOptions.connectionInfo.unified.address = 0;
				pReactorConnectInfoImpl->reactorChannelInfoImplState = RSSL_RC_CHINFO_IMPL_ST_MEM_ALLOCATION_FAILURE;
				return ;
			}
						
			strncpy(pReactorConnectInfoImpl->base.rsslConnectOptions.connectionInfo.unified.serviceName, port.data, port.length + 1);
			free(parseBuffer.data);

			pReactorConnectInfoImpl->reactorChannelInfoImplState = RSSL_RC_CHINFO_IMPL_ST_ASSIGNED_HOST_PORT;

			if ( !(pReactorChannel->reactorChannel.pRsslChannel = rsslConnect((&pReactorConnectInfoImpl->base.rsslConnectOptions), &pReactorChannel->channelWorkerCerr.rsslError)))
			{
				if (!RSSL_ERROR_INFO_CHECK(_reactorWorkerHandleChannelFailure(pReactorChannel->pParentReactor, pReactorChannel, &pReactorChannel->channelWorkerCerr) == RSSL_RET_SUCCESS, RSSL_RET_FAILURE, &pReactorWorker->workerCerr))
				{
					_reactorWorkerShutdown(pReactorChannel->pParentReactor, &pReactorWorker->workerCerr);
				}
			}
			else
			{
				RsslRet ret = RSSL_RET_SUCCESS;
				pReactorChannel->channelSetupState = RSSL_RC_CHST_INIT;
				pReactorChannel->initializationTimeout = pReactorConnectInfoImpl->base.initializationTimeout;
				if (!RSSL_ERROR_INFO_CHECK(_reactorWorkerProcessNewChannel(pReactorChannel->pParentReactor, pReactorChannel) == RSSL_RET_SUCCESS, ret, &pReactorWorker->workerCerr))
				{
					_reactorWorkerShutdown(pReactorChannel->pParentReactor, &pReactorWorker->workerCerr);
				}
			}
		}
	}
	else
	{
		rsslSetErrorInfo(&pReactorChannel->channelWorkerCerr, RSSL_EIC_FAILURE, RSSL_RET_FAILURE, __FILE__, __LINE__,
			"Received HTTP error %u status code with data body : %s.", restresponse->statusCode, restresponse->dataBody.data);

		if (pReactorConnectInfoImpl->reactorChannelInfoImplState == RSSL_RC_CHINFO_IMPL_ST_REQ_AUTH_TOKEN)
		{
			handlingAuthRequestFailure(pReactorChannel, pReactorWorker, pReactorConnectInfoImpl);
		}
		else if (pReactorConnectInfoImpl->reactorChannelInfoImplState == RSSL_RC_CHINFO_IMPL_ST_QUERYING_SERVICE_DISOVERY)
		{
			/* Notify error back to the application via the channel event */
			if (!RSSL_ERROR_INFO_CHECK(_reactorWorkerHandleChannelFailure(pReactorChannel->pParentReactor, pReactorChannel, &pReactorChannel->channelWorkerCerr) == RSSL_RET_SUCCESS, RSSL_RET_FAILURE, &pReactorWorker->workerCerr))
			{
				_reactorWorkerShutdown(pReactorChannel->pParentReactor, &pReactorWorker->workerCerr);
			}

			pReactorConnectInfoImpl->reactorChannelInfoImplState = RSSL_RC_CHINFO_IMPL_ST_REQUEST_FAILURE;
		}
	}

	return;

RequestFailed:
	rsslSetErrorInfo(&pReactorChannel->channelWorkerCerr, RSSL_EIC_FAILURE, RSSL_RET_FAILURE, __FILE__, __LINE__,
		"Failed to send the REST request. Text: %s",  rsslError.text);

	if (pReactorConnectInfoImpl->reactorChannelInfoImplState == RSSL_RC_CHINFO_IMPL_ST_QUERYING_SERVICE_DISOVERY)
	{
		/* Notify error back to the application via the channel event */
		if (!RSSL_ERROR_INFO_CHECK(_reactorWorkerHandleChannelFailure(pReactorChannel->pParentReactor, pReactorChannel, &pReactorChannel->channelWorkerCerr) == RSSL_RET_SUCCESS, RSSL_RET_FAILURE, &pReactorWorker->workerCerr))
		{
			_reactorWorkerShutdown(pReactorChannel->pParentReactor, &pReactorWorker->workerCerr);
		}

		pReactorConnectInfoImpl->reactorChannelInfoImplState = RSSL_RC_CHINFO_IMPL_ST_REQUEST_FAILURE;
	}
}

static void rsslRestErrorCallback(RsslError* rsslError, RsslRestResponseEvent* event)
{
	RsslReactorChannelImpl *pReactorChannel = (RsslReactorChannelImpl*)event->closure;
	RsslReactorWorker *pReactorWorker = &pReactorChannel->pParentReactor->reactorWorker;
	RsslReactorConnectInfoImpl *pReactorConnectInfoImpl = &pReactorChannel->connectionOptList[pReactorChannel->connectionListIter];
	
	rsslSetErrorInfo(&pReactorChannel->channelWorkerCerr, RSSL_EIC_FAILURE, RSSL_RET_FAILURE, __FILE__, __LINE__,
		"Failed to send the REST request. Text: %s", rsslError->text);

	/* Reset the HTTP response status code as there is no response */
	pReactorChannel->httpStausCode = 0;

	if (rsslError->rsslErrorId == RSSL_RET_BUFFER_TOO_SMALL)
	{
		/* Notify error back to the application via the channel event */
		if (!RSSL_ERROR_INFO_CHECK(_reactorWorkerHandleChannelFailure(pReactorChannel->pParentReactor, pReactorChannel, &pReactorChannel->channelWorkerCerr) == RSSL_RET_SUCCESS, RSSL_RET_FAILURE, &pReactorWorker->workerCerr))
		{
			_reactorWorkerShutdown(pReactorChannel->pParentReactor, &pReactorWorker->workerCerr);
		}

		pReactorConnectInfoImpl->reactorChannelInfoImplState = RSSL_RC_CHINFO_IMPL_ST_BUFFER_TOO_SMALL;
	}
	else
	{
		if (pReactorConnectInfoImpl->reactorChannelInfoImplState == RSSL_RC_CHINFO_IMPL_ST_REQ_AUTH_TOKEN)
		{
			handlingAuthRequestFailure(pReactorChannel, pReactorWorker, pReactorConnectInfoImpl);
		}
		else if (pReactorConnectInfoImpl->reactorChannelInfoImplState == RSSL_RC_CHINFO_IMPL_ST_QUERYING_SERVICE_DISOVERY)
		{
			/* Notify error back to the application via the channel event */
			if (!RSSL_ERROR_INFO_CHECK(_reactorWorkerHandleChannelFailure(pReactorChannel->pParentReactor, pReactorChannel, &pReactorChannel->channelWorkerCerr) == RSSL_RET_SUCCESS, RSSL_RET_FAILURE, &pReactorWorker->workerCerr))
			{
				_reactorWorkerShutdown(pReactorChannel->pParentReactor, &pReactorWorker->workerCerr);
			}

			pReactorConnectInfoImpl->reactorChannelInfoImplState = RSSL_RC_CHINFO_IMPL_ST_REQUEST_FAILURE;
		}
	}

	/* Cleaning up the RsslRestHandle later by the ReactorWorker */
	rsslQueueAddLinkToBack(&pReactorWorker->disposableRestHandles,&event->handle->queueLink);
}

