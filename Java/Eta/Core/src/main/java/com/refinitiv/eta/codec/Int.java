package com.refinitiv.eta.codec;

/**
 * A signed integer that currently represents a value of up to 63 bits along
 * with a one bit sign (positive or negative).
 */
public interface Int
{
    /**
     * Sets members in Int to 0.
     */
    public void clear();
    
    /**
     * Is Int blank.
     *
     * @return true, if is blank
     */
    public boolean isBlank();

    /**
     * This method will perform a deep copy into the passed in parameter's 
     *          members from the Object calling this method.
     * 
     * @param destInt the value getting populated with the values of the calling Object
     *  
     * @return {@link CodecReturnCodes#SUCCESS} on success,
     *         {@link CodecReturnCodes#INVALID_ARGUMENT} if the destInt is null. 
     */
    public int copy(Int destInt);
    
    /**
     * Return value as long.
     * 
     * @return the value as long
     */
    public long toLong();

    /**
     * Set value to int.
     * 
     * @param value the value to set
     */
    public void value(int value);

    /**
     * Set value to long.
     * 
     * @param value the value to set
     */
    public void value(long value);

    /**
     * Used to encode into a buffer.
     * 
     * @param iter {@link EncodeIterator} with buffer to encode into. Iterator
     *            should also have appropriate version information set.
     * 
     * @return {@link CodecReturnCodes}
     * 
     * @see EncodeIterator
     */
    public int encode(EncodeIterator iter);

    /**
     * Decode Int.
     * 
     * @param iter {@link DecodeIterator} with buffer to decode from and
     *            appropriate version information set.
     * 
     * @return {@link CodecReturnCodes}, SUCCESS if success, INCOMPLETE_DATA if
     *         failure, BLANK_DATA if data is blank value
     * 
     * @see DecodeIterator
     */
    public int decode(DecodeIterator iter);

    /**
     * Convert {@link Int} to a numeric string.
     * 
     * @return the string representation of this {@link Int}
     */
    public String toString();

    /**
     * Set value to int.
     * 
     * @param value the value to set
     * 
     * @return {@link CodecReturnCodes#SUCCESS} on success,
     *         {@link CodecReturnCodes#INVALID_ARGUMENT} if value is invalid. 
     */
    public int value(String value);
    
    /**
     * Checks equality of two Int data.
     * 
     * @param thatInt the other Int to compare to this one
     * 
     * @return true if Ints are equal, false otherwise
     */
    public boolean equals(Int thatInt);
        
}