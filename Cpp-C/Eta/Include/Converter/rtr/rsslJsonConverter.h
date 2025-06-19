/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2020,2022-2024 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

#ifndef RSSL_JSON_CONVERTER_H
#define RSSL_JSON_CONVERTER_H

#include "rtr/rsslDataDictionary.h"
#include "rtr/rsslMsg.h"

/**
* @addtogroup rsslJsonConverter
 * @{
*/

 /* Used when exporting or importing as Windows DLL or linking as static library */
#if defined(RSSL_RJC_EXPORTS) || defined(RSSL_EXPORTS)
#define 	RSSL_RJC_API			RTR_API_EXPORT
#elif defined(RSSL_RJC_IMPORTS) || defined(RSSL_IMPORTS)
#define 	RSSL_RJC_API				RTR_API_IMPORT
#else
#define 	RSSL_RJC_API
#endif

#ifdef __cplusplus
extern "C" {
#endif

#define MAX_CONVERTER_ERROR_TEXT 1200

/**
 * @brief Error structure passed to some converter interface methods.
 * Populated with information if an error occurs during the function call.
 */
typedef struct
{
	RsslRet			rsslErrorId;						/*!< The RSSL error code */
	char			text[MAX_CONVERTER_ERROR_TEXT + 1];	/*!< Additional information about the error */
} RsslJsonConverterError;

/**
 * @brief Initialize shared resources for RWF/JSON conversion
 */
RSSL_RJC_API void rsslJsonInitialize();

/**
 * @brief Cleanup shared resources for RWF/JSON conversion
 */
RSSL_RJC_API void rsslJsonUninitialize();

/**
 * @brief Options for rsslCreateRsslJsonConverter.
 */
typedef struct
{
	RsslUInt32 bufferSize; /*!< Size of the buffer that the converter will allocate for its output buffer. */
	RsslUInt32 jsonTokenIncrementSize; /*!< Number of json token increment size for parsing JSON messages. */
	RsslBool   skipEncodingPayload; /*!< Shall the json to Rwf simple converter skips encoding the payload data */
} RsslCreateJsonConverterOptions;

/**
 * @brief Clears an RsslCreateJsonConverterOptions structure.
 */
RTR_C_INLINE void rsslClearCreateRsslJsonConverterOptions(RsslCreateJsonConverterOptions *pOptions)
{
	memset(pOptions, 0, sizeof(RsslCreateJsonConverterOptions));
	pOptions->bufferSize = 65535;
	pOptions->jsonTokenIncrementSize = 500;
	pOptions->skipEncodingPayload = RSSL_FALSE;
}

/**
 * @brief RsslJsonConverter type. Created by rsslCreateRsslJsonConverter and used in RWF/JSON conversion functions.
 */
typedef void* RsslJsonConverter;

/**
 * @brief Create an RSSL-JSON converter.
 * @param pOptions Options for the converter.
 *
 */
RSSL_RJC_API RsslJsonConverter rsslCreateRsslJsonConverter(RsslCreateJsonConverterOptions *pOptions, RsslJsonConverterError *pError);

/**
 * @brief Destroy an RSSL-JSON converter.
 * @param pConverter Converter to destroy.
 */
RSSL_RJC_API RsslRet rsslDestroyRsslJsonConverter(RsslJsonConverter pConverter, RsslJsonConverterError *pError);

/**
 * @brief Callback to translate a Service ID to a Service Name.
 * @param pServiceName The name of the service that the callback will look up to find the appropriate ID.
 * @param closure Pointer to a user-defined object that will be provided when this callback is called.
 * @param pServiceId An RsslUInt16 that the callback should populate with the translated ID.
 * @return RSSL_RET_SUCCESS if a matching ID was found, RSSL_RET_FAILURE otherwise.
*/
typedef RsslRet RsslJsonServiceNameToIdCallback(RsslBuffer *pServiceName, void *closure, RsslUInt16 *pServiceId);

/**
 * @brief Callback to translate a Service Name to a Service ID.
 * @param pServiceId The service ID that the callback will look up to find the appropriate name.
 * @param closure Pointer to a user-defined object that will be provided when this callback is called.
 * @param pServiceName An RsslBuffer that the callback should populate with the translated name. The memory set in RsslBuffer::data must remain valid until the message conversion is complete.
 * @return RSSL_RET_SUCCESS if successful, RSSL_RET_FAILURE otherwise.
*/
typedef RsslRet RsslJsonServiceIdToNameCallback(RsslBuffer *pServiceName, void *closure, RsslUInt16 serviceId);

/**
 * @brief Structure used as the value when calling rsslJsonConverterSetProperty with the RSSL_JSON_CPC_SERVICE_ID_TO_NAME_CALLBACK code.
 */
typedef struct
{
	RsslJsonServiceIdToNameCallback *callback;		/*!< Service ID-to-name callback function. */
	void *closure;									/*!< User-defined pointer to provide when calling the callback function. */
} RsslJsonServiceIdToNameCallbackProperty;

/**
 * @brief Clears an RsslJsonServiceIdToNameCallbackProperty.
 */
RTR_C_INLINE void rsslJsonClearServiceIdToNameCallbackProperty(RsslJsonServiceIdToNameCallbackProperty *pProperty)
{
	memset(pProperty, 0, sizeof(RsslJsonServiceIdToNameCallbackProperty));
}

/**
 * @brief Structure used as the value when calling rsslJsonConverterSetProperty with the RSSL_JSON_CPC_SERVICE_NAME_TO_ID_CALLBACK code.
 */
typedef struct
{
	RsslJsonServiceNameToIdCallback *callback;	/*!< Service name-to-ID callback function. */
	void *closure;								/*!< User-defined pointer to provide when calling the callback function. */
} RsslJsonServiceNameToIdCallbackProperty;

/**
 * @brief Clears an RsslJsonServiceNameToIdCallbackProperty.
 */
RTR_C_INLINE void rsslJsonClearServiceNameToIdCallbackProperty(RsslJsonServiceNameToIdCallbackProperty *pProperty)
{
	memset(pProperty, 0, sizeof(RsslJsonServiceNameToIdCallbackProperty));
}

/**
 * @brief Structure used as the value when calling rsslJsonConverterSetProperty with the RSSL_JSON_CPC_DICTIONARY_LIST code.
 */
typedef struct
{
	RsslDataDictionary **pDictionaryList;	/*!< Pointer to a list of RsslDataDictionary pointers. */
	RsslUInt32 dictionaryListLength;		/*!< Length of the list in pDictionaryList. */
} RsslJsonDictionaryListProperty;

/**
 * @brief Clears an RsslJsonDictionaryListProperty.
 */
RTR_C_INLINE void rsslClearConverterDictionaryListProperty(RsslJsonDictionaryListProperty *pProperty)
{
	memset(pProperty, 0, sizeof(RsslJsonDictionaryListProperty));
}

/**
 * @brief Codes used with rsslJsonConverterSetProperty.
 */
typedef enum
{
	RSSL_JSON_CPC_SERVICE_ID_TO_NAME_CALLBACK	= 0,	/*!< (Simplified JSON) Set the service ID-to-name callback. (value: RsslJsonServiceIdToNameCallbackProperty). */
	RSSL_JSON_CPC_SERVICE_NAME_TO_ID_CALLBACK	= 1,	/*!< (Simplified JSON) Set the service name-to-ID callback (value: RsslJsonServiceIdToNameCallbackProperty). */
	RSSL_JSON_CPC_DICTIONARY_LIST				= 2,	/*!< Set a dictionary list on the converter.  (value: RsslJsonDictionaryListProperty). */
	RSSL_JSON_CPC_DEFAULT_SERVICE_ID			= 3,	/*!< (Simplified JSON) Set a default service ID to use when converting requests from JSON to RWF. If Request.Key.Service is not present, this ID will be used. (value: RsslUInt16) */
	RSSL_JSON_CPC_USE_DEFAULT_DYNAMIC_QOS		= 4,	/*!< (Simplified JSON) When converting from RWF to JSON, add a QoS range on requests that do not specify a Qos (value: RsslBool). */
	RSSL_JSON_CPC_EXPAND_ENUM_FIELDS			= 5,	/*!< (Simplified JSON) Expand enumerated values in field entries to their display values. Dictionary must have enumerations loaded (value: RsslBool). */
	RSSL_JSON_CPC_REAL_AS_EXPONENT				= 6,	/*!< (Standard JSON) Convert RsslReals to an exponent primitive (value: RsslBool). */
	RSSL_JSON_CPC_CATCH_UNKNOWN_JSON_KEYS		= 7,	/*!< (Simplified JSON) When converting from JSON to RWF, catch unknown JSON keys.  (value: RsslBool). */
	RSSL_JSON_CPC_CATCH_UNKNOWN_JSON_FIDS		= 8,	/*!< (Simplified JSON) When converting from JSON to RWF, catch unknown FIDS.  (value: RsslBool). */
	RSSL_JSON_CPC_ALLOW_ENUM_DISPLAY_STRINGS	= 9		/*!< (Simplified JSON) When fields with enum display strings are encountered in JSON, treat them as blank when converting to RWF instead of returning an error (value: RsslBool). */
} RsslJsonConverterPropertyCodes;

/**
 * @brief Change properties of the RSSL/JSON converter.
 * @param pConverter Converter object that will convert between JSON and RSSL.
 * @param code Code indicating the property to change. See RsslJsonConverterPropertyCodes.
 * @param value Pointer to the value to set the property to. See RsslJsonConverterPropertyCodes for the appropriate type for each value.
 * @param pError Error structure that will be populated with additional information if an error occurs.
 * @return RSSL_RET_SUCCESS when successful, other codes if an error occurred.
 */
RSSL_RJC_API RsslRet rsslJsonConverterSetProperty(RsslJsonConverter pConverter, RsslUInt32 code, void* value, RsslJsonConverterError *pError);

/**
 * @brief Flags indicating behaviors when calling rsslDecodeJsonMsg.
 * @see RsslDecodeJsonMsgOptions
 */
typedef enum
{
	RSSL_JSON_DJMF_NONE = 0x0	/*!< None */
} RsslDecodeJsonMsgFlags;

/**
 * @brief Types of JSON messages.
 * @see rsslDecodeJsonMsg
 */
typedef enum
{
	RSSL_JSON_MC_RSSL_MSG	= 1,	/*!< RWF-converted message. */
	RSSL_JSON_MC_PING		= 2,	/*!< Ping message. */
	RSSL_JSON_MC_PONG		= 3,	/*!< Pong message. */
	RSSL_JSON_MC_ERROR		= 4		/*!< Error message. */
} RsslJsonMsgClasses;

/** @brief Common base for parsed JSON messages. */
typedef struct
{
	RsslUInt8	msgClass;		/*!< Class of message. Populated by JsonMsgClasses. */
	RsslBuffer	jsonMsgBuffer;	/*!< Buffer containing the JSON of this individual message. */
} RsslJsonMsgBase;

/**
 * @brief A message converted to RWF.
 * @see RsslJsonMsg
 */
typedef struct
{
	RsslJsonMsgBase	msgBase;	/* Members common among message types. */
	RsslMsg			rsslMsg;	/* An RWF-converted message. */
} RsslJsonRsslMsg;

/**
 * @brief Union of different JSON messages.
 * @see RsslJsonMsg
 */
typedef union
{
	RsslJsonMsgBase	msgBase;		/* Members common among message types. */
	RsslJsonRsslMsg	jsonRsslMsg;	/* An RWF-converted message. */
} RsslJsonMsg;

/** @brief Clears an RsslJsonMsg structure. */
RTR_C_INLINE  void rsslClearJsonMsg(RsslJsonMsg *pJsonMsg)
{
	memset(pJsonMsg, 0, sizeof(RsslJsonMsg));
}

/**
 * @brief Defines JSON Protocol Types used over Websocket connections.
 */
typedef enum
{
	RSSL_JSON_JPT_UNKNOWN	= 0,	/*!< Unknown protocol type. */
	RSSL_JSON_JPT_JSON		= 1,	/*!< Standard JSON protocol. */
	RSSL_JSON_JPT_JSON2		= 2		/*!< Simplified JSON protocol. */
} RsslJsonProtocolType;

/**
 * @brief Flags indicating behaviors when calling rsslParseJsonBuffer.
 * @see RsslParseJsonBufferOptions
 */
typedef enum
{
	RSSL_JSON_PJBF_NONE = 0x0	/*!< None */
} RsslParseJsonBufferFlags;

/**
 * @brief Options for rsslParseJsonBuffer.
 */
typedef struct
{
	RsslUInt32	converterFlags;		/*!< Flags. See RsslParseJsonBufferFlags. */
	RsslUInt8	jsonProtocolType; /*!< Protocol type of the buffer. See RsslJsonProtocolType. */
} RsslParseJsonBufferOptions;

/**
 * @brief Clears an RsslDecodeJsonMsgOptions structure.
 */
RTR_C_INLINE void rsslClearParseJsonBufferOptions(RsslParseJsonBufferOptions *pOptions)
{
	memset(pOptions, 0, sizeof(RsslParseJsonBufferOptions));
}

/**
 * @brief Set a buffer on the converter and parse it.
 * If this call is successful, individual messages are decoded by calling rsslDecodeJsonMsg.
 * @param pOptions Options structure.
 * @param pConverter Converter object that will convert between JSON and RSSL.
 * @param pJsonBuffer Input buffer containing JSON-formatted message. RsslBuffer::data should point to the message to convert, and RsslBuffer::length should be set to the length of that message.
 * @param pError Error structure that will be populated with additional information if an error occurs.
 * @return RSSL_RET_SUCCESS on successful conversion, other codes if an error occurred.
 */
RSSL_RJC_API RsslRet rsslParseJsonBuffer(RsslJsonConverter pConverter, RsslParseJsonBufferOptions *pOptions, RsslBuffer *pJsonBuffer, RsslJsonConverterError *pError);

/**
 * @brief Options for rsslDecodeJsonMsg.
 */
typedef struct
{
	RsslUInt32	converterFlags;		/*!< Flags. See RsslDecodeJsonMsgFlags. */
	RsslUInt8	jsonProtocolType;	/*!< Protocol of the JSON message. */
	RsslUInt8	rsslProtocolType;	/*!< Protocol of the RSSL message. */
	RsslUInt8	rsslMajorVersion;	/*!< RSSL major version. */
	RsslUInt8	rsslMinorVersion;	/*!< RSSL minor version. */
 } RsslDecodeJsonMsgOptions;

/**
 * @brief Clears an RsslDecodeJsonMsgOptions structure.
 */
RTR_C_INLINE void rsslClearDecodeJsonMsgOptions(RsslDecodeJsonMsgOptions *pOptions)
{
	memset(pOptions, 0, sizeof(RsslDecodeJsonMsgOptions));
}

/**
 * @brief Convert a message inside the JSON buffer to RSSL. Call after rsslParseJsonBuffer.
 * @param pConverter Converter object that will convert between JSON and RSSL.
 * @param pOptions Options structure.
 * @param pJsonMsg The decoded message. RsslJsonMsg.msgBase.msgClass indicates if the message was converted to RWF, or if it was another type of message such as a Ping or Pong.
 * @param pRsslBuffer Output buffer to be populated with the encoded RsslMsg, if the message was converted to RWF. RsslBuffer::data will be set to the converter's buffer memory.
 * @param pError Error structure that will be populated with additional information if an error occurs.
 * @return RSSL_RET_SUCCESS if a message was successfully decoded, RSSL_RET_END_OF_CONTAINER when no more messages are present in the buffer, or other codes if an error occurred.
 */
RSSL_RJC_API RsslRet rsslDecodeJsonMsg(RsslJsonConverter pConverter, RsslDecodeJsonMsgOptions *pOptions, RsslJsonMsg *pJsonMsg, RsslBuffer *pRsslBuffer, RsslJsonConverterError *pError);

/**
 * @brief Flags indicating behaviors when calling rsslConvertRsslMsgToJson.
 * @see RsslConvertRsslMsgToJsonOptions
 */
typedef enum
{
	RSSL_JSON_RJCF_NONE					= 0x0,	/*!< None */
} RsslConvertRsslMsgToJsonFlags;

/**
 * @brief Options for rsslConvertRsslMsgToJson.
 */
typedef struct
{
	RsslUInt32								converterFlags;						/*!< Flags. See RsslConvertJsonToRsslFlags. */
	RsslUInt8								jsonProtocolType;					/*!< Protocol of the JSON message. */
	RsslUInt8								rsslMajorVersion;					/*!< RSSL major version. */
	RsslUInt8								rsslMinorVersion;					/*!< RSSL minor version. */
} RsslConvertRsslMsgToJsonOptions;

/**
 * @brief Clears an RsslConvertRsslMsgToJsonOptions structure.
 */
RTR_C_INLINE void rsslClearConvertRsslMsgToJsonOptions(RsslConvertRsslMsgToJsonOptions *pOptions)
{
	memset(pOptions, 0, sizeof(RsslConvertRsslMsgToJsonOptions));
}

/**
 * @brief Convert a buffer containing RSSL to JSON. Upon successful conversion, buffer can be retrieved and modified via rsslGetConverterJsonMsg.
 * @param pConverter Converter object that will convert between JSON and RSSL.
 * @param pOptions Options structure.
 * @param pRsslMsg The RsslMsg to be converted to JSON.
 * @param pError Error structure that will be populated with additional information if an error occurs.
 * @return RSSL_RET_SUCCESS on successful conversion, other codes if an error occurred.
 */
RSSL_RJC_API RsslRet rsslConvertRsslMsgToJson(RsslJsonConverter pConverter, RsslConvertRsslMsgToJsonOptions *pOptions, RsslMsg *pRsslMsg, RsslJsonConverterError *pError);

/**
 * @brief Enumerations for RsslGetJsonMsgOptions::transportProtocol
 */
typedef enum
{
	RSSL_JSON_TP_AJAX	= 0,	/*!< Ajax connection */
	RSSL_JSON_TP_WS		= 1		/*!< Websocket connection */
} RsslJsonTransportProtocols;

/**
 * @brief Options for rsslGetConverterJsonMsg.
 */
typedef struct
{
	RsslInt32	streamId;				/*!< Stream ID to set on the message. */
	RsslUInt8	transportProtocol;		/*!< Transport protocol. */
	RsslUInt8	jsonProtocolType;		/*!< Protocol type of the converted message. */
	RsslBool 	solicited;				/*!< Set the message as solicited or unsolicited, if applicable (standard JSON only). */
	RsslBool	isCloseMsg;				/*!< Is the message a CloseMsg. */
} RsslGetJsonMsgOptions;

/**
 * @brief Clears an RsslGetJsonMsgOptions structure.
 */
RTR_C_INLINE void rsslClearGetJsonMsgOptions(RsslGetJsonMsgOptions *pOptions)
{
	memset(pOptions, 0, sizeof(RsslGetJsonMsgOptions));
	pOptions->transportProtocol = RSSL_JSON_TP_WS;
	pOptions->solicited = RSSL_TRUE;
}

/**
 * @brief Parameters for containing Json error values
 */
typedef struct
{
	char *errorText;        /*!< Text of Error message. */
	const char *errorFile;   /*!< Error file of message. */
	RsslInt32 *errorLine;    /*!< Error line of message. */
	RsslInt32 *errorOffset;  /*!< Error offset of original message. */
	RsslBuffer *errorOriginalMessage;  /*!< Original message that caused error. */
	RsslInt32 errorStreamId;  /*!< Stream Id of original message or 0 if not obtained. */
} RsslGetJsonErrorParams;

/**
 * @brief Clears an RsslGetJsonErrorParams structure.
 */
RTR_C_INLINE void rsslClearRsslGetJsonErrorParams(RsslGetJsonErrorParams *pErrorParams)
{
	memset(pErrorParams, 0, sizeof(RsslGetJsonErrorParams));
	pErrorParams->errorStreamId = 0;
}

/**
 * @brief Get the last message converted by rsslConvertRsslMsgToJson, modified according to the given options.
 * @param pConverter Converter object that will convert between JSON and RSSL.
 * @param pOptions Options structure.
 * @param pOutBuffer Output buffer to store the copied JSON-formatted message. RsslBuffer::data will be set to the converter's buffer memory.
 * @param pError Error structure that will be populated with additional information if an error occurs.
 * @return RSSL_RET_SUCCESS on successful conversion, other codes if an error occurred.
 */
RSSL_RJC_API RsslRet rsslGetConverterJsonMsg(RsslJsonConverter pConverter, RsslGetJsonMsgOptions *pOptions, RsslBuffer *pOutBuffer, RsslJsonConverterError *pError);

/**
 * @brief Options for rsslJsonBuildTerminateStream.
 */
typedef struct
{
	RsslInt32 errorCode;	/*!< Error code to use in message. */
	RsslBuffer text;		/*!< Text to include in message. */
} RsslJsonBuildTerminateStreamOptions;

/**
 * @brief Clears an RsslJsonBuildTerminateStreamOptions structure.
 */
RTR_C_INLINE void rsslClearJsonBuildTerminateStreamOptions(RsslJsonBuildTerminateStreamOptions *pOptions)
{
	memset(pOptions, 0, sizeof(RsslJsonBuildTerminateStreamOptions));
}

/**
 * @brief Builds a JSON message for terminating a stream. For Ajax connections only.
 * @param pConverter Converter object that will convert between JSON and RSSL.
 * @param pOutBuffer Output buffer to store the copied JSON-formatted message. RsslBuffer::data will be set to the converter's buffer memory.
 * @param pOptions Options structure.
 * @param pError Error structure that will be populated with additional information if an error occurs.
 **/
RSSL_RJC_API RsslRet rsslJsonBuildTerminateStream(RsslJsonConverter pConverter, RsslJsonBuildTerminateStreamOptions *pOptions, RsslBuffer *pOutBuffer, RsslJsonConverterError *pError);

/**
 * @brief Populates an RsslGetJsonErrorParam structure with current error state values
 * @param pConverter Converter object that will convert between JSON and RSSL.
 * @param pOptions Options structure.
 * @param pError Error structure that will be populated with additional information if an error occurs.
 * @param pErrorParams Structure to contain error values
 * @param jsonMsg Original json message that fired the error
 * @param streamId Original stream ID of json message that fired the error (or 0 if not processed)
 **/
RSSL_RJC_API RsslRet rsslGetJsonSimpleErrorParams(RsslJsonConverter pConverter, RsslDecodeJsonMsgOptions *pOptions, RsslJsonConverterError *pError, RsslGetJsonErrorParams *pErrorParams, RsslBuffer *jsonMsg, RsslInt32 streamId);

/**
 * @brief Builds an error message for sending back to client
 * @param pConverter Converter object that will convert between JSON and RSSL.
 * @param pErrorParams Structure containing error values
 * @param pOutBuffer Output buffer to store the copied JSON-formatted message. RsslBuffer::data will be set to the converter's buffer memory.
 **/
RSSL_RJC_API RsslRet rsslJsonGetErrorMessage(RsslJsonConverter pConverter, RsslGetJsonErrorParams *pErrorParams, RsslBuffer *pOutBuffer);

/**
 * @brief Programmatically extracts library and product version information that is compiled into this library
 *
 * User can call this function to programmatically extract version information, or <BR>
 * query version information externally (via 'strings' command or something similar<BR>
 * and grep for the following tag:<BR>
 * 'VERSION' - contains internal library version information (e.g. rssl1.4.F2)<BR>
 * 'PACKAGE' - contains product information for load/package naming<BR>
 * @param pVerInfo RsslLibraryVersionInfo structure to populate with library version information
 * @see RsslLibraryVersionInfo
 */
RSSL_RJC_API void rsslQueryJsonConverterLibraryVersion(RsslLibraryVersionInfo *pVerInfo);

/**
* @}
*/

#ifdef __cplusplus
}
#endif


#endif
