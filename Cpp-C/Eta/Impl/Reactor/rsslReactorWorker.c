/*
 * This source code is provided under the Apache 2.0 license and is provided
 * AS IS with no warranty or guarantee of fit for purpose.  See the project's 
 * LICENSE.md for details. 
 * Copyright (C) 2019-2020 Refinitiv. All rights reserved.
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

static RsslRestRequestArgs* _reactorWorkerCreateRequestArgsForRefreshToken(RsslReactorTokenSessionImpl *pReactorTokenSession, 
	RsslReactorImpl *pRsslReactor, RsslErrorInfo* pError);

static void rsslRestServiceDiscoveryResponseCallback(RsslRestResponse* restresponse, RsslRestResponseEvent* event);

static void rsslRestErrorCallback(RsslError* rsslError, RsslRestResponseEvent* event);

static void notifySensitiveInfoReq(RsslReactorTokenSessionImpl* pTokenSession, RsslReactorWorker* pReactorWorker);

static void rsslRestAuthTokenResponseWithoutSessionCallback(RsslRestResponse* restresponse, RsslRestResponseEvent* event);

static void rsslRestAuthTokenErrorCallback(RsslError* rsslError, RsslRestResponseEvent* event);

static void rsslRestErrorWithoutSessionCallback(RsslError* rsslError, RsslRestResponseEvent* event);

static void _reactorWorkerFreeRsslReactorOAuthCredentialRenewal(RsslReactorOAuthCredentialRenewalImpl *pReactorOAuthCredentialRenewalImpl);

static void rsslRestAuthTokenResponseCallback(RsslRestResponse* restresponse, RsslRestResponseEvent* event);

RsslRet _reactorWorkerStart(RsslReactorImpl *pReactorImpl, RsslCreateReactorOptions *pReactorOptions, RsslErrorInfo *pError)
{
	RsslReactorWorker *pReactorWorker = &pReactorImpl->reactorWorker;
	int eventQueueFd;
	int i;

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

	/* Initialize the error information pool for the session management */
	rsslInitQueue(&pReactorImpl->reactorWorker.errorInfoPool);
	rsslInitQueue(&pReactorImpl->reactorWorker.errorInfoInUsedPool);
	RSSL_MUTEX_INIT(&pReactorImpl->reactorWorker.errorInfoPoolLock);

	/* Initialize error information pool */
	for (i = 0; i < 5; ++i)
	{
		RsslReactorErrorInfoImpl *pReactorErrorInfoImpl = (RsslReactorErrorInfoImpl*)malloc(sizeof(RsslReactorErrorInfoImpl));

		if (!pReactorErrorInfoImpl)
		{
			rsslSetErrorInfo(pError, RSSL_EIC_FAILURE, RSSL_RET_FAILURE, __FILE__, __LINE__, "Failed to create reactorWorker error pool.");
			return RSSL_RET_FAILURE;
		}

		rsslInitQueueLink(&pReactorErrorInfoImpl->poolLink);
		rsslQueueAddLinkToBack(&pReactorImpl->reactorWorker.errorInfoPool, &pReactorErrorInfoImpl->poolLink);
	}

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

	/* Clears the token management */
	rsslClearReactorTokenManagementImpl(&pReactorWorker->reactorTokenManagement);

	RSSL_MUTEX_LOCK(&pReactorWorker->reactorTokenManagement.tokenSessionMutex);

	/* Initializes the Hash table to keep the current token session */
	if (rsslHashTableInit(&pReactorWorker->reactorTokenManagement.sessionByNameAndClientIdHt, 10, rsslHashBufferSum,
		rsslHashBufferCompare, RSSL_TRUE, pError) != RSSL_RET_SUCCESS)
	{
		rsslSetErrorInfo(pError, RSSL_EIC_FAILURE, RSSL_RET_FAILURE, __FILE__, __LINE__, "Failed to initialize RsslHashTable for token management.");
		return RSSL_RET_FAILURE;
	}

	RSSL_MUTEX_UNLOCK(&pReactorWorker->reactorTokenManagement.tokenSessionMutex);

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
		_rsslCleanUpPackedBufferHashTable(pReactorChannel);
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
		_rsslCleanUpPackedBufferHashTable(pReactorChannel);
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
		_rsslCleanUpPackedBufferHashTable(pReactorChannel);
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
		_rsslCleanUpPackedBufferHashTable(pReactorChannel);
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
		_rsslCleanUpPackedBufferHashTable(pReactorChannel);
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
		_rsslCleanUpPackedBufferHashTable(pReactorChannel);
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

	/* Ensure that the worker thread is started before cleaning up its resources */
	if (pReactorImpl->rsslWorkerStarted)
	{
		while ((pLink = rsslQueueRemoveFirstLink(&pReactorWorker->reactorTokenManagement.tokenSessionList)))
		{
			RsslReactorTokenSessionImpl *pTokenSession = RSSL_QUEUE_LINK_TO_OBJECT(RsslReactorTokenSessionImpl, sessionLink, pLink);
			rsslFreeReactorTokenSessionImpl(pTokenSession);
		}

		/* Cleaning up the HashTable for keeping track of token session */
		RSSL_MUTEX_LOCK(&pReactorWorker->reactorTokenManagement.tokenSessionMutex);
		rsslHashTableCleanup(&pReactorWorker->reactorTokenManagement.sessionByNameAndClientIdHt);
		RSSL_MUTEX_UNLOCK(&pReactorWorker->reactorTokenManagement.tokenSessionMutex);

		RSSL_MUTEX_DESTROY(&pReactorWorker->reactorTokenManagement.tokenSessionMutex);

		/* Free RsslReactorErrorInfoImpl from the Reactor worker's pool */
		RSSL_MUTEX_LOCK(&pReactorWorker->errorInfoPoolLock);
		while ((pLink = rsslQueueRemoveFirstLink(&pReactorWorker->errorInfoPool)))
		{
			RsslReactorErrorInfoImpl *pReactorErrorInfoImpl = RSSL_QUEUE_LINK_TO_OBJECT(RsslReactorErrorInfoImpl, poolLink, pLink);
			free(pReactorErrorInfoImpl);
		}

		/* Free RsslReactorErrorInfoImpl from the Reactor worker's in used pool if any */
		while ((pLink = rsslQueueRemoveFirstLink(&pReactorWorker->errorInfoInUsedPool)))
		{
			RsslReactorErrorInfoImpl *pReactorErrorInfoImpl = RSSL_QUEUE_LINK_TO_OBJECT(RsslReactorErrorInfoImpl, poolLink, pLink);
			free(pReactorErrorInfoImpl);
		}

		RSSL_MUTEX_UNLOCK(&pReactorWorker->errorInfoPoolLock);

		RSSL_MUTEX_DESTROY(&pReactorWorker->errorInfoPoolLock);
	}

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

	/* Checks whether the JSON converter has been initialized and cleaning up associated memory. */
	if (pReactorImpl->jsonConverterInitialized == RSSL_TRUE)
	{
		pReactorImpl->jsonConverterInitialized = RSSL_FALSE;

		rsslJsonUninitialize();

		if (pReactorImpl->pJsonConverter)
		{
			RsslJsonConverterError rjcError;
			rsslDestroyRsslJsonConverter(pReactorImpl->pJsonConverter, &rjcError);
			pReactorImpl->pJsonConverter = NULL;
		}

		if (pReactorImpl->pDictionaryList)
		{
			free(pReactorImpl->pDictionaryList);
			pReactorImpl->pDictionaryList = NULL;
		}

		if (pReactorImpl->pJsonErrorInfo)
		{
			free(pReactorImpl->pJsonErrorInfo);
			pReactorImpl->pJsonErrorInfo = NULL;
		}
	}

	/* For EDP token management and service discovery */
	free(pReactorImpl->accessTokenRespBuffer.data);
	free(pReactorImpl->tokenInformationBuffer.data);
	free(pReactorImpl->serviceDiscoveryRespBuffer.data);
	free(pReactorImpl->restServiceEndpointRespBuf.data);
	free(pReactorImpl->argumentsAndHeaders.data);
	free(pReactorImpl->tokenServiceURLBuffer.data);
	free(pReactorImpl->serviceDiscoveryURLBuffer.data);

	free(pReactorImpl);

	/* rsslInitialize/rsslUninitialize are reference counted, so decrement the reactor's call. */
	rsslUninitialize();
	return;
}


RsslRet _reactorWorkerProcessChannelUp(RsslReactorImpl *pReactorImpl, RsslReactorChannelImpl *pReactorChannel)
{
	RsslReactorChannelEventImpl *pEvent = NULL;
	RsslReactorWorker *pReactorWorker = &pReactorImpl->reactorWorker;

	/* Notify the current token information from the token session */
	if (pReactorChannel->pTokenSessionImpl)
	{
		RsslReactorConnectInfoImpl *pReactorConnectInfoImpl = &pReactorChannel->connectionOptList[pReactorChannel->connectionListIter];

		if (pReactorConnectInfoImpl->base.enableSessionManagement)
		{
			if (pReactorConnectInfoImpl->lastTokenUpdatedTime != pReactorChannel->pTokenSessionImpl->lastTokenUpdatedTime)
			{
				RsslReactorTokenMgntEvent* pEventTokenMgntEvent;
				pReactorConnectInfoImpl->lastTokenUpdatedTime = pReactorChannel->pTokenSessionImpl->lastTokenUpdatedTime;

				pEventTokenMgntEvent = (RsslReactorTokenMgntEvent*)rsslReactorEventQueueGetFromPool(&pReactorChannel->eventQueue);
				rsslClearReactorTokenMgntEvent(pEventTokenMgntEvent);

				pReactorConnectInfoImpl->reactorChannelInfoImplState = RSSL_RC_CHINFO_IMPL_ST_RECEIVED_AUTH_TOKEN;
				pEventTokenMgntEvent->reactorTokenMgntEventType = RSSL_RCIMPL_TKET_REISSUE;
				pEventTokenMgntEvent->pReactorChannel = &pReactorChannel->reactorChannel;
				pEventTokenMgntEvent->pAuthTokenEventCallback = pReactorConnectInfoImpl->base.pAuthTokenEventCallback;
				pEventTokenMgntEvent->pTokenSessionImpl = pReactorChannel->pTokenSessionImpl;

				if (!RSSL_ERROR_INFO_CHECK(rsslReactorEventQueuePut(&pReactorChannel->eventQueue, (RsslReactorEventImpl*)pEventTokenMgntEvent)
					== RSSL_RET_SUCCESS, RSSL_RET_FAILURE, &pReactorWorker->workerCerr))
				{
					return RSSL_RET_FAILURE;
				}
			}
		}
	}

	pEvent = (RsslReactorChannelEventImpl*)rsslReactorEventQueueGetFromPool(&pReactorChannel->eventQueue);

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
					if ((pEvent = rsslReactorEventQueueGet(pEventQueue, pReactorImpl->maxEventsInPool, &ret)))
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

												/* Unregister the channel for the token session */
												if (pReactorChannel->pTokenSessionImpl)
												{
													RsslReactorTokenSessionImpl *pTokenSessionImpl = pReactorChannel->pTokenSessionImpl;

													RSSL_MUTEX_LOCK(&pReactorWorker->reactorTokenManagement.tokenSessionMutex);

													if (pReactorChannel->tokenSessionLink.next != NULL && pReactorChannel->tokenSessionLink.prev != NULL)
													{
														rsslQueueRemoveLink(&pTokenSessionImpl->reactorChannelList, &pReactorChannel->tokenSessionLink);
													}

													/* This is the case when the channel hasn't been added of the list of token session yet */
													if(pReactorChannel->addedToTokenSessionList == RSSL_FALSE)
														RTR_ATOMIC_DECREMENT(pTokenSessionImpl->numberOfWaitingChannels);
													
													pReactorChannel->pTokenSessionImpl = NULL;

													/* Ensure that all channels has been removed and there is no others channel is waiting to add to the list of token session */
													if (pTokenSessionImpl->reactorChannelList.count == 0 && pTokenSessionImpl->numberOfWaitingChannels == 0)
													{
														RsslReactorTokenSessionEvent *pEvent = NULL;

														/* Close the existing the REST request if any */
														if (pTokenSessionImpl->pRestHandle)
														{
															RsslRestHandle *pRestHandle = pTokenSessionImpl->pRestHandle;
															pTokenSessionImpl->pRestHandle = NULL;
															if (rsslRestCloseHandle(pRestHandle, &pReactorWorker->workerCerr.rsslError) != RSSL_RET_SUCCESS)
															{
																RSSL_MUTEX_UNLOCK(&pReactorWorker->reactorTokenManagement.tokenSessionMutex);
																return (_reactorWorkerShutdown(pReactorImpl, &pReactorWorker->workerCerr), RSSL_THREAD_RETURN());
															}
														}

														if(pTokenSessionImpl->sessionLink.next != NULL && pTokenSessionImpl->sessionLink.prev != NULL)
															rsslQueueRemoveLink(&pReactorWorker->reactorTokenManagement.tokenSessionList, &pTokenSessionImpl->sessionLink);
														
														rsslHashTableRemoveLink(&pReactorWorker->reactorTokenManagement.sessionByNameAndClientIdHt, &pTokenSessionImpl->hashLinkNameAndClientId);
														rsslFreeReactorTokenSessionImpl(pTokenSessionImpl);
													}

													RSSL_MUTEX_UNLOCK(&pReactorWorker->reactorTokenManagement.tokenSessionMutex);
												}

												/* Close the existing the REST request if any */
												if (pReactorChannel->pRestHandle)
												{
													RsslRestHandle *pRestHandle = pReactorChannel->pRestHandle;
													pReactorChannel->pRestHandle = NULL;
													if (rsslRestCloseHandle(pRestHandle, &pReactorWorker->workerCerr.rsslError) != RSSL_RET_SUCCESS)
													{
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
							case RSSL_RCIMPL_ET_CREDENTIAL_RENEWAL:
								{
									RsslReactorCredentialRenewalEvent *pReactorCredentialRenewalEvent = (RsslReactorCredentialRenewalEvent*)pEvent;
									RsslReactorOAuthCredentialRenewalImpl *pOAuthCredentialRenewalImpl = (RsslReactorOAuthCredentialRenewalImpl*)pReactorCredentialRenewalEvent->pOAuthCredentialRenewal;
									RsslReactorTokenSessionImpl *pTokenSessionImpl = pOAuthCredentialRenewalImpl->pParentTokenSession;

									switch (pReactorCredentialRenewalEvent->reactorCredentialRenewalEventType)
									{
										case RSSL_RCIMPL_CRET_AUTH_REQ_WITH_PASSWORD:
										case RSSL_RCIMPL_CRET_AUTH_REQ_WITH_PASSWORD_CHANGE:
										{
											RsslReactorConnectInfoImpl *pReactorConnectInfoImpl = NULL;
											void* pUserSpec = NULL;
											RsslRestRequestArgs * pRestRequestArgs;
											RsslBuffer *pNewPasword = NULL;
											RsslReactorImpl *pReactorImpl = (RsslReactorImpl*)(pTokenSessionImpl ? pTokenSessionImpl->pReactor : pOAuthCredentialRenewalImpl->pRsslReactor);
											RsslBuffer tokenServiceURL = (pTokenSessionImpl == 0) ? pReactorImpl->tokenServiceURL : (pTokenSessionImpl->temporaryURL.data == 0) ? pReactorImpl->tokenServiceURL : pTokenSessionImpl->temporaryURL;
												
											if (pTokenSessionImpl)
											{
												pUserSpec = (void*)pTokenSessionImpl;
											}
											else
											{
												pUserSpec = (void*)pOAuthCredentialRenewalImpl;
											}

											/* Handling error cases to get authentication token using the password */
												pRestRequestArgs = _reactorCreateRequestArgsForPassword(pReactorImpl,
												&tokenServiceURL,
												pTokenSessionImpl ? (&pTokenSessionImpl->pOAuthCredential->userName) : (&pOAuthCredentialRenewalImpl->reactorOAuthCredentialRenewal.userName),
												&pOAuthCredentialRenewalImpl->reactorOAuthCredentialRenewal.password, // Specified by users as needed.
												(pReactorCredentialRenewalEvent->reactorCredentialRenewalEventType == RSSL_RCIMPL_CRET_AUTH_REQ_WITH_PASSWORD_CHANGE) ? 
												&pOAuthCredentialRenewalImpl->reactorOAuthCredentialRenewal.newPassword : NULL,
												pTokenSessionImpl ? (&pTokenSessionImpl->pOAuthCredential->clientId) : (&pOAuthCredentialRenewalImpl->reactorOAuthCredentialRenewal.clientId),
												&pOAuthCredentialRenewalImpl->reactorOAuthCredentialRenewal.clientSecret, // Specified by users as needed.
												pTokenSessionImpl ? (&pTokenSessionImpl->pOAuthCredential->tokenScope) : (&pOAuthCredentialRenewalImpl->reactorOAuthCredentialRenewal.tokenScope),
												pTokenSessionImpl ? &pTokenSessionImpl->rsslPostDataBodyBuf : &pOAuthCredentialRenewalImpl->rsslPostDataBodyBuf,
												pUserSpec, pTokenSessionImpl ? &pTokenSessionImpl->tokenSessionWorkerCerr : &pReactorWorker->workerCerr);

											if (pRestRequestArgs)
											{
												RsslRestHandle* pRsslRestHandle = NULL;

												if (pTokenSessionImpl)
												{
													/* Checks whether the access token is being request by the user's thread */
													if (pTokenSessionImpl->requestingAccessToken == 1)
													{
														/* Clears sensitive information */
														rsslClearReactorOAuthCredentialRenewal(&pOAuthCredentialRenewalImpl->reactorOAuthCredentialRenewal);
														free(pRestRequestArgs);
														rsslClearBuffer(&pTokenSessionImpl->temporaryURL);
														break;
													}

													RSSL_MUTEX_LOCK(&pTokenSessionImpl->accessTokenMutex);

													_assignConnectionArgsToRequestArgs(&pTokenSessionImpl->proxyConnectOpts, pRestRequestArgs);
												}
												else
												{
													if (_reactorWorkerRegisterEventForRestClient(pReactorWorker, pReactorImpl) != RSSL_RET_SUCCESS)
													{
														/* Clears sensitive information */
														rsslClearReactorOAuthCredentialRenewal(&pOAuthCredentialRenewalImpl->reactorOAuthCredentialRenewal);
														free(pRestRequestArgs);
														return (_reactorWorkerShutdown(pReactorImpl, &pReactorWorker->workerCerr), RSSL_THREAD_RETURN());
													}

													/* Specify proxy configurations when there is no token session */
													pRestRequestArgs->networkArgs.proxyArgs.proxyDomain = pOAuthCredentialRenewalImpl->proxyDomain;
													pRestRequestArgs->networkArgs.proxyArgs.proxyHostName = pOAuthCredentialRenewalImpl->proxyHostName;
													pRestRequestArgs->networkArgs.proxyArgs.proxyPassword = pOAuthCredentialRenewalImpl->proxyPasswd;
													pRestRequestArgs->networkArgs.proxyArgs.proxyPort = pOAuthCredentialRenewalImpl->proxyPort;
													pRestRequestArgs->networkArgs.proxyArgs.proxyUserName = pOAuthCredentialRenewalImpl->proxyUserName;
												}

												if ( (pRsslRestHandle = rsslRestClientNonBlockingRequest(pReactorImpl->pRestClient, pRestRequestArgs,
													pTokenSessionImpl ? rsslRestAuthTokenResponseCallback : rsslRestAuthTokenResponseWithoutSessionCallback,
													pTokenSessionImpl ? rsslRestAuthTokenErrorCallback : rsslRestErrorWithoutSessionCallback,
													pTokenSessionImpl ? &pTokenSessionImpl->rsslAccessTokenRespBuffer : &pOAuthCredentialRenewalImpl->rsslAccessTokenRespBuffer,
													pTokenSessionImpl ? &pTokenSessionImpl->tokenSessionWorkerCerr.rsslError : &pReactorWorker->workerCerr.rsslError)) != 0)
												{
													if (pTokenSessionImpl)
													{
														rsslClearBuffer(&pTokenSessionImpl->temporaryURL);

														pTokenSessionImpl->pRestHandle = pRsslRestHandle;

														pTokenSessionImpl->originalExpiresIn = 0; /* Unset to indicate that the password grant is sent. */

														/* Checks whether the token session has the initial token information */
														if (pTokenSessionImpl->tokenInformation.accessToken.data == 0)
														{
															pTokenSessionImpl->tokenSessionState = RSSL_RC_TOKEN_SESSION_IMPL_REQ_INIT_AUTH_TOKEN;
															pTokenSessionImpl->tokenMgntEventType = RSSL_RCIMPL_TKET_REISSUE;
														}
														else
														{
															pTokenSessionImpl->tokenSessionState = RSSL_RC_TOKEN_SESSION_IMPL_REQ_AUTH_TOKEN_REISSUE;
															pTokenSessionImpl->tokenMgntEventType = RSSL_RCIMPL_TKET_REISSUE_NO_REFRESH;
														}
													}

													/* Clears sensitive information */
													rsslClearReactorOAuthCredentialRenewal(&pOAuthCredentialRenewalImpl->reactorOAuthCredentialRenewal);

													free(pRestRequestArgs);
												}
												else
												{
													free(pRestRequestArgs);

													/* Clears sensitive information */
													rsslClearReactorOAuthCredentialRenewal(&pOAuthCredentialRenewalImpl->reactorOAuthCredentialRenewal);

													/* Free the RsslReactorOAuthCredentialRenewalImpl if it does not belong to a token session */
													_reactorWorkerFreeRsslReactorOAuthCredentialRenewal(pOAuthCredentialRenewalImpl);

													if (pTokenSessionImpl)
													{
														rsslClearBuffer(&pTokenSessionImpl->temporaryURL);

														RSSL_MUTEX_UNLOCK(&pTokenSessionImpl->accessTokenMutex);

														RSSL_QUEUE_FOR_EACH_LINK(&pTokenSessionImpl->reactorChannelList, pLink)
														{
															pReactorChannel = RSSL_QUEUE_LINK_TO_OBJECT(RsslReactorChannelImpl, tokenSessionLink, pLink);

															if (!RSSL_ERROR_INFO_CHECK(_reactorWorkerHandleChannelFailure(pReactorImpl, pReactorChannel, &pTokenSessionImpl->tokenSessionWorkerCerr) == RSSL_RET_SUCCESS, RSSL_RET_FAILURE, &pReactorWorker->workerCerr))
															{
																return (_reactorWorkerShutdown(pReactorImpl, &pReactorWorker->workerCerr), RSSL_THREAD_RETURN());
															}
														}
													}
													else
													{
														RsslReactorTokenMgntEvent *pEvent = (RsslReactorTokenMgntEvent*)rsslReactorEventQueueGetFromPool(&pReactorImpl->reactorEventQueue);
														RsslReactorErrorInfoImpl *pReactorErrorInfoImpl = rsslReactorGetErrorInfoFromPool(pReactorWorker);
														
														rsslClearReactorTokenMgntEvent(pEvent);
														rsslClearReactorErrorInfoImpl(pReactorErrorInfoImpl);

														pEvent->pOAuthCredentialRenewal = (RsslReactorOAuthCredentialRenewal*)pOAuthCredentialRenewalImpl;
														pEvent->pAuthTokenEventCallback = pOAuthCredentialRenewalImpl->pAuthTokenEventCallback;
														pEvent->reactorTokenMgntEventType = RSSL_RCIMPL_TKET_RESP_FAILURE;

														pReactorErrorInfoImpl->referenceCount++;
														rsslSetErrorInfo(&pReactorErrorInfoImpl->rsslErrorInfo, RSSL_EIC_FAILURE, RSSL_RET_FAILURE, __FILE__, __LINE__,
															"Failed to send the REST request. Text: %s", pReactorWorker->workerCerr.rsslError.text);

														pEvent->pReactorErrorInfoImpl = pReactorErrorInfoImpl;

														if (!RSSL_ERROR_INFO_CHECK(rsslReactorEventQueuePut(&pReactorImpl->reactorEventQueue, (RsslReactorEventImpl*)pEvent)
															== RSSL_RET_SUCCESS, RSSL_RET_FAILURE, &pReactorWorker->workerCerr))
														{
															return (_reactorWorkerShutdown(pReactorImpl, &pReactorWorker->workerCerr), RSSL_THREAD_RETURN());
														}
													}
												}
											}
											else
											{
												/* Clears sensitive information */
												rsslClearReactorOAuthCredentialRenewal(&pOAuthCredentialRenewalImpl->reactorOAuthCredentialRenewal);

												if (pTokenSessionImpl)
												{
													RSSL_MUTEX_UNLOCK(&pTokenSessionImpl->accessTokenMutex);

													RSSL_QUEUE_FOR_EACH_LINK(&pTokenSessionImpl->reactorChannelList, pLink)
													{
														pReactorChannel = RSSL_QUEUE_LINK_TO_OBJECT(RsslReactorChannelImpl, tokenSessionLink, pLink);

														if (!RSSL_ERROR_INFO_CHECK(_reactorWorkerHandleChannelFailure(pReactorImpl, pReactorChannel, &pTokenSessionImpl->tokenSessionWorkerCerr) == RSSL_RET_SUCCESS, RSSL_RET_FAILURE, &pReactorWorker->workerCerr))
														{
															return (_reactorWorkerShutdown(pReactorImpl, &pReactorWorker->workerCerr), RSSL_THREAD_RETURN());
														}
													}
												}
												else
												{
													RsslReactorTokenMgntEvent *pEvent = (RsslReactorTokenMgntEvent*)rsslReactorEventQueueGetFromPool(&pReactorImpl->reactorEventQueue);
													RsslReactorErrorInfoImpl *pReactorErrorInfoImpl = rsslReactorGetErrorInfoFromPool(pReactorWorker);

													rsslClearReactorTokenMgntEvent(pEvent);
													rsslClearReactorErrorInfoImpl(pReactorErrorInfoImpl);

													pEvent->pOAuthCredentialRenewal = (RsslReactorOAuthCredentialRenewal*)pOAuthCredentialRenewalImpl;
													pEvent->pAuthTokenEventCallback = pOAuthCredentialRenewalImpl->pAuthTokenEventCallback;
													pEvent->reactorTokenMgntEventType = RSSL_RCIMPL_TKET_RESP_FAILURE;

													pReactorErrorInfoImpl->referenceCount++;
													rsslSetErrorInfo(&pReactorErrorInfoImpl->rsslErrorInfo, RSSL_EIC_FAILURE, RSSL_RET_FAILURE, __FILE__, __LINE__,
														"Failed to send the REST request. Text: %s", pReactorWorker->workerCerr.rsslError.text);

													pEvent->pReactorErrorInfoImpl = pReactorErrorInfoImpl;

													if (!RSSL_ERROR_INFO_CHECK(rsslReactorEventQueuePut(&pReactorImpl->reactorEventQueue, (RsslReactorEventImpl*)pEvent)
														== RSSL_RET_SUCCESS, RSSL_RET_FAILURE, &pReactorWorker->workerCerr))
													{
														return (_reactorWorkerShutdown(pReactorImpl, &pReactorWorker->workerCerr), RSSL_THREAD_RETURN());
													}
												}
											}

											break;
										}
										case RSSL_RCIMPL_CRET_MEMORY_DEALLOCATION:
										{
											_reactorWorkerFreeRsslReactorOAuthCredentialRenewal(pOAuthCredentialRenewalImpl);
											break;
										}
									}

									break;
								}
							case RSSL_RCIMPL_ET_TOKEN_SESSION_MGNT:
								{
									RsslReactorTokenSessionEvent *pTokenSessionEvent = (RsslReactorTokenSessionEvent*)pEvent;
									RsslReactorTokenSessionImpl *pTokenSessionImpl = pTokenSessionEvent->pTokenSessionImpl;
									RsslReactorChannelImpl *pReactorChannelImpl = (RsslReactorChannelImpl*)pTokenSessionEvent->pReactorChannel;

									switch (pTokenSessionEvent->reactorTokenSessionEventType)
									{
										case RSSL_RCIMPL_TSET_ADD_TOKEN_SESSION_TO_LIST:
										{
											if(pTokenSessionImpl->sessionLink.next == 0 || pTokenSessionImpl->sessionLink.prev == 0)
												rsslQueueAddLinkToBack(&pReactorWorker->reactorTokenManagement.tokenSessionList, &pTokenSessionImpl->sessionLink);
											break;
										}
										case RSSL_RCIMPL_TSET_ADD_TOKEN_SESSION_TO_LIST | RSSL_RCIMPL_TSET_REGISTER_CHANNEL_TO_SESSION:
										{
											if (pTokenSessionImpl->sessionLink.next == 0 || pTokenSessionImpl->sessionLink.prev == 0)
												rsslQueueAddLinkToBack(&pReactorWorker->reactorTokenManagement.tokenSessionList, &pTokenSessionImpl->sessionLink);

											if (pTokenSessionImpl->sendTokenRequest == 0)
											{
												RsslInt tokenExpiresTime = getCurrentTimeMs(pReactorImpl->ticksPerMsec) + (pTokenSessionImpl->tokenInformation.expiresIn * 1000);
												pTokenSessionImpl->nextExpiresTime = getCurrentTimeMs(pReactorImpl->ticksPerMsec) + (RsslInt)(((double)pTokenSessionImpl->tokenInformation.expiresIn  * pReactorImpl->tokenReissueRatio) * 1000);
												RTR_ATOMIC_SET64(pTokenSessionImpl->tokenExpiresTime, tokenExpiresTime);
												RTR_ATOMIC_SET(pTokenSessionImpl->sendTokenRequest, 1);
											}

											/* Fall through to register the channel to the token session */
										}
										case RSSL_RCIMPL_TSET_REGISTER_CHANNEL_TO_SESSION:
										{
											pReactorChannelImpl->pTokenSessionImpl = pTokenSessionImpl;
											rsslQueueAddLinkToBack(&pTokenSessionImpl->reactorChannelList, &pReactorChannelImpl->tokenSessionLink);
											RTR_ATOMIC_DECREMENT(pTokenSessionImpl->numberOfWaitingChannels);
											RTR_ATOMIC_SET(pReactorChannelImpl->addedToTokenSessionList, RSSL_TRUE);
											break;
										}
										case RSSL_RCIMPL_TSET_UNREGISTER_CHANNEL_FROM_SESSION:
										{
											RsslReactorTokenSessionEvent* pEvent;

											if (pReactorChannelImpl->pTokenSessionImpl)
											{
												pReactorChannelImpl->pTokenSessionImpl = NULL;
												
												if(pReactorChannelImpl->tokenSessionLink.next != NULL && pReactorChannelImpl->tokenSessionLink.prev != NULL)
													rsslQueueRemoveLink(&pTokenSessionImpl->reactorChannelList, &pReactorChannelImpl->tokenSessionLink);
											}

											if (pTokenSessionImpl->reactorChannelList.count == 0)
											{
												if(pTokenSessionImpl->sessionLink.next != NULL && pTokenSessionImpl->sessionLink.prev != NULL)
													rsslQueueRemoveLink(&pReactorImpl->reactorWorker.reactorTokenManagement.tokenSessionList, &pTokenSessionImpl->sessionLink);

												rsslFreeReactorTokenSessionImpl(pTokenSessionImpl);
											}

											pEvent = (RsslReactorTokenSessionEvent*)rsslReactorEventQueueGetFromPool(&pReactorImpl->reactorEventQueue);
											rsslClearReactorTokenSessionEvent(pEvent);

											pEvent->reactorTokenSessionEventType = RSSL_RCIMPL_TSET_RETURN_CHANNEL_TO_CHANNEL_POOL;
											pEvent->pReactorChannel = (RsslReactorChannel*)pReactorChannelImpl;
											pEvent->pTokenSessionImpl = NULL;

											if (!RSSL_ERROR_INFO_CHECK(rsslReactorEventQueuePut(&pReactorImpl->reactorEventQueue, (RsslReactorEventImpl*)pEvent) == RSSL_RET_SUCCESS, RSSL_RET_FAILURE, &pReactorWorker->workerCerr))
											{
												return (_reactorWorkerShutdown(pReactorImpl, &pReactorWorker->workerCerr), RSSL_THREAD_RETURN());
											}

											break;
										}
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

							/* Checks whether the users wants the Reactor to always a ping message for the JSON protocol by not updating the lastPingSentMs field */
							if (pReactorChannel->sendWSPingMessage == RSSL_FALSE)
							{
								pReactorChannel->lastPingSentMs = pReactorWorker->lastRecordedTimeMs;
							}

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
			RsslBool sendPingMessage = RSSL_TRUE;
			pReactorChannel = RSSL_QUEUE_LINK_TO_OBJECT(RsslReactorChannelImpl, workerLink, pLink);

			/* Checks whether to send a ping message for the JSON protocol. */
			if (pReactorChannel->reactorChannel.pRsslChannel->protocolType == RSSL_JSON_PROTOCOL_TYPE)
			{
				sendPingMessage = pReactorChannel->sendWSPingMessage; 
			}

			if (sendPingMessage)
			{
				/* Check if the elapsed time is greater than our ping-send interval. */
				if ((pReactorWorker->lastRecordedTimeMs - pReactorChannel->lastPingSentMs) > pReactorChannel->reactorChannel.pRsslChannel->pingTimeout * 1000 * pingIntervalFactor)
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
						if ((pReactorChannel->statisticFlags & RSSL_RC_ST_PING) && pReactorChannel->pChannelStatistic)
						{
							RsslReactorChannelPingEvent *pEvent = (RsslReactorChannelPingEvent*)rsslReactorEventQueueGetFromPool(&pReactorChannel->pParentReactor->reactorEventQueue);
							rsslClearReactorChannelPingEvent(pEvent);

							pEvent->pReactorChannel = (RsslReactorChannel*)pReactorChannel;

							if (!RSSL_ERROR_INFO_CHECK(rsslReactorEventQueuePut(&pReactorChannel->pParentReactor->reactorEventQueue, (RsslReactorEventImpl*)pEvent)
								== RSSL_RET_SUCCESS, RSSL_RET_FAILURE, &pReactorWorker->workerCerr))
							{
								return (_reactorWorkerShutdown(pReactorImpl, &pReactorWorker->workerCerr), RSSL_THREAD_RETURN());
							}
						}

						pReactorChannel->lastPingSentMs = pReactorWorker->lastRecordedTimeMs;
						_reactorWorkerCalculateNextTimeout(pReactorImpl, (RsslUInt32)(pReactorChannel->reactorChannel.pRsslChannel->pingTimeout * 1000 * pingIntervalFactor));
					}
				}
				else
				{
					/* Otherwise, figure out when to wake up again. (Ping interval - (current time - time of last ping). */
					_reactorWorkerCalculateNextTimeout(pReactorImpl, (RsslUInt32)(pReactorChannel->lastPingSentMs + (RsslInt64)(pReactorChannel->reactorChannel.pRsslChannel->pingTimeout * 1000 * pingIntervalFactor) - pReactorWorker->lastRecordedTimeMs));
				}
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
				RsslReactorConnectInfoImpl *pReactorConnectInfoImpl;
				RsslBool submitOriginalCredential = RSSL_FALSE;

				if ((pReactorChannel->connectionOptList[pReactorChannel->connectionListIter].reactorChannelInfoImplState == RSSL_RC_CHINFO_IMPL_ST_REQ_AUTH_TOKEN) ||
					(pReactorChannel->connectionOptList[pReactorChannel->connectionListIter].reactorChannelInfoImplState == RSSL_RC_CHINFO_IMPL_ST_QUERYING_SERVICE_DISOVERY))
				{
					continue; /* Still waiting for response from the token service or service discovery before switching to another RsslReactorConnectInfoImpl */
				}
				else if (pReactorChannel->connectionOptList[pReactorChannel->connectionListIter].reactorChannelInfoImplState == RSSL_RC_CHINFO_IMPL_ST_STOP_REQUESTING)
				{
					/* Ensure that the worker notifies the reactor only the first time */
					if (pReactorChannel->workerParentList == &pReactorWorker->reconnectingChannels)
					{
						/* Stop reconnecting the channel */
						pReactorChannel->reconnectAttemptLimit = 0;

						if (!RSSL_ERROR_INFO_CHECK(_reactorWorkerHandleChannelFailure(pReactorImpl, pReactorChannel, &pReactorChannel->channelWorkerCerr) == RSSL_RET_SUCCESS, RSSL_RET_FAILURE, &pReactorWorker->workerCerr))
							return (_reactorWorkerShutdown(pReactorImpl, &pReactorWorker->workerCerr), RSSL_THREAD_RETURN());
					}

					continue; /* Move to next channel */
				}

				/* Restore the states of RsslReactorChannelImpl before moving to the next RsslReactorConnectInfoImpl */
				if (pReactorChannel->connectionOptList[pReactorChannel->connectionListIter].base.enableSessionManagement)
				{
					/* Restore the original login request information */
					if (pReactorChannel->channelRole.ommConsumerRole.pLoginRequest)
					{
						RsslRDMLoginRequest *pRDMLoginRequest = pReactorChannel->channelRole.ommConsumerRole.pLoginRequest;
						if (pRDMLoginRequest->userName.data != pReactorChannel->userName.data)
						{
							pRDMLoginRequest->flags = pReactorChannel->flags;
							pRDMLoginRequest->userName = pReactorChannel->userName;
							pRDMLoginRequest->userNameType = pReactorChannel->userNameType;
							submitOriginalCredential = RSSL_TRUE;
						}
					}

					/* Removes the channel from the token session */
					if (pReactorChannel->pTokenSessionImpl)
					{
						if (pReactorChannel->tokenSessionLink.next != NULL && pReactorChannel->tokenSessionLink.prev != NULL)
						{
							rsslQueueRemoveLink(&pReactorChannel->pTokenSessionImpl->reactorChannelList, &pReactorChannel->tokenSessionLink);
							pReactorChannel->connectionOptList[pReactorChannel->connectionListIter].lastTokenUpdatedTime = 0;
						}
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
					RsslReactorTokenSessionImpl *pTokenSessionImpl = pReactorChannel->pTokenSessionImpl;
					RsslReactorTokenMgntEvent *pEvent = NULL;

					if (_reactorWorkerRegisterEventForRestClient(pReactorWorker, pReactorImpl) != RSSL_RET_SUCCESS)
					{
						return (_reactorWorkerShutdown(pReactorImpl, &pReactorWorker->workerCerr), RSSL_THREAD_RETURN());
					}

					/* Checks whether the access token is being requested by the user's thread */
					if (pTokenSessionImpl->requestingAccessToken == 1 ||
						pTokenSessionImpl->tokenSessionState == RSSL_RC_TOKEN_SESSION_IMPL_REQ_INIT_AUTH_TOKEN ||
						pTokenSessionImpl->tokenSessionState == RSSL_RC_TOKEN_SESSION_IMPL_REQ_AUTH_TOKEN_REISSUE)
						continue;

					RSSL_MUTEX_LOCK(&pTokenSessionImpl->accessTokenMutex);

					/* Checks whether the token session has the token information */
					if (pTokenSessionImpl->tokenInformation.accessToken.data == 0)
					{
						pTokenSessionImpl->tokenSessionState = RSSL_RC_TOKEN_SESSION_IMPL_REQUEST_FAILURE;

						/* Assign proxy information if any from the first connection for the token session */
						if (pTokenSessionImpl->copiedProxyOpts == RSSL_FALSE)
						{
							if (rsslDeepCopyConnectOpts(&pTokenSessionImpl->proxyConnectOpts, &pReactorConnectInfoImpl->base.rsslConnectOptions) != RSSL_RET_SUCCESS)
							{
								rsslSetErrorInfo(&pReactorChannel->channelWorkerCerr, RSSL_EIC_FAILURE, RSSL_RET_FAILURE, __FILE__, __LINE__,
									"Failed to copy proxy configuration parameters for the token session.");

								if (!RSSL_ERROR_INFO_CHECK(_reactorWorkerHandleChannelFailure(pReactorImpl, pReactorChannel, &pReactorChannel->channelWorkerCerr) == RSSL_RET_SUCCESS, RSSL_RET_FAILURE, &pReactorWorker->workerCerr))
								{
									RSSL_MUTEX_UNLOCK(&pTokenSessionImpl->accessTokenMutex);
									return (_reactorWorkerShutdown(pReactorImpl, &pReactorWorker->workerCerr), RSSL_THREAD_RETURN());
								}

								RSSL_MUTEX_UNLOCK(&pTokenSessionImpl->accessTokenMutex);
								continue;
							}

							pTokenSessionImpl->copiedProxyOpts = RSSL_TRUE;
						}

						if (pTokenSessionImpl->pOAuthCredential->pOAuthCredentialEventCallback == 0)
						{
							/* Handling error cases to get authentication token using the password */
							pRestRequestArgs = _reactorCreateRequestArgsForPassword(pReactorImpl, &pReactorImpl->tokenServiceURL,
								&pTokenSessionImpl->pOAuthCredential->userName,
								&pTokenSessionImpl->pOAuthCredential->password,
								NULL, /* For specifying a new password */
								&pTokenSessionImpl->pOAuthCredential->clientId,
								&pTokenSessionImpl->pOAuthCredential->clientSecret,
								&pTokenSessionImpl->pOAuthCredential->tokenScope,
								&pTokenSessionImpl->rsslPostDataBodyBuf, pTokenSessionImpl, &pReactorWorker->workerCerr);

							if (pRestRequestArgs)
							{
								_assignConnectionArgsToRequestArgs(&pTokenSessionImpl->proxyConnectOpts, pRestRequestArgs);

								if ((pTokenSessionImpl->pRestHandle = rsslRestClientNonBlockingRequest(pReactorImpl->pRestClient, pRestRequestArgs,
									rsslRestAuthTokenResponseCallback, rsslRestAuthTokenErrorCallback,
									&pTokenSessionImpl->rsslAccessTokenRespBuffer, &pTokenSessionImpl->tokenSessionWorkerCerr.rsslError)) != 0)
								{
									pTokenSessionImpl->originalExpiresIn = 0; /* Unset to indicate that the password grant is sent. */
									pReactorConnectInfoImpl->reactorChannelInfoImplState = RSSL_RC_CHINFO_IMPL_ST_REQ_AUTH_TOKEN;
									pTokenSessionImpl->tokenSessionState = RSSL_RC_TOKEN_SESSION_IMPL_REQ_INIT_AUTH_TOKEN;
									pTokenSessionImpl->tokenMgntEventType = RSSL_RCIMPL_TKET_REISSUE;
									free(pRestRequestArgs);

									/* Add the channel to the token session */
									rsslQueueAddLinkToBack(&pTokenSessionImpl->reactorChannelList, &pReactorChannel->tokenSessionLink);
									RTR_ATOMIC_SET(pReactorChannel->addedToTokenSessionList, RSSL_TRUE); /* Ensure that this flag is set when the channel is added from recovering the connection */

									/* Unlock the access mutex whenever receives the response */
								}
								else
								{
									RSSL_MUTEX_UNLOCK(&pTokenSessionImpl->accessTokenMutex);

									free(pRestRequestArgs);

									if (!RSSL_ERROR_INFO_CHECK(_reactorWorkerHandleChannelFailure(pReactorImpl, pReactorChannel, &pReactorChannel->channelWorkerCerr) == RSSL_RET_SUCCESS, RSSL_RET_FAILURE, &pReactorWorker->workerCerr))
									{
										return (_reactorWorkerShutdown(pReactorImpl, &pReactorWorker->workerCerr), RSSL_THREAD_RETURN());
									}
								}
							}
							else
							{
								RSSL_MUTEX_UNLOCK(&pTokenSessionImpl->accessTokenMutex);
								if (!RSSL_ERROR_INFO_CHECK(_reactorWorkerHandleChannelFailure(pReactorImpl, pReactorChannel, &pReactorChannel->channelWorkerCerr) == RSSL_RET_SUCCESS, RSSL_RET_FAILURE, &pReactorWorker->workerCerr))
								{
									return (_reactorWorkerShutdown(pReactorImpl, &pReactorWorker->workerCerr), RSSL_THREAD_RETURN());
								}
							}
						}
						else
						{
							RSSL_MUTEX_UNLOCK(&pTokenSessionImpl->accessTokenMutex);

							/* Notifies the application via the callback method to pass in sensitive information */
							pReactorConnectInfoImpl->reactorChannelInfoImplState = RSSL_RC_CHINFO_IMPL_ST_REQ_AUTH_TOKEN;
							pTokenSessionImpl->tokenSessionState = RSSL_RC_TOKEN_SESSION_IMPL_WAITING_TO_REQ_AUTH_TOKEN;
							
							/* Add the channel to the token session */
							rsslQueueAddLinkToBack(&pTokenSessionImpl->reactorChannelList, &pReactorChannel->tokenSessionLink);
							RTR_ATOMIC_SET(pReactorChannel->addedToTokenSessionList, RSSL_TRUE); /* Ensure that this flag is set when the channel is added from recovering the connection */

							notifySensitiveInfoReq(pTokenSessionImpl, pReactorWorker);
						}

						continue;
					}
					else
					{
						RSSL_MUTEX_UNLOCK(&pTokenSessionImpl->accessTokenMutex);

						/* Checks to ensure that the access token is still valid before establishing the connection */
						if (pTokenSessionImpl->lastTokenUpdatedTime == 0)
						{
							continue;
						}

						/* Add the channel to the token session */
						rsslQueueAddLinkToBack(&pTokenSessionImpl->reactorChannelList, &pReactorChannel->tokenSessionLink);
						RTR_ATOMIC_SET(pReactorChannel->addedToTokenSessionList, RSSL_TRUE); /* Ensure that this flag is set when the channel is added from recovering the connection */
					}

					if ((!pReactorConnectInfoImpl->base.rsslConnectOptions.connectionInfo.unified.address || !(*pReactorConnectInfoImpl->base.rsslConnectOptions.connectionInfo.unified.address)) &&
						(!pReactorConnectInfoImpl->base.rsslConnectOptions.connectionInfo.unified.serviceName || !(*pReactorConnectInfoImpl->base.rsslConnectOptions.connectionInfo.unified.serviceName)))
					{	/* Get host name and port for EDP-RT service discovery */
						RsslBuffer rsslBuffer = RSSL_INIT_BUFFER;
						RsslQueueLink *pLink = NULL;
						RsslError rsslError;
						RsslReactorDiscoveryTransportProtocol transport = RSSL_RD_TP_INIT;
						RsslRestRequestArgs* pRestRequestArgs;

						switch (pReactorConnectInfoImpl->base.rsslConnectOptions.connectionType)
						{
						case RSSL_CONN_TYPE_ENCRYPTED:
						{
							if (pReactorConnectInfoImpl->base.rsslConnectOptions.encryptionOpts.encryptedProtocol == RSSL_CONN_TYPE_SOCKET)
							{
								transport = RSSL_RD_TP_TCP;
							}
							else if (pReactorConnectInfoImpl->base.rsslConnectOptions.encryptionOpts.encryptedProtocol == RSSL_CONN_TYPE_WEBSOCKET)
							{
								transport = RSSL_RD_TP_WEBSOCKET;
							}
							else
							{
								rsslSetErrorInfo(&pReactorChannel->channelWorkerCerr, RSSL_EIC_FAILURE, RSSL_RET_INVALID_ARGUMENT, __FILE__, __LINE__,
									"Invalid encrypted protocol type(%d) for requesting EDP-RT service discovery.", pReactorConnectInfoImpl->base.rsslConnectOptions.encryptionOpts.encryptedProtocol);

								pReactorConnectInfoImpl->reactorChannelInfoImplState = RSSL_RC_CHINFO_IMPL_ST_INVALID_CONNECTION_TYPE;

								/* Notify error back to the application via the channel event */
								if (!RSSL_ERROR_INFO_CHECK(_reactorWorkerHandleChannelFailure(pReactorChannel->pParentReactor, pReactorChannel, &pReactorChannel->channelWorkerCerr) == RSSL_RET_SUCCESS, RSSL_RET_FAILURE, &pReactorWorker->workerCerr))
								{
									return (_reactorWorkerShutdown(pReactorImpl, &pReactorWorker->workerCerr), RSSL_THREAD_RETURN());
								}

								pReactorConnectInfoImpl->reactorChannelInfoImplState = RSSL_RC_CHINFO_IMPL_ST_REQUEST_FAILURE;
								continue;
							}

							break;
						}
						default:

							rsslSetErrorInfo(&pReactorChannel->channelWorkerCerr, RSSL_EIC_FAILURE, RSSL_RET_INVALID_ARGUMENT, __FILE__, __LINE__,
								"Invalid connection type(%d) for requesting EDP-RT service discovery.",
								transport);

							pReactorConnectInfoImpl->reactorChannelInfoImplState = RSSL_RC_CHINFO_IMPL_ST_INVALID_CONNECTION_TYPE;

							/* Notify error back to the application via the channel event */
							if (!RSSL_ERROR_INFO_CHECK(_reactorWorkerHandleChannelFailure(pReactorChannel->pParentReactor, pReactorChannel, &pReactorChannel->channelWorkerCerr) == RSSL_RET_SUCCESS, RSSL_RET_FAILURE, &pReactorWorker->workerCerr))
							{
								return (_reactorWorkerShutdown(pReactorImpl, &pReactorWorker->workerCerr), RSSL_THREAD_RETURN());
							}

							pReactorConnectInfoImpl->reactorChannelInfoImplState = RSSL_RC_CHINFO_IMPL_ST_REQUEST_FAILURE;
							continue;
						}

						/* Keeps the transport protocol value for subsequence request for HTTP error handling */
						pReactorConnectInfoImpl->transportProtocol = transport;

						rsslClearBuffer(&rsslBuffer);
						if ((pRestRequestArgs = _reactorCreateRequestArgsForServiceDiscovery(pReactorImpl, &pReactorChannel->pParentReactor->serviceDiscoveryURL,
							pReactorConnectInfoImpl->transportProtocol, RSSL_RD_DP_INIT, &pTokenSessionImpl->tokenInformation.tokenType,
							&pTokenSessionImpl->tokenInformation.accessToken,
							&rsslBuffer, pReactorChannel, &pReactorChannel->channelWorkerCerr)) == 0)
						{
							/* Notify error back to the application via the channel event */
							if (!RSSL_ERROR_INFO_CHECK(_reactorWorkerHandleChannelFailure(pReactorChannel->pParentReactor, pReactorChannel, &pReactorChannel->channelWorkerCerr) == RSSL_RET_SUCCESS, RSSL_RET_FAILURE, &pReactorWorker->workerCerr))
							{
								free(rsslBuffer.data);
								return (_reactorWorkerShutdown(pReactorImpl, &pReactorWorker->workerCerr), RSSL_THREAD_RETURN());
							}

							pReactorConnectInfoImpl->reactorChannelInfoImplState = RSSL_RC_CHINFO_IMPL_ST_MEM_ALLOCATION_FAILURE;
							free(rsslBuffer.data);
							continue;
						}

						if (pTokenSessionImpl->rsslServiceDiscoveryRespBuffer.data == 0)
						{
							pTokenSessionImpl->rsslServiceDiscoveryRespBuffer.length = RSSL_REST_INIT_SVC_DIS_BUF_SIZE;
							pTokenSessionImpl->rsslServiceDiscoveryRespBuffer.data = (char*)malloc(pTokenSessionImpl->rsslServiceDiscoveryRespBuffer.length);

							if (pTokenSessionImpl->rsslServiceDiscoveryRespBuffer.data == 0)
							{
								rsslSetErrorInfo(&pReactorChannel->channelWorkerCerr, RSSL_EIC_FAILURE, RSSL_RET_FAILURE, __FILE__, __LINE__,
									"Failed to allocate memory for service discovery response buffer.");

								/* Notify error back to the application via the channel event */
								if (!RSSL_ERROR_INFO_CHECK(_reactorWorkerHandleChannelFailure(pReactorChannel->pParentReactor, pReactorChannel, &pReactorChannel->channelWorkerCerr) == RSSL_RET_SUCCESS, RSSL_RET_FAILURE, &pReactorWorker->workerCerr))
								{
									free(pRestRequestArgs);
									free(rsslBuffer.data);
									return (_reactorWorkerShutdown(pReactorImpl, &pReactorWorker->workerCerr), RSSL_THREAD_RETURN());
								}

								pReactorConnectInfoImpl->reactorChannelInfoImplState = RSSL_RC_CHINFO_IMPL_ST_MEM_ALLOCATION_FAILURE;
								free(pRestRequestArgs);
								free(rsslBuffer.data);
								continue;
							}
						}

						pReactorConnectInfoImpl->reactorChannelInfoImplState = RSSL_RC_CHINFO_IMPL_ST_QUERYING_SERVICE_DISOVERY;

						_assignConnectionArgsToRequestArgs(&pReactorConnectInfoImpl->base.rsslConnectOptions, pRestRequestArgs);

						if ((pReactorChannel->pRestHandle = rsslRestClientNonBlockingRequest(pReactorChannel->pParentReactor->pRestClient, pRestRequestArgs,
							rsslRestServiceDiscoveryResponseCallback,
							rsslRestErrorCallback,
							&pTokenSessionImpl->rsslServiceDiscoveryRespBuffer, &rsslError)) == 0)
						{
							pReactorConnectInfoImpl->reactorChannelInfoImplState = RSSL_RC_CHINFO_IMPL_ST_REQUEST_FAILURE;
						}

						free(pRestRequestArgs);
						free(rsslBuffer.data);

						if (pReactorConnectInfoImpl->reactorChannelInfoImplState == RSSL_RC_CHINFO_IMPL_ST_REQUEST_FAILURE)
						{
							rsslSetErrorInfo(&pReactorChannel->channelWorkerCerr, RSSL_EIC_FAILURE, RSSL_RET_FAILURE, __FILE__, __LINE__,
								"Failed to send the REST request. Text: %s", rsslError.text);

							/* Notify error back to the application via the channel event */
							if (!RSSL_ERROR_INFO_CHECK(_reactorWorkerHandleChannelFailure(pReactorChannel->pParentReactor, pReactorChannel, &pReactorChannel->channelWorkerCerr) == RSSL_RET_SUCCESS, RSSL_RET_FAILURE, &pReactorWorker->workerCerr))
							{
								return (_reactorWorkerShutdown(pReactorImpl, &pReactorWorker->workerCerr), RSSL_THREAD_RETURN());
							}
						}
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
									return (_reactorWorkerShutdown(pReactorImpl, &pReactorWorker->workerCerr), RSSL_THREAD_RETURN());
								}
							}
							else
							{
								RsslRet ret = RSSL_RET_SUCCESS;

								/* Reset channel statistics as the new connection is established */
								if (pReactorChannel->pChannelStatistic)
								{
									rsslClearReactorChannelStatistic(pReactorChannel->pChannelStatistic);
								}

								/* Set debug callback usage here. */
								if (pReactorChannel->connectionDebugFlags != 0 && rsslIoctl(pReactorChannel->reactorChannel.pRsslChannel, RSSL_DEBUG_FLAGS, (void*)&(pReactorChannel->connectionDebugFlags), &pReactorChannel->channelWorkerCerr.rsslError) != RSSL_RET_SUCCESS)
								{
									if (!RSSL_ERROR_INFO_CHECK(_reactorWorkerHandleChannelFailure(pReactorImpl, pReactorChannel, &pReactorChannel->channelWorkerCerr) == RSSL_RET_SUCCESS, RSSL_RET_FAILURE, &pReactorWorker->workerCerr))
										return (_reactorWorkerShutdown(pReactorImpl, &pReactorWorker->workerCerr), RSSL_THREAD_RETURN());
									continue;
								}

								pReactorChannel->channelSetupState = RSSL_RC_CHST_INIT;
								pReactorChannel->initializationTimeout = pReactorConnectInfoImpl->base.initializationTimeout;
								if (!RSSL_ERROR_INFO_CHECK(_reactorWorkerProcessNewChannel(pReactorChannel->pParentReactor, pReactorChannel) == RSSL_RET_SUCCESS, ret, &pReactorWorker->workerCerr))
								{
									return (_reactorWorkerShutdown(pReactorImpl, &pReactorWorker->workerCerr), RSSL_THREAD_RETURN());
								}
							}
						}
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

					if (!(pReactorChannel->reactorChannel.pRsslChannel = rsslConnect(&(pReactorConnectInfoImpl->base.rsslConnectOptions), &pReactorChannel->channelWorkerCerr.rsslError)))
					{
						if (!RSSL_ERROR_INFO_CHECK(_reactorWorkerHandleChannelFailure(pReactorImpl, pReactorChannel, &pReactorChannel->channelWorkerCerr) == RSSL_RET_SUCCESS, RSSL_RET_FAILURE, &pReactorWorker->workerCerr))
							return (_reactorWorkerShutdown(pReactorImpl, &pReactorWorker->workerCerr), RSSL_THREAD_RETURN());
					}
					else
					{
						/* Reset channel statistics as the new connection is established */
						if (pReactorChannel->pChannelStatistic)
						{
							rsslClearReactorChannelStatistic(pReactorChannel->pChannelStatistic);
						}

						/* Set debug callback usage here. */
						if (pReactorChannel->connectionDebugFlags != 0 && rsslIoctl(pReactorChannel->reactorChannel.pRsslChannel, RSSL_DEBUG_FLAGS, (void*)&(pReactorChannel->connectionDebugFlags), &pReactorChannel->channelWorkerCerr.rsslError) != RSSL_RET_SUCCESS)
						{
							if (!RSSL_ERROR_INFO_CHECK(_reactorWorkerHandleChannelFailure(pReactorImpl, pReactorChannel, &pReactorChannel->channelWorkerCerr) == RSSL_RET_SUCCESS, RSSL_RET_FAILURE, &pReactorWorker->workerCerr))
								return (_reactorWorkerShutdown(pReactorImpl, &pReactorWorker->workerCerr), RSSL_THREAD_RETURN());
							continue;
						}

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

		/* Ensure that there is token sessions in the token management */
		if (pReactorWorker->reactorTokenManagement.tokenSessionList.count != 0)
		{
			RSSL_QUEUE_FOR_EACH_LINK(&pReactorWorker->reactorTokenManagement.tokenSessionList, pLink)
			{
				RsslReactorTokenSessionImpl *pTokenSession = RSSL_QUEUE_LINK_TO_OBJECT(RsslReactorTokenSessionImpl, sessionLink, pLink);

				/* Checks whether the token is still valid */
				if  ( (pTokenSession->tokenExpiresTime != 0 ) && ((pReactorWorker->lastRecordedTimeMs - pTokenSession->tokenExpiresTime) > 0) )
				{
					RTR_ATOMIC_SET64(pTokenSession->tokenExpiresTime, 0);
					pTokenSession->lastTokenUpdatedTime = 0;
				}

				if (pTokenSession->sendTokenRequest && ((pReactorWorker->lastRecordedTimeMs - pTokenSession->nextExpiresTime) > 0))
				{
					RsslRestRequestArgs *pRestRequestArgs;

					RTR_ATOMIC_SET(pTokenSession->sendTokenRequest, 0);

					if (_reactorWorkerRegisterEventForRestClient(pReactorWorker, pReactorImpl) != RSSL_RET_SUCCESS)
					{
						return (_reactorWorkerShutdown(pReactorImpl, &pReactorWorker->workerCerr), RSSL_THREAD_RETURN());
					}

					pTokenSession->tokenSessionState = RSSL_RC_TOKEN_SESSION_IMPL_REQUEST_FAILURE;

					if (pTokenSession->resendFromFailure)
					{
						// Reset the indication flag
						pTokenSession->resendFromFailure = 0;

						/* Checks whether the access token is being request by the user's thread */
						if (pTokenSession->requestingAccessToken == 1)
						{
							continue;
						}

						RSSL_MUTEX_LOCK(&pTokenSession->accessTokenMutex);

						if (pTokenSession->pOAuthCredential->pOAuthCredentialEventCallback == 0)
						{
							/* Handling error cases to get authentication token using the password */
							pRestRequestArgs = _reactorCreateRequestArgsForPassword(pReactorImpl, &pReactorImpl->tokenServiceURL,
								&pTokenSession->pOAuthCredential->userName,
								&pTokenSession->pOAuthCredential->password,
								NULL, /* For specifying a new password */
								&pTokenSession->pOAuthCredential->clientId,
								&pTokenSession->pOAuthCredential->clientSecret,
								&pTokenSession->pOAuthCredential->tokenScope,
								&pTokenSession->rsslPostDataBodyBuf, pTokenSession, &pReactorWorker->workerCerr);

							if (pRestRequestArgs)
							{
								_assignConnectionArgsToRequestArgs(&pTokenSession->proxyConnectOpts, pRestRequestArgs);

								if ( (pTokenSession->pRestHandle = rsslRestClientNonBlockingRequest(pReactorImpl->pRestClient, pRestRequestArgs,
									rsslRestAuthTokenResponseCallback, rsslRestAuthTokenErrorCallback,
									&pTokenSession->rsslAccessTokenRespBuffer, &pTokenSession->tokenSessionWorkerCerr.rsslError)) != 0)
								{
									pTokenSession->originalExpiresIn = 0; /* Unset to indicate that the password grant is sent. */
									pTokenSession->tokenSessionState = RSSL_RC_TOKEN_SESSION_IMPL_REQ_AUTH_TOKEN_REISSUE;
									pTokenSession->tokenMgntEventType = RSSL_RCIMPL_TKET_REISSUE_NO_REFRESH;

									free(pRestRequestArgs);
								}
								else
								{
									RSSL_MUTEX_UNLOCK(&pTokenSession->accessTokenMutex);

									free(pRestRequestArgs);

									RSSL_QUEUE_FOR_EACH_LINK(&pTokenSession->reactorChannelList, pLink)
									{
										pReactorChannel = RSSL_QUEUE_LINK_TO_OBJECT(RsslReactorChannelImpl, tokenSessionLink, pLink);

										if (!RSSL_ERROR_INFO_CHECK(_reactorWorkerHandleChannelFailure(pReactorImpl, pReactorChannel, &pReactorChannel->channelWorkerCerr) == RSSL_RET_SUCCESS, RSSL_RET_FAILURE, &pReactorWorker->workerCerr))
										{
											return (_reactorWorkerShutdown(pReactorImpl, &pReactorWorker->workerCerr), RSSL_THREAD_RETURN());
										}
									}
								}
							}
							else
							{
								RSSL_MUTEX_UNLOCK(&pTokenSession->accessTokenMutex);

								RSSL_QUEUE_FOR_EACH_LINK(&pTokenSession->reactorChannelList, pLink)
								{
									pReactorChannel = RSSL_QUEUE_LINK_TO_OBJECT(RsslReactorChannelImpl, tokenSessionLink, pLink);

									if (!RSSL_ERROR_INFO_CHECK(_reactorWorkerHandleChannelFailure(pReactorImpl, pReactorChannel, &pReactorChannel->channelWorkerCerr) == RSSL_RET_SUCCESS, RSSL_RET_FAILURE, &pReactorWorker->workerCerr))
									{
										return (_reactorWorkerShutdown(pReactorImpl, &pReactorWorker->workerCerr), RSSL_THREAD_RETURN());
									}
								}
							}
						}
						else
						{
							RSSL_MUTEX_UNLOCK(&pTokenSession->accessTokenMutex);

							/* Notifies the application via the callback method to pass in sensitive information */
							pTokenSession->tokenSessionState = RSSL_RC_TOKEN_SESSION_IMPL_WAITING_TO_REQ_AUTH_TOKEN;
							notifySensitiveInfoReq(pTokenSession, pReactorWorker);
						}
					}
					else
					{
						pRestRequestArgs = _reactorWorkerCreateRequestArgsForRefreshToken(pTokenSession, pReactorImpl, &pTokenSession->tokenSessionWorkerCerr);

						/* Checks whether the access token is being request by the user's thread */
						if (pTokenSession->requestingAccessToken == 1)
						{
							continue;
						}

						RSSL_MUTEX_LOCK(&pTokenSession->accessTokenMutex);

						if (pRestRequestArgs)
						{
							_assignConnectionArgsToRequestArgs(&pTokenSession->proxyConnectOpts, pRestRequestArgs);

							if ( (pTokenSession->pRestHandle = rsslRestClientNonBlockingRequest(pReactorImpl->pRestClient, pRestRequestArgs,
								rsslRestAuthTokenResponseCallback,
								rsslRestAuthTokenErrorCallback,
								&pTokenSession->rsslAccessTokenRespBuffer, &pTokenSession->tokenSessionWorkerCerr.rsslError)) != 0)
							{
								pTokenSession->tokenSessionState = RSSL_RC_TOKEN_SESSION_IMPL_REQ_AUTH_TOKEN_REISSUE;
								pTokenSession->tokenMgntEventType = RSSL_RCIMPL_TKET_REISSUE_NO_REFRESH;

								free(pRestRequestArgs);
							}
							else
							{
								RSSL_MUTEX_UNLOCK(&pTokenSession->accessTokenMutex);

								free(pRestRequestArgs);

								RSSL_QUEUE_FOR_EACH_LINK(&pTokenSession->reactorChannelList, pLink)
								{
									pReactorChannel = RSSL_QUEUE_LINK_TO_OBJECT(RsslReactorChannelImpl, tokenSessionLink, pLink);
									if (!RSSL_ERROR_INFO_CHECK(_reactorWorkerHandleChannelFailure(pReactorImpl, pReactorChannel, &pReactorChannel->channelWorkerCerr) == RSSL_RET_SUCCESS, RSSL_RET_FAILURE, &pReactorWorker->workerCerr))
									{
										return (_reactorWorkerShutdown(pReactorImpl, &pReactorWorker->workerCerr), RSSL_THREAD_RETURN());
									}
								}
							}
						}
						else
						{
							RSSL_MUTEX_UNLOCK(&pTokenSession->accessTokenMutex);

							RSSL_QUEUE_FOR_EACH_LINK(&pTokenSession->reactorChannelList, pLink)
							{
								pReactorChannel = RSSL_QUEUE_LINK_TO_OBJECT(RsslReactorChannelImpl, tokenSessionLink, pLink);
								if (!RSSL_ERROR_INFO_CHECK(_reactorWorkerHandleChannelFailure(pReactorImpl, pReactorChannel, &pReactorChannel->channelWorkerCerr) == RSSL_RET_SUCCESS, RSSL_RET_FAILURE, &pReactorWorker->workerCerr))
								{
									free(pRestRequestArgs);
									return (_reactorWorkerShutdown(pReactorImpl, &pReactorWorker->workerCerr), RSSL_THREAD_RETURN());
								}
							}
						}
					}
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

		pEvent = (RsslReactorStateEvent*)rsslReactorEventQueueGet(&pReactorImpl->reactorWorker.workerQueue, pReactorImpl->maxEventsInPool, &ret);
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

static void _reactorWorkerFreeRsslReactorOAuthCredentialRenewal(RsslReactorOAuthCredentialRenewalImpl *pReactorOAuthCredentialRenewalImpl)
{
	if (pReactorOAuthCredentialRenewalImpl->pParentTokenSession == 0)
	{
		free(pReactorOAuthCredentialRenewalImpl->rsslAccessTokenRespBuffer.data);
		free(pReactorOAuthCredentialRenewalImpl->rsslPostDataBodyBuf.data);
		free(pReactorOAuthCredentialRenewalImpl->tokenInformationBuffer.data);
		free(pReactorOAuthCredentialRenewalImpl);
	}
}

static RsslRestRequestArgs* _reactorWorkerCreateRequestArgsForRefreshToken(RsslReactorTokenSessionImpl *pReactorTokenSession, RsslReactorImpl *pRsslReactor,
	RsslErrorInfo* pError)
{
	RsslRestHeader  *pRsslRestAcceptHeader;
	RsslRestHeader  *pRsslRestContentTypeHeader;
	RsslUInt32 neededSize = 0;
	RsslRestRequestArgs *pRequestArgs = 0;
	RsslBuffer *pRsslUserNameEncodedUrl = 0;
	RsslBool	deallocateUserNameEncodedBuffer = RSSL_FALSE;
	RsslReactorOAuthCredential *pReactorOAuthCredential = pReactorTokenSession->pOAuthCredential;
	RsslBuffer* pUserName = &pReactorOAuthCredential->userName;
	RsslBool	hasClientSecret = RSSL_FALSE;
	char* pCurPos = 0;
	RsslBuffer clientId = RSSL_INIT_BUFFER;
	RsslReactorImpl *pReactorImpl = (RsslReactorImpl*)pReactorTokenSession->pReactor;

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

	if (pRequestArgs == 0) goto memoryAllocationFailed;

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
		pReactorTokenSession->tokenInformation.refreshToken.length + 1;

	if (pReactorOAuthCredential->clientSecret.length && pReactorOAuthCredential->clientSecret.data)
	{
		hasClientSecret = RSSL_TRUE;
		neededSize += rssl_rest_client_secret_text.length + pReactorOAuthCredential->clientSecret.length;
	}

	neededSize += (RsslUInt32)(sizeof(RsslRestHeader) * 2);

	if (pReactorTokenSession->rsslPostDataBodyBuf.length < neededSize)
	{
		free(pReactorTokenSession->rsslPostDataBodyBuf.data);
		pReactorTokenSession->rsslPostDataBodyBuf.length = neededSize;
		pReactorTokenSession->rsslPostDataBodyBuf.data = (char*)malloc(neededSize);

		if (pReactorTokenSession->rsslPostDataBodyBuf.data == 0)
		{
			rsslClearBuffer(&pReactorTokenSession->rsslPostDataBodyBuf);
			goto memoryAllocationFailed;
		}
	}

	memset(pReactorTokenSession->rsslPostDataBodyBuf.data, 0, pReactorTokenSession->rsslPostDataBodyBuf.length);

	pRequestArgs->httpBody.data = pReactorTokenSession->rsslPostDataBodyBuf.data;

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
	strncat(pRequestArgs->httpBody.data, pReactorTokenSession->tokenInformation.refreshToken.data, pReactorTokenSession->tokenInformation.refreshToken.length);
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
	pRequestArgs->url.data = pRsslReactor->tokenServiceURL.data;
	pRequestArgs->url.length = pRsslReactor->tokenServiceURL.length;

	if (deallocateUserNameEncodedBuffer)
	{
		free(pRsslUserNameEncodedUrl->data);
		free(pRsslUserNameEncodedUrl);
	}

	pRequestArgs->pUserSpecPtr = pReactorTokenSession;

	/* Assigned the request timeout in seconds */
	pRequestArgs->requestTimeOut = pReactorImpl->restRequestTimeout;

	return pRequestArgs;

memoryAllocationFailed:

	if (deallocateUserNameEncodedBuffer)
	{
		free(pRsslUserNameEncodedUrl->data);
		free(pRsslUserNameEncodedUrl);
	}

	if (pRequestArgs) free(pRequestArgs);

	rsslSetErrorInfo(pError, RSSL_EIC_FAILURE, RSSL_RET_FAILURE, __FILE__, __LINE__,
		"Failed to allocate memory for creating request arguments for the token service using the refresh token.");

	return 0;
}

static void notifySensitiveInfoReq(RsslReactorTokenSessionImpl* pTokenSession, RsslReactorWorker* pReactorWorker)
{
	RsslReactorImpl *pReactorImpl = (RsslReactorImpl*)pTokenSession->pReactor;
	RsslReactorCredentialRenewalEvent *pEvent = (RsslReactorCredentialRenewalEvent*)rsslReactorEventQueueGetFromPool(&pReactorImpl->reactorEventQueue);
	rsslClearReactorCredentialRenewalEvent(pEvent);

	pEvent->pOAuthCredentialEventCallback = pTokenSession->pOAuthCredential->pOAuthCredentialEventCallback;
	pEvent->reactorCredentialRenewalEventType = RSSL_RCIMPL_CRET_RENEWAL_CALLBACK;
	pEvent->pTokenSessionImpl = pTokenSession;

	if (!RSSL_ERROR_INFO_CHECK(rsslReactorEventQueuePut(&pReactorImpl->reactorEventQueue, (RsslReactorEventImpl*)pEvent)
		== RSSL_RET_SUCCESS, RSSL_RET_FAILURE, &pReactorWorker->workerCerr))
	{
		_reactorWorkerShutdown(pReactorImpl, &pReactorWorker->workerCerr);
		return;
	}
}

static void handlingAuthRequestFailure(RsslReactorTokenSessionImpl* pReactorTokenSession, RsslReactorWorker* pReactorWorker)
{
	RsslReactorTokenMgntEvent *pEvent;
	RsslReactorChannelImpl *pReactorChannel;
	RsslReactorConnectInfoImpl *pReactorConnectInfoImpl;
	RsslQueueLink *pLink;
	RsslReactorImpl *pReactorImpl = (RsslReactorImpl *)pReactorTokenSession->pReactor;
	RsslReactorErrorInfoImpl *pReactorErrorInfoImpl = NULL;
	RsslBool notifyChannelWarning = RSSL_FALSE;

	RSSL_QUEUE_FOR_EACH_LINK(&pReactorTokenSession->reactorChannelList, pLink)
	{
		pReactorChannel = RSSL_QUEUE_LINK_TO_OBJECT(RsslReactorChannelImpl, tokenSessionLink, pLink);
		pReactorConnectInfoImpl = &pReactorChannel->connectionOptList[pReactorChannel->connectionListIter];

		/* Gets RsslReactorErrorInfoImpl for a channel from the pool. */
		pReactorErrorInfoImpl = rsslReactorGetErrorInfoFromPool(pReactorWorker);
		rsslClearReactorErrorInfoImpl(pReactorErrorInfoImpl);
		rsslCopyErrorInfo(&pReactorErrorInfoImpl->rsslErrorInfo, &pReactorTokenSession->tokenSessionWorkerCerr);

		notifyChannelWarning = (pReactorChannel->reactorChannel.pRsslChannel && (pReactorChannel->reactorChannel.pRsslChannel->state == RSSL_CH_STATE_ACTIVE));

		/* Increase the reference count for notifying the channel warning  */
		if (notifyChannelWarning)
			pReactorErrorInfoImpl->referenceCount++;

		// Notifies users with pAuthTokenEventCallback if exists otherwise the RSSL_RC_CET_WARNING event when the channel is active
		if (pReactorConnectInfoImpl->base.pAuthTokenEventCallback)
		{
			pEvent = (RsslReactorTokenMgntEvent*)rsslReactorEventQueueGetFromPool(&pReactorChannel->pParentReactor->reactorEventQueue);
			rsslClearReactorTokenMgntEvent(pEvent);

			pEvent->pTokenSessionImpl = pReactorTokenSession;
			pEvent->reactorTokenMgntEventType = RSSL_RCIMPL_TKET_RESP_FAILURE;
			pEvent->pReactorChannel = &pReactorChannel->reactorChannel;
			pEvent->pAuthTokenEventCallback = pReactorConnectInfoImpl->base.pAuthTokenEventCallback;
			pReactorErrorInfoImpl->referenceCount++;
			pEvent->pReactorErrorInfoImpl = pReactorErrorInfoImpl;

			if (!RSSL_ERROR_INFO_CHECK(rsslReactorEventQueuePut(&pReactorChannel->pParentReactor->reactorEventQueue, (RsslReactorEventImpl*)pEvent)
				== RSSL_RET_SUCCESS, RSSL_RET_FAILURE, &pReactorWorker->workerCerr))
			{
				_reactorWorkerShutdown(pReactorChannel->pParentReactor, &pReactorWorker->workerCerr);
				return;
			}
		}

		if (notifyChannelWarning)
		{
			pEvent = (RsslReactorTokenMgntEvent*)rsslReactorEventQueueGetFromPool(&pReactorChannel->pParentReactor->reactorEventQueue);
			rsslClearReactorTokenMgntEvent(pEvent);

			pEvent->pTokenSessionImpl = pReactorTokenSession;
			pEvent->reactorTokenMgntEventType = RSSL_RCIMPL_TKET_CHANNEL_WARNING;
			pEvent->pReactorChannel = &pReactorChannel->reactorChannel;
			pEvent->pReactorErrorInfoImpl = pReactorErrorInfoImpl;

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
			if (!RSSL_ERROR_INFO_CHECK(_reactorWorkerHandleChannelFailure(pReactorChannel->pParentReactor, pReactorChannel, &pReactorTokenSession->tokenSessionWorkerCerr) == RSSL_RET_SUCCESS, RSSL_RET_FAILURE, &pReactorWorker->workerCerr))
			{
				_reactorWorkerShutdown(pReactorChannel->pParentReactor, &pReactorWorker->workerCerr);
				return;
			}
		}

		if (pReactorTokenSession->tokenSessionState == RSSL_RC_TOKEN_SESSION_IMPL_STOP_REQUESTING)
		{
			pReactorConnectInfoImpl->reactorChannelInfoImplState = RSSL_RC_CHINFO_IMPL_ST_STOP_REQUESTING;
		}
		else
		{
			pReactorConnectInfoImpl->reactorChannelInfoImplState = RSSL_RC_CHINFO_IMPL_ST_REQUEST_FAILURE;
		}
	}

	if (pReactorTokenSession->tokenSessionState != RSSL_RC_TOKEN_SESSION_IMPL_STOP_REQUESTING)
	{
		/* Setting this flag to resend the authentication request with password */
		if (pReactorTokenSession->tokenMgntEventType == RSSL_RCIMPL_TKET_REISSUE_NO_REFRESH ||
			pReactorTokenSession->tokenMgntEventType == RSSL_RCIMPL_TKET_REISSUE)
		{
			if (pReactorTokenSession->reissueTokenAttemptLimit == -1) /* there is no limit. */
			{
				pReactorTokenSession->resendFromFailure = 1;
				RTR_ATOMIC_SET(pReactorTokenSession->sendTokenRequest, 1);
				pReactorTokenSession->nextExpiresTime = getCurrentTimeMs(pReactorImpl->ticksPerMsec) + pReactorImpl->reissueTokenAttemptInterval;
			}
			else if (pReactorTokenSession->reissueTokenAttemptLimit > 0)
			{
				--pReactorTokenSession->reissueTokenAttemptLimit;
				pReactorTokenSession->resendFromFailure = 1;
				RTR_ATOMIC_SET(pReactorTokenSession->sendTokenRequest, 1);
				pReactorTokenSession->nextExpiresTime = getCurrentTimeMs(pReactorImpl->ticksPerMsec) + pReactorImpl->reissueTokenAttemptInterval;
			}
			else
			{
				/* Indicates that the Reactor will not attempt to send token reissue request. */
				RTR_ATOMIC_SET(pReactorTokenSession->stopTokenRequest, 1);
				pReactorTokenSession->tokenSessionState = RSSL_RC_TOKEN_SESSION_IMPL_STOP_REQUESTING;
			}
		}
	}
	else
	{
		/* Indicates that the Reactor will not attempt to send token reissue request. */
		RTR_ATOMIC_SET(pReactorTokenSession->stopTokenRequest, 1);
	}
}

static void rsslRestServiceDiscoveryResponseCallback(RsslRestResponse* restresponse, RsslRestResponseEvent* event)
{
	RsslReactorChannelImpl *pReactorChannel = (RsslReactorChannelImpl*)event->closure;
	RsslError rsslError;
	RsslReactorConnectInfoImpl *pReactorConnectInfoImpl = &pReactorChannel->connectionOptList[pReactorChannel->connectionListIter];
	RsslReactorWorker *pReactorWorker = &pReactorChannel->pParentReactor->reactorWorker;
	RsslReactorImpl *pReactorImpl = pReactorChannel->pParentReactor;
	RsslReactorTokenSessionImpl *pTokenSessionImpl = pReactorChannel->pTokenSessionImpl;

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
	pReactorChannel->pRestHandle = NULL;

	switch (restresponse->statusCode)
	{
		case 200: /* OK */
		{
			if (pReactorConnectInfoImpl->reactorChannelInfoImplState == RSSL_RC_CHINFO_IMPL_ST_QUERYING_SERVICE_DISOVERY)
			{
				RsslBuffer parseBuffer;
				RsslBuffer hostName;
				RsslBuffer port;

				parseBuffer.length = RSSL_REST_STORE_HOST_AND_PORT_BUF_SIZE;
				parseBuffer.data = (char*)malloc(parseBuffer.length);

				if (parseBuffer.data == 0)
				{
					pReactorConnectInfoImpl->reactorChannelInfoImplState = RSSL_RC_CHINFO_IMPL_ST_MEM_ALLOCATION_FAILURE;
					return;
				}

				if (rsslRestParseEndpoint(&restresponse->dataBody, &pReactorConnectInfoImpl->base.location, &hostName, &port, &parseBuffer, &rsslError) != RSSL_RET_SUCCESS)
				{
					free(parseBuffer.data);
					pReactorConnectInfoImpl->reactorChannelInfoImplState = RSSL_RC_CHINFO_IMPL_ST_PARSE_RESP_FAILURE;

					rsslSetErrorInfo(&pReactorChannel->channelWorkerCerr, RSSL_EIC_FAILURE, RSSL_RET_FAILURE, __FILE__, __LINE__,
						"Failed to query host name and port from the EDP-RT service discovery for the \"%s\" location", pReactorConnectInfoImpl->base.location.data);

					/* Notify error back to the application via the channel event */
					goto RequestFailed;
				}

				// Free the original copy if any such as empty string
				if (pReactorConnectInfoImpl->base.rsslConnectOptions.connectionInfo.unified.address)
					free(pReactorConnectInfoImpl->base.rsslConnectOptions.connectionInfo.unified.address);

				pReactorConnectInfoImpl->base.rsslConnectOptions.connectionInfo.unified.address = (char*)malloc(hostName.length + 1);

				if (pReactorConnectInfoImpl->base.rsslConnectOptions.connectionInfo.unified.address == 0)
				{
					free(parseBuffer.data);
					pReactorConnectInfoImpl->reactorChannelInfoImplState = RSSL_RC_CHINFO_IMPL_ST_MEM_ALLOCATION_FAILURE;
					return;
				}

				strncpy(pReactorConnectInfoImpl->base.rsslConnectOptions.connectionInfo.unified.address, hostName.data, hostName.length + 1);

				// Free the original copy if any such as empty string
				if (pReactorConnectInfoImpl->base.rsslConnectOptions.connectionInfo.unified.serviceName)
					free(pReactorConnectInfoImpl->base.rsslConnectOptions.connectionInfo.unified.serviceName);

				pReactorConnectInfoImpl->base.rsslConnectOptions.connectionInfo.unified.serviceName = (char*)malloc(port.length + 1);

				if (pReactorConnectInfoImpl->base.rsslConnectOptions.connectionInfo.unified.serviceName == 0)
				{
					free(parseBuffer.data);
					free(pReactorConnectInfoImpl->base.rsslConnectOptions.connectionInfo.unified.address);
					pReactorConnectInfoImpl->base.rsslConnectOptions.connectionInfo.unified.address = 0;
					pReactorConnectInfoImpl->reactorChannelInfoImplState = RSSL_RC_CHINFO_IMPL_ST_MEM_ALLOCATION_FAILURE;
					return;
				}

				strncpy(pReactorConnectInfoImpl->base.rsslConnectOptions.connectionInfo.unified.serviceName, port.data, port.length + 1);
				free(parseBuffer.data);

				pReactorConnectInfoImpl->reactorChannelInfoImplState = RSSL_RC_CHINFO_IMPL_ST_ASSIGNED_HOST_PORT;

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

					/* Reset channel statistics as the new connection is established */
					if (pReactorChannel->pChannelStatistic)
					{
						rsslClearReactorChannelStatistic(pReactorChannel->pChannelStatistic);
					}

					/* Set debug callback usage here. */
					if (pReactorChannel->connectionDebugFlags != 0 && rsslIoctl(pReactorChannel->reactorChannel.pRsslChannel, RSSL_DEBUG_FLAGS, (void*)&(pReactorChannel->connectionDebugFlags), &pReactorChannel->channelWorkerCerr.rsslError) != RSSL_RET_SUCCESS)
					{
						if (!RSSL_ERROR_INFO_CHECK(_reactorWorkerHandleChannelFailure(pReactorImpl, pReactorChannel, &pReactorChannel->channelWorkerCerr) == RSSL_RET_SUCCESS, RSSL_RET_FAILURE, &pReactorWorker->workerCerr))
						{
							_reactorWorkerShutdown(pReactorChannel->pParentReactor, &pReactorWorker->workerCerr);
							return;
						}
						return;
					}

					pReactorChannel->channelSetupState = RSSL_RC_CHST_INIT;
					pReactorChannel->initializationTimeout = pReactorConnectInfoImpl->base.initializationTimeout;
					if (!RSSL_ERROR_INFO_CHECK(_reactorWorkerProcessNewChannel(pReactorChannel->pParentReactor, pReactorChannel) == RSSL_RET_SUCCESS, ret, &pReactorWorker->workerCerr))
					{
						_reactorWorkerShutdown(pReactorChannel->pParentReactor, &pReactorWorker->workerCerr);
					}
				}
			}

			break;
		}
		case 301: /* Moved Permanently */
		case 308: /* Permanent Redirect */
		{
			RsslBuffer* uri = getHeaderValue(&restresponse->headers, &rssl_rest_location_header_text);

			if (uri != NULL)
			{
				RsslBuffer rsslBuffer;
				RsslRestRequestArgs *pRestRequestArgs;

				/* Ensure that there is no thread accessing the service discovery URL while updating it */
				RSSL_MUTEX_LOCK(&pReactorWorker->reactorTokenManagement.tokenSessionMutex); 

				/* Copy the new URL to the rsslReactorImpl */
				if (uri->length > pReactorImpl->serviceDiscoveryURLBuffer.length)
				{
					free(pReactorImpl->serviceDiscoveryURLBuffer.data);
					pReactorImpl->serviceDiscoveryURLBuffer.length = uri->length + 1;
					pReactorImpl->serviceDiscoveryURLBuffer.data = (char*)malloc(pReactorImpl->serviceDiscoveryURLBuffer.length);
					if (pReactorImpl->serviceDiscoveryURLBuffer.data == 0)
					{
						RSSL_MUTEX_UNLOCK(&pReactorWorker->reactorTokenManagement.tokenSessionMutex);
						goto RequestFailed;
					}
				}

				memset(pReactorImpl->serviceDiscoveryURLBuffer.data, 0, pReactorImpl->serviceDiscoveryURLBuffer.length);
				pReactorImpl->serviceDiscoveryURL.data = pReactorImpl->serviceDiscoveryURLBuffer.data;
				pReactorImpl->serviceDiscoveryURL.length = uri->length - 1;
				memcpy(pReactorImpl->serviceDiscoveryURL.data, uri->data + 1, pReactorImpl->serviceDiscoveryURL.length);

				RSSL_MUTEX_UNLOCK(&pReactorWorker->reactorTokenManagement.tokenSessionMutex);

				rsslClearBuffer(&rsslBuffer);
				if ((pRestRequestArgs = _reactorCreateRequestArgsForServiceDiscovery(pReactorChannel->pParentReactor, &pReactorChannel->pParentReactor->serviceDiscoveryURL,
					pReactorConnectInfoImpl->transportProtocol, RSSL_RD_DP_INIT, &pTokenSessionImpl->tokenInformation.tokenType,
					&pTokenSessionImpl->tokenInformation.accessToken,
					&rsslBuffer, pReactorChannel, &pReactorChannel->channelWorkerCerr)) == 0)
				{
					goto RequestFailed;
				}

				_assignConnectionArgsToRequestArgs(&pReactorConnectInfoImpl->base.rsslConnectOptions, pRestRequestArgs);

				pReactorConnectInfoImpl->reactorChannelInfoImplState = RSSL_RC_CHINFO_IMPL_ST_QUERYING_SERVICE_DISOVERY;

				if ((pReactorChannel->pRestHandle = rsslRestClientNonBlockingRequest(pReactorChannel->pParentReactor->pRestClient, pRestRequestArgs,
					rsslRestServiceDiscoveryResponseCallback,
					rsslRestErrorCallback,
					&pTokenSessionImpl->rsslServiceDiscoveryRespBuffer, &rsslError)) == 0)
				{
					pReactorConnectInfoImpl->reactorChannelInfoImplState = RSSL_RC_CHINFO_IMPL_ST_REQUEST_FAILURE;
				}

				free(pRestRequestArgs);
				free(rsslBuffer.data);

				if (pReactorConnectInfoImpl->reactorChannelInfoImplState == RSSL_RC_CHINFO_IMPL_ST_REQUEST_FAILURE)
				{
					goto RequestFailed;
				}
			}
			else
			{
				goto RequestFailed;
			}

			break;
		}
		case 302: /* Found (Previously "Moved temporarily") */
		case 303: /* See Other(Another URI using the GET method) */
		case 307: /* Temporary Redirect (Request method is not allowed to be changed) */
		{
			RsslBuffer* tempUri = getHeaderValue(&restresponse->headers, &rssl_rest_location_header_text);

			if (tempUri != NULL)
			{
				RsslBuffer rsslBuffer;
				RsslRestRequestArgs *pRestRequestArgs;

				if ( pReactorChannel->temporaryURLBufLength < (tempUri->length - 1) )
				{
					if (pReactorChannel->temporaryURL.data)
					{
						free(pReactorChannel->temporaryURL.data);
					}

					pReactorChannel->temporaryURLBufLength = tempUri->length - 1;
					pReactorChannel->temporaryURL.data = (char*)malloc(pReactorChannel->temporaryURLBufLength);
					if (pReactorChannel->temporaryURL.data == 0)
					{
						pReactorChannel->temporaryURLBufLength = 0;
						rsslClearBuffer(&pReactorChannel->temporaryURL);
						goto RequestFailed;
					}

					memset(pReactorChannel->temporaryURL.data, 0, pReactorChannel->temporaryURLBufLength);
					pReactorChannel->temporaryURL.length = pReactorChannel->temporaryURLBufLength;
				}
				else
				{
					memset(pReactorChannel->temporaryURL.data, 0, pReactorChannel->temporaryURLBufLength);
					pReactorChannel->temporaryURL.length = tempUri->length - 1;
				}

				memcpy(pReactorChannel->temporaryURL.data, tempUri->data + 1, pReactorChannel->temporaryURL.length);

				rsslClearBuffer(&rsslBuffer);
				if ((pRestRequestArgs = _reactorCreateRequestArgsForServiceDiscovery(pReactorChannel->pParentReactor, &pReactorChannel->temporaryURL,
					pReactorConnectInfoImpl->transportProtocol, RSSL_RD_DP_INIT, &pTokenSessionImpl->tokenInformation.tokenType,
					&pTokenSessionImpl->tokenInformation.accessToken,
					&rsslBuffer, pReactorChannel, &pReactorChannel->channelWorkerCerr)) == 0)
				{
					goto RequestFailed;
				}

				_assignConnectionArgsToRequestArgs(&pReactorConnectInfoImpl->base.rsslConnectOptions, pRestRequestArgs);

				pReactorConnectInfoImpl->reactorChannelInfoImplState = RSSL_RC_CHINFO_IMPL_ST_QUERYING_SERVICE_DISOVERY;

				if ((pReactorChannel->pRestHandle = rsslRestClientNonBlockingRequest(pReactorChannel->pParentReactor->pRestClient, pRestRequestArgs,
					rsslRestServiceDiscoveryResponseCallback,
					rsslRestErrorCallback,
					&pTokenSessionImpl->rsslServiceDiscoveryRespBuffer, &rsslError)) == 0)
				{
					pReactorConnectInfoImpl->reactorChannelInfoImplState = RSSL_RC_CHINFO_IMPL_ST_REQUEST_FAILURE;
				}

				free(pRestRequestArgs);
				free(rsslBuffer.data);

				if (pReactorConnectInfoImpl->reactorChannelInfoImplState == RSSL_RC_CHINFO_IMPL_ST_REQUEST_FAILURE)
				{
					goto RequestFailed;
				}
			}
			else
			{
				goto RequestFailed;
			}

			break;
		}
		case 400: /* Bad Request */
		case 401: /* Unauthorized */
		{
			RSSL_MUTEX_LOCK(&pTokenSessionImpl->accessTokenMutex);

			/* Clears the exiting access token before requesting it */
			rsslClearBuffer(&pTokenSessionImpl->tokenInformation.accessToken);

			RSSL_MUTEX_UNLOCK(&pTokenSessionImpl->accessTokenMutex);

			goto RequestFailed;
		}
		case 403: /* Forbidden */
		case 451: /* Unavailable For Legal Reasons */
		{
			/* Stop retrying with the request */
			pReactorConnectInfoImpl->reactorChannelInfoImplState = RSSL_RC_CHINFO_IMPL_ST_STOP_REQUESTING;

			rsslSetErrorInfo(&pTokenSessionImpl->tokenSessionWorkerCerr, RSSL_EIC_FAILURE, RSSL_RET_FAILURE, __FILE__, __LINE__,
				"Received HTTP error %u status code with data body : %s.", restresponse->statusCode, restresponse->dataBody.data);

			if (!RSSL_ERROR_INFO_CHECK(_reactorWorkerHandleChannelFailure(pReactorChannel->pParentReactor, pReactorChannel, &pReactorChannel->channelWorkerCerr) == RSSL_RET_SUCCESS, RSSL_RET_FAILURE, &pReactorWorker->workerCerr))
			{
				_reactorWorkerShutdown(pReactorChannel->pParentReactor, &pReactorWorker->workerCerr);
			}

			break;
		}
		default:
		{
			rsslSetErrorInfo(&pReactorChannel->channelWorkerCerr, RSSL_EIC_FAILURE, RSSL_RET_FAILURE, __FILE__, __LINE__,
				"Received HTTP error %u status code with data body : %s.", restresponse->statusCode, restresponse->dataBody.data);

			if (pReactorConnectInfoImpl->reactorChannelInfoImplState == RSSL_RC_CHINFO_IMPL_ST_QUERYING_SERVICE_DISOVERY)
			{
				goto RequestFailed;
			}
		}
	}

	return;

RequestFailed:

	/* Notify error back to the application via the channel event */
	if (!RSSL_ERROR_INFO_CHECK(_reactorWorkerHandleChannelFailure(pReactorChannel->pParentReactor, pReactorChannel, &pReactorChannel->channelWorkerCerr) == RSSL_RET_SUCCESS, RSSL_RET_FAILURE, &pReactorWorker->workerCerr))
	{
		_reactorWorkerShutdown(pReactorChannel->pParentReactor, &pReactorWorker->workerCerr);
	}

	pReactorConnectInfoImpl->reactorChannelInfoImplState = RSSL_RC_CHINFO_IMPL_ST_REQUEST_FAILURE;
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

	/* Cleaning up the RsslRestHandle later by the ReactorWorker */
	rsslQueueAddLinkToBack(&pReactorWorker->disposableRestHandles, &event->handle->queueLink);
	pReactorChannel->pRestHandle = NULL;

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
}

static void rsslRestAuthTokenResponseCallback(RsslRestResponse* restresponse, RsslRestResponseEvent* event)
{
	RsslReactorTokenSessionImpl *pReactorTokenSession = (RsslReactorTokenSessionImpl*)event->closure;
	RsslError rsslError;
	RsslReactorImpl *pReactorImpl = (RsslReactorImpl*)pReactorTokenSession->pReactor;
	RsslReactorWorker *pReactorWorker = &pReactorImpl->reactorWorker;
	RsslReactorChannelImpl *pReactorChannel = NULL;
	RsslReactorConnectInfoImpl *pReactorConnectInfoImpl = NULL;
	RsslQueueLink *pLink;

	/* Releases the old memory and points the buffer to the new location. */
	if (restresponse->isMemReallocated)
	{
		free(event->userMemory->data);
		(*event->userMemory) = restresponse->reallocatedMem;
	}

	/* Set the HTTP response status code for the channel */
	pReactorTokenSession->httpStausCode = restresponse->statusCode;

	/* Cleaning up the RsslRestHandle later by the ReactorWorker */
	rsslQueueAddLinkToBack(&pReactorWorker->disposableRestHandles, &event->handle->queueLink);
	pReactorTokenSession->pRestHandle = NULL;

	switch (restresponse->statusCode)
	{
		case 200: /* OK */
		{
			RsslReactorTokenMgntEvent *pEvent = NULL;

			/* Reset the token reissue attempt limit to the user defined value */
			pReactorTokenSession->reissueTokenAttemptLimit = pReactorImpl->reissueTokenAttemptLimit;

			if (pReactorTokenSession->tokenInformationBuffer.length < restresponse->dataBody.length)
			{
				if (pReactorTokenSession->tokenInformationBuffer.data)
				{
					free(pReactorTokenSession->tokenInformationBuffer.data);
				}

				pReactorTokenSession->tokenInformationBuffer.length = restresponse->dataBody.length;
				pReactorTokenSession->tokenInformationBuffer.data = (char*)malloc(pReactorTokenSession->tokenInformationBuffer.length);

				if (pReactorTokenSession->tokenInformationBuffer.data == 0)
				{
					rsslClearBuffer(&pReactorTokenSession->tokenInformationBuffer);
					pReactorTokenSession->tokenSessionState = RSSL_RC_TOKEN_SESSION_IMPL_MEM_ALLOCATION_FAILURE;
					RSSL_MUTEX_UNLOCK(&pReactorTokenSession->accessTokenMutex);
					return;
				}
			}

			RSSL_MUTEX_UNLOCK(&pReactorTokenSession->accessTokenMutex);

			if (rsslRestParseAccessToken(&restresponse->dataBody, &pReactorTokenSession->tokenInformation.accessToken, &pReactorTokenSession->tokenInformation.refreshToken,
				&pReactorTokenSession->tokenInformation.expiresIn, &pReactorTokenSession->tokenInformation.tokenType,
				&pReactorTokenSession->tokenInformation.scope, &pReactorTokenSession->tokenInformationBuffer, &rsslError) != RSSL_RET_SUCCESS)
			{
				pReactorTokenSession->tokenSessionState = RSSL_RC_TOKEN_SESSION_IMPL_PARSE_RESP_FAILURE;
				goto RequestFailed;
			}

			if (pReactorTokenSession->originalExpiresIn != 0)
			{
				if (pReactorTokenSession->originalExpiresIn != pReactorTokenSession->tokenInformation.expiresIn)
				{
					goto ReAuthorization; /* Perform another authorization using the password grant type as the refresh token is about to expire. */
				}
			}
			else
			{
				/* Set the original expires in seconds for the password grant. */
				pReactorTokenSession->originalExpiresIn = pReactorTokenSession->tokenInformation.expiresIn;
			}

			pReactorTokenSession->lastTokenUpdatedTime = getCurrentTimeMs(pReactorImpl->ticksPerMsec);

			if (pReactorTokenSession->tokenSessionState == RSSL_RC_TOKEN_SESSION_IMPL_REQ_INIT_AUTH_TOKEN)
			{
				/* Adds the token session to the session list and starts the timer for the token reissue */
				if(pReactorTokenSession->sessionLink.next == 0 && pReactorTokenSession->sessionLink.prev == 0)
					rsslQueueAddLinkToBack(&pReactorWorker->reactorTokenManagement.tokenSessionList, &pReactorTokenSession->sessionLink);

				if (pReactorTokenSession->sendTokenRequest == 0)
				{
					RsslInt tokenExpiresTime = getCurrentTimeMs(pReactorImpl->ticksPerMsec) + (pReactorTokenSession->tokenInformation.expiresIn * 1000);
					pReactorTokenSession->nextExpiresTime = getCurrentTimeMs(pReactorImpl->ticksPerMsec) + (RsslInt)(((double)pReactorTokenSession->tokenInformation.expiresIn  * pReactorImpl->tokenReissueRatio) * 1000);
					RTR_ATOMIC_SET64(pReactorTokenSession->tokenExpiresTime, tokenExpiresTime);
					RTR_ATOMIC_SET(pReactorTokenSession->sendTokenRequest, 1);
				}
			}

			pReactorTokenSession->tokenSessionState = RSSL_RC_TOKEN_SESSION_IMPL_RECEIVED_AUTH_TOKEN;

			// Set the callback to the RsslReactorChannel for the current RsslReactorConnectInfo
			RSSL_QUEUE_FOR_EACH_LINK(&pReactorTokenSession->reactorChannelList, pLink)
			{
				pReactorChannel = RSSL_QUEUE_LINK_TO_OBJECT(RsslReactorChannelImpl, tokenSessionLink, pLink);

				pReactorConnectInfoImpl = &pReactorChannel->connectionOptList[pReactorChannel->connectionListIter];
				pReactorConnectInfoImpl->lastTokenUpdatedTime = pReactorTokenSession->lastTokenUpdatedTime;

				pEvent = (RsslReactorTokenMgntEvent*)rsslReactorEventQueueGetFromPool(&pReactorImpl->reactorEventQueue);
				rsslClearReactorTokenMgntEvent(pEvent);

				pEvent->reactorTokenMgntEventType = pReactorTokenSession->tokenMgntEventType;
				pEvent->pTokenSessionImpl = pReactorTokenSession;
				pEvent->pReactorChannel = &pReactorChannel->reactorChannel;
				pEvent->pAuthTokenEventCallback = pReactorConnectInfoImpl->base.pAuthTokenEventCallback;

				if (!RSSL_ERROR_INFO_CHECK(rsslReactorEventQueuePut(&pReactorImpl->reactorEventQueue, (RsslReactorEventImpl*)pEvent)
					== RSSL_RET_SUCCESS, RSSL_RET_FAILURE, &pReactorWorker->workerCerr))
				{
					_reactorWorkerShutdown(pReactorImpl, &pReactorWorker->workerCerr);
					return;
				}

				if (pReactorChannel->workerParentList == &pReactorWorker->reconnectingChannels)
				{
					if ((!pReactorConnectInfoImpl->base.rsslConnectOptions.connectionInfo.unified.address || !(*pReactorConnectInfoImpl->base.rsslConnectOptions.connectionInfo.unified.address)) &&
						(!pReactorConnectInfoImpl->base.rsslConnectOptions.connectionInfo.unified.serviceName || !(*pReactorConnectInfoImpl->base.rsslConnectOptions.connectionInfo.unified.serviceName)))
					{	/* Get host name and port for EDP-RT service discovery */
						RsslBuffer rsslBuffer = RSSL_INIT_BUFFER;
						RsslQueueLink *pLink = NULL;
						RsslError rsslError;
						RsslReactorDiscoveryTransportProtocol transport = RSSL_RD_TP_INIT;
						RsslRestRequestArgs* pRestRequestArgs;

						switch (pReactorConnectInfoImpl->base.rsslConnectOptions.connectionType)
						{
						case RSSL_CONN_TYPE_ENCRYPTED:
						{
							if (pReactorConnectInfoImpl->base.rsslConnectOptions.encryptionOpts.encryptedProtocol == RSSL_CONN_TYPE_SOCKET)
							{
								transport = RSSL_RD_TP_TCP;
							}
							else if (pReactorConnectInfoImpl->base.rsslConnectOptions.encryptionOpts.encryptedProtocol == RSSL_CONN_TYPE_WEBSOCKET)
							{
								transport = RSSL_RD_TP_WEBSOCKET;
							}
							else
							{
								rsslSetErrorInfo(&pReactorChannel->channelWorkerCerr, RSSL_EIC_FAILURE, RSSL_RET_INVALID_ARGUMENT, __FILE__, __LINE__,
									"Invalid encrypted protocol type(%d) for requesting EDP-RT service discovery.", pReactorConnectInfoImpl->base.rsslConnectOptions.encryptionOpts.encryptedProtocol);

								pReactorConnectInfoImpl->reactorChannelInfoImplState = RSSL_RC_CHINFO_IMPL_ST_INVALID_CONNECTION_TYPE;

								/* Notify error back to the application via the channel event */
								if (!RSSL_ERROR_INFO_CHECK(_reactorWorkerHandleChannelFailure(pReactorChannel->pParentReactor, pReactorChannel, &pReactorChannel->channelWorkerCerr) == RSSL_RET_SUCCESS, RSSL_RET_FAILURE, &pReactorWorker->workerCerr))
								{
									_reactorWorkerShutdown(pReactorImpl, &pReactorWorker->workerCerr);
									return;
								}

								pReactorConnectInfoImpl->reactorChannelInfoImplState = RSSL_RC_CHINFO_IMPL_ST_REQUEST_FAILURE;
								continue;
							}

							break;
						}
						default:
							rsslSetErrorInfo(&pReactorChannel->channelWorkerCerr, RSSL_EIC_FAILURE, RSSL_RET_INVALID_ARGUMENT, __FILE__, __LINE__,
								"Invalid connection type(%d) for requesting EDP-RT service discovery.",
								transport);

							pReactorConnectInfoImpl->reactorChannelInfoImplState = RSSL_RC_CHINFO_IMPL_ST_INVALID_CONNECTION_TYPE;

							/* Notify error back to the application via the channel event */
							if (!RSSL_ERROR_INFO_CHECK(_reactorWorkerHandleChannelFailure(pReactorChannel->pParentReactor, pReactorChannel, &pReactorChannel->channelWorkerCerr) == RSSL_RET_SUCCESS, RSSL_RET_FAILURE, &pReactorWorker->workerCerr))
							{
								_reactorWorkerShutdown(pReactorImpl, &pReactorWorker->workerCerr);
								return;
							}

							pReactorConnectInfoImpl->reactorChannelInfoImplState = RSSL_RC_CHINFO_IMPL_ST_REQUEST_FAILURE;
							continue;
						}

						/* Keeps the transport protocol value for subsequence request for HTTP error handling */
						pReactorConnectInfoImpl->transportProtocol = transport;

						rsslClearBuffer(&rsslBuffer);
						if ((pRestRequestArgs = _reactorCreateRequestArgsForServiceDiscovery(pReactorChannel->pParentReactor, &pReactorChannel->pParentReactor->serviceDiscoveryURL,
							pReactorConnectInfoImpl->transportProtocol, RSSL_RD_DP_INIT, &pReactorTokenSession->tokenInformation.tokenType,
							&pReactorTokenSession->tokenInformation.accessToken,
							&rsslBuffer, pReactorChannel, &pReactorChannel->channelWorkerCerr)) == 0)
						{
							/* Notify error back to the application via the channel event */
							if (!RSSL_ERROR_INFO_CHECK(_reactorWorkerHandleChannelFailure(pReactorChannel->pParentReactor, pReactorChannel, &pReactorChannel->channelWorkerCerr) == RSSL_RET_SUCCESS, RSSL_RET_FAILURE, &pReactorWorker->workerCerr))
							{
								_reactorWorkerShutdown(pReactorImpl, &pReactorWorker->workerCerr);
								free(rsslBuffer.data);
								return;
							}

							pReactorConnectInfoImpl->reactorChannelInfoImplState = RSSL_RC_CHINFO_IMPL_ST_MEM_ALLOCATION_FAILURE;
							free(rsslBuffer.data);
							continue;
						}

						if (pReactorTokenSession->rsslServiceDiscoveryRespBuffer.data == 0)
						{
							pReactorTokenSession->rsslServiceDiscoveryRespBuffer.length = RSSL_REST_INIT_SVC_DIS_BUF_SIZE;
							pReactorTokenSession->rsslServiceDiscoveryRespBuffer.data = (char*)malloc(pReactorTokenSession->rsslServiceDiscoveryRespBuffer.length);

							if (pReactorTokenSession->rsslServiceDiscoveryRespBuffer.data == 0)
							{
								rsslSetErrorInfo(&pReactorChannel->channelWorkerCerr, RSSL_EIC_FAILURE, RSSL_RET_FAILURE, __FILE__, __LINE__,
									"Failed to allocate memory for service discovery response buffer.");

								/* Notify error back to the application via the channel event */
								if (!RSSL_ERROR_INFO_CHECK(_reactorWorkerHandleChannelFailure(pReactorChannel->pParentReactor, pReactorChannel, &pReactorChannel->channelWorkerCerr) == RSSL_RET_SUCCESS, RSSL_RET_FAILURE, &pReactorWorker->workerCerr))
								{
									_reactorWorkerShutdown(pReactorImpl, &pReactorWorker->workerCerr);
									free(pRestRequestArgs);
									free(rsslBuffer.data);
									return;
								}

								pReactorConnectInfoImpl->reactorChannelInfoImplState = RSSL_RC_CHINFO_IMPL_ST_MEM_ALLOCATION_FAILURE;
								free(pRestRequestArgs);
								free(rsslBuffer.data);
								continue;
							}
						}

						_assignConnectionArgsToRequestArgs(&pReactorConnectInfoImpl->base.rsslConnectOptions, pRestRequestArgs);

						if ((pReactorChannel->pRestHandle = rsslRestClientNonBlockingRequest(pReactorChannel->pParentReactor->pRestClient, pRestRequestArgs,
							rsslRestServiceDiscoveryResponseCallback,
							rsslRestErrorCallback,
							&pReactorTokenSession->rsslServiceDiscoveryRespBuffer, &rsslError)) == 0)
						{
							pReactorConnectInfoImpl->reactorChannelInfoImplState = RSSL_RC_CHINFO_IMPL_ST_REQUEST_FAILURE;
						}

						free(pRestRequestArgs);
						free(rsslBuffer.data);

						if (pReactorConnectInfoImpl->reactorChannelInfoImplState == RSSL_RC_CHINFO_IMPL_ST_REQUEST_FAILURE)
						{
							rsslSetErrorInfo(&pReactorChannel->channelWorkerCerr, RSSL_EIC_FAILURE, RSSL_RET_FAILURE, __FILE__, __LINE__,
								"Failed to send the REST request. Text: %s", rsslError.text);

							/* Notify error back to the application via the channel event */
							if (!RSSL_ERROR_INFO_CHECK(_reactorWorkerHandleChannelFailure(pReactorChannel->pParentReactor, pReactorChannel, &pReactorChannel->channelWorkerCerr) == RSSL_RET_SUCCESS, RSSL_RET_FAILURE, &pReactorWorker->workerCerr))
							{
								_reactorWorkerShutdown(pReactorImpl, &pReactorWorker->workerCerr);
								return;
							}
						}
						else
						{
							pReactorConnectInfoImpl->reactorChannelInfoImplState = RSSL_RC_CHINFO_IMPL_ST_QUERYING_SERVICE_DISOVERY;
						}
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
									_reactorWorkerShutdown(pReactorImpl, &pReactorWorker->workerCerr);
									return;
								}
							}
							else
							{
								RsslRet ret = RSSL_RET_SUCCESS;

								/* Reset channel statistics as the new connection is established */
								if (pReactorChannel->pChannelStatistic)
								{
									rsslClearReactorChannelStatistic(pReactorChannel->pChannelStatistic);
								}

								/* Set debug callback usage here. */
								if (pReactorChannel->connectionDebugFlags != 0 && rsslIoctl(pReactorChannel->reactorChannel.pRsslChannel, RSSL_DEBUG_FLAGS, (void*)&(pReactorChannel->connectionDebugFlags), &pReactorChannel->channelWorkerCerr.rsslError) != RSSL_RET_SUCCESS)
								{
									if (!RSSL_ERROR_INFO_CHECK(_reactorWorkerHandleChannelFailure(pReactorImpl, pReactorChannel, &pReactorChannel->channelWorkerCerr) == RSSL_RET_SUCCESS, RSSL_RET_FAILURE, &pReactorWorker->workerCerr))
									{
										_reactorWorkerShutdown(pReactorChannel->pParentReactor, &pReactorWorker->workerCerr);
										return;
									}
									return;
								}

								pReactorChannel->channelSetupState = RSSL_RC_CHST_INIT;
								pReactorChannel->initializationTimeout = pReactorConnectInfoImpl->base.initializationTimeout;
								if (!RSSL_ERROR_INFO_CHECK(_reactorWorkerProcessNewChannel(pReactorChannel->pParentReactor, pReactorChannel) == RSSL_RET_SUCCESS, ret, &pReactorWorker->workerCerr))
								{
									_reactorWorkerShutdown(pReactorImpl, &pReactorWorker->workerCerr);
									return;
								}
							}
						}
					}
				}
			}

			break;
		}
		case 301: /* Moved Permanently */
		case 308: /* Permanent Redirect */
		{
			RsslBuffer* uri = NULL;

			RSSL_MUTEX_UNLOCK(&pReactorTokenSession->accessTokenMutex); /* Releases the access token mutex before locking the session mutex */

			uri = getHeaderValue(&restresponse->headers, &rssl_rest_location_header_text);

			if (uri != NULL)
			{
				RsslRestRequestArgs *pRestRequestArgs;

				/* Ensure that there is no thread accessing the token service URL while updating it */
				RSSL_MUTEX_LOCK(&pReactorWorker->reactorTokenManagement.tokenSessionMutex);

				/* Copy the new URL to the rsslReactorImpl */
				if (uri->length > pReactorImpl->tokenServiceURLBuffer.length)
				{
					free(pReactorImpl->tokenServiceURLBuffer.data);
					pReactorImpl->tokenServiceURLBuffer.length = uri->length + 1;
					pReactorImpl->tokenServiceURLBuffer.data = (char*)malloc(pReactorImpl->tokenServiceURLBuffer.length);
					if (pReactorImpl->tokenServiceURLBuffer.data == 0)
					{
						RSSL_MUTEX_UNLOCK(&pReactorWorker->reactorTokenManagement.tokenSessionMutex);
						goto RequestFailed;
					}
				}

				memset(pReactorImpl->tokenServiceURLBuffer.data, 0, pReactorImpl->tokenServiceURLBuffer.length);
				pReactorImpl->tokenServiceURL.data = pReactorImpl->tokenServiceURLBuffer.data;
				pReactorImpl->tokenServiceURL.length = uri->length - 1;
				memcpy(pReactorImpl->tokenServiceURL.data, uri->data + 1, pReactorImpl->tokenServiceURL.length);

				RSSL_MUTEX_UNLOCK(&pReactorWorker->reactorTokenManagement.tokenSessionMutex);

				RSSL_MUTEX_LOCK(&pReactorTokenSession->accessTokenMutex);
				if (pReactorTokenSession->pOAuthCredential->pOAuthCredentialEventCallback == 0)
				{
					pRestRequestArgs = _reactorCreateRequestArgsForPassword(pReactorImpl, &pReactorImpl->tokenServiceURL,
						&pReactorTokenSession->pOAuthCredential->userName,
						&pReactorTokenSession->pOAuthCredential->password,
						NULL, /* For specifying a new password */
						&pReactorTokenSession->pOAuthCredential->clientId,
						&pReactorTokenSession->pOAuthCredential->clientSecret,
						&pReactorTokenSession->pOAuthCredential->tokenScope,
						&pReactorTokenSession->rsslPostDataBodyBuf, pReactorTokenSession, &pReactorWorker->workerCerr);

					if (pRestRequestArgs)
					{
						_assignConnectionArgsToRequestArgs(&pReactorTokenSession->proxyConnectOpts, pRestRequestArgs);

						if ((pReactorTokenSession->pRestHandle = rsslRestClientNonBlockingRequest(pReactorImpl->pRestClient, pRestRequestArgs,
							rsslRestAuthTokenResponseCallback, rsslRestAuthTokenErrorCallback,
							&pReactorTokenSession->rsslAccessTokenRespBuffer, &pReactorTokenSession->tokenSessionWorkerCerr.rsslError)) != 0)
						{
							RSSL_QUEUE_FOR_EACH_LINK(&pReactorTokenSession->reactorChannelList, pLink)
							{
								pReactorChannel = RSSL_QUEUE_LINK_TO_OBJECT(RsslReactorChannelImpl, tokenSessionLink, pLink);
								pReactorConnectInfoImpl = &pReactorChannel->connectionOptList[pReactorChannel->connectionListIter];
								pReactorConnectInfoImpl->reactorChannelInfoImplState = RSSL_RC_CHINFO_IMPL_ST_REQ_AUTH_TOKEN;
							}

							pReactorTokenSession->originalExpiresIn = 0; /* Unset to indicate that the password grant is sent. */
							pReactorTokenSession->tokenSessionState = RSSL_RC_TOKEN_SESSION_IMPL_REQ_INIT_AUTH_TOKEN;
							pReactorTokenSession->tokenMgntEventType = RSSL_RCIMPL_TKET_REISSUE;
							free(pRestRequestArgs);

							/* Unlock the access mutex whenever receives the response */
						}
						else
						{
							RSSL_MUTEX_UNLOCK(&pReactorTokenSession->accessTokenMutex);

							free(pRestRequestArgs);

							if (!RSSL_ERROR_INFO_CHECK(_reactorWorkerHandleChannelFailure(pReactorImpl, pReactorChannel, &pReactorChannel->channelWorkerCerr) == RSSL_RET_SUCCESS, RSSL_RET_FAILURE, &pReactorWorker->workerCerr))
							{
								_reactorWorkerShutdown(pReactorImpl, &pReactorWorker->workerCerr);
								return;
							}
						}
					}
					else
					{
						RSSL_MUTEX_UNLOCK(&pReactorTokenSession->accessTokenMutex);
						if (!RSSL_ERROR_INFO_CHECK(_reactorWorkerHandleChannelFailure(pReactorImpl, pReactorChannel, &pReactorChannel->channelWorkerCerr) == RSSL_RET_SUCCESS, RSSL_RET_FAILURE, &pReactorWorker->workerCerr))
						{
							_reactorWorkerShutdown(pReactorImpl, &pReactorWorker->workerCerr);
							return;
						}
					}
				}
				else
				{
					RSSL_MUTEX_UNLOCK(&pReactorTokenSession->accessTokenMutex);

					/* Notifies the application via the callback method to pass in sensitive information */
					RSSL_QUEUE_FOR_EACH_LINK(&pReactorTokenSession->reactorChannelList, pLink)
					{
						pReactorChannel = RSSL_QUEUE_LINK_TO_OBJECT(RsslReactorChannelImpl, tokenSessionLink, pLink);
						pReactorConnectInfoImpl = &pReactorChannel->connectionOptList[pReactorChannel->connectionListIter];
						pReactorConnectInfoImpl->reactorChannelInfoImplState = RSSL_RC_CHINFO_IMPL_ST_REQ_AUTH_TOKEN;
					}

					pReactorTokenSession->tokenSessionState = RSSL_RC_TOKEN_SESSION_IMPL_WAITING_TO_REQ_AUTH_TOKEN;

					notifySensitiveInfoReq(pReactorTokenSession, pReactorWorker);
				}
			}
			else
			{
				goto RequestFailed;
			}

			break;
		}
		case 302: /* Found (Previously "Moved temporarily") */
		case 307: /* Temporary Redirect (Request method is not allowed to be changed) */
		{
			RsslBuffer* tempUri = NULL;

			RSSL_MUTEX_UNLOCK(&pReactorTokenSession->accessTokenMutex);
				
			tempUri = getHeaderValue(&restresponse->headers, &rssl_rest_location_header_text);

			if (tempUri != NULL)
			{
				RsslRestRequestArgs *pRestRequestArgs;

				RSSL_MUTEX_LOCK(&pReactorTokenSession->accessTokenMutex);
				if (pReactorTokenSession->pOAuthCredential->pOAuthCredentialEventCallback == 0)
				{
					RsslBuffer requestURL;
					requestURL.data = (tempUri->data + 1);
					requestURL.length = tempUri->length - 1;

					pRestRequestArgs = _reactorCreateRequestArgsForPassword(pReactorImpl, &requestURL,
						&pReactorTokenSession->pOAuthCredential->userName,
						&pReactorTokenSession->pOAuthCredential->password,
						NULL, /* For specifying a new password */
						&pReactorTokenSession->pOAuthCredential->clientId,
						&pReactorTokenSession->pOAuthCredential->clientSecret,
						&pReactorTokenSession->pOAuthCredential->tokenScope,
						&pReactorTokenSession->rsslPostDataBodyBuf, pReactorTokenSession, &pReactorWorker->workerCerr);

					if (pRestRequestArgs)
					{
						_assignConnectionArgsToRequestArgs(&pReactorTokenSession->proxyConnectOpts, pRestRequestArgs);

						if ((pReactorTokenSession->pRestHandle = rsslRestClientNonBlockingRequest(pReactorImpl->pRestClient, pRestRequestArgs,
							rsslRestAuthTokenResponseCallback, rsslRestAuthTokenErrorCallback,
							&pReactorTokenSession->rsslAccessTokenRespBuffer, &pReactorTokenSession->tokenSessionWorkerCerr.rsslError)) != 0)
						{
							RSSL_QUEUE_FOR_EACH_LINK(&pReactorTokenSession->reactorChannelList, pLink)
							{
								pReactorChannel = RSSL_QUEUE_LINK_TO_OBJECT(RsslReactorChannelImpl, tokenSessionLink, pLink);
								pReactorConnectInfoImpl = &pReactorChannel->connectionOptList[pReactorChannel->connectionListIter];
								pReactorConnectInfoImpl->reactorChannelInfoImplState = RSSL_RC_CHINFO_IMPL_ST_REQ_AUTH_TOKEN;
							}

							pReactorTokenSession->originalExpiresIn = 0; /* Unset to indicate that the password grant is sent. */
							pReactorTokenSession->tokenSessionState = RSSL_RC_TOKEN_SESSION_IMPL_REQ_INIT_AUTH_TOKEN;
							pReactorTokenSession->tokenMgntEventType = RSSL_RCIMPL_TKET_REISSUE;
							free(pRestRequestArgs);

							/* Unlock the access mutex whenever receives the response */
						}
						else
						{
							RSSL_MUTEX_UNLOCK(&pReactorTokenSession->accessTokenMutex);

							free(pRestRequestArgs);

							if (!RSSL_ERROR_INFO_CHECK(_reactorWorkerHandleChannelFailure(pReactorImpl, pReactorChannel, &pReactorChannel->channelWorkerCerr) == RSSL_RET_SUCCESS, RSSL_RET_FAILURE, &pReactorWorker->workerCerr))
							{
								_reactorWorkerShutdown(pReactorImpl, &pReactorWorker->workerCerr);
								return;
							}
						}
					}
					else
					{
						RSSL_MUTEX_UNLOCK(&pReactorTokenSession->accessTokenMutex);
						if (!RSSL_ERROR_INFO_CHECK(_reactorWorkerHandleChannelFailure(pReactorImpl, pReactorChannel, &pReactorChannel->channelWorkerCerr) == RSSL_RET_SUCCESS, RSSL_RET_FAILURE, &pReactorWorker->workerCerr))
						{
							_reactorWorkerShutdown(pReactorImpl, &pReactorWorker->workerCerr);
							return;
						}
					}
				}
				else
				{
					RSSL_MUTEX_UNLOCK(&pReactorTokenSession->accessTokenMutex);

					/* Copy the new URL to the rsslReactorImpl */
					if (tempUri->length > pReactorTokenSession->temporaryURLBuffer.length)
					{
						free(pReactorTokenSession->temporaryURLBuffer.data);
						pReactorTokenSession->temporaryURLBuffer.length = tempUri->length + 1;
						pReactorTokenSession->temporaryURLBuffer.data = (char*)malloc(pReactorTokenSession->temporaryURLBuffer.length);
						if (pReactorTokenSession->temporaryURLBuffer.data == 0)
						{
							goto RequestFailed;
						}
					}

					memset(pReactorTokenSession->temporaryURLBuffer.data, 0, pReactorTokenSession->temporaryURLBuffer.length);
					pReactorTokenSession->temporaryURL.data = pReactorTokenSession->temporaryURLBuffer.data;
					pReactorTokenSession->temporaryURL.length = tempUri->length - 1;
					memcpy(pReactorTokenSession->temporaryURL.data, tempUri->data + 1, pReactorTokenSession->temporaryURL.length);


					/* Notifies the application via the callback method to pass in sensitive information */
					RSSL_QUEUE_FOR_EACH_LINK(&pReactorTokenSession->reactorChannelList, pLink)
					{
						pReactorChannel = RSSL_QUEUE_LINK_TO_OBJECT(RsslReactorChannelImpl, tokenSessionLink, pLink);
						pReactorConnectInfoImpl = &pReactorChannel->connectionOptList[pReactorChannel->connectionListIter];
						pReactorConnectInfoImpl->reactorChannelInfoImplState = RSSL_RC_CHINFO_IMPL_ST_REQ_AUTH_TOKEN;
					}

					pReactorTokenSession->tokenSessionState = RSSL_RC_TOKEN_SESSION_IMPL_WAITING_TO_REQ_AUTH_TOKEN;

					notifySensitiveInfoReq(pReactorTokenSession, pReactorWorker);
				}
			}
			else
			{
				goto RequestFailed;
			}

			break;
		}
		case 403: /* Forbidden */
		case 451: /* Unavailable For Legal Reasons */
		{
			/* Stop retrying with the request */
			RSSL_MUTEX_UNLOCK(&pReactorTokenSession->accessTokenMutex);

			pReactorTokenSession->tokenSessionState = RSSL_RC_TOKEN_SESSION_IMPL_STOP_REQUESTING;

			rsslSetErrorInfo(&pReactorTokenSession->tokenSessionWorkerCerr, RSSL_EIC_FAILURE, RSSL_RET_FAILURE, __FILE__, __LINE__,
				"Received HTTP error %u status code with data body : %s.", restresponse->statusCode, restresponse->dataBody.data);

			handlingAuthRequestFailure(pReactorTokenSession, pReactorWorker);

			break;
		}
		case 400: /* Bad Request */
		case 401: /* Unauthorized */
		{
			/* Clears the exiting access token before requesting it */
			rsslClearBuffer(&pReactorTokenSession->tokenInformation.accessToken);
			
			/* Fall through to retry */
		}
		default:
		{
			RSSL_MUTEX_UNLOCK(&pReactorTokenSession->accessTokenMutex);

			pReactorTokenSession->tokenSessionState = RSSL_RC_TOKEN_SESSION_IMPL_REQUEST_FAILURE;

			rsslSetErrorInfo(&pReactorTokenSession->tokenSessionWorkerCerr, RSSL_EIC_FAILURE, RSSL_RET_FAILURE, __FILE__, __LINE__,
				"Received HTTP error %u status code with data body : %s.", restresponse->statusCode, restresponse->dataBody.data);

			handlingAuthRequestFailure(pReactorTokenSession, pReactorWorker);
		}
	}

	return;

RequestFailed:
	rsslSetErrorInfo(&pReactorTokenSession->tokenSessionWorkerCerr, RSSL_EIC_FAILURE, RSSL_RET_FAILURE, __FILE__, __LINE__,
		"Failed to send the REST request. Text: %s", rsslError.text);

	handlingAuthRequestFailure(pReactorTokenSession, pReactorWorker);

	pReactorTokenSession->tokenSessionState = RSSL_RC_TOKEN_SESSION_IMPL_REQUEST_FAILURE;

	return;

ReAuthorization:

	/* Send another request using password grant type */
	pReactorTokenSession->resendFromFailure = 1;
	RTR_ATOMIC_SET(pReactorTokenSession->sendTokenRequest, 1);
	pReactorTokenSession->nextExpiresTime = getCurrentTimeMs(pReactorImpl->ticksPerMsec);
	pReactorTokenSession->tokenSessionState = RSSL_RC_TOKEN_SESSION_IMPL_REQUEST_FAILURE;
	pReactorTokenSession->originalExpiresIn = 0; /* Unset to indicate that the password grant will be sent. */
}

static void rsslRestAuthTokenErrorCallback(RsslError* rsslError, RsslRestResponseEvent* event)
{
	RsslReactorTokenSessionImpl *pReactorTokenSession = (RsslReactorTokenSessionImpl*)event->closure;
	RsslReactorImpl *pReactorImpl = (RsslReactorImpl*)pReactorTokenSession->pReactor;
	RsslReactorWorker *pReactorWorker = &pReactorImpl->reactorWorker;
	RsslReactorChannelImpl *pReactorChannel = NULL;
	RsslReactorConnectInfoImpl *pReactorConnectInfoImpl = NULL;
	RsslQueueLink *pLink;

	rsslSetErrorInfo(&pReactorTokenSession->tokenSessionWorkerCerr, RSSL_EIC_FAILURE, RSSL_RET_FAILURE, __FILE__, __LINE__,
		"Failed to send the REST request. Text: %s", rsslError->text);

	/* Reset the HTTP response status code as there is no response */
	pReactorTokenSession->httpStausCode = 0;

	/* Cleaning up the RsslRestHandle later by the ReactorWorker */
	rsslQueueAddLinkToBack(&pReactorWorker->disposableRestHandles, &event->handle->queueLink);
	pReactorTokenSession->pRestHandle = NULL;

	RSSL_MUTEX_UNLOCK(&pReactorTokenSession->accessTokenMutex);

	if (rsslError->rsslErrorId == RSSL_RET_BUFFER_TOO_SMALL)
	{
		RSSL_QUEUE_FOR_EACH_LINK(&pReactorTokenSession->reactorChannelList, pLink)
		{
			pReactorChannel = RSSL_QUEUE_LINK_TO_OBJECT(RsslReactorChannelImpl, tokenSessionLink, pLink);
			pReactorConnectInfoImpl = &pReactorChannel->connectionOptList[pReactorChannel->connectionListIter];

			/* Notify error back to the application via the channel event */
			if (!RSSL_ERROR_INFO_CHECK(_reactorWorkerHandleChannelFailure(pReactorImpl, pReactorChannel, &pReactorTokenSession->tokenSessionWorkerCerr) == RSSL_RET_SUCCESS, RSSL_RET_FAILURE, &pReactorWorker->workerCerr))
			{
				_reactorWorkerShutdown(pReactorChannel->pParentReactor, &pReactorWorker->workerCerr);
			}

			pReactorConnectInfoImpl->reactorChannelInfoImplState = RSSL_RC_CHINFO_IMPL_ST_BUFFER_TOO_SMALL;
		}
	}
	else
	{
		pReactorTokenSession->tokenSessionState = RSSL_RC_TOKEN_SESSION_IMPL_REQUEST_FAILURE;
		handlingAuthRequestFailure(pReactorTokenSession, pReactorWorker);
	}
}

static void rsslRestAuthTokenResponseWithoutSessionCallback(RsslRestResponse* restresponse, RsslRestResponseEvent* event)
{
	RsslReactorOAuthCredentialRenewalImpl *pReactorOAuthCredentialRenewalImpl = (RsslReactorOAuthCredentialRenewalImpl*)event->closure;
	RsslReactorImpl *pRsslReactorImpl = (RsslReactorImpl*)pReactorOAuthCredentialRenewalImpl->pRsslReactor;
	RsslReactorWorker *pReactorWorker = &pRsslReactorImpl->reactorWorker;
	RsslError rsslError;
	RsslReactorTokenMgntEvent *pEvent;
	RsslReactorErrorInfoImpl *pReactorErrorInfoImpl = NULL;

	/* Releases the old memory and points the buffer to the new location. */
	if (restresponse->isMemReallocated)
	{
		free(event->userMemory->data);
		(*event->userMemory) = restresponse->reallocatedMem;
	}

	/* Cleaning up the RsslRestHandle later by the ReactorWorker */
	rsslQueueAddLinkToBack(&pReactorWorker->disposableRestHandles, &event->handle->queueLink);

	pReactorOAuthCredentialRenewalImpl->httpStatusCode = restresponse->statusCode;

	pEvent = (RsslReactorTokenMgntEvent*)rsslReactorEventQueueGetFromPool(&pRsslReactorImpl->reactorEventQueue);
	rsslClearReactorTokenMgntEvent(pEvent);

	pEvent->pOAuthCredentialRenewal = (RsslReactorOAuthCredentialRenewal*)pReactorOAuthCredentialRenewalImpl;
	pEvent->pAuthTokenEventCallback = pReactorOAuthCredentialRenewalImpl->pAuthTokenEventCallback;

	switch (restresponse->statusCode)
	{
		case 200:
		{
			if (pReactorOAuthCredentialRenewalImpl->tokenInformationBuffer.length < restresponse->dataBody.length)
			{
				if (pReactorOAuthCredentialRenewalImpl->tokenInformationBuffer.data)
				{
					free(pReactorOAuthCredentialRenewalImpl->tokenInformationBuffer.data);
				}

				pReactorOAuthCredentialRenewalImpl->tokenInformationBuffer.length = restresponse->dataBody.length;
				pReactorOAuthCredentialRenewalImpl->tokenInformationBuffer.data = (char*)malloc(pReactorOAuthCredentialRenewalImpl->tokenInformationBuffer.length);

				if (pReactorOAuthCredentialRenewalImpl->tokenInformationBuffer.data == 0)
				{
					pReactorErrorInfoImpl = rsslReactorGetErrorInfoFromPool(pReactorWorker);
					rsslClearReactorErrorInfoImpl(pReactorErrorInfoImpl);

					pReactorErrorInfoImpl->referenceCount++;
					rsslSetErrorInfo(&pReactorErrorInfoImpl->rsslErrorInfo, RSSL_EIC_FAILURE, RSSL_RET_FAILURE, __FILE__, __LINE__,
						"Failed to allocate memory for parsing the token information");

					pEvent->pReactorErrorInfoImpl = pReactorErrorInfoImpl;

					goto RequestFailed;
				}
			}

			if (rsslRestParseAccessToken(&restresponse->dataBody, &pReactorOAuthCredentialRenewalImpl->tokenInformation.accessToken, &pReactorOAuthCredentialRenewalImpl->tokenInformation.refreshToken,
				&pReactorOAuthCredentialRenewalImpl->tokenInformation.expiresIn, &pReactorOAuthCredentialRenewalImpl->tokenInformation.tokenType,
				&pReactorOAuthCredentialRenewalImpl->tokenInformation.scope, &pReactorOAuthCredentialRenewalImpl->tokenInformationBuffer, &rsslError) != RSSL_RET_SUCCESS)
			{
				pReactorErrorInfoImpl = rsslReactorGetErrorInfoFromPool(pReactorWorker);
				rsslClearReactorErrorInfoImpl(pReactorErrorInfoImpl);

				pReactorErrorInfoImpl->referenceCount++;
				rsslSetErrorInfo(&pReactorErrorInfoImpl->rsslErrorInfo, RSSL_EIC_FAILURE, RSSL_RET_FAILURE, __FILE__, __LINE__,
					"Failed to parse the token information. Text: %s", rsslError.text);

				pEvent->pReactorErrorInfoImpl = pReactorErrorInfoImpl;

				goto RequestFailed;
			}

			pEvent->reactorTokenMgntEventType = RSSL_RCIMPL_TKET_RENEW_TOKEN;

			if (!RSSL_ERROR_INFO_CHECK(rsslReactorEventQueuePut(&pRsslReactorImpl->reactorEventQueue, (RsslReactorEventImpl*)pEvent)
				== RSSL_RET_SUCCESS, RSSL_RET_FAILURE, &pReactorWorker->workerCerr))
			{
				_reactorWorkerShutdown(pRsslReactorImpl, &pReactorWorker->workerCerr);
				return;
			}

			break;
		}
		default:
		{
			pReactorErrorInfoImpl = rsslReactorGetErrorInfoFromPool(pReactorWorker);
			rsslClearReactorErrorInfoImpl(pReactorErrorInfoImpl);

			pReactorErrorInfoImpl->referenceCount++;
			rsslSetErrorInfo(&pReactorErrorInfoImpl->rsslErrorInfo, RSSL_EIC_FAILURE, RSSL_RET_FAILURE, __FILE__, __LINE__,
				"Received HTTP error %u status code with data body : %s.", restresponse->statusCode, restresponse->dataBody.data);

			pEvent->pReactorErrorInfoImpl = pReactorErrorInfoImpl;

			goto RequestFailed;
		}
	}

	return;

RequestFailed:

	pEvent->reactorTokenMgntEventType = RSSL_RCIMPL_TKET_RESP_FAILURE;

	if (!RSSL_ERROR_INFO_CHECK(rsslReactorEventQueuePut(&pRsslReactorImpl->reactorEventQueue, (RsslReactorEventImpl*)pEvent)
		== RSSL_RET_SUCCESS, RSSL_RET_FAILURE, &pReactorWorker->workerCerr))
	{
		_reactorWorkerShutdown(pRsslReactorImpl, &pReactorWorker->workerCerr);
		return;
	}
}

static void rsslRestErrorWithoutSessionCallback(RsslError* rsslError, RsslRestResponseEvent* event)
{
	RsslReactorOAuthCredentialRenewalImpl *pReactorOAuthCredentialRenewalImpl = (RsslReactorOAuthCredentialRenewalImpl*)event->closure;
	RsslReactorImpl *pRsslReactorImpl = (RsslReactorImpl*)pReactorOAuthCredentialRenewalImpl->pRsslReactor;
	RsslReactorWorker *pReactorWorker = &pRsslReactorImpl->reactorWorker;
	RsslReactorTokenMgntEvent *pEvent;
	RsslReactorErrorInfoImpl *pReactorErrorInfoImpl = rsslReactorGetErrorInfoFromPool(pReactorWorker);

	rsslClearReactorErrorInfoImpl(pReactorErrorInfoImpl);

	/* Cleaning up the RsslRestHandle later by the ReactorWorker */
	rsslQueueAddLinkToBack(&pReactorWorker->disposableRestHandles, &event->handle->queueLink);

	/* Reset the HTTP response status code as there is no response */
	pReactorOAuthCredentialRenewalImpl->httpStatusCode = 0;

	pEvent = (RsslReactorTokenMgntEvent*)rsslReactorEventQueueGetFromPool(&pRsslReactorImpl->reactorEventQueue);
	rsslClearReactorTokenMgntEvent(pEvent);

	pEvent->pOAuthCredentialRenewal = (RsslReactorOAuthCredentialRenewal*)pReactorOAuthCredentialRenewalImpl;
	pEvent->pAuthTokenEventCallback = pReactorOAuthCredentialRenewalImpl->pAuthTokenEventCallback;
	pEvent->reactorTokenMgntEventType = RSSL_RCIMPL_TKET_RESP_FAILURE;

	pReactorErrorInfoImpl->referenceCount++;
	rsslSetErrorInfo(&pReactorErrorInfoImpl->rsslErrorInfo, RSSL_EIC_FAILURE, RSSL_RET_FAILURE, __FILE__, __LINE__,
		"Failed to send the REST request. Text: %s", rsslError->text);

	pEvent->pReactorErrorInfoImpl = pReactorErrorInfoImpl;

	if (!RSSL_ERROR_INFO_CHECK(rsslReactorEventQueuePut(&pRsslReactorImpl->reactorEventQueue, (RsslReactorEventImpl*)pEvent)
		== RSSL_RET_SUCCESS, RSSL_RET_FAILURE, &pReactorWorker->workerCerr))
	{
		_reactorWorkerShutdown(pRsslReactorImpl, &pReactorWorker->workerCerr);
		return;
	}
}

