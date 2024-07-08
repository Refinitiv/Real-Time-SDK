/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2019 LSEG. All rights reserved.                 --
 *|-----------------------------------------------------------------------------
 */


#include "ansi/q_ansi.h"
#include "ansi/decodeansi.h"
#include "ansi/ansi_int.h"


/*************************************************************************
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
#ifdef __STDC__
short _ansi_dl(
	register PAGEPTR    page,
	register char       *text,
	LISTPTR             u_list )
#else
short _ansi_dl( page, text, u_list )
	register PAGEPTR    page;
	register char       *text;
	LISTPTR             u_list;
#endif
{
	short 		num_lines;
	STATUSPTR	sts;

	sts = &(page->status);
	if (sts->row < page->scroll_top ||
	    sts->row > page->scroll_bot) {
		parse_ptr->state = INITSTATE;
		return (OK);
	}

        num_lines = (parse_ptr->param_cnt == 0) ? 1 : parse_ptr->params[0];
	if (num_lines < 1) /*  default and range check */
		num_lines = 1; 
	if (num_lines + sts->row > page->scroll_bot)
		num_lines = page->scroll_bot - sts->row;

	if (_ansi_scroll(page, u_list, num_lines, S_UP, sts->row) == ERROR){
		parse_ptr->state = INITSTATE;
		return (ERROR);
	}
	sts->col = 1;
	NEW_UPD(u_list, 1, sts->row, 1, 1);
	parse_ptr->state = INITSTATE;
	parse_ptr->wrap = FALSE;
	return(OK);
}

/*************************************************************************
*
* ROUTINE NAME	ansi_ich()
*
* DESCRIPTION	Insert character -- Inserts params[0] blanks in the
*				line, starting at current position, working to 
*				the right, pushing the rest of the line right, defaulting 
*				to one and moving one if params[0] = 0.
*				This operation is not affected by wrap; if characters are
*				pushed past the end of the line, they are destroyed.
*				The spaces inserted will only fit on one line.
*
* RETURNS	ERROR if num of updates exceeds maximum, otherwise OK.
*
**************************************************************************/
#ifdef __STDC__
short _ansi_ich(
	register PAGEPTR    page,
	register char       *text,
	LISTPTR             u_list )
#else
short _ansi_ich( page, text, u_list )
	register PAGEPTR    page;
	register char       *text;
	LISTPTR             u_list;
#endif
{
	short 		num_cols, startpg, endpg;
	register short 	i;
	STATUSPTR	sts;

	sts = &(page->status);

	/* Storing original cursor position. */
	startpg = offset(sts->row, sts->col);
	
 	num_cols = (parse_ptr->param_cnt == 0) ? 1 : parse_ptr->params[0];
	if (num_cols < 1) num_cols = 1; 	/* default and range check */

	/* Check to see if number of cols  to be inserted is longer
	   than the space available on the line. */
	if (num_cols + sts->col > PAGECOLS) {
		/* set num_cols to the number of columns left on line */
		num_cols = (PAGECOLS - sts->col + 1);
	}
	endpg = offset(sts->row, PAGECOLS);
	/* create update list */
	NEW_UPD(u_list, 2, sts->row, sts->col, PAGECOLS + 1);

	/* Moving characters to the right */
	for (i = endpg ; i >= (startpg + num_cols) ; --i) {
		page->page[i] = page->page[i - num_cols];
	}

	/* Fill in inserted positions with spaces. */
	for (i = startpg; i < (startpg + num_cols); ++i) {
		page->page[i] = null_char;
	}
	NEW_UPD(u_list, 1, sts->row, sts->col, sts->col);
	parse_ptr->state = INITSTATE;
	return (OK);
}

/*************************************************************************
*
* ROUTINE NAME	_ansi_el()
*
* DESCRIPTION	Erases some or all of a line, depending on the parameter
*				value.  The update list must already have an entry for
*				the current line in it so only the columns need to be
*				changed.  Entering more characters would cause upd_end to be
*				incremented so a new entry in update list  must be made.
*
* RETURNS	ERROR if num of updates exceeds maximum, otherwise OK.
*
**************************************************************************/
#ifdef __STDC__
short _ansi_el(
	register PAGEPTR    page,
	register char       *text,
	LISTPTR             u_list )
#else
short _ansi_el( page, text, u_list )
	register PAGEPTR    page;
	register char       *text;
	LISTPTR             u_list;
#endif
{
	register short		i;
	short			pg_pos; 
	register CHARPTR 	pg;
	register short 		end, start;
	STATUSPTR		sts;

	sts = &(page->status);

	/* default and range check. */
	parse_ptr->params[0] = (parse_ptr->param_cnt == 0) ? 
	    0 : parse_ptr->params[0];

	switch (parse_ptr->params[0]) {
	case 0: 
		end = offset(sts->row, PAGECOLS); /* end of current line */
		pg_pos = offset(sts->row, sts->col);
 		/* erasing from current position to end of line */
		pg = &(page->page[pg_pos]);
		for (i = pg_pos; i <= end; i++) {
			*pg = null_char;
			pg++;
		}
		u_list->upd_list[u_list->index].upd_end = END_OF_ROW;
		break;

	case 1:
		start = offset(sts->row, 1); /* start of current line */
		pg_pos = offset(sts->row, sts->col);
		/* erasing from start of line to current position */
		u_list->upd_list[u_list->index].upd_beg = 1;

		pg = &(page->page[start]);
		for (i = start; i <= pg_pos; i++) {
			*pg = null_char;
			pg++;
		}
		break;

	case 2:
		start = offset(sts->row, 1); /* start of current line */
		end = offset(sts->row, PAGECOLS); /* end of current line */
		/* erasing whole line */
		pg = &(page->page[start]);
		for (i = start; i <= end; i++) {
			*pg = null_char;
			pg++;
		}
		u_list->upd_list[u_list->index].upd_beg = 1;
		u_list->upd_list[u_list->index].upd_end = END_OF_ROW;
		break;
	default:
		parse_ptr->state = INITSTATE;
		return(OK);
	}
	/* Entering more characters would cause upd_end to be
	   incremented so we must make a new entry in update list */
	NEW_UPD(u_list, 1, sts->row, sts->col, sts->col);
	parse_ptr->state = INITSTATE;
	return (OK);
}

/*************************************************************************
*
* ROUTINE NAME	ansi_ed()
*
* DESCRIPTION	Erases some or all of the display, depending on the
*				parameter values. 
*
* RETURNS	ERROR if num of updates exceeds maximum, otherwise OK.
*
**************************************************************************/
#ifdef __STDC__
short _ansi_ed(
	register PAGEPTR    page,
	register char       *text,
	LISTPTR             u_list )
#else
short _ansi_ed( page, text, u_list )
	register PAGEPTR    page;
	register char       *text;
	LISTPTR             u_list;
#endif
{
	register short		i, pg_pos; 
	register CHARPTR 	pg;
	register short		col;
	short			row;
	short 			end, start;
	short			rows_needed;	/* num update entries need to
						   make */
	STATUSPTR		sts;

	sts = &(page->status);

	/* default and range check. */
	parse_ptr->params[0] = (parse_ptr->param_cnt == 0) ? 
	    0 : parse_ptr->params[0];

	col = sts->col;
	row = sts->row;
	switch (parse_ptr->params[0]) {
	case 0:
 		/* erasing from current position to end of page */
		end = offset(PAGEROWS, PAGECOLS);	/* end of page */
		pg_pos = offset(sts->row, sts->col);
		rows_needed = PAGEROWS - sts->row + 1; /* +1 for last NEW_UPD */
		/* mark current update to end of this row */
		u_list->upd_list[u_list->index].upd_end = END_OF_ROW;

		pg = &(page->page[pg_pos]);
		for (i = pg_pos; i <= end; i++) {
		    if (col <= PAGECOLS) {
		       	col++;
		    }
		    else {
			col = 1;
			row++;
			/* mark each new row as the whole row */
			NEW_UPD(u_list, rows_needed, row, 1, END_OF_ROW);
			rows_needed--;
		    }
		    *pg = null_char;
		    pg++;
		}
		NEW_UPD(u_list, 1, sts->row, sts->col, sts->col);
		break;
	case 1:
		/* erasing from start of page to current position */
		start = offset(1, 1);			/* start of page */
		pg_pos = offset(sts->row, sts->col);
		row = 1;
		/* overwrite current update, since it will be included
		   in the for loop */
		u_list->upd_list[u_list->index].row = 1;
		u_list->upd_list[u_list->index].upd_beg = 1;
		rows_needed = sts->row;

		pg = &(page->page[start]);
		for (i = start; i <= pg_pos; i++, pg++) {
		    if (col <= PAGECOLS) {
		       	col++;
		    }
		    else {
			col = 1;
			row++;
			u_list->upd_list[u_list->index].upd_end = END_OF_ROW;
			NEW_UPD(u_list, rows_needed, row, 1, 1);
			rows_needed--;
		    }
		    *pg = null_char;
		}
		u_list->upd_list[u_list->index].upd_end = sts->col;
		break;
	case 2:
		start = offset(1, 1);			/* start of page */
		/* erasing whole screen */
		pg = &(page->page[start]);
		for (row = 1; row <= PAGEROWS; row++) {
			for (col = 1; col <= PAGECOLS; col++) {
				*pg = null_char;
				pg++;
			}
			NEW_UPD(u_list, PAGEROWS - row + 2, row, 1, END_OF_ROW);
		}
		NEW_UPD(u_list, 1, sts->row, sts->col, sts->col);
		break;
	default:
		break;
	}
	parse_ptr->state = INITSTATE;
	return (OK);
}

/*************************************************************************
*
* ROUTINE NAME	ansi_il()
*
* DESCRIPTION	Inserts params[0] erased lines, starting at the current 
*				line, working downward, defaulting to one, and inserting 
*				one if params[0] = 0. The current and following lines 
*				are moved down params[0] rows, with the last params[0] 
*				lines being lost.
*
* RETURNS	ERROR if num of updates exceeds maximum, otherwise OK.
*
**************************************************************************/
#ifdef __STDC__
short _ansi_il(
	register PAGEPTR    page,
	register char       *text,
	LISTPTR             u_list )
#else
short _ansi_il( page, text, u_list )
	register PAGEPTR    page;
	register char       *text;
	LISTPTR             u_list;
#endif
{ 
	short 			num_lines; 
	STATUSPTR		sts;

	sts = &(page->status);

	/* if row outside scrolling region, ignore message */
	if (sts->row < page->scroll_top ||
	    sts->row > page->scroll_bot) {
		parse_ptr->state = INITSTATE;
		return (OK);
	}

	num_lines = (parse_ptr->param_cnt == 0) ? 1 : parse_ptr->params[0];
	if (num_lines < 1 ) num_lines = 1; 	/* default and range check. */

	if (num_lines + sts->row > page->scroll_bot) {
		/* too many lines, truncate */
		num_lines = page->scroll_bot - sts->row;
	}
	/* *** Move to the start of the current line. *** */
	sts->col = 1;
	parse_ptr->wrap = FALSE;
	if (_ansi_scroll(page, u_list, num_lines, S_DOWN, sts->row) == ERROR){
		parse_ptr->state = INITSTATE;
		return (ERROR);
	}

	NEW_UPD(u_list, 1, sts->row, 1, 1);
	parse_ptr->state = INITSTATE;
	return(OK);
}

/*************************************************************************
*
* ROUTINE NAME	_ansi_dch()
*
* DESCRIPTION	Deletes params[0] chars, starting at the current 
*				position and working forward. Defaults to one char
*				if params[0] = 0. Remaining chars are moved up to 
*				the current position.  The vacated character positions
*				at the end of the line are erased.
*
* RETURNS	ERROR if num of updates exceeds maximum, otherwise OK.
*
**************************************************************************/
#ifdef __STDC__
short _ansi_dch(
	register PAGEPTR    page,
	register char       *text,
	LISTPTR             u_list )
#else
short _ansi_dch( page, text, u_list )
	register PAGEPTR    page;
	register char       *text;
	LISTPTR             u_list;
#endif
{
	short 		i, num_chars, start, end;
	STATUSPTR	sts;

	sts = &(page->status);

	num_chars = (parse_ptr->param_cnt == 0) ? 1 : parse_ptr->params[0];
	if (num_chars < 1 ) num_chars = 1; 	/* default and range check */
	if (num_chars > PAGECOLS) num_chars = PAGECOLS;

	/* Start deleting characters, starting at the current curs. pos. */
   	start = offset(sts->row, sts->col);
   	end = offset(sts->row, PAGECOLS);
	NEW_UPD(u_list, 2, sts->row, sts->col, PAGECOLS + 1);

	for (i = (start + num_chars); i <= end; ++i) {
		page->page[(i - num_chars)] = page->page[i];
	}

	/* Fill in the end with blanks. */
	for (i = end; i > (end - num_chars); --i) {
		page->page[i] = null_char;
	}
	NEW_UPD(u_list, 1, sts->row, sts->col, sts->col);
	parse_ptr->state = INITSTATE;
	return(OK);
}

/*************************************************************************
*
* ROUTINE NAME	_ansi_su()
*
* DESCRIPTION	
*
* RETURNS	ERROR if num of updates exceeds maximum, otherwise OK.
*
**************************************************************************/
#ifdef __STDC__
short _ansi_su(
	register PAGEPTR    page,
	register char       *text,
	LISTPTR             u_list )
#else
short _ansi_su( page, text, u_list )
	register PAGEPTR    page;
	register char       *text;
	LISTPTR             u_list;
#endif
{
	short		num_lines; 
	STATUSPTR	sts;

	sts = &(page->status);

	/* if row outside scrolling region, ignore message */
	if (sts->row < page->scroll_top ||
	    sts->row > page->scroll_bot) {
		parse_ptr->state = INITSTATE;
		return (OK);
	}

	num_lines = (parse_ptr->param_cnt == 0) ? 1 : parse_ptr->params[0];
	if (num_lines < 1 ) 
		num_lines = 1; 	/* default and range check. */
	if (_ansi_scroll(page, u_list, num_lines, S_UP, sts->row) == ERROR){
		parse_ptr->state = INITSTATE;
		return (ERROR);
	}
	NEW_UPD(u_list, 1, sts->row, sts->col, sts->col);
	parse_ptr->state = INITSTATE;
	return(OK);
}

/*************************************************************************
*
* ROUTINE NAME	_ansi_sd()
*
* DESCRIPTION	
*
* RETURNS	ERROR if num of updates exceeds maximum, otherwise OK.
*
**************************************************************************/
#ifdef __STDC__
short _ansi_sd(
	register PAGEPTR    page,
	register char       *text,
	LISTPTR             u_list )
#else
short _ansi_sd( page, text, u_list )
	register PAGEPTR    page;
	register char       *text;
	LISTPTR             u_list;
#endif
{
	short		num_lines; 
	STATUSPTR	sts;

	sts = &(page->status);

	/* if row outside scrolling region, ignore message */
	if (sts->row < page->scroll_top ||
	    sts->row > page->scroll_bot) {
		parse_ptr->state = INITSTATE;
		return (OK);
	}

	num_lines = (parse_ptr->param_cnt == 0) ? 1 : parse_ptr->params[0];
	if (num_lines < 1 ) 
		num_lines = 1; 	/* default and range check. */
	if (_ansi_scroll(page, u_list, num_lines, S_DOWN, sts->row) == ERROR){
		parse_ptr->state = INITSTATE;
		return (ERROR);
	}
	NEW_UPD(u_list, 1, sts->row, sts->col, sts->col);
	parse_ptr->state = INITSTATE;
	return(OK);
}
