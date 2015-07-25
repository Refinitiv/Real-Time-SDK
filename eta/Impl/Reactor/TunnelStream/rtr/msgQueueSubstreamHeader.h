/*
 * This source code is provided under the Apache 2.0 license and is provided
 * AS IS with no warranty or guarantee of fit for purpose.  See the project's 
 * LICENSE.md for details. 
 * Copyright Thomson Reuters 2015. All rights reserved.
*/

#ifndef _RTR_MSGQUEUE_SUBSTREAM_HDR_H
#define _RTR_MSGQUEUE_SUBSTREAM_HDR_H

#ifdef __cplusplus
extern "C" {
#endif

#include "rtr/rsslDataTypeEnums.h"


typedef enum {
	MSGQUEUE_SHO_INIT			= 0,
	MSGQUEUE_SHO_DATA			= 1,
	MSGQUEUE_SHO_ACK			= 2,
	MSGQUEUE_SHO_REQUEST		= 3,
	MSGQUEUE_SHO_DEAD_LETTER	= 4,
	MSGQUEUE_SHO_REFRESH		= 5
} MsgQueueSubstreamHeaderOpcodes;

typedef struct {
	MsgQueueSubstreamHeaderOpcodes	opcode;
	RsslInt32						streamId;
	RsslUInt8						domainType;	
} MsgQueueSubstreamHeaderBase;

typedef struct {
	MsgQueueSubstreamHeaderBase	base;
	RsslBuffer					fromQueue;	
	RsslUInt32					lastOutSeqNum;
	RsslUInt32					lastInSeqNum;
} MsgQueueSubstreamRequestHeader;

RTR_C_INLINE void msgQueueClearSubstreamRequestHeader(MsgQueueSubstreamRequestHeader *requestHeader)
{
	requestHeader->base.opcode = MSGQUEUE_SHO_REQUEST;
}

RTR_C_INLINE int msgQueueSubstreamRequestHeaderBufferSize(MsgQueueSubstreamRequestHeader *requestHeader)
{
	return 128 + requestHeader->fromQueue.length;
}

typedef struct {
	MsgQueueSubstreamHeaderBase	base;
	RsslUInt32					lastOutSeqNum;
	RsslUInt32					lastInSeqNum;
	RsslUInt16					queueDepth;
} MsgQueueSubstreamRefreshHeader;

RTR_C_INLINE void msgQueueClearSubstreamRefreshHeader(MsgQueueSubstreamRequestHeader *requestHeader)
{
	requestHeader->base.opcode = MSGQUEUE_SHO_REFRESH;
}

typedef struct {
	MsgQueueSubstreamHeaderBase	base;
	RsslUInt16					flags;
	RsslBuffer					fromQueue;	
	RsslBuffer					toQueue;	
	RsslUInt32					seqNum;
	RsslInt64					timeout;
	RsslBuffer					enclosedBuffer;
	RsslUInt8					containerType;
	RsslInt64					identifier;
	RsslUInt8					undeliverableCode;
	RsslUInt16					queueDepth;
} MsgQueueSubstreamDataHeader;

typedef enum
{
	MSGQUEUE_DF_NONE				= 0x00,
	MSGQUEUE_DF_POSSIBLE_DUPLICATE	= 0x01
} MsgQueueSubstreamDataHeaderFlags;


RTR_C_INLINE void msgQueueClearSubstreamDataHeader(MsgQueueSubstreamDataHeader *dataHeader)
{
	dataHeader->base.opcode = MSGQUEUE_SHO_DATA;
}

RTR_C_INLINE int msgQueueSubstreamDataHeaderBufferSize(MsgQueueSubstreamDataHeader *dataHeader)
{
	return 128 + dataHeader->fromQueue.length + dataHeader->toQueue.length
		+ (dataHeader->enclosedBuffer.data ? dataHeader->enclosedBuffer.length : 0);
}

typedef struct {
	MsgQueueSubstreamHeaderBase	base;
	RsslBuffer					fromQueue;	
	RsslBuffer					toQueue;	
	RsslUInt32					seqNum;
	RsslInt64					identifier;
} MsgQueueSubstreamAckHeader;

RTR_C_INLINE void msgQueueClearSubstreamAckHeader(MsgQueueSubstreamAckHeader *ackHeader)
{
	ackHeader->base.opcode = MSGQUEUE_SHO_ACK;
}

RTR_C_INLINE int msgQueueSubstreamAckHeaderBufferSize(MsgQueueSubstreamAckHeader *ackHeader)
{
	return 128 + ackHeader->fromQueue.length + ackHeader->toQueue.length;
}

typedef union {
	MsgQueueSubstreamHeaderBase		base;
	MsgQueueSubstreamRequestHeader	requestHeader;
	MsgQueueSubstreamRefreshHeader	refreshHeader;
	MsgQueueSubstreamDataHeader		dataHeader;
	MsgQueueSubstreamAckHeader		ackHeader;
} MsgQueueSubstreamHeader;

#ifdef __cplusplus
}
#endif

#endif 
