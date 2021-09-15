/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|          Copyright (C) 2019-2020 Refinitiv. All rights reserved.          --
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
#include "rtr/rsslCurlJIT.h"
#include "curl/curl.h"

#include "rtr/rsslErrors.h"

#ifdef Linux
#include <asm/ioctls.h>
#endif /* Linux */

static rtr_atomic_val rtr_SocketInits = 0;

#if defined(_WIN32)
static RsslUInt32			shutdownFlag = SD_SEND;
#else
static RsslUInt32			shutdownFlag = SHUT_WR;
#endif

curl_socket_t rsslCurlOpenSocketCallback(void *clientp,
	curlsocktype purpose,
	struct curl_sockaddr *address)
{
	return (curl_socket_t)socket(AF_INET, SOCK_STREAM, getProtocolNumber());
}

static int rsslCurlSetSockOptCallback(void *clientp, curl_socket_t curlfd,
	curlsocktype purpose)
{
	ripcSocketOption	sockopts;
	RsslSocket sock_fd = (RsslSocket)curlfd;
	RsslSocketChannel *pRsslSocketChannel = (RsslSocketChannel*)clientp;

	sockopts.code = RIPC_SOPT_LINGER;
	sockopts.options.linger_time = 0;
	if (ipcSockOpts(sock_fd, &sockopts) < 0)
	{
		return CURL_SOCKOPT_ERROR;
	}

	sockopts.code = RIPC_SOPT_KEEPALIVE;
	sockopts.options.turn_on = 1;
	if (ipcSockOpts(sock_fd, &sockopts) < 0)
	{
		return CURL_SOCKOPT_ERROR;
	}

	/* if non-zero, set this */
	if (pRsslSocketChannel->recvBufSize > 0)
	{
		sockopts.code = RIPC_SOPT_RD_BUF_SIZE;
		sockopts.options.buffer_size = pRsslSocketChannel->recvBufSize;

		if (ipcSockOpts(sock_fd, &sockopts) < 0)
		{
			return CURL_SOCKOPT_ERROR;
		}
	}

	/* if non-zero set this */
	if (pRsslSocketChannel->sendBufSize > 0)
	{
		sockopts.code = RIPC_SOPT_WRT_BUF_SIZE;
		sockopts.options.buffer_size = pRsslSocketChannel->sendBufSize;

		if (ipcSockOpts(sock_fd, &sockopts) < 0)
		{
			return CURL_SOCKOPT_ERROR;
		}
	}
	return CURL_SOCKOPT_OK;
}

RSSL_THREAD_DECLARE(runBlockingLibcurlProxyConnection, pArg) 
{
    RsslSocketChannel *rsslSocketChannel = ((RsslSocketChannel*)pArg);
	RsslError *error = &(rsslSocketChannel->curlThreadInfo.error);
	RsslInt32 tempLen = 0;
	RsslInt32 portnum;
	RsslUInt32 proxyPortNum; 
	RsslCurlJITFuncs* curlFuncs;
	char* curlOptProxy = NULL;
	char* curlOptUrl = NULL;
	char* curlOptProxyUserPwd = NULL;

	CURLcode curlret;
	if ((curlFuncs = rsslGetCurlFuncs()) == NULL)
	{
		_rsslSetError(error, NULL, RSSL_RET_FAILURE, errno);
		snprintf(error->text, MAX_RSSL_ERROR_TEXT,
			"<%s:%d> Error: 1002 Curl not initialized.\n",
			__FILE__, __LINE__);
		rsslSocketChannel->curlThreadInfo.curlThreadState = RSSL_CURL_ERROR;
		/* trigger select for error condition */
		rssl_pipe_write(&rsslSocketChannel->sessPipe, "1", 1);

        RSSL_THREAD_DETACH(&(rsslSocketChannel->curlThreadInfo.curlThreadId));
		return RSSL_THREAD_RETURN();
	}

	rsslSocketChannel->curlHandle = (*(curlFuncs->curl_easy_init))();
    if((rsslSocketChannel->curlThreadInfo.curlError = (char*)_rsslMalloc(CURL_ERROR_SIZE)) == NULL) 
    {
        _rsslSetError(error, NULL, RSSL_RET_FAILURE, errno);
        snprintf(error->text, MAX_RSSL_ERROR_TEXT,
            "<%s:%d> Error: 1001 Could not initialize memory for Curl Error.\n",
            __FILE__, __LINE__);
        rsslSocketChannel->curlThreadInfo.curlThreadState = RSSL_CURL_ERROR;
        /* trigger select for error condition */
        rssl_pipe_write(&rsslSocketChannel->sessPipe, "1", 1);
        
        RSSL_THREAD_DETACH(&(rsslSocketChannel->curlThreadInfo.curlThreadId));
		return RSSL_THREAD_RETURN();
    }

	tempLen = (RsslInt32)strlen(rsslSocketChannel->hostName) + (RsslInt32)strlen(rsslSocketChannel->serverName) + 2; // For : and null character at the end

	if ((curlOptUrl = (char*)_rsslMalloc(tempLen)) == NULL)
	{
		_rsslSetError(error, NULL, RSSL_RET_FAILURE, errno);
		snprintf(error->text, MAX_RSSL_ERROR_TEXT,
			"<%s:%d> Error: 1001 Could not initialize memory for Curl Error.\n",
			__FILE__, __LINE__);
		_rsslFree(rsslSocketChannel->curlThreadInfo.curlError);
		rsslSocketChannel->curlThreadInfo.curlThreadState = RSSL_CURL_ERROR;
		/* trigger select for error condition */
		rssl_pipe_write(&rsslSocketChannel->sessPipe, "1", 1);

        RSSL_THREAD_DETACH(&(rsslSocketChannel->curlThreadInfo.curlThreadId));
		return RSSL_THREAD_RETURN();
	}

	snprintf(curlOptUrl, (const size_t)tempLen, "%s:%s", rsslSocketChannel->hostName, rsslSocketChannel->serverName);

	// Init Curl Port
	portnum = net2host_u16(ipcGetServByName(rsslSocketChannel->serverName));

	// Init Curl Proxy Port
	proxyPortNum = net2host_u16(ipcGetServByName(rsslSocketChannel->proxyPort));

	(*(curlFuncs->curl_easy_setopt))(rsslSocketChannel->curlHandle, CURLOPT_HTTPPROXYTUNNEL, 1L);

	(*(curlFuncs->curl_easy_setopt))(rsslSocketChannel->curlHandle, CURLOPT_SSL_VERIFYHOST, 0L);
    // Init Curl Proxy Domain/User/Password
	if (rsslSocketChannel->curlOptProxyUser && rsslSocketChannel->curlOptProxyPasswd)
	{
		tempLen = (RsslInt32)(strlen(rsslSocketChannel->curlOptProxyUser)) + 2;		// username, :, and \0 character
		tempLen += (RsslInt32)(strlen(rsslSocketChannel->curlOptProxyPasswd));	// password

		if (rsslSocketChannel->curlOptProxyDomain)
		{
			tempLen += (RsslInt32)(strlen(rsslSocketChannel->curlOptProxyDomain)) + 2;	// domain and \\ characters
		}
		
        if ((curlOptProxyUserPwd = (char*)_rsslMalloc(tempLen)) == NULL) // Accounts for null character at the end of the string
		{
			_rsslSetError(error, NULL, RSSL_RET_FAILURE, errno);
			snprintf(error->text, MAX_RSSL_ERROR_TEXT,
				"<%s:%d> Error: 1001 Could not initialize memory for Curl URL Host.\n",
				__FILE__, __LINE__);
			rsslSocketChannel->curlThreadInfo.curlThreadState = RSSL_CURL_ERROR;
			_rsslFree(curlOptProxy);
			_rsslFree(curlOptUrl);
            _rsslFree(rsslSocketChannel->curlThreadInfo.curlError);
            rsslSocketChannel->curlThreadInfo.curlError = 0;
			/* trigger select for error condition */
			rssl_pipe_write(&rsslSocketChannel->sessPipe, "1", 1);

            RSSL_THREAD_DETACH(&(rsslSocketChannel->curlThreadInfo.curlThreadId));
			return RSSL_THREAD_RETURN();
		}

		if (rsslSocketChannel->curlOptProxyDomain)
        {
			snprintf(curlOptProxyUserPwd, (size_t)tempLen, "%s\\%s:%s", rsslSocketChannel->curlOptProxyDomain, rsslSocketChannel->curlOptProxyUser, rsslSocketChannel->curlOptProxyPasswd);
        }
		else
        {
			snprintf(curlOptProxyUserPwd, (size_t)tempLen, "%s:%s", rsslSocketChannel->curlOptProxyUser, rsslSocketChannel->curlOptProxyPasswd);
        }

		(*(curlFuncs->curl_easy_setopt))(rsslSocketChannel->curlHandle, CURLOPT_PROXYAUTH, CURLAUTH_ANY);
		(*(curlFuncs->curl_easy_setopt))(rsslSocketChannel->curlHandle, CURLOPT_PROXYUSERPWD, curlOptProxyUserPwd);
	}
	// Set Curl URL
	(*(curlFuncs->curl_easy_setopt))(rsslSocketChannel->curlHandle, CURLOPT_URL, curlOptUrl);
	// Set Curl Port
	(*(curlFuncs->curl_easy_setopt))(rsslSocketChannel->curlHandle, CURLOPT_PORT, (long)portnum);
	// Set Curl Proxy URL
	(*(curlFuncs->curl_easy_setopt))(rsslSocketChannel->curlHandle, CURLOPT_PROXY, rsslSocketChannel->proxyHostName);
	// Set Curl Proxy Port
	(*(curlFuncs->curl_easy_setopt))(rsslSocketChannel->curlHandle, CURLOPT_PROXYPORT, (long)proxyPortNum);

	// Set interface, if specified in connectopts
	if (rsslSocketChannel->interfaceName != NULL)
		(*(curlFuncs->curl_easy_setopt))(rsslSocketChannel->curlHandle, CURLOPT_INTERFACE, rsslSocketChannel->interfaceName);

    // Turn off curl Signal handling to avoid multithreaded crashes
	(*(curlFuncs->curl_easy_setopt))(rsslSocketChannel->curlHandle, CURLOPT_NOSIGNAL, 1L);
	(*(curlFuncs->curl_easy_setopt))(rsslSocketChannel->curlHandle, CURLOPT_SHARE, curlFuncs->curlShare);
	(*(curlFuncs->curl_easy_setopt))(rsslSocketChannel->curlHandle, CURLOPT_DEFAULT_PROTOCOL, "http");
	/* Configure this curl session to only tunnel us through the proxy and not attempt to send any other data to the upstream
	   server. */
	(*(curlFuncs->curl_easy_setopt))(rsslSocketChannel->curlHandle, CURLOPT_TCP_NODELAY, (long)rsslSocketChannel->tcp_nodelay);
	(*(curlFuncs->curl_easy_setopt))(rsslSocketChannel->curlHandle, CURLOPT_TCP_KEEPALIVE, 1L);
	(*(curlFuncs->curl_easy_setopt))(rsslSocketChannel->curlHandle, CURLOPT_ERRORBUFFER, rsslSocketChannel->curlThreadInfo.curlError);

	/* Set up socket and setsockopt callbacks */
	(*(curlFuncs->curl_easy_setopt))(rsslSocketChannel->curlHandle, CURLOPT_SOCKOPTFUNCTION, rsslCurlSetSockOptCallback);
	(*(curlFuncs->curl_easy_setopt))(rsslSocketChannel->curlHandle, CURLOPT_OPENSOCKETFUNCTION, rsslCurlOpenSocketCallback);
	(*(curlFuncs->curl_easy_setopt))(rsslSocketChannel->curlHandle, CURLOPT_SOCKOPTDATA, rsslSocketChannel);
	
	if(getCurlDebugMode())
		(*(curlFuncs->curl_easy_setopt))(rsslSocketChannel->curlHandle, CURLOPT_VERBOSE, 1L);

    (*(curlFuncs->curl_easy_setopt))(rsslSocketChannel->curlHandle, CURLOPT_CONNECT_ONLY, 1L);
	/* Initiate the connection through curl */
	if ((curlret = (*(curlFuncs->curl_easy_perform))(rsslSocketChannel->curlHandle)) != CURLE_OK)
	{
		_rsslSetError(error, NULL, RSSL_RET_FAILURE, errno);
		snprintf(error->text, MAX_RSSL_ERROR_TEXT,
			"<%s:%d> Error: 1002 Curl failed. Curl error number: %i Curl error text: %s\n",
			__FILE__, __LINE__, curlret, rsslSocketChannel->curlThreadInfo.curlError);
		rsslSocketChannel->curlThreadInfo.curlThreadState = RSSL_CURL_ERROR;
		_rsslFree(curlOptProxy);
		_rsslFree(curlOptUrl);
        _rsslFree(rsslSocketChannel->curlThreadInfo.curlError);
        rsslSocketChannel->curlThreadInfo.curlError = 0;
		if(curlOptProxyUserPwd)
			_rsslFree(curlOptProxyUserPwd);
		/* trigger select for error condition */
		rssl_pipe_write(&rsslSocketChannel->sessPipe, "1", 1);

        RSSL_THREAD_DETACH(&(rsslSocketChannel->curlThreadInfo.curlThreadId));
		return RSSL_THREAD_RETURN();
	}

    /* Free the allocated memory */
	_rsslFree(curlOptProxy);
	_rsslFree(curlOptUrl);
	if (curlOptProxyUserPwd)
		_rsslFree(curlOptProxyUserPwd);

	/* trigger select with 1 for successfully getting libcurl fd */
	rsslSocketChannel->curlThreadInfo.curlThreadState = RSSL_CURL_DONE;
	rssl_pipe_write(&rsslSocketChannel->sessPipe, "1", 1);

    RSSL_THREAD_DETACH(&(rsslSocketChannel->curlThreadInfo.curlThreadId));
	return RSSL_THREAD_RETURN();
}

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

int prependTransportHdr(void *transport, rtr_msgb_t *buf, int length, RsslError *error)
{
	return length;
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

#if defined(Linux)
		case RIPC_SOPT_REUSEPORT:
		{
#if defined(SO_REUSEPORT)
			int reuseFlag = (option->options.turn_on ? 1 : 0);
			if (setsockopt(fd, SOL_SOCKET, SO_REUSEPORT, (char *)&reuseFlag,
				(int)sizeof(reuseFlag)) < 0)
				ret = -1;
#else
			ret = -1;
#endif
			break;
		}
#endif

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
	// Do nothing as the server socket is closed and the ipcCloseActiveSrvr() method in the rsslSocketCloseServer() method.
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
	ripcSocketSession* sess = (ripcSocketSession*)malloc(sizeof(ripcSocketSession));

	if (sess == NULL)
	{
		_rsslSetError(error, NULL, RSSL_RET_FAILURE, errno);
		snprintf(error->text, MAX_RSSL_ERROR_TEXT,
			"<%s:%d> Error: 1001 Could not initialize memory for ripc socket session.\n",
			__FILE__, __LINE__);
		return NULL;
	}

	*initComplete = 1;

	sess->fd = fd;
	return((void*)sess);
}

void *ipcNewClientConn(RsslSocket fd, int *initComplete, void* userSpecPtr, RsslError* error )
{
	ripcSocketSession* sess = (ripcSocketSession*)malloc(sizeof(ripcSocketSession));

	if (sess == NULL)
	{
		_rsslSetError(error, NULL, RSSL_RET_FAILURE, errno);
		snprintf(error->text, MAX_RSSL_ERROR_TEXT,
			"<%s:%d> Error: 1001 Could not initialize memory for ripc socket session.\n",
			__FILE__, __LINE__);
		return NULL;
	}

	*initComplete = 1;

	sess->fd = fd;
	return((void*)sess);
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

	RsslSocket socket = ((ripcSocketSession*)transport)->fd;

	while((int)totalBytes < max_len)
	{
#ifdef _WIN32WSA
		wsabuf.buf = buf + totalBytes;
		wsabuf.len = max_len - totalBytes;

		retval = WSARecv(socket,&wsabuf,1,&numBytes,&dflags,NULL,NULL);

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
		numBytes = SOCK_RECV(socket, (buf + totalBytes), (size_t)(max_len - totalBytes), 0);

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

	RsslSocket socket = ((ripcSocketSession*)transport)->fd;

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
	retval = WSASend(socket, iov, iovcnt, &((DWORD)numBytes), 0, 0, 0);
	if (retval < 0)
		numBytes = retval;
#else
	numBytes = writev(socket,iov,iovcnt);
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
	RsslSocket socket = ((ripcSocketSession*)transport)->fd;

	while((int)totOut < outLen)
	{
#ifdef _WIN32WSA
		wsabuf.buf = buf + totOut;
		wsabuf.len = outLen - totOut;

		retval = WSASend(socket,&wsabuf,1,&numBytes,0,0,0);

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

		numBytes = SOCK_SEND(socket, (buf + totOut), (outLen - totOut), 0);

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

/* This is used to call socket shutdown instead of sock_close.  Shutdown will gracefully close the channel, allowing any in-flight messages
   to be read by the other side prior to closing the connection. */
int ipcShutdownSckt(void* transport)
{
	/* If the socket is invalid, it already has been closed by curl */
	if (((ripcSocketSession*)transport)->fd != RIPC_INVALID_SOCKET)
		shutdown(((ripcSocketSession*)transport)->fd, shutdownFlag);

	((ripcSocketSession*)transport)->fd = RIPC_INVALID_SOCKET;

	free(transport);
	return(1);
}


int ipcCloseSckt(void *transport)
{
	/* If the socket is invalid, it already has been closed by curl */
	if(((ripcSocketSession*)transport)->fd != RIPC_INVALID_SOCKET)
		sock_close(((ripcSocketSession*)transport)->fd);

	((ripcSocketSession*)transport)->fd = RIPC_INVALID_SOCKET;

	free(transport);
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
	// Windows. When server provides sharing for the socket then we use REUSEADDR instead of EXCLUSIVEADDRUSE.
	// (Microsoft docs). As long as SO_REUSEADDR socket option can be used to potentially hijack a port in a server application,
	// the application must be considered to be not secure.
	if (srvr->serverSharedSocket == RSSL_TRUE)
	{
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
	}
	else
	{
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

	if (srvr->serverSharedSocket)
	{
		sockopts.code = RIPC_SOPT_REUSEPORT;
		sockopts.options.turn_on = 1;
		if (ipcSockOpts(sock_fd, &sockopts) < 0)
		{
#if defined(SO_REUSEPORT)
			_rsslSetError(error, NULL, RSSL_RET_FAILURE, errno);
			snprintf(error->text, MAX_RSSL_ERROR_TEXT,
				"<%s:%d> Error: 1002 Could not to set SO_REUSEPORT on socket. System errno: (%d)\n",
				__FILE__, __LINE__, errno);
#else
			_rsslSetError(error, NULL, RSSL_RET_FAILURE, 0);
			snprintf(error->text, MAX_RSSL_ERROR_TEXT,
				"<%s:%d> Error: 1002 Could not to set SO_REUSEPORT on socket. Linux PC does not support SO_REUSEPORT. GCC version: %d.%d.%d\n",
				__FILE__, __LINE__, __GNUC__, __GNUC_MINOR__, __GNUC_PATCHLEVEL__);
#endif

			sock_close(sock_fd);
			return -1;
		}
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

	IPC_MUTEX_UNLOCK(rsslServerSocketChannel);

	fdtemp = accept(rsslServerSocketChannel->stream, (struct sockaddr *)0, (socklen_t *)0);

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
		return RIPC_INVALID_SOCKET;
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
		return RIPC_INVALID_SOCKET;
	}

	_DEBUG_TRACE_CONN("ripcNewUser: accept client fd "SOCKET_PRINT_TYPE"\n", fdtemp)

	if (ipcSessSetMode(fdtemp, rsslServerSocketChannel->session_blocking, rsslServerSocketChannel->tcp_nodelay, error, __LINE__) < 0)
	{
		sock_close(fdtemp);
		return RIPC_INVALID_SOCKET;
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
		return RIPC_INVALID_SOCKET;
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
			return RIPC_INVALID_SOCKET;
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
			return RIPC_INVALID_SOCKET;
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
	/* Proxy host name is resolved by CURL, and may include http or https protocols in the URI. */
	if (pRsslSocketChannel->proxyHostName == 0 || (pRsslSocketChannel->proxyHostName[0] == '\0'))
	{
		if (rsslGetHostByName(pRsslSocketChannel->hostName, &addr) < 0)
		{
			_rsslSetError(error, NULL, RSSL_RET_FAILURE, errno);
			snprintf(error->text, MAX_RSSL_ERROR_TEXT,
				"<%s:%d> Error: 1004 rsslGetHostByName() failed. Host name is incorrect. System errno: (%d)\n",
				__FILE__, __LINE__, errno);

			return(RIPC_INVALID_SOCKET);
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

			return(RIPC_INVALID_SOCKET);
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

			return(RIPC_INVALID_SOCKET);
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

		return(RIPC_INVALID_SOCKET);
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

	/* If we're configured for a proxy, create the pipe and  */
	if (pRsslSocketChannel->proxyHostName && pRsslSocketChannel->proxyPort)
	{
		// Create pipe for libcurl proxy connection
		if (!(rssl_pipe_create(&pRsslSocketChannel->sessPipe)))
		{
			/* some error here */
			_rsslSetError(error, NULL, RSSL_RET_FAILURE, errno);
			snprintf(error->text, MAX_RSSL_ERROR_TEXT,
				"<%s:%d> Error: 1002 Unable to create internal pipe on connection request.\n",
				__FILE__, __LINE__);

			return(RIPC_INVALID_SOCKET);
		}

		RSSL_THREAD_START(&(pRsslSocketChannel->curlThreadInfo.curlThreadId), runBlockingLibcurlProxyConnection, pRsslSocketChannel);
		return rssl_pipe_get_read_fd(&pRsslSocketChannel->sessPipe);
	}

	sock_fd = socket(AF_INET, SOCK_STREAM, getProtocolNumber());

	if (!ripcValidSocket(sock_fd))
	{
		_rsslSetError(error, NULL, RSSL_RET_FAILURE, errno);
		snprintf(error->text, MAX_RSSL_ERROR_TEXT,
			"<%s:%d> ipcConnectSocket() Error: 1002 socket() failed. System errno: (%d)\n",
			__FILE__, __LINE__, errno);

		return(RIPC_INVALID_SOCKET);
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
		return(RIPC_INVALID_SOCKET);
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
		return(RIPC_INVALID_SOCKET);
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
			return(RIPC_INVALID_SOCKET);
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
			return(RIPC_INVALID_SOCKET);
		}
	}

	if (ipcSessSetMode(sock_fd, blocking, tcp_nodelay, error, __LINE__) < 0)
	{
		sock_close(sock_fd);
		return(RIPC_INVALID_SOCKET);
	}

	if (ipcBindSocket(localAddr, localPort, sock_fd) < 0)
	{
		_rsslSetError(error, NULL, RSSL_RET_FAILURE, errno);
		snprintf(error->text, MAX_RSSL_ERROR_TEXT,
			"<%s:%d> Error: 1002 ipcBindSocket() failed. System errno: (%d)\n",
			__FILE__, __LINE__, errno);

		sock_close(sock_fd);
		return(RIPC_INVALID_SOCKET);
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
			return(RIPC_INVALID_SOCKET);
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
			return(RIPC_INVALID_SOCKET);
		}
	}

	return sock_fd;
}

int ipcSetProtFuncs()
{
	ripcProtocolFuncs  func;

	func.readTransportConnMsg = ipcReadTransportMsg;  // call readTransport
	func.readTransportMsg = ipcReadTransportMsg;  // call readTransport
	func.readPrependTransportHdr = ipcReadPrependTransportHdr; // Not defined, only a stub
	func.prependTransportHdr = ipcPrependTransportHdr;  // Not defined, only a stub
	func.additionalTransportHdrLength = ipcAdditionalHeaderLength; // Not defined, only a stub
	func.getPoolBuffer = ipcGetPoolBuffer;  // Get a buffer from the socketChannel inputbuffer pool
	func.getGlobalBuffer = ipcAllocGblMsg;  // Get a simple buffer from the global pool
	
	return (ipcSetProtocolHdrFuncs(RSSL_CONN_TYPE_SOCKET, &func));
}

int ipcSetSockFuncs()
{
	ripcTransportFuncs  func;
	func.bindSrvr = ipcSrvrBind;
	func.newSrvrConnection = ipcNewSrvrConn;
	func.connectSocket = ipcConnectSocket;
	func.newClientConnection = ipcNewClientConn;
	func.initializeTransport = ipcInitTrans;
	func.shutdownTransport = ipcCloseSckt;
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
		printf("_ipcdGetSockName() addr=%s port=%u\n", strIpAddr, net2host_u16(addr.sin_port));
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


