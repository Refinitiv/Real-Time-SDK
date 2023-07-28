/*
 * This source code is provided under the Apache 2.0 license and is provided
 * AS IS with no warranty or guarantee of fit for purpose.  See the project's 
 * LICENSE.md for details. 
 * Copyright (C) 2020-2022 Refinitiv. All rights reserved.
*/

#include "consumerThreads.h"
#include "rtr/rsslGetTime.h"
#include "testUtils.h"
#include "marketPriceDecoder.h"
#include "marketPriceEncoder.h"
#include "marketByOrderDecoder.h"
#include "marketByOrderEncoder.h"
#include "xmlItemListParser.h"
#include "rtr/rsslTransport.h"
#include "rtr/rsslMessagePackage.h"
#include "rtr/rsslDataPackage.h"
#ifndef NO_ETA_CPU_BIND
#include "rtr/rsslBindThread.h"
#endif
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <ctype.h>
#include <math.h>
#include <assert.h>
#ifdef _WIN32
#include <winsock2.h>
#include <time.h>
#include <process.h>
#else
#include <sys/time.h>
#include <sys/types.h>
#include <sys/timeb.h>
#include <unistd.h>
#endif

#include <cjson/cJSON.h>
#include "jsonHandler.h"

#define LATENCY_RANDOM_ARRAY_SET_COUNT 20

//uncomment the following line for debugging only - this will greatly affect performance
//#define ENABLE_XML_TRACE

static const RsslInt32 
			LOGIN_STREAM_ID = 1,
			DIRECTORY_STREAM_ID = 2,
			FIELD_DICTIONARY_STREAM_ID = 3,
			ENUM_TYPE_DICTIONARY_STREAM_ID = 4,
			ITEM_STREAM_ID_START = 5;

static RsslBuffer applicationName = { 8, (char*)"ConsPerf" } ;

RsslPostUserInfo postUserInfo;

RsslBool shutdownThreads = RSSL_FALSE;

#ifdef _WIN32
__declspec(thread) static void* jsonTlsPtr = NULL;
__declspec(thread) static size_t jsonTlsSize = 0;
#else
static __thread void* jsonTlsPtr = NULL;
static __thread size_t jsonTlsSize = 0;
#endif
void RTR_C_INLINE SET_CJSON_ALLOCATOR(void * ptr, size_t sz)
{
	jsonTlsPtr = ptr;
	jsonTlsSize = sz;
}

static void * cJSON_malloc_fn(size_t sz)
{
	void * lpvData = jsonTlsPtr;
	size_t size = jsonTlsSize;
	if (sz > size)
		return NULL;
	char * new_ptr = (char*)(lpvData);
	new_ptr += sz;
	size -= sz;
	SET_CJSON_ALLOCATOR(new_ptr, size);
	return lpvData;
}

static void cJSON_free_fn(void *ptr)
{
	/*	It's simple continuous allocator that will 'forget' everything at once
		by moving the current pointer back to the start of the buffer.
		See: RESET_CJSON_ALLOCATOR()
		So it does not need to call cJSON_Delete() and that's why free() function
		hook is empty. */
}

#define RESET_CJSON_ALLOCATOR() SET_CJSON_ALLOCATOR(pConsumerThread->jsonAllocatorPtr, pConsumerThread->jsonAllocatorSize);

RsslReactorCallbackRet channelEventCallback(RsslReactor *pReactor, RsslReactorChannel *pReactorChannel, RsslReactorChannelEvent *pConnEvent);
RsslReactorCallbackRet loginMsgCallback(RsslReactor *pReactor, RsslReactorChannel *pReactorChannel, RsslRDMLoginMsgEvent *pLoginMsgEvent);
RsslReactorCallbackRet directoryMsgCallback(RsslReactor *pReactor, RsslReactorChannel *pReactorChannel, RsslRDMDirectoryMsgEvent *pDirectoryMsgEvent);
RsslReactorCallbackRet dictionaryMsgCallback(RsslReactor *pReactor, RsslReactorChannel *pReactorChannel, RsslRDMDictionaryMsgEvent *pDictionaryMsgEvent);
RsslReactorCallbackRet defaultMsgCallback(RsslReactor *pReactor, RsslReactorChannel *pReactorChannel, RsslMsgEvent* pMsgEvent);

RsslRet serviceNameToIdCallback(RsslBuffer* pServiceName, void* userSpecPtr, RsslUInt16* pServiceId)
{
	ConsumerThread *pConsumerThread = (ConsumerThread *)userSpecPtr;

	if (pConsumerThread->pDesiredService && pConsumerThread->pDesiredService->serviceId)
	{
		if (pConsumerThread->pDesiredService->info.serviceName.length == pServiceName->length &&
			(strncmp(pConsumerThread->pDesiredService->info.serviceName.data, 
						pServiceName->data, pServiceName->length) == 0) )
		{
			*pServiceId = (RsslUInt16)pConsumerThread->pDesiredService->serviceId;
			return RSSL_RET_SUCCESS;
		}
	}

	fprintf(stderr, "Failed to convert service name to Id in callback\n");
	return RSSL_RET_FAILURE;
}

RsslRet serviceNameToIdReactorCallback(RsslReactor *pReactor, RsslBuffer* pServiceName, RsslUInt16* pServiceId, RsslReactorServiceNameToIdEvent* pEvent)
{
	/*
	ConsumerThread *pConsumerThread = (ConsumerThread *)pEvent->pUserSpec;
	if (pConsumerThread->pDesiredService && pConsumerThread->pDesiredService->serviceId)
	{
		if (pConsumerThread->pDesiredService->info.serviceName.length == pServiceName->length &&
			(strncmp(pConsumerThread->pDesiredService->info.serviceName.data, 
						pServiceName->data, pServiceName->length) == 0) )
		{
			*pServiceId = (RsslUInt16)pConsumerThread->pDesiredService->serviceId;
			return RSSL_RET_SUCCESS;
		}
	}
	return RSSL_RET_FAILURE;
	*/
	return (serviceNameToIdCallback(pServiceName, pEvent->pUserSpec, pServiceId));
}

RsslRet RTR_C_INLINE decodePayload(RsslDecodeIterator* dIter, RsslMsg *msg, ConsumerThread* pConsumerThread)
{
	RsslRet ret;

	if (consPerfConfig.latencyIncludeJSONConversion == RSSL_TRUE
		&& pConsumerThread->rjcSess.jsonConverterInitialized == RSSL_TRUE)
	{
		RsslErrorInfo eInfo;

		ret = rjcMsgConvertToJsonEst(&pConsumerThread->rjcSess, pConsumerThread->pChannel->majorVersion, pConsumerThread->pChannel->minorVersion, msg, &eInfo);
		if (ret < RSSL_RET_SUCCESS)
		{
			rsslSetErrorInfo(&pConsumerThread->threadErrorInfo, RSSL_EIC_FAILURE, ret, __FILE__, __LINE__,
					(char*)"rjcMsgConvertToJsonEst() failed: %d(%s)", ret, rsslRetCodeToString(ret));
			return ret;
		}
	}

	switch(msg->msgBase.domainType)
	{
		case RSSL_DMT_MARKET_PRICE:
			if ((ret = decodeMPUpdate(dIter, msg, pConsumerThread)) != RSSL_RET_SUCCESS)
			{
				rsslSetErrorInfo(&pConsumerThread->threadErrorInfo, RSSL_EIC_FAILURE, ret, __FILE__, __LINE__,
						(char*)"decodeMPUpdate() failed: %d(%s)", ret, rsslRetCodeToString(ret));
				return ret;
			}
			return RSSL_RET_SUCCESS;
		case RSSL_DMT_MARKET_BY_ORDER:
			if ((ret = decodeMBOUpdate(dIter, msg, pConsumerThread) != RSSL_RET_SUCCESS))
			{
				rsslSetErrorInfo(&pConsumerThread->threadErrorInfo, RSSL_EIC_FAILURE, ret, __FILE__, __LINE__,
						(char*)"decodeMBOUpdate() failed: %d(%s)", ret, rsslRetCodeToString(ret));
				return ret;
			}
			return RSSL_RET_SUCCESS;
		default:
			rsslSetErrorInfo(&pConsumerThread->threadErrorInfo, RSSL_EIC_FAILURE, RSSL_RET_FAILURE, __FILE__, __LINE__,
					(char*)"decodePayload(): Unhandled domain type %s(%d)", 
					rsslDomainTypeToString(msg->msgBase.domainType), msg->msgBase.domainType);
			return RSSL_RET_FAILURE;
	}
}

RsslRet RTR_C_INLINE decodePayloadJson(ConsumerThread* pConsumerThread, RsslDomainTypes rsslDomainType, RsslMsgClasses rsslMsgClass, cJSON* json)
{
	RsslRet ret;

	switch (rsslDomainType)
	{
	case RSSL_DMT_MARKET_PRICE:
		if ((ret = decodeMPUpdateJson(pConsumerThread, rsslMsgClass, json)) != RSSL_RET_SUCCESS)
		{
			rsslSetErrorInfo(&pConsumerThread->threadErrorInfo, RSSL_EIC_FAILURE, ret, __FILE__, __LINE__,
				(char*)"decodeMPUpdate() failed: %d(%s)", ret, rsslRetCodeToString(ret));
			return ret;
		}
		return RSSL_RET_SUCCESS;
	case RSSL_DMT_MARKET_BY_ORDER:
		if ((ret = decodeMBOUpdateJson(pConsumerThread, rsslMsgClass, json) != RSSL_RET_SUCCESS))
		{
			rsslSetErrorInfo(&pConsumerThread->threadErrorInfo, RSSL_EIC_FAILURE, ret, __FILE__, __LINE__,
				(char*)"decodeMBOUpdate() failed: %d(%s)", ret, rsslRetCodeToString(ret));
			return ret;
		}
		return RSSL_RET_SUCCESS;
	default:
		rsslSetErrorInfo(&pConsumerThread->threadErrorInfo, RSSL_EIC_FAILURE, RSSL_RET_FAILURE, __FILE__, __LINE__,
			(char*)"decodePayloadJson(): Unhandled domain type %s(%d)",
			rsslDomainTypeToString(rsslDomainType), rsslDomainType);
		return RSSL_RET_FAILURE;
	}
}

void updateTunnelStreamBufUsageStats(ConsumerThread *pConsumerThread, const char* nameMethod)
{
	RsslTunnelStream *pTunnelStream = pConsumerThread->perfTunnelMsgHandler.tunnelStreamHandler.pTunnelStream;
	if (consPerfConfig.tunnelStreamBufsUsed &&
		pConsumerThread->tunnelMessagingEnabled == RSSL_TRUE &&
		pTunnelStream != NULL)
	{
		RsslTunnelStreamInfo tunnelInfo;
		RsslErrorInfo tunnelErrorInfo;

		RsslRet retInfo = rsslTunnelStreamGetInfo(pTunnelStream, &tunnelInfo, &tunnelErrorInfo);
		if (retInfo == RSSL_RET_SUCCESS)
		{
			updateValueStatistics(&pConsumerThread->stats.tunnelStreamBufUsageStats, (double)tunnelInfo.buffersUsed);
		}
		else
		{
			printf("%s: rsslTunnelStreamGetInfo returned error retInfo = %d\n", nameMethod, retInfo);
		}
	}
	return;
}

RsslRet sendMessage(ConsumerThread *pConsumerThread, RsslBuffer *msgBuf)
{
	if (consPerfConfig.useReactor == RSSL_FALSE && consPerfConfig.useWatchlist == RSSL_FALSE)
	{
		RsslUInt32 bytesWritten, uncompBytesWritten;
		RsslRet ret;
		RsslBuffer *pBuffer;

		pBuffer = 0;

		if (pConsumerThread->pChannel->protocolType == RSSL_JSON_PROTOCOL_TYPE && consPerfConfig.convertJSON == RSSL_TRUE)
		{
			RsslErrorInfo	eInfo;

			do {
					/* convert message to JSON */
				if ((pBuffer = rjcMsgConvertToJson(&(pConsumerThread->rjcSess), 
														pConsumerThread->pChannel,
														msgBuf, &eInfo)) == NULL)
				{
					if ((ret = rsslFlush(pConsumerThread->pChannel, &pConsumerThread->threadRsslError)) < RSSL_RET_SUCCESS)
					{
						rsslSetErrorInfoLocation(&pConsumerThread->threadErrorInfo, __FILE__, __LINE__);
						return ret;
					}
				}

			} while (eInfo.rsslError.rsslErrorId == RSSL_RET_BUFFER_NO_BUFFERS);
		}
		else
			pBuffer = msgBuf;

		ret = rsslWrite(pConsumerThread->pChannel, pBuffer, 
								RSSL_HIGH_PRIORITY, 0, &bytesWritten, 
								&uncompBytesWritten, &pConsumerThread->threadRsslError);

		/* call flush and write again */
		while (rtrUnlikely(ret == RSSL_RET_WRITE_CALL_AGAIN))
		{
			if (rtrUnlikely((ret = rsslFlush(pConsumerThread->pChannel, &pConsumerThread->threadRsslError)) < RSSL_RET_SUCCESS))
			{
				rsslSetErrorInfoLocation(&pConsumerThread->threadErrorInfo, __FILE__, __LINE__);
				return ret;
			}
			ret = rsslWrite(pConsumerThread->pChannel, pBuffer, 
								RSSL_HIGH_PRIORITY, 0, &bytesWritten, 
								&uncompBytesWritten, &pConsumerThread->threadRsslError);
		}

		if (ret > RSSL_RET_SUCCESS || ret == RSSL_RET_WRITE_FLUSH_FAILED && pConsumerThread->pChannel->state == RSSL_CH_STATE_ACTIVE)
		{
			FD_SET(pConsumerThread->pChannel->socketId, &pConsumerThread->wrtfds);
		}
		else if (ret < RSSL_RET_SUCCESS)
		{
			rsslSetErrorInfoLocation(&pConsumerThread->threadErrorInfo, __FILE__, __LINE__);
			return ret;
		}
		if ( pConsumerThread->pChannel->protocolType == RSSL_JSON_PROTOCOL_TYPE && consPerfConfig.convertJSON == RSSL_TRUE )
			rsslReleaseBuffer(msgBuf, &pConsumerThread->threadRsslError);
	}
	else if (pConsumerThread->tunnelMessagingEnabled == RSSL_TRUE  &&
			pConsumerThread->perfTunnelMsgHandler.tunnelStreamHandler.pTunnelStream != NULL)
	{
		RsslErrorInfo rsslErrorInfo;
		RsslRet	retval = 0;

		RsslTunnelStream *pTunnelStream = pConsumerThread->perfTunnelMsgHandler.tunnelStreamHandler.pTunnelStream;

		RsslTunnelStreamSubmitOptions rsslTunnelStreamSubmitOptions;
		rsslClearTunnelStreamSubmitOptions(&rsslTunnelStreamSubmitOptions);
		rsslTunnelStreamSubmitOptions.containerType = RSSL_DT_MSG;

		retval = rsslTunnelStreamSubmit(pTunnelStream, msgBuf, &rsslTunnelStreamSubmitOptions, &rsslErrorInfo);

		updateTunnelStreamBufUsageStats(pConsumerThread, "sendMessage");

		if (retval != RSSL_RET_SUCCESS)
		{
			printf("rsslTunnelStreamSubmit() failed with return code %d - <%s>\n", retval, rsslErrorInfo.rsslError.text);
			if (retval = rsslTunnelStreamReleaseBuffer(msgBuf, &rsslErrorInfo) != RSSL_RET_SUCCESS)
			{
				printf("rsslTunnelStreamReleaseBuffer() failed with return code %d - <%s>\n", retval, rsslErrorInfo.rsslError.text);
			}
			return RSSL_RET_FAILURE;
		}
	}
	else
	{
		RsslErrorInfo rsslErrorInfo;
		RsslRet	retval = 0;
		RsslUInt32 outBytes = 0;
		RsslUInt32 uncompOutBytes = 0;
		RsslUInt8 writeFlags = RSSL_WRITE_NO_FLAGS;
		RsslReactorSubmitOptions submitOpts;

		rsslClearReactorSubmitOptions(&submitOpts);

		/* send the request */
		if ((retval = rsslReactorSubmit(pConsumerThread->pReactor, pConsumerThread->pReactorChannel, msgBuf, &submitOpts, &rsslErrorInfo)) < RSSL_RET_SUCCESS)
		{
			while (retval == RSSL_RET_WRITE_CALL_AGAIN)
				retval = rsslReactorSubmit(pConsumerThread->pReactor, pConsumerThread->pReactorChannel, msgBuf, &submitOpts, &rsslErrorInfo);

			if (retval < RSSL_RET_SUCCESS)	/* Connection should be closed, return failure */
			{
				/* rsslWrite failed, release buffer */
				printf("rsslReactorSubmit() failed with return code %d - <%s>\n", retval, rsslErrorInfo.rsslError.text);
				if ((retval = rsslReactorReleaseBuffer(pConsumerThread->pReactorChannel, msgBuf, &rsslErrorInfo)) != RSSL_RET_SUCCESS)
					printf("rsslReactorReleaseBuffer() failed with return code %d - <%s>\n", retval, rsslErrorInfo.rsslError.text);

				return RSSL_RET_FAILURE;
			}
		}
	}

	return RSSL_RET_SUCCESS;
}

/* Sends a basic Login Request on the given streamID, using the RDM package. */
RsslRet sendLoginRequest(ConsumerThread *pConsumerThread, RsslInt32 streamId)
{
	RsslRDMLoginRequest loginRequest;
	RsslRet ret;
	RsslBuffer *msgBuf;
	RsslEncodeIterator eIter;

	/* Send Login Request */
	if ((ret = rsslInitDefaultRDMLoginRequest(&loginRequest, streamId)) != RSSL_RET_SUCCESS)
	{
		rsslSetErrorInfo(&pConsumerThread->threadErrorInfo, RSSL_EIC_FAILURE, RSSL_RET_FAILURE, __FILE__, __LINE__,
				(char*)"rsslInitDefaultRDMLoginRequest() failed: %d(%s)", ret, rsslRetCodeToString(ret));
		return ret;
	}

	if (strlen(consPerfConfig.username))
	{
		loginRequest.userName.data = consPerfConfig.username;
		loginRequest.userName.length = (RsslUInt32)strlen(consPerfConfig.username);
	}

	loginRequest.flags |= RDM_LG_RQF_HAS_ROLE | RDM_LG_RQF_HAS_APPLICATION_NAME;
	loginRequest.applicationName = applicationName;

	if (!(msgBuf = rsslGetBuffer(pConsumerThread->pChannel, 128, RSSL_FALSE, &pConsumerThread->threadRsslError)))
	{
		rsslSetErrorInfoLocation(&pConsumerThread->threadErrorInfo, __FILE__, __LINE__);
		return pConsumerThread->threadRsslError.rsslErrorId;
	}

	rsslClearEncodeIterator(&eIter);
	rsslSetEncodeIteratorRWFVersion(&eIter, pConsumerThread->pChannel->majorVersion, pConsumerThread->pChannel->minorVersion);
	if ( (ret = rsslSetEncodeIteratorBuffer(&eIter, msgBuf)) != RSSL_RET_SUCCESS)
	{
		rsslSetErrorInfo(&pConsumerThread->threadErrorInfo, RSSL_EIC_FAILURE, RSSL_RET_FAILURE, __FILE__, __LINE__,
				(char*)"rsslSetEncodeIteratorBuffer() failed: %d(%s)", ret, rsslRetCodeToString(ret));
		return ret;
	}

	if ((ret = rsslEncodeRDMLoginMsg(&eIter, (RsslRDMLoginMsg*)&loginRequest, &msgBuf->length, &pConsumerThread->threadErrorInfo)) != RSSL_RET_SUCCESS)
		 return ret;

	if ( sendMessage(pConsumerThread, msgBuf) != RSSL_RET_SUCCESS)
		 return ret;

	return RSSL_RET_SUCCESS;
}

/* Sends a basic JSON Login Request on the given streamID, using the RDM package. */
RsslRet sendLoginRequestJson(ConsumerThread *pConsumerThread, RsslInt32 streamId)
{
	RsslRDMLoginRequest loginRequest;
	RsslRet ret;
	const char *username;
	RsslBuffer *msgBuf;
	cJSON *json, *jsonKey, *jsonElements;

	/* Send Login Request */
	if ((ret = rsslInitDefaultRDMLoginRequest(&loginRequest, streamId)) != RSSL_RET_SUCCESS)
	{
		rsslSetErrorInfo(&pConsumerThread->threadErrorInfo, RSSL_EIC_FAILURE, RSSL_RET_FAILURE, __FILE__, __LINE__,
			(char*)"rsslInitDefaultRDMLoginRequest() failed: %d(%s)", ret, rsslRetCodeToString(ret));
		return ret;
	}

	if (!(msgBuf = rsslGetBuffer(pConsumerThread->pChannel, 512, RSSL_FALSE, &pConsumerThread->threadRsslError)))
	{
		rsslSetErrorInfoLocation(&pConsumerThread->threadErrorInfo, __FILE__, __LINE__);
		return pConsumerThread->threadRsslError.rsslErrorId;
	}

	if (strlen(consPerfConfig.username))
		username = consPerfConfig.username;
	else
		username = loginRequest.defaultUsername;

	// {"ID":1,"Type":"Request","Domain":"Login","KeyInUpdates":false,"Key":{"Name":"username","Elements":{"ApplicationId":"256","ApplicationName":"ConsPerf","Position":"10.46.188.76/net","Role":0}}}
	json = cJSON_CreateObject();
	cJSON_AddNumberToObject(json, "ID", 1.);
	cJSON_AddStringToObject(json, "Type", "Request");
	cJSON_AddStringToObject(json, "Domain", "Login");
	cJSON_AddFalseToObject(json, "KeyInUpdates");
	jsonKey = cJSON_AddObjectToObject(json, "Key");
	cJSON_AddStringToObject(jsonKey, "Name", username);
	jsonElements = cJSON_AddObjectToObject(jsonKey, "Elements");
	cJSON_AddStringToObject(jsonElements, "ApplicationId", loginRequest.applicationId.data);
	cJSON_AddStringToObject(jsonElements, "ApplicationName", applicationName.data);
	cJSON_AddStringToObject(jsonElements, "Position", loginRequest.position.data);
	cJSON_AddNumberToObject(jsonElements, "Role", 0.);

	cJSON_PrintPreallocated(json, msgBuf->data, msgBuf->length, 0);

	msgBuf->length = (rtrUInt32)strnlen(msgBuf->data, 512);

	//cJSON_Delete(json);
	RESET_CJSON_ALLOCATOR();

	if ((ret = sendMessage(pConsumerThread, msgBuf)) != RSSL_RET_SUCCESS)
		return ret;

	return RSSL_RET_SUCCESS;
}

/* Sends a basic Directory Request on the given streamID, using the RDM package. */
RsslRet sendDirectoryRequest(ConsumerThread *pConsumerThread, RsslInt32 streamId)
{
	RsslRDMDirectoryRequest directoryRequest;
	RsslRet ret;
	RsslBuffer *msgBuf;
	RsslEncodeIterator eIter;

	/* Send Directory Request */
	if ((ret = rsslInitDefaultRDMDirectoryRequest(&directoryRequest, streamId)) != RSSL_RET_SUCCESS)
	{
		rsslSetErrorInfo(&pConsumerThread->threadErrorInfo, RSSL_EIC_FAILURE, RSSL_RET_FAILURE, __FILE__, __LINE__,
				(char*)"rsslInitDefaultRDMDirectoryRequest() failed: %d(%s)", ret, rsslRetCodeToString(ret));
		return ret;
	}

	if (!(msgBuf = rsslGetBuffer(pConsumerThread->pChannel, 128, RSSL_FALSE, &pConsumerThread->threadRsslError)))
	{
		rsslSetErrorInfoLocation(&pConsumerThread->threadErrorInfo, __FILE__, __LINE__);
		return pConsumerThread->threadRsslError.rsslErrorId;
	}

	rsslClearEncodeIterator(&eIter);
	rsslSetEncodeIteratorRWFVersion(&eIter, pConsumerThread->pChannel->majorVersion, pConsumerThread->pChannel->minorVersion);
	if ( (ret = rsslSetEncodeIteratorBuffer(&eIter, msgBuf)) != RSSL_RET_SUCCESS)
	{
		rsslSetErrorInfo(&pConsumerThread->threadErrorInfo, RSSL_EIC_FAILURE, RSSL_RET_FAILURE, __FILE__, __LINE__,
				(char*)"rsslSetEncodeIteratorBuffer() failed: %d(%s)", ret, rsslRetCodeToString(ret));
		return ret;
	}

	if ((ret = rsslEncodeRDMDirectoryMsg(&eIter, (RsslRDMDirectoryMsg*)&directoryRequest, &msgBuf->length, &pConsumerThread->threadErrorInfo)) != RSSL_RET_SUCCESS)
		return ret;

	if ( sendMessage(pConsumerThread, msgBuf) != RSSL_RET_SUCCESS)
		return ret;

	return RSSL_RET_SUCCESS;
}

/* Sends a basic Directory Request on the given streamID, using the RDM package. */
RsslRet sendDirectoryRequestJson(ConsumerThread* pConsumerThread, RsslInt32 streamId)
{
	RsslRDMDirectoryRequest directoryRequest;
	RsslRet ret;
	RsslBuffer *msgBuf;
	cJSON *json, *jsonKey;

	/* Send Directory Request */
	if ((ret = rsslInitDefaultRDMDirectoryRequest(&directoryRequest, streamId)) != RSSL_RET_SUCCESS)
	{
		rsslSetErrorInfo(&pConsumerThread->threadErrorInfo, RSSL_EIC_FAILURE, RSSL_RET_FAILURE, __FILE__, __LINE__,
			(char*)"rsslInitDefaultRDMDirectoryRequest() failed: %d(%s)", ret, rsslRetCodeToString(ret));
		return ret;
	}

	if (!(msgBuf = rsslGetBuffer(pConsumerThread->pChannel, 512, RSSL_FALSE, &pConsumerThread->threadRsslError)))
	{
		rsslSetErrorInfoLocation(&pConsumerThread->threadErrorInfo, __FILE__, __LINE__);
		return pConsumerThread->threadRsslError.rsslErrorId;
	}

	// {"ID":2,"Type":"Request","Domain":"Source","KeyInUpdates":false,"Key":{"Filter":7}}
	json = cJSON_CreateObject();
	cJSON_AddNumberToObject(json, "ID", 2.);
	cJSON_AddStringToObject(json, "Type", "Request");
	cJSON_AddStringToObject(json, "Domain", "Source");
	cJSON_AddFalseToObject(json, "KeyInUpdates");
	jsonKey = cJSON_AddObjectToObject(json, "Key");
	cJSON_AddNumberToObject(jsonKey, "Filter", 7.);

	cJSON_PrintPreallocated(json, msgBuf->data, msgBuf->length, 0);

	msgBuf->length = (rtrUInt32)strnlen(msgBuf->data, 512);

	//cJSON_Delete(json);
	RESET_CJSON_ALLOCATOR();

	if (sendMessage(pConsumerThread, msgBuf) != RSSL_RET_SUCCESS)
		return ret;

	return RSSL_RET_SUCCESS;
}

RsslRet sendDictionaryRequests(ConsumerThread *pConsumerThread, RsslUInt16 serviceId, 
		RsslInt32 fieldDictionaryStreamId, RsslInt32 enumTypeDictionaryStreamId)
{
	RsslRDMDictionaryRequest dictionaryRequest;
	RsslRet ret;
	RsslBuffer *msgBuf;
	RsslEncodeIterator eIter;

	rsslClearRDMDictionaryRequest(&dictionaryRequest);
	dictionaryRequest.flags = RDM_DC_RQF_STREAMING;
	dictionaryRequest.serviceId = serviceId;


	/* Field dictionary request. */
	if (!(msgBuf = rsslGetBuffer(pConsumerThread->pChannel, 128, RSSL_FALSE, &pConsumerThread->threadRsslError)))
		return pConsumerThread->threadRsslError.rsslErrorId;

	dictionaryRequest.dictionaryName.data = (char*)"RWFFld";
	dictionaryRequest.dictionaryName.length = 6;
	dictionaryRequest.rdmMsgBase.streamId = fieldDictionaryStreamId;
	rsslClearEncodeIterator(&eIter);
	rsslSetEncodeIteratorRWFVersion(&eIter, pConsumerThread->pChannel->majorVersion, pConsumerThread->pChannel->minorVersion);
	if ( (ret = rsslSetEncodeIteratorBuffer(&eIter, msgBuf)) != RSSL_RET_SUCCESS)
	{
		rsslSetErrorInfo(&pConsumerThread->threadErrorInfo, RSSL_EIC_FAILURE, ret, __FILE__, __LINE__,
				(char*)"rsslSetEncodeIteratorBuffer() failed: %d(%s)", ret, rsslRetCodeToString(ret));
		return ret;
	}

	if ((ret = rsslEncodeRDMDictionaryMsg(&eIter, (RsslRDMDictionaryMsg*)&dictionaryRequest, &msgBuf->length, &pConsumerThread->threadErrorInfo)) != RSSL_RET_SUCCESS)
		return ret;

	if ( sendMessage(pConsumerThread, msgBuf) != RSSL_RET_SUCCESS)
		 return ret;

	/* Enumerated types dictionary request. */
	if (!(msgBuf = rsslGetBuffer(pConsumerThread->pChannel, 128, RSSL_FALSE, &pConsumerThread->threadRsslError)))
	{
		rsslSetErrorInfoLocation(&pConsumerThread->threadErrorInfo, __FILE__, __LINE__);
		return pConsumerThread->threadRsslError.rsslErrorId;
	}

	dictionaryRequest.dictionaryName.data = (char*)"RWFEnum";
	dictionaryRequest.dictionaryName.length = 7;
	dictionaryRequest.rdmMsgBase.streamId = enumTypeDictionaryStreamId;
	rsslClearEncodeIterator(&eIter);
	rsslSetEncodeIteratorRWFVersion(&eIter, pConsumerThread->pChannel->majorVersion, pConsumerThread->pChannel->minorVersion);
	if ( (ret = rsslSetEncodeIteratorBuffer(&eIter, msgBuf)) != RSSL_RET_SUCCESS)
	{
		rsslSetErrorInfo(&pConsumerThread->threadErrorInfo, RSSL_EIC_FAILURE, RSSL_RET_FAILURE, __FILE__, __LINE__, 
				(char*)"rsslSetEncodeIteratorBuffer() failed: %d(%s)", ret, rsslRetCodeToString(ret));
		return ret;
	}

	if ((ret = rsslEncodeRDMDictionaryMsg(&eIter, (RsslRDMDictionaryMsg*)&dictionaryRequest, &msgBuf->length, &pConsumerThread->threadErrorInfo)) != RSSL_RET_SUCCESS)
		 return ret;

	if ( sendMessage(pConsumerThread, msgBuf) != RSSL_RET_SUCCESS)
		 return ret;

	return RSSL_RET_SUCCESS;
}

RsslRet sendWatchlistDictionaryRequests(ConsumerThread *pConsumerThread, RsslUInt16 serviceId, 
		RsslInt32 fieldDictionaryStreamId, RsslInt32 enumTypeDictionaryStreamId)
{
	RsslRDMDictionaryRequest dictionaryRequest;
	RsslRet ret;
	RsslReactorSubmitMsgOptions submitMsgOpts;
	RsslErrorInfo rsslErrorInfo;

	rsslClearRDMDictionaryRequest(&dictionaryRequest);
	dictionaryRequest.flags = RDM_DC_RQF_STREAMING;
	dictionaryRequest.serviceId = serviceId;

	/* Field dictionary request. */
	dictionaryRequest.dictionaryName.data = (char*)"RWFFld";
	dictionaryRequest.dictionaryName.length = 6;
	dictionaryRequest.rdmMsgBase.streamId = fieldDictionaryStreamId;

	rsslClearReactorSubmitMsgOptions(&submitMsgOpts);

	// submit message to VA Reactor Watchlist
	submitMsgOpts.pRDMMsg = (RsslRDMMsg *)&dictionaryRequest;
	if ((ret = rsslReactorSubmitMsg(pConsumerThread->pReactor, pConsumerThread->pReactorChannel, &submitMsgOpts, &rsslErrorInfo)) != RSSL_RET_SUCCESS)
	{
		printf("rsslReactorSubmitMsg() failed: %d(%s)\n", ret, rsslErrorInfo.rsslError.text);
		return ret;
	}

	/* Enumerated types dictionary request. */
	dictionaryRequest.dictionaryName.data = (char*)"RWFEnum";
	dictionaryRequest.dictionaryName.length = 7;
	dictionaryRequest.rdmMsgBase.streamId = enumTypeDictionaryStreamId;

	rsslClearReactorSubmitMsgOptions(&submitMsgOpts);

	// submit message to VA Reactor Watchlist
	submitMsgOpts.pRDMMsg = (RsslRDMMsg *)&dictionaryRequest;
	if ((ret = rsslReactorSubmitMsg(pConsumerThread->pReactor, pConsumerThread->pReactorChannel, &submitMsgOpts, &rsslErrorInfo)) != RSSL_RET_SUCCESS)
	{
		printf("rsslReactorSubmitMsg() failed: %d(%s)\n", ret, rsslErrorInfo.rsslError.text);
		return ret;
	}

	return RSSL_RET_SUCCESS;
}

RsslRet sendItemRequestBurst(ConsumerThread *pConsumerThread, RsslUInt32 itemBurstCount)
{
	RsslRet ret;
	RsslRequestMsg requestMsg;
	RsslUInt32 i;
	RsslQos *pQos;
	RsslBool useTunnel = RSSL_FALSE;
	RsslTunnelStreamGetBufferOptions bufferTunnelOpts;

	assert(pConsumerThread->pDesiredService);
	assert(itemBurstCount);

	/* Use a QoS from the service, if one is given. */
	if ((pConsumerThread->pDesiredService->flags & RDM_SVCF_HAS_INFO) && (pConsumerThread->pDesiredService->info.flags & RDM_SVC_IFF_HAS_QOS ) && (pConsumerThread->pDesiredService->info.qosCount > 0))
		pQos = &pConsumerThread->pDesiredService->info.qosList[0];
	else
		pQos = NULL;

	if (pConsumerThread->tunnelMessagingEnabled == RSSL_TRUE &&
		pConsumerThread->perfTunnelMsgHandler.tunnelStreamHandler.pTunnelStream != NULL)
	{
		useTunnel = RSSL_TRUE;
	}

	for(i = 0; i < itemBurstCount; ++i)
	{
		RsslQueueLink *pLink;
		ItemRequest *pItemRequest;

	   	pLink = rsslQueuePeekFront(&pConsumerThread->requestQueue);
		if (!pLink) return RSSL_RET_SUCCESS;

		pItemRequest = RSSL_QUEUE_LINK_TO_OBJECT(ItemRequest, link, pLink);
		assert(pItemRequest->requestState == ITEM_NOT_REQUESTED);

		/* Desired service has been found, so add it to the msgKey. */
		pItemRequest->msgKey.flags |= RSSL_MKF_HAS_SERVICE_ID;
		pItemRequest->msgKey.serviceId = (RsslUInt16)pConsumerThread->pDesiredService->serviceId;

		rsslClearRequestMsg(&requestMsg);
		if (!consPerfConfig.requestSnapshots && pItemRequest->itemInfo.itemFlags & ITEM_IS_STREAMING_REQ)
			requestMsg.flags |= RSSL_RQMF_STREAMING;

		if (pQos)
		{
			requestMsg.flags |= RSSL_RQMF_HAS_QOS;
			requestMsg.qos = *pQos;
		}

		if (pItemRequest->itemInfo.itemFlags & ITEM_IS_GEN_MSG)
			requestMsg.flags |= RSSL_RQMF_PRIVATE_STREAM;

		requestMsg.msgBase.streamId = pItemRequest->itemInfo.StreamId;
		requestMsg.msgBase.domainType = pItemRequest->itemInfo.attributes.domainType;
		requestMsg.msgBase.containerType = RSSL_DT_NO_DATA;

		requestMsg.msgBase.msgKey = pItemRequest->msgKey;

		if (consPerfConfig.useWatchlist == RSSL_FALSE) // VA Reactor Watchlist not enabled
		{
			RsslBuffer *msgBuf;
			RsslEncodeIterator eIter;

			if (useTunnel == RSSL_TRUE)
			{
				RsslTunnelStream *pTunnelStream = pConsumerThread->perfTunnelMsgHandler.tunnelStreamHandler.pTunnelStream;
				const RsslUInt64 nIter = countStatGetTotal(&pConsumerThread->stats.requestCount);
				rsslClearTunnelStreamGetBufferOptions(&bufferTunnelOpts);
				bufferTunnelOpts.size = 512;

				if ( (msgBuf = rsslTunnelStreamGetBuffer(pTunnelStream, &bufferTunnelOpts, &pConsumerThread->threadErrorInfo)) == NULL)
				{
					if (pConsumerThread->threadRsslError.rsslErrorId == RSSL_RET_BUFFER_NO_BUFFERS)
					{
						printf("sendItemRequestBurst: rsslTunnelStreamGetBuffer returned (warning) NO_BUFFERS  nIter=%llu\n", nIter);
						pConsumerThread->threadRsslError.rsslErrorId = RSSL_RET_SUCCESS;
						return RSSL_RET_SUCCESS;
					}
					else
					{
						printf("sendItemRequestBurst: rsslTunnelStreamGetBuffer returned Error  nIter=%llu\n", nIter);
						rsslSetErrorInfoLocation(&pConsumerThread->threadErrorInfo, __FILE__, __LINE__);
						return pConsumerThread->threadRsslError.rsslErrorId;
					}
				}
			}
			else if (!(msgBuf = rsslGetBuffer(pConsumerThread->pChannel, 512, RSSL_FALSE, &pConsumerThread->threadRsslError)))
			{
				if (pConsumerThread->threadRsslError.rsslErrorId == RSSL_RET_BUFFER_NO_BUFFERS)
				{
					pConsumerThread->threadRsslError.rsslErrorId = RSSL_RET_SUCCESS;
					return RSSL_RET_SUCCESS;
				}
				else
				{
					rsslSetErrorInfoLocation(&pConsumerThread->threadErrorInfo, __FILE__, __LINE__);
					return pConsumerThread->threadRsslError.rsslErrorId;
				}
			}

			rsslClearEncodeIterator(&eIter);
			rsslSetEncodeIteratorRWFVersion(&eIter, pConsumerThread->pChannel->majorVersion, pConsumerThread->pChannel->minorVersion);
			if ( (ret = rsslSetEncodeIteratorBuffer(&eIter, msgBuf)) != RSSL_RET_SUCCESS)
			{
				rsslSetErrorInfo(&pConsumerThread->threadErrorInfo, RSSL_EIC_FAILURE, ret, __FILE__, __LINE__,
						(char*)"rsslSetEncodeIteratorBuffer() failed: %d(%s)", ret, pConsumerThread->threadErrorInfo.rsslError.text);
				return ret;
			}

			/* Encode request msg. */
			if ((ret = rsslEncodeMsg(&eIter, (RsslMsg*)&requestMsg)) != RSSL_RET_SUCCESS)
			{
				rsslSetErrorInfo(&pConsumerThread->threadErrorInfo, RSSL_EIC_FAILURE, ret, __FILE__, __LINE__,
						(char*)"rsslEncodeMsg() failed: %d.", ret);
				return ret;
			}

			msgBuf->length = rsslGetEncodedBufferLength(&eIter);
		

			if ( sendMessage(pConsumerThread, msgBuf) != RSSL_RET_SUCCESS)
				return ret;
		}
		else // VA Reactor Watchlist is enabled, submit message instead of buffer
		{
			RsslErrorInfo rsslErrorInfo;
			if (useTunnel == RSSL_TRUE)
			{
				RsslTunnelStream *pTunnelStream = pConsumerThread->perfTunnelMsgHandler.tunnelStreamHandler.pTunnelStream;
				RsslTunnelStreamSubmitMsgOptions submitTunnelMsgOpts;
				rsslClearTunnelStreamSubmitMsgOptions(&submitTunnelMsgOpts);

				submitTunnelMsgOpts.pRsslMsg = (RsslMsg *)&requestMsg;
				ret = rsslTunnelStreamSubmitMsg(pTunnelStream, &submitTunnelMsgOpts, &rsslErrorInfo);
				
				updateTunnelStreamBufUsageStats(pConsumerThread, "sendItemRequestBurst");

				if (ret != RSSL_RET_SUCCESS)
				{
					printf("rsslTunnelStreamSubmitMsg() failed: %d(%s)\n", ret, rsslErrorInfo.rsslError.text);
					return ret;
				}
			}
			else
			{
				RsslReactorSubmitMsgOptions submitMsgOpts;

				rsslClearReactorSubmitMsgOptions(&submitMsgOpts);

				// submit message to VA Reactor Watchlist
				submitMsgOpts.pRsslMsg = (RsslMsg *)&requestMsg;
				if ((ret = rsslReactorSubmitMsg(pConsumerThread->pReactor, pConsumerThread->pReactorChannel, &submitMsgOpts, &rsslErrorInfo)) != RSSL_RET_SUCCESS)
				{
					printf("rsslReactorSubmitMsg() failed: %d(%s)\n", ret, rsslErrorInfo.rsslError.text);
					return ret;
				}
			}
		}

		/* Requests has been made, move the link. */
		pItemRequest->requestState = ITEM_WAITING_FOR_REFRESH;
	   	rsslQueueRemoveFirstLink(&pConsumerThread->requestQueue);
		rsslQueueAddLinkToBack(&pConsumerThread->waitingForRefreshQueue, pLink);

		countStatIncr(&pConsumerThread->stats.requestCount);
	}

	return RSSL_RET_SUCCESS;
}

RsslRet sendItemRequestBurstJson(ConsumerThread *pConsumerThread, RsslUInt32 itemBurstCount)
{
	RsslRet ret;
	RsslUInt32 i;
	RsslQos *pQos;
	cJSON *json, *jsonKey, *jsonQos;
	ItemRequest *pItemRequest;
	char itemNameBuf[sizeof(pItemRequest->itemNameChar)];

	/* Use a QoS from the service, if one is given. */
	//pQos = &pConsumerThread->pDesiredService->info.qosList[0];
	pQos = NULL;

	for (i = 0; i < itemBurstCount; ++i)
	{
		RsslQueueLink *pLink;
		RsslBuffer *msgBuf;

		pLink = rsslQueuePeekFront(&pConsumerThread->requestQueue);
		if (!pLink) return RSSL_RET_SUCCESS;

		pItemRequest = RSSL_QUEUE_LINK_TO_OBJECT(ItemRequest, link, pLink);
		assert(pItemRequest->requestState == ITEM_NOT_REQUESTED);

		if (!(msgBuf = rsslGetBuffer(pConsumerThread->pChannel, 512, RSSL_FALSE, &pConsumerThread->threadRsslError)))
		{
			if (pConsumerThread->threadRsslError.rsslErrorId == RSSL_RET_BUFFER_NO_BUFFERS)
			{
				pConsumerThread->threadRsslError.rsslErrorId = RSSL_RET_SUCCESS;
				return RSSL_RET_SUCCESS;
			}
			else
			{
				rsslSetErrorInfoLocation(&pConsumerThread->threadErrorInfo, __FILE__, __LINE__);
				return pConsumerThread->threadRsslError.rsslErrorId;
			}
		}

		// {"ID":7,"Type":"Request","KeyInUpdates":false,"Qos":{"Timeliness":"Realtime","Rate":"TickByTick"},"Key":{"Service":1,"Name":"RDT2"}}

		/* Desired service has been found, so add it to the msgKey. */
		pItemRequest->msgKey.flags |= RSSL_MKF_HAS_SERVICE_ID;
		pItemRequest->msgKey.serviceId = (RsslUInt16)pConsumerThread->jsonDesiredServiceId;

//+		if (!consPerfConfig.requestSnapshots && pItemRequest->itemInfo.itemFlags & ITEM_IS_STREAMING_REQ)
//+			requestMsg.flags |= RSSL_RQMF_STREAMING;

		json = cJSON_CreateObject();
		cJSON_AddNumberToObject(json, "ID", (double)pItemRequest->itemInfo.StreamId);
		cJSON_AddStringToObject(json, "Type", "Request");
		cJSON_AddFalseToObject(json, "KeyInUpdates");
		jsonQos = cJSON_AddObjectToObject(json, "Qos");
		cJSON_AddStringToObject(jsonQos, "Timeliness", "Realtime");
		cJSON_AddStringToObject(jsonQos, "Rate", "TickByTick");
		jsonKey = cJSON_AddObjectToObject(json, "Key");
		cJSON_AddNumberToObject(jsonKey, "Service", (double)pItemRequest->msgKey.serviceId);
		strncpy(itemNameBuf, pItemRequest->msgKey.name.data, pItemRequest->msgKey.name.length);
		itemNameBuf[pItemRequest->msgKey.name.length] = '\0';
		cJSON_AddStringToObject(jsonKey, "Name", itemNameBuf);

		cJSON_PrintPreallocated(json, msgBuf->data, msgBuf->length, 0);

		msgBuf->length = (rtrUInt32)strnlen(msgBuf->data, 512);

		//cJSON_Delete(json);
		RESET_CJSON_ALLOCATOR();

		if ((ret = sendMessage(pConsumerThread, msgBuf)) != RSSL_RET_SUCCESS)
			return ret;

		/* Requests has been made, move the link. */
		pItemRequest->requestState = ITEM_WAITING_FOR_REFRESH;
		rsslQueueRemoveFirstLink(&pConsumerThread->requestQueue);
		rsslQueueAddLinkToBack(&pConsumerThread->waitingForRefreshQueue, pLink);

		countStatIncr(&pConsumerThread->stats.requestCount);
	}

	return RSSL_RET_SUCCESS;
}

RsslRet sendPostBurst(ConsumerThread *pConsumerThread, LatencyRandomArray *pRandomArray, RsslUInt32 itemBurstCount)
{
	RsslRet ret;
	RsslUInt32 i;
	RsslUInt encodeStartTime;
	RsslInt32 latencyUpdateNumber;
	RsslBool useTunnel = RSSL_FALSE;
	RsslTunnelStreamGetBufferOptions bufferTunnelOpts;

	assert(pConsumerThread->pDesiredService);
	assert(itemBurstCount);

	if (!rotatingQueueGetCount(&pConsumerThread->postItemQueue))
		return RSSL_RET_SUCCESS;

	latencyUpdateNumber = (consPerfConfig.latencyPostsPerSec > 0 && rotatingQueueGetCount(&pConsumerThread->postItemQueue)) ?
		latencyRandomArrayGetNext(pRandomArray, &pConsumerThread->randArrayIter) : -1; 

	if (pConsumerThread->tunnelMessagingEnabled == RSSL_TRUE &&
		pConsumerThread->perfTunnelMsgHandler.tunnelStreamHandler.pTunnelStream != NULL)
	{
		useTunnel = RSSL_TRUE;
	}

	for(i = 0; i < itemBurstCount; ++i)
	{
		ItemRequest *pPostItem;
		RsslQueueLink *pLink = rotatingQueueNext(&pConsumerThread->postItemQueue);

		pPostItem = RSSL_QUEUE_LINK_TO_OBJECT(ItemRequest, postQueueLink, pLink);

		assert(pPostItem->itemInfo.itemFlags & ITEM_IS_POST);

		if (latencyUpdateNumber == i)
			encodeStartTime = consPerfConfig.nanoTime ? rsslGetTimeNano() : rsslGetTimeMicro();
		else
			encodeStartTime = 0;

		if (consPerfConfig.useWatchlist == RSSL_FALSE) // VA Reactor Watchlist not enabled
		{
			RsslBuffer *msgBuf;
			RsslUInt32 requestedSize = estimateItemPostBufferLength(&pPostItem->itemInfo, pConsumerThread->pChannel->protocolType);

			if (useTunnel)
			{
				RsslTunnelStream *pTunnelStream = pConsumerThread->perfTunnelMsgHandler.tunnelStreamHandler.pTunnelStream;
				rsslClearTunnelStreamGetBufferOptions(&bufferTunnelOpts);
				bufferTunnelOpts.size = requestedSize;

				if ((msgBuf = rsslTunnelStreamGetBuffer(pTunnelStream, &bufferTunnelOpts, &pConsumerThread->threadErrorInfo)) == NULL)
				{
					if (pConsumerThread->threadRsslError.rsslErrorId == RSSL_RET_BUFFER_NO_BUFFERS)
					{
						countStatAdd(&pConsumerThread->stats.postOutOfBuffersCount, itemBurstCount - i);
						pConsumerThread->threadRsslError.rsslErrorId = RSSL_RET_SUCCESS;
						return RSSL_RET_SUCCESS;
					}
					else
					{
						rsslSetErrorInfoLocation(&pConsumerThread->threadErrorInfo, __FILE__, __LINE__);
						return pConsumerThread->threadRsslError.rsslErrorId;
					}
				}
			}
			else if (!(msgBuf = rsslGetBuffer(pConsumerThread->pChannel,
							requestedSize, RSSL_FALSE, 
							&pConsumerThread->threadRsslError)))
			{
				if (pConsumerThread->threadRsslError.rsslErrorId == RSSL_RET_BUFFER_NO_BUFFERS)
				{
					countStatAdd(&pConsumerThread->stats.postOutOfBuffersCount, itemBurstCount - i);
					pConsumerThread->threadRsslError.rsslErrorId = RSSL_RET_SUCCESS;
					return RSSL_RET_SUCCESS;
				}
				else
				{
					rsslSetErrorInfoLocation(&pConsumerThread->threadErrorInfo, __FILE__, __LINE__);
					return pConsumerThread->threadRsslError.rsslErrorId;
				}
			}


			if ((ret = encodeItemPost(pConsumerThread->pChannel, &pPostItem->itemInfo, msgBuf,
								&postUserInfo, encodeStartTime)) != RSSL_RET_SUCCESS)
			{
					rsslSetErrorInfo(&pConsumerThread->threadErrorInfo, RSSL_EIC_FAILURE, ret, __FILE__, __LINE__,
							(char*)"encodeItemPost() failed: %d(%s)", ret, rsslRetCodeToString(ret));
					return ret;
			}

			if ( sendMessage(pConsumerThread, msgBuf) != RSSL_RET_SUCCESS)
				return ret;
		}
		else // VA Reactor Watchlist is enabled, submit message instead of buffer
		{
			RsslPostMsg postMsg;
			RsslErrorInfo rsslErrorInfo;
			char bufferMemory[512];
			RsslBuffer postBuffer = {512, (char*)bufferMemory};

			// create properly encoded post message
			if ((ret = createItemPost(pConsumerThread->pChannel, &pPostItem->itemInfo, &postMsg, &postBuffer,
								&postUserInfo, encodeStartTime)) != RSSL_RET_SUCCESS)
			{
					rsslSetErrorInfo(&pConsumerThread->threadErrorInfo, RSSL_EIC_FAILURE, ret, __FILE__, __LINE__,
							(char*)"createItemPost() failed: %d(%s)", ret, rsslRetCodeToString(ret));
					return ret;
			}

			if (useTunnel == RSSL_TRUE)
			{
				RsslTunnelStream *pTunnelStream = pConsumerThread->perfTunnelMsgHandler.tunnelStreamHandler.pTunnelStream;
				RsslTunnelStreamSubmitMsgOptions submitTunnelMsgOpts;
				rsslClearTunnelStreamSubmitMsgOptions(&submitTunnelMsgOpts);

				submitTunnelMsgOpts.pRsslMsg = (RsslMsg *)&postMsg;
				ret = rsslTunnelStreamSubmitMsg(pTunnelStream, &submitTunnelMsgOpts, &rsslErrorInfo);

				updateTunnelStreamBufUsageStats(pConsumerThread, "sendPostBurst");

				if (ret != RSSL_RET_SUCCESS)
				{
					printf("rsslTunnelStreamSubmitMsg() failed: %d(%s)\n", ret, rsslErrorInfo.rsslError.text);
					return ret;
				}
			}
			else
			{
				RsslReactorSubmitMsgOptions submitMsgOpts;
				rsslClearReactorSubmitMsgOptions(&submitMsgOpts);

				// submit message to VA Reactor Watchlist
				submitMsgOpts.pRsslMsg = (RsslMsg *)&postMsg;
				if ((ret = rsslReactorSubmitMsg(pConsumerThread->pReactor, pConsumerThread->pReactorChannel, &submitMsgOpts, &rsslErrorInfo)) != RSSL_RET_SUCCESS)
				{
					printf("rsslReactorSubmitMsg() failed: %d(%s)\n", ret, rsslErrorInfo.rsslError.text);
					return ret;
				}
			}
		}

		countStatIncr(&pConsumerThread->stats.postSentCount);

	}

	return RSSL_RET_SUCCESS;
}

static int isSendPostBurstJson = 1;

RsslRet sendPostBurstJson(ConsumerThread *pConsumerThread, LatencyRandomArray *pRandomArray, RsslUInt32 itemBurstCount)
{
	assert(pConsumerThread->jsonDesiredServiceId);
	assert(itemBurstCount);

	if (!rotatingQueueGetCount(&pConsumerThread->postItemQueue))
		return RSSL_RET_SUCCESS;

	/* to show the message only once */
	if (isSendPostBurstJson)
	{
		printf("It is not supported to send JSON generic messages.\n");
		isSendPostBurstJson = 0;
	}

	return RSSL_RET_SUCCESS;
}

RsslRet sendGenMsgBurst(ConsumerThread *pConsumerThread, LatencyRandomArray *pRandomArray, RsslUInt32 itemBurstCount)
{
	RsslRet ret;
	RsslUInt32 i;
	RsslUInt encodeStartTime;
	RsslInt32 latencyGenMsgNumber;
	RsslBool useTunnel = RSSL_FALSE;
	RsslTunnelStreamGetBufferOptions bufferTunnelOpts;

	assert(pConsumerThread->pDesiredService);
	assert(itemBurstCount);

	if (!rotatingQueueGetCount(&pConsumerThread->genMsgItemQueue))
		return RSSL_RET_SUCCESS;

	latencyGenMsgNumber = (consPerfConfig.latencyGenMsgsPerSec > 0 && rotatingQueueGetCount(&pConsumerThread->genMsgItemQueue)) ?
		latencyRandomArrayGetNext(pRandomArray, &pConsumerThread->randArrayIter) : -1; 

	if (pConsumerThread->tunnelMessagingEnabled == RSSL_TRUE &&
		pConsumerThread->perfTunnelMsgHandler.tunnelStreamHandler.pTunnelStream != NULL)
	{
		useTunnel = RSSL_TRUE;
	}

	for(i = 0; i < itemBurstCount; ++i)
	{
		ItemRequest *pGenMsgItem;
		RsslQueueLink *pLink = rotatingQueueNext(&pConsumerThread->genMsgItemQueue);

		pGenMsgItem = RSSL_QUEUE_LINK_TO_OBJECT(ItemRequest, genMsgQueueLink, pLink);

		assert(pGenMsgItem->itemInfo.itemFlags & ITEM_IS_GEN_MSG);

		if (latencyGenMsgNumber == i)
		{
			countStatIncr(&pConsumerThread->stats.latencyGenMsgSentCount);
			encodeStartTime = consPerfConfig.nanoTime ? rsslGetTimeNano() : rsslGetTimeMicro();
		}
		else
			encodeStartTime = 0;

		if (consPerfConfig.useWatchlist == RSSL_FALSE) // VA Reactor Watchlist not enabled
		{
			RsslBuffer *msgBuf;
			RsslUInt32 requestedSize = estimateItemGenMsgBufferLength(&pGenMsgItem->itemInfo, pConsumerThread->pChannel->protocolType);
			if (useTunnel)
			{
				RsslTunnelStream *pTunnelStream = pConsumerThread->perfTunnelMsgHandler.tunnelStreamHandler.pTunnelStream;
				rsslClearTunnelStreamGetBufferOptions(&bufferTunnelOpts);
				bufferTunnelOpts.size = requestedSize;

				if ((msgBuf = rsslTunnelStreamGetBuffer(pTunnelStream, &bufferTunnelOpts, &pConsumerThread->threadErrorInfo)) == NULL)
				{
					if (pConsumerThread->threadRsslError.rsslErrorId == RSSL_RET_BUFFER_NO_BUFFERS)
					{
						countStatAdd(&pConsumerThread->stats.genMsgOutOfBuffersCount, itemBurstCount - i);
						pConsumerThread->threadRsslError.rsslErrorId = RSSL_RET_SUCCESS;
						return RSSL_RET_SUCCESS;
					}
					else
					{
						rsslSetErrorInfoLocation(&pConsumerThread->threadErrorInfo, __FILE__, __LINE__);
						return pConsumerThread->threadRsslError.rsslErrorId;
					}
				}
			}
			else if (!(msgBuf = rsslGetBuffer(pConsumerThread->pChannel,
							requestedSize, RSSL_FALSE, 
							&pConsumerThread->threadRsslError)))
			{
				if (pConsumerThread->threadRsslError.rsslErrorId == RSSL_RET_BUFFER_NO_BUFFERS)
				{
					countStatAdd(&pConsumerThread->stats.genMsgOutOfBuffersCount, itemBurstCount - i);
					pConsumerThread->threadRsslError.rsslErrorId = RSSL_RET_SUCCESS;
					return RSSL_RET_SUCCESS;
				}
				else
				{
					rsslSetErrorInfoLocation(&pConsumerThread->threadErrorInfo, __FILE__, __LINE__);
					return pConsumerThread->threadRsslError.rsslErrorId;
				}
			}


			if ((ret = encodeItemGenMsg(pConsumerThread->pChannel, &pGenMsgItem->itemInfo, msgBuf, encodeStartTime)) != RSSL_RET_SUCCESS)
			{
					rsslSetErrorInfo(&pConsumerThread->threadErrorInfo, RSSL_EIC_FAILURE, ret, __FILE__, __LINE__,
							(char*)"encodeItemPost() failed: %d(%s)", ret, rsslRetCodeToString(ret));
					return ret;
			}

			if ( sendMessage(pConsumerThread, msgBuf) != RSSL_RET_SUCCESS)
				return ret;
		}
		else // VA Reactor Watchlist is enabled, submit message instead of buffer
		{
			RsslGenericMsg genericMsg;
			RsslErrorInfo rsslErrorInfo;
			char bufferMemory[512];
			RsslBuffer genericBuffer = {512, (char*)bufferMemory};

			// create properly encoded generic message
			if ((ret = createItemGenMsg(pConsumerThread->pChannel, &pGenMsgItem->itemInfo, &genericMsg, &genericBuffer, encodeStartTime)) != RSSL_RET_SUCCESS)
			{
					rsslSetErrorInfo(&pConsumerThread->threadErrorInfo, RSSL_EIC_FAILURE, ret, __FILE__, __LINE__,
							(char*)"createItemGenMsg() failed: %d(%s)", ret, rsslRetCodeToString(ret));
					return ret;
			}

			if (useTunnel == RSSL_TRUE)
			{
				RsslTunnelStream *pTunnelStream = pConsumerThread->perfTunnelMsgHandler.tunnelStreamHandler.pTunnelStream;
				RsslTunnelStreamSubmitMsgOptions submitTunnelMsgOpts;
				rsslClearTunnelStreamSubmitMsgOptions(&submitTunnelMsgOpts);

				submitTunnelMsgOpts.pRsslMsg = (RsslMsg *)&genericMsg;
				ret = rsslTunnelStreamSubmitMsg(pTunnelStream, &submitTunnelMsgOpts, &rsslErrorInfo);

				updateTunnelStreamBufUsageStats(pConsumerThread, "sendGenMsgBurst");

				if (ret != RSSL_RET_SUCCESS)
				{
					printf("rsslTunnelStreamSubmitMsg() failed: %d(%s)\n", ret, rsslErrorInfo.rsslError.text);
					return ret;
				}
			}
			else
			{
				RsslReactorSubmitMsgOptions submitMsgOpts;

				rsslClearReactorSubmitMsgOptions(&submitMsgOpts);

				// submit message to VA Reactor Watchlist
				submitMsgOpts.pRsslMsg = (RsslMsg *)&genericMsg;
				if ((ret = rsslReactorSubmitMsg(pConsumerThread->pReactor, pConsumerThread->pReactorChannel, &submitMsgOpts, &rsslErrorInfo)) != RSSL_RET_SUCCESS)
				{
					printf("rsslReactorSubmitMsg() failed: %d(%s)\n", ret, rsslErrorInfo.rsslError.text);
					return ret;
				}
			}
		}

		countStatIncr(&pConsumerThread->stats.genMsgSentCount);
		if (!pConsumerThread->stats.firstGenMsgSentTime)
			pConsumerThread->stats.firstGenMsgSentTime = rsslGetTimeNano();

	}

	return RSSL_RET_SUCCESS;
}

static int isSendGenMsgBurstJson = 1;

RsslRet sendGenMsgBurstJson(ConsumerThread *pConsumerThread, LatencyRandomArray *pRandomArray, RsslUInt32 itemBurstCount)
{
	assert(pConsumerThread->jsonDesiredServiceId);
	assert(itemBurstCount);

	if (!rotatingQueueGetCount(&pConsumerThread->genMsgItemQueue))
		return RSSL_RET_SUCCESS;

	/* to show the message only once */
	if (isSendGenMsgBurstJson)
	{
		printf("It is not supported to send JSON post messages.\n");
		isSendGenMsgBurstJson = 0;
	}

	return RSSL_RET_SUCCESS;
}

static RsslRet printEstimatedPostMsgSizes(ConsumerThread *pConsumerThread)
{
	MarketPriceItem *mpItem;
	MarketByOrderItem *mboItem;
	RsslBuffer testBuffer;
	RsslRet ret;
	RsslInt32 i;
	RsslMsgKey msgKey;

	ItemInfo *pItemInfo;

	assert(pConsumerThread->pChannel);

	pItemInfo = (ItemInfo*)malloc(sizeof(ItemInfo));

	rsslClearMsgKey(&msgKey);
	msgKey.flags = RSSL_MKF_HAS_NAME_TYPE | RSSL_MKF_HAS_SERVICE_ID;
	msgKey.nameType = RDM_INSTRUMENT_NAME_TYPE_RIC;
	msgKey.name.data = (char*)"RDT0";
	msgKey.name.length = 4;
	msgKey.serviceId = 0;


	/* Market Price */
	if (xmlMsgDataHasMarketPrice)
	{

		mpItem = createMarketPriceItem();

		clearItemInfo(pItemInfo);
		pItemInfo->attributes.pMsgKey = &msgKey;
		pItemInfo->attributes.domainType = RSSL_DMT_MARKET_PRICE;
		pItemInfo->itemData = (void*)mpItem;

		if (consPerfConfig.postsPerSec)
		{
			printf("Approximate message sizes:\n");

			for (i = 0; i < getMarketPricePostMsgCount(); ++i)
			{
				testBuffer.length = estimateItemPostBufferLength(pItemInfo, RSSL_RWF_PROTOCOL_TYPE);
				testBuffer.data = (char*)malloc(testBuffer.length);
				if ((ret = encodeItemPost(pConsumerThread->pChannel, pItemInfo, &testBuffer, &postUserInfo, 0)) < RSSL_RET_SUCCESS)
				{
					rsslSetErrorInfo(&pConsumerThread->threadErrorInfo, RSSL_EIC_FAILURE, ret, __FILE__, __LINE__,
							(char*)"printEstimatedPostMsgSizes: encodeItemPost() failed: %s", rsslRetCodeToString(ret));
					return RSSL_RET_FAILURE;
				}
				printf(	"  MarketPrice PostMsg %d: \n"
						"         estimated length: %u bytes\n" 
						"    approx encoded length: %u bytes\n", 
						i+1,
						estimateItemPostBufferLength(pItemInfo, RSSL_RWF_PROTOCOL_TYPE),
						testBuffer.length);
				free(testBuffer.data);
			}
			assert(mpItem->iMsg == 0); /* encode function increments iMsg. If we've done everything right this should be 0 */

			freeMarketPriceItem(mpItem);
		}
	}

	/* Market By Order */

	if (xmlMsgDataHasMarketByOrder)
	{
		mboItem = createMarketByOrderItem();

		clearItemInfo(pItemInfo);
		pItemInfo->attributes.pMsgKey = &msgKey;
		pItemInfo->attributes.domainType = RSSL_DMT_MARKET_BY_ORDER;
		pItemInfo->itemData = (void*)mboItem;

		for (i = 0; i < getMarketByOrderPostMsgCount(); ++i)
		{
			testBuffer.length = estimateItemPostBufferLength(pItemInfo, RSSL_RWF_PROTOCOL_TYPE);
			testBuffer.data = (char *)malloc(testBuffer.length);
			if ((ret = encodeItemPost(pConsumerThread->pChannel, pItemInfo, &testBuffer, &postUserInfo, 0)) < RSSL_RET_SUCCESS)
			{
				rsslSetErrorInfo(&pConsumerThread->threadErrorInfo, RSSL_EIC_FAILURE, ret, __FILE__, __LINE__,
						(char*)"printEstimatedPostMsgSizes: encodeItemPost() failed: %s", rsslRetCodeToString(ret));
				return RSSL_RET_FAILURE;
			}
			printf(	"  MarketByOrder PostMsg %d:\n"
					"         estimated length: %u bytes\n" 
					"    approx encoded length: %u bytes\n", 
					i+1, 
					estimateItemPostBufferLength(pItemInfo, RSSL_RWF_PROTOCOL_TYPE),
					testBuffer.length);
			free(testBuffer.data);
		}
		assert(mboItem->iMsg == 0); /* encode function increments iMsg. If we've done everything right this should be 0 */

		freeMarketByOrderItem(mboItem);

		printf("\n");

	}

	free(pItemInfo);

	printf("\n");

	return RSSL_RET_SUCCESS;
}

static RsslRet printEstimatedGenMsgSizes(ConsumerThread *pConsumerThread)
{
	MarketPriceItem *mpItem;
	MarketByOrderItem *mboItem;
	RsslBuffer testBuffer;
	RsslRet ret;
	RsslInt32 i;
	RsslMsgKey msgKey;

	ItemInfo *pItemInfo;

	assert(pConsumerThread->pChannel);

	pItemInfo = (ItemInfo*)malloc(sizeof(ItemInfo));

	rsslClearMsgKey(&msgKey);
	msgKey.flags = RSSL_MKF_HAS_NAME_TYPE | RSSL_MKF_HAS_SERVICE_ID;
	msgKey.nameType = RDM_INSTRUMENT_NAME_TYPE_RIC;
	msgKey.name.data = (char*)"RDT0";
	msgKey.name.length = 4;
	msgKey.serviceId = 0;


	/* Market Price */
	if (xmlMsgDataHasMarketPrice)
	{

		mpItem = createMarketPriceItem();

		clearItemInfo(pItemInfo);
		pItemInfo->attributes.pMsgKey = &msgKey;
		pItemInfo->attributes.domainType = RSSL_DMT_MARKET_PRICE;
		pItemInfo->itemData = (void*)mpItem;

		if (consPerfConfig.genMsgsPerSec)
		{
			printf("Approximate message sizes:\n");

			for (i = 0; i < getMarketPriceGenMsgCount(); ++i)
			{
				testBuffer.length = estimateItemGenMsgBufferLength(pItemInfo, RSSL_RWF_PROTOCOL_TYPE);
				testBuffer.data = (char*)malloc(testBuffer.length);
				if ((ret = encodeItemGenMsg(pConsumerThread->pChannel, pItemInfo, &testBuffer, 0)) < RSSL_RET_SUCCESS)
				{
					rsslSetErrorInfo(&pConsumerThread->threadErrorInfo, RSSL_EIC_FAILURE, ret, __FILE__, __LINE__,
							(char*)"printEstimatedGenMsgSizes: encodeItemGenMsg() failed: %s", rsslRetCodeToString(ret));
					return RSSL_RET_FAILURE;
				}
				printf(	"  MarketPrice GenMsg %d: \n"
						"         estimated length: %u bytes\n" 
						"    approx encoded length: %u bytes\n", 
						i+1,
						estimateItemGenMsgBufferLength(pItemInfo, RSSL_RWF_PROTOCOL_TYPE),
						testBuffer.length);
				free(testBuffer.data);
			}
			assert(mpItem->iMsg == 0); /* encode function increments iMsg. If we've done everything right this should be 0 */

			freeMarketPriceItem(mpItem);
		}
	}

	/* Market By Order */

	if (xmlMsgDataHasMarketByOrder)
	{
		mboItem = createMarketByOrderItem();

		clearItemInfo(pItemInfo);
		pItemInfo->attributes.pMsgKey = &msgKey;
		pItemInfo->attributes.domainType = RSSL_DMT_MARKET_BY_ORDER;
		pItemInfo->itemData = (void*)mboItem;

		for (i = 0; i < getMarketByOrderPostMsgCount(); ++i)
		{
			testBuffer.length = estimateItemPostBufferLength(pItemInfo, RSSL_RWF_PROTOCOL_TYPE);
			testBuffer.data = (char *)malloc(testBuffer.length);
			if ((ret = encodeItemPost(pConsumerThread->pChannel, pItemInfo, &testBuffer, &postUserInfo, 0)) < RSSL_RET_SUCCESS)
			{
				rsslSetErrorInfo(&pConsumerThread->threadErrorInfo, RSSL_EIC_FAILURE, ret, __FILE__, __LINE__,
						(char*)"printEstimatedPostMsgSizes: encodeItemPost() failed: %s", rsslRetCodeToString(ret));
				return RSSL_RET_FAILURE;
			}
			printf(	"  MarketByOrder PostMsg %d:\n"
					"         estimated length: %u bytes\n" 
					"    approx encoded length: %u bytes\n", 
					i+1, 
					estimateItemPostBufferLength(pItemInfo, RSSL_RWF_PROTOCOL_TYPE),
					testBuffer.length);
			free(testBuffer.data);
		}
		assert(mboItem->iMsg == 0); /* encode function increments iMsg. If we've done everything right this should be 0 */

		freeMarketByOrderItem(mboItem);

		printf("\n");

	}

	free(pItemInfo);

	printf("\n");

	return RSSL_RET_SUCCESS;
}

static RsslRet connectChannel(ConsumerThread* pConsumerThread)
{
	RsslConnectOptions copts;
	RsslInProgInfo inProg = RSSL_INIT_IN_PROG_INFO;
	RsslRet ret = 0;
	RsslError closeError;

	FD_ZERO(&(pConsumerThread->readfds));
	FD_ZERO(&(pConsumerThread->exceptfds));
	FD_ZERO(&(pConsumerThread->wrtfds));

	rsslClearConnectOpts(&copts);
	
	/* Connect to RSSL server */
	if(strlen(consPerfConfig.interfaceName)) copts.connectionInfo.unified.interfaceName = consPerfConfig.interfaceName;

	copts.guaranteedOutputBuffers = consPerfConfig.guaranteedOutputBuffers;
	copts.numInputBuffers = consPerfConfig.numInputBuffers;
	copts.sysSendBufSize = consPerfConfig.sendBufSize;
	copts.sysRecvBufSize = consPerfConfig.recvBufSize;
	copts.connectionInfo.unified.address = consPerfConfig.hostName;
	copts.connectionInfo.unified.serviceName = consPerfConfig.portNo;
	copts.majorVersion = RSSL_RWF_MAJOR_VERSION;
	copts.minorVersion = RSSL_RWF_MINOR_VERSION;
	copts.protocolType = RSSL_RWF_PROTOCOL_TYPE;
	copts.connectionType = consPerfConfig.connectionType;

	if (consPerfConfig.connectionType == RSSL_CONN_TYPE_SOCKET || consPerfConfig.connectionType == RSSL_CONN_TYPE_ENCRYPTED)
	{
		copts.tcpOpts.tcp_nodelay = consPerfConfig.tcpNoDelay;
	}

	if (consPerfConfig.connectionType == RSSL_CONN_TYPE_WEBSOCKET || 
		(consPerfConfig.connectionType == RSSL_CONN_TYPE_ENCRYPTED && 
		 consPerfConfig.encryptedConnectionType == RSSL_CONN_TYPE_WEBSOCKET) )
		copts.wsOpts.protocols = consPerfConfig.protocolList;

	if (consPerfConfig.connectionType == RSSL_CONN_TYPE_ENCRYPTED)
	{
		copts.encryptionOpts.encryptedProtocol = consPerfConfig.encryptedConnectionType;
		if (consPerfConfig.tlsProtocolFlags != 0)
			copts.encryptionOpts.encryptionProtocolFlags = consPerfConfig.tlsProtocolFlags;
		copts.encryptionOpts.openSSLCAStore = consPerfConfig.caStore;
	}

	copts.connectionInfo.unified.address = consPerfConfig.hostName;
	copts.connectionInfo.unified.serviceName = consPerfConfig.portNo;
	copts.compressionType = consPerfConfig.compressionType;

	do
	{
		RsslError error;
		RsslChannel *pChannel;

		if (shutdownThreads)
			return RSSL_RET_FAILURE;

			printf("\nAttempting to connect to server %s:%s...\n", consPerfConfig.hostName, consPerfConfig.portNo);

		if (!CHECK_RSSLERROR((pChannel = rsslConnect(&copts,&error)), error))
		{
			SLEEP(1);
			continue;
		}

		FD_SET(pChannel->socketId,&(pConsumerThread->readfds));
		FD_SET(pChannel->socketId,&(pConsumerThread->exceptfds));

		assert(pChannel->state == RSSL_CH_STATE_INITIALIZING || pChannel->state == RSSL_CH_STATE_ACTIVE);

		/* Wait for channel to become active.  This finalizes the three-way handshake. */
		while(pChannel->state == RSSL_CH_STATE_INITIALIZING)
		{
			if (shutdownThreads)
				return RSSL_RET_FAILURE;

			ret = rsslInitChannel(pChannel, &inProg, &error);

			switch (ret)
			{
				case RSSL_RET_CHAN_INIT_IN_PROGRESS:
					if (inProg.flags & RSSL_IP_FD_CHANGE)
					{
						FD_CLR(inProg.oldSocket,&pConsumerThread->readfds);
						FD_CLR(inProg.oldSocket,&pConsumerThread->exceptfds);
						FD_SET(pChannel->socketId,&pConsumerThread->readfds);
						FD_SET(pChannel->socketId,&pConsumerThread->exceptfds);
					}
					break;
				case RSSL_RET_SUCCESS:
					/* Channel is now active. */
					assert(pChannel->state == RSSL_CH_STATE_ACTIVE);
					break;
				default:
					printf("rsslInitChannel() failed: %s(%s)\n", rsslRetCodeToString(ret),
							error.text);
					FD_CLR(pChannel->socketId,&pConsumerThread->readfds);
					FD_CLR(pChannel->socketId,&pConsumerThread->exceptfds);
					break;
			}

			if (ret < RSSL_RET_SUCCESS)
				break;
		}

		if (pChannel->state != RSSL_CH_STATE_ACTIVE)
		{
			/* Channel failed. Close and retry. */
			rsslCloseChannel(pChannel, &closeError);
			SLEEP(1);
			continue;
		}

		pConsumerThread->pChannel = pChannel;

	} while (!pConsumerThread->pChannel);

	if (consPerfConfig.highWaterMark > 0)
	{
		if (rsslIoctl(pConsumerThread->pChannel, RSSL_HIGH_WATER_MARK, &consPerfConfig.highWaterMark, &pConsumerThread->threadRsslError) != RSSL_RET_SUCCESS)
		{
			rsslSetErrorInfoLocation(&pConsumerThread->threadErrorInfo, __FILE__, __LINE__);
			rsslCloseChannel(pConsumerThread->pChannel, &closeError);
			shutdownThreads = RSSL_TRUE;
			return RSSL_RET_FAILURE;
		}
	}

	return RSSL_RET_SUCCESS;
}

static RsslRet connectReactor(ConsumerThread* pConsumerThread)
{
	RsslCreateReactorOptions reactorOpts;
	RsslReactorConnectOptions cOpts;
	RsslReactorConnectInfo cInfo;
	RsslErrorInfo rsslErrorInfo;
	RsslRet ret = 0;
	RsslReactorJsonConverterOptions jsonConverterOptions;
	RsslReactorWarmStandbyGroup			reactorWarmStandbyGroup;
	RsslReactorWarmStandbyServerInfo	standbyServerInfo;

	rsslClearReactorJsonConverterOptions(&jsonConverterOptions);
	rsslClearCreateReactorOptions(&reactorOpts);
	rsslClearReactorConnectOptions(&cOpts);
	rsslClearReactorConnectInfo(&cInfo);
	rsslClearReactorWarmStandbyGroup(&reactorWarmStandbyGroup);
	rsslClearReactorWarmStandbyServerInfo(&standbyServerInfo);

	/* The Cpu core id for the internal Reactor worker thread. */
	reactorOpts.cpuBindWorkerThread = pConsumerThread->cpuReactorWorkerId;

	/* Create an RsslReactor which will manage our channels. */
	if (!(pConsumerThread->pReactor = rsslCreateReactor(&reactorOpts, &rsslErrorInfo)))
	{
		printf("Error: %s", rsslErrorInfo.rsslError.text);
		return RSSL_RET_FAILURE;
	}

	jsonConverterOptions.pDictionary = pConsumerThread->pDictionary;
	jsonConverterOptions.defaultServiceId = 1;
	jsonConverterOptions.userSpecPtr = (void*)pConsumerThread;
	jsonConverterOptions.pServiceNameToIdCallback = serviceNameToIdReactorCallback;
	if (rsslReactorInitJsonConverter(pConsumerThread->pReactor, &jsonConverterOptions, &rsslErrorInfo) != RSSL_RET_SUCCESS)
	{
		printf("Error initializing RWF/JSON converter: %s\n", rsslErrorInfo.rsslError.text);
		return RSSL_RET_FAILURE;
	}


	/* Set the reactor's event file descriptor on our descriptor set. This, along with the file descriptors 
	 * of RsslReactorChannels, will notify us when we should call rsslReactorDispatch(). */
	FD_SET(pConsumerThread->pReactor->eventFd, &(pConsumerThread->readfds));
	
	if (consPerfConfig.warmStandbyMode == RSSL_RWSB_MODE_NONE)
	{
		/* Connect to RSSL server */
		if (strlen(consPerfConfig.interfaceName)) cInfo.rsslConnectOptions.connectionInfo.unified.interfaceName = consPerfConfig.interfaceName;

		cInfo.rsslConnectOptions.guaranteedOutputBuffers = consPerfConfig.guaranteedOutputBuffers;
		cInfo.rsslConnectOptions.numInputBuffers = consPerfConfig.numInputBuffers;
		cInfo.rsslConnectOptions.sysSendBufSize = consPerfConfig.sendBufSize;
		cInfo.rsslConnectOptions.sysRecvBufSize = consPerfConfig.recvBufSize;
		cInfo.rsslConnectOptions.majorVersion = RSSL_RWF_MAJOR_VERSION;
		cInfo.rsslConnectOptions.minorVersion = RSSL_RWF_MINOR_VERSION;
		cInfo.rsslConnectOptions.protocolType = RSSL_RWF_PROTOCOL_TYPE;
		cInfo.rsslConnectOptions.connectionType = consPerfConfig.connectionType;

		if (consPerfConfig.connectionType == RSSL_CONN_TYPE_SOCKET || consPerfConfig.connectionType == RSSL_CONN_TYPE_ENCRYPTED)
		{
			cInfo.rsslConnectOptions.tcpOpts.tcp_nodelay = consPerfConfig.tcpNoDelay;
		}

		if (consPerfConfig.connectionType == RSSL_CONN_TYPE_WEBSOCKET ||
			(consPerfConfig.connectionType == RSSL_CONN_TYPE_ENCRYPTED &&
				consPerfConfig.encryptedConnectionType == RSSL_CONN_TYPE_WEBSOCKET))
			cInfo.rsslConnectOptions.wsOpts.protocols = consPerfConfig.protocolList;

		if (consPerfConfig.connectionType == RSSL_CONN_TYPE_ENCRYPTED)
		{
			cInfo.rsslConnectOptions.encryptionOpts.encryptedProtocol = consPerfConfig.encryptedConnectionType;
			if (consPerfConfig.tlsProtocolFlags != 0)
				cInfo.rsslConnectOptions.encryptionOpts.encryptionProtocolFlags = consPerfConfig.tlsProtocolFlags;
			cInfo.rsslConnectOptions.encryptionOpts.openSSLCAStore = consPerfConfig.caStore;
		}

		cInfo.rsslConnectOptions.connectionInfo.unified.address = consPerfConfig.hostName;
		cInfo.rsslConnectOptions.connectionInfo.unified.serviceName = consPerfConfig.portNo;

		cInfo.rsslConnectOptions.userSpecPtr = pConsumerThread;
		cOpts.reactorConnectionList = &cInfo;
		cOpts.connectionCount = 1;
	}
	else
	{
		/* Enable warm standby feature to connect to starting and standby servers */
		if (strlen(consPerfConfig.interfaceName))
		{
			reactorWarmStandbyGroup.startingActiveServer.reactorConnectInfo.rsslConnectOptions.connectionInfo.unified.interfaceName = consPerfConfig.interfaceName;
			standbyServerInfo.reactorConnectInfo.rsslConnectOptions.connectionInfo.unified.interfaceName = consPerfConfig.interfaceName;
		}

		reactorWarmStandbyGroup.startingActiveServer.reactorConnectInfo.rsslConnectOptions.guaranteedOutputBuffers = consPerfConfig.guaranteedOutputBuffers;
		reactorWarmStandbyGroup.startingActiveServer.reactorConnectInfo.rsslConnectOptions.numInputBuffers = consPerfConfig.numInputBuffers;
		reactorWarmStandbyGroup.startingActiveServer.reactorConnectInfo.rsslConnectOptions.sysSendBufSize = consPerfConfig.sendBufSize;
		reactorWarmStandbyGroup.startingActiveServer.reactorConnectInfo.rsslConnectOptions.sysRecvBufSize = consPerfConfig.recvBufSize;
		reactorWarmStandbyGroup.startingActiveServer.reactorConnectInfo.rsslConnectOptions.majorVersion = RSSL_RWF_MAJOR_VERSION;
		reactorWarmStandbyGroup.startingActiveServer.reactorConnectInfo.rsslConnectOptions.minorVersion = RSSL_RWF_MINOR_VERSION;
		reactorWarmStandbyGroup.startingActiveServer.reactorConnectInfo.rsslConnectOptions.protocolType = RSSL_RWF_PROTOCOL_TYPE;
		reactorWarmStandbyGroup.startingActiveServer.reactorConnectInfo.rsslConnectOptions.connectionType = consPerfConfig.connectionType;
		reactorWarmStandbyGroup.startingActiveServer.reactorConnectInfo.rsslConnectOptions.connectionInfo.unified.address = consPerfConfig.startingHostName;
		reactorWarmStandbyGroup.startingActiveServer.reactorConnectInfo.rsslConnectOptions.connectionInfo.unified.serviceName = consPerfConfig.startingPort;
		reactorWarmStandbyGroup.startingActiveServer.reactorConnectInfo.rsslConnectOptions.userSpecPtr = pConsumerThread;

		standbyServerInfo.reactorConnectInfo.rsslConnectOptions.guaranteedOutputBuffers = consPerfConfig.guaranteedOutputBuffers;
		standbyServerInfo.reactorConnectInfo.rsslConnectOptions.numInputBuffers = consPerfConfig.numInputBuffers;
		standbyServerInfo.reactorConnectInfo.rsslConnectOptions.sysSendBufSize = consPerfConfig.sendBufSize;
		standbyServerInfo.reactorConnectInfo.rsslConnectOptions.sysRecvBufSize = consPerfConfig.recvBufSize;
		standbyServerInfo.reactorConnectInfo.rsslConnectOptions.majorVersion = RSSL_RWF_MAJOR_VERSION;
		standbyServerInfo.reactorConnectInfo.rsslConnectOptions.minorVersion = RSSL_RWF_MINOR_VERSION;
		standbyServerInfo.reactorConnectInfo.rsslConnectOptions.protocolType = RSSL_RWF_PROTOCOL_TYPE;
		standbyServerInfo.reactorConnectInfo.rsslConnectOptions.connectionType = consPerfConfig.connectionType;
		standbyServerInfo.reactorConnectInfo.rsslConnectOptions.connectionInfo.unified.address = consPerfConfig.standbyHostName;
		standbyServerInfo.reactorConnectInfo.rsslConnectOptions.connectionInfo.unified.serviceName = consPerfConfig.standbyPort;
		standbyServerInfo.reactorConnectInfo.rsslConnectOptions.userSpecPtr = pConsumerThread;

		if (consPerfConfig.connectionType == RSSL_CONN_TYPE_SOCKET || consPerfConfig.connectionType == RSSL_CONN_TYPE_ENCRYPTED)
		{
			reactorWarmStandbyGroup.startingActiveServer.reactorConnectInfo.rsslConnectOptions.tcpOpts.tcp_nodelay = consPerfConfig.tcpNoDelay;
			standbyServerInfo.reactorConnectInfo.rsslConnectOptions.tcpOpts.tcp_nodelay = consPerfConfig.tcpNoDelay;
		}

		if (consPerfConfig.connectionType == RSSL_CONN_TYPE_WEBSOCKET ||
			(consPerfConfig.connectionType == RSSL_CONN_TYPE_ENCRYPTED &&
				consPerfConfig.encryptedConnectionType == RSSL_CONN_TYPE_WEBSOCKET))
		{
			reactorWarmStandbyGroup.startingActiveServer.reactorConnectInfo.rsslConnectOptions.wsOpts.protocols = consPerfConfig.protocolList;
			standbyServerInfo.reactorConnectInfo.rsslConnectOptions.wsOpts.protocols = consPerfConfig.protocolList;
		}

		if (consPerfConfig.connectionType == RSSL_CONN_TYPE_ENCRYPTED)
		{
			reactorWarmStandbyGroup.startingActiveServer.reactorConnectInfo.rsslConnectOptions.encryptionOpts.encryptedProtocol = consPerfConfig.encryptedConnectionType;
			standbyServerInfo.reactorConnectInfo.rsslConnectOptions.encryptionOpts.encryptedProtocol = consPerfConfig.encryptedConnectionType;

			if (consPerfConfig.tlsProtocolFlags != 0)
			{
				reactorWarmStandbyGroup.startingActiveServer.reactorConnectInfo.rsslConnectOptions.encryptionOpts.encryptionProtocolFlags = consPerfConfig.tlsProtocolFlags;
				standbyServerInfo.reactorConnectInfo.rsslConnectOptions.encryptionOpts.encryptionProtocolFlags = consPerfConfig.tlsProtocolFlags;
			}

			reactorWarmStandbyGroup.startingActiveServer.reactorConnectInfo.rsslConnectOptions.encryptionOpts.openSSLCAStore = consPerfConfig.caStore;
			standbyServerInfo.reactorConnectInfo.rsslConnectOptions.encryptionOpts.openSSLCAStore = consPerfConfig.caStore;
		}

		reactorWarmStandbyGroup.standbyServerCount = 1;
		reactorWarmStandbyGroup.standbyServerList = &standbyServerInfo;
		reactorWarmStandbyGroup.warmStandbyMode = consPerfConfig.warmStandbyMode;
		cOpts.warmStandbyGroupCount = 1;
		cOpts.reactorWarmStandbyGroupList = &reactorWarmStandbyGroup;
	}

	// set consumer role information
	rsslClearOMMConsumerRole(&pConsumerThread->consumerRole);
	pConsumerThread->consumerRole.base.channelEventCallback = channelEventCallback;
	pConsumerThread->consumerRole.base.defaultMsgCallback = defaultMsgCallback;
	pConsumerThread->consumerRole.loginMsgCallback = loginMsgCallback;
	pConsumerThread->consumerRole.directoryMsgCallback = directoryMsgCallback;
	pConsumerThread->consumerRole.dictionaryMsgCallback = dictionaryMsgCallback;
	if (!pConsumerThread->dictionaryStateFlags && consPerfConfig.useWatchlist == RSSL_FALSE)
	{
		pConsumerThread->consumerRole.dictionaryDownloadMode = RSSL_RC_DICTIONARY_DOWNLOAD_FIRST_AVAILABLE;
	}
	/* Initialize the default login request(Use 1 as the Login Stream ID). */
	if (rsslInitDefaultRDMLoginRequest(&pConsumerThread->loginRequest, 1) != RSSL_RET_SUCCESS)
	{
		printf("rsslInitDefaultRDMLoginRequest() failed\n");
		return RSSL_RET_FAILURE;
	}
	/* If a username was specified, change username on login request. */
	if (strlen(consPerfConfig.username) > 0)
	{
		pConsumerThread->loginRequest.userName.data = consPerfConfig.username;
		pConsumerThread->loginRequest.userName.length = (rtrUInt32)strlen(consPerfConfig.username);
	}
	pConsumerThread->consumerRole.pLoginRequest = &pConsumerThread->loginRequest;
	/* Initialize the default directory request(Use 2 as the Directory Stream Id) */
	if (rsslInitDefaultRDMDirectoryRequest(&pConsumerThread->dirRequest, 2) != RSSL_RET_SUCCESS)
	{
		printf("rsslInitDefaultRDMDirectoryRequest() failed\n");
		return RSSL_RET_FAILURE;
	}
	pConsumerThread->consumerRole.pDirectoryRequest = &pConsumerThread->dirRequest;

	// if watchlist is configured by user, set enableWatchlist on consumer role to TRUE
	if (consPerfConfig.useWatchlist == RSSL_TRUE)
	{
		pConsumerThread->consumerRole.watchlistOptions.enableWatchlist = RSSL_TRUE;
	}
		
	// connect via Reactor
	cOpts.reconnectAttemptLimit = -1;
	cOpts.reconnectMaxDelay = 5000;
	cOpts.reconnectMinDelay = 1000;

    if ((ret = rsslReactorConnect(pConsumerThread->pReactor, &cOpts, (RsslReactorChannelRole *)&pConsumerThread->consumerRole, &rsslErrorInfo)) < RSSL_RET_SUCCESS)
    {
		printf("rsslReactorConnect failed with return code: %d error = %s", ret,  rsslErrorInfo.rsslError.text);
		return RSSL_RET_FAILURE;
    }		

	return RSSL_RET_SUCCESS;
}

RsslRet processItemMsg(RsslReactorChannel *pReactorChannel, RsslMsg *pMsg);

static RsslRet initTunnel(ConsumerThread* pConsumerThread)
{
	if (consPerfConfig.tunnelMessagingEnabled == RSSL_TRUE)
	{
		pConsumerThread->tunnelMessagingEnabled = RSSL_TRUE;
		strncpy(pConsumerThread->tunnelStreamServiceName, consPerfConfig.tunnelStreamServiceName,
			sizeof(pConsumerThread->tunnelStreamServiceName));

		perfTunnelMsgHandlerInit(&pConsumerThread->perfTunnelMsgHandler, applicationName.data,
			consPerfConfig.tunnelDomainType, consPerfConfig.tunnelUseAuthentication, RSSL_FALSE, consPerfConfig.guaranteedOutputTunnelBuffers);
		perfTunnelMsgHandlerAddCallbackProcessItemMsg(&pConsumerThread->perfTunnelMsgHandler, processItemMsg);
	}
	return RSSL_RET_SUCCESS;
}

static RsslRet createTunnel(ConsumerThread* pConsumerThread)
{
	if (pConsumerThread->tunnelMessagingEnabled)
	{
		assert(pConsumerThread->pReactorChannel);

		startPerfTunnelMsgHandler(pConsumerThread->pReactor, pConsumerThread->pReactorChannel, &pConsumerThread->perfTunnelMsgHandler);
	}
	return RSSL_RET_SUCCESS;
}

static RsslRet initialize(ConsumerThread* pConsumerThread, LatencyRandomArray* postLatencyRandomArray, LatencyRandomArray* genMsgLatencyRandomArray)
{
	RsslChannelInfo channelInfo;

	RsslInt32 i;
	RsslUInt32 count;
	
	char errTxt[256];
	RsslBuffer errorText = {255, (char*)errTxt};

	RsslRet ret = 0;
	RsslError closeError;
	RsslErrorInfo rsslErrorInfo;

	RsslBool haveMarketPricePostItems = RSSL_FALSE, haveMarketByOrderPostItems = RSSL_FALSE;
	RsslBool haveMarketPriceGenMsgItems = RSSL_FALSE, haveMarketByOrderGenMsgItems = RSSL_FALSE;

	RsslInt32 postItemCount = 0;
	RsslInt32 genMsgItemCount = 0;

	XmlItemInfoList *pXmlItemInfoList;
	RsslInt32 xmlItemListIndex;

#ifdef ENABLE_XML_TRACE
	RsslError error;
	RsslTraceOptions traceOptions;
#endif
	
	if (consPerfConfig.latencyPostsPerSec)
	{
		LatencyRandomArrayOptions randomArrayOpts;
		clearLatencyRandomArrayOptions(&randomArrayOpts);
		randomArrayOpts.totalMsgsPerSec = consPerfConfig.postsPerSec;
		randomArrayOpts.latencyMsgsPerSec = consPerfConfig.latencyPostsPerSec;
		randomArrayOpts.ticksPerSec = consPerfConfig.ticksPerSec;
		randomArrayOpts.arrayCount = LATENCY_RANDOM_ARRAY_SET_COUNT;

		createLatencyRandomArray(postLatencyRandomArray, &randomArrayOpts);
	}

	if (consPerfConfig.genMsgsPerSec)
	{
		LatencyRandomArrayOptions randomArrayOpts;
		clearLatencyRandomArrayOptions(&randomArrayOpts);
		randomArrayOpts.totalMsgsPerSec = consPerfConfig.genMsgsPerSec;
		randomArrayOpts.latencyMsgsPerSec = consPerfConfig.latencyGenMsgsPerSec;
		randomArrayOpts.ticksPerSec = consPerfConfig.ticksPerSec;
		randomArrayOpts.arrayCount = LATENCY_RANDOM_ARRAY_SET_COUNT;

		createLatencyRandomArray(genMsgLatencyRandomArray, &randomArrayOpts);
	}
	
	/* Get names of other items from file. */
	if (!(pXmlItemInfoList = createXmlItemList(consPerfConfig.itemFilename, consPerfConfig.itemRequestCount)))
	{
		printf("Failed to load item list from file '%s'.\n", consPerfConfig.itemFilename);
		exit(-1);
	}

	pConsumerThread->itemRequestList = 
		(ItemRequest*)malloc(sizeof(ItemRequest)*pConsumerThread->itemListCount);

	/* Shift item info list pointer so we can use streamID's for direct lookup. */
	pConsumerThread->itemRequestList -= ITEM_STREAM_ID_START;


	/* Should be able to store any item name that the XML parser can store. */
	assert(sizeof(pConsumerThread->itemRequestList[ITEM_STREAM_ID_START].itemNameChar) >= sizeof(pXmlItemInfoList->itemInfoList[0].name));

	if (pXmlItemInfoList->itemInfoCount < consPerfConfig.itemRequestCount)
	{
		printf("Error: Item file contained %d items, but consumer wants %d items.\n", pXmlItemInfoList->itemInfoCount, 
				consPerfConfig.itemRequestCount);
		exit(-1);
	}

	if (consPerfConfig.postsPerSec && pXmlItemInfoList->postItemCount == 0)
	{
		printf("Error: Configured for posting but no posting items found in item file.\n");
		exit(-1);
	}
	if (consPerfConfig.genMsgsPerSec && pXmlItemInfoList->genMsgItemCount == 0)
	{
		printf("Error: Configured for sending generic messages but no generic message items found in item file.\n");
		exit(-1);
	}

	/* Copy item information from the XML list. */
	xmlItemListIndex = 0;
	for(i = 0; i < pConsumerThread->itemListCount; ++i)
	{
		RsslInt32 streamId = i + ITEM_STREAM_ID_START;

		/* If there are multiple consumer connections, each consumer must build its
		 * list of items from the items that are common to all consumers (if any)
		 * and those unique to that consumer. */

		/* Once we have filled our list with the common items,
		 * start using the range of items unique to this consumer thread. */
		if (xmlItemListIndex == consPerfConfig.commonItemCount
				&& xmlItemListIndex < pConsumerThread->itemListUniqueIndex)
			xmlItemListIndex = pConsumerThread->itemListUniqueIndex;

		clearItemRequest(&pConsumerThread->itemRequestList[streamId]);
		pConsumerThread->itemRequestList[streamId].itemInfo.StreamId = streamId;
		pConsumerThread->itemRequestList[streamId].itemInfo.attributes.domainType = pXmlItemInfoList->itemInfoList[xmlItemListIndex].domainType;

		strncpy(pConsumerThread->itemRequestList[streamId].itemNameChar, pXmlItemInfoList->itemInfoList[xmlItemListIndex].name, pXmlItemInfoList->itemInfoList[xmlItemListIndex].nameLength);
		pConsumerThread->itemRequestList[streamId].msgKey.flags = RSSL_MKF_HAS_NAME;
		pConsumerThread->itemRequestList[streamId].msgKey.name.length = pXmlItemInfoList->itemInfoList[xmlItemListIndex].nameLength;
		pConsumerThread->itemRequestList[streamId].msgKey.name.data = pConsumerThread->itemRequestList[streamId].itemNameChar;
		pConsumerThread->itemRequestList[streamId].itemInfo.attributes.pMsgKey = &pConsumerThread->itemRequestList[streamId].msgKey;

		rsslQueueAddLinkToBack(&pConsumerThread->requestQueue, &pConsumerThread->itemRequestList[streamId].link);

		if (!pXmlItemInfoList->itemInfoList[xmlItemListIndex].isSnapshot)
		{
			pConsumerThread->itemRequestList[streamId].itemInfo.itemFlags |= ITEM_IS_STREAMING_REQ;
		}

		if (pXmlItemInfoList->itemInfoList[xmlItemListIndex].isPost && consPerfConfig.postsPerSec)
		{
			void *pItemData;

			pConsumerThread->itemRequestList[streamId].itemInfo.itemFlags |= ITEM_IS_POST;

			/* Get item info data from appropriate pool */
			switch(pConsumerThread->itemRequestList[streamId].itemInfo.attributes.domainType)
			{
				case RSSL_DMT_MARKET_PRICE:
					pItemData = (void*)createMarketPriceItem();
					haveMarketPricePostItems = RSSL_TRUE;
					break;
				case RSSL_DMT_MARKET_BY_ORDER:
					pItemData = (void*)createMarketByOrderItem();
					haveMarketByOrderPostItems = RSSL_TRUE;
					break;
				default:
					pItemData = 0;
					break;
			}

			pConsumerThread->itemRequestList[streamId].itemInfo.itemData = pItemData;
		}

		if (consPerfConfig.postsPerSec > 0)
		{
			if (haveMarketPricePostItems && (!xmlMsgDataHasMarketPrice || xmlMarketPriceMsgs.postMsgCount == 0))
			{
				rsslSetErrorInfo(&pConsumerThread->threadErrorInfo, RSSL_EIC_FAILURE, RSSL_RET_FAILURE, __FILE__, __LINE__,
						(char*)"Error: No MarketPrice posting data in file: %s", consPerfConfig.msgFilename);
				shutdownThreads = RSSL_TRUE;
				return RSSL_RET_FAILURE;
			}
			else if (haveMarketByOrderPostItems && (!xmlMsgDataHasMarketByOrder || xmlMarketByOrderMsgs.postMsgCount == 0))
			{
				rsslSetErrorInfo(&pConsumerThread->threadErrorInfo, RSSL_EIC_FAILURE, RSSL_RET_FAILURE, __FILE__, __LINE__,
						(char*)"Error: No MarketByOrder posting data in file: %s", consPerfConfig.msgFilename);
				shutdownThreads = RSSL_TRUE;
				return RSSL_RET_FAILURE;
			}
		}

		if (pXmlItemInfoList->itemInfoList[xmlItemListIndex].isGenMsg && consPerfConfig.genMsgsPerSec)
		{
			void *pItemData;

			pConsumerThread->itemRequestList[streamId].itemInfo.itemFlags |= ITEM_IS_GEN_MSG;

			/* Get item info data from appropriate pool */
			switch(pConsumerThread->itemRequestList[streamId].itemInfo.attributes.domainType)
			{
				case RSSL_DMT_MARKET_PRICE:
					pItemData = (void*)createMarketPriceItem();
					haveMarketPriceGenMsgItems = RSSL_TRUE;
					break;
				case RSSL_DMT_MARKET_BY_ORDER:
					pItemData = (void*)createMarketByOrderItem();
					haveMarketByOrderGenMsgItems = RSSL_TRUE;
					break;
				default:
					pItemData = 0;
					break;
			}

			pConsumerThread->itemRequestList[streamId].itemInfo.itemData = pItemData;
		}

		if (consPerfConfig.genMsgsPerSec > 0)
		{
			if (haveMarketPriceGenMsgItems && (!xmlMsgDataHasMarketPrice || xmlMarketPriceMsgs.genMsgCount == 0))
			{
				rsslSetErrorInfo(&pConsumerThread->threadErrorInfo, RSSL_EIC_FAILURE, RSSL_RET_FAILURE, __FILE__, __LINE__,
						(char*)"Error: No MarketPrice generic message data in file: %s", consPerfConfig.msgFilename);
				shutdownThreads = RSSL_TRUE;
				return RSSL_RET_FAILURE;
			}
			else if (haveMarketByOrderGenMsgItems && (!xmlMsgDataHasMarketByOrder || xmlMarketByOrderMsgs.genMsgCount == 0))
			{
				rsslSetErrorInfo(&pConsumerThread->threadErrorInfo, RSSL_EIC_FAILURE, RSSL_RET_FAILURE, __FILE__, __LINE__,
						(char*)"Error: No MarketByOrder generic message data in file: %s", consPerfConfig.msgFilename);
				shutdownThreads = RSSL_TRUE;
				return RSSL_RET_FAILURE;
			}
		}

		++xmlItemListIndex;
	}

	/* Did the index stop where we expected it to? */
	assert(
			/* No unique items */
			pConsumerThread->itemListCount == consPerfConfig.commonItemCount && 
			xmlItemListIndex == consPerfConfig.commonItemCount
			||
			xmlItemListIndex == pConsumerThread->itemListUniqueIndex 
			+ pConsumerThread->itemListCount - consPerfConfig.commonItemCount);

	destroyXmlItemList(pXmlItemInfoList);

	/* Load dictionary from file if possible. */
	if (rsslLoadFieldDictionary("RDMFieldDictionary", pConsumerThread->pDictionary, &errorText) < 0)
	{
		printf("Unable to load field dictionary: %s.\n"
				"Will request dictionaries from provider.\n\n" , errorText.data);
	}
	else if (rsslLoadEnumTypeDictionary("enumtype.def", pConsumerThread->pDictionary, &errorText) < 0)
	{
		printf("\nUnable to load enum type dictionary: %s\n", errorText.data);
		printf("Unable to load enum type dictionary: %s.\n\n"
				"Will request dictionaries from provider.\n" , errorText.data);
	}
	else
	{
		/* Dictionary successfully loaded. */
		pConsumerThread->dictionaryStateFlags =
			(DictionaryStateFlags)(DICTIONARY_STATE_HAVE_FIELD_DICT | DICTIONARY_STATE_HAVE_ENUM_DICT);
	}
	
	if (consPerfConfig.useReactor == RSSL_FALSE && consPerfConfig.useWatchlist == RSSL_FALSE)
	{
		pConsumerThread->rjcSess.options.pDictionary = pConsumerThread->pDictionary;
		pConsumerThread->rjcSess.options.defaultServiceId = 1;
		pConsumerThread->rjcSess.options.userSpecPtr = (void*)pConsumerThread;
		pConsumerThread->rjcSess.options.pServiceNameToIdCallback = serviceNameToIdCallback;

		if (rjcSessionInitialize(&(pConsumerThread->rjcSess), &rsslErrorInfo) != RSSL_RET_SUCCESS)
		{
			printf("RWF/JSON Converter failed: %s\n", rsslErrorInfo.rsslError.text);
			return RSSL_RET_FAILURE;
		}

		if ((ret = connectChannel(pConsumerThread)) < RSSL_RET_SUCCESS)
		{
			return ret;
		}
		if ((ret = rsslGetChannelInfo(pConsumerThread->pChannel, &channelInfo, &pConsumerThread->threadRsslError)) != RSSL_RET_SUCCESS)
		{
			rsslSetErrorInfoLocation(&pConsumerThread->threadErrorInfo, __FILE__, __LINE__);
			rsslCloseChannel(pConsumerThread->pChannel, &closeError);
			shutdownThreads = RSSL_TRUE;
			return ret;
		} 

		printf( "Channel "SOCKET_PRINT_TYPE" active. Channel Info:\n"
				"  maxFragmentSize: %u\n"
				"  maxOutputBuffers: %u\n"
				"  guaranteedOutputBuffers: %u\n"
				"  numInputBuffers: %u\n"
				"  pingTimeout: %u\n"
				"  clientToServerPings: %s\n"
				"  serverToClientPings: %s\n"
				"  sysSendBufSize: %u\n"
				"  sysSendBufSize: %u\n"			
				"  compressionType: %s\n"
				"  compressionThreshold: %u\n"			
				"  ComponentInfo: ", 
				pConsumerThread->pChannel->socketId,
				channelInfo.maxFragmentSize,
				channelInfo.maxOutputBuffers, channelInfo.guaranteedOutputBuffers,
				channelInfo.numInputBuffers,
				channelInfo.pingTimeout,
				channelInfo.clientToServerPings == RSSL_TRUE ? "true" : "false",
				channelInfo.serverToClientPings == RSSL_TRUE ? "true" : "false",
				channelInfo.sysSendBufSize, channelInfo.sysRecvBufSize,			
				channelInfo.compressionType == RSSL_COMP_ZLIB ? "zlib" : "none",
				channelInfo.compressionThreshold			
				);

		if (channelInfo.componentInfoCount == 0)
			printf("(No component info)");
		else
			for(count = 0; count < channelInfo.componentInfoCount; ++count)
			{
				printf("%.*s", 
						channelInfo.componentInfo[count]->componentVersion.length,
						channelInfo.componentInfo[count]->componentVersion.data);
				if (count < channelInfo.componentInfoCount - 1)
					printf(", ");
			}
		printf ("\n\n");

		consumerThreadInitPings(pConsumerThread);

		if (pConsumerThread->pChannel->protocolType == RSSL_JSON_PROTOCOL_TYPE && consPerfConfig.convertJSON == RSSL_FALSE)
		{
			ret = sendLoginRequestJson(pConsumerThread, LOGIN_STREAM_ID);
		}
		else
		{
			ret = sendLoginRequest(pConsumerThread, LOGIN_STREAM_ID);
		}
		if ( ret != RSSL_RET_SUCCESS )
		{
			rsslCloseChannel(pConsumerThread->pChannel, &closeError);
			shutdownThreads = RSSL_TRUE;
			return RSSL_RET_FAILURE;
		}
	}
	else
	{
		if ((ret = connectReactor(pConsumerThread)) < RSSL_RET_SUCCESS)
		{
			return ret;
		}

		initTunnel(pConsumerThread);
	}

#ifndef NO_ETA_CPU_BIND
	// Cpu core bind for the consumer thread.
	// The application should invoke rsslBindThread() after rsslInitialize() has invoked.
	// rsslInitialize analyzed Cpu Topology.
	if (pConsumerThread->cpuId.length > 0 && pConsumerThread->cpuId.data != NULL)
	{
		if (rsslBindThread(pConsumerThread->cpuId.data, &rsslErrorInfo) != RSSL_RET_SUCCESS)
		{
			printf("Error: Failed to bind thread to core %s: %s\n", pConsumerThread->cpuId.data, rsslErrorInfo.rsslError.text);
			exit(-1);
		}
	}
#endif

#ifdef ENABLE_XML_TRACE
	rsslClearTraceOptions(&traceOptions);
	traceOptions.traceMsgFileName = "ConsPerf";
	traceOptions.traceMsgMaxFileSize = 1000000000;
	traceOptions.traceFlags |= RSSL_TRACE_TO_FILE_ENABLE | RSSL_TRACE_WRITE | RSSL_TRACE_READ;
	rsslIoctl(pConsumerThread->pChannel, (RsslIoctlCodes)RSSL_TRACE, (void *)&traceOptions, &error);
#endif

	return RSSL_RET_SUCCESS;
}

static RsslRet processLoginResp(ConsumerThread* pConsumerThread, RsslRDMLoginMsg *pLoginMsg)
{
	RsslError closeError;

	switch(pLoginMsg->rdmMsgBase.rdmMsgType)
	{
		case RDM_LG_MT_REFRESH:

			printf(	"Received login refresh.\n");
			if (pLoginMsg->refresh.flags &
					RDM_LG_RFF_HAS_APPLICATION_NAME)
				printf( "  ApplicationName: %.*s\n",
					pLoginMsg->refresh.applicationName.length,
					pLoginMsg->refresh.applicationName.data);
			printf("\n");

			if(pLoginMsg->refresh.state.streamState != RSSL_STREAM_OPEN)
			{
				printf("Error: StreamState: %s, Login failed: %.*s\n", rsslStreamStateToString(pLoginMsg->refresh.state.streamState), 
					pLoginMsg->refresh.state.text.length, pLoginMsg->refresh.state.text.data);
				rsslCloseChannel(pConsumerThread->pChannel, &closeError);
				shutdownThreads = RSSL_TRUE;
				return RSSL_RET_FAILURE;
			}
			if (consPerfConfig.postsPerSec &&
					(!(pLoginMsg->refresh.flags & RDM_LG_RFF_HAS_SUPPORT_POST)
						|| !pLoginMsg->refresh.supportOMMPost))
			{
				rsslSetErrorInfo(&pConsumerThread->threadErrorInfo, RSSL_EIC_FAILURE, RSSL_RET_FAILURE, __FILE__, __LINE__,
						(char*)"Provider for this connection does not support posting.");
				rsslCloseChannel(pConsumerThread->pChannel, &closeError);
				shutdownThreads = RSSL_TRUE;
				return RSSL_RET_FAILURE;
			}
			break;
		case RDM_LG_MT_STATUS:
			break;
		default:
			rsslSetErrorInfo(&pConsumerThread->threadErrorInfo, RSSL_EIC_FAILURE, RSSL_RET_FAILURE, __FILE__, __LINE__,
					(char*)"Received unhandled RDMLoginMsgType %d", pLoginMsg->rdmMsgBase.rdmMsgType);
			rsslCloseChannel(pConsumerThread->pChannel, &closeError);
			shutdownThreads = RSSL_TRUE;
			return RSSL_RET_FAILURE;
	}

	return RSSL_RET_SUCCESS;
}

static RsslRet processLoginRespJson(ConsumerThread* pConsumerThread, cJSON* json)
{
	RsslError closeError;
	const char* applicationName;

	RsslRDMLoginMsgType rsslRDMLoginMsgType = jsonGetRDMLoginMsgType(json);

	switch (rsslRDMLoginMsgType)
	{
	case RDM_LG_MT_REFRESH:

		printf("Received login refresh.\n");
		if ((applicationName = jsonGetLoginRefreshApplicationName(json)) != NULL)
			printf("  ApplicationName: %s\n", applicationName);
		printf("\n");

		RsslStreamStates rsslStreamState = jsonGetStreamState(json);

		if (rsslStreamState != RSSL_STREAM_OPEN)
		{
			const char* text = jsonGetRefreshStateText(json);
			printf("Error: StreamState: %s, Login failed: %s\n", rsslStreamStateToString(rsslStreamState), text);
			rsslCloseChannel(pConsumerThread->pChannel, &closeError);
			shutdownThreads = RSSL_TRUE;
			return RSSL_RET_FAILURE;
		}

		if (consPerfConfig.postsPerSec && !jsonGetSupportOMMPost(json))
		{
			rsslSetErrorInfo(&pConsumerThread->threadErrorInfo, RSSL_EIC_FAILURE, RSSL_RET_FAILURE, __FILE__, __LINE__,
				(char*)"Provider for this connection does not support posting.");
			rsslCloseChannel(pConsumerThread->pChannel, &closeError);
			shutdownThreads = RSSL_TRUE;
			return RSSL_RET_FAILURE;
		}
		break;
	case RDM_LG_MT_STATUS:
		break;
	default:
		rsslSetErrorInfo(&pConsumerThread->threadErrorInfo, RSSL_EIC_FAILURE, RSSL_RET_FAILURE, __FILE__, __LINE__,
			(char*)"Received unhandled RDMLoginMsgType %d", rsslRDMLoginMsgType);
		rsslCloseChannel(pConsumerThread->pChannel, &closeError);
		shutdownThreads = RSSL_TRUE;
		return RSSL_RET_FAILURE;
	}

	return RSSL_RET_SUCCESS;
}

static RsslRet processSourceDirectoryResp(ConsumerThread* pConsumerThread, RsslRDMDirectoryMsg *pDirectoryMsg)
{
	RsslRet ret;
	RsslError closeError;
	RsslBuffer serviceName;
	RsslRDMService *pMsgServiceList;
	RsslUInt32 msgServiceCount, iMsgServiceList;
	RsslBool foundServiceName = RSSL_FALSE;
	RsslBuffer tunnelServiceNameBuffer;
	RsslBool runTunnelServiceUpdate = RSSL_FALSE;

	serviceName.data = consPerfConfig.serviceName;
	serviceName.length = (RsslUInt32)strlen(serviceName.data);

	/* Copy the directory message.  If our desired service is present inside, we will want to keep it. */
	if ((ret = rsslCopyRDMDirectoryMsg(&pConsumerThread->directoryMsgCopy, pDirectoryMsg, &pConsumerThread->directoryMsgCopyMemory)) != RSSL_RET_SUCCESS)
	{
		rsslSetErrorInfo(&pConsumerThread->threadErrorInfo, RSSL_EIC_FAILURE, ret, __FILE__, __LINE__,
				(char*)"rsslCopyRDMDirectoryMsg failed: %d(%s)", ret, rsslRetCodeToString(ret));
		rsslCloseChannel(pConsumerThread->pChannel, &closeError);
		shutdownThreads = RSSL_TRUE;
		return RSSL_RET_FAILURE;
	}

	switch(pConsumerThread->directoryMsgCopy.rdmMsgBase.rdmMsgType)
	{
		case RDM_DR_MT_REFRESH:
			pMsgServiceList = pConsumerThread->directoryMsgCopy.refresh.serviceList;
			msgServiceCount = pConsumerThread->directoryMsgCopy.refresh.serviceCount;
			if (pConsumerThread->tunnelMessagingEnabled)
			{
				tunnelServiceNameBuffer.data = pConsumerThread->tunnelStreamServiceName;
				tunnelServiceNameBuffer.length = (RsslUInt32)strlen(pConsumerThread->tunnelStreamServiceName);
				runTunnelServiceUpdate = RSSL_TRUE;
			}
			break;
		case RDM_DR_MT_UPDATE:
			pMsgServiceList = pConsumerThread->directoryMsgCopy.update.serviceList;
			msgServiceCount = pConsumerThread->directoryMsgCopy.update.serviceCount;
			if (pConsumerThread->tunnelMessagingEnabled)
			{
				tunnelServiceNameBuffer.data = serviceName.data;
				tunnelServiceNameBuffer.length = (RsslUInt32)strlen(serviceName.data);
				runTunnelServiceUpdate = RSSL_TRUE;
			}
			break;
		case RDM_DR_MT_STATUS:
			break;
		default:
			rsslSetErrorInfo(&pConsumerThread->threadErrorInfo, RSSL_EIC_FAILURE, RSSL_RET_FAILURE, __FILE__, __LINE__,
					(char*)"Error: Received unhandled directory message type %d.", pDirectoryMsg->rdmMsgBase.rdmMsgType);
			rsslCloseChannel(pConsumerThread->pChannel, &closeError);
			shutdownThreads = RSSL_TRUE;
			return RSSL_RET_FAILURE;

	}

	/* Search service list for our desired service. */
	for(iMsgServiceList = 0; iMsgServiceList < msgServiceCount; ++iMsgServiceList)
	{
		RsslRDMService *pService = &pMsgServiceList[iMsgServiceList];

		if ( /* Check for matching service name. */
				pService->flags & RDM_SVCF_HAS_INFO
				&& rsslBufferIsEqual(&serviceName, &pService->info.serviceName))
		{
			foundServiceName = RSSL_TRUE;

			if ( /* Wait for service to come up at least once before we begin using it. */
					pService->flags & RDM_SVCF_HAS_STATE
					&& pService->state.serviceState
					&& (!(pService->state.flags & RDM_SVC_STF_HAS_ACCEPTING_REQS) || pService->state.acceptingRequests))
			{
				pConsumerThread->pDesiredService = pService;

				/* send dictionary requests if not using VA Reactor */
				if (consPerfConfig.useReactor == RSSL_FALSE && consPerfConfig.useWatchlist == RSSL_FALSE)
				{
					if (pConsumerThread->dictionaryStateFlags != (DICTIONARY_STATE_HAVE_FIELD_DICT | DICTIONARY_STATE_HAVE_ENUM_DICT))
						if ( (ret = sendDictionaryRequests(pConsumerThread, (RsslUInt16)pConsumerThread->pDesiredService->serviceId,
										FIELD_DICTIONARY_STREAM_ID, ENUM_TYPE_DICTIONARY_STREAM_ID)) != RSSL_RET_SUCCESS)
						{
							rsslCloseChannel(pConsumerThread->pChannel, &closeError);
							shutdownThreads = RSSL_TRUE;
							return RSSL_RET_FAILURE;
						}
				}

				/* send dictionary requests if using VA Watchlist */
				if (consPerfConfig.useWatchlist == RSSL_TRUE)
				{
					if (pConsumerThread->dictionaryStateFlags != (DICTIONARY_STATE_HAVE_FIELD_DICT | DICTIONARY_STATE_HAVE_ENUM_DICT))
						if ( (ret = sendWatchlistDictionaryRequests(pConsumerThread, (RsslUInt16)pConsumerThread->pDesiredService->serviceId,
										FIELD_DICTIONARY_STREAM_ID, ENUM_TYPE_DICTIONARY_STREAM_ID)) != RSSL_RET_SUCCESS)
						{
							rsslCloseChannel(pConsumerThread->pChannel, &closeError);
							shutdownThreads = RSSL_TRUE;
							return RSSL_RET_FAILURE;
						}
				}

				if (runTunnelServiceUpdate)
				{
					tunnelStreamHandlerProcessServiceUpdate(&pConsumerThread->perfTunnelMsgHandler.tunnelStreamHandler,
						&tunnelServiceNameBuffer, pService);

					if (consPerfConfig.tunnelMessagingEnabled == RSSL_TRUE)
					{
						pConsumerThread->tunnelMessagingEnabled = RSSL_TRUE;
						strncpy(pConsumerThread->tunnelStreamServiceName, consPerfConfig.tunnelStreamServiceName,
							sizeof(pConsumerThread->tunnelStreamServiceName));
						if (createTunnel(pConsumerThread) < RSSL_RET_SUCCESS)
						{
							return RSSL_RC_CRET_FAILURE;
						}
					}
				}

				break;
			}
		}
		else if (pConsumerThread->pDesiredService &&
				 pService->serviceId == pConsumerThread->pDesiredService->serviceId &&
			     pService->action == RSSL_MPEA_DELETE_ENTRY)
		{
			pConsumerThread->pDesiredService = NULL;
			foundServiceName = RSSL_TRUE;
		}
	}

	if (!pConsumerThread->pDesiredService)
	{
		if (foundServiceName)
			printf("Service %.*s is currently down.\n\n", serviceName.length, serviceName.data);
		else
			printf("Service %.*s not found in message.\n\n", serviceName.length, serviceName.data);
	}
	else
		printf("Service %.*s is up.\n\n", serviceName.length, serviceName.data);

	return RSSL_RET_SUCCESS;
}

static RsslRet processSourceDirectoryRespJson(ConsumerThread* pConsumerThread, cJSON* json)
{
	RsslError closeError;
	RsslBool foundServiceName = RSSL_FALSE;

	RsslRDMDirectoryMsgType rsslRDMDirectoryMsgType = jsonGetRDMDirectoryMsgType(json);

	switch (rsslRDMDirectoryMsgType)
	{
	case RDM_DR_MT_REFRESH:
		break;
	case RDM_DR_MT_UPDATE:
		break;
	case RDM_DR_MT_STATUS:
	default:
		rsslSetErrorInfo(&pConsumerThread->threadErrorInfo, RSSL_EIC_FAILURE, RSSL_RET_FAILURE, __FILE__, __LINE__,
			(char*)"Error: Received unhandled directory message type %d.", rsslRDMDirectoryMsgType);
		rsslCloseChannel(pConsumerThread->pChannel, &closeError);
		shutdownThreads = RSSL_TRUE;
		return RSSL_RET_FAILURE;
	}

	/* Search service list for our desired service. */
	cJSON* jsonMapNode = cJSON_GetObjectItem(json, "Map");
	cJSON* jsonMapEntriesNode = cJSON_GetObjectItem(jsonMapNode, "Entries");
	cJSON* jsonMapEntry = NULL;
	cJSON_ArrayForEach(jsonMapEntry, jsonMapEntriesNode)
	{
		const char * serviceName = NULL;
		RsslUInt serviceState = 0;
		RsslUInt acceptingRequests = 0;

		cJSON* jsonFilterListNode = cJSON_GetObjectItem(jsonMapEntry, "FilterList");
		cJSON* jsonEntriesNode = cJSON_GetObjectItem(jsonFilterListNode, "Entries");
		cJSON* jsonEntry = NULL;
		cJSON_ArrayForEach(jsonEntry, jsonEntriesNode)
		{
			cJSON* jsonIdNode = cJSON_GetObjectItem(jsonEntry, "ID");
			RsslUInt8 id = (RsslUInt8)cJSON_GetNumberValue(jsonIdNode);
			switch (id)
			{
			case RDM_DIRECTORY_SERVICE_INFO_ID:
				{
					cJSON* jsonElementsNode = cJSON_GetObjectItem(jsonEntry, "Elements");
					cJSON* jsonNameNode = cJSON_GetObjectItem(jsonElementsNode, "Name");
					serviceName = cJSON_GetStringValue(jsonNameNode);

					// [TODO] service QoS
			}
				break;
			case RDM_DIRECTORY_SERVICE_STATE_ID:
				{
					cJSON* jsonElementsNode = cJSON_GetObjectItem(jsonEntry, "Elements");
					cJSON* jsonServiceStateNode = cJSON_GetObjectItem(jsonElementsNode, "ServiceState");
					serviceState = (RsslUInt)cJSON_GetNumberValue(jsonServiceStateNode);
					cJSON* jsonAcceptingRequestsNode = cJSON_GetObjectItem(jsonElementsNode, "AcceptingRequests");
					acceptingRequests = (RsslUInt)cJSON_GetNumberValue(jsonAcceptingRequestsNode);
				}
				break;
			case RDM_DIRECTORY_SERVICE_GROUP_ID:
				/*!< (3) Source Group Filter ID is not supported in this app */
			case RDM_DIRECTORY_SERVICE_LOAD_ID:
				/*!< (4) Source Load Filter ID is not supported in this app */
			case RDM_DIRECTORY_SERVICE_DATA_ID:
				/*!< (5) Source Data Filter ID is not supported in this app */
			case RDM_DIRECTORY_SERVICE_LINK_ID:
				/*!< (6) Communication Link Filter ID is not supported in this app */
			case RDM_DIRECTORY_SERVICE_SEQ_MCAST_ID:
				/*!< (7) Sequenced Multicast Filter ID is not supported in this app */
			default:
				continue;
			}
		}

		/* Check for matching service name. */
		if (serviceName && strcmp(serviceName, consPerfConfig.serviceName) == 0)
		{
			foundServiceName = RSSL_TRUE;
			if (serviceState > 0 && acceptingRequests > 0)
			{
				cJSON* jsonKeyNode = cJSON_GetObjectItem(jsonMapEntry, "Key");
				pConsumerThread->jsonDesiredServiceId = (RsslUInt)cJSON_GetNumberValue(jsonKeyNode);
			}
		}
	}

	if (!pConsumerThread->jsonDesiredServiceId)
	{
		if (foundServiceName)
			printf("Service %s is currently down.\n\n", consPerfConfig.serviceName);
		else
			printf("Service %s not found in message.\n\n", consPerfConfig.serviceName);
	}
	else
		printf("Service %s is up.\n\n", consPerfConfig.serviceName);

	return RSSL_RET_SUCCESS;
}

static RsslRet processDictionaryResp(ConsumerThread* pConsumerThread, RsslRDMDictionaryMsg *pDictionaryMsg, RsslDecodeIterator *pDIter)
{
	RsslRet ret;
	RsslError closeError;
	char errTxt[256];
	RsslBuffer errorText = {255, (char*)errTxt};

	switch(pDictionaryMsg->rdmMsgBase.rdmMsgType)
	{
		case RDM_DC_MT_REFRESH:
			if (pDictionaryMsg->rdmMsgBase.streamId == FIELD_DICTIONARY_STREAM_ID)
			{
				if ((ret = rsslDecodeFieldDictionary(pDIter, pConsumerThread->pDictionary, RDM_DICTIONARY_NORMAL, &errorText)) != RSSL_RET_SUCCESS)
				{
					rsslSetErrorInfo(&pConsumerThread->threadErrorInfo, RSSL_EIC_FAILURE, ret, __FILE__, __LINE__,
							(char*)"rsslDecodeFieldDictionary() failed: %.*s", errorText.length, errorText.data);
					rsslCloseChannel(pConsumerThread->pChannel, &closeError);
					shutdownThreads = RSSL_TRUE;
					return RSSL_RET_FAILURE;
				}

				if (pDictionaryMsg->refresh.flags & RDM_DC_RFF_IS_COMPLETE)
				{
					printf("Field dictionary downloaded.\n\n");
					pConsumerThread->dictionaryStateFlags = (DictionaryStateFlags)(pConsumerThread->dictionaryStateFlags | DICTIONARY_STATE_HAVE_FIELD_DICT);
				}

			}
			else if (pDictionaryMsg->rdmMsgBase.streamId == ENUM_TYPE_DICTIONARY_STREAM_ID)
			{
				if ((ret = rsslDecodeEnumTypeDictionary(pDIter, pConsumerThread->pDictionary, RDM_DICTIONARY_NORMAL, &errorText)) != RSSL_RET_SUCCESS)
				{
					rsslSetErrorInfo(&pConsumerThread->threadErrorInfo, RSSL_EIC_FAILURE, ret, __FILE__, __LINE__,
							(char*)"rsslDecodeEnumTypeDictionary() failed: %.*s", errorText.length, errorText.data);
					rsslCloseChannel(pConsumerThread->pChannel, &closeError);
					shutdownThreads = RSSL_TRUE;
					return RSSL_RET_FAILURE;
				}

				if (pDictionaryMsg->refresh.flags & RDM_DC_RFF_IS_COMPLETE)
				{
					printf("Enumerated Types downloaded.\n\n");
					pConsumerThread->dictionaryStateFlags = (DictionaryStateFlags)(pConsumerThread->dictionaryStateFlags | DICTIONARY_STATE_HAVE_ENUM_DICT);
				}

			}

			if (pConsumerThread->dictionaryStateFlags == (DICTIONARY_STATE_HAVE_FIELD_DICT | DICTIONARY_STATE_HAVE_ENUM_DICT))
				printf("Dictionary download complete.\n");
													
			break;
		case RDM_DC_MT_STATUS:
			break;
		default:
			ret = RSSL_RET_FAILURE;
			rsslSetErrorInfo(&pConsumerThread->threadErrorInfo, RSSL_EIC_FAILURE, ret, __FILE__, __LINE__,
					(char*)"Received unhandled dictionary message type %d.", pDictionaryMsg->rdmMsgBase.rdmMsgType);
			shutdownThreads = RSSL_TRUE;
			return RSSL_RET_FAILURE;
	}

	return RSSL_RET_SUCCESS;
}

static RsslRet processDictionaryRespJson(ConsumerThread* pConsumerThread, RsslDomainTypes rsslDomainType, cJSON* json)
{
	return RSSL_RET_SUCCESS;
}

static RsslRet processDefaultMsgResp(ConsumerThread* pConsumerThread, RsslMsg *pMsg, RsslDecodeIterator *pDIter)
{
	RsslRet ret = 0;
	RsslError closeError;

	RsslTimeValue decodeTimeStart, decodeTimeEnd;
	ItemInfo *pItemInfo = NULL;

	if (consPerfConfig.measureDecode)
		decodeTimeStart = rsslGetTimeNano();
	
	if (pMsg->msgBase.streamId < ITEM_STREAM_ID_START || pMsg->msgBase.streamId 
				>= ITEM_STREAM_ID_START + consPerfConfig.commonItemCount + pConsumerThread->itemListCount)
	{
		rsslSetErrorInfo(&pConsumerThread->threadErrorInfo, RSSL_EIC_FAILURE, ret, __FILE__, __LINE__,
					(char*)"Error: Received message with unexpected Stream ID: %d  Class: %s(%u) Domain: %s(%u)",
					pMsg->msgBase.streamId,
					rsslMsgClassToString(pMsg->msgBase.msgClass), pMsg->msgBase.msgClass,
					rsslDomainTypeToString(pMsg->msgBase.domainType), pMsg->msgBase.domainType);
		shutdownThreads = RSSL_TRUE;
		return RSSL_RET_FAILURE;
	}

	pItemInfo = &pConsumerThread->itemRequestList[pMsg->msgBase.streamId].itemInfo;

	if (pItemInfo->attributes.domainType != pMsg->msgBase.domainType)
	{
		rsslSetErrorInfo(&pConsumerThread->threadErrorInfo, RSSL_EIC_FAILURE, ret, __FILE__, __LINE__,
					(char*)"Error: Received message with domain type %u vs. expected type %u",
					pMsg->msgBase.domainType, pItemInfo->attributes.domainType);
		rsslCloseChannel(pConsumerThread->pChannel, &closeError);
		shutdownThreads = RSSL_TRUE;
		return RSSL_RET_FAILURE;
	}

	switch(pMsg->msgBase.msgClass)
	{
		case RSSL_MC_UPDATE:
		{

			countStatIncr(pConsumerThread->stats.imageRetrievalEndTime ?
					&pConsumerThread->stats.steadyStateUpdateCount :
					&pConsumerThread->stats.startupUpdateCount);


			if (!pConsumerThread->stats.firstUpdateTime)
				pConsumerThread->stats.firstUpdateTime = rsslGetTimeNano();

			if((ret = decodePayload(pDIter, pMsg, pConsumerThread)) 
					!= RSSL_RET_SUCCESS)
			{
				rsslCloseChannel(pConsumerThread->pChannel, &closeError);
				shutdownThreads = RSSL_TRUE;
				return RSSL_RET_FAILURE;
			}

			if (consPerfConfig.measureDecode)
			{
				decodeTimeEnd = rsslGetTimeNano();
				timeRecordSubmit(&pConsumerThread->updateDecodeTimeRecords, decodeTimeStart, decodeTimeEnd, 1000);
			}
			break;
		}

		case RSSL_MC_REFRESH:
		{
			RsslInt32 streamId = pMsg->msgBase.streamId;

			countStatIncr(&pConsumerThread->stats.refreshCount);

			if((ret = decodePayload(pDIter, pMsg, pConsumerThread)) 
					!= RSSL_RET_SUCCESS)
			{
				rsslCloseChannel(pConsumerThread->pChannel, &closeError);
				shutdownThreads = RSSL_TRUE;
				return RSSL_RET_FAILURE;
			}

			if(!pConsumerThread->stats.imageRetrievalEndTime && rsslQueueGetElementCount(&pConsumerThread->waitingForRefreshQueue))
			{
				if (rsslIsFinalState(&pMsg->refreshMsg.state))
				{
					if (pItemInfo)
					{
						RsslBuffer *pName = &pItemInfo->attributes.pMsgKey->name;
						rsslSetErrorInfo(&pConsumerThread->threadErrorInfo, RSSL_EIC_FAILURE, RSSL_RET_FAILURE, __FILE__, __LINE__,
								(char*)"Received unexpected final state %s in refresh for item: %.*s", rsslStreamStateToString(pMsg->refreshMsg.state.streamState),
								pName->length, pName->data);
					}
					else
					{
						if (pMsg->msgBase.msgKey.flags & RSSL_MKF_HAS_NAME)
							rsslSetErrorInfo(&pConsumerThread->threadErrorInfo, RSSL_EIC_FAILURE, RSSL_RET_FAILURE, __FILE__, __LINE__,
									(char*)"Received unexpected final state %s in refresh for item: %.*s", rsslStreamStateToString(pMsg->refreshMsg.state.streamState),
									pMsg->msgBase.msgKey.name.length, pMsg->msgBase.msgKey.name.data);
						else
							rsslSetErrorInfo(&pConsumerThread->threadErrorInfo, RSSL_EIC_FAILURE, RSSL_RET_FAILURE, __FILE__, __LINE__,
									(char*)"Received unexpected final state %s in refresh for unknown item", rsslStreamStateToString(pMsg->refreshMsg.state.streamState));
					}
					rsslCloseChannel(pConsumerThread->pChannel, &closeError);
					shutdownThreads = RSSL_TRUE;
					return RSSL_RET_FAILURE;
				}

				if (pMsg->refreshMsg.flags & RSSL_RFMF_REFRESH_COMPLETE
						&& pMsg->refreshMsg.state.dataState == RSSL_DATA_OK)
				{
					{
						/* Received a complete refresh. */
						if ( pConsumerThread->itemRequestList[streamId].requestState == ITEM_WAITING_FOR_REFRESH)
						{
							pConsumerThread->itemRequestList[streamId].requestState = ITEM_HAS_REFRESH;
							rsslQueueRemoveLink(&pConsumerThread->waitingForRefreshQueue, &pConsumerThread->itemRequestList[streamId].link);
							rsslQueueAddLinkToBack(&pConsumerThread->refreshCompleteQueue, &pConsumerThread->itemRequestList[streamId].link);

							if (pConsumerThread->itemRequestList[streamId].itemInfo.itemFlags & ITEM_IS_STREAMING_REQ)
							{
								if (pConsumerThread->itemRequestList[streamId].itemInfo.itemFlags & ITEM_IS_POST && consPerfConfig.postsPerSec)
								{
									pConsumerThread->itemRequestList[streamId].itemInfo.myQueue = &pConsumerThread->postItemQueue;
									rotatingQueueInsert(&pConsumerThread->postItemQueue, &pConsumerThread->itemRequestList[streamId].postQueueLink);
								}
								if (pConsumerThread->itemRequestList[streamId].itemInfo.itemFlags & ITEM_IS_GEN_MSG && consPerfConfig.genMsgsPerSec)
								{
									pConsumerThread->itemRequestList[streamId].itemInfo.myQueue = &pConsumerThread->genMsgItemQueue;
									rotatingQueueInsert(&pConsumerThread->genMsgItemQueue, &pConsumerThread->itemRequestList[streamId].genMsgQueueLink);
								}
							}

							if (rsslQueueGetElementCount(&pConsumerThread->refreshCompleteQueue) == pConsumerThread->itemListCount)
							{
								pConsumerThread->stats.imageRetrievalEndTime = rsslGetTimeNano();
								pConsumerThread->stats.steadyStateLatencyTime = pConsumerThread->stats.imageRetrievalEndTime + (RsslUInt64)consPerfConfig.delaySteadyStateCalc * 1000000ULL;
							}
						}
					}
				}
			}
										
			break;
		}

		case RSSL_MC_STATUS:
			countStatIncr(&pConsumerThread->stats.statusCount);

			/* Stop if an item is unexpectedly closed. */
			if ((pMsg->statusMsg.flags & RSSL_STMF_HAS_STATE)
					&& rsslIsFinalState(&pMsg->statusMsg.state))
			{
				if (pItemInfo)
				{
					RsslBuffer *pName = &pItemInfo->attributes.pMsgKey->name;
					rsslSetErrorInfo(&pConsumerThread->threadErrorInfo, RSSL_EIC_FAILURE, RSSL_RET_FAILURE, __FILE__, __LINE__,
							(char*)"Received unexpected final state %s for item: %.*s", rsslStreamStateToString(pMsg->statusMsg.state.streamState),
							pName->length, pName->data);
				}
				else
				{
					if (pMsg->msgBase.msgKey.flags & RSSL_MKF_HAS_NAME)
						rsslSetErrorInfo(&pConsumerThread->threadErrorInfo, RSSL_EIC_FAILURE, RSSL_RET_FAILURE, __FILE__, __LINE__,
								(char*)"Received unexpected final state %s for item: %.*s", rsslStreamStateToString(pMsg->statusMsg.state.streamState),
								pMsg->msgBase.msgKey.name.length, pMsg->msgBase.msgKey.name.data);
					else
						rsslSetErrorInfo(&pConsumerThread->threadErrorInfo, RSSL_EIC_FAILURE, RSSL_RET_FAILURE, __FILE__, __LINE__,
								(char*)"Received unexpected final state %s for unknown item", rsslStreamStateToString(pMsg->statusMsg.state.streamState));
				}
				rsslCloseChannel(pConsumerThread->pChannel, &closeError);
				shutdownThreads = RSSL_TRUE;
				return RSSL_RET_FAILURE;
			}

			break;
		case RSSL_MC_GENERIC:
			countStatIncr(&pConsumerThread->stats.genMsgRecvCount);
			if (!pConsumerThread->stats.firstGenMsgRecvTime)
				pConsumerThread->stats.firstGenMsgRecvTime = rsslGetTimeNano();
			if((ret = decodePayload(pDIter, pMsg, pConsumerThread)) 
					!= RSSL_RET_SUCCESS)
			{
				rsslCloseChannel(pConsumerThread->pChannel, &closeError);
				shutdownThreads = RSSL_TRUE;
				return RSSL_RET_FAILURE;
			}
			break;
		default:
			rsslSetErrorInfo(&pConsumerThread->threadErrorInfo, RSSL_EIC_FAILURE, RSSL_RET_FAILURE, __FILE__, __LINE__,
					(char*)"Received unhandled msg class %u on stream ID %d.", pMsg->msgBase.msgClass, pMsg->msgBase.streamId);
			rsslCloseChannel(pConsumerThread->pChannel, &closeError);
			return RSSL_RET_FAILURE;
	}

	return RSSL_RET_SUCCESS;
}

static RsslRet processDefaultMsgRespJson(ConsumerThread* pConsumerThread, RsslDomainTypes rsslDomainType, cJSON* json)
{
	RsslRet ret = 0;
	RsslError closeError;

	RsslInt32 rsslStreamId;
	RsslMsgClasses rsslMsgClass;

	RsslTimeValue decodeTimeStart, decodeTimeEnd;
	ItemInfo *pItemInfo = NULL;

	rsslMsgClass = jsonGetMsgClass(json);

	/* process JSON PING&PONG here */
	switch (rsslMsgClass)
	{
	case RSSL_MC_JSON_PING:
	{
		RsslError err;
		RsslBuffer PONG_MESSAGE = { 15, (char*)"{\"Type\":\"Pong\"}" };

		RsslBuffer *msgBuf = rsslGetBuffer(pConsumerThread->pChannel, PONG_MESSAGE.length, RSSL_FALSE, &err);
		if (msgBuf)
		{
			memcpy(msgBuf->data, PONG_MESSAGE.data, PONG_MESSAGE.length);

			/* Reply with JSON PONG message to the sender */
			ret = sendMessage(pConsumerThread, msgBuf);
			if (ret != RSSL_RET_SUCCESS)
				return ret;
			else
				return RSSL_RET_READ_PING;
		}
		else
		{
			snprintf(pConsumerThread->threadErrorInfo.rsslError.text, MAX_RSSL_ERROR_TEXT,
				"Failed to get buffer for Ping response :%s", err.text);
			return RSSL_RET_FAILURE;
		}
		return RSSL_RET_READ_PING;
		break;
	}
	case RSSL_MC_JSON_PONG:
	{
		return RSSL_RET_READ_PING;
		break;
	}
	}

	if (consPerfConfig.measureDecode)
		decodeTimeStart = rsslGetTimeNano();

	rsslStreamId = jsonGetStreamId(json);

	if (rsslStreamId < ITEM_STREAM_ID_START || rsslStreamId
		>= ITEM_STREAM_ID_START + consPerfConfig.commonItemCount + pConsumerThread->itemListCount)
	{
		rsslSetErrorInfo(&pConsumerThread->threadErrorInfo, RSSL_EIC_FAILURE, ret, __FILE__, __LINE__,
			(char*)"Error: Received message with unexpected Stream ID: %d  Class: %s(%u) Domain: %s(%u)",
			rsslStreamId,
			rsslMsgClassToString(rsslMsgClass), rsslMsgClass,
			rsslDomainTypeToString(rsslDomainType), rsslDomainType);
		shutdownThreads = RSSL_TRUE;
		return RSSL_RET_FAILURE;
	}

	pItemInfo = &pConsumerThread->itemRequestList[rsslStreamId].itemInfo;

	if (pItemInfo->attributes.domainType != rsslDomainType)
	{
		rsslSetErrorInfo(&pConsumerThread->threadErrorInfo, RSSL_EIC_FAILURE, ret, __FILE__, __LINE__,
			(char*)"Error: Received message with domain type %u vs. expected type %u",
			rsslDomainType, pItemInfo->attributes.domainType);
		rsslCloseChannel(pConsumerThread->pChannel, &closeError);
		shutdownThreads = RSSL_TRUE;
		return RSSL_RET_FAILURE;
	}

	switch (rsslMsgClass)
	{
	case RSSL_MC_UPDATE:
	{
		countStatIncr(pConsumerThread->stats.imageRetrievalEndTime ?
			&pConsumerThread->stats.steadyStateUpdateCount :
			&pConsumerThread->stats.startupUpdateCount);

		if (!pConsumerThread->stats.firstUpdateTime)
			pConsumerThread->stats.firstUpdateTime = rsslGetTimeNano();

		if ((ret = decodePayloadJson(pConsumerThread, rsslDomainType, rsslMsgClass, json))
			!= RSSL_RET_SUCCESS)
		{
			rsslCloseChannel(pConsumerThread->pChannel, &closeError);
			shutdownThreads = RSSL_TRUE;
			return RSSL_RET_FAILURE;
		}

		if (consPerfConfig.measureDecode)
		{
			decodeTimeEnd = rsslGetTimeNano();
			timeRecordSubmit(&pConsumerThread->updateDecodeTimeRecords, decodeTimeStart, decodeTimeEnd, 1000);
		}

		break;
	}
	case RSSL_MC_REFRESH:
	{
		countStatIncr(&pConsumerThread->stats.refreshCount);

		if ((ret = decodePayloadJson(pConsumerThread, rsslDomainType, rsslMsgClass, json))
			!= RSSL_RET_SUCCESS)
		{
			rsslCloseChannel(pConsumerThread->pChannel, &closeError);
			shutdownThreads = RSSL_TRUE;
			return RSSL_RET_FAILURE;
		}

		if (!pConsumerThread->stats.imageRetrievalEndTime && rsslQueueGetElementCount(&pConsumerThread->waitingForRefreshQueue))
		{
			if (jsonIsFinalState(json) == RSSL_TRUE)
			{
				if (pItemInfo)
				{
					RsslBuffer *pName = &pItemInfo->attributes.pMsgKey->name;
					rsslSetErrorInfo(&pConsumerThread->threadErrorInfo, RSSL_EIC_FAILURE, RSSL_RET_FAILURE, __FILE__, __LINE__,
						(char*)"Received unexpected final state %s in refresh for item: %.*s", rsslStreamStateToString(jsonGetStreamState(json)),
						pName->length, pName->data);
				}
				else
				{
					const char* name = jsonGetMsgKeyName(json);
					if (name != NULL)
						rsslSetErrorInfo(&pConsumerThread->threadErrorInfo, RSSL_EIC_FAILURE, RSSL_RET_FAILURE, __FILE__, __LINE__,
						(char*)"Received unexpected final state %s in refresh for item: %s", rsslStreamStateToString(jsonGetStreamState(json)),
							name);
					else
						rsslSetErrorInfo(&pConsumerThread->threadErrorInfo, RSSL_EIC_FAILURE, RSSL_RET_FAILURE, __FILE__, __LINE__,
						(char*)"Received unexpected final state %s in refresh for unknown item", rsslStreamStateToString(jsonGetStreamState(json)));
				}

				rsslCloseChannel(pConsumerThread->pChannel, &closeError);
				shutdownThreads = RSSL_TRUE;
				return RSSL_RET_FAILURE;
			}

			if (jsonIsRefreshComplete(json) == RSSL_TRUE
				&& jsonRefreshMsgDataStateOK(json) == RSSL_TRUE)
			{
				/* Received a complete refresh. */
				if (pConsumerThread->itemRequestList[rsslStreamId].requestState == ITEM_WAITING_FOR_REFRESH)
				{
					pConsumerThread->itemRequestList[rsslStreamId].requestState = ITEM_HAS_REFRESH;
					rsslQueueRemoveLink(&pConsumerThread->waitingForRefreshQueue, &pConsumerThread->itemRequestList[rsslStreamId].link);
					rsslQueueAddLinkToBack(&pConsumerThread->refreshCompleteQueue, &pConsumerThread->itemRequestList[rsslStreamId].link);

					if (pConsumerThread->itemRequestList[rsslStreamId].itemInfo.itemFlags & ITEM_IS_STREAMING_REQ)
					{
						if (pConsumerThread->itemRequestList[rsslStreamId].itemInfo.itemFlags & ITEM_IS_POST && consPerfConfig.postsPerSec)
						{
							pConsumerThread->itemRequestList[rsslStreamId].itemInfo.myQueue = &pConsumerThread->postItemQueue;
							rotatingQueueInsert(&pConsumerThread->postItemQueue, &pConsumerThread->itemRequestList[rsslStreamId].postQueueLink);
						}
						if (pConsumerThread->itemRequestList[rsslStreamId].itemInfo.itemFlags & ITEM_IS_GEN_MSG && consPerfConfig.genMsgsPerSec)
						{
							pConsumerThread->itemRequestList[rsslStreamId].itemInfo.myQueue = &pConsumerThread->genMsgItemQueue;
							rotatingQueueInsert(&pConsumerThread->genMsgItemQueue, &pConsumerThread->itemRequestList[rsslStreamId].genMsgQueueLink);
						}
					}

					if (rsslQueueGetElementCount(&pConsumerThread->refreshCompleteQueue) == pConsumerThread->itemListCount)
					{
						pConsumerThread->stats.imageRetrievalEndTime = rsslGetTimeNano();
						pConsumerThread->stats.steadyStateLatencyTime = pConsumerThread->stats.imageRetrievalEndTime + (RsslUInt64)consPerfConfig.delaySteadyStateCalc * 1000000ULL;
					}
				}
			}
		}

		break;
	}
	case RSSL_MC_STATUS:
	{
		countStatIncr(&pConsumerThread->stats.statusCount);

		/* Stop if an item is unexpectedly closed. */
		if (jsonIsFinalState(json) == RSSL_TRUE)
		{
			if (pItemInfo)
			{
				RsslBuffer *pName = &pItemInfo->attributes.pMsgKey->name;
				rsslSetErrorInfo(&pConsumerThread->threadErrorInfo, RSSL_EIC_FAILURE, RSSL_RET_FAILURE, __FILE__, __LINE__,
					(char*)"Received unexpected final state %s in refresh for item: %.*s", rsslStreamStateToString(jsonGetStreamState(json)),
					pName->length, pName->data);
			}
			else
			{
				const char* name = jsonGetMsgKeyName(json);
				if (name != NULL)
					rsslSetErrorInfo(&pConsumerThread->threadErrorInfo, RSSL_EIC_FAILURE, RSSL_RET_FAILURE, __FILE__, __LINE__,
						(char*)"Received unexpected final state %s in refresh for item: %s", rsslStreamStateToString(jsonGetStreamState(json)),
						name);
				else
					rsslSetErrorInfo(&pConsumerThread->threadErrorInfo, RSSL_EIC_FAILURE, RSSL_RET_FAILURE, __FILE__, __LINE__,
						(char*)"Received unexpected final state %s in refresh for unknown item", rsslStreamStateToString(jsonGetStreamState(json)));
			}

			rsslCloseChannel(pConsumerThread->pChannel, &closeError);
			shutdownThreads = RSSL_TRUE;
			return RSSL_RET_FAILURE;
		}

		break;
	}
	case RSSL_MC_GENERIC:
	{
		countStatIncr(&pConsumerThread->stats.genMsgRecvCount);

		if (!pConsumerThread->stats.firstGenMsgRecvTime)
			pConsumerThread->stats.firstGenMsgRecvTime = rsslGetTimeNano();

		if ((ret = decodePayloadJson(pConsumerThread, rsslDomainType, rsslMsgClass, json))
			!= RSSL_RET_SUCCESS)
		{
			rsslCloseChannel(pConsumerThread->pChannel, &closeError);
			shutdownThreads = RSSL_TRUE;
			return RSSL_RET_FAILURE;
		}
		break;
	}
	default:
	{
		rsslSetErrorInfo(&pConsumerThread->threadErrorInfo, RSSL_EIC_FAILURE, RSSL_RET_FAILURE, __FILE__, __LINE__,
			(char*)"Received unhandled msg class %u on stream ID %d.", rsslMsgClass, rsslStreamId);
		rsslCloseChannel(pConsumerThread->pChannel, &closeError);
		return RSSL_RET_FAILURE;
	}
	}

	return RSSL_RET_SUCCESS;
}

RsslRet processJsonMsg(ConsumerThread* pConsumerThread, cJSON* json)
{
	RsslError closeError;
	RsslRet ret = 0;

	RsslDomainTypes rsslDomainType = jsonGetDomainType(json);

	switch (rsslDomainType)
	{
	case RSSL_DMT_LOGIN:
	{
		if ((ret = processLoginRespJson(pConsumerThread, json)) != RSSL_RET_SUCCESS)
		{
			return ret;
		}

		if (!pConsumerThread->loggedIn)
		{
			if ((ret = sendDirectoryRequestJson(pConsumerThread, DIRECTORY_STREAM_ID)) != RSSL_RET_SUCCESS)
			{
				rsslCloseChannel(pConsumerThread->pChannel, &closeError);
				shutdownThreads = RSSL_TRUE;
				return ret;
			}

			pConsumerThread->loggedIn = RSSL_TRUE;
		}
		break;
	}

	case RSSL_DMT_SOURCE:
	{
		printf("Received source directory response.\n\n");

		/* Found our service already, ignore the message. */
		if (pConsumerThread->pDesiredService)
			break;

		if ((ret = processSourceDirectoryRespJson(pConsumerThread, json)) != RSSL_RET_SUCCESS)
		{
			return ret;
		}

		break;
	}

	case RSSL_DMT_DICTIONARY:
	{
		printf("Received dictionary response.\n\n");

		if ((ret = processDictionaryRespJson(pConsumerThread, rsslDomainType, json)) != RSSL_RET_SUCCESS)
		{
			return ret;
		}

		break;
	}

	default:
	{
		if ((ret = processDefaultMsgRespJson(pConsumerThread, rsslDomainType, json)) != RSSL_RET_SUCCESS)
		{
			if (ret == RSSL_RET_READ_PING)
				pConsumerThread->receivedPing = RSSL_TRUE;
			else
				return ret;
		}

		break;
	}
	}

	return RSSL_RET_SUCCESS;
}

RSSL_THREAD_DECLARE(runConsumerChannelConnection, threadStruct) 
{
	ConsumerThread* pConsumerThread = (ConsumerThread*)threadStruct;
	RsslInt64 nsecPerTick;
	RsslTimeValue currentTime, nextTickTime;
	struct timeval time_interval;
	fd_set useRead;
	fd_set useExcept;
	fd_set useWrt;
	int selRet;
	RsslRet ret = 0;
	RsslError closeError;
	RsslBuffer *msgBuf=0;
	RsslRet	readret;
	RsslMsg msg = RSSL_INIT_MSG;
	RsslDecodeIterator dIter;
	char errTxt[256];
	RsslBuffer errorText = {255, (char*)errTxt};
	RsslTimeValue decodetime;

	pConsumerThread->loggedIn = RSSL_FALSE;

	RsslInt32 postsPerTick, postsPerTickRemainder;
	RsslInt32 genMsgsPerTick, genMsgsPerTickRemainder;
	RsslInt32 currentTicks = 0;
	LatencyRandomArray postLatencyRandomArray, genMsgLatencyRandomArray;

	nsecPerTick = 1000000000 / consPerfConfig.ticksPerSec;

	postsPerTick = consPerfConfig.postsPerSec / consPerfConfig.ticksPerSec;
	postsPerTickRemainder = consPerfConfig.postsPerSec % consPerfConfig.ticksPerSec;
	genMsgsPerTick = consPerfConfig.genMsgsPerSec / consPerfConfig.ticksPerSec;
	genMsgsPerTickRemainder = consPerfConfig.genMsgsPerSec % consPerfConfig.ticksPerSec;

	rsslInitQueue(&pConsumerThread->requestQueue);
	rsslInitQueue(&pConsumerThread->waitingForRefreshQueue);
	rsslInitQueue(&pConsumerThread->refreshCompleteQueue);
	initRotatingQueue(&pConsumerThread->postItemQueue);
	initRotatingQueue(&pConsumerThread->genMsgItemQueue);

	RESET_CJSON_ALLOCATOR();

	cJSON_Hooks hooks;
	hooks.malloc_fn = cJSON_malloc_fn;
	hooks.free_fn = cJSON_free_fn;
	cJSON_InitHooks(&hooks);

	if (initialize(pConsumerThread, &postLatencyRandomArray, &genMsgLatencyRandomArray) < RSSL_RET_SUCCESS)
	{
		return RSSL_THREAD_RETURN();
	}

	printEstimatedPostMsgSizes(pConsumerThread);
	printEstimatedGenMsgSizes(pConsumerThread);

	currentTime = rsslGetTimeNano();
	nextTickTime = currentTime + nsecPerTick;
	time_interval.tv_sec = 0;

	while(1)
	{
		if(shutdownThreads == RSSL_TRUE)
		{
			rsslCloseChannel(pConsumerThread->pChannel, &closeError);
			return RSSL_THREAD_RETURN();
		}
				
		useRead = pConsumerThread->readfds;
		useExcept = pConsumerThread->exceptfds;
		useWrt = pConsumerThread->wrtfds;

		currentTime = rsslGetTimeNano();
		time_interval.tv_usec = (long)((currentTime > nextTickTime) ? 0 : ((nextTickTime - currentTime)/1000));

		selRet = select(FD_SETSIZE,&useRead,&useWrt,&useExcept,&time_interval);

		if (selRet > 0)
		{
			if (pConsumerThread->pChannel != NULL && FD_ISSET(pConsumerThread->pChannel->socketId, &useRead))
			{
				RsslBuffer messageBuff = RSSL_INIT_BUFFER;
				RsslRet	cRet;
				RsslInt16 numConverted = 0;

				do{
					if ((msgBuf = rsslRead(pConsumerThread->pChannel,&readret,&pConsumerThread->threadRsslError)) != 0)
					{	
						numConverted = 0;

						do {
							if (pConsumerThread->pChannel->protocolType == RSSL_JSON_PROTOCOL_TYPE)
							{
								RsslErrorInfo	errInf;

								if (!consPerfConfig.convertJSON)
								{
									cJSON *root = cJSON_ParseWithLength(msgBuf->data, msgBuf->length);
									if (cJSON_IsObject(root)) {
										processJsonMsg(pConsumerThread, root);
									}
									else if (cJSON_IsArray(root)) {
										cJSON *json = NULL;
										cJSON_ArrayForEach(json, root)
										{
											processJsonMsg(pConsumerThread, json);
										}
									}

									//cJSON_Delete(root);
									RESET_CJSON_ALLOCATOR();

									cRet = RSSL_RET_END_OF_CONTAINER;
									break;
								}
								
								messageBuff.data = pConsumerThread->rjcSess.convBuff.data;
								messageBuff.length = pConsumerThread->rjcSess.convBuff.length;

								decodetime = rsslGetTimeNano();
								cRet = rjcMsgConvertFromJson(&(pConsumerThread->rjcSess), 
															 pConsumerThread->pChannel, 
															 &messageBuff, 
															 (numConverted == 0 ? msgBuf:NULL), &errInf);
								numConverted++;
								decodetime = rsslGetTimeNano() - decodetime;
							
								if (cRet == RSSL_RET_END_OF_CONTAINER)
								{
									break;
								}

								if (cRet != RSSL_RET_SUCCESS)
									continue;
							}
							else
							{
								messageBuff.data = msgBuf->data;
								messageBuff.length = msgBuf->length;
							}

							pConsumerThread->receivedPing = RSSL_TRUE;

							/* clear decode iterator */
							rsslClearDecodeIterator(&dIter);
			
							/* set version info */
							rsslSetDecodeIteratorRWFVersion(&dIter,	
														pConsumerThread->pChannel->majorVersion, 
														pConsumerThread->pChannel->minorVersion);

							rsslSetDecodeIteratorBuffer(&dIter, &messageBuff);

							ret = rsslDecodeMsg(&dIter, &msg);	

							if (ret != RSSL_RET_SUCCESS)
							{
								rsslSetErrorInfo(&pConsumerThread->threadErrorInfo, 
									RSSL_EIC_FAILURE, ret, __FILE__, __LINE__,
									(char*)"rsslDecodeMsg() failed: %d(%s) #Conv(%d) cRet%d(%s)", 
									ret, rsslRetCodeToString(ret), numConverted, 
									cRet, rsslRetCodeToString(cRet));
								rsslCloseChannel(pConsumerThread->pChannel, &closeError);
								shutdownThreads = RSSL_TRUE;
								return RSSL_THREAD_RETURN();
							}

							switch(msg.msgBase.domainType)
							{
								case RSSL_DMT_LOGIN:
								{
									char memoryChar[1024];
									RsslBuffer memoryBuffer = { 4000, memoryChar };
									RsslRDMLoginMsg loginMsg;

									if ((ret = rsslDecodeRDMLoginMsg(&dIter, &msg, &loginMsg, &memoryBuffer, &pConsumerThread->threadErrorInfo)) != RSSL_RET_SUCCESS)
									{
										rsslCloseChannel(pConsumerThread->pChannel, &closeError);
										shutdownThreads = RSSL_TRUE;
										return RSSL_THREAD_RETURN();
									} 

									if (processLoginResp(pConsumerThread, &loginMsg) < RSSL_RET_SUCCESS)
									{
										return RSSL_THREAD_RETURN();
									}

									if (!pConsumerThread->loggedIn)
									{
										if ( (ret = sendDirectoryRequest(pConsumerThread, DIRECTORY_STREAM_ID)) != RSSL_RET_SUCCESS)
										{
											rsslCloseChannel(pConsumerThread->pChannel, &closeError);
											shutdownThreads = RSSL_TRUE;
											return RSSL_THREAD_RETURN();
										}

										pConsumerThread->loggedIn = RSSL_TRUE;
									}
									break;
								}

								case RSSL_DMT_SOURCE:
								{
									char memoryChar[16384];
									RsslBuffer memoryBuffer = { 16384, memoryChar };
									RsslRDMDirectoryMsg directoryMsg;

									printf("Received source directory response.\n\n");

									/* Found our service already, ignore the message. */
									if (pConsumerThread->pDesiredService)
										break;

									if ((ret = rsslDecodeRDMDirectoryMsg(&dIter, &msg, &directoryMsg, &memoryBuffer, &pConsumerThread->threadErrorInfo)) != RSSL_RET_SUCCESS)
									{
										rsslCloseChannel(pConsumerThread->pChannel, &closeError);
										shutdownThreads = RSSL_TRUE;
										return RSSL_THREAD_RETURN();
									}

									if (processSourceDirectoryResp(pConsumerThread, &directoryMsg) < RSSL_RET_SUCCESS)
									{
										return RSSL_THREAD_RETURN();
									}

									break;
								}

								case RSSL_DMT_DICTIONARY:
								{
									char memoryChar[1024];
									RsslBuffer memoryBuffer = { 1024, memoryChar };
									RsslRDMDictionaryMsg dictionaryMsg;


									if ((ret = rsslDecodeRDMDictionaryMsg(&dIter, &msg, &dictionaryMsg, &memoryBuffer, &pConsumerThread->threadErrorInfo)) != RSSL_RET_SUCCESS)
									{
										rsslCloseChannel(pConsumerThread->pChannel, &closeError);
										shutdownThreads = RSSL_TRUE;
										return RSSL_THREAD_RETURN();
									}
									
									if (processDictionaryResp(pConsumerThread, &dictionaryMsg, &dIter) < RSSL_RET_SUCCESS)
									{
										return RSSL_THREAD_RETURN();
									}

									break;
								}

								default:
								{
									if (processDefaultMsgResp(pConsumerThread, &msg, &dIter) < RSSL_RET_SUCCESS)
									{
										return RSSL_THREAD_RETURN();
									}

									break;
								}
							}
						} while (pConsumerThread->pChannel->protocolType == RSSL_JSON_PROTOCOL_TYPE && cRet != RSSL_RET_END_OF_CONTAINER);
					}
					else
					{
						if (readret < 0)

						switch(readret)
						{
							case RSSL_RET_READ_PING:
								pConsumerThread->receivedPing = RSSL_TRUE;
								break;
							case RSSL_RET_READ_WOULD_BLOCK:
								break;
							case RSSL_RET_READ_FD_CHANGE:
								printf("\nrsslRead() FD Change - Old FD: "SOCKET_PRINT_TYPE" New FD: "SOCKET_PRINT_TYPE"\n", pConsumerThread->pChannel->oldSocketId, pConsumerThread->pChannel->socketId);
								FD_CLR(pConsumerThread->pChannel->oldSocketId, &pConsumerThread->readfds);
								FD_CLR(pConsumerThread->pChannel->oldSocketId, &pConsumerThread->exceptfds);
								FD_SET(pConsumerThread->pChannel->socketId, &pConsumerThread->readfds);
								FD_SET(pConsumerThread->pChannel->socketId, &pConsumerThread->exceptfds);
								break;
							default:
								rsslSetErrorInfoLocation(&pConsumerThread->threadErrorInfo, __FILE__, __LINE__);
								rsslCloseChannel(pConsumerThread->pChannel, &closeError);
								shutdownThreads = RSSL_TRUE;
								return RSSL_THREAD_RETURN();

						}
					}

					if (shutdownThreads)
					{
						rsslCloseChannel(pConsumerThread->pChannel, &closeError);
						return RSSL_THREAD_RETURN();
					}

				} while(readret > 0);

			}

			if (FD_ISSET(pConsumerThread->pChannel->socketId, &useWrt))
			{
				if (rtrUnlikely((ret = rsslFlush(pConsumerThread->pChannel, &pConsumerThread->threadRsslError)) < RSSL_RET_SUCCESS))
				{
					rsslSetErrorInfoLocation(&pConsumerThread->threadErrorInfo, __FILE__, __LINE__);
					rsslCloseChannel(pConsumerThread->pChannel, &closeError);
					shutdownThreads = RSSL_TRUE;
					return RSSL_THREAD_RETURN();
				}

				if (ret == RSSL_RET_SUCCESS)
					FD_CLR(pConsumerThread->pChannel->socketId, &pConsumerThread->wrtfds);
			}
		}
		else if (selRet == 0)
		{
			if (consumerThreadCheckPings(pConsumerThread))
			{
				rsslCloseChannel(pConsumerThread->pChannel, &closeError);
				shutdownThreads = RSSL_TRUE;
				return RSSL_THREAD_RETURN();
			}

			currentTime = rsslGetTimeNano();
			nextTickTime += nsecPerTick;
			
			if (pConsumerThread->pChannel->protocolType == RSSL_JSON_PROTOCOL_TYPE
				&& consPerfConfig.convertJSON == RSSL_FALSE
				&& pConsumerThread->jsonDesiredServiceId)
			{
				/* Send some item requests(assuming dictionaries are ready). */
				if (rsslQueueGetElementCount(&pConsumerThread->requestQueue)
					&& pConsumerThread->dictionaryStateFlags ==
					(DICTIONARY_STATE_HAVE_FIELD_DICT | DICTIONARY_STATE_HAVE_ENUM_DICT))
				{
					RsslInt32 requestBurstCount;

					requestBurstCount = consPerfConfig._requestsPerTick;
					if (currentTicks > consPerfConfig._requestsPerTickRemainder)
						++requestBurstCount;

					if (!pConsumerThread->stats.imageRetrievalStartTime)
						pConsumerThread->stats.imageRetrievalStartTime = rsslGetTimeNano();

					if (sendItemRequestBurstJson(pConsumerThread, requestBurstCount) < RSSL_RET_SUCCESS)
					{
						rsslCloseChannel(pConsumerThread->pChannel, &closeError);
						shutdownThreads = RSSL_TRUE;
						return RSSL_THREAD_RETURN();
					}
				}

				if (pConsumerThread->stats.imageRetrievalEndTime)
				{
					if (rotatingQueueGetCount(&pConsumerThread->postItemQueue))
					{
						if (sendPostBurstJson(pConsumerThread, &postLatencyRandomArray,
							postsPerTick + ((currentTicks < postsPerTickRemainder) ? 1 : 0))
							< RSSL_RET_SUCCESS)
						{
							rsslCloseChannel(pConsumerThread->pChannel, &closeError);
							shutdownThreads = RSSL_TRUE;
							return RSSL_THREAD_RETURN();
						}
					}
					if (rotatingQueueGetCount(&pConsumerThread->genMsgItemQueue))
					{
						if (sendGenMsgBurstJson(pConsumerThread, &genMsgLatencyRandomArray,
							genMsgsPerTick + ((currentTicks < genMsgsPerTickRemainder) ? 1 : 0))
							< RSSL_RET_SUCCESS)
						{
							rsslCloseChannel(pConsumerThread->pChannel, &closeError);
							shutdownThreads = RSSL_TRUE;
							return RSSL_THREAD_RETURN();
						}
					}
				}
			}
			else if (pConsumerThread->pDesiredService)
			{
				/* Send some item requests(assuming dictionaries are ready). */
				if (rsslQueueGetElementCount(&pConsumerThread->requestQueue)
						&& pConsumerThread->dictionaryStateFlags == 
						(DICTIONARY_STATE_HAVE_FIELD_DICT | DICTIONARY_STATE_HAVE_ENUM_DICT))
				{
					RsslInt32 requestBurstCount;

					requestBurstCount = consPerfConfig._requestsPerTick;
					if (currentTicks > consPerfConfig._requestsPerTickRemainder)
						++requestBurstCount;

					if (!pConsumerThread->stats.imageRetrievalStartTime)
						pConsumerThread->stats.imageRetrievalStartTime = rsslGetTimeNano();

					if (sendItemRequestBurst(pConsumerThread, requestBurstCount) < RSSL_RET_SUCCESS)
					{
						rsslCloseChannel(pConsumerThread->pChannel, &closeError);
						shutdownThreads = RSSL_TRUE;
						return RSSL_THREAD_RETURN();
					}
				}

				if (pConsumerThread->stats.imageRetrievalEndTime)
				{
					if (rotatingQueueGetCount(&pConsumerThread->postItemQueue))
					{
						if (sendPostBurst(pConsumerThread, &postLatencyRandomArray,
									postsPerTick + ((currentTicks < postsPerTickRemainder ) ? 1 : 0))
								< RSSL_RET_SUCCESS)
						{
							rsslCloseChannel(pConsumerThread->pChannel, &closeError);
							shutdownThreads = RSSL_TRUE;
							return RSSL_THREAD_RETURN();
						}
					}
					if (rotatingQueueGetCount(&pConsumerThread->genMsgItemQueue))
					{
						if (sendGenMsgBurst(pConsumerThread, &genMsgLatencyRandomArray,
									genMsgsPerTick + ((currentTicks < genMsgsPerTickRemainder ) ? 1 : 0))
								< RSSL_RET_SUCCESS)
						{
							rsslCloseChannel(pConsumerThread->pChannel, &closeError);
							shutdownThreads = RSSL_TRUE;
							return RSSL_THREAD_RETURN();
						}
					}
				}
			}

			if (++currentTicks == consPerfConfig.ticksPerSec)
				currentTicks = 0;
		}
		else
		{
#ifdef _WIN32
			if (WSAGetLastError() == WSAEINTR)
				continue;
#else
			if (errno == EINTR)
			{
				continue;
			}
#endif

			perror("select");
			exit(-1);
		}
	}
}

RSSL_THREAD_DECLARE(runConsumerReactorConnection, threadStruct) 
{
	ConsumerThread* pConsumerThread = (ConsumerThread*)threadStruct;
	RsslInt64 nsecPerTick;
	RsslTimeValue currentTime, nextTickTime;
	struct timeval time_interval;
	fd_set useRead;
	fd_set useExcept;
	fd_set useWrt;
	int selRet;
	RsslRet ret = 0;
	RsslErrorInfo reactorErrorInfo;
	RsslBuffer *msgBuf=0;
	RsslRet	readret;
	RsslMsg msg = RSSL_INIT_MSG;
	char errTxt[256];
	RsslBuffer errorText = {255, (char*)errTxt};

	pConsumerThread->loggedIn = RSSL_FALSE;

	RsslInt32 postsPerTick, postsPerTickRemainder;
	RsslInt32 genMsgsPerTick, genMsgsPerTickRemainder;
	RsslInt32 currentTicks = 0;
	LatencyRandomArray postLatencyRandomArray, genMsgLatencyRandomArray;

	RsslReactorDispatchOptions dispatchOptions;

	rsslClearReactorDispatchOptions(&dispatchOptions);

	nsecPerTick = 1000000000 / consPerfConfig.ticksPerSec;

	postsPerTick = consPerfConfig.postsPerSec / consPerfConfig.ticksPerSec;
	postsPerTickRemainder = consPerfConfig.postsPerSec % consPerfConfig.ticksPerSec;
	genMsgsPerTick = consPerfConfig.genMsgsPerSec / consPerfConfig.ticksPerSec;
	genMsgsPerTickRemainder = consPerfConfig.genMsgsPerSec % consPerfConfig.ticksPerSec;

	rsslInitQueue(&pConsumerThread->requestQueue);
	rsslInitQueue(&pConsumerThread->waitingForRefreshQueue);
	rsslInitQueue(&pConsumerThread->refreshCompleteQueue);
	initRotatingQueue(&pConsumerThread->postItemQueue);
	initRotatingQueue(&pConsumerThread->genMsgItemQueue);

	if (initialize(pConsumerThread, &postLatencyRandomArray, &genMsgLatencyRandomArray) < RSSL_RET_SUCCESS)
	{
		return RSSL_THREAD_RETURN();
	}

	currentTime = rsslGetTimeNano();
	nextTickTime = currentTime + nsecPerTick;
	time_interval.tv_sec = 0;

	while(1)
	{
		if(shutdownThreads == RSSL_TRUE)
		{
			perfTunnelMsgHandlerCloseStreams(&pConsumerThread->perfTunnelMsgHandler);
			rsslReactorCloseChannel(pConsumerThread->pReactor, pConsumerThread->pReactorChannel, &reactorErrorInfo);
			rsslDestroyReactor(pConsumerThread->pReactor, &reactorErrorInfo);
			return RSSL_THREAD_RETURN();
		}
				
		useRead = pConsumerThread->readfds;
		useExcept = pConsumerThread->exceptfds;
		useWrt = pConsumerThread->wrtfds;

		currentTime = rsslGetTimeNano();
		time_interval.tv_usec = (long)((currentTime > nextTickTime) ? 0 : ((nextTickTime - currentTime)/1000));

		selRet = select(FD_SETSIZE,&useRead,&useWrt,&useExcept,&time_interval);

		if (selRet > 0)
		{
			while (shutdownThreads == RSSL_FALSE &&
				   (readret = rsslReactorDispatch(pConsumerThread->pReactor, &dispatchOptions, &reactorErrorInfo)) > RSSL_RET_SUCCESS) {}
			if (readret < RSSL_RET_SUCCESS)
			{
				rsslSetErrorInfoLocation(&pConsumerThread->threadErrorInfo, __FILE__, __LINE__);

				shutdownThreads = RSSL_TRUE;
				continue;  // return RSSL_THREAD_RETURN();
			}

			if (shutdownThreads)
			{
				continue;  // return RSSL_THREAD_RETURN();
			}
		}
		else if (selRet == 0)
		{
			currentTime = rsslGetTimeNano();
			nextTickTime += nsecPerTick;
			
			if ( pConsumerThread->pDesiredService
				&& (pConsumerThread->tunnelMessagingEnabled == RSSL_FALSE
					|| pConsumerThread->perfTunnelMsgHandler.tunnelStreamHandler.pTunnelStream) )
			{
				/* Send some item requests(assuming dictionaries are ready). */
				if (rsslQueueGetElementCount(&pConsumerThread->requestQueue)
						&& pConsumerThread->dictionaryStateFlags == 
						(DICTIONARY_STATE_HAVE_FIELD_DICT | DICTIONARY_STATE_HAVE_ENUM_DICT))
				{
					RsslInt32 requestBurstCount;

					requestBurstCount = consPerfConfig._requestsPerTick;
					if (currentTicks > consPerfConfig._requestsPerTickRemainder)
						++requestBurstCount;

					if (!pConsumerThread->stats.imageRetrievalStartTime)
						pConsumerThread->stats.imageRetrievalStartTime = rsslGetTimeNano();

					if (sendItemRequestBurst(pConsumerThread, requestBurstCount) < RSSL_RET_SUCCESS)
					{
						shutdownThreads = RSSL_TRUE;
						continue;  // return RSSL_THREAD_RETURN();
					}
				}

				if (pConsumerThread->stats.imageRetrievalEndTime)
				{
					if (rotatingQueueGetCount(&pConsumerThread->postItemQueue))
					{
						if (sendPostBurst(pConsumerThread, &postLatencyRandomArray,
									postsPerTick + ((currentTicks < postsPerTickRemainder ) ? 1 : 0))
								< RSSL_RET_SUCCESS)
						{
							shutdownThreads = RSSL_TRUE;
							continue;  // return RSSL_THREAD_RETURN();
						}
					}
					if (rotatingQueueGetCount(&pConsumerThread->genMsgItemQueue))
					{
						if (sendGenMsgBurst(pConsumerThread, &genMsgLatencyRandomArray,
									genMsgsPerTick + ((currentTicks < genMsgsPerTickRemainder ) ? 1 : 0))
								< RSSL_RET_SUCCESS)
						{
							shutdownThreads = RSSL_TRUE;
							continue;  // return RSSL_THREAD_RETURN();
						}
					}
				}
			}

			if (++currentTicks == consPerfConfig.ticksPerSec)
				currentTicks = 0;
		}
		else
		{
#ifdef _WIN32
			if (WSAGetLastError() == WSAEINTR)
				continue;
#else
			if (errno == EINTR)
			{
				continue;
			}
#endif

			perror("select");
			exit(-1);
		}
	}
}

void consumerThreadInitPings(ConsumerThread* pConsumerThread)
{
	time_t currentTime = 0;

	/* get current time */
	time(&currentTime);

	/* set time client should receive next message/ping from server */
	pConsumerThread->nextReceivePingTime = currentTime + (time_t)pConsumerThread->pChannel->pingTimeout;
	pConsumerThread->nextSendPingTime = currentTime + (time_t)pConsumerThread->pChannel->pingTimeout/3;
}


RsslBool consumerThreadCheckPings(ConsumerThread* pConsumerThread)
{
	time_t currentTime = 0;

	/* get current time */
	time(&currentTime);

	// handle server pings
	// nextReceivePingTime will be zero until we connect
	if (pConsumerThread->nextReceivePingTime != 0)
	{
		if (currentTime >= pConsumerThread->nextReceivePingTime)
		{
			/* check if client received message from server since last time */
			if (pConsumerThread->receivedPing)
			{
				/* set time client should receive next message/ping from server */
				pConsumerThread->nextReceivePingTime = currentTime + (time_t)pConsumerThread->pChannel->pingTimeout;
				pConsumerThread->receivedPing = RSSL_FALSE;
			}
			else
			{	// timed out
				rsslSetErrorInfo(&pConsumerThread->threadErrorInfo, RSSL_EIC_FAILURE, RSSL_RET_FAILURE, __FILE__, __LINE__,
						(char*)"Ping timeout.");
				return RSSL_TRUE;
			}
		}

		if (currentTime >= pConsumerThread->nextSendPingTime)
		{
			RsslRet ret;

			ret = rsslPing(pConsumerThread->pChannel, &pConsumerThread->threadRsslError);

			if (ret > RSSL_RET_SUCCESS) 
				FD_SET(pConsumerThread->pChannel->socketId, &pConsumerThread->wrtfds); /* Call for flushing if bytes remain */
			else if (ret < RSSL_RET_SUCCESS)
			{
				rsslSetErrorInfoLocation(&pConsumerThread->threadErrorInfo, __FILE__, __LINE__);
				return RSSL_TRUE;
			}

			/* set time to send next ping to client */
			pConsumerThread->nextSendPingTime = currentTime + (time_t)pConsumerThread->pChannel->pingTimeout/3;
		}
	}
	return RSSL_FALSE;
}

void consumerThreadInit(ConsumerThread *pConsumerThread, RsslInt32 consThreadId)
{

	char tmpFilename[sizeof(consPerfConfig.statsFilename) + 8];

	timeRecordQueueInit(&pConsumerThread->latencyRecords);
	timeRecordQueueInit(&pConsumerThread->postLatencyRecords);
	timeRecordQueueInit(&pConsumerThread->genMsgLatencyRecords);

	consumerStatsInit(&pConsumerThread->stats);

	pConsumerThread->pChannel = NULL;

	rsslClearBuffer(&pConsumerThread->cpuId);
	rsslClearBuffer(&pConsumerThread->cpuReactorWorkerId);
	pConsumerThread->latStreamId = 0;
	pConsumerThread->pDictionary = NULL;
	pConsumerThread->dictionaryStateFlags = DICTIONARY_STATE_NONE;

	FD_ZERO(&pConsumerThread->readfds);
	FD_ZERO(&pConsumerThread->exceptfds);
	FD_ZERO(&pConsumerThread->wrtfds);

	pConsumerThread->receivedPing = RSSL_FALSE;
	pConsumerThread->nextSendPingTime = 0;
	pConsumerThread->nextReceivePingTime = 0;
	pConsumerThread->itemListUniqueIndex = 0;
	pConsumerThread->itemListCount = 0;

	latencyRandomArrayIterInit(&pConsumerThread->randArrayIter);

	snprintf(tmpFilename, sizeof(tmpFilename), "%s%d.csv", 
			consPerfConfig.statsFilename, consThreadId);

	/* Open stats file. */
	if (!(pConsumerThread->statsFile = fopen(tmpFilename, "w")))
	{
		printf("Error: Failed to open file '%s'.\n", tmpFilename);
		exit(-1);
	}

	if (consPerfConfig.logLatencyToFile)
	{
		snprintf(tmpFilename, sizeof(tmpFilename), "%s%d.csv", 
				consPerfConfig.latencyLogFilename, consThreadId);

		/* Open latency log file. */
		pConsumerThread->latencyLogFile = fopen(tmpFilename, "w");
		if (!pConsumerThread->latencyLogFile)
		{
			printf("Failed to open latency log file: %s\n", tmpFilename);
			exit(-1);
		}

		fprintf(pConsumerThread->latencyLogFile, "Message type, Send time, Receive time, Latency (usec)\n");
	}

	fprintf(pConsumerThread->statsFile, "UTC, Latency updates, Latency avg (usec), Latency std dev (usec), Latency max (usec), Latency min (usec), Images, Update rate (msg/sec), Posting Latency updates, Posting Latency avg (usec), Posting Latency std dev (usec), Posting Latency max (usec), Posting Latency min (usec), GenMsgs sent, GenMsgs received, GenMsg Latencies sent, GenMsg latencies received, GenMsg Latency avg (usec), GenMsg Latency std dev (usec), GenMsg Latency max (usec), GenMsg Latency min (usec), CPU usage (%%), Memory (MB)\n");
	timeRecordQueueInit(&pConsumerThread->updateDecodeTimeRecords);

	pConsumerThread->threadRsslError.rsslErrorId = RSSL_RET_SUCCESS;
	pConsumerThread->directoryMsgCopyMemory.data = (char*)malloc(16384);
	pConsumerThread->directoryMsgCopyMemory.length = 16384;
	pConsumerThread->directoryMsgCopyMemoryOrig = pConsumerThread->directoryMsgCopyMemory;
	pConsumerThread->pDictionary = (RsslDataDictionary*)malloc(sizeof(RsslDataDictionary));
	rsslClearDataDictionary(pConsumerThread->pDictionary);

	pConsumerThread->pReactor = NULL;
	pConsumerThread->pReactorChannel = NULL;
	pConsumerThread->pDesiredService = NULL;

	pConsumerThread->tunnelMessagingEnabled = RSSL_FALSE;
	pConsumerThread->tunnelStreamServiceName[0] = '\0';

	pConsumerThread->jsonDesiredServiceId = 0;
	pConsumerThread->jsonAllocatorSize = consPerfConfig.jsonAllocatorSize;
	pConsumerThread->jsonAllocatorPtr = malloc(pConsumerThread->jsonAllocatorSize);
}

void consumerThreadCleanup(ConsumerThread *pConsumerThread)
{
	timeRecordQueueCleanup(&pConsumerThread->latencyRecords);
	timeRecordQueueCleanup(&pConsumerThread->postLatencyRecords);
	timeRecordQueueCleanup(&pConsumerThread->genMsgLatencyRecords);
	if (pConsumerThread->statsFile)
		fclose(pConsumerThread->statsFile);
	if (pConsumerThread->latencyLogFile)
		fclose(pConsumerThread->latencyLogFile);

	free(pConsumerThread->directoryMsgCopyMemoryOrig.data);
	if (pConsumerThread->pDictionary)
	{
		rsslDeleteDataDictionary(pConsumerThread->pDictionary);
		free(pConsumerThread->pDictionary);
	}

	if (pConsumerThread->itemRequestList)
	{
		pConsumerThread->itemRequestList += ITEM_STREAM_ID_START;
		free(pConsumerThread->itemRequestList);
	}

	if (pConsumerThread->jsonAllocatorPtr)
	{
		free(pConsumerThread->jsonAllocatorPtr);
	}
}

/* 
 * Processes events about the state of an RsslReactorChannel.
 */
RsslReactorCallbackRet channelEventCallback(RsslReactor *pReactor, RsslReactorChannel *pReactorChannel, RsslReactorChannelEvent *pConnEvent)
{
	ConsumerThread *pConsumerThread = (ConsumerThread *)pReactorChannel->userSpecPtr;
	RsslErrorInfo rsslErrorInfo;
	RsslReactorChannelInfo reactorChannelInfo;
	RsslUInt32 count;
	RsslUInt32 index;

	switch(pConnEvent->channelEventType)
	{
		case RSSL_RC_CET_CHANNEL_UP:
		{
			/* A channel that we have requested via rsslReactorConnect() has come up.  Set our
			 * file descriptor sets so we can be notified to start calling rsslReactorDispatch() for
			 * this channel. This will drive the process of setting up the connection
			 * by exchanging the Login, Directory, and (if not already loaded)Dictionary messages. 
			 * The application will receive the response messages in the appropriate callback
			 * function we specified. */

            pConsumerThread->pReactorChannel = pReactorChannel;
			pConsumerThread->pChannel = pReactorChannel->pRsslChannel;
            printf("Connected ");

            // set the high water mark if configured
            if (consPerfConfig.highWaterMark > 0)
            {
                if (rsslReactorChannelIoctl(pReactorChannel, RSSL_HIGH_WATER_MARK, &consPerfConfig.highWaterMark, &rsslErrorInfo) != RSSL_RET_SUCCESS)
                {
					shutdownThreads = RSSL_TRUE;
                    return RSSL_RC_CRET_FAILURE;
                }
            }

			if (pReactorChannel->reactorChannelType == RSSL_REACTOR_CHANNEL_TYPE_NORMAL)
			{
				/* Set file descriptor. */
				FD_SET(pReactorChannel->socketId, &(pConsumerThread->readfds));
				FD_SET(pReactorChannel->socketId, &(pConsumerThread->exceptfds));
			}
			else
			{
				/* Set file descriptors for warm standby channel. */
				FD_ZERO(&pConsumerThread->readfds);
				FD_ZERO(&pConsumerThread->exceptfds);
				FD_SET(pReactor->eventFd, &pConsumerThread->readfds);
				FD_SET(pReactor->eventFd, &pConsumerThread->exceptfds);

				for (index = 0; index < pReactorChannel->pWarmStandbyChInfo->socketIdCount; index++)
				{
					FD_SET(pReactorChannel->pWarmStandbyChInfo->socketIdList[index], &pConsumerThread->readfds);
					FD_SET(pReactorChannel->pWarmStandbyChInfo->socketIdList[index], &pConsumerThread->exceptfds);
				}
			}

			if (rsslReactorGetChannelInfo(pReactorChannel, &reactorChannelInfo, &rsslErrorInfo) != RSSL_RET_SUCCESS)
			{
				rsslSetErrorInfoLocation(&rsslErrorInfo, __FILE__, __LINE__);
				shutdownThreads = RSSL_TRUE;
				return RSSL_RC_CRET_FAILURE;
			} 

			printf( "Channel "SOCKET_PRINT_TYPE" active. Channel Info:\n"
					"  maxFragmentSize: %u\n"
					"  maxOutputBuffers: %u\n"
					"  guaranteedOutputBuffers: %u\n"
					"  numInputBuffers: %u\n"
					"  pingTimeout: %u\n"
					"  clientToServerPings: %s\n"
					"  serverToClientPings: %s\n"
					"  sysSendBufSize: %u\n"
					"  sysSendBufSize: %u\n"			
					"  compressionType: %s\n"
					"  compressionThreshold: %u\n"			
					"  ComponentInfo: ", 
					pConsumerThread->pChannel->socketId,
					reactorChannelInfo.rsslChannelInfo.maxFragmentSize,
					reactorChannelInfo.rsslChannelInfo.maxOutputBuffers, reactorChannelInfo.rsslChannelInfo.guaranteedOutputBuffers,
					reactorChannelInfo.rsslChannelInfo.numInputBuffers,
					reactorChannelInfo.rsslChannelInfo.pingTimeout,
					reactorChannelInfo.rsslChannelInfo.clientToServerPings == RSSL_TRUE ? "true" : "false",
					reactorChannelInfo.rsslChannelInfo.serverToClientPings == RSSL_TRUE ? "true" : "false",
					reactorChannelInfo.rsslChannelInfo.sysSendBufSize, reactorChannelInfo.rsslChannelInfo.sysRecvBufSize,			
					reactorChannelInfo.rsslChannelInfo.compressionType == RSSL_COMP_ZLIB ? "zlib" : "none",
					reactorChannelInfo.rsslChannelInfo.compressionThreshold			
					);

			if (reactorChannelInfo.rsslChannelInfo.componentInfoCount == 0)
				printf("(No component info)");
			else
				for(count = 0; count < reactorChannelInfo.rsslChannelInfo.componentInfoCount; ++count)
				{
					printf("%.*s", 
							reactorChannelInfo.rsslChannelInfo.componentInfo[count]->componentVersion.length,
							reactorChannelInfo.rsslChannelInfo.componentInfo[count]->componentVersion.data);
					if (count < reactorChannelInfo.rsslChannelInfo.componentInfoCount - 1)
						printf(", ");
				}
			printf ("\n\n");

			return RSSL_RC_CRET_SUCCESS;
		}
		case RSSL_RC_CET_CHANNEL_READY:
		{
			printEstimatedPostMsgSizes(pConsumerThread);
			printEstimatedGenMsgSizes(pConsumerThread);

			if (!pConsumerThread->pDesiredService)
            {
                printf("Requested service '%s' not up. Waiting for service to be up...", consPerfConfig.serviceName);
            }
			return RSSL_RC_CRET_SUCCESS;
		}
		case RSSL_RC_CET_FD_CHANGE:
		{
			if (pReactorChannel->reactorChannelType == RSSL_REACTOR_CHANNEL_TYPE_NORMAL)
			{
				/* The file descriptor representing the RsslReactorChannel has been changed.
				 * Update our file descriptor sets. */
				printf("Fd change: "SOCKET_PRINT_TYPE" to "SOCKET_PRINT_TYPE"\n", pReactorChannel->oldSocketId, pReactorChannel->socketId);
				FD_CLR(pReactorChannel->oldSocketId, &(pConsumerThread->readfds));
				FD_CLR(pReactorChannel->oldSocketId, &(pConsumerThread->exceptfds));
				FD_SET(pReactorChannel->socketId, &(pConsumerThread->readfds));
				FD_SET(pReactorChannel->socketId, &(pConsumerThread->exceptfds));
			}
			else
			{
				/* Set file descriptors for warm standby channel. */
				FD_ZERO(&pConsumerThread->readfds);
				FD_ZERO(&pConsumerThread->exceptfds);
				FD_SET(pReactor->eventFd, &pConsumerThread->readfds);
				FD_SET(pReactor->eventFd, &pConsumerThread->exceptfds);

				for (index = 0; index < pReactorChannel->pWarmStandbyChInfo->socketIdCount; index++)
				{
					FD_SET(pReactorChannel->pWarmStandbyChInfo->socketIdList[index], &pConsumerThread->readfds);
					FD_SET(pReactorChannel->pWarmStandbyChInfo->socketIdList[index], &pConsumerThread->exceptfds);
				}
			}

			return RSSL_RC_CRET_SUCCESS;
		}
		case RSSL_RC_CET_CHANNEL_DOWN:
		{
			/* The channel has failed and has gone down.  Print the error, close the channel, and reconnect later. */

			printf("Connection down: Channel fd="SOCKET_PRINT_TYPE".\n", pReactorChannel->socketId);

			if (pConnEvent->pError)
				printf("	Error text: %s\n\n", pConnEvent->pError->rsslError.text);

			shutdownThreads = RSSL_TRUE;
			return RSSL_RC_CRET_SUCCESS;
		}
		case RSSL_RC_CET_CHANNEL_DOWN_RECONNECTING:
		{
			printf("Connection down, reconnecting.  Channel fd="SOCKET_PRINT_TYPE"\n", pReactorChannel->socketId);

			if (pConnEvent->pError)
				printf("	Error text: %s\n\n", pConnEvent->pError->rsslError.text);

			if (pReactorChannel->socketId != REACTOR_INVALID_SOCKET)
			{
				if (pReactorChannel->reactorChannelType == RSSL_REACTOR_CHANNEL_TYPE_NORMAL)
				{
					FD_CLR(pReactorChannel->socketId, &(pConsumerThread->readfds));
					FD_CLR(pReactorChannel->socketId, &(pConsumerThread->exceptfds));
				}
				else
				{
					FD_ZERO(&pConsumerThread->readfds);
					FD_ZERO(&pConsumerThread->exceptfds);
					FD_SET(pReactor->eventFd, &pConsumerThread->readfds);
					FD_SET(pReactor->eventFd, &pConsumerThread->exceptfds);
				}
			}

			// only allow one connect
			if (pConsumerThread && pConsumerThread->pDesiredService)
            {
                shutdownThreads = RSSL_TRUE;
            }

			return RSSL_RC_CRET_SUCCESS;
		}
		case RSSL_RC_CET_WARNING:
		{
			/* We have received a warning event for this channel. Print the information and continue. */
			printf("Received warning for Channel fd="SOCKET_PRINT_TYPE".\n", pReactorChannel->socketId);
			printf("	Error text: %s\n\n", pConnEvent->pError->rsslError.text);
			return RSSL_RC_CRET_SUCCESS;
		}
		default:
		{
			printf("Unknown connection event!\n");
			shutdownThreads = RSSL_TRUE;
			RSSL_RC_CRET_FAILURE;
		}
	}

	return RSSL_RC_CRET_SUCCESS;
}

/*
 * Processes the information contained in Login responses.
 * Copies the refresh so we can use it to know what features are
 * supported(for example, whether posting is supported
 * by the provider).
 */
RsslReactorCallbackRet loginMsgCallback(RsslReactor *pReactor, RsslReactorChannel *pReactorChannel, RsslRDMLoginMsgEvent *pLoginMsgEvent)
{
	RsslState *pState = 0;
	RsslBuffer tempBuffer = { 1024, (char*)alloca(1024) };
	ConsumerThread *pConsumerThread = (ConsumerThread *)pReactorChannel->userSpecPtr;
	RsslRDMLoginMsg *pLoginMsg = pLoginMsgEvent->pRDMLoginMsg;

	if (!pLoginMsg)
	{
		RsslErrorInfo *pError = pLoginMsgEvent->baseMsgEvent.pErrorInfo;
		printf("loginMsgCallback: %s(%s)\n", pError->rsslError.text, pError->errorLocation);
		shutdownThreads = RSSL_TRUE;
		return RSSL_RC_CRET_FAILURE;
	}

	if (processLoginResp(pConsumerThread, pLoginMsg) < RSSL_RET_SUCCESS)
	{
		shutdownThreads = RSSL_TRUE;
        return RSSL_RC_CRET_FAILURE;
	}

	return RSSL_RC_CRET_SUCCESS;
}

/*
 * Processes information contained in Directory responses.
 * Searches the refresh for the service the application
 * wants to use for this connection and stores the information
 * so we can request items from that service.
 */
RsslReactorCallbackRet directoryMsgCallback(RsslReactor *pReactor, RsslReactorChannel *pReactorChannel, RsslRDMDirectoryMsgEvent *pDirectoryMsgEvent)
{
	char tempData[1024];
	RsslBuffer tempBuffer;
	RsslInt8 groupIdIndex = -1;
	ConsumerThread *pConsumerThread = (ConsumerThread *)pReactorChannel->userSpecPtr;
	RsslRDMDirectoryMsg *pDirectoryMsg = pDirectoryMsgEvent->pRDMDirectoryMsg;

	tempBuffer.data = tempData;
	tempBuffer.length = 1024;

	if (!pDirectoryMsg)
	{
		RsslErrorInfo *pError = pDirectoryMsgEvent->baseMsgEvent.pErrorInfo;
		printf("processDirectoryResponse: %s(%s)\n", pError->rsslError.text, pError->errorLocation);
		shutdownThreads = RSSL_TRUE;
		return RSSL_RC_CRET_FAILURE;
	}

	if (processSourceDirectoryResp(pConsumerThread, pDirectoryMsg) < RSSL_RET_SUCCESS)
	{
		shutdownThreads = RSSL_TRUE;
        return RSSL_RC_CRET_FAILURE;
	}

	return RSSL_RC_CRET_SUCCESS;
}

/*
 * Processes information contained in Dictionary responses.
 * Takes the payload of the messages and caches them
 * using the RSSL utilities for decoding dictionary info.
 */
RsslReactorCallbackRet dictionaryMsgCallback(RsslReactor *pReactor, RsslReactorChannel *pReactorChannel, RsslRDMDictionaryMsgEvent *pDictionaryMsgEvent)
{
	char	errTxt[256];
	RsslBuffer errorText = {255, (char*)errTxt};
	RsslDecodeIterator dIter;
	ConsumerThread *pConsumerThread = (ConsumerThread *)pReactorChannel->userSpecPtr;
	RsslRDMDictionaryMsg *pDictionaryMsg = pDictionaryMsgEvent->pRDMDictionaryMsg;
	RsslMsg *pMsg = pDictionaryMsgEvent->baseMsgEvent.pRsslMsg;

	if (!pDictionaryMsg && !pMsg)
	{
		RsslErrorInfo *pError = pDictionaryMsgEvent->baseMsgEvent.pErrorInfo;
		printf("dictionaryResponseCallback: %s(%s)\n", pError->rsslError.text, pError->errorLocation);
		shutdownThreads = RSSL_TRUE;
		return RSSL_RC_CRET_FAILURE;
	}

	/* clear decode iterator */
	rsslClearDecodeIterator(&dIter);
		
	/* set version info */
	rsslSetDecodeIteratorRWFVersion(&dIter, pConsumerThread->pChannel->majorVersion, pConsumerThread->pChannel->minorVersion);
	if (pMsg)
	{
		rsslSetDecodeIteratorBuffer(&dIter, &pMsg->msgBase.encDataBody);
	}
	else
	{
		rsslSetDecodeIteratorBuffer(&dIter, &pDictionaryMsg->refresh.dataBody);
	}

	if (processDictionaryResp(pConsumerThread, pDictionaryMsg, &dIter) < RSSL_RET_SUCCESS)
	{
		shutdownThreads = RSSL_TRUE;
        return RSSL_RC_CRET_FAILURE;
	}

	return RSSL_RC_CRET_SUCCESS;
}

RsslRet processItemMsg(RsslReactorChannel *pReactorChannel, RsslMsg *pMsg)
{
	RsslRet ret = 0;
	RsslDecodeIterator dIter;
	ConsumerThread *pConsumerThread = (ConsumerThread *)pReactorChannel->userSpecPtr;

	/* clear decode iterator */
	rsslClearDecodeIterator(&dIter);

	/* set version info */
	rsslSetDecodeIteratorRWFVersion(&dIter, pReactorChannel->majorVersion, pReactorChannel->minorVersion);
	rsslSetDecodeIteratorBuffer(&dIter, &pMsg->msgBase.encDataBody);

	if (processDefaultMsgResp(pConsumerThread, pMsg, &dIter) < RSSL_RET_SUCCESS)
	{
		shutdownThreads = RSSL_TRUE;
		return RSSL_RC_CRET_FAILURE;
	}

	return RSSL_RC_CRET_SUCCESS;
}

/*
 * Processes all RSSL messages that aren't processed by 
 * any domain-specific callback functions.  Responses for
 * items requested by the function are handled here. 
 */
RsslReactorCallbackRet defaultMsgCallback(RsslReactor *pReactor, RsslReactorChannel *pReactorChannel, RsslMsgEvent* pMsgEvent)
{
	RsslMsg *pMsg = pMsgEvent->pRsslMsg;
	
	if (!pMsg)
	{
		/* The message is not present because an error occurred while decoding it.  Print 
			* the error and close the channel. If desired, the un-decoded message buffer 
			* is available in pMsgEvent->pRsslMsgBuffer. */
	
		RsslErrorInfo *pError = pMsgEvent->pErrorInfo;
		printf("defaultMsgCallback: %s(%s)\n", pError->rsslError.text, pError->errorLocation);
		shutdownThreads = RSSL_TRUE;
		return RSSL_RC_CRET_FAILURE;
	}

	return processItemMsg(pReactorChannel, pMsg);
}


