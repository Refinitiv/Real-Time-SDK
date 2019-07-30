/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright Thomson Reuters 2015,2016,2018. All rights reserved.            --
 *|-----------------------------------------------------------------------------
 */

#ifndef ANSI_INT_H
#define ANSI_INT_H

#include "ansi/platform.h"

#ifdef __cplusplus
extern "C"
{
#endif

extern short 		PAGECOLS;
extern short 		PAGEROWS;
extern short 		END_OF_ROW;

	/* define default scrolling region on reset */
extern short   SCROLL_BOT;
extern	CHARTYP 	null_char;

extern	PARSEPTR	parse_ptr;


/* internally used function declarations */

#if defined(__cplusplus) || defined(__STDC__)
extern short _ansi_cuf(PAGEPTR,char*,LISTPTR);
extern short _ansi_cuu(PAGEPTR,char*,LISTPTR);
extern short _ansi_cud(PAGEPTR,char*,LISTPTR);
extern short _ansi_cub(PAGEPTR,char*,LISTPTR);
extern short _ansi_poscur(PAGEPTR,char*,LISTPTR);
extern short _ansi_decstbm(PAGEPTR,char*,LISTPTR);
extern short _ansi_ich(PAGEPTR,char*,LISTPTR);
extern short _ansi_el(PAGEPTR,char*,LISTPTR);
extern short _ansi_ed(PAGEPTR,char*,LISTPTR);
extern short _ansi_il(PAGEPTR,char*,LISTPTR);
extern short _ansi_dl(PAGEPTR,char*,LISTPTR);
extern short _ansi_dch(PAGEPTR,char*,LISTPTR);
extern short _ansi_su(PAGEPTR,char*,LISTPTR);
extern short _ansi_sd(PAGEPTR,char*,LISTPTR);
extern short _ansi_sgr(PAGEPTR,char*,LISTPTR);
extern short _ansi_db_p(PAGEPTR,char*,LISTPTR);
extern short _ansi_cret(PAGEPTR,char*,LISTPTR);
extern short _ansi_lf(PAGEPTR,char*,LISTPTR);
extern short _ansi_ri(PAGEPTR,char*,LISTPTR);
extern short _ansi_nel(PAGEPTR,char*,LISTPTR);
extern short _ansi_tab(PAGEPTR,char*,LISTPTR);
extern short _ansi_dG0(PAGEPTR,char*,LISTPTR);
extern short _ansi_dG1(PAGEPTR,char*,LISTPTR);
extern short _ansi_so(PAGEPTR,char*,LISTPTR);
extern short _ansi_si(PAGEPTR,char*,LISTPTR);
extern short _ansi_decdwl(PAGEPTR,char*,LISTPTR);
extern short _ansi_decdhl(PAGEPTR,char*,LISTPTR);
extern short _ansi_decswl(PAGEPTR,char*,LISTPTR);
extern short _ansi_sm(PAGEPTR,char*,LISTPTR);
extern short _ansi_rm(PAGEPTR,char*,LISTPTR);
extern short _ansi_decsc(PAGEPTR,char*,LISTPTR);
extern short _ansi_decrc(PAGEPTR,char*,LISTPTR);
extern short _ansi_scroll(PAGEPTR,LISTPTR,short,short,short);
extern short _ansi_nop(PAGEPTR,char*,LISTPTR);
extern short _ansi_stG0(PAGEPTR,char*,LISTPTR);
extern short _ansi_stG1(PAGEPTR,char*,LISTPTR);
extern short _ansi_esc(PAGEPTR,char*,LISTPTR);
extern short _ansi_csi(PAGEPTR,char*,LISTPTR);
extern short _ansi_pch(PAGEPTR,char*,LISTPTR);
extern short _ansi_param(PAGEPTR,char*,LISTPTR);
extern short _ansi_spesc(PAGEPTR,char*,LISTPTR);
extern short _ansi_chparm(PAGEPTR,char*,LISTPTR);
#else
extern short _ansi_cuf();
extern short _ansi_cuu();
extern short _ansi_cud();
extern short _ansi_cub();
extern short _ansi_poscur();
extern short _ansi_decstbm();
extern short _ansi_ich();
extern short _ansi_el();
extern short _ansi_ed();
extern short _ansi_il();
extern short _ansi_dl();
extern short _ansi_dch();
extern short _ansi_su();
extern short _ansi_sd();
extern short _ansi_sgr();
extern short _ansi_db_p();
extern short _ansi_cret();
extern short _ansi_lf();
extern short _ansi_ri();
extern short _ansi_nel();
extern short _ansi_tab();
extern short _ansi_dG0();
extern short _ansi_dG1();
extern short _ansi_so();
extern short _ansi_si();
extern short _ansi_decdwl();
extern short _ansi_decdhl();
extern short _ansi_decswl();
extern short _ansi_sm();
extern short _ansi_rm();
extern short _ansi_decsc();
extern short _ansi_decrc();
extern short _ansi_scroll();
extern short _ansi_nop();
extern short _ansi_stG0();
extern short _ansi_stG1();
extern short _ansi_esc();
extern short _ansi_csi();
extern short _ansi_pch();
extern short _ansi_param();
extern short _ansi_spesc();
extern short _ansi_chparm();

#endif



#ifdef __cplusplus
}
#endif


#endif
