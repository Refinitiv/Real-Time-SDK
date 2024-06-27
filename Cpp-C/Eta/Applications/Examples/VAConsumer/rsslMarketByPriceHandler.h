/*
 * This source code is provided under the Apache 2.0 license and is provided
 * AS IS with no warranty or guarantee of fit for purpose.  See the project's 
 * LICENSE.md for details. 
 * Copyright (C) 2019 LSEG. All rights reserved.
*/

#ifndef _RTR_RSSL_MARKET_BY_PRICE_HANDLER_H
#define _RTR_RSSL_MARKET_BY_PRICE_HANDLER_H

#include "rsslConsumer.h"

#ifdef __cplusplus
extern "C" {
#endif

/* called by rsslConsumer to enable snapshot requesting for market by price items */
void setMBPSnapshotRequest();

/* called when it is time to send market by price requests */
RsslRet sendMarketByPriceItemRequests(RsslReactor *pReactor, RsslReactorChannel* chnl);

/* called to process market by price responses */
RsslRet processMarketByPriceResponse(RsslReactor *pReactor, RsslReactorChannel *pReactorChannel, RsslMsgEvent* pMsgEvent, RsslMsg* msg, RsslDecodeIterator* dIter);

/* called to close all market by price streams */
RsslRet closeMarketByPriceItemStreams(RsslReactor *pReactor, ChannelCommand *pCommand);

/* called to redirect a request to a private stream */
static RsslRet redirectToPrivateStream(RsslReactor *pReactor, ChannelCommand *pCommand, ItemRequest *pRequest);

#ifdef __cplusplus
};
#endif

#endif
