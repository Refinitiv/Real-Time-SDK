#ifndef TUNNEL_MSG_H
#define TUNNEL_MSG_H

#include "rtr/rsslMsgEncoders.h"
#include "rtr/rsslErrorInfo.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum
{
	TMC_UNKNOWN		= 0,
	TMC_DATA		= 1,
	TMC_ACK			= 2,
	TMC_RETRANS		= 3,
	TMC_REQUEST		= 4,
	TMC_REFRESH		= 5
} TunnelMsgClass;

typedef struct
{
	RsslUInt8	msgClass;
	RsslInt32	streamId;
	RsslUInt8	domainType;
} TunnelMsgBase;

RTR_C_INLINE void tunnelMsgBaseClear(TunnelMsgBase *pMsg)
{
	pMsg->msgClass = TMC_UNKNOWN;
	pMsg->streamId = 0;
	pMsg->domainType = 0;
}


typedef struct
{
	TunnelMsgBase	base;
	RsslBuffer		name;
	RsslUInt16		serviceId;
} TunnelRequestMsg;

/* Clear a TunnelRequestMsg clear */
RTR_C_INLINE void tunnelRequestMsgClear(TunnelRequestMsg *pMsg)
{
	tunnelMsgBaseClear(&pMsg->base);
	rsslClearBuffer(&pMsg->name);
	pMsg->serviceId = 0;
}

/* Get an estimated encoding length of a TunnelRequestMsg. */
RTR_C_INLINE RsslUInt32 tunnelRequestMsgGetEncodeLength(TunnelRequestMsg *pMsg)
{
	return 128 + pMsg->name.length;
}

/* Encode a TunnelRequestMsg. */
RsslRet rsslEncodeTunnelRequestMsg(RsslEncodeIterator *pIter,
		TunnelRequestMsg *pRequestMsg);

typedef struct
{
	TunnelMsgBase	base;
} TunnelRefreshMsg;

typedef struct
{
	TunnelMsgBase	base;
} TunnelAckMsg;

typedef struct
{
	TunnelMsgBase	base;
} TunnelDataMsg;


typedef union
{
	TunnelMsgBase		base;
	TunnelRequestMsg	requestMsg;
	TunnelRefreshMsg	refreshMsg;
	TunnelAckMsg		ackMsg;
} TunnelMsg;

RsslRet tunnelRequestMsgEncode(RsslEncodeIterator *pIter, TunnelRequestMsg *pRequestMsg);

#ifdef __cplusplus
};
#endif

#endif
