/*
 * This source code is provided under the Apache 2.0 license and is provided
 * AS IS with no warranty or guarantee of fit for purpose.  See the project's 
 * LICENSE.md for details. 
 * Copyright Thomson Reuters 2015. All rights reserved.
*/


#ifndef _RTR_RSSL_MARKET_BY_ORDER_HANDLER_H
#define _RTR_RSSL_MARKET_BY_ORDER_HANDLER_H

#ifdef __cplusplus
extern "C" {
#endif

#include "rtr/rsslTransport.h"
#include "rtr/rsslMessagePackage.h"

/* defines maximum allowed name length for this application */
#define MAX_ITEM_NAME_STRLEN 128

/* called to process market by order responses */
RsslRet processMarketByOrderResponse(RsslMsg* msg, RsslDecodeIterator* dIter);

#ifdef __cplusplus
};
#endif

#endif
