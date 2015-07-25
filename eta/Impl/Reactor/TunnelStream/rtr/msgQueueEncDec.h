/*
 * This source code is provided under the Apache 2.0 license and is provided
 * AS IS with no warranty or guarantee of fit for purpose.  See the project's 
 * LICENSE.md for details. 
 * Copyright Thomson Reuters 2015. All rights reserved.
*/

#ifndef __RSSL_MSGQUEUE_ENCDEC_H
#define __RSSL_MSGQUEUE_ENCDEC_H

#include "rtr/rsslDataTypeEnums.h"
#include "rtr/rsslMsgEncoders.h"
#include "rtr/rsslRetCodes.h"
#include "rtr/rsslIterators.h"
#include "msgQueueHeader.h"
#include "msgQueueSubstreamHeader.h"
#include "rtr/rsslMsg.h"
#include "rtr/rsslRDMQueueMsg.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Current tunnel stream protocol version. */
static const RsslUInt COS_STREAM_VERSION = 1;

/* Encodes a tunnel stream request. */
RsslRet tunnelStreamRequestEncode(RsslEncodeIterator *pIter, TunnelStreamRequest *requestHeader, RsslClassOfService *pCos, RsslErrorInfo *pErrorInfo);

/* Populates the RsslRequestMsg to send a tunnel stream request. */
void tunnelStreamRequestSetRsslMsg(TunnelStreamRequest *requestHeader, RsslRequestMsg *oRsslMsg, RsslClassOfService *pCos);

/* Encodes a tunnel stream refresh. */
RsslRet tunnelStreamRefreshEncode(RsslEncodeIterator *pIter, TunnelStreamRefresh *pTunnelRefresh, RsslClassOfService *pCos, RsslErrorInfo *pErrorInfo);

/* Populates the RsslRefreshMsg to send a tunnel stream refresh. */
void tunnelStreamRefreshSetRsslMsg(TunnelStreamRefresh *pTunnelRefresh, RsslRefreshMsg *oRsslMsg, RsslClassOfService *pCos);

/* Encodes a tunnel stream status. */
RsslRet tunnelStreamStatusEncode(RsslEncodeIterator *pIter, TunnelStreamStatus *pTunnelStatus, RsslClassOfService *pCos, RsslErrorInfo *pErrorInfo);

/* Populates the RsslStatusMsg to send a tunnel stream status. */
void tunnelStreamStatusSetRsslMsg(TunnelStreamStatus *pTunnelStatus, RsslStatusMsg *oRsslMsg, RsslClassOfService *pCos);

/* Encodes a tunnel stream data message header. */
RsslRet tunnelStreamDataEncodeInit(RsslEncodeIterator *pIter, TunnelStreamData *dataHeader);

/* Replaces the opcode of an already-encoded data header. Intended for setting the retransmit code. */
RsslRet tunnelStreamDataReplaceOpcode(RsslEncodeIterator *pIter, RsslUInt8 opcode);

/* Encodes a tunnel stream ack message. */
RsslRet tunnelStreamAckEncode(RsslEncodeIterator *pIter, TunnelStreamAck *ackHeader, AckRangeList *ackRangeList, AckRangeList *nakRangeList);

/* Decodes a tunnel stream message. */
RsslRet tunnelStreamMsgDecode(RsslMsg *pMsg, TunnelStreamMsg *pTunnelMsg, AckRangeList *ackRangeList, AckRangeList *nakRangeList);

RsslRet rsslEncodeMsgQueueSubstreamRequestHeader(RsslEncodeIterator *pIter, MsgQueueSubstreamRequestHeader *requestHeader);

RsslRet rsslEncodeMsgQueueSubstreamDataHeader(RsslEncodeIterator *pIter, MsgQueueSubstreamDataHeader *dataHeader);

RsslRet rsslEncodeMsgQueueSubstreamAckHeader(RsslEncodeIterator *pIter, MsgQueueSubstreamAckHeader *ackHeader);

RsslRet decodeSubstreamHeader(RsslDecodeIterator *pIter, RsslMsg *pMsg, MsgQueueSubstreamHeader *pSubstreamHeader);

RsslRet decodeSubstreamRequestHeader(RsslDecodeIterator *pIter, RsslMsg *pMsg, MsgQueueSubstreamRequestHeader *pSubstreamRequestHeader);

/* Changes a data message to appear as a dead-letter message, and
 * populates the given RsslMsg. Used for local message expiry. 
 * Uses the memory in pExtHeaderBuffer to create a correct extended header. */
RsslRet substreamSetRsslMsgFromQueueDataExpiredMsg( RsslRDMQueueDataExpired *iQueueDataExpired, 
		RsslGenericMsg *oGenericMsg, RsslBuffer *pExtHeaderBuffer);

/* Encodes a locally-generated QueueAck, and
 * populates the given RsslMsg. Used for local message expiry. 
 * Uses the memory in pExtHeaderBuffer to create a correct extended header. */
RsslRet substreamSetRsslMsgFromQueueAckMsg( RsslRDMQueueAck *iQueueAck, 
		RsslGenericMsg *oGenericMsg, 
		RsslUInt32 seqNum, RsslBuffer *pExtHeaderBuffer);

/* Replaces the timeout of a QueueData message (if the value is less than the current value). */
RsslRet rsslReplaceQueueDataTimeout(RsslDecodeIterator *pIter, RsslInt64 timeout);

/* Adds the Possible-duplicate flag to a message. */
RsslRet rsslAddQueueDataDuplicateFlag(RsslDecodeIterator *pIter);

/* Encodes a class of service filter list. */
RsslRet rsslEncodeClassOfService(RsslEncodeIterator *pIter, RsslClassOfService *pCos, RsslUInt32 filter,
		 RsslBool isProvider, RsslErrorInfo *pErrorInfo);

/* Retrieves the stream version from a ClassOfService filter list. */
RsslRet rsslGetClassOfServiceStreamVersion(RsslDecodeIterator *pIter, RsslUInt *pStreamVersion,
		RsslErrorInfo *pErrorInfo);

/* Decodes a class of service filter list. */
RsslRet rsslDecodeClassOfService(RsslDecodeIterator *pIter, RsslClassOfService *pCos,
		RsslUInt *pStreamVersion, RsslErrorInfo *pErrorInfo);

#ifdef __cplusplus
}
#endif

#endif 
