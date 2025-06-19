/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2020,2024 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

#ifndef __RSSLJCONVERTER_H
#define __RSSLJCONVERTER_H

#include "rtr/rsslJsonConverter.h"
#include "rtr/rsslTransport.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef RsslRet rsslJsonSessionServiceNameToIdCallback(RsslBuffer* , RsslUInt16* ); 

typedef struct {
	rsslJsonSessionServiceNameToIdCallback	*pServiceNameToIdCallback;	/*!< Callback function that handles conversion from service name to ID. */
	RsslDataDictionary			*pDictionary;		/*!< the RsslDataDictionary to initialize the RWF/JSON converter. */
	RsslUInt16					defaultServiceId;	/*!< Specify a default service ID for a request if both service name and ID are not set. */
	RsslBool					jsonExpandedEnumFields;		/*!< Expand enumerated values in field entries to their display values for JSON protocol. */
	void						*userSpecPtr; 	/*!< user-specified pointer retrieved in the callback function. */
} RsslJsonSessionOptions;

typedef struct {
	RsslDecodeJsonMsgOptions	decodeOptions;
	RsslParseJsonBufferOptions	parseOptions;
	RsslJsonMsg					jsonMsg;
	RsslBool					failedToConvertJSONMsg;
	RsslBuffer					*buffer;
} RsslJsonSessionState;

typedef struct {
	RsslJsonSessionOptions		options;
	RsslDataDictionary		**pDictionaryList; /* Creates a list of pointer to pointer with the size of 1. */
	RsslBool				jsonSessionInitialized; 	/* This is used to indicate whether the RsslJsonConverter is initialized */
	RsslJsonSessionState		state;
	RsslJsonConverter		*pJsonConverter; 
} RsslJsonSession;

RTR_C_INLINE void rsslJCClearSession(RsslJsonSession *rjcSession)
{
	memset(rjcSession, 0, sizeof(RsslJsonSession));
}

RsslRet rsslJsonSessionResetState(RsslJsonSession *sess, RsslBuffer *pBuffer, RsslError *pError);
void rsslJsonSessionUninitialize(RsslJsonSession *rjcSession);
RsslRet rsslJsonSessionInitialize(RsslJsonSession *rjcSession, RsslError *pError);
RsslRet rsslJsonSessionSetDictionary(RsslJsonSession *rjcSession, RsslDataDictionary* dataDict, RsslError *pError);
RsslBuffer *rsslJsonSessionMsgConvertToJson(RsslJsonSession *rjcSession, RsslChannel *pChannel, 
								RsslBuffer *rwfBuffer, RsslError *pError);
RsslRet rsslJsonSessionMsgConvertFromJson(RsslJsonSession *rjcSession, RsslChannel *pChannel,
								RsslBuffer *rwfBuffer, RsslError *pError);
#ifdef __cplusplus
};
#endif

#endif
