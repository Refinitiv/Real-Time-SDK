/*
 * This source code is provided under the Apache 2.0 license and is provided
 * AS IS with no warranty or guarantee of fit for purpose.  See the project's 
 * LICENSE.md for details. 
 * Copyright (C) 2019 Refinitiv. All rights reserved.
*/

#include "dictionaryProvider.h"
#include <assert.h>
#include <stdlib.h>
#if defined(_WIN32)
#include <windows.h>
#else
#include <sys/time.h>
#endif

#include "xmlMsgDataParser.h"

static RsslBuffer fieldDictionaryDownloadName = { 6, (char*)"RWFFld" };		/* Name of the field dictionary. */
static RsslBuffer enumTypeDictionaryDownloadName = { 7, (char*)"RWFEnum" };	/* Name of the enumerated types dictionary. */

void initDictionaryProvider()
{
	/* Nothing to initialize, but the dictionary provider is dependant on the XML Msg Data 
	 * having loaded the dictionary it uses. */
	assert(xmlMsgDataLoaded);
}

RsslRet processDictionaryRequest(ChannelHandler *pChannelHandler, ChannelInfo *pChannelInfo, 
		RsslMsg* msg, RsslDecodeIterator* dIter)
{
	RsslRet ret;
	RsslRDMDictionaryMsg dictionaryMsg;
	char dictionaryMsgChar[4000];
	RsslBuffer memoryBuffer = { 4000, dictionaryMsgChar };
	RsslErrorInfo errorInfo;
	char errTxt[255];
	RsslBuffer errorText = {255, (char*)errTxt};
	RsslChannel *pChannel = pChannelInfo->pChannel;

	if ((ret = rsslDecodeRDMDictionaryMsg(dIter, msg, &dictionaryMsg, &memoryBuffer, &errorInfo)) != RSSL_RET_SUCCESS)
	{
		printf("rsslDecodeRDMDictionaryMsg() failed: %d(%s)\n", ret, errorInfo.rsslError.text);
		return ret;
	}

	switch(dictionaryMsg.rdmMsgBase.rdmMsgType)
	{
	case RDM_DC_MT_REQUEST:

		printf("\nReceived Dictionary Request for DictionaryName: %.*s\n", dictionaryMsg.request.dictionaryName.length, dictionaryMsg.request.dictionaryName.data );

		if (rsslBufferIsEqual(&fieldDictionaryDownloadName, &dictionaryMsg.request.dictionaryName))
		{
			/* send dictionary response */
			return sendDictionaryResponse(pChannelHandler, pChannelInfo, &dictionaryMsg.request, RDM_DICTIONARY_FIELD_DEFINITIONS);
		}
		else if (rsslBufferIsEqual(&enumTypeDictionaryDownloadName, &dictionaryMsg.request.dictionaryName))
		{
			/* send dictionary response */
			return sendDictionaryResponse(pChannelHandler, pChannelInfo, &dictionaryMsg.request, RDM_DICTIONARY_ENUM_TABLES);
		}
		else
		{
			sendDictionaryRequestReject(pChannelHandler, pChannelInfo, dictionaryMsg.rdmMsgBase.streamId, DICTIONARY_REJECT_UNKNOWN_NAME);
			break;
		}
		break;

	case RDM_DC_MT_CLOSE:
		printf("\nReceived Dictionary Close for StreamId %d\n", dictionaryMsg.rdmMsgBase.streamId);
		break;

	default:
		printf("\nReceived Unhandled Dictionary MsgClass: %d\n", msg->msgBase.msgClass);
    	break;
	}

	return RSSL_RET_SUCCESS;
}

RsslRet processDictionaryRequestReactor(RsslReactor *pReactor, RsslReactorChannel *pReactorChannel, RsslRDMDictionaryMsg *pDictionaryMsg)
{
	switch(pDictionaryMsg->rdmMsgBase.rdmMsgType)
	{
	case RDM_DC_MT_REQUEST:

		printf("\nReceived Dictionary Request for DictionaryName: %.*s\n", pDictionaryMsg->request.dictionaryName.length, pDictionaryMsg->request.dictionaryName.data );

		if (rsslBufferIsEqual(&fieldDictionaryDownloadName, &pDictionaryMsg->request.dictionaryName))
		{
			/* send dictionary response */
			return sendDictionaryResponseReactor(pReactor, pReactorChannel, &pDictionaryMsg->request, RDM_DICTIONARY_FIELD_DEFINITIONS);
		}
		else if (rsslBufferIsEqual(&enumTypeDictionaryDownloadName, &pDictionaryMsg->request.dictionaryName))
		{
			/* send dictionary response */
			return sendDictionaryResponseReactor(pReactor, pReactorChannel, &pDictionaryMsg->request, RDM_DICTIONARY_ENUM_TABLES);
		}
		else
		{
			sendDictionaryRequestRejectReactor(pReactor, pReactorChannel, pDictionaryMsg->rdmMsgBase.streamId, DICTIONARY_REJECT_UNKNOWN_NAME);
			break;
		}
		break;

	case RDM_DC_MT_CLOSE:
		printf("\nReceived Dictionary Close for StreamId %d\n", pDictionaryMsg->rdmMsgBase.streamId);
		break;

	default:
		printf("\nReceived Unhandled Dictionary Msg Type: %d\n", pDictionaryMsg->rdmMsgBase.rdmMsgType);
    	break;
	}

	return RSSL_RET_SUCCESS;
}

static RsslRet sendDictionaryResponse(ChannelHandler *pChannelHandler, ChannelInfo *pChannelInfo, 
		RsslRDMDictionaryRequest *pDictionaryRequest, RDMDictionaryTypes type)
{
	RsslRet ret, sendRet;
	RsslError error;
	RsslBuffer* msgBuf = 0;
	RsslInt32 dictionaryFid = RSSL_MIN_FID;
	RsslUInt32 msgSize;
	RsslRDMDictionaryRefresh dictionaryRefresh;
	RsslChannel *pChannel = pChannelInfo->pChannel;
	RsslChannelInfo channelInfo;

	rsslClearRDMDictionaryRefresh(&dictionaryRefresh);

	dictionaryRefresh.rdmMsgBase.streamId = pDictionaryRequest->rdmMsgBase.streamId;
	dictionaryRefresh.flags = RDM_DC_RFF_SOLICITED;
	dictionaryRefresh.type = type;
	dictionaryRefresh.verbosity = pDictionaryRequest->verbosity;
	dictionaryRefresh.dictionaryName = pDictionaryRequest->dictionaryName;
	dictionaryRefresh.pDictionary = &dictionary;

	if ((ret = rsslGetChannelInfo(pChannelInfo->pChannel, &channelInfo, &error) != RSSL_RET_SUCCESS))
	{
		printf("sendDictionaryResponse(): rsslGetChannelInfo() failed: %d(%s)\n", ret, error.text);
		return ret;
	}

	switch(type)
	{
		case RDM_DICTIONARY_FIELD_DEFINITIONS:
			msgSize = channelInfo.maxFragmentSize;
			break;
		case RDM_DICTIONARY_ENUM_TABLES:
			msgSize = 128000;
			break;
		default:
			assert(0);
	}


	while (RSSL_TRUE)
	{
		/* get a buffer for the dictionary response */
		msgBuf = rsslGetBuffer(pChannel, msgSize, RSSL_FALSE, &error);

		if (msgBuf != NULL)
		{
			RsslEncodeIterator eIter;
			RsslErrorInfo rsslErrorInfo;

			rsslClearEncodeIterator(&eIter);
			rsslSetEncodeIteratorRWFVersion(&eIter, pChannel->majorVersion, pChannel->minorVersion);
			rsslSetEncodeIteratorBuffer(&eIter, msgBuf);

			ret = rsslEncodeRDMDictionaryMsg(&eIter, (RsslRDMDictionaryMsg*)&dictionaryRefresh, &msgBuf->length, &rsslErrorInfo);

			if (ret < RSSL_RET_SUCCESS)
			{
				printf("\nrsslEncodeRDMDictionaryMsg() failed: %s(%s)\n", rsslErrorInfo.rsslError.text, rsslErrorInfo.errorLocation);
				exit(-1);
			}

			/* send dictionary response */
			if ((sendRet = channelHandlerWriteChannel(pChannelHandler, pChannelInfo, msgBuf, 0)) < RSSL_RET_SUCCESS)
			{
				return sendRet;
			}

			/* break out of loop when all dictionary responses sent */
			if (ret == RSSL_RET_SUCCESS)
			{
				return sendRet;
			}
		}
		else
		{
			printf("rsslGetBuffer(): Failed <%s>\n", error.text);
			return RSSL_RET_FAILURE;
		}
	}
}

static RsslRet sendDictionaryResponseReactor(RsslReactor *pReactor, RsslReactorChannel *pReactorChannel, 
		RsslRDMDictionaryRequest *pDictionaryRequest, RDMDictionaryTypes type)
{
	RsslRet ret, sendRet;
	RsslError error;
	RsslBuffer* msgBuf = 0;
	RsslInt32 dictionaryFid = RSSL_MIN_FID;
	RsslUInt32 msgSize;
	RsslRDMDictionaryRefresh dictionaryRefresh;
	RsslChannelInfo channelInfo;
	RsslErrorInfo errorInfo;
	RsslReactorSubmitOptions submitOpts;

	rsslClearReactorSubmitOptions(&submitOpts);

	rsslClearRDMDictionaryRefresh(&dictionaryRefresh);

	dictionaryRefresh.rdmMsgBase.streamId = pDictionaryRequest->rdmMsgBase.streamId;
	dictionaryRefresh.flags = RDM_DC_RFF_SOLICITED;
	dictionaryRefresh.type = type;
	dictionaryRefresh.verbosity = pDictionaryRequest->verbosity;
	dictionaryRefresh.dictionaryName = pDictionaryRequest->dictionaryName;
	dictionaryRefresh.pDictionary = &dictionary;

	if ((ret = rsslGetChannelInfo(pReactorChannel->pRsslChannel, &channelInfo, &error) != RSSL_RET_SUCCESS))
	{
		printf("sendDictionaryResponse(): rsslGetChannelInfo() failed: %d(%s)\n", ret, error.text);
		return ret;
	}

	switch(type)
	{
		case RDM_DICTIONARY_FIELD_DEFINITIONS:
			msgSize = channelInfo.maxFragmentSize;
			break;
		case RDM_DICTIONARY_ENUM_TABLES:
			msgSize = 128000;
			break;
		default:
			assert(0);
	}

	while (RSSL_TRUE)
	{
		/* get a buffer for the dictionary response */
		msgBuf = rsslReactorGetBuffer(pReactorChannel, msgSize, RSSL_FALSE, &errorInfo);

		if (msgBuf != NULL)
		{
			RsslEncodeIterator eIter;
			RsslErrorInfo rsslErrorInfo;

			rsslClearEncodeIterator(&eIter);
			rsslSetEncodeIteratorRWFVersion(&eIter, pReactorChannel->majorVersion, pReactorChannel->minorVersion);
			rsslSetEncodeIteratorBuffer(&eIter, msgBuf);

			ret = rsslEncodeRDMDictionaryMsg(&eIter, (RsslRDMDictionaryMsg*)&dictionaryRefresh, &msgBuf->length, &rsslErrorInfo);

			if (ret < RSSL_RET_SUCCESS)
			{
				printf("\nrsslEncodeRDMDictionaryMsg() failed: %s(%s)\n", rsslErrorInfo.rsslError.text, rsslErrorInfo.errorLocation);
				exit(-1);
			}

			/* send dictionary response */
			if ((sendRet = rsslReactorSubmit(pReactor, pReactorChannel, msgBuf, &submitOpts, &errorInfo)) < RSSL_RET_SUCCESS)
			{
				return sendRet;
			}

			/* break out of loop when all dictionary responses sent */
			if (ret == RSSL_RET_SUCCESS)
			{
				return sendRet;
			}
		}
		else
		{
			printf("rsslGetBuffer(): Failed <%s>\n", error.text);
			return RSSL_RET_FAILURE;
		}
	}
}

static RsslRet sendDictionaryRequestReject(ChannelHandler *pChannelHandler, ChannelInfo *pChannelInfo, 
		RsslInt32 streamId, DictionaryRejectReason reason)
{
	RsslRet ret;
	RsslError error;
	RsslBuffer* msgBuf = 0;
	RsslChannel *pChannel = pChannelInfo->pChannel;

	/* get a buffer for the dictionary request reject status */
	msgBuf = rsslGetBuffer(pChannel, 512, RSSL_FALSE, &error);

	if (msgBuf != NULL)
	{
		/* encode dictionary request reject status */
		RsslEncodeIterator eIter;
		RsslRDMDictionaryStatus dictionaryStatus;
		RsslErrorInfo errorInfo;

		rsslClearRDMDictionaryStatus(&dictionaryStatus);

		dictionaryStatus.rdmMsgBase.streamId = streamId;

		dictionaryStatus.flags = RDM_DC_STF_HAS_STATE;
		dictionaryStatus.state.streamState = RSSL_STREAM_CLOSED;


		switch(reason)
		{
			case DICTIONARY_REJECT_UNKNOWN_NAME:
				dictionaryStatus.state.dataState = RSSL_DATA_SUSPECT;
				dictionaryStatus.state.code = RSSL_SC_INVALID_ARGUMENT;
				dictionaryStatus.state.text.data = (char*)"Unknown dictionary name.";
				dictionaryStatus.state.text.length = (RsslUInt32)strlen(dictionaryStatus.state.text.data);
				break;
			default:
				assert(0);
		}

		rsslClearEncodeIterator(&eIter);
		rsslSetEncodeIteratorRWFVersion(&eIter, pChannel->majorVersion, pChannel->minorVersion);
		rsslSetEncodeIteratorBuffer(&eIter, msgBuf);

		if ((ret = rsslEncodeRDMDictionaryMsg(&eIter, (RsslRDMDictionaryMsg*)&dictionaryStatus, &msgBuf->length, &errorInfo)) != RSSL_RET_SUCCESS)
		{
			printf("rsslEncodeRDMDictionaryMsg() failed: %d(%s)", ret, errorInfo.rsslError.text);
			return ret;
		}

		/* send request reject status */
		return channelHandlerWriteChannel(pChannelHandler, pChannelInfo, msgBuf, 0);
	}
	else
	{
		printf("rsslGetBuffer(): Failed <%s>\n", error.text);
	}

	return RSSL_RET_SUCCESS;
}

static RsslRet sendDictionaryRequestRejectReactor(RsslReactor *pReactor, RsslReactorChannel *pReactorChannel, 
		RsslInt32 streamId, DictionaryRejectReason reason)
{
	RsslRet ret;
	RsslErrorInfo errorInfo;
	RsslBuffer* msgBuf = 0;

	/* get a buffer for the dictionary request reject status */
	msgBuf = rsslReactorGetBuffer(pReactorChannel, 512, RSSL_FALSE, &errorInfo);

	if (msgBuf != NULL)
	{
		/* encode dictionary request reject status */
		RsslEncodeIterator eIter;
		RsslRDMDictionaryStatus dictionaryStatus;
		RsslErrorInfo errorInfo;
		RsslReactorSubmitOptions submitOpts;

		rsslClearRDMDictionaryStatus(&dictionaryStatus);

		dictionaryStatus.rdmMsgBase.streamId = streamId;

		dictionaryStatus.flags = RDM_DC_STF_HAS_STATE;
		dictionaryStatus.state.streamState = RSSL_STREAM_CLOSED;

		switch(reason)
		{
			case DICTIONARY_REJECT_UNKNOWN_NAME:
				dictionaryStatus.state.dataState = RSSL_DATA_SUSPECT;
				dictionaryStatus.state.code = RSSL_SC_INVALID_ARGUMENT;
				dictionaryStatus.state.text.data = (char*)"Unknown dictionary name.";
				dictionaryStatus.state.text.length = (RsslUInt32)strlen(dictionaryStatus.state.text.data);
				break;
			default:
				assert(0);
		}

		rsslClearEncodeIterator(&eIter);
		rsslSetEncodeIteratorRWFVersion(&eIter, pReactorChannel->majorVersion, pReactorChannel->minorVersion);
		rsslSetEncodeIteratorBuffer(&eIter, msgBuf);

		if ((ret = rsslEncodeRDMDictionaryMsg(&eIter, (RsslRDMDictionaryMsg*)&dictionaryStatus, &msgBuf->length, &errorInfo)) != RSSL_RET_SUCCESS)
		{
			printf("rsslEncodeRDMDictionaryMsg() failed: %d(%s)", ret, errorInfo.rsslError.text);
			return ret;
		}

		/* send request reject status */
		rsslClearReactorSubmitOptions(&submitOpts);
 		return rsslReactorSubmit(pReactor, pReactorChannel, msgBuf, &submitOpts, &errorInfo);
	}
	else
	{
		printf("rsslGetBuffer(): Failed <%s>\n", errorInfo.rsslError.text);
	}

	return RSSL_RET_SUCCESS;
}
