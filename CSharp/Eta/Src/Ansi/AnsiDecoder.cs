/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2023 Refinitiv. All rights reserved.              --
 *|-----------------------------------------------------------------------------
 */

namespace LSEG.Eta.Ansi;

/// <summary>
/// Encapsulates ANSI page parser.
/// </summary>
/// <seealso cref="AnsiEncoder"/>
public class AnsiDecoder : IAnsi, ICloneable
{
    private ParserType m_Parser = new ParserType();

    /// <summary>Operation completed successfully.</summary>
    public const short OK = 1;

    /// <summary>Operation did not succeed.</summary>
    public const short ERROR = -1;

    const short S_UP = 0;
    const short S_DOWN = 1;
    const short S_LEFT = 2;
    const short S_RIGHT = 3;

    /* escape sequence number and out sequnce table offsets */
    const int DBH_TP = 1;   /* double high top */
    const int DBH_BT = 2;   /* double high bottom */
    const int DBHW_TP = 3;   /* double high/wide top */
    const int DBHW_BT = 4;

    /* double high/wide bottom */
    internal const int DB_OFF = 5;   /* double off */
    internal const int DBWD = 6;   /* double wide single high */
    internal const int DB_MIN = 1;
    internal const int DB_MAX = 6;

    static readonly short[] TAB_TABLE = {
        9,  9,  9,  9,  9,  9,  9,  9,		/* 1 - 8 */
	    17, 17, 17, 17, 17, 17, 17, 17,		/* 9 - 16 */
	    25, 25, 25, 25, 25, 25, 25, 25,		/* 17 - 24 */
	    33, 33, 33, 33, 33, 33, 33, 33,		/* 25 - 32 */
	    41, 41, 41, 41, 41, 41, 41, 41,		/* 33 - 40 */
	    49, 49, 49, 49, 49, 49, 49, 49,		/* 41 - 48 */
	    57, 57, 57, 57, 57, 57, 57, 57,		/* 49 - 56 */
	    65, 65, 65, 65, 65, 65, 65, 65,		/* 57 - 64 */
	    73, 73, 73, 73, 73, 73, 73, 73,		/* 65 - 72 */
	    80, 80, 80, 80, 80, 80, 80, 80 };   /* 73 - 80 */

    short PageColumns = 80;
    short PageRows = 25;
    short ScrollBot = 24;
    short EndOfRow = 81;

    internal const int INIT_STATE = 0;
    internal const int ESCAPE_STATE = 1;
    internal const int CSI_STATE = 2;
    internal const int G0_STATE = 3;
    internal const int G1_STATE = 4;
    internal const int CHARACTER_SIZE_STATE = 5;

    const int CR_STATE = 20;
    const int LF_STATE = 21;

    /// <summary>Max params in Control Sequence Introducer (CSI) sequence.
    /// </summary>
    public const int MAXCSIPARAMS = 20;

    #region Private methods

    /// <summary>
    ///	This routine is called when a parameter delimiter is parsed.  Increment the number
    /// of parameters, checking to make sure we don't go out of bounds.
    /// </summary>
    ///
    /// <returns><see cref="OK"/></returns>
    private short AnsiChparm()
    {
        if (m_Parser.ParamCount < MAXCSIPARAMS)
        {
            m_Parser.ParamCount++;
            m_Parser.Params[m_Parser.ParamCount - 1] = 0;
        }
        return OK;
    }

    /// <summary>
    ///	Moves cursor to the start of the current line.
    ///	</summary>
    ///
    /// <returns><see cref="ERROR"/> if update list would overflow, else <see cref="OK"/></returns>
    private short AnsiCret(PageType page, ListType u_list)
    {
        page.Status.Column = 1;
        if (m_Parser.CrState != LF_STATE)
        {
            NewUpd(u_list, 1, page.Status.Row, (short)1, (short)1);
        }
        else
        {
            u_list.UpdList[u_list.Index].UpdateColBegin = 1;
            u_list.UpdList[u_list.Index].UpdateColEnd = 1;
        }
        m_Parser.CrState = CR_STATE;
        m_Parser.State = INIT_STATE;
        m_Parser.Wrap = false;
        return OK;
    }

    /// <summary>
    /// Enters the CSI state, i.e., we have parsed an escape character followed by a '['.
    ///	The next thing to follow may be one or many parameters.
    ///	</summary>
    ///
    /// <returns><see cref="OK"/></returns>
    private short AnsiCsi()
    {
        m_Parser.State = CSI_STATE;
        return OK;
    }

    /// <summary>
    /// Moves cursor backward param_cnt positions in the line, stopping at the beginning,
    /// defaulting to one, and moving one if param_cnt = 0.
    /// </summary>
    ///
    /// <returns><see cref="ERROR"/> if update list would overflow, else <see cref="OK"/></returns>
    private short AnsiCub(PageType page, ListType u_list)
    {
        short num_col;
        num_col = (short)((m_Parser.ParamCount == 0) ? 1 : m_Parser.Params[0]);

        if (num_col < 1)
            num_col = 1; /* default and range check */

        /* Moving cursor back, being sure to go no further than the start of the line. */
        if (page.Status.Column - num_col <= 0)
            page.Status.Column = 1;
        else
            page.Status.Column -= num_col;

        NewUpd(u_list, 1, page.Status.Row, page.Status.Column, page.Status.Column);
        m_Parser.State = INIT_STATE;
        m_Parser.Wrap = false;
        return OK;
    }

    /// <summary>
    /// This routine is called whenever the cursor is moved down but scrolling should not
    /// occur.  This routine is used for processing the CUD sequence, but NOT for new
    /// lines.
    /// </summary>
    ///
    /// <returns><see cref="ERROR"/> if update list would overflow, else <see cref="OK"/></returns>
    private short AnsiCud(PageType page, ListType u_list)
    {
        short num_rows;
        StatusType sts = page.Status;

        num_rows = ((m_Parser.ParamCount == 0) ? (short)1 : m_Parser.Params[0]);

        if (sts.Row <= page.ScrollBottom)
        {
            if (sts.Row + num_rows > page.ScrollBottom)
                sts.Row = page.ScrollBottom;
            else
                sts.Row += num_rows;
        }
        else
        {
            /* cursor must be between scroll bottom and end of page */
            if (sts.Row + num_rows > PageRows)
                sts.Row = PageRows;
            else
                sts.Row += num_rows;
        }

        NewUpd(u_list, 1, sts.Row, sts.Column, sts.Column);
        m_Parser.State = INIT_STATE;
        m_Parser.Wrap = false;
        return OK;
    }

    /// <summary>
    /// Moves cursor forward param_cnt positions in the line, stopping at the line end,
    /// defaulting to one, and moving one if param_cnt = 0.
    /// </summary>
    ///
    /// <returns><see cref="ERROR"/> if update list would overflow, else <see cref="OK"/></returns>
    private short AnsiCuf(PageType page, ListType u_list)
    {
        short numcol;

        if (m_Parser.SpecialEsc != 0)
        {
            switch (m_Parser.SpecialEsc)
            {
                case '=':
                    break; /* invisible cursor currently ignored */
                default:
                    break;
            }
        }
        else
        {
            numcol = (m_Parser.ParamCount == 0) ? (short)1 : m_Parser.Params[0];

            /* Moving cursor forward, making sure to stop at the end of the line */
            if (page.Status.Column + numcol > PageColumns)
                page.Status.Column = PageColumns;
            else
                page.Status.Column += numcol;
            NewUpd(u_list, 1, page.Status.Row, page.Status.Column, page.Status.Column);
        }
        m_Parser.State = INIT_STATE;
        return OK;
    }

    /// <summary>
    /// Moves cursor up params[0] positions, stopping at the top line, defaulting to one,
    /// and moving one if params[0] = 0.
    /// </summary>
    ///
    /// <returns><see cref="ERROR"/> if update list would overflow, else <see cref="OK"/></returns>
    private short AnsiCuu(PageType page, ListType u_list)
    {
        short num_rows;
        StatusType sts = page.Status;

        num_rows = (m_Parser.ParamCount == 0) ? (short)1 : m_Parser.Params[0];

        if (sts.Row >= page.ScrollTop)
        {
            if (sts.Row - num_rows < page.ScrollTop)
                sts.Row = page.ScrollTop;
            else
                sts.Row -= num_rows;
        }
        else
        {
            /* cursor must be between scroll top and top of page */
            if (sts.Row - num_rows < 1)
                sts.Row = 1;
            else
                sts.Row -= num_rows;
        }

        NewUpd(u_list, 1, sts.Row, sts.Column, sts.Column);
        m_Parser.State = INIT_STATE;
        m_Parser.Wrap = false;
        return OK;
    }

    /// <summary>
    ///	(Set double high/wide private sequence).
    ///	<c>ESC[&gt;#Z</c>
    /// </summary>
    ///
    /// <returns><see cref="OK"/></returns>
    private short AnsiDbP(PageType page, ListType u_list)
    {
        /* Not supported */
        m_Parser.State = INIT_STATE;
        return OK;
    }

    /// <summary>
    /// Deletes params[0] chars, starting at the current position and working
    ///	forward. Defaults to one char if params[0] = 0. Remaining chars are moved up to
    ///	the current position.  The vacated character positions at the end of the line are
    ///	erased.
    /// </summary>
    ///
    /// <returns><see cref="ERROR"/> if number of updates exceeds maximum, otherwise <see cref="OK"/>.</returns>
    private short AnsiDch(PageType page, ListType u_list)
    {
        short num_chars, start, end;
        int i;
        StatusType sts = page.Status;

        num_chars = (m_Parser.ParamCount == 0) ? (short)1 : m_Parser.Params[0];
        if (num_chars < 1)
            num_chars = 1; /* default and range check */

        /* Start deleting characters, starting at the current curs. pos. */
        start = Offset(sts.Row, sts.Column);
        end = Offset(sts.Row, PageColumns);
        NewUpd(u_list, 2, sts.Row, sts.Column, (short)(PageColumns + 1));

        for (i = (start + num_chars); i <= end; ++i)
        {
            page.Page[(i - num_chars)].CopyFrom(page.Page[i]);
        }

        /* Fill in the end with blanks. */
        for (i = end; i >= (end - num_chars); --i)
        {
            page.Page[i].Clear();
        }
        NewUpd(u_list, 1, sts.Row, sts.Column, sts.Column);
        m_Parser.State = INIT_STATE;
        return (OK);
    }

    /// <summary>
    /// DEC private sequence to save cursor.
    /// </summary>
    ///
    /// <returns><see cref="OK"/></returns>
    private short AnsiDecrc(PageType page, ListType u_list)
    {
        page.Status = page.Save;
        StatusType sts = page.Status;
        NewUpd(u_list, 1, sts.Row, sts.Column, sts.Column);
        m_Parser.State = INIT_STATE;
        return OK;
    }

    /// <summary>
    /// DEC private sequence to save cursor.
    /// </summary>
    ///
    /// <returns><see cref="OK"/></returns>
    private short AnsiDecsc(PageType page, ListType u_list)
    {
        page.Save = page.Status;
        m_Parser.State = INIT_STATE;
        return OK;
    }

    /// <summary>
    ///	DEC private sequence to select top and bottoms margins for scrolling.
    /// </summary>
    ///
    /// <returns><see cref="ERROR"/> if update list would overflow, else <see cref="OK"/></returns>
    private short AnsiDecstbm(PageType page, ListType u_list)
    {
        short top;
        short bottom;

        StatusType sts = page.Status;
        switch (m_Parser.ParamCount)
        {
            case 0:
                top = 1;
                bottom = ScrollBot;
                break;
            case 1:
                top = m_Parser.Params[0];
                bottom = ScrollBot;
                break;
            case 2:
                top = m_Parser.Params[0];
                bottom = m_Parser.Params[1];
                break;
            default:
                m_Parser.State = INIT_STATE;
                return OK;
        }

        if (top < 1)
        {
            top = 1;
        }

        if (bottom > PageRows)
        {
            bottom = ScrollBot;
        }

        if (bottom > top)
        {
            page.ScrollTop = top;
            page.ScrollBottom = bottom;
            sts.Row = 1;
            sts.Column = 1;
            /* home the cursor when scrolling region changed */
            NewUpd(u_list, 1, (short)1, (short)1, (short)1);
            m_Parser.Wrap = false;
        }

        m_Parser.State = INIT_STATE;
        m_Parser.Scrolled = false;
        return OK;
    }

    /// <summary>
    /// Designate graphics set -- Installs character into G0 graphics set if it is valid.
    ///	If not, graphics set is not changed.
    ///	</summary>
    ///
    /// <returns><see cref="OK"/></returns>
    private short AnsiDesignateG0(char ch, PageType page)
    {
        if (IsPrint(ch))
        {
            page.Status.G0_Set = ch;
        }
        m_Parser.State = INIT_STATE;
        return OK;
    }

    /// <summary>
    /// Designate graphics set -- Installs character into G1 graphics set if it is valid.
    ///	If not, graphics set is not changed.
    ///	</summary>
    ///
    /// <returns><see cref="OK"/></returns>
    private short AnsiDesignateG1(char ch, PageType page)
    {
        if (IsPrint(ch))
        {
            page.Status.G1_Set = ch;
        }
        m_Parser.State = INIT_STATE;
        return OK;
    }

    /// <summary>
    ///	Deletes params[0] lines, starting at the current line and working downward.
    ///	Defaults to one line and deletes one line if params[0] = 0. Remaining lines are
    ///	moved up to the current line position.
    ///	</summary>
    ///
    /// <returns>Nothing.</returns>
    private short AnsiDeleteLines(PageType page, ListType u_list)
    {
        short num_lines;
        StatusType sts = page.Status;

        if (sts.Row < page.ScrollTop || sts.Row > page.ScrollBottom)
        {
            m_Parser.State = INIT_STATE;
            return OK;
        }

        num_lines = (m_Parser.ParamCount == 0) ? (short)1 : m_Parser.Params[0];
        if (num_lines < 1) /* default and range check */
            num_lines = 1;

        if (num_lines + sts.Row > page.ScrollBottom)
            num_lines = (short)(page.ScrollBottom - sts.Row);

        if (AnsiScroll(page, u_list, num_lines, S_UP, sts.Row) == ERROR)
        {
            m_Parser.State = INIT_STATE;
            return ERROR;
        }
        sts.Column = 1;
        NewUpd(u_list, 1, sts.Row, (short)1, (short)1);
        m_Parser.State = INIT_STATE;
        m_Parser.Wrap = false;
        return OK;
    }

    /// <summary>
	/// Erases some or all of the display, depending on the parameter values.
	/// </summary>
    ///
    /// <returns><see cref="ERROR"/> if number of updates exceeds maximum, otherwise <see cref="OK"/></returns>
    private short AnsiEraseDisplay(PageType page, ListType u_list)
    {
        short i, pg_pos;
        short col;
        short row;
        short end, start;
        short rows_needed; /* num update entries need to make */
        StatusType sts = page.Status;

        /* default and range check. */
        m_Parser.Params[0] = (m_Parser.ParamCount == 0) ? (short)0 : m_Parser.Params[0];

        col = sts.Column;
        row = sts.Row;
        switch (m_Parser.Params[0])
        {
            case 0:
                /* erasing from current position to end of page */
                end = Offset(PageRows, PageColumns); /* end of page */
                pg_pos = Offset(sts.Row, sts.Column);
                rows_needed = (short)(PageRows - sts.Row + 1); /* +1 for last NEW_UPD */
                /* mark current update to end of this row */
                u_list.UpdList[u_list.Index].UpdateColEnd = EndOfRow;

                for (i = pg_pos; i <= end; i++)
                {
                    if (col <= PageColumns)
                    {
                        col++;
                    }
                    else
                    {
                        col = 1;
                        row++;
                        /* mark each new row as the whole row */
                        NewUpd(u_list, rows_needed, row, (short)1, EndOfRow);
                        rows_needed--;
                    }
                    page.Page[i].Clear();
                }
                NewUpd(u_list, 1, sts.Row, sts.Column, sts.Column);
                break;

            case 1:
                /* erasing from start of page to current position */
                start = Offset((short)1, (short)1); /* start of page */
                pg_pos = Offset(sts.Row, sts.Column);
                row = 1;
                /* overwrite current update, since it will be included in the for loop */
                u_list.UpdList[u_list.Index].Row = 1;
                u_list.UpdList[u_list.Index].UpdateColBegin = 1;
                rows_needed = sts.Row;

                for (i = start; i <= pg_pos; i++)
                {
                    if (col <= PageColumns)
                    {
                        col++;
                    }
                    else
                    {
                        col = 1;
                        row++;
                        u_list.UpdList[u_list.Index].UpdateColEnd = EndOfRow;
                        NewUpd(u_list, rows_needed, row, (short)1, (short)1);
                        rows_needed--;
                    }
                    page.Page[i].Clear();
                }
                u_list.UpdList[u_list.Index].UpdateColEnd = sts.Column;
                break;

            case 2:
                start = Offset((short)1, (short)1); /* start of page */
                /* erasing whole screen */
                for (row = 1; row <= PageRows; row++)
                {
                    for (col = 1; col <= PageColumns; col++)
                    {
                        page.Page[col].Clear();
                    }
                    NewUpd(u_list, PageRows - row + 2, row, (short)1, EndOfRow);
                }
                NewUpd(u_list, 1, sts.Row, sts.Column, sts.Column);
                break;

            default:
                break;
        }
        m_Parser.State = INIT_STATE;
        return OK;
    }

    /// <summary>
    ///	Erases some or all of a line, depending on the parameter value.  The update list
    /// must already have an entry for the current line in it so only the columns need to
    /// be changed.  Entering more characters would cause upd_end to be incremented so a
    /// new entry in update list must be made.
    /// </summary>
    ///
    /// <returns><see cref="ERROR"/> if number of updates exceeds maximum, otherwise <see cref="OK"/></returns>
    private short AnsiEraseLine(PageType page, ListType u_list)
    {
        short i;
        short pg_pos;
        short end, start;
        StatusType sts = page.Status;

        /* default and range check. */
        m_Parser.Params[0] = (m_Parser.ParamCount == 0) ? (short)0 : m_Parser.Params[0];

        switch (m_Parser.Params[0])
        {
            case 0:
                end = Offset(sts.Row, PageColumns); /* end of current line */
                pg_pos = Offset(sts.Row, sts.Column);
                /* erasing from current position to end of line */
                for (i = pg_pos; i <= end; i++)
                {
                    page.Page[i].Clear();
                }
                u_list.UpdList[u_list.Index].UpdateColEnd = EndOfRow;
                break;

            case 1:
                start = Offset(sts.Row, (short)1); /* start of current line */
                pg_pos = Offset(sts.Row, sts.Column);
                /* erasing from start of line to current position */
                u_list.UpdList[u_list.Index].UpdateColBegin = 1;

                for (i = start; i <= pg_pos; i++)
                {
                    page.Page[i].Clear();
                }
                break;

            case 2:
                start = Offset(sts.Row, (short)1); /* start of current line */
                end = Offset(sts.Row, PageColumns); /* end of current line */
                /* erasing whole line */

                for (i = start; i <= end; i++)
                {
                    page.Page[i].Clear();
                }
                u_list.UpdList[u_list.Index].UpdateColBegin = 1;
                u_list.UpdList[u_list.Index].UpdateColEnd = EndOfRow;
                break;

            default:
                m_Parser.State = INIT_STATE;
                return OK;
        }

        /* Entering more characters would cause upd_end to be incremented so we must make
         * a new entry in update list */
        NewUpd(u_list, 1, sts.Row, sts.Column, sts.Column);
        m_Parser.State = INIT_STATE;
        return OK;
    }

    /// <summary>
	/// Enters the escape state.
	/// </summary>
    ///
    /// <returns><see cref="OK"/></returns>
    private short AnsiEsc()
    {
        m_Parser.State = ESCAPE_STATE;
        m_Parser.SpecialEsc = (char)0;
        return OK;
    }

    /// <summary>
    ///	Insert character -- Inserts params[0] blanks in the line, starting at current
    ///	position, working to the right, pushing the rest of the line right, defaulting to
    ///	one and moving one if params[0] = 0.  This operation is not affected by wrap; if
    ///	characters are pushed past the end of the line, they are destroyed.  The spaces
    ///	inserted will only fit on one line.
    ///	</summary>
    ///
    /// <returns><see cref="ERROR"/> if number of updates exceeds maximum, otherwise <see cref="OK"/></returns>
    private short AnsiInsertChar(PageType page, ListType u_list)
    {
        short num_cols, startpg, endpg;
        int i;
        StatusType sts = page.Status;

        /* Storing original cursor position. */
        startpg = Offset(sts.Row, sts.Column);

        num_cols = (m_Parser.ParamCount == 0) ? (short)1 : m_Parser.Params[0];
        if (num_cols < 1)
            num_cols = 1; /* default and range check */

        /* Check to see if number of cols to be inserted is longer than the space available on the line. */
        if (num_cols + sts.Column > PageColumns)
        {
            /* set num_cols to the number of columns left on line */
            num_cols = (short)(PageColumns - sts.Column + 1);
        }

        endpg = Offset(sts.Row, PageColumns);

        /* create update list */
        NewUpd(u_list, 2, sts.Row, sts.Column, (short)(PageColumns + 1));

        /* Moving characters to the right */
        for (i = endpg; i >= (startpg + num_cols); --i)
        {
            page.Page[i].CopyFrom(page.Page[i - num_cols]);
        }

        /* Fill in inserted positions with spaces. */
        for (i = startpg; i < (startpg + num_cols); ++i)
        {
            page.Page[i].Clear();
        }

        NewUpd(u_list, 1, sts.Row, sts.Column, sts.Column);
        m_Parser.State = INIT_STATE;
        return OK;
    }

    /// <summary>
    ///	Inserts params[0] erased lines, starting at the current line, working downward,
    ///	defaulting to one, and inserting one if params[0] = 0. The current and following
    ///	lines are moved down params[0] rows, with the last params[0] lines being lost.
    ///	</summary>
    ///
    /// <returns><see cref="ERROR"/> if number of updates exceeds maximum, otherwise <see cref="OK"/></returns>
    private short AnsiInsertLines(PageType page, ListType u_list)
    {
        short num_lines;

        StatusType sts = page.Status;

        /* if row outside scrolling region, ignore message */
        if (sts.Row < page.ScrollTop || sts.Row > page.ScrollBottom)
        {
            m_Parser.State = INIT_STATE;
            return (OK);
        }

        num_lines = (m_Parser.ParamCount == 0) ? (short)1 : m_Parser.Params[0];
        if (num_lines < 1)
            num_lines = 1; /* default and range check. */

        if (num_lines + sts.Row > page.ScrollBottom)
        {
            /* too many lines, truncate */
            num_lines = (short)(page.ScrollBottom - sts.Row);
        }

        /* move to the start of the current line. */
        sts.Column = 1;
        m_Parser.Wrap = false;
        if (AnsiScroll(page, u_list, num_lines, S_DOWN, sts.Row) == ERROR)
        {
            m_Parser.State = INIT_STATE;
            return ERROR;
        }

        NewUpd(u_list, 1, sts.Row, (short)1, (short)1);
        m_Parser.State = INIT_STATE;
        return OK;
    }

    /// <summary>
    ///	This routine is called whenever the cursor is moved down and scrolling should
    ///	occur if necessary.  This routine is used for processing new lines, but NOT for
    ///	the CUD sequence.
    ///	</summary>
    ///
    /// <returns><see cref="ERROR"/> if update list would overflow, else <see cref="OK"/></returns>
    private short AnsiLineFeed(PageType page, ListType u_list)
    {
        StatusType sts = page.Status;

        /* when moving the cursor down if we move down to bottom of scrolling region,
         * every move beyond should scroll the screen. */
        if (sts.Row == page.ScrollBottom)
        {
            if (AnsiScroll(page, u_list, (short)1, S_UP, page.ScrollTop) == ERROR)
            {
                m_Parser.State = INIT_STATE;
                return ERROR;
            }

            /* force new entry after scrolling */
            NewUpd(u_list, 1, sts.Row, sts.Column, sts.Column);
            m_Parser.CrState = LF_STATE;
            m_Parser.State = INIT_STATE;
            m_Parser.Wrap = false;
            return OK;
        }

        if (sts.Row == PageRows)
        {
            /* cursor not in scrolling region, ignore lf */
            m_Parser.State = INIT_STATE;
            return OK;
        }

        /* just move the cursor down */
        sts.Row++;

        if (m_Parser.CrState != CR_STATE)
        {
            NewUpd(u_list, 1, sts.Row, sts.Column, sts.Column);
        }
        else
        {
            u_list.UpdList[u_list.Index].Row = sts.Row;
        }
        m_Parser.CrState = LF_STATE;
        m_Parser.State = INIT_STATE;
        m_Parser.Wrap = false;
        return OK;
    }

    /// <summary>
    ///	Makes new line, i.e., combination of move to start of line and move to next lower
    ///	line, scrolling if necessary.
    ///	</summary>
    ///
    /// <returns><see cref="ERROR"/> if update list would overflow, else <see cref="OK"/></returns>
    private short AnsiNewLine(PageType page, ListType u_list)
    {
        page.Status.Column = (short)1; /* do carriage return */
        if (AnsiLineFeed(page, u_list) < 0)
            return ERROR;

        m_Parser.Wrap = false;
        m_Parser.State = INIT_STATE;

        return OK;
    }

    /// <summary>
    ///	Ignores the character at this position.  Resets the state to the initial state.
    ///	</summary>
    ///
    /// <returns><see cref="OK"/></returns>
    private short AnsiNop()
    {
        m_Parser.State = INIT_STATE;
        return OK;
    }

    private short AnsiParam(char ch, PageType page)
    {
        if (m_Parser.ParamCount == 0) /* test for 1st digit of 1st parm */
            m_Parser.ParamCount++;

        if (m_Parser.ParamCount > MAXCSIPARAMS)
            return OK;

        m_Parser.Params[m_Parser.ParamCount - 1] *= 10;
        m_Parser.Params[m_Parser.ParamCount - 1] += (short)(ch - '0');
        return OK;
    }

    /// <summary>
    ///	Will write all characters pointed to by text into the page image until a
    ///	non-printable (excluding null) character is reached.
    ///	</summary>
    ///
    ///	<remarks>
    ///	Wrap works in the following way: If wrap is on and a character is written in the
    ///	80th column, the column or the ch_ptr are not incremented, but parse_ptr->wrap is
    ///	set.  If another character is written, ch_ptr is set properly (check scrolling and
    ///	make a new entry in the update list) before the character is written to the 1
    ///	column of the next line.
    ///	</remarks>
    ///
    /// <returns>Number of characters processed.</returns>
    private short AnsiPrintChars(char b, PageType page, Stream data, ListType u_list)
    {
        int txt_left = m_Parser.TextLength;
        StatusType status = page.Status;
        UpdateType upd_ptr = u_list.UpdList[u_list.Index];
        int chptrindex = Offset(status.Row, status.Column);
        CharType chptr = page.Page[chptrindex];
        m_Parser.CrState = INIT_STATE;

        long markedPosition = 0;

        while ((IsPrint(b)) || (b == '\0'))
        {
            if (b != '\0')
            {
                /* if 80th col already written, but wrap is on */
                if (m_Parser.Wrap == true)
                {
                    /* adjust pointer BEFORE writing character.
                     * If cursor is at end of scroll reg, scroll the screen. */
                    if (status.Row == page.ScrollBottom)
                    {
                        if (AnsiScroll(page, u_list, (short)1, S_UP, page.ScrollTop) == ERROR)
                        {
                            return (short)(m_Parser.TextLength - txt_left);
                        }
                        status.Column = 1;
                        NewUpd(u_list, 1, status.Row, (short)1, (short)1);
                    }
                    else if (status.Row != PageRows)
                    {
                        if (u_list.Index >= u_list.UpdList.Length - 1)
                        {
                            /* if list full, return what we have so far */
                            return (short)(m_Parser.TextLength - txt_left);
                        }
                        /* just do line feed */
                        status.Column = 1;
                        status.Row++;
                        NewUpd(u_list, 1, status.Row, (short)1, (short)1);
                    }
                    /* else cursor not in scrolling region but at end of page do nothing. */

                    chptrindex = Offset(status.Row, status.Column);
                    chptr = page.Page[chptrindex];
                    upd_ptr = u_list.UpdList[u_list.Index];
                    m_Parser.Wrap = false;
                }

                /* for speed copy address of character in page to a */
                chptr.Ch = b;

                /* put the attribute in page */
                chptr.Attr = status.CurAttribute;
                chptr.ColorAttrib = status.ColorAttribute;
                chptr.ColorFadeAttr = status.ColorFadingAttribute;
                chptr.FadeAttr = status.Fading;

                /* use graphics set currently selected */
                if (status.GR_Set == 1)
                    chptr.GS = status.G1_Set;
                else
                    chptr.GS = status.G0_Set;

                /* check to see if at end of line */
                if (status.Column >= PageColumns)
                {
                    /* yes, set updt_end, check wrap flag */
                    upd_ptr.UpdateColEnd = EndOfRow;
                    if (status.IsWrapOn)
                    {
                        /* mark that we have written the 80th char but that the cursor is
                         * still at col 80 */
                        m_Parser.Wrap = true;
                    }
                }
                else
                {
                    status.Column++;
                    upd_ptr.UpdateColEnd += 1;
                    chptrindex++;
                    chptr = page.Page[chptrindex];
                }
            } /* end if not null character */

            markedPosition = data.Position;
            b = (char)(data.ReadByte() & 0x7F);

            /* if txt_left goes to zero, end of text has been reached */
            if ((--txt_left == 0) || (b < 0))
            {
                break;
            }
        }

        /* return count on number of chars processed */
        data.Position = markedPosition;
        return (short)(m_Parser.TextLength - txt_left + 1);
    }

    /// <summary>
    /// Moves cursor to position in line params[0] and row params[1].  Defaults to 'home'.
    /// </summary>
    ///
    /// <returns><see cref="ERROR"/> if update list would overflow, else <see cref="OK"/></returns>
    private short AnsiPositionCursor(PageType page, ListType u_list)
    {
        short row;
        short col;

        StatusType sts = page.Status;
        switch (m_Parser.ParamCount)
        {
            case 0:
                row = 1;
                col = 1;
                break;
            case 1:
                row = m_Parser.Params[0];
                col = 1;
                break;
            case 2:
                row = m_Parser.Params[0];
                col = m_Parser.Params[1];
                break;
            default:
                m_Parser.State = INIT_STATE;
                return OK;
        }

        if (row == 0)
            row = 1;

        if (col == 0)
            col = 1;

        if ((row > PageRows) || (col > PageColumns))
        {
            m_Parser.State = INIT_STATE;
            return OK; /* ignore bad positioning */
        }

        sts.Row = row;
        sts.Column = col;

        NewUpd(u_list, 1, sts.Row, sts.Column, sts.Column);
        m_Parser.State = INIT_STATE;
        m_Parser.Wrap = false;

        return OK;
    }

    /// <summary>
    /// Reverse index - move the cursor to the preceding line without changing the column
    /// position, scrolling if necessary.
    /// </summary>
    ///
    /// <returns><see cref="ERROR"/> if update list would overflow, else <see cref="OK"/></returns>
    private short AnsiReverseIndex(PageType page, ListType u_list)
    {
        StatusType sts = page.Status;

        /* when moving the cursor up if we move up to top of scrolling region, every move
         * beyond should scroll the screen. */
        if (sts.Row == page.ScrollTop)
        {
            if (AnsiScroll(page, u_list, (short)1, S_DOWN, sts.Row) == ERROR)
            {
                m_Parser.State = INIT_STATE;
                return ERROR;
            }

            /* force new entry after scrolling */
            NewUpd(u_list, 1, sts.Row, sts.Column, sts.Column);
            m_Parser.State = INIT_STATE;
            m_Parser.Wrap = false;
            return OK;
        }

        if (sts.Row == 1)
        {
            /* cursor not in scrolling region, ignore ri */
            m_Parser.State = INIT_STATE;
            m_Parser.Wrap = false;
            return OK;
        }

        /* just move cursor up */
        sts.Row--;

        NewUpd(u_list, 1, sts.Row, sts.Column, sts.Column);
        m_Parser.State = INIT_STATE;
        m_Parser.Wrap = false;
        return OK;
    }

    /// <summary>
    /// Reset Mode - Alters bit-records of modes currently set.  Test p_esc to check for
    /// private modes.
    /// </summary>
    ///
    /// <returns><see cref="OK"/></returns>
    private short AnsiResetMode(PageType page)
    {
        StatusType sts = page.Status;

        if (m_Parser.SpecialEsc != 0)
        {
            switch (m_Parser.SpecialEsc)
            {
                case '?':
                    if (m_Parser.Params[0] == 7)
                    {
                        sts.IsWrapOn = false;
                    }
                    break;
                default:
                    break;
            }
        }
        else
        {
            switch (m_Parser.Params[0])
            {
                case 0x07:
                    sts.VerticalEditMode = false;
                    break;
                case 0x10:
                    sts.HorizontalEditMode = false;
                    break;
                default:
                    break;
            }
        }
        m_Parser.State = INIT_STATE;
        return OK;
    }

    /// <summary>
    /// General scroll routine for scrolling any direction for any scrolling region.
    /// </summary>
    ///
    /// <returns><see cref="ERROR"/> if number of updates exceeds maximum, otherwise <see cref="OK"/></returns>
    private short AnsiScroll(PageType page, ListType u_list, short num, short dir, short row)
    {
        short num_chars;
        short start;
        short scroll;
        short end;
        short j;

        if (num == 0)
            return OK;

        /* make sure we have enough room in update list before modifying page image */
        if (page.ScrollBottom - page.ScrollTop + u_list.Index + 2 >= u_list.UpdList.Length)
        {
            return ERROR;
        }

        switch (dir)
        {
            case S_DOWN:
                {
                    start = Offset(row, (short)1);
                    num_chars = (short)(num * PageColumns);
                    scroll = (short)(start + num_chars - 1);

                    /* Move everything down, starting from the end of the region */
                    j = Offset(page.ScrollBottom, EndOfRow);
                    for (; j > scroll; --j)
                    {
                        page.Page[j].CopyFrom(page.Page[j - num_chars]);
                    }

                    /* Fill inserted positions with blanks. */
                    for (j = start; j <= scroll; ++j)
                    {
                        page.Page[j].Clear();
                    }

                    /* modify the update list if only a part of the scrolling region is scrolled,
                     * or if the scrolling region has never been scrolled before */
                    if (row != page.ScrollTop || m_Parser.Scrolled == false)
                    {
                        /* modify update list */
                        for (j = row; j <= page.ScrollBottom; j++)
                        {
                            NewUpd(u_list, 1, j, (short)1, EndOfRow);
                        }
                    }

                    if (row == page.ScrollTop)
                        m_Parser.Scrolled = true;
                    break;
                }
            case S_UP:
                {
                    /* if requested to scroll more lines than are in the scrolling region,
                     * reduce to size of scrolling region */
                    if (row + num > page.ScrollBottom)
                        num = (short)(page.ScrollBottom - row + 1);

                    num_chars = (short)(num * PageColumns);
                    scroll = Offset(page.ScrollBottom, EndOfRow);

                    /* Move everything up */
                    j = Offset(row, (short)1);
                    end = (short)(scroll - num_chars);

                    for (; j <= end; j++)
                    {
                        page.Page[j] = page.Page[j + num_chars];
                    }

                    /* Fill inserted positions with blanks. */
                    j = (short)(scroll - num_chars);
                    for (; j < scroll; j++)
                    {
                        page.Page[j].Clear();
                    }

                    /* modify the update list if only a part of the scrolling region is scrolled,
                     * or if the scrolling region has never been scrolled before */
                    if (row != page.ScrollTop || m_Parser.Scrolled == false)
                    {
                        /* modify update list */
                        for (j = row; j <= page.ScrollBottom; j++)
                        {
                            NewUpd(u_list, 1, j, (short)1, EndOfRow);
                        }
                    }
                    if (row == page.ScrollTop)
                        m_Parser.Scrolled = true;
                    break;
                }
            case S_LEFT:
            case S_RIGHT:
            default:
                break;
        }
        return OK;
    }

    /// <returns><see cref="ERROR"/> if number of updates exceeds maximum, otherwise <see cref="OK"/></returns>
    private short AnsiSd(PageType page, ListType u_list)
    {
        short num_lines;
        StatusType sts = page.Status;
        /* if row outside scrolling region, ignore message */
        if (sts.Row < page.ScrollTop || sts.Row > page.ScrollBottom)
        {
            m_Parser.State = INIT_STATE;
            return OK;
        }

        num_lines = (m_Parser.ParamCount == 0) ? (short)1 : m_Parser.Params[0];
        if (num_lines < 1)
            num_lines = 1; /* default and range check. */

        if (AnsiScroll(page, u_list, num_lines, S_DOWN, sts.Row) == ERROR)
        {
            m_Parser.State = INIT_STATE;
            return ERROR;
        }

        NewUpd(u_list, 1, sts.Row, sts.Column, sts.Column);
        m_Parser.State = INIT_STATE;
        return OK;
    }

    /// <summary>
    ///	Changes current character attribute following the sequence of integer instructions
    ///	stored in the array params[] by taking exclusive or of externally defined
    ///	constants with the normal attribute.  If Rich private fading sequence is used, set
    ///	fading attribute accordingly.
    ///	</summary>
    ///
    /// <returns><see cref="OK"/></returns>
    private short AnsiSetGraphicsRendition(PageType page, ListType u_list)
    {
        short i; /* number of parameters */
        StatusType sts = page.Status;

        if (m_Parser.SpecialEsc == '?')
        {
            /* DEC private -- ignore */
            m_Parser.State = INIT_STATE;
            return OK;
        }
        if (m_Parser.SpecialEsc == '>')
        {
            /* Rich private fading attr */
            sts.Fade = true;
        }
        else
        {
            sts.Fade = false;
        }

        if (m_Parser.ParamCount == 0)
        {
            /* default to set current */
            sts.SetPlain(); /* attribute byte to normal */
            sts.SetColor(CharType.MONO);
        }

        for (i = 0; i < m_Parser.ParamCount; i++)
        {
            if (m_Parser.Params[i] >= 30 && m_Parser.Params[i] < 38)
            {
                if (sts.Fade)
                {
                    sts.ColorFadingAttribute &= '\xf0';
                    sts.ColorFadingAttribute += (char)(m_Parser.Params[i] - 30);
                }
                else
                {
                    sts.ColorAttribute &= '\xf0';
                    sts.ColorAttribute += (char)(m_Parser.Params[i] - 30);
                }
                continue;
            }
            if (m_Parser.Params[i] >= 40 && m_Parser.Params[i] < 48)
            {
                if (sts.Fade)
                {
                    sts.ColorFadingAttribute &= '\x0f';
                    sts.ColorFadingAttribute += (char)((m_Parser.Params[i] - 40) << 4);
                }
                else
                {
                    sts.ColorAttribute &= '\x0f';
                    sts.ColorAttribute += (char)((m_Parser.Params[i] - 40) << 4);
                }
                continue;
            }
            switch (m_Parser.Params[i])
            {
                case 0:
                    sts.SetColor(CharType.MONO);
                    sts.SetAttr(CharType.PLAIN);
                    break;
                case 1:
                    sts.OnAttr(CharType.BRIGHT);
                    break;
                case 2:
                    sts.OnAttr(CharType.DIM);
                    break;
                case 4:
                    sts.OnAttr(CharType.UNDLN);
                    break;
                case 5:
                    sts.OnAttr(CharType.BLINK);
                    break;
                case 7:
                    sts.OnAttr(CharType.REVVID);
                    break;
                case 22:
                    sts.OffAttr((char)(CharType.DIM + CharType.BRIGHT));
                    break;
                case 24:
                    sts.OffAttr(CharType.UNDLN);
                    break;
                case 25:
                    sts.OffAttr(CharType.BLINK);
                    break;
                case 27:
                    sts.OffAttr(CharType.REVVID);
                    break;
                default:
                    break;
            }
        }
        m_Parser.State = INIT_STATE;
        return OK;
    }

    /// <summary>
    /// Shift in -- make G0 character set the current set.
    /// </summary>
    ///
    /// <returns><see cref="OK"/></returns>
    private short AnsiShiftIn(PageType page, ListType u_list)
    {
        page.Status.GR_Set = (char)0;
        m_Parser.State = INIT_STATE;
        return OK;
    }

    /// <summary>
    /// Alters bit-records of modes currently set.  Must test p_esc to check for private
    /// modes.
    /// </summary>
    ///
    /// <returns><see cref="OK"/></returns>
    private short AnsiSetModes(PageType page)
    {
        StatusType sts = page.Status;

        if (m_Parser.SpecialEsc != 0)
        {
            switch (m_Parser.SpecialEsc)
            {
                case '?':
                    if (m_Parser.Params[0] == 7)
                    {
                        sts.IsWrapOn = true;
                    }
                    break;
                default:
                    break;
            }
        }
        else
        {
            switch (m_Parser.Params[0])
            {
                case 0x07:
                    sts.VerticalEditMode = true;
                    break;
                case 0x10:
                    sts.HorizontalEditMode = true;
                    break;
                default:
                    break;
            }
        }
        m_Parser.State = INIT_STATE;
        return (OK);
    }

    /// <summary>
    /// Shift out -- make G1 character set the current set.
    /// </summary>
    ///
    /// <returns><see cref="OK"/></returns>
    private short AnsiShiftOut(PageType page, ListType u_list)
    {
        page.Status.GR_Set = '\x1';
        m_Parser.State = INIT_STATE;
        return OK;
    }

    /// <returns>OK.</returns>
    private short AnsiSpesc(char ch)
    {
        m_Parser.SpecialEsc = ch;
        return OK;
    }

    /// <summary>
    /// Enters the G0 state for selecting G0 graphics set and enters the G1 state for
    /// selecting G1 graphics set.
    /// </summary>
    ///
    /// <returns><see cref="OK"/></returns>
    private short AnsiStateG0()
    {
        m_Parser.State = G0_STATE;
        return OK;
    }

    private short AnsiStateG1()
    {
        m_Parser.State = G1_STATE;
        return OK;
    }

    /// <returns><see cref="ERROR"/> if number of updates exceeds maximum, otherwise <see cref="OK"/>.</returns>
    private short AnsiSu(PageType page, ListType u_list)
    {
        short num_lines;
        StatusType sts = page.Status;

        /* if row outside scrolling region, ignore message */
        if (sts.Row < page.ScrollTop || sts.Row > page.ScrollBottom)
        {
            m_Parser.State = INIT_STATE;
            return OK;
        }

        num_lines = (m_Parser.ParamCount == 0) ? (short)1 : m_Parser.Params[0];
        if (num_lines < 1)
            num_lines = 1; /* default and range check. */

        if (AnsiScroll(page, u_list, num_lines, S_UP, sts.Row) == ERROR)
        {
            m_Parser.State = INIT_STATE;
            return ERROR;
        }
        NewUpd(u_list, 1, sts.Row, sts.Column, sts.Column);
        m_Parser.State = INIT_STATE;
        return OK;
    }

    /// <summary>
    /// Makes new line, i.e., combination of move to start of line and move to next lower
    /// line, scrolling if necessary.
    /// </summary>
    ///
    /// <returns><see cref="ERROR"/> if update list would overflow, else <see cref="OK"/></returns>
    private short AnsiTab(PageType page, ListType u_list)
    {
        if (page.Status.Column <= TAB_TABLE.Length)
        {
            page.Status.Column = TAB_TABLE[page.Status.Column - 1];
            NewUpd(u_list, 1, page.Status.Row, page.Status.Column, page.Status.Column);
        }
        m_Parser.State = INIT_STATE;
        return OK;
    }

    private short DoDecode(char ch, PageType page, Stream data, ListType u_list)
    {
        short ret = 0;

        switch (m_Parser.State)
        {
            case INIT_STATE:
                ret = DoDecodeInit(ch, page, data, u_list);
                break;
            case ESCAPE_STATE:
                ret = DoDecodeEscape(ch, page, u_list);
                break;
            case CSI_STATE:
                ret = DoDecodeCsi(ch, page, u_list);
                break;
            case G0_STATE:
                ret = DoDecodeG0(ch, page);
                break;
            case G1_STATE:
                ret = DoDecodeG1(ch, page);
                break;
            case CHARACTER_SIZE_STATE:
                ret = DoDecodeCharacterSize(ch, page, data, u_list);
                break;
            default: /* should never be in this state */
                break;
        }
        return ret;
    }

    private short DoDecodeCharacterSize(char ch, PageType page, Stream data, ListType u_list)
    {
        short ret;
        if (ch == 27)
            ret = AnsiEsc();
        else
            ret = AnsiNop();
        return ret;
    }

    private short DoDecodeCsi(char ch, PageType page, ListType u_list)
    {
        short ret = 0;

        if (ch < 48)
        {
            ret = AnsiNop();
        }
        else if (ch < 58)
        {
            ret = AnsiParam(ch, page);
        }
        else
        {
            switch ((byte)ch)
            {
                case 59:
                    ret = AnsiChparm();
                    break;
                case 61:
                case 62:
                case 63:
                    ret = AnsiSpesc(ch);
                    break;
                case 64:
                    ret = AnsiInsertChar(page, u_list);
                    break;
                case 65:
                    ret = AnsiCuu(page, u_list);
                    break;
                case 66:
                    ret = AnsiCud(page, u_list);
                    break;
                case 67:
                    ret = AnsiCuf(page, u_list);
                    break;
                case 68:
                    ret = AnsiCub(page, u_list);
                    break;
                case 72:
                    ret = AnsiPositionCursor(page, u_list);
                    break;
                case 74:
                    ret = AnsiEraseDisplay(page, u_list);
                    break;
                case 75:
                    ret = AnsiEraseLine(page, u_list);
                    break;
                case 76:
                    ret = AnsiInsertLines(page, u_list);
                    break;
                case 77:
                    ret = AnsiDeleteLines(page, u_list);
                    break;
                case 80:
                    ret = AnsiDch(page, u_list);
                    break;
                case 83:
                    ret = AnsiSu(page, u_list);
                    break;
                case 84:
                    ret = AnsiSd(page, u_list);
                    break;
                case 90:
                    ret = AnsiDbP(page, u_list);
                    break;
                case 102:
                    ret = AnsiPositionCursor(page, u_list);
                    break;
                case 104:
                    ret = AnsiSetModes(page);
                    break;
                case 108:
                    ret = AnsiResetMode(page);
                    break;
                case 109:
                    ret = AnsiSetGraphicsRendition(page, u_list);
                    break;
                case 112:
                    ret = QaReset(page, u_list);
                    break;
                case 114:
                    ret = AnsiDecstbm(page, u_list);
                    break;
                default:
                    ret = AnsiNop();
                    break;
            }
        }

        return ret;
    }

    private short DoDecodeEscape(char ch, PageType page, ListType u_list)
    {
        short ret = 0;
        switch ((byte)ch)
        {
            case 27:
                ret = AnsiEsc();
                break;
            case 40:
                ret = AnsiStateG0();
                break;
            case 41:
                ret = AnsiStateG1();
                break;
            case 55:
                ret = AnsiDecsc(page, u_list);
                break;
            case 56:
                ret = AnsiDecrc(page, u_list);
                break;
            case 68:
                ret = AnsiLineFeed(page, u_list);
                break;
            case 69:
                ret = AnsiNewLine(page, u_list);
                break;
            case 77:
                ret = AnsiReverseIndex(page, u_list);
                break;
            case 91:
                ret = AnsiCsi();
                break;
            case 99:
                ret = QaReset(page, u_list);
                break;
            default:
                ret = AnsiNop();
                break;
        }

        return ret;
    }

    private short DoDecodeG0(char ch, PageType page)
    {
        short ret = 0;
        if ((ch < 48) || (ch == 127))
        {
            ret = AnsiNop();
        }
        else
        {
            ret = AnsiDesignateG0(ch, page);
        }
        return ret;
    }

    private short DoDecodeG1(char ch, PageType page)
    {
        short ret = 0;
        if ((ch < 48) || (ch == 127))
        {
            ret = AnsiNop();
        }
        else
        {
            ret = AnsiDesignateG1(ch, page);
        }
        return ret;
    }

    private short DoDecodeInit(char ch, PageType page, Stream data, ListType u_list)
    {
        short ret;

        switch ((byte)ch)
        {
            case 0:
            case 1:
            case 2:
            case 3:
            case 4:
            case 5:
            case 6:
            case 7:
                ret = AnsiNop();
                break;
            case 8:
                ret = AnsiCub(page, u_list);
                break;
            case 9:
                ret = AnsiTab(page, u_list);
                break;
            case 10:
                ret = AnsiLineFeed(page, u_list);
                break;
            case 11:
            case 12:
                ret = AnsiNop();
                break;
            case 13:
                ret = AnsiCret(page, u_list);
                break;
            case 14:
                ret = AnsiShiftOut(page, u_list); /* shift out */
                break;
            case 15:
                ret = AnsiShiftIn(page, u_list); /* shift in */
                break;
            case 16:
            case 17:
            case 18:
            case 19:
            case 20:
            case 21:
            case 22:
            case 23:
            case 24:
            case 25:
            case 26:
                ret = AnsiNop();
                break;
            case 27:
                ret = AnsiEsc();
                break;
            case 28:
            case 29:
            case 30:
            case 31:
            case 127:
                ret = AnsiNop();
                break;
            default:
                ret = AnsiPrintChars(ch, page, data, u_list);
                break;
        }
        return ret;
    }

    private bool IsPrint(char ch)
    {
        return ((32 <= ch) && (ch < 127));
    }

    private short NewUpd(ListType upd, int req, short row, short beg, short end)
    {
        if (upd.Index >= upd.UpdList.Length - (req))
            return ERROR;

        upd.Index++;
        upd.UpdList[upd.Index].Row = row;
        upd.UpdList[upd.Index].UpdateColBegin = beg;
        upd.UpdList[upd.Index].UpdateColEnd = end;
        return OK;
    }

    private short Offset(short row, short col)
    {
        return (short)((row - 1) * PageColumns + (col - 1));
    }
    #endregion

    #region Public methods

    /// <summary>
    /// Decodes an ANSI string.
    /// </summary>
    ///
    /// <remarks>
    /// <para>
    /// Decoding an ANSI string is accomplished using a finite state machine.
    /// Each of the ANSI escape sequences is parsed according to the state into
    /// which the previous characters have the machine.</para>
    ///
    /// <para>
    /// As the sequences are recognized, the page image is modified.</para>
    ///
    /// <para>
    /// An update list is maintained to make identification of changed regions
    /// easier.  Each entry has the row, the start column and the end column of
    /// the portion of the screen that was modified.  Entries where the start
    /// column and the end column are equal indicate that no update has occured
    /// at this location.</para>
    ///
    /// <para>
    /// The update list is maintained according to the following rules:</para>
    ///
    /// <list type="number">
    /// <item>
    /// Printable Characters written to the page increment the end col.</item>
    ///
    /// <item>
    /// When the end of the line is reached, if line wrap is on, a new entry in
    /// the list is created, the row being the next row, the start and end
    /// column being set to 1.  If wrap is off, the character overwrites the one
    /// currently at the 80th column and the end column field in the update list
    /// is not modified.</item>
    ///
    /// <item>
    /// When scrolling occurs, an entry is made in the update list for each row
    /// that was modified.</item>
    ///
    /// <item>
    /// Any sequence or character that moves the cursor position causes a new
    /// entry to be made in the list.  This includes cursor positioning
    /// sequences, backspace, linefeed, etc.</item>
    ///
    /// <item>
    /// Erase commands cause new entries to be made for each row that is
    /// affected.</item>
    /// </list>
    /// </remarks>
    ///
    /// <param name="page">base Page image</param>
    /// <param name="inStream">stream with the ANSI encoded update stream to be
    ///   decoded.</param>
    /// <param name="u_list">page updates created based on the decoded stream.</param>
    ///
    /// <returns>
    /// Returns the number of characters in text it has successfully parsed.  It
    /// will not return a length which would cause an escape sequence to be
    /// broken.  QaDecode may be called as many times as necessary until all the
    /// text is parsed.  If QaDecode has completed, the length which was passed
    /// to it is returned.
    /// </returns>
    public int QaDecode(PageType page, Stream inStream, ListType u_list)
    {
        int start_len;
        m_Parser = new ParserType();
        char ch;
        short ret;

        /* initialize index and the update list */
        u_list.Index = 0;
        u_list.UpdList[u_list.Index].Row = page.Status.Row;
        u_list.UpdList[u_list.Index].UpdateColBegin = page.Status.Column;
        u_list.UpdList[u_list.Index].UpdateColEnd = page.Status.Column;

        int len = (int)inStream.Length;

        m_Parser.TextLength = (short)len;

        start_len = len;

        int datum = inStream.ReadByte();
        while (datum > 0)
        {

            ch = (char)(datum & 0x7f); /* mask to 7 bit ASCII */
            if ((ret = DoDecode(ch, page, inStream, u_list)) == ERROR)
            {
                if (m_Parser.TextLength == start_len)
                {
                    return (start_len);
                }
                return (start_len - m_Parser.TextLength);
            }

            if (ret == 0)
            {
                /* really an error from _pch */
                return (start_len - m_Parser.TextLength);
            }

            len -= ret;

            /* if a complete escape sequence has been parsed, save the length and reset initial conditions. */
            if (m_Parser.State == INIT_STATE)
            {
                m_Parser.TextLength = (short)len;
                m_Parser.ParamCount = 0;
                m_Parser.Params[0] = 0;
                m_Parser.SpecialEsc = '\x0';
            }
            datum = inStream.ReadByte();
        }
        return (start_len);
    }

    /// <summary>
    /// Returns current end of row.
    /// </summary>
    /// <returns>current end of row</returns>
    /// <seealso cref="QaSetEndOfRow(short)"/>
    public short QaEndOfRow()
    {
        return EndOfRow;
    }

    /// <summary>
    /// Columns count in this page.
    /// </summary>
    /// <returns>columns count</returns>
    /// <seealso cref="QaSetColumns(short)"/>
    public short QaPageColumns()
    {
        return PageColumns;
    }

    /// <summary>
    /// Rows count in this page.
    /// </summary>
    /// <returns>rows count</returns>
    /// <seealso cref="QaSetRows(short)"/>
    public short QaPageRows()
    {
        return PageRows;
    }

    /// <summary>
    /// Qa reset.
    /// </summary>
    ///
    /// <param name="page">the page</param>
    /// <param name="u_list">the u list</param>
    ///
    /// <returns><see cref="OK"/></returns>
    public short QaReset(PageType page, ListType u_list)
    {
        short i;
        StatusType sts = page.Status;
        StatusType save = page.Save;

        for (i = 0; i < PageRows * PageColumns; i++)
            page.Page[i].Clear();

        sts.Row = save.Row = 1;
        sts.Column = save.Column = 1;
        sts.CurAttribute = save.CurAttribute = '\x0';
        sts.ColorAttribute = save.ColorAttribute = '\xFF';
        sts.ColorFadingAttribute = save.ColorFadingAttribute = '\xFF';
        sts.Fading = save.Fading = '\x0';
        sts.GR_Set = save.GR_Set = '\x0';
        sts.IsWrapOn = save.IsWrapOn = false;

        page.ScrollTop = 1;
        page.ScrollBottom = (short)(PageColumns - 1);

        /* if the screen is reset, any other updates will be erased so overwrite them in
         * the update list */
        u_list.Index = 0;
        for (i = 1; i <= PageRows; i++)
            NewUpd(u_list, PageRows - i + 1, i, (short)1, (short)(PageColumns + 1));

        /* set up new entry so that subsequent characters get added at the home position */
        NewUpd(u_list, 1, sts.Row, sts.Column, sts.Column);
        if (m_Parser != null)
            m_Parser.State = INIT_STATE;
        return OK;
    }

    /// <summary>
    /// Returns scroll region top row number.
    /// </summary>
    /// <returns>scroll region top row number</returns>
    /// <seealso cref="QaSetScrollBottom(short)"/>
    public short QaScrollBottom()
    {
        return ScrollBot;
    }

    /// <summary>
    /// Set the column count on the page.
    /// </summary>
    /// <param name="cl">new column count value</param>
    /// <seealso cref="QaPageColumns()"/>
    public void QaSetColumns(short cl)
    {
        PageColumns = cl;
    }

    /// <summary>
    /// Where the row ends.
    /// </summary>
    /// <param name="rw">new end of row value</param>
    public void QaSetEndOfRow(short rw)
    {
        EndOfRow = rw;
    }

    /// <summary>
    /// Rows count on a page.
    /// </summary>
    /// <param name="rw">new rows count</param>
    /// <seealso cref="QaPageRows()"/>
    public void QaSetRows(short rw)
    {
        PageRows = rw;
    }

    /// <summary>
    /// Set top row of the scroll region.
    /// </summary>
    /// <param name="cl">new scroll region row number</param>
    /// <seealso cref="QaScrollBottom()"/>
    public void QaSetScrollBottom(short cl)
    {
        ScrollBot = cl;
    }

    /// <summary>
    /// Returns a deep copy of this AnsiDecoder object.
    /// </summary>
    /// <returns>deep copy of this AnsiDecoder object.</returns>
    Object ICloneable.Clone() => Clone();

    /// <summary>
    /// Returns a deep copy of this AnsiDecoder object.
    /// </summary>
    /// <returns>deep copy of this AnsiDecoder object.</returns>
    public AnsiDecoder Clone()
    {
        AnsiDecoder decoder = new AnsiDecoder();
        decoder.PageColumns = PageColumns;
        decoder.PageRows = PageRows;
        decoder.ScrollBot = ScrollBot;
        decoder.EndOfRow = EndOfRow;
        decoder.m_Parser = m_Parser.Clone();
        return decoder;
    }

    #endregion
}
