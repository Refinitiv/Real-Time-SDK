/*
 * This source code is provided under the Apache 2.0 license and is provided
 * AS IS with no warranty or guarantee of fit for purpose.  See the project's 
 * LICENSE.md for details. 
 * Copyright (C) 2019-2022 Refinitiv. All rights reserved.
*/

#include "rtr/rsslReactorImpl.h"
#include "rtr/rsslReactorEventsImpl.h"
#include "rtr/rsslWatchlist.h"
#include "rtr/rsslWatchlistImpl.h"
#include "rtr/tunnelStreamImpl.h"
#include "rtr/msgQueueEncDec.h"

#include <assert.h>
#include <stdint.h>
#ifdef _WIN32
#include <windows.h>
#include <process.h>
#else
#include <sys/types.h>
#include <unistd.h>
#include <sys/resource.h>
#include <alloca.h>
#include "sys/time.h"
#endif

#include <sys/timeb.h>
#include <time.h>

#define RSSL_REACTOR_MAX_JSON_ERROR_MSG_SIZE 1101
#define RSSL_REACTOR_DEBUGGING_BUFFER_MIN_INIT_SIZE 512000

RsslBuffer PONG_MESSAGE = { 15, (char*)"{\"Type\":\"Pong\"}" };

RsslBuffer REACTOR_EMPTY_DEBUG_INFO = { 1, (char*)"" };

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

/* Creates a deep copy of the Reactor Channel Role structure for the addtional warmstandby channels. */
static RsslRet _reactorChannelCopyRoleForWarmStandBy(RsslReactorChannelImpl* pReactorChannel, RsslReactorChannelRole* pRole, RsslErrorInfo* pError);

/* Sends an "RSSL_RC_CET_CHANNEL_READY" to the reactor's own queue */
static RsslRet _reactorSendConnReadyEvent(RsslReactorImpl *pReactorImpl, RsslReactorChannelImpl *pReactorChannel, RsslErrorInfo *pError);

/* Stops all channels in the reactor. */
static void _reactorShutdown(RsslReactorImpl *pReactorImpl, RsslErrorInfo *pError);

/* Creates a deep copy of the RsslReactorOAuthCredentialRenewal structure */
static RsslReactorOAuthCredentialRenewal* _reactorCopyRsslReactorOAuthCredentialRenewal(RsslReactorImpl *pReactorImpl, RsslReactorTokenSessionImpl *pReactorTokenSessionImpl, RsslReactorOAuthCredentialRenewalOptions *pOptions,
	RsslReactorOAuthCredentialRenewal* pOAuthCredentialRenewal, RsslErrorInfo *pError);

RsslBool compareOAuthCredentialForTokenSession(RsslReactorOAuthCredential* pOAuthCredential, RsslBuffer *pClientID, RsslBuffer *pPassword, RsslReactorOAuthCredential *pOAuthOther , RsslErrorInfo* pErrorInfo);

static RsslBool validateLoginResponseFromStandbyServer(RsslRDMLoginRefresh *pActiveServerLoginRefersh, RsslRDMLoginRefresh *pStandbyServerLoginRefersh);

static RsslRet copyReactorSubmitMsgOption(RsslReactorImpl *pReactorImpl, RsslReactorSubmitMsgOptionsImpl* pDestination, RsslReactorSubmitMsgOptions* pSource);

static RsslBool isWarmStandbyChannelClosed(RsslReactorWarmStandByHandlerImpl* pWarmStandbyHandler, RsslReactorChannelImpl* pReactorChanelImpl);

static RsslRet _processingReactorChannelShutdown(RsslReactorImpl *pReactorImpl, RsslReactorChannelImpl *pReactorChannel, RsslErrorInfo *pError);

static RsslBool provideServiceAtStartUp(RsslReactorWarmStandbyGroupImpl *pWarmStandbyGroupImpl, RsslBuffer* pServiceName, RsslReactorChannelImpl *pReactorChannelImpl);

static void _reactorHandlesWSBSocketList(RsslReactorWarmStandByHandlerImpl* pWarmStandbyHandler, RsslReactorChannel *pReactorChannelImpl, RsslSocket *removeSocket);

static RsslBool _reactorHandleWSBServerList(RsslReactorWarmStandByHandlerImpl* pWarmStandbyHandler, RsslReactorChannelImpl *pReactorChannelImpl, RsslRDMLoginRefresh *pRDMLoginRefresh);

static RsslRet _submitQueuedMessages(RsslReactorWarmStandByHandlerImpl* pWarmStandbyHandler, RsslReactorWarmStandbyGroupImpl *pWarmStandbyGroupImpl, RsslReactorChannelImpl *pReactorChannelImpl, RsslErrorInfo *pError);

static RsslRet _reactorQueuedWSBGroupRecoveryMsg(RsslReactorWarmStandByHandlerImpl* pWarmStandbyHandler, RsslErrorInfo* pErrorInfo);
 
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
	RsslUInt8				wsbStatusFlags;	/* This is used to notify a status message for warmstandby. */
} ReactorProcessMsgOptions;

static RsslRet _reactorProcessMsg(RsslReactorImpl *pReactorImpl, RsslReactorChannelImpl *pReactorChannel, ReactorProcessMsgOptions *pOpts);

static RsslRet _reactorSubmitWatchlistMsg(RsslReactorImpl *pReactorImpl, RsslReactorChannelImpl *pReactorChannel, 
		RsslWatchlistProcessMsgOptions *pOptions, RsslErrorInfo *pError);

static RsslRet _reactorReadWatchlistMsg(RsslReactorImpl *pReactorImpl, RsslReactorChannelImpl *pReactorChannel,
		RsslWatchlistProcessMsgOptions *pOptions, ReactorWSProcessMsgOptions* pWsOption, RsslErrorInfo *pError);

static RsslBool _warmStandbyCallbackChecks(RsslReactorChannelImpl *pReactorChannelImpl, ReactorProcessMsgOptions *pOpts);

static RsslBool _getDictionaryVersion(RsslBuffer* srcBuffer, RsslReactorDictionaryVersion* pDictionaryVersion);

static RsslRet _reactorWSReadWatchlistMsg(RsslReactorImpl *pReactorImpl, RsslReactorChannelImpl *pReactorChannel,
	RsslWatchlistProcessMsgOptions *pOptions, RsslErrorInfo *pError);

static RsslRet _reactorWSHandleRecoveryStatusMsg(RsslReactorChannelImpl *pReactorChannel, ReactorProcessMsgOptions *pOptions);

static RsslRet _reactorWSNotifyStatusMsg(RsslReactorChannelImpl *pReactorChannel);

static RsslRet _reactorWSWriteWatchlistMsg(RsslReactorImpl *pReactorImpl, RsslReactorChannelImpl *pReactorChannel, RsslReactorSubmitMsgOptions *pOptions, 
	RsslWatchlistProcessMsgOptions *pProcessOpts, RsslErrorInfo *pError);

static void _reactorWSDirectoryUpdateFromChannelDown(RsslReactorWarmStandbyGroupImpl* pWarmStandbyGroupImpl, RsslReactorChannelImpl* pReactorChannelImpl, WlServiceCache* pServiceCache);

#ifdef __cplusplus
extern "C" {
#endif

static RsslRet _reactorWatchlistMsgCallback(RsslWatchlist *pWatchlist, RsslWatchlistMsgEvent *pEvent, RsslErrorInfo *pError);

#ifdef __cplusplus
};
#endif

/* current created reactor index */
static rtr_atomic_val currentIndReactor = 0;


static RsslRet _reactorSendConnReadyEvent(RsslReactorImpl *pReactorImpl, RsslReactorChannelImpl *pReactorChannel, RsslErrorInfo *pError)
{
	RsslReactorChannelEventImpl *pEvent = (RsslReactorChannelEventImpl*)rsslReactorEventQueueGetFromPool(&pReactorChannel->eventQueue);

	pReactorChannel->channelSetupState = RSSL_RC_CHST_READY;
	rsslClearReactorChannelEventImpl(pEvent);
	pEvent->channelEvent.channelEventType = RSSL_RC_CET_CHANNEL_READY;

	pEvent->channelEvent.pReactorChannel = (RsslReactorChannel*)pReactorChannel;
	pReactorChannel->hasConnected = RSSL_TRUE;
	if (!RSSL_ERROR_INFO_CHECK(rsslReactorEventQueuePut(&pReactorChannel->eventQueue, (RsslReactorEventImpl*)pEvent) == RSSL_RET_SUCCESS, RSSL_RET_FAILURE, pError))
		return RSSL_RET_FAILURE;

	return RSSL_RET_SUCCESS;
}

static RsslRet _reactorSendShutdownEvent(RsslReactorImpl *pReactorImpl, RsslErrorInfo *pError);

/* Send JSON message directly to network without using the JSON converter functionality */
static RsslRet _reactorSendJSONMessage(RsslReactorImpl *pReactorImpl, RsslReactorChannelImpl *pReactorChannel, RsslBuffer *pBuffer, RsslErrorInfo *pError)
{
	RsslRet ret;
	RsslReactorSubmitOptions submitOpts;
	RsslUInt32 dummyBytesWritten, dummyUncompBytesWritten;

	rsslClearReactorSubmitOptions(&submitOpts);

	if (pReactorChannel->reactorParentQueue != &pReactorImpl->activeChannels)
	{
		rsslSetErrorInfo(pError, RSSL_EIC_FAILURE, RSSL_RET_FAILURE, __FILE__, __LINE__, "Channel is not active.");
		return RSSL_RET_FAILURE;
	}

	ret = rsslWrite(pReactorChannel->reactorChannel.pRsslChannel,
		pBuffer,
		submitOpts.priority,
		submitOpts.writeFlags,
		&dummyBytesWritten,
		&dummyUncompBytesWritten,
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
				return ret;
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
	}

	if (ret != RSSL_RET_SUCCESS)
	{
		return _reactorHandleChannelDown(pReactorImpl, pReactorChannel, pError);
	}

	return ret;
}

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
	if (pNewList == &pReactorChannel->pParentReactor->channelPool)
	{
		_rsslCleanUpPackedBufferHashTable(pReactorChannel);
	}

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

static RsslRet _reactorSendCredentialRenewalRequest(RsslReactorImpl *pReactorImpl, RsslReactorTokenSessionImpl *pReactorTokenSessionImpl, RsslReactorOAuthCredentialRenewal *pReactorOAuthCredentialRenewal,
	RsslReactorOAuthCredentialRenewalEventType oAuthCredentialRenewalEventType, RsslErrorInfo *pError)
{
	/* Signal worker to perform authorization with the credential for this channel */
	RsslReactorCredentialRenewalEvent *pEvent = (RsslReactorCredentialRenewalEvent*)rsslReactorEventQueueGetFromPool(&pReactorImpl->reactorWorker.workerQueue);

	rsslClearReactorCredentialRenewalEvent(pEvent);
	pEvent->reactorCredentialRenewalEventType = oAuthCredentialRenewalEventType;
	pEvent->pTokenSessionImpl = pReactorTokenSessionImpl;
	pEvent->pOAuthCredentialRenewal = pReactorOAuthCredentialRenewal;
	if (!RSSL_ERROR_INFO_CHECK(rsslReactorEventQueuePut(&pReactorImpl->reactorWorker.workerQueue, (RsslReactorEventImpl*)pEvent) == RSSL_RET_SUCCESS, RSSL_RET_FAILURE, pError))
	{
		_reactorShutdown(pReactorImpl, pError);
		_reactorSendShutdownEvent(pReactorImpl, pError);
		return RSSL_RET_FAILURE;
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

static RsslRet _RsslJsonServiceNameToIdCallback(RsslBuffer *pServiceName, void *closure, RsslUInt16 *pServiceId)
{
	RsslReactorImpl *pReactorImpl = (RsslReactorImpl*)closure;
	RsslReactorServiceNameToIdEvent serviceNameToIdEvent;

	serviceNameToIdEvent.pUserSpec = pReactorImpl->userSpecPtr;

	return (*pReactorImpl->pServiceNameToIdCallback)((RsslReactor*)pReactorImpl, pServiceName, pServiceId, &serviceNameToIdEvent);
}

RSSL_VA_API RsslRet rsslReactorInitJsonConverter(RsslReactor *pReactor, RsslReactorJsonConverterOptions *pReactorJsonConverterOptions, RsslErrorInfo *pError)
{
	RsslReactorImpl *pReactorImpl = (RsslReactorImpl*)pReactor;
	RsslCreateJsonConverterOptions rjcOptions;
	RsslJsonConverterError rjcError;
	RsslJsonDictionaryListProperty dlProperty;
	RsslJsonServiceNameToIdCallbackProperty svcNameToIdCbProperty;
	RsslBool flag = RSSL_TRUE;
	RsslRet ret;

	if ((ret = reactorLockInterface(pReactorImpl, RSSL_TRUE, pError)) != RSSL_RET_SUCCESS)
		return ret;

	if (pReactorImpl->state != RSSL_REACTOR_ST_ACTIVE)
	{
		rsslSetErrorInfo(pError, RSSL_EIC_SHUTDOWN, RSSL_RET_INVALID_ARGUMENT, __FILE__, __LINE__, "Reactor is shutting down.");
		return (reactorUnlockInterface(pReactorImpl), RSSL_RET_FAILURE);
	}

	if (pReactorImpl->jsonConverterInitialized == RSSL_TRUE)
	{
		rsslSetErrorInfo(pError, RSSL_EIC_FAILURE, RSSL_RET_FAILURE, __FILE__, __LINE__,
			"The RsslJsonConverter has been initialized");

		return (reactorUnlockInterface(pReactorImpl), RSSL_RET_FAILURE);
	}

	rsslClearCreateRsslJsonConverterOptions(&rjcOptions);
	
	/* Initialize string table */
	rsslJsonInitialize();

	/* Set the maximum output buffer size for the converter. */
	rjcOptions.bufferSize = pReactorJsonConverterOptions->outputBufferSize;

	pReactorImpl->pJsonConverter = rsslCreateRsslJsonConverter(&rjcOptions, &rjcError);
	if (pReactorImpl->pJsonConverter == NULL)
	{
		rsslJsonUninitialize();
		rsslSetErrorInfo(pError, RSSL_EIC_FAILURE, RSSL_RET_FAILURE, __FILE__, __LINE__, "Failed to create RsslJsonConverter: %s", rjcError.text);
		return (reactorUnlockInterface(pReactorImpl), RSSL_RET_FAILURE);
	}

	pReactorImpl->pDictionaryList = malloc(sizeof(RsslDataDictionary*) * 1); /* RsslJsonConverter supports only one RsslDataDictionary */
	if (pReactorImpl->pDictionaryList == NULL)
	{
		rsslJsonUninitialize();
		rsslDestroyRsslJsonConverter(pReactorImpl->pJsonConverter, &rjcError);
		
		rsslSetErrorInfo(pError, RSSL_EIC_FAILURE, RSSL_RET_FAILURE, __FILE__, __LINE__, "Failed to allocate memory to keep a list of RsslDataDictionary");
		return (reactorUnlockInterface(pReactorImpl), RSSL_RET_FAILURE);
	}

	/* Set RsslDataDictionary specified users */
	pReactorImpl->pDictionaryList[0] = pReactorJsonConverterOptions->pDictionary;

	/* Set dictionary list. */
	rsslClearConverterDictionaryListProperty(&dlProperty);
	dlProperty.dictionaryListLength = 1;
	dlProperty.pDictionaryList = pReactorImpl->pDictionaryList;
	if (rsslJsonConverterSetProperty(pReactorImpl->pJsonConverter,
								RSSL_JSON_CPC_DICTIONARY_LIST, 
								&dlProperty, &rjcError) != RSSL_RET_SUCCESS)
	{
		rsslSetErrorInfo(pError, RSSL_EIC_FAILURE, RSSL_RET_FAILURE, __FILE__, __LINE__, 
		"Failed setting RsslJsonConverter property: dictionary list [%s]",
		rjcError.text);

		goto FailedToInitJsonConverter;
	}

	/* Checks whether the callback method is set by users */
	if (pReactorJsonConverterOptions->pServiceNameToIdCallback)
	{
		rsslJsonClearServiceNameToIdCallbackProperty(&svcNameToIdCbProperty);
		svcNameToIdCbProperty.callback = _RsslJsonServiceNameToIdCallback;
		svcNameToIdCbProperty.closure = pReactorImpl;
		pReactorImpl->userSpecPtr = pReactorJsonConverterOptions->userSpecPtr;
		pReactorImpl->pServiceNameToIdCallback = pReactorJsonConverterOptions->pServiceNameToIdCallback;
		/* Set service-name/ID callbacks. */
		if (rsslJsonConverterSetProperty(pReactorImpl->pJsonConverter,
			RSSL_JSON_CPC_SERVICE_NAME_TO_ID_CALLBACK,
			&svcNameToIdCbProperty, &rjcError) != RSSL_RET_SUCCESS)
		{
			rsslSetErrorInfo(pError, RSSL_EIC_FAILURE, RSSL_RET_FAILURE, __FILE__, __LINE__,
				"Failed setting RsslJsonConverter property: service name to ID callback [%s]",
				rjcError.text);

			goto FailedToInitJsonConverter;
		}
	}

	if (pReactorJsonConverterOptions->defaultServiceId > 65535)
	{
		rsslSetErrorInfo(pError, RSSL_EIC_FAILURE, RSSL_RET_FAILURE, __FILE__, __LINE__,
			"Failed setting RsslJsonConverter property: default service ID [%d]. The service ID must be in a range between 0 to 65535", pReactorJsonConverterOptions->defaultServiceId);

		goto FailedToInitJsonConverter;
	}

	if (pReactorJsonConverterOptions->defaultServiceId >= 0)
	{
		RsslUInt16 defaultServiceID = (RsslUInt16)pReactorJsonConverterOptions->defaultServiceId;
		/* Set default service ID. */
		if (rsslJsonConverterSetProperty(pReactorImpl->pJsonConverter,
			RSSL_JSON_CPC_DEFAULT_SERVICE_ID,
			&defaultServiceID, &rjcError) != RSSL_RET_SUCCESS)
		{
			rsslSetErrorInfo(pError, RSSL_EIC_FAILURE, RSSL_RET_FAILURE, __FILE__, __LINE__,
				"Failed setting RsslJsonConverter property: default service ID [%s]", rjcError.text);

			goto FailedToInitJsonConverter;
		}
	}

	/* When converting from RWF to JSON, add a QoS range on requests that do not specify a QoS */
	flag = RSSL_FALSE;
	if (rsslJsonConverterSetProperty(pReactorImpl->pJsonConverter,
								RSSL_JSON_CPC_USE_DEFAULT_DYNAMIC_QOS, 
								&flag, &rjcError) != RSSL_RET_SUCCESS)
	{
		rsslSetErrorInfo(pError, RSSL_EIC_FAILURE, RSSL_RET_FAILURE, __FILE__, __LINE__, 
		"Failed setting RsslJsonConverter property: add default QoS range [%s]", rjcError.text);

		goto FailedToInitJsonConverter;
	}

	/* Expand enumerated values in field entries to their display values. 
	 * Dictionary must have enumerations loaded */
	flag = pReactorJsonConverterOptions->jsonExpandedEnumFields;
	if (rsslJsonConverterSetProperty(pReactorImpl->pJsonConverter,
								RSSL_JSON_CPC_EXPAND_ENUM_FIELDS, 
								&flag, &rjcError) != RSSL_RET_SUCCESS)
	{
		rsslSetErrorInfo(pError, RSSL_EIC_FAILURE, RSSL_RET_FAILURE, __FILE__, __LINE__, 
		"Failed setting RsslJsonConverter property: expand enum fields [%s]", rjcError.text);

		goto FailedToInitJsonConverter;
	}

	/* When converting from JSON to RWF, catch unknown JSON keys. */
	flag = pReactorJsonConverterOptions->catchUnknownJsonKeys;
	if (rsslJsonConverterSetProperty(pReactorImpl->pJsonConverter,
								RSSL_JSON_CPC_CATCH_UNKNOWN_JSON_KEYS, 
								&flag, &rjcError) != RSSL_RET_SUCCESS)
	{
		rsslSetErrorInfo(pError, RSSL_EIC_FAILURE, RSSL_RET_FAILURE, __FILE__, __LINE__, 
		"Failed setting RsslJsonConverter property: catch unknown JSON keys [%s]", rjcError.text);

		goto FailedToInitJsonConverter;
	}

	/* When converting from JSON to RWF, catch unknown JSON FIDS. */
	flag = pReactorJsonConverterOptions->catchUnknownJsonFids;
	if (rsslJsonConverterSetProperty(pReactorImpl->pJsonConverter,
								RSSL_JSON_CPC_CATCH_UNKNOWN_JSON_FIDS, 
								&flag, &rjcError) != RSSL_RET_SUCCESS)
	{
		rsslSetErrorInfo(pError, RSSL_EIC_FAILURE, RSSL_RET_FAILURE, __FILE__, __LINE__, 
		"Failed setting RsslJsonConverter property: catch unknown JSON fields [%s]",
		rjcError.text);

		goto FailedToInitJsonConverter;
	}

	/* Enumerated values in RWF are translated to display strings in simplified JSON. 
	 * However, conversion from display strings to RWF is not currently supported.  
	 * Setting the property below will cause display strings to be converted to blank, 
	 * instead of resulting in errors. */
	flag = RSSL_TRUE;
	if (rsslJsonConverterSetProperty(pReactorImpl->pJsonConverter,
								RSSL_JSON_CPC_ALLOW_ENUM_DISPLAY_STRINGS, 
								&flag, &rjcError) != RSSL_RET_SUCCESS)
	{
		rsslSetErrorInfo(pError, RSSL_EIC_FAILURE, RSSL_RET_FAILURE, __FILE__, __LINE__, 
		"Failed setting RsslJsonConverter property: blank on enum display error [%s]",
		rjcError.text);

		goto FailedToInitJsonConverter;
	}

	/* Checks whether the callback method is set by users to receive JSON error message */
	if (pReactorJsonConverterOptions->pJsonConversionEventCallback)
	{
		/* Allocates memory for holding JSON error messages when the conversion failed. */
		pReactorImpl->pJsonErrorInfo = (RsslErrorInfo*)malloc(sizeof(RsslErrorInfo));

		if (pReactorImpl->pJsonErrorInfo == NULL)
		{
			goto FailedToInitJsonConverter;
		}

		pReactorImpl->pJsonConversionEventCallback = pReactorJsonConverterOptions->pJsonConversionEventCallback;
	}

	pReactorImpl->closeChannelFromFailure = pReactorJsonConverterOptions->closeChannelFromFailure;
	
	pReactorImpl->jsonConverterInitialized = RSSL_TRUE;

	return (reactorUnlockInterface(pReactorImpl), RSSL_RET_SUCCESS);

FailedToInitJsonConverter:

	rsslJsonUninitialize();
	rsslDestroyRsslJsonConverter(pReactorImpl->pJsonConverter, &rjcError);
	free(pReactorImpl->pDictionaryList);

	return (reactorUnlockInterface(pReactorImpl), RSSL_RET_FAILURE);
}

RSSL_VA_API RsslReactor *rsslCreateReactor(RsslCreateReactorOptions *pReactorOpts, RsslErrorInfo *pError)
{
	RsslReactorImpl *pReactorImpl;
	RsslInt32 i;
	char *memBuf;

#ifdef WIN32
	LARGE_INTEGER	perfFrequency;
#endif

	/* Call rsslInitialize to ensure that Rssl is initialized with global & channel locks. It is reference counted per call to rsslInitialize/rsslUninitialize. */
	if (rsslInitialize(RSSL_LOCK_GLOBAL_AND_CHANNEL, &pError->rsslError) != RSSL_RET_SUCCESS)
	{
		rsslSetErrorInfoLocation(pError, __FILE__, __LINE__);
		return NULL;
	}

	if (pReactorOpts->tokenReissueRatio < 0.05 || pReactorOpts->tokenReissueRatio > 0.95 )
	{
		rsslSetErrorInfo(pError, RSSL_EIC_FAILURE, RSSL_RET_FAILURE, __FILE__, __LINE__, "The token reissue ratio must be in between 0.05 to 0.95.");
		return NULL;
	}

	if (pReactorOpts->reissueTokenAttemptInterval < 100 )
	{
		rsslSetErrorInfo(pError, RSSL_EIC_FAILURE, RSSL_RET_FAILURE, __FILE__, __LINE__, "The token reissue attempt interval must not be less than 100 milliseconds.");
		return NULL;
	}

	if (pReactorOpts->reissueTokenAttemptLimit < -1)
	{
		pReactorOpts->reissueTokenAttemptLimit = -1;
	}

	if (pReactorOpts->maxEventsInPool <= 0)
	{
		pReactorOpts->maxEventsInPool = -1;
	}

	if (pReactorOpts->cpuBindWorkerThread.length > 0 && pReactorOpts->cpuBindWorkerThread.data != NULL)
	{
		if (pReactorOpts->cpuBindWorkerThread.length < 512)
		{
			if (rsslIsStrProcessorCoreNumberValid(pReactorOpts->cpuBindWorkerThread.data) != RSSL_TRUE)
			{
				RsslUInt32 nProcessors = rsslGetNumberOfProcessorCore();
				rsslSetErrorInfo(pError, RSSL_EIC_FAILURE, RSSL_RET_FAILURE, __FILE__, __LINE__,
					"The required logical processor number is not valid. The number of logical processors: %u.",
					nProcessors);
				return NULL;
			}
		}
		else
		{
			rsslSetErrorInfo(pError, RSSL_EIC_FAILURE, RSSL_RET_FAILURE, __FILE__, __LINE__,
				"Passed in value cpuId is longer then allowed width 512 bytes (%u).",
				pReactorOpts->cpuBindWorkerThread.length);
			return NULL;
		}
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
	pReactorImpl->maxEventsInPool = pReactorOpts->maxEventsInPool;
	pReactorImpl->reactor.userSpecPtr = pReactorOpts->userSpecPtr;
	pReactorImpl->tokenReissueRatio = pReactorOpts->tokenReissueRatio;
	pReactorImpl->reissueTokenAttemptLimit = pReactorOpts->reissueTokenAttemptLimit;
	pReactorImpl->reissueTokenAttemptInterval = pReactorOpts->reissueTokenAttemptInterval;
	pReactorImpl->restRequestTimeout = pReactorOpts->restRequestTimeOut;
	pReactorImpl->restEnableLog = pReactorOpts->restEnableLog;
	if (pReactorOpts->restLogOutputStream)
	{
		if (pReactorOpts->restLogOutputStream == stdout)
		{
			if (fprintf(pReactorOpts->restLogOutputStream, "%s \n", "REST log redirected to stdout.") < 0)
			{
				_reactorWorkerCleanupReactor(pReactorImpl);
				rsslSetErrorInfo(pError, RSSL_EIC_FAILURE, RSSL_RET_FAILURE, __FILE__, __LINE__, "Failed to open stdout stream");
				return NULL;
			}
			pReactorImpl->restLogOutputStream = pReactorOpts->restLogOutputStream;
		}
		else if (pReactorOpts->restLogOutputStream == stderr)
		{
			if (fprintf(pReactorOpts->restLogOutputStream, "%s \n", "REST log redirected to stderr.") < 0)
			{
				_reactorWorkerCleanupReactor(pReactorImpl);
				rsslSetErrorInfo(pError, RSSL_EIC_FAILURE, RSSL_RET_FAILURE, __FILE__, __LINE__, "Failed to open stderr stream");
				return NULL;
			}
			pReactorImpl->restLogOutputStream = pReactorOpts->restLogOutputStream;
		}
		else
		{
			if (fprintf(pReactorOpts->restLogOutputStream, "%s \n", "REST log redirected to file.") < 0)
			{
				_reactorWorkerCleanupReactor(pReactorImpl);
				rsslSetErrorInfo(pError, RSSL_EIC_FAILURE, RSSL_RET_FAILURE, __FILE__, __LINE__, "Failed to open the specified file");
				return NULL;
			}
			pReactorImpl->restLogOutputStream = pReactorOpts->restLogOutputStream;
		}
	}
	if (pReactorOpts->pRestLoggingCallback)
	{
		pReactorImpl->restEnableLogCallback = RSSL_TRUE;
		pReactorImpl->pRestLoggingCallback = pReactorOpts->pRestLoggingCallback;
	}

	if (pReactorOpts->tokenServiceURL_V1.data && pReactorOpts->tokenServiceURL_V1.length)
	{
		pReactorImpl->tokenServiceURLBufferV1.length = RSSL_REACTOR_DEFAULT_URL_LENGHT;

		/* Ensure that the buffer length is sufficient to copy the URL */
		if (pReactorOpts->tokenServiceURL_V1.length > pReactorImpl->tokenServiceURLBufferV1.length)
		{
			pReactorImpl->tokenServiceURLBufferV1.length += pReactorOpts->tokenServiceURL_V1.length;
		}

		if (!(pReactorImpl->tokenServiceURLBufferV1.data = (char*)malloc(pReactorImpl->tokenServiceURLBufferV1.length)))
		{
			_reactorWorkerCleanupReactor(pReactorImpl);
			rsslSetErrorInfo(pError, RSSL_EIC_FAILURE, RSSL_RET_FAILURE, __FILE__, __LINE__, "Failed to allocate memory for the token service URL.");
			return NULL;
		}

		memset(pReactorImpl->tokenServiceURLBufferV1.data, 0, pReactorImpl->tokenServiceURLBufferV1.length);
		pReactorImpl->tokenServiceURLV1.data = pReactorImpl->tokenServiceURLBufferV1.data;
		pReactorImpl->tokenServiceURLV1.length = pReactorOpts->tokenServiceURL_V1.length;
		strncpy(pReactorImpl->tokenServiceURLV1.data, pReactorOpts->tokenServiceURL_V1.data, pReactorImpl->tokenServiceURLV1.length);
	}
	else if (pReactorOpts->tokenServiceURL.data && pReactorOpts->tokenServiceURL.length)
	{
		pReactorImpl->tokenServiceURLBufferV1.length = RSSL_REACTOR_DEFAULT_URL_LENGHT;

		/* Ensure that the buffer length is sufficient to copy the URL */
		if (pReactorOpts->tokenServiceURL.length > pReactorImpl->tokenServiceURLBufferV1.length)
		{
			pReactorImpl->tokenServiceURLBufferV1.length += pReactorOpts->tokenServiceURL.length;
		}

		if (!(pReactorImpl->tokenServiceURLBufferV1.data = (char*)malloc(pReactorImpl->tokenServiceURLBufferV1.length)))
		{
			_reactorWorkerCleanupReactor(pReactorImpl);
			rsslSetErrorInfo(pError, RSSL_EIC_FAILURE, RSSL_RET_FAILURE, __FILE__, __LINE__, "Failed to allocate memory for the token service URL.");
			return NULL;
		}

		memset(pReactorImpl->tokenServiceURLBufferV1.data, 0, pReactorImpl->tokenServiceURLBufferV1.length);
		pReactorImpl->tokenServiceURLV1.data = pReactorImpl->tokenServiceURLBufferV1.data;
		pReactorImpl->tokenServiceURLV1.length = pReactorOpts->tokenServiceURL.length;
		strncpy(pReactorImpl->tokenServiceURLV1.data, pReactorOpts->tokenServiceURL.data, pReactorImpl->tokenServiceURLV1.length);
	}
	else
	{
		pReactorImpl->tokenServiceURLBufferV1.length = rssl_rest_token_url_v1.length;

		if (!(pReactorImpl->tokenServiceURLBufferV1.data = (char*)malloc(pReactorImpl->tokenServiceURLBufferV1.length + 1)))
		{
			_reactorWorkerCleanupReactor(pReactorImpl);
			rsslSetErrorInfo(pError, RSSL_EIC_FAILURE, RSSL_RET_FAILURE, __FILE__, __LINE__, "Failed to allocate memory for the token service URL.");
			return NULL;
		}

		memset(pReactorImpl->tokenServiceURLBufferV1.data, 0, pReactorImpl->tokenServiceURLBufferV1.length+1);
		pReactorImpl->tokenServiceURLV1.data = pReactorImpl->tokenServiceURLBufferV1.data;
		pReactorImpl->tokenServiceURLV1.length = rssl_rest_token_url_v1.length;
		strncpy(pReactorImpl->tokenServiceURLV1.data, rssl_rest_token_url_v1.data, pReactorImpl->tokenServiceURLV1.length);
	}

	if (pReactorOpts->tokenServiceURL_V2.data && pReactorOpts->tokenServiceURL_V2.length)
	{
		pReactorImpl->tokenServiceURLBufferV2.length = RSSL_REACTOR_DEFAULT_URL_LENGHT;

		/* Ensure that the buffer length is sufficient to copy the URL */
		if (pReactorOpts->tokenServiceURL_V2.length > pReactorImpl->tokenServiceURLBufferV2.length)
		{
			pReactorImpl->tokenServiceURLBufferV2.length += pReactorOpts->tokenServiceURL_V2.length;
		}

		if (!(pReactorImpl->tokenServiceURLBufferV2.data = (char*)malloc(pReactorImpl->tokenServiceURLBufferV2.length)))
		{
			_reactorWorkerCleanupReactor(pReactorImpl);
			rsslSetErrorInfo(pError, RSSL_EIC_FAILURE, RSSL_RET_FAILURE, __FILE__, __LINE__, "Failed to allocate memory for the token service URL.");
			return NULL;
		}

		memset(pReactorImpl->tokenServiceURLBufferV2.data, 0, pReactorImpl->tokenServiceURLBufferV2.length);
		pReactorImpl->tokenServiceURLV2.data = pReactorImpl->tokenServiceURLBufferV2.data;
		pReactorImpl->tokenServiceURLV2.length = pReactorOpts->tokenServiceURL_V2.length;
		strncpy(pReactorImpl->tokenServiceURLV2.data, pReactorOpts->tokenServiceURL_V2.data, pReactorImpl->tokenServiceURLV2.length);
	}
	else
	{
		pReactorImpl->tokenServiceURLBufferV2.length = rssl_rest_token_url_v2.length;

		if (!(pReactorImpl->tokenServiceURLBufferV2.data = (char*)malloc(pReactorImpl->tokenServiceURLBufferV2.length + 1)))
		{
			_reactorWorkerCleanupReactor(pReactorImpl);
			rsslSetErrorInfo(pError, RSSL_EIC_FAILURE, RSSL_RET_FAILURE, __FILE__, __LINE__, "Failed to allocate memory for the token service URL.");
			return NULL;
		}

		memset(pReactorImpl->tokenServiceURLBufferV2.data, 0, pReactorImpl->tokenServiceURLBufferV2.length+1);
		pReactorImpl->tokenServiceURLV2.data = pReactorImpl->tokenServiceURLBufferV2.data;
		pReactorImpl->tokenServiceURLV2.length = rssl_rest_token_url_v2.length;
		strncpy(pReactorImpl->tokenServiceURLV2.data, rssl_rest_token_url_v2.data, pReactorImpl->tokenServiceURLV2.length);
	}

	if (pReactorOpts->serviceDiscoveryURL.data && pReactorOpts->serviceDiscoveryURL.length)
	{
		pReactorImpl->serviceDiscoveryURLBuffer.length = RSSL_REACTOR_DEFAULT_URL_LENGHT;

		/* Ensure that the buffer length is sufficient to copy the URL */
		if (pReactorOpts->serviceDiscoveryURL.length > pReactorImpl->serviceDiscoveryURLBuffer.length)
		{
			pReactorImpl->serviceDiscoveryURLBuffer.length += pReactorOpts->serviceDiscoveryURL.length;
		}

		if (!(pReactorImpl->serviceDiscoveryURLBuffer.data = (char*)malloc(pReactorImpl->serviceDiscoveryURLBuffer.length)))
		{
			_reactorWorkerCleanupReactor(pReactorImpl);
			rsslSetErrorInfo(pError, RSSL_EIC_FAILURE, RSSL_RET_FAILURE, __FILE__, __LINE__, "Failed to allocate memory for the service discovery URL.");
			return NULL;
		}

		memset(pReactorImpl->serviceDiscoveryURLBuffer.data, 0, pReactorImpl->serviceDiscoveryURLBuffer.length);
		pReactorImpl->serviceDiscoveryURL.data = pReactorImpl->serviceDiscoveryURLBuffer.data;
		pReactorImpl->serviceDiscoveryURL.length = pReactorOpts->serviceDiscoveryURL.length;
		strncpy(pReactorImpl->serviceDiscoveryURL.data, pReactorOpts->serviceDiscoveryURL.data, pReactorImpl->serviceDiscoveryURL.length);
	}
	else
	{
		_reactorWorkerCleanupReactor(pReactorImpl);
		rsslSetErrorInfo(pError, RSSL_EIC_FAILURE, RSSL_RET_FAILURE, __FILE__, __LINE__, "The service discovery URL is not available.");
		return NULL;
	}


	pReactorImpl->state = RSSL_REACTOR_ST_ACTIVE;


	/* Setup reactor */
	if (rsslInitReactorEventQueue(&pReactorImpl->reactorEventQueue, 10, &pReactorImpl->activeEventQueueGroup) != RSSL_RET_SUCCESS)
	{
		_reactorWorkerCleanupReactor(pReactorImpl);
		rsslSetErrorInfo(pError, RSSL_EIC_FAILURE, RSSL_RET_FAILURE, __FILE__, __LINE__, "Failed to initialize event queue.");
		return NULL;
	}

	if (rsslInitReactorEventQueueGroup(&pReactorImpl->activeEventQueueGroup) != RSSL_RET_SUCCESS)
	{
		_reactorWorkerCleanupReactor(pReactorImpl);
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
	rsslInitQueue(&pReactorImpl->warmstandbyChannelPool);
	rsslInitQueue(&pReactorImpl->closingWarmstandbyChannel);

	RSSL_MUTEX_INIT(&pReactorImpl->interfaceLock);
	RSSL_MUTEX_INIT(&pReactorImpl->debugLock);

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

	/* Initialize warmstandby channel pool */
	for (i = 0; i < 10; ++i)
	{
		RsslReactorWarmStandByHandlerImpl* pNewWSChannel = (RsslReactorWarmStandByHandlerImpl*)malloc(sizeof(RsslReactorWarmStandByHandlerImpl));

		if (!pNewWSChannel)
		{
			rsslSetErrorInfo(pError, RSSL_EIC_FAILURE, RSSL_RET_FAILURE, __FILE__, __LINE__, "Failed to create reactor warmstandby channel pool.");
			return NULL;
		}
		rsslClearReactorWarmStandByHandlerImpl(pNewWSChannel);
		pNewWSChannel->pReactorImpl = pReactorImpl;

		rsslQueueAddLinkToBack(&pReactorImpl->warmstandbyChannelPool, &pNewWSChannel->reactorQueueLink);
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

	if (rsslNotifierAddEvent(pReactorImpl->pNotifier, pReactorImpl->pQueueNotifierEvent, (int)(pReactorImpl->reactor.eventFd), &pReactorImpl->reactorEventQueue) < 0)
	{
		_reactorWorkerCleanupReactor(pReactorImpl);
		rsslSetErrorInfo(pError, RSSL_EIC_FAILURE, RSSL_RET_FAILURE, __FILE__, __LINE__, "Failed to add event for reactor event queue notification.");
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

	if (_reactorWorkerStart(pReactorImpl, pReactorOpts, RTR_ATOMIC_INCREMENT_RET(currentIndReactor), pError) != RSSL_RET_SUCCESS)
	{
		_reactorWorkerCleanupReactor(pReactorImpl);
		return NULL;
	}

	pReactorImpl->debugLevel = pReactorOpts->debugLevel;

	if (pReactorOpts->debugBufferSize < RSSL_REACTOR_DEBUGGING_BUFFER_MIN_INIT_SIZE)
	{
		_reactorWorkerCleanupReactor(pReactorImpl);
		rsslSetErrorInfo(pError, RSSL_EIC_FAILURE, RSSL_RET_FAILURE, __FILE__, __LINE__, "Set debugBufferSize: %d is to small. Min debug buffer size is %d", pReactorOpts->debugBufferSize, RSSL_REACTOR_DEBUGGING_BUFFER_MIN_INIT_SIZE);
		return NULL;
	}

	pReactorImpl->debugBufferSize = pReactorOpts->debugBufferSize;


	if (isReactorDebugEnabled(pReactorImpl))
	{
		if (_initReactorDebugInfo(pReactorImpl, pError) != RSSL_RET_SUCCESS)
		{
			_reactorWorkerCleanupReactor(pReactorImpl);
			return NULL;
		}
	}

	pReactorImpl->rsslWorkerStarted = RSSL_TRUE; /* Indicates the worker thread is started */

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

RSSL_VA_API RsslRet rsslReactorCreateRestClient(RsslReactorImpl *pRsslReactorImpl, RsslErrorInfo *pError)
{
	if (!(pRsslReactorImpl->pRestClient))
	{
		RsslError rsslError;
		RsslCreateRestClientOptions rssRestClientOpts;
		rsslClearRestClientOptions(&rssRestClientOpts);

		if (rsslRestClientInitialize(&rsslError) != RSSL_RET_SUCCESS)
		{
			rsslSetErrorInfo(pError, RSSL_EIC_FAILURE, RSSL_RET_FAILURE, __FILE__, __LINE__, "Failed to initialize RsslRestClient. Text: %s", rsslError.text);
			return RSSL_RET_FAILURE;
		}

		// Enable the RsslRestClient to reallocate memory buffer if the allocated size is not sufficient.
		rssRestClientOpts.dynamicBufferSize = RSSL_TRUE;
		pRsslReactorImpl->pRestClient = rsslCreateRestClient(&rssRestClientOpts, &rsslError);

		if (!pRsslReactorImpl->pRestClient)
		{
			rsslSetErrorInfo(pError, RSSL_EIC_FAILURE, RSSL_RET_FAILURE, __FILE__, __LINE__, "Failed to create RsslRestClient. Text: %s", rsslError.text);
			return RSSL_RET_FAILURE;
		}

		if ((pRsslReactorImpl->pWorkerNotifierEvent = rsslCreateNotifierEvent()) == NULL)
		{
			rsslSetErrorInfo(pError, RSSL_EIC_FAILURE, RSSL_RET_FAILURE, __FILE__, __LINE__, "Failed to create a RsslReactorWorker notifier event.");
			return RSSL_RET_FAILURE;
		}
	}

	return RSSL_RET_SUCCESS;
}

void _assignServiceDiscoveryOptionsToRequestArgs(RsslReactorServiceDiscoveryOptions* pOpts, RsslRestRequestArgs* pRestRequestArgs)
{
	if (pOpts->proxyDomain.data && *pOpts->proxyDomain.data != '\0')
	{
		pRestRequestArgs->networkArgs.proxyArgs.proxyDomain.data = pOpts->proxyDomain.data;
		pRestRequestArgs->networkArgs.proxyArgs.proxyDomain.length = pOpts->proxyDomain.length;
	}

	if (pOpts->proxyHostName.data && *pOpts->proxyHostName.data != '\0')
	{
		pRestRequestArgs->networkArgs.proxyArgs.proxyHostName.data = pOpts->proxyHostName.data;
		pRestRequestArgs->networkArgs.proxyArgs.proxyHostName.length = pOpts->proxyHostName.length;
	}

	if (pOpts->proxyPasswd.data && *pOpts->proxyPasswd.data != '\0')
	{
		pRestRequestArgs->networkArgs.proxyArgs.proxyPassword.data = pOpts->proxyPasswd.data;
		pRestRequestArgs->networkArgs.proxyArgs.proxyPassword.length = pOpts->proxyPasswd.length;
	}

	if (pOpts->proxyPort.data && *pOpts->proxyPort.data != '\0')
	{
		pRestRequestArgs->networkArgs.proxyArgs.proxyPort.data = pOpts->proxyPort.data;
		pRestRequestArgs->networkArgs.proxyArgs.proxyPort.length = pOpts->proxyPort.length;
	}

	if (pOpts->proxyUserName.data && *pOpts->proxyUserName.data != '\0')
	{
		pRestRequestArgs->networkArgs.proxyArgs.proxyUserName.data = pOpts->proxyUserName.data;
		pRestRequestArgs->networkArgs.proxyArgs.proxyUserName.length = pOpts->proxyUserName.length;
	}
}

RSSL_VA_API RsslRet rsslReactorQueryServiceDiscovery(RsslReactor *pReactor, RsslReactorServiceDiscoveryOptions* pOpts, RsslErrorInfo *pError)
{
	RsslRestRequestArgs *pRestRequestArgs = 0;
	RsslReactorImpl *pRsslReactorImpl;
	RsslRestResponse restResponse;
	RsslRet rsslRet;
	RsslQueueLink *pLink = 0;
	RsslTokenInformation tokenInformation;
	RsslReactorServiceEndpointEvent reactorServiceEndpointEvent;
	RsslErrorInfo errorInfo;
	RsslReactorTokenManagementImpl *pTokenManagementImpl = NULL;
	RsslReactorTokenSessionImpl *pTokenSessionImpl = NULL;
	RsslHashLink *pHashLink = NULL;
	int sessionVersion;
	RsslBuffer tokenURL;

	rsslClearReactorServiceEndpointEvent(&reactorServiceEndpointEvent);
	rsslClearTokenInformation(&tokenInformation);
	reactorServiceEndpointEvent.userSpecPtr = pOpts->userSpecPtr;

	pRsslReactorImpl = (RsslReactorImpl *)pReactor;

	if (!pReactor)
	{
		rsslSetErrorInfo(pError, RSSL_EIC_FAILURE, RSSL_RET_INVALID_ARGUMENT, __FILE__, __LINE__, "RsslReactor not provided.");
		return RSSL_RET_INVALID_ARGUMENT;
	}

	if ((rsslRet = reactorLockInterface(pRsslReactorImpl, RSSL_TRUE, pError)) != RSSL_RET_SUCCESS)
		return rsslRet;

	if (pRsslReactorImpl->state != RSSL_REACTOR_ST_ACTIVE)
	{
		rsslSetErrorInfo(pError, RSSL_EIC_SHUTDOWN, RSSL_RET_INVALID_ARGUMENT, __FILE__, __LINE__, "Reactor is shutting down.");
		return (reactorUnlockInterface(pRsslReactorImpl), RSSL_RET_INVALID_ARGUMENT);
	}

	if (!pOpts)
	{
		rsslSetErrorInfo(pError, RSSL_EIC_FAILURE, RSSL_RET_INVALID_ARGUMENT, __FILE__, __LINE__, "RsslReactorServiceDiscoveryOptions not provided.");
		return (reactorUnlockInterface(pRsslReactorImpl), RSSL_RET_INVALID_ARGUMENT);
	}

	if ((!pOpts->userName.data) || (!pOpts->userName.length) && (!pOpts->password.data) || (!pOpts->password.length) && (!pOpts->clientId.data) || (!pOpts->clientId.length))
	{
		if ((!pOpts->clientId.data) || (!pOpts->clientId.length) && (((!pOpts->clientSecret.data) || (!pOpts->clientSecret.length)) ))
		{
			rsslSetErrorInfo(pError, RSSL_EIC_FAILURE, RSSL_RET_INVALID_ARGUMENT, __FILE__, __LINE__, "RsslReactorServiceDiscoveryOptions credentials are not properly provided.");
			return (reactorUnlockInterface(pRsslReactorImpl), RSSL_RET_INVALID_ARGUMENT);
		}
		else
		{
			sessionVersion = RSSL_RC_SESSMGMT_V2;

			if (pRsslReactorImpl->tokenServiceURLV2.data && pRsslReactorImpl->tokenServiceURLV2.length)
			{
				tokenURL = pRsslReactorImpl->tokenServiceURLV2;
			}
			else
			{
				tokenURL = rssl_rest_token_url_v2;
			}
		}
	}
	else
	{
		sessionVersion = RSSL_RC_SESSMGMT_V1;

		if (pRsslReactorImpl->tokenServiceURLV1.data && pRsslReactorImpl->tokenServiceURLV1.length)
		{
			tokenURL = pRsslReactorImpl->tokenServiceURLV1;
		}
		else
		{
			tokenURL = rssl_rest_token_url_v1;
		}
	}	

	if (!pOpts->pServiceEndpointEventCallback)
	{
		rsslSetErrorInfo(pError, RSSL_EIC_FAILURE, RSSL_RET_INVALID_ARGUMENT, __FILE__, __LINE__, "RsslReactorServiceDiscoveryOptions.pServiceEndpointEventCallback not provided.");
		return (reactorUnlockInterface(pRsslReactorImpl), RSSL_RET_INVALID_ARGUMENT);
	}

	switch (pOpts->transport)
	{
	case RSSL_RD_TP_INIT:
	case RSSL_RD_TP_TCP:
	case RSSL_RD_TP_WEBSOCKET:
		break;
	default:
		rsslSetErrorInfo(pError, RSSL_EIC_FAILURE, RSSL_RET_INVALID_ARGUMENT, __FILE__, __LINE__, "Invalid transport protocol type %d.", pOpts->transport);
		return (reactorUnlockInterface(pRsslReactorImpl), RSSL_RET_INVALID_ARGUMENT);
	}

	switch (pOpts->dataFormat)
	{
	case RSSL_RD_DP_INIT:
	case RSSL_RD_DP_RWF:
	case RSSL_RD_DP_JSON2:
		break;
	default:
		rsslSetErrorInfo(pError, RSSL_EIC_FAILURE, RSSL_RET_INVALID_ARGUMENT, __FILE__, __LINE__, "Invalid dataformat protocol type %d.", pOpts->dataFormat);
		return (reactorUnlockInterface(pRsslReactorImpl), RSSL_RET_INVALID_ARGUMENT);
	}

	// Initialize and create RsslRestClient if does not exist
	if (rsslReactorCreateRestClient(pRsslReactorImpl, pError) != RSSL_RET_SUCCESS)
	{
		return (reactorUnlockInterface(pRsslReactorImpl), RSSL_RET_FAILURE);
	}

	if(sessionVersion == RSSL_RC_SESSMGMT_V1)
	{
		/* Checks whether there is an existing token session for the user. */
		pTokenManagementImpl = &pRsslReactorImpl->reactorWorker.reactorTokenManagement;

		RSSL_MUTEX_LOCK(&pTokenManagementImpl->tokenSessionMutex);

		if(pTokenManagementImpl->sessionByNameAndClientIdHt.elementCount != 0)
			pHashLink = rsslHashTableFind(&pTokenManagementImpl->sessionByNameAndClientIdHt, &pOpts->userName, NULL);

		if (pHashLink != 0)
		{
			RsslReactorOAuthCredential *pOAuthCredential;
			pTokenSessionImpl = RSSL_HASH_LINK_TO_OBJECT(RsslReactorTokenSessionImpl, hashLinkNameAndClientId, pHashLink);

			/* Checks whether the token session stops sending token reissue requests. */
			if (pTokenSessionImpl->stopTokenRequest == 0)
			{
				pOAuthCredential = pTokenSessionImpl->pOAuthCredential;

				if (pOAuthCredential->pOAuthCredentialEventCallback == 0)
				{
					if ((pOAuthCredential->clientSecret.length != pOpts->clientSecret.length) ||
						(memcmp(pOAuthCredential->clientSecret.data, pOpts->clientSecret.data, pOAuthCredential->clientSecret.length) != 0))
					{
						rsslSetErrorInfo(pError, RSSL_EIC_FAILURE, RSSL_RET_INVALID_ARGUMENT, __FILE__, __LINE__,
							"The Client secret of RsslReactorServiceDiscoveryOptions is not equal with the existing token session of the same user name.");
						return (RSSL_MUTEX_UNLOCK(&pTokenManagementImpl->tokenSessionMutex), reactorUnlockInterface(pRsslReactorImpl), RSSL_RET_INVALID_ARGUMENT);
					}

					if ((pOAuthCredential->password.length != pOpts->password.length) ||
						(memcmp(pOAuthCredential->password.data, pOpts->password.data, pOAuthCredential->password.length) != 0))
					{
						rsslSetErrorInfo(pError, RSSL_EIC_FAILURE, RSSL_RET_INVALID_ARGUMENT, __FILE__, __LINE__,
							"The password of RsslReactorServiceDiscoveryOptions is not equal with the existing token session of the same user name.");
						return (RSSL_MUTEX_UNLOCK(&pTokenManagementImpl->tokenSessionMutex), reactorUnlockInterface(pRsslReactorImpl), RSSL_RET_INVALID_ARGUMENT);
					}
				}

				if ((pOAuthCredential->clientId.length != pOpts->clientId.length) ||
					(memcmp(pOAuthCredential->clientId.data, pOpts->clientId.data, pOAuthCredential->clientId.length) != 0))
				{
					rsslSetErrorInfo(pError, RSSL_EIC_FAILURE, RSSL_RET_INVALID_ARGUMENT, __FILE__, __LINE__,
						"The Client ID of RsslReactorServiceDiscoveryOptions is not equal with the existing token session of the same user name.");
					return (RSSL_MUTEX_UNLOCK(&pTokenManagementImpl->tokenSessionMutex), reactorUnlockInterface(pRsslReactorImpl), RSSL_RET_INVALID_ARGUMENT);
				}

				RSSL_MUTEX_LOCK(&pTokenSessionImpl->accessTokenMutex);
				/* Uses the access token from the token session if it is valid */
				if (pTokenSessionImpl->tokenInformation.accessToken.data != 0)
				{
					tokenInformation.accessToken = pTokenSessionImpl->tokenInformation.accessToken;
					tokenInformation.tokenType = pTokenSessionImpl->tokenInformation.tokenType;
					RSSL_MUTEX_UNLOCK(&pTokenSessionImpl->accessTokenMutex);
				}
				else
				{
					RSSL_MUTEX_UNLOCK(&pTokenSessionImpl->accessTokenMutex);
					rsslSetErrorInfo(pError, RSSL_EIC_FAILURE, RSSL_RET_FAILURE, __FILE__, __LINE__,
						"Failed to retrieve token information from the existing token session of the same user name.");
					return (RSSL_MUTEX_UNLOCK(&pTokenManagementImpl->tokenSessionMutex), reactorUnlockInterface(pRsslReactorImpl), RSSL_RET_FAILURE);
				}
			}
			else
			{
				/* Send the token request by itself as the reactor worker stops updating the access token */
				pTokenSessionImpl = NULL;
			}
		}

		RSSL_MUTEX_UNLOCK(&pTokenManagementImpl->tokenSessionMutex);

		if (pTokenSessionImpl == NULL) /* Checks whether there is an existing token session for the same user */
		{
			pRestRequestArgs = _reactorCreateTokenRequestV1(pRsslReactorImpl, &tokenURL,
				&pOpts->userName, &pOpts->password, NULL, &pOpts->clientId, &pOpts->clientSecret, &pOpts->tokenScope, 
				pOpts->takeExclusiveSignOnControl, &pRsslReactorImpl->argumentsAndHeaders, NULL, pError);

			if (pRestRequestArgs)
			{
				if (!pRsslReactorImpl->accessTokenRespBuffer.data)
				{
					pRsslReactorImpl->accessTokenRespBuffer.length = RSSL_REST_INIT_TOKEN_BUFFER_SIZE;
					pRsslReactorImpl->accessTokenRespBuffer.data = (char*)malloc(pRsslReactorImpl->accessTokenRespBuffer.length);
				}

				if (!pRsslReactorImpl->accessTokenRespBuffer.data) goto memoryAllocationFailed;

				_assignServiceDiscoveryOptionsToRequestArgs(pOpts, pRestRequestArgs);

				rsslRestRequestDump(pRsslReactorImpl, pRestRequestArgs, &errorInfo.rsslError);

				rsslRet = rsslRestClientBlockingRequest(pRsslReactorImpl->pRestClient, pRestRequestArgs, &restResponse, &pRsslReactorImpl->accessTokenRespBuffer,
					&errorInfo.rsslError);

				rsslRestResponseDump(pRsslReactorImpl, &restResponse, &errorInfo.rsslError);

				free(pRestRequestArgs);

				if (restResponse.isMemReallocated)
				{
					free(pRsslReactorImpl->accessTokenRespBuffer.data);
					pRsslReactorImpl->accessTokenRespBuffer = restResponse.reallocatedMem;
				}

				if (rsslRet != RSSL_RET_SUCCESS)
				{
					rsslSetErrorInfo(pError, RSSL_EIC_FAILURE, RSSL_RET_FAILURE, __FILE__, __LINE__,
						"Failed to send a request to the token service. Text: %s", errorInfo.rsslError.text);

					return (reactorUnlockInterface(pRsslReactorImpl), RSSL_RET_FAILURE);
				}

				if (restResponse.statusCode != 200)
				{
					rsslSetErrorInfo(&errorInfo, RSSL_EIC_FAILURE, RSSL_RET_FAILURE, __FILE__, __LINE__,
						"Failed to get token information from the token service. Text: %s", restResponse.dataBody.data);

					reactorServiceEndpointEvent.pErrorInfo = &errorInfo;
					reactorServiceEndpointEvent.statusCode = restResponse.statusCode;
					(*pOpts->pServiceEndpointEventCallback)(pReactor, &reactorServiceEndpointEvent);

					return (reactorUnlockInterface(pRsslReactorImpl), RSSL_RET_SUCCESS);
				}
			}
			else
			{
				return (reactorUnlockInterface(pRsslReactorImpl), pError->rsslError.rsslErrorId);
			}

			if (pRsslReactorImpl->tokenInformationBuffer.length < restResponse.dataBody.length)
			{
				if (pRsslReactorImpl->tokenInformationBuffer.data)
				{
					free(pRsslReactorImpl->tokenInformationBuffer.data);
				}

				pRsslReactorImpl->tokenInformationBuffer.length = restResponse.dataBody.length;
				pRsslReactorImpl->tokenInformationBuffer.data = (char*)malloc(pRsslReactorImpl->tokenInformationBuffer.length);
			}

			if (pRsslReactorImpl->tokenInformationBuffer.data == 0) goto memoryAllocationFailed;

			if (rsslRestParseAccessTokenV1(&restResponse.dataBody, &tokenInformation.accessToken, &tokenInformation.refreshToken,
				&tokenInformation.expiresIn, &tokenInformation.tokenType, &tokenInformation.scope,
				&pRsslReactorImpl->tokenInformationBuffer, &errorInfo.rsslError) != RSSL_RET_SUCCESS)
			{
				rsslSetErrorInfo(&errorInfo, RSSL_EIC_FAILURE, RSSL_RET_FAILURE, __FILE__, __LINE__,
					"Failed to parse authentication token information. Text: %s", restResponse.dataBody.data);

				reactorServiceEndpointEvent.pErrorInfo = &errorInfo;
				reactorServiceEndpointEvent.statusCode = restResponse.statusCode;
				(*pOpts->pServiceEndpointEventCallback)(pReactor, &reactorServiceEndpointEvent);

				return (reactorUnlockInterface(pRsslReactorImpl), RSSL_RET_SUCCESS);
			}
		}
	}
	else
	{
		/* V2, always request a new token */
		pRestRequestArgs = _reactorCreateTokenRequestV2(pRsslReactorImpl, &tokenURL,
			& pOpts->clientId, &pOpts->clientSecret, &pOpts->tokenScope, &pRsslReactorImpl->argumentsAndHeaders, NULL, pError);

		if (pRestRequestArgs)
		{
			if (!pRsslReactorImpl->accessTokenRespBuffer.data)
			{
				pRsslReactorImpl->accessTokenRespBuffer.length = RSSL_REST_INIT_TOKEN_BUFFER_SIZE;
				pRsslReactorImpl->accessTokenRespBuffer.data = (char*)malloc(pRsslReactorImpl->accessTokenRespBuffer.length);
			}

			if (!pRsslReactorImpl->accessTokenRespBuffer.data) goto memoryAllocationFailed;

			_assignServiceDiscoveryOptionsToRequestArgs(pOpts, pRestRequestArgs);

			rsslRestRequestDump(pRsslReactorImpl, pRestRequestArgs, &errorInfo.rsslError);

			rsslRet = rsslRestClientBlockingRequest(pRsslReactorImpl->pRestClient, pRestRequestArgs, &restResponse, &pRsslReactorImpl->accessTokenRespBuffer,
				&errorInfo.rsslError);

			rsslRestResponseDump(pRsslReactorImpl, &restResponse, &errorInfo.rsslError);

			free(pRestRequestArgs);

			if (restResponse.isMemReallocated)
			{
				free(pRsslReactorImpl->accessTokenRespBuffer.data);
				pRsslReactorImpl->accessTokenRespBuffer = restResponse.reallocatedMem;
			}

			if (rsslRet != RSSL_RET_SUCCESS)
			{
				rsslSetErrorInfo(pError, RSSL_EIC_FAILURE, RSSL_RET_FAILURE, __FILE__, __LINE__,
					"Failed to send a request to the token service. Text: %s", errorInfo.rsslError.text);

				return (reactorUnlockInterface(pRsslReactorImpl), RSSL_RET_FAILURE);
			}

			if (restResponse.statusCode != 200)
			{
				rsslSetErrorInfo(&errorInfo, RSSL_EIC_FAILURE, RSSL_RET_FAILURE, __FILE__, __LINE__,
					"Failed to get token information from the token service. Text: %s", restResponse.dataBody.data);

				reactorServiceEndpointEvent.pErrorInfo = &errorInfo;
				reactorServiceEndpointEvent.statusCode = restResponse.statusCode;
				(*pOpts->pServiceEndpointEventCallback)(pReactor, &reactorServiceEndpointEvent);

				return (reactorUnlockInterface(pRsslReactorImpl), RSSL_RET_SUCCESS);
			}
		}
		else
		{
			return (reactorUnlockInterface(pRsslReactorImpl), pError->rsslError.rsslErrorId);
		}

		if (pRsslReactorImpl->tokenInformationBuffer.length < restResponse.dataBody.length)
		{
			if (pRsslReactorImpl->tokenInformationBuffer.data)
			{
				free(pRsslReactorImpl->tokenInformationBuffer.data);
			}

			pRsslReactorImpl->tokenInformationBuffer.length = restResponse.dataBody.length;
			pRsslReactorImpl->tokenInformationBuffer.data = (char*)malloc(pRsslReactorImpl->tokenInformationBuffer.length);
		}

		if (pRsslReactorImpl->tokenInformationBuffer.data == 0) goto memoryAllocationFailed;

		if (rsslRestParseAccessTokenV2(&restResponse.dataBody, &tokenInformation.accessToken, &tokenInformation.refreshToken,
			&tokenInformation.expiresIn, &tokenInformation.tokenType, &tokenInformation.scope,
			&pRsslReactorImpl->tokenInformationBuffer, &errorInfo.rsslError) != RSSL_RET_SUCCESS)
		{
			rsslSetErrorInfo(&errorInfo, RSSL_EIC_FAILURE, RSSL_RET_FAILURE, __FILE__, __LINE__,
				"Failed to parse authentication token information. Text: %s", restResponse.dataBody.data);

			reactorServiceEndpointEvent.pErrorInfo = &errorInfo;
			reactorServiceEndpointEvent.statusCode = restResponse.statusCode;
			(*pOpts->pServiceEndpointEventCallback)(pReactor, &reactorServiceEndpointEvent);

			return (reactorUnlockInterface(pRsslReactorImpl), RSSL_RET_SUCCESS);
		}
	}
	


	pRestRequestArgs = _reactorCreateRequestArgsForServiceDiscovery(pRsslReactorImpl, &pRsslReactorImpl->serviceDiscoveryURL,
		pOpts->transport, pOpts->dataFormat, &tokenInformation.tokenType,
		&tokenInformation.accessToken, &pRsslReactorImpl->argumentsAndHeaders, NULL, pError);

	if (pRestRequestArgs)
	{
		if (!pRsslReactorImpl->serviceDiscoveryRespBuffer.data)
		{
			pRsslReactorImpl->serviceDiscoveryRespBuffer.length = RSSL_REST_INIT_SVC_DIS_BUF_SIZE;
			pRsslReactorImpl->serviceDiscoveryRespBuffer.data = (char*)malloc(pRsslReactorImpl->serviceDiscoveryRespBuffer.length);
		}

		if (pRsslReactorImpl->serviceDiscoveryRespBuffer.data == 0)
		{
			goto memoryAllocationFailed;
		}

		_assignServiceDiscoveryOptionsToRequestArgs(pOpts, pRestRequestArgs);

		rsslRestRequestDump(pRsslReactorImpl, pRestRequestArgs, &errorInfo.rsslError);

		rsslRet = rsslRestClientBlockingRequest(pRsslReactorImpl->pRestClient, pRestRequestArgs, &restResponse, 
			&pRsslReactorImpl->serviceDiscoveryRespBuffer, &errorInfo.rsslError);

		rsslRestResponseDump(pRsslReactorImpl, &restResponse, &errorInfo.rsslError);

		if (restResponse.isMemReallocated)
		{
			free(pRsslReactorImpl->serviceDiscoveryRespBuffer.data);
			pRsslReactorImpl->serviceDiscoveryRespBuffer = restResponse.reallocatedMem;
		}

		if (rsslRet == RSSL_RET_SUCCESS)
		{
			/* Assigns the HTTP response status code to the service discovery event. */
			reactorServiceEndpointEvent.statusCode = restResponse.statusCode;

			if (restResponse.statusCode == 200)
			{
				RsslUInt32 idx;
				RsslUInt32 neededBufferSize = restResponse.dataBody.length * 2;

				do
				{
					if (pRsslReactorImpl->restServiceEndpointRespBuf.length)
					{
						// Checks whether the allocated memory is sufficient to parse the response
						if (pRsslReactorImpl->restServiceEndpointRespBuf.length < neededBufferSize)
						{
							free(pRsslReactorImpl->restServiceEndpointRespBuf.data);
							pRsslReactorImpl->restServiceEndpointRespBuf.length = neededBufferSize;
							pRsslReactorImpl->restServiceEndpointRespBuf.data = (char*)malloc(pRsslReactorImpl->restServiceEndpointRespBuf.length);

							if (pRsslReactorImpl->restServiceEndpointRespBuf.data == 0) goto memoryAllocationFailed;
						}
					}
					else
					{
						// There is no allocated memory to parse the response yet.
						pRsslReactorImpl->restServiceEndpointRespBuf.length = neededBufferSize;
						pRsslReactorImpl->restServiceEndpointRespBuf.data = (char*)malloc(pRsslReactorImpl->restServiceEndpointRespBuf.length);

						if (pRsslReactorImpl->restServiceEndpointRespBuf.data == 0) goto memoryAllocationFailed;
					}

					rsslRet = rsslRestParseServiceDiscoveryResp(&restResponse.dataBody, &pRsslReactorImpl->restServiceEndpointResp,
						&pRsslReactorImpl->restServiceEndpointRespBuf, &errorInfo.rsslError);

					neededBufferSize = pRsslReactorImpl->restServiceEndpointRespBuf.length * 2;

				} while (rsslRet == RSSL_RET_BUFFER_TOO_SMALL);

				if (rsslRet == RSSL_RET_SUCCESS)
				{
					reactorServiceEndpointEvent.serviceEndpointInfoCount = pRsslReactorImpl->restServiceEndpointResp.serviceEndpointInfoCount;
					reactorServiceEndpointEvent.serviceEndpointInfoList = (RsslReactorServiceEndpointInfo*)malloc(reactorServiceEndpointEvent.serviceEndpointInfoCount *
																			sizeof(RsslReactorServiceEndpointInfo));

					if (reactorServiceEndpointEvent.serviceEndpointInfoList == 0) goto memoryAllocationFailed;

					// Populate endpoint information from RsslRestServiceEndpointInfo to RsslReactorServiceEndpointInfo
					for (idx = 0; idx < reactorServiceEndpointEvent.serviceEndpointInfoCount; idx++)
					{
						reactorServiceEndpointEvent.serviceEndpointInfoList[idx].dataFormatCount = pRsslReactorImpl->restServiceEndpointResp.serviceEndpointInfoList[idx].dataFormatCount;
						reactorServiceEndpointEvent.serviceEndpointInfoList[idx].dataFormatList = pRsslReactorImpl->restServiceEndpointResp.serviceEndpointInfoList[idx].dataFormatList;
						reactorServiceEndpointEvent.serviceEndpointInfoList[idx].endPoint = pRsslReactorImpl->restServiceEndpointResp.serviceEndpointInfoList[idx].endPoint;
						reactorServiceEndpointEvent.serviceEndpointInfoList[idx].locationCount = pRsslReactorImpl->restServiceEndpointResp.serviceEndpointInfoList[idx].locationCount;
						reactorServiceEndpointEvent.serviceEndpointInfoList[idx].locationList = pRsslReactorImpl->restServiceEndpointResp.serviceEndpointInfoList[idx].locationList;
						reactorServiceEndpointEvent.serviceEndpointInfoList[idx].port = pRsslReactorImpl->restServiceEndpointResp.serviceEndpointInfoList[idx].port;
						reactorServiceEndpointEvent.serviceEndpointInfoList[idx].provider = pRsslReactorImpl->restServiceEndpointResp.serviceEndpointInfoList[idx].provider;
						reactorServiceEndpointEvent.serviceEndpointInfoList[idx].transport = pRsslReactorImpl->restServiceEndpointResp.serviceEndpointInfoList[idx].transport;
					}

					(*pOpts->pServiceEndpointEventCallback)(pReactor, &reactorServiceEndpointEvent);

					free(reactorServiceEndpointEvent.serviceEndpointInfoList);
				}
				else
				{
					rsslSetErrorInfo(&errorInfo, RSSL_EIC_FAILURE, RSSL_RET_FAILURE, __FILE__, __LINE__,
						"Failed to parse service endpoint information from the RDP service discovery.");

					reactorServiceEndpointEvent.pErrorInfo = &errorInfo;
					(*pOpts->pServiceEndpointEventCallback)(pReactor, &reactorServiceEndpointEvent);
				}
			}
			else
			{
				rsslSetErrorInfo(&errorInfo, RSSL_EIC_FAILURE, RSSL_RET_FAILURE, __FILE__, __LINE__,
					"Failed to query service endpoint information from the RDP service discovery. Text: %s", restResponse.dataBody.data);

				reactorServiceEndpointEvent.pErrorInfo = &errorInfo;
				(*pOpts->pServiceEndpointEventCallback)(pReactor, &reactorServiceEndpointEvent);
			}
		}
		else
		{
			rsslSetErrorInfo(pError, RSSL_EIC_FAILURE, RSSL_RET_FAILURE, __FILE__, __LINE__,
				"Failed to send a request to the RDP service discovery. Text: %s", errorInfo.rsslError.text);
		}

		free(pRestRequestArgs);

		return (reactorUnlockInterface(pRsslReactorImpl), rsslRet);
	}
	else
	{
		return (reactorUnlockInterface(pRsslReactorImpl), pError->rsslError.rsslErrorId);
	}

memoryAllocationFailed:

	rsslSetErrorInfo(pError, RSSL_EIC_FAILURE, RSSL_RET_FAILURE, __FILE__, __LINE__, "Failed to allocate memory for querying service discovery.");

	return (reactorUnlockInterface(pRsslReactorImpl), pError->rsslError.rsslErrorId);
}


static RsslRet applyConnectionOptions(RsslReactorChannelImpl* pReactorChannel, RsslReactorConnectOptions* pOpts, RsslErrorInfo* pError)
{
	pReactorChannel->connectionListIter = 0;
	pReactorChannel->initializationTimeout = pReactorChannel->connectionOptList[0].base.initializationTimeout;
	pReactorChannel->reactorChannel.pRsslChannel = NULL;
	pReactorChannel->reactorChannel.pRsslServer = NULL;
	pReactorChannel->reactorChannel.userSpecPtr = pOpts->rsslConnectOptions.userSpecPtr;
	pReactorChannel->reactorChannel.userSpecPtr = pReactorChannel->connectionOptList[0].base.rsslConnectOptions.userSpecPtr;

	pReactorChannel->readRet = 0;
	pReactorChannel->connectionDebugFlags = pOpts->connectionDebugFlags;

	/* Set reconnection info here, this should be zeroed out provider bound connections */
	pReactorChannel->reconnectAttemptLimit = pOpts->reconnectAttemptLimit;

	if (pOpts->reconnectMinDelay < 100)
	{
		pReactorChannel->reconnectMinDelay = 100;
	}
	else
	{
		pReactorChannel->reconnectMinDelay = pOpts->reconnectMinDelay;
	}

	if (pOpts->reconnectMinDelay > pOpts->reconnectMaxDelay)
	{
		pReactorChannel->reconnectMaxDelay = pReactorChannel->reconnectMinDelay;
	}
	else
	{
		pReactorChannel->reconnectMaxDelay = pOpts->reconnectMaxDelay;
	}

	pReactorChannel->reconnectAttemptCount = 0;
	pReactorChannel->lastReconnectAttemptMs = 0;

	return RSSL_RET_SUCCESS;
}


RSSL_VA_API RsslRet rsslReactorConnect(RsslReactor *pReactor, RsslReactorConnectOptions *pOpts, RsslReactorChannelRole *pRole, RsslErrorInfo *pError )
{
	RsslReactorImpl *pReactorImpl = (RsslReactorImpl*)pReactor;

	RsslWatchlist *pWatchlist = NULL;
	RsslChannel *pChannel;
	RsslReactorChannelImpl *pReactorChannel = NULL;
	RsslRet ret;
	RsslConnectOptions *pConnOptions = NULL;
	RsslBool queryConnectInfo = RSSL_FALSE;
	RsslReactorCallbackRet cret;
	RsslReactorAuthTokenEvent authTokenEvent;
	RsslReactorWarmStandByHandlerImpl* pWarmStandByHandlerImpl = NULL;
	RsslQueueLink* pWarmStandByLink = NULL;
	RsslBool supportSessionManagementForWSGroup = RSSL_FALSE;
	RsslReactorConnectInfoImpl *pReactorConnectInfoImpl = NULL;

	if ((ret = reactorLockInterface(pReactorImpl, RSSL_TRUE, pError)) != RSSL_RET_SUCCESS)
		return ret;

	if (pReactorImpl->state != RSSL_REACTOR_ST_ACTIVE)
	{
		rsslSetErrorInfo(pError, RSSL_EIC_SHUTDOWN, RSSL_RET_INVALID_ARGUMENT, __FILE__, __LINE__, "Reactor is shutting down.");
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

	if(_rsslChannelCopyConnectionList(pReactorChannel, pOpts, &pReactorChannel->supportSessionMgnt) != RSSL_RET_SUCCESS)
	{
		_reactorMoveChannel(&pReactorImpl->channelPool, pReactorChannel);

		rsslSetErrorInfo(pError, RSSL_EIC_FAILURE, RSSL_RET_FAILURE, __FILE__, __LINE__, "Unable to copy connection list.");
		return (reactorUnlockInterface(pReactorImpl), RSSL_RET_FAILURE);
	}

	/* Handles the warmstandby group list for the warmstandby feature. */
	if ( (ret = _rsslChannelCopyWarmStandByGroupList(pReactorImpl, &pReactorImpl->warmstandbyChannelPool, pOpts, &pWarmStandByHandlerImpl, &supportSessionManagementForWSGroup)) != RSSL_RET_SUCCESS)
	{
		_reactorMoveChannel(&pReactorImpl->channelPool, pReactorChannel);

		if (ret == RSSL_RET_INVALID_ARGUMENT)
		{
			rsslSetErrorInfo(pError, RSSL_EIC_FAILURE, RSSL_RET_FAILURE, __FILE__, __LINE__, "Invalid warm standby mode specified in a warm standby group.");
		}
		else
		{
			rsslSetErrorInfo(pError, RSSL_EIC_FAILURE, RSSL_RET_FAILURE, __FILE__, __LINE__, "Unable to copy warmstandby group list.");
		}

		return (reactorUnlockInterface(pReactorImpl), RSSL_RET_FAILURE);
	}

	if (pReactorChannel->supportSessionMgnt || supportSessionManagementForWSGroup)
	{
		if (pRole->base.roleType == RSSL_RC_RT_OMM_CONSUMER)
		{
			if (pRole->ommConsumerRole.pOAuthCredential == 0 && pRole->ommConsumerRole.pLoginRequest == 0)
			{
				rsslSetErrorInfo(pError, RSSL_EIC_FAILURE, RSSL_RET_INVALID_ARGUMENT, __FILE__, __LINE__, "There is no user credential available for enabling session management.");
				goto reactorConnectFail;
			}
		}
		else
		{
			rsslSetErrorInfo(pError, RSSL_EIC_FAILURE, RSSL_RET_INVALID_ARGUMENT, __FILE__, __LINE__, "The session management supports only on the RSSL_RC_RT_OMM_CONSUMER role type.");
			goto reactorConnectFail;
		}

		// Initialize and create RsslRestClient if does not exist
		if (rsslReactorCreateRestClient(pReactorImpl, pError) != RSSL_RET_SUCCESS)
		{
			goto reactorConnectFail;
		}
	}

	if (applyConnectionOptions(pReactorChannel, pOpts, pError) != RSSL_RET_SUCCESS)
	{
		goto reactorConnectFail;
	}

	pReactorConnectInfoImpl = &pReactorChannel->connectionOptList[0];
	pConnOptions = &pReactorConnectInfoImpl->base.rsslConnectOptions;

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
		watchlistCreateOpts.enableWarmStandby = pWarmStandByHandlerImpl != NULL ? RSSL_TRUE : RSSL_FALSE;
		watchlistCreateOpts.loginRequestCount = pReactorChannel->supportSessionMgnt ? pReactorChannel->connectionListCount : 1; /* Account from switching from WSB group to channel list. */
		pWatchlist = rsslWatchlistCreate(&watchlistCreateOpts, pError);
		if (!pWatchlist) goto reactorConnectFail;

		pReactorChannel->pWatchlist = pWatchlist;

		if (watchlistCreateOpts.enableWarmStandby)
		{
			RsslReactorChannelImpl* pNextReactorChannel = NULL;
			RsslUInt32 groupIndex = 0, index = 0;
			RsslWatchlist* pWatchlistNext = NULL;
			RsslBool supportSessionMgnt = RSSL_FALSE;
			RsslReactorWarmStandbyGroupImpl *pWarmStandByGroupImpl;
			RsslConnectionInfo* pConnectionInfo = NULL;

			pWarmStandByHandlerImpl->enableSessionMgnt = supportSessionManagementForWSGroup;
			pReactorChannel->pWarmStandByHandlerImpl = pWarmStandByHandlerImpl;

			RSSL_MUTEX_LOCK(&pWarmStandByHandlerImpl->warmStandByHandlerMutex);
			rsslQueueAddLinkToBack(&pWarmStandByHandlerImpl->rsslChannelQueue, &pReactorChannel->warmstandbyChannelLink);
			RSSL_MUTEX_UNLOCK(&pWarmStandByHandlerImpl->warmStandByHandlerMutex);

			pWarmStandByHandlerImpl->currentWSyGroupIndex = groupIndex;

			/* Gets the first warm standby group */
			pWarmStandByGroupImpl = &pWarmStandByHandlerImpl->warmStandbyGroupList[groupIndex];
			pConnectionInfo = &pWarmStandByGroupImpl->startingActiveServer.reactorConnectInfoImpl.base.rsslConnectOptions.connectionInfo;
			pConnOptions = &pWarmStandByGroupImpl->startingActiveServer.reactorConnectInfoImpl.base.rsslConnectOptions; /* Sets the connection option to the starting active server. */

			if ((pConnectionInfo->unified.address != NULL) && (pConnectionInfo->unified.serviceName != NULL) && (*pConnectionInfo->unified.address) != 0 &&
				(*pConnectionInfo->unified.serviceName) != 0)
			{
				pReactorChannel->isStartingServerConfig = RSSL_TRUE;
				pReactorConnectInfoImpl = &pWarmStandByGroupImpl->startingActiveServer.reactorConnectInfoImpl;
				pReactorChannel->initializationTimeout = pReactorConnectInfoImpl->base.initializationTimeout;
				pReactorChannel->reactorChannel.userSpecPtr = pReactorConnectInfoImpl->base.rsslConnectOptions.userSpecPtr;
			}
			else
			{
				if (pWarmStandByGroupImpl->startingActiveServer.reactorConnectInfoImpl.base.enableSessionManagement)
				{
					/* Able to get an endpoint from the service discovery later. */
					pReactorChannel->isStartingServerConfig = RSSL_TRUE;
					pReactorConnectInfoImpl = &pWarmStandByGroupImpl->startingActiveServer.reactorConnectInfoImpl;
					pReactorChannel->initializationTimeout = pReactorConnectInfoImpl->base.initializationTimeout;
					pReactorChannel->reactorChannel.userSpecPtr = pReactorConnectInfoImpl->base.rsslConnectOptions.userSpecPtr;
				}
				else
				{
					rsslSetErrorInfo(pError, RSSL_EIC_FAILURE, RSSL_RET_INVALID_ARGUMENT, __FILE__, __LINE__, "There is no valid connection information for a starting server of the warm standby feature.");
					goto reactorConnectFail;
				}
			}

			rsslClearReactorChannelImpl(pReactorImpl, &pWarmStandByHandlerImpl->mainReactorChannelImpl);
			pWarmStandByHandlerImpl->mainReactorChannelImpl.pWarmStandByHandlerImpl = pWarmStandByHandlerImpl;
			pWarmStandByHandlerImpl->warmStandByHandlerState = RSSL_RWSB_STATE_CONNECTING_TO_A_STARTING_SERVER;
			pWarmStandByHandlerImpl->warmStandbyGroupList[pWarmStandByHandlerImpl->currentWSyGroupIndex].currentStartingServerIndex = RSSL_REACTOR_WSB_STARTING_SERVER_INDEX;

			if (pReactorChannel->connectionListCount != 0 && pReactorChannel->connectionOptList != NULL)
			{
				pWarmStandByHandlerImpl->hasConnectionList = RSSL_TRUE;
			}

			pWarmStandByHandlerImpl->pStartingReactorChannel = pReactorChannel;
			pWarmStandByHandlerImpl->mainReactorChannelImpl.reactorChannel.reactorChannelType = RSSL_REACTOR_CHANNEL_TYPE_WARM_STANDBY;

			pWarmStandByHandlerImpl->wsbChannelInfoImpl.base.socketIdList = (RsslSocket*)malloc(pWarmStandByHandlerImpl->wsbChannelInfoImpl.maxNumberOfSocket * sizeof(RsslSocket));

			if (pWarmStandByHandlerImpl->wsbChannelInfoImpl.base.socketIdList == NULL)
			{
				rsslSetErrorInfo(pError, RSSL_EIC_FAILURE, RSSL_RET_FAILURE, __FILE__, __LINE__, "Unable to allocate a socket ID list for the warm standby feature.");
				goto reactorConnectFail;
			}

			memset(pWarmStandByHandlerImpl->wsbChannelInfoImpl.base.socketIdList, 0, pWarmStandByHandlerImpl->wsbChannelInfoImpl.maxNumberOfSocket * sizeof(RsslSocket));

			pWarmStandByHandlerImpl->wsbChannelInfoImpl.base.oldSocketIdList = (RsslSocket*)malloc(pWarmStandByHandlerImpl->wsbChannelInfoImpl.maxNumberOfSocket * sizeof(RsslSocket));

			if (pWarmStandByHandlerImpl->wsbChannelInfoImpl.base.oldSocketIdList == NULL)
			{
				rsslSetErrorInfo(pError, RSSL_EIC_FAILURE, RSSL_RET_FAILURE, __FILE__, __LINE__, "Unable to allocate a old socket ID list for the warm standby feature.");
				goto reactorConnectFail;
			}

			memset(pWarmStandByHandlerImpl->wsbChannelInfoImpl.base.oldSocketIdList, 0, pWarmStandByHandlerImpl->wsbChannelInfoImpl.maxNumberOfSocket * sizeof(RsslSocket));
		}
	}
	else
	{
		pWatchlist = NULL;

		if (pWarmStandByHandlerImpl)
		{
			rsslSetErrorInfo(pError, RSSL_EIC_FAILURE, RSSL_RET_INVALID_ARGUMENT, __FILE__, __LINE__, "The watchlist must be enabled in order to support the warm standby feature.");
			goto reactorConnectFail;
		}
	}

	if (_reactorChannelCopyRole(pReactorChannel, pRole, pError) != RSSL_RET_SUCCESS)
		goto reactorConnectFail;


	/* Checks whether the session management is enable and the host name and port is set if not specified for the encrypted connection. */
	if (pReactorConnectInfoImpl->base.enableSessionManagement)
	{
		RsslReactorTokenSessionImpl *pTokenSessionImpl = NULL;
		RsslReactorTokenManagementImpl *pTokenManagementImpl = &pReactorImpl->reactorWorker.reactorTokenManagement;

		if (_reactorGetAccessTokenAndServiceDiscovery(pReactorChannel, &queryConnectInfo, pError) != RSSL_RET_SUCCESS)
		{
			if(pReactorChannel->sessionVersion == RSSL_RC_SESSMGMT_V1)
				RSSL_MUTEX_UNLOCK(&pTokenManagementImpl->tokenSessionMutex);
			goto reactorConnectFail;
		}

		rsslClearBuffer(&pReactorChannel->pTokenSessionImpl->temporaryURL);
		if (pReactorChannel->sessionVersion == RSSL_RC_SESSMGMT_V1)
			RSSL_MUTEX_UNLOCK(&pTokenManagementImpl->tokenSessionMutex); /* Release the lock acquired by the _reactorChannelCopyRole() method */

		pTokenSessionImpl = pReactorChannel->pTokenSessionImpl;

		_reactorSetInCallback(pReactorImpl, RSSL_TRUE);
		if (pReactorConnectInfoImpl->base.pAuthTokenEventCallback)
		{
			rsslClearReactorAuthTokenEvent(&authTokenEvent);
			authTokenEvent.pReactorAuthTokenInfo = &pTokenSessionImpl->tokenInformation;
			authTokenEvent.pReactorChannel = (RsslReactorChannel*)pReactorChannel;
			authTokenEvent.pError = NULL;
			authTokenEvent.statusCode = pTokenSessionImpl->httpStausCode;
			cret = (*pReactorConnectInfoImpl->base.pAuthTokenEventCallback)((RsslReactor*)pReactorImpl,
				(RsslReactorChannel*)pReactorChannel, &authTokenEvent);
		}
		else
			cret = RSSL_RC_CRET_SUCCESS;
		_reactorSetInCallback(pReactorImpl, RSSL_FALSE);

		if (cret < RSSL_RC_CRET_SUCCESS || pReactorChannel->reactorParentQueue == &pReactorImpl->closingChannels)
			goto reactorConnectFail;
	}

	/* Keeps the original login request */
	if (pReactorChannel->channelRole.ommConsumerRole.pLoginRequest)
	{
		pReactorChannel->userName = pReactorChannel->channelRole.ommConsumerRole.pLoginRequest->userName;
		pReactorChannel->flags = pReactorChannel->channelRole.ommConsumerRole.pLoginRequest->flags;
		pReactorChannel->userNameType = pReactorChannel->channelRole.ommConsumerRole.pLoginRequest->userNameType;
	}

	if (isReactorDebugLevelEnabled(pReactorImpl, RSSL_RC_DEBUG_LEVEL_CONNECTION))
	{
		if (pReactorImpl->pReactorDebugInfo == NULL || pReactorChannel->pChannelDebugInfo == NULL)
		{
			if (_initReactorAndChannelDebugInfo(pReactorImpl, pReactorChannel, pError) != RSSL_RET_SUCCESS)
			{
				_reactorShutdown(pReactorImpl, pError);
				_reactorSendShutdownEvent(pReactorImpl, pError);
				return (reactorUnlockInterface(pReactorImpl), RSSL_RET_FAILURE);
			}
		}

		pReactorChannel->pChannelDebugInfo->debugInfoState |= RSSL_RC_DEBUG_INFO_CHANNEL_SESSION_STARTUP_DONE;

		_writeDebugInfo(pReactorImpl, "Reactor("RSSL_REACTOR_POINTER_PRINT_TYPE"), Reactor channel("RSSL_REACTOR_POINTER_PRINT_TYPE") connection EXECUTED on channel fd = "RSSL_REACTOR_SOCKET_PRINT_TYPE"..]\n", pReactorImpl, pReactorChannel, pReactorChannel->reactorChannel.socketId);
	}

	if ((pReactorChannel->pTunnelManager = tunnelManagerOpen((RsslReactor*)pReactorChannel->pParentReactor, (RsslReactorChannel*)pReactorChannel, pError)) == NULL)
		return RSSL_RET_FAILURE;

	if (pWatchlist)
	{
		RsslWatchlistProcessMsgOptions processOpts;

		pWatchlist->pUserSpec = pReactorChannel;
		
		/* Add consumer requests, if provided. */
		if (pReactorChannel->channelRole.ommConsumerRole.pLoginRequest)
		{
			rsslWatchlistClearProcessMsgOptions(&processOpts);
			processOpts.pChannel = pReactorChannel->reactorChannel.pRsslChannel;
			processOpts.pRdmMsg = (RsslRDMMsg*)pReactorChannel->channelRole.ommConsumerRole.pLoginRequest;

			/* Checks whether the downloadConnectionConfig option is enabled. */
			if (_reactorHandlesWarmStandby(pReactorChannel))
			{
				RsslReactorWarmStandbyGroupImpl *pWarmStandByGroupImpl = &pWarmStandByHandlerImpl->warmStandbyGroupList[pWarmStandByHandlerImpl->currentWSyGroupIndex];

				if (pWarmStandByGroupImpl->downloadConnectionConfig)
				{
					pReactorChannel->channelRole.ommConsumerRole.pLoginRequest->downloadConnectionConfig = RSSL_TRUE;
					pReactorChannel->channelRole.ommConsumerRole.pLoginRequest->flags |= RDM_LG_RQF_HAS_DOWNLOAD_CONN_CONFIG;
				}
			}

			if ((ret = _reactorSubmitWatchlistMsg(pReactorImpl, pReactorChannel, &processOpts, pError))
				< RSSL_RET_SUCCESS)
				return (reactorUnlockInterface(pReactorImpl), ret);
		}

		if (pReactorChannel->channelRole.ommConsumerRole.pDirectoryRequest)
		{
			rsslWatchlistClearProcessMsgOptions(&processOpts);
			processOpts.pChannel = pReactorChannel->reactorChannel.pRsslChannel;
			processOpts.pRdmMsg = (RsslRDMMsg*)pReactorChannel->channelRole.ommConsumerRole.pDirectoryRequest;

			if ((ret = _reactorSubmitWatchlistMsg(pReactorImpl, pReactorChannel, &processOpts, pError))
				< RSSL_RET_SUCCESS)
				return (reactorUnlockInterface(pReactorImpl), ret);
		}

		_reactorSetInCallback(pReactorImpl, RSSL_TRUE);
		if (pReactorChannel->channelRole.ommConsumerRole.watchlistOptions.channelOpenCallback)
		{
			RsslReactorEventImpl rsslEvent;
			RsslReactorChannel* pCallbackChannel = (RsslReactorChannel*)pReactorChannel;
				
			if (_reactorHandlesWarmStandby(pReactorChannel))
			{
				pCallbackChannel = &pReactorChannel->pWarmStandByHandlerImpl->mainReactorChannelImpl.reactorChannel;
				pCallbackChannel->pRsslChannel = pReactorChannel->reactorChannel.pRsslChannel;
				pCallbackChannel->oldSocketId = pReactorChannel->reactorChannel.oldSocketId;
				pCallbackChannel->majorVersion = pReactorChannel->reactorChannel.majorVersion;
				pCallbackChannel->minorVersion = pReactorChannel->reactorChannel.minorVersion;
				pCallbackChannel->protocolType = pReactorChannel->reactorChannel.protocolType;
				pCallbackChannel->userSpecPtr = pReactorChannel->reactorChannel.userSpecPtr;
			}

			rsslClearReactorChannelEventImpl(&rsslEvent.channelEventImpl);
			rsslEvent.channelEventImpl.channelEvent.channelEventType = RSSL_RC_CET_CHANNEL_OPENED;
			rsslEvent.channelEventImpl.channelEvent.pReactorChannel = (RsslReactorChannel*)pCallbackChannel;
			rsslEvent.channelEventImpl.channelEvent.pError = NULL;

			cret = (*pReactorChannel->channelRole.ommConsumerRole.watchlistOptions.channelOpenCallback)((RsslReactor*)pReactorImpl, pCallbackChannel, &rsslEvent.channelEventImpl.channelEvent);
		}
		else
			cret = RSSL_RC_CRET_SUCCESS;
		_reactorSetInCallback(pReactorImpl, RSSL_FALSE);

		if (cret < RSSL_RC_CRET_SUCCESS || pReactorChannel->reactorParentQueue == &pReactorImpl->closingChannels)
			goto reactorConnectFail;
	}

	if (queryConnectInfo || !(pChannel = rsslConnect(pConnOptions, &pError->rsslError)))
	{

		if (isReactorDebugLevelEnabled(pReactorImpl, RSSL_RC_DEBUG_LEVEL_CONNECTION))
		{
			if (pReactorImpl->pReactorDebugInfo == NULL || pReactorChannel->pChannelDebugInfo == NULL)
			{
				if (_initReactorAndChannelDebugInfo(pReactorImpl, pReactorChannel, pError) != RSSL_RET_SUCCESS)
				{
					_reactorShutdown(pReactorImpl, pError);
					_reactorSendShutdownEvent(pReactorImpl, pError);
					return (reactorUnlockInterface(pReactorImpl), RSSL_RET_FAILURE);
				}
			}

			pReactorChannel->pChannelDebugInfo->debugInfoState &= (~RSSL_RC_DEBUG_INFO_CHANNEL_UP);
			pReactorChannel->pChannelDebugInfo->debugInfoState |= RSSL_RC_DEBUG_INFO_CHANNEL_DOWN;

			_writeDebugInfo(pReactorImpl, "Reactor("RSSL_REACTOR_POINTER_PRINT_TYPE"), Reactor channel("RSSL_REACTOR_POINTER_PRINT_TYPE") is DOWN on channel fd="RSSL_REACTOR_SOCKET_PRINT_TYPE".]\n", pReactorImpl, pReactorChannel, pReactorChannel->reactorChannel.socketId);
		}

		if (pOpts->reconnectAttemptLimit != 0)
		{
			if (_reactorHandleChannelDown(pReactorImpl, pReactorChannel, pError) != RSSL_RET_SUCCESS)
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

		if (isReactorDebugLevelEnabled(pReactorImpl, RSSL_RC_DEBUG_LEVEL_CONNECTION))
		{
			if (pReactorImpl->pReactorDebugInfo == NULL || pReactorChannel->pChannelDebugInfo == NULL)
			{
				if (_initReactorAndChannelDebugInfo(pReactorImpl, pReactorChannel, pError) != RSSL_RET_SUCCESS)
				{
					_reactorShutdown(pReactorImpl, pError);
					_reactorSendShutdownEvent(pReactorImpl, pError);
					return (reactorUnlockInterface(pReactorImpl), RSSL_RET_FAILURE);
				}
			}

			pReactorChannel->pChannelDebugInfo->debugInfoState |= RSSL_RC_DEBUG_INFO_CHANNEL_CONNECTING_PERFORMED;

			_writeDebugInfo(pReactorImpl, "Reactor("RSSL_REACTOR_POINTER_PRINT_TYPE"), Reactor channel("RSSL_REACTOR_POINTER_PRINT_TYPE") is connection PERFORMED on channel fd="RSSL_REACTOR_SOCKET_PRINT_TYPE".]\n", pReactorImpl, pReactorChannel, pReactorChannel->reactorChannel.socketId);
		}

		if (!RSSL_ERROR_INFO_CHECK((ret = _reactorAddChannel(pReactorImpl, pReactorChannel, pError)) == RSSL_RET_SUCCESS, ret, pError))
		{
			_reactorShutdown(pReactorImpl, pError);
			_reactorSendShutdownEvent(pReactorImpl, pError);
			goto reactorConnectFail;
		}
		/* Set debug callback usage here. */

		if (pReactorChannel->connectionDebugFlags != 0 && !RSSL_ERROR_INFO_CHECK((ret = rsslIoctl(pReactorChannel->reactorChannel.pRsslChannel, RSSL_DEBUG_FLAGS, (void*)&(pReactorChannel->connectionDebugFlags), &(pError->rsslError))) == RSSL_RET_SUCCESS, ret, pError))
		{
			_reactorShutdown(pReactorImpl, pError);
			_reactorSendShutdownEvent(pReactorImpl, pError);
			goto reactorConnectFail;
		}
	}

	++pReactorImpl->channelCount;

	if (pReactorChannel->pTokenSessionImpl)
	{
		pReactorChannel->pTokenSessionImpl->initialized = RSSL_TRUE;
	}

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

		if (pReactorChannel->pTokenSessionImpl)
		{
			RsslReactorTokenSessionEvent *pEvent = NULL;
			rsslClearBuffer(&pReactorChannel->pTokenSessionImpl->temporaryURL);

			/* Ensure that the token session hasn't been initialized before removing it from the hash table */
			if (pReactorChannel->sessionVersion == RSSL_RC_SESSMGMT_V1 && pReactorChannel->pTokenSessionImpl->initialized == RSSL_FALSE)
			{
				RsslReactorTokenManagementImpl *pTokenManagementImpl = &pReactorImpl->reactorWorker.reactorTokenManagement;

				RSSL_MUTEX_LOCK(&pTokenManagementImpl->tokenSessionMutex);
				rsslHashTableRemoveLink(&pReactorImpl->reactorWorker.reactorTokenManagement.sessionByNameAndClientIdHt, &pReactorChannel->pTokenSessionImpl->hashLinkNameAndClientId);
				RSSL_MUTEX_UNLOCK(&pTokenManagementImpl->tokenSessionMutex);
			}

			pEvent = (RsslReactorTokenSessionEvent*)rsslReactorEventQueueGetFromPool(&pReactorImpl->reactorWorker.workerQueue);
			rsslClearReactorTokenSessionEvent(pEvent);

			pEvent->reactorTokenSessionEventType = RSSL_RCIMPL_TSET_UNREGISTER_CHANNEL_FROM_SESSION;
			pEvent->pReactorChannel = (RsslReactorChannel*)pReactorChannel;
			pEvent->pTokenSessionImpl = pReactorChannel->pTokenSessionImpl;

			if (!RSSL_ERROR_INFO_CHECK(rsslReactorEventQueuePut(&pReactorImpl->reactorWorker.workerQueue, (RsslReactorEventImpl*)pEvent) == RSSL_RET_SUCCESS, RSSL_RET_FAILURE, pError))
			{
				_reactorShutdown(pReactorImpl, pError);
				_reactorSendShutdownEvent(pReactorImpl, pError);
				reactorUnlockInterface(pReactorImpl);
				return RSSL_RET_FAILURE;
			}
		}
		else
		{
			_reactorMoveChannel(&pReactorImpl->channelPool, pReactorChannel);
			--pReactorImpl->channelCount;
		}
	}

	/* Cleaning up the warm standby feature if it is enabled. */
	if (pWarmStandByHandlerImpl)
	{
		_rsslFreeWarmStandbyHandler(pWarmStandByHandlerImpl, pWarmStandByHandlerImpl->warmStandbyGroupCount, RSSL_TRUE);
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
		rsslSetErrorInfo(pError, RSSL_EIC_SHUTDOWN, RSSL_RET_INVALID_ARGUMENT, __FILE__, __LINE__, "Reactor is shutting down.");
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

	/* Checks whether per reactor debuging is enabled. */
	if (isReactorDebugEnabled(pReactorImpl))
	{
		if (_initReactorChannelDebugInfo(pReactorImpl, pReactorChannel, pError) != RSSL_RET_SUCCESS)
		{
			_reactorShutdown(pReactorImpl, pError);
			_reactorSendShutdownEvent(pReactorImpl, pError);
			return (reactorUnlockInterface(pReactorImpl), RSSL_RET_FAILURE);
		}
	}

	if (_reactorChannelCopyRole(pReactorChannel, pRole, pError) != RSSL_RET_SUCCESS)
	{
		rsslCloseChannel(pChannel, &pError->rsslError);

		_cleanupReactorChannelDebugInfo(pReactorChannel);

		_reactorMoveChannel(&pReactorImpl->channelPool, pReactorChannel);
		return (reactorUnlockInterface(pReactorImpl), RSSL_RET_FAILURE);
	}

	pReactorChannel->reactorChannel.pRsslChannel = pChannel;
	pReactorChannel->reactorChannel.pRsslServer = pServer;
	pReactorChannel->reactorChannel.userSpecPtr = pOpts->rsslAcceptOptions.userSpecPtr;
	pReactorChannel->initializationTimeout = pOpts->initializationTimeout;
	pReactorChannel->connectionDebugFlags = pOpts->connectionDebugFlags;

	if ((pReactorChannel->pTunnelManager = tunnelManagerOpen(pReactor, (RsslReactorChannel*)pReactorChannel, pError)) == NULL)
	{
		rsslCloseChannel(pChannel, &pError->rsslError);

		_cleanupReactorChannelDebugInfo(pReactorChannel);

		return (reactorUnlockInterface(pReactorImpl), RSSL_RET_FAILURE);
	}

	if (pReactorChannel->connectionDebugFlags != 0 && !RSSL_ERROR_INFO_CHECK((ret = rsslIoctl(pReactorChannel->reactorChannel.pRsslChannel, RSSL_DEBUG_FLAGS, (void*)&(pReactorChannel->connectionDebugFlags), &(pError->rsslError))) == RSSL_RET_SUCCESS, ret, pError))
	{
		rsslCloseChannel(pChannel, &pError->rsslError);

		_cleanupReactorChannelDebugInfo(pReactorChannel);

		return (reactorUnlockInterface(pReactorImpl), RSSL_RET_FAILURE);
	}

	if (!RSSL_ERROR_INFO_CHECK((ret = _reactorAddChannel(pReactorImpl, pReactorChannel, pError)) == RSSL_RET_SUCCESS, ret, pError))
	{
		_cleanupReactorChannelDebugInfo(pReactorChannel);
		_reactorShutdown(pReactorImpl, pError);
		_reactorSendShutdownEvent(pReactorImpl, pError);
		return (reactorUnlockInterface(pReactorImpl), RSSL_RET_FAILURE);
	}

	/* Set additional accept options for WebSocket connections */
	if (pOpts->wsocketAcceptOptions.sendPingMessage)
	{
		pReactorChannel->sendWSPingMessage = RSSL_TRUE;
	}

	++pReactorImpl->channelCount;

	if (isReactorDebugLevelEnabled(pReactorImpl, RSSL_RC_DEBUG_LEVEL_CONNECTION))
	{
		if (pReactorImpl->pReactorDebugInfo == NULL || pReactorChannel->pChannelDebugInfo == NULL)
		{
			if (_initReactorAndChannelDebugInfo(pReactorImpl, pReactorChannel, pError) != RSSL_RET_SUCCESS)
			{
				_reactorShutdown(pReactorImpl, pError);
				_reactorSendShutdownEvent(pReactorImpl, pError);
				return (reactorUnlockInterface(pReactorImpl), RSSL_RET_FAILURE);
			}
		}

		pReactorChannel->pChannelDebugInfo->debugInfoState |= RSSL_RC_DEBUG_INFO_ACCEPT_CHANNEL;

		_writeDebugInfo(pReactorImpl, "Reactor("RSSL_REACTOR_POINTER_PRINT_TYPE"), RsslServer("RSSL_REACTOR_POINTER_PRINT_TYPE") ACCEPT reactor channel("RSSL_REACTOR_POINTER_PRINT_TYPE") on channel fd="RSSL_REACTOR_SOCKET_PRINT_TYPE".]\n", pReactor, pServer, pReactorChannel, pChannel->socketId);
	}

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

	if (pReactorImpl->state != RSSL_REACTOR_ST_ACTIVE)
	{
		rsslSetErrorInfo(pError, RSSL_EIC_SHUTDOWN, RSSL_RET_FAILURE, __FILE__, __LINE__, "Reactor is shutting down.");
		return (reactorUnlockInterface(pReactorImpl), RSSL_RET_FAILURE);
	}

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
		if (!pDispatchOpts->pReactorChannel || _reactorHandlesWarmStandby((RsslReactorChannelImpl*)pDispatchOpts->pReactorChannel))
		{
			RsslReactorChannelImpl *pReactorChannel;

			if (rsslNotifierEventIsReadable(pReactorImpl->pQueueNotifierEvent))
			{
				RsslReactorEventQueue *pQueue;

				if (isReactorDebugLevelEnabled(pReactorImpl, RSSL_RC_DEBUG_LEVEL_EVENTENQUE) && (pReactorImpl->activeEventQueueGroup.readyEventQueueGroup.count > 0))
				{
					if (pReactorImpl->pReactorDebugInfo == NULL)
					{
						if (_initReactorDebugInfo(pReactorImpl, pError) != RSSL_RET_SUCCESS)
						{
							_reactorShutdown(pReactorImpl, pError);
							_reactorSendShutdownEvent(pReactorImpl, pError);
							return (reactorUnlockInterface(pReactorImpl), RSSL_RET_FAILURE);
						}
					}

					_writeDebugInfo(pReactorImpl, "Reactor("RSSL_REACTOR_POINTER_PRINT_TYPE"), Ready event queue GROUP COUNT (%lu) to be dispatched from active event queue group.]\n",
						pReactor, pReactorImpl->activeEventQueueGroup.readyEventQueueGroup.count);
				}

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

			if (isReactorDebugLevelEnabled(pReactorImpl, RSSL_RC_DEBUG_LEVEL_EVENTENQUE) && pReactorChannel->pChannelDebugInfo != NULL)
			{
				pReactorChannel->pChannelDebugInfo->numOfDispatchCall++;
			}

			if (rsslNotifierEventIsReadable(pReactorImpl->pQueueNotifierEvent))
			{
				if (isReactorDebugLevelEnabled(pReactorImpl, RSSL_RC_DEBUG_LEVEL_EVENTENQUE) && (pReactorImpl->reactorEventQueue.eventQueue.count > 0))
				{
					if (pReactorImpl->pReactorDebugInfo == NULL)
					{
						if (_initReactorDebugInfo(pReactorImpl, pError) != RSSL_RET_SUCCESS)
						{
							_reactorShutdown(pReactorImpl, pError);
							_reactorSendShutdownEvent(pReactorImpl, pError);
							return (reactorUnlockInterface(pReactorImpl), RSSL_RET_FAILURE);
						}
					}

					_writeDebugInfo(pReactorImpl, "Reactor("RSSL_REACTOR_POINTER_PRINT_TYPE") event queue count %lu to be dispatched.]\n", pReactor,
						pReactorImpl->reactorEventQueue.eventQueue.count);
				}

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

				if (isReactorDebugLevelEnabled(pReactorImpl, RSSL_RC_DEBUG_LEVEL_EVENTENQUE) && (pReactorChannel->eventQueue.eventQueue.count > 0))
				{
					if (pReactorImpl->pReactorDebugInfo == NULL || pReactorChannel->pChannelDebugInfo == NULL)
					{
						if (_initReactorAndChannelDebugInfo(pReactorImpl, pReactorChannel, pError) != RSSL_RET_SUCCESS)
						{
							_reactorShutdown(pReactorImpl, pError);
							_reactorSendShutdownEvent(pReactorImpl, pError);
							return (reactorUnlockInterface(pReactorImpl), RSSL_RET_FAILURE);
						}
					}

					_writeDebugInfo(pReactorImpl, "Reactor("RSSL_REACTOR_POINTER_PRINT_TYPE"), Per Reactor channel("RSSL_REACTOR_POINTER_PRINT_TYPE") event queue count (%lu) to be dispatched on channel fd="RSSL_REACTOR_SOCKET_PRINT_TYPE".]\n",
						pReactor, pReactorChannel, pReactorChannel->eventQueue.eventQueue.count, pReactorChannel->reactorChannel.pRsslChannel->socketId);
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


			/* A channel has something to read if:
			 * - The channel is in the active state(it was not closed above)
			 * - The last return from rsslRead() was greater than zero, indicating there were still bytes in RSSL's queue
			 * - The file descriptor is set because there is data from the socket */
			if (pReactorChannel->reactorParentQueue == &pReactorImpl->activeChannels)
			{
				if (pReactorChannel->readRet > 0 || rsslNotifierEventIsReadable(pReactorChannel->pNotifierEvent))
				{
					channelsToCheck = 1;
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
						else if (pReactorChannel->reactorParentQueue != &pReactorImpl->activeChannels)
						{
							/* Channel is no longer active, so break out of the loop and don't dispatch anything more */
							channelsToCheck = 0;
							break;
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
			}
			else
				channelsToCheck = 0;

			return (reactorUnlockInterface(pReactorImpl), channelsToCheck);
		}
		
	}
	else
	{
		rsslSetErrorInfo(pError, RSSL_EIC_SHUTDOWN, RSSL_RET_FAILURE, __FILE__, __LINE__, "Reactor is shutting down.");
		return (reactorUnlockInterface(pReactorImpl), RSSL_RET_FAILURE);
	}
}

RSSL_VA_API RsslRet rsslReactorSubmit(RsslReactor *pReactor, RsslReactorChannel *pChannel, RsslBuffer *buffer, RsslReactorSubmitOptions *pSubmitOptions, RsslErrorInfo *pError)
{
	RsslRet ret;
	RsslReactorImpl *pReactorImpl = (RsslReactorImpl*)pReactor;
	RsslReactorChannelImpl *pReactorChannel = (RsslReactorChannelImpl*)pChannel;
	RsslUInt32 dummyBytesWritten, dummyUncompBytesWritten;
	RsslBuffer *pMsgBuffer = NULL; /* The buffer to send JSON message to network only. */
	RsslBool releaseUserBuffer = RSSL_FALSE; /* Release when it writes user's buffer successfully. */
	RsslReactorPackedBufferImpl *pPackedBufferImpl = NULL;
	RsslHashLink *pHashLink;

	if ((ret = reactorLockInterface(pReactorImpl, RSSL_TRUE, pError)) != RSSL_RET_SUCCESS)
		return ret;

	/* Since the application passed in this channel, make sure it is valid for this reactor and that it is active. */
	if (!pReactorChannel || !rsslReactorChannelIsValid(pReactorImpl, pReactorChannel, pError))
		return (reactorUnlockInterface(pReactorImpl), RSSL_RET_INVALID_ARGUMENT);

	if (pReactorImpl->state != RSSL_REACTOR_ST_ACTIVE)
	{
		rsslSetErrorInfo(pError, RSSL_EIC_SHUTDOWN, RSSL_RET_INVALID_ARGUMENT, __FILE__, __LINE__, "Reactor is shutting down.");
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

	/* Checks if there is a packed buffer for the JSON protocol */
	if (pReactorChannel->packedBufferHashTable.elementCount != 0)
	{
		pHashLink = rsslHashTableFind(&pReactorChannel->packedBufferHashTable, (void*)buffer, NULL);

		if (pHashLink != NULL)
		{
			pPackedBufferImpl = RSSL_HASH_LINK_TO_OBJECT(RsslReactorPackedBufferImpl, hashLink, pHashLink);
			buffer->length = 0;
		}
	}
	
	if ( (pReactorChannel->reactorChannel.pRsslChannel->protocolType == RSSL_JSON_PROTOCOL_TYPE) && (pPackedBufferImpl == NULL) )
	{
		RsslDecodeIterator dIter;
		RsslMsg rsslMsg;

		if (pReactorChannel->pWriteCallAgainUserBuffer == NULL)
		{
			/* Added checking to ensure that the JSON converter is initialized properly.*/
			if (pReactorImpl->pJsonConverter == 0)
			{
				rsslSetErrorInfo(pError, RSSL_EIC_FAILURE, RSSL_RET_FAILURE, __FILE__, __LINE__, "The JSON converter library has not been initialized properly.");
				return (reactorUnlockInterface((RsslReactorImpl*)pReactor), RSSL_RET_FAILURE);
			}

			rsslClearMsg(&rsslMsg);
			rsslClearDecodeIterator(&dIter);
			rsslSetDecodeIteratorRWFVersion(&dIter, pReactorChannel->reactorChannel.pRsslChannel->majorVersion,
				pReactorChannel->reactorChannel.pRsslChannel->minorVersion);

			rsslSetDecodeIteratorBuffer(&dIter, buffer);

			ret = rsslDecodeMsg(&dIter, &rsslMsg);

			if (ret == RSSL_RET_SUCCESS)
			{
				RsslConvertRsslMsgToJsonOptions rjcOptions;
				RsslGetJsonMsgOptions getJsonMsgOptions;
				RsslJsonConverterError rjcError;
				RsslBuffer jsonBuffer;

				rsslClearConvertRsslMsgToJsonOptions(&rjcOptions);
				rjcOptions.jsonProtocolType = RSSL_JSON_JPT_JSON2; /* Supported only for Simplified JSON */
				if ((rsslConvertRsslMsgToJson(pReactorImpl->pJsonConverter, &rjcOptions, &rsslMsg, &rjcError)) != RSSL_RET_SUCCESS)
				{
					rsslSetErrorInfo(pError, RSSL_EIC_FAILURE, RSSL_RET_FAILURE, __FILE__, __LINE__,
						"Failed to convert RWF to JSON protocol. Error text: %s", rjcError.text);
					return (reactorUnlockInterface((RsslReactorImpl*)pReactor), RSSL_RET_FAILURE);
				}

				rsslClearGetJsonMsgOptions(&getJsonMsgOptions);
				getJsonMsgOptions.jsonProtocolType = RSSL_JSON_JPT_JSON2; /* Supported only for Simplified JSON */
				getJsonMsgOptions.streamId = rsslMsg.msgBase.streamId;
				getJsonMsgOptions.isCloseMsg = (rsslMsg.msgBase.msgClass == RSSL_MC_CLOSE) ? RSSL_TRUE : RSSL_FALSE;

				if ((ret = rsslGetConverterJsonMsg(pReactorImpl->pJsonConverter, &getJsonMsgOptions,
					&jsonBuffer, &rjcError)) != RSSL_RET_SUCCESS)
				{
					rsslSetErrorInfo(pError, RSSL_EIC_FAILURE, RSSL_RET_FAILURE, __FILE__, __LINE__,
						"Failed to get converted JSON message. Error text: %s", rjcError.text);
					return (reactorUnlockInterface((RsslReactorImpl*)pReactor), RSSL_RET_FAILURE);
				}

				/* Copies JSON data format to the buffer that belongs to RsslChannel */
				pMsgBuffer = rsslReactorGetBuffer(&pReactorChannel->reactorChannel, jsonBuffer.length, RSSL_FALSE, pError);

				if (pMsgBuffer)
				{
					pMsgBuffer->length = jsonBuffer.length;
					memcpy(pMsgBuffer->data, jsonBuffer.data, jsonBuffer.length);
				}
				else
				{
					rsslSetErrorInfo(pError, RSSL_EIC_FAILURE, RSSL_RET_FAILURE, __FILE__, __LINE__,
						"Failed to get a buffer for sending JSON message. Error text: %s", pError->rsslError.text);
					return (reactorUnlockInterface((RsslReactorImpl*)pReactor), RSSL_RET_FAILURE);
				}

				releaseUserBuffer = RSSL_TRUE; /* Release the passed in buffer when using a new buffer for the JSON message */

				ret = rsslWrite(pReactorChannel->reactorChannel.pRsslChannel,
					pMsgBuffer,
					pSubmitOptions->priority,
					pSubmitOptions->writeFlags,
					pSubmitOptions->pBytesWritten ? pSubmitOptions->pBytesWritten : &dummyBytesWritten,
					pSubmitOptions->pUncompressedBytesWritten ? pSubmitOptions->pUncompressedBytesWritten : &dummyUncompBytesWritten,
					&pError->rsslError);
			}
			else
			{
				rsslSetErrorInfo(pError, RSSL_EIC_FAILURE, ret, __FILE__, __LINE__,
					"rsslDecodeMsg() failed to decode the passed in buffer as RWF messages.");
				return (reactorUnlockInterface((RsslReactorImpl*)pReactor), ret);
			}
		}
		else
		{
			if (pReactorChannel->pUserBufferWriteCallAgain == buffer)
			{
				pMsgBuffer = pReactorChannel->pWriteCallAgainUserBuffer;

				releaseUserBuffer = RSSL_TRUE; /* Release the passed in buffer when using the buffer for the JSON message */

				ret = rsslWrite(pReactorChannel->reactorChannel.pRsslChannel,
					pMsgBuffer,
					pSubmitOptions->priority,
					pSubmitOptions->writeFlags,
					pSubmitOptions->pBytesWritten ? pSubmitOptions->pBytesWritten : &dummyBytesWritten,
					pSubmitOptions->pUncompressedBytesWritten ? pSubmitOptions->pUncompressedBytesWritten : &dummyUncompBytesWritten,
					&pError->rsslError);

				pReactorChannel->pWriteCallAgainUserBuffer = NULL;
				pReactorChannel->pUserBufferWriteCallAgain = NULL;
			}
			else
			{
				rsslSetErrorInfo(pError, RSSL_EIC_FAILURE, ret, __FILE__, __LINE__,
					"Expected to receive the same user's buffer from the RSSL_RET_WRITE_CALL_AGAIN return value to write data again.");
				return (reactorUnlockInterface((RsslReactorImpl*)pReactor), ret);
			}
		}
	}
	else
	{
		ret = rsslWrite(pReactorChannel->reactorChannel.pRsslChannel, 
				buffer, 
				pSubmitOptions->priority,
				pSubmitOptions->writeFlags, 
				pSubmitOptions->pBytesWritten ? pSubmitOptions->pBytesWritten : &dummyBytesWritten,
				pSubmitOptions->pUncompressedBytesWritten ? pSubmitOptions->pUncompressedBytesWritten : &dummyUncompBytesWritten,
				&pError->rsslError);
	}

	/* Collects write statistics */
	if ( (pReactorChannel->statisticFlags & RSSL_RC_ST_WRITE) && pReactorChannel->pChannelStatistic)
	{
		_cumulativeValue(&pReactorChannel->pChannelStatistic->bytesWritten, (pSubmitOptions->pBytesWritten) ? *pSubmitOptions->pBytesWritten : dummyBytesWritten);

		_cumulativeValue(&pReactorChannel->pChannelStatistic->uncompressedBytesWritten, 
			(pSubmitOptions->pUncompressedBytesWritten) ? *pSubmitOptions->pUncompressedBytesWritten : dummyUncompBytesWritten);
	}

	if ( ret < RSSL_RET_SUCCESS)
	{
		switch (ret)
		{
			case RSSL_RET_WRITE_FLUSH_FAILED:
				/* rsslWrite has the message, but attempted to flush and failed.  This is okay, just need to keep flushing. */
				ret = RSSL_RET_SUCCESS;
				pReactorChannel->writeRet = 1;

				if (pPackedBufferImpl && pHashLink)
				{
					rsslHashTableRemoveLink(&pReactorChannel->packedBufferHashTable, pHashLink);

					free(pPackedBufferImpl);
				}

				break;
			case RSSL_RET_WRITE_CALL_AGAIN:
				/* The message is a fragmented message and was only partially written because there were not enough output buffers in RSSL to send it.
				 * We will request flushing to make the buffers available.
				 * RSSL_RET_WRITE_CALL_AGAIN will still be returned to the application.  It should attempt to call rsslReactorSubmit() again later. */
				pReactorChannel->writeRet = 1;

				/* Sends for the converted JSON message */
				if (releaseUserBuffer == RSSL_TRUE)
				{
					pReactorChannel->pWriteCallAgainUserBuffer = pMsgBuffer;
					pReactorChannel->pUserBufferWriteCallAgain = buffer;
					releaseUserBuffer = RSSL_FALSE;
				}

				break;
			default:
			{
				/* Failure */
				rsslSetErrorInfoLocation(pError, __FILE__, __LINE__);

				/* Releases the additional buffer for sending JSON messages. */
				if (pMsgBuffer)
				{
					RsslError rsslError;
					rsslReleaseBuffer(pMsgBuffer, &rsslError);
				}

				return (reactorUnlockInterface(pReactorImpl), ret);
			}
		}
	}
	else if (ret > 0)
	{
		/* The message was written to RSSL but has not yet been fully written to the network.  Flushing is needed to complete sending. */
		pReactorChannel->writeRet = ret;
		ret = RSSL_RET_SUCCESS;

		if (pPackedBufferImpl && pHashLink)
		{
			rsslHashTableRemoveLink(&pReactorChannel->packedBufferHashTable, pHashLink);

			free(pPackedBufferImpl);
		}
	}

	if (pReactorChannel->writeRet > 0)
	{
		if (releaseUserBuffer)
		{
			RsslError rsslError;
			rsslReleaseBuffer(buffer, &rsslError);
		}

		/* Returns the error when flush failed */
		if (_reactorSendFlushRequest(pReactorImpl, pReactorChannel, pError) != RSSL_RET_SUCCESS)
		{
			return (reactorUnlockInterface(pReactorImpl), pError->rsslError.rsslErrorId);
		}

		return (reactorUnlockInterface(pReactorImpl), ret);
	}

	if (releaseUserBuffer)
	{
		RsslError rsslError;
		rsslReleaseBuffer(buffer, &rsslError);
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
	RsslBool handleWarmStandby = RSSL_FALSE;

	if ((ret = reactorLockInterface(pReactorImpl, RSSL_TRUE, pError)) != RSSL_RET_SUCCESS)
		return ret;

	/* Since the application passed in this channel, make sure it is valid for this reactor and that it is active. */
	if (!pReactorChannel || !rsslReactorChannelIsValid(pReactorImpl, pReactorChannel, pError))
		return (reactorUnlockInterface(pReactorImpl), RSSL_RET_INVALID_ARGUMENT);

	if (pReactorImpl->state != RSSL_REACTOR_ST_ACTIVE)
	{
		rsslSetErrorInfo(pError, RSSL_EIC_SHUTDOWN, RSSL_RET_INVALID_ARGUMENT, __FILE__, __LINE__, "Reactor is shutting down.");
		return (reactorUnlockInterface((RsslReactorImpl*)pReactor), RSSL_RET_FAILURE);
	}

	handleWarmStandby = _reactorHandlesWarmStandby(pReactorChannel);

	if (pReactorChannel->pWarmStandByHandlerImpl)
	{
		if (&pReactorChannel->pWarmStandByHandlerImpl->mainReactorChannelImpl == pReactorChannel)
		{
			pReactorChannel = pReactorChannel->pWarmStandByHandlerImpl->pStartingReactorChannel;
		}
	}

	if (pReactorChannel->pWatchlist || handleWarmStandby)
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

		if (handleWarmStandby == RSSL_FALSE)
		{
			ret = _reactorSubmitWatchlistMsg(pReactorImpl, pReactorChannel, &processOpts, pError);
			return (reactorUnlockInterface((RsslReactorImpl*)pReactor), ret);
		}
		else
		{
			ret = _reactorWSWriteWatchlistMsg(pReactorImpl, pReactorChannel, pOptions, &processOpts, pError);
			return (reactorUnlockInterface((RsslReactorImpl*)pReactor), ret);
		}
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
				< RSSL_RET_SUCCESS)
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

		pReactorChannel->pWriteCallAgainUserBuffer = NULL;
		pReactorChannel->pUserBufferWriteCallAgain = NULL;
	}

	if (isReactorDebugLevelEnabled(pReactorImpl, RSSL_RC_DEBUG_LEVEL_CONNECTION))
	{
		if (pReactorImpl->pReactorDebugInfo == NULL || pReactorChannel->pChannelDebugInfo == NULL)
		{
			if (_initReactorAndChannelDebugInfo(pReactorImpl, pReactorChannel, pError) != RSSL_RET_SUCCESS)
			{
				_reactorShutdown(pReactorImpl, pError);
				_reactorSendShutdownEvent(pReactorImpl, pError);
				return (reactorUnlockInterface(pReactorImpl), RSSL_RET_FAILURE);
			}
		}

		if (pEvent->channelEvent.channelEventType == RSSL_RC_CET_CHANNEL_DOWN_RECONNECTING)
		{
			pReactorChannel->pChannelDebugInfo->debugInfoState |= RSSL_RC_DEBUG_INFO_CHANNEL_RECONNECTING;

			_writeDebugInfo(pReactorImpl, "Reactor("RSSL_REACTOR_POINTER_PRINT_TYPE"), Reactor channel("RSSL_REACTOR_POINTER_PRINT_TYPE") is RECONNECTING on channel fd="RSSL_REACTOR_SOCKET_PRINT_TYPE".]\n", pReactorImpl, pReactorChannel, pReactorChannel->reactorChannel.socketId);
		}
		else
		{
			pReactorChannel->pChannelDebugInfo->debugInfoState &= (~RSSL_RC_DEBUG_INFO_CHANNEL_UP);
			pReactorChannel->pChannelDebugInfo->debugInfoState |= RSSL_RC_DEBUG_INFO_CHANNEL_DOWN;

			_writeDebugInfo(pReactorImpl, "Reactor("RSSL_REACTOR_POINTER_PRINT_TYPE"), Reactor channel("RSSL_REACTOR_POINTER_PRINT_TYPE") is DOWN on channel fd="RSSL_REACTOR_SOCKET_PRINT_TYPE".]\n", pReactorImpl, pReactorChannel, pReactorChannel->reactorChannel.socketId);
		}
	}

	/* Bring down tunnel streams. */
	if (pReactorChannel->pTunnelManager 
			&& !pReactorChannel->pWatchlist /* Watchlist will fanout closes */)
	{
		RsslRet ret;
		if ((ret = tunnelManagerHandleChannelClosed(pReactorChannel->pTunnelManager, pError)) != RSSL_RET_SUCCESS)
				return ret;
	}

	{
		RsslReactorChannel* pCallbackChannel = (RsslReactorChannel*)pReactorChannel;

		if (_reactorHandlesWarmStandby(pReactorChannel) && pReactorImpl->state == RSSL_REACTOR_ST_ACTIVE)
		{
			RsslReactorWarmStandByHandlerImpl *pWarmStandByHandlerImpl = pReactorChannel->pWarmStandByHandlerImpl;
			RsslReactorWarmStandbyGroupImpl* pWarmStandByGroup = &pWarmStandByHandlerImpl->warmStandbyGroupList[pWarmStandByHandlerImpl->currentWSyGroupIndex];
			RsslBool	sendChannelCallback = RSSL_FALSE;

			if (pReactorChannel->reactorChannel.pRsslChannel && pReactorChannel->reactorChannel.pRsslChannel->state == RSSL_CH_STATE_CLOSED)
			{
				if (pWarmStandByGroup->warmStandbyMode == RSSL_RWSB_MODE_LOGIN_BASED)
				{
					if (pReactorChannel->isActiveServer)
					{
						RsslQueueLink *pLink;
						RsslReactorChannelImpl *pNextReactorChannel = NULL;

						RSSL_MUTEX_LOCK(&pWarmStandByHandlerImpl->warmStandByHandlerMutex);
						/* Submits to a channel that belongs to the warm standby feature and it is active */
						RSSL_QUEUE_FOR_EACH_LINK(&pWarmStandByHandlerImpl->rsslChannelQueue, pLink)
						{
							pNextReactorChannel = RSSL_QUEUE_LINK_TO_OBJECT(RsslReactorChannelImpl, warmstandbyChannelLink, pLink);

							if (pReactorChannel != pNextReactorChannel && pNextReactorChannel->reactorChannel.pRsslChannel &&
								pNextReactorChannel->reactorChannel.pRsslChannel->state == RSSL_CH_STATE_ACTIVE)
							{
								RsslReactorWarmStanbyEvent *pReactorWarmStanbyEvent = (RsslReactorWarmStanbyEvent*)rsslReactorEventQueueGetFromPool(&pReactorImpl->reactorEventQueue);
								rsslClearReactorWarmStanbyEvent(pReactorWarmStanbyEvent);

								pReactorWarmStanbyEvent->reactorWarmStandByEventType = RSSL_RCIMPL_WSBET_CHANGE_ACTIVE_TO_STANDBY_SERVER;
								pReactorWarmStanbyEvent->pReactorChannel = (RsslReactorChannel*)pReactorChannel;
								pWarmStandByHandlerImpl->pNextActiveReactorChannel = pNextReactorChannel;
								if (!RSSL_ERROR_INFO_CHECK(rsslReactorEventQueuePut(&pReactorImpl->reactorEventQueue, (RsslReactorEventImpl*)pReactorWarmStanbyEvent) == RSSL_RET_SUCCESS, RSSL_RET_FAILURE, pError))
								{
									RSSL_MUTEX_UNLOCK(&pWarmStandByHandlerImpl->warmStandByHandlerMutex);
									return RSSL_RET_FAILURE;
								}

								break;
							}
						}
						RSSL_MUTEX_UNLOCK(&pWarmStandByHandlerImpl->warmStandByHandlerMutex);

						pReactorChannel->isActiveServer = RSSL_FALSE;
						pReactorChannel->pWarmStandByHandlerImpl->pActiveReactorChannel = NULL;
					}
					else
					{
						/* Checks whether there is an active channel */
						if (pWarmStandByHandlerImpl->pActiveReactorChannel == NULL)
						{
							RsslBool selectedAsActiveServer = RSSL_FALSE;
							if (pWarmStandByGroup->downloadConfigActiveServer == RSSL_REACTOR_WSB_STARTING_SERVER_INDEX && pReactorChannel->isStartingServerConfig)
							{
								selectedAsActiveServer = RSSL_TRUE;
							}
							else if (!pReactorChannel->isStartingServerConfig && pWarmStandByGroup->downloadConfigActiveServer == pReactorChannel->standByServerListIndex)
							{
								selectedAsActiveServer = RSSL_TRUE;
							}

							/* This channel failed to connect so select others server as an active server instead. */
							if (selectedAsActiveServer)
							{
								RsslQueueLink *pLink;
								RsslReactorChannelImpl *pNextReactorChannel = NULL;

								RSSL_MUTEX_LOCK(&pWarmStandByHandlerImpl->warmStandByHandlerMutex);
								/* Submits to a channel that belongs to the warm standby feature and it is active */
								RSSL_QUEUE_FOR_EACH_LINK(&pWarmStandByHandlerImpl->rsslChannelQueue, pLink)
								{
									pNextReactorChannel = RSSL_QUEUE_LINK_TO_OBJECT(RsslReactorChannelImpl, warmstandbyChannelLink, pLink);

									if (pReactorChannel != pNextReactorChannel && pNextReactorChannel->reactorChannel.pRsslChannel &&
										pNextReactorChannel->reactorChannel.pRsslChannel->state == RSSL_CH_STATE_ACTIVE)
									{
										RsslReactorWarmStanbyEvent *pReactorWarmStanbyEvent = (RsslReactorWarmStanbyEvent*)rsslReactorEventQueueGetFromPool(&pReactorImpl->reactorEventQueue);
										rsslClearReactorWarmStanbyEvent(pReactorWarmStanbyEvent);

										pReactorWarmStanbyEvent->reactorWarmStandByEventType = RSSL_RCIMPL_WSBET_CHANGE_ACTIVE_TO_STANDBY_SERVER;
										pReactorWarmStanbyEvent->pReactorChannel = (RsslReactorChannel*)pReactorChannel;
										pWarmStandByHandlerImpl->pNextActiveReactorChannel = pNextReactorChannel;
										if (!RSSL_ERROR_INFO_CHECK(rsslReactorEventQueuePut(&pReactorImpl->reactorEventQueue, (RsslReactorEventImpl*)pReactorWarmStanbyEvent) == RSSL_RET_SUCCESS, RSSL_RET_FAILURE, pError))
										{
											RSSL_MUTEX_UNLOCK(&pWarmStandByHandlerImpl->warmStandByHandlerMutex);
											return RSSL_RET_FAILURE;
										}

										break;
									}
								}
								RSSL_MUTEX_UNLOCK(&pWarmStandByHandlerImpl->warmStandByHandlerMutex);
							}
						}
					}
				}
				else
				{
					RsslReactorWarmStanbyEvent *pReactorWarmStanbyEvent = (RsslReactorWarmStanbyEvent*)rsslReactorEventQueueGetFromPool(&pReactorImpl->reactorEventQueue);
					rsslClearReactorWarmStanbyEvent(pReactorWarmStanbyEvent);

					/* Per service based warm standby */
					pReactorWarmStanbyEvent->reactorWarmStandByEventType = RSSL_RCIMPL_WSBET_CHANGE_ACTIVE_TO_STANDBY_SERVICE_CHANNEL_DOWN;
					pReactorWarmStanbyEvent->pReactorChannel = (RsslReactorChannel*)pReactorChannel;

					if (!RSSL_ERROR_INFO_CHECK(rsslReactorEventQueuePut(&pReactorImpl->reactorEventQueue, (RsslReactorEventImpl*)pReactorWarmStanbyEvent) == RSSL_RET_SUCCESS, RSSL_RET_FAILURE, pError))
					{
						return RSSL_RET_FAILURE;
					}
				}
			}

			if (pEvent->channelEvent.channelEventType == RSSL_RC_CET_CHANNEL_DOWN)
			{
				RsslBool recoverChannel = RSSL_FALSE;
				RsslUInt32 numberOfStandbyChannel = (pWarmStandByHandlerImpl->rsslChannelQueue.count - 1);

				if (pReactorChannel->isStartingServerConfig)
				{
					/* Checks whether there is an additional warm standby group or a channel list to switch to */
					if ((pWarmStandByHandlerImpl->currentWSyGroupIndex + 1) < pWarmStandByHandlerImpl->warmStandbyGroupCount)
					{
						pWarmStandByHandlerImpl->warmStandByHandlerState |= RSSL_RWSB_STATE_MOVE_TO_NEXT_WSB_GROUP;
						recoverChannel = RSSL_TRUE;
					}
					else if (pWarmStandByHandlerImpl->hasConnectionList)
					{
						recoverChannel = RSSL_TRUE;
					}

					if (recoverChannel)
					{
						pWarmStandByHandlerImpl->warmStandByHandlerState |= RSSL_RWSB_STATE_CLOSING_STANDBY_CHANNELS;
						pEvent->channelEvent.channelEventType = RSSL_RC_CET_CHANNEL_DOWN_RECONNECTING;
						_reactorMoveChannel(&pReactorImpl->reconnectingChannels, pReactorChannel);
						pReactorChannel->reconnectAttemptCount = 0;
					}
				}
				else
				{
					if (((pWarmStandByHandlerImpl->currentWSyGroupIndex + 1) < pWarmStandByHandlerImpl->warmStandbyGroupCount) || pWarmStandByHandlerImpl->hasConnectionList)
					{
						pEvent->channelEvent.channelEventType = RSSL_RC_CET_CHANNEL_DOWN_RECONNECTING;
					}

					if (isWarmStandbyChannelClosed(pReactorChannel->pWarmStandByHandlerImpl, pReactorChannel))
					{
						pCallbackChannel = &pReactorChannel->pWarmStandByHandlerImpl->mainReactorChannelImpl.reactorChannel;
						pCallbackChannel->pRsslChannel = pReactorChannel->reactorChannel.pRsslChannel;
						pCallbackChannel->socketId = pReactorChannel->reactorChannel.socketId;
						pCallbackChannel->oldSocketId = pReactorChannel->reactorChannel.oldSocketId;
						pCallbackChannel->majorVersion = pReactorChannel->reactorChannel.majorVersion;
						pCallbackChannel->minorVersion = pReactorChannel->reactorChannel.minorVersion;
						pCallbackChannel->protocolType = pReactorChannel->reactorChannel.protocolType;
						pCallbackChannel->userSpecPtr = pReactorChannel->reactorChannel.userSpecPtr;

						pEvent->channelEvent.pReactorChannel = pCallbackChannel;

						/* Notify application */
						_reactorSetInCallback(pReactorImpl, RSSL_TRUE);
						(*pReactorChannel->channelRole.base.channelEventCallback)((RsslReactor*)pReactorImpl, pCallbackChannel, &pEvent->channelEvent);
						_reactorSetInCallback(pReactorImpl, RSSL_FALSE);

						pEvent->channelEvent.pReactorChannel = (RsslReactorChannel*)pReactorChannel;

						if (pEvent->channelEvent.channelEventType == RSSL_RC_CET_CHANNEL_DOWN)
							pReactorChannel->pWarmStandByHandlerImpl->mainChannelState = RSSL_RWSB_STATE_CHANNEL_DOWN;
						else
							pReactorChannel->pWarmStandByHandlerImpl->mainChannelState = RSSL_RWSB_STATE_CHANNEL_DOWN_RECONNECTING;
					}
					else
					{
						RsslReactorChannelEventType	channelEventTypeTemp = pEvent->channelEvent.channelEventType;

						pCallbackChannel = &pReactorChannel->pWarmStandByHandlerImpl->mainReactorChannelImpl.reactorChannel;
						pCallbackChannel->pRsslChannel = pReactorChannel->reactorChannel.pRsslChannel;
						pCallbackChannel->socketId = pReactorChannel->reactorChannel.socketId;
						pCallbackChannel->oldSocketId = pReactorChannel->reactorChannel.oldSocketId;
						pCallbackChannel->majorVersion = pReactorChannel->reactorChannel.majorVersion;
						pCallbackChannel->minorVersion = pReactorChannel->reactorChannel.minorVersion;
						pCallbackChannel->protocolType = pReactorChannel->reactorChannel.protocolType;
						pCallbackChannel->userSpecPtr = pReactorChannel->reactorChannel.userSpecPtr;

						if (pReactorChannel->reactorChannel.pRsslChannel && pReactorChannel->reactorChannel.pRsslChannel->state == RSSL_CH_STATE_ACTIVE &&
							channelEventTypeTemp == RSSL_RC_CET_CHANNEL_DOWN_RECONNECTING)
						{
							/* Remove this channel as it will be closed */
							_reactorHandlesWSBSocketList(pReactorChannel->pWarmStandByHandlerImpl, pCallbackChannel, &pReactorChannel->reactorChannel.pRsslChannel->socketId);
						}
						else
						{
							_reactorHandlesWSBSocketList(pReactorChannel->pWarmStandByHandlerImpl, pCallbackChannel, NULL);
						}

						pEvent->channelEvent.channelEventType = RSSL_RC_CET_FD_CHANGE;

						pEvent->channelEvent.pReactorChannel = pCallbackChannel;

						/* Notify application */
						_reactorSetInCallback(pReactorImpl, RSSL_TRUE);
						(*pReactorChannel->channelRole.base.channelEventCallback)((RsslReactor*)pReactorImpl, pCallbackChannel, &pEvent->channelEvent);
						_reactorSetInCallback(pReactorImpl, RSSL_FALSE);

						pEvent->channelEvent.pReactorChannel = (RsslReactorChannel*)pReactorChannel;

						pEvent->channelEvent.channelEventType = channelEventTypeTemp;
					}

					sendChannelCallback = RSSL_TRUE;

					/* Closing standby server's channels if any. */
					if (_processingReactorChannelShutdown(pReactorImpl, pReactorChannel, pError) != RSSL_RET_SUCCESS)
					{
						return RSSL_RET_FAILURE;
					}

					++pWarmStandByGroup->numOfClosingStandbyServers;
				}

				if (numberOfStandbyChannel == pWarmStandByGroup->numOfClosingStandbyServers)
				{
					pWarmStandByHandlerImpl->warmStandByHandlerState &= ~RSSL_RWSB_STATE_CLOSING_STANDBY_CHANNELS;

					if ( (pWarmStandByHandlerImpl->warmStandByHandlerState & RSSL_RWSB_STATE_MOVE_TO_NEXT_WSB_GROUP) != 0 )
					{
						RsslErrorInfo errorInfo;
						RsslRet ret;

						pWarmStandByHandlerImpl->currentWSyGroupIndex++;

						/* Clears the current state and starting the servers of another warm standby group. */
						pWarmStandByHandlerImpl->warmStandByHandlerState = RSSL_RWSB_STATE_CONNECTING_TO_A_STARTING_SERVER;
						
						ret = _reactorQueuedWSBGroupRecoveryMsg(pWarmStandByHandlerImpl, &errorInfo);

						pWarmStandByHandlerImpl->pStartingReactorChannel->reactorChannel.reactorChannelType = RSSL_REACTOR_CHANNEL_TYPE_WARM_STANDBY;
						pReactorChannel->reconnectAttemptCount = 0;
					}
					else
					{
						pWarmStandByHandlerImpl->warmStandByHandlerState = RSSL_RWSB_STATE_MOVE_TO_CHANNEL_LIST;
						pWarmStandByHandlerImpl->pStartingReactorChannel->reactorChannel.reactorChannelType = RSSL_REACTOR_CHANNEL_TYPE_NORMAL;
						pReactorChannel->reconnectAttemptCount = 0;
					}
				}
			}

			if (!sendChannelCallback)
			{
				if (isWarmStandbyChannelClosed(pReactorChannel->pWarmStandByHandlerImpl, pReactorChannel))
				{
					pCallbackChannel = &pReactorChannel->pWarmStandByHandlerImpl->mainReactorChannelImpl.reactorChannel;
					pCallbackChannel->pRsslChannel = pReactorChannel->reactorChannel.pRsslChannel;
					pCallbackChannel->socketId = pReactorChannel->reactorChannel.socketId;
					pCallbackChannel->oldSocketId = pReactorChannel->reactorChannel.oldSocketId;
					pCallbackChannel->majorVersion = pReactorChannel->reactorChannel.majorVersion;
					pCallbackChannel->minorVersion = pReactorChannel->reactorChannel.minorVersion;
					pCallbackChannel->protocolType = pReactorChannel->reactorChannel.protocolType;
					pCallbackChannel->userSpecPtr = pReactorChannel->reactorChannel.userSpecPtr;

					_reactorHandlesWSBSocketList(pReactorChannel->pWarmStandByHandlerImpl, pCallbackChannel, NULL);

					pEvent->channelEvent.pReactorChannel = pCallbackChannel;

					/* Notify application */
					_reactorSetInCallback(pReactorImpl, RSSL_TRUE);
					(*pReactorChannel->channelRole.base.channelEventCallback)((RsslReactor*)pReactorImpl, pCallbackChannel, &pEvent->channelEvent);
					_reactorSetInCallback(pReactorImpl, RSSL_FALSE);

					pEvent->channelEvent.pReactorChannel = (RsslReactorChannel*)pReactorChannel;

					if (pEvent->channelEvent.channelEventType == RSSL_RC_CET_CHANNEL_DOWN)
						pReactorChannel->pWarmStandByHandlerImpl->mainChannelState = RSSL_RWSB_STATE_CHANNEL_DOWN;
					else
						pReactorChannel->pWarmStandByHandlerImpl->mainChannelState = RSSL_RWSB_STATE_CHANNEL_DOWN_RECONNECTING;
				}
				else
				{
					RsslReactorChannelEventType	channelEventTypeTemp = pEvent->channelEvent.channelEventType;

					pCallbackChannel = &pReactorChannel->pWarmStandByHandlerImpl->mainReactorChannelImpl.reactorChannel;
					pCallbackChannel->pRsslChannel = pReactorChannel->reactorChannel.pRsslChannel;
					pCallbackChannel->socketId = pReactorChannel->reactorChannel.socketId;
					pCallbackChannel->oldSocketId = pReactorChannel->reactorChannel.oldSocketId;
					pCallbackChannel->majorVersion = pReactorChannel->reactorChannel.majorVersion;
					pCallbackChannel->minorVersion = pReactorChannel->reactorChannel.minorVersion;
					pCallbackChannel->protocolType = pReactorChannel->reactorChannel.protocolType;
					pCallbackChannel->userSpecPtr = pReactorChannel->reactorChannel.userSpecPtr;

					if (pReactorChannel->reactorChannel.pRsslChannel && pReactorChannel->reactorChannel.pRsslChannel->state == RSSL_CH_STATE_ACTIVE &&
						channelEventTypeTemp == RSSL_RC_CET_CHANNEL_DOWN_RECONNECTING)
					{
						/* Remove this channel as it will be closed */
						_reactorHandlesWSBSocketList(pReactorChannel->pWarmStandByHandlerImpl, pCallbackChannel, &pReactorChannel->reactorChannel.pRsslChannel->socketId);
					}
					else
					{
						_reactorHandlesWSBSocketList(pReactorChannel->pWarmStandByHandlerImpl, pCallbackChannel, NULL);
					}

					pEvent->channelEvent.channelEventType = RSSL_RC_CET_FD_CHANGE;

					pEvent->channelEvent.pReactorChannel = pCallbackChannel;

					/* Notify application */
					_reactorSetInCallback(pReactorImpl, RSSL_TRUE);
					(*pReactorChannel->channelRole.base.channelEventCallback)((RsslReactor*)pReactorImpl, pCallbackChannel, &pEvent->channelEvent);
					_reactorSetInCallback(pReactorImpl, RSSL_FALSE);

					pEvent->channelEvent.pReactorChannel = (RsslReactorChannel*)pReactorChannel;

					pEvent->channelEvent.channelEventType = channelEventTypeTemp;
				}
			}
		}
		else
		{
			if (_reactorHandlesWarmStandby(pReactorChannel))
			{
				/* Reactor is shuting down for warm standby channel case. */
				RsslReactorWarmStandByHandlerImpl* pWarmStandByHandlerImpl = pReactorChannel->pWarmStandByHandlerImpl;

				pCallbackChannel = &pReactorChannel->pWarmStandByHandlerImpl->mainReactorChannelImpl.reactorChannel;
				pCallbackChannel->pRsslChannel = pReactorChannel->reactorChannel.pRsslChannel;
				pCallbackChannel->socketId = pReactorChannel->reactorChannel.socketId;
				pCallbackChannel->oldSocketId = pReactorChannel->reactorChannel.oldSocketId;
				pCallbackChannel->majorVersion = pReactorChannel->reactorChannel.majorVersion;
				pCallbackChannel->minorVersion = pReactorChannel->reactorChannel.minorVersion;
				pCallbackChannel->protocolType = pReactorChannel->reactorChannel.protocolType;
				pCallbackChannel->userSpecPtr = pReactorChannel->reactorChannel.userSpecPtr;

				pEvent->channelEvent.pReactorChannel = pCallbackChannel;

				_reactorSetInCallback(pReactorImpl, RSSL_TRUE);
				(*pReactorChannel->channelRole.base.channelEventCallback)((RsslReactor*)pReactorImpl, pCallbackChannel, &pEvent->channelEvent);
				_reactorSetInCallback(pReactorImpl, RSSL_FALSE);

				pEvent->channelEvent.pReactorChannel = (RsslReactorChannel*)pReactorChannel;
			}
			else
			{
				_reactorSetInCallback(pReactorImpl, RSSL_TRUE);
				(*pReactorChannel->channelRole.base.channelEventCallback)((RsslReactor*)pReactorImpl, pCallbackChannel, &pEvent->channelEvent);
				_reactorSetInCallback(pReactorImpl, RSSL_FALSE);
			}
		}
	}

	if (pReactorChannel->pWatchlist)
	{
		RsslRet ret;
		if ((ret = rsslWatchlistSetChannel(pReactorChannel->pWatchlist, NULL, pError)) != RSSL_RET_SUCCESS)
			return ret;
	}

	if(pReactorChannel->reactorParentQueue ==  &pReactorImpl->closingChannels)
	{
		rsslReactorEventQueueReturnToPool((RsslReactorEventImpl*)pEvent, &pReactorImpl->reactorWorker.workerQueue, pReactorImpl->maxEventsInPool);
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
		RsslWatchlistProcessMsgOptions *pOptions, ReactorWSProcessMsgOptions* pWsOption, RsslErrorInfo *pError)
{
	RsslRet ret;

	if ((ret = rsslWatchlistReadMsg(pReactorChannel->pWatchlist, pOptions, pWsOption, pReactorImpl->lastRecordedTimeMs, pError)) < RSSL_RET_SUCCESS)
		return _reactorHandleChannelDown(pReactorImpl, pReactorChannel, pError);

	if (pReactorImpl->state != RSSL_REACTOR_ST_ACTIVE)
	{
		_reactorShutdown(pReactorImpl, pError);
		_reactorSendShutdownEvent(pReactorImpl, pError);
		return ret;
	}

	if (pReactorChannel->pWatchlist->state & RSSLWL_STF_NEED_FLUSH) 
	{
		RsslRet flushRet;
		pReactorChannel->writeRet = 1;
		flushRet =  _reactorSendFlushRequest(pReactorImpl, pReactorChannel, pError);
		if (flushRet < RSSL_RET_SUCCESS)
			return flushRet;
	}

	if (pReactorChannel->pWatchlist->state & RSSLWL_STF_RESET_CONN_DELAY) 
	{
		/* Login stream established. Reset connection delay. */
		pReactorChannel->reconnectAttemptCount = 0;
		pReactorChannel->pWatchlist->state &= ~RSSLWL_STF_RESET_CONN_DELAY;
		if (pReactorChannel->connectionOptList)
			pReactorChannel->connectionOptList[pReactorChannel->connectionListIter].reconnectEndpointAttemptCount = 0;
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
		RsslRet flushRet;
		pReactorChannel->writeRet = 1;
		flushRet =  _reactorSendFlushRequest(pReactorImpl, pReactorChannel, pError);
		if (flushRet < RSSL_RET_SUCCESS)
			return flushRet;
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

static RsslRet _processingReactorChannelShutdown(RsslReactorImpl *pReactorImpl, RsslReactorChannelImpl *pReactorChannel, RsslErrorInfo *pError)
{
	/* Channel is not currently trying to close.  Start the process of shutting it down. */
	RsslReactorChannelEventImpl *pEvent = NULL;

	/* Channel is in the process of being closed. */
	if (pReactorChannel->reactorParentQueue == &pReactorImpl->closingChannels)
	{
		return RSSL_RET_SUCCESS;
	}
		
	pEvent = (RsslReactorChannelEventImpl*)rsslReactorEventQueueGetFromPool(&pReactorImpl->reactorWorker.workerQueue);

	if (rsslNotifierRemoveEvent(pReactorImpl->pNotifier, pReactorChannel->pNotifierEvent) < 0)
	{
		rsslSetErrorInfo(pError, RSSL_EIC_FAILURE, RSSL_RET_FAILURE, __FILE__, __LINE__,
			"Failed to remove notification event for disconnected channel.");
		return RSSL_RET_FAILURE;
	}

	_reactorMoveChannel(&pReactorImpl->closingChannels, pReactorChannel);

	pReactorChannel->pWriteCallAgainUserBuffer = NULL;
	pReactorChannel->pUserBufferWriteCallAgain = NULL;

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
		return RSSL_RET_FAILURE;


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

		if (isReactorDebugLevelEnabled(pReactorImpl, RSSL_RC_DEBUG_LEVEL_CONNECTION))
		{
			if (pReactorImpl->pReactorDebugInfo == NULL || pReactorChannel->pChannelDebugInfo == NULL)
			{
				if (_initReactorAndChannelDebugInfo(pReactorImpl, pReactorChannel, pError) != RSSL_RET_SUCCESS)
				{
					_reactorShutdown(pReactorImpl, pError);
					_reactorSendShutdownEvent(pReactorImpl, pError);
					return (reactorUnlockInterface(pReactorImpl), RSSL_RET_FAILURE);
				}
			}

			pReactorChannel->pChannelDebugInfo->debugInfoState |= RSSL_RC_DEBUG_INFO_CHANNEL_CLOSE;

			pReactorChannel->pChannelDebugInfo->numOfCloseChannelCall++;

			_writeDebugInfo(pReactorImpl, "Reactor("RSSL_REACTOR_POINTER_PRINT_TYPE"), Closes reactor channel("RSSL_REACTOR_POINTER_PRINT_TYPE") on channel fd="RSSL_REACTOR_SOCKET_PRINT_TYPE" but the channel is being closed.]\n",
				pReactor, pReactorChannel, pChannel->socketId);

			_writeDebugInfo(pReactorImpl, "Reactor("RSSL_REACTOR_POINTER_PRINT_TYPE"), Reactor channel("RSSL_REACTOR_POINTER_PRINT_TYPE"), number of closing call(%hu) and number of dispatching per channel call(%lu) ]\n",
				pReactor, pReactorChannel, pReactorChannel->pChannelDebugInfo->numOfCloseChannelCall, pReactorChannel->pChannelDebugInfo->numOfDispatchCall);
		}

		return (reactorUnlockInterface(pReactorImpl), RSSL_RET_SUCCESS);
	}
	else
	{
		if (isReactorDebugLevelEnabled(pReactorImpl, RSSL_RC_DEBUG_LEVEL_CONNECTION))
		{
			if (pReactorImpl->pReactorDebugInfo == NULL || pReactorChannel->pChannelDebugInfo == NULL)
			{
				if (_initReactorAndChannelDebugInfo(pReactorImpl, pReactorChannel, pError) != RSSL_RET_SUCCESS)
				{
					_reactorShutdown(pReactorImpl, pError);
					_reactorSendShutdownEvent(pReactorImpl, pError);
					return (reactorUnlockInterface(pReactorImpl), RSSL_RET_FAILURE);
				}
			}

			pReactorChannel->pChannelDebugInfo->numOfCloseChannelCall++;

			_writeDebugInfo(pReactorImpl, "Reactor("RSSL_REACTOR_POINTER_PRINT_TYPE"), Closes reactor channel("RSSL_REACTOR_POINTER_PRINT_TYPE") on channel fd="RSSL_REACTOR_SOCKET_PRINT_TYPE".]\n", pReactor,
				pReactorChannel, pChannel->socketId);

			_writeDebugInfo(pReactorImpl, "Reactor("RSSL_REACTOR_POINTER_PRINT_TYPE"), Reactor channel("RSSL_REACTOR_POINTER_PRINT_TYPE"), number of closing call(%hu) and number of dispatching per channel call(%lu) ]\n",
				pReactor, pReactorChannel, pReactorChannel->pChannelDebugInfo->numOfCloseChannelCall, pReactorChannel->pChannelDebugInfo->numOfDispatchCall);
		}
		
		if (_reactorHandlesWarmStandby(pReactorChannel))
		{
			RsslQueueLink* pLink = NULL;
			RsslReactorChannelImpl* pClosingReactorChannel;
			RsslReactorWarmStandByHandlerImpl* pReactorWarmStandbyHandlerImpl = pReactorChannel->pWarmStandByHandlerImpl;

			/* Channel is in the process of being closed. */
			if (pReactorWarmStandbyHandlerImpl->warmStandByHandlerState == RSSL_RWSB_STATE_CLOSING || 
				pReactorWarmStandbyHandlerImpl->warmStandByHandlerState == RSSL_RWSB_STATE_INACTIVE)
			{
				return (reactorUnlockInterface(pReactorImpl), RSSL_RET_SUCCESS);
			}

			pReactorWarmStandbyHandlerImpl->warmStandByHandlerState = RSSL_RWSB_STATE_CLOSING;

			rsslQueueAddLinkToBack(&pReactorImpl->closingWarmstandbyChannel, &pReactorWarmStandbyHandlerImpl->reactorQueueLink);

			RSSL_MUTEX_LOCK(&pReactorChannel->pWarmStandByHandlerImpl->warmStandByHandlerMutex);
			/* Submits to a channel that belongs to the warm standby feature and it is active */
			RSSL_QUEUE_FOR_EACH_LINK(&pReactorChannel->pWarmStandByHandlerImpl->rsslChannelQueue, pLink)
			{
				pClosingReactorChannel = RSSL_QUEUE_LINK_TO_OBJECT(RsslReactorChannelImpl, warmstandbyChannelLink, pLink);

				if (pClosingReactorChannel)
				{
					ret = _processingReactorChannelShutdown(pReactorImpl, pClosingReactorChannel, pError);

					if (ret != RSSL_RET_SUCCESS)
						break;
				}
			}
			RSSL_MUTEX_UNLOCK(&pReactorChannel->pWarmStandByHandlerImpl->warmStandByHandlerMutex);

			/* Sends an event to close the warm standby channel. */
			{
				RsslReactorChannelEventImpl* pEvent = NULL;

				pReactorWarmStandbyHandlerImpl->warmStandByHandlerState = RSSL_RWSB_STATE_CLOSING;

				pEvent = (RsslReactorChannelEventImpl*)rsslReactorEventQueueGetFromPool(&pReactorImpl->reactorWorker.workerQueue);

				/* Send request to worker to close this channel */
				rsslClearReactorChannelEventImpl(pEvent);
				pEvent->channelEvent.channelEventType = (RsslReactorChannelEventType)RSSL_RCIMPL_CET_CLOSE_WARMSTANDBY_CHANNEL;
				pEvent->channelEvent.pReactorChannel = (RsslReactorChannel*)pReactorChannel;
				pEvent->channelEvent.pError = pError;

				if (!RSSL_ERROR_INFO_CHECK(rsslReactorEventQueuePut(&pReactorImpl->reactorWorker.workerQueue, (RsslReactorEventImpl*)pEvent) == RSSL_RET_SUCCESS, RSSL_RET_FAILURE, pError))
					return (reactorUnlockInterface(pReactorImpl), RSSL_RET_FAILURE);
			}
		}
		else
		{
			if (pReactorChannel->pWarmStandByHandlerImpl)
			{
				if (&pReactorChannel->pWarmStandByHandlerImpl->mainReactorChannelImpl == pReactorChannel)
				{
					pReactorChannel = pReactorChannel->pWarmStandByHandlerImpl->pStartingReactorChannel;
				}
			}

			ret = _processingReactorChannelShutdown(pReactorImpl, pReactorChannel, pError);
		}

		return (reactorUnlockInterface(pReactorImpl), ret);
	}
}

/* Checks if tunnel stream needs a dispatch, flush, or timer (shouldn't produce a callback,
 * so it can be used in non-dispatch calls). */
static RsslRet _reactorHandleTunnelManagerEvents(RsslReactorImpl *pReactorImpl, RsslReactorChannelImpl *pReactorChannel, RsslErrorInfo *pError)
{
	RsslRet ret;

	if (tunnelManagerNeedsDispatchNow(pReactorChannel->pTunnelManager) && !pReactorChannel->tunnelDispatchEventQueued)
	{
		RsslReactorChannelEventImpl *pEvent;
		pReactorChannel->tunnelDispatchEventQueued = RSSL_TRUE;
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

	pEvent = rsslReactorEventQueueGet(pQueue, pReactorImpl->maxEventsInPool, &ret);


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

				    switch((int)pConnEvent->channelEvent.channelEventType)
					{
						case RSSL_RC_CET_CHANNEL_UP:
							/* Channel has been initialized by worker thread and is ready for reading & writing. */
							if (rsslNotifierAddEvent(pReactorImpl->pNotifier, pReactorChannel->pNotifierEvent, (int)(pReactorChannel->reactorChannel.socketId), pReactorChannel) < 0)
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
								if (pReactorChannel->connectionOptList)
									pReactorChannel->connectionOptList[pReactorChannel->connectionListIter].reconnectEndpointAttemptCount = 0;
							}

							_reactorMoveChannel(&pReactorImpl->activeChannels, pReactorChannel);
							pReactorChannel->lastPingReadMs = pReactorImpl->lastRecordedTimeMs;

							if (isReactorDebugLevelEnabled(pReactorImpl, RSSL_RC_DEBUG_LEVEL_CONNECTION))
							{
								if (pReactorImpl->pReactorDebugInfo == NULL || pReactorChannel->pChannelDebugInfo == NULL)
								{
									if (_initReactorAndChannelDebugInfo(pReactorImpl, pReactorChannel, pError) != RSSL_RET_SUCCESS)
									{
										_reactorShutdown(pReactorImpl, pError);
										_reactorSendShutdownEvent(pReactorImpl, pError);
										return (reactorUnlockInterface(pReactorImpl), RSSL_RET_FAILURE);
									}
								}

								pReactorChannel->pChannelDebugInfo->debugInfoState |= RSSL_RC_DEBUG_INFO_CHANNEL_UP;

								_writeDebugInfo(pReactorImpl, "Reactor("RSSL_REACTOR_POINTER_PRINT_TYPE"), Reactor channel("RSSL_REACTOR_POINTER_PRINT_TYPE") is UP on channel fd="RSSL_REACTOR_SOCKET_PRINT_TYPE".]\n", pReactorImpl, pReactorChannel, pReactorChannel->reactorChannel.socketId);
							}

							{
								RsslReactorChannel* pCallbackChannel = (RsslReactorChannel*)pReactorChannel;

								if (_reactorHandlesWarmStandby(pReactorChannel))
								{
									if (pReactorChannel->pWarmStandByHandlerImpl->mainChannelState < RSSL_RWSB_STATE_CHANNEL_UP)
									{
										pCallbackChannel = &pReactorChannel->pWarmStandByHandlerImpl->mainReactorChannelImpl.reactorChannel;
										pCallbackChannel->pRsslChannel = pReactorChannel->reactorChannel.pRsslChannel;
										pCallbackChannel->socketId = pReactorChannel->reactorChannel.socketId;
										pCallbackChannel->oldSocketId = pReactorChannel->reactorChannel.oldSocketId;
										pCallbackChannel->majorVersion = pReactorChannel->reactorChannel.majorVersion;
										pCallbackChannel->minorVersion = pReactorChannel->reactorChannel.minorVersion;
										pCallbackChannel->protocolType = pReactorChannel->reactorChannel.protocolType;
										pCallbackChannel->userSpecPtr = pReactorChannel->reactorChannel.userSpecPtr;

										_reactorHandlesWSBSocketList(pReactorChannel->pWarmStandByHandlerImpl, pCallbackChannel, NULL);

										pConnEvent->channelEvent.pReactorChannel = pCallbackChannel;

										/* Notify application */
										_reactorSetInCallback(pReactorImpl, RSSL_TRUE);
										cret = (*pReactorChannel->channelRole.base.channelEventCallback)((RsslReactor*)pReactorImpl, pCallbackChannel, &pConnEvent->channelEvent);
										_reactorSetInCallback(pReactorImpl, RSSL_FALSE);

										pConnEvent->channelEvent.pReactorChannel = (RsslReactorChannel*)pReactorChannel;

										pReactorChannel->pWarmStandByHandlerImpl->mainChannelState = RSSL_RWSB_STATE_CHANNEL_UP;
									}
									else
									{
										pCallbackChannel = &pReactorChannel->pWarmStandByHandlerImpl->mainReactorChannelImpl.reactorChannel;
										pCallbackChannel->pRsslChannel = pReactorChannel->reactorChannel.pRsslChannel;
										pCallbackChannel->socketId = pReactorChannel->reactorChannel.socketId;
										pCallbackChannel->oldSocketId = pReactorChannel->reactorChannel.oldSocketId;
										pCallbackChannel->majorVersion = pReactorChannel->reactorChannel.majorVersion;
										pCallbackChannel->minorVersion = pReactorChannel->reactorChannel.minorVersion;
										pCallbackChannel->protocolType = pReactorChannel->reactorChannel.protocolType;
										pCallbackChannel->userSpecPtr = pReactorChannel->reactorChannel.userSpecPtr;

										_reactorHandlesWSBSocketList(pReactorChannel->pWarmStandByHandlerImpl, pCallbackChannel, NULL);

										pConnEvent->channelEvent.channelEventType = RSSL_RC_CET_FD_CHANGE;

										pConnEvent->channelEvent.pReactorChannel = pCallbackChannel;

										/* Notify application */
										_reactorSetInCallback(pReactorImpl, RSSL_TRUE);
										cret = (*pReactorChannel->channelRole.base.channelEventCallback)((RsslReactor*)pReactorImpl, pCallbackChannel, &pConnEvent->channelEvent);
										_reactorSetInCallback(pReactorImpl, RSSL_FALSE);

										pConnEvent->channelEvent.pReactorChannel = (RsslReactorChannel*)pReactorChannel;

										/* Apply IOCTL codes if any */
										if(pReactorChannel->pWarmStandByHandlerImpl->ioCtlCodes != 0)
										{
											if ((pReactorChannel->pWarmStandByHandlerImpl->ioCtlCodes & RSSL_REACTOR_WS_TRACE) != 0)
											{
												if ((ret = rsslIoctl(pReactorChannel->reactorChannel.pRsslChannel, RSSL_TRACE,
													(void*)&pReactorChannel->pWarmStandByHandlerImpl->traceOptions, &pError->rsslError)) != RSSL_RET_SUCCESS)
												{
													if (_reactorHandleChannelDown(pReactorImpl, pReactorChannel, pError) != RSSL_RET_SUCCESS)
														return RSSL_RET_FAILURE;
												}
											}

											if ((pReactorChannel->pWarmStandByHandlerImpl->ioCtlCodes & RSSL_REACTOR_WS_MAX_NUM_BUFFERS) != 0)
											{
												if ((ret = rsslIoctl(pReactorChannel->reactorChannel.pRsslChannel, RSSL_MAX_NUM_BUFFERS,
													(void*)&pReactorChannel->pWarmStandByHandlerImpl->maxNumBuffers, &pError->rsslError)) != RSSL_RET_SUCCESS)
												{
													if (_reactorHandleChannelDown(pReactorImpl, pReactorChannel, pError) != RSSL_RET_SUCCESS)
														return RSSL_RET_FAILURE;
												}
											}

											if ((pReactorChannel->pWarmStandByHandlerImpl->ioCtlCodes & RSSL_REACTOR_WS_NUM_GUARANTEED_BUFFERS) != 0)
											{
												if ((ret = rsslIoctl(pReactorChannel->reactorChannel.pRsslChannel, RSSL_NUM_GUARANTEED_BUFFERS,
													(void*)&pReactorChannel->pWarmStandByHandlerImpl->numGuaranteedBuffers, &pError->rsslError)) != RSSL_RET_SUCCESS)
												{
													if (_reactorHandleChannelDown(pReactorImpl, pReactorChannel, pError) != RSSL_RET_SUCCESS)
														return RSSL_RET_FAILURE;
												}
											}

											if ((pReactorChannel->pWarmStandByHandlerImpl->ioCtlCodes & RSSL_REACTOR_WS_HIGH_WATER_MARK) != 0)
											{
												if ((ret = rsslIoctl(pReactorChannel->reactorChannel.pRsslChannel, RSSL_HIGH_WATER_MARK,
													(void*)&pReactorChannel->pWarmStandByHandlerImpl->highWaterMark, &pError->rsslError)) != RSSL_RET_SUCCESS)
												{
													if (_reactorHandleChannelDown(pReactorImpl, pReactorChannel, pError) != RSSL_RET_SUCCESS)
														return RSSL_RET_FAILURE;
												}
											}

											if ((pReactorChannel->pWarmStandByHandlerImpl->ioCtlCodes & RSSL_REACTOR_WS_SYSTEM_READ_BUFFERS) != 0)
											{
												if ((ret = rsslIoctl(pReactorChannel->reactorChannel.pRsslChannel, RSSL_SYSTEM_READ_BUFFERS,
													(void*)&pReactorChannel->pWarmStandByHandlerImpl->systemReadBuffers, &pError->rsslError)) != RSSL_RET_SUCCESS)
												{
													if (_reactorHandleChannelDown(pReactorImpl, pReactorChannel, pError) != RSSL_RET_SUCCESS)
														return RSSL_RET_FAILURE;
												}
											}

											if ((pReactorChannel->pWarmStandByHandlerImpl->ioCtlCodes & RSSL_REACTOR_WS_SYSTEM_WRITE_BUFFERS) != 0)
											{
												if ((ret = rsslIoctl(pReactorChannel->reactorChannel.pRsslChannel, RSSL_SYSTEM_WRITE_BUFFERS,
													(void*)&pReactorChannel->pWarmStandByHandlerImpl->systemWriteBuffers, &pError->rsslError)) != RSSL_RET_SUCCESS)
												{
													if (_reactorHandleChannelDown(pReactorImpl, pReactorChannel, pError) != RSSL_RET_SUCCESS)
														return RSSL_RET_FAILURE;
												}
											}

											if ((pReactorChannel->pWarmStandByHandlerImpl->ioCtlCodes & RSSL_REACTOR_WS_PRIORITY_FLUSH_ORDER) != 0)
											{
												if ((ret = rsslIoctl(pReactorChannel->reactorChannel.pRsslChannel, RSSL_PRIORITY_FLUSH_ORDER,
													(void*)&pReactorChannel->pWarmStandByHandlerImpl->priorityFlushOrder, &pError->rsslError)) != RSSL_RET_SUCCESS)
												{
													if (_reactorHandleChannelDown(pReactorImpl, pReactorChannel, pError) != RSSL_RET_SUCCESS)
														return RSSL_RET_FAILURE;
												}
											}

											if ((pReactorChannel->pWarmStandByHandlerImpl->ioCtlCodes & RSSL_REACTOR_WS_COMPRESSION_THRESHOLD) != 0)
											{
												if ((ret = rsslIoctl(pReactorChannel->reactorChannel.pRsslChannel, RSSL_COMPRESSION_THRESHOLD,
													(void*)&pReactorChannel->pWarmStandByHandlerImpl->compressionThresHold, &pError->rsslError)) != RSSL_RET_SUCCESS)
												{
													if (_reactorHandleChannelDown(pReactorImpl, pReactorChannel, pError) != RSSL_RET_SUCCESS)
														return RSSL_RET_FAILURE;
												}
											}
										}
									}
								}
								else
								{
									/* Notify application */
									_reactorSetInCallback(pReactorImpl, RSSL_TRUE);
									cret = (*pReactorChannel->channelRole.base.channelEventCallback)((RsslReactor*)pReactorImpl, pCallbackChannel, &pConnEvent->channelEvent);
									_reactorSetInCallback(pReactorImpl, RSSL_FALSE);
								}
							}

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

							{
								RsslReactorChannel* pCallbackChannel = (RsslReactorChannel*)pReactorChannel;

								if (_reactorHandlesWarmStandby(pReactorChannel))
								{
									if (pReactorChannel->pWarmStandByHandlerImpl->mainChannelState == RSSL_RWSB_STATE_CHANNEL_UP)
									{
										pCallbackChannel = &pReactorChannel->pWarmStandByHandlerImpl->mainReactorChannelImpl.reactorChannel;

										pCallbackChannel->pRsslChannel = pReactorChannel->reactorChannel.pRsslChannel;
										pCallbackChannel->socketId = pReactorChannel->reactorChannel.socketId;
										pCallbackChannel->oldSocketId = pReactorChannel->reactorChannel.oldSocketId;
										pCallbackChannel->majorVersion = pReactorChannel->reactorChannel.majorVersion;
										pCallbackChannel->minorVersion = pReactorChannel->reactorChannel.minorVersion;
										pCallbackChannel->protocolType = pReactorChannel->reactorChannel.protocolType;
										pCallbackChannel->userSpecPtr = pReactorChannel->reactorChannel.userSpecPtr;

										pConnEvent->channelEvent.pReactorChannel = pCallbackChannel;

										_reactorSetInCallback(pReactorImpl, RSSL_TRUE);
										cret = (*pReactorChannel->channelRole.base.channelEventCallback)((RsslReactor*)pReactorImpl, pCallbackChannel, &pConnEvent->channelEvent);
										_reactorSetInCallback(pReactorImpl, RSSL_FALSE);

										pConnEvent->channelEvent.pReactorChannel = (RsslReactorChannel*)pReactorChannel;

										pReactorChannel->pWarmStandByHandlerImpl->mainChannelState = RSSL_RWSB_STATE_CHANNEL_READY;
									}
								}
								else
								{
									_reactorSetInCallback(pReactorImpl, RSSL_TRUE);
									cret = (*pReactorChannel->channelRole.base.channelEventCallback)((RsslReactor*)pReactorImpl, pCallbackChannel, &pConnEvent->channelEvent);
									_reactorSetInCallback(pReactorImpl, RSSL_FALSE);
								}
							}

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
				    switch((int)pConnEvent->channelEvent.channelEventType)
					{
						case RSSL_RC_CET_CHANNEL_UP:
						case RSSL_RC_CET_CHANNEL_READY:
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

			case RSSL_RCIMPL_ET_TOKEN_MGNT:
			{
				RsslReactorTokenMgntEvent *pReactorTokenMgntEvent = (RsslReactorTokenMgntEvent*)pEvent;
				RsslReactorTokenSessionImpl *pTokenSession = pReactorTokenMgntEvent->pTokenSessionImpl;
				RsslReactorChannelImpl *pReactorChannel = NULL;
				RsslQueueLink *pLink = NULL;

				pReactorChannel = (RsslReactorChannelImpl*)pReactorTokenMgntEvent->pReactorChannel;

				/* The channel is being closed by the user. The channel can be NULL when submitting OAuth credential renewal without the token session */
				if (pReactorChannel && (pReactorChannel->reactorParentQueue == &pReactorImpl->closingChannels))
				{
					if ( (pReactorTokenMgntEvent->reactorTokenMgntEventType == RSSL_RCIMPL_TKET_RESP_FAILURE) ||
						(pReactorTokenMgntEvent->reactorTokenMgntEventType == RSSL_RCIMPL_TKET_CHANNEL_WARNING) )
					{
						if (pReactorTokenMgntEvent->pReactorErrorInfoImpl)
						{
							/* Decrease the reference count of RsslReactorErrorInfoImpl before putting it back to the pool. */
							if( --pReactorTokenMgntEvent->pReactorErrorInfoImpl->referenceCount == 0)
							{
								rsslReactorReturnErrorInfoToPool(pReactorTokenMgntEvent->pReactorErrorInfoImpl, &pReactorImpl->reactorWorker);
							}
						}
					}

					return RSSL_RET_SUCCESS;
				}

				switch (pReactorTokenMgntEvent->reactorTokenMgntEventType)
				{
					RsslWatchlistProcessMsgOptions processOpts;
					case RSSL_RCIMPL_TKET_REISSUE:
					case RSSL_RCIMPL_TKET_REISSUE_NO_REFRESH:
					{
						RsslReactorImpl *pReactorImpl = (RsslReactorImpl*)pTokenSession->pReactor;

						if (pReactorChannel)
						{
							/* Update the login request only when users specified to send initial login request after the connection is recovered. */
							if (pReactorChannel->channelRole.ommConsumerRole.pLoginRequest)
							{
								pReactorChannel->channelRole.ommConsumerRole.pLoginRequest->userName = pTokenSession->tokenInformation.accessToken;
								pReactorChannel->channelRole.ommConsumerRole.pLoginRequest->userNameType = RDM_LOGIN_USER_AUTHN_TOKEN;

								pReactorChannel->channelRole.ommConsumerRole.pLoginRequest->flags |= RDM_LG_RQF_HAS_USERNAME_TYPE;

								pReactorChannel->channelRole.ommConsumerRole.pLoginRequest->flags &= (~RDM_LG_RQF_HAS_PASSWORD);

								if (pReactorTokenMgntEvent->reactorTokenMgntEventType == RSSL_RCIMPL_TKET_REISSUE_NO_REFRESH)
									pReactorChannel->channelRole.ommConsumerRole.pLoginRequest->flags |= RDM_LG_RQF_NO_REFRESH;

								/* Send login request to reissue the new access token only when the watchlist is enable. */
								if (pReactorChannel->pWatchlist)
								{
									rsslWatchlistClearProcessMsgOptions(&processOpts);
									processOpts.pChannel = pReactorChannel->reactorChannel.pRsslChannel;
									processOpts.pRdmMsg = (RsslRDMMsg*)pReactorChannel->channelRole.ommConsumerRole.pLoginRequest;

									if ((ret = _reactorSubmitWatchlistMsg(pReactorImpl, pReactorChannel, &processOpts, pError))
										< RSSL_RET_SUCCESS)
									{
										if (_reactorHandleChannelDown(pReactorImpl, pReactorChannel, pError) != RSSL_RET_SUCCESS)
											return RSSL_RET_FAILURE;
									}
								}
							}

							if (pReactorTokenMgntEvent->pAuthTokenEventCallback)
							{
								pReactorTokenMgntEvent->reactorAuthTokenEvent.pReactorAuthTokenInfo = &pTokenSession->tokenInformation;
								pReactorTokenMgntEvent->reactorAuthTokenEvent.pReactorChannel = &pReactorChannel->reactorChannel;
								pReactorTokenMgntEvent->reactorAuthTokenEvent.pError = NULL;
								pReactorTokenMgntEvent->reactorAuthTokenEvent.statusCode = pTokenSession->httpStausCode;
								_reactorSetInCallback(pReactorImpl, RSSL_TRUE);
								cret = (*pReactorTokenMgntEvent->pAuthTokenEventCallback)((RsslReactor*)pReactorImpl, (RsslReactorChannel*)pReactorChannel, &pReactorTokenMgntEvent->reactorAuthTokenEvent);
								_reactorSetInCallback(pReactorImpl, RSSL_FALSE);

								if (cret != RSSL_RC_CRET_SUCCESS)
								{
									rsslSetErrorInfo(pError, RSSL_EIC_FAILURE, RSSL_RET_FAILURE, __FILE__, __LINE__, "Error return code %d from callback.", cret);
									return RSSL_RET_FAILURE;
								}
							}
						}

						if (pTokenSession->sendTokenRequest == 0)
						{
							RsslInt tokenExpiresTime = getCurrentTimeMs(pReactorImpl->ticksPerMsec) + (pTokenSession->tokenInformation.expiresIn * 1000);
							pTokenSession->nextExpiresTime = getCurrentTimeMs(pReactorImpl->ticksPerMsec) + (RsslInt)(((double)pTokenSession->tokenInformation.expiresIn  * pReactorImpl->tokenReissueRatio) * 1000);
							RTR_ATOMIC_SET64(pTokenSession->tokenExpiresTime, tokenExpiresTime);
							RTR_ATOMIC_SET(pTokenSession->sendTokenRequest, 1);
						}

						return RSSL_RET_SUCCESS;
					}
					case RSSL_RCIMPL_TKET_RESP_FAILURE:
					{
						if (pReactorTokenMgntEvent->pAuthTokenEventCallback)
						{
							RsslReactorOAuthCredentialRenewalImpl *pReactorOAuthCredentialRenewalImpl = (RsslReactorOAuthCredentialRenewalImpl*)pReactorTokenMgntEvent->pOAuthCredentialRenewal;
							pReactorTokenMgntEvent->reactorAuthTokenEvent.pReactorAuthTokenInfo = NULL;
							pReactorTokenMgntEvent->reactorAuthTokenEvent.pReactorChannel = pReactorChannel ? &pReactorChannel->reactorChannel : NULL;
							pReactorTokenMgntEvent->reactorAuthTokenEvent.pError = &pReactorTokenMgntEvent->pReactorErrorInfoImpl->rsslErrorInfo;
							pReactorTokenMgntEvent->reactorAuthTokenEvent.statusCode = pReactorChannel ? pTokenSession->httpStausCode : pReactorOAuthCredentialRenewalImpl->httpStatusCode;
							_reactorSetInCallback(pReactorImpl, RSSL_TRUE);
							cret = (*pReactorTokenMgntEvent->pAuthTokenEventCallback)((RsslReactor*)pReactorImpl, (RsslReactorChannel*)pReactorChannel, &pReactorTokenMgntEvent->reactorAuthTokenEvent);
							_reactorSetInCallback(pReactorImpl, RSSL_FALSE);

							/* Decrease the reference count of RsslReactorErrorInfoImpl before putting it back to the pool. */
							if (--pReactorTokenMgntEvent->pReactorErrorInfoImpl->referenceCount == 0)
							{
								rsslReactorReturnErrorInfoToPool(pReactorTokenMgntEvent->pReactorErrorInfoImpl, &pReactorImpl->reactorWorker);
							}

							if (cret != RSSL_RC_CRET_SUCCESS)
							{
								rsslSetErrorInfo(pError, RSSL_EIC_FAILURE, RSSL_RET_FAILURE, __FILE__, __LINE__, "Error return code %d from callback.", cret);
								return RSSL_RET_FAILURE;
							}
						}

						return RSSL_RET_SUCCESS;
					}
					case RSSL_RCIMPL_TKET_CHANNEL_WARNING:
					{
						RsslChannel* pRsslChannel = pReactorChannel->reactorChannel.pRsslChannel;
						if (pRsslChannel && pRsslChannel->state == RSSL_CH_STATE_ACTIVE)
						{
							RsslReactorEventImpl rsslEvent;
							RsslReactorChannel* pCallbackChannel = (RsslReactorChannel*)pReactorChannel;

							if (_reactorHandlesWarmStandby(pReactorChannel))
							{
								pCallbackChannel = &pReactorChannel->pWarmStandByHandlerImpl->mainReactorChannelImpl.reactorChannel;
								pCallbackChannel->pRsslChannel = pReactorChannel->reactorChannel.pRsslChannel;
								pCallbackChannel->socketId = pReactorChannel->reactorChannel.socketId;
								pCallbackChannel->oldSocketId = pReactorChannel->reactorChannel.oldSocketId;
								pCallbackChannel->majorVersion = pReactorChannel->reactorChannel.majorVersion;
								pCallbackChannel->minorVersion = pReactorChannel->reactorChannel.minorVersion;
								pCallbackChannel->protocolType = pReactorChannel->reactorChannel.protocolType;
								pCallbackChannel->userSpecPtr = pReactorChannel->reactorChannel.userSpecPtr;
							}

							rsslClearReactorChannelEventImpl(&rsslEvent.channelEventImpl);
							rsslEvent.channelEventImpl.channelEvent.channelEventType = RSSL_RC_CET_WARNING;
							rsslEvent.channelEventImpl.channelEvent.pReactorChannel = pCallbackChannel;
							rsslEvent.channelEventImpl.channelEvent.pError = &pReactorTokenMgntEvent->pReactorErrorInfoImpl->rsslErrorInfo;

							_reactorSetInCallback(pReactorImpl, RSSL_TRUE);
							(*pReactorChannel->channelRole.base.channelEventCallback)((RsslReactor*)pReactorImpl, pCallbackChannel, &rsslEvent.channelEventImpl.channelEvent);
							_reactorSetInCallback(pReactorImpl, RSSL_FALSE);

							rsslEvent.channelEventImpl.channelEvent.pReactorChannel = (RsslReactorChannel*)pReactorChannel;

							/* Decrease the reference count of RsslReactorErrorInfoImpl before putting it back to the pool. */
							if (--pReactorTokenMgntEvent->pReactorErrorInfoImpl->referenceCount == 0)
							{
								rsslReactorReturnErrorInfoToPool(pReactorTokenMgntEvent->pReactorErrorInfoImpl, &pReactorImpl->reactorWorker);
							}
						}

						return RSSL_RET_SUCCESS;
					}
					case RSSL_RCIMPL_TKET_SUBMIT_LOGIN_MSG:
					{
						/* Send login message only when the watchlist is enable. */
						if (pReactorChannel->pWatchlist && pReactorChannel->channelRole.ommConsumerRole.pLoginRequest)
						{
							rsslWatchlistClearProcessMsgOptions(&processOpts);
							processOpts.pChannel = pReactorChannel->reactorChannel.pRsslChannel;
							processOpts.pRdmMsg = (RsslRDMMsg*)pReactorChannel->channelRole.ommConsumerRole.pLoginRequest;

							if ((ret = _reactorSubmitWatchlistMsg(pReactorImpl, pReactorChannel, &processOpts, pError))
								< RSSL_RET_SUCCESS)
							{
								if (_reactorHandleChannelDown(pReactorImpl, pReactorChannel, pError) != RSSL_RET_SUCCESS)
									return RSSL_RET_FAILURE;
							}
						}

						return RSSL_RET_SUCCESS;
					}
					case RSSL_RCIMPL_TKET_RENEW_TOKEN:
					{
						if (pReactorTokenMgntEvent->pAuthTokenEventCallback)
						{
							RsslReactorOAuthCredentialRenewalImpl *pReactorOAuthCredentialRenewalImpl = (RsslReactorOAuthCredentialRenewalImpl*)pReactorTokenMgntEvent->pOAuthCredentialRenewal;
							pReactorTokenMgntEvent->reactorAuthTokenEvent.pReactorAuthTokenInfo = &pReactorOAuthCredentialRenewalImpl->tokenInformation;
							pReactorTokenMgntEvent->reactorAuthTokenEvent.pReactorChannel = (RsslReactorChannel*)pReactorChannel;
							pReactorTokenMgntEvent->reactorAuthTokenEvent.pError = NULL;
							pReactorTokenMgntEvent->reactorAuthTokenEvent.statusCode = pReactorOAuthCredentialRenewalImpl->httpStatusCode;
							_reactorSetInCallback(pReactorImpl, RSSL_TRUE);
							cret = (*pReactorTokenMgntEvent->pAuthTokenEventCallback)((RsslReactor*)pReactorImpl, (RsslReactorChannel*)pReactorChannel, &pReactorTokenMgntEvent->reactorAuthTokenEvent);
							_reactorSetInCallback(pReactorImpl, RSSL_FALSE);

							/* Send a request to free memory for RsslReactorOAuthCredentialRenewalImpl without a token session by the worker */
							if (pTokenSession == 0)
							{
								if (_reactorSendCredentialRenewalRequest(pReactorImpl, NULL, pReactorTokenMgntEvent->pOAuthCredentialRenewal,
									RSSL_RCIMPL_CRET_MEMORY_DEALLOCATION, pError) != RSSL_RET_SUCCESS)
								{
									return RSSL_RET_FAILURE;
								}
							}

							if (cret != RSSL_RC_CRET_SUCCESS)
							{
								rsslSetErrorInfo(pError, RSSL_EIC_FAILURE, RSSL_RET_FAILURE, __FILE__, __LINE__, "Error return code %d from callback.", cret);
								return RSSL_RET_FAILURE;
							}
						}

						return RSSL_RET_SUCCESS;
					}
					default:
						rsslSetErrorInfo(pError, RSSL_EIC_FAILURE, RSSL_RET_FAILURE, __FILE__, __LINE__, "Unknown token management event type: %d", pReactorTokenMgntEvent->reactorTokenMgntEventType);
						
						return RSSL_RET_FAILURE;
				}

			}
			case RSSL_RCIMPL_ET_CREDENTIAL_RENEWAL:
			{
				RsslReactorCredentialRenewalEvent *pReactorCredentialRenewalEvent = (RsslReactorCredentialRenewalEvent*)pEvent;
				RsslReactorTokenSessionImpl *pTokenSession = pReactorCredentialRenewalEvent->pTokenSessionImpl;

				switch (pReactorCredentialRenewalEvent->reactorCredentialRenewalEventType)
				{
					case RSSL_RCIMPL_CRET_RENEWAL_CALLBACK:
					{
						if (pReactorCredentialRenewalEvent->pOAuthCredentialEventCallback)
						{
							RsslReactorOAuthCredentialRenewal reactorOAuthCredentialRenewal;
							rsslClearReactorOAuthCredentialRenewal(&reactorOAuthCredentialRenewal);

							/* Setting the current OAuth credential */
							reactorOAuthCredentialRenewal.userName = pTokenSession->pOAuthCredential->userName;
							reactorOAuthCredentialRenewal.clientId = pTokenSession->pOAuthCredential->clientId;
							reactorOAuthCredentialRenewal.tokenScope = pTokenSession->pOAuthCredential->tokenScope;

							pReactorCredentialRenewalEvent->reactorOAuthCredentialEvent.pReactorChannel = NULL;
							pReactorCredentialRenewalEvent->reactorOAuthCredentialEvent.pReactorOAuthCredentialRenewal = &reactorOAuthCredentialRenewal;
							_reactorSetInCallback(pReactorImpl, RSSL_TRUE);
							pReactorImpl->pTokenSessionForCredentialRenewalCallback = pTokenSession;
							cret = (*pReactorCredentialRenewalEvent->pOAuthCredentialEventCallback)((RsslReactor*)pReactorImpl, &pReactorCredentialRenewalEvent->reactorOAuthCredentialEvent);
							pReactorImpl->pTokenSessionForCredentialRenewalCallback = NULL;
							_reactorSetInCallback(pReactorImpl, RSSL_FALSE);

							if (cret != RSSL_RC_CRET_SUCCESS)
							{
								rsslSetErrorInfo(pError, RSSL_EIC_FAILURE, RSSL_RET_FAILURE, __FILE__, __LINE__, "Error return code %d from callback.", cret);
								return RSSL_RET_FAILURE;
							}
						}

						return RSSL_RET_SUCCESS;
					}
					default:
						rsslSetErrorInfo(pError, RSSL_EIC_FAILURE, RSSL_RET_FAILURE, __FILE__, __LINE__, "Unknown credential renewal event type: %d", pReactorCredentialRenewalEvent->reactorCredentialRenewalEventType);
						return RSSL_RET_FAILURE;
				}

			}
			case RSSL_RCIMPL_ET_PING:
			{
				RsslReactorChannelPingEvent *pReactorChannelPingEvent = (RsslReactorChannelPingEvent*)pEvent;
				RsslReactorChannelImpl *pReactorChannel = (RsslReactorChannelImpl*)pReactorChannelPingEvent->pReactorChannel;

				_cumulativeValue(&pReactorChannel->pChannelStatistic->pingSent, (RsslUInt32)1);

				return RSSL_RET_SUCCESS;
			}
			case RSSL_RCIMPL_ET_TOKEN_SESSION_MGNT:
			{
				RsslReactorTokenSessionEvent *pTokenSessionEvent = (RsslReactorTokenSessionEvent*)pEvent;
				RsslReactorTokenSessionImpl *pTokenSessionImpl = pTokenSessionEvent->pTokenSessionImpl;
				RsslReactorChannelImpl *pReactorChannelImpl = (RsslReactorChannelImpl*)pTokenSessionEvent->pReactorChannel;
				RsslReactorTokenManagementImpl *pReactorTokenMgntImpl = &pReactorImpl->reactorWorker.reactorTokenManagement;

				switch (pTokenSessionEvent->reactorTokenSessionEventType)
				{
					case RSSL_RCIMPL_TSET_RETURN_CHANNEL_TO_CHANNEL_POOL:
					{
						_reactorMoveChannel(&pReactorImpl->channelPool, pReactorChannelImpl);
						--pReactorImpl->channelCount;
						break;
					}
					default:
						rsslSetErrorInfo(pError, RSSL_EIC_FAILURE, RSSL_RET_FAILURE, __FILE__, __LINE__, "Unknown token session event type: %d", pTokenSessionEvent->reactorTokenSessionEventType);
						return RSSL_RET_FAILURE;
				}

				return RSSL_RET_SUCCESS;
			}
			case RSSL_RCIMPL_ET_WARM_STANDBY:
			{
				RsslReactorWarmStanbyEvent* pReactorWarmStanbyEvent = (RsslReactorWarmStanbyEvent*)pEvent;
				RsslReactorChannelImpl* pReactorChannelImpl = (RsslReactorChannelImpl*)pReactorWarmStanbyEvent->pReactorChannel;
				RsslReactorWarmStandByHandlerImpl* pWarmStandByHandlerImpl = NULL;
				RsslReactorWarmStandbyGroupImpl* pWarmStandbyGroupImpl = NULL;

				if (pReactorChannelImpl != NULL)
				{
					pWarmStandByHandlerImpl = pReactorChannelImpl->pWarmStandByHandlerImpl;
					pWarmStandbyGroupImpl = &pReactorChannelImpl->pWarmStandByHandlerImpl->warmStandbyGroupList[pWarmStandByHandlerImpl->currentWSyGroupIndex];
				}

				switch (pReactorWarmStanbyEvent->reactorWarmStandByEventType)
				{
					case RSSL_RCIMPL_WSBET_CHANGE_ACTIVE_TO_STANDBY_SERVER:
					{
						if (pReactorChannelImpl == NULL)
							break;
						
						if (pWarmStandbyGroupImpl->warmStandbyMode == RSSL_RWSB_MODE_LOGIN_BASED && pWarmStandByHandlerImpl->pNextActiveReactorChannel)
						{
							RsslRDMLoginConsumerConnectionStatus consumerConnectionStatus;
							rsslClearRDMLoginConsumerConnectionStatus(&consumerConnectionStatus);
							consumerConnectionStatus.rdmMsgBase.streamId = 1;
							consumerConnectionStatus.flags = RDM_LG_CCSF_HAS_WARM_STANDBY_INFO;
							consumerConnectionStatus.warmStandbyInfo.warmStandbyMode = RDM_LOGIN_SERVER_TYPE_ACTIVE;

							if ((ret = _reactorSendRDMMessage(pReactorImpl, pWarmStandByHandlerImpl->pNextActiveReactorChannel, (RsslRDMMsg*)&consumerConnectionStatus, pError)) < RSSL_RET_SUCCESS)
							{
								if (_reactorHandleChannelDown(pReactorImpl, pWarmStandByHandlerImpl->pNextActiveReactorChannel, pError) != RSSL_RET_SUCCESS)
								{
									return RSSL_RET_FAILURE;
								}

								return RSSL_RET_SUCCESS;
							}

							pWarmStandByHandlerImpl->pNextActiveReactorChannel->isActiveServer = RSSL_TRUE;
							pWarmStandByHandlerImpl->pActiveReactorChannel = pWarmStandByHandlerImpl->pNextActiveReactorChannel;

							_reactorWSNotifyStatusMsg(pWarmStandByHandlerImpl->pNextActiveReactorChannel);
							pWarmStandByHandlerImpl->pNextActiveReactorChannel = NULL;
						}

						break;
					}
					case RSSL_RCIMPL_WSBET_CHANGE_ACTIVE_TO_STANDBY_SERVICE_CHANNEL_DOWN:
					{
						RsslQueueLink *pServiceLink = NULL, *pChannelLink = NULL;
						RsslReactorChannelImpl* pSubmitReactorChannel;
						RsslReactorWarmStandbyServiceImpl *pReactorWarmStandbyServiceImpl = NULL;

						if (pReactorChannelImpl == NULL)
                                                        break;

						if (pWarmStandbyGroupImpl->warmStandbyMode == RSSL_RWSB_MODE_SERVICE_BASED)
						{
							/* Submits to a channel that belongs to the warm standby feature and it is active */
							RSSL_QUEUE_FOR_EACH_LINK(&pWarmStandbyGroupImpl->_serviceList, pServiceLink)
							{
								pReactorWarmStandbyServiceImpl = RSSL_QUEUE_LINK_TO_OBJECT(RsslReactorWarmStandbyServiceImpl, queueLink, pServiceLink);

								if (pReactorWarmStandbyServiceImpl)
								{
									if ((RsslReactorChannelImpl*)pReactorWarmStandbyServiceImpl->pReactorChannel != pReactorChannelImpl)
									{
										continue; /* The channel doesn't provide the service */
									}
									else
									{
										/* Remove this channel from this service. */
										pReactorWarmStandbyServiceImpl->pReactorChannel = NULL;
									}

									RSSL_MUTEX_LOCK(&pWarmStandByHandlerImpl->warmStandByHandlerMutex);
									RSSL_QUEUE_FOR_EACH_LINK(&pWarmStandByHandlerImpl->rsslChannelQueue, pChannelLink)
									{
										pSubmitReactorChannel = RSSL_QUEUE_LINK_TO_OBJECT(RsslReactorChannelImpl, warmstandbyChannelLink, pChannelLink);

										if (pSubmitReactorChannel && pSubmitReactorChannel->reactorChannel.pRsslChannel)
										{
											if (pSubmitReactorChannel != pReactorChannelImpl && pSubmitReactorChannel->reactorChannel.pRsslChannel->state == RSSL_CH_STATE_ACTIVE)
											{
												RsslWatchlistImpl* pWatchlist = (RsslWatchlistImpl*)pSubmitReactorChannel->pWatchlist;
												WlServiceCache* pServiceCache = pWatchlist->base.pServiceCache;

												_reactorWSNotifyStatusMsg(pSubmitReactorChannel);

												if (pServiceCache->_serviceList.count > 0)
												{
													RsslRDMDirectoryConsumerStatus	consumerStatus;
													RsslRDMConsumerStatusService  consumerStatusService;
													RsslQueueLink* pLink = NULL;
													RsslHashLink *pHashLink = NULL;
													RsslReactorWarmStandbyServiceImpl *pActiveWarmStandbyServiceImpl = NULL;
													RDMCachedService* pService = NULL;

													pHashLink = rsslHashTableFind(&pServiceCache->_servicesById, &pReactorWarmStandbyServiceImpl->serviceID, NULL);

													if (pHashLink != NULL)
													{
														pService = RSSL_HASH_LINK_TO_OBJECT(RDMCachedService, _idLink, pHashLink);

														/* Go to next channel if the service is down. */
														if (pService->rdm.state.serviceState == 0)
															continue;

														rsslClearRDMDirectoryConsumerStatus(&consumerStatus);
														rsslClearRDMConsumerStatusService(&consumerStatusService);

														consumerStatusService.flags = RDM_DR_CSSF_HAS_WARM_STANDBY_MODE;
														consumerStatusService.warmStandbyMode = RDM_DIRECTORY_SERVICE_TYPE_ACTIVE;
														consumerStatusService.serviceId = pService->rdm.serviceId;

														consumerStatus.rdmMsgBase.streamId = pWatchlist->directory.pStream->base.streamId;
														consumerStatus.consumerServiceStatusCount = 1;
														consumerStatus.consumerServiceStatusList = &consumerStatusService;

														if ((ret = _reactorSendRDMMessage(pReactorImpl, pSubmitReactorChannel, (RsslRDMMsg*)&consumerStatus, pError)) < RSSL_RET_SUCCESS)
														{
															if (_reactorHandleChannelDown(pReactorImpl, pSubmitReactorChannel, pError) != RSSL_RET_SUCCESS)
															{
																RSSL_MUTEX_UNLOCK(&pWarmStandByHandlerImpl->warmStandByHandlerMutex);
																return RSSL_RET_FAILURE;
															}

															RSSL_MUTEX_UNLOCK(&pWarmStandByHandlerImpl->warmStandByHandlerMutex);
															return RSSL_RET_SUCCESS;
														}

														pReactorWarmStandbyServiceImpl->pReactorChannel = (RsslReactorChannel*)pSubmitReactorChannel;
														pReactorWarmStandbyServiceImpl->serviceID = pService->rdm.serviceId;

													}
												}
											}
										}
									}
									RSSL_MUTEX_UNLOCK(&pWarmStandByHandlerImpl->warmStandByHandlerMutex);
								}
							}
						}

						break;
					}
					case RSSL_RCIMPL_WSBET_ACTIVE_SERVER_SERVICE_STATE_FROM_DOWN_TO_UP:
					{
						RsslQueueLink *pLink;
						RsslReactorChannelImpl *pProcessReactorChannel;

						if (pReactorChannelImpl == NULL)
                                                        break;

						RSSL_MUTEX_LOCK(&pWarmStandByHandlerImpl->warmStandByHandlerMutex);
						RSSL_QUEUE_FOR_EACH_LINK(&pWarmStandByHandlerImpl->rsslChannelQueue, pLink)
						{
							pProcessReactorChannel = RSSL_QUEUE_LINK_TO_OBJECT(RsslReactorChannelImpl, warmstandbyChannelLink, pLink);

							/* Fanout to all standby servers to send pending requests if any. */
							if (pProcessReactorChannel != pReactorChannelImpl)
							{
								if (!pProcessReactorChannel->wlDispatchEventQueued)
								{
									RsslReactorChannelEventImpl *pEvent = (RsslReactorChannelEventImpl*)rsslReactorEventQueueGetFromPool(&pProcessReactorChannel->eventQueue);
									rsslClearReactorChannelEventImpl(pEvent);
									pEvent->channelEvent.channelEventType = (RsslReactorChannelEventType)RSSL_RCIMPL_CET_DISPATCH_WL;
									pEvent->channelEvent.pReactorChannel = (RsslReactorChannel*)pProcessReactorChannel;
									if ((ret = rsslReactorEventQueuePut(&pProcessReactorChannel->eventQueue, (RsslReactorEventImpl*)pEvent)) < RSSL_RET_SUCCESS)
									{
										RSSL_MUTEX_UNLOCK(&pWarmStandByHandlerImpl->warmStandByHandlerMutex);

										if (_reactorHandleChannelDown(pReactorImpl, pReactorChannelImpl, pError) != RSSL_RET_SUCCESS)
											return RSSL_RET_FAILURE;
										return RSSL_RET_SUCCESS;
									}
								}
							}
						}
						RSSL_MUTEX_UNLOCK(&pWarmStandByHandlerImpl->warmStandByHandlerMutex);

						break;
					}
					case RSSL_RCIMPL_WSBET_CONNECT_SECONDARY_SERVER:
					{
						RsslUInt32 index = 0;
						RsslReactorWarmStandbyServerInfoImpl* pReactorWarmStandbyServerInfo = NULL;
						RsslReactorChannelImpl* pStandByReactorChannel = NULL;
						RsslReactorChannelImpl* pStartingReactorChannel = NULL;
						RsslWatchlistImpl *pStartingWatchlistImpl = NULL;

						if (pReactorChannelImpl == NULL)
                                                        break;

						pStartingReactorChannel = pWarmStandByHandlerImpl->pStartingReactorChannel;		
						pStartingWatchlistImpl = (RsslWatchlistImpl*)pStartingReactorChannel->pWatchlist;

						if (pWarmStandbyGroupImpl->warmStandbyMode == RSSL_RWSB_MODE_SERVICE_BASED)
						{
							RsslRDMDirectoryConsumerStatus	consumerStatus;
							RsslRDMConsumerStatusService  consumerStatusService;
							RsslQueueLink* pLink = NULL;
							RsslWatchlistImpl *pWatchlistImpl = (RsslWatchlistImpl*)pReactorChannelImpl->pWatchlist;
							WlServiceCache *pServiceCache = pWatchlistImpl->base.pServiceCache;
							RsslReactorWarmStandbyServiceImpl *pReactorWarmStandbyServiceImpl = NULL;
							RsslHashLink *pHashLink = NULL;
							RsslBool selectAsActiveService = RSSL_FALSE;

							RSSL_QUEUE_FOR_EACH_LINK(&pServiceCache->_serviceList, pLink)
							{
								RDMCachedService* pService = RSSL_QUEUE_LINK_TO_OBJECT(RDMCachedService, _fullListLink,
									pLink);

								selectAsActiveService = provideServiceAtStartUp(pWarmStandbyGroupImpl, &pService->rdm.info.serviceName, pReactorChannelImpl);

								/* Notify the warm standby mode with the active state only when the service state is up. */
								if (pService->rdm.state.serviceState == 1 && selectAsActiveService)
								{
									rsslClearRDMDirectoryConsumerStatus(&consumerStatus);
									rsslClearRDMConsumerStatusService(&consumerStatusService);

									consumerStatusService.flags = RDM_DR_CSSF_HAS_WARM_STANDBY_MODE;
									consumerStatusService.warmStandbyMode = RDM_DIRECTORY_SERVICE_TYPE_ACTIVE;
									consumerStatusService.serviceId = pService->rdm.serviceId;

									consumerStatus.rdmMsgBase.streamId = pReactorWarmStanbyEvent->streamID;
									consumerStatus.consumerServiceStatusCount = 1;
									consumerStatus.consumerServiceStatusList = &consumerStatusService;

									if ((ret = _reactorSendRDMMessage(pReactorImpl, pReactorChannelImpl, (RsslRDMMsg*)&consumerStatus, pError)) < RSSL_RET_SUCCESS)
									{
										if (_reactorHandleChannelDown(pReactorImpl, pReactorChannelImpl, pError) != RSSL_RET_SUCCESS)
											return RSSL_RET_FAILURE;
										return RSSL_RET_SUCCESS;
									}

									pHashLink = rsslHashTableFind(&pWarmStandbyGroupImpl->_perServiceById, &pService->rdm.serviceId, NULL);

									if (pHashLink)
									{
										pReactorWarmStandbyServiceImpl = RSSL_HASH_LINK_TO_OBJECT(RsslReactorWarmStandbyServiceImpl, hashLink, pHashLink);

										/* Assigned this channel to handle this service as primary channel. */
										pReactorWarmStandbyServiceImpl->pReactorChannel = (RsslReactorChannel*)pReactorChannelImpl;
									}
									else
									{
										pReactorWarmStandbyServiceImpl = (RsslReactorWarmStandbyServiceImpl*)malloc(sizeof(RsslReactorWarmStandbyServiceImpl));

										if (!pReactorWarmStandbyServiceImpl)
										{
											return RSSL_RET_FAILURE;
										}

										rsslClearReactorWarmStandbyServiceImpl(pReactorWarmStandbyServiceImpl);

										/* Assigned this channel to handle this service as primary channel. */
										pReactorWarmStandbyServiceImpl->pReactorChannel = (RsslReactorChannel*)pReactorChannelImpl;
										pReactorWarmStandbyServiceImpl->serviceID = pService->rdm.serviceId;

										rsslQueueAddLinkToBack(&pWarmStandbyGroupImpl->_serviceList, &pReactorWarmStandbyServiceImpl->queueLink);

										rsslHashTableInsertLink(&pWarmStandbyGroupImpl->_perServiceById,
											&pReactorWarmStandbyServiceImpl->hashLink, &pService->rdm.serviceId, NULL);
									}
								}
								else
								{
									rsslClearRDMDirectoryConsumerStatus(&consumerStatus);
									rsslClearRDMConsumerStatusService(&consumerStatusService);

									consumerStatusService.flags = RDM_DR_CSSF_HAS_WARM_STANDBY_MODE;
									consumerStatusService.warmStandbyMode = RDM_DIRECTORY_SERVICE_TYPE_STANDBY;
									consumerStatusService.serviceId = pService->rdm.serviceId;

									consumerStatus.rdmMsgBase.streamId = pReactorWarmStanbyEvent->streamID;
									consumerStatus.consumerServiceStatusCount = 1;
									consumerStatus.consumerServiceStatusList = &consumerStatusService;

									if ((ret = _reactorSendRDMMessage(pReactorImpl, pReactorChannelImpl, (RsslRDMMsg*)&consumerStatus, pError)) < RSSL_RET_SUCCESS)
									{
										if (_reactorHandleChannelDown(pReactorImpl, pReactorChannelImpl, pError) != RSSL_RET_SUCCESS)
											return RSSL_RET_FAILURE;
										return RSSL_RET_SUCCESS;
									}
								}
							}
						}

						/* Checks whether all servers has been initiated a connection. */
						if (pWarmStandByHandlerImpl->rsslChannelQueue.count == pWarmStandbyGroupImpl->standbyServerCount + 1)
						{
							break;
						}

						for (; index < pWarmStandbyGroupImpl->standbyServerCount; index++)
						{
							pReactorWarmStandbyServerInfo = &pWarmStandbyGroupImpl->standbyServerList[index];

							pStandByReactorChannel = _reactorTakeChannel(pReactorImpl, &pReactorImpl->channelPool);

							rsslResetReactorChannel(pReactorImpl, pStandByReactorChannel);

							pStandByReactorChannel->connectionListIter = 0;
							pStandByReactorChannel->initializationTimeout = pReactorWarmStandbyServerInfo->reactorConnectInfoImpl.base.initializationTimeout;
							pStandByReactorChannel->reactorChannel.pRsslChannel = NULL;
							pStandByReactorChannel->reactorChannel.pRsslServer = NULL;
							pStandByReactorChannel->reactorChannel.userSpecPtr = pStartingReactorChannel->reactorChannel.userSpecPtr;
							pStandByReactorChannel->reactorChannel.userSpecPtr = pReactorWarmStandbyServerInfo->reactorConnectInfoImpl.base.rsslConnectOptions.userSpecPtr;
							pStandByReactorChannel->readRet = 0;
							pStandByReactorChannel->connectionDebugFlags = pStartingReactorChannel->connectionDebugFlags;
							pStandByReactorChannel->reconnectAttemptLimit = pStartingReactorChannel->reconnectAttemptLimit;
							pStandByReactorChannel->reconnectMinDelay = pStartingReactorChannel->reconnectMinDelay;
							pStandByReactorChannel->reconnectMaxDelay = pStartingReactorChannel->reconnectMaxDelay;
							pStandByReactorChannel->reconnectAttemptCount = 0;
							pStandByReactorChannel->lastReconnectAttemptMs = 0;
							pStandByReactorChannel->standByServerListIndex = index;
							pStandByReactorChannel->pTokenSessionImpl = pStartingReactorChannel->pTokenSessionImpl;

							/* Copies the same channel role from the active server */
							if (_reactorChannelCopyRoleForWarmStandBy(pStandByReactorChannel, &pStartingReactorChannel->channelRole, pError) != RSSL_RET_SUCCESS)
							{
								rsslSetErrorInfo(pError, RSSL_EIC_FAILURE, RSSL_RET_FAILURE, __FILE__, __LINE__, "Failed to copy channel rolre for the standby server");
								return RSSL_RET_FAILURE;
							}

							if (pStandByReactorChannel->channelRole.base.roleType == RSSL_RC_RT_OMM_CONSUMER
								&& pStandByReactorChannel->channelRole.ommConsumerRole.watchlistOptions.enableWatchlist)
							{
								RsslWatchlist* pWatchlist = NULL;
								RsslWatchlistCreateOptions watchlistCreateOpts;
								rsslWatchlistClearCreateOptions(&watchlistCreateOpts);
								watchlistCreateOpts.msgCallback = _reactorWatchlistMsgCallback;
								watchlistCreateOpts.itemCountHint = pStandByReactorChannel->channelRole.ommConsumerRole.watchlistOptions.itemCountHint;
								watchlistCreateOpts.obeyOpenWindow = pStandByReactorChannel->channelRole.ommConsumerRole.watchlistOptions.obeyOpenWindow;
								watchlistCreateOpts.maxOutstandingPosts = pStandByReactorChannel->channelRole.ommConsumerRole.watchlistOptions.maxOutstandingPosts;
								watchlistCreateOpts.postAckTimeout = pStandByReactorChannel->channelRole.ommConsumerRole.watchlistOptions.postAckTimeout;
								watchlistCreateOpts.requestTimeout = pStandByReactorChannel->channelRole.ommConsumerRole.watchlistOptions.requestTimeout;
								watchlistCreateOpts.ticksPerMsec = pReactorImpl->ticksPerMsec;
								watchlistCreateOpts.enableWarmStandby = RSSL_TRUE;
								watchlistCreateOpts.loginRequestCount = 1;
								pWatchlist = rsslWatchlistCreate(&watchlistCreateOpts, pError);
								if (!pWatchlist)
								{
									rsslSetErrorInfo(pError, RSSL_EIC_FAILURE, RSSL_RET_FAILURE, __FILE__, __LINE__, "Failed to create the watchlist component for the standby server");
									return RSSL_RET_FAILURE;
								}

								pStandByReactorChannel->pWatchlist = pWatchlist;
								pStandByReactorChannel->pWarmStandByHandlerImpl = pWarmStandByHandlerImpl;
								RSSL_MUTEX_LOCK(&pWarmStandByHandlerImpl->warmStandByHandlerMutex);
								rsslQueueAddLinkToBack(&pWarmStandByHandlerImpl->rsslChannelQueue, &pStandByReactorChannel->warmstandbyChannelLink);
								RSSL_MUTEX_UNLOCK(&pWarmStandByHandlerImpl->warmStandByHandlerMutex);
								pStandByReactorChannel->isActiveServer = RSSL_FALSE;

								if (pReactorWarmStandbyServerInfo->reactorConnectInfoImpl.base.enableSessionManagement)
								{
									if (_reactorGetAccessToken(pStandByReactorChannel, &pReactorWarmStandbyServerInfo->reactorConnectInfoImpl, pError) != RSSL_RET_SUCCESS)
									{
										if (_reactorHandleChannelDown(pReactorImpl, pStandByReactorChannel, pError) != RSSL_RET_SUCCESS)
											return RSSL_RET_FAILURE;
										return RSSL_RET_SUCCESS;
									}
								}

								if(pWatchlist)
								{
									RsslWatchlistProcessMsgOptions processOpts;
									RsslConnectOptions* pConnOptions;
									RsslChannel* pChannel;

									pWatchlist->pUserSpec = pStandByReactorChannel;

									/* Add consumer requests, if provided. */
									if (pStandByReactorChannel->channelRole.ommConsumerRole.pLoginRequest)
									{
										rsslWatchlistClearProcessMsgOptions(&processOpts);
										processOpts.pChannel = pStandByReactorChannel->reactorChannel.pRsslChannel;
										processOpts.pRdmMsg = (RsslRDMMsg*)pStandByReactorChannel->channelRole.ommConsumerRole.pLoginRequest;

										if ((ret = _reactorSubmitWatchlistMsg(pReactorImpl, pStandByReactorChannel, &processOpts, pError))
											< RSSL_RET_SUCCESS)
										{
											if (_reactorHandleChannelDown(pReactorImpl, pStandByReactorChannel, pError) != RSSL_RET_SUCCESS)
												return RSSL_RET_FAILURE;
											return RSSL_RET_SUCCESS;
										}
									}
									else if (pStartingWatchlistImpl->login.pRequest[0])
									{
										if (pStartingWatchlistImpl->login.pRequest[0]->pLoginReqMsg)
										{
											rsslWatchlistClearProcessMsgOptions(&processOpts);
											processOpts.pChannel = pStandByReactorChannel->reactorChannel.pRsslChannel;
											processOpts.pRdmMsg = (RsslRDMMsg*)pStartingWatchlistImpl->login.pRequest[0]->pLoginReqMsg;

											if ((ret = _reactorSubmitWatchlistMsg(pReactorImpl, pStandByReactorChannel, &processOpts, pError))
												< RSSL_RET_SUCCESS)
											{
												if (_reactorHandleChannelDown(pReactorImpl, pStandByReactorChannel, pError) != RSSL_RET_SUCCESS)
													return RSSL_RET_FAILURE;
												return RSSL_RET_SUCCESS;
											}
										}
									}

									if (pStandByReactorChannel->channelRole.ommConsumerRole.pDirectoryRequest)
									{
										rsslWatchlistClearProcessMsgOptions(&processOpts);
										processOpts.pChannel = pStandByReactorChannel->reactorChannel.pRsslChannel;
										processOpts.pRdmMsg = (RsslRDMMsg*)pStandByReactorChannel->channelRole.ommConsumerRole.pDirectoryRequest;

										if ((ret = _reactorSubmitWatchlistMsg(pReactorImpl, pStandByReactorChannel, &processOpts, pError))
											< RSSL_RET_SUCCESS)
										{
											if (_reactorHandleChannelDown(pReactorImpl, pStandByReactorChannel, pError) != RSSL_RET_SUCCESS)
												return RSSL_RET_FAILURE;
											return RSSL_RET_SUCCESS;
										}
									}

									if ((ret = _submitQueuedMessages(pWarmStandByHandlerImpl, pWarmStandbyGroupImpl, pStandByReactorChannel, pError)) < RSSL_RET_SUCCESS)
									{
										if (_reactorHandleChannelDown(pReactorImpl, pStandByReactorChannel, pError) != RSSL_RET_SUCCESS)
											return RSSL_RET_FAILURE;
										return RSSL_RET_SUCCESS;
									}

									pConnOptions = &pReactorWarmStandbyServerInfo->reactorConnectInfoImpl.base.rsslConnectOptions;

									if (!(pChannel = rsslConnect(pConnOptions, &pError->rsslError)))
									{

										if (pStandByReactorChannel->reconnectAttemptLimit != 0)
										{
											if (_reactorHandleChannelDown(pReactorImpl, pStandByReactorChannel, pError) != RSSL_RET_SUCCESS)
											{
												return RSSL_RET_FAILURE;
											}
										}
										else
										{
											if (_reactorHandleChannelDown(pReactorImpl, pStandByReactorChannel, pError) != RSSL_RET_SUCCESS)
											{
												return RSSL_RET_FAILURE;
											}
										}

										++pReactorImpl->channelCount;
									}
									else
									{
										pStandByReactorChannel->reactorChannel.pRsslChannel = pChannel;

										if (!RSSL_ERROR_INFO_CHECK((ret = _reactorAddChannel(pReactorImpl, pStandByReactorChannel, pError)) == RSSL_RET_SUCCESS, ret, pError))
										{
											return RSSL_RET_FAILURE;
										}

										/* Set debug callback usage here. */
										if (pStandByReactorChannel->connectionDebugFlags != 0 && !RSSL_ERROR_INFO_CHECK((ret = rsslIoctl(pStandByReactorChannel->reactorChannel.pRsslChannel, RSSL_DEBUG_FLAGS, (void*)&(pStandByReactorChannel->connectionDebugFlags), &(pError->rsslError))) == RSSL_RET_SUCCESS, ret, pError))
										{
											if (_reactorHandleChannelDown(pReactorImpl, pStandByReactorChannel, pError) != RSSL_RET_SUCCESS)
											{
												return RSSL_RET_FAILURE;
											}
										}

										++pReactorImpl->channelCount;
									}
								}
							}
						} /* End for loop. */

						break;
					}
					case RSSL_RCIMPL_WSBET_NOTIFY_STANDBY_SERVICE:
					{
						if (pReactorChannelImpl == NULL)
                                                        break;

						if (pWarmStandbyGroupImpl->warmStandbyMode == RSSL_RWSB_MODE_SERVICE_BASED)
						{
							RsslRDMDirectoryConsumerStatus	consumerStatus;
							RsslRDMConsumerStatusService  consumerStatusService;
							RsslQueueLink* pLink = NULL;
							RsslWatchlistImpl *pWatchlistImpl = (RsslWatchlistImpl*)pReactorChannelImpl->pWatchlist;
							WlServiceCache *pServiceCache = pWatchlistImpl->base.pServiceCache;
							RsslReactorWarmStandbyServiceImpl *pReactorWarmStandbyServiceImpl = NULL;
							RsslHashLink *pHashLink = NULL;
							RsslBool selectAsActiveService = RSSL_FALSE;

							RSSL_QUEUE_FOR_EACH_LINK(&pServiceCache->_serviceList, pLink)
							{
								RDMCachedService* pService = RSSL_QUEUE_LINK_TO_OBJECT(RDMCachedService, _fullListLink,
									pLink);

								pHashLink = rsslHashTableFind(&pWarmStandbyGroupImpl->_perServiceById, &pService->rdm.serviceId, NULL);

								if (pHashLink == NULL)
								{
									selectAsActiveService = provideServiceAtStartUp(pWarmStandbyGroupImpl, &pService->rdm.info.serviceName, pReactorChannelImpl);
								}
								else
								{
									pReactorWarmStandbyServiceImpl = RSSL_HASH_LINK_TO_OBJECT(RsslReactorWarmStandbyServiceImpl, hashLink, pHashLink);

									if (pReactorWarmStandbyServiceImpl->pReactorChannel == NULL)
									{
										selectAsActiveService = provideServiceAtStartUp(pWarmStandbyGroupImpl, &pService->rdm.info.serviceName, pReactorChannelImpl);
									}
									else
									{
										selectAsActiveService = RSSL_FALSE;
									}
								}

								/* Notify the warm standby mode wth ACTIVE state when there is no channel provides it yet. */
								if (pService->rdm.state.serviceState == 1 && selectAsActiveService)
								{
									rsslClearRDMDirectoryConsumerStatus(&consumerStatus);
									rsslClearRDMConsumerStatusService(&consumerStatusService);

									consumerStatusService.flags = RDM_DR_CSSF_HAS_WARM_STANDBY_MODE;
									consumerStatusService.warmStandbyMode = RDM_DIRECTORY_SERVICE_TYPE_ACTIVE;
									consumerStatusService.serviceId = pService->rdm.serviceId;

									consumerStatus.rdmMsgBase.streamId = pReactorWarmStanbyEvent->streamID;
									consumerStatus.consumerServiceStatusCount = 1;
									consumerStatus.consumerServiceStatusList = &consumerStatusService;

									if ((ret = _reactorSendRDMMessage(pReactorImpl, pReactorChannelImpl, (RsslRDMMsg*)&consumerStatus, pError)) < RSSL_RET_SUCCESS)
									{
										if (_reactorHandleChannelDown(pReactorImpl, pReactorChannelImpl, pError) != RSSL_RET_SUCCESS)
											return RSSL_RET_FAILURE;
										return RSSL_RET_SUCCESS;
									}

									if (pHashLink)
									{
										pReactorWarmStandbyServiceImpl = RSSL_HASH_LINK_TO_OBJECT(RsslReactorWarmStandbyServiceImpl, hashLink, pHashLink);

										/* Assigned this channel to handle this service as primary channel. */
										pReactorWarmStandbyServiceImpl->pReactorChannel = (RsslReactorChannel*)pReactorChannelImpl;
									}
									else
									{
										pReactorWarmStandbyServiceImpl = (RsslReactorWarmStandbyServiceImpl*)malloc(sizeof(RsslReactorWarmStandbyServiceImpl));

										if (!pReactorWarmStandbyServiceImpl)
										{
											return RSSL_RET_FAILURE;
										}

										rsslClearReactorWarmStandbyServiceImpl(pReactorWarmStandbyServiceImpl);

										pReactorWarmStandbyServiceImpl->pReactorChannel = (RsslReactorChannel*)pReactorChannelImpl;
										pReactorWarmStandbyServiceImpl->serviceID = pService->rdm.serviceId;

										rsslQueueAddLinkToBack(&pWarmStandbyGroupImpl->_serviceList, &pReactorWarmStandbyServiceImpl->queueLink);

										rsslHashTableInsertLink(&pWarmStandbyGroupImpl->_perServiceById,
											&pReactorWarmStandbyServiceImpl->hashLink, &pService->rdm.serviceId, NULL);
									}
								}
								else
								{
									/* This channel is already set as active channel for this service. */
									if (pReactorWarmStandbyServiceImpl->pReactorChannel == (RsslReactorChannel*)pReactorChannelImpl)
										continue;

									rsslClearRDMDirectoryConsumerStatus(&consumerStatus);
									rsslClearRDMConsumerStatusService(&consumerStatusService);

									consumerStatusService.flags = RDM_DR_CSSF_HAS_WARM_STANDBY_MODE;
									consumerStatusService.warmStandbyMode = RDM_DIRECTORY_SERVICE_TYPE_STANDBY;
									consumerStatusService.serviceId = pService->rdm.serviceId;

									consumerStatus.rdmMsgBase.streamId = pReactorWarmStanbyEvent->streamID;
									consumerStatus.consumerServiceStatusCount = 1;
									consumerStatus.consumerServiceStatusList = &consumerStatusService;

									if ((ret = _reactorSendRDMMessage(pReactorImpl, pReactorChannelImpl, (RsslRDMMsg*)&consumerStatus, pError)) < RSSL_RET_SUCCESS)
									{
										if (_reactorHandleChannelDown(pReactorImpl, pReactorChannelImpl, pError) != RSSL_RET_SUCCESS)
											return RSSL_RET_FAILURE;
										return RSSL_RET_SUCCESS;
									}
								}
							}
						}

						if ((ret = _submitQueuedMessages(pWarmStandByHandlerImpl, pWarmStandbyGroupImpl, pReactorChannelImpl, pError)) < RSSL_RET_SUCCESS)
							return RSSL_FALSE;

						break;
					}
					case RSSL_RCIMPL_WSBET_CHANGE_ACTIVE_TO_STANDBY_PER_SERVICE:
					{
						RsslQueueLink* pLink = NULL;
						RsslReactorChannelImpl* pSubmitReactorChannel;
						RsslWatchlistImpl *pWatchlistImpl = NULL;
						WlServiceCache *pServiceCache = NULL;
						RsslHashLink *pHashLink = NULL;
						RDMCachedService *pService = NULL;
						RsslReactorWarmStandbyServiceImpl *pReactorWarmStandbyServiceImpl = NULL;
						RsslRDMDirectoryConsumerStatus	consumerStatus;
						RsslRDMConsumerStatusService  consumerStatusService;

						if (pReactorChannelImpl == NULL)
                                                        break;

						pReactorWarmStandbyServiceImpl = RSSL_HASH_LINK_TO_OBJECT(RsslReactorWarmStandbyServiceImpl, hashLink, pReactorWarmStanbyEvent->pHashLink);

						RSSL_MUTEX_LOCK(&pWarmStandByHandlerImpl->warmStandByHandlerMutex);
						RSSL_QUEUE_FOR_EACH_LINK(&pWarmStandByHandlerImpl->rsslChannelQueue, pLink)
						{
							pSubmitReactorChannel = RSSL_QUEUE_LINK_TO_OBJECT(RsslReactorChannelImpl, warmstandbyChannelLink, pLink);

							if (pReactorChannelImpl != pSubmitReactorChannel)
							{
								pWatchlistImpl = (RsslWatchlistImpl*)pSubmitReactorChannel->pWatchlist;
								pServiceCache = pWatchlistImpl->base.pServiceCache;

								pHashLink = rsslHashTableFind(&pServiceCache->_servicesById, &pReactorWarmStanbyEvent->serviceID, NULL);

								if (!pHashLink)
									continue;

								pService = RSSL_HASH_LINK_TO_OBJECT(RDMCachedService, _idLink, pHashLink);

								if (pService->rdm.state.serviceState == 0)
									continue;

								/* Notify the warm standby mode only when the service state is up. */
								if (pService->rdm.state.serviceState == 1)
								{
									rsslClearRDMDirectoryConsumerStatus(&consumerStatus);
									rsslClearRDMConsumerStatusService(&consumerStatusService);

									consumerStatusService.flags = RDM_DR_CSSF_HAS_WARM_STANDBY_MODE;
									consumerStatusService.warmStandbyMode = RDM_DIRECTORY_SERVICE_TYPE_ACTIVE;
									consumerStatusService.serviceId = pService->rdm.serviceId;

									consumerStatus.rdmMsgBase.streamId = pWatchlistImpl->directory.pStream->base.streamId;
									consumerStatus.consumerServiceStatusCount = 1;
									consumerStatus.consumerServiceStatusList = &consumerStatusService;

									if ((ret = _reactorSendRDMMessage(pReactorImpl, pSubmitReactorChannel, (RsslRDMMsg*)&consumerStatus, pError)) < RSSL_RET_SUCCESS)
									{
										if (_reactorHandleChannelDown(pReactorImpl, pSubmitReactorChannel, pError) != RSSL_RET_SUCCESS)
										{
											RSSL_MUTEX_UNLOCK(&pWarmStandByHandlerImpl->warmStandByHandlerMutex);
											return RSSL_RET_FAILURE;
										}

										RSSL_MUTEX_UNLOCK(&pWarmStandByHandlerImpl->warmStandByHandlerMutex);
										return RSSL_RET_SUCCESS;
									}


									pReactorWarmStandbyServiceImpl->pReactorChannel = (RsslReactorChannel*)pSubmitReactorChannel;
									pReactorWarmStandbyServiceImpl->serviceID = pService->rdm.serviceId;

									break;
								}
							}
							else
							{
								/* Changes the warm standby mode for the service of the current channel to standby. */
								rsslClearRDMDirectoryConsumerStatus(&consumerStatus);
								rsslClearRDMConsumerStatusService(&consumerStatusService);

								consumerStatusService.flags = RDM_DR_CSSF_HAS_WARM_STANDBY_MODE;
								consumerStatusService.warmStandbyMode = RDM_DIRECTORY_SERVICE_TYPE_STANDBY;
								consumerStatusService.serviceId = pReactorWarmStanbyEvent->serviceID;

								consumerStatus.rdmMsgBase.streamId = pReactorWarmStanbyEvent->streamID;
								consumerStatus.consumerServiceStatusCount = 1;
								consumerStatus.consumerServiceStatusList = &consumerStatusService;

								/* Submits the warm standby mode from active to standby. */
								if ((ret = _reactorSendRDMMessage(pReactorImpl, pReactorChannelImpl, (RsslRDMMsg*)&consumerStatus, pError)) < RSSL_RET_SUCCESS)
								{
									RSSL_MUTEX_UNLOCK(&pWarmStandByHandlerImpl->warmStandByHandlerMutex);

									if (_reactorHandleChannelDown(pReactorImpl, pReactorChannelImpl, pError) != RSSL_RET_SUCCESS)
										return RSSL_RET_FAILURE;
									return RSSL_RET_SUCCESS;
								}

								/* Removes this channel from this service. */
								pReactorWarmStandbyServiceImpl->pReactorChannel = NULL;
							}
						}
						RSSL_MUTEX_UNLOCK(&pWarmStandByHandlerImpl->warmStandByHandlerMutex);

						break;
					}
					case RSSL_RCIMPL_WSBET_CHANGE_STANDBY_TO_ACTIVE_PER_SERVICE:
					{
						RsslRDMDirectoryConsumerStatus	consumerStatus;
						RsslRDMConsumerStatusService  consumerStatusService;
						RsslReactorWarmStandbyServiceImpl* pReactorWarmStandbyServiceImpl = NULL;

						if (pReactorChannelImpl == NULL)
                                                        break;
							
						if (pReactorWarmStanbyEvent->pHashLink != NULL)
						{
							pReactorWarmStandbyServiceImpl = RSSL_HASH_LINK_TO_OBJECT(RsslReactorWarmStandbyServiceImpl, hashLink, pReactorWarmStanbyEvent->pHashLink);

							rsslClearRDMDirectoryConsumerStatus(&consumerStatus);
							rsslClearRDMConsumerStatusService(&consumerStatusService);

							consumerStatusService.flags = RDM_DR_CSSF_HAS_WARM_STANDBY_MODE;
							consumerStatusService.warmStandbyMode = RDM_DIRECTORY_SERVICE_TYPE_ACTIVE;
							consumerStatusService.serviceId = pReactorWarmStanbyEvent->serviceID;

							consumerStatus.rdmMsgBase.streamId = pReactorWarmStanbyEvent->streamID;
							consumerStatus.consumerServiceStatusCount = 1;
							consumerStatus.consumerServiceStatusList = &consumerStatusService;

							/* Submits the warm standby mode from standby to active. */
							if ((ret = _reactorSendRDMMessage(pReactorImpl, pReactorChannelImpl, (RsslRDMMsg*)&consumerStatus, pError)) < RSSL_RET_SUCCESS)
							{
								if (_reactorHandleChannelDown(pReactorImpl, pReactorChannelImpl, pError) != RSSL_RET_SUCCESS)
									return RSSL_RET_FAILURE;
								return RSSL_RET_SUCCESS;
							}

							pReactorWarmStandbyServiceImpl->pReactorChannel = (RsslReactorChannel*)pReactorChannelImpl;
							pReactorWarmStandbyServiceImpl->serviceID = pReactorWarmStanbyEvent->serviceID;
						}

						break;
					}
					case RSSL_RCIMPL_WSBET_REMOVE_SERVER_FROM_WSB_GROUP:
					{
						RsslReactorChannel *pCallbackChannel;
						RsslReactorChannelEvent channelEvent;

						if (pReactorChannelImpl == NULL)
                                                        break;

						rsslClearReactorChannelEvent(&channelEvent);

						/* Marks this channel info as inactive to remove from the recovery list. */
						if (pReactorChannelImpl->isStartingServerConfig)
						{
							pWarmStandbyGroupImpl->startingActiveServer.isActiveChannelConfig = RSSL_FALSE;
						}
						else
						{
							pWarmStandbyGroupImpl->standbyServerList[pReactorChannelImpl->standByServerListIndex].isActiveChannelConfig = RSSL_FALSE;
						}

						pCallbackChannel = &pReactorChannelImpl->pWarmStandByHandlerImpl->mainReactorChannelImpl.reactorChannel;
						pCallbackChannel->pRsslChannel = pReactorChannelImpl->reactorChannel.pRsslChannel;
						pCallbackChannel->socketId = pReactorChannelImpl->reactorChannel.socketId;
						pCallbackChannel->oldSocketId = pReactorChannelImpl->reactorChannel.oldSocketId;
						pCallbackChannel->majorVersion = pReactorChannelImpl->reactorChannel.majorVersion;
						pCallbackChannel->minorVersion = pReactorChannelImpl->reactorChannel.minorVersion;
						pCallbackChannel->protocolType = pReactorChannelImpl->reactorChannel.protocolType;
						pCallbackChannel->userSpecPtr = pReactorChannelImpl->reactorChannel.userSpecPtr;

						_reactorHandlesWSBSocketList(pReactorChannelImpl->pWarmStandByHandlerImpl, pCallbackChannel, &pReactorChannelImpl->reactorChannel.socketId);

						channelEvent.pReactorChannel = pCallbackChannel;

						/* Sends the channel down event if this is the only channel */
						RSSL_MUTEX_LOCK(&pWarmStandByHandlerImpl->warmStandByHandlerMutex);
						channelEvent.channelEventType = pWarmStandByHandlerImpl->rsslChannelQueue.count == 1 ? RSSL_RC_CET_CHANNEL_DOWN : RSSL_RC_CET_FD_CHANGE;
						RSSL_MUTEX_UNLOCK(&pWarmStandByHandlerImpl->warmStandByHandlerMutex);
						
						if (pReactorWarmStanbyEvent->pReactorErrorInfoImpl)
						{
							channelEvent.pError = &pReactorWarmStanbyEvent->pReactorErrorInfoImpl->rsslErrorInfo;
						}

						/* Notify application */
						_reactorSetInCallback(pReactorImpl, RSSL_TRUE);
						(*pReactorChannelImpl->channelRole.base.channelEventCallback)((RsslReactor*)pReactorImpl, pCallbackChannel, &channelEvent);
						_reactorSetInCallback(pReactorImpl, RSSL_FALSE);

						if (pReactorWarmStanbyEvent->pReactorErrorInfoImpl)
						{
							rsslReactorReturnErrorInfoToPool(pReactorWarmStanbyEvent->pReactorErrorInfoImpl, &pReactorImpl->reactorWorker);
						}

						if (_processingReactorChannelShutdown(pReactorImpl, pReactorChannelImpl, pError) != RSSL_RET_SUCCESS)
						{
							return RSSL_RET_FAILURE;
						}

						break;
					}
					case RSSL_RCIMPL_WSBET_CONNECT_TO_NEXT_STARTING_SERVER:
					{
						RsslReactorWarmStandbyServerInfoImpl* pReactorWarmStandbyServerInfo = NULL;
						RsslReactorChannelImpl* pNextReactorChannel = NULL;
						RsslReactorChannelImpl* pStartingReactorChannel = NULL;

						if (pReactorChannelImpl == NULL)
                                                        break;

						pStartingReactorChannel = pWarmStandByHandlerImpl->pStartingReactorChannel;

						if (pWarmStandByHandlerImpl->warmStandByHandlerState != RSSL_RWSB_STATE_CONNECTING_TO_A_STARTING_SERVER)
						{
							/* Do nothing as this group is already connected to a server. */
							break;
						}

						if ( (pWarmStandbyGroupImpl->currentStartingServerIndex + 1) == pWarmStandbyGroupImpl->standbyServerCount)
						{
							/* Do nothing as all servers are connected within this group */
							break;
						}
						else
						{
							++pWarmStandbyGroupImpl->currentStartingServerIndex;
							pReactorWarmStandbyServerInfo = &pWarmStandbyGroupImpl->standbyServerList[pWarmStandbyGroupImpl->currentStartingServerIndex];
						}

						pNextReactorChannel = _reactorTakeChannel(pReactorImpl, &pReactorImpl->channelPool);

						rsslResetReactorChannel(pReactorImpl, pNextReactorChannel);

						pNextReactorChannel->connectionListIter = 0;
						pNextReactorChannel->initializationTimeout = pReactorWarmStandbyServerInfo->reactorConnectInfoImpl.base.initializationTimeout;
						pNextReactorChannel->reactorChannel.pRsslChannel = NULL;
						pNextReactorChannel->reactorChannel.pRsslServer = NULL;
						pNextReactorChannel->reactorChannel.userSpecPtr = pStartingReactorChannel->reactorChannel.userSpecPtr;
						pNextReactorChannel->reactorChannel.userSpecPtr = pReactorWarmStandbyServerInfo->reactorConnectInfoImpl.base.rsslConnectOptions.userSpecPtr;
						pNextReactorChannel->readRet = 0;
						pNextReactorChannel->connectionDebugFlags = pStartingReactorChannel->connectionDebugFlags;
						pNextReactorChannel->reconnectAttemptLimit = pStartingReactorChannel->reconnectAttemptLimit;
						pNextReactorChannel->reconnectMinDelay = pStartingReactorChannel->reconnectMinDelay;
						pNextReactorChannel->reconnectMaxDelay = pStartingReactorChannel->reconnectMaxDelay;
						pNextReactorChannel->reconnectAttemptCount = 0;
						pNextReactorChannel->lastReconnectAttemptMs = 0;
						pNextReactorChannel->standByServerListIndex = (RsslUInt32)pWarmStandbyGroupImpl->currentStartingServerIndex;

						/* Copies the same channel role from the active server */
						if (_reactorChannelCopyRoleForWarmStandBy(pNextReactorChannel, &pStartingReactorChannel->channelRole, pError) != RSSL_RET_SUCCESS)
						{
							rsslSetErrorInfo(pError, RSSL_EIC_FAILURE, RSSL_RET_FAILURE, __FILE__, __LINE__, "Failed to copy channel rolre for the standby server");
							return RSSL_RET_FAILURE;
						}

						if (pNextReactorChannel->channelRole.base.roleType == RSSL_RC_RT_OMM_CONSUMER
							&& pNextReactorChannel->channelRole.ommConsumerRole.watchlistOptions.enableWatchlist)
						{
							RsslWatchlist* pWatchlist = NULL;
							RsslWatchlistCreateOptions watchlistCreateOpts;
							rsslWatchlistClearCreateOptions(&watchlistCreateOpts);
							watchlistCreateOpts.msgCallback = _reactorWatchlistMsgCallback;
							watchlistCreateOpts.itemCountHint = pNextReactorChannel->channelRole.ommConsumerRole.watchlistOptions.itemCountHint;
							watchlistCreateOpts.obeyOpenWindow = pNextReactorChannel->channelRole.ommConsumerRole.watchlistOptions.obeyOpenWindow;
							watchlistCreateOpts.maxOutstandingPosts = pNextReactorChannel->channelRole.ommConsumerRole.watchlistOptions.maxOutstandingPosts;
							watchlistCreateOpts.postAckTimeout = pNextReactorChannel->channelRole.ommConsumerRole.watchlistOptions.postAckTimeout;
							watchlistCreateOpts.requestTimeout = pNextReactorChannel->channelRole.ommConsumerRole.watchlistOptions.requestTimeout;
							watchlistCreateOpts.ticksPerMsec = pReactorImpl->ticksPerMsec;
							watchlistCreateOpts.enableWarmStandby = RSSL_TRUE;
							watchlistCreateOpts.loginRequestCount = 1;
							pWatchlist = rsslWatchlistCreate(&watchlistCreateOpts, pError);
							if (!pWatchlist)
							{
								rsslSetErrorInfo(pError, RSSL_EIC_FAILURE, RSSL_RET_FAILURE, __FILE__, __LINE__, "Failed to create the watchlist component for the standby server");
								return RSSL_RET_FAILURE;
							}

							pNextReactorChannel->pWatchlist = pWatchlist;
							pNextReactorChannel->pWarmStandByHandlerImpl = pWarmStandByHandlerImpl;
							RSSL_MUTEX_LOCK(&pWarmStandByHandlerImpl->warmStandByHandlerMutex);
							rsslQueueAddLinkToBack(&pWarmStandByHandlerImpl->rsslChannelQueue, &pNextReactorChannel->warmstandbyChannelLink);
							RSSL_MUTEX_UNLOCK(&pWarmStandByHandlerImpl->warmStandByHandlerMutex);

							/* Keeps the original login request */
							if (pNextReactorChannel->channelRole.ommConsumerRole.pLoginRequest)
							{
								pNextReactorChannel->userName = pNextReactorChannel->channelRole.ommConsumerRole.pLoginRequest->userName;
								pNextReactorChannel->flags = pNextReactorChannel->channelRole.ommConsumerRole.pLoginRequest->flags;
								pNextReactorChannel->userNameType = pNextReactorChannel->channelRole.ommConsumerRole.pLoginRequest->userNameType;
							}

							if (pReactorWarmStandbyServerInfo->reactorConnectInfoImpl.base.enableSessionManagement)
							{
								if (_reactorGetAccessToken(pNextReactorChannel, &pReactorWarmStandbyServerInfo->reactorConnectInfoImpl, pError) != RSSL_RET_SUCCESS)
								{
									if (_reactorHandleChannelDown(pReactorImpl, pNextReactorChannel, pError) != RSSL_RET_SUCCESS)
										return RSSL_RET_FAILURE;
									return RSSL_RET_SUCCESS;
								}
							}

							if (pWatchlist)
							{
								RsslWatchlistProcessMsgOptions processOpts;
								RsslConnectOptions* pConnOptions;
								RsslChannel* pChannel;

								pWatchlist->pUserSpec = pNextReactorChannel;

								/* Add consumer requests, if provided. */
								if (pNextReactorChannel->channelRole.ommConsumerRole.pLoginRequest)
								{

									rsslWatchlistClearProcessMsgOptions(&processOpts);
									processOpts.pChannel = pNextReactorChannel->reactorChannel.pRsslChannel;
									processOpts.pRdmMsg = (RsslRDMMsg*)pNextReactorChannel->channelRole.ommConsumerRole.pLoginRequest;

									if ((ret = _reactorSubmitWatchlistMsg(pReactorImpl, pNextReactorChannel, &processOpts, pError))
										< RSSL_RET_SUCCESS)
									{
										if (_reactorHandleChannelDown(pReactorImpl, pNextReactorChannel, pError) != RSSL_RET_SUCCESS)
											return RSSL_RET_FAILURE;
										return RSSL_RET_SUCCESS;
									}
								}

								if (pNextReactorChannel->channelRole.ommConsumerRole.pDirectoryRequest)
								{
									rsslWatchlistClearProcessMsgOptions(&processOpts);
									processOpts.pChannel = pNextReactorChannel->reactorChannel.pRsslChannel;
									processOpts.pRdmMsg = (RsslRDMMsg*)pNextReactorChannel->channelRole.ommConsumerRole.pDirectoryRequest;

									if ((ret = _reactorSubmitWatchlistMsg(pReactorImpl, pNextReactorChannel, &processOpts, pError))
										< RSSL_RET_SUCCESS)
									{
										if (_reactorHandleChannelDown(pReactorImpl, pNextReactorChannel, pError) != RSSL_RET_SUCCESS)
											return RSSL_RET_FAILURE;
										return RSSL_RET_SUCCESS;
									}
								}

								if ((ret = _submitQueuedMessages(pWarmStandByHandlerImpl, pWarmStandbyGroupImpl, pNextReactorChannel, pError)) < RSSL_RET_SUCCESS)
								{
									if (_reactorHandleChannelDown(pReactorImpl, pNextReactorChannel, pError) != RSSL_RET_SUCCESS)
										return RSSL_RET_FAILURE;
									return RSSL_RET_SUCCESS;
								}

								pConnOptions = &pReactorWarmStandbyServerInfo->reactorConnectInfoImpl.base.rsslConnectOptions;

								if (!(pChannel = rsslConnect(pConnOptions, &pError->rsslError)))
								{

									if (pNextReactorChannel->reconnectAttemptLimit != 0)
									{
										if (_reactorHandleChannelDown(pReactorImpl, pNextReactorChannel, pError) != RSSL_RET_SUCCESS)
										{
											return RSSL_RET_FAILURE;
										}
									}
									else
									{
										if (_reactorHandleChannelDown(pReactorImpl, pNextReactorChannel, pError) != RSSL_RET_SUCCESS)
										{
											return RSSL_RET_FAILURE;
										}
									}

									++pReactorImpl->channelCount;
								}
								else
								{
									pNextReactorChannel->reactorChannel.pRsslChannel = pChannel;

									if (!RSSL_ERROR_INFO_CHECK((ret = _reactorAddChannel(pReactorImpl, pNextReactorChannel, pError)) == RSSL_RET_SUCCESS, ret, pError))
									{
										return RSSL_RET_FAILURE;
									}

									/* Set debug callback usage here. */
									if (pNextReactorChannel->connectionDebugFlags != 0 && !RSSL_ERROR_INFO_CHECK((ret = rsslIoctl(pNextReactorChannel->reactorChannel.pRsslChannel, RSSL_DEBUG_FLAGS,
										(void*)&(pNextReactorChannel->connectionDebugFlags), &(pError->rsslError))) == RSSL_RET_SUCCESS, ret, pError))
									{
										if (_reactorHandleChannelDown(pReactorImpl, pNextReactorChannel, pError) != RSSL_RET_SUCCESS)
										{
											return RSSL_RET_FAILURE;
										}
									}

									++pReactorImpl->channelCount;
								}
							}
						}

						break;
					}
					case RSSL_RCIMPL_WSBET_MOVE_WSB_HANDLER_BACK_TO_POOL:
					{

						RsslReactorWarmStandByHandlerImpl* pReturnWarmStandByHandlerImpl = (RsslReactorWarmStandByHandlerImpl*)pReactorWarmStanbyEvent->pReactorWarmStandByHandlerImpl;

						if(pReturnWarmStandByHandlerImpl != NULL)
						{	
							/* Removes from the closing queue and returns warm standby handler back to pool to reuse. */
							rsslQueueRemoveLink(&pReactorImpl->closingWarmstandbyChannel, &pReturnWarmStandByHandlerImpl->reactorQueueLink);

							rsslQueueAddLinkToBack(&pReactorImpl->warmstandbyChannelPool, &pReturnWarmStandByHandlerImpl->reactorQueueLink);
						}

						break;
					}
					default:
						rsslSetErrorInfo(pError, RSSL_EIC_FAILURE, RSSL_RET_FAILURE, __FILE__, __LINE__, "Unknown warm standby event type: %d", pReactorWarmStanbyEvent->reactorWarmStandByEventType);
						return RSSL_RET_FAILURE;
				}

				return RSSL_RET_SUCCESS;
			}
			case RSSL_RCIMPL_ET_LOGGING:
			{
				RsslReactorLoggingEvent* pLoggingEvent = (RsslReactorLoggingEvent*)pEvent;
				if (pReactorImpl->pRestLoggingCallback)
				{
					RsslReactorRestLoggingEvent restLogEvent;
					rsslClearReactorRestLoggingEvent(&restLogEvent);

					restLogEvent.pUserSpec = pReactorImpl->userSpecPtr;
					restLogEvent.pRestLoggingMessage = pLoggingEvent->pRestLoggingMessage;

					_reactorSetInCallback(pReactorImpl, RSSL_TRUE);
					cret = (*pReactorImpl->pRestLoggingCallback)((RsslReactor*)pReactorImpl, &restLogEvent);
					_reactorSetInCallback(pReactorImpl, RSSL_FALSE);

					if (pLoggingEvent->pRestLoggingMessage)
					{
						if (pLoggingEvent->pRestLoggingMessage->data)
							free(pLoggingEvent->pRestLoggingMessage->data);
						free(pLoggingEvent->pRestLoggingMessage);
					}

					if (cret != RSSL_RC_CRET_SUCCESS)
					{
						rsslSetErrorInfo(pError, RSSL_EIC_FAILURE, RSSL_RET_FAILURE, __FILE__, __LINE__, "Error return code %d from callback.", cret);
						return RSSL_RET_FAILURE;
					}
				}
				return RSSL_RET_SUCCESS;
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
	RsslBool callbackToClient = _warmStandbyCallbackChecks(pReactorChannel, pOpts);
	RsslReactorChannel* pCallbackChannel = (RsslReactorChannel*)pReactorChannel;

	if (callbackToClient)
	{
		rsslClearMsgEvent(&msgEvent);
		msgEvent.pRsslMsgBuffer = pOpts->pMsgBuf;
		msgEvent.pRsslMsg = pOpts->pRsslMsg;
		msgEvent.pStreamInfo = (RsslStreamInfo*)pOpts->pStreamInfo;
		msgEvent.pFTGroupId = pOpts->pFTGroupId;
		msgEvent.pSeqNum = pOpts->pSeqNum;

		if (_reactorHandlesWarmStandby(pReactorChannel))
		{
			pCallbackChannel = &pReactorChannel->pWarmStandByHandlerImpl->mainReactorChannelImpl.reactorChannel;
			pCallbackChannel->pRsslChannel = pReactorChannel->reactorChannel.pRsslChannel;
			pCallbackChannel->socketId = pReactorChannel->reactorChannel.socketId;
			pCallbackChannel->oldSocketId = pReactorChannel->reactorChannel.oldSocketId;
			pCallbackChannel->majorVersion = pReactorChannel->reactorChannel.majorVersion;
			pCallbackChannel->minorVersion = pReactorChannel->reactorChannel.minorVersion;
			pCallbackChannel->protocolType = pReactorChannel->reactorChannel.protocolType;
			pCallbackChannel->userSpecPtr = pReactorChannel->reactorChannel.userSpecPtr;
		}

		_reactorSetInCallback(pReactorImpl, RSSL_TRUE);
		*pOpts->pCret = (*pReactorChannel->channelRole.base.defaultMsgCallback)((RsslReactor*)pReactorImpl, pCallbackChannel, &msgEvent);
		_reactorSetInCallback(pReactorImpl, RSSL_FALSE);

		if (pOpts->wsbStatusFlags & WL_MEF_SEND_CLOSED_RECOVER)
		{
			if (_reactorHandlesWarmStandby(pReactorChannel))
			{
				RsslReactorWarmStandByHandlerImpl *pReactorWarmStandByHandlerImpl = pReactorChannel->pWarmStandByHandlerImpl;
				RsslQueueLink *pLink;
				RsslReactorChannelImpl *pProcessReactorChannel;

				RSSL_MUTEX_LOCK(&pReactorWarmStandByHandlerImpl->warmStandByHandlerMutex);
				RSSL_QUEUE_FOR_EACH_LINK(&pReactorWarmStandByHandlerImpl->rsslChannelQueue, pLink)
				{
					pProcessReactorChannel = RSSL_QUEUE_LINK_TO_OBJECT(RsslReactorChannelImpl, warmstandbyChannelLink, pLink);

					if (pProcessReactorChannel != pReactorChannel && pOpts->pRsslMsg->msgBase.streamId != 0)
					{
						RsslWatchlistProcessMsgOptions processOpts;
						RsslCloseMsg rsslCloseMsg;
						RsslErrorInfo errorInfo;
						rsslWatchlistClearProcessMsgOptions(&processOpts);
						rsslClearCloseMsg(&rsslCloseMsg);
						rsslCloseMsg.msgBase.streamId = pOpts->pRsslMsg->msgBase.streamId;
						processOpts.pChannel = pProcessReactorChannel->reactorChannel.pRsslChannel;
						processOpts.pRsslMsg = (RsslMsg*)&rsslCloseMsg;
						processOpts.majorVersion = pProcessReactorChannel->reactorChannel.majorVersion;
						processOpts.minorVersion = pProcessReactorChannel->reactorChannel.minorVersion;

						_reactorSubmitWatchlistMsg(pProcessReactorChannel->pParentReactor, pProcessReactorChannel, &processOpts, &errorInfo);
					}
				}
				RSSL_MUTEX_UNLOCK(&pReactorWarmStandByHandlerImpl->warmStandByHandlerMutex);
			}
		}
	}
	else
	{
		_reactorWSHandleRecoveryStatusMsg(pReactorChannel, pOpts);
	}
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

							rsslClearRDMLoginMsg(&loginResponse);
							rsslClearRDMLoginMsgEvent(&loginEvent);

						if (!pRdmMsg)
						{
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
							if (pLoginResponse->rdmMsgBase.rdmMsgType == RDM_LG_MT_RTT && (pReactorChannel->flags & RDM_LG_RQF_RTT_SUPPORT))
							{ 
								RsslRDMLoginRTT rttMsg;
								RsslChannelStats stats;
								RsslError error;

								rsslClearRDMLoginRTT(&rttMsg);
								if (rsslGetChannelStats(pReactorChannel->reactorChannel.pRsslChannel, &stats, &error) == RSSL_RET_FAILURE)
								{
									rsslSetErrorInfo(pError, RSSL_EIC_FAILURE, RSSL_RET_FAILURE, __FILE__, __LINE__, "Unable to retrieve channel stats, error output: %s", error.text);
									if (_reactorHandleChannelDown(pReactorImpl, pReactorChannel, pError) != RSSL_RET_SUCCESS)
										return RSSL_RET_FAILURE;
									return RSSL_RET_SUCCESS;
								}

								rttMsg.rdmMsgBase.streamId = pLoginResponse->rdmMsgBase.streamId;
								rttMsg.ticks = pLoginResponse->RTT.ticks;
								if (stats.tcpStats.flags &= RSSL_TCP_STATS_RETRANSMIT)
								{
									rttMsg.flags |= RDM_LG_RTT_HAS_TCP_RETRANS;
									rttMsg.tcpRetrans = stats.tcpStats.tcpRetransmitCount;
								}

								if ((ret = _reactorSendRDMMessage(pReactorImpl, pReactorChannel, (RsslRDMMsg*)&rttMsg, pError)) < RSSL_RET_SUCCESS)
								{
									if (_reactorHandleChannelDown(pReactorImpl, pReactorChannel, pError) != RSSL_RET_SUCCESS)
										return RSSL_RET_FAILURE;
									return RSSL_RET_SUCCESS;
								}
								loginEvent.flags |= RSSL_RDM_LG_LME_RTT_RESPONSE_SENT;

								_reactorSetInCallback(pReactorImpl, RSSL_TRUE);
								*pCret = (*pConsumerRole->loginMsgCallback)((RsslReactor*)pReactorImpl, (RsslReactorChannel*)pReactorChannel, &loginEvent);
								_reactorSetInCallback(pReactorImpl, RSSL_FALSE);
							}
							else if (_reactorHandlesWarmStandby(pReactorChannel))
							{
								/* Checks with the login response from the active server */
								RsslWatchlistImpl* pWatchlist = (RsslWatchlistImpl*)pReactorChannel->pWatchlist;
								RsslBool sendLoginCallback = RSSL_FALSE;
								RsslReactorWarmStandByHandlerImpl *pReactorWarmStandByHandlerImpl = pReactorChannel->pWarmStandByHandlerImpl;

								if (pLoginResponse->rdmMsgBase.rdmMsgType == RDM_LG_MT_REFRESH)
								{
									if (pWatchlist->base.config.supportStandby)
									{
										RsslRDMLoginConsumerConnectionStatus	consumerConnectionStatus;
										RsslReactorWarmStandbyGroupImpl *pWarmStandByGroup = &pReactorChannel->pWarmStandByHandlerImpl->warmStandbyGroupList[pReactorChannel->pWarmStandByHandlerImpl->currentWSyGroupIndex];

										if (pLoginResponse->refresh.state.streamState == RSSL_STREAM_OPEN && pLoginResponse->refresh.state.dataState == RSSL_DATA_OK)
										{
											/* Changed login state back to operational after all channels are down. */
											if (pReactorWarmStandByHandlerImpl->rdmLoginState.streamState == RSSL_STREAM_OPEN && pReactorWarmStandByHandlerImpl->rdmLoginState.dataState == RSSL_DATA_SUSPECT)
											{
												pReactorWarmStandByHandlerImpl->rdmLoginState = pLoginResponse->refresh.state;
												sendLoginCallback = RSSL_TRUE;
											}
										}

										if ( (pReactorChannel->pWarmStandByHandlerImpl->warmStandByHandlerState & RSSL_RWSB_STATE_RECEIVED_PRIMARY_LOGIN_RESP) == 0)
										{
											RsslBuffer* pChannelBuffer;
											RsslBuffer responseBuffer;

											/* Disable handling connection list. */
											//_reactorHandleWSBServerList(pReactorWarmStandByHandlerImpl, pReactorChannel, &pLoginResponse->refresh);

											/* Reset all flags */
											pReactorWarmStandByHandlerImpl->rdmLoginRefresh.flags = 0;

											do
											{
												if ((pLoginResponse->refresh.flags & RDM_LG_RFF_HAS_SUPPORT_STANDBY_MODE))
												{
													pReactorWarmStandByHandlerImpl->rdmLoginRefresh.supportStandbyMode = pLoginResponse->refresh.supportStandbyMode;
													pReactorWarmStandByHandlerImpl->rdmLoginRefresh.flags |= RDM_LG_RFF_HAS_SUPPORT_STANDBY_MODE;

													/* Checks whether warm standby mode matches with the user defined mode. */
													if ((pWarmStandByGroup->warmStandbyMode & pReactorWarmStandByHandlerImpl->rdmLoginRefresh.supportStandbyMode) == 0)
													{
														RsslReactorWarmStanbyEvent* pEvent = (RsslReactorWarmStanbyEvent*)rsslReactorEventQueueGetFromPool(&pReactorImpl->reactorEventQueue);
														rsslClearReactorWarmStanbyEvent(pEvent);

														pEvent->reactorWarmStandByEventType = RSSL_RCIMPL_WSBET_REMOVE_SERVER_FROM_WSB_GROUP;
														pEvent->pReactorChannel = (RsslReactorChannel*)pReactorChannel;
														pEvent->pReactorErrorInfoImpl = rsslReactorGetErrorInfoFromPool(&pReactorImpl->reactorWorker);

														if (pEvent->pReactorErrorInfoImpl)
														{
															rsslClearReactorErrorInfoImpl(pEvent->pReactorErrorInfoImpl);
															rsslSetErrorInfo(&pEvent->pReactorErrorInfoImpl->rsslErrorInfo, RSSL_EIC_FAILURE, RSSL_RET_FAILURE, __FILE__, __LINE__, "The warm standby mode of the login response doesn't match with the user specified in RsslReactorWarmStandbyGroup.");
														}

														if (!RSSL_ERROR_INFO_CHECK(rsslReactorEventQueuePut(&pReactorImpl->reactorEventQueue, (RsslReactorEventImpl*)pEvent) == RSSL_RET_SUCCESS, RSSL_RET_FAILURE, pError))
															return RSSL_RET_FAILURE;

														break;
													}
												}

												/* Create separate copies of strings */
												if ((pLoginResponse->refresh.flags & RDM_LG_RFF_HAS_APPLICATION_ID))
												{
													pChannelBuffer = &pReactorWarmStandByHandlerImpl->rdmLoginRefresh.applicationId;
													responseBuffer = pLoginResponse->refresh.applicationId;

													if (pChannelBuffer->length > 0 && pChannelBuffer->data != 0)
													{
														if (pChannelBuffer->length == responseBuffer.length)
														{
															if (memcmp(pChannelBuffer->data, responseBuffer.data, pChannelBuffer->length) != 0)
															{
																memcpy(pChannelBuffer->data, responseBuffer.data, responseBuffer.length);
															}
														}
														else
														{
															size_t length = responseBuffer.length;
															length += 1; // Keeping NULL ternimation
															free(pChannelBuffer->data);
															pChannelBuffer->length = 0;
															pChannelBuffer->data = calloc(length, sizeof(char));

															if (pChannelBuffer->data == NULL)
															{
																rsslSetErrorInfo(pError, RSSL_EIC_FAILURE, RSSL_RET_FAILURE, __FILE__, __LINE__, "Failed to allocate memory for storing login response.");
																if (_reactorHandleChannelDown(pReactorImpl, pReactorChannel, pError) != RSSL_RET_SUCCESS)
																	return RSSL_RET_FAILURE;
															}
															else
															{
																pChannelBuffer->length = responseBuffer.length;
																memcpy(pChannelBuffer->data, responseBuffer.data, responseBuffer.length);
															}
														}
													}
													else
													{
														size_t length = responseBuffer.length;
														length += 1; // Keeping NULL ternimation
														pChannelBuffer->length = 0;
														pChannelBuffer->data = calloc(length, sizeof(char));

														if (pChannelBuffer->data == NULL)
														{
															rsslSetErrorInfo(pError, RSSL_EIC_FAILURE, RSSL_RET_FAILURE, __FILE__, __LINE__, "Failed to allocate memory for storing login response.");
															if (_reactorHandleChannelDown(pReactorImpl, pReactorChannel, pError) != RSSL_RET_SUCCESS)
																return RSSL_RET_FAILURE;
														}
														else
														{
															pChannelBuffer->length = responseBuffer.length;
															memcpy(pChannelBuffer->data, responseBuffer.data, responseBuffer.length);
														}
													}

													pReactorWarmStandByHandlerImpl->rdmLoginRefresh.flags |= RDM_LG_RFF_HAS_APPLICATION_ID;
												}

												if (pLoginResponse->refresh.flags & RDM_LG_RFF_HAS_POSITION)
												{
													pChannelBuffer = &pReactorWarmStandByHandlerImpl->rdmLoginRefresh.position;
													responseBuffer = pLoginResponse->refresh.position;

													if (pChannelBuffer->length > 0 && pChannelBuffer->data != 0)
													{
														if (pChannelBuffer->length == responseBuffer.length)
														{
															if (memcmp(pChannelBuffer->data, responseBuffer.data, pChannelBuffer->length) != 0)
															{
																memcpy(pChannelBuffer->data, responseBuffer.data, responseBuffer.length);
															}
														}
														else
														{
															size_t length = responseBuffer.length;
															length += 1; // Keeping NULL ternimation
															free(pChannelBuffer->data);
															pChannelBuffer->length = 0;
															pChannelBuffer->data = calloc(length, sizeof(char));

															if (pChannelBuffer->data == NULL)
															{
																rsslSetErrorInfo(pError, RSSL_EIC_FAILURE, RSSL_RET_FAILURE, __FILE__, __LINE__, "Failed to allocate memory for storing login response.");
																if (_reactorHandleChannelDown(pReactorImpl, pReactorChannel, pError) != RSSL_RET_SUCCESS)
																	return RSSL_RET_FAILURE;
															}
															else
															{
																pChannelBuffer->length = responseBuffer.length;
																memcpy(pChannelBuffer->data, responseBuffer.data, responseBuffer.length);
															}
														}
													}
													else
													{
														size_t length = responseBuffer.length;
														length += 1; // Keeping NULL ternimation
														pChannelBuffer->length = 0;
														pChannelBuffer->data = calloc(length, sizeof(char));

														if (pChannelBuffer->data == NULL)
														{
															rsslSetErrorInfo(pError, RSSL_EIC_FAILURE, RSSL_RET_FAILURE, __FILE__, __LINE__, "Failed to allocate memory for storing login response.");
															if (_reactorHandleChannelDown(pReactorImpl, pReactorChannel, pError) != RSSL_RET_SUCCESS)
																return RSSL_RET_FAILURE;
														}
														else
														{
															pChannelBuffer->length = responseBuffer.length;
															memcpy(pChannelBuffer->data, responseBuffer.data, responseBuffer.length);
														}
													}

													pReactorWarmStandByHandlerImpl->rdmLoginRefresh.flags |= RDM_LG_RFF_HAS_POSITION;
												}

												/* Copies others primitive elements */
												if (pLoginResponse->refresh.flags & RDM_LG_RFF_HAS_PROVIDE_PERM_PROFILE)
												{
													pReactorWarmStandByHandlerImpl->rdmLoginRefresh.providePermissionProfile = pLoginResponse->refresh.providePermissionProfile;
													pReactorWarmStandByHandlerImpl->rdmLoginRefresh.flags |= RDM_LG_RFF_HAS_PROVIDE_PERM_PROFILE;
												}

												if (pLoginResponse->refresh.flags & RDM_LG_RFF_HAS_PROVIDE_PERM_EXPR)
												{
													pReactorWarmStandByHandlerImpl->rdmLoginRefresh.providePermissionExpressions = pLoginResponse->refresh.providePermissionExpressions;
													pReactorWarmStandByHandlerImpl->rdmLoginRefresh.flags |= RDM_LG_RFF_HAS_PROVIDE_PERM_EXPR;
												}

												if (pLoginResponse->refresh.flags & RDM_LG_RFF_HAS_SINGLE_OPEN)
												{
													pReactorWarmStandByHandlerImpl->rdmLoginRefresh.singleOpen = pLoginResponse->refresh.singleOpen;
													pReactorWarmStandByHandlerImpl->rdmLoginRefresh.flags |= RDM_LG_RFF_HAS_SINGLE_OPEN;
												}

												if (pLoginResponse->refresh.flags & RDM_LG_RFF_HAS_ALLOW_SUSPECT_DATA)
												{
													pReactorWarmStandByHandlerImpl->rdmLoginRefresh.allowSuspectData = pLoginResponse->refresh.allowSuspectData;
													pReactorWarmStandByHandlerImpl->rdmLoginRefresh.flags |= RDM_LG_RFF_HAS_ALLOW_SUSPECT_DATA;
												}

												if (pLoginResponse->refresh.flags & RDM_LG_RFF_HAS_SUPPORT_BATCH)
												{
													pReactorWarmStandByHandlerImpl->rdmLoginRefresh.supportBatchRequests = pLoginResponse->refresh.supportBatchRequests;
													pReactorWarmStandByHandlerImpl->rdmLoginRefresh.flags |= RDM_LG_RFF_HAS_SUPPORT_BATCH;
												}

												if (pLoginResponse->refresh.flags & RDM_LG_RFF_HAS_SUPPORT_POST)
												{
													pReactorWarmStandByHandlerImpl->rdmLoginRefresh.supportOMMPost = pLoginResponse->refresh.supportOMMPost;
													pReactorWarmStandByHandlerImpl->rdmLoginRefresh.flags |= RDM_LG_RFF_HAS_SUPPORT_POST;
												}

												if (pLoginResponse->refresh.flags & RDM_LG_RFF_HAS_SUPPORT_VIEW)
												{
													pReactorWarmStandByHandlerImpl->rdmLoginRefresh.supportViewRequests = pLoginResponse->refresh.supportViewRequests;
													pReactorWarmStandByHandlerImpl->rdmLoginRefresh.flags |= RDM_LG_RFF_HAS_SUPPORT_VIEW;
												}

												pReactorChannel->pWarmStandByHandlerImpl->warmStandByHandlerState |= RSSL_RWSB_STATE_RECEIVED_PRIMARY_LOGIN_RESP;

												sendLoginCallback = RSSL_TRUE;

												if (pWarmStandByGroup->warmStandbyMode == RSSL_RWSB_MODE_LOGIN_BASED)
												{
													RsslBool isServerTypeActive = RSSL_FALSE;

													if (pWarmStandByGroup->downloadConfigActiveServer == RSSL_REACTOR_WSB_STARTING_SERVER_INDEX && pReactorChannel->isStartingServerConfig)
													{
														isServerTypeActive = RSSL_TRUE;
													}
													else if (!pReactorChannel->isStartingServerConfig && pWarmStandByGroup->downloadConfigActiveServer == pReactorChannel->standByServerListIndex)
													{
														isServerTypeActive = RSSL_TRUE;
													}

													rsslClearRDMLoginConsumerConnectionStatus(&consumerConnectionStatus);
													consumerConnectionStatus.rdmMsgBase.streamId = pLoginResponse->rdmMsgBase.streamId;
													consumerConnectionStatus.flags = RDM_LG_CCSF_HAS_WARM_STANDBY_INFO;
													consumerConnectionStatus.warmStandbyInfo.warmStandbyMode = isServerTypeActive ? RDM_LOGIN_SERVER_TYPE_ACTIVE : RDM_LOGIN_SERVER_TYPE_STANDBY;

													if ((ret = _reactorSendRDMMessage(pReactorImpl, pReactorChannel, (RsslRDMMsg*)&consumerConnectionStatus, pError)) < RSSL_RET_SUCCESS)
													{
														if (_reactorHandleChannelDown(pReactorImpl, pReactorChannel, pError) != RSSL_RET_SUCCESS)
															return RSSL_RET_FAILURE;
														return RSSL_RET_SUCCESS;
													}

													if (isServerTypeActive)
													{
														pReactorChannel->isActiveServer = RSSL_TRUE;
														pReactorChannel->pWarmStandByHandlerImpl->pActiveReactorChannel = pReactorChannel;
													}
												}
											}while (RSSL_FALSE);
										}
										else
										{
											if (validateLoginResponseFromStandbyServer(&pReactorWarmStandByHandlerImpl->rdmLoginRefresh, &pLoginResponse->refresh) == RSSL_FALSE)
											{
												RsslReactorWarmStanbyEvent* pEvent = (RsslReactorWarmStanbyEvent*)rsslReactorEventQueueGetFromPool(&pReactorImpl->reactorEventQueue);
												rsslClearReactorWarmStanbyEvent(pEvent);

												pEvent->reactorWarmStandByEventType = RSSL_RCIMPL_WSBET_REMOVE_SERVER_FROM_WSB_GROUP;
												pEvent->pReactorChannel = (RsslReactorChannel*)pReactorChannel;
												pEvent->pReactorErrorInfoImpl = rsslReactorGetErrorInfoFromPool(&pReactorImpl->reactorWorker);

												if (pEvent->pReactorErrorInfoImpl)
												{
													rsslClearReactorErrorInfoImpl(pEvent->pReactorErrorInfoImpl);
													rsslSetErrorInfo(&pEvent->pReactorErrorInfoImpl->rsslErrorInfo, RSSL_EIC_FAILURE, RSSL_RET_FAILURE, __FILE__, __LINE__, "The login response from standby server does not match with the primary server.");
												}

												if (!RSSL_ERROR_INFO_CHECK(rsslReactorEventQueuePut(&pReactorImpl->reactorEventQueue, (RsslReactorEventImpl*)pEvent) == RSSL_RET_SUCCESS, RSSL_RET_FAILURE, pError))
													return RSSL_RET_FAILURE;
											}
											else
											{
												RsslReactorChannelImpl* pActiveServerChannel = pReactorWarmStandByHandlerImpl->pActiveReactorChannel;

												if (pActiveServerChannel != NULL)
												{
													if (pWarmStandByGroup->warmStandbyMode == RSSL_RWSB_MODE_LOGIN_BASED)
													{
														RsslRDMLoginConsumerConnectionStatus	consumerConnectionStatus;

														rsslClearRDMLoginConsumerConnectionStatus(&consumerConnectionStatus);
														consumerConnectionStatus.rdmMsgBase.streamId = pLoginResponse->rdmMsgBase.streamId;
														consumerConnectionStatus.flags = RDM_LG_CCSF_HAS_WARM_STANDBY_INFO;
														consumerConnectionStatus.warmStandbyInfo.warmStandbyMode = RDM_LOGIN_SERVER_TYPE_STANDBY;

														if ((ret = _reactorSendRDMMessage(pReactorImpl, pReactorChannel, (RsslRDMMsg*)&consumerConnectionStatus, pError)) < RSSL_RET_SUCCESS)
														{
															if (_reactorHandleChannelDown(pReactorImpl, pReactorChannel, pError) != RSSL_RET_SUCCESS)
																return RSSL_RET_FAILURE;
															return RSSL_RET_SUCCESS;
														}
													}
												}
												else
												{
													/* There is no active channel due to connection lost for all channels so makes this channel as the active server. */
													if (pWarmStandByGroup->warmStandbyMode == RSSL_RWSB_MODE_LOGIN_BASED)
													{
														RsslRDMLoginConsumerConnectionStatus	consumerConnectionStatus;
														RsslBool isServerTypeActive = RSSL_FALSE;

														if (pWarmStandByGroup->downloadConnectionConfig)
														{
															if (pWarmStandByGroup->downloadConfigActiveServer == RSSL_REACTOR_WSB_STARTING_SERVER_INDEX && pReactorChannel->isStartingServerConfig)
															{
																isServerTypeActive = RSSL_TRUE;
															}
															else if (!pReactorChannel->isStartingServerConfig && pWarmStandByGroup->downloadConfigActiveServer == pReactorChannel->standByServerListIndex)
															{
																isServerTypeActive = RSSL_TRUE;
															}
														}
														else
														{
															isServerTypeActive = RSSL_TRUE;
														}

														rsslClearRDMLoginConsumerConnectionStatus(&consumerConnectionStatus);
														consumerConnectionStatus.rdmMsgBase.streamId = pLoginResponse->rdmMsgBase.streamId;
														consumerConnectionStatus.flags = RDM_LG_CCSF_HAS_WARM_STANDBY_INFO;
														consumerConnectionStatus.warmStandbyInfo.warmStandbyMode = isServerTypeActive ? RDM_LOGIN_SERVER_TYPE_ACTIVE : RDM_LOGIN_SERVER_TYPE_STANDBY;

														if ((ret = _reactorSendRDMMessage(pReactorImpl, pReactorChannel, (RsslRDMMsg*)&consumerConnectionStatus, pError)) < RSSL_RET_SUCCESS)
														{
															if (_reactorHandleChannelDown(pReactorImpl, pReactorChannel, pError) != RSSL_RET_SUCCESS)
																return RSSL_RET_FAILURE;
															return RSSL_RET_SUCCESS;
														}

														if (isServerTypeActive)
														{
															pReactorChannel->isActiveServer = RSSL_TRUE;
															pReactorChannel->pWarmStandByHandlerImpl->pActiveReactorChannel = pReactorChannel;
														}
													}
												}
											}
										}
									}
									else
									{
										RsslReactorWarmStanbyEvent* pEvent = (RsslReactorWarmStanbyEvent*)rsslReactorEventQueueGetFromPool(&pReactorImpl->reactorEventQueue);
										rsslClearReactorWarmStanbyEvent(pEvent);

										pEvent->reactorWarmStandByEventType = RSSL_RCIMPL_WSBET_REMOVE_SERVER_FROM_WSB_GROUP;
										pEvent->pReactorChannel = (RsslReactorChannel*)pReactorChannel;
										pEvent->pReactorErrorInfoImpl = rsslReactorGetErrorInfoFromPool(&pReactorImpl->reactorWorker);

										if (pEvent->pReactorErrorInfoImpl)
										{
											rsslClearReactorErrorInfoImpl(pEvent->pReactorErrorInfoImpl);
											rsslSetErrorInfo(&pEvent->pReactorErrorInfoImpl->rsslErrorInfo, RSSL_EIC_FAILURE, RSSL_RET_FAILURE, __FILE__, __LINE__, "The login response doesn't support warm standby functionality.");
										}

										if (!RSSL_ERROR_INFO_CHECK(rsslReactorEventQueuePut(&pReactorImpl->reactorEventQueue, (RsslReactorEventImpl*)pEvent) == RSSL_RET_SUCCESS, RSSL_RET_FAILURE, pError))
											return RSSL_RET_FAILURE;
									}
								}
								else if (pLoginResponse->rdmMsgBase.rdmMsgType == RDM_LG_MT_STATUS)
								{
									/* Send login response status if this the last closed channel */
									if (isWarmStandbyChannelClosed(pReactorWarmStandByHandlerImpl, NULL) == RSSL_TRUE)
									{
										sendLoginCallback = RSSL_TRUE;

										pReactorWarmStandByHandlerImpl->rdmLoginState = pLoginResponse->status.state;
									}
									else
									{
										if (pLoginResponse->status.state.streamState == RSSL_STREAM_CLOSED)
										{
											/* Closes this channel if it is closed by the server */
											RsslReactorWarmStanbyEvent* pEvent = (RsslReactorWarmStanbyEvent*)rsslReactorEventQueueGetFromPool(&pReactorImpl->reactorEventQueue);
											rsslClearReactorWarmStanbyEvent(pEvent);

											pEvent->reactorWarmStandByEventType = RSSL_RCIMPL_WSBET_REMOVE_SERVER_FROM_WSB_GROUP;
											pEvent->pReactorChannel = (RsslReactorChannel*)pReactorChannel;
											pEvent->pReactorErrorInfoImpl = rsslReactorGetErrorInfoFromPool(&pReactorImpl->reactorWorker);

											if (pEvent->pReactorErrorInfoImpl)
											{
												rsslClearReactorErrorInfoImpl(pEvent->pReactorErrorInfoImpl);
												rsslSetErrorInfo(&pEvent->pReactorErrorInfoImpl->rsslErrorInfo, RSSL_EIC_FAILURE, RSSL_RET_FAILURE, __FILE__, __LINE__, "The server rejects login request.");
											}

											if (!RSSL_ERROR_INFO_CHECK(rsslReactorEventQueuePut(&pReactorImpl->reactorEventQueue, (RsslReactorEventImpl*)pEvent) == RSSL_RET_SUCCESS, RSSL_RET_FAILURE, pError))
												return RSSL_RET_FAILURE;
										}

										/* Send login response status if this the last channel to notify application */
										RSSL_MUTEX_LOCK(&pReactorWarmStandByHandlerImpl->warmStandByHandlerMutex);
										if (pReactorWarmStandByHandlerImpl->rsslChannelQueue.count == 1)
										{
											sendLoginCallback = RSSL_TRUE;

											/* Set internal login state to open/suspect so that the login steam can recovery with open/ok */
											pReactorWarmStandByHandlerImpl->rdmLoginState.streamState = RSSL_STREAM_OPEN;
											pReactorWarmStandByHandlerImpl->rdmLoginState.dataState = RSSL_DATA_SUSPECT;
										}
										RSSL_MUTEX_UNLOCK(&pReactorWarmStandByHandlerImpl->warmStandByHandlerMutex);
									}
								}
								else if (pLoginResponse->rdmMsgBase.rdmMsgType == RDM_LG_MT_ACK)
								{
									sendLoginCallback = RSSL_TRUE;
								}
						
								if (sendLoginCallback)
								{
									RsslReactorChannel*  pCallbackChannel = &pReactorChannel->pWarmStandByHandlerImpl->mainReactorChannelImpl.reactorChannel;
									pCallbackChannel->pRsslChannel = pReactorChannel->reactorChannel.pRsslChannel;
									pCallbackChannel->socketId = pReactorChannel->reactorChannel.socketId;
									pCallbackChannel->oldSocketId = pReactorChannel->reactorChannel.oldSocketId;
									pCallbackChannel->majorVersion = pReactorChannel->reactorChannel.majorVersion;
									pCallbackChannel->minorVersion = pReactorChannel->reactorChannel.minorVersion;
									pCallbackChannel->protocolType = pReactorChannel->reactorChannel.protocolType;
									pCallbackChannel->userSpecPtr = pReactorChannel->reactorChannel.userSpecPtr;

									_reactorSetInCallback(pReactorImpl, RSSL_TRUE);
									*pCret = (*pConsumerRole->loginMsgCallback)((RsslReactor*)pReactorImpl, (RsslReactorChannel*)pCallbackChannel, &loginEvent);
									_reactorSetInCallback(pReactorImpl, RSSL_FALSE);
								}
								else
								{
									*pCret = RSSL_RC_CRET_SUCCESS;
								}

							} /* End of supporting the warmstandby feature. */
							else
							{
								_reactorSetInCallback(pReactorImpl, RSSL_TRUE);
								*pCret = (*pConsumerRole->loginMsgCallback)((RsslReactor*)pReactorImpl, (RsslReactorChannel*)pReactorChannel, &loginEvent);
								_reactorSetInCallback(pReactorImpl, RSSL_FALSE);
							}
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
							case RSSL_MC_GENERIC:
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

							if (_reactorHandlesWarmStandby(pReactorChannel))
							{
								RsslReactorWarmStandByHandlerImpl *pReactorWarmStandByHandlerImpl = pReactorChannel->pWarmStandByHandlerImpl;
								RsslReactorWarmStandbyGroupImpl *pReactorWarmStandByGroupImpl = &pReactorWarmStandByHandlerImpl->warmStandbyGroupList[pReactorWarmStandByHandlerImpl->currentWSyGroupIndex];
								RsslQueueLink *pLink;
								RsslReactorWarmStandbyServiceImpl *pWarmStandbyServiceImpl = NULL;
								RsslBool isAllChannelClosed = isWarmStandbyChannelClosed(pReactorWarmStandByHandlerImpl, NULL);
								
								if ((pReactorChannel->pWarmStandByHandlerImpl->warmStandByHandlerState & RSSL_RWSB_STATE_RECEIVED_SECONDARY_DIRECTORY_RESP) == 0 && !isAllChannelClosed)
								{
									RsslReactorChannel* pCallbackChannel = &pReactorChannel->pWarmStandByHandlerImpl->mainReactorChannelImpl.reactorChannel;
									pCallbackChannel->pRsslChannel = pReactorChannel->reactorChannel.pRsslChannel;
									pCallbackChannel->socketId = pReactorChannel->reactorChannel.socketId;
									pCallbackChannel->oldSocketId = pReactorChannel->reactorChannel.oldSocketId;
									pCallbackChannel->majorVersion = pReactorChannel->reactorChannel.majorVersion;
									pCallbackChannel->minorVersion = pReactorChannel->reactorChannel.minorVersion;
									pCallbackChannel->protocolType = pReactorChannel->reactorChannel.protocolType;
									pCallbackChannel->userSpecPtr = pReactorChannel->reactorChannel.userSpecPtr;

									_reactorSetInCallback(pReactorImpl, RSSL_TRUE);
									*pCret = (*pConsumerRole->directoryMsgCallback)((RsslReactor*)pReactorImpl, (RsslReactorChannel*)pCallbackChannel, &directoryEvent);
									_reactorSetInCallback(pReactorImpl, RSSL_FALSE);

									while ((pLink = rsslQueueRemoveFirstLink(&pReactorWarmStandByGroupImpl->_updateServiceList)))
									{
										pWarmStandbyServiceImpl = RSSL_QUEUE_LINK_TO_OBJECT(RsslReactorWarmStandbyServiceImpl, updateQLink, pLink);
										/* Reset the service flag. */
										pWarmStandbyServiceImpl->updateServiceFilter = RDM_SVCF_NONE;
									}

									rsslInitQueue(&pReactorWarmStandByGroupImpl->_updateServiceList);

								}
								else
								{
									/* Handles source dirctory update for the delete action generated by watchlist when the channel down. */
									if(isRsslChannelActive(pReactorChannel) == RSSL_FALSE)
									{
										RsslWatchlistImpl* pWatchlistImpl = (RsslWatchlistImpl*)pReactorChannel->pWatchlist;
										_reactorWSDirectoryUpdateFromChannelDown(pReactorWarmStandByGroupImpl, pReactorChannel, pWatchlistImpl->base.pServiceCache);
									}

									/* Create source directory response from the update servie list if any. */
									if (pReactorWarmStandByGroupImpl->_updateServiceList.count > 0)
									{
										RsslRDMDirectoryUpdate directoryUpdate;
										RsslUInt32 index = 0;
										RsslQueueLink* pLink = NULL;
										rsslClearRDMDirectoryUpdate(&directoryUpdate);

										directoryUpdate.rdmMsgBase = pDirectoryResponse->rdmMsgBase;
										directoryUpdate.rdmMsgBase.rdmMsgType = RDM_DR_MT_UPDATE;

										directoryUpdate.serviceList = (RsslRDMService*)malloc(pReactorWarmStandByGroupImpl->_updateServiceList.count * sizeof(RsslRDMService));
										if (directoryUpdate.serviceList == NULL)
										{
											RsslReactorWarmStanbyEvent* pEvent = (RsslReactorWarmStanbyEvent*)rsslReactorEventQueueGetFromPool(&pReactorImpl->reactorEventQueue);
											rsslClearReactorWarmStanbyEvent(pEvent);

											pEvent->reactorWarmStandByEventType = RSSL_RCIMPL_WSBET_REMOVE_SERVER_FROM_WSB_GROUP;
											pEvent->pReactorChannel = (RsslReactorChannel*)pReactorChannel;
											pEvent->pReactorErrorInfoImpl = rsslReactorGetErrorInfoFromPool(&pReactorImpl->reactorWorker);

											if (pEvent->pReactorErrorInfoImpl)
											{
												rsslClearReactorErrorInfoImpl(pEvent->pReactorErrorInfoImpl);
												rsslSetErrorInfo(&pEvent->pReactorErrorInfoImpl->rsslErrorInfo, RSSL_EIC_FAILURE, RSSL_RET_FAILURE, __FILE__, __LINE__, "Failed to allocate memory for source directory update message.");
											}

											if (!RSSL_ERROR_INFO_CHECK(rsslReactorEventQueuePut(&pReactorImpl->reactorEventQueue, (RsslReactorEventImpl*)pEvent) == RSSL_RET_SUCCESS, RSSL_RET_FAILURE, pError))
												return RSSL_RET_FAILURE;
										}
										else
										{
											RsslReactorChannel* pCallbackChannel;
											directoryUpdate.serviceCount = pReactorWarmStandByGroupImpl->_updateServiceList.count;
											index = 0;

											while ((pLink = rsslQueueRemoveFirstLink(&pReactorWarmStandByGroupImpl->_updateServiceList)))
											{
												pWarmStandbyServiceImpl = RSSL_QUEUE_LINK_TO_OBJECT(RsslReactorWarmStandbyServiceImpl, updateQLink, pLink);

												rsslClearRDMService(&directoryUpdate.serviceList[index]);
												directoryUpdate.serviceList[index].action = pWarmStandbyServiceImpl->serviceAction;
												directoryUpdate.serviceList[index].flags = pWarmStandbyServiceImpl->updateServiceFilter;
												directoryUpdate.serviceList[index].serviceId = pWarmStandbyServiceImpl->serviceID;

												/* Chckes whether the source directory update has service info */
												if ((pWarmStandbyServiceImpl->updateServiceFilter & RDM_SVCF_HAS_INFO) != 0)
												{
													directoryUpdate.serviceList[index].info = pWarmStandbyServiceImpl->rdmServiceInfo;
												}

												/* Chckes whether the source directory update has service state */
												if ((pWarmStandbyServiceImpl->updateServiceFilter & RDM_SVCF_HAS_STATE) != 0)
												{
													directoryUpdate.serviceList[index].state = pWarmStandbyServiceImpl->rdmServiceState;
												}

												/* Reset the service flag. */
												pWarmStandbyServiceImpl->updateServiceFilter = RDM_SVCF_NONE;

												++index;
											}

											directoryEvent.baseMsgEvent.pRsslMsgBuffer = NULL;
											directoryEvent.baseMsgEvent.pRsslMsg = NULL;
											directoryEvent.baseMsgEvent.pStreamInfo = (RsslStreamInfo*)pStreamInfo;
											directoryEvent.baseMsgEvent.pFTGroupId = NULL;
											directoryEvent.baseMsgEvent.pSeqNum = NULL;
											directoryEvent.pRDMDirectoryMsg = (RsslRDMDirectoryMsg*)&directoryUpdate;

											pCallbackChannel = &pReactorChannel->pWarmStandByHandlerImpl->mainReactorChannelImpl.reactorChannel;
											pCallbackChannel->pRsslChannel = pReactorChannel->reactorChannel.pRsslChannel;
											pCallbackChannel->socketId = pReactorChannel->reactorChannel.socketId;
											pCallbackChannel->oldSocketId = pReactorChannel->reactorChannel.oldSocketId;
											pCallbackChannel->majorVersion = pReactorChannel->reactorChannel.majorVersion;
											pCallbackChannel->minorVersion = pReactorChannel->reactorChannel.minorVersion;
											pCallbackChannel->protocolType = pReactorChannel->reactorChannel.protocolType;
											pCallbackChannel->userSpecPtr = pReactorChannel->reactorChannel.userSpecPtr;

											_reactorSetInCallback(pReactorImpl, RSSL_TRUE);
											*pCret = (*pConsumerRole->directoryMsgCallback)((RsslReactor*)pReactorImpl, (RsslReactorChannel*)pCallbackChannel, &directoryEvent);
											_reactorSetInCallback(pReactorImpl, RSSL_FALSE);

											rsslInitQueue(&pReactorWarmStandByGroupImpl->_updateServiceList);

											if (directoryUpdate.serviceList != NULL)
											{
												free(directoryUpdate.serviceList);
											}
										}
									} /* End checks the number of service list count. */
									else
									{
										*pCret = RSSL_RC_CRET_SUCCESS;
									}
								}
							}
							else
							{
								_reactorSetInCallback(pReactorImpl, RSSL_TRUE);
								*pCret = (*pConsumerRole->directoryMsgCallback)((RsslReactor*)pReactorImpl, (RsslReactorChannel*)pReactorChannel, &directoryEvent);
								_reactorSetInCallback(pReactorImpl, RSSL_FALSE);
							}
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
					{
						break;
					}

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
									rsslSetErrorInfo(pError, RSSL_EIC_FAILURE, RSSL_RET_FAILURE, __FILE__, __LINE__, "Received an invalid directory message response.");
									pServiceList = NULL;
									pState = NULL;
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
						RsslBool callbackToClient = _warmStandbyCallbackChecks(pReactorChannel, pOpts);

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
						if (ret == RSSL_RET_SUCCESS)
						{
							dictionaryEvent.pRDMDictionaryMsg = &dictionaryResponse;

							if (_reactorHandlesWarmStandby(pReactorChannel) && dictionaryResponse.rdmMsgBase.rdmMsgType == RDM_DC_MT_REFRESH)
							{
								/* Checks whether the dictionary version is available */
								if (dictionaryResponse.refresh.flags & RDM_DC_RFF_HAS_INFO)
								{
									RsslReactorWarmStandByHandlerImpl *pWarmStandByHandlerImpl = pReactorChannel->pWarmStandByHandlerImpl;
									RsslReactorDictionaryVersion dictionaryVersion;
									RsslBool primaryDictionary = RSSL_FALSE;

									if (callbackToClient)
									{
										primaryDictionary = RSSL_TRUE;
									}

									switch (dictionaryResponse.refresh.type)
									{
										case RDM_DICTIONARY_FIELD_DEFINITIONS:
										{
											if (primaryDictionary)
											{
												if ((pWarmStandByHandlerImpl->warmStandByHandlerState & RSSL_RWSB_STATE_RECEIVED_PRIMARY_FIELD_DICT_RESP) == 0)
												{
													_getDictionaryVersion(&dictionaryResponse.refresh.version, &dictionaryVersion);

													pWarmStandByHandlerImpl->rdmFieldVersion = dictionaryVersion;

													pWarmStandByHandlerImpl->warmStandByHandlerState |= RSSL_RWSB_STATE_RECEIVED_PRIMARY_FIELD_DICT_RESP;
												}
											}
											else
											{
												if (pWarmStandByHandlerImpl->warmStandByHandlerState & RSSL_RWSB_STATE_RECEIVED_PRIMARY_FIELD_DICT_RESP)
												{
													_getDictionaryVersion(&dictionaryResponse.refresh.version, &dictionaryVersion);

													if (pWarmStandByHandlerImpl->rdmFieldVersion.major != dictionaryVersion.major)
													{
														RsslReactorWarmStanbyEvent* pEvent = (RsslReactorWarmStanbyEvent*)rsslReactorEventQueueGetFromPool(&pReactorImpl->reactorEventQueue);
														rsslClearReactorWarmStanbyEvent(pEvent);

														pEvent->reactorWarmStandByEventType = RSSL_RCIMPL_WSBET_REMOVE_SERVER_FROM_WSB_GROUP;
														pEvent->pReactorChannel = (RsslReactorChannel*)pReactorChannel;
														pEvent->pReactorErrorInfoImpl = rsslReactorGetErrorInfoFromPool(&pReactorImpl->reactorWorker);

														if (pEvent->pReactorErrorInfoImpl)
														{
															rsslClearReactorErrorInfoImpl(pEvent->pReactorErrorInfoImpl);
															rsslSetErrorInfo(&pEvent->pReactorErrorInfoImpl->rsslErrorInfo, RSSL_EIC_FAILURE, RSSL_RET_FAILURE, __FILE__, __LINE__, "The major version of field dicitonary response from standby server does not match with the primary server.");
														}

														if (!RSSL_ERROR_INFO_CHECK(rsslReactorEventQueuePut(&pReactorImpl->reactorEventQueue, (RsslReactorEventImpl*)pEvent) == RSSL_RET_SUCCESS, RSSL_RET_FAILURE, pError))
															return RSSL_RET_FAILURE;
													}
												}
											}

											break;
										}
										case RDM_DICTIONARY_ENUM_TABLES:
										{
											if (primaryDictionary)
											{
												if ((pWarmStandByHandlerImpl->warmStandByHandlerState & RSSL_RWSB_STATE_RECEIVED_PRIMARY_ENUM_DICT_RESP) == 0)
												{
													_getDictionaryVersion(&dictionaryResponse.refresh.version, &dictionaryVersion);

													pWarmStandByHandlerImpl->rdmEnumTypeVersion = dictionaryVersion;

													pWarmStandByHandlerImpl->warmStandByHandlerState |= RSSL_RWSB_STATE_RECEIVED_PRIMARY_ENUM_DICT_RESP;
												}
											}
											else
											{
												if (pWarmStandByHandlerImpl->warmStandByHandlerState & RSSL_RWSB_STATE_RECEIVED_PRIMARY_ENUM_DICT_RESP)
												{
													_getDictionaryVersion(&dictionaryResponse.refresh.version, &dictionaryVersion);

													if (pWarmStandByHandlerImpl->rdmEnumTypeVersion.major != dictionaryVersion.major)
													{
														RsslReactorWarmStanbyEvent* pEvent = (RsslReactorWarmStanbyEvent*)rsslReactorEventQueueGetFromPool(&pReactorImpl->reactorEventQueue);
														rsslClearReactorWarmStanbyEvent(pEvent);

														pEvent->reactorWarmStandByEventType = RSSL_RCIMPL_WSBET_REMOVE_SERVER_FROM_WSB_GROUP;
														pEvent->pReactorChannel = (RsslReactorChannel*)pReactorChannel;
														pEvent->pReactorErrorInfoImpl = rsslReactorGetErrorInfoFromPool(&pReactorImpl->reactorWorker);

														if (pEvent->pReactorErrorInfoImpl)
														{
															rsslClearReactorErrorInfoImpl(pEvent->pReactorErrorInfoImpl);
															rsslSetErrorInfo(&pEvent->pReactorErrorInfoImpl->rsslErrorInfo, RSSL_EIC_FAILURE, RSSL_RET_FAILURE, __FILE__, __LINE__, "The major version of enumeration dictionary response from standby server does not match with the primary server.");
														}

														if (!RSSL_ERROR_INFO_CHECK(rsslReactorEventQueuePut(&pReactorImpl->reactorEventQueue, (RsslReactorEventImpl*)pEvent) == RSSL_RET_SUCCESS, RSSL_RET_FAILURE, pError))
															return RSSL_RET_FAILURE;
													}
												}
											}
											break;
										}
									}
								}
								else
								{
									/* DO nothing as there is no dictionary version. */
								}
							}
						}
						else
						{
							dictionaryEvent.baseMsgEvent.pErrorInfo = pError;
						}

						if (callbackToClient)
						{
							RsslReactorChannel* pCallbackChannel = (RsslReactorChannel*)pReactorChannel;

							if (_reactorHandlesWarmStandby(pReactorChannel))
							{
								pCallbackChannel = &pReactorChannel->pWarmStandByHandlerImpl->mainReactorChannelImpl.reactorChannel;
								pCallbackChannel->pRsslChannel = pReactorChannel->reactorChannel.pRsslChannel;
								pCallbackChannel->socketId = pReactorChannel->reactorChannel.socketId;
								pCallbackChannel->oldSocketId = pReactorChannel->reactorChannel.oldSocketId;
								pCallbackChannel->majorVersion = pReactorChannel->reactorChannel.majorVersion;
								pCallbackChannel->minorVersion = pReactorChannel->reactorChannel.minorVersion;
								pCallbackChannel->protocolType = pReactorChannel->reactorChannel.protocolType;
								pCallbackChannel->userSpecPtr = pReactorChannel->reactorChannel.userSpecPtr;
							}

							_reactorSetInCallback(pReactorImpl, RSSL_TRUE);
							*pCret = (*pConsumerRole->dictionaryMsgCallback)((RsslReactor*)pReactorImpl, (RsslReactorChannel*)pCallbackChannel, &dictionaryEvent);
							_reactorSetInCallback(pReactorImpl, RSSL_FALSE);
						}
						else
						{
							*pCret = RSSL_RC_CRET_SUCCESS;
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
	processOpts.wsbStatusFlags = 0;

	if (pEvent->_flags)
	{
		processOpts.wsbStatusFlags = pEvent->_flags;
	}

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

static RsslRet _processRsslRwfMessage(RsslReactorImpl *pReactorImpl, RsslReactorChannelImpl *pReactorChannel, RsslReadOutArgs*readOutArgs, RsslBuffer *pMsgBuf, RsslErrorInfo *pError)
{
	RsslRet ret;
	RsslChannel *pChannel = pReactorChannel->reactorChannel.pRsslChannel;
	RsslReactorCallbackRet cret;
	RsslDecodeIterator dIter;
	RsslMsg msg;

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
			/* Pass message to watchlist for processing. */
			RsslWatchlistProcessMsgOptions wlProcessOpts;

			rsslWatchlistClearProcessMsgOptions(&wlProcessOpts);
			wlProcessOpts.pChannel = pReactorChannel->reactorChannel.pRsslChannel;
			wlProcessOpts.pDecodeIterator = &dIter;
			wlProcessOpts.pRsslBuffer = pMsgBuf;
			wlProcessOpts.pRsslMsg = &msg;

			if (readOutArgs->readOutFlags & RSSL_READ_OUT_FTGROUP_ID)
				wlProcessOpts.pFTGroupId = &readOutArgs->FTGroupId;

			if (readOutArgs->readOutFlags & RSSL_READ_OUT_SEQNUM)
				wlProcessOpts.pSeqNum = &readOutArgs->seqNum;

			if (_reactorHandlesWarmStandby(pReactorChannel))
			{
				pReactorChannel->pWarmStandByHandlerImpl->pReadMsgChannel = pReactorChannel;

				if ((ret = _reactorWSReadWatchlistMsg(pReactorImpl, pReactorChannel, &wlProcessOpts, pError))
					< RSSL_RET_SUCCESS)
				{
					pReactorChannel->pWarmStandByHandlerImpl->pReadMsgChannel = NULL;
					return ret;
				}

				pReactorChannel->pWarmStandByHandlerImpl->pReadMsgChannel = NULL;
			}
			else
			{
				if ((ret = _reactorReadWatchlistMsg(pReactorImpl, pReactorChannel, &wlProcessOpts, NULL, pError))
					< RSSL_RET_SUCCESS)
					return ret;
			}

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
			processOpts.wsbStatusFlags = 0;

			processOpts.pFTGroupId = 
				(readOutArgs->readOutFlags & RSSL_READ_OUT_FTGROUP_ID) ? 
				&readOutArgs->FTGroupId : NULL;

			processOpts.pSeqNum = 
				(readOutArgs->readOutFlags & RSSL_READ_OUT_SEQNUM) ?
				&readOutArgs->seqNum : NULL;

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

static RsslRet _reactorDispatchFromChannel(RsslReactorImpl *pReactorImpl, RsslReactorChannelImpl *pReactorChannel, RsslErrorInfo *pError)
{
	RsslRet ret;
	RsslBuffer *pMsgBuf;
	RsslChannel *pChannel = pReactorChannel->reactorChannel.pRsslChannel;

	RsslReadInArgs	readInArgs;
	RsslReadOutArgs	readOutArgs;

	rsslClearReadInArgs(&readInArgs);
	rsslClearReadOutArgs(&readOutArgs);

	pMsgBuf = rsslReadEx(pChannel, &readInArgs, &readOutArgs, &ret, &pError->rsslError);

	/* Collects read statistics */
	if ( (pReactorChannel->statisticFlags & RSSL_RC_ST_READ) && pReactorChannel->pChannelStatistic)
	{
		_cumulativeValue(&pReactorChannel->pChannelStatistic->bytesRead, readOutArgs.bytesRead);
		_cumulativeValue(&pReactorChannel->pChannelStatistic->uncompressedBytesRead, readOutArgs.uncompressedBytesRead);
	}

	pReactorChannel->readRet = ret;

	if (pMsgBuf)
	{
		/* Update ping time & notification logic */
		pReactorChannel->lastPingReadMs = pReactorImpl->lastRecordedTimeMs;

		if (ret == RSSL_RET_SUCCESS) 
			rsslNotifierEventClearNotifiedFlags(pReactorChannel->pNotifierEvent); /* Done reading. */

		if (pChannel->protocolType == RSSL_JSON_PROTOCOL_TYPE)
		{
			RsslDecodeJsonMsgOptions decodeOptions;
			RsslJsonConverterError rjcError;
			RsslParseJsonBufferOptions parseOptions;
			RsslJsonMsg jsonMsg;
			RsslBool failedToConvertJSONMsg = RSSL_TRUE;

			rjcError.rsslErrorId = RSSL_RET_SUCCESS;

			/* Added checking to ensure that the JSON converter is initialized properly.*/
			if (pReactorImpl->pJsonConverter == 0)
			{
				rsslSetErrorInfo(pError, RSSL_EIC_FAILURE, RSSL_RET_FAILURE, __FILE__, __LINE__, "The JSON converter library has not been initialized.");
				return RSSL_RET_FAILURE;
			}

			rsslClearParseJsonBufferOptions(&parseOptions);
			parseOptions.jsonProtocolType = RSSL_JSON_PROTOCOL_TYPE;
			if ( (ret = rsslParseJsonBuffer(pReactorImpl->pJsonConverter, &parseOptions, pMsgBuf, &rjcError) ) == RSSL_RET_SUCCESS) 
			{
				RsslBuffer decodedMsg = RSSL_INIT_BUFFER;
				rsslClearDecodeJsonMsgOptions(&decodeOptions);
				decodeOptions.jsonProtocolType = RSSL_JSON_PROTOCOL_TYPE;

				while ( (ret = rsslDecodeJsonMsg(pReactorImpl->pJsonConverter, &decodeOptions, &jsonMsg, &decodedMsg, &rjcError) ) != RSSL_RET_END_OF_CONTAINER)
				{
					if (ret != RSSL_RET_SUCCESS)
					{	/* Failed to convert a JSON message. */
						rjcError.rsslErrorId = ret;
						break;
					}

					switch(jsonMsg.msgBase.msgClass)
					{
						case RSSL_JSON_MC_RSSL_MSG:
						{
							failedToConvertJSONMsg = RSSL_FALSE;

							rsslDumpBuffer(pReactorChannel->reactorChannel.pRsslChannel, RSSL_RWF_PROTOCOL_TYPE, &decodedMsg, &pError->rsslError);

							ret = _processRsslRwfMessage(pReactorImpl, pReactorChannel, &readOutArgs, &decodedMsg, pError);

							if (ret < RSSL_RET_SUCCESS)
								return ret;

							break;
						}
						case RSSL_JSON_MC_PING:
						{
							RsslBuffer *pBuffer = rsslReactorGetBuffer(&pReactorChannel->reactorChannel, PONG_MESSAGE.length, RSSL_FALSE, pError);

							failedToConvertJSONMsg = RSSL_FALSE;

							if (pBuffer)
							{
								memcpy(pBuffer->data, PONG_MESSAGE.data, PONG_MESSAGE.length);

								/* Reply with JSON PONG message to the sender */
								ret = _reactorSendJSONMessage(pReactorImpl, pReactorChannel, pBuffer, pError);
							}
							else
							{
								ret = RSSL_RET_FAILURE;
							}

							break;
						}
						case RSSL_JSON_MC_PONG:
						{
							failedToConvertJSONMsg = RSSL_FALSE;
							/* Do nothing as pReactorChannel->lastPingReadMs is set at the beginning of this function */
							break;
						}
						default: /* RSSL_JSON_MC_ERROR */
						{
							/* Received JSON error message from the others side */
							char jsonMessage[RSSL_REACTOR_MAX_JSON_ERROR_MSG_SIZE];
							memset(&jsonMessage[0], 0, RSSL_REACTOR_MAX_JSON_ERROR_MSG_SIZE);
							memcpy(&jsonMessage[0], jsonMsg.msgBase.jsonMsgBuffer.data, jsonMsg.msgBase.jsonMsgBuffer.length > (RSSL_REACTOR_MAX_JSON_ERROR_MSG_SIZE - 1) ? (RSSL_REACTOR_MAX_JSON_ERROR_MSG_SIZE - 1) : jsonMsg.msgBase.jsonMsgBuffer.length);
							rsslSetErrorInfo(pError, RSSL_EIC_FAILURE, RSSL_RET_FAILURE, __FILE__, __LINE__, "Received JSON error message: %s.", &jsonMessage[0]);
							failedToConvertJSONMsg = RSSL_FALSE;
							ret = RSSL_RET_FAILURE;
							break;
						}
					}

					if (ret != RSSL_RET_SUCCESS)
						break;

					failedToConvertJSONMsg = RSSL_TRUE; /* Reset the flag to its initial state. */
				}
			}
			else
			{
				rsslSetErrorInfo(pError, RSSL_EIC_FAILURE, RSSL_RET_FAILURE, __FILE__, __LINE__, "Failed to parse JSON message: %s.", &rjcError.text[0]);
				failedToConvertJSONMsg = RSSL_FALSE;
				rjcError.rsslErrorId = ret;
			}

			if (ret < RSSL_RET_SUCCESS)
			{
				RsslGetJsonErrorParams errorParams;
				RsslBuffer outputBuffer = RSSL_INIT_BUFFER;
				if (failedToConvertJSONMsg) /* Send JSON error message back when it fails to decode JSON message */
				{
					if ( (ret = rsslGetJsonSimpleErrorParams(pReactorImpl->pJsonConverter, &decodeOptions,
						&rjcError, &errorParams, pMsgBuf, jsonMsg.jsonRsslMsg.rsslMsg.msgBase.streamId)) == RSSL_RET_SUCCESS )
					{
						RsslBuffer *pBuffer = NULL;
						rsslJsonGetErrorMessage(pReactorImpl->pJsonConverter, &errorParams, &outputBuffer);

						pBuffer = rsslReactorGetBuffer(&pReactorChannel->reactorChannel, outputBuffer.length, RSSL_FALSE, pError);

						if (pBuffer)
						{
							memcpy(pBuffer->data, outputBuffer.data, outputBuffer.length);

							/* Reply with JSON ERROR message to the sender */
							ret = _reactorSendJSONMessage(pReactorImpl, pReactorChannel, pBuffer, pError);
						}
						else
						{
							ret = RSSL_RET_FAILURE;
						}
					}
				}

				/* Notifies JSON conversion error messages if the callback is specified by users */
				if ( (pReactorImpl->pJsonConversionEventCallback) && (rjcError.rsslErrorId != RSSL_RET_SUCCESS) )
				{
					RsslReactorCallbackRet cret = RSSL_RC_CRET_SUCCESS;
					RsslReactorJsonConversionEvent jsonConversionEvent;

					rsslClearReactorJsonConversionEvent(&jsonConversionEvent);

					if(failedToConvertJSONMsg)
						rsslSetErrorInfo(pReactorImpl->pJsonErrorInfo, RSSL_EIC_FAILURE, rjcError.rsslErrorId, __FILE__, __LINE__, "Failed to convert JSON message: %s", outputBuffer.data);
					else
						rsslSetErrorInfo(pReactorImpl->pJsonErrorInfo, RSSL_EIC_FAILURE, rjcError.rsslErrorId, __FILE__, __LINE__, "Failed to convert JSON message: %s", &rjcError.text[0]);


					jsonConversionEvent.pError = pReactorImpl->pJsonErrorInfo;
					jsonConversionEvent.pUserSpec = pReactorImpl->userSpecPtr;

					_reactorSetInCallback(pReactorImpl, RSSL_TRUE);
					cret = (*pReactorImpl->pJsonConversionEventCallback)((RsslReactor*)pReactorImpl, (RsslReactorChannel*)pReactorChannel, &jsonConversionEvent);
					_reactorSetInCallback(pReactorImpl, RSSL_FALSE);

					if (cret != RSSL_RC_CRET_SUCCESS)
					{
						rsslSetErrorInfo(pError, RSSL_EIC_FAILURE, RSSL_RET_FAILURE, __FILE__, __LINE__, "Error return code %d from callback.", cret);
						return RSSL_RET_FAILURE;
					}
				}

				/* Don't closes the channel when this function can reply the JSON ERROR message back. */
				if (pReactorImpl->closeChannelFromFailure && (ret != RSSL_RET_SUCCESS) )
				{
					/* Closes the channel when fails to convert the data */
					rsslSetErrorInfoLocation(pError, __FILE__, __LINE__);
					if (_reactorHandleChannelDown(pReactorImpl, pReactorChannel, pError) != RSSL_RET_SUCCESS)
						return RSSL_RET_FAILURE;
					pReactorChannel->readRet = 0;
					return RSSL_RET_SUCCESS; /* Problem handled, so return success */
				}
			}

			return RSSL_RET_SUCCESS;
		}
		/* an RSSL_RWF_PROTOCOL_TYPE */
		else
		{
			return(_processRsslRwfMessage(pReactorImpl, pReactorChannel, &readOutArgs, pMsgBuf, pError));
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

				/* Collects ping statistics */
				if ( (pReactorChannel->statisticFlags & RSSL_RC_ST_PING)  && pReactorChannel->pChannelStatistic)
				{
					_cumulativeValue(&pReactorChannel->pChannelStatistic->pingReceived, (RsslUInt32)1);
				}

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
					RsslReactorChannel *pCallbackChannel = (RsslReactorChannel*)pReactorChannel;

					/* Descriptor changed, update notification. */
					if (rsslNotifierUpdateEventFd(pReactorImpl->pNotifier, pReactorChannel->pNotifierEvent, (int)(pChannel->socketId)) < 0)
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

					if (_reactorHandlesWarmStandby(pReactorChannel))
					{
						pCallbackChannel = &pReactorChannel->pWarmStandByHandlerImpl->mainReactorChannelImpl.reactorChannel;
						pCallbackChannel->pRsslChannel = pReactorChannel->reactorChannel.pRsslChannel;
						pCallbackChannel->socketId = pReactorChannel->reactorChannel.socketId;
						pCallbackChannel->oldSocketId = pReactorChannel->reactorChannel.oldSocketId;
						pCallbackChannel->majorVersion = pReactorChannel->reactorChannel.majorVersion;
						pCallbackChannel->minorVersion = pReactorChannel->reactorChannel.minorVersion;
						pCallbackChannel->protocolType = pReactorChannel->reactorChannel.protocolType;
						pCallbackChannel->userSpecPtr = pReactorChannel->reactorChannel.userSpecPtr;

						_reactorHandlesWSBSocketList(pReactorChannel->pWarmStandByHandlerImpl, pCallbackChannel, NULL);
					}
					else
					{
						/* Copy changed sockets to reactor channel */
						pReactorChannel->reactorChannel.socketId = pChannel->socketId;
						pReactorChannel->reactorChannel.oldSocketId = pChannel->oldSocketId;
					}

					/* Inform client of change */
					rsslClearReactorChannelEventImpl(&rsslEvent.channelEventImpl);
					rsslEvent.channelEventImpl.channelEvent.channelEventType = RSSL_RC_CET_FD_CHANGE;
					rsslEvent.channelEventImpl.channelEvent.pReactorChannel = pCallbackChannel;
					
					_reactorSetInCallback(pReactorImpl, RSSL_TRUE);
					(*pReactorChannel->channelRole.base.channelEventCallback)((RsslReactor*)pReactorImpl, pCallbackChannel, &rsslEvent.channelEventImpl.channelEvent);
					_reactorSetInCallback(pReactorImpl, RSSL_FALSE);

					rsslEvent.channelEventImpl.channelEvent.pReactorChannel = (RsslReactorChannel*)pReactorChannel;

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
					RsslReactorChannel *pCallbackChannel = (RsslReactorChannel*)pReactorChannel;

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

					if (_reactorHandlesWarmStandby(pReactorChannel))
					{
						pCallbackChannel = &pReactorChannel->pWarmStandByHandlerImpl->mainReactorChannelImpl.reactorChannel;
						pCallbackChannel->pRsslChannel = pReactorChannel->reactorChannel.pRsslChannel;
						pCallbackChannel->socketId = pReactorChannel->reactorChannel.socketId;
						pCallbackChannel->oldSocketId = pReactorChannel->reactorChannel.oldSocketId;
						pCallbackChannel->majorVersion = pReactorChannel->reactorChannel.majorVersion;
						pCallbackChannel->minorVersion = pReactorChannel->reactorChannel.minorVersion;
						pCallbackChannel->protocolType = pReactorChannel->reactorChannel.protocolType;
						pCallbackChannel->userSpecPtr = pReactorChannel->reactorChannel.userSpecPtr;
					}

					rsslClearReactorChannelEventImpl(&rsslEvent.channelEventImpl);
					rsslEvent.channelEventImpl.channelEvent.channelEventType = RSSL_RC_CET_WARNING;
					rsslEvent.channelEventImpl.channelEvent.pReactorChannel = pCallbackChannel;
					rsslEvent.channelEventImpl.channelEvent.pError = pError;
					rsslSetErrorInfoLocation(pError, __FILE__, __LINE__);
					
					_reactorSetInCallback(pReactorImpl, RSSL_TRUE);
					(*pReactorChannel->channelRole.base.channelEventCallback)((RsslReactor*)pReactorImpl, pCallbackChannel, &rsslEvent.channelEventImpl.channelEvent);
					_reactorSetInCallback(pReactorImpl, RSSL_FALSE);

					rsslEvent.channelEventImpl.channelEvent.pReactorChannel = (RsslReactorChannel*)pReactorChannel;

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

	if (pReactorImpl->state != RSSL_REACTOR_ST_ACTIVE)
	{
		rsslSetErrorInfo(pError, RSSL_EIC_SHUTDOWN, RSSL_RET_INVALID_ARGUMENT, __FILE__, __LINE__, "Reactor is shutting down.");
		return (reactorUnlockInterface(pReactorImpl), RSSL_RET_FAILURE);
	}

	if ((pTunnelStream = tunnelManagerOpenStream(pReactorChannelImpl->pTunnelManager, pOptions, RSSL_FALSE, NULL, COS_CURRENT_STREAM_VERSION, pError)) == NULL)
		return (reactorUnlockInterface(pReactorImpl), pError->rsslError.rsslErrorId);

	pTunnelStream->pReactorChannel = pReactorChannel;

	if (_reactorHandleTunnelManagerEvents(pReactorImpl, pReactorChannelImpl, pError) != RSSL_RET_SUCCESS)
		return (reactorUnlockInterface(pReactorImpl), pError->rsslError.rsslErrorId);

	return (reactorUnlockInterface(pReactorImpl), RSSL_RET_SUCCESS);
}

RSSL_VA_API RsslRet rsslReactorCloseTunnelStream(RsslTunnelStream *pTunnelStream, RsslTunnelStreamCloseOptions *pOptions, RsslErrorInfo *pError)
{
	RsslRet ret;
	RsslReactorChannelImpl *pReactorChannelImpl;
	RsslReactorImpl *pReactorImpl;

	if (pTunnelStream == NULL)
	{
		rsslSetErrorInfo(pError, RSSL_EIC_FAILURE, RSSL_RET_INVALID_ARGUMENT, __FILE__, __LINE__, "RsslTunnelStream not provided.");
		return RSSL_RET_INVALID_ARGUMENT;
	}

	if (pError == NULL)
		return RSSL_RET_INVALID_ARGUMENT;

	pReactorChannelImpl = (RsslReactorChannelImpl*)pTunnelStream->pReactorChannel;
	pReactorImpl = (RsslReactorImpl*)pReactorChannelImpl->pParentReactor;
	
	if ((ret = reactorLockInterface(pReactorImpl, RSSL_TRUE, pError)) != RSSL_RET_SUCCESS)
		return ret;

	if (pReactorImpl->state != RSSL_REACTOR_ST_ACTIVE)
	{
		rsslSetErrorInfo(pError, RSSL_EIC_SHUTDOWN, RSSL_RET_INVALID_ARGUMENT, __FILE__, __LINE__, "Reactor is shutting down.");
		return (reactorUnlockInterface(pReactorImpl), RSSL_RET_FAILURE);
	}

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

	if (pReactorImpl->state != RSSL_REACTOR_ST_ACTIVE)
	{
		rsslSetErrorInfo(pError, RSSL_EIC_SHUTDOWN, RSSL_RET_INVALID_ARGUMENT, __FILE__, __LINE__, "Reactor is shutting down.");
		return (reactorUnlockInterface(pReactorImpl), RSSL_RET_FAILURE);
	}

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

	if (pReactorImpl->state != RSSL_REACTOR_ST_ACTIVE)
	{
		rsslSetErrorInfo(pError, RSSL_EIC_SHUTDOWN, RSSL_RET_INVALID_ARGUMENT, __FILE__, __LINE__, "Reactor is shutting down.");
		return (reactorUnlockInterface(pReactorImpl), RSSL_RET_FAILURE);
	}

	if ((ret = tunnelManagerAcceptStream(pReactorChannelImpl->pTunnelManager, pEvent, pOptions, pError)) != RSSL_RET_SUCCESS)
		return (reactorUnlockInterface(pReactorImpl), ret);

	if (isReactorDebugLevelEnabled(pReactorImpl, RSSL_RC_DEBUG_LEVEL_TUNELSTREAM))
	{
		if (pReactorImpl->pReactorDebugInfo == NULL || pReactorChannelImpl->pChannelDebugInfo == NULL)
		{
			if (_initReactorAndChannelDebugInfo(pReactorImpl, pReactorChannelImpl, pError) != RSSL_RET_SUCCESS)
			{
				_reactorShutdown(pReactorImpl, pError);
				_reactorSendShutdownEvent(pReactorImpl, pError);
				return (reactorUnlockInterface(pReactorImpl), RSSL_RET_FAILURE);
			}
		}

		pReactorChannelImpl->pChannelDebugInfo->debugInfoState |= RSSL_RC_DEBUG_INFO_ACCEPT_TUNNEL_REQUEST;

		_writeDebugInfo(pReactorImpl, "Reactor("RSSL_REACTOR_POINTER_PRINT_TYPE"), Reactor channel("RSSL_REACTOR_POINTER_PRINT_TYPE") ACCEPTS a tunnel stream REQUEST(stream ID=%d) on channel fd="RSSL_REACTOR_SOCKET_PRINT_TYPE".]\n",
			pReactorImpl, pReactorChannelImpl, pEvent->streamId, pReactorChannelImpl->reactorChannel.socketId);
	}

	if (pReactorChannelImpl->tunnelDispatchEventQueued)
	{
		if (isReactorDebugLevelEnabled(pReactorImpl, RSSL_RC_DEBUG_LEVEL_TUNELSTREAM))
		{
			if (pReactorImpl->pReactorDebugInfo == NULL || pReactorChannelImpl->pChannelDebugInfo == NULL)
			{
				if (_initReactorAndChannelDebugInfo(pReactorImpl, pReactorChannelImpl, pError) != RSSL_RET_SUCCESS)
				{
					_reactorShutdown(pReactorImpl, pError);
					_reactorSendShutdownEvent(pReactorImpl, pError);
					return (reactorUnlockInterface(pReactorImpl), RSSL_RET_FAILURE);
				}
			}

			_writeDebugInfo(pReactorImpl, "Reactor("RSSL_REACTOR_POINTER_PRINT_TYPE"), Reactor channel("RSSL_REACTOR_POINTER_PRINT_TYPE") has invalid state of tunnelDispatchEventQueued(%d) for tunnel stream (stream ID=%d) on channel fd="RSSL_REACTOR_SOCKET_PRINT_TYPE".]\n",
				pReactorImpl, pReactorChannelImpl, pReactorChannelImpl->tunnelDispatchEventQueued, pEvent->streamId, pReactorChannelImpl->reactorChannel.socketId);
		}

		pReactorChannelImpl->tunnelDispatchEventQueued = RSSL_FALSE;
	}

	// send a dispatch tunnel stream event if necessary
	if (pReactorChannelImpl->pTunnelManager->_needsDispatchNow && !pReactorChannelImpl->tunnelDispatchEventQueued)
	{
		RsslReactorChannelEventImpl *pDispatchEvent;
		pReactorChannelImpl->tunnelDispatchEventQueued = RSSL_TRUE;
		pDispatchEvent = (RsslReactorChannelEventImpl*)rsslReactorEventQueueGetFromPool(&pReactorChannelImpl->eventQueue);
		rsslClearReactorChannelEventImpl(pDispatchEvent);
		pDispatchEvent->channelEvent.channelEventType = (RsslReactorChannelEventType)RSSL_RCIMPL_CET_DISPATCH_TUNNEL_STREAM;
		pDispatchEvent->channelEvent.pReactorChannel = (RsslReactorChannel*)pReactorChannelImpl;

		if (isReactorDebugLevelEnabled(pReactorImpl, RSSL_RC_DEBUG_LEVEL_TUNELSTREAM))
		{
			if (pReactorImpl->pReactorDebugInfo == NULL || pReactorChannelImpl->pChannelDebugInfo == NULL)
			{
				if (_initReactorAndChannelDebugInfo(pReactorImpl, pReactorChannelImpl, pError) != RSSL_RET_SUCCESS)
				{
					_reactorShutdown(pReactorImpl, pError);
					_reactorSendShutdownEvent(pReactorImpl, pError);
					return (reactorUnlockInterface(pReactorImpl), RSSL_RET_FAILURE);
				}
			}

			_writeDebugInfo(pReactorImpl, "Reactor("RSSL_REACTOR_POINTER_PRINT_TYPE"), Reactor channel("RSSL_REACTOR_POINTER_PRINT_TYPE") put an tunnel stream event(stream ID=%d) on channel fd="RSSL_REACTOR_SOCKET_PRINT_TYPE".]\n",
				pReactorImpl, pReactorChannelImpl, pEvent->streamId, pReactorChannelImpl->reactorChannel.socketId);
		}


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

	if (pReactorImpl->state != RSSL_REACTOR_ST_ACTIVE)
	{
		rsslSetErrorInfo(pError, RSSL_EIC_SHUTDOWN, RSSL_RET_INVALID_ARGUMENT, __FILE__, __LINE__, "Reactor is shutting down.");
		return (reactorUnlockInterface(pReactorImpl), RSSL_RET_FAILURE);
	}

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

	if (pReactorImpl->state != RSSL_REACTOR_ST_ACTIVE)
	{
		rsslSetErrorInfo(pError, RSSL_EIC_SHUTDOWN, RSSL_RET_INVALID_ARGUMENT, __FILE__, __LINE__, "Reactor is shutting down.");
		return (reactorUnlockInterface(pReactorImpl), RSSL_RET_FAILURE);
	}

	if ((ret = tunnelManagerSubmit((TunnelManager *)pTunnelManagerImpl, pTunnelStream, pRsslTunnelStreamSubmitMsgOptions, pError)) != RSSL_RET_SUCCESS)
		return (reactorUnlockInterface(pReactorChannelImpl->pParentReactor), RSSL_RET_FAILURE);

	// send a dispatch tunnel stream event if necessary
	if ((ret = _reactorHandleTunnelManagerEvents(pReactorImpl, pReactorChannelImpl, pError)) != RSSL_RET_SUCCESS)
		return (reactorUnlockInterface(pReactorImpl), ret);

	return (reactorUnlockInterface(pReactorChannelImpl->pParentReactor), RSSL_RET_SUCCESS);
}

RSSL_VA_API RsslRet rsslReactorSubmitOAuthCredentialRenewal(RsslReactor *pReactor, RsslReactorOAuthCredentialRenewalOptions *pOptions,
	RsslReactorOAuthCredentialRenewal *pReactorOAuthCredentialRenewal, RsslErrorInfo *pError)
{
	RsslRet ret;
	RsslReactorImpl *pReactorImpl = (RsslReactorImpl*)pReactor;
	RsslReactorOAuthCredentialRenewal *pOAuthCredentialRenewal = NULL;
	RsslReactorTokenSessionImpl *pTokenSessionImpl = NULL;

	if (!pError)
		return RSSL_RET_INVALID_ARGUMENT;

	if (!pReactor)
	{
		rsslSetErrorInfo(pError, RSSL_EIC_FAILURE, RSSL_RET_INVALID_ARGUMENT, __FILE__, __LINE__, "RsslReactor not provided.");
		return RSSL_RET_INVALID_ARGUMENT;
	}

	if ((ret = reactorLockInterface(pReactorImpl, RSSL_TRUE, pError)) != RSSL_RET_SUCCESS)
		return ret;

	if (pReactorImpl->state != RSSL_REACTOR_ST_ACTIVE)
	{
		rsslSetErrorInfo(pError, RSSL_EIC_SHUTDOWN, RSSL_RET_INVALID_ARGUMENT, __FILE__, __LINE__, "Reactor is shutting down.");
		goto submitFailed;
	}

	/* Checks whether the token session is available from calling with in the callback method */
	pTokenSessionImpl = pReactorImpl->pTokenSessionForCredentialRenewalCallback;

	if (!pOptions)
	{
		rsslSetErrorInfo(pError, RSSL_EIC_FAILURE, RSSL_RET_INVALID_ARGUMENT, __FILE__, __LINE__, "RsslReactorOAuthCredentialRenewalOptions not provided.");
		goto submitFailed;
	}

	if (!pReactorOAuthCredentialRenewal)
	{
		rsslSetErrorInfo(pError, RSSL_EIC_FAILURE, RSSL_RET_INVALID_ARGUMENT, __FILE__, __LINE__, "RsslReactorOAuthCredentialRenewal not provided.");
		goto submitFailed;
	}

	/* Use the user name from the token session instead if not specified */
	if (pTokenSessionImpl == 0)
	{
		if (pReactorOAuthCredentialRenewal->userName.data && pReactorOAuthCredentialRenewal->userName.length && 
			pReactorOAuthCredentialRenewal->clientId.data && pReactorOAuthCredentialRenewal->clientId.length && 
			pReactorOAuthCredentialRenewal->password.data && pReactorOAuthCredentialRenewal->password.length)
		{
			/* No op */
		}
		else if (pReactorOAuthCredentialRenewal->clientId.data && pReactorOAuthCredentialRenewal->clientId.length && 
				pReactorOAuthCredentialRenewal->clientSecret.data && pReactorOAuthCredentialRenewal->clientSecret.length)
		{
			/* No op */
		}
		else
		{
			rsslSetErrorInfo(pError, RSSL_EIC_FAILURE, RSSL_RET_INVALID_ARGUMENT, __FILE__, __LINE__, "RsslReactorOAuthCredentialRenewal invalid credentials provdied.");
			goto submitFailed;
		}
	}
	else
	{
		if (pTokenSessionImpl->sessionVersion == RSSL_RC_SESSMGMT_V1)
		{
			if (pReactorOAuthCredentialRenewal->password.length == 0 || pReactorOAuthCredentialRenewal->password.data == 0)
			{
				rsslSetErrorInfo(pError, RSSL_EIC_FAILURE, RSSL_RET_INVALID_ARGUMENT, __FILE__, __LINE__, "RsslReactorOAuthCredentialRenewal.password not provided.");
				goto submitFailed;
			}
		}
		else
		{
			if (pReactorOAuthCredentialRenewal->clientSecret.length == 0 || (pReactorOAuthCredentialRenewal->clientSecret.data == 0))
			{
				rsslSetErrorInfo(pError, RSSL_EIC_FAILURE, RSSL_RET_INVALID_ARGUMENT, __FILE__, __LINE__, "RsslReactorOAuthCredentialRenewal.clientSecret not provided.");
				goto submitFailed;
			}
		}
	}

	// Initialize and create RsslRestClient if does not exist
	if (rsslReactorCreateRestClient(pReactorImpl, pError) != RSSL_RET_SUCCESS)
	{
		goto submitFailed;
	}

	switch (pOptions->renewalMode)
	{
	case RSSL_ROC_RT_RENEW_TOKEN_WITH_PASSWORD:
		break;
	case RSSL_ROC_RT_RENEW_TOKEN_WITH_PASSWORD_CHANGE:
	{
		if (pTokenSessionImpl)
		{
			if (pTokenSessionImpl->pOAuthCredential->pOAuthCredentialEventCallback == 0)
			{
				rsslSetErrorInfo(pError, RSSL_EIC_FAILURE, RSSL_RET_INVALID_ARGUMENT, __FILE__, __LINE__, "Support changing password of the token session when RsslReactorOAuthCredentialEventCallback is specified in RsslReactorOAuthCredential only.");
				goto submitFailed;
			}

			if (pTokenSessionImpl->sessionVersion == RSSL_RC_SESSMGMT_V1)
			{
				if (pReactorOAuthCredentialRenewal->newPassword.length == 0 || pReactorOAuthCredentialRenewal->newPassword.data == 0)
				{
					rsslSetErrorInfo(pError, RSSL_EIC_FAILURE, RSSL_RET_INVALID_ARGUMENT, __FILE__, __LINE__, "RsslReactorOAuthCredentialRenewal.newPassword not provided.");
					goto submitFailed;
				}
			}
		}
		
		break;
	}
	default:
		rsslSetErrorInfo(pError, RSSL_EIC_FAILURE, RSSL_RET_INVALID_ARGUMENT, __FILE__, __LINE__, "Invalid RsslReactorOAuthCredentialRenewalType mode(%d).", pOptions->renewalMode);
		goto submitFailed;
	}

	if (pTokenSessionImpl)
	{
		if ((pOAuthCredentialRenewal = _reactorCopyRsslReactorOAuthCredentialRenewal(pReactorImpl, pTokenSessionImpl, pOptions, pReactorOAuthCredentialRenewal, pError)) == NULL)
		{
			goto submitFailed;
		}

		if (_reactorSendCredentialRenewalRequest(pReactorImpl, pTokenSessionImpl, pOAuthCredentialRenewal, (RsslReactorOAuthCredentialRenewalEventType)pOptions->renewalMode, pError) != RSSL_RET_SUCCESS)
		{
			goto submitFailed;
		}
	}
	else
	{
		if ((pOAuthCredentialRenewal = _reactorCopyRsslReactorOAuthCredentialRenewal(pReactorImpl, NULL, pOptions, pReactorOAuthCredentialRenewal, pError)) == NULL)
		{
			goto submitFailed;
		}

		if (_reactorSendCredentialRenewalRequest(pReactorImpl, NULL, pOAuthCredentialRenewal, (RsslReactorOAuthCredentialRenewalEventType)pOptions->renewalMode, pError) != RSSL_RET_SUCCESS)
		{
			goto submitFailed;
		}
	}

	return (reactorUnlockInterface(pReactorImpl), RSSL_RET_SUCCESS);

submitFailed:

	if (pOAuthCredentialRenewal) free(pOAuthCredentialRenewal);

	return (reactorUnlockInterface(pReactorImpl), pError->rsslError.rsslErrorId);
}

RSSL_VA_API RsslRet rsslReactorRetrieveChannelStatistic(RsslReactor *pReactor, RsslReactorChannel *pReactorChannel,
	RsslReactorChannelStatistic *pRsslReactorChannelStatistic, RsslErrorInfo *pError)
{
	RsslRet ret;
	RsslReactorImpl *pReactorImpl = (RsslReactorImpl*)pReactor;
	RsslReactorChannelImpl *pReactorChannelImpl = (RsslReactorChannelImpl*)pReactorChannel;

	if (!pError)
		return RSSL_RET_INVALID_ARGUMENT;

	if (!pReactor)
	{
		rsslSetErrorInfo(pError, RSSL_EIC_FAILURE, RSSL_RET_INVALID_ARGUMENT, __FILE__, __LINE__, "RsslReactor not provided.");
		return RSSL_RET_INVALID_ARGUMENT;
	}

	if ((ret = reactorLockInterface(pReactorImpl, RSSL_TRUE, pError)) != RSSL_RET_SUCCESS)
		return ret;

	if (pReactorImpl->state != RSSL_REACTOR_ST_ACTIVE)
	{
		rsslSetErrorInfo(pError, RSSL_EIC_SHUTDOWN, RSSL_RET_INVALID_ARGUMENT, __FILE__, __LINE__, "Reactor is shutting down.");
		return (reactorUnlockInterface(pReactorImpl), RSSL_RET_INVALID_ARGUMENT);
	}

	if (!pReactorChannel)
	{
		rsslSetErrorInfo(pError, RSSL_EIC_FAILURE, RSSL_RET_INVALID_ARGUMENT, __FILE__, __LINE__, "RsslReactorChannel not provided.");
		return (reactorUnlockInterface(pReactorImpl), RSSL_RET_INVALID_ARGUMENT);
	}

	if (!pRsslReactorChannelStatistic)
	{
		rsslSetErrorInfo(pError, RSSL_EIC_FAILURE, RSSL_RET_INVALID_ARGUMENT, __FILE__, __LINE__, "RsslReactorChannelStatistic not provided.");
		return (reactorUnlockInterface(pReactorImpl), RSSL_RET_INVALID_ARGUMENT);
	}

	if ( (pReactorChannelImpl->statisticFlags & (RSSL_RC_ST_READ | RSSL_RC_ST_WRITE | RSSL_RC_ST_PING)) == 0 )
	{
		rsslSetErrorInfo(pError, RSSL_EIC_FAILURE, RSSL_RET_INVALID_ARGUMENT, __FILE__, __LINE__, "RsslReactorChannel not interested in channel statistics.");
		return (reactorUnlockInterface(pReactorImpl), RSSL_RET_INVALID_ARGUMENT);
	}

	rsslClearReactorChannelStatistic(pRsslReactorChannelStatistic);
	if (pReactorChannelImpl->pChannelStatistic)
	{
		*pRsslReactorChannelStatistic = *pReactorChannelImpl->pChannelStatistic;

		/* Always reset the aggregated values after this function calls */
		rsslClearReactorChannelStatistic(pReactorChannelImpl->pChannelStatistic);
	}

	return (reactorUnlockInterface(pReactorImpl), RSSL_RET_SUCCESS);
}

RsslUInt32 packedBufferHashU64Sum(void* pReactorPackedBufferImpl)
{
	return (RsslUInt32)((RsslUInt64)pReactorPackedBufferImpl);
}

RsslBool packedBufferHashU64Compare(void *element1, void *element2)
{
	return (element1 == element2);
}

RSSL_VA_API RsslRet rsslReactorGetChannelInfo(RsslReactorChannel* pReactorChannel, RsslReactorChannelInfo* pInfo, RsslErrorInfo* pError)
{
	RsslRet ret = RSSL_RET_SUCCESS;
	if (!pError)
		return RSSL_RET_INVALID_ARGUMENT;
	if (!pReactorChannel)
	{
		rsslSetErrorInfo(pError, RSSL_EIC_FAILURE, RSSL_RET_INVALID_ARGUMENT, __FILE__, __LINE__, "RsslReactorChannel is not provided.");
		return RSSL_RET_INVALID_ARGUMENT;
	}

	if (!pReactorChannel->pRsslChannel)
	{
		rsslSetErrorInfo(pError, RSSL_EIC_FAILURE, RSSL_RET_INVALID_ARGUMENT, __FILE__, __LINE__, "RsslChannel is not provided.");
		return RSSL_RET_INVALID_ARGUMENT;
	}
	else
	{
		RsslReactorChannelImpl* pReactorChannelImpl = (RsslReactorChannelImpl*)pReactorChannel;
		RsslReactorImpl* pReactorImpl = pReactorChannelImpl->pParentReactor;

		if (pReactorImpl->state != RSSL_REACTOR_ST_ACTIVE)
		{
			rsslSetErrorInfo(pError, RSSL_EIC_SHUTDOWN, RSSL_RET_INVALID_ARGUMENT, __FILE__, __LINE__, "Reactor is shutting down.");
			return RSSL_RET_FAILURE;
		}
	}

	ret = rsslGetChannelInfo(pReactorChannel->pRsslChannel, &pInfo->rsslChannelInfo, &pError->rsslError);
	if (ret != RSSL_RET_SUCCESS)
		rsslSetErrorInfoLocation(pError, __FILE__, __LINE__);
	return ret;
}

RSSL_VA_API RsslRet rsslReactorGetChannelStats(RsslReactorChannel* pReactorChannel, RsslReactorChannelStats* pInfo, RsslErrorInfo* pError)
{
	RsslRet ret = RSSL_RET_SUCCESS;
	if (!pError)
		return RSSL_RET_INVALID_ARGUMENT;
	if (!pReactorChannel)
	{
		rsslSetErrorInfo(pError, RSSL_EIC_FAILURE, RSSL_RET_INVALID_ARGUMENT, __FILE__, __LINE__, "RsslReactorChannel is not provided.");
		return RSSL_RET_INVALID_ARGUMENT;
	}
	if (!pInfo)
	{
		rsslSetErrorInfo(pError, RSSL_EIC_FAILURE, RSSL_RET_INVALID_ARGUMENT, __FILE__, __LINE__, "RsslReactorChannelStats is not provided.");
		return RSSL_RET_INVALID_ARGUMENT;
	}

	if (!pReactorChannel->pRsslChannel)
	{
		rsslSetErrorInfo(pError, RSSL_EIC_FAILURE, RSSL_RET_INVALID_ARGUMENT, __FILE__, __LINE__, "RsslChannel is not provided.");
		return RSSL_RET_INVALID_ARGUMENT;
	}
	else
	{
		RsslReactorChannelImpl* pReactorChannelImpl = (RsslReactorChannelImpl*)pReactorChannel;
		RsslReactorImpl* pReactorImpl = pReactorChannelImpl->pParentReactor;

		if (pReactorImpl->state != RSSL_REACTOR_ST_ACTIVE)
		{
			rsslSetErrorInfo(pError, RSSL_EIC_SHUTDOWN, RSSL_RET_INVALID_ARGUMENT, __FILE__, __LINE__, "Reactor is shutting down.");
			return RSSL_RET_FAILURE;
		}
	}

	ret = rsslGetChannelStats(pReactorChannel->pRsslChannel, &pInfo->rsslChannelStats, &pError->rsslError);
	if (ret != RSSL_RET_SUCCESS)
		rsslSetErrorInfoLocation(pError, __FILE__, __LINE__);
	return ret;
}

RSSL_VA_API RsslInt32 rsslReactorChannelBufferUsage(RsslReactorChannel* pReactorChannel, RsslErrorInfo* pError)
{
	RsslRet ret = RSSL_RET_SUCCESS;
	if (!pError)
		return RSSL_RET_INVALID_ARGUMENT;
	if (!pReactorChannel)
	{
		rsslSetErrorInfo(pError, RSSL_EIC_FAILURE, RSSL_RET_INVALID_ARGUMENT, __FILE__, __LINE__, "RsslReactorChannel is not provided.");
		return RSSL_RET_INVALID_ARGUMENT;
	}

	if (!pReactorChannel->pRsslChannel)
	{
		rsslSetErrorInfo(pError, RSSL_EIC_FAILURE, RSSL_RET_INVALID_ARGUMENT, __FILE__, __LINE__, "RsslChannel is not provided.");
		return RSSL_RET_INVALID_ARGUMENT;
	}
	else
	{
		RsslReactorChannelImpl* pReactorChannelImpl = (RsslReactorChannelImpl*)pReactorChannel;
		RsslReactorImpl* pReactorImpl = pReactorChannelImpl->pParentReactor;

		if (pReactorImpl->state != RSSL_REACTOR_ST_ACTIVE)
		{
			rsslSetErrorInfo(pError, RSSL_EIC_SHUTDOWN, RSSL_RET_INVALID_ARGUMENT, __FILE__, __LINE__, "Reactor is shutting down.");
			return RSSL_RET_FAILURE;
		}
	}

	ret = rsslBufferUsage(pReactorChannel->pRsslChannel, &pError->rsslError);
	if (ret < RSSL_RET_SUCCESS)
		rsslSetErrorInfoLocation(pError, __FILE__, __LINE__);
	return ret;
}

RSSL_VA_API RsslBuffer* rsslReactorGetBuffer(RsslReactorChannel *channel, RsslUInt32 size, RsslBool packedBuffer, RsslErrorInfo *pError)
{
	RsslRet ret;
	RsslBuffer *pBuffer = NULL;

	if ( (channel->pRsslChannel && channel->pRsslChannel->protocolType == RSSL_JSON_PROTOCOL_TYPE) && packedBuffer )
	{
		RsslReactorChannelImpl *pReactorChannel = (RsslReactorChannelImpl*)channel;
		RsslReactorImpl *pReactorImpl = pReactorChannel->pParentReactor;
		RsslReactorPackedBufferImpl *pPackedBufferImpl = NULL;

		if ((ret = reactorLockInterface(pReactorImpl, RSSL_TRUE, pError)) != RSSL_RET_SUCCESS)
			return pBuffer;

		if (pReactorImpl->state != RSSL_REACTOR_ST_ACTIVE)
		{
			rsslSetErrorInfo(pError, RSSL_EIC_SHUTDOWN, RSSL_RET_INVALID_ARGUMENT, __FILE__, __LINE__, "Reactor is shutting down.");
			return (reactorUnlockInterface(pReactorImpl), pBuffer);
		}

		/* Since the application passed in this channel, make sure it is valid for this reactor and that it is active. */
		if (!pReactorChannel || !rsslReactorChannelIsValid(pReactorImpl, pReactorChannel, pError))
			return (reactorUnlockInterface(pReactorImpl), pBuffer);


		if (pReactorChannel->packedBufferHashTable.queueList == NULL)
		{
			if (rsslHashTableInit(&pReactorChannel->packedBufferHashTable, 10, packedBufferHashU64Sum,
				packedBufferHashU64Compare, RSSL_TRUE, pError) != RSSL_RET_SUCCESS)
			{
				rsslSetErrorInfo(pError, RSSL_EIC_FAILURE, RSSL_RET_FAILURE, __FILE__, __LINE__, "Failed to initialize RsslHashTable for handling packed buffer.");
				return (reactorUnlockInterface(pReactorImpl), pBuffer);
			}
		}

		pPackedBufferImpl = (RsslReactorPackedBufferImpl*)malloc(sizeof(RsslReactorPackedBufferImpl));

		if (!pPackedBufferImpl)
		{
			rsslSetErrorInfo(pError, RSSL_EIC_FAILURE, RSSL_RET_FAILURE, __FILE__, __LINE__, "Failed to create RsslReactorPackedBufferImpl for handling packed buffer.");
			return (reactorUnlockInterface(pReactorImpl), pBuffer);
		}

		rsslClearReactorPackedBufferImpl(pPackedBufferImpl);

		pBuffer = rsslGetBuffer(channel->pRsslChannel, size, packedBuffer, &pError->rsslError);
		if (!pBuffer)
		{
			rsslSetErrorInfoLocation(pError, __FILE__, __LINE__);
			return (reactorUnlockInterface(pReactorImpl), pBuffer);
		}

		pPackedBufferImpl->totalSize = size; /* Keeps the size of the packed buffer */
		pPackedBufferImpl->remainingSize = size;

		rsslHashTableInsertLink(&pReactorChannel->packedBufferHashTable, &pPackedBufferImpl->hashLink, pBuffer, NULL);

		return (reactorUnlockInterface(pReactorImpl), pBuffer);

	}
	else
	{
		pBuffer = rsslGetBuffer(channel->pRsslChannel, size, packedBuffer, &pError->rsslError);
		if (!pBuffer)
			rsslSetErrorInfoLocation(pError, __FILE__, __LINE__);

		return pBuffer;
	}
}

RSSL_VA_API RsslRet rsslReactorReleaseBuffer(RsslReactorChannel *channel, RsslBuffer *pBuffer, RsslErrorInfo *pError)
{
	RsslRet ret = RSSL_RET_FAILURE;
	RsslReactorChannelImpl *pReactorChannel = (RsslReactorChannelImpl*)channel;

	if ( (channel != NULL) && (channel->pRsslChannel != NULL) && (channel->pRsslChannel->protocolType == RSSL_JSON_PROTOCOL_TYPE) )
	{
		RsslHashLink *pHashLink;
		RsslReactorPackedBufferImpl *pPackedBufferImpl = NULL;
		RsslReactorImpl *pReactorImpl = pReactorChannel->pParentReactor;

		if ((ret = reactorLockInterface(pReactorImpl, RSSL_TRUE, pError)) != RSSL_RET_SUCCESS)
			return ret;

		if (pReactorImpl->state != RSSL_REACTOR_ST_ACTIVE)
		{
			rsslSetErrorInfo(pError, RSSL_EIC_SHUTDOWN, RSSL_RET_INVALID_ARGUMENT, __FILE__, __LINE__, "Reactor is shutting down.");
			return (reactorUnlockInterface(pReactorImpl), RSSL_RET_FAILURE);
		}

		if( pReactorChannel->packedBufferHashTable.elementCount != 0 )
		{
			pHashLink = rsslHashTableFind(&pReactorChannel->packedBufferHashTable, pBuffer, NULL);

			if (pHashLink != NULL)
			{
				pPackedBufferImpl = RSSL_HASH_LINK_TO_OBJECT(RsslReactorPackedBufferImpl, hashLink, pHashLink);

				rsslHashTableRemoveLink(&pReactorChannel->packedBufferHashTable, pHashLink);

				free(pPackedBufferImpl);
			}
		}

		ret = rsslReleaseBuffer(pBuffer, &pError->rsslError);
		if (ret != RSSL_RET_SUCCESS)
			rsslSetErrorInfoLocation(pError, __FILE__, __LINE__);

		return (reactorUnlockInterface(pReactorImpl), ret);
	}
	else
	{
		ret = rsslReleaseBuffer(pBuffer, &pError->rsslError);
		if (ret != RSSL_RET_SUCCESS)
			rsslSetErrorInfoLocation(pError, __FILE__, __LINE__);

		return ret;
	}
}

RSSL_VA_API RsslBuffer* rsslReactorPackBuffer(RsslReactorChannel *pChannel, RsslBuffer *pBuffer, RsslErrorInfo *pError)
{
	RsslBuffer *pNewBuffer = NULL;

	if (pChannel->pRsslChannel && pChannel->pRsslChannel->protocolType == RSSL_JSON_PROTOCOL_TYPE)
	{
		RsslRet ret;
		RsslDecodeIterator dIter;
		RsslMsg rsslMsg;
		RsslReactorChannelImpl *pReactorChannel = (RsslReactorChannelImpl*)pChannel;
		RsslReactorImpl *pReactorImpl = pReactorChannel->pParentReactor;
		RsslReactorPackedBufferImpl *pPackedBufferImpl = NULL;
		RsslHashLink *pHashLink;

		if ((ret = reactorLockInterface(pReactorImpl, RSSL_TRUE, pError)) != RSSL_RET_SUCCESS)
			return pNewBuffer;

		/* Since the application passed in this channel, make sure it is valid for this reactor and that it is active. */
		if (!pReactorChannel || !rsslReactorChannelIsValid(pReactorImpl, pReactorChannel, pError))
			return (reactorUnlockInterface(pReactorImpl), pNewBuffer);

		if (pReactorImpl->state != RSSL_REACTOR_ST_ACTIVE)
		{
			rsslSetErrorInfo(pError, RSSL_EIC_SHUTDOWN, RSSL_RET_INVALID_ARGUMENT, __FILE__, __LINE__, "Reactor is shutting down.");
			return (reactorUnlockInterface(pReactorImpl), pNewBuffer);
		}

		if (pReactorChannel->reactorParentQueue != &pReactorImpl->activeChannels)
		{
			rsslSetErrorInfo(pError, RSSL_EIC_FAILURE, RSSL_RET_FAILURE, __FILE__, __LINE__, "Channel is not active.");
			return (reactorUnlockInterface(pReactorImpl), pNewBuffer);
		}

		pHashLink = rsslHashTableFind(&pReactorChannel->packedBufferHashTable, pBuffer, NULL);

		if (pHashLink != NULL)
		{
			pPackedBufferImpl = RSSL_HASH_LINK_TO_OBJECT(RsslReactorPackedBufferImpl, hashLink, pHashLink);
		}
		else
		{
			rsslSetErrorInfo(pError, RSSL_EIC_FAILURE, RSSL_RET_FAILURE, __FILE__, __LINE__, "Failed to find the packed buffer handling for JSON protocol.");
			return (reactorUnlockInterface(pReactorImpl), pNewBuffer);
		}

		/* Added checking to ensure that the JSON converter is initialized properly.*/
		if (pReactorImpl->pJsonConverter == 0)
		{
			rsslSetErrorInfo(pError, RSSL_EIC_FAILURE, RSSL_RET_FAILURE, __FILE__, __LINE__, "The JSON converter library has not been initialized properly.");
			return (reactorUnlockInterface(pReactorImpl), pNewBuffer);
		}

		rsslClearMsg(&rsslMsg);
		rsslClearDecodeIterator(&dIter);
		rsslSetDecodeIteratorRWFVersion(&dIter, pReactorChannel->reactorChannel.pRsslChannel->majorVersion,
			pReactorChannel->reactorChannel.pRsslChannel->minorVersion);

		rsslSetDecodeIteratorBuffer(&dIter, pBuffer);

		ret = rsslDecodeMsg(&dIter, &rsslMsg);

		if (ret == RSSL_RET_SUCCESS)
		{
			RsslConvertRsslMsgToJsonOptions rjcOptions;
			RsslGetJsonMsgOptions getJsonMsgOptions;
			RsslJsonConverterError rjcError;
			RsslBuffer jsonBuffer;
			RsslUInt32 neededSize;

			rsslClearConvertRsslMsgToJsonOptions(&rjcOptions);
			rjcOptions.jsonProtocolType = RSSL_JSON_JPT_JSON2; /* Supported only for Simplified JSON */
			if ((rsslConvertRsslMsgToJson(pReactorImpl->pJsonConverter, &rjcOptions, &rsslMsg, &rjcError)) != RSSL_RET_SUCCESS)
			{
				rsslSetErrorInfo(pError, RSSL_EIC_FAILURE, RSSL_RET_FAILURE, __FILE__, __LINE__,
					"Failed to convert RWF to JSON protocol. Error text: %s", rjcError.text);
				return (reactorUnlockInterface(pReactorImpl), pNewBuffer);
			}

			rsslClearGetJsonMsgOptions(&getJsonMsgOptions);
			getJsonMsgOptions.jsonProtocolType = RSSL_JSON_JPT_JSON2; /* Supported only for Simplified JSON */
			getJsonMsgOptions.streamId = rsslMsg.msgBase.streamId;
			getJsonMsgOptions.isCloseMsg = (rsslMsg.msgBase.msgClass == RSSL_MC_CLOSE) ? RSSL_TRUE : RSSL_FALSE;

			if ((ret = rsslGetConverterJsonMsg(pReactorImpl->pJsonConverter, &getJsonMsgOptions,
				&jsonBuffer, &rjcError)) != RSSL_RET_SUCCESS)
			{
				rsslSetErrorInfo(pError, RSSL_EIC_FAILURE, RSSL_RET_FAILURE, __FILE__, __LINE__,
					"Failed to get converted JSON message. Error text: %s", rjcError.text);
				return (reactorUnlockInterface(pReactorImpl), pNewBuffer);
			}

			if (pPackedBufferImpl->totalSize == pPackedBufferImpl->remainingSize)
			{
				neededSize = jsonBuffer.length;
			}
			else
			{
				neededSize = jsonBuffer.length + 1; /* Plus 1 for handling ',' for JSON array */
			}

			if (neededSize < pPackedBufferImpl->remainingSize)
			{
				pPackedBufferImpl->remainingSize -= neededSize;

				pBuffer->length = jsonBuffer.length;
				memcpy(pBuffer->data, jsonBuffer.data, pBuffer->length);

				pNewBuffer = rsslPackBuffer(pChannel->pRsslChannel, pBuffer, &pError->rsslError);
			}
			else
			{
				rsslSetErrorInfo(pError, RSSL_EIC_FAILURE, RSSL_RET_BUFFER_TOO_SMALL, __FILE__, __LINE__,
					"Failed to pack buffer as the required buffer size(%d) is larger than the remaining packed buffer size(%d).", neededSize, pPackedBufferImpl->remainingSize);
				return (reactorUnlockInterface(pReactorImpl), pNewBuffer);
			}
		}
		else
		{
			rsslSetErrorInfo(pError, RSSL_EIC_FAILURE, ret, __FILE__, __LINE__,
				"rsslDecodeMsg() failed to decode the passed in buffer as RWF messages.");
			return (reactorUnlockInterface(pReactorImpl), pNewBuffer);
		}

		return (reactorUnlockInterface(pReactorImpl), pNewBuffer);
	}
	else
	{
		pNewBuffer = rsslPackBuffer(pChannel->pRsslChannel, pBuffer, &pError->rsslError);
		if (!pNewBuffer)
			rsslSetErrorInfoLocation(pError, __FILE__, __LINE__);

		return pNewBuffer;
	}
}

RSSL_VA_API RsslRet rsslReactorChannelIoctl(RsslReactorChannel *pReactorChannel, int code, void *value, RsslErrorInfo *pError)
{
	RsslRet ret = RSSL_RET_SUCCESS;
	RsslReactorChannelImpl *pReactorChannelImpl = (RsslReactorChannelImpl*)pReactorChannel;
	RsslReactorImpl *pReactorImpl = pReactorChannelImpl->pParentReactor;

	if (!pReactorChannel)
	{
		rsslSetErrorInfo(pError, RSSL_EIC_FAILURE, RSSL_RET_INVALID_ARGUMENT, __FILE__, __LINE__, "RsslReactorChannel is not provided.");
		return RSSL_RET_INVALID_ARGUMENT;
	}

	if (!pReactorChannel->pRsslChannel)
	{
		rsslSetErrorInfo(pError, RSSL_EIC_FAILURE, RSSL_RET_INVALID_ARGUMENT, __FILE__, __LINE__, "RsslChannel is not provided.");
		return RSSL_RET_INVALID_ARGUMENT;
	}

	if ((ret = reactorLockInterface(pReactorImpl, RSSL_TRUE, pError)) != RSSL_RET_SUCCESS)
		return RSSL_RET_FAILURE;

	if (pReactorImpl->state != RSSL_REACTOR_ST_ACTIVE)
	{
		rsslSetErrorInfo(pError, RSSL_EIC_SHUTDOWN, RSSL_RET_INVALID_ARGUMENT, __FILE__, __LINE__, "Reactor is shutting down.");
		return RSSL_RET_FAILURE;
	}

	if (_reactorHandlesWarmStandby(pReactorChannelImpl))
	{
		RsslReactorImpl *pReactorImpl = pReactorChannelImpl->pParentReactor;
		RsslReactorWarmStandByHandlerImpl *pReactorWarmStandByHandlerImpl = pReactorChannelImpl->pWarmStandByHandlerImpl;
		RsslQueueLink* pLink;
		RsslReactorChannelImpl* pApplyReactorChannelImpl;

		switch (code)
		{
			case RSSL_TRACE:
			{
				RsslTraceOptions *pTraceOptions = (RsslTraceOptions*)value;

				pReactorWarmStandByHandlerImpl->traceOptions.traceFlags = pTraceOptions->traceFlags;
				pReactorWarmStandByHandlerImpl->traceOptions.traceMsgMaxFileSize = pTraceOptions->traceMsgMaxFileSize;

				if (pTraceOptions->traceMsgFileName)
				{
					size_t fileNameLength = strlen(pTraceOptions->traceMsgFileName);
					char *pFileNameBuffer = NULL;

					if (pReactorWarmStandByHandlerImpl->traceFileNameLength < (fileNameLength + 1))
					{
						char *pFileNameBuffer = (char*)malloc(fileNameLength + 1);

						if (pFileNameBuffer == NULL)
						{
							rsslSetErrorInfo(pError, RSSL_EIC_FAILURE, ret, __FILE__, __LINE__,
								"Failed to allocate memory for the trace message file name.");
							return (reactorUnlockInterface(pReactorImpl), RSSL_RET_FAILURE);
						}

						pReactorWarmStandByHandlerImpl->traceFileNameLength = (RsslUInt32)(fileNameLength + 1);

						if (pReactorWarmStandByHandlerImpl->traceOptions.traceMsgFileName)
						{
							free(pReactorWarmStandByHandlerImpl->traceOptions.traceMsgFileName);
						}

						memset(pFileNameBuffer, 0, pReactorWarmStandByHandlerImpl->traceFileNameLength);
						memcpy(pFileNameBuffer, pTraceOptions->traceMsgFileName, fileNameLength);

						pReactorWarmStandByHandlerImpl->traceOptions.traceMsgFileName = pFileNameBuffer;
					}
					else
					{
						memset(pReactorWarmStandByHandlerImpl->traceOptions.traceMsgFileName, 0, pReactorWarmStandByHandlerImpl->traceFileNameLength);
						memcpy(pReactorWarmStandByHandlerImpl->traceOptions.traceMsgFileName, pTraceOptions->traceMsgFileName, fileNameLength);
					}

					pReactorWarmStandByHandlerImpl->ioCtlCodes |= RSSL_REACTOR_WS_TRACE;
				}

				break;
			}
			case RSSL_MAX_NUM_BUFFERS:
			{
				pReactorWarmStandByHandlerImpl->ioCtlCodes |= RSSL_REACTOR_WS_MAX_NUM_BUFFERS;
				pReactorWarmStandByHandlerImpl->maxNumBuffers = *(RsslUInt32*)(value);
				break;
			}
			case RSSL_NUM_GUARANTEED_BUFFERS:
			{
				pReactorWarmStandByHandlerImpl->ioCtlCodes |= RSSL_REACTOR_WS_NUM_GUARANTEED_BUFFERS;
				pReactorWarmStandByHandlerImpl->numGuaranteedBuffers = *(RsslUInt32*)(value);
				break;
			}
			case RSSL_HIGH_WATER_MARK:
			{
				pReactorWarmStandByHandlerImpl->ioCtlCodes |= RSSL_REACTOR_WS_HIGH_WATER_MARK;
				pReactorWarmStandByHandlerImpl->highWaterMark = *(RsslUInt32*)(value);
				break;
			}
			case RSSL_SYSTEM_READ_BUFFERS:
			{
				pReactorWarmStandByHandlerImpl->ioCtlCodes |= RSSL_REACTOR_WS_SYSTEM_READ_BUFFERS;
				pReactorWarmStandByHandlerImpl->systemReadBuffers = *(RsslUInt32*)(value);
				break;
			}
			case RSSL_SYSTEM_WRITE_BUFFERS:
			{
				pReactorWarmStandByHandlerImpl->ioCtlCodes |= RSSL_REACTOR_WS_SYSTEM_WRITE_BUFFERS;
				pReactorWarmStandByHandlerImpl->systemWriteBuffers = *(RsslUInt32*)(value);
				break;
			}
			case RSSL_PRIORITY_FLUSH_ORDER:
			{
				pReactorWarmStandByHandlerImpl->ioCtlCodes |= RSSL_REACTOR_WS_PRIORITY_FLUSH_ORDER;
				pReactorWarmStandByHandlerImpl->priorityFlushOrder = *(RsslUInt32*)(value);
				break;
			}
			case RSSL_COMPRESSION_THRESHOLD:
			{
				pReactorWarmStandByHandlerImpl->ioCtlCodes |= RSSL_REACTOR_WS_COMPRESSION_THRESHOLD;
				pReactorWarmStandByHandlerImpl->compressionThresHold = *(RsslUInt32*)(value);
				break;
			}
		}

		/* Apply ioctl codes to all channels */
		RSSL_MUTEX_LOCK(&pReactorWarmStandByHandlerImpl->warmStandByHandlerMutex);
		RSSL_QUEUE_FOR_EACH_LINK(&pReactorWarmStandByHandlerImpl->rsslChannelQueue, pLink)
		{
			pApplyReactorChannelImpl = RSSL_QUEUE_LINK_TO_OBJECT(RsslReactorChannelImpl, warmstandbyChannelLink, pLink);
			ret = rsslIoctl(pReactorChannel->pRsslChannel, (RsslIoctlCodes)code, value, &pError->rsslError);
			if (ret != RSSL_RET_SUCCESS)
			{
				rsslSetErrorInfoLocation(pError, __FILE__, __LINE__);
				break;
			}
		}
		RSSL_MUTEX_UNLOCK(&pReactorWarmStandByHandlerImpl->warmStandByHandlerMutex);
	}
	else
	{
		ret = rsslIoctl(pReactorChannel->pRsslChannel, (RsslIoctlCodes)code, value, &pError->rsslError);
		if (ret != RSSL_RET_SUCCESS)
			rsslSetErrorInfoLocation(pError, __FILE__, __LINE__);
	}

	return (reactorUnlockInterface(pReactorImpl), ret);
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

static RsslReactorOAuthCredentialRenewal* _reactorCopyRsslReactorOAuthCredentialRenewal(RsslReactorImpl *pReactorImpl, RsslReactorTokenSessionImpl *pTokenSessionImpl, 
	RsslReactorOAuthCredentialRenewalOptions *pOptions, RsslReactorOAuthCredentialRenewal* pOAuthCredentialRenewal, RsslErrorInfo *pError)
{
	RsslReactorOAuthCredentialRenewalImpl *pOAuthCredentialRenewalOutImpl;
	RsslReactorOAuthCredentialRenewal *pOAuthCredentialRenewalOut;
	char *pData;
	char *pCurPos;
	size_t msgSize = sizeof(RsslReactorOAuthCredentialRenewalImpl);
	size_t dataLength = pOAuthCredentialRenewal->clientSecret.length + pOAuthCredentialRenewal->password.length + pOAuthCredentialRenewal->newPassword.length;
	RsslBool allocateMemory = RSSL_FALSE;
	RsslBuffer rsslAccessTokenRespBuffer = RSSL_INIT_BUFFER;
	
	if (!pTokenSessionImpl)
	{
		dataLength += pOAuthCredentialRenewal->clientId.length + pOAuthCredentialRenewal->userName.length + pOAuthCredentialRenewal->tokenScope.length + pOAuthCredentialRenewal->clientSecret.length;

		/* Copy proxy configurations if specified by users */
		dataLength += pOptions->proxyHostName.length + pOptions->proxyPort.length + pOptions->proxyUserName.length + pOptions->proxyPasswd.length + pOptions->proxyDomain.length;

		/* Creates temporary buffer for handling token response */
		rsslAccessTokenRespBuffer.length = RSSL_REST_INIT_TOKEN_BUFFER_SIZE;
		rsslAccessTokenRespBuffer.data = (char*)malloc(rsslAccessTokenRespBuffer.length);

		if (rsslAccessTokenRespBuffer.data == NULL)
		{
			rsslSetErrorInfo(pError, RSSL_EIC_FAILURE, RSSL_RET_FAILURE, __FILE__, __LINE__, "Failed to allocate memory for handling access token response.");
			return NULL;
		}
	}

	/* Reuse the existing buffer for credential renewal if any from the channel */
	if(pTokenSessionImpl && pTokenSessionImpl->pOAuthCredentialRenewalImpl)
	{
		if (pTokenSessionImpl->pOAuthCredentialRenewalImpl->memoryLength < (RsslUInt32)(msgSize + dataLength))
		{
			free(pTokenSessionImpl->pOAuthCredentialRenewalImpl);
			pTokenSessionImpl->pOAuthCredentialRenewalImpl->memoryLength = 0;

			allocateMemory = RSSL_TRUE;
		}
		else
		{
			pData = (char*)pTokenSessionImpl->pOAuthCredentialRenewalImpl;
			memset(pData + msgSize, 0, dataLength);
			pOAuthCredentialRenewalOutImpl = pTokenSessionImpl->pOAuthCredentialRenewalImpl;
			pOAuthCredentialRenewalOut = &pOAuthCredentialRenewalOutImpl->reactorOAuthCredentialRenewal;
		}
	}
	else
	{
		allocateMemory = RSSL_TRUE;
	}

	if (allocateMemory)
	{
		if ((pData = (char*)malloc(msgSize + dataLength)) == NULL)
		{
			rsslSetErrorInfo(pError, RSSL_EIC_FAILURE, RSSL_RET_FAILURE, __FILE__, __LINE__, "Failed to allocate memory for copying RsslReactorOAuthCredentialRenewal.");
			return NULL;
		}

		memset(pData, 0, msgSize + dataLength);
		pOAuthCredentialRenewalOutImpl = (RsslReactorOAuthCredentialRenewalImpl*)pData;
		pOAuthCredentialRenewalOutImpl->memoryLength = (RsslUInt32)(msgSize + dataLength); /* Keep the actual memory size */
		
		if (pTokenSessionImpl)
		{
			pOAuthCredentialRenewalOutImpl->pParentTokenSession = pTokenSessionImpl;
			pTokenSessionImpl->pOAuthCredentialRenewalImpl = pOAuthCredentialRenewalOutImpl;
		}
		else
		{
			pOAuthCredentialRenewalOutImpl->rsslAccessTokenRespBuffer = rsslAccessTokenRespBuffer;
			pOAuthCredentialRenewalOutImpl->pAuthTokenEventCallback = pOptions->pAuthTokenEventCallback;
			pOAuthCredentialRenewalOutImpl->pRsslReactor = (RsslReactor*)pReactorImpl;
		}

		pOAuthCredentialRenewalOut = &pOAuthCredentialRenewalOutImpl->reactorOAuthCredentialRenewal;
	}

	pCurPos = pData + msgSize;

	if (!pTokenSessionImpl && (pOAuthCredentialRenewal->userName.length && pOAuthCredentialRenewal->userName.data) )
	{
		pOAuthCredentialRenewalOut->userName.length = pOAuthCredentialRenewal->userName.length;
		pOAuthCredentialRenewalOut->userName.data = pCurPos;
		memcpy(pOAuthCredentialRenewalOut->userName.data, pOAuthCredentialRenewal->userName.data, pOAuthCredentialRenewalOut->userName.length);
		pCurPos += pOAuthCredentialRenewalOut->userName.length;
	}

	if (pOAuthCredentialRenewal->password.length && pOAuthCredentialRenewal->password.data)
	{
		pOAuthCredentialRenewalOut->password.length = pOAuthCredentialRenewal->password.length;
		pOAuthCredentialRenewalOut->password.data = pCurPos;
		memcpy(pOAuthCredentialRenewalOut->password.data, pOAuthCredentialRenewal->password.data, pOAuthCredentialRenewalOut->password.length);
		pCurPos += pOAuthCredentialRenewalOut->password.length;
	}

	if (pOAuthCredentialRenewal->newPassword.length && pOAuthCredentialRenewal->newPassword.data)
	{
		pOAuthCredentialRenewalOut->newPassword.length = pOAuthCredentialRenewal->newPassword.length;
		pOAuthCredentialRenewalOut->newPassword.data = pCurPos;
		memcpy(pOAuthCredentialRenewalOut->newPassword.data, pOAuthCredentialRenewal->newPassword.data, pOAuthCredentialRenewalOut->newPassword.length);
		pCurPos += pOAuthCredentialRenewalOut->newPassword.length;
	}

	if (!pTokenSessionImpl && (pOAuthCredentialRenewal->clientId.length && pOAuthCredentialRenewal->clientId.data) )
	{
		pOAuthCredentialRenewalOut->clientId.length = pOAuthCredentialRenewal->clientId.length;
		pOAuthCredentialRenewalOut->clientId.data = pCurPos;
		memcpy(pOAuthCredentialRenewalOut->clientId.data, pOAuthCredentialRenewal->clientId.data, pOAuthCredentialRenewalOut->clientId.length);
		pCurPos += pOAuthCredentialRenewalOut->clientId.length;
	}

	if (pOAuthCredentialRenewal->clientSecret.length && pOAuthCredentialRenewal->clientSecret.data)
	{
		pOAuthCredentialRenewalOut->clientSecret.length = pOAuthCredentialRenewal->clientSecret.length;
		pOAuthCredentialRenewalOut->clientSecret.data = pCurPos;
		memcpy(pOAuthCredentialRenewalOut->clientSecret.data, pOAuthCredentialRenewal->clientSecret.data, pOAuthCredentialRenewalOut->clientSecret.length);
		pCurPos += pOAuthCredentialRenewalOut->clientSecret.length;
	}

	if (!pTokenSessionImpl && (pOAuthCredentialRenewal->tokenScope.length && pOAuthCredentialRenewal->tokenScope.data) )
	{
		pOAuthCredentialRenewalOut->tokenScope.length = pOAuthCredentialRenewal->tokenScope.length;
		pOAuthCredentialRenewalOut->tokenScope.data = pCurPos;
		memcpy(pOAuthCredentialRenewalOut->tokenScope.data, pOAuthCredentialRenewal->tokenScope.data, pOAuthCredentialRenewalOut->tokenScope.length);
		pCurPos += pOAuthCredentialRenewalOut->tokenScope.length;
	}

	if (!pTokenSessionImpl && (pOptions->proxyDomain.length && pOptions->proxyDomain.data))
	{
		pOAuthCredentialRenewalOutImpl->proxyDomain.length = pOptions->proxyDomain.length;
		pOAuthCredentialRenewalOutImpl->proxyDomain.data = pCurPos;
		memcpy(pOAuthCredentialRenewalOutImpl->proxyDomain.data, pOptions->proxyDomain.data, pOAuthCredentialRenewalOutImpl->proxyDomain.length);
		pCurPos += pOAuthCredentialRenewalOutImpl->proxyDomain.length;
	}

	if (!pTokenSessionImpl && (pOptions->proxyHostName.length && pOptions->proxyHostName.data))
	{
		pOAuthCredentialRenewalOutImpl->proxyHostName.length = pOptions->proxyHostName.length;
		pOAuthCredentialRenewalOutImpl->proxyHostName.data = pCurPos;
		memcpy(pOAuthCredentialRenewalOutImpl->proxyHostName.data, pOptions->proxyHostName.data, pOAuthCredentialRenewalOutImpl->proxyHostName.length);
		pCurPos += pOAuthCredentialRenewalOutImpl->proxyHostName.length;
	}

	if (!pTokenSessionImpl && (pOptions->proxyPort.length && pOptions->proxyPort.data))
	{
		pOAuthCredentialRenewalOutImpl->proxyPort.length = pOptions->proxyPort.length;
		pOAuthCredentialRenewalOutImpl->proxyPort.data = pCurPos;
		memcpy(pOAuthCredentialRenewalOutImpl->proxyPort.data, pOptions->proxyPort.data, pOAuthCredentialRenewalOutImpl->proxyPort.length);
		pCurPos += pOAuthCredentialRenewalOutImpl->proxyPort.length;
	}

	if (!pTokenSessionImpl && (pOptions->proxyPasswd.length && pOptions->proxyPasswd.data))
	{
		pOAuthCredentialRenewalOutImpl->proxyPasswd.length = pOptions->proxyPasswd.length;
		pOAuthCredentialRenewalOutImpl->proxyPasswd.data = pCurPos;
		memcpy(pOAuthCredentialRenewalOutImpl->proxyPasswd.data, pOptions->proxyPasswd.data, pOAuthCredentialRenewalOutImpl->proxyPasswd.length);
		pCurPos += pOAuthCredentialRenewalOutImpl->proxyPasswd.length;
	}

	if (!pTokenSessionImpl && (pOptions->proxyUserName.length && pOptions->proxyUserName.data))
	{
		pOAuthCredentialRenewalOutImpl->proxyUserName.length = pOptions->proxyUserName.length;
		pOAuthCredentialRenewalOutImpl->proxyUserName.data = pCurPos;
		memcpy(pOAuthCredentialRenewalOutImpl->proxyUserName.data, pOptions->proxyUserName.data, pOAuthCredentialRenewalOutImpl->proxyUserName.length);
		pCurPos += pOAuthCredentialRenewalOutImpl->proxyUserName.length;
	}

	pOAuthCredentialRenewalOut->takeExclusiveSignOnControl = pOAuthCredentialRenewal->takeExclusiveSignOnControl;

	return pOAuthCredentialRenewalOut;
}

RsslReactorOAuthCredential* rsslCreateOAuthCredentialCopyV1(RsslReactorOAuthCredential* pOAuthCredential, 
	RsslRDMLoginRequest* pRDMLoginRequest, RsslReactorOMMConsumerRole *pConsRole, RsslRet *ret)
{
	RsslReactorOAuthCredential *pOAuthCredentialOut;
	RsslReactorOAuthCredentialEventCallback *pOAuthCredentialEventCallback = pOAuthCredential ? pOAuthCredential->pOAuthCredentialEventCallback : 0;
	RsslReactorOAuthCredential defaultOAuthCredential;
	RsslBuffer dataBuffer;
	char *pData;
	char *pCurPos;
	size_t msgSize = sizeof(RsslReactorOAuthCredential);
	size_t userNameLength = (pOAuthCredential && pOAuthCredential->userName.length) ? pOAuthCredential->userName.length : (pRDMLoginRequest ? pRDMLoginRequest->userName.length : 0);
	size_t passwordLength = (pOAuthCredential && pOAuthCredential->password.length) ? pOAuthCredential->password.length : (pRDMLoginRequest ? pRDMLoginRequest->password.length : 0);
	size_t dataLength = pOAuthCredential ? pOAuthCredential->clientSecret.length : 0;
	RsslBuffer tokenScope;
	rsslClearReactorOAuthCredential(&defaultOAuthCredential);
	tokenScope = pOAuthCredential ? pOAuthCredential->tokenScope : defaultOAuthCredential.tokenScope;
	dataLength += tokenScope.length;
	dataLength += userNameLength;
	dataLength += passwordLength;
	dataLength += (pOAuthCredential && pOAuthCredential->clientId.length) ? pOAuthCredential->clientId.length : (pConsRole && pConsRole->clientId.length ? pConsRole->clientId.length : 0);

	/* Returns the NULL as there is no OAth credential available */
	if (userNameLength == 0 || passwordLength == 0)
	{
		*ret = RSSL_RET_INVALID_ARGUMENT;
		return NULL;
	}

	if ((pData = (char*)malloc(msgSize + dataLength)) == NULL)
	{
		*ret = RSSL_RET_FAILURE;
		return NULL;
	}

	memset(pData, 0, msgSize + dataLength);

	pOAuthCredentialOut = (RsslReactorOAuthCredential*)pData;

	/* Copies a reference to the callback if specified by users */
	pOAuthCredentialOut->pOAuthCredentialEventCallback = pOAuthCredentialEventCallback;
	pOAuthCredentialOut->takeExclusiveSignOnControl = pOAuthCredential ? pOAuthCredential->takeExclusiveSignOnControl : RSSL_TRUE;
	
	pCurPos = pData + msgSize;

	rsslClearBuffer(&dataBuffer);
	if (pOAuthCredential && pOAuthCredential->userName.length && pOAuthCredential->userName.data)
	{
		dataBuffer = pOAuthCredential->userName;
	}
	else if (pRDMLoginRequest && pRDMLoginRequest->userName.length && pRDMLoginRequest->userName.data)
	{
		dataBuffer = pRDMLoginRequest->userName;
	}

	if (dataBuffer.length)
	{
		pOAuthCredentialOut->userName.length = dataBuffer.length;
		pOAuthCredentialOut->userName.data = pCurPos;
		memcpy(pOAuthCredentialOut->userName.data, dataBuffer.data, pOAuthCredentialOut->userName.length);
		pCurPos += pOAuthCredentialOut->userName.length;
	}

	rsslClearBuffer(&dataBuffer);
	if (pOAuthCredential && pOAuthCredential->clientId.length && pOAuthCredential->clientId.data)
	{
		dataBuffer = pOAuthCredential->clientId;
	}
	else if (pConsRole && pConsRole->clientId.length && pConsRole->clientId.data)
	{
		dataBuffer = pConsRole->clientId;
	}

	if (dataBuffer.length)
	{
		pOAuthCredentialOut->clientId.length = dataBuffer.length;
		pOAuthCredentialOut->clientId.data = pCurPos;
		memcpy(pOAuthCredentialOut->clientId.data, dataBuffer.data, pOAuthCredentialOut->clientId.length);
		pCurPos += pOAuthCredentialOut->clientId.length;
	}
	
	rsslClearBuffer(&dataBuffer);
	if (pOAuthCredential && pOAuthCredential->password.length && pOAuthCredential->password.data)
	{
		dataBuffer = pOAuthCredential->password;
	}
	else if (pRDMLoginRequest && pRDMLoginRequest->password.length && pRDMLoginRequest->password.data)
	{
		dataBuffer = pRDMLoginRequest->password;
	}

	if (dataBuffer.length)
	{
		pOAuthCredentialOut->password.length = dataBuffer.length;
		pOAuthCredentialOut->password.data = pCurPos;
		memcpy(pOAuthCredentialOut->password.data, dataBuffer.data, pOAuthCredentialOut->password.length);
		pCurPos += pOAuthCredentialOut->password.length;
	}

	if (pOAuthCredential && pOAuthCredential->clientSecret.length && pOAuthCredential->clientSecret.data)
	{
		pOAuthCredentialOut->clientSecret.length = pOAuthCredential->clientSecret.length;
		pOAuthCredentialOut->clientSecret.data = pCurPos;
		memcpy(pOAuthCredentialOut->clientSecret.data, pOAuthCredential->clientSecret.data, pOAuthCredentialOut->clientSecret.length);
		pCurPos += pOAuthCredentialOut->clientSecret.length;
	}

	if (tokenScope.length && tokenScope.data)
	{
		pOAuthCredentialOut->tokenScope.length = tokenScope.length;
		pOAuthCredentialOut->tokenScope.data = pCurPos;
		memcpy(pOAuthCredentialOut->tokenScope.data, tokenScope.data, pOAuthCredentialOut->tokenScope.length);
		pCurPos += pOAuthCredentialOut->tokenScope.length;
	}

	*ret = RSSL_RET_SUCCESS;
	return pOAuthCredentialOut;
}

RsslReactorOAuthCredential* rsslCreateOAuthCredentialCopyV2(RsslReactorOAuthCredential* pOAuthCredential,
	RsslRDMLoginRequest* pRDMLoginRequest, RsslReactorOMMConsumerRole* pConsRole, RsslRet* ret)
{
	RsslReactorOAuthCredential* pOAuthCredentialOut;
	RsslReactorOAuthCredentialEventCallback* pOAuthCredentialEventCallback = pOAuthCredential ? pOAuthCredential->pOAuthCredentialEventCallback : 0;
	RsslReactorOAuthCredential defaultOAuthCredential;
	RsslBuffer dataBuffer;
	char* pData;
	char* pCurPos;
	size_t msgSize = sizeof(RsslReactorOAuthCredential);
	size_t userNameLength = (pOAuthCredential && pOAuthCredential->userName.length) ? pOAuthCredential->userName.length : (pRDMLoginRequest ? pRDMLoginRequest->userName.length : 0);
	size_t dataLength = pOAuthCredential ? pOAuthCredential->clientSecret.length : 0;
	RsslBuffer tokenScope;
	rsslClearReactorOAuthCredential(&defaultOAuthCredential);
	tokenScope = pOAuthCredential ? pOAuthCredential->tokenScope : defaultOAuthCredential.tokenScope;
	dataLength += tokenScope.length;
	dataLength += userNameLength;
	dataLength += (pOAuthCredential && pOAuthCredential->clientId.length) ? pOAuthCredential->clientId.length : (pConsRole && pConsRole->clientId.length ? pConsRole->clientId.length : 0);

	/* Returns the NULL as there is no OAth credential available */
	if (userNameLength == 0 || (pOAuthCredential->clientSecret.length == 0 ))
	{
		*ret = RSSL_RET_INVALID_ARGUMENT;
		return NULL;
	}

	if ((pData = (char*)malloc(msgSize + dataLength)) == NULL)
	{
		*ret = RSSL_RET_FAILURE;
		return NULL;
	}

	memset(pData, 0, msgSize + dataLength);

	pOAuthCredentialOut = (RsslReactorOAuthCredential*)pData;

	/* Copies a reference to the callback if specified by users */
	pOAuthCredentialOut->pOAuthCredentialEventCallback = pOAuthCredentialEventCallback;

	pCurPos = pData + msgSize;

	rsslClearBuffer(&dataBuffer);
	if (pOAuthCredential && pOAuthCredential->userName.length && pOAuthCredential->userName.data)
	{
		dataBuffer = pOAuthCredential->userName;
	}
	else if (pRDMLoginRequest && pRDMLoginRequest->userName.length && pRDMLoginRequest->userName.data)
	{
		dataBuffer = pRDMLoginRequest->userName;
	}

	if (dataBuffer.length)
	{
		pOAuthCredentialOut->userName.length = dataBuffer.length;
		pOAuthCredentialOut->userName.data = pCurPos;
		memcpy(pOAuthCredentialOut->userName.data, dataBuffer.data, pOAuthCredentialOut->userName.length);
		pCurPos += pOAuthCredentialOut->userName.length;
	}

	rsslClearBuffer(&dataBuffer);
	if (pOAuthCredential && pOAuthCredential->clientId.length && pOAuthCredential->clientId.data)
	{
		dataBuffer = pOAuthCredential->clientId;
	}
	else if (pConsRole && pConsRole->clientId.length && pConsRole->clientId.data)
	{
		dataBuffer = pConsRole->clientId;
	}

	if (dataBuffer.length)
	{
		pOAuthCredentialOut->clientId.length = dataBuffer.length;
		pOAuthCredentialOut->clientId.data = pCurPos;
		memcpy(pOAuthCredentialOut->clientId.data, dataBuffer.data, pOAuthCredentialOut->clientId.length);
		pCurPos += pOAuthCredentialOut->clientId.length;
	}

	if (pOAuthCredential && pOAuthCredential->clientSecret.length && pOAuthCredential->clientSecret.data)
	{
		pOAuthCredentialOut->clientSecret.length = pOAuthCredential->clientSecret.length;
		pOAuthCredentialOut->clientSecret.data = pCurPos;
		memcpy(pOAuthCredentialOut->clientSecret.data, pOAuthCredential->clientSecret.data, pOAuthCredentialOut->clientSecret.length);
		pCurPos += pOAuthCredentialOut->clientSecret.length;
	}

	if (tokenScope.length && tokenScope.data)
	{
		pOAuthCredentialOut->tokenScope.length = tokenScope.length;
		pOAuthCredentialOut->tokenScope.data = pCurPos;
		memcpy(pOAuthCredentialOut->tokenScope.data, tokenScope.data, pOAuthCredentialOut->tokenScope.length);
		pCurPos += pOAuthCredentialOut->tokenScope.length;
	}


	*ret = RSSL_RET_SUCCESS;
	return pOAuthCredentialOut;
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
			RsslBool supportSessionMgnt = pReactorChannel->pWarmStandByHandlerImpl ? pReactorChannel->pWarmStandByHandlerImpl->enableSessionMgnt : pReactorChannel->supportSessionMgnt;
			RsslReactorConnectInfoImpl* pReactorConnectInfoImpl = _reactorGetCurrentReactorConnectInfoImpl(pReactorChannel);

			if (supportSessionMgnt)
			{
				RsslRet ret;
				RsslBuffer sessionKey = RSSL_INIT_BUFFER;
				RsslReactorTokenSessionImpl *pTokenSessionImpl = NULL;
				RsslHashLink *pHashLink = NULL;
				RsslReactorTokenManagementImpl *pTokenManagementImpl = &pReactorChannel->pParentReactor->reactorWorker.reactorTokenManagement;
				RsslBuffer userName = (pConsRole->pOAuthCredential && pConsRole->pOAuthCredential->userName.data) ? pConsRole->pOAuthCredential->userName : pConsRole->pLoginRequest ? pConsRole->pLoginRequest->userName : sessionKey;
				RsslBuffer password = (pConsRole->pOAuthCredential && pConsRole->pOAuthCredential->password.data) ? pConsRole->pOAuthCredential->password : pConsRole->pLoginRequest ? pConsRole->pLoginRequest->password : sessionKey;
				RsslBuffer clientId = (pConsRole->pOAuthCredential && pConsRole->pOAuthCredential->clientId.data) ? pConsRole->pOAuthCredential->clientId : pConsRole->clientId;
				RsslReactorOAuthCredential *pOAuthCredential = pConsRole->pOAuthCredential;

				pConsRole->pOAuthCredential = NULL;

				if (userName.data != 0 && userName.length != 0 && clientId.data != 0 && clientId.length && password.data != 0 && password.length != 0)
				{
					pReactorChannel->sessionVersion = RSSL_RC_SESSMGMT_V1;
				}
				else if (clientId.data != 0 && clientId.length != 0 && pOAuthCredential && ((pOAuthCredential->clientSecret.data != 0 && pOAuthCredential->clientSecret.length != 0) ))
				{
					pReactorChannel->sessionVersion = RSSL_RC_SESSMGMT_V2;
				}
				else
				{
					/* Set the pointers to NULL to avoid freeing the user's memory */
					pConsRole->pLoginRequest = NULL;
					pConsRole->pDirectoryRequest = NULL;

					rsslSetErrorInfo(pError, RSSL_EIC_FAILURE, RSSL_RET_INVALID_ARGUMENT, __FILE__, __LINE__, "Failed to copy OAuth credentials for enabling the session management.");
					return RSSL_RET_INVALID_ARGUMENT;
				}

				if (pReactorChannel->sessionVersion == RSSL_RC_SESSMGMT_V1)
				{
					RSSL_MUTEX_LOCK(&pTokenManagementImpl->tokenSessionMutex);

					pHashLink = rsslHashTableFind(&pTokenManagementImpl->sessionByNameAndClientIdHt, &userName, NULL);

					if (pHashLink != 0)
					{
						RsslInt remainingTimeForReissueMs = 0;
						RsslInt currentTime = getCurrentTimeMs(pReactorChannel->pParentReactor->ticksPerMsec);

						pTokenSessionImpl = RSSL_HASH_LINK_TO_OBJECT(RsslReactorTokenSessionImpl, hashLinkNameAndClientId, pHashLink);

						if (compareOAuthCredentialForTokenSession(pTokenSessionImpl->pOAuthCredential, &clientId, &password, pOAuthCredential, pError) == RSSL_FALSE)
						{
							/* Set the pointers to NULL to avoid freeing the user's memory */
							pConsRole->pLoginRequest = NULL;
							pConsRole->pDirectoryRequest = NULL;
							RSSL_MUTEX_UNLOCK(&pTokenManagementImpl->tokenSessionMutex);
							return RSSL_RET_INVALID_ARGUMENT;
						}

						/* Check whether the token session has enough time to reissue the token for this channel */
						if (pTokenSessionImpl->nextExpiresTime != 0)
						{
							remainingTimeForReissueMs = pTokenSessionImpl->nextExpiresTime - currentTime;

							if (remainingTimeForReissueMs < (RsslInt)(pReactorChannel->pParentReactor->tokenReissueRatio * 1000))
							{
								rsslSetErrorInfo(pError, RSSL_EIC_FAILURE, RSSL_RET_FAILURE, __FILE__, __LINE__,
									"Couldn't add the channel to the token session as the token reissue interval(%lld ms) is too small.", remainingTimeForReissueMs);

								/* Set the pointers to NULL to avoid freeing the user's memory */
								pConsRole->pLoginRequest = NULL;
								pConsRole->pDirectoryRequest = NULL;

								RSSL_MUTEX_UNLOCK(&pTokenManagementImpl->tokenSessionMutex);
								return RSSL_RET_FAILURE;
							}
						}

						pReactorChannel->pTokenSessionImpl = pTokenSessionImpl;
						pReactorConnectInfoImpl->lastTokenUpdatedTime = pTokenSessionImpl->lastTokenUpdatedTime;
					}
					else
					{
						pTokenSessionImpl = (RsslReactorTokenSessionImpl*)malloc(sizeof(RsslReactorTokenSessionImpl));

						if (pTokenSessionImpl == 0)
						{
							rsslSetErrorInfo(pError, RSSL_EIC_FAILURE, RSSL_RET_FAILURE, __FILE__, __LINE__,
								"Failed to allocate memory for creating RsslReactorTokenSessionImpl.");

							/* Set the pointers to NULL to avoid freeing the user's memory */
							pConsRole->pLoginRequest = NULL;
							pConsRole->pDirectoryRequest = NULL;

							RSSL_MUTEX_UNLOCK(&pTokenManagementImpl->tokenSessionMutex);
							return RSSL_RET_FAILURE;
						}

						rsslClearReactorTokenSessionImpl(pTokenSessionImpl);
						pTokenSessionImpl->pReactor = (RsslReactor*)pReactorChannel->pParentReactor;
						pTokenSessionImpl->reissueTokenAttemptLimit = pReactorChannel->pParentReactor->reissueTokenAttemptLimit;
						pTokenSessionImpl->sessionVersion = RSSL_RC_SESSMGMT_V1;

						/* Copy the rsslReactor's token service URL if present.  If not, use the default location for V1 */
						if (pReactorChannel->pParentReactor->tokenServiceURLV1.data != 0 && pReactorChannel->pParentReactor->tokenServiceURLV1.length != 0)
						{
							pTokenSessionImpl->sessionAuthUrl.length = pReactorChannel->pParentReactor->tokenServiceURLBufferV1.length;
							if (!(pTokenSessionImpl->sessionAuthUrl.data = (char*)malloc(pReactorChannel->pParentReactor->tokenServiceURLBufferV1.length + 1)))
							{
								rsslSetErrorInfo(pError, RSSL_EIC_FAILURE, RSSL_RET_FAILURE, __FILE__, __LINE__, "Failed to allocate memory for the token service URL.");
								/* Set the pointers to NULL to avoid freeing the user's memory */
								pConsRole->pLoginRequest = NULL;
								pConsRole->pDirectoryRequest = NULL;

								RSSL_MUTEX_UNLOCK(&pTokenManagementImpl->tokenSessionMutex);
								return RSSL_RET_FAILURE;
							}

							memset(pTokenSessionImpl->sessionAuthUrl.data, 0, pReactorChannel->pParentReactor->tokenServiceURLBufferV1.length + 1);
							strncpy(pTokenSessionImpl->sessionAuthUrl.data, pReactorChannel->pParentReactor->tokenServiceURLBufferV1.data, pTokenSessionImpl->sessionAuthUrl.length);
						}
						else
						{
							pTokenSessionImpl->sessionAuthUrl.length = rssl_rest_token_url_v1.length;
							if (!(pTokenSessionImpl->sessionAuthUrl.data = (char*)malloc(rssl_rest_token_url_v1.length + 1)))
							{
								rsslSetErrorInfo(pError, RSSL_EIC_FAILURE, RSSL_RET_FAILURE, __FILE__, __LINE__, "Failed to allocate memory for the token service URL.");
								/* Set the pointers to NULL to avoid freeing the user's memory */
								pConsRole->pLoginRequest = NULL;
								pConsRole->pDirectoryRequest = NULL;

								RSSL_MUTEX_UNLOCK(&pTokenManagementImpl->tokenSessionMutex);
								return RSSL_RET_FAILURE;
							}

							memset(pTokenSessionImpl->sessionAuthUrl.data, 0, rssl_rest_token_url_v1.length + 1);
							strncpy(pTokenSessionImpl->sessionAuthUrl.data, rssl_rest_token_url_v1.data, pTokenSessionImpl->sessionAuthUrl.length);
						}

						/* Assigned the token session to the channel */
						pReactorChannel->pTokenSessionImpl = pTokenSessionImpl;

						sessionKey.length = userName.length;
						sessionKey.data = (char*)malloc(sessionKey.length);
						if (sessionKey.data == 0)
						{
							free(pTokenSessionImpl->sessionAuthUrl.data);
							free(pTokenSessionImpl);
							pReactorChannel->pTokenSessionImpl = NULL;

							rsslSetErrorInfo(pError, RSSL_EIC_FAILURE, RSSL_RET_FAILURE, __FILE__, __LINE__,
								"Failed to allocate memory for creating session identifier.");

							/* Set the pointers to NULL to avoid freeing the user's memory */
							pConsRole->pLoginRequest = NULL;
							pConsRole->pDirectoryRequest = NULL;

							RSSL_MUTEX_UNLOCK(&pTokenManagementImpl->tokenSessionMutex);
							return RSSL_RET_FAILURE;
						}

						memcpy(sessionKey.data, userName.data, sessionKey.length);

						/* Allocates memory to store temporary URL redirect */
						pTokenSessionImpl->temporaryURLBuffer.length = RSSL_REACTOR_DEFAULT_URL_LENGHT;
						pTokenSessionImpl->temporaryURLBuffer.data = (char*)malloc(pTokenSessionImpl->temporaryURLBuffer.length);
						if (pTokenSessionImpl->temporaryURLBuffer.data == 0)
						{
							rsslSetErrorInfo(pError, RSSL_EIC_FAILURE, RSSL_RET_FAILURE, __FILE__, __LINE__,
								"Failed to allocate memory for storing temporary URL redirect.");

							free(pTokenSessionImpl->sessionAuthUrl.data);
							free(sessionKey.data);
							free(pTokenSessionImpl);
							pReactorChannel->pTokenSessionImpl = NULL;

							/* Set the pointers to NULL to avoid freeing the user's memory */
							pConsRole->pLoginRequest = NULL;
							pConsRole->pDirectoryRequest = NULL;

							RSSL_MUTEX_UNLOCK(&pTokenManagementImpl->tokenSessionMutex);
							return RSSL_RET_FAILURE;
						}

						/* Copy the key for the HashTable */
						pTokenSessionImpl->userNameAndClientId = sessionKey;

						if (pTokenSessionImpl->rsslAccessTokenRespBuffer.data == 0)
						{
							pTokenSessionImpl->rsslAccessTokenRespBuffer.length = RSSL_REST_INIT_TOKEN_BUFFER_SIZE;
							pTokenSessionImpl->rsslAccessTokenRespBuffer.data =
								(char*)malloc(pTokenSessionImpl->rsslAccessTokenRespBuffer.length);

							if (pTokenSessionImpl->rsslAccessTokenRespBuffer.data == 0)
							{
								free(pTokenSessionImpl->sessionAuthUrl.data);
								free(pTokenSessionImpl->temporaryURLBuffer.data);
								free(pTokenSessionImpl->userNameAndClientId.data);
								free(pTokenSessionImpl);
								pReactorChannel->pTokenSessionImpl = NULL;

								rsslSetErrorInfo(pError, RSSL_EIC_FAILURE, RSSL_RET_FAILURE, __FILE__, __LINE__,
									"Failed to allocate memory for handling access token response.");

								/* Set the pointers to NULL to avoid freeing the user's memory */
								pConsRole->pLoginRequest = NULL;
								pConsRole->pDirectoryRequest = NULL;

								RSSL_MUTEX_UNLOCK(&pTokenManagementImpl->tokenSessionMutex);
								return RSSL_RET_FAILURE;
							}
						}

						if (pTokenSessionImpl->tokenInformationBuffer.data == 0)
						{
							pTokenSessionImpl->tokenInformationBuffer.length = RSSL_REST_INIT_TOKEN_BUFFER_SIZE;
							pTokenSessionImpl->tokenInformationBuffer.data =
								(char*)malloc(pTokenSessionImpl->tokenInformationBuffer.length);

							if (pTokenSessionImpl->tokenInformationBuffer.data == 0)
							{
								free(pTokenSessionImpl->sessionAuthUrl.data);
								free(pTokenSessionImpl->temporaryURLBuffer.data);
								free(pTokenSessionImpl->userNameAndClientId.data);
								free(pTokenSessionImpl->rsslAccessTokenRespBuffer.data);
								free(pTokenSessionImpl);
								pReactorChannel->pTokenSessionImpl = NULL;

								rsslSetErrorInfo(pError, RSSL_EIC_FAILURE, RSSL_RET_FAILURE, __FILE__, __LINE__,
									"Failed to allocate memory for storing token information.");

								/* Set the pointers to NULL to avoid freeing the user's memory */
								pConsRole->pLoginRequest = NULL;
								pConsRole->pDirectoryRequest = NULL;

								RSSL_MUTEX_UNLOCK(&pTokenManagementImpl->tokenSessionMutex);
								return RSSL_RET_FAILURE;
							}
						}

						if (pTokenSessionImpl->rsslPostDataBodyBuf.data == 0)
						{
							pTokenSessionImpl->rsslPostDataBodyBuf.length = (RSSL_REST_INIT_TOKEN_BUFFER_SIZE / 8);
							pTokenSessionImpl->rsslPostDataBodyBuf.data =
								(char*)malloc(pTokenSessionImpl->rsslPostDataBodyBuf.length);

							if (pTokenSessionImpl->rsslPostDataBodyBuf.data == 0)
							{
								free(pTokenSessionImpl->sessionAuthUrl.data);
								free(pTokenSessionImpl->temporaryURLBuffer.data);
								free(pTokenSessionImpl->userNameAndClientId.data);
								free(pTokenSessionImpl->rsslAccessTokenRespBuffer.data);
								free(pTokenSessionImpl->tokenInformationBuffer.data);
								free(pTokenSessionImpl);
								pReactorChannel->pTokenSessionImpl = NULL;

								rsslSetErrorInfo(pError, RSSL_EIC_FAILURE, RSSL_RET_FAILURE, __FILE__, __LINE__,
									"Failed to allocate memory for posting token request.");

								/* Set the pointers to NULL to avoid freeing the user's memory */
								pConsRole->pLoginRequest = NULL;
								pConsRole->pDirectoryRequest = NULL;

								RSSL_MUTEX_UNLOCK(&pTokenManagementImpl->tokenSessionMutex);
								return RSSL_RET_FAILURE;
							}
						}

						/* Inserted to the HashTable for searching the token sessions */
						rsslHashTableInsertLink(&pTokenManagementImpl->sessionByNameAndClientIdHt, &pTokenSessionImpl->hashLinkNameAndClientId,
							&pTokenSessionImpl->userNameAndClientId, NULL);

						if ((pTokenSessionImpl->pOAuthCredential = rsslCreateOAuthCredentialCopyV1(pOAuthCredential, pConsRole->pLoginRequest, pConsRole, &ret)) == NULL)
						{
							if (ret == RSSL_RET_FAILURE)
							{
								rsslSetErrorInfo(pError, RSSL_EIC_FAILURE, ret, __FILE__, __LINE__, "Failed to copy ommConsumerRole OAuth credential.");
							}
							else if (ret == RSSL_RET_INVALID_ARGUMENT)
							{
								rsslSetErrorInfo(pError, RSSL_EIC_FAILURE, ret, __FILE__, __LINE__, "Failed to copy OAuth credential for enabling the session management.");
							}

							/* Set the pointer to passed in RsslRDMLoginRequest to NULL to avoid freeing the user's memory */
							pConsRole->pLoginRequest = NULL;
							pConsRole->pDirectoryRequest = NULL;

							RSSL_MUTEX_UNLOCK(&pTokenManagementImpl->tokenSessionMutex);
							return ret;
						}

					}

					/* Don't unlock the mutex yet when the session management is enabled to perform OAuth authentication and service discovery */
					if (pReactorConnectInfoImpl->base.enableSessionManagement == RSSL_FALSE)
					{
						RSSL_MUTEX_UNLOCK(&pTokenManagementImpl->tokenSessionMutex);
					}
				}
				else // V2 login
				{
					/* For the V2 login, we will not need to manage any of the session information.  Access token lifetimes should be long enough to handle any reconnection timer, so we will just 
						request a new access token on reconnect and use that until we have successfully reconnected. In addition, we do not need to lock here, because the only time this data will change 
						is either from the worker thread on a URL redirect, or on the dispatching thread to get credentials during a reconnect scenario  */
					pTokenSessionImpl = (RsslReactorTokenSessionImpl*)malloc(sizeof(RsslReactorTokenSessionImpl));
					rsslClearReactorTokenSessionImpl(pTokenSessionImpl);
					pTokenSessionImpl->pReactor = (RsslReactor*)pReactorChannel->pParentReactor;
					pTokenSessionImpl->reissueTokenAttemptLimit = pReactorChannel->pParentReactor->reissueTokenAttemptLimit;
					pTokenSessionImpl->sessionVersion = RSSL_RC_SESSMGMT_V2;


					/* Copy the rsslReactor's token service URL if present.  If not, use the default location for V2 */
					if (pReactorChannel->pParentReactor->tokenServiceURLV2.data != 0 && pReactorChannel->pParentReactor->tokenServiceURLV2.length != 0)
					{
						pTokenSessionImpl->sessionAuthUrl.length = pReactorChannel->pParentReactor->tokenServiceURLBufferV2.length;
						if (!(pTokenSessionImpl->sessionAuthUrl.data = (char*)malloc(pReactorChannel->pParentReactor->tokenServiceURLBufferV2.length+1)))
						{
							rsslSetErrorInfo(pError, RSSL_EIC_FAILURE, RSSL_RET_FAILURE, __FILE__, __LINE__, "Failed to allocate memory for the token service URL.");
							/* Set the pointers to NULL to avoid freeing the user's memory */
							pConsRole->pLoginRequest = NULL;
							pConsRole->pDirectoryRequest = NULL;

							return RSSL_RET_FAILURE;
						}

						memset(pTokenSessionImpl->sessionAuthUrl.data, 0, pReactorChannel->pParentReactor->tokenServiceURLBufferV2.length+1);
						strncpy(pTokenSessionImpl->sessionAuthUrl.data, pReactorChannel->pParentReactor->tokenServiceURLBufferV2.data, pTokenSessionImpl->sessionAuthUrl.length);
					}
					else
					{
						pTokenSessionImpl->sessionAuthUrl.length = rssl_rest_token_url_v2.length;
						if (!(pTokenSessionImpl->sessionAuthUrl.data = (char*)malloc(rssl_rest_token_url_v2.length+1)))
						{
							rsslSetErrorInfo(pError, RSSL_EIC_FAILURE, RSSL_RET_FAILURE, __FILE__, __LINE__, "Failed to allocate memory for the token service URL.");
							/* Set the pointers to NULL to avoid freeing the user's memory */
							pConsRole->pLoginRequest = NULL;
							pConsRole->pDirectoryRequest = NULL;

							return RSSL_RET_FAILURE;
						}

						memset(pTokenSessionImpl->sessionAuthUrl.data, 0, rssl_rest_token_url_v2.length+1);
						strncpy(pTokenSessionImpl->sessionAuthUrl.data, rssl_rest_token_url_v2.data, pTokenSessionImpl->sessionAuthUrl.length);
					}

					/* Allocates memory to store temporary URL redirect */
					pTokenSessionImpl->temporaryURLBuffer.length = RSSL_REACTOR_DEFAULT_URL_LENGHT;
					pTokenSessionImpl->temporaryURLBuffer.data = (char*)malloc(pTokenSessionImpl->temporaryURLBuffer.length);
					if (pTokenSessionImpl->temporaryURLBuffer.data == 0)
					{
						rsslSetErrorInfo(pError, RSSL_EIC_FAILURE, RSSL_RET_FAILURE, __FILE__, __LINE__,
							"Failed to allocate memory for storing temporary URL redirect.");

						free(pTokenSessionImpl->sessionAuthUrl.data);
						free(pTokenSessionImpl);
						pReactorChannel->pTokenSessionImpl = NULL;

						/* Set the pointers to NULL to avoid freeing the user's memory */
						pConsRole->pLoginRequest = NULL;
						pConsRole->pDirectoryRequest = NULL;

						return RSSL_RET_FAILURE;
					}
					/* Assigned the token session to the channel */
					pReactorChannel->pTokenSessionImpl = pTokenSessionImpl;

					if ((pTokenSessionImpl->pOAuthCredential = rsslCreateOAuthCredentialCopyV2(pOAuthCredential, pConsRole->pLoginRequest, pConsRole, &ret)) == NULL)
					{
						if (ret == RSSL_RET_FAILURE)
						{
							rsslSetErrorInfo(pError, RSSL_EIC_FAILURE, ret, __FILE__, __LINE__, "Failed to copy ommConsumerRole OAuth credential.");
						}
						else if (ret == RSSL_RET_INVALID_ARGUMENT)
						{
							rsslSetErrorInfo(pError, RSSL_EIC_FAILURE, ret, __FILE__, __LINE__, "Failed to copy OAuth credential for enabling the session management.");
						}

						/* Set the pointer to passed in RsslRDMLoginRequest to NULL to avoid freeing the user's memory */
						pConsRole->pLoginRequest = NULL;
						pConsRole->pDirectoryRequest = NULL;

						return ret;
					}
				}
			}
			else
			{
				pConsRole->pOAuthCredential = NULL; /* Unset the RsslReactorOAuthCredential as the session management is disable. */
			}

			if (pConsRole->pLoginRequest
					&& (pConsRole->pLoginRequest =
						(RsslRDMLoginRequest*)rsslCreateRDMMsgCopy((RsslRDMMsg*)pConsRole->pLoginRequest, 256, &ret)) == NULL)
			{
				rsslSetErrorInfo(pError, RSSL_EIC_FAILURE, RSSL_RET_FAILURE, __FILE__, __LINE__, "Failed to copy ommConsumerRole login request.");

				if (pReactorConnectInfoImpl->base.enableSessionManagement)
				{
					RsslReactorTokenManagementImpl *pTokenManagementImpl = &pReactorChannel->pParentReactor->reactorWorker.reactorTokenManagement;
					RSSL_MUTEX_UNLOCK(&pTokenManagementImpl->tokenSessionMutex);
				}
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

				rsslSetErrorInfo(pError, RSSL_EIC_FAILURE, RSSL_RET_FAILURE, __FILE__, __LINE__, "Failed to copy ommConsumerRole directory request.");

				if (pReactorConnectInfoImpl->base.enableSessionManagement)
				{
					RsslReactorTokenManagementImpl *pTokenManagementImpl = &pReactorChannel->pParentReactor->reactorWorker.reactorTokenManagement;
					RSSL_MUTEX_UNLOCK(&pTokenManagementImpl->tokenSessionMutex);
				}
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
				rsslSetErrorInfo(pError, RSSL_EIC_FAILURE, RSSL_RET_FAILURE, __FILE__, __LINE__, "Failed to copy ommNIProviderRole login request.");
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

				rsslSetErrorInfo(pError, RSSL_EIC_FAILURE, RSSL_RET_FAILURE, __FILE__, __LINE__, "Failed to copy ommNIProviderRole directory refresh.");
				return RSSL_RET_FAILURE;
			}
			break;
		}

		default:
			break;
	}

	return RSSL_RET_SUCCESS;
}

static RsslRet _reactorChannelCopyRoleForWarmStandBy(RsslReactorChannelImpl* pReactorChannel, RsslReactorChannelRole* pRole, RsslErrorInfo* pError)
{
	RsslRet ret;

	pReactorChannel->channelRole = *pRole;

	switch (pReactorChannel->channelRole.base.roleType)
	{
		case RSSL_RC_RT_OMM_CONSUMER:
		{
			RsslReactorOMMConsumerRole* pConsRole = (RsslReactorOMMConsumerRole*)&pReactorChannel->channelRole;

			if (pConsRole->pLoginRequest
				&& (pConsRole->pLoginRequest =
				(RsslRDMLoginRequest*)rsslCreateRDMMsgCopy((RsslRDMMsg*)pConsRole->pLoginRequest, 256, &ret)) == NULL)
			{
				rsslSetErrorInfo(pError, RSSL_EIC_FAILURE, RSSL_RET_FAILURE, __FILE__, __LINE__, "Failed to copy ommConsumerRole login request.");
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

				rsslSetErrorInfo(pError, RSSL_EIC_FAILURE, RSSL_RET_FAILURE, __FILE__, __LINE__, "Failed to copy ommConsumerRole directory request.");
				return RSSL_RET_FAILURE;
			}

			break;
		}
	}

	return RSSL_RET_SUCCESS;
}



RsslRestRequestArgs* _reactorCreateRequestArgsForPassword(RsslReactorImpl *pRsslReactorImpl, RsslBuffer *pTokenServiceURL, RsslBuffer *pUserName, RsslBuffer *pPassword, RsslBuffer* pNewPassword, RsslBuffer *pClientId, 
								RsslBuffer *pClientSecret, RsslBuffer *pTokenScope, RsslBool takeExclusiveSignOnControl, RsslBuffer *pHeaderAndDataBodyBuf, void *pUserSpecPtr, RsslErrorInfo *pError)
{
	/* Get authentication token using the password */
	RsslRestRequestArgs *pRequestArgs = 0;
	RsslRestHeader  *pRsslRestAcceptHeader = 0;
	RsslRestHeader  *pRsslRestContentTypeHeader = 0;
	RsslBuffer *pRsslEncodedUrl = 0;
	RsslBuffer *pRsslUserNameEncodedUrl = 0;
	RsslBuffer *pRsslNewPasswordEncodedUrl = 0;
	RsslUInt32 neededSize = 0;
	char* pCurPos = 0;
	RsslBuffer tokenScope = RSSL_INIT_BUFFER;
	RsslBool	hasClientSecret = RSSL_FALSE;
	RsslBuffer	clientId = RSSL_INIT_BUFFER;
	RsslBool	hasNewPassword = RSSL_FALSE;
	RsslBuffer	takeExclusiveSignOn = takeExclusiveSignOnControl ? 
		rssl_rest_take_exclusive_sign_on_true_text : rssl_rest_take_exclusive_sign_on_false_text;

	pRsslEncodedUrl = rsslRestEncodeUrlData(pPassword, &pError->rsslError);

	if (pError->rsslError.rsslErrorId == RSSL_RET_FAILURE)
	{
		rsslSetErrorInfo(pError, RSSL_EIC_FAILURE, RSSL_RET_FAILURE, __FILE__, __LINE__,
			"Failed to encode URL for the HTTP data body.");

		return 0;
	}

	if (pPassword != pRsslEncodedUrl)
	{
		pPassword = pRsslEncodedUrl;
	}
	else
	{
		pRsslEncodedUrl = NULL;
	}

	/* If specified to change the password */
	if (pNewPassword)
	{
		pRsslNewPasswordEncodedUrl = rsslRestEncodeUrlData(pNewPassword, &pError->rsslError);

		if (pError->rsslError.rsslErrorId == RSSL_RET_FAILURE)
		{
			if (pRsslEncodedUrl)
			{
				free(pRsslEncodedUrl->data);
				free(pRsslEncodedUrl);
			}

			rsslSetErrorInfo(pError, RSSL_EIC_FAILURE, RSSL_RET_FAILURE, __FILE__, __LINE__,
				"Failed to encode URL for the HTTP data body.");

			return 0;
		}

		if (pNewPassword != pRsslNewPasswordEncodedUrl)
		{
			pNewPassword = pRsslNewPasswordEncodedUrl;
		}
		else
		{
			pRsslNewPasswordEncodedUrl = NULL;
		}
	}

	pRsslUserNameEncodedUrl = rsslRestEncodeUrlData(pUserName, &pError->rsslError);

	if (pError->rsslError.rsslErrorId == RSSL_RET_FAILURE)
	{
		if (pRsslEncodedUrl)
		{
			free(pRsslEncodedUrl->data);
			free(pRsslEncodedUrl);
		}

		if (pRsslNewPasswordEncodedUrl)
		{
			free(pRsslNewPasswordEncodedUrl->data);
			free(pRsslNewPasswordEncodedUrl);
		}

		rsslSetErrorInfo(pError, RSSL_EIC_FAILURE, RSSL_RET_FAILURE, __FILE__, __LINE__,
			"Failed to encode URL for the HTTP data body.");

		return 0;
	}

	if (pUserName != pRsslUserNameEncodedUrl)
	{
		pUserName = pRsslUserNameEncodedUrl;
	}
	else
	{
		pRsslUserNameEncodedUrl = NULL;
	}

	pRequestArgs = (RsslRestRequestArgs*)malloc(sizeof(RsslRestRequestArgs));

	if (pRequestArgs == 0) goto memoryAllocationFailed;

	rsslClearRestRequestArgs(pRequestArgs);

	if (pTokenScope->length && pTokenScope->data)
	{
		tokenScope = *pTokenScope;
	}
	else
	{
		tokenScope = rssl_rest_service_discovery_token_scope;
	}

	if (pClientId->length && pClientId->data)
	{
		clientId = *pClientId;
	}
	else
	{
		clientId = *pUserName;
	}

	neededSize = rssl_rest_grant_type_password_text.length + rssl_rest_username_text.length +
		pUserName->length + rssl_rest_password_text.length +
		pPassword->length + rssl_rest_client_id_text.length +
		clientId.length + rssl_rest_scope_text.length +
		tokenScope.length + takeExclusiveSignOn.length + 1;

	/* Send the client secret only when it isn't an empty string. */
	if (pClientSecret->length && pClientSecret->data)
	{
		hasClientSecret = RSSL_TRUE;
		neededSize += rssl_rest_client_secret_text.length + pClientSecret->length;
	}

	if (pNewPassword && pNewPassword->data && pNewPassword->length)
	{
		hasNewPassword = RSSL_TRUE;
		neededSize += rssl_rest_new_password_text.length + pNewPassword->length;
	}

	neededSize += (RsslUInt32)(sizeof(RsslRestHeader) * 2);

	if (pHeaderAndDataBodyBuf->length < neededSize)
	{
		if (pHeaderAndDataBodyBuf->data)
			free(pHeaderAndDataBodyBuf->data);

		pHeaderAndDataBodyBuf->data = (char*)malloc(neededSize);

		if (pHeaderAndDataBodyBuf->data == 0) 
		{
			rsslClearBuffer(pHeaderAndDataBodyBuf);
			goto memoryAllocationFailed;
		}

		pHeaderAndDataBodyBuf->length = neededSize;
	}

	memset(pHeaderAndDataBodyBuf->data, 0, pHeaderAndDataBodyBuf->length);

	pRequestArgs->httpBody.data = pHeaderAndDataBodyBuf->data;

	strncat(pRequestArgs->httpBody.data, rssl_rest_grant_type_password_text.data, rssl_rest_grant_type_password_text.length);
	strncat(pRequestArgs->httpBody.data, rssl_rest_username_text.data, rssl_rest_username_text.length);
	strncat(pRequestArgs->httpBody.data, pUserName->data, pUserName->length);
	strncat(pRequestArgs->httpBody.data, rssl_rest_password_text.data, rssl_rest_password_text.length);
	strncat(pRequestArgs->httpBody.data, pPassword->data, pPassword->length);
	if (hasNewPassword)
	{
		strncat(pRequestArgs->httpBody.data, rssl_rest_new_password_text.data, rssl_rest_new_password_text.length);
		strncat(pRequestArgs->httpBody.data, pNewPassword->data, pNewPassword->length);
	}
	strncat(pRequestArgs->httpBody.data, rssl_rest_client_id_text.data, rssl_rest_client_id_text.length);
	strncat(pRequestArgs->httpBody.data, clientId.data, clientId.length);
	if (hasClientSecret)
	{
		strncat(pRequestArgs->httpBody.data, rssl_rest_client_secret_text.data, rssl_rest_client_secret_text.length);
		strncat(pRequestArgs->httpBody.data, pClientSecret->data, pClientSecret->length);
	}
	strncat(pRequestArgs->httpBody.data, rssl_rest_scope_text.data, rssl_rest_scope_text.length);
	strncat(pRequestArgs->httpBody.data, tokenScope.data, tokenScope.length);
	strncat(pRequestArgs->httpBody.data, takeExclusiveSignOn.data, takeExclusiveSignOn.length);
	pRequestArgs->httpBody.length = (RsslUInt32)strlen(pRequestArgs->httpBody.data);

	pCurPos = (pHeaderAndDataBodyBuf->data + pRequestArgs->httpBody.length + 1); // Adding 1 for null terminate string
	pRsslRestAcceptHeader = (RsslRestHeader*)(pCurPos);

	rsslClearRestHeader(pRsslRestAcceptHeader);
	pRsslRestAcceptHeader->key.data = rssl_rest_accept_text.data;
	pRsslRestAcceptHeader->key.length = rssl_rest_accept_text.length;
	pRsslRestAcceptHeader->value.data = rssl_rest_application_json_text.data;
	pRsslRestAcceptHeader->value.length = rssl_rest_application_json_text.length;

	pRsslRestContentTypeHeader = (RsslRestHeader*)(pCurPos + sizeof(RsslRestHeader));

	rsslClearRestHeader(pRsslRestContentTypeHeader);
	pRsslRestContentTypeHeader->key.data = rssl_rest_content_type_text.data;
	pRsslRestContentTypeHeader->key.length = rssl_rest_content_type_text.length;
	pRsslRestContentTypeHeader->value.data = rssl_rest_application_form_urlencoded_text.data;
	pRsslRestContentTypeHeader->value.length = rssl_rest_application_form_urlencoded_text.length;

	rsslQueueAddLinkToBack(&pRequestArgs->httpHeaders, &pRsslRestAcceptHeader->queueLink);
	rsslQueueAddLinkToBack(&pRequestArgs->httpHeaders, &pRsslRestContentTypeHeader->queueLink);

	pRequestArgs->httpMethod = RSSL_REST_HTTP_POST;
	pRequestArgs->url.data = pTokenServiceURL->data;
	pRequestArgs->url.length = pTokenServiceURL->length;

	if (pRsslEncodedUrl)
	{
		free(pRsslEncodedUrl->data);
		free(pRsslEncodedUrl);
	}

	if (pRsslNewPasswordEncodedUrl)
	{
		free(pRsslNewPasswordEncodedUrl->data);
		free(pRsslNewPasswordEncodedUrl);
	}

	if (pRsslUserNameEncodedUrl)
	{
		free(pRsslUserNameEncodedUrl->data);
		free(pRsslUserNameEncodedUrl);
	}

	pRequestArgs->pUserSpecPtr = pUserSpecPtr;

	/* Assigned the request timeout in seconds */
	pRequestArgs->requestTimeOut = pRsslReactorImpl->restRequestTimeout;

	return pRequestArgs;

memoryAllocationFailed:

	if (pRsslEncodedUrl)
	{
		free(pRsslEncodedUrl->data);
		free(pRsslEncodedUrl);
	}

	if (pRsslNewPasswordEncodedUrl)
	{
		free(pRsslNewPasswordEncodedUrl->data);
		free(pRsslNewPasswordEncodedUrl);
	}

	if (pRsslUserNameEncodedUrl)
	{
		free(pRsslUserNameEncodedUrl->data);
		free(pRsslUserNameEncodedUrl);
	}

	if (pRequestArgs) free(pRequestArgs);

	rsslSetErrorInfo(pError, RSSL_EIC_FAILURE, RSSL_RET_FAILURE, __FILE__, __LINE__,
		"Failed to allocate memory for creating request arguments for the token service using the password.");

	return 0;
}

RsslRestRequestArgs* _reactorCreateTokenRequestV1(RsslReactorImpl* pRsslReactorImpl, RsslBuffer* pTokenServiceURL, RsslBuffer* pUserName, RsslBuffer* pPassword, RsslBuffer* pNewPassword, RsslBuffer* pClientId,
	RsslBuffer* pClientSecret, RsslBuffer* pTokenScope, RsslBool takeExclusiveSignOnControl, RsslBuffer* pHeaderAndDataBodyBuf, void* pUserSpecPtr, RsslErrorInfo* pError)
{
	/* Get authentication token using the password */
	RsslRestRequestArgs* pRequestArgs = 0;
	RsslRestHeader* pRsslRestAcceptHeader = 0;
	RsslRestHeader* pRsslRestContentTypeHeader = 0;
	RsslBuffer* pRsslEncodedUrl = 0;
	RsslBuffer* pRsslUserNameEncodedUrl = 0;
	RsslBuffer* pRsslNewPasswordEncodedUrl = 0;
	RsslUInt32 neededSize = 0;
	char* pCurPos = 0;
	RsslBuffer tokenScope = RSSL_INIT_BUFFER;
	RsslBool	hasClientSecret = RSSL_FALSE;
	RsslBuffer	clientId = RSSL_INIT_BUFFER;
	RsslBool	hasNewPassword = RSSL_FALSE;
	RsslBuffer	takeExclusiveSignOn = takeExclusiveSignOnControl ?
		rssl_rest_take_exclusive_sign_on_true_text : rssl_rest_take_exclusive_sign_on_false_text;

	pRsslEncodedUrl = rsslRestEncodeUrlData(pPassword, &pError->rsslError);

	if (pError->rsslError.rsslErrorId == RSSL_RET_FAILURE)
	{
		rsslSetErrorInfo(pError, RSSL_EIC_FAILURE, RSSL_RET_FAILURE, __FILE__, __LINE__,
			"Failed to encode URL for the HTTP data body.");

		return 0;
	}

	if (pPassword != pRsslEncodedUrl)
	{
		pPassword = pRsslEncodedUrl;
	}
	else
	{
		pRsslEncodedUrl = NULL;
	}

	/* If specified to change the password */
	if (pNewPassword)
	{
		pRsslNewPasswordEncodedUrl = rsslRestEncodeUrlData(pNewPassword, &pError->rsslError);

		if (pError->rsslError.rsslErrorId == RSSL_RET_FAILURE)
		{
			if (pRsslEncodedUrl)
			{
				free(pRsslEncodedUrl->data);
				free(pRsslEncodedUrl);
			}

			rsslSetErrorInfo(pError, RSSL_EIC_FAILURE, RSSL_RET_FAILURE, __FILE__, __LINE__,
				"Failed to encode URL for the HTTP data body.");

			return 0;
		}

		if (pNewPassword != pRsslNewPasswordEncodedUrl)
		{
			pNewPassword = pRsslNewPasswordEncodedUrl;
		}
		else
		{
			pRsslNewPasswordEncodedUrl = NULL;
		}
	}

	pRsslUserNameEncodedUrl = rsslRestEncodeUrlData(pUserName, &pError->rsslError);

	if (pError->rsslError.rsslErrorId == RSSL_RET_FAILURE)
	{
		if (pRsslEncodedUrl)
		{
			free(pRsslEncodedUrl->data);
			free(pRsslEncodedUrl);
		}

		if (pRsslNewPasswordEncodedUrl)
		{
			free(pRsslNewPasswordEncodedUrl->data);
			free(pRsslNewPasswordEncodedUrl);
		}

		rsslSetErrorInfo(pError, RSSL_EIC_FAILURE, RSSL_RET_FAILURE, __FILE__, __LINE__,
			"Failed to encode URL for the HTTP data body.");

		return 0;
	}

	if (pUserName != pRsslUserNameEncodedUrl)
	{
		pUserName = pRsslUserNameEncodedUrl;
	}
	else
	{
		pRsslUserNameEncodedUrl = NULL;
	}

	pRequestArgs = (RsslRestRequestArgs*)malloc(sizeof(RsslRestRequestArgs));

	if (pRequestArgs == 0) goto memoryAllocationFailed;

	rsslClearRestRequestArgs(pRequestArgs);

	if (pTokenScope->length && pTokenScope->data)
	{
		tokenScope = *pTokenScope;
	}
	else
	{
		tokenScope = rssl_rest_service_discovery_token_scope;
	}

	if (pClientId->length && pClientId->data)
	{
		clientId = *pClientId;
	}
	else
	{
		clientId = *pUserName;
	}

	neededSize = rssl_rest_grant_type_password_text.length + rssl_rest_username_text.length +
		pUserName->length + rssl_rest_password_text.length +
		pPassword->length + rssl_rest_client_id_text.length +
		clientId.length + rssl_rest_scope_text.length +
		tokenScope.length + takeExclusiveSignOn.length + 1;

	/* Send the client secret only when it isn't an empty string. */
	if (pClientSecret->length && pClientSecret->data)
	{
		hasClientSecret = RSSL_TRUE;
		neededSize += rssl_rest_client_secret_text.length + pClientSecret->length;
	}

	if (pNewPassword && pNewPassword->data && pNewPassword->length)
	{
		hasNewPassword = RSSL_TRUE;
		neededSize += rssl_rest_new_password_text.length + pNewPassword->length;
	}

	neededSize += (RsslUInt32)(sizeof(RsslRestHeader) * 2);

	if (pHeaderAndDataBodyBuf->length < neededSize)
	{
		if (pHeaderAndDataBodyBuf->data)
			free(pHeaderAndDataBodyBuf->data);

		pHeaderAndDataBodyBuf->data = (char*)malloc(neededSize);

		if (pHeaderAndDataBodyBuf->data == 0)
		{
			rsslClearBuffer(pHeaderAndDataBodyBuf);
			goto memoryAllocationFailed;
		}

		pHeaderAndDataBodyBuf->length = neededSize;
	}

	memset(pHeaderAndDataBodyBuf->data, 0, pHeaderAndDataBodyBuf->length);

	pRequestArgs->httpBody.data = pHeaderAndDataBodyBuf->data;

	strncat(pRequestArgs->httpBody.data, rssl_rest_grant_type_password_text.data, rssl_rest_grant_type_password_text.length);
	strncat(pRequestArgs->httpBody.data, rssl_rest_username_text.data, rssl_rest_username_text.length);
	strncat(pRequestArgs->httpBody.data, pUserName->data, pUserName->length);
	strncat(pRequestArgs->httpBody.data, rssl_rest_password_text.data, rssl_rest_password_text.length);
	strncat(pRequestArgs->httpBody.data, pPassword->data, pPassword->length);
	if (hasNewPassword)
	{
		strncat(pRequestArgs->httpBody.data, rssl_rest_new_password_text.data, rssl_rest_new_password_text.length);
		strncat(pRequestArgs->httpBody.data, pNewPassword->data, pNewPassword->length);
	}
	strncat(pRequestArgs->httpBody.data, rssl_rest_client_id_text.data, rssl_rest_client_id_text.length);
	strncat(pRequestArgs->httpBody.data, clientId.data, clientId.length);
	if (hasClientSecret)
	{
		strncat(pRequestArgs->httpBody.data, rssl_rest_client_secret_text.data, rssl_rest_client_secret_text.length);
		strncat(pRequestArgs->httpBody.data, pClientSecret->data, pClientSecret->length);
	}
	strncat(pRequestArgs->httpBody.data, rssl_rest_scope_text.data, rssl_rest_scope_text.length);
	strncat(pRequestArgs->httpBody.data, tokenScope.data, tokenScope.length);
	strncat(pRequestArgs->httpBody.data, takeExclusiveSignOn.data, takeExclusiveSignOn.length);
	pRequestArgs->httpBody.length = (RsslUInt32)strlen(pRequestArgs->httpBody.data);

	pCurPos = (pHeaderAndDataBodyBuf->data + pRequestArgs->httpBody.length + 1); // Adding 1 for null terminate string
	pRsslRestAcceptHeader = (RsslRestHeader*)(pCurPos);

	rsslClearRestHeader(pRsslRestAcceptHeader);
	pRsslRestAcceptHeader->key.data = rssl_rest_accept_text.data;
	pRsslRestAcceptHeader->key.length = rssl_rest_accept_text.length;
	pRsslRestAcceptHeader->value.data = rssl_rest_application_json_text.data;
	pRsslRestAcceptHeader->value.length = rssl_rest_application_json_text.length;

	pRsslRestContentTypeHeader = (RsslRestHeader*)(pCurPos + sizeof(RsslRestHeader));

	rsslClearRestHeader(pRsslRestContentTypeHeader);
	pRsslRestContentTypeHeader->key.data = rssl_rest_content_type_text.data;
	pRsslRestContentTypeHeader->key.length = rssl_rest_content_type_text.length;
	pRsslRestContentTypeHeader->value.data = rssl_rest_application_form_urlencoded_text.data;
	pRsslRestContentTypeHeader->value.length = rssl_rest_application_form_urlencoded_text.length;

	rsslQueueAddLinkToBack(&pRequestArgs->httpHeaders, &pRsslRestAcceptHeader->queueLink);
	rsslQueueAddLinkToBack(&pRequestArgs->httpHeaders, &pRsslRestContentTypeHeader->queueLink);

	pRequestArgs->httpMethod = RSSL_REST_HTTP_POST;
	pRequestArgs->url.data = pTokenServiceURL->data;
	pRequestArgs->url.length = pTokenServiceURL->length;

	if (pRsslEncodedUrl)
	{
		free(pRsslEncodedUrl->data);
		free(pRsslEncodedUrl);
	}

	if (pRsslNewPasswordEncodedUrl)
	{
		free(pRsslNewPasswordEncodedUrl->data);
		free(pRsslNewPasswordEncodedUrl);
	}

	if (pRsslUserNameEncodedUrl)
	{
		free(pRsslUserNameEncodedUrl->data);
		free(pRsslUserNameEncodedUrl);
	}

	pRequestArgs->pUserSpecPtr = pUserSpecPtr;

	/* Assigned the request timeout in seconds */
	pRequestArgs->requestTimeOut = pRsslReactorImpl->restRequestTimeout;

	return pRequestArgs;

memoryAllocationFailed:

	if (pRsslEncodedUrl)
	{
		free(pRsslEncodedUrl->data);
		free(pRsslEncodedUrl);
	}

	if (pRsslNewPasswordEncodedUrl)
	{
		free(pRsslNewPasswordEncodedUrl->data);
		free(pRsslNewPasswordEncodedUrl);
	}

	if (pRsslUserNameEncodedUrl)
	{
		free(pRsslUserNameEncodedUrl->data);
		free(pRsslUserNameEncodedUrl);
	}

	if (pRequestArgs) free(pRequestArgs);

	rsslSetErrorInfo(pError, RSSL_EIC_FAILURE, RSSL_RET_FAILURE, __FILE__, __LINE__,
		"Failed to allocate memory for creating request arguments for the token service using the password.");

	return 0;
}

RsslRestRequestArgs* _reactorCreateTokenRequestV2(RsslReactorImpl* pReactorImpl, RsslBuffer* pTokenServiceURL, RsslBuffer* pClientId, RsslBuffer* pClientSecret, RsslBuffer* pTokenScope, RsslBuffer* pHeaderAndDataBodyBuf, void* pUserSpecPtr, RsslErrorInfo* pError)
{
	/* Get authentication token using either a client id/client secret  */
	RsslRestRequestArgs* pRequestArgs = 0;
	RsslRestHeader* pRsslRestAcceptHeader = 0;
	RsslRestHeader* pRsslRestContentTypeHeader = 0;
	RsslRestHeader* pRsslRestUserAgentHeader = 0;
	RsslBuffer* pUrlEncodedSecret = 0;
	RsslBuffer* pUrlEncodedClientId = 0;
	RsslUInt32 neededSize = 0;
	char* pCurPos = 0;
	RsslBuffer tokenScope = RSSL_INIT_BUFFER;
	RsslBool	hasClientSecret = RSSL_FALSE;
	RsslBool	hasNewPassword = RSSL_FALSE;

	int secretLength = 0;

	pUrlEncodedClientId = rsslRestEncodeUrlData(pClientId, &pError->rsslError);

	if (pError->rsslError.rsslErrorId == RSSL_RET_FAILURE)
	{
		if (pUrlEncodedSecret)
		{
			free(pUrlEncodedSecret->data);
			free(pUrlEncodedSecret);
		}

		rsslSetErrorInfo(pError, RSSL_EIC_FAILURE, RSSL_RET_FAILURE, __FILE__, __LINE__,
			"Failed to encode cl for the HTTP data body.");

		return 0;
	}

	if (pClientId != pUrlEncodedClientId)
	{
		pClientId = pUrlEncodedClientId;
	}
	else
	{
		pUrlEncodedClientId = NULL;
	}

	hasClientSecret = RSSL_TRUE;
	pUrlEncodedSecret = rsslRestEncodeUrlData(pClientSecret, &pError->rsslError);

	if (pError->rsslError.rsslErrorId == RSSL_RET_FAILURE)
	{

		if (pUrlEncodedClientId)
		{
			free(pUrlEncodedClientId->data);
			free(pUrlEncodedClientId);
		}
		rsslSetErrorInfo(pError, RSSL_EIC_FAILURE, RSSL_RET_FAILURE, __FILE__, __LINE__,
			"Failed to encode client secret for the HTTP data body.");

		return 0;
	}

	if (pClientSecret != pUrlEncodedSecret)
	{
		pClientSecret = pUrlEncodedSecret;
	}
	else
	{
		pUrlEncodedSecret = NULL;
	}

	secretLength = pClientSecret->length;
	
	
	pRequestArgs = (RsslRestRequestArgs*)malloc(sizeof(RsslRestRequestArgs));

	if (pRequestArgs == 0) goto memoryAllocationFailed;

	rsslClearRestRequestArgs(pRequestArgs);

	neededSize = rssl_rest_grant_type_client_credential_text.length + rssl_rest_client_id_text.length +
		pClientId->length + rssl_rest_scope_text.length + pTokenScope->length + 1;
	

	neededSize += rssl_rest_client_secret_text.length + secretLength;
	

	neededSize += (RsslUInt32)(sizeof(RsslRestHeader) * 3);

	if (pHeaderAndDataBodyBuf->length < neededSize)
	{
		if (pHeaderAndDataBodyBuf->data)
			free(pHeaderAndDataBodyBuf->data);

		pHeaderAndDataBodyBuf->data = (char*)malloc(neededSize);

		if (pHeaderAndDataBodyBuf->data == 0)
		{
			rsslClearBuffer(pHeaderAndDataBodyBuf);
			goto memoryAllocationFailed;
		}

		pHeaderAndDataBodyBuf->length = neededSize;
	}

	memset(pHeaderAndDataBodyBuf->data, 0, pHeaderAndDataBodyBuf->length);

	pRequestArgs->httpBody.data = pHeaderAndDataBodyBuf->data;
	strncat(pRequestArgs->httpBody.data, rssl_rest_grant_type_client_credential_text.data, rssl_rest_grant_type_client_credential_text.length);
	strncat(pRequestArgs->httpBody.data, rssl_rest_scope_text.data, rssl_rest_scope_text.length);
	strncat(pRequestArgs->httpBody.data, pTokenScope->data, pTokenScope->length);
	strncat(pRequestArgs->httpBody.data, rssl_rest_client_id_text.data, rssl_rest_client_id_text.length);
	strncat(pRequestArgs->httpBody.data, pClientId->data, pClientId->length);

	strncat(pRequestArgs->httpBody.data, rssl_rest_client_secret_text.data, rssl_rest_client_secret_text.length);
	strncat(pRequestArgs->httpBody.data, pClientSecret->data, pClientSecret->length);

	pRequestArgs->httpBody.length = (RsslUInt32)strlen(pRequestArgs->httpBody.data);

	pCurPos = (pRequestArgs->httpBody.data + pRequestArgs->httpBody.length + 1); // Adding 1 for null terminate string


	pRsslRestContentTypeHeader = (RsslRestHeader*)(pCurPos);
	rsslClearRestHeader(pRsslRestContentTypeHeader);
	pRsslRestContentTypeHeader->key.data = rssl_rest_content_type_text.data;
	pRsslRestContentTypeHeader->key.length = rssl_rest_content_type_text.length;
	pRsslRestContentTypeHeader->value.data = rssl_rest_application_form_urlencoded_text.data;
	pRsslRestContentTypeHeader->value.length = rssl_rest_application_form_urlencoded_text.length;

	pRsslRestUserAgentHeader = (RsslRestHeader*)(pCurPos + sizeof(RsslRestHeader));
	rsslClearRestHeader(pRsslRestUserAgentHeader);
	pRsslRestUserAgentHeader->key.data = rssl_rest_user_agent_text.data;
	pRsslRestUserAgentHeader->key.length = rssl_rest_user_agent_text.length;
	pRsslRestUserAgentHeader->value.data = rssl_rest_user_agent_rtsdk_text.data;
	pRsslRestUserAgentHeader->value.length = rssl_rest_user_agent_rtsdk_text.length;

	rsslQueueAddLinkToBack(&pRequestArgs->httpHeaders, &pRsslRestContentTypeHeader->queueLink);
	rsslQueueAddLinkToBack(&pRequestArgs->httpHeaders, &pRsslRestUserAgentHeader->queueLink);

	pRequestArgs->httpMethod = RSSL_REST_HTTP_POST;
	pRequestArgs->url.data = pTokenServiceURL->data;
	pRequestArgs->url.length = pTokenServiceURL->length;

	if (pUrlEncodedSecret)
	{
		free(pUrlEncodedSecret->data);
		free(pUrlEncodedSecret);
	}


	if (pUrlEncodedClientId)
	{
		free(pUrlEncodedClientId->data);
		free(pUrlEncodedClientId);
	}

	pRequestArgs->pUserSpecPtr = pUserSpecPtr;

	/* Assigned the request timeout in seconds */
	pRequestArgs->requestTimeOut = pReactorImpl->restRequestTimeout;

	return pRequestArgs;

memoryAllocationFailed:

	if (pUrlEncodedSecret)
	{
		free(pUrlEncodedSecret->data);
		free(pUrlEncodedSecret);
	}

	if (pUrlEncodedClientId)
	{
		free(pUrlEncodedClientId->data);
		free(pUrlEncodedClientId);
	}

	if (pRequestArgs) free(pRequestArgs);

	rsslSetErrorInfo(pError, RSSL_EIC_FAILURE, RSSL_RET_FAILURE, __FILE__, __LINE__,
		"Failed to allocate memory for creating request arguments for the token service using the password.");

	return 0;
}

RsslRestRequestArgs* _reactorCreateRequestArgsForServiceDiscovery(RsslReactorImpl *pReactorImpl, RsslBuffer *pServiceDiscoveryURL, RsslReactorDiscoveryTransportProtocol transport,
	RsslReactorDiscoveryDataFormatProtocol dataFormat, RsslBuffer *pTokenType, RsslBuffer *pAccessToken,
	RsslBuffer *pArgsAndHeaderBuf, void *pUserSpecPtr, RsslErrorInfo* pError)
{
	RsslBuffer serviceDiscoveryURLBuffer;
	RsslRestHeader *pAuthHeader = 0;
	RsslRestHeader *pTokenHeader = 0;
	RsslRestRequestArgs *pRsslRestRequestArgs = 0;
	RsslUInt32 neededSize = 0;
	char* nextPos = 0;
	char parametersBuf[128]; // Allocate temporary memory to hold the entire query parameters.
	char *pParameterBuf = &parametersBuf[0];

	pRsslRestRequestArgs = (RsslRestRequestArgs*)malloc(sizeof(RsslRestRequestArgs));

	if (pRsslRestRequestArgs == 0) goto memoryAllocationFailed;

	rsslClearRestRequestArgs(pRsslRestRequestArgs);
	pRsslRestRequestArgs->httpMethod = RSSL_REST_HTTP_GET;

	neededSize = rssl_rest_authorization_text.length + pTokenType->length + 1 + pAccessToken->length +
								rssl_rest_accept_text.length + rssl_rest_application_json_text.length +
								pServiceDiscoveryURL->length + 1;

	memset(pParameterBuf, 0, 128);

	if ((transport != RSSL_RD_TP_INIT) || (dataFormat != RSSL_RD_DP_INIT))
	{
		switch (transport)
		{
		case RSSL_RD_TP_INIT:
			break;
		case RSSL_RD_TP_TCP:
			strcat(pParameterBuf, "?");
			strncat(pParameterBuf, rssl_rest_transport_type_tcp_text.data, rssl_rest_transport_type_tcp_text.length);
			break;
		case RSSL_RD_TP_WEBSOCKET:
			strcat(pParameterBuf, "?");
			strncat(pParameterBuf, rssl_rest_transport_type_websocket_text.data, rssl_rest_transport_type_websocket_text.length);
			break;
		default:
			free(pRsslRestRequestArgs);
			rsslSetErrorInfo(pError, RSSL_EIC_FAILURE, RSSL_RET_INVALID_ARGUMENT, __FILE__, __LINE__,
				"Invalid transport protocol(%d) for requesting RDP service discovery.",
				transport);
			return 0;
		}

		switch (dataFormat)
		{
		case RSSL_RD_DP_INIT:
			break;
		case RSSL_RD_DP_RWF:
			if (strlen(pParameterBuf) == 0)
			{
				strcat(pParameterBuf, "?");
				strncat(pParameterBuf, rssl_rest_dataformat_type_rwf_text.data, rssl_rest_dataformat_type_rwf_text.length);
			}
			else
			{
				strcat(pParameterBuf, "&");
				strncat(pParameterBuf, rssl_rest_dataformat_type_rwf_text.data, rssl_rest_dataformat_type_rwf_text.length);;
			}
			break;
		case RSSL_RD_DP_JSON2:
			if (strlen(pParameterBuf) == 0)
			{
				strcat(pParameterBuf, "?");
				strncat(pParameterBuf, rssl_rest_dataformat_type_tr_json2_text.data, rssl_rest_dataformat_type_tr_json2_text.length);
			}
			else
			{
				strcat(pParameterBuf, "&");
				strncat(pParameterBuf, rssl_rest_dataformat_type_tr_json2_text.data, rssl_rest_dataformat_type_tr_json2_text.length);
			}
			break;
		default:
			free(pRsslRestRequestArgs);
			rsslSetErrorInfo(pError, RSSL_EIC_FAILURE, RSSL_RET_INVALID_ARGUMENT, __FILE__, __LINE__,
				"Invalid data format protocol(%d) for requesting RDP service discovery.",
				transport);
			return 0;
		}
	}

	neededSize += (RsslUInt32)(strlen(pParameterBuf) + (sizeof(RsslRestHeader) * 2));

	if (pArgsAndHeaderBuf->length < neededSize)
	{
		if (pArgsAndHeaderBuf->data)
			free(pArgsAndHeaderBuf->data);

		pArgsAndHeaderBuf->data = (char*)malloc(neededSize);

		if (pArgsAndHeaderBuf->data == 0) 
		{
			rsslClearBuffer(pArgsAndHeaderBuf);
			goto memoryAllocationFailed;
		}

		pArgsAndHeaderBuf->length = neededSize;
	}
				
	memset(pArgsAndHeaderBuf->data, 0, pArgsAndHeaderBuf->length);
				
	pAuthHeader = (RsslRestHeader*)pArgsAndHeaderBuf->data;
	nextPos = pArgsAndHeaderBuf->data + sizeof(RsslRestHeader);

	pTokenHeader = (RsslRestHeader*)nextPos;
	nextPos += sizeof(RsslRestHeader);

	serviceDiscoveryURLBuffer.data = nextPos;
	strncat(serviceDiscoveryURLBuffer.data, pServiceDiscoveryURL->data, pServiceDiscoveryURL->length);

	if (strlen(pParameterBuf) > 0)
	{
		strcat(serviceDiscoveryURLBuffer.data, pParameterBuf);
	}
	
	serviceDiscoveryURLBuffer.length = (RsslUInt32)strlen(serviceDiscoveryURLBuffer.data);
	nextPos = serviceDiscoveryURLBuffer.data + serviceDiscoveryURLBuffer.length + 1; // Adding 1 for null terminate string

	pRsslRestRequestArgs->url = serviceDiscoveryURLBuffer;

	rsslClearRestHeader(pAuthHeader);
	pAuthHeader->key.data = rssl_rest_authorization_text.data;
	pAuthHeader->key.length = rssl_rest_authorization_text.length;

	pAuthHeader->value.data = nextPos;
	strncat(pAuthHeader->value.data, pTokenType->data,pTokenType->length);
	strcat(pAuthHeader->value.data, " ");
	strncat(pAuthHeader->value.data, pAccessToken->data, pAccessToken->length);
	pAuthHeader->value.length = pTokenType->length + 1 + pAccessToken->length;

	rsslClearRestHeader(pTokenHeader);
	pTokenHeader->key.data = rssl_rest_accept_text.data;
	pTokenHeader->key.length = rssl_rest_accept_text.length;
	pTokenHeader->value.data = rssl_rest_application_json_text.data;
	pTokenHeader->value.length = rssl_rest_application_json_text.length;

	rsslQueueAddLinkToBack(&pRsslRestRequestArgs->httpHeaders, &pAuthHeader->queueLink);
	rsslQueueAddLinkToBack(&pRsslRestRequestArgs->httpHeaders, &pTokenHeader->queueLink);

	pRsslRestRequestArgs->pUserSpecPtr = pUserSpecPtr;

	/* Assigned the request timeout in seconds */
	pRsslRestRequestArgs->requestTimeOut = pReactorImpl->restRequestTimeout;

	return pRsslRestRequestArgs;
		
memoryAllocationFailed:
			
	if(pRsslRestRequestArgs) free(pRsslRestRequestArgs);
			
	rsslSetErrorInfo(pError, RSSL_EIC_FAILURE, RSSL_RET_FAILURE, __FILE__, __LINE__,
		"Failed to allocate memory for creating request arguments for the service discovery.");

	return 0;
}

RsslRet _reactorApplyServiceDiscoveryEndpoint(RsslConnectOptions* pConnOptions, RsslBuffer *pHostName, RsslBuffer *pPort)
{
	// Free the original copy if any such as empty string
	if (pConnOptions->connectionInfo.unified.address)
	{
		free(pConnOptions->connectionInfo.unified.address);
	}

	pConnOptions->connectionInfo.unified.address = (char*)malloc(pHostName->length + (size_t)1);

	if (pConnOptions->connectionInfo.unified.address == 0)
	{
		return RSSL_RET_FAILURE;
	}

	strncpy(pConnOptions->connectionInfo.unified.address, pHostName->data, pHostName->length + (size_t)1);

	// Free the original copy if any such as empty string
	if (pConnOptions->connectionInfo.unified.serviceName)
	{
		free(pConnOptions->connectionInfo.unified.serviceName);
	}

	pConnOptions->connectionInfo.unified.serviceName = (char*)malloc(pPort->length + (size_t)1);
	if (pConnOptions->connectionInfo.unified.serviceName == 0)
	{
		free(pConnOptions->connectionInfo.unified.address);
		pConnOptions->connectionInfo.unified.address = 0;
		return RSSL_RET_FAILURE;
	}

	strncpy(pConnOptions->connectionInfo.unified.serviceName, pPort->data, pPort->length + (size_t)1);

	return RSSL_RET_SUCCESS;
}

RsslRet _reactorGetAccessToken(RsslReactorChannelImpl* pReactorChannelImpl, RsslReactorConnectInfoImpl* pReactorConnectInfoImpl, RsslErrorInfo* pError)
{
	RsslReactorTokenSessionImpl* pTokenSessionImpl = pReactorChannelImpl->pTokenSessionImpl;
	RsslReactorChannelRole* pReactorChannelRole = &pReactorChannelImpl->channelRole;
	RsslReactorImpl* pReactorImpl = pReactorChannelImpl->pParentReactor;
	RsslReactorTokenSessionEvent* pEvent = NULL;
	RsslReactorTokenSessionEventType tokenSessionEventType = RSSL_RCIMPL_TSET_INIT;
	RsslRet rsslRet;
	RsslRestRequestArgs* pRestRequestArgs = 0;
	RsslRestResponse restResponse;
	RsslReactorOAuthCredential* pReactorOAuthCredential;
	RsslConnectOptions* pConnOptions = &pReactorConnectInfoImpl->base.rsslConnectOptions;
	RsslErrorInfo errorInfo;

	pError->rsslError.rsslErrorId = RSSL_RET_SUCCESS; /* Always clears the error ID as it is used to check the result of this function. */

	pReactorOAuthCredential = pTokenSessionImpl->pOAuthCredential;

	/* Assign proxy information if any from the first connection for the token session */
	if (pTokenSessionImpl->copiedProxyOpts == RSSL_FALSE)
	{
		if (rsslDeepCopyConnectOpts(&pTokenSessionImpl->proxyConnectOpts, &pReactorConnectInfoImpl->base.rsslConnectOptions) != RSSL_RET_SUCCESS)
		{
			rsslSetErrorInfo(pError, RSSL_EIC_FAILURE, RSSL_RET_FAILURE, __FILE__, __LINE__,
				"Failed to copy proxy configuration parameters for the token session.");
			return RSSL_RET_FAILURE;
		}

		pTokenSessionImpl->copiedProxyOpts = RSSL_TRUE;
	}

	RTR_ATOMIC_SET(pTokenSessionImpl->requestingAccessToken, 1);
	RSSL_MUTEX_LOCK(&pTokenSessionImpl->accessTokenMutex);

	if (pTokenSessionImpl->tokenInformation.accessToken.data != 0)
	{
		/* Specify RDM_LOGIN_USER_AUTHN_TOKEN for handling session management when the login request is specified */
		if (pReactorChannelRole->ommConsumerRole.pLoginRequest)
		{
			pReactorChannelRole->ommConsumerRole.pLoginRequest->userName = pTokenSessionImpl->tokenInformation.accessToken;
			pReactorChannelRole->ommConsumerRole.pLoginRequest->flags |= (RDM_LG_RQF_HAS_USERNAME_TYPE);
			pReactorChannelRole->ommConsumerRole.pLoginRequest->flags &= (~RDM_LG_RQF_HAS_PASSWORD);
			pReactorChannelRole->ommConsumerRole.pLoginRequest->userNameType = RDM_LOGIN_USER_AUTHN_TOKEN;
		}

		tokenSessionEventType = RSSL_RCIMPL_TSET_REGISTER_CHANNEL_TO_SESSION;
	}
	else
	{
		RsslUInt8 retryCount = 0;
		RsslBuffer tokenServiceURL = pReactorImpl->tokenServiceURLV1;
		RsslReactorImpl* pRsslReactorImpl = pReactorChannelImpl->pParentReactor;

		while (retryCount <= 1) /* Retry the request using the redirect URL only one time. */
		{
			pTokenSessionImpl->tokenSessionState = RSSL_RC_TOKEN_SESSION_IMPL_REQ_INIT_AUTH_TOKEN;
			pReactorConnectInfoImpl->reactorChannelInfoImplState = RSSL_RC_CHINFO_IMPL_ST_REQ_AUTH_TOKEN;

			pRestRequestArgs = _reactorCreateTokenRequestV1(pReactorChannelImpl->pParentReactor, &tokenServiceURL,
				&pReactorOAuthCredential->userName, &pReactorOAuthCredential->password, NULL, &pReactorOAuthCredential->clientId,
				&pReactorOAuthCredential->clientSecret, &pReactorOAuthCredential->tokenScope, pReactorOAuthCredential->takeExclusiveSignOnControl,
				&pTokenSessionImpl->rsslPostDataBodyBuf, pTokenSessionImpl, pError);

			if (pRestRequestArgs)
			{
				_assignConnectionArgsToRequestArgs(pConnOptions, pRestRequestArgs);

				rsslRestRequestDump(pRsslReactorImpl, pRestRequestArgs, &errorInfo.rsslError);

				rsslRet = rsslRestClientBlockingRequest(pReactorChannelImpl->pParentReactor->pRestClient, pRestRequestArgs, &restResponse, &pTokenSessionImpl->rsslAccessTokenRespBuffer,
					&errorInfo.rsslError);

				rsslRestResponseDump(pRsslReactorImpl, &restResponse, &errorInfo.rsslError);

				free(pRestRequestArgs);

				if (restResponse.isMemReallocated)
				{
					free(pTokenSessionImpl->rsslAccessTokenRespBuffer.data);
					pTokenSessionImpl->rsslAccessTokenRespBuffer = restResponse.reallocatedMem;
				}
			}
			else
			{
				RTR_ATOMIC_SET(pTokenSessionImpl->requestingAccessToken, 0);
				RSSL_MUTEX_UNLOCK(&pTokenSessionImpl->accessTokenMutex);
				return RSSL_RET_FAILURE;
			}

			if (rsslRet == RSSL_RET_SUCCESS)
			{
				RsslUInt32 checkStatusCode = 0; /* Special code for catching by the default case */

				pTokenSessionImpl->httpStausCode = restResponse.statusCode;

				/* Updates the actual status code for the first retry attempt only */
				if (retryCount <= 1)
				{
					checkStatusCode = restResponse.statusCode;
				}

				switch (checkStatusCode)
				{
				case 200: /* OK */
				{
					if (rsslRestParseAccessTokenV1(&restResponse.dataBody, &pTokenSessionImpl->tokenInformation.accessToken, &pTokenSessionImpl->tokenInformation.refreshToken,
						&pTokenSessionImpl->tokenInformation.expiresIn, &pTokenSessionImpl->tokenInformation.tokenType,
						&pTokenSessionImpl->tokenInformation.scope, &pTokenSessionImpl->tokenInformationBuffer, &pError->rsslError) != RSSL_RET_SUCCESS)
					{
						rsslSetErrorInfo(pError, RSSL_EIC_FAILURE, RSSL_RET_FAILURE, __FILE__, __LINE__,
							"Failed to request authentication token information. Text: %s", restResponse.dataBody.data);

						RTR_ATOMIC_SET(pTokenSessionImpl->requestingAccessToken, 0);
						RSSL_MUTEX_UNLOCK(&pTokenSessionImpl->accessTokenMutex);
						return RSSL_RET_FAILURE;
					}

					/* Set original expires in when sending request using password grant type*/
					pTokenSessionImpl->originalExpiresIn = pTokenSessionImpl->tokenInformation.expiresIn;

					break;
				}
				case 301: /* Moved Permanently */
				case 308: /* Permanent Redirect */
				{
					RsslBuffer* uri = getHeaderValue(&restResponse.headers, &rssl_rest_location_header_text);

					if (uri != NULL)
					{
						/* Copy the new URL to the rsslReactorImpl */
						if (uri->length > pReactorImpl->tokenServiceURLBufferV1.length)
						{
							free(pReactorImpl->tokenServiceURLBufferV1.data);
							pReactorImpl->tokenServiceURLBufferV1.length = uri->length + 1;
							pReactorImpl->tokenServiceURLBufferV1.data = (char*)malloc(pReactorImpl->tokenServiceURLBufferV1.length);
							if (pReactorImpl->tokenServiceURLBufferV1.data == 0)
							{
								rsslSetErrorInfo(pError, RSSL_EIC_FAILURE, RSSL_RET_FAILURE, __FILE__, __LINE__,
									"Failed to copy the redirect URL for the token service from HTTP error %u. Text: %s", restResponse.statusCode, restResponse.dataBody.data);

								RTR_ATOMIC_SET(pTokenSessionImpl->requestingAccessToken, 0);
								RSSL_MUTEX_UNLOCK(&pTokenSessionImpl->accessTokenMutex);
								return RSSL_RET_FAILURE;
							}
						}

						memset(pReactorImpl->tokenServiceURLBufferV1.data, 0, pReactorImpl->tokenServiceURLBufferV1.length);
						pReactorImpl->tokenServiceURLV1.data = pReactorImpl->tokenServiceURLBufferV1.data;
						pReactorImpl->tokenServiceURLV1.length = uri->length - 1;
						memcpy(pReactorImpl->tokenServiceURLV1.data, uri->data + 1, pReactorImpl->tokenServiceURLV1.length);

						tokenServiceURL = pReactorImpl->tokenServiceURLV1; /* Using the redirect URL */

						++retryCount;
						continue;
					}

					rsslSetErrorInfo(pError, RSSL_EIC_FAILURE, RSSL_RET_FAILURE, __FILE__, __LINE__,
						"The redirect URL does not exist in the Location header for the token service from HTTP error %u. Text: %s", restResponse.statusCode, restResponse.dataBody.data);

					RTR_ATOMIC_SET(pTokenSessionImpl->requestingAccessToken, 0);
					RSSL_MUTEX_UNLOCK(&pTokenSessionImpl->accessTokenMutex);
					return RSSL_RET_FAILURE;
				}
				case 302: /* Found (Previously "Moved temporarily") */
				case 307: /* Temporary Redirect (Request method is not allowed to be changed) */
				{
					RsslBuffer* tempUri = getHeaderValue(&restResponse.headers, &rssl_rest_location_header_text);

					if (tempUri)
					{
						/* Using the redirect URL */
						if (tempUri->length > pTokenSessionImpl->temporaryURLBuffer.length)
						{
							free(pTokenSessionImpl->temporaryURLBuffer.data);
							pTokenSessionImpl->temporaryURLBuffer.length = tempUri->length + 1;
							pTokenSessionImpl->temporaryURLBuffer.data = (char*)malloc(pTokenSessionImpl->temporaryURLBuffer.length);
							if (pTokenSessionImpl->temporaryURLBuffer.data == 0)
							{
								rsslSetErrorInfo(pError, RSSL_EIC_FAILURE, RSSL_RET_FAILURE, __FILE__, __LINE__,
									"Failed to copy the temporary redirect URL for the token service from HTTP error %u. Text: %s", restResponse.statusCode, restResponse.dataBody.data);

								RTR_ATOMIC_SET(pTokenSessionImpl->requestingAccessToken, 0);
								RSSL_MUTEX_UNLOCK(&pTokenSessionImpl->accessTokenMutex);
								return RSSL_RET_FAILURE;
							}
						}

						memset(pTokenSessionImpl->temporaryURLBuffer.data, 0, pTokenSessionImpl->temporaryURLBuffer.length);
						pTokenSessionImpl->temporaryURL.data = pTokenSessionImpl->temporaryURLBuffer.data;
						pTokenSessionImpl->temporaryURL.length = tempUri->length - 1;
						memcpy(pTokenSessionImpl->temporaryURL.data, tempUri->data + 1, pTokenSessionImpl->temporaryURL.length);

						tokenServiceURL = pTokenSessionImpl->temporaryURL;

						++retryCount;
						continue;
					}

					rsslSetErrorInfo(pError, RSSL_EIC_FAILURE, RSSL_RET_FAILURE, __FILE__, __LINE__,
						"The redirect URL does not exist in the Location header for the token service from HTTP error %u. Text: %s", restResponse.statusCode, restResponse.dataBody.data);

					RTR_ATOMIC_SET(pTokenSessionImpl->requestingAccessToken, 0);
					RSSL_MUTEX_UNLOCK(&pTokenSessionImpl->accessTokenMutex);
					return RSSL_RET_FAILURE;
				}
				default:
				{
					rsslSetErrorInfo(pError, RSSL_EIC_FAILURE, RSSL_RET_FAILURE, __FILE__, __LINE__,
						"Failed to request authentication token information with HTTP error %u. Text: %s", restResponse.statusCode, restResponse.dataBody.data);

					RTR_ATOMIC_SET(pTokenSessionImpl->requestingAccessToken, 0);
					RSSL_MUTEX_UNLOCK(&pTokenSessionImpl->accessTokenMutex);
					return RSSL_RET_FAILURE;
				}
				}

				/* Clears the sensitive information when the callback is specified */
				if (pReactorOAuthCredential->pOAuthCredentialEventCallback)
				{
					memset(pReactorOAuthCredential->clientSecret.data, 0, pReactorOAuthCredential->clientSecret.length);
					memset(pReactorOAuthCredential->password.data, 0, pReactorOAuthCredential->password.length);
				}

				break; /* Breaks from the retry loop */
			}
			else
			{
				rsslSetErrorInfo(pError, RSSL_EIC_FAILURE, RSSL_RET_FAILURE, __FILE__, __LINE__,
					"Failed to request authentication token information. Text: %s", errorInfo.rsslError.text);

				RTR_ATOMIC_SET(pTokenSessionImpl->requestingAccessToken, 0);
				RSSL_MUTEX_UNLOCK(&pTokenSessionImpl->accessTokenMutex);
				return RSSL_RET_FAILURE;
			}
		}

		pTokenSessionImpl->tokenSessionState = RSSL_RC_TOKEN_SESSION_IMPL_RECEIVED_AUTH_TOKEN;
		pReactorConnectInfoImpl->reactorChannelInfoImplState = RSSL_RC_CHINFO_IMPL_ST_RECEIVED_AUTH_TOKEN;
		pTokenSessionImpl->lastTokenUpdatedTime = getCurrentTimeMs(pReactorImpl->ticksPerMsec);
		pReactorConnectInfoImpl->lastTokenUpdatedTime = pTokenSessionImpl->lastTokenUpdatedTime;

		/* Specify RDM_LOGIN_USER_AUTHN_TOKEN for handling session management when the login request is specified */
		if (pReactorChannelRole->ommConsumerRole.pLoginRequest)
		{
			pReactorChannelRole->ommConsumerRole.pLoginRequest->userName = pTokenSessionImpl->tokenInformation.accessToken;
			pReactorChannelRole->ommConsumerRole.pLoginRequest->flags |= (RDM_LG_RQF_HAS_USERNAME_TYPE);
			pReactorChannelRole->ommConsumerRole.pLoginRequest->flags &= (~RDM_LG_RQF_HAS_PASSWORD);
			pReactorChannelRole->ommConsumerRole.pLoginRequest->userNameType = RDM_LOGIN_USER_AUTHN_TOKEN;
		}

		/* Signal worker to add the token session and register the channel to the session management */
		if (pTokenSessionImpl->sessionLink.next == NULL && pTokenSessionImpl->sessionLink.prev == 0)
		{
			RsslInt tokenExpiresTime = getCurrentTimeMs(pReactorImpl->ticksPerMsec) + (pTokenSessionImpl->tokenInformation.expiresIn * 1000);
			tokenSessionEventType = (RSSL_RCIMPL_TSET_ADD_TOKEN_SESSION_TO_LIST | RSSL_RCIMPL_TSET_REGISTER_CHANNEL_TO_SESSION);
			pTokenSessionImpl->nextExpiresTime = getCurrentTimeMs(pReactorImpl->ticksPerMsec) + (RsslInt)(((double)pTokenSessionImpl->tokenInformation.expiresIn * pReactorImpl->tokenReissueRatio) * 1000);
			RTR_ATOMIC_SET64(pTokenSessionImpl->tokenExpiresTime, tokenExpiresTime);
			RTR_ATOMIC_SET(pTokenSessionImpl->sendTokenRequest, 1);
		}
		else
		{
			tokenSessionEventType = RSSL_RCIMPL_TSET_REGISTER_CHANNEL_TO_SESSION;
		}
	}

	RTR_ATOMIC_SET(pTokenSessionImpl->requestingAccessToken, 0);

	RSSL_MUTEX_UNLOCK(&pTokenSessionImpl->accessTokenMutex);

	/* Ensure that there is no failure before notifying the worker thread with the event */
	if (pError->rsslErrorInfoCode = pError->rsslError.rsslErrorId == RSSL_RET_SUCCESS)
	{
		pEvent = (RsslReactorTokenSessionEvent*)rsslReactorEventQueueGetFromPool(&pReactorImpl->reactorWorker.workerQueue);
		rsslClearReactorTokenSessionEvent(pEvent);

		pEvent->reactorTokenSessionEventType = tokenSessionEventType;
		pEvent->pReactorChannel = (RsslReactorChannel*)pReactorChannelImpl;
		pEvent->pTokenSessionImpl = pTokenSessionImpl;

		if (!RSSL_ERROR_INFO_CHECK(rsslReactorEventQueuePut(&pReactorImpl->reactorWorker.workerQueue, (RsslReactorEventImpl*)pEvent) == RSSL_RET_SUCCESS, RSSL_RET_FAILURE, pError))
		{
			_reactorShutdown(pReactorImpl, pError);
			_reactorSendShutdownEvent(pReactorImpl, pError);
			return RSSL_RET_FAILURE;
		}

		RTR_ATOMIC_INCREMENT(pTokenSessionImpl->numberOfWaitingChannels);
	}

	pError->rsslErrorInfoCode = pError->rsslError.rsslErrorId == RSSL_RET_SUCCESS ? RSSL_EIC_SUCCESS : RSSL_EIC_FAILURE;

	return pError->rsslError.rsslErrorId;
}

RsslRet _reactorGetAccessTokenAndServiceDiscovery(RsslReactorChannelImpl* pReactorChannelImpl, RsslBool *queryConnectInfo, RsslErrorInfo* pError)
{
	RsslRestRequestArgs *pRestRequestArgs = 0;
	RsslBuffer argumentsAndHeaders;
	RsslRestResponse restResponse;
	RsslRet rsslRet;
	RsslBuffer host;
	RsslBuffer port;
	RsslQueueLink *pLink = 0;
	RsslRestHeader *pRestHeader = 0;
	RsslReactorConnectInfoImpl* pReactorConnectInfoImpl = &pReactorChannelImpl->connectionOptList[pReactorChannelImpl->connectionListIter];
	RsslReactorChannelRole* pReactorChannelRole = &pReactorChannelImpl->channelRole;
	RsslConnectOptions *pConnOptions = &pReactorConnectInfoImpl->base.rsslConnectOptions;
	RsslErrorInfo errorInfo;
	RsslReactorOAuthCredential *pReactorOAuthCredential;
	RsslReactorTokenSessionImpl *pTokenSessionImpl = pReactorChannelImpl->pTokenSessionImpl;
	RsslReactorImpl *pReactorImpl = pReactorChannelImpl->pParentReactor;
	RsslReactorTokenSessionEvent *pEvent = NULL;
	RsslReactorTokenSessionEventType tokenSessionEventType = RSSL_RCIMPL_TSET_INIT;

	pError->rsslError.rsslErrorId = RSSL_RET_SUCCESS; /* Always clears the error ID as it is used to check the result of this function. */

	pReactorOAuthCredential = pTokenSessionImpl->pOAuthCredential;

	if (_reactorHandlesWarmStandby(pReactorChannelImpl))
	{
		RsslUInt32 index = 0;
		RsslReactorWarmStandbyGroupImpl* pWarmStandbyGroupImpl = &pReactorChannelImpl->pWarmStandByHandlerImpl->warmStandbyGroupList[0];

		pReactorConnectInfoImpl = &pWarmStandbyGroupImpl->startingActiveServer.reactorConnectInfoImpl;
		
		pConnOptions = &pWarmStandbyGroupImpl->startingActiveServer.reactorConnectInfoImpl.base.rsslConnectOptions;

		(*queryConnectInfo) = RSSL_FALSE;

		if ((!pConnOptions->connectionInfo.unified.address || !(*pConnOptions->connectionInfo.unified.address)) &&
			(!pConnOptions->connectionInfo.unified.serviceName || !(*pConnOptions->connectionInfo.unified.serviceName)))
		{
			(*queryConnectInfo) = RSSL_TRUE;
		}
		else
		{
			/* Checks from standby server */
			for (index = 0; index < pWarmStandbyGroupImpl->standbyServerCount; index++)
			{
				if (pWarmStandbyGroupImpl->standbyServerList[index].reactorConnectInfoImpl.base.enableSessionManagement)
				{
					pConnOptions = &pWarmStandbyGroupImpl->standbyServerList[index].reactorConnectInfoImpl.base.rsslConnectOptions;

					if ((!pConnOptions->connectionInfo.unified.address || !(*pConnOptions->connectionInfo.unified.address)) &&
						(!pConnOptions->connectionInfo.unified.serviceName || !(*pConnOptions->connectionInfo.unified.serviceName)))
					{
						(*queryConnectInfo) = RSSL_TRUE;
						break;
					}
				}
			}
		}
	}
	else
	{
		if ((!pConnOptions->connectionInfo.unified.address || !(*pConnOptions->connectionInfo.unified.address)) &&
			(!pConnOptions->connectionInfo.unified.serviceName || !(*pConnOptions->connectionInfo.unified.serviceName)))
		{
			(*queryConnectInfo) = RSSL_TRUE;
		}
		else
		{
			(*queryConnectInfo) = RSSL_FALSE;
		}
	}

	/* Assign proxy information if any from the first connection for the token session */
	if (pTokenSessionImpl->copiedProxyOpts == RSSL_FALSE)
	{
		if (rsslDeepCopyConnectOpts(&pTokenSessionImpl->proxyConnectOpts, &pReactorConnectInfoImpl->base.rsslConnectOptions) != RSSL_RET_SUCCESS)
		{
			rsslSetErrorInfo(pError, RSSL_EIC_FAILURE, RSSL_RET_FAILURE, __FILE__, __LINE__,
				"Failed to copy proxy configuration parameters for the token session.");
			return RSSL_RET_FAILURE;
		}

		pTokenSessionImpl->copiedProxyOpts = RSSL_TRUE;
	}

	RTR_ATOMIC_SET(pTokenSessionImpl->requestingAccessToken, 1);
	RSSL_MUTEX_LOCK(&pTokenSessionImpl->accessTokenMutex);

	if (pTokenSessionImpl->tokenInformation.accessToken.data != 0)
	{
		/* Specify RDM_LOGIN_USER_AUTHN_TOKEN for handling session management when the login request is specified */
		if (pReactorChannelRole->ommConsumerRole.pLoginRequest)
		{
			pReactorChannelRole->ommConsumerRole.pLoginRequest->userName = pTokenSessionImpl->tokenInformation.accessToken;
			pReactorChannelRole->ommConsumerRole.pLoginRequest->flags |= (RDM_LG_RQF_HAS_USERNAME_TYPE);
			pReactorChannelRole->ommConsumerRole.pLoginRequest->flags &= (~RDM_LG_RQF_HAS_PASSWORD);
			pReactorChannelRole->ommConsumerRole.pLoginRequest->userNameType = RDM_LOGIN_USER_AUTHN_TOKEN;
		}

		tokenSessionEventType = RSSL_RCIMPL_TSET_REGISTER_CHANNEL_TO_SESSION;
	}
	else
	{
		RsslUInt8 retryCount = 0;
		RsslBuffer tokenServiceURL = pTokenSessionImpl->sessionAuthUrl;
		RsslReactorImpl* pRsslReactorImpl = pReactorChannelImpl->pParentReactor;

		while (retryCount <= 1) /* Retry the request using the redirect URL only one time. */
		{
			pTokenSessionImpl->tokenSessionState = RSSL_RC_TOKEN_SESSION_IMPL_REQ_INIT_AUTH_TOKEN;
			pReactorConnectInfoImpl->reactorChannelInfoImplState = RSSL_RC_CHINFO_IMPL_ST_REQ_AUTH_TOKEN;

			if (pReactorChannelImpl->sessionVersion == RSSL_RC_SESSMGMT_V1)
			{
				pRestRequestArgs = _reactorCreateTokenRequestV1(pReactorChannelImpl->pParentReactor, &tokenServiceURL,
					&pReactorOAuthCredential->userName, &pReactorOAuthCredential->password, NULL, &pReactorOAuthCredential->clientId,
					&pReactorOAuthCredential->clientSecret, &pReactorOAuthCredential->tokenScope, pReactorOAuthCredential->takeExclusiveSignOnControl,
					&pTokenSessionImpl->rsslPostDataBodyBuf, pTokenSessionImpl, pError);
			}
			else
			{
				pRestRequestArgs = _reactorCreateTokenRequestV2(pReactorChannelImpl->pParentReactor, &tokenServiceURL, &pReactorOAuthCredential->clientId,
					&pReactorOAuthCredential->clientSecret, &pReactorOAuthCredential->tokenScope, &pTokenSessionImpl->rsslPostDataBodyBuf, pTokenSessionImpl, pError);
			}

			if (pRestRequestArgs)
			{
				_assignConnectionArgsToRequestArgs(pConnOptions, pRestRequestArgs);

				rsslRestRequestDump(pRsslReactorImpl, pRestRequestArgs, &errorInfo.rsslError);

				rsslRet = rsslRestClientBlockingRequest(pReactorChannelImpl->pParentReactor->pRestClient, pRestRequestArgs, &restResponse, &pTokenSessionImpl->rsslAccessTokenRespBuffer,
					&errorInfo.rsslError);

				rsslRestResponseDump(pRsslReactorImpl, &restResponse, &errorInfo.rsslError);

				free(pRestRequestArgs);

				if (restResponse.isMemReallocated)
				{
					free(pTokenSessionImpl->rsslAccessTokenRespBuffer.data);
					pTokenSessionImpl->rsslAccessTokenRespBuffer = restResponse.reallocatedMem;
				}
			}
			else
			{
				RTR_ATOMIC_SET(pTokenSessionImpl->requestingAccessToken, 0);
				RSSL_MUTEX_UNLOCK(&pTokenSessionImpl->accessTokenMutex);
				return RSSL_RET_FAILURE;
			}

			if (rsslRet == RSSL_RET_SUCCESS)
			{
				RsslUInt32 checkStatusCode = 0; /* Special code for catching by the default case */

				pTokenSessionImpl->httpStausCode = restResponse.statusCode;

				/* Updates the actual status code for the first retry attempt only */
				if (retryCount <= 1)
				{
					checkStatusCode = restResponse.statusCode;
				}

				switch (checkStatusCode)
				{
					case 200: /* OK */
					{
						if (pTokenSessionImpl->tokenInformationBuffer.length < restResponse.dataBody.length)
						{
							if (pTokenSessionImpl->tokenInformationBuffer.data)
							{
								free(pTokenSessionImpl->tokenInformationBuffer.data);
							}

							pTokenSessionImpl->tokenInformationBuffer.length = restResponse.dataBody.length;
							pTokenSessionImpl->tokenInformationBuffer.data = (char*)malloc(pTokenSessionImpl->tokenInformationBuffer.length);

							if (pTokenSessionImpl->tokenInformationBuffer.data == 0)
							{
								rsslSetErrorInfo(pError, RSSL_EIC_FAILURE, RSSL_RET_FAILURE, __FILE__, __LINE__,
									"Failed to allocate token information buffer. Text: %s", restResponse.dataBody.data);

								RTR_ATOMIC_SET(pTokenSessionImpl->requestingAccessToken, 0);
								RSSL_MUTEX_UNLOCK(&pTokenSessionImpl->accessTokenMutex);
								return RSSL_RET_FAILURE;
							}
						}

						if (pReactorChannelImpl->sessionVersion == RSSL_RC_SESSMGMT_V1)
						{
							if (rsslRestParseAccessTokenV1(&restResponse.dataBody, &pTokenSessionImpl->tokenInformation.accessToken, &pTokenSessionImpl->tokenInformation.refreshToken,
								&pTokenSessionImpl->tokenInformation.expiresIn, &pTokenSessionImpl->tokenInformation.tokenType,
								&pTokenSessionImpl->tokenInformation.scope, &pTokenSessionImpl->tokenInformationBuffer, &pError->rsslError) != RSSL_RET_SUCCESS)
							{
								rsslSetErrorInfo(pError, RSSL_EIC_FAILURE, RSSL_RET_FAILURE, __FILE__, __LINE__,
									"Failed to request authentication token information. Text: %s", restResponse.dataBody.data);

								RTR_ATOMIC_SET(pTokenSessionImpl->requestingAccessToken, 0);
								RSSL_MUTEX_UNLOCK(&pTokenSessionImpl->accessTokenMutex);
								return RSSL_RET_FAILURE;
							}
							
							/* Set original expires in when sending request using password grant type*/
							pTokenSessionImpl->originalExpiresIn = pTokenSessionImpl->tokenInformation.expiresIn;
						}
						else 
						{
							if (rsslRestParseAccessTokenV2(&restResponse.dataBody, &pTokenSessionImpl->tokenInformation.accessToken, &pTokenSessionImpl->tokenInformation.refreshToken,
								&pTokenSessionImpl->tokenInformation.expiresIn, &pTokenSessionImpl->tokenInformation.tokenType,
								&pTokenSessionImpl->tokenInformation.scope, &pTokenSessionImpl->tokenInformationBuffer, &pError->rsslError) != RSSL_RET_SUCCESS)
							{
								rsslSetErrorInfo(pError, RSSL_EIC_FAILURE, RSSL_RET_FAILURE, __FILE__, __LINE__,
									"Failed to request version 2 authentication token information. Text: %s", restResponse.dataBody.data);

								RTR_ATOMIC_SET(pTokenSessionImpl->requestingAccessToken, 0);
								RSSL_MUTEX_UNLOCK(&pTokenSessionImpl->accessTokenMutex);
								return RSSL_RET_FAILURE;
							}

							/* We should not need to consider this lifetime, because we will get a new token from the token generator on connection recovery */
							pTokenSessionImpl->originalExpiresIn = pTokenSessionImpl->tokenInformation.expiresIn;
						}

						break;
					}
					case 301: /* Moved Permanently */
					case 308: /* Permanent Redirect */
					{
						RsslBuffer* uri = getHeaderValue(&restResponse.headers, &rssl_rest_location_header_text);

						if (uri != NULL)
						{
							/* Copy the new URL to the rsslReactorImpl */
							if (uri->length > pTokenSessionImpl->sessionAuthUrl.length)
							{
								free(pTokenSessionImpl->sessionAuthUrl.data);
								pTokenSessionImpl->sessionAuthUrl.length = uri->length + 1;
								pTokenSessionImpl->sessionAuthUrl.data = (char*)malloc(pTokenSessionImpl->sessionAuthUrl.length);
								if (pTokenSessionImpl->sessionAuthUrl.data == 0)
								{
									rsslSetErrorInfo(pError, RSSL_EIC_FAILURE, RSSL_RET_FAILURE, __FILE__, __LINE__,
										"Failed to copy the redirect URL for the token service from HTTP error %u. Text: %s", restResponse.statusCode, restResponse.dataBody.data);

									RTR_ATOMIC_SET(pTokenSessionImpl->requestingAccessToken, 0);
									RSSL_MUTEX_UNLOCK(&pTokenSessionImpl->accessTokenMutex);
									return RSSL_RET_FAILURE;
								}
							}

							memset(pTokenSessionImpl->sessionAuthUrl.data, 0, pTokenSessionImpl->sessionAuthUrl.length);
							pTokenSessionImpl->sessionAuthUrl.length = uri->length - 1;
							memcpy(pTokenSessionImpl->sessionAuthUrl.data, uri->data + 1, pTokenSessionImpl->sessionAuthUrl.length);

							tokenServiceURL = pTokenSessionImpl->sessionAuthUrl; /* Using the redirect URL */

							++retryCount;
							continue;
						}

						rsslSetErrorInfo(pError, RSSL_EIC_FAILURE, RSSL_RET_FAILURE, __FILE__, __LINE__,
							"The redirect URL does not exist in the Location header for the token service from HTTP error %u. Text: %s", restResponse.statusCode, restResponse.dataBody.data);

						RTR_ATOMIC_SET(pTokenSessionImpl->requestingAccessToken, 0);
						RSSL_MUTEX_UNLOCK(&pTokenSessionImpl->accessTokenMutex);
						return RSSL_RET_FAILURE;
					}
					case 302: /* Found (Previously "Moved temporarily") */
					case 307: /* Temporary Redirect (Request method is not allowed to be changed) */
					{
						RsslBuffer* tempUri = getHeaderValue(&restResponse.headers, &rssl_rest_location_header_text);

						if (tempUri)
						{
							/* Using the redirect URL */
							if (tempUri->length > pTokenSessionImpl->temporaryURLBuffer.length)
							{
								free(pTokenSessionImpl->temporaryURLBuffer.data);
								pTokenSessionImpl->temporaryURLBuffer.length = tempUri->length + 1;
								pTokenSessionImpl->temporaryURLBuffer.data = (char*)malloc(pTokenSessionImpl->temporaryURLBuffer.length);
								if (pTokenSessionImpl->temporaryURLBuffer.data == 0)
								{
									rsslSetErrorInfo(pError, RSSL_EIC_FAILURE, RSSL_RET_FAILURE, __FILE__, __LINE__,
										"Failed to copy the temporary redirect URL for the token service from HTTP error %u. Text: %s", restResponse.statusCode, restResponse.dataBody.data);

									RTR_ATOMIC_SET(pTokenSessionImpl->requestingAccessToken, 0);
									RSSL_MUTEX_UNLOCK(&pTokenSessionImpl->accessTokenMutex);
									return RSSL_RET_FAILURE;
								}
							}

							memset(pTokenSessionImpl->temporaryURLBuffer.data, 0, pTokenSessionImpl->temporaryURLBuffer.length);
							pTokenSessionImpl->temporaryURL.data = pTokenSessionImpl->temporaryURLBuffer.data;
							pTokenSessionImpl->temporaryURL.length = tempUri->length - 1;
							memcpy(pTokenSessionImpl->temporaryURL.data, tempUri->data + 1, pTokenSessionImpl->temporaryURL.length);

							tokenServiceURL = pTokenSessionImpl->temporaryURL;

							++retryCount;
							continue;
						}

						rsslSetErrorInfo(pError, RSSL_EIC_FAILURE, RSSL_RET_FAILURE, __FILE__, __LINE__,
							"The redirect URL does not exist in the Location header for the token service from HTTP error %u. Text: %s", restResponse.statusCode, restResponse.dataBody.data);

						RTR_ATOMIC_SET(pTokenSessionImpl->requestingAccessToken, 0);
						RSSL_MUTEX_UNLOCK(&pTokenSessionImpl->accessTokenMutex);
						return RSSL_RET_FAILURE;
					}
					default:
					{
						rsslSetErrorInfo(pError, RSSL_EIC_FAILURE, RSSL_RET_FAILURE, __FILE__, __LINE__,
							"Failed to request authentication token information with HTTP error %u. Text: %s", restResponse.statusCode, restResponse.dataBody.data);

						RTR_ATOMIC_SET(pTokenSessionImpl->requestingAccessToken, 0);
						RSSL_MUTEX_UNLOCK(&pTokenSessionImpl->accessTokenMutex);
						return RSSL_RET_FAILURE;
					}
				}

				/* Clears the sensitive information when the callback is specified */
				if (pReactorOAuthCredential->pOAuthCredentialEventCallback)
				{
					memset(pReactorOAuthCredential->clientSecret.data, 0, pReactorOAuthCredential->clientSecret.length);
					memset(pReactorOAuthCredential->password.data, 0, pReactorOAuthCredential->password.length);
				}

				break; /* Breaks from the retry loop */
			}
			else
			{
				rsslSetErrorInfo(pError, RSSL_EIC_FAILURE, RSSL_RET_FAILURE, __FILE__, __LINE__,
					"Failed to request authentication token information. Text: %s", errorInfo.rsslError.text);

				RTR_ATOMIC_SET(pTokenSessionImpl->requestingAccessToken, 0);
				RSSL_MUTEX_UNLOCK(&pTokenSessionImpl->accessTokenMutex);
				return RSSL_RET_FAILURE;
			}
		}

		pTokenSessionImpl->tokenSessionState = RSSL_RC_TOKEN_SESSION_IMPL_RECEIVED_AUTH_TOKEN;
		pReactorConnectInfoImpl->reactorChannelInfoImplState = RSSL_RC_CHINFO_IMPL_ST_RECEIVED_AUTH_TOKEN;
		pTokenSessionImpl->lastTokenUpdatedTime = getCurrentTimeMs(pReactorImpl->ticksPerMsec);
		pReactorConnectInfoImpl->lastTokenUpdatedTime = pTokenSessionImpl->lastTokenUpdatedTime;

		/* Specify RDM_LOGIN_USER_AUTHN_TOKEN for handling session management when the login request is specified */
		if (pReactorChannelRole->ommConsumerRole.pLoginRequest)
		{
			pReactorChannelRole->ommConsumerRole.pLoginRequest->userName = pTokenSessionImpl->tokenInformation.accessToken;
			pReactorChannelRole->ommConsumerRole.pLoginRequest->flags |= (RDM_LG_RQF_HAS_USERNAME_TYPE);
			pReactorChannelRole->ommConsumerRole.pLoginRequest->flags &= (~RDM_LG_RQF_HAS_PASSWORD);
			pReactorChannelRole->ommConsumerRole.pLoginRequest->userNameType = RDM_LOGIN_USER_AUTHN_TOKEN;
		}

		/* Signal worker to add the token session and register the channel to the session management */
		if (pReactorChannelImpl->sessionVersion == RSSL_RC_SESSMGMT_V1 && pTokenSessionImpl->sessionLink.next == NULL && pTokenSessionImpl->sessionLink.prev == 0)
		{
			RsslInt tokenExpiresTime = getCurrentTimeMs(pReactorImpl->ticksPerMsec) + (pTokenSessionImpl->tokenInformation.expiresIn * 1000);
			tokenSessionEventType = (RSSL_RCIMPL_TSET_ADD_TOKEN_SESSION_TO_LIST | RSSL_RCIMPL_TSET_REGISTER_CHANNEL_TO_SESSION);
			pTokenSessionImpl->nextExpiresTime = getCurrentTimeMs(pReactorImpl->ticksPerMsec) + (RsslInt)(((double)pTokenSessionImpl->tokenInformation.expiresIn  * pReactorImpl->tokenReissueRatio) * 1000);
			RTR_ATOMIC_SET64(pTokenSessionImpl->tokenExpiresTime, tokenExpiresTime);
			RTR_ATOMIC_SET(pTokenSessionImpl->sendTokenRequest, 1);
		}
		else if (pReactorChannelImpl->sessionVersion == RSSL_RC_SESSMGMT_V2)
		{
			RsslInt expiresTime = (pTokenSessionImpl->tokenInformation.expiresIn > 600) ? pTokenSessionImpl->tokenInformation.expiresIn - 300 : (RsslInt)(.95 * pTokenSessionImpl->tokenInformation.expiresIn);
			RsslInt tokenExpiresTime = getCurrentTimeMs(pReactorImpl->ticksPerMsec) + (expiresTime * 1000);
			tokenSessionEventType = (RSSL_RCIMPL_TSET_ADD_TOKEN_SESSION_TO_LIST | RSSL_RCIMPL_TSET_REGISTER_CHANNEL_TO_SESSION);
			pTokenSessionImpl->nextExpiresTime = getCurrentTimeMs(pReactorImpl->ticksPerMsec) + (expiresTime * 1000);
			RTR_ATOMIC_SET64(pTokenSessionImpl->tokenExpiresTime, tokenExpiresTime);
			RTR_ATOMIC_SET(pTokenSessionImpl->sendTokenRequest, 1);
		}
		else
		{
			tokenSessionEventType = RSSL_RCIMPL_TSET_REGISTER_CHANNEL_TO_SESSION;
		}
	}

	RTR_ATOMIC_SET(pTokenSessionImpl->requestingAccessToken, 0);

	if (*queryConnectInfo)
	{
		RsslUInt8 retryCount = 0;
		RsslBuffer serviceDiscoveryURL = pReactorImpl->serviceDiscoveryURL;
		RsslReactorDiscoveryTransportProtocol transport = RSSL_RD_TP_INIT;
		RsslReactorImpl* pRsslReactorImpl = pReactorChannelImpl->pParentReactor;

		pReactorConnectInfoImpl->reactorChannelInfoImplState = RSSL_RC_CHINFO_IMPL_ST_QUERYING_SERVICE_DISOVERY;

		rsslClearBuffer(&argumentsAndHeaders);

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
				rsslSetErrorInfo(pError, RSSL_EIC_FAILURE, RSSL_RET_INVALID_ARGUMENT, __FILE__, __LINE__,
					"Invalid encrypted protocol type(%d) for requesting RDP service discovery.",
					pReactorConnectInfoImpl->base.rsslConnectOptions.encryptionOpts.encryptedProtocol);

				RSSL_MUTEX_UNLOCK(&pTokenSessionImpl->accessTokenMutex);
				return RSSL_RET_INVALID_ARGUMENT;
			}

			break;
		}
		default:
			rsslSetErrorInfo(pError, RSSL_EIC_FAILURE, RSSL_RET_INVALID_ARGUMENT, __FILE__, __LINE__,
				"Invalid connection type(%d) for requesting RDP service discovery.",
				transport);

			RSSL_MUTEX_UNLOCK(&pTokenSessionImpl->accessTokenMutex);
			return RSSL_RET_INVALID_ARGUMENT;
		}

		while (retryCount <= 1) /* Retry the request using the redirect URL only one time. */
		{
			/* Get host name and port for RDP service discovery */
			if ((pRestRequestArgs = _reactorCreateRequestArgsForServiceDiscovery(pReactorChannelImpl->pParentReactor, &serviceDiscoveryURL,
				transport, RSSL_RD_DP_INIT, &pTokenSessionImpl->tokenInformation.tokenType,
				&pTokenSessionImpl->tokenInformation.accessToken, &argumentsAndHeaders,
				pReactorChannelImpl, pError)) == 0)
			{
				if (pError->rsslError.rsslErrorId == RSSL_RET_INVALID_ARGUMENT)
				{
					RSSL_MUTEX_UNLOCK(&pTokenSessionImpl->accessTokenMutex);
					return RSSL_RET_INVALID_ARGUMENT;
				}

				goto memoryAllocationFailed;
			}

			if (pTokenSessionImpl->rsslServiceDiscoveryRespBuffer.data == 0)
			{
				pTokenSessionImpl->rsslServiceDiscoveryRespBuffer.length = RSSL_REST_INIT_SVC_DIS_BUF_SIZE;
				pTokenSessionImpl->rsslServiceDiscoveryRespBuffer.data = (char*)malloc(pTokenSessionImpl->rsslServiceDiscoveryRespBuffer.length);

				if (pTokenSessionImpl->rsslServiceDiscoveryRespBuffer.data == 0)
				{
					goto memoryAllocationFailed;
				}
			}

			_assignConnectionArgsToRequestArgs(pConnOptions, pRestRequestArgs);

			rsslRestRequestDump(pRsslReactorImpl, pRestRequestArgs, &errorInfo.rsslError);

			rsslRet = rsslRestClientBlockingRequest(pReactorChannelImpl->pParentReactor->pRestClient, pRestRequestArgs, &restResponse,
				&pTokenSessionImpl->rsslServiceDiscoveryRespBuffer, &errorInfo.rsslError);

			rsslRestResponseDump(pRsslReactorImpl, &restResponse, &errorInfo.rsslError);

			if (restResponse.isMemReallocated)
			{
				free(pTokenSessionImpl->rsslServiceDiscoveryRespBuffer.data);
				pTokenSessionImpl->rsslServiceDiscoveryRespBuffer = restResponse.reallocatedMem;
			}

			free(pRestRequestArgs);

			if (rsslRet == RSSL_RET_SUCCESS)
			{
				RsslUInt32 checkStatusCode = 0; /* Special code for catching by the default case */

				pTokenSessionImpl->httpStausCode = restResponse.statusCode;

				/* Updates the actual status code for the first retry attempt only */
				if (retryCount <= 1)
				{
					checkStatusCode = restResponse.statusCode;
				}

				switch (checkStatusCode)
				{
					case 200: /* OK */
					{
						RsslBuffer parseBuffer;
						char *pMemoryAlloction = (char*)malloc(RSSL_REST_STORE_HOST_AND_PORT_BUF_SIZE);
						if (pMemoryAlloction == 0)
						{
							goto memoryAllocationFailed;
						}

						memset(pMemoryAlloction, 0, RSSL_REST_STORE_HOST_AND_PORT_BUF_SIZE);

						parseBuffer.length = RSSL_REST_STORE_HOST_AND_PORT_BUF_SIZE;
						parseBuffer.data = pMemoryAlloction;

						if (_reactorHandlesWarmStandby(pReactorChannelImpl) == RSSL_FALSE)
						{
							rsslRet = rsslRestParseEndpoint(&restResponse.dataBody, &pReactorConnectInfoImpl->base.location, &host, &port, &parseBuffer, &pError->rsslError);

							if (rsslRet == RSSL_RET_SUCCESS)
							{
								if (_reactorApplyServiceDiscoveryEndpoint(pConnOptions, &host, &port) != RSSL_RET_SUCCESS)
								{
									free(parseBuffer.data);
									goto memoryAllocationFailed;
								}

								pReactorConnectInfoImpl->reactorChannelInfoImplState = RSSL_RC_CHINFO_IMPL_ST_ASSIGNED_HOST_PORT;

								(*queryConnectInfo) = RSSL_FALSE;
							}
							else
							{
								rsslSetErrorInfo(pError, RSSL_EIC_FAILURE, RSSL_RET_FAILURE, __FILE__, __LINE__,
									"Failed to query host name and port from the RDP service discovery for the \"%s\" location", pReactorConnectInfoImpl->base.location.data);
							}
						}
						else
						{
							RsslUInt32 index = 0;
							RsslReactorWarmStandbyGroupImpl* pWarmStandbyGroupImpl = &pReactorChannelImpl->pWarmStandByHandlerImpl->warmStandbyGroupList[0];

							pConnOptions = &pWarmStandbyGroupImpl->startingActiveServer.reactorConnectInfoImpl.base.rsslConnectOptions;

							rsslRet = rsslRestParseEndpoint(&restResponse.dataBody, &pReactorConnectInfoImpl->base.location, &host, &port, &parseBuffer, &pError->rsslError);

							if (rsslRet == RSSL_RET_SUCCESS)
							{
								if (_reactorApplyServiceDiscoveryEndpoint(pConnOptions, &host, &port) != RSSL_RET_SUCCESS)
								{
									free(parseBuffer.data);
									goto memoryAllocationFailed;
								}

								pReactorConnectInfoImpl->reactorChannelInfoImplState = RSSL_RC_CHINFO_IMPL_ST_ASSIGNED_HOST_PORT;

								(*queryConnectInfo) = RSSL_FALSE;

								for (index = 0; index < pWarmStandbyGroupImpl->standbyServerCount; index++)
								{
									RsslReactorConnectInfoImpl* pReactorStandbyConnectInfoImpl = &pWarmStandbyGroupImpl->standbyServerList[index].reactorConnectInfoImpl;

									if (pReactorStandbyConnectInfoImpl->base.enableSessionManagement)
									{
										RsslConnectOptions* pStandbyConnOptions = &pReactorStandbyConnectInfoImpl->base.rsslConnectOptions;

										if ((!pStandbyConnOptions->connectionInfo.unified.address || !(*pStandbyConnOptions->connectionInfo.unified.address)) &&
											(!pStandbyConnOptions->connectionInfo.unified.serviceName || !(*pStandbyConnOptions->connectionInfo.unified.serviceName)))
										{
											parseBuffer.length = RSSL_REST_STORE_HOST_AND_PORT_BUF_SIZE;
											parseBuffer.data = pMemoryAlloction;

											rsslRet = rsslRestParseEndpoint(&restResponse.dataBody, &pReactorStandbyConnectInfoImpl->base.location, &host, &port, &parseBuffer, &pError->rsslError);

											if (rsslRet == RSSL_RET_SUCCESS)
											{
												if (_reactorApplyServiceDiscoveryEndpoint(pStandbyConnOptions, &host, &port) != RSSL_RET_SUCCESS)
												{
													free(parseBuffer.data);
													goto memoryAllocationFailed;
												}

												pReactorStandbyConnectInfoImpl->reactorChannelInfoImplState = RSSL_RC_CHINFO_IMPL_ST_ASSIGNED_HOST_PORT;
											}
											else
											{
												rsslSetErrorInfo(pError, RSSL_EIC_FAILURE, RSSL_RET_FAILURE, __FILE__, __LINE__,
													"Failed to query host name and port from the RDP service discovery for the \"%s\" location", pReactorStandbyConnectInfoImpl->base.location.data);
												
												break; /* break from this for loop. */
											}
										}
									}
								}
							}
							else
							{
								rsslSetErrorInfo(pError, RSSL_EIC_FAILURE, RSSL_RET_FAILURE, __FILE__, __LINE__,
									"Failed to query host name and port from the RDP service discovery for the \"%s\" location", pReactorConnectInfoImpl->base.location.data);
							}
						}

						free(parseBuffer.data);

						break;
					}
					case 301: /* Moved Permanently */
					case 308: /* Permanent Redirect */
					{
						RsslBuffer* uri = getHeaderValue(&restResponse.headers, &rssl_rest_location_header_text);

						if (uri != NULL)
						{
							/* Copy the new URL to the rsslReactorImpl */
							if (uri->length > pReactorImpl->serviceDiscoveryURLBuffer.length)
							{
								free(pReactorImpl->serviceDiscoveryURLBuffer.data);
								pReactorImpl->serviceDiscoveryURLBuffer.length = uri->length + 1;
								pReactorImpl->serviceDiscoveryURLBuffer.data = (char*)malloc(pReactorImpl->serviceDiscoveryURLBuffer.length);
								if (pReactorImpl->serviceDiscoveryURLBuffer.data == 0)
								{
									rsslSetErrorInfo(pError, RSSL_EIC_FAILURE, RSSL_RET_FAILURE, __FILE__, __LINE__,
										"Failed to copy the redirect URL for the RDP service discovery from HTTP error %u. Text: %s", restResponse.statusCode, restResponse.dataBody.data);
									break;
								}
							}

							memset(pReactorImpl->serviceDiscoveryURLBuffer.data, 0, pReactorImpl->serviceDiscoveryURLBuffer.length);
							pReactorImpl->serviceDiscoveryURL.data = pReactorImpl->serviceDiscoveryURLBuffer.data;
							pReactorImpl->serviceDiscoveryURL.length = uri->length - 1;
							memcpy(pReactorImpl->serviceDiscoveryURL.data, uri->data + 1, pReactorImpl->serviceDiscoveryURL.length);

							serviceDiscoveryURL = pReactorImpl->serviceDiscoveryURL; /* Using the redirect URL */

							++retryCount;
							continue;
						}

						rsslSetErrorInfo(pError, RSSL_EIC_FAILURE, RSSL_RET_FAILURE, __FILE__, __LINE__,
							"The redirect URL does not exist in the Location header for the RDP service discovery from HTTP error %u. Text: %s", restResponse.statusCode, restResponse.dataBody.data);

						break;
					}
					case 302: /* Found (Previously "Moved temporarily") */
					case 303: /* See Other(Another URI using the GET method) */
					case 307: /* Temporary Redirect (Request method is not allowed to be changed) */
					{
						RsslBuffer* tempUri = getHeaderValue(&restResponse.headers, &rssl_rest_location_header_text);

						if (tempUri)
						{
							/* Using the redirect URL */
							if (tempUri->length > pTokenSessionImpl->temporaryURLBuffer.length)
							{
								free(pTokenSessionImpl->temporaryURLBuffer.data);
								pTokenSessionImpl->temporaryURLBuffer.length = tempUri->length + 1;
								pTokenSessionImpl->temporaryURLBuffer.data = (char*)malloc(pTokenSessionImpl->temporaryURLBuffer.length);
								if (pTokenSessionImpl->temporaryURLBuffer.data == 0)
								{
									rsslSetErrorInfo(pError, RSSL_EIC_FAILURE, RSSL_RET_FAILURE, __FILE__, __LINE__,
										"Failed to copy the temporary redirect URL for the RDP service discovery from HTTP error %u. Text: %s", restResponse.statusCode, restResponse.dataBody.data);
									break;
								}
							}

							memset(pTokenSessionImpl->temporaryURLBuffer.data, 0, pTokenSessionImpl->temporaryURLBuffer.length);
							pTokenSessionImpl->temporaryURL.data = pTokenSessionImpl->temporaryURLBuffer.data;
							pTokenSessionImpl->temporaryURL.length = tempUri->length - 1;
							memcpy(pTokenSessionImpl->temporaryURL.data, tempUri->data + 1, pTokenSessionImpl->temporaryURL.length);

							serviceDiscoveryURL = pTokenSessionImpl->temporaryURL;

							++retryCount;
							continue;
						}

						rsslSetErrorInfo(pError, RSSL_EIC_FAILURE, RSSL_RET_FAILURE, __FILE__, __LINE__,
							"The redirect URL does not exist in the Location header for the RDP service discovery from HTTP error %u. Text: %s", restResponse.statusCode, restResponse.dataBody.data);

						break;
					}
					default:
					{
						rsslSetErrorInfo(pError, RSSL_EIC_FAILURE, RSSL_RET_FAILURE, __FILE__, __LINE__,
							"Failed to query host name and port from the RDP service discovery from HTTP error code %u. Text: %s", restResponse.statusCode, restResponse.dataBody.data);
					}
				}
			}
			else
			{
				rsslSetErrorInfo(pError, RSSL_EIC_FAILURE, RSSL_RET_FAILURE, __FILE__, __LINE__,
					"Failed to request service discovery information. Text: %s", errorInfo.rsslError.text);
			}

			free(argumentsAndHeaders.data);
			rsslClearBuffer(&argumentsAndHeaders);
			break;
		}
	}

	RSSL_MUTEX_UNLOCK(&pTokenSessionImpl->accessTokenMutex);

	/* Ensure that there is no failure before notifying the worker thread with the event */
	if (pError->rsslErrorInfoCode = pError->rsslError.rsslErrorId == RSSL_RET_SUCCESS)
	{
		pEvent = (RsslReactorTokenSessionEvent*)rsslReactorEventQueueGetFromPool(&pReactorImpl->reactorWorker.workerQueue);
		rsslClearReactorTokenSessionEvent(pEvent);

		pEvent->reactorTokenSessionEventType = tokenSessionEventType;
		pEvent->pReactorChannel = (RsslReactorChannel*)pReactorChannelImpl;
		pEvent->pTokenSessionImpl = pTokenSessionImpl;

		if (!RSSL_ERROR_INFO_CHECK(rsslReactorEventQueuePut(&pReactorImpl->reactorWorker.workerQueue, (RsslReactorEventImpl*)pEvent) == RSSL_RET_SUCCESS, RSSL_RET_FAILURE, pError))
		{
			_reactorShutdown(pReactorImpl, pError);
			_reactorSendShutdownEvent(pReactorImpl, pError);
			return RSSL_RET_FAILURE;
		}

		RTR_ATOMIC_INCREMENT(pTokenSessionImpl->numberOfWaitingChannels);
	}

	pError->rsslErrorInfoCode = pError->rsslError.rsslErrorId == RSSL_RET_SUCCESS ? RSSL_EIC_SUCCESS : RSSL_EIC_FAILURE;

	return pError->rsslError.rsslErrorId;

memoryAllocationFailed:
	
	RSSL_MUTEX_UNLOCK(&pTokenSessionImpl->accessTokenMutex);

	if (argumentsAndHeaders.data) free(argumentsAndHeaders.data);
		
	rsslSetErrorInfo(pError, RSSL_EIC_FAILURE, RSSL_RET_FAILURE, __FILE__, __LINE__,
		"Failed to allocate memory for requesting data from REST services.");

	return RSSL_RET_FAILURE;
}

void _assignConnectionArgsToRequestArgs(RsslConnectOptions *pConnOptions, RsslRestRequestArgs* pRestRequestArgs)
{
	if (pConnOptions->proxyOpts.proxyDomain && *pConnOptions->proxyOpts.proxyDomain != '\0')
	{
		pRestRequestArgs->networkArgs.proxyArgs.proxyDomain.data = pConnOptions->proxyOpts.proxyDomain;
		pRestRequestArgs->networkArgs.proxyArgs.proxyDomain.length = (RsslUInt32)strlen(pConnOptions->proxyOpts.proxyDomain);
	}

	if (pConnOptions->proxyOpts.proxyHostName && *pConnOptions->proxyOpts.proxyHostName != '\0')
	{
		pRestRequestArgs->networkArgs.proxyArgs.proxyHostName.data = pConnOptions->proxyOpts.proxyHostName;
		pRestRequestArgs->networkArgs.proxyArgs.proxyHostName.length = (RsslUInt32)strlen(pConnOptions->proxyOpts.proxyHostName);
	}

	if (pConnOptions->proxyOpts.proxyPasswd && *pConnOptions->proxyOpts.proxyPasswd != '\0')
	{
		pRestRequestArgs->networkArgs.proxyArgs.proxyPassword.data = pConnOptions->proxyOpts.proxyPasswd;
		pRestRequestArgs->networkArgs.proxyArgs.proxyPassword.length = (RsslUInt32)strlen(pConnOptions->proxyOpts.proxyPasswd);
	}

	if (pConnOptions->proxyOpts.proxyPort && *pConnOptions->proxyOpts.proxyPort != '\0')
	{
		pRestRequestArgs->networkArgs.proxyArgs.proxyPort.data = pConnOptions->proxyOpts.proxyPort;
		pRestRequestArgs->networkArgs.proxyArgs.proxyPort.length = (RsslUInt32)strlen(pConnOptions->proxyOpts.proxyPort);
	}

	if (pConnOptions->proxyOpts.proxyUserName && *pConnOptions->proxyOpts.proxyUserName != '\0')
	{
		pRestRequestArgs->networkArgs.proxyArgs.proxyUserName.data = pConnOptions->proxyOpts.proxyUserName;
		pRestRequestArgs->networkArgs.proxyArgs.proxyUserName.length = (RsslUInt32)strlen(pConnOptions->proxyOpts.proxyUserName);
	}

	if (pConnOptions->connectionInfo.unified.interfaceName && *pConnOptions->connectionInfo.unified.interfaceName != '\0')
	{
		pRestRequestArgs->networkArgs.interfaceName.data = pConnOptions->connectionInfo.unified.interfaceName;
		pRestRequestArgs->networkArgs.interfaceName.length = (RsslUInt32)strlen(pConnOptions->connectionInfo.unified.interfaceName);
	}
}

void _cumulativeValue(RsslUInt* destination, RsslUInt32 value)
{
	if ( (*destination + value) > UINT64_MAX )
	{
		*destination = value;
	}
	else
	{
		*destination += value;
	}
}

void _populateRsslReactorAuthTokenInfo(RsslBuffer* pDestBuffer, RsslReactorAuthTokenInfo* pDestTokeninfo, RsslReactorAuthTokenInfo* pSrcTokeninfo)
{
	char *pCurPos = NULL;
	pCurPos = pDestBuffer->data;
	pDestTokeninfo->accessToken.data = pCurPos;
	pDestTokeninfo->accessToken.length = pSrcTokeninfo->accessToken.length;

	pCurPos += (pDestTokeninfo->accessToken.length + 1);
	pDestTokeninfo->refreshToken.data = pCurPos;
	pDestTokeninfo->refreshToken.length = pSrcTokeninfo->refreshToken.length;

	pCurPos += (pDestTokeninfo->refreshToken.length + 1);
	pDestTokeninfo->scope.data = pCurPos;
	pDestTokeninfo->scope.length = pSrcTokeninfo->scope.length;

	pCurPos += (pDestTokeninfo->scope.length + 1);
	pDestTokeninfo->tokenType.data = pCurPos;
	pDestTokeninfo->tokenType.length = pSrcTokeninfo->tokenType.length;

	pDestTokeninfo->expiresIn = pSrcTokeninfo->expiresIn;
}

RsslBool compareOAuthCredentialForTokenSession(RsslReactorOAuthCredential* pOAuthCredential, RsslBuffer *pClientID, RsslBuffer *pPassword, RsslReactorOAuthCredential* pOAuthOther, RsslErrorInfo* pErrorInfo)
{
	RsslReactorOAuthCredential rsslReactorOAuthCredential;
	RsslBuffer otherTokenScope = RSSL_INIT_BUFFER;

	rsslClearReactorOAuthCredential(&rsslReactorOAuthCredential);
	otherTokenScope = rsslReactorOAuthCredential.tokenScope;

	if (pOAuthOther && pOAuthOther->tokenScope.data)
	{
		otherTokenScope = pOAuthOther->tokenScope;
	}

	if (pOAuthCredential->pOAuthCredentialEventCallback)
	{
		if ((pOAuthOther == 0) || (pOAuthCredential->pOAuthCredentialEventCallback != pOAuthOther->pOAuthCredentialEventCallback))
		{
			rsslSetErrorInfo(pErrorInfo, RSSL_EIC_FAILURE, RSSL_RET_INVALID_ARGUMENT, __FILE__, __LINE__,
				"The RsslReactorOAuthCredentialEventCallback of RsslReactorOAuthCredential is not equal for the same token session.");
			return RSSL_FALSE;
		}
	}
	else
	{
		if (pOAuthOther && pOAuthOther->pOAuthCredentialEventCallback)
		{
			rsslSetErrorInfo(pErrorInfo, RSSL_EIC_FAILURE, RSSL_RET_INVALID_ARGUMENT, __FILE__, __LINE__,
				"The RsslReactorOAuthCredentialEventCallback of RsslReactorOAuthCredential is not equal for the same token session.");
			return RSSL_FALSE;
		}
	}

	/* Compares the sensitive information when the callback is not specified */
	if (pOAuthCredential->pOAuthCredentialEventCallback == 0)
	{
		RsslBuffer otherClientSecret = RSSL_INIT_BUFFER;

		if (pOAuthOther && pOAuthOther->clientSecret.data)
		{
			otherClientSecret = pOAuthOther->clientSecret;
		}

		if ((pOAuthCredential->clientSecret.length != otherClientSecret.length) ||
			(memcmp(pOAuthCredential->clientSecret.data, otherClientSecret.data, pOAuthCredential->clientSecret.length) != 0))
		{
			rsslSetErrorInfo(pErrorInfo, RSSL_EIC_FAILURE, RSSL_RET_INVALID_ARGUMENT, __FILE__, __LINE__,
				"The Client secret of RsslReactorOAuthCredential is not equal for the same token session.");
			return RSSL_FALSE;
		}

		if ((pOAuthCredential->password.length != pPassword->length) ||
			(memcmp(pOAuthCredential->password.data, pPassword->data, pOAuthCredential->password.length) != 0))
		{
			rsslSetErrorInfo(pErrorInfo, RSSL_EIC_FAILURE, RSSL_RET_INVALID_ARGUMENT, __FILE__, __LINE__,
				"The password of RsslReactorOAuthCredential is not equal for the same token session.");
			return RSSL_FALSE;
		}
	}

	if ((pOAuthCredential->clientId.length != pClientID->length) ||
		(memcmp(pOAuthCredential->clientId.data, pClientID->data, pOAuthCredential->clientId.length) != 0))
	{
		rsslSetErrorInfo(pErrorInfo, RSSL_EIC_FAILURE, RSSL_RET_INVALID_ARGUMENT, __FILE__, __LINE__,
			"The Client ID of RsslReactorOAuthCredential is not equal for the same token session.");
		return RSSL_FALSE;
	}

	if ((pOAuthCredential->tokenScope.length != otherTokenScope.length) ||
		(memcmp(pOAuthCredential->tokenScope.data, otherTokenScope.data, pOAuthCredential->tokenScope.length) != 0))
	{
		rsslSetErrorInfo(pErrorInfo, RSSL_EIC_FAILURE, RSSL_RET_INVALID_ARGUMENT, __FILE__, __LINE__,
			"The token scope of RsslReactorOAuthCredential is not equal for the same token session.");
		return RSSL_FALSE;
	}

	if (pOAuthOther && (pOAuthCredential->takeExclusiveSignOnControl != pOAuthOther->takeExclusiveSignOnControl) )
	{
		rsslSetErrorInfo(pErrorInfo, RSSL_EIC_FAILURE, RSSL_RET_INVALID_ARGUMENT, __FILE__, __LINE__,
			"The takeExclusiveSignOnControl of RsslReactorOAuthCredential is not equal for the same token session.");
		return RSSL_FALSE;
	}

	return RSSL_TRUE;
}

RsslBuffer* getHeaderValue(RsslQueue *pHeaders, RsslBuffer* pHeaderName)
{
	RsslQueueLink *pLink;
	RsslRestHeader *pRestHeader;

	while ((pLink = rsslQueueRemoveFirstLink(pHeaders)) != NULL)
	{
		pRestHeader = RSSL_QUEUE_LINK_TO_OBJECT(RsslRestHeader, queueLink, pLink);

		if (RTR_STRNICMP(pHeaderName->data, pRestHeader->key.data, pHeaderName->length) == 0)
		{
			return &pRestHeader->value;
		}
	}

	return NULL;
}

RsslReactorErrorInfoImpl *rsslReactorGetErrorInfoFromPool(RsslReactorWorker *pReactorWoker)
{
	RsslReactorErrorInfoImpl *pReactorErrorInfoImpl;
	RsslQueueLink *pLink;

	RSSL_MUTEX_LOCK(&pReactorWoker->errorInfoPoolLock);
	if ((pLink = rsslQueueRemoveFirstLink(&pReactorWoker->errorInfoPool)))
		pReactorErrorInfoImpl = RSSL_QUEUE_LINK_TO_OBJECT(RsslReactorErrorInfoImpl, poolLink, pLink);
	else
	{
		pReactorErrorInfoImpl = (RsslReactorErrorInfoImpl*)malloc(sizeof(RsslReactorErrorInfoImpl));
	}

	if (pReactorErrorInfoImpl)
	{
		rsslClearReactorErrorInfoImpl(pReactorErrorInfoImpl);

		rsslInitQueueLink(&pReactorErrorInfoImpl->poolLink);

		/* Adds to the in used pool to ensure that the memory is released properly. */
		rsslQueueAddLinkToBack(&pReactorWoker->errorInfoInUsedPool, &pReactorErrorInfoImpl->poolLink);
	}

	RSSL_MUTEX_UNLOCK(&pReactorWoker->errorInfoPoolLock);

	return pReactorErrorInfoImpl;
}

void rsslReactorReturnErrorInfoToPool(RsslReactorErrorInfoImpl *pReactorErrorInfoImpl, RsslReactorWorker *pReactorWoker)
{
	RSSL_MUTEX_LOCK(&pReactorWoker->errorInfoPoolLock);

	/* Removes from the in used pool. */
	rsslQueueRemoveLink(&pReactorWoker->errorInfoInUsedPool, &pReactorErrorInfoImpl->poolLink);

	/* Keeps the RsslReactorErrorInfoImpl in the pool until reaches the maximum pool size to reduce memory consumption in the long run. */
	if ( pReactorWoker->errorInfoPool.count < RSSL_REACTOR_WORKER_ERROR_INFO_MAX_POOL_SIZE)
	{
		rsslQueueAddLinkToBack(&pReactorWoker->errorInfoPool, &pReactorErrorInfoImpl->poolLink);
	}
	else
	{
		free(pReactorErrorInfoImpl);
	}

	RSSL_MUTEX_UNLOCK(&pReactorWoker->errorInfoPoolLock);
}

static RsslBool validateLoginResponseFromStandbyServer(RsslRDMLoginRefresh* pActiveServerLoginRefersh, RsslRDMLoginRefresh* pStandbyServerLoginRefersh)
{
	if (pActiveServerLoginRefersh->flags & RDM_LG_RFF_HAS_APPLICATION_ID)
	{
		if ((pStandbyServerLoginRefersh->flags & RDM_LG_RFF_HAS_APPLICATION_ID))
		{
			if (pActiveServerLoginRefersh->applicationId.length != pStandbyServerLoginRefersh->applicationId.length)
				return RSSL_FALSE;

			if (memcmp(pActiveServerLoginRefersh->applicationId.data, pStandbyServerLoginRefersh->applicationId.data, pActiveServerLoginRefersh->applicationId.length) != 0)
				return RSSL_FALSE;
		}
		else
			return RSSL_FALSE;
	}
	else if ((pStandbyServerLoginRefersh->flags & RDM_LG_RFF_HAS_APPLICATION_ID))
	{
		return RSSL_FALSE;
	}

	if (pActiveServerLoginRefersh->flags & RDM_LG_RFF_HAS_POSITION)
	{
		if ((pStandbyServerLoginRefersh->flags & RDM_LG_RFF_HAS_POSITION))
		{
			if (pActiveServerLoginRefersh->position.length != pStandbyServerLoginRefersh->position.length)
				return RSSL_FALSE;

			if (memcmp(pActiveServerLoginRefersh->position.data, pStandbyServerLoginRefersh->position.data, pActiveServerLoginRefersh->position.length) != 0)
				return RSSL_FALSE;
		}
		else
			return RSSL_FALSE;
	}
	else if ((pStandbyServerLoginRefersh->flags & RDM_LG_RFF_HAS_POSITION))
	{
		return RSSL_FALSE;
	}

	if (pActiveServerLoginRefersh->flags & RDM_LG_RFF_HAS_PROVIDE_PERM_PROFILE)
	{
		if (pStandbyServerLoginRefersh->flags & RDM_LG_RFF_HAS_PROVIDE_PERM_PROFILE)
		{
			if (pActiveServerLoginRefersh->providePermissionProfile != pStandbyServerLoginRefersh->providePermissionProfile)
				return RSSL_FALSE;
		}
		else
			return RSSL_FALSE;
	}
	else if (pStandbyServerLoginRefersh->flags & RDM_LG_RFF_HAS_PROVIDE_PERM_PROFILE)
	{
		return RSSL_FALSE;
	}
	
	if (pActiveServerLoginRefersh->flags & RDM_LG_RFF_HAS_PROVIDE_PERM_EXPR)
	{
		if (pStandbyServerLoginRefersh->flags & RDM_LG_RFF_HAS_PROVIDE_PERM_EXPR)
		{
			if (pActiveServerLoginRefersh->providePermissionExpressions != pStandbyServerLoginRefersh->providePermissionExpressions)
				return RSSL_FALSE;
		}
		else
			return RSSL_FALSE;
	}
	else if (pStandbyServerLoginRefersh->flags & RDM_LG_RFF_HAS_PROVIDE_PERM_EXPR)
	{
		return RSSL_FALSE;
	}
	
	if (pActiveServerLoginRefersh->flags & RDM_LG_RFF_HAS_SINGLE_OPEN)
	{
		if (pStandbyServerLoginRefersh->flags & RDM_LG_RFF_HAS_SINGLE_OPEN)
		{
			if (pActiveServerLoginRefersh->singleOpen != pStandbyServerLoginRefersh->singleOpen)
				return RSSL_FALSE;
		}
		else
			return RSSL_FALSE;
	}
	else if (pStandbyServerLoginRefersh->flags & RDM_LG_RFF_HAS_SINGLE_OPEN)
	{
		return RSSL_FALSE;
	}
	
	if (pActiveServerLoginRefersh->flags & RDM_LG_RFF_HAS_ALLOW_SUSPECT_DATA)
	{
		if (pStandbyServerLoginRefersh->flags & RDM_LG_RFF_HAS_ALLOW_SUSPECT_DATA)
		{
			if (pActiveServerLoginRefersh->allowSuspectData != pStandbyServerLoginRefersh->allowSuspectData)
				return RSSL_FALSE;
		}
		else
			return RSSL_FALSE;
	}
	else if (pStandbyServerLoginRefersh->flags & RDM_LG_RFF_HAS_ALLOW_SUSPECT_DATA)
	{
		return RSSL_FALSE;
	}
	
	if (pActiveServerLoginRefersh->flags & RDM_LG_RFF_HAS_SUPPORT_BATCH)
	{
		if (pStandbyServerLoginRefersh->flags & RDM_LG_RFF_HAS_SUPPORT_BATCH)
		{
			if (pActiveServerLoginRefersh->supportBatchRequests != pStandbyServerLoginRefersh->supportBatchRequests)
				return RSSL_FALSE;
		}
		else
			return RSSL_FALSE;
	}
	else if (pStandbyServerLoginRefersh->flags & RDM_LG_RFF_HAS_SUPPORT_BATCH)
	{
		return RSSL_FALSE;
	}
	
	if (pActiveServerLoginRefersh->flags & RDM_LG_RFF_HAS_SUPPORT_POST)
	{
		if (pStandbyServerLoginRefersh->flags & RDM_LG_RFF_HAS_SUPPORT_POST)
		{
			if (pActiveServerLoginRefersh->supportOMMPost != pStandbyServerLoginRefersh->supportOMMPost)
				return RSSL_FALSE;
		}
		else
			return RSSL_FALSE;
	}
	else if (pStandbyServerLoginRefersh->flags & RDM_LG_RFF_HAS_SUPPORT_POST)
	{
		return RSSL_FALSE;
	}
	
	if (pActiveServerLoginRefersh->flags & RDM_LG_RFF_HAS_SUPPORT_VIEW)
	{
		if (pStandbyServerLoginRefersh->flags & RDM_LG_RFF_HAS_SUPPORT_VIEW)
		{
			if (pActiveServerLoginRefersh->supportViewRequests != pStandbyServerLoginRefersh->supportViewRequests)
				return RSSL_FALSE;
		}
		else
			return RSSL_FALSE;
	}
	else if (pStandbyServerLoginRefersh->flags & RDM_LG_RFF_HAS_SUPPORT_VIEW)
	{
		return RSSL_FALSE;
	}

	if (pActiveServerLoginRefersh->flags & RDM_LG_RFF_HAS_SUPPORT_STANDBY_MODE)
	{
		if (pStandbyServerLoginRefersh->flags & RDM_LG_RFF_HAS_SUPPORT_STANDBY_MODE)
		{
			if ( (pActiveServerLoginRefersh->supportStandbyMode & pStandbyServerLoginRefersh->supportStandbyMode) == 0 )
				return RSSL_FALSE;
		}
	}

	return RSSL_TRUE;
}

static RsslRet copyReactorSubmitMsgOption(RsslReactorImpl *pReactorImpl, RsslReactorSubmitMsgOptionsImpl* pDestinationImpl, RsslReactorSubmitMsgOptions* pSource)
{
	RsslReactorSubmitMsgOptions* pDestination = (RsslReactorSubmitMsgOptions*)pDestinationImpl;
	RsslBuffer memoryBuffer;

	if (!pSource || !pDestination)
	{
		return RSSL_RET_FAILURE;
	}

	rsslClearReactorSubmitMsgOptionsImpl(pDestinationImpl);

	pDestination->majorVersion = pSource->majorVersion;
	pDestination->minorVersion = pSource->minorVersion;
	pDestination->requestMsgOptions.pUserSpec = pSource->requestMsgOptions.pUserSpec;

	/* Checks to ensure that service name is avaliable. */
	if (pSource->pServiceName != NULL && pSource->pServiceName->length && pSource->pServiceName->data && *pSource->pServiceName->data)
	{
		pDestination->pServiceName = (RsslBuffer*)malloc(sizeof(RsslBuffer));

		if (!pDestination->pServiceName)
		{
			return RSSL_RET_FAILURE;
		}

		rsslClearBuffer(pDestination->pServiceName);

		pDestination->pServiceName->data = malloc((size_t)pSource->pServiceName->length + 1);
	
		if (pDestination->pServiceName->data)
		{
			pDestination->pServiceName->data[pSource->pServiceName->length] = '\0';
			memcpy(pDestination->pServiceName->data, pSource->pServiceName->data, pSource->pServiceName->length);
			pDestination->pServiceName->length = pSource->pServiceName->length;
		}
		else
		{
			free(pDestination->pServiceName);
			return RSSL_RET_FAILURE;
		}
	}

	if (pSource->pRsslMsg)
	{
		/* Returns NULL if failed to allocate memory. */
		pDestination->pRsslMsg = rsslCopyMsg(pSource->pRsslMsg, RSSL_CMF_ALL_FLAGS, 0, NULL);

		if (!pDestination->pRsslMsg)
		{
			free(pDestination->pServiceName->data);
			free(pDestination->pServiceName);
			return RSSL_RET_FAILURE;
		}

		pDestinationImpl->copyFromMsgCopy = RSSL_TRUE;
	}
	else if (pSource->pRDMMsg)
	{
		RsslRet ret;
		pDestination->pRDMMsg = (RsslRDMMsg*)malloc(sizeof(RsslRDMMsg));

		if (pDestination->pRDMMsg == NULL)
		{
			free(pDestination->pServiceName->data);
			free(pDestination->pServiceName);
			return RSSL_RET_FAILURE;
		}

		pDestinationImpl->allocationLength = 512;
		pDestinationImpl->pMemoryLocation = (char*)malloc(pDestinationImpl->allocationLength);

		if (pDestinationImpl->pMemoryLocation == NULL)
		{
			return RSSL_RET_FAILURE;
		}

		memset(pDestinationImpl->pMemoryLocation, 0, pDestinationImpl->allocationLength);

		memoryBuffer.data = pDestinationImpl->pMemoryLocation;
		memoryBuffer.length = pDestinationImpl->allocationLength;

		ret = rsslCopyRDMMsg(pDestination->pRDMMsg, pSource->pRDMMsg, &memoryBuffer);

		while (ret == RSSL_RET_BUFFER_TOO_SMALL)
		{
			free(pDestinationImpl->pMemoryLocation);
			pDestinationImpl->allocationLength *= 2;
			pDestinationImpl->pMemoryLocation = (char*)malloc(pDestinationImpl->allocationLength);

			if (pDestinationImpl->pMemoryLocation == NULL)
			{
				return RSSL_RET_FAILURE;
			}

			memset(pDestinationImpl->pMemoryLocation, 0, pDestinationImpl->allocationLength);

			memoryBuffer.data = pDestinationImpl->pMemoryLocation;
			memoryBuffer.length = pDestinationImpl->allocationLength;
			ret = rsslCopyRDMMsg(pDestination->pRDMMsg, pSource->pRDMMsg, &memoryBuffer);
		}

		if (ret != RSSL_RET_SUCCESS)
		{
			return ret;
		}
	}

	pDestinationImpl->submitTimeStamp = getCurrentTimeMs(pReactorImpl->ticksPerMsec);

	return RSSL_RET_SUCCESS;
}

RsslRet copyWlItemRequest(RsslReactorImpl *pReactorImpl, RsslReactorSubmitMsgOptionsImpl* pDestinationImpl, WlItemRequest* pSourceItemRequest)
{
	RsslReactorSubmitMsgOptions* pDestination = (RsslReactorSubmitMsgOptions*)pDestinationImpl;
	RsslRequestMsg *pRequestMsg;
	RsslRet ret;
	RsslErrorInfo errorInfo;

	pRequestMsg = (RsslRequestMsg*)malloc(sizeof(RsslRequestMsg));

	if (pRequestMsg == NULL)
	{
		return RSSL_RET_FAILURE;
	}

	rsslClearRequestMsg(pRequestMsg);

	pDestination->pRsslMsg = (RsslMsg*)pRequestMsg;
	pDestination->requestMsgOptions.pUserSpec = pSourceItemRequest->base.pUserSpec;

	pRequestMsg->msgBase.domainType = pSourceItemRequest->base.domainType;
	pRequestMsg->msgBase.streamId = pSourceItemRequest->base.streamId;
	pRequestMsg->msgBase.containerType = pSourceItemRequest->containerType;

	pDestinationImpl->allocationLength = 0;
	pDestinationImpl->pMemoryLocation = NULL;

	pRequestMsg->flags = pSourceItemRequest->requestMsgFlags;

	if ((ret = wlItemCopyKey(&pRequestMsg->msgBase.msgKey, &pSourceItemRequest->msgKey, &pDestinationImpl->pMemoryLocation, &errorInfo)) != RSSL_RET_SUCCESS)
	{
		free(pDestination->pRsslMsg);
		return ret;
	}

	if (pSourceItemRequest->pRequestedService && pSourceItemRequest->pRequestedService->flags & WL_RSVC_HAS_NAME)
	{
		pDestination->pServiceName = (RsslBuffer*)malloc(sizeof(RsslBuffer));
		if (!pDestination->pServiceName)
		{
			free(pDestination->pRsslMsg);
			free(pDestinationImpl->pMemoryLocation);
			return RSSL_RET_FAILURE;
		}

		rsslClearBuffer(pDestination->pServiceName);

		pDestination->pServiceName->data = malloc((size_t)pSourceItemRequest->pRequestedService->serviceName.length + 1);

		if (pDestination->pServiceName->data)
		{
			pDestination->pServiceName->data[pSourceItemRequest->pRequestedService->serviceName.length] = '\0';
			memcpy(pDestination->pServiceName->data, pSourceItemRequest->pRequestedService->serviceName.data, pSourceItemRequest->pRequestedService->serviceName.length);
			pDestination->pServiceName->length = pSourceItemRequest->pRequestedService->serviceName.length;
		}
		else
		{
			free(pDestination->pServiceName);
			free(pDestination->pRsslMsg);
			free(pDestinationImpl->pMemoryLocation);
			return RSSL_RET_FAILURE;
		}
	}

	if (pSourceItemRequest->requestMsgFlags & RSSL_RQMF_HAS_QOS)
		 pRequestMsg->qos = pSourceItemRequest->qos;

	if (pSourceItemRequest->requestMsgFlags & RSSL_RQMF_HAS_WORST_QOS)
		 pRequestMsg->worstQos = pSourceItemRequest->worstQos;

	if (pSourceItemRequest->requestMsgFlags & RSSL_RQMF_HAS_PRIORITY)
	{
		pRequestMsg->priorityClass = pSourceItemRequest->priorityClass;
		pRequestMsg->priorityCount = pSourceItemRequest->priorityCount;
	}

	if (pRequestMsg->flags & WL_IRQF_PRIVATE && pRequestMsg->flags & RSSL_RQMF_HAS_EXTENDED_HEADER)
	{
		pRequestMsg->extendedHeader.data = (char*)malloc(pSourceItemRequest->extendedHeader.length);

		if (pRequestMsg->extendedHeader.data)
		{
			pRequestMsg->extendedHeader.length = pSourceItemRequest->extendedHeader.length;
			memcpy(pRequestMsg->extendedHeader.data, pSourceItemRequest->extendedHeader.data,
				pSourceItemRequest->extendedHeader.length);
		}
		else
		{
			free(pDestination->pServiceName->data);
			free(pDestination->pServiceName);
			free(pDestination->pRsslMsg);
			free(pDestinationImpl->pMemoryLocation);
			return RSSL_RET_FAILURE;
		}
	}

	pDestinationImpl->copyFromMsgCopy = RSSL_FALSE;

	pDestinationImpl->submitTimeStamp = getCurrentTimeMs(pReactorImpl->ticksPerMsec);

	return RSSL_RET_SUCCESS;
}

static RsslBool isWarmStandbyChannelClosed(RsslReactorWarmStandByHandlerImpl* pWarmStandbyHandler, RsslReactorChannelImpl* pReactorChannelImpl)
{
	RsslQueueLink* pLink = NULL;
	RsslReactorChannelImpl* pReactorChannel = NULL;


	RSSL_MUTEX_LOCK(&pWarmStandbyHandler->warmStandByHandlerMutex);
	RSSL_QUEUE_FOR_EACH_LINK(&pWarmStandbyHandler->rsslChannelQueue, pLink)
	{
		pReactorChannel = RSSL_QUEUE_LINK_TO_OBJECT(RsslReactorChannelImpl, warmstandbyChannelLink, pLink);

		if (pReactorChannelImpl && pReactorChannel == pReactorChannelImpl)
		{
			if (pReactorChannelImpl->reactorChannel.pRsslChannel && pReactorChannelImpl->reactorChannel.pRsslChannel->state == RSSL_CH_STATE_ACTIVE)
			{
				continue; /* This is called from _reactorHandleChannelDown to close the passed in channel*/
			}
		}

		if (pReactorChannel->reactorChannel.pRsslChannel)
		{
			if (pReactorChannel->reactorChannel.pRsslChannel->state == RSSL_CH_STATE_ACTIVE)
			{
				if (pReactorChannel->reactorParentQueue == &pReactorChannel->pParentReactor->reconnectingChannels ||
					pReactorChannel->reactorParentQueue == &pReactorChannel->pParentReactor->inactiveChannels)
				{
					continue;
				}

				RSSL_MUTEX_UNLOCK(&pWarmStandbyHandler->warmStandByHandlerMutex);
				return RSSL_FALSE;
			}
		}
	}

	RSSL_MUTEX_UNLOCK(&pWarmStandbyHandler->warmStandByHandlerMutex);
	return RSSL_TRUE;
}

RsslBool _reactorHandlesWarmStandby(RsslReactorChannelImpl *pReactorChannelImpl)
{
	if (pReactorChannelImpl->pWarmStandByHandlerImpl)
	{
		if ((pReactorChannelImpl->pWarmStandByHandlerImpl->warmStandByHandlerState & RSSL_RWSB_STATE_MOVE_TO_CHANNEL_LIST) == 0)
		{
			return RSSL_TRUE;
		}
	}

	return RSSL_FALSE;
}

static RsslBool provideServiceAtStartUp(RsslReactorWarmStandbyGroupImpl *pWarmStandbyGroupImpl, RsslBuffer* pServiceName, RsslReactorChannelImpl *pReactorChannelImpl)
{
	if (pWarmStandbyGroupImpl->pActiveServiceConfig != NULL && pWarmStandbyGroupImpl->pActiveServiceConfig->elementCount != 0)
	{
		RsslHashLink *pHashLink = rsslHashTableFind(pWarmStandbyGroupImpl->pActiveServiceConfig, pServiceName, NULL);

		if (pHashLink != NULL)
		{
			RsslReactorWSBServiceConfigImpl *pReactorWSBServiceConfigImpl = RSSL_HASH_LINK_TO_OBJECT(RsslReactorWSBServiceConfigImpl, hashLink, pHashLink);

			if (pReactorWSBServiceConfigImpl->isStartingServerConfig != pReactorChannelImpl->isStartingServerConfig)
			{
				/* Others server select this service at startup */
				return RSSL_FALSE;
			}
			else
			{
				if (!pReactorWSBServiceConfigImpl->isStartingServerConfig &&
					pReactorWSBServiceConfigImpl->standByServerListIndex != pReactorChannelImpl->standByServerListIndex)
				{
					/* Others server select this service at startup */
					return RSSL_FALSE;
				}
			}

			/* Removes from the startup service HashTable and RsslQueue*/
			rsslHashTableRemoveLink(pWarmStandbyGroupImpl->pActiveServiceConfig, &pReactorWSBServiceConfigImpl->hashLink);
			rsslQueueRemoveLink(pWarmStandbyGroupImpl->pActiveServiceConfigList, &pReactorWSBServiceConfigImpl->queueLink);
			free(pReactorWSBServiceConfigImpl);
		}
	}

	return RSSL_TRUE;
}

static void _reactorHandlesWSBSocketList(RsslReactorWarmStandByHandlerImpl* pWarmStandbyHandler, RsslReactorChannel *pReactorChannel, RsslSocket *removeSocket)
{
	RsslQueueLink* pLink = NULL;
	RsslReactorChannelImpl* pReactorChannelImpl = NULL;
	RsslUInt32 index = 0;
	RsslReactorWarmStandbyChannelInfo warmStandbyChInfoItemp;

	warmStandbyChInfoItemp = pWarmStandbyHandler->wsbChannelInfoImpl.base;

	/* Swap current socket list to old socket list. */
	pWarmStandbyHandler->wsbChannelInfoImpl.base.oldSocketIdList = warmStandbyChInfoItemp.socketIdList;
	pWarmStandbyHandler->wsbChannelInfoImpl.base.oldSocketIdCount = warmStandbyChInfoItemp.socketIdCount;
	pWarmStandbyHandler->wsbChannelInfoImpl.base.socketIdList = warmStandbyChInfoItemp.oldSocketIdList;
	
	memset(pWarmStandbyHandler->wsbChannelInfoImpl.base.socketIdList, 0, pWarmStandbyHandler->wsbChannelInfoImpl.maxNumberOfSocket * sizeof(RsslSocket));

	RSSL_MUTEX_LOCK(&pWarmStandbyHandler->warmStandByHandlerMutex);
	RSSL_QUEUE_FOR_EACH_LINK(&pWarmStandbyHandler->rsslChannelQueue, pLink)
	{
		pReactorChannelImpl = RSSL_QUEUE_LINK_TO_OBJECT(RsslReactorChannelImpl, warmstandbyChannelLink, pLink);

		if (pReactorChannelImpl->reactorChannel.pRsslChannel)
		{
			if (pReactorChannelImpl->reactorChannel.pRsslChannel->state == RSSL_CH_STATE_ACTIVE)
			{
				if (removeSocket)
				{
					/* Remove this socket ID from the list. */
					if (*removeSocket == pReactorChannelImpl->reactorChannel.pRsslChannel->socketId)
						continue;
				}

				if (pReactorChannelImpl->reactorChannel.pRsslChannel->socketId != (RsslSocket)REACTOR_INVALID_SOCKET)
				{
					pWarmStandbyHandler->wsbChannelInfoImpl.base.socketIdList[index] = pReactorChannelImpl->reactorChannel.pRsslChannel->socketId;
					++index;
				}
			}
		}
	}
	RSSL_MUTEX_UNLOCK(&pWarmStandbyHandler->warmStandByHandlerMutex);

	pWarmStandbyHandler->wsbChannelInfoImpl.base.socketIdCount = index;
	pReactorChannel->pWarmStandbyChInfo = &pWarmStandbyHandler->wsbChannelInfoImpl.base;
}

/* Returns RSSL_TRUE if the starting server is different from the conigured list. */
static RsslBool _reactorHandleWSBServerList(RsslReactorWarmStandByHandlerImpl* pWarmStandbyHandler, RsslReactorChannelImpl *pReactorChannelImpl, RsslRDMLoginRefresh *pRDMLoginRefresh)
{
	if ((pRDMLoginRefresh->flags & RDM_LG_RQF_HAS_DOWNLOAD_CONN_CONFIG) != 0)
	{
		RsslReactorWarmStandbyGroupImpl *pWarmStandbyGroupImpl = &pWarmStandbyHandler->warmStandbyGroupList[pWarmStandbyHandler->currentWSyGroupIndex];

		if (pRDMLoginRefresh->serverCount && pRDMLoginRefresh->serverList)
		{
			RsslRDMServerInfo *pRsslRDMServerInfo = NULL;
			RsslRDMServerInfo *pActiveRDMServerInfo = NULL;
			RsslUInt32 index;
			RsslReactorWarmStandbyServerInfoImpl *pWarmStandbyServerInfoImpl = NULL;

			/* Find the first starting active server. */
			for (index = 0; index < pRDMLoginRefresh->serverCount; index++)
			{
				pRsslRDMServerInfo = &pRDMLoginRefresh->serverList[index];

				if (pRsslRDMServerInfo->flags & RDM_LG_SIF_HAS_TYPE)
				{
					if (pRsslRDMServerInfo->serverType == RDM_LOGIN_SERVER_TYPE_ACTIVE)
					{
						pActiveRDMServerInfo = pRsslRDMServerInfo;
						break;
					}
				}
			}

			/* Select the first one on the list if there is no active server. */
			if (pActiveRDMServerInfo == NULL)
			{
				pActiveRDMServerInfo = &pRDMLoginRefresh->serverList[0];
			}

			if (pWarmStandbyGroupImpl->currentStartingServerIndex == RSSL_REACTOR_WSB_STARTING_SERVER_INDEX)
			{
				pWarmStandbyServerInfoImpl = &pWarmStandbyGroupImpl->startingActiveServer;
			}
			else
			{
				pWarmStandbyServerInfoImpl = &pWarmStandbyGroupImpl->standbyServerList[pWarmStandbyGroupImpl->currentStartingServerIndex];
			}

			/* Compares the host and port to select an active server */
			if (memcmp(pActiveRDMServerInfo->hostname.data, pWarmStandbyServerInfoImpl->reactorConnectInfoImpl.base.rsslConnectOptions.connectionInfo.unified.address,
				pActiveRDMServerInfo->hostname.length) == 0)
			{
				RsslUInt portNumber = atoi(pWarmStandbyServerInfoImpl->reactorConnectInfoImpl.base.rsslConnectOptions.connectionInfo.unified.serviceName);

				if (pActiveRDMServerInfo->port == portNumber)
				{
					pWarmStandbyGroupImpl->downloadConfigActiveServer = RSSL_REACTOR_WSB_STARTING_SERVER_INDEX;
					return RSSL_FALSE;
				}
			}

			for (index = 0; index < pWarmStandbyGroupImpl->standbyServerCount; index++)
			{
				/* Find others server from the standby list to set as active server. */
				pWarmStandbyServerInfoImpl = &pWarmStandbyGroupImpl->standbyServerList[index];

				if (memcmp(pActiveRDMServerInfo->hostname.data, pWarmStandbyServerInfoImpl->reactorConnectInfoImpl.base.rsslConnectOptions.connectionInfo.unified.address,
					pActiveRDMServerInfo->hostname.length) == 0)
				{
					RsslUInt portNumber = atoi(pWarmStandbyServerInfoImpl->reactorConnectInfoImpl.base.rsslConnectOptions.connectionInfo.unified.serviceName);

					if (pActiveRDMServerInfo->port == portNumber)
					{
						pWarmStandbyGroupImpl->downloadConfigActiveServer = index;
						return RSSL_TRUE;
					}
				}
			}
		}
	}

	return RSSL_FALSE;
}

static RsslRet _submitQueuedMessages(RsslReactorWarmStandByHandlerImpl* pWarmStandbyHandler, RsslReactorWarmStandbyGroupImpl *pWarmStandbyGroupImpl,
	RsslReactorChannelImpl *pReactorChannelImpl, RsslErrorInfo *pError)
{
	if (pWarmStandbyGroupImpl->sendReqQueueCount < pWarmStandbyGroupImpl->standbyServerCount)
	{
		if ((pWarmStandbyGroupImpl->sendReqQueueCount + 1) == pWarmStandbyGroupImpl->standbyServerCount)
		{
			pWarmStandbyGroupImpl->sendQueueReqForAll = RSSL_TRUE;
			pWarmStandbyGroupImpl->sendReqQueueCount = pWarmStandbyGroupImpl->standbyServerCount;
		}
		else
		{
			++pWarmStandbyGroupImpl->sendReqQueueCount;
		}
	}

	if (pWarmStandbyHandler->submitMsgQueue.count > 0)
	{
		RsslQueueLink* pLink = NULL;
		RsslReactorSubmitMsgOptionsImpl* pSubmitMsgOptionsImpl = NULL;
		RsslWatchlistProcessMsgOptions processOpts;

		/* Submit item request after receiving the source directory. */
		RSSL_QUEUE_FOR_EACH_LINK(&pWarmStandbyHandler->submitMsgQueue, pLink)
		{
			pSubmitMsgOptionsImpl = RSSL_QUEUE_LINK_TO_OBJECT(RsslReactorSubmitMsgOptionsImpl, submitMsgLink, pLink);

			if (pSubmitMsgOptionsImpl)
			{
				/* Submits to the channel only for new messages. */
				if (pSubmitMsgOptionsImpl->submitTimeStamp > pReactorChannelImpl->lastSubmitOptionsTime)
				{
					rsslWatchlistClearProcessMsgOptions(&processOpts);
					processOpts.pChannel = pReactorChannelImpl->reactorChannel.pRsslChannel;
					processOpts.pRsslMsg = pSubmitMsgOptionsImpl->base.pRsslMsg;
					processOpts.pRdmMsg = pSubmitMsgOptionsImpl->base.pRDMMsg;
					processOpts.pServiceName = pSubmitMsgOptionsImpl->base.pServiceName;
					processOpts.pUserSpec = pSubmitMsgOptionsImpl->base.requestMsgOptions.pUserSpec;
					processOpts.majorVersion = pSubmitMsgOptionsImpl->base.majorVersion;
					processOpts.minorVersion = pSubmitMsgOptionsImpl->base.minorVersion;

					if (_reactorSubmitWatchlistMsg(pReactorChannelImpl->pParentReactor, pReactorChannelImpl, &processOpts, pError) != RSSL_RET_SUCCESS)
					{
						return RSSL_RET_FAILURE;
					}
				}
			}

			if (pWarmStandbyGroupImpl->sendQueueReqForAll)
			{
				rsslQueueRemoveLink(&pWarmStandbyHandler->submitMsgQueue, pLink);
				rsslQueueAddLinkToBack(&pWarmStandbyHandler->freeSubmitMsgQueue, pLink);
			}
		}

		pReactorChannelImpl->lastSubmitOptionsTime = getCurrentTimeMs(pReactorChannelImpl->pParentReactor->ticksPerMsec);

		/* Cleaning up the current message queue. */
		if (pWarmStandbyGroupImpl->sendQueueReqForAll)
		{
			while ((pLink = rsslQueueRemoveFirstLink(&pWarmStandbyHandler->freeSubmitMsgQueue)))
			{
				pSubmitMsgOptionsImpl = RSSL_QUEUE_LINK_TO_OBJECT(RsslReactorSubmitMsgOptionsImpl, submitMsgLink, pLink);
				_reactorFreeSubmitMsgOptions(pSubmitMsgOptionsImpl);
			}

			rsslInitQueue(&pWarmStandbyHandler->submitMsgQueue);
		}
	}

	return RSSL_RET_SUCCESS;
}

RsslBool _isActiveServiceForWSBChannelByID(RsslReactorWarmStandbyGroupImpl *pWarmStandByGroupImpl, RsslReactorChannelImpl *pReactorChannel, RsslUInt serviceID)
{
	RsslHashLink *pHashLink = NULL;
	RsslReactorWarmStandbyServiceImpl *pActiveWarmStandbyServiceImpl = NULL;

	pHashLink = rsslHashTableFind(&pWarmStandByGroupImpl->_perServiceById, &serviceID, NULL);

	if (pHashLink)
	{
		pActiveWarmStandbyServiceImpl = RSSL_HASH_LINK_TO_OBJECT(RsslReactorWarmStandbyServiceImpl, hashLink, pHashLink);

		/* Returns if this channel doesn't provide the active service */
		if (pReactorChannel != (RsslReactorChannelImpl*)pActiveWarmStandbyServiceImpl->pReactorChannel)
			return RSSL_FALSE;
	}
	else
	{
		return RSSL_FALSE;
	}

	return RSSL_TRUE;
}

RsslBool _isActiveServiceForWSBChannelByName(RsslReactorWarmStandbyGroupImpl *pWarmStandByGroupImpl, RsslReactorChannelImpl *pReactorChannel, const RsslBuffer* pServiceName)
{
	RsslWatchlistImpl *pWatchlistImpl = (RsslWatchlistImpl*)pReactorChannel->pWatchlist;
	WlServiceCache *pServiceCache = pWatchlistImpl->base.pServiceCache;
	RDMCachedService *pRDMCacheService = NULL;
	RsslHashLink *pHashLink = NULL;
	RsslUInt serviceId;
	RsslReactorWarmStandbyServiceImpl *pActiveWarmStandbyServiceImpl = NULL;

	if (pServiceName->data && pServiceName->length > 0)
	{
		pHashLink = rsslHashTableFind(&pServiceCache->_servicesByName, (void*)(pServiceName), NULL);

		if (pHashLink)
		{
			pRDMCacheService = RSSL_HASH_LINK_TO_OBJECT(RDMCachedService, _nameLink, pHashLink);

			serviceId = pRDMCacheService->rdm.serviceId;

			pHashLink = rsslHashTableFind(&pWarmStandByGroupImpl->_perServiceById, &serviceId, NULL);

			if (pHashLink)
			{
				pActiveWarmStandbyServiceImpl = RSSL_HASH_LINK_TO_OBJECT(RsslReactorWarmStandbyServiceImpl, hashLink, pHashLink);

				/* Returns if this channel doesn't provide the active service */
				if (pReactorChannel != (RsslReactorChannelImpl*)pActiveWarmStandbyServiceImpl->pReactorChannel)
					return RSSL_FALSE;
			}
		}
		else
		{
			return RSSL_FALSE;
		}
	}

	return RSSL_TRUE;
}

static RsslBool _isActiveServerForWSBChannel(RsslReactorWarmStandbyGroupImpl *pWarmStandByGroupImpl, RsslReactorChannelImpl *pReactorChannel, ReactorProcessMsgOptions *pOpts)
{
	if (_reactorHandlesWarmStandby(pReactorChannel))
	{
		if (pWarmStandByGroupImpl->warmStandbyMode == RSSL_RWSB_MODE_SERVICE_BASED)
		{
			RsslUInt16 *pServiceID = NULL;
			const RsslBuffer* pServiceName = NULL;

			if (pWarmStandByGroupImpl->_perServiceById.elementCount != 0)
			{
				RsslUInt serviceID = 0;

				if ((pOpts->pRsslMsg->msgBase.msgKey.flags & RSSL_MKF_HAS_SERVICE_ID) != 0)
				{
					serviceID = pOpts->pRsslMsg->msgBase.msgKey.serviceId;

					return _isActiveServiceForWSBChannelByID(pWarmStandByGroupImpl, pReactorChannel, serviceID);
				}
				else if(pOpts->pStreamInfo->pServiceName)
				{
					return _isActiveServiceForWSBChannelByName(pWarmStandByGroupImpl, pReactorChannel, pOpts->pStreamInfo->pServiceName);
				}
			}
		}
		else
		{
			/* Returns if this channel is not the active server */
			if (pReactorChannel->isActiveServer == RSSL_FALSE)
				return RSSL_FALSE;
		}
	}

	return RSSL_TRUE;
}

static RsslBool _warmStandbyCallbackChecks(RsslReactorChannelImpl *pReactorChannel, ReactorProcessMsgOptions *pOpts)
{
	if (_reactorHandlesWarmStandby(pReactorChannel))
	{
		RsslReactorWarmStandByHandlerImpl *pWarmStandByHandlerImpl = pReactorChannel->pWarmStandByHandlerImpl;
		RsslReactorWarmStandbyGroupImpl *pWarmStandByGroupImpl = &pWarmStandByHandlerImpl->warmStandbyGroupList[pWarmStandByHandlerImpl->currentWSyGroupIndex];

		/* Notify status message for non-recoverable streams or channels are down. */
		if ( (pOpts->wsbStatusFlags & WL_MEF_NOTIFY_STATUS) || isWarmStandbyChannelClosed(pWarmStandByHandlerImpl, NULL))
		{
			return RSSL_TRUE;
		}

		return _isActiveServerForWSBChannel(pWarmStandByGroupImpl, pReactorChannel, pOpts);
	}

	return RSSL_TRUE;
}

static RsslBool _getDictionaryVersion(RsslBuffer* srcBuffer, RsslReactorDictionaryVersion* pDictionaryVersion)
{
	char* pToken = NULL;
	char* pNextToken = NULL;
	RsslUInt32 pos = 0;
	char versionBuffer[512];

	memset(&versionBuffer, 0, 512);
	memcpy(&versionBuffer, srcBuffer->data, srcBuffer->length);

	rsslClearReactorDictionaryVersion(pDictionaryVersion);

	pToken = strtok(&versionBuffer[0], ".");

	if (pToken == NULL)
	{
		return RSSL_FALSE;
	}

	while (pToken != NULL)
	{
		pNextToken = strtok(NULL, ".");

		if (pos == 0)
		{	/* Major version */
			pDictionaryVersion->major = atoi(pToken);

			if (pDictionaryVersion->major == 0)
			{
				return RSSL_FALSE;
			}
		}
		else if (pos == 1)
		{	/* Minor version */
			pDictionaryVersion->minor = atoi(pToken);

			if (pDictionaryVersion->minor == 0)
			{
				return RSSL_FALSE;
			}
			break;
		}

		++pos;

		pToken = pNextToken;
	}

	return RSSL_TRUE;
}

static RsslRet _reactorWSReadWatchlistMsg(RsslReactorImpl *pReactorImpl, RsslReactorChannelImpl *pReactorChannel,
	RsslWatchlistProcessMsgOptions *pOptions, RsslErrorInfo *pError)
{
	RsslRet ret = RSSL_RET_SUCCESS;
	RsslReactorWarmStandByHandlerImpl *pWarmStandByHandlerImpl = pReactorChannel->pWarmStandByHandlerImpl;
	RsslReactorWarmStandbyGroupImpl *pWarmStandByGroupImpl = &pWarmStandByHandlerImpl->warmStandbyGroupList[pWarmStandByHandlerImpl->currentWSyGroupIndex];
	RsslReactorChannelImpl *pProcessReactorChannel = NULL;
	RsslQueueLink *pLink = NULL;
	RsslStatusMsg *pStatusMsg = NULL;
	RsslBool closeOnStandby = RSSL_FALSE;
	RsslBool isActiveServer = RSSL_FALSE;
	ReactorWSProcessMsgOptions wsProcessMsgOptions;

	rsslWatchlistClearWSProcessMsgOptions(&wsProcessMsgOptions);

	ret = _reactorReadWatchlistMsg(pReactorImpl, pReactorChannel, pOptions, &wsProcessMsgOptions, pError);

	if (ret != RSSL_RET_SUCCESS)
		return ret;

	if (wsProcessMsgOptions.pStream != NULL)
	{
		WlStream *pStream = (WlStream*)wsProcessMsgOptions.pStream;

		if (pStream->base.domainType >= RSSL_DMT_MARKET_PRICE)
		{
			if (pWarmStandByGroupImpl->warmStandbyMode == RSSL_RWSB_MODE_LOGIN_BASED)
			{
				isActiveServer = pReactorChannel->isActiveServer;
			}
			else
			{
				if (pStream->item.pWlService && pStream->item.pWlService->pService)
				{
					isActiveServer = _isActiveServiceForWSBChannelByID(pWarmStandByGroupImpl, pReactorChannel, pStream->item.pWlService->pService->rdm.serviceId);
				}
			}
		}
	}

	/* Fanout the close message to all standby servers */
	if (isActiveServer)
	{
		int originalStreamId = pOptions->pRsslMsg->msgBase.streamId;
		if (pOptions->pRsslMsg && pOptions->pRsslMsg->msgBase.msgClass == RSSL_MC_STATUS && pOptions->pRsslMsg->msgBase.domainType != RSSL_DMT_SYMBOL_LIST)
		{
			pStatusMsg = (RsslStatusMsg*)pOptions->pRsslMsg;

			if ( (pStatusMsg->flags & RSSL_STMF_HAS_STATE) != 0)
			{
				RsslStreamStates streamState = pStatusMsg->state.streamState;
				if (streamState == RSSL_STREAM_CLOSED)
				{
					closeOnStandby = RSSL_TRUE;
				}
			}
		}

		if (closeOnStandby)
		{
			RsslCloseMsg rsslCloseMsg;
			rsslClearCloseMsg(&rsslCloseMsg);
			rsslCloseMsg.msgBase.streamId = pOptions->pRsslMsg->msgBase.streamId;

			RSSL_MUTEX_LOCK(&pWarmStandByHandlerImpl->warmStandByHandlerMutex);
			RSSL_QUEUE_FOR_EACH_LINK(&pWarmStandByHandlerImpl->rsslChannelQueue, pLink)
			{
				pProcessReactorChannel = RSSL_QUEUE_LINK_TO_OBJECT(RsslReactorChannelImpl, warmstandbyChannelLink, pLink);

				if(pProcessReactorChannel != pReactorChannel)
				{
					RsslWatchlistProcessMsgOptions processOpts;
					rsslWatchlistClearProcessMsgOptions(&processOpts);
					processOpts.pChannel = pProcessReactorChannel->reactorChannel.pRsslChannel;
					processOpts.pRsslMsg = (RsslMsg*)&rsslCloseMsg;
					processOpts.majorVersion = pProcessReactorChannel->reactorChannel.majorVersion;
					processOpts.minorVersion = pProcessReactorChannel->reactorChannel.minorVersion;

					_reactorSubmitWatchlistMsg(pReactorImpl, pProcessReactorChannel, &processOpts, pError);
				}

			}
			RSSL_MUTEX_UNLOCK(&pWarmStandByHandlerImpl->warmStandByHandlerMutex);
		}
	}

	return ret;
}

static RsslRet _reactorWSHandleRecoveryStatusMsg(RsslReactorChannelImpl *pReactorChannel, ReactorProcessMsgOptions *pOpts)
{
	RsslReactorWarmStandByHandlerImpl *pWarmStandbyHandleImpl = pReactorChannel->pWarmStandByHandlerImpl;

	if ( pWarmStandbyHandleImpl && (pWarmStandbyHandleImpl->pReadMsgChannel == pReactorChannel) )
	{
		if (pOpts->pRsslMsg && pOpts->pRsslMsg->msgBase.msgClass == RSSL_MC_STATUS)
		{
			RsslStatusMsg *pStatusMsg = &pOpts->pRsslMsg->statusMsg;
			RsslStreamStates streamState = pStatusMsg->state.streamState;

			if ((streamState == RSSL_STREAM_CLOSED) || (streamState == RSSL_STREAM_CLOSED_RECOVER) || (streamState == RSSL_STREAM_REDIRECTED))
			{
				RsslReactorWSRecoveryMsgInfo* pWSRecoveryMsgInfo = NULL;

				/* Keeps the information to internally generate status message when this channel becomes active later. */
				if (pReactorChannel->pWSRecoveryMsgList == NULL)
				{
					pReactorChannel->pWSRecoveryMsgList = (RsslQueue*)malloc(sizeof(RsslQueue));

					if (pReactorChannel->pWSRecoveryMsgList)
					{
						rsslInitQueue(pReactorChannel->pWSRecoveryMsgList);
					}
				}

				pWSRecoveryMsgInfo = (RsslReactorWSRecoveryMsgInfo*)malloc(sizeof(RsslReactorWSRecoveryMsgInfo));

				if (pWSRecoveryMsgInfo)
				{
					RsslMsgKey* pRsslMsgKey = &pOpts->pRsslMsg->msgBase.msgKey;

					rsslClearReactorWSRecoveryMsgInfo(pWSRecoveryMsgInfo);

					pWSRecoveryMsgInfo->containerType = RSSL_DT_NO_DATA;
					pWSRecoveryMsgInfo->domainType = pOpts->pRsslMsg->msgBase.domainType;
					pWSRecoveryMsgInfo->streamId = pOpts->pRsslMsg->msgBase.streamId;
					pWSRecoveryMsgInfo->pUserSpec = pOpts->pStreamInfo->pUserSpec;

					pWSRecoveryMsgInfo->flags |= RSSL_STMF_HAS_STATE;
					pWSRecoveryMsgInfo->rsslState.streamState = RSSL_STREAM_CLOSED_RECOVER;
					pWSRecoveryMsgInfo->rsslState.dataState = pStatusMsg->state.dataState;
					pWSRecoveryMsgInfo->rsslState.code = pStatusMsg->state.code;

					if (pStatusMsg->state.text.data && pStatusMsg->state.text.length)
					{
						pWSRecoveryMsgInfo->rsslState.text.data = (char*)malloc((size_t)pStatusMsg->state.text.length + 1);
						if (pWSRecoveryMsgInfo->rsslState.text.data)
						{
							memset(pWSRecoveryMsgInfo->rsslState.text.data, 0, (size_t)pStatusMsg->state.text.length + 1);
							memcpy(pWSRecoveryMsgInfo->rsslState.text.data, pStatusMsg->state.text.data, pStatusMsg->state.text.length);
							pWSRecoveryMsgInfo->rsslState.text.length = pStatusMsg->state.text.length;
						}
					}

					if (pOpts->pStreamInfo->pServiceName && pOpts->pStreamInfo->pServiceName->data)
					{
						RsslBuffer serviceName = RSSL_INIT_BUFFER;
						serviceName.data = (char*)malloc((size_t)pOpts->pStreamInfo->pServiceName->length + 1);

						if (serviceName.data)
						{
							memset(serviceName.data, 0, (size_t)pOpts->pStreamInfo->pServiceName->length + 1);
							memcpy(serviceName.data, pOpts->pStreamInfo->pServiceName->data, pOpts->pStreamInfo->pServiceName->length);
							serviceName.length = pOpts->pStreamInfo->pServiceName->length;
							pWSRecoveryMsgInfo->serviceName = serviceName;
						}
					}

					if (pRsslMsgKey->flags != 0)
					{
						if (pRsslMsgKey->flags & RSSL_MKF_HAS_SERVICE_ID)
						{
							pWSRecoveryMsgInfo->msgKey.serviceId = pRsslMsgKey->serviceId;
							pWSRecoveryMsgInfo->msgKey.flags |= RSSL_MKF_HAS_SERVICE_ID;
						}

						if (pRsslMsgKey->flags & RSSL_MKF_HAS_NAME)
						{
							if (pRsslMsgKey->name.data && pRsslMsgKey->name.length)
							{
								pWSRecoveryMsgInfo->msgKey.name.data = (char*)malloc((size_t)pRsslMsgKey->name.length + 1);

								if (pWSRecoveryMsgInfo->msgKey.name.data)
								{
									memset(pWSRecoveryMsgInfo->msgKey.name.data, 0, (size_t)pRsslMsgKey->name.length + 1);
									memcpy(pWSRecoveryMsgInfo->msgKey.name.data, pRsslMsgKey->name.data, pRsslMsgKey->name.length);
									pWSRecoveryMsgInfo->msgKey.name.length = pRsslMsgKey->name.length;
									pWSRecoveryMsgInfo->msgKey.flags |= RSSL_MKF_HAS_NAME;
								}
							}
						}

						if (pRsslMsgKey->flags & RSSL_MKF_HAS_NAME_TYPE)
						{
							pWSRecoveryMsgInfo->msgKey.nameType = pRsslMsgKey->nameType;
							pWSRecoveryMsgInfo->msgKey.flags |= RSSL_MKF_HAS_NAME_TYPE;
						}

						if (pRsslMsgKey->flags & RSSL_MKF_HAS_FILTER)
						{
							pWSRecoveryMsgInfo->msgKey.filter = pRsslMsgKey->filter;
							pWSRecoveryMsgInfo->msgKey.flags |= RSSL_MKF_HAS_FILTER;
						}

						if (pRsslMsgKey->flags & RSSL_MKF_HAS_IDENTIFIER)
						{
							pWSRecoveryMsgInfo->msgKey.identifier = pRsslMsgKey->identifier;
							pWSRecoveryMsgInfo->msgKey.flags |= RSSL_MKF_HAS_IDENTIFIER;
						}

						if (pWSRecoveryMsgInfo->msgKey.flags != 0)
						{
							pWSRecoveryMsgInfo->flags |= RSSL_STMF_HAS_MSG_KEY;
						}
					}

					rsslQueueAddLinkToBack(pReactorChannel->pWSRecoveryMsgList, &pWSRecoveryMsgInfo->queueLink);
				}
			}
		}
	}

	return RSSL_RET_SUCCESS;
}

RsslBool isRsslChannelActive(RsslReactorChannelImpl *pReactorChannelImpl)
{
	if (pReactorChannelImpl->reactorChannel.pRsslChannel)
	{
		if (pReactorChannelImpl->reactorChannel.pRsslChannel->state == RSSL_CH_STATE_ACTIVE)
		{
			if (pReactorChannelImpl->reactorParentQueue == &pReactorChannelImpl->pParentReactor->reconnectingChannels ||
				pReactorChannelImpl->reactorParentQueue == &pReactorChannelImpl->pParentReactor->inactiveChannels)
			{
				return RSSL_FALSE;
			}
			else
			{
				return RSSL_TRUE;
			}
		}
	}

	return RSSL_FALSE;
}

static RsslRet _reactorWSWriteWatchlistMsg(RsslReactorImpl *pReactorImpl, RsslReactorChannelImpl *pReactorChannel, RsslReactorSubmitMsgOptions *pOptions, RsslWatchlistProcessMsgOptions *pProcessOpts, 
	RsslErrorInfo *pError)
{
	RsslRet ret = RSSL_RET_SUCCESS;
	RsslQueueLink* pLink = NULL;
	RsslReactorChannelImpl* pSubmitReactorChannel;
	RsslReactorSubmitMsgOptionsImpl* pSubmitMsgOptionImpl = NULL;
	RsslReactorWarmStandByHandlerImpl *pWarmStandByHandlerImpl = pReactorChannel->pWarmStandByHandlerImpl;
	RsslReactorWarmStandbyGroupImpl *pReactorWarmStandByGroupImpl = &pWarmStandByHandlerImpl->warmStandbyGroupList[pWarmStandByHandlerImpl->currentWSyGroupIndex];
	RsslBool submitMsgToChannel = RSSL_TRUE;
	RsslBool breakLoop = RSSL_FALSE;
	RsslUInt32 submitSuccessCount = 0;
	RsslUInt32 submitCount = 0;
	RsslBool isLoginRequest = RSSL_FALSE;
	RsslBool addMessageToQueue = RSSL_FALSE;

	/* Keeps submit messages in the request queue if all channels are not available yet. */
	if (pReactorWarmStandByGroupImpl->sendQueueReqForAll == RSSL_FALSE)
	{
		/* Copies the submit message to send it later for standby connections. */
		pSubmitMsgOptionImpl = (RsslReactorSubmitMsgOptionsImpl*)malloc(sizeof(RsslReactorSubmitMsgOptionsImpl));

		if (!pSubmitMsgOptionImpl)
		{
			ret = RSSL_RET_FAILURE;
			rsslSetErrorInfo(pError, RSSL_EIC_FAILURE, RSSL_RET_FAILURE, __FILE__, __LINE__,
				"Memory allocation failed for copying submit message options.");
			return ret;
		}

		rsslClearReactorSubmitMsgOptionsImpl(pSubmitMsgOptionImpl);

		if (pSubmitMsgOptionImpl->pMemoryLocation)
		{
			free(pSubmitMsgOptionImpl->pMemoryLocation);
		}

		if (pOptions->pRDMMsg)
		{
			RsslRDMMsg* pRDMMsg = pOptions->pRDMMsg;

			if (pRDMMsg->rdmMsgBase.domainType == RSSL_DMT_LOGIN && pRDMMsg->rdmMsgBase.rdmMsgType == RDM_LG_MT_REQUEST)
			{
				isLoginRequest = RSSL_TRUE;
			}
		}
		else if (pOptions->pRsslMsg)
		{
			if (pOptions->pRsslMsg->msgBase.domainType == RSSL_DMT_LOGIN && pOptions->pRsslMsg->msgBase.msgClass == RSSL_MC_REQUEST)
			{
				isLoginRequest = RSSL_TRUE;
			}
		}

		/* Don't keep the login request as the starting server is used */
		if (!isLoginRequest)
		{
			if (copyReactorSubmitMsgOption(pReactorImpl, pSubmitMsgOptionImpl, pOptions) != RSSL_RET_SUCCESS)
			{
				ret = RSSL_RET_FAILURE;
				rsslSetErrorInfo(pError, RSSL_EIC_FAILURE, RSSL_RET_FAILURE, __FILE__, __LINE__,
					"Failed to copy submit message options.");
				return ret;
			}

			addMessageToQueue = RSSL_TRUE;
		}
	}

	ret = RSSL_RET_SUCCESS;

	RSSL_MUTEX_LOCK(&pWarmStandByHandlerImpl->warmStandByHandlerMutex);
	/* Submits to all channels that belongs to the warm standby feature */
	RSSL_QUEUE_FOR_EACH_LINK(&pReactorChannel->pWarmStandByHandlerImpl->rsslChannelQueue, pLink)
	{
		pSubmitReactorChannel = RSSL_QUEUE_LINK_TO_OBJECT(RsslReactorChannelImpl, warmstandbyChannelLink, pLink);

		pProcessOpts->pChannel = ((RsslReactorChannel*)pSubmitReactorChannel)->pRsslChannel;

		++submitCount;

		if (pProcessOpts->pRsslMsg)
		{
			switch (pProcessOpts->pRsslMsg->msgBase.msgClass)
			{
				case RSSL_MC_GENERIC:
				{
					/* Submit only to a active channel for service only. */

					if (isRsslChannelActive(pSubmitReactorChannel) == RSSL_FALSE)
						continue;

					ret = _reactorSubmitWatchlistMsg(pReactorImpl, pSubmitReactorChannel, pProcessOpts, pError);

					break;
				}
				case RSSL_MC_REQUEST:
				{
					RsslRequestMsg *pRequestMsg = &pProcessOpts->pRsslMsg->requestMsg;

					if (pRequestMsg->flags & RSSL_RQMF_PRIVATE_STREAM)
					{
						/* Don't send a private stream request to standby server(s) */
						if (pReactorWarmStandByGroupImpl->warmStandbyMode == RSSL_RWSB_MODE_LOGIN_BASED)
						{
							if(pSubmitReactorChannel->isActiveServer == RSSL_FALSE)
								continue;
						}
						else
						{
							if (pProcessOpts->pServiceName && pProcessOpts->pServiceName->data && pProcessOpts->pServiceName->length)
							{
								if (_isActiveServiceForWSBChannelByName(pReactorWarmStandByGroupImpl, pSubmitReactorChannel, pProcessOpts->pServiceName) == RSSL_FALSE)
								{
									continue;
								}
							}
							else
							{
								if (pRequestMsg->msgBase.msgKey.flags &  RSSL_MKF_HAS_SERVICE_ID)
								{
									RsslUInt serviceId = pRequestMsg->msgBase.msgKey.serviceId;

									if (_isActiveServiceForWSBChannelByID(pReactorWarmStandByGroupImpl, pSubmitReactorChannel, serviceId) == RSSL_FALSE)
									{
										continue;
									}
								}
							}
						}

						ret = _reactorSubmitWatchlistMsg(pReactorImpl, pSubmitReactorChannel, pProcessOpts, pError);

						breakLoop = RSSL_TRUE;
					}
					else
					{
						ret = _reactorSubmitWatchlistMsg(pReactorImpl, pSubmitReactorChannel, pProcessOpts, pError);
					}

					break;
				}
				case RSSL_MC_POST:
				{
					/* Submit to all active channel for the service. */
					if (isRsslChannelActive(pSubmitReactorChannel) == RSSL_FALSE)
						continue;

					_reactorSubmitWatchlistMsg(pReactorImpl, pSubmitReactorChannel, pProcessOpts, pError);
					break;
				}
				default:
				{
					 _reactorSubmitWatchlistMsg(pReactorImpl, pSubmitReactorChannel, pProcessOpts, pError);
					 break;
				}
			}
		}
		else if (pProcessOpts->pRdmMsg)
		{
			ret = _reactorSubmitWatchlistMsg(pReactorImpl, pSubmitReactorChannel, pProcessOpts, pError);
		}

		if (ret != RSSL_RET_SUCCESS || breakLoop)
		{
			break; 
		}
	}
	RSSL_MUTEX_UNLOCK(&pWarmStandByHandlerImpl->warmStandByHandlerMutex);

	if (submitCount == 0)
	{
		rsslSetErrorInfo(pError, RSSL_EIC_FAILURE, RSSL_RET_INVALID_ARGUMENT, __FILE__, __LINE__,
			"The warm standby channel is not active.");
		return RSSL_RET_INVALID_DATA;
	}

	/* Adds the message to queue when successfully submitting the message. */
	if (addMessageToQueue)
	{
		if (ret == RSSL_RET_SUCCESS)
		{
			rsslQueueAddLinkToBack(&pReactorChannel->pWarmStandByHandlerImpl->submitMsgQueue, &pSubmitMsgOptionImpl->submitMsgLink);
		}
		else
		{
			/* Cleaning up the message.*/
			_reactorFreeSubmitMsgOptions(pSubmitMsgOptionImpl);
		}
	}

	return ret;
}

static RsslRet _reactorWSNotifyStatusMsg(RsslReactorChannelImpl *pReactorChannel)
{
	RsslRet ret = RSSL_RET_SUCCESS;
	RsslMsgEvent msgEvent;
	RsslStatusMsg statusMsg;
	RsslStreamInfo streamInfo;
	RsslQueueLink *pLink;
	RsslReactorWSRecoveryMsgInfo* pWSRecoveryMsgInfo = NULL;
	RsslReactorWarmStandByHandlerImpl *pWarmStandByHandlerImpl = pReactorChannel->pWarmStandByHandlerImpl;

	if (pReactorChannel->pWSRecoveryMsgList == NULL)
		return ret;

	while ((pLink = rsslQueueRemoveFirstLink(pReactorChannel->pWSRecoveryMsgList)))
	{
		pWSRecoveryMsgInfo = RSSL_QUEUE_LINK_TO_OBJECT(RsslReactorWSRecoveryMsgInfo, queueLink, pLink);

		rsslClearMsgEvent(&msgEvent);
		rsslClearStatusMsg(&statusMsg);

		statusMsg.msgBase.containerType = pWSRecoveryMsgInfo->containerType;
		statusMsg.msgBase.domainType = pWSRecoveryMsgInfo->domainType;
		statusMsg.msgBase.streamId = pWSRecoveryMsgInfo->streamId;
		statusMsg.msgBase.msgKey = pWSRecoveryMsgInfo->msgKey;
		statusMsg.flags = pWSRecoveryMsgInfo->flags;
		statusMsg.state = pWSRecoveryMsgInfo->rsslState;

		streamInfo.pServiceName = &pWSRecoveryMsgInfo->serviceName;
		streamInfo.pUserSpec = pWSRecoveryMsgInfo->pUserSpec;

		msgEvent.pRsslMsg = (RsslMsg*)&statusMsg;
		msgEvent.pStreamInfo = (RsslStreamInfo*)&streamInfo;

		_reactorSetInCallback(pReactorChannel->pParentReactor, RSSL_TRUE);
		(*pReactorChannel->channelRole.base.defaultMsgCallback)((RsslReactor*)pReactorChannel->pParentReactor, (RsslReactorChannel*)pReactorChannel, &msgEvent);
		_reactorSetInCallback(pReactorChannel->pParentReactor, RSSL_FALSE);

		/* Send close status for all channels that still subscribes to this item. */
		{
			RsslCloseMsg rsslCloseMsg;
			RsslReactorChannelImpl *pProcessReactorChannel;
			RsslErrorInfo rsslErrorInfo;

			rsslClearCloseMsg(&rsslCloseMsg);
			rsslCloseMsg.msgBase.streamId = pWSRecoveryMsgInfo->streamId;

			RSSL_MUTEX_LOCK(&pWarmStandByHandlerImpl->warmStandByHandlerMutex);
			RSSL_QUEUE_FOR_EACH_LINK(&pWarmStandByHandlerImpl->rsslChannelQueue, pLink)
			{
				pProcessReactorChannel = RSSL_QUEUE_LINK_TO_OBJECT(RsslReactorChannelImpl, warmstandbyChannelLink, pLink);

				if (pProcessReactorChannel != pReactorChannel)
				{
					RsslWatchlistProcessMsgOptions processOpts;
					rsslWatchlistClearProcessMsgOptions(&processOpts);
					processOpts.pChannel = pProcessReactorChannel->reactorChannel.pRsslChannel;
					processOpts.pRsslMsg = (RsslMsg*)&rsslCloseMsg;
					processOpts.majorVersion = pProcessReactorChannel->reactorChannel.majorVersion;
					processOpts.minorVersion = pProcessReactorChannel->reactorChannel.minorVersion;

					_reactorSubmitWatchlistMsg(pReactorChannel->pParentReactor, pProcessReactorChannel, &processOpts, &rsslErrorInfo);
				}

			}
			RSSL_MUTEX_UNLOCK(&pWarmStandByHandlerImpl->warmStandByHandlerMutex);
		}

		if (pWSRecoveryMsgInfo->msgKey.name.data)
			free(pWSRecoveryMsgInfo->msgKey.name.data);

		if (pWSRecoveryMsgInfo->serviceName.data)
			free(pWSRecoveryMsgInfo->serviceName.data);

		if (pWSRecoveryMsgInfo->rsslState.text.data)
			free(pWSRecoveryMsgInfo->rsslState.text.data);

		free(pWSRecoveryMsgInfo);
	}

	pReactorChannel->pWSRecoveryMsgList = NULL;

	return ret;
}

static RsslRet _reactorQueuedWSBGroupRecoveryMsg(RsslReactorWarmStandByHandlerImpl* pWarmStandbyHandler, RsslErrorInfo* pErrorInfo)
{
	RsslRet ret = RSSL_RET_SUCCESS;

	/* Keeps the recovery item requests from the starting channel only when moving to a new warmstandby group. */
	if (pWarmStandbyHandler->pStartingReactorChannel)
	{
		RsslReactorChannelImpl* pReactorChannelImpl = pWarmStandbyHandler->pStartingReactorChannel;
		RsslWatchlistImpl* pWatchlistImpl = (RsslWatchlistImpl *)pReactorChannelImpl->pWatchlist;
		RsslReactorSubmitMsgOptionsImpl* pSubmitMsgOptionImpl = NULL;

		if (pReactorChannelImpl->pWarmStandByHandlerImpl->queuedRecoveryMessage == RSSL_FALSE)
		{
			RsslQueueLink* pLink;
			RsslQueueLink* pRequestLink;
			WlItemRequest* pItemRequest;

			RSSL_QUEUE_FOR_EACH_LINK(&pWatchlistImpl->base.requestedServices, pLink)
			{
				WlRequestedService* pRequestedService = RSSL_QUEUE_LINK_TO_OBJECT(WlRequestedService,
					qlServiceRequests, pLink);

				RSSL_QUEUE_FOR_EACH_LINK(&pRequestedService->itemRequests, pRequestLink)
				{
					pItemRequest = RSSL_QUEUE_LINK_TO_OBJECT(WlItemRequest,
						qlRequestedService, pRequestLink);

					pSubmitMsgOptionImpl = (RsslReactorSubmitMsgOptionsImpl*)malloc(sizeof(RsslReactorSubmitMsgOptionsImpl));

					if (!pSubmitMsgOptionImpl)
					{
						ret = RSSL_RET_FAILURE;
						rsslSetErrorInfo(pErrorInfo, RSSL_EIC_FAILURE, RSSL_RET_FAILURE, __FILE__, __LINE__,
							"Memory allocation failed for copying submit message options.");
						return ret;
					}

					rsslClearReactorSubmitMsgOptionsImpl(pSubmitMsgOptionImpl);

					if (copyWlItemRequest(pReactorChannelImpl->pParentReactor, pSubmitMsgOptionImpl, pItemRequest) != RSSL_RET_SUCCESS)
					{
						ret = RSSL_RET_FAILURE;
						rsslSetErrorInfo(pErrorInfo, RSSL_EIC_FAILURE, RSSL_RET_FAILURE, __FILE__, __LINE__,
							"Failed to copy submit message options.");
						return ret;
					}

					rsslQueueAddLinkToBack(&pReactorChannelImpl->pWarmStandByHandlerImpl->submitMsgQueue, &pSubmitMsgOptionImpl->submitMsgLink);
				}
			}

			RSSL_QUEUE_FOR_EACH_LINK(&pWatchlistImpl->base.newRequests, pLink)
			{
				pItemRequest =
					RSSL_QUEUE_LINK_TO_OBJECT(WlItemRequest, base.qlStateQueue, pLink);

				pSubmitMsgOptionImpl = (RsslReactorSubmitMsgOptionsImpl*)malloc(sizeof(RsslReactorSubmitMsgOptionsImpl));

				if (!pSubmitMsgOptionImpl)
				{
					ret = RSSL_RET_FAILURE;
					rsslSetErrorInfo(pErrorInfo, RSSL_EIC_FAILURE, RSSL_RET_FAILURE, __FILE__, __LINE__,
						"Memory allocation failed for copying submit message options.");
					return ret;
				}

				rsslClearReactorSubmitMsgOptionsImpl(pSubmitMsgOptionImpl);

				if (copyWlItemRequest(pReactorChannelImpl->pParentReactor, pSubmitMsgOptionImpl, pItemRequest) != RSSL_RET_SUCCESS)
				{
					ret = RSSL_RET_FAILURE;
					rsslSetErrorInfo(pErrorInfo, RSSL_EIC_FAILURE, RSSL_RET_FAILURE, __FILE__, __LINE__,
						"Failed to copy submit message options.");
					return ret;
				}

				rsslQueueAddLinkToBack(&pReactorChannelImpl->pWarmStandByHandlerImpl->submitMsgQueue, &pSubmitMsgOptionImpl->submitMsgLink);
			}

			pReactorChannelImpl->pWarmStandByHandlerImpl->queuedRecoveryMessage = RSSL_TRUE;
		}
	}

	return ret;
}

RsslReactorConnectInfoImpl* _reactorGetCurrentReactorConnectInfoImpl(RsslReactorChannelImpl* pReactorChannel)
{
	RsslReactorConnectInfoImpl* pReactorConnectInfoImpl = NULL;

	if (_reactorHandlesWarmStandby(pReactorChannel))
	{
		RsslReactorWarmStandbyGroupImpl* pWarmStandbyGroupImpl = &pReactorChannel->pWarmStandByHandlerImpl->warmStandbyGroupList[pReactorChannel->pWarmStandByHandlerImpl->currentWSyGroupIndex];

		if (pReactorChannel->isStartingServerConfig)
		{
			pReactorConnectInfoImpl = &pWarmStandbyGroupImpl->startingActiveServer.reactorConnectInfoImpl;
		}
		else
		{
			pReactorConnectInfoImpl = &pWarmStandbyGroupImpl->standbyServerList[pReactorChannel->standByServerListIndex].reactorConnectInfoImpl;
		}
	}
	else
	{
		pReactorConnectInfoImpl = &pReactorChannel->connectionOptList[pReactorChannel->connectionListIter];
	}

	return pReactorConnectInfoImpl;
}

static void _reactorWSDirectoryUpdateFromChannelDown(RsslReactorWarmStandbyGroupImpl* pWarmStandbyGroupImpl, RsslReactorChannelImpl* pReactorChannelImpl, WlServiceCache* pServiceCache)
{
	RsslRet ret = RSSL_RET_SUCCESS;
	RsslQueueLink* pLink = NULL;
	RsslHashLink* pHashLink = NULL;
	RDMCachedService* pService = NULL;
	RsslReactorWarmStandbyServiceImpl* pReactorWarmStandbyServiceImpl = NULL;

	RSSL_QUEUE_FOR_EACH_LINK(&pServiceCache->_serviceList, pLink)
	{
		pService = RSSL_QUEUE_LINK_TO_OBJECT(RDMCachedService, _fullListLink, pLink);

		if (pService)
		{
			pHashLink = rsslHashTableFind(&pWarmStandbyGroupImpl->_perServiceById, &pService->rdm.serviceId, NULL);

			if (pHashLink)
			{
				RsslUInt currentServiceState = 0;
				pReactorWarmStandbyServiceImpl = RSSL_HASH_LINK_TO_OBJECT(RsslReactorWarmStandbyServiceImpl, hashLink, pHashLink);

				/* Handles the delete action to remove the service */
				if (pService->rdm.action == RSSL_MPEA_DELETE_ENTRY)
				{
					RsslReactorChannelImpl* pProcessReactorChannel = NULL;
					RsslQueueLink* pLink;
					RsslBool hasService = RSSL_FALSE;
					RsslWatchlistImpl* pWatchlistImpl = NULL;
					RsslReactorWarmStandByHandlerImpl* pReactorWarmStandByHandlerImpl = pReactorChannelImpl->pWarmStandByHandlerImpl;
					RDMCachedService* pOtherCachedService = NULL;

					/* Checks whether others channels provide this service as well. */
					RSSL_MUTEX_LOCK(&pReactorWarmStandByHandlerImpl->warmStandByHandlerMutex);
					RSSL_QUEUE_FOR_EACH_LINK(&pReactorWarmStandByHandlerImpl->rsslChannelQueue, pLink)
					{
						pProcessReactorChannel = RSSL_QUEUE_LINK_TO_OBJECT(RsslReactorChannelImpl, warmstandbyChannelLink, pLink);

						if (pProcessReactorChannel != pReactorChannelImpl)
						{
							if (pProcessReactorChannel->reactorParentQueue == &pProcessReactorChannel->pParentReactor->closingChannels)
							{
								continue; /* Countinue if this channel is being closed. */
							}

							pWatchlistImpl = (RsslWatchlistImpl*)pProcessReactorChannel->pWatchlist;

							pHashLink = rsslHashTableFind(&pWatchlistImpl->base.pServiceCache->_servicesById, &pService->rdm.serviceId, NULL);
							if (pHashLink)
							{
								pOtherCachedService = RSSL_HASH_LINK_TO_OBJECT(RDMCachedService, _idLink, pHashLink);

								if (pOtherCachedService->rdm.action != RSSL_MPEA_DELETE_ENTRY)
								{
									hasService = RSSL_TRUE;
									break;
								}
							}
						}
					}
					RSSL_MUTEX_UNLOCK(&pReactorWarmStandByHandlerImpl->warmStandByHandlerMutex);

					if (hasService == RSSL_FALSE)
					{
						pReactorWarmStandbyServiceImpl->serviceAction = RSSL_MPEA_DELETE_ENTRY;

						rsslQueueAddLinkToBack(&pWarmStandbyGroupImpl->_updateServiceList, &pReactorWarmStandbyServiceImpl->updateQLink);
					}
				}
			}
		}
	}
}

RSSL_VA_API RsslRet rsslReactorIoctl(RsslReactor* pReactor, RsslReactorIoctlCodes code, void* value, RsslErrorInfo* pError)
{
	RsslReactorImpl* pReactorImpl = (RsslReactorImpl*)pReactor;
	RsslRet ret = RSSL_RET_SUCCESS;

	if (!pError)
		return RSSL_RET_INVALID_ARGUMENT;

	if (!pReactor)
	{
		rsslSetErrorInfo(pError, RSSL_EIC_FAILURE, RSSL_RET_INVALID_ARGUMENT, __FILE__, __LINE__, "RsslReactor not provided.");
		return RSSL_RET_INVALID_ARGUMENT;
	}

	if ((ret = reactorLockInterface(pReactorImpl, RSSL_TRUE, pError)) != RSSL_RET_SUCCESS)
		return ret;

	if (pReactorImpl->state != RSSL_REACTOR_ST_ACTIVE)
	{
		rsslSetErrorInfo(pError, RSSL_EIC_FAILURE, RSSL_RET_INVALID_ARGUMENT, __FILE__, __LINE__, "Reactor is shutting down.");
		return (reactorUnlockInterface(pReactorImpl), RSSL_RET_FAILURE);
	}

	switch (code)
	{
	case RSSL_RIC_ENABLE_REST_LOGGING:
	{
		RsslBool restEnableLog = RSSL_FALSE;
		if (value != NULL && *((int*)value) != 0)
		{
			restEnableLog = RSSL_TRUE;
		}
		pReactorImpl->restEnableLog = restEnableLog;
		break;
	}
	case RSSL_RIC_ENABLE_REST_CALLBACK_LOGGING:
	{
		RsslBool restEnableLogCallback = RSSL_FALSE;
		if (value != NULL && *((int*)value) != 0)
		{
			restEnableLogCallback = RSSL_TRUE;
		}
		pReactorImpl->restEnableLogCallback = restEnableLogCallback;
		break;
	}

	default:
		ret = RSSL_RET_INVALID_ARGUMENT;
		rsslSetErrorInfo(pError, RSSL_EIC_FAILURE, RSSL_RET_INVALID_ARGUMENT, __FILE__, __LINE__, "rsslReactorIoctl code (%d) undefined.", code);
		break;
	}

	return (reactorUnlockInterface(pReactorImpl), ret);
}

RSSL_VA_API RsslRet rsslReactorGetDebugInfo(RsslReactor* pReactor, RsslReactorDebugInfo* pDebugInfo, RsslErrorInfo* pError)
{
	RsslRet ret = RSSL_RET_SUCCESS;
	RsslReactorImpl* pReactorImpl = (RsslReactorImpl*)pReactor;
	RsslReactorDebugInfoImpl* pReactorDebugInfoImpl = NULL;
	RsslUInt32 outputSize = 0;

	if (pError == NULL)
		return RSSL_RET_FAILURE;

	if (pReactor == NULL)
	{
		rsslSetErrorInfo(pError, RSSL_EIC_FAILURE, RSSL_RET_FAILURE, __FILE__, __LINE__,
			"The pReactor parameter is NULL");

		return RSSL_RET_FAILURE;
	}

	if (pReactorImpl->state != RSSL_REACTOR_ST_ACTIVE)
	{
		rsslSetErrorInfo(pError, RSSL_EIC_FAILURE, RSSL_RET_INVALID_ARGUMENT, __FILE__, __LINE__, "Reactor is shutting down.");
		return RSSL_RET_FAILURE;
	}

	if (pDebugInfo == NULL)
	{
		rsslSetErrorInfo(pError, RSSL_EIC_FAILURE, RSSL_RET_FAILURE, __FILE__, __LINE__,
			"The pDebugInfo parameter is NULL");

		return RSSL_RET_FAILURE;
	}

	rsslClearBuffer(&pDebugInfo->debugInfoBuffer);

	if (!isReactorDebugEnabled(pReactorImpl))
	{
		rsslSetErrorInfo(pError, RSSL_EIC_FAILURE, RSSL_RET_FAILURE, __FILE__, __LINE__,
			"Per reactor debuging is not enabled.");

		return RSSL_RET_FAILURE;
	}

	pReactorDebugInfoImpl = pReactorImpl->pReactorDebugInfo;

	if ((ret = reactorLockInterface(pReactorImpl, RSSL_TRUE, pError)) != RSSL_RET_SUCCESS)
		return ret;

	RSSL_MUTEX_LOCK(&pReactorImpl->debugLock);

	outputSize = pReactorDebugInfoImpl->debuggingMemory.length - pReactorDebugInfoImpl->remainingLength;

	if (outputSize != 0)
	{
		memset(pReactorDebugInfoImpl->outputMemory.data, 0, pReactorImpl->debugBufferSize);
		memcpy(pReactorDebugInfoImpl->outputMemory.data, pReactorDebugInfoImpl->debuggingMemory.data, outputSize);

		/* Transfer to the parameter. */
		pDebugInfo->debugInfoBuffer.data = pReactorDebugInfoImpl->outputMemory.data;
		pDebugInfo->debugInfoBuffer.length = outputSize;

		/* Resets the debuging memory portiion */
		pReactorDebugInfoImpl->pCurrentWrite = pReactorDebugInfoImpl->debuggingMemory.data;
		pReactorDebugInfoImpl->remainingLength = pReactorDebugInfoImpl->debuggingMemory.length;
	}
	else
	{
		pDebugInfo->debugInfoBuffer.data = REACTOR_EMPTY_DEBUG_INFO.data;
		pDebugInfo->debugInfoBuffer.length = 0;
	}

	RSSL_MUTEX_UNLOCK(&pReactorImpl->debugLock);

	return (reactorUnlockInterface(pReactorImpl), RSSL_RET_SUCCESS);
}

RSSL_VA_API RsslRet rsslReactorGetDebugLevel(RsslReactor *pReactor, RsslUInt32 *debugLevel, RsslErrorInfo* pError)
{
	RsslReactorImpl *pReactorImpl = (RsslReactorImpl*)pReactor;

	if (pReactor == NULL)
	{
		rsslSetErrorInfo(pError, RSSL_EIC_FAILURE, RSSL_RET_INVALID_ARGUMENT, __FILE__, __LINE__, "Income pReactor is NULL");
		return RSSL_RET_FAILURE;
	}

	if (pError == NULL)
	{
		rsslSetErrorInfo(pError, RSSL_EIC_FAILURE, RSSL_RET_INVALID_ARGUMENT, __FILE__, __LINE__, "Income pError is NULL");
		return RSSL_RET_FAILURE;
	}

	if (debugLevel == NULL)
	{
		rsslSetErrorInfo(pError, RSSL_EIC_FAILURE, RSSL_RET_INVALID_ARGUMENT, __FILE__, __LINE__, "Income debugLevel is NULL.");
		return RSSL_RET_FAILURE;
	}

	if (reactorLockInterface(pReactorImpl, RSSL_TRUE, pError) != RSSL_RET_SUCCESS)
		return RSSL_RET_INVALID_ARGUMENT;

	if (pReactorImpl->state != RSSL_REACTOR_ST_ACTIVE)
	{
		rsslSetErrorInfo(pError, RSSL_EIC_FAILURE, RSSL_RET_INVALID_ARGUMENT, __FILE__, __LINE__, "Reactor is shutting down.");
		return (reactorUnlockInterface(pReactorImpl), RSSL_RET_INVALID_ARGUMENT);
	}

	*debugLevel = pReactorImpl->debugLevel;

	return (reactorUnlockInterface(pReactorImpl), RSSL_RET_SUCCESS);
}

RSSL_VA_API RsslRet rsslReactorSetDebugLevel(RsslReactor *pReactor, RsslUInt32 debugLevel, RsslErrorInfo* pError)
{
	RsslReactorImpl *pReactorImpl = (RsslReactorImpl*)pReactor;

	if (pReactor == NULL)
	{
		rsslSetErrorInfo(pError, RSSL_EIC_FAILURE, RSSL_RET_INVALID_ARGUMENT, __FILE__, __LINE__, "Income pReactor is NULL");
		return RSSL_RET_FAILURE;
	}

	if (pError == NULL)
	{
		rsslSetErrorInfo(pError, RSSL_EIC_FAILURE, RSSL_RET_INVALID_ARGUMENT, __FILE__, __LINE__, "Income pError is NULL");
		return RSSL_RET_FAILURE;
	}

	if (reactorLockInterface(pReactorImpl, RSSL_TRUE, pError) != RSSL_RET_SUCCESS)
		return RSSL_RET_INVALID_ARGUMENT;

	if (pReactorImpl->state != RSSL_REACTOR_ST_ACTIVE)
	{
		rsslSetErrorInfo(pError, RSSL_EIC_FAILURE, RSSL_RET_INVALID_ARGUMENT, __FILE__, __LINE__, "Reactor is shutting down.");
		return (reactorUnlockInterface(pReactorImpl), RSSL_RET_INVALID_ARGUMENT);
	}

	(((RsslReactorImpl*)pReactor)->debugLevel) = debugLevel;

	return (reactorUnlockInterface(pReactorImpl), RSSL_RET_SUCCESS);
}


RsslRet _initReactorAndChannelDebugInfo(RsslReactorImpl *pReactorImpl, RsslReactorChannelImpl *pReactorChannel, RsslErrorInfo *pError)
{
	RsslRet ret = RSSL_RET_SUCCESS;

	if (pReactorImpl->pReactorDebugInfo == NULL)
	{
		if ((ret =_initReactorDebugInfo(pReactorImpl, pError)) != RSSL_RET_SUCCESS) return ret;
	}

	if (pReactorChannel->pChannelDebugInfo == NULL)
	{
		if ((ret = _initReactorChannelDebugInfo(pReactorImpl, pReactorChannel, pError)) != RSSL_RET_SUCCESS) return ret;
	}

	return ret;
}

RsslRet _initReactorDebugInfo(RsslReactorImpl *pReactorImpl, RsslErrorInfo *pError)
{
	RSSL_MUTEX_LOCK(&pReactorImpl->debugLock);

	pReactorImpl->pReactorDebugInfo = (RsslReactorDebugInfoImpl*)malloc(sizeof(RsslReactorDebugInfoImpl));

	if (!pReactorImpl->pReactorDebugInfo)
	{
		RSSL_MUTEX_UNLOCK(&pReactorImpl->debugLock);
		rsslSetErrorInfo(pError, RSSL_EIC_FAILURE, RSSL_RET_FAILURE, __FILE__, __LINE__, "Failed to create reactor debugging information.");
		return RSSL_RET_FAILURE;
	}

	rsslClearReactorDebugInfoImpl(pReactorImpl->pReactorDebugInfo);

	pReactorImpl->pReactorDebugInfo->debuggingMemory.data = (char*)malloc(pReactorImpl->debugBufferSize);

	if (!pReactorImpl->pReactorDebugInfo->debuggingMemory.data)
	{
		RSSL_MUTEX_UNLOCK(&pReactorImpl->debugLock);
		rsslSetErrorInfo(pError, RSSL_EIC_FAILURE, RSSL_RET_FAILURE, __FILE__, __LINE__, "Failed to create reactor debugging information.");
		return RSSL_RET_FAILURE;
	}

	pReactorImpl->pReactorDebugInfo->debuggingMemory.length = pReactorImpl->debugBufferSize;
	pReactorImpl->pReactorDebugInfo->remainingLength = pReactorImpl->pReactorDebugInfo->debuggingMemory.length;
	pReactorImpl->pReactorDebugInfo->pCurrentWrite = pReactorImpl->pReactorDebugInfo->debuggingMemory.data;
	memset(pReactorImpl->pReactorDebugInfo->debuggingMemory.data, 0, pReactorImpl->debugBufferSize);

	pReactorImpl->pReactorDebugInfo->outputMemory.data = (char*)malloc(pReactorImpl->debugBufferSize);

	if (!pReactorImpl->pReactorDebugInfo->outputMemory.data)
	{
		RSSL_MUTEX_UNLOCK(&pReactorImpl->debugLock);
		rsslSetErrorInfo(pError, RSSL_EIC_FAILURE, RSSL_RET_FAILURE, __FILE__, __LINE__, "Failed to create reactor debugging information.");
		return RSSL_RET_FAILURE;
	}

	RSSL_MUTEX_UNLOCK(&pReactorImpl->debugLock);

	return RSSL_RET_SUCCESS;
}

RsslRet _initReactorChannelDebugInfo(RsslReactorImpl *pReactorImpl, RsslReactorChannelImpl *pReactorChannel, RsslErrorInfo *pError)
{
	RSSL_MUTEX_LOCK(&pReactorImpl->debugLock);

	pReactorChannel->pChannelDebugInfo = (RsslReactorChannelDebugInfo*)malloc(sizeof(RsslReactorChannelDebugInfo));

	if (!pReactorChannel->pChannelDebugInfo)
	{
		RSSL_MUTEX_UNLOCK(&pReactorImpl->debugLock);
		rsslSetErrorInfo(pError, RSSL_EIC_FAILURE, RSSL_RET_FAILURE, __FILE__, __LINE__, "Failed to create channel debugging information.");
		return RSSL_RET_FAILURE;
	}

	rsslClearReactorChannelDebugInfo(pReactorChannel->pChannelDebugInfo);
	pReactorChannel->pChannelDebugInfo->debugInfoState |= RSSL_RC_DEBUG_INFO_ACCEPT_CHANNEL;

	RSSL_MUTEX_UNLOCK(&pReactorImpl->debugLock);

	return RSSL_RET_SUCCESS;
}

void _cleanupReactorChannelDebugInfo(RsslReactorChannelImpl *pReactorChannel)
{
	if (pReactorChannel && pReactorChannel->pChannelDebugInfo)
	{
		free(pReactorChannel->pChannelDebugInfo);
		pReactorChannel->pChannelDebugInfo = NULL;
	}
}

RsslRet _getCurrentTimestamp(RsslBuffer* pTimeStamp)
{
	long hour = 0,
		min = 0,
		sec = 0,
		msec = 0;

#if defined(WIN32)
	struct _timeb	_time;
	_ftime(&_time);
	sec = (long)(_time.time - 60 * (_time.timezone - _time.dstflag * 60));
	min = sec / 60 % 60;
	hour = sec / 3600 % 24;
	sec = sec % 60;
	msec = _time.millitm;
#elif defined(LINUX)
	/* localtime must be used to get the correct system time. */
	struct tm stamptime;
	time_t currTime;
	currTime = time(NULL);
	stamptime = *localtime_r(&currTime, &stamptime);
	sec = stamptime.tm_sec;
	min = stamptime.tm_min;
	hour = stamptime.tm_hour;

	/* localtime, however, does not give us msec. */
	struct timeval tv;
	gettimeofday(&tv, NULL);
	msec = tv.tv_usec / 1000;
#endif

	if(!pTimeStamp)
		return RSSL_RET_FAILURE;

	/* The time stamp length is 38. */
	pTimeStamp->length = (RsslUInt32)sprintf(pTimeStamp->data, "[Reactor DEBUG Time: %ld:%02ld:%02ld:%03ld -> ",
		hour,
		min,
		sec,
		msec);

	return RSSL_RET_SUCCESS;
}

void _writeDebugInfo(RsslReactorImpl* pReactorImpl, const char* debugText, ...)
{
	RsslBuffer timeStamp;
	va_list fmtArgs;
	int writeLength;
	RsslReactorDebugInfoImpl* pReactorDebugInfoImpl;

	if (!pReactorImpl || !pReactorImpl->pReactorDebugInfo)
		return;

	pReactorDebugInfoImpl = pReactorImpl->pReactorDebugInfo;

	/* Ensure that there is enough space to write a debugging message. */
	if (pReactorDebugInfoImpl->remainingLength < 155 || !debugText)
		return;

	rsslClearBuffer(&timeStamp);

	RSSL_MUTEX_LOCK(&pReactorImpl->debugLock);

	timeStamp.data = pReactorDebugInfoImpl->pCurrentWrite;

	_getCurrentTimestamp(&timeStamp);

	pReactorDebugInfoImpl->pCurrentWrite += timeStamp.length;
	pReactorDebugInfoImpl->remainingLength -= timeStamp.length;

	va_start(fmtArgs, debugText);
	writeLength = vsnprintf(pReactorDebugInfoImpl->pCurrentWrite, pReactorDebugInfoImpl->remainingLength, debugText, fmtArgs);
	va_end(fmtArgs);

	pReactorDebugInfoImpl->pCurrentWrite += writeLength;
	pReactorDebugInfoImpl->remainingLength -= writeLength;

	RSSL_MUTEX_UNLOCK(&pReactorImpl->debugLock);
}
