/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2020,2025 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.eta.valueadd.cache;


/**
 * {@link PayloadCursor} is a utility class to allow retrieving encoded data
 * in multiple parts from a cache payload entry.
 * 
 * It is recommended for a cache reader to create a cursor and re-use it
 * (using clear prior to the first retrieval of an entry).
 * This will be more efficient than creating and destroying a cursor prior to each
 * retrieval.
 *
 * The cursor can only be used on a single payload entry at a time, from
 * first retrieval to last. If a cache reader needs to interleave retrieval 
 * calls across multiple entries, it should use a cursor for each payload entry.
 * 
 * @see PayloadCache
 * @see PayloadEntry
 */
public interface PayloadCursor
{
	/**
	 * Destroy the instance of the payload cursor.
	 * This is a thread safe method.
	 */
	public void destroy();

	/** 
	 * Clear the state of the payload cursor. This should be called prior
	 * to retrieving the first part from a payload entry.
	 * 
	 */
	public void clear();
	
	/** 
	 * Returns the completion state of a multi-part payload entry retrieval.
	 * @return Returns true after the last part has been retrieved from the
	 * entry.
	 */
	public boolean isComplete();
}