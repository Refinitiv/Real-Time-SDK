/*
 * This source code is provided under the Apache 2.0 license and is provided
 * AS IS with no warranty or guarantee of fit for purpose.  See the project's 
 * LICENSE.md for details. 
 * Copyright Thomson Reuters 2015. All rights reserved.
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

/* Default select timeout in milliseconds if the worker thread is not waiting on a new ping, reconnection, or connection timeout event. */
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

/* Recalculate the "ping interval." It is the maximum time the worker will block in select(), since it must check for pings
 * it may need to send. */
static void _reactorWorkerCalculateNextTimeout(RsslReactorImpl *pReactorImpl, RsslUInt32 newPingTimeoutSeconds);

static void _reactorWorkerMoveChannel(RsslQueue *pNewList, RsslReactorChannelImpl *pReactorChannel);

/* Shutdown the worker due to some error. Sends a request to the reactor and waits for response, then shuts down. */
void _reactorWorkerShutdown(RsslReactorImpl *pReactorImpl, RsslErrorInfo *pErrorInfo);

/* Shuts down in response to a request from the reactor */
void _reactorWorkerHandleShutdownRequest(RsslReactorImpl *pReactorImpl, RsslErrorInfo *pErrorInfo);

/* Setup and start the worker thread (Should be called from rsslCreateReactor) */
RsslRet _reactorWorkerStart(RsslReactorImpl *pReactorImpl, RsslCreateReactorOptions *pReactorOptions, RsslErrorInfo *pError);

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

	if ((pReactorWorker->readFds = (fd_set*)malloc(pReactorImpl->fdSetSizeInBytes)) == NULL)
	{
		rsslSetErrorInfo(pError, RSSL_EIC_FAILURE, RSSL_RET_FAILURE, __FILE__, __LINE__, "Failed to allocate memory.");
		return RSSL_RET_FAILURE;
	}

	if ((pReactorWorker->writeFds = (fd_set*)malloc(pReactorImpl->fdSetSizeInBytes)) == NULL)
	{
		rsslSetErrorInfo(pError, RSSL_EIC_FAILURE, RSSL_RET_FAILURE, __FILE__, __LINE__, "Failed to allocate memory.");
		return RSSL_RET_FAILURE;
	}

	if ((pReactorWorker->exceptFds = (fd_set*)malloc(pReactorImpl->fdSetSizeInBytes)) == NULL)
	{
		rsslSetErrorInfo(pError, RSSL_EIC_FAILURE, RSSL_RET_FAILURE, __FILE__, __LINE__, "Failed to allocate memory.");
		return RSSL_RET_FAILURE;
	}

	if ((pReactorWorker->useReadFds = (fd_set*)malloc(pReactorImpl->fdSetSizeInBytes)) == NULL)
	{
		rsslSetErrorInfo(pError, RSSL_EIC_FAILURE, RSSL_RET_FAILURE, __FILE__, __LINE__, "Failed to allocate memory.");
		return RSSL_RET_FAILURE;
	}

	if ((pReactorWorker->useWriteFds = (fd_set*)malloc(pReactorImpl->fdSetSizeInBytes)) == NULL)
	{
		rsslSetErrorInfo(pError, RSSL_EIC_FAILURE, RSSL_RET_FAILURE, __FILE__, __LINE__, "Failed to allocate memory.");
		return RSSL_RET_FAILURE;
	}

	if ((pReactorWorker->useExceptFds = (fd_set*)malloc(pReactorImpl->fdSetSizeInBytes)) == NULL)
	{
		rsslSetErrorInfo(pError, RSSL_EIC_FAILURE, RSSL_RET_FAILURE, __FILE__, __LINE__, "Failed to allocate memory.");
		return RSSL_RET_FAILURE;
	}

	memset(pReactorWorker->readFds, 0, pReactorImpl->fdSetSizeInBytes);
	memset(pReactorWorker->writeFds, 0, pReactorImpl->fdSetSizeInBytes);
	memset(pReactorWorker->exceptFds, 0, pReactorImpl->fdSetSizeInBytes);
	memset(pReactorWorker->useReadFds, 0, pReactorImpl->fdSetSizeInBytes);
	memset(pReactorWorker->useWriteFds, 0, pReactorImpl->fdSetSizeInBytes);
	memset(pReactorWorker->useExceptFds, 0, pReactorImpl->fdSetSizeInBytes);

	eventQueueFd = rsslGetEventQueueGroupSignalFD(&pReactorImpl->reactorWorker.activeEventQueueGroup);
	FD_SET(eventQueueFd, pReactorImpl->reactorWorker.readFds);
	FD_SET(eventQueueFd, pReactorImpl->reactorWorker.exceptFds);

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
	while (pLink = rsslQueueRemoveFirstLink(&pReactorImpl->channelPool))
	{
		RsslReactorChannelImpl *pReactorChannel = RSSL_QUEUE_LINK_TO_OBJECT(RsslReactorChannelImpl, reactorQueueLink, pLink);
		_rsslChannelFreeConnectionList(pReactorChannel);
		rsslCleanupReactorEventQueue(&pReactorChannel->eventQueue);
		free(pReactorChannel);
	}
	while (pLink = rsslQueueRemoveFirstLink(&pReactorImpl->initializingChannels))
	{
		RsslReactorChannelImpl *pReactorChannel = RSSL_QUEUE_LINK_TO_OBJECT(RsslReactorChannelImpl, reactorQueueLink, pLink);
		_rsslChannelFreeConnectionList(pReactorChannel);
		rsslCleanupReactorEventQueue(&pReactorChannel->eventQueue);
		if (pReactorChannel->pWatchlist)
			rsslWatchlistDestroy(pReactorChannel->pWatchlist);
		free(pReactorChannel);
	}
	while (pLink = rsslQueueRemoveFirstLink(&pReactorImpl->activeChannels))
	{
		RsslReactorChannelImpl *pReactorChannel = RSSL_QUEUE_LINK_TO_OBJECT(RsslReactorChannelImpl, reactorQueueLink, pLink);
		_rsslChannelFreeConnectionList(pReactorChannel);
		rsslCleanupReactorEventQueue(&pReactorChannel->eventQueue);
		if (pReactorChannel->pWatchlist)
			rsslWatchlistDestroy(pReactorChannel->pWatchlist);
		free(pReactorChannel);
	}
	while (pLink = rsslQueueRemoveFirstLink(&pReactorImpl->inactiveChannels))
	{
		RsslReactorChannelImpl *pReactorChannel = RSSL_QUEUE_LINK_TO_OBJECT(RsslReactorChannelImpl, reactorQueueLink, pLink);
		_rsslChannelFreeConnectionList(pReactorChannel);
		rsslCleanupReactorEventQueue(&pReactorChannel->eventQueue);
		if (pReactorChannel->pWatchlist)
			rsslWatchlistDestroy(pReactorChannel->pWatchlist);
		free(pReactorChannel);
	}
	while (pLink = rsslQueueRemoveFirstLink(&pReactorImpl->closingChannels))
	{
		RsslReactorChannelImpl *pReactorChannel = RSSL_QUEUE_LINK_TO_OBJECT(RsslReactorChannelImpl, reactorQueueLink, pLink);
		_rsslChannelFreeConnectionList(pReactorChannel);
		rsslCleanupReactorEventQueue(&pReactorChannel->eventQueue);
		if (pReactorChannel->pWatchlist)
			rsslWatchlistDestroy(pReactorChannel->pWatchlist);
		free(pReactorChannel);
	}
	while (pLink = rsslQueueRemoveFirstLink(&pReactorImpl->reconnectingChannels))
	{
		RsslReactorChannelImpl *pReactorChannel = RSSL_QUEUE_LINK_TO_OBJECT(RsslReactorChannelImpl, reactorQueueLink, pLink);
		_rsslChannelFreeConnectionList(pReactorChannel);
		rsslCleanupReactorEventQueue(&pReactorChannel->eventQueue);
		if (pReactorChannel->pWatchlist)
			rsslWatchlistDestroy(pReactorChannel->pWatchlist);
		free(pReactorChannel);
	}

	if (pReactorImpl->readFds)
		free(pReactorImpl->readFds);

	if (pReactorImpl->exceptFds)
		free(pReactorImpl->exceptFds);

	if (pReactorImpl->useReadFds)
		free(pReactorImpl->useReadFds);

	if (pReactorImpl->useExceptFds)
		free(pReactorImpl->useExceptFds);

	if (pReactorImpl->memoryBuffer.data)
		free(pReactorImpl->memoryBuffer.data);

	if (pReactorWorker->readFds)
		free(pReactorWorker->readFds);

	if (pReactorWorker->writeFds)
		free(pReactorWorker->writeFds);

	if (pReactorWorker->exceptFds)
		free(pReactorWorker->exceptFds);

	if (pReactorWorker->useReadFds)
		free(pReactorWorker->useReadFds);

	if (pReactorWorker->useWriteFds)
		free(pReactorWorker->useWriteFds);

	if (pReactorWorker->useExceptFds)
		free(pReactorWorker->useExceptFds);

	RSSL_MUTEX_DESTROY(&pReactorImpl->interfaceLock);
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
	FD_CLR(pReactorChannel->reactorChannel.pRsslChannel->socketId, pReactorWorker->readFds);
	FD_CLR(pReactorChannel->reactorChannel.pRsslChannel->socketId, pReactorWorker->writeFds);
	FD_CLR(pReactorChannel->reactorChannel.pRsslChannel->socketId, pReactorWorker->exceptFds);

	_reactorWorkerMoveChannel(&pReactorWorker->activeChannels, pReactorChannel);
	_reactorWorkerCalculateNextTimeout(pReactorImpl, (RsslUInt32)(pReactorChannel->reactorChannel.pRsslChannel->pingTimeout * 1000 * pingIntervalFactor));
	pReactorChannel->lastPingSentMs = pReactorWorker->lastRecordedTimeMs;

	/* Copy RsslChannel parameters */
	pReactorChannel->reactorChannel.socketId = pReactorChannel->reactorChannel.pRsslChannel->socketId;
	pReactorChannel->reactorChannel.oldSocketId = pReactorChannel->reactorChannel.pRsslChannel->oldSocketId;
	pReactorChannel->reactorChannel.majorVersion = pReactorChannel->reactorChannel.pRsslChannel->majorVersion;
	pReactorChannel->reactorChannel.minorVersion = pReactorChannel->reactorChannel.pRsslChannel->minorVersion;
	pReactorChannel->reactorChannel.protocolType = pReactorChannel->reactorChannel.pRsslChannel->protocolType;

	pReactorChannel->reconnectAttemptCount = 0;
	pReactorChannel->lastReconnectAttemptMs = 0;

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


	if(pReactorChannel->reactorChannel.pRsslChannel != 0)
	{
		FD_CLR(pReactorChannel->reactorChannel.pRsslChannel->socketId, pReactorWorker->readFds);
		FD_CLR(pReactorChannel->reactorChannel.pRsslChannel->socketId, pReactorWorker->exceptFds);
		FD_CLR(pReactorChannel->reactorChannel.pRsslChannel->socketId, pReactorWorker->writeFds);
	}

	_reactorWorkerMoveChannel(&pReactorImpl->reactorWorker.inactiveChannels, pReactorChannel);

	rsslClearReactorChannelEventImpl(pEvent);

	pEvent->channelEvent.channelEventType = RSSL_RC_CET_CHANNEL_DOWN;
	pEvent->channelEvent.pReactorChannel = (RsslReactorChannel*)pReactorChannel;
	pEvent->channelEvent.pError = pError;

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
	int eventQueueFd = rsslGetEventQueueGroupSignalFD(&pReactorWorker->activeEventQueueGroup);

	RsslInt32 activeChannelSize = 0;

	pReactorWorker->sleepTimeMs = 3000;


	while (1)
	{
		RsslRet ret;
		RsslReactorChannelImpl *pReactorChannel;
		RsslQueueLink *pLink;

		struct timeval selectTime;
		memcpy(pReactorWorker->useReadFds, pReactorWorker->readFds, pReactorImpl->fdSetSizeInBytes);
		memcpy(pReactorWorker->useWriteFds, pReactorWorker->writeFds, pReactorImpl->fdSetSizeInBytes);
		memcpy(pReactorWorker->useExceptFds, pReactorWorker->exceptFds, pReactorImpl->fdSetSizeInBytes);

		selectTime.tv_sec = pReactorWorker->sleepTimeMs / 1000;
		selectTime.tv_usec = (pReactorWorker->sleepTimeMs - (selectTime.tv_sec * 1000)) * 1000;

		ret = select(pReactorImpl->fdSetSize, pReactorWorker->useReadFds, pReactorWorker->useWriteFds, pReactorWorker->useExceptFds, &selectTime);

		pReactorWorker->lastRecordedTimeMs = getCurrentTimeMs(pReactorImpl->ticksPerMsec);


		if (ret > 0)
		{
			RSSL_QUEUE_FOR_EACH_LINK(&pReactorWorker->initializingChannels, pLink)
			{
				pReactorChannel = RSSL_QUEUE_LINK_TO_OBJECT(RsslReactorChannelImpl, workerLink, pLink);

				if (FD_ISSET(pReactorChannel->reactorChannel.pRsslChannel->socketId, pReactorWorker->useReadFds) ||
					FD_ISSET(pReactorChannel->reactorChannel.pRsslChannel->socketId, pReactorWorker->useWriteFds) ||
					FD_ISSET(pReactorChannel->reactorChannel.pRsslChannel->socketId, pReactorWorker->useExceptFds))
				{
					RsslInProgInfo inProg = RSSL_INIT_IN_PROG_INFO;

					/* Write descriptor is set initially in case this end starts the message hanshakes that rsslInitChannel() performs. 
					 * Once rsslInitChannel() is called for the first time the channel can wait on the read descriptor for more messages.  
					 * We will set the write descriptor again if an FD_CHANGE event occurs. */
					FD_CLR(pReactorChannel->reactorChannel.pRsslChannel->socketId, pReactorWorker->writeFds);
					ret = rsslInitChannel(pReactorChannel->reactorChannel.pRsslChannel, &inProg, &pReactorChannel->channelWorkerCerr.rsslError);


					switch(ret)
					{
						case RSSL_RET_CHAN_INIT_IN_PROGRESS:
							{
								if (inProg.flags & RSSL_IP_FD_CHANGE)
								{
									/* File descriptor changed. Update descriptor set.
									 * No need to send an event back -- the application will only get the descriptors when we're done initializing. */
									FD_CLR(inProg.oldSocket, pReactorWorker->exceptFds);
									FD_CLR(inProg.oldSocket, pReactorWorker->readFds);

									FD_SET(inProg.newSocket, pReactorWorker->readFds);
									FD_SET(inProg.newSocket, pReactorWorker->exceptFds);
									FD_SET(inProg.newSocket, pReactorWorker->writeFds);
								}

								break;
							}

						case RSSL_RET_SUCCESS:
							{
								if (_reactorWorkerProcessChannelUp(pReactorImpl, pReactorChannel) != RSSL_RET_SUCCESS)
									return (_reactorWorkerShutdown(pReactorImpl, &pReactorWorker->workerCerr), RSSL_THREAD_RETURN());
								/* Channel is now active. Clear descriptor so we don't think we need to flush right away */
								FD_CLR(pReactorChannel->reactorChannel.pRsslChannel->socketId, pReactorWorker->useWriteFds);
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

			}

			RSSL_QUEUE_FOR_EACH_LINK(&pReactorWorker->activeChannels, pLink)
			{
				pReactorChannel = RSSL_QUEUE_LINK_TO_OBJECT(RsslReactorChannelImpl, workerLink, pLink);
				if (FD_ISSET(pReactorChannel->reactorChannel.pRsslChannel->socketId, pReactorWorker->useWriteFds))
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
						FD_CLR(pReactorChannel->reactorChannel.pRsslChannel->socketId, pReactorWorker->writeFds);
						_reactorWorkerSendFlushComplete(pReactorImpl, pReactorChannel);
					}
				}
			}

			if (FD_ISSET(eventQueueFd, pReactorWorker->useReadFds))
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

							switch(pConnEvent->channelEvent.channelEventType)
							{
								case RSSL_RCIMPL_CET_NEW_CHANNEL:
								{
									if (!RSSL_ERROR_INFO_CHECK(_reactorWorkerProcessNewChannel(pReactorImpl, pReactorChannel) == RSSL_RET_SUCCESS, ret, &pReactorWorker->workerCerr))
										return (_reactorWorkerShutdown(pReactorImpl, &pReactorWorker->workerCerr), RSSL_THREAD_RETURN());
									break;
								}
								case RSSL_RC_CET_CHANNEL_DOWN:
								{
									/* Make sure descriptors are cleared */
									FD_CLR(pReactorChannel->reactorChannel.pRsslChannel->socketId, pReactorWorker->readFds);
									FD_CLR(pReactorChannel->reactorChannel.pRsslChannel->socketId, pReactorWorker->exceptFds);
									FD_CLR(pReactorChannel->reactorChannel.pRsslChannel->socketId, pReactorWorker->writeFds);

									/* Remove channel from worker's list */
									_reactorWorkerMoveChannel(&pReactorWorker->inactiveChannels, pReactorChannel);

									break;
								}
								case RSSL_RC_CET_CHANNEL_DOWN_RECONNECTING:
								{
									_reactorWorkerMoveChannel(&pReactorImpl->reactorWorker.reconnectingChannels, pReactorChannel);

									if(pReactorChannel->reactorChannel.pRsslChannel != 0)
									{
										/* Make sure descriptors are cleared */
										FD_CLR(pReactorChannel->reactorChannel.pRsslChannel->socketId, pReactorWorker->readFds);
										FD_CLR(pReactorChannel->reactorChannel.pRsslChannel->socketId, pReactorWorker->exceptFds);
										FD_CLR(pReactorChannel->reactorChannel.pRsslChannel->socketId, pReactorWorker->writeFds);

										pReactorChannel->reactorChannel.socketId = REACTOR_INVALID_SOCKET;

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
										FD_CLR(pReactorChannel->reactorChannel.pRsslChannel->socketId, pReactorWorker->readFds);
										FD_CLR(pReactorChannel->reactorChannel.pRsslChannel->socketId, pReactorWorker->exceptFds);
										FD_CLR(pReactorChannel->reactorChannel.pRsslChannel->socketId, pReactorWorker->writeFds);
									}

									/* Close RSSL channel */
									if (!RSSL_ERROR_INFO_CHECK((ret = rsslCloseChannel(pReactorChannel->reactorChannel.pRsslChannel, &pReactorWorker->workerCerr.rsslError)) >= RSSL_RET_SUCCESS, RSSL_RET_FAILURE, &pReactorWorker->workerCerr))
									{
										rsslSetErrorInfoLocation(&pReactorChannel->channelWorkerCerr, __FILE__, __LINE__);
										return (_reactorWorkerShutdown(pReactorImpl, &pReactorWorker->workerCerr), RSSL_THREAD_RETURN());
									}

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
									FD_SET(pReactorChannel->reactorChannel.pRsslChannel->socketId, pReactorWorker->writeFds);
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
		}
		else if (ret < 0)
		{
			RsslQueueLink *pLink;
#ifdef WIN32
			switch(WSAGetLastError())
#else
			switch(errno)
#endif
			{
#ifdef WIN32
				case WSAENOTSOCK:
#else
				case EBADF:
#endif
					/* When a bad file descriptor is encountered in the reactorWorker, it is likely because rsslRead() 
					 * received an FD_CHANGE event. Reset all descriptors and try again. */
					memset(pReactorWorker->readFds, 0, pReactorImpl->fdSetSizeInBytes);
					memset(pReactorWorker->writeFds, 0, pReactorImpl->fdSetSizeInBytes);
					memset(pReactorWorker->exceptFds, 0, pReactorImpl->fdSetSizeInBytes);

					FD_SET(eventQueueFd, pReactorWorker->readFds);
					FD_SET(eventQueueFd, pReactorWorker->exceptFds);

					/* Set all channel descriptors again, including write descriptors(this may result in extra "flush done" events being sent back). */
					RSSL_QUEUE_FOR_EACH_LINK(&pReactorWorker->activeChannels, pLink)
					{
						pReactorChannel = RSSL_QUEUE_LINK_TO_OBJECT(RsslReactorChannelImpl, workerLink, pLink);
						FD_SET(pReactorChannel->reactorChannel.pRsslChannel->socketId, pReactorWorker->writeFds);
					}
					RSSL_QUEUE_FOR_EACH_LINK(&pReactorWorker->initializingChannels, pLink)
					{
						pReactorChannel = RSSL_QUEUE_LINK_TO_OBJECT(RsslReactorChannelImpl, workerLink, pLink);
						FD_SET(pReactorChannel->reactorChannel.pRsslChannel->socketId, pReactorWorker->writeFds);
						FD_SET(pReactorChannel->reactorChannel.pRsslChannel->socketId, pReactorWorker->readFds);
						FD_SET(pReactorChannel->reactorChannel.pRsslChannel->socketId, pReactorWorker->exceptFds);
					}

					continue;

#ifdef WIN32
				case WSAEINTR:
#else
				case EINTR:
#endif
					continue;

				default:
					rsslSetErrorInfo(&pReactorWorker->workerCerr, RSSL_EIC_FAILURE, RSSL_RET_FAILURE, __FILE__, __LINE__, 
#ifdef WIN32
							"select() failed: %d", WSAGetLastError()
#else
							"select() failed: %d", errno
#endif
							);
					return (_reactorWorkerShutdown(pReactorImpl, &pReactorWorker->workerCerr), RSSL_THREAD_RETURN());
			}
		}

		/* Ping/initialization/recovery timeout check */

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
			if ((pReactorWorker->lastRecordedTimeMs - pReactorChannel->lastPingSentMs) > pReactorChannel->reactorChannel.pRsslChannel->pingTimeout * 1000 * pingIntervalFactor )
			{
				ret = rsslPing(pReactorChannel->reactorChannel.pRsslChannel, &pReactorChannel->channelWorkerCerr.rsslError);
				if (ret < 0)
				{
					rsslSetErrorInfoLocation(&pReactorChannel->channelWorkerCerr, __FILE__, __LINE__);
					if (!RSSL_ERROR_INFO_CHECK(_reactorWorkerHandleChannelFailure(pReactorImpl, pReactorChannel, &pReactorChannel->channelWorkerCerr) == RSSL_RET_SUCCESS, RSSL_RET_FAILURE, &pReactorWorker->workerCerr))
						return (_reactorWorkerShutdown(pReactorImpl, &pReactorWorker->workerCerr), RSSL_THREAD_RETURN());
				}
				else
					pReactorChannel->lastPingSentMs = pReactorWorker->lastRecordedTimeMs;
			}
			else
			{
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
		}

		RSSL_QUEUE_FOR_EACH_LINK(&pReactorWorker->reconnectingChannels, pLink)
		{
			pReactorChannel = RSSL_QUEUE_LINK_TO_OBJECT(RsslReactorChannelImpl, workerLink, pLink);
			
			if((pReactorWorker->lastRecordedTimeMs - pReactorChannel->lastReconnectAttemptMs) >= pReactorChannel->reconnectDelay)
				/* Attempt to reconnect here */
			{

				pReactorChannel->connectionListIter++;

				if(pReactorChannel->connectionListIter >= pReactorChannel->connectionListCount)
				{
					pReactorChannel->connectionListIter = 0;
				}

				pReactorChannel->reactorChannel.userSpecPtr = pReactorChannel->connectionOptList[pReactorChannel->connectionListIter].rsslConnectOptions.userSpecPtr;

				if (!(pReactorChannel->reactorChannel.pRsslChannel = rsslConnect(&(pReactorChannel->connectionOptList[pReactorChannel->connectionListIter].rsslConnectOptions), &pReactorChannel->channelWorkerCerr.rsslError)))
				{
					if (!RSSL_ERROR_INFO_CHECK(_reactorWorkerHandleChannelFailure(pReactorImpl, pReactorChannel, &pReactorChannel->channelWorkerCerr) == RSSL_RET_SUCCESS, RSSL_RET_FAILURE, &pReactorWorker->workerCerr))
						return (_reactorWorkerShutdown(pReactorImpl, &pReactorWorker->workerCerr), RSSL_THREAD_RETURN());
				}
				else
				{
					pReactorChannel->channelSetupState = RSSL_RC_CHST_INIT;
					pReactorChannel->initializationTimeout = pReactorChannel->connectionOptList[pReactorChannel->connectionListIter].initializationTimeout;
					if (!RSSL_ERROR_INFO_CHECK(_reactorWorkerProcessNewChannel(pReactorImpl, pReactorChannel) == RSSL_RET_SUCCESS, ret, &pReactorWorker->workerCerr))
						return (_reactorWorkerShutdown(pReactorImpl, &pReactorWorker->workerCerr), RSSL_THREAD_RETURN());
				}
		}
			else
			{
				_reactorWorkerCalculateNextTimeout(pReactorImpl, (RsslUInt32)(pReactorChannel->lastReconnectAttemptMs + pReactorChannel->reconnectDelay - pReactorWorker->lastRecordedTimeMs));
			}
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

	RSSL_QUEUE_FOR_EACH_LINK(&pReactorWorker->inactiveChannels, pLink)
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
	FD_SET(pReactorChannel->reactorChannel.pRsslChannel->socketId, pReactorWorker->writeFds);

	

	switch (pReactorChannel->reactorChannel.pRsslChannel->state)
	{
		case RSSL_CH_STATE_INITIALIZING:
			_reactorWorkerMoveChannel(&pReactorWorker->initializingChannels, pReactorChannel);
			_reactorWorkerCalculateNextTimeout(pReactorImpl, pReactorChannel->initializationTimeout*1000);
			pReactorChannel->initializationStartTimeMs = pReactorWorker->lastRecordedTimeMs;
			FD_SET(pReactorChannel->reactorChannel.pRsslChannel->socketId, pReactorWorker->readFds);
			FD_SET(pReactorChannel->reactorChannel.pRsslChannel->socketId, pReactorWorker->exceptFds);
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
