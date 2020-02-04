/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2019 Refinitiv. All rights reserved.            --
 *|-----------------------------------------------------------------------------
 */

#ifndef __decodeRoutines_h_
#define __decodeRoutines_h_

#include "rtr/rsslDataTypeEnums.h"
#include "rtr/rsslElementList.h"
#include "rtr/rsslFieldList.h"
#include "rtr/rsslMap.h"
#include "rtr/rsslArray.h"
#include "rtr/rsslVector.h"
#include "rtr/rsslFilterList.h"
#include "rtr/rsslSeries.h"
#include "rtr/rsslRDM.h"
#include "rtr/rsslDataDictionary.h"
#include "rtr/rsslMsg.h"
#include <stdio.h>
#include "rtr/rsslTypes.h"
#ifdef __cplusplus
extern "C" {
#endif

RSSL_API int decodeDataTypeToXML(FILE * file, RsslDataType dataType, RsslBuffer * buffer, const RsslDataDictionary * dictionary, void *setDb, RsslDecodeIterator *iter);
RSSL_API int decodeEncodedDataToXML(FILE * file, RsslDataType dataEncoding, const RsslBuffer * buffer, const RsslDataDictionary * dictionary, void *setDb);
RSSL_API int decodeSummaryData(FILE *file, RsslDecodeIterator *dIter, RsslContainerType containerType, const RsslBuffer * input, const RsslUInt8 majorVer, const RsslUInt8 minorVer, const RsslDataDictionary *dictionary, void *setDb);
RSSL_API int decodeElementListToXML(FILE * file, RsslDecodeIterator *iter, const RsslDataDictionary * dictionary, RsslLocalElementSetDefDb *setDb);
RSSL_API int decodeFieldListToXML(FILE * file, RsslDecodeIterator *iter, const RsslDataDictionary * dictionary, RsslLocalFieldSetDefDb *setDb);
RSSL_API int decodeFilterListToXML(FILE * file, RsslDecodeIterator *iter, const RsslDataDictionary * dictionary);
RSSL_API int decodeSeriesToXML(FILE * file, RsslDecodeIterator *iter, const RsslDataDictionary * dictionary);
RSSL_API int decodeAnsiPageToXML(FILE * file, const RsslBuffer * input, const RsslDataDictionary * dictionary);
RSSL_API int decodeJSONToXML(FILE * file, const RsslBuffer *input, const RsslDataDictionary * dictionary);
RSSL_API int dumpOpaqueToXML(FILE * file, const RsslBuffer * input, const RsslDataDictionary * dictionary);
RSSL_API int decodeVectorToXML(FILE * file, RsslDecodeIterator *iter, const RsslDataDictionary * dictionary);
RSSL_API int decodeMapToXML(FILE * file, RsslDecodeIterator *iter, const RsslDataDictionary * dictionary);
RSSL_API int decodeArrayToXML(FILE * file, RsslDecodeIterator *iter, const RsslDataDictionary * dictionary);
RSSL_API RsslRet decodeKeyOpaque(FILE * file, const RsslMsgKey * key, RsslDecodeIterator *iter, const RsslDataDictionary * dictionary);
RSSL_API RsslRet decodeKeysToXML(FILE * file, const RsslMsgKey * key, RsslDecodeIterator *iter, const RsslDataDictionary * dictionary);
RSSL_API RsslRet decodeMsgClassToXML(FILE * file, const RsslMsg * msg, RsslDecodeIterator *iter, const RsslDataDictionary * dictionary);
/* set version on the msg structure before calling this */
RSSL_API RsslRet decodeMsgToXML(FILE * file, const RsslMsg * msg, const RsslDataDictionary * dictionary, RsslDecodeIterator *iter);
RSSL_API RsslRet decodeNestedRwfMsgToXML(FILE * file, RsslDecodeIterator *iter, const RsslDataDictionary * dictionary);

RSSL_API const char* getStateCodeAsString( RsslUInt8 code );
RSSL_API const char* getInstrumentNameTypeAsString( RDMInstrumentNameTypes symbolType );
RSSL_API const char* getStreamStateAsString( RsslUInt8 code);
RSSL_API const char* getDataStateAsString( RsslUInt8 code);
RSSL_API const char* getNakCodeAsString(RsslUInt8 code);

RSSL_API int dumpJSON(FILE * file, const RsslBuffer *input);

#ifdef __cplusplus
}
#endif

#endif

