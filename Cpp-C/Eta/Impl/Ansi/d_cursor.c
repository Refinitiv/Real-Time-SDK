/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright Thomson Reuters 2015,2016,2018. All rights reserved.            --
 *|-----------------------------------------------------------------------------
 */

#include "ansi/q_ansi.h"
#include "ansi/decodeansi.h"
#include "ansi/ansi_int.h"


/*************************************************************************
*
* ROUTINE NAME	_cud()
*
* DESCRIPTION	This routine is called whenever the cursor is moved down
*		but scrolling should not occur.  This routine
*		is used for processing the CUD sequence, but NOT for 
*		new lines. 
*
* RETURNS	ERROR if update list would overflow, else OK
*
**************************************************************************/
#ifdef __STDC__
short _ansi_cud(
	PAGEPTR page,
	char	*text,
	LISTPTR u_list )
#else
short _ansi_cud( page, text, u_list )
	PAGEPTR page;
	char    *text;
	LISTPTR u_list;
#endif
{
   	register short num_rows;
	register STATUSPTR	sts;

	sts = &(page->status);

   	num_rows = (parse_ptr->param_cnt == 0) ? 1 : parse_ptr->params[0];

	if (sts->row <= page->scroll_bot) {
		if (sts->row + num_rows > page->scroll_bot)
			sts->row = page->scroll_bot;
		else
			sts->row += num_rows;
	}
	else {
		/* cursor must be between scroll bottom and end of page */
		if (sts->row + num_rows > PAGEROWS)
			sts->row = PAGEROWS;
		else
			sts->row += num_rows;
	}

	NEW_UPD(u_list, 1, sts->row, sts->col, sts->col);
	parse_ptr->state = INITSTATE;
	parse_ptr->wrap = FALSE;
	return (OK);
}

/*************************************************************************
*
* ROUTINE NAME	_ansi_cub()
*
* DESCRIPTION	Moves cursor backward param_cnt positions in the
*		        line, stopping at the beginning, defaulting to
*				one, and moving one if param_cnt = 0.
*
* RETURNS	ERROR if update list would overflow, else OK
*
**************************************************************************/
#ifdef __STDC__
short _ansi_cub(
	PAGEPTR page,
	char	*text,
	LISTPTR u_list )
#else
short _ansi_cub( page, text, u_list )
	PAGEPTR page;
	char    *text;
	LISTPTR u_list;
#endif
{
   	short 	num_col;

   	num_col = (parse_ptr->param_cnt == 0) ? 1 : parse_ptr->params[0];

   	if (num_col < 1) 
		num_col = 1;	/*  default and range check  */

	/* Moving cursor back, being sure to go no further than the start of the           line. */
	if (page->status.col - num_col <= 0)
		page->status.col = 1;
	else
		page->status.col -= num_col;
	NEW_UPD(u_list, 1, page->status.row, page->status.col, page->status.col);
	parse_ptr->state = INITSTATE;
	parse_ptr->wrap = FALSE;
	return (OK);
}

/*************************************************************************
*
* ROUTINE NAME	_ansi_cuf()
*
* DESCRIPTION	Moves cursor forward param_cnt positions in the 
*		        line, stopping at the line end, defaulting to one,
*				and moving one if param_cnt = 0.
*
* RETURNS	ERROR if update list would overflow, else OK
*
**************************************************************************/
#ifdef __STDC__
short _ansi_cuf(
	PAGEPTR page,
	char	*text,
	LISTPTR u_list )
#else
short _ansi_cuf( page, text, u_list )
	PAGEPTR page;
	char    *text;
	LISTPTR u_list;
#endif
{
	short numcol;

	if (parse_ptr->special_esc != 0) {
	    switch(parse_ptr->special_esc) {
		case '=':
			break;		/* invisible cursor currently ignored */
		default:
			break;
	    }    
	}
	else {
		numcol = (parse_ptr->param_cnt == 0) ? 1 : parse_ptr->params[0];

		/* Moving cursor forward, making sure */
		/* to stop at the end of the line */
		if (page->status.col + numcol > PAGECOLS)
			page->status.col = PAGECOLS;
		else
			page->status.col += numcol;
		NEW_UPD (u_list, 1, page->status.row, page->status.col,
		         page->status.col);
	}
	parse_ptr->state = INITSTATE;
	return (OK);
}

/*************************************************************************
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
#ifdef __STDC__
short _ansi_cuu(
	PAGEPTR page,
	char	*text,
	LISTPTR u_list )
#else
short _ansi_cuu( page, text, u_list )
	PAGEPTR page;
	char    *text;
	LISTPTR u_list;
#endif
{
	short 			num_rows;
	register STATUSPTR	sts;

	sts = &(page->status);

	num_rows = (parse_ptr->param_cnt == 0) ? 1 :parse_ptr->params[0];

	if (sts->row >= page->scroll_top) {
		if (sts->row - num_rows < page->scroll_top)
			sts->row = page->scroll_top;
		else
			sts->row -= num_rows;
	}
	else {
		/* cursor must be between scroll top and top of page */
		if (sts->row - num_rows < SCROLL_TOP)
			sts->row = SCROLL_TOP;
		else
			sts->row -= num_rows;
	}

	NEW_UPD(u_list, 1, sts->row, sts->col, sts->col);
	parse_ptr->state = INITSTATE;
	parse_ptr->wrap = FALSE;
	return (OK);
}

/*************************************************************************
*
* ROUTINE NAME	ansi_poscur()
*
* DESCRIPTION	Moves cursor to position in line params[0] and
*		row params[1].  Defaults to 'home'.  
*
* RETURNS	ERROR if update list would overflow, else OK
*
**************************************************************************/
#ifdef __STDC__
short _ansi_poscur(
	PAGEPTR page,
	char	*text,
	LISTPTR u_list )
#else
short _ansi_poscur( page, text, u_list )
	PAGEPTR page;
	char    *text;
	LISTPTR u_list;
#endif
{
	register STATUSPTR	sts;
	short			row;
	short			col;

	sts = &(page->status);
	switch (parse_ptr->param_cnt ) {
	case 0:
		row = 1;
		col = 1;
		break;
	case 1:
		row = parse_ptr->params[0];
		col = 1;
		break;
	case 2:
		row = parse_ptr->params[0];
		col = parse_ptr->params[1];
		break;
	default:
		parse_ptr->state = INITSTATE;
		return(OK);
	}
	if (row == 0)
		row = 1;
	if (col == 0)
		col = 1;
	if ((row > PAGEROWS) || (col > PAGECOLS)) {
		parse_ptr->state = INITSTATE;
#ifdef TRACE
		if (stdout != NULL)
			fprintf (stdout, "_ansi_poscur: bad position row = %d col = %d\n", row, col);
#endif
		return(OK); /* ignore bad positioning */
	}
	
	sts->row = row;
	sts->col = col;

	NEW_UPD(u_list, 1, sts->row, sts->col, sts->col);
	parse_ptr->state = INITSTATE;
	parse_ptr->wrap = FALSE;
#ifdef TRACE
	if (stdout != NULL)
		fprintf (stdout, "_ansi_poscur: index = %d row = %d col = %d\n",
	u_list->index, u_list->upd_list[u_list->index].row,
	u_list->upd_list[u_list->index].upd_beg);
#endif
	return (OK);
}

/*************************************************************************
*
* ROUTINE NAME	ansi_decstbm()
*
* DESCRIPTION	DEC private sequence to select top and bottoms margins
*		for scrolling.
*
* RETURNS	ERROR if update list would overflow, else OK
*
**************************************************************************/
#ifdef __STDC__
short _ansi_decstbm(
	PAGEPTR page,
	char	*text,
	LISTPTR u_list )
#else
short _ansi_decstbm( page, text, u_list )
	PAGEPTR page;
	char    *text;
	LISTPTR u_list;
#endif
{
	register STATUSPTR	sts;
	short			top;
	short			bottom;

	sts = &(page->status);
	switch (parse_ptr->param_cnt ) {
	case 0:
		top = SCROLL_TOP;
		bottom = SCROLL_BOT;
		break;
	case 1:
		top = parse_ptr->params[0];
		bottom = SCROLL_BOT;
		break;
	case 2:
		top = parse_ptr->params[0];
		bottom = parse_ptr->params[1];
		break;
	default:
		parse_ptr->state = INITSTATE;
		return(OK);
	}
	if (top < 1) {
		top = SCROLL_TOP;
	}
	if (bottom > PAGEROWS) {
		bottom = SCROLL_BOT;
	}

	if (bottom > top) {
		page->scroll_top = top;
		page->scroll_bot = bottom;
		sts->row = 1;
		sts->col = 1;
		/* home the cursor when scrolling region changed */
		NEW_UPD(u_list, 1, 1, 1, 1);
		parse_ptr->wrap = FALSE;
	}
	parse_ptr->state = INITSTATE;
	parse_ptr->scrolled = FALSE;
	return(OK);
}
