/*
 * This source code is provided under the Apache 2.0 license and is provided
 * AS IS with no warranty or guarantee of fit for purpose.  See the project's 
 * LICENSE.md for details. 
 * Copyright (C) 2019 Refinitiv. All rights reserved.
*/

/* marketByOrderEncoder.h
 * Provides encoding of a MarketByOrder domain data payload. */

#ifndef _MARKET_BY_ORDER_ENCODER_H
#define _MARKET_BY_ORDER_ENCODER_H

#ifdef __cplusplus
extern "C" {
#endif

#include "xmlMsgDataParser.h"
#include "rtr/rsslQueue.h"
#include "rtr/rsslTransport.h"
#include "rtr/rsslMessagePackage.h"

/* market by order item data */
typedef struct {
	RsslInt32		iMsg;	/* Index of next message payload to send. */
} MarketByOrderItem;

/* Encodes a MarketByOrder data body for a message. */
RsslRet encodeMarketByOrderDataBody(RsslEncodeIterator *pIter, MarketByOrderMsg *pMboMsg,
		RsslMsgClasses msgClass, RsslUInt encodeStartTime);

/* Clears a MarketByOrderItem structure. */
RTR_C_INLINE void clearMarketByOrderItem(MarketByOrderItem* itemInfo)
{
	itemInfo->iMsg = 0;
}

/* Create a MarketByOrderItem. */
MarketByOrderItem *createMarketByOrderItem();

/* Cleanup a MarketByOrderItem. */
void freeMarketByOrderItem(MarketByOrderItem* mboItem);

/* Estimate the size of the next MarketByOrder update payload. */
RsslUInt32 getNextMarketByOrderUpdateEstimatedContentLength(MarketByOrderItem *mboItem);

/* Get the next MarketByOrder update(moves over the list). */
MarketByOrderMsg *getNextMarketByOrderUpdate(MarketByOrderItem *mboItem);

/* Get the total number of sample update payloads available from the message file. */
RsslInt32 getMarketByOrderUpdateMsgCount();

/* Estimate the size of the next MarketByOrder post payload. */
RsslUInt32 getNextMarketByOrderPostEstimatedContentLength(MarketByOrderItem *mboItem);

/* Get the next MarketByOrder post(moves over the list). */
MarketByOrderMsg *getNextMarketByOrderPost(MarketByOrderItem *mboItem);

/* Get the total number of sample post payloads available from the message file. */
RsslInt32 getMarketByOrderPostMsgCount();

/* Estimate the size of the next MarketByOrder generic msg payload. */
RsslUInt32 getNextMarketByOrderGenMsgEstimatedContentLength(MarketByOrderItem *mboItem);

/* Get the next MarketByOrder generic msg(moves over the list). */
MarketByOrderMsg *getNextMarketByOrderGenMsg(MarketByOrderItem *mboItem);

/* Get the total number of sample generic msg payloads available from the message file. */
RsslInt32 getMarketByOrderGenMsgMsgCount();

#ifdef __cplusplus
};
#endif

#endif

