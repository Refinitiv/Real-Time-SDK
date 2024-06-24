/*
 * This source code is provided under the Apache 2.0 license and is provided
 * AS IS with no warranty or guarantee of fit for purpose.  See the project's 
 * LICENSE.md for details. 
 * Copyright (C) 2024 LSEG. All rights reserved.
*/

#ifndef _RTR_RSSL_NIMARKETPRICE_ITEMS_H
#define _RTR_RSSL_NIMARKETPRICE_ITEMS_H


#ifdef __cplusplus
extern "C" {
#endif

#include "rtr/rsslTransport.h"
#include "rtr/rsslMessagePackage.h"
#include "rtr/rsslDataPackage.h"
#include "rsslNIItemInfo.h"

#define RDNDISPLAY_FID 2
#define RDN_EXCHID_FID 4
#define DIVPAYDATE_FID 38
#define TRDPRC_1_FID 6
#define BID_FID 22
#define ASK_FID 25
#define ACVOL_1_FID 32
#define NETCHNG_1_FID 11
#define ASK_TIME_FID 267

/* market price item data */
typedef struct {
	RsslBool		isInUse;
	RsslUInt64		RDNDISPLAY;
	RsslEnum		RDN_EXCHID;
	RsslDate		DIVPAYDATE;
	double			TRDPRC_1;
	double			BID;
	double			ASK;
	double			ACVOL_1;
	double			NETCHNG_1;
	RsslDateTime	ASK_TIME;
} RsslMarketPriceItem;

void initMarketPriceItemFields(RsslMarketPriceItem* mpItem);
void updateMarketPriceItemFields(RsslMarketPriceItem* mpItem);
RsslRet encodeMarketPriceResponse(RsslReactorChannel* chnl, RsslNIItemInfo* itemInfo, RsslBuffer* msgBuf, RsslInt32 streamId, RsslUInt16 serviceId, RsslDataDictionary* dictionary);

RsslRet encodeMarketPriceResponseMsgInit(RsslNIItemInfo* itemInfo, RsslEncodeIterator* encodeIter, RsslInt32 streamId, RsslUInt16 serviceId, RsslBool buildRefresh);
RsslRet encodeMPFieldList(RsslMarketPriceItem *mpItem, RsslEncodeIterator *encodeIter, RsslDataDictionary* dictionary, RsslBool buildRefresh);
RsslRet encodeResponseMsgComplete(RsslEncodeIterator *eIter);

/*
 * Clears the item information.
 * itemInfo - The item information to be cleared
 */
RTR_C_INLINE void clearMarketPriceItem(RsslMarketPriceItem* itemInfo)
{
	itemInfo->isInUse = RSSL_FALSE;
	itemInfo->RDNDISPLAY = 0;
	itemInfo->RDN_EXCHID = 0;
	rsslClearDate(&itemInfo->DIVPAYDATE);
	itemInfo->TRDPRC_1 = 0;
	itemInfo->BID = 0;
	itemInfo->ASK = 0;
	itemInfo->ACVOL_1 = 0;
	itemInfo->NETCHNG_1 = 0;
	rsslClearDateTime(&itemInfo->ASK_TIME);
}

void initMarketPriceItems();
RsslMarketPriceItem* getMarketPriceItem(const char* itemName);
void freeMarketPriceItem(RsslMarketPriceItem* mpItem);

#ifdef __cplusplus
};
#endif

#endif
