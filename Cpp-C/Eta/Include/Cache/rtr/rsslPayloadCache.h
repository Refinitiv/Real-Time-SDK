/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2019 Refinitiv. All rights reserved.            --
 *|-----------------------------------------------------------------------------
 */

#ifndef __RTR_C_RSSL_PAYLOAD_CACHE_H
#define __RTR_C_RSSL_PAYLOAD_CACHE_H

#include "rtr/rsslTypes.h"
#include "rtr/rsslRetCodes.h"
#include "rtr/rsslDataDictionary.h"

#include "rtr/rsslVAExports.h"

#include "rtr/rsslCacheDefs.h"
#include "rtr/rsslCacheError.h"
#include "rtr/rsslPayloadCacheConfig.h"

#ifdef __cplusplus
extern "C" {
#endif


/**
 *	@addtogroup RSSLVACacheInstance
 *	@{
 */

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
RSSL_VA_API	RsslRet		rsslPayloadCacheInitialize();

/**
 * @brief Uninitialize the Value Add Cache
 * This method will cleanup all cache resources, including cache instances
 * and cache payload entries. Cache resources are uninitialized only
 * if the initialization reference count is zero.
 *
 * @remark thread safe function
 * @see rsslPayloadCacheInitialize
 */
RSSL_VA_API	void		rsslPayloadCacheUninitialize();

/**
 * @brief Indicates if the Value Add Cache component has been initialized
 * @return True if the cache has been initialized
 * @see rsslPayloadCacheInitialize
 */
RSSL_VA_API RsslBool	rsslPayloadCacheIsInitialized();

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
RSSL_VA_API RsslPayloadCacheHandle rsslPayloadCacheCreate(const RsslPayloadCacheConfigOptions* configOptions,
							RsslCacheError* error);

/**
 * @brief Destroy the instance of the cache container.
 * @remark thread safe function
 * @param handle The handle to the cache to be destroyed
 * @see RsslPayloadCacheHandle
 */
RSSL_VA_API	void		rsslPayloadCacheDestroy(RsslPayloadCacheHandle handle);

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
RSSL_VA_API RsslRet		rsslPayloadCacheSetDictionary( RsslPayloadCacheHandle cacheHandle,
							const RsslDataDictionary *rsslDictionary,
							const char* dictionaryKey,
							RsslCacheError* error);

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
RSSL_VA_API RsslRet		rsslPayloadCacheBindDictionary( RsslPayloadCacheHandle cacheHandle,
							const RsslDataDictionary *rsslDictionary,
							const char* dictionaryKey,
							RsslCacheError* error);

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
RSSL_VA_API RsslRet		rsslPayloadCacheSetSharedDictionaryKey( RsslPayloadCacheHandle cacheHandle,
							const char* dictionaryKey,
							RsslCacheError *error);

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
RSSL_VA_API RsslRet		rsslPayloadCacheBindSharedDictionaryKey( RsslPayloadCacheHandle cacheHandle,
							const char* dictionaryKey,
							RsslCacheError *error);

/**
 * @brief Returns the number of payload entries contained in this cache
 * @param cacheHandle The cache instance handle
 * @return The number of entries in this cache instance
 */
RSSL_VA_API	RsslUInt	rsslPayloadCacheGetEntryCount(RsslPayloadCacheHandle cacheHandle);

/**
 * @brief Returns a list of payload entry handles contained in this cache
 * @param cacheHandle The cache instance handle
 * @param arrHandles[] Array where the RsslPayloadEntryHandle list will be written
 * @param arrSize Size of the array provided by the caller
 * @return The number of entry handles written to arrHandles[]
 */
RSSL_VA_API	RsslUInt	rsslPayloadCacheGetEntryList(RsslPayloadCacheHandle cacheHandle,
							RsslPayloadEntryHandle arrHandles[],
							RsslUInt arrSize);

/**
 * @brief Clears the cache by destroying all payload entries
 * @param cacheHandle The cache instance handle
 */
RSSL_VA_API	void		rsslPayloadCacheClearAll(RsslPayloadCacheHandle cacheHandle);

/**
 * @}
 */

#ifdef __cplusplus
}
#endif

#endif
