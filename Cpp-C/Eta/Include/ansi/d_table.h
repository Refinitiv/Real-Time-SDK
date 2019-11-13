/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2019 Refinitiv. All rights reserved.            --
 *|-----------------------------------------------------------------------------
 */


#ifndef D_TABLE_H
#define D_TABLE_H



typedef short(* FUNC)();

#ifdef ATTRIBUTES
FUNC _ansi_do_decode[6][128] = {
/* INIT STATE */
{
_ansi_nop,_ansi_nop,_ansi_nop,_ansi_nop,_ansi_nop,_ansi_nop,_ansi_nop,_ansi_nop,	/* 00 - 07 */
_ansi_cub,_ansi_tab,_ansi_lf,_ansi_nop,_ansi_nop,_ansi_cret,_ansi_so,_ansi_si,		/* 08 - 0f */
_ansi_nop,_ansi_nop,_ansi_nop,_ansi_nop,_ansi_nop,_ansi_nop,_ansi_nop,_ansi_nop,	/* 10 - 17 */
_ansi_nop,_ansi_nop,_ansi_nop,_ansi_esc,_ansi_nop,_ansi_nop,_ansi_nop,_ansi_nop,	/* 18 - 1f */
_ansi_pch,_ansi_pch,_ansi_pch,_ansi_pch,_ansi_pch,_ansi_pch,_ansi_pch,_ansi_pch,	/* 20 - 27 */
_ansi_pch,_ansi_pch,_ansi_pch,_ansi_pch,_ansi_pch,_ansi_pch,_ansi_pch,_ansi_pch,	/* 28 - 2f */
_ansi_pch,_ansi_pch,_ansi_pch,_ansi_pch,_ansi_pch,_ansi_pch,_ansi_pch,_ansi_pch,	/* 30 - 37 */
_ansi_pch,_ansi_pch,_ansi_pch,_ansi_pch,_ansi_pch,_ansi_pch,_ansi_pch,_ansi_pch,	/* 38 - 3f */
_ansi_pch,_ansi_pch,_ansi_pch,_ansi_pch,_ansi_pch,_ansi_pch,_ansi_pch,_ansi_pch,	/* 40 - 47 */
_ansi_pch,_ansi_pch,_ansi_pch,_ansi_pch,_ansi_pch,_ansi_pch,_ansi_pch,_ansi_pch,	/* 48 - 4f */
_ansi_pch,_ansi_pch,_ansi_pch,_ansi_pch,_ansi_pch,_ansi_pch,_ansi_pch,_ansi_pch,	/* 50 - 57 */
_ansi_pch,_ansi_pch,_ansi_pch,_ansi_pch,_ansi_pch,_ansi_pch,_ansi_pch,_ansi_pch,	/* 58 - 5f */
_ansi_pch,_ansi_pch,_ansi_pch,_ansi_pch,_ansi_pch,_ansi_pch,_ansi_pch,_ansi_pch,	/* 60 - 67 */
_ansi_pch,_ansi_pch,_ansi_pch,_ansi_pch,_ansi_pch,_ansi_pch,_ansi_pch,_ansi_pch,	/* 68 - 6f */
_ansi_pch,_ansi_pch,_ansi_pch,_ansi_pch,_ansi_pch,_ansi_pch,_ansi_pch,_ansi_pch,	/* 70 - 77 */
_ansi_pch,_ansi_pch,_ansi_pch,_ansi_pch,_ansi_pch,_ansi_pch,_ansi_pch,_ansi_nop},	/* 78 - 7f */

/* ESCAPE STATE */
{
_ansi_nop,_ansi_nop,_ansi_nop,_ansi_nop,_ansi_nop,_ansi_nop,_ansi_nop,_ansi_nop,	/* 00 - 07 */
_ansi_nop,_ansi_nop,_ansi_nop,_ansi_nop,_ansi_nop,_ansi_nop,_ansi_nop,_ansi_nop,	/* 08 - 0f */
_ansi_nop,_ansi_nop,_ansi_nop,_ansi_nop,_ansi_nop,_ansi_nop,_ansi_nop,_ansi_nop,	/* 10 - 17 */
_ansi_nop,_ansi_nop,_ansi_nop,_ansi_esc,_ansi_nop,_ansi_nop,_ansi_nop,_ansi_nop,	/* 18 - 1f */
_ansi_nop,_ansi_nop,_ansi_nop,_ansi_nop,_ansi_nop,_ansi_nop,_ansi_nop,_ansi_nop,	/* 20 - 27 */
_ansi_stG0,_ansi_stG1,_ansi_nop,_ansi_nop,_ansi_nop,_ansi_nop,_ansi_nop,_ansi_nop,	/* 28 - 2f */
_ansi_nop,_ansi_nop,_ansi_nop,_ansi_nop,_ansi_nop,_ansi_nop,_ansi_nop,_ansi_decsc,	/* 30 - 37 */
_ansi_decrc,_ansi_nop,_ansi_nop,_ansi_nop,_ansi_nop,_ansi_nop,_ansi_nop,_ansi_nop,	/* 38 - 3f */
_ansi_nop,_ansi_nop,_ansi_nop,_ansi_nop,_ansi_lf,_ansi_nel,_ansi_nop,_ansi_nop,		/* 40 - 47 */
_ansi_nop,_ansi_nop,_ansi_nop,_ansi_nop,_ansi_nop,_ansi_ri,_ansi_nop,_ansi_nop,		/* 48 - 4f */
_ansi_nop,_ansi_nop,_ansi_nop,_ansi_nop,_ansi_nop,_ansi_nop,_ansi_nop,_ansi_nop,	/* 50 - 57 */
_ansi_nop,_ansi_nop,_ansi_nop,_ansi_csi,_ansi_nop,_ansi_nop,_ansi_nop,_ansi_nop,	/* 58 - 5f */
_ansi_nop,_ansi_nop,_ansi_nop,qa_reset,_ansi_nop,_ansi_nop,_ansi_nop,_ansi_nop,	/* 60 - 67 */
_ansi_nop,_ansi_nop,_ansi_nop,_ansi_nop,_ansi_nop,_ansi_nop,_ansi_nop,_ansi_nop,	/* 68 - 6f */
_ansi_nop,_ansi_nop,_ansi_nop,_ansi_nop,_ansi_nop,_ansi_nop,_ansi_nop,_ansi_nop,	/* 70 - 77 */
_ansi_nop,_ansi_nop,_ansi_nop,_ansi_nop,_ansi_nop,_ansi_nop,_ansi_nop,_ansi_nop},	/* 78 - 7f */

/* CSI STATE */
{
_ansi_nop,_ansi_nop,_ansi_nop,_ansi_nop,_ansi_nop,_ansi_nop,_ansi_nop,_ansi_nop,	/* 00 - 07 */
_ansi_nop,_ansi_nop,_ansi_nop,_ansi_nop,_ansi_nop,_ansi_nop,_ansi_nop,_ansi_nop,	/* 08 - 0f */
_ansi_nop,_ansi_nop,_ansi_nop,_ansi_nop,_ansi_nop,_ansi_nop,_ansi_nop,_ansi_nop,	/* 10 - 17 */
_ansi_nop,_ansi_nop,_ansi_nop,_ansi_esc,_ansi_nop,_ansi_nop,_ansi_nop,_ansi_nop,	/* 18 - 1f */
_ansi_nop,_ansi_nop,_ansi_nop,_ansi_nop,_ansi_nop,_ansi_nop,_ansi_nop,_ansi_nop,	/* 20 - 27 */
_ansi_nop,_ansi_nop,_ansi_nop,_ansi_nop,_ansi_nop,_ansi_nop,_ansi_nop,_ansi_nop,	/* 28 - 2f */
_ansi_param,_ansi_param,_ansi_param,_ansi_param,_ansi_param,_ansi_param,_ansi_param,_ansi_param,/* 30 - 37 */
_ansi_param,_ansi_param,_ansi_nop,_ansi_chparm,_ansi_nop,_ansi_spesc,_ansi_spesc,_ansi_spesc,/* 38 - 3f */
_ansi_ich,_ansi_cuu,_ansi_cud,_ansi_cuf,_ansi_cub,_ansi_nop,_ansi_nop,_ansi_nop,	/* 40 - 47 */
_ansi_poscur,_ansi_nop,_ansi_ed,_ansi_el,_ansi_il,_ansi_dl,_ansi_nop,_ansi_nop,		/* 48 - 4f */
_ansi_dch,_ansi_nop,_ansi_nop,_ansi_su,_ansi_sd,_ansi_nop,_ansi_nop,_ansi_nop,		/* 50 - 57 */
_ansi_nop,_ansi_nop,_ansi_db_p,_ansi_nop,_ansi_nop,_ansi_nop,_ansi_nop,_ansi_nop,	/* 58 - 5f */
_ansi_nop,_ansi_nop,_ansi_nop,_ansi_nop,_ansi_nop,_ansi_nop,_ansi_poscur,_ansi_nop,	/* 60 - 67 */
_ansi_sm,_ansi_nop,_ansi_nop,_ansi_nop,_ansi_rm,_ansi_sgr,_ansi_nop,_ansi_nop,		/* 68 - 6f */
qa_reset,_ansi_nop,_ansi_decstbm,_ansi_nop,_ansi_nop,_ansi_nop,_ansi_nop,_ansi_nop,/* 70 - 77 */
_ansi_nop,_ansi_nop,_ansi_nop,_ansi_nop,_ansi_nop,_ansi_nop,_ansi_nop,_ansi_nop},	/* 78 - 7f */

/* DESIGNATE GRAPHICS STATE */
{
_ansi_nop,_ansi_nop,_ansi_nop,_ansi_nop,_ansi_nop,_ansi_nop,_ansi_nop,_ansi_nop,	/* 00 - 07 */
_ansi_nop,_ansi_nop,_ansi_nop,_ansi_nop,_ansi_nop,_ansi_nop,_ansi_nop,_ansi_nop,	/* 08 - 0f */
_ansi_nop,_ansi_nop,_ansi_nop,_ansi_nop,_ansi_nop,_ansi_nop,_ansi_nop,_ansi_nop,	/* 10 - 17 */
_ansi_nop,_ansi_nop,_ansi_nop,_ansi_esc,_ansi_nop,_ansi_nop,_ansi_nop,_ansi_nop,	/* 18 - 1f */
_ansi_nop,_ansi_nop,_ansi_nop,_ansi_nop,_ansi_nop,_ansi_nop,_ansi_nop,_ansi_nop,	/* 20 - 27 */
_ansi_nop,_ansi_nop,_ansi_nop,_ansi_nop,_ansi_nop,_ansi_nop,_ansi_nop,_ansi_nop,	/* 28 - 2f */
_ansi_dG0,_ansi_dG0,_ansi_dG0,_ansi_dG0,_ansi_dG0,_ansi_dG0,_ansi_dG0,_ansi_dG0,	/* 30 - 37 */
_ansi_dG0,_ansi_dG0,_ansi_dG0,_ansi_dG0,_ansi_dG0,_ansi_dG0,_ansi_dG0,_ansi_dG0,	/* 38 - 3f */
_ansi_dG0,_ansi_dG0,_ansi_dG0,_ansi_dG0,_ansi_dG0,_ansi_dG0,_ansi_dG0,_ansi_dG0,	/* 40 - 47 */
_ansi_dG0,_ansi_dG0,_ansi_dG0,_ansi_dG0,_ansi_dG0,_ansi_dG0,_ansi_dG0,_ansi_dG0,	/* 48 - 4f */
_ansi_dG0,_ansi_dG0,_ansi_dG0,_ansi_dG0,_ansi_dG0,_ansi_dG0,_ansi_dG0,_ansi_dG0,	/* 50 - 57 */
_ansi_dG0,_ansi_dG0,_ansi_dG0,_ansi_dG0,_ansi_dG0,_ansi_dG0,_ansi_dG0,_ansi_dG0,	/* 58 - 5f */
_ansi_dG0,_ansi_dG0,_ansi_dG0,_ansi_dG0,_ansi_dG0,_ansi_dG0,_ansi_dG0,_ansi_dG0,	/* 60 - 67 */
_ansi_dG0,_ansi_dG0,_ansi_dG0,_ansi_dG0,_ansi_dG0,_ansi_dG0,_ansi_dG0,_ansi_dG0,	/* 68 - 6f */
_ansi_dG0,_ansi_dG0,_ansi_dG0,_ansi_dG0,_ansi_dG0,_ansi_dG0,_ansi_dG0,_ansi_dG0,	/* 70 - 77 */
_ansi_dG0,_ansi_dG0,_ansi_dG0,_ansi_dG0,_ansi_dG0,_ansi_dG0,_ansi_dG0,_ansi_nop},	/* 78 - 7f */

/* DESIGNATE GRAPHICS STATE G1 */
{
_ansi_nop,_ansi_nop,_ansi_nop,_ansi_nop,_ansi_nop,_ansi_nop,_ansi_nop,_ansi_nop,	/* 00 - 07 */
_ansi_nop,_ansi_nop,_ansi_nop,_ansi_nop,_ansi_nop,_ansi_nop,_ansi_nop,_ansi_nop,	/* 08 - 0f */
_ansi_nop,_ansi_nop,_ansi_nop,_ansi_nop,_ansi_nop,_ansi_nop,_ansi_nop,_ansi_nop,	/* 10 - 17 */
_ansi_nop,_ansi_nop,_ansi_nop,_ansi_esc,_ansi_nop,_ansi_nop,_ansi_nop,_ansi_nop,	/* 18 - 1f */
_ansi_nop,_ansi_nop,_ansi_nop,_ansi_nop,_ansi_nop,_ansi_nop,_ansi_nop,_ansi_nop,	/* 20 - 27 */
_ansi_nop,_ansi_nop,_ansi_nop,_ansi_nop,_ansi_nop,_ansi_nop,_ansi_nop,_ansi_nop,	/* 28 - 2f */
_ansi_dG1,_ansi_dG1,_ansi_dG1,_ansi_dG1,_ansi_dG1,_ansi_dG1,_ansi_dG1,_ansi_dG1,	/* 30 - 37 */
_ansi_dG1,_ansi_dG1,_ansi_dG1,_ansi_dG1,_ansi_dG1,_ansi_dG1,_ansi_dG1,_ansi_dG1,	/* 38 - 3f */
_ansi_dG1,_ansi_dG1,_ansi_dG1,_ansi_dG1,_ansi_dG1,_ansi_dG1,_ansi_dG1,_ansi_dG1,	/* 40 - 47 */
_ansi_dG1,_ansi_dG1,_ansi_dG1,_ansi_dG1,_ansi_dG1,_ansi_dG1,_ansi_dG1,_ansi_dG1,	/* 48 - 4f */
_ansi_dG1,_ansi_dG1,_ansi_dG1,_ansi_dG1,_ansi_dG1,_ansi_dG1,_ansi_dG1,_ansi_dG1,	/* 50 - 57 */
_ansi_dG1,_ansi_dG1,_ansi_dG1,_ansi_dG1,_ansi_dG1,_ansi_dG1,_ansi_dG1,_ansi_dG1,	/* 58 - 5f */
_ansi_dG1,_ansi_dG1,_ansi_dG1,_ansi_dG1,_ansi_dG1,_ansi_dG1,_ansi_dG1,_ansi_dG1,	/* 60 - 67 */
_ansi_dG1,_ansi_dG1,_ansi_dG1,_ansi_dG1,_ansi_dG1,_ansi_dG1,_ansi_dG1,_ansi_dG1,	/* 68 - 6f */
_ansi_dG1,_ansi_dG1,_ansi_dG1,_ansi_dG1,_ansi_dG1,_ansi_dG1,_ansi_dG1,_ansi_dG1,	/* 70 - 77 */
_ansi_dG1,_ansi_dG1,_ansi_dG1,_ansi_dG1,_ansi_dG1,_ansi_dG1,_ansi_dG1,_ansi_nop},	/* 78 - 7f */

/* CHARACTER SIZE STATE */
{
_ansi_nop,_ansi_nop,_ansi_nop,_ansi_nop,_ansi_nop,_ansi_nop,_ansi_nop,_ansi_nop,	/* 00 - 07 */
_ansi_nop,_ansi_nop,_ansi_nop,_ansi_nop,_ansi_nop,_ansi_nop,_ansi_nop,_ansi_nop,	/* 08 - 0f */
_ansi_nop,_ansi_nop,_ansi_nop,_ansi_nop,_ansi_nop,_ansi_nop,_ansi_nop,_ansi_nop,	/* 10 - 17 */
_ansi_nop,_ansi_nop,_ansi_nop,_ansi_esc,_ansi_nop,_ansi_nop,_ansi_nop,_ansi_nop,	/* 18 - 1f */
_ansi_nop,_ansi_nop,_ansi_nop,_ansi_nop,_ansi_nop,_ansi_nop,_ansi_nop,_ansi_nop,	/* 20 - 27 */
_ansi_nop,_ansi_nop,_ansi_nop,_ansi_nop,_ansi_nop,_ansi_nop,_ansi_nop,_ansi_nop,	/* 28 - 2f */
_ansi_nop,_ansi_nop,_ansi_nop,_ansi_nop,_ansi_nop,_ansi_nop,_ansi_nop,_ansi_nop,	/* 30 - 37 */
_ansi_nop,_ansi_nop,_ansi_nop,_ansi_nop,_ansi_nop,_ansi_nop,_ansi_nop,_ansi_nop,	/* 38 - 3f */
_ansi_nop,_ansi_nop,_ansi_nop,_ansi_nop,_ansi_nop,_ansi_nop,_ansi_nop,_ansi_nop,	/* 40 - 47 */
_ansi_nop,_ansi_nop,_ansi_nop,_ansi_nop,_ansi_nop,_ansi_nop,_ansi_nop,_ansi_nop,	/* 48 - 4f */
_ansi_nop,_ansi_nop,_ansi_nop,_ansi_nop,_ansi_nop,_ansi_nop,_ansi_nop,_ansi_nop,	/* 50 - 57 */
_ansi_nop,_ansi_nop,_ansi_nop,_ansi_nop,_ansi_nop,_ansi_nop,_ansi_nop,_ansi_nop,	/* 58 - 5f */
_ansi_nop,_ansi_nop,_ansi_nop,_ansi_nop,_ansi_nop,_ansi_nop,_ansi_nop,_ansi_nop,	/* 60 - 67 */
_ansi_nop,_ansi_nop,_ansi_nop,_ansi_nop,_ansi_nop,_ansi_nop,_ansi_nop,_ansi_nop,	/* 68 - 6f */
_ansi_nop,_ansi_nop,_ansi_nop,_ansi_nop,_ansi_nop,_ansi_nop,_ansi_nop,_ansi_nop,	/* 70 - 77 */
_ansi_nop,_ansi_nop,_ansi_nop,_ansi_nop,_ansi_nop,_ansi_nop,_ansi_nop,_ansi_nop}	/* 78 - 7f */
};

#else
/* this table does not support any color or monochrome attribute selection */

FUNC _ansi_do_decode[6][128] = {
/* INIT STATE */
{
_ansi_nop,_ansi_nop,_ansi_nop,_ansi_nop,_ansi_nop,_ansi_nop,_ansi_nop,_ansi_nop,	/* 00 - 07 */
_ansi_cub,_ansi_tab,_ansi_lf,_ansi_nop,_ansi_nop,_ansi_cret,_ansi_so,_ansi_si,		/* 08 - 0f */
_ansi_nop,_ansi_nop,_ansi_nop,_ansi_nop,_ansi_nop,_ansi_nop,_ansi_nop,_ansi_nop,	/* 10 - 17 */
_ansi_nop,_ansi_nop,_ansi_nop,_ansi_esc,_ansi_nop,_ansi_nop,_ansi_nop,_ansi_nop,	/* 18 - 1f */
_ansi_pch,_ansi_pch,_ansi_pch,_ansi_pch,_ansi_pch,_ansi_pch,_ansi_pch,_ansi_pch,	/* 20 - 27 */
_ansi_pch,_ansi_pch,_ansi_pch,_ansi_pch,_ansi_pch,_ansi_pch,_ansi_pch,_ansi_pch,	/* 28 - 2f */
_ansi_pch,_ansi_pch,_ansi_pch,_ansi_pch,_ansi_pch,_ansi_pch,_ansi_pch,_ansi_pch,	/* 30 - 37 */
_ansi_pch,_ansi_pch,_ansi_pch,_ansi_pch,_ansi_pch,_ansi_pch,_ansi_pch,_ansi_pch,	/* 38 - 3f */ 
_ansi_pch,_ansi_pch,_ansi_pch,_ansi_pch,_ansi_pch,_ansi_pch,_ansi_pch,_ansi_pch,	/* 40 - 47 */ 
_ansi_pch,_ansi_pch,_ansi_pch,_ansi_pch,_ansi_pch,_ansi_pch,_ansi_pch,_ansi_pch,	/* 48 - 4f */ 
_ansi_pch,_ansi_pch,_ansi_pch,_ansi_pch,_ansi_pch,_ansi_pch,_ansi_pch,_ansi_pch,	/* 50 - 57 */ 
_ansi_pch,_ansi_pch,_ansi_pch,_ansi_pch,_ansi_pch,_ansi_pch,_ansi_pch,_ansi_pch,	/* 58 - 5f */ 
_ansi_pch,_ansi_pch,_ansi_pch,_ansi_pch,_ansi_pch,_ansi_pch,_ansi_pch,_ansi_pch,	/* 60 - 67 */ 
_ansi_pch,_ansi_pch,_ansi_pch,_ansi_pch,_ansi_pch,_ansi_pch,_ansi_pch,_ansi_pch,	/* 68 - 6f */ 
_ansi_pch,_ansi_pch,_ansi_pch,_ansi_pch,_ansi_pch,_ansi_pch,_ansi_pch,_ansi_pch,	/* 70 - 77 */ 
_ansi_pch,_ansi_pch,_ansi_pch,_ansi_pch,_ansi_pch,_ansi_pch,_ansi_pch,_ansi_nop},	/* 78 - 7f */ 

/* ESCAPE STATE */
{
_ansi_nop,_ansi_nop,_ansi_nop,_ansi_nop,_ansi_nop,_ansi_nop,_ansi_nop,_ansi_nop,	/* 00 - 07 */
_ansi_nop,_ansi_nop,_ansi_nop,_ansi_nop,_ansi_nop,_ansi_nop,_ansi_nop,_ansi_nop,	/* 08 - 0f */
_ansi_nop,_ansi_nop,_ansi_nop,_ansi_nop,_ansi_nop,_ansi_nop,_ansi_nop,_ansi_nop,	/* 10 - 17 */
_ansi_nop,_ansi_nop,_ansi_nop,_ansi_esc,_ansi_nop,_ansi_nop,_ansi_nop,_ansi_nop,	/* 18 - 1f */
_ansi_nop,_ansi_nop,_ansi_nop,_ansi_nop,_ansi_nop,_ansi_nop,_ansi_nop,_ansi_nop,	/* 20 - 27 */
_ansi_stG0,_ansi_stG1,_ansi_nop,_ansi_nop,_ansi_nop,_ansi_nop,_ansi_nop,_ansi_nop,	/* 28 - 2f */
_ansi_nop,_ansi_nop,_ansi_nop,_ansi_nop,_ansi_nop,_ansi_nop,_ansi_nop,_ansi_decsc,	/* 30 - 37 */
_ansi_decrc,_ansi_nop,_ansi_nop,_ansi_nop,_ansi_nop,_ansi_nop,_ansi_nop,_ansi_nop,	/* 38 - 3f */
_ansi_nop,_ansi_nop,_ansi_nop,_ansi_nop,_ansi_lf,_ansi_nel,_ansi_nop,_ansi_nop,		/* 40 - 47 */
_ansi_nop,_ansi_nop,_ansi_nop,_ansi_nop,_ansi_nop,_ansi_ri,_ansi_nop,_ansi_nop,		/* 48 - 4f */
_ansi_nop,_ansi_nop,_ansi_nop,_ansi_nop,_ansi_nop,_ansi_nop,_ansi_nop,_ansi_nop,	/* 50 - 57 */
_ansi_nop,_ansi_nop,_ansi_nop,_ansi_csi,_ansi_nop,_ansi_nop,_ansi_nop,_ansi_nop,	/* 58 - 5f */
_ansi_nop,_ansi_nop,_ansi_nop,qa_reset,_ansi_nop,_ansi_nop,_ansi_nop,_ansi_nop,	/* 60 - 67 */
_ansi_nop,_ansi_nop,_ansi_nop,_ansi_nop,_ansi_nop,_ansi_nop,_ansi_nop,_ansi_nop,	/* 68 - 6f */
_ansi_nop,_ansi_nop,_ansi_nop,_ansi_nop,_ansi_nop,_ansi_nop,_ansi_nop,_ansi_nop,	/* 70 - 77 */
_ansi_nop,_ansi_nop,_ansi_nop,_ansi_nop,_ansi_nop,_ansi_nop,_ansi_nop,_ansi_nop},	/* 78 - 7f */

/* CSI STATE */
{
_ansi_nop,_ansi_nop,_ansi_nop,_ansi_nop,_ansi_nop,_ansi_nop,_ansi_nop,_ansi_nop,	/* 00 - 07 */
_ansi_nop,_ansi_nop,_ansi_nop,_ansi_nop,_ansi_nop,_ansi_nop,_ansi_nop,_ansi_nop,	/* 08 - 0f */
_ansi_nop,_ansi_nop,_ansi_nop,_ansi_nop,_ansi_nop,_ansi_nop,_ansi_nop,_ansi_nop,	/* 10 - 17 */
_ansi_nop,_ansi_nop,_ansi_nop,_ansi_esc,_ansi_nop,_ansi_nop,_ansi_nop,_ansi_nop,	/* 18 - 1f */
_ansi_nop,_ansi_nop,_ansi_nop,_ansi_nop,_ansi_nop,_ansi_nop,_ansi_nop,_ansi_nop,	/* 20 - 27 */
_ansi_nop,_ansi_nop,_ansi_nop,_ansi_nop,_ansi_nop,_ansi_nop,_ansi_nop,_ansi_nop,	/* 28 - 2f */
_ansi_param,_ansi_param,_ansi_param,_ansi_param,_ansi_param,_ansi_param,_ansi_param,_ansi_param,/* 30 - 37 */
_ansi_param,_ansi_param,_ansi_nop,_ansi_chparm,_ansi_nop,_ansi_spesc,_ansi_spesc,_ansi_spesc,/* 38 - 3f */
_ansi_ich,_ansi_cuu,_ansi_cud,_ansi_cuf,_ansi_cub,_ansi_nop,_ansi_nop,_ansi_nop,	/* 40 - 47 */
_ansi_poscur,_ansi_nop,_ansi_ed,_ansi_el,_ansi_il,_ansi_dl,_ansi_nop,_ansi_nop,		/* 48 - 4f */
_ansi_dch,_ansi_nop,_ansi_nop,_ansi_su,_ansi_sd,_ansi_nop,_ansi_nop,_ansi_nop,		/* 50 - 57 */
_ansi_nop,_ansi_nop,_ansi_nop,_ansi_nop,_ansi_nop,_ansi_nop,_ansi_nop,_ansi_nop,	/* 58 - 5f */
_ansi_nop,_ansi_nop,_ansi_nop,_ansi_nop,_ansi_nop,_ansi_nop,_ansi_poscur,_ansi_nop,	/* 60 - 67 */
_ansi_sm,_ansi_nop,_ansi_nop,_ansi_nop,_ansi_rm,_ansi_nop,_ansi_nop,_ansi_nop,		/* 68 - 6f */
qa_reset,_ansi_nop,_ansi_decstbm,_ansi_nop,_ansi_nop,_ansi_nop,_ansi_nop,_ansi_nop,/* 70 - 77 */
_ansi_nop,_ansi_nop,_ansi_nop,_ansi_nop,_ansi_nop,_ansi_nop,_ansi_nop,_ansi_nop},	/* 78 - 7f */

/* DESIGNATE GRAPHICS STATE G0 */
{
_ansi_nop,_ansi_nop,_ansi_nop,_ansi_nop,_ansi_nop,_ansi_nop,_ansi_nop,_ansi_nop,	/* 00 - 07 */
_ansi_nop,_ansi_nop,_ansi_nop,_ansi_nop,_ansi_nop,_ansi_nop,_ansi_nop,_ansi_nop,	/* 08 - 0f */
_ansi_nop,_ansi_nop,_ansi_nop,_ansi_nop,_ansi_nop,_ansi_nop,_ansi_nop,_ansi_nop,	/* 10 - 17 */
_ansi_nop,_ansi_nop,_ansi_nop,_ansi_esc,_ansi_nop,_ansi_nop,_ansi_nop,_ansi_nop,	/* 18 - 1f */
_ansi_nop,_ansi_nop,_ansi_nop,_ansi_nop,_ansi_nop,_ansi_nop,_ansi_nop,_ansi_nop,	/* 20 - 27 */
_ansi_nop,_ansi_nop,_ansi_nop,_ansi_nop,_ansi_nop,_ansi_nop,_ansi_nop,_ansi_nop,	/* 28 - 2f */
_ansi_dG0,_ansi_dG0,_ansi_dG0,_ansi_dG0,_ansi_dG0,_ansi_dG0,_ansi_dG0,_ansi_dG0,	/* 30 - 37 */
_ansi_dG0,_ansi_dG0,_ansi_dG0,_ansi_dG0,_ansi_dG0,_ansi_dG0,_ansi_dG0,_ansi_dG0,	/* 38 - 3f */
_ansi_dG0,_ansi_dG0,_ansi_dG0,_ansi_dG0,_ansi_dG0,_ansi_dG0,_ansi_dG0,_ansi_dG0,	/* 40 - 47 */
_ansi_dG0,_ansi_dG0,_ansi_dG0,_ansi_dG0,_ansi_dG0,_ansi_dG0,_ansi_dG0,_ansi_dG0,	/* 48 - 4f */
_ansi_dG0,_ansi_dG0,_ansi_dG0,_ansi_dG0,_ansi_dG0,_ansi_dG0,_ansi_dG0,_ansi_dG0,	/* 50 - 57 */
_ansi_dG0,_ansi_dG0,_ansi_dG0,_ansi_dG0,_ansi_dG0,_ansi_dG0,_ansi_dG0,_ansi_dG0,	/* 58 - 5f */
_ansi_dG0,_ansi_dG0,_ansi_dG0,_ansi_dG0,_ansi_dG0,_ansi_dG0,_ansi_dG0,_ansi_dG0,	/* 60 - 67 */
_ansi_dG0,_ansi_dG0,_ansi_dG0,_ansi_dG0,_ansi_dG0,_ansi_dG0,_ansi_dG0,_ansi_dG0,	/* 68 - 6f */
_ansi_dG0,_ansi_dG0,_ansi_dG0,_ansi_dG0,_ansi_dG0,_ansi_dG0,_ansi_dG0,_ansi_dG0,	/* 70 - 77 */
_ansi_dG0,_ansi_dG0,_ansi_dG0,_ansi_dG0,_ansi_dG0,_ansi_dG0,_ansi_dG0,_ansi_nop},	/* 78 - 7f */

/* DESIGNATE GRAPHICS STATE G1 */
{
_ansi_nop,_ansi_nop,_ansi_nop,_ansi_nop,_ansi_nop,_ansi_nop,_ansi_nop,_ansi_nop,	/* 00 - 07 */
_ansi_nop,_ansi_nop,_ansi_nop,_ansi_nop,_ansi_nop,_ansi_nop,_ansi_nop,_ansi_nop,	/* 08 - 0f */
_ansi_nop,_ansi_nop,_ansi_nop,_ansi_nop,_ansi_nop,_ansi_nop,_ansi_nop,_ansi_nop,	/* 10 - 17 */
_ansi_nop,_ansi_nop,_ansi_nop,_ansi_esc,_ansi_nop,_ansi_nop,_ansi_nop,_ansi_nop,	/* 18 - 1f */
_ansi_nop,_ansi_nop,_ansi_nop,_ansi_nop,_ansi_nop,_ansi_nop,_ansi_nop,_ansi_nop,	/* 20 - 27 */
_ansi_nop,_ansi_nop,_ansi_nop,_ansi_nop,_ansi_nop,_ansi_nop,_ansi_nop,_ansi_nop,	/* 28 - 2f */
_ansi_dG1,_ansi_dG1,_ansi_dG1,_ansi_dG1,_ansi_dG1,_ansi_dG1,_ansi_dG1,_ansi_dG1,	/* 30 - 37 */
_ansi_dG1,_ansi_dG1,_ansi_dG1,_ansi_dG1,_ansi_dG1,_ansi_dG1,_ansi_dG1,_ansi_dG1,	/* 38 - 3f */
_ansi_dG1,_ansi_dG1,_ansi_dG1,_ansi_dG1,_ansi_dG1,_ansi_dG1,_ansi_dG1,_ansi_dG1,	/* 40 - 47 */
_ansi_dG1,_ansi_dG1,_ansi_dG1,_ansi_dG1,_ansi_dG1,_ansi_dG1,_ansi_dG1,_ansi_dG1,	/* 48 - 4f */
_ansi_dG1,_ansi_dG1,_ansi_dG1,_ansi_dG1,_ansi_dG1,_ansi_dG1,_ansi_dG1,_ansi_dG1,	/* 50 - 57 */
_ansi_dG1,_ansi_dG1,_ansi_dG1,_ansi_dG1,_ansi_dG1,_ansi_dG1,_ansi_dG1,_ansi_dG1,	/* 58 - 5f */
_ansi_dG1,_ansi_dG1,_ansi_dG1,_ansi_dG1,_ansi_dG1,_ansi_dG1,_ansi_dG1,_ansi_dG1,	/* 60 - 67 */
_ansi_dG1,_ansi_dG1,_ansi_dG1,_ansi_dG1,_ansi_dG1,_ansi_dG1,_ansi_dG1,_ansi_dG1,	/* 68 - 6f */
_ansi_dG1,_ansi_dG1,_ansi_dG1,_ansi_dG1,_ansi_dG1,_ansi_dG1,_ansi_dG1,_ansi_dG1,	/* 70 - 77 */
_ansi_dG1,_ansi_dG1,_ansi_dG1,_ansi_dG1,_ansi_dG1,_ansi_dG1,_ansi_dG1,_ansi_nop},	/* 78 - 7f */

/* CHARACTER SIZE STATE */
{
_ansi_nop,_ansi_nop,_ansi_nop,_ansi_nop,_ansi_nop,_ansi_nop,_ansi_nop,_ansi_nop,	/* 00 - 07 */
_ansi_nop,_ansi_nop,_ansi_nop,_ansi_nop,_ansi_nop,_ansi_nop,_ansi_nop,_ansi_nop,	/* 08 - 0f */
_ansi_nop,_ansi_nop,_ansi_nop,_ansi_nop,_ansi_nop,_ansi_nop,_ansi_nop,_ansi_nop,	/* 10 - 17 */
_ansi_nop,_ansi_nop,_ansi_nop,_ansi_esc,_ansi_nop,_ansi_nop,_ansi_nop,_ansi_nop,	/* 18 - 1f */
_ansi_nop,_ansi_nop,_ansi_nop,_ansi_nop,_ansi_nop,_ansi_nop,_ansi_nop,_ansi_nop,	/* 20 - 27 */
_ansi_nop,_ansi_nop,_ansi_nop,_ansi_nop,_ansi_nop,_ansi_nop,_ansi_nop,_ansi_nop,	/* 28 - 2f */
_ansi_nop,_ansi_nop,_ansi_nop,_ansi_nop,_ansi_nop,_ansi_nop,_ansi_nop,_ansi_nop,	/* 30 - 37 */
_ansi_nop,_ansi_nop,_ansi_nop,_ansi_nop,_ansi_nop,_ansi_nop,_ansi_nop,_ansi_nop,	/* 38 - 3f */
_ansi_nop,_ansi_nop,_ansi_nop,_ansi_nop,_ansi_nop,_ansi_nop,_ansi_nop,_ansi_nop,	/* 40 - 47 */
_ansi_nop,_ansi_nop,_ansi_nop,_ansi_nop,_ansi_nop,_ansi_nop,_ansi_nop,_ansi_nop,	/* 48 - 4f */
_ansi_nop,_ansi_nop,_ansi_nop,_ansi_nop,_ansi_nop,_ansi_nop,_ansi_nop,_ansi_nop,	/* 50 - 57 */
_ansi_nop,_ansi_nop,_ansi_nop,_ansi_nop,_ansi_nop,_ansi_nop,_ansi_nop,_ansi_nop,	/* 58 - 5f */
_ansi_nop,_ansi_nop,_ansi_nop,_ansi_nop,_ansi_nop,_ansi_nop,_ansi_nop,_ansi_nop,	/* 60 - 67 */
_ansi_nop,_ansi_nop,_ansi_nop,_ansi_nop,_ansi_nop,_ansi_nop,_ansi_nop,_ansi_nop,	/* 68 - 6f */
_ansi_nop,_ansi_nop,_ansi_nop,_ansi_nop,_ansi_nop,_ansi_nop,_ansi_nop,_ansi_nop,	/* 70 - 77 */
_ansi_nop,_ansi_nop,_ansi_nop,_ansi_nop,_ansi_nop,_ansi_nop,_ansi_nop,_ansi_nop}	/* 78 - 7f */
};
#endif




#endif	/*D_TABLE_H*/
