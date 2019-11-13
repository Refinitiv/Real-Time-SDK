/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for dansiils.                  --
 *|           Copyright (C) 2019 Refinitiv. All rights reserved.            --
 *|-----------------------------------------------------------------------------
 */
#ifndef __ANSI_VERSION_H
#define __ANSI_VERSION_H

#define STR_EXPAND(str) #str
#define MKSTR(str) STR_EXPAND(str)

#ifdef NDEBUG
#define BLDTYPE " Release"
#else
#define BLDTYPE " Debug"
#endif

#define ANSI_Release_Major     1		// Release major version number
#define ANSI_Release_Minor     0		// Release minor version number
#define ANSI_Product_Major     0 	// Release patch number
#define ANSI_Product_Minor     44   	// Release tweak number

#define ANSI_Year				"2019"
#define ANSI_CompanyName		" Refinitiv"
#define ANSI_Version			ANSI_Release_Major,ANSI_Release_Minor,ANSI_Product_Major,ANSI_Product_Minor
#define ANSI_VersionString		MKSTR(ANSI_Release_Major) "." MKSTR(ANSI_Release_Minor) "." MKSTR(ANSI_Product_Major) "." MKSTR(ANSI_Product_Minor) " (" BLDTYPE ")"
#define ANSI_LegalCopyRight		"Copyright (C) " ANSI_Year ANSI_CompanyName ", All Rights Reserved. "
#define ANSI_ExternalName		"ansi1.0.0."
#define ANSI_InternalName		"ansi1.0.0.44"
#define ANSI_ReleaseType		"rrg"
#define ANSI_ProdName			"ANSI Library C Edition"
#define ANSI_Package			" PACKAGE " ANSI_ExternalName " " ANSI_ReleaseType

static char ansiOrigin[]    = ANSI_LegalCopyRight;
static char ansiVersion[]   = " VERSION " ANSI_InternalName;
static char ansiWhat[]      = "@(#)ansiVersion.h" ANSI_InternalName;
static char ansiWhere[]     = "s.ansiVersion.h";
static char ansiPackage[]   = ANSI_Package;
static char ansiDeltaDate[] = " Tue Jul 30 09:57:00 CST " ANSI_Year;

#endif
