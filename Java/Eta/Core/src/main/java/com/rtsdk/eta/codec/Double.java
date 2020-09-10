package com.rtsdk.eta.codec;

/**
* UPA 8-byte double value
*/
public interface Double
{
    /**
     * Sets members in Double to 0.
     */
    public void clear();

    /**
     * Sets Double to blank.
     */
    public void blank();
     
     /**
      * Is Double blank.
      *
      * @return true, if is blank
      */
    public boolean isBlank();

    /**
     * This method will perform a deep copy into the passed in parameter's 
     *          members from the Object calling this method.
     * 
     * @param destDouble the value getting populated with the values of the calling Object
     * 
     * @return {@link CodecReturnCodes#SUCCESS} on success,
     *         {@link CodecReturnCodes#INVALID_ARGUMENT} if the destDouble is null. 
     */
    public int copy(Double destDouble);
    
    /**
     * Return value as double.
     * 
     * @return the value as double
     */
    public double toDouble();
    
    /**
     * Convert to a {@link Real} .
     *
     * @param hint {@link RealHints} enumeration hint value to use for converting double.
     * Must be in the range of {@link RealHints#EXPONENT_14} - {@link RealHints#MAX_DIVISOR}.
     * @return the value as {@link Real}
     */
    public Real toReal(int hint);

    /**
     * Convert {@link Double} to a numeric string.
     * 
     * @return the string representation of this {@link Double}
     */
    public String toString();

    /**
     * Set value to double.
     * 
     * @param value the value to set
     */
    public void value(double value);
    
    /**
     * Check equality of two Double types.
     * 
     * @param thatDouble the other Double to compare to this one
     * 
     * @return true if equal, false otherwise
     */
    public boolean equals(Double thatDouble);

    /**
     * Set value to string.
     * 
     * @param value the value to set
     * 
     * @return {@link CodecReturnCodes#SUCCESS} on success,
     *         {@link CodecReturnCodes#INVALID_ARGUMENT} if value is invalid. 
     */
    public int value(String value);

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
     * Decode Double.
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
}
