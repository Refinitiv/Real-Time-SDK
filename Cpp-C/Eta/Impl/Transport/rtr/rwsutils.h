/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2020 Refinitiv. All rights reserved.              --
 *|-----------------------------------------------------------------------------
 */

#ifndef __rwsutils_h
#define __rwsutils_h

#include <stdio.h>

#include "rtr/os.h"
#include "rtr/ripch.h"
#include "rtr/ripcutils.h"
#include "rtr/rsslTransport.h"
#include "rtr/rsslSocketTransportImpl.h"
#include "rtr/rsslErrors.h"
#ifdef _MSC_VER 
	#ifndef strncasecmp
		#define strncasecmp _strnicmp
	#endif
	#define strtok_r	strtok_s
	#define strdup		_strdup
	#ifndef strcasecmp
		#define strcasecmp _stricmp
	#endif
#endif

/*
 *       0                   1                   2                   3
 *       0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
 *       +-+-+-+-+-------+-+-------------+-------------------------------+
 *       |F|R|R|R| opcode|M| Payload len |    Extended payload length    |
 *       |I|S|S|S|  (4)  |A|     (7)     |             (16/64)           |
 *       |N|V|V|V|       |S|             |   (if payload len==126/127)   |
 *       | |1|2|3|       |K|             |                               |
 *       +-+-+-+-+-------+-+-------------+ - - - - - - - - - - - - - - - +
 *       |     Extended payload length continued, if payload len == 127  |
 *       + - - - - - - - - - - - - - - - +-------------------------------+
 *       |                               |Masking-key, if MASK set to 1  |
 *       +-------------------------------+-------------------------------+
 *       | Masking-key (continued)       |          Payload Data         |
 *       +-------------------------------- - - - - - - - - - - - - - - - +
 *       :                     Payload Data continued ...                :
 *       +---------------------------------------------------------------+
 *       */
#define _WS_BIT_POS_FIN					7
#define _WS_BIT_POS_RSV1				6
#define _WS_BIT_POS_RSV2				5
#define _WS_BIT_POS_RSV3				4

#define _WS_OPCODE_MASK					0x0F

#define _WS_BIT_POS_MASKKEY				7

#define _WS_PAYLOAD_LEN_MASK			0x7F

#define _WS_FLAG_2BYTE_EXT_PAYLOAD		126
#define _WS_FLAG_8BYTE_EXT_PAYLOAD		127

/* The CONTROL_HEADER the start of the packet up too and including the payload 7 bit length */
#define _WS_CONTROL_HEADER_LEN			2

#define _WS_PAYLOAD_LEN_OFFSET			1 /* the MSbit is the mask flag */
#define _WS_EXTENDED_PAYLOAD_OFFSET		2 /* for 2 and 8 byte payload lengths */

#define _WS_2BYTE_EXT_PAYLOAD			2
#define _WS_126PAYLOAD_FIELD_LEN		_WS_2BYTE_EXT_PAYLOAD
#define _WS_8BYTE_EXT_PAYLOAD			8
#define _WS_127PAYLOAD_FIELD_LEN		_WS_8BYTE_EXT_PAYLOAD

#define _WS_MASK_KEY_FIELD_LEN			4

#define _WS_MASK_KEY_126PAY_OFFSET		(_WS_CONTROL_HEADER_LEN + _WS_126PAYLOAD_FIELD_LEN)
#define _WS_MASK_KEY_127PAY_OFFSET		(_WS_CONTROL_HEADER_LEN + _WS_127PAYLOAD_FIELD_LEN)

#define _WS_MIN_HEADER_LEN				_WS_CONTROL_HEADER_LEN
#define _WS_126_HEADER_LEN				(_WS_MIN_HEADER_LEN + _WS_126PAYLOAD_FIELD_LEN) /* 2 + 2 */
#define _WS_127_HEADER_LEN				(_WS_MIN_HEADER_LEN + _WS_127PAYLOAD_FIELD_LEN) /* 2 + 8 */

#define _WS_MAX_HEADER_LEN				(_WS_127_HEADER_LEN + _WS_MASK_KEY_FIELD_LEN) /* 2 + 8 + 4 */

#define _WS_MAX_CONTROL_FRAME			(_WS_FLAG_2BYTE_EXT_PAYLOAD - 1)

#define RWS_MIN_HEADER_LEN				_WS_MIN_HEADER_LEN 
#define RWS_MASK_KEY_LEN				_WS_MASK_KEY_FIELD_LEN

#define RWS_MAX_HEADER_SIZE				_WS_MAX_HEADER_LEN

#define RWS_PROTOCOL_VERSION			13

#define RWS_RESP_BUFFER_SIZE			1024
#define RWS_MAX_HTTP_HEADER_SIZE		32768	/* Browsers support 16K - 48K HTTP msg lengths */
#define RWS_MAX_MSG_SIZE				65535   /* TODO Determine what the supported max message size will be, 6 or 8 byte max */

#define RWS_CONT_FRAME_SIZE				RSSL_MAX_MSG_SIZE

#define RWS_PROT_JSON2_HEADER_SIZE		2
#define RWS_PROT_RWF_HEADER_SIZE		4

#define RWS_MAX_CONTROL_FRAME_SIZE		_WS_MAX_CONTROL_FRAME

//close frame status codes 
typedef enum {
	/* UNDEFINED */
	RWS_CFSC_NORMAL_UNDEFINED = 0,
	/* 1000 */
	RWS_CFSC_NORMAL_CLOSE = 1000,
	/* 1001 */
	RWS_CFSC_ENDPOINT_GONE = 1001,
	/* 1002 */
	RWS_CFSC_ENDPOINT_PROTOCOL_ERROR = 1002,
	/* 1004 */
	RWS_CFSC_INVALID_FRAME_DATA_TYPE = 1003,
	/* 1004 */
	RWS_CFSC_UNKNOWN_4 = 1004,
	/* 1005 */
	RWS_CFSC_UNKNOWN_5 = 1005,
	/* 1006 */
	RWS_CFSC_UNKNOWN_6 = 1006,
	/* 1007 */
	RWS_CFSC_INVALID_CONT_FRAME_DATA = 1007,
	/* 1008 */
	RWS_CFSC_INVALID_FRAME = 1008,
	/* 1009 */
	RWS_CFSC_INVALID_FRAME_SIZE = 1009,
	/* 1010 */
	RWS_CFSC_INVALID_SERVER_EXTENSION = 1010,
	/* 1011 */
	RWS_CFSC_UNEXPECTED_EVENT = 1011,
	/* 1015 */
	RWS_CFSC_UNKNOWN_15 = 1015,

	RWS_CFSC_MAX_CODE = 4999
	
} rwsCFStatusCodes_t; 

static const char * rwsCloseFrameText[] = {
	/* 1000 indicates a normal closure, meaning that the purpose for which the connection was established has been fulfilled.", */
	"Normal Closure",
	/* 1001 indicates that an endpoint is \"going away\", such as a server going down or a browser having navigated away from a page.",*/
	"Going Away",
	/* 1002 indicates that an endpoint is terminating the connection due to a protocol error.", */
	"Protocol Error",
	/* 1003 indicates that an endpoint is terminating the connection because it has received a type of data it cannot accept 
	 * (e.g., an endpoint that understands only text data MAY send this if it receives a binary message).",*/
	"Unsupported Data",
	/* 1004 "Reserved. The specific meaning might be defined in the future.",*/
	"Reserved",
	/* 1005 is a reserved value and MUST NOT be set as a status code in a Close control frame by an endpoint. It is designated 
	 * for use in applications expecting a status code to indicate that no status code was actually present.",*/
	"No Status Received",
	/* 1006 is a reserved value and MUST NOT be set as a status code in a Close control frame by an endpoint. It is designated 
	 * for use in applications expecting a status code to indicate that the connection was closed abnormally, e.g., without 
	 * sending or receiving a Close control frame.",*/
	"Abnormal Closure",
	/* 1007 indicates that an endpoint is terminating the connection because it has received data within a message that was 
	 * not consistent with the type of the message (e.g., non-UTF-8 [RFC3629] data within a text message).", */
	"Invalid frame payload data",
	/* 1008 indicates that an endpoint is terminating the connection because it has received a message that violates its policy. 
	 * This is a generic status code that can be returned when there is no other more suitable status code (e.g., 1003 or 1009) 
	 * or if there is a need to hide specific details about the policy.", */
	"Policy Violation",
	/* 1009 indicates that an endpoint is terminating the connection because it has received a message that is too big for it 
	 * to process.", */
	"Message Too Big",
	/* 1010 indicates that an endpoint (client) is terminating the connection because it has expected the server to negotiate 
	 * one or more extension, but the server didn’t return them in the response message of the WebSocket handshake. The list 
	 * of extensions that are needed SHOULD appear in the /reason/ part of the Close frame.  Note that this status code is not 
	 * used by the server, because it can fail the WebSocket handshake instead.", */
	"Mandatory Extension",
	/* 1011 indicates that a server is terminating the connection because it encountered an unexpected condition that prevented 
	 * it from fulfilling the request.", */
	"Internal Server Error",
	/* 1012 */
	"undefined",
	/* 1013 */
	"undefined",
	/* 1014 */
	"undefined",
	/* 1015 is a reserved value and MUST NOT be set as a status code in a Close control frame by an endpoint. It is designated 
	 * for use in applications expecting a status code to indicate that the connection was closed due to a failure to perform 
	 * a TLS handshake" */
	"TLS Handshake"
};

typedef enum {
	RWS_COMPF_NONE = 0x0, /* None */
	/* Don't maintain compression context on messages we send (no outbound 'context takeover'). */
	RWS_COMPF_DEFLATE_NO_OUTBOUND_CONTEXT = 0x1,
	/* Don't maintain compression context on messages we send (no outbound 'context takeover'). */
	RWS_COMPF_DEFLATE_NO_INBOUND_CONTEXT = 0x2 
} rwsCompFlags_t;

#define RWS_DEFAULT_SUBPROTOCOL			"tr_json2"
#define RWS_DEFAULT_SUBPROTOCOL_LIST	"rssl.json.v2, rssl.rwf, tr_json2"

typedef enum {
	RWS_SP_NONE   = -1,
	RWS_SP_RWF    = RIPC_RWF_PROTOCOL_TYPE, // set = #define RIPC_JSON_PROTOCOL_TYPE 2 
	RWS_SP_JSON2  = RIPC_JSON_PROTOCOL_TYPE
} rwsSubProtocol_t;
#define RWS_SP_MAX   2

/* WebSocket Frame Masks */
typedef enum {
	RWS_FIN		= 0x8000,		/* Final Fragment */
	RWS_COMP	= 0x4000,		/* Compressed packet */ 
	RWS_OPCODE	= 0x0F00,		/* OpCode See Below */
	RWS_MASK	= 0x0080,		/* Is the payload masked by Masking-key */
	RWS_LEN		= 0x007F		/* Length if < 126 otherwise indicates length size 
									 * 126 == 16 bits
									 * 127 == 64 bits */
} rwsMasks_t;

/* WebSocket Frame OpCodes */
typedef enum {
	RWS_CONT	= 0x000,	/* Continuation Frame */
	RWS_TEXT	= 0x100,	/* Teaxt Frame */
	RWS_BINARY	= 0x200,	/* Binary Frame */
	RWS_CLOSE	= 0x800,	/* Connection Closed Frame */
	RWS_PING	= 0x900,	/* Ping Frame */
	RWS_PONG	= 0xa00		/* Pong Frame */
} rwsOpCodeU16Mask_t;

#define _WS_OPC_NONE	-1
#define _WS_OPC_CONT	0
#define _WS_OPC_TEXT	1
#define _WS_OPC_BINARY	2
#define _WS_OPC_CLOSE	8
#define _WS_OPC_PING	9
#define _WS_OPC_PONG	10

typedef enum {
	RWS_OPC_NONE	= _WS_OPC_NONE,		/* Undefined OpCode */
	RWS_OPC_CONT	= _WS_OPC_CONT,		/* Continuation Frame */
	RWS_OPC_TEXT	= _WS_OPC_TEXT,		/* Text Frame */
	RWS_OPC_BINARY	= _WS_OPC_BINARY,	/* Binary Frame */
	RWS_OPC_CLOSE	= _WS_OPC_CLOSE,	/* Connection Closed Frame */
	RWS_OPC_PING	= _WS_OPC_PING,		/* Ping Frame */
	RWS_OPC_PONG	= _WS_OPC_PONG		/* Pong Frame */
} rwsOpCodes_t;


typedef enum {
	RSSL_WS_REJECT_CONN_ERROR = 1,
	RSSL_WS_REJECT_AUTH_FAIL = 2,
	RSSL_WS_REJECT_NO_SESS = 3,
	RSSL_WS_REJECT_NO_RESRC = 4,
	RSSL_WS_REJECT_REQUEST_TOO_LARGE = 5,
	RSSL_WS_REJECT_UNSUPPORTED_VERSION = 6,
	RSSL_WS_REJECT_UNSUPPORTED_SUB_PROTOCOL = 7
} RsslRejectCodeType;

#define RWS_IS_CONTROL_FRAME(code)\
	((code == RWS_OPC_CLOSE) || (code == RWS_OPC_PING) || (code == RWS_OPC_PONG))

typedef struct rwsSubProtocol {
	rwsSubProtocol_t		protocol;
	RsslBuffer				oldProtocolName;
	RsslBuffer				protocolName;
} rwsSubProtocolList_t;

typedef struct rwsCookies {
	RsslBuffer		authToken;		/* xxxxxxxxxxxxxxx */
	RsslBuffer		position;			/* xxxxxxxxxxxxxxx */
	RsslBuffer		applicationId;		/* xxxxxxxxxxxxxxx */
} rwsCookies_t;

typedef struct rwsCompression {
	RsslUInt32		type;
	rwsCompFlags_t	flags;
	ripcCompFuncs	*compressFuncs;
	RsslUInt16		inDecompress;
	RsslCompTypes	outCompression;
	ripcCompFuncs	*inDecompFuncs;
	ripcCompFuncs	*outCompFuncs;
	void			*c_stream_in;
	void			*c_stream_out;			/* Compression stream information */
	RsslUInt32		zlibLevel;			/* compression level for zlib */
	rtr_msgb_t		*decompressBuf;				/* decompress buffer */
} rwsComp_t;

typedef struct rwsFrameHdr {
	char		buffer[RWS_MAX_HEADER_SIZE];
	char		*pCtlHdr;
	char		*pExtHdr;
	RsslInt32	cursor;
	RsslInt32	hdrLen;
	RsslInt32	extHdrLen;
	RsslBool	partial;
	RsslBool	finSet;
	RsslBool	rsv1Set;
	RsslBool	rsv2Set;
	RsslBool	rsv3Set;
	RsslUInt16	opcode;
	RsslUInt16	dataType;
	RsslBool	control;
	RsslBool	fragment;
	RsslBool	compressed;
	RsslBool	maskSet;
	char		mask[RWS_MASK_KEY_LEN];
	RsslUInt32	maskVal;
	RsslUInt64	payloadLen;
	char		*payload;
	RsslBool	advancedInputCursor; /* A flag to keep track whether the inputBufCursor has been advanced beyond the frame header. */
} rwsFrameHdr_t;

typedef struct rwsHandshake {
	char			buffer[RWS_MAX_MSG_SIZE];
	rwsHttpHdr_t	hsReceived;
	RsslInt32		headerLineNum;
	RsslInt32		httpHeaderComplete;
	RsslBool		upgrade;
	RsslBool		connUpgrade;
	RsslBool		deflate;
	RsslBool		compress;
	rwsCompFlags_t	compFlags;
	RsslUInt16		perMsgContext;
	RsslInt32		protocol;
	RsslBuffer		*url;
	RsslBuffer		*host;
	RsslBuffer		*port;
	RsslBuffer		*origin;
	RsslBuffer		*userAgent;
	RsslBuffer		*reqProtocols;
	RsslBuffer		*wsKeyRecvd;
	RsslBuffer		*wsKey;
	RsslBuffer		*cookie;
	RsslInt32		version;
} rwsHandshake_t;

typedef struct rwsServer {
	//TODO RsslQueue		*protocolList;
	char				*protocolList;
	RsslUInt32			compressionSupported;
	RsslUInt32			zlibCompLevel;
	rwsCookies_t		cookies;
	RsslInt32			version;
} rwsServer_t;

typedef struct rwsSession {
	rwsServer_t		*server;
	RsslUInt64      actualInBuffLen;
	RsslUInt64      inputReadCursor;
	rwsHttpHdr_t	hsReceived;
	RsslInt32		headerLineNum;
	RsslInt32		statusCode;
	RsslBool		upgrade;
	RsslBool		connUpgrade;
	RsslBool		deflate;
	RsslBool		compressed;
	RsslInt32		protocol;
	char			*url;
	char			*host;
	char			*port;
	char			*hostname;
	char			*peerIP;
	char			*origin;
	char			*userAgent;
	char			*protocolName;
	char			*protocolList;
	rwsCookies_t	cookies;
	RsslBuffer		keyRecv;
	RsslBuffer		keyAccept;
	RsslBuffer		keyNonce;
	RsslInt32		versionRecv;
	RsslInt32		version;
	// end hs
	rwsFrameHdr_t	frameHdr;
	RsslUInt32		mask;
	char			maskFld[_WS_MASK_KEY_FIELD_LEN];
	rwsComp_t		comp;
	RsslUInt32		maxPayload;
	rtr_msgb_t		*reassemblyBuffer;
	RsslBool		reassemblyUnfinished;
	RsslBool		reassemblyCompressed;
	RsslBool		recvGetReq;
	RsslBool		recvClose;
	RsslBool		sentClose;
	RsslBool		pingRecvd;
	RsslBool		sendPong;
	RsslBool		isClient;
	RsslBool		finBit; /* For supporting fragmented message */
	RsslUInt64		maxMsgSize; /* Stores the maximum message size for WebSocket client */
} rwsSession_t;

typedef struct 
{
	char* hostName;
	char* cookie;
	RsslInt32 portNum;
	RsslInt32 blocking;
	RsslUInt32 address;
	RsslUInt32 id;
	RsslUInt32 connectionFlags;
	RsslUInt16 pID; 
} rwsConnectOpts_t;

rwsSubProtocol_t rwsValidateSubProtocolResponse(rwsSession_t *, RsslBuffer *, RsslBool, RsslError *);
rwsSubProtocol_t rwsValidateSubProtocolRequest(rwsSession_t * , const char* , RsslBuffer *, RsslBool , RsslError *error);
char * rwsSetSubProtocols(const char *, RsslBool , RsslError *);
rtr_msgb_t *rwsDataBuffer(RsslSocketChannel *, size_t, RsslError *);
rtr_msgb_t *rwsGetPoolBuffer(rtr_bufferpool_t *, size_t );
rtr_msgb_t *rwsGetSimpleBuffer(size_t );

RsslInt32 checkInputBufferSpace(RsslSocketChannel *, size_t );
RsslInt32 rwsIntTotalUsedOutputBuffers(RsslSocketChannel *rsslSocketChannel, RsslError *error);

rtr_msgb_t *checkSizeAndRealloc(rtr_msgb_t*, size_t, size_t, RsslError *);
rtr_msgb_t *doubleSizeAndRealloc(rtr_msgb_t*, size_t, size_t, RsslError *);

void rwsReleaseLargeBuffer(RsslSocketChannel *, rtr_msgb_t *);

RsslInt32 rwsReadHttpHeader(char *, RsslInt32, RsslInt32, rwsSession_t *, rwsHttpHdr_t *, RsslError *);

RsslRet rwsReadOpeningHandshake(char *, RsslInt32 , RsslInt32 , RsslSocketChannel * , RsslError *);
RsslInt32 rwsReadResponseHandshake(char *, RsslInt32 , RsslInt32 , rwsSession_t * , RsslError *);
ripcSessInit rwsSendOpeningHandshake(RsslSocketChannel * , ripcSessInProg *, RsslError *);
ripcSessInit rwsWaitResponseHandshake(RsslSocketChannel * , ripcSessInProg *, RsslError *);

RsslInt32 rwsSendResponseHandshake(RsslSocketChannel *, rwsSession_t *, RsslError *);
ripcSessInit rwsValidateWebSocketRequest(RsslSocketChannel *, char *, RsslInt32, RsslError *);
ripcSessInit rwsAcceptWebSocket(RsslSocketChannel *, RsslError *);
RsslInt32 rwsRejectSession(RsslSocketChannel *, RsslRejectCodeType , RsslError *);

/* creates and initializes new client/session structure */
rtr_msgb_t *rwsReadWebSocket(RsslSocketChannel *, RsslRet *, RsslInt32 *, RsslInt32*, RsslInt32*, RsslInt32 *, RsslError *);
RsslInt32 rwsReadWsConnMsg(void *, char *, int , ripcRWFlags , RsslError *);
RsslInt32 rwsReadTransportMsg(void *, char * , int, ripcRWFlags , RsslError *error);
RsslInt32 rwsReadPrependTransportHdr(void *, char *, int, ripcRWFlags, int *,RsslError *error);
RsslInt32 rwsAdditionalHeaderLength();
RsslRet rwsWriteWebSocket(RsslSocketChannel *, rsslBufferImpl *, RsslInt32, RsslInt32 *, RsslInt32 *, RsslInt32, RsslError *);
RsslInt32 rwsWriteAndFlush(RsslSocketChannel *, rtr_msgb_t *, int *, RsslError *);
RsslUInt8 rwsGetWsHdrSize(RsslUInt64 , RsslInt32 );
RsslUInt8 rwsWriteWsHdrBuffer(char * , RsslUInt64 , rwsSession_t *, RsslBool, RsslInt32 , rwsOpCodes_t );
RsslUInt8 rwsWriteWsHdr(rtr_msgb_t * msgb, rtr_msgb_t * msgb2, rwsSession_t * wsSess, RsslInt32 compressed, rwsOpCodes_t opCode);
RsslInt32 rwsPrependWsHdr(void *, rtr_msgb_t *msgb, RsslError *error);
RsslInt32 rwsSendPingData(RsslSocketChannel *, RsslBuffer *, RsslError *);
RsslInt32 rwsSendWsPing( RsslSocketChannel *, RsslBuffer *, RsslError *);
RsslInt32 rwsSendWsPong(RsslSocketChannel *, RsslBuffer *, RsslError *);
RsslInt32 rwsSendWsClose(RsslSocketChannel *, rwsCFStatusCodes_t, RsslError *);

RsslInt32 URLdecode(char *, RsslInt32 , char *);
RsslInt32 getIntValue(RsslInt32 *, RsslInt32 , char *);


void rsslReleaseWebSocketServer(void *);
void rsslReleaseWebSocketSession(void *);
void clearFrameHeader(rwsFrameHdr_t * );
void rwsClearCompression(rwsComp_t *);
void rwsRelCompression(rwsComp_t *);
void rwsClearCookies(rwsCookies_t *);
void rwsRelCookies(rwsCookies_t *);

void rwsClearSession(rwsSession_t *);
rwsSession_t *rwsNewSession();
void rwsReleaseSession(rwsSession_t *);

void rwsClearServer(rwsServer_t *);
rwsServer_t *rwsNewServer();
void rwsReleaseServer(rwsServer_t *);

RsslRet rwsInitSessionOptions(RsslSocketChannel *, RsslWSocketOpts *, RsslError *);
RsslRet rwsInitServerOptions(RsslServerSocketChannel *, RsslWSocketOpts *, RsslError *);

/* Transport functions */
RTR_C_INLINE RsslRet rwsInitializeSessionTransport()
{
	_DEBUG_TRACE_CONN("called\n")
	return RSSL_RET_SUCCESS;
}

RTR_C_INLINE int rwsInitializeTransport(void *transport, ripcSessInProg *inPr, RsslError *error)
{
	_DEBUG_TRACE_CONN("called\n")
	return RSSL_RET_SUCCESS;
}
/* creates and initializes new client/session structure */
RTR_C_INLINE ripcSessInit rwsInitTrans(void *transport, ripcSessInProg *inPr, RsslError *error)
{
	_DEBUG_TRACE_CONN("called\n")
	return RIPC_CONN_IN_PROGRESS;
}

RTR_C_INLINE RsslRet rwsInitConnectOpts(rwsConnectOpts_t *config, RsslError *error)
{
	_DEBUG_TRACE_CONN("called\n")
	return RSSL_RET_SUCCESS;
}

RTR_C_INLINE RsslSocket acceptSocket(rsslServerImpl *srvr, void** userSpecPtr, RsslError *error)
{
	_DEBUG_TRACE_CONN("called\n")
	return RSSL_RET_SUCCESS;
}

RTR_C_INLINE void rwsFreeConnectOpts(rwsConnectOpts_t *config)
{

}

RTR_C_INLINE RsslRet rwsInitializeProtocolFuncs()
{
	ripcProtocolFuncs  funcs;

	funcs.readTransportConnMsg = rwsReadWsConnMsg; /* intended for smaller messages, pre or post data message flow */
	funcs.readTransportMsg = rwsReadTransportMsg;  /* intended for during data message flow with reading as much as possible */
	funcs.readPrependTransportHdr = rwsReadPrependTransportHdr;
	funcs.prependTransportHdr = rwsPrependWsHdr;
	funcs.additionalTransportHdrLength = rwsAdditionalHeaderLength;
	funcs.getPoolBuffer = rwsGetPoolBuffer;
	funcs.getGlobalBuffer = rwsGetSimpleBuffer;
	
	return (ipcSetProtocolHdrFuncs(RSSL_CONN_TYPE_WEBSOCKET, &funcs));
}


RTR_C_INLINE RsslRet rwsInitialize()
{
	RsslInt32 retVal = 0;

	ripcTransportFuncs rwsFuncs;

	_DEBUG_TRACE_CONN("called\n")
	
	rwsInitializeProtocolFuncs();

	rwsFuncs.bindSrvr = 0;
	rwsFuncs.newSrvrConnection = 0;
	rwsFuncs.connectSocket = 0;
	rwsFuncs.newClientConnection = 0;
	rwsFuncs.initializeTransport = rwsInitializeTransport;
	rwsFuncs.shutdownTransport = 0;
	rwsFuncs.readTransport = 0;
	rwsFuncs.writeTransport = 0;
	rwsFuncs.writeVTransport = 0;
	rwsFuncs.reconnectClient = 0;
	rwsFuncs.acceptSocket = 0;
	rwsFuncs.shutdownSrvrError = 0;
	rwsFuncs.sessIoctl = 0;
	rwsFuncs.getSockName = 0;
	rwsFuncs.setSockOpts = 0;
	rwsFuncs.getSockOpts = 0;
	rwsFuncs.connected = 0;
	rwsFuncs.shutdownServer = 0;
	rwsFuncs.uninitialize = 0;

	return (ipcSetTransFunc(RSSL_CONN_TYPE_WEBSOCKET, &rwsFuncs));
}

#endif  /* __rwsutils_h */

