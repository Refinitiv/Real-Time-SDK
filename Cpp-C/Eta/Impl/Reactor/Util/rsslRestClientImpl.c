/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2019-2022 Refinitiv. All rights reserved.         --
 *|-----------------------------------------------------------------------------
*/

#include "rtr/rsslRestClientImpl.h"
#include "rtr/rtratomic.h"
#include "rtr/socket.h"
#include "rtr/rsslEventSignal.h"
#include "rtr/rsslHashTable.h"
#include "rtr/rsslThread.h"
#include "rtr/rsslCurlJIT.h"
#include "rtr/ripcutils.h"
#include "rtr/ripcflip.h"
#include "rtr/ripcssljit.h"
#include "rtr/rsslSocketTransportImpl.h"
#include "xmlDump.h"

#include <curl/curl.h>
#include <cjson/cJSON.h>
#include <stdlib.h>
#include <sys/timeb.h>

#ifdef WIN32
#include <winsock2.h>
#else
#include <netinet/in.h>
#endif

#define RSSL_REST_DEFAULT_MAX_HEADER_LENGTH 8192
#define RSSL_REST_PADDING_BUF_SIZE 512
//#define RSSL_REST_CLIENT_VERBOSE  /* uncomment to get debugging information from libcurl */

static rtr_atomic_val	rssl_rest_numInitCalls = 0;
static struct curl_slist *rssl_rest_proxy_header_list = 0;

static RsslError rssl_rest_Error;
static RsslCurlJITFuncs* rssl_rest_CurlJITFuncs = 0;

RsslBuffer rssl_rest_service_discovery_token_scope = { 28, "trapi.streaming.pricing.read" };
RsslBuffer rssl_rest_authorization_text = { 13, "Authorization" };
RsslBuffer rssl_rest_accept_text = { 6 ,"Accept" };
RsslBuffer rssl_rest_application_json_text = { 16, "application/json" };
RsslBuffer rssl_rest_transport_type_tcp_text = { 13, "transport=tcp" };
RsslBuffer rssl_rest_transport_type_websocket_text = { 19, "transport=websocket" };
RsslBuffer rssl_rest_dataformat_type_rwf_text = { 14, "dataformat=rwf" };
RsslBuffer rssl_rest_dataformat_type_tr_json2_text = { 20, "dataformat=tr_json2" };
RsslBuffer rssl_rest_grant_type_refresh_token_text = { 24, "grant_type=refresh_token" };
RsslBuffer rssl_rest_grant_type_password_text = { 19, "grant_type=password" };
RsslBuffer rssl_rest_grant_type_client_credential_text = {29 , "grant_type=client_credentials" };
RsslBuffer rssl_rest_username_text = { 10, "&username=" };
RsslBuffer rssl_rest_password_text = { 10, "&password=" };
RsslBuffer rssl_rest_new_password_text = { 13, "&newPassword=" };
RsslBuffer rssl_rest_client_id_text = { 11, "&client_id=" };
RsslBuffer rssl_rest_client_secret_text = { 15, "&client_secret=" };
RsslBuffer rssl_rest_refresh_token_text = { 15, "&refresh_token=" };
RsslBuffer rssl_rest_scope_text = { 7, "&scope=" };
RsslBuffer rssl_rest_take_exclusive_sign_on_false_text = { 33, "&takeExclusiveSignOnControl=false" };
RsslBuffer rssl_rest_take_exclusive_sign_on_true_text = { 32, "&takeExclusiveSignOnControl=true" };
RsslBuffer rssl_rest_content_type_text = { 12, "Content-Type" };
RsslBuffer rssl_rest_application_form_urlencoded_text = { 33, "application/x-www-form-urlencoded" };
RsslBuffer rssl_rest_content_length_text = { 14, "Content-Length" };
RsslBuffer rssl_rest_content_encoding_text = { 16, "Content-Encoding" };
RsslBuffer rssl_rest_accept_encoding_text = { 15, "Accept-Encoding" };
RsslBuffer rssl_rest_location_header_text = { 8, "Location" };
RsslBuffer rssl_rest_user_agent_text = { 10, "User-Agent" };
RsslBuffer rssl_rest_user_agent_rtsdk_text = { 5, "RTSDK" };
RsslBuffer rssl_rest_client_assertion = { 18, "&client_assertion=" };



RsslBuffer rssl_rest_token_url_v1 = { 46, "https://api.refinitiv.com/auth/oauth2/v1/token" };

RsslBuffer rssl_rest_token_url_v2 = { 46, "https://api.refinitiv.com/auth/oauth2/v2/token" };

static const char * const tokenkeysV1[] =
{
	"access_token",
	"refresh_token",
	"expires_in",
	"scope",
	"token_type"
};

static const char* const tokenkeysV2[] =
{
	"access_token",
	"expires_in",
	"token_type"
};

typedef struct {
	RsslQueueLink	queueLink;	// This is usd to keep in the buffer pool
	char*			pStartPos;
	char*			pCurrentPos;
	RsslUInt32		remainingLength;
	RsslUInt32		bufferLength;
	RsslBool		isOwnedByRestClient; // whether the memory is allocated by RsslRestClient
} RsslRestBufferImpl;

typedef struct {
	RsslRestResponse*	pRestResponse;
	RsslUInt32			headerLength;
	RsslRestBufferImpl	restBufferImpl;
}RsslRestResponseImpl;

typedef struct {
	RsslRestClient			rsslRestClient;
	CURLM*					pCURLM;
	RsslRestResponseEvent	rsslRestResponseEvent;
	RsslEventSignal			rsslEventSignal;
	RsslHashTable 			restHandleImplTable;	/* hash table for handling request status */
	RsslBool				dynamicBufferSize;
	RsslUInt32				numberOfBuffers;
	RsslMutex				headersPoolMutex;
	RsslQueue				headerBufferPool;
} RsslRestClientImpl;

typedef struct {
	RsslRestHandle				rsslRestHandle;
	RsslHashLink				hashLink;		/* link used for putting in a hashtable */
	RsslRestResponseImpl		rsslRestResponseImpl;
	RsslRestResponseCallback*	pRsslRestReponseCallback;
	RsslRestErrorCallback*		pRsslRestErrorCallback;
	CURL*						handle;
	RsslRestClientImpl*			pRsslRestClientImpl;
	void*						userPtr;
	RsslError					rsslError;
	struct curl_slist*			pCurlHeaderList;
	RsslBuffer					httpDataBodyForPost;
	RsslUInt32					contentLength;
	RsslBool					hasContentEncoding;
	RsslUInt32					numOfHeaders;
	RsslRestBufferImpl*			pRsslRestBufferImpl;
	RsslBuffer*					pUserMemory;
} RsslRestHandleImpl;

int rsslRestGetServByName(char *serv_name)
{
	struct servent *serv_port;	 /* Service port */

#if defined (x86_Linux_4X) || (x86_Linux_3X) || (x86_Linux_2X)
	struct servent serv_result;
	char tbuf[1024];
#endif

	if (serv_name != (char *)0)
	{
		int port;

		/* Check for port number definition first */
		if (((port = atoi(serv_name)) > 0) && (port <= 65535))
		{
			u16 prt = port;
			return host2net_u16(prt);
		}

#if defined (x86_Linux_4X) || defined (x86_Linux_3X) || defined (x86_Linux_2X)
		getservbyname_r(serv_name, "tcp", &serv_result, tbuf, 1024, &serv_port);
#else
		serv_port = getservbyname(serv_name, "tcp");
#endif
	}
	else
		serv_port = NULL;

	if (serv_port != NULL)
		return (serv_port->s_port);

	return(-1);
}

RsslUInt32 RsslRestCurlHandleSumFunction(void* curlHandle)
{
	return (RsslUInt32)((RsslUInt64)curlHandle);
}

RsslBool RsslRestCurlHandleCompareFunction(void* key1, void* key2)
{
	return (key1 == key2);
}

void _rsslClearRestBufferImpl(RsslRestBufferImpl* rsslRestBufferImpl)
{
	memset(rsslRestBufferImpl, 0, sizeof(RsslRestBufferImpl));
	rsslInitQueueLink(&rsslRestBufferImpl->queueLink);
}

void _rsslClearRestResponseImpl(RsslRestResponseImpl* rsslRestResponseImpl)
{
	rsslRestResponseImpl->pRestResponse = 0;
	rsslRestResponseImpl->headerLength = 0;
	_rsslClearRestBufferImpl(&rsslRestResponseImpl->restBufferImpl);
}

void _rsslRestClearError(RsslError* rsslError)
{
	rsslError->channel = 0;
	rsslError->rsslErrorId = RSSL_RET_SUCCESS;
	rsslError->sysError = 0;
	memset(rsslError->text, 0, MAX_RSSL_ERROR_TEXT+1);
}

void _rsslClearRestHandleImpl(RsslRestHandleImpl* rsslRestHandleImpl)
{
	rsslRestHandleImpl->contentLength = 0;
	rsslRestHandleImpl->handle = 0;
	rsslRestHandleImpl->numOfHeaders = 0;
	rsslRestHandleImpl->pRsslRestReponseCallback = 0;
	rsslRestHandleImpl->pRsslRestErrorCallback = 0;
	rsslRestHandleImpl->pRsslRestClientImpl = 0;
	rsslRestHandleImpl->pRsslRestBufferImpl = 0;
	_rsslRestClearError(&rsslRestHandleImpl->rsslError);
	rsslRestHandleImpl->pCurlHeaderList = 0;
	rsslRestHandleImpl->pUserMemory = 0;
	rsslRestHandleImpl->hasContentEncoding = RSSL_FALSE;
	rsslClearBuffer(&rsslRestHandleImpl->httpDataBodyForPost);
	rsslClearRestHandle(&rsslRestHandleImpl->rsslRestHandle);
	_rsslClearRestResponseImpl(&rsslRestHandleImpl->rsslRestResponseImpl);
}

void _rsslClearRestClientImpl(RsslRestClientImpl* restClientImpl)
{
	restClientImpl->dynamicBufferSize = RSSL_FALSE;
	restClientImpl->numberOfBuffers = 0;
	restClientImpl->pCURLM = 0;
	restClientImpl->rsslRestClient.userSpecPtr = 0;
	rsslClearEventSignal(&restClientImpl->rsslEventSignal);
	rsslInitQueue(&restClientImpl->headerBufferPool);
}

void _rsslRestSetToRsslRestBufferImpl(RsslBuffer* memoryBuffer, RsslRestBufferImpl* restBufferImpl)
{
	restBufferImpl->pStartPos = memoryBuffer->data;
	restBufferImpl->bufferLength = memoryBuffer->length;
	restBufferImpl->pCurrentPos = memoryBuffer->data;
	restBufferImpl->remainingLength = memoryBuffer->length;
	memset(restBufferImpl->pStartPos, 0, restBufferImpl->bufferLength);
}

RsslRet _rsslRestGetBuffer(RsslBuffer* destBuffer, RsslUInt32 requiredSize, RsslRestBufferImpl* restBufferImpl)
{
	if (requiredSize > restBufferImpl->remainingLength)
	{
		return RSSL_RET_BUFFER_TOO_SMALL;
	}

	destBuffer->data = restBufferImpl->pCurrentPos;
	restBufferImpl->pCurrentPos += requiredSize;
	restBufferImpl->remainingLength -= requiredSize;

	return RSSL_RET_SUCCESS;
}

RsslRestBufferImpl* _rsslRestCreateBufferImpl(size_t bufferSize)
{
	RsslRestBufferImpl* pRestBufferImpl = (RsslRestBufferImpl*)malloc(sizeof(RsslRestBufferImpl));

	if (pRestBufferImpl)
	{
		RsslBuffer tempBuffer;

		_rsslClearRestBufferImpl(pRestBufferImpl);

		tempBuffer.length = (RsslUInt32)bufferSize;
		tempBuffer.data = (char*)malloc(bufferSize);

		if (tempBuffer.data)
		{
			pRestBufferImpl->isOwnedByRestClient = RSSL_TRUE;
			_rsslRestSetToRsslRestBufferImpl(&tempBuffer, pRestBufferImpl);
		}
		else
		{
			free(pRestBufferImpl);
			pRestBufferImpl = 0;
		}
	}

	return pRestBufferImpl;
}

void _rsslRestReleaseBufferImpl(RsslRestBufferImpl* pRestBufferImpl)
{
	if (pRestBufferImpl)
	{
		free(pRestBufferImpl->pStartPos);
		free(pRestBufferImpl);
	}
}

void _rsslRestReleaseBufferPool(RsslRestClientImpl* rsslRestClientImpl)
{
	RsslQueueLink *pLink;
	RsslRestBufferImpl* pRestBufferImpl;

	(void)RSSL_MUTEX_LOCK(&rsslRestClientImpl->headersPoolMutex);

	while ((pLink = rsslQueueRemoveLastLink(&rsslRestClientImpl->headerBufferPool)))
	{
		pRestBufferImpl = RSSL_QUEUE_LINK_TO_OBJECT(RsslRestBufferImpl, queueLink, pLink);

		_rsslRestReleaseBufferImpl(pRestBufferImpl);
	}

	(void)RSSL_MUTEX_UNLOCK(&rsslRestClientImpl->headersPoolMutex);
}

RsslRestBufferImpl* _getRsslRestBufferImplFromPool(RsslRestClientImpl* rsslRestClientImpl)
{
	RsslRestBufferImpl* pRestBufferImpl = 0;
	RsslQueueLink *pLink;

	(void)RSSL_MUTEX_LOCK(&rsslRestClientImpl->headersPoolMutex);

	pLink = rsslQueueRemoveLastLink(&rsslRestClientImpl->headerBufferPool);

	if (pLink)
	{
		pRestBufferImpl = RSSL_QUEUE_LINK_TO_OBJECT(RsslRestBufferImpl, queueLink, pLink);
	}
	else
	{
		pRestBufferImpl = _rsslRestCreateBufferImpl(RSSL_REST_DEFAULT_MAX_HEADER_LENGTH);
	}

	(void)RSSL_MUTEX_UNLOCK(&rsslRestClientImpl->headersPoolMutex);

	return pRestBufferImpl;
}

void _returnRsslRestBufferImplToPool(RsslRestClientImpl* rsslRestClientImpl, RsslRestBufferImpl* rsslRestBufferImpl)
{
	if (!rsslRestBufferImpl)
		return;

	(void)RSSL_MUTEX_LOCK(&rsslRestClientImpl->headersPoolMutex);

	if ((rsslRestClientImpl->headerBufferPool.count + 1) <= rsslRestClientImpl->numberOfBuffers)
	{
		// Reset RsslRestBufferImpl to original state
		rsslRestBufferImpl->pCurrentPos = rsslRestBufferImpl->pStartPos;
		rsslRestBufferImpl->remainingLength = rsslRestBufferImpl->bufferLength;
		rsslInitQueueLink(&rsslRestBufferImpl->queueLink);

		rsslQueueAddLinkToBack(&rsslRestClientImpl->headerBufferPool, &rsslRestBufferImpl->queueLink);
	}
	else
	{
		_rsslRestReleaseBufferImpl(rsslRestBufferImpl);
	}

	(void)RSSL_MUTEX_UNLOCK(&rsslRestClientImpl->headersPoolMutex);
}

struct curl_slist * _rsslRestExtractHeaderInfo(CURL* curl, RsslRestRequestArgs* rsslRestRequestArgs, RsslError* pError)
{
	RsslQueueLink     *pLink;
	RsslRestHeader    *rsslRestHeader;
	CURLcode          curlCode;
	size_t			  keyLen;
	size_t			  valueLen;
	size_t			  headerLen;
	char*             header;
	struct curl_slist *pHeaderList = 0;
	RsslBool          specifiedAcceptEncoding = RSSL_FALSE;
	
	RSSL_QUEUE_FOR_EACH_LINK(&rsslRestRequestArgs->httpHeaders, pLink)
	{
		rsslRestHeader = RSSL_QUEUE_LINK_TO_OBJECT(RsslRestHeader, queueLink, pLink);
		keyLen = strlen((const char*)(rsslRestHeader->key.data));
		valueLen = strlen((const char*)(rsslRestHeader->value.data));
		// String is key + ": " + value + "\0"
		headerLen = keyLen + valueLen + 2 + 1;

		header = (char*)malloc(headerLen);

		if (header == 0)
		{
			_rsslRestClearError(pError);
			pError->rsslErrorId = RSSL_RET_FAILURE;
			snprintf(pError->text, MAX_RSSL_ERROR_TEXT,
				"<%s:%d> Error: _rsslRestExtractHeaderInfo() failed with text: failed to allocate memory.", __FILE__, __LINE__);
			return pHeaderList;
		}

		memset((void*)header, 0, headerLen);

		strncpy(header, rsslRestHeader->key.data, keyLen);
		strncat(header, ": ", headerLen - keyLen);
		strncat(header, rsslRestHeader->value.data, headerLen - keyLen - 2);

		if (strncmp(rsslRestHeader->key.data, rssl_rest_accept_encoding_text.data, rssl_rest_accept_encoding_text.length) == 0)
			specifiedAcceptEncoding = RSSL_TRUE;

		pHeaderList = (*(rssl_rest_CurlJITFuncs->curl_slist_append))(pHeaderList, (const char*)header);
		free(header);
	}

	// Set the default accept if not specified by users
	if (specifiedAcceptEncoding == RSSL_FALSE)
	{
		(*(rssl_rest_CurlJITFuncs->curl_easy_setopt))(curl, CURLOPT_ACCEPT_ENCODING, "gzip, deflate");
	}

	/* Set custom headers to libcurl */
	curlCode = (*(rssl_rest_CurlJITFuncs->curl_easy_setopt))(curl, CURLOPT_HTTPHEADER, pHeaderList);

	if (curlCode != CURLE_OK)
	{
		_rsslRestClearError(pError);
		pError->rsslErrorId = RSSL_RET_FAILURE;
		snprintf(pError->text, MAX_RSSL_ERROR_TEXT,
			"<%s:%d> Error: _rsslRestExtractHeaderInfo() failed with text: %s", __FILE__, __LINE__,
			(*(rssl_rest_CurlJITFuncs->curl_easy_strerror))(curlCode));

		return pHeaderList;
	}

	return pHeaderList;
}

void notifyResponseCallback(RsslRestHandleImpl* handleImpl)
{
	RsslRestResponseImpl* restResponseImpl = (RsslRestResponseImpl*)(&handleImpl->rsslRestResponseImpl);
	RsslRestResponseEvent rsslRestResponseEvent;
	RsslRestResponse*     restResponse = restResponseImpl->pRestResponse;

	rsslClearRestResponseEvent(&rsslRestResponseEvent);
	rsslRestResponseEvent.handle = &handleImpl->rsslRestHandle;
	rsslRestResponseEvent.closure = handleImpl->userPtr;
	rsslRestResponseEvent.userMemory = handleImpl->pUserMemory;

	// Notify only when there is no error
	if ( (handleImpl->pRsslRestReponseCallback) && (handleImpl->rsslError.rsslErrorId == RSSL_RET_SUCCESS) )
	{
		(*handleImpl->pRsslRestReponseCallback)(restResponse, &rsslRestResponseEvent);
	}
}

size_t rssl_rest_write_databody_callback(char *ptr, size_t size, size_t nmemb, void *userdata)
{
	RsslRestHandleImpl*   handleImpl = (RsslRestHandleImpl*)userdata;
	RsslRestResponseImpl* restResponseImpl = (RsslRestResponseImpl*)(&handleImpl->rsslRestResponseImpl);
	RsslRestBufferImpl*   restBufferImpl = &restResponseImpl->restBufferImpl;
	RsslRestResponse*     restResponse = restResponseImpl->pRestResponse;
	RsslRestResponseEvent rsslRestResponseEvent;
	RsslUInt32			  prevDataBodyLength = 0;
	RsslBuffer			  writeDataBody;
	RsslBuffer*			  pDataBodyTemp = 0;
	size_t retValue = size * nmemb;

	// Do not write data body portion if there is an error when writing the header portion.
	if (handleImpl->rsslError.rsslErrorId != RSSL_RET_SUCCESS)
		return retValue;

	rsslClearRestResponseEvent(&rsslRestResponseEvent);
	rsslRestResponseEvent.handle = &handleImpl->rsslRestHandle;
	rsslRestResponseEvent.closure = handleImpl->userPtr;
	rsslRestResponseEvent.userMemory = handleImpl->pUserMemory;

	rsslClearBuffer(&writeDataBody);

	if (restResponse->dataBody.length == 0)
	{
		prevDataBodyLength = 0;
		restResponse->dataBody.length = (RsslUInt32)retValue;
		pDataBodyTemp = &restResponse->dataBody;
	}
	else
	{
		prevDataBodyLength = restResponse->dataBody.length;
		restResponse->dataBody.length += (RsslUInt32)retValue;
		writeDataBody.length = (RsslUInt32)retValue;
		pDataBodyTemp = &writeDataBody;
	}

	if (handleImpl->pRsslRestClientImpl->dynamicBufferSize && (restResponse->dataBody.length > restBufferImpl->remainingLength) )
	{
		RsslBuffer rsslBuffer;
		rsslBuffer.length = (restResponseImpl->headerLength + restResponse->dataBody.length)
			+ RSSL_REST_PADDING_BUF_SIZE;

		rsslBuffer.data = (char*)malloc(rsslBuffer.length);

		if (rsslBuffer.data == 0)
		{
			_rsslRestClearError(&handleImpl->rsslError);
			handleImpl->rsslError.rsslErrorId = RSSL_RET_FAILURE;
			snprintf(handleImpl->rsslError.text, MAX_RSSL_ERROR_TEXT,
				"<%s:%d> Error: rssl_rest_write_databody_callback() failed to allocate memory to write data body.",
				__FILE__, __LINE__);

			if (restBufferImpl->isOwnedByRestClient)
				free(restBufferImpl->pStartPos);

			if (handleImpl->pRsslRestErrorCallback)
				(*handleImpl->pRsslRestErrorCallback)(&handleImpl->rsslError, &rsslRestResponseEvent);

			return retValue;
		}
		else
		{
			RsslRestHeader* pOldRestHeader;
			RsslRestHeader* pNewRestHeader;
			RsslQueue tempQueue;
			RsslQueueLink* pLink;
			char *oldOrigin = restBufferImpl->isOwnedByRestClient ? restBufferImpl->pStartPos : 0;

			restBufferImpl->isOwnedByRestClient = RSSL_TRUE;
			_rsslRestSetToRsslRestBufferImpl(&rsslBuffer, restBufferImpl);
			rsslInitQueue(&tempQueue);

			/* Replaces the old headers with the new ones from the new memory location */
			while ((pLink = rsslQueueRemoveLastLink(&restResponse->headers)))
			{
				pOldRestHeader = RSSL_QUEUE_LINK_TO_OBJECT(RsslRestHeader, queueLink, pLink);
				if (pOldRestHeader)
				{
					rsslBuffer.length = sizeof(RsslRestHeader);
					_rsslRestGetBuffer(&rsslBuffer, rsslBuffer.length, restBufferImpl);
					pNewRestHeader = (RsslRestHeader*)rsslBuffer.data;
					rsslClearRestHeader(pNewRestHeader);

					/* Copies key value */
					pNewRestHeader->key.length = pOldRestHeader->key.length;
					_rsslRestGetBuffer(&pNewRestHeader->key, pNewRestHeader->key.length + 1, restBufferImpl);

					pNewRestHeader->key.data[pNewRestHeader->key.length] = '\0';
					strncpy(pNewRestHeader->key.data, pOldRestHeader->key.data, pNewRestHeader->key.length);

					/* Copies header value */
					pNewRestHeader->value.length = pOldRestHeader->value.length;
					_rsslRestGetBuffer(&pNewRestHeader->value, pNewRestHeader->value.length + 1, restBufferImpl);
			
					pNewRestHeader->value.data[pNewRestHeader->value.length] = '\0';
					strncpy(pNewRestHeader->value.data, pOldRestHeader->value.data, pNewRestHeader->value.length);

					rsslQueueAddLinkToBack(&tempQueue, &pNewRestHeader->queueLink);
				}
			}

			while ((pLink = rsslQueueRemoveLastLink(&tempQueue)))
			{
				pNewRestHeader = RSSL_QUEUE_LINK_TO_OBJECT(RsslRestHeader, queueLink, pLink);
				if (pNewRestHeader)
				{
					rsslQueueAddLinkToBack(&restResponse->headers, &pNewRestHeader->queueLink);
				}
			}

			// Copy the previous data body portion if any
			if (prevDataBodyLength != 0)
			{
				RsslBuffer dataBodyTemp;
				dataBodyTemp.length = prevDataBodyLength;
				_rsslRestGetBuffer(&dataBodyTemp, dataBodyTemp.length, restBufferImpl);
				strncpy(dataBodyTemp.data, restResponse->dataBody.data, prevDataBodyLength);

				// Assign databody portion to new memory location
				restResponse->dataBody.data = dataBodyTemp.data;
				restResponse->dataBody.length = dataBodyTemp.length;
			}

			if (oldOrigin) free(oldOrigin); // free the old memory
		}
	}

	if (_rsslRestGetBuffer(pDataBodyTemp, pDataBodyTemp->length, restBufferImpl) == RSSL_RET_BUFFER_TOO_SMALL)
	{
		_rsslRestClearError(&handleImpl->rsslError);
		handleImpl->rsslError.rsslErrorId = RSSL_RET_BUFFER_TOO_SMALL;
		snprintf(handleImpl->rsslError.text, MAX_RSSL_ERROR_TEXT,
			"<%s:%d> Error: rssl_rest_write_databody_callback() failed because the memory buffer size is not enough to write data body.",
			__FILE__, __LINE__);

		if ( handleImpl->pRsslRestErrorCallback )
			(*handleImpl->pRsslRestErrorCallback)(&handleImpl->rsslError, &rsslRestResponseEvent);

		return retValue;
	}

	strncpy(pDataBodyTemp->data, ptr, pDataBodyTemp->length);
	restResponse->isMemReallocated = restBufferImpl->isOwnedByRestClient;
	
	if (restResponse->isMemReallocated)
	{
		restResponse->reallocatedMem.data = restBufferImpl->pStartPos;
		restResponse->reallocatedMem.length = restBufferImpl->bufferLength;
	}

	return retValue;
}

RsslRet parseRsslRestHeaders(RsslRestHandleImpl* handleImpl, RsslRestResponse* restResponse, char* ptr, RsslRestBufferImpl* restBufferImpl)
{
	RsslBuffer rsslBuffer;
	RsslRestHeader* rsslRestHeader;
	size_t size;
	size_t pos;
	char* value;
	long httpCode;

	value = strtok(ptr, "\r\n");
	do
	{
		size = strlen(value);
		if (restResponse->protocolVersion.length == 0)
		{
			for (pos = 0; pos < size; pos++)
			{
				if (value[pos] == ' ') break;
			}

			restResponse->protocolVersion.length = (RsslUInt32)(pos);
			if (_rsslRestGetBuffer(&restResponse->protocolVersion, restResponse->protocolVersion.length + 1, restBufferImpl) == RSSL_RET_BUFFER_TOO_SMALL)
			{
				return RSSL_RET_FAILURE;
			}

			restResponse->protocolVersion.data[restResponse->protocolVersion.length] = '\0';
			strncpy(restResponse->protocolVersion.data, value, restResponse->protocolVersion.length);

			if ((*(rssl_rest_CurlJITFuncs->curl_easy_getinfo))(handleImpl->handle, CURLINFO_RESPONSE_CODE, &httpCode) == CURLE_OK)
			{
				restResponse->statusCode = httpCode;
			}
		}
		else
		{
			for (pos = 0; pos < size; pos++)
			{
				if (value[pos] == ':') break;
			}

			if (pos + 1 != size) // Ensure that the colon is found.
			{
				rsslBuffer.length = sizeof(RsslRestHeader);
				if (_rsslRestGetBuffer(&rsslBuffer, rsslBuffer.length, restBufferImpl) == RSSL_RET_BUFFER_TOO_SMALL)
				{
					return RSSL_RET_FAILURE;
				}
				rsslRestHeader = (RsslRestHeader*)rsslBuffer.data;
				rsslClearRestHeader(rsslRestHeader);

				/* Extract header key */
				rsslRestHeader->key.length = (RsslUInt32)pos;
				if (_rsslRestGetBuffer(&rsslRestHeader->key, rsslRestHeader->key.length + 1, restBufferImpl) == RSSL_RET_BUFFER_TOO_SMALL)
				{
					return RSSL_RET_FAILURE;
				}

				rsslRestHeader->key.data[rsslRestHeader->key.length] = '\0';
				strncpy(rsslRestHeader->key.data, value, rsslRestHeader->key.length);

				/* Extract header value */
				rsslRestHeader->value.length = (RsslUInt32)(size - (pos + 1));
				if (_rsslRestGetBuffer(&rsslRestHeader->value, rsslRestHeader->value.length + 1, restBufferImpl) == RSSL_RET_BUFFER_TOO_SMALL)
				{
					return RSSL_RET_FAILURE;
				}

				rsslRestHeader->value.data[rsslRestHeader->value.length] = '\0';
				strncpy(rsslRestHeader->value.data, value + pos + 1, rsslRestHeader->value.length);

				rsslQueueAddLinkToBack(&restResponse->headers, &rsslRestHeader->queueLink);
			}
		}

		value = strtok(0, "\r\n");

	} while (value != 0);

	return RSSL_RET_SUCCESS;
}

size_t rssl_rest_write_header_callback_with_dynamic_size(char *ptr, size_t size, size_t nmemb, void *userdata)
{
	RsslRestHandleImpl*   handleImpl = (RsslRestHandleImpl*)userdata;
	RsslRestResponseImpl* restResponseImpl = (RsslRestResponseImpl*)(&handleImpl->rsslRestResponseImpl);
	RsslRestBufferImpl*   restBufferImpl = &restResponseImpl->restBufferImpl;
	RsslRestBufferImpl*   restBufferImplTemp = handleImpl->pRsslRestBufferImpl; // temporary holds all headers
	RsslRestResponse*     restResponse = restResponseImpl->pRestResponse;
	RsslRestResponseEvent rsslRestResponseEvent;
	size_t totalSize = size * nmemb;
	RsslBuffer rsslBuffer;
	RsslRet ret;
	char* value;

	rsslBuffer.length = (RsslUInt32)totalSize;

	do
	{
		ret = _rsslRestGetBuffer(&rsslBuffer, rsslBuffer.length, restBufferImplTemp);

		if (ret == RSSL_RET_BUFFER_TOO_SMALL)
		{
			RsslUInt32 dataLength;
			RsslBuffer newRsslBuffer;

			newRsslBuffer.length = restBufferImplTemp->bufferLength * 2;
			newRsslBuffer.data = (char*)malloc(newRsslBuffer.length);

			if (newRsslBuffer.data == 0)
				goto Fail;

			dataLength = (restBufferImplTemp->bufferLength - restBufferImplTemp->remainingLength);

			strncpy(newRsslBuffer.data, restBufferImplTemp->pStartPos, dataLength);
			free(restBufferImplTemp->pStartPos);

			restBufferImplTemp->pStartPos = newRsslBuffer.data;
			restBufferImplTemp->bufferLength = newRsslBuffer.length;
			restBufferImplTemp->pCurrentPos = restBufferImplTemp->pStartPos + dataLength;
			restBufferImplTemp->remainingLength = restBufferImplTemp->bufferLength - dataLength;
		}

	} while (ret == RSSL_RET_BUFFER_TOO_SMALL);

	// Checks to see whether the library receives the entire headers
	if (strncmp(ptr, "\r\n", 2) != 0)
	{
		++handleImpl->numOfHeaders;
		strncpy(rsslBuffer.data, ptr, rsslBuffer.length);

		if (strncmp(rsslBuffer.data, rssl_rest_content_length_text.data, rssl_rest_content_length_text.length) == 0)
		{
			value = strtok(rsslBuffer.data, ":");
			value = strtok(0, ":");
			value = strtok(value, "\r\n");

			handleImpl->contentLength = atoi(value);
			strncpy(rsslBuffer.data, ptr, rsslBuffer.length);
		}
		else if (strncmp(rsslBuffer.data, rssl_rest_content_encoding_text.data, rssl_rest_content_encoding_text.length) == 0)
		{
			handleImpl->hasContentEncoding = RSSL_TRUE;
		}
	}
	else
	{
		restResponseImpl->headerLength = (restBufferImplTemp->bufferLength - restBufferImplTemp->remainingLength) + (handleImpl->numOfHeaders * sizeof(RsslRestHeader));

		// Reset the content lenght as it represents the compressed data length
		if (handleImpl->hasContentEncoding)
		{
			handleImpl->contentLength = 0;
		}

		// Reallocated the memory to ensure that the library can store the entire response.
		if (restBufferImpl->bufferLength < (restResponseImpl->headerLength + handleImpl->contentLength) )
		{
			if(restBufferImpl->isOwnedByRestClient)
				free(restBufferImpl->pStartPos);

			if (handleImpl->contentLength != 0)
			{
				rsslBuffer.length = restResponseImpl->headerLength + handleImpl->contentLength + RSSL_REST_PADDING_BUF_SIZE;
			}
			else
			{
				rsslBuffer.length = (restBufferImpl->bufferLength * 2) > (restResponseImpl->headerLength * 5) ? (restBufferImpl->bufferLength * 2) : (restResponseImpl->headerLength * 5);
				rsslBuffer.length += RSSL_REST_PADDING_BUF_SIZE;
			}
			
			rsslBuffer.data = (char*)malloc(rsslBuffer.length);

			if (rsslBuffer.data == 0)
			{
				goto Fail;
			}

			restBufferImpl->isOwnedByRestClient = RSSL_TRUE;
			_rsslRestSetToRsslRestBufferImpl(&rsslBuffer, restBufferImpl);
		}

		if (parseRsslRestHeaders(handleImpl, restResponse, restBufferImplTemp->pStartPos, restBufferImpl) != RSSL_RET_SUCCESS)
		{
			goto Fail;
		}
	}

	return totalSize;

Fail:

	rsslClearRestResponseEvent(&rsslRestResponseEvent);
	rsslRestResponseEvent.handle = &handleImpl->rsslRestHandle;
	rsslRestResponseEvent.closure = handleImpl->userPtr;
	rsslRestResponseEvent.userMemory = handleImpl->pUserMemory;
	_rsslRestClearError(&handleImpl->rsslError);
	handleImpl->rsslError.rsslErrorId = RSSL_RET_FAILURE;
	snprintf(handleImpl->rsslError.text, MAX_RSSL_ERROR_TEXT,
		"<%s:%d> Error: rssl_rest_write_header_callback_with_dynamic_size() failed to allocate memory to populate headers.",
		__FILE__, __LINE__);

	if (handleImpl->pRsslRestErrorCallback)
		(*handleImpl->pRsslRestErrorCallback)(&handleImpl->rsslError, &rsslRestResponseEvent);

	return totalSize;
}

size_t rssl_rest_write_header_callback(char *ptr, size_t size, size_t nmemb, void *userdata)
{
	RsslRestHandleImpl*   handleImpl = (RsslRestHandleImpl*)userdata;
	RsslRestResponseImpl* restResponseImpl = (RsslRestResponseImpl*)(&handleImpl->rsslRestResponseImpl);
	RsslRestBufferImpl*   restBufferImpl = &restResponseImpl->restBufferImpl;
	RsslRestResponse*     restResponse = restResponseImpl->pRestResponse;
	RsslRestResponseEvent rsslRestResponseEvent;
	size_t totalSize = size * nmemb;
	char* value;

	if ( (strncmp(ptr, "\r\n", 2) == 0) || ( handleImpl->rsslError.rsslErrorId != RSSL_RET_SUCCESS ) )
	{
		restResponseImpl->headerLength = restBufferImpl->bufferLength - restBufferImpl->remainingLength;
		return totalSize;
	}

	value = strtok(ptr, "\r\n");
	if (parseRsslRestHeaders(handleImpl, restResponse, value, restBufferImpl) != RSSL_RET_SUCCESS)
	{
		goto Fail;
	}

	return totalSize;

Fail:
	rsslClearRestResponseEvent(&rsslRestResponseEvent);
	rsslRestResponseEvent.handle = &handleImpl->rsslRestHandle;
	rsslRestResponseEvent.closure = handleImpl->userPtr;
	rsslRestResponseEvent.userMemory = handleImpl->pUserMemory;
	_rsslRestClearError(&handleImpl->rsslError);
	handleImpl->rsslError.rsslErrorId = RSSL_RET_BUFFER_TOO_SMALL;
	snprintf(handleImpl->rsslError.text, MAX_RSSL_ERROR_TEXT,
		"<%s:%d> Error: rssl_rest_write_header_callback() failed because the memory buffer size is not enough to populate headers.",
		__FILE__, __LINE__);

	if (handleImpl->pRsslRestErrorCallback)
		(*handleImpl->pRsslRestErrorCallback)(&handleImpl->rsslError, &rsslRestResponseEvent);

	return totalSize;
}

size_t _rssl_rest_read_databody_callback(char *ptr, size_t size, size_t nmemb, void *userp) {
	RsslBuffer *data = (RsslBuffer*)userp;

	size_t readbuffersize = size * nmemb;
	if (data->length < readbuffersize) {
		readbuffersize = data->length;
	}
	memcpy(ptr, data->data, readbuffersize);
	data->data += readbuffersize;
	data->length -= (RsslUInt32)readbuffersize;
	return readbuffersize;
}

RsslRet _rsslRestSetHttpMethod(CURL* curl, RsslUInt16 httpMethod, RsslError* error)
{
	switch (httpMethod)
	{
	case RSSL_REST_HTTP_GET:
		(*(rssl_rest_CurlJITFuncs->curl_easy_setopt))(curl, CURLOPT_HTTPGET, 1L);
		break;
	case RSSL_REST_HTTP_POST:
		(*(rssl_rest_CurlJITFuncs->curl_easy_setopt))(curl, CURLOPT_POST, 1L);
		break;
	case RSSL_REST_HTTP_PUT:
		(*(rssl_rest_CurlJITFuncs->curl_easy_setopt))(curl, CURLOPT_PUT, 1L);
		break;
	case RSSL_REST_HTTP_DELETE:
		(*(rssl_rest_CurlJITFuncs->curl_easy_setopt))(curl, CURLOPT_CUSTOMREQUEST, "DELETE");
		break;
	case RSSL_REST_HTTP_PATCH:
		(*(rssl_rest_CurlJITFuncs->curl_easy_setopt))(curl, CURLOPT_CUSTOMREQUEST, "PATCH");
		break;
	default:
		_rsslRestClearError(error);
		error->rsslErrorId = RSSL_RET_FAILURE;
		snprintf(error->text, MAX_RSSL_ERROR_TEXT,
			"<%s:%d> Error: _rsslRestSetHttpMethod() invalid HTTP method(%us).", __FILE__, __LINE__, httpMethod);

		return RSSL_RET_FAILURE;
	}

	return RSSL_RET_SUCCESS;
}

RsslRet rsslRestClientInitialize(RsslError *error)
{
	if ( !(rssl_rest_CurlJITFuncs = ipcGetCurlFuncs(error)) )
	{
		return RSSL_RET_FAILURE;
	}

	RTR_ATOMIC_INCREMENT(rssl_rest_numInitCalls);
	if ((rssl_rest_numInitCalls == 1 ) && (rssl_rest_proxy_header_list == 0))
	{
		rssl_rest_proxy_header_list = (*(rssl_rest_CurlJITFuncs->curl_slist_append))(NULL, "Shoesize: 20");
		rssl_rest_proxy_header_list = (*(rssl_rest_CurlJITFuncs->curl_slist_append))(rssl_rest_proxy_header_list, "Accept:");
	}

	return RSSL_RET_SUCCESS;
}

RsslRet rsslRestClientUninitialize(RsslError *error)
{
	RTR_ATOMIC_DECREMENT(rssl_rest_numInitCalls);

	if (rssl_rest_numInitCalls < 0)
	{
		/* They called uninitialize without a successful initialization */
		RTR_ATOMIC_SET(rssl_rest_numInitCalls, 0);
		_rsslRestClearError(error);
		error->rsslErrorId = RSSL_RET_INIT_NOT_INITIALIZED;
		snprintf(error->text, MAX_RSSL_ERROR_TEXT,
			"<%s:%d> Error: Failed to perform uninitialization without a successful initialization.", __FILE__, __LINE__);
		return RSSL_RET_INIT_NOT_INITIALIZED;
	}
	else if (rssl_rest_numInitCalls == 0)
	{
		if(rssl_rest_proxy_header_list)
		{
			(*(rssl_rest_CurlJITFuncs->curl_slist_free_all))(rssl_rest_proxy_header_list);
			rssl_rest_proxy_header_list = 0;
		}

	}

	return RSSL_RET_SUCCESS;
}

RsslRestClient* rsslCreateRestClient(RsslCreateRestClientOptions *pRestClientOpts, RsslError *pError)
{
	CURLM* curlm;
	RsslErrorInfo rsslErrorInfo;
	RsslRestClientImpl* rsslRestClientImpl = NULL;
		
	if (rssl_rest_numInitCalls == 0)
	{
		_rsslRestClearError(pError);
		pError->rsslErrorId = RSSL_RET_FAILURE;
		snprintf(pError->text, MAX_RSSL_ERROR_TEXT,
			"<%s:%d> Error: The RsslRestClient is not initialized.", __FILE__, __LINE__);
		return 0;
	}

	rsslRestClientImpl = (RsslRestClientImpl*)malloc(sizeof(RsslRestClientImpl));

	if (!rsslRestClientImpl)
	{
		_rsslRestClearError(pError);
		pError->rsslErrorId = RSSL_RET_FAILURE;
		snprintf(pError->text, MAX_RSSL_ERROR_TEXT,
			"<%s:%d> Error: Failed to allocate memory for RsslRestClient.", __FILE__, __LINE__);
		return 0;
	}

	_rsslClearRestClientImpl(rsslRestClientImpl);

	rsslRestClientImpl->dynamicBufferSize = pRestClientOpts->dynamicBufferSize;
	rsslRestClientImpl->numberOfBuffers = pRestClientOpts->numberOfBuffers;

	// Allocate temporary buffer to decode headers.
	if (rsslRestClientImpl->dynamicBufferSize)
	{
		RsslUInt32 i;
		RsslRestBufferImpl* pRestBufferImpl;

		(void)RSSL_MUTEX_INIT_RTSDK(&rsslRestClientImpl->headersPoolMutex);

		for (i = 0; i < rsslRestClientImpl->numberOfBuffers; i++)
		{
			pRestBufferImpl = _rsslRestCreateBufferImpl(RSSL_REST_DEFAULT_MAX_HEADER_LENGTH);

			if (pRestBufferImpl == 0)
			{
				// Release allocated memory before returning the call
				_rsslRestReleaseBufferPool(rsslRestClientImpl);
				free(rsslRestClientImpl);

				_rsslRestClearError(pError);
				pError->rsslErrorId = RSSL_RET_FAILURE;
				snprintf(pError->text, MAX_RSSL_ERROR_TEXT,
					"<%s:%d> Error: Failed to allocate memory for the buffer pool of RsslRestClient.", __FILE__, __LINE__);
				return 0;
			}
			else
			{
				rsslQueueAddLinkToBack(&rsslRestClientImpl->headerBufferPool, &pRestBufferImpl->queueLink);
			}
		}
	}

	if (!rsslInitEventSignal(&rsslRestClientImpl->rsslEventSignal))
	{
		_rsslRestClearError(pError);
		pError->rsslErrorId = RSSL_RET_FAILURE;
		snprintf(pError->text, MAX_RSSL_ERROR_TEXT,
			"<%s:%d> Error: rsslInitEventSignal() failed.", __FILE__, __LINE__);

		return 0;
	}

	rsslRestClientImpl->rsslRestClient.userSpecPtr = pRestClientOpts->userSpecPtr;
	rsslRestClientImpl->rsslRestClient.eventFd = rsslGetEventSignalFD(&rsslRestClientImpl->rsslEventSignal);

	curlm = (*(rssl_rest_CurlJITFuncs->curl_multi_init))();

	if (curlm == 0)
	{
		free(rsslRestClientImpl);
		_rsslRestClearError(pError);
		pError->rsslErrorId = RSSL_RET_FAILURE;
		snprintf(pError->text, MAX_RSSL_ERROR_TEXT,
			"<%s:%d> Error: curl_multi_init() initialize failed.", __FILE__, __LINE__);

		return 0;
	}

	rsslRestClientImpl->pCURLM = curlm;

	if (rsslHashTableInit(&rsslRestClientImpl->restHandleImplTable, pRestClientOpts->requestCountHint, RsslRestCurlHandleSumFunction,
                RsslRestCurlHandleCompareFunction, RSSL_TRUE, &rsslErrorInfo) != RSSL_RET_SUCCESS)
	{
		free(rsslRestClientImpl);
		(*(rssl_rest_CurlJITFuncs->curl_multi_cleanup))(curlm);

		_rsslRestClearError(pError);
		pError->rsslErrorId = RSSL_RET_FAILURE;
		snprintf(pError->text, MAX_RSSL_ERROR_TEXT,
			"<%s:%d> Error: rsslHashTableInit() failed with text: %s", __FILE__, __LINE__, rsslErrorInfo.rsslError.text);
		return 0;
	}

	return &rsslRestClientImpl->rsslRestClient;
}

RsslRet rsslDestroyRestClient(RsslRestClient *pRsslRestClient, RsslError *pError)
{
	RsslRestClientImpl* rsslRestClientImpl = (RsslRestClientImpl*)pRsslRestClient;
	CURLMcode mcode;
	RsslRet ret = RSSL_RET_SUCCESS;

	/* Ensure that there is no RsslRestHandleImpl left in restHandleImplTable */
	if (rsslRestClientImpl->restHandleImplTable.elementCount > 0)
	{
		RsslRestHandleImpl *restHandleImpl = NULL;
		RsslQueueLink *pLink;
		RsslUInt32 index;

		for (index = 0; index < rsslRestClientImpl->restHandleImplTable.queueCount; index++)
		{
			RSSL_QUEUE_FOR_EACH_LINK(&rsslRestClientImpl->restHandleImplTable.queueList[index], pLink)
			{
				restHandleImpl = RSSL_QUEUE_LINK_TO_OBJECT(RsslRestHandleImpl, hashLink, pLink);

				rsslRestCloseHandle(&restHandleImpl->rsslRestHandle, pError);
			}
		}
	}

	if (rsslRestClientImpl->dynamicBufferSize)
	{
		_rsslRestReleaseBufferPool(rsslRestClientImpl);

		(void)RSSL_MUTEX_DESTROY(&rsslRestClientImpl->headersPoolMutex);
	}

	mcode = (*(rssl_rest_CurlJITFuncs->curl_multi_cleanup))(rsslRestClientImpl->pCURLM);

	if (mcode != CURLM_OK)
	{
		_rsslRestClearError(pError);
		pError->rsslErrorId = RSSL_RET_FAILURE;
		snprintf(pError->text, MAX_RSSL_ERROR_TEXT,
			"<%s:%d> Error: curl_multi_cleanup() failed with text: %s", __FILE__, __LINE__,
			(*(rssl_rest_CurlJITFuncs->curl_multi_strerror))(mcode));

		ret = RSSL_RET_FAILURE;
	}

	rsslCleanupEventSignal(&rsslRestClientImpl->rsslEventSignal);

	rsslHashTableCleanup(&rsslRestClientImpl->restHandleImplTable);

	free(rsslRestClientImpl);

	return ret;
}

RsslInt64 rsslRestClientDispatch(RsslRestClient* RsslRestClient)
{
	RsslRestClientImpl* rsslRestClientImpl = (RsslRestClientImpl*)RsslRestClient;
	int still_running;
	CURLMcode mcode;
	struct CURLMsg *pCurlMsg = NULL;
	int msginqueue = 0;
	RsslHashLink *pRsslHashLink = NULL;
	RsslRestHandleImpl *pRestHandleImpl = NULL;
	CURL *pHandle = NULL;
	int numfds = 0;

	mcode = (*(rssl_rest_CurlJITFuncs->curl_multi_perform))(rsslRestClientImpl->pCURLM, &still_running);
	
	if (!still_running)
	{
		rsslResetEventSignal(&rsslRestClientImpl->rsslEventSignal);
	}
	else
	{
		/* wait for activity, timeout or "nothing" */
		mcode = (*(rssl_rest_CurlJITFuncs->curl_multi_poll))(rsslRestClientImpl->pCURLM, NULL, 0, 10, &numfds);

		if (!numfds)
		{
			return still_running;
		}
	}

	/* Handling status for each individual requests */
	do {
		pCurlMsg = (*(rssl_rest_CurlJITFuncs->curl_multi_info_read))(rsslRestClientImpl->pCURLM, &msginqueue);
		if (pCurlMsg && (pCurlMsg->msg == CURLMSG_DONE)) {

			pHandle = pCurlMsg->easy_handle;

			pRsslHashLink = rsslHashTableFind(&rsslRestClientImpl->restHandleImplTable, (void*)pHandle, NULL);
			
			if (pRsslHashLink)
			{
				pRestHandleImpl = RSSL_HASH_LINK_TO_OBJECT(RsslRestHandleImpl, hashLink, pRsslHashLink);

				if (pCurlMsg->data.result == CURLE_OK)
				{
					notifyResponseCallback(pRestHandleImpl);
				}
				else
				{
					/* Fires the RsslRestErrorCallback event when an error occurs */
					RsslRestResponseEvent rsslRestResponseEvent;

					_rsslRestClearError(&pRestHandleImpl->rsslError);
					pRestHandleImpl->rsslError.rsslErrorId = RSSL_RET_FAILURE;
					snprintf(pRestHandleImpl->rsslError.text, MAX_RSSL_ERROR_TEXT,
						"<%s:%d> Error: The REST request failed with text: %s",
						__FILE__, __LINE__, (*(rssl_rest_CurlJITFuncs->curl_easy_strerror))(pCurlMsg->data.result));

					rsslRestResponseEvent.closure = pRestHandleImpl->userPtr;
					rsslRestResponseEvent.handle = &pRestHandleImpl->rsslRestHandle;

					if (pRestHandleImpl->pRsslRestErrorCallback)
						(*pRestHandleImpl->pRsslRestErrorCallback)(&pRestHandleImpl->rsslError, &rsslRestResponseEvent);
				}
			}
		}

	} while (pCurlMsg);

	return still_running;
}

static curl_socket_t _opensocket(void *clientp,
	curlsocktype purpose, struct curl_sockaddr *address)
{
	RsslRestHandleImpl* rsslRestHandleImpl;
	rsslRestHandleImpl = (RsslRestHandleImpl *)clientp;

	rsslRestHandleImpl->rsslRestHandle.socketId = socket(address->family, address->socktype, address->protocol);

	return rsslRestHandleImpl->rsslRestHandle.socketId;
}

CURLcode commonCurlOptions(CURL* curl, RsslRestRequestArgs* requestArgs)
{
	CURLcode curlCode;
	size_t userPassLength = 0;
	char* proxyUserAndPasswd = NULL;
	RsslError rsslError;
	RsslBuffer* userNameEncodedUrl;
	RsslBuffer* passwordEncodedUrl;
	size_t addtionalSize = 2;
	long timeout = (long)requestArgs->requestTimeOut;

	if (requestArgs->networkArgs.proxyArgs.proxyUserName.length && requestArgs->networkArgs.proxyArgs.proxyPassword.length)
	{
		userNameEncodedUrl = rsslRestEncodeUrlData(&requestArgs->networkArgs.proxyArgs.proxyUserName, &rsslError);
		if (rsslError.rsslErrorId == RSSL_RET_FAILURE)
		{
			return CURLE_OUT_OF_MEMORY;
		}

		passwordEncodedUrl = rsslRestEncodeUrlData(&requestArgs->networkArgs.proxyArgs.proxyPassword, &rsslError);
		if (rsslError.rsslErrorId == RSSL_RET_FAILURE)
		{
			return CURLE_OUT_OF_MEMORY;
		}

		userPassLength = ((size_t)userNameEncodedUrl->length) + addtionalSize; /* add additional memory for : and \0 */
		userPassLength += passwordEncodedUrl->length;

		if (requestArgs->networkArgs.proxyArgs.proxyDomain.length)
		{
			userPassLength += ((size_t)requestArgs->networkArgs.proxyArgs.proxyDomain.length) + addtionalSize; /* add additional memory for \\ */
		}

		if ((proxyUserAndPasswd = (char*)malloc(userPassLength)) == NULL) // Accounts for null character at the end of the string
		{
			return CURLE_OUT_OF_MEMORY;
		}

		if (requestArgs->networkArgs.proxyArgs.proxyDomain.length)
		{
			snprintf(proxyUserAndPasswd, userPassLength, "%s\\%s:%s", requestArgs->networkArgs.proxyArgs.proxyDomain.data, userNameEncodedUrl->data,
				passwordEncodedUrl->data);
		}
		else
		{
			snprintf(proxyUserAndPasswd, userPassLength, "%s:%s", userNameEncodedUrl->data, passwordEncodedUrl->data);
		}

		if ((curlCode = (*(rssl_rest_CurlJITFuncs->curl_easy_setopt))(curl, CURLOPT_PROXYUSERPWD, proxyUserAndPasswd)) != CURLE_OK)
			return curlCode;

		free(proxyUserAndPasswd);

		if (userNameEncodedUrl != &requestArgs->networkArgs.proxyArgs.proxyUserName)
		{
			free(userNameEncodedUrl->data);
			free(userNameEncodedUrl);
		}

		if (passwordEncodedUrl != &requestArgs->networkArgs.proxyArgs.proxyPassword)
		{
			free(passwordEncodedUrl->data);
			free(passwordEncodedUrl);
		}
	}

	if (requestArgs->networkArgs.proxyArgs.proxyHostName.length)
	{
		if ((curlCode = (*(rssl_rest_CurlJITFuncs->curl_easy_setopt))(curl, CURLOPT_PROXY, requestArgs->networkArgs.proxyArgs.proxyHostName.data)) != CURLE_OK)
			return curlCode;
	}

	if (requestArgs->networkArgs.proxyArgs.proxyPort.length)
	{
		long proxyPortNum = net2host_u16(rsslRestGetServByName(requestArgs->networkArgs.proxyArgs.proxyPort.data));
		
		if (proxyPortNum != -1)
		{
			if ((curlCode = (*(rssl_rest_CurlJITFuncs->curl_easy_setopt))(curl, CURLOPT_PROXYPORT, proxyPortNum)) != CURLE_OK)
				return curlCode;
		}
	}

	if (requestArgs->networkArgs.openSSLCAStore.length)
	{
		if ((curlCode = (*(rssl_rest_CurlJITFuncs->curl_easy_setopt))(curl, CURLOPT_CAPATH, requestArgs->networkArgs.openSSLCAStore.data)) != CURLE_OK)
			return curlCode;
	}

	if (requestArgs->networkArgs.interfaceName.length)
	{
		if ((curlCode = (*(rssl_rest_CurlJITFuncs->curl_easy_setopt))(curl, CURLOPT_INTERFACE, requestArgs->networkArgs.interfaceName.data)) != CURLE_OK)
			return curlCode;
	}

	if ((curlCode = (*(rssl_rest_CurlJITFuncs->curl_easy_setopt))(curl, CURLOPT_PROXYAUTH, CURLAUTH_ANY)) != CURLE_OK)
		return curlCode;

	if ((curlCode = (*(rssl_rest_CurlJITFuncs->curl_easy_setopt))(curl, CURLOPT_DEFAULT_PROTOCOL, "http")) != CURLE_OK)
		return curlCode;

	if ((curlCode = (*(rssl_rest_CurlJITFuncs->curl_easy_setopt))(curl, CURLOPT_TCP_NODELAY, (long)requestArgs->networkArgs.tcpNoDelay)) != CURLE_OK)
		return curlCode;

	if ((curlCode = (*(rssl_rest_CurlJITFuncs->curl_easy_setopt))(curl, CURLOPT_NOSIGNAL, 1L)) != CURLE_OK)
		return curlCode;

	if ((curlCode = (*(rssl_rest_CurlJITFuncs->curl_easy_setopt))(curl, CURLOPT_HTTPPROXYTUNNEL, 1L)) != CURLE_OK)
		return curlCode;

	if ((curlCode = (*(rssl_rest_CurlJITFuncs->curl_easy_setopt))(curl, CURLOPT_SSL_VERIFYPEER, 1L)) != CURLE_OK)
		return curlCode;

	if ((curlCode = (*(rssl_rest_CurlJITFuncs->curl_easy_setopt))(curl, CURLOPT_SSL_VERIFYHOST, 2L)) != CURLE_OK)
		return curlCode;

	if ((curlCode = (*(rssl_rest_CurlJITFuncs->curl_easy_setopt))(curl, CURLOPT_URL, requestArgs->url.data)) != CURLE_OK)
		return curlCode;

	/* Set headers for proxy separately */
	if ((curlCode = (*(rssl_rest_CurlJITFuncs->curl_easy_setopt))(curl, CURLOPT_PROXYHEADER, rssl_rest_proxy_header_list)) != CURLE_OK)
		return curlCode;

	/* Suppress handling proxy header via the header callback */
	if ((curlCode = (*(rssl_rest_CurlJITFuncs->curl_easy_setopt))(curl, CURLOPT_SUPPRESS_CONNECT_HEADERS, 1L)) != CURLE_OK)
		return curlCode;

#ifdef RSSL_REST_CLIENT_VERBOSE
	if ((curlCode = (*(rssl_rest_CurlJITFuncs->curl_easy_setopt))(curl, CURLOPT_VERBOSE, 1L)) != CURLE_OK)
		return curlCode;
#endif

	if ((curlCode = (*(rssl_rest_CurlJITFuncs->curl_easy_setopt))(curl, CURLOPT_TIMEOUT, timeout)) != CURLE_OK)
		return curlCode;

	return CURLE_OK;
}

RsslRestHandle* rsslRestClientNonBlockingRequest(RsslRestClient* restClient, RsslRestRequestArgs* requestArgs,
	RsslRestResponseCallback* restResponseCallback,
	RsslRestErrorCallback*    restErrorCallback,
	RsslBuffer*	memoryBuffer,
	RsslError *error)
{
	CURL* curl;
	CURLcode curlCode;
	CURLMcode mcode;
	RsslRestHandleImpl* rsslRestHandleImpl;
	RsslRestClientImpl* restClientImpl = (RsslRestClientImpl*)restClient;

	_rsslRestClearError(error);

	rsslRestHandleImpl = (RsslRestHandleImpl*)malloc(sizeof(RsslRestHandleImpl));

	if (rsslRestHandleImpl == 0)
	{
		error->rsslErrorId = RSSL_RET_FAILURE;
		snprintf(error->text, MAX_RSSL_ERROR_TEXT,
			"<%s:%d> Error: memory allocation failed for RsslRestHandle.", __FILE__, __LINE__);

		return 0;
	}

	_rsslClearRestHandleImpl(rsslRestHandleImpl);

	rsslRestHandleImpl->rsslRestResponseImpl.pRestResponse = (RsslRestResponse*)malloc(sizeof(RsslRestResponse));

	if (rsslRestHandleImpl->rsslRestResponseImpl.pRestResponse == 0)
	{
		free(rsslRestHandleImpl);

		error->rsslErrorId = RSSL_RET_FAILURE;
		snprintf(error->text, MAX_RSSL_ERROR_TEXT,
			"<%s:%d> Error: memory allocation failed for RsslRestResponse.", __FILE__, __LINE__);

		return 0;
	}

	rsslClearRestResponse(rsslRestHandleImpl->rsslRestResponseImpl.pRestResponse);

	rsslRestHandleImpl->pUserMemory = memoryBuffer;
	rsslRestHandleImpl->pRsslRestReponseCallback = restResponseCallback;
	rsslRestHandleImpl->pRsslRestErrorCallback = restErrorCallback;
	_rsslRestSetToRsslRestBufferImpl(memoryBuffer, &rsslRestHandleImpl->rsslRestResponseImpl.restBufferImpl);

	rsslRestHandleImpl->pRsslRestClientImpl = restClientImpl;

	rsslRestHandleImpl->userPtr = requestArgs->pUserSpecPtr;

	curl = (*(rssl_rest_CurlJITFuncs->curl_easy_init))();

	if (curl == 0)
	{
		free(rsslRestHandleImpl->rsslRestResponseImpl.pRestResponse);
		free(rsslRestHandleImpl);

		error->rsslErrorId = RSSL_RET_FAILURE;
		snprintf(error->text, MAX_RSSL_ERROR_TEXT,
			"<%s:%d> Error: curl_easy_init() initialize failed.", __FILE__, __LINE__);

		return 0;
	}

	if (_rsslRestSetHttpMethod(curl, requestArgs->httpMethod, error) != RSSL_RET_SUCCESS)
	{
		(*(rssl_rest_CurlJITFuncs->curl_easy_cleanup))(curl);
		free(rsslRestHandleImpl->rsslRestResponseImpl.pRestResponse);
		free(rsslRestHandleImpl);

		return 0;
	}

	rsslRestHandleImpl->pCurlHeaderList = _rsslRestExtractHeaderInfo(curl, requestArgs, error);

	if (error ->rsslErrorId != RSSL_RET_SUCCESS)
	{
		goto Failed;
	}

	if ((curlCode = commonCurlOptions(curl, requestArgs)) != CURLE_OK)
	{
		if (curlCode == CURLE_OUT_OF_MEMORY)
		{
			error->rsslErrorId = RSSL_RET_FAILURE;
			snprintf(error->text, MAX_RSSL_ERROR_TEXT,
				"<%s:%d> Error: Memory allocation failed for setting CURL options.", __FILE__, __LINE__);
		}
		else
		{
			error->rsslErrorId = RSSL_RET_FAILURE;
			snprintf(error->text, MAX_RSSL_ERROR_TEXT,
				"<%s:%d> Error: Failed to set CURL options with text: %s", __FILE__, __LINE__, (*(rssl_rest_CurlJITFuncs->curl_easy_strerror))(curlCode));
		}

		goto Failed;
	}

	if (requestArgs->httpBody.length != 0)
	{
		rsslRestHandleImpl->httpDataBodyForPost.data = requestArgs->httpBody.data;
		rsslRestHandleImpl->httpDataBodyForPost.length = requestArgs->httpBody.length;
		if ((curlCode = (*(rssl_rest_CurlJITFuncs->curl_easy_setopt))(curl, CURLOPT_READFUNCTION, _rssl_rest_read_databody_callback)) != CURLE_OK)
			goto Failed;

		if ((curlCode = (*(rssl_rest_CurlJITFuncs->curl_easy_setopt))(curl, CURLOPT_READDATA, &rsslRestHandleImpl->httpDataBodyForPost)) != CURLE_OK)
			goto Failed;

		if ((curlCode = (*(rssl_rest_CurlJITFuncs->curl_easy_setopt))(curl, CURLOPT_POSTFIELDSIZE, rsslRestHandleImpl->httpDataBodyForPost.length)) != CURLE_OK)
			goto Failed;
	}

	if ((curlCode = (*(rssl_rest_CurlJITFuncs->curl_easy_setopt))(curl, CURLOPT_WRITEFUNCTION, rssl_rest_write_databody_callback)) != CURLE_OK)
		goto Failed;

	if ((curlCode = (*(rssl_rest_CurlJITFuncs->curl_easy_setopt))(curl, CURLOPT_WRITEDATA, rsslRestHandleImpl)) != CURLE_OK)
		goto Failed;

	if (restClientImpl->dynamicBufferSize)
	{
		if ((curlCode = (*(rssl_rest_CurlJITFuncs->curl_easy_setopt))(curl, CURLOPT_HEADERFUNCTION, rssl_rest_write_header_callback_with_dynamic_size)) != CURLE_OK)
			goto Failed;
	}
	else
	{
		if ((curlCode = (*(rssl_rest_CurlJITFuncs->curl_easy_setopt))(curl, CURLOPT_HEADERFUNCTION, rssl_rest_write_header_callback)) != CURLE_OK)
			goto Failed;
	}

	if ((curlCode = (*(rssl_rest_CurlJITFuncs->curl_easy_setopt))(curl, CURLOPT_HEADERDATA, rsslRestHandleImpl)) != CURLE_OK)
		goto Failed;

	if ((curlCode = (*(rssl_rest_CurlJITFuncs->curl_easy_setopt))(curl, CURLOPT_OPENSOCKETFUNCTION, _opensocket)) != CURLE_OK)
		goto Failed;

	if ((curlCode = (*(rssl_rest_CurlJITFuncs->curl_easy_setopt))(curl, CURLOPT_OPENSOCKETDATA, rsslRestHandleImpl)) != CURLE_OK)
		goto Failed;

	if (restClientImpl->dynamicBufferSize)
	{
		rsslRestHandleImpl->pRsslRestBufferImpl = _getRsslRestBufferImplFromPool(restClientImpl);

		if (rsslRestHandleImpl->pRsslRestBufferImpl == 0)
		{
			error->rsslErrorId = RSSL_RET_FAILURE;
			snprintf(error->text, MAX_RSSL_ERROR_TEXT,
				"<%s:%d> Error: memory allocation failed for the buffer pool of RsslRestClient.", __FILE__, __LINE__);

			goto Failed;
		}

		memset(rsslRestHandleImpl->pRsslRestBufferImpl->pStartPos, 0, rsslRestHandleImpl->pRsslRestBufferImpl->bufferLength);
	}

	mcode = (*(rssl_rest_CurlJITFuncs->curl_multi_add_handle))(restClientImpl->pCURLM, curl);

	if (mcode != CURLM_OK)
	{
		error->rsslErrorId = RSSL_RET_FAILURE;
		snprintf(error->text, MAX_RSSL_ERROR_TEXT,
			"<%s:%d> Error: curl_multi_add_handle() initialize failed with text: %s", __FILE__, __LINE__, 
			(*(rssl_rest_CurlJITFuncs->curl_multi_strerror))(mcode));

		goto Failed;
	}

	rsslRestHandleImpl->handle = curl;

	rsslHashTableInsertLink(&restClientImpl->restHandleImplTable, &rsslRestHandleImpl->hashLink, (void*)rsslRestHandleImpl->handle, NULL);

	rsslSetEventSignal(&restClientImpl->rsslEventSignal);

	return &rsslRestHandleImpl->rsslRestHandle;

Failed:

	// The error message has not been set yet.
	if ( error->rsslErrorId == RSSL_RET_SUCCESS)
	{
		error->rsslErrorId = RSSL_RET_FAILURE;
		snprintf(error->text, MAX_RSSL_ERROR_TEXT,
			"<%s:%d> Error: Failed to perform the request with text: %s", __FILE__, __LINE__, (*(rssl_rest_CurlJITFuncs->curl_easy_strerror))(curlCode));
	}

	(*(rssl_rest_CurlJITFuncs->curl_easy_cleanup))(curl);
	(*(rssl_rest_CurlJITFuncs->curl_slist_free_all))(rsslRestHandleImpl->pCurlHeaderList);
	free(rsslRestHandleImpl->rsslRestResponseImpl.pRestResponse);
	free(rsslRestHandleImpl);

	return 0;
}

RsslRet _rsslRestClientBlockingRequest(
	RsslRestRequestArgs* requestArgs, 
	size_t (*readbody_callback)(char* readptr, size_t size, size_t memblock, void* userptr),
	size_t(*writebody_callback)(char* writeptr, size_t size, size_t memblock, void* userptr),
	size_t(*header_callback)(char* headerptr, size_t size, size_t memblock, void* userptr),
	RsslBuffer*	memorybuffer,
	RsslError *error)
{
	CURLcode code = CURLE_FAILED_INIT;
	CURL* curl;
	char error_buf[CURL_ERROR_SIZE];
	RsslRestHandleImpl*  restHandleImpl = (RsslRestHandleImpl*)requestArgs->pUserSpecPtr;
	error_buf[0] = '\0';

	curl = (*(rssl_rest_CurlJITFuncs->curl_easy_init))();

	if (curl == 0)
	{
		error->rsslErrorId = RSSL_RET_FAILURE;
		snprintf(error->text, MAX_RSSL_ERROR_TEXT,
			"<%s:%d> Error: curl_easy_init() initialize failed.", __FILE__, __LINE__);

		return RSSL_RET_FAILURE;
	}

	if (_rsslRestSetHttpMethod(curl, requestArgs->httpMethod, error) != RSSL_RET_SUCCESS)
	{
		(*(rssl_rest_CurlJITFuncs->curl_easy_cleanup))(curl);
		return error->rsslErrorId;
	}

	if ((code = commonCurlOptions(curl, requestArgs)) != CURLE_OK)
	{
		if (code == CURLE_OUT_OF_MEMORY)
		{
			error->rsslErrorId = RSSL_RET_FAILURE;
			snprintf(error->text, MAX_RSSL_ERROR_TEXT,
				"<%s:%d> Error: Memory allocation failed for setting CURL options.", __FILE__, __LINE__);
		}
		else
		{
			error->rsslErrorId = RSSL_RET_FAILURE;
			snprintf(error->text, MAX_RSSL_ERROR_TEXT,
				"<%s:%d> Error: Failed to set CURL options with text: %s", __FILE__, __LINE__, (*(rssl_rest_CurlJITFuncs->curl_easy_strerror))(code));
		}

		(*(rssl_rest_CurlJITFuncs->curl_easy_cleanup))(curl);

		return RSSL_RET_FAILURE;
	}

	restHandleImpl->pCurlHeaderList = _rsslRestExtractHeaderInfo(curl, requestArgs, error);

	if (error->rsslErrorId != RSSL_RET_SUCCESS)
	{
		goto Failed;
	}

	if (readbody_callback)
	{
		restHandleImpl->httpDataBodyForPost.data = requestArgs->httpBody.data;
		restHandleImpl->httpDataBodyForPost.length = requestArgs->httpBody.length;
		if ((code = (*(rssl_rest_CurlJITFuncs->curl_easy_setopt))(curl, CURLOPT_READFUNCTION, _rssl_rest_read_databody_callback)) != CURLE_OK)
			goto Failed;

		if ((code = (*(rssl_rest_CurlJITFuncs->curl_easy_setopt))(curl, CURLOPT_READDATA, &restHandleImpl->httpDataBodyForPost)) != CURLE_OK)
			goto Failed;

		if ((code = (*(rssl_rest_CurlJITFuncs->curl_easy_setopt))(curl, CURLOPT_POSTFIELDSIZE, restHandleImpl->httpDataBodyForPost.length)) != CURLE_OK)
			goto Failed;
	}

	if (writebody_callback)
	{
		if ((code = (*(rssl_rest_CurlJITFuncs->curl_easy_setopt))(curl, CURLOPT_WRITEFUNCTION, writebody_callback)) != CURLE_OK)
			goto Failed;

		if ((code = (*(rssl_rest_CurlJITFuncs->curl_easy_setopt))(curl, CURLOPT_WRITEDATA, requestArgs->pUserSpecPtr)) != CURLE_OK)
			goto Failed;
	}

	if (header_callback)
	{
		if ((code = (*(rssl_rest_CurlJITFuncs->curl_easy_setopt))(curl, CURLOPT_HEADERFUNCTION, header_callback)) != CURLE_OK)
			goto Failed;

		if ((code = (*(rssl_rest_CurlJITFuncs->curl_easy_setopt))(curl, CURLOPT_HEADERDATA, requestArgs->pUserSpecPtr)) != CURLE_OK)
			goto Failed;
	}

	restHandleImpl->handle = curl;

	code = (*(rssl_rest_CurlJITFuncs->curl_easy_perform))(curl);

	if (code != CURLE_OK)
	{
		goto Failed;
	}

	if (restHandleImpl->rsslError.rsslErrorId != RSSL_RET_SUCCESS)
	{
		error->rsslErrorId = restHandleImpl->rsslError.rsslErrorId;
		strncpy(error->text, restHandleImpl->rsslError.text, MAX_RSSL_ERROR_TEXT);
	}

	(*(rssl_rest_CurlJITFuncs->curl_slist_free_all))(restHandleImpl->pCurlHeaderList);
	(*(rssl_rest_CurlJITFuncs->curl_easy_cleanup))(curl);
	
	return restHandleImpl->rsslError.rsslErrorId;

Failed:

	// The error message has not been set yet.
	if (error->rsslErrorId == RSSL_RET_SUCCESS)
	{
		error->rsslErrorId = RSSL_RET_FAILURE;
		snprintf(error->text, MAX_RSSL_ERROR_TEXT,
			"<%s:%d> Error: Failed to perform the request with text: %s", __FILE__, __LINE__, (*(rssl_rest_CurlJITFuncs->curl_easy_strerror))(code));
	}

	(*(rssl_rest_CurlJITFuncs->curl_slist_free_all))(restHandleImpl->pCurlHeaderList);
	(*(rssl_rest_CurlJITFuncs->curl_easy_cleanup))(curl);

	return RSSL_RET_FAILURE;
}

RsslRet rsslRestClientBlockingRequest(RsslRestClient* restClient, RsslRestRequestArgs* restRequestArgs,
	RsslRestResponse* restResponse,
	RsslBuffer*	memorybuffer,
	RsslError *error)
{
	void* userDefiendPtr = restRequestArgs->pUserSpecPtr;
	RsslRet rsslRet;
	RsslRestHandleImpl restHandleImpl;
	RsslRestClientImpl* pRsslRestClientImpl = (RsslRestClientImpl*)restClient;

	rsslClearRestResponse(restResponse);
	_rsslClearRestHandleImpl(&restHandleImpl);

	_rsslRestClearError(error);

	_rsslRestSetToRsslRestBufferImpl(memorybuffer, &restHandleImpl.rsslRestResponseImpl.restBufferImpl);
	restHandleImpl.rsslRestResponseImpl.pRestResponse = restResponse;

	restHandleImpl.pRsslRestClientImpl = pRsslRestClientImpl;
	restRequestArgs->pUserSpecPtr = (void*)(&restHandleImpl);

	if (pRsslRestClientImpl->dynamicBufferSize)
	{
		restHandleImpl.pRsslRestBufferImpl = _getRsslRestBufferImplFromPool(pRsslRestClientImpl);

		if (restHandleImpl.pRsslRestBufferImpl == 0)
		{
			error->rsslErrorId = RSSL_RET_FAILURE;
			snprintf(error->text, MAX_RSSL_ERROR_TEXT,
				"<%s:%d> Error: Failed to allocate memory from the buffer pool of RsslRestClient.", __FILE__, __LINE__);

			return RSSL_RET_FAILURE;
		}

		memset(restHandleImpl.pRsslRestBufferImpl->pStartPos, 0, restHandleImpl.pRsslRestBufferImpl->bufferLength);

		rsslRet = _rsslRestClientBlockingRequest(restRequestArgs, _rssl_rest_read_databody_callback,
			rssl_rest_write_databody_callback, rssl_rest_write_header_callback_with_dynamic_size, memorybuffer, error);

		_returnRsslRestBufferImplToPool(pRsslRestClientImpl, restHandleImpl.pRsslRestBufferImpl);
	}
	else
	{
		rsslRet = _rsslRestClientBlockingRequest(restRequestArgs, _rssl_rest_read_databody_callback,
			rssl_rest_write_databody_callback, rssl_rest_write_header_callback, memorybuffer, error);
	}

	restHandleImpl.userPtr = userDefiendPtr;
	restRequestArgs->pUserSpecPtr = userDefiendPtr;

	return rsslRet;
}

RsslRet rsslRestCloseHandle(RsslRestHandle* handle, RsslError* pError)
{
	RsslRestHandleImpl* restHandleImpl = (RsslRestHandleImpl*)handle;
	CURLMcode mcode;

	if (restHandleImpl->pRsslRestClientImpl->dynamicBufferSize)
	{
		_returnRsslRestBufferImplToPool(restHandleImpl->pRsslRestClientImpl, restHandleImpl->pRsslRestBufferImpl);
	}

	mcode = (*(rssl_rest_CurlJITFuncs->curl_multi_remove_handle))(restHandleImpl->pRsslRestClientImpl->pCURLM, restHandleImpl->handle);

	if (mcode != CURLM_OK)
	{
		_rsslRestClearError(pError);
		pError->rsslErrorId = RSSL_RET_FAILURE;
		snprintf(pError->text, MAX_RSSL_ERROR_TEXT,
			"<%s:%d> Error: curl_multi_remove_handle() failed with text: %s", __FILE__, __LINE__,
			(*(rssl_rest_CurlJITFuncs->curl_multi_strerror))(mcode));

		return RSSL_RET_FAILURE;
	}

	rsslHashTableRemoveLink(&restHandleImpl->pRsslRestClientImpl->restHandleImplTable, &restHandleImpl->hashLink);

	(*(rssl_rest_CurlJITFuncs->curl_easy_cleanup))(restHandleImpl->handle);

	(*(rssl_rest_CurlJITFuncs->curl_slist_free_all))(restHandleImpl->pCurlHeaderList);
	free(restHandleImpl->rsslRestResponseImpl.pRestResponse);
	free(restHandleImpl);

	return RSSL_RET_SUCCESS;
}

void _rsslAllocateAndConvertBufferForUrlData(RsslBuffer* inputBuffer, RsslUInt32 inputIndex, RsslBuffer** buffer, 
													RsslUInt32* bufferIndex, const char* value, RsslError* pError)
{
	RsslUInt32 valueLength = (RsslUInt32)strlen(value);

	if (!(*buffer))
	{
		(*buffer) = (RsslBuffer*)malloc(sizeof(RsslBuffer));
		if ((*buffer) == 0)
		{
			pError->rsslErrorId = RSSL_RET_FAILURE;
			return;
		}

		(*buffer)->length = inputBuffer->length + (inputBuffer->length * 2) + 1;
		(*buffer)->data = (char*)malloc((*buffer)->length);
                
		if ((*buffer)->data == 0)
		{
			free((*buffer));
			pError->rsslErrorId = RSSL_RET_FAILURE;
			return;
		}

		memset((*buffer)->data, 0, (*buffer)->length);

		strncpy((*buffer)->data, inputBuffer->data, inputIndex);

		strncpy((*buffer)->data + inputIndex, value, valueLength);

		(*bufferIndex) = inputIndex + valueLength - 1;
	}
	else
	{
		strcpy((*buffer)->data + (*bufferIndex) + 1, value);
		(*bufferIndex) = (*bufferIndex) + valueLength;
	}
}

RsslBuffer* rsslRestEncodeUrlData(RsslBuffer* inputBuffer, RsslError* pError)
{
	RsslUInt32 idx = 0;
	RsslBuffer* allocatedBuffer = 0;
	RsslUInt32 bufferIndex = 0;

	pError->rsslErrorId = RSSL_RET_SUCCESS;

	/* Execulding unreserved characters ALPHA / DIGIT / "-" / "." / "_" / "~" characters. Ref:https://tools.ietf.org/html/rfc3986#section-2.3 */
	for (; idx < inputBuffer->length; idx++)
	{
		switch (inputBuffer->data[idx])
		{
			case 33: // ascii code for !

				_rsslAllocateAndConvertBufferForUrlData(inputBuffer, idx, &allocatedBuffer, &bufferIndex, "%21", pError);

				break;
			case 42: // ascii code for *

				_rsslAllocateAndConvertBufferForUrlData(inputBuffer, idx, &allocatedBuffer, &bufferIndex, "%2A", pError);

				break;
			case 39: // ascii code for '

				_rsslAllocateAndConvertBufferForUrlData(inputBuffer, idx, &allocatedBuffer, &bufferIndex, "%27", pError);

				break;
			case 40: // ascii code for (

				_rsslAllocateAndConvertBufferForUrlData(inputBuffer, idx, &allocatedBuffer, &bufferIndex, "%28", pError);

				break;
			case 41: // ascii code for )

				_rsslAllocateAndConvertBufferForUrlData(inputBuffer, idx, &allocatedBuffer, &bufferIndex, "%29", pError);

				break;
			case 59: // ascii code for ;

				_rsslAllocateAndConvertBufferForUrlData(inputBuffer, idx, &allocatedBuffer, &bufferIndex, "%3B", pError);

				break;
			case 58: // ascii code for :

				_rsslAllocateAndConvertBufferForUrlData(inputBuffer, idx, &allocatedBuffer, &bufferIndex, "%3A", pError);

				break;
			case 64: // ascii code for @

				_rsslAllocateAndConvertBufferForUrlData(inputBuffer, idx, &allocatedBuffer, &bufferIndex, "%40", pError);

				break;
			case 38: // ascii code for &

				_rsslAllocateAndConvertBufferForUrlData(inputBuffer, idx, &allocatedBuffer, &bufferIndex, "%26", pError);

				break;
			case 61: // ascii code for =

				_rsslAllocateAndConvertBufferForUrlData(inputBuffer, idx, &allocatedBuffer, &bufferIndex, "%3D", pError);

				break;
			case 43: // ascii code for +

				_rsslAllocateAndConvertBufferForUrlData(inputBuffer, idx, &allocatedBuffer, &bufferIndex, "%2B", pError);

				break;
			case 36: // ascii code for $

				_rsslAllocateAndConvertBufferForUrlData(inputBuffer, idx, &allocatedBuffer, &bufferIndex, "%24", pError);

				break;
			case 44: // ascii code for ,

				_rsslAllocateAndConvertBufferForUrlData(inputBuffer, idx, &allocatedBuffer, &bufferIndex, "%2C", pError);

				break;
			case 47: // ascii code for /

				_rsslAllocateAndConvertBufferForUrlData(inputBuffer, idx, &allocatedBuffer, &bufferIndex, "%2F", pError);

				break;
			case 63: // ascii code for ?

				_rsslAllocateAndConvertBufferForUrlData(inputBuffer, idx, &allocatedBuffer, &bufferIndex, "%3F", pError);

				break;
			case 35: // ascii code for #

				_rsslAllocateAndConvertBufferForUrlData(inputBuffer, idx, &allocatedBuffer, &bufferIndex, "%23", pError);

				break;
			case 91: // ascii code for [

				_rsslAllocateAndConvertBufferForUrlData(inputBuffer, idx, &allocatedBuffer, &bufferIndex, "%5B", pError);

				break;
			case 93: // ascii code for ]

				_rsslAllocateAndConvertBufferForUrlData(inputBuffer, idx, &allocatedBuffer, &bufferIndex, "%5D", pError);

				break;
			case 10: // ascii code for new line

				_rsslAllocateAndConvertBufferForUrlData(inputBuffer, idx, &allocatedBuffer, &bufferIndex, "%0A", pError);

				break;
			case 32: // ascii code for space

				_rsslAllocateAndConvertBufferForUrlData(inputBuffer, idx, &allocatedBuffer, &bufferIndex, "%20", pError);

				break;
			case 34: // ascii code for "

				_rsslAllocateAndConvertBufferForUrlData(inputBuffer, idx, &allocatedBuffer, &bufferIndex, "%22", pError);

				break;
			case 37: // ascii code for %

				_rsslAllocateAndConvertBufferForUrlData(inputBuffer, idx, &allocatedBuffer, &bufferIndex, "%25", pError);

				break;
			case 60: // ascii code for <

				_rsslAllocateAndConvertBufferForUrlData(inputBuffer, idx, &allocatedBuffer, &bufferIndex, "%3C", pError);

				break;
			case 62: // ascii code for >

				_rsslAllocateAndConvertBufferForUrlData(inputBuffer, idx, &allocatedBuffer, &bufferIndex, "%3E", pError);

				break;
			case 92: /* ascii code for \ */ 

				_rsslAllocateAndConvertBufferForUrlData(inputBuffer, idx, &allocatedBuffer, &bufferIndex, "%5C", pError);

				break;
			case 94: // ascii code for ^

				_rsslAllocateAndConvertBufferForUrlData(inputBuffer, idx, &allocatedBuffer, &bufferIndex, "%5E", pError);

				break;
			case 96: // ascii code for `

				_rsslAllocateAndConvertBufferForUrlData(inputBuffer, idx, &allocatedBuffer, &bufferIndex, "%60", pError);

				break;
			case 123: // ascii code for {

				_rsslAllocateAndConvertBufferForUrlData(inputBuffer, idx, &allocatedBuffer, &bufferIndex, "%7B", pError);

				break;
			case 124: // ascii code for |

				_rsslAllocateAndConvertBufferForUrlData(inputBuffer, idx, &allocatedBuffer, &bufferIndex, "%7C", pError);

				break;
			case 125: // ascii code for }

				_rsslAllocateAndConvertBufferForUrlData(inputBuffer, idx, &allocatedBuffer, &bufferIndex, "%7D", pError);

				break;
			default:
				
				/* Copies the original input character */
				if (allocatedBuffer)
				{
					++bufferIndex;
					strncpy(allocatedBuffer->data + bufferIndex, inputBuffer->data + idx, 1);
				}

				break;
		}

		if (pError->rsslErrorId == RSSL_RET_FAILURE)
			return inputBuffer;
	}

	if (allocatedBuffer)
	{
		allocatedBuffer->length = (RsslUInt32)strlen(allocatedBuffer->data);
		return allocatedBuffer;
	}
	else
		return inputBuffer;
}

RsslRet rsslRestParseServiceDiscoveryResp(RsslBuffer* dataBody, RsslRestServiceEndpointResp* pRestServiceEndpointResp, RsslBuffer* memoryBuffer, RsslError* pError)
{
	cJSON* root;
	cJSON* items;
	cJSON* value;
	cJSON* serviceitem;
	cJSON* arrayValue;
	RsslUInt32 idx;
	RsslUInt32 arrayIdx;
	RsslBuffer memoryBlock;
	RsslRestBufferImpl rsslRestBufferImpl;
	RsslRestServiceEndpointInfo *pRsslRestServiceEndpointInfo;

	_rsslClearRestBufferImpl(&rsslRestBufferImpl);
	memset(memoryBuffer->data, 0, memoryBuffer->length);
	_rsslRestSetToRsslRestBufferImpl(memoryBuffer, &rsslRestBufferImpl);

	root = cJSON_Parse(dataBody->data);

	if (root == 0)
	{
		pError->rsslErrorId = RSSL_RET_INVALID_ARGUMENT;
		snprintf(pError->text, MAX_RSSL_ERROR_TEXT,
			"<%s:%d> Error: rsslRestParseServiceDiscoveryResp() failed because the passed in data body is not a valid JSON format.",
			__FILE__, __LINE__);

		return RSSL_RET_INVALID_ARGUMENT;
	}

	items = cJSON_GetObjectItem(root, "services");

	rsslClearRestServiceEndpointResp(pRestServiceEndpointResp);

	if (items && ((pRestServiceEndpointResp->serviceEndpointInfoCount = cJSON_GetArraySize(items)) > 0))
	{
		memoryBlock.length = pRestServiceEndpointResp->serviceEndpointInfoCount * sizeof(RsslRestServiceEndpointInfo);
		if (_rsslRestGetBuffer(&memoryBlock, memoryBlock.length, &rsslRestBufferImpl) != RSSL_RET_SUCCESS)
		{
			goto Fail;
		}

		pRestServiceEndpointResp->serviceEndpointInfoList = (RsslRestServiceEndpointInfo*)memoryBlock.data;

		for (idx = 0; idx < pRestServiceEndpointResp->serviceEndpointInfoCount; idx++)
		{
			serviceitem = cJSON_GetArrayItem(items, idx);

			if (serviceitem)
			{
				pRsslRestServiceEndpointInfo = &pRestServiceEndpointResp->serviceEndpointInfoList[idx];
				rsslClearRestServiceEndpointInfo(pRsslRestServiceEndpointInfo);

				value = cJSON_GetObjectItem(serviceitem, "location");

				if (value && ((pRsslRestServiceEndpointInfo->locationCount = cJSON_GetArraySize(value)) > 0))
				{
					memoryBlock.length = pRsslRestServiceEndpointInfo->locationCount * sizeof(RsslBuffer);
					if (_rsslRestGetBuffer(&memoryBlock, memoryBlock.length, &rsslRestBufferImpl) != RSSL_RET_SUCCESS)
					{
						goto Fail;
					}

					pRsslRestServiceEndpointInfo->locationList = (RsslBuffer*)memoryBlock.data;
					for (arrayIdx = 0; arrayIdx < pRsslRestServiceEndpointInfo->locationCount; arrayIdx++)
					{
						rsslClearBuffer(&pRsslRestServiceEndpointInfo->locationList[arrayIdx]);
						arrayValue = cJSON_GetArrayItem(value, arrayIdx);

						if (arrayValue)
						{
							pRsslRestServiceEndpointInfo->locationList[arrayIdx].length = (RsslUInt32)strlen(arrayValue->valuestring);
							if (_rsslRestGetBuffer(&pRsslRestServiceEndpointInfo->locationList[arrayIdx], 
								pRsslRestServiceEndpointInfo->locationList[arrayIdx].length + 1, &rsslRestBufferImpl) != RSSL_RET_SUCCESS)
							{
								goto Fail;
							}

							pRsslRestServiceEndpointInfo->locationList[arrayIdx].data[pRsslRestServiceEndpointInfo->locationList[arrayIdx].length] = '\0';
							strncpy(pRsslRestServiceEndpointInfo->locationList[arrayIdx].data, arrayValue->valuestring, pRsslRestServiceEndpointInfo->locationList[arrayIdx].length);
						}
					}
				}

				value = cJSON_GetObjectItem(serviceitem, "dataFormat");

				if (value && ((pRsslRestServiceEndpointInfo->dataFormatCount = cJSON_GetArraySize(value)) > 0))
				{
					memoryBlock.length = pRsslRestServiceEndpointInfo->dataFormatCount * sizeof(RsslBuffer);
					if (_rsslRestGetBuffer(&memoryBlock, memoryBlock.length, &rsslRestBufferImpl) != RSSL_RET_SUCCESS)
					{
						goto Fail;
					}

					pRsslRestServiceEndpointInfo->dataFormatList = (RsslBuffer*)memoryBlock.data;
					for (arrayIdx = 0; arrayIdx < pRsslRestServiceEndpointInfo->dataFormatCount; arrayIdx++)
					{
						rsslClearBuffer(&pRsslRestServiceEndpointInfo->dataFormatList[arrayIdx]);
						arrayValue = cJSON_GetArrayItem(value, arrayIdx);

						if (arrayValue)
						{
							pRsslRestServiceEndpointInfo->dataFormatList[arrayIdx].length = (RsslUInt32)strlen(arrayValue->valuestring);
							if (_rsslRestGetBuffer(&pRsslRestServiceEndpointInfo->dataFormatList[arrayIdx], 
								pRsslRestServiceEndpointInfo->dataFormatList[arrayIdx].length + 1, &rsslRestBufferImpl) != RSSL_RET_SUCCESS)
							{
								goto Fail;
							}

							pRsslRestServiceEndpointInfo->dataFormatList[arrayIdx].data[pRsslRestServiceEndpointInfo->dataFormatList[arrayIdx].length] = '\0';
							strncpy(pRsslRestServiceEndpointInfo->dataFormatList[arrayIdx].data, arrayValue->valuestring, pRsslRestServiceEndpointInfo->dataFormatList[arrayIdx].length);
						}
					}
				}

				value = cJSON_GetObjectItem(serviceitem, "transport");
				if (value) {
					pRsslRestServiceEndpointInfo->transport.length = (RsslUInt32)strlen(value->valuestring);
					if (_rsslRestGetBuffer(&pRsslRestServiceEndpointInfo->transport, pRsslRestServiceEndpointInfo->transport.length + 1, 
								&rsslRestBufferImpl) != RSSL_RET_SUCCESS)
					{
						goto Fail;
					}

					pRsslRestServiceEndpointInfo->transport.data[pRsslRestServiceEndpointInfo->transport.length] = '\0';
					strncpy(pRsslRestServiceEndpointInfo->transport.data, value->valuestring, pRsslRestServiceEndpointInfo->transport.length);
				}

				value = cJSON_GetObjectItem(serviceitem, "provider");
				if (value) {
					pRsslRestServiceEndpointInfo->provider.length = (RsslUInt32)strlen(value->valuestring);
					if (_rsslRestGetBuffer(&pRsslRestServiceEndpointInfo->provider, pRsslRestServiceEndpointInfo->provider.length + 1, 
								&rsslRestBufferImpl) != RSSL_RET_SUCCESS)
					{
						goto Fail;
					}

					pRsslRestServiceEndpointInfo->provider.data[pRsslRestServiceEndpointInfo->provider.length] = '\0';
					strncpy(pRsslRestServiceEndpointInfo->provider.data, value->valuestring, pRsslRestServiceEndpointInfo->provider.length);
				}

				value = cJSON_GetObjectItem(serviceitem, "endpoint");
				if (value) {
					pRsslRestServiceEndpointInfo->endPoint.length = (RsslUInt32)strlen(value->valuestring);
					if (_rsslRestGetBuffer(&pRsslRestServiceEndpointInfo->endPoint, pRsslRestServiceEndpointInfo->endPoint.length + 1, 
								&rsslRestBufferImpl) != RSSL_RET_SUCCESS)
					{
						goto Fail;
					}

					pRsslRestServiceEndpointInfo->endPoint.data[pRsslRestServiceEndpointInfo->endPoint.length] = '\0';
					strncpy(pRsslRestServiceEndpointInfo->endPoint.data, value->valuestring, pRsslRestServiceEndpointInfo->endPoint.length);
				}

				value = cJSON_GetObjectItem(serviceitem, "port");
				if (value) {
					pRsslRestServiceEndpointInfo->port.length = 5; // Max port number + null termination
					if (_rsslRestGetBuffer(&pRsslRestServiceEndpointInfo->port, pRsslRestServiceEndpointInfo->port.length + 1, &rsslRestBufferImpl) != RSSL_RET_SUCCESS)
					{
						goto Fail;
					}

					pRsslRestServiceEndpointInfo->port.length = (RsslUInt32)sprintf(pRsslRestServiceEndpointInfo->port.data, "%d", value->valueint);
				}
			}
		}
	}

	cJSON_Delete(root);
	return RSSL_RET_SUCCESS;

Fail:
	pError->rsslErrorId = RSSL_RET_BUFFER_TOO_SMALL;
	snprintf(pError->text, MAX_RSSL_ERROR_TEXT,
		"<%s:%d> Error: rsslRestParseServiceDiscoveryResp() failed because the memory buffer size is not enough to get service endpoint information.",
		__FILE__, __LINE__);

	cJSON_Delete(root);
	return RSSL_RET_BUFFER_TOO_SMALL;
}

RsslRet rsslRestParseEndpoint(RsslBuffer* dataBody, RsslBuffer* pLocation, RsslBuffer* pHostName, RsslBuffer* pPort, RsslBuffer* memoryBuffer, RsslError* pError)
{
	cJSON* root;
	cJSON* items;
	cJSON* location;
	cJSON* value;
	cJSON* arrayValue;
	RsslInt32 idx;
	RsslRestBufferImpl rsslRestBufferImpl;
	char* endpoint = NULL;
	RsslInt32 port = 0;

	rsslClearBuffer(pHostName);
	rsslClearBuffer(pPort);

	_rsslClearRestBufferImpl(&rsslRestBufferImpl);
	memset(memoryBuffer->data, 0, memoryBuffer->length);
	_rsslRestSetToRsslRestBufferImpl(memoryBuffer, &rsslRestBufferImpl);

	root = cJSON_Parse(dataBody->data);

	if (root == 0)
	{
		pError->rsslErrorId = RSSL_RET_INVALID_ARGUMENT;
		snprintf(pError->text, MAX_RSSL_ERROR_TEXT,
		"<%s:%d> Error: rsslRestParseEndpoint() failed because the passed in data body is not a valid JSON format.",
		__FILE__, __LINE__);

		return RSSL_RET_INVALID_ARGUMENT;
	}

	items = cJSON_GetObjectItem(root, "services");

	for (idx = 0; idx < cJSON_GetArraySize(items); idx++)
	{
		cJSON * subitem = cJSON_GetArrayItem(items, idx);
		location = cJSON_GetObjectItem(subitem, "location");

		if (cJSON_GetArraySize(location) >= 2)
		{
			arrayValue = cJSON_GetArrayItem(location, 0);

			if (arrayValue && (strncmp(pLocation->data, arrayValue->valuestring, strlen(pLocation->data)) == 0))
			{
				value = cJSON_GetObjectItem(subitem, "endpoint");

				if (value)
					endpoint = value->valuestring;
				else
					break;

				value = cJSON_GetObjectItem(subitem, "port");

				if (value)
					port = value->valueint;
				else
					break;

				break;
			}
		}
		else if (endpoint == NULL && port == 0 && cJSON_GetArraySize(location) > 0)
		{
			arrayValue = cJSON_GetArrayItem(location, 0);

			if (arrayValue && (strncmp(pLocation->data, arrayValue->valuestring, strlen(pLocation->data)) == 0))
			{
				value = cJSON_GetObjectItem(subitem, "endpoint");

				if (value)
					endpoint = value->valuestring;
				else
					continue;

				value = cJSON_GetObjectItem(subitem, "port");

				if (value)
					port = value->valueint;
				else
					endpoint = NULL;
			}
		}
	}

	if (endpoint != NULL && port != 0)
	{
		pHostName->length = (RsslUInt32)strlen(endpoint);
		if (_rsslRestGetBuffer(pHostName, pHostName->length + 1, &rsslRestBufferImpl) != RSSL_RET_SUCCESS)
		{
			goto Fail;
		}

		pHostName->data[pHostName->length] = '\0';
		strncpy(pHostName->data, endpoint, pHostName->length);

		pPort->length = 5;
		if (_rsslRestGetBuffer(pPort, pPort->length + 1, &rsslRestBufferImpl) != RSSL_RET_SUCCESS)
		{
			goto Fail;
		}

		pPort->length = (RsslUInt32)sprintf(pPort->data, "%d", port);
	}

	if ( (pHostName->length == 0) || (pPort->length == 0) )
	{
		pError->rsslErrorId = RSSL_RET_INVALID_ARGUMENT;
		snprintf(pError->text, MAX_RSSL_ERROR_TEXT,
			"<%s:%d> Error: rsslRestParseEndpoint() failed to parse host name and port due to field name not found.",
			__FILE__, __LINE__);
		cJSON_Delete(root);
		return RSSL_RET_INVALID_ARGUMENT;
	}

	cJSON_Delete(root);
	return RSSL_RET_SUCCESS;

Fail:
	pError->rsslErrorId = RSSL_RET_BUFFER_TOO_SMALL;
	snprintf(pError->text, MAX_RSSL_ERROR_TEXT,
		"<%s:%d> Error: rsslRestParseEndpoint() failed because the memory buffer size is not enough to get endpoint information.",
		__FILE__, __LINE__);
	
	cJSON_Delete(root);
	return RSSL_RET_BUFFER_TOO_SMALL;
}

RsslRet rsslRestParseAccessTokenV1(RsslBuffer* dataBody, RsslBuffer *accessToken, RsslBuffer *refreshToken, RsslInt *expiresIn,
									RsslBuffer *tokenType, RsslBuffer *scope, RsslBuffer* memoryBuffer, RsslError* pError)
{
	cJSON* root;
	cJSON* value;
	char* tokenvalue[5];
	RsslBool parseError = RSSL_FALSE;
	RsslRestBufferImpl rsslRestBufferImpl;
	int idx;

	root = cJSON_Parse(dataBody->data);

	if (!root)
	{
		pError->rsslErrorId = RSSL_RET_INVALID_ARGUMENT;
		snprintf(pError->text, MAX_RSSL_ERROR_TEXT,
		"<%s:%d> Error: rsslRestParseAccessToken() failed because the passed in data body is not a valid JSON format.",
		__FILE__, __LINE__);

		return RSSL_RET_INVALID_ARGUMENT;
	}

	for (idx = 0; idx < 5; idx++)
	{
		value = cJSON_GetObjectItem(root, tokenkeysV1[idx]);

		if (value)
			tokenvalue[idx] = value->valuestring;
		else
		{
			parseError = RSSL_TRUE;
			break;
		}
	}

	if (parseError)
	{
		pError->rsslErrorId = RSSL_RET_INVALID_ARGUMENT;
		snprintf(pError->text, MAX_RSSL_ERROR_TEXT,
			"<%s:%d> Error: rsslRestParseAccessToken() failed to parse token information due to field name not found.",
			__FILE__, __LINE__);
		cJSON_Delete(root);
		return RSSL_RET_INVALID_ARGUMENT;
	}

	_rsslClearRestBufferImpl(&rsslRestBufferImpl);
	_rsslRestSetToRsslRestBufferImpl( memoryBuffer, &rsslRestBufferImpl );

	accessToken->length = (RsslUInt32)strlen(tokenvalue[0]);
	if ( _rsslRestGetBuffer( accessToken, accessToken->length + 1, &rsslRestBufferImpl ) != RSSL_RET_SUCCESS )
	{
		goto Fail;
	}

	accessToken->data[accessToken->length] = '\0';
	strncpy(accessToken->data, tokenvalue[0], accessToken->length);

	refreshToken->length = (RsslUInt32)strlen(tokenvalue[1]);
	if ( _rsslRestGetBuffer( refreshToken, refreshToken->length + 1, &rsslRestBufferImpl ) != RSSL_RET_SUCCESS )
	{
		goto Fail;
	}

	refreshToken->data[refreshToken->length] = '\0';
	strncpy(refreshToken->data, tokenvalue[1], refreshToken->length);

	(*expiresIn) = atol(tokenvalue[2]);

	scope->length = (RsslUInt32)strlen(tokenvalue[3]);
	if ( _rsslRestGetBuffer( scope, scope->length + 1, &rsslRestBufferImpl ) != RSSL_RET_SUCCESS )
	{
		goto Fail;
	}

	scope->data[scope->length] = '\0';
	strncpy(scope->data, tokenvalue[3], scope->length);

	tokenType->length = (RsslUInt32)strlen(tokenvalue[4]);
	if ( _rsslRestGetBuffer( tokenType, tokenType->length + 1, &rsslRestBufferImpl ) != RSSL_RET_SUCCESS )
	{
		goto Fail;
	}

	tokenType->data[tokenType->length] = '\0';
	strncpy(tokenType->data, tokenvalue[4], tokenType->length);

	cJSON_Delete(root);
	return RSSL_RET_SUCCESS;

Fail:
	pError->rsslErrorId = RSSL_RET_BUFFER_TOO_SMALL;
	snprintf(pError->text, MAX_RSSL_ERROR_TEXT,
	"<%s:%d> Error: rsslRestParseAccessToken() failed because the memory buffer size is not enough to populate token information.",
	__FILE__, __LINE__);
	
	cJSON_Delete(root);
	return RSSL_RET_BUFFER_TOO_SMALL;
}


RsslUInt32 dumpTimestamp(char* buf, RsslUInt32 size)
{
	long hour = 0,
		min = 0,
		sec = 0,
		msec = 0;

	RsslUInt32 res = 0;

#if defined(WIN32)
	struct _timeb	_time;
	_ftime(&_time);
	sec = (long)(_time.time - 60 * (_time.timezone - _time.dstflag * 60));
	min = sec / 60 % 60;
	hour = sec / 3600 % 24;
	sec = sec % 60;
	msec = _time.millitm;
#elif defined(LINUX)
	/* localtime must be used to get the correct system time. */
	struct tm stamptime;
	time_t currTime;
	currTime = time(NULL);
	stamptime = *localtime_r(&currTime, &stamptime);
	sec = stamptime.tm_sec;
	min = stamptime.tm_min;
	hour = stamptime.tm_hour;

	/* localtime, however, does not give us msec. */
	struct timeval tv;
	gettimeofday(&tv, NULL);
	msec = tv.tv_usec / 1000;
#endif

	res = snprintf(buf, size, "<!-- Time: %ld:%02ld:%02ld:%03ld -->\n",
		hour,
		min,
		sec,
		msec);
	return res;
}

// Constants for Rest logging methods
static RsslBuffer headerRestRequest			= { 23, "\n--- REST REQUEST ---\n\n" };
static RsslBuffer rest_password_text		= {  9, "password=" };
static RsslBuffer rest_new_password_text	= { 12, "newPassword=" };
static RsslBuffer rest_client_secret_text	= { 14, "client_secret=" };
static RsslBuffer sUrl						= {  5, "URL: " };
static RsslBuffer sHttpMethod				= { 13, "HTTP method: " };
static RsslBuffer sHttpHeaderData			= { 18, "HTTP header data:\n" };
static RsslBuffer sHttpBodyData				= { 16, "HTTP body data: " };
static RsslBuffer sInterfaceName			= { 16, "Interface name: " };
static RsslBuffer sProxyDomainName			= { 19, "Proxy domain name: " };
static RsslBuffer sProxyHostName			= { 17, "Proxy host name: " };
static RsslBuffer sProxyPort				= { 12, "Proxy port: " };
static RsslBuffer sRequestTimeout			= { 17, "Request timeout: " };

static RsslBuffer headerRestResponse		= { 24, "\n--- REST RESPONSE ---\n\n" };
static RsslBuffer sProtocolVersion			= { 18, "Protocol version: " };
static RsslBuffer sHttpStatusCode			= { 18, "HTTP status code: " };

static RsslBuffer headerRestErr				= { 29, "\n--- REST RESPONSE ERROR---\n\n" };
static RsslBuffer sError					= {  7, "Error: " };
static RsslBuffer sErrorID					= { 11, " Error ID: " };
static RsslBuffer sSystemError				= { 15, " System error: " };

static const RsslUInt32 maxHttpMethodLength = 7U;
static const RsslUInt32 szDumpTimestampLength = 64U;

///////////////////////////
// Logging RestRequest
///////////////////////////

static RsslUInt32 estimateSizeRestRequestDump(RsslRestRequestArgs* pRestRequest)
{
	RsslQueueLink* pLink = NULL;
	RsslQueue* httpHeaders = NULL;
	RsslUInt32 n = 0, nameLength = 0;
	RsslRestHeader* httpHeaderData = NULL;

	//fprintf(pOutputStream, "\n--- REST REQUEST ---\n\n");
	RsslUInt32 sizeRestRequest = headerRestRequest.length;
	sizeRestRequest += szDumpTimestampLength;  // xmlDumpTimestamp()
	
	//fprintf(pOutputStream, "URL: %s\n", pRestRequest->url.data);
	sizeRestRequest += sUrl.length + pRestRequest->url.length + 1U;

	//fprintf(pOutputStream, "HTTP method %s\n", httpMethod[(RsslInt32)pRestRequest->httpMethod]);
	sizeRestRequest += sHttpMethod.length + maxHttpMethodLength + 1U;

	//fprintf(pOutputStream, "HTTP header data: \n");
	sizeRestRequest += sHttpHeaderData.length;

	httpHeaders = &(pRestRequest->httpHeaders);
	for (pLink = rsslQueuePeekFront(httpHeaders), n = 0; pLink; pLink = rsslQueuePeekNext(httpHeaders, pLink), n++)
	{
		httpHeaderData = RSSL_QUEUE_LINK_TO_OBJECT(RsslRestHeader, queueLink, pLink);
		//fprintf(pOutputStream, " %s : ", httpHeaderData->key.data);
		sizeRestRequest += httpHeaderData->key.length + 4U;
		//fprintf(pOutputStream, "%s\n", httpHeaderData->value.data);
		sizeRestRequest += httpHeaderData->value.length + 1U;
	}

	//fprintf(pOutputStream, "HTTP body data: ");
	sizeRestRequest += sHttpBodyData.length;
	sizeRestRequest += pRestRequest->httpBody.length + 1U;

	//fprintf(pOutputStream, "Interface name: %s\n", pRestRequest->networkArgs.interfaceName.data);
	if (pRestRequest->networkArgs.interfaceName.length > 0)
		sizeRestRequest += sInterfaceName.length + pRestRequest->networkArgs.interfaceName.length + 1U;

	//fprintf(pOutputStream, "Proxy domain name: %s\n", pRestRequest->networkArgs.proxyArgs.proxyDomain.data);
	if (pRestRequest->networkArgs.proxyArgs.proxyDomain.length > 0)
		sizeRestRequest += sProxyDomainName.length + pRestRequest->networkArgs.proxyArgs.proxyDomain.length + 1U;

	//fprintf(pOutputStream, "Proxy host name: %s\n", pRestRequest->networkArgs.proxyArgs.proxyHostName.data);
	if (pRestRequest->networkArgs.proxyArgs.proxyHostName.length > 0)
		sizeRestRequest += sProxyHostName.length + pRestRequest->networkArgs.proxyArgs.proxyHostName.length + 1U;

	//fprintf(pOutputStream, "Proxy port: %s\n", pRestRequest->networkArgs.proxyArgs.proxyPort.data);
	if (pRestRequest->networkArgs.proxyArgs.proxyPort.length > 0)
		sizeRestRequest += sProxyPort.length + pRestRequest->networkArgs.proxyArgs.proxyPort.length + 1U;

	//fprintf(pOutputStream, "Request timeout: %d\n", pRestRequest->requestTimeOut);
	sizeRestRequest += sRequestTimeout.length + 9U + 1U;

	sizeRestRequest += 64U;

	return sizeRestRequest;
}

RsslBuffer* rsslRestRequestDumpBuffer(RsslRestRequestArgs* pRestRequest, RsslError* pError)
{
	RsslQueueLink* pLink = NULL;
	RsslQueue* httpHeaders = NULL;
	RsslRestHeader* httpHeaderData = NULL;
	RsslUInt32 n = 0, nameLength = 0;
	const char endSymbol = '&';
	const char* httpMethod[6] = { "UNKNOWN", "GET", "POST", "PUT", "DELETE", "PATCH" }; // from RsslRestHttpMethod

	RsslUInt32 sizeRestRequest = 0;
	RsslBuffer* restRequestBuffer = NULL;
	char* data = NULL;
	RsslUInt32 pos = 0;

	if (!pRestRequest)
	{
		pError->rsslErrorId = RSSL_RET_INVALID_ARGUMENT;
		snprintf(pError->text, MAX_RSSL_ERROR_TEXT,
			"<%s:%d> Error: rsslRestRequestDumpBuffer() failed because REST request argument is empty.",
			__FILE__, __LINE__);
		return NULL;
	}

	sizeRestRequest = estimateSizeRestRequestDump(pRestRequest);

	restRequestBuffer = (RsslBuffer*)malloc(sizeof(RsslBuffer));
	if (restRequestBuffer == NULL)
	{
		pError->rsslErrorId = RSSL_RET_FAILURE;
		snprintf(pError->text, MAX_RSSL_ERROR_TEXT,
			"<%s:%d> Error: rsslRestRequestDumpBuffer() failed to allocate memory RsslBuffer.", __FILE__, __LINE__);
		return NULL;
	}

	data = (char*)malloc(sizeRestRequest);
	if (data == NULL)
	{
		free(restRequestBuffer);
		pError->rsslErrorId = RSSL_RET_FAILURE;
		snprintf(pError->text, MAX_RSSL_ERROR_TEXT,
			"<%s:%d> Error: rsslRestRequestDumpBuffer() failed to allocate memory data (%u).", __FILE__, __LINE__, sizeRestRequest);
		return NULL;
	}

	//fprintf(pOutputStream, "\n--- REST REQUEST ---\n\n");
	pos += snprintf(data + pos, (sizeRestRequest - pos), headerRestRequest.data);

	//xmlDumpTimestamp(pOutputStream);
	pos += dumpTimestamp(data + pos, (sizeRestRequest - pos));

	//fprintf(pOutputStream, "URL: %s\n", pRestRequest->url.data);
	pos += snprintf(data + pos, (sizeRestRequest - pos), "%s%s\n", sUrl.data, pRestRequest->url.data);

	//fprintf(pOutputStream, "HTTP method %s\n", httpMethod[(RsslInt32)pRestRequest->httpMethod]);
	pos += snprintf(data + pos, (sizeRestRequest - pos), "%s%s\n", sHttpMethod.data, httpMethod[(RsslInt32)pRestRequest->httpMethod]);

	//fprintf(pOutputStream, "HTTP header data: \n");
	pos += snprintf(data + pos, (sizeRestRequest - pos), "%s", sHttpHeaderData.data);

	httpHeaders = &(pRestRequest->httpHeaders);
	for (pLink = rsslQueuePeekFront(httpHeaders), n = 0; pLink; pLink = rsslQueuePeekNext(httpHeaders, pLink), n++)
	{
		httpHeaderData = RSSL_QUEUE_LINK_TO_OBJECT(RsslRestHeader, queueLink, pLink);
		//fprintf(pOutputStream, " %s : ", httpHeaderData->key.data);
		pos += snprintf(data + pos, (sizeRestRequest - pos), " %s : ", httpHeaderData->key.data);

		//fprintf(pOutputStream, "%s\n", httpHeaderData->value.data);
		pos += snprintf(data + pos, (sizeRestRequest - pos), "%s\n", httpHeaderData->value.data);
	}

	//fprintf(pOutputStream, "HTTP body data: ");
	pos += snprintf(data + pos, (sizeRestRequest - pos), "%s", sHttpBodyData.data);

	/* Cut off all confident information: username, newPassword, client_secret */
	for (n = 0; n < pRestRequest->httpBody.length && pos < sizeRestRequest; n++)
	{
		if ((strncmp(pRestRequest->httpBody.data + n, rest_password_text.data, nameLength = rest_password_text.length) == 0) ||
			(strncmp((pRestRequest->httpBody.data) + n, rest_new_password_text.data, nameLength = rest_new_password_text.length) == 0) ||
			(strncmp((pRestRequest->httpBody.data) + n, rest_client_secret_text.data, nameLength = rest_client_secret_text.length) == 0))
		{
			n += nameLength;
			while (n++, (n < pRestRequest->httpBody.length && pRestRequest->httpBody.data[n] != endSymbol));
		}
		else
		{
			//fprintf(pOutputStream, "%c", pRestRequest->httpBody.data[n]);
			*(data + pos) = pRestRequest->httpBody.data[n];
			++pos;
		}
	}

	//fprintf(pOutputStream, "\n");
	if (pos < sizeRestRequest)
	{
		*(data + pos) = '\n';
		++pos;
	}

	if (pRestRequest->networkArgs.interfaceName.length > 0)
	{
		//fprintf(pOutputStream, "Interface name: %s\n", pRestRequest->networkArgs.interfaceName.data);
		pos += snprintf(data + pos, (sizeRestRequest - pos), "%s%s\n", sInterfaceName.data, pRestRequest->networkArgs.interfaceName.data);
	}
	if (pRestRequest->networkArgs.proxyArgs.proxyDomain.length > 0)
	{
		//fprintf(pOutputStream, "Proxy domain name: %s\n", pRestRequest->networkArgs.proxyArgs.proxyDomain.data);
		pos += snprintf(data + pos, (sizeRestRequest - pos), "%s%s\n", sProxyDomainName.data, pRestRequest->networkArgs.proxyArgs.proxyDomain.data);
	}
	if (pRestRequest->networkArgs.proxyArgs.proxyHostName.length > 0)
	{
		//fprintf(pOutputStream, "Proxy host name: %s\n", pRestRequest->networkArgs.proxyArgs.proxyHostName.data);
		pos += snprintf(data + pos, (sizeRestRequest - pos), "%s%s\n", sProxyHostName.data, pRestRequest->networkArgs.proxyArgs.proxyHostName.data);
	}
	if (pRestRequest->networkArgs.proxyArgs.proxyPort.length > 0)
	{
		//fprintf(pOutputStream, "Proxy port: %s\n", pRestRequest->networkArgs.proxyArgs.proxyPort.data);
		pos += snprintf(data + pos, (sizeRestRequest - pos), "%s%s\n", sProxyPort.data, pRestRequest->networkArgs.proxyArgs.proxyPort.data);
	}

	//fprintf(pOutputStream, "Request timeout: %d\n", pRestRequest->requestTimeOut);
	pos += snprintf(data + pos, (sizeRestRequest - pos), "%s%d\n", sRequestTimeout.data, pRestRequest->requestTimeOut);

	restRequestBuffer->data = data;
	restRequestBuffer->length = pos;

	return restRequestBuffer;
}


///////////////////////////
// Logging RestResponse
///////////////////////////

static RsslUInt32 estimateSizeRestResponseDump(RsslRestResponse* pRestResponse)
{
	RsslQueueLink* pLink = NULL;
	RsslQueue* httpHeaders = NULL;
	RsslRestHeader* httpHeaderData = NULL;
	RsslInt32 n = 0;

	//fprintf(pOutputStream, "\n--- REST RESPONSE ---\n\n");
	RsslUInt32 sizeRestResponse = headerRestResponse.length;
	sizeRestResponse += szDumpTimestampLength;  // xmlDumpTimestamp()

	//fprintf(pOutputStream, "HTTP header data:\n");
	sizeRestResponse += sHttpHeaderData.length;

	httpHeaders = &(pRestResponse->headers);
	for (pLink = rsslQueuePeekFront(httpHeaders), n = 0; pLink; pLink = rsslQueuePeekNext(httpHeaders, pLink), n++)
	{
		httpHeaderData = RSSL_QUEUE_LINK_TO_OBJECT(RsslRestHeader, queueLink, pLink);
		//fprintf(pOutputStream, " %s : ", httpHeaderData->key.data);
		sizeRestResponse += httpHeaderData->key.length + 4U;
		//fprintf(pOutputStream, "%s\n", httpHeaderData->value.data);
		sizeRestResponse += httpHeaderData->value.length + 1U;
	}

	//fprintf(pOutputStream, "HTTP body data: %s\n", pRestResponse->dataBody.data);
	sizeRestResponse += sHttpBodyData.length + pRestResponse->dataBody.length + 1U;

	//fprintf(pOutputStream, "Protocol version: %s\n", pRestResponse->protocolVersion.data);
	sizeRestResponse += sProtocolVersion.length + pRestResponse->protocolVersion.length + 1U;

	//fprintf(pOutputStream, "HTTP status code: %d\n", pRestResponse->statusCode);
	sizeRestResponse += sHttpStatusCode.length + 9U + 1U;

	sizeRestResponse += 64U;

	return sizeRestResponse;
}

RsslBuffer* rsslRestResponseDumpBuffer(RsslRestResponse* pRestResponse, RsslError* pError)
{
	RsslQueueLink* pLink = NULL;
	RsslQueue* httpHeaders = NULL;
	RsslRestHeader* httpHeaderData = NULL;
	RsslInt32 n = 0;

	RsslUInt32 sizeRestResponse = 0;
	RsslBuffer* restResponseBuffer = NULL;
	char* data = NULL;
	RsslUInt32 pos = 0;

	if (!pRestResponse)
	{
		pError->rsslErrorId = RSSL_RET_INVALID_ARGUMENT;
		snprintf(pError->text, MAX_RSSL_ERROR_TEXT,
			"<%s:%d> Error: rsslRestResponseDumpBuffer() failed because REST response arguments is empty.",
			__FILE__, __LINE__);
		return NULL;
	}

	sizeRestResponse = estimateSizeRestResponseDump(pRestResponse);

	restResponseBuffer = (RsslBuffer*)malloc(sizeof(RsslBuffer));
	if (restResponseBuffer == NULL)
	{
		pError->rsslErrorId = RSSL_RET_FAILURE;
		snprintf(pError->text, MAX_RSSL_ERROR_TEXT,
			"<%s:%d> Error: rsslRestResponseDumpBuffer() failed to allocate memory RsslBuffer.", __FILE__, __LINE__);
		return NULL;
	}

	data = (char*)malloc(sizeRestResponse);
	if (data == NULL)
	{
		free(restResponseBuffer);
		pError->rsslErrorId = RSSL_RET_FAILURE;
		snprintf(pError->text, MAX_RSSL_ERROR_TEXT,
			"<%s:%d> Error: rsslRestResponseDumpBuffer() failed to allocate memory data (%u).", __FILE__, __LINE__, sizeRestResponse);
		return NULL;
	}

	//fprintf(pOutputStream, "\n--- REST RESPONSE ---\n\n");
	pos += snprintf(data + pos, (sizeRestResponse - pos), headerRestResponse.data);

	//xmlDumpTimestamp(pOutputStream);
	pos += dumpTimestamp(data + pos, (sizeRestResponse - pos));

	//fprintf(pOutputStream, "HTTP header data:\n");
	pos += snprintf(data + pos, (sizeRestResponse - pos), sHttpHeaderData.data);
	
	httpHeaders = &(pRestResponse->headers);
	for (pLink = rsslQueuePeekFront(httpHeaders), n = 0; pLink; pLink = rsslQueuePeekNext(httpHeaders, pLink), n++)
	{
		httpHeaderData = RSSL_QUEUE_LINK_TO_OBJECT(RsslRestHeader, queueLink, pLink);
		
		//fprintf(pOutputStream, " %s : ", httpHeaderData->key.data);
		pos += snprintf(data + pos, (sizeRestResponse - pos), " %s : ", httpHeaderData->key.data);

		//fprintf(pOutputStream, "%s\n", httpHeaderData->value.data);
		pos += snprintf(data + pos, (sizeRestResponse - pos), "%s\n", httpHeaderData->value.data);
	}

	//fprintf(pOutputStream, "HTTP body data: %s\n", pRestResponse->dataBody.data);
	pos += snprintf(data + pos, (sizeRestResponse - pos), "%s%s\n", sHttpBodyData.data, pRestResponse->dataBody.data);

	//fprintf(pOutputStream, "Protocol version: %s\n", pRestResponse->protocolVersion.data);
	pos += snprintf(data + pos, (sizeRestResponse - pos), "%s%s\n", sProtocolVersion.data, pRestResponse->protocolVersion.data);

	//fprintf(pOutputStream, "HTTP status code: %d\n", pRestResponse->statusCode);
	pos += snprintf(data + pos, (sizeRestResponse - pos), "%s%d\n", sHttpStatusCode.data, pRestResponse->statusCode);

	restResponseBuffer->data = data;
	restResponseBuffer->length = pos;

	return restResponseBuffer;
}


////////////////////////////////
// Logging Rest Response Error
////////////////////////////////

static RsslUInt32 estimateSizeRestResponseErrDump(RsslError* pErrorOutput)
{
	//fprintf(pOutputStream, "\n--- REST RESPONSE ERROR---\n\n");
	RsslUInt32 sizeRestErr = headerRestErr.length;
	sizeRestErr += szDumpTimestampLength;  // xmlDumpTimestamp()

	//fprintf(pOutputStream, "Error: %s\n Error ID: %d\n System error: %d\n", pErrorOutput->text,
	//	pErrorOutput->rsslErrorId, pErrorOutput->sysError);
	sizeRestErr += sError.length + (RsslUInt32)strlen(pErrorOutput->text) + 1U;
	sizeRestErr += sErrorID.length + 9U + 1U;
	sizeRestErr += sSystemError.length + 9U + 1U;

	sizeRestErr += 64U;

	return sizeRestErr;
}

RsslBuffer* rsslRestResponseErrDumpBuffer(RsslError* pErrorOutput)
{
	RsslUInt32 sizeRestResponseErr = 0;
	RsslBuffer* restResponseErrBuffer = NULL;
	char* data = NULL;
	RsslUInt32 pos = 0;

	if (!pErrorOutput)
	{
		return NULL;
	}

	sizeRestResponseErr = estimateSizeRestResponseErrDump(pErrorOutput);

	restResponseErrBuffer = (RsslBuffer*)malloc(sizeof(RsslBuffer));
	if (restResponseErrBuffer == NULL)
	{
		return NULL;
	}

	data = (char*)malloc(sizeRestResponseErr);
	if (data == NULL)
	{
		free(restResponseErrBuffer);
		return NULL;
	}

	//fprintf(pOutputStream, "\n--- REST RESPONSE ERROR---\n\n");
	pos += snprintf(data + pos, (sizeRestResponseErr - pos), headerRestErr.data);

	//xmlDumpTimestamp(pOutputStream);
	pos += dumpTimestamp(data + pos, (sizeRestResponseErr - pos));

	//fprintf(pOutputStream, "Error: %s\n Error ID: %d\n System error: %d\n", pErrorOutput->text,
	//	pErrorOutput->rsslErrorId, pErrorOutput->sysError);
	pos += snprintf(data + pos, (sizeRestResponseErr - pos), "%s%s\n%s%d\n%s%d\n",
		sError.data, pErrorOutput->text,
		sErrorID.data, pErrorOutput->rsslErrorId,
		sSystemError.data, pErrorOutput->sysError);

	restResponseErrBuffer->data = data;
	restResponseErrBuffer->length = pos;

	return restResponseErrBuffer;
}

RsslRet rsslRestParseAccessTokenV2(RsslBuffer* dataBody, RsslBuffer* accessToken, RsslBuffer* refreshToken, RsslInt* expiresIn,
	RsslBuffer* tokenType, RsslBuffer* scope, RsslBuffer* memoryBuffer, RsslError* pError)
{
	cJSON* root;
	cJSON* value;
	char* tokenvalue[3];
	RsslBool parseError = RSSL_FALSE;
	RsslRestBufferImpl rsslRestBufferImpl;
	int idx;

	root = cJSON_Parse(dataBody->data);

	if (!root)
	{
		pError->rsslErrorId = RSSL_RET_INVALID_ARGUMENT;
		snprintf(pError->text, MAX_RSSL_ERROR_TEXT,
			"<%s:%d> Error: rsslRestParseAccessToken() failed because the passed in data body is not a valid JSON format.",
			__FILE__, __LINE__);

		return RSSL_RET_INVALID_ARGUMENT;
	}

	for (idx = 0; idx < 3; idx++)
	{
		value = cJSON_GetObjectItem(root, tokenkeysV2[idx]);

		if (value)
		{
			if (idx == 1)
				(*expiresIn) = value->valueint;
			else
				tokenvalue[idx] = value->valuestring;
		}
		else
		{
			parseError = RSSL_TRUE;

			break;
		}
	}

	if (parseError)
	{
		pError->rsslErrorId = RSSL_RET_INVALID_ARGUMENT;
		snprintf(pError->text, MAX_RSSL_ERROR_TEXT,
			"<%s:%d> Error: rsslRestParseAccessToken() failed to parse token information due to field name not found.",
			__FILE__, __LINE__);
		cJSON_Delete(root);
		return RSSL_RET_INVALID_ARGUMENT;
	}

	_rsslClearRestBufferImpl(&rsslRestBufferImpl);
	_rsslRestSetToRsslRestBufferImpl(memoryBuffer, &rsslRestBufferImpl);

	accessToken->length = (RsslUInt32)strlen(tokenvalue[0]);
	if (_rsslRestGetBuffer(accessToken, accessToken->length + 1, &rsslRestBufferImpl) != RSSL_RET_SUCCESS)
	{
		goto Fail;
	}

	accessToken->data[accessToken->length] = '\0';
	strncpy(accessToken->data, tokenvalue[0], accessToken->length);

	tokenType->length = (RsslUInt32)strlen(tokenvalue[2]);
	if (_rsslRestGetBuffer(tokenType, tokenType->length + 1, &rsslRestBufferImpl) != RSSL_RET_SUCCESS)
	{
		goto Fail;
	}

	tokenType->data[tokenType->length] = '\0';
	strncpy(tokenType->data, tokenvalue[2], tokenType->length);

	cJSON_Delete(root);
	return RSSL_RET_SUCCESS;

Fail:
	pError->rsslErrorId = RSSL_RET_BUFFER_TOO_SMALL;
	snprintf(pError->text, MAX_RSSL_ERROR_TEXT,
		"<%s:%d> Error: rsslRestParseAccessToken() failed because the memory buffer size is not enough to populate token information.",
		__FILE__, __LINE__);

	cJSON_Delete(root);
	return RSSL_RET_BUFFER_TOO_SMALL;
}

OPENSSL_BIGNUM* rsslBase64URLToBigNum(RsslBuffer* in, ripcSSLApiFuncs* sslFuncs, ripcCryptoApiFuncs* cryptoFuncs, RsslError* error)
{
	OPENSSL_BIO* base64buf;
	OPENSSL_BIO* outbuf;
	OPENSSL_BIGNUM* bignumout;
	int i;
	int inputRemainder;
	int readOut = 0;
	char* tempBuf = malloc((size_t)in->length);
	RsslBuffer tempInputBuf;

	if (tempBuf == NULL)
	{
		return NULL;
	}
	memset(tempBuf, 0, (size_t)in->length);

	tempInputBuf.data = malloc((size_t)in->length + 3);
	if (tempInputBuf.data == NULL)
	{
		free(tempBuf);
		return NULL;
	}
	memset(tempInputBuf.data, 0, (size_t)in->length+3);
	memcpy(tempInputBuf.data, in->data, in->length);

	inputRemainder = in->length % 4;
	if (inputRemainder == 2)
	{
		tempInputBuf.data[in->length] = '=';
		tempInputBuf.data[in->length+1] = '=';
		tempInputBuf.length = in->length + 2;
	}
	else if (inputRemainder == 3)
	{
		tempInputBuf.data[in->length] = '=';
		tempInputBuf.length = in->length + 1;
	}
	else
	{
		tempInputBuf.length = in->length;
	}

	/* First, we need to convert the input buffer from Base64 URL to Base64*/

	for (i = 0; i < (int)in->length; i++)
	{
		if (tempInputBuf.data[i] == '-')
			tempInputBuf.data[i] = '+';
		else if (in->data[i] == '_')
			tempInputBuf.data[i] = '/';
	}

	/* Now setup a base64 BIO */
	base64buf = (*(sslFuncs->BIO_new))((*(sslFuncs->BIO_f_base64))());
	outbuf = (*(sslFuncs->BIO_new_mem_buf))(tempInputBuf.data, tempInputBuf.length);
	outbuf = (*(sslFuncs->BIO_push))(base64buf, outbuf);

	(*(sslFuncs->BIO_set_flags))(outbuf, RSSL_BIO_FLAGS_BASE64_NO_NL);

	/* BIO_set_close */
	(*(sslFuncs->BIO_ctrl))(outbuf, RSSL_BIO_CTRL_SET_CLOSE, RSSL_BIO_CLOSE, NULL);

	readOut = (*(sslFuncs->BIO_read))(outbuf, tempBuf, tempInputBuf.length);

	bignumout = (*(cryptoFuncs->bin2bn))(tempBuf, readOut, NULL);

	(*(sslFuncs->BIO_free_all))(outbuf);
	free(tempBuf);
	memset(tempInputBuf.data, 0, (size_t)in->length + 3);
	free(tempInputBuf.data);
	return bignumout;
}

