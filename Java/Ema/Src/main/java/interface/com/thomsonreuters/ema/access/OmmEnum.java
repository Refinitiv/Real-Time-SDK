///*|-----------------------------------------------------------------------------
// *|            This source code is provided under the Apache 2.0 license      --
// *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
// *|                See the project's LICENSE.md for details.                  --
// *|           Copyright Thomson Reuters 2015. All rights reserved.            --
///*|-----------------------------------------------------------------------------

package com.thomsonreuters.ema.access;

/**
 * OmmEnum represents int value in Omm. The enumeration is the meaning of the int value.
 */
public interface OmmEnum extends Data
{
	/**
	 * Returns int.
	 * 
	 * @return int value of Enum
	*/
	public int enumValue();
}