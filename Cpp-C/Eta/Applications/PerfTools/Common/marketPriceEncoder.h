/*
 * This source code is provided under the Apache 2.0 license and is provided
 * AS IS with no warranty or guarantee of fit for purpose.  See the project's 
 * LICENSE.md for details. 
 * Copyright (C) 2024 LSEG. All rights reserved.
*/

/* marketPriceEncoder.h
 * Provides encoding of a MarketPrice domain data payload. */

#ifndef _MARKET_PRICE_ENCODER_H
#define _MARKET_PRICE_ENCODER_H

#ifdef __cplusplus
extern "C" {
#endif

#include "itemEncoder.h"
#include "xmlMsgDataParser.h"
#include "rtr/rsslTransport.h"
#include "rtr/rsslMessagePackage.h"
#include "rtr/rsslQueue.h"

/* market price item data */
typedef struct {
	RsslInt32		iMsg;
} MarketPriceItem;

/* Encodes a MarketPrice data body for a message. */
RsslRet encodeMarketPriceDataBody(RsslEncodeIterator *pIter, MarketPriceMsg *mpMsg,
		RsslMsgClasses msgClass, RsslUInt encodeStartTime);

/* Clears a MarketPriceItem. */
RTR_C_INLINE void clearMarketPriceItem(MarketPriceItem* itemInfo)
{
	itemInfo->iMsg = 0;
}


/* Creates a MarketPriceItem. */
MarketPriceItem *createMarketPriceItem();

/* Cleans up a MarketPriceItem. */
void freeMarketPriceItem(MarketPriceItem* mpItem);

/* Estimate the size of the next MarketPrice update payload. */
RsslUInt32 getNextMarketPriceUpdateEstimatedContentLength(MarketPriceItem *mpItem);

/* Get the next MarketPrice update(moves over the list). */
MarketPriceMsg *getNextMarketPriceUpdate(MarketPriceItem *mpItem);

/* Get the total number of sample update payloads available from the message file. */
RsslInt32 getMarketPriceUpdateMsgCount();

/* Estimate the size of the next MarketPrice post payload. */
RsslUInt32 getNextMarketPricePostEstimatedContentLength(MarketPriceItem *mpItem);

/* Get the next MarketPrice post(moves over the list). */
MarketPriceMsg *getNextMarketPricePost(MarketPriceItem *mpItem);

/* Get the total number of sample post payloads available from the message file. */
RsslInt32 getMarketPricePostMsgCount();

/* Estimate the size of the next MarketPrice generic msg payload. */
RsslUInt32 getNextMarketPriceGenMsgEstimatedContentLength(MarketPriceItem *mpItem);

/* Get the next MarketPrice generic msg(moves over the list). */
MarketPriceMsg *getNextMarketPriceGenMsg(MarketPriceItem *mpItem);

/* Get the total number of sample generic msg payloads available from the message file. */
RsslInt32 getMarketPriceGenMsgCount();

#ifdef __cplusplus
};
#endif

#endif
