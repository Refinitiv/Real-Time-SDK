/*
 * This source code is provided under the Apache 2.0 license and is provided
 * AS IS with no warranty or guarantee of fit for purpose.  See the project's 
 * LICENSE.md for details. 
 * Copyright Thomson Reuters 2015. All rights reserved.
*/

/* marketPriceDecoder.h
 * Decodes MarketPrice data and processes any time information present. */

#ifndef _MARKET_PRICE_DECODER_H
#define _MARKET_PRICE_DECODER_H

#include "consumerThreads.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Decodes a MarketPrice message. */
RsslRet decodeMPUpdate(RsslDecodeIterator *pIter, RsslMsg *msg, ConsumerThread* pConsumerThread);

#ifdef __cplusplus
};
#endif

#endif
