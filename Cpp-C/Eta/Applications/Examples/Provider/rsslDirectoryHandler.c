
/*
 * This is the source directory handler for the rsslProvider application.
 * Only one source directory stream per channel is allowed by this simple
 * provider.  It provides functions for processing source directory requests
 * from consumers and sending back the responses.  Functions for sending
 * source directory request reject/close status messages, initializing the
 * source directory handler, setting the service name, getting/setting the
 * service id, checking if a request has minimal filter flags, and closing
 * source directory streams are also provided.
 */

#include "rsslDirectoryHandler.h"
#include "rsslSendMessage.h"

/* service name of provider */
static char* serviceName;
/* service id associated with the service name of provider */
static RsslUInt64 serviceId = 1234;
/* source directory request information list */
static RsslSourceDirectoryRequestInfo sourceDirectoryReqInfoList[NUM_CLIENT_SESSIONS];

/* vendor name */
static const char* vendorName = "Refinitiv";
/* field dictionary name */
static const char *fieldDictionaryName = "RWFFld";
/* enumtype dictionary name */
static const char *enumTypeDictionaryName = "RWFEnum";
/* link name */
static const char* linkName = "rsslProvider link";

/*
 * Initializes source directory information fields.
 */
void initDirectoryHandler()
{
	int i;

	for(i = 0; i < NUM_CLIENT_SESSIONS; ++i)
	{
		clearSourceDirectoryReqInfo(&sourceDirectoryReqInfoList[i]);
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

RsslRet serviceNameToIdCallback(RsslBuffer* name, RsslUInt16* Id)
{
	int i = 0;
	for (i = 0; i < MAX_SOURCE_DIRECTORY_SERVICES; i++)
	{
		if (strlen(serviceName) == name->length)
		{

			if (!strncmp(serviceName, name->data, name->length))
			{
				*Id = (RsslUInt16)serviceId;
			}
			return RSSL_RET_SUCCESS;
		}
	}

	return RSSL_RET_FAILURE;
}


/*
 * Gets a source directory request information structure for a channel.
 * chnl - The channel to get the source directory request information structure for
 * msg - The partially decoded message
 * key - The message key
 */
static RsslSourceDirectoryRequestInfo* getSourceDirectoryReqInfo(RsslChannel* chnl, RsslMsg* msg, RsslMsgKey* key)
{
	int i;
	RsslSourceDirectoryRequestInfo* sourceDirectoryReqInfo = NULL;

	/* first check if one already in use for this channel */
	for (i = 0; i < NUM_CLIENT_SESSIONS; i++)
	{
		if (sourceDirectoryReqInfoList[i].IsInUse &&
			sourceDirectoryReqInfoList[i].Chnl == chnl)
		{
			sourceDirectoryReqInfo = &sourceDirectoryReqInfoList[i];
			/* if stream id is different from last request, this is an invalid request */
			if (sourceDirectoryReqInfo->StreamId != msg->msgBase.streamId)
			{
				return NULL;
			}
			break;
		}
	}

	/* get a new one if one is not already in use */
	if (!sourceDirectoryReqInfo)
	{
		for (i = 0; i < NUM_CLIENT_SESSIONS; i++)
		{
			if(sourceDirectoryReqInfoList[i].IsInUse == RSSL_FALSE)
			{
				sourceDirectoryReqInfoList[i].Chnl = chnl;
				sourceDirectoryReqInfoList[i].IsInUse = RSSL_TRUE;
				sourceDirectoryReqInfo = &sourceDirectoryReqInfoList[i];
				if (rsslCopyMsgKey(&sourceDirectoryReqInfo->MsgKey, key) == RSSL_RET_FAILURE)
				{
					return NULL;
				}
				break;
			}
		}
	}

	return sourceDirectoryReqInfo;
}

/*
 * Processes a source directory request.  This consists of calling
 * decodeSourceDirectoryRequest() to decode the request and calling
 * sendSourceDirectoryResponse() to send the source directory response.
 * chnl - The channel of the response
 * msg - The partially decoded message
 * dIter - The decode iterator
 */
RsslRet processSourceDirectoryRequest(RsslChannel* chnl, RsslMsg* msg, RsslDecodeIterator* dIter)
{
	RsslMsgKey* key = 0;
	RsslSourceDirectoryRequestInfo *srcDirectoryReqInfo;

	switch(msg->msgBase.msgClass)
	{
	case RSSL_MC_REQUEST:
		/* get key */
		key = (RsslMsgKey *)rsslGetMsgKey(msg);

		/* check if key has minimal filter flags */
		if (!keyHasMinFilterFlags(key))
		{
			if (sendSrcDirectoryRequestReject(chnl, msg->msgBase.streamId, INCORRECT_FILTER_FLAGS) != RSSL_RET_SUCCESS)
				return RSSL_RET_FAILURE;
			break;
		}

		srcDirectoryReqInfo = getSourceDirectoryReqInfo(chnl, msg, key);

		if (!srcDirectoryReqInfo)
		{
			if (sendSrcDirectoryRequestReject(chnl, msg->msgBase.streamId, MAX_SRCDIR_REQUESTS_REACHED) != RSSL_RET_SUCCESS)
				return RSSL_RET_FAILURE;
			break;
		}
		/* decode source directory request */
		if (decodeSourceDirectoryRequest(srcDirectoryReqInfo, msg, dIter) != RSSL_RET_SUCCESS)
		{
			printf("\ndecodeSourceDirectoryRequest() failed\n");
			return RSSL_RET_FAILURE;
		}

		printf("\nReceived Source Directory Request\n");

		/* send source directory response */
		if (sendSourceDirectoryResponse(chnl, srcDirectoryReqInfo) != RSSL_RET_SUCCESS)
			return RSSL_RET_FAILURE;
		break;

	case RSSL_MC_CLOSE:
		printf("\nReceived Source Directory Close for StreamId %d\n", msg->msgBase.streamId);

		/* close source directory stream */
		closeSrcDirectoryStream(msg->msgBase.streamId);

		break;

	default:
		printf("\nReceived Unhandled Source Directory Msg Class: %d\n", msg->msgBase.msgClass);
    	return RSSL_RET_FAILURE;
	}

	return RSSL_RET_SUCCESS;
}

/*
 * Sends a source directory response to a channel.  This consists
 * of getting a message buffer, setting the source directory response
 * information, encoding the source directory response, and sending
 * the source directory response to the server.
 * chnl - The channel to send a source directory response to
 * srcDirReqInfo - The source directory request information
 */
static RsslRet sendSourceDirectoryResponse(RsslChannel* chnl, RsslSourceDirectoryRequestInfo* srcDirReqInfo)
{
	RsslError error;
	RsslBuffer* msgBuf = 0;
	RsslSourceDirectoryResponseInfo srcDirRespInfo;
	RsslUInt16 refreshFlags = 0;

	/* initialize source directory response info */
	initSourceDirRespInfo(&srcDirRespInfo);

	/* get a buffer for the source directory response */
	msgBuf = rsslGetBuffer(chnl, MAX_MSG_SIZE, RSSL_FALSE, &error);

	if (msgBuf != NULL)
	{
		/* provide source directory response information */
		/* set refresh flags */
		refreshFlags = RSSL_RFMF_HAS_MSG_KEY | RSSL_RFMF_SOLICITED | RSSL_RFMF_REFRESH_COMPLETE | RSSL_RFMF_CLEAR_CACHE;
		/* StreamId */
		srcDirRespInfo.StreamId = srcDirReqInfo->StreamId;
		/* ServiceId */
		srcDirRespInfo.ServiceId = getServiceId();
		/* ServiceName */
		snprintf(srcDirRespInfo.ServiceGeneralInfo.ServiceName, 256, "%s", serviceName);
		/* Vendor */
		snprintf(srcDirRespInfo.ServiceGeneralInfo.Vendor, 256, "%s", vendorName);
		/* Capabilities */
		srcDirRespInfo.ServiceGeneralInfo.Capabilities[0] = RSSL_DMT_DICTIONARY;
		srcDirRespInfo.ServiceGeneralInfo.Capabilities[1] = RSSL_DMT_MARKET_PRICE;
		srcDirRespInfo.ServiceGeneralInfo.Capabilities[2] = RSSL_DMT_MARKET_BY_ORDER;
		srcDirRespInfo.ServiceGeneralInfo.Capabilities[3] = RSSL_DMT_MARKET_BY_PRICE;
		srcDirRespInfo.ServiceGeneralInfo.Capabilities[4] = RSSL_DMT_SYMBOL_LIST;
		srcDirRespInfo.ServiceGeneralInfo.Capabilities[5] = RSSL_DMT_YIELD_CURVE;
		/* DictionariesProvided */
		snprintf(srcDirRespInfo.ServiceGeneralInfo.DictionariesProvided[0], 256, "%s", fieldDictionaryName);
		snprintf(srcDirRespInfo.ServiceGeneralInfo.DictionariesProvided[1], 256, "%s", enumTypeDictionaryName);
		/* DictionariesUsed */
		snprintf(srcDirRespInfo.ServiceGeneralInfo.DictionariesUsed[0], 256, "%s", fieldDictionaryName);
		snprintf(srcDirRespInfo.ServiceGeneralInfo.DictionariesUsed[1], 256, "%s", enumTypeDictionaryName);
		/* Qos */
		srcDirRespInfo.ServiceGeneralInfo.QoS[0].dynamic = RSSL_FALSE;
		srcDirRespInfo.ServiceGeneralInfo.QoS[0].rate = RSSL_QOS_RATE_TICK_BY_TICK;
		srcDirRespInfo.ServiceGeneralInfo.QoS[0].timeliness = RSSL_QOS_TIME_REALTIME;
		/* ItemList */
		snprintf(srcDirRespInfo.ServiceGeneralInfo.ItemList, 256, "_ETA_ITEM_LIST");
		/* Service StateInfo Status */
		srcDirRespInfo.ServiceStateInfo.Status.streamState = RSSL_STREAM_OPEN;
		srcDirRespInfo.ServiceStateInfo.Status.dataState = RSSL_DATA_OK;
		srcDirRespInfo.ServiceStateInfo.Status.code = RSSL_SC_NONE;
		srcDirRespInfo.ServiceStateInfo.Status.text.data = (char *)"OK";
		srcDirRespInfo.ServiceStateInfo.Status.text.length = (RsslUInt32)strlen("OK");
		/* OpenLimit */
		srcDirRespInfo.ServiceLoadInfo.OpenLimit = OPEN_LIMIT;
		/* Link Name */
		snprintf(srcDirRespInfo.ServiceLinkInfo[0].LinkName, 256, "%s", linkName);
		/* Link Text */
		snprintf(srcDirRespInfo.ServiceLinkInfo[0].Text, 256, "Link state is up");
		/* keep default values for all others */

		/* encode source directory response */
		if (encodeSourceDirectoryResponse(chnl, &srcDirRespInfo, &srcDirReqInfo->MsgKey, msgBuf, refreshFlags) != RSSL_RET_SUCCESS)
		{
			rsslReleaseBuffer(msgBuf, &error);
			printf("\nencodeSourceDirectoryResponse() failed\n");
			return RSSL_RET_FAILURE;
		}

		/* send source directory response */
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
 * Sends the source directory request reject status message for a channel.
 * chnl - The channel to send request reject status message to
 * streamId - The stream id of the request
 * reason - The reason for the reject
 */
static RsslRet sendSrcDirectoryRequestReject(RsslChannel* chnl, RsslInt32 streamId, RsslSrcDirRejectReason reason)
{
	RsslError error;
	RsslBuffer* msgBuf = 0;

	/* get a buffer for the source directory request reject status */
	msgBuf = rsslGetBuffer(chnl, MAX_MSG_SIZE, RSSL_FALSE, &error);

	if (msgBuf != NULL)
	{
		/* encode source directory request reject status */
		if (encodeSrcDirectoryRequestReject(chnl, streamId, reason, msgBuf) != RSSL_RET_SUCCESS)
		{
			rsslReleaseBuffer(msgBuf, &error);
			printf("\nencodeSrcDirectoryRequestReject() failed\n");
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
 * Does key have minimal filter flags.  Request key must minimally have
 * RDM_DIRECTORY_SERVICE_INFO_FILTER, RDM_DIRECTORY_SERVICE_STATE_FILTER,
 * and RDM_DIRECTORY_SERVICE_GROUP_FILTER filter flags.
 * key - The message key
 */
static RsslBool keyHasMinFilterFlags(RsslMsgKey* key)
{
	return ((key->flags & RSSL_MKF_HAS_FILTER) &&
		(key->filter & RDM_DIRECTORY_SERVICE_INFO_FILTER) &&
		(key->filter & RDM_DIRECTORY_SERVICE_STATE_FILTER) &&
		(key->filter & RDM_DIRECTORY_SERVICE_GROUP_FILTER));
}

/* 
 * Closes the source directory stream for a channel. 
 * chnl - The channel to close the source directory stream for
 */
void closeSrcDirectoryChnlStream(RsslChannel* chnl)
{
	int i;
	RsslSourceDirectoryRequestInfo* sourceDirectoryReqInfo = NULL;

	/* find original request information associated with chnl */
	for (i = 0; i < NUM_CLIENT_SESSIONS; i++)
	{
		if(sourceDirectoryReqInfoList[i].Chnl == chnl)
		{
			sourceDirectoryReqInfo = &sourceDirectoryReqInfoList[i];
			/* clear original request information */
			printf("Closing source directory stream id %d with service name: %.*s\n", sourceDirectoryReqInfo->StreamId, (int)strlen(serviceName), serviceName);
			clearSourceDirectoryReqInfo(sourceDirectoryReqInfo);
			break;
		}
	}
}

/* 
 * Closes a source directory stream. 
 * streamId - The stream id to close the source directory for
 */
static void closeSrcDirectoryStream(RsslInt32 streamId)
{
	int i;
	RsslSourceDirectoryRequestInfo* sourceDirectoryReqInfo = NULL;

	/* find original request information associated with streamId */
	for (i = 0; i < NUM_CLIENT_SESSIONS; i++)
	{
		if(sourceDirectoryReqInfoList[i].StreamId == streamId)
		{
			sourceDirectoryReqInfo = &sourceDirectoryReqInfoList[i];
			/* clear original request information */
			printf("Closing source directory stream id %d with service name: %.*s\n", sourceDirectoryReqInfo->StreamId, (int)strlen(serviceName), serviceName);
			clearSourceDirectoryReqInfo(sourceDirectoryReqInfo);
			break;
		}
	}
}
