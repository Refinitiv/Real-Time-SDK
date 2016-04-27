///*|-----------------------------------------------------------------------------
// *|            This source code is provided under the Apache 2.0 license      --
// *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
// *|                See the project's LICENSE.md for details.                  --
// *|           Copyright Thomson Reuters 2015. All rights reserved.            --
///*|-----------------------------------------------------------------------------

package com.thomsonreuters.ema.access;

/**
 * OmmDouble represents double in Omm.
 * 
 * OmmDouble is a read only class.
 * 
 * @see Data
 */
public interface OmmDouble extends Data
{
	/**
	 * Returns Double.
	 * 
	 * @return double
	 */
	public double doubleValue();
}