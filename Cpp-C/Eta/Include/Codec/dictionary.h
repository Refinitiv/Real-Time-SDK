/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2019 LSEG. All rights reserved.                 --
 *|-----------------------------------------------------------------------------
 */

#ifndef _dictionary_h_
#define _dictionary_h_

#include "rtr/rsslDataTypeEnums.h"
#include "rtr/rsslMsg.h"

#ifdef __cplusplus
extern "C" {
#endif


typedef struct
{
	char * name;
	char * longName;
	RsslInt16 fid;
	RsslInt16 rippleFid;
	RsslDataType type;
	RsslUInt16 length;
	RsslUInt8 block;
	RsslUInt8 mode;
} FieldDef;

typedef struct
{
	RsslBuffer name;
	RsslUInt32 count;
	FieldDef * defs;
} Dictionary;

void freeDictionary(Dictionary * dictionary);
FieldDef * getFieldDef(const Dictionary * dictionary, RsslInt16 id);
RsslRet cacheDictionary(const RsslMsg * msg, Dictionary ** dictionary);
RsslBool isDictionaryMsg(const RsslMsg * msg);

#ifdef __cplusplus
}
#endif

#endif

