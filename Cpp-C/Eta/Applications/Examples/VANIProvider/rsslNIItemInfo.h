/*
 * This source code is provided under the Apache 2.0 license and is provided
 * AS IS with no warranty or guarantee of fit for purpose.  See the project's 
 * LICENSE.md for details. 
 * Copyright (C) 2019 LSEG. All rights reserved.
*/

#ifndef _RTR_RSSL_NIITEM_INFO_H
#define _RTR_RSSL_NIITEM_INFO_H

#include "rtr/rsslReactor.h"

#include "rtr/rsslPayloadEntry.h"

#ifdef __cplusplus
extern "C" {
#endif


/* item information. */
typedef struct {
	RsslBool		IsRefreshComplete;
	char			Itemname[128];
	RsslUInt8		domainType;
	RsslInt32 		streamId;
	RsslBool 		isActive;
	void*			itemData; /* Holds information about the item's data. This data will be different depending on the domain of the item. */
	RsslPayloadEntryHandle cacheEntryHandle;
} RsslNIItemInfo;

/*
 * Clears the item information.
 * itemInfo - The item information to be cleared
 */
RTR_C_INLINE void clearNIItemInfo(RsslNIItemInfo* itemInfo)
{
	itemInfo->IsRefreshComplete = RSSL_FALSE;
	itemInfo->Itemname[0] = '\0';
	itemInfo->isActive = RSSL_FALSE;
	itemInfo->itemData = 0;
	itemInfo->streamId = 0;
	itemInfo->domainType = 0;
	if (itemInfo->cacheEntryHandle != 0)
		rsslPayloadEntryDestroy(itemInfo->cacheEntryHandle);
	itemInfo->cacheEntryHandle = 0;
}

#ifdef __cplusplus
}
#endif

#endif

