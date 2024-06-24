/*
 * This source code is provided under the Apache 2.0 license and is provided
 * AS IS with no warranty or guarantee of fit for purpose.  See the project's 
 * LICENSE.md for details. 
 * Copyright (C) 2024 LSEG. All rights reserved.
*/

#ifndef _RTR_RSSL_NIMARKETBYORDER_ITEMS_H
#define _RTR_RSSL_NIMARKETBYORDER_ITEMS_H

#include "rtr/rsslTransport.h"
#include "rtr/rsslMessagePackage.h"
#include "rtr/rsslDataPackage.h"
#include "rtr/rsslReactor.h"
#include "rsslNIItemInfo.h"

#ifdef __cplusplus
extern "C" {
#endif

#define MAX_MKT_MKR_ID_STRLEN 64
#define MAX_ORDER_ID_STRLEN 64

#define ORDER_ID_FID	3426
#define ORDER_PRC_FID	3427
#define ORDER_SIDE_FID  3428
#define ORDER_SIZE_FID	3429
#define MKT_MKR_ID_FID	212
#define QUOTIM_MS_FID	3855

/* Enumerated values for the ORDER_SIDE field. Matching enums can be found in enumtype.def */
#define ORDER_BID 1
#define ORDER_ASK 2

/* market by order fields for each order*/
#define MAX_ORDERS 3
typedef struct {
	RsslReal		ORDER_PRC;
	char			MKT_MKR_ID[MAX_MKT_MKR_ID_STRLEN];
	RsslReal		ORDER_SIZE;
	char			ORDER_ID[MAX_ORDER_ID_STRLEN];
	RsslEnum		ORDER_SIDE;
	RsslUInt		QUOTIM_MS;

	/* To demonstrate the container actions "Add," "Update," and "Delete," 
	* Each order follows a cycle where it is first added, then updated some number of times,
	* then deleted, then added again. The 'lifetime' member is used to control this cycle. */
	RsslUInt		lifetime;

} RsslOrderInfo;

/* market by order item data */
typedef struct {
	RsslBool		isInUse;
	RsslOrderInfo	orders[MAX_ORDERS];
} RsslMarketByOrderItem;

void initMarketByOrderItemFields(RsslMarketByOrderItem* mboItem);
void updateMarketByOrderItemFields(RsslMarketByOrderItem* itemInfo);
RsslRet encodeMarketByOrderResponse(RsslReactorChannel* chnl, RsslNIItemInfo* itemInfo, RsslBuffer* msgBuf, RsslInt32 streamId, RsslUInt16 serviceId, RsslDataDictionary* dictionary);
RsslRet encodeMarketByOrderResponseMsgInit(RsslNIItemInfo* itemInfo, RsslEncodeIterator *encodeIter, RsslInt32 streamId, RsslUInt16 serviceId, RsslBool buildRefresh);
RsslRet encodeMarketByOrderMap(RsslMarketByOrderItem* mboItem, RsslEncodeIterator *encodeIter, RsslDataDictionary* dictionary, RsslBool buildRefresh);

/*
 * Clears the item information.
 * itemInfo - The item information to be cleared
 */
RTR_C_INLINE void clearMarketByOrderItem(RsslMarketByOrderItem* itemInfo)
{
	int i;
	itemInfo->isInUse = RSSL_FALSE;
	for(i = 0; i < MAX_ORDERS; ++i)
	{
		RsslOrderInfo *order = &itemInfo->orders[i];

		order->MKT_MKR_ID[0] = '\0';
		order->ORDER_ID[0] = '\0';
		rsslClearReal(&order->ORDER_PRC);
		rsslClearReal(&order->ORDER_SIZE);
		order->ORDER_SIDE = 0;
		order->QUOTIM_MS = 0;
	}
}

void initMarketByOrderItems();
RsslMarketByOrderItem* getMarketByOrderItem(const char* itemName);
void freeMarketByOrderItem(RsslMarketByOrderItem* mboItem);

#ifdef __cplusplus
};
#endif

#endif

