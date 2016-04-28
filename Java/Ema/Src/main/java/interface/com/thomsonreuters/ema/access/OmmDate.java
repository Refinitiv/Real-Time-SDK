///*|-----------------------------------------------------------------------------
// *|            This source code is provided under the Apache 2.0 license      --
// *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
// *|                See the project's LICENSE.md for details.                  --
// *|           Copyright Thomson Reuters 2015. All rights reserved.            --
///*|-----------------------------------------------------------------------------

package com.thomsonreuters.ema.access;

/**
 * OmmDate represents Date info in Omm.
 * <br>OmmDate encapsulates year, month and day information.
 * 
 * OmmDate is a read only class.
 * 
 * @see Data
 */
public interface OmmDate extends Data
{
	/**
	 * Returns Year.
	 * @return value of year
	*/
	public int year();
	
	/**
	 * Returns Month.
	 * @return value of month
	*/
	public int month();
	
	/**
	 * Returns Day.
	 * @return value of day
	*/
	public int day();
}