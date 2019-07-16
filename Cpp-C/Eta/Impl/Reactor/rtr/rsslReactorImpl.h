/*
 * This source code is provided under the Apache 2.0 license and is provided
 * AS IS with no warranty or guarantee of fit for purpose.  See the project's 
 * LICENSE.md for details. 
 * Copyright Thomson Reuters 2019. All rights reserved.
*/

#ifndef _RTR_RSSL_REACTOR_IMPL_H
#define _RTR_RSSL_REACTOR_IMPL_H

#include "rtr/rsslNotifier.h"
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
#include "rtr/rsslRestClientImpl.h"
#include "rtr/rtratomic.h"

#ifdef WIN32
#include <windows.h>
#else
#include <sys/time.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif

typedef struct _RsslReactorImpl RsslReactorImpl;

typedef struct _RsslReactorOAuthCredentialRenewalImpl RsslReactorOAuthCredentialRenewalImpl;

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

/* RsslReactorChannelInfoImplState
* - Represents states for token management and requesting service discovery */
typedef enum
{
	RSSL_RC_CHINFO_IMPL_ST_INVALID_CONNECTION_TYPE = -5,
	RSSL_RC_CHINFO_IMPL_ST_MEM_ALLOCATION_FAILURE = -4,
	RSSL_RC_CHINFO_IMPL_ST_PARSE_RESP_FAILURE = -3,
	RSSL_RC_CHINFO_IMPL_ST_REQUEST_FAILURE = -2,
	RSSL_RC_CHINFO_IMPL_ST_BUFFER_TOO_SMALL = -1,
	RSSL_RC_CHINFO_IMPL_ST_INIT = 0,
	RSSL_RC_CHINFO_IMPL_ST_REQ_SENSITIVE_INFO = 1,
	RSSL_RC_CHINFO_IMPL_ST_WAITING_TO_REQ_AUTH_TOKEN = 2,
	RSSL_RC_CHINFO_IMPL_ST_REQ_AUTH_TOKEN = 3,
	RSSL_RC_CHINFO_IMPL_ST_RECEIVED_AUTH_TOKEN = 4,
	RSSL_RC_CHINFO_IMPL_ST_QUERYING_SERVICE_DISOVERY = 5, 
	RSSL_RC_CHINFO_IMPL_ST_ASSIGNED_HOST_PORT = 6, 
} RsslReactorChannelInfoImplState;

/* RsslReactorConnectInfoImpl
* - Handles a channel information including token management */
typedef struct
{
	RsslReactorConnectInfo base;

	RsslInt32						reissueTokenAttemptLimit; /* Keeping track of token renewal attempt */

	RsslReactorChannelInfoImplState	reactorChannelInfoImplState; /* Keeping track the state of this session */
	RsslReactorTokenMgntEventType	reactorTokenMgntEventType; /* Specify an event type for sending to the Reactor */

} RsslReactorConnectInfoImpl;

RTR_C_INLINE void rsslClearReactorConnectInfoImpl(RsslReactorConnectInfoImpl* pReactorConnectInfoImpl)
{
	memset(pReactorConnectInfoImpl, 0, sizeof(RsslReactorConnectInfoImpl));
	rsslClearReactorConnectInfo(&pReactorConnectInfoImpl->base);
}

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
	RsslNotifierEvent *pNotifierEvent;
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
	RsslNotifierEvent *pWorkerNotifierEvent;

	/* Reconnection logic */
	RsslInt32 reconnectMinDelay;
	RsslInt32 reconnectMaxDelay;
	RsslInt32 reconnectDelay;
	RsslInt32 reconnectAttemptLimit;
	RsslInt32 reconnectAttemptCount;
	RsslInt64 lastReconnectAttemptMs;
	
	RsslInt32 connectionListCount;
	RsslInt32 connectionListIter;
	RsslReactorConnectInfoImpl *connectionOptList;
	TunnelManager *pTunnelManager;

	/* Support session management and EDP-RT service discovery. */
	RsslBool supportSessionMgnt;
	RsslReactorAuthTokenInfo	tokenInformation;
	RsslBuffer				tokenInformationBuffer;
	RsslInt					nextExpiresTime; /* the next expires time in millisecond */
	RsslBool				resendFromFailure; /* Indicates to resend the request after a response failure */
	rtr_atomic_val			sendTokenRequest;
	RsslBuffer				rsslAccessTokenRespBuffer;
	RsslBuffer				rsslServiceDiscoveryRespBuffer;
	RsslBuffer				rsslPostDataBodyBuf;
	RsslUInt32				httpStausCode; /* the latest HTTP status code */

	/* This is original login request information */
	RsslBuffer				userName;
	RsslUInt32				flags;
	RsslUInt8				userNameType;

	/* This is used for OAuth credential renewal */
	RsslReactorOAuthCredentialRenewalImpl *pOAuthCredentialRenewalImpl;

	RsslUInt32 connectionDebugFlags;	/*!< Set of RsslDebugFlags for calling the user-set debug callbacks */

} RsslReactorChannelImpl;

RTR_C_INLINE void rsslClearReactorChannelImpl(RsslReactorImpl *pReactorImpl, RsslReactorChannelImpl *pInfo)
{
	memset(pInfo, 0, sizeof(RsslReactorChannelImpl));
	pInfo->pParentReactor = pReactorImpl;
	pInfo->nextExpireTime = RCIMPL_TIMER_UNSET;
	pInfo->lastRequestedExpireTime = RCIMPL_TIMER_UNSET;
}

/* RsslReactorOAuthCredentialRenewalImpl
 * - Handles allocated memory length and the association with RsslReactorChannelImpl for RsslReactorOAuthCredentialRenewal */
struct _RsslReactorOAuthCredentialRenewalImpl
{
	RsslReactorOAuthCredentialRenewal reactorOAuthCredentialRenewal;
	RsslReactorChannelImpl			*pParentChannel; /* Specify the owner of this type if any */

	/* Keeps track of memory allocation length */
	size_t		memoryLength;

	/* The following member variables is used only when submitting without a channel */
	RsslBuffer					rsslAccessTokenRespBuffer;
	RsslReactorImpl				*pRsslReactorImpl;
	RsslReactorAuthTokenInfo	tokenInformation;
	RsslBuffer					tokenInformationBuffer;
	RsslBuffer					rsslPostDataBodyBuf;
	RsslReactorAuthTokenEventCallback	*pAuthTokenEventCallback;
	RsslUInt32					httpStatusCode;
	RsslBuffer					proxyHostName;
	RsslBuffer					proxyPort;
	RsslBuffer					proxyUserName;
	RsslBuffer					proxyPasswd;
	RsslBuffer					proxyDomain;
};

RTR_C_INLINE void rsslClearReactorOAuthCredentialRenewalImpl(RsslReactorOAuthCredentialRenewalImpl *pInfo)
{
	memset(pInfo, 0, sizeof(RsslReactorOAuthCredentialRenewalImpl));
}

RTR_C_INLINE RsslRet _rsslChannelCopyConnectionList(RsslReactorChannelImpl *pReactorChannel, RsslReactorConnectOptions *pOpts, 
															RsslBool *enableSessionMgnt)
{
	RsslConnectOptions *destOpts, *sourceOpts;
	RsslUInt32 i, j, k;

	if(pOpts->connectionCount != 0)
	{
		pReactorChannel->connectionOptList = (RsslReactorConnectInfoImpl*)malloc(pOpts->connectionCount*sizeof(RsslReactorConnectInfoImpl));

		if(!pReactorChannel->connectionOptList)
		{
			return RSSL_RET_FAILURE;
		}

		(*enableSessionMgnt) = RSSL_FALSE;
		for(i = 0; i < pOpts->connectionCount; i++)
		{
			rsslClearReactorConnectInfoImpl(&pReactorChannel->connectionOptList[i]);

			pReactorChannel->connectionOptList[i].base.location.length = (RsslUInt32)strlen(pOpts->reactorConnectionList[i].location.data);
			pReactorChannel->connectionOptList[i].base.location.data = (char*)malloc((size_t)pReactorChannel->connectionOptList[i].base.location.length + (size_t)1);
			if (pReactorChannel->connectionOptList[i].base.location.data == 0)
			{
				for (k = 0; k < i; k++)
				{
					free(pReactorChannel->connectionOptList[k].base.location.data);
				}
				
				free(pReactorChannel->connectionOptList);
				pReactorChannel->connectionOptList = NULL;
				return RSSL_RET_FAILURE;
			}

			memset(pReactorChannel->connectionOptList[i].base.location.data, 0, pReactorChannel->connectionOptList[i].base.location.length + 1);
			strncpy(pReactorChannel->connectionOptList[i].base.location.data, pOpts->reactorConnectionList[i].location.data, 
							pReactorChannel->connectionOptList[i].base.location.length);

			pReactorChannel->connectionOptList[i].base.initializationTimeout = pOpts->reactorConnectionList[i].initializationTimeout;
			pReactorChannel->connectionOptList[i].base.enableSessionManagement = pOpts->reactorConnectionList[i].enableSessionManagement;
			pReactorChannel->connectionOptList[i].base.pAuthTokenEventCallback = pOpts->reactorConnectionList[i].pAuthTokenEventCallback;
			pReactorChannel->connectionOptList[i].base.reissueTokenAttemptLimit = 
				pOpts->reactorConnectionList[i].reissueTokenAttemptLimit < -1  ? -1 : pOpts->reactorConnectionList[i].reissueTokenAttemptLimit;
			pReactorChannel->connectionOptList[i].reissueTokenAttemptLimit = pReactorChannel->connectionOptList[i].base.reissueTokenAttemptLimit;

			if (!(*enableSessionMgnt))
				(*enableSessionMgnt) = pReactorChannel->connectionOptList[i].base.enableSessionManagement;

			destOpts = &pReactorChannel->connectionOptList[i].base.rsslConnectOptions;
			sourceOpts = &pOpts->reactorConnectionList[i].rsslConnectOptions;

			if(rsslDeepCopyConnectOpts(destOpts, sourceOpts) != RSSL_RET_SUCCESS)
			{
				for(j = 0; j <= i; j++)
				{
					rsslFreeConnectOpts(&pReactorChannel->connectionOptList[j].base.rsslConnectOptions);
				}
				free(pReactorChannel->connectionOptList);
				pReactorChannel->connectionOptList = NULL;
				return RSSL_RET_FAILURE;
			}
		}
		pReactorChannel->connectionListCount = pOpts->connectionCount;
	}
	else
	{
		pReactorChannel->connectionOptList = (RsslReactorConnectInfoImpl*)malloc(sizeof(RsslReactorConnectInfoImpl));

		if(!pReactorChannel->connectionOptList)
		{
			return RSSL_RET_FAILURE;
		}

		rsslClearReactorConnectInfoImpl(pReactorChannel->connectionOptList);

		if(rsslDeepCopyConnectOpts(&(pReactorChannel->connectionOptList->base.rsslConnectOptions), &pOpts->rsslConnectOptions) != RSSL_RET_SUCCESS)
		{
			rsslFreeConnectOpts(&pReactorChannel->connectionOptList->base.rsslConnectOptions);

			free(pReactorChannel->connectionOptList);
			pReactorChannel->connectionOptList = NULL;
			return RSSL_RET_FAILURE;
		}

		pReactorChannel->connectionOptList->base.initializationTimeout = pOpts->initializationTimeout;
		pReactorChannel->connectionOptList->base.enableSessionManagement = RSSL_FALSE;
		pReactorChannel->connectionListCount = 1;
	}

	return RSSL_RET_SUCCESS;
}

/* All RsslReactorChannelImpl's member variables must be reset properly in rsslResetReactorChannel
   as the RsslReactorChannelImpl can be reused from the channel pool */
RTR_C_INLINE RsslRet _rsslChannelFreeConnectionList(RsslReactorChannelImpl *pReactorChannel)
{
	int i;

	if(pReactorChannel->connectionOptList)
	{
		for(i = 0; i < pReactorChannel->connectionListCount; i++)
		{
			/* Cleaning memory relating to the session management */
			if (pReactorChannel->connectionOptList[i].base.enableSessionManagement)
			{
				free(pReactorChannel->connectionOptList[i].base.location.data);
			}

			rsslFreeConnectOpts(&pReactorChannel->connectionOptList[i].base.rsslConnectOptions);
		}

		free(pReactorChannel->rsslPostDataBodyBuf.data);
		free(pReactorChannel->rsslServiceDiscoveryRespBuffer.data);
		free(pReactorChannel->rsslAccessTokenRespBuffer.data);
		free(pReactorChannel->tokenInformationBuffer.data);
		free(pReactorChannel->pOAuthCredentialRenewalImpl);

		if (pReactorChannel->channelRole.base.roleType == RSSL_RC_RT_OMM_CONSUMER)
		{
			free(pReactorChannel->channelRole.ommConsumerRole.pOAuthCredential);
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
	pReactorChannel->connectionDebugFlags = 0;
	pReactorChannel->reactorChannel.socketId = (RsslSocket)REACTOR_INVALID_SOCKET;
	pReactorChannel->reactorChannel.oldSocketId = (RsslSocket)REACTOR_INVALID_SOCKET;

	/* Reset all buffers for the session management */
	rsslClearBuffer(&pReactorChannel->rsslPostDataBodyBuf);
	rsslClearBuffer(&pReactorChannel->rsslServiceDiscoveryRespBuffer);
	rsslClearBuffer(&pReactorChannel->rsslAccessTokenRespBuffer);
	rsslClearBuffer(&pReactorChannel->tokenInformationBuffer);
	pReactorChannel->pOAuthCredentialRenewalImpl = NULL;

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
	RsslQueue disposableRestHandles; /* Rest handles that needs to be cleanup */

	RsslNotifier *pNotifier; /* Notifier for workerQueue and channels */
	RsslNotifierEvent *pQueueNotifierEvent;	/* Notification for workerQueue */

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

	RsslNotifier *pNotifier; /* Notifier for reactorEventQueue and channels */
	RsslNotifierEvent *pQueueNotifierEvent; /* Notification for reactorEventQueue */

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

	/* For EDP token management and service discovery */
	RsslBuffer			serviceDiscoveryURL;
	RsslBuffer			tokenServiceURL;
	RsslBuffer			accessTokenRespBuffer;
	RsslBuffer			tokenInformationBuffer;
	RsslBuffer			serviceDiscoveryRespBuffer;
	RsslBuffer			argumentsAndHeaders;
	RsslRestClient*			pRestClient;
	RsslBool			registeredRsslRestClientEventFd;
	RsslNotifierEvent *pWorkerNotifierEvent; /* This is used for RsslRestClient to receive notification */
	RsslBuffer			restServiceEndpointRespBuf; /* This is memory allocation for RsslRestServiceEndpointResp */
	RsslRestServiceEndpointResp	restServiceEndpointResp; /* This is used for querying service discovery by users */
	RsslDouble			tokenReissueRatio; /* User defined ration multiply with the expires_in field to retrieve and reissue the access token */
};

RTR_C_INLINE void rsslClearReactorImpl(RsslReactorImpl *pReactorImpl)
{
	memset(pReactorImpl, 0, sizeof(RsslReactorImpl));
}

void _assignConnectionArgsToRequestArgs(RsslConnectOptions *pConnOptions, RsslRestRequestArgs* pRestRequestArgs);

RsslRestRequestArgs* _reactorCreateRequestArgsForPassword(RsslBuffer *pTokenServiceURL, RsslBuffer *pUserName, RsslBuffer *password, RsslBuffer *pNewPassword,
	RsslBuffer *pClientID, RsslBuffer *pClientSecret, RsslBuffer *pTokenScope, RsslBuffer *pPostDataBodyBuf, void *pUserSpecPtr, RsslErrorInfo *pError);

RsslRestRequestArgs* _reactorCreateRequestArgsForServiceDiscovery(RsslBuffer *pServiceDiscoveryURL, RsslReactorDiscoveryTransportProtocol transport,
																RsslReactorDiscoveryDataFormatProtocol dataFormat, RsslBuffer *pTokenType, RsslBuffer *pAccessToken,
																RsslBuffer *pArgsAndHeaderBuf, void *pUserSpecPtr, RsslErrorInfo* pError);

RsslRet _reactorGetAccessTokenAndServiceDiscovery(RsslReactorChannelImpl* pReactorChannelImpl, RsslBool *queryConnectInfo, RsslErrorInfo* pError);

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
