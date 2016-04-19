///*|-----------------------------------------------------------------------------
// *|            This source code is provided under the Apache 2.0 license      --
// *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
// *|                See the project's LICENSE.md for details.                  --
// *|           Copyright Thomson Reuters 2015. All rights reserved.            --
///*|-----------------------------------------------------------------------------

package com.thomsonreuters.ema.access;

/**
 * OmmTime represents Time info in Omm.
 * <br>OmmTime encapsulates hour, minute, second, millisecond, microsecond and nanosecond information.
 */
public interface OmmTime extends Data
{
	/**
	 * Returns Hour.
	 * 
	 * @return int
	 */
	public int hour();

	/**
	 * Returns Minute.
	 * 
	 * @return int
	 */
	public int minute();

	/**
	 * Returns Second.
	 * 
	 * @return int
	 */
	public int second();

	/**
	 * Returns Millisecond.
	 * 
	 * @return int
	 */
	public int millisecond();

	/**
	 * Returns Microsecond.
	 * 
	 * @return int
	 */
	public int microsecond();

	/**
	 * Returns Nanosecond.
	 * 
	 * @return int
	 */
	public int nanosecond();
}