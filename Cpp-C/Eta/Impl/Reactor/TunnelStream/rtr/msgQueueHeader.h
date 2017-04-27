/*
 * This source code is provided under the Apache 2.0 license and is provided
 * AS IS with no warranty or guarantee of fit for purpose.  See the project's 
 * LICENSE.md for details. 
 * Copyright Thomson Reuters 2015. All rights reserved.
*/

#ifndef _RTR_MSGQUEUE_HDR_H
#define _RTR_MSGQUEUE_HDR_H

#ifdef __cplusplus
extern "C" {
#endif

#include "rtr/rsslDataTypeEnums.h"
#include "rtr/rsslClassOfService.h"


typedef enum {
	TS_MC_INIT		= 0,
	TS_MC_DATA		= 1,
	TS_MC_ACK		= 2,
	TS_MC_RETRANS	= 3,
	TS_MC_REQUEST	= 4,
	TS_MC_REFRESH	= 5,
	TS_MC_STATUS	= 6,
	TS_MC_CLOSE		= 7
} TunnelStreamMsgOpcodes;

typedef enum {
	TS_DF_NONE			= 0x00,
	TS_DF_FRAGMENTED	= 0x01
} TunnelStreamDataFlags;

#define MAX_RANGES 255

typedef struct {
	RsslUInt32	count;
	RsslUInt32	rangeArray[MAX_RANGES * 2];
} AckRangeList;

typedef struct {
	RsslUInt8				domainType;	
	RsslInt32				streamId;
	TunnelStreamMsgOpcodes	opcode;
} TunnelStreamMsgBase;

typedef struct {
	TunnelStreamMsgBase	base;
	RsslUInt16			serviceId;
	RsslBuffer			name;	
} TunnelStreamRequest;

RTR_C_INLINE void tunnelStreamRequestClear(TunnelStreamRequest *requestHeader)
{
	memset(requestHeader, 0, sizeof(TunnelStreamRequest));
	requestHeader->base.opcode = TS_MC_REQUEST;
}

RTR_C_INLINE int tunnelStreamRequestBufferSize(TunnelStreamRequest *requestHeader)
{
	return 512 + requestHeader->name.length;
}

typedef enum
{
	TS_RFMF_NONE			= 0x0,
	TS_RFMF_HAS_NAME		= 0x1,
	TS_RFMF_HAS_SERVICE_ID	= 0x2
} TunnelStreamRefreshFlags;

typedef struct {
	TunnelStreamMsgBase	base;
	RsslUInt8			flags;
	RsslUInt16			serviceId;
	RsslBuffer			name;
	RsslState			state;
	RsslUInt8			containerType;
	RsslBuffer			encDataBody;
} TunnelStreamRefresh;

RTR_C_INLINE void tunnelStreamRefreshClear(TunnelStreamRefresh *pRefresh)
{
	memset(pRefresh, 0, sizeof(TunnelStreamRefresh));
	pRefresh->base.opcode = TS_MC_REFRESH;
	pRefresh->containerType = RSSL_DT_FILTER_LIST;
}

RTR_C_INLINE int tunnelStreamRefreshBufferSize(TunnelStreamRefresh *pRefresh)
{
	return 512 + ((pRefresh->flags & TS_RFMF_HAS_NAME) ? pRefresh->name.length : 0)
		+ ((pRefresh->state.text.data != NULL) ? pRefresh->state.text.length : 0);
}

typedef enum
{
	TS_STMF_NONE		= 0x0,
	TS_STMF_HAS_STATE	= 0x1
} TunnelStreamStatusFlags;

typedef struct {
	TunnelStreamMsgBase	base;
	RsslUInt8 			flags;
	RsslUInt16			serviceId;
	RsslBuffer			name;
	RsslState			state;
} TunnelStreamStatus;

RTR_C_INLINE void tunnelStreamStatusClear(TunnelStreamStatus *pStatus)
{
	memset(pStatus, 0, sizeof(TunnelStreamStatus));
	pStatus->base.opcode = TS_MC_STATUS;
}

RTR_C_INLINE int tunnelStreamStatusBufferSize(TunnelStreamRefresh *pStatus)
{
	return 64 + ((pStatus->flags & TS_STMF_HAS_STATE && pStatus->state.text.data != NULL) ? 
		pStatus->state.text.length : 0);
}

typedef struct {
	TunnelStreamMsgBase	base;
	RsslUInt32			seqNum;
	RsslUInt8 			flags;
	RsslUInt32			totalMsgLength;
	RsslUInt32			fragmentNumber;
	RsslUInt16			messageId;
	RsslUInt8 			containerType;
	RsslBool			msgComplete;
} TunnelStreamData;

RTR_C_INLINE void tunnelStreamDataClear(TunnelStreamData *dataHeader)
{
	memset(dataHeader, 0, sizeof(TunnelStreamData));
	dataHeader->base.opcode = TS_MC_DATA;
}

RTR_C_INLINE int tunnelStreamDataMsgBufferSize(TunnelStreamData *dataHeader)
{
	return 128;
}

RTR_C_INLINE void tunnelStreamRetransClear(TunnelStreamData *dataHeader)
{
	memset(dataHeader, 0, sizeof(TunnelStreamData));
	dataHeader->base.opcode = TS_MC_RETRANS;
}

typedef enum {
	TS_ACKF_NONE		= 0x0,
	TS_ACKF_FIN			= 0x1
} TunnelStreamAckFlags;

typedef struct {
	TunnelStreamMsgBase	base;
	RsslUInt32			seqNum;
	RsslInt32			recvWindow;
	RsslUInt16			flags;
} TunnelStreamAck;

RTR_C_INLINE void tunnelStreamAckClear(TunnelStreamAck *ackHeader)
{
	memset(ackHeader, 0, sizeof(TunnelStreamAck));
	ackHeader->base.opcode = TS_MC_ACK;
}

RTR_C_INLINE int tunnelStreamAckBufferSize(TunnelStreamAck *ackHeader, AckRangeList *ackRangeList, AckRangeList *nakRangeList)
{
	return 128 + 
		((ackRangeList != NULL) ? ackRangeList->count * 2 * 4 : 0)
		+ ((nakRangeList != NULL) ? nakRangeList->count * 2 * 4 : 0);
}

typedef union {
	TunnelStreamMsgBase	base;
	TunnelStreamRequest	requestHeader;
	TunnelStreamRefresh	refreshHeader;
	TunnelStreamStatus	statusHeader;
	TunnelStreamData	dataHeader;
	TunnelStreamAck		ackHeader;
} TunnelStreamMsg;

RTR_C_INLINE void tunnelStreamMsgClear(TunnelStreamMsg *pMsg)
{
	memset(pMsg, 0, sizeof(TunnelStreamMsg));
	pMsg->base.opcode = TS_MC_INIT;
}

#ifdef __cplusplus
}
#endif

#endif 
