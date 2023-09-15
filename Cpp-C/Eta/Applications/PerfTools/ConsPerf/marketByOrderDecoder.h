/*
 * This source code is provided under the Apache 2.0 license and is provided
 * AS IS with no warranty or guarantee of fit for purpose.  See the project's 
 * LICENSE.md for details. 
 * Copyright (C) 2019 Refinitiv. All rights reserved.
*/

/* marketByOrderDecoder.h
 * Decodes MarketByOrder data and processes any time information present. */

#ifndef _MARKET_BY_ORDER_DECODER_H
#define _MARKET_BY_ORDER_DECODER_H

#include "consumerThreads.h"
#include "jsonHandler.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Decodes a MarketByOrder message. */
RsslRet decodeMBOUpdate(RsslDecodeIterator *pIter, RsslMsg *msg, ConsumerThread* pConsumerThread);

/* Decodes a MarketByOrder JSON message. */
RsslRet decodeMBOUpdateJson(ConsumerThread* pConsumerThread, RsslMsgClasses rsslMsgClass, cJSON* json);

#ifdef __cplusplus
};
#endif

#endif
