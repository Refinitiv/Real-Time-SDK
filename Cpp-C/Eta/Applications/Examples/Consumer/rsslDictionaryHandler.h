/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2019-2022 Refinitiv. All rights reserved.         --
 *|-----------------------------------------------------------------------------
 */


#ifndef _RTR_RSSL_DICTIONARY_HANDLER_H
#define _RTR_RSSL_DICTIONARY_HANDLER_H

#include "rsslConsumer.h"

#ifdef __cplusplus
extern "C" {
#endif

#define FIELD_DICTIONARY_STREAM_ID 3
#define ENUM_TYPE_DICTIONARY_STREAM_ID 4

void loadDictionary();
RsslBool isFieldDictionaryLoaded();
RsslBool isFieldDictionaryLoadedFromFile();
RsslBool isEnumTypeDictionaryLoaded();
RsslBool isEnumTypeDictionaryLoadedFromFile();
RsslDataDictionary* getDictionary();
RsslRet sendDictionaryRequests(RsslChannel* chnl);
RsslRet sendDictionaryRequest(RsslChannel* chnl, const char *dictionaryName, RsslInt32 streamId);
static RsslRet encodeDictionaryRequest(RsslChannel* chnl, RsslBuffer* msgBuf, const char *dictionaryName, RsslInt32 streamId);
RsslRet processDictionaryResponse(RsslChannel* chnl, RsslMsg* msg, RsslDecodeIterator* dIter);
RsslRet closeDictionaryStream(RsslChannel* chnl, RsslInt32 streamId);
static RsslRet encodeDictionaryClose(RsslChannel* chnl, RsslBuffer* msgBuf, RsslInt32 streamId);
void freeDictionary();
void resetDictionaryStreamId();
RsslBool needToDeleteDictionary();

#ifdef __cplusplus
};
#endif

#endif
