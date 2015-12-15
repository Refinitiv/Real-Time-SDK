/*
 * This source code is provided under the Apache 2.0 license and is provided
 * AS IS with no warranty or guarantee of fit for purpose.  See the project's 
 * LICENSE.md for details. 
 * Copyright Thomson Reuters 2015. All rights reserved.
*/

#ifndef TUNNEL_SUBSTREAM_H
#define TUNNEL_SUBSTREAM_H

#include "rtr/rsslTunnelStream.h"
#include "rtr/rsslQueue.h"
#include "rtr/rsslHashTable.h"
#include "rtr/persistFile.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum
{
	SBS_NOT_OPEN		= 0,
	SBS_WAIT_REFRESH	= 1,
	SBS_OPEN			= 2,
	SBS_CLOSED			= 3
} TunnelSubstreamState;

typedef struct
{
	RsslHashLink			_tunnelTableLink;	/* Link for tunnel stream hash table by Id. */
	RsslQueueLink			_tunnelQueueLink;	/* Link for tunnel stream queue. */
	RsslInt32				_streamId;			/* Stream ID. */
	RsslUInt8				_domainType;		/* Domain type. */
	RsslBuffer				_sourceName;		/* Name of the queue stream. */
	TunnelSubstreamState	_state;				/* Current internal state of the queue stream. */
} TunnelSubstream;

/* Opens a substream. */
TunnelSubstream *tunnelSubstreamOpen(RsslTunnelStream  *pTunnel,
		RsslRDMQueueRequest *pRequestMsg, RsslErrorInfo *pErrorInfo);

typedef struct
{
	RsslRDMMsg *pRDMMsg;
} TunnelSubstreamSubmitOptions;

RTR_C_INLINE void tunnelSubstreamSubmitOptionsClear(TunnelSubstreamSubmitOptions *pOpts)
{
	memset(pOpts, 0, sizeof(TunnelSubstreamSubmitOptions));
}

/* Submit a message to the substream. */
RsslRet tunnelSubstreamSubmit(TunnelSubstream *pSubstream,
		TunnelSubstreamSubmitOptions *pOptions,
		RsslErrorInfo *pErrorInfo);

/* Read a message intended for the substream. */
RsslRet tunnelSubstreamRead(TunnelSubstream *pSubstream,
		RsslMsg *pMsg, RsslErrorInfo *pErrorInfo);

/* Removes buffers whose timeout has expired and calls back the application. */
RsslRet tunnelSubstreamExpireBuffer(TunnelSubstream *pSubstream, RsslBuffer *pTunnelBuffer,
		PersistentMsg *pPersistentMsg, RsslUInt8 undeliverableCode, RsslErrorInfo *pErrorInfo);

/* Updates a buffer for transmission (ensures persistence is updated and updates any timeout */
RsslRet	tunnelSubstreamUpdateMsgForTransmit(TunnelSubstream *pSubstream, RsslBuffer *pBuffer, RsslErrorInfo *pErrorInfo);

/* Closes a substream. */
RsslRet tunnelSubstreamClose(TunnelSubstream *pSubstream,
		RsslErrorInfo *pErrorInfo);

/* Cleans up a substream. */
void tunnelSubstreamDestroy(TunnelSubstream *pSubstream);

/* Returns the substream's Stream ID. */
RTR_C_INLINE RsslInt32 tunnelSubstreamGetStreamId(TunnelSubstream *pSubstream)
{
	return pSubstream->_streamId;
}

/* Sets substream to closed since the tunnel stream was closed; so that it doesn't
 * try to send a close. */
RTR_C_INLINE void tunnelSubstreamSetTunnelClosed(TunnelSubstream *pSubstream)
{
	pSubstream->_state = SBS_CLOSED;
}


#ifdef __cplusplus
};
#endif

#endif
