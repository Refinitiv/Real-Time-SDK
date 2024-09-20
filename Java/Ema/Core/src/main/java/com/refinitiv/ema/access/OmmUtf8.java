///*|-----------------------------------------------------------------------------
// *|            This source code is provided under the Apache 2.0 license
// *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
// *|                See the project's LICENSE.md for details.
// *|           Copyright (C) 2019 LSEG. All rights reserved.     
///*|-----------------------------------------------------------------------------

package com.refinitiv.ema.access;

import java.nio.ByteBuffer;

/**
 * OmmUtf8 represents Utf8 string value in Omm.
 * 
 * <p>Method {@link #string()} can have performance decrease, since it will trigger garbage collection.</p>
 * 
 * OmmUtf8 is a read only class.
 *
 * @see Data
 */
public interface OmmUtf8 extends Data
{
	/**
	 * Returns a buffer that in turn provides an alphanumeric null-terminated hexadecimal string representation.
	 * 
	 * @return ByteBuffer with the object hex information
	 */
	public ByteBuffer buffer();
	 
	/**
	 * Returns a string representation of the class instance.
	 * 
	 * @return string representation of the class instance (e.g., "-1234" )
	 */
	public String string();
}