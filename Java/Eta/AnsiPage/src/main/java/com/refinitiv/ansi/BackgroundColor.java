/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2019-2022 Refinitiv. All rights reserved.         --
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.ansi;

public class BackgroundColor implements Cloneable
{
    private byte[] _value;

    private BackgroundColor(byte[] value)
    {
        _value = new byte[value.length];
        for (int i = 0; i < value.length; i++)
            _value[i] = value[i];
    }

    static byte[] color0 = { ';', '4', '0' };
    static byte[] color1 = { ';', '4', '1' };
    static byte[] color2 = { ';', '4', '2' };
    static byte[] color3 = { ';', '4', '3' };
    static byte[] color4 = { ';', '4', '4' };
    static byte[] color5 = { ';', '4', '5' };
    static byte[] color6 = { ';', '4', '6' };
    static byte[] color7 = { ';', '4', '7' };
    public final static BackgroundColor
    BColor0 = new BackgroundColor(color0),
    BColor1 = new BackgroundColor(color1),
    BColor2 = new BackgroundColor(color2),
    BColor3 = new BackgroundColor(color3),
    BColor4 = new BackgroundColor(color4),
    BColor5 = new BackgroundColor(color5),
    BColor6 = new BackgroundColor(color6),
    BColor7 = new BackgroundColor(color7);

    public final static BackgroundColor[] backgroundColor = {
        BColor0, BColor1, BColor2, BColor3, BColor4, BColor5, BColor6, BColor7 };

    public Object clone()
    {
        return new BackgroundColor(_value);
    }

    /**
     * To bytes.
     *
     * @return the byte[]
     */
    public byte[] toBytes()
    {
        return _value;
    }
  
    /**
     * Instantiates a new background color.
     */
    public BackgroundColor()
    {
    }
}
