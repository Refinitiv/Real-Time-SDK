/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2019 Refinitiv. All rights reserved.            --
 *|-----------------------------------------------------------------------------
 */

#ifndef __RTR_SEQ_MCAST_TRANSPORT_IMPL_H
#define __RTR_SEQ_MCAST_TRANSPORT_IMPL_H

/* Contains function declarations necessary for the
 * Sequence Multicast transport 
 */

#ifdef __cplusplus
extern "C" {
#endif

#include "rtr/rsslTypes.h"
#include "rtr/rsslChanManagement.h"
#include "rtr/rwfNet.h"
#include "rtr/rwfNetwork.h"
#include <stdio.h>

#define 	RSSL_RSSL_SEQ_MCAST_IMPL_FAST(ret)		ret RTR_FASTCALL

// Contains code necessary for creating a Sequence Multicast network 
RsslRet rsslSeqMcastBind(rsslServerImpl* rsslSrvrImpl, RsslBindOptions *opts, RsslError *error );

// Contains code necessary for accepting inbound Sequence Multicast connections to a Sequence Multicast network
rsslChannelImpl* rsslSeqMcastAccept(rsslServerImpl *rsslSrvrImpl, RsslAcceptOptions *opts, RsslError *error);

// Contains code necessary for connecting to a Sequence Multicast network
RsslRet rsslSeqMcastConnect(rsslChannelImpl* rsslChnlImpl, RsslConnectOptions *opts, RsslError *error);

// Contains code necessary to reconnect Sequence Multicast connections and bridge data flow (no-op)
RsslRet rsslSeqMcastReconnect(rsslChannelImpl *rsslChnlImpl, RsslError *error);

// Contains code necessary for Sequence Multicast connections (client or server side) to perform and complete 
// handshake process (no-op)
RsslRet rsslSeqMcastInitChannel(rsslChannelImpl* rsslChnlImpl, RsslInProgInfo *inProg, RsslError *error);

// Contains code necessary to disconnect from a Sequence Multicast network (client or server side)
RsslRet rsslSeqMcastCloseChannel(rsslChannelImpl* rsslChnlImpl, RsslError *error);

// Contains code necessary to read from a Sequence Multicast network (client only)
RSSL_RSSL_SEQ_MCAST_IMPL_FAST(RsslBuffer*) rsslSeqMcastRead(rsslChannelImpl* rsslChnlImpl, RsslReadOutArgs *readOutArgs, RsslRet *readRet, RsslError *error);

// Contains code necessary to write data going to a Sequence Multicast network (server side only)
RSSL_RSSL_SEQ_MCAST_IMPL_FAST(RsslRet) rsslSeqMcastWrite(rsslChannelImpl *rsslChnlImpl, rsslBufferImpl *rsslBufImpl, RsslWriteInArgs *writeInArgs, RsslWriteOutArgs *writeOutArgs, RsslError *error);

// Contains code necessary to flush data to Sequence Multicast network (no-op)
RSSL_RSSL_SEQ_MCAST_IMPL_FAST(RsslRet) rsslSeqMcastFlush(rsslChannelImpl *rsslChnlImpl, RsslError *error);

// Contains code necessary to obtain a buffer to put data in for writing to Sequence Multicast network (server side only)
RSSL_RSSL_SEQ_MCAST_IMPL_FAST(rsslBufferImpl*) rsslSeqMcastGetBuffer(rsslChannelImpl *rsslChnlImpl, RsslUInt32 size, RsslBool packedBuffer, RsslError *error);

// Contains code necessary to release an unused/unsuccessfully written buffer to Sequence Multicast network (server side only)
RSSL_RSSL_SEQ_MCAST_IMPL_FAST(RsslRet) rsslSeqMcastReleaseBuffer(rsslChannelImpl *rsslChnlImpl, rsslBufferImpl *rsslBufImpl, RsslError *error);

// Contains code necessary to query number of used output buffers for Sequence Multicast connection (no-op)
RSSL_RSSL_SEQ_MCAST_IMPL_FAST(RsslInt32) rsslSeqMcastBufferUsage(rsslChannelImpl *rsslChnlImpl, RsslError *error);

// Contains code necessary to query number of used buffers by the server (shared pool buffers typically (no-op)
RSSL_RSSL_SEQ_MCAST_IMPL_FAST(RsslInt32) rsslSeqMcastSrvrBufferUsage(rsslServerImpl *rsslSrvrImpl, RsslError *error);

// Contains code necessary for buffer packing with Sequence Multicast connection buffer (server side)
RSSL_RSSL_SEQ_MCAST_IMPL_FAST(RsslBuffer*) rsslSeqMcastPackBuffer(rsslChannelImpl *rsslChnlImpl, rsslBufferImpl *rsslBufImpl, RsslError *error);

// Contains code necessary to send a ping message (no-op)
RSSL_RSSL_SEQ_MCAST_IMPL_FAST(RsslRet) rsslSeqMcastPing(rsslChannelImpl *rsslChnlImpl, RsslError *error);

// Contains code necessary to query Sequence Multicast channel for more detailed connection info (client or server side)
RSSL_RSSL_SEQ_MCAST_IMPL_FAST(RsslRet) rsslSeqMcastGetChannelInfo(rsslChannelImpl *rsslChnlImpl, RsslChannelInfo *info, RsslError *error);

// Contains code necessary to query Sequence Multicast server for more detailed connection info (server only)
RSSL_RSSL_SEQ_MCAST_IMPL_FAST(RsslRet) rsslSeqMcastGetSrvrInfo(rsslServerImpl *rsslSrvrImpl, RsslServerInfo *info, RsslError *error);

// Contains code necessary to do an ioctl on a Sequence Multicast client (no-op)
RSSL_RSSL_SEQ_MCAST_IMPL_FAST(RsslRet) rsslSeqMcastIoctl(rsslChannelImpl *rsslChnlImpl, RsslIoctlCodes code, void *value, RsslError *error);

// Contains code necessary to do an ioctl on a Sequence Multicast server (no-op)
RSSL_RSSL_SEQ_MCAST_IMPL_FAST(RsslRet) rsslSeqMcastSrvrIoctl(rsslServerImpl *rsslSrvrImpl, RsslIoctlCodes code, void *value, RsslError *error);

// Contains code necessary to set the debug func pointers for Sequence Multicast transport
RsslRet rsslSetSeqMcastDebugFunctions(
	void(*dumpRsslIn)(const char *functionName, char *buffer, RsslUInt32 length, RsslSocket socketId),
	void(*dumpRsslOut)(const char *functionName, char *buffer, RsslUInt32 length, RsslSocket socketId),
	RsslError *error);

#ifdef __cplusplus
};
#endif


#endif
