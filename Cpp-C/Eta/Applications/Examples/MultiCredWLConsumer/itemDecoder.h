/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2020,2022,2024 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

#ifndef ITEM_DECODER_H
#define ITEM_DECODER_H

#include "rtr/rsslReactor.h"

#ifdef __cplusplus
extern "C" {
#endif

extern RsslBool fieldDictionaryLoaded;
extern RsslBool enumDictionaryLoaded;

static RsslBuffer fieldDictionaryName = { 6, (char*)"RWFFld" };
static RsslBuffer enumDictionaryName = { 7, (char*)"RWFEnum" };


RsslRet itemDecoderInit();

void itemDecoderCleanup();

RsslRet decodeDataBody(RsslReactorChannel *pChannel, RsslMsg *pRsslMsg);

RsslRet decodeDictionaryDataBody(RsslReactorChannel *pReactorChannel, 
		RsslRDMDictionaryRefresh *pDictionaryRefresh);

RsslRet decodeMarketPriceDataBody(RsslReactorChannel *pReactorChannel, RsslMsg *pRsslMsg);

RsslRet decodeMarketByOrderDataBody(RsslReactorChannel *pReactorChannel, RsslMsg *pRsslMsg);

RsslRet decodeMarketByPriceDataBody(RsslReactorChannel *pReactorChannel, RsslMsg *pRsslMsg);

RsslRet decodeYieldCurveDataBody(RsslReactorChannel *pReactorChannel, RsslMsg *pRsslMsg);

RsslRet decodeSymbolListDataBody(RsslReactorChannel *pReactorChannel, RsslMsg *pRsslMsg);

static RsslRet decodeVector(RsslDecodeIterator *dIter, RsslUInt32 indentCount);

static RsslRet decodeArray(RsslDecodeIterator *pIter, RsslUInt32 indentCount);

static RsslRet decodeFieldList(RsslDecodeIterator *pIter, RsslLocalFieldSetDefDb *pSetDefDb,
		RsslUInt32 indent);

static RsslRet decodeDataType(RsslDecodeIterator *pIter, RsslUInt8 dataType, 
		RsslDictionaryEntry *dictionaryEntry, RsslUInt32 indentCount);

const char *mapActionToString(RsslMapEntryActions action);

#ifdef __cplusplus
}
#endif

#endif
