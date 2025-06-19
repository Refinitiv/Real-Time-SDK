/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2020,2022,2024 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.eta.codec;

/**
 * Represents the date and time (month, day, year, hour, minute, second, 
 * millisecond, microsecond, and nanosecond) in a bandwidth-optimized fashion. 
 * This time value is represented as Greenwich Mean Time (GMT) unless noted otherwise
 */
public interface DateTime
{
    /**
     * Sets all members in DateTime to 0.
     */
    public void clear();

    /**
     * Sets {@link DateTime} to blank. Sets all members in {@link DateTime} to
     * their respective blank values.
     * <p>
     * For date, all values are set to 0.
     * For time, hour, minute, and second are set to 255 millisecond is set
     * to 65535, microsecond and nanosecond are set to 2047.
     */
    public void blank();

    /**
     * Returns true if all members in {@link DateTime} are set to the values used to signify
     * blank. A blank {@link DateTime} contains hour, minute, and second values of 255, 
     * a millisecond value of 65535, microsecond and nanosecond values of 2047,  and 
     * day, month and year value of 0.
     * 
     * @return true if {@link DateTime} is blank, false otherwise.
     */
    public boolean isBlank();

    /**
     * Verifies the contents of a populated {@link DateTime} object. Determines
     * whether day is valid for the specified month (e.g., a day greater than 31
     * is considered invalid for any month) as determined by the specified year
     * (to calculate whether it is a leap year). Also validates the range of
     * hour, minute, second, millisecond, microsecond, and nanosecond members. 
     * If {@link DateTime} is blank or valid, true is returned; false otherwise.
     * 
     * @return true if {@link DateTime} is blank or valid, false otherwise.
     */
    public boolean isValid();

    /**
     * Checks equality of two {@link DateTime} objects.
     * 
     * @param thatDateTime the other date and time to compare to this one
     * 
     * @return true if dates and times are equal, false otherwise
     */
    public boolean equals(DateTime thatDateTime);

    /**
     * Sets the format of the output string when DateTime is represented as string. 
     * 
     * @param format of the output string when toString() is used.
     * 
     * @return {@link CodecReturnCodes#SUCCESS} on success,
     *         {@link CodecReturnCodes#INVALID_ARGUMENT} if format is invalid. 
     *         
     * @see DateTimeStringFormatTypes
     */
    public int format(int format); 

    /**
     *  Format of DateTime when converted to string.
     * 
     * @return format of the output string when toString() is used.
     * 
     *  @see DateTimeStringFormatTypes
     */
    public int format();     
    
   /**
     * Convert DateTime to a String. Returns a String  based on the format. 
     * as "%d %b %Y hour:minute:second:milli:micro:nano" (e.g., 30 NOV 2010 15:24:54:627:529:436) or 
     * IS8601 "YYYY-MM-DDTHH:MIN:SEC.nnnnnnnnn" (e.g., 2010-11-30T15:24:54.627529439).
     * 
     * @return the string representation of this {@link DateTime}
     * 
     *  @see DateTimeStringFormatTypes
     */
    public String toString();

    /**
     * Converts a String representation of a date and time to a DateTime. This method
     * supports Date values following "%d %b %Y" format (e.g., 30 NOV 2010) or "%m/%d/%y"
     * format (e.g., 11/30/2010) or ISO8601 format. This method supports Time values that conform to
     * "%H:%M" (e.g., 15:24) or "%H:%M:%S" (e.g., 15:24:54) or "hour:minute:second:milli:micro:nano" (e.g., 15:24:54:627:529:436)
     * formats as well as ISO8601's dateTime format.
     * 
     * @param value string containing an appropriately formatted string to convert from
     * 
     * @return {@link CodecReturnCodes#SUCCESS} on success,
     *         {@link CodecReturnCodes#INVALID_ARGUMENT} if value is invalid. 
     */
    public int value(String value);

    /**
     * Sets date time from a number equal to milliseconds since the January 1, 1970 (midnight UTC/GMT) epoch.
     * Must be a positive number.
     * 
     * @param value number equal to milliseconds since epoch
     * 
     * @return {@link CodecReturnCodes#SUCCESS} on success,
     *         {@link CodecReturnCodes#INVALID_ARGUMENT} if value is invalid. 
     */
    public int value(long value);

    /**
     * Used to encode a date and time into a buffer.
     * 
     * @param iter {@link EncodeIterator} with buffer to encode into. Iterator
     *            should also have appropriate version information set.
     * 
     * @return {@link CodecReturnCodes#SUCCESS} if success,
     *         {@link CodecReturnCodes#FAILURE} if failure,
     *         {@link CodecReturnCodes#BUFFER_TOO_SMALL} if the encode buffer is too small
     * 
     * @see EncodeIterator
     * @see CodecReturnCodes
     */
    public int encode(EncodeIterator iter);

    /**
     * Decode date and time.
     * 
     * @param iter DecodeIterator with buffer to decode from and appropriate
     *            version information set.
     * 
     * @return {@link CodecReturnCodes#SUCCESS} if success,
     *         {@link CodecReturnCodes#INCOMPLETE_DATA} if failure,
     *         {@link CodecReturnCodes#BLANK_DATA} if data is blank value
     * 
     * @see DecodeIterator
     * @see CodecReturnCodes
     */
    public int decode(DecodeIterator iter);

    /**
     * {@link Date} portion of the {@link DateTime}.
     * 
     * @return the date
     */
    public Date date();

    /**
     * Day of the month (0 - 31 where 0 indicates blank).
     * 
     * @param day the day to set
     * 
     * @return {@link CodecReturnCodes#SUCCESS} on success,
     *         {@link CodecReturnCodes#INVALID_ARGUMENT} if day is invalid. 
     */
    public int day(int day);

    /**
     * Day of the month (0 - 31 where 0 indicates blank).
     * 
     * @return the day
     */
    public int day();

    /**
     * Month of the year (0 - 12 where 0 indicates blank).
     * 
     * @param month the month to set
     * 
     * @return {@link CodecReturnCodes#SUCCESS} on success,
     *         {@link CodecReturnCodes#INVALID_ARGUMENT} if month is invalid. 
     */
    public int month(int month);

    /**
     * Month of the year (0 - 12 where 0 indicates blank).
     * 
     * @return the month
     */
    public int month();

    /**
     * Year (0 - 4095 where 0 indicates blank).
     * 
     * @param year the year to set
     * 
     * @return {@link CodecReturnCodes#SUCCESS} on success,
     *         {@link CodecReturnCodes#INVALID_ARGUMENT} if year is invalid. 
     */
    public int year(int year);

    /**
     * Year (0 - 4095 where 0 indicates blank).
     * 
     * @return the year
     */
    public int year();

    /**
     * {@link Time} portion of the {@link DateTime}.
     * 
     * @return the time
     */
    public Time time();

    /**
     * The hour of the day (0 - 23 where 255 indicates blank).
     * 
     * @param hour the hour to set
     * 
     * @return {@link CodecReturnCodes#SUCCESS} on success,
     *         {@link CodecReturnCodes#INVALID_ARGUMENT} if hour is invalid. 
     */
    public int hour(int hour);

    /**
     * The hour of the day (0 - 23 where 255 indicates blank).
     * 
     * @return the hour
     */
    public int hour();

    /**
     * The minute of the hour (0 - 59 where 255 indicates blank).
     * 
     * @param minute the minute to set
     * 
     * @return {@link CodecReturnCodes#SUCCESS} on success,
     *         {@link CodecReturnCodes#INVALID_ARGUMENT} if minute is invalid. 
     */
    public int minute(int minute);

    /**
     * The minute of the hour (0 - 59 where 255 indicates blank).
     * 
     * @return the minute
     */
    public int minute();

    /**
     * The second of the minute (0 - 59 where 255 indicates blank).
     * 
     * @param second the second to set
     * 
     * @return {@link CodecReturnCodes#SUCCESS} on success,
     *         {@link CodecReturnCodes#INVALID_ARGUMENT} if second is invalid. 
     */
    public int second(int second);

    /**
     * The second of the minute (0 - 60 where 255 indicates blank and 60 is to account for leap second).
     * 
     * @return the second
     */
    public int second();

    /**
     * The millisecond of the second (0 - 999 where 65535 indicates blank).
     * 
     * @param millisecond the millisecond to set
     * 
     * @return {@link CodecReturnCodes#SUCCESS} on success,
     *         {@link CodecReturnCodes#INVALID_ARGUMENT} if millisecond is invalid. 
     */
    public int millisecond(int millisecond);

    /**
     * The millisecond of the second (0 - 999 where 65535 indicates blank).
     * 
     * @return the millisecond
     */
    public int millisecond();
    
    /**
     * The microsecond of the millisecond (0 - 999 where 2047 indicates blank).
     * 
     * @param microsecond the microsecond to set
     * 
     * @return {@link CodecReturnCodes#SUCCESS} on success,
     *         {@link CodecReturnCodes#INVALID_ARGUMENT} if microsecond is invalid. 
     */
    public int microsecond(int microsecond);

    /**
     * The microsecond of the millisecond (0 - 999 where 2047 indicates blank).
     * 
     * @return the microsecond
     */
    public int microsecond();

    /**
     * The nanosecond of the microsecond (0 - 999 where 2047 indicates blank).
     * 
     * @param nanosecond the nanosecond to set
     * 
     * @return {@link CodecReturnCodes#SUCCESS} on success,
     *         {@link CodecReturnCodes#INVALID_ARGUMENT} if nanosecond is invalid. 
     */
    public int nanosecond(int nanosecond);

    /**
     * The nanosecond of the microsecond (0 - 999 where 2047 indicates blank).
     * 
     * @return the nanosecond
     */
    public int nanosecond();    
    
    /**
     * Set the date time to now in the local time zone.
     */
    public void localTime();


    /**
     *  This method will perform a deep copy of this Object to destDateTime.
     *          
     *  @param destDateTime the destination DataTime Object.
     *           
     *  @return {@link CodecReturnCodes#SUCCESS} on success,
     *         {@link CodecReturnCodes#INVALID_ARGUMENT} if the destDateTime is null. 
     */
    public int copy(DateTime destDateTime);
    
    /**
     * Set the date time to now in the GMT zone.
     */
    public void gmtTime();
    
    /**
     * Returns this date time value as milliseconds since the January 1, 1970 (midnight UTC/GMT) epoch.
     * 
     * @return milliseconds since epoch
     */
    public long millisSinceEpoch();
}
