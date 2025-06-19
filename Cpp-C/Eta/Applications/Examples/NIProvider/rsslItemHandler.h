/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2015,2022,2024 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

#ifndef _RTR_RSSL_ITEM_HANDLER_H
#define _RTR_RSSL_ITEM_HANDLER_H

#include "rsslNIProvider.h"
#include "rsslItemEncode.h"

#ifdef __cplusplus
extern "C" {
#endif

void initItemHandler();
void resetRefreshComplete();
void addItemName(const char* itemname, RsslUInt8 domainType);
void updateItemInfo();
RsslRet sendItemUpdates(RsslChannel* chnl);
void sendItemCloseStatusMsgs(RsslChannel* chnl);

#ifdef __cplusplus
};
#endif

#endif

