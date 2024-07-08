/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2019 LSEG. All rights reserved.                 --
 *|-----------------------------------------------------------------------------
 */


#ifndef DECODEANSI_H
#define DECODEANSI_H

#define offset(row,col)		((((row) - 1) * PAGECOLS) + ((col) - 1))
#define addch(BUFFER, INDXPTR, CH) { BUFFER[*INDXPTR] = CH; ++(*INDXPTR); }

#define TRUE	1
#define FALSE	0

/* define scrolling directions */
#define S_UP	0
#define S_DOWN	1
#define S_LEFT	2
#define S_RIGHT	3

#define MAX_SINGLE_ANSI_SEQUENCE 30
#define MAXCSIPARAMS		 20	/* max params in CSI sequence */

#define	isplain(attribs)	((attribs & DB_ATT_MASK) == (_PLAIN))
#define	no_attr_ht(attribs)	((attribs) == (_PLAIN))
#define	isrevvid(attribs)	((attribs) & (_REVVID))
#define	isundln(attribs)	((attribs) & (_UNDLN))
#define	isdim(attribs)		((attribs) & (_DIM))
#define	isblink(attribs)	((attribs) & (_BLINK))
#define	isbright(attribs)	((attribs) & (_BRIGHT))
#define	onplain(gflag)		((gflag) |= (_PLAIN))
#define	onrevvid(gflag)		((gflag) |= (_REVVID))
#define	onundln(gflag)		((gflag) |= (_UNDLN))
#define	ondim(gflag)		((gflag) |= (_DIM))
#define	onblink(gflag)		((gflag) |= (_BLINK))
#define	onbright(gflag)		((gflag) |= (_BRIGHT))
#define	offplain(gflag)		((gflag) &= ~(_PLAIN))
#define	offrevvid(gflag)	((gflag) &= ~(_REVVID))
#define	offundln(gflag)		((gflag) &= ~(_UNDLN))
#define	offint(gflag)		((gflag) &= ~(_DIM + _BRIGHT))
#define	offblink(gflag)		((gflag) &= ~(_BLINK))
#define	setplain(gflag)		((gflag) &= (_PLAIN | DB_BITS))
#define	setrevvid(gflag)	onrevvid(gflag);((gflag) &= (_REVVID))
#define	setundln(gflag)		onundln(gflag);((gflag) &= (_UNDLN))
#define	setdim(gflag)		ondim(gflag);((gflag) &= (_DIM))
#define	setblink(gflag)		onblink(gflag);((gflag) &= (_BLINK))
#define	setbright(gflag)	onbright(gflag);((gflag) &= (_BRIGHT))

#define ondouble(gflag,val)	(gflag) &= DB_ATT_MASK;((gflag) |= (val << DB_SHIFT))
#define offdouble(gflag)	((gflag) &= ~(DB_BITS))
#define db_bits(gflag)		((gflag) & DB_BITS)
#define sdb_bits(gflag)		((gflag) & DB_SFT_BITS)
#define att_bits(gflag)		((gflag) & DB_ATT_MASK)

#define ESC 	'\033'
#define SPACE	' '
#define ERROR	-1
#define OK	1		/* number of characters processed */


#define INITSTATE 	0
#define ESCSTATE	1
#define CSISTATE	2
#define G0STATE		3
#define G1STATE		4
#define SIZESTATE	5

#define CR_STATE	20
#define LF_STATE	21

typedef struct parse_status {
	unsigned short	txt_lngth;		/* len of text to be decoded */
	short 		param_cnt;			/* number of parameters */
	short 		params[MAXCSIPARAMS];
	short 		state;				/* current state */
	short 		cr_state;
	short		wrap;				/* mark if we should wrap next*/
	short		scrolled;			/* set if region has scrolled */
	char  		special_esc;		/* private sequence */
} PARSETYP, *PARSEPTR;

/* define the table for tabulation.  Each entry corresponds to the next
   tab stop in the sequence.  If changeable tab stops are implemented,
   this table will have to become part of the page image.   */
#define TABCOLS 80
static char TAB_TABLE[TABCOLS] = {
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

/* create a new entry in update list.  req is the number of subsequent
   entries that will be required to completely parse this escape sequence */
#define NEW_UPD(upd, req, Row, Beg, End)   {		\
	if (upd->index >= upd->max_updt - (req))	\
		return (ERROR);				\
	upd->index++;				\
	upd->upd_list[upd->index].row = Row;	\
	upd->upd_list[upd->index].upd_beg = Beg;	\
	upd->upd_list[upd->index].upd_end = End;	\
	}

#endif
