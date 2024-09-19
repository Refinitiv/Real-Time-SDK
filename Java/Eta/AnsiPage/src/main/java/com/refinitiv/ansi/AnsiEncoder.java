/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2019-2022 LSEG. All rights reserved.     
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.ansi;

import java.io.*;

/**
 * The Class AnsiEncoder.
 */
public class AnsiEncoder implements Cloneable
{
    final static byte[] ESCCURSOROFF  = { '\033', '[', '?', '2', '5', 'l' };
    final static byte[] ESCSCREENWRAP = { '\033', '[', '?', '7', 'h' };
    final static byte[] ESCWRAPOFF    = { '\033', '[', '?', '7', 'l' };
    final static byte[] ESCFADEBLINK  = { '\033', '[', '>', '5', 'm' };
    final static byte[] ESCFADEOFF    = { '\033', '[', '>', '0', 'm' };
    final static byte[] ESCRESETSCR   = { '\033', 'c' };
    final static byte[] ESCPLAIN      = { '\033', '[', 'm' };
    final static byte[] ESCCLRLINE    = { '\033', '[', '2', 'K' };
	
    /* the following are used to build up an attribute change escape sequence */
    final static byte[] CSI_FADE      = { '\033', '[', '>' };
    final static byte[] CSI           = { '\033', '[' };
    final static byte[] ENDSET        = { 'm' };
    final static byte[] ESCMVCURS     = { '\033', '[', '%', 'd', ';', '%', 'd', 'H' };
    final static byte[] ESC_G0_SET    = { '\033', '(' };
    final static byte[] ESC_G1_SET    = { '\033', ')' };
    final static byte   USE_G0        = '\017';
    final static byte   USE_G1        = '\016';
    final static int MAX_SINGLE_ANSI_SEQUENCE = 30;
    final static short ERROR = -1;
    final static int DONE = 0;
    final static int NOT_DONE = 1;
	
    /* define tens and ones arrays to speed building of escape sequence for moving the cursor */
    final static byte tens[] = { '0',
		'0','0','0','0','0','0','0','0','0','1',
		'1','1','1','1','1','1','1','1','1','2',
		'2','2','2','2','2','2','2','2','2','3',
		'3','3','3','3','3','3','3','3','3','4',
		'4','4','4','4','4','4','4','4','4','5',
		'5','5','5','5','5','5','5','5','5','6',
		'6','6','6','6','6','6','6','6','6','7',
		'7','7','7','7','7','7','7','7','7','8'
    };

    final static byte ones[] = { '0',
		'1','2','3','4','5','6','7','8','9','0',
		'1','2','3','4','5','6','7','8','9','0',
		'1','2','3','4','5','6','7','8','9','0',
		'1','2','3','4','5','6','7','8','9','0',
		'1','2','3','4','5','6','7','8','9','0',
		'1','2','3','4','5','6','7','8','9','0',
		'1','2','3','4','5','6','7','8','9','0',
		'1','2','3','4','5','6','7','8','9','0'
    };

    final static byte DB_SFT_BITS = 0x07;	/* mask out double flag bit after shifting DB_SHIFT */

    public Object clone()
    {
        AnsiEncoder encoder = null;
        try
        {
            encoder = (AnsiEncoder)super.clone();
        }
        catch (CloneNotSupportedException e)
        {
            e.printStackTrace(System.err);
        }
        return encoder;
    }

    /**
     * Instantiates a new ansi encoder.
     */
    public AnsiEncoder()
    {
    }

    /**
     * Qa encode.
     *
     * @param page the page
     * @param text the text
     * @param fadeEnable the fade enable
     * @param updateList the update list
     * @param _nNumberOfRows the n number of rows
     * @param _nNumberOfColumns the n number of columns
     * @return the long
     */
    public long qa_encode(PageType page, ByteArrayOutputStream text, boolean fadeEnable, ListType updateList, short _nNumberOfRows,
            short _nNumberOfColumns)
    {
        byte  fore; /* temp foreground color */
        byte  back; /* temp background color */
        int   db_flag = 0;
        short mod_count; /* 03/09/87 */
        short col;       /* 03/03/87 */
        short save_col;  /* 8/23/93 modify space saving strategy */
        @SuppressWarnings("unused")
        int count = 0;
        /* initially start index at 0 and put in initial esc sequences if it is an update */
        _ansi_addstr(text, ESCCURSOROFF);
        _ansi_addstr(text, ESCSCREENWRAP);
        _ansi_addstr(text, ESCPLAIN);
        /* if G0 and G1 specified load proper ansi sequences */
        char c_G0 = 0;
        if ((c_G0 = page.status.G0_set) != '\0')
        {
            _ansi_addstr(text, ESC_G0_SET);
            text.write((byte)c_G0);
        }
        char c_G1 = 0;
        if ((c_G1 = page.status.G1_set) != '\0')
        {
            _ansi_addstr(text, ESC_G1_SET);
            text.write((byte)c_G1);
        }

        /* use invalid Graphics set designation (Delete) to force a graphic set selection on first character */
        char r_gs_flag = '\177';
        char cur_attr = CharType.PLAIN;
        char fading = CharType.PLAIN;
        char pr_color = CharType.MONO;   /* prev color 01/07/88 */
        char pr_f_color = CharType.MONO; /* prev fading color 01/07/88 */
        boolean stop = false;
        byte db_attr = 0; /* double flag, etc BF 10/92 */

        /* Last_mod is initialized to -1 when page is constructed.
         * But PageType initialize the value to 0, so test for 0 is used */
        if (page.last_mod != -1)
            mod_count = page.last_mod;
        else
            mod_count = 0;
        int page_pos = 0;
        UpdateType modptr = new UpdateType();
        CharType chptr = new CharType();
        int chptr_start = 0;
        char sv_pr_color = 0; /* saving state for skip space code */
        char sv_cur_attr = 0;
        char sv_fading = 0;
        char sv_pr_f_color = 0;
        char sv_r_gs_flag = 0;
        char changes = 0;

        for (; mod_count <= updateList.index && !stop; mod_count++)
        {
            modptr = updateList.upd_list[mod_count];
            col = modptr.upd_beg;
            page_pos = ((((modptr.row) - 1) * _nNumberOfColumns) + ((col) - 1));

            /* if this is on last line, turn off auto-wrap */
            if (modptr.row == _nNumberOfRows)
            {
                _ansi_addstr(text, ESCWRAPOFF);
                text.write((byte)c_G1);
            }

            /* BF 10/92 skip spaces if whole line being rewitten */
            boolean whole_line_flag = false;
            int skip_spc = 0;
            /* BF 10/92 flag indicating spaces with no attr or color can be skipped because the line has been cleared */

            if (modptr.upd_beg == 1 && modptr.upd_end == (_nNumberOfColumns + 1))
            {
                skip_spc = 1;
                whole_line_flag = true;
            }
            else
            {
                skip_spc = 0;
            }

            boolean mvflag = true; /* 09/06/88 */
            int spc_start = 0; /* save location where spaces start */

            for (; col < modptr.upd_end; col++, page_pos++)
            {
                chptr = page.page[page_pos];
                if (chptr.ch == '\0')
                {
                    /* skip nulls, but mark that we should move cursor */
                    mvflag = true;
                    continue;
                    /* BF 10/92 skip spaces logic */
                }
                else if ((skip_spc != 0) && spc_start == 0
                        && chptr.ch == ' ' 
                        && (chptr.attr != CharType.PLAIN)
                        && (chptr.c_attr == CharType.MONO || ((chptr.c_attr & CharType.BACK_COL_MASK) != 0)))
                {
                    spc_start = text.size(); /* save state */
                    chptr_start = page_pos;
                    sv_cur_attr = cur_attr;
                    sv_pr_color = pr_color;
                    sv_fading = fading;
                    sv_pr_f_color = pr_f_color;
                    sv_r_gs_flag = r_gs_flag;
                }
                else if (spc_start != 0 &&
                        (chptr.ch != ' ' || (chptr.attr != CharType.PLAIN)
                        || (chptr.c_attr != CharType.MONO && (0 != (chptr.c_attr & CharType.BACK_COL_MASK)))))
                {
                    if ((page_pos - chptr_start) > 10)
                    {
                        count = spc_start;
                        cur_attr = sv_cur_attr;
                        sv_cur_attr = 0;
                        pr_color = sv_pr_color;
                        fading = sv_fading;
                        pr_f_color = sv_pr_f_color;
                        if ((cur_attr & StatusType.DB_BITS) != 0)
                            db_flag = 1;
                        r_gs_flag = sv_r_gs_flag;
                        mvflag = true;
                    }
                    spc_start = 0;
                }

                if (whole_line_flag)
                {
                    save_col = col;
                    col = 1;
                    _ansi_wrpos(text, (byte)modptr.row, (byte)col);
                    _ansi_addstr(text, ESCCLRLINE);
                    col = save_col;
                    mvflag = true;
                    whole_line_flag = false;
                }
                if (mvflag == true)
                {
                    _ansi_wrpos(text, modptr.row, col);
                    mvflag = false;
                }

                /* determine if a change of graphics sets is required;
                 * check if currently invoked set is desired for next char */
                if (r_gs_flag != chptr.gs && (r_gs_flag != CharType.USAscii || isprint(chptr.gs)))
                {
                    r_gs_flag = chptr.gs;
                    /* if graphic set undefined, set to US_ASCII */
                    if (!isprint(r_gs_flag))
                    {
                        r_gs_flag = CharType.USAscii;
                    }
                    if (r_gs_flag == c_G0)
                    {
                        /* G0 set already designated properly, only a shift is required. */
                        text.write(USE_G0);
                    }
                    else
                    {
                        if (r_gs_flag == c_G1)
                        {
                            /* G0 set already designated properly, only a shift is required. */
                            byte use_G1 = USE_G1;
                            text.write(use_G1);
                        }
                        else
                        {
                            if (c_G0 != '\0')
                            {
                                /* set not presently designated, invoke as G1 set */
                                c_G1 = r_gs_flag;
                                _ansi_addstr(text, ESC_G1_SET);
                                text.write(c_G1);
                                text.write(USE_G1);
                            }
                            else
                            {
                                /* no pre-determined G0 set, assume first set required is the most often used */
                                c_G0 = r_gs_flag;
                                _ansi_addstr(text, ESC_G0_SET);
                                text.write(c_G0);
                                text.write(USE_G0);
                            }
                        }
                    }
                }
                /* determine changes needed in tflag then update cur_attr */
                changes = (char)(cur_attr ^ chptr.attr);
                cur_attr = chptr.attr;
                /* check to see if any attributes changed */
                if (!no_attr_ht(changes) || (chptr.c_attr != pr_color))
                {
                    _ansi_addstr(text, CSI);

                    if (cur_attr < 32)
                    {
                        if (db_flag != 0)
                            db_attr = AnsiDecoder.DB_OFF;
                        _ansi_addstr(text, Attribute.attribute[att_bits(cur_attr)].toBytes());
                        /* double high/wide flag set */
                    }
                    else if ((cur_attr & StatusType.DB_BITS) != 0)
                    {
                        db_flag = 1;
                        /* determine sequence to send */
                        db_attr = (byte)sdb_bits((char)(cur_attr >> StatusType.DB_SHIFT));
                        /* add normal attribute bits as above */
                        _ansi_addstr(text, Attribute.attribute[att_bits(cur_attr)].toBytes());
                    }
                    else
                    {
                        _ansi_addstr(text, Attribute.attribute[0].toBytes());
                    }

                    fore = (byte)(chptr.c_attr & 0x0f);
                    back = (byte)(chptr.c_attr >> 4);

                    if (fore != 0x0f)
                        _ansi_addstr(text, ForegroundColor.foregroundColor[fore].toBytes());
                    if (back != 0x0f)
                        _ansi_addstr(text, BackgroundColor.backgroundColor[back].toBytes());
                    pr_color = chptr.c_attr;

                    /* print end of set */
                    _ansi_addstr(text, ENDSET);
                    if (db_flag != 0)
                    {
                        /* output double string BF 10/92 */
                        if (db_attr < 7)
                            _ansi_addstr(text, DoubleSquence.doubleSequence[db_attr].toBytes());
                        if (db_attr == AnsiDecoder.DB_OFF)
                            db_flag = 0;
                    }
                }
                /* if fading disabled, add char and return */
                if (!fadeEnable)
                {
                    text.write((byte)chptr.ch);
                    continue;
                }
                /* determine changes needed in fading */
                changes = (char)(fading ^ chptr.fade_attr);
                fading = chptr.fade_attr;
                if (no_attr_ht(changes) && (chptr.c_fade_attr == pr_f_color))
                {
                    /* if nothing changed add the character and return */
                    text.write((byte)chptr.ch);
                    continue;
                }

                /* some attributes changed so reset the fading */
                _ansi_addstr(text, CSI_FADE);
                if (chptr.fade_attr < 32)
                {
                    _ansi_addstr(text, Attribute.attribute[chptr.fade_attr].toBytes());
                }
                else
                {
                    _ansi_addstr(text, Attribute.attribute[0].toBytes());
                }
                fore = (byte)(chptr.c_fade_attr & 0x0f);
                back = (byte)(chptr.c_fade_attr >> 4);

                if (fore != 0x0f)
                    _ansi_addstr(text, ForegroundColor.foregroundColor[fore].toBytes());
                if (back != 0x0f)
                    _ansi_addstr(text, BackgroundColor.backgroundColor[back].toBytes());
                pr_f_color = chptr.c_fade_attr;

                _ansi_addstr(text, ENDSET);
                /* generate the character and return */
                text.write((byte)chptr.ch);
            }
            if (spc_start != 0)
            {
                /* EOL ?? BF 10/92 */
                // if((chptr - chptr_start) > 10)
                if ((page_pos - chptr_start) > 10)
                {
                    count = spc_start;
                    cur_attr = sv_cur_attr;
                    sv_cur_attr = 0;
                    pr_color = sv_pr_color;
                    fading = sv_fading;
                    pr_f_color = sv_pr_f_color;
                    if ((cur_attr & StatusType.DB_BITS) != 0)
                        db_flag = 1;
                    r_gs_flag = sv_r_gs_flag;
                }
                spc_start = 0;
            }
        }

        if (((cur_attr & StatusType.DB_BITS) != 0) || (0 != (sv_cur_attr & StatusType.DB_BITS)))
        {
            /* if any double sequences sent */
            _ansi_addstr(text, DoubleSquence.doubleSequence[AnsiDecoder.DB_OFF].toBytes());
        }
        _ansi_addstr(text, ESCFADEOFF);
        text.write('\0');
        if (!stop)
        {
            return (DONE);
        }

        /* redo the last modified field */
        page.last_mod = (short)(mod_count - 1);
        return (NOT_DONE);
    }

    int _ansi_addstr(ByteArrayOutputStream buffer, byte[] str)
    {
        try
        {
            buffer.write(str);
        }
        catch (IOException ioe)
        {
            ioe.printStackTrace();
        }
        return str.length;
    }

    int _ansi_wrpos(ByteArrayOutputStream str, int row, int col)
    {
        str.write('\033');
        str.write('[');
        str.write(tens[row]);
        str.write(ones[row]);
        str.write(';');
        str.write(tens[col]);
        str.write(ones[col]);
        str.write('H');
        return(8);
    }

    boolean isprint(char c)
    {
        return (c >= 32) && (c <= 126);
    }

    boolean no_attr_ht(char attrib)
    {
        return (attrib == 0);
    }

    int att_bits(char gflag)
    {
        return ((gflag) & StatusType.DB_ATT_MASK);
    }

    int sdb_bits(char gflag)
    {
        return ((gflag) & DB_SFT_BITS);
    }
}
