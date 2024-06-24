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
* ROUTINE NAME	qa_reset()
*
* DESCRIPTION	Full reset of page image to initial state: 
*		Clears screen and resets control structure.
*		Function speeded up by doing a block structure copy 
*		of null_page instead of going through a loop.
*
* RETURNS	Nothing.
*
**************************************************************************/
#ifdef __STDC__
short qa_reset(
    register PAGEPTR    page,
    register char       *text,
    LISTPTR             u_list )
#else
short qa_reset( page, text, u_list )
    register PAGEPTR    page;
    register char       *text;
    LISTPTR             u_list;
#endif
{
	short		i;
	STATUSPTR	sts, save;
	register CHARPTR chptr;

	sts = &(page->status);
	save = &(page->save);

	chptr = &(page->page[0]);
	for (i = 0; i < PAGEROWS * PAGECOLS; i++)
		*chptr++ = null_char;
	sts->row = save->row = 1;
	sts->col = save->col = 1;
	sts->cur_attr = save->cur_attr = _PLAIN;
	sts->c_attr = save->c_attr = MONO;
	sts->c_fade_attr = save->c_fade_attr = MONO;
	sts->fading = save->fading = _PLAIN;
	sts->gr_set = save->gr_set = 0;
	sts->wrap_on = save->wrap_on = FALSE;

	page->scroll_top = SCROLL_TOP;
	page->scroll_bot = SCROLL_BOT;

	/* if the screen is reset, any other updates will be erased so
	   overwrite them in the update list */
	u_list->index = 0;
	for (i = 1; i <= PAGEROWS; i++)
		NEW_UPD(u_list, PAGEROWS - i + 1, i, 1, PAGECOLS + 1);
	/* set up new entry so that subsequent characters get added at the
  	   home position */
	NEW_UPD(u_list, 1, sts->row, sts->col, sts->col);
	parse_ptr->state = INITSTATE;
	return (OK);
	
}

/*************************************************************************
*
* ROUTINE NAME	_ansi_rm()
*
* DESCRIPTION	Reset Mode - Alters bit-records of modes currently set.  
*		Test p_esc to check for private modes.
*
* RETURNS	OK
*
**************************************************************************/
#ifdef __STDC__
short _ansi_rm(
    register PAGEPTR    page,
    register char       *text,
    LISTPTR             u_list )
#else
short _ansi_rm( page, text, u_list )
    register PAGEPTR    page;
    register char       *text;
    LISTPTR             u_list;
#endif
{
	STATUSPTR	sts;

	sts = &(page->status);
	if (parse_ptr->special_esc != 0) {
	    switch(parse_ptr->special_esc) {
		case '?':
			if (parse_ptr->params[0] == 7) {
				sts->wrap_on = FALSE;
			}
			break;
		default:
			break;
	    }    
	}
	else {
	    switch(parse_ptr->params[0]) {
		case 0x07: sts->vem = FALSE;
			   break;
		case 0x10: sts->hem = FALSE;
			   break;
		default:
			   break;
	    }
	}
	parse_ptr->state = INITSTATE;
	return(OK);
}

/*************************************************************************
*
* ROUTINE NAME	_ansi_sm()
*
* DESCRIPTION	Alters bit-records of modes currently set.  
*		Must test p_esc to check for private modes. 
*
* RETURNS	OK.
*
**************************************************************************/
#ifdef __STDC__
short _ansi_sm(
    register PAGEPTR    page,
    register char       *text,
    LISTPTR             u_list )
#else
short _ansi_sm( page, text, u_list )
    register PAGEPTR    page;
    register char       *text;
    LISTPTR             u_list;
#endif
{
	STATUSPTR	sts;

	sts = &(page->status);
	if (parse_ptr->special_esc != 0) {
	    switch(parse_ptr->special_esc) {
		case '?':
			if (parse_ptr->params[0] == 7) {
				sts->wrap_on = TRUE;
			}
			break;
		default:
			break;
	    }    
	}
	else {
	    switch(parse_ptr->params[0]) {
		case 0x07: sts->vem = TRUE;
			   break;
		case 0x10: sts->hem = TRUE;
			   break;
		default:
			   break;
	    }
	}
	parse_ptr->state = INITSTATE;
	return(OK);
}

/*************************************************************************
*
* ROUTINE NAME	_ansi_decrc()
*
* DESCRIPTION	DEC private sequence to save cursor.
*
* RETURNS	OK.
*
**************************************************************************/
#ifdef __STDC__
short _ansi_decrc(
    register PAGEPTR    page,
    register char       *text,
    LISTPTR             u_list )
#else
short _ansi_decrc( page, text, u_list )
    register PAGEPTR    page;
    register char       *text;
    LISTPTR             u_list;
#endif
{
	register STATUSPTR sts;

	sts = &(page->status);
	*sts = page->save;
	NEW_UPD(u_list, 1, sts->row, sts->col, sts->col);
	parse_ptr->state = INITSTATE;
	return(OK);
}

/*************************************************************************
*
* ROUTINE NAME	_ansi_decsc()
*
* DESCRIPTION	DEC private sequence to save cursor.
*
* RETURNS	OK.
*
**************************************************************************/
#ifdef __STDC__
short _ansi_decsc(
    register PAGEPTR    page,
    register char       *text,
    LISTPTR             u_list )
#else
short _ansi_decsc( page, text, u_list )
    register PAGEPTR    page;
    register char       *text;
    LISTPTR             u_list;
#endif
{
	page->save = page->status;
	parse_ptr->state = INITSTATE;
	return(OK);
}

/*************************************************************************
*
* ROUTINE NAME	_ansi_scroll()
*
* DESCRIPTION	General scroll routine for scrolling any direction for
*		any scrolling region.
*
* RETURNS	ERROR if num of updates exceeds maximum, otherwise OK.
*
*************************************************************************/
#ifdef __STDC__
short _ansi_scroll(
    register PAGEPTR    page,
    LISTPTR             u_list,
	short   num,
	short   dir,
	short   row )
#else
short _ansi_scroll( page, u_list, num, dir, row )
    register PAGEPTR    page;
    LISTPTR             u_list;
	short   num;
	short   dir;
	short   row;
#endif
{
	short			num_chars;
	short			start;
	register short		scroll;
	register short		end;
	register short		j;
	register CHARPTR	from, to;

	if (num == 0)
		return(OK);

	/* make sure we have enough room in update list before modifying 
	   page image */
	if (page->scroll_bot - page->scroll_top + u_list->index + 2 >= 
	    u_list->max_updt) {
		return(ERROR);
	}

	switch (dir) {
	case S_DOWN: 
	    {
		start = offset(row, 1);

		num_chars = num * PAGECOLS;
		scroll = start + num_chars - 1;

		/* Move everything down, starting from the end of the region */
		j = offset(page->scroll_bot, END_OF_ROW);
		to = &(page->page[j]);
		from = &(page->page[j - num_chars]);
#ifdef TRACE
if (stdout != NULL)
	fprintf (stdout, "scrolling down j = %d, i = %d, end = %d, num_chars = %d\n",
	j,j - num_chars,scroll, num_chars);
#endif
		for (; j >= scroll; --j) {
			*to-- = *from--;
		}

#ifdef TRACE
if (stdout != NULL)
	fprintf (stdout, "done copying... fill from %d to %d\n",
	start, scroll);
#endif
		/* *** Fill inserted positions with blanks. *** */
		to = &(page->page[start]);
		for (j = start; j <= scroll; ++j) {
			*to++ = null_char;
		}

		/* modify the update list if only a part of the 
		   scrolling region is scrolled, or if the scrolling
		   region has never been scrolled before */
		if (row != page->scroll_top || parse_ptr->scrolled == FALSE) {
			/* modify update list */
			for (j = row; j <= page->scroll_bot; j++) {
				NEW_UPD(u_list, 1, j, 1, END_OF_ROW);
			}
		}
		if (row == page->scroll_top)
			parse_ptr->scrolled = TRUE;
		break;
	    }
	case S_UP:
	    {
		/* if requested to scroll more lines than are in
		   the scrolling region, reduce to size of scrolling
		    region */
		if (row + num > page->scroll_bot)
			num = page->scroll_bot - row + 1;

		num_chars = num * PAGECOLS;
		scroll = offset(page->scroll_bot, END_OF_ROW);

		/* Move everything up** */
		j = offset(row, 1);
		to = &(page->page[j]);
		from = &(page->page[j + num_chars]);
		end = scroll - num_chars;
#ifdef TRACE
if (stdout != NULL)
	fprintf (stdout, "scrolling up j = %d, i = %d, end = %d, num_chars = %d\n",
	j,j + num_chars,end, num_chars);
#endif
		for (; j <= end; j++) {
			*to++ = *from++; 
		}
#ifdef TRACE
if (stdout != NULL)
	fprintf (stdout, "done copying... fill from %d to %d\n",
	scroll - num_chars, scroll);
#endif

		/* *** Fill inserted positions with blanks. *** */
		j = scroll - num_chars;
		to = &(page->page[j]);
		for (; j < scroll; j++) {
			*to++ = null_char;
		}

		/* modify the update list if only a part of the 
		   scrolling region is scrolled, or if the scrolling
		   region has never been scrolled before */
		if (row != page->scroll_top || parse_ptr->scrolled == FALSE) {
			/* modify update list */
			for (j = row; j <= page->scroll_bot; j++) {
				NEW_UPD(u_list, 1, j, 1, END_OF_ROW);
			}
		}
		if (row == page->scroll_top)
			parse_ptr->scrolled = TRUE;
		break;
	    }
	case S_LEFT:
	case S_RIGHT:
	default:
		break;
	}

	return(OK);
}
