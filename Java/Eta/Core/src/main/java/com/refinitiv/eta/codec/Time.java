package com.refinitiv.eta.codec;

/**
 * The ETA Time type allows for bandwidth optimized representation of a time
 * value containing hour, minute, second, millisecond, microsecond, and nanosecond information.
 * 
 * {@link Time} is always represented as GMT unless otherwise specified.
 */
public interface Time
{
    /**
     * Clears {@link Time}.
     */
    public void clear();

    /**
     *  This method will perform a deep copy of this Object to destTime.
     *          
     *  @param destTime the destination Time Object.
     *           
     *  @return {@link CodecReturnCodes#SUCCESS} on success,
     *         {@link CodecReturnCodes#INVALID_ARGUMENT} if the destTime is null. 
     * 
     */
    public int copy(Time destTime);

    /**
     * Sets all members in {@link Time} to the values used to signify blank. A blank
     * {@link Time} contains hour, minute, and second values of 255 and a millisecond value of 65535.
     */
    public void blank();

    /**
     * Returns true if all members in {@link Time} are set to the values used to signify
     * blank. A blank {@link Time} contains hour, minute, and second values of 255 
     * and a millisecond value of 65535.
     * 
     * @return true if blank, false otherwise
     */
    public boolean isBlank();

    /**
     * Verifies contents of populated {@link Time} structure. Validates the ranges
     * of the hour, minute, second, and millisecond members.
     * If {@link Time} is blank or valid, true is returned; false otherwise.
     * 
     * @return true if valid, false otherwise
     */
    public boolean isValid();

    /**
     * Checks equality of two {@link Time} objects.
     * 
     * @param thatTime the other time to compare to this one
     * 
     * @return true if times are equal, false otherwise
     */
    public boolean equals(Time thatTime);

    /**
     * Sets the format of the output string when Time is represented as string. 
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
     *  Format of Time when converted to string.
     * 
     * @return format of the output string when toString() is used.
     * 
     *  @see DateTimeStringFormatTypes
     */
    public int format(); 

    /**
     * Convert {@link Time} to a string based on format. Returns the string representation of this Time.
     * 
     * @return the string representation of this {@link Time}
     * 
     *  @see DateTimeStringFormatTypes
     */
    public String toString();

    /**
     * Converts string time from "HH:MM" (13:01) or "HH:MM:SS" (15:23:54) or ISO8601 format to {@link Time}.
     * 
     * @param value string containing an appropriately formatted string to convert from
     * 
     * @return {@link CodecReturnCodes#SUCCESS} on success,
     *         {@link CodecReturnCodes#INVALID_ARGUMENT} if value is invalid. 
     */
    public int value(String value);

    /**
     * Used to encode a time into a buffer.
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
     * Decode time.
     * 
     * @param iter {@link DecodeIterator} with buffer to decode from and
     *            appropriate version information set.
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
}
