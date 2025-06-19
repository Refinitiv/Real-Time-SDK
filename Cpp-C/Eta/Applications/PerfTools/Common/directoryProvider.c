/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2015-2016,2019-2020,2024 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

#include "directoryProvider.h"
#include "xmlMsgDataParser.h"
#include <assert.h>

/* Contains the global DirectoryHandler configuration. */
DirectoryConfig directoryConfig;

/* Will be populated at initialization. */
RsslRDMService service;

static RsslUInt serviceCapabilities[4];
static RsslUInt32 serviceCapabilitiesCount;
static RsslBuffer serviceDictionaries[] = { { 6, (char*)"RWFFld" }, { 7, (char*)"RWFEnum" } };

void initDirectoryConfig()
{
	directoryConfig.qos.dynamic = RSSL_FALSE;
	directoryConfig.qos.rate = RSSL_QOS_RATE_TICK_BY_TICK;
	directoryConfig.qos.timeliness = RSSL_QOS_TIME_REALTIME;
	rsslClearRDMService(&service);
}

void directoryServiceInit()
{
	assert(xmlMsgDataLoaded);

	rsslClearRDMService(&service);
	service.flags = RDM_SVCF_HAS_INFO | RDM_SVCF_HAS_STATE | RDM_SVCF_HAS_LOAD;
	service.serviceId = directoryConfig.serviceId;
	service.action = RSSL_MPEA_ADD_ENTRY;


	/* Info */
	service.info.action = RSSL_FTEA_SET_ENTRY;

	service.info.serviceName.data = directoryConfig.serviceName;
	service.info.serviceName.length = (RsslUInt32)strlen(service.info.serviceName.data);

	service.info.flags |= RDM_SVC_IFF_HAS_VENDOR;
	service.info.vendor.data = (char*)"LSEG";
	service.info.vendor.length = (RsslUInt32)strlen(service.info.vendor.data);

	service.info.flags |= RDM_SVC_IFF_HAS_IS_SOURCE;
	service.info.isSource = 1;

	serviceCapabilitiesCount = 1;
	serviceCapabilities[0] = RSSL_DMT_DICTIONARY;

	/* Add domains to the service based on whether they were
	 * present in the XML parser found sample data for it. */
	if (xmlMsgDataHasMarketPrice)
	{
		serviceCapabilities[serviceCapabilitiesCount] = RSSL_DMT_MARKET_PRICE;
		++serviceCapabilitiesCount;
	}

	if (xmlMsgDataHasMarketByOrder)
	{
		serviceCapabilities[serviceCapabilitiesCount] = RSSL_DMT_MARKET_BY_ORDER;
		++serviceCapabilitiesCount;
	}

	/* For accepting tunnel stream */
	{
		serviceCapabilities[serviceCapabilitiesCount] = RSSL_DMT_SYSTEM;
		++serviceCapabilitiesCount;
	}

	service.info.capabilitiesList = serviceCapabilities;
	service.info.capabilitiesCount = serviceCapabilitiesCount;

	service.info.flags |= RDM_SVC_IFF_HAS_DICTS_PROVIDED;
	service.info.dictionariesProvidedList = serviceDictionaries;
	service.info.dictionariesProvidedCount = sizeof(serviceDictionaries)/sizeof(RsslBuffer);

	service.info.flags |= RDM_SVC_IFF_HAS_DICTS_USED;
	service.info.dictionariesUsedList = serviceDictionaries;
	service.info.dictionariesUsedCount = sizeof(serviceDictionaries)/sizeof(RsslBuffer);

	service.info.flags |= RDM_SVC_IFF_HAS_QOS;
	service.info.qosList = &directoryConfig.qos;
	service.info.qosCount = 1;

	service.info.flags |= RDM_SVC_IFF_HAS_SUPPORT_QOS_RANGE;
	service.info.supportsQosRange = 0;

	service.info.flags |= RDM_SVC_IFF_HAS_SUPPORT_OOB_SNAPSHOTS;
	service.info.supportsOutOfBandSnapshots = 0;

	service.info.flags |= RDM_SVC_IFF_HAS_ACCEPTING_CONS_STATUS;
	service.info.acceptingConsumerStatus = 0;

	/* State */
	service.state.action = RSSL_FTEA_SET_ENTRY;

	service.state.serviceState = 1;

	service.state.flags |= RDM_SVC_STF_HAS_ACCEPTING_REQS;
	service.state.acceptingRequests = 1;

	/* Load */
	if (directoryConfig.openLimit > 0)
	{
		service.load.action = RSSL_FTEA_SET_ENTRY;
		service.load.flags |= RDM_SVC_LDF_HAS_OPEN_LIMIT;
		service.load.openLimit = directoryConfig.openLimit;
	}
}

RsslRet processDirectoryRequest(ChannelHandler *pChannelHandler, ChannelInfo *pChannelInfo, RsslMsg* msg, RsslDecodeIterator* dIter)
{
	RsslRet ret;
	RsslState *pState = 0;
	RsslRDMDirectoryMsg directoryMsg;
	char directoryMsgChar[4000];
	RsslBuffer memoryBuffer = { 4000, directoryMsgChar };
	RsslErrorInfo errorInfo;
	RsslChannel *pChannel = pChannelInfo->pChannel;

	if ((ret = rsslDecodeRDMDirectoryMsg(dIter, msg, &directoryMsg, &memoryBuffer, &errorInfo)) != RSSL_RET_SUCCESS)
	{
		printf("rsslDecodeRDMDirectoryMsg() failed: %d(%s)\n", ret, errorInfo.rsslError.text);
		return ret;
	}

	switch(directoryMsg.rdmMsgBase.rdmMsgType)
	{
	case RDM_DR_MT_REQUEST:
	{
		RsslError error;
		RsslBuffer* msgBuf = 0;
		RsslChannel *pChannel = pChannelInfo->pChannel;
		RsslEncodeIterator eIter;
		RsslRDMDirectoryRefresh directoryRefresh;

		printf("Received Directory Request.\n\n");

		if ((msgBuf = rsslGetBuffer(pChannel, 512, RSSL_FALSE, &error)) == NULL)
		{
			printf("processDirectoryRequest(): rsslGetBuffer() failed: %d(%s)\n",
					error.rsslErrorId, error.text);
			return error.rsslErrorId;
		}

		rsslClearRDMDirectoryRefresh(&directoryRefresh);

		directoryRefresh.flags = RDM_DR_RFF_SOLICITED | RDM_DR_RFF_CLEAR_CACHE;
		directoryRefresh.filter = directoryMsg.request.filter;


		/* StreamId */
		directoryRefresh.rdmMsgBase.streamId = directoryMsg.request.rdmMsgBase.streamId;

		/* ServiceId */
		if (directoryMsg.request.flags & RDM_DR_RQF_HAS_SERVICE_ID)
		{
			/* Match the ServiceID if requested */
			directoryRefresh.flags |= RDM_DR_RFF_HAS_SERVICE_ID;
			directoryRefresh.serviceId = directoryMsg.request.serviceId;

			if (directoryMsg.request.serviceId == service.serviceId)
			{
				directoryRefresh.serviceList = &service;
				directoryRefresh.serviceCount = 1;
			}
		}
		else
		{
			directoryRefresh.serviceList = &service;
			directoryRefresh.serviceCount = 1;
		}

		rsslClearEncodeIterator(&eIter);
		rsslSetEncodeIteratorRWFVersion(&eIter, pChannel->majorVersion, pChannel->minorVersion);
		rsslSetEncodeIteratorBuffer(&eIter, msgBuf);

		if ((ret = rsslEncodeRDMDirectoryMsg(&eIter, (RsslRDMDirectoryMsg*)&directoryRefresh, &msgBuf->length, &errorInfo)) != RSSL_RET_SUCCESS)
		{
			printf("rsslEncodeRDMDirectoryMsg() failed: %d(%s)", ret, errorInfo.rsslError.text);
			return ret;
		}

		return channelHandlerWriteChannel(pChannelHandler, pChannelInfo, msgBuf, 0);
	}

	case RDM_DR_MT_CLOSE:
		printf("\nReceived Source Directory Close for StreamId %d\n", directoryMsg.rdmMsgBase.streamId);
		return RSSL_RET_SUCCESS;

	default:
		printf("\nReceived Unhandled Source Directory Msg Class: %d\n", msg->msgBase.msgClass);
		return RSSL_RET_FAILURE;
	}
}

RsslRet processDirectoryRequestReactor(RsslReactor *pReactor, RsslReactorChannel *pReactorChannel, RsslRDMDirectoryMsg *pDirectoryMsg)
{
	RsslRet ret;
	RsslState *pState = 0;
	RsslErrorInfo errorInfo;
	RsslReactorSubmitOptions submitOpts;

	switch(pDirectoryMsg->rdmMsgBase.rdmMsgType)
	{
	case RDM_DR_MT_REQUEST:
	{
		RsslError error;
		RsslBuffer* msgBuf = 0;
		RsslEncodeIterator eIter;
		RsslRDMDirectoryRefresh directoryRefresh;

		printf("Received Directory Request.\n\n");

		if ((msgBuf = rsslReactorGetBuffer(pReactorChannel, 512, RSSL_FALSE, &errorInfo)) == NULL)
		{
			printf("processDirectoryRequest(): rsslReactorGetBuffer() failed: %d(%s)\n",
					error.rsslErrorId, error.text);
			return error.rsslErrorId;
		}

		rsslClearRDMDirectoryRefresh(&directoryRefresh);

		directoryRefresh.flags = RDM_DR_RFF_SOLICITED | RDM_DR_RFF_CLEAR_CACHE;
		directoryRefresh.filter = pDirectoryMsg->request.filter;

		/* StreamId */
		directoryRefresh.rdmMsgBase.streamId = pDirectoryMsg->request.rdmMsgBase.streamId;

		/* ServiceId */
		if (pDirectoryMsg->request.flags & RDM_DR_RQF_HAS_SERVICE_ID)
		{
			/* Match the ServiceID if requested */
			directoryRefresh.flags |= RDM_DR_RFF_HAS_SERVICE_ID;
			directoryRefresh.serviceId = pDirectoryMsg->request.serviceId;

			if (pDirectoryMsg->request.serviceId == service.serviceId)
			{
				directoryRefresh.serviceList = &service;
				directoryRefresh.serviceCount = 1;
			}
		}
		else
		{
			directoryRefresh.serviceList = &service;
			directoryRefresh.serviceCount = 1;
		}

		rsslClearEncodeIterator(&eIter);
		rsslSetEncodeIteratorRWFVersion(&eIter, pReactorChannel->majorVersion, pReactorChannel->minorVersion);
		rsslSetEncodeIteratorBuffer(&eIter, msgBuf);

		if ((ret = rsslEncodeRDMDirectoryMsg(&eIter, (RsslRDMDirectoryMsg*)&directoryRefresh, &msgBuf->length, &errorInfo)) != RSSL_RET_SUCCESS)
		{
			printf("rsslEncodeRDMDirectoryMsg() failed: %d(%s)", ret, errorInfo.rsslError.text);
			return ret;
		}

		rsslClearReactorSubmitOptions(&submitOpts);
 		return rsslReactorSubmit(pReactor, pReactorChannel, msgBuf, &submitOpts, &errorInfo);
	}

	case RDM_DR_MT_CLOSE:
		printf("\nReceived Source Directory Close for StreamId %d\n", pDirectoryMsg->rdmMsgBase.streamId);
		return RSSL_RET_SUCCESS;

	default:
		printf("\nReceived Unhandled Source Directory Msg Type: %d\n", pDirectoryMsg->rdmMsgBase.rdmMsgType);
		return RSSL_RET_FAILURE;
	}
}

RsslRet publishDirectoryRefresh(ChannelHandler *pChannelHandler, ChannelInfo *pChannelInfo, RsslInt32 streamId)
{
	RsslError error;
	RsslBuffer* msgBuf = 0;
	RsslRet ret;
	RsslChannel *pChannel = pChannelInfo->pChannel;

	/* get a buffer for the source directory response */
	msgBuf = rsslGetBuffer(pChannel, 512, RSSL_FALSE, &error);

	if (msgBuf != NULL)
	{
		RsslEncodeIterator eIter;
		RsslErrorInfo errorInfo;

		RsslRDMDirectoryRefresh directoryRefresh;


		rsslClearRDMDirectoryRefresh(&directoryRefresh);

		directoryRefresh.flags = RDM_DR_RFF_HAS_SERVICE_ID | RDM_DR_RFF_CLEAR_CACHE;
		directoryRefresh.filter = RDM_DIRECTORY_SERVICE_INFO_FILTER | RDM_DIRECTORY_SERVICE_STATE_FILTER | RDM_DIRECTORY_SERVICE_GROUP_FILTER;

		/* StreamId */
		directoryRefresh.rdmMsgBase.streamId = streamId;

		directoryRefresh.serviceList = &service;
		directoryRefresh.serviceCount = 1;

		rsslClearEncodeIterator(&eIter);
		rsslSetEncodeIteratorRWFVersion(&eIter, pChannel->majorVersion, pChannel->minorVersion);
		rsslSetEncodeIteratorBuffer(&eIter, msgBuf);

		if ((ret = rsslEncodeRDMDirectoryMsg(&eIter, (RsslRDMDirectoryMsg*)&directoryRefresh, &msgBuf->length, &errorInfo)) != RSSL_RET_SUCCESS)
		{
			printf("rsslEncodeRDMDirectoryMsg() failed: %d(%s)", ret, errorInfo.rsslError.text);
			return ret;
		}

		return channelHandlerWriteChannel(pChannelHandler, pChannelInfo, msgBuf, 0);
	}
	else
	{
		printf("rsslGetBuffer(): Failed <%s>\n", error.text);
		return error.rsslErrorId;
	}
}
