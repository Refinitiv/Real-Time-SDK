/*
 * This source code is provided under the Apache 2.0 license and is provided
 * AS IS with no warranty or guarantee of fit for purpose.  See the project's 
 * LICENSE.md for details. 
 * Copyright (C) 2020 Refinitiv. All rights reserved.
*/

#ifndef __RJCONVERTER_H
#define __RJCONVERTER_H

#include "rtr/rsslErrorInfo.h"
#include "rtr/rsslJsonConverter.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef RsslRet rjcServiceNameToIdCallback(RsslBuffer* , RsslUInt16* ); 

typedef struct {
	rjcServiceNameToIdCallback	*pServiceNameToIdCallback;	/*!< Callback function that handles conversion from service name to ID. */
	RsslDataDictionary			*pDictionary;		/*!< the RsslDataDictionary to initialize the RWF/JSON converter. */
	RsslUInt16					defaultServiceId;	/*!< Specify a default service ID for a request if both service name and ID are not set. */
	RsslBool					jsonExpandedEnumFields;		/*!< Expand enumerated values in field entries to their display values for JSON protocol. */
	void						*userSpecPtr; 	/*!< user-specified pointer retrieved in the callback function. */
} rjConverterOptions;

typedef struct {
	RsslDecodeJsonMsgOptions	decodeOptions;
	RsslParseJsonBufferOptions	parseOptions;
	RsslJsonMsg					jsonMsg;
	RsslBool					failedToConvertJSONMsg;
	RsslBuffer					*buffer;
} rjConverterState;

typedef struct {
	rjConverterOptions		options;
	RsslDataDictionary		**pDictionaryList; /* Creates a list of pointer to pointer with the size of 1. */
	RsslBool				jsonConverterInitialized; 	/* This is used to indicate whether the RsslJsonConverter is initialized */
	rjConverterState		state;
	RsslJsonConverter		*pJsonConverter; 
} rjConverterSession;

RTR_C_INLINE void rjcClearSession(rjConverterSession *rjcSession)
{
	memset(rjcSession, 0, sizeof(rjConverterSession));
}

RsslRet rjcResetConverterState(rjConverterSession *sess, RsslBuffer *pBuffer, RsslErrorInfo *pError);
void rjcSessionUninitialize(rjConverterSession *rjcSession);
RsslRet rjcSessionInitialize(rjConverterSession *rjcSession, RsslErrorInfo *pError);
RsslBuffer *rjcMsgConvertToJson(rjConverterSession *rjcSession, RsslChannel *pChannel, 
								RsslBuffer *rwfBuffer, RsslErrorInfo *pError);
RsslRet rjcMsgConvertFromJson(rjConverterSession *rjcSession, RsslChannel *pChannel,
								RsslBuffer *rwfBuffer, RsslBuffer *jsonBuffer, RsslErrorInfo *pError);


#ifdef __cplusplus
};
#endif

#endif
