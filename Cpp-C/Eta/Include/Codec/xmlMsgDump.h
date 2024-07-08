/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2019 LSEG. All rights reserved.                 --
 *|-----------------------------------------------------------------------------
 */

#ifndef __xmlMsgDump_h_
#define __xmlMsgDump_h_

#include "rtr/rsslMsg.h"
#include "rtr/rsslRDM.h"
#include <stdio.h>

#ifdef __cplusplus
extern "C" {
#endif

void xmlDumpReqKeyBegin(FILE * file, const RsslMsgKey * keye);
void xmlDumpKeyBegin(FILE * file, const RsslMsgKey * keye);
void xmlDumpKeyEnd(FILE * file);
void xmlDumpReqKeyEnd(FILE * file);
void xmlDumpExtendedHeader(FILE * file, const RsslBuffer * header);


void xmlDumpMsgBegin(FILE * file, const RsslMsg * msg, const char *tagName);
void xmlDumpMsgEnd(FILE * file, const char *tagName, RsslBool isNested);


#ifdef __cplusplus
}
#endif


#endif

