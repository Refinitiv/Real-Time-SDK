/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2019-2022 Refinitiv. All rights reserved.         --
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.eta.codec;

/**
 * Represents bandwidth optimized date value containing month, day, and year information.
 */
public interface Date
{
    /**
     * Sets all members in Date to 0. Because 0 represents a blank date value,
     * this performs the same functionality as #{@link Date#blank()}.
     */
    public void clear();

    /**
     *  This method will perform a deep copy of this Object to destDate.
     *          
     *  @param destDate the destination Data Object.
     *           
     *  @return {@link CodecReturnCodes#SUCCESS} on success,
     *         {@link CodecReturnCodes#INVALID_ARGUMENT} if the destDate is null. 
     */
    public int copy(Date destDate);

    /**
     * Sets all members in Date to 0. Because 0 represents a blank date value,
     * this performs the same functionality as #{@link Date#clear()}.
     */
    public void blank();

    /**
     * Returns true if all members in {@link Date} are set to the values used to signify
     * blank. A blank {@link Date} contains day, month and year values of 0. 
     * 
     * @return true if {@link Date} is blank, false otherwise
     */
    public boolean isBlank();

    /**
     * Verifies the contents of a populated {@link Date} object. Determines
     * whether the specified day is valid within the specified month (e.g., a
     * day greater than 31 is considered invalid for any month). This method
     * uses the year member to determine leap year validity of day numbers for February.
     * 
     * @return true if {@link Date} is blank or valid, false otherwise
     */
    public boolean isValid();

    /**
     * Checks equality of two Date structures.
     * 
     * @param thatDate the other date to compare to this one
     * 
     * @return true if dates are equal, false otherwise
     */
    public boolean equals(Date thatDate);

    /**
     * Sets the format of the output string when date is represented as string. 
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
     *  Format of Date when converted to string.
     * 
     * @return format of the output string when toString() is used.
     * 
     *  @see DateTimeStringFormatTypes
     */
    public int format(); 
    
    /**
     * Converts Date to a String based on the format.
     * 
     * 
     * @return the string representation of this Date
     * 
     *  @see DateTimeStringFormatTypes
     */
    public String toString();
    
  /**
     * Converts string date from "DD MMM YYYY" (01 JUN 2003) or "MM/DD/YYYY" (6/1/2003) or ISO8601's "YYYY-MM-DD" format to Date.
     * 
     * @param value string containing an appropriately formatted string to
     *            convert from
     *
     * @return {@link CodecReturnCodes#SUCCESS} on success,
     *         {@link CodecReturnCodes#INVALID_ARGUMENT} if value is invalid. 
     */
    public int value(String value);

    /**
     * Used to encode a date into a buffer.
     * 
     * @param iter {@link EncodeIterator} with buffer to encode into. Iterator
     *            should also have appropriate version information set.
     * 
     * @return {@link CodecReturnCodes#SUCCESS} if success,
     *         {@link CodecReturnCodes#FAILURE} if failure,
     *         {@link CodecReturnCodes#BUFFER_TOO_SMALL} if the encode buffer is
     *         too small
     * 
     * @see EncodeIterator
     * @see CodecReturnCodes
     */
    public int encode(EncodeIterator iter);

    /**
     * Decode date.
     * 
     * @param iter {@link DecodeIterator} with buffer to decode from and
     *            appropriate version information set.
     * 
     * @return {@link CodecReturnCodes#SUCCESS} if success,
     *         {@link CodecReturnCodes#INCOMPLETE_DATA} if failure,
     *         {@link CodecReturnCodes#BLANK_DATA} if data is blank value.
     * 
     * @see DecodeIterator
     * @see CodecReturnCodes
     */
    public int decode(DecodeIterator iter);

    /**
     * Represents the day of the month, where 0 indicates a blank entry. day
     * allows for a range of 0 to 255, though the value typically does not exceed 31.
     * 
     * @param day the day to set
     * 
     * @return {@link CodecReturnCodes#SUCCESS} on success,
     *         {@link CodecReturnCodes#INVALID_ARGUMENT} if day is invalid. 
     */
    public int day(int day);

    /**
     * Represents the day of the month, where 0 indicates a blank entry. day
     * allows for a range of 0 to 255, though the value typically does not exceed 31.
     * 
     * @return the day
     */
    public int day();

    /**
     * Represents the month of the year, where 0 indicates a blank entry. month
     * allows for a range of 0 to 255, though the value typically does not exceed 12.
     * 
     * @param month the month to set
     * 
     * @return {@link CodecReturnCodes#SUCCESS} on success,
     *         {@link CodecReturnCodes#INVALID_ARGUMENT} if month is invalid. 
     */
    public int month(int month);

    /**
     * Represents the month of the year, where 0 indicates a blank entry. month
     * allows for a range of 0 to 255, though the value typically does not exceed 12.
     * 
     * @return the month
     */
    public int month();

    /**
     * Represents the year, where 0 indicates a blank entry. You can use this
     * member to specify a two or four digit year (where specific usage is
     * indicated outside of ETA). year allows for a range of 0 to 65,535.
     * 
     * @param year the year to set
     * 
     * @return {@link CodecReturnCodes#SUCCESS} on success,
     *         {@link CodecReturnCodes#INVALID_ARGUMENT} if year is invalid. 
     */
    public int year(int year);

    /**
     * Represents the year, where 0 indicates a blank entry. You can use this
     * member to specify a two or four digit year (where specific usage is
     * indicated outside of ETA). year allows for a range of 0 to 65,535.
     * 
     * @return the year
     */
    public int year();
}