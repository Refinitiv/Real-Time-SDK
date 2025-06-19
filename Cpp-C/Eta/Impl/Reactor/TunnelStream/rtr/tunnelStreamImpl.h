/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2015,2017,2019,2024 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

#ifndef TUNNEL_STREAM_IMPL_H
#define TUNNEL_STREAM_IMPL_H

#include "rtr/rsslReactorImpl.h"
#include "rtr/rsslTunnelStream.h"
#include "rtr/tunnelStreamReturnCodes.h"
#include "rtr/msgQueueHeader.h"
#include "rtr/msgQueueSubstreamHeader.h"
#include "rtr/rsslQueue.h"
#include "rtr/tunnelSubstream.h"
#include "rtr/tunnelManagerImpl.h"
#include "rtr/rsslHashTable.h"
#include <assert.h>

#ifdef __cplusplus
extern "C" {
#endif

#define MAX_REQUEST_RETRIES 1 /* Maximum tunnel stream request retries. */

#define NUM_POOLS 32

typedef struct
{
	RsslQueue _pools[NUM_POOLS];
	RsslUInt _maxSize;
	RsslUInt _maxPool;
	RsslUInt _fragmentSize;
	RsslUInt _maxNumBuffers;
	RsslUInt _currentNumBuffers;
} BigBufferPool;

typedef enum
{
	TS_BT_DATA		= 0, /* Represents a data message. */
	TS_BT_FIN		= 1  /* Does not contain data. Represents the FIN. */
} TunnelBufferType;

typedef enum
{
	TBF_NONE 		= 0x0,	/* None */
	TBF_QUEUE_CLOSE = 0x1,	/* Destroy the substream associated with this buffer. */
	TBF_IGNORE_FC	= 0x2	/* Ignore the flowControl window for this message. */
} TunnelBufferFlags;

typedef struct
{
	PoolBuffer			_poolBuffer;				/* Pool buffer object. Must be first member (RsslBuffer contained in PoolBuffer must also be first). */
	RsslUInt8			_bufferType;				/* See TunnelBufferType. */
	RsslUInt32			_integrity;					/* Used to ensure this is a valid Tunnel Stream Buffer. */
	RsslQueueLink		_tbpLink;					/* Link to pool of TunnelBuffer objects, or for tunnel stream queues.  */
	RsslQueueLink		_timeoutLink;				/* Link for tunnel stream timeout queue. */
	RsslUInt32			_seqNum;					/* Sequence number assigned to this buffer. */
	RsslUInt32			_maxLength;					/* Length allocated to this buffer when it was retrieved from the pool. */
	char				*_startPos;					/* Start of buffer, where stream header is encoded. */
	char				*_dataStartPos;				/* Start of content of buffers (substream header starts here) */
	RsslTunnelStream	*_tunnel;					/* Tunnel that is holding this buffer. */
	TunnelSubstream		*_substream;				/* Substream that sent this message, if any. */
	PersistentMsg		*_persistentMsg;			/* Persistence associated with this message, if any. */
	RsslBool			_isTransmitted;				/* Buffer has been transmitted (so seqNum is already set) */
	RsslInt64			_expireTime;				/* Timeout associated with this message. */
	RsslUInt8			_flags;						/* See TunnelBufferFlags. */
	RsslBool			_isBigBuffer;				/* Buffer is a big buffer for fragmentation. */
	RsslBool			_fragmentationInProgress;	/* Fragmentation is in progress. */
	RsslUInt32			_totalMsgLength;			/* Total length of fragmented message. */
	RsslUInt32			_bytesRemainingToSend;		/* Fragmentation bytes remaining to send. */
	RsslUInt32			_lastFragmentId;			/* Last fragment id used while sending. */
	RsslUInt16			_messageId;					/* Message id used for fragmentation. */
	RsslUInt8			_containerType;				/* Container type of fragmented message. */
	RsslUInt8			_bigBufferPoolIndex;		/* Index of big buffer pool. */
} TunnelBufferImpl;

RTR_C_INLINE void tunnelBufferImplClear(TunnelBufferImpl *pBufferImpl)
{
	memset(pBufferImpl, 0, sizeof(TunnelBufferImpl));
}

RTR_C_INLINE void tunnelBufferImplSetIsTransmitted(TunnelBufferImpl *pBufferImpl, RsslBool isTransmitted)
{
	pBufferImpl->_isTransmitted = isTransmitted;
}

RTR_C_INLINE RsslBool tunnelBufferImplIsTransmitted(TunnelBufferImpl *pBufferImpl)
{
	return pBufferImpl->_isTransmitted;
}

RTR_C_INLINE void tunnelBufferImplSetPersistence(TunnelBufferImpl *pBufferImpl,
		TunnelSubstream *pSubstream, PersistentMsg *pPersistentMsg)
{
	pBufferImpl->_substream = pSubstream;
	pBufferImpl->_persistentMsg = pPersistentMsg;
}

typedef enum
{
	TSS_INACTIVE						= 0,	/* Stream is not in use (should not encounter this) */
	TSS_SEND_REQUEST					= 1,	/* Stream needs to send request. */
	TSS_WAIT_REFRESH					= 2,	/* Stream is waiting for a refresh */
	TSS_SEND_REFRESH					= 3,	/* Stream is a provider that needs to send a refresh. */
	TSS_SEND_AUTH_LOGIN_REQUEST			= 4,	/* Stream needs to send a login request for authentication. */
	TSS_WAIT_AUTH_LOGIN_RESPONSE		= 5,	/* Stream is waiting for its login auth */
	TSS_OPEN							= 6,	/* Stream is open */
	TSS_SEND_FIN						= 8,	/* Stream needs to send a FIN. */
	TSS_WAIT_FIN						= 9,	/* Waiting for a FIN from the remote end. */
	TSS_SEND_ACK_OF_FIN					= 11,	/* Send an AckMsg to a received FIN-ACK */
	TSS_WAIT_ACK_OF_FIN					= 12,	/* Wait for an AckMsg to a FIN-ACK we sent. */
	TSS_WAIT_CLOSE						= 13,	/* Stream has sent a FIN-ACK and is waiting for a close message. */
	TSS_SEND_CLOSE						= 14,	/* Stream is closed for this end, but needs to send a close message. */
	TSS_CLOSED							= 15	/* Stream is closed. */
} TunnelStreamState;

typedef enum
{
	TSF_NONE					= 0x00,	/* None. */
	TSF_ACTIVE					= 0x01,	/* Application has not closed this tunnel stream. */
	TSF_NEED_FINAL_STATUS_EVENT	= 0x02,	/* Application closed this tunnel stream but still wants a final status event. */
	TSF_STARTED_CLOSE			= 0x04,	/* Closing of the tunnel stream was initiated by this end. */
	TSF_FIN_RECEIVED			= 0x08,	/* Received a FIN from the remote end. */
	TSF_SENT_ACK_OF_FIN			= 0x10,	/* Sent an ack the remote end's FIN. */
	TSF_RECEIVED_ACK_OF_FIN		= 0x20,	/* Received an ack for the FIN we sent. */
	TSF_SEND_NACK				= 0x40	/* Need to send a nack. */
} TunnelStreamFlags;

typedef struct
{
	RsslTunnelStream 					base;
	RsslQueueLink						_managerLink;
	RsslQueueLink						_managerOpenLink;
	RsslQueueLink						_dispatchLink;
	RsslQueueLink						_timeoutLink;
	RsslUInt32							_nameLength;
	RsslBool							_isNameAllocated;
	RsslBool							_hasNextDispatchTime;
	RsslInt64							_nextDispatchTime;
	TunnelStreamState					_state;
	RsslUInt8							_requestStateFlags;
	RsslUInt32							_flags;
	RsslTunnelStreamStatusEventCallback	*_statusEventCallback;
	RsslTunnelStreamQueueMsgCallback	*_queueMsgCallback;
	RsslTunnelStreamDefaultMsgCallback	*_defaultMsgCallback;
	RsslChannel*						_channel;
	TunnelManagerImpl*					_manager;
	RsslInt								_bytesWaitingAck;
	RsslQueue							_tunnelBufferTransmitList;
	RsslQueue							_tunnelBufferWaitAckList;
	RsslQueue							_tunnelBufferImmediateList;
	RsslQueue							_tunnelBufferTimeoutList;
	RsslInt64							_responseTimeout;

	RsslUInt32							_lastOutSeqNum;
	RsslUInt32							_lastInSeqNum;
	RsslUInt32							_lastInSeqNumAccepted;
	RsslUInt32							_lastInAckedSeqNum;
	RsslQueue							_substreams;
	RsslHashTable						_substreamsById;
	RsslBool							_persistLocally;
	RsslBool							_needsDispatch;
	RsslBool							_queuedFirstMsg;
	RsslBool							_interfaceError;
	RsslHashLink						_managerHashLink;
	RsslHashLink						_requestHashLink;
	RsslInt32							_authLoginStreamId;
	RsslBuffer							_memoryBuffer;
	char*								_persistenceFilePath;
	RsslRDMLoginRequest					*_pAuthLoginRequest;
	RsslBool							_authLoginRequestIsCopied;
	RsslInt64							_nextExpireTime;
	RsslInt64							_responseExpireTime;
	RsslUInt32							_retransRetryCount;	/* Number of retries attempted when sending certain messages. */
	BufferPool							_memoryBufferPool;
	RsslUInt32							_guaranteedOutputBuffersAppLimit;
	BigBufferPool						_bigBufferPool; /* big buffer pool for tunnel stream fragmentation */
	RsslUInt32							_requestRetryCount; /* for tunnel stream request retries when falling back to lesser stream versions */
	RsslUInt							_streamVersion; /* stream version of this tunnel stream */
	RsslHashTable						_fragmentationProgressHashTable; /* hash table indexed by message id for tracking fragmentation progress */
	RsslQueue							_fragmentationProgressQueue; /* queue for tracking fragmentation progress */
	RsslQueue							_pendingBigBufferList; /* pending big buffer list (holds big buffers that weren't fully written during submit) */
	RsslUInt16							_messageId; /* message id for fragmentation */
} TunnelStreamImpl;

RsslRet tunnelStreamEnqueueBuffer(RsslTunnelStream *pTunnelStream,
		RsslBuffer *pBuffer, RsslUInt8 containerType, RsslErrorInfo *pErrorInfo);

RsslRet tunnelStreamEnqueueBigBuffer(RsslTunnelStream *pTunnelStream,
		TunnelBufferImpl *pBufferImpl, RsslUInt8 containerType, RsslErrorInfo *pErrorInfo);

RTR_C_INLINE void tunnelStreamSetNeedsDispatch(TunnelStreamImpl *pTunnelImpl)
{
	assert(pTunnelImpl->_state != TSS_INACTIVE);
	if (!pTunnelImpl->_needsDispatch)
	{
		rsslQueueAddLinkToBack(&pTunnelImpl->_manager->_tunnelStreamDispatchList,
				&pTunnelImpl->_dispatchLink);
		pTunnelImpl->_needsDispatch = RSSL_TRUE;
		tunnelManagerSetNeedsDispatchNow(pTunnelImpl->_manager, RSSL_TRUE);
	}
}

RTR_C_INLINE void tunnelStreamUnsetNeedsDispatch(TunnelStreamImpl *pTunnelImpl)
{
	if (pTunnelImpl->_needsDispatch)
	{
		rsslQueueRemoveLink(&pTunnelImpl->_manager->_tunnelStreamDispatchList,
				&pTunnelImpl->_dispatchLink);
		pTunnelImpl->_needsDispatch = RSSL_FALSE;
	}
}

/* Updates the next time at which this tunnel stream can expire some buffers (if the new
 * timeout occurs before the existing one) */
void tunnelStreamSetNextExpireTime(TunnelStreamImpl *pTunnelImpl, RsslInt64 timeout);

/* Unset a tunnel stream's expire time. */
RTR_C_INLINE void tunnelStreamUnsetHasExpireTime(TunnelStreamImpl *pTunnelImpl);

/* Process time-related events, such as expiring any timed-out messages. */
RsslRet tunnelStreamProcessTimer(TunnelStreamImpl *pTunnelImpl, RsslInt64 currentTime,
		RsslErrorInfo *pErrorInfo);

/* Call when the tunnel stream is closed. */
RsslRet tunnelStreamHandleState(TunnelStreamImpl *pTunnelImpl, RsslState *pState, RsslMsg *pRsslMsg, 
		TunnelStreamMsg *pTunnelMsg, RsslTunnelStreamAuthInfo *pAuthInfo, RsslBool isInternalClose, RsslErrorInfo *pErrorInfo);

/* Used when errors are returned from tunnel stream functions. */
RsslRet tunnelStreamHandleError(TunnelStreamImpl *pTunnelImpl, RsslErrorInfo *pErrorInfo);

/* Opens a tunnel stream. */
RsslTunnelStream* tunnelStreamOpen(TunnelManager *pManager, RsslTunnelStreamOpenOptions *pOpts,
		RsslBool isProvider, RsslClassOfService *pRemoteCos, RsslUInt streamVersion, RsslErrorInfo *pErrorInfo);

/* Sets the channel for a tunnel stream. Set a NULL channel to indicate a
 * closed channel. */
RsslTunnelStream *tunnelStreamSetChannel(RsslTunnelStream *pTunnel,
		RsslChannel *pChannel);

/* Submit a message to the tunnel stream. 
 * Returns TunnelStreamReturnCodes. */
RsslRet tunnelStreamSubmitMsg(RsslTunnelStream *pTunnel, 
		RsslTunnelStreamSubmitMsgOptions *pOpts, RsslErrorInfo *pErrorInfo);

/* Submits a tunnel buffer. */
RsslRet tunnelStreamSubmitBuffer(RsslTunnelStream *pTunnel, RsslBuffer *pBuffer,
		RsslTunnelStreamSubmitOptions *pOptions, RsslErrorInfo *pErrorInfo);

/* Process a message received for a tunnel stream.
 * Returns TunnelStreamReturnCodes. */
RsslRet tunnelStreamRead(RsslTunnelStream *pTunnel, RsslMsg *pMsg, RsslErrorInfo *pErrorInfo);

/* Handles TunnelStream events.
 * Returns TunnelStreamReturnCodes. */
RsslRet tunnelStreamDispatch(RsslTunnelStream *pTunnel,
		RsslErrorInfo *pErrorInfo);

/* Calls the tunnel stream's defaultMsgCallback with the given message. */
RsslRet tunnelStreamCallMsgCallback(TunnelStreamImpl *pTunnelImpl, RsslBuffer *pBuffer, RsslMsg *pRsslMsg,
		RsslUInt8 containerType, RsslErrorInfo *pErrorInfo);

TunnelBufferImpl* tunnelStreamGetBuffer(
		TunnelStreamImpl *pTunnelImpl, RsslUInt32 length, RsslBool isAppBuffer, RsslBool isTranslationBuffer, RsslErrorInfo *pErrorInfo);

void tunnelStreamReleaseBuffer(
		TunnelStreamImpl *pTunnelImpl, TunnelBufferImpl *pBufferImpl);

/* Close a tunnel stream.
 * Returns TunnelStreamReturnCodes. */
RsslRet tunnelStreamClose(RsslTunnelStream *pTunnel, RsslTunnelStreamCloseOptions *pOptions, RsslErrorInfo *pErrorInfo);

/* Get a tunnel stream information.
 * Returns TunnelStreamReturnCodes. */
RsslRet tunnelStreamGetInfo(TunnelStreamImpl* pTunnelImpl, RsslTunnelStreamInfo *pInfo, RsslErrorInfo *pErrorInfo);

/* Adds a buffer with a timeout (used by substreams when sending queue messages). */
void tunnelStreamAddTimeoutBuffer(TunnelStreamImpl *pTunnelImpl, TunnelBufferImpl *pBufferImpl,
		RsslInt64 expireTime);

/* Adds a buffer with an immediate timeout (used by substreams when sending queue messages). */
void tunnelStreamAddImmediateTimeoutBuffer(TunnelStreamImpl *pTunnelImpl, 
		TunnelBufferImpl *pBufferImpl);

/* Returns the tunnel's next expire time. */
RTR_C_INLINE RsslInt64 tunnelStreamGetNextExpireTime(TunnelStreamImpl *pTunnelImpl);

/* Destroy a tunnel stream. */
void tunnelStreamDestroy(RsslTunnelStream *pTunnel);

RTR_C_INLINE void tunnelStreamStatusEventClear(RsslTunnelStreamStatusEvent *pEvent)
{
	memset(pEvent, 0, sizeof(RsslTunnelStreamStatusEvent));
}

RTR_C_INLINE void tunnelStreamMsgEventClear(RsslTunnelStreamMsgEvent *pEvent)
{
	memset(pEvent, 0, sizeof(RsslTunnelStreamMsgEvent));
}

RTR_C_INLINE void tunnelStreamQueueMsgEventClear(RsslTunnelStreamQueueMsgEvent *pEvent)
{
	memset(pEvent, 0, sizeof(RsslTunnelStreamQueueMsgEvent));
}

RTR_C_INLINE char *tunnelStreamGetPersistenceFilePath(TunnelStreamImpl *pTunnel)
{
	return pTunnel->_persistenceFilePath;
}

RTR_C_INLINE RsslBuffer *tunnelStreamGetMemoryHeapBuffer(TunnelStreamImpl *pTunnel)
{
	return &pTunnel->_memoryBuffer;
}

/* Inline function definitions. */

RTR_C_INLINE RsslInt64 tunnelStreamGetCurrentTimeMs(TunnelStreamImpl *pTunnel)
{
	return ((RsslReactorImpl*)pTunnel->_manager->_pParentReactor)->lastRecordedTimeMs;
}

RTR_C_INLINE void tunnelStreamUnsetHasExpireTime(TunnelStreamImpl *pTunnelImpl)
{
	if (pTunnelImpl->_nextExpireTime != RDM_QMSG_TC_INFINITE)
	{
		rsslQueueRemoveLink(&pTunnelImpl->_manager->_tunnelStreamTimeoutList,
				&pTunnelImpl->_timeoutLink);
		pTunnelImpl->_nextExpireTime = RDM_QMSG_TC_INFINITE;
	}
}

RTR_C_INLINE RsslInt64 tunnelStreamGetNextExpireTime(TunnelStreamImpl *pTunnelImpl)
{
	return pTunnelImpl->_nextExpireTime;
}

/* structure for tracking fragmentation progress */
typedef struct
{
	RsslHashLink		fragmentationHashLink;	/* Link for _fragmentationProgressHashTable by message id. */
	RsslQueueLink		fragmentationQueueLink;	/* Link for _fragmentationProgressQueue. */
	TunnelBufferImpl	*pBigBuffer;			/* Big buffer for re-assembling the fragmented message. */
	RsslUInt32			bytesAlreadyCopied;		/* Number of bytes already copied. */
} TunnelStreamFragmentationProgress;

RTR_C_INLINE void clearTunnelStreamFragmentationProgress(TunnelStreamFragmentationProgress *pFragmentationProgress)
{
	memset(pFragmentationProgress, 0, sizeof(TunnelStreamFragmentationProgress));
}

/* Saves the write progress for fragmentation. */
RTR_C_INLINE void saveWriteProgress(TunnelBufferImpl *pBufferImpl, RsslUInt32 totalMsgLength, RsslUInt32 bytesRemaining, RsslUInt32 lastFragmentId, RsslUInt16 messageId, RsslUInt8 containerType)
{
	pBufferImpl->_fragmentationInProgress = RSSL_TRUE;
	pBufferImpl->_totalMsgLength = totalMsgLength;
	pBufferImpl->_bytesRemainingToSend = bytesRemaining;
	pBufferImpl->_lastFragmentId = lastFragmentId;
	pBufferImpl->_messageId = messageId;
	pBufferImpl->_containerType = containerType;
}

#ifdef __cplusplus
};
#endif

#endif
