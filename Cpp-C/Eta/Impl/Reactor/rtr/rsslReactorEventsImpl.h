/*
 * This source code is provided under the Apache 2.0 license and is provided
 * AS IS with no warranty or guarantee of fit for purpose.  See the project's 
 * LICENSE.md for details. 
 * Copyright (C) 2021 Refinitiv. All rights reserved.
*/

#ifndef _RTR_RSSL_EVENTS_INT_H
#define _RTR_RSSL_EVENTS_INT_H

#include "rtr/rsslReactorEvents.h"
#include "rtr/rsslQueue.h"
#include "rtr/rsslReactorTokenMgntImpl.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum 
{
	RSSL_RCIMPL_ET_TUNNEL_STREAM_LISTENER = 2,		/* TunnelStream Listener event */
	RSSL_RCIMPL_ET_CHANNEL = 1,		/* Channel event */
	RSSL_RCIMPL_ET_INIT = 0,		/* Unknown event. */
	RSSL_RCIMPL_ET_REACTOR = -1,	/* Reactor-related event */
	RSSL_RCIMPL_ET_FLUSH = -2,		/* Flushing needs to start or has finished */
	RSSL_RCIMPL_ET_TIMER = -3,		/* A timer event. */
	RSSL_RCIMPL_ET_TOKEN_MGNT = -4,	/* Token management event on Login stream */
	RSSL_RCIMPL_ET_CREDENTIAL_RENEWAL = -5, /* OAuth credential renewal event */
	RSSL_RCIMPL_ET_PING = -6, /* Ping event for channel statistics */
	RSSL_RCIMPL_ET_TOKEN_SESSION_MGNT = -7,	/* For handling token session */
	RSSL_RCIMPL_ET_WARM_STANDBY = -8, /* For handling warm standby feature */
	RSSL_RCIMPL_ET_LOGGING = -9,		/* Logging message event */
	RSSL_RCIMPL_ET_LOGIN_RENEWAL = -10, /* RDM Login Msg credential renewal event */
	RSSL_RCIMPL_ET_REST_REQ_RESP = -11 /* REST request and response event for service discovery */
} RsslReactorEventImplType;

typedef struct
{
	RsslQueueLink eventQueueLink;
	RsslReactorEventImplType eventType;
} RsslReactorEventImplBase;

typedef enum
{
	RSSL_RCIMPL_CET_NEW_CHANNEL = -1,
	RSSL_RCIMPL_CET_CLOSE_CHANNEL = -2,
	RSSL_RCIMPL_CET_CLOSE_CHANNEL_ACK = -3,
	RSSL_RCIMPL_CET_DISPATCH_WL = -4,
	RSSL_RCIMPL_CET_DISPATCH_TUNNEL_STREAM = -5,
	RSSL_RCIMPL_CET_CLOSE_WARMSTANDBY_CHANNEL = -6,
} RsslReactorChannelEventImplType;

typedef enum
{
	RSSL_RCIMPL_FET_INIT = 0,
	RSSL_RCIMPL_FET_START_FLUSH = -1,
	RSSL_RCIMPL_FET_FLUSH_DONE = -2
} RsslReactorFlushEventType;

typedef struct
{
	RsslReactorEventImplBase base;
	RsslReactorFlushEventType flushEventType;
	RsslReactorChannel *pReactorChannel;
} RsslReactorFlushEvent;

RTR_C_INLINE void rsslInitFlushEvent(RsslReactorFlushEvent *pEvent)
{
	memset(pEvent, 0, sizeof(RsslReactorFlushEvent));
	pEvent->base.eventType = RSSL_RCIMPL_ET_FLUSH;
}

/* Time that indicates that the reactor's timer is not currently in use.  */
static const RsslInt64 RCIMPL_TIMER_UNSET = 0x7fffffffffffffffLL;

typedef struct
{
	RsslReactorEventImplBase	base;
	RsslInt64					expireTime;
	RsslReactorChannel			*pReactorChannel;
} RsslReactorTimerEvent;

RTR_C_INLINE void rsslInitTimerEvent(RsslReactorTimerEvent *pEvent)
{
	memset(pEvent, 0, sizeof(RsslReactorTimerEvent));
	pEvent->base.eventType = RSSL_RCIMPL_ET_TIMER;
}


typedef struct
{
	RsslReactorEventImplBase base;
	RsslReactorChannelEvent channelEvent;
	RsslBool isConnectFailure; /* Indicated by worker for channel-down events. Indicates whether the failure occurred while
								* attempting to connect/initialize the channel. */
} RsslReactorChannelEventImpl;

RTR_C_INLINE void rsslClearReactorChannelEventImpl(RsslReactorChannelEventImpl *pEvent)
{
	memset(pEvent, 0, sizeof(RsslReactorChannelEventImpl));
	pEvent->base.eventType = RSSL_RCIMPL_ET_CHANNEL;
}

typedef enum
{
	RSSL_RCIMPL_STET_INIT = 0,
	RSSL_RCIMPL_STET_SHUTDOWN = -1,
	RSSL_RCIMPL_STET_DESTROY = -2
} RsslReactorStateEventType;

typedef struct
{
	RsslReactorEventImplBase base;
	RsslReactorStateEventType reactorEventType;
	RsslErrorInfo *pErrorInfo;
} RsslReactorStateEvent;

RTR_C_INLINE void rsslClearReactorEvent(RsslReactorStateEvent *pEvent)
{
	memset(pEvent, 0, sizeof(RsslReactorStateEvent));
	pEvent->base.eventType = RSSL_RCIMPL_ET_REACTOR;
}

typedef enum
{
	RSSL_RCIMPL_TKET_INIT = 0,
	RSSL_RCIMPL_TKET_REISSUE = -1,
	RSSL_RCIMPL_TKET_REISSUE_NO_REFRESH = -2,
	RSSL_RCIMPL_TKET_RESP_FAILURE = -3,
	RSSL_RCIMPL_TKET_SUBMIT_LOGIN_MSG = -4,
	RSSL_RCIMPL_TKET_CHANNEL_WARNING = -5,
	RSSL_RCIMPL_TKET_RENEW_TOKEN = -6,
	RSSL_RCIMPL_TKET_SUBMIT_LOGIN_MSG_NEW_CONNECTION = -7,
	RSSL_RCIMPL_TKET_REISSUE_NEW_CONNECTION = -8
} RsslReactorTokenMgntEventType;

typedef struct
{
	RsslReactorEventImplBase base;
	RsslReactorTokenMgntEventType reactorTokenMgntEventType;
	RsslReactorChannel *pReactorChannel;
	RsslReactorOAuthCredentialRenewal *pOAuthCredentialRenewal;
	RsslReactorAuthTokenEvent reactorAuthTokenEvent;
	RsslReactorAuthTokenEventCallback *pAuthTokenEventCallback;
	RsslReactorTokenSessionImpl *pTokenSessionImpl;
	RsslReactorErrorInfoImpl *pReactorErrorInfoImpl;
} RsslReactorTokenMgntEvent;

RTR_C_INLINE void rsslClearReactorTokenMgntEvent(RsslReactorTokenMgntEvent *pEvent)
{
	memset(pEvent, 0, sizeof(RsslReactorTokenMgntEvent));
	pEvent->base.eventType = RSSL_RCIMPL_ET_TOKEN_MGNT; 
}

typedef enum
{
	RSSL_RCIMPL_CRET_MEM_ALLOC_FAILED = -1,
	RSSL_RCIMPL_CRET_INIT = 0,
	RSSL_RCIMPL_CRET_AUTH_REQ_WITH_PASSWORD = 0x01,			/* Matches RSSL_ROC_RT_RENEW_TOKEN_WITH_PASSWORD */
	RSSL_RCIMPL_CRET_AUTH_REQ_WITH_PASSWORD_CHANGE = 0x02,  /* Matches RSSL_ROC_RT_RENEW_TOKEN_WITH_PASSWORD_CHANGE */
	RSSL_RCIMPL_CRET_RENEWAL_CALLBACK = 0x04,
	RSSL_RCIMPL_CRET_MEMORY_DEALLOCATION = 0x08
} RsslReactorOAuthCredentialRenewalEventType;

typedef struct
{
	RsslReactorEventImplBase base;
	RsslReactorOAuthCredentialRenewal *pOAuthCredentialRenewal;
	RsslReactorOAuthCredentialRenewalEventType	reactorCredentialRenewalEventType;
	RsslReactorOAuthCredentialEvent reactorOAuthCredentialEvent;
	RsslReactorOAuthCredentialEventCallback *pOAuthCredentialEventCallback;
	RsslReactorTokenSessionImpl *pTokenSessionImpl;
	RsslReactorErrorInfoImpl *pReactorErrorInfoImpl;
} RsslReactorCredentialRenewalEvent;

RTR_C_INLINE void rsslClearReactorCredentialRenewalEvent(RsslReactorCredentialRenewalEvent *pEvent)
{
	memset(pEvent, 0, sizeof(RsslReactorCredentialRenewalEvent));
	pEvent->base.eventType = RSSL_RCIMPL_ET_CREDENTIAL_RENEWAL;
}

typedef struct
{
	RsslReactorEventImplBase base;
	RsslReactorLoginRequestMsgCredential* pRequest;
	RsslReactorChannel* pReactorChannel;
	RsslReactorErrorInfoImpl* pReactorErrorInfoImpl;
} RsslReactorLoginCredentialRenewalEvent;

RTR_C_INLINE void rsslClearReactorLoginCredentialRenewalEvent(RsslReactorLoginCredentialRenewalEvent* pEvent)
{
	memset(pEvent, 0, sizeof(RsslReactorLoginCredentialRenewalEvent));
	pEvent->base.eventType = RSSL_RCIMPL_ET_LOGIN_RENEWAL;
}

/* This is used to notify ping sent from the worker thread to the dispatching thread */
typedef struct
{
	RsslReactorEventImplBase base;
	RsslReactorChannel *pReactorChannel;
} RsslReactorChannelPingEvent;

RTR_C_INLINE void rsslClearReactorChannelPingEvent(RsslReactorChannelPingEvent *pEvent)
{
	memset(pEvent, 0, sizeof(RsslReactorChannelPingEvent));
	pEvent->base.eventType = RSSL_RCIMPL_ET_PING;
}

typedef enum
{
	RSSL_RCIMPL_TSET_INIT = 0,
	RSSL_RCIMPL_TSET_ADD_TOKEN_SESSION_TO_LIST = 0x01,
	RSSL_RCIMPL_TSET_REGISTER_CHANNEL_TO_SESSION = 0x02,
	RSSL_RCIMPL_TSET_UNREGISTER_CHANNEL_FROM_SESSION = 0x04,
	RSSL_RCIMPL_TSET_RETURN_CHANNEL_TO_CHANNEL_POOL = 0x08
} RsslReactorTokenSessionEventType;

typedef struct
{
	RsslReactorEventImplBase base;
	RsslReactorTokenSessionEventType reactorTokenSessionEventType;
	RsslReactorChannel *pReactorChannel;
	RsslReactorTokenSessionImpl *pTokenSessionImpl;
} RsslReactorTokenSessionEvent;

RTR_C_INLINE void rsslClearReactorTokenSessionEvent(RsslReactorTokenSessionEvent *pEvent)
{
	memset(pEvent, 0, sizeof(RsslReactorTokenSessionEvent));
	pEvent->base.eventType = RSSL_RCIMPL_ET_TOKEN_SESSION_MGNT;
}

typedef enum
{
	RSSL_RCIMPL_WSBET_INIT = 0,
	RSSL_RCIMPL_WSBET_CONNECT_SECONDARY_SERVER = 0x01,
	RSSL_RCIMPL_WSBET_CHANGE_ACTIVE_TO_STANDBY_SERVER = 0x02,
	RSSL_RCIMPL_WSBET_CHANGE_ACTIVE_TO_STANDBY_PER_SERVICE = 0x04,
	RSSL_RCIMPL_WSBET_NOTIFY_STANDBY_SERVICE = 0x08,
	RSSL_RCIMPL_WSBET_CHANGE_ACTIVE_TO_STANDBY_SERVICE_CHANNEL_DOWN = 0x10,
	RSSL_RCIMPL_WSBET_CHANGE_STANDBY_TO_ACTIVE_PER_SERVICE = 0x20,
	RSSL_RCIMPL_WSBET_REMOVE_SERVER_FROM_WSB_GROUP = 0x40,
	RSSL_RCIMPL_WSBET_CONNECT_TO_NEXT_STARTING_SERVER = 0x80,			/* currently not used */
	RSSL_RCIMPL_WSBET_ACTIVE_SERVER_SERVICE_STATE_FROM_DOWN_TO_UP = 0x100,
	RSSL_RCIMPL_WSBET_MOVE_WSB_HANDLER_BACK_TO_POOL = 0x200
} RsslReactorWarmStandByEventType;

typedef struct
{
	RsslReactorEventImplBase base;
	RsslReactorWarmStandByEventType reactorWarmStandByEventType;
	RsslReactorChannel* pReactorChannel;
	RsslUInt serviceID; /* This is used for per service based warm standby. */
	RsslInt32 streamID; /* This is used for per service based warm standby. */
	RsslHashLink *pHashLink; /* This is used for per service based warm standby. */
	RsslReactorErrorInfoImpl *pReactorErrorInfoImpl; /* This is used to covey error message if any*/
	void* pReactorWarmStandByHandlerImpl; /* Keeps the RsslReactorWarmStandByHandlerImpl for returing it back to the pool */
} RsslReactorWarmStanbyEvent;

RTR_C_INLINE void rsslClearReactorWarmStanbyEvent(RsslReactorWarmStanbyEvent* pEvent)
{
	memset(pEvent, 0, sizeof(RsslReactorWarmStanbyEvent));
	pEvent->base.eventType = RSSL_RCIMPL_ET_WARM_STANDBY;
}

/* Rest Logging Event. Used internally, transmit via reactor events queue. */
typedef struct
{
	RsslReactorEventImplBase base;
	RsslBuffer* pRestLoggingMessage;
} RsslReactorLoggingEvent;

RTR_C_INLINE void rsslClearReactorLoggingEvent(RsslReactorLoggingEvent* pRestLoggingEvent)
{
	memset(pRestLoggingEvent, 0, sizeof(RsslReactorLoggingEvent));
	pRestLoggingEvent->base.eventType = RSSL_RCIMPL_ET_LOGGING;
}

/* Rest request and response Event. This is used internally to send REST request/response between main and work threads. */
typedef struct
{
	RsslReactorEventImplBase base;
	RsslReactorExplicitServiceDiscoveryInfo* pExplicitSDInfo;
	RsslReactorErrorInfoImpl* pReactorErrorInfoImpl;
} RsslReactorRestRequestResponseEvent;

RTR_C_INLINE void rsslClearReactorRestRequestResponseEvent(RsslReactorRestRequestResponseEvent* pRestRequestResponseEvent)
{
	memset(pRestRequestResponseEvent, 0, sizeof(RsslReactorRestRequestResponseEvent));
	pRestRequestResponseEvent->base.eventType = RSSL_RCIMPL_ET_REST_REQ_RESP;
}

typedef union 
{
	RsslReactorEventImplBase			base;
	RsslReactorChannelEventImpl			channelEventImpl;
	RsslReactorCredentialRenewalEvent   credentialRenewalEvent;
	RsslReactorFlushEvent				flushEvent;
	RsslReactorChannelPingEvent			pingEvent;
	RsslReactorTokenMgntEvent			tokenMgntEvent;
	RsslReactorTokenSessionEvent		tokenSessionEvent;
	RsslReactorStateEvent				reactorEvent;
	RsslReactorTimerEvent				timerEvent;
	RsslReactorLoggingEvent				restLoggingEvent;
	RsslReactorLoginCredentialRenewalEvent loginRenewalEvent;
	RsslReactorRestRequestResponseEvent restRequestResponseEvent;
} RsslReactorEventImpl;

RTR_C_INLINE void rsslClearReactorEventImpl(RsslReactorEventImpl *pEvent)
{
	memset(pEvent, 0, sizeof(RsslReactorEventImpl));
}

#ifdef __cplusplus
};
#endif

#endif

