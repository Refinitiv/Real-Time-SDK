/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright Thomson Reuters 2018. All rights reserved.            --
 *|-----------------------------------------------------------------------------
 */

#ifndef __RTR_RSSL_UNIDIRECTION_SHMEM_TRANSPORT_IMPL_H
#define __RTR_RSSL_UNIDIRECTION_SHMEM_TRANSPORT_IMPL_H

/* Contains function declarations necessary for the
 * unidirectional shared memory connection type 
 * (typically used for Tier 0 shared memory connection type).
 */

#ifdef __cplusplus
extern "C" {
#endif

#include "rtr/rsslTypes.h"
#include "rtr/rsslChanManagement.h"
#include "rtr/shmemtrans.h"
#include "rtr/rwfNet.h"
#include "rtr/rwfNetwork.h"
#include <stdio.h>

#define 	RSSL_RSSL_UNIDIRECTION_SHMEM_IMPL_FAST(ret)		ret RTR_FASTCALL

/* Contains code necessary for creating a shared memory segment */
RsslRet rsslUniShMemBind(rsslServerImpl* rsslSrvrImpl, RsslBindOptions *opts, RsslError *error );

/* Contains code necessary for accepting inbound shared memory connections to a shm segment */
rsslChannelImpl* rsslUniShMemAccept(rsslServerImpl *rsslSrvrImpl, RsslAcceptOptions *opts, RsslError *error);

/* Contains code necessary for connecting to a shared memory segment */
RsslRet rsslUniShMemConnect(rsslChannelImpl* rsslChnlImpl, RsslConnectOptions *opts, RsslError *error);

/* Contains code necessary to reconnect shmem connections and bridge data flow (no-op) */
RsslRet rsslUniShMemReconnect(rsslChannelImpl *rsslChnlImpl, RsslError *error);

/* Contains code necessary for shared memory connections (client or server side) to perform and complete handshake process (no-op) */
RsslRet rsslUniShMemInitChannel(rsslChannelImpl* rsslChnlImpl, RsslInProgInfo *inProg, RsslError *error);

/* Contains code necessary to disconnect from a shared memory segment (client or server side) */
RsslRet rsslUniShMemCloseChannel(rsslChannelImpl* rsslChnlImpl, RsslError *error);

/* Contains code necessary to read from a shared memory segment (client only) */
RSSL_RSSL_UNIDIRECTION_SHMEM_IMPL_FAST(RsslBuffer*) rsslUniShMemRead(rsslChannelImpl* rsslChnlImpl, RsslReadOutArgs *readOutArgs, RsslRet *readRet, RsslError *error);

/* Contains code necessary to write data going to a shared memory segment (server side only) */
RSSL_RSSL_UNIDIRECTION_SHMEM_IMPL_FAST(RsslRet) rsslUniShMemWrite(rsslChannelImpl *rsslChnlImpl, rsslBufferImpl *rsslBufImpl, RsslWriteInArgs *writeInArgs, RsslWriteOutArgs *writeOutArgs, RsslError *error);

/* Contains code necessary to flush data to shared memory segment (no-op) */
RSSL_RSSL_UNIDIRECTION_SHMEM_IMPL_FAST(RsslRet) rsslUniShMemFlush(rsslChannelImpl *rsslChnlImpl, RsslError *error);

/* Contains code necessary to obtain a buffer to put data in for writing to shared memory segment (server side only) */
RSSL_RSSL_UNIDIRECTION_SHMEM_IMPL_FAST(rsslBufferImpl*) rsslUniShMemGetBuffer(rsslChannelImpl *rsslChnlImpl, RsslUInt32 size, RsslBool packedBuffer, RsslError *error);

/* Contains code necessary to release an unused/unsuccessfully written buffer to shmem segments (server side only) */
RSSL_RSSL_UNIDIRECTION_SHMEM_IMPL_FAST(RsslRet) rsslUniShMemReleaseBuffer(rsslChannelImpl *rsslChnlImpl, rsslBufferImpl *rsslBufImpl, RsslError *error);

/* Contains code necessary to query number of used output buffers for shared memory connection (no-op) */
RSSL_RSSL_UNIDIRECTION_SHMEM_IMPL_FAST(RsslInt32) rsslUniShMemBufferUsage(rsslChannelImpl *rsslChnlImpl, RsslError *error);

/* Contains code necessary to query number of used buffers by the server (shared pool buffers typically (no-op) */
RSSL_RSSL_UNIDIRECTION_SHMEM_IMPL_FAST(RsslInt32) rsslUniShMemSrvrBufferUsage(rsslServerImpl *rsslSrvrImpl, RsslError *error);

/* Contains code necessary for buffer packing with shared memory connection buffer (server side) */
RSSL_RSSL_UNIDIRECTION_SHMEM_IMPL_FAST(RsslBuffer*) rsslUniShMemPackBuffer(rsslChannelImpl *rsslChnlImpl, rsslBufferImpl *rsslBufImpl, RsslError *error);

/* Contains code necessary to send a ping message (no-op) */
RSSL_RSSL_UNIDIRECTION_SHMEM_IMPL_FAST(RsslRet) rsslUniShMemPing(rsslChannelImpl *rsslChnlImpl, RsslError *error);

/* Contains code necessary to query shared memory channel for more detailed connection info (client or server side) */
RSSL_RSSL_UNIDIRECTION_SHMEM_IMPL_FAST(RsslRet) rsslUniShMemGetChannelInfo(rsslChannelImpl *rsslChnlImpl, RsslChannelInfo *info, RsslError *error);

/* Contains code necessary to query shared memory server for more detailed connection info (server only) */
RSSL_RSSL_UNIDIRECTION_SHMEM_IMPL_FAST(RsslRet) rsslUniShMemGetSrvrInfo(rsslServerImpl *rsslSrvrImpl, RsslServerInfo *info, RsslError *error);

/* Contains code necessary to do an ioctl on a shmem client (no-op) */
RSSL_RSSL_UNIDIRECTION_SHMEM_IMPL_FAST(RsslRet) rsslUniShMemIoctl(rsslChannelImpl *rsslChnlImpl, RsslIoctlCodes code, void *value, RsslError *error);

/* Contains code necessary to do an ioctl on a shmem server (no-op) */
RSSL_RSSL_UNIDIRECTION_SHMEM_IMPL_FAST(RsslRet) rsslUniShMemSrvrIoctl(rsslServerImpl *rsslSrvrImpl, RsslIoctlCodes code, void *value, RsslError *error);

// Contains code necessary to set the debug func pointers for UniShMem transport
RsslRet rsslSetUniShMemDebugFunctions(
	void(*dumpRsslIn)(const char *functionName, char *buffer, RsslUInt32 length, RsslSocket socketId),
	void(*dumpRsslOut)(const char *functionName, char *buffer, RsslUInt32 length, RsslSocket socketId),
	RsslError *error);

#ifdef __cplusplus
};
#endif


#endif
