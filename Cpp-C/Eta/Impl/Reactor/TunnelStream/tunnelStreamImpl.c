/*
 * This source code is provided under the Apache 2.0 license and is provided
 * AS IS with no warranty or guarantee of fit for purpose.  See the project's 
 * LICENSE.md for details. 
 * Copyright (C) 2024 LSEG. All rights reserved.
*/

#include "rtr/tunnelStreamImpl.h"
#include "rtr/rsslRDMQueueMsgInt.h"
#include "rtr/msgQueueEncDec.h"
#include "rtr/rsslReactorImpl.h"
#include "rtr/rsslHeapBuffer.h"
#include "rtr/rsslReactorUtils.h"
#include "rtr/bigBufferPool.h"

#include <assert.h>

typedef enum
{
	TS_DBG_NONE = 0x0,
	TS_DBG_ACKS = 0x1	/* Prints info about buffers acked, and send window */
} TunnelStreamDebugFlags;

static const int tunnelStreamDebugFlags = TS_DBG_NONE;

/* Default size to use if bidirectional flow control is enabled */
static const RsslInt32 TS_DEFAULT_BIDRECTIONAL_WINDOW_SIZE = 6144 * 2;

/* Position of the containerType in an encoded RSSL message. */
static const RsslUInt32 TS_CONTAINER_TYPE_POS = 9;

/* Number used to validate whether a buffer is a TunnelStream buffer. */
static const RsslUInt32 TS_BUFFER_INTEGRITY = 0x2a030d20;

/* Timeout for retransmission of FIN/FIN-ACK */
static const RsslUInt32 TS_RETRANSMIT_TIMEOUT = 150;

static const RsslUInt32 TS_RETRANSMIT_MAX_ATTEMPTS = 4;

static RsslRet _tunnelStreamSubmitChannelMsg(TunnelStreamImpl *pTunnelImpl,
		RsslMsg *pRsslMsg, RsslErrorInfo *pErrorInfo);

/* Send a data message. */
static RsslRet _tunnelStreamSendMessages(TunnelStreamImpl *pTunnelImpl, RsslErrorInfo *pErrorInfo);

/* Call status event callback. */
static RsslRet _tunnelStreamCallStatusEventCallback(TunnelStreamImpl *pTunnelImpl, RsslState *pState, RsslMsg *pRsslMsg, RsslTunnelStreamAuthInfo *pAuthInfo, RsslErrorInfo *pErrorInfo);

/* Send an acknowledgement message. */
static RsslRet _tunnelStreamSendAck(TunnelStreamImpl *pTunnelImpl, TunnelStreamAck *pAckMsg,
		AckRangeList *nakRangeList, RsslErrorInfo *pErrorInfo);

/* Adds a buffer with an immediate timeout (used by substreams when sending queue messages). */
static void _tunnelStreamRemoveTimeoutBuffer(TunnelStreamImpl *pTunnelImpl, 
		TunnelBufferImpl *pBufferImpl);

/* Performs initialization TunnelStream based on negotiated class of service parameters,
 * such as creating the tunnel buffer pool. */
static RsslRet _tunnelStreamHandleEstablished(TunnelStreamImpl *pTunnelImpl, RsslErrorInfo *pErrorInfo);

/* Checks if we received unsupported or mismatched ClassOfService parameters. */
static RsslRet _tunnelStreamCheckReceivedCos(RsslClassOfService *pLocalCos, RsslClassOfService *pRemoteCos, RsslUInt streamVersion, RsslErrorInfo *pErrorInfo);

/* Returns a TunnelBufferImpl for use (no buffer space attached yet) */
static TunnelBufferImpl *_tunnelStreamGetBufferImplObject(TunnelStreamImpl *pTunnelImpl, RsslErrorInfo *pErrorInfo);

/* Handles a received fragmented message. */
static RsslRet _tunnelStreamHandleFragmentedMsg(TunnelStreamImpl *pTunnelImpl, TunnelStreamData *pDataMsg, RsslBuffer *pFragmentedData, RsslErrorInfo *pErrorInfo);

/* Gets message id for fragmentation. */
static RsslUInt16 _tunnelStreamFragMsgId(TunnelStreamImpl *pTunnelImpl);

/* Gets a buffer for fragmentation. */
static TunnelBufferImpl* _tunnelStreamGetBufferForFragmentation(TunnelStreamImpl *pTunnelImpl, RsslUInt32 length, RsslUInt32 totalMsgLen, RsslUInt32 fragmentNumber,
																RsslUInt16 msgId, RsslUInt8 containerType, RsslBool msgComplete, RsslErrorInfo *pErrorInfo);
/* Handle a tunnel stream request retry if possible. */
static RsslBool _tunnelStreamHandleRequestRetry(TunnelStreamImpl *pTunnelImpl);

static void _tunnelStreamSetResponseTimerWithBackoff(TunnelStreamImpl *pTunnelImpl)
{
	RsslUInt32 i;
	pTunnelImpl->_responseExpireTime = TS_RETRANSMIT_TIMEOUT;
	for (i = 0; i < pTunnelImpl->_retransRetryCount; ++i)
	{
		pTunnelImpl->_responseExpireTime *= 2;
	}
	++pTunnelImpl->_retransRetryCount;
	pTunnelImpl->_responseExpireTime += tunnelStreamGetCurrentTimeMs(pTunnelImpl);
	tunnelStreamSetNextExpireTime(pTunnelImpl, pTunnelImpl->_responseExpireTime);
}

static void _tunnelStreamSetResponseTimer(TunnelStreamImpl *pTunnelImpl)
{
	pTunnelImpl->_responseExpireTime = tunnelStreamGetCurrentTimeMs(pTunnelImpl) + (pTunnelImpl->_responseTimeout);
	tunnelStreamSetNextExpireTime(pTunnelImpl, pTunnelImpl->_responseExpireTime);
}

static void _tunnelStreamUnsetResponseTimer(TunnelStreamImpl *pTunnelImpl)
{
	pTunnelImpl->_responseExpireTime = RDM_QMSG_TC_INFINITE;
}

static void _tunnelStreamFreeAckedBuffer(TunnelStreamImpl *pTunnelImpl, TunnelBufferImpl *pBufferImpl)
{
	rsslQueueRemoveLink(&pTunnelImpl->_tunnelBufferWaitAckList, &pBufferImpl->_tbpLink);
	pTunnelImpl->_bytesWaitingAck -= pBufferImpl->_poolBuffer.buffer.length;
	if (tunnelStreamDebugFlags & TS_DBG_ACKS)
		printf("<TunnelStreamDebug streamId:%d> Inbound AckMsg freed buffer seqNum: %u, length: %u, bytes waiting ack: " RTR_LLD "\n", pTunnelImpl->base.streamId, pBufferImpl->_seqNum, pBufferImpl->_poolBuffer.buffer.length, pTunnelImpl->_bytesWaitingAck);
	tunnelStreamReleaseBuffer(pTunnelImpl, pBufferImpl);
}

/* Search the given list for a buffer with this sequence number. */
static TunnelBufferImpl *_tunnelStreamGetBufferWithSeqNum(RsslQueue *pQueue, RsslUInt32 seqNum)
{
	RsslQueueLink *pLink;

	for(pLink = rsslQueueStart(pQueue);
			pLink != NULL;
			pLink = rsslQueueForth(pQueue))
	{
		TunnelBufferImpl *pBufferImpl = RSSL_QUEUE_LINK_TO_OBJECT(TunnelBufferImpl, _tbpLink, pLink);
		if (pBufferImpl->_seqNum == seqNum)
			return pBufferImpl;
	}

	return NULL;
}

/* Returns if there is room in the send window to do send this message. */
static RsslBool _tunnelStreamCanSendMessage(TunnelStreamImpl *pTunnelImpl, TunnelBufferImpl *pBufferImpl)
{
	/* Flow control is not enabled, safe to send */
	if (pTunnelImpl->base.classOfService.flowControl.type == RDM_COS_FC_NONE)
		return RSSL_TRUE;

	/* Some messages (such as the auth request & FIN) go out regardless of the window. */
	if (pBufferImpl->_flags & TBF_IGNORE_FC)
		return RSSL_TRUE;

	/* Check if there is room for the content of the message 
	 * (not including the tunnel stream message header) */
	if (pBufferImpl->_poolBuffer.buffer.length - (pBufferImpl->_dataStartPos - pBufferImpl->_startPos) + pTunnelImpl->_bytesWaitingAck
			<= pTunnelImpl->base.classOfService.flowControl.sendWindowSize)
		return RSSL_TRUE;

	return RSSL_FALSE;
}

RsslTunnelStream* tunnelStreamOpen(TunnelManager *pManager, RsslTunnelStreamOpenOptions *pOpts,
		RsslBool isProvider, RsslClassOfService *pRemoteCos, RsslUInt streamVersion, RsslErrorInfo *pErrorInfo)
{
	TunnelStreamImpl *pTunnelImpl;
	TunnelManagerImpl *pManagerImpl = (TunnelManagerImpl*)pManager;
	RsslReactorChannelRoleType roleType = ((RsslReactorChannelImpl*)pManagerImpl->base._pReactorChannel)->channelRole.base.roleType;

	if (pOpts->statusEventCallback == NULL)
	{
		rsslSetErrorInfo(pErrorInfo, RSSL_EIC_FAILURE, RSSL_RET_INVALID_ARGUMENT, 
				__FILE__, __LINE__, "RsslTunnelStream statusEventCallback not specified.");
		return NULL;
	}

	if (pOpts->defaultMsgCallback == NULL)
	{
		rsslSetErrorInfo(pErrorInfo, RSSL_EIC_FAILURE, RSSL_RET_FAILURE, 
				__FILE__, __LINE__, "RsslTunnelStream defaultMsgCallback not specified.");
		return NULL;
	}

	if (rsslHashTableFind(&pManagerImpl->_streamIdToTunnelStreamTable, &pOpts->streamId, NULL)
		!= NULL)
	{
		rsslSetErrorInfo(pErrorInfo, RSSL_EIC_FAILURE, RSSL_RET_FAILURE, __FILE__, __LINE__, 
				"TunnelStream is already open for stream id %d", pOpts->streamId);
		return NULL;
	}


	/* Check ClassOfService elements. */
	if (!tunnelManagerValidateCos(&pOpts->classOfService, isProvider, pErrorInfo))
		return NULL;


	if ((pTunnelImpl = (TunnelStreamImpl*)malloc(
					sizeof(TunnelStreamImpl))) == NULL)
	{
		rsslSetErrorInfo(pErrorInfo, RSSL_EIC_FAILURE, RSSL_RET_FAILURE, 
				__FILE__, __LINE__, "Failed to allocate tunnel object.");
		return NULL;
	}

	memset(pTunnelImpl, 0, sizeof(TunnelStreamImpl));

	rsslInitQueue(&pTunnelImpl->_tunnelBufferTransmitList);
	rsslInitQueue(&pTunnelImpl->_tunnelBufferWaitAckList);
	rsslInitQueue(&pTunnelImpl->_tunnelBufferTimeoutList);
	rsslInitQueue(&pTunnelImpl->_tunnelBufferImmediateList);

	pTunnelImpl->base.streamId = pOpts->streamId;
	pTunnelImpl->base.domainType = pOpts->domainType;
	pTunnelImpl->base.serviceId = pOpts->serviceId;
	pTunnelImpl->base.userSpecPtr = pOpts->userSpecPtr;
	pTunnelImpl->_persistLocally = pOpts->classOfService.guarantee.persistLocally;
	pTunnelImpl->_nextExpireTime = RDM_QMSG_TC_INFINITE;
	pTunnelImpl->_guaranteedOutputBuffersAppLimit = pOpts->guaranteedOutputBuffers;

	/* Add to manager's list now (tunnelStreamDestroy will remove the link) */
	rsslQueueAddLinkToBack(&pManagerImpl->_tunnelStreams, &pTunnelImpl->_managerLink);

	/* Store persistence file path, if provided. */
	if (pOpts->classOfService.guarantee.type == RDM_COS_GU_PERSISTENT_QUEUE
			&& pOpts->classOfService.guarantee.persistLocally
			&& pOpts->classOfService.guarantee.persistenceFilePath != NULL)
	{
		size_t strLength = strlen(pOpts->classOfService.guarantee.persistenceFilePath);
		if ((pTunnelImpl->_persistenceFilePath = 
					(char*)malloc(strlen(pOpts->classOfService.guarantee.persistenceFilePath))) 
				== NULL)
		{
			tunnelStreamDestroy((RsslTunnelStream*)pTunnelImpl);
			rsslSetErrorInfo(pErrorInfo, RSSL_EIC_FAILURE, RSSL_RET_FAILURE, 
					__FILE__, __LINE__, "Failed to allocate memory for storing persistence file path.");
			return NULL;
		}
		strncpy(pTunnelImpl->_persistenceFilePath, pOpts->classOfService.guarantee.persistenceFilePath, strLength + 1);
	}

	rsslClearState(&pTunnelImpl->base.state);
	pTunnelImpl->base.state.streamState = RSSL_STREAM_OPEN;
	pTunnelImpl->base.state.dataState = RSSL_DATA_SUSPECT;
	pTunnelImpl->base.state.code = RSSL_SC_NONE;

	pTunnelImpl->base.classOfService = pOpts->classOfService;

	pTunnelImpl->_statusEventCallback = pOpts->statusEventCallback;
	pTunnelImpl->_queueMsgCallback = pOpts->queueMsgCallback;
	pTunnelImpl->_defaultMsgCallback = pOpts->defaultMsgCallback;

	pTunnelImpl->_manager = (TunnelManagerImpl*)pManager;
	pTunnelImpl->base.pReactorChannel = pManager->_pReactorChannel;

	if (pOpts->classOfService.guarantee.type == RDM_COS_GU_PERSISTENT_QUEUE)
	{
		rsslInitQueue(&pTunnelImpl->_substreams);
		if (rsslHashTableInit(&pTunnelImpl->_substreamsById, 101, rsslHashU32Sum, rsslHashU32Compare,
					RSSL_TRUE, pErrorInfo) != RSSL_RET_SUCCESS)
		{
			tunnelStreamDestroy((RsslTunnelStream*)pTunnelImpl);
			return NULL;
		}
	}

	rsslInitQueue(&pTunnelImpl->_fragmentationProgressQueue);
	if (rsslHashTableInit(&pTunnelImpl->_fragmentationProgressHashTable, 11, rsslHashU16Sum, rsslHashU16Compare,
				RSSL_TRUE, pErrorInfo) != RSSL_RET_SUCCESS)
	{
		tunnelStreamDestroy((RsslTunnelStream*)pTunnelImpl);
		return NULL;
	}
	rsslInitQueue(&pTunnelImpl->_pendingBigBufferList);

	if (pTunnelImpl->base.classOfService.flowControl.type == RDM_COS_FC_BIDIRECTIONAL)
	{
		if (pTunnelImpl->base.classOfService.flowControl.recvWindowSize == TS_USE_DEFAULT_RECV_WINDOW_SIZE)
			pTunnelImpl->base.classOfService.flowControl.recvWindowSize = TS_DEFAULT_BIDRECTIONAL_WINDOW_SIZE;

		if ( (RsslUInt) pTunnelImpl->base.classOfService.flowControl.recvWindowSize < pTunnelImpl->base.classOfService.common.maxFragmentSize)
			pTunnelImpl->base.classOfService.flowControl.recvWindowSize = pTunnelImpl->base.classOfService.common.maxFragmentSize;
	}
	
	/* Make sure consumer provided login request to reuse if authenticating. */
	if (!isProvider)
	{
		assert(pRemoteCos == NULL);
		if (pOpts->classOfService.authentication.type == RDM_COS_AU_OMM_LOGIN)
		{
			RsslReactorChannelImpl *pReactorChannelImpl 
				= (RsslReactorChannelImpl*)pTunnelImpl->base.pReactorChannel;

			if (pOpts->pAuthLoginRequest != NULL)
			{
				/* Copy the login request provided in the authentication options. */
				RsslRet copyRet;

				if (pOpts->pAuthLoginRequest->rdmMsgBase.domainType != RSSL_DMT_LOGIN
						|| pOpts->pAuthLoginRequest->rdmMsgBase.rdmMsgType != RDM_LG_MT_REQUEST)
				{
					rsslSetErrorInfo(pErrorInfo, RSSL_EIC_FAILURE, RSSL_RET_FAILURE, 
							__FILE__, __LINE__, "Message provided for authentication does not appear to be a login request.");
					tunnelStreamDestroy((RsslTunnelStream*)pTunnelImpl);
					return NULL;
				}

				if ((pTunnelImpl->_pAuthLoginRequest = (RsslRDMLoginRequest*)rsslCreateRDMMsgCopy((RsslRDMMsg*)pOpts->pAuthLoginRequest, 128, &copyRet)) == NULL)
				{
					tunnelStreamDestroy((RsslTunnelStream*)pTunnelImpl);

					switch(copyRet)
					{
						case RSSL_RET_BUFFER_NO_BUFFERS: 
							rsslSetErrorInfo(pErrorInfo, RSSL_EIC_FAILURE, RSSL_RET_FAILURE, 
									__FILE__, __LINE__, "Failed to allocate memory for copying authentication login request.");
							return NULL;

						case RSSL_RET_INVALID_ARGUMENT:
							rsslSetErrorInfo(pErrorInfo, RSSL_EIC_FAILURE, RSSL_RET_INVALID_ARGUMENT, 
									__FILE__, __LINE__, "Failed to copy authenication login request; it does not appear to be a valid object.");
							return NULL;

						default:
							rsslSetErrorInfo(pErrorInfo, RSSL_EIC_FAILURE, RSSL_RET_FAILURE, 
									__FILE__, __LINE__, "Failed to copy authenication login request.");
							return NULL;
					}

				}

				pTunnelImpl->_authLoginStreamId = pTunnelImpl->_pAuthLoginRequest->rdmMsgBase.streamId;
				pTunnelImpl->_authLoginRequestIsCopied = RSSL_TRUE;

			}
			/* If the login request wasn't provided to the tunnel stream but the
			 * application expects to do it, return error.
			 * (If we discover later that we need the login request, we can get it later). */
			else if (pReactorChannelImpl->channelRole.ommConsumerRole.pLoginRequest == NULL)
			{
				rsslSetErrorInfo(pErrorInfo, RSSL_EIC_FAILURE, RSSL_RET_INVALID_ARGUMENT, 
						__FILE__, __LINE__, "Authentication requested but no login request provided via RsslTunnelStreamOpenOptions or RsslReactorOMMConsumerRole.");
				tunnelStreamDestroy((RsslTunnelStream*)pTunnelImpl);
				return NULL;
			}

		}

	}
	else
	{
		assert(pRemoteCos != NULL);
		if (pTunnelImpl->base.classOfService.flowControl.type != RDM_COS_FC_NONE)
			if ( (RsslUInt) pRemoteCos->flowControl.recvWindowSize < pTunnelImpl->base.classOfService.common.maxFragmentSize)
				pTunnelImpl->base.classOfService.flowControl.sendWindowSize = pTunnelImpl->base.classOfService.common.maxFragmentSize;
			else
				pTunnelImpl->base.classOfService.flowControl.sendWindowSize = pRemoteCos->flowControl.recvWindowSize;
		else
			pTunnelImpl->base.classOfService.flowControl.sendWindowSize = TS_USE_DEFAULT_RECV_WINDOW_SIZE;

		if (rsslHashTableFind(&pTunnelImpl->_manager->_streamIdToTunnelStreamTable, &pTunnelImpl->base.streamId, NULL)
				!= NULL)
		{
			rsslSetErrorInfo(pErrorInfo, RSSL_EIC_FAILURE, RSSL_RET_FAILURE, __FILE__, __LINE__, 
					"TunnelStream is already open for stream id %d", pTunnelImpl->base.streamId);
			tunnelStreamDestroy((RsslTunnelStream*)pTunnelImpl);
			return NULL;
		}
	}

	if (rsslHeapBufferInit(&pTunnelImpl->_memoryBuffer, 128) != RSSL_RET_SUCCESS)
	{
		rsslSetErrorInfo(pErrorInfo, RSSL_EIC_FAILURE, RSSL_RET_FAILURE, 
				__FILE__, __LINE__, "Memory allocation failed.");
		tunnelStreamDestroy((RsslTunnelStream*)pTunnelImpl);
		return NULL;
	}

	if (pOpts->name != NULL)
	{
		pTunnelImpl->_nameLength = (RsslUInt32)strlen(pOpts->name);

		if (pTunnelImpl->_nameLength > 255)
		{
			rsslSetErrorInfo(pErrorInfo, RSSL_EIC_FAILURE, RSSL_RET_INVALID_ARGUMENT, 
					__FILE__, __LINE__, "Tunnel stream name is too long.");
			tunnelStreamDestroy((RsslTunnelStream*)pTunnelImpl);
			return NULL;
		}

		if (!isProvider)
		{
			if ((pTunnelImpl->base.name = (char*)malloc(pTunnelImpl->_nameLength + 1)) == NULL)
			{
				rsslSetErrorInfo(pErrorInfo, RSSL_EIC_FAILURE, RSSL_RET_FAILURE, 
						__FILE__, __LINE__, "Memory allocation failed.");
				tunnelStreamDestroy((RsslTunnelStream*)pTunnelImpl);
				return NULL;
			}

			strncpy(pTunnelImpl->base.name, pOpts->name, pTunnelImpl->_nameLength);
			pTunnelImpl->base.name[pTunnelImpl->_nameLength] = '\0';
			pTunnelImpl->_isNameAllocated = RSSL_TRUE;
		}
		else
		{
			/* If this is a provider,  name is already allocated, so take that string instead of 
			 * making our own. */
			pTunnelImpl->base.name = pOpts->name;
			pTunnelImpl->_isNameAllocated = RSSL_TRUE;
		}
	}
	else
	{
		pTunnelImpl->base.name = (char*)"TunnelStream";
		pTunnelImpl->_nameLength = 12;
	}

	if (isProvider)
		pTunnelImpl->_state = TSS_SEND_REFRESH;
	else
	{
		/* Start response timer. */
		pTunnelImpl->_responseExpireTime = RDM_QMSG_TC_INFINITE;
		pTunnelImpl->_responseTimeout = pOpts->responseTimeout * 1000;
		pTunnelImpl->_state = TSS_SEND_REQUEST;
	}

	rsslHashLinkInit(&pTunnelImpl->_managerHashLink);
	rsslHashTableInsertLink(&pTunnelImpl->_manager->_streamIdToTunnelStreamTable, &pTunnelImpl->_managerHashLink, &pTunnelImpl->base.streamId, NULL);
	rsslQueueAddLinkToBack(&pTunnelImpl->_manager->_tunnelStreamsOpen, &pTunnelImpl->_managerOpenLink);
	pTunnelImpl->_flags |= TSF_ACTIVE;
	pTunnelImpl->_streamVersion = streamVersion;

	tunnelStreamSetNeedsDispatch(pTunnelImpl);

	return (RsslTunnelStream*)pTunnelImpl;
}

RsslRet tunnelStreamSubmitMsg(RsslTunnelStream *pTunnel, 
		RsslTunnelStreamSubmitMsgOptions *pOpts, RsslErrorInfo *pErrorInfo)
{
	TunnelStreamImpl*				pTunnelImpl = (TunnelStreamImpl*)pTunnel;
	RsslRet ret;
	RsslReactorChannel	*pReactorChannel = pTunnelImpl->_manager->base._pReactorChannel;
	RsslRDMMsg *pRdmMsg	= NULL;
	TunnelManagerImpl *pManagerImpl = (TunnelManagerImpl*)pTunnelImpl->_manager;
	RsslRDMQueueMsg queueMsg;
	RsslBool isQueueMsg = RSSL_FALSE;

	if (pTunnelImpl->_state != TSS_OPEN)
	{
		rsslSetErrorInfo(pErrorInfo, RSSL_EIC_FAILURE, RSSL_RET_INVALID_ARGUMENT, 
				__FILE__, __LINE__, "This stream is not open. Only open tunnel streams can submit messages.");
		return RSSL_RET_INVALID_ARGUMENT;
	}

	if (pOpts->pRsslMsg != NULL)
	{
		if (pOpts->pRDMMsg != NULL)
		{
			rsslSetErrorInfo(pErrorInfo, RSSL_EIC_FAILURE, RSSL_RET_INVALID_ARGUMENT,
					__FILE__, __LINE__, "Options should not provide both an RsslRDMMsg and RsslMsg.");
			return RSSL_RET_FAILURE;
		}

		if (pTunnel->classOfService.guarantee.type == RDM_COS_GU_PERSISTENT_QUEUE)
		{
			switch(pOpts->pRsslMsg->msgBase.domainType)
			{
				case RSSL_DMT_LOGIN:
				case RSSL_DMT_SOURCE:
				case RSSL_DMT_DICTIONARY:
				case RSSL_DMT_SYMBOL_LIST:
					break;
				default: /* Assume queue message. */ 
				{
					RsslBuffer memoryBuffer;
					RsslDecodeIterator dIter;

					/* No extra memory needed to decode queue message. */
					rsslClearBuffer(&memoryBuffer);

					rsslClearDecodeIterator(&dIter);
					rsslSetDecodeIteratorRWFVersion(&dIter, pTunnel->classOfService.common.protocolMajorVersion,
							pTunnel->classOfService.common.protocolMinorVersion);
					rsslSetDecodeIteratorBuffer(&dIter, &pOpts->pRsslMsg->msgBase.encDataBody);

					if ((ret = rsslDecodeRDMQueueMsg(&dIter, pOpts->pRsslMsg, &queueMsg, &memoryBuffer, pErrorInfo)) 
							!= RSSL_RET_SUCCESS)
						return RSSL_RET_FAILURE;

					pRdmMsg = (RsslRDMMsg*)&queueMsg;
					isQueueMsg = RSSL_TRUE;
					break;
				}
			}
		}
	}
	else if (pOpts->pRDMMsg != NULL)
	{
		if (pTunnel->classOfService.guarantee.type == RDM_COS_GU_PERSISTENT_QUEUE)
		{
			switch(pOpts->pRDMMsg->rdmMsgBase.domainType)
			{
				case RSSL_DMT_LOGIN:
				case RSSL_DMT_SOURCE:
				case RSSL_DMT_DICTIONARY:
				case RSSL_DMT_SYMBOL_LIST:
					break;
				default: /* Assume queue message. */
					isQueueMsg = RSSL_TRUE;
					pRdmMsg = pOpts->pRDMMsg;
					break;
			}
		}
		else
		{
			pRdmMsg = pOpts->pRDMMsg;
		}
	}
	else
	{
		rsslSetErrorInfo(pErrorInfo, RSSL_EIC_FAILURE, RSSL_RET_INVALID_ARGUMENT, 
				__FILE__, __LINE__, "No message provided.");
		return RSSL_RET_INVALID_ARGUMENT;
	}

	/* Handling an RDMMsg (either submitted as one or decoded from an RsslMsg that's a queue message). */
	if (pRdmMsg != NULL)
	{
		if (isQueueMsg)
		{
			RsslHashLink *pHashLink;
			TunnelSubstream *pSubstream;

			if ((pHashLink = rsslHashTableFind(&pTunnelImpl->_substreamsById,
							&pRdmMsg->rdmMsgBase.streamId, NULL)) != NULL)
				pSubstream = RSSL_HASH_LINK_TO_OBJECT(TunnelSubstream,
						_tunnelTableLink, pHashLink);
			else
				pSubstream = NULL;



			switch(pRdmMsg->rdmMsgBase.rdmMsgType)
			{
				case RDM_QMSG_MT_DATA:
				{
					TunnelSubstreamSubmitOptions submitOpts;

					if (pSubstream == NULL)
					{
						rsslSetErrorInfo(pErrorInfo, RSSL_EIC_FAILURE, RSSL_RET_INVALID_ARGUMENT, 
								__FILE__, __LINE__, "Unrecognized queue stream ID %d.", pRdmMsg->rdmMsgBase.streamId);
						return RSSL_RET_INVALID_ARGUMENT;
					}

					tunnelSubstreamSubmitOptionsClear(&submitOpts);
					submitOpts.pRDMMsg = pRdmMsg;

					if ((ret = tunnelSubstreamSubmit(pSubstream, &submitOpts, pErrorInfo))
							!= RSSL_RET_SUCCESS)
					{
						switch(ret)
						{
							case RSSL_RET_BUFFER_NO_BUFFERS:
							case RSSL_RET_PERSISTENCE_FULL:
							case RSSL_RET_INVALID_ARGUMENT:
							case RSSL_RET_INVALID_DATA:
								break;

							default:
								/* Fatal error. */
								pTunnelImpl->_interfaceError = RSSL_TRUE;
								tunnelStreamSetNeedsDispatch(pTunnelImpl);
								break;
						}

						return ret;
					}
					break;
				}

				case RDM_QMSG_MT_REQUEST:
					if (pSubstream != NULL)
					{
						rsslSetErrorInfo(pErrorInfo, RSSL_EIC_FAILURE, RSSL_RET_INVALID_ARGUMENT, 
								__FILE__, __LINE__, "Queue stream with ID %d is already open.", pRdmMsg->rdmMsgBase.streamId);
						return RSSL_RET_INVALID_ARGUMENT;
					}

					if ((pSubstream = tunnelSubstreamOpen(pTunnel,
									&pRdmMsg->queueMsg.request,
									pErrorInfo)) == NULL)
						return pErrorInfo->rsslError.rsslErrorId;

					rsslHashTableInsertLink(&pTunnelImpl->_substreamsById, &pSubstream->_tunnelTableLink, &pSubstream->_streamId, NULL);
					rsslQueueAddLinkToBack(&pTunnelImpl->_substreams, &pSubstream->_tunnelQueueLink);
					break;

				case RDM_QMSG_MT_CLOSE:
					if (pSubstream == NULL)
					{
						rsslSetErrorInfo(pErrorInfo, RSSL_EIC_FAILURE, RSSL_RET_INVALID_ARGUMENT, 
								__FILE__, __LINE__, "Unrecognized queue stream ID %d.", pRdmMsg->rdmMsgBase.streamId);
						return RSSL_RET_INVALID_ARGUMENT;
					}

					if ((ret = tunnelSubstreamClose(pSubstream, pErrorInfo)) != RSSL_RET_SUCCESS)
						return ret;

					rsslHashTableRemoveLink(&pTunnelImpl->_substreamsById, &pSubstream->_tunnelTableLink);
					rsslQueueRemoveLink(&pTunnelImpl->_substreams, &pSubstream->_tunnelQueueLink);

					tunnelStreamSetNeedsDispatch(pTunnelImpl);

					break;

				default:
					rsslSetErrorInfo(pErrorInfo, RSSL_EIC_FAILURE, RSSL_RET_INVALID_ARGUMENT, 
							__FILE__, __LINE__, "Received unhandled queue message class.");
					return RSSL_RET_INVALID_ARGUMENT;
			}
		}
		else
		{
			/* No queue message handling -- just encode and send it. */
			RsslEncodeIterator eIter;
			RsslBuffer *pBuffer;

			RsslTunnelStreamGetBufferOptions bufferOpts;

			rsslClearTunnelStreamGetBufferOptions(&bufferOpts);
			bufferOpts.size = (RsslUInt32)pTunnelImpl->base.classOfService.common.maxFragmentSize;
			if ((pBuffer = (RsslBuffer*)tunnelStreamGetBuffer(pTunnelImpl,
				(RsslUInt32)pTunnel->classOfService.common.maxFragmentSize, RSSL_FALSE, RSSL_FALSE, pErrorInfo)) == NULL)
				return pErrorInfo->rsslError.rsslErrorId;

			rsslClearEncodeIterator(&eIter);
			rsslSetEncodeIteratorRWFVersion(&eIter, pTunnelImpl->base.classOfService.common.protocolMajorVersion,
					pTunnelImpl->base.classOfService.common.protocolMinorVersion);
			rsslSetEncodeIteratorBuffer(&eIter, pBuffer);

			if ((ret = rsslEncodeRDMMsg(&eIter, pRdmMsg, &pBuffer->length, pErrorInfo))
					!= RSSL_RET_SUCCESS) 
			{
				RsslErrorInfo errorInfo;

				if (pErrorInfo->rsslError.rsslErrorId == RSSL_RET_BUFFER_TOO_SMALL)
					rsslSetErrorInfo(pErrorInfo, RSSL_EIC_FAILURE, ret,
							__FILE__, __LINE__, "Failed to encode RDM message -- the ClassOfService.common.maxMsgSize is too small.");

				rsslTunnelStreamReleaseBuffer(pBuffer, &errorInfo);
				return ret;
			}


			if ((ret = tunnelStreamEnqueueBuffer(pTunnel,
							pBuffer, RSSL_DT_MSG, pErrorInfo)) != RSSL_RET_SUCCESS)
			{
				rsslTunnelStreamReleaseBuffer(pBuffer, pErrorInfo);
				rsslSetErrorInfoLocation(pErrorInfo, __FILE__, __LINE__);
				return ret;
			}


			return RSSL_RET_SUCCESS;
		}
	}
	else // RsslMsg that's not a queue message
	{
		// get encoded message size estimate
		RsslUInt32 bufLength = _reactorMsgEncodedSize(pOpts->pRsslMsg);

		// get buffer of estimated size
		TunnelBufferImpl *pBufferImpl = tunnelStreamGetBuffer(pTunnelImpl, bufLength, RSSL_FALSE, RSSL_FALSE, pErrorInfo);

		if (pBufferImpl != NULL)
		{
			RsslEncodeIterator eIter;
			RsslRet ret;

			// set encode iterator buffer
			rsslClearEncodeIterator(&eIter);
			rsslSetEncodeIteratorRWFVersion(&eIter, pReactorChannel->majorVersion, pReactorChannel->minorVersion);
			rsslSetEncodeIteratorBuffer(&eIter, &pBufferImpl->_poolBuffer.buffer);

			// encode message into buffer
			if ((ret = rsslEncodeMsg(&eIter, pOpts->pRsslMsg)) == RSSL_RET_SUCCESS)
			{
				RsslTunnelStreamSubmitOptions submitOptions;
				rsslClearTunnelStreamSubmitOptions(&submitOptions);

				// submit encoded buffer
				submitOptions.containerType = RSSL_DT_MSG;
				pBufferImpl->_poolBuffer.buffer.length = rsslGetEncodedBufferLength(&eIter);
				if ((ret = tunnelStreamSubmitBuffer(pTunnel, &pBufferImpl->_poolBuffer.buffer, &submitOptions, pErrorInfo)) < RSSL_RET_SUCCESS)
				{
					return ret;
				}
			}
			else
			{
				return ret;
			}
		}
		else
		{
			return pErrorInfo->rsslError.rsslErrorId;
		}
	}

	if (pTunnelImpl->_tunnelBufferTransmitList.count > 0)
		tunnelStreamSetNeedsDispatch(pTunnelImpl);

	return RSSL_RET_SUCCESS;
}

RsslRet tunnelStreamSubmitBuffer(RsslTunnelStream *pTunnel, RsslBuffer *pBuffer,
		RsslTunnelStreamSubmitOptions *pOptions, RsslErrorInfo *pErrorInfo)
{
	RsslRDMQueueMsg						queueMsg;
	RsslDecodeIterator					dIter;
	RsslRet								ret;
	RsslBuffer							memoryBuffer;
	RsslMsg								msg;
	RsslTunnelStreamSubmitMsgOptions	submitOpts;
	TunnelStreamImpl					*pTunnelImpl = (TunnelStreamImpl*)pTunnel;
	TunnelBufferImpl					*pBufferImpl = (TunnelBufferImpl*)pBuffer;

	if (pTunnelImpl->_state != TSS_OPEN)
	{
		rsslSetErrorInfo(pErrorInfo, RSSL_EIC_FAILURE, RSSL_RET_INVALID_ARGUMENT, 
				__FILE__, __LINE__, "This stream is not open. Only open tunnel streams can submit buffers.");
		return RSSL_RET_INVALID_ARGUMENT;
	}

	if (pBufferImpl->_integrity != TS_BUFFER_INTEGRITY)
	{
		rsslSetErrorInfo(pErrorInfo, RSSL_EIC_FAILURE, RSSL_RET_INVALID_ARGUMENT, 
				__FILE__, __LINE__, "Integrity check failed. This does not appear to be a valid tunnel stream buffer.");
		return RSSL_RET_INVALID_ARGUMENT;
	}

	if (pBufferImpl->_tunnel != pTunnel)
	{
		rsslSetErrorInfo(pErrorInfo, RSSL_EIC_FAILURE, RSSL_RET_INVALID_ARGUMENT, 
				__FILE__, __LINE__, "This buffer does not belong to this tunnel stream.");
		return RSSL_RET_INVALID_ARGUMENT;
	}

	if (pBuffer->length == 0)
	{
		rsslSetErrorInfo(pErrorInfo, RSSL_EIC_FAILURE, RSSL_RET_INVALID_ARGUMENT, 
				__FILE__, __LINE__, "Cannot submit a buffer of zero length.");
		return RSSL_RET_INVALID_ARGUMENT;
	}

	if (pBuffer->length > pBufferImpl->_maxLength)
	{
		rsslSetErrorInfo(pErrorInfo, RSSL_EIC_FAILURE, RSSL_RET_INVALID_ARGUMENT, 
				__FILE__, __LINE__, "Buffer submitted length is longer than its initial maximum length.");
		return RSSL_RET_INVALID_ARGUMENT;
	}

	if (!pBufferImpl->_isBigBuffer) // not big buffer
	{
		rsslClearDecodeIterator(&dIter);
		rsslSetDecodeIteratorRWFVersion(&dIter, pTunnel->classOfService.common.protocolMajorVersion,
				pTunnel->classOfService.common.protocolMinorVersion);
		rsslSetDecodeIteratorBuffer(&dIter, pBuffer);

		if (pOptions->containerType == RSSL_DT_MSG
				&& pTunnel->classOfService.guarantee.type == RDM_COS_GU_PERSISTENT_QUEUE)
		{
			if ((ret = rsslDecodeMsg(&dIter, &msg)) != RSSL_RET_SUCCESS)
			{
				rsslSetErrorInfo(pErrorInfo, RSSL_EIC_FAILURE, ret, 
						__FILE__, __LINE__, "Failed to decode submitted message.");
				return RSSL_RET_INVALID_ARGUMENT;
			}

			switch(msg.msgBase.domainType)
			{
				case RSSL_DMT_LOGIN:
				case RSSL_DMT_SOURCE:
				case RSSL_DMT_DICTIONARY:
				case RSSL_DMT_SYMBOL_LIST:
					break;

				default:
				{
					/* No extra memory needed to decode queue message. */
					rsslClearBuffer(&memoryBuffer);

					if ((ret = rsslDecodeRDMQueueMsg(&dIter, &msg, &queueMsg, &memoryBuffer, pErrorInfo)) 
							!= RSSL_RET_SUCCESS)
						return RSSL_RET_FAILURE;

					rsslClearTunnelStreamSubmitMsgOptions(&submitOpts);
					submitOpts.pRDMMsg = (RsslRDMMsg*)&queueMsg;

					if ((ret = tunnelStreamSubmitMsg(pTunnel, &submitOpts, pErrorInfo)) != RSSL_RET_SUCCESS)
						return ret;

					/* This buffer no longer used, release it. */
					tunnelStreamReleaseBuffer(pTunnelImpl, pBufferImpl);

					return RSSL_RET_SUCCESS;
				}
			}
		}

		if ((ret = tunnelStreamEnqueueBuffer(pTunnel, pBuffer, pOptions->containerType, pErrorInfo)) != RSSL_RET_SUCCESS)
			return ret;
	}
	else // big buffer
	{
		ret = tunnelStreamEnqueueBigBuffer(pTunnel, pBufferImpl, pOptions->containerType, pErrorInfo);
		if (ret > RSSL_RET_SUCCESS)
		{
	    	// release big buffer since now fully fragmented
			bigBufferPoolRelease(&pTunnelImpl->_bigBufferPool, &pBufferImpl->_poolBuffer);
		}
		else if (ret < RSSL_RET_SUCCESS)
		{
			return ret;
		}
	}

	return RSSL_RET_SUCCESS;
}

RsslRet tunnelStreamRead(RsslTunnelStream *pTunnel, RsslMsg *pMsg, RsslErrorInfo *pErrorInfo)
{
	TunnelStreamImpl	*pTunnelImpl = (TunnelStreamImpl*)pTunnel;
	TunnelStreamMsg		streamMsg;
	RsslRet				ret;
	RsslReactorChannel	*pReactorChannel = pTunnelImpl->_manager->base._pReactorChannel;
	AckRangeList 		ackRangeList, nakRangeList;

	if ((ret = tunnelStreamMsgDecode(pMsg, &streamMsg, &ackRangeList, &nakRangeList, pTunnelImpl->_streamVersion) != RSSL_RET_SUCCESS))
	{
		rsslSetErrorInfo(pErrorInfo, RSSL_EIC_FAILURE, 
				ret, __FILE__, __LINE__,
				"Failed to decode stream message.");
		return RSSL_RET_FAILURE;
	}

	switch(streamMsg.base.opcode)
	{
		case TS_MC_REFRESH:
		{
			if ((ret = tunnelStreamHandleState(pTunnelImpl, &streamMsg.refreshHeader.state, 
							pMsg, &streamMsg, NULL, RSSL_FALSE, pErrorInfo)) != RSSL_RET_SUCCESS)
				return ret;

			break;
		}

		case TS_MC_STATUS:
		{
        	if (!_tunnelStreamHandleRequestRetry(pTunnelImpl))
			{
				if ((ret = tunnelStreamHandleState(pTunnelImpl, 
								(streamMsg.statusHeader.flags & TS_STMF_HAS_STATE) ? &streamMsg.statusHeader.state : NULL,
								pMsg, &streamMsg, NULL, RSSL_FALSE, pErrorInfo)) != RSSL_RET_SUCCESS)
					return ret;
			}

			break;
		}

		case TS_MC_CLOSE:
		{
			RsslState state;

			rsslClearState(&state);
			state.streamState = RSSL_STREAM_CLOSED;
			state.dataState = RSSL_DATA_SUSPECT;
			state.text.data = (char*)"Received a close message from the remote end.";
			state.text.length = 45;

			if ((ret = tunnelStreamHandleState(pTunnelImpl, &state, pMsg, &streamMsg, NULL, RSSL_FALSE, 
							pErrorInfo)) != RSSL_RET_SUCCESS)
				return ret;

			break;
		}

		case TS_MC_ACK:
		{
			TunnelStreamAck *pAckMsg = (TunnelStreamAck*)&streamMsg;
			RsslQueueLink *pLink;
			RsslUInt32 ui;
			RsslQueue retransmitQueue;
			RsslRet ret;

			switch(pTunnelImpl->_state)
			{

				case TSS_SEND_REQUEST:
				case TSS_INACTIVE:
				case TSS_WAIT_REFRESH:
				case TSS_SEND_REFRESH:
					/* Ignore -- may be an old message. */
					return RSSL_RET_SUCCESS;

				case TSS_SEND_AUTH_LOGIN_REQUEST:
				case TSS_WAIT_AUTH_LOGIN_RESPONSE:
				case TSS_OPEN:
				case TSS_SEND_FIN:
				case TSS_WAIT_FIN:
				case TSS_SEND_ACK_OF_FIN:
				case TSS_WAIT_ACK_OF_FIN:
				{
					if (!(pAckMsg->flags & TS_ACKF_FIN)) 
					{
						if (tunnelStreamDebugFlags & TS_DBG_ACKS)
						{
							printf("<TunnelStreamDebug streamId:%d> Received AckMsg seqNum: %u, recvWindow: %d\n", 
									pTunnelImpl->base.streamId, pAckMsg->seqNum, pAckMsg->recvWindow);
						} 

						pTunnelImpl->base.classOfService.flowControl.sendWindowSize = pAckMsg->recvWindow;

						/* Acknowledge messages up to the cumulative sequence number. */
						for (pLink = rsslQueueStart(&pTunnelImpl->_tunnelBufferWaitAckList);
								pLink != NULL;
								pLink = rsslQueueForth(&pTunnelImpl->_tunnelBufferWaitAckList))
						{
							TunnelBufferImpl *pBufferImpl =
								RSSL_QUEUE_LINK_TO_OBJECT(TunnelBufferImpl, _tbpLink, pLink);

							if (rsslSeqNumCompare(pBufferImpl->_seqNum, pAckMsg->seqNum) <= 0)
								_tunnelStreamFreeAckedBuffer(pTunnelImpl, pBufferImpl);
						}

						/* Acknowledge buffers in ack ranges. */
						if (tunnelStreamDebugFlags & TS_DBG_ACKS && ackRangeList.count > 0)
						{
							RsslUInt32 ui;

							printf("<TunnelStreamDebug streamId:%d> Received selective ack for", pTunnelImpl->base.streamId);
							for(ui = 0; ui < ackRangeList.count * 2; ui += 2)
								printf(" %u-%u", ackRangeList.rangeArray[ui], ackRangeList.rangeArray[ui + 1]);

							printf ("\n");
						}

						pLink = rsslQueueStart(&pTunnelImpl->_tunnelBufferWaitAckList);

						for(ui = 0; ui < ackRangeList.count * 2; ui += 2)
						{
							while(pLink != NULL)
							{
								/* Remove each buffer within the ack range.
								 * Have to check the whole list, as the buffers in 
								 * _tunnelBufferWaitAckList aren't necessarily in order. */

								TunnelBufferImpl *pBufferImpl =
									RSSL_QUEUE_LINK_TO_OBJECT(TunnelBufferImpl, _tbpLink, pLink);

								/* If buffer is in current range, free it. */
								if (rsslSeqNumCompare(pBufferImpl->_seqNum, ackRangeList.rangeArray[ui]) >= 0
										&& rsslSeqNumCompare(pBufferImpl->_seqNum, ackRangeList.rangeArray[ui + 1]) <= 0)
									_tunnelStreamFreeAckedBuffer(pTunnelImpl, pBufferImpl);

								pLink = rsslQueueForth(&pTunnelImpl->_tunnelBufferWaitAckList);
							}
						}

						/* Retransmit buffers in nak ranges. */
						if (tunnelStreamDebugFlags & TS_DBG_ACKS && nakRangeList.count > 0)
						{
							RsslUInt32 ui;

							printf("<TunnelStreamDebug streamId:%d> Received nack for", pTunnelImpl->base.streamId);
							for(ui = 0; ui < nakRangeList.count * 2; ui += 2)
								printf(" %u-%u", nakRangeList.rangeArray[ui], nakRangeList.rangeArray[ui + 1]);

							printf ("\n");
						}

						rsslInitQueue(&retransmitQueue);
						for(ui = 0; ui < nakRangeList.count * 2; ui += 2)
						{
							RsslUInt32 uj;

							for (uj = nakRangeList.rangeArray[ui]; 
									uj <= nakRangeList.rangeArray[ui+1]; ++uj)
							{
								TunnelBufferImpl *pBufferImpl;

								if ((pBufferImpl = _tunnelStreamGetBufferWithSeqNum(
												&pTunnelImpl->_tunnelBufferWaitAckList, uj)) != NULL)
								{
									RsslEncodeIterator eIter;

									/* Move this buffer back to the transmit list. */
									if (pBufferImpl->_bufferType == TS_BT_DATA)
									{
										/* Change message opcode to indicate this is a retransmission. */
										rsslClearEncodeIterator(&eIter);
										rsslSetEncodeIteratorRWFVersion(&eIter, pReactorChannel->majorVersion,
												pReactorChannel->minorVersion);
										rsslSetEncodeIteratorBuffer(&eIter, &pBufferImpl->_poolBuffer.buffer);
										if ((ret = tunnelStreamDataReplaceOpcode(&eIter, TS_MC_RETRANS))
												!= RSSL_RET_SUCCESS)
										{
											rsslSetErrorInfo(pErrorInfo, RSSL_EIC_FAILURE, 
													ret, __FILE__, __LINE__,
													"Setting retransmission opcode on data message failed.");
											/* Put existing retransmits onto transmit queue so they are cleaned up. */
											rsslQueuePrepend(&pTunnelImpl->_tunnelBufferTransmitList, &retransmitQueue);
											return RSSL_RET_FAILURE;
										}

										pTunnelImpl->_bytesWaitingAck -= pBufferImpl->_poolBuffer.buffer.length;
									}

									rsslQueueRemoveLink(&pTunnelImpl->_tunnelBufferWaitAckList, &pBufferImpl->_tbpLink);
									rsslQueueAddLinkToBack(&retransmitQueue, &pBufferImpl->_tbpLink);

									if (tunnelStreamDebugFlags & TS_DBG_ACKS)
									{
										printf("<TunnelStreamDebug streamId:%d> Moving message %u back for retransmission\n", pTunnelImpl->base.streamId,
												pBufferImpl->_seqNum);
									}
								}
								else if ((_tunnelStreamGetBufferWithSeqNum(
												&pTunnelImpl->_tunnelBufferTransmitList, uj)) != NULL)
								{
									/* Ignore -- message already queued for retransmission. */
								}
								else
								{
									/* Buffer is not present in transmission queue
									 * or waiting ack queue. */
									rsslSetErrorInfo(pErrorInfo, RSSL_EIC_FAILURE, 
											RSSL_RET_FAILURE, __FILE__, __LINE__,
											"Cannot find message requested for retransmission.");
									/* Put existing retransmits onto transmit queue so they are cleaned up. */
									rsslQueuePrepend(&pTunnelImpl->_tunnelBufferTransmitList, &retransmitQueue);
									return RSSL_RET_FAILURE;
								}
							}
						}

						/* Put any retransmissions back on the list. */
						rsslQueuePrepend(&pTunnelImpl->_tunnelBufferTransmitList, &retransmitQueue);

						/* Don't unset the response time while the tunnel stream is waiting for the authentication response. */
						if (pTunnelImpl->_state != TSS_WAIT_AUTH_LOGIN_RESPONSE)
						{
							_tunnelStreamUnsetResponseTimer(pTunnelImpl);
						}

						if (rsslQueueGetElementCount(&pTunnelImpl->_tunnelBufferTransmitList) > 0)
							tunnelStreamSetNeedsDispatch(pTunnelImpl);

						if (pTunnelImpl->_state > TSS_SEND_FIN)
						{
							/* Previously sent a FIN, check current state of stream. */
							if (!(pTunnelImpl->_flags & TSF_FIN_RECEIVED))
							{
								/* Still waiting for the remote end's FIN. */
								if (pTunnelImpl->_state != TSS_WAIT_FIN)
									_tunnelStreamSetResponseTimerWithBackoff(pTunnelImpl);
								pTunnelImpl->_state = TSS_WAIT_FIN;
							}
							else if (!(pTunnelImpl->_flags & TSF_SENT_ACK_OF_FIN))
							{
								/* Received the FIN but haven't acknowledged it yet. */
								pTunnelImpl->_state = TSS_SEND_ACK_OF_FIN;
								pTunnelImpl->_retransRetryCount = 0;
								tunnelStreamSetNeedsDispatch(pTunnelImpl);
							}
							else if (pTunnelImpl->_flags & TSF_RECEIVED_ACK_OF_FIN)
							{
								/* Got the remote FIN and we know the remote end
								 * got our FIN. We are done. */
								if (pTunnelImpl->_flags & TSF_STARTED_CLOSE)
								{
									pTunnelImpl->_state = TSS_SEND_CLOSE;
									tunnelStreamSetNeedsDispatch(pTunnelImpl);
								}
								else
								{
									pTunnelImpl->_state = TSS_WAIT_CLOSE;
								}
							}
						}
					}
					else
					{
						/* This is a FIN message. Count it against other received
						 * data messages. */
						RsslInt32 diff = rsslSeqNumCompare(pAckMsg->seqNum, pTunnelImpl->_lastInSeqNumAccepted);

						if (tunnelStreamDebugFlags & TS_DBG_ACKS)
						{
							printf("<TunnelStreamDebug streamId:%d> Received FinMsg seqNum: %u\n", 
									pTunnelImpl->base.streamId, pAckMsg->seqNum);
						} 

						if (diff == 1)
						{
							/* Need to send an ack for it. */
							++pTunnelImpl->_lastInSeqNumAccepted;
							if (rsslSeqNumCompare(pTunnelImpl->_lastInSeqNumAccepted, pTunnelImpl->_lastInSeqNum) > 0)
								pTunnelImpl->_lastInSeqNum = pTunnelImpl->_lastInSeqNumAccepted;
							pTunnelImpl->_flags |= TSF_FIN_RECEIVED;

							if (pTunnelImpl->_state <= TSS_OPEN)
							{
								RsslState state;
								rsslClearState(&state);

								state.streamState = RSSL_STREAM_OPEN;
								state.dataState = RSSL_DATA_SUSPECT;
								state.text.data = (char*)"Received final message for stream, closing it.";
								state.text.length = 46;

								if ((ret = tunnelStreamHandleState(pTunnelImpl, &state,
										pMsg, NULL, NULL, RSSL_FALSE, pErrorInfo)) != RSSL_RET_SUCCESS)
									return ret;

								/* Tunnel was open; send our own FIN in response. */
								pTunnelImpl->_state = TSS_SEND_FIN;
								pTunnelImpl->_retransRetryCount = 0;
							}
							else if (pTunnelImpl->_state == TSS_WAIT_FIN)
							{
								/* We sent our FIN already, just need to send our acknowledgement of the 
								 * received FIN. */
								pTunnelImpl->_state = TSS_SEND_ACK_OF_FIN;
							}

							_tunnelStreamUnsetResponseTimer(pTunnelImpl);

							tunnelStreamSetNeedsDispatch(pTunnelImpl);
						}
						else if (diff < 1)
						{
							/* If we receive a FIN and are otherwise good, resend the FIN ACK. */
							if (pAckMsg->flags & TS_ACKF_FIN
									&& pTunnelImpl->_lastInAckedSeqNum == pTunnelImpl->_lastInSeqNumAccepted)
								pTunnelImpl->_lastInAckedSeqNum = pTunnelImpl->_lastInSeqNumAccepted - 1;
							tunnelStreamSetNeedsDispatch(pTunnelImpl);
						}
						else
						{
							/* Missing messages. Need to send a nack for them. */
							if (rsslSeqNumCompare(pAckMsg->seqNum, pTunnelImpl->_lastInSeqNum) > 0)
								pTunnelImpl->_lastInSeqNum = pAckMsg->seqNum;
							pTunnelImpl->_flags |= TSF_SEND_NACK;
							tunnelStreamSetNeedsDispatch(pTunnelImpl);
						}
					}
					break;
				}

				

			}

			break;
		}

		case TS_MC_DATA:
		case TS_MC_RETRANS:
		{
			TunnelStreamData *pDataMsg = (TunnelStreamData*)&streamMsg;
			RsslInt32 diff = rsslSeqNumCompare(pDataMsg->seqNum, pTunnelImpl->_lastInSeqNumAccepted);

			if (tunnelStreamDebugFlags & TS_DBG_ACKS)
			{
				printf("<TunnelStreamDebug streamId:%d> Received message seqNum: %u, Latest received seqNum: %u, latest accepted seqNum: %u, last acked seqNum: %u\n", pTunnelImpl->base.streamId, 
						pDataMsg->seqNum, pTunnelImpl->_lastInSeqNum, pTunnelImpl->_lastInSeqNumAccepted, pTunnelImpl->_lastInAckedSeqNum);
			}

			if (pTunnelImpl->_state <= TSS_WAIT_REFRESH)
			{
				rsslSetErrorInfo(pErrorInfo, RSSL_EIC_FAILURE, 
						RSSL_RET_FAILURE, __FILE__, __LINE__, "Received unexpected data message.");
				return RSSL_RET_FAILURE;
			}

			if (diff == 1)
			{
				RsslDecodeIterator dIter;
				RsslMsg substreamMsg;

				/* Will need to send ack for this message. */
				++pTunnelImpl->_lastInSeqNumAccepted;
				if (rsslSeqNumCompare(pTunnelImpl->_lastInSeqNumAccepted, pTunnelImpl->_lastInSeqNum) > 0)
					pTunnelImpl->_lastInSeqNum = pTunnelImpl->_lastInSeqNumAccepted;
				tunnelStreamSetNeedsDispatch(pTunnelImpl);

				if (!(pTunnelImpl->_flags & (TSF_ACTIVE | TSF_NEED_FINAL_STATUS_EVENT)))
					return RSSL_RET_SUCCESS; /* Client is no longer expecting events; do not process this message beyond acknowledging. */

				if (pTunnelImpl->base.classOfService.common.protocolType == RSSL_RWF_PROTOCOL_TYPE
						&& pMsg->msgBase.containerType == RSSL_DT_MSG)
				{
					rsslClearDecodeIterator(&dIter);
					rsslSetDecodeIteratorRWFVersion(&dIter, pTunnel->classOfService.common.protocolMajorVersion,
						pTunnel->classOfService.common.protocolMinorVersion);
					rsslSetDecodeIteratorBuffer(&dIter, &pMsg->msgBase.encDataBody);

					if ((ret = rsslDecodeMsg(&dIter, &substreamMsg)) != RSSL_RET_SUCCESS)
					{
						rsslSetErrorInfo(pErrorInfo, RSSL_EIC_FAILURE, ret, 
							__FILE__, __LINE__, "Failed to decode substream message.");
						return RSSL_RET_FAILURE;
					}

					switch(substreamMsg.msgBase.domainType)
					{
						case RSSL_DMT_LOGIN:
						{
							if (((RsslReactorChannelImpl*)pReactorChannel)->channelRole.base.roleType != RSSL_RC_RT_OMM_PROVIDER
								&& pTunnelImpl->_state >= TSS_WAIT_AUTH_LOGIN_RESPONSE
								&& pTunnelImpl->base.classOfService.authentication.type == RDM_COS_AU_OMM_LOGIN
								&& pTunnelImpl->_authLoginStreamId == substreamMsg.msgBase.streamId)
							{
								RsslRDMLoginMsg loginMsg;
								RsslBuffer memoryBuffer = pTunnelImpl->_memoryBuffer;
								RsslTunnelStreamAuthInfo authInfo;

								do
								{
									if ((ret = rsslDecodeRDMLoginMsg(&dIter, &substreamMsg, &loginMsg, &memoryBuffer, pErrorInfo))
										== RSSL_RET_SUCCESS)
										break;

									if (ret == RSSL_RET_BUFFER_TOO_SMALL)
									{
										if (rsslHeapBufferResize(&pTunnelImpl->_memoryBuffer,
											pTunnelImpl->_memoryBuffer.length * 2, RSSL_FALSE)
											!= RSSL_RET_SUCCESS)
										{
											rsslSetErrorInfo(pErrorInfo, RSSL_EIC_FAILURE, RSSL_RET_FAILURE, __FILE__, __LINE__, 
												"Memory allocation failed.");
											return RSSL_RET_FAILURE;
										}
									}
									else
									{
										rsslSetErrorInfo(pErrorInfo, RSSL_EIC_FAILURE, RSSL_RET_FAILURE, __FILE__, __LINE__, 
											"Failed to decode login authentication response: %d", pErrorInfo->rsslError.rsslErrorId);
										return RSSL_RET_FAILURE;
									}
								} while (ret != RSSL_RET_SUCCESS);

								switch(loginMsg.rdmMsgBase.rdmMsgType)
								{
								case RDM_LG_MT_REFRESH:
									memset(&authInfo, 0, sizeof(RsslTunnelStreamAuthInfo));
									authInfo.pLoginMsg = &loginMsg;

									return tunnelStreamHandleState(pTunnelImpl, &loginMsg.refresh.state,
										pMsg, &streamMsg, &authInfo, RSSL_FALSE, pErrorInfo);

								case RDM_LG_MT_STATUS:
									memset(&authInfo, 0, sizeof(RsslTunnelStreamAuthInfo));
									authInfo.pLoginMsg = &loginMsg;

									return tunnelStreamHandleState(pTunnelImpl, 
										(loginMsg.status.flags & RDM_LG_STF_HAS_STATE) ? &loginMsg.status.state : NULL, 
										pMsg, &streamMsg, &authInfo, RSSL_FALSE, pErrorInfo);
								default:
									rsslSetErrorInfo(pErrorInfo, RSSL_EIC_FAILURE, 
										RSSL_RET_FAILURE, __FILE__, __LINE__,
										"Received unexpected login rdmMsgType %u on authentication stream", 
										loginMsg.rdmMsgBase.rdmMsgType);
									return RSSL_RET_FAILURE;
								}

							}
							else
							{
								if (tunnelStreamCallMsgCallback(pTunnelImpl, &pMsg->msgBase.encDataBody, &substreamMsg, RSSL_DT_MSG, pErrorInfo)
									!= RSSL_RET_SUCCESS)
									return RSSL_RET_FAILURE;

							}
							break;
						}

						default:
						{
							if (pTunnelImpl->base.classOfService.guarantee.type == RDM_COS_GU_PERSISTENT_QUEUE)
							{
								switch(substreamMsg.msgBase.domainType)
								{
									case RSSL_DMT_SOURCE:
									case RSSL_DMT_DICTIONARY:
									case RSSL_DMT_SYMBOL_LIST:
									{
										/* Ignore non-login messages if waiting for refresh or authentication */
										if (pTunnelImpl->_state < TSS_OPEN)
											break;

										if (tunnelStreamCallMsgCallback(pTunnelImpl, &pMsg->msgBase.encDataBody, &substreamMsg, RSSL_DT_MSG, pErrorInfo)
												!= RSSL_RET_SUCCESS)
											return RSSL_RET_FAILURE;
										break;
									}

									default:
									{
										RsslHashLink *pHashLink;
										TunnelSubstream *pSubstream;

										/* Ignore non-login messages if waiting for refresh or authentication */
										if (pTunnelImpl->_state < TSS_OPEN)
											break;

										if ((pHashLink = rsslHashTableFind(&pTunnelImpl->_substreamsById,
											&substreamMsg.msgBase.streamId, NULL)) != NULL)
										{
											pSubstream = RSSL_HASH_LINK_TO_OBJECT(TunnelSubstream,
												_tunnelTableLink, pHashLink);

											if ((ret = tunnelSubstreamRead(pSubstream, &substreamMsg, pErrorInfo))
												!= RSSL_RET_SUCCESS)
												return RSSL_RET_FAILURE;
										}
										/* Otherwise ignore. */

										break;
									}
								}
							}
							else
							{
								/* Ignore non-login messages if waiting for refresh or authentication */
								if (pTunnelImpl->_state < TSS_OPEN)
									break;

								if (tunnelStreamCallMsgCallback(pTunnelImpl, &pMsg->msgBase.encDataBody, &substreamMsg, RSSL_DT_MSG, pErrorInfo)
										!= RSSL_RET_SUCCESS)
									return RSSL_RET_FAILURE;
								break;
							}
						}
					}
				}
				else
				{
					if (!(pDataMsg->flags & TS_DF_FRAGMENTED)) // non-fragmented message
					{
						if (tunnelStreamCallMsgCallback(pTunnelImpl, &pMsg->msgBase.encDataBody, NULL, pMsg->msgBase.containerType, pErrorInfo)
							!= RSSL_RET_SUCCESS)
							return RSSL_RET_FAILURE;
					}
					else // fragmented message
					{
						return _tunnelStreamHandleFragmentedMsg(pTunnelImpl, pDataMsg, &pMsg->msgBase.encDataBody, pErrorInfo);
					}
				}
			}
			else if (diff < 1)
			{
				/* Old data. Ignore. */
			}
			else
			{
				/* Missing messages. Need to send a nack for them. */
				if (rsslSeqNumCompare(pDataMsg->seqNum, pTunnelImpl->_lastInSeqNum) > 0)
					pTunnelImpl->_lastInSeqNum = pDataMsg->seqNum;
				pTunnelImpl->_flags |= TSF_SEND_NACK;
				tunnelStreamSetNeedsDispatch(pTunnelImpl);
			}


			break;
		}

		default:
		{
			rsslSetErrorInfo(pErrorInfo, RSSL_EIC_FAILURE, 
					RSSL_RET_FAILURE, __FILE__, __LINE__,
					"Received unhandled stream message class.");
			return RSSL_RET_FAILURE;
		}
	}

	return RSSL_RET_SUCCESS;
}

RsslRet tunnelStreamDispatch(RsslTunnelStream *pTunnel,
		RsslErrorInfo *pErrorInfo)
{
	RsslRet ret;
	TunnelStreamImpl *pTunnelImpl = (TunnelStreamImpl*)pTunnel;
	RsslReactorChannel	*pReactorChannel = pTunnelImpl->_manager->base._pReactorChannel;
	RsslUInt32 pendingBigBufferListCount, i;
	RsslReactorChannelImpl* pReactorChannelImpl = (RsslReactorChannelImpl*)pReactorChannel;
	RsslReactorImpl* pReactorImpl = pReactorChannelImpl->pParentReactor;

	if (pTunnelImpl->_interfaceError && pTunnelImpl->_state <= TSS_OPEN)
	{
		RsslState state;
		rsslClearState(&state);
		state.streamState = RSSL_STREAM_CLOSED_RECOVER;
		state.dataState = RSSL_DATA_SUSPECT;
		state.code = RSSL_SC_NONE;
		state.text.data = (char*)"A fatal error occurred in an interface call, closing stream.";
		state.text.length = 60;
		pTunnelImpl->_interfaceError = RSSL_FALSE;

		return tunnelStreamHandleState(pTunnelImpl, &state, NULL, NULL, NULL, RSSL_TRUE, pErrorInfo);
	}

    // if big buffer send in progress, process pending big buffers
	pendingBigBufferListCount = rsslQueueGetElementCount(&pTunnelImpl->_pendingBigBufferList);
	for (i = 0; i < pendingBigBufferListCount; i++)
	{
		RsslQueueLink *pQueueLink = rsslQueuePeekFront(&pTunnelImpl->_pendingBigBufferList);
		TunnelBufferImpl *pBigBuffer = RSSL_QUEUE_LINK_TO_OBJECT(TunnelBufferImpl, _tbpLink, pQueueLink);
		if (tunnelStreamEnqueueBigBuffer(pTunnel, pBigBuffer, pBigBuffer->_containerType, pErrorInfo) > RSSL_RET_SUCCESS)
		{
	    	// remove from pending big buffer list
			rsslQueueRemoveLink(&pTunnelImpl->_pendingBigBufferList, pQueueLink);
        		
	    	// release big buffer since now fully fragmented
			bigBufferPoolRelease(&pTunnelImpl->_bigBufferPool, &pBigBuffer->_poolBuffer);
		}
	}

	switch(pTunnelImpl->_state)
	{
		case TSS_SEND_REQUEST:
		{
			RsslBuffer *pBuffer;
			RsslEncodeIterator eIter;
			TunnelStreamRequest tRequestMsg;

			if (pTunnelImpl->_flags & TSF_ACTIVE)
			{
				/* Send a request to open the tunnel stream. */

				tunnelStreamRequestClear(&tRequestMsg);
				tRequestMsg.base.streamId = pTunnelImpl->base.streamId;
				tRequestMsg.base.domainType = pTunnelImpl->base.domainType;
				tRequestMsg.serviceId = pTunnelImpl->base.serviceId;
				tRequestMsg.name.data = pTunnelImpl->base.name;
				tRequestMsg.name.length = pTunnelImpl->_nameLength;

    			// decrement stream version if this is a retry
    			if (pTunnelImpl->_requestRetryCount > 0)
    			{
					--pTunnelImpl->_streamVersion;
    			}

				if (((RsslReactorChannelImpl*)pReactorChannel)->pWatchlist == NULL)
				{
					if ((pBuffer = tunnelManagerGetChannelBuffer(pTunnelImpl->_manager, &eIter,
									tunnelStreamRequestBufferSize(&tRequestMsg),
									RSSL_FALSE, pErrorInfo)) == NULL)
					{
						if (pErrorInfo->rsslError.rsslErrorId == RSSL_RET_BUFFER_NO_BUFFERS)
							return RSSL_RET_BUFFER_NO_BUFFERS;

						rsslSetErrorInfoLocation(pErrorInfo, __FILE__, __LINE__);
						return RSSL_RET_FAILURE;
					}

					if ((ret = tunnelStreamRequestEncode(&eIter, &tRequestMsg, &pTunnelImpl->base.classOfService, pTunnelImpl->_streamVersion, pErrorInfo))
							!= RSSL_RET_SUCCESS) 
						return ret;


					pBuffer->length = rsslGetEncodedBufferLength(&eIter);

					if ((ret = tunnelManagerSubmitChannelBuffer(pTunnelImpl->_manager, pBuffer,
									pErrorInfo))
							< RSSL_RET_SUCCESS)
					{
						rsslSetErrorInfoLocation(pErrorInfo, __FILE__, __LINE__);
						return RSSL_RET_CHANNEL_ERROR;
					}
				}
				else
				{
					/* Populate an RsslRequestMsg and submit that. It should
					 * go through the watchlist more efficiently. */
					RsslRequestMsg requestMsg;
					RsslBuffer cosBuffer;
					RsslEncodeIterator eIter;

					if (rsslHeapBufferResize(&pTunnelImpl->_memoryBuffer,
								tunnelStreamRequestBufferSize(&tRequestMsg) , RSSL_FALSE)
							!= RSSL_RET_SUCCESS)
					{
						rsslSetErrorInfo(pErrorInfo, RSSL_EIC_FAILURE, RSSL_RET_FAILURE, __FILE__, __LINE__, 
								"Memory allocation failed.");
						return RSSL_RET_FAILURE;
					}
					cosBuffer = pTunnelImpl->_memoryBuffer;

					rsslClearEncodeIterator(&eIter);
					rsslSetEncodeIteratorRWFVersion(&eIter, pReactorChannel->majorVersion,
							pReactorChannel->minorVersion);
					rsslSetEncodeIteratorBuffer(&eIter, &cosBuffer);

					tunnelStreamRequestSetRsslMsg(&tRequestMsg, &requestMsg, &pTunnelImpl->base.classOfService);
					if (rsslEncodeClassOfService(&eIter, &pTunnelImpl->base.classOfService, requestMsg.msgBase.msgKey.filter, RSSL_FALSE, pTunnelImpl->_streamVersion, pErrorInfo)
							!= RSSL_RET_SUCCESS)
						return RSSL_RET_FAILURE;

					requestMsg.msgBase.encDataBody = cosBuffer;
					requestMsg.msgBase.encDataBody.length = rsslGetEncodedBufferLength(&eIter);


					if ((ret = _tunnelStreamSubmitChannelMsg(pTunnelImpl, (RsslMsg*)&requestMsg,
									pErrorInfo))
							< RSSL_RET_SUCCESS)
					{
						rsslSetErrorInfoLocation(pErrorInfo, __FILE__, __LINE__);
						return RSSL_RET_CHANNEL_ERROR;
					}
				}

				pTunnelImpl->_state = TSS_WAIT_REFRESH;

				if (pTunnelImpl->_responseTimeout > RDM_QMSG_TC_INFINITE)
					_tunnelStreamSetResponseTimer(pTunnelImpl);

				tunnelStreamUnsetNeedsDispatch(pTunnelImpl);
			}
			else
			{
				/* Inactive tunnel stream destroy it. */
			}

			break;
		}

		case TSS_WAIT_REFRESH:
			tunnelStreamUnsetNeedsDispatch(pTunnelImpl);
			break;

		case TSS_SEND_REFRESH:
		{
			RsslBuffer *pBuffer;
			RsslEncodeIterator eIter;
			TunnelStreamRefresh tRefreshMsg;
			RsslState state;

			tunnelStreamRefreshClear(&tRefreshMsg);
			tRefreshMsg.base.streamId = pTunnelImpl->base.streamId;
			tRefreshMsg.base.domainType = pTunnelImpl->base.domainType;
			tRefreshMsg.flags = TS_RFMF_HAS_SERVICE_ID | TS_RFMF_HAS_NAME;
			tRefreshMsg.serviceId = pTunnelImpl->base.serviceId;
			tRefreshMsg.name.data = pTunnelImpl->base.name;
			tRefreshMsg.name.length = pTunnelImpl->_nameLength;


			if ((pBuffer = tunnelManagerGetChannelBuffer(pTunnelImpl->_manager, &eIter,
							tunnelStreamRefreshBufferSize(&tRefreshMsg),
							RSSL_FALSE, pErrorInfo)) == NULL)
			{
				if (pErrorInfo->rsslError.rsslErrorId == RSSL_RET_BUFFER_NO_BUFFERS)
					return RSSL_RET_BUFFER_NO_BUFFERS;

				rsslSetErrorInfoLocation(pErrorInfo, __FILE__, __LINE__);
				return RSSL_RET_FAILURE;
			}

			if ((ret = tunnelStreamRefreshEncode(&eIter, &tRefreshMsg, &pTunnelImpl->base.classOfService, pTunnelImpl->_streamVersion, pErrorInfo))
					!= RSSL_RET_SUCCESS) 
				return ret;

			pBuffer->length = rsslGetEncodedBufferLength(&eIter);

			if ((ret = tunnelManagerSubmitChannelBuffer(pTunnelImpl->_manager, pBuffer,
							pErrorInfo))
					< RSSL_RET_SUCCESS)
			{
				rsslSetErrorInfoLocation(pErrorInfo, __FILE__, __LINE__);
				return RSSL_RET_CHANNEL_ERROR;
			}

			if (isReactorDebugLevelEnabled(pReactorImpl, RSSL_RC_DEBUG_LEVEL_TUNNELSTREAM))
			{
				if (pReactorImpl->pReactorDebugInfo == NULL || pReactorChannelImpl->pChannelDebugInfo == NULL)
				{
					if (_initReactorAndChannelDebugInfo(pReactorImpl, pReactorChannelImpl, &pReactorChannelImpl->channelWorkerCerr) == RSSL_RET_SUCCESS)
					{
						return RSSL_RET_FAILURE;
					}
				}

				pReactorChannelImpl->pChannelDebugInfo->debugInfoState |= RSSL_RC_DEBUG_INFO_SUBMIT_TUNNEL_STREAM_RESP;

				_writeDebugInfo(pReactorImpl, "Reactor(0x%p), Reactor channel(0x%p) SUBMITED a tunnel stream open RESPONSE(stream ID=%d) on channel fd="RSSL_REACTOR_SOCKET_PRINT_TYPE".]\n",
					pReactorImpl, pReactorChannelImpl, pTunnelImpl->base.streamId, pReactorChannelImpl->reactorChannel.socketId);
			}

			pTunnelImpl->_state = TSS_OPEN;

			if ((ret = _tunnelStreamHandleEstablished(pTunnelImpl, pErrorInfo)) != RSSL_RET_SUCCESS)
				return RSSL_RET_FAILURE;

			rsslClearState(&state);
			state.streamState = RSSL_STREAM_OPEN;
			state.dataState = RSSL_DATA_OK;
			state.text.data = (char*)"Tunnel stream established.";
			state.text.length = 26;


			if ((ret = tunnelStreamHandleState(pTunnelImpl, &state,
							NULL, NULL, NULL, RSSL_FALSE, pErrorInfo)) != RSSL_RET_SUCCESS)
				return ret;

			if (isReactorDebugLevelEnabled(pReactorImpl, RSSL_RC_DEBUG_LEVEL_TUNNELSTREAM))
			{
				if (pReactorImpl->pReactorDebugInfo == NULL || pReactorChannelImpl->pChannelDebugInfo == NULL)
				{
					if (_initReactorAndChannelDebugInfo(pReactorImpl, pReactorChannelImpl, &pReactorChannelImpl->channelWorkerCerr) == RSSL_RET_SUCCESS)
					{
						return RSSL_RET_FAILURE;
					}
				}

				pReactorChannelImpl->pChannelDebugInfo->debugInfoState |= RSSL_RC_DEBUG_INFO_TUNNEL_STREAM_ESTABLISHED;

				_writeDebugInfo(pReactorImpl, "Reactor(0x%p), Reactor channel(0x%p), tunnel stream ESTABLISHED(stream ID=%d) on channel fd="RSSL_REACTOR_SOCKET_PRINT_TYPE".]\n",
					pReactorImpl, pReactorChannelImpl, pTunnelImpl->base.streamId, pReactorChannelImpl->reactorChannel.socketId);
			}

			tunnelStreamUnsetNeedsDispatch(pTunnelImpl);
			break;
		}

		case TSS_SEND_AUTH_LOGIN_REQUEST:
		{
			RsslReactorChannelImpl *pReactorChannelImpl 
				= (RsslReactorChannelImpl*)pTunnelImpl->base.pReactorChannel;

			RsslBuffer *pBuffer;
			RsslRet ret;
			RsslEncodeIterator eIter;

			assert (pTunnelImpl->_pAuthLoginRequest != NULL);

			if ((pBuffer = (RsslBuffer*)tunnelStreamGetBuffer(pTunnelImpl,
				(RsslUInt32)pTunnelImpl->base.classOfService.common.maxFragmentSize, RSSL_FALSE, RSSL_FALSE, pErrorInfo)) == NULL)
				return RSSL_RET_FAILURE;

			rsslClearEncodeIterator(&eIter);
			rsslSetEncodeIteratorRWFVersion(&eIter, pTunnel->classOfService.common.protocolMajorVersion,
					pTunnel->classOfService.common.protocolMinorVersion);
			rsslSetEncodeIteratorBuffer(&eIter, pBuffer);

			if ((ret = rsslEncodeRDMLoginMsg(&eIter, (RsslRDMLoginMsg*)pTunnelImpl->_pAuthLoginRequest, &pBuffer->length, pErrorInfo))
					!= RSSL_RET_SUCCESS) 
			{
				if (ret == RSSL_RET_BUFFER_TOO_SMALL)
					rsslSetErrorInfo(pErrorInfo, RSSL_EIC_FAILURE, ret,
							__FILE__, __LINE__, "Failed to encode login message for authentication -- the ClassOfService.common.maxMsgSize is too small.");
				else
					rsslSetErrorInfo(pErrorInfo, RSSL_EIC_FAILURE, ret,
							__FILE__, __LINE__, "Failed to encode login message for authentication -- %d.", ret);
				return RSSL_RET_FAILURE;
			}

			if ((ret = tunnelStreamEnqueueBuffer(pTunnel, pBuffer, RSSL_DT_MSG, pErrorInfo))
					!= RSSL_RET_SUCCESS)
				return ret;

			pTunnelImpl->_state = TSS_WAIT_AUTH_LOGIN_RESPONSE;

			if (pTunnelImpl->_responseTimeout != 0)
				_tunnelStreamSetResponseTimer(pTunnelImpl);

			break;
		}

		case TSS_WAIT_AUTH_LOGIN_RESPONSE:
		case TSS_OPEN:
		case TSS_WAIT_CLOSE:
		case TSS_WAIT_FIN:
		case TSS_WAIT_ACK_OF_FIN:
		{
			/* Send an ack message if needed. */
			if (rsslSeqNumCompare(pTunnelImpl->_lastInSeqNumAccepted, pTunnelImpl->_lastInAckedSeqNum) > 0
					|| (pTunnelImpl->_flags & TSF_SEND_NACK) && rsslSeqNumCompare(pTunnelImpl->_lastInSeqNum, pTunnelImpl->_lastInSeqNumAccepted) > 0)
			{
				TunnelStreamAck ackMsg;

				tunnelStreamAckClear(&ackMsg);
				ackMsg.base.streamId = pTunnelImpl->base.streamId;
				ackMsg.base.domainType = pTunnelImpl->base.domainType;
				ackMsg.seqNum = pTunnelImpl->_lastInSeqNumAccepted;
				ackMsg.recvWindow = (RsslInt32)pTunnelImpl->base.classOfService.flowControl.recvWindowSize;

				/* If a gap was previously detected, add a nak range. */
				if ((pTunnelImpl->_flags & TSF_SEND_NACK) && rsslSeqNumCompare(pTunnelImpl->_lastInSeqNum, pTunnelImpl->_lastInSeqNumAccepted) > 0)
				{
					AckRangeList ackRangeList;

					if (tunnelStreamDebugFlags & TS_DBG_ACKS)
						printf("<TunnelStreamDebug streamId:%d> Sending nack for gap %u-%u\n", pTunnelImpl->base.streamId, 
								pTunnelImpl->_lastInSeqNumAccepted + 1,
								pTunnelImpl->_lastInSeqNum);

					ackRangeList.count = 1;
					ackRangeList.rangeArray[0] = pTunnelImpl->_lastInSeqNumAccepted + 1;
					ackRangeList.rangeArray[1] = pTunnelImpl->_lastInSeqNum;
					if ((ret = _tunnelStreamSendAck(pTunnelImpl, &ackMsg, &ackRangeList, pErrorInfo)) != RSSL_RET_SUCCESS)
						return ret;


				}
				else
					if ((ret = _tunnelStreamSendAck(pTunnelImpl, &ackMsg, NULL, pErrorInfo)) != RSSL_RET_SUCCESS)
						return ret;

				/* Nak sent or no longer needed. */
				pTunnelImpl->_flags &= ~TSF_SEND_NACK;

				pTunnelImpl->_lastInAckedSeqNum = pTunnelImpl->_lastInSeqNumAccepted;
			}

			if ((ret = _tunnelStreamSendMessages(pTunnelImpl, pErrorInfo))< RSSL_RET_SUCCESS)
				return ret;

			if (rsslQueueGetElementCount(&pTunnelImpl->_tunnelBufferTransmitList) == 0)
				tunnelStreamUnsetNeedsDispatch(pTunnelImpl);

			break;
		}

		case TSS_SEND_ACK_OF_FIN:
		{
			TunnelStreamAck ackMsg;

			tunnelStreamAckClear(&ackMsg);
			ackMsg.base.streamId = pTunnelImpl->base.streamId;
			ackMsg.base.domainType = pTunnelImpl->base.domainType;
			ackMsg.seqNum = pTunnelImpl->_lastInSeqNumAccepted;
			ackMsg.recvWindow = (RsslInt32)pTunnelImpl->base.classOfService.flowControl.recvWindowSize;

			if ((ret = _tunnelStreamSendAck(pTunnelImpl, &ackMsg, NULL, pErrorInfo)) != RSSL_RET_SUCCESS)
				return ret;

			/* If we sent the first FIN, we send the close. */
			if (pTunnelImpl->_flags & TSF_STARTED_CLOSE)
			{
				pTunnelImpl->_state = TSS_SEND_CLOSE;
				tunnelStreamSetNeedsDispatch(pTunnelImpl);
			}
			else
			{
				pTunnelImpl->_state = TSS_WAIT_CLOSE;
				_tunnelStreamSetResponseTimerWithBackoff(pTunnelImpl);
				tunnelStreamUnsetNeedsDispatch(pTunnelImpl);
			}
			break;
		}

		case TSS_CLOSED:
			tunnelStreamUnsetNeedsDispatch(pTunnelImpl);
			return RSSL_RET_SUCCESS;

		case TSS_SEND_FIN:
		{
			TunnelBufferImpl *pBufferImpl;
			/* Put a FIN message on the transmit list. */
			if ((pBufferImpl = _tunnelStreamGetBufferImplObject(pTunnelImpl, pErrorInfo)) == NULL)
				return RSSL_RET_FAILURE;

			pBufferImpl->_bufferType = TS_BT_FIN;
			pBufferImpl->_flags |= TBF_IGNORE_FC;
			pTunnelImpl->_state = TSS_WAIT_ACK_OF_FIN;

			/* Add to transmission list and keep dispatching. */
			rsslQueueAddLinkToBack(&pTunnelImpl->_tunnelBufferTransmitList,
					&pBufferImpl->_tbpLink);

			/* Set timer for retransmission of this */
			_tunnelStreamSetResponseTimerWithBackoff(pTunnelImpl);
			tunnelStreamSetNextExpireTime(pTunnelImpl, pTunnelImpl->_responseExpireTime);
			break;
		}

		case TSS_SEND_CLOSE:
		{
			RsslBuffer *pBuffer;
			RsslEncodeIterator eIter;
			RsslState state;

			assert(pTunnelImpl->_manager->base._pReactorChannel);

			rsslClearState(&state);

			switch(((RsslReactorChannelImpl*)pTunnelImpl->_manager->base._pReactorChannel)->channelRole.base.roleType)
			{
				case RSSL_RC_RT_OMM_CONSUMER:
				case RSSL_RC_RT_OMM_NI_PROVIDER:
				{
					RsslCloseMsg closeMsg;

					rsslClearCloseMsg(&closeMsg);
					closeMsg.msgBase.streamId = pTunnelImpl->base.streamId;
					closeMsg.msgBase.domainType = pTunnelImpl->base.domainType;
					closeMsg.msgBase.containerType = RSSL_DT_NO_DATA;

					if (((RsslReactorChannelImpl*)pReactorChannel)->pWatchlist == NULL)
					{
						if ((pBuffer = tunnelManagerGetChannelBuffer(pTunnelImpl->_manager, &eIter,
										64, RSSL_FALSE, pErrorInfo)) == NULL)
						{
							if (pErrorInfo->rsslError.rsslErrorId == RSSL_RET_BUFFER_NO_BUFFERS)
								return RSSL_RET_BUFFER_NO_BUFFERS;

							rsslSetErrorInfoLocation(pErrorInfo, __FILE__, __LINE__);
							return RSSL_RET_FAILURE;
						}

						if ((ret = rsslEncodeMsg(&eIter, (RsslMsg*)&closeMsg))
								!= RSSL_RET_SUCCESS) 
						{
							rsslSetErrorInfo(pErrorInfo, RSSL_EIC_FAILURE, ret,
									__FILE__, __LINE__, "Failed to encode tunnel close message.");
							/* We cannot encode the close, return as channel error.
							 * This shouldn't generally happen.	*/
							return RSSL_RET_CHANNEL_ERROR;
						}

						pBuffer->length = rsslGetEncodedBufferLength(&eIter);

						if ((ret = tunnelManagerSubmitChannelBuffer(pTunnelImpl->_manager, pBuffer,
										pErrorInfo))
								< RSSL_RET_SUCCESS)
						{
							rsslSetErrorInfoLocation(pErrorInfo, __FILE__, __LINE__);
							return RSSL_RET_CHANNEL_ERROR;
						}
					}
					else
					{
						/* Populate an RsslCloseMsg and submit that. It should
						 * go through the watchlist more efficiently. */
						if ((ret = _tunnelStreamSubmitChannelMsg(pTunnelImpl, (RsslMsg*)&closeMsg,
										pErrorInfo))
								< RSSL_RET_SUCCESS)
						{
							rsslSetErrorInfoLocation(pErrorInfo, __FILE__, __LINE__);
							return RSSL_RET_CHANNEL_ERROR;
						}
					}

					state.streamState = RSSL_STREAM_CLOSED_RECOVER;
					break;
				}

				case RSSL_RC_RT_OMM_PROVIDER: 
				{
					RsslStatusMsg statusMsg;

					rsslClearStatusMsg(&statusMsg);
					statusMsg.msgBase.streamId = pTunnelImpl->base.streamId;
					statusMsg.msgBase.domainType = pTunnelImpl->base.domainType;
					statusMsg.msgBase.containerType = RSSL_DT_NO_DATA;

					rsslStatusMsgApplyPrivateStream(&statusMsg);
					rsslStatusMsgApplyHasState(&statusMsg);
					statusMsg.state.streamState = RSSL_STREAM_CLOSED_RECOVER;
					statusMsg.state.dataState = RSSL_DATA_OK;
					statusMsg.state.text.data = (char*)"Provider closing stream.";
					statusMsg.state.text.length = 24;

					if ((pBuffer = tunnelManagerGetChannelBuffer(pTunnelImpl->_manager, &eIter,
									64, RSSL_FALSE, pErrorInfo)) == NULL)
					{
						if (pErrorInfo->rsslError.rsslErrorId == RSSL_RET_BUFFER_NO_BUFFERS)
							return RSSL_RET_BUFFER_NO_BUFFERS;

						rsslSetErrorInfoLocation(pErrorInfo, __FILE__, __LINE__);
						return RSSL_RET_FAILURE;
					}

					if ((ret = rsslEncodeMsg(&eIter, (RsslMsg*)&statusMsg))
							!= RSSL_RET_SUCCESS) 
					{
						rsslSetErrorInfo(pErrorInfo, RSSL_EIC_FAILURE, ret,
								__FILE__, __LINE__, "Failed to encode tunnel closing status message.");
						/* We cannot encode the close, return as channel error.
						 * This shouldn't generally happen.	*/
						return RSSL_RET_CHANNEL_ERROR;
					}

					pBuffer->length = rsslGetEncodedBufferLength(&eIter);

					if ((ret = tunnelManagerSubmitChannelBuffer(pTunnelImpl->_manager, pBuffer,
									pErrorInfo))
							< RSSL_RET_SUCCESS)
					{
						rsslSetErrorInfoLocation(pErrorInfo, __FILE__, __LINE__);
						return RSSL_RET_CHANNEL_ERROR;
					}

					state.streamState = RSSL_STREAM_CLOSED;
					break;
				}

				default:
					assert(0);
					break;
			}

			state.dataState = RSSL_DATA_SUSPECT;
			state.text.data = (char*)"Sent final closing message for this tunnel stream.";
			state.text.length = 50;


			rsslHashTableRemoveLink(&pTunnelImpl->_manager->_streamIdToTunnelStreamTable, &pTunnelImpl->_managerHashLink);
			rsslQueueRemoveLink(&pTunnelImpl->_manager->_tunnelStreamsOpen, &pTunnelImpl->_managerOpenLink);
			pTunnelImpl->_state = TSS_CLOSED;

			if (pTunnelImpl->_flags & (TSF_ACTIVE | TSF_NEED_FINAL_STATUS_EVENT))
			{
				RsslUInt8 flags = pTunnelImpl->_flags;

				if (_tunnelStreamCallStatusEventCallback(pTunnelImpl, &state, NULL, NULL, pErrorInfo)
						!= RSSL_RET_SUCCESS)
					return ret;

				/* If tunnel stream is already closed but requested a final status event,
				 * destroy now. */
				if (!(flags & TSF_ACTIVE))
					tunnelStreamDestroy((RsslTunnelStream*)pTunnelImpl);
				return RSSL_RET_SUCCESS;
			}

			break;
		}

		default:
			rsslSetErrorInfo(pErrorInfo, RSSL_EIC_FAILURE, RSSL_RET_FAILURE, 
					__FILE__, __LINE__, "RsslTunnelStream is in an unknown state %d while dispatching.",
					pTunnelImpl->_state);
			return RSSL_RET_FAILURE;
	}
			
	return RSSL_RET_SUCCESS;
}

RsslRet tunnelStreamCallMsgCallback(TunnelStreamImpl *pTunnelImpl, RsslBuffer *pBuffer, RsslMsg *pRsslMsg,
		RsslUInt8 containerType, RsslErrorInfo *pErrorInfo)
{
	RsslReactorCallbackRet ret;
	RsslTunnelStreamMsgEvent msgEvent;

	tunnelStreamMsgEventClear(&msgEvent);
	msgEvent.pReactorChannel = pTunnelImpl->base.pReactorChannel;
	msgEvent.pRsslMsg = pRsslMsg;
	msgEvent.pRsslBuffer = pBuffer;
	msgEvent.containerType = containerType;

	ret = pTunnelImpl->_defaultMsgCallback(&pTunnelImpl->base, &msgEvent);

	if (ret != RSSL_RC_CRET_SUCCESS)
	{
		rsslSetErrorInfo(pErrorInfo, RSSL_EIC_FAILURE, RSSL_RET_FAILURE, 
				__FILE__, __LINE__, "Unexpected return code %d from tunnel stream defaultMsgCallback.", ret);
		return RSSL_RET_FAILURE;
	}

	return RSSL_RET_SUCCESS;

}

RsslRet tunnelStreamClose(RsslTunnelStream *pTunnel, RsslTunnelStreamCloseOptions *pOptions, RsslErrorInfo *pErrorInfo)
{
	TunnelStreamImpl *pTunnelImpl = (TunnelStreamImpl*)pTunnel;
	TunnelManagerImpl *pManagerImpl = (TunnelManagerImpl*)pTunnelImpl->_manager;

	tunnelStreamUnsetHasExpireTime(pTunnelImpl);

	if (!(pTunnelImpl->_flags & TSF_ACTIVE))
	{

		/* Be nice and let them do extra closes if they're waiting on the final status event. */
		if (pTunnelImpl->_flags & TSF_NEED_FINAL_STATUS_EVENT)
			return RSSL_RET_SUCCESS;

		rsslSetErrorInfo(pErrorInfo, RSSL_EIC_FAILURE, RSSL_RET_INVALID_ARGUMENT, 
				__FILE__, __LINE__, "This RsslTunnelStream is inactive. Closing it indicates invalid usage.");
		return RSSL_RET_INVALID_ARGUMENT;
	}
	pTunnelImpl->_flags &= ~TSF_ACTIVE;

	if (pOptions->finalStatusEvent)
		pTunnelImpl->_flags |= TSF_NEED_FINAL_STATUS_EVENT;

	switch(pTunnelImpl->_state)
	{
		case TSS_SEND_REQUEST:
		{
			/* Wait for freeEventCallback before cleaning up */
			tunnelStreamSetNeedsDispatch(pTunnelImpl);
			break;
		}

		case TSS_SEND_REFRESH:
		case TSS_WAIT_REFRESH:
		case TSS_SEND_AUTH_LOGIN_REQUEST:
		case TSS_WAIT_AUTH_LOGIN_RESPONSE:
		case TSS_OPEN:
		{
			/* Start the close process. */
			pTunnelImpl->_state = TSS_SEND_FIN;
			pTunnelImpl->_retransRetryCount = 0;
			pTunnelImpl->_flags |= TSF_STARTED_CLOSE;
			_tunnelStreamUnsetResponseTimer(pTunnelImpl);
			tunnelStreamSetNeedsDispatch(pTunnelImpl);
			pTunnelImpl->base.state.dataState = RSSL_DATA_SUSPECT;
			break;
		}

		case TSS_CLOSED:
		{
			/* Stream already closed. If final status event not requested, safe to destroy now. */
			if (!(pTunnelImpl->_flags & TSF_NEED_FINAL_STATUS_EVENT))
				tunnelStreamDestroy(pTunnel);
			break;
		}

		case TSS_SEND_FIN:
		case TSS_WAIT_FIN:
		case TSS_WAIT_ACK_OF_FIN:
		case TSS_WAIT_CLOSE:
		case TSS_SEND_CLOSE:
		case TSS_SEND_ACK_OF_FIN:
			/* Already closing, nothing to do. */
			break;

		case TSS_INACTIVE:
			rsslSetErrorInfo(pErrorInfo, RSSL_EIC_FAILURE, RSSL_RET_INVALID_ARGUMENT, 
					__FILE__, __LINE__, "This RsslTunnelStream is inactive. Closing it indicates invalid usage.");
			return RSSL_RET_INVALID_ARGUMENT;

		default:
			rsslSetErrorInfo(pErrorInfo, RSSL_EIC_FAILURE, RSSL_RET_FAILURE, 
					__FILE__, __LINE__, "RsslTunnelStream is in an unknown state %d while closing.", pTunnelImpl->_state);
			return RSSL_RET_FAILURE;
	}

	return RSSL_RET_SUCCESS;
}

RsslRet tunnelStreamEnqueueBuffer(RsslTunnelStream *pTunnel,
		RsslBuffer *pBuffer, RsslUInt8 containerType, RsslErrorInfo *pErrorInfo)
{
	TunnelStreamImpl*	pTunnelImpl = (TunnelStreamImpl*)pTunnel;
	TunnelBufferImpl*	pBufferImpl;

	pBufferImpl = (TunnelBufferImpl*)pBuffer;
	/* First message should be authentication request/response. Make sure it gets through. */
	if (!pTunnelImpl->_queuedFirstMsg &&
			pTunnelImpl->base.classOfService.authentication.type == RDM_COS_AU_OMM_LOGIN)
		pBufferImpl->_flags |= TBF_IGNORE_FC;

	pBufferImpl->_poolBuffer.buffer.length += (RsslUInt32)(pBufferImpl->_poolBuffer.buffer.data
		- pBufferImpl->_startPos);
	pBufferImpl->_poolBuffer.buffer.data = pBufferImpl->_startPos;

	/* Update containerType */
	if (pBuffer->length < TS_CONTAINER_TYPE_POS + 1)
	{
		rsslSetErrorInfo(pErrorInfo, RSSL_EIC_FAILURE, RSSL_RET_FAILURE, 
				__FILE__, __LINE__, "Unable to set containerType. This may not be a valid TunnelStream buffer.");
		return RSSL_RET_FAILURE;
	}

	if (containerType < RSSL_DT_CONTAINER_TYPE_MIN 
		|| containerType > RSSL_DT_CONTAINER_TYPE_MAX)
	{
		rsslSetErrorInfo(pErrorInfo, RSSL_EIC_FAILURE, RSSL_RET_FAILURE, 
				__FILE__, __LINE__, "Uknown containerType %d specified.", containerType);
		return RSSL_RET_FAILURE;
	}
	pBuffer->data[TS_CONTAINER_TYPE_POS] = containerType - RSSL_DT_CONTAINER_TYPE_MIN;

	bufferPoolTrimUnusedLength(&pTunnelImpl->_memoryBufferPool, &pBufferImpl->_poolBuffer);

	rsslQueueAddLinkToBack(&pTunnelImpl->_tunnelBufferTransmitList,
			&pBufferImpl->_tbpLink);

	tunnelStreamSetNeedsDispatch(pTunnelImpl);

	pTunnelImpl->_queuedFirstMsg = RSSL_TRUE;

	return RSSL_RET_SUCCESS;
}

RsslRet tunnelStreamEnqueueBigBuffer(RsslTunnelStream *pTunnel,
		TunnelBufferImpl *pBufferImpl, RsslUInt8 containerType, RsslErrorInfo *pErrorInfo)
{
	TunnelStreamImpl*	pTunnelImpl = (TunnelStreamImpl*)pTunnel;
	RsslQueueLink *pQueueLink;

	// fragment buffer
	RsslUInt32 totalMsgLength = pBufferImpl->_poolBuffer.buffer.length;
    RsslUInt32 bytesRemainingToSend = !pBufferImpl->_fragmentationInProgress ? totalMsgLength : pBufferImpl->_bytesRemainingToSend;
    RsslUInt32 fragmentNumber = !pBufferImpl->_fragmentationInProgress ? 1 : pBufferImpl->_lastFragmentId;
    RsslUInt16 messageId = !pBufferImpl->_fragmentationInProgress ? _tunnelStreamFragMsgId(pTunnelImpl) : pBufferImpl->_messageId;
    if (rsslQueueGetElementCount(&pTunnelImpl->_pendingBigBufferList) == 0 || pBufferImpl->_fragmentationInProgress) // process if no pending big buffers in list or fragmentation has already started
    {
	    while (bytesRemainingToSend > 0)
	    {
	    	RsslUInt32 lengthOfFragment = bytesRemainingToSend >= pTunnelImpl->base.classOfService.common.maxFragmentSize ? (RsslUInt32)pTunnelImpl->base.classOfService.common.maxFragmentSize : bytesRemainingToSend;
	    	RsslBool msgComplete = bytesRemainingToSend <= pTunnelImpl->base.classOfService.common.maxFragmentSize ? RSSL_TRUE	 : RSSL_FALSE;
	    	TunnelBufferImpl *pTunnelBuffer = _tunnelStreamGetBufferForFragmentation(pTunnelImpl, lengthOfFragment, totalMsgLength, fragmentNumber++, messageId, containerType, msgComplete, pErrorInfo);
	    	if (pTunnelBuffer != NULL)
	    	{
	    		// copy data to fragment buffer
				memcpy(pTunnelBuffer->_poolBuffer.buffer.data, &pBufferImpl->_poolBuffer.buffer.data[totalMsgLength - bytesRemainingToSend], lengthOfFragment);
	    			
	    		// adjust bytesRemainingToSend
	    		bytesRemainingToSend -= lengthOfFragment;
	    		
				// modify buffer for transmit
				pTunnelBuffer->_poolBuffer.buffer.length += (RsslUInt32)(pTunnelBuffer->_poolBuffer.buffer.data - pTunnelBuffer->_startPos);
				pTunnelBuffer->_poolBuffer.buffer.data = pTunnelBuffer->_startPos;
				bufferPoolTrimUnusedLength(&pTunnelImpl->_memoryBufferPool, &pTunnelBuffer->_poolBuffer);

				// queue for transmit
				rsslQueueAddLinkToBack(&pTunnelImpl->_tunnelBufferTransmitList, &pTunnelBuffer->_tbpLink);
				tunnelStreamSetNeedsDispatch(pTunnelImpl);
	    	}
	    	else // cannot fully fragment buffer
	    	{
	    		// save progress in big buffer and add to pending big buffer list if not already there
	    		saveWriteProgress(pBufferImpl, totalMsgLength, bytesRemainingToSend, fragmentNumber - 1, messageId, containerType);
				pQueueLink = rsslQueuePeekFront(&pTunnelImpl->_pendingBigBufferList); // assumes big buffer to fragment is always at top of list
				if (pQueueLink == NULL || pBufferImpl != RSSL_QUEUE_LINK_TO_OBJECT(TunnelBufferImpl, _tbpLink, pQueueLink))
				{
					rsslQueueAddLinkToBack(&pTunnelImpl->_pendingBigBufferList, &pBufferImpl->_tbpLink);
				}
	    		break;
	    	}
	    }

	    if (bytesRemainingToSend == 0)
	    {
			// return 1 to indicate finished with buffer
			return 1;
	    }
    }
    else // list already has pending big buffers
    {
		// save progress in big buffer and add to pending big buffer list if not already there
		saveWriteProgress(pBufferImpl, totalMsgLength, bytesRemainingToSend, fragmentNumber, messageId, containerType);
		pQueueLink = rsslQueuePeekFront(&pTunnelImpl->_pendingBigBufferList); // assumes big buffer to fragment is always at top of list
		if (pQueueLink == NULL || pBufferImpl != RSSL_QUEUE_LINK_TO_OBJECT(TunnelBufferImpl, _tbpLink, pQueueLink))
		{
			rsslQueueAddLinkToBack(&pTunnelImpl->_pendingBigBufferList, &pBufferImpl->_tbpLink);
		}
    }

	return RSSL_RET_SUCCESS;
}

void tunnelStreamSetNextExpireTime(TunnelStreamImpl *pTunnelImpl, RsslInt64 expireTime)
{
	RsslQueueLink *pLink;
	assert(expireTime > RDM_QMSG_TC_IMMEDIATE);

	if (pTunnelImpl->_nextExpireTime != RDM_QMSG_TC_INFINITE)
	{
		/* Tunnel is already in timeout list. */
		if (expireTime >= pTunnelImpl->_nextExpireTime)
			return; /* No change in next timeout. */

		/* Check if we might need to move this stream up. */
		pLink = rsslQueuePeekPrev(&pTunnelImpl->_manager->_tunnelStreamTimeoutList,
				&pTunnelImpl->_timeoutLink);

		rsslQueueRemoveLink(&pTunnelImpl->_manager->_tunnelStreamTimeoutList, &pTunnelImpl->_timeoutLink);
	}
	else /* Tunnel is not currently in timeout list. */
		pLink = rsslQueuePeekBack(&pTunnelImpl->_manager->_tunnelStreamTimeoutList);

	pTunnelImpl->_nextExpireTime = expireTime;
	tunnelManagerSetNextDispatchTime(pTunnelImpl->_manager, pTunnelImpl->_nextExpireTime);

	/* Find a stream that expires sooner, and put this stream behind it. */
	while (pLink != NULL)
	{
		TunnelStreamImpl *pTimeoutTunnelImpl = RSSL_QUEUE_LINK_TO_OBJECT(TunnelStreamImpl,
				_timeoutLink, pLink);

		if (pTimeoutTunnelImpl->_nextExpireTime <= pTunnelImpl->_nextExpireTime)
		{
			rsslQueueInsertAfter(&pTunnelImpl->_manager->_tunnelStreamTimeoutList,
					&pTimeoutTunnelImpl->_timeoutLink, &pTunnelImpl->_timeoutLink);
			return;
		}
		pLink = rsslQueuePeekPrev(&pTunnelImpl->_manager->_tunnelStreamTimeoutList, pLink);
	}

	rsslQueueAddLinkToFront(&pTunnelImpl->_manager->_tunnelStreamTimeoutList, 
			&pTunnelImpl->_timeoutLink);
}

RsslRet tunnelStreamProcessTimer(TunnelStreamImpl *pTunnelImpl, RsslInt64 currentTime,
		RsslErrorInfo *pErrorInfo)
{
	RsslQueueLink *pLink;
	RsslInt64 nextExpireTime;

	if ( pTunnelImpl->_responseExpireTime > RDM_QMSG_TC_INFINITE)
	{
		if (pTunnelImpl->_responseExpireTime <= tunnelStreamGetCurrentTimeMs(pTunnelImpl))
		{
			RsslState state;

			_tunnelStreamUnsetResponseTimer(pTunnelImpl);

			switch(pTunnelImpl->_state)
			{
			case TSS_WAIT_REFRESH:
				rsslClearState(&state);
				state.streamState = RSSL_STREAM_CLOSED_RECOVER;
				state.dataState = RSSL_DATA_SUSPECT;
				state.code = RSSL_SC_TIMEOUT;
				state.text.data = (char*)"Timed out waiting for provider response.";
				state.text.length = 40;
				return tunnelStreamHandleState(pTunnelImpl, &state, NULL, NULL, NULL, RSSL_TRUE, pErrorInfo);

			case TSS_WAIT_AUTH_LOGIN_RESPONSE:
				rsslClearState(&state);
				state.streamState = RSSL_STREAM_CLOSED_RECOVER;
				state.dataState = RSSL_DATA_SUSPECT;
				state.code = RSSL_SC_TIMEOUT;
				state.text.data = (char*)"Timed out waiting for provider authentication response.";
				state.text.length = 55;
				return tunnelStreamHandleState(pTunnelImpl, &state, NULL, NULL, NULL, RSSL_TRUE, pErrorInfo);

			case TSS_WAIT_FIN:
			case TSS_WAIT_ACK_OF_FIN:
				if (pTunnelImpl->_retransRetryCount <= TS_RETRANSMIT_MAX_ATTEMPTS)
				{
					/* If the FIN is sent but unacked, retransmit it. */
					for(pLink = rsslQueuePeekBack(&pTunnelImpl->_tunnelBufferWaitAckList);
							pLink != NULL;
							pLink = rsslQueuePeekPrev(&pTunnelImpl->_tunnelBufferWaitAckList, pLink))
					{
						TunnelBufferImpl *pBufferImpl = RSSL_QUEUE_LINK_TO_OBJECT(TunnelBufferImpl, _tbpLink, pLink);
						if (pBufferImpl->_bufferType == TS_BT_FIN)
						{
							rsslQueueRemoveLink(&pTunnelImpl->_tunnelBufferWaitAckList, pLink);
							rsslQueueAddLinkToBack(&pTunnelImpl->_tunnelBufferTransmitList, pLink);
							break;
						}
					}

					/* Set timer for retransmission of this (even it wasn't sent, e.g. due to data messages before it that couldn't be sent) */
					_tunnelStreamSetResponseTimerWithBackoff(pTunnelImpl);
					tunnelStreamSetNextExpireTime(pTunnelImpl, pTunnelImpl->_responseExpireTime);
				}
				else
					pTunnelImpl->_state = TSS_SEND_CLOSE; /* Give up and send a close message. */

				tunnelStreamSetNeedsDispatch(pTunnelImpl);
				break;


			default:
				return RSSL_RET_SUCCESS;
			}
		}
	}

	for(pLink = rsslQueueStart(&pTunnelImpl->_tunnelBufferTimeoutList); pLink != NULL;
		   pLink = rsslQueueForth(&pTunnelImpl->_tunnelBufferTimeoutList))
	{
		TunnelBufferImpl *pBufferImpl = RSSL_QUEUE_LINK_TO_OBJECT(TunnelBufferImpl, _timeoutLink, pLink);
		RsslRet ret;

		if (pBufferImpl->_expireTime >= currentTime)
			break;

		assert(pBufferImpl->_substream);

		pBufferImpl->_poolBuffer.buffer.length -= (RsslUInt32)(pBufferImpl->_dataStartPos - pBufferImpl->_startPos);
		pBufferImpl->_poolBuffer.buffer.data = pBufferImpl->_dataStartPos;

		ret = tunnelSubstreamExpireBuffer(pBufferImpl->_substream, (RsslBuffer*)pBufferImpl, pBufferImpl->_persistentMsg,
			RDM_QMSG_UC_EXPIRED, pErrorInfo);
		rsslQueueRemoveLink(&pTunnelImpl->_tunnelBufferTransmitList, &pBufferImpl->_tbpLink);
		rsslQueueRemoveLink(&pTunnelImpl->_tunnelBufferTimeoutList, &pBufferImpl->_timeoutLink);
		tunnelStreamReleaseBuffer(pTunnelImpl, pBufferImpl);

		if (ret != RSSL_RET_SUCCESS)
			return RSSL_RET_FAILURE;
	}

	nextExpireTime = RDM_QMSG_TC_INFINITE;
	if ((pLink = rsslQueuePeekFront(&pTunnelImpl->_tunnelBufferTimeoutList)) != NULL)
	{
		TunnelBufferImpl *pBufferImpl = RSSL_QUEUE_LINK_TO_OBJECT(TunnelBufferImpl, _timeoutLink, pLink);
		nextExpireTime = pBufferImpl->_expireTime;
	}

	if (pTunnelImpl->_responseExpireTime > RDM_QMSG_TC_INFINITE)
	{
		if (nextExpireTime == RDM_QMSG_TC_INFINITE)
			nextExpireTime = pTunnelImpl->_responseExpireTime;
		else if (pTunnelImpl->_responseExpireTime < nextExpireTime)
			nextExpireTime = pTunnelImpl->_nextExpireTime;
	}

	if (nextExpireTime != RDM_QMSG_TC_INFINITE)
		tunnelStreamSetNextExpireTime(pTunnelImpl, nextExpireTime);
	else
		tunnelStreamUnsetHasExpireTime(pTunnelImpl);

	return RSSL_RET_SUCCESS;
}

static RsslRet _tunnelStreamSubmitChannelMsg(TunnelStreamImpl *pTunnelImpl,
		RsslMsg *pRsslMsg, RsslErrorInfo *pErrorInfo)
{
	RsslRet				ret;
	RsslReactorChannelImpl	*pReactorChannelImpl = (RsslReactorChannelImpl*)pTunnelImpl->_manager->base._pReactorChannel;
	RsslWatchlistProcessMsgOptions processOpts;

	/* Meant for submitting messages through the watchlist. */
	assert(pReactorChannelImpl->pWatchlist != NULL);

	rsslWatchlistClearProcessMsgOptions(&processOpts);
	processOpts.pChannel = pReactorChannelImpl->reactorChannel.pRsslChannel;
	processOpts.majorVersion = pReactorChannelImpl->reactorChannel.majorVersion;
	processOpts.minorVersion = pReactorChannelImpl->reactorChannel.minorVersion;
	processOpts.pRsslMsg = pRsslMsg;

	ret = rsslWatchlistSubmitMsg(pReactorChannelImpl->pWatchlist, &processOpts, pErrorInfo);

	if (ret < RSSL_RET_SUCCESS)
		return ret;

	return RSSL_RET_SUCCESS;
}

static RsslRet _tunnelStreamCheckReceivedCos(RsslClassOfService *pLocalCos, RsslClassOfService *pRemoteCos, RsslUInt streamVersion, RsslErrorInfo *pErrorInfo)
{
	if (streamVersion > COS_CURRENT_STREAM_VERSION)
	{
		rsslSetErrorInfo(pErrorInfo, RSSL_EIC_FAILURE, RSSL_RET_FAILURE, 
				__FILE__, __LINE__, "Negotiated tunnel stream version %llu is unknown", streamVersion);
		return RSSL_RET_FAILURE;
	}

	switch (pRemoteCos->dataIntegrity.type)
	{
		case RDM_COS_DI_BEST_EFFORT:
			rsslSetErrorInfo(pErrorInfo, RSSL_EIC_FAILURE, RSSL_RET_FAILURE, 
					__FILE__, __LINE__, "Negotiated ClassOfService.dataIntegrity.type of RDM_COS_DI_BEST_EFFORT is not supported in this version.");
			return RSSL_RET_FAILURE;

		default:
			break;
	}

	switch (pRemoteCos->guarantee.type)
	{
		case RDM_COS_GU_PERSISTENT_QUEUE:
			if (pRemoteCos->common.protocolType != RSSL_RWF_PROTOCOL_TYPE)
			{
				rsslSetErrorInfo(pErrorInfo, RSSL_EIC_FAILURE, RSSL_RET_FAILURE, 
						__FILE__, __LINE__, "Negotiated ClassOfService.guarantee.type of RDM_COS_GU_PERSISTENT_QUEUE requires ClassOfService.common.protocolType of RSSL_RWF_PROTOCOL_TYPE.");
				return RSSL_RET_FAILURE;
			}

		default:
			break;
	}

	return RSSL_RET_SUCCESS;
}

RsslRet tunnelStreamHandleState(TunnelStreamImpl *pTunnelImpl, RsslState *pState, RsslMsg *pRsslMsg, 
		TunnelStreamMsg *pTunnelMsg, RsslTunnelStreamAuthInfo *pAuthInfo, RsslBool isInternalClose, RsslErrorInfo *pErrorInfo)
{
	RsslRet ret;
	TunnelManagerImpl *pManagerImpl = pTunnelImpl->_manager;
	RsslReactorChannel* pReactorChannel = pTunnelImpl->base.pReactorChannel;
	RsslReactorChannelImpl* pReactorChannelImpl = (RsslReactorChannelImpl*)pReactorChannel;
	RsslReactorImpl* pReactorImpl = pReactorChannelImpl->pParentReactor;

	if (pState != NULL)
	{
		pTunnelImpl->base.state.streamState = pState->streamState;
		pTunnelImpl->base.state.dataState = pState->dataState;

		if (pState->streamState != RSSL_STREAM_OPEN)
		{
			RsslQueueLink *pLink;
			RsslReactorChannelImpl *pReactorChannelImpl 
				= (RsslReactorChannelImpl*)pTunnelImpl->base.pReactorChannel;

			if (pRsslMsg && pRsslMsg->msgBase.containerType == RSSL_DT_FILTER_LIST)
			{
				RsslClassOfService cos;
				RsslDecodeIterator dIter;
				rsslClearDecodeIterator(&dIter);
				rsslSetDecodeIteratorRWFVersion(&dIter, pReactorChannelImpl->reactorChannel.majorVersion, 
						pReactorChannelImpl->reactorChannel.minorVersion);
				rsslSetDecodeIteratorBuffer(&dIter, &pRsslMsg->msgBase.encDataBody);

				if ((ret = rsslDecodeClassOfService(&dIter, &cos, NULL, pErrorInfo)) != RSSL_RET_SUCCESS)
					return RSSL_RET_FAILURE;

				pTunnelImpl->base.classOfService = cos;
			}

			/* Expire any messages with an immediate timeout code. */
			while ((pLink = rsslQueuePeekFront(&pTunnelImpl->_tunnelBufferImmediateList)) != NULL)
			{
				TunnelBufferImpl *pBufferImpl = RSSL_QUEUE_LINK_TO_OBJECT(TunnelBufferImpl,
						_timeoutLink, pLink);

				pBufferImpl->_poolBuffer.buffer.length -= (RsslUInt32)(pBufferImpl->_dataStartPos - pBufferImpl->_startPos);
				pBufferImpl->_poolBuffer.buffer.data = pBufferImpl->_dataStartPos;

				if (tunnelSubstreamExpireBuffer(pBufferImpl->_substream, 
							(RsslBuffer*)pBufferImpl, pBufferImpl->_persistentMsg,
							RDM_QMSG_UC_EXPIRED, pErrorInfo) != RSSL_RET_SUCCESS)
					return RSSL_RET_FAILURE;

				rsslQueueRemoveLink(&pTunnelImpl->_tunnelBufferImmediateList, pLink);
				tunnelStreamReleaseBuffer(pTunnelImpl, pBufferImpl);
			}


			switch(pTunnelImpl->_state)
			{
				case TSS_CLOSED:
					/* Stream not open, ignore it. */
					return RSSL_RET_SUCCESS;

				case TSS_SEND_REQUEST:
					/* Haven't sent request, so ignore external messages. */
					if (!isInternalClose)
						break;
				case TSS_WAIT_REFRESH:
				case TSS_SEND_AUTH_LOGIN_REQUEST:
				case TSS_WAIT_AUTH_LOGIN_RESPONSE:
				case TSS_OPEN:
				case TSS_SEND_CLOSE:
				case TSS_SEND_FIN:
				case TSS_WAIT_FIN:
				case TSS_WAIT_ACK_OF_FIN:
				case TSS_SEND_ACK_OF_FIN:
				case TSS_WAIT_CLOSE:
				case TSS_SEND_REFRESH:

					if (isReactorDebugLevelEnabled(pReactorImpl, RSSL_RC_DEBUG_LEVEL_TUNNELSTREAM))
					{
						if (pReactorImpl->pReactorDebugInfo == NULL || pReactorChannelImpl->pChannelDebugInfo == NULL)
						{
							if (_initReactorAndChannelDebugInfo(pReactorImpl, pReactorChannelImpl, &pReactorChannelImpl->channelWorkerCerr) == RSSL_RET_SUCCESS)
							{
								return RSSL_RET_FAILURE;
							}
						}

						pReactorChannelImpl->pChannelDebugInfo->debugInfoState |= RSSL_RC_DEBUG_INFO_HANDLE_TUNNEL_CLOSE;

						_writeDebugInfo(pReactorImpl, "Reactor(0x%p), Reactor channel(0x%p) HANDLES tunnel stream CLOSE(stream ID=%d) on channel fd="RSSL_REACTOR_SOCKET_PRINT_TYPE".]\n",
							pReactorImpl, pReactorChannelImpl, pTunnelImpl->base.streamId, pReactorChannelImpl->reactorChannel.socketId);
					}

					if (pAuthInfo)
					{
						RsslState state = *pState;


						/* If pAuthInfo present, we're closing the stream because the auth login
						 * closed us, so try to close gracefully. */
						pTunnelImpl->base.state.streamState = state.streamState = RSSL_STREAM_OPEN;
						pTunnelImpl->base.state.dataState = state.dataState = RSSL_DATA_SUSPECT;
						if (pTunnelImpl->_state < TSS_SEND_FIN)
						{
							pTunnelImpl->_retransRetryCount = 0;
							pTunnelImpl->_state = TSS_SEND_FIN;
							pTunnelImpl->_flags |= TSF_STARTED_CLOSE;
						}

						tunnelStreamSetNeedsDispatch(pTunnelImpl);

						if (_tunnelStreamCallStatusEventCallback(pTunnelImpl, &state, pRsslMsg, pAuthInfo, pErrorInfo)
							!= RSSL_RET_SUCCESS)
							return ret;
					}
					else if (!isInternalClose)
					{
						rsslHashTableRemoveLink(&pManagerImpl->_streamIdToTunnelStreamTable, &pTunnelImpl->_managerHashLink);
						rsslQueueRemoveLink(&pManagerImpl->_tunnelStreamsOpen, &pTunnelImpl->_managerOpenLink);
						pTunnelImpl->_state = TSS_CLOSED;

						if (_tunnelStreamCallStatusEventCallback(pTunnelImpl, pState, pRsslMsg, pAuthInfo, pErrorInfo)
							!= RSSL_RET_SUCCESS)
							return ret;
					}
					else
					{
						 /* This is due to an internal error, go straight to closing the stream. */
						/* If consumer and request not yet sent, just send CLOSED_RECOVER status event. */
						RsslState state = *pState;
						if (pTunnelImpl->_state == TSS_SEND_REQUEST)
						{
							pTunnelImpl->base.state.streamState = state.streamState = RSSL_STREAM_CLOSED_RECOVER;
							pTunnelImpl->base.state.dataState = state.dataState = RSSL_DATA_SUSPECT;

							rsslHashTableRemoveLink(&pManagerImpl->_streamIdToTunnelStreamTable, &pTunnelImpl->_managerHashLink);
							rsslQueueRemoveLink(&pManagerImpl->_tunnelStreamsOpen, &pTunnelImpl->_managerOpenLink);
							pTunnelImpl->_state = TSS_CLOSED;

							if (_tunnelStreamCallStatusEventCallback(pTunnelImpl, &state, pRsslMsg, pAuthInfo, pErrorInfo)
								!= RSSL_RET_SUCCESS)
								return ret;
						}
						else
						{
						pTunnelImpl->base.state.streamState = state.streamState = RSSL_STREAM_OPEN;
						pTunnelImpl->base.state.dataState = state.dataState = RSSL_DATA_SUSPECT;

						pTunnelImpl->_state = TSS_SEND_CLOSE;

						tunnelStreamSetNeedsDispatch(pTunnelImpl);

						if (_tunnelStreamCallStatusEventCallback(pTunnelImpl, &state, pRsslMsg, pAuthInfo, pErrorInfo)
							!= RSSL_RET_SUCCESS)
							return ret;
					}
					}

					return RSSL_RET_SUCCESS;

				default:
					rsslSetErrorInfo(pErrorInfo, RSSL_EIC_FAILURE, RSSL_RET_FAILURE, 
							__FILE__, __LINE__, "RsslTunnelStream is in an unknown state %d while handling closed state.", pTunnelImpl->_state);
					return RSSL_RET_FAILURE;
			}

		}
		else
		{
			switch(pTunnelImpl->_state)
			{
				case TSS_SEND_REQUEST:
				case TSS_CLOSED:
					/* Ignore message. */
					return RSSL_RET_SUCCESS;

				case TSS_WAIT_REFRESH:

					if (pState->dataState == RSSL_DATA_OK
						&& pTunnelMsg && pTunnelMsg->base.opcode == TS_MC_REFRESH)
					{
						RsslReactorChannelImpl *pReactorChannelImpl 
							= (RsslReactorChannelImpl*)pTunnelImpl->base.pReactorChannel;
						
						TunnelStreamRefresh *pRefresh = &pTunnelMsg->refreshHeader;
						RsslDecodeIterator dIter;
						RsslInt32 sendWindowSize = TS_DEFAULT_BIDRECTIONAL_WINDOW_SIZE;

						if (pRefresh->containerType == RSSL_DT_FILTER_LIST)
						{
							RsslInt32 recvWindowSize = (RsslInt32)pTunnelImpl->base.classOfService.flowControl.recvWindowSize;
							RsslClassOfService cos;
							RsslUInt streamVersion;
							rsslClearDecodeIterator(&dIter);
							rsslSetDecodeIteratorRWFVersion(&dIter, pReactorChannelImpl->reactorChannel.majorVersion, 
								pReactorChannelImpl->reactorChannel.minorVersion);
							rsslSetDecodeIteratorBuffer(&dIter, &pRefresh->encDataBody);

							if ((ret = rsslDecodeClassOfService(&dIter, &cos, &streamVersion, pErrorInfo)) != RSSL_RET_SUCCESS)
								return RSSL_RET_FAILURE;

							/* Check for class of service mismatches. */
							if (_tunnelStreamCheckReceivedCos(&pTunnelImpl->base.classOfService, &cos, streamVersion, pErrorInfo) != RSSL_RET_SUCCESS)
								return tunnelStreamHandleError(pTunnelImpl, pErrorInfo);

							/* Set stream version to that received */
							pTunnelImpl->_streamVersion = streamVersion;

							/* Use the negotiated CoS (but remember our own recvWindow). */
							pTunnelImpl->base.classOfService = cos;
							pTunnelImpl->base.classOfService.flowControl.recvWindowSize = recvWindowSize;
							sendWindowSize = (RsslInt32)cos.flowControl.recvWindowSize;
						}

						if (pTunnelImpl->base.classOfService.flowControl.recvWindowSize != RDM_COS_FC_NONE)
							if (sendWindowSize < pTunnelImpl->base.classOfService.common.maxFragmentSize)
								pTunnelImpl->base.classOfService.flowControl.sendWindowSize = pTunnelImpl->base.classOfService.common.maxFragmentSize;
							else
								pTunnelImpl->base.classOfService.flowControl.sendWindowSize = sendWindowSize;
						else
							pTunnelImpl->base.classOfService.flowControl.sendWindowSize = TS_USE_DEFAULT_RECV_WINDOW_SIZE;

						if ((ret = _tunnelStreamHandleEstablished(pTunnelImpl, pErrorInfo)) != RSSL_RET_SUCCESS)
							return RSSL_RET_FAILURE;

						if (pTunnelImpl->base.classOfService.authentication.type == RDM_COS_AU_OMM_LOGIN)
						{
							if (pTunnelImpl->_pAuthLoginRequest == NULL)
							{
								
								/* Check if we can get it from the consumer role */
								if (pReactorChannelImpl->channelRole.base.roleType
										&& pReactorChannelImpl->channelRole.ommConsumerRole.pLoginRequest != NULL)
								{
									/* Not present on RsslTunnelStreamOpenOptions; so use the one that was used to login. */
									pTunnelImpl->_pAuthLoginRequest = 
										pReactorChannelImpl->channelRole.ommConsumerRole.pLoginRequest;
									pTunnelImpl->_authLoginStreamId = 
										pReactorChannelImpl->channelRole.ommConsumerRole.pLoginRequest->rdmMsgBase.streamId;
								}
								else
								{
									rsslSetErrorInfo(pErrorInfo, RSSL_EIC_FAILURE, RSSL_RET_FAILURE, 
											__FILE__, __LINE__, "Provider requires authentication but consumer specified no login request.", pTunnelImpl->_state);
									return tunnelStreamHandleError(pTunnelImpl, pErrorInfo);
								}
							}

							/* Wait for login response before sending status */
							pTunnelImpl->_state = TSS_SEND_AUTH_LOGIN_REQUEST;
							tunnelStreamSetNeedsDispatch(pTunnelImpl);
							return RSSL_RET_SUCCESS;
						}
						else
						{
							_tunnelStreamUnsetResponseTimer(pTunnelImpl);
							pTunnelImpl->_state = TSS_OPEN;
						}
					}
					break;

				case TSS_SEND_AUTH_LOGIN_REQUEST:
				{
					/* Wait for login response before sending status */
					return RSSL_RET_SUCCESS;
				}

				case TSS_WAIT_AUTH_LOGIN_RESPONSE:
				case TSS_OPEN:
					if ( pAuthInfo != NULL /* State comes from a login response */
							&& pTunnelImpl->_state == TSS_WAIT_AUTH_LOGIN_RESPONSE)

					if (pState->dataState == RSSL_DATA_OK)
					{
						pTunnelImpl->_state = TSS_OPEN;
						_tunnelStreamUnsetResponseTimer(pTunnelImpl);
					}
					break;

				case TSS_SEND_CLOSE:
				case TSS_SEND_FIN:
				case TSS_WAIT_FIN:
				case TSS_SEND_ACK_OF_FIN:
				case TSS_WAIT_ACK_OF_FIN:
				case TSS_WAIT_CLOSE:

					/* Closing stream, ignore. */
					return RSSL_RET_SUCCESS;

				default:
					rsslSetErrorInfo(pErrorInfo, RSSL_EIC_FAILURE, RSSL_RET_FAILURE, 
							__FILE__, __LINE__, "RsslTunnelStream is in an unknown state %d while handling open state.", pTunnelImpl->_state);
					return RSSL_RET_FAILURE;
			}

			if (_tunnelStreamCallStatusEventCallback(pTunnelImpl, pState, pRsslMsg, pAuthInfo, pErrorInfo)
					!= RSSL_RET_SUCCESS)
				return ret;
		}

	}

	return RSSL_RET_SUCCESS;
}

RsslRet tunnelStreamHandleError(TunnelStreamImpl *pTunnelImpl, RsslErrorInfo *pErrorInfo)
{
	RsslState state;

	rsslClearState(&state);
	state.streamState = RSSL_STREAM_CLOSED_RECOVER;
	state.dataState = RSSL_DATA_SUSPECT;
	state.code = RSSL_SC_NONE;
	state.text.data = pErrorInfo->rsslError.text;
	state.text.length = (RsslUInt32)strlen(pErrorInfo->rsslError.text);

	return tunnelStreamHandleState(pTunnelImpl, &state, NULL, NULL, NULL, RSSL_TRUE, pErrorInfo);
}

static RsslRet _tunnelStreamSendAck(TunnelStreamImpl *pTunnelImpl, TunnelStreamAck *pAckMsg,
		AckRangeList *nakRangeList, RsslErrorInfo *pErrorInfo)
{
	RsslEncodeIterator eIter;
	RsslBuffer *pBuffer;
	RsslRet ret;

	/* Event if the watchlist is enabled, it should be able to send this
	 * message through as a buffer. */
	if ((pBuffer = tunnelManagerGetChannelBuffer(pTunnelImpl->_manager, &eIter,
					tunnelStreamAckBufferSize(pAckMsg, NULL, NULL),
					RSSL_FALSE, pErrorInfo)) == NULL)
	{
		if (pErrorInfo->rsslError.rsslErrorId == RSSL_RET_BUFFER_NO_BUFFERS)
			return RSSL_RET_BUFFER_NO_BUFFERS;

		rsslSetErrorInfoLocation(pErrorInfo, __FILE__, __LINE__);
		return RSSL_RET_FAILURE;
	}

	if ((ret = tunnelStreamAckEncode(&eIter, pAckMsg, NULL, nakRangeList))
			!= RSSL_RET_SUCCESS) 
	{
		rsslSetErrorInfo(pErrorInfo, RSSL_EIC_FAILURE, ret,
				__FILE__, __LINE__, "Failed to encode ack message.");
		return RSSL_RET_FAILURE;
	}

	pBuffer->length = rsslGetEncodedBufferLength(&eIter);

	if ((ret = tunnelManagerSubmitChannelBuffer(pTunnelImpl->_manager, pBuffer,
					pErrorInfo))
			< RSSL_RET_SUCCESS)
	{
		rsslSetErrorInfoLocation(pErrorInfo, __FILE__, __LINE__);
		return RSSL_RET_CHANNEL_ERROR;
	}

	/* If the FIN was received, this ack should include it. */
	if (pTunnelImpl->_flags & TSF_FIN_RECEIVED
		&& pAckMsg->seqNum == pTunnelImpl->_lastInSeqNum)
		pTunnelImpl->_flags |= TSF_SENT_ACK_OF_FIN;


	return RSSL_RET_SUCCESS;
}

void tunnelStreamDestroy(RsslTunnelStream *pTunnel)
{
	TunnelStreamImpl *pTunnelImpl = (TunnelStreamImpl*)pTunnel;
	TunnelManagerImpl *pManagerImpl = (TunnelManagerImpl*)pTunnelImpl->_manager;
	RsslQueueLink *pLink;

	pTunnelImpl->_state = TSS_INACTIVE;
	tunnelStreamUnsetNeedsDispatch((TunnelStreamImpl*)pTunnel);
	tunnelStreamUnsetHasExpireTime((TunnelStreamImpl*)pTunnel);
	rsslQueueRemoveLink(&pManagerImpl->_tunnelStreams, &pTunnelImpl->_managerLink);

	for (pLink = rsslQueueStart(&pTunnelImpl->_substreams); pLink != NULL;
			pLink = rsslQueueForth(&pTunnelImpl->_substreams))
	{
		TunnelSubstream *pSubstream = RSSL_QUEUE_LINK_TO_OBJECT(TunnelSubstream,
				_tunnelQueueLink, pLink);

		rsslHashTableRemoveLink(&pTunnelImpl->_substreamsById, &pSubstream->_tunnelTableLink);
		rsslQueueRemoveLink(&pTunnelImpl->_substreams, &pSubstream->_tunnelQueueLink);
		tunnelSubstreamDestroy(pSubstream);
	}

	while((pLink = rsslQueueRemoveFirstLink(&pTunnelImpl->_tunnelBufferTransmitList))
			!= NULL)
	{
		TunnelBufferImpl *pBufferImpl = RSSL_QUEUE_LINK_TO_OBJECT(TunnelBufferImpl, _tbpLink, pLink);
		tunnelStreamReleaseBuffer(pTunnelImpl, pBufferImpl);
	}

	while((pLink = rsslQueueRemoveFirstLink(&pTunnelImpl->_tunnelBufferWaitAckList))
			!= NULL)
	{
		TunnelBufferImpl *pBufferImpl = RSSL_QUEUE_LINK_TO_OBJECT(TunnelBufferImpl, _tbpLink, pLink);
		tunnelStreamReleaseBuffer(pTunnelImpl, pBufferImpl);
	}


	bufferPoolCleanup(&pTunnelImpl->_memoryBufferPool);

	rsslHeapBufferCleanup(&pTunnelImpl->_memoryBuffer);
	rsslHashTableCleanup(&pTunnelImpl->_substreamsById);

	for (pLink = rsslQueueStart(&pTunnelImpl->_fragmentationProgressQueue); pLink != NULL;
		 pLink = rsslQueueForth(&pTunnelImpl->_fragmentationProgressQueue))
	{
		TunnelStreamFragmentationProgress *pFragmentationProgress = RSSL_QUEUE_LINK_TO_OBJECT(TunnelStreamFragmentationProgress, fragmentationQueueLink, pLink);

		rsslHashTableRemoveLink(&pTunnelImpl->_fragmentationProgressHashTable, &pFragmentationProgress->fragmentationHashLink);
		rsslQueueRemoveLink(&pTunnelImpl->_fragmentationProgressQueue, &pFragmentationProgress->fragmentationQueueLink);

		bigBufferPoolRelease(&pTunnelImpl->_bigBufferPool, &pFragmentationProgress->pBigBuffer->_poolBuffer);
		free(pFragmentationProgress);
	}
	rsslHashTableCleanup(&pTunnelImpl->_fragmentationProgressHashTable);

	for (pLink = rsslQueueStart(&pTunnelImpl->_pendingBigBufferList); pLink != NULL;
		 pLink = rsslQueueForth(&pTunnelImpl->_pendingBigBufferList))
	{
		TunnelBufferImpl *pBigBuffer = RSSL_QUEUE_LINK_TO_OBJECT(TunnelBufferImpl, _tbpLink, pLink);

		rsslQueueRemoveLink(&pTunnelImpl->_pendingBigBufferList, pLink);

		bigBufferPoolRelease(&pTunnelImpl->_bigBufferPool, &pBigBuffer->_poolBuffer);
	}
	bigBufferPoolCleanup(&pTunnelImpl->_bigBufferPool);
	pTunnelImpl->_streamVersion = COS_CURRENT_STREAM_VERSION;
	pTunnelImpl->_requestRetryCount = 0;
	pTunnelImpl->_messageId = 0;

	if (pTunnelImpl->_isNameAllocated)
		free(pTunnelImpl->base.name);
	if (pTunnelImpl->_persistenceFilePath != NULL)
		free(pTunnelImpl->_persistenceFilePath);
	if (pTunnelImpl->_pAuthLoginRequest != NULL && pTunnelImpl->_authLoginRequestIsCopied)
		free(pTunnelImpl->_pAuthLoginRequest);
	free(pTunnel);
}

void tunnelStreamAddTimeoutBuffer(TunnelStreamImpl *pTunnelImpl, TunnelBufferImpl *pBufferImpl,
		RsslInt64 expireTime)
{
	RsslQueueLink *pLink;

	assert(expireTime != RDM_QMSG_TC_IMMEDIATE);

	pBufferImpl->_expireTime = expireTime;
	tunnelStreamSetNextExpireTime(pTunnelImpl, expireTime);

	/* Starting from the back (as most likely this message will be put on or near the back),
	 * find a message that expires sooner, and put this message behind it. */
	for (pLink = rsslQueuePeekBack(&pTunnelImpl->_tunnelBufferTimeoutList);
			pLink != NULL;
			pLink = rsslQueuePeekPrev(&pTunnelImpl->_tunnelBufferTimeoutList, pLink))
	{
		TunnelBufferImpl *pTimeoutBufferImpl = RSSL_QUEUE_LINK_TO_OBJECT(TunnelBufferImpl,
				_timeoutLink, pLink);

		if (pTimeoutBufferImpl->_expireTime <= pBufferImpl->_expireTime)
		{
			rsslQueueInsertAfter(&pTunnelImpl->_tunnelBufferTimeoutList,
					&pTimeoutBufferImpl->_timeoutLink, &pBufferImpl->_timeoutLink);
			return;
		}
	}

	rsslQueueAddLinkToFront(&pTunnelImpl->_tunnelBufferTimeoutList, 
			&pBufferImpl->_timeoutLink);
}

void tunnelStreamAddImmediateTimeoutBuffer(TunnelStreamImpl *pTunnelImpl, TunnelBufferImpl *pBufferImpl)
{
	pBufferImpl->_expireTime = RDM_QMSG_TC_IMMEDIATE;
	rsslQueueAddLinkToBack(&pTunnelImpl->_tunnelBufferImmediateList, 
			&pBufferImpl->_timeoutLink);
}


static void _tunnelStreamRemoveTimeoutBuffer(TunnelStreamImpl *pTunnelImpl, 
		TunnelBufferImpl *pBufferImpl)
{
	if (pBufferImpl->_expireTime > RDM_QMSG_TC_IMMEDIATE)
		rsslQueueRemoveLink(&pTunnelImpl->_tunnelBufferTimeoutList, 
				&pBufferImpl->_timeoutLink);
	else if (pBufferImpl->_expireTime == RDM_QMSG_TC_IMMEDIATE)
		rsslQueueRemoveLink(&pTunnelImpl->_tunnelBufferImmediateList,
				&pBufferImpl->_timeoutLink);
}

static TunnelBufferImpl *_tunnelStreamGetBufferImplObject(TunnelStreamImpl *pTunnelImpl, RsslErrorInfo *pErrorInfo)
{
	RsslQueueLink *pLink;
	TunnelBufferImpl *pBufferImpl;

	if ((pLink = rsslQueueRemoveFirstLink(&pTunnelImpl->_manager->_tunnelBufferPool))
			!= NULL)
	{
		pBufferImpl = RSSL_QUEUE_LINK_TO_OBJECT(TunnelBufferImpl, _tbpLink, pLink);
	}
	else if ((pBufferImpl = (TunnelBufferImpl*)malloc(sizeof(TunnelBufferImpl))) == NULL)
	{
		rsslSetErrorInfo(pErrorInfo, RSSL_EIC_FAILURE, 
				RSSL_RET_FAILURE, __FILE__, __LINE__,
				"Failed to allocate tunnel buffer object.");
		return NULL;
	}

	tunnelBufferImplClear(pBufferImpl);
	return pBufferImpl;

}

TunnelBufferImpl* tunnelStreamGetBuffer(
		TunnelStreamImpl *pTunnelImpl, RsslUInt32 length, RsslBool isAppBuffer, RsslBool isTranslationBuffer, RsslErrorInfo *pErrorInfo) 
{
	TunnelBufferImpl *pBufferImpl;
	TunnelStreamData	dataMsg;
	RsslEncodeIterator	eIter;
	RsslRet				ret;

	if (isAppBuffer && pTunnelImpl->_state != TSS_OPEN)
	{
		rsslSetErrorInfo(pErrorInfo, RSSL_EIC_FAILURE, RSSL_RET_INVALID_ARGUMENT, 
				__FILE__, __LINE__, "This stream is not open. Only open tunnel streams can get buffers.");
		return NULL;
	}

	if (length == 0)
	{
		rsslSetErrorInfo(pErrorInfo, RSSL_EIC_FAILURE, RSSL_RET_INVALID_ARGUMENT, 
				__FILE__, __LINE__, "Cannot get a buffer of zero length.");
		return NULL;
	}

	if (length > pTunnelImpl->base.classOfService.common.maxMsgSize)
	{
		rsslSetErrorInfo(pErrorInfo, RSSL_EIC_FAILURE, 
				RSSL_RET_INVALID_ARGUMENT, __FILE__, __LINE__,
				"Requested buffer length is larger than max message size.");
		return NULL;
	}

	if (length <= pTunnelImpl->base.classOfService.common.maxFragmentSize) // not big buffer
	{
		/* Get buffer object. */
		if ((pBufferImpl = _tunnelStreamGetBufferImplObject(pTunnelImpl, pErrorInfo)) == NULL)
			return NULL;

		/* Get memory for buffer. */
		if (bufferPoolGet(&pTunnelImpl->_memoryBufferPool,
					&pBufferImpl->_poolBuffer, length + TS_HEADER_MAX_LENGTH, isAppBuffer,
					isTranslationBuffer, pErrorInfo)
				!= RSSL_RET_SUCCESS)
		{
			rsslQueueAddLinkToBack(&pTunnelImpl->_manager->_tunnelBufferPool,
					&pBufferImpl->_tbpLink);
			return NULL;
		}

		pBufferImpl->_startPos = pBufferImpl->_poolBuffer.buffer.data;
	}
	else // big buffer
	{
		pBufferImpl = (TunnelBufferImpl *)bigBufferPoolGet(&pTunnelImpl->_bigBufferPool, length + TS_HEADER_MAX_LENGTH, pErrorInfo);
	}

	if (pBufferImpl != NULL)
	{
		/* Encode tunnel header. */
		tunnelStreamDataClear(&dataMsg);
		dataMsg.base.streamId = pTunnelImpl->base.streamId;
		dataMsg.base.domainType = pTunnelImpl->base.domainType;
		dataMsg.seqNum = 0; /* (This will be replaced later) */

		rsslClearEncodeIterator(&eIter);
		rsslSetEncodeIteratorRWFVersion(&eIter, pTunnelImpl->base.pReactorChannel->majorVersion,
			pTunnelImpl->base.pReactorChannel->minorVersion);
		rsslSetEncodeIteratorBuffer(&eIter, &pBufferImpl->_poolBuffer.buffer);

		if ((ret = tunnelStreamDataEncodeInit(&eIter, &dataMsg, pTunnelImpl->_streamVersion))
				!= RSSL_RET_SUCCESS)
		{
			rsslSetErrorInfo(pErrorInfo, RSSL_EIC_FAILURE, 
					ret, __FILE__, __LINE__,
					"Failed to encode tunnel stream header.");
			tunnelStreamReleaseBuffer(pTunnelImpl, pBufferImpl);
			return NULL;
		}

		assert(pBufferImpl->_poolBuffer.buffer.length >= rsslGetEncodedBufferLength(&eIter));
		assert(rsslGetEncodedBufferLength(&eIter) < TS_HEADER_MAX_LENGTH);

		/* Move buffer position to user's starting point. */
		pBufferImpl->_poolBuffer.buffer.data += rsslGetEncodedBufferLength(&eIter);
		pBufferImpl->_poolBuffer.buffer.length = length;
		pBufferImpl->_dataStartPos = pBufferImpl->_poolBuffer.buffer.data;
		pBufferImpl->_tunnel = (RsslTunnelStream*)pTunnelImpl;
		pBufferImpl->_integrity = TS_BUFFER_INTEGRITY;
		pBufferImpl->_maxLength = length;
	}

	return pBufferImpl;
}

void tunnelStreamReleaseBuffer(
		TunnelStreamImpl *pTunnelImpl, TunnelBufferImpl *pBufferImpl)
{
	if (!pBufferImpl->_isBigBuffer)
	{
		if (pBufferImpl->_bufferType == TS_BT_DATA)
		{
			if (pBufferImpl->_substream && pBufferImpl->_flags & TBF_QUEUE_CLOSE)
			{
				tunnelSubstreamDestroy(pBufferImpl->_substream);
				pBufferImpl->_substream = NULL;
			}

			/* Release buffer memory */
			bufferPoolRelease(&pTunnelImpl->_memoryBufferPool,
					&pBufferImpl->_poolBuffer);
		}
		else
		{
			/* An ack for our FIN was acknowledged. Stop the retransmission timer. */
			_tunnelStreamUnsetResponseTimer(pTunnelImpl);
			pTunnelImpl->_flags |= TSF_RECEIVED_ACK_OF_FIN;
		}

		/* Release buffer object. */
		rsslQueueAddLinkToBack(&pTunnelImpl->_manager->_tunnelBufferPool,
				&pBufferImpl->_tbpLink);
	}
	else
	{
		bigBufferPoolRelease(&pTunnelImpl->_bigBufferPool, &pBufferImpl->_poolBuffer);
	}
}

RsslRet tunnelStreamGetInfo(TunnelStreamImpl* pTunnelImpl, RsslTunnelStreamInfo *pInfo, RsslErrorInfo *pErrorInfo)
{
	pInfo->buffersUsed = bufferPoolGetUsed(&pTunnelImpl->_memoryBufferPool) + bigBufferPoolGetUsed(&pTunnelImpl->_bigBufferPool);
	return RSSL_RET_SUCCESS;
}

static RsslRet _tunnelStreamHandleEstablished(TunnelStreamImpl *pTunnelImpl, RsslErrorInfo *pErrorInfo)
{
	RsslClassOfService *pCos = &pTunnelImpl->base.classOfService;

	if (initBufferPool(&pTunnelImpl->_memoryBufferPool,
		pTunnelImpl->_guaranteedOutputBuffersAppLimit,
		(RsslUInt32)pCos->common.maxFragmentSize + TS_HEADER_MAX_LENGTH, pTunnelImpl->_guaranteedOutputBuffersAppLimit, pErrorInfo)
		!= RSSL_RET_SUCCESS)
		return RSSL_RET_FAILURE;

	bigBufferPoolInit(&pTunnelImpl->_bigBufferPool, pCos->common.maxFragmentSize, pTunnelImpl->_guaranteedOutputBuffersAppLimit);

	return RSSL_RET_SUCCESS;
}

static RsslRet _tunnelStreamSendMessages(TunnelStreamImpl *pTunnelImpl, RsslErrorInfo *pErrorInfo)
{
	RsslQueueLink *pLink;
	RsslReactorChannel *pReactorChannel = pTunnelImpl->base.pReactorChannel;
	RsslRet ret;

	/* Send a message, if needed. */
	while ((pLink = rsslQueuePeekFront(
				&pTunnelImpl->_tunnelBufferTransmitList)) != NULL)
	{
		TunnelBufferImpl *pBufferImpl =
			RSSL_QUEUE_LINK_TO_OBJECT(TunnelBufferImpl, _tbpLink, 
					pLink);

		RsslBuffer *pChannelBuffer;
		RsslEncodeIterator eIter;

		if (pBufferImpl->_bufferType == TS_BT_DATA)
		{

			if (!_tunnelStreamCanSendMessage(pTunnelImpl, pBufferImpl))
			{
				/* Send window is full. */
				tunnelStreamUnsetNeedsDispatch(pTunnelImpl);
				return RSSL_RET_SUCCESS;
			}

			/* Get channel buffer. */
			/* Even if the watchlist is enabled, it should be able to send this
			 * message through as a buffer. */
			if ((pChannelBuffer = tunnelManagerGetChannelBuffer(pTunnelImpl->_manager, NULL,
				pBufferImpl->_poolBuffer.buffer.length, RSSL_FALSE, pErrorInfo))
				== NULL)
			{
				if (pErrorInfo->rsslError.rsslErrorId == RSSL_RET_BUFFER_NO_BUFFERS)
					return RSSL_RET_BUFFER_NO_BUFFERS;

				rsslSetErrorInfoLocation(pErrorInfo, __FILE__, __LINE__);
				return RSSL_RET_FAILURE;
			}

			if (!tunnelBufferImplIsTransmitted(pBufferImpl))
			{
				/* Update sequence number */
				rsslClearEncodeIterator(&eIter);
				rsslSetEncodeIteratorRWFVersion(&eIter, pReactorChannel->majorVersion, pReactorChannel->minorVersion);
				rsslSetEncodeIteratorBuffer(&eIter, &pBufferImpl->_poolBuffer.buffer);
				++pTunnelImpl->_lastOutSeqNum;
				pBufferImpl->_seqNum = pTunnelImpl->_lastOutSeqNum;
				if ((ret = rsslReplaceSeqNum(&eIter, pTunnelImpl->_lastOutSeqNum)) != RSSL_RET_SUCCESS)
				{
					rsslSetErrorInfo(pErrorInfo, RSSL_EIC_FAILURE, ret,
							__FILE__, __LINE__, "Failed to set sequence number on message.");
					return RSSL_RET_FAILURE;
				}

				if (pBufferImpl->_substream != NULL)
				{
					if (pBufferImpl->_flags & TBF_QUEUE_CLOSE)
					{
						tunnelSubstreamDestroy(pBufferImpl->_substream);
						pBufferImpl->_substream = NULL;
					}
					else
					{
						RsslBuffer tmpBuffer = pBufferImpl->_poolBuffer.buffer;

						pBufferImpl->_poolBuffer.buffer.length -= (RsslUInt32)(pBufferImpl->_dataStartPos - pBufferImpl->_startPos);
						pBufferImpl->_poolBuffer.buffer.data = pBufferImpl->_dataStartPos;

						/* If the message cannot be marked for transmission, it is likely due to a
						 * failure of the persistence file. */
						if (tunnelSubstreamUpdateMsgForTransmit(pBufferImpl->_substream, (RsslBuffer*)pBufferImpl, pErrorInfo)
								!= RSSL_RET_SUCCESS)
							return tunnelStreamHandleError(pTunnelImpl, pErrorInfo);

						/* Timeout should not be processed now. */
						_tunnelStreamRemoveTimeoutBuffer(pTunnelImpl, pBufferImpl);
						pBufferImpl->_poolBuffer.buffer = tmpBuffer;
					}
				}

				tunnelBufferImplSetIsTransmitted(pBufferImpl, RSSL_TRUE);
			}


			/* Copy message to channel buffer. */
			memcpy(pChannelBuffer->data, pBufferImpl->_poolBuffer.buffer.data,
					pBufferImpl->_poolBuffer.buffer.length);

			/* Send it. */
			if ((ret = tunnelManagerSubmitChannelBuffer(pTunnelImpl->_manager, pChannelBuffer,
							pErrorInfo))
					< RSSL_RET_SUCCESS)
			{
				rsslSetErrorInfoLocation(pErrorInfo, __FILE__, __LINE__);
				return RSSL_RET_CHANNEL_ERROR;
			}

			pTunnelImpl->_bytesWaitingAck += pBufferImpl->_poolBuffer.buffer.length;
		}
		else /* TS_BT_FIN */
		{
			/* Send a FIN */

			TunnelStreamAck ackMsg;
			tunnelStreamAckClear(&ackMsg);

			if (!tunnelBufferImplIsTransmitted(pBufferImpl))
			{
				/* Update sequence number. */
				++pTunnelImpl->_lastOutSeqNum;
				pBufferImpl->_seqNum = pTunnelImpl->_lastOutSeqNum;
				tunnelBufferImplSetIsTransmitted(pBufferImpl, RSSL_TRUE);
			}

			ackMsg.base.streamId = pTunnelImpl->base.streamId;
			ackMsg.seqNum = pBufferImpl->_seqNum;
			ackMsg.base.domainType = pTunnelImpl->base.domainType;
			ackMsg.recvWindow = (RsslInt32)pTunnelImpl->base.classOfService.flowControl.recvWindowSize;
			ackMsg.flags = TS_ACKF_FIN;

			if ((ret = _tunnelStreamSendAck(pTunnelImpl, &ackMsg, NULL, pErrorInfo)) != RSSL_RET_SUCCESS)
				return ret;
		}

		rsslQueueRemoveLink(&pTunnelImpl->_tunnelBufferTransmitList,
				pLink);

		rsslQueueAddLinkToBack(&pTunnelImpl->_tunnelBufferWaitAckList,
				pLink);

		if (tunnelStreamDebugFlags & TS_DBG_ACKS)
			printf("<TunnelStreamDebug streamId:%d> Sent message seqNum: %u, Latest received seqNum: %u, latest accepted seqNum: %u, last acked seqNum: %u, bytes waiting ack: %lld, send window: %lld\n", 
					pTunnelImpl->base.streamId, 
					pBufferImpl->_seqNum, pTunnelImpl->_lastInSeqNum, pTunnelImpl->_lastInSeqNumAccepted, pTunnelImpl->_lastInAckedSeqNum, pTunnelImpl->_bytesWaitingAck, pTunnelImpl->base.classOfService.flowControl.sendWindowSize);
	}

	if (rsslQueueGetElementCount(&pTunnelImpl->_tunnelBufferTransmitList) == 0)
		tunnelStreamUnsetNeedsDispatch(pTunnelImpl);

	return RSSL_RET_SUCCESS;
}

static RsslRet _tunnelStreamCallStatusEventCallback(TunnelStreamImpl *pTunnelImpl, RsslState *pState, RsslMsg *pRsslMsg, RsslTunnelStreamAuthInfo *pAuthInfo, RsslErrorInfo *pErrorInfo)
{
	RsslTunnelStreamStatusEvent statusEvent;
	RsslReactorCallbackRet cret;

	tunnelStreamStatusEventClear(&statusEvent);
	statusEvent.pState = pState;
	statusEvent.pRsslMsg = pRsslMsg;
	statusEvent.pReactorChannel = pTunnelImpl->_manager->base._pReactorChannel;
	statusEvent.pAuthInfo = pAuthInfo;

	cret = (*pTunnelImpl->_statusEventCallback)((RsslTunnelStream*)pTunnelImpl,
			&statusEvent);

	if (cret != RSSL_RC_CRET_SUCCESS)
	{
		rsslSetErrorInfo(pErrorInfo, RSSL_EIC_FAILURE, RSSL_RET_FAILURE, 
				__FILE__, __LINE__, "Unexpected return code %d from tunnel stream statusEventCallback.", cret);
		return RSSL_RET_FAILURE;
	}

	return RSSL_RET_SUCCESS;
}

static RsslRet _tunnelStreamHandleFragmentedMsg(TunnelStreamImpl *pTunnelImpl, TunnelStreamData *pDataMsg, RsslBuffer *pFragmentedData, RsslErrorInfo *pErrorInfo)
{
	RsslRet ret = RSSL_RET_SUCCESS;
	TunnelBufferImpl *pBigBuffer;
	RsslHashLink *pHashLink;
	TunnelStreamFragmentationProgress *pFragmentationProgress;
	if (pDataMsg->fragmentNumber > 1) // subsequent fragment
	{
		// look up message id in hash table and continue re-assembly
		if ((pHashLink = rsslHashTableFind(&pTunnelImpl->_fragmentationProgressHashTable, &pDataMsg->messageId, NULL)) != NULL)
			pFragmentationProgress = RSSL_HASH_LINK_TO_OBJECT(TunnelStreamFragmentationProgress, fragmentationHashLink, pHashLink);
		else
			pFragmentationProgress = NULL;

		if (pFragmentationProgress == NULL)
		{
			rsslSetErrorInfo(pErrorInfo, RSSL_EIC_FAILURE, RSSL_RET_FAILURE, 
					__FILE__, __LINE__, "Received fragmented message with fragmentNumber > 1 but never received fragmentNumber of 1");
			return RSSL_RET_FAILURE;
		}

		// copy received buffer contents
		pBigBuffer = pFragmentationProgress->pBigBuffer;
		memcpy(&pBigBuffer->_poolBuffer.buffer.data[pFragmentationProgress->bytesAlreadyCopied], pFragmentedData->data, pFragmentedData->length);

		// update fragmentation progress structure
		pFragmentationProgress->bytesAlreadyCopied += pFragmentedData->length;

		// if all bytes received, call back user with re-assembled buffer and clean up
		if (pFragmentationProgress->bytesAlreadyCopied == pDataMsg->totalMsgLength)
		{
			pBigBuffer->_poolBuffer.buffer.length = pDataMsg->totalMsgLength;

			// call back user
			if (pDataMsg->containerType != RSSL_DT_MSG)
			{
				ret = tunnelStreamCallMsgCallback(pTunnelImpl, &pBigBuffer->_poolBuffer.buffer, NULL, pDataMsg->containerType, pErrorInfo);
			}
			else
			{
				RsslDecodeIterator dIter;
				RsslMsg msg;

				rsslClearDecodeIterator(&dIter);
				rsslSetDecodeIteratorRWFVersion(&dIter, pTunnelImpl->base.classOfService.common.protocolMajorVersion,
						pTunnelImpl->base.classOfService.common.protocolMinorVersion);
				rsslSetDecodeIteratorBuffer(&dIter, &pBigBuffer->_poolBuffer.buffer);

				if ((ret = rsslDecodeMsg(&dIter, &msg)) != RSSL_RET_SUCCESS)
				{
					rsslSetErrorInfo(pErrorInfo, RSSL_EIC_FAILURE, ret, 
							__FILE__, __LINE__, "Failed to decode re-assembled message.");
					return RSSL_RET_FAILURE;
				}

				ret = tunnelStreamCallMsgCallback(pTunnelImpl, &pBigBuffer->_poolBuffer.buffer, &msg, pDataMsg->containerType, pErrorInfo);
			}

			// remove from hash table
			rsslHashTableRemoveLink(&pTunnelImpl->_fragmentationProgressHashTable, &pFragmentationProgress->fragmentationHashLink);

			// remove from queue
			rsslQueueRemoveLink(&pTunnelImpl->_fragmentationProgressQueue, &pFragmentationProgress->fragmentationQueueLink);

			// free memory
			bigBufferPoolRelease(&pTunnelImpl->_bigBufferPool, &pBigBuffer->_poolBuffer);
			free(pFragmentationProgress);
		}
	}
	else // first fragment
	{
		// check if re-assembly already in progress and overwrite previous re-assembly if it exists
		if ((pHashLink = rsslHashTableFind(&pTunnelImpl->_fragmentationProgressHashTable, &pDataMsg->messageId, NULL)) != NULL)
		{
			pFragmentationProgress = RSSL_HASH_LINK_TO_OBJECT(TunnelStreamFragmentationProgress, fragmentationHashLink, pHashLink);

			// release previous big buffer
			bigBufferPoolRelease(&pTunnelImpl->_bigBufferPool, &pFragmentationProgress->pBigBuffer->_poolBuffer);

			// reset progress
			pFragmentationProgress->bytesAlreadyCopied = 0;
		}
		else
		{
			// create new structure to track fragmentation progress
			pFragmentationProgress = (TunnelStreamFragmentationProgress *)malloc(sizeof(TunnelStreamFragmentationProgress));
			clearTunnelStreamFragmentationProgress(pFragmentationProgress);
		}

		// create big buffer
		pBigBuffer = (TunnelBufferImpl *)bigBufferPoolGet(&pTunnelImpl->_bigBufferPool, pDataMsg->totalMsgLength, pErrorInfo);
		if (pBigBuffer == NULL)
		{
			rsslSetErrorInfo(pErrorInfo, RSSL_EIC_FAILURE, RSSL_RET_BUFFER_NO_BUFFERS, 
					__FILE__, __LINE__, "Unable to acquire a big buffer.");
			return RSSL_RET_BUFFER_NO_BUFFERS;
		}

		// copy received buffer contents
		memcpy(pBigBuffer->_poolBuffer.buffer.data, pFragmentedData->data, pFragmentedData->length);

		// update fragmentation progress structure
		pFragmentationProgress->pBigBuffer = pBigBuffer;
		pFragmentationProgress->bytesAlreadyCopied = pFragmentedData->length;

		// put fragmentation progress structure into hash table indexed by message id
		rsslHashLinkInit(&pFragmentationProgress->fragmentationHashLink);
		rsslHashTableInsertLink(&pTunnelImpl->_fragmentationProgressHashTable, &pFragmentationProgress->fragmentationHashLink, &pDataMsg->messageId, NULL);

		// also insert into fragmentation queue
		rsslQueueAddLinkToBack(&pTunnelImpl->_fragmentationProgressQueue, &pFragmentationProgress->fragmentationQueueLink);		
	}

	return ret;
}

static RsslUInt16 _tunnelStreamFragMsgId(TunnelStreamImpl *pTunnelImpl)
{
    // defined as unsigned short starting from 1
    RsslUInt16 msgId = ++pTunnelImpl->_messageId;
    if (msgId == 0)
    {
    	msgId = pTunnelImpl->_messageId = 1;
    }
    	
    return msgId;
}

static TunnelBufferImpl* _tunnelStreamGetBufferForFragmentation(TunnelStreamImpl *pTunnelImpl, RsslUInt32 length, RsslUInt32 totalMsgLen, RsslUInt32 fragmentNumber,
																RsslUInt16 msgId, RsslUInt8 containerType, RsslBool msgComplete, RsslErrorInfo *pErrorInfo)
{
    TunnelBufferImpl *pBufferImpl;
	TunnelStreamData dataMsg;
	RsslEncodeIterator eIter;
    int ret;
        
	/* Get buffer object. */
	if ((pBufferImpl = _tunnelStreamGetBufferImplObject(pTunnelImpl, pErrorInfo)) == NULL)
		return NULL;

	/* Get memory for buffer. */
	if (bufferPoolGet(&pTunnelImpl->_memoryBufferPool, &pBufferImpl->_poolBuffer,
					length + TS_HEADER_MAX_LENGTH, RSSL_TRUE, RSSL_FALSE, pErrorInfo) != RSSL_RET_SUCCESS)
	{
		rsslQueueAddLinkToBack(&pTunnelImpl->_manager->_tunnelBufferPool,
				&pBufferImpl->_tbpLink);
		return NULL;
	}

	pBufferImpl->_startPos = pBufferImpl->_poolBuffer.buffer.data;

	tunnelStreamDataClear(&dataMsg);
	dataMsg.base.streamId = pTunnelImpl->base.streamId;
	dataMsg.base.domainType = pTunnelImpl->base.domainType;
	dataMsg.seqNum = 0; /* (This will be replaced later) */
	if (totalMsgLen > pTunnelImpl->base.classOfService.common.maxFragmentSize)
	{
		dataMsg.flags |= TS_DF_FRAGMENTED;
		dataMsg.totalMsgLength = totalMsgLen;
		dataMsg.fragmentNumber = fragmentNumber;
		dataMsg.messageId = msgId;
		dataMsg.containerType = containerType;
		dataMsg.msgComplete = msgComplete;
	}
	else
	{
		// let tunnelStreamDataEncodeInit() know tunnel stream data header container type
		dataMsg.containerType = containerType;
	}

	rsslClearEncodeIterator(&eIter);
	rsslSetEncodeIteratorRWFVersion(&eIter, pTunnelImpl->base.classOfService.common.protocolMajorVersion,
			pTunnelImpl->base.classOfService.common.protocolMinorVersion);
	rsslSetEncodeIteratorBuffer(&eIter, &pBufferImpl->_poolBuffer.buffer);

	if ((ret = tunnelStreamDataEncodeInit(&eIter, &dataMsg, pTunnelImpl->_streamVersion))
			!= RSSL_RET_SUCCESS)
	{
		rsslSetErrorInfo(pErrorInfo, RSSL_EIC_FAILURE, 
				ret, __FILE__, __LINE__,
				"Failed to encode tunnel stream header.");
		tunnelStreamReleaseBuffer(pTunnelImpl, pBufferImpl);
		return NULL;
	}

	assert(pBufferImpl->_poolBuffer.buffer.length >= rsslGetEncodedBufferLength(&eIter));
	assert(rsslGetEncodedBufferLength(&eIter) < TS_HEADER_MAX_LENGTH);

	/* Move buffer position to user's starting point. */
	pBufferImpl->_poolBuffer.buffer.data += rsslGetEncodedBufferLength(&eIter);
	pBufferImpl->_poolBuffer.buffer.length = length;
	pBufferImpl->_dataStartPos = pBufferImpl->_poolBuffer.buffer.data;
	pBufferImpl->_tunnel = (RsslTunnelStream*)pTunnelImpl;
	pBufferImpl->_integrity = TS_BUFFER_INTEGRITY;
	pBufferImpl->_maxLength = length;

	return pBufferImpl;
}

static RsslBool _tunnelStreamHandleRequestRetry(TunnelStreamImpl *pTunnelImpl)
{
	if (pTunnelImpl->_state != TSS_WAIT_REFRESH || pTunnelImpl->_requestRetryCount >= MAX_REQUEST_RETRIES)
	{
		pTunnelImpl->_requestRetryCount = 0;
		return RSSL_FALSE;
	}
	else // retry request
	{
		// change back to SEND_REQUEST state to force retry with different stream version
		pTunnelImpl->_state = TSS_SEND_REQUEST;
		pTunnelImpl->_requestRetryCount++;
        tunnelStreamSetNeedsDispatch(pTunnelImpl);
		return RSSL_TRUE;
	}
}    
