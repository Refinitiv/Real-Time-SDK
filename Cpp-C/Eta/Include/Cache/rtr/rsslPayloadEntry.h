/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2019 Refinitiv. All rights reserved.            --
 *|-----------------------------------------------------------------------------
 */

#ifndef _RTR_C_RSSL_PAYLOAD_ENTRY_H
#define _RTR_C_RSSL_PAYLOAD_ENTRY_H

#include "rtr/rsslTypes.h"
#include "rtr/rsslIterators.h"
#include "rtr/rsslMsg.h"
#include "rtr/rsslDataDictionary.h"

#include "rtr/rsslPayloadCache.h"
#include "rtr/rsslPayloadCursor.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @addtogroup RSSLVAPayloadEntry
 * @{
 */

/**
 * @brief Creates a payload entry within the specified cache
 * @param cacheHandle Handle to the cache where this entry will be created
 * @param error Error information structure populated if the cache entry cannot be created
 * @return Returns a handle to the OMM data payload entry, or 0 if an entry could not be created
 * @see RsslPayloadCacheHandle RsslCacheError RsslPayloadEntryHandle
 */
RSSL_VA_API RsslPayloadEntryHandle rsslPayloadEntryCreate(RsslPayloadCacheHandle cacheHandle, RsslCacheError *error);

/**
 * @brief Removes the payload entry from its cache and destroys the entry.
 * The handle is no longer valid after calling this function.
 * @param handle The payload entry reference handle
 * @see RsslPayloadEntryHandle
 */
RSSL_VA_API void rsslPayloadEntryDestroy(RsslPayloadEntryHandle handle);

/**
 * @brief Clears the OMM data contents of this entry. The entry remains in the cache and can be re-used.
 * @param handle The payload entry reference handle
 * @see RsslPayloadEntryHandle
 */
RSSL_VA_API void rsslPayloadEntryClear(RsslPayloadEntryHandle handle);

/**
 * @brief Returns the payload entry data type
 * @param handle The payload entry reference handle
 * @return The OMM data type of this container from the RsslDataTypes enumeration.
 * When the entry is first created, or cleared, returns RSSL_DT_UNKNOWN.
 * @see RsslPayloadEntryHandle
 */
RSSL_VA_API RsslContainerType rsslPayloadEntryGetDataType(RsslPayloadEntryHandle handle);

/**
 * @brief Applies the partially decoded RsslMsg to the payload entry. The
 * OMM container in the message payload is applied to the cache entry
 * following the OMM rules of the message and the container type.
 *
 * When this function returns an error ( < RSSL_RET_SUCCESS), some or all
 * of the data could not be cached. Some errors returned by this function
 * can be handled as warnings, depending on application requirements. When
 * the function returns an error code, the RsslCacheError.rsslErrorId can be
 * tested for a warning indication.  For errors, the rsslErrorId will have the
 * same value as the function return value. For warnings, rsslErrorId will be
 * set to RSSL_RET_SUCCESS. The RsslCacheError.text can be reviewed for further
 * explanation about the errors or warnings. Examples of warning conditions: data
 * truncated for exceeding size allocated in the field dictionary; fields
 * in an RsslUpdateMsg ignored if they were not in the RsslRefreshMsg.
 *
 * @param handle The payload entry reference handle
 * @param dIter The decode iterator positioned at the payload of the message. This iterator
 * will be used for decoding as the OMM data is written to the cache entry.
 * @param msg The partially decoded message structure
 * @param errorInfo The error information structure will be populated if the
 * payload data from the message could not be written to the cache entry
 * @return Returns failure codes if the data could not be applied to the cache entry
 * @see RsslPayloadEntryHandle RsslDecodeIterator RsslMsg RsslCacheError
 */
RSSL_VA_API RsslRet rsslPayloadEntryApply(RsslPayloadEntryHandle handle,
			RsslDecodeIterator *dIter,
			RsslMsg *msg,
			RsslCacheError *errorInfo);

/**
 * @brief Retrieves encoded OMM data from the cache payload entry.
 *
 * This function encodes data from the cache entry into the buffer provided
 * with the eIter parameter. The retrieval process will attempt to encode
 * the entire container into the given buffer, but also supports
 * multi-part retrieval for OMM containers that support fragmentation.
 * (Note that FieldList and ElementList containers must be retrieved
 * in a single part). If the application does not wish to use fragmentation,
 * it should pass a null RsslPayloadCursorHandle.
 *
 * For multi-part retrieval, an RsslPayloadCursorHandle is required. The
 * application will call this function to retrieve fragments of the cached
 * container until the cursor indicates the final part has been retrieved
 * (\ref rsslPayloadCursorIsComplete). The size of the buffer provided
 * for retrieval will determine the number of fragments needed to encode
 * the entire container.
 *
 * @param handle The payload entry reference handle
 * @param eIter The OMM payload data in the cache entry will be encoded into the
 * buffer associated with this iterator.
 * @param cursorHandle The cursor handle is required to support multi-part (fragmented)
 * retrieval of the OMM data in the cache entry. Clear the cursor (\ref rsslPayloadCursorClear)
 * prior to the first retrieval from the payload entry. The cursor is not needed for
 * single part retrieval.
 * @param errorInfo The error in formation structure will be populated if payload
 * entry cache data could not be retrieved.
 * @return Failure codes ( < RSSL_RET_SUCCESS) if data could not be retrieved from the container.
 * RSSL_RET_BUFFER_TOO_SMALL indicates that the buffer size should be increased
 * in order to retrieve the data from the entry (single part or multi-part).
 * RSSL_RET_SUCCESS is returned when data is successfully encoded into the buffer,
 * or if there is no data to retrieve from the entry. Note that for multi-part
 * retrieval, RSSL_RET_SUCCESS only indicates the successful encoding of the current
 * fragment. Check the state of the RsslPayloadCursorHandle to determine if the current
 * fragment is the final part.
 * @see RsslPayloadEntryHandle RsslEncodeIterator RsslPayloadCursorHandle RsslCacheError
 */
RSSL_VA_API RsslRet rsslPayloadEntryRetrieve(RsslPayloadEntryHandle handle,
			RsslEncodeIterator *eIter,
			RsslPayloadCursorHandle cursorHandle,
			RsslCacheError *errorInfo);

/**
 * @brief Trace format option to get a decoded output of the payload entry data in XML format
 */
#define PAYLOAD_ENTRY_TRACE_OPTION_XML 1

/**
 * @brief Display the cache payload entry contents by writing to the given file handle.
 * @param handle The payload entry reference handle
 * @param traceFormat Option for specifying the type of information displayed for the entry
 * @param file The file handle where the trace information will be written
 * @param dictionary The dictionary structure used to decode the OMM data in the cache entry
 * @return Returns failure code if trace function fails
 */
RSSL_VA_API RsslRet rsslPayloadEntryTrace(RsslPayloadEntryHandle handle,
			RsslInt traceFormat,
			FILE *file,
			RsslDataDictionary *dictionary);

/**
 * @}
 */

#ifdef __cplusplus
}
#endif

#endif

