/*
 * This source code is provided under the Apache 2.0 license and is provided
 * AS IS with no warranty or guarantee of fit for purpose.  See the project's 
 * LICENSE.md for details. 
 * Copyright Thomson Reuters 2015. All rights reserved.
*/

#ifndef WL_BASE_H
#define WL_BASE_H

#include "rtr/rsslMemoryPool.h"
#include "rtr/rsslWatchlist.h"
#include "rtr/rsslRDMMsg.h"
#include "rtr/rsslErrorInfo.h"
#include "rtr/rsslHashTable.h"
#include "rtr/rsslReactorUtils.h"
#include "rtr/rsslQueue.h"
#include "rtr/rsslTypes.h"
#include <assert.h>

static const RsslInt64 WL_TIME_UNSET = 0x7fffffffffffffffLL;

#define rssl_set_buffer_to_string(__buffer, __string) \
	(__buffer.data = (char*)__string, __buffer.length = sizeof(__string) - 1) 

#ifdef __cplusplus
extern "C" {
#endif

/* Creates a copy of an RsslRDMMsg. */
RsslRDMMsg *wlCreateRdmMsgCopy(RsslRDMMsg *pRdmMsg, RsslBuffer *pMemoryBuffer, 
		RsslErrorInfo *pErrorInfo);

typedef struct WlStreamBase WlStreamBase;

static const RsslInt32 LOGIN_STREAM_ID = 1;
static const RsslInt32 DIRECTORY_STREAM_ID = 2;
static const RsslInt32 MIN_STREAM_ID = 3;
static const RsslInt32 MAX_STREAM_ID = 0x7fffffff;

/* Attributes that uniquely identify a stream. */
typedef struct 
{
	RsslBool 	hasQos;
	RsslUInt8	domainType;
	RsslMsgKey	msgKey;
	RsslQos		qos;
} WlStreamAttributes;

/* Hash sum function for stream attributes. */
RsslUInt32 wlStreamAttributesHashSum(void *pKey);

/* Hash comparison function  for stream attributes. */
RsslBool wlStreamAttributesHashCompare(void *pKey1, void *pKey2);

/* Base structure for requests. */
typedef struct
{
	RsslHashLink		hlStreamId;
	RsslQueueLink		qlStateQueue;
	RsslQueue			*pStateQueue;
	RsslInt32			streamId;
	RsslUInt8			domainType;
	void				*pUserSpec;
	WlStreamBase		*pStream;
} WlRequestBase;

/* Initializes a WlRequestBase. */
RTR_C_INLINE void wlRequestBaseInit(WlRequestBase *pBase, RsslInt32 streamId, RsslUInt8 domainType,
		void *pUserSpec)
{
	memset(pBase, 0, sizeof(WlRequestBase));
	pBase->streamId = streamId;
	pBase->domainType = domainType;
	pBase->pUserSpec = pUserSpec;
}

typedef enum
{
	WL_STRS_NONE				= 0x0,	/* Stream has no request to send. */
	WL_STRS_PENDING_REQUEST		= 0x1,	/* Stream needs to send a request. */
	WL_STRS_PENDING_RESPONSE	= 0x2	/* Stream has sent a request and is awaiting response. */
} WlStreamBaseRequestState;

/* Base structure for streams. */
struct WlStreamBase
{
	RsslHashLink	hlStreamId;
	RsslQueueLink	qlStreamsList;
	RsslQueueLink	qlStreamsPendingRequest;
	RsslQueueLink	qlStreamsPendingResponse;
	RsslInt32		streamId;
	RsslUInt8		domainType;
	RsslInt64		requestExpireTime;			/* Time at which the request is considered timed out. */
	RsslBool		isClosing;
	RsslBool		tempStream;
	RsslUInt8		requestState;
};

/* Initializes a WlStreamBase. */
RTR_C_INLINE void wlStreamBaseInit(WlStreamBase *pBase, RsslInt32 streamId, RsslUInt8 domainType)
{
	memset(pBase, 0, sizeof(WlStreamBase));
	pBase->streamId = streamId;
	pBase->domainType = domainType;
}

/* Contains configuration. May be set at watchlist initialiation, or by login request/refresh. */
typedef struct
{
	RsslUInt					supportOptimizedPauseResume;	/* Login refresh parameter, SupportOptimizedPauseResume. */
	RsslUInt					supportViewRequests;			/* Login refresh parameter, SupportViewRequests. */
	RsslUInt					singleOpen;						/* Login request parameter, SingleOpen. */
	RsslUInt					allowSuspectData;				/* Login request parameter, AllowSuspectData. */
	RsslWatchlistMsgCallback	*msgCallback;					/* Callback the watchlist should use to forward messages. */
	RsslBool					obeyOpenWindow;					/* Whether the watchlist obeys a service's OpenWindow. */
	RsslUInt32					requestTimeout;					/* Request timeout, in milliseconds. */
} WlConfig;

/* Represents the state of the current channel session. */
typedef enum 
{
	WL_CHS_START				= 0, /* Initial state. */
	WL_CHS_LOGIN_REQUESTED		= 1, /* Login request has been sent. */
	WL_CHS_LOGGED_IN			= 2, /* Login refresh has been received. */
	WL_CHS_READY				= 3, /* Directory request has been sent. */
	WL_CHS_CLOSED				= 4  /* Watchlist is fully closed (this is likely due to a login
									  * or directory stream receiving a CLOSED state). */
} WlChannelState;

/* Watchlist base structure.
 * Contains elements commonly operated on by the different domain handlers. */
typedef struct
{
	RsslWatchlist		watchlist;				/* Watchlist reference used by the reactor. */
	WlConfig			config;					/* Configuration options. */
	RsslQueue			openStreams;			/* List of currently open streams. */
	RsslHashTable		openStreamsByAttrib;	/* Table of open streams, by WlStreamAttributes. */
	RsslQueue			newRequests;			/* Recently-submitted requests. */
	RsslQueue			requestedServices;		/* List of requested services. */
	RsslHashTable		requestedSvcByName;		/* Table of requested service names. */
	RsslHashTable		requestedSvcById;		/* Table of requested service ID's. */
	RsslBuffer			tempDecodeBuffer;		/* Reusable decoding buffer. */
	RsslBuffer			tempEncodeBuffer;		/* Reusable encoding buffer. */
	RsslBuffer			tempFanoutBuffer;		/* Reusable fanout buffer. */
	RsslHashTable		streamsById;			/* Table of open streams, by Stream ID. */
	RsslHashTable		requestsByStreamId;		/* Table of requests, by stream ID. */
	RsslUInt32			channelMaxFragmentSize;	/* Channel's maxFragmentSize. */
	WlChannelState		channelState;			/* Channel state. */
	RsslChannel			*pRsslChannel;			/* Current channel, if any. */
	RsslBuffer			*pWriteCallAgainBuffer;	/* Used to handle RSSL_RET_WRITE_CALL_AGAIN codes from rsslWrite. */
	WlServiceCache		*pServiceCache;			/* Serivce cache. */
	RsslQueue			streamsPendingRequest;	/* Streams that need to send a request. */
	RsslQueue			streamsPendingResponse;	/* Streams opened but waiting for a response. */
	RsslMemoryPool		requestPool;			/* Pool of WlRequest structures. */
	RsslMemoryPool		streamPool;				/* Pool of WlStream structures. */
	RsslInt64			currentTime;			/* Latest timestamp set by caller, in milliseconds. */
	RsslUInt			gapRecovery;			/* Multicast: Whether to recover from sequence number gaps. */
	RsslUInt			gapTimeout;				/* Multicast: Time to wait for a sequence gap to resolve itself before recovering. */
	RsslUInt			maxBufferedBroadcastMsgs;	/* Multicast: Maximum number of messages to buffer per stream when reordering messages. */
	RsslInt32			nextStreamId;			/* Next ID to use when opening a stream. */
	RsslInt32			nextProviderStreamId;	/* Next ID to use when opening a stream. */
	RsslInt64			ticksPerMsec;			/* Ticks per millisecond. Used when getting current time (windows only) */
} WlBase;

/* Options for initializing the base structure. */
typedef struct
{
	RsslWatchlistMsgCallback		*msgCallback;			/* Callback the watchlist should use to forward messsages. */
	WlServiceCacheUpdateCallback	*updateCallback;		/* Callback for service cache updates. */
	int								requestPoolBlockSize;	/* Size of the WlRequest structure. */
	int								requestPoolCount;		/* Size of WlRequest pool. */
	int								streamPoolBlockSize;	/* Size of the WlStream structure. */
	int								streamPoolCount;		/* Size of WlStream pool. */
	RsslBool						obeyOpenWindow;			/* Whether the watchlist should obey a service's OpenWinow. */
	RsslUInt32						requestTimeout;			/* Time a stream will wait for a response, in milliseconds. */
	RsslInt64						ticksPerMsec;			/* Ticks per millisecond. Used when getting current time (windows only) */
} WlBaseInitOptions;

/* Initializes a WlBase structure. */
RsslRet wlBaseInit(WlBase *pBase, WlBaseInitOptions *pOpts, RsslErrorInfo *pErrorInfo);

/* Cleans up a WlBase structure. */
void wlBaseCleanup(WlBase *pBase);

/* Adds a request to the watchlist. */
void wlAddRequest(WlBase *pBase, WlRequestBase *pRequestBase);

/* Removes a request from the watchlist. */
void wlRemoveRequest(WlBase *pBase, WlRequestBase *pRequestBase);

/* Initializes a status message. */
static void wlInitErrorStatusMsg(RsslStatusMsg *pStatusMsg, RsslUInt8 domainType)
{
	rsslClearStatusMsg(pStatusMsg);
	pStatusMsg->flags |= RSSL_STMF_HAS_STATE;
	pStatusMsg->state.streamState = RSSL_STREAM_CLOSED;
	pStatusMsg->state.dataState = RSSL_DATA_SUSPECT;
	pStatusMsg->state.code = RSSL_SC_USAGE_ERROR;
	pStatusMsg->msgBase.domainType = domainType;
	pStatusMsg->msgBase.containerType = RSSL_DT_NO_DATA;
}

/* Initializes a messasge event. */
static void wlInitMsgEvent(WlBase *pBase, RsslWatchlistMsgEvent *pMsgEvent, 
		RsslWatchlistStreamInfo *pStreamInfo, RsslMsg *pMsg)
{
	wlMsgEventClear(pMsgEvent);
	wlStreamInfoClear(pStreamInfo);
	pMsgEvent->pRsslMsg = pMsg;
	pMsgEvent->pStreamInfo = pStreamInfo;
}

/* Sets a stream to send a message. */
void wlSetStreamMsgPending(WlBase *pBase, WlStreamBase *pStreamBase);

/* Removes a stream from the message pending queue. */
void wlUnsetStreamMsgPending(WlBase *pBase, WlStreamBase *pStreamBase);

/* Sets stream to wait for a response. */
void wlSetStreamPendingResponse(WlBase *pBase, WlStreamBase *pStreamBase);

/* Resets the timer on a stream waiting for a response, if one is running. */
void wlResetStreamPendingResponse(WlBase *pBase, WlStreamBase *pStreamBase);

/* Removes stream from waiting for a response (shortcut of wlUnsetStreamMsgPending). */
void wlUnsetStreamPendingResponse(WlBase *pBase, WlStreamBase *pStreamBase);

/* Remove stream from request & response pending lists. Used when closing streams. */
void wlUnsetStreamFromPendingLists(WlBase *pBase, WlStreamBase *pStreamBase);

/* Retrieves an unused stream ID. */
RsslInt32 wlBaseTakeStreamId(WlBase *pBase);

/* Retrieves an unused stream ID. */
RsslInt32 wlBaseTakeProviderStreamId(WlBase *pBase);

#ifdef __cplusplus
}
#endif


#endif

