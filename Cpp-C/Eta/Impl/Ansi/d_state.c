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
* ROUTINE NAME	_ansi_nop()
*
* DESCRIPTION	Ignores the character at this position.  Resets the
*		state to the initial state.
*
* RETURNS	OK.
*
**************************************************************************/
#ifdef __STDC__
short _ansi_nop(
    register PAGEPTR    page,
    register char       *text,
    LISTPTR             u_list )
#else
short _ansi_nop( page, text, u_list )
    register PAGEPTR    page;
    register char       *text;
    LISTPTR             u_list;
#endif
{
	parse_ptr->state = INITSTATE;
	return (OK);
}


/*************************************************************************
*
* ROUTINE NAME	_ansi_esc()
*
* DESCRIPTION	Enters the escape state.
*
* RETURNS	OK.
*
**************************************************************************/
#ifdef __STDC__
short _ansi_esc(
    register PAGEPTR    page,
    register char       *text,
    LISTPTR             u_list )
#else
short _ansi_esc( page, text, u_list )
    register PAGEPTR    page;
    register char       *text;
    LISTPTR             u_list;
#endif
{
	parse_ptr->state = ESCSTATE;
	parse_ptr->special_esc = 0;
	return (OK);
}


/*************************************************************************
*
* ROUTINE NAME	_ansi_csi()
*
* DESCRIPTION	Enters the CSI state, i.e., we have parsed an escape
*		character followed by a '['.  The next thing to follow
*		may be one or many parameters.
*
* RETURNS	OK.
*
**************************************************************************/
#ifdef __STDC__
short _ansi_csi(
    register PAGEPTR    page,
    register char       *text,
    LISTPTR             u_list )
#else
short _ansi_csi( page, text, u_list )
    register PAGEPTR    page;
    register char       *text;
    LISTPTR             u_list;
#endif
{
	parse_ptr->state = CSISTATE;
	return (OK);
}


/*************************************************************************
*
* ROUTINE NAME	_ansi_stG0() and _ansi_stG1()
*
* DESCRIPTION	Enters the G0 state for selecting G0 graphics set and
* 		enters the G1 state for selecting G1 graphics set.
*
* RETURNS	OK.
*
**************************************************************************/
#ifdef __STDC__
short _ansi_stG0(
    register PAGEPTR    page,
    register char       *text,
    LISTPTR             u_list )
#else
short _ansi_stG0( page, text, u_list )
    register PAGEPTR    page;
    register char       *text;
    LISTPTR             u_list;
#endif
{
	parse_ptr->state = G0STATE;
	return (OK);
}


#ifdef __STDC__
short _ansi_stG1(
    register PAGEPTR    page,
    register char       *text,
    LISTPTR             u_list )
#else
short _ansi_stG1( page, text, u_list )
    register PAGEPTR    page;
    register char       *text;
    LISTPTR             u_list;
#endif
{
	parse_ptr->state = G1STATE;
	return (OK);
}

