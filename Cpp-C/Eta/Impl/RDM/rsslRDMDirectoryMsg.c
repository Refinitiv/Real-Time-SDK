/*
 * This source code is provided under the Apache 2.0 license and is provided
 * AS IS with no warranty or guarantee of fit for purpose.  See the project's 
 * LICENSE.md for details. 
 * Copyright Thomson Reuters 2015. All rights reserved.
*/

#include "rtr/rsslErrorInfo.h"
#include "rtr/rsslRDMDirectoryMsg.h"

static RsslRet _rsslEncodeServiceList(RsslEncodeIterator *pEncodeIter, RsslRDMDirectoryMsg *pDirectoryResponse, RsslErrorInfo *pError);

RSSL_VA_API RsslRet rsslEncodeRDMDirectoryMsg(RsslEncodeIterator *pEncodeIter, RsslRDMDirectoryMsg *pDirectoryMsg, RsslUInt32 *pBytesWritten, RsslErrorInfo *pError)
{
	RsslRet ret;
	RsslMsg msg;

	switch(pDirectoryMsg->rdmMsgBase.rdmMsgType)
	{
		case RDM_DR_MT_REQUEST:
		{
			RsslRequestMsg *pReqMsg = &msg.requestMsg;
			RsslRDMDirectoryRequest *pDirectoryRequest = &pDirectoryMsg->request;
			rsslClearRequestMsg(pReqMsg);

			/* Populate msgBase */
			pReqMsg->msgBase.msgClass = RSSL_MC_REQUEST;
			pReqMsg->msgBase.streamId = pDirectoryRequest->rdmMsgBase.streamId;
			pReqMsg->msgBase.domainType = RSSL_DMT_SOURCE;
			pReqMsg->msgBase.containerType = RSSL_DT_NO_DATA;
			pReqMsg->flags = RSSL_RQMF_NONE;

			if (pDirectoryRequest->flags & RDM_DR_RQF_STREAMING)
				pReqMsg->flags = RSSL_RQMF_STREAMING;

			/* Populate key with username */
			pReqMsg->msgBase.msgKey.flags = RSSL_MKF_HAS_FILTER;
			pReqMsg->msgBase.msgKey.filter = pDirectoryRequest->filter;

			if (pDirectoryRequest->serviceId)
			{
				pReqMsg->msgBase.msgKey.flags |= RSSL_MKF_HAS_SERVICE_ID;
				pReqMsg->msgBase.msgKey.serviceId = pDirectoryRequest->serviceId;
			}

			/* Begin encoding message */
			if (!RSSL_ERROR_INFO_CHECK((ret = rsslEncodeMsg(pEncodeIter, (RsslMsg*)pReqMsg)) == RSSL_RET_SUCCESS, ret, pError)) return RSSL_RET_FAILURE;

			*pBytesWritten = rsslGetEncodedBufferLength(pEncodeIter);
			pError->rsslErrorInfoCode = RSSL_EIC_SUCCESS;
			return RSSL_RET_SUCCESS;
		}

		case RDM_DR_MT_CONSUMER_STATUS:
		{
			RsslGenericMsg *pGenericMsg = &msg.genericMsg;
			RsslMap map;
			RsslMapEntry mapEntry;
			RsslElementList elementList;
			RsslElementEntry elementEntry;
			RsslUInt32 i;

			RsslRDMDirectoryConsumerStatus *pConsumerStatus = &pDirectoryMsg->consumerStatus;

			rsslClearGenericMsg(pGenericMsg);
			pGenericMsg->msgBase.msgClass = RSSL_MC_GENERIC;
			pGenericMsg->msgBase.streamId = pConsumerStatus->rdmMsgBase.streamId;
			pGenericMsg->msgBase.domainType = RSSL_DMT_SOURCE;
			pGenericMsg->msgBase.containerType = RSSL_DT_MAP;
			pGenericMsg->flags = RSSL_GNMF_HAS_MSG_KEY;

			pGenericMsg->msgBase.msgKey.flags = RSSL_MKF_HAS_NAME;
			pGenericMsg->msgBase.msgKey.name = RSSL_ENAME_CONS_STATUS;

			if (!RSSL_ERROR_INFO_CHECK((ret = rsslEncodeMsgInit(pEncodeIter, &msg, 0)) == RSSL_RET_ENCODE_CONTAINER, ret, pError)) return RSSL_RET_FAILURE;

			rsslClearMap(&map);
			map.flags = RSSL_MPF_NONE;
			map.keyPrimitiveType = RSSL_DT_UINT;
			map.containerType = RSSL_DT_ELEMENT_LIST;

			if (!RSSL_ERROR_INFO_CHECK((ret = rsslEncodeMapInit(pEncodeIter, &map, 0, 0)) == RSSL_RET_SUCCESS, ret, pError)) return RSSL_RET_FAILURE;

			for( i = 0; i < pConsumerStatus->consumerServiceStatusCount; ++i)
			{
				RsslRDMConsumerStatusService *pServiceStatus = &pConsumerStatus->consumerServiceStatusList[i];
				rsslClearMapEntry(&mapEntry);
				mapEntry.flags = RSSL_MPEF_NONE;
				mapEntry.action = pServiceStatus->action;

				if (mapEntry.action != RSSL_MPEA_DELETE_ENTRY)
				{
					if (!RSSL_ERROR_INFO_CHECK((ret = rsslEncodeMapEntryInit(pEncodeIter, &mapEntry, &pServiceStatus->serviceId, 0)) == RSSL_RET_SUCCESS, ret, pError)) return RSSL_RET_FAILURE;

					/* Encode one element indicating the WarmStandbyMode*/
					rsslClearElementList(&elementList);
					elementList.flags = RSSL_ELF_HAS_STANDARD_DATA;
					if (!RSSL_ERROR_INFO_CHECK((ret = rsslEncodeElementListInit(pEncodeIter, &elementList, 0, 0)) == RSSL_RET_SUCCESS, ret, pError)) return RSSL_RET_FAILURE;

					rsslClearElementEntry(&elementEntry);
					elementEntry.name = RSSL_ENAME_SOURCE_MIROR_MODE;
					elementEntry.dataType = RSSL_DT_UINT;
					if (!RSSL_ERROR_INFO_CHECK((ret = rsslEncodeElementEntry(pEncodeIter, &elementEntry, &pConsumerStatus->consumerServiceStatusList[i].sourceMirroringMode)) == RSSL_RET_SUCCESS, ret, pError)) return RSSL_RET_FAILURE;

					if (!RSSL_ERROR_INFO_CHECK((ret = rsslEncodeElementListComplete(pEncodeIter, RSSL_TRUE)) == RSSL_RET_SUCCESS, ret, pError)) return RSSL_RET_FAILURE;

					if (!RSSL_ERROR_INFO_CHECK((ret = rsslEncodeMapEntryComplete(pEncodeIter, RSSL_TRUE)) == RSSL_RET_SUCCESS, ret, pError)) return RSSL_RET_FAILURE;
				}
				else
				{
					if (!RSSL_ERROR_INFO_CHECK((ret = rsslEncodeMapEntry(pEncodeIter, &mapEntry, &pServiceStatus->serviceId)) == RSSL_RET_SUCCESS, ret, pError)) return RSSL_RET_FAILURE;
				}
			}

				if (!RSSL_ERROR_INFO_CHECK((ret = rsslEncodeMapComplete(pEncodeIter, RSSL_TRUE)) == RSSL_RET_SUCCESS, ret, pError)) return RSSL_RET_FAILURE;

				if (!RSSL_ERROR_INFO_CHECK((ret = rsslEncodeMsgComplete(pEncodeIter, RSSL_TRUE)) == RSSL_RET_SUCCESS, ret, pError)) return RSSL_RET_FAILURE;

			*pBytesWritten = rsslGetEncodedBufferLength(pEncodeIter);
			pError->rsslErrorInfoCode = RSSL_EIC_SUCCESS;
			return RSSL_RET_SUCCESS;
		}

		case RDM_DR_MT_CLOSE:
		{
			RsslCloseMsg *pCloseMsg = &msg.closeMsg;
			rsslClearCloseMsg(pCloseMsg);
			pCloseMsg->msgBase.msgClass = RSSL_MC_CLOSE;
			pCloseMsg->msgBase.streamId = pDirectoryMsg->rdmMsgBase.streamId;
			pCloseMsg->msgBase.domainType = RSSL_DMT_SOURCE;
			pCloseMsg->msgBase.containerType = RSSL_DT_NO_DATA;
			pCloseMsg->flags = RSSL_CLMF_NONE;

			if (!RSSL_ERROR_INFO_CHECK((ret = rsslEncodeMsg(pEncodeIter, &msg)) == RSSL_RET_SUCCESS, ret, pError)) return RSSL_RET_FAILURE;

			*pBytesWritten = rsslGetEncodedBufferLength(pEncodeIter);
			pError->rsslErrorInfoCode = RSSL_EIC_SUCCESS;
			return RSSL_RET_SUCCESS;
		}

		case RDM_DR_MT_REFRESH:
		{
			RsslRefreshMsg refreshMsg;
			RsslRDMDirectoryRefresh *pDirectoryRefresh = &pDirectoryMsg->refresh;

			
			rsslClearRefreshMsg(&refreshMsg);
			/* Populate msgBase */
			refreshMsg.msgBase.msgClass = RSSL_MC_REFRESH;
			refreshMsg.msgBase.streamId = pDirectoryRefresh->rdmMsgBase.streamId;
			refreshMsg.msgBase.domainType = RSSL_DMT_SOURCE;
			refreshMsg.msgBase.containerType = RSSL_DT_MAP;
			refreshMsg.flags = RSSL_RFMF_HAS_MSG_KEY | RSSL_RFMF_REFRESH_COMPLETE;

			if (pDirectoryRefresh->flags & RDM_DR_RFF_SOLICITED)
				refreshMsg.flags |= RSSL_RFMF_SOLICITED;

			if (pDirectoryRefresh->flags & RDM_DR_RFF_CLEAR_CACHE)
				refreshMsg.flags |= RSSL_RFMF_CLEAR_CACHE;

			refreshMsg.state = pDirectoryRefresh->state;

			/* Populate key with username */
			refreshMsg.msgBase.msgKey.flags = RSSL_MKF_HAS_FILTER;
			refreshMsg.msgBase.msgKey.filter = pDirectoryRefresh->filter;

			if (pDirectoryRefresh->flags & RDM_DR_RFF_HAS_SERVICE_ID)
			{
				refreshMsg.msgBase.msgKey.flags |= RSSL_MKF_HAS_SERVICE_ID;
				refreshMsg.msgBase.msgKey.serviceId = pDirectoryRefresh->serviceId;
			}

			if (pDirectoryRefresh->flags & RDM_DR_RFF_HAS_SEQ_NUM)
			{
				refreshMsg.flags |= RSSL_RFMF_HAS_SEQ_NUM;
				refreshMsg.seqNum = pDirectoryRefresh->sequenceNumber;
			}

			/* Begin encoding message */
			if (!RSSL_ERROR_INFO_CHECK((ret = rsslEncodeMsgInit(pEncodeIter, (RsslMsg*)&refreshMsg, 0)) == RSSL_RET_ENCODE_CONTAINER, ret, pError)) return RSSL_RET_FAILURE;

			if ((ret = _rsslEncodeServiceList(pEncodeIter, pDirectoryMsg, pError)) != RSSL_RET_SUCCESS)
				return ret;

			if (!RSSL_ERROR_INFO_CHECK((ret = rsslEncodeMsgComplete(pEncodeIter, RSSL_TRUE)) == RSSL_RET_SUCCESS, ret, pError)) return RSSL_RET_FAILURE;

			*pBytesWritten = rsslGetEncodedBufferLength(pEncodeIter);
			pError->rsslErrorInfoCode = RSSL_EIC_SUCCESS;
			return RSSL_RET_SUCCESS;
		}

		case RDM_DR_MT_UPDATE:
		{
			RsslUpdateMsg updateMsg;
			RsslRDMDirectoryUpdate *pDirectoryUpdate = &pDirectoryMsg->update;
			
			rsslClearUpdateMsg(&updateMsg);
			/* Populate msgBase */
			updateMsg.msgBase.msgClass = RSSL_MC_UPDATE;
			updateMsg.msgBase.streamId = pDirectoryUpdate->rdmMsgBase.streamId;
			updateMsg.msgBase.domainType = RSSL_DMT_SOURCE;
			updateMsg.msgBase.containerType = RSSL_DT_MAP;
			updateMsg.flags = RSSL_UPMF_DO_NOT_CONFLATE;

			if (pDirectoryUpdate->flags & RDM_DR_UPF_HAS_FILTER)
			{
				updateMsg.flags |= RSSL_UPMF_HAS_MSG_KEY;
				updateMsg.msgBase.msgKey.flags |= RSSL_MKF_HAS_FILTER;
				updateMsg.msgBase.msgKey.filter = pDirectoryUpdate->filter;
			}

			if (pDirectoryUpdate->flags & RDM_DR_UPF_HAS_SERVICE_ID)
			{
				updateMsg.flags |= RSSL_UPMF_HAS_MSG_KEY;
				updateMsg.msgBase.msgKey.flags |= RSSL_MKF_HAS_SERVICE_ID;
				updateMsg.msgBase.msgKey.serviceId = pDirectoryUpdate->serviceId;
			}

			if (pDirectoryUpdate->flags & RDM_DR_UPF_HAS_SEQ_NUM)
			{
				updateMsg.flags |= RSSL_UPMF_HAS_SEQ_NUM;
				updateMsg.seqNum = pDirectoryUpdate->sequenceNumber;
			}

			/* Begin encoding message */
			if (!RSSL_ERROR_INFO_CHECK((ret = rsslEncodeMsgInit(pEncodeIter, (RsslMsg*)&updateMsg, 0)) == RSSL_RET_ENCODE_CONTAINER, ret, pError)) return RSSL_RET_FAILURE;

			if ((ret = _rsslEncodeServiceList(pEncodeIter, pDirectoryMsg, pError)) != RSSL_RET_SUCCESS)
				return ret;

			if (!RSSL_ERROR_INFO_CHECK((ret = rsslEncodeMsgComplete(pEncodeIter, RSSL_TRUE)) == RSSL_RET_SUCCESS, ret, pError)) return RSSL_RET_FAILURE;

			*pBytesWritten = rsslGetEncodedBufferLength(pEncodeIter);
			pError->rsslErrorInfoCode = RSSL_EIC_SUCCESS;
			return RSSL_RET_SUCCESS;
		}
		case RDM_DR_MT_STATUS:
		{
			RsslStatusMsg statusMsg;
			RsslRDMDirectoryStatus *pDirectoryStatus = &pDirectoryMsg->status;

			rsslClearStatusMsg(&statusMsg);

			if (pDirectoryStatus->flags & RDM_DR_STF_HAS_FILTER)
			{
				statusMsg.flags |= RSSL_STMF_HAS_MSG_KEY;
				statusMsg.msgBase.msgKey.flags |= RSSL_MKF_HAS_FILTER;
				statusMsg.msgBase.msgKey.filter = pDirectoryStatus->filter;
			}

			if (pDirectoryStatus->flags & RDM_DR_STF_HAS_SERVICE_ID)
			{
				statusMsg.flags |= RSSL_STMF_HAS_MSG_KEY;
				statusMsg.msgBase.msgKey.flags |= RSSL_MKF_HAS_SERVICE_ID;
				statusMsg.msgBase.msgKey.serviceId = pDirectoryStatus->serviceId;
			}

			/* Populate msgBase */
			statusMsg.msgBase.msgClass = RSSL_MC_STATUS;
			statusMsg.msgBase.streamId = pDirectoryStatus->rdmMsgBase.streamId;
			statusMsg.msgBase.domainType = RSSL_DMT_SOURCE;
			statusMsg.msgBase.containerType = RSSL_DT_NO_DATA;

			if (pDirectoryStatus->flags & RDM_DR_STF_CLEAR_CACHE)
				statusMsg.flags |= RSSL_STMF_CLEAR_CACHE;

			if (pDirectoryStatus->flags & RDM_DR_STF_HAS_STATE) 
			{
				statusMsg.flags |= RSSL_STMF_HAS_STATE;
				statusMsg.state = pDirectoryStatus->state;
			}

			if (!RSSL_ERROR_INFO_CHECK((ret = rsslEncodeMsg(pEncodeIter, (RsslMsg*)&statusMsg)) == RSSL_RET_SUCCESS, ret, pError)) return RSSL_RET_FAILURE;

			*pBytesWritten = rsslGetEncodedBufferLength(pEncodeIter);
			pError->rsslErrorInfoCode = RSSL_EIC_SUCCESS;
			return RSSL_RET_SUCCESS;
		}
		default:
			pError->rsslErrorInfoCode = RSSL_EIC_FAILURE;
			return RSSL_RET_FAILURE;
	}
}

/* Encode service list payload. Assumes iterator is ready to encode map */
static RsslRet _rsslEncodeServiceList(RsslEncodeIterator *pEncodeIter, RsslRDMDirectoryMsg *pDirectoryResponse, RsslErrorInfo *pError)
{
	RsslElementList elementList;
	RsslElementEntry elementEntry;
	RsslUInt32 i;
	RsslMap map; RsslMapEntry mapEntry;
	RsslFilterList filterList; RsslFilterEntry filterEntry;
	RsslRet ret;

	RsslUInt32 filter;
	RsslUInt32 serviceCount;
	RsslRDMService *pServices;

	switch(pDirectoryResponse->rdmMsgBase.rdmMsgType)
	{
		case RDM_DR_MT_REFRESH:
			filter = pDirectoryResponse->refresh.filter;
			serviceCount = pDirectoryResponse->refresh.serviceCount;
			pServices = pDirectoryResponse->refresh.serviceList;
			break;
		case RDM_DR_MT_UPDATE: 
			filter = (pDirectoryResponse->update.flags & RDM_DR_UPF_HAS_FILTER) ? pDirectoryResponse->update.filter: 0;
			serviceCount = pDirectoryResponse->update.serviceCount;
			pServices = pDirectoryResponse->update.serviceList;
			break;
		default:
			rsslSetErrorInfo(pError, RSSL_EIC_FAILURE, RSSL_RET_FAILURE, __FILE__, __LINE__, "Unknown directory message type %u\n", pDirectoryResponse->rdmMsgBase.rdmMsgType);
			return RSSL_RET_FAILURE;
	}


	/* Encode services */
	rsslClearMap(&map);
	map.flags = RSSL_MPF_NONE;
	map.keyPrimitiveType = RSSL_DT_UINT;
	map.containerType = RSSL_DT_FILTER_LIST;
	if (!RSSL_ERROR_INFO_CHECK((ret = rsslEncodeMapInit(pEncodeIter, &map, 0, 0)) == RSSL_RET_SUCCESS, ret, pError)) return RSSL_RET_FAILURE;

	for (i = 0; i < serviceCount; ++i)
	{
		RsslUInt32 j;
		RsslRDMService *pService = &pServices[i];
		rsslClearMapEntry(&mapEntry);
		mapEntry.flags = RSSL_MPEF_NONE;
		mapEntry.action = pService->action;

		if(pService->serviceId > 0xFFFF)
		{
			rsslSetErrorInfo(pError, RSSL_EIC_FAILURE, RSSL_RET_FAILURE, __FILE__, __LINE__, "Service Id: %d is out of range.\n", pService->serviceId);
			return RSSL_RET_FAILURE;
		}

		if(mapEntry.action != RSSL_MPEA_DELETE_ENTRY)
		{
			if (!RSSL_ERROR_INFO_CHECK((ret = rsslEncodeMapEntryInit(pEncodeIter, &mapEntry, &pService->serviceId, 0)) == RSSL_RET_SUCCESS, ret, pError)) return RSSL_RET_FAILURE;

			rsslClearFilterList(&filterList);
			filterList.flags = RSSL_FTF_NONE;
			filterList.containerType = RSSL_DT_ELEMENT_LIST;

			if (!RSSL_ERROR_INFO_CHECK((ret = rsslEncodeFilterListInit(pEncodeIter, &filterList)) == RSSL_RET_SUCCESS, ret, pError)) return RSSL_RET_FAILURE;

			/* Encode each category of information, if the information is present and the filter
			 * shows that it is needed */

			/* Info */
			if (pService->flags & RDM_SVCF_HAS_INFO)
			{
				RsslRDMServiceInfo *pInfo = &pService->info;
				RsslArray array;

				rsslClearFilterEntry(&filterEntry);
				filterEntry.flags = RSSL_FTEF_NONE;
				filterEntry.action = pInfo->action;
				filterEntry.id = RDM_DIRECTORY_SERVICE_INFO_ID;

				if (filterEntry.action != RSSL_FTEA_CLEAR_ENTRY)
				{
					if (!RSSL_ERROR_INFO_CHECK((ret = rsslEncodeFilterEntryInit(pEncodeIter, &filterEntry, 0)) == RSSL_RET_SUCCESS, ret, pError)) return RSSL_RET_FAILURE;

					rsslClearElementList(&elementList);
					elementList.flags = RSSL_ELF_HAS_STANDARD_DATA;
					if (!RSSL_ERROR_INFO_CHECK((ret = rsslEncodeElementListInit(pEncodeIter, &elementList, 0, 0)) == RSSL_RET_SUCCESS, ret, pError)) return RSSL_RET_FAILURE;

					rsslClearElementEntry(&elementEntry);

					/* Name */
					elementEntry.name = RSSL_ENAME_NAME;
					elementEntry.dataType = RSSL_DT_ASCII_STRING;
					if (!RSSL_ERROR_INFO_CHECK((ret = rsslEncodeElementEntry(pEncodeIter, &elementEntry, &pInfo->serviceName)) == RSSL_RET_SUCCESS, ret, pError)) return RSSL_RET_FAILURE;

					/* Vendor */
					if (pInfo->flags & RDM_SVC_IFF_HAS_VENDOR)
					{
						elementEntry.name = RSSL_ENAME_VENDOR;
						elementEntry.dataType = RSSL_DT_ASCII_STRING;
						if (!RSSL_ERROR_INFO_CHECK((ret = rsslEncodeElementEntry(pEncodeIter, &elementEntry, &pInfo->vendor)) == RSSL_RET_SUCCESS, ret, pError)) return RSSL_RET_FAILURE;
					}

					/* IsSource */
					if (pInfo->flags & RDM_SVC_IFF_HAS_IS_SOURCE)
					{
						elementEntry.name = RSSL_ENAME_IS_SOURCE;
						elementEntry.dataType = RSSL_DT_UINT;
						if (!RSSL_ERROR_INFO_CHECK((ret = rsslEncodeElementEntry(pEncodeIter, &elementEntry, &pInfo->isSource)) == RSSL_RET_SUCCESS, ret, pError)) return RSSL_RET_FAILURE;
					}

					/* Capabilities */
					elementEntry.name = RSSL_ENAME_CAPABILITIES;
					elementEntry.dataType = RSSL_DT_ARRAY;
					if (!RSSL_ERROR_INFO_CHECK((ret = rsslEncodeElementEntryInit(pEncodeIter, &elementEntry, 0)) == RSSL_RET_SUCCESS, ret, pError)) return RSSL_RET_FAILURE;

					rsslClearArray(&array);
					array.primitiveType = RSSL_DT_UINT;
					array.itemLength = 0;
					if (!RSSL_ERROR_INFO_CHECK((ret = rsslEncodeArrayInit(pEncodeIter, &array)) == RSSL_RET_SUCCESS, ret, pError)) return RSSL_RET_FAILURE;

					for (j = 0; j < pInfo->capabilitiesCount; ++j)
					{
						if (!RSSL_ERROR_INFO_CHECK((ret = rsslEncodeArrayEntry(pEncodeIter, 0, &pInfo->capabilitiesList[j])) == RSSL_RET_SUCCESS, ret, pError)) return RSSL_RET_FAILURE;
					}

					if (!RSSL_ERROR_INFO_CHECK((ret = rsslEncodeArrayComplete(pEncodeIter, RSSL_TRUE)) == RSSL_RET_SUCCESS, ret, pError)) return RSSL_RET_FAILURE;

					if (!RSSL_ERROR_INFO_CHECK((ret = rsslEncodeElementEntryComplete(pEncodeIter, RSSL_TRUE)) == RSSL_RET_SUCCESS, ret, pError)) return RSSL_RET_FAILURE;

					/* DictionariesProvided */
					if (pInfo->flags & RDM_SVC_IFF_HAS_DICTS_PROVIDED)
					{
						elementEntry.name = RSSL_ENAME_DICTIONARIES_PROVIDED;
						elementEntry.dataType = RSSL_DT_ARRAY;
						if (!RSSL_ERROR_INFO_CHECK((ret = rsslEncodeElementEntryInit(pEncodeIter, &elementEntry, 0)) == RSSL_RET_SUCCESS, ret, pError)) return RSSL_RET_FAILURE;

						rsslClearArray(&array);
						array.primitiveType = RSSL_DT_ASCII_STRING;
						array.itemLength = 0;
						if (!RSSL_ERROR_INFO_CHECK((ret = rsslEncodeArrayInit(pEncodeIter, &array)) == RSSL_RET_SUCCESS, ret, pError)) return RSSL_RET_FAILURE;

						for (j = 0; j < pInfo->dictionariesProvidedCount; ++j)
						{
							if (!RSSL_ERROR_INFO_CHECK((ret = rsslEncodeArrayEntry(pEncodeIter, 0, &pInfo->dictionariesProvidedList[j])) == RSSL_RET_SUCCESS, ret, pError)) return RSSL_RET_FAILURE;
						}

						if (!RSSL_ERROR_INFO_CHECK((ret = rsslEncodeArrayComplete(pEncodeIter, RSSL_TRUE)) == RSSL_RET_SUCCESS, ret, pError)) return RSSL_RET_FAILURE;

						if (!RSSL_ERROR_INFO_CHECK((ret = rsslEncodeElementEntryComplete(pEncodeIter, RSSL_TRUE)) == RSSL_RET_SUCCESS, ret, pError)) return RSSL_RET_FAILURE;
					}

					/* DictionariesUsed */
					if (pInfo->flags & RDM_SVC_IFF_HAS_DICTS_USED)
					{
						elementEntry.name = RSSL_ENAME_DICTIONARIES_USED;
						elementEntry.dataType = RSSL_DT_ARRAY;
						if (!RSSL_ERROR_INFO_CHECK((ret = rsslEncodeElementEntryInit(pEncodeIter, &elementEntry, 0)) == RSSL_RET_SUCCESS, ret, pError)) return RSSL_RET_FAILURE;

						rsslClearArray(&array);
						array.primitiveType = RSSL_DT_ASCII_STRING;
						array.itemLength = 0;
						if (!RSSL_ERROR_INFO_CHECK((ret = rsslEncodeArrayInit(pEncodeIter, &array)) == RSSL_RET_SUCCESS, ret, pError)) return RSSL_RET_FAILURE;

						for (j = 0; j < pInfo->dictionariesUsedCount; ++j)
						{
							if (!RSSL_ERROR_INFO_CHECK((ret = rsslEncodeArrayEntry(pEncodeIter, 0, &pInfo->dictionariesUsedList[j])) == RSSL_RET_SUCCESS, ret, pError)) return RSSL_RET_FAILURE;
						}

						if (!RSSL_ERROR_INFO_CHECK((ret = rsslEncodeArrayComplete(pEncodeIter, RSSL_TRUE)) == RSSL_RET_SUCCESS, ret, pError)) return RSSL_RET_FAILURE;

						if (!RSSL_ERROR_INFO_CHECK((ret = rsslEncodeElementEntryComplete(pEncodeIter, RSSL_TRUE)) == RSSL_RET_SUCCESS, ret, pError)) return RSSL_RET_FAILURE;
					}

					/* QoS */
					if (pInfo->flags & RDM_SVC_IFF_HAS_QOS)
					{
						elementEntry.name = RSSL_ENAME_QOS;
						elementEntry.dataType = RSSL_DT_ARRAY;
						if (!RSSL_ERROR_INFO_CHECK((ret = rsslEncodeElementEntryInit(pEncodeIter, &elementEntry, 0)) == RSSL_RET_SUCCESS, ret, pError)) return RSSL_RET_FAILURE;

						rsslClearArray(&array);
						array.primitiveType = RSSL_DT_QOS;
						array.itemLength = 0;
						if (!RSSL_ERROR_INFO_CHECK((ret = rsslEncodeArrayInit(pEncodeIter, &array)) == RSSL_RET_SUCCESS, ret, pError)) return RSSL_RET_FAILURE;

						for (j = 0; j < pInfo->qosCount; ++j)
						{
							if (!RSSL_ERROR_INFO_CHECK((ret = rsslEncodeArrayEntry(pEncodeIter, 0, &pInfo->qosList[j])) == RSSL_RET_SUCCESS, ret, pError)) return RSSL_RET_FAILURE;
						}

						if (!RSSL_ERROR_INFO_CHECK((ret = rsslEncodeArrayComplete(pEncodeIter, RSSL_TRUE)) == RSSL_RET_SUCCESS, ret, pError)) return RSSL_RET_FAILURE;

						if (!RSSL_ERROR_INFO_CHECK((ret = rsslEncodeElementEntryComplete(pEncodeIter, RSSL_TRUE)) == RSSL_RET_SUCCESS, ret, pError)) return RSSL_RET_FAILURE;
					}

					/* SupportsQosRange */
					if (pInfo->flags & RDM_SVC_IFF_HAS_SUPPORT_QOS_RANGE)
					{
						elementEntry.name = RSSL_ENAME_SUPPS_QOS_RANGE;
						elementEntry.dataType = RSSL_DT_UINT;
						if (!RSSL_ERROR_INFO_CHECK((ret = rsslEncodeElementEntry(pEncodeIter, &elementEntry, &pInfo->supportsQosRange)) == RSSL_RET_SUCCESS, ret, pError)) return RSSL_RET_FAILURE;
					}

					/* ItemList */
					if (pInfo->flags & RDM_SVC_IFF_HAS_ITEM_LIST)
					{
						elementEntry.name = RSSL_ENAME_ITEM_LIST;
						elementEntry.dataType = RSSL_DT_ASCII_STRING;
						if (!RSSL_ERROR_INFO_CHECK((ret = rsslEncodeElementEntry(pEncodeIter, &elementEntry, &pInfo->itemList)) == RSSL_RET_SUCCESS, ret, pError)) return RSSL_RET_FAILURE;
					}

					/* SupportsOutOfBandSnapshots */
					if (pInfo->flags & RDM_SVC_IFF_HAS_SUPPORT_OOB_SNAPSHOTS)
					{
						elementEntry.name = RSSL_ENAME_SUPPS_OOB_SNAPSHOTS;
						elementEntry.dataType = RSSL_DT_UINT;
						if (!RSSL_ERROR_INFO_CHECK((ret = rsslEncodeElementEntry(pEncodeIter, &elementEntry, &pInfo->supportsOutOfBandSnapshots)) == RSSL_RET_SUCCESS, ret, pError)) return RSSL_RET_FAILURE;
					}

					/* AcceptingConsumerStatus */
					if (pInfo->flags & RDM_SVC_IFF_HAS_ACCEPTING_CONS_STATUS)
					{
						elementEntry.name = RSSL_ENAME_ACCEPTING_CONS_STATUS;
						elementEntry.dataType = RSSL_DT_UINT;
						if (!RSSL_ERROR_INFO_CHECK((ret = rsslEncodeElementEntry(pEncodeIter, &elementEntry, &pInfo->acceptingConsumerStatus)) == RSSL_RET_SUCCESS, ret, pError)) return RSSL_RET_FAILURE;
					}

					/* Complete */
					if (!RSSL_ERROR_INFO_CHECK((ret = rsslEncodeElementListComplete(pEncodeIter, RSSL_TRUE)) == RSSL_RET_SUCCESS, ret, pError)) return RSSL_RET_FAILURE;

					if (!RSSL_ERROR_INFO_CHECK((ret = rsslEncodeFilterEntryComplete(pEncodeIter, RSSL_TRUE)) == RSSL_RET_SUCCESS, ret, pError)) return RSSL_RET_FAILURE;
				}
				else
				{
					if (!RSSL_ERROR_INFO_CHECK((ret = rsslEncodeFilterEntry(pEncodeIter, &filterEntry)) == RSSL_RET_SUCCESS, ret, pError)) return RSSL_RET_FAILURE;
				}

			}

			/* State */
			if (pService->flags & RDM_SVCF_HAS_STATE)
			{
				RsslRDMServiceState *pState = &pService->state;
				rsslClearFilterEntry(&filterEntry);
				filterEntry.flags = RSSL_FTEF_NONE;
				filterEntry.action = pState->action;
				filterEntry.id = RDM_DIRECTORY_SERVICE_STATE_ID;

				if (filterEntry.action != RSSL_FTEA_CLEAR_ENTRY)
				{
					if (!RSSL_ERROR_INFO_CHECK((ret = rsslEncodeFilterEntryInit(pEncodeIter, &filterEntry, 0)) == RSSL_RET_SUCCESS, ret, pError)) return RSSL_RET_FAILURE;

					rsslClearElementList(&elementList);
					elementList.flags = RSSL_ELF_HAS_STANDARD_DATA;
					if (!RSSL_ERROR_INFO_CHECK((ret = rsslEncodeElementListInit(pEncodeIter, &elementList, 0, 0)) == RSSL_RET_SUCCESS, ret, pError)) return RSSL_RET_FAILURE;

					/* ServiceState */
					elementEntry.name = RSSL_ENAME_SVC_STATE;
					elementEntry.dataType = RSSL_DT_UINT;
					if (!RSSL_ERROR_INFO_CHECK((ret = rsslEncodeElementEntry(pEncodeIter, &elementEntry, &pState->serviceState)) == RSSL_RET_SUCCESS, ret, pError)) return RSSL_RET_FAILURE;

					/* AcceptingRequests */
					if (pState->flags & RDM_SVC_STF_HAS_ACCEPTING_REQS)
					{
						elementEntry.name = RSSL_ENAME_ACCEPTING_REQS;
						elementEntry.dataType = RSSL_DT_UINT;
						if (!RSSL_ERROR_INFO_CHECK((ret = rsslEncodeElementEntry(pEncodeIter, &elementEntry, &pState->acceptingRequests)) == RSSL_RET_SUCCESS, ret, pError)) return RSSL_RET_FAILURE;
					}

					if (pState->flags & RDM_SVC_STF_HAS_STATUS)
					{
						/* Status */
						elementEntry.name = RSSL_ENAME_STATUS;
						elementEntry.dataType = RSSL_DT_STATE;
						if (!RSSL_ERROR_INFO_CHECK((ret = rsslEncodeElementEntry(pEncodeIter, &elementEntry, &pState->status)) == RSSL_RET_SUCCESS, ret, pError)) return RSSL_RET_FAILURE;
					}

					if (!RSSL_ERROR_INFO_CHECK((ret = rsslEncodeElementListComplete(pEncodeIter, RSSL_TRUE)) == RSSL_RET_SUCCESS, ret, pError)) return RSSL_RET_FAILURE;

					if (!RSSL_ERROR_INFO_CHECK((ret = rsslEncodeFilterEntryComplete(pEncodeIter, RSSL_TRUE)) == RSSL_RET_SUCCESS, ret, pError)) return RSSL_RET_FAILURE;
				}
				else
				{
					if (!RSSL_ERROR_INFO_CHECK((ret = rsslEncodeFilterEntry(pEncodeIter, &filterEntry)) == RSSL_RET_SUCCESS, ret, pError)) return RSSL_RET_FAILURE;
				}
			}

			/* Group */
			for(j = 0; j < pService->groupStateCount; ++j)
			{
				RsslRDMServiceGroupState *pGroup = &pService->groupStateList[j];

				rsslClearFilterEntry(&filterEntry);
				filterEntry.flags = RSSL_FTEF_NONE;
				filterEntry.action = pGroup->action;
				filterEntry.id = RDM_DIRECTORY_SERVICE_GROUP_ID;

				if (filterEntry.action != RSSL_FTEA_CLEAR_ENTRY)
				{
					if (!RSSL_ERROR_INFO_CHECK((ret = rsslEncodeFilterEntryInit(pEncodeIter, &filterEntry, 0)) == RSSL_RET_SUCCESS, ret, pError)) return RSSL_RET_FAILURE;

					rsslClearElementList(&elementList);
					elementList.flags = RSSL_ELF_HAS_STANDARD_DATA;
					if (!RSSL_ERROR_INFO_CHECK((ret = rsslEncodeElementListInit(pEncodeIter, &elementList, 0, 0)) == RSSL_RET_SUCCESS, ret, pError)) return RSSL_RET_FAILURE;

					rsslClearElementEntry(&elementEntry);

					/* Group */
					elementEntry.name = RSSL_ENAME_GROUP;
					elementEntry.dataType = RSSL_DT_BUFFER;
					if (!RSSL_ERROR_INFO_CHECK((ret = rsslEncodeElementEntry(pEncodeIter, &elementEntry, &pGroup->group)) == RSSL_RET_SUCCESS, ret, pError)) return RSSL_RET_FAILURE;

					/* MergedToGroup */
					if (pGroup->flags & RDM_SVC_GRF_HAS_MERGED_TO_GROUP)
					{
						elementEntry.name = RSSL_ENAME_MERG_TO_GRP;
						elementEntry.dataType = RSSL_DT_BUFFER;
						if (!RSSL_ERROR_INFO_CHECK((ret = rsslEncodeElementEntry(pEncodeIter, &elementEntry, &pGroup->mergedToGroup)) == RSSL_RET_SUCCESS, ret, pError)) return RSSL_RET_FAILURE;
					}

					if (pGroup->flags & RDM_SVC_GRF_HAS_STATUS)
					{
						/* Status */
						elementEntry.name = RSSL_ENAME_STATUS;
						elementEntry.dataType = RSSL_DT_STATE;
						if (!RSSL_ERROR_INFO_CHECK((ret = rsslEncodeElementEntry(pEncodeIter, &elementEntry, &pGroup->status)) == RSSL_RET_SUCCESS, ret, pError)) return RSSL_RET_FAILURE;
					}

					if (!RSSL_ERROR_INFO_CHECK((ret = rsslEncodeElementListComplete(pEncodeIter, RSSL_TRUE)) == RSSL_RET_SUCCESS, ret, pError)) return RSSL_RET_FAILURE;

					if (!RSSL_ERROR_INFO_CHECK((ret = rsslEncodeFilterEntryComplete(pEncodeIter, RSSL_TRUE)) == RSSL_RET_SUCCESS, ret, pError)) return RSSL_RET_FAILURE;
				}
				else
				{
					if (!RSSL_ERROR_INFO_CHECK((ret = rsslEncodeFilterEntry(pEncodeIter, &filterEntry)) == RSSL_RET_SUCCESS, ret, pError)) return RSSL_RET_FAILURE;
				}
			}

			/* Load */
			if (pService->flags & RDM_SVCF_HAS_LOAD)
			{
				RsslRDMServiceLoad *pLoad = &pService->load;

				rsslClearFilterEntry(&filterEntry);
				filterEntry.flags = RSSL_FTEF_NONE;
				filterEntry.action = pLoad->action;
				filterEntry.id = RDM_DIRECTORY_SERVICE_LOAD_ID;

				if (filterEntry.action != RSSL_FTEA_CLEAR_ENTRY)
				{
					if (!RSSL_ERROR_INFO_CHECK((ret = rsslEncodeFilterEntryInit(pEncodeIter, &filterEntry, 0)) == RSSL_RET_SUCCESS, ret, pError)) return RSSL_RET_FAILURE;

					rsslClearElementList(&elementList);
					elementList.flags = RSSL_ELF_HAS_STANDARD_DATA;
					if (!RSSL_ERROR_INFO_CHECK((ret = rsslEncodeElementListInit(pEncodeIter, &elementList, 0, 0)) == RSSL_RET_SUCCESS, ret, pError)) return RSSL_RET_FAILURE;

					/* OpenLimit */
					if (pLoad->flags & RDM_SVC_LDF_HAS_OPEN_LIMIT)
					{
						elementEntry.name = RSSL_ENAME_OPEN_LIMIT;
						elementEntry.dataType = RSSL_DT_UINT;
						if (!RSSL_ERROR_INFO_CHECK((ret = rsslEncodeElementEntry(pEncodeIter, &elementEntry, &pLoad->openLimit)) == RSSL_RET_SUCCESS, ret, pError)) return RSSL_RET_FAILURE;
					}

					/* OpenWindow */
					if (pLoad->flags & RDM_SVC_LDF_HAS_OPEN_WINDOW)
					{
						elementEntry.name = RSSL_ENAME_OPEN_WINDOW;
						elementEntry.dataType = RSSL_DT_UINT;
						if (!RSSL_ERROR_INFO_CHECK((ret = rsslEncodeElementEntry(pEncodeIter, &elementEntry, &pLoad->openWindow)) == RSSL_RET_SUCCESS, ret, pError)) return RSSL_RET_FAILURE;
					}

					/* LoadFactor */
					if (pLoad->flags & RDM_SVC_LDF_HAS_LOAD_FACTOR)
					{
						elementEntry.name = RSSL_ENAME_LOAD_FACT;
						elementEntry.dataType = RSSL_DT_UINT;
						if (!RSSL_ERROR_INFO_CHECK((ret = rsslEncodeElementEntry(pEncodeIter, &elementEntry, &pLoad->loadFactor)) == RSSL_RET_SUCCESS, ret, pError)) return RSSL_RET_FAILURE;
					}

					if (!RSSL_ERROR_INFO_CHECK((ret = rsslEncodeElementListComplete(pEncodeIter, RSSL_TRUE)) == RSSL_RET_SUCCESS, ret, pError)) return RSSL_RET_FAILURE;

					if (!RSSL_ERROR_INFO_CHECK((ret = rsslEncodeFilterEntryComplete(pEncodeIter, RSSL_TRUE)) == RSSL_RET_SUCCESS, ret, pError)) return RSSL_RET_FAILURE;
				}
				else
				{
					if (!RSSL_ERROR_INFO_CHECK((ret = rsslEncodeFilterEntry(pEncodeIter, &filterEntry)) == RSSL_RET_SUCCESS, ret, pError)) return RSSL_RET_FAILURE;
				}
			}

			/* Data */
			if (pService->flags & RDM_SVCF_HAS_DATA)
			{
				RsslRDMServiceData *pData = &pService->data;

				rsslClearFilterEntry(&filterEntry);
				filterEntry.flags = RSSL_FTEF_NONE;
				filterEntry.action = pData->action;
				filterEntry.id = RDM_DIRECTORY_SERVICE_DATA_ID;

				if (filterEntry.action != RSSL_FTEA_CLEAR_ENTRY)
				{
					if (!RSSL_ERROR_INFO_CHECK((ret = rsslEncodeFilterEntryInit(pEncodeIter, &filterEntry, 0)) == RSSL_RET_SUCCESS, ret, pError)) return RSSL_RET_FAILURE;

					rsslClearElementList(&elementList);
					elementList.flags = RSSL_ELF_HAS_STANDARD_DATA;
					if (!RSSL_ERROR_INFO_CHECK((ret = rsslEncodeElementListInit(pEncodeIter, &elementList, 0, 0)) == RSSL_RET_SUCCESS, ret, pError)) return RSSL_RET_FAILURE;

					if (pData->flags & RDM_SVC_DTF_HAS_DATA)
					{
						/* Type */
						elementEntry.name = RSSL_ENAME_TYPE;
						elementEntry.dataType = RSSL_DT_UINT;
						if (!RSSL_ERROR_INFO_CHECK((ret = rsslEncodeElementEntry(pEncodeIter, &elementEntry, &pData->type)) == RSSL_RET_SUCCESS, ret, pError)) return RSSL_RET_FAILURE;

						/* Data */
						elementEntry.name = RSSL_ENAME_DATA;
						elementEntry.dataType = pData->dataType;
						elementEntry.encData = pData->data;
						if (!RSSL_ERROR_INFO_CHECK((ret = rsslEncodeElementEntry(pEncodeIter, &elementEntry, 0)) == RSSL_RET_SUCCESS, ret, pError)) return RSSL_RET_FAILURE;
					}

					if (!RSSL_ERROR_INFO_CHECK((ret = rsslEncodeElementListComplete(pEncodeIter, RSSL_TRUE)) == RSSL_RET_SUCCESS, ret, pError)) return RSSL_RET_FAILURE;

					if (!RSSL_ERROR_INFO_CHECK((ret = rsslEncodeFilterEntryComplete(pEncodeIter, RSSL_TRUE)) == RSSL_RET_SUCCESS, ret, pError)) return RSSL_RET_FAILURE;
				}
				else
				{
					if (!RSSL_ERROR_INFO_CHECK((ret = rsslEncodeFilterEntry(pEncodeIter, &filterEntry)) == RSSL_RET_SUCCESS, ret, pError)) return RSSL_RET_FAILURE;
				}

			}

			/* Link */
			if (pService->flags & RDM_SVCF_HAS_LINK)
			{
				RsslMap linkMap;
				RsslMapEntry linkMapEntry;

				rsslClearFilterEntry(&filterEntry);
				filterEntry.flags = RSSL_FTEF_HAS_CONTAINER_TYPE;
				filterEntry.action = pService->linkInfo.action;
				filterEntry.id = RDM_DIRECTORY_SERVICE_LINK_ID;
				filterEntry.containerType = RSSL_DT_MAP;

				if (filterEntry.action != RSSL_FTEA_CLEAR_ENTRY)
				{
					if (!RSSL_ERROR_INFO_CHECK((ret = rsslEncodeFilterEntryInit(pEncodeIter, &filterEntry, 0)) == RSSL_RET_SUCCESS, ret, pError)) return RSSL_RET_FAILURE;

					rsslClearMap(&linkMap);
					linkMap.flags = RSSL_MPF_NONE;
					linkMap.keyPrimitiveType = RSSL_DT_ASCII_STRING;
					linkMap.containerType = RSSL_DT_ELEMENT_LIST;
					if (!RSSL_ERROR_INFO_CHECK((ret = rsslEncodeMapInit(pEncodeIter, &linkMap, 0, 0)) == RSSL_RET_SUCCESS, ret, pError)) return RSSL_RET_FAILURE;

					for (j = 0; j < pService->linkInfo.linkCount; ++j)
					{
						RsslRDMServiceLink *pLink = &pService->linkInfo.linkList[j];

						rsslClearMapEntry(&linkMapEntry);
						linkMapEntry.flags = RSSL_MPEF_NONE;
						linkMapEntry.action = pLink->action;
						if (linkMapEntry.action != RSSL_MPEA_DELETE_ENTRY)
						{
							if (!RSSL_ERROR_INFO_CHECK((ret = rsslEncodeMapEntryInit(pEncodeIter, &linkMapEntry, &pLink->name, 0)) == RSSL_RET_SUCCESS, ret, pError)) return RSSL_RET_FAILURE;

							rsslClearElementList(&elementList);
							elementList.flags = RSSL_ELF_HAS_STANDARD_DATA;
							if (!RSSL_ERROR_INFO_CHECK((ret = rsslEncodeElementListInit(pEncodeIter, &elementList, 0, 0)) == RSSL_RET_SUCCESS, ret, pError)) return RSSL_RET_FAILURE;

							/* Type */
							if (pLink->flags & RDM_SVC_LKF_HAS_TYPE)
							{
								elementEntry.name = RSSL_ENAME_TYPE;
								elementEntry.dataType = RSSL_DT_UINT;
								if (!RSSL_ERROR_INFO_CHECK((ret = rsslEncodeElementEntry(pEncodeIter, &elementEntry, &pLink->type)) == RSSL_RET_SUCCESS, ret, pError)) return RSSL_RET_FAILURE;
							}

							/* LinkState */
							elementEntry.name = RSSL_ENAME_LINK_STATE;
							elementEntry.dataType = RSSL_DT_UINT;
							if (!RSSL_ERROR_INFO_CHECK((ret = rsslEncodeElementEntry(pEncodeIter, &elementEntry, &pLink->linkState)) == RSSL_RET_SUCCESS, ret, pError)) return RSSL_RET_FAILURE;

							/* Code */
							if (pLink->flags & RDM_SVC_LKF_HAS_CODE)
							{
								elementEntry.name = RSSL_ENAME_LINK_CODE;
								elementEntry.dataType = RSSL_DT_UINT;
								if (!RSSL_ERROR_INFO_CHECK((ret = rsslEncodeElementEntry(pEncodeIter, &elementEntry, &pLink->linkCode)) == RSSL_RET_SUCCESS, ret, pError)) return RSSL_RET_FAILURE;
							}

							/* Text */
							if (pLink->flags & RDM_SVC_LKF_HAS_TEXT)
							{
								elementEntry.name = RSSL_ENAME_TEXT;
								elementEntry.dataType = RSSL_DT_ASCII_STRING;
								if (!RSSL_ERROR_INFO_CHECK((ret = rsslEncodeElementEntry(pEncodeIter, &elementEntry, &pLink->text)) == RSSL_RET_SUCCESS, ret, pError)) return RSSL_RET_FAILURE;
							}

							if (!RSSL_ERROR_INFO_CHECK((ret = rsslEncodeElementListComplete(pEncodeIter, RSSL_TRUE)) == RSSL_RET_SUCCESS, ret, pError)) return RSSL_RET_FAILURE;

							if (!RSSL_ERROR_INFO_CHECK((ret = rsslEncodeMapEntryComplete(pEncodeIter, RSSL_TRUE)) == RSSL_RET_SUCCESS, ret, pError)) return RSSL_RET_FAILURE;
						}
						else
						{
							if (!RSSL_ERROR_INFO_CHECK((ret = rsslEncodeMapEntry(pEncodeIter, &linkMapEntry, &pLink->name)) == RSSL_RET_SUCCESS, ret, pError)) return RSSL_RET_FAILURE;
						}

					}

					if (!RSSL_ERROR_INFO_CHECK((ret = rsslEncodeMapComplete(pEncodeIter, RSSL_TRUE)) == RSSL_RET_SUCCESS, ret, pError)) return RSSL_RET_FAILURE;

					if (!RSSL_ERROR_INFO_CHECK((ret = rsslEncodeFilterEntryComplete(pEncodeIter, RSSL_TRUE)) == RSSL_RET_SUCCESS, ret, pError)) return RSSL_RET_FAILURE;
				}
				else
				{
					if (!RSSL_ERROR_INFO_CHECK((ret = rsslEncodeFilterEntry(pEncodeIter, &filterEntry)) == RSSL_RET_SUCCESS, ret, pError)) return RSSL_RET_FAILURE;
				}

			}

			/* Complete service encoding */
			if (!RSSL_ERROR_INFO_CHECK((ret = rsslEncodeFilterListComplete(pEncodeIter, RSSL_TRUE)) == RSSL_RET_SUCCESS, ret, pError)) return RSSL_RET_FAILURE;

			if (!RSSL_ERROR_INFO_CHECK((ret = rsslEncodeMapEntryComplete(pEncodeIter, RSSL_TRUE)) == RSSL_RET_SUCCESS, ret, pError)) return RSSL_RET_FAILURE;
		}
		else
		{
			if (!RSSL_ERROR_INFO_CHECK((ret = rsslEncodeMapEntry(pEncodeIter, &mapEntry, &pService->serviceId)) == RSSL_RET_SUCCESS, ret, pError)) return RSSL_RET_FAILURE;
		}
	}

	if (!RSSL_ERROR_INFO_CHECK((ret = rsslEncodeMapComplete(pEncodeIter, RSSL_TRUE)) == RSSL_RET_SUCCESS, ret, pError)) return RSSL_RET_FAILURE;

	return RSSL_RET_SUCCESS;
}

/* Decode service list payload. Assumes iterator is ready to decode map. */
static RsslRet _rsslDecodeServiceList(RsslDecodeIterator *pIter, RsslBuffer *pDataBody, RsslRDMDirectoryMsg *pDirectoryResponse, RsslBuffer *pMemoryBuffer, RsslErrorInfo *pError)
{
	RsslRet ret;
	RsslMap map;
	RsslMapEntry mapEntry;
	RsslFilterList filterList;
	RsslFilterEntry filterEntry;
	RsslUInt serviceId;
	RsslRDMService *pService;
	RsslDecodeIterator tmpCountIter;

	RsslUInt32 serviceCount;
	RsslRDMService *pServices;

	rsslClearDecodeIterator(&tmpCountIter);
	rsslSetDecodeIteratorBuffer(&tmpCountIter, pDataBody);
	rsslSetDecodeIteratorRWFVersion(&tmpCountIter, pIter->_majorVersion, pIter->_minorVersion);

	/* Count up how many map entries there are so we can figure out how much memory to reserve for services array */
	if (!RSSL_ERROR_INFO_CHECK((ret = rsslDecodeMap(&tmpCountIter, &map)) == RSSL_RET_SUCCESS, ret, pError)) return RSSL_RET_FAILURE;

	if (!RSSL_ERROR_INFO_CHECK(map.containerType == RSSL_DT_FILTER_LIST && map.keyPrimitiveType == RSSL_DT_UINT, RSSL_RET_FAILURE, pError)) return RSSL_RET_FAILURE;

	serviceCount = 0;
	while ((ret = rsslDecodeMapEntry(&tmpCountIter, &mapEntry, &serviceId)) != RSSL_RET_END_OF_CONTAINER)
	{
		if (!RSSL_ERROR_INFO_CHECK(ret == RSSL_RET_SUCCESS, ret, pError)) return RSSL_RET_FAILURE;
		++serviceCount;
	}

	/* Reserve memory and decode services */

	if (!RSSL_ERROR_INFO_CHECK(pService = pServices = (RsslRDMService*)rsslReserveAlignedBufferMemory(pMemoryBuffer, serviceCount, sizeof(RsslRDMService)), RSSL_RET_BUFFER_TOO_SMALL, pError)) return RSSL_RET_BUFFER_TOO_SMALL;

	if (!RSSL_ERROR_INFO_CHECK((ret = rsslDecodeMap(pIter, &map)) == RSSL_RET_SUCCESS, ret, pError)) return RSSL_RET_FAILURE;

	if (!RSSL_ERROR_INFO_CHECK(map.containerType == RSSL_DT_FILTER_LIST && map.keyPrimitiveType == RSSL_DT_UINT, RSSL_RET_FAILURE, pError)) return RSSL_RET_FAILURE;

	while ((ret = rsslDecodeMapEntry(pIter, &mapEntry, &serviceId)) != RSSL_RET_END_OF_CONTAINER)
	{
		RsslRDMServiceGroupState *pGroupState;

		if (!RSSL_ERROR_INFO_CHECK(ret == RSSL_RET_SUCCESS, ret, pError)) return RSSL_RET_FAILURE;

		rsslClearRDMService(pService);

		/* Found one service. Decode it to get its details */

		pService->serviceId = serviceId;
		pService->action = (RsslMapEntryActions)mapEntry.action;

		if (mapEntry.action != RSSL_MPEA_DELETE_ENTRY)
		{
			rsslClearFilterList(&filterList);
			if (!RSSL_ERROR_INFO_CHECK((ret = rsslDecodeFilterList(pIter, &filterList)) == RSSL_RET_SUCCESS, ret, pError)) return RSSL_RET_FAILURE;

			/* Count up how many group status entries there are so we can figure out how much memory to reserve for them */
			rsslClearDecodeIterator(&tmpCountIter);
			rsslSetDecodeIteratorBuffer(&tmpCountIter, &mapEntry.encData);
			rsslSetDecodeIteratorRWFVersion(&tmpCountIter, pIter->_majorVersion, pIter->_minorVersion);
			if (!RSSL_ERROR_INFO_CHECK((ret = rsslDecodeFilterList(&tmpCountIter, &filterList)) == RSSL_RET_SUCCESS, ret, pError)) return RSSL_RET_FAILURE;
			while ((ret = rsslDecodeFilterEntry(&tmpCountIter, &filterEntry)) != RSSL_RET_END_OF_CONTAINER)
			{
				if (!RSSL_ERROR_INFO_CHECK(ret == RSSL_RET_SUCCESS, ret, pError)) return RSSL_RET_FAILURE;
				if (filterEntry.id == RDM_DIRECTORY_SERVICE_GROUP_ID)
					++pService->groupStateCount;
			}
			if (!RSSL_ERROR_INFO_CHECK(pGroupState = pService->groupStateList = (RsslRDMServiceGroupState*)rsslReserveAlignedBufferMemory(pMemoryBuffer, pService->groupStateCount, sizeof(RsslRDMServiceGroupState)), RSSL_RET_BUFFER_TOO_SMALL, pError)) return RSSL_RET_BUFFER_TOO_SMALL;

			rsslClearDecodeIterator(&tmpCountIter);
			rsslSetDecodeIteratorBuffer(&tmpCountIter, &mapEntry.encData);
			rsslSetDecodeIteratorRWFVersion(&tmpCountIter, pIter->_majorVersion, pIter->_minorVersion);


			rsslClearFilterEntry(&filterEntry);
			while ((ret = rsslDecodeFilterEntry(pIter, &filterEntry)) != RSSL_RET_END_OF_CONTAINER)
			{
				if (!RSSL_ERROR_INFO_CHECK(ret == RSSL_RET_SUCCESS, ret, pError)) return RSSL_RET_FAILURE;

				switch(filterEntry.id)
				{
					case RDM_DIRECTORY_SERVICE_INFO_ID:
						{
							RsslElementList elementList;
							RsslElementEntry elementEntry;

							/* Required elements */
							RsslBool foundServiceInfoName = RSSL_FALSE,
									 foundServiceInfoCapabilities = RSSL_FALSE;

							pService->flags |= RDM_SVCF_HAS_INFO;
							pService->info.action = (RsslFilterEntryActions)filterEntry.action;

							if (filterEntry.action == RSSL_FTEA_CLEAR_ENTRY)
								break;

							if (!RSSL_ERROR_INFO_CHECK(filterEntry.containerType == RSSL_DT_ELEMENT_LIST, RSSL_RET_FAILURE, pError)) return RSSL_RET_FAILURE;

							rsslClearElementList(&elementList);
							if (!RSSL_ERROR_INFO_CHECK((ret = rsslDecodeElementList(pIter, &elementList, 0)) == RSSL_RET_SUCCESS, ret, pError)) return RSSL_RET_FAILURE;



							rsslClearElementEntry(&elementEntry);

							while ((ret = rsslDecodeElementEntry(pIter, &elementEntry)) != RSSL_RET_END_OF_CONTAINER)
							{
								if (!RSSL_ERROR_INFO_CHECK(ret == RSSL_RET_SUCCESS, ret, pError)) return RSSL_RET_FAILURE;

								if (rsslBufferIsEqual(&elementEntry.name, &RSSL_ENAME_NAME))
								{
									if (!RSSL_ERROR_INFO_CHECK(elementEntry.dataType == RSSL_DT_ASCII_STRING || elementEntry.dataType == RSSL_DT_BUFFER, RSSL_RET_FAILURE, pError)) return RSSL_RET_FAILURE;

									pService->info.serviceName = elementEntry.encData;
									foundServiceInfoName = RSSL_TRUE;
								}
								else if (rsslBufferIsEqual(&elementEntry.name, &RSSL_ENAME_VENDOR))
								{
									pService->info.flags |= RDM_SVC_IFF_HAS_VENDOR;
									if (!RSSL_ERROR_INFO_CHECK(elementEntry.dataType == RSSL_DT_ASCII_STRING || elementEntry.dataType == RSSL_DT_BUFFER, RSSL_RET_FAILURE, pError)) return RSSL_RET_FAILURE;

									pService->info.vendor = elementEntry.encData;
								}
								else if (rsslBufferIsEqual(&elementEntry.name, &RSSL_ENAME_IS_SOURCE))
								{
									pService->info.flags |= RDM_SVC_IFF_HAS_IS_SOURCE;
									if (!RSSL_ERROR_INFO_CHECK(elementEntry.dataType == RSSL_DT_UINT, RSSL_RET_FAILURE, pError)) return RSSL_RET_FAILURE;

									if (!RSSL_ERROR_INFO_CHECK((ret = rsslDecodeUInt(pIter, &pService->info.isSource)) == RSSL_RET_SUCCESS, ret, pError)) return RSSL_RET_FAILURE;
								}
								else if (rsslBufferIsEqual(&elementEntry.name, &RSSL_ENAME_CAPABILITIES))
								{
									RsslArray array;
									RsslBuffer arrayEntry;
									RsslUInt *pCapability;

									if (!RSSL_ERROR_INFO_CHECK(elementEntry.dataType == RSSL_DT_ARRAY, RSSL_RET_FAILURE, pError)) return RSSL_RET_FAILURE;

									if (!RSSL_ERROR_INFO_CHECK((ret = rsslDecodeArray(pIter, &array)) == RSSL_RET_SUCCESS, ret, pError)) return RSSL_RET_FAILURE;

									if (!RSSL_ERROR_INFO_CHECK(array.primitiveType == RSSL_DT_UINT, RSSL_RET_FAILURE, pError)) return RSSL_RET_FAILURE;

									rsslClearBuffer(&arrayEntry);
									if ((ret = rsslDecodeArrayEntry(pIter, &arrayEntry)) != RSSL_RET_END_OF_CONTAINER)
									{
										/* Align buffer so we can start an array */
										if (!RSSL_ERROR_INFO_CHECK(pService->info.capabilitiesList = (RsslUInt*)rsslReserveAlignedBufferMemory(pMemoryBuffer, 0, sizeof(RsslUInt)), RSSL_RET_BUFFER_TOO_SMALL, pError)) return RSSL_RET_BUFFER_TOO_SMALL;

										do
										{
											if (!RSSL_ERROR_INFO_CHECK(ret == RSSL_RET_SUCCESS, ret, pError)) return RSSL_RET_FAILURE;

											if (!RSSL_ERROR_INFO_CHECK(pCapability = (RsslUInt*)rsslReserveBufferMemory(pMemoryBuffer, 1, sizeof(RsslUInt)), RSSL_RET_BUFFER_TOO_SMALL, pError)) return RSSL_RET_BUFFER_TOO_SMALL;
											if (!RSSL_ERROR_INFO_CHECK((ret = rsslDecodeUInt(pIter, pCapability)) == RSSL_RET_SUCCESS, ret, pError)) return RSSL_RET_FAILURE;
											++pService->info.capabilitiesCount;

											rsslClearBuffer(&arrayEntry);
										} while ((ret = rsslDecodeArrayEntry(pIter, &arrayEntry)) != RSSL_RET_END_OF_CONTAINER);
									}

									foundServiceInfoCapabilities = RSSL_TRUE;
								}
								else if (rsslBufferIsEqual(&elementEntry.name, &RSSL_ENAME_DICTIONARIES_PROVIDED))
								{
									RsslArray array;
									RsslBuffer arrayEntry;
									RsslBuffer *pDictionary;

									pService->info.flags |= RDM_SVC_IFF_HAS_DICTS_PROVIDED;

									if (!RSSL_ERROR_INFO_CHECK(elementEntry.dataType == RSSL_DT_ARRAY, RSSL_RET_FAILURE, pError)) return RSSL_RET_FAILURE;

									if (!RSSL_ERROR_INFO_CHECK((ret = rsslDecodeArray(pIter, &array)) == RSSL_RET_SUCCESS, ret, pError)) return RSSL_RET_FAILURE;

									if (!RSSL_ERROR_INFO_CHECK(array.primitiveType == RSSL_DT_ASCII_STRING || array.primitiveType == RSSL_DT_BUFFER , RSSL_RET_FAILURE, pError)) return RSSL_RET_FAILURE;

									rsslClearBuffer(&arrayEntry);
									if ((ret = rsslDecodeArrayEntry(pIter, &arrayEntry)) != RSSL_RET_END_OF_CONTAINER)
									{
										/* Align buffer so we can start an array */
										if (!RSSL_ERROR_INFO_CHECK(pService->info.dictionariesProvidedList = (RsslBuffer*)rsslReserveAlignedBufferMemory(pMemoryBuffer, 0, sizeof(RsslBuffer)), RSSL_RET_BUFFER_TOO_SMALL, pError)) return RSSL_RET_BUFFER_TOO_SMALL;

										do
										{
											if (!RSSL_ERROR_INFO_CHECK(ret == RSSL_RET_SUCCESS, ret, pError)) return RSSL_RET_FAILURE;

											if (!RSSL_ERROR_INFO_CHECK(pDictionary = (RsslBuffer*)rsslReserveBufferMemory(pMemoryBuffer, 1, sizeof(RsslBuffer)), RSSL_RET_BUFFER_TOO_SMALL, pError)) return RSSL_RET_BUFFER_TOO_SMALL;
											*pDictionary = arrayEntry;
											++pService->info.dictionariesProvidedCount;

											rsslClearBuffer(&arrayEntry);
										} while ((ret = rsslDecodeArrayEntry(pIter, &arrayEntry)) != RSSL_RET_END_OF_CONTAINER);
									}

								}
								else if (rsslBufferIsEqual(&elementEntry.name, &RSSL_ENAME_DICTIONARIES_USED))
								{
									RsslArray array;
									RsslBuffer arrayEntry;
									RsslBuffer *pDictionary;

									pService->info.flags |= RDM_SVC_IFF_HAS_DICTS_USED;

									if (!RSSL_ERROR_INFO_CHECK(elementEntry.dataType == RSSL_DT_ARRAY, RSSL_RET_FAILURE, pError)) return RSSL_RET_FAILURE;

									if (!RSSL_ERROR_INFO_CHECK((ret = rsslDecodeArray(pIter, &array)) == RSSL_RET_SUCCESS, ret, pError)) return RSSL_RET_FAILURE;

									if (!RSSL_ERROR_INFO_CHECK(array.primitiveType == RSSL_DT_ASCII_STRING || array.primitiveType == RSSL_DT_BUFFER , RSSL_RET_FAILURE, pError)) return RSSL_RET_FAILURE;

									rsslClearBuffer(&arrayEntry);
									if ((ret = rsslDecodeArrayEntry(pIter, &arrayEntry)) != RSSL_RET_END_OF_CONTAINER)
									{
										/* Align buffer so we can start an array */
										if (!RSSL_ERROR_INFO_CHECK(pService->info.dictionariesUsedList = (RsslBuffer*)rsslReserveAlignedBufferMemory(pMemoryBuffer, 0, sizeof(RsslBuffer)), RSSL_RET_BUFFER_TOO_SMALL, pError)) return RSSL_RET_BUFFER_TOO_SMALL;

										do
										{
											if (!RSSL_ERROR_INFO_CHECK(ret == RSSL_RET_SUCCESS, ret, pError)) return RSSL_RET_FAILURE;

											if (!RSSL_ERROR_INFO_CHECK(pDictionary = (RsslBuffer*)rsslReserveBufferMemory(pMemoryBuffer, 1, sizeof(RsslBuffer)), RSSL_RET_BUFFER_TOO_SMALL, pError)) return RSSL_RET_BUFFER_TOO_SMALL;
											*pDictionary = arrayEntry;
											++pService->info.dictionariesUsedCount;

											rsslClearBuffer(&arrayEntry);
										} while ((ret = rsslDecodeArrayEntry(pIter, &arrayEntry)) != RSSL_RET_END_OF_CONTAINER);
									}

								}
								else if (rsslBufferIsEqual(&elementEntry.name, &RSSL_ENAME_QOS))
								{
									RsslArray array;
									RsslBuffer arrayEntry;
									RsslQos *pQos;

									pService->info.flags |= RDM_SVC_IFF_HAS_QOS;

									if (!RSSL_ERROR_INFO_CHECK(elementEntry.dataType == RSSL_DT_ARRAY, RSSL_RET_FAILURE, pError)) return RSSL_RET_FAILURE;

									if (!RSSL_ERROR_INFO_CHECK((ret = rsslDecodeArray(pIter, &array)) == RSSL_RET_SUCCESS, ret, pError)) return RSSL_RET_FAILURE;

									if (!RSSL_ERROR_INFO_CHECK(array.primitiveType == RSSL_DT_QOS, RSSL_RET_FAILURE, pError)) return RSSL_RET_FAILURE;

									rsslClearBuffer(&arrayEntry);
									if ((ret = rsslDecodeArrayEntry(pIter, &arrayEntry)) != RSSL_RET_END_OF_CONTAINER)
									{
										/* Align buffer so we can start an array */
										if (!RSSL_ERROR_INFO_CHECK(pService->info.qosList = (RsslQos*)rsslReserveAlignedBufferMemory(pMemoryBuffer, 0, sizeof(RsslQos)), RSSL_RET_BUFFER_TOO_SMALL, pError)) return RSSL_RET_BUFFER_TOO_SMALL;

										do
										{
											if (!RSSL_ERROR_INFO_CHECK(ret == RSSL_RET_SUCCESS, ret, pError)) return RSSL_RET_FAILURE;

											if (!RSSL_ERROR_INFO_CHECK(pQos = (RsslQos*)rsslReserveBufferMemory(pMemoryBuffer, 1, sizeof(RsslQos)), RSSL_RET_BUFFER_TOO_SMALL, pError)) return RSSL_RET_BUFFER_TOO_SMALL;
											if (!RSSL_ERROR_INFO_CHECK((ret = rsslDecodeQos(pIter, pQos)) == RSSL_RET_SUCCESS, ret, pError)) return RSSL_RET_FAILURE;
											++pService->info.qosCount;

											rsslClearBuffer(&arrayEntry);
										} while ((ret = rsslDecodeArrayEntry(pIter, &arrayEntry)) != RSSL_RET_END_OF_CONTAINER);
									}
								}
								else if (rsslBufferIsEqual(&elementEntry.name, &RSSL_ENAME_SUPPS_QOS_RANGE))
								{
									pService->info.flags |= RDM_SVC_IFF_HAS_SUPPORT_QOS_RANGE;

									if (!RSSL_ERROR_INFO_CHECK(elementEntry.dataType == RSSL_DT_UINT, RSSL_RET_FAILURE, pError)) return RSSL_RET_FAILURE;

									if (!RSSL_ERROR_INFO_CHECK((ret = rsslDecodeUInt(pIter, &pService->info.supportsQosRange)) == RSSL_RET_SUCCESS, ret, pError)) return RSSL_RET_FAILURE;
								}
								else if (rsslBufferIsEqual(&elementEntry.name, &RSSL_ENAME_ITEM_LIST))
								{
									pService->info.flags |= RDM_SVC_IFF_HAS_ITEM_LIST;

									if (!RSSL_ERROR_INFO_CHECK(elementEntry.dataType == RSSL_DT_ASCII_STRING || elementEntry.dataType == RSSL_DT_BUFFER , RSSL_RET_FAILURE, pError)) return RSSL_RET_FAILURE;
									pService->info.itemList  = elementEntry.encData;
								}
								else if (rsslBufferIsEqual(&elementEntry.name, &RSSL_ENAME_SUPPS_OOB_SNAPSHOTS))
								{
									pService->info.flags |= RDM_SVC_IFF_HAS_SUPPORT_OOB_SNAPSHOTS;

									if (!RSSL_ERROR_INFO_CHECK(elementEntry.dataType == RSSL_DT_UINT, RSSL_RET_FAILURE, pError)) return RSSL_RET_FAILURE;

									if (!RSSL_ERROR_INFO_CHECK((ret = rsslDecodeUInt(pIter, &pService->info.supportsOutOfBandSnapshots)) == RSSL_RET_SUCCESS, ret, pError)) return RSSL_RET_FAILURE;
								}
								else if (rsslBufferIsEqual(&elementEntry.name, &RSSL_ENAME_ACCEPTING_CONS_STATUS))
								{
									pService->info.flags |= RDM_SVC_IFF_HAS_ACCEPTING_CONS_STATUS;
									if (!RSSL_ERROR_INFO_CHECK(elementEntry.dataType == RSSL_DT_UINT, RSSL_RET_FAILURE, pError)) return RSSL_RET_FAILURE;

									if (!RSSL_ERROR_INFO_CHECK((ret = rsslDecodeUInt(pIter, &pService->info.acceptingConsumerStatus)) == RSSL_RET_SUCCESS, ret, pError)) return RSSL_RET_FAILURE;
								}

							}

							if (!RSSL_ERROR_INFO_CHECK(foundServiceInfoName, RSSL_RET_FAILURE, pError)) return RSSL_RET_FAILURE;
							if (!RSSL_ERROR_INFO_CHECK(foundServiceInfoCapabilities, RSSL_RET_FAILURE, pError)) return RSSL_RET_FAILURE;
							break;
						}

					case RDM_DIRECTORY_SERVICE_STATE_ID:
						{
							RsslElementList elementList;
							RsslElementEntry elementEntry;

							/* Required */
							RsslBool foundServiceState = RSSL_FALSE;

							pService->flags |= RDM_SVCF_HAS_STATE;
							pService->state.action = (RsslFilterEntryActions)filterEntry.action;

							if (filterEntry.action == RSSL_FTEA_CLEAR_ENTRY)
								break;

							if (!RSSL_ERROR_INFO_CHECK(filterEntry.containerType == RSSL_DT_ELEMENT_LIST, RSSL_RET_FAILURE, pError)) return RSSL_RET_FAILURE;

							rsslClearElementList(&elementList);
							if (!RSSL_ERROR_INFO_CHECK((ret = rsslDecodeElementList(pIter, &elementList, 0)) == RSSL_RET_SUCCESS, ret, pError)) return RSSL_RET_FAILURE;

							rsslClearElementEntry(&elementEntry);

							while ((ret = rsslDecodeElementEntry(pIter, &elementEntry)) != RSSL_RET_END_OF_CONTAINER)
							{
								if (!RSSL_ERROR_INFO_CHECK(ret == RSSL_RET_SUCCESS, ret, pError)) return RSSL_RET_FAILURE;

								if (rsslBufferIsEqual(&elementEntry.name, &RSSL_ENAME_SVC_STATE))
								{
									if (!RSSL_ERROR_INFO_CHECK(elementEntry.dataType == RSSL_DT_UINT, RSSL_RET_FAILURE, pError)) return RSSL_RET_FAILURE;

									if (!RSSL_ERROR_INFO_CHECK((ret = rsslDecodeUInt(pIter, &pService->state.serviceState)) == RSSL_RET_SUCCESS, ret, pError)) return RSSL_RET_FAILURE;
									foundServiceState = RSSL_TRUE;
								}
								else if (rsslBufferIsEqual(&elementEntry.name, &RSSL_ENAME_ACCEPTING_REQS))
								{
									pService->state.flags |= RDM_SVC_STF_HAS_ACCEPTING_REQS;
									if (!RSSL_ERROR_INFO_CHECK(elementEntry.dataType == RSSL_DT_UINT, RSSL_RET_FAILURE, pError)) return RSSL_RET_FAILURE;

									if (!RSSL_ERROR_INFO_CHECK((ret = rsslDecodeUInt(pIter, &pService->state.acceptingRequests)) == RSSL_RET_SUCCESS, ret, pError)) return RSSL_RET_FAILURE;
								}
								else if (rsslBufferIsEqual(&elementEntry.name, &RSSL_ENAME_STATUS))
								{
									pService->state.flags |= RDM_SVC_STF_HAS_STATUS;
									if (!RSSL_ERROR_INFO_CHECK((ret = rsslDecodeState(pIter, &pService->state.status)) == RSSL_RET_SUCCESS, ret, pError)) return RSSL_RET_FAILURE;

								}


							}

							if (!RSSL_ERROR_INFO_CHECK(foundServiceState, RSSL_RET_FAILURE, pError)) return RSSL_RET_FAILURE;
							break;
						}

					case RDM_DIRECTORY_SERVICE_GROUP_ID:
						{
							RsslElementList elementList;
							RsslElementEntry elementEntry;

							/* Required */
							RsslBool foundServiceGroupId = RSSL_FALSE;

							rsslClearRDMServiceGroupState(pGroupState);

							pGroupState->action = (RsslFilterEntryActions)filterEntry.action;
							if (filterEntry.action != RSSL_FTEA_CLEAR_ENTRY)
							{
								if (!RSSL_ERROR_INFO_CHECK(filterEntry.containerType == RSSL_DT_ELEMENT_LIST, RSSL_RET_FAILURE, pError)) return RSSL_RET_FAILURE;

								rsslClearElementList(&elementList);
								if (!RSSL_ERROR_INFO_CHECK((ret = rsslDecodeElementList(pIter, &elementList, 0)) == RSSL_RET_SUCCESS, ret, pError)) return RSSL_RET_FAILURE;


								rsslClearElementEntry(&elementEntry);

								while ((ret = rsslDecodeElementEntry(pIter, &elementEntry)) != RSSL_RET_END_OF_CONTAINER)
								{
									if (!RSSL_ERROR_INFO_CHECK(ret == RSSL_RET_SUCCESS, ret, pError)) return RSSL_RET_FAILURE;

									if (rsslBufferIsEqual(&elementEntry.name, &RSSL_ENAME_GROUP))
									{
										if (!RSSL_ERROR_INFO_CHECK(elementEntry.dataType == RSSL_DT_BUFFER, RSSL_RET_FAILURE, pError)) return RSSL_RET_FAILURE;

										pGroupState->group = elementEntry.encData;
										foundServiceGroupId = RSSL_TRUE;
									}
									else if (rsslBufferIsEqual(&elementEntry.name, &RSSL_ENAME_MERG_TO_GRP))
									{
										pGroupState->flags |= RDM_SVC_GRF_HAS_MERGED_TO_GROUP;
										if (!RSSL_ERROR_INFO_CHECK(elementEntry.dataType == RSSL_DT_BUFFER, RSSL_RET_FAILURE, pError)) return RSSL_RET_FAILURE;

										pGroupState->mergedToGroup = elementEntry.encData;
									}
									else if (rsslBufferIsEqual(&elementEntry.name, &RSSL_ENAME_STATUS))
									{
										if (!RSSL_ERROR_INFO_CHECK(elementEntry.dataType == RSSL_DT_STATE, RSSL_RET_FAILURE, pError)) return RSSL_RET_FAILURE;
										pGroupState->flags |= RDM_SVC_GRF_HAS_STATUS;
										if (!RSSL_ERROR_INFO_CHECK((ret = rsslDecodeState(pIter, &pGroupState->status)) == RSSL_RET_SUCCESS, ret, pError)) return RSSL_RET_FAILURE;
									}


								}

								if (!RSSL_ERROR_INFO_CHECK(foundServiceGroupId, RSSL_RET_FAILURE, pError)) return RSSL_RET_FAILURE;
							}
							++pGroupState;
							break;
						}

					case RDM_DIRECTORY_SERVICE_LOAD_ID:
						{
							RsslElementList elementList;
							RsslElementEntry elementEntry;
							RsslRDMServiceLoad *pLoad = &pService->load;

							pService->flags |= RDM_SVCF_HAS_LOAD;
							pLoad->action = (RsslFilterEntryActions)filterEntry.action;

							if (filterEntry.action == RSSL_FTEA_CLEAR_ENTRY)
								break;

							if (!RSSL_ERROR_INFO_CHECK(filterEntry.containerType == RSSL_DT_ELEMENT_LIST, RSSL_RET_FAILURE, pError)) return RSSL_RET_FAILURE;

							rsslClearElementList(&elementList);
							if (!RSSL_ERROR_INFO_CHECK((ret = rsslDecodeElementList(pIter, &elementList, 0)) == RSSL_RET_SUCCESS, ret, pError)) return RSSL_RET_FAILURE;

							rsslClearElementEntry(&elementEntry);

							while ((ret = rsslDecodeElementEntry(pIter, &elementEntry)) != RSSL_RET_END_OF_CONTAINER)
							{
								if (!RSSL_ERROR_INFO_CHECK(ret == RSSL_RET_SUCCESS, ret, pError)) return RSSL_RET_FAILURE;

								if (rsslBufferIsEqual(&elementEntry.name, &RSSL_ENAME_OPEN_LIMIT))
								{
									pLoad->flags |= RDM_SVC_LDF_HAS_OPEN_LIMIT;
									if (!RSSL_ERROR_INFO_CHECK(elementEntry.dataType == RSSL_DT_UINT, RSSL_RET_FAILURE, pError)) return RSSL_RET_FAILURE;

									if (!RSSL_ERROR_INFO_CHECK((ret = rsslDecodeUInt(pIter, &pLoad->openLimit)) == RSSL_RET_SUCCESS, ret, pError)) return RSSL_RET_FAILURE;
								}
								else if (rsslBufferIsEqual(&elementEntry.name, &RSSL_ENAME_OPEN_WINDOW))
								{
									pLoad->flags |= RDM_SVC_LDF_HAS_OPEN_WINDOW;
									if (!RSSL_ERROR_INFO_CHECK(elementEntry.dataType == RSSL_DT_UINT, RSSL_RET_FAILURE, pError)) return RSSL_RET_FAILURE;

									if (!RSSL_ERROR_INFO_CHECK((ret = rsslDecodeUInt(pIter, &pLoad->openWindow)) == RSSL_RET_SUCCESS, ret, pError)) return RSSL_RET_FAILURE;
								}
								else if (rsslBufferIsEqual(&elementEntry.name, &RSSL_ENAME_LOAD_FACT))
								{
									pLoad->flags |= RDM_SVC_LDF_HAS_LOAD_FACTOR;
									if (!RSSL_ERROR_INFO_CHECK(elementEntry.dataType == RSSL_DT_UINT, RSSL_RET_FAILURE, pError)) return RSSL_RET_FAILURE;

									if (!RSSL_ERROR_INFO_CHECK((ret = rsslDecodeUInt(pIter, &pLoad->loadFactor)) == RSSL_RET_SUCCESS, ret, pError)) return RSSL_RET_FAILURE;
								}

							}
							break;
						}

					case RDM_DIRECTORY_SERVICE_DATA_ID:
						{
							RsslElementList elementList;
							RsslElementEntry elementEntry;
							RsslRDMServiceData *pData = &pService->data;

							pService->flags |= RDM_SVCF_HAS_DATA;
							pData->action = (RsslFilterEntryActions)filterEntry.action;

							if (filterEntry.action == RSSL_FTEA_CLEAR_ENTRY)
								break;

							if (!RSSL_ERROR_INFO_CHECK(filterEntry.containerType == RSSL_DT_ELEMENT_LIST, RSSL_RET_FAILURE, pError)) return RSSL_RET_FAILURE;


							rsslClearElementList(&elementList);
							if (!RSSL_ERROR_INFO_CHECK((ret = rsslDecodeElementList(pIter, &elementList, 0)) == RSSL_RET_SUCCESS, ret, pError)) return RSSL_RET_FAILURE;

							rsslClearElementEntry(&elementEntry);

							while ((ret = rsslDecodeElementEntry(pIter, &elementEntry)) != RSSL_RET_END_OF_CONTAINER)
							{
								RsslBool foundType = RSSL_FALSE, foundData = RSSL_FALSE;

								if (!RSSL_ERROR_INFO_CHECK(ret == RSSL_RET_SUCCESS, ret, pError)) return RSSL_RET_FAILURE;

								if (rsslBufferIsEqual(&elementEntry.name, &RSSL_ENAME_TYPE))
								{
									if (!RSSL_ERROR_INFO_CHECK(elementEntry.dataType == RSSL_DT_UINT, RSSL_RET_FAILURE, pError)) return RSSL_RET_FAILURE;

									if (!RSSL_ERROR_INFO_CHECK((ret = rsslDecodeUInt(pIter, &pData->type)) == RSSL_RET_SUCCESS, ret, pError)) return RSSL_RET_FAILURE;
									foundType = RSSL_TRUE;
								}
								else if (rsslBufferIsEqual(&elementEntry.name, &RSSL_ENAME_DATA))
								{
									pData->flags |= RDM_SVC_DTF_HAS_DATA;
									pData->dataType = (RsslDataTypes)elementEntry.dataType;
									pData->data = elementEntry.encData;
									foundData = RSSL_TRUE;
								}

								/* If Data element is present, type must be too. */
								RSSL_ERROR_INFO_CHECK(!foundData || foundType, RSSL_RET_FAILURE, pError);
							}
							break;
						}
					case RDM_DIRECTORY_SERVICE_SEQ_MCAST_ID:
						{
							RsslVector vector;
							RsslVectorEntry vectorEntry;
							RsslElementList elementList;
							RsslElementEntry elementEntry;
							RsslLocalElementSetDefDb elementSetDefDb;

							RsslBool foundSnapshotPort = RSSL_FALSE, foundSnapshotAddr = RSSL_FALSE;
							RsslBool foundGapRecPort = RSSL_FALSE, foundGapRecAddr = RSSL_FALSE;
							RsslBool foundRefDataPort = RSSL_FALSE, foundRefDataAddr = RSSL_FALSE;

							pService->flags |= RDM_SVCF_HAS_SEQ_MCAST;
							pService->seqMCastInfo.action = (RsslFilterEntryActions)filterEntry.action;

							if (filterEntry.action == RSSL_FTEA_CLEAR_ENTRY)
								break;

							if (!RSSL_ERROR_INFO_CHECK(filterEntry.containerType == RSSL_DT_ELEMENT_LIST, RSSL_RET_FAILURE, pError)) return RSSL_RET_FAILURE;

							rsslClearElementList(&elementList);
							if (!RSSL_ERROR_INFO_CHECK((ret = rsslDecodeElementList(pIter, &elementList, 0)) == RSSL_RET_SUCCESS, ret, pError)) return RSSL_RET_FAILURE;

							rsslClearElementEntry(&elementEntry);

							while ((ret = rsslDecodeElementEntry(pIter, &elementEntry)) != RSSL_RET_END_OF_CONTAINER)
							{
								if (!RSSL_ERROR_INFO_CHECK(ret == RSSL_RET_SUCCESS, ret, pError)) return RSSL_RET_FAILURE;

								if (rsslBufferIsEqual(&elementEntry.name, &RSSL_ENAME_SNAPSHOT_SERVER_HOST))
								{
									if (!RSSL_ERROR_INFO_CHECK(elementEntry.dataType == RSSL_DT_ASCII_STRING || elementEntry.dataType == RSSL_DT_BUFFER, RSSL_RET_FAILURE, pError)) return RSSL_RET_FAILURE;

									pService->seqMCastInfo.snapshotServer.address = elementEntry.encData;

									foundSnapshotAddr = RSSL_TRUE;
								}
								else if (rsslBufferIsEqual(&elementEntry.name, &RSSL_ENAME_SNAPSHOT_SERVER_PORT))
								{
									if (!RSSL_ERROR_INFO_CHECK(elementEntry.dataType == RSSL_DT_UINT, RSSL_RET_FAILURE, pError)) return RSSL_RET_FAILURE;

									if (!RSSL_ERROR_INFO_CHECK((ret = rsslDecodeUInt(pIter, &pService->seqMCastInfo.snapshotServer.port)) == RSSL_RET_SUCCESS, ret, pError)) return RSSL_RET_FAILURE;

									foundSnapshotPort = RSSL_TRUE;
								}
								else if (rsslBufferIsEqual(&elementEntry.name, &RSSL_ENAME_GAP_RECOVERY_SERVER_HOST))
								{
									if (!RSSL_ERROR_INFO_CHECK(elementEntry.dataType == RSSL_DT_ASCII_STRING || elementEntry.dataType == RSSL_DT_BUFFER, RSSL_RET_FAILURE, pError)) return RSSL_RET_FAILURE;

									pService->seqMCastInfo.gapRecoveryServer.address = elementEntry.encData;

									foundGapRecAddr = RSSL_TRUE;
								}
								else if (rsslBufferIsEqual(&elementEntry.name, &RSSL_ENAME_GAP_RECOVERY_SERVER_PORT))
								{
									if (!RSSL_ERROR_INFO_CHECK(elementEntry.dataType == RSSL_DT_UINT, RSSL_RET_FAILURE, pError)) return RSSL_RET_FAILURE;

									if (!RSSL_ERROR_INFO_CHECK((ret = rsslDecodeUInt(pIter, &pService->seqMCastInfo.gapRecoveryServer.port)) == RSSL_RET_SUCCESS, ret, pError)) return RSSL_RET_FAILURE;

									foundGapRecPort = RSSL_TRUE;
								}
								else if (rsslBufferIsEqual(&elementEntry.name, &RSSL_ENAME_REFERENCE_DATA_SERVER_HOST))
								{
									if (!RSSL_ERROR_INFO_CHECK(elementEntry.dataType == RSSL_DT_ASCII_STRING || elementEntry.dataType == RSSL_DT_BUFFER, RSSL_RET_FAILURE, pError)) return RSSL_RET_FAILURE;

									pService->seqMCastInfo.refDataServer.address = elementEntry.encData;

									foundRefDataAddr = RSSL_TRUE;
								}
								else if (rsslBufferIsEqual(&elementEntry.name, &RSSL_ENAME_REFERENCE_DATA_SERVER_PORT))
								{
									if (!RSSL_ERROR_INFO_CHECK(elementEntry.dataType == RSSL_DT_UINT, RSSL_RET_FAILURE, pError)) return RSSL_RET_FAILURE;

									if (!RSSL_ERROR_INFO_CHECK((ret = rsslDecodeUInt(pIter, &pService->seqMCastInfo.refDataServer.port)) == RSSL_RET_SUCCESS, ret, pError)) return RSSL_RET_FAILURE;

									foundRefDataPort = RSSL_TRUE;
								}
								else if (rsslBufferIsEqual(&elementEntry.name, &RSSL_ENAME_STREAMING_MCAST_CHANNELS))
								{
									RsslRDMMCAddressPortInfo *channelInfoElem;

									rsslClearVector(&vector);
									if (!RSSL_ERROR_INFO_CHECK((ret = rsslDecodeVector(pIter, &vector)) == RSSL_RET_SUCCESS, ret, pError)) return RSSL_RET_FAILURE;

									if (!RSSL_ERROR_INFO_CHECK(vector.containerType == RSSL_DT_ELEMENT_LIST, RSSL_RET_FAILURE, pError)) return RSSL_RET_FAILURE;

									if (vector.flags & RSSL_VTF_HAS_SET_DEFS)
									{
										rsslClearLocalElementSetDefDb(&elementSetDefDb);
										if (!RSSL_ERROR_INFO_CHECK((ret = rsslDecodeLocalElementSetDefDb(pIter, &elementSetDefDb)) == RSSL_RET_SUCCESS, ret, pError)) return RSSL_RET_FAILURE;			
									}

									rsslClearVectorEntry(&vectorEntry);
									if ((ret = rsslDecodeVectorEntry(pIter, &vectorEntry)) != RSSL_RET_END_OF_CONTAINER)
									{
										RsslElementList vectorElementList;
										RsslBool foundPort = RSSL_FALSE, foundMCGroup = RSSL_FALSE, foundDomain = RSSL_FALSE;

										if (!RSSL_ERROR_INFO_CHECK(pService->seqMCastInfo.StreamingMCastChanServerList = (RsslRDMMCAddressPortInfo*)rsslReserveAlignedBufferMemory(pMemoryBuffer, 0, sizeof(RsslRDMMCAddressPortInfo)), RSSL_RET_BUFFER_TOO_SMALL, pError)) return RSSL_RET_BUFFER_TOO_SMALL;

										do {
											if (!RSSL_ERROR_INFO_CHECK(ret == RSSL_RET_SUCCESS, ret, pError)) return RSSL_RET_FAILURE;

											if (!RSSL_ERROR_INFO_CHECK(vector.containerType == RSSL_DT_ELEMENT_LIST, RSSL_RET_FAILURE, pError)) return RSSL_RET_FAILURE;

											if (!RSSL_ERROR_INFO_CHECK(channelInfoElem = (RsslRDMMCAddressPortInfo*)rsslReserveBufferMemory(pMemoryBuffer, 1, sizeof(RsslRDMMCAddressPortInfo)), RSSL_RET_BUFFER_TOO_SMALL, pError)) return RSSL_RET_BUFFER_TOO_SMALL;

											rsslClearElementList(&vectorElementList);
											if (!RSSL_ERROR_INFO_CHECK((ret = rsslDecodeElementList(pIter, &vectorElementList, &elementSetDefDb)) == RSSL_RET_SUCCESS, ret, pError)) return RSSL_RET_FAILURE;

											++pService->seqMCastInfo.StreamingMCastChanServerCount;

											while ((ret = rsslDecodeElementEntry(pIter, &elementEntry)) != RSSL_RET_END_OF_CONTAINER)
											{
												if (!RSSL_ERROR_INFO_CHECK(ret == RSSL_RET_SUCCESS, ret, pError)) return RSSL_RET_FAILURE;

												if (rsslBufferIsEqual(&elementEntry.name, &RSSL_ENAME_MULTICAST_GROUP))
												{
													if (!RSSL_ERROR_INFO_CHECK(elementEntry.dataType == RSSL_DT_ASCII_STRING || elementEntry.dataType == RSSL_DT_BUFFER, RSSL_RET_FAILURE, pError)) return RSSL_RET_FAILURE;

													channelInfoElem->address = elementEntry.encData;

													foundMCGroup = RSSL_TRUE;

												}
												else if (rsslBufferIsEqual(&elementEntry.name, &RSSL_ENAME_PORT))
												{
													if (!RSSL_ERROR_INFO_CHECK(elementEntry.dataType == RSSL_DT_UINT, RSSL_RET_FAILURE, pError)) return RSSL_RET_FAILURE;

													if (!RSSL_ERROR_INFO_CHECK((ret = rsslDecodeUInt(pIter, &channelInfoElem->port)) == RSSL_RET_SUCCESS, ret, pError)) return RSSL_RET_FAILURE;

													foundPort = RSSL_TRUE;
												}
												else if (rsslBufferIsEqual(&elementEntry.name, &RSSL_ENAME_DOMAIN))
												{
													if (!RSSL_ERROR_INFO_CHECK(elementEntry.dataType == RSSL_DT_UINT, RSSL_RET_FAILURE, pError)) return RSSL_RET_FAILURE;

													if (!RSSL_ERROR_INFO_CHECK((ret = rsslDecodeUInt(pIter, &channelInfoElem->domain)) == RSSL_RET_SUCCESS, ret, pError)) return RSSL_RET_FAILURE;

													foundDomain = RSSL_TRUE;
												}
											}
										} while ((ret = rsslDecodeVectorEntry(pIter, &vectorEntry)) != RSSL_RET_END_OF_CONTAINER);
										
										if (foundPort && foundMCGroup && foundDomain)
											pService->seqMCastInfo.flags |= RDM_SVC_SMF_HAS_SMC_SERV;
									}
								}
								else if (rsslBufferIsEqual(&elementEntry.name, &RSSL_ENAME_GAP_MCAST_CHANNELS))
								{
									RsslRDMMCAddressPortInfo *channelInfoElem;

									rsslClearVector(&vector);
									if (!RSSL_ERROR_INFO_CHECK((ret = rsslDecodeVector(pIter, &vector)) == RSSL_RET_SUCCESS, ret, pError)) return RSSL_RET_FAILURE;

									if (!RSSL_ERROR_INFO_CHECK(vector.containerType == RSSL_DT_ELEMENT_LIST, RSSL_RET_FAILURE, pError)) return RSSL_RET_FAILURE;

									if (vector.flags & RSSL_VTF_HAS_SET_DEFS)
									{
										rsslClearLocalElementSetDefDb(&elementSetDefDb);
										if (!RSSL_ERROR_INFO_CHECK((ret = rsslDecodeLocalElementSetDefDb(pIter, &elementSetDefDb)) == RSSL_RET_SUCCESS, ret, pError)) return RSSL_RET_FAILURE;			
									}

									rsslClearVectorEntry(&vectorEntry);
									if ((ret = rsslDecodeVectorEntry(pIter, &vectorEntry)) != RSSL_RET_END_OF_CONTAINER)
									{
										RsslElementList vectorElementList;
										RsslBool foundPort = RSSL_FALSE, foundMCGroup = RSSL_FALSE, foundDomain = RSSL_FALSE;

										if (!RSSL_ERROR_INFO_CHECK(pService->seqMCastInfo.GapMCastChanServerList = (RsslRDMMCAddressPortInfo*)rsslReserveAlignedBufferMemory(pMemoryBuffer, 0, sizeof(RsslRDMMCAddressPortInfo)), RSSL_RET_BUFFER_TOO_SMALL, pError)) return RSSL_RET_BUFFER_TOO_SMALL;

										do {
											if (!RSSL_ERROR_INFO_CHECK(ret == RSSL_RET_SUCCESS, ret, pError)) return RSSL_RET_FAILURE;

											if (!RSSL_ERROR_INFO_CHECK(vector.containerType == RSSL_DT_ELEMENT_LIST, RSSL_RET_FAILURE, pError)) return RSSL_RET_FAILURE;

											if (!RSSL_ERROR_INFO_CHECK(channelInfoElem = (RsslRDMMCAddressPortInfo*)rsslReserveBufferMemory(pMemoryBuffer, 1, sizeof(RsslRDMMCAddressPortInfo)), RSSL_RET_BUFFER_TOO_SMALL, pError)) return RSSL_RET_BUFFER_TOO_SMALL;

											rsslClearElementList(&vectorElementList);
											if (!RSSL_ERROR_INFO_CHECK((ret = rsslDecodeElementList(pIter, &vectorElementList, &elementSetDefDb)) == RSSL_RET_SUCCESS, ret, pError)) return RSSL_RET_FAILURE;

											++pService->seqMCastInfo.GapMCastChanServerCount;

											while ((ret = rsslDecodeElementEntry(pIter, &elementEntry)) != RSSL_RET_END_OF_CONTAINER)
											{
												if (!RSSL_ERROR_INFO_CHECK(ret == RSSL_RET_SUCCESS, ret, pError)) return RSSL_RET_FAILURE;

												if (rsslBufferIsEqual(&elementEntry.name, &RSSL_ENAME_MULTICAST_GROUP))
												{
													if (!RSSL_ERROR_INFO_CHECK(elementEntry.dataType == RSSL_DT_ASCII_STRING || elementEntry.dataType == RSSL_DT_BUFFER, RSSL_RET_FAILURE, pError)) return RSSL_RET_FAILURE;

													channelInfoElem->address = elementEntry.encData;

													foundMCGroup = RSSL_TRUE;

												}
												else if (rsslBufferIsEqual(&elementEntry.name, &RSSL_ENAME_PORT))
												{
													if (!RSSL_ERROR_INFO_CHECK(elementEntry.dataType == RSSL_DT_UINT, RSSL_RET_FAILURE, pError)) return RSSL_RET_FAILURE;

													if (!RSSL_ERROR_INFO_CHECK((ret = rsslDecodeUInt(pIter, &channelInfoElem->port)) == RSSL_RET_SUCCESS, ret, pError)) return RSSL_RET_FAILURE;

													foundPort = RSSL_TRUE;
												}
												else if (rsslBufferIsEqual(&elementEntry.name, &RSSL_ENAME_DOMAIN))
												{
													if (!RSSL_ERROR_INFO_CHECK(elementEntry.dataType == RSSL_DT_UINT, RSSL_RET_FAILURE, pError)) return RSSL_RET_FAILURE;

													if (!RSSL_ERROR_INFO_CHECK((ret = rsslDecodeUInt(pIter, &channelInfoElem->domain)) == RSSL_RET_SUCCESS, ret, pError)) return RSSL_RET_FAILURE;

													foundDomain = RSSL_TRUE;
												}
											}
										} while ((ret = rsslDecodeVectorEntry(pIter, &vectorEntry)) != RSSL_RET_END_OF_CONTAINER);

										if (foundPort && foundMCGroup && foundDomain)
											pService->seqMCastInfo.flags |= RDM_SVC_SMF_HAS_GMC_SERV;
									}
								}

								if (foundSnapshotPort && foundSnapshotAddr)
									pService->seqMCastInfo.flags |= RDM_SVC_SMF_HAS_SNAPSHOT_SERV;

								if (foundGapRecPort && foundGapRecAddr)
									pService->seqMCastInfo.flags |= RDM_SVC_SMF_HAS_GAP_REC_SERV;

								if (foundRefDataPort && foundRefDataAddr)
									pService->seqMCastInfo.flags |= RDM_SVC_SMF_HAS_REF_DATA_SERV;
							}
							break;
						}
					case RDM_DIRECTORY_SERVICE_LINK_ID:
						{
							RsslMap map;
							RsslMapEntry mapEntry;
							RsslElementList elementList;
							RsslElementEntry elementEntry;
							RsslRDMServiceLink *pLink;
							RsslBuffer mapKey;
							
							pService->flags |= RDM_SVCF_HAS_LINK;
							pService->linkInfo.action = (RsslFilterEntryActions)filterEntry.action;

							if (filterEntry.action == RSSL_FTEA_CLEAR_ENTRY)
								break;


							if (!RSSL_ERROR_INFO_CHECK(filterEntry.containerType == RSSL_DT_MAP, RSSL_RET_FAILURE, pError)) return RSSL_RET_FAILURE;

							if (!RSSL_ERROR_INFO_CHECK((ret = rsslDecodeMap(pIter, &map)) == RSSL_RET_SUCCESS, ret, pError)) return RSSL_RET_FAILURE;

							if (!RSSL_ERROR_INFO_CHECK(map.containerType == RSSL_DT_ELEMENT_LIST && map.keyPrimitiveType == RSSL_DT_ASCII_STRING || map.keyPrimitiveType == RSSL_DT_BUFFER, RSSL_RET_FAILURE, pError)) return RSSL_RET_FAILURE;

							rsslClearMapEntry(&mapEntry);
							if ((ret = rsslDecodeMapEntry(pIter, &mapEntry, &mapKey)) != RSSL_RET_END_OF_CONTAINER)
							{
								/* Align buffer so we can start an array */
								if (!RSSL_ERROR_INFO_CHECK(pService->linkInfo.linkList = (RsslRDMServiceLink*)rsslReserveAlignedBufferMemory(pMemoryBuffer, 0, sizeof(RsslRDMServiceLink)), RSSL_RET_BUFFER_TOO_SMALL, pError)) return RSSL_RET_BUFFER_TOO_SMALL;

								do
								{
									RsslBool foundServiceLinkState = RSSL_FALSE;

									if (!RSSL_ERROR_INFO_CHECK(ret == RSSL_RET_SUCCESS, ret, pError)) return RSSL_RET_FAILURE;

									if (!RSSL_ERROR_INFO_CHECK(pLink = (RsslRDMServiceLink*)rsslReserveBufferMemory(pMemoryBuffer, 1, sizeof(RsslRDMServiceLink)), RSSL_RET_BUFFER_TOO_SMALL, pError)) return RSSL_RET_BUFFER_TOO_SMALL;

									rsslClearRDMServiceLink(pLink);

									pLink->name = mapKey;
									pLink->action = (RsslMapEntryActions)mapEntry.action;

									if (pLink->action != RSSL_MPEA_DELETE_ENTRY)
									{
										rsslClearElementList(&elementList);
										if (!RSSL_ERROR_INFO_CHECK((ret = rsslDecodeElementList(pIter, &elementList, 0)) == RSSL_RET_SUCCESS, ret, pError)) return RSSL_RET_FAILURE;

										rsslClearElementEntry(&elementEntry);

										while ((ret = rsslDecodeElementEntry(pIter, &elementEntry)) != RSSL_RET_END_OF_CONTAINER)
										{
											if (!RSSL_ERROR_INFO_CHECK(ret == RSSL_RET_SUCCESS, ret, pError)) return RSSL_RET_FAILURE;

											if (rsslBufferIsEqual(&elementEntry.name, &RSSL_ENAME_TYPE))
											{
												pLink->flags |= RDM_SVC_LKF_HAS_TYPE;
												if (!RSSL_ERROR_INFO_CHECK(elementEntry.dataType == RSSL_DT_UINT, RSSL_RET_FAILURE, pError)) return RSSL_RET_FAILURE;

												if (!RSSL_ERROR_INFO_CHECK((ret = rsslDecodeUInt(pIter, &pLink->type)) == RSSL_RET_SUCCESS, ret, pError)) return RSSL_RET_FAILURE;
											}
											else if (rsslBufferIsEqual(&elementEntry.name, &RSSL_ENAME_LINK_STATE))
											{
												if (!RSSL_ERROR_INFO_CHECK(elementEntry.dataType == RSSL_DT_UINT, RSSL_RET_FAILURE, pError)) return RSSL_RET_FAILURE;

												if (!RSSL_ERROR_INFO_CHECK((ret = rsslDecodeUInt(pIter, &pLink->linkState)) == RSSL_RET_SUCCESS, ret, pError)) return RSSL_RET_FAILURE;
												foundServiceLinkState = RSSL_TRUE;
											}
											else if (rsslBufferIsEqual(&elementEntry.name, &RSSL_ENAME_LINK_CODE))
											{
												pLink->flags |= RDM_SVC_LKF_HAS_CODE;
												if (!RSSL_ERROR_INFO_CHECK(elementEntry.dataType == RSSL_DT_UINT, RSSL_RET_FAILURE, pError)) return RSSL_RET_FAILURE;

												if (!RSSL_ERROR_INFO_CHECK((ret = rsslDecodeUInt(pIter, &pLink->linkCode)) == RSSL_RET_SUCCESS, ret, pError)) return RSSL_RET_FAILURE;
											}
											else if (rsslBufferIsEqual(&elementEntry.name, &RSSL_ENAME_TEXT))
											{
												pLink->flags |= RDM_SVC_LKF_HAS_TEXT;
												if (!RSSL_ERROR_INFO_CHECK(elementEntry.dataType == RSSL_DT_ASCII_STRING || elementEntry.dataType == RSSL_DT_BUFFER, RSSL_RET_FAILURE, pError)) return RSSL_RET_FAILURE;

												pLink->text = elementEntry.encData;
											}
										}

										if (!RSSL_ERROR_INFO_CHECK(foundServiceLinkState, RSSL_RET_FAILURE, pError)) return RSSL_RET_FAILURE;
									}
									++pService->linkInfo.linkCount;

									rsslClearMapEntry(&mapEntry);
								} while ((ret = rsslDecodeMapEntry(pIter, &mapEntry, &mapKey)) != RSSL_RET_END_OF_CONTAINER);
							}
							break;
						}

				}
			}
		}
		++pService;
	}

	switch(pDirectoryResponse->rdmMsgBase.rdmMsgType)
	{
		case RDM_DR_MT_REFRESH:
			pDirectoryResponse->refresh.serviceCount = serviceCount;
			pDirectoryResponse->refresh.serviceList = pServices;
			break;
		case RDM_DR_MT_UPDATE: 
			pDirectoryResponse->update.serviceCount = serviceCount;
			pDirectoryResponse->update.serviceList = pServices;
			break;
		default:
			rsslSetErrorInfo(pError, RSSL_EIC_FAILURE, RSSL_RET_FAILURE, __FILE__, __LINE__, "Unknown directory message type %d\n", pDirectoryResponse->rdmMsgBase.rdmMsgType);
			return RSSL_RET_FAILURE;
	}

	return RSSL_RET_SUCCESS;
}

RSSL_VA_API RsslRet rsslDecodeRDMDirectoryMsg(RsslDecodeIterator *pIter, RsslMsg *pMsg, RsslRDMDirectoryMsg *pDirectoryMsg, RsslBuffer *pMemoryBuffer, RsslErrorInfo *pError)
{
	RsslRet ret = 0;
	const RsslMsgKey* key = 0;

	switch (pMsg->msgBase.msgClass)
	{
		case RSSL_MC_REQUEST:
		{
			RsslRDMDirectoryRequest *pDirectoryRequest = &pDirectoryMsg->request;

			rsslClearRDMDirectoryRequest(pDirectoryRequest);

			if (pMsg->requestMsg.flags & RSSL_RQMF_STREAMING)
				pDirectoryRequest->flags |= RDM_DR_RQF_STREAMING;

			/* get key */
			key = (RsslMsgKey *)rsslGetMsgKey(pMsg);

			if (key->flags & RSSL_MKF_HAS_SERVICE_ID)
			{
				pDirectoryRequest->flags |= RDM_DR_RQF_HAS_SERVICE_ID;
				pDirectoryRequest->serviceId = key->serviceId;
			}

			/* get filter */
			if (key->flags & RSSL_MKF_HAS_FILTER)
				pDirectoryRequest->filter = key->filter;

			break;
		}
		case RSSL_MC_GENERIC:
		{
			/* get key */
			key = (RsslMsgKey *)rsslGetMsgKey(pMsg);

			if (!RSSL_ERROR_INFO_CHECK(key && (key->flags & RSSL_MKF_HAS_NAME), RSSL_RET_FAILURE, pError)) return RSSL_RET_FAILURE;

			if (rsslBufferIsEqual(&key->name, &RSSL_ENAME_CONS_STATUS))
			{
				RsslRDMDirectoryConsumerStatus *pConsumerStatus = &pDirectoryMsg->consumerStatus;
				RsslMap map; RsslMapEntry mapEntry;
				RsslUInt mapKey;
				RsslElementList elementList; RsslElementEntry elementEntry;

				rsslClearRDMDirectoryConsumerStatus(pConsumerStatus);

				if (!RSSL_ERROR_INFO_CHECK(pMsg->msgBase.containerType == RSSL_DT_MAP, RSSL_RET_FAILURE, pError)) return RSSL_RET_FAILURE;

				if (!RSSL_ERROR_INFO_CHECK((ret = rsslDecodeMap(pIter, &map)) == RSSL_RET_SUCCESS, ret, pError)) return RSSL_RET_FAILURE;

				if (!RSSL_ERROR_INFO_CHECK(map.keyPrimitiveType == RSSL_DT_UINT, RSSL_RET_FAILURE, pError)) return RSSL_RET_FAILURE;

				if (!RSSL_ERROR_INFO_CHECK(map.containerType == RSSL_DT_ELEMENT_LIST, RSSL_RET_FAILURE, pError)) return RSSL_RET_FAILURE;

				rsslClearMapEntry(&mapEntry);
				if (( ret = rsslDecodeMapEntry(pIter, &mapEntry, &mapKey)) != RSSL_RET_END_OF_CONTAINER)
				{
					/* Align buffer so we can start an array */
					if (!RSSL_ERROR_INFO_CHECK(pConsumerStatus->consumerServiceStatusList = (RsslRDMConsumerStatusService*)rsslReserveAlignedBufferMemory(pMemoryBuffer, 0, sizeof(RsslRDMConsumerStatusService)), RSSL_RET_BUFFER_TOO_SMALL, pError)) return RSSL_RET_BUFFER_TOO_SMALL;

					do
					{
						RsslRDMConsumerStatusService *pServiceStatus;
						if (!RSSL_ERROR_INFO_CHECK(ret == RSSL_RET_SUCCESS, ret, pError)) return RSSL_RET_FAILURE;

						if (!RSSL_ERROR_INFO_CHECK(pServiceStatus = (RsslRDMConsumerStatusService*)rsslReserveBufferMemory(pMemoryBuffer, 1, sizeof(RsslRDMConsumerStatusService)), RSSL_RET_BUFFER_TOO_SMALL, pError)) return RSSL_RET_BUFFER_TOO_SMALL;

						rsslClearRDMConsumerStatusService(pServiceStatus);
						pServiceStatus->serviceId = mapKey;
						pServiceStatus->action = (RsslMapEntryActions)mapEntry.action;

						if (mapEntry.action != RSSL_MPEA_DELETE_ENTRY)
						{
							RsslBool foundSourceMirroringMode = RSSL_FALSE;

							if (!RSSL_ERROR_INFO_CHECK((ret = rsslDecodeElementList(pIter, &elementList, NULL)) == RSSL_RET_SUCCESS, ret, pError)) return RSSL_RET_FAILURE;

							while((ret = rsslDecodeElementEntry(pIter, &elementEntry)) != RSSL_RET_END_OF_CONTAINER)
							{
								if (!RSSL_ERROR_INFO_CHECK(ret == RSSL_RET_SUCCESS, ret, pError)) return RSSL_RET_FAILURE;

								if (rsslBufferIsEqual(&elementEntry.name, &RSSL_ENAME_SOURCE_MIROR_MODE))
								{
									if (!RSSL_ERROR_INFO_CHECK(elementEntry.dataType == RSSL_DT_UINT, RSSL_RET_FAILURE, pError)) return RSSL_RET_FAILURE;
									if (!RSSL_ERROR_INFO_CHECK((ret = rsslDecodeUInt(pIter, &pServiceStatus->sourceMirroringMode)) == RSSL_RET_SUCCESS, ret, pError)) return RSSL_RET_FAILURE;
									foundSourceMirroringMode = RSSL_TRUE;
								}
							}

							if (!RSSL_ERROR_INFO_CHECK(foundSourceMirroringMode, RSSL_RET_FAILURE, pError)) return RSSL_RET_FAILURE;

						}
						++pConsumerStatus->consumerServiceStatusCount;

						rsslClearMapEntry(&mapEntry);
					} while (( ret = rsslDecodeMapEntry(pIter, &mapEntry, &mapKey)) != RSSL_RET_END_OF_CONTAINER);
				}
			}
			else
			{
				rsslSetErrorInfo(pError, RSSL_EIC_FAILURE, RSSL_RET_FAILURE, __FILE__, __LINE__, "Unknown generic msg name \"%.*s\"\n", key->name.length, key->name.data);
				return RSSL_RET_FAILURE;
			}

			break;
		}
		case RSSL_MC_CLOSE:
			rsslClearRDMDirectoryClose(&pDirectoryMsg->close);
			pError->rsslErrorInfoCode = RSSL_EIC_SUCCESS;
			break;
		case RSSL_MC_REFRESH:
		{
			RsslRDMDirectoryRefresh *pDirectoryRefresh = &pDirectoryMsg->refresh;

			rsslClearRDMDirectoryRefresh(pDirectoryRefresh);

			pDirectoryRefresh->state = pMsg->refreshMsg.state;

			key = rsslGetMsgKey(pMsg);

			if (key)
			{

				if (key->flags & RSSL_MKF_HAS_SERVICE_ID)
				{
					pDirectoryRefresh->flags |= RDM_DR_RFF_HAS_SERVICE_ID;
					pDirectoryRefresh->serviceId = key->serviceId;
				}

				if (key->flags & RSSL_MKF_HAS_FILTER)
					pDirectoryRefresh->filter = key->filter;
			}

			if (pMsg->refreshMsg.flags & RSSL_RFMF_SOLICITED)
				pDirectoryRefresh->flags |= RDM_DR_RFF_SOLICITED;

			if (pMsg->refreshMsg.flags & RSSL_RFMF_CLEAR_CACHE)
				pDirectoryRefresh->flags |= RDM_DR_RFF_CLEAR_CACHE;

			if (pMsg->refreshMsg.flags & RSSL_RFMF_HAS_SEQ_NUM)
			{
				pDirectoryRefresh->flags |= RDM_DR_RFF_HAS_SEQ_NUM;
				pDirectoryRefresh->sequenceNumber = pMsg->refreshMsg.seqNum;
			}

			if (pMsg->msgBase.containerType == RSSL_DT_NO_DATA) break;
			else if (!RSSL_ERROR_INFO_CHECK(pMsg->msgBase.containerType == RSSL_DT_MAP, RSSL_RET_FAILURE, pError)) return RSSL_RET_FAILURE;

			if ((ret = _rsslDecodeServiceList(pIter, &pMsg->msgBase.encDataBody, pDirectoryMsg, pMemoryBuffer, pError)) != RSSL_RET_SUCCESS)
				return ret;

			break;
		}

		case RSSL_MC_UPDATE:
		{
			RsslRDMDirectoryUpdate *pDirectoryUpdate = &pDirectoryMsg->update;

			rsslClearRDMDirectoryUpdate(pDirectoryUpdate);
			key = rsslGetMsgKey(pMsg);
			if (key)
			{
			   if(key->flags & RSSL_MKF_HAS_FILTER)
			   {
				   pDirectoryUpdate->flags |= RDM_DR_UPF_HAS_FILTER;
				   pDirectoryUpdate->filter = key->filter;
			   }

			   if(key->flags & RSSL_MKF_HAS_SERVICE_ID)
			   {
				   pDirectoryUpdate->flags |= RDM_DR_UPF_HAS_SERVICE_ID;
				   pDirectoryUpdate->serviceId = key->serviceId;
			   }

			}

			if(pMsg->updateMsg.flags & RSSL_UPMF_HAS_SEQ_NUM)
			{
				pDirectoryUpdate->flags |= RDM_DR_UPF_HAS_SEQ_NUM;
				pDirectoryUpdate->sequenceNumber = pMsg->updateMsg.seqNum;
			}

			if (pMsg->msgBase.containerType == RSSL_DT_NO_DATA) break;
			else if (!RSSL_ERROR_INFO_CHECK(pMsg->msgBase.containerType == RSSL_DT_MAP, RSSL_RET_FAILURE, pError)) return RSSL_RET_FAILURE;

			if ((ret = _rsslDecodeServiceList(pIter, &pMsg->msgBase.encDataBody, pDirectoryMsg, pMemoryBuffer, pError)) != RSSL_RET_SUCCESS)
				return ret;

			break;
		}
		case RSSL_MC_STATUS:
		{
			RsslRDMDirectoryStatus *pDirectoryStatus = &pDirectoryMsg->status;

			rsslClearRDMDirectoryStatus(pDirectoryStatus);

			key = rsslGetMsgKey(pMsg);

			if (key)
			{
			   if(key->flags & RSSL_MKF_HAS_FILTER)
			   {
				   pDirectoryStatus->flags |= RDM_DR_STF_HAS_FILTER;
				   pDirectoryStatus->filter = key->filter;
			   }
			   if(key->flags & RSSL_MKF_HAS_SERVICE_ID)
			   {
				   pDirectoryStatus->flags |= RDM_DR_STF_HAS_SERVICE_ID;
				   pDirectoryStatus->serviceId = key->serviceId;
			   }
			}

			if (pMsg->statusMsg.flags & RSSL_STMF_CLEAR_CACHE)
				pDirectoryStatus->flags |= RDM_DR_STF_CLEAR_CACHE;

			if (pMsg->statusMsg.flags & RSSL_STMF_HAS_STATE)
			{
				pDirectoryStatus->flags |= RDM_DR_STF_HAS_STATE;
				pDirectoryStatus->state = pMsg->statusMsg.state;
			}
			break;
		}
		default:
			pError->rsslErrorInfoCode = RSSL_EIC_FAILURE;
			return RSSL_RET_FAILURE;
	}

	pDirectoryMsg->rdmMsgBase.streamId = pMsg->msgBase.streamId;
	pError->rsslErrorInfoCode = RSSL_EIC_SUCCESS;
	return RSSL_RET_SUCCESS;
}

static RsslRet _copyServiceList(RsslRDMService **ppNewServiceList, RsslRDMService *pOldServiceList, RsslUInt32 serviceCount, RsslBuffer *pNewMemoryBuffer)
{
	RsslRDMService *pNewServiceList;
	RsslUInt32 i, j;

	*ppNewServiceList = pNewServiceList = (RsslRDMService*)rsslReserveAlignedBufferMemory(pNewMemoryBuffer, serviceCount, sizeof(RsslRDMService));
	if (!pNewServiceList) return RSSL_RET_BUFFER_TOO_SMALL;

	for(i = 0; i < serviceCount; ++i)
	{
		pNewServiceList[i] = pOldServiceList[i];

		/* Info */
		if (pOldServiceList[i].flags & RDM_SVCF_HAS_INFO)
		{
			RsslRDMServiceInfo *pNewInfo = &pNewServiceList[i].info, *pOldInfo = &pOldServiceList[i].info;

			if (!rsslCopyBufferMemory(&pNewInfo->serviceName, &pOldInfo->serviceName, pNewMemoryBuffer))
				return RSSL_RET_BUFFER_TOO_SMALL;

			if(pOldInfo->flags & RDM_SVC_IFF_HAS_VENDOR)
			{
				if (!rsslCopyBufferMemory(&pNewInfo->vendor, &pOldInfo->vendor, pNewMemoryBuffer))
					return RSSL_RET_BUFFER_TOO_SMALL;
			}

			/* Capabilities */
			pNewInfo->capabilitiesList = (RsslUInt*)rsslReserveAlignedBufferMemory(pNewMemoryBuffer, pOldInfo->capabilitiesCount, sizeof(RsslUInt));
			if (!pNewInfo->capabilitiesList) return RSSL_RET_BUFFER_TOO_SMALL;
			memcpy(pNewInfo->capabilitiesList, pOldInfo->capabilitiesList, pOldInfo->capabilitiesCount * sizeof(RsslUInt));

			/* Dictionaries Provided */
			if (pOldInfo->flags & RDM_SVC_IFF_HAS_DICTS_PROVIDED)
			{
				pNewInfo->dictionariesProvidedList = (RsslBuffer*)rsslReserveAlignedBufferMemory(pNewMemoryBuffer, pOldInfo->dictionariesProvidedCount, sizeof(RsslBuffer));
				if (!pNewInfo->dictionariesProvidedList) return RSSL_RET_BUFFER_TOO_SMALL;

				for(j = 0; j < pOldInfo->dictionariesProvidedCount; ++j)
					if (!rsslCopyBufferMemory(&pNewInfo->dictionariesProvidedList[j], &pOldInfo->dictionariesProvidedList[j], pNewMemoryBuffer))
						return RSSL_RET_BUFFER_TOO_SMALL;
			}

			/* Dictionaries Used */
			if (pOldInfo->flags & RDM_SVC_IFF_HAS_DICTS_USED)
			{
				pNewInfo->dictionariesUsedList = (RsslBuffer*)rsslReserveAlignedBufferMemory(pNewMemoryBuffer, pOldInfo->dictionariesUsedCount, sizeof(RsslBuffer));
				if (!pNewInfo->dictionariesUsedList) return RSSL_RET_BUFFER_TOO_SMALL;

				for(j = 0; j < pOldInfo->dictionariesUsedCount; ++j)
					if (!rsslCopyBufferMemory(&pNewInfo->dictionariesUsedList[j], &pOldInfo->dictionariesUsedList[j], pNewMemoryBuffer))
						return RSSL_RET_BUFFER_TOO_SMALL;
			}

			/* Qualities of Service */
			if (pOldInfo->flags & RDM_SVC_IFF_HAS_QOS)
			{
				pNewInfo->qosList = (RsslQos*)rsslReserveAlignedBufferMemory(pNewMemoryBuffer, pOldInfo->qosCount, sizeof(RsslQos));
				if (!pNewInfo->qosList) return RSSL_RET_BUFFER_TOO_SMALL;
				memcpy(pNewInfo->qosList, pOldInfo->qosList, pOldInfo->qosCount * sizeof(RsslQos));
			}

			if (!rsslCopyBufferMemory(&pNewInfo->itemList, &pOldInfo->itemList, pNewMemoryBuffer))
				return RSSL_RET_BUFFER_TOO_SMALL;
		} 

		/* State */
		if (pOldServiceList[i].flags & RDM_SVCF_HAS_STATE)
		{
			RsslRDMServiceState *pNewState = &pNewServiceList[i].state, *pOldState = &pOldServiceList[i].state;

			if (pOldState->flags & RDM_SVC_GRF_HAS_STATUS)
			{
				/* Status already soft-copied, just copy text */
				if (!rsslCopyBufferMemory(&pNewState->status.text, &pOldState->status.text, pNewMemoryBuffer))
					return RSSL_RET_BUFFER_TOO_SMALL;
			}
		}

		/* Group */
		if (pOldServiceList[i].groupStateCount)
		{
			pNewServiceList[i].groupStateList = (RsslRDMServiceGroupState*)rsslReserveAlignedBufferMemory(pNewMemoryBuffer, pNewServiceList[i].groupStateCount, sizeof(RsslRDMServiceGroupState));
			if (!pNewServiceList[i].groupStateList) return RSSL_RET_BUFFER_TOO_SMALL;

			for(j = 0; j < pOldServiceList[i].groupStateCount; ++j)
			{
				RsslRDMServiceGroupState *pNewGroupState = &pNewServiceList[i].groupStateList[j], *pOldGroupState = &pOldServiceList[i].groupStateList[j];

				*pNewGroupState = *pOldGroupState;

				if (!rsslCopyBufferMemory(&pNewGroupState->group, &pOldGroupState->group, pNewMemoryBuffer))
					return RSSL_RET_BUFFER_TOO_SMALL;

				if (pOldGroupState->flags & RDM_SVC_GRF_HAS_MERGED_TO_GROUP)
				{
					if (!rsslCopyBufferMemory(&pNewGroupState->mergedToGroup, &pOldGroupState->mergedToGroup, pNewMemoryBuffer))
						return RSSL_RET_BUFFER_TOO_SMALL;
				}

				if (pOldGroupState->flags & RDM_SVC_GRF_HAS_STATUS)
				{
					/* Status already soft-copied, just copy text */
					if (!rsslCopyBufferMemory(&pNewGroupState->status.text, &pOldGroupState->status.text, pNewMemoryBuffer))
						return RSSL_RET_BUFFER_TOO_SMALL;
				}

			}
		}

		/* (Load has nothing to deep copy) */

		/* Data */
		if (pOldServiceList[i].flags & RDM_SVCF_HAS_DATA)
		{
			if (!rsslCopyBufferMemory(&pNewServiceList[i].data.data, &pOldServiceList[i].data.data, pNewMemoryBuffer))
				return RSSL_RET_BUFFER_TOO_SMALL;
		}

		/* Link */
		if (pOldServiceList[i].flags & RDM_SVCF_HAS_LINK)
		{
			pNewServiceList[i].linkInfo.linkList = (RsslRDMServiceLink*)rsslReserveAlignedBufferMemory(pNewMemoryBuffer, pOldServiceList[i].linkInfo.linkCount, sizeof(RsslRDMServiceLink));
			if (!pNewServiceList[i].linkInfo.linkList) return RSSL_RET_BUFFER_TOO_SMALL;

			for(j = 0; j < pOldServiceList[i].linkInfo.linkCount; ++j)
			{
				RsslRDMServiceLink *pNewLink = &pNewServiceList[i].linkInfo.linkList[j], *pOldLink = &pOldServiceList[i].linkInfo.linkList[j];

				*pNewLink = *pOldLink;

				if (!rsslCopyBufferMemory(&pNewLink->name, &pOldLink->name, pNewMemoryBuffer))
					return RSSL_RET_BUFFER_TOO_SMALL;

				if ((pNewLink->flags & RDM_SVC_LKF_HAS_TEXT) && !rsslCopyBufferMemory(&pNewLink->text, &pOldLink->text, pNewMemoryBuffer))
					return RSSL_RET_BUFFER_TOO_SMALL;
			}

		}

		/* Sequenced Mcast */
		if (pOldServiceList[i].flags & RDM_SVCF_HAS_SEQ_MCAST)
		{
			RsslRDMServiceSeqMCastInfo *pNewSeqMCastInfo = &pNewServiceList[i].seqMCastInfo, *pOldSeqMCastInfo = &pOldServiceList[i].seqMCastInfo;

			/* Snapshot Server */
			if (pOldSeqMCastInfo->flags & RDM_SVC_SMF_HAS_SNAPSHOT_SERV)
			{
				if (!rsslCopyBufferMemory(&pNewSeqMCastInfo->snapshotServer.address, &pOldSeqMCastInfo->snapshotServer.address, pNewMemoryBuffer))
					return RSSL_RET_BUFFER_TOO_SMALL;
			}

			/* GapFill Server */
			if (pOldSeqMCastInfo->flags & RDM_SVC_SMF_HAS_GAP_REC_SERV)
			{
				if (!rsslCopyBufferMemory(&pNewSeqMCastInfo->gapRecoveryServer.address, &pOldSeqMCastInfo->gapRecoveryServer.address, pNewMemoryBuffer))
					return RSSL_RET_BUFFER_TOO_SMALL;
			}

			/* Ref Data Server */
			if (pOldSeqMCastInfo->flags & RDM_SVC_SMF_HAS_REF_DATA_SERV)
			{
				if (!rsslCopyBufferMemory(&pNewSeqMCastInfo->refDataServer.address, &pOldSeqMCastInfo->refDataServer.address, pNewMemoryBuffer))
					return RSSL_RET_BUFFER_TOO_SMALL;
			}

			/* Streaming Multicast Channels */
			if (pOldSeqMCastInfo->flags & RDM_SVC_SMF_HAS_SMC_SERV)
			{
				pNewServiceList[i].seqMCastInfo.StreamingMCastChanServerList = (RsslRDMMCAddressPortInfo*)rsslReserveAlignedBufferMemory(pNewMemoryBuffer, pOldServiceList[i].seqMCastInfo.StreamingMCastChanServerCount, sizeof(RsslRDMMCAddressPortInfo));
				if (!pNewServiceList[i].seqMCastInfo.StreamingMCastChanServerList) return RSSL_RET_BUFFER_TOO_SMALL;

				for(j = 0; j < pOldServiceList[i].seqMCastInfo.StreamingMCastChanServerCount; ++j)
				{
					RsslRDMMCAddressPortInfo *pNewLink = &pNewServiceList[i].seqMCastInfo.StreamingMCastChanServerList[j], *pOldLink = &pOldServiceList[i].seqMCastInfo.StreamingMCastChanServerList[j];

					*pNewLink = *pOldLink;

					if (!rsslCopyBufferMemory(&pNewLink->address, &pOldLink->address, pNewMemoryBuffer))
						return RSSL_RET_BUFFER_TOO_SMALL;
				}
			}

			/* Gap Multicast Channels */
			if (pOldSeqMCastInfo->flags & RDM_SVC_SMF_HAS_GMC_SERV)
			{
				pNewServiceList[i].seqMCastInfo.GapMCastChanServerList = (RsslRDMMCAddressPortInfo*)rsslReserveAlignedBufferMemory(pNewMemoryBuffer, pOldServiceList[i].seqMCastInfo.GapMCastChanServerCount, sizeof(RsslRDMMCAddressPortInfo));
				if (!pNewServiceList[i].seqMCastInfo.GapMCastChanServerList) return RSSL_RET_BUFFER_TOO_SMALL;

				for(j = 0; j < pOldServiceList[i].seqMCastInfo.GapMCastChanServerCount; ++j)
				{
					RsslRDMMCAddressPortInfo *pNewLink = &pNewServiceList[i].seqMCastInfo.GapMCastChanServerList[j], *pOldLink = &pOldServiceList[i].seqMCastInfo.GapMCastChanServerList[j];

					*pNewLink = *pOldLink;

					if (!rsslCopyBufferMemory(&pNewLink->address, &pOldLink->address, pNewMemoryBuffer))
						return RSSL_RET_BUFFER_TOO_SMALL;
				}
			}
		}
	}

	return RSSL_RET_SUCCESS;
}

RSSL_VA_API RsslRet rsslCopyRDMDirectoryRequest(RsslRDMDirectoryRequest *pNewRequest, RsslRDMDirectoryRequest *pOldRequest, RsslBuffer *pNewMemoryBuffer)
{
	*pNewRequest = *pOldRequest;
	return RSSL_RET_SUCCESS;
}

RSSL_VA_API RsslRet rsslCopyRDMDirectoryClose(RsslRDMDirectoryClose *pNewClose, RsslRDMDirectoryClose *pOldClose, RsslBuffer *pNewMemoryBuffer)
{
	*pNewClose = *pOldClose;
	return RSSL_RET_SUCCESS;
}

RSSL_VA_API RsslRet rsslCopyRDMDirectoryConsumerStatus(RsslRDMDirectoryConsumerStatus *pNewConsStatus, RsslRDMDirectoryConsumerStatus *pOldConsStatus, RsslBuffer *pNewMemoryBuffer)
{
	RsslUInt32 i;

	*pNewConsStatus = *pOldConsStatus;

	if (pOldConsStatus->consumerServiceStatusCount)
	{
		pNewConsStatus->consumerServiceStatusList = (RsslRDMConsumerStatusService*)rsslReserveAlignedBufferMemory(pNewMemoryBuffer, pOldConsStatus->consumerServiceStatusCount, sizeof(RsslRDMConsumerStatusService));
		if (!pNewConsStatus->consumerServiceStatusList)
			return RSSL_RET_BUFFER_TOO_SMALL;

		for(i = 0; i < pOldConsStatus->consumerServiceStatusCount; ++i)
			pNewConsStatus->consumerServiceStatusList[i] = pOldConsStatus->consumerServiceStatusList[i];
	}

	return RSSL_RET_SUCCESS;

}

RSSL_VA_API RsslRet rsslCopyRDMDirectoryRefresh(RsslRDMDirectoryRefresh *pNewRefresh, RsslRDMDirectoryRefresh *pOldRefresh, RsslBuffer *pNewMemoryBuffer)
{
	RsslRet ret;

	*pNewRefresh = *pOldRefresh;

	if (pOldRefresh->serviceCount)
		if ((ret = _copyServiceList(&pNewRefresh->serviceList, pOldRefresh->serviceList, pOldRefresh->serviceCount, pNewMemoryBuffer)) != RSSL_RET_SUCCESS)
			return ret;

	return RSSL_RET_SUCCESS;
}

RSSL_VA_API RsslRet rsslCopyRDMDirectoryUpdate(RsslRDMDirectoryUpdate *pNewUpdate, RsslRDMDirectoryUpdate *pOldUpdate, RsslBuffer *pNewMemoryBuffer)
{
	RsslRet ret;

	*pNewUpdate = *pOldUpdate;

	if (pOldUpdate->serviceCount)
		if ((ret = _copyServiceList(&pNewUpdate->serviceList, pOldUpdate->serviceList, pOldUpdate->serviceCount, pNewMemoryBuffer)) != RSSL_RET_SUCCESS)
			return ret;

	return RSSL_RET_SUCCESS;
}

RSSL_VA_API RsslRet rsslCopyRDMDirectoryStatus(RsslRDMDirectoryStatus *pNewStatus, RsslRDMDirectoryStatus *pOldStatus, RsslBuffer *pNewMemoryBuffer)
{
	*pNewStatus = *pOldStatus;
	if (pOldStatus->flags & RDM_DR_STF_HAS_STATE)
	{
		if (!rsslCopyBufferMemory(&pNewStatus->state.text, &pOldStatus->state.text, pNewMemoryBuffer))
			return RSSL_RET_BUFFER_TOO_SMALL;
	}
	return RSSL_RET_SUCCESS;
}

RSSL_VA_API RsslRet rsslCopyRDMDirectoryMsg(RsslRDMDirectoryMsg *pNewMsg, RsslRDMDirectoryMsg *pOldMsg, RsslBuffer *pNewMemoryBuffer)
{

	switch(pOldMsg->rdmMsgBase.rdmMsgType)
	{
		case RDM_DR_MT_REQUEST: return rsslCopyRDMDirectoryRequest(&pNewMsg->request, &pOldMsg->request, pNewMemoryBuffer);
		case RDM_DR_MT_CLOSE: return rsslCopyRDMDirectoryClose(&pNewMsg->close, &pOldMsg->close, pNewMemoryBuffer);
		case RDM_DR_MT_CONSUMER_STATUS: return rsslCopyRDMDirectoryConsumerStatus(&pNewMsg->consumerStatus, &pOldMsg->consumerStatus, pNewMemoryBuffer);
		case RDM_DR_MT_REFRESH: return rsslCopyRDMDirectoryRefresh(&pNewMsg->refresh, &pOldMsg->refresh, pNewMemoryBuffer);
		case RDM_DR_MT_UPDATE: return rsslCopyRDMDirectoryUpdate(&pNewMsg->update, &pOldMsg->update, pNewMemoryBuffer);
		case RDM_DR_MT_STATUS: return rsslCopyRDMDirectoryStatus(&pNewMsg->status, &pOldMsg->status, pNewMemoryBuffer);
		default: return RSSL_RET_INVALID_ARGUMENT;
	}
}
