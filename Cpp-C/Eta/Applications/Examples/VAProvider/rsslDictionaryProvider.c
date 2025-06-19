/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2015,2019-2020,2023-2024 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

/*
 * This is the dictionary provider for both the rsslVAProvider and
 * rsslVANIProvider applications.  Only two dictionary streams (field
 * and enum type) per channel are allowed by this simple provider.
 * It provides functions for processing dictionary requests from
 * consumers and sending back the responses.  Functions for sending
 * dictionary request reject/close status messages, initializing the
 * dictionary provider, loading dictionaries from files, getting the
 * loaded dictionary, and closing dictionary streams are also provided.
 */

#include "rsslProvider.h"
#include "rsslDictionaryProvider.h"
#include "rsslDirectoryHandler.h"
#include "rsslVASendMessage.h"

#if defined(_WIN32)
#include <windows.h>
#else
#include <time.h>
#include <sys/time.h>
#endif

/* data dictionary */
static RsslDataDictionary dictionary;
/* field dictionary file name  */
static const char *fieldDictionaryFileName = "RDMFieldDictionary";
/* field dictionary name */
static RsslBuffer fieldDictionaryDownloadName = { 6 , (char *)"RWFFld" };
/* enum type dictionary file name  */
static const char *enumTypeDictionaryFileName = "enumtype.def";
/* enumtype dictionary name */
static RsslBuffer enumTypeDictionaryDownloadName = { 7, (char *)"RWFEnum" };

/* dictionary request information */
static DictionaryRequestInfo dictionaryRequestInfoList[MAX_DICTIONARY_REQUESTS];

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

	/* Add enumerated types information to dictionary */
	if (rsslLoadEnumTypeDictionary(enumTypeDictionaryFileName, &dictionary, &errorText) < 0)
	{
		printf("\nUnable to load enum type dictionary: %s.\n\tError Text: %s\n", enumTypeDictionaryFileName, errorText.data);
		retVal = RSSL_RET_FAILURE;
	}

	return retVal;
}

/*
 * Processes information contained in Dictionary requests, and provides responses.
 */
RsslReactorCallbackRet dictionaryMsgCallback(RsslReactor* pReactor, RsslReactorChannel* pReactorChannel, RsslRDMDictionaryMsgEvent* pDictionaryMsgEvent)
{
	RsslRet ret;
	RsslMsgKey* key = 0;
	RsslState *pState = 0;
	char	errTxt[256];
	RsslBuffer errorText = {255, (char*)errTxt};
	DictionaryRequestInfo *dictionaryRequestInfo;
	RsslRDMDictionaryMsg *pDictionaryMsg = pDictionaryMsgEvent->pRDMDictionaryMsg;

	if (!pDictionaryMsg)
	{
		RsslMsg *pRsslMsg = pDictionaryMsgEvent->baseMsgEvent.pRsslMsg;
		RsslErrorInfo *pError = pDictionaryMsgEvent->baseMsgEvent.pErrorInfo;

		printf("dictionaryMsgCallback() received error: %s(%s)\n", pError->rsslError.text, pError->errorLocation);
		if (pRsslMsg)
		{
			if (sendDictionaryRequestReject(pReactor, pReactorChannel, pRsslMsg->msgBase.streamId, DICTIONARY_RDM_DECODER_FAILED, pError) != RSSL_RET_SUCCESS)
				removeClientSessionForChannel(pReactor, pReactorChannel);

			return RSSL_RC_CRET_SUCCESS;
		}
		else
		{
			removeClientSessionForChannel(pReactor, pReactorChannel);
			return RSSL_RC_CRET_SUCCESS;
		}
	}

	switch(pDictionaryMsg->rdmMsgBase.rdmMsgType)
	{
	case RDM_DC_MT_REQUEST:
	{
		RsslRDMDictionaryRequest *pRequest = &pDictionaryMsg->request;

		/* get key */
		dictionaryRequestInfo = getDictionaryReqInfo(pReactorChannel, pRequest);

		if (!dictionaryRequestInfo)
		{
			if (sendDictionaryRequestReject(pReactor, pReactorChannel, pRequest->rdmMsgBase.streamId, MAX_DICTIONARY_REQUESTS_REACHED, NULL) != RSSL_RET_SUCCESS)
			{
				removeClientSessionForChannel(pReactor, pReactorChannel);
				return RSSL_RC_CRET_SUCCESS;
			}
			break;
		}

		printf("\nReceived Dictionary Request for DictionaryName: %.*s\n", dictionaryRequestInfo->dictionaryRequest.dictionaryName.length, dictionaryRequestInfo->dictionaryRequest.dictionaryName.data);

		if (rsslBufferIsEqual(&fieldDictionaryDownloadName, &dictionaryRequestInfo->dictionaryRequest.dictionaryName))
		{
			/* Name matches field dictionary. Send the field dictionary refresh. */
			if ((ret = sendFieldDictionaryRefresh(pReactor, pReactorChannel, dictionaryRequestInfo)) != RSSL_RET_SUCCESS)
			{
				printf("sendFieldDictionaryRefresh() failed: %d\n", ret);
				removeClientSessionForChannel(pReactor, pReactorChannel);
				return RSSL_RC_CRET_SUCCESS;
			}
		}
		else if (rsslBufferIsEqual(&enumTypeDictionaryDownloadName, &dictionaryRequestInfo->dictionaryRequest.dictionaryName))
		{
			/* Name matches the enum types dictionary. Send the enum types dictionary refresh. */
			if ((ret = sendEnumTypeDictionaryRefresh(pReactor, pReactorChannel, dictionaryRequestInfo)) != RSSL_RET_SUCCESS)
			{
				printf("sendEnumTypeDictionaryRefresh() failed: %d\n", ret);
				removeClientSessionForChannel(pReactor, pReactorChannel);
				return RSSL_RC_CRET_SUCCESS;
			}
		}
		else
		{
			if (sendDictionaryRequestReject(pReactor, pReactorChannel, dictionaryRequestInfo->dictionaryRequest.rdmMsgBase.streamId, UNKNOWN_DICTIONARY_NAME, NULL) != RSSL_RET_SUCCESS)
			{
				removeClientSessionForChannel(pReactor, pReactorChannel);
				return RSSL_RC_CRET_SUCCESS;
			}
			break;
		}
		break;

	}
	case RDM_DC_MT_CLOSE:
		printf("\nReceived Dictionary Close for StreamId %d\n", pDictionaryMsg->rdmMsgBase.streamId);

		/* close dictionary stream */
		closeDictionaryStream(pDictionaryMsg->rdmMsgBase.streamId);
		break;

	default:
		printf("\nReceived Unhandled Dictionary Msg Type: %d\n", pDictionaryMsg->rdmMsgBase.rdmMsgType);
    	break;
	}

	return RSSL_RC_CRET_SUCCESS;
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
 * pReactorChannel - The channel to get the dictionary request information structure for
 * msg - The partially decoded message
 * key - The message key
 */
static DictionaryRequestInfo* getDictionaryReqInfo(RsslReactorChannel* pReactorChannel, RsslRDMDictionaryRequest *pDictionaryRequest)
{
	int i;
	DictionaryRequestInfo* dictionaryRequestInfo = NULL;

	/* first check if one already in use for this channel and stream id */
	for (i = 0; i < MAX_DICTIONARY_REQUESTS; i++)
	{
		if (dictionaryRequestInfoList[i].IsInUse &&
			dictionaryRequestInfoList[i].Chnl == pReactorChannel &&
			dictionaryRequestInfoList[i].dictionaryRequest.rdmMsgBase.streamId == pDictionaryRequest->rdmMsgBase.streamId)
		{
			dictionaryRequestInfo = &dictionaryRequestInfoList[i];
			/* if dictionary name is different from last request, this is an invalid request */
			if (!rsslBufferIsEqual(&pDictionaryRequest->dictionaryName, &dictionaryRequestInfoList[i].dictionaryRequest.dictionaryName))
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
				dictionaryRequestInfoList[i].Chnl = pReactorChannel;
				dictionaryRequestInfoList[i].IsInUse = RSSL_TRUE;
				dictionaryRequestInfo = &dictionaryRequestInfoList[i];
				if (rsslCopyRDMDictionaryMsg((RsslRDMDictionaryMsg*)&dictionaryRequestInfo->dictionaryRequest, (RsslRDMDictionaryMsg*)pDictionaryRequest, &dictionaryRequestInfo->memoryBuffer) != RSSL_RET_SUCCESS)
					return NULL;
				break;
			}
		}
	}

	return dictionaryRequestInfo;
}

/*
 * Sends a field dictionary response to a channel.  This consists of getting
 * a message buffer, encoding the dictionary response, and sending the
 * dictionary response to the server.  Returns success if send dictionary
 * response succeeds or failure if send response fails.
 * pReactorChannel - The channel to send a dictionary response to
 * dictionaryReqInfo - The dictionary request information
 */
static RsslRet sendFieldDictionaryRefresh(RsslReactor *pReactor, RsslReactorChannel* pReactorChannel, DictionaryRequestInfo* dictionaryReqInfo)
{
	RsslRet ret;
	RsslErrorInfo rsslErrorInfo;
	RsslBuffer* msgBuf = 0;
	RsslDataDictionary* dictionary = getDictionary();
	RsslRDMDictionaryRefresh dictionaryRefresh;
	RsslBool firstPartMultiPartRefresh = RSSL_TRUE;

	rsslClearRDMDictionaryRefresh(&dictionaryRefresh);
	dictionaryRefresh.rdmMsgBase.streamId = dictionaryReqInfo->dictionaryRequest.rdmMsgBase.streamId;
	dictionaryRefresh.flags = RDM_DC_RFF_SOLICITED;
	dictionaryRefresh.type = RDM_DICTIONARY_FIELD_DEFINITIONS;
	dictionaryRefresh.verbosity = dictionaryReqInfo->dictionaryRequest.verbosity;
	dictionaryRefresh.dictionaryName = fieldDictionaryDownloadName;
	dictionaryRefresh.pDictionary = dictionary;
	dictionaryRefresh.serviceId = (RsslUInt16)getServiceId();
	dictionaryRefresh.state.streamState = RSSL_STREAM_OPEN;
	dictionaryRefresh.state.dataState = RSSL_DATA_OK;
	dictionaryRefresh.state.code = RSSL_SC_NONE;


	while (RSSL_TRUE)
	{
		if (firstPartMultiPartRefresh)
		{
			dictionaryRefresh.flags |= RDM_DC_RFF_CLEAR_CACHE;
			firstPartMultiPartRefresh = RSSL_FALSE;
		}
		else
			dictionaryRefresh.flags = RDM_DC_RFF_SOLICITED;

		/* get a buffer for the dictionary response */
		msgBuf = rsslReactorGetBuffer(pReactorChannel, MAX_FIELD_DICTIONARY_MSG_SIZE, RSSL_FALSE, &rsslErrorInfo);

		if (msgBuf != NULL)
		{
			RsslEncodeIterator eIter;
			char stateText[MAX_DICTIONARY_REQ_INFO_STRLEN];
			RsslErrorInfo rsslErrorInfo;

			rsslClearEncodeIterator(&eIter);
			rsslSetEncodeIteratorRWFVersion(&eIter, pReactorChannel->majorVersion, pReactorChannel->minorVersion);
			if((ret = rsslSetEncodeIteratorBuffer(&eIter, msgBuf)) < RSSL_RET_SUCCESS)
			{
				rsslReactorReleaseBuffer(pReactorChannel, msgBuf, &rsslErrorInfo);
				printf("\nrsslSetEncodeIteratorBuffer() failed with return code: %d\n", ret);
				return RSSL_RET_FAILURE;
			}

			printf("Field Dictionary Refresh (starting fid %d)\n", dictionaryRefresh.startFid);
			sprintf(stateText, "Field Dictionary Refresh (starting fid %d)", dictionaryRefresh.startFid);
			dictionaryRefresh.state.text.data = stateText;
			dictionaryRefresh.state.text.length = (RsslUInt32)strlen(stateText) + 1;

			ret = rsslEncodeRDMDictionaryMsg(&eIter, (RsslRDMDictionaryMsg*)&dictionaryRefresh, &msgBuf->length, &rsslErrorInfo);

			if (ret < RSSL_RET_SUCCESS)
			{
				rsslReactorReleaseBuffer(pReactorChannel, msgBuf, &rsslErrorInfo);
				printf("\nrsslEncodeRDMDictionaryMsg() failed: %s(%s)\n", rsslErrorInfo.rsslError.text, rsslErrorInfo.errorLocation);
				return RSSL_RET_FAILURE;
			}

			/* send dictionary response */
			if (sendMessage(pReactor, pReactorChannel, msgBuf) != RSSL_RET_SUCCESS)
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
			printf("rsslReactorGetBuffer(): Failed <%s>\n", rsslErrorInfo.rsslError.text);
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
 * pReactorChannel - The channel to send a dictionary response to
 * dictionaryReqInfo - The dictionary request information
 */
static RsslRet sendEnumTypeDictionaryRefresh(RsslReactor *pReactor, RsslReactorChannel* pReactorChannel, DictionaryRequestInfo* dictionaryReqInfo)
{
	RsslRet ret;
	RsslErrorInfo rsslErrorInfo;
	RsslBuffer* msgBuf = 0;
	RsslDataDictionary* dictionary = getDictionary();
	RsslRDMDictionaryRefresh dictionaryRefresh;
	char stateText[MAX_DICTIONARY_REQ_INFO_STRLEN];
	RsslBool firstPartMultiPartRefresh = RSSL_TRUE;


	rsslClearRDMDictionaryRefresh(&dictionaryRefresh);
	dictionaryRefresh.rdmMsgBase.streamId = dictionaryReqInfo->dictionaryRequest.rdmMsgBase.streamId;
	dictionaryRefresh.rdmMsgBase.rdmMsgType = RDM_DC_MT_REFRESH;
	dictionaryRefresh.flags = RDM_DC_RFF_SOLICITED;
	dictionaryRefresh.type = RDM_DICTIONARY_ENUM_TABLES;
	dictionaryRefresh.verbosity = dictionaryReqInfo->dictionaryRequest.verbosity;
	dictionaryRefresh.dictionaryName = enumTypeDictionaryDownloadName;
	dictionaryRefresh.pDictionary = dictionary;
	dictionaryRefresh.serviceId = (RsslUInt16)getServiceId();
	dictionaryRefresh.state.streamState = RSSL_STREAM_OPEN;
	dictionaryRefresh.state.dataState = RSSL_DATA_OK;
	dictionaryRefresh.state.code = RSSL_SC_NONE;


	while (RSSL_TRUE)
	{
		if (firstPartMultiPartRefresh)
		{
			dictionaryRefresh.flags |= RDM_DC_RFF_CLEAR_CACHE;
			firstPartMultiPartRefresh = RSSL_FALSE;
		}
		else
			dictionaryRefresh.flags = RDM_DC_RFF_SOLICITED;

		/* get a buffer for the dictionary response */
		msgBuf = rsslReactorGetBuffer(pReactorChannel, MAX_ENUM_TYPE_DICTIONARY_MSG_SIZE, RSSL_FALSE, &rsslErrorInfo);

		if (msgBuf != NULL)
		{
			RsslEncodeIterator eIter;
			RsslErrorInfo rsslErrorInfo;

			rsslClearEncodeIterator(&eIter);
			rsslSetEncodeIteratorRWFVersion(&eIter, pReactorChannel->majorVersion, pReactorChannel->minorVersion);
			if((ret = rsslSetEncodeIteratorBuffer(&eIter, msgBuf)) < RSSL_RET_SUCCESS)
			{
				rsslReactorReleaseBuffer(pReactorChannel, msgBuf, &rsslErrorInfo);
				printf("\nrsslSetEncodeIteratorBuffer() failed with return code: %d\n", ret);
				return RSSL_RET_FAILURE;
			}

			printf("Enum Dictionary Refresh (starting enum fid %d)\n", dictionaryRefresh.enumStartFid);
			sprintf(stateText, "Enum Dictionary Refresh (starting enum fid %d)", dictionaryRefresh.enumStartFid);
			dictionaryRefresh.state.text.data = stateText;
			dictionaryRefresh.state.text.length = (RsslUInt32)strlen(stateText) + 1;

			ret = rsslEncodeRDMDictionaryMsg(&eIter, (RsslRDMDictionaryMsg*)&dictionaryRefresh, &msgBuf->length, &rsslErrorInfo);

			if (ret < RSSL_RET_SUCCESS)
			{
				rsslReactorReleaseBuffer(pReactorChannel, msgBuf, &rsslErrorInfo);
				printf("\nrsslEncodeRDMDictionaryMsg() failed with return code: %d\n", ret);
				return RSSL_RET_FAILURE;
			}

			/* send dictionary response */
			if (sendMessage(pReactor, pReactorChannel, msgBuf) != RSSL_RET_SUCCESS)
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
			printf("rsslReactorGetBuffer(): Failed <%s>\n", rsslErrorInfo.rsslError.text);
			return RSSL_RET_FAILURE;
		}
	}

	return RSSL_RET_SUCCESS;
}

/* 
 * Closes all open dictionary streams for a channel. 
 * pReactorChannel - The channel to close the dictionary streams for
 */
void closeDictionaryChnlStreams(RsslReactorChannel *pReactorChannel)
{
	int i;
	DictionaryRequestInfo* dictionaryReqInfo = NULL;

	/* find original request information associated with pReactorChannel */
	for (i = 0; i < MAX_DICTIONARY_REQUESTS; i++)
	{
		if(dictionaryRequestInfoList[i].Chnl == pReactorChannel)
		{
			dictionaryReqInfo = &dictionaryRequestInfoList[i];
			/* clear original request information */
			printf("Closing dictionary stream id %d with dictionary name: %.*s\n", dictionaryReqInfo->dictionaryRequest.rdmMsgBase.streamId, dictionaryReqInfo->dictionaryRequest.dictionaryName.length, dictionaryReqInfo->dictionaryRequest.dictionaryName.data);
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
	DictionaryRequestInfo* dictionaryReqInfo = NULL;

	/* find original request information associated with streamId */
	for (i = 0; i < MAX_DICTIONARY_REQUESTS; i++)
	{
		if(dictionaryRequestInfoList[i].dictionaryRequest.rdmMsgBase.streamId == streamId)
		{
			dictionaryReqInfo = &dictionaryRequestInfoList[i];
			/* clear original request information */
			printf("Closing dictionary stream id %d with dictionary name: %.*s\n", dictionaryReqInfo->dictionaryRequest.rdmMsgBase.streamId, dictionaryReqInfo->dictionaryRequest.dictionaryName.length, dictionaryReqInfo->dictionaryRequest.dictionaryName.data);
			clearDictionaryReqInfo(dictionaryReqInfo);
			break;
		}
	}
}

/*
 * Sends the dictionary close status message(s) for a channel.
 * This consists of finding all request information for this channel
 * and sending the close status messages to the channel.
 * pReactorChannel - The channel to send close status message(s) to
 */
void sendDictionaryCloseStatusMsgs(RsslReactor *pReactor, RsslReactorChannel* pReactorChannel)
{
	int i;

	for (i = 0; i < MAX_DICTIONARY_REQUESTS; i++)
	{
		if (dictionaryRequestInfoList[i].IsInUse && dictionaryRequestInfoList[i].Chnl == pReactorChannel)
		{
			sendDictionaryCloseStatusMsg(pReactor, pReactorChannel, dictionaryRequestInfoList[i].dictionaryRequest.rdmMsgBase.streamId);
		}
	}
}

/*
 * Sends the dictionary close status message for a channel.
 * Returns success if send dictionary close status succeeds
 * or failure if it fails.
 * pReactorChannel - The channel to send close status message to
 * streamId - The stream id of the close status
 */
RsslRet sendDictionaryCloseStatusMsg(RsslReactor *pReactor, RsslReactorChannel* pReactorChannel, RsslInt32 streamId)
{
	RsslErrorInfo rsslErrorInfo;
	RsslBuffer* msgBuf = 0;

	/* get a buffer for the dictionary close status */
	msgBuf = rsslReactorGetBuffer(pReactorChannel, MAX_DICTIONARY_STATUS_MSG_SIZE, RSSL_FALSE, &rsslErrorInfo);

	if (msgBuf != NULL)
	{
		RsslEncodeIterator eIter;
		RsslRDMDictionaryStatus dictionaryStatus;
		RsslErrorInfo rsslErrorInfo;
		RsslRet ret;

		rsslClearRDMDictionaryStatus(&dictionaryStatus);
		dictionaryStatus.flags |= RDM_DC_STF_HAS_STATE;
		dictionaryStatus.state.streamState = RSSL_STREAM_CLOSED;
		dictionaryStatus.state.dataState = RSSL_DATA_SUSPECT;
		dictionaryStatus.state.text.data = (char *)"Dictionary Stream closed";
		dictionaryStatus.state.text.length = (RsslUInt32)strlen(dictionaryStatus.state.text.data);

		rsslClearEncodeIterator(&eIter);
		if((ret = rsslSetEncodeIteratorBuffer(&eIter, msgBuf)) < RSSL_RET_SUCCESS)
		{
			rsslReactorReleaseBuffer(pReactorChannel, msgBuf, &rsslErrorInfo);
			printf("\nrsslSetEncodeIteratorBuffer() failed with return code: %d\n", ret);
			return RSSL_RET_FAILURE;
		}
		rsslSetEncodeIteratorRWFVersion(&eIter, pReactorChannel->majorVersion, pReactorChannel->minorVersion);

		if ((ret = rsslEncodeRDMDictionaryMsg(&eIter, (RsslRDMDictionaryMsg*)&dictionaryStatus, &msgBuf->length, &rsslErrorInfo)) != RSSL_RET_SUCCESS)
		{
			rsslReactorReleaseBuffer(pReactorChannel, msgBuf, &rsslErrorInfo);
			printf("\nrsslEncodeRDMDictionaryMsg failed: %s(%s)\n", rsslErrorInfo.rsslError.text, rsslErrorInfo.errorLocation);
			return RSSL_RET_FAILURE;
		}

		/* send close status */
		if (sendMessage(pReactor, pReactorChannel, msgBuf) != RSSL_RET_SUCCESS)
			return RSSL_RET_FAILURE;
	}
	else
	{
		printf("rsslReactorGetBuffer(): Failed <%s>\n", rsslErrorInfo.rsslError.text);
		return RSSL_RET_FAILURE;
	}

	return RSSL_RET_SUCCESS;
}

/*
 * Sends the dictionary request reject status message for a channel.
 * pReactorChannel - The channel to send request reject status message to
 * streamId - The stream id of the request
 * reason - The reason for the reject
 */
static RsslRet sendDictionaryRequestReject(RsslReactor *pReactor, RsslReactorChannel* pReactorChannel, RsslInt32 streamId, RsslDictionaryRejectReason reason, RsslErrorInfo *pError)
{
	RsslErrorInfo rsslErrorInfo;
	RsslBuffer* msgBuf = 0;

	/* get a buffer for the dictionary request reject status */
	msgBuf = rsslReactorGetBuffer(pReactorChannel, MAX_DICTIONARY_STATUS_MSG_SIZE, RSSL_FALSE, &rsslErrorInfo);

	if (msgBuf != NULL)
	{
		RsslEncodeIterator eIter;
		char stateText[MAX_DICTIONARY_REQ_INFO_STRLEN];
		RsslRDMDictionaryStatus dictionaryStatus;
		RsslErrorInfo rsslErrorInfo;
		RsslRet ret;

		rsslClearRDMDictionaryStatus(&dictionaryStatus);
		dictionaryStatus.flags |= RDM_DC_STF_HAS_STATE;

		switch(reason)
		{
			case UNKNOWN_DICTIONARY_NAME:
				dictionaryStatus.state.code = RSSL_SC_NOT_FOUND;
				dictionaryStatus.state.streamState = RSSL_STREAM_CLOSED;
				snprintf(stateText, sizeof(stateText), "Dictionary request rejected for stream id %d - dictionary name unknown", streamId);
				break;
			case MAX_DICTIONARY_REQUESTS_REACHED:
				dictionaryStatus.state.code = RSSL_SC_TOO_MANY_ITEMS;
				dictionaryStatus.state.streamState = RSSL_STREAM_CLOSED_RECOVER;
				snprintf(stateText, sizeof(stateText), "Dictionary request rejected for stream id %d - max request count reached", streamId);
				break;
			case DICTIONARY_RDM_DECODER_FAILED:
				dictionaryStatus.state.code = RSSL_SC_USAGE_ERROR;
#if defined(__GNUC__) && (__GNUC__ >= 9)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wformat-truncation"
#endif
				snprintf(stateText, sizeof(stateText), "Dictionary request rejected for stream id %d - decoding failure: %s", streamId, pError->rsslError.text);
#if defined(__GNUC__) && (__GNUC__ >= 9)
#pragma GCC diagnostic pop
#endif
				dictionaryStatus.state.text.data = stateText;
				dictionaryStatus.state.text.length = (RsslUInt32)strlen(stateText) + 1;
				break;
			default:
				break;
		}

		dictionaryStatus.state.dataState = RSSL_DATA_SUSPECT;
		dictionaryStatus.state.text.data = (char *)"Dictionary Stream closed";
		dictionaryStatus.state.text.length = (RsslUInt32)strlen(dictionaryStatus.state.text.data);

		rsslClearEncodeIterator(&eIter);
		if((ret = rsslSetEncodeIteratorBuffer(&eIter, msgBuf)) < RSSL_RET_SUCCESS)
		{
			rsslReactorReleaseBuffer(pReactorChannel, msgBuf, &rsslErrorInfo);
			printf("\nrsslSetEncodeIteratorBuffer() failed with return code: %d\n", ret);
			return RSSL_RET_FAILURE;
		}
		rsslSetEncodeIteratorRWFVersion(&eIter, pReactorChannel->majorVersion, pReactorChannel->minorVersion);

		if ((ret = rsslEncodeRDMDictionaryMsg(&eIter, (RsslRDMDictionaryMsg*)&dictionaryStatus, &msgBuf->length, &rsslErrorInfo)) != RSSL_RET_SUCCESS)
		{
			rsslReactorReleaseBuffer(pReactorChannel, msgBuf, &rsslErrorInfo);
			printf("\nrsslEncodeRDMDictionaryMsg failed: %s(%s)\n", rsslErrorInfo.rsslError.text, rsslErrorInfo.errorLocation);
			return RSSL_RET_FAILURE;
		}

		/* send request reject status */
		if (sendMessage(pReactor, pReactorChannel, msgBuf) != RSSL_RET_SUCCESS)
			return RSSL_RET_FAILURE;
	}
	else
	{
		printf("rsslReactorGetBuffer(): Failed <%s>\n", rsslErrorInfo.rsslError.text);
		return RSSL_RET_FAILURE;
	}

	return RSSL_RET_SUCCESS;
}

/*
 * Calls the function to delete the dictionary, freeing all memory associated with it.
 */
void freeDictionary()
{
	rsslDeleteDataDictionary(&dictionary);
}
