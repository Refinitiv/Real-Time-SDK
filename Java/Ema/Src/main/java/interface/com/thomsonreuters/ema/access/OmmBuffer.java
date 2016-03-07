///*|-----------------------------------------------------------------------------
// *|            This source code is provided under the Apache 2.0 license      --
// *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
// *|                See the project's LICENSE.md for details.                  --
// *|           Copyright Thomson Reuters 2015. All rights reserved.            --
///*|-----------------------------------------------------------------------------

package com.thomsonreuters.ema.access;

import java.nio.ByteBuffer;

/**
 * OmmBuffer represents a binary buffer value in Omm.
 */
public interface OmmBuffer extends Data
{
	/**
	 * Returns Buffer.
	 * @return binary buffer contained in ByteBuffer
	 */
	public ByteBuffer buffer();
}