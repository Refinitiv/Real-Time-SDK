/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2023-2024 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

namespace LSEG.Eta.Ansi;

/// <summary>
/// The Class AnsiEncoder.
/// </summary>
/// <seealso cref="AnsiDecoder"/>
public class AnsiEncoder : ICloneable
{
    const byte ESC = 0x1B;

    static readonly byte[] ESCCURSOROFF = { ESC, (byte)'[', (byte)'?', (byte)'2', (byte)'5', (byte)'l' };
    static readonly byte[] ESCSCREENWRAP = { ESC, (byte)'[', (byte)'?', (byte)'7', (byte)'h' };
    static readonly byte[] ESCWRAPOFF = { ESC, (byte)'[', (byte)'?', (byte)'7', (byte)'l' };
    static readonly byte[] ESCFADEBLINK = { ESC, (byte)'[', (byte)'>', (byte)'5', (byte)'m' };
    static readonly byte[] ESCFADEOFF = { ESC, (byte)'[', (byte)'>', (byte)'0', (byte)'m' };
    static readonly byte[] ESCRESETSCR = { ESC, (byte)'c' };
    static readonly byte[] ESCPLAIN = { ESC, (byte)'[', (byte)'m' };
    static readonly byte[] ESCCLRLINE = { ESC, (byte)'[', (byte)'2', (byte)'K' };

    /* the following are used to build up an attribute change escape sequence */
    static readonly byte[] CSI_FADE = { ESC, (byte)'[', (byte)'>' };
    static readonly byte[] CSI = { ESC, (byte)'[' };
    static readonly byte[] ENDSET = { (byte)'m' };
    static readonly byte[] ESCMVCURS = { ESC, (byte)'[', (byte)'%', (byte)'d', (byte)';', (byte)'%', (byte)'d', (byte)'H' };
    static readonly byte[] ESC_G0_SET = { (byte)ESC, (byte)'(' };
    static readonly byte[] ESC_G1_SET = { (byte)ESC, (byte)')' };

    const byte USE_G0 = 0xF; // ASCII SO
    const byte USE_G1 = 0xE; // ASCII SI
    const int MAX_SINGLE_ANSI_SEQUENCE = 30;

    /// <summary>
    /// Return code when encoder was able to encode provided updates.
    /// </summary>
    /// <seealso cref="QaEncode(PageType, Stream, bool, ListType, short, short)"/>
    public const int DONE = 0;

    /// <summary>
    /// Return code when encoder did not encode all provided updates.
    /// </summary>
    /// <seealso cref="QaEncode(PageType, Stream, bool, ListType, short, short)"/>
    public const int NOT_DONE = 1;

    /* define tens and ones arrays to speed building of escape sequence for moving the cursor */
    static readonly char[] Tens = { '0',
        '0','0','0','0','0','0','0','0','0','1',
        '1','1','1','1','1','1','1','1','1','2',
        '2','2','2','2','2','2','2','2','2','3',
        '3','3','3','3','3','3','3','3','3','4',
        '4','4','4','4','4','4','4','4','4','5',
        '5','5','5','5','5','5','5','5','5','6',
        '6','6','6','6','6','6','6','6','6','7',
        '7','7','7','7','7','7','7','7','7','8'
    };

    static readonly char[] Ones = { '0',
        '1','2','3','4','5','6','7','8','9','0',
        '1','2','3','4','5','6','7','8','9','0',
        '1','2','3','4','5','6','7','8','9','0',
        '1','2','3','4','5','6','7','8','9','0',
        '1','2','3','4','5','6','7','8','9','0',
        '1','2','3','4','5','6','7','8','9','0',
        '1','2','3','4','5','6','7','8','9','0',
        '1','2','3','4','5','6','7','8','9','0'
    };

    const byte DB_SFT_BITS = 0x07;   /* mask out double flag bit after shifting DB_SHIFT */

    #region Public methods

    /// <summary>
    /// Returns a new AnsiEncoder instance.
    /// </summary>
    /// <returns>new AnsiEncoder instance</returns>
    Object ICloneable.Clone() => Clone();

    /// <summary>
    /// Returns a new AnsiEncoder instance.
    /// </summary>
    /// <returns>new AnsiEncoder instance</returns>
    public AnsiEncoder Clone()
    {
        return new AnsiEncoder();
    }

    /// <summary>
    /// Instantiates a new ansi encoder.
    /// </summary>
    public AnsiEncoder()
    {
    }

    /// <summary>
    /// Encodes stored page image and update list using ANSI X3.64 protocol.
    /// </summary>
    ///
    /// <remarks>
    /// <para>
    /// This routine takes a stored page image and encodes it into ANSI X3.64
    /// protocol.</para>
    ///
    /// <para>
    /// QaEncode processes each entry in the update list up to the index.  For
    /// the row and start column specified in the update list to (but not
    /// including) the end column, the corresponding characters and attributes
    /// in the page image are added to <paramref name="text"/>.</para>
    /// </remarks>
    ///
    /// <param name="page">stored page image</param>
    /// <param name="text">buffer for ANSI text</param>
    /// <param name="fadeEnable">flag to enable fading</param>
    /// <param name="updateList"> update list with a list of all the blocks that
    ///     need to be encoded.</param>
    /// <param name="numberOfRows"> the n number of rows</param>
    /// <param name="numberOfColumns"> the n number of columns</param>
    ///
    /// <returns>
    /// If QaEncode is able to encode all of the data regions defined by the
    /// update list into <paramref name="text"/>, it returns <see cref="DONE"/>.
    ///
    /// page.LastMod in the page image is set to the entry in the update list
    /// where QaEncode will resume encoding the next time it is called.
    /// QaEncode may be called as many times as necessary to completely encode
    /// all the regions defined in the update list.
    /// </returns>
    public long QaEncode(PageType page, Stream text, bool fadeEnable, ListType updateList, short numberOfRows,
            short numberOfColumns)
    {
        byte fore; /* temp foreground color */
        byte back; /* temp background color */
        int db_flag = 0;
        short mod_count; /* 03/09/87 */
        short col;       /* 03/03/87 */
        short save_col;  /* 8/23/93 modify space saving strategy */
        int count = 0;
        /* initially start index at 0 and put in initial esc sequences if it is an update */
        AnsiAddString(text, ESCCURSOROFF);
        AnsiAddString(text, ESCSCREENWRAP);
        AnsiAddString(text, ESCPLAIN);
        /* if G0 and G1 specified load proper ansi sequences */
        char c_G0 = (char)0;
        if ((c_G0 = page.Status.G0_Set) != '\0')
        {
            AnsiAddString(text, ESC_G0_SET);
            text.WriteByte((byte)c_G0);
        }
        char c_G1 = (char)0;
        if ((c_G1 = page.Status.G1_Set) != '\0')
        {
            AnsiAddString(text, ESC_G1_SET);
            text.WriteByte((byte)c_G1);
        }

        /* use invalid Graphics set designation (Delete) to force a graphic set selection
         * on first character */
        char r_gs_flag = (char)177;
        char cur_attr = CharType.PLAIN;
        char fading = CharType.PLAIN;
        char pr_color = CharType.MONO;   /* prev color 01/07/88 */
        char pr_f_color = CharType.MONO; /* prev fading color 01/07/88 */
        bool stop = false;
        byte db_attr = 0; /* double flag, etc BF 10/92 */

        /* Last_mod is initialized to -1 when page is constructed.
         * But PageType initialize the value to 0, so test for 0 is used */
        if (page.LastMod != -1)
            mod_count = page.LastMod;
        else
            mod_count = 0;
        int page_pos = 0;
        UpdateType modptr = new UpdateType();
        CharType chptr = new CharType();
        int chptr_start = 0;
        char sv_pr_color = (char)0; /* saving state for skip space code */
        char sv_cur_attr = (char)0;
        char sv_fading = (char)0;
        char sv_pr_f_color = (char)0;
        char sv_r_gs_flag = (char)0;
        char changes = (char)0;

        for (; mod_count <= updateList.Index && !stop; mod_count++)
        {
            modptr = updateList.UpdList[mod_count];
            col = modptr.UpdateColBegin;
            page_pos = ((((modptr.Row) - 1) * numberOfColumns) + ((col) - 1));

            /* if this is on last line, turn off auto-wrap */
            if (modptr.Row == numberOfRows)
            {
                AnsiAddString(text, ESCWRAPOFF);
                text.WriteByte((byte)c_G1);
            }

            /* BF 10/92 skip spaces if whole line being rewitten */
            bool whole_line_flag = false;
            int skip_spc = 0;
            /* BF 10/92 flag indicating spaces with no attr or color can be skipped
             * because the line has been cleared */

            if (modptr.UpdateColBegin == 1 && modptr.UpdateColEnd == (numberOfColumns + 1))
            {
                skip_spc = 1;
                whole_line_flag = true;
            }
            else
            {
                skip_spc = 0;
            }

            bool mvflag = true; /* 09/06/88 */
            int spc_start = 0; /* save location where spaces start */

            for (; col < modptr.UpdateColEnd; col++, page_pos++)
            {
                chptr = page.Page[page_pos];
                if (chptr.Ch == '\0')
                {
                    /* skip nulls, but mark that we should move cursor */
                    mvflag = true;
                    continue;
                    /* BF 10/92 skip spaces logic */
                }
                else if ((skip_spc != 0) && spc_start == 0
                        && chptr.Ch == ' '
                        && (chptr.Attr != CharType.PLAIN)
                        && (chptr.ColorAttrib == CharType.MONO || ((chptr.ColorAttrib & CharType.BACK_COL_MASK) != 0)))
                {
                    spc_start = (int)text.Length; /* save state */
                    chptr_start = page_pos;
                    sv_cur_attr = cur_attr;
                    sv_pr_color = pr_color;
                    sv_fading = fading;
                    sv_pr_f_color = pr_f_color;
                    sv_r_gs_flag = r_gs_flag;
                }
                else if (spc_start != 0 &&
                        (chptr.Ch != ' ' || (chptr.Attr != CharType.PLAIN)
                        || (chptr.ColorAttrib != CharType.MONO && (0 != (chptr.ColorAttrib & CharType.BACK_COL_MASK)))))
                {
                    if ((page_pos - chptr_start) > 10)
                    {
                        count = spc_start;
                        cur_attr = sv_cur_attr;
                        sv_cur_attr = (char)0;
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
                    AnsiWrpos(text, (byte)modptr.Row, (byte)col);
                    AnsiAddString(text, ESCCLRLINE);
                    col = save_col;
                    mvflag = true;
                    whole_line_flag = false;
                }
                if (mvflag == true)
                {
                    AnsiWrpos(text, modptr.Row, col);
                    mvflag = false;
                }

                /* determine if a change of graphics sets is required;
                 * check if currently invoked set is desired for next char */
                if (r_gs_flag != chptr.GS && (r_gs_flag != CharType.USAscii || IsPrint(chptr.GS)))
                {
                    r_gs_flag = chptr.GS;
                    /* if graphic set undefined, set to US_ASCII */
                    if (!IsPrint(r_gs_flag))
                    {
                        r_gs_flag = CharType.USAscii;
                    }
                    if (r_gs_flag == c_G0)
                    {
                        /* G0 set already designated properly, only a shift is required. */
                        text.WriteByte(USE_G0);
                    }
                    else
                    {
                        if (r_gs_flag == c_G1)
                        {
                            /* G0 set already designated properly, only a shift is required. */
                            byte use_G1 = USE_G1;
                            text.WriteByte(use_G1);
                        }
                        else
                        {
                            if (c_G0 != '\0')
                            {
                                /* set not presently designated, invoke as G1 set */
                                c_G1 = r_gs_flag;
                                AnsiAddString(text, ESC_G1_SET);
                                text.WriteByte((byte)c_G1);
                                text.WriteByte(USE_G1);
                            }
                            else
                            {
                                /* no pre-determined G0 set, assume first set required is the most often used */
                                c_G0 = r_gs_flag;
                                AnsiAddString(text, ESC_G0_SET);
                                text.WriteByte((byte)c_G0);
                                text.WriteByte(USE_G0);
                            }
                        }
                    }
                }
                /* determine changes needed in tflag then update cur_attr */
                changes = (char)(cur_attr ^ chptr.Attr);
                cur_attr = chptr.Attr;
                /* check to see if any attributes changed */
                if (!NoAttributeHt(changes) || (chptr.ColorAttrib != pr_color))
                {
                    AnsiAddString(text, CSI);

                    if (cur_attr < 32)
                    {
                        if (db_flag != 0)
                            db_attr = AnsiDecoder.DB_OFF;
                        AnsiAddString(text, AnsiAttribute.attribute[AttributeBits(cur_attr)].ToBytes());
                        /* double high/wide flag set */
                    }
                    else if ((cur_attr & StatusType.DB_BITS) != 0)
                    {
                        db_flag = 1;
                        /* determine sequence to send */
                        db_attr = (byte)SdbBits((char)(cur_attr >> StatusType.DB_SHIFT));
                        /* add normal attribute bits as above */
                        AnsiAddString(text, AnsiAttribute.attribute[AttributeBits(cur_attr)].ToBytes());
                    }
                    else
                    {
                        AnsiAddString(text, AnsiAttribute.attribute[0].ToBytes());
                    }

                    fore = (byte)(chptr.ColorAttrib & 0x0f);
                    back = (byte)(chptr.ColorAttrib >> 4);

                    if (fore != 0x0f)
                        AnsiAddString(text, ForegroundColor.ForegroundColors[fore].ToBytes());
                    if (back != 0x0f)
                        AnsiAddString(text, BackgroundColor.BackgroundColors[back].ToBytes());
                    pr_color = chptr.ColorAttrib;

                    /* print end of set */
                    AnsiAddString(text, ENDSET);
                    if (db_flag != 0)
                    {
                        /* output double string BF 10/92 */
                        if (db_attr < 7)
                            AnsiAddString(text, DoubleSquence.DoubleSequences[db_attr].ToBytes());
                        if (db_attr == AnsiDecoder.DB_OFF)
                            db_flag = 0;
                    }
                }
                /* if fading disabled, add char and return */
                if (!fadeEnable)
                {
                    text.WriteByte((byte)chptr.Ch);
                    continue;
                }
                /* determine changes needed in fading */
                changes = (char)(fading ^ chptr.FadeAttr);
                fading = chptr.FadeAttr;
                if (NoAttributeHt(changes) && (chptr.ColorFadeAttr == pr_f_color))
                {
                    /* if nothing changed add the character and return */
                    text.WriteByte((byte)chptr.Ch);
                    continue;
                }

                /* some attributes changed so reset the fading */
                AnsiAddString(text, CSI_FADE);
                if (chptr.FadeAttr < 32)
                {
                    AnsiAddString(text, AnsiAttribute.attribute[chptr.FadeAttr].ToBytes());
                }
                else
                {
                    AnsiAddString(text, AnsiAttribute.attribute[0].ToBytes());
                }
                fore = (byte)(chptr.ColorFadeAttr & 0x0f);
                back = (byte)(chptr.ColorFadeAttr >> 4);

                if (fore != 0x0f)
                    AnsiAddString(text, ForegroundColor.ForegroundColors[fore].ToBytes());
                if (back != 0x0f)
                    AnsiAddString(text, BackgroundColor.BackgroundColors[back].ToBytes());
                pr_f_color = chptr.ColorFadeAttr;

                AnsiAddString(text, ENDSET);
                /* generate the character and return */
                text.WriteByte((byte)chptr.Ch);
            }
            if (spc_start != 0)
            {
                /* EOL ?? BF 10/92 */
                // if((chptr - chptr_start) > 10)
                if ((page_pos - chptr_start) > 10)
                {
                    count = spc_start;
                    cur_attr = sv_cur_attr;
                    sv_cur_attr = (char)0;
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
            AnsiAddString(text, DoubleSquence.DoubleSequences[AnsiDecoder.DB_OFF].ToBytes());
        }
        AnsiAddString(text, ESCFADEOFF);
        text.WriteByte(0);
        if (!stop)
        {
            return (DONE);
        }

        /* redo the last modified field */
        page.LastMod = (short)(mod_count - 1);
        return (NOT_DONE);
    }
    #endregion

    #region Private methods

    private int AnsiAddString(Stream buffer, byte[] str)
    {
        try
        {
            buffer.Write(str);
        }
        catch (Exception ex)
        {
            Console.Error.WriteLine(ex.Message);
        }
        return str.Length;
    }

    private int AnsiWrpos(Stream str, int row, int col)
    {
        str.WriteByte(ESC);
        str.WriteByte((byte)'[');
        str.WriteByte((byte)Tens[row]);
        str.WriteByte((byte)Ones[row]);
        str.WriteByte((byte)';');
        str.WriteByte((byte)Tens[col]);
        str.WriteByte((byte)Ones[col]);
        str.WriteByte((byte)'H');
        return (8);
    }

    bool IsPrint(char c)
    {
        return (c >= 32) && (c <= 126);
    }

    bool NoAttributeHt(char attrib)
    {
        return (attrib == 0);
    }

    int AttributeBits(char gflag)
    {
        return ((gflag) & StatusType.DB_ATT_MASK);
    }

    int SdbBits(char gflag)
    {
        return ((gflag) & DB_SFT_BITS);
    }

    #endregion
}
