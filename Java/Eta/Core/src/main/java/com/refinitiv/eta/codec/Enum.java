/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2019-2022 Refinitiv. All rights reserved.         --
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.eta.codec;

/**
 * Represents an enumeration type, defined as an unsigned two-byte value. Many
 * times, this enumeration value is cross-referenced with an enumeration
 * dictionary (e.g., enumtype.def) or a well-known enumeration definition (e.g.,
 * those contained in RDM package).
 * 
 * This type allows a value ranging from 0 to 65,535.
 */
public interface Enum
{
    /**
     * Clears {@link Enum}.
     */
    public void clear();
    
    /**
     * Is Enum blank.
     *
     * @return true, if is blank
     */
    public boolean isBlank();

    /**
     * This method will perform a deep copy into the passed in parameter's 
     *          members from the Object calling this method.
     * 
     * @param destEnum the value getting populated with the values of the calling Object
     * 
     * @return {@link CodecReturnCodes#SUCCESS} on success,
     *         {@link CodecReturnCodes#INVALID_ARGUMENT} if the destEnum is null. 
     */
    public int copy(Enum destEnum);

    /**
     * Return value as int.
     * 
     * @return the value as int
     */
    public int toInt();

    /**
     * The enum value.
     * 
     * @param value the value to set
     * 
     * @return {@link CodecReturnCodes#SUCCESS} on success,
     *         {@link CodecReturnCodes#INVALID_ARGUMENT} if value is invalid. 
     */
    public int value(int value);

    /**
     * Decode enum.
     * 
     * @param iter {@link DecodeIterator} with buffer to decode from and
     *            appropriate version information set.
     * 
     * @return {@link CodecReturnCodes#SUCCESS} if success,
     *         {@link CodecReturnCodes#INCOMPLETE_DATA} if failure,
     *         {@link CodecReturnCodes#BLANK_DATA} if data is blank value.
     * 
     * @see DecodeIterator
     */
    public int decode(DecodeIterator iter);

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
     * Convert {@link Enum} to a numeric string.
     * 
     * @return the string representation of this {@link Enum}
     */
    public String toString();

    /**
     * Set value to {@link Enum}.
     * 
     * @param value the value to set
     * 
     * @return {@link CodecReturnCodes#SUCCESS} on success,
     *         {@link CodecReturnCodes#INVALID_ARGUMENT} if value is invalid. 
     */
    public int value(String value);
    
    /**
     * Checks equality of two Enum data.
     * 
     * @param thatEnum the other enum to compare to this one
     * 
     * @return true if enums are equal, false otherwise
     */
    public boolean equals(Enum thatEnum);
        
}