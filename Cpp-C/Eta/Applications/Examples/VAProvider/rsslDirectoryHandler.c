/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2015-2016,2019-2020,2023-2024 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

/*
 * This is the implementation of the callback for Directory requests
 * received by the rsslVAProvider application.
 */

#include "rtr/rsslRDMDirectoryMsg.h"
#include "rsslDirectoryHandler.h"
#include "rsslVASendMessage.h"

/* service name of provider */
static char* serviceName;

/* service id associated with the service name of provider */
static RsslUInt64 serviceId = 1234;

static DirectoryRequestInfo directoryReqList[NUM_CLIENT_SESSIONS];

/*
 * Initializes source directory information fields.
 */
void initDirectoryHandler()
{
	int i;

	for(i = 0; i < NUM_CLIENT_SESSIONS; ++i)
	{
		clearDirectoryRequestInfo(&directoryReqList[i]);
	}
}

/*
 * Processes information contained in Directory requests, and provides responses.
 */
RsslReactorCallbackRet directoryMsgCallback(RsslReactor *pReactor, RsslReactorChannel *pReactorChannel, RsslRDMDirectoryMsgEvent *pDirectoryMsgEvent)
{
	RsslRDMDirectoryRequest *pDirectoryRequest;
	RsslRDMDirectoryMsg *pDirectoryMsg = pDirectoryMsgEvent->pRDMDirectoryMsg;

	if (!pDirectoryMsg)
	{
		RsslMsg *pRsslMsg = pDirectoryMsgEvent->baseMsgEvent.pRsslMsg;
		RsslErrorInfo *pError = pDirectoryMsgEvent->baseMsgEvent.pErrorInfo;

		printf("directoryMsgCallback() received error: %s(%s)\n", pError->rsslError.text, pError->errorLocation);
		if (pRsslMsg)
		{
			if (sendDirectoryRequestReject(pReactor, pReactorChannel, pRsslMsg->msgBase.streamId, DIRECTORY_RDM_DECODER_FAILED, pError) != RSSL_RET_SUCCESS)
				removeClientSessionForChannel(pReactor, pReactorChannel);

			return RSSL_RC_CRET_SUCCESS;
		}
		else
		{
			removeClientSessionForChannel(pReactor, pReactorChannel);
			return RSSL_RC_CRET_SUCCESS;
		}
	}

	switch(pDirectoryMsg->rdmMsgBase.rdmMsgType)
	{
		case RDM_DR_MT_REQUEST:
		{
			RsslRDMDirectoryRequest *pRDMDirectoryRequest = &pDirectoryMsg->request;

			/* Reject any request that does not request at least the Info, State, and Group filters. */
			if ( !(pRDMDirectoryRequest->filter & RDM_DIRECTORY_SERVICE_INFO_FILTER) ||
					!(pRDMDirectoryRequest->filter & RDM_DIRECTORY_SERVICE_STATE_FILTER) ||
					!(pRDMDirectoryRequest->filter & RDM_DIRECTORY_SERVICE_GROUP_FILTER))
			{
				if (sendDirectoryRequestReject(pReactor, pReactorChannel, pRDMDirectoryRequest->rdmMsgBase.streamId, INCORRECT_FILTER_FLAGS, NULL) != RSSL_RET_SUCCESS)
					removeClientSessionForChannel(pReactor, pReactorChannel);

				return RSSL_RC_CRET_SUCCESS;
			}
			pDirectoryRequest = getDirectoryRequest(pReactorChannel, pRDMDirectoryRequest);

			if (!pDirectoryRequest)
			{
				if (sendDirectoryRequestReject(pReactor, pReactorChannel, pRDMDirectoryRequest->rdmMsgBase.streamId, MAX_SRCDIR_REQUESTS_REACHED, NULL) != RSSL_RET_SUCCESS)
					removeClientSessionForChannel(pReactor, pReactorChannel);

				return RSSL_RC_CRET_SUCCESS;
			}

			printf("\nReceived Source Directory Request\n");

			/* send source directory response */
			if (sendDirectoryRefresh(pReactor, pReactorChannel, pRDMDirectoryRequest) != RSSL_RET_SUCCESS)
				removeClientSessionForChannel(pReactor, pReactorChannel);

			return RSSL_RC_CRET_SUCCESS;
		}
		case RDM_DR_MT_CLOSE:
		{
			RsslRDMDirectoryClose *pClose = &pDirectoryMsg->close;
			printf("\nReceived Source Directory Close for StreamId %d\n", pClose->rdmMsgBase.streamId);

			/* close source directory stream */
			closeDirectoryStream(pClose->rdmMsgBase.streamId);
			return RSSL_RC_CRET_SUCCESS;
		}
		default:
			printf("\nReceived unhandled Source Directory msg type: %d\n", pDirectoryMsg->rdmMsgBase.rdmMsgType);
			return RSSL_RC_CRET_SUCCESS;
	}
}

/*
 * Sets the service name requested by the application.
 * servicename - The service name requested by the application
 */
void setServiceName(char* servicename)
{
	serviceName = servicename;
}

/*
 * Sets the service id requested by the application.
 * serviceid - The service id requested by the application
 */
void setServiceId(RsslUInt64 serviceid)
{
	serviceId = serviceid;
}

/*
 * Returns the service id associated with service name
 * requested by the application
 */
RsslUInt64 getServiceId()
{
	return serviceId;
}

/*
 * Retrieves DirectoryRequestInfo structure to use with a consumer that is requesting the source directory.
 */
static RsslRDMDirectoryRequest* getDirectoryRequest(RsslReactorChannel* pReactorChannel, RsslRDMDirectoryRequest *pRequest)
{
	int i;
	RsslRDMDirectoryRequest *pStoredRequest = NULL;

	/* first check if one already in use for this channel */
	for (i = 0; i < NUM_CLIENT_SESSIONS; i++)
	{
		if (directoryReqList[i].isInUse &&
			directoryReqList[i].chnl == pReactorChannel)
		{
			pStoredRequest = &directoryReqList[i].dirRequest;
			/* if stream id is different from last request, this is an invalid request */
			if (pStoredRequest->rdmMsgBase.streamId != pRequest->rdmMsgBase.streamId)
			{
				return NULL;
			}
			break;
		}
	}

	/* get a new one if one is not already in use */
	if (!pStoredRequest)
	{
		for (i = 0; i < NUM_CLIENT_SESSIONS; i++)
		{
			if(directoryReqList[i].isInUse == RSSL_FALSE)
			{
				RsslBuffer memoryBuffer;
				rsslClearBuffer(&memoryBuffer); /* Buffer can be blank since copying the Directory request message doesn't need any additional memory. */

				directoryReqList[i].chnl = pReactorChannel;
				directoryReqList[i].isInUse = RSSL_TRUE;
				pStoredRequest = &directoryReqList[i].dirRequest;
				rsslCopyRDMDirectoryMsg((RsslRDMDirectoryMsg*)pStoredRequest, (RsslRDMDirectoryMsg*)pRequest, &memoryBuffer);
				break;
			}
		}
	}

	return pStoredRequest;
}


/*
 * Sends a directory refresh to a channel.  This consists of getting
 * a message buffer, initializing the RsslRDMDirectoryRefresh structure, 
 * encoding it, and sending the encoded message.
 * pReactorChannel - The channel to send a login response to
 * pDirectoryRequest - The directory request that solicited this refresh.
 */
static RsslRet sendDirectoryRefresh(RsslReactor *pReactor, RsslReactorChannel* pReactorChannel, RsslRDMDirectoryRequest* pDirectoryRequest)
{
	RsslErrorInfo rsslErrorInfo;
	RsslBuffer* msgBuf = 0;
	RsslRet ret;

	/* get a buffer for the source directory response */
	msgBuf = rsslReactorGetBuffer(pReactorChannel, MAX_MSG_SIZE, RSSL_FALSE, &rsslErrorInfo);

	if (msgBuf != NULL)
	{
		RsslReactorSubmitOptions submitOpts;

		RsslRDMDirectoryRefresh dirRefresh;

		RsslRDMService service;

		RsslBuffer serviceNameString = { 256, (char *)alloca(256) };
		RsslBuffer vendorString = { 4, (char *)"LSEG" };
		RsslBuffer linkName = { 12, (char *)"rsslProvider" };

		RsslRDMServiceLink links[1];

		/* Advertise MarketPrice, and MarketByOrder, and Dictionary domains */
		RsslUInt capabilitiesList[] = { RSSL_DMT_DICTIONARY, RSSL_DMT_MARKET_PRICE, RSSL_DMT_MARKET_BY_ORDER, RSSL_DMT_MARKET_BY_PRICE, RSSL_DMT_YIELD_CURVE, RSSL_DMT_SYMBOL_LIST,
			RSSL_DMT_SYSTEM };
		RsslUInt32 capabilitiesCount = 7;

		/* Advertise that service provides and uses field and enum-type dictionaries */
		RsslBuffer dictionaryStrings[] = { { 6, (char *)"RWFFld"}, { 7, (char *)"RWFEnum" } };
		RsslUInt32 dictionariesCount = 2;

		/* Only supports real-time/tick-by-tick */
		RsslQos qosList[] = { { RSSL_QOS_TIME_REALTIME, RSSL_QOS_RATE_TICK_BY_TICK, 0, 0, 0} };
		RsslUInt32 qosCount = 1;

		RsslEncodeIterator encIter;

		rsslClearRDMDirectoryRefresh(&dirRefresh);
		rsslClearRDMService(&service);

		links[0].flags = RDM_SVC_LKF_HAS_TYPE;
		links[0].name = linkName;
		links[0].type = 1; /* Indicates we are interactive(see RDM Usage Guide) */

		serviceNameString.length = snprintf(serviceNameString.data, serviceNameString.length, "%s", serviceName);

		dirRefresh.rdmMsgBase.streamId = pDirectoryRequest->rdmMsgBase.streamId;

		dirRefresh.state.streamState = RSSL_STREAM_OPEN;
		dirRefresh.state.dataState = RSSL_DATA_OK;
		dirRefresh.state.code = RSSL_SC_NONE;

		dirRefresh.flags = RDM_DR_RFF_SOLICITED | RDM_DR_RFF_CLEAR_CACHE;
		dirRefresh.filter = pDirectoryRequest->filter;
		dirRefresh.serviceCount = 1;
		dirRefresh.serviceList = &service;

		service.flags = RDM_SVCF_HAS_INFO | RDM_SVCF_HAS_STATE;
		service.action = RSSL_MPEA_ADD_ENTRY;
		service.serviceId = getServiceId();

		/* Populate Service Info filter */

		service.info.serviceName = serviceNameString;
		service.info.vendor = vendorString; service.info.flags |= RDM_SVC_IFF_HAS_VENDOR;

		service.info.capabilitiesCount = capabilitiesCount;
		service.info.capabilitiesList = capabilitiesList;

		service.info.dictionariesProvidedCount = sizeof(dictionaryStrings)/sizeof(RsslBuffer);
		service.info.dictionariesProvidedList = dictionaryStrings;
		service.info.flags |= RDM_SVC_IFF_HAS_DICTS_PROVIDED;

		service.info.dictionariesUsedCount = sizeof(dictionaryStrings)/sizeof(RsslBuffer);
		service.info.dictionariesUsedList = dictionaryStrings;
		service.info.flags |= RDM_SVC_IFF_HAS_DICTS_USED;

		service.info.qosCount = sizeof(qosList)/sizeof(RsslQos);
		service.info.qosList = qosList;
		service.info.flags |= RDM_SVC_IFF_HAS_QOS;

		service.info.itemList.data = (char *)"_ETA_ITEM_LIST";
		service.info.itemList.length = 14;
		service.info.flags |= RDM_SVC_IFF_HAS_ITEM_LIST;

		/* Populate Service State filter */

		service.state.flags = RDM_SVC_STF_HAS_ACCEPTING_REQS;

		/* Service is up and accepting requests */
		service.state.serviceState = 1;
		service.state.acceptingRequests = 1;

		rsslClearEncodeIterator(&encIter);
		rsslSetEncodeIteratorRWFVersion(&encIter, pReactorChannel->majorVersion, pReactorChannel->minorVersion);
		if((ret = rsslSetEncodeIteratorBuffer(&encIter, msgBuf)) < RSSL_RET_SUCCESS)
		{
			rsslReactorReleaseBuffer(pReactorChannel, msgBuf, &rsslErrorInfo);
			printf("\nrsslSetEncodeIteratorBuffer() failed with return code: %d\n", ret);
			return RSSL_RET_FAILURE;
		}
		if (rsslEncodeRDMDirectoryMsg(&encIter, (RsslRDMDirectoryMsg*)&dirRefresh, &msgBuf->length, &rsslErrorInfo) != RSSL_RET_SUCCESS)
		{
			rsslReactorReleaseBuffer(pReactorChannel, msgBuf, &rsslErrorInfo);
			printf("\nrsslEncodeRDMDirectoryMsg() failed:%s(%s)\n", rsslErrorInfo.rsslError.text, rsslErrorInfo.errorLocation);
			return RSSL_RET_FAILURE;
		}

		/* If requested, populate service load filter */
		if (pDirectoryRequest->filter & RDM_DIRECTORY_SERVICE_LOAD_FILTER)
		{
			service.load.flags = RDM_SVC_LDF_HAS_OPEN_LIMIT;
			service.load.openLimit = OPEN_LIMIT;
		}

		/* If requested, populate service link filter */
		if (pDirectoryRequest->filter & RDM_DIRECTORY_SERVICE_LINK_FILTER)
		{
			service.linkInfo.linkCount = 1;
			service.linkInfo.linkList = links;
		}

		rsslClearReactorSubmitOptions(&submitOpts);
		/* send source directory response */
		if (ret = rsslReactorSubmit(pReactor, pReactorChannel, msgBuf, &submitOpts, &rsslErrorInfo) < RSSL_RET_SUCCESS)
		{
			rsslReactorReleaseBuffer(pReactorChannel, msgBuf, &rsslErrorInfo);
			printf("\nrsslReactorSubmit() failed: %d: %s\n", ret , rsslErrorInfo.rsslError.text);
			return RSSL_RET_FAILURE;
		}
	}
	else
	{
		printf("rsslReactorGetBuffer(): Failed <%s>\n", rsslErrorInfo.rsslError.text);
		return RSSL_RET_FAILURE;
	}

	return RSSL_RET_SUCCESS;
}


/*
 * Sends the directory request reject status message for a channel.
 * pReactorChannel - The channel to send request reject status message to
 * streamId - The stream id of the request
 * reason - The reason for the reject
 */
static RsslRet sendDirectoryRequestReject(RsslReactor *pReactor, RsslReactorChannel* pReactorChannel, RsslInt32 streamId, RsslSrcDirRejectReason reason, RsslErrorInfo *pError)
{
	RsslErrorInfo rsslErrorInfo;
	RsslBuffer* msgBuf = 0;

	/* get a buffer for the source directory request reject status */
	msgBuf = rsslReactorGetBuffer(pReactorChannel, MAX_MSG_SIZE, RSSL_FALSE, &rsslErrorInfo);

	if (msgBuf != NULL)
	{
		RsslRet ret = 0;
		char stateText[MAX_SRCDIR_INFO_STRLEN];
		RsslEncodeIterator encodeIter;
		RsslRDMDirectoryStatus directoryStatus;
		RsslErrorInfo rsslErrorInfo;

		rsslClearRDMDirectoryStatus(&directoryStatus);
		directoryStatus.flags |= RDM_DR_STF_HAS_STATE;
		directoryStatus.rdmMsgBase.streamId = streamId;
		directoryStatus.state.streamState = RSSL_STREAM_CLOSED_RECOVER;
		directoryStatus.state.dataState = RSSL_DATA_SUSPECT;

#if defined(__GNUC__) && (__GNUC__ >= 9)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wformat-truncation"
#endif
		switch(reason)
		{
			case MAX_SRCDIR_REQUESTS_REACHED:
				directoryStatus.state.code = RSSL_SC_TOO_MANY_ITEMS;
				snprintf(stateText, sizeof(stateText), "Source directory request rejected for stream id %d - max request count reached", streamId);
				directoryStatus.state.text.data = stateText;
				directoryStatus.state.text.length = (RsslUInt32)strlen(stateText) + 1;
				break;
			case INCORRECT_FILTER_FLAGS:
				directoryStatus.state.code = RSSL_SC_USAGE_ERROR;
				snprintf(stateText, sizeof(stateText), "Source directory request rejected for stream id %d - request must minimally have RDM_DIRECTORY_SERVICE_INFO_FILTER, RDM_DIRECTORY_SERVICE_STATE_FILTER, and RDM_DIRECTORY_SERVICE_GROUP_FILTER filter flags", streamId);
				directoryStatus.state.text.data = stateText;
				directoryStatus.state.text.length = (RsslUInt32)strlen(stateText) + 1;
				break;
			case DIRECTORY_RDM_DECODER_FAILED:
				directoryStatus.state.code = RSSL_SC_USAGE_ERROR;
				snprintf(stateText, sizeof(stateText), "Source directory request rejected for stream id %d - decoding failure: %s", streamId, pError->rsslError.text);
				directoryStatus.state.text.data = stateText;
				directoryStatus.state.text.length = (RsslUInt32)strlen(stateText) + 1;
				break;
			default:
				break;
		}
#if defined(__GNUC__) && (__GNUC__ >= 9)
#pragma GCC diagnostic pop
#endif

		/* encode message */
		rsslClearEncodeIterator(&encodeIter);
		if((ret = rsslSetEncodeIteratorBuffer(&encodeIter, msgBuf)) < RSSL_RET_SUCCESS)
		{
			rsslReactorReleaseBuffer(pReactorChannel, msgBuf, &rsslErrorInfo);
			printf("\nrsslSetEncodeIteratorBuffer() failed with return code: %d\n", ret);
			return RSSL_RET_FAILURE;
		}
		rsslSetEncodeIteratorRWFVersion(&encodeIter, pReactorChannel->majorVersion, pReactorChannel->minorVersion);
		if (ret = rsslEncodeRDMDirectoryMsg(&encodeIter, (RsslRDMDirectoryMsg*)&directoryStatus, &msgBuf->length, &rsslErrorInfo) != RSSL_RET_SUCCESS)
		{
			rsslReactorReleaseBuffer(pReactorChannel, msgBuf, &rsslErrorInfo);
			printf("\nrsslEncodeRDMDirectoryMsg() failed\n");
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
 * Closes the source directory stream for a channel. 
 * pReactorChannel - The channel to close the source directory stream for
 */
void closeDirectoryStreamForChannel(RsslReactorChannel* pReactorChannel)
{
	int i;
	RsslRDMDirectoryRequest* sourceDirectoryReqInfo = NULL;

	/* find original request information associated with pReactorChannel */
	for (i = 0; i < NUM_CLIENT_SESSIONS; i++)
	{
		if(directoryReqList[i].chnl == pReactorChannel)
		{
			sourceDirectoryReqInfo = &directoryReqList[i].dirRequest;
			/* clear original request information */
			printf("Closing source directory stream id %d with service name: %.*s\n", sourceDirectoryReqInfo->rdmMsgBase.streamId, (int)strlen(serviceName), serviceName);
			clearDirectoryRequestInfo(&directoryReqList[i]);
			break;
		}
	}
}

/* 
 * Closes a source directory stream. 
 * streamId - The stream id to close the source directory for
 */
static void closeDirectoryStream(RsslInt32 streamId)
{
	int i;
	RsslRDMDirectoryRequest* sourceDirectoryReqInfo = NULL;

	/* find original request information associated with streamId */
	for (i = 0; i < NUM_CLIENT_SESSIONS; i++)
	{
		if(directoryReqList[i].dirRequest.rdmMsgBase.streamId == streamId)
		{
			sourceDirectoryReqInfo = &directoryReqList[i].dirRequest;
			/* clear original request information */
			printf("Closing source directory stream id %d with service name: %.*s\n", sourceDirectoryReqInfo->rdmMsgBase.streamId, (int)strlen(serviceName), serviceName);
			clearDirectoryRequestInfo(&directoryReqList[i]);
			break;
		}
	}
}
