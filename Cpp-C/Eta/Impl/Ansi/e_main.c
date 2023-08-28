/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|          Copyright (C) 2019-2020 Refinitiv. All rights reserved.          --
 *|-----------------------------------------------------------------------------
 */

/*************************************************************************
*
* ROUTINE NAME	qa_encode()
*
* DESCRIPTION	This routine takes a stored page image and encodes it
*		into ANSI X3.64 protocol.
*		
*		Parameters:
*		  pageptr -	pointer to the stored page image
*		  str -		pointer to buffer for ANSI text
*		  maxstrlen -	size of str buffer
*		  len -		pointer to length str after encoding
*		  fade_enable -	flag to enable fading
*		  u_list -	update list pointer whose data structure
*				contains a list of all the blocks that
*				need to be encoded.
*				
*		qa_encode processes each entry in the update list up to 
*		the index.  For the row and start column specified
*		in the update list to (but not including) the end column,
*		the corresponding characters and attributes in the page
*		image are added to str.
*
* RETURNS	If qa_encode is able to encode all of the data regions 
*		defined by the update list into str, it returns DONE.
*
*		If qa_encode runs out of room in str before the image is
*		completely encoded, it returns NOT_DONE.  last_mod in the 
*		page image is set to the entry in the update list where
*		qa_encode will resume encoding the next time it is called.
*		qa_encode may be called as many times as necessary to
*		completely encode all the regions defined in the update list.
*
*		The memory pointed to by len contains the length of str in
*		either case.
*		
*************************************************************************/

#include <stdio.h>
#include <ctype.h>
#include "ansi/q_ansi.h"
#include "ansi/decodeansi.h"
#include "ansi/ansi_int.h"
#include "ansiVersion.h"

#if defined(_WIN16)
#define	stderr	(FILE*)0	/* no message will be displayed */
#endif
/*
#define TRACE
*/

#define ESCCURSOROFF	"\033[?25l"
#define ESCSCREENWRAP   "\033[?7h"
#define ESCWRAPOFF	"\033[?7l"
#define ESCFADEBLINK	"\033[>5m"
#define ESCFADEOFF	"\033[>0m"
#define ESCRESETSCR     "\033c"
#define ESCPLAIN 	"\033[m"
#define ESCCLRLINE      "\033[2K"

/* the following are used to build up an attribute change escape sequence */
#define CSI_FADE	"\033[>"
#define CSI			"\033["
#define ENDSET		"m"

#define ESCMVCURS	"\033[%d;%dH"

#define	ESC_G0_SET	"\033("
#define	ESC_G1_SET	"\033)"
#define	USE_G0		'\017'
#define	USE_G1		'\016'

/* define tens and ones arrays to speed building of escape sequence for
   moving the cursor */
char tens[] = { '0',
	'0','0','0','0','0','0','0','0','0','1',
	'1','1','1','1','1','1','1','1','1','2',
	'2','2','2','2','2','2','2','2','2','3',
	'3','3','3','3','3','3','3','3','3','4',
	'4','4','4','4','4','4','4','4','4','5',
	'5','5','5','5','5','5','5','5','5','6',
	'6','6','6','6','6','6','6','6','6','7',
	'7','7','7','7','7','7','7','7','7','8'
};

char ones[] = { '0',
	'1','2','3','4','5','6','7','8','9','0',
	'1','2','3','4','5','6','7','8','9','0',
	'1','2','3','4','5','6','7','8','9','0',
	'1','2','3','4','5','6','7','8','9','0',
	'1','2','3','4','5','6','7','8','9','0',
	'1','2','3','4','5','6','7','8','9','0',
	'1','2','3','4','5','6','7','8','9','0',
	'1','2','3','4','5','6','7','8','9','0'
};

#ifdef ATTRIBUTES
/* define foreground colors */
static char *f_col_map[] = { ";30", ";31", ";32", ";33", 
			     ";34", ";35", ";36", ";37"}; 
/* define background colors */
static char *b_col_map[] = { ";40", ";41", ";42", ";43", 
			     ";44", ";45", ";46", ";47"}; 
static char *attr_map[] = {	"0",		/*plain */
				"0;5",		/* blink */
				"0;7",		/* rev vid */
				"0;5;7",	/* blink rev */
				"0;2",		/* dim */
				"0;2;5",	/* dim blink */
				"0;2;7",	/* dim rev */
				"0;2;5;7",	/* dim blink rev */
				"0;4",		/* underline */
				"0;4;5",	/* under blink */
				"0;4;7",	/* under rev */
				"0;4;5;7",	/* under blink rev */
				"0;2;4",	/* under dim */
				"0;2;4;5",	/* under dim blink */
				"0;2;4;7",	/* under dim rev */
				"0;2;4;5;7",	/* under dim blink rev */
				"0;1",		/* brt */
				"0;1;5",	/* brt blink */
				"0;1;7",	/* brt rev vid */
				"0;1;5;7",	/* brt blink rev */
				"0;1;2",	/* brt dim */
				"0;1;2;5",	/* brt dim blink */
				"0;1;2;7",	/* brt dim rev */
				"0;1;2;5;7",	/* brt dim blink rev */
				"0;1;4",	/* brt underline */
				"0;1;4;5",	/* brt under blink */
				"0;1;4;7",	/* brt under rev */
				"0;1;4;5;7",	/* brt under blink rev */
				"0;1;2;4",	/* brt under dim */
				"0;1;2;4;5",	/* brt under dim blink */
				"0;1;2;4;7",	/* brt under dim rev */
				"0;1;2;4;5;7" };/* brt under dim blink rev */


/*sequences for private double sequences for TOPIC*/
static char *double_seq[] = {	"\033[>5Z",     /*illegal - def single high wide*/
				"\033[>1Z",	/*Double height top*/
				"\033[>2Z",	/*Double height bottom*/
				"\033[>3Z",	/*Double height/wide top*/
				"\033[>4Z",	/*Double hight/wide bottom*/
				"\033[>5Z",	/*single high wide*/
				"\033[>6Z",	/*Double wide singel high*/
				"\033[>5Z",	/*illegal - def single high wide*/
				"\033[>5Z" };	/*illegal - def single high wide*/

#endif

#ifdef __STDC__
long _ansi_addstr(unsigned char*,char* );
int _ansi_wrpos(unsigned char*,unsigned char,unsigned char );
#else
long _ansi_addstr();
int _ansi_wrpos();
#endif


#ifdef __STDC__
int qa_encode(
	register PAGEPTR	 	pageptr,
	register unsigned char 	*str,
	long 					maxstrlen,
	long					*len,
	short					fade_enable,
	LISTPTR					u_list )
#else
int qa_encode( pageptr , str, maxstrlen, len, fade_enable, u_list )
	register PAGEPTR        pageptr;
	register unsigned char  *str;
	long                    maxstrlen;
	long                    *len;
	short                   fade_enable;
	LISTPTR                 u_list;
#endif
{
	unsigned char*		start_str;
	short 			stop;

	register CHARPTR 	chptr;
	register long 		page_pos;
	register char 		r_gs_flag;
	char 			c_G0, c_G1;
#ifdef ATTRIBUTES
	char 			changes;
	unsigned char		fading;		/* 01/26/87 */
	unsigned char 		pr_color;	/* prev color 01/07/88 */
	unsigned char 		pr_f_color;	/* prev fading color 01/07/88 */
	unsigned char 		fore;		/* temp foreground color */
	unsigned char 		back;		/* temp background color */
	register unsigned char	cur_attr;
	unsigned char		db_attr;	/*double flag, etc BF 10/92*/
	int 			db_flag;
	char			sv_pr_color;	/*saving state for skip space code*/
	unsigned char		sv_cur_attr;
	char			sv_fading;
	char			sv_pr_f_color;
#endif
	char			sv_r_gs_flag;
	unsigned char		*spc_start;	/*save location where spaces start*/
	CHARPTR 		chptr_start;
	int			skip_spc;	/*BF 10/92 flag indicating spaces with
						  no attr or color can be skipped because
						  the line has been cleared*/
	register short		mod_count;	/* 03/09/87 */
	register short 		col;		/* 03/03/87 */
	register struct upd_type *modptr;
	short			mvflag;		/* 09/06/88 */
	short			whole_line_flag, save_col; /* 8/23/93 modify space saving strategy */


	/* initially start index at 0 and put in initial escsequences
		if it is an update */
	if(maxstrlen < sizeof(ESCCURSOROFF) + sizeof(ESCSCREENWRAP) +
			sizeof(ESCFADEBLINK) + sizeof(ESCMVCURS) + 
			sizeof(ESCFADEOFF) + sizeof(ESCPLAIN) + 1 +
			sizeof(ESC_G0_SET) + 2 + MAX_SINGLE_ANSI_SEQUENCE) {
		if (stderr != NULL)
			fprintf(stderr, "encode_ansi: maxstrlen %ld  not long enough \n",
		     maxstrlen);
		str[0] = '\0';
		return(ERROR);
	}
#ifdef ATTRIBUTES
	db_flag = 0;
	sv_cur_attr = 0;
#endif

	start_str = str;
	str += _ansi_addstr(str, ESCCURSOROFF);
	str += _ansi_addstr(str, ESCSCREENWRAP);
	str += _ansi_addstr(str, ESCPLAIN);

	/* if G0 and G1 specified load proper ansi sequences	*/
	if ((c_G0 = pageptr->status.G0_set) != '\0') {
		str += _ansi_addstr(str, ESC_G0_SET);
		*str++ = c_G0;
		}
	
	if ((c_G1 = pageptr->status.G1_set) != '\0') {
		str += _ansi_addstr(str, ESC_G1_SET);
		*str++ = c_G1;
		}
	/* use invalid Graphics set designation (Delete) to force
	a graphic set selection on first character	*/
	r_gs_flag = '\177';

	/* starting at the current cursor position start encoding */
#ifdef ATTRIBUTES
	cur_attr  = _PLAIN;
	fading = _PLAIN;
	pr_color = MONO;
	pr_f_color = MONO;
#endif
	stop = FALSE;
#ifdef TRACE
if(stderr != NULL)
{
	fprintf (stderr,"maxstrlen before loop = %ld\n",maxstrlen);
	fprintf (stderr,"address of data array = %ld\n",pageptr);
}
#endif 
    /* make maxstrlen the max allowable string size */
    maxstrlen -= (long)(MAX_SINGLE_ANSI_SEQUENCE + strlen(ESCFADEOFF) + 3);
    if (pageptr->last_mod != -1) 
	mod_count = pageptr->last_mod;
    else
	mod_count = 0;

    for (; mod_count <= u_list->index && !stop; mod_count++) {

	modptr = &u_list->upd_list[mod_count];
	col = modptr->upd_beg;
	page_pos = offset (modptr->row, col);
#ifdef TRACE
if (stdout != NULL)
	fprintf (stdout, "mod_count =  %d, col = %d, row = %d, page_pos = %d\n", 
		mod_count, col, modptr->row, page_pos);
#endif

	/* if this is on last line, turn off auto-wrap */
	if (modptr->row == PAGEROWS) {
		str += _ansi_addstr(str, ESCWRAPOFF);
	}
	/*BF 10/92 skip spaces if whole line being rewitten*/
	whole_line_flag = FALSE;
	if (modptr->upd_beg == 1 && modptr->upd_end == END_OF_ROW){
		skip_spc = 1;
		whole_line_flag = TRUE;
	} else {
		skip_spc = 0;
	}
	mvflag = TRUE;		/* mark that we should move the cursor */

	spc_start = NULL;

	for (; col < modptr->upd_end; col++, page_pos++) {

		chptr = &(pageptr->page[page_pos]);
		if (chptr->ch == '\0') {
			/* skip nulls, but mark that we should move cursor */
			mvflag = TRUE;
			continue;
		/*BF 10/92 skip spaces logic*/
		} else if(skip_spc && spc_start == NULL && chptr->ch == ' '
#ifdef ATTRIBUTES
			  && !chptr->attr && (chptr->c_attr == MONO ||
			    !(chptr->c_attr & BACK_COL_MASK))
#endif
		){
			spc_start = str;	/*save state*/
			chptr_start = chptr;
#ifdef ATTRIBUTES
			sv_cur_attr = cur_attr;
			sv_pr_color = pr_color;
			sv_fading = fading;
			sv_pr_f_color = pr_f_color;
#endif
			sv_r_gs_flag = r_gs_flag;
		} else if(spc_start != NULL && (chptr->ch != ' ' 
#ifdef ATTRIBUTES
			  || chptr->attr || (chptr->c_attr != MONO && 
			  (chptr->c_attr & BACK_COL_MASK))
#endif
		)){
			if((chptr - chptr_start) > 10){
				str = spc_start;
#ifdef ATTRIBUTES
				cur_attr = sv_cur_attr;
				sv_cur_attr = 0;
				pr_color = sv_pr_color;
				fading = sv_fading;
				pr_f_color = sv_pr_f_color;
				if (cur_attr & DB_BITS)
					db_flag = 1;
#endif
				r_gs_flag = sv_r_gs_flag;
				mvflag = TRUE;
			}
			spc_start = NULL;
		}

        /* if we might run out of buffer space, stop and return. */
        if (whole_line_flag)
        {
            if ( ((long)(str - start_str) + sizeof(ESCCLRLINE)) >= maxstrlen )
            {
#ifdef TRACE
if (stdout != NULL)
            fprintf (stdout, " ran out of buffer space after %lu chars\n",
                (str - start_str));
#endif
                    stop = TRUE;
                    break;
            }
        }
        else
        {
            if ( ((long)(str - start_str)) >= maxstrlen )
            {
#ifdef TRACE
if (stdout != NULL)
            fprintf (stdout, " ran out of buffer space after %lu chars\n",
                (str - start_str));
#endif
                    stop = TRUE;
                    break;
            }
        }

		if (whole_line_flag) {
				save_col = col;
				col = 1;
		        str += _ansi_wrpos(str, (unsigned char)modptr->row, (unsigned char)col);
		        str += _ansi_addstr(str, ESCCLRLINE);
				col = save_col;
				mvflag = TRUE;
				whole_line_flag = FALSE;
		}

		if (mvflag == TRUE) {
			str += _ansi_wrpos(str, (unsigned char)modptr->row, 
			    (unsigned char)col);
			mvflag = FALSE;
			}

#ifdef TRACE2
if (stderr != NULL)
	fprintf(stderr, "in = %d pg = %d ch = %d \n", 
			str - start_str, page_pos, chptr->ch);
#endif

		/* determine if a change of graphics sets is required; 
		   check if currently invoked set is desired for next char */
		if (r_gs_flag != chptr->gs && (r_gs_flag != US_ASCII ||
		    isprint(chptr->gs))) {
			r_gs_flag = chptr->gs;
			/* if graphic set undefined, set to US_ASCII */
			if (!isprint(r_gs_flag))
			     r_gs_flag = US_ASCII;
			if (r_gs_flag == c_G0) {
				/* G0 set already designated properly,
				only a shift is required. */
				*str++ =  USE_G0;
			}
			else {
				if (r_gs_flag == c_G1) {
					/* G0 set already designated
					properly, only a shift is
					required. */
					*str++ =  USE_G1;
				}
				else {
					if (c_G0 != '\0') {
					    /* set not presently design-
					    ated, invoke as G1 set */
					    c_G1 = r_gs_flag;
					    str += _ansi_addstr(str, ESC_G1_SET);
					    *str++ =  c_G1;
					    *str++ =  USE_G1;
					}
					else {
					    /* no pre-determined G0 set,
					    assume first set required is
					    the most often used */
					    c_G0 = r_gs_flag;
					    str += _ansi_addstr(str, ESC_G0_SET);
					    *str++ =  c_G0;
					    *str++ =  USE_G0;
					}
				}
			}
		}

#ifdef ATTRIBUTES
		/* determine changes needed in tflag then update cur_attr */
		changes = cur_attr ^ chptr->attr;
		cur_attr = chptr->attr;

		/* check to see if any attributes changed */
		if (!no_attr_ht(changes) || (chptr->c_attr != pr_color)) {
			str += _ansi_addstr(str, CSI);
			if (cur_attr < 32) {
				if(db_flag)
					db_attr = DB_OFF;
				str += _ansi_addstr(str, attr_map[att_bits(cur_attr)]);
			}
			/*double high/wide flag set*/
			else if (cur_attr & DB_BITS){
				db_flag = 1;
				/*determine sequence to send*/
				db_attr = sdb_bits(cur_attr >> DB_SHIFT);
				/*add normal attribute bits as above*/
				str += _ansi_addstr(str, attr_map[att_bits(cur_attr)]);
			} else {
				str += _ansi_addstr(str, attr_map[0]);
				if (stderr != NULL)
					fprintf (stderr, "bad attr 0x%x at pos %ld\n",
				    cur_attr, page_pos);
			}
	
			fore = chptr->c_attr & 0x0f;
			back = chptr->c_attr >> 4;
#ifdef CDEBUG
if (stdout != NULL)
	fprintf (stdout, "encode: color 0x%x fore %d back %d\n", chptr->c_attr, fore, back);
#endif
			if (fore != 0x0f)
			    str += _ansi_addstr (str, f_col_map[fore]);
			if (back != 0x0f)
			    str += _ansi_addstr (str, b_col_map[back]);
			pr_color = chptr->c_attr;
		
			/* print end of set */
			str += _ansi_addstr(str, ENDSET);

			if(db_flag){		/*output double string BF 10/92*/
				if(db_attr < 7)
					str += _ansi_addstr(str,double_seq[db_attr]);
				if(db_attr == DB_OFF)
					db_flag = 0;
			}
		}

		/* if fading disabled, add char and return */
		if (!fade_enable) {
			*str++ = chptr->ch;
			continue;
		}
		/* determine changes needed in fading */
		changes = fading ^ chptr->fade_attr;
		fading = chptr->fade_attr;
		if (no_attr_ht(changes) && (chptr->c_fade_attr == pr_f_color)) {
			/* if nothing changed add the character and return */
			*str++ = chptr->ch;
			continue;
		}
		/* some attributes changed so reset the fading */
		str += _ansi_addstr(str, CSI_FADE);
		if (chptr->fade_attr < 32) {
			str += _ansi_addstr(str, attr_map[chptr->fade_attr]);
		}
		else {
			str += _ansi_addstr(str, attr_map[0]);
			if (stderr != NULL)
				fprintf (stderr, "bad attr 0x%x at pos %ld\n",
			    chptr->fade_attr, page_pos);
		}

		fore = chptr->c_fade_attr & 0x0f;
		back = chptr->c_fade_attr >> 4;
#ifdef CDEBUG
if (stdout != NULL)
	fprintf (stdout, "fading encode: color 0x%x fore %d back %d\n", 
		chptr->c_fade_attr, fore, back);
#endif
		if (fore != 0x0f)
		    str += _ansi_addstr (str, f_col_map[fore]);
		if (back != 0x0f)
		    str += _ansi_addstr (str, b_col_map[back]);
		pr_f_color = chptr->c_fade_attr;
	
		str += _ansi_addstr(str, ENDSET);
#endif
		/* generate the character and return */
        	*str++ = chptr->ch;
	
	    }
	    if(spc_start != NULL){		/*EOL ?? BF 10/92*/
		if((chptr - chptr_start) > 10){
			str = spc_start;
#ifdef ATTRIBUTES
			cur_attr = sv_cur_attr;
			sv_cur_attr = 0;
			pr_color = sv_pr_color;
			fading = sv_fading;
			pr_f_color = sv_pr_f_color;
			if (cur_attr & DB_BITS)
				db_flag = 1;
#endif
			r_gs_flag = sv_r_gs_flag;
		}
		spc_start = NULL;
	    }
	}

#ifdef TRACE
if (stderr != NULL)
{
	fprintf(stderr, "index = %d\n",str - start_str);
	fprintf(stderr, "page_pos = %d\n", page_pos);
}
#endif 
#ifdef ATTRIBUTES
	if (cur_attr & DB_BITS || sv_cur_attr & DB_BITS){/*if any double sequences sent*/
		str += _ansi_addstr(str,double_seq[DB_OFF]);
	}
#endif


	str += _ansi_addstr(str, ESCFADEOFF);

	*str = '\0';

	if(!stop) {
		*len = (long)(str - start_str);
		return(DONE);
	}
	else {
		/* redo the last modified field */
		pageptr->last_mod = mod_count - 1;
		*len = (long)(str - start_str);
		return(NOT_DONE);
	}
	
} /* qa_encode */


 /*
 * DESCRIPTION: This function adds a string to a buffer and increments the 
 * indexptr accordingly.
 * MODIFICATIONS:
 */
#ifdef __STDC__
long _ansi_addstr(
	register unsigned char *buffer,
	register char *str )
#else
long _ansi_addstr( buffer, str )
	register unsigned char *buffer;
	register char *str;
#endif
{
	char*	str_beg;

	str_beg = str;
	while (*str != '\0') {
		*buffer++ = *(unsigned char *)str++;
	}

	return(long)(str - str_beg);
}	

#ifdef __STDC__
int _ansi_wrpos(
	register unsigned char *str,
	register unsigned char row,
	register unsigned char col )
#else
int _ansi_wrpos( str, row, col )

	register unsigned char *str;
	register unsigned char row;
	register unsigned char col;

#endif

{

	*str++ = '\033';
	*str++ = '[';
	*str++ = tens[row];
	*str++ = ones[row];
	*str++ = ';';
	*str++ = tens[col];
	*str++ = ones[col];
	*str++ = 'H';
	return(8);
}
