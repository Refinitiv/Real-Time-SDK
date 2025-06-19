/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2020,2022,2024 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.eta.codec;

/**
 * ETA 4-byte floating point value
 */
public interface Float
{
    /**
     * Sets members in Float to 0.
     */
    public void clear();
 
    /**
     * Sets Float to blank.
     */
    public void blank();
 
    /**
     * Is Float blank.
     *
     * @return true, if is blank
     */
    public boolean isBlank();

    /**
     * This method will perform a deep copy into the passed in parameter's 
     *          members from the Object calling this method.
     * 
     * @param destFloat the value getting populated with the values of the calling Object
     * 
     * @return {@link CodecReturnCodes#SUCCESS} on success,
     *         {@link CodecReturnCodes#INVALID_ARGUMENT} if the destFloat is null. 
     */
    public int copy(Float destFloat);

    /**
     * Check equality of two Float types.
     * 
     * @param thatFloat the other Float to compare to this one
     * 
     * @return true if equal, false otherwise
     */
    public boolean equals(Float thatFloat);

    /**
     * Return value as float.
     * 
     * @return the value as float
     */
    public float toFloat();
    
    /**
     * Convert to a {@link Real}. Must be in the range of {@link RealHints#EXPONENT_14}
     * - {@link RealHints#MAX_DIVISOR}.
     * 
     * @param hint {@link RealHints} enumeration hint value to use for converting float
     * 
     * @return the value as {@link Real} 
     */
    public Real toReal(int hint);

    /**
     * Convert {@link Float} to a numeric string.
     * 
     * @return the string representation of this {@link Float}
     */
    public String toString();

    /**
     * Set value to float.
     * 
     * @param value the value to set
     */
    public void value(float value);

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
     * Decode Float.
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
