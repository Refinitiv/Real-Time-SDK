
/*
 * This is the dictionary provider for both the rsslProvider and
 * rsslNIProvider applications.  Only two dictionary streams (field
 * and enum type) per channel are allowed by this simple provider.
 * It provides functions for processing dictionary requests from
 * consumers and sending back the responses.  Functions for sending
 * dictionary request reject/close status messages, initializing the
 * dictionary provider, loading dictionaries from files, getting the
 * loaded dictionary, and closing dictionary streams are also provided.
 */

#include "rsslDictionaryProvider.h"
#include "rsslSendMessage.h"
#if defined(_WIN32)
#include <windows.h>
#else
#include <time.h>
#include <sys/time.h>
#endif

/* remember the StreamID of the field/enum refreshes and use that to determine the dictionary type */
static RsslInt32 fieldDictionaryStreamId = 0, enumDictionaryStreamId = 0;
/* enum table loaded flag */
static RsslBool enumTypeDictionaryLoaded = RSSL_FALSE;
/* dictionary loaded flag */
static RsslBool fieldDictionaryLoaded = RSSL_FALSE;
/* data dictionary */
static RsslDataDictionary dictionary;
/* field dictionary file name  */
static const char *fieldDictionaryFileName = "RDMFieldDictionary";
/* field dictionary name */
static const char *fieldDictionaryDownloadName = "RWFFld";
/* enum type dictionary file name  */
static const char *enumTypeDictionaryFileName = "enumtype.def";
/* enumtype dictionary name */
static const char *enumTypeDictionaryDownloadName = "RWFEnum";

/* dictionary request information */
static RsslDictionaryRequestInfo dictionaryRequestInfoList[MAX_DICTIONARY_REQUESTS];

#ifdef __cplusplus
extern "C" {
#endif
RsslUInt64 getServiceId();
#ifdef __cplusplus
};
#endif

/*
 * Returns whether or not field dictionary has been loaded
 */
RsslBool isFieldDictionaryLoaded()
{
	return fieldDictionaryLoaded;
}

/*
 * Returns whether or not the enumeration types dictionary has been loaded
 */
RsslBool isEnumTypeDictionaryLoaded()
{
	return enumTypeDictionaryLoaded;
}

/*
 * Initializes dictionary information fields.
 */
void initDictionaryProvider()
{
	int i;

	for(i = 0; i < MAX_DICTIONARY_REQUESTS; ++i)
	{
		clearDictionaryReqInfo(&dictionaryRequestInfoList[i]);
	}
}

/*
 * Loads dictionary files
 */
RsslRet loadDictionary()
{
	RsslRet retVal = RSSL_RET_SUCCESS;
	char errTxt[256];
	RsslBuffer errorText = {255, (char*)errTxt};

	rsslClearDataDictionary(&dictionary);

	/* Add field information to dictionary */
	if (rsslLoadFieldDictionary(fieldDictionaryFileName, &dictionary, &errorText) < 0)
	{
		printf("\nUnable to load field dictionary: %s.\n\tError Text: %s\n", fieldDictionaryFileName, errorText.data);
		retVal = RSSL_RET_FAILURE;
	}
	else
		fieldDictionaryLoaded = RSSL_TRUE;


	/* Add enumerated types information to dictionary */
	if (rsslLoadEnumTypeDictionary(enumTypeDictionaryFileName, &dictionary, &errorText) < 0)
	{
		printf("\nUnable to load enum type dictionary: %s.\n\tError Text: %s\n", enumTypeDictionaryFileName, errorText.data);
		retVal = RSSL_RET_FAILURE;
	}
	else
		enumTypeDictionaryLoaded = RSSL_TRUE;

	return retVal;
}

/*
 * Returns the data dictionary.
 */
RsslDataDictionary* getDictionary()
{
	return &dictionary;
}

/*
 * Gets a dictionary request information structure for a channel.
 * chnl - The channel to get the dictionary request information structure for
 * msg - The partially decoded message
 * key - The message key
 */
static RsslDictionaryRequestInfo* getDictionaryReqInfo(RsslChannel* chnl, RsslMsg* msg, RsslMsgKey* key)
{
	int i;
	RsslDictionaryRequestInfo* dictionaryRequestInfo = NULL;
	RsslBuffer tempBuffer;

	/* first check if one already in use for this channel and stream id */
	for (i = 0; i < MAX_DICTIONARY_REQUESTS; i++)
	{
		tempBuffer.data = dictionaryRequestInfoList[i].DictionaryName;
		tempBuffer.length = (RsslUInt32)strlen(dictionaryRequestInfoList[i].DictionaryName);
		if (dictionaryRequestInfoList[i].IsInUse &&
			dictionaryRequestInfoList[i].Chnl == chnl &&
			dictionaryRequestInfoList[i].StreamId == msg->msgBase.streamId)
		{
			dictionaryRequestInfo = &dictionaryRequestInfoList[i];
			/* if dictionary name is different from last request, this is an invalid request */
			if (!rsslBufferIsEqual(&key->name, &tempBuffer))
			{
				return NULL;
			}
			break;
		}
	}

	/* get a new one if one is not already in use */
	if (!dictionaryRequestInfo)
	{
		for (i = 0; i < MAX_DICTIONARY_REQUESTS; i++)
		{
			if(dictionaryRequestInfoList[i].IsInUse == RSSL_FALSE)
			{
				dictionaryRequestInfoList[i].Chnl = chnl;
				dictionaryRequestInfoList[i].IsInUse = RSSL_TRUE;
				dictionaryRequestInfo = &dictionaryRequestInfoList[i];
				if (rsslCopyMsgKey(&dictionaryRequestInfo->MsgKey, key) == RSSL_RET_FAILURE)
				{
					return NULL;
				}
				break;
			}
		}
	}

	return dictionaryRequestInfo;
}

/*
 * Encodes the field dictionary response.  Returns success if encoding
 * succeeds or failure if encoding fails.
 * chnl - The channel to send a dictionary response to
 * dictionaryReqInfo - The dictionary request information
 * msgBuf - The message buffer to encode the dictionary response into
 * dictionaryFid - The starting dictionary fid for the encode
 * firstPartMultiPartRefresh - flag indicating first part of potential multi part refresh message
 */
static RsslRet encodeFieldDictionaryResponse(RsslChannel* chnl, RsslDictionaryRequestInfo* dictionaryReqInfo, RsslBuffer* msgBuf, RsslInt32* dictionaryFid, RsslBool firstPartMultiPartRefresh)
{
	RsslRet ret = 0;
	RsslRefreshMsg msg = RSSL_INIT_REFRESH_MSG;
	char stateText[MAX_DICTIONARY_REQ_INFO_STRLEN];
	char errTxt[256];
	RsslBuffer errorText = {255, (char*)errTxt};
	RsslBool dictionaryComplete = RSSL_FALSE;
	RsslEncodeIterator encodeIter;

	/* clear encode iterator */
	rsslClearEncodeIterator(&encodeIter);

	/* set-up message */
	msg.msgBase.msgClass = RSSL_MC_REFRESH;
	msg.msgBase.domainType = RSSL_DMT_DICTIONARY;
	msg.msgBase.containerType = RSSL_DT_SERIES;
	msg.state.streamState = RSSL_STREAM_OPEN;
	msg.state.dataState = RSSL_DATA_OK;
	msg.state.code = RSSL_SC_NONE;
	msg.msgBase.msgKey.filter = dictionaryReqInfo->MsgKey.filter;
	msg.msgBase.msgKey.flags = RSSL_MKF_HAS_NAME | RSSL_MKF_HAS_FILTER | RSSL_MKF_HAS_SERVICE_ID;
	msg.msgBase.msgKey.serviceId = dictionaryReqInfo->serviceId;
	msg.flags = RSSL_RFMF_HAS_MSG_KEY | RSSL_RFMF_SOLICITED;

	/* when doing a multi part refresh only the first part has the RSSL_RFMF_CLEAR_CACHE flag set */
	if (firstPartMultiPartRefresh)
	{
		msg.flags |= RSSL_RFMF_CLEAR_CACHE;
	}

	snprintf(stateText, MAX_DICTIONARY_REQ_INFO_STRLEN, "Field Dictionary Refresh (starting fid %d)", *dictionaryFid);
	msg.state.text.data = stateText;
	msg.state.text.length = (RsslUInt32)strlen(stateText) + 1;
	
	/* DictionaryName */
	msg.msgBase.msgKey.name.data = dictionaryReqInfo->DictionaryName;
	msg.msgBase.msgKey.name.length = (RsslUInt32)strlen(dictionaryReqInfo->DictionaryName);

	/* StreamId */
	msg.msgBase.streamId = dictionaryReqInfo->StreamId;

	/* encode message */
	if((ret = rsslSetEncodeIteratorBuffer(&encodeIter, msgBuf)) < RSSL_RET_SUCCESS)
	{
		printf("rsslSetEncodeIteratorBuffer() failed with return code: %d\n", ret);
		return ret;
	}
	rsslSetEncodeIteratorRWFVersion(&encodeIter, chnl->majorVersion, chnl->minorVersion);
	if ((ret = rsslEncodeMsgInit(&encodeIter, (RsslMsg*)&msg, 0)) < RSSL_RET_SUCCESS)
	{
		printf("rsslEncodeMsgInit() failed with return code: %d\n", ret);
		return ret;
	}

	/* encode dictionary into message */
	if ((ret = rsslEncodeFieldDictionary(&encodeIter, &dictionary, dictionaryFid, (RDMDictionaryVerbosityValues)dictionaryReqInfo->MsgKey.filter, &errorText)) != RSSL_RET_SUCCESS)
	{
		if (ret != RSSL_RET_DICT_PART_ENCODED) /* dictionary encode failed */
		{
			printf("rsslEncodeFieldDictionary() failed '%s'\n",errorText.data );
			return ret;
		}
	}
	else /* dictionary encode complete */
	{
		dictionaryComplete = RSSL_TRUE;
		/* set refresh complete flag */
		rsslSetRefreshCompleteFlag(&encodeIter);
	}
	
	/* complete encode message */
	if ((ret = rsslEncodeMsgComplete(&encodeIter, RSSL_TRUE)) < RSSL_RET_SUCCESS)
	{
		printf("rsslEncodeMsgComplete() failed with return code: %d\n", ret);
		return ret;
	}
	msgBuf->length = rsslGetEncodedBufferLength(&encodeIter);

	if (dictionaryComplete)
	{
		return RSSL_RET_SUCCESS;
	}
	else
	{
		return RSSL_RET_DICT_PART_ENCODED;
	}
}

/*
 * Sends a dictionary request to a channel.  This consists of
 * getting a message buffer, encoding the dictionary request,
 * and sending the dictionary request to the server.
 * chnl - The channel to send a dictionary request to
 * dictionaryName - The name of the dictionary to request
 * streamId - The stream id of the dictionary request 
 */
RsslRet sendDictionaryRequest(RsslChannel* chnl, const char *dictionaryName, RsslInt32 streamId)
{
	RsslError error;
	RsslBuffer* msgBuf = 0;

	/* get a buffer for the dictionary request */
	msgBuf = rsslGetBuffer(chnl, MAX_MSG_SIZE, RSSL_FALSE, &error);

	if (msgBuf != NULL)
	{
		/* encode dictionary request */
		if (encodeDictionaryRequest(chnl, msgBuf, dictionaryName, streamId) != RSSL_RET_SUCCESS)
		{
			rsslReleaseBuffer(msgBuf, &error);
			printf("\nencodeDictionaryRequest() failed\n");
			return RSSL_RET_FAILURE;
		}

		/* send request */
		if (sendMessage(chnl, msgBuf) != RSSL_RET_SUCCESS)
			return RSSL_RET_FAILURE;
	}
	else
	{
		printf("rsslGetBuffer(): Failed <%s>\n", error.text);
		return RSSL_RET_FAILURE;
	}

	return RSSL_RET_SUCCESS;
}

/*
 * Sends both field and enumType dictionary requests to a channel. 
 * chnl - The channel to send a dictionary requests to
 */
RsslRet sendDictionaryRequests(RsslChannel* chnl)
{
	if (sendDictionaryRequest(chnl, fieldDictionaryDownloadName, FIELD_DICTIONARY_STREAM_ID) != RSSL_RET_SUCCESS)
		return RSSL_RET_FAILURE;

	if (sendDictionaryRequest(chnl, enumTypeDictionaryDownloadName, ENUM_TYPE_DICTIONARY_STREAM_ID) != RSSL_RET_SUCCESS)
		return RSSL_RET_FAILURE;

	return RSSL_RET_SUCCESS;
}

/*
 * Encodes the dictionary request.  Returns success
 * if encoding succeeds or failure if encoding fails.
 * chnl - The channel to send a dictionary request to
 * msgBuf - The message buffer to encode the dictionary request into
 * dictionaryName - The name of the dictionary to request
 * streamId - The stream id of the dictionary request 
 */
static RsslRet encodeDictionaryRequest(RsslChannel* chnl, RsslBuffer* msgBuf, const char *dictionaryName, RsslInt32 streamId)
{
	RsslRet ret = 0;
	RsslRequestMsg msg = RSSL_INIT_REQUEST_MSG;
	RsslEncodeIterator encodeIter;

	/* clear encode iterator */
	rsslClearEncodeIterator(&encodeIter);

	/* set-up message */
	msg.msgBase.msgClass = RSSL_MC_REQUEST;
	msg.msgBase.streamId = streamId;
	msg.msgBase.domainType = RSSL_DMT_DICTIONARY;
	msg.msgBase.containerType = RSSL_DT_NO_DATA;
	msg.flags = RSSL_RQMF_NONE;

	/* set msgKey members */
	msg.msgBase.msgKey.flags = RSSL_MKF_HAS_FILTER | RSSL_MKF_HAS_NAME_TYPE | RSSL_MKF_HAS_NAME | RSSL_MKF_HAS_SERVICE_ID;
	msg.msgBase.msgKey.nameType = RDM_INSTRUMENT_NAME_TYPE_RIC;
	msg.msgBase.msgKey.name.data = (char *)dictionaryName;
	msg.msgBase.msgKey.name.length = (RsslUInt32)strlen(dictionaryName);
	msg.msgBase.msgKey.serviceId = (RsslUInt16)getServiceId();
	msg.msgBase.msgKey.filter = RDM_DICTIONARY_VERBOSE;

	/* encode message */
	if((ret = rsslSetEncodeIteratorBuffer(&encodeIter, msgBuf)) < RSSL_RET_SUCCESS)
	{
		printf("rsslSetEncodeIteratorBuffer() failed with return code: %d\n", ret);
		return ret;
	}
	rsslSetEncodeIteratorRWFVersion(&encodeIter, chnl->majorVersion, chnl->minorVersion);
	if ((ret = rsslEncodeMsg(&encodeIter, (RsslMsg*)&msg)) < RSSL_RET_SUCCESS)
	{
		printf("rsslEncodeMsg() failed with return code: %d\n", ret);
		return ret;
	}

	msgBuf->length = rsslGetEncodedBufferLength(&encodeIter);

	return RSSL_RET_SUCCESS;
}


/*
 * Encodes the enum type dictionary response.  Returns success if encoding
 * succeeds or failure if encoding fails.
 * chnl - The channel to send a dictionary response to
 * dictionaryReqInfo - The dictionary request information
 * msgBuf - The message buffer to encode the dictionary response into
 */
static RsslRet encodeEnumTypeDictionaryResponse(RsslChannel* chnl, RsslDictionaryRequestInfo* dictionaryReqInfo, RsslInt32* dictionaryFid, RsslBuffer* msgBuf, RsslBool firstPartMultiPartRefresh)
{
	RsslRet ret = 0;
	RsslRefreshMsg msg = RSSL_INIT_REFRESH_MSG;
	char stateText[MAX_DICTIONARY_REQ_INFO_STRLEN];
	char errTxt[256];
	RsslBuffer errorText = {255, (char*)errTxt};
	RsslBool dictionaryComplete = RSSL_FALSE;
	RsslEncodeIterator encodeIter;

	/* clear encode iterator */
	rsslClearEncodeIterator(&encodeIter);

	/* set-up message */
	msg.msgBase.msgClass = RSSL_MC_REFRESH;
	msg.msgBase.domainType = RSSL_DMT_DICTIONARY;
	msg.msgBase.containerType = RSSL_DT_SERIES;
	msg.state.streamState = RSSL_STREAM_OPEN;
	msg.state.dataState = RSSL_DATA_OK;
	msg.state.code = RSSL_SC_NONE;
	msg.msgBase.msgKey.filter = dictionaryReqInfo->MsgKey.filter;
	msg.msgBase.msgKey.flags = RSSL_MKF_HAS_NAME | RSSL_MKF_HAS_FILTER | RSSL_MKF_HAS_SERVICE_ID;
	msg.msgBase.msgKey.serviceId = dictionaryReqInfo->serviceId;
	msg.flags = RSSL_RFMF_HAS_MSG_KEY | RSSL_RFMF_SOLICITED;
	
	/* when doing a multi part refresh only the first part has the RSSL_RFMF_CLEAR_CACHE flag set */
	if (firstPartMultiPartRefresh)
	{
		msg.flags |= RSSL_RFMF_CLEAR_CACHE;
	}

	snprintf(stateText, MAX_DICTIONARY_REQ_INFO_STRLEN, "Enum Type Dictionary Refresh (starting fid %d)", *dictionaryFid);
	msg.state.text.data = stateText;
	msg.state.text.length = (RsslUInt32)strlen(stateText) + 1;
	
	/* DictionaryName */
	msg.msgBase.msgKey.name.data = dictionaryReqInfo->DictionaryName;
	msg.msgBase.msgKey.name.length = (RsslUInt32)strlen(dictionaryReqInfo->DictionaryName);

	/* StreamId */
	msg.msgBase.streamId = dictionaryReqInfo->StreamId;

	/* encode message */
	if((ret = rsslSetEncodeIteratorBuffer(&encodeIter, msgBuf)) < RSSL_RET_SUCCESS)
	{
		printf("rsslSetEncodeIteratorBuffer() failed with return code: %d\n", ret);
		return ret;
	}
	rsslSetEncodeIteratorRWFVersion(&encodeIter, chnl->majorVersion, chnl->minorVersion);
	if ((ret = rsslEncodeMsgInit(&encodeIter, (RsslMsg*)&msg, 0)) < RSSL_RET_SUCCESS)
	{
		printf("rsslEncodeMsgInit() failed with return code: %d\n", ret);
		return ret;
	}

	if ((ret = rsslEncodeEnumTypeDictionaryAsMultiPart(&encodeIter, &dictionary, dictionaryFid, (RDMDictionaryVerbosityValues)dictionaryReqInfo->MsgKey.filter, &errorText)) != RSSL_RET_SUCCESS)
	{
		if (ret != RSSL_RET_DICT_PART_ENCODED)
		{
			printf("rsslEncodeEnumTypeDictionaryAsMultiPart() failed '%s'\n", errorText.data);
		return ret;
	}
	}
	else /* dictionary encode complete */
	{
		dictionaryComplete = RSSL_TRUE;
		/* set refresh complete flag */
		rsslSetRefreshCompleteFlag(&encodeIter);
	}
	
	/* complete encode message */
	if ((ret = rsslEncodeMsgComplete(&encodeIter, RSSL_TRUE)) < RSSL_RET_SUCCESS)
	{
		printf("rsslEncodeMsgComplete() failed with return code: %d\n", ret);
		return ret;
	}
	msgBuf->length = rsslGetEncodedBufferLength(&encodeIter);

	if (dictionaryComplete)
	{
	return RSSL_RET_SUCCESS;
}
	else
	{
		return RSSL_RET_DICT_PART_ENCODED;
	}
}

/*
 * Processes a dictionary request.  This consists of calling
 * decodeDictionaryRequest() to decode the request and calling
 * sendDictionaryResonse() to send the dictionary response.
 * Returns success if dictionary request processing succeeds
 * or failure if processing fails.
 * chnl - The channel of the request
 * msg - The partially decoded message
 * dIter - The decode iterator
 */
RsslRet processDictionaryRequest(RsslChannel *chnl, RsslMsg* msg, RsslDecodeIterator* dIter)
{
	RsslRet ret;
	RsslMsgKey* key = 0;
	RsslState *pState = 0;
	char	errTxt[256];
	RsslBuffer errorText = {255, (char*)errTxt};
	RsslDictionaryRequestInfo *dictionaryRequestInfo;

	switch(msg->msgBase.msgClass)
	{
	case RSSL_MC_REQUEST:
		/* get key */
		key = (RsslMsgKey *)rsslGetMsgKey(msg);

		dictionaryRequestInfo = getDictionaryReqInfo(chnl, msg, key);

		if (!dictionaryRequestInfo)
		{
			if (sendDictionaryRequestReject(chnl, msg->msgBase.streamId, MAX_DICTIONARY_REQUESTS_REACHED) != RSSL_RET_SUCCESS)
				return RSSL_RET_FAILURE;
			break;
		}

		/* decode dictionary request */
		if ((ret = decodeDictionaryRequest(dictionaryRequestInfo, msg, key, dIter)) != RSSL_RET_SUCCESS)
		{
			printf("\ndecodeDictionaryRequest() failed with return code: %d\n", ret);
			return ret;
		}

		printf("\nReceived Dictionary Request for DictionaryName: %.*s\n", (int)strlen(dictionaryRequestInfo->DictionaryName), dictionaryRequestInfo->DictionaryName);

		if (!strcmp(fieldDictionaryDownloadName, dictionaryRequestInfo->DictionaryName))
		{
			/* send dictionary response */
			if ((ret = sendFieldDictionaryResponse(chnl, dictionaryRequestInfo)) != RSSL_RET_SUCCESS)
			{
				return ret;
			}
		}
		else if (!strcmp(enumTypeDictionaryDownloadName, dictionaryRequestInfo->DictionaryName))
		{
			/* send dictionary response */
			if ((ret = sendEnumTypeDictionaryResponse(chnl, dictionaryRequestInfo)) != RSSL_RET_SUCCESS)
			{
				return ret;
			}
		}
		else
		{
			if (sendDictionaryRequestReject(chnl, msg->msgBase.streamId, UNKNOWN_DICTIONARY_NAME) != RSSL_RET_SUCCESS)
				return RSSL_RET_FAILURE;
			break;
		}
		break;

	case RSSL_MC_CLOSE:
		printf("\nReceived Dictionary Close for StreamId %d\n", msg->msgBase.streamId);

		/* close dictionary stream */
		closeDictionaryStream(msg->msgBase.streamId);
		break;

	default:
		printf("\nReceived Unhandled Dictionary MsgClass: %d\n", msg->msgBase.msgClass);
    	break;
	}

	return RSSL_RET_SUCCESS;
}

/*
 * Decodes the dictionary request into the RsslDictionaryRequestInfo structure.
 * Returns success if decoding succeeds or failure if decoding fails.
 * dictionaryReqInfo - The dictionary request information structure
 * msg - The partially decoded message
 * key - The message key
 * dIter - The decode iterator
 */
static RsslRet decodeDictionaryRequest(RsslDictionaryRequestInfo* dictionaryReqInfo, RsslMsg* msg, RsslMsgKey* key, RsslDecodeIterator* dIter)
{
	/* get StreamId */
	dictionaryReqInfo->StreamId = msg->requestMsg.msgBase.streamId;

	return RSSL_RET_SUCCESS;
}

/*
 * Sends a field dictionary response to a channel.  This consists of getting
 * a message buffer, encoding the dictionary response, and sending the
 * dictionary response to the server.  Returns success if send dictionary
 * response succeeds or failure if send response fails.
 * chnl - The channel to send a dictionary response to
 * dictionaryReqInfo - The dictionary request information
 */
static RsslRet sendFieldDictionaryResponse(RsslChannel* chnl, RsslDictionaryRequestInfo* dictionaryReqInfo)
{
	RsslRet ret;
	RsslError error;
	RsslBuffer* msgBuf = 0;
	RsslInt32 dictionaryFid = RSSL_MIN_FID;
	RsslDataDictionary* dictionary = getDictionary();
	RsslBool firstPart = RSSL_TRUE;

	/* set starting fid to loaded dictionary's minimum fid */
	dictionaryFid = dictionary->minFid;

	while (RSSL_TRUE)
	{
		/* get a buffer for the dictionary response */
		msgBuf = rsslGetBuffer(chnl, MAX_FIELD_DICTIONARY_MSG_SIZE, RSSL_FALSE, &error);

		if (msgBuf != NULL)
		{
			/* encode dictionary response */
			ret = encodeFieldDictionaryResponse(chnl, dictionaryReqInfo, msgBuf, &dictionaryFid, firstPart);
			if (ret < RSSL_RET_SUCCESS)
			{
				rsslReleaseBuffer(msgBuf, &error); 
				printf("\nencodeFieldDictionaryResponse() failed with return code: %d\n", ret);
				return ret;
			}

			firstPart = RSSL_FALSE;

			/* send dictionary response */
			if (sendMessage(chnl, msgBuf) != RSSL_RET_SUCCESS)
				return RSSL_RET_FAILURE;

			/* break out of loop when all dictionary responses sent */
			if (ret == RSSL_RET_SUCCESS)
			{
				break;
			}

			/* sleep between dictionary responses */
#if defined(_WIN32)
			Sleep(1);
#else
			struct timespec sleeptime;
			sleeptime.tv_sec = 0;
			sleeptime.tv_nsec = 1000000;
			nanosleep(&sleeptime,0);
#endif
		}
		else
		{
			printf("rsslGetBuffer(): Failed <%s>\n", error.text);
			return RSSL_RET_FAILURE;
		}
	}

	return RSSL_RET_SUCCESS;
}

/*
 * Sends an enum type dictionary response to a channel.  This consists of getting
 * a message buffer, encoding the dictionary response, and sending the
 * dictionary response to the server.  Returns success if send dictionary
 * response succeeds or failure if send response fails.
 * chnl - The channel to send a dictionary response to
 * dictionaryReqInfo - The dictionary request information
 */
static RsslRet sendEnumTypeDictionaryResponse(RsslChannel* chnl, RsslDictionaryRequestInfo* dictionaryReqInfo)
{
	RsslRet ret;
	RsslError error;
	RsslBuffer* msgBuf = 0;
	RsslInt32 dictionaryFid = 0;
	RsslBool firstPart = RSSL_TRUE;

	while (RSSL_TRUE)
	{
	/* get a buffer for the dictionary response */
	msgBuf = rsslGetBuffer(chnl, MAX_ENUM_TYPE_DICTIONARY_MSG_SIZE, RSSL_FALSE, &error);

	if (msgBuf != NULL)
	{
		/* encode dictionary response */
			ret = encodeEnumTypeDictionaryResponse(chnl, dictionaryReqInfo, &dictionaryFid, msgBuf, firstPart);
		if (ret < RSSL_RET_SUCCESS)
		{
			rsslReleaseBuffer(msgBuf, &error); 
			printf("\nencodeEnumTypeDictionaryResponse() failed with return code: %d\n", ret);
			return ret;
		}

			firstPart = RSSL_FALSE;

		/* send dictionary response */
		if (sendMessage(chnl, msgBuf) != RSSL_RET_SUCCESS)
			return RSSL_RET_FAILURE;

			/* break out of loop when all dictionary responses sent */
			if (ret == RSSL_RET_SUCCESS)
			{
				break;
			}

			/* sleep between dictionary responses */
#if defined(_WIN32)
			Sleep(1);
#else
			struct timespec sleeptime;
			sleeptime.tv_sec = 0;
			sleeptime.tv_nsec = 1000000;
			nanosleep(&sleeptime, 0);
#endif
	}
	else
	{
		printf("rsslGetBuffer(): Failed <%s>\n", error.text);
		return RSSL_RET_FAILURE;
	}
	}

	return RSSL_RET_SUCCESS;
}


/* 
 * Closes all open dictionary streams for a channel. 
 * chnl - The channel to close the dictionary streams for
 */
void closeDictionaryChnlStreams(RsslChannel *chnl)
{
	int i;
	RsslDictionaryRequestInfo* dictionaryReqInfo = NULL;

	/* find original request information associated with chnl */
	for (i = 0; i < MAX_DICTIONARY_REQUESTS; i++)
	{
		if(dictionaryRequestInfoList[i].Chnl == chnl)
		{
			dictionaryReqInfo = &dictionaryRequestInfoList[i];
			/* clear original request information */
			printf("Closing dictionary stream id %d with dictionary name: %.*s\n", dictionaryReqInfo->StreamId, (int)strlen(dictionaryReqInfo->DictionaryName), dictionaryReqInfo->DictionaryName);
			clearDictionaryReqInfo(dictionaryReqInfo);
		}
	}
}

/* 
 * Closes a dictionary stream. 
 * streamId - The stream id to close the dictionary for
 */
static void closeDictionaryStream(RsslInt32 streamId)
{
	int i;
	RsslDictionaryRequestInfo* dictionaryReqInfo = NULL;

	/* find original request information associated with streamId */
	for (i = 0; i < MAX_DICTIONARY_REQUESTS; i++)
	{
		if(dictionaryRequestInfoList[i].StreamId == streamId)
		{
			dictionaryReqInfo = &dictionaryRequestInfoList[i];
			/* clear original request information */
			printf("Closing dictionary stream id %d with dictionary name: %.*s\n", dictionaryReqInfo->StreamId, (int)strlen(dictionaryReqInfo->DictionaryName), dictionaryReqInfo->DictionaryName);
			clearDictionaryReqInfo(dictionaryReqInfo);
			break;
		}
	}
}

/*
 * Sends the dictionary close status message(s) for a channel.
 * This consists of finding all request information for this channel
 * and sending the close status messages to the channel.
 * chnl - The channel to send close status message(s) to
 */
void sendDictionaryCloseStatusMsgs(RsslChannel* chnl)
{
	int i;

	for (i = 0; i < MAX_DICTIONARY_REQUESTS; i++)
	{
		if (dictionaryRequestInfoList[i].IsInUse && dictionaryRequestInfoList[i].Chnl == chnl)
		{
			sendDictionaryCloseStatusMsg(chnl, dictionaryRequestInfoList[i].StreamId);
		}
	}
}

/*
 * Sends the dictionary close status message for a channel.
 * Returns success if send dictionary close status succeeds
 * or failure if it fails.
 * chnl - The channel to send close status message to
 * streamId - The stream id of the close status
 */
RsslRet sendDictionaryCloseStatusMsg(RsslChannel* chnl, RsslInt32 streamId)
{
	RsslRet ret;
	RsslError error;
	RsslBuffer* msgBuf = 0;

	/* get a buffer for the dictionary close status */
	msgBuf = rsslGetBuffer(chnl, MAX_DICTIONARY_STATUS_MSG_SIZE, RSSL_FALSE, &error);

	if (msgBuf != NULL)
	{
		/* encode dictionary close status */
		if ((ret = encodeDictionaryCloseStatus(chnl, msgBuf, streamId)) != RSSL_RET_SUCCESS)
		{
			rsslReleaseBuffer(msgBuf, &error);
			printf("\nencodeDictionaryCloseStatus() failed with return code: %d\n", ret);
			return ret;
		}

		/* send close status */
		if (sendMessage(chnl, msgBuf) != RSSL_RET_SUCCESS) 
			return RSSL_RET_FAILURE;
	}
	else
	{
		printf("rsslGetBuffer(): Failed <%s>\n", error.text);
		return RSSL_RET_FAILURE;
	}

	return RSSL_RET_SUCCESS;
}

/*
 * Encodes the dictionary close status.  Returns success if
 * encoding succeeds or failure if encoding fails.
 * chnl - The channel to send close status message to
 * msgBuf - The message buffer to encode the dictionary close into
 * streamId - The stream id of the close status
 */
static RsslRet encodeDictionaryCloseStatus(RsslChannel *chnl, RsslBuffer* msgBuf, RsslInt32 streamId)
{
	RsslRet ret = 0;
	RsslStatusMsg msg = RSSL_INIT_STATUS_MSG;
	char stateText[MAX_DICTIONARY_REQ_INFO_STRLEN];
	RsslEncodeIterator encodeIter;

	/* clear encode iterator */
	rsslClearEncodeIterator(&encodeIter);

	/* set-up message */
	msg.msgBase.msgClass = RSSL_MC_STATUS;
	msg.msgBase.streamId = streamId;
	msg.msgBase.domainType = RSSL_DMT_DICTIONARY;
	msg.msgBase.containerType = RSSL_DT_NO_DATA;
	msg.flags = RSSL_STMF_HAS_STATE;
	msg.state.streamState = RSSL_STREAM_CLOSED;
	msg.state.dataState = RSSL_DATA_SUSPECT;
	msg.state.code = RSSL_SC_NONE;
	sprintf(stateText, "Dictionary stream closed");
	msg.state.text.data = stateText;
	msg.state.text.length = (RsslUInt32)strlen(stateText) + 1;

	/* encode message */
	if((ret = rsslSetEncodeIteratorBuffer(&encodeIter, msgBuf)) < RSSL_RET_SUCCESS)
	{
		printf("rsslSetEncodeIteratorBuffer() failed with return code: %d\n", ret);
		return ret;
	}
	rsslSetEncodeIteratorRWFVersion(&encodeIter, chnl->majorVersion, chnl->minorVersion);
	if ((ret = rsslEncodeMsg(&encodeIter, (RsslMsg*)&msg)) < RSSL_RET_SUCCESS)
	{
		printf("rsslEncodeMsg() failed with return code: %d\n", ret);
		return ret;
	}

	msgBuf->length = rsslGetEncodedBufferLength(&encodeIter);

	return RSSL_RET_SUCCESS;
}

/*
 * Sends the dictionary request reject status message for a channel.
 * chnl - The channel to send request reject status message to
 * streamId - The stream id of the request
 * reason - The reason for the reject
 */
static RsslRet sendDictionaryRequestReject(RsslChannel* chnl, RsslInt32 streamId, RsslDictionaryRejectReason reason)
{
	RsslError error;
	RsslBuffer* msgBuf = 0;

	/* get a buffer for the dictionary request reject status */
	msgBuf = rsslGetBuffer(chnl, MAX_DICTIONARY_STATUS_MSG_SIZE, RSSL_FALSE, &error);

	if (msgBuf != NULL)
	{
		/* encode dictionary request reject status */
		if (encodeDictionaryRequestReject(chnl, streamId, reason, msgBuf) != RSSL_RET_SUCCESS)
		{
			rsslReleaseBuffer(msgBuf, &error);
			printf("\nencodeDictionaryRequestReject() failed\n");
			return RSSL_RET_FAILURE;
		}

		/* send request reject status */
		if (sendMessage(chnl, msgBuf) != RSSL_RET_SUCCESS)
			return RSSL_RET_FAILURE;
	}
	else
	{
		printf("rsslGetBuffer(): Failed <%s>\n", error.text);
		return RSSL_RET_FAILURE;
	}

	return RSSL_RET_SUCCESS;
}

/*
 * Encodes the dictionary request reject status.  Returns success if
 * encoding succeeds or failure if encoding fails.
 * chnl - The channel to send request reject status message to
 * streamId - The stream id of the request
 * reason - The reason for the reject
 * msgBuf - The message buffer to encode the dictionary request reject into
 */
static RsslRet encodeDictionaryRequestReject(RsslChannel* chnl, RsslInt32 streamId, RsslDictionaryRejectReason reason, RsslBuffer* msgBuf)
{
	RsslRet ret = 0;
	RsslStatusMsg msg = RSSL_INIT_STATUS_MSG;
	char stateText[MAX_DICTIONARY_REQ_INFO_STRLEN];
	RsslEncodeIterator encodeIter;

	/* clear encode iterator */
	rsslClearEncodeIterator(&encodeIter);

	/* set-up message */
	msg.msgBase.msgClass = RSSL_MC_STATUS;
	msg.msgBase.streamId = streamId;
	msg.msgBase.domainType = RSSL_DMT_DICTIONARY;
	msg.msgBase.containerType = RSSL_DT_NO_DATA;
	msg.flags = RSSL_STMF_HAS_STATE;
	msg.state.dataState = RSSL_DATA_SUSPECT;
	switch(reason)
	{
	case UNKNOWN_DICTIONARY_NAME:
		msg.state.code = RSSL_SC_NOT_FOUND;
		msg.state.streamState = RSSL_STREAM_CLOSED;
		sprintf(stateText, "Dictionary request rejected for stream id %d - dictionary name unknown", streamId);
		msg.state.text.data = stateText;
		msg.state.text.length = (RsslUInt32)strlen(stateText) + 1;
		break;
	case MAX_DICTIONARY_REQUESTS_REACHED:
		msg.state.code = RSSL_SC_TOO_MANY_ITEMS;
		msg.state.streamState = RSSL_STREAM_CLOSED_RECOVER;
		sprintf(stateText, "Dictionary request rejected for stream id %d - max request count reached", streamId);
		msg.state.text.data = stateText;
		msg.state.text.length = (RsslUInt32)strlen(stateText) + 1;
		break;
	default:
		break;
	}

	/* encode message */
	if((ret = rsslSetEncodeIteratorBuffer(&encodeIter, msgBuf)) < RSSL_RET_SUCCESS)
	{
		printf("rsslSetEncodeIteratorBuffer() failed with return code: %d\n", ret);
		return ret;
	}
	rsslSetEncodeIteratorRWFVersion(&encodeIter, chnl->majorVersion, chnl->minorVersion);
	if ((ret = rsslEncodeMsg(&encodeIter, (RsslMsg*)&msg)) < RSSL_RET_SUCCESS)
	{
		printf("rsslEncodeMsg() failed with return code: %d\n", ret);
		return ret;
	}

	msgBuf->length = rsslGetEncodedBufferLength(&encodeIter);

	return RSSL_RET_SUCCESS;
}

/*
 * Calls the function to delete the dictionary, freeing all memory associated with it.
 */
void freeDictionary()
{
	rsslDeleteDataDictionary(&dictionary);
}

/*
 * Processes a dictionary response.  This consists of calling
 * rsslDecodeFieldDictionary() to decode the dictionary and calling
 * sendItemRequest() to send the item request if the dictionary
 * has been fully loaded.
 * chnl - The channel of the response
 * msg - The partially decoded message
 * dIter - The decode iterator
 */
RsslRet processDictionaryResponse(RsslChannel *chnl, RsslMsg* msg, RsslDecodeIterator* dIter)
{
	RsslMsgKey* key = 0;
	RsslState *pState = 0;
	char	errTxt[256];
	RsslBuffer errorText = {255, (char*)errTxt};
	char tempData[1024];
	RsslBuffer tempBuffer;
	RDMDictionaryTypes dictionaryType = (RDMDictionaryTypes)0;

	tempBuffer.data = tempData;
	tempBuffer.length = 1024;

	switch(msg->msgBase.msgClass)
	{
	case RSSL_MC_REFRESH:
		/* decode dictionary response */

		/* get key */
		key = (RsslMsgKey *)rsslGetMsgKey(msg);

		if (key)
		{
			printf("\nReceived Dictionary Response: %.*s\n", key->name.length, key->name.data);
		}
		else 
		{
			printf("\nReceived Dictionary Response\n");
		}

		pState = &msg->refreshMsg.state;
		rsslStateToString(&tempBuffer, pState);
		printf("	%.*s\n\n", tempBuffer.length, tempBuffer.data);

		if ((msg->msgBase.streamId != fieldDictionaryStreamId) && (msg->msgBase.streamId != enumDictionaryStreamId))
		{
			/* The first part of a dictionary refresh should contain information about its type.
			 * Save this information and use it as subsequent parts arrive. */

			if (rsslExtractDictionaryType(dIter, &dictionaryType, &errorText) != RSSL_RET_SUCCESS)
    		{
    			printf("rsslGetDictionaryType() failed: %.*s\n", errorText.length, errorText.data);
    			return RSSL_RET_SUCCESS;
    		}

			switch (dictionaryType)
			{
			case RDM_DICTIONARY_FIELD_DEFINITIONS:
    			fieldDictionaryStreamId = msg->msgBase.streamId; 
				break;
			case RDM_DICTIONARY_ENUM_TABLES:
    			enumDictionaryStreamId = msg->msgBase.streamId;
				break;
			default:
				printf("Unknown dictionary type %llu from message on stream %d\n", (RsslUInt)dictionaryType, msg->msgBase.streamId);
				return RSSL_RET_SUCCESS;
			}
		}

		if (msg->msgBase.streamId == fieldDictionaryStreamId)
		{
    		if (rsslDecodeFieldDictionary(dIter, &dictionary, RDM_DICTIONARY_VERBOSE, &errorText) != RSSL_RET_SUCCESS)
    		{
    			printf("Decoding Dictionary failed: %.*s\n", errorText.length, errorText.data);
    			return RSSL_RET_SUCCESS;
    		}

			if (msg->refreshMsg.flags & RSSL_RFMF_REFRESH_COMPLETE)
			{
				fieldDictionaryLoaded = RSSL_TRUE;
				fieldDictionaryStreamId = 0;
				if (!isEnumTypeDictionaryLoaded())
					printf("Field Dictionary complete, waiting for Enum Table...\n");
				else
					printf("Field Dictionary complete.\n");
			}
		} 
		else if (msg->msgBase.streamId == enumDictionaryStreamId)
		{
    		if (rsslDecodeEnumTypeDictionary(dIter, &dictionary, RDM_DICTIONARY_VERBOSE, &errorText) != RSSL_RET_SUCCESS)
    		{
    			printf("Decoding Dictionary failed: %.*s\n", errorText.length, errorText.data);
    			return RSSL_RET_SUCCESS;
    		}

			if (msg->refreshMsg.flags & RSSL_RFMF_REFRESH_COMPLETE)
			{
				enumTypeDictionaryLoaded = RSSL_TRUE;
				enumDictionaryStreamId = 0;
				if (!isFieldDictionaryLoaded())
					printf("Enumerated Types Dictionary complete, waiting for Field Dictionary...\n");
				else
					printf("Enumerated Types Dictionary complete.\n");
			}
		}
		else
		{
			printf("Received unexpected dictionary message on stream %d\n", msg->msgBase.streamId);
			return RSSL_RET_SUCCESS;
		}

		

    	break;

	case RSSL_MC_STATUS:
		printf("\nReceived StatusMsg for dictionary\n");
		if (msg->statusMsg.flags & RSSL_STMF_HAS_STATE)
    	{
    		RsslState *pState = &msg->statusMsg.state;
			rsslStateToString(&tempBuffer, pState);
			printf("	%.*s\n\n", tempBuffer.length, tempBuffer.data);
    	}
		break;

	default:
		printf("\nReceived Unhandled Dictionary MsgClass: %d\n", msg->msgBase.msgClass);
    	break;
	}

	return RSSL_RET_SUCCESS;
}

RsslBool isDictionaryReady()
{
	if (enumTypeDictionaryLoaded && fieldDictionaryLoaded)
		return RSSL_TRUE;
	else
		return RSSL_FALSE;

}
