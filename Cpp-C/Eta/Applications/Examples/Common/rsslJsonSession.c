/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2019-2022 Refinitiv. All rights reserved.         --
 *|-----------------------------------------------------------------------------
 */

/*
 * This source code is provided under the Apache 2.0 license and is provided
 * AS IS with no warranty or guarantee of fit for purpose.  See the project's 
 * LICENSE.md for details. 
 * Copyright (C) 2020 Refinitiv. All rights reserved.
*/

#include "rsslJsonSession.h"
#include "rtr/rsslTransport.h"
#include "rtr/rsslMessagePackage.h"
#include <stdio.h>  
#include <stdlib.h>
#include <string.h>
#include <errno.h>


#define RSSL_MAX_JSON_ERROR_MSG_SIZE 1101

static RsslRet _rsslJsonSessionSendJsonMessage(RsslChannel *pChannel, RsslBuffer *pBuffer, RsslUInt8 wrtFlags, RsslError *pError)
{
	RsslUInt32  bWritten, ucbWritten;
	RsslError	error;	
	RsslRet		ret;

	ret = rsslWrite(pChannel, pBuffer, RSSL_HIGH_PRIORITY, wrtFlags, &bWritten, &ucbWritten, &error);

	while (ret == RSSL_RET_WRITE_CALL_AGAIN)
	{
		if ((ret = rsslFlush(pChannel, &error)) < RSSL_RET_SUCCESS)
		{
#if defined(__GNUC__) && (__GNUC__ >= 9)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wformat-truncation"
#endif
			snprintf(pError->text, MAX_RSSL_ERROR_TEXT, "rsslFlush() failed with return code %d - <%s>\n", ret, error.text);
#if defined(__GNUC__) && (__GNUC__ >= 9)
#pragma GCC diagnostic pop
#endif
			return ret;
		}
		ret = rsslWrite(pChannel, pBuffer, RSSL_HIGH_PRIORITY, wrtFlags, &bWritten, &ucbWritten, &error);
	}

	if (ret != RSSL_RET_SUCCESS)
	{
#if defined(__GNUC__) && (__GNUC__ >= 9)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wformat-truncation"
#endif
		snprintf(pError->text, MAX_RSSL_ERROR_TEXT, "rsslWrite() failed with return code %d - <%s>\n", ret, error.text);
#if defined(__GNUC__) && (__GNUC__ >= 9)
#pragma GCC diagnostic pop
#endif
	}

	return ret;
}

static RsslRet _rsslJsonSessionServiceNameToIdCallback(RsslBuffer *pServiceName, void *closure, RsslUInt16 *pServiceId)
{
	RsslJsonSession *sess = (RsslJsonSession*)closure;

	if (sess != NULL)
		return (*sess->options.pServiceNameToIdCallback)(pServiceName, pServiceId);
	else
		return RSSL_RET_FAILURE;
}

RsslRet rsslJsonSessionResetState(RsslJsonSession *sess, RsslBuffer *pBuffer, RsslError *pError)
{
	RsslRet	ret;
	RsslJsonConverterError error;
	

	if (pBuffer != NULL)
		sess->state.buffer = pBuffer;

	rsslClearParseJsonBufferOptions(&(sess->state.parseOptions));
	sess->state.parseOptions.jsonProtocolType = RSSL_JSON_PROTOCOL_TYPE;

	rsslClearDecodeJsonMsgOptions(&(sess->state.decodeOptions));
	sess->state.decodeOptions.jsonProtocolType = RSSL_JSON_PROTOCOL_TYPE;

	ret = rsslParseJsonBuffer(sess->pJsonConverter, &sess->state.parseOptions, 
								sess->state.buffer, &error);

#if defined(__GNUC__) && (__GNUC__ >= 9)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wformat-truncation"
#endif
	if (ret != RSSL_RET_SUCCESS)
		snprintf(pError->text, MAX_RSSL_ERROR_TEXT, "rjcError: %s", error.text);
#if defined(__GNUC__) && (__GNUC__ >= 9)
#pragma GCC diagnostic pop
#endif

	return ret;
}


void rsslJsonSessionUninitialize(RsslJsonSession *rjcSession)
{
	RsslJsonConverterError rjcError;

	if (rjcSession->pJsonConverter != 0)
		rsslDestroyRsslJsonConverter(rjcSession->pJsonConverter, &rjcError);

	if (rjcSession->pDictionaryList)
		free(rjcSession->pDictionaryList);

	rjcSession->jsonSessionInitialized = RSSL_FALSE;
}

RsslRet rsslJsonSessionInitialize(RsslJsonSession *rjcSession, RsslError *pError)
{
	RsslCreateJsonConverterOptions rjcOptions;
	RsslJsonConverterError rjcError;
	RsslBool flag = RSSL_TRUE;


	if (rjcSession->jsonSessionInitialized == RSSL_TRUE)
	{
		snprintf(pError->text, MAX_RSSL_ERROR_TEXT,	"This Json Session has been initialized");

		return RSSL_RET_FAILURE;
	}

	rsslClearCreateRsslJsonConverterOptions(&rjcOptions);
	
	rjcSession->pJsonConverter = rsslCreateRsslJsonConverter(&rjcOptions, &rjcError);
	if (rjcSession->pJsonConverter == NULL)
	{
#if defined(__GNUC__) && (__GNUC__ >= 9)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wformat-truncation"
#endif
		snprintf(pError->text, MAX_RSSL_ERROR_TEXT, "Failed to create RsslJsonConverter: %s", rjcError.text);
#if defined(__GNUC__) && (__GNUC__ >= 9)
#pragma GCC diagnostic pop
#endif
		return (rsslJsonSessionUninitialize(rjcSession), RSSL_RET_FAILURE);
	}
	
	if(rjcSession->options.pDictionary != NULL)
	{
		if(rsslJsonSessionSetDictionary(rjcSession, rjcSession->options.pDictionary, pError) != RSSL_RET_SUCCESS)
			return RSSL_RET_FAILURE;
	}

	/* Checks whether the callback method is set by users */
	if (rjcSession->options.pServiceNameToIdCallback)
	{
		RsslJsonServiceNameToIdCallbackProperty svcNameToIdCbProperty;

		rsslJsonClearServiceNameToIdCallbackProperty(&svcNameToIdCbProperty);

		svcNameToIdCbProperty.callback = _rsslJsonSessionServiceNameToIdCallback;
		svcNameToIdCbProperty.closure = rjcSession;

		/* Set service-name/ID callbacks. */
		if (rsslJsonConverterSetProperty(rjcSession->pJsonConverter,
			RSSL_JSON_CPC_SERVICE_NAME_TO_ID_CALLBACK,
			&svcNameToIdCbProperty, &rjcError) != RSSL_RET_SUCCESS)
		{
#if defined(__GNUC__) && (__GNUC__ >= 9)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wformat-truncation"
#endif
			snprintf(pError->text, MAX_RSSL_ERROR_TEXT, "Failed setting RsslJsonConverter property: service name to ID callback [%s]",
				rjcError.text);
#if defined(__GNUC__) && (__GNUC__ >= 9)
#pragma GCC diagnostic pop
#endif

			return (rsslJsonSessionUninitialize(rjcSession), RSSL_RET_FAILURE);
		}
	}

	/* Set default service ID. */
	if (rsslJsonConverterSetProperty(rjcSession->pJsonConverter,
								RSSL_JSON_CPC_DEFAULT_SERVICE_ID, 
								&rjcSession->options.defaultServiceId, &rjcError) != RSSL_RET_SUCCESS)
	{
#if defined(__GNUC__) && (__GNUC__ >= 9)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wformat-truncation"
#endif
		snprintf(pError->text, MAX_RSSL_ERROR_TEXT, "Failed setting RsslJsonConverter property: default service ID [%s]", rjcError.text);
#if defined(__GNUC__) && (__GNUC__ >= 9)
#pragma GCC diagnostic pop
#endif

		return (rsslJsonSessionUninitialize(rjcSession), RSSL_RET_FAILURE);
	}

	/* When converting from RWF to JSON, add a QoS range on requests that do not specify a QoS */
	flag = RSSL_TRUE;
	if (rsslJsonConverterSetProperty(rjcSession->pJsonConverter,
								RSSL_JSON_CPC_USE_DEFAULT_DYNAMIC_QOS, 
								&flag, &rjcError) != RSSL_RET_SUCCESS)
	{
#if defined(__GNUC__) && (__GNUC__ >= 9)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wformat-truncation"
#endif
		snprintf(pError->text, MAX_RSSL_ERROR_TEXT, "Failed setting RsslJsonConverter property: add default QoS range [%s]", rjcError.text);
#if defined(__GNUC__) && (__GNUC__ >= 9)
#pragma GCC diagnostic pop
#endif

		return (rsslJsonSessionUninitialize(rjcSession), RSSL_RET_FAILURE);
	}

	/* Expand enumerated values in field entries to their display values. 
	 * Dictionary must have enumerations loaded */
	flag = rjcSession->options.jsonExpandedEnumFields;
	if (rsslJsonConverterSetProperty(rjcSession->pJsonConverter,
								RSSL_JSON_CPC_EXPAND_ENUM_FIELDS, 
								&flag, &rjcError) != RSSL_RET_SUCCESS)
	{
#if defined(__GNUC__) && (__GNUC__ >= 9)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wformat-truncation"
#endif
		snprintf(pError->text, MAX_RSSL_ERROR_TEXT, "Failed setting RsslJsonConverter property: expand enum fields [%s]", rjcError.text);
#if defined(__GNUC__) && (__GNUC__ >= 9)
#pragma GCC diagnostic pop
#endif

		return (rsslJsonSessionUninitialize(rjcSession), RSSL_RET_FAILURE);
	}

	/* When converting from JSON to RWF, catch unknown JSON keys. */
	flag = RSSL_TRUE;
	if (rsslJsonConverterSetProperty(rjcSession->pJsonConverter,
								RSSL_JSON_CPC_CATCH_UNKNOWN_JSON_KEYS, 
								&flag, &rjcError) != RSSL_RET_SUCCESS)
	{
#if defined(__GNUC__) && (__GNUC__ >= 9)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wformat-truncation"
#endif
		snprintf(pError->text, MAX_RSSL_ERROR_TEXT, "Failed setting RsslJsonConverter property: catch unknown JSON keys [%s]", rjcError.text);
#if defined(__GNUC__) && (__GNUC__ >= 9)
#pragma GCC diagnostic pop
#endif

		return (rsslJsonSessionUninitialize(rjcSession), RSSL_RET_FAILURE);
	}

	/* When converting from JSON to RWF, catch unknown JSON FIDS. */
	flag = RSSL_TRUE;
	if (rsslJsonConverterSetProperty(rjcSession->pJsonConverter,
								RSSL_JSON_CPC_CATCH_UNKNOWN_JSON_FIDS, 
								&flag, &rjcError) != RSSL_RET_SUCCESS)
	{
#if defined(__GNUC__) && (__GNUC__ >= 9)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wformat-truncation"
#endif
		snprintf(pError->text, MAX_RSSL_ERROR_TEXT, "Failed setting RsslJsonConverter property: catch unknown JSON fields [%s]",
		rjcError.text);
#if defined(__GNUC__) && (__GNUC__ >= 9)
#pragma GCC diagnostic pop
#endif

		return (rsslJsonSessionUninitialize(rjcSession), RSSL_RET_FAILURE);
	}

	/* Enumerated values in RWF are translated to display strings in simplified JSON. 
	 * However, conversion from display strings to RWF is not currently supported.  
	 * Setting the property below will cause display strings to be converted to blank, 
	 * instead of resulting in errors. */
	flag = RSSL_TRUE;
	if (rsslJsonConverterSetProperty(rjcSession->pJsonConverter,
								RSSL_JSON_CPC_ALLOW_ENUM_DISPLAY_STRINGS, 
								&flag, &rjcError) != RSSL_RET_SUCCESS)
	{
#if defined(__GNUC__) && (__GNUC__ >= 9)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wformat-truncation"
#endif
		snprintf(pError->text, MAX_RSSL_ERROR_TEXT, "Failed setting RsslJsonConverter property: blank on enum display error [%s]",
		rjcError.text);
#if defined(__GNUC__) && (__GNUC__ >= 9)
#pragma GCC diagnostic pop
#endif

		return (rsslJsonSessionUninitialize(rjcSession), RSSL_RET_FAILURE);
	}
	
	rjcSession->jsonSessionInitialized = RSSL_TRUE;

	return RSSL_RET_SUCCESS;

}

RsslRet rsslJsonSessionSetDictionary(RsslJsonSession *rjcSession, RsslDataDictionary* dataDict, RsslError *pError)
{
	RsslJsonConverterError rjcError;
	RsslJsonDictionaryListProperty dlProperty;
	
	rjcSession->pDictionaryList = malloc(sizeof(RsslDataDictionary*) * 1); /* RsslJsonConverter supports only one RsslDataDictionary */
	if (rjcSession->pDictionaryList == NULL)
	{
		snprintf(pError->text, MAX_RSSL_ERROR_TEXT, "Failed to allocate memory to keep a list of RsslDataDictionary");
		return RSSL_RET_FAILURE;
	}

	/* Set RsslDataDictionary specified users */
	rjcSession->pDictionaryList[0] = dataDict;

	/* Set dictionary list. */
	rsslClearConverterDictionaryListProperty(&dlProperty);
	dlProperty.dictionaryListLength = 1;
	dlProperty.pDictionaryList = rjcSession->pDictionaryList;
	if (rsslJsonConverterSetProperty(rjcSession->pJsonConverter,
								RSSL_JSON_CPC_DICTIONARY_LIST, 
								&dlProperty, &rjcError) != RSSL_RET_SUCCESS)
	{
#if defined(__GNUC__) && (__GNUC__ >= 9)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wformat-truncation"
#endif
		snprintf(pError->text, MAX_RSSL_ERROR_TEXT, "Failed setting RsslJsonConverter property: dictionary list [%s]", rjcError.text);
#if defined(__GNUC__) && (__GNUC__ >= 9)
#pragma GCC diagnostic pop
#endif

		return RSSL_RET_FAILURE;
	}
	
	return RSSL_RET_SUCCESS;
}

RsslBuffer *rsslJsonSessionMsgConvertToJson(RsslJsonSession *rjcSession, RsslChannel *pChannel, 
								RsslBuffer *rwfBuffer, RsslError *pError)
{
	RsslDecodeIterator dIter;
	RsslMsg rsslMsg;
	RsslRet ret;
	RsslBuffer jsonBuffer = RSSL_INIT_BUFFER;
	RsslBuffer *pMsgBuffer = NULL;
	RsslError  error;

	rsslClearMsg(&rsslMsg);
	rsslClearDecodeIterator(&dIter);
	rsslSetDecodeIteratorRWFVersion(&dIter, pChannel->majorVersion, pChannel->minorVersion);
	rsslSetDecodeIteratorBuffer(&dIter, rwfBuffer);

	ret = rsslDecodeMsg(&dIter, &rsslMsg);

	if (ret == RSSL_RET_SUCCESS)
	{
		RsslConvertRsslMsgToJsonOptions rjcOptions;
		RsslGetJsonMsgOptions getJsonMsgOptions;
		RsslJsonConverterError rjcError;

		rsslClearConvertRsslMsgToJsonOptions(&rjcOptions);
		rjcOptions.jsonProtocolType = RSSL_JSON_JPT_JSON2;
		if ((rsslConvertRsslMsgToJson(rjcSession->pJsonConverter, &rjcOptions, &rsslMsg, &rjcError)) != RSSL_RET_SUCCESS)
		{
#if defined(__GNUC__) && (__GNUC__ >= 9)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wformat-truncation"
#endif
			snprintf(pError->text, MAX_RSSL_ERROR_TEXT, "Failed to convert RWF to JSON protocol. Error text: %s", rjcError.text);
#if defined(__GNUC__) && (__GNUC__ >= 9)
#pragma GCC diagnostic pop
#endif
			return NULL;
		}

		rsslClearGetJsonMsgOptions(&getJsonMsgOptions); 
		getJsonMsgOptions.jsonProtocolType = RSSL_JSON_JPT_JSON2;
		getJsonMsgOptions.streamId = rsslMsg.msgBase.streamId;
		getJsonMsgOptions.isCloseMsg = (rsslMsg.msgBase.msgClass == RSSL_MC_CLOSE) ? RSSL_TRUE : RSSL_FALSE;

		if ((ret = rsslGetConverterJsonMsg(rjcSession->pJsonConverter, &getJsonMsgOptions,
										&jsonBuffer, &rjcError)) != RSSL_RET_SUCCESS)
		{
#if defined(__GNUC__) && (__GNUC__ >= 9)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wformat-truncation"
#endif
			snprintf(pError->text, MAX_RSSL_ERROR_TEXT, "Failed to get converted JSON message. Error text: %s", rjcError.text);
#if defined(__GNUC__) && (__GNUC__ >= 9)
#pragma GCC diagnostic pop
#endif
			return NULL;
		}

		// Copies JSON data format to the buffer that belongs to RsslChannel
		pMsgBuffer = rsslGetBuffer(pChannel, jsonBuffer.length, RSSL_FALSE, &error);
		if (pMsgBuffer)
		{
			pMsgBuffer->length = jsonBuffer.length;
			memcpy(pMsgBuffer->data, jsonBuffer.data, jsonBuffer.length);
		}
		else
		{
#if defined(__GNUC__) && (__GNUC__ >= 9)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wformat-truncation"
#endif
			snprintf(pError->text, MAX_RSSL_ERROR_TEXT, "Failed to get buffer for converted msg. (%d) %s", error.rsslErrorId, error.text);
#if defined(__GNUC__) && (__GNUC__ >= 9)
#pragma GCC diagnostic pop
#endif
		}
	}

	return pMsgBuffer;
}

RsslRet rsslJsonSessionMsgConvertFromJson(RsslJsonSession *rjcSession, RsslChannel *pChannel,
								RsslBuffer *decodedMsg, RsslError *pError)
{
	RsslJsonConverterError rjcError;
	RsslDecodeJsonMsgOptions *decodeOptions;
	RsslJsonMsg *jsonMsg;
	RsslRet	ret;

	decodeOptions = &(rjcSession->state.decodeOptions);	
	jsonMsg = &(rjcSession->state.jsonMsg);	

	ret = rsslDecodeJsonMsg(rjcSession->pJsonConverter, decodeOptions, jsonMsg, 
							decodedMsg, &rjcError);

	if (ret != RSSL_RET_END_OF_CONTAINER)
	{
		if (ret == RSSL_RET_SUCCESS)
		{

			switch(jsonMsg->msgBase.msgClass) {
			case RSSL_JSON_MC_RSSL_MSG:
			{
				rsslDumpBuffer(pChannel, RSSL_RWF_PROTOCOL_TYPE, decodedMsg, pError);
				break;
			}
			case RSSL_JSON_MC_PING:
			{
				RsslError err;
				RsslBuffer PONG_MESSAGE = { 15, (char*)"{\"Type\":\"Pong\"}" };

				RsslBuffer *pBuffer = rsslGetBuffer(pChannel, PONG_MESSAGE.length, RSSL_FALSE, &err);

				if (pBuffer)
				{
					memcpy(pBuffer->data, PONG_MESSAGE.data, PONG_MESSAGE.length);

					// Reply with JSON PONG message to the sender
					ret = _rsslJsonSessionSendJsonMessage(pChannel, pBuffer, RSSL_WRITE_DIRECT_SOCKET_WRITE, pError);
					if (ret != RSSL_RET_FAILURE)
					{
						ret = RSSL_RET_READ_PING;
					}
				}
				else
				{
					ret = RSSL_RET_FAILURE;
				}

				break;
			}
			case RSSL_JSON_MC_PONG:
			{
				ret = RSSL_RET_READ_PING;
				break;
			}
			default: // RSSL_JSON_MC_ERROR
			{
				// Received JSON error message from the others side 
				char jsonMessage[RSSL_MAX_JSON_ERROR_MSG_SIZE];
				memset(&jsonMessage[0], 0, RSSL_MAX_JSON_ERROR_MSG_SIZE);
				memcpy(&jsonMessage[0], jsonMsg->msgBase.jsonMsgBuffer.data, 
						jsonMsg->msgBase.jsonMsgBuffer.length > RSSL_MAX_JSON_ERROR_MSG_SIZE ?
						RSSL_MAX_JSON_ERROR_MSG_SIZE : 
						jsonMsg->msgBase.jsonMsgBuffer.length);
			
				snprintf(pError->text, MAX_RSSL_ERROR_TEXT, 
							"Received JSON error message: %s.", &jsonMessage[0]);
				ret = RSSL_RET_FAILURE;

				break;
			}
			}
		} // if == _RET_SUCCESS 
		else
		{
#if defined(__GNUC__) && (__GNUC__ >= 9)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wformat-truncation"
#endif
			snprintf(pError->text, MAX_RSSL_ERROR_TEXT, "%s", rjcError.text);
#if defined(__GNUC__) && (__GNUC__ >= 9)
#pragma GCC diagnostic pop
#endif
		}
	}

	return ret;
}


