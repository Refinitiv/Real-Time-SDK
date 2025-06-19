/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2020,2022,2024 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.ansi;

public final class StatusType implements Cloneable
{
    short   row;
    short   col;
    char    cur_attr;       /* monochrome attribute */
    char    c_attr;         /* color attribute */
    char    c_fade_attr;    /* color fading attr */
    char    fading;         /* mono fading attr */
    char    gr_set;         /* flag to select G0 or G1 */
    char    G0_set;         /* G0 graphics set */
    char    G1_set;         /* G1 graphics set */
    boolean wrap_on;        /* flag for line wrap */

    boolean	vem;            /* flag for vert editing mode */
    boolean	hem;            /* flag for horiz edit mode */
    boolean _fade;

    final static int DB_BITS = 0xE0;
    final static int DB_SHT_BITES = 0x07;
    final static int DB_SHIFT = 5;
    final static int DB_ATT_MASK = 0x1f;

    final static int BACK_COL_MASK = 0xf0;
    final static int FORG_COL_MASK = 0xf0;

    public Object clone()
    {
        StatusType status = new StatusType();
        status.row = row;
        status.col = col;
        status.cur_attr = cur_attr; /* monochrome attribute */
        status.c_attr = c_attr;     /* color attribute */
        status.c_fade_attr = c_fade_attr; /* color fading attr */
        status.fading = fading;     /* mono fading attr */
        status.gr_set = gr_set;     /* flag to select G0 or G1 */
        status.G0_set = G0_set;     /* G0 graphics set */
        status.G1_set = G1_set;     /* G1 graphics set */
        status.wrap_on = wrap_on;   /* flag for line wrap */
        status.vem = vem;   /* flag for vert editing mode */
        status.hem = hem;   /* flag for horiz edit mode */
        status._fade = _fade;
        return status;
    }

    /**
     * Inits the status.
     */
    public void initStatus()
    {
        row = 1;
        col = 1;
        cur_attr = CharType.PLAIN;      /* monochrome attribute */
        c_attr = CharType.MONO;         /* color attribute */
        c_fade_attr = CharType.MONO;    /* color fading attr */
        fading = CharType.PLAIN;        /* mono fading attr */
        gr_set = 0;                     /* flag to select G0 or G1 */
        G0_set = CharType.USAscii;      /* G0 graphics set */
        G1_set = CharType.USAscii;      /* G1 graphics set */
        wrap_on = false;                /* flag for line wrap */
        _fade = false;                  /* flag for fading */
    }

    boolean isattr(char b)
    {
        return ((_fade) ? fading & b : cur_attr & b) == 0;
    }

    void offattr(char c)
    {
        if (_fade)
            fading &= c;
        else
            cur_attr &= c;
    }

    void onattr(char arg)
    {
        if (_fade)
            fading |= arg;
        else
            cur_attr |= arg;
    }

    void setattr(char b)
    {
        if (_fade)
            fading &= b;
        else
            cur_attr &= b;
    }

    void setcolor(char b)
    {
        if (_fade)
            c_fade_attr = b;
        else
            c_attr = b;
    }
	
    void setFade(boolean f)
    {
        _fade = f;
    }

    void setplain()
    {
        if (_fade)
            fading &= CharType.PLAIN | DB_BITS;
        else
            cur_attr &= CharType.PLAIN | DB_BITS;
    }
}

