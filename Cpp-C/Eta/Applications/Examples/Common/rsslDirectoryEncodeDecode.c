/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2015,2022,2024 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

/*
 * This file contains all source directory encode and decode
 * functions required by the rsslConsumer, rsslProvider, and
 * rsslNIProvider applications.
 */

#include "rsslDirectoryEncodeDecode.h"

/*
 * Encodes the source directory request.  Returns success
 * if encoding succeeds or failure if encoding fails.
 * chnl - The channel to send a source directory request to
 * msgBuf - The message buffer to encode the source directory request into
 * streamId - The stream id of the source directory request
 */
RsslRet encodeSourceDirectoryRequest(RsslChannel* chnl, RsslBuffer* msgBuf, RsslInt32 streamId)
{
	RsslRet ret = 0;
	RsslRequestMsg msg = RSSL_INIT_REQUEST_MSG;
	RsslEncodeIterator encodeIter;

	/* clear encode iterator */
	rsslClearEncodeIterator(&encodeIter);

	/* set-up message */
	msg.msgBase.msgClass = RSSL_MC_REQUEST;
	msg.msgBase.streamId = streamId;
	msg.msgBase.domainType = RSSL_DMT_SOURCE;
	msg.msgBase.containerType = RSSL_DT_NO_DATA;
	msg.flags = RSSL_RQMF_STREAMING | RSSL_RQMF_HAS_PRIORITY;
	msg.priorityClass = 1;
	msg.priorityCount = 1;


	/* set members in msgKey */
	msg.msgBase.msgKey.flags = RSSL_MKF_HAS_FILTER;
	msg.msgBase.msgKey.filter = RDM_DIRECTORY_SERVICE_INFO_FILTER | \
								RDM_DIRECTORY_SERVICE_STATE_FILTER | \
								RDM_DIRECTORY_SERVICE_GROUP_FILTER | \
								RDM_DIRECTORY_SERVICE_LOAD_FILTER | \
								RDM_DIRECTORY_SERVICE_DATA_FILTER | \
								RDM_DIRECTORY_SERVICE_LINK_FILTER;
	
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
 * Decodes the source directory request into the RsslSourceDirectoryRequestInfo
 * structure. Returns success if decoding succeeds or failure if decoding fails.
 * srcDirReqInfo - The source directory request information structure
 * msg - The partially decoded message
 * dIter - The decode iterator
 */
RsslRet decodeSourceDirectoryRequest(RsslSourceDirectoryRequestInfo* srcDirReqInfo, RsslMsg* msg, RsslDecodeIterator* dIter)
{
	/* get StreamId */
	srcDirReqInfo->StreamId = msg->requestMsg.msgBase.streamId;

	return RSSL_RET_SUCCESS;
}

/*
 * Encodes the source directory response.  Returns success if encoding
 * succeeds or failure if encoding fails.
 * chnl - The channel to send a source directory response to
 * srcDirRespInfo - The source directory response information to be encoded
 * requestKey - The request key with filter flags that indicate the filter entries to include
 * msgBuf - The message buffer to encode the source directory response into
 * refreshFlags - The refresh flags of the source directory response
 */
RsslRet encodeSourceDirectoryResponse(RsslChannel* chnl, RsslSourceDirectoryResponseInfo* srcDirRespInfo, RsslMsgKey* requestKey, RsslBuffer* msgBuf, RsslUInt16 refreshFlags)
{
	RsslRet ret = 0;
	RsslRefreshMsg msg = RSSL_INIT_REFRESH_MSG;
	RsslMap map = RSSL_INIT_MAP;
	RsslMapEntry mEntry = RSSL_INIT_MAP_ENTRY;
	RsslFilterList rsslFilterList = RSSL_INIT_FILTER_LIST;
	char stateText[MAX_SRCDIR_INFO_STRLEN];
	RsslBuffer tempBuffer;
	RsslEncodeIterator encodeIter;

	/* clear encode iterator */
	rsslClearEncodeIterator(&encodeIter);

	/* set-up message */
	msg.msgBase.msgClass = RSSL_MC_REFRESH;
	msg.msgBase.domainType = RSSL_DMT_SOURCE;
	msg.msgBase.containerType = RSSL_DT_MAP;
	msg.flags = refreshFlags;
	msg.state.streamState = RSSL_STREAM_OPEN;
	msg.state.dataState = RSSL_DATA_OK;
	msg.state.code = RSSL_SC_NONE;
	snprintf(stateText, 256, "Source Directory Refresh Completed");
	msg.state.text.data = stateText;
	msg.state.text.length = (RsslUInt32)strlen(stateText);

	/* set members in msgKey */
	msg.msgBase.msgKey.flags = RSSL_MKF_HAS_FILTER;
	msg.msgBase.msgKey.filter = requestKey->filter;

	/* StreamId */
	msg.msgBase.streamId = srcDirRespInfo->StreamId;

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
	
	/* encode map */
	map.keyPrimitiveType = RSSL_DT_UINT;
	map.containerType = RSSL_DT_FILTER_LIST;
	if ((ret = rsslEncodeMapInit(&encodeIter, &map, 0, 0)) < RSSL_RET_SUCCESS)
	{
		printf("rsslEncodeMapInit() failed with return code: %d\n", ret);
		return ret;
	}

	/* check if service id and/or service name is present in request key */
	/* if present, they must match those for provider */
	tempBuffer.data = srcDirRespInfo->ServiceGeneralInfo.ServiceName;
	tempBuffer.length = (RsslUInt32)strlen(srcDirRespInfo->ServiceGeneralInfo.ServiceName);
	if ((!(requestKey->flags & RSSL_MKF_HAS_SERVICE_ID) && !(requestKey->flags & RSSL_MKF_HAS_NAME)) ||
		((requestKey->flags & RSSL_MKF_HAS_SERVICE_ID) && (requestKey->serviceId == srcDirRespInfo->ServiceId) && !(requestKey->flags & RSSL_MKF_HAS_NAME)) ||
		((requestKey->flags & RSSL_MKF_HAS_NAME) && rsslBufferIsEqual(&requestKey->name, &tempBuffer) && !(requestKey->flags & RSSL_MKF_HAS_SERVICE_ID)) ||
		(((requestKey->flags & RSSL_MKF_HAS_SERVICE_ID) && (requestKey->serviceId == srcDirRespInfo->ServiceId) &&
		  (requestKey->flags & RSSL_MKF_HAS_NAME) && rsslBufferIsEqual(&requestKey->name, &tempBuffer))))
	{
		/* encode map entry */
		mEntry.action = RSSL_MPEA_ADD_ENTRY;
		if ((ret = rsslEncodeMapEntryInit(&encodeIter, &mEntry, &srcDirRespInfo->ServiceId, 0)) < RSSL_RET_SUCCESS)
		{
			printf("rsslEncodeMapEntry() failed with return code: %d\n", ret);
			return ret;
		}

		/* encode filter list */
		rsslFilterList.containerType = RSSL_DT_ELEMENT_LIST;
		if ((ret = rsslEncodeFilterListInit(&encodeIter, &rsslFilterList)) < RSSL_RET_SUCCESS)
		{
			printf("rsslEncodeFilterListInit() failed with return code: %d\n", ret);
			return ret;
		}

		/* encode filter list items */
		if (requestKey->filter & RDM_DIRECTORY_SERVICE_INFO_FILTER)
		{
			if ((ret = encodeServiceGeneralInfo(&srcDirRespInfo->ServiceGeneralInfo, &encodeIter)) != RSSL_RET_SUCCESS)
			{
				printf("encodeServiceGeneralInfo() failed with return code: %d\n", ret);
				return ret;
			}
		}
		if (requestKey->filter & RDM_DIRECTORY_SERVICE_STATE_FILTER)
		{
			if ((ret = encodeServiceStateInfo(&srcDirRespInfo->ServiceStateInfo, &encodeIter)) != RSSL_RET_SUCCESS)
			{
				printf("encodeServiceStateInfo() failed with return code: %d\n", ret);
				return ret;
			}
		}
		/* not applicable for refresh message - here for reference */
		/*if (requestKey->filter & RDM_DIRECTORY_SERVICE_GROUP_FILTER)
		{
			if ((ret = encodeServiceGroupInfo(&srcDirRespInfo->ServiceGroupInfo, &encodeIter)) != RSSL_RET_SUCCESS)
			{
				printf("encodeServiceGroupInfo() failed with return code: %d\n", ret);
				return ret;
			}
		}*/
		if (requestKey->filter & RDM_DIRECTORY_SERVICE_LOAD_FILTER)
		{
			if ((ret = encodeServiceLoadInfo(&srcDirRespInfo->ServiceLoadInfo, &encodeIter)) != RSSL_RET_SUCCESS)
			{
				printf("encodeServiceLoadInfo() failed with return code: %d\n", ret);
				return ret;
			}
		}
		/* not applicable for non-ANSI Page based provider - here for reference */
		/*		if (requestKey->filter & RDM_DIRECTORY_SERVICE_DATA_FILTER)
		{
			if ((ret = encodeServiceDataInfo(&srcDirRespInfo->ServiceDataInfo, &encodeIter)) != RSSL_RET_SUCCESS)
			{
				printf("encodeServiceDataInfo() failed with return code: %d\n", ret);
				return ret;
			}
		}*/
		if (requestKey->filter & RDM_DIRECTORY_SERVICE_LINK_FILTER)
		{
			if ((ret = encodeServiceLinkInfo(&srcDirRespInfo->ServiceLinkInfo[0], &encodeIter)) != RSSL_RET_SUCCESS)
			{
				printf("encodeServiceLinkInfo() failed with return code: %d\n", ret);
				return ret;
			}
		}

		/* complete encode filter list */
		if ((ret = rsslEncodeFilterListComplete(&encodeIter, RSSL_TRUE)) < RSSL_RET_SUCCESS)
		{
			printf("rsslEncodeFilterListComplete() failed with return code: %d\n", ret);
			return ret;
		}

		/* complete encode map entry */
		if ((ret = rsslEncodeMapEntryComplete(&encodeIter, RSSL_TRUE)) < RSSL_RET_SUCCESS)
		{
			printf("rsslEncodeMapEntryComplete() failed with return code: %d\n", ret);
			return ret;
		}
	}

	/* complete encode map */
	if ((ret = rsslEncodeMapComplete(&encodeIter, RSSL_TRUE)) < RSSL_RET_SUCCESS)
	{
		printf("rsslEncodeMapComplete() failed with return code: %d\n", ret);
		return ret;
	}

	/* complete encode message */
	if ((ret = rsslEncodeMsgComplete(&encodeIter, RSSL_TRUE)) < RSSL_RET_SUCCESS)
	{
		printf("rsslEncodeMsgComplete() failed with return code: %d\n", ret);
		return ret;
	}
	msgBuf->length = rsslGetEncodedBufferLength(&encodeIter);

	return RSSL_RET_SUCCESS;
}

/*
 * Encodes the service's general information.  Returns success if encoding
 * succeeds or failure if encoding fails.
 * serviceGeneralInfo - The service's general information to be encoded
 * eIter - The encode iterator
 */
RsslRet encodeServiceGeneralInfo(RsslServiceGeneralInfo* serviceGeneralInfo, RsslEncodeIterator* eIter)
{
	RsslRet ret = 0;
	RsslFilterEntry filterListItem = RSSL_INIT_FILTER_ENTRY;
	RsslElementEntry element = RSSL_INIT_ELEMENT_ENTRY;
	RsslElementList	elementList = RSSL_INIT_ELEMENT_LIST;
	RsslArray rsslArray = RSSL_INIT_ARRAY;
	RsslBuffer tempBuffer;
	int i;

	/* encode filter list item */
	filterListItem.id = RDM_DIRECTORY_SERVICE_INFO_ID;
	filterListItem.action = RSSL_FTEA_SET_ENTRY;
	if ((ret = rsslEncodeFilterEntryInit(eIter, &filterListItem, 0)) < RSSL_RET_SUCCESS)
	{
		printf("rsslEncodeFilterEntryInit() failed with return code: %d\n", ret);
		return ret;
	}

	/* encode the element list */
	elementList.flags = RSSL_ELF_HAS_STANDARD_DATA;
	if ((ret = rsslEncodeElementListInit(eIter, &elementList, 0, 0)) < RSSL_RET_SUCCESS)
	{
		printf("rsslEncodeElementListInit() failed with return code: %d\n", ret);
		return ret;
	}
	/* ServiceName */
	tempBuffer.data = (char*)serviceGeneralInfo->ServiceName;
	tempBuffer.length = (RsslUInt32)strlen(serviceGeneralInfo->ServiceName);
	element.dataType = RSSL_DT_ASCII_STRING;
	element.name = RSSL_ENAME_NAME;
	if ((ret = rsslEncodeElementEntry(eIter, &element, &tempBuffer)) < RSSL_RET_SUCCESS)
	{
		printf("rsslEncodeElementEntry() failed with return code: %d\n", ret);
		return ret;
	}
	/* Vendor */
	tempBuffer.data = (char*)serviceGeneralInfo->Vendor;
	tempBuffer.length = (RsslUInt32)strlen(serviceGeneralInfo->Vendor);
	element.dataType = RSSL_DT_ASCII_STRING;
	element.name = RSSL_ENAME_VENDOR;
	if ((ret = rsslEncodeElementEntry(eIter, &element, &tempBuffer)) < RSSL_RET_SUCCESS)
	{
		printf("rsslEncodeElementEntry() failed with return code: %d\n", ret);
		return ret;
	}
	/* IsSource */
	element.dataType = RSSL_DT_UINT;
	element.name = RSSL_ENAME_IS_SOURCE;
	if ((ret = rsslEncodeElementEntry(eIter, &element, &serviceGeneralInfo->IsSource)) < RSSL_RET_SUCCESS)
	{
		printf("rsslEncodeElementEntry() failed with return code: %d\n", ret);
		return ret;
	}
	/* Capabilities */
	element.dataType = RSSL_DT_ARRAY;
	element.name = RSSL_ENAME_CAPABILITIES;
	if ((ret = rsslEncodeElementEntryInit(eIter, &element, 0)) < RSSL_RET_SUCCESS)
	{
		printf("rsslEncodeElementEntryInit() failed with return code: %d\n", ret);
		return ret;
	}
	rsslClearArray(&rsslArray);
	rsslArray.itemLength = 1;
	rsslArray.primitiveType = RSSL_DT_UINT;
	if ((ret = rsslEncodeArrayInit(eIter, &rsslArray)) < RSSL_RET_SUCCESS)
	{
		printf("rsslEncodeArrayInit() failed with return code: %d\n", ret);
		return ret;
	}
	for(i = 0; i < MAX_CAPABILITIES && serviceGeneralInfo->Capabilities[i] != 0; i++)
	{
		if ((ret = rsslEncodeArrayEntry(eIter, 0, &serviceGeneralInfo->Capabilities[i])) < RSSL_RET_SUCCESS)
		{
			printf("rsslEncodeArrayEntry() failed with return code: %d\n", ret);
			return ret;
		}
	}
	if ((ret = rsslEncodeArrayComplete(eIter, RSSL_TRUE)) < RSSL_RET_SUCCESS)
	{
		printf("rsslEncodeArrayComplete() failed with return code: %d\n", ret);
		return ret;
	}
	if ((ret = rsslEncodeElementEntryComplete(eIter, RSSL_TRUE)) < RSSL_RET_SUCCESS)
	{
		printf("rsslEncodeElementEntryComplete() failed with return code: %d\n", ret);
		return ret;
	}
	/* DictionariesProvided */
	element.dataType = RSSL_DT_ARRAY;
	element.name = RSSL_ENAME_DICTIONARYS_PROVIDED;
	if ((ret = rsslEncodeElementEntryInit(eIter, &element, 0)) < RSSL_RET_SUCCESS)
	{
		printf("rsslEncodeElementEntryInit() failed with return code: %d\n", ret);
		return ret;
	}
	rsslClearArray(&rsslArray);
	rsslArray.itemLength = 0;
	rsslArray.primitiveType = RSSL_DT_ASCII_STRING;
	if ((ret = rsslEncodeArrayInit(eIter, &rsslArray)) < RSSL_RET_SUCCESS)
	{
		printf("rsslEncodeArrayInit() failed with return code: %d\n", ret);
		return ret;
	}
	tempBuffer.data = (char*)serviceGeneralInfo->DictionariesProvided[0];
	tempBuffer.length = (RsslUInt32)strlen(serviceGeneralInfo->DictionariesProvided[0]);
	if ((ret = rsslEncodeArrayEntry(eIter, 0, &tempBuffer)) < RSSL_RET_SUCCESS)
	{
		printf("rsslEncodeArrayEntry() failed with return code: %d\n", ret);
		return ret;
	}
	tempBuffer.data = (char*)serviceGeneralInfo->DictionariesProvided[1];
	tempBuffer.length = (RsslUInt32)strlen(serviceGeneralInfo->DictionariesProvided[1]);
	if ((ret = rsslEncodeArrayEntry(eIter, 0, &tempBuffer)) < RSSL_RET_SUCCESS)
	{
		printf("rsslEncodeArrayEntry() failed with return code: %d\n", ret);
		return ret;
	}
	if ((ret = rsslEncodeArrayComplete(eIter, RSSL_TRUE)) < RSSL_RET_SUCCESS)
	{
		printf("rsslEncodeArrayComplete() failed with return code: %d\n", ret);
		return ret;
	}
	if ((ret = rsslEncodeElementEntryComplete(eIter, RSSL_TRUE)) < RSSL_RET_SUCCESS)
	{
		printf("rsslEncodeElementEntryComplete() failed with return code: %d\n", ret);
		return ret;
	}
	/* DictionariesUsed */
	element.dataType = RSSL_DT_ARRAY;
	element.name = RSSL_ENAME_DICTIONARYS_USED;
	if ((ret = rsslEncodeElementEntryInit(eIter, &element, 0)) < RSSL_RET_SUCCESS)
	{
		printf("rsslEncodeElementEntryInit() failed with return code: %d\n", ret);
		return ret;
	}
	rsslClearArray(&rsslArray);
	rsslArray.itemLength = 0;
	rsslArray.primitiveType = RSSL_DT_ASCII_STRING;
	if ((ret = rsslEncodeArrayInit(eIter, &rsslArray)) < RSSL_RET_SUCCESS)
	{
		printf("rsslEncodeArrayInit() failed with return code: %d\n", ret);
		return ret;
	}
	tempBuffer.data = (char*)serviceGeneralInfo->DictionariesUsed[0];
	tempBuffer.length = (RsslUInt32)strlen(serviceGeneralInfo->DictionariesUsed[0]);
	if ((ret = rsslEncodeArrayEntry(eIter, 0, &tempBuffer)) < RSSL_RET_SUCCESS)
	{
		printf("rsslEncodeArrayEntry() failed with return code: %d\n", ret);
		return ret;
	}
	tempBuffer.data = (char*)serviceGeneralInfo->DictionariesUsed[1];
	tempBuffer.length = (RsslUInt32)strlen(serviceGeneralInfo->DictionariesUsed[1]);
	if ((ret = rsslEncodeArrayEntry(eIter, 0, &tempBuffer)) < RSSL_RET_SUCCESS)
	{
		printf("rsslEncodeArrayEntry() failed with return code: %d\n", ret);
		return ret;
	}
	if ((ret = rsslEncodeArrayComplete(eIter, RSSL_TRUE)) < RSSL_RET_SUCCESS)
	{
		printf("rsslEncodeArrayComplete() failed with return code: %d\n", ret);
		return ret;
	}
	if ((ret = rsslEncodeElementEntryComplete(eIter, RSSL_TRUE)) < RSSL_RET_SUCCESS)
	{
		printf("rsslEncodeElementEntryComplete() failed with return code: %d\n", ret);
		return ret;
	}
	/* QoS */
	element.dataType = RSSL_DT_ARRAY;
	element.name = RSSL_ENAME_QOS;
	if ((ret = rsslEncodeElementEntryInit(eIter, &element, 0)) < RSSL_RET_SUCCESS)
	{
		printf("rsslEncodeElementEntryInit() failed with return code: %d\n", ret);
		return ret;
	}
	rsslClearArray(&rsslArray);
	rsslArray.itemLength = 0;
	rsslArray.primitiveType = RSSL_DT_QOS;
	if ((ret = rsslEncodeArrayInit(eIter, &rsslArray)) < RSSL_RET_SUCCESS)
	{
		printf("rsslEncodeArrayInit() failed with return code: %d\n", ret);
		return ret;
	}
	if ((ret = rsslEncodeArrayEntry(eIter, 0, &serviceGeneralInfo->QoS[0])) < RSSL_RET_SUCCESS)
	{
		printf("rsslEncodeArrayEntry() failed with return code: %d\n", ret);
		return ret;
	}
	if ((ret = rsslEncodeArrayComplete(eIter, RSSL_TRUE)) < RSSL_RET_SUCCESS)
	{
		printf("rsslEncodeArrayComplete() failed with return code: %d\n", ret);
		return ret;
	}
	if ((ret = rsslEncodeElementEntryComplete(eIter, RSSL_TRUE)) < RSSL_RET_SUCCESS)
	{
		printf("rsslEncodeElementEntryComplete() failed with return code: %d\n", ret);
		return ret;
	}
	/* SupportsQosRange */
	element.dataType = RSSL_DT_UINT;
	element.name = RSSL_ENAME_SUPPS_QOS_RANGE;
	if ((ret = rsslEncodeElementEntry(eIter, &element, &serviceGeneralInfo->SupportsQosRange)) < RSSL_RET_SUCCESS)
	{
		printf("rsslEncodeElementEntry() failed with return code: %d\n", ret);
		return ret;
	}
	/* ItemList */
	tempBuffer.data = (char*)serviceGeneralInfo->ItemList;
	tempBuffer.length = (RsslUInt32)strlen(serviceGeneralInfo->ItemList);
	element.dataType = RSSL_DT_ASCII_STRING;
	element.name = RSSL_ENAME_ITEM_LIST;
	if ((ret = rsslEncodeElementEntry(eIter, &element, &tempBuffer)) < RSSL_RET_SUCCESS)
	{
		printf("rsslEncodeElementEntry() failed with return code: %d\n", ret);
		return ret;
	}
	/* SupportsOutOfBandSnapshots */
	element.dataType = RSSL_DT_UINT;
	element.name = RSSL_ENAME_SUPPS_OOB_SNAPSHOTS;
	if ((ret = rsslEncodeElementEntry(eIter, &element, &serviceGeneralInfo->SupportsOutOfBandSnapshots)) < RSSL_RET_SUCCESS)
	{
		printf("rsslEncodeElementEntry() failed with return code: %d\n", ret);
		return ret;
	}
	/* AcceptingConsumerStatus */
	element.dataType = RSSL_DT_UINT;
	element.name = RSSL_ENAME_ACCEPTING_CONS_STATUS;
	if ((ret = rsslEncodeElementEntry(eIter, &element, &serviceGeneralInfo->AcceptingConsumerStatus)) < RSSL_RET_SUCCESS)
	{
		printf("rsslEncodeElementEntry() failed with return code: %d\n", ret);
		return ret;
	}

	/* complete encode element list */
	if ((ret = rsslEncodeElementListComplete(eIter, RSSL_TRUE)) < RSSL_RET_SUCCESS)
	{
		printf("rsslEncodeElementListComplete() failed with return code: %d\n", ret);
		return ret;
	}

	/* complete encode filter list item */
	if ((ret = rsslEncodeFilterEntryComplete(eIter, RSSL_TRUE)) < RSSL_RET_SUCCESS)
	{
		printf("rsslEncodeFilterEntryComplete() failed with return code: %d\n", ret);
		return ret;
	}

	return RSSL_RET_SUCCESS;
}

/*
 * Encodes the service's state information.  Returns success if encoding
 * succeeds or failure if encoding fails.
 * serviceStateInfo - The service's state information to be encoded
 * eIter - The encode iterator
 */
RsslRet encodeServiceStateInfo(RsslServiceStateInfo* serviceStateInfo, RsslEncodeIterator* eIter)
{
	RsslRet ret = 0;
	RsslFilterEntry filterListItem = RSSL_INIT_FILTER_ENTRY;
	RsslElementEntry element = RSSL_INIT_ELEMENT_ENTRY;
	RsslElementList	elementList = RSSL_INIT_ELEMENT_LIST;

	/* encode filter list item */
	filterListItem.id = RDM_DIRECTORY_SERVICE_STATE_ID;
	filterListItem.action = RSSL_FTEA_SET_ENTRY;
	if ((ret = rsslEncodeFilterEntryInit(eIter, &filterListItem, 0)) < RSSL_RET_SUCCESS)
	{
		printf("rsslEncodeFilterEntryInit() failed with return code: %d\n", ret);
		return ret;
	}

	/* encode the element list */
	elementList.flags = RSSL_ELF_HAS_STANDARD_DATA;
	if ((ret = rsslEncodeElementListInit(eIter, &elementList, 0, 0)) < RSSL_RET_SUCCESS)
	{
		printf("rsslEncodeElementListInit() failed with return code: %d\n", ret);
		return ret;
	}
	/* ServiceState */
	element.dataType = RSSL_DT_UINT;
	element.name = RSSL_ENAME_SVC_STATE;
	if ((ret = rsslEncodeElementEntry(eIter, &element, &serviceStateInfo->ServiceState)) < RSSL_RET_SUCCESS)
	{
		printf("rsslEncodeElementEntry() failed with return code: %d\n", ret);
		return ret;
	}
	/* AcceptingRequests */
	element.dataType = RSSL_DT_UINT;
	element.name = RSSL_ENAME_ACCEPTING_REQS;
	if ((ret = rsslEncodeElementEntry(eIter, &element, &serviceStateInfo->AcceptingRequests)) < RSSL_RET_SUCCESS)
	{
		printf("rsslEncodeElementEntry() failed with return code: %d\n", ret);
		return ret;
	}
	/* Status */
	element.dataType = RSSL_DT_STATE;
	element.name = RSSL_ENAME_STATUS;
	if ((ret = rsslEncodeElementEntry(eIter, &element, &serviceStateInfo->Status)) < RSSL_RET_SUCCESS)
	{
		printf("rsslEncodeElementEntry() failed with return code: %d\n", ret);
		return ret;
	}

	/* complete encode element list */
	if ((ret = rsslEncodeElementListComplete(eIter, RSSL_TRUE)) < RSSL_RET_SUCCESS)
	{
		printf("rsslEncodeElementListComplete() failed with return code: %d\n", ret);
		return ret;
	}

	/* complete encode filter list item */
	if ((ret = rsslEncodeFilterEntryComplete(eIter, RSSL_TRUE)) < RSSL_RET_SUCCESS)
	{
		printf("rsslEncodeFilterEntryComplete() failed with return code: %d\n", ret);
		return ret;
	}

	return RSSL_RET_SUCCESS;
}

/*
 * Encodes the service's group information.  Returns success if encoding
 * succeeds or failure if encoding fails.
 * serviceGroupInfo - The service's group information to be encoded
 * eIter - The encode iterator
 */
RsslRet encodeServiceGroupInfo(RsslServiceGroupInfo* serviceGroupInfo, RsslEncodeIterator* eIter)
{
	RsslRet ret = 0;
	RsslFilterEntry filterListItem = RSSL_INIT_FILTER_ENTRY;
	RsslElementEntry element = RSSL_INIT_ELEMENT_ENTRY;
	RsslElementList	elementList = RSSL_INIT_ELEMENT_LIST;
	RsslBuffer tempBuffer;

	/* encode filter list item */
	filterListItem.id = RDM_DIRECTORY_SERVICE_GROUP_ID;
	filterListItem.action = RSSL_FTEA_SET_ENTRY;
	if ((ret = rsslEncodeFilterEntryInit(eIter, &filterListItem, 0)) < RSSL_RET_SUCCESS)
	{
		printf("rsslEncodeFilterEntryInit() failed with return code: %d\n", ret);
		return ret;
	}

	/* encode the element list */
	elementList.flags = RSSL_ELF_HAS_STANDARD_DATA;
	if ((ret = rsslEncodeElementListInit(eIter, &elementList, 0, 0)) < RSSL_RET_SUCCESS)
	{
		printf("rsslEncodeElementListInit() failed with return code: %d\n", ret);
		return ret;
	}
	
	/* Group */
	element.dataType = RSSL_DT_BUFFER;
	element.name = RSSL_ENAME_GROUP;
	tempBuffer.data = (char *)&serviceGroupInfo->Group;
	tempBuffer.length = sizeof(RsslUInt16);
	if ((ret = rsslEncodeElementEntry(eIter, &element, &tempBuffer)) < RSSL_RET_SUCCESS)
	{
		printf("rsslEncodeElementEntry() failed with return code: %d\n", ret);
		return ret;
	}
	/* MergedToGroup */
	element.dataType = RSSL_DT_BUFFER;
	element.name = RSSL_ENAME_MERG_TO_GRP;
	tempBuffer.data = (char *)&serviceGroupInfo->MergedToGroup;
	tempBuffer.length = sizeof(RsslUInt16);
	if ((ret = rsslEncodeElementEntry(eIter, &element, &tempBuffer)) < RSSL_RET_SUCCESS)
	{
		printf("rsslEncodeElementEntry() failed with return code: %d\n", ret);
		return ret;
	}

	/* Status */
	element.dataType = RSSL_DT_STATE;
	element.name = RSSL_ENAME_STATUS;
	if ((ret = rsslEncodeElementEntry(eIter, &element, &serviceGroupInfo->Status)) < RSSL_RET_SUCCESS)
	{
		printf("rsslEncodeElementEntry() failed with return code: %d\n", ret);
		return ret;
	}

	/* complete encode element list */
	if ((ret = rsslEncodeElementListComplete(eIter, RSSL_TRUE)) < RSSL_RET_SUCCESS)
	{
		printf("rsslEncodeElementListComplete() failed with return code: %d\n", ret);
		return ret;
	}

	/* complete encode filter list item */
	if ((ret = rsslEncodeFilterEntryComplete(eIter, RSSL_TRUE)) < RSSL_RET_SUCCESS)
	{
		printf("rsslEncodeFilterEntryComplete() failed with return code: %d\n", ret);
		return ret;
	}

	return RSSL_RET_SUCCESS;
}

/*
 * Encodes the service's load information.  Returns success if encoding
 * succeeds or failure if encoding fails.
 * serviceLoadInfo - The service's load information to be encoded
 * eIter - The encode iterator
 */
RsslRet encodeServiceLoadInfo(RsslServiceLoadInfo* serviceLoadInfo, RsslEncodeIterator* eIter)
{
	RsslRet ret = 0;
	RsslFilterEntry filterListItem = RSSL_INIT_FILTER_ENTRY;
	RsslElementEntry element = RSSL_INIT_ELEMENT_ENTRY;
	RsslElementList	elementList = RSSL_INIT_ELEMENT_LIST;

	/* encode filter list item */
	filterListItem.id = RDM_DIRECTORY_SERVICE_LOAD_ID;
	filterListItem.action = RSSL_FTEA_SET_ENTRY;
	if ((ret = rsslEncodeFilterEntryInit(eIter, &filterListItem, 0)) < RSSL_RET_SUCCESS)
	{
		printf("rsslEncodeFilterEntryInit() failed with return code: %d\n", ret);
		return ret;
	}

	/* encode the element list */
	elementList.flags = RSSL_ELF_HAS_STANDARD_DATA;
	if ((ret = rsslEncodeElementListInit(eIter, &elementList, 0, 0)) < RSSL_RET_SUCCESS)
	{
		printf("rsslEncodeElementListInit() failed with return code: %d\n", ret);
		return ret;
	}
	/* OpenLimit */
	element.dataType = RSSL_DT_UINT;
	element.name = RSSL_ENAME_OPEN_LIMIT;
	if ((ret = rsslEncodeElementEntry(eIter, &element, &serviceLoadInfo->OpenLimit)) < RSSL_RET_SUCCESS)
	{
		printf("rsslEncodeElementEntry() failed with return code: %d\n", ret);
		return ret;
	}

	if (serviceLoadInfo->OpenWindow)
	{
		/* OpenWindow */
		element.dataType = RSSL_DT_UINT;
		element.name = RSSL_ENAME_OPEN_WINDOW;
		if ((ret = rsslEncodeElementEntry(eIter, &element, &serviceLoadInfo->OpenWindow)) < RSSL_RET_SUCCESS)
		{
			printf("rsslEncodeElementEntry() failed with return code: %d\n", ret);
			return ret;
		}
	}

	/* LoadFactor */
	element.dataType = RSSL_DT_UINT;
	element.name = RSSL_ENAME_LOAD_FACT;
	if ((ret = rsslEncodeElementEntry(eIter, &element, &serviceLoadInfo->LoadFactor)) < RSSL_RET_SUCCESS)
	{
		printf("rsslEncodeElementEntry() failed with return code: %d\n", ret);
		return ret;
	}

	/* complete encode element list */
	if ((ret = rsslEncodeElementListComplete(eIter, RSSL_TRUE)) < RSSL_RET_SUCCESS)
	{
		printf("rsslEncodeElementListComplete() failed with return code: %d\n", ret);
		return ret;
	}

	/* complete encode filter list item */
	if ((ret = rsslEncodeFilterEntryComplete(eIter, RSSL_TRUE)) < RSSL_RET_SUCCESS)
	{
		printf("rsslEncodeFilterEntryComplete() failed with return code: %d\n", ret);
		return ret;
	}

	return RSSL_RET_SUCCESS;
}

/*
 * Encodes the service's data information.  Returns success if encoding
 * succeeds or failure if encoding fails.
 * serviceDataInfo - The service's data information to be encoded
 * eIter - The encode iterator
 */
RsslRet encodeServiceDataInfo(RsslServiceDataInfo* serviceDataInfo, RsslEncodeIterator* eIter)
{
	RsslRet ret = 0;
	RsslFilterEntry filterListItem = RSSL_INIT_FILTER_ENTRY;
	RsslElementEntry element = RSSL_INIT_ELEMENT_ENTRY;
	RsslElementList	elementList = RSSL_INIT_ELEMENT_LIST;

	/* encode filter list item */
	filterListItem.id = RDM_DIRECTORY_SERVICE_DATA_ID;
	filterListItem.action = RSSL_FTEA_SET_ENTRY;
	if ((ret = rsslEncodeFilterEntryInit(eIter, &filterListItem, 0)) < RSSL_RET_SUCCESS)
	{
		printf("rsslEncodeFilterEntryInit() failed with return code: %d\n", ret);
		return ret;
	}

	/* encode the element list */
	elementList.flags = RSSL_ELF_HAS_STANDARD_DATA;
	if ((ret = rsslEncodeElementListInit(eIter, &elementList, 0, 0)) < RSSL_RET_SUCCESS)
	{
		printf("rsslEncodeElementListInit() failed with return code: %d\n", ret);
		return ret;
	}
	/* Type */
	element.dataType = RSSL_DT_UINT;
	element.name = RSSL_ENAME_TYPE;
	if ((ret = rsslEncodeElementEntry(eIter, &element, &serviceDataInfo->Type)) < RSSL_RET_SUCCESS)
	{
		printf("rsslEncodeElementEntry() failed with return code: %d\n", ret);
		return ret;
	}
	/* Data */
	element.dataType = RSSL_DT_UINT;
	element.name = RSSL_ENAME_DATA;
	if ((ret = rsslEncodeElementEntry(eIter, &element, &serviceDataInfo->Data)) < RSSL_RET_SUCCESS)
	{
		printf("rsslEncodeElementEntry() failed with return code: %d\n", ret);
		return ret;
	}

	/* complete encode element list */
	if ((ret = rsslEncodeElementListComplete(eIter, RSSL_TRUE)) < RSSL_RET_SUCCESS)
	{
		printf("rsslEncodeElementListComplete() failed with return code: %d\n", ret);
		return ret;
	}

	/* complete encode filter list item */
	if ((ret = rsslEncodeFilterEntryComplete(eIter, RSSL_TRUE)) < RSSL_RET_SUCCESS)
	{
		printf("rsslEncodeFilterEntryComplete() failed with return code: %d\n", ret);
		return ret;
	}

	return RSSL_RET_SUCCESS;
}

/*
 * Encodes the service's link information.  Returns success if encoding
 * succeeds or failure if encoding fails.
 * serviceLinkInfo - The service's link information to be encoded
 * eIter - The encode iterator
 */
RsslRet encodeServiceLinkInfo(RsslServiceLinkInfo* serviceLinkInfo, RsslEncodeIterator* eIter)
{
	RsslRet ret = 0;
	RsslFilterEntry filterListItem = RSSL_INIT_FILTER_ENTRY;
	RsslMap map = RSSL_INIT_MAP;
	RsslMapEntry mEntry = RSSL_INIT_MAP_ENTRY;
	RsslElementEntry element = RSSL_INIT_ELEMENT_ENTRY;
	RsslElementList	elementList = RSSL_INIT_ELEMENT_LIST;
	RsslBuffer tempBuffer;

	/* encode filter list item */
	filterListItem.id = RDM_DIRECTORY_SERVICE_LINK_ID;
	filterListItem.action = RSSL_FTEA_SET_ENTRY;
	filterListItem.flags = RSSL_FTEF_HAS_CONTAINER_TYPE;
	filterListItem.containerType = RSSL_DT_MAP;
	if ((ret = rsslEncodeFilterEntryInit(eIter, &filterListItem, 0)) < RSSL_RET_SUCCESS)
	{
		printf("rsslEncodeFilterEntryInit() failed with return code: %d\n", ret);
		return ret;
	}

	/* encode map */
	map.keyPrimitiveType = RSSL_DT_ASCII_STRING;
	map.containerType = RSSL_DT_ELEMENT_LIST;
	if ((ret = rsslEncodeMapInit(eIter, &map, 0, 0)) < RSSL_RET_SUCCESS)
	{
		printf("rsslEncodeMapInit() failed with return code: %d\n", ret);
		return ret;
	}

	/* encode map entry */
	mEntry.action = RSSL_MPEA_ADD_ENTRY;
	tempBuffer.data = (char*)serviceLinkInfo->LinkName;
	tempBuffer.length = (RsslUInt32)strlen(serviceLinkInfo->LinkName);
	if ((ret = rsslEncodeMapEntryInit(eIter, &mEntry, &tempBuffer, 0)) < RSSL_RET_SUCCESS)
	{
		printf("rsslEncodeMapEntry() failed with return code: %d\n", ret);
		return ret;
	}

	/* encode the element list */
	elementList.flags = RSSL_ELF_HAS_STANDARD_DATA;
	if ((ret = rsslEncodeElementListInit(eIter, &elementList, 0, 0)) < RSSL_RET_SUCCESS)
	{
		printf("rsslEncodeElementListInit() failed with return code: %d\n", ret);
		return ret;
	}
	/* Type */
	element.dataType = RSSL_DT_UINT;
	element.name = RSSL_ENAME_TYPE;
	if ((ret = rsslEncodeElementEntry(eIter, &element, &serviceLinkInfo->Type)) < RSSL_RET_SUCCESS)
	{
		printf("rsslEncodeElementEntry() failed with return code: %d\n", ret);
		return ret;
	}
	/* LinkState */
	element.dataType = RSSL_DT_UINT;
	element.name = RSSL_ENAME_LINK_STATE;
	if ((ret = rsslEncodeElementEntry(eIter, &element, &serviceLinkInfo->LinkState)) < RSSL_RET_SUCCESS)
	{
		printf("rsslEncodeElementEntry() failed with return code: %d\n", ret);
		return ret;
	}
	/* LinkCode */
	element.dataType = RSSL_DT_UINT;
	element.name = RSSL_ENAME_LINK_CODE;
	if ((ret = rsslEncodeElementEntry(eIter, &element, &serviceLinkInfo->LinkCode)) < RSSL_RET_SUCCESS)
	{
		printf("rsslEncodeElementEntry() failed with return code: %d\n", ret);
		return ret;
	}
	/* Text */
	element.dataType = RSSL_DT_ASCII_STRING;
	element.name = RSSL_ENAME_TEXT;
	tempBuffer.data = (char*)serviceLinkInfo->Text;
	tempBuffer.length = (RsslUInt32)strlen(serviceLinkInfo->Text);
	if ((ret = rsslEncodeElementEntry(eIter, &element, &tempBuffer)) < RSSL_RET_SUCCESS)
	{
		printf("rsslEncodeElementEntry() failed with return code: %d\n", ret);
		return ret;
	}

	/* complete encode element list */
	if ((ret = rsslEncodeElementListComplete(eIter, RSSL_TRUE)) < RSSL_RET_SUCCESS)
	{
		printf("rsslEncodeElementListComplete() failed with return code: %d\n", ret);
		return ret;
	}

	/* complete encode map entry */
	if ((ret = rsslEncodeMapEntryComplete(eIter, RSSL_TRUE)) < RSSL_RET_SUCCESS)
	{
		printf("rsslEncodeMapEntryComplete() failed with return code: %d\n", ret);
		return ret;
	}

	/* complete encode map */
	if ((ret = rsslEncodeMapComplete(eIter, RSSL_TRUE)) < RSSL_RET_SUCCESS)
	{
		printf("rsslEncodeMapComplete() failed with return code: %d\n", ret);
		return ret;
	}

	/* complete encode filter list item */
	if ((ret = rsslEncodeFilterEntryComplete(eIter, RSSL_TRUE)) < RSSL_RET_SUCCESS)
	{
		printf("rsslEncodeFilterEntryComplete() failed with return code: %d\n", ret);
		return ret;
	}

	return RSSL_RET_SUCCESS;
}

/*
 * Decodes the source directory response into the RsslSourceDirectoryResponseInfo
 * structure.  Returns success if decoding succeeds or failure if decoding fails.
 * srcDirRespInfo - The source directory response information structure
 * dIter - The decode iterator
 * maxServices - The maximum number of services that the structure holds
 * maxCapabilities - The maximum number of capabilities that the structure holds
 * maxQOS - The maximum number of QOS entries that the structure holds
 * maxDictionaries - The maximum number of dictionaries that the structure holds
 * maxLinks - The maximum number of link entries that the structure holds
 */
RsslRet decodeSourceDirectoryResponse(RsslSourceDirectoryResponseInfo* srcDirRespInfo,
										  RsslDecodeIterator* dIter,
										  RsslUInt32 maxServices,
										  RsslUInt32 maxCapabilities,
										  RsslUInt32 maxQOS,
										  RsslUInt32 maxDictionaries,
										  RsslUInt32 maxLinks)
{
	RsslRet ret = 0;
	RsslMap map;
	RsslMapEntry mEntry;
	RsslFilterList rsslFilterList;
	RsslFilterEntry filterListItem;
	int serviceCount = 0;
	RsslUInt64 serviceIdTemp;

	/* decode source directory response */

	/* decode map */
	if ((ret = rsslDecodeMap(dIter, &map)) < RSSL_RET_SUCCESS)
	{
		printf("rsslDecodeMap() failed with return code: %d\n", ret);
		return ret;
	}

	/* source directory response key data type must be RSSL_DT_UINT */
	if (map.keyPrimitiveType != RSSL_DT_UINT)
	{
		printf("Map has incorrect keyPrimitiveType: %s", rsslDataTypeToString(map.keyPrimitiveType));
		return RSSL_RET_FAILURE;
	}

	/* decode map entries */
	/* service id is contained in map entry encKey */
	/* store service id in source directory response information */
	while ((ret = rsslDecodeMapEntry(dIter, &mEntry, &serviceIdTemp)) != RSSL_RET_END_OF_CONTAINER)
	{
		/* break out of decoding when max services reached */
		if (serviceCount == maxServices)
		{
			rsslFinishDecodeEntries(dIter);
			printf("decodeSourceDirectoryResponse() maxServices limit reached - more services in message than memory can support\n");
			break;
		}

		if (ret == RSSL_RET_SUCCESS)
		{
			if (ret != RSSL_RET_SUCCESS && ret != RSSL_RET_BLANK_DATA)
			{
				printf("rsslDecodeUInt() failed with return code: %d\n", ret);
				return ret;
			}

			srcDirRespInfo[serviceCount].ServiceId = serviceIdTemp;

			/* decode filter list */
			if ((ret = rsslDecodeFilterList(dIter, &rsslFilterList)) < RSSL_RET_SUCCESS)
			{
				printf("rsslDecodeFilterList() failed with return code: %d\n", ret);
				return ret;
			}

			/* decode filter list items */
			while ((ret = rsslDecodeFilterEntry(dIter, &filterListItem)) != RSSL_RET_END_OF_CONTAINER)
			{
				if (ret == RSSL_RET_SUCCESS)
				{
					/* decode source directory response information */
					switch (filterListItem.id)
					{
					case RDM_DIRECTORY_SERVICE_INFO_ID:
						if ((ret = decodeServiceGeneralInfo(&srcDirRespInfo[serviceCount].ServiceGeneralInfo, dIter, maxCapabilities, maxQOS, maxDictionaries)) != RSSL_RET_SUCCESS)
						{
							printf("decodeServiceGeneralInfo() failed with return code: %d\n", ret);
							return ret;
						}
						break;
					case RDM_DIRECTORY_SERVICE_STATE_ID:
						if ((ret = decodeServiceStateInfo(&srcDirRespInfo[serviceCount].ServiceStateInfo, dIter)) != RSSL_RET_SUCCESS)
						{
							printf("decodeServiceStateInfo() failed with return code: %d\n", ret);
							return ret;
						}
						break;
					case RDM_DIRECTORY_SERVICE_GROUP_ID:
						if ((ret = decodeServiceGroupInfo(&srcDirRespInfo[serviceCount].ServiceGroupInfo, dIter)) != RSSL_RET_SUCCESS)
						{
							printf("decodeServiceGroupInfo() failed with return code: %d\n", ret);
							return ret;
						}
						break;
					case RDM_DIRECTORY_SERVICE_LOAD_ID:
						if ((ret = decodeServiceLoadInfo(&srcDirRespInfo[serviceCount].ServiceLoadInfo, dIter)) != RSSL_RET_SUCCESS)
						{
							printf("decodeServiceLoadInfo() failed with return code: %d\n", ret);
							return ret;
						}
						break;
					case RDM_DIRECTORY_SERVICE_DATA_ID:
						if ((ret = decodeServiceDataInfo(&srcDirRespInfo[serviceCount].ServiceDataInfo, dIter)) != RSSL_RET_SUCCESS)
						{
							printf("decodeServiceDataInfo() failed with return code: %d\n", ret);
							return ret;
						}
						break;
					case RDM_DIRECTORY_SERVICE_LINK_ID:
						if ((ret = decodeServiceLinkInfo(&srcDirRespInfo[serviceCount].ServiceLinkInfo[0], dIter, maxLinks)) != RSSL_RET_SUCCESS)
						{
							printf("decodeServiceLinkInfo() failed with return code: %d\n", ret);
							return ret;
						}
						break;
					default:
						printf("Unkonwn FilterListEntry filterID: %d\n", filterListItem.id);
						return RSSL_RET_FAILURE;
					}
				}
			}
		}
		serviceCount++;
	}

	return RSSL_RET_SUCCESS;
}

/*
 * Decodes the service's general information into the RsslServiceGeneralInfo structure.
 * Returns success if decoding succeeds or failure if decoding fails.
 * serviceGeneralInfo - The service's general information structure
 * dIter - The decode iterator
 * maxCapabilities - The maximum number of capabilities that the structure holds
 * maxQOS - The maximum number of QOS entries that the structure holds
 * maxDictionaries - The maximum number of dictionaries that the structure holds
 */
RsslRet decodeServiceGeneralInfo(RsslServiceGeneralInfo* serviceGeneralInfo,
										RsslDecodeIterator* dIter,
										RsslUInt32 maxCapabilities,
										RsslUInt32 maxQOS,
										RsslUInt32 maxDictionaries)
{
	RsslRet ret = 0;
	RsslElementList	elementList;
	RsslElementEntry element;
	RsslArray rsslArray = RSSL_INIT_ARRAY;
	RsslBuffer arrayBuffer;
	RsslUInt32 arrayCount = 0;
	RsslBool foundQoS = RSSL_FALSE;

	/* decode element list */
	if ((ret = rsslDecodeElementList(dIter, &elementList, NULL)) < RSSL_RET_SUCCESS)
	{
		printf("rsslDecodeElementList() failed with return code: %d\n", ret);
		return ret;
	}

	/* decode element list elements */
	while ((ret = rsslDecodeElementEntry(dIter, &element)) != RSSL_RET_END_OF_CONTAINER)
	{
		if (ret == RSSL_RET_SUCCESS)
		{
			/* get service general information */
			/* Name */
			if (rsslBufferIsEqual(&element.name, &RSSL_ENAME_NAME))
			{
				/* store service name in source directory response information */
				if (element.encData.length < MAX_SRCDIR_INFO_STRLEN)
				{
					strncpy(serviceGeneralInfo->ServiceName, element.encData.data, element.encData.length);
					serviceGeneralInfo->ServiceName[element.encData.length] = '\0';
				}
				else
				{
					strncpy(serviceGeneralInfo->ServiceName, element.encData.data, MAX_SRCDIR_INFO_STRLEN - 1);
					serviceGeneralInfo->ServiceName[MAX_SRCDIR_INFO_STRLEN - 1] = '\0';
				}
			}
			/* Vendor */
			else if (rsslBufferIsEqual(&element.name, &RSSL_ENAME_VENDOR))
			{
				if (element.encData.length < MAX_SRCDIR_INFO_STRLEN)
				{
					strncpy(serviceGeneralInfo->Vendor, element.encData.data, element.encData.length);
					serviceGeneralInfo->Vendor[element.encData.length] = '\0';
				}
				else
				{
					strncpy(serviceGeneralInfo->Vendor, element.encData.data, MAX_SRCDIR_INFO_STRLEN - 1);
					serviceGeneralInfo->Vendor[MAX_SRCDIR_INFO_STRLEN - 1] = '\0';
				}
			}
			/* IsSource */
			else if (rsslBufferIsEqual(&element.name, &RSSL_ENAME_IS_SOURCE))
			{
				ret = rsslDecodeUInt(dIter, &serviceGeneralInfo->IsSource);
				if (ret != RSSL_RET_SUCCESS && ret != RSSL_RET_BLANK_DATA)
				{
					printf("rsslDecodeUInt() failed with return code: %d\n", ret);
					return ret;
				}
			}
			/* Capabilities */
			else if (rsslBufferIsEqual(&element.name, &RSSL_ENAME_CAPABILITIES))
			{
				if ((ret = rsslDecodeArray(dIter, &rsslArray)) < RSSL_RET_SUCCESS)
				{
					printf("rsslDecodeArray() failed with return code: %d\n", ret);
					return ret;
				}
				while ((ret = rsslDecodeArrayEntry(dIter, &arrayBuffer)) != RSSL_RET_END_OF_CONTAINER)
				{
					/* break out of decoding array items when max capabilities reached */
					if (arrayCount == maxCapabilities)
					{
						rsslFinishDecodeEntries(dIter);
						break;
					}

					if (ret == RSSL_RET_SUCCESS)
					{
						ret = rsslDecodeUInt(dIter, &serviceGeneralInfo->Capabilities[arrayCount]);
						if (ret != RSSL_RET_SUCCESS && ret != RSSL_RET_BLANK_DATA)
						{
							printf("rsslDecodeUInt() failed with return code: %d\n", ret);
							return ret;
						}
					}
					else if (ret != RSSL_RET_BLANK_DATA)
					{
						printf("rsslDecodeArrayEntry() failed with return code: %d\n", ret);
						return ret;
					}
					arrayCount++;
				}
				arrayCount = 0;
			}
			/* DictionariesProvided */
			else if (rsslBufferIsEqual(&element.name, &RSSL_ENAME_DICTIONARYS_PROVIDED))
			{
				if ((ret = rsslDecodeArray(dIter, &rsslArray)) < RSSL_RET_SUCCESS)
				{
					printf("rsslDecodeArray() failed with return code: %d\n", ret);
					return ret;
				}
				while ((ret = rsslDecodeArrayEntry(dIter, &arrayBuffer)) != RSSL_RET_END_OF_CONTAINER)
				{
					/* break out of decoding array items when max dictionaries reached */
					if (arrayCount == maxDictionaries)
					{
						rsslFinishDecodeEntries(dIter);
						break;
					}

					if (ret == RSSL_RET_SUCCESS)
					{
						if (arrayBuffer.length < MAX_SRCDIR_INFO_STRLEN)
						{
							strncpy(serviceGeneralInfo->DictionariesProvided[arrayCount],
									arrayBuffer.data,
									arrayBuffer.length);
							serviceGeneralInfo->DictionariesProvided[arrayCount][arrayBuffer.length] = '\0';
						}
						else
						{
							strncpy(serviceGeneralInfo->DictionariesProvided[arrayCount],
									arrayBuffer.data,
									MAX_SRCDIR_INFO_STRLEN - 1);
							serviceGeneralInfo->DictionariesProvided[arrayCount][MAX_SRCDIR_INFO_STRLEN - 1] = '\0';
						}
					}
					else if (ret != RSSL_RET_BLANK_DATA)
					{
						printf("rsslDecodeArrayEntry() failed with return code: %d\n", ret);
						return ret;
					}
					arrayCount++;
				}
				arrayCount = 0;
			}
			/* DictionariesUsed */
			else if (rsslBufferIsEqual(&element.name, &RSSL_ENAME_DICTIONARYS_USED))
			{
				if ((ret = rsslDecodeArray(dIter, &rsslArray)) < RSSL_RET_SUCCESS)
				{
					printf("rsslDecodeArray() failed with return code: %d\n", ret);
					return ret;
				}
				while ((ret = rsslDecodeArrayEntry(dIter, &arrayBuffer)) != RSSL_RET_END_OF_CONTAINER)
				{
					/* break out of decoding array items when max dictionaries reached */
					if (arrayCount == maxDictionaries)
					{
						rsslFinishDecodeEntries(dIter);
						break;
					}

					if (ret == RSSL_RET_SUCCESS)
					{
						if (arrayBuffer.length < MAX_SRCDIR_INFO_STRLEN)
						{
							strncpy(serviceGeneralInfo->DictionariesUsed[arrayCount],
									arrayBuffer.data,
									arrayBuffer.length);
							serviceGeneralInfo->DictionariesUsed[arrayCount][arrayBuffer.length] = '\0';
						}
						else
						{
							strncpy(serviceGeneralInfo->DictionariesUsed[arrayCount],
									arrayBuffer.data,
									MAX_SRCDIR_INFO_STRLEN - 1);
							serviceGeneralInfo->DictionariesUsed[arrayCount][MAX_SRCDIR_INFO_STRLEN - 1] = '\0';
						}
					}
					else if (ret != RSSL_RET_BLANK_DATA)
					{
						printf("rsslDecodeArrayEntry() failed with return code: %d\n", ret);
						return ret;
					}
					arrayCount++;
				}
				arrayCount = 0;
			}
			/* QoS */
			else if (rsslBufferIsEqual(&element.name, &RSSL_ENAME_QOS))
			{
				foundQoS = RSSL_TRUE;
				if ((ret = rsslDecodeArray(dIter, &rsslArray)) < RSSL_RET_SUCCESS)
				{
					printf("rsslDecodeArray() failed with return code: %d\n", ret);
					return ret;
				}
				while ((ret = rsslDecodeArrayEntry(dIter, &arrayBuffer)) != RSSL_RET_END_OF_CONTAINER)
				{
					/* break out of decoding array items when max QOS reached */
					if (arrayCount == maxQOS)
					{
						rsslFinishDecodeEntries(dIter);
						break;
					}

					if (ret == RSSL_RET_SUCCESS)
					{
						ret = rsslDecodeQos(dIter, &serviceGeneralInfo->QoS[arrayCount]);
						if (ret != RSSL_RET_SUCCESS && ret != RSSL_RET_BLANK_DATA)
						{
							printf("rsslDecodeQos() failed with return code: %d\n", ret);
							return ret;
						}
					}
					else if (ret != RSSL_RET_BLANK_DATA)
					{
						printf("rsslDecodeArrayEntry() failed with return code: %d\n", ret);
						return ret;
					}
					arrayCount++;
				}
				arrayCount = 0;
			}
			/* SupportsQosRange */
			else if (rsslBufferIsEqual(&element.name, &RSSL_ENAME_SUPPS_QOS_RANGE))
			{
				ret = rsslDecodeUInt(dIter, &serviceGeneralInfo->SupportsQosRange);
				if (ret != RSSL_RET_SUCCESS && ret != RSSL_RET_BLANK_DATA)
				{
					printf("rsslDecodeUInt() failed with return code: %d\n", ret);
					return ret;
				}
			}
			/* ItemList */
			else if (rsslBufferIsEqual(&element.name, &RSSL_ENAME_ITEM_LIST))
			{
				if (element.encData.length < MAX_SRCDIR_INFO_STRLEN)
				{
					strncpy(serviceGeneralInfo->ItemList, element.encData.data, element.encData.length);
					serviceGeneralInfo->ItemList[element.encData.length] = '\0';
				}
				else
				{
					strncpy(serviceGeneralInfo->ItemList, element.encData.data, MAX_SRCDIR_INFO_STRLEN - 1);
					serviceGeneralInfo->ItemList[MAX_SRCDIR_INFO_STRLEN - 1] = '\0';
				}
			}
			/* SupportsOutOfBandSnapshots */
			else if (rsslBufferIsEqual(&element.name, &RSSL_ENAME_SUPPS_OOB_SNAPSHOTS))
			{
				ret = rsslDecodeUInt(dIter, &serviceGeneralInfo->SupportsOutOfBandSnapshots);
				if (ret != RSSL_RET_SUCCESS && ret != RSSL_RET_BLANK_DATA)
				{
					printf("rsslDecodeUInt() failed with return code: %d\n", ret);
					return ret;
				}
			}
			/* AcceptingConsumerStatus */
			else if (rsslBufferIsEqual(&element.name, &RSSL_ENAME_ACCEPTING_CONS_STATUS))
			{
				ret = rsslDecodeUInt(dIter, &serviceGeneralInfo->AcceptingConsumerStatus);
				if (ret != RSSL_RET_SUCCESS && ret != RSSL_RET_BLANK_DATA)
				{
					printf("rsslDecodeUInt() failed with return code: %d\n", ret);
					return ret;
				}
			}
		}
		else
		{
			printf("rsslDecodeElementEntry() failed with return code: %d\n", ret);
			return ret;
		}
	}

	/* if QoS was not send in the directory refresh message set it to the default values */
	if (!foundQoS)
		for (arrayCount = 0; arrayCount < maxQOS; arrayCount++)
		{
			serviceGeneralInfo->QoS[arrayCount].timeliness = RSSL_QOS_TIME_REALTIME;
			serviceGeneralInfo->QoS[arrayCount].rate = RSSL_QOS_RATE_TICK_BY_TICK;
		}
	
	return RSSL_RET_SUCCESS;
}

/*
 * Decodes the service's state information into the RsslServiceStateInfo structure.
 * Returns success if decoding succeeds or failure if decoding fails.
 * serviceStateInfo - The service's state information structure
 * dIter - The decode iterator
 */
RsslRet decodeServiceStateInfo(RsslServiceStateInfo* serviceStateInfo,
									  RsslDecodeIterator* dIter)
{
	RsslRet ret = 0;
	RsslElementList	elementList;
	RsslElementEntry element;

	/* decode element list */
	if ((ret = rsslDecodeElementList(dIter, &elementList, NULL)) < RSSL_RET_SUCCESS)
	{
		printf("rsslDecodeElementList() failed with return code: %d\n", ret);
		return ret;
	}

	/* decode element list elements */
	while ((ret = rsslDecodeElementEntry(dIter, &element)) != RSSL_RET_END_OF_CONTAINER)
	{
		if (ret == RSSL_RET_SUCCESS)
		{
			/* get service state information */
			/* ServiceState */
			if (rsslBufferIsEqual(&element.name, &RSSL_ENAME_SVC_STATE))
			{
				ret = rsslDecodeUInt(dIter, &serviceStateInfo->ServiceState);
				if (ret != RSSL_RET_SUCCESS && ret != RSSL_RET_BLANK_DATA)
				{
					printf("rsslDecodeUInt() failed with return code: %d\n", ret);
					return ret;
				}
			}
			/* AcceptingRequests */
			else if (rsslBufferIsEqual(&element.name, &RSSL_ENAME_ACCEPTING_REQS))
			{
				ret = rsslDecodeUInt(dIter, &serviceStateInfo->AcceptingRequests);
				if (ret != RSSL_RET_SUCCESS && ret != RSSL_RET_BLANK_DATA)
				{
					printf("rsslDecodeUInt() failed with return code: %d\n", ret);
					return ret;
				}
			}
			/* Status */
			else if (rsslBufferIsEqual(&element.name, &RSSL_ENAME_STATUS))
			{
				ret = rsslDecodeState(dIter, &serviceStateInfo->Status);
				if (ret != RSSL_RET_SUCCESS && ret != RSSL_RET_BLANK_DATA)
				{
					printf("rsslDecodeUInt() failed with return code: %d\n", ret);
					return ret;
				}
			}
		}
		else
		{
			printf("rsslDecodeElementEntry() failed with return code: %d\n", ret);
			return ret;
		}
	}

	return RSSL_RET_SUCCESS;
}

/*
 * Decodes the service's group information into the RsslServiceGroupInfo structure.
 * Returns success if decoding succeeds or failure if decoding fails.
 * serviceGroupInfo - The service's group information structure
 * dIter - The decode iterator
 */
RsslRet decodeServiceGroupInfo(RsslServiceGroupInfo* serviceGroupInfo,
									  RsslDecodeIterator* dIter)
{
	RsslRet ret = 0;
	RsslElementList	elementList;
	RsslElementEntry element;

	/* decode element list */
	if ((ret = rsslDecodeElementList(dIter, &elementList, NULL)) < RSSL_RET_SUCCESS)
	{
		printf("rsslDecodeElementList() failed with return code: %d\n", ret);
		return ret;
	}

	/* decode element list elements */
	while ((ret = rsslDecodeElementEntry(dIter, &element)) != RSSL_RET_END_OF_CONTAINER)
	{
		if (ret == RSSL_RET_SUCCESS)
		{
			/* get service group information */
			/* Group */
			if (rsslBufferIsEqual(&element.name, &RSSL_ENAME_GROUP))
			{
				if (element.encData.length < MAX_GROUP_INFO_LEN)
				{
					memcpy(serviceGroupInfo->Group, element.encData.data, element.encData.length);
				}
				else
				{
					memcpy(serviceGroupInfo->Group, element.encData.data, MAX_GROUP_INFO_LEN);
				}
			}
			/* MergedToGroup */
			else if (rsslBufferIsEqual(&element.name, &RSSL_ENAME_MERG_TO_GRP))
			{
				if (element.encData.length < MAX_GROUP_INFO_LEN)
				{
					memcpy(serviceGroupInfo->MergedToGroup, element.encData.data, element.encData.length);
				}
				else
				{
					memcpy(serviceGroupInfo->MergedToGroup, element.encData.data, MAX_GROUP_INFO_LEN);
				}
			}
			/* Status */
			else if (rsslBufferIsEqual(&element.name, &RSSL_ENAME_STATUS))
			{
				ret = rsslDecodeState(dIter, &serviceGroupInfo->Status);
				if (ret != RSSL_RET_SUCCESS && ret != RSSL_RET_BLANK_DATA)
				{
					printf("rsslDecodeUInt() failed with return code: %d\n", ret);
					return ret;
				}
			}
		}
		else
		{
			printf("rsslDecodeElementEntry() failed with return code: %d\n", ret);
			return ret;
		}
	}

	return RSSL_RET_SUCCESS;
}

/*
 * Decodes the service's load information into the RsslServiceLoadInfo structure.
 * Returns success if decoding succeeds or failure if decoding fails.
 * serviceLoadInfo - The service's load information structure
 * dIter - The decode iterator
 */
RsslRet decodeServiceLoadInfo(RsslServiceLoadInfo* serviceLoadInfo,
									 RsslDecodeIterator* dIter)
{
	RsslRet ret = 0;
	RsslElementList	elementList;
	RsslElementEntry element;

	/* decode element list */
	if ((ret = rsslDecodeElementList(dIter, &elementList, NULL)) < RSSL_RET_SUCCESS)
	{
		printf("rsslDecodeElementList() failed with return code: %d\n", ret);
		return ret;
	}

	/* decode element list elements */
	while ((ret = rsslDecodeElementEntry(dIter, &element)) != RSSL_RET_END_OF_CONTAINER)
	{
		if (ret == RSSL_RET_SUCCESS)
		{
			/* get service load information */
			/* OpenLimit */
			if (rsslBufferIsEqual(&element.name, &RSSL_ENAME_OPEN_LIMIT))
			{
				ret = rsslDecodeUInt(dIter, &serviceLoadInfo->OpenLimit);
				if (ret != RSSL_RET_SUCCESS && ret != RSSL_RET_BLANK_DATA)
				{
					printf("rsslDecodeUInt() failed with return code: %d\n", ret);
					return ret;
				}
			}
			/* OpenWindow */
			else if (rsslBufferIsEqual(&element.name, &RSSL_ENAME_OPEN_WINDOW))
			{
				ret = rsslDecodeUInt(dIter, &serviceLoadInfo->OpenWindow);
				if (ret != RSSL_RET_SUCCESS && ret != RSSL_RET_BLANK_DATA)
				{
					printf("rsslDecodeUInt() failed with return code: %d\n", ret);
					return ret;
				}
			}
			/* LoadFactor */
			else if (rsslBufferIsEqual(&element.name, &RSSL_ENAME_LOAD_FACT))
			{
				ret = rsslDecodeUInt(dIter, &serviceLoadInfo->LoadFactor);
				if (ret != RSSL_RET_SUCCESS && ret != RSSL_RET_BLANK_DATA)
				{
					printf("rsslDecodeUInt() failed with return code: %d\n", ret);
					return ret;
				}
			}
		}
		else
		{
			printf("rsslDecodeElementEntry() failed with return code: %d\n", ret);
			return ret;
		}
	}

	return RSSL_RET_SUCCESS;
}

/*
 * Decodes the service's data information into the RsslServiceDataInfo structure.
 * Returns success if decoding succeeds or failure if decoding fails.
 * serviceDataInfo - The service's data information structure
 * dIter - The decode iterator
 */
RsslRet decodeServiceDataInfo(RsslServiceDataInfo* serviceDataInfo,
									 RsslDecodeIterator* dIter)
{
	RsslRet ret = 0;
	RsslElementList	elementList;
	RsslElementEntry element;

	/* decode element list */
	if ((ret = rsslDecodeElementList(dIter, &elementList, NULL)) < RSSL_RET_SUCCESS)
	{
		printf("rsslDecodeElementList() failed with return code: %d\n", ret);
		return ret;
	}

	/* decode element list elements */
	while ((ret = rsslDecodeElementEntry(dIter, &element)) != RSSL_RET_END_OF_CONTAINER)
	{
		if (ret == RSSL_RET_SUCCESS)
		{
			/* get service data information */
			/* Type */
			if (rsslBufferIsEqual(&element.name, &RSSL_ENAME_TYPE))
			{
				ret = rsslDecodeUInt(dIter, &serviceDataInfo->Type);
				if (ret != RSSL_RET_SUCCESS && ret != RSSL_RET_BLANK_DATA)
				{
					printf("rsslDecodeUInt() failed with return code: %d\n", ret);
					return ret;
				}
			}
			/* Data */
			else if (rsslBufferIsEqual(&element.name, &RSSL_ENAME_DATA))
			{
				if (element.encData.length < MAX_DATA_INFO_LEN)
				{
					memcpy(serviceDataInfo->Data, element.encData.data, element.encData.length);
				}
				else
				{
					memcpy(serviceDataInfo->Data, element.encData.data, MAX_DATA_INFO_LEN);
				}
			}
		}
		else
		{
			printf("rsslDecodeElementEntry() failed with return code: %d\n", ret);
			return ret;
		}
	}

	return RSSL_RET_SUCCESS;
}

/*
 * Decodes the service's link information into the RsslServiceLinkInfo structure.
 * Returns success if decoding succeeds or failure if decoding fails.
 * serviceLinkInfo - The service's link information structure
 * dIter - The decode iterator
 * maxLinks - The maximum number of link entries that the structure holds
 */
RsslRet decodeServiceLinkInfo(RsslServiceLinkInfo* serviceLinkInfo,
									 RsslDecodeIterator* dIter,
									 RsslUInt32 maxLinks)
{
	RsslRet ret = 0;
	RsslMap map;
	RsslMapEntry mEntry;
	RsslBuffer linkNameBuf;
	RsslElementList	elementList;
	RsslElementEntry element;
	int linkCount = 0;

	/* decode map */
	if ((ret = rsslDecodeMap(dIter, &map)) < RSSL_RET_SUCCESS)
	{
		printf("rsslDecodeMap() failed with return code: %d\n", ret);
		return ret;
	}

	/* service link key data type must be RSSL_DT_ASCII_STRING */
	if (map.keyPrimitiveType != RSSL_DT_ASCII_STRING)
	{
		printf("Map has incorrect keyPrimitiveType: %s", rsslDataTypeToString(map.keyPrimitiveType));
		return RSSL_RET_FAILURE;
	}

	/* decode map entries */
	/* link name is contained in map entry encKey */
	while ((ret = rsslDecodeMapEntry(dIter, &mEntry, &linkNameBuf)) != RSSL_RET_END_OF_CONTAINER)
	{
		/* break out of decoding when max services reached */
		if (linkCount == maxLinks)
		{
			rsslFinishDecodeEntries(dIter);
			break;
		}

		/* store link name in service link information */
		if (linkNameBuf.length < MAX_SRCDIR_INFO_STRLEN)
		{
			strncpy(serviceLinkInfo[linkCount].LinkName, linkNameBuf.data, linkNameBuf.length);
			serviceLinkInfo[linkCount].LinkName[linkNameBuf.length] = '\0';
		}
		else
		{
			strncpy(serviceLinkInfo[linkCount].LinkName, linkNameBuf.data, MAX_SRCDIR_INFO_STRLEN - 1);
			serviceLinkInfo[linkCount].LinkName[MAX_SRCDIR_INFO_STRLEN - 1] = '\0';
		}

		if (ret == RSSL_RET_SUCCESS)
		{
			if (ret != RSSL_RET_SUCCESS && ret != RSSL_RET_BLANK_DATA)
			{
				printf("rsslDecodeUInt() failed with return code: %d\n", ret);
				return ret;
			}

			/* decode element list */
			if ((ret = rsslDecodeElementList(dIter, &elementList, NULL)) < RSSL_RET_SUCCESS)
			{
				printf("rsslDecodeElementList() failed with return code: %d\n", ret);
				return ret;
			}

			/* decode element list elements */
			while ((ret = rsslDecodeElementEntry(dIter, &element)) != RSSL_RET_END_OF_CONTAINER)
			{
				if (ret == RSSL_RET_SUCCESS)
				{
					/* get service link information */
					/* Type */
					if (rsslBufferIsEqual(&element.name, &RSSL_ENAME_TYPE))
					{
						ret = rsslDecodeUInt(dIter, &serviceLinkInfo[linkCount].Type);
						if (ret != RSSL_RET_SUCCESS && ret != RSSL_RET_BLANK_DATA)
						{
							printf("rsslDecodeUInt() failed with return code: %d\n", ret);
							return ret;
						}
					}
					/* LinkState */
					else if (rsslBufferIsEqual(&element.name, &RSSL_ENAME_LINK_STATE))
					{
						ret = rsslDecodeUInt(dIter, &serviceLinkInfo[linkCount].LinkState);
						if (ret != RSSL_RET_SUCCESS && ret != RSSL_RET_BLANK_DATA)
						{
							printf("rsslDecodeUInt() failed with return code: %d\n", ret);
							return ret;
						}
					}
					/* LinkCode */
					else if (rsslBufferIsEqual(&element.name, &RSSL_ENAME_LINK_CODE))
					{
						ret = rsslDecodeUInt(dIter, &serviceLinkInfo[linkCount].LinkCode);
						if (ret != RSSL_RET_SUCCESS && ret != RSSL_RET_BLANK_DATA)
						{
							printf("rsslDecodeUInt() failed with return code: %d\n", ret);
							return ret;
						}
					}
					/* Text */
					else if (rsslBufferIsEqual(&element.name, &RSSL_ENAME_TEXT))
					{
						if (element.encData.length < MAX_SRCDIR_INFO_STRLEN)
						{
							strncpy(serviceLinkInfo[linkCount].Text, element.encData.data, element.encData.length);
							serviceLinkInfo[linkCount].Text[element.encData.length] = '\0';
						}
						else
						{
							strncpy(serviceLinkInfo[linkCount].Text, element.encData.data, MAX_SRCDIR_INFO_STRLEN - 1);
							serviceLinkInfo[linkCount].Text[MAX_SRCDIR_INFO_STRLEN - 1] = '\0';
						}
					}
				}
				else
				{
					printf("rsslDecodeElementEntry() failed with return code: %d\n", ret);
					return ret;
				}
			}
		}
		linkCount++;
	}

	return RSSL_RET_SUCCESS;
}

/*
 * Encodes the source directory close.  Returns success if
 * encoding succeeds or failure if encoding fails.
 * chnl - The channel to send a source directory close to
 * msgBuf - The message buffer to encode the source directory close into
 * streamId - The stream id of the source directory close
 */
RsslRet encodeSourceDirectoryClose(RsslChannel* chnl, RsslBuffer* msgBuf, RsslInt32 streamId)
{
	RsslRet ret = 0;
	RsslCloseMsg msg = RSSL_INIT_CLOSE_MSG;
	RsslEncodeIterator encodeIter;

	/* clear encode iterator */
	rsslClearEncodeIterator(&encodeIter);

	/* set-up message */
	msg.msgBase.msgClass = RSSL_MC_CLOSE;
	msg.msgBase.streamId = streamId;
	msg.msgBase.domainType = RSSL_DMT_SOURCE;
	msg.msgBase.containerType = RSSL_DT_NO_DATA;
	
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
 * Encodes the source directory request reject status.  Returns success if
 * encoding succeeds or failure if encoding fails.
 * chnl - The channel to send request reject status message to
 * streamId - The stream id of the request
 * reason - The reason for the reject
 * msgBuf - The message buffer to encode the source directory request reject into
 */
RsslRet encodeSrcDirectoryRequestReject(RsslChannel* chnl, RsslInt32 streamId, RsslSrcDirRejectReason reason, RsslBuffer* msgBuf)
{
	RsslRet ret = 0;
	RsslStatusMsg msg = RSSL_INIT_STATUS_MSG;
	char stateText[MAX_SRCDIR_INFO_STRLEN];
	RsslEncodeIterator encodeIter;

	/* clear encode iterator */
	rsslClearEncodeIterator(&encodeIter);

	/* set-up message */
	msg.msgBase.msgClass = RSSL_MC_STATUS;
	msg.msgBase.streamId = streamId;
	msg.msgBase.domainType = RSSL_DMT_SOURCE;
	msg.msgBase.containerType = RSSL_DT_NO_DATA;
	msg.flags = RSSL_STMF_HAS_STATE;
	msg.state.streamState = RSSL_STREAM_CLOSED_RECOVER;
	msg.state.dataState = RSSL_DATA_SUSPECT;
	switch(reason)
	{
	case MAX_SRCDIR_REQUESTS_REACHED:
		msg.state.code = RSSL_SC_TOO_MANY_ITEMS;
		snprintf(stateText, 256, "Source directory request rejected for stream id %d - max request count reached", streamId);
		msg.state.text.data = stateText;
		msg.state.text.length = (RsslUInt32)strlen(stateText) + 1;
		break;
	case INCORRECT_FILTER_FLAGS:
		msg.state.code = RSSL_SC_USAGE_ERROR;
		snprintf(stateText, 256, "Source directory request rejected for stream id %d - request must minimally have RDM_DIRECTORY_SERVICE_INFO_FILTER, RDM_DIRECTORY_SERVICE_STATE_FILTER, and RDM_DIRECTORY_SERVICE_GROUP_FILTER filter flags", streamId);
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

