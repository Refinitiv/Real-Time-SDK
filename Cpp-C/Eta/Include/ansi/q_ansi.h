/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2019 Refinitiv. All rights reserved.            --
 *|-----------------------------------------------------------------------------
 */

#ifndef Q_ANSI_H
#define Q_ANSI_H

#include "rtr/platform.h"



	/* define return values for qa_encode */
#define DONE		0
#define NOT_DONE	1

/* define ATTRIBUTES when monochrome, color, and fading attributes are
   needed.  Leaving ATTRIBUTES undefined saves space. */
#define ATTRIBUTES 

struct upd_type {		/* structure for incoming updates  */
	short	row;			/* row on which the update appears */
	short	upd_beg;		/* beginning column of update      */
	short	upd_end;		/* ending column of update	   */
};

typedef struct list_type {
	short		max_updt;	/* size of upd_list */
	short 		index;	/* num of entries in upd_list */
	struct upd_type upd_list[1];	/* this structure is overlaid on
					   the space the caller provides */
} LISTTYP, *LISTPTR;

typedef struct status {
	short 		row;
	short 		col;
	unsigned char 	cur_attr;		/* monochrome attribute */
	unsigned char 	c_attr;			/* color attribute */
	unsigned char 	c_fade_attr;		/* color fading attr */
	unsigned char 	fading;			/* mono fading attr */
	unsigned char 	gr_set;			/* flag to select G0 or G1 */
	unsigned char	G0_set;			/* G0 graphics set */
	unsigned char	G1_set;			/* G1 graphics set */
	short 		wrap_on;		/* flag for line wrap */
	short		vem;			/* flag for vert editing mode */
	short		hem;			/* flag for horiz edit mode */
} STATUSTYP, *STATUSPTR;

typedef struct char_type {
	char 		ch;
	unsigned char 	gs;		/* graphic set */
#ifdef ATTRIBUTES
	unsigned char 	attr;		/* mono attrib */
	unsigned char 	fade_attr;	/* mono fading */
	unsigned char 	c_attr;		/* color attrib */
	unsigned char 	c_fade_attr;	/* color fading */
#endif
} CHARTYP, *CHARPTR;


typedef struct page_type {
	CHARTYP 	*page; /* size of [PAGEROWS * PAGECOLS] */
	STATUSTYP 	status;
	STATUSTYP 	save;			/* for cursor save/restore */
	short		scroll_top;		/* top of scrolling region */
	short		scroll_bot;		/* bottom of scrolling reg */
	short		last_mod;		/* qa_encode marks last entry
						   in u_list it was able to
						   encode */
} PAGETYP, *PAGEPTR;

#define PAGELENGTH 	sizeof(PAGETYP)
#define CHARLENGTH	sizeof(CHARTYP)

/* ATTRIBUTE BYTE definitions */
#define	_PLAIN		0x00
#define	_BLINK		0x01
#define	_REVVID		0x02
#define	_DIM		0x04
#define	_UNDLN		0x08
#define _BRIGHT		0x10

/* definitions for color */
#define MONO		0xff	/* monochrome */
#define F_BLACK		0x00
#define F_RED		0x01
#define F_GREEN		0x02
#define F_YELLOW	0x03
#define F_BLUE		0x04
#define F_MAGENTA	0x05
#define F_CYAN		0x06
#define F_WHITE		0x07
#define B_BLACK		0x00
#define B_RED		0x10
#define B_GREEN		0x20
#define B_YELLOW	0x30
#define B_BLUE		0x40
#define B_MAGENTA	0x50
#define B_CYAN		0x60
#define B_WHITE		0x70

/* definitions for character size */
#define SHSW		0x00		/* single height single width */
#define DHSW_TOP	0x01		/* double height single width top */
#define DHSW_BOT	0x02		/* double height single width bottom */
#define DHDW_TOP	0x03		/* double height double width top */
#define DHDW_BOT	0x04		/* double height double width bottom */
#define SHDW		0x05		/* single height double width */

/* Modes for encoding ansi 
   UPDATE mode adds fading blink */
#define NO_FADE		0
#define NORM  		1
#define UPDATE  	2
#define ISSUPDT  	3

/*	define character set selection parameters for use with encode ansi
	routine.  These parameters are to be used to indicate character set
	or sets being used to create the page image.
*/
#define	US_ASCII	'B'
#define	UK_ASCII	'A'
#define	VT100_GS	'0'	/* vt100 graphics */
#define	CHAP_SPC	':'	/* chapdelaine special ascii */
#define	RMJ_SPEC	';'	/*    rmj	 "       "   */
#define	GARBAN_S	'<'	/*  garban	 "	 "   */
#define	MABON_SP	'='	/*  mabon	 "	 "   */
#define	MOSAIC_G	'>'	/* mosaic graphic set */
#define	FBI_ASCI	'?'	/* fbi special ascii */
#define	FBI_SPEC	'f'	/*  "     "    graphics */
#define	TOPIC_CS	't'	/* topic character set */
#define	GENERL_G	'g'	/* general graphics */
#define	VIEW_MOS	'v'	/* viewdata mosaics */



/*  escape sequence number and out sequnce table offsets*/
#define DBH_TP		1	/*double high top*/
#define DBH_BT		2	/*double high bottom*/
#define DBHW_TP		3	/*double high/wide top*/
#define DBHW_BT		4	/*double high/wide bottom*/
#define	DB_OFF		5	/*double off*/
#define DBWD		6	/*double wide single high*/
#define DB_MIN		1
#define DB_MAX		6

/*masks for bits in attribute sequence
	fddd aaaa		
	where f is 1 when double is active
	where d is the double attribute info
	where a is the normal attribute info
*/
#define DB_BITS		0xE0	/*mask out double flag bit*/
#define DB_SFT_BITS	0x07	/*mask out double flag bit after shifting DB_SHIFT*/
#define DB_SHIFT	5	/*number shifted right to right justify
				double attribute bits*/
#define DB_ATT_MASK	0x1f	/*normal attribute mask*/

#define BACK_COL_MASK 0xf0
#define FORG_COL_MASK 0x0f


#ifdef __cplusplus
extern "C"
{
#endif

extern short  PAGECOLS;
extern short  PAGEROWS;
extern short  END_OF_ROW;
extern short  SCROLL_BOT;
#define                             SCROLL_TOP      1
extern CHARTYP null_char;


#if defined(__cplusplus) || defined(__STDC__)

extern int qa_decode( PAGEPTR,char*,int,LISTPTR);
extern int qa_encode( PAGEPTR,unsigned char*,long,long*,short,LISTPTR);
extern short qa_page_rows();
extern short qa_page_columns();
extern short qa_end_of_row();
extern short qa_scroll_bot();
extern void qa_set_rows( short );
extern void qa_set_columns( short );
extern void qa_set_end_of_row( short );
extern void qa_set_scroll_bot( short );
extern short qa_reset(PAGEPTR,char*,LISTPTR);

#else

extern int qa_decode();
extern int qa_encode();
extern short qa_page_rows();
extern short qa_page_columns();
extern short qa_end_of_row();
extern short qa_scroll_bot();
extern void qa_set_rows();
extern void qa_set_columns();
extern void qa_set_end_of_row();
extern void qa_set_scroll_bot();
extern short qa_reset();

#endif

#ifdef __cplusplus
}
#endif


#endif
