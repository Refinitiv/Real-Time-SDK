/*
 * This source code is provided under the Apache 2.0 license and is provided
 * AS IS with no warranty or guarantee of fit for purpose.  See the project's 
 * LICENSE.md for details. 
 * Copyright (C) 2019 Refinitiv. All rights reserved.
*/

#ifndef _RTR_RSSL_PAYLOAD_CACHE_CONFIG_H
#define _RTR_RSSL_PAYLOAD_CACHE_CONFIG_H

#include "rtr/rsslTypes.h"
#include "rtr/rsslRetCodes.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @addtogroup RSSLVAPayloadCacheUtil
 * @{
 */

/**
 * @brief Configuration options used when creating a payload cache instance.
 * @see rsslPayloadCacheCreate
 */
typedef struct {
	RsslUInt	maxItems;	/*!< Limit on the number of payload entries in this cache. Zero indicates no limit. */
} RsslPayloadCacheConfigOptions;

/**
 * @}
 */

#ifdef __cplusplus
}
#endif

#endif
