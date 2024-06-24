/*
 * This source code is provided under the Apache 2.0 license and is provided
 * AS IS with no warranty or guarantee of fit for purpose.  See the project's 
 * LICENSE.md for details. 
 * Copyright (C) 2024 LSEG. All rights reserved.
*/

/* Provides internal decoding of QueueMsgs */

#include "rtr/rsslRDMQueueMsgInt.h"

RSSL_VA_API RsslRet rsslDecodeRDMQueueMsg(RsslDecodeIterator *pIter, RsslMsg *pMsg, RsslRDMQueueMsg *pQueueMsg, RsslBuffer *pMemoryBuffer, RsslErrorInfo *pError)
{
	switch(pMsg->msgBase.msgClass)
	{
		case RSSL_MC_GENERIC:
		{
			RsslRDMQueueData *pQueueData = (RsslRDMQueueData*)pQueueMsg;
			RsslGenericMsg *pGenericMsg = (RsslGenericMsg*)pMsg;
			RsslBuffer tmpBuffer;
			RsslUInt32 tmpPos;
			RsslUInt32 msgLength;
			char *pData;

			if (rsslGenericMsgCheckHasExtendedHdr(pGenericMsg) == RSSL_FALSE)
			{
				rsslSetErrorInfo(pError, RSSL_EIC_FAILURE, RSSL_RET_INCOMPLETE_DATA, __FILE__, __LINE__,
						"Message is missing extended header.");
				return RSSL_RET_INCOMPLETE_DATA;
			}

			if (rsslGenericMsgCheckHasMsgKey(pGenericMsg) == RSSL_FALSE
					|| rsslMsgKeyCheckHasName(&pGenericMsg->msgBase.msgKey) == RSSL_FALSE)
			{
				rsslSetErrorInfo(pError, RSSL_EIC_FAILURE, RSSL_RET_INCOMPLETE_DATA, __FILE__, __LINE__,
						"Message is missing name.");
				return RSSL_RET_INCOMPLETE_DATA;
			}

			
			rsslClearRDMQueueData(pQueueData);

			pQueueData->rdmMsgBase.rdmMsgType = RDM_QMSG_MT_DATA;
			pQueueData->rdmMsgBase.streamId = pGenericMsg->msgBase.streamId;
			pQueueData->rdmMsgBase.domainType = pGenericMsg->msgBase.domainType;

			/* Decode extended header. */
			tmpBuffer = pMsg->genericMsg.extendedHeader;
			tmpPos = 0;
			pData = tmpBuffer.data;
										
			pQueueData->containerType = pGenericMsg->msgBase.containerType;

			pQueueData->destName = pGenericMsg->msgBase.msgKey.name;

			pQueueData->encDataBody = pGenericMsg->msgBase.encDataBody;

			if (tmpBuffer.length - tmpPos < 1)
			{
				rsslSetErrorInfo(pError, RSSL_EIC_FAILURE, RSSL_RET_INCOMPLETE_DATA, __FILE__, __LINE__,
						"Header incomplete.");
				return RSSL_RET_INCOMPLETE_DATA;
			}

			/* fromQueue length */
			msgLength = (unsigned char)pData[tmpPos++];

			if (tmpBuffer.length - tmpPos < msgLength)
			{
				rsslSetErrorInfo(pError, RSSL_EIC_FAILURE, RSSL_RET_INCOMPLETE_DATA, __FILE__, __LINE__,
						"Header incomplete.");
				return RSSL_RET_INCOMPLETE_DATA;
			}

			/* fromQueue */
			pQueueData->sourceName.data = &pData[tmpPos];
			pQueueData->sourceName.length = msgLength;
			tmpPos += msgLength;

			if (tmpBuffer.length - tmpPos < 16)
			{
				rsslSetErrorInfo(pError, RSSL_EIC_FAILURE, RSSL_RET_INCOMPLETE_DATA, __FILE__, __LINE__,
						"Header incomplete.");
				return RSSL_RET_INCOMPLETE_DATA;
			}

			/* Timeout  */
			memcpy(&pQueueData->timeout, &pData[tmpPos], 8);
			tmpPos += 8;

			/* Identifier */
			memcpy(&pQueueData->identifier, &pData[tmpPos], 8);
			tmpPos += 8;

			return RSSL_RET_SUCCESS;
		}

		default:
		{
			rsslSetErrorInfo(pError, RSSL_EIC_FAILURE, RSSL_RET_FAILURE, __FILE__, __LINE__,
					"Unknown queue message type: %u\n", pMsg->msgBase.msgClass);
			return RSSL_RET_FAILURE;
		}
	}
}
