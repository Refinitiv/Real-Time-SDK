/*
 * This source code is provided under the Apache 2.0 license and is provided
 * AS IS with no warranty or guarantee of fit for purpose.  See the project's 
 * LICENSE.md for details. 
 * Copyright (C) 2020 Refinitiv. All rights reserved.
*/

#include "rjConverter.h"
#include "rtr/rsslReactor.h"
#include <stdio.h>  
#include <stdlib.h>
#include <string.h>
#include <errno.h>


#define RSSL_MAX_JSON_ERROR_MSG_SIZE 1101

static RsslRet _rjcSendJsonMessage(RsslChannel *pChannel, RsslBuffer *pBuffer, RsslUInt8 wrtFlags, RsslErrorInfo *pError)
{
	RsslUInt32  bWritten, ucbWritten;
	RsslError	error;	
	RsslRet		ret;

	ret = rsslWrite(pChannel, pBuffer, RSSL_HIGH_PRIORITY, wrtFlags, &bWritten, &ucbWritten, &error);

	while (ret == RSSL_RET_WRITE_CALL_AGAIN)
	{
		if ((ret = rsslFlush(pChannel, &error)) < RSSL_RET_SUCCESS)
		{
			snprintf(pError->rsslError.text, MAX_RSSL_ERROR_TEXT, 
						"rsslFlush() failed with return code %d - <%s>\n", ret, error.text);
			return ret;
		}
		ret = rsslWrite(pChannel, pBuffer, RSSL_HIGH_PRIORITY, wrtFlags, &bWritten, &ucbWritten, &error);
	}

	if (ret != RSSL_RET_SUCCESS)
		snprintf(pError->rsslError.text, MAX_RSSL_ERROR_TEXT, 
					"rsslWrite() failed with return code %d - <%s>\n", ret, error.text);

	return ret;
}

static RsslRet _rjcServiceNameToIdCallback(RsslBuffer *pServiceName, void *closure, RsslUInt16 *pServiceId)
{
	rjConverterSession *sess = (rjConverterSession*)closure;

	if (sess != NULL)
		return (*sess->options.pServiceNameToIdCallback)(pServiceName, sess->options.userSpecPtr, pServiceId);
	else
		return RSSL_RET_FAILURE;
}

RsslRet rjcResetConverterState(rjConverterSession *sess, RsslBuffer *pBuffer, RsslErrorInfo *pError)
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

	if (ret != RSSL_RET_SUCCESS)
		snprintf(pError->rsslError.text, MAX_RSSL_ERROR_TEXT, "rjcError: %s", error.text);
	
	return ret;
}


void rjcSessionUninitialize(rjConverterSession *rjcSession)
{
	RsslJsonConverterError rjcError;

	rsslJsonUninitialize();

	if (rjcSession->pJsonConverter != 0)
		rsslDestroyRsslJsonConverter(rjcSession->pJsonConverter, &rjcError);

	if (rjcSession->convBuff.data)
		free(rjcSession->convBuff.data);

	if (rjcSession->pDictionaryList)
		free(rjcSession->pDictionaryList);
}

RsslRet rjcSessionInitialize(rjConverterSession *rjcSession, RsslErrorInfo *pError)
{
	RsslCreateJsonConverterOptions rjcOptions;
	RsslJsonConverterError rjcError;
	RsslJsonDictionaryListProperty dlProperty;
	RsslBool flag = RSSL_TRUE;

	if (rjcSession->jsonConverterInitialized == RSSL_TRUE)
	{
		rsslSetErrorInfo(pError, RSSL_EIC_FAILURE, RSSL_RET_FAILURE, __FILE__, __LINE__,
			"The RsslJsonConverter has been initialized");

		return RSSL_RET_FAILURE;
	}

	rsslClearCreateRsslJsonConverterOptions(&rjcOptions);

	rjcSession->convBuff.length = RJC_MAX_BUFFER;
	rjcSession->convBuff.data = (char*)malloc(rjcSession->convBuff.length);

	rjcSession->options.catchUnknownJsonFids = RSSL_TRUE;

	/* Initialize string table */
	rsslJsonInitialize();

	rjcSession->pJsonConverter = rsslCreateRsslJsonConverter(&rjcOptions, &rjcError);
	if (rjcSession->pJsonConverter == NULL)
	{
		rsslSetErrorInfo(pError, RSSL_EIC_FAILURE, RSSL_RET_FAILURE, __FILE__, __LINE__, "Failed to create RsslJsonConverter: %s", rjcError.text);
		return (rjcSessionUninitialize(rjcSession), RSSL_RET_FAILURE);
	}

	rjcSession->pDictionaryList = malloc(sizeof(RsslDataDictionary*) * 1); /* RsslJsonConverter supports only one RsslDataDictionary */
	if (rjcSession->pDictionaryList == NULL)
	{
		rsslSetErrorInfo(pError, RSSL_EIC_FAILURE, RSSL_RET_FAILURE, __FILE__, __LINE__, "Failed to allocate memory to keep a list of RsslDataDictionary");
		return (rjcSessionUninitialize(rjcSession), RSSL_RET_FAILURE);
	}

	/* Set RsslDataDictionary specified users */
	rjcSession->pDictionaryList[0] = rjcSession->options.pDictionary;

	/* Set dictionary list. */
	rsslClearConverterDictionaryListProperty(&dlProperty);
	dlProperty.dictionaryListLength = 1;
	dlProperty.pDictionaryList = rjcSession->pDictionaryList;
	if (rsslJsonConverterSetProperty(rjcSession->pJsonConverter,
								RSSL_JSON_CPC_DICTIONARY_LIST, 
								&dlProperty, &rjcError) != RSSL_RET_SUCCESS)
	{
		rsslSetErrorInfo(pError, RSSL_EIC_FAILURE, RSSL_RET_FAILURE, __FILE__, __LINE__, 
		"Failed setting RsslJsonConverter property: dictionary list [%s]",
		rjcError.text);

		return (rjcSessionUninitialize(rjcSession), RSSL_RET_FAILURE);
	}

	/* Checks whether the callback method is set by users */
	if (rjcSession->options.pServiceNameToIdCallback)
	{
		RsslJsonServiceNameToIdCallbackProperty svcNameToIdCbProperty;

		rsslJsonClearServiceNameToIdCallbackProperty(&svcNameToIdCbProperty);

		svcNameToIdCbProperty.callback = _rjcServiceNameToIdCallback;
		svcNameToIdCbProperty.closure = rjcSession;

		/* Set service-name/ID callbacks. */
		if (rsslJsonConverterSetProperty(rjcSession->pJsonConverter,
			RSSL_JSON_CPC_SERVICE_NAME_TO_ID_CALLBACK,
			&svcNameToIdCbProperty, &rjcError) != RSSL_RET_SUCCESS)
		{
			rsslSetErrorInfo(pError, RSSL_EIC_FAILURE, RSSL_RET_FAILURE, __FILE__, __LINE__,
				"Failed setting RsslJsonConverter property: service name to ID callback [%s]",
				rjcError.text);

			return (rjcSessionUninitialize(rjcSession), RSSL_RET_FAILURE);
		}
	}

	/* Set default service ID. */
	if (rsslJsonConverterSetProperty(rjcSession->pJsonConverter,
								RSSL_JSON_CPC_DEFAULT_SERVICE_ID, 
								&rjcSession->options.defaultServiceId, &rjcError) != RSSL_RET_SUCCESS)
	{
		rsslSetErrorInfo(pError, RSSL_EIC_FAILURE, RSSL_RET_FAILURE, __FILE__, __LINE__, 
		"Failed setting RsslJsonConverter property: default service ID [%s]", rjcError.text);

		return (rjcSessionUninitialize(rjcSession), RSSL_RET_FAILURE);
	}

	/* When converting from RWF to JSON, add a QoS range on requests that do not specify a QoS */
	flag = RSSL_TRUE;
	if (rsslJsonConverterSetProperty(rjcSession->pJsonConverter,
								RSSL_JSON_CPC_USE_DEFAULT_DYNAMIC_QOS, 
								&flag, &rjcError) != RSSL_RET_SUCCESS)
	{
		rsslSetErrorInfo(pError, RSSL_EIC_FAILURE, RSSL_RET_FAILURE, __FILE__, __LINE__, 
		"Failed setting RsslJsonConverter property: add default QoS range [%s]", rjcError.text);

		return (rjcSessionUninitialize(rjcSession), RSSL_RET_FAILURE);
	}

	/* Expand enumerated values in field entries to their display values. 
	 * Dictionary must have enumerations loaded */
	flag = rjcSession->options.jsonExpandedEnumFields;
	if (rsslJsonConverterSetProperty(rjcSession->pJsonConverter,
								RSSL_JSON_CPC_EXPAND_ENUM_FIELDS, 
								&flag, &rjcError) != RSSL_RET_SUCCESS)
	{
		rsslSetErrorInfo(pError, RSSL_EIC_FAILURE, RSSL_RET_FAILURE, __FILE__, __LINE__, 
		"Failed setting RsslJsonConverter property: expand enum fields [%s]", rjcError.text);

		return (rjcSessionUninitialize(rjcSession), RSSL_RET_FAILURE);
	}

	/* When converting from JSON to RWF, catch unknown JSON keys. */
	flag = rjcSession->options.catchUnknownJsonKeys;
	if (rsslJsonConverterSetProperty(rjcSession->pJsonConverter,
								RSSL_JSON_CPC_CATCH_UNKNOWN_JSON_KEYS, 
								&flag, &rjcError) != RSSL_RET_SUCCESS)
	{
		rsslSetErrorInfo(pError, RSSL_EIC_FAILURE, RSSL_RET_FAILURE, __FILE__, __LINE__, 
		"Failed setting RsslJsonConverter property: catch unknown JSON keys [%s]", rjcError.text);

		return (rjcSessionUninitialize(rjcSession), RSSL_RET_FAILURE);
	}

	/* When converting from JSON to RWF, catch unknown JSON FIDS. */
	flag = rjcSession->options.catchUnknownJsonFids;
	if (rsslJsonConverterSetProperty(rjcSession->pJsonConverter,
								RSSL_JSON_CPC_CATCH_UNKNOWN_JSON_FIDS, 
								&flag, &rjcError) != RSSL_RET_SUCCESS)
	{
		rsslSetErrorInfo(pError, RSSL_EIC_FAILURE, RSSL_RET_FAILURE, __FILE__, __LINE__, 
		"Failed setting RsslJsonConverter property: catch unknown JSON fields [%s]",
		rjcError.text);

		return (rjcSessionUninitialize(rjcSession), RSSL_RET_FAILURE);
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
		rsslSetErrorInfo(pError, RSSL_EIC_FAILURE, RSSL_RET_FAILURE, __FILE__, __LINE__, 
		"Failed setting RsslJsonConverter property: blank on enum display error [%s]",
		rjcError.text);

		return (rjcSessionUninitialize(rjcSession), RSSL_RET_FAILURE);
	}
	
	rjcSession->jsonConverterInitialized = RSSL_TRUE;

	return RSSL_RET_SUCCESS;

}

RsslBuffer *rjcMsgConvertToJson(rjConverterSession *rjcSession, RsslChannel *pChannel, 
								RsslBuffer *rwfBuffer, RsslErrorInfo *pError)
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

	/* Clears the previous error code */
	pError->rsslError.rsslErrorId = 0;

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
			rsslSetErrorInfo(pError, RSSL_EIC_FAILURE, RSSL_RET_FAILURE, __FILE__, __LINE__, 
							"Failed to convert RWF to JSON protocol. Error text: %s", rjcError.text);
			return NULL;
		}

		rsslClearGetJsonMsgOptions(&getJsonMsgOptions); 
		getJsonMsgOptions.jsonProtocolType = RSSL_JSON_JPT_JSON2;
		getJsonMsgOptions.streamId = rsslMsg.msgBase.streamId;
		getJsonMsgOptions.isCloseMsg = (rsslMsg.msgBase.msgClass == RSSL_MC_CLOSE) ? RSSL_TRUE : RSSL_FALSE;

		if ((ret = rsslGetConverterJsonMsg(rjcSession->pJsonConverter, &getJsonMsgOptions,
										&jsonBuffer, &rjcError)) != RSSL_RET_SUCCESS)
		{
			rsslSetErrorInfo(pError, RSSL_EIC_FAILURE, RSSL_RET_FAILURE, __FILE__, __LINE__, 
							"Failed to get converted JSON message. Error text: %s", rjcError.text);
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
			rsslSetErrorInfo(pError, RSSL_EIC_FAILURE, error.rsslErrorId, 
						__FILE__, __LINE__, 
						"Failed to get buffer for converted msg. (%d) %s", 
						error.rsslErrorId, error.text);

	}
	else
		rsslSetErrorInfo(pError, RSSL_EIC_FAILURE, RSSL_RET_FAILURE, 
						__FILE__, __LINE__, "rsslDecodeMsg() failed: %d", ret);

	return pMsgBuffer;
}

RsslRet rjcMsgConvertFromJson(rjConverterSession *rjcSession, RsslChannel *pChannel,
								RsslBuffer *decodedMsg, RsslBuffer *pJsonBuffer, RsslErrorInfo *pError)
{
	RsslJsonConverterError rjcError;
	RsslDecodeJsonMsgOptions *decodeOptions;
	RsslJsonMsg *jsonMsg;
	RsslRet	ret;

	if (pJsonBuffer != NULL)
	{
		if ((ret = rjcResetConverterState(rjcSession, pJsonBuffer, pError)) != RSSL_RET_SUCCESS)
			// Check to see if need to send JSON error msg
			return RSSL_RET_FAILURE;
	}
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
					ret = _rjcSendJsonMessage(pChannel, pBuffer, RSSL_WRITE_NO_FLAGS, pError);
					if (ret != RSSL_RET_FAILURE)
						ret = RSSL_RET_READ_PING;
				}
				else
				{
					ret = RSSL_RET_FAILURE;
					snprintf(pError->rsslError.text, MAX_RSSL_ERROR_TEXT, 
						"Failed to get buffer for Ping response :%s", err.text);
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
				
				snprintf(pError->rsslError.text, MAX_RSSL_ERROR_TEXT, 
							"Received JSON error message: %s.", &jsonMessage[0]);
				ret = RSSL_RET_FAILURE;

				break;
			}
			}
		} // if == _RET_SUCCESS 
		else
			snprintf(pError->rsslError.text, MAX_RSSL_ERROR_TEXT, "%s", rjcError.text);
	}

	return ret;
}


