/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2019-2022 Refinitiv. All rights reserved.         --
 *|-----------------------------------------------------------------------------
 */


#ifndef _RTR_RSSL_MARKET_BY_ORDER_HANDLER_H
#define _RTR_RSSL_MARKET_BY_ORDER_HANDLER_H

#include "rsslConsumer.h"

#ifdef __cplusplus
extern "C" {
#endif

/* market by order item information */
typedef struct {
	char		itemname[MAX_ITEM_NAME_STRLEN+1];
	RsslUInt32	nameLength;
	RsslInt32	streamId;
	RsslState	itemState;
} MarketByOrderItemInfo;

/* called by rsslConsumer to enable snapshot requesting for market by order items */
void setMBOSnapshotRequest();
/* called from rsslConsumer to add a market by order item */
void addMarketByOrderItemName(char* itemname, RsslBool isPrivateStream);
/* called when it is time to send market by order requests */
RsslRet sendMarketByOrderItemRequests(RsslChannel* chnl);
/* called to process market by order responses */
RsslRet processMarketByOrderResponse(RsslChannel* chnl, RsslMsg* msg, RsslDecodeIterator* dIter);
/* called to close market by order streams */
RsslRet closeMarketByOrderItemStreams(RsslChannel* chnl);
/* called to redirect a request to a private stream */
RsslRet redirectToMarketByOrderPrivateStream(RsslChannel* chnl, RsslInt32 streamId);


#ifdef __cplusplus
};
#endif

#endif
