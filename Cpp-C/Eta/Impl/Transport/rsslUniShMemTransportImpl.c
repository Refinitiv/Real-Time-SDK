/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|          Copyright (C) 2019-2020 Refinitiv. All rights reserved.          --
 *|-----------------------------------------------------------------------------
 */

#include "rtr/rsslUniShMemTransport.h"
#include "rtr/rsslUniShMemTransportImpl.h"
#include "rtr/rsslAlloc.h"
#include "rtr/rsslErrors.h"
#include "rtr/rsslDataUtils.h"

#if SHM_NAMEDPIPE || SHM_SOCKET_UDP || SHM_SOCKET_TCP
#if _WIN32
#include <Windows.h>
#include <WinBase.h>
#include <io.h>
#else
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#endif
#endif

#ifdef SHM_BYTE_COUNT
static RsslUInt32 readCnt = 0;
static RsslUInt32 readSuccess = 0;
static RsslUInt32 readByteCnt = 0;
static RsslUInt32 readByteSuccess = 0;
static RsslUInt32 readByteFailed = 0;
static RsslUInt32 writeCnt = 0;
static RsslUInt32 writeByteCnt = 0;
#endif

/* global debug function pointers */
static void(*rsslUniShMemDumpInFunc)(const char *functionName, char *buffer, RsslUInt32 length, RsslSocket socketId) = 0;
static void(*rsslUniShMemDumpOutFunc)(const char *functionName, char *buffer, RsslUInt32 length, RsslSocket socketId) = 0;

/***************************
 * START INLINE HELPER FUNCTIONS 
 ***************************/

/* grabs new buffer from freeList */
/* for shmem, there is only one rsslBuffer in the freelist */
/* we limit the client to only one rsslBuffer to prevent out of order writes */
RTR_C_ALWAYS_INLINE rsslBufferImpl *_rsslUniShMemNewBuffer(rsslChannelImpl *chnl)
{
	rsslBufferImpl *buffer;
	RsslQueueLink *pLink = 0;

	if (rtrUnlikely(multiThread == RSSL_LOCK_GLOBAL_AND_CHANNEL))
	  (void) RSSL_MUTEX_LOCK(&chnl->chanMutex);

	pLink = rsslQueueRemoveFirstLink(&(chnl->freeBufferList));
	buffer = RSSL_QUEUE_LINK_TO_OBJECT(rsslBufferImpl, link1, pLink);

	if (rtrUnlikely(multiThread == RSSL_LOCK_GLOBAL_AND_CHANNEL))
	  (void) RSSL_MUTEX_UNLOCK(&chnl->chanMutex);

	return buffer;
}


/***************************
 * START NON-PUBLIC ABSTRACTED FUNCTIONS 
 ***************************/

/* rssl ShMem Bind call */
RsslRet rsslUniShMemBind(rsslServerImpl* rsslSrvrImpl, RsslBindOptions *opts, RsslError *error)
{
	rtrShmTransServer *shMemServer;
	rtrShmCreateOpts shMemOpts;

	RsslInt32 nBytes;

	shMemOpts.serverBlocking = RSSL_FALSE;
	shMemOpts.channelsBlocking = RSSL_FALSE;

	/* set up shared memory options */
	shMemOpts.majorVersion = opts->majorVersion;
	shMemOpts.minorVersion = opts->minorVersion;
	shMemOpts.protocolType = opts->protocolType;
	shMemOpts.serverToClientPing = opts->serverToClientPings;
		
	if (opts->interfaceName)
		nBytes = snprintf(shMemOpts.shMemKey, sizeof(shMemOpts.shMemKey), "%s%s",
							opts->interfaceName, opts->serviceName);
	else
		nBytes = snprintf(shMemOpts.shMemKey, sizeof(shMemOpts.shMemKey), "%s", opts->serviceName);

	if ((nBytes >= (RsslInt32)sizeof(shMemOpts.shMemKey)) || nBytes < 0)
	{
		_rsslSetError(error, NULL, RSSL_RET_FAILURE,  0);
		snprintf(error->text, MAX_RSSL_ERROR_TEXT, "<%s:%d> rsslUniShMemBind() bad interface and/or service name\n", __FILE__, __LINE__);
		return RSSL_RET_FAILURE;
	}

	/* max ping timeout is 255 seconds (which is an eternity for a shmem connection) */
	if (opts->pingTimeout <= 0xFF)
		shMemOpts.pingTimeout = opts->pingTimeout;
	else
		shMemOpts.pingTimeout = 0xFF;

	if (opts->maxOutputBuffers < opts->guaranteedOutputBuffers)
		shMemOpts.numBuffers = opts->guaranteedOutputBuffers;
	else
		shMemOpts.numBuffers = opts->maxOutputBuffers;

	shMemOpts.maxBufSize = opts->maxFragmentSize;
	shMemOpts.userSpecPtr = opts->userSpecPtr;

	/* Now create shared memory segment */
	/* do shared memory segment creation */
	shMemServer = rtrShmTransCreate(&shMemOpts, error);
	if (rtrUnlikely(!shMemServer))
		return RSSL_RET_FAILURE;		/* rtrShmTransCreate() sets RsslError */

	rsslSrvrImpl->connectionType = RSSL_CONN_TYPE_UNIDIR_SHMEM;
	rsslSrvrImpl->transportInfo = shMemServer;
	rsslSrvrImpl->Server.socketId = shMemServer->_bindFD;
	rsslSrvrImpl->Server.userSpecPtr = opts->userSpecPtr; 
	rsslSrvrImpl->Server.state = RSSL_CH_STATE_ACTIVE;
	rsslSrvrImpl->Server.portNumber = 0; 
	return RSSL_RET_SUCCESS;
}

/* rssl ShMem Connect */
RsslRet rsslUniShMemConnect(rsslChannelImpl* rsslChnlImpl, RsslConnectOptions *opts, RsslError *error)
{
	rtrShmTransClient *channelShMemClient = 0;
	rtrShmAttachOpts shMemOpts;
	RsslRet ret;
	RsslInt32 nBytes;

	shMemOpts.maxReaderRetryThreshhold = 0;

	/* For all connection types we minimally need this value populated (unified.serviceName == segmented.recvPort) */
	if (RSSL_NULL_PTR(opts->connectionInfo.unified.serviceName, "rsslConnect", "opts->connectionInfo.unified.serviceName", error))
		return RSSL_RET_FAILURE;

	/* set up shared memory connect options */
	shMemOpts.majorVersion = opts->majorVersion;
	shMemOpts.minorVersion = opts->minorVersion;
	shMemOpts.protocolType = opts->protocolType;
	shMemOpts.maxReaderSeqNumLag = opts->shmemOpts.maxReaderLag;
	shMemOpts.userSpecPtr = rsslChnlImpl;
	shMemOpts.blockingIO = opts->blocking;
#ifndef SHM_PIPE	/* used when using a notifier */
	shMemOpts.maxReaderRetryThreshhold = opts->shmemOpts.maxReaderRetryThreshhold;
#endif

	if (opts->connectionInfo.unified.interfaceName)
		nBytes = snprintf(shMemOpts.shMemKey, sizeof(shMemOpts.shMemKey), "%s%s", 
							opts->connectionInfo.unified.interfaceName, opts->connectionInfo.unified.serviceName);
	else
		nBytes = snprintf(shMemOpts.shMemKey, sizeof(shMemOpts.shMemKey), "%s", opts->connectionInfo.unified.serviceName);

	if ((nBytes >= (RsslInt32)sizeof(shMemOpts.shMemKey)) || nBytes < 0)
	{
		_rsslSetError(error, NULL, RSSL_RET_FAILURE,  0);
		snprintf(error->text, MAX_RSSL_ERROR_TEXT, "<%s:%d> rsslUniShMemConnect() bad interface and/or service name\n", __FILE__, __LINE__);
		return RSSL_RET_FAILURE;
	}
		
	/* Shared memory connection type */

	if ((channelShMemClient = rtrShmTransAttach(&shMemOpts, error)) == 0)
	{
		/* the error structure was populated in rtrShmTransAttach() */
		return RSSL_RET_FAILURE;
	}

	rsslChnlImpl->transportClientInfo = channelShMemClient;
	rsslChnlImpl->maxMsgSize = channelShMemClient->circularBufferClient.maxBufSize - sizeof(rtrShmBuffer);
	rsslChnlImpl->maxGuarMsgs = channelShMemClient->circularBufferClient.numBuffers;

	rsslChnlImpl->Channel.pingTimeout = *channelShMemClient->pingTimeout;		/* the server dictates the ping timeout */
	rsslChnlImpl->Channel.majorVersion = *channelShMemClient->majorVersion;
	rsslChnlImpl->Channel.minorVersion = *channelShMemClient->minorVersion;
	rsslChnlImpl->Channel.protocolType = *channelShMemClient->protocolType;
	rsslChnlImpl->Channel.connectionType = RSSL_CONN_TYPE_UNIDIR_SHMEM;
	
	if (*channelShMemClient->flags & RSSL_SHM_SERVER_PING_ENABLED)
		rsslChnlImpl->rsslFlags = SERVER_TO_CLIENT;
	else
		rsslChnlImpl->rsslFlags = 0;

	rsslChnlImpl->Channel.socketId = channelShMemClient->clientFD;
	rsslChnlImpl->Channel.userSpecPtr = opts->userSpecPtr;
	rsslChnlImpl->Channel.state	= RSSL_CH_STATE_INITIALIZING;

	/* if its a nonblocking connect, we are done here */
	if (opts->blocking == 0)
		return RSSL_RET_SUCCESS;

	while (rsslChnlImpl->Channel.state == RSSL_CH_STATE_INITIALIZING)
	{
		/* for a blocking connection, call rsslInitChannel (which will set state to active) for the user */
		if ((ret = rsslUniShMemInitChannel(rsslChnlImpl, NULL/* not used */, error )) != RSSL_RET_SUCCESS)
			return RSSL_RET_FAILURE;	/* error will have been set by rsslUniShMemInitChannel() */
	}
	return RSSL_RET_SUCCESS;
}

/* rssl ShMem accept */
rsslChannelImpl* rsslUniShMemAccept(rsslServerImpl *rsslSrvrImpl, RsslAcceptOptions *opts, RsslError *error)
{
	rsslChannelImpl	*rsslChnlImpl;
	RsslRet ret;
	rtrShmTransServer *shMemServer = rsslSrvrImpl->transportInfo;
	rtrShmTransServer *channelShMemServer;

	if ((rsslChnlImpl = _rsslNewChannel()) == 0)
	{
		_rsslSetError(error, (RsslChannel*)(&rsslSrvrImpl->Server), RSSL_RET_FAILURE,  0);
		snprintf(error->text, MAX_RSSL_ERROR_TEXT, "<%s:%d> rsslAccept() could not allocate memory for new channel\n", __FILE__, __LINE__);
		return NULL;
	}	

	if (rtrShmTransAccept(shMemServer, error) < RSSL_RET_SUCCESS)
	{
		/* rtrShmTransAccept fills in the error structure. So we dont need to. */
		_rsslReleaseChannel(rsslChnlImpl);
		return NULL;
	}

	/* successfully connected */
	rsslChnlImpl->Channel.connectionType = rsslSrvrImpl->connectionType;
	channelShMemServer = shMemServer;
	rsslChnlImpl->transportServerInfo = channelShMemServer;
	channelShMemServer->_hasBuffer = 0;

	/* these are all set from shMemServer and are the same values passed in on rsslBind */
	rsslChnlImpl->maxMsgSize = channelShMemServer->circularBufferServer->maxBufSize - sizeof(rtrShmBuffer);
	rsslChnlImpl->maxGuarMsgs = channelShMemServer->circularBufferServer->numBuffers;
	rsslChnlImpl->Channel.pingTimeout = *channelShMemServer->pingTimeout;
	rsslChnlImpl->Channel.majorVersion = *channelShMemServer->majorVersion;
	rsslChnlImpl->Channel.minorVersion = *channelShMemServer->minorVersion;
	rsslChnlImpl->Channel.protocolType = *channelShMemServer->protocolType;

	if (*channelShMemServer->flags & RSSL_SHM_SERVER_PING_ENABLED)
		rsslChnlImpl->rsslFlags = SERVER_TO_CLIENT;
	else
		rsslChnlImpl->rsslFlags = 0;

	rsslChnlImpl->Channel.socketId = channelShMemServer->_acceptFD;
	
	rsslChnlImpl->Channel.clientHostname = (char*)_rsslMalloc(32);
	strncpy(rsslChnlImpl->Channel.clientHostname, "localhost", 32);
	rsslChnlImpl->Channel.clientIP = (char*)_rsslMalloc(32);
	strncpy(rsslChnlImpl->Channel.clientIP, "127.0.0.1", 32);
	rsslChnlImpl->Channel.port = 0;

	if (!opts->userSpecPtr)
	{
		rsslChnlImpl->Channel.userSpecPtr = rsslSrvrImpl->Server.userSpecPtr; 
	}
	else
	{
		rsslChnlImpl->Channel.userSpecPtr = opts->userSpecPtr;
	}

	rsslChnlImpl->channelFuncs = rsslSrvrImpl->channelFuncs;
	rsslChnlImpl->Channel.state	= RSSL_CH_STATE_INITIALIZING;

	/* if its a nonblocking connect, then set to initializing and exit */
	/* the user will need to call rsslInitChannel next */
	if (channelShMemServer->channelsBlocking == 0)
		return rsslChnlImpl;

	/* for a blocking connection, call rsslInitChannel for the user */
	if ((ret = rsslUniShMemInitChannel(rsslChnlImpl, NULL/* not used */, error )) != RSSL_RET_SUCCESS)
		return NULL;

	return rsslChnlImpl;
}

/* rssl ShMem ReConnect (for tunneling, basically a no-op) */
RsslRet rsslUniShMemReconnect(rsslChannelImpl *rsslChnlImpl, RsslError *error)
{
	return RSSL_RET_SUCCESS;
}

/* rssl ShMem InitChannel */
RsslRet rsslUniShMemInitChannel(rsslChannelImpl *rsslChnlImpl, RsslInProgInfo *inProg, RsslError *error )
{
	rtrShmTransServer *channelShMemServer = rsslChnlImpl->transportServerInfo;
	rtrShmTransClient *channelShMemClient = rsslChnlImpl->transportClientInfo;

	/* state may have been set to active by bind and connect */
	if (rsslChnlImpl->Channel.state == RSSL_CH_STATE_ACTIVE)
		return RSSL_RET_SUCCESS;

	if (rsslChnlImpl->Channel.state != RSSL_CH_STATE_INITIALIZING)
	{
		error->rsslErrorId = RSSL_RET_FAILURE;
		snprintf(error->text, MAX_RSSL_ERROR_TEXT, "<%s:%d> rsslUniShMemInitChannel failed. Unexpected Channel state(%d)\n", __FILE__, __LINE__, rsslChnlImpl->Channel.state);
		return RSSL_RET_FAILURE;
	}

#if SHM_SOCKET_TCP	/* socket notifier knows how to do non-blocking connect/accept */
	if (channelShMemServer)
	{
		/* if its a blocking channel, then we must have fully initialized when we did the accept() */
		if (channelShMemServer->channelsBlocking)
		{
			rsslChnlImpl->Channel.state = RSSL_CH_STATE_ACTIVE;
			return RSSL_RET_SUCCESS;
		}
		
		/* try to write a notification byte to the client, if it works, then the TCP channel is fully initialized */
		/* sending a notification byte here gets the consumer ready to read before calling rsslWrite() */
		fprintf(stderr, "writing a byte during rsslUniShMemInitChannel()\n");
		if (send(channelShMemServer->_acceptFD, "1", 1, 0) != 1)
		{
			if (errno != EAGAIN && errno != EWOULDBLOCK)
			{
				error->rsslErrorId = RSSL_RET_FAILURE;
				snprintf(error->text, MAX_RSSL_ERROR_TEXT, "<%s:%d> rsslUniShMemInitChannel failed. Unexpected errno(%d)\n", __FILE__, __LINE__, errno);
				rsslChnlImpl->Channel.state = RSSL_CH_STATE_CLOSED;
				return RSSL_RET_FAILURE;			/* unexpected error */
			}
			snprintf(error->text, MAX_RSSL_ERROR_TEXT, "notification write failed in rsslUniShMemInitChannel. Channel still initializing(errno = %d)\n", errno);
			return RSSL_RET_CHAN_INIT_IN_PROGRESS;	/* cant write a byte, so the channel is still initializing */
		}
		/* we successfully sent a byte */
		writeByteCnt++;
		*channelShMemServer->byteWritten = 1;
	}
	else	/* client */
	{
		rsslChnlImpl->Channel.state = RSSL_CH_STATE_ACTIVE;
	}
#endif
#if SHM_PIPE
	/* if its a server, take the byte out of the accept pipe so the user doesnt call rsslAccept() again */
	if (channelShMemServer)
	{
		char temp[4];
		RsslInt32 ret;

		ret = rssl_pipe_read(&channelShMemServer->_acceptPipe, temp, 1);
		if (ret == 0 || (ret == -1 && (errno == EAGAIN || errno == EWOULDBLOCK)))
		{
			/* this shouldnt happen. Do not set state to active if this does happen */
			return RSSL_RET_SUCCESS; /* assume the byte isnt ready yet, return success */
		}
		else if (ret < 0) 
		{
			snprintf(error->text, MAX_RSSL_ERROR_TEXT, "pipe read failed (errno = %d)\n", errno);
			_rsslSetError(error, &rsslChnlImpl->Channel, RSSL_RET_FAILURE, 0);
			rsslChnlImpl->Channel.state = RSSL_CH_STATE_CLOSED;
			return RSSL_RET_FAILURE;
		}
	}
#endif

	if (channelShMemServer)
	{
		/* copy the component version into the shmem seg, set the channel to active, and set the seg as initialized */
		if (rsslChnlImpl->componentVer.componentVersion.length > 0)
		{
			*channelShMemServer->serverComponentVersionLen = rsslChnlImpl->componentVer.componentVersion.length;
			memcpy(channelShMemServer->serverComponentVersion,
					rsslChnlImpl->componentVer.componentVersion.data, 
					rsslChnlImpl->componentVer.componentVersion.length);
		}
		*channelShMemServer->flags |= RSSL_SHM_SERVER_INITIALIZED;
		rsslChnlImpl->Channel.state = RSSL_CH_STATE_ACTIVE;
	}
	else
	{
		if (channelShMemClient && (*channelShMemClient->flags & RSSL_SHM_SERVER_SHUTDOWN))
		{
			snprintf(error->text, MAX_RSSL_ERROR_TEXT, "server has shutdown\n");
			_rsslSetError(error, &rsslChnlImpl->Channel, RSSL_RET_FAILURE, 0);
			rsslChnlImpl->Channel.state = RSSL_CH_STATE_CLOSED;
			return RSSL_RET_FAILURE;
		}

		/* set the channel state to active once the server says the shmem seg is ready */
		if (channelShMemClient && (*channelShMemClient->flags & RSSL_SHM_SERVER_INITIALIZED))
			rsslChnlImpl->Channel.state = RSSL_CH_STATE_ACTIVE;
	}
	return RSSL_RET_SUCCESS;
}

/* rssl ShMem CloseChannel */
RsslRet rsslUniShMemCloseChannel(rsslChannelImpl* rsslChnlImpl, RsslError *error)
{
	if (!rsslChnlImpl->transportClientInfo && !rsslChnlImpl->transportServerInfo)
	{
		_rsslSetError(error, 0, RSSL_RET_FAILURE, 0);
		snprintf(error->text, MAX_RSSL_ERROR_TEXT, "<%s:%d> rsslUniShMemCloseChannel failed due to no shared memory transport.\n", __FILE__, __LINE__);
		return -1;
	}
	
	rsslChnlImpl->Channel.state = RSSL_CH_STATE_INACTIVE;

	if (rsslChnlImpl->transportClientInfo)
	{	/* client */
		rtrShmTransDetach(((rtrShmTransClient*)rsslChnlImpl->transportClientInfo), error);
		rsslChnlImpl->transportClientInfo = 0;
	}
	if (rsslChnlImpl->transportServerInfo)
	{	/* server */
		rtrShmTransServer *channelShMemServer = rsslChnlImpl->transportServerInfo;
		rssl_pipe_close(&channelShMemServer->_acceptPipe);
		channelShMemServer->_hasBuffer = 0;
#ifdef SHM_BYTE_COUNT
		printf("writeCnt = %u\t\twriteByteCnt = %u\n", writeCnt, writeByteCnt);
#endif
	}

	/* Release memory allocated by rsslUniShMemAccept */
	if (rsslChnlImpl->Channel.clientHostname) 
	{
		_rsslFree(rsslChnlImpl->Channel.clientHostname);
	}

	if (rsslChnlImpl->Channel.clientIP)
	{
		_rsslFree(rsslChnlImpl->Channel.clientIP);
	}

	return RSSL_RET_SUCCESS;
}

/* rssl ShMem read */
RSSL_RSSL_UNIDIRECTION_SHMEM_IMPL_FAST(RsslBuffer*) rsslUniShMemRead(rsslChannelImpl* rsslChnlImpl, RsslReadOutArgs *readOutArgs, RsslRet *readRet, RsslError *error)
{
	rtrShmBuffer* buf;
	rtrShmTransClient *channelShMemClient = rsslChnlImpl->transportClientInfo;

#ifdef SHM_BYTE_COUNT
	readCnt++;
#endif
	/* we are shared memory - only clients can read */
	if (rtrUnlikely(!channelShMemClient))
	{
		_rsslSetError(error, 0, RSSL_RET_FAILURE, 0);
		snprintf(error->text, MAX_RSSL_ERROR_TEXT, "<%s:%d> rsslUniShMemRead failed due to no shared memory transport.\n", __FILE__, __LINE__);
		*readRet = RSSL_RET_FAILURE;
		return NULL;
	}

	if (multiThread == RSSL_LOCK_GLOBAL_AND_CHANNEL)
	{
#ifdef WIN32
	  EnterCriticalSection(&rsslChnlImpl->chanMutex);
#else
	  if (RSSL_MUTEX_LOCK(&rsslChnlImpl->chanMutex))
	  {
		*readRet = RSSL_RET_READ_IN_PROGRESS;
		return NULL;
	  }
#endif
	}

	/* read the next buffer */
	buf = rtrShmTransClientRead(channelShMemClient, &rsslChnlImpl->Channel, readRet, error);
	if (multiThread == RSSL_LOCK_GLOBAL_AND_CHANNEL)
	  (void) RSSL_MUTEX_UNLOCK(&rsslChnlImpl->chanMutex);
	
	if (rtrUnlikely(buf == NULL))
	{
		/* check the return code */
		switch (error->rsslErrorId)
		{
		case RSSL_RET_SLOW_READER:
			/* very slow readers are detached from shared memory */
			*readRet = RSSL_RET_SLOW_READER;
#ifdef SHM_BYTE_COUNT
			printf("readCnt = %lu\t\treadSuccess = %u\t\treadByteCnt = %u\nreadByteSuccess = %u\t\treadByteFailed = %u\n", readCnt, readSuccess, readByteCnt, readByteSuccess, readByteFailed);
#endif
			break;

		case RSSL_RET_FAILURE:
			/* readers are also detached if the provider exits and we've read everything in shmem */
			*readRet = RSSL_RET_FAILURE;
			break;

		case RSSL_RET_READ_WOULD_BLOCK:
		default:
			/* shmem is empty and there was no error */
#if SHM_NAMEDPIPE || SHM_SOCKET_UDP || SHM_SOCKET_TCP

			/* This controls Hybrid mode */
			/* This is a hybrid of a notifier and "burn a core" methods */
			/* We keep telling the client to read until we reach a threshhold */
			if (channelShMemClient->readRetries++ < channelShMemClient->maxReaderRetryThreshhold)
			{
				*readRet = 1;	/* call us again without select */
				return NULL;
			}

			channelShMemClient->readRetries = 0;
			*readRet = RSSL_RET_READ_WOULD_BLOCK;
			if (RTR_ATOMIC_COMPARE_AND_SWAP ((*channelShMemClient->byteWritten), 1L, 0L) == 1)
			{
				char temp[10];
				RsslInt32 ret;

				readByteCnt++;

#if SHM_SOCKET_UDP || SHM_SOCKET_TCP
				ret = recv(channelShMemClient->clientFD, temp, 1, 0);
#else
				ret = read(channelShMemClient->clientFD, temp, 1);
#endif
				if (ret > 0) 
				{
#if 0	/* TCP_QUICKACK test */
					RsslInt32 flag = 1;

					if (setsockopt( channelShMemClient->clientFD, IPPROTO_TCP, TCP_QUICKACK,(char *) &flag, sizeof(RsslInt32)) == -1)
					{
						snprintf(error->text, MAX_RSSL_ERROR_TEXT, "rsslRead() setsockopt TCP_QUICKACK failed (errno = %d)n", errno);
						_rsslSetError(error, &rsslChnlImpl->Channel, RSSL_RET_FAILURE, 0);
						rsslCloseChannel(&rsslChnlImpl->Channel, error);
						*readRet = RSSL_RET_FAILURE;
						return NULL;
					}
#endif
					readByteSuccess++;
					if (ret > 1) fprintf(stderr, "we got too many! (ret = %d)\n", ret);
				}
				else if (ret == 0 || (ret == -1 && (errno == EAGAIN || errno == EWOULDBLOCK)))
				{
					*channelShMemClient->byteWritten = 1;
					*readRet = 1;	/* We didnt get a byte, so call us again */
					readByteFailed++;
				}
				else
				{
					snprintf(error->text, MAX_RSSL_ERROR_TEXT, "notifier read failed (errno = %d)\n", errno);
					_rsslSetError(error, &rsslChnlImpl->Channel, RSSL_RET_FAILURE, 0);
					rsslCloseChannel(&rsslChnlImpl->Channel, error);
					*readRet = RSSL_RET_FAILURE;
					return NULL;
				}
			}
#else
			*readRet = 1;
#endif
			break;
		}
		return NULL;
	}

#ifdef SHM_BYTE_COUNT
	readSuccess++;
#endif
	/* if we got here, we read something */
	channelShMemClient->readRetries = 0;
	if (rtrUnlikely(buf->flags & RSSL_SHMBUF_PING))
	{
		*readRet = RSSL_RET_READ_PING;
		return NULL;	/* we read a ping, but we return it as a NULL buffer */
	}

	rsslChnlImpl->returnBuffer.length = buf->length;
	rsslChnlImpl->returnBuffer.data = buf->buffer;

	if (readOutArgs != NULL)
	{
		readOutArgs->bytesRead = buf->length;
		readOutArgs->uncompressedBytesRead = buf->length;
	}

	if (rtrUnlikely((rsslChnlImpl->debugFlags & RSSL_DEBUG_RSSL_DUMP_IN) && (rsslChnlImpl->returnBuffer.length )))
		(*(rsslUniShMemDumpInFunc))(__FUNCTION__, rsslChnlImpl->returnBuffer.data, rsslChnlImpl->returnBuffer.length, rsslChnlImpl->Channel.socketId);

	/* readRet was set by rtrShmTransClientRead() to the number of messages left to read */
	return &(rsslChnlImpl->returnBuffer);
}

/* rssl ShMem Write */
RSSL_RSSL_UNIDIRECTION_SHMEM_IMPL_FAST(RsslRet) rsslUniShMemWrite(rsslChannelImpl *rsslChnlImpl, rsslBufferImpl *rsslBufImpl, RsslWriteInArgs *writeInArgs, RsslWriteOutArgs *writeOutArgs, RsslError *error)
{
	rtrShmBuffer *shmBuffer = rsslBufImpl->bufferInfo;
	rtrShmTransServer *channelShMemServer = rsslChnlImpl->transportServerInfo;
	
	if (rtrUnlikely(!channelShMemServer))
	{
		_rsslSetError(error, &rsslChnlImpl->Channel, RSSL_RET_FAILURE, 0);
		snprintf(error->text, MAX_RSSL_ERROR_TEXT, "<%s:%d> rsslWrite() RSSL shared memory server not available", __FILE__, __LINE__);
		return RSSL_RET_FAILURE;
	}
#ifdef SHM_BYTE_COUNT
	writeCnt++;
#endif
	/* make sure the length they gave us doesnt exceed the buffer */
	if (rtrUnlikely(rsslBufImpl->buffer.length > shmBuffer->maxLength))
	{
		_rsslSetError(error, &rsslChnlImpl->Channel, RSSL_RET_BUFFER_TOO_SMALL, 0);
		snprintf(error->text, MAX_RSSL_ERROR_TEXT, "Buffer too small - %d bytes written into buffer of %d bytes\n", rsslBufImpl->buffer.length, shmBuffer->maxLength);
		rsslChnlImpl->Channel.state = RSSL_CH_STATE_CLOSED;
		return RSSL_RET_BUFFER_TOO_SMALL;
	}
	shmBuffer->length = rsslBufImpl->buffer.length;

	rtrShmTransServerWrite(channelShMemServer);

	writeOutArgs->bytesWritten = shmBuffer->length;
	writeOutArgs->uncompressedBytesWritten = shmBuffer->length;

	if (rtrUnlikely((rsslChnlImpl->debugFlags & RSSL_DEBUG_RSSL_DUMP_OUT) && (shmBuffer->length)))
		(*(rsslUniShMemDumpOutFunc))(__FUNCTION__, shmBuffer->buffer, shmBuffer->length, rsslChnlImpl->Channel.socketId);
	
	/* since it was a successful write, free the RsslBuffer */
	/* remove it from the active buffer list and then add to free buffer list */
	_rsslCleanBuffer(rsslBufImpl);
	if (rtrUnlikely(memoryDebug)) printf("adding to freeBufferList and removing from activeBufferList\n");

	if (rtrUnlikely(multiThread == RSSL_LOCK_GLOBAL_AND_CHANNEL))
	  (void) RSSL_MUTEX_LOCK(&rsslChnlImpl->chanMutex);

	if (rsslQueueLinkInAList(&(rsslBufImpl->link1)))
		rsslQueueRemoveLink(&(rsslChnlImpl->activeBufferList), &(rsslBufImpl->link1));

	rsslQueueAddLinkToBack(&(rsslChnlImpl->freeBufferList), &(rsslBufImpl->link1));

#if SHM_NAMEDPIPE
	if (RTR_ATOMIC_COMPARE_AND_SWAP ((*channelShMemServer->byteWritten), 0L, 1L) == 0)
	{
		if (write(channelShMemServer->_acceptFD, "1", 1) != 1)
		{
			snprintf(error->text, MAX_RSSL_ERROR_TEXT, "notification write failed (errno = %d)\n", errno);
			_rsslSetError(error, &rsslChnlImpl->Channel, RSSL_RET_FAILURE, 0);
			rsslCloseChannel(&rsslChnlImpl->Channel, error); 
			return RSSL_RET_FAILURE;
		}
		writeByteCnt++;
	}
#endif
#if SHM_SOCKET_TCP
	if (RTR_ATOMIC_COMPARE_AND_SWAP ((*channelShMemServer->byteWritten), 0L, 1L) == 0)
	{
		if (send(channelShMemServer->_acceptFD, "1", 1, 0) != 1)
		{
			snprintf(error->text, MAX_RSSL_ERROR_TEXT, "notification write failed (errno = %d)\n", errno);
			_rsslSetError(error, &rsslChnlImpl->Channel, RSSL_RET_FAILURE, 0);
			rsslCloseChannel(&rsslChnlImpl->Channel, error);
			return RSSL_RET_FAILURE;
		}
		writeByteCnt++;
	}
#endif
#if SHM_SOCKET_UDP
	if (RTR_ATOMIC_COMPARE_AND_SWAP ((*channelShMemServer->byteWritten), 0L, 1L) == 0)
	{
		if (sendto(channelShMemServer->_acceptFD, "1", 1, /* MSG_DONTWAIT */0, (struct sockaddr *)&channelShMemServer->cliAddr, sizeof (channelShMemServer->cliAddr)) == -1)
		{	
			_rsslSetError(error, &rsslChnlImpl->Channel, RSSL_RET_FAILURE, 0);
			snprintf(error->text, MAX_RSSL_ERROR_TEXT, "sendto write failed (errno = %d)\n", errno);
			rsslCloseChannel(&rsslChnlImpl->Channel, error);
			return RSSL_RET_FAILURE;
		}
		writeByteCnt++;
	}
#endif
	channelShMemServer->_hasBuffer = 0;
	if (rtrUnlikely(multiThread == RSSL_LOCK_GLOBAL_AND_CHANNEL))
	  (void) RSSL_MUTEX_UNLOCK(&rsslChnlImpl->chanMutex);

	/* always return success(0) since there would never be any bytes left to be written */
	return RSSL_RET_SUCCESS;
}


/* rssl ShMem GetBuffer */
RSSL_RSSL_UNIDIRECTION_SHMEM_IMPL_FAST(rsslBufferImpl*) rsslUniShMemGetBuffer(rsslChannelImpl *rsslChnlImpl, RsslUInt32 size, RsslBool packedBuffer, RsslError *error)
{
	rsslBufferImpl *rsslBufImpl;
	rtrShmBuffer *shmBuffer;
	rtrShmTransServer *channelShMemServer = rsslChnlImpl->transportServerInfo;

	if (rtrUnlikely(!channelShMemServer))
	{
		_rsslSetError(error, 0, RSSL_RET_FAILURE, 0);
		snprintf(error->text, MAX_RSSL_ERROR_TEXT, "<%s:%d> rsslUniShMemGetBuffer failed due to no shared memory transport.\n", __FILE__, __LINE__);
		return NULL;
	}

	if (rtrUnlikely(size > rsslChnlImpl->maxMsgSize))
	{
		_rsslSetError(error, &rsslChnlImpl->Channel, RSSL_RET_FAILURE, 0);
		snprintf(error->text, MAX_RSSL_ERROR_TEXT, "<%s:%d> rsslGetBuffer() requested buffer size of %d exceeds the maxMsgSize of %d. Fragmented messages are not supported for shared memory transport.\n", __FILE__, __LINE__, size, rsslChnlImpl->maxMsgSize);
		return NULL;
	}

	if (rtrUnlikely(packedBuffer))
	{
		_rsslSetError(error, &rsslChnlImpl->Channel, RSSL_RET_FAILURE, 0);
		snprintf(error->text, MAX_RSSL_ERROR_TEXT, "<%s:%d> rsslGetBuffer() packed messages are not supported for shared memory transport.\n", __FILE__, __LINE__);
		return NULL;
	}

	if (rtrUnlikely(channelShMemServer->_hasBuffer != 0))
	{
		_rsslSetError(error, &rsslChnlImpl->Channel, RSSL_RET_BUFFER_NO_BUFFERS, 0);
		snprintf(error->text, MAX_RSSL_ERROR_TEXT, "<%s:%d> rsslGetBuffer() Cannot allocate more than one rsslBuffer for shmem connections.\n", __FILE__, __LINE__);
		return NULL;
	}

	shmBuffer = rtrShmTransGetFreeBuffer(channelShMemServer);

	/* successful - now allocate rsslbuffer */
	rsslBufImpl = _rsslUniShMemNewBuffer(rsslChnlImpl);

	if (rtrUnlikely(rsslBufImpl == NULL))
	{
		/* We didnt get an rsslBuffer */
		/* dont do anything with shmem. The next rsslGetBuffer will get the same shmem buffer */
		_rsslSetError(error, &rsslChnlImpl->Channel, RSSL_RET_BUFFER_NO_BUFFERS, 0);
		snprintf(error->text, MAX_RSSL_ERROR_TEXT, "<%s:%d> rsslGetBuffer() Cannot allocate more than one rsslBuffer for shmem connections.\n", __FILE__, __LINE__);
		return NULL;
	}

	channelShMemServer->_hasBuffer = 1;
	rsslBufImpl->buffer.data = shmBuffer->buffer;
	rsslBufImpl->buffer.length = size;
	rsslBufImpl->bufferInfo = shmBuffer;
	shmBuffer->flags = 0;
	rsslBufImpl->packingOffset = 0;
	rsslBufImpl->totalLength = size;

	return rsslBufImpl;
}

/* rssl ShMem ReleaseBuffer */
RSSL_RSSL_UNIDIRECTION_SHMEM_IMPL_FAST(RsslRet) rsslUniShMemReleaseBuffer(rsslChannelImpl *rsslChnlImpl, rsslBufferImpl *rsslBufImpl, RsslError *error)
{

	if (rtrUnlikely(!rsslChnlImpl->transportServerInfo))
	{
		_rsslSetError(error, 0, RSSL_RET_FAILURE, 0);
		snprintf(error->text, MAX_RSSL_ERROR_TEXT, "<%s:%d> rsslUniShMemReleaseBuffer failed due to no shared memory transport.\n", __FILE__, __LINE__);
		return RSSL_RET_FAILURE;
	}

	((rtrShmTransServer*)rsslChnlImpl->transportServerInfo)->_hasBuffer = 0;
	return RSSL_RET_SUCCESS;
}

/* rssl ShMem PackBuffer */
RSSL_RSSL_UNIDIRECTION_SHMEM_IMPL_FAST(RsslBuffer*) rsslUniShMemPackBuffer(rsslChannelImpl *rsslChnlImpl, rsslBufferImpl *rsslBufImpl, RsslError *error)
{
	if (rtrUnlikely(!rsslChnlImpl->transportServerInfo))
	{
		_rsslSetError(error, 0, RSSL_RET_FAILURE, 0);
		snprintf(error->text, MAX_RSSL_ERROR_TEXT, "<%s:%d> rsslUniShMemPackBuffer failed due to no shared memory transport.\n", __FILE__, __LINE__);
		return NULL;
	}
	_rsslSetError(error, &rsslChnlImpl->Channel, RSSL_RET_FAILURE, 0);
	snprintf(error->text, MAX_RSSL_ERROR_TEXT, "<%s:%d> rsslPackBuffer() packed messages currently not supported for shared memory transport.\n", __FILE__, __LINE__);

	return NULL;
}

/* rssl ShMem Flush */
/* Since we are writing directly to memory, we dont need to flush. All written messages are available to read immediately */
RSSL_RSSL_UNIDIRECTION_SHMEM_IMPL_FAST(RsslRet) rsslUniShMemFlush(rsslChannelImpl *rsslChnlImpl, RsslError *error)
{
	return RSSL_RET_SUCCESS;
}

/* rssl ShMem Ping */
RSSL_RSSL_UNIDIRECTION_SHMEM_IMPL_FAST(RsslRet) rsslUniShMemPing(rsslChannelImpl *rsslChnlImpl, RsslError *error)
{
	rtrShmBuffer *shmBuffer;
	rtrShmTransServer *channelShMemServer = rsslChnlImpl->transportServerInfo;

	if (rtrUnlikely(!channelShMemServer))
	{
		_rsslSetError(error, 0, RSSL_RET_FAILURE, 0);
		snprintf(error->text, MAX_RSSL_ERROR_TEXT, "<%s:%d> rsslUniShMemPing failed due to no shared memory transport.\n", __FILE__, __LINE__);
		return RSSL_RET_FAILURE;
	}

	shmBuffer = rtrShmTransGetFreeBuffer(channelShMemServer);
	shmBuffer->flags = RSSL_SHMBUF_PING;
	shmBuffer->length = 0;
	rtrShmTransServerWrite(channelShMemServer);
			
	return RSSL_RET_SUCCESS;
}

/* rssl ShMem GetChannelInfo */
RSSL_RSSL_UNIDIRECTION_SHMEM_IMPL_FAST(RsslRet) rsslUniShMemGetChannelInfo(rsslChannelImpl *rsslChnlImpl, RsslChannelInfo *info, RsslError *error)
{
	RsslInt32 i;

	if (rtrUnlikely(rsslChnlImpl->transportServerInfo == 0 && rsslChnlImpl->transportClientInfo == 0))
	{
		_rsslSetError(error, 0, RSSL_RET_FAILURE, 0);
		snprintf(error->text, MAX_RSSL_ERROR_TEXT, "<%s:%d> rsslUniShMemGetChannelInfo failed due to no shared memory transport.\n", __FILE__, __LINE__);
		return RSSL_RET_FAILURE;
	}

	info->maxFragmentSize = rsslChnlImpl->maxMsgSize;
	info->guaranteedOutputBuffers = rsslChnlImpl->maxGuarMsgs;
	info->compressionType = RSSL_COMP_NONE;
	info->compressionThreshold = 0;
	info->numInputBuffers = rsslChnlImpl->maxGuarMsgs;
	info->maxOutputBuffers = rsslChnlImpl->maxGuarMsgs;
	info->tcpRecvBufSize = 0;
	info->tcpSendBufSize = 0;
	info->compressionType = RSSL_COMP_NONE;	/* we dont support compression with shmem connection */
	info->compressionThreshold = 0;
	info->pingTimeout = rsslChnlImpl->Channel.pingTimeout;

	if (rsslChnlImpl->rsslFlags & SERVER_TO_CLIENT)
	{
		info->serverToClientPings = RSSL_TRUE;
	}
	else
	{
		info->serverToClientPings = RSSL_FALSE;
	}

	if (rsslChnlImpl->rsslFlags & CLIENT_TO_SERVER)
	{
		info->clientToServerPings = RSSL_TRUE;
	}
	else
	{
		info->clientToServerPings = RSSL_FALSE;
	}

	if (rsslChnlImpl->transportClientInfo)
	{	/* client */
		info->sysRecvBufSize = rsslChnlImpl->maxGuarMsgs * rsslChnlImpl->maxMsgSize;
	}
	else
	{
		info->sysRecvBufSize = 0;
	}

	if (rsslChnlImpl->transportServerInfo)
	{	/* server */
		info->sysSendBufSize = rsslChnlImpl->maxGuarMsgs * rsslChnlImpl->maxMsgSize;
	}
	else
	{
		info->sysSendBufSize = 0;
	}

	/* get connected component version info here */
	/* this is a dynamic array because of multicast - if memory is here, it means user already called
	 * get channel info so just reuse it.  it will be cleaned up when connection is closed */
	if (rsslChnlImpl->transportClientInfo)
	{
		rtrShmTransClient *channelShMemClient = rsslChnlImpl->transportClientInfo;

		if (!rsslChnlImpl->componentInfo)
		{
			rsslChnlImpl->componentInfo = (RsslComponentInfo **)_rsslMalloc(sizeof(void*));
			rsslChnlImpl->componentInfo[0] = (RsslComponentInfo *)_rsslMalloc(sizeof(RsslComponentInfo));
		}
		rsslChnlImpl->componentInfo[0]->componentVersion.length = *channelShMemClient->serverComponentVersionLen;
		rsslChnlImpl->componentInfo[0]->componentVersion.data = channelShMemClient->serverComponentVersion;

		info->componentInfoCount = 1;
		info->componentInfo = rsslChnlImpl->componentInfo;
	}
	else
	{
		info->componentInfoCount = 0;
		info->componentInfo = 0;
	}

	/* clear out other fields not used by shmem connection */
	for (i=0; i < RSSL_RSSL_MAX_FLUSH_STRATEGY; i++)
	{
		info->priorityFlushStrategy[i] = 0;
	}
	
	info->encryptionProtocol = RSSL_ENC_NONE;

	/* clear other stats types */
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

/* rssl ShMem GetServerInfo */
RSSL_RSSL_UNIDIRECTION_SHMEM_IMPL_FAST(RsslRet) rsslUniShMemGetSrvrInfo(rsslServerImpl *rsslSrvrImpl, RsslServerInfo *info, RsslError *error)
{
	rtrShmTransServer *shMemServer = rsslSrvrImpl->transportInfo;

	if (!shMemServer)
	{
		_rsslSetError(error, 0, RSSL_RET_FAILURE, 0);
		snprintf(error->text, MAX_RSSL_ERROR_TEXT, "<%s:%d> rsslUniShMemGetSrvrInfo failed due to no shared memory transport.\n", __FILE__, __LINE__);
		return RSSL_RET_FAILURE;
	}

	if (shMemServer->_hasBuffer)
		info->currentBufferUsage = 1;
	else
		info->currentBufferUsage = 0;

	info->peakBufferUsage = 1;		/* shmem has only one rsslBuffer, so the peak cannot be more than that */

	return RSSL_RET_SUCCESS;
}

/* rssl ShMem Buffer Usage */
RSSL_RSSL_UNIDIRECTION_SHMEM_IMPL_FAST(RsslInt32) rsslUniShMemBufferUsage(rsslChannelImpl *rsslChnlImpl, RsslError *error)
{
	if (rsslChnlImpl->transportClientInfo)
	{	/* clients dont call rsslGetBuffer() because they only read */
		return 0;
	}

	if (rsslChnlImpl->transportServerInfo)
	{	/* server - There's only one buffer. Either we have a buffer or we don't */
		return (((rtrShmTransServer *)rsslChnlImpl->transportServerInfo)->_hasBuffer);
	}

	_rsslSetError(error, 0, RSSL_RET_FAILURE, 0);
	snprintf(error->text, MAX_RSSL_ERROR_TEXT, "<%s:%d> rsslUniShMemBufferUsage failed due to no shared memory transport.\n", __FILE__, __LINE__);
	return 0;
}

/* get info about the shared buffer pool. */
RSSL_RSSL_UNIDIRECTION_SHMEM_IMPL_FAST(RsslInt32) rsslUniShMemSrvrBufferUsage(rsslServerImpl *rsslSrvrImpl, RsslError *error)
{
	/* shmem doesnt used a shared buffer pool */
	return 0;
}


RSSL_RSSL_UNIDIRECTION_SHMEM_IMPL_FAST(RsslRet) rsslUniShMemSrvrIoctl(rsslServerImpl *rsslSrvrImpl, RsslIoctlCodes code, void *value, RsslError *error)
{
	return RSSL_RET_SUCCESS;
}

RSSL_RSSL_UNIDIRECTION_SHMEM_IMPL_FAST(RsslRet) rsslUniShMemCloseServer(rsslServerImpl *rsslSrvrImpl, RsslError *error)
{
	rtrShmTransDestroy(rsslSrvrImpl->transportInfo, error);

	return RSSL_RET_SUCCESS;
}

RSSL_RSSL_UNIDIRECTION_SHMEM_IMPL_FAST(RsslRet) rsslUniShMemIoctl(rsslChannelImpl *rsslChnlImpl, RsslIoctlCodes code, void *value, RsslError *error)
{
	return RSSL_RET_SUCCESS;
}



/***************************
 * START PUBLIC ABSTRACTED FUNCTIONS 
 ***************************/

RsslRet rsslUniShMemSetChannelFunctions()
{
	RsslTransportChannelFuncs funcs;

	funcs.channelBufferUsage = rsslUniShMemBufferUsage;
	funcs.channelClose = rsslUniShMemCloseChannel;
	funcs.channelConnect = rsslUniShMemConnect;
	funcs.channelFlush = rsslUniShMemFlush;
	funcs.channelGetBuffer = rsslUniShMemGetBuffer;
	funcs.channelGetInfo = rsslUniShMemGetChannelInfo;
	funcs.channelIoctl = rsslUniShMemIoctl;
	funcs.channelPackBuffer = rsslUniShMemPackBuffer;
	funcs.channelPing = rsslUniShMemPing;
	funcs.channelRead = rsslUniShMemRead;
	funcs.channelReconnect = rsslUniShMemReconnect;
	funcs.channelReleaseBuffer = rsslUniShMemReleaseBuffer;
	funcs.channelWrite = rsslUniShMemWrite;
	funcs.initChannel = rsslUniShMemInitChannel;
	
	return(rsslSetTransportChannelFunc(RSSL_UNIDIRECTION_SHMEM_TRANSPORT,&funcs));
}

RsslRet rsslUniShMemSetServerFunctions()
{
	RsslTransportServerFuncs funcs;

	funcs.serverAccept = rsslUniShMemAccept;
	funcs.serverBind = rsslUniShMemBind;
	funcs.serverIoctl = rsslUniShMemSrvrIoctl;
	funcs.serverGetInfo = rsslUniShMemGetSrvrInfo;
	funcs.serverBufferUsage = rsslUniShMemSrvrBufferUsage;
	funcs.closeServer = rsslUniShMemCloseServer;

	return(rsslSetTransportServerFunc(RSSL_UNIDIRECTION_SHMEM_TRANSPORT,&funcs));
}

/* init, uninit, set function pointers */
RsslRet rsslUniShMemInitialize(RsslLockingTypes lockingType, RsslError *error)
{
	rsslUniShMemSetServerFunctions();
	rsslUniShMemSetChannelFunctions();
	/* nothing to do for this function for now */
	return RSSL_RET_SUCCESS;
}

RsslRet rsslUniShMemUninitialize()
{
	/* nothing to do for this function for now */

	return RSSL_RET_SUCCESS;
}

/* Sets UniShMem debug dump functions */
RsslRet rsslSetUniShMemDebugFunctions(
		void(*dumpRsslIn)(const char *functionName, char *buffer, RsslUInt32 length, RsslSocket socketId),
		void(*dumpRsslOut)(const char *functionName, char *buffer, RsslUInt32 length, RsslSocket socketId),
		RsslError *error)
{
	RsslRet retVal = 0;

	if ((dumpRsslIn && rsslUniShMemDumpInFunc) || (dumpRsslOut && rsslUniShMemDumpOutFunc))
	{
		_rsslSetError(error, NULL, RSSL_RET_FAILURE, 0);
		snprintf(error->text, MAX_RSSL_ERROR_TEXT, "<%s:%d> rsslSetDebugFunctions() Cannot set shared memory Rssl dump functions.\n", __FILE__, __LINE__);

		retVal = RSSL_RET_FAILURE;
	}
	else
	{
		rsslUniShMemDumpInFunc = dumpRsslIn;
		rsslUniShMemDumpOutFunc = dumpRsslOut;
		retVal = RSSL_RET_SUCCESS;
	}

	return retVal;
}




