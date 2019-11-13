/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2019 Refinitiv. All rights reserved.            --
 *|-----------------------------------------------------------------------------
 */

#ifndef _RTR_C_RSSL_PAYLOAD_CURSOR_H
#define _RTR_C_RSSL_PAYLOAD_CURSOR_H

#include "rtr/rsslVAExports.h"
#include "rtr/rsslTypes.h"

#include "rtr/rsslCacheDefs.h"


#ifdef __cplusplus
extern "C" {
#endif

/**
 * @addtogroup RSSLVAPayloadCursor
 * @{
 */

/**
 * @brief Create a cache payload cursor for use when retrieving encoded data
 * in multiple parts from a cache payload entry.
 *
 * It is recommended for a cache reader to create a cursor and re-use it
 * (using \ref rsslPayloadCursorClear prior to the first retrieval of an entry).
 * This will be more efficient than creating and destroying a cursor prior to each
 * retrieval.
 *
 * The cursor can only be used on a single payload entry at a time, from
 * first retrieval to last, since it maintains the position for resuming
 * encoding of the next part. If a cache reader needs to interleave
 * retrieval calls across multiple entries, it should use a cursor
 * for each payload entry.
 *
 * @remark thread safe function
 * @return A handle reference to the payload cursor
 */
RSSL_VA_API	RsslPayloadCursorHandle	rsslPayloadCursorCreate();

/**
 * @brief Destroy the payload cursor. The handle is invalid after it is destroyed.
 *
 * @remark thread safe function
 * @param cursorHandle The reference handle to the payload cursor
 */
RSSL_VA_API	void rsslPayloadCursorDestroy(RsslPayloadCursorHandle cursorHandle);

/**
 * @brief Clear the state of the payload cursor. This should be called prior
 * to retrieving the first part from a payload container.
 * @param cursorHandle The reference handle to the payload cursor
 */
RSSL_VA_API	void rsslPayloadCursorClear(RsslPayloadCursorHandle cursorHandle);

/**
 * @brief Returns the completion state of a multi-part payload entry retrieval.
 * @param cursorHandle The reference handle to the payload cursor
 * @return Returns RSSL_TRUE after the last part has been retrieved from the
 * container.
 * @see rsslPayloadEntryRetrieve
 */
RSSL_VA_API	RsslBool rsslPayloadCursorIsComplete(RsslPayloadCursorHandle cursorHandle);

/**
 * @}
 */

#ifdef __cplusplus
}
#endif

#endif

