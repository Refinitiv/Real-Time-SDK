/*
 * This source code is provided under the Apache 2.0 license and is provided
 * AS IS with no warranty or guarantee of fit for purpose.  See the project's 
 * LICENSE.md for details. 
 * Copyright (C) 2019 Refinitiv. All rights reserved.
*/


#ifndef _RTR_RSSL_MARKETBYPRICE_ITEMS_H
#define _RTR_RSSL_MARKETBYPRICE_ITEMS_H

#include "rtr/rsslTransport.h"
#include "rtr/rsslMessagePackage.h"
#include "rsslVAItemEncode.h"

#ifdef __cplusplus
extern "C" {
#endif

#define MAX_REFRESH_PARTS 8

#define MAX_MKT_MKR_ID_STRLEN 64
#define MAX_PRICE_POINT_STRLEN 64

#define ORDER_ID_FID		3426
#define ORDER_PRC_FID		3427
#define ORDER_SIDE_FID		3428
#define ORDER_SIZE_FID		3429
#define MKT_MKR_ID_FID		212
#define QUOTIM_MS_FID		3855
#define CURRENCY_FID		15
#define MKT_STATUS_IND_FID	133
#define ACTIVE_DATE_FID		17
#define NO_ORD_FID			3430
#define	MKOASK_VOL_FID		3839
#define MKOBID_VOL_FID		3840

#define USD_ENUM 840
#define BBO_ENUM 20

/* Enumerated values for the ORDER_SIDE field. Matching enums can be found in enumtype.def */
#define ORDER_BID 1
#define ORDER_ASK 2

/* market by price fields for each order*/
#define MBP_MAX_ORDERS 8
typedef struct {
	RsslReal		ORDER_PRC;
	char			MKT_MKR_ID[MAX_MKT_MKR_ID_STRLEN];
	RsslReal		ORDER_SIZE;
	char			PRICE_POINT[MAX_PRICE_POINT_STRLEN];
	RsslEnum		ORDER_SIDE;
	RsslUInt		QUOTIM_MS;
	RsslUInt		NO_ORD;
	RsslReal		MKOASK_VOL;
	RsslReal		MKOBID_VOL;

	/* To demonstrate the container actions "Add," "Update," and "Delete," 
	* Each order follows a cycle where it is first added, then updated some number of times,
	* then deleted, then added again. The 'lifetime' member is used to control this cycle. */
	RsslUInt		lifetime;

} RsslPriceInfo;

/* market by price item data */
typedef struct {
	RsslBool		isInUse;
	RsslPriceInfo	orders[MBP_MAX_ORDERS];
	RsslUInt		updateCount;
} RsslMarketByPriceItem;

void initMarketByPriceItemFields(RsslMarketByPriceItem* mboItem);
void updateMarketByPriceItemFields(RsslMarketByPriceItem* itemInfo);
RsslRet encodeMarketByPriceRefresh(RsslReactorChannel* pReactorChannel, RsslItemInfo* itemInfo, RsslBuffer* msgBuf, RsslBool isSolicited, RsslInt32 streamId, RsslBool isStreaming, RsslBool isPrivateStream, RsslUInt16 serviceId, RsslDataDictionary* dictionary, int multiPartNo);
RsslRet encodeMarketByPriceUpdate(RsslReactorChannel* pReactorChannel, RsslItemInfo* itemInfo, RsslBuffer* msgBuf, RsslBool isSolicited, RsslInt32 streamId, RsslBool isStreaming, RsslBool isPrivateStream, RsslUInt16 serviceId, RsslDataDictionary* dictionary);

RsslRet encodeMarketByPriceRefreshMsgInit(RsslItemInfo* itemInfo, RsslEncodeIterator *encodeIter, RsslBool isSolicited, RsslInt32 streamId, RsslBool isStreaming, RsslBool isPrivateStream, RsslUInt16 serviceId, int multiPartNo, const char* stateTextStr);
RsslRet encodeMarketByPriceRefreshMap(RsslItemInfo* itemInfo, RsslEncodeIterator *encodeIter, RsslBool isPrivateStream, RsslDataDictionary* dictionary, int multiPartNo);
RsslRet encodeMarketByPriceResponseMsgComplete(RsslEncodeIterator *eIter);

/*
 * Clears the item information.
 * itemInfo - The item information to be cleared
 */
RTR_C_INLINE void clearMarketByPriceItem(RsslMarketByPriceItem* itemInfo)
{
	int i;
	itemInfo->isInUse = RSSL_FALSE;
	for(i = 0; i < MBP_MAX_ORDERS; ++i)
	{
		RsslPriceInfo *order = &itemInfo->orders[i];

		order->MKT_MKR_ID[0] = '\0';
		order->PRICE_POINT[0] = '\0';
		rsslClearReal(&order->ORDER_PRC);
		rsslClearReal(&order->ORDER_SIZE);
		order->ORDER_SIDE = 0;
		order->QUOTIM_MS = 0;
		order->NO_ORD = 0;
		rsslClearReal(&order->MKOASK_VOL);
		rsslClearReal(&order->MKOBID_VOL);
	}
	itemInfo->updateCount = 0;
}

void initMarketByPriceItems();
RsslMarketByPriceItem* getMarketByPriceItem(const char* itemName);
void freeMarketByPriceItem(RsslMarketByPriceItem* mboItem);

#ifdef __cplusplus
};
#endif

#endif

