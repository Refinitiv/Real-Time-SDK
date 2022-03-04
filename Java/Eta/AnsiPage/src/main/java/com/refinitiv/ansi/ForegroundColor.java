/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2019-2022 Refinitiv. All rights reserved.         --
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.ansi;

public final class ForegroundColor
{
    private byte[] _value;

    private ForegroundColor(byte[] value)
    {
        _value = new byte[value.length];
        for (int i = 0; i < value.length; i++)
            _value[i] = value[i];
    }
  
    static byte[] color0 = { ';', '3', '0' };
    static byte[] color1 = { ';', '3', '1' };
    static byte[] color2 = { ';', '3', '2' };
    static byte[] color3 = { ';', '3', '3' };
    static byte[] color4 = { ';', '3', '4' };
    static byte[] color5 = { ';', '3', '5' };
    static byte[] color6 = { ';', '3', '6' };
    static byte[] color7 = { ';', '3', '7' };
    public final static ForegroundColor
    FColor0 = new ForegroundColor(color0),
    FColor1 = new ForegroundColor(color1),
    FColor2 = new ForegroundColor(color2),
    FColor3 = new ForegroundColor(color3),
    FColor4 = new ForegroundColor(color4),
    FColor5 = new ForegroundColor(color5),
    FColor6 = new ForegroundColor(color6),
    FColor7 = new ForegroundColor(color7);

    public final static ForegroundColor[] foregroundColor = {
      FColor0, FColor1, FColor2, FColor3, FColor4, FColor5, FColor6, FColor7};

    /**
     * To bytes.
     *
     * @return the byte[]
     */
    public byte[] toBytes()
    {
        return _value;
    }
}
