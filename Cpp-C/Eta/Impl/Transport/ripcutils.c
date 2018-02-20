/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright Thomson Reuters 2018. All rights reserved.            --
 *|-----------------------------------------------------------------------------
 */

#include "rtr/ripcplat.h"

#include <stdio.h>
#include <errno.h>
#include <fcntl.h>
#include <stdlib.h>

#include "rtr/socket.h"

#if defined(_WIN32)
#include <ctype.h>
#include <sys/types.h>
#else
#include <sys/types.h>
#include <unistd.h>
#include <netinet/in.h> 
#include <netinet/tcp.h>
#include <ctype.h>
#include <netdb.h>
#include <string.h>
#endif

#ifdef DEV_HAS_POLL
#include <sys/poll.h>
#endif

#include "rtr/rsslSocketTransportImpl.h"
#include "rtr/ripcflip.h"
#include "rtr/ripcutils.h"
#include "rtr/rtratomic.h"

#include "rtr/rsslErrors.h"

#ifdef Linux
#include <asm/ioctls.h>
#endif /* Linux */

static rtr_atomic_val rtr_SocketInits = 0;

#ifdef _WIN32
/************************************************************************
Function:	doSocketStart
Abstract:	This function will initialize Winsock.
************************************************************************/
static int doSocketStart(char *errorText)
{
	WSADATA	wsaData;
	if (WSAStartup(MAKEWORD(2, 2), &wsaData))
	{
		snprintf(errorText, 128, "WSAStartup() Failed (%d).", WSAGetLastError());
		return -1;
	}
	if ((LOBYTE(wsaData.wVersion) != 2) || (LOBYTE(wsaData.wHighVersion) != 2))
	{
		WSACleanup();
		snprintf(errorText, 128, "WSAStartup() does not support version (%d.%d).", 2, 2);
		return -1;
	}
	return 1;
}
#else
static int doSocketStart(char *errorText)
{
	return 1;
}
#endif
/************************************************************************
Function:	rssl_socket_startup
Abstract:	This function will initialize Winsock.
************************************************************************/
int rssl_socket_startup(char *errorText)
{
	int ret = 1;
	if (RTR_ATOMIC_INCREMENT_RET(rtr_SocketInits) == 1)
	{
		ret = doSocketStart(errorText);
		if (ret < 0)
		{
			RTR_ATOMIC_DECREMENT(rtr_SocketInits);
		}
	}
	return ret;
}


/************************************************************************
Function:	rssl_socket_startup
Abstract:	This function will clean up Winsock.
************************************************************************/
int rssl_socket_shutdown()
{
	if (RTR_ATOMIC_DECREMENT_RET(rtr_SocketInits) == 0)
	{
#ifdef _WIN32
		WSACleanup();
#endif
	}
	return 1;
}



/************************************************************************
Function:	ipcValidServerName
Abstract:	This function will check a given server name to make
			 sure it is a possible server name.
Description: This function first makes sure all the characters in
			 the server name are printable and then makes sure the
			 name is not too long and null terminated.
************************************************************************/
int ipcValidServerName(char *name, int nlen)
{
	char *s;
	int  i;

	for (s = name, i = 0; (i <= nlen)&&(*s != (char)0);  ++i)
	{
		if (! isprint(*s++))
			return(-1);
	}

	return( ( (i <= nlen) && (i > 0) ) ? 1 : -1 );
}

/************************************************************************
Function:	ipcGetServByName
Abstract:	This function will get the port number given the service
			 name.
Description: This function getservbyname to get the port structure.
************************************************************************/
int ipcGetServByName(char *serv_name)
{
	struct servent *serv_port;	 /* Service port */

#if defined (x86_Linux_4X) || (x86_Linux_3X) || (x86_Linux_2X)
	struct servent serv_result;
	char tbuf[1024];
#endif

	if (serv_name != (char *)0)
	{
		int port;

		/* Check for port number definition first */
		if ( ((port = atoi(serv_name)) > 0) && (port <= 65535))
		{
			u16 prt=port;
			return host2net_u16(prt);
		}

#if defined (x86_Linux_4X) || defined (x86_Linux_3X) || defined (x86_Linux_2X)
		getservbyname_r(serv_name,"tcp",&serv_result,tbuf,1024,&serv_port);
#else
		serv_port = getservbyname(serv_name,"tcp");
#endif
	}
	else
		serv_port = NULL;

	if (serv_port != NULL)
		return (serv_port->s_port); 
	else if ((serv_name) && ((!strcmp(serv_name, "rmds_rssl_sink")) || (!strcmp(serv_name, "rssl_consumer"))))
		return host2net_u16(14002);
	else if ((serv_name) && ((!strcmp(serv_name, "rmds_rssl_source")) || (!strcmp(serv_name, "rssl_provider"))))
		return host2net_u16(14003);
	else if ((serv_name) && ((!strcmp(serv_name, "triarch_sink")) || (!strcmp(serv_name, "rmds_ssl_sink"))))
		return host2net_u16(8101);
	else if ((serv_name) && ((!strcmp(serv_name, "triarch_src")) || (!strcmp(serv_name, "rmds_ssl_source"))))
		return host2net_u16(8102);
	else if ((serv_name) && (!strcmp(serv_name, "triarch_dbms")))
		return host2net_u16(8103);
	else if ((serv_name) && (!strcmp(serv_name, "triarch_rrbp")))
		return host2net_u16(8500);

	return(-1);
}

#ifdef DEV_SVR4
#define RIPCFCNTL_NBLOCK_FLAG O_NONBLOCK
#endif

/* Not support for ipc Sockets */
int ipcSessIoctl(void *transport, int code, int value, RsslError *error)
{
	_rsslSetError(error, NULL, RSSL_RET_FAILURE,  errno);
	snprintf(error->text, MAX_RSSL_ERROR_TEXT, "<%s:%d> ipcSessIoctl() Error: 1002 ipcSessIoctl() Not supported", __FILE__, __LINE__);
	return  RSSL_RET_FAILURE;
}

/* Not support for ipc Sockets */
void ipcUninitialize()
{
	return;
}

int ipcGetSockName(RsslSocket fd, struct sockaddr *address, int *address_len, void *transport)
{
	RsslInt32 ret = RSSL_RET_SUCCESS;

	if (getsockname(fd, address, (socklen_t*)address_len) < 0)
		ret = RSSL_RET_FAILURE;

	return ret;
}

int ipcGetSockOpts(RsslSocket fd, int code, int* value, void *transport, RsslError *error)
{
	RsslInt32 size = 0;
	RsslInt32 ret = RSSL_RET_SUCCESS;
#ifdef Linux
	socklen_t len = 0;
#else
	RsslInt32 len = 0;
#endif

	*value = size;
	len = sizeof(size);

	switch (code)
	{
		case RIPC_SYSTEM_READ_BUFFERS:
			if (getsockopt(fd, SOL_SOCKET, SO_RCVBUF, (char*)&size, &len) < 0)
			{
				_rsslSetError(error, NULL, RSSL_RET_FAILURE,  errno);
				snprintf(error->text, MAX_RSSL_ERROR_TEXT, "<%s:%d> ipcGetSockOpts() Error: 1002 getsockopt() SOO_RCVBUF failed.  System errno: (%d)", __FILE__, __LINE__, errno);
				ret = RSSL_RET_FAILURE;
			}
			else
				*value = size;
		break;

		case RIPC_SYSTEM_WRITE_BUFFERS:
			if (getsockopt(fd, SOL_SOCKET, SO_SNDBUF, (char*)&size, &len) < 0)
			{
				_rsslSetError(error, NULL, RSSL_RET_FAILURE,  errno);
				snprintf(error->text, MAX_RSSL_ERROR_TEXT, "<%s:%d> ipcGetSockOpts() Error: 1002 getsockopt() SOO_SNDBUF failed.  System errno: (%d)", __FILE__, __LINE__, errno);
				ret = RSSL_RET_FAILURE;
			}
			else
				*value = size;
		break;

		default:
			ret = RSSL_RET_FAILURE;
	}

	return(ret);
}

int ipcSetSockOpts(RsslSocket fd, ripcSocketOption *option, void *transport)
{
	return ipcSockOpts(fd, option);
}

int ipcSockOpts(RsslSocket fd, ripcSocketOption *option)
{
	int ret=1;

	switch (option->code)
	{
		case RIPC_SOPT_BLOCKING:
		{
#ifdef RIPCFCNTL_NBLOCK_FLAG
			if (option->options.turn_on)
			{
				if (fcntl(fd,F_SETFL,(fcntl(fd,F_GETFL) & ~RIPCFCNTL_NBLOCK_FLAG))<0)
					ret=-1;
			}
			else
			{
				if (fcntl(fd,F_SETFL,(fcntl(fd,F_GETFL) | RIPCFCNTL_NBLOCK_FLAG))<0)
					ret=-1;
			}
#else
			char *no_wait=(char*)1;
			int rc = 1;

			if (option->options.turn_on)
			{
				no_wait=(char*)0;
				rc = 0;
			}
#if defined(_WIN32) || defined(_WINDOWS)
			if (ioctl(fd, FIONBIO, (unsigned long *)&no_wait) < 0)
				ret=-1;
#else
			if (ioctl(fd, FIONBIO, (char *)&no_wait) < 0)
				ret=-1;
#endif
#endif
			break;
		}

		case RIPC_SOPT_CLOEXEC:
		{
#if defined(Linux)
			if (option->options.turn_on)
			{
				if (fcntl(fd,F_SETFD,(fcntl(fd,F_GETFD) | FD_CLOEXEC))<0)
				ret=-1;
			}
			else
			{
				if (fcntl(fd,F_SETFD,(fcntl(fd,F_GETFD) & ~FD_CLOEXEC))<0)
				ret=-1;
			}
#endif
			break;
		}

		case RIPC_SOPT_LINGER:
		{
			struct linger lo;

			lo.l_onoff = (option->options.linger_time ? 1 : 0);
			lo.l_linger = option->options.linger_time;
			if (setsockopt(fd,SOL_SOCKET,SO_LINGER,(char *)&lo,(int)sizeof(lo))<0)
				ret = -1;
			break;
		}

		case RIPC_SOPT_REUSEADDR:
		{
			int reuseFlag = (option->options.turn_on ? 1 : 0);
			if (setsockopt(fd,SOL_SOCKET,SO_REUSEADDR,(char *)&reuseFlag,
								(int)sizeof(reuseFlag)) < 0)
				ret = -1;
			break;
		}

#if defined(_WIN32) && defined(SO_EXCLUSIVEADDRUSE)
		case RIPC_SOPT_EXCLUSIVEADDRUSE:
		{
			int reuseExclusiveFlag = (option->options.turn_on ? 1 : 0);
			if (setsockopt(fd,SOL_SOCKET,SO_EXCLUSIVEADDRUSE,(char *)&reuseExclusiveFlag,
								(int)sizeof(reuseExclusiveFlag)) < 0)
				ret = -1;
			break;
		}
#endif

		case RIPC_SOPT_RD_BUF_SIZE:
		{
			int maxBuf = option->options.buffer_size;
			if (setsockopt(fd,SOL_SOCKET,SO_RCVBUF,(char*)&maxBuf,
								(int)sizeof(maxBuf)) < 0)
				ret = -1;
			break;
		}

		case RIPC_SOPT_WRT_BUF_SIZE:
		{
			int maxBuf = option->options.buffer_size;
			if (setsockopt(fd,SOL_SOCKET,SO_SNDBUF,(char*)&maxBuf,
								(int)sizeof(maxBuf)) < 0)
				ret = -1;
			break;
		}

		case RIPC_SOPT_TCP_NODELAY:
		{
			int nodelayFlag = (option->options.turn_on ? 1 : 0);
			if (setsockopt(fd, IPPROTO_TCP, TCP_NODELAY, (char*)&nodelayFlag, sizeof(nodelayFlag)) < 0)
				ret = -1;
			break;
		}

		case RIPC_SOPT_KEEPALIVE:
		{
			int keepalive = (option->options.turn_on ? 1 : 0);
			if (setsockopt(fd, SOL_SOCKET, SO_KEEPALIVE, (char*)&keepalive, sizeof(keepalive)) < 0)
				ret = -1;
			break;
		}

		default:
			ret = -1;
			break;
	}
	return(ret);
}

int ipcBindSocket(u32 ipaddr, int portNum, RsslSocket fd)
{
	struct sockaddr_in baddr;
	
	/* bind() needs the address struct in network byte order */
  	baddr.sin_family = AF_INET;
	baddr.sin_addr.s_addr = ipaddr;
	baddr.sin_port = (u16)portNum;

	if (bind(fd,(struct sockaddr*)&baddr,(int)sizeof(baddr)) < 0)
		return(-1);

	return(1);
}

int ipcIsConnected(RsslSocket fd, void *transport)
{
	return ipcConnected(fd);
}

int ipcServerShutdown(void *transport)
{
	RsslServerSocketChannel *rsslServerSocketChannel = (RsslServerSocketChannel *)transport;

	if (rsslServerSocketChannel->stream != RIPC_INVALID_SOCKET)
	{
		sock_close(rsslServerSocketChannel->stream);
		ipcCloseActiveSrvr(rsslServerSocketChannel);
	}

	return 1;
}

/* This call is meant to be used after a non-blocking connect()
 * system call to see if the connection has succeeded, failed,
 * or is still working.
 *
 * Returns : 1 if fd is connected
 *		   0 if fd is not connected and is working on connecting
 *		   -1 if error
 */
int ipcConnected(RsslSocket fd)
{
	struct sockaddr_in baddr;
	int size=sizeof(baddr);
	int retval;

	/* When a non-blocking connect is outstanding,
	 * ready for writing is set to false.
	 * However, when ready for writing is set to
	 * true, the operation is complete. We use
	 * getpeername() to see if it succeeded or failed.
	 */
	retval=ipcReadyWrite(fd);
	if (retval == 1)
	{
		if (getpeername(fd,(struct sockaddr*)&baddr, (socklen_t*)&size) < 0)
		{
#ifdef _WIN32

			/* When a getpeername() fails, the socket 
   			 * connection has failed. However, on NT,
			 * the OS may have accepted the connection 
 			 * but not the application. We must then check
			 * to see if the socket is ready for exception
			 * to determine if the conenction is still 
			 * in progress.
			 */
			if (ipcReadyException(fd) != 0)
			{
				/* if this is positive, we should try to read in order to get the errno */
				if (retval == 1)
				{
					/* this should fail because it was an exception */
					char tempBuf[10];
					SOCK_RECV(fd, tempBuf, 10, 0); 
				}
				/* if we dont do the read above, errno should already be set from select */
				return(-1);
			}
			return(0);
#else
			return(-1);
#endif
		}
		return(1);
	}
	else if (retval == 0)
	{
#ifdef _WIN32
			/* On NT, a nonblocking connect has failed if
			 * not ready for writing and there is an exception.
			 */
		if ((retval = ipcReadyException(fd)) != 0)
		{
			/* if this is positive, we should try to read in order to get the errno */
			if (retval == 1)
			{
				/* this should fail because it was an exception */
				char tempBuf[10];
				SOCK_RECV(fd, tempBuf, 10, 0); 
			}
			/* if we dont do the read above, errno should already be set from select */
			return(-1);
		}
#endif
		return(0);
	}

	return(-1);
}

void *ipcNewSrvrConn(void *srvr, RsslSocket fd,
								int *initComplete, void* userSpecPtr, RsslError *error)
{
	*initComplete = 1;
	return((void*)(intptr_t)fd);
}

void *ipcNewClientConn(RsslSocket fd, int *initComplete, void* userSpecPtr, RsslError* error )
{
	*initComplete = 1;
	return((void*)(intptr_t)fd);
}

int ipcInitTrans(void *transport, ripcSessInProg *inPr, RsslError *error)
{
	return(1);
}

int ipcRead( void *transport, char *buf, int max_len, ripcRWFlags flags, RsslError *error)
{
#ifdef _WIN32WSA
	WSABUF wsabuf;
	int retval;
	DWORD numBytes;
	DWORD totalBytes = 0;
	DWORD dflags = 0;
#else
#ifdef WIN32
	int numBytes;
#else
	ssize_t numBytes;
#endif
	int totalBytes = 0;
#endif

	while((int)totalBytes < max_len)
	{
#ifdef _WIN32WSA
		wsabuf.buf = buf + totalBytes;
		wsabuf.len = max_len - totalBytes;

		retval = WSARecv((RsslSocket)transport,&wsabuf,1,&numBytes,&dflags,NULL,NULL);

		if (retval == SOCKET_ERROR)
		{
			int error = WSAGetLastError();
			error->errorText[0] = '\0';
			if ((error == WSAEWOULDBLOCK) || (error == WSAEINTR))
			{
				if (!(flags & RIPC_RW_BLOCKING))
					return(totalBytes);
			}
			else if (error == WSAESHUTDOWN)
			{
				return(-2);
			}
			else
			{
				return(-1);
			}
		}
		else if (numBytes == 0)
		{
			if (totalBytes)
				return totalBytes;
			else
			{
				error->errorText[0] = '\0';
				return(-2);
			}
		}
		else
			totalBytes += numBytes;
#else
		numBytes = SOCK_RECV((RsslSocket)(intptr_t)transport, (buf + totalBytes), (size_t)(max_len - totalBytes), 0);

		if (numBytes > 0)
			totalBytes += numBytes;
		else if (numBytes < 0)
		{
			error->text[0] = '\0';
				/* Check for EINPROGRESS on windows? */
			if ((errno == _IPC_WOULD_BLOCK) || (errno == EINTR))
			{
				if (!(flags & RIPC_RW_BLOCKING))
					return(totalBytes);
			}
			else
			{
				return(-1);
			}
		}
		else	/* numBytes == 0 */
		{
			if (totalBytes)
				return totalBytes;
			else
			{
				error->text[0] = '\0';
				return(-2);
			}
		}
#endif

		if ((flags & RIPC_RW_BLOCKING) && (totalBytes != 0) && (!(flags & RIPC_RW_WAITALL)))
			break;
	}

	return((int)totalBytes);
}

int ipcWriteV( void *transport, ripcIovType *iov, int iovcnt, int outLen, ripcRWFlags flags, RsslError *error)
{
#ifdef _WIN32
	int		retval;
#endif	
	int numBytes = 0;
	int totOut = 0;

ripcwritevagain:

/* Debugging
	printf("Writev %d\n",iovcnt);
	for (int i=0;i<iovcnt;i++)
	{
		int j;
		printf("Base %2.2x Len %d ",iov[i].iov_base,iov[i].iov_len);
		for (j=0;j<40;j+=2)
			printf(" %2.2x%2.2x",iov[i].iov_base[j],iov[i].iov_base[j+1]);
		printf("\n");
	}
	*/

#ifdef _WIN32
	retval = WSASend((RsslSocket)transport, iov, iovcnt, &((DWORD)numBytes), 0, 0, 0);
	if (retval < 0)
		numBytes = retval;
#else
	numBytes = writev((RsslSocket)(intptr_t)transport,iov,iovcnt);
#endif

/* Debugging
	printf("  retval %d\n",numBytes);
	fflush(stdout);
	*/

	if (numBytes > 0)
		totOut += numBytes;
	else if (numBytes < 0)
	{
		if ((errno == _IPC_WOULD_BLOCK) || (errno == EINTR))
		{
			if (flags & RIPC_RW_BLOCKING)
				goto ripcwritevagain;
		}
		else
		{
			error->text[0] = '\0';
			totOut = -1;
		}
	}
	else /* numBytes == 0 */
	{
#ifdef _WIN32
		if ((errno == _IPC_WOULD_BLOCK) && (flags & RIPC_RW_BLOCKING))
			goto ripcwritevagain;
#endif
		error->text[0] = '\0';
		totOut = -2;
	}

	return(totOut);
}

int ipcWrite( void *transport, char *buf, int outLen, ripcRWFlags flags, RsslError *error)
{
#ifdef _WIN32WSA
	WSABUF wsabuf;
	int retval;
	DWORD numBytes;
	DWORD totOut = 0;
#else
	int numBytes;
	int totOut = 0;
#endif

	while((int)totOut < outLen)
	{
#ifdef _WIN32WSA
		wsabuf.buf = buf + totOut;
		wsabuf.len = outLen - totOut;

		retval = WSASend((RsslSocket)transport,&wsabuf,1,&numBytes,0,0,0);

		if (retval == SOCKET_ERROR)
		{
			int error = WSAGetLastError();
			error->errorText[0] = '\0';
			if ((error == WSAEWOULDBLOCK) || (error == WSAEINTR))
			{
				if (!(flags & RIPC_RW_BLOCKING))
					return(totOut);
			}
			else if (error == WSAESHUTDOWN)
			{
				return(-2);
			}
			else
			{
				return(-1);
			}
		}
		else
			totOut += numBytes;

#else

		numBytes = SOCK_SEND((RsslSocket)(intptr_t)transport, (buf + totOut), (outLen - totOut), 0);

		if (numBytes > 0)
			totOut += numBytes;
		else if (numBytes < 0)
		{	
			error->text[0] = '\0';
			if ((errno == _IPC_WOULD_BLOCK) || (errno == EINTR))
			{
				if (!(flags & RIPC_RW_BLOCKING))
					return(totOut);
			}
			else
			{
				return(-1);
			}
		}
		else	/* numBytes == 0 */
		{
#ifdef _WIN32
			if ((errno == _IPC_WOULD_BLOCK) && (!(flags & RIPC_RW_BLOCKING)))
				return(totOut);
#endif
			error->text[0] = '\0';
			return(-2);
		}
#endif

		if ((flags & RIPC_RW_BLOCKING) && (totOut != 0) && (!(flags & RIPC_RW_WAITALL)))
			break;
	}

	return(totOut);
}

int ipcShutdownSckt(void *transport)
{
	sock_close((RsslSocket)(intptr_t)transport);
	return(1);
}

int ipcScktReconnectClient(void *transport, RsslError *error)
{
	/* for normal socket connections there is really nothing to do for this -
	   its just here in case we call it for normal connections also */
	return 1;
}

RsslInt32 ipcSrvrBind(rsslServerImpl *srvr, RsslError *error)
{
	RsslSocket			sock_fd;
	RsslInt32			portnum;
	RsslUInt32			addr;
	ripcSocketOption	sockopts;

	RsslServerSocketChannel*	rsslServerSocketChannel = (RsslServerSocketChannel*)srvr->transportInfo;

#ifdef IPC_DEBUG_SOCKET_NAME
	char* tmp;
	char localAddrStr[129];
	struct in_addr inaddr;
	RsslUInt32 localAddr;
	char* interfaceName;
#endif

	sock_fd = socket(AF_INET, SOCK_STREAM, getProtocolNumber());

	if (!ripcValidSocket(sock_fd))
	{
		_rsslSetError(error, NULL, RSSL_RET_FAILURE, errno);
		snprintf(error->text, MAX_RSSL_ERROR_TEXT,
			"<%s:%d> Error: 1002 Call to socket() failed System errno: (%d)\n",
			__FILE__, __LINE__, errno);

		return -1;
	}

#if defined(_WIN32)
	sockopts.code = RIPC_SOPT_EXCLUSIVEADDRUSE;
	sockopts.options.turn_on = 1;
	if (ipcSockOpts(sock_fd, &sockopts) < 0)
	{
		_rsslSetError(error, NULL, RSSL_RET_FAILURE, errno);
		snprintf(error->text, MAX_RSSL_ERROR_TEXT,
			"<%s:%d> Error: 1002 Could not to set SO_EXCLUSIVEADDRUSE on socket. System errno: (%d)\n",
			__FILE__, __LINE__, errno);

		sock_close(sock_fd);
		return -1;
	}
#else
	sockopts.code = RIPC_SOPT_REUSEADDR;
	sockopts.options.turn_on = 1;
	if (ipcSockOpts(sock_fd, &sockopts) < 0)
	{
		_rsslSetError(error, NULL, RSSL_RET_FAILURE, errno);
		snprintf(error->text, MAX_RSSL_ERROR_TEXT,
			"<%s:%d> Error: 1002 Could not to set SO_REUSEADDR on socket. System errno: (%d)\n",
			__FILE__, __LINE__, errno);

		sock_close(sock_fd);
		return -1;
	}
#endif

	sockopts.code = RIPC_SOPT_LINGER;
	sockopts.options.linger_time = 0;
	if (ipcSockOpts(sock_fd, &sockopts) < 0)
	{
		_rsslSetError(error, NULL, RSSL_RET_FAILURE, errno);
		snprintf(error->text, MAX_RSSL_ERROR_TEXT,
			"<%s:%d> Error: 1002 Could not set SO_LINGER time to 0 on socket. System errno: (%d)\n",
			__FILE__, __LINE__, errno);

		sock_close(sock_fd);
		return -1;
	}

	if ((portnum = ipcGetServByName(rsslServerSocketChannel->serverName)) == -1)
	{
		_rsslSetError(error, NULL, RSSL_RET_FAILURE, errno);
		snprintf(error->text, MAX_RSSL_ERROR_TEXT,
			"<%s:%d> Error: 1004 ipcGetServByName() failed. Port number is incorrect. (%d)\n",
			__FILE__, __LINE__, errno);

		sock_close(sock_fd);
		return -1;
	}

	if (rsslGetHostByName(rsslServerSocketChannel->interfaceName, &addr) < 0)
	{
		_rsslSetError(error, NULL, RSSL_RET_FAILURE, errno);
		snprintf(error->text, MAX_RSSL_ERROR_TEXT,
			"<%s:%d> Error: 1004 rsslGetHostByName() failed. Interface name is incorrect (%d)\n",
			__FILE__, __LINE__, errno);

		sock_close(sock_fd);
		return -1;
	}

#ifdef IPC_DEBUG_SOCKET_NAME
	interfaceName = rsslServerSocketChannel->interfaceName;
	localAddr = addr;
	inaddr.s_addr = localAddr;
	tmp = (char*)inet_ntoa(inaddr);
	strncpy(localAddrStr, tmp, 129);
	printf("<%s:%d> ipcSrvrBind() ipcHostByName(interfaceName=%s) returns localAddr = %s\n", __FILE__, __LINE__, interfaceName ? interfaceName : "NULL", localAddrStr);

	if (interfaceName && (strcmp(interfaceName, "127.0.0.1") == 0))
	{
		printf("<%s:%d> ipcSrvrBind(interfaceName==127.0.0.1) localAddr = %s => INADDR_LOOPBACK = 127.0.0.1\n", __FILE__, __LINE__, localAddrStr);
		localAddr = host2net_u32(INADDR_LOOPBACK);
	}
	else if (localAddr == host2net_u32(INADDR_LOOPBACK))
	{
		printf("<%s:%d> ipcSrvrBind(localAddr == INADDR_LOOPBACK) localAddr = %s => INADDR_ANY = 0.0.0.0\n", __FILE__, __LINE__, localAddrStr);
		localAddr = host2net_u32(INADDR_ANY);
	}
	addr = localAddr;
#else
	if (rsslServerSocketChannel->interfaceName && (strcmp(rsslServerSocketChannel->interfaceName, "127.0.0.1") == 0))
		addr = host2net_u32(INADDR_LOOPBACK);
	else if (addr == host2net_u32(INADDR_LOOPBACK))
		addr = host2net_u32(INADDR_ANY);
#endif

	if (ipcBindSocket(addr, portnum, sock_fd) < 0)
	{
		_rsslSetError(error, NULL, RSSL_RET_FAILURE, errno);
		snprintf(error->text, MAX_RSSL_ERROR_TEXT,
			"<%s:%d> Error: 1002 Unable to bind socket. System errno: (%d)\n",
			__FILE__, __LINE__, errno);

		sock_close(sock_fd);
		return -1;
	}

#ifdef IPC_DEBUG_SOCKET_NAME
	_ipcdGetSockName(sock_fd);
#endif

	if (ipcSessSetMode(sock_fd, rsslServerSocketChannel->server_blocking, rsslServerSocketChannel->tcp_nodelay, error, __LINE__) < 0)
	{
		sock_close(sock_fd);
		return -1;
	}

	if (listen(sock_fd, 1024) < 0)
	{
		_rsslSetError(error, NULL, RSSL_RET_FAILURE, errno);
		snprintf(error->text, MAX_RSSL_ERROR_TEXT,
			"<%s:%d> Error: 1002 Unable to listen on socket. System errno: (%d)\n",
			__FILE__, __LINE__, errno);

		sock_close(sock_fd);
		return -1;
	}

	rsslServerSocketChannel->stream = sock_fd;

#ifdef RIPC_SSL_ENABLED
	if (rsslServerSocketChannel->connType == RIPC_CONN_TYPE_ENCRYPTED)
	{
		if (getSSLTransFuncs()->newSSLServer == 0)
		{
			_rsslSetError(error, NULL, RSSL_RET_FAILURE, 0);
			snprintf(error->text, MAX_RSSL_ERROR_TEXT,
				"<%s:%d> Cannot enable ssl. Secure Sockets layer library not initialized. ",
				__FILE__, __LINE__);

			sock_close(rsslServerSocketChannel->stream);
			relRsslServerSocketChannel(rsslServerSocketChannel);
		}
		else
		{
			rsslServerSocketChannel->connType = RIPC_CONN_TYPE_ENCRYPTED;
			rsslServerSocketChannel->transportInfo = (*(getSSLTransFuncs()->newSSLServer))(rsslServerSocketChannel->stream, rsslServerSocketChannel->serverName, error);

			if (rsslServerSocketChannel->transportInfo == 0)
			{
				_rsslSetError(error, NULL, RSSL_RET_FAILURE, errno);

				sock_close(rsslServerSocketChannel->stream);
				relRsslServerSocketChannel(rsslServerSocketChannel);
			}
		}
	}
#endif 

	return 0;
}

void ipcSrvrShutdownError(rsslServerImpl* srvr)
{
	RsslServerSocketChannel* pRsslServerSocketChannel = 0;

	if (srvr)
	{
		pRsslServerSocketChannel = (RsslServerSocketChannel*)srvr->transportInfo;

		if (pRsslServerSocketChannel)
		{
			RsslSocket serverFD = (RsslSocket)(pRsslServerSocketChannel->stream);

			sock_close(serverFD);
		}
	}
}

RsslSocket ipcSrvrAccept(rsslServerImpl *srvr, void** userSpecPtr, RsslError *error)
{
	RsslSocket	    fdtemp;
	ripcSocketOption	sockopts;
	RsslServerSocketChannel	*rsslServerSocketChannel = (RsslServerSocketChannel*)srvr->transportInfo;

#ifdef MUTEX_DEBUG
	printf("UNLOCK rsslServerSocketChannel -- ipcSrvrAccept (before accept)\n");
#endif
	IPC_MUTEX_UNLOCK(rsslServerSocketChannel);

	fdtemp = accept(rsslServerSocketChannel->stream, (struct sockaddr *)0, (socklen_t *)0);

#ifdef MUTEX_DEBUG
	printf("LOCK rsslServerSocketChannel -- ipcSrvrAccept (after accept)\n");
#endif
	IPC_MUTEX_LOCK(rsslServerSocketChannel);

	if (rsslServerSocketChannel->stream == RIPC_INVALID_SOCKET)
	{
		if (ripcValidSocket(fdtemp))
			sock_close(fdtemp);

		_rsslSetError(error, NULL, RSSL_RET_FAILURE, errno);
		snprintf(error->text, MAX_RSSL_ERROR_TEXT,
			"<%s:%d> Error: 1002 accept() failed due to server shutting down. System errno: (%d)\n",
			__FILE__, __LINE__, errno);

		ipcCloseActiveSrvr(rsslServerSocketChannel);
		/* rest of cleanup done after return */
		return(0);
	}

	if (!ripcValidSocket(fdtemp))
	{
		if ((errno == _IPC_WOULD_BLOCK) || (errno == EINTR))
		{
			_rsslSetError(error, NULL, RSSL_RET_FAILURE, errno);
			snprintf(error->text, MAX_RSSL_ERROR_TEXT,
				"<%s:%d> Error: 1002 accept() would block. System errno: (%d)\n",
				__FILE__, __LINE__, errno);
		}
		else
		{
			snprintf(error->text, MAX_RSSL_ERROR_TEXT,
				"<%s:%d> Error: 1002 accept() failed. System errno: (%d)\n",
				__FILE__, __LINE__, errno);

			switch (errno)
			{
			case EINVAL:
				_rsslSetError(error, NULL, RSSL_RET_FAILURE, errno);
				break;
			default:
				_rsslSetError(error, NULL, RSSL_RET_FAILURE, errno);
				break;
			}
		}
		return(0);
	}

	if (rtrUnlikely(getConndebug()))
		printf("\nripcNewUser: accept client "SOCKET_PRINT_TYPE"\n", fdtemp);

	if (ipcSessSetMode(fdtemp, rsslServerSocketChannel->session_blocking, rsslServerSocketChannel->tcp_nodelay, error, __LINE__) < 0)
	{
		sock_close(fdtemp);
		return(0);
	}

	sockopts.code = RIPC_SOPT_KEEPALIVE;
	sockopts.options.turn_on = 1;
	if (ipcSockOpts(fdtemp, &sockopts) < 0)
	{
		_rsslSetError(error, NULL, RSSL_RET_FAILURE, errno);
		snprintf(error->text, MAX_RSSL_ERROR_TEXT,
			"<%s:%d> Error: 1002 Could not set SO_KEEPALIVE on socket. System errno:(%d)\n",
			__FILE__, __LINE__, errno);

		sock_close(fdtemp);
		return(0);
	}

	/* if non-zero, set this */
	if (rsslServerSocketChannel->recvBufSize > 0)
	{
		sockopts.code = RIPC_SOPT_RD_BUF_SIZE;
		sockopts.options.buffer_size = rsslServerSocketChannel->recvBufSize;

		if (ipcSockOpts(fdtemp, &sockopts) < 0)
		{
			_rsslSetError(error, NULL, RSSL_RET_FAILURE, errno);
			snprintf(error->text, MAX_RSSL_ERROR_TEXT,
				"<%s:%d> Error: 1002 Unable to set receive buffer size to (%d). System errno: (%d)\n",
				__FILE__, __LINE__, rsslServerSocketChannel->recvBufSize, errno);

			sock_close(fdtemp);
			return(0);
		}
	}

	/* if non-zero set this */
	if (rsslServerSocketChannel->sendBufSize > 0)
	{
		sockopts.code = RIPC_SOPT_WRT_BUF_SIZE;
		sockopts.options.buffer_size = rsslServerSocketChannel->sendBufSize;

		if (ipcSockOpts(fdtemp, &sockopts) < 0)
		{
			_rsslSetError(error, NULL, RSSL_RET_FAILURE, errno);
			snprintf(error->text, MAX_RSSL_ERROR_TEXT,
				"<%s:%d> Error: 1002 Unable to set send buffer size to (%d). System errno: (%d)\n",
				__FILE__, __LINE__, rsslServerSocketChannel->sendBufSize, errno);

			sock_close(fdtemp);
			return(0);
		}
	}

	*userSpecPtr = 0;
	return fdtemp;
}

/* This call is meant to connect to a socket
*
* Returns : portnum if connected
*		   0 if not connected to a socket
*/
RsslSocket ipcConnectSocket(RsslInt32 *portnum, void *opts, RsslInt32 flags, void** userSpecPtr, RsslError *error)
{
	RsslSocket			sock_fd;
	RsslUInt32 			addr;
	struct				sockaddr_in	toaddr;
	ripcSocketOption	sockopts;
	RsslInt32	tcp_nodelay = (flags & RIPC_INT_CS_FLAG_TCP_NODELAY);
	RsslInt32	blocking = (flags & RIPC_INT_CS_FLAG_BLOCKING);

	RsslUInt32	localAddr;
	RsslUInt16	localPort = 0;
	RsslSocketChannel* pRsslSocketChannel = (RsslSocketChannel*)opts;

#ifdef IPC_DEBUG_SOCKET_NAME
	char* tmp;
	char localAddrStr[129];
	struct in_addr inaddr;
	char localHostName[256];
#endif

	sock_fd = socket(AF_INET, SOCK_STREAM, getProtocolNumber());

	if (!ripcValidSocket(sock_fd))
	{
		_rsslSetError(error, NULL, RSSL_RET_FAILURE, errno);
		snprintf(error->text, MAX_RSSL_ERROR_TEXT,
			"<%s:%d> ipcConnectSocket() Error: 1002 socket() failed. System errno: (%d)\n",
			__FILE__, __LINE__, errno);

		return(0);
	}

	sockopts.code = RIPC_SOPT_LINGER;
	sockopts.options.linger_time = 0;
	if (ipcSockOpts(sock_fd, &sockopts) < 0)
	{
		_rsslSetError(error, NULL, RSSL_RET_FAILURE, errno);
		snprintf(error->text, MAX_RSSL_ERROR_TEXT,
			"<%s:%d> Error: 1002 Could not set SO_LINGER time to 0 on socket. System errno: (%d)\n",
			__FILE__, __LINE__, errno);

		sock_close(sock_fd);
		return(0);
	}

	sockopts.code = RIPC_SOPT_KEEPALIVE;
	sockopts.options.turn_on = 1;
	if (ipcSockOpts(sock_fd, &sockopts) < 0)
	{
		_rsslSetError(error, NULL, RSSL_RET_FAILURE, errno);
		snprintf(error->text, MAX_RSSL_ERROR_TEXT,
			"<%s:%d> Error: 1002 Could not set SO_KEEPALIVE time to 0 on socket. System errno: (%d)\n",
			__FILE__, __LINE__, errno);

		sock_close(sock_fd);
		return(0);
	}

	/* if non-zero, set this */
	if (pRsslSocketChannel->recvBufSize > 0)
	{
		sockopts.code = RIPC_SOPT_RD_BUF_SIZE;
		sockopts.options.buffer_size = pRsslSocketChannel->recvBufSize;

		if (ipcSockOpts(sock_fd, &sockopts) < 0)
		{
			_rsslSetError(error, NULL, RSSL_RET_FAILURE, errno);
			snprintf(error->text, MAX_RSSL_ERROR_TEXT,
				"<%s:%d> Error: 1002 Could not set receive buffer size to (%d) on socket. System errno: (%d)\n",
				__FILE__, __LINE__, pRsslSocketChannel->recvBufSize, errno);

			sock_close(sock_fd);
			return(0);
		}
	}

	/* if non-zero set this */
	if (pRsslSocketChannel->sendBufSize > 0)
	{
		sockopts.code = RIPC_SOPT_WRT_BUF_SIZE;
		sockopts.options.buffer_size = pRsslSocketChannel->sendBufSize;

		if (ipcSockOpts(sock_fd, &sockopts) < 0)
		{
			_rsslSetError(error, NULL, RSSL_RET_FAILURE, errno);
			snprintf(error->text, MAX_RSSL_ERROR_TEXT,
				"<%s:%d> Error: 1002 Could not set send buffer size to (%d) on socket. System errno: (%d)\n",
				__FILE__, __LINE__, pRsslSocketChannel->sendBufSize, errno);

			sock_close(sock_fd);
			return(0);
		}
	}

	if (ipcSessSetMode(sock_fd, blocking, tcp_nodelay, error, __LINE__) < 0)
	{
		sock_close(sock_fd);
		return(0);
	}

	if (pRsslSocketChannel->proxyHostName != 0 && (pRsslSocketChannel->proxyHostName[0] != '\0'))
	{
		if (rsslGetHostByName(pRsslSocketChannel->proxyHostName, &addr) < 0)
		{
			_rsslSetError(error, NULL, RSSL_RET_FAILURE, errno);
			snprintf(error->text, MAX_RSSL_ERROR_TEXT,
				"<%s:%d> Error: 1004 rsslGetHostByName() failed. Host name is incorrect. System errno: (%d)\n",
				__FILE__, __LINE__, errno);

			sock_close(sock_fd);
			return(0);
		}
	}
	else
	{
		if (rsslGetHostByName(pRsslSocketChannel->hostName, &addr) < 0)
		{
			_rsslSetError(error, NULL, RSSL_RET_FAILURE, errno);
			snprintf(error->text, MAX_RSSL_ERROR_TEXT,
				"<%s:%d> Error: 1004 rsslGetHostByName() failed. Host name is incorrect. System errno: (%d)\n",
				__FILE__, __LINE__, errno);

			sock_close(sock_fd);
			return(0);
		}
	}

	if (pRsslSocketChannel->proxyPort != 0 && (pRsslSocketChannel->proxyPort[0] != '\0'))
	{
		if ((*portnum = ipcGetServByName(pRsslSocketChannel->proxyPort)) == -1)
		{
			_rsslSetError(error, NULL, RSSL_RET_FAILURE, errno);
			snprintf(error->text, MAX_RSSL_ERROR_TEXT,
				"<%s:%d> Error: 1004 ipcGetServByName() failed. Port name/number is incorrect. System errno: (%d)\n",
				__FILE__, __LINE__, errno);

			sock_close(sock_fd);
			return(0);
		}
	}
	else
	{
		if ((*portnum = ipcGetServByName(pRsslSocketChannel->serverName)) == -1)
		{
			_rsslSetError(error, NULL, RSSL_RET_FAILURE, errno);
			snprintf(error->text, MAX_RSSL_ERROR_TEXT,
				"<%s:%d> Error: 1004 ipcGetServByName() failed. Port name/number is incorrect. System errno: (%d)\n",
				__FILE__, __LINE__, errno);

			sock_close(sock_fd);
			return(0);
		}
	}

#ifdef IPC_DEBUG_SOCKET_NAME
	if (gethostname(localHostName, 256))
		printf("<%s:%d> ipcConnectSocket() hostname() returns ERROR = %d\n", __FILE__, __LINE__, errno);
	else
		printf("<%s:%d> ipcConnectSocket() hostname() returns localHostName = %s\n", __FILE__, __LINE__, localHostName);
#endif

	if (rsslGetHostByName(pRsslSocketChannel->interfaceName, &localAddr) < 0)
	{
		_rsslSetError(error, NULL, RSSL_RET_FAILURE, errno);
		snprintf(error->text, MAX_RSSL_ERROR_TEXT,
			"<%s:%d> Error: 1004 rsslGetHostByName() failed. Interface name (%s) is incorrect. System errno: (%d)\n",
			__FILE__, __LINE__, pRsslSocketChannel->interfaceName, errno);

		sock_close(sock_fd);
		return(0);
	}


#ifdef IPC_DEBUG_SOCKET_NAME
	inaddr.s_addr = localAddr;
	tmp = (char*)inet_ntoa(inaddr);
	strncpy(localAddrStr, tmp, 129);
	printf("<%s:%d> ipcConnectSocket() rsslGetHostByName(interfaceName=%s) returns localAddr = %s\n", __FILE__, __LINE__, pRsslSocketChannel->interfaceName
		? pRsslSocketChannel->interfaceName : "NULL", localAddrStr);

	if (pRsslSocketChannel->interfaceName && (strcmp(pRsslSocketChannel->interfaceName, "127.0.0.1") == 0))
	{
		printf("<%s:%d> ipcConnectSocket(interfaceName==127.0.0.1) localAddr = %s => INADDR_LOOPBACK = 127.0.0.1\n", __FILE__, __LINE__, localAddrStr);
		localAddr = host2net_u32(INADDR_LOOPBACK);
	}
	else if (localAddr == host2net_u32(INADDR_LOOPBACK))
	{
		printf("<%s:%d> ipcConnectSocket(localAddr == INADDR_LOOPBACK) localAddr = %s => INADDR_ANY = 0.0.0.0\n", __FILE__, __LINE__, localAddrStr);
		localAddr = host2net_u32(INADDR_ANY);
	}
#else
	if (pRsslSocketChannel->interfaceName && (strcmp(pRsslSocketChannel->interfaceName, "127.0.0.1") == 0))
		localAddr = host2net_u32(INADDR_LOOPBACK);
	else if (localAddr == host2net_u32(INADDR_LOOPBACK))
		localAddr = host2net_u32(INADDR_ANY);
#endif

	if (ipcBindSocket(localAddr, localPort, sock_fd) < 0)
	{
		_rsslSetError(error, NULL, RSSL_RET_FAILURE, errno);
		snprintf(error->text, MAX_RSSL_ERROR_TEXT,
			"<%s:%d> Error: 1002 ipcBindSocket() failed. System errno: (%d)\n",
			__FILE__, __LINE__, errno);

		sock_close(sock_fd);
		return(0);
	}
#ifdef IPC_DEBUG_SOCKET_NAME
	_ipcdGetSockName(sock_fd);
#endif

	/* connect() needs the sockaddr struct in network byte order.
	* "addr" is already in network byte order.
	*/
	toaddr.sin_family = AF_INET;
	toaddr.sin_addr.s_addr = addr;
	toaddr.sin_port = (RsslUInt16)*portnum;

	if (connect(sock_fd, (struct sockaddr *)&toaddr, (int)sizeof(toaddr)) < 0)
	{
		if (blocking)
		{
			_rsslSetError(error, NULL, RSSL_RET_FAILURE, errno);
			snprintf(error->text, MAX_RSSL_ERROR_TEXT,
				"<%s:%d> Error: 1002 ipcConnectSocket() Blocking connect() failed. System errno: (%d)\n",
				__FILE__, __LINE__, errno);

			sock_close(sock_fd);
			return(0);
		}

#if defined(_WIN32)
		else if ((errno != WSAEINPROGRESS) && (errno != WSAEWOULDBLOCK))
#else
		else if ((errno != EINPROGRESS) && (errno != EALREADY))
#endif
		{
			_rsslSetError(error, NULL, RSSL_RET_FAILURE, errno);
			snprintf(error->text, MAX_RSSL_ERROR_TEXT,
				"<%s:%d> Error: 1002 ipcConnectSocket() connect() failed. System errno: (%d)\n",
				__FILE__, __LINE__, errno);

			sock_close(sock_fd);
			return(0);
		}
	}

	return sock_fd;
}

int ipcSetSockFuncs()
{
	ripcTransportFuncs  func;
	func.bindSrvr = ipcSrvrBind;
	func.newSrvrConnection = ipcNewSrvrConn;
	func.connectSocket = ipcConnectSocket;
	func.newClientConnection = ipcNewClientConn;
	func.initializeTransport = ipcInitTrans;
	func.shutdownTransport = ipcShutdownSckt;
	func.readTransport = ipcRead;
	func.writeTransport = ipcWrite;
	func.writeVTransport = ipcWriteV;
	func.reconnectClient = ipcScktReconnectClient;
	func.acceptSocket = ipcSrvrAccept;
	func.shutdownSrvrError = ipcSrvrShutdownError;
	func.sessIoctl = ipcSessIoctl;  // Not Support, only a stub
	func.getSockName = ipcGetSockName;
	func.setSockOpts = ipcSetSockOpts;
	func.getSockOpts = ipcGetSockOpts;
	func.connected = ipcIsConnected; 
	func.shutdownServer = ipcServerShutdown;
	func.uninitialize = ipcUninitialize;  // Not defined, only a stub

	return(ipcSetTransFunc(RSSL_CONN_TYPE_SOCKET, &func));
}

/* Returns : 1 if fd ready to write.
 *		   0 if fd not ready to write.
 *		   -1 if error
 */
int ipcReadyWrite(RsslSocket fd)
{
	int				retval;

#ifdef DEV_HAS_POLL
	struct pollfd   pollFds;

	pollFds.fd = fd;
	pollFds.events = POLLOUT;

	/* We don't have this in a loop since
	 * a non blocking poll should not return EINTR
	 */
	retval = poll(&pollFds,1,0);
	if (retval < 0)
	{
		if ((errno == EINTR) || (errno == EAGAIN))
			retval = 0;
		else
			retval = -1;
	}
#else
	fd_set	wrtFds;
	struct timeval	timeout;

	FD_ZERO(&wrtFds);
	FD_SET(fd,&wrtFds);
	timeout.tv_sec = 0;
	timeout.tv_usec = 0;

	/* We don't have this in a loop since
	 * a non blocking poll should not return EINTR
	 */
	retval = select( (int)(fd+1),DEV_SELECT_BITMAP_CAST 0,
					DEV_SELECT_BITMAP_CAST &wrtFds,
					DEV_SELECT_BITMAP_CAST 0, &timeout);
	if (retval < 0)
	{
		if ((errno == _IPC_WOULD_BLOCK) || (errno == EINTR))
			retval = 0;
		else
			retval = -1;
	}
#endif
	return(retval);
}


/* Returns : 1 if fd ready to read.
 *		   0 if fd not ready to read.
 *		   -1 if error
 */
int ipcReadyRead(RsslSocket fd)
{
	int				retval;

#ifdef DEV_HAS_POLL
	struct pollfd   pollFds;

	pollFds.fd = fd;
	pollFds.events = POLLIN;

	/* We don't have this in a loop since
	 * a non blocking poll should not return EINTR
	 */
	retval = poll(&pollFds,1,0);
	if (retval < 0)
	{
		if ((errno == EINTR) || (errno == EAGAIN))
			retval = 0;
		else
			retval = -1;
	}
#else
	fd_set			readFds;
	struct timeval	timeout;

	FD_ZERO(&readFds);
	FD_SET(fd,&readFds);
	timeout.tv_sec = 0;
	timeout.tv_usec = 0;

	/* We don't have this in a loop since
	 * a non blocking poll should not return EINTR
	 */
	retval = select((int)(fd+1),DEV_SELECT_BITMAP_CAST &readFds,
					DEV_SELECT_BITMAP_CAST 0,
					DEV_SELECT_BITMAP_CAST 0, &timeout);
	if (retval < 0)
	{
		if ((errno == _IPC_WOULD_BLOCK) || (errno == EINTR))
			retval = 0;
		else
			retval = -1;
	}
#endif
	return(retval);
}

/* Returns : 1 if fd ready for exception
 *		   0 if fd not ready for exception
 *		   -1 if error
 */
int ipcReadyException(RsslSocket fd)
{
	int				retval;

#ifdef DEV_HAS_POLL
	struct pollfd   pollFds;

	pollFds.fd = fd;
	pollFds.events = POLLPRI|POLLHUP;

	/* We don't have this in a loop since
	 * a non blocking poll should not return EINTR
	 */
	retval = poll(&pollFds,1,0);
	if (retval < 0)
	{
		if ((errno == EINTR) || (errno == EAGAIN))
			retval = 0;
		else
			retval = -1;
	}
#else
	fd_set			exceptFds;
	struct timeval	timeout;

	FD_ZERO(&exceptFds);
	FD_SET(fd,&exceptFds);
	timeout.tv_sec = 0;
	timeout.tv_usec = 0;

	/* We don't have this in a loop since
	 * a non blocking poll should not return EINTR
	 */
	retval = select((int)(fd+1),DEV_SELECT_BITMAP_CAST 0,
					DEV_SELECT_BITMAP_CAST 0,
					DEV_SELECT_BITMAP_CAST &exceptFds, &timeout);
	if (retval < 0)
	{
		if ((errno == _IPC_WOULD_BLOCK) || (errno == EINTR))
			retval = 0;
		else
			retval = -1;
	}
#endif
	return(retval);
}

#ifdef IPC_DEBUG_SOCKET_NAME
int _ipcdGetSockName( RsslSocket fd )
{
	char* tmp;
	char strIpAddr[129];
	struct sockaddr_in addr;
	int ipLen = sizeof(addr);
	int iRetVal = getsockname(fd, (struct sockaddr*)&addr, &ipLen);
	if (iRetVal == 0)
	{
		tmp = (char*)inet_ntoa(addr.sin_addr);
		strcpy(strIpAddr, tmp);
		printf("_ipcdGetSockName() addr=%s port=%d\n", strIpAddr, net2host_u16(addr.sin_port));
	}
	else
		printf("<%s:%d> _ipcdGetSockName() failed (%d)\n", __FILE__,__LINE__,errno);
	return(iRetVal);
}

int _ipcdGetPeerName( RsslSocket fd )
{
	char* tmp;
	char strIpAddr[129];
	struct sockaddr_in addr;
	int ipLen = sizeof(addr);
	int iRetVal = getpeername(fd, (struct sockaddr*)&addr, &ipLen);
	if (iRetVal == 0)
	{
		tmp = (char*)inet_ntoa(addr.sin_addr);
		strcpy(strIpAddr, tmp);
		printf("_ipcdGetPeerName() addr=%s port=%d\n", strIpAddr, net2host_u16(addr.sin_port));
	}
	else
		printf("<%s:%d> _ipcdGetPeerName() failed (%d)\n", __FILE__,__LINE__,errno);
	return(iRetVal);
}
#endif
