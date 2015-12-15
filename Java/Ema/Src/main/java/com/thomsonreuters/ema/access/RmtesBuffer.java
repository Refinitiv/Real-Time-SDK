///*|-----------------------------------------------------------------------------
// *|            This source code is provided under the Apache 2.0 license      --
// *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
// *|                See the project's LICENSE.md for details.                  --
// *|           Copyright Thomson Reuters 2015. All rights reserved.            --
///*|-----------------------------------------------------------------------------

package com.thomsonreuters.ema.access;

import java.nio.ByteBuffer;
import java.nio.CharBuffer;

/**
 * RmtesBuffer represents RMTES data.
 * RmtesBuffer stores and applies RMTES data.
 * <p>Methods {@link #asUTF8()} and {@link #asUTF16()} will trigger garbage collection.</p>
 * 
 * <p>RmtesBuffer class contains a copy of the buffer passed on apply methods.</p>
 */
public interface RmtesBuffer
{
	/** 
	 * Returns the content converted as UTF8.
	 * 
	 * @throw OmmInvalidUsageException if fails to convert
	 * 
	 * @return CharBuffer containing RMTES data converted to UTF8
	 */
	public CharBuffer asUTF8();
	
	/** 
	 * Puts content converted as UTF8 into passed in CharBuffer.
	 * Client allocates memory of CharBuffer.
	 * 
	 * @throw OmmInvalidUsageException if fails to convert
	 * 
	 * @return void
	 */
	public void asUTF8(CharBuffer outBuffer);

	/** Returns the content converted as UTF16.
	 * 
	 * @throw OmmInvalidUsageException if fails to convert
	 * 
	 * @return CharBuffer containing RMTES data converted to UTF16
	 */
	public CharBuffer asUTF16();
	
	/** 
	 * Puts content converted as UTF16 into passed in CharBuffer.
	 * Client allocates memory of CharBuffer.
	 * 
	 * @throw OmmInvalidUsageException if fails to convert
	 * 
	 * @return void
	 */
	public void asUTF16(CharBuffer outBuffer);

	/**
	 * Returns a string representation of the class instance which is converted to UTF8.
	 * 
	 * @throw OmmInvalidUsageException if fails to convert
	 * 
	 * @return String containing RMTES data converted to UTF8
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
	 * @throw OmmInvalidUsageException if fails to apply
	 * 
	 * @param data buf specifies RmtesBuffer to be applied to this object
	 * @return reference to this object
	 */
	public RmtesBuffer apply(RmtesBuffer data);

	/**
	 * Apply passed in RMTES data
	 * 
	 * 	@throw OmmInvalidUsageException if fails to apply
	 * 
	 * @param data specifies OmmRmtes containing RMTES data to be applied to this object
	 * @return reference to this object
	 */
	public RmtesBuffer apply(OmmRmtes data);
	
	/**
	 * Hold encoded RMTES data from a preallocated ByteBuffer.
	 * 
	 * @param buffer preallocated ByteBuffer
	 * @return reference to this object
	 */
	public RmtesBuffer setBackingBuffer(ByteBuffer buffer);
}