/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|          Copyright (C) 2019-2020 LSEG. All rights reserved.               --
 *|-----------------------------------------------------------------------------
 */

#ifndef __rsslchanmanagement
#define __rsslchanmanagement

#include "rtr/rsslTransport.h"
#include "rtr/rsslAlloc.h"
#include "rtr/rsslQueue.h"
#include "rtr/rsslHashTable.h"
#include "rtr/ripch.h"
#include "rtr/rsslThread.h"
#include "rtr/rtrdefs.h"

#include <limits.h>

#ifndef WIN32
#include <netdb.h>
#else
#include <WS2tcpip.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif

/* forward declaration needed so this can be contained in channel and 
   server info structures */
typedef struct RsslTransChannelFuncs RsslTransChannelFuncs;
typedef struct RsslTransServerFuncs RsslTransServerFuncs;
			  
#define RSSL_SOCKET_TRANSPORT   0
#define RSSL_UNIDIRECTION_SHMEM_TRANSPORT  1
#define RSSL_RRCP_TRANSPORT 2
#define RSSL_SEQ_MCAST_TRANSPORT 3
#define RSSL_WEBSOCKET_TRANSPORT   4
#define RSSL_MAX_TRANSPORTS     RSSL_WEBSOCKET_TRANSPORT + 1

/* used for all connection types to control locking */
extern RsslLockingTypes multiThread;  /* 0 == No Locking; 1 == All locking; 2 == Only global locking */

/* debug globals - set to 0 is off, set to 1 will print debug msgs */
extern unsigned char memoryDebug;

/* max ripc fragment size */
#define RSSL_MAX_MSG_SIZE 6*1024

/* default max JSON message fragment size */
#define RSSL_MAX_JSON_MSG_SIZE 6*1024*10

/* number of pool buffers */
#define RSSL_POOL_SIZE 1048576  

/* Maximum WS session payload size(3G)*/
#define RSSL_MAX_JSON_PAYLOAD 0xC0000000

/* Maximum WS session payload size(500M)*/
#define RSSL_MIN_JSON_PAYLOAD 0x1F400000

/* Maximum WS message size (6M) */
#define RSSL_MAX_JSON_FRAGMENT_SIZE 0x600000

/* rssl flag settings */
#define CLIENT_TO_SERVER 0x1
#define SERVER_TO_CLIENT 0x2

/**
* @brief RSSL Trace Options Information
* @see RsslIoctlCodes
*/
typedef struct {
	RsslTraceOptions traceOptions; /* The trace Options as passed in by the users */
	int traceMsgOrigFileNameSize;  /* The number of characters in traceMsgFileName supplied by the users */
	char* newTraceMsgFileName;	   /* This is a modified file name is based on the original file name supplied by the user.
								   * This string also includes a time stamp appended to it if a new trace file needs to be created
								   * if traceMsgMaxFileSize is reached. The ".xml" extension is appended to the end of the string*/
	FILE* traceMsgFilePtr;		   /* Pointer to the user specified file */
} RsslTraceOptionsInfo;

/** @brief Monitoring information of allocates/deallocates instances and a close call.
 * rsslServerImpl - pool of preallocated objects are stored in the queues freeServerList/activeServerList
 * and members:
 * rsslServerImpl.transportInfo => an instance of RsslServerSocketChannel - pool of preallocated objects are stored in the queues freeServerSocketChannelList/activeServerSocketChannelList
 * RsslServerSocketChannel.stream => a socket API object - monitoring close_sock API call
 * RsslServerSocketChannel.transportInfo => a SSL server instance on encrypted connection
 *
 * rsslServerImpl.transportInfo => an instance of rtrShmTransServer on shared memory connection
 *
 * @see rsslServerImpl, RsslServerSocketChannel, rtrShmTransServer
*/
typedef struct {
	RsslUInt32		countOfFreeServerList;		/* stores a current amount of elements in the queue freeServerList rsslImpl.c (rsslServerImpl) */
	RsslUInt32		countOfActiveServerList;	/* stores a current amount of elements in the queue activeServerList rsslImpl.c (rsslServerImpl) */
	RsslUInt32		countOfFreeServerSocketChannelList;		/* stores a current amount of elements in the queue freeServerSocketChannelList rsslSocketTransportImpl.c (RsslServerSocketChannel) */
	RsslUInt32		countOfActiveServerSocketChannelList;	/* stores a current amount of elements in the queue activeServerSocketChannelList rsslSocketTransportImpl.c (RsslServerSocketChannel) */

	RsslUInt32		numberCallsOfReleaseSSLServer;	/* stores a current number of ripcReleaseSSLServer calls for RsslServerSocketChannel::transportInfo */
	RsslUInt32		numberCallsOfShmTransDestroy;	/* stores a current number of rtrShmTransDestroy calls for rsslServerImpl::transportInfo */
} rsslServerCountersInfo;

typedef struct {
	RsslChannel 	Channel;			/* stores actual channel structure */
	RsslQueueLink	link1;				/* storage for the activeChannelList qtool links */
	RsslQueueLink	link2;				/* storage for the servers activeChannels qtool links */
	RsslMutex		chanMutex;			/* internal mutex used for reads */
	RsslMutex		traceMutex;			/* used when tracing to prevent multiple threads logging at the same time */
	RsslQueue		activeBufferList;	/* list of active buffers by this channel */
	int				debugFlags;			/* set to provide outbound or inbound rssl message debugging */
	int				rsslFlags;			/* holds clientToServer and serverToClient and flags */
	RsslUInt32		maxMsgSize;			/* used for fragmentation - max msg length*/
	int 			maxGuarMsgs;		/* used for fragmentation - number of guar msgs */
	RsslUInt32		fragId;				/* used to keep track of the current frag id to use */
	RsslUInt32		unpackOffset;		/* used to keep track of unpacking offset into packedbuffer */
	int				moreData;			/* used to keep track of the read moreData flag so we can post it after unpacking */
	rtr_msgb_t		*packedBuffer;		/* used to keep track of packed buffer if present */
	int				returnBufferOwner;	/* 1 if I own return buffer, 0 if not */
	RsslBuffer		returnBuffer;		/* this is used as the return buffer */
	void*			transportClientInfo;		
	void*			transportServerInfo;	/* This variable keeps pointer to a server of specific transrpot type*/
	RsslHashTable 		assemblyBuffers;		/* hash table of assembly buffers */
	RsslQueue		freeBufferList;			/* list of free buffers to use for writing */
	RsslTransChannelFuncs *channelFuncs;	/* channel function pointers */
	RsslTraceOptionsInfo traceOptionsInfo;	/* message tracing */
	void*			transportInfo;			/* pointer to transport specific memory - this will be created/cleaned up by specific transports if needed */
	RsslBool		isBlocking;				/* use by blocking read */
	RsslBool		ownCompVer;				/* if true, we created memory for component version.  false otherwise */
	RsslComponentInfo	componentVer;		/* the component version string if passed in by the user */
	RsslComponentInfo   **componentInfo;	/* the component info that comes out for socket transport types */
	RsslUInt16		fragIdMax;
	RsslUInt64		shared_key;				/* shared key for encryption/decryption.  If 0 this is not present */
	RsslBool			ownConnOptCompVer;	/* if true, we created memory for connn opts component version.  false otherwise */
	RsslComponentInfo	connOptsCompVer;	/* the component version string passed in by the user through the connectOpts */
} rsslChannelImpl;	

typedef struct {
	RsslServer		Server;					/* stores actual server structure */
	RsslQueueLink	link1;					/* storage for the qtool links */
	RsslMutex		srvrMutex;				/* internal mutex used for server */
	RsslMutex		sharedBufPoolMutex;		/* mutex used for shared buf pool */
	int				hasSharedBufPool;		/* if we have a sharedBufPoolMutex this is true */
	RsslConnectionTypes connectionType;		/*!< Type that this connection is */
	char			packedBuffer;			/* this is used to switch on if shmem packing is on or off */
	RsslQueue		activeChannels;			/* list of channels connected to server */
	RsslTransServerFuncs *serverFuncs;		/* server function pointers */
	RsslTransChannelFuncs *channelFuncs;	/* channel function pointers */
	void*			transportInfo;			/* pointer to transport specific memory - this will be created/cleaned up by specific transports if needed */
	RsslBool		isBlocking;				/* use by blocking accept */
	RsslComponentInfo	componentVer;		/* the component version string if passed in by the user.  If present in server, we always own the memoryb */
	RsslUInt32		sendBufSize;			/* send buffer size for accepted connections */
	RsslUInt32		recvBufSize;			/* receive buffer size to use for accepted connections */
	RsslComponentInfo	connOptsCompVer;	/* the component version string passed in by the user through the connectOpts*/
	RsslBool		serverSharedSocket;		/* will be allowed to share socket */
	rsslServerCountersInfo	serverCountersInfo;	/* stores amount of active and free memory instances rsslServerImpl, RsslServerSocketChannel */
} rsslServerImpl;

typedef struct {
	RsslHashLink   link1;				/* link used when this struct is put in a hashtable */
	RsslBuffer     buffer;				/* pointer to buffer */
	RsslUInt64	   nodeId;				/* nodeID (for multicast) */
	RsslNodeId	   returnNodeId;	    /* the node ID we will return from rsslReadEx */
	RsslUInt32	   fragId;				/* fragmentation id of this buffer */
	RsslUInt32	   readCursor;			/* number of bytes read so far */
	RsslUInt32	   lastPerFragIDSeqNum;	/* last per frag ID seqNum received (for multicast) */
	RsslUInt32	   msgSeqNum;			/* sequence number for the whole message (for multicast) */
	RsslUInt16	   readOutFlags;		/* for return of readOutArgs */
	RsslUInt8	   FTGroupId;			/* the FTGroup this was sent from (for multicast) */
} rsslAssemblyBuffer;

typedef enum {
	BUFFER_IMPL_NONE = 0,
	BUFFER_IMPL_FIRST_FRAG_HEADER = 1,
	BUFFER_IMPL_SUBSEQ_FRAG_HEADER = 2,
	BUFFER_IMPL_LAST_FRAG_HEADER = 3,
	BUFFER_IMPL_ONLY_ONE_FRAG_MSG = 4
} fragmentationHeaderTypes;

typedef struct {
	RsslBuffer	buffer;				/* actual buffer */
	RsslQueueLink	link1;			/* storage for the qtool links */
	RsslInt32	integrity;			/* store some number in here and verify - should help alert us to any overrun issues */ 
	int		    owner;				/* if 1, i allocated memory in buffer, if 0, its a ripcBuffer */		
	RsslUInt32	fragId;				/* fragmentation id */
	RsslUInt32	writeCursor;		/* write position - for fragmentation */
	RsslUInt32	packingOffset;		/* packing offset for packing */
	RsslUInt32	totalLength;		/* total length - used to calculated packing buffer sizes */
	rsslChannelImpl *RsslChannel;	/* channel that currently owns this buffer */
	int	priority;					/* which priority queue to write to */
	void			*bufferInfo;		/* The new type to abstract the underlying transport's buffer type*/	
	RsslUInt8		fragmentationFlag; /* indicate whether the buffer is used for fragmentation*/
	RsslBuffer  compressedBuffer; /* This buffer is used to compress the entire message before spliting into multiple fragmented messages. */
	int			memoryAllocationOffset;  /* This is memory offset from the orignal memory allocation. */
} rsslBufferImpl;

/**
* @brief Clears RsslComponentInfo structure
*
* This initializes the RsslComponentInfo structure.
* @param componentInfo
* @see RsslComponentInfo
*/
RTR_C_INLINE void rsslClearComponentInfo(RsslComponentInfo *componentInfo)
{
	componentInfo->componentVersion.length = 0;
	componentInfo->componentVersion.data = 0;
}

/**
* @brief Trace Options Info dynamic initialization
* @param opts RsslTraceOptionsInfo
* @see RsslTraceOptionsInfo
*/
RTR_C_INLINE void rsslClearTraceOptionsInfo(RsslTraceOptionsInfo *traceOptionsInfo)
{
	rsslClearTraceOptions(&(traceOptionsInfo->traceOptions));
	traceOptionsInfo->newTraceMsgFileName = NULL;
	traceOptionsInfo->traceMsgOrigFileNameSize = 0;
	traceOptionsInfo->traceMsgFilePtr = NULL;
}

RTR_C_INLINE RsslUInt32 UInt32_key_hash(void *element)
{
	RTPRECONDITION(element != 0);
	return ((*(rsslAssemblyBuffer*)element).fragId);
}

RTR_C_INLINE RsslBool checkFragID(void *element1, void *element2)
{
	RTPRECONDITION(element1 != 0);
	RTPRECONDITION(element2 != 0);

	return (((*(rsslAssemblyBuffer*)element1).fragId) == ((*(rsslAssemblyBuffer*)element2).fragId)) &&
		(((*(rsslAssemblyBuffer*)element1).nodeId) == ((*(rsslAssemblyBuffer*)element2).nodeId));
}


/* define function pointers structures */

	/* This structure represents the function entry points for the different
	 * transports that can be used by rssl. This structure is used for 
	 * RsslChannel structures.  There are currently two transports
	 * available:
	 * -- Socket (includes HTTP and HTTPS)
	 * -- Unidirectional Shared Memory
	 */
typedef struct RsslTransChannelFuncs {
	/* Creates an outbound connection and returns the channelImpl structure. NULL returned on error. */
	RsslRet (*channelConnect)( rsslChannelImpl* rsslChnlImpl, RsslConnectOptions *opts, RsslError *error );
	/* Will perform a reconnection for some transports that require it (mainly HTTP/HTTPS) */
    RsslRet   (*channelReconnect)( rsslChannelImpl *rsslChnlImpl, RsslError *error );
	/* Performs channel initialization process/handshake. */
	RsslRet   (*initChannel)( rsslChannelImpl* rsslChnlImpl, RsslInProgInfo *inProg, RsslError *error );
	/* Close channel */
	RsslRet   (*channelClose)( rsslChannelImpl* rsslChnlImpl, RsslError *error );
	/* Reads from the transport, returns buffer  */
	RsslBuffer*  (RTR_FASTCALL *channelRead)( rsslChannelImpl* rsslChnlImpl, RsslReadOutArgs *readOutArgs, RsslRet *readRet, RsslError *error );
	/* Writes to the transport */
	RsslRet     (RTR_FASTCALL *channelWrite)( rsslChannelImpl *rsslChnlImpl, rsslBufferImpl *rsslBufImpl, RsslWriteInArgs *writeInArgs, RsslWriteOutArgs *writeOutArgs, RsslError *error );
	/* Flush data written to transport */
	RsslRet   (RTR_FASTCALL *channelFlush)( rsslChannelImpl *rsslChnlImpl, RsslError *error );					
	/* Gets buffer used for writing to transport */
	rsslBufferImpl*  (RTR_FASTCALL *channelGetBuffer)( rsslChannelImpl *rsslChnlImpl, RsslUInt32 size, RsslBool packedBuffer, RsslError *error );
	/* Releases buffer used for failed write or unused buffer to transport */
	RsslRet  (RTR_FASTCALL *channelReleaseBuffer)( rsslChannelImpl *rsslChnlImpl, rsslBufferImpl *rsslBufImpl, RsslError *error );
	/* Returns current number of used buffers */
	RsslInt32  (RTR_FASTCALL *channelBufferUsage)( rsslChannelImpl *rsslChnlImpl, RsslError *error );
	/* Packs content into buffer for writing */
	RsslBuffer*  (RTR_FASTCALL *channelPackBuffer)( rsslChannelImpl *rsslChnlImpl, rsslBufferImpl *rsslBufImpl, RsslError *error );
	/* Sends ping or flushes buffers */
	RsslRet  (RTR_FASTCALL *channelPing)( rsslChannelImpl *rsslChnlImpl, RsslError *error );
	/* Gets Channel Information */
	RsslRet  (RTR_FASTCALL *channelGetInfo)( rsslChannelImpl *rsslChnlImpl, RsslChannelInfo *info, RsslError *error );
	/* Gets Channel Information */
	RsslRet(RTR_FASTCALL *channelGetStats)(rsslChannelImpl *rsslChnlImpl, RsslChannelStats *info, RsslError *error);
	/* Allows for changing channel options */
	RsslRet  (RTR_FASTCALL *channelIoctl)( rsslChannelImpl *rsslChnlImpl, RsslIoctlCodes code, void *value, RsslError *error );
} RsslTransportChannelFuncs;


	/* This structure represents the function entry points for the different
	 * transports that can be used by rssl. This structure is used for 
	 * RsslServer structures.  There are currently two transports
	 * available:
	 * -- Socket (includes HTTP and HTTPS)
	 * -- Unidirectional Shared Memory
	 */
typedef struct RsslTransServerFuncs {
	/* Creates a listening connection and returns the serverImpl structure. NULL returned on error */
	RsslRet (*serverBind)( rsslServerImpl* rsslSrvrImpl, RsslBindOptions *opts, RsslError *error );
	/* Accepts incoming server connections - returns rsslChannelImpl with proper transport type functions already set */
    rsslChannelImpl* (*serverAccept)( rsslServerImpl *rsslSrvrImpl, RsslAcceptOptions *opts, RsslError *error );
	/* Allows for changing server properties */
	RsslRet  (RTR_FASTCALL *serverIoctl)( rsslServerImpl *rsslSrvrImpl, RsslIoctlCodes code, void *value, RsslError *error );
	/* Gets server information */
	RsslRet (RTR_FASTCALL *serverGetInfo)( rsslServerImpl *rsslSrvrImpl, RsslServerInfo *info, RsslError *error );
	/* Returns current number of used server (shared pool) buffers */
	RsslInt32  (RTR_FASTCALL *serverBufferUsage)( rsslServerImpl *rsslSrvrImpl, RsslError *error );

	/* Closes the server channel */
	RsslRet (RTR_FASTCALL *closeServer)(rsslServerImpl* rsslSrvrImpl, RsslError *error);

} RsslTransportServerFuncs;


/* used by each transport to set its functions into the array */
RsslRet rsslSetTransportChannelFunc( int transportType, RsslTransportChannelFuncs *funcs );
RsslRet rsslSetTransportServerFunc( int transportType, RsslTransportServerFuncs *funcs );
RsslTransportChannelFuncs* rsslGetTransportChannelFunc(int transportType);

/* Maximum number of protocol types for function debug dump entry points */
#define MAX_PROTOCOL_TYPES UCHAR_MAX

/* This structure represents the function entry points that can be used by rssl.
*/
typedef struct {
	/* Function pointer to the incoming IPC message debug function. */
	void(*rsslDumpInFunc)(const char* functionName, char* buffer, RsslUInt32 length, RsslChannel* channel);
	/* Function pointer to the outgoing IPC message debug function. */
	void(*rsslDumpOutFunc)(const char* functionName, char* buffer, RsslUInt32 length, RsslChannel* channel);
} RsslDumpFuncs;


/**********************************
 *  INLINE UTILITY/HELPER FUNCTIONS
 **********************************/


/* BUFFER HELPERS */

/* initializes the buffer struct */
RTR_C_ALWAYS_INLINE void _rsslCleanBuffer(rsslBufferImpl *buffer)
{
	buffer->buffer.data = 0;
	buffer->buffer.length = 0;

	/* we reset integrity just so that if somebody casts the buffer when it shouldnt be cast,
	   the integrity check wont work - we set it back to 69 when we use GetBuffer to get it */
	buffer->integrity = 0;
	buffer->bufferInfo = NULL;
	buffer->owner = 0;
	buffer->fragId = 0;
	buffer->writeCursor = 0;
	buffer->packingOffset = 0;
	buffer->totalLength = 0;
	buffer->RsslChannel = NULL;
	buffer->priority = -1;

	buffer->fragmentationFlag = BUFFER_IMPL_NONE;
	buffer->compressedBuffer.data = 0;
	buffer->compressedBuffer.length = 0;
	buffer->memoryAllocationOffset = 0;
}

/* does memory allocation and initialization of buffer */
RTR_C_ALWAYS_INLINE rsslBufferImpl *_rsslCreateBuffer(rsslChannelImpl *chnl)
{
	rsslBufferImpl *buffer = (rsslBufferImpl*)_rsslMalloc(sizeof(rsslBufferImpl));

	if (buffer)
	{
		_rsslCleanBuffer(buffer);

		rsslInitQueueLink(&(buffer->link1));
	}

	return buffer;
}

/* grabs new buffer from freeList */
RTR_C_ALWAYS_INLINE rsslBufferImpl *_rsslNewBuffer(rsslChannelImpl *chnl)
{
	rsslBufferImpl *buffer=0;
	RsslQueueLink *pLink = 0;

	if (multiThread == RSSL_LOCK_GLOBAL_AND_CHANNEL)
	  (void) RSSL_MUTEX_LOCK(&chnl->chanMutex);

	if ((pLink = rsslQueuePeekFront(&(chnl->freeBufferList))) == 0)
		buffer = _rsslCreateBuffer(chnl);
	else
	{
		buffer = RSSL_QUEUE_LINK_TO_OBJECT(rsslBufferImpl, link1, pLink);
		if (rsslQueueLinkInAList(&(buffer->link1)))
		{
			rsslQueueRemoveLink(&(chnl->freeBufferList), &(buffer->link1));
			if (memoryDebug)
				printf("removing from freeBufferList\n");
		}
	}
	if (multiThread == RSSL_LOCK_GLOBAL_AND_CHANNEL)
	  (void) RSSL_MUTEX_UNLOCK(&chnl->chanMutex);

	return buffer;
}

/* initializes the assemblyBuffer struct */
RTR_C_ALWAYS_INLINE void _rsslCleanAssemblyBuffer(rsslAssemblyBuffer *assemblyBuf)
{
	assemblyBuf->buffer.data = 0;
	assemblyBuf->buffer.length = 0;
	assemblyBuf->fragId = 0;
	assemblyBuf->readCursor = 0;
	assemblyBuf->lastPerFragIDSeqNum = 0;
	assemblyBuf->returnNodeId.nodeAddr = 0;
	assemblyBuf->returnNodeId.port = 0;
	assemblyBuf->FTGroupId = 0;
	assemblyBuf->readOutFlags = 0;
	assemblyBuf->nodeId = 0;
}	


/* CHANNEL HELPERS */

/* initializes the channel struct */
RTR_C_ALWAYS_INLINE void _rsslCleanChan(rsslChannelImpl *chnl)
{
	chnl->Channel.socketId = RIPC_INVALID_SOCKET;
	chnl->Channel.oldSocketId = RIPC_INVALID_SOCKET;
	chnl->Channel.state = RSSL_CH_STATE_INACTIVE;

	/* these two guys are just pointers into the RIPC socket struct */
	/* ripc should free the memory when the channel goes away */
	chnl->Channel.clientIP = 0;
	chnl->Channel.clientHostname = 0;
	chnl->Channel.hostname = 0;
	chnl->Channel.port = 0;

	chnl->Channel.pingTimeout = 0;

	chnl->Channel.majorVersion = 0;
	chnl->Channel.minorVersion = 0;
	chnl->Channel.protocolType = 0;

	chnl->Channel.connectionType = RSSL_CONN_TYPE_INIT;

	chnl->rsslFlags = 0x0;

	chnl->maxMsgSize = 0;
	chnl->maxGuarMsgs = 0;
	chnl->fragId = 1;
	chnl->returnBuffer.data = 0;
	chnl->returnBuffer.length = 0;
	chnl->returnBufferOwner = 0;

	/* set this to the typical value.  If ripc allows for more (e.g. greater than conn version 13) it will be increased when we connect */
	chnl->fragIdMax = 255;

	/* reinitialize shared key */
	chnl->shared_key = 0;

	chnl->packedBuffer = 0;
	chnl->moreData = 0;
	chnl->unpackOffset = 0;

	chnl->debugFlags = 0x0;

	chnl->Channel.userSpecPtr = NULL; 

	chnl->transportInfo = NULL;
	chnl->transportServerInfo = NULL;
	chnl->transportClientInfo = NULL;

	chnl->channelFuncs = 0;

	chnl->transportInfo = 0;

	chnl->isBlocking = RSSL_FALSE;

	chnl->ownCompVer = RSSL_FALSE;
	rsslClearComponentInfo(&chnl->componentVer);

	/* component info should be freed by each transports specific code, we just initialize it here */
	chnl->componentInfo = 0;

	chnl->ownConnOptCompVer = RSSL_FALSE;
	rsslClearComponentInfo(&chnl->connOptsCompVer);

	rsslClearTraceOptionsInfo(&chnl->traceOptionsInfo);
}

/* SERVER HELPERS */

/* initializes the server struct */
RTR_C_ALWAYS_INLINE void _rsslCleanServer(rsslServerImpl *srvr)
{
	srvr->Server.socketId = RIPC_INVALID_SOCKET;
	srvr->Server.state = RSSL_CH_STATE_INACTIVE;
	srvr->Server.userSpecPtr = NULL;
	srvr->connectionType = RSSL_CONN_TYPE_SOCKET;
	srvr->packedBuffer = 0;
	
	srvr->transportInfo = NULL; 

	srvr->channelFuncs = 0;
	srvr->serverFuncs = 0;
	/* activeChannels list should be empty */

	rsslClearComponentInfo(&srvr->componentVer);

	rsslClearComponentInfo(&srvr->connOptsCompVer);

	srvr->isBlocking = RSSL_FALSE;

	srvr->sendBufSize = 0;
	srvr->recvBufSize = 0;
}

/* Hostname/IP Addr/Port conversion helpers */
/* These are based off of ripc function (ipcGetServByName) 
 * if changes go into either, we should keep both updated */

#ifdef	RTR_LITTLE_ENDIAN
#define	host2netAddr(p) \
   (u32)(((p>>24)&0xff)|((p>>8)&0xff00)|((p<<8)&0xff0000)|((p<<24)&0xff000000))
#else
#define host2netAddr(p) ((u32)(p))
#endif


RTR_C_INLINE RsslInt32 rsslGetHostByName(char *hostName, RsslUInt32 *address)
{
#if defined (x86_Linux_4X) || (x86_Linux_3X) || (x86_Linux_2X)
	struct hostent	hostEntry;
	RsslInt32				h_errnop;
	char					*hostBuff = (char *)_rsslMalloc(IPC_MAX_HOST_NAME);
	RsslInt32				buflen = IPC_MAX_HOST_NAME;
	RsslInt32				prevBufLen = buflen;
	struct hostent			*hep = 0;
#else
	DWORD retResult;
	struct addrinfo *result = 0;
	struct addrinfo hints;
#endif

	if ((hostName == 0) || (hostName[0] == '\0') || (strcmp(hostName, "localhost") == 0))
	{
		*address = host2netAddr(INADDR_LOOPBACK);
#if defined (x86_Linux_4X) || (x86_Linux_3X) || (x86_Linux_2X)
		_rsslFree(hostBuff);
#endif
		return(0);
	}

#if defined (x86_Linux_4X) || (x86_Linux_3X) || (x86_Linux_2X)

	while(gethostbyname_r(hostName,&hostEntry,hostBuff,buflen,&hep,&h_errnop))
	{
		if (h_errnop == NETDB_INTERNAL && errno == ERANGE)
		{
			prevBufLen = buflen;
			buflen *= 2;
			hostBuff = (char*)_rsslRealloc(hostBuff, prevBufLen, buflen);
		}
		else
			break;
	}

	if (hep)
	{
		*address = *((u32*)(hep->h_addr));
		_rsslFree(hostBuff);
		hostBuff = NULL;
		return(0);
	}
	else
	{
		_rsslFree(hostBuff);
		hostBuff = NULL;
	}

#else
	ZeroMemory(&hints, sizeof(hints));
	hints.ai_family = AF_INET; // Query only (IPv4) address family
	hints.ai_socktype = SOCK_STREAM;

	retResult = getaddrinfo(hostName, NULL, &hints, &result);

	// Retrieve only the first address information of the list
	if (retResult == 0 && result)
	{
		*address = ((struct sockaddr_in *)result->ai_addr)->sin_addr.S_un.S_addr;

		freeaddrinfo(result);
		return (0);
	}
#endif

	return(-1);
}


/* 0 returned if failure */
RTR_C_INLINE RsslUInt16 rsslGetServByName(char *serv_name)
{
	char* s;
	RsslInt32 i;
	RsslInt32 maxLen = 1024;
	struct servent *serv_port;	 /* Service port */

#if defined (x86_Linux_4X) || (x86_Linux_3X) || (x86_Linux_2X)
	struct servent serv_result;
	char tbuf[1024];
#endif

	for (s = serv_name, i = 0; (i <= maxLen) && (*s != (char)0);  ++i)
	{
		if (! isprint(*s++))
			return(0);
	}

	if (serv_name != (char *)0)
	{
		RsslInt32 port;

		/* Check for port number definition first */
		if ( ((port = atoi(serv_name)) > 0) && (port <= 65535))
		{
			RsslUInt16 prt=(RsslUInt16)port;
			return htons(prt);
		}

#if defined (x86_Linux_4X) || (x86_Linux_3X) || (x86_Linux_2X)
		getservbyname_r(serv_name,"udp",&serv_result,tbuf,1024,&serv_port);
#else
		serv_port = getservbyname(serv_name,"udp");
#endif
	}
	else
		serv_port = NULL;

	if (serv_port != NULL)
		return (serv_port->s_port); 
	else if ((serv_name) && ((!strcmp(serv_name, "rmds_rssl_sink")) || (!strcmp(serv_name, "rssl_consumer"))))
		return htons(14002);
	else if ((serv_name) && ((!strcmp(serv_name, "rmds_rssl_source")) || (!strcmp(serv_name, "rssl_provider"))))
		return htons(14003);

	return(0);
}


/* Defined in rsslImpl.c as these are needed in several places */

void _rsslCleanUp();

rsslChannelImpl *_rsslNewChannel();
rsslServerImpl *_rsslNewServer();

void RTR_FASTCALL _rsslReleaseChannel(rsslChannelImpl *chnl);
void RTR_FASTCALL _rsslReleaseServer(rsslServerImpl *srvr);


#ifdef __cplusplus
} /* extern "C" */
#endif

#endif

