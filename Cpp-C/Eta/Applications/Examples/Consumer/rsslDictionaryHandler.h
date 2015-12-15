
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
