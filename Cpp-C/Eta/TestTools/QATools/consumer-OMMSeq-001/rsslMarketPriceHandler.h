/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2017 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */


#ifndef _RTR_RSSL_MARKETPRICE_HANDLER_H
#define _RTR_RSSL_MARKETPRICE_HANDLER_H

#include "rsslConsumer.h"

#ifdef __cplusplus
extern "C" {
#endif

/* market price item information */
typedef struct {
	char		itemname[MAX_ITEM_NAME_STRLEN+1];
	RsslUInt32	nameLength;
	RsslInt32	streamId;
	RsslState	itemState;
} MarketPriceItemInfo;

/* called from rsslConsumer to add a market price item */
void addMarketPriceItemName(char* itemname, RsslBool isPrivateStream);
/* called by rsslConsumer to enable view requesting for market price items */
void setViewRequest();
/* called by rsslConsumer to enable snapshot requesting for market price items */
void setMPSnapshotRequest();
/* called to send all market price requests */
RsslRet sendMarketPriceItemRequests(RsslChannel* chnl);
//APIQA
void setReissueQos(int rate, int timeliness);

void setReissueQosAndWorstQos(int rate, int timeliness, int worstRate, int worstTimeliness);

void setReissueChangeServiceName(int newServiceId);

RsslRet sendMarketPriceItemReissue(RsslChannel* chnl, RsslQos* qos, RsslQos* worstQos);

RsslRet sendMarketPriceItemReissueService(RsslChannel* chnl, int serviceId);

void setPriviateStream();

void setReqSameKeyWithDiffStreamId();

void setSpecificServiceId(int id);
//END APIQA

RsslRet sendMarketPriceItemReissue(RsslChannel* chnl, RsslQos* qos, RsslQos* worstQos);

RsslRet sendMarketPriceItemReissueService(RsslChannel* chnl, int serviceId);

void setPriviateStream();

void setReqSameKeyWithDiffStreamId();

void setSpecificServiceId(int id);


/* called to process all market price responses */
RsslRet processMarketPriceResponse(RsslChannel* chnl, RsslMsg* msg, RsslDecodeIterator* dIter);
/* called to close all market price streams */
RsslRet closeMarketPriceItemStreams(RsslChannel* chnl);
/* called by market price, market by order, and market by price domain handlers
   to parse any field list content */
RsslRet decodeFieldEntry(RsslFieldEntry* fEntry, RsslDecodeIterator* dIter);

/* Used when posting to get the first open and available market price item to post on.
 * returns streamId and populates mpItemName with name and length info.  If 
 * 0 is returned, no valid market price items are available */
RsslInt32 getFirstMarketPriceItem(RsslBuffer *mpItemName);

/* called to redirect a request to a private stream */
RsslRet redirectToMarketPricePrivateStream(RsslChannel* chnl, RsslInt32 streamId);

#ifdef __cplusplus
};
#endif

#endif
