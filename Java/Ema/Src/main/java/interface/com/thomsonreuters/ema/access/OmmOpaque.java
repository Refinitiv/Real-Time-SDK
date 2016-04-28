///*|-----------------------------------------------------------------------------
// *|            This source code is provided under the Apache 2.0 license      --
// *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
// *|                See the project's LICENSE.md for details.                  --
// *|           Copyright Thomson Reuters 2015. All rights reserved.            --
///*|-----------------------------------------------------------------------------

package com.thomsonreuters.ema.access;

import java.nio.ByteBuffer;

/**
 * OmmOpaque represents Opaque data format in Omm.
 * 
 * Objects of this class are intended to be short lived or rather transitional.<br>
 * This class is designed to efficiently perform setting and extracting of Opaque and its content.<br>
 * Objects of this class are not cache-able.
 *
 * @see Data
 */
public interface OmmOpaque extends ComplexType
{
	/** 
	 * Returns a string representation of the class instance.
	 * @return string representation of the class instance
	 */
	public String string();

	/**
	 * Returns Opaque buffer.
	 * @return ByteBuffer containing the Opaque data
	 */
	public ByteBuffer buffer();

	/**
	 * Clears the OmmOpaque.<br>
	 * Invoking clear() method clears all the values and resets all the defaults.
	 * @return reference to this object
	 */
	public OmmOpaque clear();

	/**
	 * Specifies Set.
	 * @param value specifies Opaque data using ByteBuffer
	 * @return reference to this object
	 */
	public OmmOpaque buffer(ByteBuffer value);

	/**
	 * Specifies Set.
	 * @param value specifies Opaque data using String
	 * @return reference to this object
	 */
	public OmmOpaque string(String value);
}