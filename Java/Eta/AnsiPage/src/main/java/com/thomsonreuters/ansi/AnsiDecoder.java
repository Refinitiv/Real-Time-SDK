package com.thomsonreuters.ansi;

import java.io.*;

public class AnsiDecoder implements Ansi, Cloneable
{
    ParserType _parser;
    final static short OK = 1;
    final static short ERROR = -1;
    final static short S_UP = 0;
    final static short S_DOWN = 1;
    final static short S_LEFT = 2;
    final static short S_RIGHT = 3;

    /* escape sequence number and out sequnce table offsets */
    final static int DBH_TP  =   1;   /* double high top */
    final static int DBH_BT  =   2;   /* double high bottom */
    final static int DBHW_TP =   3;   /* double high/wide top */
    final static int DBHW_BT =   4;

    /* double high/wide bottom */
    final static int DB_OFF  =   5;   /* double off */
    final static int DBWD    =   6;   /* double wide single high */
    final static int DB_MIN  =   1;
    final static int DB_MAX  =   6;

    final static short TAB_TABLE[] =
	{
	 9,  9,  9,  9,  9,  9,  9,  9,		/* 1 - 8 */
	17, 17, 17, 17, 17, 17, 17, 17,		/* 9 - 16 */
	25, 25, 25, 25, 25, 25, 25, 25,		/* 17 - 24 */
	33, 33, 33, 33, 33, 33, 33, 33,		/* 25 - 32 */
	41, 41, 41, 41, 41, 41, 41, 41,		/* 33 - 40 */
	49, 49, 49, 49, 49, 49, 49, 49,		/* 41 - 48 */
	57, 57, 57, 57, 57, 57, 57, 57,		/* 49 - 56 */
	65, 65, 65, 65, 65, 65, 65, 65,		/* 57 - 64 */
	73, 73, 73, 73, 73, 73, 73, 73,		/* 65 - 72 */
	80, 80, 80, 80, 80, 80, 80, 80};	/* 73 - 80 */

    short PageColumns = 80;
    short PageRows = 25;
    short ScrollBot = 24;
    short EndOfRow = 81;

    final static int INIT_STATE = 0;
    final static int ESCAPE_STATE = 1;
    final static int CSI_STATE = 2;
    final static int G0_STATE = 3;
    final static int G1_STATE = 4;
    final static int CHARACTER_SIZE_STATE = 5;

    final static int CR_STATE = 20;
    final static int LF_STATE = 21;

    final static int MAXCSIPARAMS = 20;	/* max params in CSI sequence */


    public Object clone()
    {
        AnsiDecoder decoder = new AnsiDecoder();
        decoder.PageColumns = PageColumns;
        decoder.PageRows = PageRows;
        decoder.ScrollBot = ScrollBot;
        decoder.EndOfRow = EndOfRow;
        decoder._parser = (ParserType)_parser.clone();
        return decoder;
    }

    /* ************************************************************************
     *
     * ROUTINE NAME	_ansi_chparm()
     *
     * DESCRIPTION	This routine is called when a parameter delimiter is
     *		parsed.  Increment the number of parameters, checking
     *		to make sure we don't go out of bounds.
     *
     * RETURNS	OK.
     *
     **************************************************************************/
    short ansi_chparm()
    {
        if (_parser.param_cnt < MAXCSIPARAMS)
        {
            _parser.param_cnt++;
            _parser.params[_parser.param_cnt - 1] = 0;
        }
        return OK;
    }
	
    /* ************************************************************************
     *
     * ROUTINE NAME	_ansi_cret()
     *
     * DESCRIPTION	Moves cursor to the start of the current line.
     *
     * RETURNS	ERROR if update list would overflow, else OK
     *
     **************************************************************************/
    short ansi_cret(PageType page, ListType u_list)
    {
        page.status.col = 1;
        if (_parser.cr_state != LF_STATE)
        {
            newUpd(u_list, 1, page.status.row, (short)1, (short)1);
        }
        else
        {
            u_list.upd_list[u_list.index].upd_beg = 1;
            u_list.upd_list[u_list.index].upd_end = 1;
        }
        _parser.cr_state = CR_STATE;
        _parser.state = INIT_STATE;
        _parser.wrap = false;
        return OK;
    }

    /* ************************************************************************
     *
     * ROUTINE NAME	_ansi_csi()
     *
     * DESCRIPTION	Enters the CSI state, i.e., we have parsed an escape
     *		character followed by a '['.  The next thing to follow
     *		may be one or many parameters.
     *
     * RETURNS	OK.
     *
     **************************************************************************/
    short ansi_csi()
    {
        _parser.state = CSI_STATE;
        return OK;
    }
	
    /* ************************************************************************
     *
     * ROUTINE NAME	_ansi_cub()
     *
     * DESCRIPTION	Moves cursor backward param_cnt positions in the
     *		line, stopping at the beginning, defaulting to
     *	        one, and moving one if param_cnt = 0.
     *
     * RETURNS	ERROR if update list would overflow, else OK
     *
     **************************************************************************/
    short ansi_cub(PageType page, ListType u_list)
    {
        short num_col;
        num_col = (_parser.param_cnt == 0) ? 1 : _parser.params[0];

        if (num_col < 1)
            num_col = 1; /* default and range check */

        /* Moving cursor back, being sure to go no further than the start of the line. */
        if (page.status.col - num_col <= 0)
            page.status.col = 1;
        else
            page.status.col -= num_col;

        newUpd(u_list, 1, page.status.row, page.status.col, page.status.col);
        _parser.state = INIT_STATE;
        _parser.wrap = false;
        return OK;
    }
        
    /* ************************************************************************
     *
     * ROUTINE NAME	_cud()
     *
     * DESCRIPTION	This routine is called whenever the cursor is moved down
     *		but scrolling should not occur.  This routine
     *		is used for processing the CUD sequence, but NOT for new lines.
     *
     * RETURNS	ERROR if update list would overflow, else OK
     *
     **************************************************************************/
    short ansi_cud(PageType page, ListType u_list)
    {
        short num_rows;
        StatusType sts = page.status;

        num_rows = (_parser.param_cnt == 0) ? 1 : _parser.params[0];

        if (sts.row <= page.scroll_bot)
        {
            if (sts.row + num_rows > page.scroll_bot)
                sts.row = page.scroll_bot;
            else
                sts.row += num_rows;
        }
        else
        {
            /* cursor must be between scroll bottom and end of page */
            if (sts.row + num_rows > PageRows)
                sts.row = PageRows;
            else
                sts.row += num_rows;
        }

        newUpd(u_list, 1, sts.row, sts.col, sts.col);
        _parser.state = INIT_STATE;
        _parser.wrap = false;
        return OK;
    }
	
    /* ************************************************************************
     *
     * ROUTINE NAME	_ansi_cuf()
     *
     * DESCRIPTION	Moves cursor forward param_cnt positions in the
     *		line, stopping at the line end, defaulting to one,
     *		and moving one if param_cnt = 0.
     *
     * RETURNS	ERROR if update list would overflow, else OK
     *
     **************************************************************************/
    short ansi_cuf(PageType page, ListType u_list)
    {
        short numcol;

        if (_parser.special_esc != 0)
        {
            switch (_parser.special_esc)
            {
                case '=':
                    break; /* invisible cursor currently ignored */
                default:
                    break;
            }
        }
        else
        {
            numcol = (_parser.param_cnt == 0) ? 1 : _parser.params[0];

            /* Moving cursor forward, making sure to stop at the end of the line */
            if (page.status.col + numcol > PageColumns)
                page.status.col = PageColumns;
            else
                page.status.col += numcol;
            newUpd(u_list, 1, page.status.row, page.status.col, page.status.col);
        }
        _parser.state = INIT_STATE;
        return OK;
    }
	
    /* ************************************************************************
     *
     * ROUTINE NAME	_ansi_cuu()
     *
     * DESCRIPTION	Moves cursor up params[0] positions, stopping
     *				at the top line, defaulting to one, and moving
     *				one if params[0] = 0.
     *
     * RETURNS	ERROR if update list would overflow, else OK
     *
     **************************************************************************/
    short ansi_cuu(PageType page, ListType u_list)
    {
        short num_rows;
        StatusType sts = page.status;

        num_rows = (_parser.param_cnt == 0) ? 1 : _parser.params[0];

        if (sts.row >= page.scroll_top)
        {
            if (sts.row - num_rows < page.scroll_top)
                sts.row = page.scroll_top;
            else
                sts.row -= num_rows;
        }
        else
        {
            /* cursor must be between scroll top and top of page */
            if (sts.row - num_rows < 1)
                sts.row = 1;
            else
                sts.row -= num_rows;
        }

        newUpd(u_list, 1, sts.row, sts.col, sts.col);
        _parser.state = INIT_STATE;
        _parser.wrap = false;
        return OK;
    }
	
    /* ************************************************************************
     *
     * ROUTINE NAME	_ansi_db_p()
     *
     * DESCRIPTION	(Set double high/wide private sequence).
     *		ESC[>#Z
     *
     * RETURNS	OK
     *
     **************************************************************************/
    short ansi_db_p(PageType page, ListType u_list)
    {
        /* Not supported */
        _parser.state = INIT_STATE;
        return OK;
    }

    /* ************************************************************************
     *
     * ROUTINE NAME	_ansi_dch()
     *
     * DESCRIPTION	Deletes params[0] chars, starting at the current
     *		position and working forward. Defaults to one char
     *		if params[0] = 0. Remaining chars are moved up to
     *		the current position.  The vacated character positions
     *		at the end of the line are erased.
     *
     * RETURNS	ERROR if number of updates exceeds maximum, otherwise OK.
     *
     **************************************************************************/
    short ansi_dch(PageType page, ListType u_list)
    {
        short num_chars, start, end;
        int i;
        StatusType sts = page.status;

        num_chars = (_parser.param_cnt == 0) ? 1 : _parser.params[0];
        if (num_chars < 1)
            num_chars = 1; /* default and range check */

        /* Start deleting characters, starting at the current curs. pos. */
        start = offset(sts.row, sts.col);
        end = offset(sts.row, PageColumns);
        newUpd(u_list, 2, sts.row, sts.col, (short)(PageColumns + 1));

        for (i = (start + num_chars); i <= end; ++i)
        {
            page.page[(i - num_chars)].copyFrom(page.page[i]);
        }

        /* Fill in the end with blanks. */
        for (i = end; i >= (end - num_chars); --i)
        {
            page.page[i].clear();
        }
        newUpd(u_list, 1, sts.row, sts.col, sts.col);
        _parser.state = INIT_STATE;
        return (OK);
    }
	
    /* ************************************************************************
     *
     * ROUTINE NAME	_ansi_decdhl()
     *
     * DESCRIPTION	DEC private sequence for double width
     *
     * RETURNS	OK.
     *
     **************************************************************************/
    short ansi_decdhl(PageType page, ListType u_list)
    {
        /* double height is not supported */
        return OK;
    }
	
    /* ************************************************************************
     *
     * ROUTINE NAME	_ansi_decdwl()
     *
     * DESCRIPTION	DEC private sequence for double width
     *
     * RETURNS	OK.
     *
     **************************************************************************/
    short ansi_decdwl(PageType page, ListType u_list)
    {
        /* double width is not supported */
        return OK;
    }

    /* ************************************************************************
     * 
     * ROUTINE NAME _ansi_decrc()
     * 
     * DESCRIPTION DEC private sequence to save cursor.
     * 
     * RETURNS OK.
     * 
     **************************************************************************/
    short ansi_decrc(PageType page, ListType u_list)
    {
        page.status = page.save;
        StatusType sts = page.status;
        newUpd(u_list, 1, sts.row, sts.col, sts.col);
        _parser.state = INIT_STATE;
        return OK;
    }
	
    /* ************************************************************************
     * 
     * ROUTINE NAME _ansi_decsc()
     * 
     * DESCRIPTION DEC private sequence to save cursor.
     * 
     * RETURNS OK.
     * 
     **************************************************************************/
    short ansi_decsc(PageType page, ListType u_list)
    {
        page.save = page.status;
        _parser.state = INIT_STATE;
        return OK;
    }
	
    /* ************************************************************************
     *
     * ROUTINE NAME	ansi_decstbm()
     *
     * DESCRIPTION	DEC private sequence to select top and bottoms margins
     *		for scrolling.
     *
     * RETURNS	ERROR if update list would overflow, else OK
     *
     **************************************************************************/
    short ansi_decstbm(PageType page, ListType u_list)
    {
        short top;
        short bottom;

        StatusType sts = page.status;
        switch (_parser.param_cnt)
        {
            case 0:
                top = 1;
                bottom = ScrollBot;
                break;
            case 1:
                top = _parser.params[0];
                bottom = ScrollBot;
                break;
            case 2:
                top = _parser.params[0];
                bottom = _parser.params[1];
                break;
            default:
                _parser.state = INIT_STATE;
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
            page.scroll_top = top;
            page.scroll_bot = bottom;
            sts.row = 1;
            sts.col = 1;
            /* home the cursor when scrolling region changed */
            newUpd(u_list, 1, (short)1, (short)1, (short)1);
            _parser.wrap = false;
        }

        _parser.state = INIT_STATE;
        _parser.scrolled = false;
        return OK;
    }
        
    /* ************************************************************************
     *
     * ROUTINE NAME	_ansi_decswl()
     *
     * DESCRIPTION	DEC private sequence for double width
     *
     * RETURNS	OK.
     *
     **************************************************************************/
    short ansi_decswl(PageType page, ListType u_list)
    {
        /* double width is not supported */
        return OK;
    }
	
    /* ************************************************************************
     *
     * ROUTINE NAME	_ansi_dG0()
     *
     * DESCRIPTION	Designate graphics set -- Installs character into G0
     *				graphics set if it is valid.  If not, graphics set is
     *				not changed.
     *
     * RETURNS	OK.
     *
     **************************************************************************/
    short ansi_dG0(char ch, PageType page)
    {
        if (isprint(ch))
        {
            page.status.G0_set = ch;
        }
        _parser.state = INIT_STATE;
        return OK;
    }
	
    /* ************************************************************************
     *
     * ROUTINE NAME	_ansi_dG1()
     *
     * DESCRIPTION	Designate graphics set -- Installs character into G1
     *		graphics set if it is valid.  If not, graphics set is not changed.
     *
     * RETURNS	OK.
     **************************************************************************/
    short ansi_dG1(char ch, PageType page)
    {
        if (isprint(ch))
        {
            page.status.G1_set = ch;
        }
        _parser.state = INIT_STATE;
        return OK;
    }
    
    /* ************************************************************************
     *
     * ROUTINE NAME	_ansi_dl()
     *
     * DESCRIPTION	Deletes params[0] lines, starting at the current
     *				line and working downward.  Defaults to one line
     *				and deletes one line if params[0] = 0. Remaining
     *				lines are moved up to the current line position.
     *
     * RETURNS	Nothing.
     *
     **************************************************************************/
    short ansi_dl(PageType page, ListType u_list)
    {
        short num_lines;
        StatusType sts = page.status;

        if (sts.row < page.scroll_top || sts.row > page.scroll_bot)
        {
            _parser.state = INIT_STATE;
            return OK;
        }

        num_lines = (_parser.param_cnt == 0) ? 1 : _parser.params[0];
        if (num_lines < 1) /* default and range check */
            num_lines = 1;

        if (num_lines + sts.row > page.scroll_bot)
            num_lines = (short)(page.scroll_bot - sts.row);

        if (ansi_scroll(page, u_list, num_lines, S_UP, sts.row) == ERROR)
        {
            _parser.state = INIT_STATE;
            return ERROR;
        }
        sts.col = 1;
        newUpd(u_list, 1, sts.row, (short)1, (short)1);
        _parser.state = INIT_STATE;
        _parser.wrap = false;
        return OK;
    }
        
    /* ************************************************************************
     *
     * ROUTINE NAME	ansi_ed()
     *
     * DESCRIPTION	Erases some or all of the display, depending on the
     *				parameter values.
     *
     * RETURNS	ERROR if number of updates exceeds maximum, otherwise OK.
     *
     **************************************************************************/
    short ansi_ed(PageType page, ListType u_list)
    {
        short i, pg_pos;
        short col;
        short row;
        short end, start;
        short rows_needed; /* num update entries need to make */
        StatusType sts = page.status;

        /* default and range check. */
        _parser.params[0] = (_parser.param_cnt == 0) ? 0 : _parser.params[0];

        col = sts.col;
        row = sts.row;
        switch (_parser.params[0])
        {
            case 0:
                /* erasing from current position to end of page */
                end = offset(PageRows, PageColumns); /* end of page */
                pg_pos = offset(sts.row, sts.col);
                rows_needed = (short)(PageRows - sts.row + 1); /* +1 for last NEW_UPD */
                /* mark current update to end of this row */
                u_list.upd_list[u_list.index].upd_end = EndOfRow;

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
                        newUpd(u_list, rows_needed, row, (short)1, EndOfRow);
                        rows_needed--;
                    }
                    page.page[i].clear();
                }
                newUpd(u_list, 1, sts.row, sts.col, sts.col);
                break;

            case 1:
                /* erasing from start of page to current position */
                start = offset((short)1, (short)1); /* start of page */
                pg_pos = offset(sts.row, sts.col);
                row = 1;
                /* overwrite current update, since it will be included in the for loop */
                u_list.upd_list[u_list.index].row = 1;
                u_list.upd_list[u_list.index].upd_beg = 1;
                rows_needed = sts.row;

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
                        u_list.upd_list[u_list.index].upd_end = EndOfRow;
                        newUpd(u_list, rows_needed, row, (short)1, (short)1);
                        rows_needed--;
                    }
                    page.page[i].clear();
                }
                u_list.upd_list[u_list.index].upd_end = sts.col;
                break;

            case 2:
                start = offset((short)1, (short)1); /* start of page */
                /* erasing whole screen */
                for (row = 1; row <= PageRows; row++)
                {
                    for (col = 1; col <= PageColumns; col++)
                    {
                        page.page[col].clear();
                    }
                    newUpd(u_list, PageRows - row + 2, row, (short)1, EndOfRow);
                }
                newUpd(u_list, 1, sts.row, sts.col, sts.col);
                break;

            default:
                break;
        }
        _parser.state = INIT_STATE;
        return OK;
    }
        
    /* ************************************************************************
     *
     * ROUTINE NAME	_ansi_el()
     *
     * DESCRIPTION	Erases some or all of a line, depending on the parameter
     *				value.  The update list must already have an entry for
     *				the current line in it so only the columns need to be
     *				changed.  Entering more characters would cause upd_end to be
     *				incremented so a new entry in update list  must be made.
     *
     * RETURNS	ERROR if number of updates exceeds maximum, otherwise OK.
     *
     **************************************************************************/
    short ansi_el(PageType page, ListType u_list)
    {
        short i;
        short pg_pos;
        short end, start;
        StatusType sts = page.status;

        /* default and range check. */
        _parser.params[0] = (_parser.param_cnt == 0) ? 0 : _parser.params[0];

        switch (_parser.params[0])
        {
            case 0:
                end = offset(sts.row, PageColumns); /* end of current line */
                pg_pos = offset(sts.row, sts.col);
                /* erasing from current position to end of line */
                for (i = pg_pos; i <= end; i++)
                {
                    page.page[i].clear();
                }
                u_list.upd_list[u_list.index].upd_end = EndOfRow;
                break;

            case 1:
                start = offset(sts.row, (short)1); /* start of current line */
                pg_pos = offset(sts.row, sts.col);
                /* erasing from start of line to current position */
                u_list.upd_list[u_list.index].upd_beg = 1;

                for (i = start; i <= pg_pos; i++)
                {
                    page.page[i].clear();
                }
                break;

            case 2:
                start = offset(sts.row, (short)1); /* start of current line */
                end = offset(sts.row, PageColumns); /* end of current line */
                /* erasing whole line */

                for (i = start; i <= end; i++)
                {
                    page.page[i].clear();
                }
                u_list.upd_list[u_list.index].upd_beg = 1;
                u_list.upd_list[u_list.index].upd_end = EndOfRow;
                break;

            default:
                _parser.state = INIT_STATE;
                return OK;
        }

        /* Entering more characters would cause upd_end to be incremented so we must make a new entry in update list */
        newUpd(u_list, 1, sts.row, sts.col, sts.col);
        _parser.state = INIT_STATE;
        return OK;
    }
        
    /* ************************************************************************
     *
     * ROUTINE NAME	_ansi_esc()
     *
     * DESCRIPTION	Enters the escape state.
     *
     * RETURNS	OK.
     *
     **************************************************************************/
    short ansi_esc()
    {
        _parser.state = ESCAPE_STATE;
        _parser.special_esc = 0;
        return OK;
    }
        
    /* ************************************************************************
     *
     * ROUTINE NAME	ansi_ich()
     *
     * DESCRIPTION	Insert character -- Inserts params[0] blanks in the
     *		line, starting at current position, working to
     *		the right, pushing the rest of the line right, defaulting
     *		to one and moving one if params[0] = 0.
     *		This operation is not affected by wrap; if characters are
     *		pushed past the end of the line, they are destroyed.
     *		The spaces inserted will only fit on one line.
     *
     * RETURNS	ERROR if number of updates exceeds maximum, otherwise OK.
     *
     **************************************************************************/
    short ansi_ich(PageType page, ListType u_list)
    {
        short num_cols, startpg, endpg;
        int i;
        StatusType sts = page.status;

        /* Storing original cursor position. */
        startpg = offset(sts.row, sts.col);

        num_cols = (_parser.param_cnt == 0) ? 1 : _parser.params[0];
        if (num_cols < 1)
            num_cols = 1; /* default and range check */

        /* Check to see if number of cols to be inserted is longer than the space available on the line. */
        if (num_cols + sts.col > PageColumns)
        {
            /* set num_cols to the number of columns left on line */
            num_cols = (short)(PageColumns - sts.col + 1);
        }

        endpg = offset(sts.row, PageColumns);

        /* create update list */
        newUpd(u_list, 2, sts.row, sts.col, (short)(PageColumns + 1));

        /* Moving characters to the right */
        for (i = endpg; i >= (startpg + num_cols); --i)
        {
            page.page[i].copyFrom(page.page[i - num_cols]);
        }

        /* Fill in inserted positions with spaces. */
        for (i = startpg; i < (startpg + num_cols); ++i)
        {
            page.page[i].clear();
        }

        newUpd(u_list, 1, sts.row, sts.col, sts.col);
        _parser.state = INIT_STATE;
        return OK;
    }
        
    /* ************************************************************************
     *
     * ROUTINE NAME	ansi_il()
     *
     * DESCRIPTION	Inserts params[0] erased lines, starting at the current
     *				line, working downward, defaulting to one, and inserting
     *				one if params[0] = 0. The current and following lines
     *				are moved down params[0] rows, with the last params[0]
     *				lines being lost.
     *
     * RETURNS	ERROR if number of updates exceeds maximum, otherwise OK.
     *
     **************************************************************************/
    short ansi_il(PageType page, ListType u_list)
    {
        short num_lines;

        StatusType sts = page.status;

        /* if row outside scrolling region, ignore message */
        if (sts.row < page.scroll_top || sts.row > page.scroll_bot)
        {
            _parser.state = INIT_STATE;
            return (OK);
        }

        num_lines = (_parser.param_cnt == 0) ? 1 : _parser.params[0];
        if (num_lines < 1)
            num_lines = 1; /* default and range check. */

        if (num_lines + sts.row > page.scroll_bot)
        {
            /* too many lines, truncate */
            num_lines = (short)(page.scroll_bot - sts.row);
        }

        /* move to the start of the current line. */
        sts.col = 1;
        _parser.wrap = false;
        if (ansi_scroll(page, u_list, num_lines, S_DOWN, sts.row) == ERROR)
        {
            _parser.state = INIT_STATE;
            return ERROR;
        }

        newUpd(u_list, 1, sts.row, (short)1, (short)1);
        _parser.state = INIT_STATE;
        return OK;
    }
        
    /* ************************************************************************
     *
     * ROUTINE NAME	_ansi_lf()
     *
     * DESCRIPTION	This routine is called whenever the cursor is moved down
     *		and scrolling should occur if necessary.  This routine
     *		is used for processing new lines, but NOT for the CUD sequence.
     *
     * RETURNS	ERROR if update list would overflow, else OK
     *
     **************************************************************************/
    short ansi_lf(PageType page, ListType u_list)
    {
        StatusType sts = page.status;

        /* when moving the cursor down if we move down to bottom of scrolling region, every move beyond should scroll the screen. */
        if (sts.row == page.scroll_bot)
        {
            if (ansi_scroll(page, u_list, (short)1, S_UP, page.scroll_top) == ERROR)
            {
                _parser.state = INIT_STATE;
                return ERROR;
            }

            /* force new entry after scrolling */
            newUpd(u_list, 1, sts.row, sts.col, sts.col);
            _parser.cr_state = LF_STATE;
            _parser.state = INIT_STATE;
            _parser.wrap = false;
            return OK;
        }

        if (sts.row == PageRows)
        {
            /* cursor not in scrolling region, ignore lf */
            _parser.state = INIT_STATE;
            return OK;
        }

        /* just move the cursor down */
        sts.row++;

        if (_parser.cr_state != CR_STATE)
        {
            newUpd(u_list, 1, sts.row, sts.col, sts.col);
        }
        else
        {
            u_list.upd_list[u_list.index].row = sts.row;
        }
        _parser.cr_state = LF_STATE;
        _parser.state = INIT_STATE;
        _parser.wrap = false;
        return OK;
    }
        
    /* ************************************************************************
     *
     * ROUTINE NAME	_ansi_nel()
     *
     * DESCRIPTION	Makes new line, i.e., combination of move to start of line
     *				and  move to next lower line, scrolling if necessary.
     *
     * RETURNS	ERROR if update list would overflow, else OK
     *
     **************************************************************************/
    short ansi_nel(PageType page, ListType u_list)
    {
        page.status.col = (short)1; /* do carriage return */
        if (ansi_lf(page, u_list) < 0)
            return ERROR;

        _parser.wrap = false;
        _parser.state = INIT_STATE;

        return OK;
    }
        
    /* ************************************************************************
     *
     * ROUTINE NAME	_ansi_nop()
     *
     * DESCRIPTION	Ignores the character at this position.  Resets the
     *		state to the initial state.
     *
     * RETURNS	OK.
     *
     **************************************************************************/
    short ansi_nop()
    {
        _parser.state = INIT_STATE;
        return OK;
    }
        
    short ansi_param(char ch, PageType page)
    {
        if (_parser.param_cnt == 0) /* test for 1st digit of 1st parm */
            _parser.param_cnt++;

        if (_parser.param_cnt > MAXCSIPARAMS)
            return OK;

        _parser.params[_parser.param_cnt - 1] *= 10;
        _parser.params[_parser.param_cnt - 1] += ch - '0';
        return OK;
    }
        
    /* ************************************************************************
     *
     * ROUTINE NAME	_ansi_pch()
     *
     * DESCRIPTION	Will write all characters pointed to by text into the
     *		page image until a non-printable (excluding null)
     *		character is reached.
     *
     *		Wrap works in the following way: If wrap is on and a character
     *		is written in the 80th column, the column or the ch_ptr are
     *		not incremented, but parse_ptr->wrap is set.  If another
     *		character is written, ch_ptr is set properly (check scrolling
     *		and make a new entry in the update list) before the
     *		character is written to the 1 column of the next line.
     *
     * RETURNS	Number of characters processed.
     *
     **************************************************************************/
    short ansi_pch(char b, PageType page, ByteArrayInputStream data, ListType u_list)
    {
        int txt_left = _parser.txt_lngth;
        StatusType status = page.status;
        UpdateType upd_ptr = u_list.upd_list[u_list.index];
        int chptrindex = offset(status.row, status.col);
        CharType chptr = page.page[chptrindex];
        _parser.cr_state = INIT_STATE;

        while ((isprint(b)) || (b == '\0'))
        {
            if (b != '\0')
            {
                /* if 80th col already written, but wrap is on */
                if (_parser.wrap == true)
                {
                    /* adjust pointer BEFORE writing character.
                     * If cursor is at end of scroll reg, scroll the screen. */
                    if (status.row == page.scroll_bot)
                    {
                        if (ansi_scroll(page, u_list, (short)1, S_UP, page.scroll_top) == ERROR)
                        {
                            return (short)(_parser.txt_lngth - txt_left);
                        }
                        status.col = 1;
                        newUpd(u_list, 1, status.row, (short)1, (short)1);
                    }
                    else if (status.row != PageRows)
                    {
                        if (u_list.index >= u_list.upd_list.length - 1)
                        {
                            /* if list full, return what we have so far */
                            return (short)(_parser.txt_lngth - txt_left);
                        }
                        /* just do line feed */
                        status.col = 1;
                        status.row++;
                        newUpd(u_list, 1, status.row, (short)1, (short)1);
                    }
                    /* else cursor not in scrolling region but at end of page do nothing. */

                    chptrindex = offset(status.row, status.col);
                    chptr = page.page[chptrindex];
                    upd_ptr = u_list.upd_list[u_list.index];
                    _parser.wrap = false;
                }

                /* for speed copy address of character in page to a */
                chptr.ch = b;

                /* put the attribute in page */
                chptr.attr = status.cur_attr;
                chptr.c_attr = status.c_attr;
                chptr.c_fade_attr = status.c_fade_attr;
                chptr.fade_attr = status.fading;

                /* use graphics set currently selected */
                if (status.gr_set == 1)
                    chptr.gs = status.G1_set;
                else
                    chptr.gs = status.G0_set;

                /* check to see if at end of line */
                if (status.col >= PageColumns)
                {
                    /* yes, set updt_end, check wrap flag */
                    upd_ptr.upd_end = EndOfRow;
                    if (status.wrap_on)
                    {
                        /* mark that we have written the 80th char but that the cursor is still at col 80 */
                        _parser.wrap = true;
                    }
                }
                else
                {
                    status.col++;
                    upd_ptr.upd_end += 1;
                    chptrindex++;
                    chptr = page.page[chptrindex];
                }
            } /* end if not null character */

            data.mark(_parser.txt_lngth);
            b = (char)(data.read() & 0x7F);

            /* if txt_left goes to zero, end of text has been reached */
            if ((--txt_left == 0) || (b < 0))
            {
                break;
            }
        }

        /* return count on number of chars processed */
        data.reset();
        return (short)(_parser.txt_lngth - txt_left + 1);
    }
        
    /* ************************************************************************
     *
     * ROUTINE NAME	ansi_poscur()
     *
     * DESCRIPTION	Moves cursor to position in line params[0] and
     *		row params[1].  Defaults to 'home'.
     *
     * RETURNS	ERROR if update list would overflow, else OK
     *
     **************************************************************************/
    short ansi_poscur(PageType page, ListType u_list)
    {
        short row;
        short col;

        StatusType sts = page.status;
        switch (_parser.param_cnt)
        {
            case 0:
                row = 1;
                col = 1;
                break;
            case 1:
                row = _parser.params[0];
                col = 1;
                break;
            case 2:
                row = _parser.params[0];
                col = _parser.params[1];
                break;
            default:
                _parser.state = INIT_STATE;
                return OK;
        }

        if (row == 0)
            row = 1;

        if (col == 0)
            col = 1;

        if ((row > PageRows) || (col > PageColumns))
        {
            _parser.state = INIT_STATE;
            return OK; /* ignore bad positioning */
        }

        sts.row = row;
        sts.col = col;

        newUpd(u_list, 1, sts.row, sts.col, sts.col);
        _parser.state = INIT_STATE;
        _parser.wrap = false;

        return OK;
    }
        
    /* ************************************************************************
     *
     * ROUTINE NAME	_ansi_ri()
     *
     * DESCRIPTION	Reverse index - move the cursor to the preceding line
     *				without changing the column position, scrolling if necessary.
     *
     * RETURNS	ERROR if update list would overflow, else OK
     *
     **************************************************************************/
    short ansi_ri(PageType page, ListType u_list)
    {
        StatusType sts = page.status;

        /* when moving the cursor up if we move up to top of scrolling region, every move beyond should scroll the screen. */
        if (sts.row == page.scroll_top)
        {
            if (ansi_scroll(page, u_list, (short)1, S_DOWN, sts.row) == ERROR)
            {
                _parser.state = INIT_STATE;
                return ERROR;
            }

            /* force new entry after scrolling */
            newUpd(u_list, 1, sts.row, sts.col, sts.col);
            _parser.state = INIT_STATE;
            _parser.wrap = false;
            return OK;
        }

        if (sts.row == 1)
        {
            /* cursor not in scrolling region, ignore ri */
            _parser.state = INIT_STATE;
            _parser.wrap = false;
            return OK;
        }

        /* just move cursor up */
        sts.row--;

        newUpd(u_list, 1, sts.row, sts.col, sts.col);
        _parser.state = INIT_STATE;
        _parser.wrap = false;
        return OK;
    }
        
    /* ************************************************************************
     *
     * ROUTINE NAME	_ansi_rm()
     *
     * DESCRIPTION	Reset Mode - Alters bit-records of modes currently set.
     *		Test p_esc to check for private modes.
     *
     * RETURNS	OK
     *
     **************************************************************************/
    short ansi_rm(PageType page)
    {
        StatusType sts = page.status;

        if (_parser.special_esc != 0)
        {
            switch (_parser.special_esc)
            {
                case '?':
                    if (_parser.params[0] == 7)
                    {
                        sts.wrap_on = false;
                    }
                    break;
                default:
                    break;
            }
        }
        else
        {
            switch (_parser.params[0])
            {
                case 0x07:
                    sts.vem = false;
                    break;
                case 0x10:
                    sts.hem = false;
                    break;
                default:
                    break;
            }
        }
        _parser.state = INIT_STATE;
        return OK;
    }
        
    /* ************************************************************************
     *
     * ROUTINE NAME	_ansi_scroll()
     *
     * DESCRIPTION	General scroll routine for scrolling any direction for
     *		any scrolling region.
     *
     * RETURNS	ERROR if number of updates exceeds maximum, otherwise OK.
     *
     *************************************************************************/
    short ansi_scroll(PageType page, ListType u_list, short num, short dir, short row)
    {
        short num_chars;
        short start;
        short scroll;
        short end;
        short j;

        if (num == 0)
            return OK;

        /* make sure we have enough room in update list before modifying page image */
        if (page.scroll_bot - page.scroll_top + u_list.index + 2 >= u_list.upd_list.length)
        {
            return ERROR;
        }

        switch (dir)
        {
            case S_DOWN:
            {
                start = offset(row, (short)1);
                num_chars = (short)(num * PageColumns);
                scroll = (short)(start + num_chars - 1);

                /* Move everything down, starting from the end of the region */
                j = offset(page.scroll_bot, EndOfRow);
                for (; j > scroll; --j)
                {
                    page.page[j].copyFrom(page.page[j - num_chars]);
                }

                /* Fill inserted positions with blanks. */
                for (j = start; j <= scroll; ++j)
                {
                    page.page[j].clear();
                }

                /* modify the update list if only a part of the scrolling region is scrolled,
                 * or if the scrolling region has never been scrolled before */
                if (row != page.scroll_top || _parser.scrolled == false)
                {
                    /* modify update list */
                    for (j = row; j <= page.scroll_bot; j++)
                    {
                        newUpd(u_list, 1, j, (short)1, EndOfRow);
                    }
                }

                if (row == page.scroll_top)
                    _parser.scrolled = true;
                break;
            }
            case S_UP:
            {
                /* if requested to scroll more lines than are in the scrolling region,
                 * reduce to size of scrolling region */
                if (row + num > page.scroll_bot)
                    num = (short)(page.scroll_bot - row + 1);

                num_chars = (short)(num * PageColumns);
                scroll = offset(page.scroll_bot, EndOfRow);

                /* Move everything up */
                j = offset(row, (short)1);
                end = (short)(scroll - num_chars);

                for (; j <= end; j++)
                {
                    page.page[j] = page.page[j + num_chars];
                }

                /* Fill inserted positions with blanks. */
                j = (short)(scroll - num_chars);
                for (; j < scroll; j++)
                {
                    page.page[j].clear();
                }

                /* modify the update list if only a part of the scrolling region is scrolled,
                 * or if the scrolling region has never been scrolled before */
                if (row != page.scroll_top || _parser.scrolled == false)
                {
                    /* modify update list */
                    for (j = row; j <= page.scroll_bot; j++)
                    {
                        newUpd(u_list, 1, j, (short)1, EndOfRow);
                    }
                }
                if (row == page.scroll_top)
                    _parser.scrolled = true;
                break;
            }
            case S_LEFT:
            case S_RIGHT:
            default:
                break;
        }
        return OK;
    }
        
    /* ************************************************************************
     *
     * ROUTINE NAME	_ansi_sd()
     *
     * DESCRIPTION
     *
     * RETURNS	ERROR if number of updates exceeds maximum, otherwise OK.
     *
     **************************************************************************/
    short ansi_sd(PageType page, ListType u_list)
    {
        short num_lines;
        StatusType sts = page.status;
        /* if row outside scrolling region, ignore message */
        if (sts.row < page.scroll_top || sts.row > page.scroll_bot)
        {
            _parser.state = INIT_STATE;
            return OK;
        }

        num_lines = (_parser.param_cnt == 0) ? 1 : _parser.params[0];
        if (num_lines < 1)
            num_lines = 1; /* default and range check. */

        if (ansi_scroll(page, u_list, num_lines, S_DOWN, sts.row) == ERROR)
        {
            _parser.state = INIT_STATE;
            return ERROR;
        }

        newUpd(u_list, 1, sts.row, sts.col, sts.col);
        _parser.state = INIT_STATE;
        return OK;
    }

    /* ************************************************************************
     *
     * ROUTINE NAME	_ansi_sgr()
     *
     * DESCRIPTION	(Set graphics rendition).  Changes current character
     *		attribute following the sequence of integer instructions
     *		stored in the array params[] by taking exclusive or of
     *		externally defined constants with the normal attribute.
     *		If Rich private fading sequence is used, set fading
     *		attribute accordingly.
     *
     * RETURNS	OK
     *
     **************************************************************************/
    short ansi_sgr(PageType page, ListType u_list)
    {
        short i; /* number of parameters */
        StatusType sts = page.status;

        if (_parser.special_esc == '?')
        {
            /* DEC private -- ignore */
            _parser.state = INIT_STATE;
            return OK;
        }
        if (_parser.special_esc == '>')
        {
            /* Rich private fading attr */
            sts.setFade(true);
        }
        else
        {
            sts.setFade(false);
        }

        if (_parser.param_cnt == 0)
        {
            /* default to set current */
            sts.setplain(); /* attribute byte to normal */
            sts.setcolor(CharType.MONO);
        }

        for (i = 0; i < _parser.param_cnt; i++)
        {
            if (_parser.params[i] >= 30 && _parser.params[i] < 38)
            {
                if (sts._fade)
                {
                    sts.c_fade_attr &= 0xf0;
                    sts.c_fade_attr += (_parser.params[i] - 30);
                }
                else
                {
                    sts.c_attr &= 0xf0;
                    sts.c_attr += (_parser.params[i] - 30);
                }
                continue;
            }
            if (_parser.params[i] >= 40 && _parser.params[i] < 48)
            {
                if (sts._fade)
                {
                    sts.c_fade_attr &= 0x0f;
                    sts.c_fade_attr += ((_parser.params[i] - 40) << 4);
                }
                else
                {
                    sts.c_attr &= 0x0f;
                    sts.c_attr += ((_parser.params[i] - 40) << 4);
                }
                continue;
            }
            switch (_parser.params[i])
            {
                case 0:
                    sts.setcolor(CharType.MONO);
                    sts.setattr(CharType.PLAIN);
                    break;
                case 1:
                    sts.onattr(CharType.BRIGHT);
                    break;
                case 2:
                    sts.onattr(CharType.DIM);
                    break;
                case 4:
                    sts.onattr(CharType.UNDLN);
                    break;
                case 5:
                    sts.onattr(CharType.BLINK);
                    break;
                case 7:
                    sts.onattr(CharType.REVVID);
                    break;
                case 22:
                    sts.offattr((char)(CharType.DIM + CharType.BRIGHT));
                    break;
                case 24:
                    sts.offattr(CharType.UNDLN);
                    break;
                case 25:
                    sts.offattr(CharType.BLINK);
                    break;
                case 27:
                    sts.offattr(CharType.REVVID);
                    break;
                default:
                    break;
            }
        }
        _parser.state = INIT_STATE;
        return OK;
    }
        
    /* ************************************************************************
     *
     * ROUTINE NAME	_ansi_si()
     *
     * DESCRIPTION	Shift in -- make G0 character set the current set.
     *
     * RETURNS	OK.
     *
     **************************************************************************/
    short ansi_si(PageType page, ListType u_list)
    {
        page.status.gr_set = 0;
        _parser.state = INIT_STATE;
        return OK;
    }
        
    /* ************************************************************************
     *
     * ROUTINE NAME	_ansi_sm()
     *
     * DESCRIPTION	Alters bit-records of modes currently set.
     *		Must test p_esc to check for private modes.
     *
     * RETURNS	OK.
     *
     **************************************************************************/
    short ansi_sm(PageType page)
    {
        StatusType sts = page.status;

        if (_parser.special_esc != 0)
        {
            switch (_parser.special_esc)
            {
                case '?':
                    if (_parser.params[0] == 7)
                    {
                        sts.wrap_on = true;
                    }
                    break;
                default:
                    break;
            }
        }
        else
        {
            switch (_parser.params[0])
            {
                case 0x07:
                    sts.vem = true;
                    break;
                case 0x10:
                    sts.hem = true;
                    break;
                default:
                    break;
            }
        }
        _parser.state = INIT_STATE;
        return (OK);
    }
        
    /* ************************************************************************
     * 
     * ROUTINE NAME _ansi_so()
     * 
     * DESCRIPTION Shift out -- make G1 character set the current set.
     * 
     * RETURNS OK.
     * 
     **************************************************************************/
    short ansi_so(PageType page, ListType u_list)
    {
        page.status.gr_set = 1;
        _parser.state = INIT_STATE;
        return OK;
    }
        
    /* ************************************************************************
     * 
     * ROUTINE NAME _ansi_spesc()
     * 
     * DESCRIPTION
     * 
     * RETURNS OK.
     * 
     **************************************************************************/
    short ansi_spesc(char ch)
    {
        _parser.special_esc = ch;
        return OK;
    }
        
    /* ************************************************************************
     *
     * ROUTINE NAME	_ansi_stG0() and _ansi_stG1()
     *
     * DESCRIPTION	Enters the G0 state for selecting G0 graphics set and
     * 		enters the G1 state for selecting G1 graphics set.
     *
     * RETURNS	OK.
     *
     **************************************************************************/
    short ansi_stG0()
    {
        _parser.state = G0_STATE;
        return OK;
    }
        
    short ansi_stG1()
    {
        _parser.state = G1_STATE;
        return OK;
    }
	
    /* ************************************************************************
     * 
     * ROUTINE NAME _ansi_su()
     * 
     * DESCRIPTION
     * 
     * RETURNS ERROR if number of updates exceeds maximum, otherwise OK.
     * 
     **************************************************************************/
    short ansi_su(PageType page, ListType u_list)
    {
        short num_lines;
        StatusType sts = page.status;

        /* if row outside scrolling region, ignore message */
        if (sts.row < page.scroll_top || sts.row > page.scroll_bot)
        {
            _parser.state = INIT_STATE;
            return (OK);
        }

        num_lines = (_parser.param_cnt == 0) ? 1 : _parser.params[0];
        if (num_lines < 1)
            num_lines = 1; /* default and range check. */

        if (ansi_scroll(page, u_list, num_lines, S_UP, sts.row) == ERROR)
        {
            _parser.state = INIT_STATE;
            return (ERROR);
        }
        newUpd(u_list, 1, sts.row, sts.col, sts.col);
        _parser.state = INIT_STATE;
        return (OK);
    }
        
    /* ************************************************************************
     *
     * ROUTINE NAME	_ansi_tab()
     *
     * DESCRIPTION	Makes new line, i.e., combination of move to start
     *				of line and  move to next lower line, scrolling if
     *				necessary.
     *
     * RETURNS	ERROR if update list would overflow, else OK
     *
     **************************************************************************/
    short ansi_tab(PageType page, ListType u_list)
    {
        if (page.status.col <= TAB_TABLE.length)
        {
            page.status.col = TAB_TABLE[page.status.col - 1];
            newUpd(u_list, 1, page.status.row, page.status.col, page.status.col);
        }
        _parser.state = INIT_STATE;
        return OK;
    }

    short do_decode(char ch, PageType page, ByteArrayInputStream data, ListType u_list)
    {
        short ret = 0;

        switch (_parser.state)
        {
            case INIT_STATE:
                ret = do_decode_init(ch, page, data, u_list);
                break;
            case ESCAPE_STATE:
                ret = do_decode_escape(ch, page, u_list);
                break;
            case CSI_STATE:
                ret = do_decode_csi(ch, page, u_list);
                break;
            case G0_STATE:
                ret = do_decode_G0(ch, page);
                break;
            case G1_STATE:
                ret = do_decode_G1(ch, page);
                break;
            case CHARACTER_SIZE_STATE:
                ret = do_decode_character_size(ch, page, data, u_list);
                break;
            default: /* should never be in this state */
                break;
        }
        return ret;
    }

    short do_decode_character_size(char ch, PageType page, ByteArrayInputStream data, ListType u_list)
    {
        short ret = 0;
        if (ch == 27)
            ret = ansi_esc();
        else
            ret = ansi_nop();
        return ret;
    }
	
    short do_decode_csi(char ch, PageType page, ListType u_list)
    {
        short ret = 0;

        if (ch < 48)
        {
            ret = ansi_nop();
        }
        else if (ch < 58)
        {
            ret = ansi_param(ch, page);
        }
        else
        {
            switch (ch)
            {
                case 59:
                    ret = ansi_chparm();
                    break;
                case 61:
                case 62:
                case 63:
                    ret = ansi_spesc(ch);
                    break;
                case 64:
                    ret = ansi_ich(page, u_list);
                    break;
                case 65:
                    ret = ansi_cuu(page, u_list);
                    break;
                case 66:
                    ret = ansi_cud(page, u_list);
                    break;
                case 67:
                    ret = ansi_cuf(page, u_list);
                    break;
                case 68:
                    ret = ansi_cub(page, u_list);
                    break;
                case 72:
                    ret = ansi_poscur(page, u_list);
                    break;
                case 74:
                    ret = ansi_ed(page, u_list);
                    break;
                case 75:
                    ret = ansi_el(page, u_list);
                    break;
                case 76:
                    ret = ansi_il(page, u_list);
                    break;
                case 77:
                    ret = ansi_dl(page, u_list);
                    break;
                case 80:
                    ret = ansi_dch(page, u_list);
                    break;
                case 83:
                    ret = ansi_su(page, u_list);
                    break;
                case 84:
                    ret = ansi_sd(page, u_list);
                    break;
                case 90:
                    ret = ansi_db_p(page, u_list);
                    break;
                case 102:
                    ret = ansi_poscur(page, u_list);
                    break;
                case 104:
                    ret = ansi_sm(page);
                    break;
                case 108:
                    ret = ansi_rm(page);
                    break;
                case 109:
                    ret = ansi_sgr(page, u_list);
                    break;
                case 112:
                    ret = qa_reset(page, u_list);
                    break;
                case 114:
                    ret = ansi_decstbm(page, u_list);
                    break;
                default:
                    ret = ansi_nop();
                    break;
            }
        }

        return ret;
    }
	
    short do_decode_escape(char ch, PageType page, ListType u_list)
    {
        short ret = 0;
        switch (ch)
        {
            case 27:
                ret = ansi_esc();
                break;
            case 40:
                ret = ansi_stG0();
                break;
            case 41:
                ret = ansi_stG1();
                break;
            case 55:
                ret = ansi_decsc(page, u_list);
                break;
            case 56:
                ret = ansi_decrc(page, u_list);
                break;
            case 68:
                ret = ansi_lf(page, u_list);
                break;
            case 69:
                ret = ansi_nel(page, u_list);
                break;
            case 77:
                ret = ansi_ri(page, u_list);
                break;
            case 91:
                ret = ansi_csi();
                break;
            case 99:
                ret = qa_reset(page, u_list);
                break;
            default:
                ret = ansi_nop();
                break;
        }

        return ret;
    }
        
    short do_decode_G0(char ch, PageType page)
    {
        short ret = 0;
        if ((ch < 48) || (ch == 127))
        {
            ret = ansi_nop();
        }
        else
        {
            ret = ansi_dG0(ch, page);
        }
        return ret;
    }
        
    short do_decode_G1(char ch, PageType page)
    {
        short ret = 0;
        if ((ch < 48) || (ch == 127))
        {
            ret = ansi_nop();
        }
        else
        {
            ret = ansi_dG1(ch, page);
        }
        return ret;
    }
        
    short do_decode_init(char ch, PageType page, ByteArrayInputStream data, ListType u_list)
    {
        short ret = 0;

        switch (ch)
        {
            case 0:
            case 1:
            case 2:
            case 3:
            case 4:
            case 5:
            case 6:
            case 7:
                ret = ansi_nop();
                break;
            case 8:
                ret = ansi_cub(page, u_list);
                break;
            case 9:
                ret = ansi_tab(page, u_list);
                break;
            case 10:
                ret = ansi_lf(page, u_list);
                break;
            case 11:
            case 12:
                ret = ansi_nop();
                break;
            case 13:
                ret = ansi_cret(page, u_list);
                break;
            case 14:
                ret = ansi_so(page, u_list); /* shift out */
                break;
            case 15:
                ret = ansi_si(page, u_list); /* shift in */
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
                ret = ansi_nop();
                break;
            case 27:
                ret = ansi_esc();
                break;
            case 28:
            case 29:
            case 30:
            case 31:
            case 127:
                ret = ansi_nop();
                break;
            default:
                ret = ansi_pch(ch, page, data, u_list);
                break;
        }
        return ret;
    }
        
    boolean isprint(char ch)
    {
        return ((32 <= ch) && (ch < 127));
    }
	
    short newUpd(ListType upd, int req, short row, short beg, short end)
    {
        if (upd.index >= upd.upd_list.length - (req))
            return ERROR;

        upd.index++;
        upd.upd_list[upd.index].row = row;
        upd.upd_list[upd.index].upd_beg = beg;
        upd.upd_list[upd.index].upd_end = end;
        return OK;
    }
        
    short offset(short row, short col)
    {
        return (short)((row - 1) * PageColumns + (col - 1));
    }
        
    public int qa_decode(PageType page, ByteArrayInputStream is, ListType u_list)
    {
        int start_len;
        _parser = new ParserType();
        char ch;
        short ret;

        /* initialize index and the update list */
        u_list.index = 0;
        u_list.upd_list[u_list.index].row = page.status.row;
        u_list.upd_list[u_list.index].upd_beg = page.status.col;
        u_list.upd_list[u_list.index].upd_end = page.status.col;

        int len = is.available();

        _parser.txt_lngth = (short)len;

        start_len = len;

        int datum = is.read();
        while (datum > 0)
        {

            ch = (char)(datum & 0x7f); /* mask to 7 bit ASCII */
            if ((ret = do_decode(ch, page, is, u_list)) == ERROR)
            {
                if (_parser.txt_lngth == start_len)
                {
                    return (start_len);
                }
                return (start_len - _parser.txt_lngth);
            }

            if (ret == 0)
            {
                /* really an error from _pch */
                return (start_len - _parser.txt_lngth);
            }

            len -= ret;

            /* if a complete escape sequence has been parsed, save the length and reset initial conditions. */
            if (_parser.state == INIT_STATE)
            {
                _parser.txt_lngth = (short)len;
                _parser.param_cnt = 0;
                _parser.params[0] = 0;
                _parser.special_esc = 0;
            }
            datum = is.read();
        }
        return (start_len);
    }

    public short qa_end_of_row()
    {
        return EndOfRow;
    }
	
    public short qa_page_columns()
    {
        return PageColumns;
    }
	
    public short qa_page_rows()
    {
        return PageRows;
    }
	
    /**
     * Qa reset.
     *
     * @param page the page
     * @param u_list the u list
     * @return the short
     */
    public short qa_reset(PageType page, ListType u_list)
    {
        short i;
        StatusType sts = page.status;
        StatusType save = page.save;

        for (i = 0; i < PageRows * PageColumns; i++)
            page.page[i].clear();

        sts.row = save.row = 1;
        sts.col = save.col = 1;
        sts.cur_attr = save.cur_attr = 0;
        sts.c_attr = save.c_attr = 255;
        sts.c_fade_attr = save.c_fade_attr = 255;
        sts.fading = save.fading = 0;
        sts.gr_set = save.gr_set = 0;
        sts.wrap_on = save.wrap_on = false;

        page.scroll_top = 1;
        page.scroll_bot = (short)(PageColumns - 1);

        /* if the screen is reset, any other updates will be erased so overwrite them in the update list */
        u_list.index = 0;
        for (i = 1; i <= PageRows; i++)
            newUpd(u_list, PageRows - i + 1, i, (short)1, (short)(PageColumns + 1));

        /* set up new entry so that subsequent characters get added at the home position */
        newUpd(u_list, 1, sts.row, sts.col, sts.col);
        if (_parser != null)
            _parser.state = INIT_STATE;
        return OK;
    }
        
    public short qa_scroll_bot()
    {
        return ScrollBot;
    }
        
    public void qa_set_columns(short cl)
    {
        PageColumns = cl;
    }
        
    public void qa_set_end_of_row(short rw)
    {
        EndOfRow = rw;
    }
        
    public void qa_set_rows(short rw)
    {
        PageRows = rw;
    }
        
    public void qa_set_scroll_bot(short cl)
    {
        ScrollBot = cl;
    }
}

