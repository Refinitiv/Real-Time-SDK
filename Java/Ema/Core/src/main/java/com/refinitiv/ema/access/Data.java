/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2020,2024 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.ema.access;

import java.nio.ByteBuffer;

/**
 * Data is the common interface for all Data type classes.
 * 
 * All classes representing OMM Data implement this interface.
 * 
 * Objects from this interface are intended to be short lived or rather transitional.
 * Objects from this interface are not cache-able.
 */
public interface Data
{
	/**
	 * DataCode represents special state of Data.
	 */
	public static class DataCode
	{
		/**
		 * Indicates no special code.<br>
		 * An application typically processes a valid DataType value.
		 */
		public final static int NO_CODE = 0;
		
	 	/**
		 * Indicates the value is unspecified.<br>
		 * An application typically sets the blank code
		 * when needing to initialize or clear a field.
		 */
		public final static int BLANK = 1;
	}

	/**
	 * Returns the DataCode value in a string format.
	 * 
	 * @return string representation of this object's data code
	 */
	public String codeAsString();

	/**
	 * Returns the DataType, which is the type of Omm data.
	 * 
	 * @return {@link com.refinitiv.ema.access.DataType.DataTypes} integer of this object.
	 */
	public int dataType();

	/**
	 * Returns the Code, which indicates a special state of a DataType.
	 * 
	 * @return {@link com.refinitiv.ema.access.Data.DataCode} integer of this object
	 */
	public int code();

	/**
	 * Returns a buffer that in turn provides an alphanumeric null-terminated
	 * hexadecimal string representation.
	 * 
	 * @return ByteBuffer with this object's hex representation
	 */
	public ByteBuffer asHex();

	/**
	 * Returns a string representation of the class instance.
	 * 
	 * @return string representation of the class instance
	 */
	public String toString();
}