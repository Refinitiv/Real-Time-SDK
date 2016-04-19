///*|-----------------------------------------------------------------------------
// *|            This source code is provided under the Apache 2.0 license      --
// *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
// *|                See the project's LICENSE.md for details.                  --
// *|           Copyright Thomson Reuters 2015. All rights reserved.            --
///*|-----------------------------------------------------------------------------

package com.thomsonreuters.ema.access;

import java.util.Collection;

/**
 * OmmArray is a homogeneous container of primitive data type entries.
 * 
 * <p>
 * OmmArray is a collection which provides iterator over the elements in this
 * collection.
 * </p>
 */
public interface OmmArray extends Data, Collection<OmmArrayEntry>
{
	/**
	 * Indicates presence of FixedWidth.
	 * 
	 * @return true if fixed width is set; false otherwise
	 */
	public boolean hasFixedWidth();

	/**
	 * Returns FixedWidth.
	 * 
	 * @return fixed width
	 */
	public int fixedWidth();

	/**
	 * Clears the OmmArray. Invoking clear() method clears all the values and
	 * resets all the defaults.
	 */
	public void clear();

	/**
	 * Specifies FixedWidth.
	 * 
	 * @throws OmmInvalidUsageException
	 *             if an entry was already added
	 * 
	 * @param width
	 *            specifies fixed width value
	 * @return reference to this object
	 */
	public OmmArray fixedWidth(int width);
}