/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|          Copyright (C) 2019-2020 Refinitiv. All rights reserved.          --
 *|-----------------------------------------------------------------------------
 */

#include "rtr/rsslSocketTransport.h"
#include "rtr/rsslSocketTransportImpl.h"
#include "rtr/rsslLoadInitTransport.h"
#include "rtr/rsslTransport.h"
#include "rtr/rsslAlloc.h"
#include "rtr/rsslErrors.h"
#include "rtr/ripcflip.h"
#include "rtr/ripcutils.h"
#include "rtr/rtratomic.h"
#include "rtr/rsslQueue.h"
#include "lz4.h"
 /* OpenSSL tunneling */
#include "rtr/ripcsslutils.h"

#if !defined(_WIN32)
#include <netdb.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <signal.h>
#include <unistd.h>
#include <arpa/inet.h>
#endif

#if defined(_WIN32)
#include <process.h>
#include <signal.h>
/* winInet tunneling */
#include "rtr/ripcinetutils.h"
#include <Ws2tcpip.h>
#include <tcpmib.h>
#include <Tcpestats.h>
#include <iphlpapi.h>
#endif


#include <setjmp.h>
#include <ctype.h>
#include <stdint.h>

#include "rtr/ripchttp.h"
#include "rtr/rsslCurlJIT.h"
#include "curl/curl.h"

#include "rtr/rwsutils.h"

#define FIRST_FRAG_HEADER_SIZE		10
#define SUBSEQ_FRAG_HEADER_SIZE		6

/* These only matter when compression expands the data (like with compression Level 0) */
#define FIRST_COMPRESSION_OVERHEAD	6		/* first compressed data in a stream has this */
#define SUBSEQ_COMPRESSION_OVERHEAD	6		/* subsequent compressed data may contain some overhead */

#define RSSL_MIN_FRAG_SIZE		25		/* This is the minimum fragmentation size that can convey users data */

/* These are not exact and there are probably exceptions to these */
/* for example, the first frag may not be the first message in a stream */
#define FIRST_FRAG_OVERHEAD			( FIRST_FRAG_HEADER_SIZE + FIRST_COMPRESSION_OVERHEAD )
#define SUBSEQ_FRAG_OVERHEAD		( SUBSEQ_FRAG_HEADER_SIZE + SUBSEQ_COMPRESSION_OVERHEAD )

#define IPC_ADD_CR_LF(des,it) \
	((char*)(des))[it++] = 13; \
	((char*)(des))[it++] = 10;

#define IPC_MAXIMUM_PINGTIMEOUT		0xFF
#define IPC_MINIMUM_PINGTIMEOUT		1

/* global debug function pointers */
static void(*rsslSocketDumpInFunc)(const char *functionName, char *buffer, RsslUInt32 length, RsslSocket socketId) = 0;
static void(*rsslSocketDumpOutFunc)(const char *functionName, char *buffer, RsslUInt32 length, RsslSocket socketId) = 0;

void(*ripcDumpInFunc)(const char *functionName, char *buf, RsslUInt32 len, RsslUInt64 opaque) = 0;
void(*ripcDumpOutFunc)(const char *functionName, char *buf, RsslUInt32 len, RsslUInt64 opaque) = 0;

extern void(*webSocketDumpInFunc)(const char *functionName, char *buf, RsslUInt32 len, RsslUInt64 opaque);
extern void(*webSocketDumpOutFunc)(const char *functionName, char *buf, RsslUInt32 len, RsslUInt64 opaque);

static ripcSessInit ipcReadHdr(RsslSocketChannel*, ripcSessInProg*, RsslError*);
static ripcSessInit ipcInitTransport(RsslSocketChannel*, ripcSessInProg*, RsslError*);
static ripcSessInit ipcInitClientTransport(RsslSocketChannel*, ripcSessInProg*, RsslError*);
static ripcSessInit ipcFinishSess(RsslSocketChannel*, ripcSessInProg*, RsslError*);
static ripcSessInit ipcWaitProxyAck(RsslSocketChannel*, ripcSessInProg*, RsslError*);
static ripcSessInit ipcWaitClientKey(RsslSocketChannel*, ripcSessInProg*, RsslError*);
static ripcSessInit ipcSendClientKey(RsslSocketChannel*, ripcSessInProg*, RsslError*);
static ripcSessInit ipcRejectSession(RsslSocketChannel*, RsslUInt16, RsslError*);
static ripcSessInit ipcProxyConnecting(RsslSocketChannel *rsslSocketChannel, ripcSessInProg *inPr, RsslError *error);
static ripcSessInit ipcClientAccept(RsslSocketChannel *rsslSocketChannel, ripcSessInProg *inPr, RsslError *error);
static ripcSessInit ipcReconnectSocket(RsslSocketChannel *rsslSocketChannel, ripcSessInProg *inPr, RsslError *error);
ripcSessInit ipcWaitProxyAck(RsslSocketChannel *rsslSocketChannel, ripcSessInProg *inPr, RsslError *error);
RsslRet ipcIntWrtHeader(RsslSocketChannel *rssl, RsslError *error);
ripcSessInit ipcSessionInit(RsslSocketChannel *rsslSocketChannel, ripcSessInProg *inPr, RsslError *error);
RsslRet ipcShutdownSockectChannel(RsslSocketChannel* rsslSocketChannel, RsslError *error);
RsslRet ipcSessDropRef(RsslSocketChannel *rsslSocketChannel, RsslError *error);
RsslRet rsslSocketGetSockOpts(RsslSocketChannel *rsslSocketChannel, RsslInt32 code, RsslInt32* value, RsslError *error);

/* Retrieves and sets the socket row from the Windows TCP table.  No-op for Linux */
static RsslRet ipcGetSocketRow(RsslSocketChannel *rsslSocketChannel, RsslError *error);

RsslRet ripcInitZlibComp();
RsslRet ripcInitLz4Comp();

// used to assign global sessionID's for each session. Will need to optimize to reuse session ID
static RsslUInt32					g_sessionID = 0;
static RsslInt32					protocolNumber;

static rtr_bufferpool_t *gblInputBufs = 0;

static RsslUInt8			ipc10Connack = IPC_CONNACK;
static RsslUInt8			ipc10Connnak = IPC_CONNNAK;

#if defined(_WIN32)
static RsslUInt32			shutdownFlag =  SD_SEND;
#else
static RsslUInt32			shutdownFlag =  SHUT_WR;
#endif

static RsslMutex		ripcMutex;
static RsslQueue			activeSocketChannelList;
static RsslQueue			freeSocketChannelList;
static RsslQueue			freeServerSocketChannelList;

/* Used to tell if openSSL has been loaded */
static RsslUInt8 openSSLInit = 0;
static u8 libsslNameLen = 0;
static u8 libcryptoNameLen = 0;

/* Curl lib info */
static RsslUInt8 libcurlInit = 0;
static u8 libcurlNameLen = 0;

/* This list of compression functions indexed by the
* type of compression.
*/
static ripcCompFuncs	compressFuncs[RSSL_COMP_MAX_TYPE + 1];

static const RsslUInt32	RSSL_COMP_DFLT_THRESHOLD_ZLIB = 30;
static const RsslUInt32	RSSL_COMP_DFLT_THRESHOLD_LZ4 = 300;

static RsslInitializeExOpts  transOpts = RSSL_INIT_INITIALIZE_EX_OPTS;

static ripcProtocolFuncs   protHdrFuncs[RIPC_MAX_TRANSPORTS];
static ripcTransportFuncs   transFuncs[RIPC_MAX_TRANSPORTS];

static ripcTransportFuncs 	encryptedSSLTransFuncs[RIPC_MAX_SSL_PROTOCOLS];

static ripcSSLFuncs		SSLTransFuncs;

static RsslUInt16		numInitCalls = 0;
static RsslUInt8 		initialized = 0;
static rtr_atomic_val		gblmutexinit = 0;

static u8 ripccompressions[][3]	=	{	{ 0, 0x00, RSSL_COMP_NONE  },	/* no compression	*/
										{ 0, 0x01, RSSL_COMP_ZLIB  },	/* zlib compression	*/
										{ 0, 0x02, RSSL_COMP_LZ4 } };	/* LZ4 compression	*/

/* winInet tunneling */
#include "rtr/ripcinetutils.h"
extern RsslRet getSSLProtocolTransFuncs(RsslSocketChannel* rsslSocketChannel, ripcSSLProtocolFlags protocolBitmap);

extern RIPC_SESS_VERS ripc10Ver;
extern RIPC_SESS_VERS ripc11Ver;
extern RIPC_SESS_VERS ripc11WinInetVer;
extern RIPC_SESS_VERS ripc12Ver;
extern RIPC_SESS_VERS ripc12WinInetVer;
extern RIPC_SESS_VERS ripc13Ver;
extern RIPC_SESS_VERS ripc13WinInetVer;
extern RIPC_SESS_VERS ripc14Ver;
extern RIPC_SESS_VERS ripc14WinInetVer;

/************* global variables ************/
//#define IPC_DEBUG

static RsslUInt8 conndebug = 0;
static RsslUInt8 readdebug = 0;
static RsslUInt8 refdebug = 0;

RsslUInt8 getConndebug()
{
	return conndebug;
}

/***************************
 * START INLINE HELPER FUNCTIONS
 ***************************/

RTR_C_ALWAYS_INLINE void _rsslSocketToChannel(rsslChannelImpl *chnl, RsslSocketChannel *sckt)
{
	chnl->Channel.socketId = (RsslSocket)sckt->stream;
	chnl->Channel.state	= (RsslInt32)sckt->state;

	//need this for tunneling reconnection ?
	chnl->Channel.oldSocketId = (RsslSocket)sckt->oldStream;

	chnl->Channel.pingTimeout = sckt->pingTimeout;

	chnl->Channel.protocolType = sckt->protocolType;
	chnl->Channel.majorVersion = sckt->majorVersion;
	chnl->Channel.minorVersion = sckt->minorVersion;

	chnl->Channel.connectionType = sckt->connType;

	// valid for rsslAccept
	chnl->Channel.clientHostname = sckt->clientHostname;
	chnl->Channel.clientIP = sckt->clientIP;

	// valid for rsslConnect
	chnl->Channel.hostname = sckt->hostName;
	chnl->Channel.port = sckt->port;

	return;
}

RTR_C_ALWAYS_INLINE void _rsslSocketChannelToIpcSocket(RIPC_SOCKET* ipcSckt, RsslSocketChannel *scktChannel)
{
	ipcSckt->componentVer = scktChannel->componentVer;
	ipcSckt->componentVerLen = scktChannel->componentVerLen;
	ipcSckt->connectionType = scktChannel->connType;
	ipcSckt->hostname = scktChannel->clientHostname;
	ipcSckt->IPaddress = scktChannel->clientIP;
	ipcSckt->majorVersion = scktChannel->majorVersion;
	ipcSckt->minorVersion = scktChannel->minorVersion;
	ipcSckt->mutex = scktChannel->mutex;
	ipcSckt->oldStream = (RsslSocket)scktChannel->oldStream;
	ipcSckt->outComponentVer = scktChannel->outComponentVer;
	ipcSckt->outComponentVerLen = scktChannel->outComponentVerLen;
	ipcSckt->pingTimeout = scktChannel->pingTimeout;
	ipcSckt->protocolType = scktChannel->protocolType;
	ipcSckt->rsslFlags = scktChannel->rsslFlags;
	ipcSckt->shared_key = scktChannel->shared_key;
	ipcSckt->state = scktChannel->state;
	ipcSckt->stream = (RsslSocket)scktChannel->stream;
}

RsslRet ipcNullPtr(char *funcName, char *ptrname, char *file, RsslInt32 line, RsslError *err)
{
	err->rsslErrorId = RSSL_RET_FAILURE;
	snprintf(err->text, MAX_RSSL_ERROR_TEXT, "<%s:%d> %s() Error: 1001 %s is NULL\n", file, line, funcName, ptrname);
	return(1);
}

void _rsslBufferMap(rsslBufferImpl *buffer, rtr_msgb_t *ripcBuffer)
{
	buffer->buffer.data = ripcBuffer->buffer;
	buffer->buffer.length = (RsslUInt32)ripcBuffer->length;

	buffer->bufferInfo = ripcBuffer;

	/* set this to a well known value */
	/* and check when we use the buffer -
	   if its different that means the buffer was overwritten */
	buffer->integrity = 69;

	return;
}

/* To keep the global Input Buffer in scope to rsslSocketTransport, provide an interface
 * for some other transports which use them */
rtr_msgb_t * ipcAllocGblMsg(size_t length)
{
	return rtr_smplcAllocMsg(gblInputBufs, length);
}

rtr_msgb_t * ipcDupGblMsg(rtr_msgb_t * buffer)
{
	return  rtr_smplcDupMsg(gblInputBufs, buffer);
}

RsslRet ipcSetCompFunc(RsslInt32 compressionType, ripcCompFuncs *funcs)
{
	_DEBUG_TRACE_INIT("HERE comp type %d\n", compressionType)
	if (compressionType > RSSL_COMP_MAX_TYPE)
		return(-1);
	compressFuncs[compressionType] = *funcs;

	return(1);
}

ripcCompFuncs *ipcGetCompFunc(RsslInt32 compressionType)
{
	ripcCompFuncs *funcs = 0;

	if (compressionType <= RSSL_COMP_MAX_TYPE)
		funcs = &(compressFuncs[compressionType]);

	return(funcs);
}

RsslRet ipcSetSocketChannelProtocolHdrFuncs(RsslSocketChannel * rsslSocketChannel, RsslInt32 type)
{

	_DEBUG_TRACE_INIT("HERE type %d\n", type)
	if (type >= RIPC_MAX_TRANSPORTS)
		return(-1);

	rsslSocketChannel->protocolFuncs = &(protHdrFuncs[type]);

	return (1);
}

RsslRet ipcSetProtocolHdrFuncs(RsslInt32 type, ripcProtocolFuncs *funcs)
{

	_DEBUG_TRACE_INIT("HERE type %d\n", type)
	if (type >= RIPC_MAX_TRANSPORTS)
		return(-1);

	protHdrFuncs[type] = *funcs;

	return (1);
}

RsslRet ipcSetTransFunc(RsslInt32 type, ripcTransportFuncs *funcs)
{
	if (type >= RIPC_MAX_TRANSPORTS)
		return(-1);

	transFuncs[type].bindSrvr = funcs->bindSrvr;

	transFuncs[type].newSrvrConnection = funcs->newSrvrConnection;

	transFuncs[type].connectSocket = funcs->connectSocket;

	transFuncs[type].newClientConnection = funcs->newClientConnection;

	transFuncs[type].initializeTransport = funcs->initializeTransport;

	transFuncs[type].shutdownTransport = funcs->shutdownTransport;

	transFuncs[type].readTransport = funcs->readTransport;

	transFuncs[type].writeTransport = funcs->writeTransport;

	transFuncs[type].writeVTransport = funcs->writeVTransport;

	transFuncs[type].reconnectClient = funcs->reconnectClient;

	transFuncs[type].acceptSocket = funcs->acceptSocket;

	/* If the calling function hasn't been assigned
	 * its own, then the default ipc definition will be used
	 * */

	transFuncs[type].sessIoctl = (funcs->sessIoctl ? funcs->sessIoctl : transFuncs[RSSL_CONN_TYPE_SOCKET].sessIoctl);

	transFuncs[type].getSockName = (funcs->getSockName ? funcs->getSockName : transFuncs[RSSL_CONN_TYPE_SOCKET].getSockName);

	transFuncs[type].setSockOpts = (funcs->setSockOpts ? funcs->setSockOpts : transFuncs[RSSL_CONN_TYPE_SOCKET].setSockOpts);

	transFuncs[type].getSockOpts = (funcs->getSockOpts ? funcs->getSockOpts : transFuncs[RSSL_CONN_TYPE_SOCKET].getSockOpts);

	transFuncs[type].connected = (funcs->connected ? funcs->connected : transFuncs[RSSL_CONN_TYPE_SOCKET].connected);

	transFuncs[type].shutdownServer = (funcs->shutdownServer ? funcs->shutdownServer : transFuncs[RSSL_CONN_TYPE_SOCKET].shutdownServer);

	transFuncs[type].shutdownSrvrError = (funcs->shutdownSrvrError ? funcs->shutdownSrvrError : transFuncs[RSSL_CONN_TYPE_SOCKET].shutdownSrvrError);

	transFuncs[type].uninitialize = (funcs->uninitialize ? funcs->uninitialize : transFuncs[RSSL_CONN_TYPE_SOCKET].uninitialize);

	return(1);
}

RsslRet ipcSetSSLTransFunc(RsslInt32 type, ripcTransportFuncs *funcs)
{
	if (type >= RIPC_MAX_SSL_PROTOCOLS)
		return(-1);

	/* Each calling transport should have a definition for every
	 * function pointer. 	 * */
	encryptedSSLTransFuncs[type].bindSrvr = funcs->bindSrvr;

	encryptedSSLTransFuncs[type].newSrvrConnection = funcs->newSrvrConnection;

	encryptedSSLTransFuncs[type].connectSocket = funcs->connectSocket;

	encryptedSSLTransFuncs[type].newClientConnection = funcs->newClientConnection;

	encryptedSSLTransFuncs[type].initializeTransport = funcs->initializeTransport;

	encryptedSSLTransFuncs[type].shutdownTransport = funcs->shutdownTransport;

	encryptedSSLTransFuncs[type].readTransport = funcs->readTransport;

	encryptedSSLTransFuncs[type].writeTransport = funcs->writeTransport;

	encryptedSSLTransFuncs[type].writeVTransport = funcs->writeVTransport;

	encryptedSSLTransFuncs[type].reconnectClient = funcs->reconnectClient;

	encryptedSSLTransFuncs[type].acceptSocket = funcs->acceptSocket;

	/* If the calling function hasn't been assigned
	 * its own, then the default ipc definition will be used
	 * */
	encryptedSSLTransFuncs[type].sessIoctl = (funcs->sessIoctl ? funcs->sessIoctl : transFuncs[RSSL_CONN_TYPE_SOCKET].sessIoctl);

	encryptedSSLTransFuncs[type].getSockName = (funcs->getSockName ? funcs->getSockName : transFuncs[RSSL_CONN_TYPE_SOCKET].getSockName);

	encryptedSSLTransFuncs[type].setSockOpts = (funcs->setSockOpts ? funcs->setSockOpts : transFuncs[RSSL_CONN_TYPE_SOCKET].setSockOpts);

	encryptedSSLTransFuncs[type].getSockOpts = (funcs->getSockOpts ? funcs->getSockOpts : transFuncs[RSSL_CONN_TYPE_SOCKET].getSockOpts);

	encryptedSSLTransFuncs[type].connected = (funcs->connected ? funcs->connected : transFuncs[RSSL_CONN_TYPE_SOCKET].connected);

	encryptedSSLTransFuncs[type].shutdownServer = (funcs->shutdownServer ? funcs->shutdownServer : transFuncs[RSSL_CONN_TYPE_SOCKET].shutdownServer);

	encryptedSSLTransFuncs[type].shutdownSrvrError = (funcs->shutdownSrvrError ? funcs->shutdownSrvrError : transFuncs[RSSL_CONN_TYPE_SOCKET].shutdownSrvrError);

	encryptedSSLTransFuncs[type].uninitialize = (funcs->uninitialize ? funcs->uninitialize : transFuncs[RSSL_CONN_TYPE_SOCKET].uninitialize);

	return(1);
}

RsslRet getSSLProtocolTransFuncs(RsslSocketChannel* rsslSocketChannel, ripcSSLProtocolFlags protocolBitmap)
{
	if (protocolBitmap == RIPC_PROTO_SSL_NONE)
		return RSSL_RET_FAILURE;
	if ((protocolBitmap & RIPC_PROTO_SSL_TLS) != 0)
	{
        rsslSocketChannel->sslCurrentProtocol = RIPC_PROTO_SSL_TLS;
		rsslSocketChannel->transportFuncs = &encryptedSSLTransFuncs[RIPC_SSL_TLS];
		return RSSL_RET_SUCCESS;
	}
	if ((protocolBitmap & RIPC_PROTO_SSL_TLS_V1_2) != 0)
	{
        rsslSocketChannel->sslCurrentProtocol = RIPC_PROTO_SSL_TLS_V1_2;
		rsslSocketChannel->transportFuncs = &encryptedSSLTransFuncs[RIPC_SSL_TLS_V1_2];
		return RSSL_RET_SUCCESS;
	}
	if ((protocolBitmap & RIPC_PROTO_SSL_TLS_V1_1) != 0)
	{
		rsslSocketChannel->sslCurrentProtocol = RIPC_PROTO_SSL_TLS_V1_1;
		rsslSocketChannel->transportFuncs = &encryptedSSLTransFuncs[RIPC_SSL_TLS_V1_1];
		return RSSL_RET_SUCCESS;
	}
	if ((protocolBitmap & RIPC_PROTO_SSL_TLS_V1) != 0)
	{
		rsslSocketChannel->sslCurrentProtocol = RIPC_PROTO_SSL_TLS_V1;
		rsslSocketChannel->transportFuncs = &encryptedSSLTransFuncs[RIPC_SSL_TLS_V1];
		return RSSL_RET_SUCCESS;
	}

	return RSSL_RET_FAILURE;
}

RsslRet ipcSetSSLFuncs(ripcSSLFuncs *funcs)
{
	if ((SSLTransFuncs.newSSLServer != 0) ||
		(SSLTransFuncs.freeSSLServer != 0))
		return RSSL_RET_FAILURE;

	SSLTransFuncs = *funcs;

	return(1);
}

RsslRet ipcLoadOpenSSL(RsslError *error)
{
	if (openSSLInit == 0)
	{
		/* Initialize open ssl */
		if (ripcInitializeSSL(transOpts.jitOpts.libsslName, transOpts.jitOpts.libcryptoName) == 1)
			openSSLInit = 1;
		else
		{
			_rsslSetError(error, NULL, RSSL_RET_FAILURE, errno);
			snprintf(error->text, MAX_RSSL_ERROR_TEXT,
				"<%s:%d> Error: 0012 Unable to load openSSL Libraries.\n",
				__FILE__, __LINE__);

			return RSSL_RET_FAILURE;
		}
	}
	return RSSL_RET_SUCCESS;
}

RsslSocketChannel* createRsslSocketChannel()
{
	RsslSocketChannel* rsslSocketChannel = (RsslSocketChannel*)_rsslMalloc(sizeof(RsslSocketChannel));

	if (rsslSocketChannel)
	{
		rsslInitQueueLink(&(rsslSocketChannel->link1));
		ripcClearRsslSocketChannel(rsslSocketChannel);

		/* set sessionID here - this is going to persist for the life of the session on the server side */
		rsslSocketChannel->sessionID = ++g_sessionID;
	}

	return rsslSocketChannel;
}

RsslServerSocketChannel* createRsslServerSocketChannel()
{
    RsslServerSocketChannel* rsslServerSocketChannel = (RsslServerSocketChannel*)_rsslMalloc(sizeof(RsslServerSocketChannel));

    if (rsslServerSocketChannel)
    {
		rsslInitQueueLink(&(rsslServerSocketChannel->link1));
		rsslClearRsslServerSocketChannel(rsslServerSocketChannel);
    }

    return rsslServerSocketChannel;
}

RsslSocketChannel* newRsslSocketChannel()
{
	RsslSocketChannel* rsslSocketChannel = 0;
	RsslQueueLink* pLink = 0;

	if (multiThread)
	{
		(void) RSSL_MUTEX_LOCK(&ripcMutex);
		_DEBUG_MUTEX_TRACE("RSSL_MUTEX_LOCK", NULL, &ripcMutex)
	}

	if ((pLink = rsslQueuePeekFront(&freeSocketChannelList)) == 0)
		rsslSocketChannel = createRsslSocketChannel();
	else
	{
		rsslSocketChannel = RSSL_QUEUE_LINK_TO_OBJECT(RsslSocketChannel, link1, pLink);
		rsslQueueRemoveLink(&freeSocketChannelList, &(rsslSocketChannel->link1));
	}

	if (multiThread)
	{
		(void) RSSL_MUTEX_UNLOCK(&ripcMutex);
		_DEBUG_MUTEX_TRACE("RSSL_MUTEX_UNLOCK", NULL, &ripcMutex)
	}

    _DEBUG_TRACE_REF("RsslSocketChannel=0x%p *rSC 0x%p\n", rsslSocketChannel, *rsslSocketChannel)

	return rsslSocketChannel;
}

RsslServerSocketChannel* newRsslServerSocketChannel()
{
	RsslServerSocketChannel* rsslServerSocketChannel = 0;
	RsslQueueLink* pLink = 0;

	if (multiThread)
	{
		(void) RSSL_MUTEX_LOCK(&ripcMutex);
		_DEBUG_MUTEX_TRACE("RSSL_MUTEX_LOCK", NULL, &ripcMutex)
    }

	if ((pLink = rsslQueuePeekFront(&freeServerSocketChannelList)) == 0)
		rsslServerSocketChannel = createRsslServerSocketChannel();
	else
	{
		rsslServerSocketChannel = RSSL_QUEUE_LINK_TO_OBJECT(RsslServerSocketChannel, link1, pLink);
		rsslQueueRemoveLink(&freeServerSocketChannelList, &(rsslServerSocketChannel->link1));
	}

    if (multiThread)
    {
        (void) RSSL_MUTEX_UNLOCK(&ripcMutex);
		_DEBUG_MUTEX_TRACE("RSSL_MUTEX_UNLOCK", NULL, &ripcMutex)
    }

    _DEBUG_TRACE_REF("RsslServerSocketChannel=0x%p\n",rsslServerSocketChannel)

    return rsslServerSocketChannel;
}

/*********************************************
*	The following defines the functions needed
*	for reading ipc messages
*      The parameter moreData returns the number of bytes
*      remaining in the input buffer waiting to be read.
*      It is of type int*.( WARNING )
*		If compression is on the moreData will be a bool
*		that identifies there being more data and not
*		the number of bytes remaining as there is no way to tell
*		from the ipc headers with compression enabled.
*********************************************/

rtr_msgb_t *ipcReadSession( RsslSocketChannel *rsslSocketChannel, RsslRet *readret, RsslInt32 *moreData, RsslInt32 *fragLength, RsslInt32 *fragId,
	RsslInt32* bytesRead, RsslInt32* uncompBytesRead, RsslInt32 *packing, RsslError *error)
{
	RsslInt32  cc = 0;
	RsslUInt8  canRead = 1;
	RsslUInt32 ipcLen = 0;
	RsslUInt16 ipcOpcode = 0;
	RsslUInt16 ipcFlags = 0;
	RsslUInt32 tempLen;
	RsslUInt32 IPC_header_size = 0;
	RsslUInt16 messageLength;
	RsslUInt8  cOpcode = 0;
	RsslUInt8  cFlags = 0;
	RsslUInt8  cHdrLen = 0;
	RsslInt32  extendedHdr = 0;
	RsslInt32  cont = 1;
	RsslInt32  inBytes = 0;
	ripcRWFlags	rwflags = RIPC_RW_NONE;
	RsslInt32 httpHeaderLen = 0;
	void* tmpTransportInfo = 0;
	size_t inputBufferLength = 0;

	if (IPC_NULL_PTR(rsslSocketChannel, "ipcReadSession", "rsslSocketChannel", error))
	{
		*readret = RSSL_RET_FAILURE;
		return(0);
	}

	rwflags |= (rsslSocketChannel->blocking ? RIPC_RW_BLOCKING : 0);

	*moreData = 0;

	_DEBUG_TRACE_READ("inputBufCursor:%u inputBuffer->length:%llu inBufProtOffset %u\n",
					rsslSocketChannel->inputBufCursor, rsslSocketChannel->inputBuffer->length, rsslSocketChannel->inBufProtOffset)
	/* If we are at the beginning of the input buffer,
	* then attempt to read readSize of data in one big chunk.
	*/

	/* we need to set this so we can subtract it out and not adjust for it on read
	* - we only need this when writing
	*/
	if (rsslSocketChannel->httpHeaders)
		httpHeaderLen = 6;

	inputBufferLength = rsslSocketChannel->inputBuffer->length; /* Keeps the initial input buffer length */

	if (rsslSocketChannel->inputBuffer->length == 0)
	{
		cc = (*(rsslSocketChannel->protocolFuncs->readTransportMsg))((void*)rsslSocketChannel, rsslSocketChannel->inputBuffer->buffer, rsslSocketChannel->readSize, rwflags, error);

		if (rsslSocketChannel->workState & RIPC_INT_SHTDOWN_PEND)
		{
			_rsslSetError(error, NULL, RSSL_RET_FAILURE, errno);
			snprintf(error->text, MAX_RSSL_ERROR_TEXT,
				"<%s:%d> Error: 1003 ipcReadSession() failed due to channel shutting down.\n",
				__FILE__, __LINE__);

			*readret = RSSL_RET_FAILURE;

			return 0;
		}

		_DEBUG_TRACE_READ(" fd "SOCKET_PRINT_TYPE" cc %d err %d\n",
			rsslSocketChannel->stream, cc, ((cc <= 0) ? errno : 0))

		if (cc < 0)
		{
			/* this would be an FD change event */
			if ((rsslSocketChannel->newStream != RIPC_INVALID_SOCKET) && (rsslSocketChannel->tunnelingState == RIPC_TUNNEL_ACTIVE))
			{
				/* change around file descriptors */
				/* Keep the old Stream ID due to WinInet bug not properly finishing the TCP close handshake*/
				rsslSocketChannel->oldStream = rsslSocketChannel->stream;
				tmpTransportInfo = rsslSocketChannel->transportInfo;
				rsslSocketChannel->stream = rsslSocketChannel->newStream;
				rsslSocketChannel->transportInfo = rsslSocketChannel->newTransportInfo;
				(*(rsslSocketChannel->transportFuncs->shutdownTransport))(tmpTransportInfo);
				// Null out oldTransportInfo, because it has been free'd 
				rsslSocketChannel->newStream = RIPC_INVALID_SOCKET;
				rsslSocketChannel->newTransportInfo = NULL;
				*readret = RSSL_RET_READ_FD_CHANGE;

				return 0;
			}

			/* Note: ipcRead() returns -1 if error, -2 if connection is down. */
			if (cc == -2)
			{
				_rsslSetError(error, NULL, RSSL_RET_FAILURE, errno);
				snprintf(error->text, MAX_RSSL_ERROR_TEXT,
					"<%s:%d> Error:1002 ipcRead() failure. Connection reset by peer\n",
					__FILE__, __LINE__);

				*readret = RSSL_RET_FAILURE;

				return 0;
			}
			else if (cc == RSSL_RET_READ_WOULD_BLOCK)
			{
				*readret = RSSL_RET_READ_WOULD_BLOCK;
				inBytes = (RsslInt32)(rsslSocketChannel->inputBuffer->length - inputBufferLength); /* Set the partial bytes read if any */

				return 0;
			}
			else
			{
				_rsslSetError(error, NULL, RSSL_RET_FAILURE, errno);
				snprintf((error->text), MAX_RSSL_ERROR_TEXT,
					"<%s:%d> Error:1002 ipcRead() failure. System errno: (%d)\n",
					__FILE__, __LINE__, errno);

				*readret = RSSL_RET_FAILURE;

				return 0;
			}
		}

		rsslSocketChannel->inputBuffer->length += cc;
		inBytes = cc;
		if (rsslSocketChannel->blocking == 0)
			canRead = 0;

		/* Handle partial read for websocket frame for the websocket connection. */
		if (rsslSocketChannel->rwsSession)
		{
			rwsSession_t		*wsSess = (rwsSession_t*)rsslSocketChannel->rwsSession;
			rwsFrameHdr_t		*frame = &(wsSess->frameHdr);
			
			if (frame->partial)
			{
				*readret = RSSL_RET_READ_WOULD_BLOCK;
				return 0;
			}
			else if ( (frame->opcode == RWS_OPC_PING) || (frame->opcode == RWS_OPC_PONG) )
			{
				wsSess->inputReadCursor += frame->hdrLen + frame->payloadLen;

				if (wsSess->inputReadCursor == wsSess->actualInBuffLen)
				{
					wsSess->inputReadCursor = 0;
					wsSess->actualInBuffLen = 0;
				}

				rsslSocketChannel->inputBufCursor += (RsslUInt32)frame->payloadLen;

				if (rsslSocketChannel->inputBufCursor == rsslSocketChannel->inputBuffer->length)
				{
					rsslSocketChannel->inputBufCursor = 0;
					rsslSocketChannel->inputBuffer->length = 0;
					*readret = RSSL_RET_READ_WOULD_BLOCK;
					return 0;
				}
				else
				{
					*readret = RSSL_RET_SUCCESS;
					return 0;
				}
			}
		}
	}
	else
	{
		/* Parse or read additional transport header if any. */
		cc = (*(rsslSocketChannel->protocolFuncs->readPrependTransportHdr))((void*)rsslSocketChannel, rsslSocketChannel->inputBuffer->buffer, 
			rsslSocketChannel->readSize, rwflags, readret, error);

		if (*readret != RSSL_RET_SUCCESS)
		{
			inBytes = (RsslInt32)(rsslSocketChannel->inputBuffer->length - inputBufferLength); /* Set the partial bytes read if any */

			/* Handles RWS_OPC_PING or  RWS_OPC_PING frames to ignore and read more data if any. */
			if (*readret == RSSL_RET_READ_PING)
				*readret = RSSL_RET_SUCCESS;

			return 0;
		}

		inBytes = cc;
	}

	IPC_header_size = (rsslSocketChannel->version->dataHeaderLen - httpHeaderLen);

	rwflags |= RIPC_RW_WAITALL;

	/* The ipcLen can be variable depending on the protocol */
	while (cont)
	{
		tempLen = (RsslUInt32)(rsslSocketChannel->inputBuffer->length - rsslSocketChannel->inputBufCursor);
		while (tempLen < (IPC_header_size + extendedHdr))
		{
			/* A full header does not exist.
			* So attempt to read the rest of the header if we haven't already tried to read.
			*/
			if (canRead)
			{
				IPC_MUTEX_UNLOCK(rsslSocketChannel);

				cc = (*(rsslSocketChannel->transportFuncs->readTransport))(
					rsslSocketChannel->transportInfo,
					(rsslSocketChannel->inputBuffer->buffer + rsslSocketChannel->inputBuffer->length),
					((IPC_header_size + extendedHdr) - tempLen), rwflags, error);

				IPC_MUTEX_LOCK(rsslSocketChannel);

				if (rsslSocketChannel->workState & RIPC_INT_SHTDOWN_PEND)
				{
					_rsslSetError(error, NULL, RSSL_RET_FAILURE, errno);
					snprintf(error->text, MAX_RSSL_ERROR_TEXT,
						"<%s:%d> Error: 1003 ipcReadSession() failed due to channel shutting down.\n",
						__FILE__, __LINE__);

					*readret = RSSL_RET_FAILURE;

					return 0;
				}

				if (cc < 0)
				{
					/* this would be an FD change event */
					if ((rsslSocketChannel->newStream != RIPC_INVALID_SOCKET) && (rsslSocketChannel->tunnelingState == RIPC_TUNNEL_ACTIVE))
					{
						/* change around file descriptors */
						rsslSocketChannel->oldStream = rsslSocketChannel->stream;
						tmpTransportInfo = rsslSocketChannel->transportInfo;
						rsslSocketChannel->stream = rsslSocketChannel->newStream;
						rsslSocketChannel->transportInfo = rsslSocketChannel->newTransportInfo;
						(*(rsslSocketChannel->transportFuncs->shutdownTransport))(tmpTransportInfo);
						// Null out oldTransportInfo, because it has been free'd 
						rsslSocketChannel->newStream = RIPC_INVALID_SOCKET;
						rsslSocketChannel->newTransportInfo = NULL;
						*readret = RSSL_RET_READ_FD_CHANGE;

						return 0;
					}

					/* Note: ipcRead() returns -1 if error, -2 if connection is down. */
					if (cc == -2)
					{
						_rsslSetError(error, NULL, RSSL_RET_FAILURE, errno);
						snprintf((error->text), MAX_RSSL_ERROR_TEXT,
							"<%s:%d> Error:1002 ipcRead() failure. Connection reset by peer\n",
							__FILE__, __LINE__);

						*readret = RSSL_RET_FAILURE;

						return 0;
					}
					else
					{
						_rsslSetError(error, NULL, RSSL_RET_FAILURE, errno);
						snprintf(error->text, MAX_RSSL_ERROR_TEXT,
							"<%s:%d> Error: 1002 ipcRead() failure. System errno: (%d)\n",
							__FILE__, __LINE__, errno);

						*readret = RSSL_RET_FAILURE;

						return 0;
					}
				}
				rsslSocketChannel->inputBuffer->length += cc;
				inBytes = cc;
				if (rsslSocketChannel->blocking == 0)
					canRead = 0;

				tempLen = (RsslUInt32)(rsslSocketChannel->inputBuffer->length - rsslSocketChannel->inputBufCursor);
			}
			else
			{
				*readret = RSSL_RET_READ_WOULD_BLOCK;

				return 0;
			}
		}

		rsslSocketChannel->curInputBuf->protocol = 0;

		/* process standard ripc 3 byte header, same for all conn versions */

		RTR_GET_16(messageLength, (rsslSocketChannel->inputBuffer->buffer + rsslSocketChannel->inputBufCursor));
		if ((ipcLen = (RsslUInt32)messageLength) > (RsslUInt32)rsslSocketChannel->maxMsgSize)
		{
			_rsslSetError(error, NULL, RSSL_RET_FAILURE, errno);
			snprintf(error->text, MAX_RSSL_ERROR_TEXT,
				"<%s:%d> Error: 1007 Invalid Message Size. Message size is: (%d). Max Message size is(%zu)\n",
				__FILE__, __LINE__, ipcLen, rsslSocketChannel->inputBuffer->maxLength);

			*readret = RSSL_RET_FAILURE;

			return 0;
		}

		if (ipcLen < (u32)(rsslSocketChannel->version->dataHeaderLen - httpHeaderLen))
		{
			cont = 0;
			break;
		}

		cOpcode = rsslSocketChannel->inputBuffer->buffer[rsslSocketChannel->inputBufCursor + 2];
		ipcOpcode = (RsslUInt16)cOpcode;
		extendedHdr = 0;

		/* done processing fixed header */
		cont = 0;
		cHdrLen = 3; /* 3 bytes down */
	}

	if (!(ipcOpcode & IPC_COMP_DATA))
	{
		if (ipcLen < (RsslUInt32)(rsslSocketChannel->version->dataHeaderLen - httpHeaderLen))
		{
			_rsslSetError(error, NULL, RSSL_RET_FAILURE, errno);
			snprintf(error->text, MAX_RSSL_ERROR_TEXT,
				"<%s:%d> Error: 1007 Invalid Message Size. Message size is: (%d). Max Message size is(%zu)\n",
				__FILE__, __LINE__, ipcLen, rsslSocketChannel->inputBuffer->maxLength);


			*readret = RSSL_RET_FAILURE;

			return 0;
		}

		if (ipcLen > rsslSocketChannel->inputBuffer->maxLength)
		{
			_rsslSetError(error, NULL, RSSL_RET_FAILURE, errno);
			snprintf(error->text, MAX_RSSL_ERROR_TEXT,
				"<%s:%d> Error: 1007 Invalid Message Size. Message size is: (%d). Max Message size is(%zu)\n",
				__FILE__, __LINE__, ipcLen, rsslSocketChannel->inputBuffer->maxLength);

			*readret = RSSL_RET_FAILURE;

			return 0;
		}

		/* Check for valid opcode */
		if (!(ipcOpcode & IPC_DATA))
		{
			_rsslSetError(error, NULL, RSSL_RET_FAILURE, errno);
			snprintf(error->text, MAX_RSSL_ERROR_TEXT,
				"<%s:%d> Error: 1007 Invalid Message Opcode: (%d)\n",
				__FILE__, __LINE__, ipcOpcode);

			*readret = RSSL_RET_FAILURE;

			return 0;
		}
	}

	/* do not move input buf cursor yet, moved below */

	/* Check for the rest of the message */
	tempLen = (RsslUInt32)(rsslSocketChannel->inputBuffer->length - rsslSocketChannel->inputBufCursor);
	while (tempLen < ipcLen)
	{
		if (canRead)
		{
			IPC_MUTEX_UNLOCK(rsslSocketChannel);

			cc = (*(rsslSocketChannel->transportFuncs->readTransport))(
				rsslSocketChannel->transportInfo,
				(rsslSocketChannel->inputBuffer->buffer + rsslSocketChannel->inputBuffer->length),
				(ipcLen - tempLen), rwflags, error);

			IPC_MUTEX_LOCK(rsslSocketChannel);

			if (rsslSocketChannel->workState & RIPC_INT_SHTDOWN_PEND)
			{
				_rsslSetError(error, NULL, RSSL_RET_FAILURE, errno);
				snprintf(error->text, MAX_RSSL_ERROR_TEXT,
					"<%s:%d> Error: 1003 ipcReadSession() failed due to channel shutting down.\n",
					__FILE__, __LINE__);

				*readret = RSSL_RET_FAILURE;

				return 0;
			}

			if (cc < 0)
			{
				/* this would be an FD change event */
				if ((rsslSocketChannel->newStream != RIPC_INVALID_SOCKET) && (rsslSocketChannel->tunnelingState == RIPC_TUNNEL_ACTIVE))
				{
					/* change around file descriptors */
					rsslSocketChannel->oldStream = rsslSocketChannel->stream;
					tmpTransportInfo = rsslSocketChannel->transportInfo;
					rsslSocketChannel->stream = rsslSocketChannel->newStream;
					rsslSocketChannel->transportInfo = rsslSocketChannel->newTransportInfo;
					(*(rsslSocketChannel->transportFuncs->shutdownTransport))(tmpTransportInfo);
					// Null out oldTransportInfo, because it has been free'd 
					rsslSocketChannel->newStream = RIPC_INVALID_SOCKET;
					rsslSocketChannel->newTransportInfo = NULL;
					*readret = RSSL_RET_READ_FD_CHANGE;

					return 0;
				}

				/* Note: ipcRead() returns -1 if error, -2 if connection is down. */
				if (cc == -2)
				{
					_rsslSetError(error, NULL, RSSL_RET_FAILURE, errno);
					snprintf((error->text), MAX_RSSL_ERROR_TEXT,
						"<%s:%d> Error:1002 ipcRead() failure. Connection reset by peer\n",
						__FILE__, __LINE__);

					*readret = RSSL_RET_FAILURE;

					return 0;
				}
				else
				{
					_rsslSetError(error, NULL, RSSL_RET_FAILURE, errno);
					snprintf((error->text), MAX_RSSL_ERROR_TEXT,
						"<%s:%d> Error:1002 ipcRead() failure. System errno: (%d)\n",
						__FILE__, __LINE__, errno);

					*readret = RSSL_RET_FAILURE;

					return 0;
				}
			}
			rsslSocketChannel->inputBuffer->length += cc;
			inBytes = cc;
			if (rsslSocketChannel->blocking == 0)
				canRead = 0;

			tempLen = (RsslUInt32)(rsslSocketChannel->inputBuffer->length - rsslSocketChannel->inputBufCursor);
		}
		else
		{
			if (bytesRead != NULL)
				*bytesRead = inBytes;
			*readret = RSSL_RET_READ_WOULD_BLOCK;

			return 0;
		}
	}

	/* if we get here, we have a full message with respect to the length in the ripc header */
	/* if it was compressed, we should decompress it before processing the rest of the header */
	if (ipcOpcode & IPC_COMP_DATA)
	{
		ripcCompBuffer	compBuf;

		/* the offsets used for pre and post compression are:*/
		/* 4 - for the header offset of each message buffer received */
		/* 2 - for the two byte header field that is compressed within each msg */
		/*     if a compressed message is extended too two buffers, the 2nd buffer */
		/*     will not contain the two byte part of the header */
		if (!(rsslSocketChannel->decompressBuf))
		{
			_rsslSetError(error, NULL, RSSL_RET_FAILURE, errno);
			snprintf(error->text, MAX_RSSL_ERROR_TEXT,
				"<%s:%d> Error: 1007 ipcRead() Attempting to decompress when compression not enabled.\n",
				__FILE__, __LINE__);

			*readret = RSSL_RET_FAILURE;

			return 0;
		}

		/* this block of code sets up the header information in the decompress buffer
		* and points the compression at the right place to do decompression */
		if (rsslSocketChannel->decompressBuf->length == 0)
		{
			char *newHdr = rsslSocketChannel->decompressBuf->buffer;
			char *origHdr = rsslSocketChannel->inputBuffer->buffer + rsslSocketChannel->inputBufCursor;

			newHdr[0] = origHdr[0];  /* len */
			newHdr[1] = origHdr[1];  /* len */
			newHdr[2] = origHdr[2];  /* opcode */
			/* since we have the header the first time through, we can use ipcOpcode because this is always
			* parsed out before this */
			cHdrLen = 3;
			if (ipcOpcode & IPC_EXTENDED_FLAGS)
			{
				newHdr[3] = origHdr[3]; /* ipc ext flags */
				++cHdrLen;
				/* ipcFlags are not parsed out yet, so we have to look at the byte that represents ipcFlags, which is  */
				if (origHdr[3] & IPC_FRAG_HEADER)
				{
					/* add frag header - 4 byte length, plus frag id */
					newHdr[4] = origHdr[4];
					newHdr[5] = origHdr[5];
					newHdr[6] = origHdr[6];
					newHdr[7] = origHdr[7];
					/* frag id */
					newHdr[8] = origHdr[8];
					cHdrLen += 5;
					if (rsslSocketChannel->version->connVersion > CONN_VERSION_12)
					{
						/* if version 13 or greater, frag id is two bytes */
						newHdr[9] = origHdr[9];
						++cHdrLen;
					}
				}
				else if (origHdr[3] & IPC_FRAG)
				{
					/* add frag id */
					newHdr[4] = origHdr[4];
					++cHdrLen;
					if (rsslSocketChannel->version->connVersion > CONN_VERSION_12)
					{
						/* if version 13 or greater, frag id is two bytes */
						newHdr[5] = origHdr[5];
						++cHdrLen;
					}
				}
			}
			rsslSocketChannel->decompressBuf->length += cHdrLen;	/* decompress past the header on the first message only */
		}

		/* header has been set up, need to determine if we have to decompress or not */
		/* if this was a compression type that was not good at decompression, we may need to copy off content and decompress when next part comes */
		if ((rsslSocketChannel->inDecompress == RSSL_COMP_LZ4) && ((rsslSocketChannel->tempDecompressBuf->length) || (ipcOpcode & IPC_COMP_FRAG)))
		{
			/* need to come in here if this is LZ and its the first part of compression, or need to know its the second part */
			if (ipcOpcode & IPC_COMP_FRAG)
			{
				/* first part, copy it and do not decompress */
				/* copy only the compressed data */
				rsslSocketChannel->tempDecompressBuf->length = ipcLen - cHdrLen;
				MemCopyByInt(rsslSocketChannel->tempDecompressBuf->buffer, (rsslSocketChannel->inputBuffer->buffer + rsslSocketChannel->inputBufCursor + cHdrLen), rsslSocketChannel->tempDecompressBuf->length);
				/* set this to 0 to pass the avail_in check below */
				compBuf.avail_in = 0;
				/* set this to the header length, this will preserve our place in the decompress buffer */
				compBuf.next_out = rsslSocketChannel->decompressBuf->buffer + rsslSocketChannel->decompressBuf->length;
			}
			else
			{
				/* second part, copy it and decompress */
				MemCopyByInt((rsslSocketChannel->tempDecompressBuf->buffer + rsslSocketChannel->tempDecompressBuf->length), (rsslSocketChannel->inputBuffer->buffer + rsslSocketChannel->inputBufCursor + cHdrLen), (ipcLen - cHdrLen));
				rsslSocketChannel->tempDecompressBuf->length += (ipcLen - cHdrLen);
				/* decompress from temporary buffer */
				compBuf.next_in = rsslSocketChannel->tempDecompressBuf->buffer;
				compBuf.avail_in = (unsigned int)rsslSocketChannel->tempDecompressBuf->length;
				compBuf.next_out = rsslSocketChannel->decompressBuf->buffer + rsslSocketChannel->decompressBuf->length;
				compBuf.avail_out = (unsigned long)(rsslSocketChannel->decompressBuf->maxLength - rsslSocketChannel->decompressBuf->length);
				if ((*(rsslSocketChannel->inDecompFuncs->decompress)) (rsslSocketChannel->c_stream_in, &compBuf, 0, error) < 0)
				{
					_rsslSetError(error, NULL, RSSL_RET_FAILURE, errno);

					*readret = RSSL_RET_FAILURE;

					return 0;
				}

				/*the number of bytes decompressed and the number of header bytes associated with the message*/
				if (uncompBytesRead != NULL)
					*uncompBytesRead += (compBuf.bytes_out_used + cHdrLen + rsslSocketChannel->inBufProtOffset);

				/* reset temp buffer length for next use */
				rsslSocketChannel->tempDecompressBuf->length = 0;
			}
		}
		else
		{
			/* not LZ or single message LZ */
			compBuf.next_in = rsslSocketChannel->inputBuffer->buffer + rsslSocketChannel->inputBufCursor + cHdrLen;
			compBuf.avail_in = ipcLen - cHdrLen;
			compBuf.next_out = rsslSocketChannel->decompressBuf->buffer + rsslSocketChannel->decompressBuf->length;
			compBuf.avail_out = (unsigned long)(rsslSocketChannel->decompressBuf->maxLength - rsslSocketChannel->decompressBuf->length);
			if ((*(rsslSocketChannel->inDecompFuncs->decompress)) (rsslSocketChannel->c_stream_in, &compBuf, 0, error) < 0)
			{
				_rsslSetError(error, NULL, RSSL_RET_FAILURE, errno);

				*readret = RSSL_RET_FAILURE;

				return 0;
			}

			/*the number of bytes decompressed and the number of header bytes associated with the message*/
			if (uncompBytesRead != NULL)
				*uncompBytesRead += (compBuf.bytes_out_used + cHdrLen + rsslSocketChannel->inBufProtOffset);
		}

		/* We should always be able to decompress all the data into a single
		* buffer since we know what the maximum buffer size is.
		*/
		if (compBuf.avail_in != 0)
		{
			_rsslSetError(error, NULL, RSSL_RET_FAILURE, errno);
			snprintf(error->text, MAX_RSSL_ERROR_TEXT,
				"<%s:%d> Error: 1007 ipcRead() decompress failed.\n",
				__FILE__, __LINE__);

			*readret = RSSL_RET_FAILURE;

			return 0;
		}

		/* if we have a compressed message that is within two (e.g. tbitmap = 0x02)*/
		/* message buffers we don't notify the user until we have received both msg buffers */
		*moreData = (RsslInt32)ipcOpcode;

		if ((rsslSocketChannel->dbgFlags & RSSL_DEBUG_IPC_DUMP_COMP) && (rsslSocketChannel->dbgFlags & RSSL_DEBUG_IPC_DUMP_INIT))
		{
			(*(ripcDumpInFunc))(__FUNCTION__, rsslSocketChannel->inputBuffer->buffer + rsslSocketChannel->inputBufCursor,
				ipcLen, rsslSocketChannel->stream);
		}

		rsslSocketChannel->inputBufCursor += ipcLen;
		rsslSocketChannel->decompressBuf->length = (char*)compBuf.next_out - rsslSocketChannel->decompressBuf->buffer;

		/* return WOULD_BLOCK to let the user know we need to read more to complete the message */
		if (*moreData & IPC_COMP_FRAG)
		{

			_DEBUG_TRACE_READ("found IPC_COMP_FRAG\n")

			/* If we are at the end of our buffer, then reset and return RSSL_RET_READ_WOULD_BLOCK.
			If we have more in our buffer, then return RSSL_RET_SUCCESS so the client calls read again */
			if ((size_t)(rsslSocketChannel->inputBufCursor) == rsslSocketChannel->inputBuffer->length)
			{
				rsslSocketChannel->inputBuffer->length = 0;
				rsslSocketChannel->inputBufCursor = 0;
				*readret = RSSL_RET_READ_WOULD_BLOCK;
			}
			else
				*readret = RSSL_RET_SUCCESS;

			if (bytesRead != NULL)
				*bytesRead = inBytes;

			return 0;
		}

		/* header length is no longer in the header */
		/* need to use the values in the decompress buffer */
		rsslSocketChannel->curInputBuf->buffer = rsslSocketChannel->decompressBuf->buffer;
		rsslSocketChannel->curInputBuf->length = rsslSocketChannel->decompressBuf->length;
		rsslSocketChannel->curInputBuf->maxLength = rsslSocketChannel->curInputBuf->length;

		rsslSocketChannel->decompressBuf->protocol = 0;
		rsslSocketChannel->decompressBuf->protocolHdr = 0;
		rsslSocketChannel->decompressBuf->length = 0;

		/* we need to set ipcOpcode and ipcFlags again - if this was a multi part compressed buffer
		* those values may have been lost when decompressing second part
		*/
		ipcOpcode = rsslSocketChannel->curInputBuf->buffer[2]; /* opcode location in standard ripc header */
		if (ipcOpcode & IPC_EXTENDED_FLAGS)
			ipcFlags = rsslSocketChannel->curInputBuf->buffer[3];

		if ((size_t)(rsslSocketChannel->inputBufCursor) == rsslSocketChannel->inputBuffer->length)
		{
			rsslSocketChannel->inputBuffer->length = 0;
			rsslSocketChannel->inputBufCursor = 0;
			/* need to reset this - all the flags we set in here
			make the app think there is more data to read */
			*moreData = 0;
		}
		else
			*moreData = (RsslInt32)(rsslSocketChannel->inputBuffer->length - rsslSocketChannel->inputBufCursor);
	}
	else
	{
		/* not compressed so set up input buffer to be parsed from */
		rsslSocketChannel->curInputBuf->length = ipcLen;
		rsslSocketChannel->curInputBuf->maxLength = rsslSocketChannel->curInputBuf->length;
		if (uncompBytesRead != NULL)
			/* ->inBufProtOffset accounts for any bytes read is there is an underlying protocol
			 * like a WebSocket transport */
			*uncompBytesRead += (RsslInt32)(rsslSocketChannel->curInputBuf->length + rsslSocketChannel->inBufProtOffset);
		rsslSocketChannel->curInputBuf->buffer = rsslSocketChannel->inputBuffer->buffer + rsslSocketChannel->inputBufCursor;

		rsslSocketChannel->inputBufCursor += ipcLen;
	}

	switch (rsslSocketChannel->version->connVersion)
	{
	case CONN_VERSION_10:
	case CONN_VERSION_11:
	case CONN_VERSION_12:
	{
		RsslUInt8  cFragId;

		if (ipcOpcode & IPC_EXTENDED_FLAGS)
		{
			/* read extended flags byte */
			extendedHdr++;
			cFlags = rsslSocketChannel->curInputBuf->buffer[3];
			ipcFlags = (RsslUInt16)cFlags;

			if (ipcFlags & IPC_FRAG_HEADER)
			{
				/* read fragmentation header */
				/* read 4 byte length */
				/* fragLength */

				_move_u32_swap(fragLength, (rsslSocketChannel->curInputBuf->buffer + 4));

				cFragId = rsslSocketChannel->curInputBuf->buffer[8];
				*fragId = (RsslInt32)cFragId;
				cHdrLen = 9;
				extendedHdr += 5;
			}
			else if (ipcFlags & IPC_FRAG)
			{
				/* read frag id */
				cFragId = rsslSocketChannel->curInputBuf->buffer[4];
				*fragId = (RsslInt32)cFragId;

				cHdrLen = 5;
				extendedHdr++;
			}
			else
				cHdrLen = 4;
		}
		else
		{
			cHdrLen = 3;
		}
	}
	break;
	/* Need seperate handling for this because of different fragID length */
	case CONN_VERSION_13:
	case CONN_VERSION_14:
	{
		RsslUInt16 sFragId;

		if (ipcOpcode & IPC_EXTENDED_FLAGS)
		{
			/* read extended flags byte */
			extendedHdr++;
			cFlags = rsslSocketChannel->curInputBuf->buffer[3];
			ipcFlags = (RsslUInt16)cFlags;

			_DEBUG_TRACE_READ(" extended flags (ipcFlags=0x%x)\n", ipcFlags)

			if (ipcFlags & IPC_FRAG_HEADER)
			{
				/* read fragmentation header */
				/* read 4 byte length */
				/* frag ID */

				_move_u32_swap(fragLength, (rsslSocketChannel->curInputBuf->buffer + 4));

				_move_u16_swap(&sFragId, (rsslSocketChannel->curInputBuf->buffer + 8));
				*fragId = (RsslInt32)sFragId;
				cHdrLen = 10;
				extendedHdr += rsslSocketChannel->version->firstFragHdrLen;

				_DEBUG_TRACE_READ(" extended flags IPC_FRAG_HEADER (sFragId=%d)\n", sFragId)

			}
			else if (ipcFlags & IPC_FRAG)
			{
				/* read frag id */
				_move_u16_swap(&sFragId, (rsslSocketChannel->curInputBuf->buffer + 4));
				*fragId = (RsslInt32)sFragId;

				cHdrLen = 6;
				extendedHdr += rsslSocketChannel->version->subsequentFragHdrLen;

				_DEBUG_TRACE_READ(" extended flags IPC_FRAG (sFragId=%d)\n", sFragId)

			}
			else
				cHdrLen = 4;
		}
		else
		{
			cHdrLen = 3;
		}
	}
	break;

	default:
		_rsslSetError(error, NULL, RSSL_RET_FAILURE, errno);
		snprintf(error->text, MAX_RSSL_ERROR_TEXT,
			"<%s:%d> Error: 1007 Unknown connection version: 0x%x\n",
			__FILE__, __LINE__, rsslSocketChannel->version->connVersion);

		*readret = RSSL_RET_FAILURE;

		return(0);
		break;
	}

	/* advance buffer to return by the processed header length */
	rsslSocketChannel->curInputBuf->length -= cHdrLen;
	rsslSocketChannel->curInputBuf->maxLength = rsslSocketChannel->curInputBuf->length;
	rsslSocketChannel->curInputBuf->buffer += cHdrLen;

	if (rsslSocketChannel->dbgFlags & RSSL_DEBUG_IPC_DUMP_IN)
		(*ripcDumpInFunc)(__FUNCTION__,rsslSocketChannel->curInputBuf->buffer - cHdrLen,
		(RsslUInt32)(rsslSocketChannel->curInputBuf->length + cHdrLen), rsslSocketChannel->stream);

	/* Reset the read information
	* when complete with message.
	*/
	if ((size_t)(rsslSocketChannel->inputBufCursor) == rsslSocketChannel->inputBuffer->length)
	{
		rsslSocketChannel->inputBuffer->length = 0;
		rsslSocketChannel->inputBufCursor = 0;
	}
	else
	{
		*moreData = (RsslInt32)(rsslSocketChannel->inputBuffer->length - rsslSocketChannel->inputBufCursor);
	}

	if (ipcOpcode & IPC_PACKING)
		*packing = 1;
	else
		*packing = 0;

	if (bytesRead != NULL)
		*bytesRead = inBytes;

	*readret = RSSL_RET_SUCCESS;

	return(rsslSocketChannel->curInputBuf);
}

rtr_msgb_t *ipcDataBuffer(RsslSocketChannel *rsslSocketChannel, RsslInt32 size, RsslError *error)
{
	rtr_msgb_t			*retBuf = 0;
	RsslInt32			headerLength;
	RsslInt32			neededSize = size;

	if (IPC_NULL_PTR(rsslSocketChannel, "ipcDataBuffer", "rsslSocketChannel", error))
		return 0;

	headerLength = rsslSocketChannel->version->dataHeaderLen + rsslSocketChannel->version->footerLen;
	neededSize += headerLength;

	IPC_MUTEX_LOCK(rsslSocketChannel);

	retBuf = (*(rsslSocketChannel->protocolFuncs->getPoolBuffer))(&(rsslSocketChannel->guarBufPool->bufpool), neededSize);
	if (retBuf == 0)
	{
		if (ipcFlushSession(rsslSocketChannel, error) >= 0)
			retBuf = (*(rsslSocketChannel->protocolFuncs->getPoolBuffer))(&(rsslSocketChannel->guarBufPool->bufpool), neededSize);
	}

	if (retBuf != 0)
	{
		retBuf->buffer += rsslSocketChannel->version->dataHeaderLen;
		retBuf->maxLength -= headerLength;
	_DEBUG_TRACE_BUFFER("     After mx -= Buffer:%p buf %p prot %d ln %u mxln %u\n", 
															(retBuf ? retBuf:0), 
															(retBuf ? retBuf->buffer:0),
															(retBuf ? retBuf->protocol:0),
															(retBuf ? retBuf->length:0),
															(retBuf ? retBuf->maxLength:0))
	}
	else
	{
		error->sysError = 0;
		error->rsslErrorId = RSSL_RET_BUFFER_NO_BUFFERS;
		snprintf(error->text, MAX_RSSL_ERROR_TEXT,
			"<%s:%d> Error: 1009 ipcDataBuffer() failed, out of output buffers. The output buffer may need to be flushed.\n",
			__FILE__, __LINE__);
	}

	IPC_MUTEX_UNLOCK(rsslSocketChannel);

	return(retBuf);
}

rtr_msgb_t *ipcFragmentationDataBuffer(RsslSocketChannel *rsslSocketChannel, RsslUInt32 firstHeader, RsslUInt32 dataSize, RsslError *error)
{
	rtr_msgb_t			*retBuf = 0;
	RsslInt32			headerLength;
	rtr_msgb_t			*pbuf = 0;
	RsslUInt32			neededSize;

	if (IPC_NULL_PTR(rsslSocketChannel, "ipcFragmentationDataBuffer", "rsslSocketChannel", error))
		return 0;

	IPC_MUTEX_LOCK(rsslSocketChannel);

	headerLength = rsslSocketChannel->version->dataHeaderLen + rsslSocketChannel->version->footerLen;

	do
	{
		if (firstHeader)
		{
			neededSize = rsslSocketChannel->maxMsgSize - RWS_MAX_HEADER_SIZE;
			dataSize -= (neededSize - (headerLength + rsslSocketChannel->version->firstFragHdrLen + 1));
		}
		else
		{
			neededSize = (dataSize + (headerLength + rsslSocketChannel->version->subsequentFragHdrLen + 1));
			if ( neededSize  <= (rsslSocketChannel->maxMsgSize - RWS_MAX_HEADER_SIZE) )
			{
				dataSize = 0;
			}
			else
			{
				neededSize = rsslSocketChannel->maxMsgSize - RWS_MAX_HEADER_SIZE;
				dataSize -= (neededSize - (headerLength + rsslSocketChannel->version->subsequentFragHdrLen + 1));
			}
		}

		pbuf = (*(rsslSocketChannel->protocolFuncs->getPoolBuffer))(&(rsslSocketChannel->guarBufPool->bufpool), neededSize);
		if (pbuf == 0)
		{
			if (ipcFlushSession(rsslSocketChannel, error) >= 0)
				pbuf = (*(rsslSocketChannel->protocolFuncs->getPoolBuffer))(&(rsslSocketChannel->guarBufPool->bufpool), neededSize);
		}

		if (pbuf == 0)
		{
			while (retBuf)
			{
				pbuf = retBuf->nextMsg;
				retBuf->nextMsg = 0;
				rtr_dfltcFreeMsg(retBuf);
				retBuf = pbuf;
			}
			break;
		}
		else
		{
			pbuf->protocol = 0;	/* pad is protocol */
			/* the + 1 is for the extra flags */
			if (firstHeader)
			{
				/* we add for the frag header data */
				pbuf->buffer += (rsslSocketChannel->version->dataHeaderLen + rsslSocketChannel->version->firstFragHdrLen + 1);
				pbuf->maxLength -= (headerLength + rsslSocketChannel->version->firstFragHdrLen + 1);
				retBuf = pbuf;

				firstHeader = 0;
			}
			else
			{
				pbuf->buffer += (rsslSocketChannel->version->dataHeaderLen + rsslSocketChannel->version->subsequentFragHdrLen + 1);
				pbuf->maxLength -= (headerLength + rsslSocketChannel->version->subsequentFragHdrLen + 1);

				if (retBuf)
					retBuf->nextMsg = pbuf;
				else
					retBuf = pbuf;
			}
		}
	} while (dataSize);

	IPC_MUTEX_UNLOCK(rsslSocketChannel);

	return(retBuf);
}

RsslInt32 ipcReleaseDataBuffer(RsslSocketChannel *rsslSocketChannel, rtr_msgb_t *msgb, RsslError *error)
{
	RsslRet				retval = RSSL_RET_SUCCESS;
	RsslInt32			first = 1;
	rtr_msgb_t			*nmb;
	rtr_msgb_t			*nextBuf = 0;
	RsslInt32			i = 0;

	if (IPC_NULL_PTR(rsslSocketChannel, "ipcReleaseDataBuffer", "rsslSocketChannel", error))
		return RSSL_RET_FAILURE;

	IPC_MUTEX_LOCK(rsslSocketChannel);

	/* this will take care of the extra header stuff for chained messages */
	if ((msgb) && (msgb->nextMsg))
	{
		nextBuf = msgb;

		/* should loop through the chain of buffers -
		first buffer has been moved up 6 places (for frag header)
		every other buffer has been moved up 2 places for frag id */
		while (nextBuf)
		{
			if (msgb->protocolHdr)
				msgb->protocolHdr = 0;

			if (first)
			{
				/* remove frag header */
				nextBuf->buffer -= 6;
				first = 0;
				nextBuf = nextBuf->nextMsg;
			}
			else
			{
				/* remove frag id and extended flags */
				nextBuf->buffer -= 2;
				nextBuf = nextBuf->nextMsg;
			}
		}
	}

	/* this will take care of any fragments that arent part of the chain */
	if (msgb)
		msgb->buffer -= msgb->fragOffset;

	while (msgb)
	{
		nmb = msgb->nextMsg;
		if (msgb->protocolHdr)
			msgb->protocolHdr = 0;

		if (msgb->maxLength > rsslSocketChannel->maxMsgSize)
			rwsReleaseLargeBuffer(rsslSocketChannel, msgb);
		else
		{
			msgb->nextMsg = 0;
			msgb->buffer -= rsslSocketChannel->version->dataHeaderLen;
			rtr_dfltcFreeMsg(msgb);
		}
		msgb = nmb;
	}

	/* retval = rsslSocketChannel->outputBufLength; */
	for (i = 0; i < RIPC_MAX_PRIORITY_QUEUE; i++)
		retval += rsslSocketChannel->priorityQueues[i].queueLength;

	IPC_MUTEX_UNLOCK(rsslSocketChannel);

	return(retval);
}

rtr_bufferpool_t *ipcCreatePool(RsslInt32 max_bufs, RsslMutex *mutex)
{
	rtr_bufferpool_t *retpool = NULL;
	rtr_dfltcbufferpool_t *cpool;

	cpool = rtr_dfltcAllocPool(1, max_bufs, 10, 0, 0, mutex);
	if (cpool == 0)
	{
		return(retpool);
	}
	retpool = &(cpool->bufpool);
	return(retpool);
}

rtr_msgb_t *ipcGetPoolBuffer(rtr_bufferpool_t *bufpool, size_t size)
{
	rtr_msgb_t			*retBuf = 0;

	retBuf = rtr_dfltcAllocMsg(bufpool, size);

	_DEBUG_TRACE_BUFFER("Returning Pool Buffer:%p buf %p prot %d ln %u mxln %u\n", 
															(retBuf ? retBuf:0), 
															(retBuf ? retBuf->buffer:0),
															(retBuf ? retBuf->protocol:0),
															(retBuf ? retBuf->length:0),
															(retBuf ? retBuf->maxLength:0))
	return(retBuf);
}

/* This is additional method to handle additional header information if any. */
RsslInt32 ipcReadPrependTransportHdr(void* transport, char* buffer, int len, ripcRWFlags flags, int* ret, RsslError* error)
{
	/* Do nothing for socket transport */
	*ret = RSSL_RET_SUCCESS;

	return (0);
}

/* This is additional header length for transport protocol */
RsslInt32 ipcAdditionalHeaderLength()
{
	return 0;
}

/* This additional call is needed for other transports to have the ability to parse their
 * protocol header before passing along what is being read within this abstracted call for
 * _SOCKET typ connection */
RsslInt32 ipcReadTransportMsg(void * transport, char *buf, int len, ripcRWFlags flags, RsslError *error)
{
	RsslInt32 bytesRead;
	RsslSocketChannel *rsslSocketChannel = (RsslSocketChannel *)transport;

	if (IPC_NULL_PTR(rsslSocketChannel, "", "rsslSocketChannel", error))
		return RSSL_RET_FAILURE;

	IPC_MUTEX_UNLOCK(rsslSocketChannel);

	bytesRead = ((*(rsslSocketChannel->transportFuncs->readTransport))(rsslSocketChannel->transportInfo, 
																  buf, len, flags, error));
	IPC_MUTEX_LOCK(rsslSocketChannel);

	return bytesRead;
}

/* This additional call is needed for other transports to have the ability to parse their
 * protocol header before passing along what is being read within this abstracted call for
 * _SOCKET typ connection */
RsslInt32 ipcPrependTransportHdr(void * transport, rtr_msgb_t *buf, RsslError *error)
{
	return (0);
}

RsslInt32 ipcWrtHeader(RsslSocketChannel *rsslSocketChannel, RsslError *error)
{
	RsslRet		retval = RSSL_RET_SUCCESS;

	if (IPC_NULL_PTR(rsslSocketChannel, "ipcWrtHeader", "rsslSocketChannel", error))
		return RSSL_RET_FAILURE;

	IPC_MUTEX_LOCK(rsslSocketChannel);

	if ((retval = ipcIntWrtHeader(rsslSocketChannel, error)) >= 0)
		retval = ipcFlushSession(rsslSocketChannel, error);

	IPC_MUTEX_UNLOCK(rsslSocketChannel);

	return(retval);
}

RsslInt32 ipcIntWrtHeader(RsslSocketChannel *rsslSocketChannel, RsslError *error)
{
	RsslInt32		totalSize;
	caddr_t			hdr;
	RsslInt32 		IPC_header_size;
	rtr_msgb_t		*buffer = 0;
	rtr_msgb_t		*lastmb = 0;
	RsslUInt16		messageLength;
	RsslInt32		neededSize;
	RsslUInt8		flags = 0x0;
	RsslRet			retval = RSSL_RET_SUCCESS;
	RsslInt32		i = 0;
	RsslInt32		iterator = 0;
	RsslQueueLink	*pLink = 0;


	if (rsslSocketChannel->workState & RIPC_INT_SHTDOWN_PEND)
	{
		_rsslSetError(error, NULL, RSSL_RET_FAILURE, errno);
		snprintf(error->text, MAX_RSSL_ERROR_TEXT,
			"<%s:%d> Error: 1003 ipcIntWrtHeader() failed due to channel shutting down.\n",
			__FILE__, __LINE__);

		return RSSL_RET_FAILURE;
	}

	/* if we are tunneling, dataHeaderLen already has 6 bytes reserved for chunking */
	/* this is plenty for writing both chunk header and footer onto a ping message */
	IPC_header_size = rsslSocketChannel->version->dataHeaderLen;
	neededSize = IPC_header_size;

	/* if we have stuff in the queues, just flush that */
	for (i = 0; i < RIPC_MAX_PRIORITY_QUEUE; i++)
		retval += rsslSocketChannel->priorityQueues[i].queueLength;

	if (retval > 0)
		return ipcFlushSession(rsslSocketChannel, error);

	/* get buffer here of proper size */
	//if ((buffer = rtr_dfltcAllocMsg(&(rsslSocketChannel->guarBufPool->bufpool), neededSize)) == 0)
	buffer = (*(rsslSocketChannel->protocolFuncs->getPoolBuffer))(&(rsslSocketChannel->guarBufPool->bufpool), neededSize);
	if (buffer == 0)
	{
		if (ipcFlushSession(rsslSocketChannel, error) >= 0)
			//buffer = rtr_dfltcAllocMsg(&(rsslSocketChannel->guarBufPool->bufpool), neededSize);
			buffer = (*(rsslSocketChannel->protocolFuncs->getPoolBuffer))(&(rsslSocketChannel->guarBufPool->bufpool), neededSize);
	}

	if (buffer == 0)
	{
		error->sysError = 0;
		error->rsslErrorId = RSSL_RET_BUFFER_NO_BUFFERS;
		snprintf(error->text, MAX_RSSL_ERROR_TEXT,
			"<%s:%d> Error: 1009 ipcDataBuffer() failed, out of ouptut buffers. the output buffer may need to be flushed.\n",
			__FILE__, __LINE__);

		return RSSL_RET_FAILURE;
	}

	buffer->nextMsg = 0;

	hdr = (caddr_t)buffer->buffer;

	switch (rsslSocketChannel->version->connVersion)
	{
	case CONN_VERSION_10:
	case CONN_VERSION_11:
	case CONN_VERSION_12:
	case CONN_VERSION_13:
	case CONN_VERSION_14:
		messageLength = (RsslUInt16)(IPC_header_size);
		flags |= IPC_DATA;

		if (rsslSocketChannel->httpHeaders)
		{
			/* take the http header overhead out of the message length */
			messageLength -= 6;

			/* this should come back as 3 */
			iterator = sprintf(hdr, "%x", messageLength);
			IPC_ADD_CR_LF(hdr, iterator);
		}

		/* we only want this to be the length of the message itself */
		RTR_PUT_16((hdr + iterator), messageLength);
		iterator += 2;

		hdr[iterator++] = (char)flags;

		/* add http footer if needed */
		if (rsslSocketChannel->httpHeaders)
		{
			IPC_ADD_CR_LF(hdr, iterator);
		}

		/* End */
		buffer->length = totalSize = iterator;

		if (rsslSocketChannel->dbgFlags & RSSL_DEBUG_IPC_DUMP_OUT)
			(*ripcDumpOutFunc)(__FUNCTION__, buffer->buffer, (RsslUInt32)(buffer->length), rsslSocketChannel->stream);
		break;

	default:
		_rsslSetError(error, NULL, RSSL_RET_FAILURE, errno);
		snprintf(error->text, MAX_RSSL_ERROR_TEXT,
			"<%s:%d> Error: 1007 Unknown connection version 0x%x.\n",
			__FILE__, __LINE__, rsslSocketChannel->version->connVersion);

		/* since I created this buffer locally I need to release it in every case - error or success */
		buffer->buffer = 0;
		buffer->length = 0;
		rtr_dfltcFreeMsg(buffer);
		return RSSL_RET_FAILURE;
		break;
	}

	/* Set/populate the prefix protocol header if one exists */
	(*(rsslSocketChannel->protocolFuncs->prependTransportHdr))((void*)rsslSocketChannel, buffer, error);

	totalSize += buffer->protocolHdrLength;

	buffer->local = buffer->buffer;

	/* it is always assumed pings are high priority */
	rsslSocketChannel->priorityQueues[0].queueLength += totalSize;
	pLink = rsslQueuePeekBack(&rsslSocketChannel->priorityQueues[0].priorityQueue);

	if (pLink && (lastmb = RSSL_QUEUE_LINK_TO_OBJECT(rtr_msgb_t, link, pLink)) && ((lastmb->buffer + lastmb->length) == buffer->buffer))
	{
		lastmb->length += buffer->length;
		lastmb->maxLength += buffer->length;
		rtr_dfltcFreeMsg(buffer);
		buffer->buffer = 0;
		buffer->length = 0;
	}
	else
	{
		/* pings are always assumed high priority */
		rsslQueueAddLinkToBack(&(rsslSocketChannel->priorityQueues[0].priorityQueue), &(buffer->link));
	}

	rsslSocketChannel->bytesOutLastMsg += (RsslUInt32)(buffer->length);

	/* return total length of all output buffers here */
	for (i = 0; i < RIPC_MAX_PRIORITY_QUEUE; i++)
		retval += rsslSocketChannel->priorityQueues[i].queueLength;
	return retval;
}


RsslRet ipcWriteSession(RsslSocketChannel *rsslSocketChannel, rsslBufferImpl *rsslBufferImpl, RsslInt32 wFlags, RsslInt32 *bytesWritten,
	RsslInt32 *uncompBytesWritten, RsslInt32 forceFlush, RsslError *error)
{
	RsslRet			retval = RSSL_RET_SUCCESS;
	RsslInt32		i = 0;
	RsslInt32		totalSize;
	caddr_t			hdr;
	caddr_t			chunkhdr;
	caddr_t			footeraddr;
	RsslInt32		IPC_header_size, footer_size;
	rtr_msgb_t		*nmb,*lastmb;
	RsslUInt32		messageLength;
	RsslUInt8		headerLength = 0;
	RsslUInt8 		flags = 0; /* header flags - always present */
	RsslUInt8		opCodes = 0; /* extended flags - only present when 0x4 is set on flags */
	RsslInt32		uncompBytes = 0;
	RsslInt32		chunkLen = 0;
	RsslUInt16		httpHeaderLen = 0;
	RsslUInt16		tempLen;
	rtr_msgb_t		*compressedmb1;
	rtr_msgb_t		*msgb = NULL;
	RsslUInt32		size = rsslBufferImpl->buffer.length;
	RsslUInt32		fragId = rsslBufferImpl->fragId;
	RsslQueueLink	*pLink = 0;

	_DEBUG_TRACE_WRITE("called\n")

	if (IPC_NULL_PTR(rsslSocketChannel, "ipcWriteSession", "rsslSocketChannel", error))
		return RSSL_RET_FAILURE;

	IPC_MUTEX_LOCK(rsslSocketChannel);

	if (rsslSocketChannel->workState & RIPC_INT_SHTDOWN_PEND)
	{
		_rsslSetError(error, NULL, RSSL_RET_FAILURE, errno);
		snprintf(error->text, MAX_RSSL_ERROR_TEXT,
			"<%s:%d> Error: 1003 ipcIntWrtSess() failed due to channel shutting down.\n",
			__FILE__, __LINE__);

		IPC_MUTEX_UNLOCK(rsslSocketChannel);

		return RSSL_RET_FAILURE;
	}

	if (rsslBufferImpl->bufferInfo == 0)
        {
                _rsslSetError(error, NULL, RSSL_RET_FAILURE, errno);
                snprintf(error->text, MAX_RSSL_ERROR_TEXT,
                        "<%s:%d> Error: 1007 ipcIntWrtSess() failed due the buffer has been released.\n",
                        __FILE__, __LINE__);

                IPC_MUTEX_UNLOCK(rsslSocketChannel);

                return RSSL_RET_FAILURE;
        }

	msgb = (rtr_msgb_t*)rsslBufferImpl->bufferInfo;
	rsslSocketChannel->bytesOutLastMsg = 0;
	IPC_header_size = rsslSocketChannel->version->dataHeaderLen;
	footer_size = rsslSocketChannel->version->footerLen;

	/* for first message in the chain, add the Frag header flag and header */
	if ((rsslBufferImpl->fragmentationFlag == BUFFER_IMPL_FIRST_FRAG_HEADER) && (rsslBufferImpl->fragId > 0))
	{
		flags |= IPC_EXTENDED_FLAGS;
		opCodes |= IPC_FRAG_HEADER;
	}

	if ((rsslBufferImpl->fragmentationFlag == BUFFER_IMPL_SUBSEQ_FRAG_HEADER) && (rsslBufferImpl->fragId > 0))
	{
		flags |= IPC_EXTENDED_FLAGS;
		opCodes |= IPC_FRAG;
	}

	if (rsslBufferImpl->packingOffset > 0)
		flags |= IPC_PACKING;

	while (msgb)
	{
		if (msgb->length > msgb->maxLength)
		{
			_rsslSetError(error, NULL, RSSL_RET_FAILURE, errno);
			snprintf(error->text, MAX_RSSL_ERROR_TEXT,
				"<%s:%d> Error: 1004 ipcWriteSession() failed, buffer->length set to value greater than requested buffer size length %zu > maxLength %zu (protocol %u).\n",
				__FILE__, __LINE__, msgb->length, msgb->maxLength, msgb->protocol);

			retval = RSSL_RET_FAILURE;
			break;
		}

		nmb = msgb->nextMsg;
		msgb->nextMsg = 0;

		if (msgb->length > 0)
		{
			if ((nmb == 0) && (msgb->length != msgb->maxLength))
			{
				rtr_dfltcSetUsedLast(rsslSocketChannel->guarBufPool, msgb);
			}
			if (flags & IPC_EXTENDED_FLAGS)
			{
				/* if frag header is set, we add its overhead (1 for extended header, 4 for length, 1 (or 2 when conn version greater than 12) for frag ID)
				* if frag is set, we add 2 (1 for extended header, 1 (or 2 when connver greater than 12) for frag Id)
				* else its not fragmented so there is no fragmentation header - just add 1 for extended header
				*/
				if (opCodes & IPC_FRAG_HEADER)
					msgb->buffer -= (IPC_header_size + rsslSocketChannel->version->firstFragHdrLen + 1);
				else if (opCodes & IPC_FRAG)
					msgb->buffer -= (IPC_header_size + rsslSocketChannel->version->subsequentFragHdrLen + 1);
				else
					msgb->buffer -= (IPC_header_size + 1);
			}
			else
				msgb->buffer -= IPC_header_size;

			hdr = (caddr_t)msgb->buffer;

			/* if we are writing to a tunneling session we need the chunk length in here */
			if (rsslSocketChannel->httpHeaders)
			{
				httpHeaderLen = 6;
				chunkhdr = hdr;
				hdr += httpHeaderLen;
			}

			/* need to set this to NULL */
			compressedmb1 = NULL;

			switch (rsslSocketChannel->version->connVersion)
			{
			case CONN_VERSION_10:
			case CONN_VERSION_11:
			case CONN_VERSION_12:
			case CONN_VERSION_13: /* even though ver 13 has two byte frag ID, didnt want to duplicate all this code */
			case CONN_VERSION_14:
				/* if we are compressing, and
				* the compressQueue we are using matches this message or hasnt been set and
				* the message is greater or equal to than the minimum size
				*/
				if (((rsslSocketChannel->outCompFuncs != 0) &&
					((rsslSocketChannel->compressQueue == -1) || (rsslSocketChannel->compressQueue == msgb->priority))) &&
					(msgb->length >= rsslSocketChannel->lowerCompressionThreshold) &&
					((rsslSocketChannel->safeLZ4 == 0) || ((rsslSocketChannel->safeLZ4 == 1) && (msgb->length <= rsslSocketChannel->upperCompressionThreshold))) &&
					(!(wFlags & RIPC_WRITE_DO_NOT_COMPRESS)))
				{
					/* get first buffer to compress into */
					/* need to unlock the mutex to avoid deadlock */
					IPC_MUTEX_UNLOCK(rsslSocketChannel);

					compressedmb1 = ipcDataBuffer(rsslSocketChannel, rsslSocketChannel->maxUserMsgSize, error);

					IPC_MUTEX_LOCK(rsslSocketChannel);

					/* if this failed, just send it normally */
				}

				/* if we couldnt get a buffer or we are not compressing go through this code */
				if (compressedmb1 == 0)
				{

					_DEBUG_TRACE_WRITE("NOT doing compression (flags=0x%x, wFlags=0x%x, opCodes=0x%x, fragId=0x%x, len=%llu)\n",
						flags, wFlags, opCodes, fragId, msgb->length)

					if (flags & IPC_EXTENDED_FLAGS)
					{
						if (opCodes & IPC_FRAG_HEADER)
							headerLength = IPC_header_size + rsslSocketChannel->version->firstFragHdrLen + 1;
						else if (opCodes & IPC_FRAG)
							headerLength = IPC_header_size + rsslSocketChannel->version->subsequentFragHdrLen + 1;
						else
							headerLength = IPC_header_size + 1;
					}
					else
						headerLength = IPC_header_size;

					messageLength = (RsslUInt32)(msgb->length + headerLength);
					uncompBytes += messageLength;

					/* add the chunking header if we are tunneling */
					if (rsslSocketChannel->httpHeaders)
					{
						/* we should have 6 bytes reserved at the beginning of the message for this */
						/* this should never consume more than 4 bytes - if it does we have
						trouble */
						/* do not want this length to include the http header */
						chunkLen = sprintf(chunkhdr, "%x", (messageLength - httpHeaderLen));
						/* add ; in any space as comment padding */
						while (chunkLen < 4)
							chunkLen += sprintf(chunkhdr + chunkLen, "%s", ";");
						/* add the carrage return line feed */
						IPC_ADD_CR_LF(chunkhdr, chunkLen);

						/* messageLength includes httpHeaderLen */
						footeraddr = (caddr_t)msgb->buffer + messageLength;
						/* add the footer */
					}

					/* Change header */
					/* we do not want any http header info included in this length */
					/* httpHeaderLen is usually 0 unless we are tunneling */
					tempLen = messageLength - httpHeaderLen;
					RTR_PUT_16(hdr, tempLen);
					hdr[2] = (char)flags | IPC_DATA;			/* hdr.IPC_opcode */

					if (flags & IPC_EXTENDED_FLAGS)
					{
						/* Add extended flags */
						hdr[3] = (char)opCodes;

						if (opCodes & IPC_FRAG_HEADER)
						{
							/* add frag header */
							/* move 32 bit int here */
							_move_u32_swap(hdr + 4, &size);

							if (rsslSocketChannel->version->connVersion > CONN_VERSION_12)
							{
								RsslUInt16 sFragId = (RsslUInt16)fragId;
								_move_u16_swap((hdr + 8), &sFragId);
							}
							else
								hdr[8] = (RsslUInt8)fragId;
						}
						else if (opCodes & IPC_FRAG)
						{
							if (rsslSocketChannel->version->connVersion > CONN_VERSION_12)
							{
								RsslUInt16 sFragId = (RsslUInt16)fragId;
								_move_u16_swap((hdr + 4), &sFragId);
							}
							else
								hdr[4] = (RsslUInt8)fragId;
						}
					}

					/* put on the chunking footer if we are tunneling */
					if (rsslSocketChannel->httpHeaders)
					{
						RsslUInt8 index = 0;
						IPC_ADD_CR_LF(footeraddr, index);
					}

					msgb->length = totalSize = (messageLength + footer_size);

					if (rsslSocketChannel->dbgFlags & RSSL_DEBUG_IPC_DUMP_OUT)
						(*ripcDumpOutFunc)(__FUNCTION__, msgb->buffer, (RsslUInt32)(msgb->length), rsslSocketChannel->stream);
				}
				else /* _conn_version_10/11/12/13 && compression */
				{
					RsslUInt16				messageLength, compLen1, compLen2;
					RsslUInt16				tempCompLen1 = 0;
					RsslUInt8				headerLength = 0;
					rtr_msgb_t				*compressedmb2;
					ripcCompBuffer				compBuf;


					_DEBUG_TRACE_WRITE("doing compression (flags = 0x%x, wFlags = 0x%x, opCodes = 0x%x, fragId = 0x%x)\n",
						flags, wFlags, opCodes, fragId)


					if (flags & IPC_EXTENDED_FLAGS)
					{
						if (opCodes & IPC_FRAG_HEADER)
						{
							headerLength = IPC_header_size + rsslSocketChannel->version->firstFragHdrLen + 1;
						}
						else if (opCodes & IPC_FRAG)
						{
							headerLength = IPC_header_size + rsslSocketChannel->version->subsequentFragHdrLen + 1;
						}
						else
						{
							headerLength = IPC_header_size + 1;
						}
					}
					else
						headerLength = IPC_header_size;

					/* If this is the first compressed message, store this queue in our list so we only compress with this queue */
					if (rsslSocketChannel->compressQueue == -1)
						rsslSocketChannel->compressQueue = msgb->priority;

					messageLength = (RsslUInt16)(msgb->length + headerLength);
					uncompBytes += messageLength;

					/* map priority from msgb to compressedmb */
					compressedmb1->priority = msgb->priority;

					flags |= IPC_COMP_DATA;

					/* this is going to be inside the compression */
					/* do not want it to have any http headers */
					tempLen = messageLength - httpHeaderLen;
					RTR_PUT_16(hdr, tempLen);
					hdr[2] = (char)flags;           /* hdr.IPC_opcode */

					if (flags & IPC_EXTENDED_FLAGS)
					{
						/* add fragmentation header */
						hdr[3] = (char)opCodes;

						if (opCodes & IPC_FRAG_HEADER)
						{
							/* add fragmentation header */
							/* add 32 bit length (size) */
							_move_u32_swap(hdr + 4, &size);

							if (rsslSocketChannel->version->connVersion > CONN_VERSION_12)
							{
								RsslUInt16 sFragId = (RsslUInt16)fragId;
								_move_u16_swap((hdr + 8), &sFragId);
							}
							else
								hdr[8] = (RsslUInt8)fragId;
						}
						else if (opCodes & IPC_FRAG)
						{
							if (rsslSocketChannel->version->connVersion > CONN_VERSION_12)
							{
								RsslUInt16 sFragId = (RsslUInt16)fragId;
								_move_u16_swap((hdr + 4), &sFragId);
							}
							else
								hdr[4] = (RsslUInt8)fragId;
						}
					}
					if (rsslSocketChannel->dbgFlags & RSSL_DEBUG_IPC_DUMP_OUT)
					{
						hdr[2] = (char)0;           /* hdr.IPC_opcode */
						(*ripcDumpOutFunc)(__FUNCTION__, msgb->buffer, messageLength, rsslSocketChannel->stream);
						hdr[2] = (char)flags;           /* hdr.IPC_opcode */
					}

					/* if doing a compression that does not grow across buffers, and it is over the threshold that will grow, use the intermediate buffer to compress into */
					if ((rsslSocketChannel->outCompression == RSSL_COMP_LZ4) && (tempLen >= rsslSocketChannel->upperCompressionThreshold))
					{
						compBuf.next_out = rsslSocketChannel->tempCompressBuf->buffer;
						compBuf.avail_out = (unsigned long)rsslSocketChannel->tempCompressBuf->maxLength;
					}
					else
					{
						compBuf.next_out = compressedmb1->buffer + headerLength;
						compBuf.avail_out = (unsigned long)(compressedmb1->maxLength - headerLength - footer_size - (*(rsslSocketChannel->protocolFuncs->additionalTransportHdrLength))());
					}
					/* comp inputs stay the same */
					compBuf.next_in = msgb->buffer + headerLength;
					compBuf.avail_in = messageLength - headerLength;

					if ((*(rsslSocketChannel->outCompFuncs->compress)) (rsslSocketChannel->c_stream_out, &compBuf, 0, error) < 0)
					{
						_rsslSetError(error, NULL, RSSL_RET_FAILURE, errno);

						/* Release the compressedbuffer here */
						rtr_dfltcFreeMsg(compressedmb1);

						retval = RSSL_RET_FAILURE;
						break;
					}

					compLen1 = compBuf.bytes_out_used;

					_DEBUG_TRACE_WRITE("#1 compressed %d bytes into %d bytes (avail_out = %d)\n", messageLength - headerLength, compLen1, compBuf.avail_out)


					/* if we have to split content, now do it */
					if ((rsslSocketChannel->outCompression == RSSL_COMP_LZ4) && (tempLen >= rsslSocketChannel->upperCompressionThreshold))
					{
						/* save length of content in the buffer */
						rsslSocketChannel->tempCompressBuf->length = compLen1;
						/* change compLen1 to what can fit in the buffer */
						compLen1 = (RsslUInt16)(compressedmb1->maxLength - headerLength - footer_size);
						/* if what we have is less than what will fit, shrink compLen 1 */
						if (rsslSocketChannel->tempCompressBuf->length <= compLen1)
						{
							compLen1 = (RsslUInt16)(rsslSocketChannel->tempCompressBuf->length);
							/* reset length for reuse,  since this doesnt span two buffers we do not need it */
							rsslSocketChannel->tempCompressBuf->length = 0;
						}
						else
						{
							/* content needs to span two buffers, leave length alone and fudge compBuf.avail_out */
							compBuf.avail_out = 0;
						}

						/* now copy what we can, take into account header and footer */
						MemCopyByInt((compressedmb1->buffer + headerLength), rsslSocketChannel->tempCompressBuf->buffer, compLen1);
						tempCompLen1 = compLen1;
					}

					if (compBuf.avail_out == 0)
					{
						compLen1 += headerLength - httpHeaderLen;

						/* add the chunking header if we are tunneling */
						if (rsslSocketChannel->httpHeaders)
						{
							/* we should have 6 bytes reserved at the beginning of the message for this */
							/* this should never consume more than 4 bytes - if it does we have	trouble */
							/* do not want this length to include the http header */
							chunkLen = sprintf(compressedmb1->buffer, "%x", (compLen1));
							/* add ; in any space as comment padding */
							while (chunkLen < 4)
								chunkLen += sprintf(compressedmb1->buffer + chunkLen, "%s", ";");
							/* add the carrage return line feed */
							IPC_ADD_CR_LF(compressedmb1->buffer, chunkLen);

							footeraddr = (caddr_t)((compressedmb1->buffer + compLen1 + httpHeaderLen));
							/* add the footer */
						}
						else
							chunkLen = 0;

						if (flags & IPC_EXTENDED_FLAGS)
						{

							_DEBUG_TRACE_WRITE("sending extended flags = 0x%x\n", opCodes)

							(compressedmb1->buffer)[2 + chunkLen] = (char)flags | IPC_COMP_FRAG;
							(compressedmb1->buffer)[3 + chunkLen] = hdr[3] = (char)opCodes;
							if (opCodes & IPC_FRAG_HEADER)
							{
								caddr_t sizePtr = &compressedmb1->buffer[4 + chunkLen];
								caddr_t fragIdPtr = &compressedmb1->buffer[8 + chunkLen];


								_DEBUG_TRACE_WRITE("sending first frag header (total msg size = %d)\n", size)

								/* Add fragmentation header */
								_move_u32_swap(sizePtr, &size);
								if (rsslSocketChannel->version->connVersion > CONN_VERSION_12)
								{
									RsslUInt16 sFragId = (RsslUInt16)fragId;
									_move_u16_swap(fragIdPtr, &sFragId);
								}
								else
									*fragIdPtr = hdr[8] = (RsslUInt8)fragId;
							}
							else if (opCodes & IPC_FRAG)
							{
								caddr_t fragIdPtr = &compressedmb1->buffer[4 + chunkLen];

								_DEBUG_TRACE_WRITE("sending subseq frag header\n")

								if (rsslSocketChannel->version->connVersion > CONN_VERSION_12)
								{
									RsslUInt16 sFragId = (RsslUInt16)fragId;
									_move_u16_swap(fragIdPtr, &sFragId);
								}
								else
									*fragIdPtr = hdr[4] = (RsslUInt8)fragId;
							}
						}
						else
							(compressedmb1->buffer)[2 + chunkLen] = (char)flags | IPC_COMP_FRAG;

						/* put on the chunking footer if we are tunneling */
						if (rsslSocketChannel->httpHeaders)
						{
							RsslUInt8 index = 0;
							IPC_ADD_CR_LF(footeraddr, index);
						}

						_DEBUG_TRACE_WRITE("ripc buffer len = %d\n", compLen1)

						_move_u16_swap(compressedmb1->buffer + chunkLen, &compLen1);
						compressedmb1->length = totalSize = (compLen1 + footer_size + httpHeaderLen);

						if ((rsslSocketChannel->dbgFlags & RSSL_DEBUG_IPC_DUMP_COMP) &&
							(rsslSocketChannel->dbgFlags & RSSL_DEBUG_IPC_DUMP_OUT))
							(*ripcDumpOutFunc)(__FUNCTION__, compressedmb1->buffer, (RsslUInt32)(compressedmb1->length), rsslSocketChannel->stream);

						/* Set/populate the prefix protocol header if one exists */
						(*(rsslSocketChannel->protocolFuncs->prependTransportHdr))((void*)rsslSocketChannel, compressedmb1, error);

						totalSize += compressedmb1->protocolHdrLength;

						compressedmb1->local = compressedmb1->buffer;
						compressedmb1->priority = msgb->priority;

						/* need to handle the forceFlush case for compression as well */
						if (forceFlush == 1)
						{
							RsslInt32	lenToWrite = 0;
							RsslInt32	cc = 0;
							ripcRWFlags	rwflags = 0;
							RsslInt32	queuedBytesToWrite = 0;
							RsslInt32	i = 0;

							rwflags |= (rsslSocketChannel->blocking ? RIPC_RW_BLOCKING : 0);

							if (compressedmb1->local == compressedmb1->buffer)
								lenToWrite = (RsslInt32)compressedmb1->length;
							else
								lenToWrite = (RsslInt32)(compressedmb1->length - ((caddr_t)compressedmb1->local - compressedmb1->buffer));

							/* the user wants to force a flush - check if the output buffers have any data */
							for (i = 0; i < RIPC_MAX_PRIORITY_QUEUE; i++)
							{
								if ((queuedBytesToWrite = rsslSocketChannel->priorityQueues[i].queueLength))
									break;
							}

							if (!queuedBytesToWrite)
							{
								/* pass the buffer directly to write and free the buffer */

								if (rsslSocketChannel->httpHeaders)
									cc = (*(rsslSocketChannel->transportFuncs->writeTransport))(rsslSocketChannel->tunnelTransportInfo, compressedmb1->local, lenToWrite, rwflags, error);
								else
									cc = (*(rsslSocketChannel->transportFuncs->writeTransport))(rsslSocketChannel->transportInfo, compressedmb1->local, lenToWrite, rwflags, error);

								/* if it failed flat out, see if we are tunneling and we should switch the FD */
								if ((cc < 0) && (rsslSocketChannel->newTunnelTransportInfo) && (rsslSocketChannel->httpHeaders))
								{
									rsslSocketChannel->tunnelTransportInfo = rsslSocketChannel->newTunnelTransportInfo;
									rsslSocketChannel->newTunnelTransportInfo = 0;
									/* now reattempt the write */
									cc = (*(rsslSocketChannel->transportFuncs->writeTransport))(rsslSocketChannel->tunnelTransportInfo, compressedmb1->local, lenToWrite, rwflags, error);
								}

								/* normal processing */
								if (cc == lenToWrite)
								{
									/* if write succeeds */
									rsslSocketChannel->bytesOutLastMsg += lenToWrite;

									if (compressedmb1->protocolHdr)
										compressedmb1->protocolHdr = 0;

									/* entire thing was written */
									rtr_dfltcFreeMsg(compressedmb1);
									compressedmb1->buffer = 0;
									compressedmb1->length = 0;

									/* need to switch stream IDs after sending the end of chunk message */
									if (rsslSocketChannel->newTunnelTransportInfo && rsslSocketChannel->httpHeaders  && rsslSocketChannel->sentControlAck)
									{
										RsslInt32 outLen;
										RsslInt32 writtenLen = 0;
										char outBuf[255];

										rsslSocketChannel->sentControlAck = 0;
										outLen = sprintf(outBuf, "%x\r\n\r\n", 0);
										/* we may need to send this footer instead of just the second \r\n */
										//outLen += snprintf(outBuf + outLen,(255-outLen), "%s", "Connection: close\r\n");

										writtenLen = (*(rsslSocketChannel->transportFuncs->writeTransport))(rsslSocketChannel->tunnelTransportInfo, outBuf, outLen, rwflags, error);

										/* if it doesnt write the entire thing or it returns a neg value we are in a bad state */
										if (writtenLen == outLen)
										{
											/* Close the old FD here, if present */
											if ((rsslSocketChannel->oldTunnelStreamFd) &&
												((rsslSocketChannel->connType == RSSL_CONN_TYPE_ENCRYPTED) ?
												(rsslSocketChannel->oldTunnelStreamFd != ((ripcSSLSession*)rsslSocketChannel->newTunnelTransportInfo)->socket) :
												(rsslSocketChannel->oldTunnelStreamFd != (RsslSocket)(intptr_t)(rsslSocketChannel->newTunnelTransportInfo))))
											{
												sock_close((RsslSocket)(intptr_t)rsslSocketChannel->oldTunnelStreamFd);
												rsslSocketChannel->oldTunnelStreamFd = RIPC_INVALID_SOCKET;
											}

											/* success - switch file descriptors */
											/* Only store the old FD when this is a WinInet-based tunnel.
											* Otherwise the shutdown() call will have already closed the socket properly. */
											if (rsslSocketChannel->connType == RSSL_CONN_TYPE_ENCRYPTED)
											{
												if (!rsslSocketChannel->isJavaTunnel)
												{
													rsslSocketChannel->oldTunnelStreamFd = (RsslSocket)(((ripcSSLSession*)rsslSocketChannel->tunnelTransportInfo)->socket);
												}
												ripcShutdownSSLSocket(rsslSocketChannel->tunnelTransportInfo);
											}
											else
											{
												if (!rsslSocketChannel->isJavaTunnel)
												{
													rsslSocketChannel->oldTunnelStreamFd = (RsslSocket)(intptr_t)(rsslSocketChannel->tunnelTransportInfo);
												}
												shutdown((RsslSocket)(intptr_t)(rsslSocketChannel->tunnelTransportInfo), shutdownFlag);
											}

											rsslSocketChannel->tunnelTransportInfo = rsslSocketChannel->newTunnelTransportInfo;
											rsslSocketChannel->newTunnelTransportInfo = 0;
										}
										else if (writtenLen != 0)
										{
											/* this means we didnt write entire chunked footer, but we wrote some of it */
											_rsslSetError(error, NULL, RSSL_RET_FAILURE, errno);
											snprintf(error->text, MAX_RSSL_ERROR_TEXT,
												"<%s:%d> Error: 1007 ipcWriteSession() Error writing chunk footer(%d)\n",
												__FILE__, __LINE__, writtenLen);

											retval = RSSL_RET_FAILURE;
											break;
										}
									}
								}
								else if (cc < 0)
								{
									/* if write fails, we need to queue the buffer up like normal,
									* but try to call flush when we finish write */
									forceFlush = 2;
								}
								else
								{
									/* cc is somewhere between nothing and our size */
									/* update the local pointer, and put buffer in queue */
									compressedmb1->local = (caddr_t)compressedmb1->local + cc;
									forceFlush = 2;
								}
							}
							else
							{
								/* there is something to flush out of the queue */
								forceFlush = 2;
							}
						}

						if ((forceFlush == 0) || (forceFlush == 2))
						{
							/* put it in the correct priority out list */
							rsslSocketChannel->priorityQueues[compressedmb1->priority].queueLength += totalSize;
							rsslQueueAddLinkToBack(&(rsslSocketChannel->priorityQueues[compressedmb1->priority].priorityQueue), &(compressedmb1->link));
						}

						IPC_MUTEX_UNLOCK(rsslSocketChannel);

						rsslSocketChannel->bytesOutLastMsg += (RsslUInt32)(compressedmb1->length);	/* need to add this in so rsslWrite returns the right number of bytes written */
						compressedmb2 = ipcDataBuffer(rsslSocketChannel, rsslSocketChannel->maxUserMsgSize, error);

						IPC_MUTEX_LOCK(rsslSocketChannel);

						/* if we cant get this one, this is catastrophic - we need to get this data buffer to finish compressing */
						if (compressedmb2 == 0)
						{
							_rsslSetError(error, NULL, RSSL_RET_FAILURE, errno);
							snprintf(error->text, MAX_RSSL_ERROR_TEXT,
								"<%s:%d> Error: 1007 ipcWriteSession() Compress Failed.  No buffers available to compress into.\n",
								__FILE__, __LINE__);

							retval = RSSL_RET_FAILURE;
							break;
						}

						/* map priority */
						compressedmb2->priority = msgb->priority;

						/* Just used standard header on any fragments off of a compressed buffer */
						headerLength = IPC_header_size;


						if (rsslSocketChannel->outCompression == RSSL_COMP_LZ4) /* we should not be here with LZ4 unless it was over the comp threshold && (tempLen >= sess->upperCompressionThreshold) */
						{
							/* compression was already done above, just need to continue copy */
							/* compLen2 should be whatever is left that we didnt copy into the buffer */
							compLen2 = (RsslUInt16)(rsslSocketChannel->tempCompressBuf->length - tempCompLen1);  /* because compLen1 gets changed, we needed to store what it was before the changes */
							MemCopyByInt((compressedmb2->buffer + headerLength), (rsslSocketChannel->tempCompressBuf->buffer + tempCompLen1), compLen2);
							rsslSocketChannel->tempCompressBuf->length = 0;
						}
						else
						{
							/* have to continue compressing */
							compBuf.next_out = compressedmb2->buffer + headerLength;
							compBuf.avail_out = (unsigned long)(compressedmb2->maxLength - headerLength - footer_size - (*(rsslSocketChannel->protocolFuncs->additionalTransportHdrLength))());

							if ((*(rsslSocketChannel->outCompFuncs->compress)) (rsslSocketChannel->c_stream_out, &compBuf, 0, error) < 0)
							{
								_rsslSetError(error, NULL, RSSL_RET_FAILURE, errno);

								/* free compressedmb2 here */
								rtr_dfltcFreeMsg(compressedmb2);
								retval = RSSL_RET_FAILURE;
								break;
							}

							compLen2 = compBuf.bytes_out_used;
						}

						_DEBUG_TRACE_WRITE("#2 compressed %d bytes into %d bytes (avail_out = %d)\n",
							messageLength - headerLength, compBuf.bytes_out_used, compBuf.avail_out)


						compLen2 += headerLength - httpHeaderLen;

						/* add the chunking header if we are tunneling */
						if (rsslSocketChannel->httpHeaders)
						{
							/* we should have 6 bytes reserved at the beginning of the message for this */
							/* this should never consume more than 4 bytes - if it does we have	trouble */
							/* do not want this length to include the http header */
							chunkLen = sprintf(compressedmb2->buffer, "%x", (compLen2));
							/* add ; in any space as comment padding */
							while (chunkLen < 4)
								chunkLen += sprintf(compressedmb2->buffer + chunkLen, "%s", ";");
							/* add the carrage return line feed */
							IPC_ADD_CR_LF(compressedmb2->buffer, chunkLen);

							footeraddr = (caddr_t)((compressedmb2->buffer + compLen2 + httpHeaderLen));
							/* add the footer */
						}
						else
							chunkLen = 0;

						_move_u16_swap((compressedmb2->buffer + chunkLen), &compLen2);

						(compressedmb2->buffer)[2 + chunkLen] = IPC_COMP_DATA;

						/* put on the chunking footer if we are tunneling */
						if (rsslSocketChannel->httpHeaders)
						{
							RsslUInt8 index = 0;
							IPC_ADD_CR_LF(footeraddr, index);
						}
						uncompBytes += headerLength;
						compressedmb2->length = totalSize = (compLen2 + footer_size + httpHeaderLen);
						compressedmb2->priority = msgb->priority;
						if ((rsslSocketChannel->dbgFlags & RSSL_DEBUG_IPC_DUMP_COMP) &&
							(rsslSocketChannel->dbgFlags & RSSL_DEBUG_IPC_DUMP_OUT))
							(*ripcDumpOutFunc)(__FUNCTION__, compressedmb2->buffer, (RsslUInt32)(compressedmb2->length), rsslSocketChannel->stream);
						rtr_dfltcFreeMsg(msgb);
						msgb = compressedmb2;
					}
					else
					{
						/* non compression fragmentation case (e.g. single compressed buffer) */
						compLen1 += headerLength - httpHeaderLen;

						/* add the chunking header if we are tunneling */
						if (rsslSocketChannel->httpHeaders)
						{
							/* we should have 6 bytes reserved at the beginning of the message for this */
							/* this should never consume more than 4 bytes - if it does we have	trouble */
							/* do not want this length to include the http header */
							chunkLen = sprintf(compressedmb1->buffer, "%x", (compLen1));
							/* add ; in any space as comment padding */
							while (chunkLen < 4)
								chunkLen += sprintf(compressedmb1->buffer + chunkLen, "%s", ";");
							/* add the carrage return line feed */
							IPC_ADD_CR_LF(compressedmb1->buffer, chunkLen);

							footeraddr = (caddr_t)((compressedmb1->buffer + compLen1 + httpHeaderLen));
							/* add the footer */
						}
						else
							chunkLen = 0;

						flags |= IPC_COMP_DATA;
						(compressedmb1->buffer)[2 + chunkLen] = hdr[2];

						/* end */
						/* add the frag header */
						if (flags & IPC_EXTENDED_FLAGS)
						{
							(compressedmb1->buffer)[3 + chunkLen] = hdr[3];

							if (hdr[3] & IPC_FRAG_HEADER)
							{
								/* move 32 bit length here (size) */
								_move_u32_swap((compressedmb1->buffer + 4 + chunkLen), &size);

								if (rsslSocketChannel->version->connVersion > CONN_VERSION_12)
								{
									RsslUInt16 sFragId = (RsslUInt16)fragId;
									_move_u16_swap((compressedmb1->buffer + 8 + chunkLen), &sFragId);
								}
								else
									(compressedmb1->buffer)[8 + chunkLen] = (RsslUInt8)fragId;
							}
							else if (hdr[3] & IPC_FRAG)
							{
								if (rsslSocketChannel->version->connVersion > CONN_VERSION_12)
								{
									RsslUInt16 sFragId = (RsslUInt16)fragId;
									_move_u16_swap((compressedmb1->buffer + 4 + chunkLen), &sFragId);
								}
								else
									(compressedmb1->buffer)[4 + chunkLen] = (RsslUInt8)fragId;
							}
						}

						_move_u16_swap((compressedmb1->buffer + chunkLen), &compLen1);

						/* put on the chunking footer if we are tunneling */
						if (rsslSocketChannel->httpHeaders)
						{
							RsslUInt8 index = 0;
							IPC_ADD_CR_LF(footeraddr, index);
						}

						compressedmb1->length = totalSize = (compLen1 + footer_size + httpHeaderLen);
						if ((rsslSocketChannel->dbgFlags & RSSL_DEBUG_IPC_DUMP_COMP) &&
							(rsslSocketChannel->dbgFlags & RSSL_DEBUG_IPC_DUMP_OUT))
							(*ripcDumpOutFunc)(__FUNCTION__, compressedmb1->buffer, (RsslUInt32)(compressedmb1->length), rsslSocketChannel->stream);
						rtr_dfltcFreeMsg(msgb);
						msgb = compressedmb1;
					}
				}
				break;

			default:
				_rsslSetError(error, NULL, RSSL_RET_FAILURE, errno);
				snprintf(error->text, MAX_RSSL_ERROR_TEXT,
					"<%s:%d> Error: 1007 Unknown connection version: 0x%x.\n",
					__FILE__, __LINE__, rsslSocketChannel->version->connVersion);
				retval = RSSL_RET_FAILURE;
				break;
			}

			/* Set/populate the prefix protocol header if one exists */
			(*(rsslSocketChannel->protocolFuncs->prependTransportHdr))((void*)rsslSocketChannel, msgb, error);

			totalSize += msgb->protocolHdrLength;
			uncompBytes += msgb->protocolHdrLength;

			msgb->local = msgb->buffer;

			if (forceFlush == 1)
			{
				RsslInt32 lenToWrite = 0;
				RsslInt32 cc = 0;
				RsslInt32 i = 0;
				RsslInt32 queuedBytesToWrite = 0;
				ripcRWFlags			rwflags = 0;

				rwflags |= (rsslSocketChannel->blocking ? RIPC_RW_BLOCKING : 0);

				if (msgb->local == msgb->buffer)
					lenToWrite = (RsslInt32)(msgb->length);
				else
					lenToWrite = (RsslInt32)(msgb->length - ((caddr_t)msgb->local - msgb->buffer));

				/* the user wants to force a flush - check if the output buffers have any data */
				for (i = 0; i < RIPC_MAX_PRIORITY_QUEUE; i++)
				{
					if ((queuedBytesToWrite = rsslSocketChannel->priorityQueues[i].queueLength))
						break;
				}

				if (!queuedBytesToWrite)
				{
					/* pass the buffer directly to write and free the buffer */
					if (rsslSocketChannel->httpHeaders)
						cc = (*(rsslSocketChannel->transportFuncs->writeTransport))(rsslSocketChannel->tunnelTransportInfo, msgb->local, lenToWrite, rwflags, error);
					else
						cc = (*(rsslSocketChannel->transportFuncs->writeTransport))(rsslSocketChannel->transportInfo, msgb->local, lenToWrite, rwflags, error);


					/* if it failed flat out, see if we are tunneling and we should switch the FD */
					if ((cc < 0) && (rsslSocketChannel->newTunnelTransportInfo) && (rsslSocketChannel->httpHeaders))
					{
						rsslSocketChannel->tunnelTransportInfo = rsslSocketChannel->newTunnelTransportInfo;
						rsslSocketChannel->newTunnelTransportInfo = 0;
						/* now reattempt the write */
						cc = (*(rsslSocketChannel->transportFuncs->writeTransport))(rsslSocketChannel->tunnelTransportInfo, msgb->local, lenToWrite, rwflags, error);
					}

					/* normal processing */
					if (cc == lenToWrite)
					{
						/* if write succeeds */
						rsslSocketChannel->bytesOutLastMsg += lenToWrite;

						/* entire thing was written */
						rtr_dfltcFreeMsg(msgb);
						msgb->buffer = 0;
						msgb->length = 0;


						/* need to switch stream IDs after sending the end of chunk message */
						if (rsslSocketChannel->newTunnelTransportInfo && rsslSocketChannel->httpHeaders  && rsslSocketChannel->sentControlAck)
						{
							RsslInt32 outLen;
							RsslInt32 writtenLen = 0;
							char outBuf[255];

							rsslSocketChannel->sentControlAck = 0;
							outLen = sprintf(outBuf, "%x\r\n\r\n", 0);

							writtenLen = (*(rsslSocketChannel->transportFuncs->writeTransport))(rsslSocketChannel->tunnelTransportInfo, outBuf, outLen, rwflags, error);

							/* if it doesnt write the entire thing or it returns a neg value we are in a bad state */
							if (writtenLen == outLen)
							{
								/* Close the old FD here, if present */
								if ((rsslSocketChannel->oldTunnelStreamFd) &&
									(rsslSocketChannel->connType == RSSL_CONN_TYPE_ENCRYPTED) ?
										(rsslSocketChannel->oldTunnelStreamFd != ((ripcSSLSession*)rsslSocketChannel->newTunnelTransportInfo)->socket) :
										(rsslSocketChannel->oldTunnelStreamFd != (RsslSocket)(intptr_t)(rsslSocketChannel->newTunnelTransportInfo)))
								{
									sock_close((RsslSocket)(intptr_t)rsslSocketChannel->oldTunnelStreamFd);
									rsslSocketChannel->oldTunnelStreamFd = RIPC_INVALID_SOCKET;
								}

								/* success - switch file descriptors */
								/* Only store the old FD when this is a WinInet-based tunnel.
								* Otherwise the shutdown() call will have already closed the socket properly. */
								if (rsslSocketChannel->connType == RSSL_CONN_TYPE_ENCRYPTED)
								{
									if (!rsslSocketChannel->isJavaTunnel)
									{
										rsslSocketChannel->oldTunnelStreamFd = (RsslSocket)(((ripcSSLSession*)rsslSocketChannel->tunnelTransportInfo)->socket);
									}
									ripcShutdownSSLSocket(rsslSocketChannel->tunnelTransportInfo);
								}
								else
								{
									if (!rsslSocketChannel->isJavaTunnel)
									{
										rsslSocketChannel->oldTunnelStreamFd = (RsslSocket)(intptr_t)(rsslSocketChannel->tunnelTransportInfo);
									}
									shutdown((RsslSocket)(intptr_t)(rsslSocketChannel->tunnelTransportInfo), shutdownFlag);
								}

								rsslSocketChannel->tunnelTransportInfo = rsslSocketChannel->newTunnelTransportInfo;
								rsslSocketChannel->newTunnelTransportInfo = 0;
							}
							else if (writtenLen != 0)
							{
								/* this means we didnt write entire chunked footer, but we wrote some of it */
								_rsslSetError(error, NULL, RSSL_RET_FAILURE, errno);
								snprintf(error->text, MAX_RSSL_ERROR_TEXT,
									"<%s:%d> Error:1007 ipcWriteSession() Error writing chunk footer(%d).\n",
									__FILE__, __LINE__, writtenLen);

								retval = RSSL_RET_FAILURE;
								break;
							}
						}
					}
					else if (cc < 0)
					{
						/* if write fails, we need to queue the buffer up like normal,
						* but try to call flush when we finish write */
						forceFlush = 2;
					}
					else
					{
						/* cc is somewhere between nothing and our size */
						/* update the local pointer, and put buffer in queue */
						msgb->local = (caddr_t)msgb->local + cc;
						rsslSocketChannel->nextOutBuf = msgb->priority;
						forceFlush = 2;
					}
				}
				else
				{
					/* there is something to flush out of the queue */
					forceFlush = 2;
				}
			}

			if ((forceFlush == 0) || (forceFlush == 2))
			{
				/* figure out which priority list to put this in */
				rsslSocketChannel->priorityQueues[msgb->priority].queueLength += totalSize;

				pLink = rsslQueuePeekBack(&rsslSocketChannel->priorityQueues[msgb->priority].priorityQueue);

				/* since there are dblks - actual memory block, and mblks - pointers
				into dblks, it is possible that there are several mblks sharing
				the same dblk.  If this is the case, there is possibly a pointer
				to earlier in this dblk in this output queue.  This checks for that.
				If it is the case, we update the length of the block (to avoid adding
				another pointer to the same block).  If its not the
				case, we put the block in the list */
				if (pLink && (lastmb = RSSL_QUEUE_LINK_TO_OBJECT(rtr_msgb_t, link, pLink)) && ((lastmb->buffer + lastmb->length) == msgb->buffer))
				{
					lastmb->length += msgb->length;
					lastmb->maxLength += msgb->length;

					rsslSocketChannel->bytesOutLastMsg += (RsslUInt32)(msgb->length);

					if (msgb->protocolHdr)
						msgb->protocolHdr = 0;

					rtr_dfltcFreeMsg(msgb);
					msgb->buffer = 0;
					msgb->length = 0;
				}
				else
				{
					/* figure out which priority list to put this in */

					_DEBUG_TRACE_WRITE("#2 Queuing %d bytes (forceFlush=%d, queueLength=%d)\n", totalSize, forceFlush, rsslSocketChannel->priorityQueues[msgb->priority].queueLength)


					rsslQueueAddLinkToBack(&(rsslSocketChannel->priorityQueues[msgb->priority].priorityQueue), &(msgb->link));
					rsslSocketChannel->bytesOutLastMsg += (RsslUInt32)(msgb->length);
				}
			}

		} /* if(buf->length > 0) */
		else
		{/* No bytes were written by the user */
			if (flags & IPC_EXTENDED_FLAGS)
			{
				if (opCodes & IPC_FRAG_HEADER)
				{
					msgb->buffer -= (IPC_header_size + rsslSocketChannel->version->firstFragHdrLen + 1);
					msgb->length += (IPC_header_size + rsslSocketChannel->version->firstFragHdrLen + 1 + footer_size);
				}
				else if (opCodes & IPC_FRAG)
				{
					msgb->buffer -= (IPC_header_size + rsslSocketChannel->version->subsequentFragHdrLen + 1);
					msgb->length += (IPC_header_size + rsslSocketChannel->version->subsequentFragHdrLen + 1 + footer_size);
				}
				else
				{
					msgb->buffer -= (IPC_header_size + 1);
					msgb->length += (IPC_header_size + 1 + footer_size);
				}
			}
			else
			{
				msgb->buffer -= IPC_header_size;
				msgb->length += (IPC_header_size + footer_size);
			}

			if (msgb->protocolHdr)
				msgb->protocolHdr = 0;

			rtr_dfltcFreeMsg(msgb);
		}

		*bytesWritten = rsslSocketChannel->bytesOutLastMsg;
		*uncompBytesWritten = uncompBytes;

		msgb = nmb;
		/* reset flags */
		if (flags & IPC_EXTENDED_FLAGS)
		{
			flags = 0x0;
			flags |= IPC_EXTENDED_FLAGS;
		}
		else
			flags = 0x0;

		if ((opCodes & IPC_FRAG_HEADER) || (opCodes & IPC_FRAG))
		{
			opCodes = 0x0;
			opCodes |= IPC_FRAG;
		}
		else
			opCodes = 0x0;
	}

	/* If this is a chained message and the full value has not been queued*/
	rsslBufferImpl->bufferInfo = (void*)msgb;

	if (retval != RSSL_RET_FAILURE)
	{
		for (i = 0; i < RIPC_MAX_PRIORITY_QUEUE; i++)
			retval += rsslSocketChannel->priorityQueues[i].queueLength;

		if ((forceFlush == RSSL_WRITE_DIRECT_SOCKET_WRITE) || (retval >(RsslInt32)rsslSocketChannel->high_water_mark))
		{
			retval = ipcFlushSession(rsslSocketChannel, error);
		}
	}

	IPC_MUTEX_UNLOCK(rsslSocketChannel);

	return(retval);
}

RsslRet ipcFlushSession(RsslSocketChannel *rsslSocketChannel, RsslError *error)
{
	rtr_msgb_t			*curmsgb = 0;
	RsslInt32			lenToWrite;
	RsslInt32			cc = 0;
	ripcRWFlags			rwflags = 0;
	ripcIovType			wrtvec[RIPC_MAXIOVLEN];
	RsslInt32			iovPriority[RIPC_MAXIOVLEN + 1];
	RsslInt32			wrtveclen = 0;
	RsslInt32			cont = 1;
	RsslInt32			tempOutList = 0;
	RsslInt32			i = 0;
	RsslRet				retVal = RSSL_RET_SUCCESS;
	RsslInt32			iovLength = RIPC_MAXIOVLEN;
	RsslInt32			reducedIovLen = 0;
	RsslQueueLink		*pLink = 0;

	if (rsslSocketChannel->workState & RIPC_INT_SHTDOWN_PEND)
	{
		_rsslSetError(error, NULL, RSSL_RET_FAILURE, errno);
		snprintf(error->text, MAX_RSSL_ERROR_TEXT,
			"<%s:%d> Error: 1003 ipcFlushSession() failed due to channel shutting down.\n",
			__FILE__, __LINE__);

		return RSSL_RET_FAILURE;
	}

	rwflags |= (rsslSocketChannel->blocking ? RIPC_RW_BLOCKING : 0);

	/* keep track of this so we know if we went though the entire out list */
	tempOutList = rsslSocketChannel->currentOutList;

	iovPriority[0] = -1;
	for (i = 0; i < RIPC_MAX_PRIORITY_QUEUE; i++)
		rsslSocketChannel->priorityQueues[i].tempList[0] = 0;

	/* If there is no writev, then write one at a time. */
	if (rsslSocketChannel->transportFuncs->writeVTransport == 0) {
		while (cont)
		{
			lenToWrite = 0;

			/* see which queue the flush strategy says to flush */
			/* while we have a buffer, or we do not have a buffer and have gone through
			the entire flush strategy we want to exit the while */

			/* this should get a msgb from the output queue that should be flushed */
			while (!curmsgb)
			{
				pLink = rsslQueuePeekFront(&rsslSocketChannel->priorityQueues[rsslSocketChannel->flushStrategy[rsslSocketChannel->currentOutList]].priorityQueue);
				rsslSocketChannel->currentOutList++;

				if ((rsslSocketChannel->flushStrategy[rsslSocketChannel->currentOutList] == -1) ||
					(rsslSocketChannel->currentOutList >= RIPC_MAX_FLUSH_STRATEGY))
					rsslSocketChannel->currentOutList = 0;

				if (pLink)
					curmsgb = RSSL_QUEUE_LINK_TO_OBJECT(rtr_msgb_t, link, pLink);

				if ((rsslSocketChannel->currentOutList == tempOutList) && (!curmsgb))
					break;
			}

			/* since we exited the while loop either we have a buffer, or
			we do not have a buffer and we have gone through the strategy */
			/* since we do not require having 'L' in the strategy, we should check
			if we went through the entire strategy - if we did and we do not have
			a buffer, try to grab a buffer from the low queue
			*/
			if (!curmsgb)
			{
				/* this needs to change if we add more than the high, med, and low queues */
				pLink = rsslQueuePeekFront(&rsslSocketChannel->priorityQueues[2].priorityQueue);
				if(pLink)
					curmsgb = RSSL_QUEUE_LINK_TO_OBJECT(rtr_msgb_t, link, pLink);

			}
			else
			{
				/* we got a buffer - update the tempOutList pointer */
				tempOutList = rsslSocketChannel->currentOutList;
			}

			if (!curmsgb)
			{
				/* if we still do not have a buffer here, we have nothing to write */
				/* if we are tunneling, we may have an fd change that we need to perform */
				/* need to switch stream IDs after sending the end of chunk message */
				if (rsslSocketChannel->newTunnelTransportInfo && rsslSocketChannel->httpHeaders  && rsslSocketChannel->sentControlAck)
				{
					RsslInt32 outLen;
					RsslInt32 writtenLen = 0;
					char outBuf[255];

					rsslSocketChannel->sentControlAck = 0;
					outLen = sprintf(outBuf, "%x\r\n\r\n", 0);

					writtenLen = (*(rsslSocketChannel->transportFuncs->writeTransport))(rsslSocketChannel->tunnelTransportInfo, outBuf, outLen, rwflags, error);

					/* if it doesnt write the entire thing or it returns a neg value we are in a bad state */
					if (writtenLen == outLen)
					{
						/* Close the old FD here, if present */
						if ((rsslSocketChannel->oldTunnelStreamFd) &&
							((rsslSocketChannel->connType == RSSL_CONN_TYPE_ENCRYPTED) ?
							(rsslSocketChannel->oldTunnelStreamFd != ((ripcSSLSession*)rsslSocketChannel->newTunnelTransportInfo)->socket) :
								(rsslSocketChannel->oldTunnelStreamFd != (RsslSocket)(intptr_t)(rsslSocketChannel->newTunnelTransportInfo))))
						{
							sock_close((RsslSocket)(intptr_t)rsslSocketChannel->oldTunnelStreamFd);
							rsslSocketChannel->oldTunnelStreamFd = RIPC_INVALID_SOCKET;
						}

						/* success - switch file descriptors */
						/* Only store the old FD when this is a WinInet-based tunnel.
						* Otherwise the shutdown() call will have already closed the socket properly. */
						if (rsslSocketChannel->connType == RSSL_CONN_TYPE_ENCRYPTED)
						{
							if (!rsslSocketChannel->isJavaTunnel)
							{
								rsslSocketChannel->oldTunnelStreamFd = (RsslSocket)(((ripcSSLSession*)rsslSocketChannel->tunnelTransportInfo)->socket);
							}
							ripcShutdownSSLSocket(rsslSocketChannel->tunnelTransportInfo);
						}
						else
						{
							if (!rsslSocketChannel->isJavaTunnel)
							{
								rsslSocketChannel->oldTunnelStreamFd = (RsslSocket)(intptr_t)(rsslSocketChannel->tunnelTransportInfo);
							}
							shutdown((RsslSocket)(intptr_t)(rsslSocketChannel->tunnelTransportInfo), shutdownFlag);
						}

						rsslSocketChannel->tunnelTransportInfo = rsslSocketChannel->newTunnelTransportInfo;
						rsslSocketChannel->newTunnelTransportInfo = 0;
					}
					else if (writtenLen != 0)
					{
						/* this means we didnt write entire chunked footer, but we wrote some of it */
						_rsslSetError(error, NULL, RSSL_RET_FAILURE, errno);
						snprintf(error->text, MAX_RSSL_ERROR_TEXT,
							"<%s:%d> Error: 1007 ipcFlushSession() Error writing chunk footer(%d).\n",
							__FILE__, __LINE__, writtenLen);

						return RSSL_RET_FAILURE;
					}
				}
				cont = 0; /* we didnt find a buffer to write, so we are done */
			}

			/* if we found a buffer with data, this loop will write it out */
			while (curmsgb)
			{
				if (curmsgb->local == curmsgb->buffer)
					lenToWrite = (RsslInt32)(curmsgb->length);
				else
					lenToWrite = (RsslInt32)(curmsgb->length - ((caddr_t)curmsgb->local - curmsgb->buffer));

				if (rsslSocketChannel->httpHeaders)
					cc = (*(rsslSocketChannel->transportFuncs->writeTransport))(rsslSocketChannel->tunnelTransportInfo, curmsgb->local, lenToWrite, rwflags, error);
				else
					cc = (*(rsslSocketChannel->transportFuncs->writeTransport))(rsslSocketChannel->transportInfo, curmsgb->local, lenToWrite, rwflags, error);


				if (cc < 0)
				{
					/* there was an error on write - this could be because we didnt send anything and a proxy killed our connection if we are tunneling */
					/* flip the fd's and write this data again */
					if (rsslSocketChannel->newTunnelTransportInfo && rsslSocketChannel->httpHeaders)
					{
						rsslSocketChannel->tunnelTransportInfo = rsslSocketChannel->newTunnelTransportInfo;
						rsslSocketChannel->newTunnelTransportInfo = 0;

						cc = (*(rsslSocketChannel->transportFuncs->writeTransport))(rsslSocketChannel->tunnelTransportInfo, curmsgb->local, lenToWrite, rwflags, error);
						/* if it didnt work this time, end the connection */
						if (cc < 0)
						{
							_rsslSetError(error, NULL, RSSL_RET_FAILURE, errno);
							snprintf(error->text, MAX_RSSL_ERROR_TEXT,
								"<%s:%d> Error: 1002 ipcWrite() failed. System errno: (%d)\n",
								__FILE__, __LINE__, errno);

							return RSSL_RET_FAILURE;
						}
					}
					else
					{
						_rsslSetError(error, NULL, RSSL_RET_FAILURE, errno);
						snprintf(error->text, MAX_RSSL_ERROR_TEXT,
							"<%s:%d> Error: 1002 ipcWrite() failed. System errno: (%d)\n",
							__FILE__, __LINE__, errno);

						return RSSL_RET_FAILURE;
					}
				}

				/* now update the particular output buffers length */
				rsslSocketChannel->priorityQueues[curmsgb->priority].queueLength -= cc;

				if (cc == lenToWrite) /* check if we wrote out everything we expected to */
				{
					/* now actually remove the buffer from the queue */
					/* and free it */
					pLink = rsslQueueRemoveFirstLink(&rsslSocketChannel->priorityQueues[curmsgb->priority].priorityQueue);
					if (pLink)
						curmsgb = RSSL_QUEUE_LINK_TO_OBJECT(rtr_msgb_t, link, pLink);
					else
						curmsgb = 0;

					RIPC_ASSERT(curmsgb);
					rtr_dfltcFreeMsg(curmsgb);
					curmsgb = 0;

					/* need to switch stream IDs after sending the end of chunk message */
					if (rsslSocketChannel->newTunnelTransportInfo && rsslSocketChannel->httpHeaders  && rsslSocketChannel->sentControlAck)
					{
						RsslInt32 outLen;
						RsslInt32 writtenLen = 0;
						char outBuf[255];

						rsslSocketChannel->sentControlAck = 0;
						outLen = sprintf(outBuf, "%x\r\n\r\n", 0);

						writtenLen = (*(rsslSocketChannel->transportFuncs->writeTransport))(rsslSocketChannel->tunnelTransportInfo, outBuf, outLen, rwflags, error);

						/* if it doesnt write the entire thing or it returns a neg value we are in a bad state */
						if (writtenLen == outLen)
						{
							/* Close the old FD here, if present */
							if ((rsslSocketChannel->oldTunnelStreamFd) &&
								((rsslSocketChannel->connType == RSSL_CONN_TYPE_ENCRYPTED) ?
								(rsslSocketChannel->oldTunnelStreamFd != ((ripcSSLSession*)rsslSocketChannel->newTunnelTransportInfo)->socket) :
									(rsslSocketChannel->oldTunnelStreamFd != (RsslSocket)(intptr_t)(rsslSocketChannel->newTunnelTransportInfo))))
							{
								sock_close((RsslSocket)(intptr_t)rsslSocketChannel->oldTunnelStreamFd);
								rsslSocketChannel->oldTunnelStreamFd = RIPC_INVALID_SOCKET;
							}

							/* success - switch file descriptors */
							/* Only store the old FD when this is a WinInet-based tunnel.
							* Otherwise the shutdown() call will have already closed the socket properly. */
							if (rsslSocketChannel->connType == RSSL_CONN_TYPE_ENCRYPTED)
							{
								if (!rsslSocketChannel->isJavaTunnel)
								{
									rsslSocketChannel->oldTunnelStreamFd = (RsslSocket)(((ripcSSLSession*)rsslSocketChannel->tunnelTransportInfo)->socket);
								}
								ripcShutdownSSLSocket(rsslSocketChannel->tunnelTransportInfo);
							}
							else
							{
								if (!rsslSocketChannel->isJavaTunnel)
								{
									rsslSocketChannel->oldTunnelStreamFd = (RsslSocket)(intptr_t)(rsslSocketChannel->tunnelTransportInfo);
								}
								shutdown((RsslSocket)(intptr_t)(rsslSocketChannel->tunnelTransportInfo), shutdownFlag);
							}

							rsslSocketChannel->tunnelTransportInfo = rsslSocketChannel->newTunnelTransportInfo;
							rsslSocketChannel->newTunnelTransportInfo = 0;
						}
						else if (writtenLen != 0)
						{
							/* this means we didnt write entire chunked footer, but we wrote some of it */
							_rsslSetError(error, NULL, RSSL_RET_FAILURE, errno);
							snprintf(error->text, MAX_RSSL_ERROR_TEXT,
								"<%s:%d> Error: 1007 ipcFlushSession() Error writing chunk footer(%d)\n",
								__FILE__, __LINE__, writtenLen);

							return RSSL_RET_FAILURE;
						}
					}
				}
				else /* we did a partial write */
				{
					curmsgb->local = (caddr_t)curmsgb->local + cc;	/* advance the buffer pointer past the stuff we wrote out */
				}
				/* We only try to write one buffer in non-blocking mode
				if we decide to attempt to loop and write several buffers, this test should
				move into the above else statement where we couldnt write the entire buffer */
				if (rsslSocketChannel->blocking == 0) /* if this is a non-blocking write, then do not try to write out more from this buffer */
					curmsgb = 0;
			}

			if ((rsslSocketChannel->blocking) == 0) /* if this is a non-blocking write, then do not try to write more than one buffer */
				cont = 0;
		}
		for (i = 0; i < RIPC_MAX_PRIORITY_QUEUE; i++)
			retVal += rsslSocketChannel->priorityQueues[i].queueLength;

		return(retVal);
	}

	/* this is where we do a vectored write */
	while (cont)
	{
		lenToWrite = 0;
		wrtveclen = 0;
		for (i = 0; i < RIPC_MAX_PRIORITY_QUEUE; i++)
			rsslSocketChannel->priorityQueues[i].tempIndex = -1;

		/* if we are doing http tunneling and require headers, limit this to one buffer and then write end of the chunk */
		/* there really shouldnt be a case where we have a new Tunnel stream fd and not using http headers */
		if (rsslSocketChannel->newTunnelTransportInfo && rsslSocketChannel->httpHeaders  && rsslSocketChannel->sentControlAck) {
			reducedIovLen = 1;
			iovLength = 1;
		}

		/* get the proper buffer */
		/* if we couldnt write the entire vector, and we had to stop in the middle of one of the buffers, we need to be sure to write that guy first */
		if ((!curmsgb) && (rsslSocketChannel->nextOutBuf != -1))
		{
			pLink = rsslQueuePeekFront(&rsslSocketChannel->priorityQueues[rsslSocketChannel->nextOutBuf].priorityQueue);
			if (pLink)
				curmsgb = RSSL_QUEUE_LINK_TO_OBJECT(rtr_msgb_t, link, pLink);

			if (curmsgb) {
				iovPriority[wrtveclen] = rsslSocketChannel->nextOutBuf;
				rsslSocketChannel->priorityQueues[rsslSocketChannel->nextOutBuf].tempIndex++;
				rsslSocketChannel->priorityQueues[rsslSocketChannel->nextOutBuf].tempList[rsslSocketChannel->priorityQueues[rsslSocketChannel->nextOutBuf].tempIndex] = curmsgb;
			}
		}

		while (!curmsgb)
		{
			/* do work to get buffer here */
			pLink = rsslQueuePeekFront(&rsslSocketChannel->priorityQueues[rsslSocketChannel->flushStrategy[rsslSocketChannel->currentOutList]].priorityQueue);
			if (pLink)
				curmsgb = RSSL_QUEUE_LINK_TO_OBJECT(rtr_msgb_t, link, pLink);

			if (curmsgb)
			{
				iovPriority[wrtveclen] = rsslSocketChannel->flushStrategy[rsslSocketChannel->currentOutList];
				rsslSocketChannel->priorityQueues[rsslSocketChannel->flushStrategy[rsslSocketChannel->currentOutList]].tempIndex++;
				rsslSocketChannel->priorityQueues[rsslSocketChannel->flushStrategy[rsslSocketChannel->currentOutList]].tempList[rsslSocketChannel->priorityQueues[rsslSocketChannel->flushStrategy[rsslSocketChannel->currentOutList]].tempIndex] = curmsgb;
			}

			rsslSocketChannel->currentOutList++;

			if ((rsslSocketChannel->flushStrategy[rsslSocketChannel->currentOutList] == -1) ||
				(rsslSocketChannel->currentOutList >= RIPC_MAX_FLUSH_STRATEGY))
				rsslSocketChannel->currentOutList = 0;

			if ((rsslSocketChannel->currentOutList == tempOutList) && (!curmsgb))
				break;
		}

		if (!curmsgb)
		{
			/* try to get from the low queue, just in case its not in the strategy */
			/* this needs to change if we have more than the 3 lists */
			pLink = rsslQueuePeekFront(&rsslSocketChannel->priorityQueues[2].priorityQueue);
			if (pLink)
				curmsgb = RSSL_QUEUE_LINK_TO_OBJECT(rtr_msgb_t, link, pLink);

			if (curmsgb)
			{
				iovPriority[wrtveclen] = 2;
				rsslSocketChannel->priorityQueues[2].tempIndex++;
				rsslSocketChannel->priorityQueues[2].tempList[rsslSocketChannel->priorityQueues[2].tempIndex] = curmsgb;
			}
		}
		else
		{
			tempOutList = rsslSocketChannel->currentOutList;
		}

		while ((curmsgb) && (wrtveclen < iovLength))
		{
			RIPC_IOV_SETBUF(&wrtvec[wrtveclen], curmsgb->local);
			if (curmsgb->local == curmsgb->buffer)
				RIPC_IOV_SETLEN(&wrtvec[wrtveclen], (RsslUInt32)curmsgb->length);
			else
				RIPC_IOV_SETLEN(&wrtvec[wrtveclen], (RsslUInt32)(curmsgb->length -
				((caddr_t)curmsgb->local - curmsgb->buffer)));

			if (RIPC_IOV_GETLEN(&wrtvec[wrtveclen]) > 0)
			{
				lenToWrite += RIPC_IOV_GETLEN(&wrtvec[wrtveclen]);
				wrtveclen++;
			}
			else
			{
				/* since the length is 0, we do not know if we will replace
				this in the write vector so we should set it back to -1 */
				iovPriority[wrtveclen] = -1;

				/* for now, just remove the curmsgb and release it */
				/* this is because it has length of 0 - not sure if this would actually happen */
				rsslSocketChannel->priorityQueues[curmsgb->priority].tempList[rsslSocketChannel->priorityQueues[curmsgb->priority].tempIndex] = 0;
				rsslSocketChannel->priorityQueues[curmsgb->priority].tempIndex--;
				rsslQueueRemoveLink(&rsslSocketChannel->priorityQueues[curmsgb->priority].priorityQueue, &(curmsgb->link));

				RIPC_ASSERT(curmsgb);
				rtr_dfltcFreeMsg(curmsgb);

			}
			/* set this to 0 */
			curmsgb = 0;

			/* now here get the buffer - this is instead of doing it in each of the if/else statements above */
			while ((!curmsgb) && (wrtveclen < iovLength))
			{
				if (rsslSocketChannel->priorityQueues[rsslSocketChannel->flushStrategy[rsslSocketChannel->currentOutList]].tempIndex > -1)
				{
					pLink = rsslQueuePeekNext(&rsslSocketChannel->priorityQueues[rsslSocketChannel->flushStrategy[rsslSocketChannel->currentOutList]].priorityQueue,
						&((rsslSocketChannel->priorityQueues[rsslSocketChannel->flushStrategy[rsslSocketChannel->currentOutList]].tempList[rsslSocketChannel->priorityQueues[rsslSocketChannel->flushStrategy[rsslSocketChannel->currentOutList]].tempIndex])->link));
					if (pLink)
						curmsgb = RSSL_QUEUE_LINK_TO_OBJECT(rtr_msgb_t, link, pLink);
				}
				else
				{
					pLink = rsslQueuePeekFront(&rsslSocketChannel->priorityQueues[rsslSocketChannel->flushStrategy[rsslSocketChannel->currentOutList]].priorityQueue);
					if (pLink)
						curmsgb = RSSL_QUEUE_LINK_TO_OBJECT(rtr_msgb_t, link, pLink);
				}

				if (curmsgb)
				{
					iovPriority[wrtveclen] = rsslSocketChannel->flushStrategy[rsslSocketChannel->currentOutList];
					rsslSocketChannel->priorityQueues[rsslSocketChannel->flushStrategy[rsslSocketChannel->currentOutList]].tempIndex++;
					rsslSocketChannel->priorityQueues[rsslSocketChannel->flushStrategy[rsslSocketChannel->currentOutList]].tempList[rsslSocketChannel->priorityQueues[rsslSocketChannel->flushStrategy[rsslSocketChannel->currentOutList]].tempIndex] = curmsgb;
				}

				rsslSocketChannel->currentOutList++;

				if ((rsslSocketChannel->flushStrategy[rsslSocketChannel->currentOutList] == -1) ||
					(rsslSocketChannel->currentOutList == RIPC_MAX_FLUSH_STRATEGY))
					rsslSocketChannel->currentOutList = 0;

				if ((rsslSocketChannel->currentOutList == tempOutList) && (!curmsgb))
					break;
			}

			if ((!curmsgb) && (wrtveclen < iovLength))
			{
				/* try to get from low list */
				/* this needs to change if we allow more than the 3 lists */
				if (rsslSocketChannel->priorityQueues[2].tempIndex > -1) {
					pLink = rsslQueuePeekNext(&rsslSocketChannel->priorityQueues[2].priorityQueue, &((rsslSocketChannel->priorityQueues[2].tempList[rsslSocketChannel->priorityQueues[2].tempIndex])->link));
					if(pLink)
						curmsgb = RSSL_QUEUE_LINK_TO_OBJECT(rtr_msgb_t, link, pLink);
				}
				else
				{
					pLink = rsslQueuePeekFront(&rsslSocketChannel->priorityQueues[2].priorityQueue);
					if (pLink)
						curmsgb = RSSL_QUEUE_LINK_TO_OBJECT(rtr_msgb_t, link, pLink);
				}

				if (curmsgb)
				{
					iovPriority[wrtveclen] = 2;

					rsslSocketChannel->priorityQueues[2].tempIndex++;
					rsslSocketChannel->priorityQueues[2].tempList[rsslSocketChannel->priorityQueues[2].tempIndex] = curmsgb;
				}
			}
			else
			{
				tempOutList = rsslSocketChannel->currentOutList;
			}
		}

		if (wrtveclen > 0)
		{
			if (rsslSocketChannel->httpHeaders)
				cc = (*(rsslSocketChannel->transportFuncs->writeVTransport))(rsslSocketChannel->tunnelTransportInfo, wrtvec, wrtveclen, lenToWrite, rwflags, error);
			else
				cc = (*(rsslSocketChannel->transportFuncs->writeVTransport))(rsslSocketChannel->transportInfo, wrtvec, wrtveclen, lenToWrite, rwflags, error);

			if (rsslSocketChannel->workState & RIPC_INT_SHTDOWN_PEND)
			{
				_rsslSetError(error, NULL, RSSL_RET_FAILURE, errno);
				snprintf(error->text, MAX_RSSL_ERROR_TEXT,
					"<%s:%d> Error: 1003 ipcFlushSession() failed due to channel shutting down.\n",
					__FILE__, __LINE__);

				return RSSL_RET_FAILURE;
			}

			if (cc < 0)
			{
				/* there was an error on write - this could be because we didnt send anything and a proxy killed our connection if we are tunneling */
				/* flip the fd's and write this data again */
				if (rsslSocketChannel->newTunnelTransportInfo && rsslSocketChannel->httpHeaders)
				{
					rsslSocketChannel->tunnelTransportInfo = rsslSocketChannel->newTunnelTransportInfo;
					rsslSocketChannel->newTunnelTransportInfo = 0;

					cc = (*(rsslSocketChannel->transportFuncs->writeVTransport))(rsslSocketChannel->tunnelTransportInfo, wrtvec, wrtveclen, lenToWrite, rwflags, error);
					/* if it didnt work this time, end the connection */
					if (cc < 0)
					{
						_rsslSetError(error, NULL, RSSL_RET_FAILURE, errno);
						snprintf(error->text, MAX_RSSL_ERROR_TEXT,
							"<%s:%d> Error: 1002 ipcWrite() failed. System errno: (%d)\n",
							__FILE__, __LINE__, errno);

						return RSSL_RET_FAILURE;
					}
				}
				else
				{
					_rsslSetError(error, NULL, RSSL_RET_FAILURE, errno);
					snprintf(error->text, MAX_RSSL_ERROR_TEXT,
						"<%s:%d> Error: 1002 ipcWrite() failed. System errno: (%d)\n",
						__FILE__, __LINE__, errno);

					return RSSL_RET_FAILURE;
				}
			}

			if (cc == lenToWrite)
			{
				while (wrtveclen > 0)
				{
					wrtveclen--;
					pLink = rsslQueueRemoveFirstLink(&rsslSocketChannel->priorityQueues[iovPriority[wrtveclen]].priorityQueue);
					if(pLink)
						curmsgb = RSSL_QUEUE_LINK_TO_OBJECT(rtr_msgb_t, link, pLink);

					rsslSocketChannel->priorityQueues[iovPriority[wrtveclen]].queueLength -= (RsslInt32)curmsgb->length;
					iovPriority[wrtveclen] = -1;
					rsslSocketChannel->nextOutBuf = -1;
					RIPC_ASSERT(curmsgb);
					rtr_dfltcFreeMsg(curmsgb);
					curmsgb = 0;
				}

				/* need to switch stream IDs after sending the end of chunk message */
				if (reducedIovLen && rsslSocketChannel->newTunnelTransportInfo && rsslSocketChannel->httpHeaders  && rsslSocketChannel->sentControlAck)
				{
					RsslInt32 outLen;
					RsslInt32 writtenLen = 0;
					char outBuf[255];

					rsslSocketChannel->sentControlAck = 0;
					outLen = sprintf(outBuf, "%x\r\n\r\n", 0);
					/* we may need to send this footer instead of just the second \r\n */
					//outLen += snprintf(outBuf + outLen,(255-outLen), "%s", "Connection: close\r\n");

					writtenLen = (*(rsslSocketChannel->transportFuncs->writeTransport))(rsslSocketChannel->tunnelTransportInfo, outBuf, outLen, rwflags, error);

					/* if it doesnt write the entire thing or it returns a neg value we are in a bad state */
					if (writtenLen == outLen)
					{
						/* Close the old FD here, if present */
						if ((rsslSocketChannel->oldTunnelStreamFd) &&
							((rsslSocketChannel->connType == RSSL_CONN_TYPE_ENCRYPTED) ?
							(rsslSocketChannel->oldTunnelStreamFd != ((ripcSSLSession*)rsslSocketChannel->newTunnelTransportInfo)->socket) :
								(rsslSocketChannel->oldTunnelStreamFd != (RsslSocket)(intptr_t)(rsslSocketChannel->newTunnelTransportInfo))))
						{
							sock_close((RsslSocket)(intptr_t)rsslSocketChannel->oldTunnelStreamFd);
							rsslSocketChannel->oldTunnelStreamFd = RIPC_INVALID_SOCKET;
						}

						/* success - switch file descriptors */
						/* Only store the old FD when this is a WinInet-based tunnel.
						* Otherwise the shutdown() call will have already closed the socket properly. */
						if (rsslSocketChannel->connType == RSSL_CONN_TYPE_ENCRYPTED)
						{
							if (!rsslSocketChannel->isJavaTunnel)
							{
								rsslSocketChannel->oldTunnelStreamFd = (RsslSocket)(((ripcSSLSession*)rsslSocketChannel->tunnelTransportInfo)->socket);
							}
							ripcShutdownSSLSocket(rsslSocketChannel->tunnelTransportInfo);
						}
						else
						{
							if (!rsslSocketChannel->isJavaTunnel)
							{
								rsslSocketChannel->oldTunnelStreamFd = (RsslSocket)(intptr_t)(rsslSocketChannel->tunnelTransportInfo);
							}
							shutdown((RsslSocket)(intptr_t)(rsslSocketChannel->tunnelTransportInfo), shutdownFlag);
						}

						rsslSocketChannel->tunnelTransportInfo = rsslSocketChannel->newTunnelTransportInfo;
						rsslSocketChannel->newTunnelTransportInfo = 0;
						/* set this back to the max length in case we go through the while loop again */
						iovLength = RIPC_MAXIOVLEN;
						reducedIovLen = 0;

					}
					else if (writtenLen != 0)
					{
						/* this means we didnt write entire chunked footer, but we wrote some of it */
						_rsslSetError(error, NULL, RSSL_RET_FAILURE, errno);
						snprintf(error->text, MAX_RSSL_ERROR_TEXT,
							"<%s:%d> Error: 1007 ipcFlushSession() Error writing chunk footer(%d)",
							__FILE__, __LINE__, writtenLen);

						return RSSL_RET_FAILURE;
					}
				}
			}
			else
			{
				RsslInt32 curpos = 0;
				while (cc > 0)
				{
					if (RIPC_IOV_GETLEN(&wrtvec[curpos]) <= (RsslUInt32)cc)
					{
						pLink = rsslQueueRemoveFirstLink(&rsslSocketChannel->priorityQueues[iovPriority[curpos]].priorityQueue);
						if (pLink)
							curmsgb = RSSL_QUEUE_LINK_TO_OBJECT(rtr_msgb_t, link, pLink);

						rsslSocketChannel->priorityQueues[iovPriority[curpos]].queueLength -= (RsslInt32)curmsgb->length;
						iovPriority[curpos] = -1;

						rsslSocketChannel->nextOutBuf = -1;

						RIPC_ASSERT(curmsgb);
						rtr_dfltcFreeMsg(curmsgb);

						curmsgb = 0;

						cc -= RIPC_IOV_GETLEN(&wrtvec[curpos]);
						curpos++;
					}
					else
					{
						/* do not want to change output lengths here - this will be done
						when we write the entire buffer */
						pLink = rsslQueuePeekFront(&rsslSocketChannel->priorityQueues[iovPriority[curpos]].priorityQueue);
						if (pLink)
							curmsgb = RSSL_QUEUE_LINK_TO_OBJECT(rtr_msgb_t, link, pLink);

						rsslSocketChannel->nextOutBuf = iovPriority[curpos];
						iovPriority[curpos] = -1;

						RIPC_ASSERT(curmsgb);

						curmsgb->local = (caddr_t)curmsgb->local + cc;
						/* The length of curmsgb does not need to be modified since the rest of the data left
						* in the buffer to be flushed is recalculated when the iovector is reset.
						* See above.
						*/

						cc = 0;
						curmsgb = 0;
						curpos++;
					}
				}
			}

			if (rsslSocketChannel->blocking == 0)
				cont = 0;
			else
				curmsgb = 0;
		}
		else
		{
			if (curmsgb == 0)
			{
				break;
			}
			else
			{
				rsslQueueRemoveLink(&rsslSocketChannel->priorityQueues[curmsgb->priority].priorityQueue, &(curmsgb->link));

				RIPC_ASSERT(curmsgb);
				rtr_dfltcFreeMsg(curmsgb);
				curmsgb = 0;
			}
		}
	}
	for (i = 0; i < RIPC_MAX_PRIORITY_QUEUE; i++)
		retVal += rsslSocketChannel->priorityQueues[i].queueLength;
	return(retVal);
}

/*********************************************
*	The following defines the functions needed
*	for checking the ripc.
*********************************************/

ripcSessInit ipcIntSessInit(RsslSocketChannel *rsslSocketChannel, ripcSessInProg *inPr, RsslError *error)
{
	RsslInt32	cont = 1;
	RsslInt32	ret = RIPC_CONN_ERROR;

	RIPC_ASSERT(rsslSocketChannel->state == RSSL_CH_STATE_INITIALIZING);

	_DEBUG_TRACE_CONN("fd %d\n",rsslSocketChannel->stream);

	inPr->types = 0;
	inPr->intConnState = 0;
	while (cont)
	{
		switch (rsslSocketChannel->intState)
		{
		case RIPC_INT_ST_TRANSPORT_INIT:
			ret = ipcInitTransport(rsslSocketChannel, inPr, error);
			break;
		case RIPC_INT_ST_CLIENT_TRANSPORT_INIT:
			ret = ipcInitClientTransport(rsslSocketChannel, inPr, error);
			break;
		case RIPC_INT_ST_READ_HDR:
			ret = ipcReadHdr(rsslSocketChannel, inPr, error);
			break;
		case RIPC_INT_ST_COMPLETE:
			ret = ipcFinishSess(rsslSocketChannel, inPr, error);
			break;
		case RIPC_INT_ST_WAIT_CLIENT_KEY:
			ret = ipcWaitClientKey(rsslSocketChannel, inPr, error);
			break;
		case RIPC_INT_ST_SEND_CLIENT_KEY:
			ret = ipcSendClientKey(rsslSocketChannel, inPr, error);
			break;
		case RIPC_INT_ST_CONNECTING:
			ret = ipcConnecting(rsslSocketChannel, inPr, error);
			break;
		case RIPC_INT_ST_PROXY_CONNECTING:
			ret = ipcProxyConnecting(rsslSocketChannel, inPr, error);
			break;
		case RIPC_INT_ST_ACCEPTING:
			ret = ipcClientAccept(rsslSocketChannel, inPr, error);
			break;
		case RIPC_INT_ST_WAIT_ACK:
			ret = ipcWaitAck(rsslSocketChannel, inPr, error, NULL, 0);
			break;
		case RIPC_INT_ST_CLIENT_WAIT_PROXY_ACK:
			ret = ipcWaitProxyAck(rsslSocketChannel, inPr, error);
			break;
		case RIPC_INT_ST_WS_SEND_OPENING_HANDSHAKE:
			ret = rwsSendOpeningHandshake(rsslSocketChannel, inPr, error);
			break;
		case RIPC_INT_ST_WS_WAIT_HANDSHAKE_RESPONSE:
			ret = rwsWaitResponseHandshake(rsslSocketChannel, inPr, error);
			break;
		}
		if ((ret != RIPC_CONN_IN_PROGRESS) || (rsslSocketChannel->blocking == 0))
			cont = 0;
	}
	/* put the standard state in the bottom byte; we may have put internal states in the higher order bytes */
	inPr->intConnState |= (RsslUInt8)rsslSocketChannel->intState;

	_DEBUG_TRACE_CONN("ret %d intConnState %d upper %d lower %d intState %d\n", ret, inPr->intConnState, (inPr->intConnState&0xff00),(inPr->intConnState&0x00ff), rsslSocketChannel->intState)

	return(ret);
}

static ripcSessInit ipcInitTransport(RsslSocketChannel *rsslSocketChannel, ripcSessInProg *inProg, RsslError *error)
{
	RsslInt32 cc;

	_DEBUG_TRACE_CONN("called\n")
	cc = (*(rsslSocketChannel->transportFuncs->initializeTransport))(
		rsslSocketChannel->transportInfo, inProg, error);

	/* If cc is 0, this indicates that the transport initialization has not completed yet.  A negative return indicates failure, positive success. */
	if (cc < 0)
	{
		_rsslSetError(error, NULL, RSSL_RET_FAILURE, errno);

		return(RIPC_CONN_ERROR);
	}
	else if (cc > 0)
	{
		rsslSocketChannel->intState = RIPC_INT_ST_READ_HDR;
	}

	return(RIPC_CONN_IN_PROGRESS);
}

static ripcSessInit ipcInitClientTransport(RsslSocketChannel *rsslSocketChannel, ripcSessInProg *inProg, RsslError *error)
{
	RsslInt32 cc;

	_DEBUG_TRACE_CONN("called\n")
	cc = (*(rsslSocketChannel->transportFuncs->initializeTransport))(
		rsslSocketChannel->transportInfo, inProg, error);

	if (cc < 0)
	{
		/* This function should only be called for openSSL-based encrypted connections. */
		if ( rsslSocketChannel->usingWinInet)
		{
			return(RIPC_CONN_ERROR);
		}
		else
		{
			/* InitializeTransport returns -2 on certificate failure */
			if (cc == -2)
			{
				return(RIPC_CONN_ERROR);
			}
			rsslSocketChannel->sslProtocolBitmap = ripcRemoveHighestSSLVersionFlag(rsslSocketChannel->sslProtocolBitmap);
			if (rsslSocketChannel->sslProtocolBitmap == RIPC_PROTO_SSL_NONE)
			{
				return(RIPC_CONN_ERROR);
			}
			return ipcReconnectSocket(rsslSocketChannel, inProg, error);
		}

	}
	else if (cc > 0)
	{
		if (rsslSocketChannel->sslEncryptedProtocolType != RSSL_CONN_TYPE_WEBSOCKET)
		{
			rsslSocketChannel->intState = RIPC_INT_ST_CONNECTING;
			return ipcConnecting(rsslSocketChannel, inProg, error);
		}
		else
		{
			rsslSocketChannel->intState = RIPC_INT_ST_WS_SEND_OPENING_HANDSHAKE;
			return rwsSendOpeningHandshake(rsslSocketChannel, inProg, error);
		}
	}

	return(RIPC_CONN_IN_PROGRESS);
}


static RsslRet ipcGetSocketRow(RsslSocketChannel *rsslSocketChannel, RsslError *error)
{
#if (defined(_WINDOWS) || defined(_WIN32))
	PMIB_TCPTABLE tcpTable = NULL;
	PMIB_TCPROW tmpRow = NULL;
	DWORD size = 0;
	DWORD status = 0;
	DWORD i = 0;
	char *chnlInfo;						// Need to set this as a larger buffer because getsockopt SO_BSP_STATE requires more memory than the CSADDR_INFO structure
	struct sockaddr_in *localInfo;
	struct sockaddr_in *remoteInfo;
	TCP_ESTATS_PATH_RW_v0 collectionFlags;
	int len;
	ULONG tmp;

	/* If this is a winInet or Extended Line connection, do not set the row, and just exit.  If this is an HTTP Java connection, we can continue */
	if (rsslSocketChannel->usingWinInet || rsslSocketChannel->connType == RSSL_CONN_TYPE_EXT_LINE_SOCKET || (rsslSocketChannel->httpHeaders == 1 && rsslSocketChannel->isJavaTunnel == 0))
	{
		rsslSocketChannel->socketRowSet = RSSL_FALSE;
		return RSSL_RET_SUCCESS;
	}

	len = (int)sizeof(CSADDR_INFO)+128;

	chnlInfo = malloc(len+128);
	if (chnlInfo == NULL) {
		_rsslSetError(error, NULL, RSSL_RET_FAILURE, errno);
		snprintf(error->text, MAX_RSSL_ERROR_TEXT, "<%s:%d> ipcGetSocketRow() Error: 1001 Could not allocate memory for the Channel Info.\n", __FILE__, __LINE__);
		return RSSL_RET_FAILURE;
	}
	
	if(getsockopt(rsslSocketChannel->stream, SOL_SOCKET, SO_BSP_STATE, chnlInfo, &len) < 0)
	{
		_rsslSetError(error, NULL, RSSL_RET_FAILURE, errno);
		snprintf(error->text, MAX_RSSL_ERROR_TEXT, "<%s:%d> ipcGetSocketRow() Error: 1002 getsockopt() SO_BSP_STATE failed.  System errno: (%d)", __FILE__, __LINE__, errno);
		free((void*)chnlInfo);
		return RSSL_RET_FAILURE;
	}

	localInfo = (struct sockaddr_in*)((CSADDR_INFO*)chnlInfo)->LocalAddr.lpSockaddr;
	remoteInfo = (struct sockaddr_in*)((CSADDR_INFO*)chnlInfo)->RemoteAddr.lpSockaddr;
	
	status = GetTcpTable(tcpTable, &size, TRUE);
	if (status != ERROR_INSUFFICIENT_BUFFER) {
		_rsslSetError(error, NULL, RSSL_RET_FAILURE, errno);
		snprintf(error->text, MAX_RSSL_ERROR_TEXT, "<%s:%d> ipcGetSocketRow() Error: 1002 GetTcpTable() failed.  Error from function: (%d)", __FILE__, __LINE__, status);
		free((void*)chnlInfo);
		return RSSL_RET_FAILURE;
	}

	tcpTable = (PMIB_TCPTABLE)malloc(size);
	if(tcpTable == NULL){
		_rsslSetError(error, NULL, RSSL_RET_FAILURE, errno);
		snprintf(error->text, MAX_RSSL_ERROR_TEXT, "<%s:%d> ipcGetSocketRow() Error: 1001 Could not allocate memory for the TCP Table.\n", __FILE__, __LINE__);
		free((void*)chnlInfo);
		return RSSL_RET_FAILURE;
	}

	status = GetTcpTable(tcpTable, &size, TRUE);
	if (status != ERROR_SUCCESS) {
		_rsslSetError(error, NULL, RSSL_RET_FAILURE, errno);
		snprintf(error->text, MAX_RSSL_ERROR_TEXT, "<%s:%d> ipcGetSocketRow() Error: 1002 GetTcpTable() failed.  Error from function: (%d)", __FILE__, __LINE__, status);
		free((void*)tcpTable);
		free((void*)chnlInfo);
		return RSSL_RET_FAILURE;
	}

	for (i = 0; i < tcpTable->dwNumEntries; i++)
	{
		tmpRow = &(tcpTable->table[i]);

		/* If this is an accepted server connection, the socket was not bound to a specific interface, so the dwLocalAddr will be 0 */
		if (tmpRow->dwLocalPort == localInfo->sin_port && (rsslSocketChannel->server != NULL || tmpRow->dwLocalAddr == localInfo->sin_addr.s_addr) &&
			tmpRow->dwRemotePort == remoteInfo->sin_port && tmpRow->dwRemoteAddr == remoteInfo->sin_addr.s_addr)
		{
			rsslSocketChannel->socketRow = *(tmpRow);
			free((void*)tcpTable);
			collectionFlags.EnableCollection = TRUE;

			/* Attempt to set collection stats on the channel.  If this fails, it is not catastrophic, so we should not error out here. */
			if ((tmp = SetPerTcpConnectionEStats(&(rsslSocketChannel->socketRow), TcpConnectionEstatsPath, (PUCHAR)&collectionFlags, 0, sizeof(collectionFlags), 0)) != NO_ERROR)
			{
				rsslSocketChannel->socketRowSet = RSSL_FALSE;
			}

			free((void*)chnlInfo);
			return RSSL_RET_SUCCESS;
		}
	}

	free((void*)tcpTable);
	_rsslSetError(error, NULL, RSSL_RET_FAILURE, 0);
	snprintf(error->text, MAX_RSSL_ERROR_TEXT, "<%s:%d> ipcGetSocketRow() Error: 1002 Unable to find the table entry for the current connection", __FILE__, __LINE__);
	free((void*)chnlInfo);
	return RSSL_RET_FAILURE;
#else
	/* For Linux, this is a no-op */
	return RSSL_RET_SUCCESS;
#endif

}


static ripcSessInit ipcReadHdr(RsslSocketChannel *rsslSocketChannel, ripcSessInProg *inProg, RsslError *error)
{
	RsslInt32 cc;
	ripcRWFlags				rwflags = RIPC_RW_NONE;

	rwflags |= (rsslSocketChannel->blocking ? RIPC_RW_BLOCKING : 0);

	RIPC_ASSERT(rsslSocketChannel->intState == RIPC_INT_ST_READ_HDR);

	_DEBUG_TRACE_CONN("fd "SOCKET_PRINT_TYPE"\n", rsslSocketChannel->stream)

	if (rsslSocketChannel->tunnelingState == RIPC_TUNNEL_REMOVE_SESSION)
	{
		_rsslSetError(error, NULL, RSSL_RET_FAILURE, errno);
		snprintf((error->text), MAX_RSSL_ERROR_TEXT,
			" <%s:%d> Error: 1006 Not a new connection request - keeping alive tunneling connection.\n",
			__FILE__, __LINE__);

		return(RIPC_CONN_ERROR);
	}


	//         What is the limit for an opening HS with additional headers
	cc = (*(rsslSocketChannel->protocolFuncs->readTransportMsg))((void*)rsslSocketChannel,
		(rsslSocketChannel->inputBuffer->buffer + rsslSocketChannel->inputBuffer->length),
		(RsslInt32)(rsslSocketChannel->inputBuffer->maxLength - rsslSocketChannel->inputBuffer->length), 
		rwflags, error);


	if (rsslSocketChannel->workState & RIPC_INT_SHTDOWN_PEND)
	{
		_rsslSetError(error, NULL, RSSL_RET_FAILURE, errno);
		snprintf(error->text, MAX_RSSL_ERROR_TEXT,
			"<%s:%d> Error: 1003 ipcReadHdr() failed due to channel shutting down.\n",
			__FILE__, __LINE__);

		return(RIPC_CONN_ERROR);
	}

	_DEBUG_TRACE_CONN("fd "SOCKET_PRINT_TYPE" %d\n", rsslSocketChannel->stream, cc)

	if (cc == RSSL_RET_READ_WOULD_BLOCK)
	{
		cc = 0;
	}

	if (cc < 0)
	{
		_rsslSetError(error, NULL, RSSL_RET_FAILURE, errno);
		snprintf(error->text, MAX_RSSL_ERROR_TEXT,
			"<%s:%d> Error: 1002 Could not read IPC Mount Request. System errno: (%d)\n",
			__FILE__, __LINE__, errno);

		return(RIPC_CONN_ERROR);
	}
	rsslSocketChannel->inputBuffer->length += cc;

_DEBUG_TRACE_CONN("read WS Header: conType %d proTyp: %d\n", 
				rsslSocketChannel->connType, 
				rsslSocketChannel->protocolType)

	
	return ipcProcessHdr(rsslSocketChannel, inProg, error, cc);
}

ripcSessInit ipcProcessHdr(RsslSocketChannel *rsslSocketChannel, ripcSessInProg *inProg, RsslError *error, RsslInt32 cc)
{
	RsslUInt32			version_number;
	RsslInt32			i;
	ripcSocketOption	opts;
	char 				*hdrStart;
	RsslInt32			discoveredVersion = 0;
	RsslUInt16			length = 0;
	RsslUInt8			opCode = 0;
	RsslInt32			totalMsgLength = 0;
	RsslQueueLink		*pLink = 0;

	hdrStart = rsslSocketChannel->inputBuffer->buffer + rsslSocketChannel->inputBufCursor;

	_DEBUG_TRACE_CONN("hdrStart:%p, cc:%d buf:%p\n", (void*)hdrStart, cc, (void*)(rsslSocketChannel->inputBuffer->buffer))

	if (cc < 7)
	{
		/* This will happen when there is no data atall to be read,
		and the read fails with error _IPC_WOULD_BLOCK or EINTR */
		if (cc < 0)
		{
			_rsslSetError(error, NULL, RSSL_RET_FAILURE, errno);
			snprintf(error->text, MAX_RSSL_ERROR_TEXT,
				"<%s:%d> Error: 1002 Invalid mount request size. System errno: (%d)\n",
				__FILE__, __LINE__, cc);

			return(RIPC_CONN_ERROR);
		}
		else
		{
			return(RIPC_CONN_IN_PROGRESS);
		}
	}

	if (rsslSocketChannel->dbgFlags & RSSL_DEBUG_IPC_DUMP_INIT)
	{
		(*(ripcDumpInFunc))(__FUNCTION__, hdrStart, (RsslUInt32)(rsslSocketChannel->inputBuffer->length - rsslSocketChannel->inputBufCursor), rsslSocketChannel->stream);
	}

	_move_u16_swap(&length, hdrStart);
	opCode = hdrStart[2];
	_move_u32_swap(&version_number, hdrStart + 3);

	_DEBUG_TRACE_READ("read (len=%d, opCode=0x%x, version=0x%x)\n", length, opCode, version_number)

	/* because tunneling can send this message as multiple parts, we cant depend on CC for the length of the message */
	totalMsgLength = (RsslInt32)(rsslSocketChannel->inputBuffer->length - rsslSocketChannel->inputBufCursor);

	switch (version_number)
	{
		/* Have all versions go into common code where we switch on what is different for maintainability */
	case CONN_VERSION_10:  /* initial version */
	case CONN_VERSION_11:  /* bidirectional compression */
	case CONN_VERSION_12:  /* lz4 and protocol type exchange */
	case CONN_VERSION_13:  /* force compression as non ZLIB and component versioning */
	case CONN_VERSION_14:  /* app signing key negotiation */
	{
		/* Start of connection handshake */
		_DEBUG_TRACE_CONN("processing a version %d header\n", dumpConnVersion(version_number))

		if (totalMsgLength < V10_MIN_CONN_HDR)
		{
			if (totalMsgLength != OURSOCKADDR_SIZE)
			{
				_rsslSetError(error, NULL, RSSL_RET_FAILURE, errno);
				snprintf(error->text, MAX_RSSL_ERROR_TEXT,
					"<%s:%d> Error: 1007 Invalid Ripc connection request size (%d)\n",
					__FILE__, __LINE__, totalMsgLength);

				return(RIPC_CONN_ERROR);
			}
		}
		else
		{
			{
				RsslUInt16	hdrSize;
				RsslUInt8	flags;
				RsslUInt8	compbitmapsize;
				RsslInt32	hdrCursor = 0;
				RsslUInt8	addrLen = 0;
				RsslUInt8	hostnameLen = 0;
				RsslInt32	pingTimeout = 0;
				RsslUInt8	minor = 0;
				RsslUInt8	major = 0;
				RsslUInt8	protocoltype = 0;
				RsslInt32	tempIter;
				/* These intentionally start at -1, so places below for older
				* connection versions that do not use it add 0 to counts */
				RsslInt16 compVerLen = -1;
				RsslInt16 compStringLen = -1;
				unsigned char componentVersionLen = 0;
				unsigned char componentStringLen = 0;


				flags = hdrStart[7];  /* may indicate that client wants key exchange if version 14 or higher */
				hdrSize = hdrStart[8];

				compbitmapsize = hdrStart[9];

				switch (version_number)
				{
				case CONN_VERSION_13:
				case CONN_VERSION_14:
				{
					/* figure out component version lengths up front */
					hostnameLen = hdrStart[15 + compbitmapsize];
					addrLen = hdrStart[16 + compbitmapsize + hostnameLen];
					componentVersionLen = hdrStart[17 + compbitmapsize + hostnameLen + addrLen];
					compVerLen = componentVersionLen;
				}
				break;
				default:
					/* fall through for all older versions */
					;
				}

				/* Browsers put CR/NEWL after the end of all messages they send.
				* If this is the case, ignore that last two characters.
				*/
				if (((hdrSize + compVerLen + 1) != totalMsgLength) && (((totalMsgLength - (hdrSize + compVerLen + 1)) == 2)))
				{
					if ((hdrStart[totalMsgLength - 2] == '\r') && (hdrStart[totalMsgLength - 1] == '\n'))
						totalMsgLength = (hdrSize + compVerLen + 1);
				}

				if ((hdrSize + compVerLen + 1) != totalMsgLength)
				{
					_rsslSetError(error, NULL, RSSL_RET_FAILURE, errno);
					snprintf(error->text, MAX_RSSL_ERROR_TEXT,
						"<%s:%d> Error: 1007 Invalid Conn Ver %d header size %d\n",
						__FILE__, __LINE__, dumpConnVersion(version_number), (hdrSize + compVerLen + 1));

					return(RIPC_CONN_ERROR);
				}

				/* Compression Logic Start */
				/* set cursor to next position after comp bitmap */
				hdrCursor = 10 + compbitmapsize;

				/* Only care about compression if the server wants to do compression */
				switch (version_number)
				{
				case CONN_VERSION_10:
				{
					if ((compbitmapsize > 0) && (rsslSocketChannel->srvrcomp))
					{
						RsslUInt8 *compbitmap = (RsslUInt8*)&(hdrStart[10]);
						for (i = 0; i <= RSSL_COMP_MAX_TYPE; i++)
						{
							RsslInt16 idx = ripccompressions[i][RSSL_COMP_BYTEINDEX];
							if ((idx < RSSL_COMP_BITMAP_SIZE) && (idx < (RsslInt16)compbitmapsize))
							{
								if (compbitmap[idx] & ripccompressions[i][RSSL_COMP_BYTEBIT])
								{
									rsslSocketChannel->outCompression = ripccompressions[i][RSSL_COMP_TYPE];
									break;
								}
							}
						}
						if (rsslSocketChannel->outCompression > RSSL_COMP_MAX_TYPE)
							rsslSocketChannel->outCompression = RSSL_COMP_NONE;
						rsslSocketChannel->outCompFuncs = &(compressFuncs[rsslSocketChannel->outCompression]);
						if ((rsslSocketChannel->outCompFuncs->compressInit == 0) ||
							(rsslSocketChannel->outCompFuncs->compressEnd == 0) ||
							(rsslSocketChannel->outCompFuncs->compress == 0))
						{
							rsslSocketChannel->outCompression = RSSL_COMP_NONE;
							rsslSocketChannel->outCompFuncs = 0;
						}
						if (rsslSocketChannel->outCompFuncs)
						{
								_DEBUG_TRACE_CONN("about to initialize compression = %d\n", rsslSocketChannel->inDecompress)
							rsslSocketChannel->c_stream_out = (*(rsslSocketChannel->outCompFuncs->compressInit))(
									rsslSocketChannel->server->zlibCompressionLevel, 0, error);
							if (rsslSocketChannel->c_stream_out == 0)
							{
								_rsslSetError(error, NULL, RSSL_RET_FAILURE, errno);

								return(RIPC_CONN_ERROR);
							}
						}
					}
					rsslSocketChannel->curInputBuf = rtr_smplcDupMsg(gblInputBufs, rsslSocketChannel->inputBuffer);

					if (rsslSocketChannel->curInputBuf == 0)
					{
						_rsslSetError(error, NULL, RSSL_RET_FAILURE, errno);
						snprintf(error->text, MAX_RSSL_ERROR_TEXT,
							"<%s:%d> Error: 1001 Could not allocate memory for channel's input buffer.\n",
							__FILE__, __LINE__);

						return(RIPC_CONN_ERROR);
					}
						_DEBUG_TRACE_CONN("rsslSocketChannel->outCompression = %d\n", rsslSocketChannel->outCompression)
				}
				break;
				/* Adds bidirectional compression */
				case CONN_VERSION_11:
				{
					if (((compbitmapsize > 0) && (rsslSocketChannel->srvrcomp)) || (rsslSocketChannel->forcecomp))
					{
						/* take bitmap off wire if its there */
						if (compbitmapsize > 0)
						{
							RsslUInt8 *compbitmap = (RsslUInt8*)&(hdrStart[10]);

							for (i = 0; i <= RSSL_COMP_MAX_TYPE; i++)
							{
								RsslInt16 idx = ripccompressions[i][RSSL_COMP_BYTEINDEX];
								if ((idx < RSSL_COMP_BITMAP_SIZE) && (idx < (RsslInt16)compbitmapsize))
								{
									if ((compbitmap[idx] & ripccompressions[i][RSSL_COMP_BYTEBIT]) != 0 &&
										((ripccompressions[i][RSSL_COMP_TYPE] & rsslSocketChannel->srvrcomp) != 0))
									{
										rsslSocketChannel->outCompression = ripccompressions[i][RSSL_COMP_TYPE];
										break;
									}
								}
							}
						}
						else
						{
							/* if we are going to force compression, force zlib since we know all versions of ripc support this */
							rsslSocketChannel->outCompression = RSSL_COMP_ZLIB;
						}
						if (rsslSocketChannel->outCompression > RSSL_COMP_MAX_TYPE)
							rsslSocketChannel->outCompression = RSSL_COMP_NONE;
						rsslSocketChannel->inDecompress = rsslSocketChannel->outCompression;
						rsslSocketChannel->outCompFuncs = &(compressFuncs[rsslSocketChannel->outCompression]);
						rsslSocketChannel->inDecompFuncs = &(compressFuncs[rsslSocketChannel->inDecompress]);
						if ((rsslSocketChannel->outCompFuncs->compressInit == 0) ||
							(rsslSocketChannel->outCompFuncs->compressEnd == 0) ||
							(rsslSocketChannel->outCompFuncs->compress == 0))
						{
							rsslSocketChannel->outCompression = RSSL_COMP_NONE;
							rsslSocketChannel->outCompFuncs = 0;
						}
						if ((rsslSocketChannel->inDecompFuncs->decompress == 0) ||
							(rsslSocketChannel->inDecompFuncs->decompressEnd == 0) ||
							(rsslSocketChannel->inDecompFuncs->decompressInit == 0))
						{
							rsslSocketChannel->inDecompress = 0;
							rsslSocketChannel->inDecompFuncs = 0;
						}
						if (rsslSocketChannel->outCompFuncs)
						{
								_DEBUG_TRACE_CONN("about to initialize compression = %d\n", rsslSocketChannel->outCompression)
							rsslSocketChannel->c_stream_out = (*(rsslSocketChannel->outCompFuncs->compressInit))(
									rsslSocketChannel->server->zlibCompressionLevel, 0, error);
							if (rsslSocketChannel->c_stream_out == 0)
							{
								_rsslSetError(error, NULL, RSSL_RET_FAILURE, errno);
								return(RIPC_CONN_ERROR);
							}
						}
						if (rsslSocketChannel->inDecompFuncs)
						{
								_DEBUG_TRACE_CONN("about to initialize decompression = %d\n", rsslSocketChannel->inDecompress)
								rsslSocketChannel->c_stream_in = (*(rsslSocketChannel->inDecompFuncs->decompressInit))(0, error);
							if (rsslSocketChannel->c_stream_in == 0)
							{
								_rsslSetError(error, NULL, RSSL_RET_FAILURE, errno);
								return(RIPC_CONN_ERROR);
							}
						}
					}
						_DEBUG_TRACE_CONN("rsslSocketChannel->outCompression = %d\n", rsslSocketChannel->outCompression)
					/* Compression both directions */
					if (rsslSocketChannel->inDecompress)
					{
						rsslSocketChannel->decompressBuf = rtr_smplcAllocMsg(gblInputBufs, rsslSocketChannel->maxMsgSize);
						if (rsslSocketChannel->decompressBuf == 0)
						{
							_rsslSetError(error, NULL, RSSL_RET_FAILURE, errno);
							snprintf(error->text, MAX_RSSL_ERROR_TEXT,
								"<%s:%d> Error: 1001 Could not allocate compression buffer memory.\n",
								__FILE__, __LINE__);

							return(RIPC_CONN_ERROR);
						}
						rsslSocketChannel->curInputBuf = rtr_smplcDupMsg(gblInputBufs, rsslSocketChannel->decompressBuf);
					}
					else
						rsslSocketChannel->curInputBuf = rtr_smplcDupMsg(gblInputBufs, rsslSocketChannel->inputBuffer);

					if (rsslSocketChannel->curInputBuf == 0)
					{
						_rsslSetError(error, NULL, RSSL_RET_FAILURE, errno);
						snprintf(error->text, MAX_RSSL_ERROR_TEXT,
							"<%s:%d> Error: 1001 Could not allocate compression buffer memory.\n",
							__FILE__, __LINE__);

						return(RIPC_CONN_ERROR);
					}
				}
				break;
				/* Adds in LZ4 compression */
				case CONN_VERSION_12:
				{
					if (((compbitmapsize > 0) && (rsslSocketChannel->srvrcomp)) || (rsslSocketChannel->forcecomp))
					{
						/* take bitmap off wire if its there */
						if (compbitmapsize > 0)
						{
							RsslUInt8 *compbitmap = (RsslUInt8*)&(hdrStart[10]);

							for (i = 0; i <= RSSL_COMP_MAX_TYPE; i++)
							{
								/* if the client is requesting the same compression we want to do, then do it! */
								RsslInt16 idx = ripccompressions[i][RSSL_COMP_BYTEINDEX];
								if ((compbitmap[idx] & ripccompressions[i][RSSL_COMP_BYTEBIT]) != 0 &&
									((ripccompressions[i][RSSL_COMP_TYPE] & rsslSocketChannel->srvrcomp) != 0))
								{
									rsslSocketChannel->outCompression = ripccompressions[i][RSSL_COMP_TYPE];
									break;
								}
							}
						}
						else
						{
							/* if we are going to force compression, force zlib since we know all versions of ripc support this */
							rsslSocketChannel->outCompression = RSSL_COMP_ZLIB;
						}
						if (rsslSocketChannel->outCompression > RSSL_COMP_MAX_TYPE)
							rsslSocketChannel->outCompression = RSSL_COMP_NONE;
						rsslSocketChannel->inDecompress = rsslSocketChannel->outCompression;
						rsslSocketChannel->outCompFuncs = &(compressFuncs[rsslSocketChannel->outCompression]);
						rsslSocketChannel->inDecompFuncs = &(compressFuncs[rsslSocketChannel->inDecompress]);
						if ((rsslSocketChannel->outCompFuncs->compressInit == 0) ||
							(rsslSocketChannel->outCompFuncs->compressEnd == 0) ||
							(rsslSocketChannel->outCompFuncs->compress == 0))
						{
							rsslSocketChannel->outCompression = RSSL_COMP_NONE;
							rsslSocketChannel->outCompFuncs = 0;
						}
						if ((rsslSocketChannel->inDecompFuncs->decompress == 0) ||
							(rsslSocketChannel->inDecompFuncs->decompressEnd == 0) ||
							(rsslSocketChannel->inDecompFuncs->decompressInit == 0))
						{
							rsslSocketChannel->inDecompress = 0;
							rsslSocketChannel->inDecompFuncs = 0;
						}
						if (rsslSocketChannel->outCompFuncs)
						{
								_DEBUG_TRACE_CONN("about to initialize compression = %d\n", rsslSocketChannel->outCompression)
							rsslSocketChannel->c_stream_out = (*(rsslSocketChannel->outCompFuncs->compressInit))(
								rsslSocketChannel->server->zlibCompressionLevel, 0, error);
							if (rsslSocketChannel->c_stream_out == 0)
							{
								_rsslSetError(error, NULL, RSSL_RET_FAILURE, errno);

								return(RIPC_CONN_ERROR);
							}
						}
						if (rsslSocketChannel->inDecompFuncs)
						{
								_DEBUG_TRACE_CONN("about to initialize decompression = %d\n", rsslSocketChannel->inDecompress)
								rsslSocketChannel->c_stream_in = (*(rsslSocketChannel->inDecompFuncs->decompressInit))(0, error);
							if (rsslSocketChannel->c_stream_in == 0)
							{
								_rsslSetError(error, NULL, RSSL_RET_FAILURE, errno);

								return(RIPC_CONN_ERROR);
							}
						}

						/* LZ4 compression can sometimes grow data instead of shrinking it
						* the compression routine isnt smart enough to stop at the end of a buffer (yes, this is hard to believe)
						* so we need to make sure we do not exceed the buffer.
						* so we wont compress buffers that are so full that they might compress past the end of the buffer
						* so we need to calculate the largest buffer that we will compress
						*/
						if (rsslSocketChannel->outCompression == RSSL_COMP_LZ4)
						{
							RsslInt32 i = rsslSocketChannel->maxUserMsgSize;

							while (LZ4_compressBound(i) > rsslSocketChannel->maxUserMsgSize && i > 0)
							{
								i--;
							}
							if (i <= 0)
							{
								snprintf(error->text, MAX_RSSL_ERROR_TEXT,
									"<%s:%d> Error: 1004 Could not calculate upperCompressionThreshold for maxUserMsgSize = %d\n",
									__FILE__, __LINE__, rsslSocketChannel->maxUserMsgSize);

								return(RIPC_CONN_ERROR);
							}
							/* this is the threshold that we will not compress directly into the output buffer, but use the intermediate */
							rsslSocketChannel->upperCompressionThreshold = i;

							/* Create buffer to compress into and decompress from */
							i = LZ4_compressBound(rsslSocketChannel->maxUserMsgSize);
							i += 5; /* add in small fudge factor to ensure these buffers are larger than needed */
							rsslSocketChannel->tempCompressBuf = rtr_smplcAllocMsg(gblInputBufs, i);
							if (rsslSocketChannel->tempCompressBuf == 0)
							{
								_rsslSetError(error, NULL, RSSL_RET_FAILURE, errno);
								snprintf(error->text, MAX_RSSL_ERROR_TEXT,
									"<%s:%d> Error: 1001 Could not allocate buffer memory for LZ compression\n",
									__FILE__, __LINE__);

								return(RIPC_CONN_ERROR);
							}
							rsslSocketChannel->tempDecompressBuf = rtr_smplcAllocMsg(gblInputBufs, i);
							if (rsslSocketChannel->tempDecompressBuf == 0)
							{
								_rsslSetError(error, NULL, RSSL_RET_FAILURE, errno);
								snprintf(error->text, MAX_RSSL_ERROR_TEXT,
									"<%s:%d> Error: 1001 Could not allocate buffer memory for LZ decompression\n",
									__FILE__, __LINE__);

								return(RIPC_CONN_ERROR);
							}
						}
					}
						_DEBUG_TRACE_CONN("rsslSocketChannel->outCompression = %d\n", rsslSocketChannel->outCompression)
					/* Compression both directions */
					if (rsslSocketChannel->inDecompress)
					{
						rsslSocketChannel->decompressBuf = rtr_smplcAllocMsg(gblInputBufs, rsslSocketChannel->maxMsgSize);
						if (rsslSocketChannel->decompressBuf == 0)
						{
							_rsslSetError(error, NULL, RSSL_RET_FAILURE, errno);
							snprintf(error->text, MAX_RSSL_ERROR_TEXT,
								"<%s:%d> Error: 1001 Could not allocate compression buffer memory.\n",
								__FILE__, __LINE__);

							return(RIPC_CONN_ERROR);
						}
						rsslSocketChannel->curInputBuf = rtr_smplcDupMsg(gblInputBufs, rsslSocketChannel->decompressBuf);
					}
					else
						rsslSocketChannel->curInputBuf = rtr_smplcDupMsg(gblInputBufs, rsslSocketChannel->inputBuffer);
					if (rsslSocketChannel->curInputBuf == 0)
					{
						_rsslSetError(error, NULL, RSSL_RET_FAILURE, errno);
						snprintf(error->text, MAX_RSSL_ERROR_TEXT,
							"<%s:%d> Error: 1001 Could not allocate compression buffer memory.\n",
							__FILE__, __LINE__);

						return(RIPC_CONN_ERROR);
					}
				}
				break;
				/* Version 13/14 is similar to 12, but it allows force compression to be whatever server says (V12 forced it as ZLIB always) */
				case CONN_VERSION_13:
				case CONN_VERSION_14:
				{
					if (rsslSocketChannel->srvrcomp)
					{
						if (rsslSocketChannel->forcecomp)
						{
							rsslSocketChannel->outCompression = (RsslCompTypes)rsslSocketChannel->srvrcomp;
						}
						else if (compbitmapsize > 0)
						{
							/* take bitmap off wire (if its there) to see what compression the client wants */
							RsslUInt8 *compbitmap = (RsslUInt8*)&(hdrStart[10]);
							for (i = 0; i <= RSSL_COMP_MAX_TYPE; i++)
							{
								/* check if the client is requesting a compression type we support */
								/* if not, then there will be no compression */
								RsslInt16 idx = ripccompressions[i][RSSL_COMP_BYTEINDEX];
								if ((compbitmap[idx] & ripccompressions[i][RSSL_COMP_BYTEBIT]) != 0 &&
									((ripccompressions[i][RSSL_COMP_TYPE] & rsslSocketChannel->srvrcomp) != 0))
								{
									rsslSocketChannel->outCompression = (RsslCompTypes)ripccompressions[i][RSSL_COMP_TYPE];
									break;
								}
							}
						}
						if (rsslSocketChannel->outCompression > RSSL_COMP_MAX_TYPE)
							rsslSocketChannel->outCompression = RSSL_COMP_NONE;
						rsslSocketChannel->inDecompress = rsslSocketChannel->outCompression;
						rsslSocketChannel->outCompFuncs = &(compressFuncs[rsslSocketChannel->outCompression]);
						rsslSocketChannel->inDecompFuncs = &(compressFuncs[rsslSocketChannel->inDecompress]);
						if ((rsslSocketChannel->outCompFuncs->compressInit == 0) ||
							(rsslSocketChannel->outCompFuncs->compressEnd == 0) ||
							(rsslSocketChannel->outCompFuncs->compress == 0))
						{
							rsslSocketChannel->outCompression = RSSL_COMP_NONE;
							rsslSocketChannel->outCompFuncs = 0;
						}
						if ((rsslSocketChannel->inDecompFuncs->decompress == 0) ||
							(rsslSocketChannel->inDecompFuncs->decompressEnd == 0) ||
							(rsslSocketChannel->inDecompFuncs->decompressInit == 0))
						{
							rsslSocketChannel->inDecompress = 0;
							rsslSocketChannel->inDecompFuncs = 0;
						}
						if (rsslSocketChannel->outCompFuncs)
						{
								_DEBUG_TRACE_CONN("about to initialize compression = %d\n", rsslSocketChannel->outCompression)
							rsslSocketChannel->c_stream_out = (*(rsslSocketChannel->outCompFuncs->compressInit))(
								rsslSocketChannel->server->zlibCompressionLevel, 0, error);
							if (rsslSocketChannel->c_stream_out == 0)
							{
								_rsslSetError(error, NULL, RSSL_RET_FAILURE, errno);

								return(RIPC_CONN_ERROR);
							}
						}
						if (rsslSocketChannel->inDecompFuncs)
						{
								_DEBUG_TRACE_CONN("about to initialize decompression = %d\n", rsslSocketChannel->inDecompress)
								rsslSocketChannel->c_stream_in = (*(rsslSocketChannel->inDecompFuncs->decompressInit))(0, error);
							if (rsslSocketChannel->c_stream_in == 0)
							{
								_rsslSetError(error, NULL, RSSL_RET_FAILURE, errno);

								return(RIPC_CONN_ERROR);
							}
						}

						/* LZ4 compression can sometimes grow data instead of shrinking it
						*  the compression routine isnt smart enough to stop at the end of a buffer (yes, this is hard to believe)
						* so we need to make sure we do not exceed the buffer.
						* so we wont compress buffers that are so full that they might compress past the end of the buffer
						* so we need to calculate the largest buffer that we will compress
						*/
						if (rsslSocketChannel->outCompression == RSSL_COMP_LZ4)
						{
							RsslInt32 i = rsslSocketChannel->maxUserMsgSize;
							while (LZ4_compressBound(i) > rsslSocketChannel->maxUserMsgSize && i > 0)
							{
								i--;
							}
							if (i <= 0)
							{
								_rsslSetError(error, NULL, RSSL_RET_FAILURE, errno);
								snprintf(error->text, MAX_RSSL_ERROR_TEXT,
									"<%s:%d> Error: 1004 Could not calculate upperCompressionThreshold for maxUserMsgSize = %d\n",
									__FILE__, __LINE__, rsslSocketChannel->maxUserMsgSize);

								return(RIPC_CONN_ERROR);
							}
							/* this is the threshold that we will not compress directly into the output buffer, but use the intermediate */
							rsslSocketChannel->upperCompressionThreshold = i;
							/* Create buffer to compress into and decompress from */
							i = LZ4_compressBound(rsslSocketChannel->maxUserMsgSize);
							i += 5; /* add in small fudge factor to ensure these buffers are larger than needed */
							rsslSocketChannel->tempCompressBuf = rtr_smplcAllocMsg(gblInputBufs, i);
							if (rsslSocketChannel->tempCompressBuf == 0)
							{
								_rsslSetError(error, NULL, RSSL_RET_FAILURE, errno);
								snprintf(error->text, MAX_RSSL_ERROR_TEXT,
									"<%s:%d> Error: 1001 Could not allocate buffer memory for LZ compression\n",
									__FILE__, __LINE__);

								return(RIPC_CONN_ERROR);
							}
							rsslSocketChannel->tempDecompressBuf = rtr_smplcAllocMsg(gblInputBufs, i);
							if (rsslSocketChannel->tempDecompressBuf == 0)
							{
								_rsslSetError(error, NULL, RSSL_RET_FAILURE, errno);
								snprintf(error->text, MAX_RSSL_ERROR_TEXT,
									"<%s:%d> Error: 1001 Could not allocate buffer memory for LZ decompression\n",
									__FILE__, __LINE__);
								return(RIPC_CONN_ERROR);
							}
						}
					}
						_DEBUG_TRACE_CONN("rsslSocketChannel->outCompression = %d\n", rsslSocketChannel->outCompression)
					/* Compression both directions */
					if (rsslSocketChannel->inDecompress)
					{
						rsslSocketChannel->decompressBuf = rtr_smplcAllocMsg(gblInputBufs, rsslSocketChannel->maxMsgSize);
						if (rsslSocketChannel->decompressBuf == 0)
						{
							_rsslSetError(error, NULL, RSSL_RET_FAILURE, errno);
							snprintf(error->text, MAX_RSSL_ERROR_TEXT,
								"<%s:%d> Error: 1001 Could not allocate compression buffer memory\n",
								__FILE__, __LINE__);

							return(RIPC_CONN_ERROR);
						}
						rsslSocketChannel->curInputBuf = rtr_smplcDupMsg(gblInputBufs, rsslSocketChannel->decompressBuf);
					}
					else
						rsslSocketChannel->curInputBuf = rtr_smplcDupMsg(gblInputBufs, rsslSocketChannel->inputBuffer);
					if (rsslSocketChannel->curInputBuf == 0)
					{
						_rsslSetError(error, NULL, RSSL_RET_FAILURE, errno);
						snprintf(error->text, MAX_RSSL_ERROR_TEXT,
							"<%s:%d> Error: 1001 Could not allocate compression buffer memory\n",
							__FILE__, __LINE__);

						return(RIPC_CONN_ERROR);
					}
				}
				break;
				default:
					_rsslSetError(error, NULL, RSSL_RET_FAILURE, errno);
					snprintf(error->text, MAX_RSSL_ERROR_TEXT,
						"<%s:%d> Error: 1007 Illegal connection version: (%d).\n",
						__FILE__, __LINE__, dumpConnVersion(version_number));
					return(RIPC_CONN_ERROR);
				}

				/* End of compression section */

				/* get ping stuff */
				pingTimeout = (RsslUInt8)hdrStart[hdrCursor];
				hdrCursor++;

				if (pingTimeout < rsslSocketChannel->minPingTimeout)
				{
					/* use min ping timeout - client wants a timeout that is too small */
					rsslSocketChannel->pingTimeout = rsslSocketChannel->minPingTimeout;
				}
				else if (pingTimeout < (RsslInt32)rsslSocketChannel->pingTimeout)
				{
					/* since its not too small but its smaller than ours, use that */
					rsslSocketChannel->pingTimeout = pingTimeout;
				}

				/* Read out the rssl flags */
				rsslSocketChannel->rsslFlags |= (RsslUInt8)hdrStart[hdrCursor];
				hdrCursor++;

				switch (version_number)
				{
				case CONN_VERSION_10:
				case CONN_VERSION_11:
					/* validate protocolTypes CONN_VERSION_10 and CONN_VERSION_11 must be RWF */
					if (rsslSocketChannel->protocolType != RIPC_RWF_PROTOCOL_TYPE)
					{
						/* cause NAK to be sent */
						rsslSocketChannel->mountNak = 1;
					}
					break;
				case CONN_VERSION_12:
				case CONN_VERSION_13:
				case CONN_VERSION_14:
					protocoltype = (RsslUInt8)hdrStart[hdrCursor];
					hdrCursor++;
					/* validate protocolType */
					if (rsslSocketChannel->protocolType != protocoltype)
					{
						/* cause NAK to be sent */
						rsslSocketChannel->mountNak = 1;
					}
					break;

				default:
					_rsslSetError(error, NULL, RSSL_RET_FAILURE, errno);
					snprintf(error->text, MAX_RSSL_ERROR_TEXT,
						"<%s:%d> Error: 1007 Illegal connection version: (%d).\n",
						__FILE__, __LINE__, dumpConnVersion(version_number));
					return(RIPC_CONN_ERROR);
				}
				major = (RsslUInt8)hdrStart[hdrCursor];
				hdrCursor++;

				minor = (RsslUInt8)hdrStart[hdrCursor];
				hdrCursor++;

				if (major < rsslSocketChannel->majorVersion)
				{
					/* use the smaller version */
					rsslSocketChannel->majorVersion = major;
					rsslSocketChannel->minorVersion = minor;
				}
				else if (major == rsslSocketChannel->majorVersion)
				{
					/* our majors are the same, grab smaller minor */
					if (minor < rsslSocketChannel->minorVersion)
					{
						/* use the smaller version */
						rsslSocketChannel->minorVersion = minor;
					}
				}

				/* else we leave the versions alone as ours are smaller */
				if (rsslSocketChannel->connType == RSSL_CONN_TYPE_INIT)
					rsslSocketChannel->connType = RSSL_CONN_TYPE_SOCKET;

				/* get host name and ip address */
				hostnameLen = (RsslUInt8)hdrStart[hdrCursor];
				hdrCursor++;
				if (hostnameLen > 0)
				{
					/* get the host name here */
					rsslSocketChannel->clientHostname = (char*)_rsslMalloc(hostnameLen + 1);

					MemCopyByInt(rsslSocketChannel->clientHostname, &(hdrStart[hdrCursor]), hostnameLen);

					rsslSocketChannel->clientHostname[hostnameLen] = '\0';
				}
				hdrCursor += hostnameLen;

				addrLen = (RsslUInt8)hdrStart[hdrCursor];
				hdrCursor++;

				if (addrLen > 0)
				{
					/* get the ip address here */
					rsslSocketChannel->clientIP = (char*)_rsslMalloc(addrLen + 1);
					MemCopyByInt(rsslSocketChannel->clientIP, &(hdrStart[hdrCursor]), addrLen);
					rsslSocketChannel->clientIP[addrLen] = '\0';
				}
				hdrCursor += addrLen;

				/* If we are past the right connection version (14 or higher) and client asked for key exchange,
				* set up the internal structure so we know to provide this on the ack and have the extra handshake steps */
				if ((version_number > CONN_VERSION_13) && (flags & RIPC_KEY_EXCHANGE))
				{
					rsslSocketChannel->keyExchange = 1;
					/* this is the only type we support for now.  if we offer others, we should
					* probably allow this to be configurable */
					/* Right now, server sets the type here */
					rsslSocketChannel->encryptionType = TR_SL_1;
				}

				/* Component Versioning Logic - version 13 and higher */
				switch (version_number)
				{
				case CONN_VERSION_13:
				case CONN_VERSION_14:
				{
					/* get component versioning stuff */
					/* rb15 total length followed by rb15 length specified product version string */
					/* having both lengths lets us add component versioning elements in the future without adding new connection handshakes */
					/* get component versioning info */
					/* initialize iterator */
					tempIter = hdrCursor;
					/* get overall length of component versioning */
					tempIter += rwfGet8(componentVersionLen, (hdrStart + tempIter));
					compVerLen = componentVersionLen;
					if (componentVersionLen > 0)
					{
						/* get version string length */
						tempIter += rwfGet8(componentStringLen, (hdrStart + tempIter));
						compStringLen = componentStringLen;
						/* write version string */
						if (componentStringLen > 0)
						{
							/* cant put this in the same place we got it from  because ack isnt sent until after this */
							/* get the version string - with ripc, there will only ever be one */
							rsslSocketChannel->outComponentVer = (char*)_rsslMalloc(compStringLen + 1);
							/* copy string here */
							MemCopyByInt(rsslSocketChannel->outComponentVer, (hdrStart + tempIter), componentStringLen);
							rsslSocketChannel->outComponentVer[componentStringLen] = '\0';
							rsslSocketChannel->outComponentVerLen = componentStringLen;
							tempIter += componentStringLen;
						}
					}
					/* in case we need it later, update tempIter to skip over full componentVersionLen (in case we add other things that this version of code is unaware of) */
					hdrCursor += compVerLen;
				}
				break;
				default:
					/* fall through, no stoppage */
					;
				}
				/* End of component versioning logic */

				discoveredVersion = 1;
				switch (version_number)
				{
				case CONN_VERSION_10:
					/* Nothing else to do for version 10 */
					rsslSocketChannel->version = &ripc10Ver;
					break;
				case CONN_VERSION_11:
					rsslSocketChannel->version = &ripc11Ver;
					/* there should not be any 10 versions that can do tunneling */
					if (rsslSocketChannel->httpHeaders)
					{
						/* we need to add to the header and footer length */
						/* 6 bytes is the max length we can represent as a chunk */
						/* need to reserve footer for carrage return -line feed */
						rsslSocketChannel->version = &ripc11WinInetVer;
						rsslSocketChannel->maxUserMsgSize -= 8;
					}
					break;
				case CONN_VERSION_12:
					rsslSocketChannel->version = &ripc12Ver;
					/* there should not be any 10 versions that can do tunneling */
					if (rsslSocketChannel->httpHeaders)
					{
						/* we need to add to the header and footer length */
						/* 6 butes is the max length we can represent as a chunk */
						/* need to reserve footer for carrage return -line feed */
						rsslSocketChannel->version = &ripc12WinInetVer;
						rsslSocketChannel->maxUserMsgSize -= 8;
					}
					break;
				case CONN_VERSION_13:
					rsslSocketChannel->version = &ripc13Ver;
					/* there should not be any 10 versions that can do tunneling */
					if (rsslSocketChannel->httpHeaders)
					{
						/* we need to add to the header and footer length */
						/* 6 butes is the max length we can represent as a chunk */
						/* need to reserve footer for carrage return -line feed */
						rsslSocketChannel->version = &ripc13WinInetVer;
						rsslSocketChannel->maxUserMsgSize -= 8;
					}
					break;
				case CONN_VERSION_14:
					rsslSocketChannel->version = &ripc14Ver;
					/* there should not be any 10 versions that can do tunneling */
					if (rsslSocketChannel->httpHeaders)
					{
						/* we need to add to the header and footer length */
						/* 6 butes is the max length we can represent as a chunk */
						/* need to reserve footer for carrage return -line feed */
						rsslSocketChannel->version = &ripc14WinInetVer;
						rsslSocketChannel->maxUserMsgSize -= 8;
					}
					break;

				default:
					_rsslSetError(error, NULL, RSSL_RET_FAILURE, errno);
					snprintf(error->text, MAX_RSSL_ERROR_TEXT,
						"<%s:%d> Error: 1007 Illegal connection version: (%d).\n",
						__FILE__, __LINE__, dumpConnVersion(version_number));
					return(RIPC_CONN_ERROR);
				}
			}
		}
	}
	break;

	default:
	{
		/* this handles the first tunneling message and determines that the session
		has HTTP headers on the data */
		RsslUInt16 hdrLen;
		RsslInt32 rwflags = 0;
		RsslInt32 outLen = 0;
		char outBuf[1024];
		RsslInt32 httpHeaderLen = 0;
		RsslInt32 headerCursor = 0;
		RsslUInt16 pID = 0;
		RsslUInt32 sessionID = 0;
		RsslUInt32 ipAddr = 0;
		char outFlags = 0;
		RsslInt32 ackLen;

		if ((hdrStart[0] == 'G') && (hdrStart[1] == 'E') && (hdrStart[2] == 'T'))
		{
			/* Process the WS client handshake  */
			return (rwsValidateWebSocketRequest(rsslSocketChannel, hdrStart, cc, error));

		}
		else if ((hdrStart[0] == 'P') && (hdrStart[1] == 'O') && 
		(hdrStart[2] == 'S') && (hdrStart[3] == 'T'))
		{
			/* this is the HTTP version of the request - strip off the HTTP header and then
			parse the tunnel header */

			httpHeaderLen = ipcHttpHdrComplete(hdrStart, cc, 0);

			/* move past header length */
			headerCursor += httpHeaderLen;
		}
		else if (rsslSocketChannel->inputBufCursor == 0)
		{
			/* some error here */
			_rsslSetError(error, NULL, RSSL_RET_FAILURE, errno);
			snprintf(error->text, MAX_RSSL_ERROR_TEXT,
				"<%s:%d> Error: 1007 Unknown connection type.\n",
				__FILE__, __LINE__);

			return(RIPC_CONN_ERROR);
		}

		if (headerCursor < cc)
		{
			/* we know we have tunneling header here - parse it */
			_move_u16_swap(&hdrLen, (hdrStart + headerCursor));
			headerCursor += 2;
			opCode = hdrStart[headerCursor];
			++headerCursor;
			_move_u32_swap(&sessionID, (hdrStart + headerCursor));
			headerCursor += 4;
			_move_u16_swap(&pID, (hdrStart + headerCursor));
			headerCursor += 2;
			_move_u32_swap(&ipAddr, (hdrStart + headerCursor));
			headerCursor += 4;

			if (opCode & RIPC_WININET_TUNNELING)
			{
				rsslSocketChannel->connType = RSSL_CONN_TYPE_HTTP;
				rsslSocketChannel->httpHeaders = 1;
			}
			else if (opCode & RIPC_JAVA_WITH_HTTP_TUNNELING)
			{
				rsslSocketChannel->connType = RSSL_CONN_TYPE_HTTP;
				rsslSocketChannel->httpHeaders = 1;
				rsslSocketChannel->isJavaTunnel = 1;
			}
			else
			{
				/* this is OpensSL */
				rsslSocketChannel->connType = RSSL_CONN_TYPE_ENCRYPTED;
			}

			rsslSocketChannel->inputBufCursor += headerCursor;
		}
		else
		{
			/* increment by the size we parsed over */
			/* this should be the case where we see the new connection
			from the control channel - we just have to wait at this point */
			if ((headerCursor == cc))
			{
				rsslSocketChannel->inputBufCursor += headerCursor;
				return RIPC_CONN_IN_PROGRESS;
			}
			else
			{
				/* some error here */
				_rsslSetError(error, NULL, RSSL_RET_FAILURE, errno);
				snprintf(error->text, MAX_RSSL_ERROR_TEXT,
					"<%s:%d> Error: 1007 Invalid mount request size <%d>\n",
					__FILE__, __LINE__, cc);

				return(RIPC_CONN_ERROR);
			}
		}

		if (sessionID)
		{
			/* lookup session based on ID, then compare pID and ipAddr */
			if (opCode & RIPC_TUNNEL_CONTROL)
			{
				RsslSocketChannel* oldSession = 0;
				/* make sure this is the first control channel establishment.  If we are already done with this
				once do not do it again */
				/* this is for later use during the reconnection process */
				if (multiThread)
				{
						(void) RSSL_MUTEX_LOCK(&ripcMutex);
						_DEBUG_MUTEX_TRACE("RSSL_MUTEX_LOCK", NULL, &ripcMutex)
				}

				for (pLink = rsslQueuePeekFront(&activeSocketChannelList);
					pLink != 0;
					pLink = rsslQueuePeekNext(&activeSocketChannelList, pLink))
				{
					oldSession = RSSL_QUEUE_LINK_TO_OBJECT(RsslSocketChannel, link1, pLink);
					if ((oldSession->sessionID == sessionID) && (oldSession->pID == pID) && (oldSession->ipAddress == ipAddr))
					{
						/* we found the old session */
						break;
					}
				}

				if (multiThread)
				{
				  (void) RSSL_MUTEX_UNLOCK(&ripcMutex);
						_DEBUG_MUTEX_TRACE("RSSL_MUTEX_UNLOCK", NULL, &ripcMutex)
				}

				if (!oldSession)
				{
					/* the old session is gone - this connection is bad */
					/* some error here */
					_rsslSetError(error, NULL, RSSL_RET_FAILURE, errno);
					snprintf(error->text, MAX_RSSL_ERROR_TEXT,
						"<%s:%d> Error: 1007 Unable to complete tunneling connection successfully.\n",
						__FILE__, __LINE__);

					return(RIPC_CONN_ERROR);
				}

				if (oldSession->tunnelingState == RIPC_TUNNEL_INIT)
				{
					/* this is the first establishment of this connection - set up the pipe, move the streaming FD here */
					/* we moved the actual FD to here before we switched it on the app */
					rsslSocketChannel->tunnelTransportInfo = oldSession->tunnelTransportInfo;
					/* flip the sessionIDs */
					oldSession->sessionID = rsslSocketChannel->sessionID;
					rsslSocketChannel->sessionID = sessionID;

					/* move the IP address and PID */
					rsslSocketChannel->ipAddress = oldSession->ipAddress;
					rsslSocketChannel->pID = oldSession->pID;

					/* set this so we know not to close the fd when they shutdown the old session */
					oldSession->tunnelingState = RIPC_TUNNEL_REMOVE_SESSION;
					/* signal on the old pipe so they will call init channel */
					/* this will signal the application - they will call InitChannel, this will result in error
					and cleaning up old session */
					rssl_pipe_write(&oldSession->sessPipe, "1", 1);

					if (!(rssl_pipe_create(&rsslSocketChannel->sessPipe)))
					{
						/* some error here */
						_rsslSetError(error, NULL, RSSL_RET_FAILURE, errno);
						snprintf(error->text, MAX_RSSL_ERROR_TEXT,
							"<%s:%d> Error: 1002 Unable to create internal pipe on connection request.\n",
							__FILE__, __LINE__);

						return(RIPC_CONN_ERROR);
					}

					/* tunnel handshake is essentially done */
					rsslSocketChannel->tunnelingState = RIPC_TUNNEL_ACTIVE;

					/* could be a ripc handshake or a new open request - check session state to determine */
					/* if we have more bytes to read, we probably already have the ripc connection request -
					just call this function again after moving the inputBufCursor */
					/* if we do not have more bytes to read, we may not have gotten it yet, or this may be a reconnection */
					if (headerCursor < cc)
						return ipcProcessHdr(rsslSocketChannel, inProg, error, cc - headerCursor);
					else
						return RIPC_CONN_IN_PROGRESS;
				}
				else
				{
					/* this could be a reconnection request for the control channel */
					if ((opCode & RIPC_TUNNEL_RECONNECT) && (oldSession->tunnelingState == RIPC_TUNNEL_ACTIVE))
					{
						/* in this case, put the newFD into the oldSession and close the new one */
						/* no need for an ack - in Read and write we will watch for closed socket return codes
						and if we have a newstream we will intercept it */
						oldSession->newStream = rsslSocketChannel->stream;
						oldSession->newTransportInfo = rsslSocketChannel->transportInfo;
						/* we should be able to send ack on newStreaming channel first */
						if (oldSession->newTunnelTransportInfo)
						{
							/* send 3 - this is the ack for the control channel */
							outLen = sprintf(outBuf, "%x\r\n", 1);
							outBuf[outLen++] = (char)3;
							outLen += sprintf((outBuf + outLen), "\r\n");
							/* there should be nothing going out on this yet */
							(*(oldSession->transportFuncs->writeTransport))(oldSession->newTunnelTransportInfo, outBuf, outLen, rwflags, error);
							/* if it doesnt write it all we are in a bad state anyhow */
							oldSession->sentControlAck = 1;
						}

						/* now remove this session, but leave the fd */
						rsslSocketChannel->tunnelingState = RIPC_TUNNEL_REMOVE_SESSION;
						_rsslSetError(error, NULL, RSSL_RET_FAILURE, errno);
						snprintf(error->text, MAX_RSSL_ERROR_TEXT,
							"<%s:%d> Error: 1006 Not a new connection request - keeping alive tunneling connection.\n",
							__FILE__, __LINE__);

						return(RIPC_CONN_ERROR);
					}
					else
					{
						/* not a valid message */
						_rsslSetError(error, NULL, RSSL_RET_FAILURE, errno);
						snprintf(error->text, MAX_RSSL_ERROR_TEXT,
							"<%s:%d> Error: 1007 Invalid tunneling reconnection request on control channel.\n",
							__FILE__, __LINE__);

						return (RIPC_CONN_ERROR);
					}
				}
			}
			else if (opCode & RIPC_JAVA_TUNNEL_RECONNECT) /* reconnect request for Java single tunnel connection */
			{
				RsslSocketChannel* oldSession = 0;

				/* this is for later use during the reconnection process */
				if (multiThread)
				{
				  (void) RSSL_MUTEX_LOCK(&ripcMutex);
						_DEBUG_MUTEX_TRACE("RSSL_MUTEX_LOCK", NULL, &ripcMutex)
				}

				for (pLink = rsslQueuePeekFront(&activeSocketChannelList);
					pLink != 0;
					pLink = rsslQueuePeekNext(&activeSocketChannelList, pLink))
				{
					oldSession = RSSL_QUEUE_LINK_TO_OBJECT(RsslSocketChannel, link1, pLink);
					if ((oldSession->sessionID == sessionID) && (oldSession->pID == pID) && (oldSession->ipAddress == ipAddr))
					{
						/* we found the old session */
						break;
					}
				}

				if (multiThread)
				{
				  (void) RSSL_MUTEX_UNLOCK(&ripcMutex);
						_DEBUG_MUTEX_TRACE("RSSL_MUTEX_UNLOCK", NULL, &ripcMutex)
				}

				if (!oldSession)
				{
					/* the old session is gone - this connection is bad */
					/* some error here */
					_rsslSetError(error, NULL, RSSL_RET_FAILURE, errno);
					snprintf(error->text, MAX_RSSL_ERROR_TEXT,
						"<%s:%d> Error: 1007 Unable to complete tunneling connection successfully.\n",
						__FILE__, __LINE__);

					return(RIPC_CONN_ERROR);
				}

				if (oldSession->tunnelingState == RIPC_TUNNEL_ACTIVE)
				{
					/* 1st */
					oldSession->newTunnelTransportInfo = rsslSocketChannel->transportInfo;

					/* 2nd */
					oldSession->newStream = rsslSocketChannel->stream;
					/* we should be able to send ack on newStreaming channel first */
					if (oldSession->newTunnelTransportInfo)
					{
						/* send 3 - this is the ack for the new channel */
						outLen = sprintf(outBuf, "%x\r\n", 1);
						outBuf[outLen++] = (char)3;
						outLen += sprintf((outBuf + outLen), "\r\n");
						/* there should be nothing going out on this yet */
						(*(oldSession->transportFuncs->writeTransport))(oldSession->newTunnelTransportInfo, outBuf, outLen, rwflags, error);
						/* if it doesnt write it all we are in a bad state anyhow */
						oldSession->sentControlAck = 1;
					}
					/* now remove this session, but leave the fd */
					rsslSocketChannel->tunnelingState = RIPC_TUNNEL_REMOVE_SESSION;

					_rsslSetError(error, NULL, RSSL_RET_FAILURE, errno);
					snprintf(error->text, MAX_RSSL_ERROR_TEXT,
						"<%s:%d> Error: 1006 Not a new connection request - keeping alive tunneling connection.\n",
						__FILE__, __LINE__);

					return(RIPC_CONN_ERROR);
				}
			}
			else
			{
				/* attempt to establish a new streaming channel for an existing session - find it and set it up */
				RsslSocketChannel* oldSession = 0;
				if (multiThread)
				{
				  (void) RSSL_MUTEX_LOCK(&ripcMutex);
						_DEBUG_MUTEX_TRACE("RSSL_MUTEX_LOCK", NULL, &ripcMutex)
				}

				for (pLink = rsslQueuePeekFront(&activeSocketChannelList);
					pLink != 0;
					pLink = rsslQueuePeekNext(&activeSocketChannelList, pLink))
				{
					oldSession = RSSL_QUEUE_LINK_TO_OBJECT(RsslSocketChannel, link1, pLink);
					if ((oldSession->sessionID == sessionID) && (oldSession->pID == pID) && (oldSession->ipAddress == ipAddr))
						break;
				}

				if (multiThread)
				{
				  (void) RSSL_MUTEX_UNLOCK(&ripcMutex);
						_DEBUG_MUTEX_TRACE("RSSL_MUTEX_UNLOCK", NULL, &ripcMutex)
				}

				if (!oldSession)
				{
					/* the old session is gone - this connection is bad */
					/* some error here */
					_rsslSetError(error, NULL, RSSL_RET_FAILURE, errno);
					snprintf(error->text, MAX_RSSL_ERROR_TEXT,
						"<%s:%d> Error: 1007 Unable to complete tunneling reconnection successfully.\n",
						__FILE__, __LINE__);

					return(RIPC_CONN_ERROR);
				}

				if ((opCode & RIPC_TUNNEL_RECONNECT) && (oldSession->tunnelingState == RIPC_TUNNEL_ACTIVE))
				{
					/* in this case, put the newFD into the oldSession and close the new one */
					/* no need for an ack - in Read and write we will watch for closed socket return codes
					and if we have a newstream we will intercept it */

					/* send out the chunked header first if we need it - if we do not need it
					that should mean we are openSSL tunneling */

					if (rsslSocketChannel->httpHeaders)
					{
						outLen = snprintf(outBuf, 1024, "%s", "HTTP/1.1 200 OK\r\n");
						outLen += snprintf((outBuf + outLen), (1024 - outLen), "%s", "Transfer-Encoding: chunked\r\n");
						outLen += snprintf((outBuf + outLen), (1024 - outLen), "%s", "Content-Type: application/octet-stream\r\n");
						outLen += snprintf((outBuf + outLen), (1024 - outLen), "%s", "\r\n");
						outLen += snprintf((outBuf + outLen), (1024 - outLen), "%x\r\n", 1);  /* need to convert ackLen to hex first */
						outBuf[outLen++] = (char)1;
						outLen += snprintf((outBuf + outLen), (1024 - outLen), "%s", "\r\n");

						(*(rsslSocketChannel->transportFuncs->writeTransport))(((void*)(intptr_t)rsslSocketChannel->transportInfo), outBuf, outLen, rwflags, error);
					}

					/* if we are OpenSSL tunneling, this is just one stream */
					/* if not, it's WinInet so we have two streams */
					if (opCode & RIPC_OPENSSL_TUNNELING)
					{
						oldSession->newStream = rsslSocketChannel->stream;
						oldSession->newTransportInfo = rsslSocketChannel->transportInfo;
					}
					else if (opCode & RIPC_WININET_TUNNELING)
						oldSession->newTunnelTransportInfo = (void*)rsslSocketChannel->transportInfo;
					else
					{
						/* there is a problem */
						_rsslSetError(error, NULL, RSSL_RET_FAILURE, errno);
						snprintf(error->text, MAX_RSSL_ERROR_TEXT,
							"<%s:%d> Error: 1007 Invalid tunneling reconnection request <%d>\n",
							__FILE__, __LINE__, cc);

						return (RIPC_CONN_ERROR);
					}

					/* now remove this session, but leave the fd */
					rsslSocketChannel->tunnelingState = RIPC_TUNNEL_REMOVE_SESSION;

					_rsslSetError(error, NULL, RSSL_RET_FAILURE, errno);
					snprintf(error->text, MAX_RSSL_ERROR_TEXT,
						"<%s:%d> Error: 1006 Not a new connection request - keeping alive tunneling connection.\n",
						__FILE__, __LINE__);

					return(RIPC_CONN_ERROR);
				}
				else
				{
					/* not a valid message */
					_rsslSetError(error, NULL, RSSL_RET_FAILURE, errno);
					snprintf(error->text, MAX_RSSL_ERROR_TEXT,
						"<%s:%d> Error: 1007 Invalid tunneling reconnection request on streaming channel.\n",
						__FILE__, __LINE__);

					return (RIPC_CONN_ERROR);
				}
			}
		}
		else
		{
			if (opCode == RIPC_JAVA_WITH_HTTP_TUNNELING)
			{
				rsslSocketChannel->ipAddress = ipAddr;
				rsslSocketChannel->pID = pID;

				rsslSocketChannel->tunnelingState = RIPC_TUNNEL_ACTIVE;

				rsslSocketChannel->tunnelTransportInfo = rsslSocketChannel->transportInfo;

				if (rsslSocketChannel->httpHeaders)
				{
					ackLen = 7;

					/* start the chunking */
					outLen = snprintf(outBuf, 1024, "%s", "HTTP/1.1 200 OK\r\n");
					outLen += snprintf((outBuf + outLen), (1024 - outLen), "%s", "Transfer-Encoding: chunked\r\n");
					outLen += snprintf(outBuf + outLen, (1024 - outLen), "%s", "Content-Type: application/octet-stream\r\n");
					outLen += snprintf((outBuf + outLen), (1024 - outLen), "%s", "\r\n");
					outLen += snprintf((outBuf + outLen), (1024 - outLen), "%x\r\n", ackLen);  /* need to convert ackLen to hex first */
				}
				/* encode the ack message for tunneling */
				hdrLen = 7;
				_move_u16_swap((outBuf + outLen), &hdrLen);
				outLen += 2;
				outBuf[outLen] = outFlags;
				++outLen;
				/* Send actual sessionID from the session */
				_move_u32_swap((outBuf + outLen), &rsslSocketChannel->sessionID);
				outLen += 4;
				/* apply trailer to end of actual data */
				if (rsslSocketChannel->httpHeaders)
					outLen += sprintf(outBuf + outLen, "%s", "\r\n");
				if (rsslSocketChannel->httpHeaders)
					(*(rsslSocketChannel->transportFuncs->writeTransport))(rsslSocketChannel->tunnelTransportInfo, outBuf, outLen, rwflags, error);
				else
					(*(rsslSocketChannel->transportFuncs->writeTransport))(rsslSocketChannel->transportInfo, outBuf, outLen, rwflags, error);

				return RIPC_CONN_IN_PROGRESS;
			}

			if (opCode & RIPC_TUNNEL_CONTROL)
			{
				/* error */
				_rsslSetError(error, NULL, RSSL_RET_FAILURE, errno);
				snprintf(error->text, MAX_RSSL_ERROR_TEXT,
					"<%s:%d> Error: 1007 Invalid connection request <%d>\n",
					__FILE__, __LINE__, cc);

				return(RIPC_CONN_ERROR);
			}

			rsslSocketChannel->ipAddress = ipAddr;
			rsslSocketChannel->pID = pID;

			rsslSocketChannel->tunnelingState = RIPC_TUNNEL_INIT;


			/* this is the tunnel connect request - put together the ack and send it out.  */
			/* initialize the pipe as we will need it later. */
			/* most of this only has to be done for WinInet */
			if (opCode & RIPC_WININET_TUNNELING)
			{
				if (!(rssl_pipe_create(&rsslSocketChannel->sessPipe)))
				{
					/* some error here */
					_rsslSetError(error, NULL, RSSL_RET_FAILURE, errno);
					snprintf(error->text, MAX_RSSL_ERROR_TEXT,
						"<%s:%d> Error: 1002 Unable to create internal pipe on connection request.\n",
						__FILE__, __LINE__);

					return(RIPC_CONN_ERROR);
				}

				/* flip the FD to be the pipe so when we signal they see it */
				inProg->types = RIPC_INPROG_NEW_FD;
				inProg->oldSocket = rsslSocketChannel->stream;
				rsslSocketChannel->tunnelTransportInfo = rsslSocketChannel->transportInfo;
				/* switch the stream with the pipe fd */
				rsslSocketChannel->stream = rssl_pipe_get_read_fd(&rsslSocketChannel->sessPipe);

				_rsslSocketChannelToIpcSocket(&(inProg->newSocket), rsslSocketChannel);
			}

			if (rsslSocketChannel->httpHeaders)
			{
				ackLen = 7;

				/* start the chunking! */
				outLen = snprintf(outBuf, 1024, "%s", "HTTP/1.1 200 OK\r\n");
				outLen += snprintf((outBuf + outLen), (1024 - outLen), "%s", "Transfer-Encoding: chunked\r\n");
				outLen += snprintf(outBuf + outLen, (1024 - outLen), "%s", "Content-Type: application/octet-stream\r\n");
				outLen += snprintf((outBuf + outLen), (1024 - outLen), "%s", "\r\n");
				outLen += snprintf((outBuf + outLen), (1024 - outLen), "%x\r\n", ackLen);  /* need to convert ackLen to hex first */
			}
			/* encode the ack message for tunneling */
			hdrLen = 7;
			_move_u16_swap((outBuf + outLen), &hdrLen);
			outLen += 2;
			outBuf[outLen] = outFlags;
			++outLen;
			/* Send actual sessionID from the session */
			_move_u32_swap((outBuf + outLen), &rsslSocketChannel->sessionID);
			outLen += 4;

			/* apply trailer to end of actual data */
			if (rsslSocketChannel->httpHeaders)
				outLen += sprintf(outBuf + outLen, "%s", "\r\n");

			if (rsslSocketChannel->httpHeaders)
				(*(rsslSocketChannel->transportFuncs->writeTransport))(rsslSocketChannel->tunnelTransportInfo, outBuf, outLen, rwflags, error);
			else
				(*(rsslSocketChannel->transportFuncs->writeTransport))(rsslSocketChannel->transportInfo, outBuf, outLen, rwflags, error);

			return RIPC_CONN_IN_PROGRESS;
		}
	}
	}

	if (!discoveredVersion)
	{
		_rsslSetError(error, NULL, RSSL_RET_FAILURE, errno);
		snprintf(error->text, MAX_RSSL_ERROR_TEXT,
			"<%s:%d> Error: 1007 Invalid mount request version 0x%x\n",
			__FILE__, __LINE__, version_number);

		return(RIPC_CONN_ERROR);
	}

	/* Reset the values since the input buffer is used for reading the header.
	* This is extremely important for tunelling since the cursor is used.
	*/
	rsslSocketChannel->inputBuffer->length = 0;
	rsslSocketChannel->inputBufCursor = 0;

	_DEBUG_TRACE_CONN("Version is ConnVer 0x%x comp <%d>\n", version_number, rsslSocketChannel->outCompression)

	rsslSocketChannel->guarBufPool = rtr_dfltcAllocatePool(
		rsslSocketChannel->server->maxGuarMsgs, rsslSocketChannel->server->maxGuarMsgs,
		10, rsslSocketChannel->maxMsgSize, rsslSocketChannel->server->sharedBufPool,
		(rsslSocketChannel->server->maxNumMsgs - rsslSocketChannel->server->maxGuarMsgs), 0);

	if (rsslSocketChannel->guarBufPool == 0)
	{
		_rsslSetError(error, NULL, RSSL_RET_FAILURE, errno);
		snprintf(error->text, MAX_RSSL_ERROR_TEXT,
			"<%s:%d> Error: 1001 Could not allocate memory for channel's output buffer pool.\n",
			__FILE__, __LINE__);

		return(RIPC_CONN_ERROR);
	}

	_DEBUG_TRACE_CONN("NOT doing renegotiation "SOCKET_PRINT_TYPE"\n", rsslSocketChannel->stream)

	opts.code = RIPC_SOPT_LINGER;
	opts.options.linger_time = 0;
	if (((*(rsslSocketChannel->transportFuncs->setSockOpts))(rsslSocketChannel->stream, &opts,
															rsslSocketChannel->transportInfo)) < 0)
	{
		_rsslSetError(error, NULL, RSSL_RET_FAILURE, errno);
		snprintf(error->text, MAX_RSSL_ERROR_TEXT,
			"<%s:%d> Error: 1002 Could not set SO_LINGER:(%d) on socket. System errno: (%d)\n",
			__FILE__, __LINE__, opts.options.linger_time, errno);

		return(RIPC_CONN_ERROR);
	}

	rsslSocketChannel->intState = RIPC_INT_ST_COMPLETE;
	return(ipcFinishSess(rsslSocketChannel, inProg, error));
}

static ripcSessInit ipcRejectSession(RsslSocketChannel *rsslSocketChannel, RsslUInt16 err, RsslError *error)
{
	char			conMsg[V10_MIN_CONN_HDR + MAX_RSSL_ERROR_TEXT + 1];
	rtr_msgb_t		*cMsg = 0;
	RsslUInt32		ipcMsgLength = (RsslUInt32)V10_MIN_CONN_HDR;
	ripcRWFlags		rwflags = RIPC_RW_WAITALL;
	RsslUInt32		chunkLength = 0;
	RsslUInt32		iterator = 0;

	rwflags |= (rsslSocketChannel->blocking ? RIPC_RW_BLOCKING : 0);

	_DEBUG_TRACE_CONN("called\n")

	RIPC_ASSERT(rsslSocketChannel->intState == RIPC_INT_ST_COMPLETE);

	{
		RsslUInt16	newIpcMsgLength = 0;
		RsslUInt16	textLength = 0;
		RsslUInt8	headerLength = IPC_100_OTHER_HEADER_SIZE;
		RsslUInt8 	flags = 0;

		/* sending the null on wire because this is a one time hit,
		and client side isnt adding null terminator.  This will fix it for
		clients and servers */

		/* since the text isnt changing, no need to do strlen */
		/*strlen(error->text); */
		textLength = 20;

		newIpcMsgLength = headerLength + textLength;

		/* if tunneling with winInet */
		if (rsslSocketChannel->httpHeaders)
		{
			chunkLength = sprintf((conMsg + iterator), "%x\r\n", newIpcMsgLength);
			iterator += chunkLength;
		}

		flags |= IPC_EXTENDED_FLAGS;

		_move_u16_swap((conMsg + iterator), &newIpcMsgLength);	/* hdr.IPC_len */
		iterator += 2;
		conMsg[iterator++] = (char)flags;
		conMsg[iterator++] = (char)ipc10Connnak;		/* hdr.IPC_opcode */
		conMsg[iterator++] = (char)headerLength;		/* hdr length */
		conMsg[iterator++] = (char)0;					/* hdr.IPC_BITMAP */
		_move_u16_swap((conMsg + iterator), &textLength);	/* error text length */
		iterator += 2;

		MemCopyByInt((conMsg + iterator), error->text, textLength);
		iterator += textLength;

		/* apply trailer to end of actual data */
		if (rsslSocketChannel->httpHeaders)
			chunkLength += sprintf((conMsg + iterator), "%s", "\r\n");

		ipcMsgLength = (RsslUInt32)newIpcMsgLength + chunkLength;
	}

	cMsg = (*(rsslSocketChannel->protocolFuncs->getGlobalBuffer))( ipcMsgLength);
	MemCopyByInt(cMsg->buffer, conMsg, ipcMsgLength);
	cMsg->length = ipcMsgLength;
	/* Set/populate the prefix protocol header if one exists */
	(*(rsslSocketChannel->protocolFuncs->prependTransportHdr))((void*)rsslSocketChannel, cMsg, error);

	if (rsslSocketChannel->dbgFlags & RSSL_DEBUG_IPC_DUMP_INIT)
	{
		(*(ripcDumpOutFunc))(__FUNCTION__, cMsg->buffer, (RsslUInt32)cMsg->length, rsslSocketChannel->stream);
	}

	IPC_MUTEX_UNLOCK(rsslSocketChannel);

	if (rsslSocketChannel->httpHeaders)
		(*(rsslSocketChannel->transportFuncs->writeTransport))(rsslSocketChannel->tunnelTransportInfo, (char*)cMsg->buffer, (RsslInt32)cMsg->length, rwflags, error);
	else
		(*(rsslSocketChannel->transportFuncs->writeTransport))(rsslSocketChannel->transportInfo, cMsg->buffer, (RsslInt32)cMsg->length, rwflags, error);

	IPC_MUTEX_LOCK(rsslSocketChannel);

	if (cMsg) rtr_smplcFreeMsg(cMsg);

	if (rsslSocketChannel->workState & RIPC_INT_SHTDOWN_PEND)
	{
		_rsslSetError(error, NULL, RSSL_RET_FAILURE, errno);
		snprintf(error->text, MAX_RSSL_ERROR_TEXT,
			"<%s:%d> Error: 1003 ipcRejectSession() failed due to channel shutting down.\n",
			__FILE__, __LINE__);

		return(RIPC_CONN_ERROR);
	}

	snprintf(error->text, MAX_RSSL_ERROR_TEXT,
		"<%s:%d> Error: 1006 ipcRejectSession() Connection refused.\n",
		__FILE__, __LINE__);

	/* shutdown the socket without calling SessFail */

	error->sysError = 0;
	error->rsslErrorId = RSSL_RET_FAILURE;

	if (rsslSocketChannel->newStream != RIPC_INVALID_SOCKET)
	{
		if (!(rsslSocketChannel->workState & RIPC_INT_SOCK_CLOSED))
		{
			if(rsslSocketChannel->connType == RSSL_CONN_TYPE_ENCRYPTED)
				ripcShutdownSSLSocket(rsslSocketChannel->tunnelTransportInfo);
			else
				shutdown(((RsslSocket)(intptr_t)rsslSocketChannel->tunnelTransportInfo), shutdownFlag);

			/* we used to make the socket invalid here, but the FDs werent always cleaned up properly */
			/* if we do not change the state, they shoudl get a notification and this will cause them
			to call a function that will then go ahead and properly close and clean up resources */
		}
	}
	else if (rsslSocketChannel->stream != RIPC_INVALID_SOCKET)
	{
		if (!(rsslSocketChannel->workState & RIPC_INT_SOCK_CLOSED))
		{
			if(rsslSocketChannel->connType == RSSL_CONN_TYPE_ENCRYPTED)
				ripcShutdownSSLSocket(rsslSocketChannel->tunnelTransportInfo);
			else
				shutdown((RsslSocket)(intptr_t)(rsslSocketChannel->tunnelTransportInfo), shutdownFlag);
			/* we used to make the socket invalid here, but the FDs weren't always cleaned up properly */
		}
	}

	if (!(rsslSocketChannel->workState & RIPC_INT_SHTDOWN_PEND))
		rsslSocketChannel->workState |= RIPC_INT_SHTDOWN_PEND;

	if (rsslSocketChannel->state != RSSL_CH_STATE_INACTIVE)
		rsslSocketChannel->state = RSSL_CH_STATE_INACTIVE;

	/* end of shutting down code */

	return RSSL_RET_SUCCESS;
}

static ripcSessInit ipcFinishSess(RsslSocketChannel *rsslSocketChannel, ripcSessInProg *inPr, RsslError *error)
{
	char			conMsg[1024];
	rtr_msgb_t		*cMsg = 0;
	RsslRet			retval;
	RsslUInt32		ipcMsgLength = V10_MIN_CONN_HDR;
	ripcRWFlags		rwflags = RIPC_RW_WAITALL;
	RsslUInt16		maxMsgSize = (RsslUInt16)rsslSocketChannel->maxUserMsgSize;
	RsslUInt16		newIpcMsgLength;
	RsslUInt32		versionNumber = (RsslUInt32)rsslSocketChannel->version->ipcVersion;
	RsslUInt8		headerLength;
	RsslUInt8 		flags = 0;
	RsslUInt32		chunkLength = 0;
	RsslUInt8		iterator = 0;
	RsslUInt8		componentVersionLength;

	_DEBUG_TRACE_CONN("fd "SOCKET_PRINT_TYPE"\n", rsslSocketChannel->stream)

	rwflags |= (rsslSocketChannel->blocking ? RIPC_RW_BLOCKING : 0);

	RIPC_ASSERT(rsslSocketChannel->intState == RIPC_INT_ST_COMPLETE);

	if (rsslSocketChannel->blocking == 0)
	{
		/* Check to see if the connect() call has completed. */
		retval = (*(rsslSocketChannel->transportFuncs->connected))(rsslSocketChannel->stream,
																   rsslSocketChannel->transportInfo);

		if (retval != 1)
		{
			if (retval != 0)
			{
				_rsslSetError(error, NULL, RSSL_RET_FAILURE, errno);
				snprintf(error->text, MAX_RSSL_ERROR_TEXT,
					"<%s:%d> Error: 1002 connect() failed. System errno: (%d)\n",
					__FILE__, __LINE__, errno);

				return(RIPC_CONN_ERROR);
			}
			else
			{
				inPr->types = RIPC_INPROG_WRT_NOT;
				return(RIPC_CONN_IN_PROGRESS);
			}
		}
	}

	if (rsslSocketChannel->mountNak != 0)
	{
		ipcRejectSession(rsslSocketChannel, (RsslUInt16)rsslSocketChannel->mountNak, error);

		return(RIPC_CONN_ERROR);
	}

	/* Build and send ACK back */
	/* need to put chunk length in front of connection ack if we are using WinInet tunneling */

	headerLength = IPC_100_CONN_ACK;
	newIpcMsgLength = headerLength + 8;
	/* Base IPC Msg Length; gets modified through switch */
	switch (rsslSocketChannel->version->connVersion)
	{
	case CONN_VERSION_11:
	case CONN_VERSION_12:
		newIpcMsgLength += 1;  /* protocolType: 1 */
		break;
	case CONN_VERSION_13:
		/* figure out total length of component version stuff */
		componentVersionLength = 2 + ((rsslSocketChannel->componentVerLen > 253) ? 253 : rsslSocketChannel->componentVerLen);
		newIpcMsgLength += componentVersionLength + 1; /* +1 is for protocol length */
		break;
	case CONN_VERSION_14:
		/* figure out total length of component version stuff */
		componentVersionLength = 2 + ((rsslSocketChannel->componentVerLen > 253) ? 253 : rsslSocketChannel->componentVerLen);
		newIpcMsgLength += componentVersionLength + 1; /* +1 is for protocol length */
		/* If doing key exchange and type is TR_SL_1, add 27 */
		/* If other types of encryption are supported, this should turn into a switch or something */
		if ((rsslSocketChannel->keyExchange == 1) && (rsslSocketChannel->encryptionType == TR_SL_1))
			newIpcMsgLength += 27;
		break;
	}

	/* Added in version 11 and beyond */
	if (rsslSocketChannel->version->connVersion != CONN_VERSION_10)
	{
		/* force compression here */
		if ((rsslSocketChannel->outCompression != 0) && (rsslSocketChannel->forcecomp))
			flags |= RSSL_COMP_FORCE;
	}

	if (rsslSocketChannel->httpHeaders)
	{
		chunkLength = sprintf((conMsg + iterator), "%x\r\n", (RsslUInt32)newIpcMsgLength);
		iterator += chunkLength;
	}

	flags |= IPC_EXTENDED_FLAGS;
	_move_u16_swap((conMsg + iterator), &newIpcMsgLength);	/* hdr.IPC_len */
	iterator += 2;
	conMsg[iterator++] = (char)flags;
	conMsg[iterator++] = (char)ipc10Connack;			/* hdr.IPC_opcode */
	conMsg[iterator++] = (char)headerLength;			/* hdr length */
	conMsg[iterator++] = (char)0;						/* Currently 1 byte */
	_move_u32_swap((conMsg + iterator), &versionNumber);	/* hdr.IPC_version */
	iterator += 4;
	_move_u16_swap((conMsg + iterator), &maxMsgSize);
	iterator += 2;
	conMsg[iterator++] = (char)rsslSocketChannel->rsslFlags;
	conMsg[iterator++] = (RsslUInt8)rsslSocketChannel->pingTimeout;
	conMsg[iterator++] = (RsslUInt8)rsslSocketChannel->majorVersion;
	conMsg[iterator++] = (RsslUInt8)rsslSocketChannel->minorVersion;
	{
		RsslUInt16 outCompression = rsslSocketChannel->outCompression;
		_move_u16_swap((conMsg + iterator), &outCompression);	/* make sure to use a 16 bit data type */
	}
	iterator += 2;

	/* This was added in version 11 and beyond */
	if (rsslSocketChannel->version->connVersion != CONN_VERSION_10)
		conMsg[iterator++] = (RsslUInt8)rsslSocketChannel->server->zlibCompressionLevel;

	/* Add component versioning info and key exchange to handshake */
	switch (rsslSocketChannel->version->connVersion)
	{
	case CONN_VERSION_14:  /* add key negotiation; should be on version 14 and higher */
		if (rsslSocketChannel->keyExchange == 1)
		{
			RsslUInt64 server_send_key;
			/* need to store P as well */
			rsslSocketChannel->P = randull();
			/* In the rare case randull returns 0, the exchange Key should be set to 0 */
			if (rsslSocketChannel->P == 0)
				rsslSocketChannel->G = 0;
			else
				/* G definition can be fixed as Ted says */
				rsslSocketChannel->G = 5;

			/* first byte indicates the encryption type - lets us add other types of key exchange possibly without changing handshake */
			conMsg[iterator++] = (RsslUInt8)RIPC_KEY_EXCHANGE;
			/* right now, SHA 1 is the only one.  If we add others, we will
			need to refactor this section of code to support multiples */
			conMsg[iterator++] = rsslSocketChannel->encryptionType;
			conMsg[iterator++] = (RsslUInt8)24; /* put the length of the following key stuff here */
			iterator += rwfPut64((conMsg + iterator), rsslSocketChannel->P);
			iterator += rwfPut64((conMsg + iterator), rsslSocketChannel->G);

			/* In the rare case randull returns 0, the key exchange should be set to 0 */
			if (rsslSocketChannel->P == 0)
				server_send_key = rsslSocketChannel->random_key = 0;
			else
			{
				/* need server random to calculate multiple keys, store this */
				rsslSocketChannel->random_key = randull();
				/* Server creates key to send to client */
				server_send_key = modPowFast(rsslSocketChannel->G, rsslSocketChannel->random_key, rsslSocketChannel->P);
			}
			iterator += rwfPut64((conMsg + iterator), server_send_key);
		}
		/* add key exchange stuff if needed and fall through */
	case CONN_VERSION_13:
		/* put component versioning stuff */
		/* rb15 total length followed by rb15 length specified product version string */
		/* having both lengths lets us add component versioning elements in the future without adding new connection handshakes */
		/* write total length of component versioning */
		iterator += rwfPut8((conMsg + iterator), componentVersionLength);
		/* write version string length */
		/* reuse componentVersoinLength for namelength */
		componentVersionLength = ((rsslSocketChannel->componentVerLen > 253) ? 253 : rsslSocketChannel->componentVerLen);
		iterator += rwfPut8((conMsg + iterator), componentVersionLength);
		/* write version string */
		if (componentVersionLength > 0)
		{
			MemCopyByInt((conMsg + iterator), rsslSocketChannel->componentVer, componentVersionLength);
			iterator += componentVersionLength;
		}
		break;
	}

	/* apply trailer to end of actual data */
	if (rsslSocketChannel->httpHeaders)
		chunkLength += sprintf((conMsg + iterator), "%s", "\r\n");

	ipcMsgLength = (RsslUInt32)newIpcMsgLength + chunkLength;

	/* End of ack */

	cMsg = (*(rsslSocketChannel->protocolFuncs->getGlobalBuffer))( ipcMsgLength);
	MemCopyByInt(cMsg->buffer, conMsg, ipcMsgLength);
	cMsg->length = ipcMsgLength;
	/* Set/populate the prefix protocol header if one exists */
	(*(rsslSocketChannel->protocolFuncs->prependTransportHdr))((void*)rsslSocketChannel, cMsg, error);

	if (rsslSocketChannel->dbgFlags & RSSL_DEBUG_IPC_DUMP_INIT)
	{
		(*(ripcDumpOutFunc))(__FUNCTION__, cMsg->buffer, (RsslUInt32)cMsg->length, rsslSocketChannel->stream);
	}

	IPC_MUTEX_UNLOCK(rsslSocketChannel);

	if (rsslSocketChannel->httpHeaders)
		retval = (*(rsslSocketChannel->transportFuncs->writeTransport))(rsslSocketChannel->tunnelTransportInfo, (char*)cMsg->buffer, (RsslInt32)cMsg->length, rwflags, error);
	else
		retval = (*(rsslSocketChannel->transportFuncs->writeTransport))(rsslSocketChannel->transportInfo, cMsg->buffer, (RsslInt32)cMsg->length, rwflags, error);

	IPC_MUTEX_LOCK(rsslSocketChannel);

	if (cMsg) rtr_smplcFreeMsg(cMsg);
			
	if (rsslSocketChannel->workState & RIPC_INT_SHTDOWN_PEND)
	{
		_rsslSetError(error, NULL, RSSL_RET_FAILURE, errno);
		snprintf(error->text, MAX_RSSL_ERROR_TEXT,
			"<%s:%d> Error: 1003 ipcFinishSession() failed due to channel shutting down.\n",
			__FILE__, __LINE__);

		return(RIPC_CONN_ERROR);
	}

	if (retval < 0)
	{
		size_t len = strlen(error->text);
		_rsslSetError(error, NULL, RSSL_RET_FAILURE, errno);
		snprintf((error->text + len), (MAX_RSSL_ERROR_TEXT - len),
			"<%s:%d> Error: 1002 ipcWrite() could not write connack. System errno: (%d)\n",
			__FILE__, __LINE__, errno);

		return(RIPC_CONN_ERROR);
	}

	/* Key exchange requires a third leg of the handshake so we have to
	* keep the state progressing.  A new internal state is needed so the next call to InitChannel goes into that method */
	if (rsslSocketChannel->keyExchange == 1)
	{
		rsslSocketChannel->intState = RIPC_INT_ST_WAIT_CLIENT_KEY;
		return(RIPC_CONN_IN_PROGRESS);
	}
	else
	{

		if (ipcGetSocketRow(rsslSocketChannel, error) == RSSL_RET_FAILURE)
		{
			return(RIPC_CONN_ERROR);
		}

		/* All older versions are active at this point */
		rsslSocketChannel->state = RSSL_CH_STATE_ACTIVE;
		rsslSocketChannel->intState = RIPC_INT_ST_ACTIVE;

		if (rsslSocketChannel->lowerCompressionThreshold == 0)
		{
			/* Use the default compression threshold, based on the negotiated compression type. */
			switch (rsslSocketChannel->outCompression)
			{
			case RSSL_COMP_ZLIB:
				rsslSocketChannel->lowerCompressionThreshold = RSSL_COMP_DFLT_THRESHOLD_ZLIB;
				break;
			case RSSL_COMP_LZ4:
				rsslSocketChannel->lowerCompressionThreshold = RSSL_COMP_DFLT_THRESHOLD_LZ4;
				break;
			default:
				break;
			}
		}

		return(RIPC_CONN_ACTIVE);
	}
}

/* This method gets invoked on the client side to send the key to the server.  channel should transition to
* active after successful completion */
static ripcSessInit ipcSendClientKey(RsslSocketChannel *rsslSocketChannel, ripcSessInProg *inPr, RsslError *error)
{
	ripcRWFlags		rwflags = RIPC_RW_WAITALL;

	_DEBUG_TRACE_CONN("called\n")

	rwflags |= (rsslSocketChannel->blocking ? RIPC_RW_BLOCKING : 0);

	//	if (sess->blocking == 0)
	{
		/* want to have more than we need for this */
		char	ripcHead[65535];
		rtr_msgb_t *ckeyMsg = 0;
		RsslInt32		bytesSent;
		RsslUInt16		len = 0;
#ifdef _WIN32
		ripcWinInetSession *winInet = rsslSocketChannel->transportInfo;
#endif

		if (rsslSocketChannel->version->connVersion > CONN_VERSION_13)
		{
			RsslUInt64 client_send_key;
			len = 2;
			ripcHead[len++] = RIPC_KEY_EXCHANGE;

			if (rsslSocketChannel->encryptionType == TR_SL_1 && rsslSocketChannel->P != 0)
			{
				/* Client computes shared secret */
				rsslSocketChannel->random_key = randull();
				client_send_key = modPowFast(rsslSocketChannel->G, rsslSocketChannel->random_key, rsslSocketChannel->P);
				/* rsslSocketChannel->send_key will be what the server sent, along with P and G; sess->random_key is calculated above */
				rsslSocketChannel->shared_key = modPowFast(rsslSocketChannel->send_key, rsslSocketChannel->random_key, rsslSocketChannel->P);

				ripcHead[len++] = (RsslUInt8)8; /* length, for future proofing -type is already sent */
				len += rwfPut64((ripcHead + len), client_send_key);
			}
			else
			{
				/* we do not know the servers type, so send back a 0 length key */
				ripcHead[len++] = (RsslUInt8)0;
				rsslSocketChannel->shared_key = 0;
				rsslSocketChannel->shared_key = 0;
				rsslSocketChannel->encryptionType = 0;
			}

			/* now put the length in the first 2 bytes of header */
			_move_u16_swap(ripcHead, &len);
		}
		else
		{
			_rsslSetError(error, NULL, RSSL_RET_FAILURE, errno);
			snprintf(error->text, MAX_RSSL_ERROR_TEXT,
				"<%s:%d> Error: 1007 Illegal connection version: (%d).\n",
				__FILE__, __LINE__, rsslSocketChannel->version->connVersion);

			return(RIPC_CONN_ERROR);
		}

		ckeyMsg = (*(rsslSocketChannel->protocolFuncs->getGlobalBuffer))( len);

		MemCopyByInt(ckeyMsg->buffer, ripcHead, len);
		ckeyMsg->length = len;
		/* Set/populate the prefix protocol header if one exists */
		(*(rsslSocketChannel->protocolFuncs->prependTransportHdr))((void*)rsslSocketChannel, ckeyMsg, error);

		if (rsslSocketChannel->dbgFlags & RSSL_DEBUG_IPC_DUMP_INIT)
		{
			(*(ripcDumpOutFunc))(__FUNCTION__, ckeyMsg->buffer, (RsslUInt32)ckeyMsg->length, rsslSocketChannel->stream);
		}

		IPC_MUTEX_UNLOCK(rsslSocketChannel);

		/* do not need to worry about httpHeader here because this is client side only  - function pointers will take care of it */
#if (defined(_WINDOWS) || defined(_WIN32))
		{
			RsslInt32	i;
			for (i = 0; i<10; i++)
			{
				bytesSent = (*(rsslSocketChannel->transportFuncs->writeTransport))(
					rsslSocketChannel->transportInfo, ckeyMsg->buffer, (RsslInt32)ckeyMsg->length, rwflags, error);
				if (bytesSent > 0)
					break;
			}
		}
#else
		bytesSent = (*(rsslSocketChannel->transportFuncs->writeTransport))(rsslSocketChannel->transportInfo,
			ckeyMsg->buffer, ckeyMsg->length, rwflags, error);
#endif

		IPC_MUTEX_LOCK(rsslSocketChannel);

		_DEBUG_TRACE_WRITE("fd "SOCKET_PRINT_TYPE" headerLen: %d bytes %u\n", rsslSocketChannel->stream, len, bytesSent)

		if (ckeyMsg) rtr_smplcFreeMsg(ckeyMsg);
			
		if (rsslSocketChannel->workState & RIPC_INT_SHTDOWN_PEND)
		{
			_rsslSetError(error, NULL, RSSL_RET_FAILURE, errno);
			snprintf(error->text, MAX_RSSL_ERROR_TEXT,
				"<%s:%d> Error: 1003 ipcSendClientKey() failed due to channel shutting down.\n",
				__FILE__, __LINE__);

			return(RIPC_CONN_ERROR);
		}

		if (bytesSent < 0)
		{
			_rsslSetError(error, NULL, RSSL_RET_FAILURE, errno);
			snprintf(error->text, MAX_RSSL_ERROR_TEXT,
				"<%s:%d> Error: 1002 ipcWrite() failed to write version number. System errno: (%d)\n",
				__FILE__, __LINE__, errno);

			return(RIPC_CONN_ERROR);
		}

		if (ipcGetSocketRow(rsslSocketChannel, error) == RSSL_RET_FAILURE)
		{
			return(RIPC_CONN_ERROR);
		}

		rsslSocketChannel->state = RSSL_CH_STATE_ACTIVE;
		rsslSocketChannel->intState = RIPC_INT_ST_ACTIVE;



		return(RIPC_CONN_ACTIVE);
	}
}


/* This method gets called on server side when we have the third leg to our handshake due to
* key negotiation.  This should just receive the final handshake message, process it and
* make the servers connection active */
static ripcSessInit ipcWaitClientKey(RsslSocketChannel *rsslSocketChannel, ripcSessInProg *inPr, RsslError *error)
{
	RsslInt32			cc;
	ripcRWFlags			rwflags = RIPC_RW_NONE;
	char 				*hdrStart;
	RsslUInt16			length = 0;
	RsslUInt8			keyLen = 0;

	_DEBUG_TRACE_CONN("read header fd "SOCKET_PRINT_TYPE"\n", rsslSocketChannel->stream)

	rwflags |= (rsslSocketChannel->blocking ? RIPC_RW_BLOCKING : 0);

	RIPC_ASSERT(rsslSocketChannel->intState == RIPC_INT_ST_WAIT_CLIENT_KEY);



	/* This is intended to read 14 bytes in a single read.  The message it should get is 4 or 12 bytes.  If this reads too much
	* the subsequent login message can be swallowed up.  We will properly keep it in the input buffer, however
	* there will be no IO Notification for it and we have no way to inform the user of this either (e.g. no readret at this
	* point in the connection handshake.
	* By reading only our message + a few extra bytes, any remaining data will stay in the TCP_RECV buffer and trigger
	* another round of IO Notification, so the app will know to call rsslRead */
	cc = (*(rsslSocketChannel->protocolFuncs->readTransportMsg))((void*)rsslSocketChannel,
		(rsslSocketChannel->inputBuffer->buffer + rsslSocketChannel->inputBuffer->length),
		(V10_MIN_CONN_HDR+8), rwflags, error);

	if (rsslSocketChannel->workState & RIPC_INT_SHTDOWN_PEND)
	{
		_rsslSetError(error, NULL, RSSL_RET_FAILURE, errno);
		snprintf(error->text, MAX_RSSL_ERROR_TEXT,
			"<%s:%d> Error: 1003 ipcWaitClientKey() failed due to channel shutting down.\n",
			__FILE__, __LINE__);

		return(RIPC_CONN_ERROR);
	}

	_DEBUG_TRACE_READ("read header fd "SOCKET_PRINT_TYPE" read %d\n", rsslSocketChannel->stream, cc)

	if (cc == RSSL_RET_READ_WOULD_BLOCK)
	{
		cc = 0;
	}

	if (cc < 0)
	{
		_rsslSetError(error, NULL, RSSL_RET_FAILURE, errno);
		snprintf(error->text, MAX_RSSL_ERROR_TEXT,
			"<%s:%d> Error: 1002 Could not read IPC Mount Request. System errno: (%d)\n",
			__FILE__, __LINE__, errno);

		return(RIPC_CONN_ERROR);
	}
	rsslSocketChannel->inputBuffer->length += cc;

	if (cc < 4)  /* this needs to be our base key exchange message length */
	{
		/* This will happen when there is no data atall to be read,
		* and the read fails with error _IPC_WOULD_BLOCK or EINTR
		*/
		if (cc < 0)
		{
			_rsslSetError(error, NULL, RSSL_RET_FAILURE, errno);
			snprintf(error->text, MAX_RSSL_ERROR_TEXT,
				"<%s:%d> Error: 1002 Invalid mount request size. System errno: (%d)\n",
				__FILE__, __LINE__, cc);

			return(RIPC_CONN_ERROR);
		}
		else
		{
			return(RIPC_CONN_IN_PROGRESS);
		}
	}

	hdrStart = rsslSocketChannel->inputBuffer->buffer + rsslSocketChannel->inputBufCursor;

	if (rsslSocketChannel->dbgFlags & RSSL_DEBUG_IPC_DUMP_INIT)
	{
		(*(ripcDumpInFunc))(__FUNCTION__, hdrStart, (RsslUInt32)(rsslSocketChannel->inputBuffer->length - rsslSocketChannel->inputBufCursor), rsslSocketChannel->stream);
	}

	_move_u16_swap(&length, hdrStart);
	rsslSocketChannel->inputBufCursor += 3;
	rsslSocketChannel->inputBufCursor += rwfGet8(keyLen, (hdrStart + rsslSocketChannel->inputBufCursor));

	if (keyLen > 0)
	{
		/* The client can do the same encryption as us */
		if (rsslSocketChannel->encryptionType == TR_SL_1 && rsslSocketChannel->P != 0)
		{
			rsslSocketChannel->inputBufCursor += rwfGet64(rsslSocketChannel->send_key, (hdrStart + rsslSocketChannel->inputBufCursor));
			/* Now calculate servers shared secret */
			rsslSocketChannel->shared_key = modPowFast(rsslSocketChannel->send_key, rsslSocketChannel->random_key, rsslSocketChannel->P);
		}
		else
		{
			rsslSocketChannel->inputBufCursor += keyLen;  /* not sure how we would get here. */
			rsslSocketChannel->encryptionType = 0;
			rsslSocketChannel->shared_key = 0;
		}
	}
	else
	{
		/* if key length is 0, client cant do our encryption */
		rsslSocketChannel->inputBufCursor += keyLen;
		rsslSocketChannel->encryptionType = 0;
		rsslSocketChannel->shared_key = 0;
	}

	/* If we read more bytes than in the header, make sure we move the inputBufCursor.
	* This can happen in quick applications that send data as soon as the channel becomes active.
	*/
	if (rsslSocketChannel->inputBuffer->length == rsslSocketChannel->inputBufCursor)
	{
		/* reset */
		rsslSocketChannel->inputBuffer->length = 0;
		rsslSocketChannel->inputBufCursor = 0;
	}

	if (ipcGetSocketRow(rsslSocketChannel, error) == RSSL_RET_FAILURE)
	{
		return(RIPC_CONN_ERROR);
	}

	rsslSocketChannel->state = RSSL_CH_STATE_ACTIVE;
	rsslSocketChannel->intState = RIPC_INT_ST_ACTIVE;



	if (rsslSocketChannel->lowerCompressionThreshold == 0)
	{
		/* Use the default compression threshold, based on the negotiated compression type. */
		switch (rsslSocketChannel->outCompression)
		{
		case RSSL_COMP_ZLIB:
			rsslSocketChannel->lowerCompressionThreshold = RSSL_COMP_DFLT_THRESHOLD_ZLIB;
			break;
		case RSSL_COMP_LZ4:
			rsslSocketChannel->lowerCompressionThreshold = RSSL_COMP_DFLT_THRESHOLD_LZ4;
			break;
		default:
			break;
		}
	}
	return(RIPC_CONN_ACTIVE);
}


void ipcCloseActiveSrvr(RsslServerSocketChannel *rsslServerSocketChannel)
{
	rsslServerSocketChannel->stream = RIPC_INVALID_SOCKET;

	if (rsslServerSocketChannel->state != RSSL_CH_STATE_INACTIVE)
		rsslServerSocketChannel->state = RSSL_CH_STATE_INACTIVE;
}

RsslInt32 ipcSessSetMode(RsslSocket sock_fd, RsslInt32 blocking, RsslInt32 tcp_nodelay, RsslError *error, RsslInt32 line)
{
	ripcSocketOption	opts;
	opts.code = RIPC_SOPT_BLOCKING;
	opts.options.turn_on = (blocking ? 1 : 0);

	if (ipcSockOpts(sock_fd, &opts) < 0)
	{
		_rsslSetError(error, NULL, RSSL_RET_FAILURE, errno);
		snprintf(error->text, MAX_RSSL_ERROR_TEXT,
			"<%s:%d> Error: 1002 Could not set blocking(%d) on socket. System errno: %d\n",
			__FILE__, line, blocking, errno);

		return(-1);
	}

	opts.code = RIPC_SOPT_CLOEXEC;
	opts.options.turn_on = 1;

	if (ipcSockOpts(sock_fd, &opts) < 0)
	{
		_rsslSetError(error, NULL, RSSL_RET_FAILURE, errno);
		snprintf(error->text, MAX_RSSL_ERROR_TEXT,
			"<%s:%d> Error: 1002 Could not set cloexec(%d) on socket. System errno: %d\n",
			__FILE__, line, 1, errno);

		return(-1);
	}

	if (tcp_nodelay)
	{
		opts.code = RIPC_SOPT_TCP_NODELAY;
		opts.options.turn_on = (tcp_nodelay ? 1 : 0);
		if (ipcSockOpts(sock_fd, &opts) < 0)
		{
			snprintf(error->text, MAX_RSSL_ERROR_TEXT,
				"<%s:%d> Error: 1002 Could not set NoDelay(%d) on socket. System errno: %d\n",
				__FILE__, __LINE__, tcp_nodelay, errno);

			return(-1);
		}
	}

	return(0);
}

static ripcSessInit ipcProxyConnecting(RsslSocketChannel *rsslSocketChannel, ripcSessInProg *inPr, RsslError *error)
{
	ripcRWFlags		rwflags = RIPC_RW_WAITALL;
	RsslInt32		len = 0;
	RsslInt32		bytesSent = 0;
	
	rwflags |= (rsslSocketChannel->blocking ? RIPC_RW_BLOCKING : 0);
	
	rsslSocketChannel->intState = RIPC_INT_ST_CLIENT_WAIT_PROXY_ACK;

	return(RIPC_CONN_IN_PROGRESS);
}

ripcSessInit ipcConnecting(RsslSocketChannel *rsslSocketChannel, ripcSessInProg *inPr, RsslError *error)
{
	RsslInt32 retval;
	ripcRWFlags		rwflags = RIPC_RW_WAITALL;

	_DEBUG_TRACE_CONN("called\n")

	rwflags |= (rsslSocketChannel->blocking ? RIPC_RW_BLOCKING : 0);

	if (rsslSocketChannel->blocking == 0)
	{
		/* Check to see if the connect() call has completed. */

		retval = (*(rsslSocketChannel->transportFuncs->connected))(rsslSocketChannel->stream,
																   rsslSocketChannel->transportInfo);

		if (retval != 1)
		{
			if (retval != 0)
			{
				_rsslSetError(error, NULL, RSSL_RET_FAILURE, errno);
				snprintf(error->text, MAX_RSSL_ERROR_TEXT,
					"<%s:%d> Error: 1002 ipcConnecting() client connect() failed.  System errno: (%d)\n",
					__FILE__, __LINE__, errno);

				return(RIPC_CONN_ERROR);
			}
			else
			{
				inPr->types = RIPC_INPROG_WRT_NOT;
				return(RIPC_CONN_IN_PROGRESS);
			}
		}
#ifdef IPC_DEBUG_SOCKET_NAME
		_ipcdGetSockName(rsslSocketChannel->stream);
		_ipcdGetPeerName(rsslSocketChannel->stream);
#endif
	}

	{
		rtr_msgb_t	*cMsg = 0;
		/* want to have more than we need for this */
		char	ripcHead[65535];
		RsslInt32	bytesSent;
		RsslUInt16	headerSize, ripcHeaderSize;
		RsslUInt8	opCode = 0;
		RsslUInt8	componentVersionLength;

		RsslUInt16	len = 0;
		char	strHostname[IPC_MAX_HOST_NAME];
		/* made this 129 (128 + \0) to support IPv6 if needed in the future */
		char	strIpAddr[129] = {0};
		char	*pBuf;
		/*
		struct hostent *pHostent;
		*/
		RsslInt32	iRetVal;
		RsslUInt8	hostnameLen = 0;
		RsslUInt8	addrLen = 0;
		struct sockaddr_in ipInfo;
		RsslInt32	ipLen = sizeof(ipInfo);
#ifdef _WIN32
		ripcWinInetSession *winInet = rsslSocketChannel->transportInfo;
#endif

		switch (rsslSocketChannel->version->connVersion)
		{
		case CONN_VERSION_10:
		case CONN_VERSION_11:
		case CONN_VERSION_12:
		case CONN_VERSION_13:
		case CONN_VERSION_14:
		{
			if (rsslSocketChannel->inDecompress == 0)
				len = 7;
			else
				len = 7 + RSSL_COMP_BITMAP_SIZE;
			/*_move_u16_swap(ripcHead, &len); */
			ripcHead[2] = (char)opCode;
			_move_u32_swap(ripcHead + 3, &rsslSocketChannel->version->connVersion);
			/* flags */
			/* for now, always asking for key exchange with version 14 or higher. */
			/* receive side is lookign for this presence, so if we ever unset this version 14 could theoretically work without the key exchange if needed. */
			if (rsslSocketChannel->version->connVersion > CONN_VERSION_13)
			{
				ripcHead[7] = RIPC_KEY_EXCHANGE;
				rsslSocketChannel->keyExchange = 1;
			}
			else
				ripcHead[7] = 0;
			/* get the host name */
			iRetVal = gethostname(strHostname, IPC_MAX_HOST_NAME);
			hostnameLen = (RsslUInt8)strlen(strHostname);

			/* want to make sure domain name not in result */
			if (iRetVal == 0)
			{
#ifdef _WIN32
				if (rsslSocketChannel->usingWinInet)
				{
					struct in_addr temp;

					temp.S_un.S_addr = winInet->config.address;

					/* we already got the IP address */
					pBuf = strIpAddr;

					inet_ntop(AF_INET, (void*)&temp, (PSTR)pBuf, sizeof(strIpAddr));

					pBuf +=
						addrLen = (RsslUInt8)strlen(strIpAddr);
				}
				else
				{
					if (((*(rsslSocketChannel->transportFuncs->getSockName))(rsslSocketChannel->stream,
					                                    (struct sockaddr*)&ipInfo, &ipLen,
														rsslSocketChannel->transportInfo)) == RSSL_RET_SUCCESS)
					{
						pBuf = strIpAddr;

						inet_ntop(AF_INET, (void*)&ipInfo.sin_addr, (PSTR)pBuf, sizeof(strIpAddr));

						pBuf +=
							addrLen = (RsslUInt8)strlen(pBuf);
					}
					else
						addrLen = 0;
				}
#else
				if (((*(rsslSocketChannel->transportFuncs->getSockName))(rsslSocketChannel->stream,
					                                    (struct sockaddr*)&ipInfo, &ipLen,
														rsslSocketChannel->transportInfo)) == RSSL_RET_SUCCESS)
				{
					pBuf = strIpAddr;
					strncpy(pBuf, inet_ntoa(ipInfo.sin_addr), 129);
					pBuf +=
						addrLen = strlen(strIpAddr);
				}
				else
					addrLen = 0;
#endif
			}
			else
			{
				addrLen = 0;
				hostnameLen = 0;
			}

			/* add hostnamelength and addrLen and 2 - the 2 is the length field	for the hostname and ip address */
			if (rsslSocketChannel->inDecompress == 0)
			{
				ripcHeaderSize = V10_MIN_CONN_HDR + hostnameLen + addrLen + 2;
				if (rsslSocketChannel->version->connVersion > CONN_VERSION_11)
					++ripcHeaderSize;
				ripcHead[8] = (char)ripcHeaderSize;
				ripcHead[9] = 0;
				/* set len to the next element for the variable portion */
				len = 10;
			}
			else
			{
				ripcHeaderSize = V10_MIN_CONN_HDR + RSSL_COMP_BITMAP_SIZE +
					hostnameLen + addrLen + 2;
				if (rsslSocketChannel->version->connVersion > CONN_VERSION_11)
					++ripcHeaderSize;
				ripcHead[8] = (char)ripcHeaderSize;
				ripcHead[9] = RSSL_COMP_BITMAP_SIZE;
				MemCopyByInt((ripcHead + 10), (char*)rsslSocketChannel->compressionBitmap, RSSL_COMP_BITMAP_SIZE);
				/* set len to the next element for the variable portion */
				len = 10 + RSSL_COMP_BITMAP_SIZE;
			}
			/* add the ping interval in */
			ripcHead[len++] = (RsslUInt8)rsslSocketChannel->pingTimeout;
			ripcHead[len++] = (RsslUInt8)rsslSocketChannel->rsslFlags;
			/* Add in protocol type if we are higher than version 11 */
			if (rsslSocketChannel->version->connVersion > CONN_VERSION_11)
			{
				ripcHead[len++] = (RsslUInt8)rsslSocketChannel->protocolType;
			}
			ripcHead[len++] = (RsslUInt8)rsslSocketChannel->majorVersion;
			ripcHead[len++] = (RsslUInt8)rsslSocketChannel->minorVersion;
			ripcHead[len++] = (char)hostnameLen;
			if (hostnameLen > 0)
			{
				MemCopyByInt((ripcHead + len), strHostname, hostnameLen);
			}
			len += hostnameLen;
			ripcHead[len++] = (char)addrLen;

			if (addrLen > 0)
			{
				MemCopyByInt((ripcHead + len), strIpAddr, addrLen);
			}
			len += addrLen;

			/* Component versioning is in ripc handshake 13 and higher */
			if (rsslSocketChannel->version->connVersion > CONN_VERSION_12)
			{
				/* put component versioning stuff */
				/* rb15 total length followed by rb15 length specified product version string */
				/* having both lengths lets us add component versioning elements in the future without adding new connection handshakes */
				/* component version length is  precalculated above for ripcHeadLen */
				/* write total length of component versioning */

				/* figure out total length of component version stuff */
				componentVersionLength = 1 + ((rsslSocketChannel->componentVerLen > 253) ? 253 : rsslSocketChannel->componentVerLen);
				len += rwfPut8((ripcHead + len), componentVersionLength);
				/* write version string length */
				componentVersionLength = ((rsslSocketChannel->componentVerLen > 253) ? 253 : rsslSocketChannel->componentVerLen);
				len += rwfPut8((ripcHead + len), componentVersionLength);
				/* write version string */
				if (componentVersionLength > 0)
				{
					MemCopyByInt((ripcHead + len), rsslSocketChannel->componentVer, componentVersionLength);
					len += componentVersionLength;
				}

				/* now put the length in the first 2 bytes of header */
				_move_u16_swap(ripcHead, &len);

				/* ripcHeaderSize does not include component info, but we need it here */
				headerSize = ripcHeaderSize + componentVersionLength + 2;
			}
			else
			{
				/* No component versioning */
				/* now put the length in the first 2 bytes of header */
				_move_u16_swap(ripcHead, &len);

				headerSize = ripcHeaderSize;
			}

			_DEBUG_TRACE_CONN("fd "SOCKET_PRINT_TYPE" connTyp %d wsProt %d\n", 
							rsslSocketChannel->stream, 
							rsslSocketChannel->connType,
							rsslSocketChannel->protocolType)
			/* Need to get a buffer to write the message from this abstracted function 
			 * in case the message needs a protocol header prefix */
			cMsg = (*(rsslSocketChannel->protocolFuncs->getGlobalBuffer))( headerSize);

			MemCopyByInt(cMsg->buffer, ripcHead, headerSize);
			cMsg->length = headerSize;
			/* Set/populate the prefix protocol header if one exists */
			(*(rsslSocketChannel->protocolFuncs->prependTransportHdr))((void*)rsslSocketChannel, cMsg, error);
		}
		break;
		default:
			_rsslSetError(error, NULL, RSSL_RET_FAILURE, errno);
			snprintf(error->text, MAX_RSSL_ERROR_TEXT,
				"<%s:%d> Error: 1007 Illegal connection version: (%d).\n",
				__FILE__, __LINE__, rsslSocketChannel->version->connVersion);

			return(RIPC_CONN_ERROR);
			break;
		}

		if (cMsg && (rsslSocketChannel->dbgFlags & RSSL_DEBUG_IPC_DUMP_INIT))
		{
			(*(ripcDumpOutFunc))(__FUNCTION__, cMsg->buffer, (RsslUInt32)cMsg->length, rsslSocketChannel->stream);
		}

		IPC_MUTEX_UNLOCK(rsslSocketChannel);

		/* do not need to worry about httpHeader here because this is client side only  - function pointers will take care of it */
#if (defined(_WINDOWS) || defined(_WIN32))
		{
			RsslInt32		i;
			for (i = 0; i<10; i++)
			{
				bytesSent = (*(rsslSocketChannel->transportFuncs->writeTransport))(
					rsslSocketChannel->transportInfo, cMsg->buffer, (RsslInt32)cMsg->length, rwflags, error);
				if (bytesSent > 0)
					break;
			}
		}
#else
		bytesSent = (*(rsslSocketChannel->transportFuncs->writeTransport))(rsslSocketChannel->transportInfo,
			cMsg->buffer, cMsg->length, rwflags, error);
#endif

		IPC_MUTEX_LOCK(rsslSocketChannel);

		_DEBUG_TRACE_WRITE("fd "SOCKET_PRINT_TYPE" headerLen %d bytes %u\n", rsslSocketChannel->stream, headerSize, bytesSent)

		if (cMsg) rtr_smplcFreeMsg(cMsg);

		if (rsslSocketChannel->workState & RIPC_INT_SHTDOWN_PEND)
		{
			_rsslSetError(error, NULL, RSSL_RET_FAILURE, errno);
			snprintf(error->text, MAX_RSSL_ERROR_TEXT,
				"<%s:%d> Error: 1003 ipcConnecting() failed due to channel shutting down.\n",
				__FILE__, __LINE__);

			return(RIPC_CONN_ERROR);
		}

		if (bytesSent < 0)
		{
			_rsslSetError(error, NULL, RSSL_RET_FAILURE, errno);
			snprintf(error->text, MAX_RSSL_ERROR_TEXT,
				"<%s:%d> Error: 1002 ipcWrite() failed to write version number. System errno: (%d)\n",
				__FILE__, __LINE__, errno);

			return(RIPC_CONN_ERROR);
		}

		rsslSocketChannel->intState = RIPC_INT_ST_WAIT_ACK;

		return(RIPC_CONN_IN_PROGRESS);
	}
}

static ripcSessInit ipcClientAccept(RsslSocketChannel *rsslSocketChannel, ripcSessInProg *inPr, RsslError *error)
{
	RsslSocket	fdtemp;
	RsslInt32	retval;
	ripcSocketOption	opts;

	if (rsslSocketChannel->blocking == 0)
	{
		/* Make sure the connection is ready for reading.
		* If not then the accept() call will block.
		*/
		retval = ipcReadyRead(rsslSocketChannel->stream);
		if (retval != 1)
		{
			if (retval != 0)
			{
				_rsslSetError(error, NULL, RSSL_RET_FAILURE, errno);
				snprintf(error->text, MAX_RSSL_ERROR_TEXT,
					"<%s:%d> Error: 1002 ipcClientAccept() System accept() not ready.  System errno: (%d)\n",
					__FILE__, __LINE__, errno);

				return(RIPC_CONN_ERROR);
			}
			else
				return(RIPC_CONN_IN_PROGRESS);
		}
	}

	IPC_MUTEX_UNLOCK(rsslSocketChannel);

	fdtemp = accept(rsslSocketChannel->stream, (struct sockaddr *)0, (socklen_t *)0);

	IPC_MUTEX_LOCK(rsslSocketChannel);

	if (rsslSocketChannel->workState & RIPC_INT_SHTDOWN_PEND)
	{
		_rsslSetError(error, NULL, RSSL_RET_FAILURE, errno);
		snprintf(error->text, MAX_RSSL_ERROR_TEXT,
			"<%s:%d> Error: 1003 ipcClientAccept() failed due to channel shutting down.\n",
			__FILE__, __LINE__);

		return(RIPC_CONN_ERROR);
	}

	if (!ripcValidSocket(fdtemp))
	{
		_rsslSetError(error, NULL, RSSL_RET_FAILURE, errno);
		snprintf(error->text, MAX_RSSL_ERROR_TEXT,
			"<%s:%d> Error: 1002 accept() failed. System errno: (%d)\n",
			__FILE__, __LINE__, errno);

		return(RIPC_CONN_ERROR);
	}

	opts.code = RIPC_SOPT_LINGER;
	opts.options.linger_time = 0;
	if (ipcSockOpts(fdtemp, &opts) < 0)
	{
		_rsslSetError(error, NULL, RSSL_RET_FAILURE, errno);
		snprintf(error->text, MAX_RSSL_ERROR_TEXT,
			"<%s:%d> Error: 1002 ipcClientAccept() Could not set SO_LINGER time to 0 on socket. System errno: (%d)\n",
			__FILE__, __LINE__, errno);

		sock_close(fdtemp);
		return(RIPC_CONN_ERROR);
	}

	if (ipcSessSetMode(fdtemp, rsslSocketChannel->blocking, rsslSocketChannel->tcp_nodelay, error, __LINE__) < 0)
	{
		_rsslSetError(error, NULL, RSSL_RET_FAILURE, errno);

		sock_close(fdtemp);
		return(RIPC_CONN_ERROR);
	}

	sock_close(rsslSocketChannel->stream);

	inPr->types = RIPC_INPROG_NEW_FD;
	inPr->oldSocket = rsslSocketChannel->stream;
	_rsslSocketChannelToIpcSocket(&(inPr->newSocket), rsslSocketChannel);

	rsslSocketChannel->stream = fdtemp;
	rsslSocketChannel->intState = RIPC_INT_ST_WAIT_ACK;

	return(RIPC_CONN_IN_PROGRESS);
}

static ripcSessInit ipcReconnectOld(RsslSocketChannel *rsslSocketChannel, ripcSessInProg *inPr, RsslError *error)
{
	switch (rsslSocketChannel->version->connVersion)
	{
	case CONN_VERSION_14:
		/* Since we may have been doing key exchange, we have to reset this in the session
		* before we go down to a version of code that does not support that */
		rsslSocketChannel->keyExchange = 0;
		rsslSocketChannel->shared_key = 0;
		rsslSocketChannel->P = 0;
		rsslSocketChannel->G = 0;
		rsslSocketChannel->random_key = 0;
		rsslSocketChannel->send_key = 0;
		rsslSocketChannel->shared_key = 0;
		rsslSocketChannel->encryptionType = 0;
		rsslSocketChannel->version = &ripc13Ver;
		break;
	case CONN_VERSION_13:
		rsslSocketChannel->version = &ripc12Ver;
		break;
	case CONN_VERSION_12:
		rsslSocketChannel->version = &ripc11Ver;
		break;
	case CONN_VERSION_11:
		rsslSocketChannel->version = &ripc10Ver;
		break;
	default:
		rsslSocketChannel->version = &ripc10Ver;
	}

	return ipcReconnectSocket(rsslSocketChannel, inPr, error);
}

static ripcSessInit ipcReconnectSocket(RsslSocketChannel *rsslSocketChannel, ripcSessInProg *inPr, RsslError *error)
{
	RsslSocket          sock_fd;
	RsslInt32			portnum;
	RsslInt32			initcomplete;

	void*				userSpecPtr = 0;
	RsslInt32			csFlags = RIPC_INT_CS_FLAG_NONE;
	RsslCurlJITFuncs*	curlFuncs;

	if (rsslSocketChannel->blocking)
		csFlags |= RIPC_INT_CS_FLAG_BLOCKING;

	if (rsslSocketChannel->tcp_nodelay)
		csFlags |= RIPC_INT_CS_FLAG_TCP_NODELAY;

	if (rsslSocketChannel->connType == RSSL_CONN_TYPE_HTTP)
		csFlags |= RIPC_INT_CS_FLAG_TUNNEL_NO_ENCRYPTION;

	if (rsslSocketChannel->workState & RIPC_INT_SHTDOWN_PEND)
	{
		_rsslSetError(error, NULL, RSSL_RET_FAILURE, errno);
		snprintf(error->text, MAX_RSSL_ERROR_TEXT,
			"<%s:%d> Error: 1003 ipcReconnectOld() failed due to channel shutting down.\n",
			__FILE__, __LINE__);

		return(RIPC_CONN_ERROR);
	}

	/* call shutdownTransport for old connection here */
	if (rsslSocketChannel->curlHandle)
	{
		if ((curlFuncs = rsslGetCurlFuncs()) == NULL)
		{
			_rsslSetError(error, NULL, RSSL_RET_FAILURE, 0);
			snprintf(error->text, MAX_RSSL_ERROR_TEXT,
				"<%s:%d> Error: 1002 ipcReconnectOld() failed due to Curl not being initialized.\n",
				__FILE__, __LINE__);

			return(RIPC_CONN_ERROR);
		}
		
        (*(curlFuncs->curl_easy_cleanup))(rsslSocketChannel->curlHandle);
		rsslSocketChannel->curlHandle = NULL;
        
        if(rsslSocketChannel->curlThreadInfo.curlError) 
            _rsslFree(rsslSocketChannel->curlThreadInfo.curlError);
        rsslSocketChannel->curlThreadInfo.curlError = 0;
		/* a CURL handle will only be set if this is an openSSL encrypted connection.  WinInet connections do not use curl */
		if (rsslSocketChannel->connType == RSSL_CONN_TYPE_ENCRYPTED)
		{
			((ripcSSLSession*)rsslSocketChannel->transportInfo)->socket = RIPC_INVALID_SOCKET;
		}
		else if (rsslSocketChannel->connType == RSSL_CONN_TYPE_SOCKET)
		{
			/* Set the transportInfo FD to RIPC_INVALID_SOCKET, because the curl cleanup call will close the channel for us */
			rsslSocketChannel->transportInfo = (void*)RIPC_INVALID_SOCKET;
		}
	}

	(*(rsslSocketChannel->transportFuncs->shutdownTransport))(rsslSocketChannel->transportInfo);
	rsslSocketChannel->transportInfo = NULL;

	if (rsslSocketChannel->connType == RSSL_CONN_TYPE_EXT_LINE_SOCKET)
	{
		userSpecPtr = (void*)&(rsslSocketChannel->numConnections);
		rsslSocketChannel->transportFuncs = &(transFuncs[RSSL_CONN_TYPE_EXT_LINE_SOCKET]);
	}

	IPC_MUTEX_UNLOCK(rsslSocketChannel);

	if ((sock_fd = (*(rsslSocketChannel->transportFuncs->connectSocket))(
		&portnum, rsslSocketChannel, csFlags, &userSpecPtr, error)) == RIPC_INVALID_SOCKET)
	{
		_rsslSetError(error, NULL, RSSL_RET_FAILURE, errno);

		IPC_MUTEX_LOCK(rsslSocketChannel);

		return(RIPC_CONN_ERROR);
	}

	IPC_MUTEX_LOCK(rsslSocketChannel);

	inPr->types = RIPC_INPROG_NEW_FD;
	inPr->types = RIPC_INPROG_NEW_FD | RIPC_INPROG_WRT_NOT;
	inPr->oldSocket = rsslSocketChannel->stream;

	rsslSocketChannel->stream = sock_fd;
	_rsslSocketChannelToIpcSocket(&(inPr->newSocket), rsslSocketChannel);

	rsslSocketChannel->intState = RIPC_INT_ST_CONNECTING;

	/* reset state for tunneled connections */
	if ((rsslSocketChannel->proxyPort && *(rsslSocketChannel->proxyPort) != '\0') && (rsslSocketChannel->usingWinInet == 0))
	{
		rsslSocketChannel->intState = RIPC_INT_ST_PROXY_CONNECTING;
	}
	else
	{
		if (rsslSocketChannel->connType == RSSL_CONN_TYPE_ENCRYPTED && rsslSocketChannel->usingWinInet == 0)
		{
			// This is where to go
			if (getSSLProtocolTransFuncs(rsslSocketChannel, rsslSocketChannel->sslProtocolBitmap) != 0)
			{
				_rsslSetError(error, NULL, RSSL_RET_FAILURE, errno);
				snprintf(error->text, MAX_RSSL_ERROR_TEXT,
					"<%s:%d> Error: 0012 Out of available SSL/TLS connection protocol options.\n",
					__FILE__, __LINE__);

				sock_close(sock_fd);
				return(RIPC_CONN_ERROR);
			}

			userSpecPtr = (void*)rsslSocketChannel;
		}
		/* Get the transport info */
		rsslSocketChannel->transportInfo = (*(rsslSocketChannel->transportFuncs->newClientConnection))(
			rsslSocketChannel->stream, &initcomplete, userSpecPtr, error);

		if (!rsslSocketChannel->transportInfo)
		{
			sock_close(sock_fd);
			return(RIPC_CONN_ERROR);
		}

		/* This state indicates that we have some initialization beyond a socket connect to initialize the underlying transport
		* So for HTTP, openSSL Encrypted connections, and long lining, this should initialize the socket.
		*/
		if (!initcomplete && (rsslSocketChannel->connType != RSSL_CONN_TYPE_SOCKET ||
							  rsslSocketChannel->connType != RSSL_CONN_TYPE_WEBSOCKET) )
		{
			rsslSocketChannel->intState = RIPC_INT_ST_CLIENT_TRANSPORT_INIT;
		}
	}

	return(RIPC_CONN_IN_PROGRESS);
}

ripcSessInit ipcWaitProxyAck(RsslSocketChannel *rsslSocketChannel, ripcSessInProg *inPr, RsslError *error)
{
	RsslInt32		ret;
	RsslInt32		msgLen = 0;
	ripcRWFlags		rwflags = RIPC_RW_NONE;
	RsslInt32		initcomplete = 0;
	char tempBuf[5];
	CURLcode curlret;
	RsslCurlJITFuncs* curlFuncs;

	rwflags |= (rsslSocketChannel->blocking ? RIPC_RW_BLOCKING : 0);

	IPC_MUTEX_UNLOCK(rsslSocketChannel);

	ret = rssl_pipe_read(&rsslSocketChannel->sessPipe, tempBuf, 1);
	if (ret == 0 || (ret == -1 && (errno == EAGAIN || errno == EWOULDBLOCK)))
	{
		/* Nothing to read, just return from here */
		return RIPC_CONN_IN_PROGRESS; /* assume the byte isnt ready yet, return success */
	}
	else if (ret < 0)
	{
		_rsslSetError(error, NULL, RSSL_RET_FAILURE, errno);
		snprintf(error->text, MAX_RSSL_ERROR_TEXT,
			"<%s:%d> Error: 1002 Pipe Read failed.  System errno: %i\n",
			__FILE__, __LINE__, errno );
		ripcRelSocketChannel(rsslSocketChannel);
		return RSSL_RET_FAILURE;
	}

	if ((curlFuncs = rsslGetCurlFuncs()) == NULL)
	{
		_rsslSetError(error, NULL, RSSL_RET_FAILURE, 0);
		snprintf(error->text, MAX_RSSL_ERROR_TEXT,
			"<%s:%d> Error: 1002 Curl not initialized.\n",
			__FILE__, __LINE__);
		ripcRelSocketChannel(rsslSocketChannel);
		return RSSL_RET_FAILURE;
	}

	/* The CURL thread has finished, and set the thread status to either DONE or ERRROR.  If ERROR, cleanup the channel and return.  CURL should have already closed everything */
	if (rsslSocketChannel->curlThreadInfo.curlThreadState == RSSL_CURL_ERROR)
	{
		_rsslSetError(error, NULL, RSSL_RET_FAILURE, errno);
		snprintf(error->text, MAX_RSSL_ERROR_TEXT,
			"%s", rsslSocketChannel->curlThreadInfo.error.text);
		ripcRelSocketChannel(rsslSocketChannel);
		return RSSL_RET_FAILURE;
	}

	inPr->types = RIPC_INPROG_NEW_FD;

	// Set the new FD and old FD on the ripcSessInProg structure
	inPr->oldSocket = rsslSocketChannel->stream;
	if ((curlret = (*(curlFuncs->curl_easy_getinfo))(rsslSocketChannel->curlHandle, CURLINFO_ACTIVESOCKET, (curl_socket_t*)&inPr->newSocket.stream)) != CURLE_OK)
	{
		_rsslSetError(error, NULL, RSSL_RET_FAILURE, errno);
		snprintf(error->text, MAX_RSSL_ERROR_TEXT,
			"<%s:%d> Error: 1000 Curl failed. %i\n",
			__FILE__, __LINE__, curlret);
		ripcRelSocketChannel(rsslSocketChannel);
		return RSSL_RET_FAILURE;
	}
	rsslSocketChannel->stream = inPr->newSocket.stream;

	if (rsslSocketChannel->connType == RSSL_CONN_TYPE_ENCRYPTED && rsslSocketChannel->usingWinInet == 0)
	{
		// This is where to go
		if (getSSLProtocolTransFuncs(rsslSocketChannel, rsslSocketChannel->sslProtocolBitmap) != 0)
		{
			_rsslSetError(error, NULL, RSSL_RET_FAILURE, errno);
			snprintf(error->text, MAX_RSSL_ERROR_TEXT,
				"<%s:%d> Error: 0012 Out of available SSL/TLS connection protocol options.\n",
				__FILE__, __LINE__);

			ripcRelSocketChannel(rsslSocketChannel);

			return(RIPC_CONN_ERROR);
		}
	}

	rsslSocketChannel->transportInfo = (*(rsslSocketChannel->transportFuncs->newClientConnection))(
		rsslSocketChannel->stream, &initcomplete, rsslSocketChannel, error);

		
	if (rsslSocketChannel->transportInfo == 0)
	{
		/*_rsslSetError(error, NULL, RSSL_RET_FAILURE, errno);
		snprintf(error->text, MAX_RSSL_ERROR_TEXT,
			"<%s:%d> Error: 0012 Error on new client connection.\n",
			__FILE__, __LINE__);*/

		ripcRelSocketChannel(rsslSocketChannel);

		return(RIPC_CONN_ERROR);
	}

	if ((rsslSocketChannel->connType == RSSL_CONN_TYPE_ENCRYPTED) && !initcomplete)
	{
		rsslSocketChannel->intState = RIPC_INT_ST_CLIENT_TRANSPORT_INIT;
	}

	if (rsslSocketChannel->connType == RSSL_CONN_TYPE_SOCKET)
	{
		// normal ripc connection - send connect request
		rsslSocketChannel->intState = RIPC_INT_ST_CONNECTING;
	}
	else if (rsslSocketChannel->connType == RSSL_CONN_TYPE_WEBSOCKET)
	{
		rsslSocketChannel->intState = RIPC_INT_ST_WS_SEND_OPENING_HANDSHAKE;
	}

	if ((ret = ipcIntSessInit(rsslSocketChannel, inPr, error)) < 0)
	{
		IPC_MUTEX_UNLOCK(rsslSocketChannel);
		return(ret);
	}

	inPr->types = RIPC_INPROG_NEW_FD;

	rssl_pipe_close(&rsslSocketChannel->sessPipe);

	return ret;

}


ripcSessInit ipcWaitAck(RsslSocketChannel *rsslSocketChannel, ripcSessInProg *inPr, RsslError *error, char* curBuf, RsslInt32 bufLen)
{
	char			buf[1024];
	rtr_msgb_t		*ackMsg = 0;
	RsslInt32		cc = 0;
	RsslInt32		idOffset = 0; /* will always be 0 if not tunneling */
	RsslInt32		opCode = 0;
	RsslUInt16		maxMsgSize = 0;
	RsslUInt16		msgLen = 0;
	RsslUInt32		versionNumber = 0;
	RsslUInt8 		flags = 0x0;
	RsslInt32		i = 0;
	RsslUInt8		componentVersionLen;
	RsslUInt8		componentStringLen;
	RsslInt32		tempIter;
	ripcRWFlags			rwflags = RIPC_RW_NONE;

	_DEBUG_TRACE_CONN("called\n")
	rwflags |= (rsslSocketChannel->blocking ? RIPC_RW_BLOCKING : 0);

	if (curBuf)   /* if this is the tail end of an HTTP OK Message */
	{
		MemCopyByInt(buf, curBuf, bufLen);
		cc = bufLen;
	}
	else
	{

		cc = (*(rsslSocketChannel->protocolFuncs->readTransportConnMsg))((void*)rsslSocketChannel, buf, 1024, rwflags, error);

	}

	if (rsslSocketChannel->workState & RIPC_INT_SHTDOWN_PEND)
	{
		_rsslSetError(error, NULL, RSSL_RET_FAILURE, errno);
		snprintf(error->text, MAX_RSSL_ERROR_TEXT,
			"<%s:%d> Error: 1003 ipcWaitAck() failed due to channel shutting down.\n",
			__FILE__, __LINE__);

		return(RIPC_CONN_ERROR);
	}

	_DEBUG_TRACE_READ("fd "SOCKET_PRINT_TYPE" cc %d err %d\n", rsslSocketChannel->stream, cc, ((cc <= 0) ? errno : 0))

	if (cc < 0)
	{
		/* Try to re-connect with an older header */
		switch (rsslSocketChannel->version->connVersion)
		{
		case CONN_VERSION_13:
			if (rsslSocketChannel->connType == RSSL_CONN_TYPE_EXT_LINE_SOCKET)
			{
				/* cant downgrade if EL, only supports version 14 and 13 */
				_rsslSetError(error, NULL, RSSL_RET_FAILURE, errno);
				snprintf(error->text, MAX_RSSL_ERROR_TEXT,
					"<%s:%d> Error: 1002 Could not read IPC Mount Ack.  Connection attempt has failed. System errno: (%d)\n",
					__FILE__, __LINE__, errno);

				return(RIPC_CONN_ERROR);
			}
			/* else fall through */
		case CONN_VERSION_14:
			return (ipcReconnectOld(rsslSocketChannel, inPr, error));
			break;
		case CONN_VERSION_12:
			/* only re-connect with older header for RWF protocol type */
			if (rsslSocketChannel->protocolType == RIPC_RWF_PROTOCOL_TYPE)	/* apps using version 11 shouldnt be using anything others than RWF */
			{
				return (ipcReconnectOld(rsslSocketChannel, inPr, error));
			}
			else
			{
				_rsslSetError(error, NULL, RSSL_RET_FAILURE, errno);
				snprintf(error->text, MAX_RSSL_ERROR_TEXT,
					"<%s:%d> Error: 1002 Could not read IPC Mount Ack.  Connection attempt has failed. System errno: (%d)\n",
					__FILE__, __LINE__, errno);

				return(RIPC_CONN_ERROR);
			}
			break;
		case CONN_VERSION_11:
			/* only re-connect with older header for non-tunneled connections */
			if (rsslSocketChannel->connType == RSSL_CONN_TYPE_SOCKET || (rsslSocketChannel->connType == RSSL_CONN_TYPE_ENCRYPTED && rsslSocketChannel->usingWinInet == 0))	/* tunneling doesnt exist in version 10 */
			{
				return (ipcReconnectOld(rsslSocketChannel, inPr, error));
			}
			else
			{
				_rsslSetError(error, NULL, RSSL_RET_FAILURE, errno);
                snprintf(error->text, MAX_RSSL_ERROR_TEXT,
					"<%s:%d> Error: 1002 Could not read IPC Mount Ack.  Connection attempt has failed. System errno: (%d)\n",
					__FILE__, __LINE__, errno);

				return(RIPC_CONN_ERROR);
			}
			break;
		default:
			_rsslSetError(error, NULL, RSSL_RET_FAILURE, errno);
			snprintf(error->text, MAX_RSSL_ERROR_TEXT,
				"<%s:%d> Error: 1002 Could not read IPC Mount Ack.  Connection attempt has failed. System errno: (%d)\n",
				__FILE__, __LINE__, errno);

			return(RIPC_CONN_ERROR);
			break;
		}
	}
	else if (cc == 0)
	{
		/* This will happen when there is no data atall to be read,
		and the read fails with error _IPC_WOULD_BLOCK or EINTR
		*/
		return (RIPC_CONN_IN_PROGRESS);
	}
	else if (rsslSocketChannel->dbgFlags & RSSL_DEBUG_IPC_DUMP_INIT)
	{
		(*(ripcDumpInFunc))(__FUNCTION__, buf, cc, rsslSocketChannel->stream);
	}

	if (cc < IPC_100_CONN_ACK)  /* 451 ACK is more so it safe to asume an error */
	{
		_rsslSetError(error, NULL, RSSL_RET_FAILURE, errno);
		snprintf(error->text, MAX_RSSL_ERROR_TEXT,
			"<%s:%d> Error: 1002 Invalid IPC Mount Ack. System errno: (%d)\n",
			__FILE__, __LINE__, errno);

		return(RIPC_CONN_ERROR);
	}

	_move_u16_swap(&msgLen, (buf + idOffset));
	flags = (unsigned char)buf[2 + idOffset];	/* 2 past the header length field */

	if (flags & IPC_EXTENDED_FLAGS)
		opCode = (unsigned char)buf[3 + idOffset]; /* 3 past the header length field */

	if (opCode & IPC_CONNNAK)
	{
		/* get nak text */
		if (msgLen > IPC_100_OTHER_HEADER_SIZE)
		{
			RsslUInt16 nakTextLen = 0;
			_move_u16_swap(&nakTextLen, (buf + 6 + idOffset));
			/* error text is present */
			if (nakTextLen > 0)
			{
				/* text is actually there */
				MemCopyByInt(error->text, buf + (8 + idOffset), nakTextLen);
			}
		}

		_rsslSetError(error, NULL, RSSL_RET_FAILURE, errno);
		snprintf(error->text, MAX_RSSL_ERROR_TEXT,
			"<%s:%d> Error: 1006 This connection has received a negative acknowledgement response from the server.\n",
			__FILE__, __LINE__);

		return(RIPC_CONN_REFUSED);
	}

	if (opCode & IPC_CONNACK)
	{
		_move_u32_swap(&versionNumber, (buf + 6 + idOffset));
	}

 	if (!(opCode & IPC_CONNACK))
	{
		_rsslSetError(error, NULL, RSSL_RET_FAILURE, errno);
		snprintf(error->text, MAX_RSSL_ERROR_TEXT,
			"<%s:%d> Error: 1002 Invalid IPC Mount Opcode: (%d)\n",
			__FILE__, __LINE__, opCode);

		return(RIPC_CONN_ERROR);
	}

	/* future versions will probably make use of the version number sent back in the ACK message. */
	{
		RsslUInt16 comp;

		switch (versionNumber)
		{
		case RIPC_VERSION_10:
			rsslSocketChannel->version = &ripc10Ver;
			break;
		case RIPC_VERSION_11:
			rsslSocketChannel->version = &ripc11Ver;
			break;
		case RIPC_VERSION_12:
			rsslSocketChannel->version = &ripc12Ver;
			break;
		case RIPC_VERSION_13:
			rsslSocketChannel->version = &ripc13Ver;
			break;
		case RIPC_VERSION_14:
			rsslSocketChannel->version = &ripc14Ver;
			break;
		default:
			/* version error */
			_rsslSetError(error, NULL, RSSL_RET_FAILURE, errno);
			snprintf(error->text, MAX_RSSL_ERROR_TEXT,
				"<%s:%d> Error: 1007 Invalid IPC Version: (%d)\n",
				__FILE__, __LINE__, versionNumber);

			return(RIPC_CONN_ERROR);
		}

		rsslSocketChannel->rsslFlags = (RsslInt32)buf[12 + idOffset];

		/* get ping interval */
		rsslSocketChannel->pingTimeout = (RsslUInt8)buf[13 + idOffset];
		rsslSocketChannel->majorVersion = (RsslUInt8)buf[14 + idOffset];
		rsslSocketChannel->minorVersion = (RsslUInt8)buf[15 + idOffset];
		_move_u16_swap(&comp, (buf + 16 + idOffset));
		if (versionNumber > RIPC_VERSION_10)
		{
			rsslSocketChannel->zlibCompLevel = (char)buf[18 + idOffset];
		}
		_move_u16_swap(&maxMsgSize, (buf + 10 + idOffset));
		/* Need to iterate and parse the handshake */
		/* Initialize iterator so it can be used through both of the following sections*/
		tempIter = 19 + idOffset;
		if ((rsslSocketChannel->keyExchange == 1) && (versionNumber > RIPC_VERSION_13))
		{
			/* 1 byte KEY_EXCHANGE
			1 byte type (TR_SL_1)
			8 byte P
			8 byte G
			8 byte send key
			*/
			RsslUInt8 encType = 0;
			RsslUInt8 encLen = 0;

			tempIter += 1;
			tempIter += rwfGet8(encType, (buf + tempIter));
			tempIter += rwfGet8(encLen, (buf + tempIter));
			/* Server tells us the type to do.  This version of product only knows TR_SL_1,
			* if we see soemthing else, skip it */
			if (encType == TR_SL_1)
			{
				tempIter += rwfGet64(rsslSocketChannel->P, (buf + tempIter));
				tempIter += rwfGet64(rsslSocketChannel->G, (buf + tempIter));
				tempIter += rwfGet64(rsslSocketChannel->send_key, (buf + tempIter));
				rsslSocketChannel->encryptionType = TR_SL_1;
			}
			else
			{
				/* zero out everything and skip the length */
				/* we do not know how to handle other types in this connection version */
				tempIter += encLen;
				rsslSocketChannel->encryptionType = 0;  /* cant do it */
			}
		}

		/* Component versioning is added in Version 13 and higher */
		if (versionNumber > RIPC_VERSION_12)
		{
			RsslInt32 skipIter = tempIter;  /* store our current position */
			/* get component versioning info */

			/* get overall length of component versioning */
			tempIter += rwfGet8(componentVersionLen, (buf + tempIter));
			if (componentVersionLen > 0)
			{
				/* get version string length */
				tempIter += rwfGet8(componentStringLen, (buf + tempIter));
				/* write version string */
				if (componentStringLen > 0)
				{
					/* cant put this in the same place we got it from  because ack isnt sent until after this */
					/* get the version string - with ripc, there will only ever be one */
					rsslSocketChannel->outComponentVer = (char*)_rsslMalloc(componentStringLen + 1);
					/* copy string here */
					MemCopyByInt(rsslSocketChannel->outComponentVer, (buf + tempIter), componentStringLen);
					rsslSocketChannel->outComponentVer[componentStringLen] = '\0';
					rsslSocketChannel->outComponentVerLen = componentStringLen;
					tempIter += componentStringLen;
				}
			}
			/* in case we need it later, update tempIter to skip over full componentVersionLen (in case we add other things that this version of code is unaware of) */
			tempIter = skipIter + componentVersionLen;
		}

		/* set up compression */
		if (comp > RSSL_COMP_MAX_TYPE)
		{
			_rsslSetError(error, NULL, RSSL_RET_FAILURE, errno);
			snprintf(error->text, MAX_RSSL_ERROR_TEXT,
				"<%s:%d> Error: 1007 Server has specified an unknown compression type (%d)\n",
				__FILE__, __LINE__, comp);

			return(RIPC_CONN_ERROR);
		}

		/* Version specific compression */
		switch (versionNumber)
		{
		case RIPC_VERSION_10:
		{
			rsslSocketChannel->inDecompress = comp;
			if (comp)
			{
				rsslSocketChannel->inDecompFuncs = &(compressFuncs[comp]);
				if ((rsslSocketChannel->inDecompFuncs->decompressInit == 0) ||
					(rsslSocketChannel->inDecompFuncs->decompressEnd == 0) ||
					(rsslSocketChannel->inDecompFuncs->decompress == 0))
				{
					rsslSocketChannel->inDecompFuncs = 0;

					_rsslSetError(error, NULL, RSSL_RET_FAILURE, errno);
					snprintf(error->text, MAX_RSSL_ERROR_TEXT,
						"<%s:%d> Error: 1007 Server has specified an unknown compression type (%d)\n",
						__FILE__, __LINE__, comp);

					return(RIPC_CONN_ERROR);
				}
			}
			if (rsslSocketChannel->inDecompFuncs)
			{
				_DEBUG_TRACE_CONN("about to initialize decompression\n")

				rsslSocketChannel->c_stream_in = (*(rsslSocketChannel->inDecompFuncs->decompressInit))(0, error);
				if (rsslSocketChannel->c_stream_in == 0)
				{
					_rsslSetError(error, NULL, RSSL_RET_FAILURE, errno);

					return(RIPC_CONN_ERROR);
				}
			}
		}
		break;
		case RIPC_VERSION_11:
		case RIPC_VERSION_12:
		case RIPC_VERSION_13:
		case RIPC_VERSION_14:
		{
			if ((flags & RSSL_COMP_FORCE) && (!comp))
				comp = RSSL_COMP_ZLIB;

			rsslSocketChannel->inDecompress = comp;
			rsslSocketChannel->outCompression = comp;

			/* LZ4 cannot compress and span multiple buffers when content grows as a result of compression.
			* We want to know where the threshold for this is, and when it can grow we want to compress
			* into a larger buffer and then split it across two buffers.
			*/
			if (comp == RSSL_COMP_LZ4)
			{
				RsslInt32 i = maxMsgSize;

				while (LZ4_compressBound(i) > maxMsgSize && i > 0)
				{
					i--;
				}
				if (i <= 0)
				{
					_rsslSetError(error, NULL, RSSL_RET_FAILURE, errno);
					snprintf(error->text, MAX_RSSL_ERROR_TEXT,
						"<%s:%d> Error: 1007 Could not calculate client upperCompressionThreshold for maxUserMsgSize = %d\n",
						__FILE__, __LINE__, maxMsgSize);

					return(RIPC_CONN_ERROR);
				}
				/* this is the threshold that we will not compress directly into the output buffer, but use the intermediate */
				rsslSocketChannel->upperCompressionThreshold = i;

				/* Create buffer to compress into and decompress from */
				i = LZ4_compressBound(maxMsgSize);
				i += 5; /* add in small fudge factor to ensure these buffers are larger than needed */
				rsslSocketChannel->tempCompressBuf = rtr_smplcAllocMsg(gblInputBufs, i);
				if (rsslSocketChannel->tempCompressBuf == 0)
				{
					_rsslSetError(error, NULL, RSSL_RET_FAILURE, errno);
					snprintf(error->text, MAX_RSSL_ERROR_TEXT,
						"<%s:%d> Error: 1001 Could not allocate buffer memory for LZ compression.\n",
						__FILE__, __LINE__);

					return(RIPC_CONN_ERROR);
				}
				rsslSocketChannel->tempDecompressBuf = rtr_smplcAllocMsg(gblInputBufs, i);
				if (rsslSocketChannel->tempDecompressBuf == 0)
				{
					_rsslSetError(error, NULL, RSSL_RET_FAILURE, errno);
					snprintf(error->text, MAX_RSSL_ERROR_TEXT,
						"<%s:%d> Error: 1001 Could not allocate buffer memory for LZ decompression.\n",
						__FILE__, __LINE__);

					return(RIPC_CONN_ERROR);
				}
			}
			else
				rsslSocketChannel->upperCompressionThreshold = maxMsgSize;
			if (comp)
			{
				rsslSocketChannel->inDecompFuncs = &(compressFuncs[comp]);
				rsslSocketChannel->outCompFuncs = &(compressFuncs[comp]);
				if ((rsslSocketChannel->inDecompFuncs->decompressInit == 0) ||
					(rsslSocketChannel->inDecompFuncs->decompressEnd == 0) ||
					(rsslSocketChannel->inDecompFuncs->decompress == 0) ||
					(rsslSocketChannel->outCompFuncs->compress == 0) ||
					(rsslSocketChannel->outCompFuncs->compressEnd == 0) ||
					(rsslSocketChannel->outCompFuncs->compressInit == 0))
				{
					rsslSocketChannel->inDecompFuncs = 0;
					rsslSocketChannel->outCompFuncs = 0;

					_rsslSetError(error, NULL, RSSL_RET_FAILURE, errno);
					snprintf(error->text, MAX_RSSL_ERROR_TEXT,
						"<%s:%d> Error: 1007 Server has specified an unknown compression type (%d)\n",
						__FILE__, __LINE__, comp);

					return(RIPC_CONN_ERROR);
				}
			}
			if (rsslSocketChannel->inDecompFuncs)
			{
				_DEBUG_TRACE_CONN("about to initialize decompression\n")

				rsslSocketChannel->c_stream_in = (*(rsslSocketChannel->inDecompFuncs->decompressInit))(0, error);
				if (rsslSocketChannel->c_stream_in == 0)
				{
					_rsslSetError(error, NULL, RSSL_RET_FAILURE, errno);

					return(RIPC_CONN_ERROR);
				}
			}
			if (rsslSocketChannel->outCompFuncs)
			{
				_DEBUG_TRACE_CONN("about to initialize compression\n")
				rsslSocketChannel->c_stream_out = (*(rsslSocketChannel->outCompFuncs->compressInit))(rsslSocketChannel->zlibCompLevel, 0, error);
				if (rsslSocketChannel->c_stream_out == 0)
				{
					_rsslSetError(error, NULL, RSSL_RET_FAILURE, errno);

					return(RIPC_CONN_ERROR);
				}
			}
		}
		break;
		default:
			/* version error */
			_rsslSetError(error, NULL, RSSL_RET_FAILURE, errno);
			snprintf(error->text, MAX_RSSL_ERROR_TEXT,
					"<%s:%d> Error: 1007 Invalid IPC Version: (%d)\n",
				__FILE__, __LINE__, versionNumber);
			return(RIPC_CONN_ERROR);
		}

		rsslSocketChannel->maxUserMsgSize = maxMsgSize;
		rsslSocketChannel->maxMsgSize = maxMsgSize + rsslSocketChannel->version->dataHeaderLen + RWS_MAX_HEADER_SIZE;
	}

	_DEBUG_TRACE_CONN("Session active ConnVer 0x%x ripcVer 0x%x comp %d\n", rsslSocketChannel->version->connVersion, versionNumber, rsslSocketChannel->inDecompress)

	/* Initialize the input buffer */
	rsslSocketChannel->inputBuffer = rtr_smplcAllocMsg(gblInputBufs, (rsslSocketChannel->maxMsgSize * rsslSocketChannel->readSize));

	if (rsslSocketChannel->inDecompress)
	{
		rsslSocketChannel->decompressBuf = rtr_smplcAllocMsg(gblInputBufs, rsslSocketChannel->maxMsgSize);
		if (rsslSocketChannel->decompressBuf == 0)
		{
			_rsslSetError(error, NULL, RSSL_RET_FAILURE, errno);
			snprintf(error->text, MAX_RSSL_ERROR_TEXT,
				"<%s:%d> Error: 1001 Could not compression allocate buffer memory.\n",
				__FILE__, __LINE__);

			return(RIPC_CONN_ERROR);
		}
		rsslSocketChannel->curInputBuf = rtr_smplcDupMsg(gblInputBufs, rsslSocketChannel->decompressBuf);
	}
	else
		rsslSocketChannel->curInputBuf = rtr_smplcDupMsg(gblInputBufs, rsslSocketChannel->inputBuffer);

	if ((rsslSocketChannel->inputBuffer == 0) || (rsslSocketChannel->curInputBuf == 0))
	{
		_rsslSetError(error, NULL, RSSL_RET_FAILURE, errno);
		snprintf(error->text, MAX_RSSL_ERROR_TEXT,
			"<%s:%d> Error: 1001 Could not allocate compression buffer memory.\n",
			__FILE__, __LINE__);

		return(RIPC_CONN_ERROR);
	}
	rsslSocketChannel->readSize = (RsslInt32)rsslSocketChannel->inputBuffer->maxLength / 2;

	if ((rsslSocketChannel->guarBufPool->sharedPool) &&
		(rsslSocketChannel->guarBufPool->sharedPool->initialized == 0))
	{
		if (rtrBufferPoolFinishInit(rsslSocketChannel->guarBufPool->sharedPool, rsslSocketChannel->maxMsgSize) < 0)
		{
			_rsslSetError(error, NULL, RSSL_RET_FAILURE, errno);
			snprintf(error->text, MAX_RSSL_ERROR_TEXT,
				"<%s:%d> Error: 1001 Could not initialize shared buffer pool.\n",
				__FILE__, __LINE__);

			return(RIPC_CONN_ERROR);
		}
	}

	if (rtr_dfltcSetBufSize(rsslSocketChannel->guarBufPool, rsslSocketChannel->maxMsgSize) < 0)
	{
		_rsslSetError(error, NULL, RSSL_RET_FAILURE, errno);
		snprintf(error->text, MAX_RSSL_ERROR_TEXT,
			"<%s:%d> Error: 1001 Could not allocate buffer memory\n",
			__FILE__, __LINE__);

		return(RIPC_CONN_ERROR);
	}

	/* priority write stuff */
	for (i = 0; i < RIPC_MAX_PRIORITY_QUEUE; i++);
	{
		rsslInitQueue(&rsslSocketChannel->priorityQueues[i].priorityQueue);
		rsslSocketChannel->priorityQueues[i].queueLength = 0;
	}

	/* this may change if I let them pass it in off bind/connect */
	rsslSocketChannel->flushStrategy[0] = 0;
	rsslSocketChannel->flushStrategy[1] = 1;
	rsslSocketChannel->flushStrategy[2] = 0;
	rsslSocketChannel->flushStrategy[3] = 2;
	rsslSocketChannel->flushStrategy[4] = 0;
	rsslSocketChannel->flushStrategy[5] = 1;
	rsslSocketChannel->flushStrategy[6] = -1;

	rsslSocketChannel->currentOutList = 0;

	rsslSocketChannel->compressQueue = -1;
	rsslSocketChannel->nextOutBuf = -1;

	/* If we read more bytes than in the header, put them
	* into the read buffer. This can happen in quick applications
	* that send data as soon as the channel becomes active.
	*/
	if (cc > (RsslInt32)msgLen + idOffset)
	{
		RsslInt32 putback = cc - msgLen;
		MemCopyByInt(rsslSocketChannel->inputBuffer->buffer, (buf + msgLen), putback);
		rsslSocketChannel->inputBuffer->length += putback;
	}

	if (rsslSocketChannel->keyExchange == 1)
	{
		rsslSocketChannel->intState = RIPC_INT_ST_SEND_CLIENT_KEY;
		return (ipcSendClientKey(rsslSocketChannel, inPr, error));
	}
	else
	{
		if (ipcGetSocketRow(rsslSocketChannel, error) == RSSL_RET_FAILURE)
		{
			return(RIPC_CONN_ERROR);
		}

		rsslSocketChannel->state = RSSL_CH_STATE_ACTIVE;
		rsslSocketChannel->intState = RIPC_INT_ST_ACTIVE;

		/* Set default compression threshold according to negotiated compression type. */
		if (rsslSocketChannel->lowerCompressionThreshold == 0)
		{
			switch (rsslSocketChannel->outCompression)
			{
			case RSSL_COMP_ZLIB:
				rsslSocketChannel->lowerCompressionThreshold = RSSL_COMP_DFLT_THRESHOLD_ZLIB;
				break;
			case RSSL_COMP_LZ4:
				rsslSocketChannel->lowerCompressionThreshold = RSSL_COMP_DFLT_THRESHOLD_LZ4;
				break;
			default:
				break;
			}
		}

		return(RIPC_CONN_ACTIVE);
	}
}

/***************************
 * START NON-PUBLIC ABSTRACTED FUNCTIONS
 ***************************/

/* SocketBind impl */
RsslRet rsslSocketBind(rsslServerImpl* rsslSrvrImpl, RsslBindOptions *opts, RsslError *error )
{
	RsslServerSocketChannel*	rsslServerSocketChannel = NULL;
	rtr_bufferpool_t	*serverPool=0; /* buffer pool */
	RsslUInt32			poolSize = 0;	/* number of pool buffers */
	RsslInt32			connType = opts->connectionType;
	RsslInt32			tempLen = 0;
	RsslInt32			retCode = 0;

	if (opts->numInputBuffers <= 1)
	{
		_rsslSetError(error, NULL, RSSL_RET_FAILURE, 0);
		snprintf(error->text, MAX_RSSL_ERROR_TEXT,
			"<%s:%d> Error: 1004 invalid number of input buffers specified(<%d>), must be at least <%d>.\n",
			__FILE__, __LINE__, opts->numInputBuffers, 2);
		return RSSL_RET_FAILURE;
	}

	if (opts->guaranteedOutputBuffers < 1)
	{
		_rsslSetError(error, NULL, RSSL_RET_FAILURE, 0);
		snprintf(error->text, MAX_RSSL_ERROR_TEXT,
			"<%s:%d> Error: 1004 invalid number of guaranteed output buffers <%d>, must be at least <%d>.\n",
			__FILE__, __LINE__, opts->guaranteedOutputBuffers, 1);
		return RSSL_RET_FAILURE;
	}

	if ((opts->connectionType != RSSL_CONN_TYPE_SOCKET) && 
		(opts->connectionType != RSSL_CONN_TYPE_ENCRYPTED) && 
		(opts->connectionType != RSSL_CONN_TYPE_HTTP) && 
		(opts->connectionType != RSSL_CONN_TYPE_EXT_LINE_SOCKET) &&
		(opts->connectionType != RSSL_CONN_TYPE_WEBSOCKET))
	{
		_rsslSetError(error, NULL, RSSL_RET_INVALID_ARGUMENT, 0);
		snprintf(error->text, MAX_RSSL_ERROR_TEXT, "<%s:%d> Error: 0006 Connection type %d is not supported.\n", __FILE__, __LINE__, opts->connectionType);
		return RSSL_RET_FAILURE;
	}

	if (ipcValidServerName(opts->serviceName, MAX_SERV_NAME) < 0)
	{
		_rsslSetError(error, NULL, RSSL_RET_FAILURE, 0);
		snprintf(error->text, MAX_RSSL_ERROR_TEXT, "<%s:%d> Error: 1004 Invalid serviceName or port number specified <%s>\n",
			__FILE__, __LINE__, opts->serviceName);
		return RSSL_RET_FAILURE;
	}

	if ((opts->compressionType & ~RSSL_COMP_ALL_TYPE) != 0)
	{
		_rsslSetError(error, NULL, RSSL_RET_FAILURE, 0);
		snprintf(error->text, MAX_RSSL_ERROR_TEXT, "<%s:%d> Error: 1004 Invalid compression type <0x%x>\n",
			__FILE__, __LINE__, opts->compressionType);
		return RSSL_RET_FAILURE;
	}

	// make sure that only one compression type is specified if using forced compression
	if(opts->compressionType && opts->forceCompression)
	{
		RsslUInt32 i, count = 0;

		for (i = 0; i<32; i++)
		{
			if ((1 << i) & opts->compressionType)
				count++;
		}
		if (count == 0)
		{
			_rsslSetError(error, NULL, RSSL_RET_FAILURE, 0);
			snprintf(error->text, MAX_RSSL_ERROR_TEXT, "<%s:%d> Error: 1004 Force compression cannot be used without a valid compression type.\n", __FILE__, __LINE__);
			return RSSL_RET_FAILURE;
		}
		if (count > 1)
		{
			_rsslSetError(error, NULL, RSSL_RET_FAILURE, 0);
			snprintf(error->text, MAX_RIPC_ERROR_TEXT, "<%s:%d> Error: 1004 Cannot specify multiple compressionTypes when using force compression.\n", __FILE__, __LINE__);
			return RSSL_RET_FAILURE;
		}
	}

	/* Set the transportInfo here so any further failures are properly handled when the calling function cleans them up */
	rsslServerSocketChannel = newRsslServerSocketChannel();
	rsslSrvrImpl->transportInfo = rsslServerSocketChannel;

	if (rsslServerSocketChannel == 0)
	{
		_rsslSetError(error, NULL, RSSL_RET_FAILURE, 0);
		snprintf(error->text, MAX_RSSL_ERROR_TEXT, "<%s:%d> Error: 0005 Could not allocate memory for binding socket.\n", __FILE__, __LINE__);
		return RSSL_RET_FAILURE;
	}

	/* added for infra group */
	if ((opts->maxFragmentSize >= RSSL_MIN_FRAG_SIZE) && (opts->maxFragmentSize <= 0xFFFF))
		rsslServerSocketChannel->maxMsgSize = opts->maxFragmentSize;
	else
		rsslServerSocketChannel->maxMsgSize = RSSL_MAX_MSG_SIZE;

	rsslServerSocketChannel->interfaceName = opts->interfaceName;

	/* setup compression */
	rsslServerSocketChannel->compressionSupported = opts->compressionType;
	/* set to default compression - may want to expose this if we decide
	   to start using compression */
	if (opts->compressionType != 0)
	{
		rsslServerSocketChannel->zlibCompressionLevel = opts->compressionLevel;
		rsslServerSocketChannel->forcecomp = opts->forceCompression;
	}

	rsslServerSocketChannel->server_blocking = (opts->serverBlocking ? 1 : 0);
	rsslServerSocketChannel->session_blocking = (opts->channelsBlocking ? 1 : 0);

	/* check to make sure ping timeouts are valid - we set to 0 if client and server pings are off */
	/* 0 implies no pinging */

	if ((opts->minPingTimeout > 0) && (opts->minPingTimeout <= IPC_MAXIMUM_PINGTIMEOUT))
		rsslServerSocketChannel->minPingTimeout = opts->minPingTimeout;
	else if ((opts->minPingTimeout == 0) && (opts->clientToServerPings || opts->serverToClientPings))
		rsslServerSocketChannel->minPingTimeout = IPC_MINIMUM_PINGTIMEOUT;
	else if (opts->pingTimeout > IPC_MAXIMUM_PINGTIMEOUT)
		rsslServerSocketChannel->minPingTimeout = IPC_MAXIMUM_PINGTIMEOUT;

	if ((opts->pingTimeout > 0) && (opts->pingTimeout <= IPC_MAXIMUM_PINGTIMEOUT) && (opts->pingTimeout >= (RsslUInt32)rsslServerSocketChannel->minPingTimeout))
		rsslServerSocketChannel->pingTimeout = opts->pingTimeout;
	else if ((opts->pingTimeout == 0) && (opts->clientToServerPings || opts->serverToClientPings))
		rsslServerSocketChannel->pingTimeout = IPC_MINIMUM_PINGTIMEOUT;
	else if (opts->pingTimeout > IPC_MAXIMUM_PINGTIMEOUT)
		rsslServerSocketChannel->pingTimeout = IPC_MAXIMUM_PINGTIMEOUT;
	else if (opts->pingTimeout < (RsslUInt32)rsslServerSocketChannel->minPingTimeout)
		rsslServerSocketChannel->pingTimeout = (RsslUInt32)rsslServerSocketChannel->minPingTimeout;

	rsslServerSocketChannel->majorVersion = opts->majorVersion;
	rsslServerSocketChannel->minorVersion = opts->minorVersion;
	rsslServerSocketChannel->protocolType = opts->protocolType;

	poolSize = opts->sharedPoolSize ? opts->sharedPoolSize : RSSL_POOL_SIZE;

	/* if either are true, set to true, else set to false */
	if (opts->tcpOpts.tcp_nodelay)
		rsslServerSocketChannel->tcp_nodelay = opts->tcpOpts.tcp_nodelay;
	else if (opts->tcp_nodelay)
		rsslServerSocketChannel->tcp_nodelay = opts->tcp_nodelay;
	else
		rsslServerSocketChannel->tcp_nodelay = 0;

	if (opts->maxOutputBuffers < opts->guaranteedOutputBuffers)
		rsslServerSocketChannel->maxNumMsgs = opts->guaranteedOutputBuffers;
	else
		rsslServerSocketChannel->maxNumMsgs = opts->maxOutputBuffers;

	if ((rsslServerSocketChannel->maxNumMsgs - opts->guaranteedOutputBuffers) > poolSize)
	{
		/* configured to be able to get more shared pool buffers than the shared pool will contain */
		_rsslSetError(error, NULL, RSSL_RET_INVALID_ARGUMENT, 0);
		snprintf(error->text, MAX_RSSL_ERROR_TEXT, "<%s:%d> Error: 0012 SharedPoolSize (%d) is less than configured number of shared pool buffers allowed per connection (%d).\n",
				__FILE__, __LINE__, poolSize, (rsslServerSocketChannel->maxNumMsgs - opts->guaranteedOutputBuffers));
		poolSize = rsslServerSocketChannel->maxNumMsgs - opts->guaranteedOutputBuffers;
	}

	rsslServerSocketChannel->maxGuarMsgs = opts->guaranteedOutputBuffers;

	rsslServerSocketChannel->numInputBufs = opts->numInputBuffers;

	/* Create buffer pool for server */
	if (opts->sharedPoolLock)
	{
	    (void) RSSL_MUTEX_INIT_ESDK(&rsslSrvrImpl->sharedBufPoolMutex);
		rsslSrvrImpl->hasSharedBufPool = RSSL_TRUE;
		serverPool = ipcCreatePool(poolSize, &(rsslSrvrImpl->sharedBufPoolMutex));
	}
	else
	{
		rsslSrvrImpl->hasSharedBufPool = RSSL_FALSE;
		serverPool = ipcCreatePool(poolSize, 0);
	}

	if (serverPool == NULL)
	{
		_rsslSetError(error, NULL, RSSL_RET_BUFFER_NO_BUFFERS,  0);
		snprintf(error->text, MAX_RSSL_ERROR_TEXT, "<%s:%d> Error: 0005 Unable to create buffer pool.\n", __FILE__, __LINE__);
		return RSSL_RET_FAILURE;
	}
	else
		rsslServerSocketChannel->sharedBufPool = serverPool;

	/* End pool creation */

	/* If the connType is encrypted, initialize openSSL and copy all of the configuration parameters */
	if (connType == RSSL_CONN_TYPE_ENCRYPTED)
	{
		if (openSSLInit == 0)
		{
			_rsslSetError(error, NULL, RSSL_RET_FAILURE, errno);
			snprintf(error->text, MAX_RSSL_ERROR_TEXT,
				"<%s:%d> Error: 0012 OpenSSL Libraries have not been loaded.\n",
				__FILE__, __LINE__);
			return RSSL_RET_FAILURE;
		}

		if (opts->encryptionOpts.encryptionProtocolFlags == 0)
		{
			_rsslSetError(error, NULL, RSSL_RET_FAILURE, 0);
			snprintf(error->text, MAX_RSSL_ERROR_TEXT,
				"<%s:%d> Error: 0012 No valid TLS encryption protocol set.\n",
				__FILE__, __LINE__);
			return RSSL_RET_FAILURE;
		}

		rsslServerSocketChannel->encryptionProtocolFlags = opts->encryptionOpts.encryptionProtocolFlags | RIPC_PROTO_SSL_TLS;

		rsslServerSocketChannel->encryptionProtocolFlags &= ripcGetSupportedProtocolFlags();
		if (rsslServerSocketChannel->encryptionProtocolFlags == 0)
		{
			_rsslSetError(error, NULL, RSSL_RET_FAILURE, 0);
			snprintf(error->text, MAX_RSSL_ERROR_TEXT,
				"<%s:%d> Error: 0012 OpenSSL does not support any of the configured TLS versions.\n",
				__FILE__, __LINE__);
			return RSSL_RET_FAILURE;
		}

		if (opts->encryptionOpts.cipherSuite != NULL && strlen(opts->encryptionOpts.cipherSuite) != 0)
		{
			tempLen = (RsslInt32)(strlen(opts->encryptionOpts.cipherSuite) + 1);
			rsslServerSocketChannel->cipherSuite = (char*)strcpy((char*)_rsslMalloc(tempLen), opts->encryptionOpts.cipherSuite);
			if (rsslServerSocketChannel->cipherSuite == 0)
			{
				_rsslSetError(error, NULL, RSSL_RET_FAILURE, errno);
				snprintf(error->text, MAX_RIPC_ERROR_TEXT, "<%s:%d> Error: 1001 Failed to allocate or copy the encryption cipherSuite. System errno: (%d)\n",
					__FILE__, __LINE__, errno);
				transFuncs[rsslServerSocketChannel->connType].shutdownSrvrError(rsslSrvrImpl);
				return RSSL_RET_FAILURE;
			}
		}

		if (opts->encryptionOpts.dhParams != NULL && strlen(opts->encryptionOpts.dhParams) != 0)
		{
			tempLen = (RsslInt32)(strlen(opts->encryptionOpts.dhParams) + 1);
			rsslServerSocketChannel->dhParams = (char*)strcpy((char*)_rsslMalloc(tempLen), opts->encryptionOpts.dhParams);
			if (rsslServerSocketChannel->dhParams == 0)
			{
				_rsslSetError(error, NULL, RSSL_RET_FAILURE, errno);
				snprintf(error->text, MAX_RIPC_ERROR_TEXT, "<%s:%d> Error: 1001 Failed to allocate or copy the encryption dhParams. System errno: (%d)\n",
					__FILE__, __LINE__, errno);
				transFuncs[rsslServerSocketChannel->connType].shutdownSrvrError(rsslSrvrImpl);
				return RSSL_RET_FAILURE;
			}
		}

		if (opts->encryptionOpts.serverCert != NULL && strlen(opts->encryptionOpts.serverCert) != 0)
		{
			tempLen = (RsslInt32)(strlen(opts->encryptionOpts.serverCert) + 1);
			rsslServerSocketChannel->serverCert = (char*)strcpy((char*)_rsslMalloc(tempLen), opts->encryptionOpts.serverCert);
			if (rsslServerSocketChannel->serverCert == 0)
			{
				_rsslSetError(error, NULL, RSSL_RET_FAILURE, errno);
				snprintf(error->text, MAX_RIPC_ERROR_TEXT, "<%s:%d> Error: 1001 Failed to allocate or copy the encryption dhParams. System errno: (%d)\n",
					__FILE__, __LINE__, errno);
				transFuncs[rsslServerSocketChannel->connType].shutdownSrvrError(rsslSrvrImpl);
				return RSSL_RET_FAILURE;
			}
		}

		if (opts->encryptionOpts.serverPrivateKey != NULL && strlen(opts->encryptionOpts.serverPrivateKey) != 0)
		{
			tempLen = (RsslInt32)(strlen(opts->encryptionOpts.serverPrivateKey) + 1);
			rsslServerSocketChannel->serverPrivateKey = (char*)strcpy((char*)_rsslMalloc(tempLen), opts->encryptionOpts.serverPrivateKey);
			if (rsslServerSocketChannel->serverPrivateKey == 0)
			{
				_rsslSetError(error, NULL, RSSL_RET_FAILURE, errno);
				snprintf(error->text, MAX_RIPC_ERROR_TEXT, "<%s:%d> Error: 1001 Failed to allocate or copy the encryption dhParams. System errno: (%d)\n",
					__FILE__, __LINE__, errno);
				transFuncs[rsslServerSocketChannel->connType].shutdownSrvrError(rsslSrvrImpl);
				return RSSL_RET_FAILURE;
			}
		}

	}

	tempLen = (RsslInt32)(strlen(opts->serviceName) + 1);
	rsslServerSocketChannel->serverName = (char*)strcpy((char*)_rsslMalloc(tempLen), opts->serviceName);
	if (rsslServerSocketChannel->serverName == 0)
	{
		_rsslSetError(error, NULL, RSSL_RET_FAILURE, errno);
		snprintf(error->text, MAX_RIPC_ERROR_TEXT, "<%s:%d> Error: 1001 Failed to allocate or copy serverName. System errno: (%d)\n",
			__FILE__, __LINE__, errno);
		transFuncs[rsslServerSocketChannel->connType].shutdownSrvrError(rsslSrvrImpl);
		return RSSL_RET_FAILURE;
	}

	/* if WININET tunneling, change to socket here */
	if (connType == RSSL_CONN_TYPE_HTTP || connType == RSSL_CONN_TYPE_WEBSOCKET)
		connType = RSSL_CONN_TYPE_SOCKET;

	if (connType == RSSL_CONN_TYPE_EXT_LINE_SOCKET)
	{
		if (rsslLoadInitTransport(&(transFuncs[RSSL_CONN_TYPE_EXT_LINE_SOCKET]),
											0,
											transOpts.initConfig) < 0)
		{
			_rsslSetError(error, NULL, RSSL_RET_FAILURE, errno);
			snprintf(error->text, MAX_RIPC_ERROR_TEXT,
					"<%s:%d> Error: 1008 Unable to set Extended Line Socket functions (%d).",
					__FILE__, __LINE__, errno);
			if (transFuncs[rsslServerSocketChannel->connType].shutdownSrvrError)
				transFuncs[rsslServerSocketChannel->connType].shutdownSrvrError(rsslSrvrImpl);
			return RSSL_RET_FAILURE;
		}

	}

	rsslServerSocketChannel->connType = connType;
	
	rsslSrvrImpl->serverSharedSocket = opts->serverSharedSocket;

	if ((retCode = (transFuncs[connType].bindSrvr(rsslSrvrImpl, error))) < 0)
	{
		return RSSL_RET_FAILURE;
	}

	if (rsslServerSocketChannel->connType == RSSL_CONN_TYPE_ENCRYPTED)
	{
		if (getSSLTransFuncs()->newSSLServer == 0)
		{
			_rsslSetError(error, NULL, RSSL_RET_FAILURE, 0);
			snprintf(error->text, MAX_RSSL_ERROR_TEXT,
				"<%s:%d> Cannot enable ssl. Secure Sockets layer library not initialized. ",
				__FILE__, __LINE__);
			sock_close(rsslServerSocketChannel->stream);
			return RSSL_RET_FAILURE;
		}
		else
		{
			rsslServerSocketChannel->connType = RSSL_CONN_TYPE_ENCRYPTED;
			rsslServerSocketChannel->transportInfo = (*(getSSLTransFuncs()->newSSLServer))(rsslServerSocketChannel, error);

			if (rsslServerSocketChannel->transportInfo == 0)
			{
				_rsslSetError(error, NULL, RSSL_RET_FAILURE, errno);
				sock_close(rsslServerSocketChannel->stream);
				return RSSL_RET_FAILURE;
			}
		}
	}

	//set rsslServerSocketChannel->mutex when global lock and per-channel locks enabled
	if (multiThread == RSSL_LOCK_GLOBAL_AND_CHANNEL)
		rsslServerSocketChannel->mutex = &(rsslSrvrImpl->srvrMutex);

	rsslServerSocketChannel->state = RSSL_CH_STATE_ACTIVE;

	rsslServerSocketChannel->maxUserMsgSize = rsslServerSocketChannel->maxMsgSize;
	rsslServerSocketChannel->maxMsgSize = V10_MIN_HDR + rsslServerSocketChannel->maxMsgSize + 8 + RWS_MAX_HEADER_SIZE; // the 8 is for the maximum http overhead if we are tunneling

	if (opts->serverToClientPings)
		rsslServerSocketChannel->rsslFlags |= 0x2;
	if (opts->clientToServerPings)
		rsslServerSocketChannel->rsslFlags |= 0x1;

	/* Set the buffer pool for this server */
	if (rsslServerSocketChannel->sharedBufPool)
	{
		rtrBufferPoolAddRef(rsslServerSocketChannel->sharedBufPool);
		if (rsslServerSocketChannel->sharedBufPool->initialized == 0)
		{
			if (rtrBufferPoolFinishInit(rsslServerSocketChannel->sharedBufPool, rsslServerSocketChannel->maxMsgSize) < 0)
			{
				_rsslSetError(error, NULL, RSSL_RET_FAILURE, errno);
				snprintf(error->text, MAX_RSSL_ERROR_TEXT, "<%s:%d> Error: 1001 Failed to initialize shared buffer pool.\n",
					__FILE__, __LINE__);
				transFuncs[rsslServerSocketChannel->connType].shutdownSrvrError(rsslSrvrImpl);
				return RSSL_RET_FAILURE;
			}
		}
	}

	rsslSrvrImpl->Server.socketId = (RsslSocket)rsslServerSocketChannel->stream;
	rsslSrvrImpl->Server.state = rsslServerSocketChannel->state;
	rsslSrvrImpl->transportInfo = (void*)rsslServerSocketChannel;
	rsslSrvrImpl->Server.portNumber = net2host_u16(ipcGetServByName(rsslServerSocketChannel->serverName));

	/* this connection type is mainly used to determine between transport types */
	rsslSrvrImpl->connectionType = RSSL_CONN_TYPE_SOCKET;

	rsslSrvrImpl->Server.userSpecPtr = opts->userSpecPtr;
	/* put the server state at active */
	rsslSrvrImpl->Server.state = RSSL_CH_STATE_ACTIVE;

	/* set isBlocking flag */
	rsslSrvrImpl->isBlocking = opts->serverBlocking;

	/* bridge through send/recv buffer sizes for calls to accept */
	rsslSrvrImpl->sendBufSize = opts->sysSendBufSize;
	rsslSrvrImpl->recvBufSize = opts->sysRecvBufSize;

	/*store user defined component version info from bind options, if it's present*/
	if (opts->componentVersion  != NULL)
	{
		rsslSrvrImpl->connOptsCompVer.componentVersion.length = (RsslUInt32)strlen(opts->componentVersion);
		rsslSrvrImpl->connOptsCompVer.componentVersion.data = _rsslMalloc(rsslSrvrImpl->connOptsCompVer.componentVersion.length);
		MemCopyByInt(rsslSrvrImpl->connOptsCompVer.componentVersion.data, opts->componentVersion, rsslSrvrImpl->connOptsCompVer.componentVersion.length);
	}

	if (opts->wsOpts.protocols != 0 && opts->wsOpts.protocols[0] != '\0')
	{
		if (rwsInitServerOptions(rsslServerSocketChannel, &(opts->wsOpts), error) == RSSL_RET_FAILURE)	
		{
			_rsslSetError(error, NULL, RSSL_RET_FAILURE, errno);
			snprintf(error->text+strlen(error->text), MAX_RSSL_ERROR_TEXT,
				"<%s:%d> Error: 1001 Failed to Init WS Server struct. System errno: (%d)\n",
				__FILE__, __LINE__, errno);

			transFuncs[rsslServerSocketChannel->connType].shutdownSrvrError(rsslSrvrImpl);
			
			return RSSL_RET_FAILURE;
		}
	}

	/* RsslServer object does not directly keep hold of the shared buffer pool, so remove one reference count.
	   Ripc object should have its own reference to this. */
	rtrBufferPoolDropRef(serverPool);

	return RSSL_RET_SUCCESS;
}

/* rssl Socket Connect */
RsslRet rsslSocketConnect(rsslChannelImpl* rsslChnlImpl, RsslConnectOptions *opts, RsslError *error)
{
	RsslSocketChannel *rsslSocketChannel = NULL;
	RsslInt32 tempLen = 0;
	RsslSocket	sock_fd;
	RsslUInt32	portnum;
	RIPC_SESS_VERS	*version;
	RsslInt32	csFlags = RIPC_INT_CS_FLAG_NONE;
	void*	userSpecPtr = 0;
	RsslInt32	initcomplete = 0;

	if( !(opts->hostName) && !(opts->connectionInfo.unified.address) )
	{
		_rsslSetError(error, NULL, RSSL_RET_INVALID_ARGUMENT, 0);
		snprintf(error->text, MAX_RSSL_ERROR_TEXT, "<%s:%d> Error: 0013 rsslSocketConnect() No hostName or unified.address provided.\n", __FILE__, __LINE__);
		return RSSL_RET_FAILURE;
	}

	if( !(opts->serviceName) && !(opts->connectionInfo.unified.serviceName) )
	{
		_rsslSetError(error, NULL, RSSL_RET_INVALID_ARGUMENT, 0);
		snprintf(error->text, MAX_RSSL_ERROR_TEXT, "<%s:%d> Error: 0013 rsslSocketConnect() No serviceName or unified.serviceName provided.\n", __FILE__, __LINE__);
		return RSSL_RET_FAILURE;
	}

	if (opts->numInputBuffers < 2)
	{
		_rsslSetError(error, NULL, RSSL_RET_FAILURE, errno);
		snprintf(error->text, MAX_RSSL_ERROR_TEXT,
				"<%s:%d> ipcBind() Error: 1004 invalid number of input buffers <%d>, must be at least <%d>.\n",
				__FILE__, __LINE__, opts->numInputBuffers, 2);

		return RSSL_RET_FAILURE;
	}

	/* Create transport info. */
	rsslSocketChannel = newRsslSocketChannel();

	if (rsslSocketChannel == 0)
	{
		_rsslSetError(error, NULL, RSSL_RET_FAILURE, 0);
		snprintf(error->text, MAX_RSSL_ERROR_TEXT, "<%s:%d> Error: 0005 rsslSocketConnect() failed to allocate the socket channel structure.\n", __FILE__, __LINE__);
		return RSSL_RET_FAILURE;
	}

	if (opts->hostName)
	{
		tempLen = (RsslInt32)strlen(opts->hostName) + 1;
		rsslSocketChannel->hostName = (char*) strcpy((char*)_rsslMalloc(tempLen), opts->hostName);
		if (rsslSocketChannel->hostName == 0)
		{
			_rsslSetError(error, NULL, RSSL_RET_FAILURE, errno);
			snprintf(error->text, MAX_RSSL_ERROR_TEXT,
					"<%s:%d> Error: 1001 Failed to allocate or copy hostName. System errno: (%d)\n",
					__FILE__, __LINE__, errno);

			ripcRelSocketChannel(rsslSocketChannel);
			return RSSL_RET_FAILURE;
		}
	}
	else if (opts->connectionInfo.unified.address)
	{
		tempLen = (RsslInt32)(strlen(opts->connectionInfo.unified.address) + 1);
		rsslSocketChannel->hostName = (char*) strcpy((char*)_rsslMalloc(tempLen), opts->connectionInfo.unified.address);
		if (rsslSocketChannel->hostName == 0)
		{
			_rsslSetError(error, NULL, RSSL_RET_FAILURE, errno);
			snprintf(error->text, MAX_RSSL_ERROR_TEXT,
					"<%s:%d> Error: 1001 Failed to allocate or copy unified hostName. System errno: (%d)\n",
					__FILE__, __LINE__, errno);

			ripcRelSocketChannel(rsslSocketChannel);
			return RSSL_RET_FAILURE;
		}
	}

	if (opts->serviceName)
	{
		tempLen = (RsslInt32)(strlen(opts->serviceName) + 1);
		rsslSocketChannel->serverName = (char*) strcpy((char*)_rsslMalloc(tempLen), opts->serviceName);
		if (rsslSocketChannel->serverName == 0)
		{
			_rsslSetError(error, NULL, RSSL_RET_FAILURE, errno);
			snprintf(error->text, MAX_RSSL_ERROR_TEXT,
					"<%s:%d> Error: 1001 Failed to allocate or copy serviceName. System errno: (%d)\n",
					__FILE__, __LINE__, errno);

			ripcRelSocketChannel(rsslSocketChannel);
			return RSSL_RET_FAILURE;
		}
	}
	else if (opts->connectionInfo.unified.serviceName)
	{
		tempLen = (RsslInt32)(strlen(opts->connectionInfo.unified.serviceName) + 1);
		rsslSocketChannel->serverName = (char*) strcpy((char*)_rsslMalloc(tempLen), opts->connectionInfo.unified.serviceName);
		if (rsslSocketChannel->serverName == 0)
		{
			_rsslSetError(error, NULL, RSSL_RET_FAILURE, errno);
			snprintf(error->text, MAX_RSSL_ERROR_TEXT,
					"<%s:%d> Error: 1001 Failed to allocate or copy unified.serviceName. System errno: (%d)\n",
					__FILE__, __LINE__, errno);

			ripcRelSocketChannel(rsslSocketChannel);
			return RSSL_RET_FAILURE;
		}
	}

	if ((opts->connectionInfo.unified.interfaceName && opts->connectionInfo.unified.interfaceName[0] != '\0'))
	{
		tempLen = (RsslInt32)(strlen(opts->connectionInfo.unified.interfaceName) + 1);
		rsslSocketChannel->interfaceName = (char*) strcpy((char*)_rsslMalloc(tempLen), opts->connectionInfo.unified.interfaceName);
		if (rsslSocketChannel->interfaceName == 0)
		{
			_rsslSetError(error, NULL, RSSL_RET_FAILURE, errno);
			snprintf(error->text, MAX_RSSL_ERROR_TEXT,
					"<%s:%d> Error: 1001 Failed to allocate or copy interfaceName. System errno: (%d)\n",
					__FILE__, __LINE__, errno);

			ripcRelSocketChannel(rsslSocketChannel);
			return RSSL_RET_FAILURE;
		}
	}

	if ((opts->objectName) && (opts->objectName[0] != '\0'))
	{
		tempLen = (RsslInt32)(strlen(opts->objectName) + 1);
		rsslSocketChannel->objectName = (char*) strcpy((char*)_rsslMalloc(tempLen), opts->objectName);
		if (rsslSocketChannel->objectName == 0)
		{
			_rsslSetError(error, NULL, RSSL_RET_FAILURE, errno);
			snprintf(error->text, MAX_RSSL_ERROR_TEXT,
					"<%s:%d> Error: 1001 Failed to allocate or copy objectName. System errno: (%d)\n",
					__FILE__, __LINE__, errno);

			ripcRelSocketChannel(rsslSocketChannel);
			return RSSL_RET_FAILURE;
		}
	}
	rsslSocketChannel->blocking = opts->blocking;
	rsslSocketChannel->compression = opts->compressionType;

	/* check ping timeout for valid value */
	if ((opts->pingTimeout > 0) && (opts->pingTimeout <= IPC_MAXIMUM_PINGTIMEOUT))
		rsslSocketChannel->pingTimeout = opts->pingTimeout;
	else if (opts->pingTimeout <= 0)
		rsslSocketChannel->pingTimeout = IPC_MINIMUM_PINGTIMEOUT;
	else if (opts->pingTimeout > IPC_MAXIMUM_PINGTIMEOUT)
		rsslSocketChannel->pingTimeout = IPC_MAXIMUM_PINGTIMEOUT;

	rsslSocketChannel->majorVersion = opts->majorVersion;
	rsslSocketChannel->minorVersion = opts->minorVersion;
	rsslSocketChannel->protocolType = opts->protocolType;

	/* pass through send/recv buf sizes */
	rsslSocketChannel->sendBufSize = opts->sysSendBufSize;
	rsslSocketChannel->recvBufSize = opts->sysRecvBufSize;

	/*store user defined component version info from connect options, if it's present*/
	if (opts->componentVersion != NULL)
	{
		rsslChnlImpl->connOptsCompVer.componentVersion.length = (RsslUInt32)strlen(opts->componentVersion);
		rsslChnlImpl->connOptsCompVer.componentVersion.data = _rsslMalloc(rsslChnlImpl->connOptsCompVer.componentVersion.length);
		MemCopyByInt(rsslChnlImpl->connOptsCompVer.componentVersion.data, opts->componentVersion, rsslChnlImpl->connOptsCompVer.componentVersion.length);
		rsslChnlImpl->ownConnOptCompVer = RSSL_TRUE;
	}

	/* added for infra group */
	if (opts->guaranteedOutputBuffers < 5)
		rsslSocketChannel->numGuarOutputBufs = 5;
	else
		rsslSocketChannel->numGuarOutputBufs = opts->guaranteedOutputBuffers;

	rsslSocketChannel->numMaxOutputBufs = opts->guaranteedOutputBuffers;

	/* if either of these are true, enable.  Otherwise set as false */
	if (opts->tcpOpts.tcp_nodelay)
		rsslSocketChannel->tcp_nodelay = opts->tcpOpts.tcp_nodelay;
	else if (opts->tcp_nodelay)
		rsslSocketChannel->tcp_nodelay = opts->tcp_nodelay;
	else
		rsslSocketChannel->tcp_nodelay = 0;

	rsslSocketChannel->numInputBufs = opts->numInputBuffers;

	rsslSocketChannel->encryptionProtocolFlags = opts->encryptionOpts.encryptionProtocolFlags;
	rsslSocketChannel->sslEncryptedProtocolType = opts->encryptionOpts.encryptedProtocol;

	//additional initializing
	rsslSocketChannel->clientHostname = 0;
	rsslSocketChannel->clientIP = 0;
	rsslSocketChannel->connType = RSSL_CONN_TYPE_INIT;

	rsslSocketChannel->port = 0;

	switch(opts->connectionType)
	{
		case RSSL_CONN_TYPE_EXT_LINE_SOCKET:
			/* Set EL option here */
			rsslSocketChannel->numConnections = (RsslInt32)opts->extLineOptions.numConnections;
		case RSSL_CONN_TYPE_SOCKET:
		case RSSL_CONN_TYPE_ENCRYPTED:
		case RSSL_CONN_TYPE_HTTP:
		case RSSL_CONN_TYPE_WEBSOCKET:
			rsslSocketChannel->connType = (RsslUInt32)opts->connectionType;
			break;
		default:
			break;
	}

	/* still need bufPool */
	rsslSocketChannel->bufPool = NULL;

	//set rsslSocketChannel->mutex when global lock and per-channel locks enabled
	if (multiThread == RSSL_LOCK_GLOBAL_AND_CHANNEL)
		rsslSocketChannel->mutex = &(rsslChnlImpl->chanMutex);

	rsslChnlImpl->transportInfo = rsslSocketChannel;


	switch(rsslSocketChannel->ripcVersion)
	{
		case RIPC_VERSION_14:
			version = &ripc14Ver;
			break;
		case RIPC_VERSION_13:
			version = &ripc13Ver;
			break;
		case RIPC_VERSION_12:
			version = &ripc12Ver;
			break;
		case RIPC_VERSION_11:
			version = &ripc11Ver;
			break;
		case RIPC_VERSION_10:
			version = &ripc10Ver;
			break;
		default:
			  snprintf(error->text, MAX_RSSL_ERROR_TEXT,
					  "<%s:%d> Connecting with unknown RIPC version %d.",
					  __FILE__,__LINE__, rsslSocketChannel->ripcVersion);
			  return RSSL_RET_FAILURE;
	}

	if (ipcValidServerName(rsslSocketChannel->serverName,MAX_SERV_NAME) < 0)
	{
		_rsslSetError(error, NULL, RSSL_RET_FAILURE, 0);
		snprintf(error->text, MAX_RSSL_ERROR_TEXT,
				"<%s:%d> Error: 1004 Invalid serviceName or port number specified <%s>\n",
				__FILE__, __LINE__, rsslSocketChannel->serverName);

		return RSSL_RET_FAILURE;
	}
	if (rsslSocketChannel->numGuarOutputBufs < 1)
	{
		_rsslSetError(error, NULL, RSSL_RET_FAILURE, 0);
		snprintf(error->text, MAX_RSSL_ERROR_TEXT,
				"<%s:%d> Error: 1004 invalid number of guaranteed output buffers <%d>, must be at least <%d>.\n",
				__FILE__, __LINE__, rsslSocketChannel->numGuarOutputBufs, 1);

		return RSSL_RET_FAILURE;
	}

	if (rsslSocketChannel->blocking)
		csFlags |= RIPC_INT_CS_FLAG_BLOCKING;

	if (rsslSocketChannel->tcp_nodelay)
		csFlags |= RIPC_INT_CS_FLAG_TCP_NODELAY;

	if (rsslSocketChannel->connType == RSSL_CONN_TYPE_HTTP)
		csFlags |= RIPC_INT_CS_FLAG_TUNNEL_NO_ENCRYPTION;

	switch (rsslSocketChannel->connType)
	{
	case RSSL_CONN_TYPE_SOCKET:
		rsslSocketChannel->transportFuncs = &(transFuncs[RSSL_CONN_TYPE_SOCKET]);
		rsslSocketChannel->protocolFuncs = &(protHdrFuncs[RSSL_CONN_TYPE_SOCKET]);
		break;
	case RSSL_CONN_TYPE_EXT_LINE_SOCKET:
		if (rsslLoadInitTransport(&(transFuncs[RSSL_CONN_TYPE_EXT_LINE_SOCKET]),
											0,
											transOpts.initConfig) < 0)
		{
			_rsslSetError(error, NULL, RSSL_RET_FAILURE, 0);
			snprintf(error->text, MAX_RSSL_ERROR_TEXT,
					"<%s:%d> Error: 1008 Unable to set Extended Line Socket functions.",
					__FILE__, __LINE__);

			return RSSL_RET_FAILURE;
		}

		userSpecPtr = (void*)&(rsslSocketChannel->numConnections);
		rsslSocketChannel->transportFuncs = &(transFuncs[RSSL_CONN_TYPE_EXT_LINE_SOCKET]);
		rsslSocketChannel->protocolFuncs = &(protHdrFuncs[RSSL_CONN_TYPE_SOCKET]);
		break;
	case RSSL_CONN_TYPE_HTTP:
#ifdef WIN32
		if (initRipcWinInet() <= 0)
		{
			_rsslSetError(error, NULL, RSSL_RET_FAILURE, errno);
			snprintf(error->text, MAX_RSSL_ERROR_TEXT,
					"<%s:%d> Error: 1008 Unable to set WinInet functions.\n",
					__FILE__, __LINE__);

			return RSSL_RET_FAILURE;
		}
		rsslSocketChannel->transportFuncs = &(transFuncs[RSSL_CONN_TYPE_HTTP]);
		rsslSocketChannel->protocolFuncs = &(protHdrFuncs[RSSL_CONN_TYPE_SOCKET]);
		rsslSocketChannel->usingWinInet = RSSL_CONN_TYPE_HTTP;
#else
		_rsslSetError(error, NULL, RSSL_RET_FAILURE, 0);
		snprintf(error->text, MAX_RSSL_ERROR_TEXT,
				"<%s:%d> Error: 1004 HTTP Tunneling not supported for this platform.\n",
				__FILE__, __LINE__);

		return RSSL_RET_FAILURE;
#endif
		break;
	case RSSL_CONN_TYPE_ENCRYPTED:
		/* We assume that if proxyHostName is empty it implies a proxy running on localhost */
		/* if we arent on windows and tunneling is on */

		/* The default protocol functions for the encrypted connection */
		rsslSocketChannel->protocolFuncs = &(protHdrFuncs[RSSL_CONN_TYPE_SOCKET]);
#ifdef _WIN32
		if (opts->encryptionOpts.encryptedProtocol == RSSL_CONN_TYPE_HTTP)
		{
			if (initRipcWinInet() <= 0)
			{
				_rsslSetError(error, NULL, RSSL_RET_FAILURE, errno);
				snprintf(error->text, MAX_RSSL_ERROR_TEXT,
					"<%s:%d> Error: 1008 Unable to set WinInet functions.\n.",
					__FILE__, __LINE__);

				return RSSL_RET_FAILURE;
			}
			rsslSocketChannel->transportFuncs = &(transFuncs[RSSL_CONN_TYPE_HTTP]);
			rsslSocketChannel->usingWinInet = RSSL_CONN_TYPE_HTTP;
			break;
		}
#endif
		if (opts->encryptionOpts.encryptedProtocol == RSSL_CONN_TYPE_WEBSOCKET)
		{
			if (rwsInitialize() == RSSL_RET_FAILURE )
			{
				_rsslSetError(error, NULL, RSSL_RET_FAILURE, errno);
				snprintf(error->text, MAX_RSSL_ERROR_TEXT,
						"<%s:%d> Error: 1XXX Unable to set WebSocket transport functions.\n",
						__FILE__, __LINE__);

				return RSSL_RET_FAILURE;
			}

			rsslSocketChannel->transportFuncs = &(transFuncs[RSSL_CONN_TYPE_SOCKET]);
			rsslSocketChannel->protocolFuncs = &(protHdrFuncs[RSSL_CONN_TYPE_SOCKET]);

			if (rwsInitSessionOptions(rsslSocketChannel, &(opts->wsOpts), error) == RSSL_RET_FAILURE)	
			{
				_rsslSetError(error, NULL, RSSL_RET_FAILURE, errno);
				snprintf(error->text+strlen(error->text), MAX_RSSL_ERROR_TEXT,
					"<%s:%d> Error: 1001 Failed to allocate session or copy WS Opts. System errno: (%d)\n",
					__FILE__, __LINE__, errno);

				ripcRelSocketChannel(rsslSocketChannel);
				return RSSL_RET_FAILURE;
			}
		}

		if ( (opts->encryptionOpts.encryptedProtocol != RSSL_CONN_TYPE_SOCKET) && (opts->encryptionOpts.encryptedProtocol != RSSL_CONN_TYPE_WEBSOCKET) )
		{
			/* For Linux the only supported encrypted protocol is RSSL_CONN_TYPE_SOCKET. */
			_rsslSetError(error, NULL, RSSL_RET_FAILURE, 0);
			snprintf(error->text, MAX_RSSL_ERROR_TEXT,
				"<%s:%d> Error: 1004 Invalid encrypted protocol type.  Only supported protocols are RSSL_CONN_TYPE_HTTP(Windows only), RSSL_CONN_TYPE_SOCKET and RSSL_CONN_TYPE_WEBSOCKET.\n.", 
				__FILE__, __LINE__);

			return RSSL_RET_FAILURE;
		}
		/* We are not using WinInet, so clear the flag on the socketChannel */
		rsslSocketChannel->usingWinInet = 0;
		if(openSSLInit == 0)
		{
			_rsslSetError(error, NULL, RSSL_RET_FAILURE, errno);
			snprintf(error->text, MAX_RSSL_ERROR_TEXT,
					"<%s:%d> Error: 0012 openSSL Libraries have not been loaded.\n",
					__FILE__, __LINE__);

			return RSSL_RET_FAILURE;
		}

		if(opts->encryptionOpts.openSSLCAStore != NULL && (opts->encryptionOpts.openSSLCAStore[0] != '\0'))
		{
			tempLen = (RsslInt32)strlen(opts->encryptionOpts.openSSLCAStore) + 1;
			rsslSocketChannel->sslCAStore = (char*)strcpy((char*)_rsslMalloc(tempLen), opts->encryptionOpts.openSSLCAStore);
			if (rsslSocketChannel->sslCAStore == 0)
			{
				_rsslSetError(error, NULL, RSSL_RET_FAILURE, errno);
				snprintf(error->text, MAX_RSSL_ERROR_TEXT,
					"<%s:%d> Error: 1001 Failed to allocate or copy hostName. System errno: (%d)\n",
					__FILE__, __LINE__, errno);

				ripcRelSocketChannel(rsslSocketChannel);
				return RSSL_RET_FAILURE;
			}
		}

		if (rsslSocketChannel->encryptionProtocolFlags == 0)
		{
			_rsslSetError(error, NULL, RSSL_RET_FAILURE, 0);
			snprintf(error->text, MAX_RSSL_ERROR_TEXT,
				"<%s:%d> Error: 0012 No valid TLS encryption protocol set.\n",
				__FILE__, __LINE__);

			return RSSL_RET_FAILURE;
		}

		rsslSocketChannel->sslProtocolBitmap = rsslSocketChannel->encryptionProtocolFlags | RIPC_PROTO_SSL_TLS;

		rsslSocketChannel->sslProtocolBitmap &= ripcGetSupportedProtocolFlags();
		if(rsslSocketChannel->sslProtocolBitmap == 0)
		{
			_rsslSetError(error, NULL, RSSL_RET_FAILURE, 0);
			snprintf(error->text, MAX_RSSL_ERROR_TEXT,
					"<%s:%d> Error: 0012 OpenSSL does not support any of the configured TLS versions.\n",
					__FILE__, __LINE__);

			return RSSL_RET_FAILURE;
		}
		
		if(getSSLProtocolTransFuncs(rsslSocketChannel, rsslSocketChannel->sslProtocolBitmap) != 0)
		{
			_rsslSetError(error, NULL, RSSL_RET_FAILURE, 0);
			snprintf(error->text, MAX_RSSL_ERROR_TEXT,
					"<%s:%d> Error: 0012 Out of available SSL/TLS connection protocol options.\n",
					__FILE__, __LINE__);

			return RSSL_RET_FAILURE;
		}

		/* Set the userSpecPtr to the current rsslSocketChannel */
		userSpecPtr = (void*)rsslSocketChannel;
		break;

	case RSSL_CONN_TYPE_WEBSOCKET:

		/* Create a WebSocket session if connType is RSSL_CONN_TYPE_WEBSOCKET or
		 * if RSSL_CONN_TYPE_ENCRYPTION and its protocol is WebSocket is handeled witin the
		 * _ENCRYPTION block */
		if (rwsInitialize() == RSSL_RET_FAILURE )
		{
			_rsslSetError(error, NULL, RSSL_RET_FAILURE, errno);
			snprintf(error->text, MAX_RSSL_ERROR_TEXT,
					"<%s:%d> Error: 1XXX Unable to set WebSocket transport functions.\n",
					__FILE__, __LINE__);

			return RSSL_RET_FAILURE;
		}

		rsslSocketChannel->transportFuncs = &(transFuncs[RSSL_CONN_TYPE_SOCKET]);
		rsslSocketChannel->protocolFuncs = &(protHdrFuncs[RSSL_CONN_TYPE_SOCKET]);

		if (rwsInitSessionOptions(rsslSocketChannel, &(opts->wsOpts), error) == RSSL_RET_FAILURE)	
		{
			_rsslSetError(error, NULL, RSSL_RET_FAILURE, errno);
			snprintf(error->text+strlen(error->text), MAX_RSSL_ERROR_TEXT,
				"<%s:%d> Error: 1001 Failed to allocate session or copy WS Opts. System errno: (%d)\n",
				__FILE__, __LINE__, errno);

			ripcRelSocketChannel(rsslSocketChannel);
			return RSSL_RET_FAILURE;
		}

		break;

	default:
		_rsslSetError(error, NULL, RSSL_RET_FAILURE, 0);
		snprintf(error->text, MAX_RSSL_ERROR_TEXT,
				"<%s:%d> Error: 1004 Unsupported connection type.\n",
				__FILE__, __LINE__);

		return RSSL_RET_FAILURE;
	}

	/* set up the compression bitfield whicseh tells the server which compression types we would like to use */
	/* for now we can only set one compression type */
	/* not sure why we are indexing into compressionBitmap since its an array of only 1 byte */
	{
		RsslInt16 idx = ripccompressions[rsslSocketChannel->compression][RSSL_COMP_BYTEINDEX];
		if (idx < RSSL_COMP_BITMAP_SIZE)
			rsslSocketChannel->compressionBitmap[idx] |= ripccompressions[rsslSocketChannel->compression][RSSL_COMP_BYTEBIT];
	}

	/* Set Proxy options, if present */
	if (opts->proxyOpts.proxyHostName && (opts->proxyOpts.proxyHostName[0] != '\0'))
	{
		tempLen = (RsslInt32)(strlen(opts->proxyOpts.proxyHostName) + 1);
		rsslSocketChannel->proxyHostName = (char*)strcpy((char*)_rsslMalloc(tempLen), opts->proxyOpts.proxyHostName);
		if (rsslSocketChannel->proxyHostName == 0)
		{
			_rsslSetError(error, NULL, RSSL_RET_FAILURE, errno);
			snprintf(error->text, MAX_RSSL_ERROR_TEXT,
				"<%s:%d> Error: 1001 Failed to allocate or copy proxyHostName. System errno: (%d)\n",
				__FILE__, __LINE__, errno);

			ripcRelSocketChannel(rsslSocketChannel);
			return RSSL_RET_FAILURE;
		}

		if (opts->proxyOpts.proxyPort && (opts->proxyOpts.proxyPort[0] != '\0'))
		{
			tempLen = (RsslInt32)(strlen(opts->proxyOpts.proxyPort) + 1);
			rsslSocketChannel->proxyPort = (char*)strcpy((char*)_rsslMalloc(tempLen), opts->proxyOpts.proxyPort);
			if (rsslSocketChannel->proxyPort == 0)
			{
				_rsslSetError(error, NULL, RSSL_RET_FAILURE, errno);
				snprintf(error->text, MAX_RSSL_ERROR_TEXT,
					"<%s:%d> Error: 1001 Failed to allocate or copy proxyPort. System errno: (%d)\n",
					__FILE__, __LINE__, errno);

				ripcRelSocketChannel(rsslSocketChannel);
				return RSSL_RET_FAILURE;
			}
		}

		if (opts->proxyOpts.proxyUserName && (opts->proxyOpts.proxyUserName[0] != '\0'))
		{
			tempLen = (RsslInt32)(strlen(opts->proxyOpts.proxyUserName) + 1);
			rsslSocketChannel->curlOptProxyUser = (char*)strcpy((char*)_rsslMalloc(tempLen), opts->proxyOpts.proxyUserName);
			if (rsslSocketChannel->curlOptProxyUser == 0)
			{
				_rsslSetError(error, NULL, RSSL_RET_FAILURE, errno);
				snprintf(error->text, MAX_RSSL_ERROR_TEXT,
					"<%s:%d> Error: 1001 Failed to allocate or copy curlOptProxyUser. System errno: (%d)\n",
					__FILE__, __LINE__, errno);

				ripcRelSocketChannel(rsslSocketChannel);
				return RSSL_RET_FAILURE;
			}
		}

		if (opts->proxyOpts.proxyPasswd && (opts->proxyOpts.proxyPasswd[0] != '\0'))
		{
			tempLen = (RsslInt32)(strlen(opts->proxyOpts.proxyPasswd) + 1);
			rsslSocketChannel->curlOptProxyPasswd = (char*)strcpy((char*)_rsslMalloc(tempLen), opts->proxyOpts.proxyPasswd);
			if (rsslSocketChannel->curlOptProxyPasswd == 0)
			{
				_rsslSetError(error, NULL, RSSL_RET_FAILURE, errno);
				snprintf(error->text, MAX_RSSL_ERROR_TEXT,
					"<%s:%d> Error: 1001 Failed to allocate or copy curlOptProxyPasswd. System errno: (%d)\n",
					__FILE__, __LINE__, errno);

				ripcRelSocketChannel(rsslSocketChannel);
				return RSSL_RET_FAILURE;
			}
		}

		if (opts->proxyOpts.proxyDomain && (opts->proxyOpts.proxyDomain[0] != '\0'))
		{
			tempLen = (RsslInt32)(strlen(opts->proxyOpts.proxyDomain) + 1);
			rsslSocketChannel->curlOptProxyDomain = (char*)strcpy((char*)_rsslMalloc(tempLen), opts->proxyOpts.proxyDomain);
			if (rsslSocketChannel->curlOptProxyDomain == 0)
			{
				_rsslSetError(error, NULL, RSSL_RET_FAILURE, errno);
				snprintf(error->text, MAX_RSSL_ERROR_TEXT,
					"<%s:%d> Error: 1001 Failed to allocate or copy curlOptProxyDomain. System errno: (%d)\n",
					__FILE__, __LINE__, errno);

				ripcRelSocketChannel(rsslSocketChannel);
				return RSSL_RET_FAILURE;
			}
		}

		/* If we are not using winInet, check if libcurl has been loaded and initialized.  If it has, load it */
		if (rsslSocketChannel->usingWinInet == 0 && rsslCurlIsInitialized() == RSSL_FALSE)
		{
			if (rsslInitCurlApi(transOpts.jitOpts.libcurlName, error) == NULL)
			{
				/* Error text has been populated in the init call */
				ripcRelSocketChannel(rsslSocketChannel);
				return RSSL_RET_FAILURE;
			}

			/* We called Init, so we must call Uninit when we uninitialize rssl */
			libcurlInit = 1;
		}
	}
	
	/* normal connection */
	if ((sock_fd = (*(rsslSocketChannel->transportFuncs->connectSocket))(
		&portnum, rsslChnlImpl->transportInfo, csFlags, &userSpecPtr, error)) == RIPC_INVALID_SOCKET)
	{
		ripcRelSocketChannel(rsslSocketChannel);
		return RSSL_RET_FAILURE;
	}

	rsslSocketChannel->stream = sock_fd;
	
	rsslSocketChannel->state = RSSL_CH_STATE_INITIALIZING;

	/* This is where we set the version to connect as */
	/* Connection something in to determine which version to use */
	rsslSocketChannel->version = version;

	rsslSocketChannel->intState = RIPC_INT_ST_CONNECTING;
	rsslSocketChannel->maxMsgSize = 0;
	rsslSocketChannel->maxUserMsgSize = 0;
	/* Store numInputBufs in readSize until we are connected */
	rsslSocketChannel->readSize = opts->numInputBuffers;
	rsslSocketChannel->guarBufPool = rtr_dfltcAllocPool(rsslSocketChannel->numGuarOutputBufs, rsslSocketChannel->numGuarOutputBufs, 10,
		rsslSocketChannel->bufPool, (rsslSocketChannel->numMaxOutputBufs - rsslSocketChannel->numGuarOutputBufs), 0);
	if (rsslSocketChannel->guarBufPool == 0)
	{
		_rsslSetError(error, NULL, RSSL_RET_FAILURE, errno);
		snprintf(error->text, MAX_RSSL_ERROR_TEXT,
				"<%s:%d> Error: 1001 Could not allocate part of output pool.\n",
				__FILE__, __LINE__);

		sock_close(sock_fd);
		ripcRelSocketChannel(rsslSocketChannel);
		return RSSL_RET_FAILURE;
	}

	rsslSocketChannel->clientSession = 1;
	rsslSocketChannel->blocking = (rsslSocketChannel->blocking ? 1 : 0);
	rsslSocketChannel->tcp_nodelay = (rsslSocketChannel->tcp_nodelay ? 1 : 0);
	rsslSocketChannel->pingTimeout = (RsslUInt8)rsslSocketChannel->pingTimeout;

	rsslSocketChannel->protocolType = (RsslUInt8)rsslSocketChannel->protocolType;
	rsslSocketChannel->minorVersion = (RsslUInt8)rsslSocketChannel->minorVersion;
	rsslSocketChannel->majorVersion = (RsslUInt8)rsslSocketChannel->majorVersion;

	rsslSocketChannel->inDecompress = opts->compressionType;

	// the port number that was used to connect to the server
	// portnum is not used here because returns either the proxy port(if specified) or the server port
	rsslSocketChannel->port = htons(ipcGetServByName(rsslSocketChannel->serverName));

	if (rsslSocketChannel->proxyPort && (rsslSocketChannel->proxyPort[0] != '\0') && (rsslSocketChannel->usingWinInet == 0))
	{
		/* we need to send and wait for the proxy ack if we are using a proxy connection */
		rsslSocketChannel->intState = RIPC_INT_ST_PROXY_CONNECTING;
	}
	else
	{
		/* if we are using a proxy, this stuff happens after we get the HTTP ack */
		rsslSocketChannel->transportInfo = (*(rsslSocketChannel->transportFuncs->newClientConnection))(
			rsslSocketChannel->stream, &initcomplete, userSpecPtr, error);

		if (rsslSocketChannel->transportInfo == 0)
		{
			sock_close(sock_fd);
			ripcRelSocketChannel(rsslSocketChannel);

			return RSSL_RET_FAILURE;
		}

		if ((!initcomplete) &&
			((rsslSocketChannel->connType == RSSL_CONN_TYPE_ENCRYPTED) ||
			 (rsslSocketChannel->connType == RSSL_CONN_TYPE_HTTP) ||
			 (rsslSocketChannel->connType == RSSL_CONN_TYPE_EXT_LINE_SOCKET)))
			rsslSocketChannel->intState = RIPC_INT_ST_CLIENT_TRANSPORT_INIT;

		if (rsslSocketChannel->connType == RSSL_CONN_TYPE_WEBSOCKET)
			rsslSocketChannel->intState = RIPC_INT_ST_WS_SEND_OPENING_HANDSHAKE;
	}

	_DEBUG_TRACE_CONN("connType %d intState %d\n", rsslSocketChannel->connType, rsslSocketChannel->intState)
	if (rsslSocketChannel->blocking)
	{
		ripcSessInProg inPr;
		ripcSessInit ret = RIPC_CONN_IN_PROGRESS;

		while (ret == RIPC_CONN_IN_PROGRESS)
			ret = ipcSessionInit(rsslSocketChannel,&inPr,error);

		if (ret != RIPC_CONN_ACTIVE)
		{
			ipcSessDropRef(rsslSocketChannel,error);
			return RSSL_RET_FAILURE;
		}
	}

	/* map RsslSocketChannel to RsslChannel */
	_rsslSocketToChannel(rsslChnlImpl, rsslSocketChannel);

	rsslChnlImpl->Channel.userSpecPtr = opts->userSpecPtr;

	if (opts->blocking)
	{
		IPC_MUTEX_LOCK(rsslSocketChannel);
		if (rsslSocketChannel->workState & RIPC_INT_SHTDOWN_PEND)
		{
			_rsslSetError(error, NULL, RSSL_RET_FAILURE, 0);
			snprintf(error->text, MAX_RSSL_ERROR_TEXT,
				"<%s:%d> Error: 1003 rsslSocketConnect() failed due to channel shutting down.\n",
				__FILE__, __LINE__);

			ipcShutdownSockectChannel(rsslSocketChannel, error);

		IPC_MUTEX_UNLOCK(rsslSocketChannel);
			return RSSL_RET_FAILURE;
		}
		else if (rsslSocketChannel->intState != RIPC_INT_ST_ACTIVE)
		{
			_rsslSetError(error, NULL, RSSL_RET_FAILURE, 0);
			snprintf(error->text, MAX_RSSL_ERROR_TEXT,
					"<%s:%d> Error: 1003 rsslSocketConnect() failed because the channel is not active.\n",
					__FILE__, __LINE__);

			ipcShutdownSockectChannel(rsslSocketChannel, error);

		IPC_MUTEX_UNLOCK(rsslSocketChannel);
			return RSSL_RET_FAILURE;
		}

		rsslChnlImpl->maxMsgSize = rsslSocketChannel->maxUserMsgSize;
		rsslChnlImpl->maxGuarMsgs = rsslSocketChannel->guarBufPool->bufpool.maxBufs; //guarOutputMsgs

		if (rsslSocketChannel->version->connVersion > CONN_VERSION_12)
			rsslChnlImpl->fragIdMax = 65535;
		else
			rsslChnlImpl->fragIdMax = 255;

		/* if its blocking we can get ping info here */
		rsslChnlImpl->rsslFlags = rsslSocketChannel->rsslFlags;
		rsslChnlImpl->Channel.pingTimeout = rsslSocketChannel->pingTimeout;

		rsslChnlImpl->Channel.protocolType = rsslSocketChannel->protocolType;
		rsslChnlImpl->Channel.majorVersion = rsslSocketChannel->majorVersion;
		rsslChnlImpl->Channel.minorVersion = rsslSocketChannel->minorVersion;

		rsslChnlImpl->Channel.connectionType = rsslSocketChannel->connType;

		rsslChnlImpl->Channel.clientHostname = rsslSocketChannel->clientHostname;
		rsslChnlImpl->Channel.clientIP = rsslSocketChannel->clientIP;

		/* set shared key */
		rsslChnlImpl->shared_key = rsslSocketChannel->shared_key;
		IPC_MUTEX_UNLOCK(rsslSocketChannel);
	}

   	if (multiThread)
	{
	  (void) RSSL_MUTEX_LOCK(&ripcMutex);
		_DEBUG_MUTEX_TRACE("RSSL_MUTEX_LOCK", NULL, &ripcMutex)
	}

	rsslQueueAddLinkToBack(&activeSocketChannelList, &(rsslSocketChannel->link1));

	if (multiThread)
	{
	  (void) RSSL_MUTEX_UNLOCK(&ripcMutex);
		_DEBUG_MUTEX_TRACE("RSSL_MUTEX_UNLOCK", NULL, &ripcMutex)
	}

	return RSSL_RET_SUCCESS;
}

static RsslSocketChannel *ipcClientChannel(rsslServerImpl* serverImpl, RsslError *error)
{
	RsslSocketChannel	*rsslSocketChannel;
	RsslSocket			fdtemp;
	RsslInt32			complete;
	void*				userSpecPtr = 0;
	RsslInt32			i = 0;
	RsslServerSocketChannel *rsslServerSocketChannel = 0;
	ripcSSLServer* info = 0;

	rsslServerSocketChannel = (RsslServerSocketChannel*)serverImpl->transportInfo;

	_DEBUG_TRACE_CONN("fd "SOCKET_PRINT_TYPE" conTyp:%d\n",
					rsslServerSocketChannel->stream, rsslServerSocketChannel->connType)

	if ((rsslSocketChannel = newRsslSocketChannel()) == 0)
	{
		_rsslSetError(error, NULL, RSSL_RET_FAILURE, errno);
		snprintf(error->text, MAX_RSSL_ERROR_TEXT,
			"<%s:%d> Error: 1001 Failed to allocate memory for new RsslSocketChannel.\n",
			__FILE__, __LINE__);

		return 0;
	}

	// Do range check first.

	switch (rsslServerSocketChannel->connType)
	{
	case RSSL_CONN_TYPE_SOCKET: /* these are already set */
	case RSSL_CONN_TYPE_HTTP:
	case RSSL_CONN_TYPE_WEBSOCKET:
		rsslSocketChannel->connType = RSSL_CONN_TYPE_SOCKET;
		rsslSocketChannel->transportFuncs = &(transFuncs[rsslSocketChannel->connType]);
		rsslSocketChannel->protocolFuncs = &(protHdrFuncs[rsslSocketChannel->connType]);
		break;
	case RSSL_CONN_TYPE_EXT_LINE_SOCKET:
		
		if (rsslLoadInitTransport(&(transFuncs[RSSL_CONN_TYPE_EXT_LINE_SOCKET]),
											0,
											transOpts.initConfig) < 0)
		{
			_rsslSetError(error, NULL, RSSL_RET_FAILURE, errno);
			snprintf(error->text, MAX_RSSL_ERROR_TEXT,
				"<%s:%d> Error: 1008 Unable to set Extended Line Socket functions.",
				__FILE__, __LINE__);

			return 0;
		}
		rsslSocketChannel->protocolFuncs = &(protHdrFuncs[RSSL_CONN_TYPE_SOCKET]);
		rsslSocketChannel->connType = RSSL_CONN_TYPE_EXT_LINE_SOCKET;
		rsslSocketChannel->transportFuncs = &(transFuncs[rsslSocketChannel->connType]);
		break;
	case RSSL_CONN_TYPE_ENCRYPTED:
		info = (ripcSSLServer*)rsslServerSocketChannel->transportInfo;
		rsslSocketChannel->sslProtocolBitmap = info->chnl->encryptionProtocolFlags;
		
		/* Set the protocol functions for the encrypted connection. */
		rsslSocketChannel->protocolFuncs = &(protHdrFuncs[RSSL_CONN_TYPE_SOCKET]);
		if (getSSLProtocolTransFuncs(rsslSocketChannel, rsslSocketChannel->sslProtocolBitmap) != 0)
		{
			_rsslSetError(error, NULL, RSSL_RET_FAILURE, errno);
			snprintf(error->text, MAX_RSSL_ERROR_TEXT,
				"<%s:%d> Error: 0012 Out of available SSL/TLS connection protocol options.\n",
				__FILE__, __LINE__);

			ripcRelSocketChannel(rsslSocketChannel);
			return 0;
		}
		break;
	default:
		_rsslSetError(error, NULL, RSSL_RET_FAILURE, errno);
		snprintf(error->text, MAX_RSSL_ERROR_TEXT,
			"<%s:%d> Error: 0012 Unknown connection type (%d) specified.\n ",
			__FILE__, __LINE__, rsslSocketChannel->connType);

		ripcRelSocketChannel(rsslSocketChannel);
		return(0);
		break;
	}

	fdtemp = (*(rsslSocketChannel->transportFuncs->acceptSocket))(serverImpl, &userSpecPtr, error);

	if (fdtemp == RIPC_INVALID_SOCKET)
	{
		ripcRelSocketChannel(rsslSocketChannel);

		/* when this fails, we only should close the server if the accept failed.  if it blocked or setting options on the session
		* failed, do not shut down the server */
		if (error->rsslErrorId == RSSL_RET_FAILURE)
		{
			if (error->sysError == EINVAL)
			{
				ipcCloseActiveSrvr(rsslServerSocketChannel);
			}
		}
		return 0;
	}

	if(userSpecPtr != 0)
		rsslSocketChannel->transportInfo = userSpecPtr;
#ifdef IPC_DEBUG_SOCKET_NAME
	_ipcdGetSockName(fdtemp);
	_ipcdGetPeerName(fdtemp);
#endif

	rsslSocketChannel->server = rsslServerSocketChannel;

	rsslSocketChannel->blocking = (rsslServerSocketChannel->session_blocking ? 1 : 0);
	rsslSocketChannel->tcp_nodelay = (rsslServerSocketChannel->tcp_nodelay ? 1 : 0);
	rsslSocketChannel->maxMsgSize = rsslServerSocketChannel->maxMsgSize;
	rsslSocketChannel->maxUserMsgSize = rsslServerSocketChannel->maxUserMsgSize;
	rsslSocketChannel->srvrcomp = rsslServerSocketChannel->compressionSupported;
	rsslSocketChannel->forcecomp = rsslServerSocketChannel->forcecomp ? 1 : 0;
	rsslSocketChannel->upperCompressionThreshold = rsslServerSocketChannel->maxUserMsgSize;

	rsslSocketChannel->rsslFlags = rsslServerSocketChannel->rsslFlags;
	rsslSocketChannel->pingTimeout = rsslServerSocketChannel->pingTimeout;
	rsslSocketChannel->minPingTimeout = rsslServerSocketChannel->minPingTimeout;
	rsslSocketChannel->protocolType = rsslServerSocketChannel->protocolType;
	rsslSocketChannel->minorVersion = rsslServerSocketChannel->minorVersion;
	rsslSocketChannel->majorVersion = rsslServerSocketChannel->majorVersion;

	/* Take care of input buffer */
	rsslSocketChannel->inputBuffer = rtr_smplcAllocMsg(gblInputBufs, (rsslSocketChannel->maxMsgSize * rsslServerSocketChannel->numInputBufs));

	/*do not need this as we do it later when processing connect msgs */
	/*rsslSocketChannel->curInputBuf = rtr_smplcDupMsg(gblInputBufs,rsslSocketChannel->inputBuffer);  */

	if (rsslSocketChannel->inputBuffer == 0) /*|| (rsslSocketChannel->curInputBuf == 0)) */
	{
		_rsslSetError(error, NULL, RSSL_RET_FAILURE, errno);
		snprintf(error->text, MAX_RSSL_ERROR_TEXT,
			"<%s:%d> Error: 1001 Could not allocate memory for channel's input buffer.\n",
			__FILE__, __LINE__);

		sock_close(fdtemp);
		ripcRelSocketChannel(rsslSocketChannel);
		return(0);
	}
	/* ref for global Input Buffers */
	rsslSocketChannel->gblInputBufs = gblInputBufs;

	/* ref for compression functions */
	rsslSocketChannel->compressFuncs = &(compressFuncs[0]);

	/* this should let us read all but one buffers worth of data */
	rsslSocketChannel->readSize = (RsslInt32)(rsslSocketChannel->inputBuffer->maxLength - rsslSocketChannel->maxMsgSize);

	/* Don't allocate guarBufPool until we know the session is a real session */

	/* priority writing stuff */
	/* initialize output priority lists */
	for (i = 0; i< RIPC_MAX_PRIORITY_QUEUE; i++)
	{
		rsslInitQueue(&rsslSocketChannel->priorityQueues[i].priorityQueue);
		rsslSocketChannel->priorityQueues[i].queueLength = 0;
	}

	/* initialize flush strategy */
	/* H - high, M - medium, L - low */
	rsslSocketChannel->flushStrategy[0] = 0;
	rsslSocketChannel->flushStrategy[1] = 1;
	rsslSocketChannel->flushStrategy[2] = 0;
	rsslSocketChannel->flushStrategy[3] = 2;
	rsslSocketChannel->flushStrategy[4] = 0;
	rsslSocketChannel->flushStrategy[5] = 1;
	rsslSocketChannel->flushStrategy[6] = -1;

	/* start at beginning of flush strategy */
	rsslSocketChannel->currentOutList = 0;

	rsslSocketChannel->compressQueue = -1;
	rsslSocketChannel->nextOutBuf = -1;

	rsslSocketChannel->stream = fdtemp;
	rsslSocketChannel->state = RSSL_CH_STATE_INITIALIZING;
	rsslSocketChannel->mountNak = rsslServerSocketChannel->mountNak;

	rsslSocketChannel->stream = fdtemp;

	rsslSocketChannel->bytesOutLastMsg = 0;

	/* set to initial version to connect as */
	rsslSocketChannel->version = &ripc13Ver;

	rsslSocketChannel->clientSession = 0;

	userSpecPtr = rsslSocketChannel;

	rsslSocketChannel->transportInfo = (*(rsslSocketChannel->transportFuncs->newSrvrConnection))(
		rsslServerSocketChannel->transportInfo, fdtemp, &complete, userSpecPtr, error);

	if (rsslSocketChannel->transportInfo == 0)
	{
		/*_rsslSetError(error, NULL, RSSL_RET_FAILURE, errno);
		snprintf(error->text, MAX_RSSL_ERROR_TEXT,
			"<%s:%d> Error: 1005 Could not initialize the transport for the new session.\n",
			__FILE__, __LINE__);*/

		sock_close(fdtemp);
		ripcRelSocketChannel(rsslSocketChannel);
		return(0);
	}

	if (complete)
	{
		rsslSocketChannel->intState = RIPC_INT_ST_READ_HDR;
	}
	else
		rsslSocketChannel->intState = RIPC_INT_ST_TRANSPORT_INIT;

	/* Insert the new RsslSocketChannel into the server list */
	if (multiThread)
	{
	  (void) RSSL_MUTEX_LOCK(&ripcMutex);
		_DEBUG_MUTEX_TRACE("RSSL_MUTEX_LOCK", NULL, &ripcMutex)
	}
	rsslQueueAddLinkToBack(&activeSocketChannelList, &(rsslSocketChannel->link1));
	if (multiThread)
	{
	  (void) RSSL_MUTEX_UNLOCK(&ripcMutex);
		_DEBUG_MUTEX_TRACE("RSSL_MUTEX_UNLOCK", NULL, &ripcMutex)
	}

	return(rsslSocketChannel);
}

/* rssl socket accept */
rsslChannelImpl* rsslSocketAccept(rsslServerImpl *rsslSrvrImpl, RsslAcceptOptions *opts, RsslError *error)
{
	rsslChannelImpl		*rsslChnlImpl=0;
	RsslSocketChannel		*rsslSocketChannel = 0;
	RsslServerSocketChannel	*rsslServerSocketChannel;

	rsslServerSocketChannel = (RsslServerSocketChannel*)rsslSrvrImpl->transportInfo;

	if (IPC_NULL_PTR(rsslServerSocketChannel, "rsslSocketAccept", "rsslServerSocketChannel", error))
		return 0;

	if ((rsslChnlImpl = _rsslNewChannel()) == 0)
	{
		/* error */
		_rsslSetError(error, 0, RSSL_RET_FAILURE,  0);
		snprintf(error->text, MAX_RSSL_ERROR_TEXT, "<%s:%d> Error: 0005 rsslSocketAccept() Could not allocate memory for new channel\n", __FILE__, __LINE__);
		return 0;
	}

	rsslServerSocketChannel->mountNak = opts->nakMount ? 1 : 0;

	/* bridge through the send and receive buffer sizes */
	rsslServerSocketChannel->sendBufSize = rsslSrvrImpl->sendBufSize;
	rsslServerSocketChannel->recvBufSize = rsslSrvrImpl->recvBufSize;

	IPC_MUTEX_LOCK(rsslServerSocketChannel);

	RIPC_ASSERT(rsslServerSocketChannel->state == RSSL_CH_STATE_ACTIVE);

	while (rsslSocketChannel == 0)
	{
		rsslSocketChannel = ipcClientChannel(rsslSrvrImpl, error);
		/* Otherwise, if blocking fatal then return. */
		if (rsslServerSocketChannel->state != RSSL_CH_STATE_ACTIVE)
		{
			IPC_MUTEX_UNLOCK(rsslServerSocketChannel);
			return 0;
		}

		/* If non blocking, then return result. */
		if (rsslServerSocketChannel->server_blocking == 0)
			break;
	}



	IPC_MUTEX_UNLOCK(rsslServerSocketChannel);

	if (rsslSocketChannel == 0)
	{
		_rsslReleaseChannel(rsslChnlImpl);

		return 0;
	}
	else
	{
		//set rsslSocketChannel->mutex when global lock and per-channel locks enabled
		if (multiThread == RSSL_LOCK_GLOBAL_AND_CHANNEL)
			rsslSocketChannel->mutex = &(rsslChnlImpl->chanMutex);

		/*Add callback to the socket channel*/
		rsslSocketChannel->httpCallback = rsslServerSocketChannel->httpCallback;

		/*Add coockes ptr to the socket channel*/
		rsslSocketChannel->cookies = rsslServerSocketChannel->cookies;

		/* map RsslSocketChannel to RsslChannel struct */
		_rsslSocketToChannel(rsslChnlImpl, rsslSocketChannel);
		rsslChnlImpl->transportInfo = rsslSocketChannel;
		rsslChnlImpl->maxMsgSize = rsslSocketChannel->maxMsgSize;

		if (!opts->userSpecPtr)
		{
			rsslChnlImpl->Channel.userSpecPtr = rsslSrvrImpl->Server.userSpecPtr;
		}
		else
		{
			rsslChnlImpl->Channel.userSpecPtr = opts->userSpecPtr;
		}

		/* map ping stuff */
		rsslChnlImpl->rsslFlags = rsslSocketChannel->rsslFlags;
		rsslChnlImpl->Channel.pingTimeout = rsslSocketChannel->pingTimeout;

		rsslChnlImpl->Channel.majorVersion = rsslSocketChannel->majorVersion;
		rsslChnlImpl->Channel.minorVersion = rsslSocketChannel->minorVersion;
		rsslChnlImpl->Channel.protocolType = rsslSocketChannel->protocolType;

		rsslChnlImpl->Channel.connectionType = rsslSocketChannel->connType;
	}

	rsslChnlImpl->channelFuncs = rsslSrvrImpl->channelFuncs;

	/* perform initChannel here if server is blocking */
	if (rsslSrvrImpl->isBlocking)
	{
		RsslRet	retval;
		RsslInProgInfo inProg = RSSL_INIT_IN_PROG_INFO;

		while (rsslChnlImpl->Channel.state != RSSL_CH_STATE_ACTIVE)
		{
			if ((retval = rsslSocketInitChannel(rsslChnlImpl, &inProg, error)) < RSSL_RET_SUCCESS)
			{
				return 0;
			}
		}
	}

	return rsslChnlImpl;
}

RsslRet ipcReconnectClient(RsslSocketChannel* rsslSocketChannel, RsslError* error)
{
	RsslRet retval = RSSL_RET_SUCCESS;

	if (IPC_NULL_PTR(rsslSocketChannel, "ipcReconnectClient", "rsslSocketChannel", error))
		return RSSL_RET_FAILURE;

	IPC_MUTEX_LOCK(rsslSocketChannel);

	if (rsslSocketChannel->workState & RIPC_INT_SHTDOWN_PEND)
	{
		_rsslSetError(error, NULL, RSSL_RET_FAILURE, errno);
		snprintf(error->text, MAX_RSSL_ERROR_TEXT,
			"<%s:%d> Error: 1003 ipcReconnectClient() failed due to channel shutting down.\n",
			__FILE__, __LINE__);

		IPC_MUTEX_UNLOCK(rsslSocketChannel);

		return RSSL_RET_FAILURE;
	}

	retval = (*(rsslSocketChannel->transportFuncs->reconnectClient))(rsslSocketChannel->transportInfo, error);

	IPC_MUTEX_UNLOCK(rsslSocketChannel);

	return(retval);
}

/* rssl Socket ReConnect (for tunneling) */
RsslRet rsslSocketReconnect(rsslChannelImpl *rsslChnlImpl, RsslError *error)
{
	RsslRet 		retVal = RSSL_RET_FAILURE;
	RsslSocketChannel*	rsslSocketChannel = (RsslSocketChannel*)rsslChnlImpl->transportInfo;

	retVal = ipcReconnectClient(rsslSocketChannel, error);

	if (!retVal)
	{
		rsslChnlImpl->Channel.state = RSSL_CH_STATE_CLOSED;
		error->channel = &rsslChnlImpl->Channel;
		return RSSL_RET_FAILURE;
	}
	else
		return RSSL_RET_SUCCESS;
}

/* rssl Socket InitChannel function */
RsslRet rsslSocketInitChannel(rsslChannelImpl* rsslChnlImpl, RsslInProgInfo *inProg, RsslError *error)
{
	ripcSessInProg		ripcInProg;
	RsslRet				retVal = RSSL_RET_FAILURE;
	RsslSocketChannel*	rsslSocketChannel = (RsslSocketChannel*)rsslChnlImpl->transportInfo;

	/* bridge through connected component version info if it wasnt already set */
	if ((!rsslSocketChannel->componentVerLen) && (!rsslSocketChannel->componentVer))
	{
		/* ipc does not own this and will not free or change it */
		rsslSocketChannel->componentVer = rsslChnlImpl->componentVer.componentVersion.data;
		rsslSocketChannel->componentVerLen = rsslChnlImpl->componentVer.componentVersion.length;
	}

	/* Call ipcSessionInit */
	retVal = ipcSessionInit(rsslSocketChannel, &ripcInProg, error);
	inProg->internalConnState = ripcInProg.intConnState;
	switch (retVal)
	{
		case RIPC_CONN_ERROR:
			rsslChnlImpl->Channel.clientHostname = rsslSocketChannel->clientHostname;
			rsslChnlImpl->Channel.clientIP = rsslSocketChannel->clientIP;

			error->channel = &rsslChnlImpl->Channel;
			retVal = RSSL_RET_FAILURE;
			break;

		case RIPC_CONN_IN_PROGRESS:
			if (ripcInProg.types & RIPC_INPROG_NEW_FD)
			{
				inProg->flags = RSSL_IP_FD_CHANGE;

				inProg->newSocket = (RsslSocket)ripcInProg.newSocket.stream;
				inProg->oldSocket = (RsslSocket)ripcInProg.oldSocket;

				rsslChnlImpl->Channel.socketId = (RsslSocket)ripcInProg.newSocket.stream;
			}
			else
			{
				inProg->flags = RSSL_IP_NONE;
				inProg->newSocket = 0;
				inProg->oldSocket = 0;
			}

			retVal = RSSL_RET_CHAN_INIT_IN_PROGRESS;
			break;

		case RIPC_CONN_REFUSED:
			rsslChnlImpl->Channel.clientHostname = rsslSocketChannel->clientHostname;
			rsslChnlImpl->Channel.clientIP = rsslSocketChannel->clientIP;

			error->channel = &rsslChnlImpl->Channel;
			/* possibly map state to inactive here */

			retVal = RSSL_RET_CHAN_INIT_REFUSED;
			break;

		case RIPC_CONN_ACTIVE:
			/* state should go to active */
			rsslChnlImpl->Channel.state = RSSL_CH_STATE_ACTIVE;

			rsslChnlImpl->Channel.clientHostname = rsslSocketChannel->clientHostname;
			rsslChnlImpl->Channel.clientIP = rsslSocketChannel->clientIP;

			rsslChnlImpl->rsslFlags = rsslSocketChannel->rsslFlags;
			rsslChnlImpl->Channel.pingTimeout = rsslSocketChannel->pingTimeout;
			rsslChnlImpl->Channel.majorVersion = rsslSocketChannel->majorVersion;
			rsslChnlImpl->Channel.minorVersion = rsslSocketChannel->minorVersion;
			rsslChnlImpl->Channel.protocolType = rsslSocketChannel->protocolType;


			rsslChnlImpl->Channel.connectionType = rsslSocketChannel->connType;

			if (rsslSocketChannel->protocolType == RIPC_JSON_PROTOCOL_TYPE)
			{
				//Set the rssl Channel function abstractions to handle
				//reading and writing the JSON protocol
				rsslChnlImpl->channelFuncs = rsslGetTransportChannelFunc(RSSL_WEBSOCKET_TRANSPORT);
			}

			/* get channel info here */
			IPC_MUTEX_LOCK(rsslSocketChannel);
			if (rsslSocketChannel->workState & RIPC_INT_SHTDOWN_PEND)
			{
				_rsslSetError(error, &rsslChnlImpl->Channel, RSSL_RET_FAILURE, errno);
				snprintf(error->text, MAX_RSSL_ERROR_TEXT,
					"<%s:%d> Error: 1003 rsslSocketInitChannel failed due to channel shutting down.\n",
					__FILE__, __LINE__);
				error->channel = &rsslChnlImpl->Channel;

				IPC_MUTEX_UNLOCK(rsslSocketChannel);

				return RSSL_RET_FAILURE;
			}
			else if (rsslSocketChannel->intState != RIPC_INT_ST_ACTIVE)
			{
				_rsslSetError(error, &rsslChnlImpl->Channel, RSSL_RET_FAILURE, errno);
				snprintf(error->text, MAX_RSSL_ERROR_TEXT,
						"<%s:%d> Error: 1003 rsslSocketInitChannel failed because the channel is not active.\n",
						__FILE__, __LINE__);

				IPC_MUTEX_UNLOCK(rsslSocketChannel);

				return RSSL_RET_FAILURE;
			}

			rsslChnlImpl->maxMsgSize = (RsslUInt32)rsslSocketChannel->maxUserMsgSize;
			rsslChnlImpl->maxGuarMsgs = rsslSocketChannel->guarBufPool->bufpool.maxBufs;
			/* if we are conn version 13 or higher, we can have a two byte frag Id, otherwise only one */
			if (rsslSocketChannel->version->connVersion > CONN_VERSION_12)
				rsslChnlImpl->fragIdMax = 65535;
			else
				rsslChnlImpl->fragIdMax = 255;

			/* set shared key */
			rsslChnlImpl->shared_key = rsslSocketChannel->shared_key;
			IPC_MUTEX_UNLOCK(rsslSocketChannel);

			retVal = RSSL_RET_SUCCESS;
			break;

		case RIPC_CONN_HTTP_ID_RQD:
			/* This is for tunneling */
			/* What should we return here?  */

			retVal = RSSL_RET_FAILURE;
			break;

		case RIPC_CONN_HTTP_READ:
			/* This is for tunneling */
			/* What should we return here? */

			retVal = RSSL_RET_FAILURE;
			break;

		default:
			/* Should never get here */
			retVal = RSSL_RET_FAILURE;
	}

	return retVal;
}

/* rssl Socket CloseChannel */
RsslRet rsslSocketCloseChannel(rsslChannelImpl* rsslChnlImpl, RsslError *error)
{
	RsslRet 		retVal = RSSL_RET_SUCCESS;
	RsslSocketChannel*		rsslSocketChannel = (RsslSocketChannel*)rsslChnlImpl->transportInfo;

	rsslChnlImpl->Channel.state = RSSL_CH_STATE_INACTIVE;

	if ((rsslChnlImpl->returnBufferOwner == 1) && (rsslChnlImpl->returnBuffer.data))
	{
		/* I own this, means it was fragmented. release memory */
		_rsslFree(rsslChnlImpl->returnBuffer.data);
	}

		retVal = ipcShutdownSockectChannel(rsslSocketChannel, error);

		if (retVal < RSSL_RET_SUCCESS)
		{
			/* should we return here?? */
			error->channel = &rsslChnlImpl->Channel;

			return retVal;
		}
		else
		{
			retVal = ipcSessDropRef(rsslSocketChannel, error);
		}
		/* check for dropRef error */
		if (retVal < RSSL_RET_SUCCESS)
			error->channel = &rsslChnlImpl->Channel;
		else
			retVal = RSSL_RET_SUCCESS;

	if (rsslChnlImpl->componentInfo)
	{
		_rsslFree(rsslChnlImpl->componentInfo[0]);
		_rsslFree(rsslChnlImpl->componentInfo);
	}

	return retVal;
}

/* rssl Socket Read */
RSSL_RSSL_SOCKET_IMPL_FAST(RsslBuffer*) rsslSocketRead(rsslChannelImpl* rsslChnlImpl, RsslReadOutArgs *readOutArgs, RsslRet *readRet, RsslError *error)
{
	rtr_msgb_t     *ripcBuffer = 0;
	RsslRet        ipcReadRet;
	RsslInt32      ripcMoreData = 0;
	RsslInt32	   ripcFragSize = 0;
	RsslInt32      ripcFragId = 0;
	RsslInt32      packing = 0;
	RsslInt32      returnNull = 0;
	RsslInt32      inBytes = 0;
	RsslInt32      uncompInBytes = 0;
	rsslAssemblyBuffer *rsslAssemblyBuf = 0;
	RsslSocketChannel  *rsslSocketChannel = (RsslSocketChannel*)rsslChnlImpl->transportInfo;

	if (IPC_NULL_PTR(rsslSocketChannel, "rsslSocketRead", "rsslSocketChannel", error))
		return NULL;

	if (multiThread == RSSL_LOCK_GLOBAL_AND_CHANNEL)
	{

		_DEBUG_MUTEX_TRACE("RSSL_MUTEX_LOCK", rsslChnlImpl, rsslChnlImpl->chanMutex)
	  if (RSSL_MUTEX_LOCK(&rsslChnlImpl->chanMutex))
	  {
		*readRet = RSSL_RET_READ_IN_PROGRESS;
		return NULL;
	  }
	}

	/* if this channel has a returnBuffer - clean it */
	if (rsslChnlImpl->returnBuffer.length)
	{
		if ((rsslChnlImpl->returnBufferOwner == 1) && (rsslChnlImpl->returnBuffer.data))
		{
			/* I own this, means it was fragmented. release memory */
			_rsslFree(rsslChnlImpl->returnBuffer.data);
		}

		/* now default this stuff */
		rsslChnlImpl->returnBuffer.data = 0;
		rsslChnlImpl->returnBuffer.length = 0;
		rsslChnlImpl->returnBufferOwner = 0;
	}

	/* packed */
	if ((rsslChnlImpl->packedBuffer))
	{
		RsslUInt16 bufLength = 0;
		/* Im not owner */
		rsslChnlImpl->returnBufferOwner = 0;

		if (readOutArgs != NULL)
		{
			readOutArgs->bytesRead = 0;
			readOutArgs->uncompressedBytesRead = 0;
		}

		rsslChnlImpl->unpackOffset += rwfGet16(bufLength, (rsslChnlImpl->packedBuffer->buffer + rsslChnlImpl->unpackOffset));
		rsslChnlImpl->returnBuffer.length = bufLength;
		rsslChnlImpl->returnBuffer.data = rsslChnlImpl->packedBuffer->buffer + rsslChnlImpl->unpackOffset;

		if ((rsslChnlImpl->packedBuffer->buffer + rsslChnlImpl->packedBuffer->length) != (rsslChnlImpl->returnBuffer.data + rsslChnlImpl->returnBuffer.length))
		{
			/* packedBuffer should already be set */
			rsslChnlImpl->unpackOffset += rsslChnlImpl->returnBuffer.length;
			/* still have more data */
			ripcMoreData = 1;
		}
		else
		{
			/* no more packed data */
			rsslChnlImpl->unpackOffset = 0;
			rsslChnlImpl->packedBuffer = 0;
			/* This is a 'workaround' so that the user will call read one more time after the buffer is empty
			   some notifiers, etc. require that an EWOULDBLOCK is returned before they reset themselves */
			ripcMoreData = 1;
		}

		if (multiThread == RSSL_LOCK_GLOBAL_AND_CHANNEL)
		{
		  (void) RSSL_MUTEX_UNLOCK(&rsslChnlImpl->chanMutex);
			_DEBUG_MUTEX_TRACE("RSSL_MUTEX_UNLOCK", rsslChnlImpl, rsslChnlImpl->chanMutex)
		}

		*readRet = ripcMoreData;

		if ((rsslChnlImpl->debugFlags & RSSL_DEBUG_RSSL_DUMP_IN) && (rsslChnlImpl->returnBuffer.length))
			(*(rsslSocketDumpInFunc))(__FUNCTION__, rsslChnlImpl->returnBuffer.data, rsslChnlImpl->returnBuffer.length, rsslChnlImpl->Channel.socketId);

		return &(rsslChnlImpl->returnBuffer);
	}

	/* This can only happen in the threaded case */
	if (rsslSocketChannel->workState & RIPC_INT_READ_THR)
	{
		ipcReadRet = RSSL_RET_READ_WOULD_BLOCK;

		ripcBuffer = 0;
	}
	else
	{
		rsslSocketChannel->workState |= RIPC_INT_READ_THR;

		ripcBuffer = (rtr_msgb_t *)ipcReadSession(rsslSocketChannel, &ipcReadRet, &ripcMoreData, &ripcFragSize, &ripcFragId, &inBytes, &uncompInBytes, &packing, error);
		
		rsslSocketChannel->workState &= ~RIPC_INT_READ_THR;
	}

	/* non packed */
	if (ripcBuffer != 0)
	{
		if(readOutArgs != NULL)
		{
			readOutArgs->bytesRead = inBytes;
			readOutArgs->uncompressedBytesRead = uncompInBytes;
		}
		/* we have read something */
		/* map ripcBuffer to RsslBuffer */
		if ((ripcFragSize == 0) && (ripcFragId == 0))
		{
			/* message is one fragment */
			if (ripcBuffer->length > 0)
			{
				/* im not the memory owner */
				rsslChnlImpl->returnBufferOwner = 0;

				/* check for packing */
				if (packing != 0)
				{
					RsslUInt16 bufLength = 0;
					/* packing occurred */
					/* set the length */
					rsslChnlImpl->unpackOffset = rwfGet16(bufLength, ripcBuffer->buffer);
					rsslChnlImpl->returnBuffer.length = bufLength;
					rsslChnlImpl->returnBuffer.data = ripcBuffer->buffer + rsslChnlImpl->unpackOffset;

					/* check if this is the only message in the packed buffer -
					  if so, just continue like normal */
					if ((ripcBuffer->buffer + ripcBuffer->length) != (rsslChnlImpl->returnBuffer.data + rsslChnlImpl->returnBuffer.length))
					{
						/* we have more data to unpack */
						rsslChnlImpl->packedBuffer = ripcBuffer;
						/* want to keep track of what this originally was, so if we have more ripcBuffers
						   waiting, we will eventually read them */
						rsslChnlImpl->moreData = ripcMoreData;
						/* set the offset */
						rsslChnlImpl->unpackOffset += rsslChnlImpl->returnBuffer.length;

						/* this has to be true in this case */
						ripcMoreData = 1;
					}
					else
					{
						/* no more packed messages */
						rsslChnlImpl->unpackOffset = 0;
						/* This is a 'workaround' so that the user will call read one more time after the buffer is empty
						 some notifiers, etc. require that an EWOULDBLOCK is returned before they reset themselves */
						ripcMoreData = 1;

						rsslChnlImpl->moreData = 0;
						rsslChnlImpl->packedBuffer = 0;
					}
				}
				else
				{
					rsslChnlImpl->returnBuffer.length = (RsslUInt32)ripcBuffer->length;
					rsslChnlImpl->returnBuffer.data = ripcBuffer->buffer;
				}
			}
			else
			{
				/* received a ping with no payload */
				returnNull = 1;
			}
		}
		else if (ripcFragSize > 0)
		{
			/* first fragment in a fragmented message */
			rsslAssemblyBuf = (rsslAssemblyBuffer*)_rsslMalloc(sizeof(rsslAssemblyBuffer));

			if (rsslAssemblyBuf)
			{
				_rsslCleanAssemblyBuffer(rsslAssemblyBuf);
				/* now create the data portion */
				rsslAssemblyBuf->buffer.data = (char*)_rsslMalloc(ripcFragSize + 7);
				if (rsslAssemblyBuf->buffer.data)
					rsslAssemblyBuf->buffer.length = ripcFragSize;
			}

			if (!rsslAssemblyBuf || !rsslAssemblyBuf->buffer.data)
			{
				/* error */
				if (multiThread == RSSL_LOCK_GLOBAL_AND_CHANNEL)
				{
				  (void) RSSL_MUTEX_UNLOCK(&rsslChnlImpl->chanMutex);
					_DEBUG_MUTEX_TRACE("RSSL_MUTEX_UNLOCK", rsslChnlImpl, rsslChnlImpl->chanMutex)
				}

				if (rsslAssemblyBuf)
					_rsslFree(rsslAssemblyBuf);

				_rsslSetError(error, &rsslChnlImpl->Channel, RSSL_RET_BUFFER_NO_BUFFERS, 0);
				snprintf(error->text, MAX_RSSL_ERROR_TEXT, "<%s:%d> Error: 0005 rsslSocketRead() Cannot allocate memory of size %d for read buffer.\n", __FILE__, __LINE__, ripcFragSize);
				*readRet = RSSL_RET_FAILURE;
				return NULL;
			}

			MemCopyByInt(rsslAssemblyBuf->buffer.data, ripcBuffer->buffer, ripcBuffer->length);
			rsslAssemblyBuf->fragId = ripcFragId;
			rsslAssemblyBuf->readCursor += (RsslUInt32)ripcBuffer->length;

			/* need to check if we have the entire message here */
			if (rsslAssemblyBuf->readCursor == rsslAssemblyBuf->buffer.length)
			{
				/* no more data for this - not sure why fragmentation was used in this case */
				/* its not in the queue yet so we do not have to remove it */

				/* i am the owner since its a fragment */
				rsslChnlImpl->returnBufferOwner = 1;

				rsslChnlImpl->returnBuffer.length = rsslAssemblyBuf->buffer.length;
				rsslChnlImpl->returnBuffer.data = rsslAssemblyBuf->buffer.data;

				/* clean up assembly buffer */
				_rsslFree(rsslAssemblyBuf);
				rsslAssemblyBuf=0;
			}
			else
			{
				rsslAssemblyBuffer	*rsslTempAssemblyBuf=0;
				RsslHashLink		*rsslHashLink;
				RsslUInt32			hashSum;
				/* there is more data coming - put this in the queue */

				/* before we put it in the queue, check if one is there with the same frag id */
				/* this should be rare - it means we didnt receive the entire message and we
				   have looped around back to this frag id */
				/* if this happens we should remove the old one - odds are we are never completing it
				   and even if we did, its probably not very important anymore */
				/* hopefully, this will never happen - but it is a precaution we need to take because it will
				   really screw up reassembly */

				hashSum = UInt32_key_hash(rsslAssemblyBuf);

				rsslHashLink = rsslHashTableFind(&rsslChnlImpl->assemblyBuffers, rsslAssemblyBuf, &hashSum);

				if (rsslHashLink)
				{
					rsslTempAssemblyBuf = RSSL_HASH_LINK_TO_OBJECT(rsslAssemblyBuffer, link1, rsslHashLink);

					/* we found one */

					rsslHashTableRemoveLink(&rsslChnlImpl->assemblyBuffers, rsslHashLink);

					_DEBUG_TRACE_BUFFER("removing from assemblyBuffers hash\n")

					/* now release this memory back into the pool */
					_rsslFree(rsslTempAssemblyBuf->buffer.data);
					rsslTempAssemblyBuf->buffer.length = 0;
					_rsslFree(rsslTempAssemblyBuf);
					rsslTempAssemblyBuf = 0;
				}
				/* if we are fragmenting, we have nothing to return in this case so we need to return NULL */
				returnNull = 2;

				rsslHashTableInsertLink(&(rsslChnlImpl->assemblyBuffers), &(rsslAssemblyBuf->link1), rsslAssemblyBuf, &hashSum);

				_DEBUG_TRACE_BUFFER("inserting into assemblyBuffers hash\n")
			}
		}
		else
		{	/* should be another/subsequent fragment in the current message */
			rsslAssemblyBuffer	rsslAssemblyBufferSearch;
			RsslHashLink		*rsslHashLink;

			_rsslCleanAssemblyBuffer(&rsslAssemblyBufferSearch);

			if (ripcBuffer->length == 0)	/* if we get a subsequent fragment with a zero size, ignore it */
			{
				/* we cant just blindly return ping here because there may be more data in the input buffer.
				   to combat this, check if ripc said there is more data to read, we do not know whether it is a full message
				   available or not, but if there is anything there we should tell app to read again */
				if (ripcMoreData)
					*readRet = ripcMoreData;
				else
					*readRet = RSSL_RET_READ_PING;
				return NULL;
			}

			/* use the frag ID to determine which message this fragment is a part of */
			rsslAssemblyBufferSearch.fragId = ripcFragId;
			rsslHashLink = rsslHashTableFind(&rsslChnlImpl->assemblyBuffers, &rsslAssemblyBufferSearch, NULL);

			if (rsslHashLink)
				rsslAssemblyBuf = RSSL_HASH_LINK_TO_OBJECT(rsslAssemblyBuffer, link1, rsslHashLink);

			if (!rsslHashLink || !rsslAssemblyBuf->buffer.data)
			{
				/* error */
				if (multiThread == RSSL_LOCK_GLOBAL_AND_CHANNEL)
				{
				  (void) RSSL_MUTEX_UNLOCK(&rsslChnlImpl->chanMutex);
					_DEBUG_MUTEX_TRACE("RSSL_MUTEX_UNLOCK", rsslChnlImpl, rsslChnlImpl->chanMutex)
				}

				if (rsslAssemblyBuf)
					_rsslFree(rsslAssemblyBuf);

				_rsslSetError(error, &rsslChnlImpl->Channel, RSSL_RET_BUFFER_NO_BUFFERS, 0);
				snprintf(error->text, MAX_RSSL_ERROR_TEXT, "<%s:%d> rsslRead() Error: 0014 Attempting to reassemble a message with frag ID %d without seeing first fragment.\n",
						__FILE__, __LINE__, ripcFragId);
				*readRet = RSSL_RET_FAILURE;
				return NULL;
			}

			MemCopyByInt((rsslAssemblyBuf->buffer.data) + rsslAssemblyBuf->readCursor, ripcBuffer->buffer, ripcBuffer->length);

			rsslAssemblyBuf->readCursor += (RsslUInt32)ripcBuffer->length;

			if (rsslAssemblyBuf->readCursor == rsslAssemblyBuf->buffer.length)
			{
				/* should be the last fragment */
				/* set the buffer we return to the readBuffer */
				/* create bufimpl here and set that all up and then remove this assembly buffer from hash */

				/* remove buffer from hash */
				rsslHashTableRemoveLink(&rsslChnlImpl->assemblyBuffers, &rsslAssemblyBuf->link1);
				_DEBUG_TRACE_BUFFER("removing from assemblyBuffers hash\n")

				/* I am the owner */
				rsslChnlImpl->returnBufferOwner = 1;

				rsslChnlImpl->returnBuffer.length = rsslAssemblyBuf->buffer.length;
				rsslChnlImpl->returnBuffer.data = rsslAssemblyBuf->buffer.data;

				/* clean up rsslAssemblyBuf */
				_rsslFree(rsslAssemblyBuf);
				rsslAssemblyBuf = 0;
			}
			else
				returnNull = 2;
		}

		if (multiThread == RSSL_LOCK_GLOBAL_AND_CHANNEL)
		{
			(void) RSSL_MUTEX_UNLOCK(&rsslChnlImpl->chanMutex);
			_DEBUG_MUTEX_TRACE("RSSL_MUTEX_UNLOCK", rsslChnlImpl, rsslChnlImpl->chanMutex)
		}

		if (ripcMoreData)
		{
			*readRet = ripcMoreData;

			/* it is possible that the returnBuffer is null in this case -
			   this is because of fragmentation, if we receive multiple fragments but do not have the
			   entire message, we return moreData but no buffer */
			if ((rsslChnlImpl->debugFlags & RSSL_DEBUG_RSSL_DUMP_IN) && (rsslChnlImpl->returnBuffer.length ))
				(*(rsslSocketDumpInFunc))(__FUNCTION__, rsslChnlImpl->returnBuffer.data, rsslChnlImpl->returnBuffer.length, rsslChnlImpl->Channel.socketId);

			if (!returnNull)
				return &(rsslChnlImpl->returnBuffer);
			else
				return NULL;
		}
		else
		{
			if ((rsslAssemblyBuf) && (rsslAssemblyBuf->readCursor > 0) && (rsslAssemblyBuf->readCursor != rsslAssemblyBuf->buffer.length))
			{
				*readRet = 1;
				return NULL;
			}
			else
			{
				if ((rsslChnlImpl->debugFlags & RSSL_DEBUG_RSSL_DUMP_IN) && (rsslChnlImpl->returnBuffer.length))
					(*(rsslSocketDumpInFunc))(__FUNCTION__, rsslChnlImpl->returnBuffer.data, rsslChnlImpl->returnBuffer.length, rsslChnlImpl->Channel.socketId);

				if (!returnNull)
				{
					*readRet = RSSL_RET_SUCCESS;
					return &(rsslChnlImpl->returnBuffer);
				}
				else
				{
					/* this was a ping message */
					if (returnNull == 1)
						*readRet = RSSL_RET_READ_PING;
					return NULL;
				}
			}
		}
	}
	else
	{
		switch (ipcReadRet)
		{
		case RSSL_RET_FAILURE:
				/* if read fails we need to update state */
				rsslChnlImpl->Channel.state = RSSL_CH_STATE_CLOSED;

				error->channel = &rsslChnlImpl->Channel;

				if (multiThread == RSSL_LOCK_GLOBAL_AND_CHANNEL)
				{
				  (void) RSSL_MUTEX_UNLOCK(&rsslChnlImpl->chanMutex);
					_DEBUG_MUTEX_TRACE("RSSL_MUTEX_UNLOCK", rsslChnlImpl, rsslChnlImpl->chanMutex)
				}
				*readRet = ipcReadRet;
				return NULL;

			case RSSL_RET_READ_FD_CHANGE:
				rsslChnlImpl->Channel.oldSocketId = (RsslSocket)rsslSocketChannel->oldStream;
				rsslChnlImpl->Channel.socketId = (RsslSocket)rsslSocketChannel->stream;

				if (multiThread == RSSL_LOCK_GLOBAL_AND_CHANNEL)
				{
				  (void) RSSL_MUTEX_UNLOCK(&rsslChnlImpl->chanMutex);
					_DEBUG_MUTEX_TRACE("RSSL_MUTEX_UNLOCK", rsslChnlImpl, rsslChnlImpl->chanMutex)
				}
				*readRet = ipcReadRet;
				return NULL;

			case RSSL_RET_SUCCESS:
				/* Ripc read the first part of a ripc fragmented message */
				/* Tell the user to read again so ripc can get the second part and reassemble it */
				if(readOutArgs != NULL)
				{
					readOutArgs->bytesRead = inBytes;
					readOutArgs->uncompressedBytesRead = uncompInBytes;
				}

				if (multiThread == RSSL_LOCK_GLOBAL_AND_CHANNEL)
				{
				  (void) RSSL_MUTEX_UNLOCK(&rsslChnlImpl->chanMutex);
					_DEBUG_MUTEX_TRACE("RSSL_MUTEX_UNLOCK", rsslChnlImpl, rsslChnlImpl->chanMutex)
				}
				*readRet = 1;
				return NULL;

			case RSSL_RET_READ_WOULD_BLOCK:
				if(readOutArgs != NULL)
				{
					readOutArgs->bytesRead += inBytes;
					readOutArgs->uncompressedBytesRead += uncompInBytes;
				}

				if (multiThread == RSSL_LOCK_GLOBAL_AND_CHANNEL)
				{
				  (void) RSSL_MUTEX_UNLOCK(&rsslChnlImpl->chanMutex);
					_DEBUG_MUTEX_TRACE("RSSL_MUTEX_UNLOCK", rsslChnlImpl, rsslChnlImpl->chanMutex)
				}
				*readRet = ipcReadRet;
				return NULL;

			default: /* should never get here */
				if (multiThread == RSSL_LOCK_GLOBAL_AND_CHANNEL)
				{
				  (void) RSSL_MUTEX_UNLOCK(&rsslChnlImpl->chanMutex);
					_DEBUG_MUTEX_TRACE("RSSL_MUTEX_UNLOCK", rsslChnlImpl, rsslChnlImpl->chanMutex)
				}
				*readRet = RSSL_RET_FAILURE;
				return NULL;
		}
	}

	return NULL;
}

/* rssl Socket Write */
RSSL_RSSL_SOCKET_IMPL_FAST(RsslRet) rsslSocketWrite(rsslChannelImpl *rsslChnlImpl, rsslBufferImpl *rsslBufImpl, RsslWriteInArgs *writeInArgs,
	RsslWriteOutArgs *writeOutArgs, RsslError *error)
{
	RsslInt32 retVal = RSSL_RET_FAILURE;
	RsslUInt32 outBytes = 0;
	RsslUInt32 uncompOutBytes = 0;
	RsslUInt32 totalOutBytes = 0;
	RsslUInt32 totalUncompOutBytes = 0;
	RsslUInt32 writeFlags = writeInArgs->writeInFlags;
	rtr_msgb_t	*ripcBuffer;
	RsslSocketChannel *rsslSocketChannel = (RsslSocketChannel*)rsslChnlImpl->transportInfo;

	if (IPC_NULL_PTR(rsslSocketChannel, "rsslSocketWrite", "rsslSocketChannel", error))
		return RSSL_RET_FAILURE;

	// Set ripcBuffer to bufferInfo on rsslBufImpl.  When this changes, we will update rsslBufImpl->bufferInfo at the same time.
	ripcBuffer = (rtr_msgb_t*)(rsslBufImpl->bufferInfo);


	/* check if we are doing fragmentation */
	if (ripcBuffer && (!(rsslBufImpl->fragmentationFlag)) && (rsslBufImpl->writeCursor == 0))
	{
		/* no fragmentation */

		/* packed case */
		if (rsslBufImpl->packingOffset > 0)
		{
			RsslUInt16 bufLength;

			/* if the length is zero, then there is no message at the end. */
			/* We can take out the space we allocated for its length */
			if (rsslBufImpl->buffer.length == 0)
			{
				rsslBufImpl->packingOffset -= 2;
			}
			else
			{
				bufLength = rsslBufImpl->buffer.length;
				rwfPut16((ripcBuffer->buffer + rsslBufImpl->packingOffset - 2), bufLength);	/* fill in the length of the last message */
				rsslBufImpl->packingOffset += rsslBufImpl->buffer.length;						/* advance the packing offset to include this last message */
			}
			ripcBuffer->length = rsslBufImpl->packingOffset;		/* the packing offset is the entire length of everything in the buffer we want to send */
		}
		else
		{
			/* standard case - buffer is within size bounds */
			/* make sure rssl buffer matches ripcbuffer size */
			ripcBuffer->length = rsslBufImpl->buffer.length;
		}

		ripcBuffer->priority = rsslBufImpl->priority;

		/* Queue the buffer for writing */
		retVal = ipcWriteSession(rsslSocketChannel, rsslBufImpl, writeFlags, (RsslInt32*)&outBytes, (RsslInt32*)&uncompOutBytes, (writeFlags & RSSL_WRITE_DIRECT_SOCKET_WRITE) != 0, error);

		totalOutBytes += outBytes;
		outBytes = 0;
		totalUncompOutBytes += uncompOutBytes;
		uncompOutBytes = 0;
	}
	else
	{	/* fragmentation */
		RsslUInt32 tempSize = 0;
		RsslUInt32 copyUserMsgSize = 0;
		RsslUInt16 fragHeaderSize = 0;
		rtr_msgb_t	*nextBuffer;
		RsslBool firstFragment = rsslBufImpl->fragmentationFlag == BUFFER_IMPL_FIRST_FRAG_HEADER ? 1 : 0;

		/* sets and increments frag id for this chain */
		/* for WRITE_CALL_AGAIN fragId should already be set */
		if (rsslBufImpl->fragId == 0)
		{
			if (multiThread == RSSL_LOCK_GLOBAL_AND_CHANNEL)
			{
			  (void) RSSL_MUTEX_LOCK(&rsslChnlImpl->chanMutex);
				_DEBUG_MUTEX_TRACE("RSSL_MUTEX_LOCK", rsslChnlImpl, rsslChnlImpl->chanMutex)
			}
			rsslBufImpl->fragId = rsslChnlImpl->fragId;
			if (rsslChnlImpl->fragId == rsslChnlImpl->fragIdMax)
				rsslChnlImpl->fragId = 1;
			else
				rsslChnlImpl->fragId++;
			if (multiThread == RSSL_LOCK_GLOBAL_AND_CHANNEL)
			{
			  (void) RSSL_MUTEX_UNLOCK(&rsslChnlImpl->chanMutex);
				_DEBUG_MUTEX_TRACE("RSSL_MUTEX_UNLOCK", rsslChnlImpl, rsslChnlImpl->chanMutex)
			}
		}

		/* chained message - this means that I allocated the memory.
		   I have to break it into its respective ripc messages,
		   set the sizes accordingly, and then free the memory on a successful write */

		/* even though the memcpy's arent the most efficient,
		   this case should only happen on messages that are > than
		   max message size, so this shouldnt happen very frequently. */
		tempSize = rsslBufImpl->buffer.length - rsslBufImpl->writeCursor;

		/* This while loop support the WRITE_CALL_AGAIN case as well*/
		while (tempSize > 0)
		{
			if (ripcBuffer)
			{
				do
				{
					if (firstFragment)
					{
						fragHeaderSize = FIRST_FRAG_OVERHEAD;
						firstFragment = 0;
					}
					else
					{
						fragHeaderSize = SUBSEQ_FRAG_OVERHEAD;
					}

					copyUserMsgSize = rsslChnlImpl->maxMsgSize - fragHeaderSize;

					if (tempSize >= copyUserMsgSize)
					{
						/* use rsslChnlImpl->maxMsgSize here - because of tunneling, server side ripcBuffer->maxLen can have
							some additional bytes available there that we shouldnt be using */
						MemCopyByInt(ripcBuffer->buffer, rsslBufImpl->buffer.data + rsslBufImpl->writeCursor, copyUserMsgSize);
						tempSize -= copyUserMsgSize;
						rsslBufImpl->writeCursor += copyUserMsgSize;
						ripcBuffer->length = copyUserMsgSize;
						ripcBuffer->priority = rsslBufImpl->priority;

						/* go to the next buffer if needed*/
						if (ripcBuffer->nextMsg)
						{
							nextBuffer = ripcBuffer->nextMsg;
							ripcBuffer = nextBuffer;
						}
						else
						{
							nextBuffer = NULL;
						}
					}
					else
					{
						/* this will all actually fit in one message */
						MemCopyByInt(ripcBuffer->buffer, rsslBufImpl->buffer.data + rsslBufImpl->writeCursor, tempSize);
						rsslBufImpl->writeCursor += tempSize;
						ripcBuffer->length = tempSize;
						ripcBuffer->priority = rsslBufImpl->priority;
						tempSize -= tempSize;
						break;
					}

				} while (nextBuffer);

				retVal = ipcWriteSession(rsslSocketChannel, rsslBufImpl, writeFlags, (RsslInt32*)&outBytes, (RsslInt32*)&uncompOutBytes, (writeFlags & RSSL_WRITE_DIRECT_SOCKET_WRITE) != 0, error);

				rsslBufImpl->fragmentationFlag = BUFFER_IMPL_SUBSEQ_FRAG_HEADER;
				totalOutBytes += outBytes;
				outBytes = 0;
				totalUncompOutBytes += uncompOutBytes;
				uncompOutBytes = 0;

				if (tempSize == 0 || retVal == RSSL_RET_FAILURE)
					break;
			}

			copyUserMsgSize = rsslChnlImpl->maxMsgSize - SUBSEQ_FRAG_OVERHEAD;

			/* Created an individual subsequent fragmented buffer */
			ripcBuffer = (void*)ipcFragmentationDataBuffer(rsslSocketChannel, 0, tempSize >= copyUserMsgSize ? copyUserMsgSize : tempSize, error);
			if (ripcBuffer == NULL)
			{
				/* call flush and try again, then return error */
				rsslFlush(&rsslChnlImpl->Channel, error);

				ripcBuffer = (void*)ipcFragmentationDataBuffer(rsslSocketChannel, 0, tempSize >= copyUserMsgSize ? copyUserMsgSize : tempSize, error);
				if (ripcBuffer == NULL)
				{
					/* return error here */
					error->channel = &rsslChnlImpl->Channel;

					/* return the callWriteAgain error here */
					return RSSL_RET_WRITE_CALL_AGAIN;
				}
			}
			/* set the new buffer on the rsslBufImpl->bufferInfo.  Since we've already sent the first chain set, this should go through the loop without another nextBuffer. */
			rsslBufImpl->bufferInfo = (void*)ripcBuffer;
		}
	}

	if (retVal == RSSL_RET_FAILURE)
	{
		if (rsslBufImpl->bufferInfo)
		{
			/* if write fails we should close socket */
			rsslChnlImpl->Channel.state = RSSL_CH_STATE_CLOSED;

			/* in this case, no buffers are freed */
			error->channel = &rsslChnlImpl->Channel;

			return RSSL_RET_FAILURE;
		}
		else
		{
			if ((errno == EINTR) || (errno == EAGAIN) || (errno == _IPC_WOULD_BLOCK))
			{
				/* flush was blocked */
				rsslBufImpl->bufferInfo = NULL;
				return RSSL_RET_WRITE_FLUSH_FAILED;
			}
			else
			{
				/* socket error */
				rsslChnlImpl->Channel.state = RSSL_CH_STATE_CLOSED;
				rsslBufImpl->bufferInfo = NULL;
				error->channel = &rsslChnlImpl->Channel;

				return RSSL_RET_WRITE_FLUSH_FAILED;
			}
		}
	}
	else
	{
		/* if its a successful write ripc should have freed its buffer so we
			should follow suit and free the RsslBuffer here */
		/* first remove it from the list */
		if (multiThread == RSSL_LOCK_GLOBAL_AND_CHANNEL)
		{
		  (void) RSSL_MUTEX_LOCK(&rsslChnlImpl->chanMutex);
			_DEBUG_MUTEX_TRACE("RSSL_MUTEX_LOCK", rsslChnlImpl, rsslChnlImpl->chanMutex)
		}
		if (rsslQueueLinkInAList(&(rsslBufImpl->link1)))
		{
			rsslQueueRemoveLink(&(rsslChnlImpl->activeBufferList), &(rsslBufImpl->link1));
			_DEBUG_TRACE_BUFFER("removing from activeBufferList\n")
		}
		if (multiThread == RSSL_LOCK_GLOBAL_AND_CHANNEL)
		{
		  (void) RSSL_MUTEX_UNLOCK(&rsslChnlImpl->chanMutex);
			_DEBUG_MUTEX_TRACE("RSSL_MUTEX_UNLOCK", rsslChnlImpl, rsslChnlImpl->chanMutex)
		}

		if (rsslBufImpl->writeCursor > 0)
		{
			/* if we get here, the message has been written fully so reset writeCursor and fragId */
			/* this means I allocated the data portion */
			rsslBufImpl->writeCursor = 0;
			rsslBufImpl->fragId = 0;
			rsslBufImpl->owner = 0;
			_rsslFree(rsslBufImpl->buffer.data);
			rsslBufImpl->buffer.length = 0;
		}

		/* now add to free buffer list */
		_rsslCleanBuffer(rsslBufImpl);
		_DEBUG_TRACE_BUFFER("adding to freeBufferList\n")

		if (multiThread == RSSL_LOCK_GLOBAL_AND_CHANNEL)
		{
		  (void) RSSL_MUTEX_LOCK(&rsslChnlImpl->chanMutex);
			_DEBUG_MUTEX_TRACE("RSSL_MUTEX_LOCK", rsslChnlImpl, rsslChnlImpl->chanMutex)
		}
		rsslInitQueueLink(&(rsslBufImpl->link1));
		rsslQueueAddLinkToBack(&(rsslChnlImpl->freeBufferList), &(rsslBufImpl->link1));
		if (multiThread == RSSL_LOCK_GLOBAL_AND_CHANNEL)
		{
		  (void) RSSL_MUTEX_UNLOCK(&rsslChnlImpl->chanMutex);
			_DEBUG_MUTEX_TRACE("RSSL_MUTEX_UNLOCK", rsslChnlImpl, rsslChnlImpl->chanMutex)
		}

		/* Write was either 0 or number of bytes left to be written */
		/* if it was 0, we should have -2 returned which corresponds to a blocked socket */
		writeOutArgs->bytesWritten = totalOutBytes;
		writeOutArgs->uncompressedBytesWritten = totalUncompOutBytes;
		/* retVal should be number of bytes left to be written */
		return retVal;
	}
}

/* rssl Socket GetBuffer */
RSSL_RSSL_SOCKET_IMPL_FAST(rsslBufferImpl*) rsslSocketGetBuffer(rsslChannelImpl *rsslChnlImpl, RsslUInt32 size, RsslBool packedBuffer, RsslError *error)
{
	rtr_msgb_t *ipcBuf=0;
	rsslBufferImpl *rsslBufImpl=0;
	RsslInt32 maxOutputMsgs;
	RsslCompTypes compression;
	RsslUInt8	fragmentation = 0;

	RsslSocketChannel*	rsslSocketChannel = (RsslSocketChannel*)rsslChnlImpl->transportInfo;

	if (packedBuffer) {
	  if (size >= UINT32_MAX - 2) {
		_rsslSetError(error, &rsslChnlImpl->Channel, RSSL_RET_FAILURE, 0);
		snprintf(error->text, MAX_RSSL_ERROR_TEXT, "<%s:%d> rsslGetBuffer() Error: 0010 Invaid buffer size specified.\n", __FILE__, __LINE__);
		return NULL;
	  }
	  else
		size += 2;		/* make room for the message length that goes at the beginning */
	}

	/* not fragmented, can be packable */
	if (size <= rsslChnlImpl->maxMsgSize)
	{
		ipcBuf = ipcDataBuffer(rsslSocketChannel, size, error);
	}
	else
	{
		RsslInt32 usedBuf;

		/* must be fragmented case */
		if (packedBuffer)
		{
			_rsslSetError(error, &rsslChnlImpl->Channel, RSSL_RET_FAILURE, 0);
			snprintf(error->text, MAX_RSSL_ERROR_TEXT, "<%s:%d> Error: 0015 rsslSocketGetBuffer() Cannot get a packed buffer larger than maximum message size for connection.\n", __FILE__, __LINE__);
			return NULL;
		}

		IPC_MUTEX_LOCK(rsslSocketChannel);

		/* Return the number of guaranteed buffers used + pool buffers used */
		usedBuf = rsslSocketChannel->guarBufPool->numRegBufsUsed + rsslSocketChannel->guarBufPool->numPoolBufs;
		maxOutputMsgs = rsslSocketChannel->guarBufPool->maxPoolBufs + rsslSocketChannel->guarBufPool->bufpool.maxBufs;
		compression = rsslSocketChannel->outCompression;

		IPC_MUTEX_UNLOCK(rsslSocketChannel);

		if (usedBuf < 0)
		{
			error->channel = &rsslChnlImpl->Channel;
			return NULL;
		}

		/* fragmentation */
		/* the top DataBuffer statement is for cases where our maxGuarMsgs is larger - the reason we need to
		do this check is for compression.  We need to keep a few guaranteed buffers available in the case
		that we compress - ripc needs to be able to get a new buffer to compress into. We also need to get
		at least 2 buffers on the first fragmented message - this sets up the fragmentation header in ripc.
		*/
		if ((((maxOutputMsgs - usedBuf) < 4) && compression) ||
			((maxOutputMsgs - usedBuf) < 2))
		{
			_rsslSetError(error, &rsslChnlImpl->Channel, RSSL_RET_BUFFER_NO_BUFFERS, 0);
			snprintf(error->text, MAX_RSSL_ERROR_TEXT,
					"<%s:%d> Error: 0016 rsslSocketGetBuffer() Cannot obtain enough buffers for fragmentation to occur.\n", __FILE__, __LINE__);
			return NULL;
		}
		else
		{
			fragmentation = 1;

			/* Subtract the data size with FIRST_FRAG_HEADER_SIZE to create a buffer chain which has the first header and only one secondary header */
			ipcBuf = ipcFragmentationDataBuffer(rsslSocketChannel, 1, (rsslChnlImpl->maxMsgSize * 2) - FIRST_FRAG_HEADER_SIZE, error);
		}
	}

	if (ipcBuf == NULL)
	{
		/* cant get buffer */
		error->channel = &rsslChnlImpl->Channel;

		return NULL;
	}

	/* else successful - now allocate rsslbuffer */
	rsslBufImpl = _rsslNewBuffer(rsslChnlImpl);

	if (rsslBufImpl == NULL)
	{
		_rsslSetError(error, &rsslChnlImpl->Channel, RSSL_RET_BUFFER_NO_BUFFERS, 0);
		snprintf(error->text, MAX_RSSL_ERROR_TEXT,
				"<%s:%d> Error: 0016 rsslSocketGetBuffer() Cannot allocate memory of size %d for buffer.\n", __FILE__, __LINE__, size);

		/* return buffer */
		ipcReleaseDataBuffer(rsslSocketChannel, ipcBuf, error);

		return NULL;
	}

	if (fragmentation)
	{
		rsslBufImpl->fragmentationFlag = BUFFER_IMPL_FIRST_FRAG_HEADER;

		/* this is a chained message */
		/* that means I need to do some memory creation to hide the fact that this is actually
		   multiple buffers.  We also need to make sure that when writing, we copy the real
		   data into the ripcBuffer */
		rsslBufImpl->bufferInfo = ipcBuf;

		rsslBufImpl->buffer.data = (char*)_rsslMalloc(size + 7);

		if (rsslBufImpl->buffer.data == NULL)
		{
			_rsslSetError(error, &rsslChnlImpl->Channel, RSSL_RET_BUFFER_NO_BUFFERS, 0);
			snprintf(error->text, MAX_RSSL_ERROR_TEXT,
					"<%s:%d> Error: 0016 rsslSocketGetBuffer() Cannot allocate memory of size %d for buffer.\n", __FILE__, __LINE__, size);
			/* return ripc buffer */
			ipcReleaseDataBuffer(rsslSocketChannel, ipcBuf, error);

			return NULL;
		}
		/* set me as owner */
		rsslBufImpl->owner = 1;
	}
	else
	{
		_rsslBufferMap(rsslBufImpl, ipcBuf);
	}

	if (packedBuffer)
	{
		rsslBufImpl->packingOffset = 2;
		rsslBufImpl->buffer.data = rsslBufImpl->buffer.data + rsslBufImpl->packingOffset;
		rsslBufImpl->buffer.length = size - rsslBufImpl->packingOffset;
	}
	else
	{
		rsslBufImpl->packingOffset = 0;
		rsslBufImpl->buffer.length = size;
	}

	rsslBufImpl->totalLength = size;
	return rsslBufImpl;
}

/* rssl Socket ReleaseBuffer */
RSSL_RSSL_SOCKET_IMPL_FAST(RsslRet) rsslSocketReleaseBuffer(rsslChannelImpl *rsslChnlImpl, rsslBufferImpl *rsslBufImpl, RsslError *error)
{
	RsslSocketChannel* rsslSocketChannel = (RsslSocketChannel*)rsslChnlImpl->transportInfo;
	if (ipcReleaseDataBuffer(rsslSocketChannel, ((rtr_msgb_t*)rsslBufImpl->bufferInfo), error) < RSSL_RET_SUCCESS)
	{
		error->channel = &rsslChnlImpl->Channel;
		return RSSL_RET_FAILURE;
	}

	IPC_MUTEX_LOCK(rsslSocketChannel);

	/* The lock around the following statement is needed to prevent race condition
	   when a thread is releasing the buffer while another thread is writing data
	   from the buffer on the same channel */

	rsslBufImpl->bufferInfo = 0;

	IPC_MUTEX_UNLOCK(rsslSocketChannel);

	return RSSL_RET_SUCCESS;
}

/* rssl Socket PackBuffer */
RSSL_RSSL_SOCKET_IMPL_FAST(RsslBuffer*) rsslSocketPackBuffer(rsslChannelImpl *rsslChnlImpl, rsslBufferImpl *rsslBufImpl, RsslError *error)
{
	RsslUInt16 bufLength = rsslBufImpl->buffer.length;
	rtr_msgb_t	*ripcBuffer = rsslBufImpl->bufferInfo;

	rwfPut16((ripcBuffer->buffer + rsslBufImpl->packingOffset - 2), bufLength);	/* put the length of what we just wrote before what we just wrote */
	rsslBufImpl->packingOffset += rsslBufImpl->buffer.length;					/* move the packing offset to where the next msg would start */
	rsslBufImpl->packingOffset += 2;	/* make room for the next message length (This might go past the end of the buffer, but rsslWrite() will fix it). */

	if (rsslBufImpl->packingOffset < rsslBufImpl->totalLength)		/* make sure there is room for another message in the buffer */
	{
		rsslBufImpl->buffer.data = ripcBuffer->buffer + rsslBufImpl->packingOffset;
		rsslBufImpl->buffer.length = rsslBufImpl->totalLength - rsslBufImpl->packingOffset;
	}
	else	/* There isnt enough room left to store another message (and its length) */
	{
		rsslBufImpl->buffer.length = 0;		/* tell them there is no room left */
		rsslBufImpl->buffer.data = 0;		/* set this to zero. The alternative (which we do not like) is to point beyond the buffer */
	}
	return (&(rsslBufImpl->buffer));
}

/* rssl Socket Flush */
RSSL_RSSL_SOCKET_IMPL_FAST(RsslRet) rsslSocketFlush(rsslChannelImpl *rsslChnlImpl, RsslError *error)
{
	RsslRet		 retVal;
	RsslSocketChannel *rsslSocketChannel = (RsslSocketChannel*)rsslChnlImpl->transportInfo;

	if (IPC_NULL_PTR(rsslSocketChannel, "rsslSocketFlush", "rsslSocketChannel", error))
		return RSSL_RET_FAILURE;

	IPC_MUTEX_LOCK(rsslSocketChannel);
	retVal = ipcFlushSession(rsslSocketChannel, error);
	IPC_MUTEX_UNLOCK(rsslSocketChannel);

	if (retVal < RSSL_RET_SUCCESS)
	{
		/* if this fails, close socket */
		rsslChnlImpl->Channel.state = RSSL_CH_STATE_CLOSED;

		error->channel = &rsslChnlImpl->Channel;
	}

	return retVal;
}

/* rssl Socket Ping */
RSSL_RSSL_SOCKET_IMPL_FAST(RsslRet) rsslSocketPing(rsslChannelImpl *rsslChnlImpl, RsslError *error)
{
	RsslRet retVal;

	/* no buffer passed in */
	/* use ripcWrtHeader - should write and flush */
	retVal = ipcWrtHeader(  ((RsslSocketChannel*)rsslChnlImpl->transportInfo), error);
	if (retVal < RSSL_RET_SUCCESS)
	{
		error->channel = &rsslChnlImpl->Channel;
	}

	return retVal;
}

/* rssl Socket GetChannelInfo */
RSSL_RSSL_SOCKET_IMPL_FAST(RsslRet) rsslSocketGetChannelInfo(rsslChannelImpl *rsslChnlImpl, RsslChannelInfo *info, RsslError *error)
{
	RsslInt32		i = 0;
	RsslInt32 		size = 0;

	char*		componentVersion;
	RsslUInt32	componentVersionLength;

	RsslSocketChannel* rsslSocketChannel = (RsslSocketChannel*)rsslChnlImpl->transportInfo;

	IPC_MUTEX_LOCK(rsslSocketChannel);

	if (rsslSocketChannel->workState & RIPC_INT_SHTDOWN_PEND)
	{
		_rsslSetError(error, &rsslChnlImpl->Channel, RSSL_RET_FAILURE, errno);
		snprintf(error->text, MAX_RSSL_ERROR_TEXT,
			"<%s:%d> Error: 1003 rsslSocketGetChannelInfo() failed due to channel shutting down.\n",
			__FILE__, __LINE__);

		IPC_MUTEX_UNLOCK(rsslSocketChannel);
		return RSSL_RET_FAILURE;
	}
	else if (rsslSocketChannel->intState != RIPC_INT_ST_ACTIVE)
	{
		_rsslSetError(error, &rsslChnlImpl->Channel, RSSL_RET_FAILURE, errno);
		snprintf(error->text, MAX_RSSL_ERROR_TEXT,
				"<%s:%d> Error: 1003 rsslSocketGetChannelInfo() failed because the channel is not active.\n",
				__FILE__, __LINE__);

		IPC_MUTEX_UNLOCK(rsslSocketChannel);
		return RSSL_RET_FAILURE;
	}

	/* subtract 2 so maxFragmentSize is for packable messages */
	info->maxFragmentSize = rsslSocketChannel->maxUserMsgSize - 2;

	info->maxOutputBuffers = rsslSocketChannel->guarBufPool->maxPoolBufs + rsslSocketChannel->guarBufPool->bufpool.maxBufs;
	info->guaranteedOutputBuffers = rsslSocketChannel->guarBufPool->bufpool.maxBufs;
	info->numInputBuffers = (RsslUInt32)(rsslSocketChannel->inputBuffer->maxLength / rsslSocketChannel->maxMsgSize);
	info->compressionThreshold = rsslSocketChannel->lowerCompressionThreshold;
	info->compressionType = (RsslCompTypes)rsslSocketChannel->outCompression;
	info->encryptionProtocol = rsslSocketChannel->sslCurrentProtocol;

	/* until we own this memory, we have not gotten the info from the other side of the connection */
	if (rsslSocketChannel->outComponentVer)
	{
		componentVersion = rsslSocketChannel->outComponentVer;
		componentVersionLength = rsslSocketChannel->outComponentVerLen;
	}
	else
	{
		componentVersion = 0;
		componentVersionLength = 0;
	}

	while (i < RIPC_MAX_FLUSH_STRATEGY)
	{
		switch (rsslSocketChannel->flushStrategy[i])
		{
			case 0:
				info->priorityFlushStrategy[i] = 'H';
				break;
			case 1:
				info->priorityFlushStrategy[i] = 'M';
				break;
			case 2:
				info->priorityFlushStrategy[i] = 'L';
				break;
			default:
				info->priorityFlushStrategy[i] = '\0';
		}

		if (rsslSocketChannel->flushStrategy[i] == -1)
				break;

		i++;
	}

	IPC_MUTEX_UNLOCK(rsslSocketChannel);


	/* pass through connected component version info here */
	/* this is a dynamic array because of multicast - if memory is here, it means user already called
	   get channel info for socket transport so just reuse it.  it will be cleaned up when connection is closed */
	if (!rsslChnlImpl->componentInfo)
	{
		rsslChnlImpl->componentInfo = _rsslMalloc(sizeof(void*));
		rsslChnlImpl->componentInfo[0] = _rsslMalloc(sizeof(RsslComponentInfo));
	}
	rsslChnlImpl->componentInfo[0]->componentVersion.length = componentVersionLength;
	rsslChnlImpl->componentInfo[0]->componentVersion.data = componentVersion;

	if (componentVersionLength)
		info->componentInfoCount = 1;
	else
		info->componentInfoCount = 0;
	info->componentInfo = rsslChnlImpl->componentInfo;

	if (rsslSocketGetSockOpts(rsslSocketChannel, RIPC_SYSTEM_READ_BUFFERS, &size, error) < 0)
	{
		error->channel = &rsslChnlImpl->Channel;

		return RSSL_RET_FAILURE;
	}

	info->tcpRecvBufSize = size;
	info->sysRecvBufSize = size;

	size = 0;
	if (rsslSocketGetSockOpts(rsslSocketChannel, RIPC_SYSTEM_WRITE_BUFFERS, &size, error) < 0)
	{
		error->channel = &rsslChnlImpl->Channel;

		return RSSL_RET_FAILURE;
	}

	info->tcpSendBufSize = size;
	info->sysSendBufSize = size;

	if (rsslChnlImpl->rsslFlags & SERVER_TO_CLIENT)
	{
		info->serverToClientPings = RSSL_TRUE;
	}
	else
		info->serverToClientPings = RSSL_FALSE;

	if (rsslChnlImpl->rsslFlags & CLIENT_TO_SERVER)
	{
		info->clientToServerPings = RSSL_TRUE;
	}
	else
		info->clientToServerPings = RSSL_FALSE;

	info->pingTimeout = rsslChnlImpl->Channel.pingTimeout;

	/* clear other stats types (multicast, shmem, etc) */
	info->multicastStats.mcastRcvd = 0;
	info->multicastStats.mcastSent = 0;
	info->multicastStats.retransPktsRcvd = 0;
	info->multicastStats.retransPktsSent = 0;
	info->multicastStats.retransReqRcvd = 0;
	info->multicastStats.retransReqSent = 0;
	info->multicastStats.unicastRcvd = 0;
	info->multicastStats.unicastSent = 0;
	info->multicastStats.gapsDetected = 0;

	return RSSL_RET_SUCCESS;
}

/* rssl Socket GetChannelStats */
RSSL_RSSL_SOCKET_IMPL_FAST(RsslRet) rsslSocketGetChannelStats(rsslChannelImpl *rsslChnlImpl, RsslChannelStats *stats, RsslError *error)
{
	RsslInt32		i = 0;
	RsslInt32 		size = 0;

#ifdef Linux
	socklen_t len = 0;
	struct tcp_info value;
#else
	TCP_ESTATS_PATH_ROD_v0 value;
	ULONG err;
#endif

	RsslSocketChannel* rsslSocketChannel = (RsslSocketChannel*)rsslChnlImpl->transportInfo;

	memset((void*)stats, 0x00, sizeof(RsslChannelStats));

	IPC_MUTEX_LOCK(rsslSocketChannel);

	if (rsslSocketChannel->workState & RIPC_INT_SHTDOWN_PEND)
	{
		_rsslSetError(error, &rsslChnlImpl->Channel, RSSL_RET_FAILURE, errno);
		snprintf(error->text, MAX_RSSL_ERROR_TEXT,
			"<%s:%d> Error: 1003 rsslSocketGetChannelStats() failed due to channel shutting down.\n",
			__FILE__, __LINE__);

		IPC_MUTEX_UNLOCK(rsslSocketChannel);
		return RSSL_RET_FAILURE;
	}
	else if (rsslSocketChannel->intState != RIPC_INT_ST_ACTIVE)
	{
		_rsslSetError(error, &rsslChnlImpl->Channel, RSSL_RET_FAILURE, errno);
		snprintf(error->text, MAX_RSSL_ERROR_TEXT,
			"<%s:%d> Error: 1003 rsslSocketGetChannelStats() failed because the channel is not active.\n",
			__FILE__, __LINE__);

		IPC_MUTEX_UNLOCK(rsslSocketChannel);
		return RSSL_RET_FAILURE;
	}

	if (rsslSocketChannel->usingWinInet == 1 || (rsslSocketChannel->httpHeaders == 1 && rsslSocketChannel->isJavaTunnel == 0))
	{
		_rsslSetError(error, &rsslChnlImpl->Channel, RSSL_RET_FAILURE, 0);
		snprintf(error->text, MAX_RSSL_ERROR_TEXT,
			"<%s:%d> Error: 1006 rsslSocketGetChannelStats() does not work with WinInet connections, or non-Java HTTP server connections.\n",
			__FILE__, __LINE__);

		IPC_MUTEX_UNLOCK(rsslSocketChannel);
		return RSSL_RET_FAILURE;
	}

#ifdef Linux
	len = sizeof(struct tcp_info);
	if (getsockopt(rsslSocketChannel->stream, IPPROTO_TCP, TCP_INFO, (char*)&value, &len) != 0)
	{
		_rsslSetError(error, NULL, RSSL_RET_FAILURE, errno);
		snprintf(error->text, MAX_RSSL_ERROR_TEXT, "<%s:%d> rsslSocketGetChannelStats() Error: 1002 getsockopt() IPPROTO_TCP failed.  System errno: (%d)", __FILE__, __LINE__, errno);
		IPC_MUTEX_UNLOCK(rsslSocketChannel);
		return RSSL_RET_FAILURE;
	}

	stats->tcpStats.tcpRetransmitCount = value.tcpi_total_retrans;
#else

	if (rsslSocketChannel->socketRowSet == RSSL_FALSE)
	{
		IPC_MUTEX_UNLOCK(rsslSocketChannel);
		return RSSL_RET_SUCCESS;
	}

	memset((void*)&value, 0, sizeof(TCP_ESTATS_PATH_ROD_v0));
	err = GetPerTcpConnectionEStats(&(rsslSocketChannel->socketRow), TcpConnectionEstatsPath, NULL, 0, 0, NULL, 0, 0, (PUCHAR)&value, 0, sizeof(TCP_ESTATS_PATH_ROD_v0));
	if (err != NO_ERROR)
	{
		_rsslSetError(error, NULL, RSSL_RET_FAILURE, errno);
		snprintf(error->text, MAX_RSSL_ERROR_TEXT, "<%s:%d> rsslSocketGetChannelStats() Error: 1002 GetPerTcpConnectionEStats() failed.  Error from function (%d)", __FILE__, __LINE__, err);
		IPC_MUTEX_UNLOCK(rsslSocketChannel);
		return RSSL_RET_FAILURE;
	}
	stats->tcpStats.flags |= RSSL_TCP_STATS_RETRANSMIT;
	stats->tcpStats.tcpRetransmitCount = value.PktsRetrans;
#endif 

	IPC_MUTEX_UNLOCK(rsslSocketChannel);
	return RSSL_RET_SUCCESS;
}

/* rssl Socket GetServerInfo */
RSSL_RSSL_SOCKET_IMPL_FAST(RsslRet) rsslSocketGetSrvrInfo(rsslServerImpl *rsslSrvrImpl, RsslServerInfo *info, RsslError *error)
{
	rtr_dfltcbufferpool_t	*rtr_dfltcpool;

	RsslServerSocketChannel* rsslServerSocketChannel = (RsslServerSocketChannel*)rsslSrvrImpl->transportInfo;

	if (rsslServerSocketChannel == 0)
	{
		_rsslSetError(error, (RsslChannel*)(&rsslSrvrImpl->Server), RSSL_RET_FAILURE, 0);
		snprintf(error->text, MAX_RSSL_ERROR_TEXT,
			"<%s:%d> Error: 1004 rsslSocketGetSrvrInfo() failed, server is NULL.\n", __FILE__, __LINE__);
		return RSSL_RET_FAILURE;
	}

	IPC_MUTEX_LOCK(rsslServerSocketChannel);

	if (rsslServerSocketChannel->sharedBufPool)
	{
		rtr_dfltcpool = (rtr_dfltcbufferpool_t*)rsslServerSocketChannel->sharedBufPool->internal;

		info->currentBufferUsage = rtr_dfltcpool->numRegBufsUsed;
		info->peakBufferUsage = rtr_dfltcpool->peakNumBufsUsed;
	}
	else
	{
		_rsslSetError(error, (RsslChannel*)(&rsslSrvrImpl->Server), RSSL_RET_FAILURE, 0);
		snprintf(error->text, MAX_RSSL_ERROR_TEXT,
				"<%s:%d> Error: 1004 rsslSocketGetSrvrInfo() failed, no shared buffer pool.\n", __FILE__, __LINE__);

		IPC_MUTEX_UNLOCK(rsslServerSocketChannel);
		return RSSL_RET_FAILURE;
	}

	IPC_MUTEX_UNLOCK(rsslServerSocketChannel);

	return RSSL_RET_SUCCESS;
}

/* rssl Socket Buffer Usage */
RSSL_RSSL_SOCKET_IMPL_FAST(RsslInt32) rsslSocketBufferUsage(rsslChannelImpl *rsslChnlImpl, RsslError *error)
{
	RsslInt32 retVal = RSSL_RET_SUCCESS;

	RsslSocketChannel* rsslSocketChannel = 0;

	rsslSocketChannel = (RsslSocketChannel*)rsslChnlImpl->transportInfo;

	IPC_MUTEX_LOCK(rsslSocketChannel);

	if (rsslSocketChannel->workState & RIPC_INT_SHTDOWN_PEND)
	{
		_rsslSetError(error, (RsslChannel*)(&rsslChnlImpl->Channel), RSSL_RET_FAILURE, 0);
		snprintf(error->text, MAX_RSSL_ERROR_TEXT,
				"<%s:%d> Error: 1003 rsslSocketBufferUsage() failed due to channel shutting down.\n", __FILE__, __LINE__);

		IPC_MUTEX_UNLOCK(rsslSocketChannel);

		return RSSL_RET_FAILURE;
	}

	/* Return the number of guaranteed buffers used + pool buffers used */
	retVal = rsslSocketChannel->guarBufPool->numRegBufsUsed + rsslSocketChannel->guarBufPool->numPoolBufs;

	IPC_MUTEX_UNLOCK(rsslSocketChannel);

	return retVal;
}

/* rssl Server Socket Buffer usage */
RSSL_RSSL_SOCKET_IMPL_FAST(RsslInt32) rsslSocketSrvrBufferUsage(rsslServerImpl *rsslSrvrImpl, RsslError *error)
{
	RsslInt32 retVal = RSSL_RET_SUCCESS;
	rtr_dfltcbufferpool_t	*rtr_dfltcpool;

	RsslServerSocketChannel* rsslServerSocketChannel = (RsslServerSocketChannel*)rsslSrvrImpl->transportInfo;

	if (rsslServerSocketChannel == 0)
	{
		_rsslSetError(error, (RsslChannel*)(&rsslSrvrImpl->Server), RSSL_RET_FAILURE, 0);
		snprintf(error->text, MAX_RSSL_ERROR_TEXT,
			"<%s:%d> Error: 1004 rsslSocketSrvrBufferUsage() failed, server is NULL.\n", __FILE__, __LINE__);

		return RSSL_RET_FAILURE;
	}

	IPC_MUTEX_LOCK(rsslServerSocketChannel);

	if (rsslServerSocketChannel->sharedBufPool)
	{
		rtr_dfltcpool = (rtr_dfltcbufferpool_t*)rsslServerSocketChannel->sharedBufPool->internal;
		retVal = rtr_dfltcpool->numRegBufsUsed;
	}
	else
	{
		_rsslSetError(error, (RsslChannel*)(&rsslSrvrImpl->Server), RSSL_RET_FAILURE, 0);
		snprintf(error->text, MAX_RSSL_ERROR_TEXT,
				"<%s:%d> Error: 1004 rsslSocketSrvrBufferUsage() failed, server is NULL.\n", __FILE__, __LINE__);

		IPC_MUTEX_UNLOCK(rsslServerSocketChannel);
		return RSSL_RET_FAILURE;
	}

	IPC_MUTEX_UNLOCK(rsslServerSocketChannel);

	return retVal;
}

RSSL_RSSL_SOCKET_IMPL_FAST(RsslRet) rsslSocketSrvrIoctl(rsslServerImpl *rsslSrvrImpl, RsslIoctlCodes code, void *value, RsslError *error)
{
	RsslInt32			iValue = *(RsslInt32*)value;

	RsslServerSocketChannel* rsslServerSocketChannel = (RsslServerSocketChannel*)rsslSrvrImpl->transportInfo;

	/* In the multithreaded case a race condition may happen */
	if (rsslServerSocketChannel == 0)
	{
		_rsslSetError(error, (RsslChannel*)(&rsslSrvrImpl->Server), RSSL_RET_FAILURE, 0);
		snprintf(error->text, MAX_RSSL_ERROR_TEXT,
				"<%s:%d> Error: 1004 rsslSocketSrvrIoctl() failed, server is NULL.\n", __FILE__, __LINE__);

		return RSSL_RET_FAILURE;
	}

	IPC_MUTEX_LOCK(rsslServerSocketChannel);

	/* component info is done up above in rsslImpl */
	switch(code)
	{
		case RSSL_SERVER_NUM_POOL_BUFFERS:
		{
			if (iValue < 1)
			{
				/* error */
				_rsslSetError(error, (RsslChannel*)(&rsslSrvrImpl->Server), RSSL_RET_FAILURE, 0);
				snprintf(error->text, MAX_RSSL_ERROR_TEXT,
						"<%s:%d> Error: 0017 rsslSocketServerIoctl() Invalid number of pool buffers specified (%d).\n", __FILE__, __LINE__, iValue);

				IPC_MUTEX_UNLOCK(rsslServerSocketChannel);
				return RSSL_RET_FAILURE;
			}

			if (rtr_dfltcSetMaxBufs(rsslServerSocketChannel->sharedBufPool, iValue) < 0)
			{
				_rsslSetError(error, (RsslChannel*)(&rsslSrvrImpl->Server), RSSL_RET_FAILURE, 0);
				snprintf(error->text, MAX_RSSL_ERROR_TEXT,
						"<%s:%d> Error: 1001 rsslSocketServerIoctl() failed, could not change pool size from <%d> to <%d>\n",
						__FILE__, __LINE__, rsslServerSocketChannel->sharedBufPool->maxBufs, iValue);

				IPC_MUTEX_UNLOCK(rsslServerSocketChannel);
				return RSSL_RET_FAILURE;
			}
		}
		break;
		case RSSL_SERVER_PEAK_BUF_RESET:
		{
			if (rtr_dfltcResetPeakNumBufs(rsslServerSocketChannel->sharedBufPool) < 0)
			{
				_rsslSetError(error, (RsslChannel*)(&rsslSrvrImpl->Server), RSSL_RET_FAILURE, 0);
				snprintf(error->text, MAX_RSSL_ERROR_TEXT,
						"<%s:%d> Error: 1002 rsslSocketServerIoctl() failed, could not reset peak number of buffers used.\n", __FILE__, __LINE__);

				IPC_MUTEX_UNLOCK(rsslServerSocketChannel);
				return RSSL_RET_FAILURE;
			}
		}
		break;
		default:
		{
			/* error */
			_rsslSetError(error, (RsslChannel*)(&rsslSrvrImpl->Server), RSSL_RET_FAILURE, 0);
			snprintf(error->text, MAX_RSSL_ERROR_TEXT,
				"<%s:%d> Error: 0017 rsslSocketServerIoctl() Invalid RSSL Server IOCtl code (%d).\n", __FILE__, __LINE__, code);

			IPC_MUTEX_UNLOCK(rsslServerSocketChannel);
			return RSSL_RET_FAILURE;
		}
	}

	IPC_MUTEX_UNLOCK(rsslServerSocketChannel);

	return RSSL_RET_SUCCESS;
}

RSSL_RSSL_SOCKET_FAST(RsslRet) rsslSocketIoctl(rsslChannelImpl *rsslChnlImpl, RsslIoctlCodes code, void *value, RsslError *error)
{
	RsslInt32	iValue = *(RsslInt32*)value;

	RsslInt32 	guarOutputMsgs;

	ripcSocketOption	opts;
	char 	*tempFlushStrat = 0;
	RsslInt32		HMpresent = 0;
	RsslInt32		flushStratIndex = 0;
	RsslInt32		len = 0;
	RsslInt32		i = 0;

	RsslSocketChannel*	rsslSocketChannel = (RsslSocketChannel*)rsslChnlImpl->transportInfo;

	IPC_MUTEX_LOCK(rsslSocketChannel);

	if (rsslSocketChannel->workState & RIPC_INT_SHTDOWN_PEND)
	{
		_rsslSetError(error, &rsslChnlImpl->Channel, RSSL_RET_FAILURE, 0);
		snprintf(error->text, MAX_RSSL_ERROR_TEXT,
				"<%s:%d> Error: 1003 rsslSocketIoctl() failed due to channel shutting down.\n",
				__FILE__, __LINE__);

		IPC_MUTEX_UNLOCK(rsslSocketChannel);
		return RSSL_RET_FAILURE;
	}

	switch((int)code)
	{
	case RSSL_MAX_NUM_BUFFERS:
		if (rsslSocketChannel->intState != RIPC_INT_ST_ACTIVE)
		{
			_rsslSetError(error, &rsslChnlImpl->Channel, RSSL_RET_FAILURE, errno);
			snprintf(error->text, MAX_RSSL_ERROR_TEXT,
					"<%s:%d> Error: 1003 rsslSocketIoctl() failed because the channel is not active.\n",
					__FILE__, __LINE__);

			IPC_MUTEX_UNLOCK(rsslSocketChannel);
			return RSSL_RET_FAILURE;
		}

		guarOutputMsgs = rsslSocketChannel->guarBufPool->bufpool.maxBufs;

		iValue = iValue - guarOutputMsgs;
		if (iValue < 0)
			iValue = 0;

		if (rtr_dfltcSetMaxSharedBufs(&(rsslSocketChannel->guarBufPool->bufpool), iValue) < 0)
		{
			_rsslSetError(error, (RsslChannel*)(&rsslChnlImpl->Channel), RSSL_RET_FAILURE, 0);
			snprintf(error->text, MAX_RSSL_ERROR_TEXT,
					"<%s:%d> Error: 1001 rsslSocketIoctl() failed, could not change the number of shared output buffers from <%d> to <%d>\n.",
					__FILE__, __LINE__, rsslSocketChannel->guarBufPool->maxPoolBufs, iValue);

			IPC_MUTEX_UNLOCK(rsslSocketChannel);
			return RSSL_RET_FAILURE;
		}
		break;

	case RSSL_NUM_GUARANTEED_BUFFERS:
		if(iValue < 1)
		{
			_rsslSetError(error, (RsslChannel*)(&rsslChnlImpl->Channel), RSSL_RET_FAILURE, 0);
			snprintf(error->text, MAX_RSSL_ERROR_TEXT,
					"<%s:%d> Error: 1004 rsslSocketIoctl() failed, invalid value <%d>, value for guaranteed output buffers should be at least 1\n",
					__FILE__, __LINE__, iValue);

			IPC_MUTEX_UNLOCK(rsslSocketChannel);
			return RSSL_RET_FAILURE;
		}

		if (rtr_dfltcSetMaxBufs(&(rsslSocketChannel->guarBufPool->bufpool), iValue) < 0)
		{
			_rsslSetError(error, (RsslChannel*)(&rsslChnlImpl->Channel), RSSL_RET_FAILURE, errno);
			snprintf(error->text, MAX_RSSL_ERROR_TEXT,
					"<%s:%d> Error: 1001 rsslSocketIoctl() failed, could not change the number of output buffers from <%d> to <%d>.\n",
					__FILE__, __LINE__, rsslSocketChannel->guarBufPool->bufpool.maxBufs, iValue);

			IPC_MUTEX_UNLOCK(rsslSocketChannel);
			return RSSL_RET_FAILURE;
		}
		break;

	case RSSL_HIGH_WATER_MARK:
		if(iValue >= 0)
			rsslSocketChannel->high_water_mark = iValue;
		else
		{
			_rsslSetError(error, (RsslChannel*)(&rsslChnlImpl->Channel), RSSL_RET_FAILURE, 0);
			snprintf(error->text, MAX_RSSL_ERROR_TEXT,
					"<%s:%d> Error: 1004 rsslSocketIoctl() failed, could not set the high water mark to <%d>, must be a postive number.\n",
					__FILE__, __LINE__, iValue);

			IPC_MUTEX_UNLOCK(rsslSocketChannel);
			return RSSL_RET_FAILURE;
		}
		break;

	case RSSL_SYSTEM_READ_BUFFERS:
		opts.code = RIPC_SOPT_RD_BUF_SIZE;
		opts.options.buffer_size = iValue;

		if (((*(rsslSocketChannel->transportFuncs->setSockOpts))(rsslSocketChannel->stream, &opts,
																rsslSocketChannel->transportInfo)) < 0)
		{
			_rsslSetError(error, (RsslChannel*)(&rsslChnlImpl->Channel), RSSL_RET_FAILURE, errno);
			snprintf(error->text, MAX_RSSL_ERROR_TEXT,
					"<%s:%d> Error: 1002 Could not set number of system read buffers to (%d). System errno: (%d)\n",
					__FILE__, __LINE__, iValue, errno);

			IPC_MUTEX_UNLOCK(rsslSocketChannel);
			return RSSL_RET_FAILURE;
		}
		break;

	case RSSL_SYSTEM_WRITE_BUFFERS:
		opts.code = RIPC_SOPT_WRT_BUF_SIZE;
		opts.options.buffer_size = iValue;

		if (((*(rsslSocketChannel->transportFuncs->setSockOpts))(rsslSocketChannel->stream, &opts,
																rsslSocketChannel->transportInfo)) < 0)
		{
			_rsslSetError(error, (RsslChannel*)(&rsslChnlImpl->Channel), RSSL_RET_FAILURE, errno);
			snprintf(error->text, MAX_RSSL_ERROR_TEXT,
					"<%s:%d> Error: 1002 Could not set number of system write buffers to (%d). System errno: (%d)\n",
					__FILE__, __LINE__, iValue, errno);

			IPC_MUTEX_UNLOCK(rsslSocketChannel);
			return RSSL_RET_FAILURE;
		}
		break;

	case RSSL_DEBUG_FLAGS:
		/* reset debug flags - if user still wants these on, they should continue passing them in */
		rsslChnlImpl->debugFlags = 0;
		/* set rssl debug options and unset them in the opts we pass to ripc */
		if ((iValue & RSSL_DEBUG_RSSL_DUMP_IN) && (rsslSocketDumpInFunc))
		{
			iValue ^= RSSL_DEBUG_RSSL_DUMP_IN;
			rsslChnlImpl->debugFlags |= RSSL_DEBUG_RSSL_DUMP_IN;
		}
		if ((iValue & RSSL_DEBUG_RSSL_DUMP_OUT) && (rsslSocketDumpOutFunc))
		{
			iValue ^= RSSL_DEBUG_RSSL_DUMP_OUT;
			rsslChnlImpl->debugFlags |= RSSL_DEBUG_RSSL_DUMP_OUT;
		}

		rsslSocketChannel->dbgFlags = iValue;
		if ((rsslSocketChannel->dbgFlags & RSSL_DEBUG_IPC_DUMP_IN) && (ripcDumpInFunc == 0))
			rsslSocketChannel->dbgFlags &= !RSSL_DEBUG_IPC_DUMP_IN;
		if ((rsslSocketChannel->dbgFlags & RSSL_DEBUG_IPC_DUMP_OUT) && (ripcDumpOutFunc == 0))
			rsslSocketChannel->dbgFlags &= !RSSL_DEBUG_IPC_DUMP_OUT;
		if ((rsslSocketChannel->dbgFlags & RSSL_DEBUG_IPC_DUMP_INIT) && ((ripcDumpInFunc == 0) || (ripcDumpOutFunc == 0)))
			rsslSocketChannel->dbgFlags &= !RSSL_DEBUG_IPC_DUMP_INIT;
		break;

	case RSSL_COMPRESSION_THRESHOLD:
	{
		RsslInt32 lowerThreshold;

	    if (rsslSocketChannel->outCompression == RSSL_COMP_NONE)
		  break;

		lowerThreshold = (rsslSocketChannel->outCompression == RSSL_COMP_ZLIB ?
									RSSL_COMP_DFLT_THRESHOLD_ZLIB : RSSL_COMP_DFLT_THRESHOLD_LZ4);
		if(iValue >= lowerThreshold)
			rsslSocketChannel->lowerCompressionThreshold = iValue;
		else
		{
			_rsslSetError(error, (RsslChannel*)(&rsslChnlImpl->Channel), RSSL_RET_FAILURE, 0);
			snprintf(error->text, MAX_RSSL_ERROR_TEXT,
					"<%s,%d> Error: 1004 rsslSocketIoctl() failed, could not set the compression threshold mark to <%d>, must be equal or greater than %d bytes\n",
					__FILE__, __LINE__, iValue, lowerThreshold);

			IPC_MUTEX_UNLOCK(rsslSocketChannel);
			return RSSL_RET_FAILURE;
		}
	}
		break;

	case RSSL_PRIORITY_FLUSH_ORDER:
		/* cast value to char*, go through one by one to verify that they are all H, M, L
		   Null terminate - Make sure at least H and M are represented.  Then store in sess->flushStrategy[];
		*/
		/* if we allow more than 3, we want to change this to just use numbers starting from 0 */

		tempFlushStrat = (char*)value;

		len = (RsslInt32)strlen(tempFlushStrat);

		/* we only use up to the ripc max flush strategy amount */
		if (len > RIPC_MAX_FLUSH_STRATEGY)
			len = RIPC_MAX_FLUSH_STRATEGY;

		for (i = 0; i < len;  i++)
		{
			/* this checks if it has H and M */
			/* if we have both, we can break this loop */
			if (HMpresent == 3)
				break;

			if ((tempFlushStrat[i]) == 'H')
			{
				if (HMpresent == 0)
					HMpresent = 1;
				else if (HMpresent == 2)
					HMpresent = 3;
			}

			if ((tempFlushStrat[i]) == 'M')
			{
				if (HMpresent == 0)
					HMpresent = 2;
				else if (HMpresent == 1)
					HMpresent = 3;
			}
		}

		/* Now if H and M are present, lets transfer this to our local array */
		if (HMpresent == 3)
		{
			/* H = 0, M = 1, L = 2.  If we ever add variable priorities, should just have users pass in numbers */
			for (i = 0; i < len; i++)
			{
				if (((tempFlushStrat[i]) == 'H') ||
					((tempFlushStrat[i]) == 'M') ||
					((tempFlushStrat[i]) == 'L'))
				{
					/* valid entry */
					if (tempFlushStrat[i] == 'H')
						rsslSocketChannel->flushStrategy[flushStratIndex] = 0;
					if (tempFlushStrat[i] == 'M')
						rsslSocketChannel->flushStrategy[flushStratIndex] = 1;
					if (tempFlushStrat[i] == 'L')
						rsslSocketChannel->flushStrategy[flushStratIndex] = 2;

					flushStratIndex++;
				}

				/* otherwise its not a valid letter */
			}

			/* now null terminate */
			rsslSocketChannel->flushStrategy[flushStratIndex] = -1;
			flushStratIndex++;

			/* set current flush strategy index back to start */
			rsslSocketChannel->currentOutList = 0;
		}
		else
		{
			/* generate an error here and return failure */
			_rsslSetError(error, (RsslChannel*)(&rsslChnlImpl->Channel), RSSL_RET_FAILURE, 0);
			snprintf(error->text, MAX_RSSL_ERROR_TEXT,
					"<%s:%d> Error: 1004 rsslSocketIoctl() failed, flush strategy <%s> minimally needs to contain both 'H' and 'M'\n",
					__FILE__, __LINE__, tempFlushStrat);

			IPC_MUTEX_UNLOCK(rsslSocketChannel);
			return RSSL_RET_FAILURE;
		}
		break;

	case RSSL_IGNORE_CERT_REVOCATION:
		if (rsslSocketChannel->transportFuncs->sessIoctl)
			return (*(rsslSocketChannel->transportFuncs->sessIoctl))(rsslSocketChannel->transportInfo, code, iValue, error);
		else
		{
			_rsslSetError(error, (RsslChannel*)(&rsslChnlImpl->Channel), RSSL_RET_FAILURE, 0);
			snprintf(error->text, MAX_RSSL_ERROR_TEXT,
					"<%s:%d> Error: 1004 rsslSocketIoctl() failed, Certificate revocation applies only to Windows platform tunneling connections.\n",
					__FILE__, __LINE__);

			IPC_MUTEX_UNLOCK(rsslSocketChannel);
			return RSSL_RET_FAILURE;
		}
		break;

	default:
		_rsslSetError(error, (RsslChannel*)(&rsslChnlImpl->Channel), RSSL_RET_FAILURE, 0);
		snprintf(error->text, MAX_RSSL_ERROR_TEXT,
				"<%s:%d> Error: 0017 rsslSocketIoctl() failed, invalid IOCtl code <%d>\n",
				__FILE__, __LINE__, code);

		IPC_MUTEX_UNLOCK(rsslSocketChannel);
		return RSSL_RET_FAILURE;
		break;
	}

	IPC_MUTEX_UNLOCK(rsslSocketChannel);

	return RSSL_RET_SUCCESS;
}

#if defined(LINUX)
RsslBool rssl_ignore_sig_handler(int sig)
{
	struct sigaction	nact;

	nact.sa_handler = SIG_IGN;
	sigemptyset(&nact.sa_mask);

	nact.sa_flags = 0;

	return((sigaction(sig, &nact, (struct sigaction *)0) == 0) ?
		RSSL_TRUE : RSSL_FALSE);
}
#endif

/*********************************************
*	The following defines the functions needed
*	for setting up and cleaning up the ripc.
*********************************************/

RsslInt32 ipcInitialize(RsslInt32 numServers, RsslInt32 numClients, RsslInitializeExOpts *initOpts, RsslError *error)
{
	RsslInt32		i;
	RsslRet			ret;
	char			eText[128];
	RsslSocketChannel* rsslSocketChannel;
	RsslServerSocketChannel* rsslServerSocketChannel;
	RsslMutex *poolMutex = 0;

	if (multiThread)
	{
		if (!gblmutexinit)
		{
			RTR_ATOMIC_SET(gblmutexinit,1);
			RSSL_MUTEX_INIT_ESDK(&ripcMutex);
			poolMutex = &ripcMutex;
		}

		(void) RSSL_MUTEX_LOCK(&ripcMutex);
		_DEBUG_MUTEX_TRACE("RSSL_MUTEX_LOCK", NULL, &ripcMutex)
	}

	if (!initialized)
	{
		rsslInitQueue(&freeServerSocketChannelList);
		rsslInitQueue(&freeSocketChannelList);
		rsslInitQueue(&activeSocketChannelList);

		ripc10Ver.connVersion = _conn_version_10;
		ripc10Ver.ipcVersion = RIPC_VERSION_10;
		ripc10Ver.dataHeaderLen = IPC_100_DATA_HEADER_SIZE;
		ripc10Ver.footerLen = 0;
		ripc10Ver.firstFragHdrLen = 5;
		ripc10Ver.subsequentFragHdrLen = 1;

		ripc11Ver.connVersion = _conn_version_11;
		ripc11Ver.ipcVersion = RIPC_VERSION_11;
		ripc11Ver.dataHeaderLen = IPC_100_DATA_HEADER_SIZE;
		ripc11Ver.footerLen = 0;
		ripc11Ver.firstFragHdrLen = 5;
		ripc11Ver.subsequentFragHdrLen = 1;

		/* set this up for tunneling */
		ripc11WinInetVer.connVersion = _conn_version_11;
		ripc11WinInetVer.ipcVersion = RIPC_VERSION_11;
		ripc11WinInetVer.dataHeaderLen = IPC_100_DATA_HEADER_SIZE + 6;
		ripc11WinInetVer.footerLen = 2;
		ripc11WinInetVer.firstFragHdrLen = 5;
		ripc11WinInetVer.subsequentFragHdrLen = 1;

		ripc12Ver.connVersion = _conn_version_12;
		ripc12Ver.ipcVersion = RIPC_VERSION_12;
		ripc12Ver.dataHeaderLen = IPC_100_DATA_HEADER_SIZE;
		ripc12Ver.footerLen = 0;
		ripc12Ver.firstFragHdrLen = 5;
		ripc12Ver.subsequentFragHdrLen = 1;

		/* set this up for tunneling */
		ripc12WinInetVer.connVersion = _conn_version_12;
		ripc12WinInetVer.ipcVersion = RIPC_VERSION_12;
		ripc12WinInetVer.dataHeaderLen = IPC_100_DATA_HEADER_SIZE + 6;
		ripc12WinInetVer.footerLen = 2;
		ripc12WinInetVer.firstFragHdrLen = 5;
		ripc12WinInetVer.subsequentFragHdrLen = 1;

		ripc13Ver.connVersion = _conn_version_13;
		ripc13Ver.ipcVersion = RIPC_VERSION_13;
		ripc13Ver.dataHeaderLen = IPC_100_DATA_HEADER_SIZE;
		ripc13Ver.footerLen = 0;
		ripc13Ver.firstFragHdrLen = 6;
		ripc13Ver.subsequentFragHdrLen = 2;

		/* set this up for tunneling */
		ripc13WinInetVer.connVersion = _conn_version_13;
		ripc13WinInetVer.ipcVersion = RIPC_VERSION_13;
		ripc13WinInetVer.dataHeaderLen = IPC_100_DATA_HEADER_SIZE + 6;
		ripc13WinInetVer.footerLen = 2;
		ripc13WinInetVer.firstFragHdrLen = 6;
		ripc13WinInetVer.subsequentFragHdrLen = 2;

		ripc14Ver.connVersion = _conn_version_14;
		ripc14Ver.ipcVersion = RIPC_VERSION_14;
		ripc14Ver.dataHeaderLen = IPC_100_DATA_HEADER_SIZE;
		ripc14Ver.footerLen = 0;
		ripc14Ver.firstFragHdrLen = 6;
		ripc14Ver.subsequentFragHdrLen = 2;

		/* set this up for tunneling */
		ripc14WinInetVer.connVersion = _conn_version_14;
		ripc14WinInetVer.ipcVersion = RIPC_VERSION_14;
		ripc14WinInetVer.dataHeaderLen = IPC_100_DATA_HEADER_SIZE + 6;
		ripc14WinInetVer.footerLen = 2;
		ripc14WinInetVer.firstFragHdrLen = 6;
		ripc14WinInetVer.subsequentFragHdrLen = 2;

		transOpts.rsslLocking = 0;
		transOpts.jitOpts.libsslName = 0;
		transOpts.jitOpts.libcryptoName = 0;
		transOpts.initConfig = 0;
		transOpts.initConfigSize = 0;
		transOpts.initCurlDebug = initOpts->initCurlDebug;

		if (initOpts->initConfig && initOpts->initConfigSize)
		{
			transOpts.initConfigSize = initOpts->initConfigSize;
			transOpts.initConfig = malloc(transOpts.initConfigSize);
			if (transOpts.initConfig == 0)
			{
				error->rsslErrorId = RSSL_RET_FAILURE;
				snprintf(error->text, MAX_RSSL_ERROR_TEXT,
					"<%s:%d> Error: 1001 malloc failed.\n", __FILE__, __LINE__);

				ret = RSSL_RET_FAILURE;
				goto ripcinitend;
			}
			memcpy(transOpts.initConfig, initOpts->initConfig, transOpts.initConfigSize);
		}

		for (i = 0; i<RIPC_MAX_PROTOCOL_TYPE; i++)
		{
			protHdrFuncs[i].readTransportConnMsg = 0;
			protHdrFuncs[i].readPrependTransportHdr = 0;
			protHdrFuncs[i].readTransportMsg = 0;
			protHdrFuncs[i].prependTransportHdr = 0;
			protHdrFuncs[i].getPoolBuffer = 0;
			protHdrFuncs[i].additionalTransportHdrLength = 0;
		}
		ipcSetProtFuncs();

		SSLTransFuncs.newSSLServer = 0;
		SSLTransFuncs.freeSSLServer = 0;

		for (i = 0; i<RIPC_MAX_TRANSPORTS; i++)
		{
			transFuncs[i].bindSrvr = 0;
			transFuncs[i].newSrvrConnection = 0;
			transFuncs[i].connectSocket = 0;
			transFuncs[i].newClientConnection = 0;
			transFuncs[i].initializeTransport = 0;
			transFuncs[i].shutdownTransport = 0;
			transFuncs[i].readTransport = 0;
			transFuncs[i].writeTransport = 0;
			transFuncs[i].writeVTransport = 0;
			transFuncs[i].reconnectClient = 0;
			transFuncs[i].acceptSocket = 0;
			transFuncs[i].shutdownSrvrError = 0;

			transFuncs[i].sessIoctl = 0;

			transFuncs[i].getSockName = 0;
			transFuncs[i].setSockOpts = 0;
			transFuncs[i].getSockOpts = 0;
			transFuncs[i].connected = 0;
			transFuncs[i].shutdownServer = 0;
			transFuncs[i].uninitialize = 0;
		}

		for (i = 0; i< RIPC_MAX_SSL_PROTOCOLS; i++)
		{
			encryptedSSLTransFuncs[i].bindSrvr = 0;
			encryptedSSLTransFuncs[i].newSrvrConnection = 0;
			encryptedSSLTransFuncs[i].connectSocket = 0;
			encryptedSSLTransFuncs[i].newClientConnection = 0;
			encryptedSSLTransFuncs[i].initializeTransport = 0;
			encryptedSSLTransFuncs[i].shutdownTransport = 0;
			encryptedSSLTransFuncs[i].readTransport = 0;
			encryptedSSLTransFuncs[i].writeTransport = 0;
			encryptedSSLTransFuncs[i].writeVTransport = 0;
			encryptedSSLTransFuncs[i].reconnectClient = 0;
			encryptedSSLTransFuncs[i].acceptSocket = 0;
			encryptedSSLTransFuncs[i].shutdownSrvrError = 0;
			encryptedSSLTransFuncs[i].sessIoctl = 0;

			encryptedSSLTransFuncs[i].getSockName = 0;
			encryptedSSLTransFuncs[i].setSockOpts = 0;
			encryptedSSLTransFuncs[i].getSockOpts = 0;
			encryptedSSLTransFuncs[i].connected = 0;
			encryptedSSLTransFuncs[i].shutdownServer = 0;
			encryptedSSLTransFuncs[i].uninitialize = 0;
		}

		ipcSetSockFuncs();

		for (i = 0; i <= RSSL_COMP_MAX_TYPE; i++)
		{
			compressFuncs[i].compressInit = 0;
			compressFuncs[i].decompressInit = 0;
			compressFuncs[i].compressEnd = 0;
			compressFuncs[i].decompressEnd = 0;
			compressFuncs[i].compress = 0;
			compressFuncs[i].decompress = 0;
		}

		ripcInitZlibComp();
		ripcInitLz4Comp();

		/* initialize open SSL library */
		/* Copy the ssl and crypto lib name config */

		if (initOpts->jitOpts.libsslName != 0 && initOpts->jitOpts.libcryptoName != 0)
		{
			libsslNameLen = (u8)strlen(initOpts->jitOpts.libsslName) + 1; // Include null character
			libcryptoNameLen = (u8)strlen(initOpts->jitOpts.libcryptoName) + 1; // Include null character
			transOpts.jitOpts.libsslName = (char*)malloc(libsslNameLen);
			transOpts.jitOpts.libcryptoName = (char*)malloc(libcryptoNameLen);
			if (transOpts.jitOpts.libsslName == 0 || transOpts.jitOpts.libcryptoName == 0)
			{
				error->rsslErrorId = RSSL_RET_FAILURE;
				snprintf(error->text, MAX_RSSL_ERROR_TEXT,
					"<%s:%d> Error: 1001 malloc failed.\n",
					__FILE__, __LINE__);

				ret = RSSL_RET_FAILURE;
				goto ripcinitend;
			}

			snprintf(transOpts.jitOpts.libsslName, libsslNameLen, "%s", initOpts->jitOpts.libsslName);
			snprintf(transOpts.jitOpts.libcryptoName, libcryptoNameLen, "%s", initOpts->jitOpts.libcryptoName);
		}

		if (initOpts->jitOpts.libcurlName != 0)
		{
			libcurlNameLen = (u8)strlen(initOpts->jitOpts.libcurlName) + 1;  // Include null charcter
			transOpts.jitOpts.libcurlName = (char*)malloc(libcurlNameLen);
			if (transOpts.jitOpts.libcurlName == 0)
			{
				error->rsslErrorId = RSSL_RET_FAILURE;
				snprintf(error->text, MAX_RSSL_ERROR_TEXT,
					"<%s:%d> Error: 1001 malloc failed.\n",
					__FILE__, __LINE__);

				ret = RSSL_RET_FAILURE;
				goto ripcinitend;
			}

			snprintf(transOpts.jitOpts.libcurlName, libcurlNameLen, "%s", initOpts->jitOpts.libcurlName);
		}

		protocolNumber = 6;  /* tcp (transmission control protocol) */

		if ((gblInputBufs = rtr_smplcAllocatePool(poolMutex)) == 0)
		{
			error->rsslErrorId = RSSL_RET_FAILURE;
			snprintf(error->text, MAX_RSSL_ERROR_TEXT,
				"<%s:%d> Error: 1001 ipcInitialize() rtr_smplcAllocatePool() failed.\n",
				__FILE__, __LINE__);

			ret = RSSL_RET_FAILURE;
			goto ripcinitend;
		}

		if (rssl_socket_startup(eText) < 0)
		{
			error->rsslErrorId = RSSL_RET_FAILURE;
			snprintf(error->text, MAX_RSSL_ERROR_TEXT,
				"<%s:%d> Error: 1001 ipcInitialize() rssl_socket_startup() failed. %s\n",
				__FILE__, __LINE__, eText);

			ret = RSSL_RET_FAILURE;
			goto ripcinitend;
		}

#if defined(SIGPIPE)
		if (rssl_ignore_sig_handler(SIGPIPE) == RSSL_FALSE)
		{
			_rsslSetError(error, NULL, RSSL_RET_FAILURE, errno);
			snprintf(error->text, MAX_RSSL_ERROR_TEXT,
				"<%s:%d> Error: 1001 ipcInitialize() Signal function call (SIGPIPE) failed (%d)",
				__FILE__, __LINE__, errno);

			ret = RSSL_RET_FAILURE;
			goto ripcinitend;
		}
#endif
#if defined(SIGURG)
		if (rssl_ignore_sig_handler(SIGURG) == RSSL_FALSE)
		{
			_rsslSetError(error, NULL, RSSL_RET_FAILURE, errno);
			snprintf(error->text, MAX_RSSL_ERROR_TEXT,
				"<%s:%d> Error: 1001 ipcInitialize() Signal function call (SIGURG) failed (%d)",
				__FILE__, __LINE__, errno);

			ret = RSSL_RET_FAILURE;
			goto ripcinitend;
		}
#endif

		for (i = 0; i<numServers; i++)
		{
			if ((rsslServerSocketChannel = createRsslServerSocketChannel()) != 0)
				rsslQueueAddLinkToBack(&freeServerSocketChannelList, &(rsslServerSocketChannel->link1));
		}

		for (i = 0; i<numClients; i++)
		{
			if ((rsslSocketChannel = createRsslSocketChannel()) != 0)
				rsslQueueAddLinkToBack(&freeSocketChannelList, &(rsslSocketChannel->link1));
		}

		initialized = 1;
	}
	ret = RSSL_RET_SUCCESS;
	++numInitCalls;

ripcinitend:
	if (ret == RSSL_RET_FAILURE)
	{
		transOpts.initCurlDebug = RSSL_FALSE;

		if (transOpts.initConfig)
		{
			free(transOpts.initConfig);
			transOpts.initConfig = 0;
		}
		if (transOpts.jitOpts.libsslName)
		{
			free(transOpts.jitOpts.libsslName);
			transOpts.jitOpts.libsslName = 0;
		}
		if (transOpts.jitOpts.libcryptoName)
		{
			free(transOpts.jitOpts.libcryptoName);
			transOpts.jitOpts.libcryptoName = 0;
		}
		if (transOpts.jitOpts.libcurlName)
		{
			free(transOpts.jitOpts.libcurlName);
			transOpts.jitOpts.libcurlName = 0;
		}
	}

	if (multiThread)
	{
	  (void) RSSL_MUTEX_UNLOCK(&ripcMutex);
		_DEBUG_MUTEX_TRACE("RSSL_MUTEX_UNLOCK", NULL, &ripcMutex)
	}

	return(ret);
}

RsslBool getCurlDebugMode() 
{
	return transOpts.initCurlDebug;
}

RsslInt32 getProtocolNumber()
{
	return protocolNumber;
}

ripcSSLFuncs* getSSLTransFuncs()
{
	return &SSLTransFuncs;
}

void ipcCleanRsslSocketChannel()
{
	RsslSocketChannel* session;
	RsslQueueLink* pLink;

    while (pLink = rsslQueueRemoveFirstLink(&freeSocketChannelList))
    {
		session = RSSL_QUEUE_LINK_TO_OBJECT(RsslSocketChannel, link1, pLink);
        _rsslFree(session);
    }
}

void ipcCleanRsslServerSocketChannel()
{
    RsslServerSocketChannel* session;
	RsslQueueLink* pLink;

    while (pLink = rsslQueueRemoveFirstLink(&freeServerSocketChannelList))
    {
		session = RSSL_QUEUE_LINK_TO_OBJECT(RsslServerSocketChannel, link1, pLink);
        _rsslFree(session);
    }
}

RsslInt32 ipcCleanup()
{
	if (multiThread)
	{
	  (void) RSSL_MUTEX_LOCK(&ripcMutex);
		_DEBUG_MUTEX_TRACE("RSSL_MUTEX_LOCK", NULL, &ripcMutex)
	}

	if (numInitCalls == 0)
	{
		if (multiThread)
		{
		  (void) RSSL_MUTEX_UNLOCK(&ripcMutex);
			_DEBUG_MUTEX_TRACE("RSSL_MUTEX_UNLOCK", NULL, &ripcMutex)
		}
		return(-1);
	}

	numInitCalls--;

	if ((numInitCalls == 0) && initialized)
	{
		int i;
		ipcCleanRsslServerSocketChannel();
		ipcCleanRsslSocketChannel();

		transOpts.initCurlDebug = RSSL_FALSE;

		if (gblInputBufs)
			rtr_smplcDropRef(gblInputBufs);

		for ( i=0; i<RIPC_MAX_TRANSPORTS; i++)
		{
			if (transFuncs[i].uninitialize)
				(*(transFuncs[i].uninitialize))();
		}

		if (transOpts.initConfig)
		{
			free(transOpts.initConfig);
			transOpts.initConfig = 0;
			transOpts.initConfigSize = 0;
		}

		if (openSSLInit == 1)
			openSSLInit = 0;

		if (transOpts.jitOpts.libsslName) {
			free(transOpts.jitOpts.libsslName);
			transOpts.jitOpts.libsslName = 0;
		}
		if (transOpts.jitOpts.libcryptoName) {
			free(transOpts.jitOpts.libcryptoName);
			transOpts.jitOpts.libcryptoName = 0;
		}

		if (transOpts.jitOpts.libcurlName)
		{
			free(transOpts.jitOpts.libcurlName);
			transOpts.jitOpts.libcurlName = 0;
		}

		if (libcurlInit == 1)
		{
			rsslUninitCurlApi();
			libcurlInit = 0;
		}

		rssl_socket_shutdown();

		initialized = 0;

		/* kill/cleanup threads - they should do this themselves when initialized is changed */
	}

	if (multiThread)
	{
	  (void) RSSL_MUTEX_UNLOCK(&ripcMutex);
		_DEBUG_MUTEX_TRACE("RSSL_MUTEX_UNLOCK", NULL, &ripcMutex)
	}

	if (numInitCalls == 0)
	{
		if (multiThread)
		{
			if (gblmutexinit)
			{
				RTR_ATOMIC_SET(gblmutexinit,0);
				RSSL_MUTEX_DESTROY(&ripcMutex);
			}
		}
	}

	return 1;
}

RsslInt32 ipcShutdownServer(RsslServerSocketChannel* rsslServerSocketChannel, RsslError *error)
{
	RsslInt32			ret = RSSL_RET_FAILURE;

	if (IPC_NULL_PTR(rsslServerSocketChannel, "ipcShutdownServer", "server", error))
		return RSSL_RET_FAILURE;

	if (rsslServerSocketChannel != 0)
	{
	IPC_MUTEX_LOCK(rsslServerSocketChannel);

		if (SSLTransFuncs.freeSSLServer && rsslServerSocketChannel->connType == RSSL_CONN_TYPE_ENCRYPTED)
			(*(SSLTransFuncs.freeSSLServer))(rsslServerSocketChannel->transportInfo, error);


		/* this should only be done on the platforms this is supported on */
		(*(transFuncs[rsslServerSocketChannel->connType].shutdownServer))((void*)rsslServerSocketChannel);

	IPC_MUTEX_UNLOCK(rsslServerSocketChannel);
    }

	return(ret);
}

/***************************
 * START PUBLIC ABSTRACTED FUNCTIONS
 ***************************/

RsslInt32 ipcSrvrDropRef(RsslServerSocketChannel *rsslServerSocketChannel, RsslError *error)
{
	RsslMutex *mutex = 0;

	if (IPC_NULL_PTR(rsslServerSocketChannel, "ipcSrvrDropRef", "rsslServerSocketChannel", error))
		return RSSL_RET_FAILURE;

	mutex = rsslServerSocketChannel->mutex;

	IPC_MUTEX_LOCK(rsslServerSocketChannel);

	relRsslServerSocketChannel(rsslServerSocketChannel);

	if ((multiThread) && (mutex))
	{
	  (void) RSSL_MUTEX_UNLOCK(mutex);
		_DEBUG_MUTEX_TRACE("RSSL_MUTEX_UNLOCK", NULL, mutex)
	}

	return(1);
}

RSSL_RSSL_SOCKET_FAST(RsslRet) rsslCloseSocketSrvr(rsslServerImpl *rsslSrvrImpl, RsslError *error)
{
	RsslRet retVal = RSSL_RET_SUCCESS;

	RsslServerSocketChannel* rsslSrvrSocketChannel = (RsslServerSocketChannel*)rsslSrvrImpl->transportInfo;

	if (rsslSrvrSocketChannel != 0)
	{
		/* this should only be done on the platforms this is supported on */
		if (rsslSrvrSocketChannel->stream != RIPC_INVALID_SOCKET)
		{
			sock_close(rsslSrvrSocketChannel->stream);
			rsslSrvrSocketChannel->stream = RIPC_INVALID_SOCKET;

			if (rsslSrvrSocketChannel->state != RSSL_CH_STATE_INACTIVE)
				rsslSrvrSocketChannel->state = RSSL_CH_STATE_INACTIVE;
		}

		rsslSrvrSocketChannel->transportInfo = NULL;
		retVal = ipcSrvrDropRef(rsslSrvrSocketChannel, error);
	}

	_rsslCleanServer(rsslSrvrImpl);

	return retVal;
}

RsslRet rsslSocketSetChannelFunctions()
{
	RsslTransportChannelFuncs funcs;

	_DEBUG_TRACE_INIT("HERE\n")

	funcs.channelBufferUsage = rsslSocketBufferUsage;
	funcs.channelClose = rsslSocketCloseChannel;
	funcs.channelConnect = rsslSocketConnect;
	funcs.channelFlush = rsslSocketFlush;
	funcs.channelGetBuffer = rsslSocketGetBuffer;
	funcs.channelGetInfo = rsslSocketGetChannelInfo;
	funcs.channelIoctl = rsslSocketIoctl;
	funcs.channelPackBuffer = rsslSocketPackBuffer;
	funcs.channelPing = rsslSocketPing;
	funcs.channelRead = rsslSocketRead;
	funcs.channelReconnect = rsslSocketReconnect;
	funcs.channelReleaseBuffer = rsslSocketReleaseBuffer;
	funcs.channelWrite = rsslSocketWrite;
	funcs.initChannel = rsslSocketInitChannel;

	return(rsslSetTransportChannelFunc(RSSL_SOCKET_TRANSPORT,&funcs));
}

RsslRet rsslWebSocketSetChannelFunctions()
{
	RsslTransportChannelFuncs funcs;

	_DEBUG_TRACE_INIT("HERE\n")

	funcs.channelBufferUsage = rsslSocketBufferUsage;
	funcs.channelClose = rsslWebSocketCloseChannel;
	funcs.channelConnect = rsslSocketConnect;
	funcs.channelFlush = rsslSocketFlush;
	funcs.channelGetBuffer = rsslWebSocketGetBuffer;
	funcs.channelGetInfo = rsslSocketGetChannelInfo;
	funcs.channelIoctl = rsslSocketIoctl;
	funcs.channelPackBuffer = rsslWebSocketPackBuffer;
	funcs.channelPing = rsslWebSocketPing;
	funcs.channelRead = rsslWebSocketRead;
	funcs.channelReconnect = rsslSocketReconnect;
	funcs.channelReleaseBuffer = rsslSocketReleaseBuffer;
	funcs.channelWrite = rsslWebSocketWrite;
	funcs.initChannel = rsslSocketInitChannel;

	return(rsslSetTransportChannelFunc(RSSL_WEBSOCKET_TRANSPORT,&funcs));
}

RsslRet rsslSocketSetServerFunctions()
{
	RsslTransportServerFuncs funcs;

	_DEBUG_TRACE_INIT("HERE\n")
	funcs.serverAccept = rsslSocketAccept;
	funcs.serverBind = rsslSocketBind;
	funcs.serverIoctl = rsslSocketSrvrIoctl;
	funcs.serverGetInfo = rsslSocketGetSrvrInfo;
	funcs.serverBufferUsage = rsslSocketSrvrBufferUsage;
	funcs.closeServer = rsslCloseSocketSrvr;

	return(rsslSetTransportServerFunc(RSSL_SOCKET_TRANSPORT, &funcs));
}

RsslRet rsslSocketInitialize(RsslInitializeExOpts *initOpts, RsslError *error)
{
	RsslRet retVal = RSSL_RET_SUCCESS;

	retVal = ipcInitialize(10, 10, initOpts, error);

	_DEBUG_TRACE_INIT("HERE\n")

	if (retVal < RSSL_RET_SUCCESS)
		return retVal;
	else
	{
		retVal = rsslSocketSetServerFunctions();
		retVal = rsslSocketSetChannelFunctions();
		retVal = rsslWebSocketSetChannelFunctions();
	}

	return retVal;
}

RsslRet rsslSocketUninitialize()
{
	/* clean up ipc */
	return ipcCleanup();
}

RsslInt32 ripcSetDbgFuncs(
	void(*dumpIn)(const char *functionName, char *buf, RsslUInt32 len, RsslUInt64 opaque),
	void(*dumpOut)(const char *functionName, char *buf, RsslUInt32 len, RsslUInt64 opaque))
{
	if (multiThread)
	{
		if (!gblmutexinit)
		{
			(void) RSSL_MUTEX_INIT_ESDK(&ripcMutex);
			RTR_ATOMIC_SET(gblmutexinit,1);
		}

		(void) RSSL_MUTEX_LOCK(&ripcMutex);
		_DEBUG_MUTEX_TRACE("RSSL_MUTEX_LOCK", NULL, &ripcMutex)
	}

	if ((dumpIn && ripcDumpInFunc) || (dumpOut && ripcDumpOutFunc))
	{
		if (multiThread)
		{
		  (void) RSSL_MUTEX_UNLOCK(&ripcMutex);
			_DEBUG_MUTEX_TRACE("RSSL_MUTEX_UNLOCK", NULL, &ripcMutex)
		}
		return(-1);
	}
	ripcDumpInFunc = dumpIn;
	ripcDumpOutFunc = dumpOut;

	if (multiThread)
	{
	  (void) RSSL_MUTEX_UNLOCK(&ripcMutex);
		_DEBUG_MUTEX_TRACE("RSSL_MUTEX_UNLOCK", NULL, &ripcMutex)
	}

	return(0);
}

RsslInt32 rwsDbgFuncs(
	void(*dumpIn)(const char *functionName, char *buf, RsslUInt32 len, RsslUInt64 opaque),
	void(*dumpOut)(const char *functionName, char *buf, RsslUInt32 len, RsslUInt64 opaque))
{
	if (multiThread)
	{
		if (!gblmutexinit)
		{
			(void)RSSL_MUTEX_INIT_ESDK(&ripcMutex);
			RTR_ATOMIC_SET(gblmutexinit, 1);
		}

		(void)RSSL_MUTEX_LOCK(&ripcMutex);
		_DEBUG_MUTEX_TRACE("RSSL_MUTEX_LOCK", NULL, &ripcMutex)
	}

	if ((dumpIn && webSocketDumpInFunc) || (dumpOut && webSocketDumpOutFunc))
	{
		if (multiThread)
		{
			(void)RSSL_MUTEX_UNLOCK(&ripcMutex);
			_DEBUG_MUTEX_TRACE("RSSL_MUTEX_UNLOCK", NULL, &ripcMutex)
		}
		return(-1);
	}
	webSocketDumpInFunc = dumpIn;
	webSocketDumpOutFunc = dumpOut;

	if (multiThread)
	{
		(void)RSSL_MUTEX_UNLOCK(&ripcMutex);
		_DEBUG_MUTEX_TRACE("RSSL_MUTEX_UNLOCK", NULL, &ripcMutex)
	}

	return(0);
}


/* Sets Socket debug dump functions */
RsslRet rsslSetSocketDebugFunctions(
		void(*dumpIpcIn)(const char *functionName, char *buffer, RsslUInt32 length, RsslUInt64 opaque),
		void(*dumpIpcOut)(const char *functionName, char *buffer, RsslUInt32 length, RsslUInt64 opaque),
		void(*dumpRsslIn)(const char *functionName, char *buffer, RsslUInt32 length, RsslSocket socketId),
		void(*dumpRsslOut)(const char *functionName, char *buffer, RsslUInt32 length, RsslSocket socketId),
		RsslError *error)
{
	RsslRet retVal = RSSL_RET_SUCCESS;
	RsslInt32	tempRetVal = 0;

	if ((dumpRsslIn && rsslSocketDumpInFunc) || (dumpRsslOut && rsslSocketDumpOutFunc))
	{
		/* set error message */
		_rsslSetError(error, NULL, RSSL_RET_FAILURE, 0);
		snprintf(error->text, MAX_RSSL_ERROR_TEXT, "<%s:%d> rsslSetDebugFunctions() Cannot set socket Rssl dump functions.\n", __FILE__, __LINE__);

		retVal = RSSL_RET_FAILURE;
	}
	else
	{
		rsslSocketDumpInFunc = dumpRsslIn;
		rsslSocketDumpOutFunc = dumpRsslOut;
		retVal = RSSL_RET_SUCCESS;
	}

	tempRetVal = ripcSetDbgFuncs(dumpIpcIn, dumpIpcOut);

	if ((tempRetVal < RSSL_RET_SUCCESS) && (retVal < RSSL_RET_SUCCESS))
	{
		/* set error message */
		_rsslSetError(error, NULL, RSSL_RET_FAILURE, 0);
		snprintf(error->text, MAX_RSSL_ERROR_TEXT, "<%s:%d> rsslSetDebugFunctions() Cannot set socket Rssl and IPC dump functions.\n", __FILE__, __LINE__);

		retVal = RSSL_RET_FAILURE;
	}
	else if (tempRetVal < RSSL_RET_SUCCESS)
	{
		/* set error message */
		_rsslSetError(error, NULL, RSSL_RET_FAILURE, 0);
		snprintf(error->text, MAX_RSSL_ERROR_TEXT, "<%s:%d> rsslSetDebugFunctions() Cannot set socket IPC dump functions.\n", __FILE__, __LINE__);

		retVal = RSSL_RET_FAILURE;
	}

	return retVal;
}


RsslInt32 rsslSocketGetSockOpts( RsslSocketChannel *rsslSocketChannel, RsslInt32 code, RsslInt32* value, RsslError *error)
{
	RsslInt32 ret=1;

	if ((code < 0) || (code > RIPC_MAX_IOCTL_CODE))
	{
		_rsslSetError(error, NULL, RSSL_RET_FAILURE, errno);
		snprintf(error->text, MAX_RSSL_ERROR_TEXT,
				"<%s:%d> Error: 1005 rsslSocketGetSockOpts() failed for code %d\n",
				__FILE__, __LINE__, code);
		error->rsslErrorId = RSSL_RET_FAILURE;

		return RSSL_RET_FAILURE;
	}

	IPC_MUTEX_LOCK(rsslSocketChannel);

	if (rsslSocketChannel->workState & RIPC_INT_SHTDOWN_PEND)
	{
		_rsslSetError(error, NULL, RSSL_RET_FAILURE, errno);
		snprintf(error->text, MAX_RSSL_ERROR_TEXT,
				"<%s:%d> Error: 1005 rsslSocketGetSockOpts() failed due to channel shutdown.\n",
				__FILE__, __LINE__);

		IPC_MUTEX_UNLOCK(rsslSocketChannel);

		return RSSL_RET_FAILURE;
	}

	switch (code)
	{
		case RIPC_SYSTEM_READ_BUFFERS:

			if (((*(rsslSocketChannel->transportFuncs->getSockOpts))(rsslSocketChannel->stream,
																	 RIPC_SYSTEM_READ_BUFFERS, value,
																	 rsslSocketChannel->transportInfo,
																	 error)) < 0)
				ret = RSSL_RET_FAILURE;
		break;

		case RIPC_SYSTEM_WRITE_BUFFERS:

			if (((*(rsslSocketChannel->transportFuncs->getSockOpts))(rsslSocketChannel->stream,
																	 RIPC_SYSTEM_WRITE_BUFFERS, value,
																	 rsslSocketChannel->transportInfo,
																	 error)) < 0)
				ret = RSSL_RET_FAILURE;
		break;

		default:

			ret = RSSL_RET_FAILURE;
	}

	IPC_MUTEX_UNLOCK(rsslSocketChannel);

	return ret;
}

RsslInt32 ipcShutdownSockectChannel(RsslSocketChannel* rsslSocketChannel, RsslError *error)
{
	if (IPC_NULL_PTR(rsslSocketChannel,"ipcShutdownSockectChannel","socket",error))
		return RSSL_RET_FAILURE;

	IPC_MUTEX_LOCK(rsslSocketChannel);

	/* clear up the socket struct */
	rsslSocketChannelClose(rsslSocketChannel);

	IPC_MUTEX_UNLOCK(rsslSocketChannel);

	return(1);
}

RsslInt32 ipcSessDropRef(RsslSocketChannel *rsslSocketChannel, RsslError *error)
{
	RsslMutex* mutex = NULL;

	if (IPC_NULL_PTR(rsslSocketChannel, "ipcSessDropRef", "socket", error))
		return RSSL_RET_FAILURE;

	mutex = rsslSocketChannel->mutex;

	if(mutex)
	{
		(void) RSSL_MUTEX_LOCK(mutex);
		_DEBUG_MUTEX_TRACE("RSSL_MUTEX_LOCK", NULL, mutex)
	}

	ripcRelSocketChannel(rsslSocketChannel);

	if(mutex)
	{
	(void) RSSL_MUTEX_UNLOCK(mutex);
		_DEBUG_MUTEX_TRACE("RSSL_MUTEX_UNLOCK", NULL, mutex)
	}

	return(1);
}

ripcSessInit ipcSessionInit(RsslSocketChannel *rsslSocketChannel, ripcSessInProg *inPr, RsslError *error)
{
	RsslInt32			ret;

	_DEBUG_TRACE_CONN("called\n")

	if (IPC_NULL_PTR(rsslSocketChannel,"ipcSessionInit","socket",error))
		return(RIPC_CONN_ERROR);

	IPC_MUTEX_LOCK(rsslSocketChannel);

	if (rsslSocketChannel->workState & RIPC_INT_SHTDOWN_PEND)
	{
		RIPC_ASSERT(rsslSocketChannel->state == RSSL_CH_STATE_INACTIVE);

		_rsslSetError(error, NULL, RSSL_RET_FAILURE, errno);
		snprintf(error->text, MAX_RSSL_ERROR_TEXT,
				"<%s:%d> Error: 1003 ipcSessionInit failed due to channel shutting down.\n",
				__FILE__, __LINE__);

		IPC_MUTEX_UNLOCK(rsslSocketChannel);

		return RSSL_RET_FAILURE;
	}
	else
	{
		/* This can only happen in the threaded case */
		if (rsslSocketChannel->workState & RIPC_INT_READ_THR)
			ret = 1;
		else
		{
			rsslSocketChannel->workState |= RIPC_INT_READ_THR;

			if((ret = ipcIntSessInit(rsslSocketChannel,inPr,error)) < 0)
			{
				IPC_MUTEX_UNLOCK(rsslSocketChannel);
				return(ret);
			}

			rsslSocketChannel->workState &= ~RIPC_INT_READ_THR;
		}
	}

	IPC_MUTEX_UNLOCK(rsslSocketChannel);

	return(ret);
}

RSSL_RSSL_SOCKET_IMPL_FAST(void) relRsslServerSocketChannel(RsslServerSocketChannel* rsslServerSocketChannel)
{
	if (rsslServerSocketChannel)
	{
		_DEBUG_TRACE_REF("RsslServerSocketChannel=0x%p *rSC 0x%p\n",rsslServerSocketChannel,*rsslServerSocketChannel)

		if (multiThread)
		{
		  (void) RSSL_MUTEX_LOCK(&ripcMutex);
			_DEBUG_MUTEX_TRACE("RSSL_MUTEX_LOCK", NULL, &ripcMutex)
		}

		if (rsslServerSocketChannel->sharedBufPool)
		{
			rtrBufferPoolDropRef(rsslServerSocketChannel->sharedBufPool);
			rsslServerSocketChannel->sharedBufPool = 0;
		}

		if (rsslServerSocketChannel->serverName != 0)
		{
			_rsslFree((void*)rsslServerSocketChannel->serverName);
			rsslServerSocketChannel->serverName = 0;
		}

		if (rsslServerSocketChannel->rwsServer)
		{
			rsslReleaseWebSocketServer(rsslServerSocketChannel->rwsServer);
			rsslServerSocketChannel->rwsServer = 0;
		}

		if (rsslServerSocketChannel->cipherSuite != 0)
		{
			_rsslFree((void*)rsslServerSocketChannel->cipherSuite);
			rsslServerSocketChannel->cipherSuite = 0;
		}

		if (rsslServerSocketChannel->serverCert != 0)
		{
			_rsslFree((void*)rsslServerSocketChannel->serverCert);
			rsslServerSocketChannel->serverCert = 0;
		}

		if (rsslServerSocketChannel->serverPrivateKey != 0)
		{
			_rsslFree((void*)rsslServerSocketChannel->serverPrivateKey);
			rsslServerSocketChannel->serverPrivateKey = 0;
		}

		if (rsslServerSocketChannel->dhParams != 0)
		{
			_rsslFree((void*)rsslServerSocketChannel->dhParams);
			rsslServerSocketChannel->dhParams = 0;
		}

		rsslClearRsslServerSocketChannel(rsslServerSocketChannel);

		rsslQueueAddLinkToBack(&freeServerSocketChannelList, &(rsslServerSocketChannel->link1));

		if (multiThread)
		{
		  (void) RSSL_MUTEX_UNLOCK(&ripcMutex);
			_DEBUG_MUTEX_TRACE("RSSL_MUTEX_UNLOCK", NULL, &ripcMutex)
		}
	}
}

RSSL_RSSL_SOCKET_IMPL_FAST(void) ripcRelSocketChannel(RsslSocketChannel *rsslSocketChannel)
{
	RsslQueueLink *pLink = 0;
	RsslCurlJITFuncs* curlFuncs;

    _DEBUG_TRACE_REF("RsslSocketChannel=0x%p *rSC 0x%p\n", rsslSocketChannel, *rsslSocketChannel)

	if (multiThread)
	{
	  (void) RSSL_MUTEX_LOCK(&ripcMutex);
		_DEBUG_MUTEX_TRACE("RSSL_MUTEX_LOCK", NULL, &ripcMutex)
	}

	if (rsslSocketChannel->curlHandle)
	{
		if (rsslSocketChannel->curlThreadInfo.curlThreadState == RSSL_CURL_ACTIVE)
		{
			// Wait for the curl thread to finish.  It should always complete
			RSSL_THREAD_JOIN(rsslSocketChannel->curlThreadInfo.curlThreadId);
		}
			
		/* If this is false, then this is an error condition, so clean out the curlHandle and continue with the shutdown */
		if ((curlFuncs = rsslGetCurlFuncs()) != NULL)
		{
			(*(curlFuncs->curl_easy_cleanup))(rsslSocketChannel->curlHandle);
            
            if(rsslSocketChannel->curlThreadInfo.curlError)
                _rsslFree(rsslSocketChannel->curlThreadInfo.curlError);
            rsslSocketChannel->curlThreadInfo.curlError = 0;
		}
		
		rsslSocketChannel->curlHandle = 0;
	}

	rssl_pipe_close(&rsslSocketChannel->sessPipe);
	
	if (rsslQueueLinkInAList(&(rsslSocketChannel->link1)))
	{
		rsslQueueRemoveLink(&activeSocketChannelList, &(rsslSocketChannel->link1));
	}

    if (rsslSocketChannel->guarBufPool)
	{
		int i;
		rtr_msgb_t *mblk;
		for (i = 0; i < RIPC_MAX_PRIORITY_QUEUE; i++)
		{
			while ((pLink = rsslQueueRemoveFirstLink(&(rsslSocketChannel->priorityQueues[i].priorityQueue))) != 0)
			{
				mblk = RSSL_QUEUE_LINK_TO_OBJECT(rtr_msgb_t, link, pLink);
				rtr_dfltcFreeMsg(mblk);
			}
		}

		rtr_dfltcDropRef(&(rsslSocketChannel->guarBufPool->bufpool));
		rsslSocketChannel->guarBufPool = 0;
	}

	if (rsslSocketChannel->curInputBuf)
	{
		rtr_smplcFreeMsg(rsslSocketChannel->curInputBuf);
		rsslSocketChannel->curInputBuf = 0;
	}

	if (rsslSocketChannel->inputBuffer)
	{
		rtr_smplcFreeMsg(rsslSocketChannel->inputBuffer);
		rsslSocketChannel->inputBuffer = 0;
	}

	if (rsslSocketChannel->decompressBuf)
	{
		rtr_smplcFreeMsg(rsslSocketChannel->decompressBuf);
		rsslSocketChannel->decompressBuf = 0;
	}

	if (rsslSocketChannel->tempCompressBuf)
	{
		rtr_smplcFreeMsg(rsslSocketChannel->tempCompressBuf);
		rsslSocketChannel->tempCompressBuf = 0;
	}

	if (rsslSocketChannel->tempDecompressBuf)
	{
		rtr_smplcFreeMsg(rsslSocketChannel->tempDecompressBuf);
		rsslSocketChannel->tempDecompressBuf = 0;
	}

	if (rsslSocketChannel->c_stream_out && rsslSocketChannel->outCompFuncs)
		(*(rsslSocketChannel->outCompFuncs->compressEnd))(rsslSocketChannel->c_stream_out);

	if (rsslSocketChannel->c_stream_in && rsslSocketChannel->inDecompFuncs)
		(*(rsslSocketChannel->inDecompFuncs->decompressEnd))(rsslSocketChannel->c_stream_in);

	if (rsslSocketChannel->hostName != 0)
	{
		_rsslFree((void*)rsslSocketChannel->hostName);
		rsslSocketChannel->hostName = 0;
	}

    if (rsslSocketChannel->clientIP != 0)
	{
		_rsslFree((void*)rsslSocketChannel->clientIP);
		rsslSocketChannel->clientIP = 0;
	}

    if (rsslSocketChannel->clientHostname != 0)
    {
        _rsslFree((void*)rsslSocketChannel->clientHostname);
        rsslSocketChannel->clientHostname = 0;
    }

	if (rsslSocketChannel->serverName != 0)
	{
		_rsslFree((void*)rsslSocketChannel->serverName);
		rsslSocketChannel->serverName = 0;
	}
	if (rsslSocketChannel->objectName != 0)
	{
		_rsslFree((void*)rsslSocketChannel->objectName);
		rsslSocketChannel->objectName = 0;
	}
	if (rsslSocketChannel->interfaceName != 0)
	{
		_rsslFree((void*)rsslSocketChannel->interfaceName);
		rsslSocketChannel->interfaceName = 0;
	}
	if (rsslSocketChannel->proxyHostName != 0)
	{
		_rsslFree((void*)rsslSocketChannel->proxyHostName);
		rsslSocketChannel->proxyHostName = 0;
	}
	if (rsslSocketChannel->proxyPort != 0)
	{
		_rsslFree((void*)rsslSocketChannel->proxyPort);
		rsslSocketChannel->proxyPort = 0;
	}
	if (rsslSocketChannel->curlOptProxyUser != 0)
	{
		_rsslFree((void*)rsslSocketChannel->curlOptProxyUser);
		rsslSocketChannel->curlOptProxyUser = 0;
	}
	if (rsslSocketChannel->curlOptProxyPasswd != 0)
	{
		_rsslFree((void*)rsslSocketChannel->curlOptProxyPasswd);
		rsslSocketChannel->curlOptProxyPasswd = 0;
	}
	if (rsslSocketChannel->curlOptProxyDomain != 0)
	{
		_rsslFree((void*)rsslSocketChannel->curlOptProxyDomain);
		rsslSocketChannel->curlOptProxyDomain = 0;
	}
    if (rsslSocketChannel->outComponentVer)
	{
		/* we created and own memory for this, free it */
		_rsslFree((void*)rsslSocketChannel->outComponentVer);
	}
	if(rsslSocketChannel->sslCAStore)
	{
		/* we created and own memory for this, free it */
		_rsslFree((void*)rsslSocketChannel->sslCAStore);
	}

	while (rsslSocketChannel->rwsLargeMsgBufferList!= 0)
	{
		rtr_msgb_t *bufPtr = 0;

		bufPtr = rsslSocketChannel->rwsLargeMsgBufferList;
		rsslSocketChannel->rwsLargeMsgBufferList = (rtr_msgb_t*)rsslSocketChannel->rwsLargeMsgBufferList->internal;
		if (bufPtr->protocolHdr)
			bufPtr->buffer -= bufPtr->protocolHdr;

		_rsslFree((void*)bufPtr->buffer);
		_rsslFree((void*)bufPtr);
	}

	rssl_pipe_close(&rsslSocketChannel->sessPipe);

	if (rsslSocketChannel->rwsSession)
	{
		rsslReleaseWebSocketSession(rsslSocketChannel->rwsSession);
		rsslSocketChannel->rwsSession = 0;
	}

	ripcClearRsslSocketChannel(rsslSocketChannel);
	/* do not clear sessionID here */

	rsslQueueAddLinkToFront(&freeSocketChannelList, &(rsslSocketChannel->link1));

	if (multiThread)
	{
		(void) RSSL_MUTEX_UNLOCK(&ripcMutex);
		_DEBUG_MUTEX_TRACE("RSSL_MUTEX_UNLOCK", NULL, &ripcMutex)
	}
}

/* Session should be locked before call */
void rsslSocketChannelClose(RsslSocketChannel *rsslSocketChannel)
{
	/* When we are setting up for renegotiation a new socket
	* is created and the old is closed. We need to keep
	* track of both in case of an error for feedback to user.
	*/
	if (rsslSocketChannel->newStream != RIPC_INVALID_SOCKET )
	{
		if (!(rsslSocketChannel->workState & RIPC_INT_SOCK_CLOSED))
		{
			if (rsslSocketChannel->tunnelingState != RIPC_TUNNEL_REMOVE_SESSION)
				(*(rsslSocketChannel->transportFuncs->shutdownTransport))(rsslSocketChannel->newTransportInfo);

			rsslSocketChannel->newStream = RIPC_INVALID_SOCKET;
			rsslSocketChannel->newTransportInfo = NULL;
			rsslSocketChannel->workState |= RIPC_INT_SOCK_CLOSED;
		}
	}

	if (rsslSocketChannel->stream != RIPC_INVALID_SOCKET)
	{
		if (!(rsslSocketChannel->workState & RIPC_INT_SOCK_CLOSED))
		{
			/* If we're in a proxy connecting state, and transportInfo should not be setup yet.  
				Join the CURL thread and close out the CURL channel, then close the pipe */
			if (rsslSocketChannel->intState == RIPC_INT_ST_PROXY_CONNECTING || rsslSocketChannel->intState == RIPC_INT_ST_CLIENT_WAIT_PROXY_ACK)
			{
				RSSL_THREAD_JOIN(rsslSocketChannel->curlThreadInfo.curlThreadId);
				rssl_pipe_close(&rsslSocketChannel->sessPipe);
			}
			
			/* if we are doing tunneling, this state means we dont want to close the real fd */
			if (rsslSocketChannel->tunnelingState != RIPC_TUNNEL_REMOVE_SESSION)
			{
				RsslCurlJITFuncs* curlFuncs;
				if (rsslSocketChannel->curlHandle != NULL)
				{
					/* If the curl functions are null, the handle should already have been cleaned up, so set it to NULL here. */

					if ((curlFuncs = rsslGetCurlFuncs()) != NULL)
                    {
						(*(curlFuncs->curl_easy_cleanup))(rsslSocketChannel->curlHandle);
                        if(rsslSocketChannel->curlThreadInfo.curlError) 
                            _rsslFree(rsslSocketChannel->curlThreadInfo.curlError);
                        rsslSocketChannel->curlThreadInfo.curlError = 0;
                    }
					rsslSocketChannel->curlHandle = NULL;

					/* a CURL handle will only be set if this is an openSSL encrypted connection.  WinInet connections do not use curl */
					if (rsslSocketChannel->connType == RSSL_CONN_TYPE_ENCRYPTED && rsslSocketChannel->usingWinInet == 0)
					{
						if(rsslSocketChannel->transportInfo != NULL)
							((ripcSSLSession*)rsslSocketChannel->transportInfo)->socket = RIPC_INVALID_SOCKET;
					}
					else if (rsslSocketChannel->connType == RSSL_CONN_TYPE_SOCKET)
					{
						/* Set the transportInfo FD to RIPC_INVALID_SOCKET, because the curl cleanup call will close the channel for us */
						rsslSocketChannel->transportInfo = (void*)RIPC_INVALID_SOCKET;
					}
				}

				(*(rsslSocketChannel->transportFuncs->shutdownTransport))(rsslSocketChannel->transportInfo);

				/* Handle the Java HTTP Tunnelling case, where transportInfo may be the same as tunnelTransportInfo */
				if (rsslSocketChannel->transportInfo == rsslSocketChannel->tunnelTransportInfo)
					rsslSocketChannel->tunnelTransportInfo = NULL;
				rsslSocketChannel->transportInfo = NULL;
			}

			rsslSocketChannel->stream = RIPC_INVALID_SOCKET;
			rsslSocketChannel->workState |= RIPC_INT_SOCK_CLOSED;
		}
	}

	/* if we are tunneling, these may be set, and will be handled appropriately */
	if (rsslSocketChannel->tunnelTransportInfo != 0)
	{
		if (rsslSocketChannel->tunnelingState != RIPC_TUNNEL_REMOVE_SESSION)
			(*(rsslSocketChannel->transportFuncs->shutdownTransport))(rsslSocketChannel->tunnelTransportInfo);
		rsslSocketChannel->tunnelTransportInfo = 0;
	}

	if (rsslSocketChannel->newTunnelTransportInfo != 0)
	{
		if (rsslSocketChannel->oldTunnelStreamFd == ((rsslSocketChannel->connType == RSSL_CONN_TYPE_ENCRYPTED) ? 
			((ripcSSLSession*)(rsslSocketChannel->newTunnelTransportInfo))->socket : (RsslSocket)(intptr_t)(rsslSocketChannel->newTunnelTransportInfo)))
			rsslSocketChannel->oldTunnelStreamFd = RIPC_INVALID_SOCKET;
		if (rsslSocketChannel->tunnelingState != RIPC_TUNNEL_REMOVE_SESSION)
			(*(rsslSocketChannel->transportFuncs->shutdownTransport))(rsslSocketChannel->newTunnelTransportInfo);
		rsslSocketChannel->newTunnelTransportInfo = 0;
	}

	/* Unlike the other two up top, here we're just getting rid of the old streaming FD, if it hasn't been cleared yet.*/
	if (rsslSocketChannel->oldTunnelStreamFd != RIPC_INVALID_SOCKET)
	{
		sock_close(rsslSocketChannel->oldTunnelStreamFd);
		rsslSocketChannel->oldTunnelStreamFd = RIPC_INVALID_SOCKET;
	}

	if (!(rsslSocketChannel->workState & RIPC_INT_SHTDOWN_PEND))
		rsslSocketChannel->workState |= RIPC_INT_SHTDOWN_PEND;

	if (rsslSocketChannel->state != RSSL_CH_STATE_INACTIVE)
		rsslSocketChannel->state = RSSL_CH_STATE_INACTIVE;
}

