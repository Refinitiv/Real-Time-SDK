/*
 * This source code is provided under the Apache 2.0 license and is provided
 * AS IS with no warranty or guarantee of fit for purpose.  See the project's 
 * LICENSE.md for details. 
 * Copyright (C) 2019 Refinitiv. All rights reserved.
*/

#ifndef RSSL_WATCHLIST_H
#define RSSL_WATCHLIST_H

#include "rtr/wlServiceCache.h"
#include "rtr/rsslRDMMsg.h"
#include "rtr/rsslMessagePackage.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Provides the interfaces that the RsslReactor uses internally to interact with the watchlist. */

typedef struct RsslWatchlist RsslWatchlist;
typedef struct RsslWatchlistMsgEvent RsslWatchlistMsgEvent;
typedef struct RsslWatchlistStreamInfo RsslWatchlistStreamInfo;

/* The callback the watchlist uses to call the reactor to provide messages. */
typedef RsslRet RsslWatchlistMsgCallback(RsslWatchlist*, RsslWatchlistMsgEvent*,RsslErrorInfo*);

struct RsslWatchlistStreamInfo
{
	const RsslBuffer	*pServiceName;	/* Service name used to request the item, if any. */
	void				*pUserSpec;		/* Pointer user provided when requesting this item. */
};

RTR_C_INLINE void wlStreamInfoClear(RsslWatchlistStreamInfo *pStreamInfo)
{ memset(pStreamInfo, 0, sizeof(RsslWatchlistStreamInfo)); }

typedef enum
{
	WL_MEF_NONE			= 0x00,
	WL_MEF_SEND_CLOSE	= 0x01 /* Message in event was internally generated and may require
								* us to send a close to the provider. */
} WlMsgEventFlags;

/* A message event from the watchlist. */
struct RsslWatchlistMsgEvent
{
	RsslBuffer				*pRsslBuffer;
	RsslMsg					*pRsslMsg;
	RsslRDMMsg				*pRdmMsg;
	RsslWatchlistStreamInfo *pStreamInfo;
	RsslUInt32				*pSeqNum;
	RsslUInt8				*pFTGroupId;

	/* For internal use by the watchlist only. */
	RsslUInt8				_flags;
};

static void wlMsgEventClear(RsslWatchlistMsgEvent *pEvent)
{
	memset(pEvent, 0, sizeof(RsslWatchlistMsgEvent));
}

typedef enum
{
	RSSLWL_STF_NEED_FLUSH		= 0x1,	/* The watchlist has written data to the channel and needs the 
										 * channel to be flushed. */
	RSSLWL_STF_NEED_TIMER		= 0x2,	/* The watchlist may have new timeouts. */
	RSSLWL_STF_RESET_CONN_DELAY	= 0x4	/* The reactor should reset its reconnection time delay
										 * (set when a login stream is established). */
} RsslWatchlistStateFlags;

/* The current state of the watchlist.  The reactor may use this to determine when certain
 * actions are needed. */
typedef struct 
{
	RsslUInt32	watchlistStateFlags;
} RsslWatchlistState;

/* Indicates the occurence of the next timeout event in the watchlist. */
RsslInt64 rsslWatchlistGetNextTimeout(RsslWatchlist *pWatchlist);

/* Checks watchlist timer events, based on the current time. */
RsslRet rsslWatchlistProcessTimer(RsslWatchlist *pWatchlist, RsslInt64 currentTime,
		RsslErrorInfo *pErrorInfo);

/* Resets gap timeout to its starting value. Should be called when
 * gaps are detected in the transport. */
void rsslWatchlistResetGapTimer(RsslWatchlist *pWatchlist);

/* Process FTGroup pings. */
RsslInt64 rsslWatchlistProcessFTGroupPing(RsslWatchlist *pWatchlist, RsslUInt8 ftGroupId, 
		RsslInt64 currentTime);

/* Options for rsslWatchlistCreate. */
typedef struct
{
	RsslWatchlistMsgCallback	*msgCallback;
	RsslUInt32					itemCountHint;
	RsslBool					obeyOpenWindow;
	RsslUInt32					requestTimeout;
	RsslUInt32					maxOutstandingPosts;
	RsslUInt32					postAckTimeout;
	RsslInt64					ticksPerMsec;
	RsslInt32					loginRequestCount;
} RsslWatchlistCreateOptions;

/* Reactor-facing watchlist structure. */
struct RsslWatchlist
{
	void				*pUserSpec;
	RsslUInt32			state;
};

RTR_C_INLINE void rsslWatchlistClearCreateOptions(RsslWatchlistCreateOptions *pOptions)
{
	memset(pOptions, 0, sizeof(RsslWatchlistCreateOptions));
	pOptions->obeyOpenWindow = RSSL_TRUE;
	pOptions->requestTimeout = 15000;
	pOptions->postAckTimeout = 15000;
}

/* Creates an RsslWatchlist. */
RsslWatchlist *rsslWatchlistCreate(RsslWatchlistCreateOptions *pCreateOptions, 
		RsslErrorInfo *pErrorInfo);

/* Sets the RsslChannel used by the watchlist. Set the channel to NULL to indicate
 * disconnection. */
RsslRet rsslWatchlistSetChannel(RsslWatchlist *pWatchlist, RsslChannel *pChannel,
		RsslErrorInfo *pErrorInfo);

/* Cleans up an RsslWatchlist. */
void rsslWatchlistDestroy(RsslWatchlist *pWatchlist);


/* Options for processing an RsslMsg in the watchlist. */
typedef struct
{
	RsslChannel			*pChannel;
	RsslBuffer			*pRsslBuffer;
	RsslDecodeIterator	*pDecodeIterator;
	RsslMsg				*pRsslMsg;
	RsslRDMMsg			*pRdmMsg;
	RsslBuffer			*pServiceName;
	RsslUInt32			viewAction;
	RsslUInt			viewType;
	RsslUInt32			viewElemCount;
	void				*viewElemList;
	void				*pUserSpec;
	RsslUInt32			majorVersion;
	RsslUInt32			minorVersion;
	RsslUInt8			*pFTGroupId;
	RsslUInt32			*pSeqNum;
} RsslWatchlistProcessMsgOptions;

RTR_C_INLINE void rsslWatchlistClearProcessMsgOptions(RsslWatchlistProcessMsgOptions *pOptions)
{
	memset(pOptions, 0, sizeof(RsslWatchlistProcessMsgOptions));
}

/* Reads a message from the provider. */
RsslRet rsslWatchlistReadMsg(RsslWatchlist *pWatchlist, 
		RsslWatchlistProcessMsgOptions *pOptions, RsslInt64 currentTime, RsslErrorInfo *pErrorInfo);


/* Reads a buffer from the consumer. */
RsslRet rsslWatchlistSubmitBuffer(RsslWatchlist *pWatchlist,
		RsslWatchlistProcessMsgOptions *pOptions, RsslErrorInfo *pErrorInfo);

/* Reads a message from the consumer. */
RsslRet rsslWatchlistSubmitMsg(RsslWatchlist *pWatchlist, 
		RsslWatchlistProcessMsgOptions *pOptions, RsslErrorInfo *pErrorInfo);

/* Used to drive the Reactor when needed (e.g. there are new requests to
 * process, messages that need to be sent). */
RsslRet rsslWatchlistDispatch(RsslWatchlist *pWatchlist, RsslInt64 currentTime, RsslErrorInfo *pErrorInfo);

#ifdef __cplusplus
}
#endif

#endif
