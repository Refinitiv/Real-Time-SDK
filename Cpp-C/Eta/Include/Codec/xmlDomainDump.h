/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2019 Refinitiv. All rights reserved.            --
 *|-----------------------------------------------------------------------------
 */

#ifndef _xmlDomainDump_h_
#define _xmlDomainDump_h_

#include <stdio.h>


#ifdef __cplusplus
extern "C" {
#endif


void xmlDumpHeaderBegin(FILE * file, const char* tagName);
void xmlDumpHeaderEnd(FILE * file, const char *tagName);

void xmlDumpDataBodyBegin(FILE * file);
void xmlDumpDataBodyEnd(FILE * file);

#ifdef __cplusplus
}
#endif


#endif
