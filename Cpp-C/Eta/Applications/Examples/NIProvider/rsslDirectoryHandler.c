/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2015,2019-2020,2022-2024 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

/*
 * This is the source directory handler for the rsslNIProvider application.
 * It provides functions for sending a source directory response and close
 * status message to an ADH.  Functions for setting the service name and
 * getting/setting the service id are also provided.
 */

#include "rsslDirectoryHandler.h"
#include "rsslDirectoryEncodeDecode.h"
#include "rsslSendMessage.h"

/* service name of provider */
static char* serviceName;
/* service id associated with the service name of provider */
static RsslUInt64 serviceId = 1;

/* vendor name */
static const char* vendorName = "LSEG";
/* field dictionary name */
static const char *fieldDictionaryName = "RWFFld";
/* enumtype dictionary name */
static const char *enumTypeDictionaryName = "RWFEnum";
/* link name */
static const char* linkName = "rsslNIProvider link";

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
 * Sends a source directory response to a channel.  This consists
 * of getting a message buffer, setting the source directory response
 * information, encoding the source directory response, and sending
 * the source directory response to the server.
 * chnl - The channel to send a source directory response to
 */
RsslRet sendSourceDirectoryResponse(RsslChannel* chnl)
{
	RsslError error;
	RsslBuffer* msgBuf = 0;
	RsslSourceDirectoryResponseInfo srcDirRespInfo;
	RsslUInt16 refreshFlags = 0;
	RsslMsgKey key = RSSL_INIT_MSG_KEY;

	/* initialize source directory response info */
	initSourceDirRespInfo(&srcDirRespInfo);

	/* get a buffer for the source directory response */
	msgBuf = rsslGetBuffer(chnl, MAX_MSG_SIZE, RSSL_FALSE, &error);

	if (msgBuf != NULL)
	{
		/* provide source directory response information */
		/* set refresh flags */
		refreshFlags = RSSL_RFMF_HAS_MSG_KEY | RSSL_RFMF_REFRESH_COMPLETE | RSSL_RFMF_CLEAR_CACHE;
		/* set filter flags */
		key.filter = RDM_DIRECTORY_SERVICE_INFO_FILTER | \
					RDM_DIRECTORY_SERVICE_STATE_FILTER| \
					/*RDM_DIRECTORY_SERVICE_GROUP_FILTER | \ not applicable for refresh message - here for reference*/
					RDM_DIRECTORY_SERVICE_LOAD_FILTER | \
					/*RDM_DIRECTORY_SERVICE_DATA_FILTER | \ not applicable for non-ANSI Page based provider - here for reference*/
					RDM_DIRECTORY_SERVICE_LINK_FILTER;
		/* StreamId */
		srcDirRespInfo.StreamId = SRCDIR_STREAM_ID;
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
		snprintf(srcDirRespInfo.ServiceGeneralInfo.ItemList, 256, "%s", "");
		/* Service StateInfo Status */
		srcDirRespInfo.ServiceStateInfo.Status.streamState = RSSL_STREAM_OPEN;
		srcDirRespInfo.ServiceStateInfo.Status.dataState = RSSL_DATA_OK;
		srcDirRespInfo.ServiceStateInfo.Status.code = RSSL_SC_NONE;
		srcDirRespInfo.ServiceStateInfo.Status.text.data = (char *)"OK";
		srcDirRespInfo.ServiceStateInfo.Status.text.length = (RsslUInt32)strlen("OK");
		/* OpenLimit */
		srcDirRespInfo.ServiceLoadInfo.OpenLimit = OPEN_LIMIT;
		srcDirRespInfo.ServiceLoadInfo.OpenWindow = 0;	 /* Don't send OpenWindow. */
		/* Link Name */
		snprintf(srcDirRespInfo.ServiceLinkInfo[0].LinkName, 256, "%s", linkName);
		/* Link Text */
		snprintf(srcDirRespInfo.ServiceLinkInfo[0].Text, 256, "Link state is up");
		/* keep default values for all others */

		/* encode source directory response */
		if (encodeSourceDirectoryResponse(chnl, &srcDirRespInfo, &key, msgBuf, refreshFlags) != RSSL_RET_SUCCESS)
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

