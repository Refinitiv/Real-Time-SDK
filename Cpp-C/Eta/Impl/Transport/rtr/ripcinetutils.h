/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2019 Refinitiv. All rights reserved.            --
 *|-----------------------------------------------------------------------------
 */

#ifdef _WIN32  

#ifndef __ripcinetutils_h
#define __ripcinetutils_h

#include "rtr/os.h"
#include "rtr/ripch.h"
#include "rtr/ripcutils.h"
#include "rtr/rsslSocketTransportImpl.h"
#include "rtr/rsslpipe.h"
#include "rtr/rsslErrors.h"
#include <stdio.h>
#include <process.h>
#include <wininet.h>


#define MAX_OUT_BYTES 0x7FFFFFFF

/* SSL Configuration structure */
typedef struct 
{
	char* hostName;
	char* proxyHostName;
	char* objectName;
	RsslInt32 portNum;
	RsslInt32 blocking;
	RsslUInt32 address;
	RsslUInt32 id;
	DWORD connectionFlags;
	RsslUInt16 pID; 
} ripcWinInetConnectOpts;

typedef enum
{
	WININET_ERROR	=                        -1,
	WININET_INITIALIZING =                    0,
	WININET_INIT_STREAM_SENDRQST =			  1,		/* Wait for streaming HttpSendRequest async response */
	WININET_INIT_STREAM_RESPONSE =			  2,		/* Wait for Reply from HttpSendRequest */
	WININET_INIT_STREAM_ACK_RECEIVED =		  3,		/* The Streaming channel ACK has been received */
	WININET_INIT_CONTROL_SENDRQST =			  4,		/* Wait for the control HttpSendRequest async response */
	WININET_INIT_CONTROL_SNDRQST_COMPL =	  5,		/* Ready to send on the control channel */
	WININET_ACTIVE						=	  6,
	WININET_INIT_CONTROL_SNDRQST_PARTIAL =    7,		/* Control channel didnt send full request synchronously.  Wait for the async response */
	WININET_INIT_CONTROL_SNDRQST_FULL_COMPL = 8		/* Control channel entire handshake is done.  Move to ACTIVE */
} ripcWinInetConnectionStates;


/* these are the reconnection states that we use in WinInet */
typedef enum
{
	WININET_INIT_OK = 0,
	WININET_STREAMING_CONN_REINITIALIZING	= 1,    /* wait for streaming HttpSendRequest async response, should trigger read and call IQDA */
	WININET_STREAMING_CONN_CALL_IQDA		= 2,    /* call IQDA from read function and wait for the ack */
	WININET_STREAMING_CONN_WAIT_ACK			= 3,    /* called IQDA, waiting for ack message */
	WININET_STREAMING_CONN_ACK_RECEIVED		= 4,    /* called IQDA and got ack - this should go directly to read */
	WININET_CONTROL_CONN_REINITIALIZING		= 6,	/* wait for control HttpSendRequestEx async response */
	WININET_CONTROL_CONN_SEND_POST			= 7,	/* HttpSendRequestEx is ready - send the message */
	WININET_CONTROL_CONN_WAIT_ACK			= 8,	/* Sent conn request, waiting for ack */
	WININET_CONTROL_CONN_ACK_RECEIVED		= 9,	/* Control connection received ack */
	WININET_WAIT_FINAL_CHUNK				= 10,	/* Waiting to get the final chunk */
	WININET_FINAL_CHUNK_RECEIVED			= 11	/* Final chunk was received - switch streaming FDs */
} ripcWinInetReconnectionStates;

/* our client structure */
typedef struct {
	HINTERNET openHandle;
	HINTERNET connectHandle;
	HINTERNET streamingReqHandle;
	HINTERNET newStreamingReqHandle;
	HINTERNET controlReqHandle;
	HINTERNET newControlReqHandle;
	RsslUInt32 bytesToRead;
	RsslUInt32 reconnectBytesToRead;
	DWORD	  totalBytesWritten;

	/* need persistent storage for the return values from wininet because we use asynchronous calls */
	/* we need separate read and write variables because reading and writing can be done at the same time */
	DWORD	  outBytesForRead;		/* because reads are asynchronous, we can't use a stack variable for this */
	DWORD	  outBytesForWrite;		/* because writes are asynchronous, we can't use a stack variable for this */

	char*	  outPutBuffer;
	RsslMutex tunnelMutex;
	rssl_pipe  _pipe; 
	RsslInt32	 byteWritten;
	char	  callbackPending;
	char	  writeCallbackPending;
	RsslInt32 writeBytesPending;
	ripcWinInetConnectionStates	 state;
	ripcWinInetReconnectionStates	reconnectState;
	char    reconnectWindow;
	volatile char	inCallback;
	LPCSTR* acceptTypes;
	INTERNET_BUFFERS iBuffer;
	ripcWinInetConnectOpts  config;  /* this holds the config for the clients (if this is server side, the config is copied from the servers */
	char	errorText[MAX_RSSL_ERROR_TEXT];  /* used to relay error text from the callback thread */
} ripcWinInetSession;

/* our transport read function -
 this will read from the network using WinInet and return the appropriate value to the ripc layer */
RsslRet ripcWinInetRead( void *winInetSess, char *buf, RsslInt32 max_len, ripcRWFlags flags, RsslError *error );

/* our transport write function -
   this will write to the network using WinInet and return the appropriate value to the ripc layer */
RsslRet ripcWinInetWrite(void *winInetSess, char *buf, RsslInt32 len, ripcRWFlags flags, RsslError *error);

/* shutdown the WinInet and the socket */
RsslRet ripcShutdownWinInetSocket(void *session);

/* creates and initializes new client/session structure */
ripcWinInetSession *ripcInitializeWinInetSession(RsslSocket fd, char* name, RsslError *error);

/* creates a new server side session, which is not supported by this transport */
void *ripcWinInetNewSrvr(void *srvr, RsslSocket fd, int *initComplete, void* userSpecPtr, RsslError *error);

/* connects session to a server */
void *ripcWinInetConnect(RsslSocket fd, RsslInt32 *initComplete, void* userSpecPtr, RsslError *error);

/* release the WinInet Session */
RsslRet ripcReleaseWinInetSession(void* session, RsslError *error);

RsslRet ripcWinInetInitConnection(void *session, ripcSessInProg *inPr, RsslError *error);

/* performs the reconnection handshaking to keep proxy sessions alive */
RsslRet ripcWinInetReconnection(void *session, RsslError *error);

RsslSocket ripcWinInetCreatePipe(RsslInt32 *portnum, RsslSocketChannel *opts, RsslInt32 flags, void** userSpecPtr, RsslError *error);

RsslRet ripcWinInetIoctl(void *session, RsslInt32 code, RsslInt32 value, RsslError *error);

/* handle errors */
RTR_C_INLINE void ripcWinInetErrors(RsslError *error, size_t initPos)
{
	DWORD dw = GetLastError();
	
	error->sysError = dw;

	if (dw != ERROR_SUCCESS)
	{
		FormatMessage(
			FORMAT_MESSAGE_FROM_SYSTEM |
			FORMAT_MESSAGE_IGNORE_INSERTS,
			NULL, 
			dw,  
			0,  /* language neutral */
			(LPTSTR)(error->text + (RsslInt32)(initPos)),
			(DWORD)(MAX_RSSL_ERROR_TEXT - initPos),
			NULL );
	}
}


/* creates a new server side session, which is not supported by this transport.
 * This function is only a stub which returns 0 */
RTR_C_INLINE void *ripcWinInetNewSrvr(void *srvr, RsslSocket fd, RsslInt32 *initComplete, void* userSpecPtr, RsslError *error)
{
	_rsslSetError(error, NULL, RSSL_RET_FAILURE, 0);
	snprintf(error->text, MAX_RSSL_ERROR_TEXT, "<%s:%d> ripcWinInetNewSrvr error: not supported", __FILE__, __LINE__);

	return 0;
}

/* initializes the Ripc portion of this - sets function pointers, etc */
RTR_C_INLINE RsslRet initRipcWinInet()
{
	RsslInt32 retVal = 0;
	
	ripcTransportFuncs winInetfuncs;

	winInetfuncs.bindSrvr = 0;
	winInetfuncs.newSrvrConnection = ripcWinInetNewSrvr;  /* Not Supported and only a stub */
	winInetfuncs.connectSocket = ripcWinInetCreatePipe; /* create the pipe and return the fd */
	winInetfuncs.newClientConnection = ripcWinInetConnect; /* calls SSL connect on client side */
	winInetfuncs.initializeTransport = ripcWinInetInitConnection; /* make sure this is where we do continuation of connection handshake */
	winInetfuncs.shutdownTransport = ripcShutdownWinInetSocket; /* shuts down socket on client or server side */
	winInetfuncs.readTransport = ripcWinInetRead; /* read function on client or server side */
	winInetfuncs.writeTransport = ripcWinInetWrite; /* write function on client or server side */
	winInetfuncs.writeVTransport = 0;   /* no writeV function yet (or maybe ever) */
	winInetfuncs.reconnectClient = ripcWinInetReconnection; /* reconnects client */
	winInetfuncs.sessIoctl = ripcWinInetIoctl;
	winInetfuncs.getSockName = 0;
	winInetfuncs.setSockOpts = 0;
	winInetfuncs.getSockOpts = 0;
	winInetfuncs.connected = 0;
	winInetfuncs.shutdownServer = 0;
	winInetfuncs.uninitialize = 0;

	retVal = ipcSetTransFunc(RSSL_CONN_TYPE_HTTP, &winInetfuncs);

	return retVal;
}

/* returns 0 for failure and 1 for success */
RTR_C_INLINE RsslRet ripcInitWinInetConnectOpts(ripcWinInetConnectOpts *config, RsslError *error)
{
	config->hostName = 0;
	config->objectName = 0;
	config->blocking = 0;
	config->portNum = 0;
	config->address = 0;
	config->pID = 0;
	config->id = 0;  /* instance ID assigned by server */
	config->connectionFlags = 0;
	config->proxyHostName = 0;
	
	return 1;
}

RTR_C_INLINE ripcWinInetSession *ripcWinInetNewSession(RsslSocket fd, char* name, RsslError *error)
{
	char* acceptType; 
	
	ripcWinInetSession *session = (ripcWinInetSession*)_rsslMalloc(sizeof(ripcWinInetSession));

	if (session == 0)
	{
		_rsslSetError(error, NULL, RSSL_RET_FAILURE, 0);
		snprintf(error->text, MAX_RSSL_ERROR_TEXT, "<%s:%d> Could not allocate space for ripcWinInetSession.", __FILE__, __LINE__);
		return 0;
	}
	
	/* do other initialization stuff here */

	if (ripcInitWinInetConnectOpts(&session->config, error) == 0)
	{
        _rsslFree(session);
		return 0;
	}

	session->connectHandle = 0;
	session->openHandle = 0;
	session->streamingReqHandle = 0;
	session->newStreamingReqHandle = 0;
	session->controlReqHandle = 0;
	session->newControlReqHandle = 0;

	/* initialize INTERNET_BUFFERS */
	session->iBuffer.dwStructSize = sizeof(INTERNET_BUFFERS);
	session->iBuffer.Next = NULL; 
    session->iBuffer.lpcszHeader = NULL;
    session->iBuffer.dwHeadersLength = 0;
    session->iBuffer.dwHeadersTotal = 0;
    session->iBuffer.lpvBuffer = NULL;                
    session->iBuffer.dwBufferLength = 0;
    /* this is just used in the Content-Length portion of the HTTP header. */
	/* It shouldnt matter since our server doesnt care and the firewalls/proxies will see encrypted data */
	session->iBuffer.dwBufferTotal = MAX_OUT_BYTES;  
    session->iBuffer.dwOffsetLow = 0;
    session->iBuffer.dwOffsetHigh = 0;

	rssl_pipe_init(&session->_pipe);
	session->byteWritten = 0;
	session->callbackPending = 0;
	session->writeCallbackPending = 0;
	session->bytesToRead = 0;
	session->totalBytesWritten = 0;
	session->writeBytesPending = 0;

	session->inCallback = 0;

	/* this is used during reconnection periods */
	session->state = WININET_INITIALIZING;
	session->reconnectState = WININET_INIT_OK;
	session->reconnectBytesToRead = 0;
    session->reconnectWindow = 0;

	session->errorText[0] = '\0';

	RSSL_MUTEX_INIT_ESDK( &session->tunnelMutex );
		
	/* create reusable output buffer for connection messages */
	session->outPutBuffer = (char*)_rsslMalloc(MAX_RSSL_ERROR_TEXT);

	/* set up the accept type */
	acceptType = (char*)_rsslMalloc(25);
	session->acceptTypes = (LPCSTR*)_rsslMalloc(2*sizeof(LPCSTR));
	memcpy(acceptType, "application/octet-stream", 24);
	acceptType[24] = '\0';
	session->acceptTypes[0] = acceptType;
	session->acceptTypes[1] = '\0';
	
	return session;
}

RTR_C_INLINE void ripcFreeWinInetConnectOpts(ripcWinInetConnectOpts *config)
{
	if (config->hostName)
	{
		_rsslFree(config->hostName);
		config->hostName = 0;
	}

	if (config->objectName)
	{
		_rsslFree(config->objectName);
		config->objectName = 0;
	}

	if (config->proxyHostName)
	{
		_rsslFree(config->proxyHostName);
		config->proxyHostName = 0;
	}
}

#endif /* WIN32 */

#endif  /* __ripcsslutils_h */

