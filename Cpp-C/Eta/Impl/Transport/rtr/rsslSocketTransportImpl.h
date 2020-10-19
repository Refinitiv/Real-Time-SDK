/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|          Copyright (C) 2019-2020 Refinitiv. All rights reserved.          --
 *|-----------------------------------------------------------------------------
 */

#ifndef __RTR_RSSL_SOCKET_TRANSPORT_IMPL_H
#define __RTR_RSSL_SOCKET_TRANSPORT_IMPL_H

/* Contains function declarations necessary for the
 * bi-directional socket (and HTTP/HTTPS) connection type 
 */

#ifdef __cplusplus
extern "C" {
#endif

#include "rtr/cutildfltcbuffer.h"

#include "rtr/rsslTypes.h"
#include "rtr/rsslChanManagement.h"
#include "rtr/ripch.h"
#include "rtr/ripc_int.h"
#include "rtr/ripcutils.h"
#include "rtr/custmem.h"
#include "rtr/rwfNet.h"
#include "rtr/rwfNetwork.h"
#include "rtr/rsslpipe.h"
#include "rtr/application_signing.h"
#include "rtr/cutilsmplcbuffer.h"
#include "rtr/rsslQueue.h"
#include "rtr/debugPrint.h"
#include <stdio.h>
#include "curl/curl.h"
#if defined(_WIN32)
#include <tcpmib.h>
#endif

#define RIPC_MAX_CONN_HDR_LEN  V10_MIN_CONN_HDR + MAX_RSSL_ERROR_TEXT 

#define RSSL_RSSL_SOCKET_IMPL_FAST(ret)		ret RTR_FASTCALL

#define RIPC_RWF_PROTOCOL_TYPE 0 /* must match definition for RWF in RSSL */
#define RIPC_TRWF_PROTOCOL_TYPE 1 /* must match definition for TRWF in RSSL */
#define RIPC_JSON_PROTOCOL_TYPE 2 /* must match definition for JSON2 in RSSL */
#define RIPC_MAX_PROTOCOL_TYPE 3 /* must match definition for TRWF in RSSL */

/* Current number of bytes in the compression bitmap */
#define RIPC_COMP_BITMAP_SIZE 1

#define RIPC_SOCKET_TRANSPORT   0
#define RIPC_OPENSSL_TRANSPORT  1
#define RIPC_WININET_TRANSPORT  2
#define RIPC_EXT_LINE_SOCKET_TRANSPORT 5
#define RIPC_WEBSOCKET_TRANSPORT   7
#define RIPC_MAX_TRANSPORTS     RIPC_WEBSOCKET_TRANSPORT + 1
#define RIPC_MAX_SSL_PROTOCOLS  4		/* TLSv1, TLSv1.1, TLSv1.2 */

typedef enum {
	RIPC_SSL_TLS_V1 = 0,
	RIPC_SSL_TLS_V1_1 = 1,
	RIPC_SSL_TLS_V1_2 = 2,
	RIPC_SSL_TLS = 3
} ripcSSLProtocolIndex;

#define IPC_MUTEX_LOCK(session) \
	{					\
		if ((session)->mutex) \
		{ \
		  (void) RSSL_MUTEX_LOCK((session)->mutex);	\
		  _DEBUG_MUTEX_TRACE("IPC_MUTEX_LOCK", session, (session)->mutex)\
		} \
	}

#define IPC_MUTEX_UNLOCK(session) \
		{					\
		if ((session)->mutex) \
		{ \
		  (void) RSSL_MUTEX_UNLOCK((session)->mutex);	\
		  _DEBUG_MUTEX_TRACE("IPC_MUTEX_UNLOCK", session, (session)->mutex)\
		} \
	}

typedef enum {
	RIPC_NO_TUNNELING = 0,
	RIPC_TUNNEL_INIT = 1,
	RIPC_TUNNEL_ACTIVE = 2,
	RIPC_TUNNEL_REMOVE_SESSION = 7
} ripcTunnelState;

typedef enum {
	RIPC_INT_WORK_NONE = 0,
	RIPC_INT_READ_THR = 0x01,
	RIPC_INT_SOCK_CLOSED = 0x02,
	RIPC_INT_SHTDOWN_PEND = 0x04
} ripcWorkState;

typedef enum {
	RIPC_INT_CS_FLAG_NONE = 0x0,
	RIPC_INT_CS_FLAG_BLOCKING = 0x1,
	RIPC_INT_CS_FLAG_TCP_NODELAY = 0x2,
	RIPC_INT_CS_FLAG_TUNNEL_NO_ENCRYPTION = 0x4
} ripcConnectSocketFlags;


typedef enum {
	RIPC_INT_ST_INACTIVE = 0,
	RIPC_INT_ST_READ_HDR = 1,
	RIPC_INT_ST_COMPLETE = 2,
	RIPC_INT_ST_ACTIVE = 3,
	RIPC_INT_ST_CONNECTING = 4,
	RIPC_INT_ST_ACCEPTING = 5,
	RIPC_INT_ST_WAIT_ACK = 6,
	RIPC_INT_ST_TRANSPORT_INIT = 11,
	RIPC_INT_ST_CLIENT_TRANSPORT_INIT = 12,
	RIPC_INT_ST_PROXY_CONNECTING = 13,
	RIPC_INT_ST_CLIENT_WAIT_PROXY_ACK = 14,
	RIPC_INT_ST_CLIENT_WAIT_HTTP_ACK = 15,  /* wininet client is waiting for our HTTP connection response with session ID */
	RIPC_INT_ST_WAIT_CLIENT_KEY = 16,  /* we have sent our server key exchange info, waiting on client side before we can go active */
	RIPC_INT_ST_SEND_CLIENT_KEY = 17,	/* client is in the third phase of handshake, needs to have a state to know it should send the client key if interrupted. */
	RIPC_INT_ST_WS_SEND_OPENING_HANDSHAKE = 18,	/* client needs to send the initial WebSocket handshake. This could 
												 * possibly be apart of _CLIENT_TRANSPORT_INIT. So, this_ ST may not be needed */
	RIPC_INT_ST_WS_WAIT_HANDSHAKE_RESPONSE = 19,	/* client is waiting/ready to receive the Servers WebSocket handshake */
	RIPC_INT_ST_WS_CLOSED_PENDING = 20			/* WebSocket session is waiting for a closed from it's peer */
} ripcIntState;

#if defined(_WIN32) 
#define RIPC_INVALID_SOCKET INVALID_SOCKET
#define ripcValidSocket(s) ((s) != RIPC_INVALID_SOCKET)
#else
#define RIPC_INVALID_SOCKET -1
#define ripcValidSocket(s) ((s) >= 0)
#endif

typedef struct {
	RsslQueue priorityQueue;
	RsslInt32 queueLength;
	rtr_msgb_t		*tempList[RIPC_MAXIOVLEN + 1];
	RsslInt32	tempIndex;
} RIPC_PRIORITY_WRITE;

typedef struct {
	RsslUInt32		connVersion;
	RsslUInt32		ipcVersion;
	RsslUInt16		dataHeaderLen;
	RsslUInt16		footerLen;
	RsslUInt16		firstFragHdrLen;
	RsslUInt16		subsequentFragHdrLen;
} RIPC_SESS_VERS;

typedef enum {
	RIPC_PROTO_SSL_NONE = 0,
	RIPC_PROTO_SSL_TLS_V1 = 0x1,
	RIPC_PROTO_SSL_TLS_V1_1 = 0x2,
	RIPC_PROTO_SSL_TLS_V1_2 = 0x4,
	RIPC_PROTO_SSL_TLS = 0x8		// Used for OpenSSLv1.1.X
} ripcSSLProtocolFlags;


typedef struct {

	int(*bindSrvr)(rsslServerImpl *srvr, RsslError *error);
	/*  Binds the server with the options set in the USER_SERVER structure,
	*	and opens up a listening port.
	*	Returns: < = on failure
	*			 = 0 on success
	*/

	void*   (*newSrvrConnection)(void * srvr, RsslSocket fd, int *initComplete, void* userSpecPtr, RsslError* error);
	/* A new server connection has been setup. Returns
	* the new transport information. Return of 0 implies
	* a failure. Upon successful return, initComplete tells
	* if the transport initialization is complete. If not,
	* the initializeTransport func must be called in order
	* to finish the transport initialization.
	*/

	RsslSocket(*connectSocket)(int *portnum, void *opts, int flags, void** userSpecPtr, RsslError *error);
	/* Creates the socket and returns the file descriptor -
	* for WinInet this will create the pipe and return it.
	* The userSpecPtr is for returning some additonal piece of information
	* that should be passed in on the newClientConnection call
	*/

	void*   (*newClientConnection)(RsslSocket fd, int *initComplete, void* userSpecPtr, RsslError* error);
	/* A new client connection has been setup. Returns
	* the new transport information. Return of 0 implies
	* a failure. Upon successful return, initComplete tells
	* if the transport initialization is complete. If not,
	* the initializeTransport func must be called in order
	* to finish the transport initialization.
	*/

	int(*initializeTransport)(void *transport, ripcSessInProg *inPr, RsslError *error);
	/* Continue with transport initialization.
	* Returns : < 0 on failure.
	*           = 0 on success, but transport init not complete.
	*           > 0 on success, transport init complete.
	*/

	int(*shutdownTransport)(void *transport);
	/* Shutdown the transport */

	int(*readTransport)(void *transport, char *buf, int max_len, ripcRWFlags flags, RsslError *error);
	/* Read from the transport.
	* Returns : < 0 on failure.
	*           = 0 on ewouldblock.
	*           > 0 is the number of bytes successfully read.
	*/

	int(*writeTransport)(void *transport, char *buf, int outLen, ripcRWFlags flags, RsslError *error);
	/* Write to the transport.
	* Returns : < 0 on failure.
	*           = 0 on ewouldblock.
	*           > 0 is the number of bytes successfully written.
	*/

	int(*writeVTransport)(void *transport, ripcIovType *iov,
		int iovcnt, int outLen, ripcRWFlags flags, RsslError *error);
	/* Write vector to the transport. This function may not be supported
	* Returns : < 0 on failure.
	*           = 0 on ewouldblock.
	*           > 0 is the number of bytes successfully written.
	*/

	int(*reconnectClient)(void *transport, RsslError *error);
	/* used for tunneling solutions to reconnect and bridge connections -
	this will keep connections alive through proxy servers */

	RsslSocket(*acceptSocket)(rsslServerImpl *srvr, void** userSpecPtr, RsslError *error);

	void(*shutdownSrvrError)(rsslServerImpl* srvr);
	/* Used to shutdown a server if an error happens during initialization */

	int(*sessIoctl)(void *transport, int code, int value, RsslError *error);
	/* used for tunneling solutions to modify select options */

	int (*getSockName)(RsslSocket fd, struct sockaddr *address, int *address_len, void* transport);
	int (*setSockOpts)(RsslSocket fd, ripcSocketOption *option, void* transport);
	int (*getSockOpts)(RsslSocket fd, int code, int* value, void* transport, RsslError *error);
	int (*connected)(RsslSocket fd, void* transport);
	int (*shutdownServer)(void *srvr);
	void (*uninitialize)();
} ripcTransportFuncs;

typedef struct {

	RsslInt32(*readTransportConnMsg)(void *, char *, int , ripcRWFlags , RsslError *);
	RsslInt32(*readTransportMsg)(void *, char *, int , ripcRWFlags , RsslError *);
	RsslInt32(*readPrependTransportHdr)(void *, char *, int, ripcRWFlags, int*, RsslError *);
	RsslInt32(*prependTransportHdr)(void *, rtr_msgb_t *, RsslError *);
	RsslInt32(*additionalTransportHdrLength)(); /* This is additional header length for transport protocols */
	rtr_msgb_t *(*getPoolBuffer)(rtr_bufferpool_t *, size_t);
	rtr_msgb_t *(*getGlobalBuffer)(size_t);

} ripcProtocolFuncs;


typedef struct {
	char	        *next_in;			/* Buffer to compress */
	unsigned int	avail_in;			/* Number of bytes to compress */
	char	        *next_out;			/* Buffer to place compressed data */
	unsigned long	avail_out;			/* Number of bytes left in compressed data buffer */

	int		bytes_in_used;		/* Upon return, the number of bytes used from next_in */
	int		bytes_out_used;		/* Upon return, the number of bytes used from next_out */
} ripcCompBuffer;

typedef struct {
	void*	(*compressInit)(int compressionLevel, int useInit2, RsslError*);
	void*	(*decompressInit)(int useInit2, RsslError*);
	void(*compressEnd)(void *compressInfo);
	void(*decompressEnd)(void *compressInfo);
	int(*compress)(void *compressInfo, ripcCompBuffer *buf, int resetContext, RsslError *error);
	int(*decompress)(void *compressInfo, ripcCompBuffer *buf, int resetContext, RsslError *error);
} ripcCompFuncs;

typedef struct {
	RsslQueueLink		link1;		/* It is used to add this type to RsslQueue freeServerSocketChannelList */
	RsslQueueLink		link2;		/* It is used to add this type to RsslQueue activeServerSocketChannelList */
	char		*serverName;		/* portName or port number */
	char		*interfaceName;		/* Inteface hostName or ip address */
	RsslUInt32	maxMsgSize;			/* Maximum Message Size */
	RsslUInt32	maxUserMsgSize;		/* Maximum User Message Size without IPC header */
	RsslUInt32	maxGuarMsgs;		/* Guar. number output messages per session */
	RsslUInt32	maxNumMsgs;			/* Max number of output messages per session */
	RsslUInt32	numInputBufs;		/* number of input buffers used to read in data */
	RsslUInt32  compressionSupported;	/* a bitmask of The types of compression supported by this server. 0 means no compression */
	RsslUInt32	zlibCompressionLevel;	/* compression level for zlib */
	RsslBool	forcecomp;			/* Force compression */
	RsslBool	server_blocking;	/* Perform server blocking operations */
	RsslBool	session_blocking;	/* Perform session blocking operations */
	RsslBool	tcp_nodelay;		/* Disable Nagle Algorithm */
	RsslInt32	connType;			/* Controls the connection type */
	RsslUInt32	rsslFlags;			/* this flag keeps track of client to server and server to client ping*/
	RsslUInt8	pingTimeout; 		/* ping timeout */
	RsslUInt8	minPingTimeout; 	/* minimum ping timeout */
	RsslUInt8	majorVersion;		/* major version of RSSL */
	RsslUInt8	minorVersion;		/* minor version of RSSL */
	RsslUInt8	protocolType;		/* protocol type, e.g RWF */
	rtr_bufferpool_t	*sharedBufPool;		/* Pointer to the buffer pool to be used */
	RsslMutex	*mutex;
	RsslUInt32 sendBufSize;
	RsslUInt32 recvBufSize;
	RsslBool	mountNak;
	RsslSocket	stream;
	RsslChannelState	state;		/* channel state */
	void		*rwsServer;
	
	/* Different Transport information */
	void			*transportInfo; /* For normal cases, the transport is
									* the OS calls read/write. For Secure
									* Sockets we use SSLRead/SSLWrite calls.
									*/
	char*			dhParams;
	char*			cipherSuite;
	RsslUInt32		encryptionProtocolFlags;
	char*			serverCert;
	char*			serverPrivateKey;
	RsslHttpCallback *httpCallback;
	RsslUserCookies	cookies;
} RsslServerSocketChannel;

#define RSSL_INIT_SERVER_SOCKET_Bind { 0, 0, 0, 0, 0, 0, 0, RSSL_COMP_NONE, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, RSSL_ENC_TLSV1_2, 0, 0 };

RTR_C_INLINE void rsslClearRsslServerSocketChannel(RsslServerSocketChannel *rsslServerSocketChannel)
{
	// save link to freeServerSocketChannelList
	RsslQueueLink* prevlink1 = rsslServerSocketChannel->link1.prev;
	RsslQueueLink* nextlink1 = rsslServerSocketChannel->link1.next;

	memset((void*)rsslServerSocketChannel, 0, sizeof(RsslServerSocketChannel));
	rsslServerSocketChannel->link1.prev = prevlink1;
	rsslServerSocketChannel->link1.next = nextlink1;
	rsslServerSocketChannel->compressionSupported = RSSL_COMP_NONE;
	rsslServerSocketChannel->protocolType = 0; // RIPC_RWF_PROTOCOL_TYPE;
	rsslServerSocketChannel->encryptionProtocolFlags = RSSL_ENC_TLSV1_2;
}

RSSL_RSSL_SOCKET_IMPL_FAST(void) relRsslServerSocketChannel(RsslServerSocketChannel* rsslServerSocketChannel);

/* This structure represents the function entry points for the different
* Secure Sockets Layer implementations.
* -- OpenSSL Secure Sockets
*/
typedef struct RsslSSLSrvrFuncs {

	void*	(*newSSLServer)(RsslServerSocketChannel* chnl, RsslError* err);
	/* A new server has been created */

	int(*freeSSLServer)(void *server, RsslError* err);
	/* Free the SSL server */

} ripcSSLFuncs;

ripcSSLFuncs*		getSSLTransFuncs();


typedef enum
{
	RSSL_CURL_ERROR = -1,
	RSSL_CURL_INACTIVE = 0,
	RSSL_CURL_ACTIVE = 1,
	RSSL_CURL_DONE = 2
} RsslLibCurlStatus;


typedef struct
{
	RsslError error;
	RsslInt32 curlThreadState;
	RsslThreadId curlThreadId;
    char *curlError;
} RsslLibcurlThreadInfo;

typedef enum
{
	RWS_ST_INIT			= 0,
	RWS_ST_CONNECTING	= 1,
	RWS_ST_ACTIVE		= 2,
	RWS_ST_CLOSING		= 3
} rwsConnState_t;

typedef struct headerLine {
	char		*data;		/* NULL terminates string of one HTTP header line */
	RsslBuffer field;		/* A pointer to the start of the header field and length */
	RsslBuffer value;		/* A pointer to the start of the field-value and length */
} headerLine_t;

typedef struct rwsHttpHdr {
	int				total;
	headerLine_t	*lines;
} rwsHttpHdr_t;

typedef struct
{
	RsslQueueLink	  	link1;					/* It is used to add this type to RsslQueue */
	char				*hostName;				/* hostName or ip address */
	char				*serverName;			/* portName or port number */
	char				*interfaceName;			/* local interface name to bind to */
	char				*objectName;			/* object name, used with tunneling */
	char				*proxyHostName;			/* hostname of the proxy */
	char				*proxyPort;				/* proxy port number */
	char				*curlOptProxyUser;		/* username used for tunneling connection */
	char				*curlOptProxyPasswd;	/* password used for tunneling connection */
	char				*curlOptProxyDomain;	/* domain used for tunneling connection */
	RsslBool			blocking : 1;			/* Perform blocking operations */
	RsslBool			tcp_nodelay : 1;		/* Disable Nagle Algorithm */
	RsslUInt32			compression;			/* Use compression defined by server, otherwise none */
	RsslUInt32			numConnections;			/* Number of concurrent connections for an extended line connection */
	RsslUInt32			numGuarOutputBufs;		/* Number of guaranteed output buffers */
	RsslUInt32			numMaxOutputBufs;		/* Maximum number of output buffers */
	RsslUInt32			numInputBufs;			/* number of input buffers used to read in data */
	RsslUInt32			pingTimeout;			/* ping timeout */
	RsslUInt32			rsslFlags;				/* rssl flag settings */
	RsslUInt32			dbgFlags;				/* debug flags */
	RsslInt32			connType;				/* Controls connection type. 0:Socket 1:ENCRYPTED 2:HTTP 3:ELSocket */
	RsslUInt8			majorVersion;
	RsslUInt8			minorVersion;
	RsslUInt8			protocolType;			/* protocol type, e.g RWF */
	rtr_bufferpool_t	*gblInputBufs;
	rtr_bufferpool_t		*bufPool;
	rtr_dfltcbufferpool_t	*guarBufPool;
	RsslMutex			*mutex;
	RsslUInt32			sendBufSize;
	RsslUInt32			recvBufSize;

	RsslUInt32 			encryptionProtocolFlags;

	/* SSL/TLS Encryption information */
	RsslUInt32 sslProtocolBitmap;		/* Represents the protocols supported by the dynamically loaded openSSL library */
	RsslUInt32 sslCurrentProtocol;	/* This is the current TLS protocol */
	RsslInt32			 sslEncryptedProtocolType;	/* Encrypted protocol type.  Currently either RSSL_CONN_TYPE_SOCKET or RSSL_CONN_TYPE_HTTP */
	char				 *sslCAStore;


	RsslInt32			ripcVersion;

	RsslSocket			newStream;				/* used for tunneling reconnection */

	RsslUInt32			mountNak : 1;

	RsslServerSocketChannel		*server;		/* The RsslServerSocketChannel for this channel */
	RsslUInt32			sessionID;				/* used for tunneling connection */
	RsslUInt8			intState;				/* internal state */
	RsslUInt8			workState;
	ripcCompFuncs		*compressFuncs;
	RsslUInt16			inDecompress;
	RsslCompTypes		outCompression;
	ripcCompFuncs		*inDecompFuncs;
	ripcCompFuncs		*outCompFuncs;
	void				*c_stream_in;
	void				*c_stream_out;			/* Compression stream information */

	RsslUInt32			httpHeaders : 1;		/* Is there http header information */
	RsslUInt32			isJavaTunnel : 1;		/* Is this a single-channel HTTP connection (i.e. from a java client) */
	RsslUInt32			sentControlAck : 1;		/* used for tunneling connection */

	RsslUInt32			zlibCompLevel;			/* compression level for zlib */
	RsslUInt32			lowerCompressionThreshold;			/* dont compress any buffers smaller than this */
	RsslUInt32			upperCompressionThreshold;			/* dont compress any buffers larger than this */
	RsslUInt32			high_water_mark;		/* used for the upper buffer usage threshold for this channel */
	RsslUInt32			safeLZ4 : 1;			/* limits LZ4 compression to only packets that wont span multiple buffers */

	ripcTransportFuncs	*transportFuncs; /* The transport functions to use */

	ripcProtocolFuncs	*protocolFuncs; /* The protocol Hdr functions to use */

	RsslUInt32			keyExchange : 1; /* indicates whether we should be exchanging encryption keys on the handshake */
	RsslUInt8			usingWinInet;	/* indicates whether we are using wininet - needed for proxy override */
	RsslUInt64			P;				/* key exchange p */
	RsslUInt64			G;				/* key exchange g */
	RsslUInt64			random_key;		/* calculated random key */
	RsslUInt64			shared_key;		/* calculated shared key */
	RsslUInt64			send_key;		/* key for sending; need to store on both sides */
	RsslUInt8			encryptionType; /* encryption type definitions */
	RIPC_PRIORITY_WRITE	priorityQueues[RIPC_MAX_PRIORITY_QUEUE];
	RsslInt8			flushStrategy[RIPC_MAX_FLUSH_STRATEGY + 1];  /* the flush strategy  */
	RsslInt32			currentOutList;		/* points into the flush strategy so we know which out list we are on */
	RsslInt8			compressQueue;      /* since we should only allow compression on one queue - or reinitialize
												to avoid potential zlib dictionary issues, we keep track of the first
												queue compression was done on and only allow it on that queue */
	RsslInt8			nextOutBuf;		/* used to keep track of next out buffer in case of partial write */

	rtr_msgb_t			*decompressBuf;		/* decompress buffer */
	rtr_msgb_t			*tempDecompressBuf;	/* temporary buffer to use when decompressing with compression types that dont effectively handle data growth (LZ4) */
	rtr_msgb_t			*tempCompressBuf;	/* temporary buffer to use when compressing with compression types that dont effectively handle data growth (LZ4) */
	RsslInt32			clientSession : 1;	/* identify if this is a client based session */
	RsslUInt32			maxMsgSize;
	RsslInt32			maxUserMsgSize;
	rtr_msgb_t			*inputBuffer;
	rtr_msgb_t			*curInputBuf;
	RsslUInt32			inputBufCursor;
	RsslUInt32			inBufProtOffset;    /* # of bytes in the inputBuffer related to the WebSocket protocol header length total */
	RsslInt32			readSize;
	RsslUInt32			bytesOutLastMsg;	/* # of bytes in the last sent message */
	RIPC_SESS_VERS		*version;			/* Session version information */
	rssl_pipe			sessPipe;			/* used for tunneling solutions for reconnection periods and initial handshake */
	RsslUInt32			ipAddress;
	RsslUInt16			pID;				/* process ID for tunneling */
	char				tunnelingState;		/* used for basic state management of connection handshake for tunneling management */
	void				*tunnelTransportInfo;	/* keeps track of the streaming transport Info for wininet based tunnel solutions */
	void				*newTunnelTransportInfo; /* keeps track of streaming transport info for winInet based tunnel solutions during the reconnect period */
	RsslSocket			oldTunnelStreamFd; /* This is the streamingFD of the old connection */
	RsslUInt8			compressionBitmap[RIPC_COMP_BITMAP_SIZE];	/* 1 byte array used in compression */
	RsslInt32			minPingTimeout;		/* minimum ping timeout */
	RsslUInt32			srvrcomp;			/* compression types allowed by the server */
	RsslUInt32			forcecomp : 1;		/* Force compression */
	RsslSocket			stream;
	RsslSocket			oldStream;			/* used for tunneling reconnection */
	RsslChannelState	state;				/* channel state */
	char				*clientHostname;
	char				*clientIP;
	char				*componentVer;		/* component version set by user - we dont own this memory */
	RsslUInt32			componentVerLen;	/* length of component version set by user */
	char				*outComponentVer;	/* component version from wire - we own this memory */	
	RsslUInt32			outComponentVerLen; /* length of component version from wire */
	CURL*				curlHandle;
	RsslLibcurlThreadInfo curlThreadInfo;

	rtr_msgb_t			*rwsLargeMsgBufferList;
	void				*rwsSession;
	rwsHttpHdr_t		*hsReceived;
	/* Different Transport information */
	void				*transportInfo;		/* For normal cases, the transport is
											* the OS calls read/write. For Secure
											* Sockets we use SSLRead/SSLWrite calls.
											*/
	void				*newTransportInfo;
	RsslUInt16			port;				/* port number that was used to connect to the server (for Consumer, NiProvider). */
	RsslHttpCallback    *httpCallback;		/* callback to provide http headers*/
	RsslUserCookies		cookies;			/* income user cookies */
#if (defined(_WINDOWS) || defined(_WIN32))
	RsslBool socketRowSet;
	MIB_TCPROW socketRow;
#endif
} RsslSocketChannel;


RTR_C_INLINE void ripcClearRsslSocketChannel(RsslSocketChannel *rsslSocketChannel)
{
	RsslInt32 i = 0;

	rsslSocketChannel->stream = RIPC_INVALID_SOCKET;
	rsslSocketChannel->newStream = RIPC_INVALID_SOCKET;
	rsslSocketChannel->intState = RIPC_INT_ST_INACTIVE;
	rsslSocketChannel->workState = RIPC_INT_WORK_NONE;

	rsslSocketChannel->mutex = 0;
	rsslSocketChannel->blocking = 0;
	rsslSocketChannel->mountNak = 0;
	rsslSocketChannel->inDecompress = 0;
	rsslSocketChannel->outCompression = (RsslCompTypes)0;
	rsslSocketChannel->inDecompFuncs = 0;
	rsslSocketChannel->outCompFuncs = 0;
	rsslSocketChannel->c_stream_out = 0;
	rsslSocketChannel->c_stream_in = 0;
	rsslSocketChannel->srvrcomp = 0;
	rsslSocketChannel->forcecomp = 0;
	rsslSocketChannel->httpHeaders = 0;
	rsslSocketChannel->isJavaTunnel = 0;
	rsslSocketChannel->sentControlAck = 0;
	/* Dont clear sessionID here - we want this to stay persistant for the life of the session */
	rsslSocketChannel->zlibCompLevel = 6;  /* set this to the default we want */
	rsslSocketChannel->lowerCompressionThreshold = 0;
	rsslSocketChannel->upperCompressionThreshold = 10000000;
	rsslSocketChannel->high_water_mark = 6000;
	rsslSocketChannel->safeLZ4 = 0;
	rsslSocketChannel->keyExchange = 0;
	rsslSocketChannel->transportFuncs = 0; 
	rsslSocketChannel->protocolFuncs = 0; 
	rsslSocketChannel->transportInfo = 0; 
	rsslSocketChannel->sendBufSize = 0;
	rsslSocketChannel->recvBufSize = 0;
	rsslSocketChannel->usingWinInet = 0;
	rsslSocketChannel->P = 0;
	rsslSocketChannel->G = 0;
	rsslSocketChannel->random_key = 0;
	rsslSocketChannel->shared_key = 0;
	rsslSocketChannel->send_key = 0;
	rsslSocketChannel->encryptionType = 0;
	rsslSocketChannel->numConnections = 0;

	for (i = 0; i < RIPC_MAX_PRIORITY_QUEUE; i++)
	{
		rsslInitQueue(&rsslSocketChannel->priorityQueues[i].priorityQueue);
		rsslSocketChannel->priorityQueues[i].queueLength = 0;
	}

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

	rsslSocketChannel->decompressBuf = 0;
	rsslSocketChannel->tempCompressBuf = 0;
	rsslSocketChannel->tempDecompressBuf = 0;
	rsslSocketChannel->objectName = 0;
	rsslSocketChannel->interfaceName = 0;
	rsslSocketChannel->proxyPort = 0;
	rsslSocketChannel->proxyHostName = 0;
	rsslSocketChannel->curlOptProxyUser = 0;
	rsslSocketChannel->curlOptProxyPasswd = 0;
	rsslSocketChannel->curlOptProxyDomain = 0;
	rsslSocketChannel->server = 0;
	rsslSocketChannel->serverName = 0;
	rsslSocketChannel->hostName = 0;
	rsslSocketChannel->connType = 0;
	rsslSocketChannel->clientSession = 0;
	rsslSocketChannel->maxMsgSize = 0;
	rsslSocketChannel->maxUserMsgSize = 0;
	rsslSocketChannel->pingTimeout = 0;
	rsslSocketChannel->rsslFlags = 0;
	rsslSocketChannel->protocolType = RIPC_RWF_PROTOCOL_TYPE;
	rsslSocketChannel->majorVersion = 0;
	rsslSocketChannel->minorVersion = 0;
	rsslSocketChannel->inputBuffer = 0;
	rsslSocketChannel->inputBufCursor = 0;
	rsslSocketChannel->inBufProtOffset = 0;
	rsslSocketChannel->curInputBuf = 0;
	rsslSocketChannel->readSize = 0;
	rsslSocketChannel->bytesOutLastMsg = 0;
	rsslSocketChannel->version = 0;
	rsslSocketChannel->ripcVersion = RIPC_VERSION_LATEST;
	rsslSocketChannel->guarBufPool = 0;
	rsslSocketChannel->bufPool = 0;
	rsslSocketChannel->shared_key = 0;
	rsslSocketChannel->encryptionType = 0;

	rssl_pipe_init(&rsslSocketChannel->sessPipe);
	rsslSocketChannel->ipAddress = 0;
	rsslSocketChannel->pID = 0;
	rsslSocketChannel->tunnelingState = RIPC_NO_TUNNELING;
	rsslSocketChannel->tunnelTransportInfo = 0;
	rsslSocketChannel->newTunnelTransportInfo = 0;
	rsslSocketChannel->oldTunnelStreamFd = RIPC_INVALID_SOCKET;

	rsslSocketChannel->dbgFlags = 0;

	for (i = 0; i<RIPC_COMP_BITMAP_SIZE; i++)
		rsslSocketChannel->compressionBitmap[i] = 0;

	rsslSocketChannel->oldStream = RIPC_INVALID_SOCKET;
	rsslSocketChannel->state = RSSL_CH_STATE_INACTIVE;

	rsslSocketChannel->clientHostname = 0;

	rsslSocketChannel->clientIP = 0;

	rsslSocketChannel->port = 0;

	rsslSocketChannel->outComponentVer = 0;
	rsslSocketChannel->outComponentVerLen = 0;
	rsslSocketChannel->componentVer = 0;
	rsslSocketChannel->componentVerLen = 0;
	rsslSocketChannel->curlHandle = 0;
	rsslSocketChannel->curlThreadInfo.curlThreadState = RSSL_CURL_INACTIVE;

	rsslSocketChannel->sslCurrentProtocol = RSSL_ENC_NONE;
	rsslSocketChannel->sslProtocolBitmap = RSSL_ENC_NONE;
	rsslSocketChannel->sslEncryptedProtocolType = 0;
	rsslSocketChannel->sslCAStore = 0;

	rsslSocketChannel->rwsSession = 0;
	rsslSocketChannel->rwsLargeMsgBufferList = 0;
	rsslSocketChannel->httpCallback = 0;

#if (defined(_WINDOWS) || defined(_WIN32))
	rsslSocketChannel->socketRowSet = RSSL_FALSE;
#endif
}


RSSL_RSSL_SOCKET_IMPL_FAST(void) ripcRelSocketChannel(RsslSocketChannel *rsslSocketChannel);

/* Session should be locked before call */
void rsslSocketChannelClose(RsslSocketChannel *rsslSocketChannel);

/* Contains code necessary for binding a listening socket */
RsslRet rsslSocketBind(rsslServerImpl* rsslSrvrImpl, RsslBindOptions *opts, RsslError *error );

/* Contains code necessary for accepting inbound socket connections to a listening socket */
rsslChannelImpl* rsslSocketAccept(rsslServerImpl *rsslSrvrImpl, RsslAcceptOptions *opts, RsslError *error);

/* Contains code necessary for creating an outbound socket connectioni */
RsslRet rsslSocketConnect(rsslChannelImpl* rsslChnlImpl, RsslConnectOptions *opts, RsslError *error);

/* Contains code necessary to reconnect socket connections and bridge data flow (mainly for HTTP/HTTPS)
RsslRet rsslSocketReconnect(rsslChannelImpl *rsslChnlImpl, RsslError *error); */

/* Contains code necessary for non-blocking socket connections (client or server side) to perform and complete handshake process */
RsslRet rsslSocketInitChannel(rsslChannelImpl* rsslChnlImpl, RsslInProgInfo *inProg, RsslError *error);

/* Contains code necessary to close a socket connection (client or server side) */
RsslRet rsslSocketCloseChannel(rsslChannelImpl* rsslChnlImpl, RsslError *error);

/* Contains code necessary to read from a socket connection (client or server side) */
RSSL_RSSL_SOCKET_IMPL_FAST(RsslBuffer*) rsslSocketRead(rsslChannelImpl* rsslChnlImpl, RsslReadOutArgs *readOutArgs, RsslRet *readRet, RsslError *error);

/* Contains code necessary to write/queue data going to a socket connection (client or server side) */
RSSL_RSSL_SOCKET_IMPL_FAST(RsslRet) rsslSocketWrite(rsslChannelImpl *rsslChnlImpl, rsslBufferImpl *rsslBufImpl, RsslWriteInArgs *writeInArgs, RsslWriteOutArgs *writeOutArgs, RsslError *error);

/* Contains code necessary to flush queued data to socket connection (client or server side) */
RSSL_RSSL_SOCKET_IMPL_FAST(RsslRet) rsslSocketFlush(rsslChannelImpl *rsslChnlImpl, RsslError *error);

/* Contains code necessary to obtain a buffer to put data in for writing to socket connection (client or server side) */
RSSL_RSSL_SOCKET_IMPL_FAST(rsslBufferImpl*) rsslSocketGetBuffer(rsslChannelImpl *rsslChnlImpl, RsslUInt32 size, RsslBool packedBuffer, RsslError *error);

/* Contains code necessary to release an unused/unsuccessfully written buffer to socket connection (client or server) */
RSSL_RSSL_SOCKET_IMPL_FAST(RsslRet) rsslSocketReleaseBuffer(rsslChannelImpl *rsslChnlImpl, rsslBufferImpl *rsslBufImpl, RsslError *error);

/* Contains code necessary to query number of used output buffers for socket connection (client or server side) */
RSSL_RSSL_SOCKET_IMPL_FAST(RsslRet) rsslSocketBufferUsage(rsslChannelImpl *rsslChnlImpl, RsslError *error);

/* Contains code necessary to query number of used buffers by the server (shared pool buffers typically (server only) */
RSSL_RSSL_SOCKET_IMPL_FAST(RsslRet) rsslSocketSrvrBufferUsage(rsslServerImpl *rsslSrvrImpl, RsslError *error);

/* Contains code necessary for buffer packing with socket connection buffer (client or server side) */
RSSL_RSSL_SOCKET_IMPL_FAST(RsslBuffer*) rsslSocketPackBuffer(rsslChannelImpl *rsslChnlImpl, rsslBufferImpl *rsslBufImpl, RsslError *error);

/* Contains code necessary to send a ping message (or flush queued data) on socket connection (client or server side) */
RSSL_RSSL_SOCKET_IMPL_FAST(RsslRet) rsslSocketPing(rsslChannelImpl *rsslChnlImpl, RsslError *error);

/* Contains code necessary to query socket channel for more detailed connection info (client or server side) */
RSSL_RSSL_SOCKET_IMPL_FAST(RsslRet) rsslSocketGetChannelInfo(rsslChannelImpl *rsslChnlImpl, RsslChannelInfo *info, RsslError *error);

/* Contains code necessary to query server for more detailed connection info (server only) */
RSSL_RSSL_SOCKET_IMPL_FAST(RsslRet) rsslSocketGetSrvrInfo(rsslServerImpl *rsslSrvrImpl, RsslServerInfo *info, RsslError *error);

/* Contains code necessary to do ioctl on a client (client side only) */
RSSL_RSSL_SOCKET_IMPL_FAST(RsslRet) rsslSocketIoctl(rsslChannelImpl *rsslChnlImpl, RsslIoctlCodes code, void *value, RsslError *error);

/* Contains code necessary to do ioctl on a server socket (server side only) */
RSSL_RSSL_SOCKET_IMPL_FAST(RsslRet) rsslSocketSrvrIoctl(rsslServerImpl *rsslSrvrImpl, RsslIoctlCodes code, void *value, RsslError *error);

/* Contains code necessary to cleanup the server socket channel */
RSSL_RSSL_SOCKET_IMPL_FAST(RsslRet) rsslCloseSocketSrvr(rsslServerImpl *rsslSrvrImpl, RsslError *error);

/* Socket Channel functions for active WS connections using a non-RWF protocol */
/* Contains code necessary to close a socket connection (client or server side) */
RsslRet rsslWebSocketCloseChannel(rsslChannelImpl* rsslChnlImpl, RsslError *error);

/* Contains code necessary to read from a socket connection (client or server side) */
RSSL_RSSL_SOCKET_IMPL_FAST(RsslBuffer*) rsslWebSocketRead(rsslChannelImpl* rsslChnlImpl, RsslReadOutArgs *readOutArgs, RsslRet *readRet, RsslError *error);

/* Contains code necessary to write/queue data going to a socket connection (client or server side) */
RSSL_RSSL_SOCKET_IMPL_FAST(RsslRet) rsslWebSocketWrite(rsslChannelImpl *rsslChnlImpl, rsslBufferImpl *rsslBufImpl, RsslWriteInArgs *writeInArgs, RsslWriteOutArgs *writeOutArgs, RsslError *error);

/* Contains code necessary to obtain a buffer to put data in for writing to socket connection (client or server side) */
RSSL_RSSL_SOCKET_IMPL_FAST(rsslBufferImpl*) rsslWebSocketGetBuffer(rsslChannelImpl *rsslChnlImpl, RsslUInt32 size, RsslBool packedBuffer, RsslError *error);

/* Contains code necessary for buffer packing with socket connection buffer (client or server side) */
RSSL_RSSL_SOCKET_IMPL_FAST(RsslBuffer*) rsslWebSocketPackBuffer(rsslChannelImpl *rsslChnlImpl, rsslBufferImpl *rsslBufImpl, RsslError *error);

/* Contains code necessary to send a ping message (or flush queued data) on socket connection (client or server side) */
RSSL_RSSL_SOCKET_IMPL_FAST(RsslRet) rsslWebSocketPing(rsslChannelImpl *rsslChnlImpl, RsslError *error);

RSSL_RSSL_SOCKET_IMPL_FAST(RsslRet) rsslSocketGetChannelStats(rsslChannelImpl *rsslSocketChannel, RsslChannelStats* stats, RsslError *error);



RsslInt32 ipcSessSetMode(RsslSocket sock_fd, RsslInt32 blocking, RsslInt32 tcp_nodelay, RsslError *error, RsslInt32 line);

RsslInt32 getProtocolNumber();

RsslBool getCurlDebugMode();

RsslUInt8 getConndebug();

#define IPC_NULL_PTR(__ptr,__fnm,__ptrname,__err) \
	(( __ptr == 0 ) ? ipcNullPtr((char*)(__fnm == 0?__FUNCTION__:__fnm),__ptrname,__FILE__,__LINE__,__err) : 0 )

rtr_msgb_t * ipcAllocGblMsg(size_t length);
rtr_msgb_t * ipcDupGblMsg(rtr_msgb_t * buffer);
void _rsslBufferMap(rsslBufferImpl *buffer, rtr_msgb_t *ripcBuffer);
rtr_msgb_t *ipcDataBuffer(RsslSocketChannel *rsslSocketChannel, RsslInt32 size, RsslError *error);
extern RsslRet ipcNullPtr(char*, char*, char*, int, RsslError*);
extern rtr_msgb_t *ipcIntReadSess(RsslSocketChannel*, RsslRet*, int*, int*, int*, int*, int*, int*, RsslError*);
extern ripcSessInit ipcIntSessInit(RsslSocketChannel*, ripcSessInProg*, RsslError*);
extern RsslRet ipcFlushSession(RsslSocketChannel*, RsslError*);

extern ripcSessInit ipcProcessHdr(RsslSocketChannel*, ripcSessInProg*, RsslError*, int cc);
ripcSessInit ipcConnecting(RsslSocketChannel *, ripcSessInProg *, RsslError *);
extern ripcSessInit ipcWaitAck(RsslSocketChannel*, ripcSessInProg*, RsslError*, char*, int);
extern RsslRet ipcSetCompFunc(RsslInt32, ripcCompFuncs*);
ripcCompFuncs *ipcGetCompFunc(RsslInt32);
RsslRet ipcSetSocketChannelProtocolHdrFuncs(RsslSocketChannel * , RsslInt32 );
RsslRet ipcSetProtocolHdrFuncs(RsslInt32 , ripcProtocolFuncs *);
extern RsslRet ipcSetTransFunc(int, ripcTransportFuncs*);
extern RsslRet ipcSetSSLFuncs(ripcSSLFuncs*);
extern RsslRet ipcSetSSLTransFunc(int, ripcTransportFuncs*);
extern RsslRet ipcLoadOpenSSL(RsslError *error);

RsslInt32 ipcReleaseDataBuffer(RsslSocketChannel *, rtr_msgb_t *, RsslError *);
rtr_msgb_t *ipcGetPoolBuffer(rtr_bufferpool_t *, size_t );
rtr_msgb_t *ipcGetGlobalBuffer(RsslInt32 );
RsslInt32 ipcReadTransportMsg(void *, char *, int , ripcRWFlags , RsslError *);
RsslInt32 ipcReadPrependTransportHdr(void *, char *, int, ripcRWFlags, int *, RsslError *);
RsslInt32 ipcPrependTransportHdr(void *, rtr_msgb_t *, RsslError *);
RsslInt32 ipcAdditionalHeaderLength();

extern RsslRet ipcShutdownServer(RsslServerSocketChannel* socket, RsslError *error);
extern RsslRet ipcSrvrDropRef(RsslServerSocketChannel *rsslServerSocketChannel, RsslError *error);
extern void ipcCloseActiveSrvr(RsslServerSocketChannel *rsslServerSocketChannel);

extern void ipcGetOfServerSocketChannelCounters(rsslServerCountersInfo* serverCountersInfo);

// Contains code necessary to set the debug func pointers for Socket transport
RsslRet rsslSetSocketDebugFunctions(
	void(*dumpIpcIn)(const char *functionName, char *buffer, RsslUInt32 length, RsslUInt64 opaque), 
	void(*dumpIpcOut)(const char *functionName, char *buffer, RsslUInt32 length, RsslUInt64 opaque),
	void(*dumpRsslIn)(const char *functionName, char *buffer, RsslUInt32 length, RsslSocket socketId),
	void(*dumpRsslOut)(const char *functionName, char *buffer, RsslUInt32 length, RsslSocket socketId),
	RsslError *error);

RsslRet rsslSetWebSocketDebugFunctions(
	void(*dumpIpcIn)(const char *functionName, char *buffer, RsslUInt32 length, RsslUInt64 opaque),
	void(*dumpIpcOut)(const char *functionName, char *buffer, RsslUInt32 length, RsslUInt64 opaque),
	void(*dumpRsslIn)(const char *functionName, char *buffer, RsslUInt32 length, RsslSocket socketId),
	void(*dumpRsslOut)(const char *functionName, char *buffer, RsslUInt32 length, RsslSocket socketId),
	RsslError *error);

RSSL_THREAD_DECLARE(testBlocking, threadStruct);

#ifdef __cplusplus
};
#endif


#endif
