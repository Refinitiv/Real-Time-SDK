/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2019-2022 LSEG. All rights reserved.     
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.eta.codec;

import com.refinitiv.eta.codec.Buffer;

/** 
 * A single defined enumerated value.
 */
public interface EnumType 
{
    /**
     * The actual value representing the type.
     * 
     * @return the value
     */
    public int value();

    /**
     * A brief string representation describing what the type means (For example,
     * this may be an abbreviation of a currency to be displayed to a user).
     *
     * Note: toString() method called on the display buffer
     * will return String object obtained from the underlying bytes using ISO-8859-1 encoding regardless of the
     * encoding set for JVM in order to avoid data corruption in case the bytes represent an RMTES string incompatible
     * with, e.g., UTF-8 encoding
     *
     * 
     * @return the display
     */
    public Buffer display();

    /**
     * A more elaborate description of what the value means. This information is
     * typically optional and not displayed.
     * 
     * @return the meaning
     */
    public Buffer meaning();
}