/*
 * This source code is provided under the Apache 2.0 license and is provided
 * AS IS with no warranty or guarantee of fit for purpose.  See the project's 
 * LICENSE.md for details. 
 * Copyright (C) 2019 LSEG. All rights reserved.
*/

#include "rtr/persistFile.h"
#include "rtr/tunnelSubstream.h"
#include "rtr/tunnelStreamImpl.h"
#include "rtr/tunnelStreamReturnCodes.h"
#include "rtr/msgQueueSubstreamHeader.h"
#include "rtr/msgQueueEncDec.h"
#include "rtr/rsslHeapBuffer.h"

#include <stdlib.h>
#include <assert.h>

typedef struct
{
	TunnelSubstream			base;
	TunnelStreamImpl*		_tunnelImpl;
	RsslUInt32				_lastOutSeqNum;
	RsslUInt32				_lastInSeqNum;
	PersistFile				*_pPersistFile;
	char					_sourceNameMemory[256];
	RsslUInt16				_lastQueueDepth;
} TunnelSubstreamImpl;

static RsslRet _tunnelSubstreamEnqueueDataBuffer(TunnelSubstreamImpl *pSubstreamImpl, TunnelBufferImpl *pBufferImpl, RsslErrorInfo *pErrorInfo);

/* Reads a PersistentMsg into a buffer. */
static TunnelBufferImpl* _tunnelSubstreamLoadSavedMsgToTunnelBuffer(TunnelSubstreamImpl *pSubstreamImpl, PersistentMsg *pMsg, RsslErrorInfo *pErrorInfo);

/* Calls the QueueMsgCallback with the given message. Handles a RAISE return code. */
RTR_C_INLINE RsslRet tunnelSubstreamCallQueueCallback(TunnelSubstreamImpl *pSubstreamImpl,
		RsslMsg *pRsslMsg, RsslRDMQueueMsg *pQueueMsg, RsslBool isLocallyGenerated, 
		RsslBuffer *pAckSourceName, RsslBuffer *pAckDestName, RsslUInt32 ackSeqNum,
		RsslErrorInfo *pErrorInfo);

TunnelSubstream *tunnelSubstreamOpen(RsslTunnelStream *pTunnel,
		RsslRDMQueueRequest *pRequestMsg, RsslErrorInfo *pErrorInfo)
{
	TunnelSubstreamImpl				*pSubstreamImpl;
	MsgQueueSubstreamRequestHeader	requestMsg;
	RsslEncodeIterator				eIter;
	RsslBuffer*						pBuffer;
	RsslRet							ret;
	RsslUInt32						length;


	if (pRequestMsg->sourceName.data == NULL)
	{
		rsslSetErrorInfo(pErrorInfo, RSSL_EIC_FAILURE, RSSL_RET_INVALID_ARGUMENT, 
				__FILE__, __LINE__, "Queue source name not provided.");
		return NULL;
	}

	if (pRequestMsg->sourceName.length > RDM_QMSG_MAX_NAME_LENGTH)
	{
		rsslSetErrorInfo(pErrorInfo, RSSL_EIC_FAILURE, RSSL_RET_INVALID_ARGUMENT, 
				__FILE__, __LINE__, "Queue source name is too long.");
		return NULL;
	}

	if ((pSubstreamImpl = (TunnelSubstreamImpl*)malloc(sizeof(TunnelSubstreamImpl)))
				== NULL)
	{
		rsslSetErrorInfo(pErrorInfo, RSSL_EIC_FAILURE, RSSL_RET_FAILURE, 
				__FILE__, __LINE__, "Failed to allocate queue stream object.");
		return NULL;
	}

	memset(pSubstreamImpl, 0, sizeof(TunnelSubstreamImpl));
	pSubstreamImpl->_tunnelImpl = (TunnelStreamImpl*)pTunnel;

	memcpy(pSubstreamImpl->_sourceNameMemory,
			pRequestMsg->sourceName.data,
			pRequestMsg->sourceName.length);
	pSubstreamImpl->base._sourceName.data = pSubstreamImpl->_sourceNameMemory;
	pSubstreamImpl->base._sourceName.length = pRequestMsg->sourceName.length;
	pSubstreamImpl->base._sourceName.data[pRequestMsg->sourceName.length] = '\0';

	/* Copy request information */
	pSubstreamImpl->base._streamId = pRequestMsg->rdmMsgBase.streamId;
	pSubstreamImpl->base._domainType = pRequestMsg->rdmMsgBase.domainType;

	msgQueueClearSubstreamRequestHeader(&requestMsg);
	requestMsg.base.streamId = pSubstreamImpl->base._streamId;
	requestMsg.base.domainType = pSubstreamImpl->base._domainType;
	requestMsg.fromQueue = pSubstreamImpl->base._sourceName;

	if (pSubstreamImpl->_tunnelImpl->_persistLocally)
	{
		PersistFileOpenOptions pfOpts;
		char *persistenceFilePath = tunnelStreamGetPersistenceFilePath(pSubstreamImpl->_tunnelImpl);

		persistFileOpenOptionsClear(&pfOpts);

		pfOpts.streamId = pRequestMsg->rdmMsgBase.streamId;

		if (persistenceFilePath != NULL)
		{
			RsslUInt32 tmpLen = (RsslUInt32)strlen(persistenceFilePath) + pRequestMsg->sourceName.length + 2;
			RsslBuffer *pTunnelMemoryBuffer = tunnelStreamGetMemoryHeapBuffer(pSubstreamImpl->_tunnelImpl);

			/* Concatenate the file path and queue name. */

			if (rsslHeapBufferResize(pTunnelMemoryBuffer,
						tmpLen, RSSL_FALSE)
					!= RSSL_RET_SUCCESS)
			{
				rsslSetErrorInfo(pErrorInfo, RSSL_EIC_FAILURE, RSSL_RET_FAILURE, __FILE__, __LINE__, 
						"Memory allocation failed.");
				return NULL;
			}

			snprintf(pTunnelMemoryBuffer->data, tmpLen, "%s%s%s", persistenceFilePath, 
#ifdef WIN32
					"\\", 
#else
					"/", 
#endif
					pSubstreamImpl->base._sourceName.data);

			pfOpts.filename = pTunnelMemoryBuffer->data;
		}
		else
		{
			pfOpts.filename = pSubstreamImpl->base._sourceName.data;
		}

		pfOpts.currentTimeMs = tunnelStreamGetCurrentTimeMs(pSubstreamImpl->_tunnelImpl);
		pfOpts.maxMsgSize = (RsslUInt32)pSubstreamImpl->_tunnelImpl->base.classOfService.common.maxFragmentSize;

		pSubstreamImpl->_pPersistFile = persistFileOpen(&pfOpts, &pSubstreamImpl->_lastInSeqNum, &pSubstreamImpl->_lastOutSeqNum, pErrorInfo);

		if (pSubstreamImpl->_pPersistFile == NULL)
		{
			tunnelSubstreamClose((TunnelSubstream*)pSubstreamImpl, pErrorInfo);
			tunnelSubstreamDestroy((TunnelSubstream*)pSubstreamImpl);
			return NULL;
		}

		requestMsg.lastOutSeqNum = pSubstreamImpl->_lastOutSeqNum;
		requestMsg.lastInSeqNum = pSubstreamImpl->_lastInSeqNum;
	}
	else
	{
		requestMsg.lastOutSeqNum = 0;
		requestMsg.lastInSeqNum = 0;
	}

	length = msgQueueSubstreamRequestHeaderBufferSize(&requestMsg);
	if (length > pSubstreamImpl->_tunnelImpl->base.classOfService.common.maxFragmentSize)
		length = (RsslUInt32)pSubstreamImpl->_tunnelImpl->base.classOfService.common.maxFragmentSize;

	/* Queue request in tunnel stream. */
	if ((pBuffer = (RsslBuffer*)tunnelStreamGetBuffer(pSubstreamImpl->_tunnelImpl,
					length, RSSL_FALSE, RSSL_FALSE, pErrorInfo)) == NULL)
	{
		tunnelSubstreamClose((TunnelSubstream*)pSubstreamImpl, pErrorInfo);
		tunnelSubstreamDestroy((TunnelSubstream*)pSubstreamImpl);
		return NULL;
	}

	rsslClearEncodeIterator(&eIter);
	rsslSetEncodeIteratorRWFVersion(&eIter, pTunnel->classOfService.common.protocolMajorVersion,
			pTunnel->classOfService.common.protocolMinorVersion);
	rsslSetEncodeIteratorBuffer(&eIter, pBuffer);

	/* Encode request */
	if ((ret = rsslEncodeMsgQueueSubstreamRequestHeader(&eIter,
					&requestMsg)) != RSSL_RET_SUCCESS)
	{
		tunnelSubstreamClose((TunnelSubstream*)pSubstreamImpl, pErrorInfo);
		tunnelSubstreamDestroy((TunnelSubstream*)pSubstreamImpl);
		rsslTunnelStreamReleaseBuffer(pBuffer, pErrorInfo);
		return NULL;
	}

	pBuffer->length = rsslGetEncodedBufferLength(&eIter);

	/* Add request to tunnel queue. */
	if ((ret = tunnelStreamEnqueueBuffer(&pSubstreamImpl->_tunnelImpl->base,
					pBuffer, RSSL_DT_MSG, pErrorInfo)) != RSSL_RET_SUCCESS)
	{
		tunnelSubstreamClose((TunnelSubstream*)pSubstreamImpl, pErrorInfo);
		tunnelSubstreamDestroy((TunnelSubstream*)pSubstreamImpl);
		rsslTunnelStreamReleaseBuffer(pBuffer, pErrorInfo);
		return NULL;
	}

	/* Now wait for refresh. */
	pSubstreamImpl->base._state = SBS_WAIT_REFRESH;

	return (TunnelSubstream*)pSubstreamImpl;
}

RsslRet tunnelSubstreamExpireBuffer(TunnelSubstream *pSubstream, RsslBuffer *pTunnelBuffer,
		PersistentMsg *pPersistentMsg, RsslUInt8 undeliverableCode, RsslErrorInfo *pErrorInfo)
{
	TunnelSubstreamImpl *pSubstreamImpl =  (TunnelSubstreamImpl*)pSubstream;
	RsslMsg rsslMsg;
	RsslDecodeIterator dIter;
	RsslRet ret;
	MsgQueueSubstreamHeader	substreamMsg;
	MsgQueueSubstreamDataHeader *pDataMsg;
	RsslRDMQueueDataExpired queueDataExpired;
	TunnelStreamImpl	*pTunnelImpl = pSubstreamImpl->_tunnelImpl;

	rsslClearDecodeIterator(&dIter);
	rsslSetDecodeIteratorRWFVersion(&dIter, pTunnelImpl->base.classOfService.common.protocolMajorVersion,
			pTunnelImpl->base.classOfService.common.protocolMinorVersion);
	rsslSetDecodeIteratorBuffer(&dIter, pTunnelBuffer);

	if ((ret = rsslDecodeMsg(&dIter, &rsslMsg)) != RSSL_RET_SUCCESS)
	{
		/* This shouldn't happen. */
		rsslSetErrorInfo(pErrorInfo, RSSL_EIC_FAILURE, ret, 
				__FILE__, __LINE__, "Failed to decode substream message while expiring it.");
		return RSSL_RET_FAILURE;
	}

	if ((ret = decodeSubstreamHeader(&dIter, &rsslMsg, &substreamMsg))
			!= RSSL_RET_SUCCESS)
	{
		rsslSetErrorInfo(pErrorInfo, RSSL_EIC_FAILURE, ret, 
				__FILE__, __LINE__, "Failed to decode queue message while expiring it.");
		return RSSL_RET_FAILURE;
	}


	pDataMsg = (MsgQueueSubstreamDataHeader*)&substreamMsg;

	rsslClearRDMQueueDataExpired(&queueDataExpired);
	queueDataExpired.rdmMsgBase.streamId = rsslMsg.msgBase.streamId;
	queueDataExpired.rdmMsgBase.domainType = rsslMsg.msgBase.domainType;

	queueDataExpired.identifier = pDataMsg->identifier;
	queueDataExpired.undeliverableCode = undeliverableCode;
	
	queueDataExpired.sourceName = pDataMsg->toQueue;
	queueDataExpired.destName = pDataMsg->fromQueue;
	queueDataExpired.encDataBody = pDataMsg->enclosedBuffer;
	queueDataExpired.containerType = pDataMsg->containerType;
	queueDataExpired.queueDepth = pSubstreamImpl->_lastQueueDepth;

	ret = tunnelSubstreamCallQueueCallback(pSubstreamImpl, NULL, (RsslRDMQueueMsg*)&queueDataExpired,
			RSSL_TRUE, NULL, NULL, 0, pErrorInfo);

	/* Returned from callback, free message from persistence. */
	if (pPersistentMsg != NULL)
	{
		RsslRet freeRet;
		if ((freeRet = persistenceFreeMsg(pSubstreamImpl->_pPersistFile, pPersistentMsg,
						pErrorInfo)) != RSSL_RET_SUCCESS)
			return freeRet;
	}

	return ret;
}

RsslRet tunnelSubstreamSubmit(TunnelSubstream *pSubstream,
		TunnelSubstreamSubmitOptions *pOptions,
		RsslErrorInfo *pErrorInfo)
{
	TunnelSubstreamImpl *pSubstreamImpl =  (TunnelSubstreamImpl*)pSubstream;
	RsslRDMQueueMsg *pQueueMsg = (RsslRDMQueueMsg*)pOptions->pRDMMsg;

	assert(pOptions->pRDMMsg != NULL);

	switch(pQueueMsg->rdmMsgBase.rdmMsgType)
	{
		case RDM_QMSG_MT_DATA:
		{
			MsgQueueSubstreamDataHeader dataMsg;
			RsslRDMQueueData *pInData =
				(RsslRDMQueueData*)pQueueMsg;
			RsslRet ret;
			TunnelBufferImpl *pBufferImpl;
			RsslEncodeIterator eIter;
			TunnelStreamImpl *pTunnelImpl = pSubstreamImpl->_tunnelImpl;
			RsslInt64 currentTime = tunnelStreamGetCurrentTimeMs(pTunnelImpl);
			RsslUInt32 length;

			if (pSubstream->_state != SBS_OPEN)
			{
				rsslSetErrorInfo(pErrorInfo, RSSL_EIC_FAILURE, RSSL_RET_INVALID_DATA, 
						__FILE__, __LINE__, "Queue stream is not established. May be waiting to receive a refresh or status with an Open streamState and Ok dataState.");
				return RSSL_RET_INVALID_DATA;
			}

			msgQueueClearSubstreamDataHeader(&dataMsg);
			dataMsg.base.streamId = pInData->rdmMsgBase.streamId;
			dataMsg.base.domainType = pInData->rdmMsgBase.domainType;
			dataMsg.fromQueue = pInData->sourceName;
			dataMsg.toQueue = pInData->destName;
			dataMsg.seqNum = 0; /* This will be replaced later. */
			dataMsg.timeout = pInData->timeout;
			dataMsg.enclosedBuffer = pInData->encDataBody;
			dataMsg.containerType = pInData->containerType;
			dataMsg.identifier = pInData->identifier;

			length = msgQueueSubstreamDataHeaderBufferSize(&dataMsg);
			if (length > pSubstreamImpl->_tunnelImpl->base.classOfService.common.maxFragmentSize)
				length = (RsslUInt32)pSubstreamImpl->_tunnelImpl->base.classOfService.common.maxFragmentSize;

			/* Since we translate, get a separate buffer. Note that this buffer
			 * is released if something fails -- the application must release it's own buffer, if any. */
			if ((pBufferImpl = (TunnelBufferImpl*)tunnelStreamGetBuffer(pSubstreamImpl->_tunnelImpl,
							length, RSSL_TRUE, RSSL_TRUE, pErrorInfo)) == NULL)
				return RSSL_RET_FAILURE;

			rsslClearEncodeIterator(&eIter);
			rsslSetEncodeIteratorRWFVersion(&eIter, pTunnelImpl->base.classOfService.common.protocolMajorVersion,
					pTunnelImpl->base.classOfService.common.protocolMinorVersion);
			rsslSetEncodeIteratorBuffer(&eIter, &pBufferImpl->_poolBuffer.buffer);

			/* Encode data message. */
			if ((ret = rsslEncodeMsgQueueSubstreamDataHeader(&eIter,
							&dataMsg)) != RSSL_RET_SUCCESS)
			{
				rsslTunnelStreamReleaseBuffer((RsslBuffer*)pBufferImpl, pErrorInfo);
				rsslSetErrorInfo(pErrorInfo, RSSL_EIC_FAILURE, ret, 
						__FILE__, __LINE__, "Failed to encode substream data message.");
				return RSSL_RET_FAILURE;
			}

			pBufferImpl->_poolBuffer.buffer.length = rsslGetEncodedBufferLength(&eIter);

			/* Try to persist to file. */
			if (pSubstreamImpl->_pPersistFile != NULL)
			{
				PersistentMsg *pPersistentMsg;
				if ((pPersistentMsg = persistFileSaveMsg(pSubstreamImpl->_pPersistFile, &pBufferImpl->_poolBuffer.buffer,
								dataMsg.timeout,
								currentTime, pErrorInfo)) == NULL)
				{
					rsslTunnelStreamReleaseBuffer((RsslBuffer*)pBufferImpl, pErrorInfo);
					return pErrorInfo->rsslError.rsslErrorId;
				}

				tunnelBufferImplSetPersistence(pBufferImpl, (TunnelSubstream*)pSubstreamImpl, 
						pPersistentMsg);
			}
			else
				tunnelBufferImplSetPersistence(pBufferImpl, (TunnelSubstream*)pSubstreamImpl, NULL);

			/* Add request to tunnel queue. */
			if ((ret = _tunnelSubstreamEnqueueDataBuffer(pSubstreamImpl, pBufferImpl, pErrorInfo)) != RSSL_RET_SUCCESS)
			{
				pSubstreamImpl->base._state = SBS_CLOSED;
				rsslTunnelStreamReleaseBuffer((RsslBuffer*)pBufferImpl, pErrorInfo);
				return RSSL_RET_FAILURE;
			}

			if (pInData->timeout == RDM_QMSG_TC_IMMEDIATE)
				tunnelStreamAddImmediateTimeoutBuffer(pSubstreamImpl->_tunnelImpl, pBufferImpl);
			else if (pInData->timeout > RDM_QMSG_TC_IMMEDIATE)
				tunnelStreamAddTimeoutBuffer(pSubstreamImpl->_tunnelImpl, pBufferImpl,
						tunnelStreamGetCurrentTimeMs(pSubstreamImpl->_tunnelImpl) + pInData->timeout);
			else
				pBufferImpl->_expireTime = pInData->timeout;
			
			break;
		}
		default:
			rsslSetErrorInfo(pErrorInfo, RSSL_EIC_FAILURE, RSSL_RET_INVALID_ARGUMENT, 
					__FILE__, __LINE__, "Received unhandled message class in substream.");
			return RSSL_RET_INVALID_ARGUMENT;
	}

	return RSSL_RET_SUCCESS;
}

static RsslRet _tunnelSubstreamEnqueueDataBuffer(TunnelSubstreamImpl *pSubstreamImpl, TunnelBufferImpl *pBufferImpl, RsslErrorInfo *pErrorInfo)
{
	RsslEncodeIterator	eIter;
	RsslBuffer			tmpBuffer;
	RsslRet				ret;
	TunnelStreamImpl	*pTunnelImpl = pSubstreamImpl->_tunnelImpl;

	/* Update substream sequence number. */
	rsslClearEncodeIterator(&eIter);
	rsslSetEncodeIteratorRWFVersion(&eIter, pTunnelImpl->base.classOfService.common.protocolMajorVersion,
			pTunnelImpl->base.classOfService.common.protocolMinorVersion);
	rsslClearBuffer(&tmpBuffer);
	tmpBuffer.length = pBufferImpl->_poolBuffer.buffer.length;
	tmpBuffer.data = pBufferImpl->_dataStartPos;
	rsslSetEncodeIteratorBuffer(&eIter, &tmpBuffer);

	if ((ret = tunnelStreamEnqueueBuffer(&pSubstreamImpl->_tunnelImpl->base,
					(RsslBuffer*)pBufferImpl, RSSL_DT_MSG, pErrorInfo)) != RSSL_RET_SUCCESS)
	{
		pSubstreamImpl->base._state = SBS_CLOSED;
		return RSSL_RET_FAILURE;
	}

	return RSSL_RET_SUCCESS;
}

static TunnelBufferImpl* _tunnelSubstreamLoadSavedMsgToTunnelBuffer(TunnelSubstreamImpl *pSubstreamImpl, PersistentMsg *pMsg, RsslErrorInfo *pErrorInfo)
{
	TunnelBufferImpl *pBufferImpl;

	if ((pBufferImpl = (TunnelBufferImpl*)tunnelStreamGetBuffer(pSubstreamImpl->_tunnelImpl,
					persistentMsgGetLength(pMsg), RSSL_FALSE, RSSL_FALSE, pErrorInfo)) == NULL)
		return NULL;

	if (persistFileReadSavedMsg(pSubstreamImpl->_pPersistFile, (RsslBuffer*)pBufferImpl, pMsg, pErrorInfo)
			!= RSSL_RET_SUCCESS)
	{
		rsslTunnelStreamReleaseBuffer((RsslBuffer*)pBufferImpl, pErrorInfo);
		return NULL;
	}

	return pBufferImpl;
}

static RsslRet _tunnelSubstreamLoadSavedMsgToTempMemory(TunnelSubstreamImpl *pSubstreamImpl, PersistentMsg *pMsg, RsslBuffer *oRsslBuffer, RsslErrorInfo *pErrorInfo)
{
	TunnelStreamImpl *pTunnelImpl = pSubstreamImpl->_tunnelImpl;

	/* Make sure memory buffer is large enough to hold the buffer. */
	if (rsslHeapBufferResize(&pTunnelImpl->_memoryBuffer,
				pMsg->_msgLength, RSSL_FALSE)
			!= RSSL_RET_SUCCESS)
	{
		rsslSetErrorInfo(pErrorInfo, RSSL_EIC_FAILURE, RSSL_RET_FAILURE, __FILE__, __LINE__, 
				"Memory allocation failed.");
		return RSSL_RET_FAILURE;
	}

	*oRsslBuffer = pTunnelImpl->_memoryBuffer;
	if (persistFileReadSavedMsg(pSubstreamImpl->_pPersistFile, oRsslBuffer, pMsg, pErrorInfo)
			!= RSSL_RET_SUCCESS)
		return RSSL_RET_FAILURE;

	return RSSL_RET_SUCCESS;
}

static RsslRet _tunnelSubstreamSendRecoveredMessage(TunnelSubstreamImpl *pSubstreamImpl, 
		PersistentMsg *pMsg, RsslBool isPossibleDuplicate, RsslErrorInfo *pErrorInfo)
{
	RsslEncodeIterator	eIter;
	RsslDecodeIterator	dIter;
	RsslRet				ret;
	TunnelBufferImpl	*pBufferImpl;
	TunnelStreamImpl	*pTunnelImpl = pSubstreamImpl->_tunnelImpl;

	assert(pSubstreamImpl->_pPersistFile);
	assert(pSubstreamImpl->base._state == SBS_OPEN);


	if (pMsg->_msgLength > pTunnelImpl->base.classOfService.common.maxFragmentSize)
	{
		/* Message is too large to send. */
		RsslBuffer tmpBuffer;

		if ((ret = _tunnelSubstreamLoadSavedMsgToTempMemory(pSubstreamImpl, pMsg, 
						&tmpBuffer, pErrorInfo)) != RSSL_RET_SUCCESS)
			return ret;

		if ((ret = tunnelSubstreamExpireBuffer((TunnelSubstream*)pSubstreamImpl, &tmpBuffer, 
						pMsg, RDM_QMSG_UC_MAX_MSG_SIZE, pErrorInfo))
				!= RSSL_RET_SUCCESS)
			return ret;

		return RSSL_RET_SUCCESS;
	}
	else if (pMsg->_timeout == RDM_QMSG_TC_IMMEDIATE)
	{
		/* Since we just loaded the file, expire anything with an immediate code. */
		RsslBuffer tmpBuffer;

		if ((ret = _tunnelSubstreamLoadSavedMsgToTempMemory(pSubstreamImpl, pMsg, 
						&tmpBuffer, pErrorInfo)) != RSSL_RET_SUCCESS)
			return ret;

		if ((ret = tunnelSubstreamExpireBuffer((TunnelSubstream*)pSubstreamImpl, &tmpBuffer, 
						pMsg, RDM_QMSG_UC_EXPIRED, pErrorInfo))
				!= RSSL_RET_SUCCESS)
			return ret;

		return RSSL_RET_SUCCESS;
	}
	else if (pMsg->_timeout > RDM_QMSG_TC_IMMEDIATE)
	{
		RsslInt64 currentTime = tunnelStreamGetCurrentTimeMs(pSubstreamImpl->_tunnelImpl); 
		RsslInt64 expireTime = pMsg->_timeQueued + pMsg->_timeout;
		RsslBuffer tmpBuffer;

		if (currentTime - expireTime > 0)
		{
			/* Timeout already passed, expire the message. */
			if ((ret = _tunnelSubstreamLoadSavedMsgToTempMemory(pSubstreamImpl, pMsg, 
							&tmpBuffer, pErrorInfo)) != RSSL_RET_SUCCESS)
				return ret;

			if ((ret = tunnelSubstreamExpireBuffer((TunnelSubstream*)pSubstreamImpl, &tmpBuffer, 
							pMsg, RDM_QMSG_UC_EXPIRED, pErrorInfo)) != RSSL_RET_SUCCESS)
				return ret;
			return RSSL_RET_SUCCESS;
		}

	}

	if ((pBufferImpl = _tunnelSubstreamLoadSavedMsgToTunnelBuffer(pSubstreamImpl, pMsg, pErrorInfo)) == NULL)
		return RSSL_RET_FAILURE;

	if (pMsg->_timeout > RDM_QMSG_TC_IMMEDIATE)
	{
		pBufferImpl->_expireTime = pMsg->_timeQueued + pMsg->_timeout;
		tunnelStreamAddTimeoutBuffer(pSubstreamImpl->_tunnelImpl, pBufferImpl, pBufferImpl->_expireTime);
	}
	else
		pBufferImpl->_expireTime = pMsg->_timeout;


	/* Application can open the queue with a different stream ID. Make sure it's up to date. */
	rsslClearEncodeIterator(&eIter);
	rsslSetEncodeIteratorRWFVersion(&eIter, pTunnelImpl->base.classOfService.common.protocolMajorVersion, 
		pTunnelImpl->base.classOfService.common.protocolMinorVersion);

	rsslSetEncodeIteratorBuffer(&eIter, (RsslBuffer*)pBufferImpl);
	if ((ret = rsslReplaceStreamId(&eIter, pSubstreamImpl->base._streamId)) != RSSL_RET_SUCCESS)
	{
		rsslSetErrorInfo(pErrorInfo, RSSL_EIC_FAILURE, ret,
				__FILE__, __LINE__, "Failed to set stream ID on queue message.");
		pSubstreamImpl->base._state = SBS_CLOSED;
		rsslTunnelStreamReleaseBuffer((RsslBuffer*)pBufferImpl, pErrorInfo);
		return RSSL_RET_FAILURE;
	}

	tunnelBufferImplSetPersistence(pBufferImpl, (TunnelSubstream*)pSubstreamImpl, pMsg);

	/* Add Possible-Duplicate flag. */
	if (isPossibleDuplicate)
	{
		rsslClearDecodeIterator(&dIter);
		rsslSetDecodeIteratorRWFVersion(&dIter, pTunnelImpl->base.classOfService.common.protocolMajorVersion, 
				pTunnelImpl->base.classOfService.common.protocolMinorVersion);

		rsslSetDecodeIteratorBuffer(&dIter, (RsslBuffer*)pBufferImpl);

		if ((ret = rsslAddQueueDataDuplicateFlag(&dIter)) != RSSL_RET_SUCCESS)
		{
			rsslSetErrorInfo(pErrorInfo, RSSL_EIC_FAILURE, ret,
					__FILE__, __LINE__, "Failed to set possible-duplicate flag on queue data message.");
			return RSSL_RET_FAILURE;
		}
	}


	if ((ret = tunnelStreamEnqueueBuffer(&pSubstreamImpl->_tunnelImpl->base,
					(RsslBuffer*)pBufferImpl, RSSL_DT_MSG, pErrorInfo)) != RSSL_RET_SUCCESS)
	{
		pSubstreamImpl->base._state = SBS_CLOSED;
		rsslTunnelStreamReleaseBuffer((RsslBuffer*)pBufferImpl, pErrorInfo);
		return RSSL_RET_FAILURE;
	}

	return RSSL_RET_SUCCESS;
}


RTR_C_INLINE RsslRet tunnelSubstreamCallQueueCallback(TunnelSubstreamImpl *pSubstreamImpl,
		RsslMsg *pRsslMsg, RsslRDMQueueMsg *pQueueMsg, RsslBool isLocallyGenerated, 
		RsslBuffer *pAckSourceName, RsslBuffer *pAckDestName, RsslUInt32 ackSeqNum,
		RsslErrorInfo *pErrorInfo)
{
	RsslTunnelStreamQueueMsgEvent	queueMsgEvent;
	TunnelStreamImpl		*pTunnelImpl = (TunnelStreamImpl*)pSubstreamImpl->_tunnelImpl;

	/* No RsslMsg should be provided on locally-generated QueueMsgs -- if one is needed we will create it. */
	assert(isLocallyGenerated && pRsslMsg == NULL && (pQueueMsg->rdmMsgBase.rdmMsgType == RDM_QMSG_MT_DATA_EXPIRED || pQueueMsg->rdmMsgBase.rdmMsgType == RDM_QMSG_MT_ACK) 
			|| !isLocallyGenerated && pRsslMsg != NULL);

	tunnelStreamQueueMsgEventClear(&queueMsgEvent);
	queueMsgEvent.base.pReactorChannel = pTunnelImpl->_manager->base._pReactorChannel;
	queueMsgEvent.pQueueMsg = pQueueMsg;

	if (pTunnelImpl->_queueMsgCallback)
	{
		RsslReactorCallbackRet cret;
		cret = (*pTunnelImpl->_queueMsgCallback)(&pTunnelImpl->base, &queueMsgEvent);

		switch(cret)
		{
			case RSSL_RC_CRET_SUCCESS:
				return RSSL_RET_SUCCESS;
			case RSSL_RC_CRET_RAISE:
			{

				if (!isLocallyGenerated)
					return tunnelStreamCallMsgCallback(pTunnelImpl, NULL, pRsslMsg, RSSL_DT_MSG, pErrorInfo);
				else
				{
					char extHeaderMemory[255];
					RsslBuffer extHeaderBuf = { 255, extHeaderMemory };
					RsslGenericMsg genericMsg;
					RsslRet ret;

					/* Application raised a message we expired locally.
					 * Create an RsslMsg version of it. */
					switch(pQueueMsg->rdmMsgBase.rdmMsgType)
					{
						case RDM_QMSG_MT_ACK:
						{
							if ((ret = substreamSetRsslMsgFromQueueAckMsg(&pQueueMsg->ack, 
											&genericMsg, ackSeqNum, &extHeaderBuf)) != RSSL_RET_SUCCESS)
							{
								rsslSetErrorInfo(pErrorInfo, RSSL_EIC_FAILURE, ret, 
										__FILE__, __LINE__, "Failed to encode locally-generated QueueAck RsslMsg.");
								return RSSL_RET_FAILURE;
							}
							break;
						}
							
						default:
						{
							assert(pQueueMsg->rdmMsgBase.rdmMsgType == RDM_QMSG_MT_DATA_EXPIRED);

							if ((ret = substreamSetRsslMsgFromQueueDataExpiredMsg(&pQueueMsg->dataExpired, 
											&genericMsg, &extHeaderBuf)) != RSSL_RET_SUCCESS)
							{
								rsslSetErrorInfo(pErrorInfo, RSSL_EIC_FAILURE, ret, 
										__FILE__, __LINE__, "Failed to encode locally-generated QueueDataExpired RsslMsg.");
								return RSSL_RET_FAILURE;
							}
							break;
						}
					}

					return tunnelStreamCallMsgCallback(pTunnelImpl, NULL, (RsslMsg*)&genericMsg, RSSL_DT_MSG, pErrorInfo);
				}
			}
			default:
				rsslSetErrorInfo(pErrorInfo, RSSL_EIC_FAILURE, RSSL_RET_FAILURE, 
						__FILE__, __LINE__, "Unexpected return code %d from tunnel stream queueMsgCallback.", cret);
				return RSSL_RET_FAILURE;
		}
	}
	else
		return tunnelStreamCallMsgCallback(pTunnelImpl, NULL, pRsslMsg, RSSL_DT_MSG, pErrorInfo);

}


RsslRet tunnelSubstreamRead(TunnelSubstream *pSubstream,
		RsslMsg *pMsg, RsslErrorInfo *pErrorInfo)
{
	RsslRet ret;
	MsgQueueSubstreamHeader	substreamMsg;
	RsslDecodeIterator		dIter;
	TunnelSubstreamImpl		*pSubstreamImpl =  (TunnelSubstreamImpl*)pSubstream;

	/* Save the tunnel in case the substream is destroyed before the callback. */
	TunnelStreamImpl		*pTunnelImpl = (TunnelStreamImpl*)pSubstreamImpl->_tunnelImpl;

	switch(pMsg->msgBase.msgClass)
	{
		case RSSL_MC_REFRESH:
		{
			RsslRefreshMsg *pRefresh = (RsslRefreshMsg*)pMsg;
			RsslRDMQueueRefresh queueRefresh;
			MsgQueueSubstreamRefreshHeader *pSubRefresh;

			rsslClearRDMQueueRefresh(&queueRefresh);

			queueRefresh.rdmMsgBase.streamId = pMsg->msgBase.streamId;
			queueRefresh.rdmMsgBase.domainType = pMsg->msgBase.domainType;

			if (pRefresh->flags & RSSL_RFMF_HAS_MSG_KEY
					&& pRefresh->msgBase.msgKey.flags & RSSL_MKF_HAS_NAME)
				queueRefresh.sourceName = pRefresh->msgBase.msgKey.name;
			queueRefresh.state = pRefresh->state;

			if ((ret = decodeSubstreamHeader(&dIter, pMsg, &substreamMsg))
					!= RSSL_RET_SUCCESS)
			{
				rsslSetErrorInfo(pErrorInfo, RSSL_EIC_FAILURE, ret, 
						__FILE__, __LINE__, "Failed to decode queue message.");
				return RSSL_RET_FAILURE;
			}

			pSubRefresh = (MsgQueueSubstreamRefreshHeader*)&substreamMsg;

			pSubstreamImpl->_lastQueueDepth = queueRefresh.queueDepth = pSubRefresh->queueDepth;
			pSubstreamImpl->_lastInSeqNum = pSubRefresh->lastOutSeqNum;

			if (pRefresh->state.streamState == RSSL_STREAM_OPEN)
			{
				if (pRefresh->state.dataState == RSSL_DATA_OK && pSubstreamImpl->base._state != SBS_OPEN)
				{
					pSubstreamImpl->base._state = SBS_OPEN;
					if ((ret = tunnelSubstreamCallQueueCallback(pSubstreamImpl, pMsg, (RsslRDMQueueMsg*)&queueRefresh, RSSL_FALSE, NULL, NULL, 0, pErrorInfo)) != RSSL_RET_SUCCESS)
						return ret;

					if (pSubstreamImpl->_pPersistFile)
					{
						RsslQueue *pSavedMsgQueue;
						RsslQueueLink *pLink;

						pSavedMsgQueue = persistFileGetSavedList(pSubstreamImpl->_pPersistFile);

						if (pSubRefresh->lastInSeqNum != 0)
						{
							for(pLink = rsslQueueStart(pSavedMsgQueue); pLink != NULL;
									pLink = rsslQueueForth(pSavedMsgQueue))
							{
								/* Generate acknowledgements for anything that wasn't acknowledged during the last session,
								 * but is indicating by the refresh as being received. */
								PersistentMsg *pMsg = RSSL_QUEUE_LINK_TO_OBJECT(PersistentMsg, _qLink, pLink);
								RsslRDMQueueAck queueAck;
								RsslMsg rsslMsg;
								MsgQueueSubstreamHeader substreamMsg;
								TunnelBufferImpl *pBufferImpl;

								if (rsslSeqNumCompare(pMsg->_seqNum, pSubRefresh->lastInSeqNum) > 0)
									break;

								if ((pBufferImpl = _tunnelSubstreamLoadSavedMsgToTunnelBuffer(pSubstreamImpl, pMsg, pErrorInfo)) == NULL)
									return RSSL_RET_FAILURE;

								rsslClearDecodeIterator(&dIter);
								rsslSetDecodeIteratorRWFVersion(&dIter, pTunnelImpl->base.classOfService.common.protocolMajorVersion,
										pTunnelImpl->base.classOfService.common.protocolMinorVersion);
								rsslSetDecodeIteratorBuffer(&dIter, (RsslBuffer*)pBufferImpl);

								if ((ret = rsslDecodeMsg(&dIter, &rsslMsg)) != RSSL_RET_SUCCESS)
								{
									rsslSetErrorInfo(pErrorInfo, RSSL_EIC_FAILURE, ret, 
											__FILE__, __LINE__, "Failed to decode message while generating local acknowledgement.");
									rsslTunnelStreamReleaseBuffer((RsslBuffer*)pBufferImpl, pErrorInfo);
									return RSSL_RET_FAILURE;
								}

								if ((ret = decodeSubstreamHeader(&dIter, &rsslMsg, &substreamMsg))
										!= RSSL_RET_SUCCESS)
								{
									rsslTunnelStreamReleaseBuffer((RsslBuffer*)pBufferImpl, pErrorInfo);
									rsslSetErrorInfo(pErrorInfo, RSSL_EIC_FAILURE, ret, 
											__FILE__, __LINE__, "Failed to decode queue message while generating local acknowledgement.");
									return RSSL_RET_FAILURE;
								}

								if (substreamMsg.base.opcode != MSGQUEUE_SHO_DATA)
								{
									rsslTunnelStreamReleaseBuffer((RsslBuffer*)pBufferImpl, pErrorInfo);
									rsslSetErrorInfo(pErrorInfo, RSSL_EIC_FAILURE, ret, 
											__FILE__, __LINE__, "Found non-QueueData message while generating local acknowledgement.");
									return RSSL_RET_FAILURE;
								}

								rsslClearRDMQueueAck(&queueAck);
								queueAck.rdmMsgBase.streamId = pSubstreamImpl->base._streamId;
								queueAck.identifier = substreamMsg.dataHeader.identifier;

								/* QueueAck flips source and destination names from data message. */
								queueAck.sourceName = substreamMsg.dataHeader.toQueue;
								queueAck.destName = substreamMsg.dataHeader.fromQueue;

								if ((ret = tunnelSubstreamCallQueueCallback(pSubstreamImpl, NULL, (RsslRDMQueueMsg*)&queueAck, RSSL_TRUE, 
									&substreamMsg.ackHeader.fromQueue, &substreamMsg.ackHeader.toQueue, substreamMsg.ackHeader.seqNum, pErrorInfo)) != RSSL_RET_SUCCESS)
								{
									rsslTunnelStreamReleaseBuffer((RsslBuffer*)pBufferImpl, pErrorInfo);
									return ret;
								}

								rsslTunnelStreamReleaseBuffer((RsslBuffer*)pBufferImpl, pErrorInfo);

								if ((ret = persistFileFreeMsgs(pSubstreamImpl->_pPersistFile, pMsg->_seqNum, pErrorInfo))
										!= RSSL_RET_SUCCESS)
									return ret;
							}
						}

						/* If zero is received, it may be because the queue stream is new,
						 * or because the provider has lost its existing knowledge of its state.
						 * In the latter case, continue normally, but don't free any currently-persisted messages (we'll resend them). */

						for(pLink = rsslQueueStart(pSavedMsgQueue); pLink != NULL;
								pLink = rsslQueueForth(pSavedMsgQueue))
						{
							PersistentMsg *pMsg = RSSL_QUEUE_LINK_TO_OBJECT(PersistentMsg, _qLink, pLink);
							if ((ret = _tunnelSubstreamSendRecoveredMessage(pSubstreamImpl, pMsg, (pSubRefresh->lastInSeqNum == 0), pErrorInfo))
									!= RSSL_RET_SUCCESS)
								return ret;
						}

						if ((ret = persistFileSaveLastInSeqNum(
										pSubstreamImpl->_pPersistFile, pSubRefresh->lastOutSeqNum, pErrorInfo)) 
								!= RSSL_RET_SUCCESS)
							return ret;

					}
					else /* Just take what the refresh says. */
						pSubstreamImpl->_lastOutSeqNum = pSubRefresh->lastInSeqNum;

				}
			}
			else
			{
				pSubstreamImpl->base._state = SBS_CLOSED;
				if ((ret = tunnelSubstreamCallQueueCallback(pSubstreamImpl, pMsg, (RsslRDMQueueMsg*)&queueRefresh, RSSL_FALSE, NULL, NULL, 0, pErrorInfo)) != RSSL_RET_SUCCESS)
					return ret;
			}


			break;

		}

		case RSSL_MC_STATUS:
		{
			RsslStatusMsg *pStatus = (RsslStatusMsg*)pMsg;
			RsslRDMQueueStatus queueStatus;

			rsslClearRDMQueueStatus(&queueStatus);

			queueStatus.rdmMsgBase.streamId = pMsg->msgBase.streamId;
			queueStatus.rdmMsgBase.domainType = pMsg->msgBase.domainType;

			if (pStatus->flags & RSSL_STMF_HAS_STATE)
			{
				queueStatus.flags |= RDM_QMSG_STF_HAS_STATE;
				queueStatus.state = pStatus->state;
			}

			queueStatus.state = pStatus->state;

			if (pStatus->flags & RSSL_STMF_HAS_STATE
					&& pStatus->state.streamState != RSSL_STREAM_OPEN)
				pSubstreamImpl->base._state = SBS_CLOSED;

			if ((ret = tunnelSubstreamCallQueueCallback(pSubstreamImpl, pMsg, (RsslRDMQueueMsg*)&queueStatus, RSSL_FALSE, NULL, NULL, 0, pErrorInfo)) != RSSL_RET_SUCCESS)
				return ret;

			break;
		}

		case RSSL_MC_GENERIC:
		{
			if ((ret = decodeSubstreamHeader(&dIter, pMsg, &substreamMsg))
					!= RSSL_RET_SUCCESS)
			{
				rsslSetErrorInfo(pErrorInfo, RSSL_EIC_FAILURE, ret, 
						__FILE__, __LINE__, "Failed to decode queue message.");
				return RSSL_RET_FAILURE;
			}


			switch(substreamMsg.base.opcode)
			{
				case MSGQUEUE_SHO_ACK:
				{
					
					RsslRDMQueueAck queueAck;
					MsgQueueSubstreamAckHeader *pAckMsg = 
						(MsgQueueSubstreamAckHeader*)&substreamMsg;

					rsslClearRDMQueueAck(&queueAck);
					queueAck.rdmMsgBase.streamId = pMsg->msgBase.streamId;
					queueAck.rdmMsgBase.domainType = pMsg->msgBase.domainType;

					queueAck.identifier = pAckMsg->identifier;
					queueAck.sourceName = substreamMsg.dataHeader.fromQueue;
					queueAck.destName = substreamMsg.dataHeader.toQueue;

					if (pSubstreamImpl->_pPersistFile != NULL)
					{
						if ((ret = persistFileFreeMsgs(pSubstreamImpl->_pPersistFile, substreamMsg.ackHeader.seqNum, pErrorInfo)
								!= RSSL_RET_SUCCESS))
							return ret;
					}

					if ((ret = tunnelSubstreamCallQueueCallback(pSubstreamImpl, pMsg, (RsslRDMQueueMsg*)&queueAck, RSSL_FALSE, NULL, NULL, 0, pErrorInfo)) != RSSL_RET_SUCCESS)
						return ret;

					break;
				}

				case MSGQUEUE_SHO_DATA:
				case MSGQUEUE_SHO_DEAD_LETTER:
				{
					
					MsgQueueSubstreamAckHeader ackMsg;
					MsgQueueSubstreamDataHeader *pDataMsg = 
						(MsgQueueSubstreamDataHeader*)&substreamMsg;
					RsslEncodeIterator eIter;
					RsslBuffer *pBuffer;
					RsslUInt32 length;

					/* Do not process Queue data messages if tunnel stream is closing. We can't ack them. */
					if (pTunnelImpl->_state > TSS_OPEN)
						return RSSL_RET_SUCCESS;

					switch(substreamMsg.base.opcode)
					{
						case MSGQUEUE_SHO_DATA:
						{
							RsslRDMQueueData queueData;

							rsslClearRDMQueueData(&queueData);
							queueData.rdmMsgBase.streamId = pMsg->msgBase.streamId;
							queueData.rdmMsgBase.domainType = pMsg->msgBase.domainType;

							queueData.flags = pDataMsg->flags;
							queueData.identifier = pDataMsg->identifier;
							queueData.sourceName = pDataMsg->fromQueue;
							queueData.destName = pDataMsg->toQueue;
							queueData.encDataBody = pDataMsg->enclosedBuffer;
							queueData.containerType = pDataMsg->containerType;
							pSubstreamImpl->_lastQueueDepth = queueData.queueDepth = pDataMsg->queueDepth;
							queueData.timeout = pDataMsg->timeout;

							if ((ret = tunnelSubstreamCallQueueCallback(pSubstreamImpl, pMsg, (RsslRDMQueueMsg*)&queueData, RSSL_FALSE, NULL, NULL, 0, pErrorInfo)) != RSSL_RET_SUCCESS)
								return ret;

							break;
						}

						default: /* Expired */
						{
							RsslRDMQueueDataExpired queueDataExpired;

							rsslClearRDMQueueDataExpired(&queueDataExpired);
							queueDataExpired.rdmMsgBase.streamId = pMsg->msgBase.streamId;
							queueDataExpired.rdmMsgBase.domainType = pMsg->msgBase.domainType;

							queueDataExpired.flags = pDataMsg->flags;
							queueDataExpired.identifier = pDataMsg->identifier;
							queueDataExpired.undeliverableCode = pDataMsg->undeliverableCode;
							queueDataExpired.sourceName = pDataMsg->fromQueue;
							queueDataExpired.destName = pDataMsg->toQueue;
							queueDataExpired.encDataBody = pDataMsg->enclosedBuffer;
							queueDataExpired.containerType = pDataMsg->containerType;
							pSubstreamImpl->_lastQueueDepth = queueDataExpired.queueDepth = pDataMsg->queueDepth;

							if ((ret = tunnelSubstreamCallQueueCallback(pSubstreamImpl, pMsg, (RsslRDMQueueMsg*)&queueDataExpired, RSSL_FALSE, NULL, NULL, 0, pErrorInfo)) != RSSL_RET_SUCCESS)
								return ret;
							break;
						}
						
					}

					/* Send an acknowledgement for this message. */
					msgQueueClearSubstreamAckHeader(&ackMsg);
					ackMsg.base.streamId = pDataMsg->base.streamId;
					ackMsg.base.domainType = pDataMsg->base.domainType;
					ackMsg.identifier = pDataMsg->identifier;

					ackMsg.toQueue = pDataMsg->fromQueue;
					ackMsg.fromQueue = pDataMsg->toQueue;
					ackMsg.seqNum = pDataMsg->seqNum;

					length = msgQueueSubstreamAckHeaderBufferSize(&ackMsg);
					if (length > pSubstreamImpl->_tunnelImpl->base.classOfService.common.maxFragmentSize)
						length = (RsslUInt32)pSubstreamImpl->_tunnelImpl->base.classOfService.common.maxFragmentSize;

					if ((pBuffer = (RsslBuffer*)tunnelStreamGetBuffer(pSubstreamImpl->_tunnelImpl,
									length, RSSL_FALSE, RSSL_FALSE, pErrorInfo)) == NULL)
						return RSSL_RET_FAILURE;

					rsslClearEncodeIterator(&eIter);
					rsslSetEncodeIteratorRWFVersion(&eIter, pTunnelImpl->base.classOfService.common.protocolMajorVersion, pTunnelImpl->base.classOfService.common.protocolMinorVersion);
					rsslSetEncodeIteratorBuffer(&eIter, pBuffer);

					/* Encode ack */
					if ((ret = rsslEncodeMsgQueueSubstreamAckHeader(&eIter,
									&ackMsg)) != RSSL_RET_SUCCESS)
					{
						rsslSetErrorInfo(pErrorInfo, RSSL_EIC_FAILURE, ret, 
								__FILE__, __LINE__, "Failed to encode substream request.");
						rsslTunnelStreamReleaseBuffer(pBuffer, pErrorInfo);
						return RSSL_RET_FAILURE;
					}

					pBuffer->length = rsslGetEncodedBufferLength(&eIter);


					/* Add ack message to tunnel queue. */
					if ((ret = tunnelStreamEnqueueBuffer(&pSubstreamImpl->_tunnelImpl->base, pBuffer, RSSL_DT_MSG, pErrorInfo)) != RSSL_RET_SUCCESS)
					{
						rsslTunnelStreamReleaseBuffer(pBuffer, pErrorInfo);
						return ret;
					}

					if (pSubstreamImpl->_pPersistFile != NULL)
					{
						/* Record that we got this message. */
						if ((ret = persistFileSaveLastInSeqNum(
										pSubstreamImpl->_pPersistFile, pDataMsg->seqNum, pErrorInfo)) 
								!= RSSL_RET_SUCCESS)
							return ret;
					}

					break;
				}

				default:
					rsslSetErrorInfo(pErrorInfo, RSSL_EIC_FAILURE, RSSL_RET_FAILURE, 
							__FILE__, __LINE__, "Unknown queue message class.");
					return RSSL_RET_FAILURE;

			}

			break;
		}

		default:
			rsslSetErrorInfo(pErrorInfo, RSSL_EIC_FAILURE, RSSL_RET_FAILURE, 
					__FILE__, __LINE__, "Received unhandled substream message class.");
			return RSSL_RET_FAILURE;

	}

	return RSSL_RET_SUCCESS;
}

RsslRet	tunnelSubstreamUpdateMsgForTransmit(TunnelSubstream *pSubstream, RsslBuffer *pBuffer, RsslErrorInfo *pErrorInfo)
{
	TunnelSubstreamImpl *pSubstreamImpl =  (TunnelSubstreamImpl*)pSubstream;
	TunnelBufferImpl *pBufferImpl = (TunnelBufferImpl*)pBuffer;
	RsslRet ret;
	RsslEncodeIterator eIter;
	RsslUInt32 seqNum;
	TunnelStreamImpl		*pTunnelImpl = (TunnelStreamImpl*)pSubstreamImpl->_tunnelImpl;

	if (pBufferImpl->_persistentMsg != NULL)
	{
		if ((ret = persistentMsgUpdateForTransmit(pSubstreamImpl->_pPersistFile, pBufferImpl->_persistentMsg,
			pBuffer, &pSubstreamImpl->_lastOutSeqNum, pErrorInfo)) != RSSL_RET_SUCCESS)
		return ret;

		seqNum = pBufferImpl->_persistentMsg->_seqNum;
	}
	else
	{
		++pSubstreamImpl->_lastOutSeqNum;
		seqNum = pSubstreamImpl->_lastOutSeqNum;
	}

	/* Update sequence number in message buffer. */
	rsslClearEncodeIterator(&eIter);
	rsslSetEncodeIteratorRWFVersion(&eIter, pTunnelImpl->base.classOfService.common.protocolMajorVersion,
			pTunnelImpl->base.classOfService.common.protocolMinorVersion);
	rsslSetEncodeIteratorBuffer(&eIter, pBuffer);
	if ((ret = rsslReplaceSeqNum(&eIter, seqNum)) != RSSL_RET_SUCCESS)
	{
		rsslSetErrorInfo(pErrorInfo, RSSL_EIC_FAILURE, ret,
				__FILE__, __LINE__, "Failed to set sequence number on queue message.");
		return RSSL_RET_FAILURE;
	}

	if (pBufferImpl->_expireTime > RDM_QMSG_TC_IMMEDIATE)
	{
		/* Adjust timeout to account for time passed. */
		RsslInt64 newTime = tunnelStreamGetCurrentTimeMs(pSubstreamImpl->_tunnelImpl);
		RsslDecodeIterator dIter;

		/* Make sure value is positive (just in case -- if it's not positive, this message generally 
		 * should've expired already, but time isn't perfect). */
		if ((pBufferImpl->_expireTime - newTime) > 1) newTime = pBufferImpl->_expireTime - newTime;
		else newTime = 1;

		rsslClearDecodeIterator(&dIter);
		rsslSetDecodeIteratorRWFVersion(&dIter, pTunnelImpl->base.classOfService.common.protocolMajorVersion,
				pTunnelImpl->base.classOfService.common.protocolMinorVersion);
		rsslSetDecodeIteratorBuffer(&dIter, pBuffer);
		if ((ret = rsslReplaceQueueDataTimeout(&dIter, newTime)) != RSSL_RET_SUCCESS)
		{
			rsslSetErrorInfo(pErrorInfo, RSSL_EIC_FAILURE, ret,
					__FILE__, __LINE__, "Failed to update timeout on queue message.");
			return RSSL_RET_FAILURE;
		}
	}

	return RSSL_RET_SUCCESS;
}

RsslRet tunnelSubstreamClose(TunnelSubstream *pSubstream,
		RsslErrorInfo *pErrorInfo)
{
	TunnelSubstreamImpl *pSubstreamImpl =  (TunnelSubstreamImpl*)pSubstream;

	RsslEncodeIterator				eIter;
	RsslBuffer*						pBuffer;
	RsslRet							ret;

	/* If close message was supplied, send it. */
	switch(pSubstream->_state)
	{
		case SBS_NOT_OPEN:
		case SBS_CLOSED:
			break;

		case SBS_WAIT_REFRESH:
		case SBS_OPEN:
		{
			RsslCloseMsg closeMsg;
			TunnelStreamImpl	*pTunnelImpl = pSubstreamImpl->_tunnelImpl;
			RsslUInt32 length;

			rsslClearCloseMsg(&closeMsg);
			closeMsg.msgBase.streamId = pSubstream->_streamId;
			closeMsg.msgBase.domainType = pSubstream->_domainType;
			closeMsg.msgBase.containerType = RSSL_DT_NO_DATA;

			length = 128;
			if (length > pSubstreamImpl->_tunnelImpl->base.classOfService.common.maxFragmentSize)
				length = (RsslUInt32)pSubstreamImpl->_tunnelImpl->base.classOfService.common.maxFragmentSize;

			/* Queue close in tunnel stream. */
			if ((pBuffer = (RsslBuffer*)tunnelStreamGetBuffer(pSubstreamImpl->_tunnelImpl,
							128, RSSL_FALSE, RSSL_FALSE, pErrorInfo)) == NULL)
				return RSSL_RET_FAILURE;

			rsslClearEncodeIterator(&eIter);
			rsslSetEncodeIteratorRWFVersion(&eIter, pTunnelImpl->base.classOfService.common.protocolMajorVersion,
					pTunnelImpl->base.classOfService.common.protocolMinorVersion);
			rsslSetEncodeIteratorBuffer(&eIter, pBuffer);

			/* Encode close */
			if ((ret = rsslEncodeMsg(&eIter,
							(RsslMsg*)&closeMsg)) != RSSL_RET_SUCCESS)
			{
				rsslTunnelStreamReleaseBuffer(pBuffer, pErrorInfo);
				rsslSetErrorInfo(pErrorInfo, RSSL_EIC_FAILURE, ret, 
						__FILE__, __LINE__, "Failed to encode substream close.");
				return RSSL_RET_FAILURE;
			}

			pBuffer->length = rsslGetEncodedBufferLength(&eIter);

			((TunnelBufferImpl*)pBuffer)->_flags = TBF_QUEUE_CLOSE;
			((TunnelBufferImpl*)pBuffer)->_substream = pSubstream;

			/* Add close to tunnel queue. */
			if ((ret = tunnelStreamEnqueueBuffer(&pSubstreamImpl->_tunnelImpl->base,
							pBuffer, RSSL_DT_MSG, pErrorInfo)) != RSSL_RET_SUCCESS)
			{
				rsslTunnelStreamReleaseBuffer(pBuffer, pErrorInfo);
				return RSSL_RET_FAILURE;
			}
		}
	}

	return RSSL_RET_SUCCESS;
}

void tunnelSubstreamDestroy(TunnelSubstream *pSubstream)
{
	TunnelSubstreamImpl *pSubstreamImpl = (TunnelSubstreamImpl*)pSubstream;

	if (pSubstreamImpl->_pPersistFile)
	{
		persistFileClose(pSubstreamImpl->_pPersistFile);
		pSubstreamImpl->_pPersistFile = NULL;
	}

	free(pSubstream);
}
