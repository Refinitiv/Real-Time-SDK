#include "rtr/rsslCacheDefs.h"
#include "rtr/rsslPayloadCache.h"
#include "rtr/rsslPayloadCursor.h"
#include "rtr/rsslPayloadEntry.h"

#include "rtr/rsslTypes.h"
#include "rtr/rsslRetCodes.h"
#include "rtr/rsslDataDictionary.h"
#include "rtr/rsslVAExports.h"
#include "rtr/rsslCacheError.h"
#include "rtr/rsslCacheError.h"
#include "rtr/rsslPayloadCacheConfig.h"
#include "rtr/rsslIterators.h"
#include "rtr/rsslMsg.h"

////////////////////////////////////	rsslPayloadCache.h	/////////////////////////////////////
/**
 * @brief Initializes the Value Add Cache component.
 * Call this prior to using any other methods in the Value Add Cache.
 *
 * This method only needs to be called one time for an application.
 * A reference count is kept for the number of calls to initialize. An
 * equal number of uninitialize calls must be made before the component
 * is uninitialized.
 *
 * @remark thread safe function
 * @return failure code if cache fails to initialize
 * @see rsslPayloadCacheUninitialize
 */
RsslRet rsslPayloadCacheInitialize()
{
	return RSSL_RET_FAILURE;
}

/**
 * @brief Uninitialize the Value Add Cache
 * This method will cleanup all cache resources, including cache instances
 * and cache payload entries. Cache resources are uninitialized only
 * if the initialization reference count is zero.
 *
 * @remark thread safe function
 * @see rsslPayloadCacheInitialize
 */
void rsslPayloadCacheUninitialize()
{
	return;
}

/**
 * @brief Indicates if the Value Add Cache component has been initialized
 * @return True if the cache has been initialized
 * @see rsslPayloadCacheInitialize
 */
RsslBool rsslPayloadCacheIsInitialized()
{
	return RSSL_FALSE;
}

/**
 * @brief Creates an instance of a payload cache container.
 * All operations on the container require the RsslPayloadCacheHandle.
 *
 * @remark thread safe function
 * @param configOptions The options for configuring this cache container
 * @param error Error information populated if this function fails
 * @return The handle to the payload cache instance
 * @see RsslPayloadCacheHandle RsslPayloadCacheConfigOptions RsslCacheError
 */
RsslPayloadCacheHandle rsslPayloadCacheCreate(const RsslPayloadCacheConfigOptions* configOptions, RsslCacheError* error)
{
	return RSSL_RET_FAILURE;
}

/**
 * @brief Destroy the instance of the cache container.
 * @remark thread safe function
 * @param handle The handle to the cache to be destroyed
 * @see RsslPayloadCacheHandle
 */
void rsslPayloadCacheDestroy(RsslPayloadCacheHandle handle)
{
	return;
}

/**
 * @brief Sets the RDM Field Dictionary using the given dictionaryKey name
 * An RDM Field Dictionary (type RWFFld) is required for each cache.
 *
 * To share a dictionary with more than one cache, use rsslPayloadCacheSetSharedDictionaryKey
 * with the same dictionaryKey after using this function for the initial
 * setting.
 *
 * If this function is called a second time with the same dictionaryKey,
 * it is handled as a dictionary extension. If the contents of the rsslDictionary
 * are a valid extension of the dictionary previously set by this key,
 * then these additions will be added to the field database in the cache for this key.
 * Valid extensions to a dictionary are field additions; changes or deletions
 * to the field list will cause the extension setting to fail.
 *
 * The enum dictionary is not used by the cache, and is not required to be loaded
 * in the rsslDictionary structure.
 *
 * @param cacheHandle The handle to the cache which will be associated with this dictionary
 * @param rsslDictionary The dictionary structure containing a decoded RDM Field Dictionary
 * @param dictionaryKey A string token naming the dictionary being set to the cache
 * @param error Error information struct populated if the cache set fails
 * @return Will return a failure code if the dictionary cannot be set to the cache.
 * @see RsslPayloadCacheHandle RsslDataDictionary RsslCacheError
 */
RsslRet rsslPayloadCacheSetDictionary(RsslPayloadCacheHandle cacheHandle, const RsslDataDictionary *rsslDictionary, const char* dictionaryKey, RsslCacheError* error)
{
	return RSSL_RET_FAILURE;
}

/**
 * @brief DEPRECATED: Binds the RDM Field Dictionary using the given dictionaryKey name
 * An RDM Field Dictionary (type RWFFld) is required for each cache.
 * Users should migrate to use rsslPayloadCacheSetDictionary.
 *
 * To share a dictionary with more than one cache, use rsslPayloadCacheBindSharedDictionaryKey
 * with the same dictionaryKey after using this function for the initial
 * binding.
 *
 * If this function is called a second time with the same dictionaryKey,
 * it is handled as a dictionary extension. If the contents of the rsslDictionary
 * are a valid extension of the dictionary previously bound by this key,
 * then these additions will be added to the field database in the cache for this key.
 * Valid extensions to a dictionary are field additions; changes or deletions
 * to the field list will cause the extension binding to fail.
 *
 * The enum dictionary is not used by the cache, and is not required to be loaded
 * in the rsslDictionary structure.
 *
 * @param cacheHandle The handle to the cache which will be associated with this dictionary
 * @param rsslDictionary The dictionary structure containing a decoded RDM Field Dictionary
 * @param dictionaryKey A string token naming the dictionary being bound to the cache
 * @param error Error information struct populated if the cache bind fails
 * @return Will return a failure code if the dictionary cannot be bound to the cache.
 * @see RsslPayloadCacheHandle RsslDataDictionary RsslCacheError
 */
RsslRet rsslPayloadCacheBindDictionary(RsslPayloadCacheHandle cacheHandle, const RsslDataDictionary *rsslDictionary, const char* dictionaryKey, RsslCacheError* error)
{
	return RSSL_RET_FAILURE;
}

/**
 * @brief Set a dictionary to this cache that was previously set to another cache.
 * This function enables sharing a dictionary between more than one cache.
 *
 * @param cacheHandle The handle to the cache which will be associated with this dictionary
 * @param dictionaryKey A string token naming the dictionary being set to the cache
 * @param error Error information struct populated if the set fails
 * @return Will return a failure code if this dictionaryKey is not known (from a previous bind
 * using the rsslPayloadCacheSetDictionary function).
 * @see RsslPayloadCacheHandle RsslCacheError
 */
RsslRet rsslPayloadCacheSetSharedDictionaryKey(RsslPayloadCacheHandle cacheHandle, const char* dictionaryKey, RsslCacheError *error)
{
	return RSSL_RET_FAILURE;
}

/**
 * @brief DEPRECATED: Bind a dictionary to this cache that was previously bound to another cache.
 * Users should migrate to use rsslPayloadCacheSetSharedDictionaryKey.
 * This function enables sharing a dictionary between more than one cache.
 *
 * @param cacheHandle The handle to the cache which will be associated with this dictionary
 * @param dictionaryKey A string token naming the dictionary being bound to the cache
 * @param error Error information struct populated if the bind fails
 * @return Will return a failure code if this dictionaryKey is not known (from a previous bind
 * using the rsslPayloadCacheSetDictionary function).
 * @see RsslPayloadCacheHandle RsslCacheError
 */
RsslRet rsslPayloadCacheBindSharedDictionaryKey(RsslPayloadCacheHandle cacheHandle, const char* dictionaryKey, RsslCacheError *error)
{
	return RSSL_RET_FAILURE;
}

/**
 * @brief Returns the number of payload entries contained in this cache
 * @param cacheHandle The cache instance handle
 * @return The number of entries in this cache instance
 */
RsslUInt rsslPayloadCacheGetEntryCount(RsslPayloadCacheHandle cacheHandle)
{
	return 0;
}

/**
 * @brief Returns a list of payload entry handles contained in this cache
 * @param cacheHandle The cache instance handle
 * @param arrHandles[] Array where the RsslPayloadEntryHandle list will be written
 * @param arrSize Size of the array provided by the caller
 * @return The number of entry handles written to arrHandles[]
 */
RsslUInt rsslPayloadCacheGetEntryList(RsslPayloadCacheHandle cacheHandle, RsslPayloadEntryHandle arrHandles[], RsslUInt arrSize)
{
	return 0;
}

/**
 * @brief Clears the cache by destroying all payload entries
 * @param cacheHandle The cache instance handle
 */
void rsslPayloadCacheClearAll(RsslPayloadCacheHandle cacheHandle)
{
	return;
}

////////////////////////////////////	rsslPayloadCursor.h	/////////////////////////////////////
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
RsslPayloadCursorHandle rsslPayloadCursorCreate()
{
	return RSSL_RET_FAILURE;
}

/**
 * @brief Destroy the payload cursor. The handle is invalid after it is destroyed.
 *
 * @remark thread safe function
 * @param cursorHandle The reference handle to the payload cursor
 */
void rsslPayloadCursorDestroy(RsslPayloadCursorHandle cursorHandle)
{
	return;
}

/**
 * @brief Clear the state of the payload cursor. This should be called prior
 * to retrieving the first part from a payload container.
 * @param cursorHandle The reference handle to the payload cursor
 */
void rsslPayloadCursorClear(RsslPayloadCursorHandle cursorHandle)
{
	return;
}

/**
 * @brief Returns the completion state of a multi-part payload entry retrieval.
 * @param cursorHandle The reference handle to the payload cursor
 * @return Returns RSSL_TRUE after the last part has been retrieved from the
 * container.
 * @see rsslPayloadEntryRetrieve
 */
RsslBool rsslPayloadCursorIsComplete(RsslPayloadCursorHandle cursorHandle)
{
	return RSSL_FALSE;
}

////////////////////////////////////	rsslPayloadEntry.h	/////////////////////////////////////
/**
 * @brief Creates a payload entry within the specified cache
 * @param cacheHandle Handle to the cache where this entry will be created
 * @param error Error information structure populated if the cache entry cannot be created
 * @return Returns a handle to the OMM data payload entry, or 0 if an entry could not be created
 * @see RsslPayloadCacheHandle RsslCacheError RsslPayloadEntryHandle
 */
RsslPayloadEntryHandle rsslPayloadEntryCreate(RsslPayloadCacheHandle cacheHandle, RsslCacheError *error)
{
	return RSSL_RET_FAILURE;
}

/**
 * @brief Removes the payload entry from its cache and destroys the entry.
 * The handle is no longer valid after calling this function.
 * @param handle The payload entry reference handle
 * @see RsslPayloadEntryHandle
 */
void rsslPayloadEntryDestroy(RsslPayloadEntryHandle handle)
{
	return;
}

/**
 * @brief Clears the OMM data contents of this entry. The entry remains in the cache and can be re-used.
 * @param handle The payload entry reference handle
 * @see RsslPayloadEntryHandle
 */
void rsslPayloadEntryClear(RsslPayloadEntryHandle handle)
{
	return;
}

/**
 * @brief Returns the payload entry data type
 * @param handle The payload entry reference handle
 * @return The OMM data type of this container from the RsslDataTypes enumeration.
 * When the entry is first created, or cleared, returns RSSL_DT_UNKNOWN.
 * @see RsslPayloadEntryHandle
 */
RsslContainerType rsslPayloadEntryGetDataType(RsslPayloadEntryHandle handle)
{
	return RSSL_RET_FAILURE;
}

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
RsslRet rsslPayloadEntryApply(RsslPayloadEntryHandle handle, RsslDecodeIterator *dIter, RsslMsg *msg, RsslCacheError *errorInfo)
{
	return RSSL_RET_FAILURE;
}

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
RsslRet rsslPayloadEntryRetrieve(RsslPayloadEntryHandle handle, RsslEncodeIterator *eIter, RsslPayloadCursorHandle cursorHandle, RsslCacheError *errorInfo)
{
	return RSSL_RET_FAILURE;
}

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
RsslRet rsslPayloadEntryTrace(RsslPayloadEntryHandle handle, RsslInt traceFormat, FILE *file, RsslDataDictionary *dictionary)
{
	return RSSL_RET_FAILURE;
}
