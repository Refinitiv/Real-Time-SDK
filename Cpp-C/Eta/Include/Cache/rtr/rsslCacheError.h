/*
 * This source code is provided under the Apache 2.0 license and is provided
 * AS IS with no warranty or guarantee of fit for purpose.  See the project's 
 * LICENSE.md for details. 
 * Copyright (C) 2019 Refinitiv. All rights reserved.
*/

#ifndef _RTR_RSSL_CACHE_ERROR_H
#define _RTR_RSSL_CACHE_ERROR_H

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
 * @brief Defines the size of the string text[] in \ref RsslCacheError
 */
#define MAX_OMM_CACHE_ERROR_TEXT 1200

/**
 * @brief Error structure passed to some payload cache interface methods.
 * Populated with information if an error occurs during the function call.
 */
typedef struct
{
	RsslRet			rsslErrorId;	/*!< The RSSL error code */
	char			text[MAX_OMM_CACHE_ERROR_TEXT + 1]; /*!< Additional information about the error */
} RsslCacheError;

/**
 * @brief Clears the RsslCacheError structure
 * @see RsslCacheError
 */
RTR_C_INLINE void rsslCacheErrorClear(RsslCacheError* err)
{
	if (err)
	{
		err->rsslErrorId = RSSL_RET_SUCCESS;
		err->text[0] = '\0';
	}
}

/**
 * @}
 */

#ifdef __cplusplus
}
#endif

#endif
