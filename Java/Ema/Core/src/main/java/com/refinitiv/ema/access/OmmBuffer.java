///*|-----------------------------------------------------------------------------
// *|            This source code is provided under the Apache 2.0 license      --
// *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
// *|                See the project's LICENSE.md for details.                  --
// *|           Copyright (C) 2019 Refinitiv. All rights reserved.            --
///*|-----------------------------------------------------------------------------

package com.refinitiv.ema.access;

import java.nio.ByteBuffer;

/**
 * OmmBuffer represents a binary buffer value in Omm.
 */
public interface OmmBuffer extends Data
{
	/**
	 * Gets the underlying ByteBuffer.
	 * 
	 * The data length of the buffer is the limit() of the returned ByteBuffer.
	 * @return binary buffer contained in ByteBuffer
	 */
	public ByteBuffer buffer();
}