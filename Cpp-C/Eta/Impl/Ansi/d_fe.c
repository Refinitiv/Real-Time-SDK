/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2019 Refinitiv. All rights reserved.            --
 *|-----------------------------------------------------------------------------
 */


#include "ansi/q_ansi.h"
#include "ansi/decodeansi.h"
#include "ansi/ansi_int.h"


/*************************************************************************
*
* ROUTINE NAME	_ansi_sgr()
*
* DESCRIPTION	(Set graphics rendition).  Changes current character
*				attribute following the sequence of integer instructions
*				stored in the array params[] by taking exclusive or of
*				externally defined constants with the normal attribute.
*				If Rich private fading sequence is used, set fading
*				attribute accordingly.
*
* RETURNS	OK
*
**************************************************************************/
#ifdef __STDC__
short _ansi_sgr(
	register PAGEPTR    page,
	register char       *text,
	LISTPTR             u_list )
#else
short _ansi_sgr( page, text, u_list )
	register PAGEPTR    page;
	register char       *text;
	LISTPTR             u_list;
#endif
{
  	short 		i;		  /*  number of parameters  */
	unsigned char 	*attr;
	unsigned char 	*color;
	STATUSPTR	sts;

	sts = &(page->status);

	if (parse_ptr->special_esc == '?') {	  /* DEC private -- ignore */
		parse_ptr->state = INITSTATE;
		return(OK);
	}
  	if (parse_ptr->special_esc == '>') {  /*  Rich private fading attr */
    		attr = &(sts->fading);		  /* 1/23/87 */
		color = &(sts->c_fade_attr);
	}
	else {
		attr = &(sts->cur_attr);
		color = &(sts->c_attr);
	}

  	if (parse_ptr->param_cnt == 0) {     	/*  default to set current    */
    		setplain(*attr);     /*  attribute byte to normal  */
		*color = MONO;
	}

  	for (i = 0; i < parse_ptr->param_cnt; i++) {
		if (parse_ptr->params[i] >= 30 && parse_ptr->params[i] < 38) {
			*color &= 0xf0;
			*color += (parse_ptr->params[i] - 30);
#ifdef CDEBUG
if(stdout != NULL)
	fprintf (stdout, "setting fore to %d color: %x\n", parse_ptr->params[i], *color); 
#endif
			continue;
		}
		if (parse_ptr->params[i] >= 40 && parse_ptr->params[i] < 48) {
			*color &= 0x0f;
			*color += ((parse_ptr->params[i] - 40) << 4);
#ifdef CDEBUG
if(stdout != NULL)
	fprintf (stdout, "setting back to %d color: %x\n", parse_ptr->params[i], *color); 
#endif
			continue;
		}
    		switch (parse_ptr->params[i]) {
    			case 0:
				*color = MONO;
      				setplain(*attr);
      				break;
    			case 1:
				onbright(*attr);
      				break;
    			case 2:
      				ondim(*attr);
      				break;
    			case 4:
      				onundln(*attr);
      				break;
    			case 5:
      				onblink(*attr);
      				break;
    			case 7:
      				onrevvid(*attr);
      				break;
			case 22:
				offint(*attr);
				break;
			case 24:
				offundln(*attr);
				break;
			case 25:
				offblink(*attr);
				break;
			case 27:
				offrevvid(*attr);
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
* ROUTINE NAME	_ansi_db_p()
*
* DESCRIPTION	(Set double high/wide private sequence).
*		ESC[>#Z
*
* RETURNS	OK
*
**************************************************************************/
#ifdef __STDC__
short _ansi_db_p(
	register PAGEPTR    page,
	register char       *text,
	LISTPTR             u_list )
#else
short _ansi_db_p( page, text, u_list )
	register PAGEPTR    page;
	register char       *text;
	LISTPTR             u_list;
#endif
{
  	short 		i;		  /*  number of parameters  */
	unsigned char 	*attr;
	STATUSPTR	sts;

	sts = &(page->status);

  	if (parse_ptr->special_esc != '>') {  /*  Rich private ?*/
		parse_ptr->state = INITSTATE;
		return(OK);
	}
	else {
		attr = &(sts->cur_attr);
	}

  	for (i = 0; i < parse_ptr->param_cnt; i++) {
    		switch (parse_ptr->params[i]) {
    			case DBH_TP:
      				ondouble(*attr,DBH_TP);
      				break;
    			case DBH_BT:
      				ondouble(*attr,DBH_BT);
      				break;
    			case DBHW_TP:
      				ondouble(*attr,DBHW_TP);
      				break;
    			case DBHW_BT:
      				ondouble(*attr,DBHW_BT);
      				break;
    			case DB_OFF:
      				offdouble(*attr);
      				break;
    			case DBWD:
      				ondouble(*attr,DBWD);
      				break;
    			default:
      				offdouble(*attr);
      				break;
   		}
 	}
	parse_ptr->state = INITSTATE;
	return(OK);
}

/*************************************************************************
*
* ROUTINE NAME	_ansi_cret()
*
* DESCRIPTION	Moves cursor to the start of the current line. 
*
* RETURNS	ERROR if update list would overflow, else OK
*
**************************************************************************/
#ifdef __STDC__
short _ansi_cret(
	register PAGEPTR    page,
	register char       *text,
	LISTPTR             u_list )
#else
short _ansi_cret( page, text, u_list )
	register PAGEPTR    page;
	register char       *text;
	LISTPTR             u_list;
#endif
{
	page->status.col = 1;
	if (parse_ptr->cr_state != LF_STATE) {
	    NEW_UPD(u_list, 1, page->status.row, 1, 1);
	}
	else {
	    u_list->upd_list[u_list->index].upd_beg = 1;
	    u_list->upd_list[u_list->index].upd_end = 1;
	}
	parse_ptr->cr_state = CR_STATE;
	parse_ptr->state = INITSTATE;
	parse_ptr->wrap = FALSE;
	return (OK);
}

/*************************************************************************
*
* ROUTINE NAME	_ansi_lf()
*
* DESCRIPTION	This routine is called whenever the cursor is moved down
*				and scrolling should occur if necessary.  This routine
*				is used for processing new lines, but NOT for the CUD
*				sequence.
*
* RETURNS	ERROR if update list would overflow, else OK
*
**************************************************************************/
#ifdef __STDC__
short _ansi_lf(
	register PAGEPTR    page,
	register char       *text,
	LISTPTR             u_list )
#else
short _ansi_lf( page, text, u_list )
	register PAGEPTR    page;
	register char       *text;
	LISTPTR             u_list;
#endif
{
	register STATUSPTR	sts;

	sts = &(page->status);

	/* when moving the cursor down if we move down to bottom of
	   scrolling region, every move beyond should scroll the screen. */
	if (sts->row == page->scroll_bot) {
		if (_ansi_scroll(page, u_list, 1, S_UP, page->scroll_top) == ERROR){
			parse_ptr->state = INITSTATE;
			return (ERROR);
		}
		/* force new entry after scrolling */
		NEW_UPD(u_list, 1,sts->row, sts->col, sts->col);
		parse_ptr->cr_state = LF_STATE;
		parse_ptr->state = INITSTATE;
		parse_ptr->wrap = FALSE;
		return (OK);
	}
	if (sts->row == PAGEROWS) {
		/* cursor not in scrolling region, ignore lf */
		parse_ptr->state = INITSTATE;
		return (OK);
		}

	/* just move the cursor down */
	sts->row++;

	if (parse_ptr->cr_state != CR_STATE) {
	    NEW_UPD(u_list, 1,sts->row, sts->col, 
		sts->col);
	}
	else {
	    u_list->upd_list[u_list->index].row = sts->row;
	}
	parse_ptr->cr_state = LF_STATE;
	parse_ptr->state = INITSTATE;
	parse_ptr->wrap = FALSE;
	return (OK);
}

/*************************************************************************
*
* ROUTINE NAME	_ansi_ri()
*
* DESCRIPTION	Reverse index - move the cursor to the preceding line 
*				without changing the column position, scrolling if necessary.  
*
* RETURNS	ERROR if update list would overflow, else OK
*
**************************************************************************/
#ifdef __STDC__
short _ansi_ri(
	register PAGEPTR    page,
	register char       *text,
	LISTPTR             u_list )
#else
short _ansi_ri( page, text, u_list )
	register PAGEPTR    page;
	register char       *text;
	LISTPTR             u_list;
#endif
{
	register STATUSPTR	sts;

	sts = &(page->status);

	/* when moving the cursor up if we move up to top of
	   scrolling region, every move beyond should scroll the screen. */
	if (sts->row == page->scroll_top) {
		if (_ansi_scroll(page, u_list, 1, S_DOWN, sts->row) == ERROR){
			parse_ptr->state = INITSTATE;
			return (ERROR);
		}
		/* force new entry after scrolling */
		NEW_UPD(u_list, 1,sts->row, sts->col, sts->col);
		parse_ptr->state = INITSTATE;
		parse_ptr->wrap = FALSE;
		return (OK);
	}
	if (sts->row == 1) {
		/* cursor not in scrolling region, ignore ri */
		parse_ptr->state = INITSTATE;
		parse_ptr->wrap = FALSE;
		return (OK);
		}

	/* just move cursor up */
	sts->row-- ;
	 
	NEW_UPD(u_list, 1,sts->row, sts->col, 
		sts->col);
	parse_ptr->state = INITSTATE;
	parse_ptr->wrap = FALSE;
	return (OK);
}

/*************************************************************************
*
* ROUTINE NAME	_ansi_nel()
*
* DESCRIPTION	Makes new line, i.e., combination of move to start 
*				of line and  move to next lower line, scrolling if 
*				necessary. 
*
* RETURNS	ERROR if update list would overflow, else OK
*
**************************************************************************/
#ifdef __STDC__
short _ansi_nel(
	register PAGEPTR    page,
	register char       *text,
	LISTPTR             u_list )
#else
short _ansi_nel( page, text, u_list )
	register PAGEPTR    page;
	register char       *text;
	LISTPTR             u_list;
#endif
{
	page->status.col = 1;			/* do carriage return */
   	if (_ansi_lf(page, text, u_list) < 0)
	    return (ERROR);
	parse_ptr->wrap = FALSE;
	parse_ptr->state = INITSTATE;
	return (OK);
}


/*************************************************************************
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
#ifdef __STDC__
short _ansi_tab(
	register PAGEPTR    page,
	register char       *text,
	LISTPTR             u_list )
#else
short _ansi_tab( page, text, u_list )
	register PAGEPTR    page;
	register char       *text;
	LISTPTR             u_list;
#endif
{
    if (page->status.col <= TABCOLS)
	{
		page->status.col = TAB_TABLE[page->status.col - 1];
		NEW_UPD(u_list,1,page->status.row, page->status.col, page->status.col);
	}
	parse_ptr->state = INITSTATE;
	return (OK);
}
