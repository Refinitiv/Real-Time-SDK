/*
 * This source code is provided under the Apache 2.0 license and is provided
 * AS IS with no warranty or guarantee of fit for purpose.  See the project's 
 * LICENSE.md for details. 
 * Copyright (C) 2019 Refinitiv. All rights reserved.
*/

#ifndef RSSL_WATCHLIST_IMPL_H
#define RSSL_WATCHLIST_IMPL_H

#include "rtr/wlView.h"

#include "rtr/wlService.h"
#include "rtr/wlStream.h"

#include "rtr/rsslWatchlist.h"
#include "rtr/rsslHeapBuffer.h"


#ifdef __cplusplus
extern "C" {
#endif

/* Main header for the watchlist implementation. */

typedef struct RsslWatchlistImpl RsslWatchlistImpl;

/*** Watchlist ***/

/* The watchlist structure. */
struct RsslWatchlistImpl
{
	WlBase						base;
	WlItems						items;
	WlLogin						login;
	WlDirectory					directory;
	RsslQueue					services;
};

/* Callback used to process service cache updates. */
static RsslRet wlServiceUpdateCallback(WlServiceCache *pServiceCache,
		WlServiceCacheUpdateEvent *pEvent, RsslErrorInfo *pErrorInfo);

static RsslRet wlServiceStateChangeCallback(WlServiceCache *pServiceCache, 
	RDMCachedService *pCachedService, RsslErrorInfo *pErrorInfo);

static RsslRet wlServiceCacheInitCallback(WlServiceCache *pServiceCache,
	WlServiceCacheUpdateEvent *pEvent, RsslErrorInfo *pErrorInfo);

static RsslRet wlServiceCacheUpdateCallback(WlServiceCache *pServiceCache,
	WlServiceCacheUpdateEvent *pEvent, RsslErrorInfo *pErrorInfo);

/* Reads a message from the provider. */
static RsslRet wlProcessProviderMsg(RsslWatchlistImpl *pWatchlistImpl, 
		RsslWatchlistProcessMsgOptions *pOptions, RsslErrorInfo *pErrorInfo);

/* Reads a message from the consumer. */
static RsslRet wlProcessConsumerMsg(RsslWatchlistImpl *pWatchlistImpl, 
		RsslWatchlistProcessMsgOptions *pOptions, RsslErrorInfo *pErrorInfo);

/* Clears all streams in the watchlist, setting everything to prepare for recovery.
 * Used in response to login failures and connection losses. */
static RsslRet wlRecoverAllItems(RsslWatchlistImpl *pWatchlistImpl, RsslErrorInfo *pErrorInfo);

/* Estimates the size needed to encodes an RsslMsg. */
static RsslUInt32 wlEstimateEncodedMsgSize(RsslMsg *pMsg);

/* Encodes the given RsslMsg. */
static RsslRet wlEncodeAndSubmitMsg(RsslWatchlistImpl *pWatchlistImpl, 
		RsslMsg *pRsslMsg, RsslRDMMsg *pRdmMsg, RsslBool hasView, WlAggregateView *pView, 
		RsslErrorInfo *pError);


/* Forwards a refresh to a request.
 * Like wlSendMsgEventToItemRequest but fixes refresh solicited flag to fit request. */
static RsslRet wlSendRefreshEventToItemRequest(RsslWatchlistImpl *pWatchlistImpl,
		RsslWatchlistMsgEvent *pEvent, WlItemRequest *pItemRequest, RsslErrorInfo *pErrorInfo);

/* Process an item message, and forward it to the appropriate requests. */
static RsslRet wlFanoutItemMsgEvent(RsslWatchlistImpl *pWatchlistImpl, WlItemStream *pItemStream,
		RsslWatchlistMsgEvent *pEvent, RsslErrorInfo *pErrorInfo);

/* Constructs and sends a request message for a stream. */
static RsslRet wlStreamSubmitMsg(RsslWatchlistImpl *pWatchlistImpl,
		WlStream *pStream, RsslUInt32 *pendingWaitCount, RsslErrorInfo *pError);

static RsslRet wlProcessRemovedService(RsslWatchlistImpl *pWatchlistImpl,
		WlService *pWlService, RsslErrorInfo *pErrorInfo);

void wlDestroyItemRequest(RsslWatchlistImpl *pWatchlistImpl,
		WlItemRequest *pItemRequest, RsslErrorInfo *pErrorInfo);

RsslRet wlSendMsgEventToItemRequest(RsslWatchlistImpl *pWatchlistImpl,
		RsslWatchlistMsgEvent *pEvent, WlItemRequest *pItemRequest, RsslErrorInfo *pErrorInfo);

RsslRet wlProcessItemRequest(RsslWatchlistImpl *pWatchlistImpl, WlItemRequestCreateOpts *pOpts,
		WlItemRequest *pExistingRequest, RsslErrorInfo *pErrorInfo);

static RsslRet wlProcessItemBatchRequest(RsslWatchlistImpl *pWatchlistImpl, 
		WlItemRequestCreateOpts *pOpts, RsslErrorInfo *pErrorInfo);

/* Handles sequencing of message streams on multicast connections for the given item stream,
 * including handling gaps, and synchronizing the unicast/broadcast streams. */
static RsslRet wlItemStreamOrderMsg(RsslWatchlistImpl *pWatchlistImpl, 
		WlItemStream *pItemStream, RsslWatchlistMsgEvent *pEvent, RsslErrorInfo *pErrorInfo);

/* Used by wlItemStreamOrderMsg when we want to a broadcast message to synchronize the stream
 * in a manner similar to the synchronization done when a unicast message is received before
 * the refresh completes. */
static RsslRet wlItemStreamOrderBroadcastSynchMsg(RsslWatchlistImpl *pWatchlistImpl, 
		WlItemStream *pItemStream, RsslWatchlistMsgEvent *pEvent, RsslErrorInfo *pErrorInfo);

/* Sets gap timer for an item stream, according to the gap condition given by 'flag.' */
static void wlSetGapTimer(RsslWatchlistImpl *pWatchlistImpl, WlItemStream *pItemStream,
		RsslUInt32 flag);

/* Indicates that the gap condtion given by 'flag' is no longer true and
 * the stream may no longer need the gap timer. */
static void wlUnsetGapTimer(RsslWatchlistImpl *pWatchlistImpl, WlItemStream *pItemStream,
		RsslUInt32 flag);

#ifdef __cplusplus
}
#endif

#endif
