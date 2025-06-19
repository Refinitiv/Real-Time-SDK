/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2015,2017,2022,2024 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

#ifndef _RTR_RSSL_DICTIONARY_PROVIDER_H
#define _RTR_RSSL_DICTIONARY_PROVIDER_H

#include "rtr/rsslTransport.h"
#include "rtr/rsslMessagePackage.h"

#ifdef __cplusplus
extern "C" {
#endif

#define MAX_DICTIONARY_REQUESTS_PER_CLIENT 2
#define MAX_DICTIONARY_REQUESTS (MAX_DICTIONARY_REQUESTS_PER_CLIENT * NUM_CLIENT_SESSIONS)
#define MAX_DICTIONARY_REQ_INFO_STRLEN 128
#define MAX_DICTIONARY_STATUS_MSG_SIZE 1024
#define MAX_FIELD_DICTIONARY_MSG_SIZE 8192
#define MAX_ENUM_TYPE_DICTIONARY_MSG_SIZE 12800

#define FIELD_DICTIONARY_STREAM_ID -1
#define ENUM_TYPE_DICTIONARY_STREAM_ID -2

/* reasons a dictionary request is rejected */
typedef enum {
	UNKNOWN_DICTIONARY_NAME			= 0,
	MAX_DICTIONARY_REQUESTS_REACHED	= 1
} RsslDictionaryRejectReason;

/* dictionary request information */
typedef struct {
	RsslInt32	StreamId;
	char		DictionaryName[MAX_DICTIONARY_REQ_INFO_STRLEN];
	RsslMsgKey		MsgKey;
	RsslChannel*	Chnl;
	RsslBool		IsInUse;
	RsslUInt16		serviceId;
} RsslDictionaryRequestInfo;

void initDictionaryProvider();
RsslRet loadDictionary();
RsslDataDictionary* getDictionary();
static RsslRet encodeFieldDictionaryResponse(RsslChannel* chnl, RsslDictionaryRequestInfo* dictionaryReqInfo, RsslBuffer* msgBuf, RsslInt32* dictionaryFid, RsslBool firstPartMultiPartRefresh);
static RsslRet encodeDictionaryRequestReject(RsslChannel* chnl, RsslInt32 streamId, RsslDictionaryRejectReason reason, RsslBuffer* msgBuf);
static RsslDictionaryRequestInfo* getDictionaryReqInfo(RsslChannel* chnl, RsslMsg* msg, RsslMsgKey* key);
static RsslRet encodeEnumTypeDictionaryResponse(RsslChannel* chnl, RsslDictionaryRequestInfo* dictionaryReqInfo, RsslInt32* dictionaryFid, RsslBuffer* msgBuf, RsslBool firstPartMultiPartRefresh);
RsslRet processDictionaryRequest(RsslChannel* chnl, RsslMsg* msg, RsslDecodeIterator* dIter);
static RsslRet decodeDictionaryRequest(RsslDictionaryRequestInfo* dictionaryReqInfo, RsslMsg* msg, RsslMsgKey* key, RsslDecodeIterator* dIter);
static RsslRet sendFieldDictionaryResponse(RsslChannel* chnl, RsslDictionaryRequestInfo* dictionaryReqInfo);
static RsslRet sendEnumTypeDictionaryResponse(RsslChannel* chnl, RsslDictionaryRequestInfo* dictionaryReqInfo);
static RsslRet sendDictionaryRequestReject(RsslChannel* chnl, RsslInt32 streamId, RsslDictionaryRejectReason reason);
void closeDictionaryChnlStreams(RsslChannel *chnl);
static void closeDictionaryStream(RsslInt32 streamId);
void sendDictionaryCloseStatusMsgs(RsslChannel* chnl);
RsslRet sendDictionaryCloseStatusMsg(RsslChannel* chnl, RsslInt32 streamId);
static RsslRet encodeDictionaryCloseStatus(RsslChannel* chnl, RsslBuffer* msgBuf, RsslInt32 streamId);
RsslRet sendDictionaryRequest(RsslChannel* chnl, const char *dictionaryName, RsslInt32 streamId);
RsslRet sendDictionaryRequests(RsslChannel* chnl);
static RsslRet encodeDictionaryRequest(RsslChannel* chnl, RsslBuffer* msgBuf, const char *dictionaryName, RsslInt32 streamId);
RsslRet processDictionaryResponse(RsslChannel* chnl, RsslMsg* msg, RsslDecodeIterator* dIter);
RsslBool isDictionaryReady();
void freeDictionary();

/*
 * Clears the dictionary request information.
 * dictionaryReqInfo - The dictionary request information to be cleared
 */
RTR_C_INLINE void clearDictionaryReqInfo(RsslDictionaryRequestInfo* dictionaryReqInfo)
{
	dictionaryReqInfo->StreamId = 0;
	dictionaryReqInfo->DictionaryName[0] = '\0';
	rsslClearMsgKey(&dictionaryReqInfo->MsgKey);
	dictionaryReqInfo->MsgKey.name.data = dictionaryReqInfo->DictionaryName;
	dictionaryReqInfo->MsgKey.name.length = MAX_DICTIONARY_REQ_INFO_STRLEN;
	dictionaryReqInfo->MsgKey.filter = RDM_DICTIONARY_NORMAL;
	dictionaryReqInfo->Chnl = 0;
	dictionaryReqInfo->IsInUse = RSSL_FALSE;
}

#ifdef __cplusplus
};
#endif

#endif
