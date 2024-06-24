/*
 * This source code is provided under the Apache 2.0 license and is provided
 * AS IS with no warranty or guarantee of fit for purpose.  See the project's 
 * LICENSE.md for details. 
 * Copyright (C) 2024 LSEG. All rights reserved.
*/

#ifndef _RTR_RSSL_DICTIONARY_PROVIDER_H
#define _RTR_RSSL_DICTIONARY_PROVIDER_H

#ifdef __cplusplus
extern "C" {
#endif

#include "rtr/rsslReactor.h"
#include "rtr/rsslTransport.h"
#include "rtr/rsslMessagePackage.h"

#define MAX_DICTIONARY_REQUESTS_PER_CLIENT 2
#define MAX_DICTIONARY_REQUESTS (MAX_DICTIONARY_REQUESTS_PER_CLIENT * NUM_CLIENT_SESSIONS)
#define MAX_DICTIONARY_REQ_INFO_STRLEN 128
#define MAX_DICTIONARY_STATUS_MSG_SIZE 1024
#define MAX_FIELD_DICTIONARY_MSG_SIZE 8192
#define MAX_ENUM_TYPE_DICTIONARY_MSG_SIZE 12800
#define MAX_DICTIONARY_MEMORY_BLOCK_LEN 256

/* reasons a dictionary request is rejected */
typedef enum {
	UNKNOWN_DICTIONARY_NAME			= 0,
	MAX_DICTIONARY_REQUESTS_REACHED	= 1,
	DICTIONARY_RDM_DECODER_FAILED   = 2
} RsslDictionaryRejectReason;

/* Stores information about a consumer's dictionary request. */
typedef struct {
	RsslRDMDictionaryRequest dictionaryRequest;
	char		memoryBlock[MAX_DICTIONARY_MEMORY_BLOCK_LEN];
	RsslBuffer memoryBuffer;
	RsslReactorChannel*	Chnl;
	RsslBool		IsInUse;
} DictionaryRequestInfo;

void initDictionaryProvider();
RsslRet loadDictionary();
RsslDataDictionary* getDictionary();
static DictionaryRequestInfo* getDictionaryReqInfo(RsslReactorChannel* chnl, RsslRDMDictionaryRequest *pDictionaryRequest);
RsslRet processDictionaryRequest(RsslReactor *pReactor, RsslReactorChannel* chnl, RsslMsg* msg, RsslDecodeIterator* dIter);
static RsslRet decodeDictionaryRequest(DictionaryRequestInfo* dictionaryReqInfo, RsslMsg* msg, RsslMsgKey* key, RsslDecodeIterator* dIter);
static RsslRet sendFieldDictionaryRefresh(RsslReactor *pReactor, RsslReactorChannel* chnl, DictionaryRequestInfo* dictionaryReqInfo);
static RsslRet sendEnumTypeDictionaryRefresh(RsslReactor *pReactor, RsslReactorChannel* chnl, DictionaryRequestInfo* dictionaryReqInfo);
static RsslRet sendDictionaryRequestReject(RsslReactor *pReactor, RsslReactorChannel* chnl, RsslInt32 streamId, RsslDictionaryRejectReason reason, RsslErrorInfo *pError);
void closeDictionaryChnlStreams(RsslReactorChannel *chnl);
static void closeDictionaryStream(RsslInt32 streamId);
void sendDictionaryCloseStatusMsgs(RsslReactor *pReactor, RsslReactorChannel* chnl);
RsslRet sendDictionaryCloseStatusMsg(RsslReactor *pReactor, RsslReactorChannel* chnl, RsslInt32 streamId);
void freeDictionary();
RsslReactorCallbackRet dictionaryMsgCallback(RsslReactor* pReactor, RsslReactorChannel* pReactorChannel, RsslRDMDictionaryMsgEvent* pDictionaryMsgEvent);

/*
 * Clears the dictionary request information.
 * dictionaryReqInfo - The dictionary request information to be cleared
 */
RTR_C_INLINE void clearDictionaryReqInfo(DictionaryRequestInfo* dictionaryReqInfo)
{
	rsslClearRDMDictionaryRequest(&dictionaryReqInfo->dictionaryRequest);
	dictionaryReqInfo->Chnl = 0;
	dictionaryReqInfo->IsInUse = RSSL_FALSE;
	dictionaryReqInfo->memoryBuffer.data = dictionaryReqInfo->memoryBlock;
	dictionaryReqInfo->memoryBuffer.length = sizeof(dictionaryReqInfo->memoryBlock);
}

#ifdef __cplusplus
};
#endif

#endif
