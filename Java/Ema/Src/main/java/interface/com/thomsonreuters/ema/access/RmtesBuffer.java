///*|-----------------------------------------------------------------------------
// *|            This source code is provided under the Apache 2.0 license      --
// *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
// *|                See the project's LICENSE.md for details.                  --
// *|           Copyright Thomson Reuters 2015. All rights reserved.            --
///*|-----------------------------------------------------------------------------

package com.thomsonreuters.ema.access;

import java.nio.CharBuffer;

/**
 * RmtesBuffer represents RMTES data. RmtesBuffer stores and applies RMTES data.
 * <p>
 * Method {@link #asUTF16()} will trigger garbage
 * collection.
 * </p>
 * 
 * <p>
 * RmtesBuffer class contains a copy of the buffer passed on apply methods.
 * </p>
 */
public interface RmtesBuffer
{
	/**
	 * Returns the content converted as UTF16.
	 * 
	 * @throws OmmInvalidUsageException
	 *             if fails to convert
	 * 
	 * @return CharBuffer containing RMTES data converted to UTF16
	 */
	public CharBuffer asUTF16();

	/**
	 * Returns a string representation of decoded RMTES data.
	 * 
	 * @throws OmmInvalidUsageException
	 *             if fails to convert
	 * 
	 * @return String containing decoded RMTES data
	 */
	public String toString();

	/**
	 * Clears contained content.
	 * 
	 * @return reference to this object
	 */
	public RmtesBuffer clear();

	/**
	 * Apply passed in RMTES data
	 * 
	 * @throws OmmInvalidUsageException
	 *             if fails to apply
	 * 
	 * @param data
	 *            buf specifies RmtesBuffer to be applied to this object
	 * @return reference to this object
	 */
	public RmtesBuffer apply(RmtesBuffer data);

	/**
	 * Apply passed in RMTES data
	 * 
	 * @throws OmmInvalidUsageException
	 *             if fails to apply
	 * 
	 * @param data
	 *            specifies OmmRmtes containing RMTES data to be applied to this
	 *            object
	 * @return reference to this object
	 */
	public RmtesBuffer apply(OmmRmtes data);
}