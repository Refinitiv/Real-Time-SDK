/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|          Copyright (C) 2019-2023, 2025 LSEG. All rights reserved.         --
 *|-----------------------------------------------------------------------------
 */


#include "rtr/rsslJsonConverter.h"
#include "rtr/jsonToRwfSimple.h"
#include "rtr/rwfToJsonSimple.h"
#include "rtr/jsonToRwfConverter.h"
#include "rtr/rwfToJsonConverter.h"
#include "rtr/jsonToRsslMsgDecoder.h"
#include "jsonVersion.h"

#ifdef WIN32
#if _MSC_VER < 1900
#define snprintf _snprintf
#endif
#endif

typedef struct
{
	jsonToRwfSimple			*_jsonToRwfSimple;		/** Simple JSON-to-RWF converter. */
	rwfToJsonSimple			*_rwfToJsonSimple;		/** RWF-to-Simple JSON converter. */
	jsonToRwfConverter		*_jsonToRwfConverter;	/** Standard JSON-to-RWF converter. */
	rwfToJsonConverter		*_rwfToJsonConverter;	/** RWF-to-JSON Standard converter. */
} RsslJsonConverterImpl;

RSSL_RJC_API void rsslJsonInitialize()
{
	rwfToJsonBase::initializeIntToStringTable();
}

RSSL_RJC_API void rsslJsonUninitialize()
{
	rwfToJsonBase::uninitializeIntToStringTable();
}

RSSL_RJC_API RsslJsonConverter rsslCreateRsslJsonConverter(RsslCreateJsonConverterOptions *pOptions, RsslJsonConverterError *pError)
{
	RsslJsonConverterImpl *pConverterImpl = (RsslJsonConverterImpl*)malloc(sizeof(RsslJsonConverterImpl));

	if (!pConverterImpl)
	{
		snprintf(pError->text, MAX_CONVERTER_ERROR_TEXT, "Failed to allocate converter object.");
		return NULL;
	}

	/*The purpose of this option is to optimize performance of rmdstestclient*/
	if (pOptions->skipEncodingPayload)
		pConverterImpl->_jsonToRwfSimple = new jsonToRsslMsgDecoder(pOptions->bufferSize, 0, DEFAULT_NUM_TOKENS, pOptions->jsonTokenIncrementSize);
	else
		pConverterImpl->_jsonToRwfSimple = new jsonToRwfSimple(pOptions->bufferSize, 0, DEFAULT_NUM_TOKENS, pOptions->jsonTokenIncrementSize);

	if (!pConverterImpl->_jsonToRwfSimple)
	{
		snprintf(pError->text, MAX_CONVERTER_ERROR_TEXT, "Failed to allocate JSON-to-RWF converter.");
		rsslDestroyRsslJsonConverter((RsslJsonConverter*)pConverterImpl, pError);
		return NULL;
	}

	pConverterImpl->_rwfToJsonSimple = new rwfToJsonSimple(pOptions->bufferSize, 0);
	if (!pConverterImpl->_rwfToJsonSimple)
	{
		snprintf(pError->text, MAX_CONVERTER_ERROR_TEXT, "Failed to allocate RWF-to-JSON converter.");
		rsslDestroyRsslJsonConverter((RsslJsonConverter*)pConverterImpl, pError);
		return NULL;
	}

	pConverterImpl->_jsonToRwfConverter = new jsonToRwfConverter(pOptions->bufferSize, 0);
	if (!pConverterImpl->_jsonToRwfConverter)
	{
		snprintf(pError->text, MAX_CONVERTER_ERROR_TEXT, "Failed to allocate JSON-to-RWF standard converter.");
		rsslDestroyRsslJsonConverter((RsslJsonConverter*)pConverterImpl, pError);
		return NULL;
	}

	pConverterImpl->_rwfToJsonConverter = new rwfToJsonConverter(pOptions->bufferSize, 0);
	if (!pConverterImpl->_rwfToJsonConverter)
	{
		snprintf(pError->text, MAX_CONVERTER_ERROR_TEXT, "Failed to allocate RWF-to-JSON standard converter.");
		rsslDestroyRsslJsonConverter((RsslJsonConverter*)pConverterImpl, pError);
		return NULL;
	}

	return (RsslJsonConverter*)pConverterImpl;
}

RSSL_RJC_API RsslRet rsslDestroyRsslJsonConverter(RsslJsonConverter pConverter, RsslJsonConverterError *pError)
{
	RsslJsonConverterImpl *pConverterImpl = (RsslJsonConverterImpl*)pConverter;

	if (pConverterImpl->_jsonToRwfSimple)
	{
		delete pConverterImpl->_jsonToRwfSimple;
		pConverterImpl->_jsonToRwfSimple = 0;
	}

	if (pConverterImpl->_rwfToJsonSimple)
	{
		delete pConverterImpl->_rwfToJsonSimple;
		pConverterImpl->_rwfToJsonSimple = 0;
	}

	if (pConverterImpl->_jsonToRwfConverter)
	{
		delete pConverterImpl->_jsonToRwfConverter;
		pConverterImpl->_jsonToRwfConverter = 0;
	}

	if (pConverterImpl->_rwfToJsonConverter)
	{
		delete pConverterImpl->_rwfToJsonConverter;
		pConverterImpl->_rwfToJsonConverter = 0;
	}

	free(pConverter);

	return RSSL_RET_SUCCESS;
}


RSSL_RJC_API RsslRet rsslJsonConverterSetProperty(RsslJsonConverter pConverter, RsslUInt32 code, void* value, RsslJsonConverterError *pError)
{
	RsslJsonConverterImpl *pConverterImpl = (RsslJsonConverterImpl*)pConverter;
	jsonToRwfSimple *pJsonToRwfSimple = pConverterImpl->_jsonToRwfSimple;
	rwfToJsonSimple *pRwfToJsonSimple = pConverterImpl->_rwfToJsonSimple;
	rwfToJsonConverter *pRwfToJsonConverter = pConverterImpl->_rwfToJsonConverter;
	jsonToRwfConverter *pJsonToRwfConverter = pConverterImpl->_jsonToRwfConverter;

	switch(code)
	{
		case RSSL_JSON_CPC_SERVICE_ID_TO_NAME_CALLBACK:
		{
			RsslJsonServiceIdToNameCallbackProperty *cbProperty = (RsslJsonServiceIdToNameCallbackProperty*)value ;
			pRwfToJsonSimple->setRsslServiceIdToNameCallback(cbProperty->closure, cbProperty->callback);
			return RSSL_RET_SUCCESS;
		}

		case RSSL_JSON_CPC_SERVICE_NAME_TO_ID_CALLBACK:
		{
			RsslJsonServiceNameToIdCallbackProperty *cbProperty = (RsslJsonServiceNameToIdCallbackProperty*)value ;
			pJsonToRwfSimple->setRsslServiceNameToIdCallback(cbProperty->closure, cbProperty->callback);
			return RSSL_RET_SUCCESS;
		}

		case RSSL_JSON_CPC_DICTIONARY_LIST:
		{
			RsslJsonDictionaryListProperty *dictionaryListProperty = (RsslJsonDictionaryListProperty*)value;
			RsslUInt32 index = 0;

			if (dictionaryListProperty == NULL || dictionaryListProperty->dictionaryListLength == 0)
			{
				snprintf(pError->text, MAX_CONVERTER_ERROR_TEXT, "<%s:%d> %s", __FILE__, __LINE__, "Passing invalid RsslJsonDictionaryListProperty");
				pError->rsslErrorId = RSSL_RET_FAILURE;
				return RSSL_RET_FAILURE;
			}

			for (; index < dictionaryListProperty->dictionaryListLength; index++)
			{
				if (dictionaryListProperty->pDictionaryList[index] == NULL)
				{
					snprintf(pError->text, MAX_CONVERTER_ERROR_TEXT, "<%s:%d> %s", __FILE__, __LINE__, "RsslDataDictionary is NULL");
					pError->rsslErrorId = RSSL_RET_FAILURE;
					return RSSL_RET_FAILURE;
				}
			}

			pJsonToRwfSimple->setDictionaryList(dictionaryListProperty->pDictionaryList, dictionaryListProperty->dictionaryListLength);
			pRwfToJsonSimple->setDictionaryList(dictionaryListProperty->pDictionaryList, dictionaryListProperty->dictionaryListLength);
			pJsonToRwfConverter->setDictionaryList(dictionaryListProperty->pDictionaryList, dictionaryListProperty->dictionaryListLength);
			pRwfToJsonConverter->setDictionaryList(dictionaryListProperty->pDictionaryList, dictionaryListProperty->dictionaryListLength);
			return RSSL_RET_SUCCESS;
		}

		case RSSL_JSON_CPC_DEFAULT_SERVICE_ID:
		{
			RsslUInt16 *pServiceId = (RsslUInt16*)value;
			pJsonToRwfSimple->setDefaultServiceId(*pServiceId);
			return RSSL_RET_SUCCESS;
		}

		case RSSL_JSON_CPC_USE_DEFAULT_DYNAMIC_QOS:
		{
			RsslBool *pFlag = (RsslBool*)value;
			pJsonToRwfSimple->setUseDefaultDynamicQos(*pFlag);
			return RSSL_RET_SUCCESS;
		}

		case RSSL_JSON_CPC_EXPAND_ENUM_FIELDS:
		{
			RsslBool *pFlag = (RsslBool*)value;
			pRwfToJsonSimple->setExpandEnumFields(*pFlag);
			return RSSL_RET_SUCCESS;
		}

		case RSSL_JSON_CPC_REAL_AS_EXPONENT:
		{
			RsslBool *pFlag = (RsslBool*)value;
			pRwfToJsonConverter->setEncodeRealAsPrimitive(*pFlag);
			return RSSL_RET_SUCCESS;
		}

		case RSSL_JSON_CPC_CATCH_UNKNOWN_JSON_KEYS:
		{
			RsslBool *pFlag = (RsslBool*)value;
			pJsonToRwfSimple->setCatchUnknownJsonKeys(*pFlag);
			return RSSL_RET_SUCCESS;
		}

		case RSSL_JSON_CPC_CATCH_UNKNOWN_JSON_FIDS:
		{
			RsslBool *pFlag = (RsslBool*)value;
			pJsonToRwfSimple->setCatchUnknownJsonFids(*pFlag);
			return RSSL_RET_SUCCESS;
		}

		case RSSL_JSON_CPC_ALLOW_ENUM_DISPLAY_STRINGS:
		{
			RsslBool *pFlag = (RsslBool*)value;
			
			if (pJsonToRwfSimple->setAllowEnumDisplayStrings(*pFlag) == false)
			{
				snprintf(pError->text, MAX_CONVERTER_ERROR_TEXT, "<%s:%d> %s", pJsonToRwfSimple->errorFile(), *pJsonToRwfSimple->errorLineNum(), pJsonToRwfSimple->errorText()->data);
				pError->rsslErrorId = RSSL_RET_FAILURE;
				return RSSL_RET_FAILURE;
			}
			
			return RSSL_RET_SUCCESS;
		}

		default:
		{
			snprintf(pError->text, MAX_CONVERTER_ERROR_TEXT, "Unknown property code: %u", code);
			return RSSL_RET_INVALID_ARGUMENT;
		}
	}
}

RSSL_RJC_API RsslRet rsslParseJsonBuffer(RsslJsonConverter pConverter, RsslParseJsonBufferOptions *pOptions, RsslBuffer *pJsonBuffer, RsslJsonConverterError *pError)
{
	RsslRet ret;
	RsslJsonConverterImpl *pConverterImpl = (RsslJsonConverterImpl*)pConverter;
	jsonToRwfBase *pIntConverter;

	switch(pOptions->jsonProtocolType)
	{
		case RSSL_JSON_JPT_JSON: pIntConverter = pConverterImpl->_jsonToRwfConverter; break;
		case RSSL_JSON_JPT_JSON2: pIntConverter = pConverterImpl->_jsonToRwfSimple; break;
		default:
		{
			snprintf(pError->text, MAX_CONVERTER_ERROR_TEXT, "Unknown JSON protocol type %u.", pOptions->jsonProtocolType);
			return RSSL_RET_INVALID_ARGUMENT;
		}
	}

	if ((ret = pIntConverter->parseJsonBuffer(pJsonBuffer, 0)) < 0)
	{
		snprintf(pError->text, MAX_CONVERTER_ERROR_TEXT, "%s", pIntConverter->errorText()->data);
		return ret;
	}

	return ret;
}

RSSL_RJC_API RsslRet rsslDecodeJsonMsg(RsslJsonConverter pConverter, RsslDecodeJsonMsgOptions *pOptions, RsslJsonMsg *pJsonMsg, RsslBuffer *pRsslBuffer, RsslJsonConverterError *pError)
{
	RsslJsonConverterImpl *pConverterImpl = (RsslJsonConverterImpl*)pConverter;
	jsonToRwfBase *pIntConverter;
	RsslRet ret;

	switch(pOptions->jsonProtocolType)
	{
		case RSSL_JSON_JPT_JSON: pIntConverter = pConverterImpl->_jsonToRwfConverter; break;
		case RSSL_JSON_JPT_JSON2: pIntConverter = pConverterImpl->_jsonToRwfSimple; break;
		default:
		{
			snprintf(pError->text, MAX_CONVERTER_ERROR_TEXT, "Unknown JSON protocol type %u.", pOptions->jsonProtocolType);
			return RSSL_RET_INVALID_ARGUMENT;
		}
	}

	if ((ret = pIntConverter->decodeJsonMsg(*pJsonMsg)) < 0)
	{
		snprintf(pError->text, MAX_CONVERTER_ERROR_TEXT, "%s", pIntConverter->errorText()->data);
		return ret;
	}

	*pRsslBuffer = pIntConverter->_outBuf;

	return ret > 0 ? RSSL_RET_SUCCESS : RSSL_RET_END_OF_CONTAINER;
}

RSSL_RJC_API RsslRet rsslGetJsonSimpleErrorParams(RsslJsonConverter pConverter, RsslDecodeJsonMsgOptions *pOptions, RsslJsonConverterError *pError, RsslGetJsonErrorParams *pErrorParams, RsslBuffer *jsonMsg, RsslInt32 streamId)
{
	RsslJsonConverterImpl *pConverterImpl = (RsslJsonConverterImpl*)pConverter;
	jsonToRwfBase *pIntConverter;

	switch(pOptions->jsonProtocolType)
	{
		case RSSL_JSON_JPT_JSON: pIntConverter = pConverterImpl->_jsonToRwfConverter; break;
		case RSSL_JSON_JPT_JSON2: pIntConverter = pConverterImpl->_jsonToRwfSimple; break;
		default:
		{
			snprintf(pError->text, MAX_CONVERTER_ERROR_TEXT, "Unknown JSON protocol type %u.", pOptions->jsonProtocolType);
			return RSSL_RET_INVALID_ARGUMENT;
		}
	}

	RsslInt32 *offset = 0;
	if(pIntConverter->errorToken()) {
		offset = &pIntConverter->errorToken()->start;
	}

	rsslClearRsslGetJsonErrorParams(pErrorParams);

	RsslBuffer *errorText = pIntConverter->errorText();
	pErrorParams->errorText = errorText->data;
	pErrorParams->errorFile = pIntConverter->errorFile();
	pErrorParams->errorLine = pIntConverter->errorLineNum();
	pErrorParams->errorOffset = offset;
	pErrorParams->errorOriginalMessage = jsonMsg;
	pErrorParams->errorStreamId = streamId;

	return RSSL_RET_SUCCESS;
}

RSSL_RJC_API RsslRet rsslConvertRsslMsgToJson(RsslJsonConverter pConverter, RsslConvertRsslMsgToJsonOptions *pOptions, RsslMsg *pRsslMsg, RsslJsonConverterError *pError)
{
	RsslJsonConverterImpl *pConverterImpl = (RsslJsonConverterImpl*)pConverter;
	rwfToJsonBase *pIntConverter;

	switch(pOptions->jsonProtocolType)
	{
		case RSSL_JSON_JPT_JSON: pIntConverter = pConverterImpl->_rwfToJsonConverter; break;
		case RSSL_JSON_JPT_JSON2: pIntConverter = pConverterImpl->_rwfToJsonSimple; break;
		default:
		{
			snprintf(pError->text, MAX_CONVERTER_ERROR_TEXT, "Unknown JSON protocol type %u.", pOptions->jsonProtocolType);
			return RSSL_RET_INVALID_ARGUMENT;
		}
	}

	pIntConverter->reset();
	if (pIntConverter->convertMsg(*pRsslMsg) != 1 || pIntConverter->error())
	{
		snprintf(pError->text, MAX_CONVERTER_ERROR_TEXT, "Conversion failed.");
		return RSSL_RET_FAILURE;
	}

	return RSSL_RET_SUCCESS;
}

RSSL_RJC_API RsslRet rsslGetConverterJsonMsg(RsslJsonConverter pConverter, RsslGetJsonMsgOptions *pOptions, RsslBuffer *pOutBuffer, RsslJsonConverterError *pError)
{
	RsslJsonConverterImpl *pConverterImpl = (RsslJsonConverterImpl*)pConverter;
	rwfToJsonBase *pIntConverter;

	switch(pOptions->jsonProtocolType)
	{
		case RSSL_JSON_JPT_JSON: pIntConverter = pConverterImpl->_rwfToJsonConverter; break;
		case RSSL_JSON_JPT_JSON2: pIntConverter = pConverterImpl->_rwfToJsonSimple; break;
		default:
		{
			snprintf(pError->text, MAX_CONVERTER_ERROR_TEXT, "Unknown JSON protocol type %u.", pOptions->jsonProtocolType);
			return RSSL_RET_INVALID_ARGUMENT;
		}
	}

	switch(pOptions->transportProtocol)
	{
		case RSSL_JSON_TP_WS:
			*pOutBuffer = *pIntConverter->getJsonMsg(pOptions->streamId, pOptions->solicited != 0, pOptions->isCloseMsg != 0);
			return RSSL_RET_SUCCESS;

		default:
			snprintf(pError->text, MAX_CONVERTER_ERROR_TEXT, "Unknown transport protocol %u.", pOptions->transportProtocol);
			return RSSL_RET_INVALID_ARGUMENT;
	}
}

RSSL_RJC_API RsslRet rsslJsonBuildTerminateStream(RsslJsonConverter pConverter, RsslJsonBuildTerminateStreamOptions *pOptions, RsslBuffer *pOutBuffer, RsslJsonConverterError *pError)
{
	RsslJsonConverterImpl *pConverterImpl = (RsslJsonConverterImpl*)pConverter;

 	rwfToJsonConverter *pIntConverter = pConverterImpl->_rwfToJsonConverter;
	RJCBuffer rjcBuffer;
	RJCString errorText(pOptions->text.data, pOptions->text.length);

	if (!pIntConverter->buildJsonTerminateStream(rjcBuffer, pOptions->errorCode, errorText))
		return RSSL_RET_FAILURE;

	pOutBuffer->length = rjcBuffer.count();
	pOutBuffer->data = rjcBuffer.to_c(1);
	return RSSL_RET_SUCCESS;
}

RSSL_RJC_API RsslRet rsslJsonGetErrorMessage(RsslJsonConverter pConverter, RsslGetJsonErrorParams *pErrorParams, RsslBuffer *pOutBuffer)
{
	RsslJsonConverterImpl *pConverterImpl = (RsslJsonConverterImpl*)pConverter;
	rwfToJsonSimple *pIntConverter = pConverterImpl->_rwfToJsonSimple;

	pIntConverter->reset();

	if (const RsslBuffer *errorMessage = pIntConverter->generateErrorMessage(pErrorParams->errorText, pErrorParams->errorFile, pErrorParams->errorLine, pErrorParams->errorOffset, pErrorParams->errorOriginalMessage, pErrorParams->errorStreamId))
	{
		pOutBuffer->length = errorMessage->length;
		pOutBuffer->data = errorMessage->data;
		pIntConverter->reset();
		return RSSL_RET_SUCCESS;
	}

	return RSSL_RET_FAILURE;


}

RSSL_RJC_API void rsslQueryJsonConverterLibraryVersion(RsslLibraryVersionInfo *pVerInfo)
{
	if (pVerInfo)
	{
		pVerInfo->productDate = jsonDeltaDate;
		pVerInfo->internalVersion = jsonVersion;
		pVerInfo->productVersion = jsonPackage;
		pVerInfo->interfaceVersion = jsonInterfaceVersion;
	}
}
