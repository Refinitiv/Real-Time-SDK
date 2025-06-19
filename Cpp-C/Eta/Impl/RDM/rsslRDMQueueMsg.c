/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2015,2019,2024 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

#include "rtr/rsslRDMQueueMsg.h"

RSSL_VA_API RsslRet rsslEncodeRDMQueueMsg(RsslEncodeIterator *pEncodeIter, RsslRDMQueueMsg *pQueueMsg, RsslUInt32 *pBytesWritten, RsslErrorInfo *pError)
{
	switch(pQueueMsg->rdmMsgBase.rdmMsgType)
	{
		case RDM_QMSG_MT_DATA:
		{
			RsslRDMQueueData *pQueueData = (RsslRDMQueueData*)pQueueMsg;
			RsslBuffer tmpBuffer;
			int ret, encodedLen = 0;
			unsigned char *pData = NULL;


			if (pQueueData->containerType != RSSL_DT_NO_DATA)
			{
				/* Encode message. */
				if ((ret = rsslEncodeRDMQueueMsgInit(pEncodeIter, pQueueMsg, pError)) != RSSL_RET_ENCODE_CONTAINER)
					return ret;
			
				/* Encode data body. */
				if ((ret = rsslEncodeNonRWFDataTypeInit(pEncodeIter, &tmpBuffer)) != RSSL_RET_SUCCESS)
				{
					rsslSetErrorInfo(pError, RSSL_EIC_FAILURE, ret, 
							__FILE__, __LINE__, "rsslEncodeNonRWFDataTypeInit failed.");
					return (ret == RSSL_RET_BUFFER_TOO_SMALL) ? RSSL_RET_BUFFER_TOO_SMALL : RSSL_RET_FAILURE;
				}

				if (pQueueData->encDataBody.length > tmpBuffer.length)
				{
					rsslSetErrorInfo(pError, RSSL_EIC_FAILURE, RSSL_RET_BUFFER_TOO_SMALL, 
							__FILE__, __LINE__, "Buffer too small for encoding data body.");
					return RSSL_RET_BUFFER_TOO_SMALL;
				}

				memcpy(tmpBuffer.data, pQueueData->encDataBody.data,
						pQueueData->encDataBody.length);
				tmpBuffer.length = pQueueData->encDataBody.length;

				if ((ret = rsslEncodeNonRWFDataTypeComplete(pEncodeIter, &tmpBuffer, RSSL_TRUE)) != RSSL_RET_SUCCESS)
				{
					rsslSetErrorInfo(pError, RSSL_EIC_FAILURE, ret, 
							__FILE__, __LINE__, "rsslEncodeNonRWFDataTypeComplete failed.");
					return (ret == RSSL_RET_BUFFER_TOO_SMALL) ? RSSL_RET_BUFFER_TOO_SMALL : RSSL_RET_FAILURE;
				}
			}
			else
			{
				/* Encode message. */
				if ((ret = rsslEncodeRDMQueueMsgInit(pEncodeIter, pQueueMsg, pError)) != RSSL_RET_SUCCESS)
					return ret;
			}

			return rsslEncodeRDMQueueMsgComplete(pEncodeIter, RSSL_TRUE, pBytesWritten, pError);
		}

		case RDM_QMSG_MT_REQUEST:
		{
			pError->rsslErrorInfoCode = RSSL_EIC_FAILURE;
			rsslSetErrorInfo(pError, RSSL_EIC_FAILURE, RSSL_RET_INVALID_ARGUMENT, __FILE__, __LINE__, "Encoding of RsslRDMQueueRequest is not supported, please submit using rsslTunnelStreamSubmitMsg.");
			return RSSL_RET_INVALID_ARGUMENT;
		}

		case RDM_QMSG_MT_CLOSE:
		{
			pError->rsslErrorInfoCode = RSSL_EIC_FAILURE;
			rsslSetErrorInfo(pError, RSSL_EIC_FAILURE, RSSL_RET_INVALID_ARGUMENT, __FILE__, __LINE__, "Encoding of RsslRDMQueueClose is not supported, please submit using rsslTunnelStreamSubmitMsg.");
			return RSSL_RET_INVALID_ARGUMENT;
		}

		case RDM_QMSG_MT_REFRESH:
		{
			pError->rsslErrorInfoCode = RSSL_EIC_FAILURE;
			rsslSetErrorInfo(pError, RSSL_EIC_FAILURE, RSSL_RET_INVALID_ARGUMENT, __FILE__, __LINE__, "Encoding of RsslRDMQueueRefresh is not supported.");
			return RSSL_RET_INVALID_ARGUMENT;
		}

		case RDM_QMSG_MT_STATUS:
		{
			pError->rsslErrorInfoCode = RSSL_EIC_FAILURE;
			rsslSetErrorInfo(pError, RSSL_EIC_FAILURE, RSSL_RET_INVALID_ARGUMENT, __FILE__, __LINE__, "Encoding of RsslRDMQueueStatus is not supported.");
			return RSSL_RET_INVALID_ARGUMENT;
		}

		case RDM_QMSG_MT_DATA_EXPIRED:
		{
			pError->rsslErrorInfoCode = RSSL_EIC_FAILURE;
			rsslSetErrorInfo(pError, RSSL_EIC_FAILURE, RSSL_RET_INVALID_ARGUMENT, __FILE__, __LINE__, "Encoding of RsslRDMQueueDataExpired is not supported.");
			return RSSL_RET_INVALID_ARGUMENT;
		}

		case RDM_QMSG_MT_ACK:
		{
			pError->rsslErrorInfoCode = RSSL_EIC_FAILURE;
			rsslSetErrorInfo(pError, RSSL_EIC_FAILURE, RSSL_RET_INVALID_ARGUMENT, __FILE__, __LINE__, "Encoding of RsslRDMQueueAck is not supported.");
			return RSSL_RET_INVALID_ARGUMENT;
		}

		default:
		{
			pError->rsslErrorInfoCode = RSSL_EIC_FAILURE;
			rsslSetErrorInfo(pError, RSSL_EIC_FAILURE, RSSL_RET_INVALID_ARGUMENT, __FILE__, __LINE__, "Unknown queue msg type %d\n", pQueueMsg->rdmMsgBase.rdmMsgType);
			return RSSL_RET_INVALID_ARGUMENT;
		}
	}
}

RSSL_VA_API RsslRet rsslEncodeRDMQueueMsgInit(RsslEncodeIterator *pEncodeIter, RsslRDMQueueMsg *pQueueMsg, RsslErrorInfo *pError)
{
	switch(pQueueMsg->rdmMsgBase.rdmMsgType)
	{
		case RDM_QMSG_MT_DATA:
		{
			RsslRDMQueueData *pQueueData = (RsslRDMQueueData*)pQueueMsg;
			RsslBuffer tmpBuffer;
			RsslRet expectedRet;
			int ret, encodedLen = 0;
			unsigned char *pData = NULL;

			if (pQueueData->destName.length > RDM_QMSG_MAX_NAME_LENGTH)
			{
				rsslSetErrorInfo(pError, RSSL_EIC_FAILURE, RSSL_RET_INVALID_ARGUMENT, 
						__FILE__, __LINE__, "destName is too long.");
				return RSSL_RET_INVALID_ARGUMENT;
			}

			if (pQueueData->sourceName.length > RDM_QMSG_MAX_NAME_LENGTH)
			{
				rsslSetErrorInfo(pError, RSSL_EIC_FAILURE, RSSL_RET_INVALID_ARGUMENT, 
						__FILE__, __LINE__, "sourceName is too long.");
				return RSSL_RET_INVALID_ARGUMENT;
			}

			rsslClearGenericMsg(&pQueueData->_encodeMsg);
			pQueueData->_encodeMsg.msgBase.msgClass = RSSL_MC_GENERIC;
			pQueueData->_encodeMsg.msgBase.streamId = pQueueData->rdmMsgBase.streamId;
			pQueueData->_encodeMsg.msgBase.containerType = pQueueData->containerType;
			pQueueData->_encodeMsg.msgBase.domainType = pQueueData->rdmMsgBase.domainType;
			rsslGenericMsgApplyHasExtendedHdr(&pQueueData->_encodeMsg);
			rsslGenericMsgApplyMessageComplete(&pQueueData->_encodeMsg);

			rsslGenericMsgApplyHasMsgKey(&pQueueData->_encodeMsg);
			rsslMsgKeyApplyHasName(&pQueueData->_encodeMsg.msgBase.msgKey);
			pQueueData->_encodeMsg.msgBase.msgKey.name = pQueueData->destName;

			if ((ret = rsslEncodeMsgInit(pEncodeIter, (RsslMsg *)&pQueueData->_encodeMsg, 0)) != RSSL_RET_ENCODE_EXTENDED_HEADER)
			{
				rsslSetErrorInfo(pError, RSSL_EIC_FAILURE, ret, 
						__FILE__, __LINE__, "rsslEncodeMsgInit failed.");
				return (ret == RSSL_RET_BUFFER_TOO_SMALL) ? RSSL_RET_BUFFER_TOO_SMALL : RSSL_RET_FAILURE;
			}

			if ((ret = rsslEncodeNonRWFDataTypeInit(pEncodeIter, &tmpBuffer)) != RSSL_RET_SUCCESS)
			{
				rsslSetErrorInfo(pError, RSSL_EIC_FAILURE, ret, 
						__FILE__, __LINE__, "rsslEncodeNonRWFDataTypeInit failed.");
				return (ret == RSSL_RET_BUFFER_TOO_SMALL) ? RSSL_RET_BUFFER_TOO_SMALL : RSSL_RET_FAILURE;
			}

			if (tmpBuffer.length < 1 + 1 + pQueueData->sourceName.length + 8 + 8 + 1)
			{
				rsslSetErrorInfo(pError, RSSL_EIC_FAILURE, RSSL_RET_BUFFER_TOO_SMALL, 
						__FILE__, __LINE__, "Buffer too small for encoding message header.");
				return RSSL_RET_BUFFER_TOO_SMALL;
			}


			pData = (unsigned char*)tmpBuffer.data;

			/* From queue */
			pData[encodedLen++] = pQueueData->sourceName.length;

			memcpy(&pData[encodedLen], pQueueData->sourceName.data, pQueueData->sourceName.length);
			encodedLen += pQueueData->sourceName.length;

			/* Timeout */
			memcpy(&pData[encodedLen], &pQueueData->timeout, 8);
			encodedLen += 8;

			/* Identifier */
			memcpy(&pData[encodedLen], &pQueueData->identifier, 8);
			encodedLen += 8;

			tmpBuffer.length = encodedLen;

			if ((ret = rsslEncodeNonRWFDataTypeComplete(pEncodeIter, &tmpBuffer, RSSL_TRUE)) != RSSL_RET_SUCCESS)
			{
				rsslSetErrorInfo(pError, RSSL_EIC_FAILURE, ret, 
						__FILE__, __LINE__, "rsslEncodeNonRWFDataTypeComplete failed.");
				return (ret == RSSL_RET_BUFFER_TOO_SMALL) ? RSSL_RET_BUFFER_TOO_SMALL : RSSL_RET_FAILURE;
			}

			expectedRet = (pQueueData->_encodeMsg.msgBase.containerType == RSSL_DT_NO_DATA) ? RSSL_RET_SUCCESS : RSSL_RET_ENCODE_CONTAINER;


			if ((ret = rsslEncodeExtendedHeaderComplete(pEncodeIter,  RSSL_TRUE)) != expectedRet)
			{
				rsslSetErrorInfo(pError, RSSL_EIC_FAILURE, ret, 
						__FILE__, __LINE__, "rsslEncodeExtendedHeaderComplete failed.");
				return (ret == RSSL_RET_BUFFER_TOO_SMALL) ? RSSL_RET_BUFFER_TOO_SMALL : RSSL_RET_FAILURE;
			}

			pError->rsslErrorInfoCode = RSSL_EIC_SUCCESS;
			return expectedRet;
		}

		case RDM_QMSG_MT_REQUEST:
		{
			pError->rsslErrorInfoCode = RSSL_EIC_FAILURE;
			rsslSetErrorInfo(pError, RSSL_EIC_FAILURE, RSSL_RET_INVALID_ARGUMENT, __FILE__, __LINE__, "Encoding of RsslRDMQueueRequest is not supported, please submit using rsslTunnelStreamSubmitMsg.");
			return RSSL_RET_INVALID_ARGUMENT;
		}

		case RDM_QMSG_MT_CLOSE:
		{
			pError->rsslErrorInfoCode = RSSL_EIC_FAILURE;
			rsslSetErrorInfo(pError, RSSL_EIC_FAILURE, RSSL_RET_INVALID_ARGUMENT, __FILE__, __LINE__, "Encoding of RsslRDMQueueClose is not supported, please submit using rsslTunnelStreamSubmitMsg.");
			return RSSL_RET_INVALID_ARGUMENT;
		}

		case RDM_QMSG_MT_REFRESH:
		{
			pError->rsslErrorInfoCode = RSSL_EIC_FAILURE;
			rsslSetErrorInfo(pError, RSSL_EIC_FAILURE, RSSL_RET_INVALID_ARGUMENT, __FILE__, __LINE__, "Encoding of RsslRDMQueueRefresh is not supported.");
			return RSSL_RET_INVALID_ARGUMENT;
		}

		case RDM_QMSG_MT_STATUS:
		{
			pError->rsslErrorInfoCode = RSSL_EIC_FAILURE;
			rsslSetErrorInfo(pError, RSSL_EIC_FAILURE, RSSL_RET_INVALID_ARGUMENT, __FILE__, __LINE__, "Encoding of RsslRDMQueueStatus is not supported.");
			return RSSL_RET_INVALID_ARGUMENT;
		}

		case RDM_QMSG_MT_DATA_EXPIRED:
		{
			pError->rsslErrorInfoCode = RSSL_EIC_FAILURE;
			rsslSetErrorInfo(pError, RSSL_EIC_FAILURE, RSSL_RET_INVALID_ARGUMENT, __FILE__, __LINE__, "Encoding of RsslRDMQueueDataExpired is not supported.");
			return RSSL_RET_INVALID_ARGUMENT;
		}

		case RDM_QMSG_MT_ACK:
		{
			pError->rsslErrorInfoCode = RSSL_EIC_FAILURE;
			rsslSetErrorInfo(pError, RSSL_EIC_FAILURE, RSSL_RET_INVALID_ARGUMENT, __FILE__, __LINE__, "Encoding of RsslRDMQueueAck is not supported.");
			return RSSL_RET_INVALID_ARGUMENT;
		}

		default:
		{
			rsslSetErrorInfo(pError, RSSL_EIC_FAILURE, RSSL_RET_INVALID_ARGUMENT, __FILE__, __LINE__, "Unknown queue msg type %d\n", pQueueMsg->rdmMsgBase.rdmMsgType);
			return RSSL_RET_INVALID_ARGUMENT;
		}
	}
}

RSSL_VA_API RsslRet rsslEncodeRDMQueueMsgComplete(RsslEncodeIterator *pEncodeIter, RsslBool success, RsslUInt32 *pBytesWritten, RsslErrorInfo *pError)
{
	RsslRet ret;

	if ((ret = rsslEncodeMsgComplete(pEncodeIter, success)) != RSSL_RET_SUCCESS)
	{
		rsslSetErrorInfo(pError, RSSL_EIC_FAILURE, ret, 
				__FILE__, __LINE__, "rsslEncodeMsgComplete failed.");
		return (ret == RSSL_RET_BUFFER_TOO_SMALL) ? RSSL_RET_BUFFER_TOO_SMALL : RSSL_RET_FAILURE;
	}

	if (success)
		*pBytesWritten = rsslGetEncodedBufferLength(pEncodeIter);

	pError->rsslErrorInfoCode = RSSL_EIC_SUCCESS;
	return RSSL_RET_SUCCESS;

}

RSSL_VA_API const char* rsslRDMQueueDataMsgTypeToString(RsslRDMQueueDataMsgType type)
{
	switch(type)
	{
		case RDM_QMSG_DATA_MT_INIT:				return "RDM_QMSG_DATA_MT_INIT";
		case RDM_QMSG_DATA_MT_DATA:				return "RDM_QMSG_DATA_MT_DATA";
		case RDM_QMSG_DATA_MT_UNDELIVERABLE:	return "RDM_QMSG_DATA_MT_UNDELIVERABLE";
		default:								return "Unknown";
	}
}

RSSL_VA_API const char* rsslRDMQueueUndeliverableCodeToString(RsslUInt8 code)
{
	switch(code)
	{
		case RDM_QMSG_UC_UNSPECIFIED:		return "RDM_QMSG_UC_UNSPECIFIED";
		case RDM_QMSG_UC_EXPIRED:			return "RDM_QMSG_UC_EXPIRED";
		case RDM_QMSG_UC_NO_PERMISSION:		return "RDM_QMSG_UC_NO_PERMISSION";
		case RDM_QMSG_UC_INVALID_TARGET:	return "RDM_QMSG_UC_INVALID_TARGET";
		case RDM_QMSG_UC_QUEUE_FULL:		return "RDM_QMSG_UC_QUEUE_FULL";
		case RDM_QMSG_UC_QUEUE_DISABLED:	return "RDM_QMSG_UC_QUEUE_DISABLED";
		case RDM_QMSG_UC_MAX_MSG_SIZE:		return "RDM_QMSG_UC_MAX_MSG_SIZE";
		case RDM_QMSG_UC_INVALID_SENDER:	return "RDM_QMSG_UC_INVALID_SENDER";
		case RDM_QMSG_UC_TARGET_DELETED:	return "RDM_QMSG_UC_TARGET_DELETED";
		default:							return "Unknown";
	}
}

RSSL_VA_API const char* rsslRDMQueueTimeoutCodeToString(RsslRDMQueueTimeoutCode code)
{
	switch(code)
	{
		case RDM_QMSG_TC_PROVIDER_DEFAULT:		return "RDM_QMSG_TC_PROVIDER_DEFAULT";
		case RDM_QMSG_TC_INFINITE:				return "RDM_QMSG_TC_INFINITE";
		case RDM_QMSG_TC_IMMEDIATE:				return "RDM_QMSG_TC_IMMEDIATE";
		default:								return "Unknown";
	}
}

RSSL_VA_API const char* rsslRDMQueueMsgTypeToString(RsslRDMQueueMsgType type)
{
	switch(type)
	{
		case RDM_QMSG_MT_UNKNOWN:	return "RDM_QMSG_MT_UNKNOWN";
		case RDM_QMSG_MT_REQUEST:	return "RDM_QMSG_MT_REQUEST";
		case RDM_QMSG_MT_REFRESH:	return "RDM_QMSG_MT_REFRESH";
		case RDM_QMSG_MT_STATUS:	return "RDM_QMSG_MT_STATUS";
		case RDM_QMSG_MT_DATA:		return "RDM_QMSG_MT_DATA";
		case RDM_QMSG_MT_ACK:		return "RDM_QMSG_MT_ACK";
		case RDM_QMSG_MT_CLOSE:		return "RDM_QMSG_MT_CLOSE";
		default:					return "Unknown";
	}
}
