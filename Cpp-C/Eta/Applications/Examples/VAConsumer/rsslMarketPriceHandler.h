/*
 * This source code is provided under the Apache 2.0 license and is provided
 * AS IS with no warranty or guarantee of fit for purpose.  See the project's 
 * LICENSE.md for details. 
 * Copyright (C) 2024 LSEG. All rights reserved.
*/

#ifndef _RTR_RSSL_MARKETPRICE_HANDLER_H
#define _RTR_RSSL_MARKETPRICE_HANDLER_H

#include "rsslConsumer.h"
#include "rsslChannelCommand.h"

#ifdef __cplusplus
extern "C" {
#endif

/* called by rsslConsumer to enable view requesting for market price items */
void setViewRequest();

/* called by rsslConsumer to enable snapshot requesting for market price items */
void setMPSnapshotRequest();

/* called to send all market price requests */
RsslRet sendMarketPriceItemRequests(RsslReactor *pReactor, RsslReactorChannel* chnl);

/* called to process all market price responses */
RsslRet processMarketPriceResponse(RsslReactor *pReactor, RsslReactorChannel *pReactorChannel, RsslMsgEvent* pMsgEvent, RsslMsg* msg, RsslDecodeIterator* dIter);

/* called by market price, market by order, and market by price domain handlers
   to parse any field list content */
RsslRet decodeFieldEntry(RsslDataDictionary *dictionary, RsslFieldEntry* fEntry, RsslDecodeIterator* dIter);

/* Used when posting to get the first open and available market price item to post on. */
ItemRequest* getFirstMarketPriceItem(ChannelCommand *pCommand);

/* called to close all market price streams */
RsslRet closeMarketPriceItemStreams(RsslReactor *pReactor, ChannelCommand *pCommand);

/* called to redirect a request to a private stream */
static RsslRet redirectToPrivateStream(RsslReactor *pReactor, ChannelCommand *pCommand, ItemRequest *pRequest);

/* called to decode a Market Price field list container */
RsslRet decodeMarketPriceFieldList(RsslDataDictionary *dictionary, RsslDecodeIterator *dIter);

#ifdef __cplusplus
};
#endif

#endif
