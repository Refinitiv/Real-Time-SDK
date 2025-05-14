/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2025 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.eta.valueadd.cache;


import java.util.List;

import com.refinitiv.eta.codec.DataDictionary;

/**
 * {@link PayloadCache} is a collection of {@link PayloadEntry} instances that use
 * the same DataDictionary. 
 * @see PayloadEntry
 * @see CacheError
 */
public interface PayloadCache
{
	/**
	 * Sets the RDM Field Dictionary using the given dictionaryKey name
	 * An RDM Field Dictionary (type RWFFld) is required for each cache.
	 *
	 * To share a dictionary with more than one cache, use setSharedDictionaryKey
	 * with the same dictionaryKey after using this function for the initial
	 * setting.
	 *
	 * If this function is called a second time with the same dictionaryKey,
	 * it is handled as a dictionary extension. If the contents of the fidDictionary
	 * are a valid extension of the fidDictionary previously bound by this key,
	 * then these additions will be added to the field database in the cache for this key.
	 * Valid extensions to a dictionary are field additions; changes or deletions
	 * to the field list will cause the extension setting to fail.
	 *
	 * The enum dictionary is not used by the cache, and is not required to be loaded
	 * in the fidDictionary structure.
	 *
	 * @param fidDictionary The dictionary structure containing a decoded RDM Field Dictionary
	 * @param dictionaryKey A string token naming the dictionary being bound to the cache
	 * @param error Error information structure populated if the cache set fails
	 * @return Will return a failure code if the fidDictionary cannot be set to the cache.
	 */
	public int setDictionary( DataDictionary fidDictionary, 
										String dictionaryKey,
									    CacheError error );

	/**
	 * Sets a dictionary to this cache that was previously bound to another cache.
	 * This function enables sharing a dictionary between more than one cache.
	 *
	 * @param dictionaryKey A string token naming the dictionary being set to the cache
	 * @param error Error information structure populated if the set fails
	 * @return Will return a failure code if this dictionaryKey is not known 
	 * (from a previous bind using the bindDictionary function).
	 */
	public int setSharedDictionaryKey( String dictionaryKey, CacheError error );

	/**
	 * Returns the number of payload entries contained in this cache.
	 * 
	 * @return The number of entries in this cache instance
	 */
	public int entryCount();

	/**
	 * Returns a list of payload entries contained in this cache.
	 * 
	 * @return The list of entries in this cache instance
	 */
	public List<PayloadEntry> entryList();

	/**
	 * Destroy the instance of the cache container.
	 * 
	 * This is a thread safe method 
	 */
	public void destroy();
	
	/**
	 * Clears the cache by destroying all payload entries.
	 */
	public void clear();
	
	/**
	 * Destroy all cache instances.
	 * 
	 * This is a thread safe method. <br>
	 * - invalidates all caches, entries and cursors;<br>
	 * - application is responsible for removing references to all caches, entries and cursors;<br>
	 * 
	 */
	public void destroyAll();
}