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