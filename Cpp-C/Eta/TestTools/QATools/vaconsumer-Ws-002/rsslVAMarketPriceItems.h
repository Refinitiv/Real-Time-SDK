/*
 * This source code is provided under the Apache 2.0 license and is provided
 * AS IS with no warranty or guarantee of fit for purpose.  See the project's 
 * LICENSE.md for details. 
 * Copyright (C) 2019-2020 LSEG. All rights reserved.     
*/



#ifndef _RTR_RSSL_MARKETPRICE_ITEMS_H
#define _RTR_RSSL_MARKETPRICE_ITEMS_H


#ifdef __cplusplus
extern "C" {
#endif

#include "rtr/rsslTransport.h"
#include "rtr/rsslMessagePackage.h"
#include "rsslVAItemEncode.h"

#define RDNDISPLAY_FID 2
#define RDN_EXCHID_FID 4
#define DIVPAYDATE_FID 38
	//API QA
#define PRCTCK_1_FID 14
#define LST_PRCTCK_FID 1791
#define DIVIDENDTP_FID 37
#define TRD_UNITS_FID 53
#define MPV_FID 3364
#define LOTSZUNITS_FID 54 
	// END API QA
#define TRDPRC_1_FID 6
#define BID_FID 22
#define ASK_FID 25
#define ACVOL_1_FID 32
#define NETCHNG_1_FID 11
#define ASK_TIME_FID 267
#define PERATIO_FID 36
#define SALTIME_FID 379

/* market price item data */
typedef struct {
	RsslBool		isInUse;
	RsslUInt64		RDNDISPLAY;
	RsslEnum		RDN_EXCHID;
        //APIQA
	RsslEnum		PRCTCK_1;
	RsslEnum		LST_PRCTCK;
	RsslEnum                DIVIDENDTP;
	RsslEnum                TRD_UNITS;
	RsslEnum                MPV;
	RsslEnum                LOTSZUNITS;
	//END APIQA
	RsslDate		DIVPAYDATE;
	double			TRDPRC_1;
	double			BID;
	double			ASK;
	double			ACVOL_1;
	double			NETCHNG_1;
	RsslDateTime	ASK_TIME;
	double			PERATIO;
	RsslDateTime	SALTIME;
	RsslUInt		updateCount;
} RsslMarketPriceItem;

void initMarketPriceItemFields(RsslMarketPriceItem* mpItem);
void updateMarketPriceItemFields(RsslMarketPriceItem* mpItem);
RsslRet encodeMarketPriceResponse(RsslReactorChannel* pReactorChannel, RsslItemInfo* itemInfo, RsslBuffer* msgBuf, RsslBool isSolicited, RsslInt32 streamId, RsslBool isStreaming, RsslBool isPrivateStream, RsslUInt16 serviceId, RsslDataDictionary* dictionary);
RsslRet updateMarketPriceItemFieldsFromPost(RsslMarketPriceItem* mpItem, RsslDecodeIterator* dIter, RsslErrorInfo *error);

RsslRet encodeMarketPriceResponseMsgInit(RsslItemInfo* itemInfo, RsslEncodeIterator *eIter, RsslBool isSolicited, RsslInt32 streamId, RsslBool isStreaming, RsslBool isPrivateStream, RsslUInt16 serviceId);
RsslRet encodeMPFieldList(RsslItemInfo* itemInfo, RsslEncodeIterator* encodeIter, RsslBool isPrivateStream, RsslDataDictionary* dictionary);
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
        //API QA
	itemInfo->PRCTCK_1 = 0;
	itemInfo->LST_PRCTCK = 0;
	itemInfo->DIVIDENDTP = 0;
	itemInfo->TRD_UNITS = 0;
	itemInfo->MPV = 0;
	itemInfo->LOTSZUNITS = 0;
        //END API QA
	rsslClearDate(&itemInfo->DIVPAYDATE);
	itemInfo->TRDPRC_1 = 0;
	itemInfo->BID = 0;
	itemInfo->ASK = 0;
	itemInfo->ACVOL_1 = 0;
	itemInfo->NETCHNG_1 = 0;
	rsslClearDateTime(&itemInfo->ASK_TIME);
	itemInfo->PERATIO = 0;
	rsslClearDateTime(&itemInfo->SALTIME);
	itemInfo->updateCount = 0;
}

void initMarketPriceItems();
RsslMarketPriceItem* getMarketPriceItem(const char* itemName);
void freeMarketPriceItem(RsslMarketPriceItem* mpItem);

#ifdef __cplusplus
};
#endif

#endif
