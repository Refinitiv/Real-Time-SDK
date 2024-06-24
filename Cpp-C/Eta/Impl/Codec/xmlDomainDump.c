/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2019 LSEG. All rights reserved.                 --
 *|-----------------------------------------------------------------------------
 */

#include "xmlDomainDump.h"
#include "xmlDump.h"
#include "decodeRoutines.h"

extern int indents;


void xmlDumpHeaderBegin(FILE * file, const char *tagName)
{
	encodeindents(file);
	fprintf(file, "<%s>\n", tagName);
	indents++;
}


void xmlDumpHeaderEnd(FILE * file, const char *tagName)
{
	indents--;
	encodeindents(file);
	fprintf(file, "</%s>", tagName);
	fprintf( file, "\n");
}

void xmlDumpDataBodyBegin(FILE * file)
{
	encodeindents(file);
	fprintf(file, "<dataBody>\n");
	indents++;
}

void xmlDumpDataBodyEnd(FILE * file)
{
	indents--;
	encodeindents(file);
	fprintf(file, "</dataBody>\n");
}
