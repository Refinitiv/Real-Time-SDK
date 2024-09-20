/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2019-2022 LSEG. All rights reserved.     
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.eta.codec;

/**
 * Represents the Date, Time and DateTime to string conversion format types.
 * 
 * @see Date
 * @see DateTime
 * @see Time
 */
public class DateTimeStringFormatTypes
{
    /**
     * This class is not instantiated
     */
    private DateTimeStringFormatTypes()
    {
        throw new AssertionError();
    }

    /** (0x00) Date/Time/DateTime to string output in ISO8601's dateTime format:
     * "YYYY-MM-DDThour:minute:second.nnnnnnnnn" (e.g., 2010-11-30T:24:54.627529436). 
     */
    public static final int STR_DATETIME_ISO8601 = 0x01;

    /** (0x01) Date/Time/DateTime to string output in the format:
     * "DD MON YYYY hour:minute:second:milli:micro:nano" (e.g., 30 NOV 2010 15:24:54:627:529:436). 
     * This is the default format.
     */
    public static final int STR_DATETIME_RSSL = 0x02;
}
