/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2019 Refinitiv. All rights reserved.            --
 *|-----------------------------------------------------------------------------
 */

#ifndef __ripcutils_h
#define __ripcutils_h


#include "rtr/socket.h"
#include "rtr/rsslChanManagement.h"
#include <sys/stat.h>

#if !defined(_WIN16) && !defined(_WIN32)
#include <sys/uio.h>
#if defined(LINUX)
typedef struct iovec iovec_t;
#endif
#endif


#ifdef _WIN32
#include <stdio.h>
typedef WSABUF	ripcIovType;
#define RIPC_IOV_SETLEN(iov,length) (iov)->len = length
#define RIPC_IOV_GETLEN(iov) (iov)->len
#define RIPC_IOV_SETBUF(iov,buffer) (iov)->buf = buffer
#define RIPC_IOV_GETBUF(iov) (iov)->buf

#define RIPC_MAXIOVLEN 16

#ifndef snprintf
#define snprintf	_snprintf
#endif

#ifndef stat
#define stat		_stat
#endif

#ifndef S_IFDIR
#define S_IFDIR _S_IFDIR
#endif

#ifndef S_IFREG
#define S_IFREG _S_IFREG
#endif

#else

#ifdef IPC_DEBUG_SOCKET_NAME
#include <arpa/inet.h>
#endif

typedef iovec_t	ripcIovType;
#define RIPC_IOV_SETLEN(iov,length) (iov)->iov_len = length
#define RIPC_IOV_GETLEN(iov) (iov)->iov_len
#define RIPC_IOV_SETBUF(iov,buf) (iov)->iov_base = (caddr_t)buf
#define RIPC_IOV_GETBUF(iov) (iov)->iov_base

#endif

#ifndef RIPC_MAXIOVLEN
#define RIPC_MAXIOVLEN 16
#endif

typedef enum {
	RIPC_RW_NONE		= 0x00,
	RIPC_RW_BLOCKING	= 0x01,		/* Perform a blocking operation */
	RIPC_RW_WAITALL		= 0x02		/* If blocking, wait for max_len
									 * read bytes or outLen written bytes.
									 */
} ripcRWFlags;

typedef enum {
	RIPC_SOPT_BLOCKING		= 1,	/* Use turn_on */
	RIPC_SOPT_LINGER		= 2,	/* Use linger_time */
	RIPC_SOPT_REUSEADDR		= 3,	/* Use turn_on */
	RIPC_SOPT_RD_BUF_SIZE	= 4,	/* Use buffer_size */
	RIPC_SOPT_WRT_BUF_SIZE	= 5,	/* Use buffer_size */
	RIPC_SOPT_CLOEXEC		= 6,	/* Use turn_on */
	RIPC_SOPT_TCP_NODELAY	= 7,	/* Use turn_on */
	RIPC_SOPT_EXCLUSIVEADDRUSE = 8,	/* Use Exclusive Address Reuse (WIN) */
	RIPC_SOPT_KEEPALIVE		= 9
} ripcSocketOptionsCode;

typedef struct {
	ripcSocketOptionsCode	code;
	union {
		int			turn_on;		/* turn the option on */
		int			linger_time;	/* linger_time == 0 turns off */
		int			buffer_size;	/* set to buffer size */
	} options;
} ripcSocketOption;


extern int ipcValidServerName(char *name, int nlen);
extern int ipcGetServByName(char *serv_name);
extern int ipcSockOpts( RsslSocket fd, ripcSocketOption *option );
//
extern int ipcBindSocket(u32 ipaddr, int portNum, RsslSocket fd);
extern int ipcReadyWrite(RsslSocket fd);
extern int ipcReadyRead(RsslSocket fd);
extern int ipcReadyException(RsslSocket fd);
extern int ipcConnected(RsslSocket fd);
extern int ipcSetSockFuncs();


#ifdef IPC_DEBUG_SOCKET_NAME
extern int _ipcdGetSockName( RsslSocket fd );
extern int _ipcdGetPeerName( RsslSocket fd );
#endif

// All signature of function definitions for function pointers

extern RsslInt32 ipcSrvrBind(rsslServerImpl *srvr, RsslError *error);

extern void *ipcNewSrvrConn(void *srvr, RsslSocket fd, int *initComplete, void* userSpecPtr, RsslError *error);

extern void *ipcNewClientConn(RsslSocket fd, int *initComplete, void* userSpecPtr, RsslError* error );

extern int ipcInitTrans(void *transport, ripcSessInProg *inPr, RsslError *error);

extern int ipcRead(void *transport, char *buf, int max_len, ripcRWFlags flags, RsslError *error);

extern int ipcWrite(void *transport, char *buf, int outLen, ripcRWFlags flags, RsslError *error);

extern int ipcWriteV(void *transport, ripcIovType *iov, int iovcnt, int outLen, ripcRWFlags flags, RsslError *error);

extern int ipcWrite(void *transport, char *buf, int outLen, ripcRWFlags flags, RsslError *error);

extern int ipcScktReconnectClient(void *transport, RsslError *error);

extern RsslSocket ipcSrvrAccept(rsslServerImpl *srvr, void** userSpecPtr, RsslError *error);

extern int ipcShutdownSckt(void *transport);

/* Not support for ipc Sockets */
extern int ipcSessIoctl(void *transport, int code, int value, RsslError *error);

/* Not support for ipc Sockets */
extern void ipcUninitialize();

extern int ipcGetSockName(RsslSocket fd, struct sockaddr *address, int *address_len, void *transport);

extern int ipcGetSockOpts(RsslSocket fd, int code, int* value, void *transport, RsslError *error);

extern int ipcSetSockOpts(RsslSocket fd, ripcSocketOption *option, void *transport);

extern int ipcIsConnected(RsslSocket fd, void *transport);

extern int ipcServerShutdown(void *transport);

// extern for SSLfuncs.connectSocket 
extern RsslSocket ipcConnectSocket(RsslInt32 *portnum, void *opts, RsslInt32 flags, void** userSpecPtr, RsslError *error);

extern int rssl_socket_startup(char *errorText);
extern int rssl_socket_shutdown();

#endif
