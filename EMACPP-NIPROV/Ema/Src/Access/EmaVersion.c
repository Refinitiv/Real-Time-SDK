/*	ATTENTION:
 *
 *	This file does NOT contain the version info.  Please alter the version.h file.
 *
 */

#include "EmaVersion.h"

#ifdef __cplusplus
extern "C" {
#endif


/* ------------------------------------------------- */
/* The following are used for the non-windows builds */
/* ------------------------------------------------- */
static const char Origin[]  	= Ema_LegalCopyright;
static const char Version[]		= "ema_version.c   VERSION " PRODVERNAME " " BLDTYPE "  " INTERNALVERSION ;
static const char What[]		= "@(#)ema_version.c "	PRODNAME " " PRODVERNAME " " BLDTYPE;
static const char Where[]		= "s.ema_version.c";
static const char DeltaDate[]	= DDATE " " COPYRIGHTYEAR;
static const char Comments[]    = Ema_Comments;

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
char* emaComments = (char *)Comments;


#ifdef __cplusplus
} /* extern "C" */
#endif
