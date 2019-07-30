/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright Thomson Reuters 2015,2016,2018. All rights reserved.            --
 *|-----------------------------------------------------------------------------
 */


#include <ctype.h>
#include "ansi/q_ansi.h"
#include "ansi/decodeansi.h"
#include "ansi/ansi_int.h"


/*************************************************************************
*
* ROUTINE NAME	_ansi_dG0()
*
* DESCRIPTION	Designate graphics set -- Installs character into G0 
*				graphics set if it is valid.  If not, graphics set is
*				not changed.
*
* RETURNS	OK.
*
**************************************************************************/
#ifdef __STDC__
short _ansi_dG0(
    register PAGEPTR    page,
    register char       *text,
    LISTPTR             u_list )
#else
short _ansi_dG0( page, text, u_list )
    register PAGEPTR    page;
    register char       *text;
    LISTPTR             u_list;
#endif
{
	if (isprint(*text)) {
		page->status.G0_set = *text;
	}
	parse_ptr->state = INITSTATE;
	return(OK);
}

/*************************************************************************
*
* ROUTINE NAME	_ansi_dG1()
*
* DESCRIPTION	Designate graphics set -- Installs character into G1 
*		graphics set if it is valid.  If not, graphics set is
*		not changed.
*
* RETURNS	OK.
**************************************************************************/
#ifdef __STDC__
short _ansi_dG1(
    register PAGEPTR    page,
    register char       *text,
    LISTPTR             u_list )
#else
short _ansi_dG1( page, text, u_list )
    register PAGEPTR    page;
    register char       *text;
    LISTPTR             u_list;
#endif
{
	if (isprint(*text)) {
		page->status.G1_set = *text;
	}
	parse_ptr->state = INITSTATE;
	return(OK);
}

/*************************************************************************
*
* ROUTINE NAME	_ansi_so()
*
* DESCRIPTION	Shift out -- make G1 character set the current set.
*
* RETURNS	OK.
*
**************************************************************************/
#ifdef __STDC__
short _ansi_so(
    register PAGEPTR    page,
    register char       *text,
    LISTPTR             u_list )
#else
short _ansi_so( page, text, u_list )
    register PAGEPTR    page;
    register char       *text;
    LISTPTR             u_list;
#endif
{
	page->status.gr_set = 1;
	parse_ptr->state = INITSTATE;
	return(OK);
}

/*************************************************************************
*
* ROUTINE NAME	_ansi_si()
*
* DESCRIPTION	Shift in -- make G0 character set the current set.
*
* RETURNS	OK.
*
**************************************************************************/
#ifdef __STDC__
short _ansi_si(
    register PAGEPTR    page,
    register char       *text,
    LISTPTR             u_list )
#else
short _ansi_si( page, text, u_list )
    register PAGEPTR    page;
    register char       *text;
    LISTPTR             u_list;
#endif
{
	page->status.gr_set = 0;
	parse_ptr->state = INITSTATE;
	return(OK);
}

/*************************************************************************
*
* ROUTINE NAME	_ansi_decdwl()
*
* DESCRIPTION	DEC private sequence for double width
*
* RETURNS	OK.
*
**************************************************************************/
#ifdef __STDC__
short _ansi_decdwl(
    register PAGEPTR    page,
    register char       *text,
    LISTPTR             u_list )
#else
short _ansi_decdwl( page, text, u_list )
    register PAGEPTR    page;
    register char       *text;
    LISTPTR             u_list;
#endif
{
	return(OK);
}

/*************************************************************************
*
* ROUTINE NAME	_ansi_decdhl()
*
* DESCRIPTION	DEC private sequence for double width
*
* RETURNS	OK.
*
**************************************************************************/
#ifdef __STDC__
short _ansi_decdhl(
    register PAGEPTR    page,
    register char       *text,
    LISTPTR             u_list )
#else
short _ansi_decdhl( page, text, u_list )
    register PAGEPTR    page;
    register char       *text;
    LISTPTR             u_list;
#endif
{
	return(OK);
}

/*************************************************************************
*
* ROUTINE NAME	_ansi_decswl()
*
* DESCRIPTION	DEC private sequence for double width
*
* RETURNS	OK.
*
**************************************************************************/
#ifdef __STDC__
short _ansi_decswl(
    register PAGEPTR    page,
    register char       *text,
    LISTPTR             u_list )
#else
short _ansi_decswl( page, text, u_list )
    register PAGEPTR    page;
    register char       *text;
    LISTPTR             u_list;
#endif
{
	return(OK);
}
