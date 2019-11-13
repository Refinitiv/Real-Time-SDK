/*
 * This source code is provided under the Apache 2.0 license and is provided
 * AS IS with no warranty or guarantee of fit for purpose.  See the project's 
 * LICENSE.md for details. 
 * Copyright (C) 2019 Refinitiv. All rights reserved.
*/

/* upacProvPerf.h
 * The upacProvPerf application.  Implements an interactive provider, which allows
 * the requesting of items, and responds to them with images and bursts of updates. */

#ifndef _UPAC_PROV_PERF_H
#define _UPAC_PROV_PERF_H

#include "providerThreads.h"
#include "loginProvider.h"
#include "directoryProvider.h"
#include "channelHandler.h"
#include "getTime.h"
#include "rtr/rsslQueue.h"
#include "rtr/rsslTransport.h"
#include "rtr/rsslThread.h"
#if defined(_WIN32)
#include <winsock2.h>
#include <time.h>
#else
#include <sys/time.h>
#include <sys/resource.h>
#include <sys/types.h>
#include <sys/timeb.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif


/* Callback for newly-activated channels. */
RsslRet processActiveChannel(ChannelHandler *pChanHandler, ChannelInfo *pChannelInfo);

/* Callback for failed channels . */
void processInactiveChannel(ChannelHandler *pChanHandler, ChannelInfo *pChannelInfo, RsslError *pError);

/* Callback for received messages. */
RsslRet processMsg(ChannelHandler *pChannelHandler, ChannelInfo* pChannelInfo, RsslBuffer* pBuffer);

/* Clean up and exit application. */
void cleanUpAndExit();

/* Sends a newly-connected channel to one of the ProviderThreads. */
static RsslRet sendToLeastLoadedThread(RsslChannel *chnl);

/* Adds an item request to the watchlist, if the request is legal. */
static RsslRet processItemRequest(ProviderThread *pHandler, ProviderSession *watchlist, RsslMsg* msg, RsslDecodeIterator* dIter);

/* Searches for an item in the watchlist that matches the given attributes.  */
static ItemInfo *findAlreadyOpenItem(ProviderSession *watchlist, RsslMsg* msg, RsslItemAttributes* attribs);

/* Checks if this stream is being used by an item with a different key */
static RsslBool isStreamInUse(ProviderSession *watchlist, RsslInt32 streamId, RsslMsgKey* key);

/* Sends a CloseMsg rejecting a request */
static RsslRet sendItemRequestReject(ProviderThread *pHandler, ProviderSession* watchlist, RsslInt32 streamId, RsslUInt8 domainType, ItemRejectReason reason);

/* Sends a CloseMsg for the given ItemInfo. */
static RsslRet sendItemCloseStatusMsg(ProviderThread *pHandler, ProviderSession* watchlist, ItemInfo* itemInfo);

/* Processes a post message received from a consumer and reflects its contents. */
static RsslRet reflectPostMsg(ProviderThread *pHandler, RsslDecodeIterator *pIter, ProviderSession *watchlist, RsslPostMsg *pPostMsg);

/* Processes a generic message received from a consumer. */
static RsslRet processGenMsg(ProviderThread *pHandler, RsslDecodeIterator *pIter, ProviderSession *watchlist, RsslGenericMsg *pGenMsg);

/* Decodes a MarketPrice message. */
RsslRet decodeMPUpdate(RsslDecodeIterator *pIter, RsslMsg *msg, ProviderThread* pHandler);


/* Decodes a MarketByOrder message. */
RsslRet decodeMBOUpdate(RsslDecodeIterator *pIter, RsslMsg *msg, ProviderThread * pProviderThread);

#ifdef __cplusplus
};
#endif

#endif
