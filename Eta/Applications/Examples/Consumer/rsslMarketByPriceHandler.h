
#ifndef _RTR_RSSL_MARKET_BY_PRICE_HANDLER_H
#define _RTR_RSSL_MARKET_BY_PRICE_HANDLER_H

#include "rsslConsumer.h"

#ifdef __cplusplus
extern "C" {
#endif

/* market by price item information */
typedef struct {
	char		itemname[MAX_ITEM_NAME_STRLEN+1];
	RsslUInt32	nameLength;
	RsslInt32	streamId;
	RsslState	itemState;
} MarketByPriceItemInfo;

/* called by rsslConsumer to enable snapshot requesting for market by price items */
void setMBPSnapshotRequest();
/* called from rsslConsumer to add a market by price item */
void addMarketByPriceItemName(char* itemname, RsslBool isPrivateStream);
/* called when it is time to send market by price requests */
RsslRet sendMarketByPriceItemRequests(RsslChannel* chnl);
/* called to process market by price responses */
RsslRet processMarketByPriceResponse(RsslChannel* chnl, RsslMsg* msg, RsslDecodeIterator* dIter);
/* called to close market by price streams */
RsslRet closeMarketByPriceItemStreams(RsslChannel* chnl);
/* called to redirect a request to a private stream */
RsslRet redirectToMarketByPricePrivateStream(RsslChannel* chnl, RsslInt32 streamId);

#ifdef __cplusplus
};
#endif

#endif
