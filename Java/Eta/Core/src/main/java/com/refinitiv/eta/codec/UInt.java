/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2019-2022 Refinitiv. All rights reserved.         --
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.eta.codec;

import java.math.BigInteger;

/**
 * ETA Unsigned Integer type. Can currently represent an unsigned value with precision of up to 64 bits.
 */
public interface UInt
{
    /**
     * Clears {@link UInt}.
     */
    public void clear();
    
    /**
     * This method will perform a deep copy into the passed in parameter's 
     *          members from the Object calling this method.
     * 
     * @param destUInt the value getting populated with the values of the calling Object
     *
     * @return {@link CodecReturnCodes#SUCCESS} on success,
     *         {@link CodecReturnCodes#INVALID_ARGUMENT} if the destUInt is null. 
    */
    public int copy(UInt destUInt);
    
    /**
     * Is UInt blank.
     *
     * @return true, if is blank
     */
    public boolean isBlank();

    /**
     * Returns value as long. The value is returned as signed long. If the value
     * is negative, the user has to convert it to an unsigned value.
     * Alternatively, the user could call {@link #toBigInteger}.
     * 
     * @return the value as long
     */
    public long toLong();

    /**
     * Returns value as unsigned BigInteger.
     * This method may introduce garbage collection, as it creates BigInteger.
     * To avoid it, user may instead call the toLong method and implement the conversion.
     * 
     * @return the value as BigInteger
     */
    public BigInteger toBigInteger();

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
     * Set value to BigInteger. Must be in the range of 0 - (2^64-1).
     * 
     * @param value the value to set
     * 
     * @return {@link CodecReturnCodes#SUCCESS} on success,
     *         {@link CodecReturnCodes#INVALID_ARGUMENT} if value is invalid. 
     */
    public int value(BigInteger value);

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
     * Decode {@link UInt}.
     * 
     * @param iter {@link DecodeIterator} with buffer to decode from and appropriate
     *            version information set.
     * 
     * @return {@link CodecReturnCodes#SUCCESS} if success, {@link CodecReturnCodes#INCOMPLETE_DATA} if
     *         failure, {@link CodecReturnCodes#BLANK_DATA} if data is blank value
     * 
     * @see DecodeIterator
     */
    public int decode(DecodeIterator iter);

    /**
     * Convert {@link UInt} to a numeric string.
     * 
     * @return the string representation of this {@link UInt}
     */
    public String toString();

    /**
     * Set value to uint.
     * 
     * @param value the value to set
     * 
     * @return {@link CodecReturnCodes#SUCCESS} on success,
     *         {@link CodecReturnCodes#INVALID_ARGUMENT} if value is invalid. 
     */
    public int value(String value);
    
    /**
     * Checks equality of two UInt data.
     * 
     * @param thatUInt the other UInt to compare to this one
     * 
     * @return true if UInts are equal, false otherwise
     */
    public boolean equals(UInt thatUInt);

}