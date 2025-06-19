/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2015,2018 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

/*	ATTENTION:
 *
 *	This file does NOT contain the version info.  Please alter the version.h file.
 *
 */

#include "EmaVersion.h"

#ifndef WIN32
char emaComponentBldtype[] = BLDTYPE;
char emaComponentLinkType[] = EMA_LINK_TYPE;
#endif

#ifdef __cplusplus
extern "C" {
#endif


/* ------------------------------------------------- */
/* The following are used for the non-windows builds */
/* ------------------------------------------------- */
static const char Origin[]  	= Ema_LegalCopyright;
static const char Version[]		= "ema_version.c   VERSION " PRODVERNAME " " BLDTYPE;
static const char What[]		= "@(#)ema_version.c "	PRODNAME " " PRODVERNAME " " BLDTYPE;
static const char Where[]		= "s.ema_version.c";
static const char DeltaDate[]	= DDATE " " COPYRIGHTYEAR;

#ifdef SOLARIS2
#pragma ident "@(#)" BLDTYPE " EmaVersion " PRODVERNAME
#else
static const char FullVersion[] =  "@(#)" BLDTYPE " EmaVersion " PRODVERNAME;
#endif

char* emaOrigin =   (char *)Origin; 
char* emaVersion = (char *)Version; 
char* emaWhat =   (char *)What; 
char* emaWhere = (char *)Where; 
char* emaDeltaDate = (char *)DeltaDate;


#ifdef __cplusplus
} /* extern "C" */
#endif
