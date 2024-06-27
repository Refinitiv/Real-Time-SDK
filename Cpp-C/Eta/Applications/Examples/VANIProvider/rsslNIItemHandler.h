/*
 * This source code is provided under the Apache 2.0 license and is provided
 * AS IS with no warranty or guarantee of fit for purpose.  See the project's 
 * LICENSE.md for details. 
 * Copyright (C) 2019 LSEG. All rights reserved.
*/

#ifndef _RTR_RSSL_NIITEM_HANDLER_H
#define _RTR_RSSL_NIITEM_HANDLER_H

#include "rsslNIProvider.h"
#include "rsslVANIMarketByOrderItems.h"
#include "rsslVANIMarketPriceItems.h"
#include "rsslNIChannelCommand.h"
#include "rsslNIItemInfo.h"
#include "rsslVASendMessage.h"


#ifdef __cplusplus
extern "C" {
#endif

void updateItemInfo(NIChannelCommand *chnlCommand);
static RsslRet sendItemResponse(RsslReactor *pReactor, RsslReactorChannel* chnl, RsslNIItemInfo* itemInfo);
RsslRet sendItemUpdates(RsslReactor *pReactor, RsslReactorChannel* chnl);
RsslRet cacheItemData(NIChannelCommand* chnlCommand);

#ifdef __cplusplus
}
#endif

#endif
