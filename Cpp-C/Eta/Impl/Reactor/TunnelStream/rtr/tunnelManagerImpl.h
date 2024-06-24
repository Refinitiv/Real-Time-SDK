/*
 * This source code is provided under the Apache 2.0 license and is provided
 * AS IS with no warranty or guarantee of fit for purpose.  See the project's 
 * LICENSE.md for details. 
 * Copyright (C) 2024 LSEG. All rights reserved.
*/

#ifndef TUNNEL_MANAGER_IMPL_H
#define TUNNEL_MANAGER_IMPL_H

#include "rtr/tunnelManager.h"
#include "rtr/tunnelSubstream.h"
#include "rtr/rsslHashTable.h"
#include "rtr/bufferPool.h"
#include "rtr/persistFile.h"
#include <stdlib.h>

static const RsslUInt32 TS_HEADER_MAX_LENGTH = 128;

#ifdef __cplusplus
extern "C" {
#endif

typedef struct
{
	TunnelManager						base;
	RsslQueue							_tunnelStreams;
	RsslQueue							_tunnelStreamsOpen;
	RsslQueue							_tunnelStreamDispatchList;
	RsslQueue							_tunnelStreamTimeoutList;
	RsslQueue							_tunnelBufferPool;
	RsslHashTable 						_streamIdToTunnelStreamTable;
	RsslReactor							*_pParentReactor;
	RsslInt64							_nextExpireTime;
	RsslTunnelStreamListenerCallback	*_listenerCallback;
	RsslTunnelStream					*_pCurrentTunnel;
} TunnelManagerImpl;

typedef enum
{
	TSR_WAIT_ACCEPT = 0,
	TSR_ACCEPTED = 1,
	TSR_REJECTED = 2
} NewTunnelStreamRequestState;

typedef struct
{
	RsslTunnelStreamRequestEvent	event;				/* The event passed to the application. */
	RsslUInt8						state;				/* State of accept/reject -- see NewTunnelStreamRequestState. */
	RsslRequestMsg					*pReqMsg;			/* Associated RsslRequestMsg. */
	RsslBool						eventCosDecoded;	/* Whether the classOfService has been decoded. */
	RsslClassOfService				classOfService;		/* Class of service for this request. Only valid if eventCosDecoded is true. */
	RsslUInt						streamVersion;		/* stream version of this request */
} NewTunnelStreamRequest;

/* Retrieve a channel buffer from the TunnelManager's channel. */
RsslBuffer* tunnelManagerGetChannelBuffer(TunnelManagerImpl *pManagerImpl,
		RsslEncodeIterator *pIter, RsslUInt32 length, RsslBool mustSendMsg, RsslErrorInfo *pErrorInfo);

RTR_C_INLINE void tunnelManagerSetCurrentTunnel(TunnelManagerImpl *pManagerImpl, RsslTunnelStream *pTunnel)
{
	pManagerImpl->_pCurrentTunnel = pTunnel;
}

RTR_C_INLINE void tunnelManagerMarkIfCurrentTunnelDestroyed(TunnelManagerImpl *pManagerImpl, RsslTunnelStream *pTunnel)
{
	if (pTunnel == pManagerImpl->_pCurrentTunnel)
		pManagerImpl->_pCurrentTunnel = NULL;
}

RTR_C_INLINE RsslBool tunnelManagerCheckIfCurrentTunnelDestroyed(TunnelManagerImpl *pManagerImpl)
{
	return (pManagerImpl->_pCurrentTunnel == NULL);
}


/* Writes a buffer to the TunnelManager channel. */
RsslRet tunnelManagerSubmitChannelBuffer(TunnelManagerImpl *pManagerImpl,
		RsslBuffer *pBuffer, RsslErrorInfo *pErrorInfo);

RTR_C_INLINE void tunnelManagerSetNeedsDispatchNow(
		TunnelManagerImpl *pManagerImpl,
		RsslBool needsDispatchNow)
{
	pManagerImpl->base._needsDispatchNow = needsDispatchNow;
}

RTR_C_INLINE void tunnelManagerSetNextDispatchTime(TunnelManagerImpl *pManagerImpl, RsslInt64 expireTime)
{
	if (pManagerImpl->base._nextDispatchTime == RDM_QMSG_TC_INFINITE
			|| expireTime < pManagerImpl->base._nextDispatchTime)
		pManagerImpl->base._nextDispatchTime = expireTime;
}

#ifdef __cplusplus
};
#endif

#endif
