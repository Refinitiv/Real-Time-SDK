/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2015,2017,2019-2021,2024-2025 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

#include "rtr/rsslRDMDictionaryMsg.h"

RSSL_VA_API RsslRet rsslEncodeRDMDictionaryMsg(RsslEncodeIterator *pIter, RsslRDMDictionaryMsg *pDictionaryMsg, RsslUInt32 *pBytesWritten, RsslErrorInfo *pError)
{
	RsslRet ret;

	switch(pDictionaryMsg->rdmMsgBase.rdmMsgType)
	{
		case RDM_DC_MT_REQUEST:
		{
			RsslRequestMsg msg;
			RsslRDMDictionaryRequest *pDictionaryRequest = &pDictionaryMsg->request;

			rsslClearRequestMsg(&msg);
			msg.msgBase.msgClass = RSSL_MC_REQUEST;
			msg.msgBase.streamId = pDictionaryRequest->rdmMsgBase.streamId;
			msg.msgBase.domainType = RSSL_DMT_DICTIONARY;
			msg.msgBase.containerType = RSSL_DT_NO_DATA;
			msg.flags = RSSL_RQMF_NONE;

			if (pDictionaryRequest->flags & RDM_DC_RQF_STREAMING)
				msg.flags |= RSSL_RQMF_STREAMING;

			msg.msgBase.msgKey.flags = RSSL_MKF_HAS_FILTER | RSSL_MKF_HAS_NAME | RSSL_MKF_HAS_SERVICE_ID;
			msg.msgBase.msgKey.name = pDictionaryRequest->dictionaryName;
			msg.msgBase.msgKey.serviceId = pDictionaryRequest->serviceId;
			msg.msgBase.msgKey.filter = pDictionaryRequest->verbosity;

			if (!RSSL_ERROR_INFO_CHECK((ret = rsslEncodeMsg(pIter, (RsslMsg*)&msg)) == RSSL_RET_SUCCESS, ret, pError)) return ret;

			*pBytesWritten = rsslGetEncodedBufferLength(pIter);
			pError->rsslErrorInfoCode = RSSL_EIC_SUCCESS;
			return RSSL_RET_SUCCESS;
		}
		case RDM_DC_MT_CLOSE:
		{
			RsslCloseMsg msg;
			RsslRDMDictionaryClose *pDictionaryClose = &pDictionaryMsg->close;
			rsslClearCloseMsg(&msg);
			msg.msgBase.msgClass = RSSL_MC_CLOSE;
			msg.msgBase.streamId = pDictionaryClose->rdmMsgBase.streamId;
			msg.msgBase.domainType = RSSL_DMT_DICTIONARY;
			msg.msgBase.containerType = RSSL_DT_NO_DATA;
			msg.flags = RSSL_CLMF_NONE;

			if (!RSSL_ERROR_INFO_CHECK((ret = rsslEncodeMsg(pIter, (RsslMsg*)&msg)) == RSSL_RET_SUCCESS, ret, pError)) return ret;

			*pBytesWritten = rsslGetEncodedBufferLength(pIter);
			pError->rsslErrorInfoCode = RSSL_EIC_SUCCESS;
			return RSSL_RET_SUCCESS;
		}

		case RDM_DC_MT_REFRESH:
		{
			RsslRefreshMsg msg;
			char errorTextBuf[256];
			RsslBuffer errorText = { 256, errorTextBuf };

			RsslRDMDictionaryRefresh *pDictionaryRefresh = &pDictionaryMsg->refresh;

			/* RDM_DC_RFF_IS_COMPLETE and RDM_DC_RFF_HAS_INFO are for the decode function only.  */
			if (!RSSL_ERROR_INFO_CHECK(!(pDictionaryRefresh->flags & (RDM_DC_RFF_IS_COMPLETE | RDM_DC_RFF_HAS_INFO)), RSSL_RET_INVALID_ARGUMENT, pError)) return RSSL_RET_INVALID_ARGUMENT;

			rsslClearRefreshMsg(&msg);
			/* Populate msgBase */
			msg.msgBase.msgClass = RSSL_MC_REFRESH;
			msg.msgBase.streamId = pDictionaryRefresh->rdmMsgBase.streamId;
			msg.msgBase.domainType = RSSL_DMT_DICTIONARY;
			msg.msgBase.containerType = RSSL_DT_SERIES;
			msg.flags = RSSL_RFMF_HAS_MSG_KEY;

			if(pDictionaryRefresh->flags & RDM_DC_RFF_SOLICITED)
				msg.flags |= RSSL_RFMF_SOLICITED;

			if(pDictionaryRefresh->flags & RDM_DC_RFF_CLEAR_CACHE)
				msg.flags |= RSSL_RFMF_CLEAR_CACHE;

			if(pDictionaryRefresh->flags & RDM_DC_RFF_HAS_SEQ_NUM)
			{
				msg.flags |= RSSL_RFMF_HAS_SEQ_NUM;
				msg.seqNum = pDictionaryRefresh->sequenceNumber;
			}

			msg.state = pDictionaryRefresh->state;

			msg.msgBase.msgKey.flags = RSSL_MKF_HAS_NAME | RSSL_MKF_HAS_FILTER | RSSL_MKF_HAS_SERVICE_ID;
			msg.msgBase.msgKey.name = pDictionaryRefresh->dictionaryName;
			msg.msgBase.msgKey.filter = pDictionaryRefresh->verbosity;
			msg.msgBase.msgKey.serviceId = pDictionaryRefresh->serviceId;

			/* Begin encoding message */
			if (!RSSL_ERROR_INFO_CHECK((ret = rsslEncodeMsgInit(pIter, (RsslMsg*)&msg, 0)) == RSSL_RET_ENCODE_CONTAINER, ret, pError)) return ret;

			/* encode dictionary into message */
			switch(pDictionaryRefresh->type)
			{
				case RDM_DICTIONARY_FIELD_DEFINITIONS:
				{
					RsslRet dictEncodeRet;
					if ((dictEncodeRet = rsslEncodeFieldDictionary(pIter, pDictionaryRefresh->pDictionary, &pDictionaryRefresh->startFid, (RDMDictionaryVerbosityValues)pDictionaryRefresh->verbosity, &errorText)) != RSSL_RET_SUCCESS)
					{
						if (dictEncodeRet != RSSL_RET_DICT_PART_ENCODED)
						{
							rsslSetErrorInfo(pError, RSSL_EIC_FAILURE, RSSL_RET_FAILURE, __FILE__, __LINE__, "rsslEncodeFieldDictionary failed: %.*s", errorText.length, errorText.data);
							return dictEncodeRet;
						}
					}

					if (dictEncodeRet == RSSL_RET_SUCCESS)
						rsslSetRefreshCompleteFlag(pIter);

					if (!RSSL_ERROR_INFO_CHECK((ret = rsslEncodeMsgComplete(pIter, RSSL_TRUE)) == RSSL_RET_SUCCESS, ret, pError)) return ret;

					pError->rsslErrorInfoCode = RSSL_EIC_SUCCESS;
					*pBytesWritten = rsslGetEncodedBufferLength(pIter);
					return dictEncodeRet;
				}
				case RDM_DICTIONARY_ENUM_TABLES:
				{
					RsslRet dictEncodeRet;
					if ((dictEncodeRet = rsslEncodeEnumTypeDictionaryAsMultiPart(pIter, pDictionaryRefresh->pDictionary, &pDictionaryRefresh->enumStartFid, (RDMDictionaryVerbosityValues)pDictionaryRefresh->verbosity, &errorText)) != RSSL_RET_SUCCESS)
					{
						if (dictEncodeRet != RSSL_RET_DICT_PART_ENCODED)
						{
							rsslSetErrorInfo(pError, RSSL_EIC_FAILURE, RSSL_RET_FAILURE, __FILE__, __LINE__, "rsslEncodeEnumTypeDictionaryAsMultiPart failed: %.*s", errorText.length, errorText.data);
							return dictEncodeRet;
						}
					}

					if (dictEncodeRet == RSSL_RET_SUCCESS)
						rsslSetRefreshCompleteFlag(pIter);

					if (!RSSL_ERROR_INFO_CHECK((ret = rsslEncodeMsgComplete(pIter, RSSL_TRUE)) == RSSL_RET_SUCCESS, ret, pError)) return ret;

					pError->rsslErrorInfoCode = RSSL_EIC_SUCCESS;
					*pBytesWritten = rsslGetEncodedBufferLength(pIter);
					return dictEncodeRet;
				}
				default:
					pError->rsslErrorInfoCode = RSSL_EIC_FAILURE;
					rsslSetErrorInfo(pError, RSSL_EIC_FAILURE, RSSL_RET_FAILURE, __FILE__, __LINE__, "Unknown dictionary type: %llu", pDictionaryRefresh->type);
					return RSSL_RET_FAILURE;

			}
		}
		case RDM_DC_MT_STATUS:
		{
			RsslStatusMsg msg;
			RsslRDMDictionaryStatus *pDictionaryStatus = &pDictionaryMsg->status;

			rsslClearStatusMsg(&msg);
			/* Populate msgBase */
			msg.msgBase.msgClass = RSSL_MC_STATUS;
			msg.msgBase.streamId = pDictionaryStatus->rdmMsgBase.streamId;
			msg.msgBase.domainType = RSSL_DMT_DICTIONARY;
			msg.msgBase.containerType = RSSL_DT_NO_DATA;

			if (pDictionaryStatus->flags & RDM_DC_STF_HAS_STATE) 
			{
				msg.flags |= RSSL_STMF_HAS_STATE;
				msg.state = pDictionaryStatus->state;
			}
			if (!RSSL_ERROR_INFO_CHECK((ret = rsslEncodeMsg(pIter, (RsslMsg*)&msg)) == RSSL_RET_SUCCESS, ret, pError)) return ret;

			*pBytesWritten = rsslGetEncodedBufferLength(pIter);
			pError->rsslErrorInfoCode = RSSL_EIC_SUCCESS;
			return RSSL_RET_SUCCESS;
		}
		default:
			pError->rsslErrorInfoCode = RSSL_EIC_FAILURE;
			snprintf(pError->rsslError.text, MAX_RSSL_ERROR_TEXT, "Unknown dictionary msg type %d\n", pDictionaryMsg->rdmMsgBase.rdmMsgType);
			return RSSL_RET_FAILURE;
	}
}



RSSL_VA_API RsslRet rsslDecodeRDMDictionaryMsg(RsslDecodeIterator *pIter, RsslMsg *pMsg, RsslRDMDictionaryMsg *pDictionaryMsg, RsslBuffer *pMemoryBuffer, RsslErrorInfo *pError)
{
	RsslRet ret = 0;
	RsslMsgKey* key = 0;


	switch (pMsg->msgBase.msgClass)
	{
		case RSSL_MC_REQUEST:
		{
			RsslRDMDictionaryRequest *pDictionaryRequest = &pDictionaryMsg->request;
			/* get key */
			key = (RsslMsgKey *)rsslGetMsgKey(pMsg);

			rsslClearRDMDictionaryRequest(pDictionaryRequest);

			if (pMsg->requestMsg.flags & RSSL_RQMF_STREAMING)
				pDictionaryRequest->flags |= RDM_DC_RQF_STREAMING;

			if (!RSSL_ERROR_INFO_CHECK(key, RSSL_RET_FAILURE, pError)) return RSSL_RET_FAILURE;
			if (!RSSL_ERROR_INFO_CHECK(key->flags & RSSL_MKF_HAS_FILTER, RSSL_RET_FAILURE, pError)) return RSSL_RET_FAILURE;
			if (!RSSL_ERROR_INFO_CHECK(key->flags & RSSL_MKF_HAS_SERVICE_ID, RSSL_RET_FAILURE, pError)) return RSSL_RET_FAILURE;
			if (!RSSL_ERROR_INFO_CHECK(key->flags & RSSL_MKF_HAS_NAME, RSSL_RET_FAILURE, pError)) return RSSL_RET_FAILURE;

			pDictionaryRequest->serviceId = key->serviceId;
			pDictionaryRequest->dictionaryName = key->name;
			pDictionaryRequest->verbosity = key->filter;
			break;
		}
		case RSSL_MC_CLOSE:
		{
			rsslClearRDMDictionaryClose(&pDictionaryMsg->close);
			break;
		}
		case RSSL_MC_REFRESH:
		{
			RsslRDMDictionaryRefresh *pDictionaryRefresh = &pDictionaryMsg->refresh;
			RsslSeries series;
			RsslElementList elementList; RsslElementEntry elementEntry;
			RsslDecodeIterator tmpIter;

			RsslBool foundVersion = RSSL_FALSE, foundType = RSSL_FALSE; /* required */
			RsslBool foundDictionaryId = RSSL_FALSE; /* optional */

			rsslClearRDMDictionaryRefresh(pDictionaryRefresh);

			key = (RsslMsgKey *)rsslGetMsgKey(pMsg);
			if (!key || !(key->flags & RSSL_MKF_HAS_NAME) || !(key->flags & RSSL_MKF_HAS_FILTER) || !(key->flags & RSSL_MKF_HAS_SERVICE_ID))
				return RSSL_RET_FAILURE;

			pDictionaryRefresh->dataBody = pMsg->msgBase.encDataBody;

			pDictionaryRefresh->dictionaryName = key->name;
			pDictionaryRefresh->serviceId = key->serviceId;
			pDictionaryRefresh->verbosity = key->filter;
			pDictionaryRefresh->state = pMsg->refreshMsg.state;

			if (pMsg->refreshMsg.flags & RSSL_RFMF_SOLICITED)
				pDictionaryRefresh->flags |= RDM_DC_RFF_SOLICITED;

			if (pMsg->refreshMsg.flags & RSSL_RFMF_CLEAR_CACHE)
				pDictionaryRefresh->flags |= RDM_DC_RFF_CLEAR_CACHE;

			if (pMsg->refreshMsg.flags & RSSL_RFMF_REFRESH_COMPLETE)
				pDictionaryRefresh->flags |= RDM_DC_RFF_IS_COMPLETE;

			if (pMsg->refreshMsg.flags & RSSL_RFMF_HAS_SEQ_NUM)
			{
				pDictionaryRefresh->flags |= RDM_DC_RFF_HAS_SEQ_NUM;
				pDictionaryRefresh->sequenceNumber = pMsg->refreshMsg.seqNum;
			}

			rsslClearDecodeIterator(&tmpIter);
			rsslSetDecodeIteratorRWFVersion(&tmpIter, pIter->_majorVersion, pIter->_minorVersion);
			rsslSetDecodeIteratorBuffer(&tmpIter, &pMsg->msgBase.encDataBody);

			rsslClearSeries(&series);
			if (!RSSL_ERROR_INFO_CHECK((ret = rsslDecodeSeries(&tmpIter, &series)) == RSSL_RET_SUCCESS, ret, pError)) return RSSL_RET_FAILURE;

			/* If there is no summary data present, don't go looking for info. */
			if (series.flags & RSSL_SRF_HAS_SUMMARY_DATA)
			{
				pDictionaryRefresh->flags |= RDM_DC_RFF_HAS_INFO;

				rsslClearElementList(&elementList);
				if (!RSSL_ERROR_INFO_CHECK((ret = rsslDecodeElementList(&tmpIter, &elementList, 0)) == RSSL_RET_SUCCESS, ret, pError)) return RSSL_RET_FAILURE;

				if (!(elementList.flags & (RSSL_ELF_HAS_STANDARD_DATA | RSSL_ELF_HAS_SET_DATA)))
					return RSSL_RET_FAILURE;

				rsslClearElementEntry(&elementEntry);
				while ((ret = rsslDecodeElementEntry(&tmpIter, &elementEntry)) != RSSL_RET_END_OF_CONTAINER)
				{
					if (!RSSL_ERROR_INFO_CHECK(ret == RSSL_RET_SUCCESS, ret, pError)) return RSSL_RET_FAILURE;

					if (rsslBufferIsEqual(&elementEntry.name, &RSSL_ENAME_VERSION))
					{
						pDictionaryRefresh->version = elementEntry.encData;
						foundVersion = RSSL_TRUE;
					}
					else if (rsslBufferIsEqual(&elementEntry.name, &RSSL_ENAME_TYPE))
					{
						if (!RSSL_ERROR_INFO_CHECK((ret = rsslDecodeUInt(&tmpIter, &pDictionaryRefresh->type)) == RSSL_RET_SUCCESS, ret, pError)) return RSSL_RET_FAILURE;
						foundType = RSSL_TRUE;
					}
					else if (rsslBufferIsEqual(&elementEntry.name, &RSSL_ENAME_DICTIONARY_ID))
					{
						if (!RSSL_ERROR_INFO_CHECK((ret = rsslDecodeInt(&tmpIter, &pDictionaryRefresh->dictionaryId)) == RSSL_RET_SUCCESS, ret, pError)) return RSSL_RET_FAILURE;
						foundDictionaryId = RSSL_TRUE;
					}
				}
				if (!RSSL_ERROR_INFO_CHECK(foundVersion, RSSL_RET_FAILURE, pError)) return RSSL_RET_FAILURE;
				if (!RSSL_ERROR_INFO_CHECK(foundType, RSSL_RET_FAILURE, pError)) return RSSL_RET_FAILURE;
				if (!foundDictionaryId) pDictionaryRefresh->dictionaryId = 0;
			}
			break;
		}
		case RSSL_MC_STATUS:
		{
			RsslRDMDictionaryStatus *pDictionaryStatus = &pDictionaryMsg->status;
			
			rsslClearRDMDictionaryStatus(pDictionaryStatus);

			if (pMsg->statusMsg.flags & RSSL_STMF_HAS_STATE)
			{
				pDictionaryStatus->flags |= RDM_DC_STF_HAS_STATE;
				pDictionaryStatus->state = pMsg->statusMsg.state;
			}
			break;
		}
		default:
			pError->rsslErrorInfoCode = RSSL_EIC_FAILURE;
			rsslSetErrorInfo(pError, RSSL_EIC_FAILURE, RSSL_RET_FAILURE, __FILE__, __LINE__, "Unknown msg class %u", pMsg->msgBase.msgClass);
			return RSSL_RET_FAILURE;
	}

	pDictionaryMsg->rdmMsgBase.streamId = pMsg->msgBase.streamId;
	pError->rsslErrorInfoCode = RSSL_EIC_SUCCESS;
	return RSSL_RET_SUCCESS;
}

RSSL_VA_API RsslRet rsslCopyRDMDictionaryRequest(RsslRDMDictionaryRequest *pNewRequest, RsslRDMDictionaryRequest *pOldRequest, RsslBuffer *pNewMemoryBuffer)
{
	*pNewRequest = *pOldRequest;
	if (!rsslCopyBufferMemory(&pNewRequest->dictionaryName, &pOldRequest->dictionaryName, pNewMemoryBuffer))
		return RSSL_RET_BUFFER_TOO_SMALL;

	return RSSL_RET_SUCCESS;
}

RSSL_VA_API RsslRet rsslCopyRDMDictionaryClose(RsslRDMDictionaryClose *pNewClose, RsslRDMDictionaryClose *pOldClose, RsslBuffer *pNewMemoryBuffer)
{
	*pNewClose = *pOldClose;
	return RSSL_RET_SUCCESS;
}

RSSL_VA_API RsslRet rsslCopyRDMDictionaryRefresh(RsslRDMDictionaryRefresh *pNewRefresh, RsslRDMDictionaryRefresh *pOldRefresh, RsslBuffer *pNewMemoryBuffer)
{
	*pNewRefresh = *pOldRefresh;

	if (!rsslCopyBufferMemory(&pNewRefresh->dictionaryName, &pOldRefresh->dictionaryName, pNewMemoryBuffer))
		return RSSL_RET_BUFFER_TOO_SMALL;

	if (!rsslCopyBufferMemory(&pNewRefresh->version, &pOldRefresh->version, pNewMemoryBuffer))
		return RSSL_RET_BUFFER_TOO_SMALL;

	if (!rsslCopyBufferMemory(&pNewRefresh->state.text, &pOldRefresh->state.text, pNewMemoryBuffer))
		return RSSL_RET_BUFFER_TOO_SMALL;
	
	if (!rsslCopyBufferMemory(&pNewRefresh->dataBody, &pOldRefresh->dataBody, pNewMemoryBuffer))
		return RSSL_RET_BUFFER_TOO_SMALL;

	return RSSL_RET_SUCCESS;
}

RSSL_VA_API RsslRet rsslCopyRDMDictionaryStatus(RsslRDMDictionaryStatus *pNewStatus, RsslRDMDictionaryStatus *pOldStatus, RsslBuffer *pNewMemoryBuffer)
{
	/* Copy members */
	*pNewStatus = *pOldStatus;

	/* Create separate copies of strings */
	if (pOldStatus->flags & RDM_DC_STF_HAS_STATE)
	{
		if (!rsslCopyBufferMemory(&pNewStatus->state.text, &pOldStatus->state.text, pNewMemoryBuffer))
			return RSSL_RET_BUFFER_TOO_SMALL;
	}
	return RSSL_RET_SUCCESS;
}

RSSL_VA_API RsslRet rsslCopyRDMDictionaryMsg(RsslRDMDictionaryMsg *pNewMsg, RsslRDMDictionaryMsg *pOldMsg, RsslBuffer *pNewMemoryBuffer)
{
	switch(pOldMsg->rdmMsgBase.rdmMsgType)
	{
		case RDM_DC_MT_REQUEST: return rsslCopyRDMDictionaryRequest(&pNewMsg->request, &pOldMsg->request, pNewMemoryBuffer);
		case RDM_DC_MT_CLOSE: return rsslCopyRDMDictionaryClose(&pNewMsg->close, &pOldMsg->close, pNewMemoryBuffer);
		case RDM_DC_MT_REFRESH: return rsslCopyRDMDictionaryRefresh(&pNewMsg->refresh, &pOldMsg->refresh, pNewMemoryBuffer);
		case RDM_DC_MT_STATUS: return rsslCopyRDMDictionaryStatus(&pNewMsg->status, &pOldMsg->status, pNewMemoryBuffer);
		default: return RSSL_RET_INVALID_ARGUMENT;
	}
}
