/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2020,2022,2024 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.ansi;

import java.awt.Color;

public final class CharType implements Cloneable
{
    public static final char PLAIN = 0;
    public static final char BLINK = 1;
    public static final char REVVID = 2;
    public static final char DIM = 4;
    public static final char UNDLN = 8;
    public static final char BRIGHT = 16;
    public static final int BACK_COL_MASK = 0xf0;
    public static final int FORG_COL_MASK = 0x0f;

    public static final char BLACK = 0;
    public static final char RED = 1;
    public static final char GREEN = 2;
    public static final char YELLOW = 3;
    public static final char BLUE = 4;
    public static final char MAGENTA = 5;
    public static final char CYAN = 6;
    public static final char WHITE = 7;
    public static final char MONOCOLOR = 15;
    public static final char MONO = 255;

    public static final char USAscii = 'B';
    public static final char UKAscii = 'A';
    public static final char VT100Graphics = '0';
    public static final char chapdelaineAscii = ':';
    public static final char RMJAscii = ';';
    public static final char garbanAscii = '<';
    public static final char garbanGraphics = 'b';
    public static final char mabonAscii = '=';
    public static final char mosaicGraphics = '>';
    public static final char FBIAscii = '?';
    public static final char FBIGraphics = 'f';
    public static final char telerateAscii = 's';
    public static final char topic = 't';
    public static final char generalGraphics = 'g';
    public static final char viewdataMosaic = 'v';
    public static final char viewdataSeparatedMosaic = 'w';

    public char     ch;
    public char     gs;             /* graphic set */
    public char     attr;           /* mono attrib */
    public char     fade_attr;      /* mono fading */
    public char     c_attr;         /* color attrib */
    public char     c_fade_attr;    /* color fading */

    /**
     * Copy from.
     *
     * @param from the from
     */
    public void copyFrom(CharType from)
    {
        ch = from.ch;
        gs = from.gs;
        attr = from.attr;
        fade_attr = from.fade_attr;
        c_attr = from.c_attr;
        c_fade_attr = from.c_fade_attr;
    }

    public Object clone()
    {
        CharType charType = new CharType();
        charType.ch = ch;
        charType.gs = gs;               /* graphic set */
        charType.attr = attr;           /* mono attrib */
        charType.fade_attr = fade_attr; /* mono fading */
        charType.c_attr = c_attr;           /* color attrib */
        charType.c_fade_attr = c_fade_attr; /* color fading */
        return charType;
    }

    /**
     * Instantiates a new char type.
     */
    public CharType()
    {
        clear();
    }

    /**
     * Clear.
     */
    public void clear()
    {
        ch = ' ';
        gs = USAscii;
        attr = PLAIN;
        fade_attr = PLAIN;
        c_attr = MONO;
        c_fade_attr = MONO;
    }
  
    /**
     * Color for.
     *
     * @param i the i
     * @return the color
     */
    public static Color colorFor(int i)
    {
        Color c = null;
        switch (i)
        {
            case BLACK:
                c = Color.black;
                break;
            case RED:
                c = Color.red;
                break;
            case GREEN:
                c = Color.green;
                break;
            case YELLOW:
                c = Color.yellow;
                break;
            case BLUE:
                c = Color.blue;
                break;
            case MAGENTA:
                c = Color.magenta;
                break;
            case CYAN:
                c = Color.cyan;
                break;
            case WHITE:
            default:
                c = Color.white;
                break;
        }
        return c;
    }
}
