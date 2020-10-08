///*|-----------------------------------------------------------------------------
// *|            This source code is provided under the Apache 2.0 license      --
// *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
// *|                See the project's LICENSE.md for details.                  --
// *|           Copyright (C) 2019 Refinitiv. All rights reserved.            --
///*|-----------------------------------------------------------------------------

package com.refinitiv.ema.rdm;

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
     * @return the display
     */
    public String display();

    /**
     * A more elaborate description of what the value means. This information is
     * typically optional and not displayed.
     * 
     * @return the meaning
     */
    public String meaning();
    
	/**
     * Convert information contained in the enumerated type to a string.
     * 
     * @return the string representation of this {@link EnumType}
     */
     public String toString();
}