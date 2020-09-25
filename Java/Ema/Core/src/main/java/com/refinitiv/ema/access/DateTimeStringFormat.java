///*|-----------------------------------------------------------------------------
// *|            This source code is provided under the Apache 2.0 license      --
// *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
// *|                See the project's LICENSE.md for details.                  --
// *|           Copyright (C) 2019 Refinitiv. All rights reserved.            --
///*|-----------------------------------------------------------------------------

/**
 * DateTimeStringFormat is an interface to string conversion methods for OmmDate, OmmTime & OmmDateTime.
 * <br>DateTimeStringFormat provides interface to methods that convert OmmDate/OmmTime/OmmDateTime
 * into string based on a specified format.
 *
 * @see Data, OmmDate, OmmTime, OmmDateTime.
 * 
 */
package com.refinitiv.ema.access;

public interface DateTimeStringFormat 
{
	public static class DateTimeStringFormatTypes
	{
		/**
		 *Indicates Date/Time/DateTime to string output in ISO8601's dateTime format:
	     * "YYYY-MM-DDThour:minute:second.nnnnnnnnn" (e.g., 2010-11-30T15:24:54.627529436). 
		 */
		public static final int STR_DATETIME_ISO8601 = 1;
	
		/**
		 *Indicates Date/Time/DateTime to string output in to string output in the format:
	     * "DD MON YYYY hour:minute:second:milli:micro:nano" (e.g., 30 NOV 2010 15:24:54:627:529:436). 
	     * This is the default format. 
		 */
		public static final int STR_DATETIME_RSSL = 2;
	}
	
    /**
     * Indicates format of the output string when OmmDate/OmmTime/OmmDateTime is represented as string. 
     * 
     * @param format of the output string when OmmDate/OmmTime/OmmDateTime is converted to string.
     * 
     * @throws OmmInvalidUsageException if format is invalid. 
     * 
     * @return value of format
     *         
     * @see DateTimeStringFormatTypes
     */
    public int format(int format); 

    /**
     *  Output Format type when OmmDate/OmmTime/OmmDateTime is converted to string.
     * 
     * @return format of the output string.
     * 
     * @see DateTimeStringFormatTypes
     */
    public int format(); 

	/**
	 * Returns the OmmDate value as a string in a specified format.
	 * 
	 * @param date that needs to be converted to string.
	 * 
	 * @return string representation of this object OmmDate in the specified format
	*/
	public String dateAsString(OmmDate date);

	/**
	 * Returns the OmmTime value as a string in a specified format.
	 * 
	 * @param time that needs to be converted to string.
	 * 
	 * @return string representation of this object OmmTime in the specified format
	*/
	public String timeAsString(OmmTime time);
	
	/**
	 * Returns the OmmTime value as a string in a specified format.
	 * 
	 * @param datetime that needs to be converted to string.
	 * 
	 * @return string representation of this object OmmTime in the specified format
	*/
	public String dateTimeAsString(OmmDateTime datetime);	
}
