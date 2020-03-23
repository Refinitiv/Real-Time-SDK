/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|          Copyright (C) 2019-2020 Refinitiv. All rights reserved.          --
 *|-----------------------------------------------------------------------------
 */

#include <stdio.h>
#include <ctype.h>
#include "ansi/q_ansi.h"
#include "ansi/decodeansi.h"
#include "ansi/ansi_int.h"

#if defined(_WIN16) 
#define	stderr	(FILE*)0		// note that no messsage will be displayed
#endif

#ifdef OS2
#define toascii(c) ((c)&0x7f)
#endif

#if defined(WIN32) && __STDC__
#define toascii __toascii
#endif

PARSEPTR	parse_ptr;		/* pointer to internal parser state */
short 	PAGECOLS=80;
short 	PAGEROWS=25;
short 	END_OF_ROW=81; /* PAGECOLS + 1 */
short 	SCROLL_BOT=24; /* PAGEROWS - 1 */

/* ---- IMPORTANT ---  If the order or size of CHARTYP changes,
   the erased character definitions must also be changed. */
#ifdef ATTRIBUTES
    CHARTYP null_char = {0x20, US_ASCII, _PLAIN, _PLAIN, MONO, MONO};
#else
    CHARTYP null_char = {0x20, US_ASCII};
#endif

#include "ansi/d_table.h"



short qa_page_rows()
{
	return(PAGEROWS);
}

short qa_page_columns()
{
	return(PAGECOLS);
}

short qa_end_of_row()
{
	return(END_OF_ROW);
}

short qa_scroll_bot()
{
	return(SCROLL_BOT);
}

#ifdef __STDC__
void qa_set_rows(
	short rw )
#else
void qa_set_rows( rw )
	short rw;
#endif
{
	PAGEROWS = rw;
}

#ifdef __STDC__
void qa_set_columns(
	short cl )
#else
void qa_set_columns( cl )
	short cl;
#endif
{
	PAGECOLS = cl;
}

#ifdef __STDC__
void qa_set_end_of_row(
	short rw )
#else
void qa_set_end_of_row( rw )
	short rw;
#endif
{
	END_OF_ROW = rw;
}

#ifdef __STDC__
void qa_set_scroll_bot(
	short cl )
#else
void qa_set_scroll_bot( cl )
	short cl;
#endif
{
	SCROLL_BOT = cl;
}

/*************************************************************************
*
* ROUTINE NAME	qa_decode()
*
* DESCRIPTION	Decoding an ANSI string is accomplished using a finite 
*		state machine.  Each of the ANSI escape sequences is
*		parsed according to the state into which the previous 
*		characters have the machine.
*
*		As the sequences are recognized, the page image is modified.
*
*		An update list is maintained to make identification of
*		changed regions easier.  Each entry has the row, the
*		start column and the end column of the portion of the
*		screen that was modified.  Entries where the start column
*		and the end column are equal indicate that no update
*		has occured at this location.
*
*		The update list is maintained according to the following 
*		rules:
*
*		  1. Printable Characters written to the page increment 
*		     the end col.
*
*		  2. When the end of the line is reached, if line wrap is 
*		     on, a new entry in the list is created, the row being 
*		     the next row, the start and end column being set to 1.
*		     If wrap is off, the character overwrites the one
*		     currently at the 80th column and the end column
*		     field in the update list is not modified.
* 
*		  3. When scrolling occurs, an entry is made in the
*		     update list for each row that was modified.
*
*		  4. Any sequence or character that moves the cursor
*		     position causes a new entry to be made in the list.
*		     This includes cursor positioning sequences, backspace,
*		     linefeed, etc.
*
*		  5. Erase commands cause new entries to be made for each
*		     row that is affected.
*
* RETURNS	Returns the number of characters in text it has successfully 
*		parsed.  It will not return a length which would cause
*		an escape sequence to be broken.  Qa_decode may be called 
*		as many times as necessary until all the text is parsed.
*		If qa_decode has completed, the length which was passed to it
*		is returned.
*
**************************************************************************/
#ifdef __STDC__
int qa_decode(
	register PAGEPTR 	page,
	register char 		*text,
	register int		len,
	LISTPTR				u_list )
#else
int qa_decode( page, text, len, u_list )

	register PAGEPTR    page;
	register char       *text;
	register int        len;
	LISTPTR             u_list;

#endif

{
	int		start_len;
	PARSETYP	parser;
	char		ch;
	short		ret;

	/* initialize index and the update list */
	u_list->index = 0;
	u_list->upd_list[u_list->index].row = page->status.row;
	u_list->upd_list[u_list->index].upd_beg = page->status.col;
	u_list->upd_list[u_list->index].upd_end = page->status.col;

	/* initialize parser state */
	parse_ptr = &parser;
	parse_ptr->txt_lngth = len;
	parse_ptr->param_cnt = 0;
	parse_ptr->params[0] = 0;
	parse_ptr->state = INITSTATE;
	parse_ptr->cr_state = INITSTATE;
	parse_ptr->special_esc = 0;
	parse_ptr->wrap = FALSE;
	parse_ptr->scrolled = FALSE;
	
	start_len = len;

	while (len) {
	    ch = *text & 0x7f;		/* mask to 7 bit ASCII */
	    if ((ret = (*_ansi_do_decode[parse_ptr->state][ch])(page, text, u_list))
	      == ERROR) {
		ASSERT(parse_ptr->txt_lngth != start_len);
		if (parse_ptr->txt_lngth == (unsigned) start_len) {
			if (stderr != NULL)
		    {
				fprintf(stderr, 
		  		"qa_decode: Update list too small!\n");
		    	fprintf(stderr, 
				"           Please recompile with a larger size list\n");
			}
		    return(start_len);
		}
		return(start_len - parse_ptr->txt_lngth);
	    }
	    if (ret == 0) {
		/* really an error from _pch */
		return(start_len - parse_ptr->txt_lngth);
	    }
	    text += ret;
	    len -= ret;
	    /* if a complete escape sequence has been parsed,
    	       save the length and reset initial conditions. */
	    if (parse_ptr->state == INITSTATE) {
		parse_ptr->txt_lngth = len;	
		parse_ptr->param_cnt = 0;
		parse_ptr->params[0] = 0;
		parse_ptr->special_esc = 0;
	    }
	}
   	return(start_len);
} /* qa_decode */
  
/*************************************************************************
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

#ifdef __STDC__
short _ansi_pch(
	register PAGEPTR 	page,
	register char 		*text,
	LISTPTR 			u_list )
#else
short _ansi_pch( page, text, u_list )
	register PAGEPTR    page;
	register char       *text;
	LISTPTR             u_list;
#endif

{
	register int			txt_left;
	register CHARPTR		chptr;
	register STATUSPTR		status;
	register struct upd_type	*upd_ptr;

	txt_left = parse_ptr->txt_lngth;
	status = &(page->status);
	upd_ptr = &(u_list->upd_list[u_list->index]);
	chptr = &(page->page[offset(status->row, status->col)]);
	parse_ptr->cr_state = INITSTATE;

	while ((isprint(toascii(*text))) || (*text == '\0')) {

	    if (*text != '\0') {

		/* if 80th col already written, but wrap is on */
		if (parse_ptr->wrap == TRUE) {
			/* adjust pointer BEFORE writing character. If 
			   cursor is at end of scroll reg, scroll the screen. */
			if (status->row == page->scroll_bot) {
			    if (_ansi_scroll(page, u_list, 1, S_UP, 
				page->scroll_top) == ERROR){
   				    return(parse_ptr->txt_lngth - txt_left);
				}
			    status->col = 1;
			    NEW_UPD(u_list, 1, status->row, 1, 1);
			    }
			else if (status->row != PAGEROWS) {
			    if(u_list->index >= u_list->max_updt - 1) {
				/* if list full, return what we have so far */
   				return(parse_ptr->txt_lngth - txt_left);
			    }
			    /* just do line feed */
			    status->col = 1;
			    status->row++;
			    NEW_UPD(u_list, 1, status->row, 1, 1);
			    }
			/*
			else
			    cursor not in scrolling region but at end of page
			    do nothing.
			*/

			chptr = &(page->page[offset(status->row,
			     status->col)]);
			upd_ptr = &(u_list->upd_list[u_list->index]);
			parse_ptr->wrap = FALSE;
		}
			
		/* for speed copy address of character in page to a register */
		chptr->ch = *text;
#ifdef ATTRIBUTES
		/* put the attribute in page */
		chptr->attr = status->cur_attr;
		chptr->c_attr = status->c_attr;
		chptr->c_fade_attr = status->c_fade_attr;
		chptr->fade_attr = status->fading;
#endif

		/* use graphics set currently selected */
		if (status->gr_set == 1)
			chptr->gs = status->G1_set;
		else
			chptr->gs = status->G0_set;

		/* check to see if at end of line */
		if (status->col >= PAGECOLS) {
			/* yes, set updt_end, check wrap flag*/
			upd_ptr->upd_end = END_OF_ROW; 
		    	if (status->wrap_on) {
				/* mark that we have written the 80th char
				   but that the cursor is still at col 80 */
				parse_ptr->wrap = TRUE;
		    	}
		}
		else {
			status->col++;
			upd_ptr->upd_end += 1; 
	    		chptr++;
		}

	    } /* end if not null character */

	    text++;
    
	    /* if txt_left goes to zero, end of text has been reached */
	    if (--(txt_left) == 0) {
		break;
	    }
	}
	/* return count on number of chars processed */
   	return(parse_ptr->txt_lngth - txt_left);
}


/*************************************************************************
*
* ROUTINE NAME	_ansi_param()
*
* DESCRIPTION	If the parameter count exceeds the maximum, ignore the
*		number.
*
* RETURNS	OK.
*
**************************************************************************/

#ifdef __STDC__
short _ansi_param(
	PAGEPTR page,
	char 	*text,
	LISTPTR u_list )
#else
short _ansi_param( page , text, u_list )
	PAGEPTR page;
	char    *text;
	LISTPTR u_list;
#endif
{
	if (parse_ptr->param_cnt == 0) /*test for 1st digit of 1st parm  */
		(parse_ptr->param_cnt)++;
	if (parse_ptr->param_cnt > MAXCSIPARAMS)
		return(OK);

	parse_ptr->params[parse_ptr->param_cnt - 1] *= 10;
	parse_ptr->params[parse_ptr->param_cnt - 1] += *text - '0';
	return(OK);
}

/*************************************************************************
*
* ROUTINE NAME	_ansi_spesc()
*
* DESCRIPTION	
*
* RETURNS	OK.
*
**************************************************************************/

#ifdef __STDC__
short _ansi_spesc(
	PAGEPTR page,
	char 	*text,
	LISTPTR u_list )
#else
short _ansi_spesc( page, text, u_list )
	PAGEPTR page;
	char    *text;
	LISTPTR u_list;
#endif
{
	parse_ptr->special_esc = *text;
	return(OK);
}

/*************************************************************************
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
#ifdef __STDC__
short _ansi_chparm(
	PAGEPTR page,
	char 	*text,
	LISTPTR u_list )
#else
short _ansi_chparm( page, text, u_list )
	PAGEPTR page;
	char    *text;
	LISTPTR u_list;
#endif
{
	if (parse_ptr->param_cnt < MAXCSIPARAMS) {
		(parse_ptr->param_cnt)++;
		parse_ptr->params[parse_ptr->param_cnt - 1] = 0;
	}
	return(OK);
}
