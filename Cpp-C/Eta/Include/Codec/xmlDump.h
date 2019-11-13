/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2019 Refinitiv. All rights reserved.            --
 *|-----------------------------------------------------------------------------
 */

#ifndef _xmlDump_h_
#define _xmlDump_h_

#include "rtr/rsslDataTypeEnums.h"
#include "rtr/rsslDataPackage.h"
#include "rtr/rsslTypes.h"

#include <stdio.h>

#ifdef __cplusplus
extern "C" {
#endif

RSSL_API void encodeindents(FILE * file);
RSSL_API void xmlDumpUInt(FILE *file, RsslUInt64 value);
RSSL_API void xmlDumpInt(FILE *file, RsslInt64 value);
RSSL_API void xmlDumpDouble(FILE *file, RsslDouble value);
RSSL_API void xmlDumpDate(FILE *file, RsslDate * value);
RSSL_API void xmlDumpTime(FILE *file, RsslTime * value);
RSSL_API void xmlDumpDateTime(FILE *file, RsslDateTime * value);

RSSL_API void xmlDumpStreamState(FILE *file, RsslUInt8 code);
RSSL_API void xmlDumpDataState(FILE *file, RsslUInt8 code);
RSSL_API void xmlDumpStateCode(FILE *file, RsslUInt8 code);


RSSL_API void xmlDumpQos(FILE * file, const RsslQos * qos);
RSSL_API void xmlDumpWorstQos(FILE * file, const RsslQos * qos);
RSSL_API void xmlDumpReal(FILE *file, RsslReal * value);
RSSL_API void xmlDumpString(FILE *file, int length, const char * data);
RSSL_API void xmlDumpBuffer(FILE *file, const RsslBuffer * buffer);
RSSL_API void xmlDumpGroupId(FILE *file, const RsslBuffer * buffer);
RSSL_API void xmlDumpHexBuffer(FILE *file, const RsslBuffer * buffer);

RSSL_API void xmlDumpFieldEncodingDef(FILE *file, int fieldId, int dataType);

RSSL_API void xmlDumpState(FILE *file, const RsslState *state);
RSSL_API void xmlDumpDataType(FILE *file, RsslDataType type);

RSSL_API void xmlDumpElementListBegin(FILE *file, RsslElementList *eList);
RSSL_API void xmlDumpElementBegin(FILE *file, RsslElementEntry *element);
RSSL_API void xmlDumpElementEnd(FILE *file);
RSSL_API void xmlDumpElementListEnd(FILE *file);


RSSL_API void xmlDumpFieldListBegin(FILE *file, RsslFieldList *fList);
RSSL_API void xmlDumpFieldBegin(FILE *file, RsslFieldEntry *field, RsslDataType dataType);
RSSL_API void xmlDumpFieldEnd(FILE *file);
RSSL_API void xmlDumpFieldListEnd(FILE *file);

RSSL_API void xmlDumpSummaryDataBegin(FILE *file);
RSSL_API void xmlDumpSummaryDataEnd(FILE *file);

RSSL_API void xmlDumpVectorBegin(FILE *file, RsslVector *vec);
RSSL_API void xmlDumpVectorEntryBegin(FILE *file, RsslVectorEntry *vEntry);
RSSL_API void xmlDumpVectorEntryEnd(FILE *file);
RSSL_API void xmlDumpVectorEnd(FILE *file);

RSSL_API void xmlDumpMapBegin(FILE *file, RsslMap *rsslMap);
RSSL_API void xmlDumpMapEntryBegin(FILE *file, RsslDataType keyPrimitiveType, RsslMapEntry *mEntry, void *pMapKeyData);
RSSL_API void xmlDumpMapEntryEnd(FILE *file);
RSSL_API void xmlDumpMapEnd(FILE *file);


RSSL_API void xmlDumpArrayBegin(FILE *file, RsslArray *rsslArray);
RSSL_API void xmlDumpArrayItemBegin(FILE *file);
RSSL_API void xmlDumpArrayItemEnd(FILE *file);
RSSL_API void xmlDumpArrayEnd(FILE *file);

RSSL_API void xmlDumpFilterListBegin(FILE *file, RsslFilterList *fList);
RSSL_API void xmlDumpFilterItemBegin(FILE *file, RsslFilterEntry *fItem);
RSSL_API void xmlDumpFilterItemEnd(FILE *file);
RSSL_API void xmlDumpFilterListEnd(FILE *file);

RSSL_API void xmlDumpSeriesBegin(FILE *file, RsslSeries *series);
RSSL_API void xmlDumpSeriesRowBegin(FILE *file, RsslSeriesEntry *row);
RSSL_API void xmlDumpSeriesRowEnd(FILE *file);
RSSL_API void xmlDumpSeriesEnd(FILE *file);

RSSL_API void xmlDumpLocalElementSetDefDb(FILE *file, RsslLocalElementSetDefDb *elListSetDb);
RSSL_API void xmlDumpLocalFieldSetDefDb(FILE *file, RsslLocalFieldSetDefDb *flListSetDb);

RSSL_API void xmlDumpEndNoTag(FILE *file);

RSSL_API void xmlDumpComment(FILE *file, const char* comment);
RSSL_API void xmlDumpTimestamp(FILE *file);
RSSL_API void xmlGetTimeFromEpoch(unsigned long long *hour, unsigned long long *min, unsigned long long *sec, unsigned long long *msec);

#ifdef __cplusplus
}
#endif


#endif
