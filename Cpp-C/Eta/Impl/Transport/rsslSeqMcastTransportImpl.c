/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2019 Refinitiv. All rights reserved.            --
 *|-----------------------------------------------------------------------------
 */

#include "rtr/rsslSeqMcastTransportImpl.h"
#include "rtr/rsslSeqMcastTransport.h"
#include "rtr/rsslAlloc.h"
#include "rtr/rsslErrors.h"
#include "rtr/retmacros.h"
#include "decodeRoutines.h"
#include "xmlDump.h"
#include <time.h>

#include <sys/timeb.h>
#if defined(_WIN32) || defined(WIN32)
#include <process.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#define snprintf _snprintf
#define getpid _getpid
#else
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fcntl.h>
#endif

#if defined(_WIN16) || defined(_WIN32)
#undef ETIMEDOUT
#undef EISCONN
#undef EADDRNOTAVAIL
#undef EADDRINUSE
#undef ECONNREFUSED
#undef EWOULDBLOCK
#undef EINPROGRESS
#define ETIMEDOUT               WSAETIMEDOUT
#define EISCONN                 WSAEISCONN
#define EADDRNOTAVAIL           WSAEADDRNOTAVAIL
#define EADDRINUSE              WSAEADDRINUSE
#define ECONNREFUSED            WSAECONNREFUSED
#define EWOULDBLOCK             WSAEWOULDBLOCK
#define EINPROGRESS             WSAEINPROGRESS

#ifdef  errno
#undef  errno
#endif
#define errno	h_errno		/* WSAGetLastError() */

#include        <sys/types.h>           

#define ioctl           ioctlsocket
#define sock_close      closesocket
#define getpid          _getpid

#else

#define sock_close	close       /* for compatibility */

#endif

#define SEQ_MCAST_MAX_VERSION 0x01

#define SEQ_MCAST_SEQUENCE_NUM_LEN 4
#define SEQ_MCAST_MESSAGE_LENGTH_LEN 2
#define SEQ_MCAST_VERSION_LEN 1
#define SEQ_MCAST_FLAGS_LEN 1
#define SEQ_MCAST_PROTOCOL_TYPE_LEN 1
#define SEQ_MCAST_HDR_LENGTH_LEN 1
#define SEQ_MCAST_PROTO_VER_MAJOR_LEN 1
#define SEQ_MCAST_PROTO_VER_MINOR_LEN 1
#define SEQ_MCAST_INSTANCE_ID_LEN 2

/* Maximum length of the header, including any optional members, including the length of the first message */
#define SEQ_MCAST_MAX_HDR_LEN 14

/* Minimum length of the header, excluding any optional members, but also including the 1st message. */
#define SEQ_MCAST_MIN_HDR_LEN 14

/* Ping length is the total number of bytes in the header, minus the message length. */
#define SEQ_MCAST_PING_LEN 12

typedef struct
{
	RsslMutex			lock;
	RsslSocket			sendSock;
	RsslUInt32			maxMsgSize;
	RsslUInt16			instanceId;
	RsslBool			bufferInUse;
	char*				bufferMem;
	RsslBuffer			inputBuffer;
	char*				inputBufferMem;
	RsslInt32			processedPacketLen;
	RsslUInt32			readSeqNum;
	RsslUInt16			readInstanceID;				/* Instance ID for the current packet */
	RsslBool			stillProcessingPacket;
	RsslBuffer			readBuffer;
	RsslUInt32			writeSeqNum;
	RsslUInt32			readAddr;
	RsslUInt16			readPort;
	RsslUInt64			pktRecvCount;
	RsslUInt64			pktSentCount;
	struct sockaddr_in	sendAddr;
	struct sockaddr_in	recvAddr;
	rtrSeqMcastBuffer	writeBuffer;
} RsslSeqMcastChannel;


/* global debug function pointers */
static void(*rsslSeqMcastDumpInFunc)(const char *functionName, char *buffer, RsslUInt32 length, RsslSocket socketId) = 0;
static void(*rsslSeqMcastDumpOutFunc)(const char *functionName, char *buffer, RsslUInt32 length, RsslSocket socketId) = 0;

/* The list of the Sequence Multicast transport debug functions' entry points index by the protocol type. */
static RsslDumpFuncs rsslSeqMcastDumpFuncs[MAX_PROTOCOL_TYPES];

RTR_C_INLINE void rsslSeqMcastDumpInFuncImpl(const char* functionName, char* buffer, RsslUInt32 length, RsslSocket socketId, RsslChannel* pChannel);
RTR_C_INLINE void rsslSeqMcastDumpOutFuncImpl(const char* functionName, char* buffer, RsslUInt32 length, RsslSocket socketId, RsslChannel* pChannel);

/* lock information */
static RsslBool chnlLocking;

/* initialize lock */
RTR_C_ALWAYS_INLINE void seqMcastInitLock(RsslMutex *seqMcastLock)
{
  (void) RSSL_MUTEX_INIT_RTSDK(seqMcastLock);
}

/* destroy lock */
RTR_C_ALWAYS_INLINE void seqMcastDestroyLock(RsslMutex *seqMcastLock)
{
  (void) RSSL_MUTEX_DESTROY(seqMcastLock);
}

/* get lock */
RTR_C_ALWAYS_INLINE void seqMcastGetLock(RsslMutex *seqMcastLock)
{
  (void) RSSL_MUTEX_LOCK(seqMcastLock);
}

/* release lock */
RTR_C_ALWAYS_INLINE void seqMcastUnlock(RsslMutex *seqMcastLock)
{
  (void) RSSL_MUTEX_UNLOCK(seqMcastLock);
}

/* try lock */
RTR_C_ALWAYS_INLINE int seqMcastTrylock(RsslMutex *seqMcastLock)
{
	return RSSL_MUTEX_TRYLOCK(seqMcastLock);
}


/* Write the Sequence Multicast Header to the channel impl.  
	Preconditions: buf's space has been pre-allocated, and everything's been range checked to ensure that no overflow is possible
	
	Returns: offset value from beginning of buffer indicating where the header begins in the buffer.
			NOTE: As of version 1, this should always be 0.  If/when the wire is extended to include 
			more optional members, this may be a positive value as more space is reserved for optional members.
			
			returns -1 on failure, with the error populated. */

RTR_C_ALWAYS_INLINE RsslInt32 rsslSeqMcastWriteHdr(rsslChannelImpl *rsslChnlImpl, char* buf, RsslUInt8 flags, RsslUInt32 seqNum, RsslError *error)
{
	RsslSeqMcastChannel *pSeqMcastChannel = (RsslSeqMcastChannel*)rsslChnlImpl->transportInfo;
	char* iter;

	/* Should check flags and offet here.  Don't need to because there aren't any optional members, so we start at 0 */
	iter = buf;

	/* Put Sequence Multicast Protocol Version here */
	iter += rwfPut8(iter, SEQ_MCAST_MAX_VERSION);

	iter += rwfPut8(iter, flags);
	
	/* payload protocol type */
	iter += rwfPut8(iter, (RsslUInt8)rsslChnlImpl->Channel.protocolType);

	/* Sequence Multicast Protocol header length.  Since we need all of the remaining ones, it's a static 12 bytes */
	iter += rwfPut8(iter, SEQ_MCAST_HDR_LENGTH_LEN + SEQ_MCAST_FLAGS_LEN + SEQ_MCAST_PROTOCOL_TYPE_LEN
			+ SEQ_MCAST_HDR_LENGTH_LEN + SEQ_MCAST_INSTANCE_ID_LEN + SEQ_MCAST_PROTO_VER_MAJOR_LEN + SEQ_MCAST_PROTO_VER_MINOR_LEN + SEQ_MCAST_SEQUENCE_NUM_LEN);

	/* Instance Id, provided by the channel */
	iter += rwfPut16(iter, pSeqMcastChannel->instanceId);

	/* major protocol version */
	iter += rwfPut8(iter, (RsslUInt8)rsslChnlImpl->Channel.majorVersion);

	/* minor protocol version */
	iter += rwfPut8(iter, (RsslUInt8)rsslChnlImpl->Channel.minorVersion);

	/* Sequence Number */
	iter += rwfPut32(iter, seqNum);

	return 0;
}

/* Contains code necessary for accepting inbound Sequence Multicast connections to a Sequence Multicast network */
/* Not implemented */
rsslChannelImpl* rsslSeqMcastAccept(rsslServerImpl *rsslSrvrImpl, RsslAcceptOptions *opts, RsslError *error)
{
	_rsslSetError(error, NULL, RSSL_RET_FAILURE,  0);
	snprintf(error->text, MAX_RSSL_ERROR_TEXT,  "<%s:%d> rsslAccept() Error: 0006 rsslAccept is not implemented for the Sequenced Multicast transport.\n", __FILE__, __LINE__);

	return NULL;
}

/* Contains code necessary for creating a Sequence Multicast network */
/* Not implemented */
RsslRet rsslSeqMcastBind(rsslServerImpl* rsslSrvrImpl, RsslBindOptions *opts, RsslError *error )
{
	_rsslSetError(error, NULL, RSSL_RET_FAILURE,  0);
	snprintf(error->text, MAX_RSSL_ERROR_TEXT, "<%s:%d> rsslBind() Error: 0006 rsslBind is not implemented for the Sequenced Multicast transport.\n", __FILE__, __LINE__);

	return RSSL_RET_FAILURE;
}

/* Contains code necessary to query number of used output buffers for Sequence Multicast connection (no-op) */
RSSL_RSSL_SEQ_MCAST_IMPL_FAST(RsslInt32) rsslSeqMcastBufferUsage(rsslChannelImpl *rsslChnlImpl, RsslError *error)
{
	RsslSeqMcastChannel *pSeqMcastChannel = (RsslSeqMcastChannel*)rsslChnlImpl->transportInfo;
	RsslBool bufferInUse = 0;

	if (chnlLocking)
		seqMcastGetLock(&pSeqMcastChannel->lock);
	if (pSeqMcastChannel && pSeqMcastChannel->bufferInUse)
	{
		bufferInUse = 1;
	}
	if (chnlLocking)
		seqMcastUnlock(&pSeqMcastChannel->lock);

	return bufferInUse;
}

/* Contains code necessary to disconnect from a Sequence Multicast network (client or server side) */
RsslRet rsslSeqMcastCloseChannel(rsslChannelImpl* rsslChnlImpl, RsslError *error)
{
	RsslSeqMcastChannel *pSeqMcastChannel = (RsslSeqMcastChannel*)rsslChnlImpl->transportInfo;
	rsslBufferImpl *rsslBufImpl = NULL;
	RsslQueueLink *pLink = NULL;

	if (chnlLocking)
		seqMcastGetLock(&pSeqMcastChannel->lock);

	sock_close(rsslChnlImpl->Channel.socketId);

	rsslChnlImpl->Channel.state = RSSL_CH_STATE_INACTIVE;

	/* cleanup rsslChnlImpl->activeBufferList and rsslChnlImpl->freeBufferList */
	pLink = rsslQueuePeekFront(&(rsslChnlImpl->activeBufferList));
	if (pLink != NULL)
	{
		rsslQueueRemoveLink(&(rsslChnlImpl->activeBufferList), pLink);
		rsslInitQueueLink(pLink);
		rsslQueueAddLinkToBack(&(rsslChnlImpl->freeBufferList), pLink);
	}

	_rsslFree(pSeqMcastChannel->bufferMem);
	pSeqMcastChannel->bufferMem = 0;
	_rsslFree(pSeqMcastChannel->inputBufferMem);
	pSeqMcastChannel->inputBufferMem = 0;
	if (chnlLocking)
		seqMcastUnlock(&pSeqMcastChannel->lock);
	if (chnlLocking)
		seqMcastDestroyLock(&pSeqMcastChannel->lock);
	_rsslFree(rsslChnlImpl->transportInfo);
	rsslChnlImpl->transportInfo = 0;

	return RSSL_RET_SUCCESS;
}

/* Contains code necessary for connecting to a Sequence Multicast network */
RsslRet rsslSeqMcastConnect(rsslChannelImpl* rsslChnlImpl, RsslConnectOptions *opts, RsslError *error)
{
	RsslSocket socketId;
	struct ip_mreq mreg;
    RsslInt32 reuse=1;
	RsslSeqMcastChannel *pSeqMcastChannel = NULL;
	RsslUInt32 addr;
#ifdef _WIN32
	RsslUInt32 ifaddr;
#else
	struct in_addr ifaddr;
#endif

	/* Create transport info. */
	if (!(pSeqMcastChannel = (RsslSeqMcastChannel*)_rsslMalloc(sizeof(RsslSeqMcastChannel))))
	{
		_rsslSetError(error, NULL, RSSL_RET_FAILURE,  0);
		snprintf(error->text, MAX_RSSL_ERROR_TEXT, "<%s:%d> rsslConnect() Error: 0005 Failed to allocate the sequenced multicast channel structure.\n", __FILE__, __LINE__);
		return RSSL_RET_FAILURE;
	}
	
	if (chnlLocking)
		seqMcastInitLock(&pSeqMcastChannel->lock);

	if (opts->seqMulticastOpts.maxMsgSize > 65493)
		pSeqMcastChannel->maxMsgSize = 65493;
	else
		pSeqMcastChannel->maxMsgSize = opts->seqMulticastOpts.maxMsgSize;

	/* Add 7 to avoid any full word byte swap issues at the end of the buffer */
	if (!(pSeqMcastChannel->bufferMem = (char*)_rsslMalloc(opts->seqMulticastOpts.maxMsgSize + SEQ_MCAST_MAX_HDR_LEN + 7)))
	{
		_rsslSetError(error, NULL, RSSL_RET_FAILURE,  0);
		snprintf(error->text, MAX_RSSL_ERROR_TEXT, "<%s:%d> rsslConnect() Error: 0005 Failed to allocate the sequenced multicast output buffer.\n", __FILE__, __LINE__);
		_rsslFree(pSeqMcastChannel);
		return RSSL_RET_FAILURE;
	}
	if (!(pSeqMcastChannel->inputBufferMem = (char*)_rsslMalloc(opts->seqMulticastOpts.maxMsgSize + SEQ_MCAST_MAX_HDR_LEN + 7)))
	{
		_rsslSetError(error, NULL, RSSL_RET_FAILURE,  0);
		snprintf(error->text, MAX_RSSL_ERROR_TEXT, "<%s:%d> rsslConnect() Error: 0005 Failed to allocate the sequenced multicast input buffer.\n", __FILE__, __LINE__);
		_rsslFree(pSeqMcastChannel->bufferMem);
		_rsslFree(pSeqMcastChannel);
		return RSSL_RET_FAILURE;
	}

	rsslChnlImpl->transportInfo = pSeqMcastChannel;
	
	pSeqMcastChannel->bufferInUse = RSSL_FALSE;
	pSeqMcastChannel->processedPacketLen = 0;
	pSeqMcastChannel->readSeqNum = 0;
	pSeqMcastChannel->stillProcessingPacket = RSSL_FALSE;
	pSeqMcastChannel->writeSeqNum = 0;
	pSeqMcastChannel->pktRecvCount = 0;
	pSeqMcastChannel->pktSentCount = 0;
	pSeqMcastChannel->instanceId = opts->seqMulticastOpts.instanceId;
	memset(&pSeqMcastChannel->sendAddr, 0, sizeof(pSeqMcastChannel->sendAddr));
	memset(&pSeqMcastChannel->recvAddr, 0, sizeof(pSeqMcastChannel->recvAddr));
	memset(&pSeqMcastChannel->writeBuffer, 0, sizeof(pSeqMcastChannel->writeBuffer));

	if (opts->connectionInfo.unified.address == NULL)
	{
		_rsslFree(pSeqMcastChannel->inputBufferMem);
		_rsslFree(pSeqMcastChannel->bufferMem);
		_rsslFree(pSeqMcastChannel);
		_rsslSetError(error, NULL, RSSL_RET_FAILURE, 0);
		snprintf(error->text, MAX_RSSL_ERROR_TEXT, "<%s:%d> rsslConnect() Error: 0013 unified.address not provided.\n", __FILE__, __LINE__);
		return RSSL_RET_FAILURE;
	}

	if (opts->connectionInfo.unified.serviceName == NULL)
	{
		_rsslFree(pSeqMcastChannel->inputBufferMem);
		_rsslFree(pSeqMcastChannel->bufferMem);
		_rsslFree(pSeqMcastChannel);
		_rsslSetError(error, NULL, RSSL_RET_FAILURE, 0);
		snprintf(error->text, MAX_RSSL_ERROR_TEXT, "<%s:%d> rsslConnect() Error: 0013 unified.serviceName not provided.\n", __FILE__, __LINE__);
		return RSSL_RET_FAILURE;
	}

	/* create socket */
	if ((socketId = (RsslSocket)socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1)
	{
		_rsslSetError(error, NULL, RSSL_RET_FAILURE, errno);
		snprintf(error->text, MAX_RSSL_ERROR_TEXT, "<%s:%d> rsslConnect() Error: 1002 Call to socket() failed. System errno: (%d).\n", __FILE__, __LINE__, errno);
		_rsslFree(pSeqMcastChannel->inputBufferMem);
		_rsslFree(pSeqMcastChannel->bufferMem);
		_rsslFree(pSeqMcastChannel);
		return RSSL_RET_FAILURE;
	}

	if (opts->sysSendBufSize)
	{
		if (setsockopt(socketId, SOL_SOCKET, SO_SNDBUF, (char *)&opts->sysSendBufSize, sizeof(opts->sysSendBufSize)) < 0)
		{
			sock_close(socketId);

			_rsslSetError(error, NULL, RSSL_RET_FAILURE, errno);
			snprintf(error->text, MAX_RSSL_ERROR_TEXT, "<%s:%d> rsslConnect() Error: 1002 setsockopt() failed. Unable to set SO_SNDBUF on socket. System errno: (%d).\n", __FILE__, __LINE__, errno);
			_rsslFree(pSeqMcastChannel->inputBufferMem);
			_rsslFree(pSeqMcastChannel->bufferMem);
			_rsslFree(pSeqMcastChannel);
			return RSSL_RET_FAILURE;
		}
	}

	if (opts->sysRecvBufSize)
	{
		if (setsockopt(socketId, SOL_SOCKET, SO_RCVBUF, (char *)&opts->sysRecvBufSize, sizeof(opts->sysRecvBufSize)) < 0)
		{
			sock_close(socketId);

			_rsslSetError(error, NULL, RSSL_RET_FAILURE, errno);
			snprintf(error->text, MAX_RSSL_ERROR_TEXT, "<%s:%d> rsslConnect() Error: 1002 setsockopt() failed. Unable to set SO_RCVBUF on socket. System errno: (%d).\n", __FILE__, __LINE__, errno);
			_rsslFree(pSeqMcastChannel->inputBufferMem);
			_rsslFree(pSeqMcastChannel->bufferMem);
			_rsslFree(pSeqMcastChannel);
			return RSSL_RET_FAILURE;
		}
	}

	/* Enable SO_REUSEADDR to allow multiple instances of application to receive multicast datagrams. */
	/* This allows multiple apps to bind to same address/port. */
	/* If we use this without disable loopback and use same send and receive address/port,
	   we receive all packets we send. If we disable loopback, we cannot test send and
	   receive on same machine. */
    if (setsockopt(socketId, SOL_SOCKET, SO_REUSEADDR, (char *)&reuse, sizeof(reuse)) < 0)
	{
		sock_close(socketId);

		_rsslSetError(error, NULL, RSSL_RET_FAILURE, errno);
		snprintf(error->text, MAX_RSSL_ERROR_TEXT, "<%s:%d> rsslConnect() Error: 1002 setsockopt() failed. Unable to set SO_REUSEADDR on socket. System errno: (%d).\n", __FILE__, __LINE__, errno);
		_rsslFree(pSeqMcastChannel->inputBufferMem);
		_rsslFree(pSeqMcastChannel->bufferMem);
		_rsslFree(pSeqMcastChannel);
		return RSSL_RET_FAILURE;
    }

	pSeqMcastChannel->recvAddr.sin_family = AF_INET;

	/* Linux: Bind required, needs to be the connection address or INADDR_ANY */
	/* Windows: Looks like bind to interface works */

#ifdef _WIN32
	if (opts->connectionInfo.segmented.interfaceName && opts->connectionInfo.segmented.interfaceName[0] != 0)
	{
		if (rsslGetHostByName(opts->connectionInfo.segmented.interfaceName, &addr) < 0)
		{
			sock_close(socketId);

			_rsslSetError(error, NULL, RSSL_RET_FAILURE, errno);
			snprintf(error->text, MAX_RSSL_ERROR_TEXT, "<%s:%d> rsslConnect() Error 1004: getHostByName() failed.  Interface name (%s) is incorrect.  System errno: (%d)\n", __FILE__, __LINE__, opts->connectionInfo.segmented.interfaceName, errno);
			_rsslFree(pSeqMcastChannel->inputBufferMem);
			_rsslFree(pSeqMcastChannel->bufferMem);
			_rsslFree(pSeqMcastChannel);
			
			return RSSL_RET_FAILURE;
		} 
		pSeqMcastChannel->recvAddr.sin_addr.s_addr = addr;
	}
	else /* use default interface */
	{
		pSeqMcastChannel->recvAddr.sin_addr.s_addr = INADDR_ANY;
	}
#else
/* Linux requires the socket to be bound to the remote address */
	if (rsslGetHostByName(opts->connectionInfo.segmented.recvAddress, &addr) < 0)
	{
		sock_close(socketId);

		_rsslSetError(error, NULL, RSSL_RET_FAILURE, errno);
		snprintf(error->text, MAX_RSSL_ERROR_TEXT,"<%s:%d> rsslConnect() Error 1004: getHostByName() failed.  Receive address (%s) is incorrect. System errno: (%d)\n", __FILE__, __LINE__, opts->connectionInfo.segmented.recvAddress, errno);
		_rsslFree(pSeqMcastChannel->inputBufferMem);
		_rsslFree(pSeqMcastChannel->bufferMem);
		_rsslFree(pSeqMcastChannel);
		return RSSL_RET_FAILURE;
	}
	pSeqMcastChannel->recvAddr.sin_addr.s_addr = addr;
#endif


	pSeqMcastChannel->recvAddr.sin_port = rsslGetServByName(opts->connectionInfo.segmented.recvServiceName);

	if(pSeqMcastChannel->recvAddr.sin_port == 0)
	{
		sock_close(socketId);

		_rsslSetError(error, NULL, RSSL_RET_FAILURE, errno);
		snprintf(error->text, MAX_RSSL_ERROR_TEXT, "<%s:%d> rsslConnect() Error: 1004 getServByName() failed.  Receive service (%s) is incorrect.  System errno: (%d)\n", __FILE__, __LINE__, opts->connectionInfo.segmented.recvServiceName, errno);
		_rsslFree(pSeqMcastChannel->inputBufferMem);
		_rsslFree(pSeqMcastChannel->bufferMem);
		_rsslFree(pSeqMcastChannel);
		return RSSL_RET_FAILURE;
	} 

	if (bind(socketId, (struct sockaddr *)&pSeqMcastChannel->recvAddr, sizeof(pSeqMcastChannel->recvAddr)) < 0)
	{
		sock_close(socketId);

		_rsslSetError(error, NULL, RSSL_RET_FAILURE, errno);
		snprintf(error->text, MAX_RSSL_ERROR_TEXT, "<%s:%d> rsslConnect() Error: 1002 Call to system bind() failed. System errno: (%d).\n", __FILE__, __LINE__, errno);
		_rsslFree(pSeqMcastChannel->inputBufferMem);
		_rsslFree(pSeqMcastChannel->bufferMem);
		_rsslFree(pSeqMcastChannel);
		return RSSL_RET_FAILURE;
	} 

	/* join multicast group */

	if (rsslGetHostByName(opts->connectionInfo.segmented.recvAddress, &addr) < 0 )
	{
		sock_close(socketId);

		_rsslSetError(error, NULL, RSSL_RET_FAILURE, errno);
		snprintf(error->text, MAX_RSSL_ERROR_TEXT, "<%s:%d> rsslConnect() Error: 1004 getHostByName() failed.  Receive address (%s) is incorrect.  System errno: (%d).\n", __FILE__, __LINE__, opts->connectionInfo.segmented.recvAddress, errno);
		_rsslFree(pSeqMcastChannel->inputBufferMem);
		_rsslFree(pSeqMcastChannel->bufferMem);
		_rsslFree(pSeqMcastChannel);
		return RSSL_RET_FAILURE;
	}
	mreg.imr_multiaddr.s_addr = addr;

	if (opts->connectionInfo.segmented.interfaceName && opts->connectionInfo.segmented.interfaceName[0] != 0)
	{
		if (rsslGetHostByName(opts->connectionInfo.segmented.interfaceName, &addr) < 0)
		{
			sock_close(socketId);

			_rsslSetError(error, NULL, RSSL_RET_FAILURE, errno);
			snprintf(error->text, MAX_RSSL_ERROR_TEXT, "<%s:%d> rsslConnect() Error: 1004 getHostByName() failed.  Interface address (%s) is incorrect.  System errno: (%d).\n", __FILE__, __LINE__, opts->connectionInfo.segmented.interfaceName, errno);
			_rsslFree(pSeqMcastChannel->inputBufferMem);
			_rsslFree(pSeqMcastChannel->bufferMem);
			_rsslFree(pSeqMcastChannel);
			return RSSL_RET_FAILURE;
		}
		mreg.imr_interface.s_addr = addr;
#ifdef WIN32
		ifaddr = addr;
#else
		ifaddr.s_addr = addr;
#endif
	}
	else /* use default interface */
	{
		mreg.imr_interface.s_addr = INADDR_ANY;
#ifdef WIN32
		ifaddr = INADDR_ANY;
#else
		ifaddr.s_addr = INADDR_ANY;
#endif
	}

	if (setsockopt(socketId, IPPROTO_IP, IP_ADD_MEMBERSHIP, (char *)&mreg, sizeof(struct ip_mreq)) < 0)
	{
		sock_close(socketId);

		_rsslSetError(error, NULL, RSSL_RET_FAILURE, errno);
		snprintf(error->text, MAX_RSSL_ERROR_TEXT, "<%s:%d> rsslConnect() Error: 1002 setsockopt() failed.  Unable to add membership to multicast group.  System errno: (%d).\n", __FILE__, __LINE__, errno);
		_rsslFree(pSeqMcastChannel->inputBufferMem);
		_rsslFree(pSeqMcastChannel->bufferMem);
		_rsslFree(pSeqMcastChannel);
		return RSSL_RET_FAILURE;
	}
	
	if (setsockopt(socketId, IPPROTO_IP, IP_MULTICAST_IF, (char *)&ifaddr, sizeof(ifaddr)) < 0)
	{
		sock_close(socketId);

		_rsslSetError(error, NULL, RSSL_RET_FAILURE, errno);
		snprintf(error->text, MAX_RSSL_ERROR_TEXT, "<%s:%d> rsslConnect() Error: 1002 setsockopt() failed.  Unable to set multicast interface.  System errno: (%d).\n", __FILE__, __LINE__, errno);
		_rsslFree(pSeqMcastChannel->inputBufferMem);
		_rsslFree(pSeqMcastChannel->bufferMem);
		_rsslFree(pSeqMcastChannel);
		return RSSL_RET_FAILURE;
	}

	if (!opts->blocking)
	{
#if _WIN32
		unsigned long arg = 1;
		if (ioctlsocket(socketId, FIONBIO, &arg ) == SOCKET_ERROR)
#else
		if (fcntl(socketId, F_SETFL, O_NONBLOCK) < 0)
#endif
		{
			sock_close(socketId);

			_rsslSetError(error, NULL, RSSL_RET_FAILURE,  errno);
			snprintf(error->text, MAX_RSSL_ERROR_TEXT, "<%s:%d> rsslConnect() Error: 1002 fcntl() failed.  Unable to set blocking.  System errno: (%d).\n", __FILE__, __LINE__, errno);
			_rsslFree(pSeqMcastChannel->inputBufferMem);
			_rsslFree(pSeqMcastChannel->bufferMem);
			_rsslFree(pSeqMcastChannel);
			return RSSL_RET_FAILURE;
		}
	}

	/* save send information for writing later */
	pSeqMcastChannel->sendAddr.sin_family = AF_INET;
	if (opts->connectionInfo.segmented.sendAddress)
	{
		if (rsslGetHostByName(opts->connectionInfo.segmented.sendAddress, &addr) < 0)
		{
			sock_close(socketId);

			_rsslSetError(error, NULL, RSSL_RET_FAILURE, errno);
			snprintf(error->text, MAX_RSSL_ERROR_TEXT, "<%s:%d> rsslConnect() Error: 1004 getHostByName() failed.  Send address (%s) is incorrect.  System errno: (%d).\n", __FILE__, __LINE__, opts->connectionInfo.segmented.sendAddress, errno);
			_rsslFree(pSeqMcastChannel->inputBufferMem);
			_rsslFree(pSeqMcastChannel->bufferMem);
			_rsslFree(pSeqMcastChannel);
			return RSSL_RET_FAILURE;
		}
		pSeqMcastChannel->sendAddr.sin_addr.s_addr = addr;
		pSeqMcastChannel->sendAddr.sin_port = rsslGetServByName(opts->connectionInfo.segmented.sendServiceName);
	}
	else /* use recv info id send not populated */
	{
		if (rsslGetHostByName(opts->connectionInfo.segmented.recvAddress, &addr) < 0)
		{
			sock_close(socketId);

			_rsslSetError(error, NULL, RSSL_RET_FAILURE, errno);
			snprintf(error->text, MAX_RSSL_ERROR_TEXT, "<%s:%d> rsslConnect() Error: 1004 getHostByName() failed.  Receive address (%s) is incorrect.  System errno: (%d).\n", __FILE__, __LINE__, opts->connectionInfo.segmented.recvAddress, errno);
			_rsslFree(pSeqMcastChannel->inputBufferMem);
			_rsslFree(pSeqMcastChannel->bufferMem);
			_rsslFree(pSeqMcastChannel);
			return RSSL_RET_FAILURE;
		}
		pSeqMcastChannel->sendAddr.sin_addr.s_addr = addr;
		pSeqMcastChannel->sendAddr.sin_port = rsslGetServByName(opts->connectionInfo.segmented.recvServiceName);
	}

	/* Update Channel information */
	rsslChnlImpl->Channel.socketId = socketId;
	rsslChnlImpl->Channel.state = RSSL_CH_STATE_ACTIVE;
	rsslChnlImpl->Channel.connectionType = RSSL_CONN_TYPE_SEQ_MCAST;
	rsslChnlImpl->Channel.pingTimeout = opts->pingTimeout;
	rsslChnlImpl->Channel.majorVersion = opts->majorVersion;
	rsslChnlImpl->Channel.minorVersion = opts->minorVersion;
	rsslChnlImpl->Channel.protocolType = opts->protocolType;
	rsslChnlImpl->Channel.userSpecPtr = opts->userSpecPtr;
	
	return RSSL_RET_SUCCESS;
}

/* Contains code necessary to flush data to Sequence Multicast network (no-op) */
RSSL_RSSL_SEQ_MCAST_IMPL_FAST(RsslRet) rsslSeqMcastFlush(rsslChannelImpl *rsslChnlImpl, RsslError *error)
{
	return RSSL_RET_SUCCESS;
}

/* Contains code necessary to obtain a buffer to put data in for writing to Sequence Multicast network */
/* Right now, all buffers will be packable, since there is 0 difference on the wire */
RSSL_RSSL_SEQ_MCAST_IMPL_FAST(rsslBufferImpl*) rsslSeqMcastGetBuffer(rsslChannelImpl *rsslChnlImpl, RsslUInt32 size, RsslBool packedBuffer, RsslError *error)
{
	RsslSeqMcastChannel *pSeqMcastChannel = (RsslSeqMcastChannel*)rsslChnlImpl->transportInfo;
	rsslBufferImpl *pBufferImpl;
	rtrSeqMcastBuffer *seqMcastBuffer;
	RsslQueueLink *pLink = 0;

	if (chnlLocking) seqMcastGetLock(&pSeqMcastChannel->lock);
	if (size > pSeqMcastChannel->maxMsgSize)
	{
		_rsslSetError(error, NULL, RSSL_RET_FAILURE,  0);
		snprintf(error->text, MAX_RSSL_ERROR_TEXT, "<%s:%d> RsslGetBuffer() Error: 0015 Cannot get a buffer larger than the configured maximum message size (%d).\n", __FILE__, __LINE__, pSeqMcastChannel->maxMsgSize);
		if (chnlLocking) seqMcastUnlock(&pSeqMcastChannel->lock);
		return NULL;
	}

	if (pSeqMcastChannel->bufferInUse)
	{
		_rsslSetError(error, NULL, RSSL_RET_FAILURE,  0);
		snprintf(error->text, MAX_RSSL_ERROR_TEXT, "<%s:%d> RsslGetBuffer() Error: 0015 Sequence Multicast transport does not allow getting multiple buffers.\n", __FILE__, __LINE__);
		if (chnlLocking) seqMcastUnlock(&pSeqMcastChannel->lock);
		return NULL;
	}

	pSeqMcastChannel->bufferInUse = RSSL_TRUE;

	pLink = rsslQueueRemoveFirstLink(&(rsslChnlImpl->freeBufferList));

	/* This is theoretically impossible, since we have 1 allocated buffer, and a pool of... many, but it's here to just make sure the queues haven't been corrupted. */
	if (!pLink)
	{
		_rsslSetError(error, NULL, RSSL_RET_FAILURE,  0);
		snprintf(error->text, MAX_RSSL_ERROR_TEXT, "<%s:%d> RsslGetBuffer() Error: 0015 Sequence Multicast transport does not allow getting multiple buffers.\n", __FILE__, __LINE__);
		if (chnlLocking)
			seqMcastUnlock(&pSeqMcastChannel->lock);
		return NULL;
	}
	pBufferImpl = RSSL_QUEUE_LINK_TO_OBJECT(rsslBufferImpl, link1, pLink);

	pBufferImpl->buffer.length = size;
	pBufferImpl->buffer.data = pSeqMcastChannel->bufferMem + SEQ_MCAST_MAX_HDR_LEN;
	/* Since the packing header and non-packed header are identical, every buffer is packable. 
	   The packingOffset will be initially set to after the seqNum(4 bytes in), then every subsequent pack will place the length at that point
	   Each subsequent pack should then place the packingOffset to the end of the previous pack's data, without reserving the length in the offset */
	
	seqMcastBuffer = &(pSeqMcastChannel->writeBuffer);
	pBufferImpl->packingOffset = SEQ_MCAST_MAX_HDR_LEN;
	pBufferImpl->bufferInfo = seqMcastBuffer;
	seqMcastBuffer->buffer = pSeqMcastChannel->bufferMem;
	seqMcastBuffer->maxLength = size;
	/* This memory is owned entirely by the SeqMCast channel */
	pBufferImpl->owner = 0;
	if (chnlLocking)
		seqMcastUnlock(&pSeqMcastChannel->lock);

	return pBufferImpl;
}

/* Contains code necessary to query Sequence Multicast channel for more detailed connection info (client or server side) */
RSSL_RSSL_SEQ_MCAST_IMPL_FAST(RsslRet) rsslSeqMcastGetChannelInfo(rsslChannelImpl *rsslChnlImpl, RsslChannelInfo *info, RsslError *error)
{
	RsslSeqMcastChannel *pSeqMcastChannel = (RsslSeqMcastChannel*)rsslChnlImpl->transportInfo;
	socklen_t optlen;

	if (chnlLocking)
		seqMcastGetLock(&pSeqMcastChannel->lock);
	memset(info, 0, sizeof(RsslChannelInfo));
	info->maxFragmentSize = pSeqMcastChannel->maxMsgSize;
	info->maxOutputBuffers = 1;
	info->guaranteedOutputBuffers = 1;
	info->numInputBuffers = 1;
	info->pingTimeout = rsslChnlImpl->Channel.pingTimeout;
	info->clientToServerPings = RSSL_FALSE;
	info->serverToClientPings = RSSL_FALSE;
	info->multicastStats.mcastRcvd = pSeqMcastChannel->pktRecvCount;
	info->multicastStats.mcastSent = pSeqMcastChannel->pktSentCount;
	
	info->encryptionProtocol = RSSL_ENC_NONE;

	optlen = sizeof(info->sysSendBufSize);
	if (getsockopt(rsslChnlImpl->Channel.socketId, SOL_SOCKET, SO_SNDBUF, (char *)&info->sysSendBufSize, &optlen) < 0)
	{
		_rsslSetError(error, NULL, RSSL_RET_FAILURE,  errno);
		snprintf(error->text, MAX_RSSL_ERROR_TEXT, "<%s:%d> rsslGetChannelInfo() Error: 1002 getsockopt() failed.  System errno: (%d)\n", __FILE__, __LINE__, errno);
		if (chnlLocking)
			seqMcastUnlock(&pSeqMcastChannel->lock);
		return RSSL_RET_FAILURE;
	}

	optlen = sizeof(info->sysRecvBufSize);
	if (getsockopt(rsslChnlImpl->Channel.socketId, SOL_SOCKET, SO_RCVBUF, (char *)&info->sysRecvBufSize, &optlen) < 0)
	{
		_rsslSetError(error, NULL, RSSL_RET_FAILURE,  errno);
		snprintf(error->text, MAX_RSSL_ERROR_TEXT, "<%s:%d> rsslGetChannelInfo() Error: 1002 getsockopt() failed.  System errno: (%d)\n", __FILE__, __LINE__, errno);
		if (chnlLocking)
			seqMcastUnlock(&pSeqMcastChannel->lock);
		return RSSL_RET_FAILURE;
	}
	if (chnlLocking)
		seqMcastUnlock(&pSeqMcastChannel->lock);

	return RSSL_RET_SUCCESS;
}

/* Contains code necessary to query Sequence Multicast server for more detailed connection info (server only) */
/* Not implemented */
RSSL_RSSL_SEQ_MCAST_IMPL_FAST(RsslRet) rsslSeqMcastGetSrvrInfo(rsslServerImpl *rsslSrvrImpl, RsslServerInfo *info, RsslError *error)
{
	_rsslSetError(error, NULL, RSSL_RET_FAILURE,  0);
	snprintf(error->text, MAX_RSSL_ERROR_TEXT, "<%s:%d> rsslGetServerInfo() Error: 0006 Server operations are not implemented for the sequenced multicast transport.\n", __FILE__, __LINE__);

	return RSSL_RET_FAILURE;
}

/* Contains code necessary for Sequence Multicast connections (client or server side) to perform and complete handshake process (no-op) */
RsslRet rsslSeqMcastInitChannel(rsslChannelImpl* rsslChnlImpl, RsslInProgInfo *inProg, RsslError *error)
{
	return RSSL_RET_SUCCESS;
}

/* Contains code necessary to do an ioctl on a Sequence Multicast client */
RSSL_RSSL_SEQ_MCAST_IMPL_FAST(RsslRet) rsslSeqMcastIoctl(rsslChannelImpl *rsslChnlImpl, RsslIoctlCodes code, void *value, RsslError *error)
{
	RsslSeqMcastChannel *pSeqMcastChannel = (RsslSeqMcastChannel*)rsslChnlImpl->transportInfo;

	if (chnlLocking) seqMcastGetLock(&pSeqMcastChannel->lock);
	/* component info is handled above in rsslImpl */
	switch(code)
	{
		case RSSL_SYSTEM_READ_BUFFERS:
			if (setsockopt(rsslChnlImpl->Channel.socketId, SOL_SOCKET, SO_RCVBUF, value, sizeof(RsslInt32)) < 0)
			{
				_rsslSetError(error, NULL, RSSL_RET_FAILURE, errno);
				snprintf(error->text, MAX_RSSL_ERROR_TEXT, "<%s:%d> rsslIoctrl()  Error: 1002 setsockopt() failed. Unable to set SO_RCVBUF on socket. System errno: (%d).\n", __FILE__, __LINE__, errno);
				if (chnlLocking)
					seqMcastUnlock(&pSeqMcastChannel->lock);
				return RSSL_RET_FAILURE;
			}
			break;
		case RSSL_SYSTEM_WRITE_BUFFERS:
			if (setsockopt(rsslChnlImpl->Channel.socketId, SOL_SOCKET, SO_SNDBUF, value, sizeof(RsslInt32)) < 0)
			{
				_rsslSetError(error, NULL, RSSL_RET_FAILURE, errno);
				snprintf(error->text, MAX_RSSL_ERROR_TEXT, "<%s:%d> rsslIoctrl()  Error: 1002 setsockopt() failed. Unable to set SO_SNDBUF on socket. System errno: (%d).\n", __FILE__, __LINE__, errno);
				if (chnlLocking)
					seqMcastUnlock(&pSeqMcastChannel->lock);
				return RSSL_RET_FAILURE;
			}
			break;
		/* we dont do anything with these codes in the Sequence Multicast connection. Just return success. */
		case RSSL_MAX_NUM_BUFFERS:
		case RSSL_NUM_GUARANTEED_BUFFERS:
		case RSSL_HIGH_WATER_MARK:
		case RSSL_REGISTER_HASH_ID:
		case RSSL_UNREGISTER_HASH_ID:
		case RSSL_PRIORITY_FLUSH_ORDER:
		case RSSL_SERVER_NUM_POOL_BUFFERS:
		case RSSL_COMPRESSION_THRESHOLD:
		case RSSL_SERVER_PEAK_BUF_RESET:
		case RSSL_DEBUG_FLAGS:
			break;
		default:
			_rsslSetError(error, &rsslChnlImpl->Channel, RSSL_RET_FAILURE, 0);
			snprintf(error->text, MAX_RSSL_ERROR_TEXT, "<%s:%d> rsslIoctl() Error: 0017 Invalid RSSL IOCtl Code (%d).\n", __FILE__, __LINE__, code);
			if (chnlLocking)
				seqMcastUnlock(&pSeqMcastChannel->lock);
			return RSSL_RET_FAILURE;
	}
	if (chnlLocking)
		seqMcastUnlock(&pSeqMcastChannel->lock);

	return RSSL_RET_SUCCESS;
}

/* Contains code necessary for buffer packing with Sequence Multicast connection buffer */
RSSL_RSSL_SEQ_MCAST_IMPL_FAST(RsslBuffer*) rsslSeqMcastPackBuffer(rsslChannelImpl *rsslChnlImpl, rsslBufferImpl *rsslBufImpl, RsslError *error)
{
	rtrSeqMcastBuffer *seqMcastBuffer = (rtrSeqMcastBuffer *)rsslBufImpl->bufferInfo;

	/* Do we need this check? */
	if (rsslBufImpl->buffer.length > seqMcastBuffer->maxLength - (rsslBufImpl->packingOffset - SEQ_MCAST_MAX_HDR_LEN))
	{
		_rsslSetError(error, &rsslChnlImpl->Channel, RSSL_RET_INVALID_ARGUMENT, 0);
		snprintf(error->text, MAX_RSSL_ERROR_TEXT, "<%s:%d> rsslPackBuffer() Error: 0008 Indicated buffer length is longer than allocated transport buffer length.\n", __FILE__, __LINE__);
		return NULL;
	}

	/* packing offset is set to immediately after the length, so we write the 2 byte length on the 2 previous bytes */
	rwfPut16((seqMcastBuffer->buffer + rsslBufImpl->packingOffset - SEQ_MCAST_MESSAGE_LENGTH_LEN), rsslBufImpl->buffer.length);

	/* Move packingOffset beyond the 2 byte message length plus end of the current buffer */
	rsslBufImpl->packingOffset += rsslBufImpl->buffer.length + SEQ_MCAST_MESSAGE_LENGTH_LEN;
	/* The total used amount is now packingOffset - SEQ_MCAST_SEQUENCE_NUM_LEN */
	seqMcastBuffer->length = rsslBufImpl->packingOffset - SEQ_MCAST_MESSAGE_LENGTH_LEN;

	if ((rsslBufImpl->packingOffset) < (RsslUInt32)(seqMcastBuffer->maxLength + SEQ_MCAST_MAX_HDR_LEN))
	{
		rsslBufImpl->buffer.data = seqMcastBuffer->buffer + rsslBufImpl->packingOffset;
		/* The remaining length is the maxLength(which does not include the intial header) - (packingOffset(which includes the initial 6 byte seq num and length) - initial header(6 bytes)) */
		rsslBufImpl->buffer.length = seqMcastBuffer->maxLength - (rsslBufImpl->packingOffset - SEQ_MCAST_MAX_HDR_LEN);
	}
	else
	{
		rsslBufImpl->buffer.data = 0;		/* tell them there is no room left */
		rsslBufImpl->buffer.length = 0;		/* set this to zero. The alternative (which we dont like) is to point beyond the buffer */
	}

	return &(rsslBufImpl->buffer);
}

/* Contains code necessary to send a ping message */
/* Sequence Multicast ping message is just sending last sequence number */
RSSL_RSSL_SEQ_MCAST_IMPL_FAST(RsslRet) rsslSeqMcastPing(rsslChannelImpl *rsslChnlImpl, RsslError *error)
{
	RsslSeqMcastChannel *pSeqMcastChannel = (RsslSeqMcastChannel*)rsslChnlImpl->transportInfo;
	RsslInt32 ret;
	char sendBuf[20];  /* Only need 12, padding just in case */

	if (chnlLocking)
		seqMcastGetLock(&pSeqMcastChannel->lock);

	rsslSeqMcastWriteHdr(rsslChnlImpl, sendBuf, 0, pSeqMcastChannel->writeSeqNum, error);

	/* send packet */
	do
	{
		if ((ret = sendto(rsslChnlImpl->Channel.socketId, sendBuf,
						SEQ_MCAST_PING_LEN, 
						0, (struct sockaddr*)&pSeqMcastChannel->sendAddr, sizeof(pSeqMcastChannel->sendAddr))) < 0)
		{
			if(errno == EWOULDBLOCK || errno == EAGAIN)
			{
				if (chnlLocking)
					seqMcastUnlock(&pSeqMcastChannel->lock);
				return RSSL_RET_SUCCESS;
			}
			else if(errno == EINTR)
			{
				continue;
			}
			else
			{
				_rsslSetError(error, NULL, RSSL_RET_FAILURE,  errno);
				snprintf(error->text, MAX_RSSL_ERROR_TEXT, "<%s:%d> rsslPing() Error: 1002 sendto() failed.  System errno: (%d).\n", __FILE__, __LINE__, errno);
				if (chnlLocking)
					seqMcastUnlock(&pSeqMcastChannel->lock);
				return RSSL_RET_FAILURE;
			}
		}
	} while (0);

	pSeqMcastChannel->pktSentCount++;
	pSeqMcastChannel->bufferInUse = RSSL_FALSE;

	if (chnlLocking)
		seqMcastUnlock(&pSeqMcastChannel->lock);

	return RSSL_RET_SUCCESS;
}

/* Contains code necessary to read from a Sequence Multicast network (client only) */
RSSL_RSSL_SEQ_MCAST_IMPL_FAST(RsslBuffer*) rsslSeqMcastRead(rsslChannelImpl* rsslChnlImpl, RsslReadOutArgs *readOutArgs, RsslRet *readRet, RsslError *error)
{
	RsslSeqMcastChannel *pSeqMcastChannel = (RsslSeqMcastChannel*)rsslChnlImpl->transportInfo;
	RsslInt32 cc, remainingLen;
	struct sockaddr_in srcAddr;
	RsslInt32 srcAddrLen = sizeof(struct sockaddr_in);
	RsslUInt8 tmpChar;
	RsslInt32 hdrLen;
	RsslInt32 readFlags;

	if (!chnlLocking || (seqMcastTrylock(&pSeqMcastChannel->lock) ==  RSSL_TRUE))
	{
		if (pSeqMcastChannel->stillProcessingPacket == RSSL_FALSE) /* process a new packet from network */
		{
			pSeqMcastChannel->inputBuffer.data = pSeqMcastChannel->inputBufferMem;

			if ((cc = recvfrom(rsslChnlImpl->Channel.socketId, pSeqMcastChannel->inputBuffer.data, pSeqMcastChannel->maxMsgSize + SEQ_MCAST_MAX_HDR_LEN, 0, (struct sockaddr*)&srcAddr, (socklen_t*)&srcAddrLen)) < 0)
			{
				if(errno == EINTR || errno == EWOULDBLOCK || errno == EAGAIN)
				{
					if(errno == EINTR)
						*readRet = 1;
					else
						*readRet = RSSL_RET_READ_WOULD_BLOCK;

					if (chnlLocking)
						seqMcastUnlock(&pSeqMcastChannel->lock);

					return NULL;
				}
				else
				{
					*readRet = -1;
					_rsslSetError(error, NULL, RSSL_RET_FAILURE,  errno);
					snprintf(error->text, MAX_RSSL_ERROR_TEXT, "<%s:%d> rsslRead() Error: 1002  Call to recvfrom() failed.  System errno: (%d).\n", __FILE__, __LINE__, errno);
					if (chnlLocking)
						seqMcastUnlock(&pSeqMcastChannel->lock);

					return NULL;
				}
			}

			pSeqMcastChannel->pktRecvCount++;

			/* each packet contains one or more messages */

			pSeqMcastChannel->processedPacketLen = 0; /* reset processed packet length */
			pSeqMcastChannel->inputBuffer.length = (RsslUInt32)cc; /* this is total packet length */
			hdrLen = 0;

			if (cc < SEQ_MCAST_PING_LEN)
			{
				/* set bytes read, uncompressed bytes read */
				readOutArgs->bytesRead = 0;
				readOutArgs->uncompressedBytesRead = 0;
				*readRet = -1;
				_rsslSetError(error, NULL, RSSL_RET_FAILURE, errno);
				snprintf(error->text, MAX_RSSL_ERROR_TEXT, "<%s:%d> rsslRead() Error: 1007 Incomming UDP packet length is too short to contain a ETA Sequenced Multicast header.\n", __FILE__, __LINE__);
				if (chnlLocking)
					seqMcastUnlock(&pSeqMcastChannel->lock);

				return NULL;
			}

			/* Header version */
			RTR_GET_8(tmpChar, &pSeqMcastChannel->inputBuffer.data[pSeqMcastChannel->processedPacketLen]);
			pSeqMcastChannel->processedPacketLen += SEQ_MCAST_VERSION_LEN;

			/* Right now, this can only be version 1 */
			if (tmpChar != SEQ_MCAST_MAX_VERSION)
			{
				/* set bytes read, uncompressed bytes read */
				readOutArgs->bytesRead = 0;
				readOutArgs->uncompressedBytesRead = 0;
				*readRet = -1;
				_rsslSetError(error, NULL, RSSL_RET_FAILURE, errno);
				snprintf(error->text, MAX_RSSL_ERROR_TEXT, "<%s:%d> rsslRead() Error: 1007 Unknown ETA Sequenced Multicast header version(%d).\n", __FILE__, __LINE__, tmpChar);
				if (chnlLocking)
					seqMcastUnlock(&pSeqMcastChannel->lock);

				return NULL;
			}

			/* Flags */
			RTR_GET_8(tmpChar, &pSeqMcastChannel->inputBuffer.data[pSeqMcastChannel->processedPacketLen]);

			readFlags = (RsslInt32)tmpChar;
			if (readFlags & SEQ_MCAST_FLAGS_RETRANSMIT)
				readOutArgs->readOutFlags |= RSSL_READ_OUT_RETRANSMIT; 	

			pSeqMcastChannel->processedPacketLen += SEQ_MCAST_FLAGS_LEN;

			/* Payload protocol type */
			RTR_GET_8(tmpChar, &pSeqMcastChannel->inputBuffer.data[pSeqMcastChannel->processedPacketLen]);
			pSeqMcastChannel->processedPacketLen += SEQ_MCAST_PROTOCOL_TYPE_LEN;

			/* Check to make sure there isn't a protocol mismatch */
			if (tmpChar != rsslChnlImpl->Channel.protocolType)
			{
				/* set bytes read, uncompressed bytes read */
				readOutArgs->bytesRead = 0;
				readOutArgs->uncompressedBytesRead = 0;
				*readRet = -1;
				_rsslSetError(error, NULL, RSSL_RET_FAILURE, errno);
				snprintf(error->text, MAX_RSSL_ERROR_TEXT, "<%s:%d> rsslRead() Error: 1007 Protocol type does not match configured protocol.\n", __FILE__, __LINE__);
				if (chnlLocking)
					seqMcastUnlock(&pSeqMcastChannel->lock);

				return NULL;
			}

			/* Header Length, store it here */
			RTR_GET_8(tmpChar, &pSeqMcastChannel->inputBuffer.data[pSeqMcastChannel->processedPacketLen]);
			hdrLen = (RsslInt32)tmpChar;
			pSeqMcastChannel->processedPacketLen += SEQ_MCAST_HDR_LENGTH_LEN;

			/* Instance ID */
			RTR_GET_16(pSeqMcastChannel->readInstanceID, &pSeqMcastChannel->inputBuffer.data[pSeqMcastChannel->processedPacketLen]);
			pSeqMcastChannel->processedPacketLen += SEQ_MCAST_INSTANCE_ID_LEN;

			/* Protocol Major */
			RTR_GET_8(tmpChar, &pSeqMcastChannel->inputBuffer.data[pSeqMcastChannel->processedPacketLen]);
			pSeqMcastChannel->processedPacketLen += SEQ_MCAST_PROTO_VER_MAJOR_LEN;

			/* TODO: Verify version here */

			/* Protocol Minor */
			RTR_GET_8(tmpChar, &pSeqMcastChannel->inputBuffer.data[pSeqMcastChannel->processedPacketLen]);
			pSeqMcastChannel->processedPacketLen += SEQ_MCAST_PROTO_VER_MINOR_LEN;

			/* TODO: Verify version here */

			/* get packet sequence number (byte swap) */
			RTR_GET_32(pSeqMcastChannel->readSeqNum, &pSeqMcastChannel->inputBuffer.data[pSeqMcastChannel->processedPacketLen]);
			pSeqMcastChannel->processedPacketLen += SEQ_MCAST_SEQUENCE_NUM_LEN;

			/* We're now at the end of the header, so move the processedPacketLen to where hdrLen is */
			pSeqMcastChannel->processedPacketLen = hdrLen;

			/* set bytes read, uncompressed bytes read */
			readOutArgs->bytesRead = cc;
			readOutArgs->uncompressedBytesRead = cc;
			
			/* get packet's sender address and port */
			RTR_GET_32(pSeqMcastChannel->readAddr, &(srcAddr.sin_addr));
			RTR_GET_16(pSeqMcastChannel->readPort, &(srcAddr.sin_port));
			
			/* check for ping */
			if (pSeqMcastChannel->inputBuffer.length == SEQ_MCAST_PING_LEN)
			{
				readOutArgs->bytesRead = cc;
				readOutArgs->uncompressedBytesRead = cc;
				readOutArgs->readOutFlags |= RSSL_READ_OUT_SEQNUM | RSSL_READ_OUT_NODE_ID | RSSL_READ_OUT_INSTANCE_ID;
				readOutArgs->seqNum = pSeqMcastChannel->readSeqNum;
				readOutArgs->nodeId.nodeAddr = pSeqMcastChannel->readAddr;
				readOutArgs->nodeId.port = pSeqMcastChannel->readPort;
				readOutArgs->instanceId = pSeqMcastChannel->readInstanceID;
				*readRet = RSSL_RET_READ_PING;
				if (chnlLocking)
					seqMcastUnlock(&pSeqMcastChannel->lock);
				return NULL;
			}

			/* get message length (byte swap) */
			RWF_GET_16_AS_32(&(pSeqMcastChannel->readBuffer.length), &(pSeqMcastChannel->inputBuffer.data[pSeqMcastChannel->processedPacketLen]));
			pSeqMcastChannel->processedPacketLen += SEQ_MCAST_MESSAGE_LENGTH_LEN;

			/* Check msg length.  If it's greater than read in, error out.  This should be catastrophic */
			if(  (RsslInt32)(pSeqMcastChannel->readBuffer.length + pSeqMcastChannel->processedPacketLen)  > cc)
			{
				rsslChnlImpl->Channel.state = RSSL_CH_STATE_CLOSED;
				*readRet = RSSL_RET_FAILURE;
				_rsslSetError(error, NULL, RSSL_RET_FAILURE,  errno);
				snprintf(error->text, MAX_RSSL_ERROR_TEXT, "<%s:%d> rsslRead() Error: 1007 Indicated message length is longer than remaining length of datagram.\n", __FILE__, __LINE__);
				if (chnlLocking)
					seqMcastUnlock(&pSeqMcastChannel->lock);
				return NULL;
			}

			/* get message data */
			pSeqMcastChannel->readBuffer.data = &pSeqMcastChannel->inputBuffer.data[pSeqMcastChannel->processedPacketLen];
			pSeqMcastChannel->processedPacketLen += pSeqMcastChannel->readBuffer.length;

			/* determine if there are more messages in this packet */
			remainingLen = pSeqMcastChannel->inputBuffer.length - pSeqMcastChannel->processedPacketLen;
			if (remainingLen > 0)
			{
				pSeqMcastChannel->stillProcessingPacket = RSSL_TRUE;
			}
			*readRet = remainingLen;
		}
		else /* still processing previous packet */
		{
			/* set bytes read, uncompressed bytes read */
			readOutArgs->bytesRead = 0;
			readOutArgs->uncompressedBytesRead = 0;
			/* continue processing previous packet */
			/* Check length here, error out if we can't read */
			if( (RsslUInt32)(pSeqMcastChannel->processedPacketLen + SEQ_MCAST_MESSAGE_LENGTH_LEN) > pSeqMcastChannel->inputBuffer.length)
			{
				rsslChnlImpl->Channel.state = RSSL_CH_STATE_CLOSED;
				*readRet = RSSL_RET_FAILURE;
				_rsslSetError(error, NULL, RSSL_RET_FAILURE,  errno);
				snprintf(error->text, MAX_RSSL_ERROR_TEXT, "<%s:%d> rsslRead() Error: 1007 Unable to read length from packed datagram.\n", __FILE__, __LINE__);
				if (chnlLocking)
					seqMcastUnlock(&pSeqMcastChannel->lock);
				return NULL;
			}
			RWF_GET_16_AS_32(&(pSeqMcastChannel->readBuffer.length), &(pSeqMcastChannel->inputBuffer.data[pSeqMcastChannel->processedPacketLen]));
			pSeqMcastChannel->processedPacketLen += SEQ_MCAST_MESSAGE_LENGTH_LEN;

			if(pSeqMcastChannel->processedPacketLen + pSeqMcastChannel->readBuffer.length > pSeqMcastChannel->inputBuffer.length)
			{
				rsslChnlImpl->Channel.state = RSSL_CH_STATE_CLOSED;
				*readRet = RSSL_RET_FAILURE;
				_rsslSetError(error, NULL, RSSL_RET_FAILURE,  errno);
				snprintf(error->text, MAX_RSSL_ERROR_TEXT, "<%s:%d> rsslRead() Error: 1007 Indicated message length is longer than remaining length of datagram.\n", __FILE__, __LINE__);
				if (chnlLocking)
					seqMcastUnlock(&pSeqMcastChannel->lock);
				return NULL;
			}

			/* get message data */
			pSeqMcastChannel->readBuffer.data = &pSeqMcastChannel->inputBuffer.data[pSeqMcastChannel->processedPacketLen];
			pSeqMcastChannel->processedPacketLen += pSeqMcastChannel->readBuffer.length;

			/* determine if there are more messages in this packet */
			remainingLen = pSeqMcastChannel->inputBuffer.length - pSeqMcastChannel->processedPacketLen;
			if (remainingLen == 0)
			{
				pSeqMcastChannel->stillProcessingPacket = RSSL_FALSE;
			}
			*readRet = remainingLen;
		}
		if (chnlLocking)
			seqMcastUnlock(&pSeqMcastChannel->lock);
	}
	else /* trylock failed */
	{
		*readRet = RSSL_RET_READ_IN_PROGRESS;
		return NULL;
	}

	/* set sequence number */
	readOutArgs->readOutFlags |= RSSL_READ_OUT_SEQNUM | RSSL_READ_OUT_NODE_ID | RSSL_READ_OUT_INSTANCE_ID;
	readOutArgs->seqNum = pSeqMcastChannel->readSeqNum;
	readOutArgs->nodeId.nodeAddr = pSeqMcastChannel->readAddr;
	readOutArgs->nodeId.port = pSeqMcastChannel->readPort;
	readOutArgs->instanceId = pSeqMcastChannel->readInstanceID;

	if ((rsslChnlImpl->debugFlags & RSSL_DEBUG_RSSL_DUMP_IN) && (pSeqMcastChannel->readBuffer.length))
	{
		rsslSeqMcastDumpInFuncImpl(__FUNCTION__, pSeqMcastChannel->readBuffer.data, pSeqMcastChannel->readBuffer.length,
			rsslChnlImpl->Channel.socketId, &rsslChnlImpl->Channel);
	}

	return &pSeqMcastChannel->readBuffer;
}

/* Contains code necessary to reconnect Sequence Multicast connections and bridge data flow (no-op) */
RsslRet rsslSeqMcastReconnect(rsslChannelImpl *rsslChnlImpl, RsslError *error)
{
	return RSSL_RET_SUCCESS;
}

/* Contains code necessary to release an unused/unsuccessfully written buffer to Sequence Multicast multicast network (server side only) */
RSSL_RSSL_SEQ_MCAST_IMPL_FAST(RsslRet) rsslSeqMcastReleaseBuffer(rsslChannelImpl *rsslChnlImpl, rsslBufferImpl *rsslBufImpl, RsslError *error)
{
	RsslSeqMcastChannel *pSeqMcastChannel = (RsslSeqMcastChannel*)rsslChnlImpl->transportInfo;
	if (chnlLocking)
		seqMcastGetLock(&pSeqMcastChannel->lock);
	pSeqMcastChannel->bufferInUse = RSSL_FALSE;
	if (chnlLocking)
		seqMcastUnlock(&pSeqMcastChannel->lock);

	return RSSL_RET_SUCCESS;
}

/* Contains code necessary to query number of used buffers by the server (shared pool buffers typically) (no-op) */
/* Not implemented */
RSSL_RSSL_SEQ_MCAST_IMPL_FAST(RsslInt32) rsslSeqMcastSrvrBufferUsage(rsslServerImpl *rsslSrvrImpl, RsslError *error)
{
	_rsslSetError(error, NULL, RSSL_RET_FAILURE,  0);
	snprintf(error->text, MAX_RSSL_ERROR_TEXT, "<%s:%d> rsslSrvrBufferUsage() Error 0006 Not implemented.", __FILE__, __LINE__);

	return RSSL_RET_FAILURE;
}

/* Contains code necessary to do an ioctl on a Sequence Multicast server */
/* Not implemented */
RSSL_RSSL_SEQ_MCAST_IMPL_FAST(RsslRet) rsslSeqMcastSrvrIoctl(rsslServerImpl *rsslSrvrImpl, RsslIoctlCodes code, void *value, RsslError *error)
{
	_rsslSetError(error, NULL, RSSL_RET_FAILURE,  0);
	snprintf(error->text, MAX_RSSL_ERROR_TEXT, "<%s:%d> Not implemented.", __FILE__, __LINE__);

	return RSSL_RET_FAILURE;
}

/* Contains code necessary to write data going to a Sequence Multicast network */
RSSL_RSSL_SEQ_MCAST_IMPL_FAST(RsslRet) rsslSeqMcastWrite(rsslChannelImpl *rsslChnlImpl, rsslBufferImpl *rsslBufImpl, RsslWriteInArgs *writeInArgs, RsslWriteOutArgs *writeOutArgs,  RsslError *error)
{
	RsslSeqMcastChannel *pSeqMcastChannel = (RsslSeqMcastChannel*)rsslChnlImpl->transportInfo;
	RsslInt32 ret;
	RsslUInt16 bufLength;
	RsslInt32 hdrOffset;
	RsslInt32 pktLength;

	RsslUInt32 writeFlags = writeInArgs->writeInFlags; 
	RsslUInt32 wSeqNum;
	RsslUInt8 flags = 0x0;
	rtrSeqMcastBuffer *seqMcastBuffer = (rtrSeqMcastBuffer *)rsslBufImpl->bufferInfo;

	if (chnlLocking)
		seqMcastGetLock(&pSeqMcastChannel->lock);

	if (!(writeFlags & RSSL_WRITE_IN_SEQNUM))
	{
		pSeqMcastChannel->writeSeqNum++;
		if (pSeqMcastChannel->writeSeqNum == 0) /* skip 0 since 0 is for reset */
		{
			pSeqMcastChannel->writeSeqNum = 1;
		}
		wSeqNum = pSeqMcastChannel->writeSeqNum;
	}
	else
	{
		wSeqNum = writeInArgs->seqNum;
	}
	
	if ((writeFlags & RSSL_WRITE_IN_RETRANSMIT) && !(writeFlags & RSSL_WRITE_IN_SEQNUM))
	{
		_rsslSetError(error, &rsslChnlImpl->Channel, RSSL_RET_FAILURE, 0);
		snprintf(error->text, MAX_RSSL_ERROR_TEXT, "<%s:%d> rsslWrite() Error 0012 SeqNum flag not set when Retransmit flag is on.\n", __FILE__, __LINE__);
		if (chnlLocking)
			seqMcastUnlock(&pSeqMcastChannel->lock);
		return RSSL_RET_FAILURE;	
	}

	if (!(writeFlags & RSSL_WRITE_IN_RETRANSMIT) && (writeFlags & RSSL_WRITE_IN_SEQNUM))
	{
		_rsslSetError(error, &rsslChnlImpl->Channel, RSSL_RET_FAILURE, 0);
		snprintf(error->text, MAX_RSSL_ERROR_TEXT, "<%s:%d> rsslWrite() Error 0012 Retransmit flag not set when SeqNum flag is on.\n", __FILE__, __LINE__);
		if (chnlLocking)
			seqMcastUnlock(&pSeqMcastChannel->lock);
		return RSSL_RET_FAILURE;	
	}
	
	RSSL_ASSERT(rsslBufImpl->packingOffset >= SEQ_MCAST_MAX_HDR_LEN, "Transport memory buffer has been corrupted\n");

	if (writeFlags & RSSL_WRITE_IN_RETRANSMIT)
		flags |= SEQ_MCAST_FLAGS_RETRANSMIT;

	hdrOffset = rsslSeqMcastWriteHdr(rsslChnlImpl, seqMcastBuffer->buffer, flags, wSeqNum, error);
	
	if (hdrOffset < 0)
	{
		/* Error info should be populated in rsslSeqMcastWriteHdr */
		if (chnlLocking)
			seqMcastUnlock(&pSeqMcastChannel->lock);
		return RSSL_RET_FAILURE;
	}

	/* If packing, check for last message length.  If it's 0 and packing offest is > 0, send the buffer.
	   Otherwise, put in length, then advance the total length. */
	if(rsslBufImpl->buffer.length != 0)
	{
		if (rsslBufImpl->buffer.length > seqMcastBuffer->maxLength - (rsslBufImpl->packingOffset - SEQ_MCAST_MAX_HDR_LEN))
		{
			_rsslSetError(error, &rsslChnlImpl->Channel, RSSL_RET_FAILURE, 0);
			snprintf(error->text, MAX_RSSL_ERROR_TEXT, "<%s:%d> rsslWrite() Error 0008 Packed buffer size %d is longer than remaining packed buffer length.\n", __FILE__, __LINE__, rsslBufImpl->buffer.length);
			if (chnlLocking)
				seqMcastUnlock(&pSeqMcastChannel->lock);
			return RSSL_RET_FAILURE;
		}
		
		/* Put on the length if the final(or first) message in the pack has a length greater than 0 */
		bufLength = rsslBufImpl->buffer.length;
		/* Packing offset includes the length of the message */
		rwfPut16((seqMcastBuffer->buffer + rsslBufImpl->packingOffset - SEQ_MCAST_MESSAGE_LENGTH_LEN), bufLength);
		/* Advance packing offset to include last message, but remove the extra message length and any header offset due to optional members */
		pktLength = rsslBufImpl->packingOffset + rsslBufImpl->buffer.length - hdrOffset;
	}
	else
	{
		/* If the length is 0, the seqMcastBuffer.legnth has already been set on a previous pack */
		/* However, if the total packingOffset is 14(i.e. a header), nothing has been written in, and this should error out */
		if (rsslBufImpl->packingOffset == SEQ_MCAST_MAX_HDR_LEN)
		{
			_rsslSetError(error, NULL, RSSL_RET_FAILURE,  0);
			snprintf(error->text, MAX_RSSL_ERROR_TEXT, "<%s:%d> rsslWrite() Error: 0009 Cannot write a 0 length buffer.\n", __FILE__, __LINE__);
			if (chnlLocking)
				seqMcastUnlock(&pSeqMcastChannel->lock);
			return RSSL_RET_FAILURE;
		}

		pktLength = seqMcastBuffer->length - hdrOffset;
	}
		
	/* send packet */
	do
	{
		if ((ret = sendto(rsslChnlImpl->Channel.socketId, seqMcastBuffer->buffer + hdrOffset,
						  pktLength, 0, (struct sockaddr*)&pSeqMcastChannel->sendAddr, sizeof(pSeqMcastChannel->sendAddr))) < 0)
		{
			if(errno == EWOULDBLOCK || errno == EAGAIN)
			{
				writeOutArgs->bytesWritten = 0;
				writeOutArgs->uncompressedBytesWritten = 0;
				if (chnlLocking)
					seqMcastUnlock(&pSeqMcastChannel->lock);
				return RSSL_RET_WRITE_CALL_AGAIN;
			}
			else if (errno == EINTR)
			{
				/* Check to see if this works */
				continue;
			}
			else
			{
				writeOutArgs->bytesWritten = 0;
				writeOutArgs->uncompressedBytesWritten = 0;
				rsslChnlImpl->Channel.state = RSSL_CH_STATE_CLOSED;
				_rsslSetError(error, NULL, RSSL_RET_FAILURE,  errno);
				snprintf(error->text, MAX_RSSL_ERROR_TEXT, "<%s:%d> rsslWrite() Error: 1002 Call to send() failed.  System errno: (%d).\n", __FILE__, __LINE__, errno);
				if (chnlLocking)
					seqMcastUnlock(&pSeqMcastChannel->lock);
				return RSSL_RET_FAILURE;
			}
		}
	} while (0);

	pSeqMcastChannel->pktSentCount++;

	pSeqMcastChannel->bufferInUse = RSSL_FALSE;
	/* Move the buffer out of the activeBufferList to the freeBufferList */
	if (rsslQueueLinkInAList(&(rsslBufImpl->link1)))
		rsslQueueRemoveLink(&(rsslChnlImpl->activeBufferList), &(rsslBufImpl->link1));

	rsslQueueAddLinkToBack(&(rsslChnlImpl->freeBufferList), &(rsslBufImpl->link1));

	writeOutArgs->bytesWritten = seqMcastBuffer->length;
	writeOutArgs->uncompressedBytesWritten = seqMcastBuffer->length;
	if (chnlLocking)
		seqMcastUnlock(&pSeqMcastChannel->lock);

	return RSSL_RET_SUCCESS;
}

/***************************
 * START PUBLIC ABSTRACTED FUNCTIONS 
 ***************************/

RsslRet rsslSeqMcastSetChannelFunctions()
{
	RsslTransportChannelFuncs funcs;

	funcs.channelBufferUsage = rsslSeqMcastBufferUsage;
	funcs.channelClose = rsslSeqMcastCloseChannel;
	funcs.channelConnect = rsslSeqMcastConnect;
	funcs.channelFlush = rsslSeqMcastFlush;
	funcs.channelGetBuffer = rsslSeqMcastGetBuffer;
	funcs.channelGetInfo = rsslSeqMcastGetChannelInfo;
	funcs.channelIoctl = rsslSeqMcastIoctl;
	funcs.channelPackBuffer = rsslSeqMcastPackBuffer;
	funcs.channelPing = rsslSeqMcastPing;
	funcs.channelRead = rsslSeqMcastRead;
	funcs.channelReconnect = rsslSeqMcastReconnect;
	funcs.channelReleaseBuffer = rsslSeqMcastReleaseBuffer;
	funcs.channelWrite = rsslSeqMcastWrite;
	funcs.initChannel = rsslSeqMcastInitChannel;
	
	return(rsslSetTransportChannelFunc(RSSL_SEQ_MCAST_TRANSPORT,&funcs));
}

RsslRet rsslSeqMcastSetServerFunctions()
{
	RsslTransportServerFuncs funcs;

	funcs.serverAccept = rsslSeqMcastAccept;
	funcs.serverBind = rsslSeqMcastBind;
	funcs.serverIoctl = rsslSeqMcastSrvrIoctl;
	funcs.serverGetInfo = rsslSeqMcastGetSrvrInfo;
	funcs.serverBufferUsage = rsslSeqMcastSrvrBufferUsage;

	return(rsslSetTransportServerFunc(RSSL_SEQ_MCAST_TRANSPORT,&funcs));
}

/* init, uninit, set function pointers */
RsslRet rsslSeqMcastInitialize(RsslLockingTypes lockingType, RsslError *error)
{
	/* initialize mutex functions */
	if (lockingType == RSSL_LOCK_GLOBAL_AND_CHANNEL) /* use actual mutex functions */
	{
		chnlLocking = RSSL_TRUE;
	}
	else /* use dummy mutex functions */
	{
		chnlLocking = RSSL_FALSE;
	}

	rsslSeqMcastSetServerFunctions();
	rsslSeqMcastSetChannelFunctions();

	return RSSL_RET_SUCCESS;
}

/* Sets Sequence Multicast debug dump functions */
RsslRet rsslSetSeqMcastDebugFunctions(
	void(*dumpRsslIn)(const char *functionName, char *buffer, RsslUInt32 length, RsslSocket socketId),
	void(*dumpRsslOut)(const char *functionName, char *buffer, RsslUInt32 length, RsslSocket socketId),
	RsslError *error)
{
	RsslRet retVal = 0;

	if ((dumpRsslIn && rsslSeqMcastDumpInFunc) || (dumpRsslOut && rsslSeqMcastDumpOutFunc))
	{
		_rsslSetError(error, NULL, RSSL_RET_FAILURE, 0);
		snprintf(error->text, MAX_RSSL_ERROR_TEXT, "<%s:%d> rsslSetSeqMcastDebugFunctions() Cannot set shared memory Rssl dump functions.\n", __FILE__, __LINE__);

		retVal = RSSL_RET_FAILURE;
	}
	else
	{
		rsslSeqMcastDumpInFunc = dumpRsslIn;
		rsslSeqMcastDumpOutFunc = dumpRsslOut;
		retVal = RSSL_RET_SUCCESS;
	}

	return retVal;
}

/* Initialization the Sequence Multicast transport debug dump functions' entries. */
void rsslClearSeqMcastDebugFunctionsEx()
{
	memset(rsslSeqMcastDumpFuncs, 0, (sizeof(RsslDumpFuncs) * MAX_PROTOCOL_TYPES));
}

/* Sets Sequence Multicast debug dump functions for the protocol type */
RsslRet rsslSetSeqMcastDebugFunctionsEx(RsslDebugFunctionsExOpts* pOpts, RsslError* error)
{
	RsslRet retVal = RSSL_RET_SUCCESS;

	if ((pOpts->dumpRsslIn && rsslSeqMcastDumpFuncs[pOpts->protocolType].rsslDumpInFunc)
		|| (pOpts->dumpRsslOut && rsslSeqMcastDumpFuncs[pOpts->protocolType].rsslDumpOutFunc))
	{
		_rsslSetError(error, NULL, RSSL_RET_FAILURE, 0);
		snprintf(error->text, MAX_RSSL_ERROR_TEXT, "<%s:%d> rsslSetSeqMcastDebugFunctionsEx() Cannot set sequence multicast Rssl dump functions.\n", __FILE__, __LINE__);
		retVal = RSSL_RET_FAILURE;
	}
	else
	{
		rsslSeqMcastDumpFuncs[pOpts->protocolType].rsslDumpInFunc = pOpts->dumpRsslIn;
		rsslSeqMcastDumpFuncs[pOpts->protocolType].rsslDumpOutFunc = pOpts->dumpRsslOut;
	}

	return retVal;
}

void rsslSeqMcastDumpInFuncImpl(const char* functionName, char* buffer, RsslUInt32 length, RsslSocket socketId, RsslChannel* pChannel)
{
	if (rsslSeqMcastDumpInFunc)
	{
		(*(rsslSeqMcastDumpInFunc))(functionName, buffer, length, socketId);
	}
	if (rsslSeqMcastDumpFuncs[pChannel->protocolType].rsslDumpInFunc)
	{
		(*(rsslSeqMcastDumpFuncs[pChannel->protocolType].rsslDumpInFunc))(functionName, buffer, length, pChannel);
	}
}

void rsslSeqMcastDumpOutFuncImpl(const char* functionName, char* buffer, RsslUInt32 length, RsslSocket socketId, RsslChannel* pChannel)
{
	if (rsslSeqMcastDumpOutFunc)
	{
		(*(rsslSeqMcastDumpOutFunc))(functionName, buffer, length, socketId);
	}
	if (rsslSeqMcastDumpFuncs[pChannel->protocolType].rsslDumpOutFunc)
	{
		(*(rsslSeqMcastDumpFuncs[pChannel->protocolType].rsslDumpOutFunc))(functionName, buffer, length, pChannel);
	}
}
