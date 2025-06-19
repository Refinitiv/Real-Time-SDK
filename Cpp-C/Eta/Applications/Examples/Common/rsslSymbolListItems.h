/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2015,2022,2024 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

#ifndef _RTR_RSSL_SYMBOLLIST_ITEMS_H
#define _RTR_RSSL_SYMBOLLIST_ITEMS_H

#include "rtr/rsslTransport.h"
#include "rtr/rsslMessagePackage.h"
#include "rsslItemEncode.h"

#ifdef __cplusplus
extern "C" {
#endif

#define MAX_SL_ITEM_NAME_SIZE 128

/* symbol list item data */
typedef struct {
	RsslBool		isInUse;
	RsslUInt32		interestCount;
	char			itemName[MAX_SL_ITEM_NAME_SIZE];
} RsslSymbolListItem;

/* Function that handles encoding the symbol list response */
RsslRet encodeSymbolListResponse(RsslChannel* chnl, RsslItemInfo* itemInfo, RsslBuffer* msgBuf, RsslBool isSolicited, RsslInt32 streamId, RsslBool isStreaming, RsslUInt16 serviceId, RsslDataDictionary* dictionary, char* itemName, RsslUInt8 responseType);

/* Global function used to initialize the symbol list items */
void initSymbolListItemList();

/*  Function for removing items in the symbol list */
void clearSymbolListItem(RsslUInt32 itemNum);

RsslBool getSymbolListItemStatus(int itemNum);

char* getSymbolListItemName(int itemNum);

void setSymbolListItemInfo(char* itemName, int itemNum);

void incrementItemCount();

void decrementItemCount();

RsslUInt32 getItemCount();

void incrementInterestCount(int itemNum);

void decrementInterestCount(int itemNum);

RsslUInt32 getInterestCount(int itemNum);


#ifdef __cplusplus
};
#endif

#endif
