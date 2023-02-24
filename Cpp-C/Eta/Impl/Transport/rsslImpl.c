/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|          Copyright (C) 2019-2022 Refinitiv. All rights reserved.          --
 *|-----------------------------------------------------------------------------
 */

#include "rtr/rsslTransport.h"
#include "rtr/rsslSocketTransport.h"
#include "rtr/intDataTypes.h"
#include "rtr/rsslUniShMemTransport.h"
#include "rtr/rsslSeqMcastTransport.h"
#include "rtr/rsslQueue.h"

#include "rtr/rsslMessagePackage.h"
#include "decodeRoutines.h"
#include "xmlDump.h"

#include "rtr/rsslErrors.h"
#include "rtr/rsslAlloc.h"
#ifndef NO_ETA_CPU_BIND
#include "rtr/bindthread.h"
#endif
#include "rtr/rwfNetwork.h"
#include "curl/curl.h"
#include "rtr/ripcssljit.h"

/* for encryption/decryption helpers */
#include "rtr/tr_sl1_64.h"

#include "rsslVersion.h"
#if defined( _WIN32)
#include <Windows.h>
#else
#include <dlfcn.h>
#endif

#ifndef _WIN32
#include <pwd.h>
#endif

/* Include other transport type headers */
#include "rtr/rsslChanManagement.h"
#include "rtr/rsslSeqMcastTransportImpl.h"
#include "rtr/rsslSocketTransportImpl.h"
#include "rtr/rsslUniShMemTransportImpl.h"
#include "rtr/rsslLoadInitTransport.h"

/* globals */
static void(*rsslDumpInFunc)(const char *functionName, char *buffer, RsslUInt32 length, RsslSocket socketId) = 0;
static void(*rsslDumpOutFunc)(const char *functionName, char *buffer, RsslUInt32 length, RsslSocket socketId) = 0;

/* The list of debug functions' entries index by the protocol type. */
static RsslDumpFuncs rsslDumpFuncs[MAX_PROTOCOL_TYPES];

/* Initialization the debug dump functions' entries. */
void rsslClearDebugFunctionsEx();

RTR_C_INLINE void rsslDumpInFuncImpl(const char* functionName, char* buffer, RsslUInt32 length, RsslSocket socketId, RsslChannel* pChannel);
RTR_C_INLINE void rsslDumpOutFuncImpl(const char* functionName, char* buffer, RsslUInt32 length, RsslSocket socketId, RsslChannel* pChannel);

/* 33 additional chars to hold time stamps (when needed) */
#define TIME_STAMP_SIZE 33

/* used for all connection types to control locking */
RsslLockingTypes  multiThread = 0;  /* 0 == No Locking; 1 == All locking; 2 == Only global locking */

/*  debug globals - set to 0 is off, set to 1 will print debug msgs */
unsigned char memoryDebug = 0;

/* used to keep track of the allocated channels */
static RsslQueue freeChannelList;
static RsslQueue freeServerList;
static RsslQueue activeServerList;
static RsslQueue activeChannelList;

/* used to help out with initialization and clean up */
static rtr_atomic_val	numInitCalls = 0;
static rtr_atomic_val	initialized = 0;
static rtr_atomic_val	initMutexFucs = 0;

/* The list of transport functions index by the type of transport.
 * RSSL_SOCKET_TRANSPORT 0 - Use base socket calls.
 * RSSL_UNIDIRECTION_SHMEM_TRANSPORT 1 - Use the OpenSSL socket calls.
 */
static RsslTransportChannelFuncs  channelTransFuncs[RSSL_MAX_TRANSPORTS];
static RsslTransportServerFuncs   serverTransFuncs[RSSL_MAX_TRANSPORTS];

/* used by each transport to set its functions into the array */
RsslRet rsslSetTransportChannelFunc( int transportType, RsslTransportChannelFuncs *funcs )
{
	_DEBUG_TRACE_INIT("transportType %d\n", transportType) 
	if (transportType >= RSSL_MAX_TRANSPORTS)
		return(RSSL_RET_FAILURE);

	channelTransFuncs[transportType] = *funcs;
	return(RSSL_RET_SUCCESS);
}

RsslRet rsslSetTransportServerFunc( int transportType, RsslTransportServerFuncs *funcs )
{
	_DEBUG_TRACE_INIT("transportType %d\n", transportType) 
	if (transportType >= RSSL_MAX_TRANSPORTS)
		return(RSSL_RET_FAILURE);

	serverTransFuncs[transportType] = *funcs;
	return(RSSL_RET_SUCCESS);
}

RsslTransportChannelFuncs* rsslGetTransportChannelFunc(int transportType)
{
	return &(channelTransFuncs[transportType]);
}

/* Static Mutex Functions */
static RSSL_STATIC_MUTEX_DECL(s_rsslStaticMutex);

RTR_C_ALWAYS_INLINE void _rsslStaticMutexLock()
{
  RSSL_STATIC_MUTEX_LOCK( s_rsslStaticMutex );
}

RTR_C_ALWAYS_INLINE void _rsslStaticMutexUnlock()
{
  RSSL_STATIC_MUTEX_UNLOCK( s_rsslStaticMutex );
}

RTR_C_ALWAYS_INLINE void _rsslStaticMutexLockDummy()
{
}

RTR_C_ALWAYS_INLINE void _rsslStaticMutexUnlockDummy()
{
}

typedef struct {
	void	(*staticMutexLock)();
	void	(*staticMutexUnlock)();
} rsslMutexFuncs;

rsslMutexFuncs mutexFuncs;


void rsslQueryTransportLibraryVersion(RsslLibraryVersionInfo *pVerInfo)
{
	if (pVerInfo)
	{
		pVerInfo->productDate = rsslDeltaDate;
		pVerInfo->internalVersion = rsslVersion;
		pVerInfo->productVersion = rsslPackage;
		pVerInfo->interfaceVersion = rsslInterfaceVersion;
	}
}


/**************************
 *  UTILITY/HELPER FUNCTIONS
 **************************/

/* does memory allocation and initialization of channel */
RTR_C_ALWAYS_INLINE rsslChannelImpl *_rsslCreateChannel()
{
	rsslBufferImpl *buffer = 0;
	RsslErrorInfo	rsslErrorInfo;
	int i;

	/* Create memory */
	rsslChannelImpl *chnl = (rsslChannelImpl*)_rsslMalloc(sizeof(rsslChannelImpl));

	if (chnl) 
		_rsslCleanChan(chnl);
	else
		return NULL;

	if (multiThread == RSSL_LOCK_GLOBAL_AND_CHANNEL)
	{
	  (void) RSSL_MUTEX_INIT_RTSDK( &chnl->chanMutex );
	}
	(void) RSSL_MUTEX_INIT_RTSDK( &chnl->traceMutex );

	// Allocate hash table for handling fragmentation
	rsslHashTableInit(&(chnl->assemblyBuffers), 65535, UInt32_key_hash, checkFragID, RSSL_TRUE, &rsslErrorInfo);
	
	rsslInitQueue(&(chnl->activeBufferList));
	rsslInitQueue(&(chnl->freeBufferList));

	for(i = 0; i < 10; i++)
	{
		if ((buffer = _rsslCreateBuffer(chnl)) != 0)
		{
			if (memoryDebug)
				printf("adding to freeBufferList\n");
			rsslQueueAddLinkToBack(&(chnl->freeBufferList), &(buffer->link1));
		}
	}

	rsslInitQueueLink(&(chnl->link1));

	return chnl;
}

/* does memory allocation and initialization of server */
RTR_C_ALWAYS_INLINE rsslServerImpl* _rsslCreateServer()
{
	/* create memory */
	rsslServerImpl *srvr = (rsslServerImpl*)_rsslMalloc(sizeof(rsslServerImpl));

	if (srvr) 
		_rsslCleanServer(srvr);
	else
		return NULL;

	if (multiThread == RSSL_LOCK_GLOBAL_AND_CHANNEL)
	{
	  (void) RSSL_MUTEX_INIT_RTSDK( &srvr->srvrMutex );
	}
	
	srvr->hasSharedBufPool = RSSL_FALSE;
	
	rsslInitQueueLink(&(srvr->link1));

	return srvr;
}

/* grabs new channel from freeList */
rsslChannelImpl *_rsslNewChannel()
{
	rsslChannelImpl *chnl=0;
	RsslQueueLink	*pLink = 0;
	
	mutexFuncs.staticMutexLock();

	if ((pLink = rsslQueuePeekFront(&freeChannelList)) == 0)
		chnl = _rsslCreateChannel();
	else
	{
		chnl = RSSL_QUEUE_LINK_TO_OBJECT(rsslChannelImpl, link1, pLink);
		if (rsslQueueLinkInAList(&(chnl->link1)) == RSSL_TRUE)
		{
			rsslQueueRemoveLink(&freeChannelList, &(chnl->link1));
			if (memoryDebug)
				printf("removing from freeChannelList\n");
		}
	}

	mutexFuncs.staticMutexUnlock();

	return chnl;
}

/* grabs a new server from freeList */
rsslServerImpl *_rsslNewServer()
{
	rsslServerImpl *srvr=0;
	RsslQueueLink  *pLink = 0;

	mutexFuncs.staticMutexLock();

	if ((pLink = rsslQueueRemoveFirstLink(&freeServerList)) == 0)
		srvr = _rsslCreateServer();
	else
	{
		srvr = RSSL_QUEUE_LINK_TO_OBJECT(rsslServerImpl, link1, pLink);
		if (rsslQueueLinkInAList(&(srvr->link1)))
		{
			rsslQueueRemoveLink(&freeServerList, &(srvr->link1));
			if (memoryDebug)
				printf("removing from freeServerList \n");
		}
	}

	srvr->serverCountersInfo.countOfFreeServerList = rsslQueueGetElementCount(&freeServerList);
	srvr->serverCountersInfo.countOfActiveServerList = rsslQueueGetElementCount(&activeServerList);

	mutexFuncs.staticMutexUnlock();
	return srvr;
}

/* release the active buffer list - this needs to happen before calling ReleaseChannel */
RTR_C_ALWAYS_INLINE void _rsslReleaseActiveBuffers(rsslChannelImpl *chnl)
{
	rsslBufferImpl *rsslBufImpl=0;
	RsslQueueLink *pLink = 0;
	RsslError error;
	
	while ((pLink = rsslQueueRemoveLastLink(&(chnl->activeBufferList))))
	{
		rsslBufImpl = RSSL_QUEUE_LINK_TO_OBJECT(rsslBufferImpl, link1, pLink);
		rsslReleaseBuffer(&(rsslBufImpl->buffer), &error);
	}

	return;
}

/* releases channel to freeList */
void RTR_FASTCALL _rsslReleaseChannel(rsslChannelImpl *chnl)
{
	rsslAssemblyBuffer *rsslAssemblyBuf=0;
	RsslQueueLink *pLink;

	/* if we own the component version, free it */
	if ((chnl->ownCompVer == RSSL_TRUE) && (chnl->componentVer.componentVersion.length) && (chnl->componentVer.componentVersion.data))
	{
		_rsslFree(chnl->componentVer.componentVersion.data);
		chnl->ownCompVer = RSSL_FALSE;
	}

	if ((chnl->ownConnOptCompVer == RSSL_TRUE) && (chnl->connOptsCompVer.componentVersion.length) && (chnl->connOptsCompVer.componentVersion.data))
	{
		_rsslFree(chnl->connOptsCompVer.componentVersion.data);
		chnl->ownConnOptCompVer = RSSL_FALSE;
	}

	// Free up the memory as the memory is allocated for every channel for reliable multicast based connection.
	if (chnl->Channel.connectionType == RSSL_CONN_TYPE_RELIABLE_MCAST)
	{
		_rsslFree(chnl->transportInfo);
		_rsslFree(chnl->transportClientInfo);
	}

	// Ensure that there is no rsslAssemblyBuffer left in chnl->assemblyBuffers
	if (chnl->assemblyBuffers.elementCount > 0)
	{
		RsslUInt32 index;
		for (index = 0; index < chnl->assemblyBuffers.queueCount; index++)
		{
			RSSL_QUEUE_FOR_EACH_LINK(&chnl->assemblyBuffers.queueList[index], pLink)
			{
				rsslAssemblyBuf = RSSL_QUEUE_LINK_TO_OBJECT(rsslAssemblyBuffer, link1, pLink);

				if (rsslAssemblyBuf->buffer.data)
				{
					_rsslFree(rsslAssemblyBuf->buffer.data);
					rsslAssemblyBuf->buffer.data = 0;
				}

				rsslHashTableRemoveLink(&chnl->assemblyBuffers, &rsslAssemblyBuf->link1);

				_rsslFree(rsslAssemblyBuf);
				rsslAssemblyBuf = 0;
			}
		}
	}

	_rsslCleanChan(chnl);	
	mutexFuncs.staticMutexLock();
	if (rsslQueueLinkInAList(&(chnl->link1)))
	{
		rsslQueueRemoveLink(&activeChannelList, &(chnl->link1));
		if (memoryDebug)
			printf("removing from activeChannelList\n");
	}

	rsslQueueAddLinkToBack(&freeChannelList, &(chnl->link1));
	if (memoryDebug)
		printf("adding to freeChannelList\n");
	mutexFuncs.staticMutexUnlock();
}

/* releases server to freeList */
void RTR_FASTCALL _rsslReleaseServer(rsslServerImpl *srvr)
{
	rsslServerCountersInfo* countersInfo = &srvr->serverCountersInfo;

	/* server always owns component version string when present */
	if ((srvr->componentVer.componentVersion.length) && (srvr->componentVer.componentVersion.data))
	{
		_rsslFree(srvr->componentVer.componentVersion.data);
	}

	if ((srvr->connOptsCompVer.componentVersion.length) && (srvr->connOptsCompVer.componentVersion.data))
	{
		_rsslFree(srvr->connOptsCompVer.componentVersion.data);
	}

	_rsslCleanServer(srvr);

	mutexFuncs.staticMutexLock();
	if (rsslQueueLinkInAList(&(srvr->link1)))
	{
		rsslQueueRemoveLink(&activeServerList, &(srvr->link1));
		if (memoryDebug)
			printf("removing from activeServerList\n");
	}

	rsslQueueAddLinkToBack(&freeServerList, &(srvr->link1));
	if (memoryDebug)
		printf("adding to freeServerList\n");
	
	{
		countersInfo->countOfFreeServerList = rsslQueueGetElementCount(&freeServerList);
		countersInfo->countOfActiveServerList = rsslQueueGetElementCount(&activeServerList);
	}
	mutexFuncs.staticMutexUnlock();

	ipcGetOfServerSocketChannelCounters(countersInfo);
}

/* cleans up the channel list */
void _rsslCleanUp()
{
	rsslChannelImpl *chnl=0;
	rsslServerImpl *srvr=0;
	rsslBufferImpl *buffer=0;
	RsslQueueLink  *pLink = 0;

	/* if we go through the actives list and call release on them, then they
	   will be put on the free lists and all their internal memory will be cleaned up */

	while ((pLink = rsslQueueRemoveLastLink(&activeChannelList)))
	{
		chnl = RSSL_QUEUE_LINK_TO_OBJECT(rsslChannelImpl, link1, pLink);
		_rsslReleaseChannel(chnl);
	}

	while ((pLink = rsslQueueRemoveLastLink(&activeServerList)))
	{
		srvr = RSSL_QUEUE_LINK_TO_OBJECT(rsslServerImpl, link1, pLink);
		_rsslReleaseServer(srvr);
	}

	mutexFuncs.staticMutexLock();
	while ((pLink = rsslQueueRemoveLastLink(&freeChannelList)))
	{
		chnl = RSSL_QUEUE_LINK_TO_OBJECT(rsslChannelImpl, link1, pLink);
		if (memoryDebug)
			printf("cleaning up free hash table\n");

		rsslHashTableCleanup(&(chnl->assemblyBuffers));

		if (memoryDebug)
			printf("cleaning up free channel list\n");
	
		/* Channel has a free buffer list - clean this up */
		while ((pLink = rsslQueueRemoveLastLink(&(chnl->freeBufferList))))
		{
			buffer = RSSL_QUEUE_LINK_TO_OBJECT(rsslBufferImpl, link1, pLink);
			if (memoryDebug)
				printf("cleaning up freeBufferList\n");
	
			/* since its on the free list it can be freed */
			_rsslFree(buffer);
			buffer = 0;
		}

		/* destroy the mutex */
		if (multiThread == RSSL_LOCK_GLOBAL_AND_CHANNEL)
		  (void) RSSL_MUTEX_DESTROY(&chnl->chanMutex);

		(void) RSSL_MUTEX_DESTROY(&chnl->traceMutex);
		/* since its on the free list, its list of buffers should be empty */
		/* free it */
		 _rsslFree(chnl); 
		 chnl=0; 
	}

	while ((pLink = rsslQueueRemoveLastLink(&freeServerList)))
	{
		srvr = RSSL_QUEUE_LINK_TO_OBJECT(rsslServerImpl, link1, pLink);
		if (memoryDebug)
			printf("cleaning up freeServerList \n");

		/* destroy the mutex */
		if (multiThread == RSSL_LOCK_GLOBAL_AND_CHANNEL)
		  (void) RSSL_MUTEX_DESTROY(&srvr->srvrMutex);

		if (srvr->hasSharedBufPool)
		{
		  (void) RSSL_MUTEX_DESTROY(&srvr->sharedBufPoolMutex);
			srvr->hasSharedBufPool = RSSL_FALSE;
		}

		/* since its on the free list its list of channels should be empty */
		/* free it */
		_rsslFree(srvr); 
		srvr=0; 
	}

	mutexFuncs.staticMutexUnlock();
}

/* Write an XML comment to stdout and/or to a file, depending on the trace options set for the channel.
 *   timestamp - Adds a timestamp comment.
 *   fileFlush - Flushes to the file (used if this is the last part to be written for an event).
 */
RTR_C_ALWAYS_INLINE void _rsslXMLDumpComment(rsslChannelImpl *rsslChnlImpl, const char* comment, RsslBool timestamp, RsslBool fileFlush)
{
	if(rsslChnlImpl->traceOptionsInfo.traceMsgFilePtr != NULL)
	{
		xmlDumpComment(rsslChnlImpl->traceOptionsInfo.traceMsgFilePtr, comment);
		if (timestamp)
			xmlDumpTimestamp(rsslChnlImpl->traceOptionsInfo.traceMsgFilePtr);
		if (fileFlush)
			fflush(rsslChnlImpl->traceOptionsInfo.traceMsgFilePtr);
	}
	if(rsslChnlImpl->traceOptionsInfo.traceOptions.traceFlags & RSSL_TRACE_TO_STDOUT)
	{
		xmlDumpComment(stdout, comment);
		if (timestamp)
			xmlDumpTimestamp(stdout);
	}
}

/* this routine is called to trace both reads and writes
 * the return code from the rsslRead/rsslWrite is passed in
 * the isRead flag signifies if we are tracing a read or write
 * we need a start and end trace function because the RSSL buffer is freed after the rsslWrite succeeds
 */
typedef enum {
	traceRead = 1,
	traceWrite = 2,
	tracePack = 3,
	traceDump = 4
} traceOperation;

void _rsslTraceStartMsg(rsslChannelImpl *rsslChnlImpl, RsslUInt32 protocolType, RsslBuffer *buffer, RsslRet *retTrace, traceOperation op, RsslError *error)
{
	RsslDecodeIterator dIter;
	RsslMsg msg = RSSL_INIT_MSG;
	RsslRet ret = RSSL_RET_SUCCESS;
	char message[128];
	RsslInt64 filePos = 0;
	rsslBufferImpl *pRsslBufferImpl = (rsslBufferImpl *)buffer;

	if (buffer == NULL)
		return;

	if (*retTrace == RSSL_RET_FAILURE)
		return;
	
	(void) RSSL_MUTEX_LOCK(&rsslChnlImpl->traceMutex);
	if(rsslChnlImpl->traceOptionsInfo.traceMsgFilePtr != NULL)
	{
		filePos = ftell(rsslChnlImpl->traceOptionsInfo.traceMsgFilePtr);
		if((filePos >= rsslChnlImpl->traceOptionsInfo.traceOptions.traceMsgMaxFileSize))
		{
			unsigned long long hour = 0 , min = 0, sec = 0, msec = 0;
			char timeVal[TIME_STAMP_SIZE];
			int numChars = 0;

			/* Close this file */
			fclose(rsslChnlImpl->traceOptionsInfo.traceMsgFilePtr);
			rsslChnlImpl->traceOptionsInfo.traceMsgFilePtr = NULL;

			/* and open a new one */
			if (rsslChnlImpl->traceOptionsInfo.traceOptions.traceFlags & RSSL_TRACE_TO_MULTIPLE_FILES)
			{
				/* The new file name will be the original file name with secs, msecs, and ".xml" extension appeneded to the end*/
				xmlGetTimeFromEpoch(&hour, &min, &sec, &msec);

				numChars = snprintf(timeVal, TIME_STAMP_SIZE, "%03llu.xml", msec);

				memcpy(rsslChnlImpl->traceOptionsInfo.newTraceMsgFileName, rsslChnlImpl->traceOptionsInfo.traceOptions.traceMsgFileName, rsslChnlImpl->traceOptionsInfo.traceMsgOrigFileNameSize);
				memcpy(rsslChnlImpl->traceOptionsInfo.newTraceMsgFileName + rsslChnlImpl->traceOptionsInfo.traceMsgOrigFileNameSize, timeVal, numChars);
				rsslChnlImpl->traceOptionsInfo.newTraceMsgFileName[rsslChnlImpl->traceOptionsInfo.traceMsgOrigFileNameSize + numChars] = '\0';

				rsslChnlImpl->traceOptionsInfo.traceMsgFilePtr = fopen(rsslChnlImpl->traceOptionsInfo.newTraceMsgFileName, "a+");

				if (rsslChnlImpl->traceOptionsInfo.traceMsgFilePtr == NULL)
				{
					snprintf(error->text, MAX_RSSL_ERROR_TEXT, "<%s:%d> rsslTraceStartMsg() Error: Unable to open file. fopen() failed\n", __FILE__, __LINE__);
				}
			}
		}
	}
	
	/* check if we got an FD change */
	if (*retTrace == RSSL_RET_READ_FD_CHANGE)
	{
	    snprintf(message, sizeof(message), "Incoming FD Change (Channel IPC descriptor = "SOCKET_PRINT_TYPE, rsslChnlImpl->Channel.socketId);
		_rsslXMLDumpComment(rsslChnlImpl, message, RSSL_TRUE, RSSL_TRUE);
	}
	/* we actually read or wrote something */
	else if (*retTrace >= RSSL_RET_SUCCESS || *retTrace == RSSL_RET_WRITE_CALL_AGAIN)
	{
		if (op == traceRead)
			snprintf(message, sizeof(message), "Incoming Message (Channel IPC descriptor = "SOCKET_PRINT_TYPE")", rsslChnlImpl->Channel.socketId);
		else if (op == traceWrite)
			snprintf(message, sizeof(message), "Outgoing Message (Channel IPC descriptor = "SOCKET_PRINT_TYPE")", rsslChnlImpl->Channel.socketId);
		else if (op == tracePack)
		  snprintf(message, sizeof(message), "Pack Message (Channel IPC descriptor = "SOCKET_PRINT_TYPE")", rsslChnlImpl->Channel.socketId);
		else if (op == traceDump)
			snprintf(message, sizeof(message), "Dump Message (Channel IPC descriptor = "SOCKET_PRINT_TYPE")", rsslChnlImpl->Channel.socketId);
		
		_rsslXMLDumpComment(rsslChnlImpl, message, RSSL_TRUE, RSSL_TRUE);

		/* XML dump RWF messages */
		if (protocolType == RSSL_RWF_PROTOCOL_TYPE)
		{
			if(rsslChnlImpl->traceOptionsInfo.traceMsgFilePtr != NULL)
			{
				rsslClearDecodeIterator(&dIter);
				rsslSetDecodeIteratorRWFVersion(&dIter, rsslChnlImpl->Channel.majorVersion, rsslChnlImpl->Channel.minorVersion);
				rsslSetDecodeIteratorBuffer(&dIter, buffer);

				if ((ret = rsslDecodeMsg(&dIter, &msg)) == RSSL_RET_SUCCESS)
					decodeMsgToXML(rsslChnlImpl->traceOptionsInfo.traceMsgFilePtr, &msg, NULL, &dIter);
			}
			if(rsslChnlImpl->traceOptionsInfo.traceOptions.traceFlags & RSSL_TRACE_TO_STDOUT)
			{
				rsslClearDecodeIterator(&dIter);
				rsslSetDecodeIteratorRWFVersion(&dIter, rsslChnlImpl->Channel.majorVersion, rsslChnlImpl->Channel.minorVersion);
				rsslSetDecodeIteratorBuffer(&dIter, buffer);

				if ((ret = rsslDecodeMsg(&dIter, &msg)) == RSSL_RET_SUCCESS)
					decodeMsgToXML(stdout, &msg, NULL, &dIter);
			}
		}
		else if (protocolType == RSSL_JSON_PROTOCOL_TYPE)
		{
			RsslBuffer *pDumpJSON = buffer;
			RsslBuffer tempBuffer = RSSL_INIT_BUFFER;
			rtr_msgb_t	*ripcBuffer = (rtr_msgb_t*)(pRsslBufferImpl->bufferInfo);

			if (buffer->length == 0 && pRsslBufferImpl->packingOffset > 1) // Indicates packed buffer
			{
				if (ripcBuffer)
				{
					tempBuffer.data = ripcBuffer->buffer + 1;
					tempBuffer.length = pRsslBufferImpl->packingOffset - 2;
					pDumpJSON = &tempBuffer;
				}
			}

			if (rsslChnlImpl->traceOptionsInfo.traceMsgFilePtr != NULL)
			{
				fprintf(rsslChnlImpl->traceOptionsInfo.traceMsgFilePtr, "<!-- JSON protocol\n");
				dumpJSON(rsslChnlImpl->traceOptionsInfo.traceMsgFilePtr, pDumpJSON);
				fprintf(rsslChnlImpl->traceOptionsInfo.traceMsgFilePtr, "\n-->");
				fputc('\n', rsslChnlImpl->traceOptionsInfo.traceMsgFilePtr);
			}
			if (rsslChnlImpl->traceOptionsInfo.traceOptions.traceFlags & RSSL_TRACE_TO_STDOUT)
			{
				dumpJSON(stdout, pDumpJSON);
				fputc('\n', stdout);
			}
		}

		/* Do a hex dump of the message if we cant decode it, if it's not RWF, or if they asked for it */
		if (ret != RSSL_RET_SUCCESS									|| 
			(rsslChnlImpl->traceOptionsInfo.traceOptions.traceFlags & RSSL_TRACE_HEX)	||
			(protocolType != RSSL_RWF_PROTOCOL_TYPE && protocolType != RSSL_JSON_PROTOCOL_TYPE) )
		{
			if(rsslChnlImpl->traceOptionsInfo.traceMsgFilePtr != NULL)
			{
				xmlDumpHexBuffer(rsslChnlImpl->traceOptionsInfo.traceMsgFilePtr, buffer);
				fputc('\n', rsslChnlImpl->traceOptionsInfo.traceMsgFilePtr);
			}
			if(rsslChnlImpl->traceOptionsInfo.traceOptions.traceFlags & RSSL_TRACE_TO_STDOUT)
			{
				xmlDumpHexBuffer(stdout, buffer);
				fputc('\n', stdout);
			}
		}
	}
	(void) RSSL_MUTEX_UNLOCK(&rsslChnlImpl->traceMutex);
}

void _rsslTraceEndMsg(rsslChannelImpl *rsslChnlImpl, RsslRet *retTrace, RsslBool isRead)
{
	char message[128];

	/* we only need to log these on rsslWrite calls because the message is dumped before the call to rsslWrite */
	(void) RSSL_MUTEX_LOCK(&rsslChnlImpl->traceMutex);
	if (!isRead)
	{
		switch(*retTrace)
		{
			case RSSL_RET_FAILURE:
				snprintf(message, sizeof(message), "rsslWrite Failed (Channel IPC descriptor = "SOCKET_PRINT_TYPE")", rsslChnlImpl->Channel.socketId);
				_rsslXMLDumpComment(rsslChnlImpl, message, RSSL_FALSE, RSSL_FALSE);
				break;

			case RSSL_RET_WRITE_CALL_AGAIN:
				snprintf(message, sizeof(message), "rsslWrite returned RSSL_RET_WRITE_CALL_AGAIN (Channel IPC descriptor = "SOCKET_PRINT_TYPE")", rsslChnlImpl->Channel.socketId);
				_rsslXMLDumpComment(rsslChnlImpl, message, RSSL_FALSE, RSSL_FALSE);
				break;

			case RSSL_RET_INIT_NOT_INITIALIZED:
				snprintf(message, sizeof(message), "rsslWrite returned RSSL_RET_INIT_NOT_INITIALIZED (Channel IPC descriptor = "SOCKET_PRINT_TYPE")", rsslChnlImpl->Channel.socketId);
				_rsslXMLDumpComment(rsslChnlImpl, message, RSSL_FALSE, RSSL_FALSE);
				break;

			default:
				break;
		}
		snprintf(message, sizeof(message), "End Message (Channel IPC descriptor = "SOCKET_PRINT_TYPE")", rsslChnlImpl->Channel.socketId);
		_rsslXMLDumpComment(rsslChnlImpl, message, RSSL_FALSE, RSSL_TRUE);
	}
	else
	{
		if ((*retTrace >= RSSL_RET_SUCCESS) || (*retTrace == RSSL_RET_READ_PING))
		{
			snprintf(message, sizeof(message), "End Message (Channel IPC descriptor = "SOCKET_PRINT_TYPE")", rsslChnlImpl->Channel.socketId);
			_rsslXMLDumpComment(rsslChnlImpl, message, RSSL_FALSE, RSSL_TRUE);
		}
	}
	(void) RSSL_MUTEX_UNLOCK(&rsslChnlImpl->traceMutex);
}

void _rsslTraceClosed(rsslChannelImpl *rsslChnlImpl, RsslRet *retTrace)
{
	if (*retTrace == RSSL_RET_FAILURE && rsslChnlImpl->Channel.state == RSSL_CH_STATE_CLOSED)
	{
		char message[128];

		snprintf(message, sizeof(message), "Channel Closed (Channel IPC descriptor = "SOCKET_PRINT_TYPE")", rsslChnlImpl->Channel.socketId);
		(void) RSSL_MUTEX_LOCK(&rsslChnlImpl->traceMutex);
		_rsslXMLDumpComment(rsslChnlImpl, message, RSSL_TRUE, RSSL_TRUE);
		(void) RSSL_MUTEX_UNLOCK(&rsslChnlImpl->traceMutex);
	}
}
	

/***********************
 * PUBLIC FUNCTIONS
 **********************/

/* Initializes rssl session */
RsslRet rsslInitialize(RsslLockingTypes rsslLocking, RsslError *error)
{
	RsslInitializeExOpts initOpts = RSSL_INIT_INITIALIZE_EX_OPTS;
	
	initOpts.rsslLocking = rsslLocking;
	
	return rsslInitializeEx(&initOpts, error);
}

/* Initializes rssl session */
RsslRet rsslInitializeEx(RsslInitializeExOpts *rsslInitOpts, RsslError *error)
{
	int retVal = RSSL_RET_FAILURE;
	rsslChannelImpl *chnl=0;
	rsslServerImpl  *srvr=0;
	int i = 0;
	
	if (!initialized)
	{
		multiThread = rsslInitOpts->rsslLocking;
	}

	if (initialized)
	{
		if (multiThread != rsslInitOpts->rsslLocking)
		{
			_rsslSetError(error, NULL, RSSL_RET_FAILURE, 0);
			snprintf(error->text, MAX_RSSL_ERROR_TEXT, "<%s:%d> rsslInitialize() Error 0004 Cannot change mutex locking type from %d to %d\n", __FILE__, __LINE__, multiThread, rsslInitOpts->rsslLocking);
			return RSSL_RET_FAILURE;
		}
	}
	if (rsslInitOpts->initConfig && !rsslInitOpts->initConfigSize)
	{
		_rsslSetError(error, NULL, RSSL_RET_FAILURE, 0);
		snprintf(error->text, MAX_RSSL_ERROR_TEXT, "<%s:%d> rsslInitialize() Error 0004 Cannot initallize *initConfig without setting initConfigSize to sizeof\n", __FILE__, __LINE__);
		return RSSL_RET_FAILURE;
	}

	if(!initMutexFucs)
	{
		RTR_ATOMIC_SET(initMutexFucs,1);

		/* initialize mutex functions */
		if (multiThread) /* use actual mutex functions */
		{
			mutexFuncs.staticMutexLock = _rsslStaticMutexLock;
			mutexFuncs.staticMutexUnlock = _rsslStaticMutexUnlock;
		}
		else /* use dummy mutex functions */
		{
			mutexFuncs.staticMutexLock = _rsslStaticMutexLockDummy;
			mutexFuncs.staticMutexUnlock = _rsslStaticMutexUnlockDummy;
		}
	}

	mutexFuncs.staticMutexLock();

	if (!initialized)
	{
		/* Initialize All transports here */

		/* initialize cpuid library */
#ifndef NO_ETA_CPU_BIND
		retVal = rsslBindThreadInitialize(error);

		if (retVal < RSSL_RET_SUCCESS)
		{
			mutexFuncs.staticMutexUnlock();
			return retVal;
		}
#endif

		/* initialize debug dump functions */
		rsslClearDebugFunctionsEx();

		/* initialize socket transport */
		retVal = rsslSocketInitialize(rsslInitOpts, error);

		if (retVal < RSSL_RET_SUCCESS)
		{
			mutexFuncs.staticMutexUnlock();	
			return retVal;
		}

		/* initialize shmem transport */
		retVal = rsslUniShMemInitialize(rsslInitOpts->rsslLocking, error);

		if (retVal < RSSL_RET_SUCCESS)
		{
			mutexFuncs.staticMutexUnlock();	
			return retVal;
		}

		/* initialize SeqMcast transport */
		retVal = rsslSeqMcastInitialize(rsslInitOpts->rsslLocking, error);

		if (retVal < RSSL_RET_SUCCESS)
		{
			mutexFuncs.staticMutexUnlock();	
			return retVal;
		}

		/* Done initializing all transports */

		/* Initialize lists */
		rsslInitQueue(&freeChannelList);
		rsslInitQueue(&freeServerList);
		rsslInitQueue(&activeServerList);
		rsslInitQueue(&activeChannelList);
	
		/* preallocate memory for channels - this should
		speed up performance when requesting a channel */
		for(i = 0; i < 10; i++)
		{
			if ((chnl = _rsslCreateChannel()) != 0)
			{
				if (memoryDebug)	
					printf("adding to freeChannelList\n");
				rsslQueueAddLinkToBack(&freeChannelList, &(chnl->link1));
			}
			if ((srvr = _rsslCreateServer()) != 0)
			{
				if (memoryDebug)	
					printf("adding to freeServerList \n");
				rsslQueueAddLinkToBack(&freeServerList, &(srvr->link1));
			}
		}

		RTR_ATOMIC_SET(initialized,1);
	}

	RTR_ATOMIC_INCREMENT(numInitCalls);

	mutexFuncs.staticMutexUnlock();

	return RSSL_RET_SUCCESS;
}

/* Sets debug dump functions */
/* need to turn these on with Ioctl in order to use them */
RsslRet rsslSetDebugFunctions(
	void(*dumpIpcIn)(const char *functionName, char *buffer, RsslUInt32 length, RsslUInt64 opaque),
	void(*dumpIpcOut)(const char *functionName, char *buffer, RsslUInt32 length, RsslUInt64 opaque),
	void(*dumpRsslIn)(const char *functionName, char *buffer, RsslUInt32 length, RsslSocket socketId),
	void(*dumpRsslOut)(const char *functionName, char *buffer, RsslUInt32 length, RsslSocket socketId),
	RsslError *error)
{
	RsslRet retVal = RSSL_RET_SUCCESS;
	
	mutexFuncs.staticMutexLock();

	if ((dumpRsslIn && rsslDumpInFunc) || (dumpRsslOut && rsslDumpOutFunc))
	{
		/* set error message */
		_rsslSetError(error, NULL, RSSL_RET_FAILURE, 0);
		snprintf(error->text, MAX_RSSL_ERROR_TEXT, "<%s:%d> rsslSetDebugFunctions() Cannot change Rssl dump functions.\n", __FILE__, __LINE__);

		retVal = RSSL_RET_FAILURE;
	}
	else
	{
		rsslDumpInFunc = dumpRsslIn;
		rsslDumpOutFunc = dumpRsslOut;
		if(rsslSetSocketDebugFunctions(dumpIpcIn, dumpIpcOut, dumpRsslIn, dumpRsslOut, error) < RSSL_RET_SUCCESS)
		{
			retVal = RSSL_RET_FAILURE;
		}

		if (rsslSetWebSocketDebugFunctions(dumpIpcIn, dumpIpcOut, dumpRsslIn, dumpRsslOut, error) < RSSL_RET_SUCCESS)
		{
			retVal = RSSL_RET_FAILURE;
		}
			
		if(rsslSetUniShMemDebugFunctions(dumpRsslIn, dumpRsslOut, error) < RSSL_RET_SUCCESS)
		{
			_rsslSetError(error, NULL, RSSL_RET_FAILURE, 0);
			snprintf(error->text, MAX_RSSL_ERROR_TEXT, "<%s:%d> rsslSetUniShMemDebugFunctions() Cannot set Rssl shared memory dump functions.\n", __FILE__, __LINE__);
			retVal = RSSL_RET_FAILURE;
		}

		if (rsslSetSeqMcastDebugFunctions(dumpRsslIn, dumpRsslOut, error) < RSSL_RET_SUCCESS)
		{
			_rsslSetError(error, NULL, RSSL_RET_FAILURE, 0);
			snprintf(error->text, MAX_RSSL_ERROR_TEXT, "<%s:%d> rsslSetSeqMcastDebugFunctions() Cannot set Rssl sequence multicast dump functions.\n", __FILE__, __LINE__);
			retVal = RSSL_RET_FAILURE;
		}
	}
	
	mutexFuncs.staticMutexUnlock();

	return retVal;
}

/* Initialization debug dump functions' entries */
void rsslClearDebugFunctionsEx()
{
	memset(rsslDumpFuncs, 0, (sizeof(RsslDumpFuncs) * MAX_PROTOCOL_TYPES));
	
	rsslClearSoketDebugFunctionsEx();
	rsslClearUniShMemDebugFunctionsEx();
	rsslClearSeqMcastDebugFunctionsEx();
}

/** Sets debug dump functions for the protocol type
* need to turn these on with Ioctl in order to use them
*/
RSSL_API RsslRet rsslSetDebugFunctionsEx(RsslDebugFunctionsExOpts* pOpts, RsslError* error)
{
	RsslRet retVal = RSSL_RET_SUCCESS;

	mutexFuncs.staticMutexLock();

	if ((pOpts->dumpRsslIn && rsslDumpFuncs[pOpts->protocolType].rsslDumpInFunc)
		|| (pOpts->dumpRsslOut && rsslDumpFuncs[pOpts->protocolType].rsslDumpOutFunc))
	{
		/* set error message */
		_rsslSetError(error, NULL, RSSL_RET_FAILURE, 0);
		snprintf(error->text, MAX_RSSL_ERROR_TEXT, "<%s:%d> rsslSetDebugFunctionsEx() Cannot change Rssl dump functions.\n", __FILE__, __LINE__);

		retVal = RSSL_RET_FAILURE;
	}
	else
	{
		rsslDumpFuncs[pOpts->protocolType].rsslDumpInFunc = pOpts->dumpRsslIn;
		rsslDumpFuncs[pOpts->protocolType].rsslDumpOutFunc = pOpts->dumpRsslOut;

		if (rsslSetSocketDebugFunctionsEx(pOpts, error) < RSSL_RET_SUCCESS)
		{
			retVal = RSSL_RET_FAILURE;
		}

		if (rsslSetWebSocketDebugFunctionsEx(pOpts, error) < RSSL_RET_SUCCESS)
		{
			retVal = RSSL_RET_FAILURE;
		}

		if (rsslSetUniShMemDebugFunctionsEx(pOpts, error) < RSSL_RET_SUCCESS)
		{
			retVal = RSSL_RET_FAILURE;
		}

		if (rsslSetSeqMcastDebugFunctionsEx(pOpts, error) < RSSL_RET_SUCCESS)
		{
			retVal = RSSL_RET_FAILURE;
		}
	}

	mutexFuncs.staticMutexUnlock();

	return retVal;
}

void rsslDumpInFuncImpl(const char* functionName, char* buffer, RsslUInt32 length, RsslSocket socketId, RsslChannel* pChannel)
{
	if (rsslDumpInFunc)
	{
		(*(rsslDumpInFunc))(functionName, buffer, length, socketId);
	}
	if (rsslDumpFuncs[pChannel->protocolType].rsslDumpInFunc)
	{
		(*(rsslDumpFuncs[pChannel->protocolType].rsslDumpInFunc))(functionName, buffer, length, pChannel);
	}
}

void rsslDumpOutFuncImpl(const char* functionName, char* buffer, RsslUInt32 length, RsslSocket socketId, RsslChannel* pChannel)
{
	if (rsslDumpOutFunc)
	{
		(*(rsslDumpOutFunc))(functionName, buffer, length, socketId);
	}
	if (rsslDumpFuncs[pChannel->protocolType].rsslDumpOutFunc)
	{
		(*(rsslDumpFuncs[pChannel->protocolType].rsslDumpOutFunc))(functionName, buffer, length, pChannel);
	}
}


/* returned ipAddr in host byte order */
RsslRet rsslHostByName(RsslBuffer *hostName, RsslUInt32 *ipAddr)
{
	RsslRet retVal;
	RsslUInt32 tempUInt;

	retVal = rsslGetHostByName(hostName->data, ipAddr);

	if (retVal != -1)
	{
		/* change ipAddr to host byte order since rsslGetHostByName() gets ipAddr in network byte order */
		rwfGet32(tempUInt, ipAddr);
		*ipAddr = tempUInt;
	}

	return retVal;
}

RsslRet rsslGetUserName(RsslBuffer *userName)
{
	char *pTempUserName, *pBuf;
#if defined(_WIN32)
	char tempUserName[1024];
	int tempUserNameSize = sizeof(tempUserName);
#endif
	unsigned int i, userNameLength = 0;
#if !defined(_WIN32)
	struct  passwd *passwd;
	char    pwd_buffer[1024];
	struct  passwd pwd;
#endif

#if defined(_WIN32)
	if (!GetUserName(tempUserName, &tempUserNameSize))
		return RSSL_RET_FAILURE;

	pTempUserName = tempUserName;
#else
	if(getpwuid_r(getuid(), &pwd, pwd_buffer, sizeof(pwd_buffer), &passwd) != 0)
		return RSSL_RET_FAILURE;
	pTempUserName = passwd->pw_name;
	if (pTempUserName == NULL)
		return RSSL_RET_FAILURE;
#endif

	pBuf = userName->data;
	*pBuf = '\0';

	for (i = 0; i < userName->length - 1; i++)
	{
		if ((*pTempUserName == '\0') || isspace(*pTempUserName))
		{
			userNameLength = i;
			break;
		}
		*pBuf++ = *pTempUserName++;
	}

	/* null-terminate the user name */
	*pBuf = '\0';

	userName->length = userNameLength;

	return RSSL_RET_SUCCESS;
}

/* Binds rssl Server to port and interface */ 
RsslServer* rsslBind(RsslBindOptions *opts, RsslError *error)
{
	rsslServerImpl 	*rsslSrvrImpl=0;
	int				retVal = RSSL_RET_FAILURE;
	
	if (!initialized)
	{
		_rsslSetError(error, NULL, RSSL_RET_INIT_NOT_INITIALIZED, 0);
		snprintf(error->text, MAX_RSSL_ERROR_TEXT, "<%s:%d> rsslBind() Error: 0001 RSSL not initialized.\n", __FILE__, __LINE__);
		return NULL;
	}

	if (RSSL_NULL_PTR(opts, "rsslBind", "opts", error))
		return NULL;

	if (RSSL_NULL_PTR(opts->serviceName, "rsslBind", "opts->serviceName", error))
		return NULL;

	/* create rssl server */
	if ((rsslSrvrImpl = _rsslNewServer()) == 0)
	{
		/* error */
		_rsslSetError(error, NULL, RSSL_RET_FAILURE, 0);
		snprintf(error->text, MAX_RSSL_ERROR_TEXT, "<%s:%d> rsslBind() Error: 0005 Could not allocate memory for new server\n", __FILE__, __LINE__);

		return NULL;
	}

	/* we set both channel and server pointers here - 
	   server uses its function pointers.  Channel pointers are used
	   to set on accepted channels */
	switch (opts->connectionType)
	{
		case RSSL_CONN_TYPE_UNIDIR_SHMEM:
		{
			rsslSrvrImpl->serverFuncs = &(serverTransFuncs[RSSL_UNIDIRECTION_SHMEM_TRANSPORT]);
			rsslSrvrImpl->channelFuncs = &(channelTransFuncs[RSSL_UNIDIRECTION_SHMEM_TRANSPORT]);
		}
		break;
		case RSSL_CONN_TYPE_RELIABLE_MCAST:
		{
			_rsslSetError(error, NULL, RSSL_RET_FAILURE, 0);
			snprintf(error->text, MAX_RSSL_ERROR_TEXT, "<%s:%d> rsslBind() Error: 0006 Reliable Multicast connection type (%d) is currently not supported for a server\n", __FILE__, __LINE__, opts->connectionType);
			_rsslReleaseServer(rsslSrvrImpl);
			return NULL;
		}
		break;
		case RSSL_CONN_TYPE_SEQ_MCAST:
		{
			_rsslSetError(error, NULL, RSSL_RET_FAILURE, 0);
			snprintf(error->text, MAX_RSSL_ERROR_TEXT, "<%s:%d> rsslBind() Error: 0006 Sequenced Multicast connection type (%d) is currently not supported for a server\n", __FILE__, __LINE__, opts->connectionType);
			_rsslReleaseServer(rsslSrvrImpl);
			return NULL;
		}
		break;
		default:
		{
			/* SOCKET, HTTP, and ENCRYPTED use the same transport type and can allow for either connection type */
			rsslSrvrImpl->serverFuncs = &(serverTransFuncs[RSSL_SOCKET_TRANSPORT]);
			rsslSrvrImpl->channelFuncs = &(channelTransFuncs[RSSL_SOCKET_TRANSPORT]);
		}
	}

	/* If this is an encrypted connection type, load OpenSSL.  This needs to be globally locked to ensure that multiple threads cannot attempt to load at the same time. */
	if (opts->connectionType == RSSL_CONN_TYPE_ENCRYPTED)
	{
		mutexFuncs.staticMutexLock();
		retVal = ipcLoadOpenSSL(error);
		mutexFuncs.staticMutexUnlock();

		if (retVal != RSSL_RET_SUCCESS)
		{
			_rsslReleaseServer(rsslSrvrImpl);
			return NULL;
		}
	}
	
	retVal = (*(rsslSrvrImpl->serverFuncs->serverBind))(rsslSrvrImpl, opts, error);

	if (retVal < RSSL_RET_SUCCESS)
	{
		_rsslReleaseServer(rsslSrvrImpl);
		return NULL;
	}
	
	/* put rsslSrvr on activeServerList */
	mutexFuncs.staticMutexLock();
	rsslInitQueueLink(&(rsslSrvrImpl->link1));
	rsslQueueAddLinkToBack(&activeServerList,  &(rsslSrvrImpl->link1));
	if (memoryDebug)
		printf("adding to activeServerList\n");

	rsslSrvrImpl->serverCountersInfo.countOfFreeServerList = rsslQueueGetElementCount(&freeServerList);
	rsslSrvrImpl->serverCountersInfo.countOfActiveServerList = rsslQueueGetElementCount(&activeServerList);

	mutexFuncs.staticMutexUnlock();

	ipcGetOfServerSocketChannelCounters(&(rsslSrvrImpl->serverCountersInfo));

	return (&(rsslSrvrImpl->Server));
}

/* Accepts a new client connection */
RsslChannel* rsslAccept(RsslServer *srvr, RsslAcceptOptions *opts, RsslError *error)
{
	rsslChannelImpl	*rsslChnlImpl=0;
	rsslServerImpl	*rsslSrvrImpl=0;	

	if (!initialized)
	{
		_rsslSetError(error, (RsslChannel*)srvr, RSSL_RET_INIT_NOT_INITIALIZED, 0);
		snprintf(error->text, MAX_RSSL_ERROR_TEXT, "<%s:%d> rsslAccept() Error: 0001 RSSL not initialized.\n", __FILE__, __LINE__);		
		return NULL;
	}
	
	if (RSSL_NULL_PTR(srvr, "rsslAccept", "srvr", error))
		return NULL;

	if (RSSL_NULL_PTR(opts, "rsslAccept", "opts", error))
		return NULL;

	/* Map the RsslServer to the ripcServer */
	rsslSrvrImpl = (rsslServerImpl*)srvr;
	
	rsslChnlImpl = (*(rsslSrvrImpl->serverFuncs->serverAccept))(rsslSrvrImpl, opts, error);

	if (!rsslChnlImpl)
	{
		return NULL;
	}

	mutexFuncs.staticMutexLock();
	rsslInitQueueLink(&(rsslChnlImpl->link1));
	rsslQueueAddLinkToBack(&activeChannelList, &(rsslChnlImpl->link1));
	if (memoryDebug)
		printf("adding chnl "SOCKET_PRINT_TYPE" to activeChannelList\n", rsslChnlImpl->Channel.socketId);
	mutexFuncs.staticMutexUnlock();

	/* if server has component info set, bridge it through to channel, but indicate channel does not own it */
	if ((rsslSrvrImpl->componentVer.componentVersion.length) && (rsslSrvrImpl->componentVer.componentVersion.data))
	{
		rsslChnlImpl->componentVer.componentVersion.length = rsslSrvrImpl->componentVer.componentVersion.length;
		rsslChnlImpl->componentVer.componentVersion.data = rsslSrvrImpl->componentVer.componentVersion.data;
		rsslChnlImpl->ownCompVer = RSSL_FALSE;
	}

	/* if server has component info set, bridge it through to channel, but indicate channel does not own it */
	if ((rsslSrvrImpl->connOptsCompVer.componentVersion.length) && (rsslSrvrImpl->connOptsCompVer.componentVersion.data))
	{
		rsslChnlImpl->connOptsCompVer.componentVersion.length = rsslSrvrImpl->connOptsCompVer.componentVersion.length;
		rsslChnlImpl->connOptsCompVer.componentVersion.data = rsslSrvrImpl->connOptsCompVer.componentVersion.data;
		rsslChnlImpl->ownConnOptCompVer = RSSL_FALSE;
	}

	return (&(rsslChnlImpl->Channel)); 
}

/* Closes server */
RsslRet rsslCloseServer(RsslServer *srvr, RsslError *error)
{
	rsslServerImpl *rsslSrvrImpl=0;

	if (!initialized)
	{
		_rsslSetError(error, (RsslChannel*)srvr, RSSL_RET_INIT_NOT_INITIALIZED, 0);
		snprintf(error->text, MAX_RSSL_ERROR_TEXT, "<%s:%d> rsslCloseServer() Error: 0001 RSSL not initialized.\n", __FILE__, __LINE__);
		return RSSL_RET_INIT_NOT_INITIALIZED;
	}

	if (RSSL_NULL_PTR(srvr, "rsslCloseServer", "srvr", error))
		return RSSL_RET_FAILURE;
	

	/* map the RsslServer to the ripcServer */
	rsslSrvrImpl = (rsslServerImpl*)srvr;

	srvr->state = RSSL_CH_STATE_INACTIVE;
	
	/* remove server from active list and put in free list */
	(*(rsslSrvrImpl->serverFuncs->closeServer))(rsslSrvrImpl, error);
	
	_rsslReleaseServer(rsslSrvrImpl);
	srvr=0;

	return RSSL_RET_SUCCESS;
}

/* Connects a client to an rssl server */
RsslChannel* rsslConnect(RsslConnectOptions *opts, RsslError *error)
{
	rsslChannelImpl	*rsslChnlImpl=0;
	int	retVal = RSSL_RET_FAILURE;

	if (!initialized)
	{
		_rsslSetError(error, NULL, RSSL_RET_INIT_NOT_INITIALIZED, 0);
		snprintf(error->text, MAX_RSSL_ERROR_TEXT, "<%s:%d> rsslConnect() Error: 0001 RSSL not initialized.\n", __FILE__, __LINE__);
		return NULL;
	}
	
	if (RSSL_NULL_PTR(opts, "rsslConnect", "opts", error))
		return NULL;
	
	if ((rsslChnlImpl = _rsslNewChannel()) == 0)
	{
		/* error */
		_rsslSetError(error, NULL, RSSL_RET_FAILURE, 0);
		snprintf(error->text, MAX_RSSL_ERROR_TEXT, "<%s:%d> rsslConnect() Error: 0005 could not allocate memory for new channel\n", __FILE__, __LINE__);

		return NULL;
	}

	switch(opts->connectionType)
	{
		case RSSL_CONN_TYPE_UNIDIR_SHMEM:
			rsslChnlImpl->channelFuncs = &(channelTransFuncs[RSSL_UNIDIRECTION_SHMEM_TRANSPORT]);
		break;

	    case RSSL_CONN_TYPE_RELIABLE_MCAST:
		{
			if (rsslLoadInitRsslTransportChannel(&(channelTransFuncs[RSSL_RRCP_TRANSPORT]), DEFAULT_RRCP_LIB_NAME, (void*)(&multiThread)) < 0)
			{
				_rsslSetError(error, NULL, RSSL_RET_FAILURE, errno);
				snprintf(error->text, MAX_RSSL_ERROR_TEXT,
					"<%s:%d> Error: 1008 Unable to set Reliable Multicast functions (%d).",
					__FILE__, __LINE__, errno);
				_rsslReleaseChannel(rsslChnlImpl);
				return NULL;
			}

		    rsslChnlImpl->channelFuncs = &(channelTransFuncs[RSSL_RRCP_TRANSPORT]);
		}
		break;
		case RSSL_CONN_TYPE_SEQ_MCAST:
			rsslChnlImpl->channelFuncs = &(channelTransFuncs[RSSL_SEQ_MCAST_TRANSPORT]);
		break;
		default:
			/* handles all HTTP/ENCRYPTED/(W)SOCKET cases */
			rsslChnlImpl->channelFuncs = &(channelTransFuncs[RSSL_SOCKET_TRANSPORT]);
	}		

	/* If this is an encrypted connection type and not HTTP, load OpenSSL.  This needs to be globally locked to ensure that multiple threads cannot attempt to load at the same time. */
	if (opts->connectionType == RSSL_CONN_TYPE_ENCRYPTED && opts->encryptionOpts.encryptedProtocol != RSSL_CONN_TYPE_HTTP)
	{
		mutexFuncs.staticMutexLock();
		retVal = ipcLoadOpenSSL(error);
		mutexFuncs.staticMutexUnlock();

		if (retVal != RSSL_RET_SUCCESS)
		{
			_rsslReleaseChannel(rsslChnlImpl);
			return NULL;
		}
	}

	retVal = (*(rsslChnlImpl->channelFuncs->channelConnect))(rsslChnlImpl, opts, error);

	if (retVal < RSSL_RET_SUCCESS)
	{
		_rsslReleaseChannel(rsslChnlImpl);
		return NULL;
	}

	/* add rsslChannelImpl to activeChannelList */
	mutexFuncs.staticMutexLock();
	rsslInitQueueLink(&(rsslChnlImpl->link1));
	rsslQueueAddLinkToBack(&activeChannelList, &(rsslChnlImpl->link1));
	if (memoryDebug)
		printf("adding chnl "SOCKET_PRINT_TYPE" to activeChannelList\n", rsslChnlImpl->Channel.socketId);
	mutexFuncs.staticMutexUnlock();	

	return (&(rsslChnlImpl->Channel));
}

RsslRet rsslReconnectClient(RsslChannel *chnl,  RsslError *error)
{
	rsslChannelImpl *rsslChnlImpl=0;

	if (!initialized)
	{
		_rsslSetError(error, chnl, RSSL_RET_INIT_NOT_INITIALIZED, 0);
		snprintf(error->text, MAX_RSSL_ERROR_TEXT, "<%s:%d> rsslReconnectClient() Error: 0001 RSSL not initialized.\n", __FILE__, __LINE__);
		return RSSL_RET_INIT_NOT_INITIALIZED;
	}
	
	if (RSSL_NULL_PTR(chnl, "rsslReconnectClient", "chnl", error))
		return RSSL_RET_FAILURE;

	if (chnl->state != RSSL_CH_STATE_ACTIVE)
	{
		_rsslSetError(error, chnl, RSSL_RET_FAILURE, 0);
		snprintf(error->text, MAX_RSSL_ERROR_TEXT, "<%s:%d> rsslReconnectClient() Error: 0007 Only Channels in RSSL_CH_STATE_ACTIVE state can perform tunneling reconnections.\n", __FILE__, __LINE__);
		return RSSL_RET_FAILURE;
	}

	/* Map RsslChannel to rsslChannelImpl */
	rsslChnlImpl = (rsslChannelImpl*)chnl;

	return ((*(rsslChnlImpl->channelFuncs->channelReconnect))(rsslChnlImpl, error));
}

/* Rssl channel initialization */
RsslRet rsslInitChannel(RsslChannel *chnl, RsslInProgInfo *inProg, RsslError *error)
{
	RsslRet ret;

	/* We may need to worry about the INPROG case of 
	   RIPC_INPROG_NEW_FD for tunneling, etc.  */
	rsslChannelImpl *rsslChnlImpl=0;
	
	if (!initialized)
	{
		_rsslSetError(error, chnl, RSSL_RET_INIT_NOT_INITIALIZED, 0);
		snprintf(error->text, MAX_RSSL_ERROR_TEXT, "<%s:%d> rsslInitChannel() Error: 0001 RSSL not initialized.\n", __FILE__, __LINE__);
		return RSSL_RET_INIT_NOT_INITIALIZED;
	}
	
	if (RSSL_NULL_PTR(chnl, "rsslInitChannel", "chnl", error))
		return RSSL_RET_FAILURE;

	if (rtrUnlikely(chnl->state == RSSL_CH_STATE_CLOSED))
	{
		_rsslSetError(error, chnl, RSSL_RET_FAILURE, 0);
		snprintf(error->text, MAX_RSSL_ERROR_TEXT, "<%s:%d> rsslInitChannel() Error: 0007 Channel has been closed due to prior rejection or failure, cannot continue to initialize connection.\n", __FILE__, __LINE__);
		return RSSL_RET_FAILURE;
	}
	
	rsslChnlImpl = (rsslChannelImpl*)chnl;

	/* if we have connected component versioning, bridge it through on channel here */
	if ((!rsslChnlImpl->componentVer.componentVersion.length) && (!rsslChnlImpl->componentVer.componentVersion.data))
	{
		rtrUInt32 length = (rtrUInt32) RSSL_ComponentVersionStart_Len;

		if (rsslChnlImpl->connOptsCompVer.componentVersion.data == NULL)
		{
			/* use our product version information */
			/* build it first */
			size_t rsslLinkTypeLen = strlen(rsslLinkType);

			if ((rsslChnlImpl->componentVer.componentVersion.data = _rsslMalloc(length + RSSL_ComponentVersionEnd_Len + Rssl_ComponentVersionPlatform_Len + Rssl_Bits_Len + rsslLinkTypeLen)) == NULL)
			{
				_rsslSetError(error, chnl, RSSL_RET_FAILURE, 0);
				snprintf(error->text, MAX_RSSL_ERROR_TEXT, "<%s:%d> rsslInitChannel() Error: 0005 Memory allocation failed", __FILE__, __LINE__);
				rsslChnlImpl->Channel.state = RSSL_CH_STATE_CLOSED;
				return RSSL_RET_FAILURE;
			}
			MemCopyByInt(rsslChnlImpl->componentVer.componentVersion.data, rsslComponentVersionStart, RSSL_ComponentVersionStart_Len);
			MemCopyByInt((rsslChnlImpl->componentVer.componentVersion.data + length), rsslComponentVersionPlatform, Rssl_ComponentVersionPlatform_Len);
			length += (rtrUInt32)Rssl_ComponentVersionPlatform_Len;
			MemCopyByInt((rsslChnlImpl->componentVer.componentVersion.data + length), rsslComponentVersionEnd, RSSL_ComponentVersionEnd_Len);
			length += (rtrUInt32)RSSL_ComponentVersionEnd_Len;
			MemCopyByInt((rsslChnlImpl->componentVer.componentVersion.data + length), rsslBits, Rssl_Bits_Len);
			length += (rtrUInt32)Rssl_Bits_Len;
			MemCopyByInt((rsslChnlImpl->componentVer.componentVersion.data + length), rsslLinkType, rsslLinkTypeLen);
			length += (rtrUInt32)rsslLinkTypeLen;
		}
		else
		{
			/* the user passed in component version data via connect opts*/
			/* since the string rsslComponentVersionEnd, ".rrg", begins with a period and we don't want to include that char in our
			component version string because it's redundant in this case, subtract it from the default length */
			rtrUInt32 defaultLength = (rtrUInt32)(RSSL_ComponentVersionStart_Len + RSSL_ComponentVersionEnd_Len - __RSZI8);
			rtrUInt32 totalLength = rsslChnlImpl->connOptsCompVer.componentVersion.length + __RSZI8 + defaultLength;
			rtrUInt32 userInfoLength = 0;

			if (totalLength > 253)
			{
				/* the total component data length is too long, so truncate the user defined data */
				totalLength = 253;
				userInfoLength = 253 - defaultLength - __RSZI8;
			}
			else
			{
				userInfoLength = rsslChnlImpl->connOptsCompVer.componentVersion.length;
			}

			if ((rsslChnlImpl->componentVer.componentVersion.data = _rsslMalloc(totalLength)) == NULL)
			{
				_rsslSetError(error, chnl, RSSL_RET_FAILURE, 0);
				snprintf(error->text, MAX_RSSL_ERROR_TEXT, "<%s:%d> rsslInitChannel() Error: 0005 Memory allocation failed", __FILE__, __LINE__);
				rsslChnlImpl->Channel.state = RSSL_CH_STATE_CLOSED;
				return RSSL_RET_FAILURE;
			}
			MemCopyByInt(rsslChnlImpl->componentVer.componentVersion.data, rsslComponentVersionStart, RSSL_ComponentVersionStart_Len);
			length = (rtrUInt32)RSSL_ComponentVersionStart_Len;
			/* see explanation above the declaration of defaultLength to understand the pointer arithmetic below */
			MemCopyByInt(rsslChnlImpl->componentVer.componentVersion.data + length, rsslComponentVersionEnd + __RSZI8, RSSL_ComponentVersionEnd_Len - __RSZI8);
			length += (rtrUInt32)RSSL_ComponentVersionEnd_Len - __RSZI8;
			MemCopyByInt(rsslChnlImpl->componentVer.componentVersion.data + length, "|", __RSZI8);
			length += __RSZI8;
			MemCopyByInt(rsslChnlImpl->componentVer.componentVersion.data + length, rsslChnlImpl->connOptsCompVer.componentVersion.data, userInfoLength);
			length += userInfoLength;
		}

		/* dont include null terminator, this will be done by layer below so it is consistent whether user gives us value or we use our own */
		rsslChnlImpl->componentVer.componentVersion.length = length;
		rsslChnlImpl->ownCompVer = RSSL_TRUE;
	}

	ret = ((*(rsslChnlImpl->channelFuncs->initChannel))(rsslChnlImpl, inProg, error));

	if (ret < RSSL_RET_SUCCESS)
	{
		/* set channel state to closed - error and return value are already set */
		rsslChnlImpl->Channel.state = RSSL_CH_STATE_CLOSED;
	}

	if (ret == RSSL_RET_SUCCESS)
	{
		char message[128];
		snprintf(message, sizeof(message), "Connection Established (Channel IPC descriptor = "SOCKET_PRINT_TYPE")", rsslChnlImpl->Channel.socketId);
		(void) RSSL_MUTEX_LOCK(&rsslChnlImpl->traceMutex);
		_rsslXMLDumpComment(rsslChnlImpl, message, RSSL_TRUE, RSSL_TRUE);
		(void) RSSL_MUTEX_UNLOCK(&rsslChnlImpl->traceMutex);
	}

	return ret;
}

/* Close client Channel */
RsslRet rsslCloseChannel(RsslChannel *chnl, RsslError *error)
{
	rsslChannelImpl *rsslChnlImpl=0;
	RsslRet retVal = RSSL_RET_SUCCESS;

	if (!initialized)
	{
		_rsslSetError(error, chnl, RSSL_RET_INIT_NOT_INITIALIZED, 0);
		snprintf(error->text, MAX_RSSL_ERROR_TEXT, "<%s:%d> rsslCloseChannel() Error: 0001 RSSL not initialized.\n", __FILE__, __LINE__);
		return RSSL_RET_INIT_NOT_INITIALIZED;
	}
	
	if (RSSL_NULL_PTR(chnl, "rsslCloseChannel", "chnl", error))
		return RSSL_RET_FAILURE;
	
	rsslChnlImpl = (rsslChannelImpl*)chnl;
	if (rsslChnlImpl->Channel.state == RSSL_CH_STATE_INACTIVE)
		return RSSL_RET_SUCCESS;

	if (((rsslChnlImpl->Channel.state == RSSL_CH_STATE_ACTIVE) || (rsslChnlImpl->Channel.state == RSSL_CH_STATE_INITIALIZING)))
	{
		char message[128];
		snprintf(message, sizeof(message), "rsslCloseChannel Connection closed (Channel IPC descriptor = "SOCKET_PRINT_TYPE")", rsslChnlImpl->Channel.socketId);
		(void) RSSL_MUTEX_LOCK(&rsslChnlImpl->traceMutex);
		_rsslXMLDumpComment(rsslChnlImpl, message, RSSL_TRUE, RSSL_TRUE);
		(void) RSSL_MUTEX_UNLOCK(&rsslChnlImpl->traceMutex);
	}

	/* release active buffers before closing the channel and removing the channels pool */
	_rsslReleaseActiveBuffers(rsslChnlImpl);

	retVal = (*(rsslChnlImpl->channelFuncs->channelClose))(rsslChnlImpl, error);
	
	/* free memory and close file descriptors associated with the trace options */
	if(rsslChnlImpl->traceOptionsInfo.traceMsgFilePtr != 0)
	{
		fclose(rsslChnlImpl->traceOptionsInfo.traceMsgFilePtr);
	}
	if(rsslChnlImpl->traceOptionsInfo.traceOptions.traceMsgFileName != NULL)
	{
		free(rsslChnlImpl->traceOptionsInfo.traceOptions.traceMsgFileName);
		rsslChnlImpl->traceOptionsInfo.traceOptions.traceMsgFileName = NULL;
	}
	if(rsslChnlImpl->traceOptionsInfo.newTraceMsgFileName != NULL)
	{
		free(rsslChnlImpl->traceOptionsInfo.newTraceMsgFileName);
		rsslChnlImpl->traceOptionsInfo.newTraceMsgFileName = NULL;
	}

	rsslClearTraceOptionsInfo(&(rsslChnlImpl->traceOptionsInfo));

	/* now remove the channel from active and put channel back into the free list */
	_rsslReleaseChannel(rsslChnlImpl);
	chnl=0;

	return retVal;
}

/* Change the int to be rssl data type */
/* IO control operations */
RsslRet rsslServerIoctl(RsslServer *srvr, RsslIoctlCodes code, void *value, RsslError *error)
{
	rsslServerImpl *rsslSrvrImpl=0;

	if (!initialized)
	{
		_rsslSetError(error, (RsslChannel*)srvr, RSSL_RET_INIT_NOT_INITIALIZED, 0);
		snprintf(error->text, MAX_RSSL_ERROR_TEXT, "<%s:%d> rsslServerIoctl() Error: 0001 RSSL not initialized.\n", __FILE__, __LINE__);
		return RSSL_RET_INIT_NOT_INITIALIZED;
	}

	if (RSSL_NULL_PTR(srvr, "rsslServerIoctl", "srvr", error))
		return RSSL_RET_FAILURE;

	rsslSrvrImpl = (rsslServerImpl*)srvr;
	
	return ((*(rsslSrvrImpl->serverFuncs->serverIoctl))(rsslSrvrImpl, code, value, error));
}

static void closeTraceMsgFile(RsslTraceOptionsInfo *traceOptionsInfo)
{
	/* free memory and close file descriptors associated with the trace options */
	if (traceOptionsInfo->traceMsgFilePtr != 0)
	{
		fclose(traceOptionsInfo->traceMsgFilePtr);
		traceOptionsInfo->traceMsgFilePtr = 0;
	}
	if (traceOptionsInfo->traceOptions.traceMsgFileName != NULL)
	{
		free(traceOptionsInfo->traceOptions.traceMsgFileName);
		traceOptionsInfo->traceOptions.traceMsgFileName = NULL;
	}
	if (traceOptionsInfo->newTraceMsgFileName != NULL)
	{
		free(traceOptionsInfo->newTraceMsgFileName);
	}
	traceOptionsInfo->newTraceMsgFileName = NULL;
	traceOptionsInfo->traceMsgOrigFileNameSize = 0;
}

RsslRet rsslIoctl(RsslChannel *chnl, RsslIoctlCodes code, void *value, RsslError *error)
{
	rsslChannelImpl *rsslChnlImpl=0;
	RsslTraceOptions *traceOptions=0;

	if (!initialized)
	{
		_rsslSetError(error, chnl, RSSL_RET_INIT_NOT_INITIALIZED, 0);
		snprintf(error->text, MAX_RSSL_ERROR_TEXT, "<%s:%d> rsslIoctl() Error: 0001 RSSL not initialized.\n", __FILE__, __LINE__);
		return RSSL_RET_INIT_NOT_INITIALIZED;
	}
	
	if (RSSL_NULL_PTR(chnl, "rsslIoctl", "chnl", error))
		return RSSL_RET_FAILURE;

	if (RSSL_NULL_PTR(value, "rsslIoctl", "value", error))
		return RSSL_RET_FAILURE;

	if ((chnl->state != RSSL_CH_STATE_ACTIVE) && (chnl->state != RSSL_CH_STATE_INITIALIZING))
	{
		_rsslSetError(error, chnl, RSSL_RET_FAILURE, 0);
		snprintf(error->text, MAX_RSSL_ERROR_TEXT, "<%s:%d> rsslIoctl() Error: 0007 Only Channels in RSSL_CH_STATE_ACTIVE or RSSL_CH_STATE_INITIALIZING states can change parameters.\n", __FILE__, __LINE__);
		return RSSL_RET_FAILURE;
	}

	rsslChnlImpl = (rsslChannelImpl*)chnl;
	
	switch (code)
	{
		case RSSL_TRACE:
			/* Open the file to log XML trace data in */
			traceOptions = (RsslTraceOptions*)value;
			if(traceOptions != NULL)
			{
				unsigned long long hour = 0 , min = 0, sec = 0, msec = 0;
				char timeVal[TIME_STAMP_SIZE];
				int numChars = 0;
				int needNewFile = 0;
				
				/* tracing is only intended for RWF or JSON data */
				if ( (rsslChnlImpl->Channel.protocolType != RSSL_RWF_PROTOCOL_TYPE) && (rsslChnlImpl->Channel.protocolType != RSSL_JSON_PROTOCOL_TYPE) )
				{
					_rsslSetError(error, chnl, RSSL_RET_FAILURE, 0);
					snprintf(error->text, MAX_RSSL_ERROR_TEXT, "<%s:%d> rsslIoctl() Error: Code RSSL_TRACE was specified, but the channel's protocolType is not RSSL_RWF_PROTOCOL_TYPE or RSSL_JSON_PROTOCOL_TYPE.\n", __FILE__, __LINE__);
					return RSSL_RET_FAILURE;
				}

				rsslChnlImpl->traceOptionsInfo.traceOptions.traceFlags = traceOptions->traceFlags;
				rsslChnlImpl->traceOptionsInfo.traceOptions.traceMsgMaxFileSize = traceOptions->traceMsgMaxFileSize;

				/* check if file tracing is being disabled */
				if (!(traceOptions->traceFlags & RSSL_TRACE_TO_FILE_ENABLE))
				{
					closeTraceMsgFile(&rsslChnlImpl->traceOptionsInfo);
					return RSSL_RET_SUCCESS;
				}

				if (traceOptions->traceMsgFileName == NULL)
				{
					if (rsslChnlImpl->traceOptionsInfo.traceMsgFilePtr == NULL)
					{
						/* user is attempting to enable file tracing for the first time without specifying a file name*/
						return RSSL_RET_FAILURE;
					}
					else
					{
						/* we already have a file name */
						return RSSL_RET_SUCCESS;
					}
				}
				else if ((rsslChnlImpl->traceOptionsInfo.traceOptions.traceMsgFileName == NULL)
					|| (0 != strncmp(traceOptions->traceMsgFileName, rsslChnlImpl->traceOptionsInfo.traceOptions.traceMsgFileName, strlen(traceOptions->traceMsgFileName))))
				{
					/* the user wants to change the output file for the XML trace. */
					needNewFile = 1;
				}

				if (needNewFile)
				{
					rtrUInt32 allocated;

					closeTraceMsgFile(&rsslChnlImpl->traceOptionsInfo);

					/* deep copy of traceMsgFileName */
					allocated = rsslChnlImpl->traceOptionsInfo.traceMsgOrigFileNameSize = (rtrUInt32)strlen(traceOptions->traceMsgFileName);
					rsslChnlImpl->traceOptionsInfo.traceOptions.traceMsgFileName = (char*)malloc(allocated + sizeof(char));
					if (!rsslChnlImpl->traceOptionsInfo.traceOptions.traceMsgFileName)
					{
						_rsslSetError(error, chnl, RSSL_RET_FAILURE, 0);
						snprintf(error->text, MAX_RSSL_ERROR_TEXT, "<%s:%d> rsslIoctl() Error: Unable to create memory to store file name\n", __FILE__, __LINE__);
						return RSSL_RET_FAILURE;
					}
					memcpy(rsslChnlImpl->traceOptionsInfo.traceOptions.traceMsgFileName, traceOptions->traceMsgFileName, rsslChnlImpl->traceOptionsInfo.traceMsgOrigFileNameSize);
					rsslChnlImpl->traceOptionsInfo.traceOptions.traceMsgFileName[rsslChnlImpl->traceOptionsInfo.traceMsgOrigFileNameSize] = '\0';

					/* malloc space for the modified file name, which includes the original name and TIME_STAMP_SIZE additional chars to hold time stamps (when needed)
					 * and the ".xml" extension*/
					rsslChnlImpl->traceOptionsInfo.newTraceMsgFileName = (char*)malloc(rsslChnlImpl->traceOptionsInfo.traceMsgOrigFileNameSize + TIME_STAMP_SIZE * sizeof(char));
					if (!rsslChnlImpl->traceOptionsInfo.newTraceMsgFileName)
					{
						_rsslSetError(error, chnl, RSSL_RET_FAILURE, 0);
						snprintf(error->text, MAX_RSSL_ERROR_TEXT, "<%s:%d> rsslIoctl() Error: Unable to create memory to store file name\n", __FILE__, __LINE__);
						return RSSL_RET_FAILURE;
					}

					memcpy(rsslChnlImpl->traceOptionsInfo.newTraceMsgFileName, rsslChnlImpl->traceOptionsInfo.traceOptions.traceMsgFileName, rsslChnlImpl->traceOptionsInfo.traceMsgOrigFileNameSize);

					/* add timestamp to the file's name */
					xmlGetTimeFromEpoch(&hour, &min, &sec, &msec);
					numChars = snprintf(timeVal, TIME_STAMP_SIZE, "%03llu.xml", msec);
					memcpy(rsslChnlImpl->traceOptionsInfo.newTraceMsgFileName + rsslChnlImpl->traceOptionsInfo.traceMsgOrigFileNameSize, timeVal, numChars);
					rsslChnlImpl->traceOptionsInfo.newTraceMsgFileName[rsslChnlImpl->traceOptionsInfo.traceMsgOrigFileNameSize + numChars * sizeof(char)] = '\0';

					rsslChnlImpl->traceOptionsInfo.traceMsgFilePtr = fopen(rsslChnlImpl->traceOptionsInfo.newTraceMsgFileName, "a+");

					if (rsslChnlImpl->traceOptionsInfo.traceMsgFilePtr == NULL)
					{
						_rsslSetError(error, chnl, RSSL_RET_FAILURE, 0);
						snprintf(error->text, MAX_RSSL_ERROR_TEXT, "<%s:%d> rsslIoctl() Error: Unable to open file. fopen() failed\n", __FILE__, __LINE__);
						return RSSL_RET_FAILURE;
					}
				}
			}
			return RSSL_RET_SUCCESS;
		break;
		default:
			return ((*(rsslChnlImpl->channelFuncs->channelIoctl))(rsslChnlImpl, code, value, error));
	}
}	

/* Read from socket */
RSSL_API RsslBuffer* rsslRead(RsslChannel *chnl, RsslRet *readRet, RsslError *error)
{
	RsslReadOutArgs readOutArgs = RSSL_INIT_READ_OUT_ARGS;
	RsslReadInArgs readInArgs = RSSL_INIT_READ_IN_ARGS;

	return rsslReadEx(chnl, &readInArgs, &readOutArgs, readRet, error);
}

/* Read (with extra arguments) from socket */
RSSL_API RsslBuffer* rsslReadEx(RsslChannel *chnl, RsslReadInArgs *readInArgs, RsslReadOutArgs *readOutArgs, RsslRet *readRet, RsslError *error)
{
	rsslChannelImpl *rsslChnlImpl=0;
	RsslBuffer *retBuf;

	if (rtrUnlikely(!initialized))
	{
		_rsslSetError(error, chnl, RSSL_RET_INIT_NOT_INITIALIZED, 0);
		snprintf(error->text, MAX_RSSL_ERROR_TEXT, "<%s:%d> rsslReadEx() Error: 0001 RSSL not initialized.\n", __FILE__, __LINE__);
		*readRet = RSSL_RET_INIT_NOT_INITIALIZED;
		return NULL;
	}
	
	if (rtrUnlikely(RSSL_NULL_PTR(chnl, "rsslRead", "chnl", error)))
	{
		*readRet = RSSL_RET_FAILURE;
		return NULL;
	}

	if (rtrUnlikely(RSSL_NULL_PTR(readRet, "rsslRead", "readRet", error)))
	{
		*readRet = RSSL_RET_FAILURE;
		return NULL;
	}
	
	if(rtrUnlikely(RSSL_NULL_PTR(readInArgs, "rsslRead", "readInArgs", error)))
	{
		*readRet = RSSL_RET_FAILURE;
		return NULL;
	}

	if(rtrUnlikely(RSSL_NULL_PTR(readOutArgs, "rsslRead", "readOutArgs", error)))
	{
		*readRet = RSSL_RET_FAILURE;
		return NULL;
	}

	if (rtrUnlikely(chnl->state != RSSL_CH_STATE_ACTIVE))
	{
		*readRet = RSSL_RET_FAILURE;
		_rsslSetError(error, chnl, RSSL_RET_FAILURE, 0);
		snprintf(error->text, MAX_RSSL_ERROR_TEXT, "<%s:%d> rsslReadEx() Error: 0007 Only Channels in RSSL_CH_STATE_ACTIVE state can get read.\n", __FILE__, __LINE__);
		return NULL;
	}

	readOutArgs->readOutFlags = RSSL_READ_OUT_NO_FLAGS;
	/* lock the channel mutex so that only one read per channel can occur */
	/* if its already locked, return read in progress */
	rsslChnlImpl = (rsslChannelImpl*)chnl;

	retBuf = (*(rsslChnlImpl->channelFuncs->channelRead))(rsslChnlImpl, readOutArgs, readRet, error);

	if (rtrUnlikely(rsslChnlImpl->traceOptionsInfo.traceOptions.traceFlags & (RSSL_TRACE_TO_FILE_ENABLE | RSSL_TRACE_TO_STDOUT)))
	{
		if ((retBuf != NULL) && (rsslChnlImpl->traceOptionsInfo.traceOptions.traceFlags & RSSL_TRACE_READ))
		{
			_rsslTraceStartMsg(rsslChnlImpl, rsslChnlImpl->Channel.protocolType, retBuf, readRet, traceRead, error);
			_rsslTraceEndMsg(rsslChnlImpl, readRet, RSSL_TRUE);
		}
		/* check if we read a ping */
		else if (*readRet == RSSL_RET_READ_PING)
		{	
			/* are we tracing pings? */
			if ( (rsslChnlImpl->traceOptionsInfo.traceOptions.traceFlags & RSSL_TRACE_READ) 
				&& (rsslChnlImpl->traceOptionsInfo.traceOptions.traceFlags & RSSL_TRACE_PING)
				|| (rsslChnlImpl->traceOptionsInfo.traceOptions.traceFlags & RSSL_TRACE_PING_ONLY) )
			{
				char message[128];

				snprintf(message, sizeof(message), "Incoming Ping (Channel IPC descriptor = "SOCKET_PRINT_TYPE")", rsslChnlImpl->Channel.socketId);
				(void) RSSL_MUTEX_LOCK(&rsslChnlImpl->traceMutex);
				_rsslXMLDumpComment(rsslChnlImpl, message, RSSL_TRUE, RSSL_FALSE);

				snprintf(message, sizeof(message), "End Message (Channel IPC descriptor = "SOCKET_PRINT_TYPE")", rsslChnlImpl->Channel.socketId);
				_rsslXMLDumpComment(rsslChnlImpl, message, RSSL_FALSE, RSSL_TRUE);
				(void) RSSL_MUTEX_UNLOCK(&rsslChnlImpl->traceMutex);
			}
		}
		_rsslTraceClosed(rsslChnlImpl, readRet);
	}

	return retBuf;
}

/* Write */
RsslRet rsslWrite(RsslChannel *chnl, RsslBuffer *buffer, RsslWritePriorities rsslPriority, RsslUInt8 writeFlags, RsslUInt32 *bytesWritten, RsslUInt32 *uncompressedBytesWritten, RsslError *error)
{
	rsslChannelImpl *rsslChnlImpl=0;
	rsslBufferImpl *rsslBufImpl=0;
	RsslWriteInArgs writeInArgs;
	RsslWriteOutArgs writeOutArgs = RSSL_INIT_WRITE_OUT_ARGS;
	RsslInt32 priority;
	RsslRet ret;
	
	/* we dont need to clear bytesWritten because they are cleared by the called routines */
	writeOutArgs.writeOutFlags = RSSL_WRITE_OUT_NO_FLAGS;
	writeInArgs.writeInFlags = (RsslUInt32)writeFlags;
	writeInArgs.rsslPriority = rsslPriority;

	if (rtrUnlikely(!initialized))
	{
		_rsslSetError(error, chnl, RSSL_RET_INIT_NOT_INITIALIZED, 0);
		snprintf(error->text, MAX_RSSL_ERROR_TEXT, "<%s:%d> rsslWrite() Error: 0001 RSSL not initialized.\n", __FILE__, __LINE__);
		return RSSL_RET_INIT_NOT_INITIALIZED;
	}
	
	if (rtrUnlikely(RSSL_NULL_PTR(chnl, "rsslWrite", "chnl", error)))
		return RSSL_RET_FAILURE;

	if (rtrUnlikely(RSSL_NULL_PTR(buffer, "rsslWrite", "buffer", error)))
		return RSSL_RET_FAILURE;

	if (rtrUnlikely(chnl->state != RSSL_CH_STATE_ACTIVE))
	{
		_rsslSetError(error, chnl, RSSL_RET_FAILURE, 0);
		snprintf(error->text, MAX_RSSL_ERROR_TEXT, "<%s:%d> rsslWrite() Error: 0007 Only Channels in RSSL_CH_STATE_ACTIVE state can write.\n", __FILE__, __LINE__);
		return RSSL_RET_FAILURE;
	}

	/* valid cases are a buffer with length was passed in, or it is a packed buffer and
	   a 0 length buffer is passed in - this signifys that nothing is written into the last portion of the buffer */
	if (rtrLikely((buffer->length > 0) || ((buffer->length == 0) && (((rsslBufferImpl*)buffer)->packingOffset > 0))))
	{
		rsslChnlImpl = (rsslChannelImpl*)chnl;
		rsslBufImpl = (rsslBufferImpl*)buffer;
		
		/* make sure the integrity checks out */
		if (rtrUnlikely(rsslBufImpl->integrity != 69))
		{
			/* the data has overwritten memory */
			_rsslSetError(error, chnl, RSSL_RET_BUFFER_TOO_SMALL, 0);
			snprintf(error->text, MAX_RSSL_ERROR_TEXT, "<%s:%d> rsslWrite() Error: 0008 Data has overflowed the allocated buffer length or RSSL is not owner.\n", __FILE__, __LINE__);
			return RSSL_RET_BUFFER_TOO_SMALL;
		}

		if (rtrUnlikely(rsslBufImpl->RsslChannel != rsslChnlImpl))
		{
			_rsslSetError(error, chnl, RSSL_RET_FAILURE, 0);
			snprintf(error->text, MAX_RSSL_ERROR_TEXT, "<%s:%d> rsslWrite() Error: 0018 Channel is not owner of buffer.\n", __FILE__, __LINE__);
			return RSSL_RET_FAILURE;
		}

		/* get priority checked for valid range */
		if (rtrUnlikely((rsslPriority < RSSL_HIGH_PRIORITY) || (rsslPriority > RSSL_LOW_PRIORITY)))
			priority = RSSL_MEDIUM_PRIORITY;
		else
			priority = rsslPriority;

		/* if rsslBufImpl->priorty is greater than 0 (i.e. -1), then the priority has already been set once.
		To prevent message fragments from having different priorities, we don't want to allow this to be changed again*/
		if(rsslBufImpl->priority < 0)
			rsslBufImpl->priority = priority;

		/* do debugging if wanted */
		if (rtrUnlikely((rsslChnlImpl->debugFlags & RSSL_DEBUG_RSSL_DUMP_OUT) && (buffer->length > 0)))
		{
			rsslDumpOutFuncImpl((char*)__FUNCTION__, buffer->data, buffer->length, chnl->socketId, chnl);
		}

		if (rtrUnlikely(rsslChnlImpl->traceOptionsInfo.traceOptions.traceFlags & (RSSL_TRACE_TO_FILE_ENABLE | RSSL_TRACE_TO_STDOUT)))
		{
			ret = RSSL_RET_SUCCESS;
			if(rsslChnlImpl->traceOptionsInfo.traceOptions.traceFlags & RSSL_TRACE_WRITE)
			{
				_rsslTraceStartMsg(rsslChnlImpl, rsslChnlImpl->Channel.protocolType, buffer, &ret, traceWrite, error);
			}
			ret = (*(rsslChnlImpl->channelFuncs->channelWrite))(rsslChnlImpl, rsslBufImpl, &writeInArgs, &writeOutArgs, error);
			if(rsslChnlImpl->traceOptionsInfo.traceOptions.traceFlags & RSSL_TRACE_WRITE)
			{
				_rsslTraceEndMsg(rsslChnlImpl, &ret, RSSL_FALSE);
			}
			_rsslTraceClosed(rsslChnlImpl, &ret);
		}
		else
		{
			ret = (*(rsslChnlImpl->channelFuncs->channelWrite))(rsslChnlImpl, rsslBufImpl, &writeInArgs, &writeOutArgs, error);
		}
		*bytesWritten = writeOutArgs.bytesWritten;
		*uncompressedBytesWritten = writeOutArgs.uncompressedBytesWritten;
		return ret;
	}
	else
	{
		/* trying to write empty buffer */
		_rsslSetError(error, chnl, RSSL_RET_FAILURE, 0);
		snprintf(error->text, MAX_RSSL_ERROR_TEXT, "<%s:%d> rsslWrite() Error: 0009 Buffer of length zero cannot be written\n", __FILE__, __LINE__);

		return RSSL_RET_FAILURE;
	}
}

/* Write with extra arguments */
RsslRet rsslWriteEx(RsslChannel *chnl, RsslBuffer *buffer, RsslWriteInArgs *writeInArgs, RsslWriteOutArgs *writeOutArgs, RsslError *error)
{
	rsslChannelImpl *rsslChnlImpl=0;
	rsslBufferImpl *rsslBufImpl=0;
	RsslInt32 priority;
	RsslRet ret;

	if (rtrUnlikely(!initialized))
	{
		_rsslSetError(error, chnl, RSSL_RET_INIT_NOT_INITIALIZED, 0);
		snprintf(error->text, MAX_RSSL_ERROR_TEXT, "<%s:%d> rsslWriteEx() Error: 0001 RSSL not initialized.\n", __FILE__, __LINE__);
		return RSSL_RET_INIT_NOT_INITIALIZED;
	}
	
	if (rtrUnlikely(RSSL_NULL_PTR(chnl, "rsslWrite", "chnl", error)))
		return RSSL_RET_FAILURE;

	if (rtrUnlikely(RSSL_NULL_PTR(buffer, "rsslWrite", "buffer", error)))
		return RSSL_RET_FAILURE;
	
	if (rtrUnlikely(RSSL_NULL_PTR(writeOutArgs, "rsslWrite", "writeOutArgs", error)))
		return RSSL_RET_FAILURE;

	if (rtrUnlikely(RSSL_NULL_PTR(writeInArgs, "rsslWrite", "writeInArgs", error)))
		return RSSL_RET_FAILURE;

	if (rtrUnlikely(chnl->state != RSSL_CH_STATE_ACTIVE))
	{
		_rsslSetError(error, chnl, RSSL_RET_FAILURE, 0);
		snprintf(error->text, MAX_RSSL_ERROR_TEXT, "<%s:%d> rsslWriteEx() Error: 0007 Only Channels in RSSL_CH_STATE_ACTIVE state can write.\n", __FILE__, __LINE__);
		return RSSL_RET_FAILURE;
	}

	writeOutArgs->writeOutFlags = RSSL_WRITE_OUT_NO_FLAGS;
	/* valid cases are a buffer with length was passed in, or it is a packed buffer and
	   a 0 length buffer is passed in - this signifys that nothing is written into the last portion of the buffer */
	if (rtrLikely((buffer->length > 0) || ((buffer->length == 0) && (((rsslBufferImpl*)buffer)->packingOffset > 0))))
	{
		rsslChnlImpl = (rsslChannelImpl*)chnl;
		rsslBufImpl = (rsslBufferImpl*)buffer;
		
		/* make sure the integrity checks out */
		if (rtrUnlikely(rsslBufImpl->integrity != 69))
		{
			/* the data has overwritten memory */
			_rsslSetError(error, chnl, RSSL_RET_BUFFER_TOO_SMALL, 0);
			snprintf(error->text, MAX_RSSL_ERROR_TEXT, "<%s:%d> rsslWriteEx() Error: 0008 Data has overflowed the allocated buffer length or RSSL is not owner.\n", __FILE__, __LINE__);
			return RSSL_RET_BUFFER_TOO_SMALL;
		}

		if (rtrUnlikely(rsslBufImpl->RsslChannel != rsslChnlImpl))
		{
			_rsslSetError(error, chnl, RSSL_RET_FAILURE, 0);
			snprintf(error->text, MAX_RSSL_ERROR_TEXT, "<%s:%d> rsslWriteEx()  Error: 0018 Channel is not owner of buffer.\n", __FILE__, __LINE__);
			return RSSL_RET_FAILURE;
		}

		/* get priority checked for valid range */
		if (rtrUnlikely((writeInArgs->rsslPriority < RSSL_HIGH_PRIORITY) || (writeInArgs->rsslPriority > RSSL_LOW_PRIORITY)))
			priority = RSSL_MEDIUM_PRIORITY;
		else
			priority = writeInArgs->rsslPriority;

		/* if rsslBufImpl->priorty is greater than 0 (i.e. -1), then the priority has already been set once.
		To prevent message fragments from having different priorities, we don't want to allow this to be changed again*/
		if(rsslBufImpl->priority < 0)
			rsslBufImpl->priority = priority;

		/* do debugging if wanted */
		if (rtrUnlikely((rsslChnlImpl->debugFlags & RSSL_DEBUG_RSSL_DUMP_OUT) && (buffer->length > 0)))
		{
			rsslDumpOutFuncImpl((char*)__FUNCTION__, buffer->data, buffer->length, chnl->socketId, chnl);
		}

		if (rtrUnlikely(rsslChnlImpl->traceOptionsInfo.traceOptions.traceFlags & (RSSL_TRACE_TO_FILE_ENABLE | RSSL_TRACE_TO_STDOUT)))
		{
			ret = RSSL_RET_SUCCESS;
			if(rsslChnlImpl->traceOptionsInfo.traceOptions.traceFlags & RSSL_TRACE_WRITE)
			{
				_rsslTraceStartMsg(rsslChnlImpl, rsslChnlImpl->Channel.protocolType, buffer, &ret, traceWrite, error);
			}
			ret = (*(rsslChnlImpl->channelFuncs->channelWrite))(rsslChnlImpl, rsslBufImpl,  writeInArgs, writeOutArgs, error);
			if(rsslChnlImpl->traceOptionsInfo.traceOptions.traceFlags & RSSL_TRACE_WRITE)
			{
				_rsslTraceEndMsg(rsslChnlImpl, &ret, RSSL_FALSE);
			}
			_rsslTraceClosed(rsslChnlImpl, &ret);
			return ret;
		}
		else
		{
			ret = (*(rsslChnlImpl->channelFuncs->channelWrite))(rsslChnlImpl, rsslBufImpl, writeInArgs, writeOutArgs, error);
			return ret;
		}
	}
	else
	{
		/* trying to write empty buffer */
		_rsslSetError(error, chnl, RSSL_RET_FAILURE, 0);
		snprintf(error->text, MAX_RSSL_ERROR_TEXT, "<%s:%d> rsslWriteEx() Error: 0009 Buffer of length zero cannot be written\n", __FILE__, __LINE__);

		return RSSL_RET_FAILURE;
	}
}

/* Flush socket */
RSSL_API RsslRet rsslFlush(RsslChannel *chnl, RsslError *error)
{
	RsslRet ret;
	rsslChannelImpl *rsslChnlImpl=0;

	if (rtrUnlikely(!initialized))
	{
		_rsslSetError(error, chnl, RSSL_RET_INIT_NOT_INITIALIZED, 0);
		snprintf(error->text, MAX_RSSL_ERROR_TEXT, "<%s:%d> rsslFlush() Error: 0001 RSSL not initialized.\n", __FILE__, __LINE__);
		return RSSL_RET_INIT_NOT_INITIALIZED;
	}
	
	if (rtrUnlikely(RSSL_NULL_PTR(chnl, "rsslFlush", "chnl", error)) )
		return RSSL_RET_FAILURE;

	if (rtrUnlikely(chnl->state != RSSL_CH_STATE_ACTIVE))
	{
		_rsslSetError(error, chnl, RSSL_RET_FAILURE, 0);
		snprintf(error->text, MAX_RSSL_ERROR_TEXT, "<%s:%d> rsslFlush() Error: 0007 Only Channels in RSSL_CH_STATE_ACTIVE state can flush.\n", __FILE__, __LINE__);
		return RSSL_RET_FAILURE;
	}

	rsslChnlImpl = (rsslChannelImpl*)chnl;

	ret =  ((*(rsslChnlImpl->channelFuncs->channelFlush))(rsslChnlImpl, error));
	
	if (rtrUnlikely(rsslChnlImpl->traceOptionsInfo.traceOptions.traceFlags & (RSSL_TRACE_TO_FILE_ENABLE | RSSL_TRACE_TO_STDOUT)))
		_rsslTraceClosed(rsslChnlImpl, &ret);

	return ret;
}

/* sends ping or heartbeat */
RSSL_API RsslRet rsslPing(RsslChannel *chnl, RsslError *error)
{
	rsslChannelImpl *rsslChnlImpl=0;
	
	if (!initialized)
	{
		_rsslSetError(error, chnl, RSSL_RET_INIT_NOT_INITIALIZED, 0);
		snprintf(error->text, MAX_RSSL_ERROR_TEXT, "<%s:%d> rsslPing() Error: 0001 RSSL not initialized.\n", __FILE__, __LINE__);
		return RSSL_RET_INIT_NOT_INITIALIZED;
	}

	/* should only be pinging from the active state */
	if (chnl->state != RSSL_CH_STATE_ACTIVE)
	{
		_rsslSetError(error, chnl, RSSL_RET_FAILURE, 0);
		snprintf(error->text, MAX_RSSL_ERROR_TEXT, "<%s:%d> rsslPing() Error: 0007 Only Channels in RSSL_CH_STATE_ACTIVE can send a ping.\n", __FILE__, __LINE__);
		return RSSL_RET_FAILURE;
	}

	rsslChnlImpl = (rsslChannelImpl*)chnl;

	if (rtrUnlikely(rsslChnlImpl->traceOptionsInfo.traceOptions.traceFlags & (RSSL_TRACE_TO_FILE_ENABLE | RSSL_TRACE_TO_STDOUT)))
	{
		/* are we tracing pings? */
		if ( (rsslChnlImpl->traceOptionsInfo.traceOptions.traceFlags & RSSL_TRACE_WRITE)
			&& (rsslChnlImpl->traceOptionsInfo.traceOptions.traceFlags & RSSL_TRACE_PING)
			|| (rsslChnlImpl->traceOptionsInfo.traceOptions.traceFlags & RSSL_TRACE_PING_ONLY) )
		{
			char message[128];

			(void) RSSL_MUTEX_LOCK(&rsslChnlImpl->traceMutex);
			snprintf(message, sizeof(message), "Outgoing Ping (Channel IPC descriptor = "SOCKET_PRINT_TYPE")", rsslChnlImpl->Channel.socketId);
			_rsslXMLDumpComment(rsslChnlImpl, message, RSSL_TRUE, RSSL_FALSE);

			snprintf(message, sizeof(message), "End Message (Channel IPC descriptor = "SOCKET_PRINT_TYPE")", rsslChnlImpl->Channel.socketId);
			_rsslXMLDumpComment(rsslChnlImpl, message, RSSL_FALSE, RSSL_TRUE);
			(void) RSSL_MUTEX_UNLOCK(&rsslChnlImpl->traceMutex);
		}
	}

	return ((*(rsslChnlImpl->channelFuncs->channelPing))(rsslChnlImpl, error));
}

RsslRet rsslGetChannelInfo(RsslChannel *chnl, RsslChannelInfo *info, RsslError *error)
{
	rsslChannelImpl *rsslChnlImpl=0;
	
	if (!initialized)
	{
		_rsslSetError(error, chnl, RSSL_RET_INIT_NOT_INITIALIZED, 0);
		snprintf(error->text, MAX_RSSL_ERROR_TEXT, "<%s:%d> rsslGetChannelInfo() Error: 0001 RSSL not initialized.\n", __FILE__, __LINE__);
		return RSSL_RET_INIT_NOT_INITIALIZED;
	}

	if (RSSL_NULL_PTR(chnl, "rsslGetChannelInfo", "chnl", error))
		return RSSL_RET_FAILURE;

	if (RSSL_NULL_PTR(info, "rsslGetChannelInfo", "info", error))
		return RSSL_RET_FAILURE;

	if (chnl->state != RSSL_CH_STATE_ACTIVE)
	{
		_rsslSetError(error, chnl, RSSL_RET_FAILURE, 0);
		snprintf(error->text, MAX_RSSL_ERROR_TEXT, "<%s:%d> rsslGetChannelInfo() Error: 0007 Only Channels in RSSL_CH_STATE_ACTIVE can get channel information.\n", __FILE__, __LINE__);
		return RSSL_RET_FAILURE;
	}

	rsslChnlImpl = (rsslChannelImpl*)chnl;

	return ((*(rsslChnlImpl->channelFuncs->channelGetInfo))(rsslChnlImpl, info, error));
}

RsslRet rsslGetChannelStats(RsslChannel *chnl, RsslChannelStats *stats, RsslError *error)
{
	rsslChannelImpl *rsslChnlImpl = 0;
	RsslChannelInfo info;
	RsslRet ret;

	if (!initialized)
	{
		_rsslSetError(error, chnl, RSSL_RET_INIT_NOT_INITIALIZED, 0);
		snprintf(error->text, MAX_RSSL_ERROR_TEXT, "<%s:%d> rsslGetChannelInfo() Error: 0001 RSSL not initialized.\n", __FILE__, __LINE__);
		return RSSL_RET_INIT_NOT_INITIALIZED;
	}

	if (RSSL_NULL_PTR(chnl, "rsslGetChannelInfo", "chnl", error))
		return RSSL_RET_FAILURE;

	if (RSSL_NULL_PTR(stats, "rsslGetChannelInfo", "stats", error))
		return RSSL_RET_FAILURE;

	if (chnl->state != RSSL_CH_STATE_ACTIVE)
	{
		_rsslSetError(error, chnl, RSSL_RET_FAILURE, 0);
		snprintf(error->text, MAX_RSSL_ERROR_TEXT, "<%s:%d> rsslGetChannelStats() Error: 0007 Only Channels in RSSL_CH_STATE_ACTIVE can get channel information.\n", __FILE__, __LINE__);
		return RSSL_RET_FAILURE;
	}

	rsslChnlImpl = (rsslChannelImpl*)chnl;

	if (chnl->connectionType == RSSL_CONN_TYPE_SOCKET || chnl->connectionType == RSSL_CONN_TYPE_ENCRYPTED || chnl->connectionType == RSSL_CONN_TYPE_WEBSOCKET)
	{
		return rsslSocketGetChannelStats(rsslChnlImpl, stats, error);
	}
	if (chnl->connectionType == RSSL_CONN_TYPE_RELIABLE_MCAST)
	{
		memset((void*)&info, 0, sizeof(RsslChannelInfo));
		ret = ((*(rsslChnlImpl->channelFuncs->channelGetInfo))(rsslChnlImpl, &info, error));
		if (ret == RSSL_RET_SUCCESS)
		{
			stats->multicastStats = info.multicastStats;
			return RSSL_RET_SUCCESS;
		}
		else
		{
			return RSSL_RET_SUCCESS;
		}
	}
	else
	{
		_rsslSetError(error, chnl, RSSL_RET_FAILURE, 0);
		snprintf(error->text, MAX_RSSL_ERROR_TEXT, "<%s:%d> rsslGetChannelStats() Error: 0006 Only SOCKET, ENCRYPTED(non WinInet), and RELIABLE_MULTICAST channels supported by rsslGetChannelStats.\n", __FILE__, __LINE__);
		return RSSL_RET_FAILURE;
	}
}

RsslRet rsslGetServerInfo( RsslServer *srvr, RsslServerInfo *info, RsslError *error)
{
	rsslServerImpl *rsslSrvrImpl=0;

	if (!initialized)
	{
		_rsslSetError(error, (RsslChannel*)srvr, RSSL_RET_INIT_NOT_INITIALIZED, 0);
		snprintf(error->text, MAX_RSSL_ERROR_TEXT, "<%s:%d> rsslGetServerInfo() Error: 0001 RSSL not initialized.\n", __FILE__, __LINE__);
		return RSSL_RET_INIT_NOT_INITIALIZED;
	}

	if (RSSL_NULL_PTR(srvr, "rsslGetServerInfo", "srvr", error))
		return RSSL_RET_FAILURE;

	if (RSSL_NULL_PTR(info, "rsslGetServerInfo", "info", error))
		return RSSL_RET_FAILURE;

	if (srvr->state != RSSL_CH_STATE_ACTIVE)
	{
		_rsslSetError(error, (RsslChannel*)srvr, RSSL_RET_FAILURE, 0);
		snprintf(error->text, MAX_RSSL_ERROR_TEXT, "<%s:%d> rsslGetServerInfo() Error: 0007 Only Servers in RSSL_CH_STATE_ACTIVE can get server information.\n", __FILE__, __LINE__);
		return RSSL_RET_FAILURE;
	}

	rsslSrvrImpl = (rsslServerImpl*)srvr;

	return ((*(rsslSrvrImpl->serverFuncs->serverGetInfo))(rsslSrvrImpl, info, error));
}

RSSL_API RsslInt32 rsslBufferUsage(RsslChannel *chnl, RsslError *error)
{
	rsslChannelImpl *rsslChnlImpl=0;

	if (!initialized)
	{
		_rsslSetError(error, chnl, RSSL_RET_INIT_NOT_INITIALIZED, 0);
		snprintf(error->text, MAX_RSSL_ERROR_TEXT, "<%s:%d> rsslBufferUsage() Error: 0001 RSSL not initialized.\n", __FILE__, __LINE__);
		return RSSL_RET_INIT_NOT_INITIALIZED;
	}

	if (RSSL_NULL_PTR(chnl, "rsslBufferUsage", "chnl", error))
		return RSSL_RET_FAILURE;

	
	if (chnl->state != RSSL_CH_STATE_ACTIVE)
	{
		_rsslSetError(error, chnl, RSSL_RET_FAILURE, 0);
		snprintf(error->text, MAX_RSSL_ERROR_TEXT, "<%s:%d> rsslBufferUsage() Error: 0007 Only Channels in RSSL_CH_STATE_ACTIVE can get buffer usage information.\n", __FILE__, __LINE__);
		return RSSL_RET_FAILURE;
	}

	rsslChnlImpl = (rsslChannelImpl*)chnl;

	return ((*(rsslChnlImpl->channelFuncs->channelBufferUsage))(rsslChnlImpl, error));
}

RSSL_API RsslInt32 rsslServerBufferUsage(RsslServer *srvr, RsslError *error)
{
	rsslServerImpl *rsslSrvrImpl=0;
	
	if (!initialized)
	{
		_rsslSetError(error, (RsslChannel*)srvr, RSSL_RET_INIT_NOT_INITIALIZED, 0);
		snprintf(error->text, MAX_RSSL_ERROR_TEXT, "<%s:%d> rsslServerBufferUsage() Error: 0001 RSSL not initialized.\n", __FILE__, __LINE__);
		return RSSL_RET_INIT_NOT_INITIALIZED;
	}

	if (RSSL_NULL_PTR(srvr, "rsslServerBufferUsage", "srvr", error))
		return RSSL_RET_FAILURE;

	if (srvr->state != RSSL_CH_STATE_ACTIVE)
	{
		_rsslSetError(error, (RsslChannel*)srvr, RSSL_RET_FAILURE, 0);
		snprintf(error->text, MAX_RSSL_ERROR_TEXT, "<%s:%d> rsslServerBufferUsage() Error: 0007 Only Servers in RSSL_CH_STATE_ACTIVE can get buffer usage information.\n", __FILE__, __LINE__);
		return RSSL_RET_FAILURE;
	}

	rsslSrvrImpl = (rsslServerImpl*)srvr;

	return ((*(rsslSrvrImpl->serverFuncs->serverBufferUsage))(rsslSrvrImpl, error));
}

RsslRet rsslUninitialize()
{
	_rsslStaticMutexLock();

	RTR_ATOMIC_DECREMENT(numInitCalls);

	if (numInitCalls < 0)
	{
		/* They called uninitialize without a successful initialization */
		RTR_ATOMIC_SET(numInitCalls,0);
		_rsslStaticMutexUnlock();
		return RSSL_RET_INIT_NOT_INITIALIZED;
	}
	_rsslStaticMutexUnlock();		/* release this before calling _rsslCleanUp because it uses it */

	/* clean up once the reference counter hits zero */
	if ((numInitCalls == 0) && initialized)
	{
		RTR_ATOMIC_SET(initialized,0);
		_rsslCleanUp();
		rsslUnloadTransport();

#ifndef NO_ETA_CPU_BIND
		/* Uninitialize cpuid library */
		rsslBindThreadUninitialize();
#endif

		/* uninitialize various transports */
		rsslSocketUninitialize();
		rsslUniShMemUninitialize();

		/* Unset the flag here as it is needed by rsslSocketUninitialize()*/
		multiThread = 0;
	}
	
	return RSSL_RET_SUCCESS;
}

RSSL_API RsslBuffer* rsslPackBuffer(RsslChannel *chnl, RsslBuffer *buffer,  RsslError *error)
{
	rsslBufferImpl *rsslBufImpl = 0;
	rsslChannelImpl *rsslChnlImpl = 0;

	if (rtrUnlikely(!initialized))
	{
		_rsslSetError(error, chnl, RSSL_RET_INIT_NOT_INITIALIZED, 0);
		snprintf(error->text, MAX_RSSL_ERROR_TEXT, "<%s:%d> rsslPackBuffer() Error: 0001 RSSL not initialized.\n", __FILE__, __LINE__);
		return NULL;
	}

	if (rtrUnlikely(RSSL_NULL_PTR(chnl, "rsslPackBuffer", "chnl", error)))
	{
		return NULL;
	}

	if (rtrUnlikely(RSSL_NULL_PTR(buffer, "rsslPackBuffer", "buffer", error)))
	{
		return NULL;
	}

	rsslChnlImpl = (rsslChannelImpl*)chnl;
	rsslBufImpl = (rsslBufferImpl*)buffer;

	if (rtrUnlikely(chnl->state != RSSL_CH_STATE_ACTIVE))
	{
		_rsslSetError(error, chnl, RSSL_RET_FAILURE, 0);
		snprintf(error->text, MAX_RSSL_ERROR_TEXT, "<%s:%d> rsslPackBuffer() Error: 0007 Only Channels in RSSL_CH_STATE_ACTIVE state can pack buffers.\n", __FILE__, __LINE__);
		return (&(rsslBufImpl->buffer));
	}
	
	if (rtrUnlikely(rsslBufImpl->RsslChannel != rsslChnlImpl))
	{
		_rsslSetError(error, chnl, RSSL_RET_FAILURE, 0);
		snprintf(error->text, MAX_RSSL_ERROR_TEXT, "<%s:%d> rsslPackBuffer()  Error: 0017 Channel is not owner of buffer.\n", __FILE__, __LINE__);
		return NULL;
	}

	/* make sure the integrity checks out */
	if (rtrUnlikely(rsslBufImpl->integrity != 69))
	{
		/* the data has overwritten memory */
		_rsslSetError(error, chnl, RSSL_RET_BUFFER_TOO_SMALL, 0);
		snprintf(error->text, MAX_RSSL_ERROR_TEXT, "<%s:%d> rsslPackBuffer() Error: 0011 Data has overflowed the allocated buffer length or RSSL is not owner.\n", __FILE__, __LINE__);
		return NULL;
	}

	if (rtrUnlikely(rsslBufImpl->packingOffset == 0))
	{
		_rsslSetError(error, chnl, RSSL_RET_FAILURE, 0);
		snprintf(error->text, MAX_RSSL_ERROR_TEXT, "<%s:%d> rsslPackBuffer() Error: 0017 Not a packable buffer.\n", __FILE__, __LINE__);
		return NULL;
	}

	if (rtrUnlikely(rsslChnlImpl->debugFlags & RSSL_DEBUG_RSSL_DUMP_OUT))
	{
		rsslDumpOutFuncImpl((char*)__FUNCTION__, buffer->data, buffer->length, chnl->socketId, chnl);
	}

	if (rtrUnlikely(rsslChnlImpl->traceOptionsInfo.traceOptions.traceFlags & (RSSL_TRACE_TO_FILE_ENABLE | RSSL_TRACE_TO_STDOUT)))
	{
		RsslBuffer *retBuffer;
		RsslRet ret = RSSL_RET_SUCCESS;
		_rsslTraceStartMsg(rsslChnlImpl, rsslChnlImpl->Channel.protocolType, buffer, &ret, tracePack, error);
		retBuffer = (*(rsslChnlImpl->channelFuncs->channelPackBuffer))(rsslChnlImpl, rsslBufImpl, error);
		if (retBuffer == NULL) ret = error->rsslErrorId;
		_rsslTraceEndMsg(rsslChnlImpl, &ret, RSSL_FALSE);
		_rsslTraceClosed(rsslChnlImpl, &ret);
		return buffer;
	}
		
	/* return from rsslPackBuffer function pointer */
	return (*(rsslChnlImpl->channelFuncs->channelPackBuffer))(rsslChnlImpl, rsslBufImpl, error);
}	

RSSL_API RsslBuffer* rsslGetBuffer(RsslChannel *chnl, RsslUInt32 size, RsslBool packedBuffer, RsslError *error)
{
	rsslChannelImpl *rsslChnlImpl=0;
	rsslBufferImpl *rsslBufImpl = 0;

	if (rtrUnlikely(!initialized))
	{
		_rsslSetError(error, chnl, RSSL_RET_INIT_NOT_INITIALIZED, 0);
		snprintf(error->text, MAX_RSSL_ERROR_TEXT, "<%s:%d> rsslGetBuffer() Error: 0001 RSSL not initialized.\n", __FILE__, __LINE__);
		return NULL;
	}
	
	if (rtrUnlikely(RSSL_NULL_PTR(chnl, "rsslGetBuffer", "chnl", error)))
	{
		return NULL;
	}
	
	/* possibly restrict to only active channels */
	 
	if (rtrUnlikely(chnl->state != RSSL_CH_STATE_ACTIVE))
	{
		_rsslSetError(error, chnl, RSSL_RET_FAILURE, 0);
		snprintf(error->text, MAX_RSSL_ERROR_TEXT, "<%s:%d> rsslGetBuffer() Error: 0007 Only Channels in RSSL_CH_STATE_ACTIVE state can get buffers.\n", __FILE__, __LINE__);

		return NULL;
	}

	if (rtrUnlikely(size <= 0))
	{
		_rsslSetError(error, chnl, RSSL_RET_FAILURE, 0);
		snprintf(error->text, MAX_RSSL_ERROR_TEXT, "<%s:%d> rsslGetBuffer() Error: 0010 Invaid buffer size specified.\n", __FILE__, __LINE__);
		return NULL;
	}

	rsslChnlImpl = (rsslChannelImpl*)chnl;

	rsslBufImpl = (*(rsslChnlImpl->channelFuncs->channelGetBuffer))(rsslChnlImpl, size, packedBuffer, error);

	/* error is already set from within function call above */
	if (rtrUnlikely(!rsslBufImpl))
		return NULL;
	
	/* common work done for all transport types */
	rsslBufImpl->RsslChannel = rsslChnlImpl;
	/* Set integrity to well known value */
	rsslBufImpl->integrity = 69;

	/* add to buffer list */
	if (multiThread == RSSL_LOCK_GLOBAL_AND_CHANNEL)
	  (void) RSSL_MUTEX_LOCK(&rsslChnlImpl->chanMutex);
	rsslInitQueueLink(&(rsslBufImpl->link1));
	rsslQueueAddLinkToBack(&(rsslChnlImpl->activeBufferList), &(rsslBufImpl->link1));
	if (rtrUnlikely(memoryDebug)) printf("adding to activeBufferList\n");
	if (multiThread == RSSL_LOCK_GLOBAL_AND_CHANNEL)
	  (void) RSSL_MUTEX_UNLOCK(&rsslChnlImpl->chanMutex);

	return (&(rsslBufImpl->buffer));
}

RSSL_API RsslRet rsslReleaseBuffer(RsslBuffer *buffer, RsslError *error)
{
	rsslChannelImpl *rsslChnlImpl=0;
	rsslBufferImpl* rsslBufImpl=0;

	if (rtrUnlikely(!initialized))
	{
		_rsslSetError(error, NULL, RSSL_RET_INIT_NOT_INITIALIZED, 0);
		snprintf(error->text, MAX_RSSL_ERROR_TEXT, "<%s:%d> rsslReleaseBuffer() Error: 0001 RSSL not initialized.\n", __FILE__, __LINE__);
		return RSSL_RET_INIT_NOT_INITIALIZED;
	}
	
	if (rtrUnlikely(RSSL_NULL_PTR(buffer, "rsslReleaseBuffer", "buffer", error)))
		return RSSL_RET_FAILURE;

	rsslBufImpl = (rsslBufferImpl*)buffer;

	if (rtrUnlikely(rsslBufImpl->integrity != 69))
	{
		/* buffer integrity does not check out - the casts to rsslChnlImpl and ripcSckt will not work */
		_rsslSetError(error, NULL, RSSL_RET_FAILURE, 0);
		snprintf(error->text, MAX_RSSL_ERROR_TEXT, "<%s:%d> rsslReleaseBuffer() Error: 0011 RSSL Buffer can not be released due to integrity issues.\n", __FILE__, __LINE__);
		return RSSL_RET_FAILURE;
	}

	rsslChnlImpl = rsslBufImpl->RsslChannel;

	/* general clean up first */

	/* remove buffer from list */
	if (multiThread == RSSL_LOCK_GLOBAL_AND_CHANNEL)
	  (void) RSSL_MUTEX_LOCK(&rsslChnlImpl->chanMutex);
	if (rsslQueueLinkInAList(&(rsslBufImpl->link1)) == RSSL_TRUE)
	{
		rsslQueueRemoveLink(&(rsslChnlImpl->activeBufferList), &(rsslBufImpl->link1));
		if (rtrUnlikely(memoryDebug))
			printf("removing from activeBufferList\n");
	}
	if (multiThread == RSSL_LOCK_GLOBAL_AND_CHANNEL)
	  (void) RSSL_MUTEX_UNLOCK(&rsslChnlImpl->chanMutex);

	/* first check if I allocated the data portion of the buffer */
	if (rsslBufImpl->owner == 1)
	{
		/* Checks for the websocket case */
		if (rsslBufImpl->memoryAllocationOffset != 0)
		{
			rsslBufImpl->buffer.data -= rsslBufImpl->memoryAllocationOffset;
			rsslBufImpl->memoryAllocationOffset = 0;
		}

		/* I allocated it - now free it */
		_rsslFree(buffer->data);

		if (rsslBufImpl->compressedBuffer.data)
		{
			_rsslFree(rsslBufImpl->compressedBuffer.data);
		}
	}

	(*(rsslChnlImpl->channelFuncs->channelReleaseBuffer))(rsslChnlImpl, rsslBufImpl, error);

	/* add to free buffer list */
	_rsslCleanBuffer(rsslBufImpl);
	if (rtrUnlikely(memoryDebug))
		printf("adding to freeBufferList\n");
	if (multiThread == RSSL_LOCK_GLOBAL_AND_CHANNEL)
	  (void) RSSL_MUTEX_LOCK(&rsslChnlImpl->chanMutex);
	rsslInitQueueLink(&(rsslBufImpl->link1));
	rsslQueueAddLinkToBack(&(rsslChnlImpl->freeBufferList), &(rsslBufImpl->link1));
	if (multiThread == RSSL_LOCK_GLOBAL_AND_CHANNEL)
	  (void) RSSL_MUTEX_UNLOCK(&rsslChnlImpl->chanMutex);

	return RSSL_RET_SUCCESS;
}

RSSL_API RsslUInt32 rsslCalculateEncryptedSize(const RsslBuffer *bufferToEncrypt)
{
	return CalculateEncryptedLength(bufferToEncrypt);
}

RSSL_API RsslRet rsslEncryptBuffer(const RsslChannel *chnl, const RsslBuffer* unencryptedInput, RsslBuffer* encryptedOutput, RsslError *error)
{
	RsslInt32 retval;
	rsslChannelImpl *rsslChnlImpl=0;

	/* null pointer checks, ensure that shared key exists in channel */
	if (rtrUnlikely(!initialized))
	{
		_rsslSetError(error, NULL, RSSL_RET_INIT_NOT_INITIALIZED, 0);
		snprintf(error->text, MAX_RSSL_ERROR_TEXT, "<%s:%d> rsslEncryptBuffer() Error: 0001 RSSL not initialized.\n", __FILE__, __LINE__);
		return RSSL_RET_INIT_NOT_INITIALIZED;
	}
	
	if (rtrUnlikely(RSSL_NULL_PTR(chnl, "rsslEncryptBuffer", "chnl", error)))
		return RSSL_RET_FAILURE;

	if (rtrUnlikely(RSSL_NULL_PTR(unencryptedInput, "rsslEncryptBuffer", "unencryptedInput", error)))
		return RSSL_RET_FAILURE;

	if (rtrUnlikely(RSSL_NULL_PTR(encryptedOutput, "rsslEncryptBuffer", "encryptedOutput", error)))
		return RSSL_RET_FAILURE;

	rsslChnlImpl = (rsslChannelImpl*)chnl;

	if ((unencryptedInput->length == 0) || !unencryptedInput->data)
	{
		_rsslSetError(error, &(rsslChnlImpl->Channel), RSSL_RET_FAILURE, 0);
		snprintf(error->text, MAX_RSSL_ERROR_TEXT, "<%s:%d> rsslEncryptBuffer() Error: 0009 Buffer of length zero cannot be encrypted\n", __FILE__, __LINE__);			
		return RSSL_RET_FAILURE;
	}

	if (rtrUnlikely(rsslChnlImpl->shared_key == 0))
	{
		/* populate error and return failure */		
		_rsslSetError(error, &(rsslChnlImpl->Channel), RSSL_RET_FAILURE, 0);
		snprintf(error->text, MAX_RSSL_ERROR_TEXT, "<%s:%d> rsslEncryptBuffer() Error: 1005 No encryption key present, connection does not support key exchange.\n", __FILE__, __LINE__);			
		return RSSL_RET_FAILURE;
	}

	retval = Encrypt_TR_SL1_64((RsslUInt8*)&rsslChnlImpl->shared_key, unencryptedInput, encryptedOutput);
	
	/* look for and populate error */
	switch (retval)
	{
		case 0:
			/* this should do nothing.  no need to set up text in error */
		break;
		case -1: 
			_rsslSetError(error, &(rsslChnlImpl->Channel), RSSL_RET_BUFFER_TOO_SMALL, 0);
			snprintf(error->text, MAX_RSSL_ERROR_TEXT, "<%s:%d> rsslEncryptBuffer() Error: 0020 Cannot encrypt into output buffer of size (%d). Expected length: (%d).\n", __FILE__, __LINE__, encryptedOutput->length, CalculateEncryptedLength(unencryptedInput));			
			retval = RSSL_RET_BUFFER_TOO_SMALL;
		break;
		case -2:
			_rsslSetError(error, &(rsslChnlImpl->Channel), RSSL_RET_FAILURE, 0);
			snprintf(error->text, MAX_RSSL_ERROR_TEXT, "<%s:%d> rsslEncryptBuffer() Error: 0005 Could not allocate space for encryption key\n", __FILE__, __LINE__);			
			retval = RSSL_RET_FAILURE;
		default:			
			/* something changed in cutil and we didnt account for it here... */
			_rsslSetError(error, &(rsslChnlImpl->Channel), RSSL_RET_FAILURE, 0);
			snprintf(error->text, MAX_RSSL_ERROR_TEXT, "<%s:%d> rsslEncryptBuffer() Error: 1005 Unexpected error occurred\n", __FILE__, __LINE__);			
			retval = RSSL_RET_FAILURE;
	}
	return ((retval == 0) ? RSSL_RET_SUCCESS : retval);
}

RSSL_API RsslRet rsslDecryptBuffer(const RsslChannel *chnl, const RsslBuffer* encryptedInput, RsslBuffer* decryptedOutput, RsslError *error)
{
	RsslInt32 retval;
	rsslChannelImpl *rsslChnlImpl=0;

	/* null pointer checks, ensure that shared key exists in channel */
	if (rtrUnlikely(!initialized))
	{
		_rsslSetError(error, NULL, RSSL_RET_INIT_NOT_INITIALIZED, 0);
		snprintf(error->text, MAX_RSSL_ERROR_TEXT, "<%s:%d> rsslDecryptBuffer() Error: 0001 RSSL not initialized.\n", __FILE__, __LINE__);
		return RSSL_RET_INIT_NOT_INITIALIZED;
	}
	
	if (rtrUnlikely(RSSL_NULL_PTR(chnl, "rsslDecryptBuffer", "chnl", error)))
		return RSSL_RET_FAILURE;

	if (rtrUnlikely(RSSL_NULL_PTR(encryptedInput, "rsslDecryptBuffer", "encryptedInput", error)))
		return RSSL_RET_FAILURE;

	if (rtrUnlikely(RSSL_NULL_PTR(decryptedOutput, "rsslDecryptBuffer", "decryptedOutput", error)))
		return RSSL_RET_FAILURE;

	rsslChnlImpl = (rsslChannelImpl*)chnl;

	if (rtrUnlikely(rsslChnlImpl->shared_key == 0))
	{
		/* populate error and return failure */
		_rsslSetError(error, &(rsslChnlImpl->Channel), RSSL_RET_FAILURE, 0);
		snprintf(error->text, MAX_RSSL_ERROR_TEXT, "<%s:%d> rsslDecryptBuffer() Error: 1005 No decryption key present, connection does not support key exchange.\n", __FILE__, __LINE__);			
		return RSSL_RET_FAILURE;
	}

	retval = Decrypt_TR_SL1_64((RsslUInt8*)(&rsslChnlImpl->shared_key), encryptedInput, decryptedOutput);
	
	/* look for and populate error */
	switch (retval)
	{
		case 0:
			/* success case, do nothing */
		break;
		case -1:
		case -2: 
			_rsslSetError(error, &(rsslChnlImpl->Channel), RSSL_RET_BUFFER_TOO_SMALL, 0);
			snprintf(error->text, MAX_RSSL_ERROR_TEXT, "<%s:%d> rsslDecryptBuffer() Error: 0019 encryptedInput length of (%d) is not long enough for an encrypted buffer.\n", __FILE__, __LINE__, encryptedInput->length);			
			retval = RSSL_RET_BUFFER_TOO_SMALL;
		break;
		case -3:
			_rsslSetError(error, &(rsslChnlImpl->Channel), RSSL_RET_FAILURE, 0);
			snprintf(error->text, MAX_RSSL_ERROR_TEXT, "<%s:%d> rsslDecryptBuffer() Error: 0005 Could not allocate temporary space for decryption\n", __FILE__, __LINE__);			
			retval = RSSL_RET_FAILURE;

		break;
		case -4:
			_rsslSetError(error, &(rsslChnlImpl->Channel), RSSL_RET_FAILURE, 0);
			snprintf(error->text, MAX_RSSL_ERROR_TEXT, "<%s:%d> rsslDecryptBuffer() Error: 0005 Could not allocate space for decryption key\n", __FILE__, __LINE__);			
			retval = RSSL_RET_FAILURE;
		break;
		case -5:
		case -6: 
			_rsslSetError(error, &(rsslChnlImpl->Channel), RSSL_RET_FAILURE, 0);
			snprintf(error->text, MAX_RSSL_ERROR_TEXT, "<%s:%d> rsslDecryptBuffer() Error: 1010 Content appears invalid after decryption\n", __FILE__, __LINE__);			
			retval = RSSL_RET_FAILURE;
		break;
		case -7:
			_rsslSetError(error, &(rsslChnlImpl->Channel), RSSL_RET_BUFFER_TOO_SMALL, 0);
			snprintf(error->text, MAX_RSSL_ERROR_TEXT, "<%s:%d> rsslDecryptBuffer() Error: 0020 Cannot fit decrypted output into output buffer of size (%d).\n", __FILE__, __LINE__, decryptedOutput->length);			
		break;
		default:
			/* something changed in cutil and we didnt account for it here... */
			_rsslSetError(error, &(rsslChnlImpl->Channel), RSSL_RET_FAILURE, 0);
			snprintf(error->text, MAX_RSSL_ERROR_TEXT, "<%s:%d> rsslDecryptBuffer() Error: 1005 Unexpected error occurred\n", __FILE__, __LINE__);			
			retval = RSSL_RET_FAILURE;	
	}
	return ((retval == 0) ? RSSL_RET_SUCCESS : retval);
}

RSSL_API RsslUInt32 rsslCalculateHexDumpOutputSize(const RsslBuffer *bufferToHexDump, RsslUInt32 valuesPerLine)
{
	int	charPerLine;
	int	bufferNeeded;
	/* if they dont pass these in right, we cant dump as hex so return this as 0 */
	if (!bufferToHexDump || (valuesPerLine == 0))
		return 0;

	/*          Ascii hex         +  Spaces Ascii Hex   + spaces + ascii out + null */
	charPerLine = (valuesPerLine * 2) + (valuesPerLine / 2) + 3 + valuesPerLine + 1;
	bufferNeeded = (((bufferToHexDump->length / valuesPerLine) + 1) * charPerLine);
	
	return (bufferNeeded);
}

#define RSSL_HEXDUMP_LINE_LEN 256

static char * rssl_startNewLineBuffer(
	char	*oBufPtr,
	char	*hexbuf,
	char	*strbuf,
	int		cursor,
	int		vperline,
	RsslInt *oBufLen)
{
	/* We have pre-checked input buffer size to ensure that there is enough space in the buffer. */
	short totCursor = (vperline * 2) + (vperline / 2);
	short curCursor = (cursor * 2) + (cursor / 2);
	int temp;

	temp = snprintf(oBufPtr, (size_t)*oBufLen, "%s", hexbuf);
	oBufPtr += temp;
	*oBufLen -= (RsslInt64)temp;

	while (curCursor++ < totCursor)
	{
		temp = snprintf(oBufPtr, (size_t)*oBufLen, " ");
		oBufPtr += temp;
		*oBufLen -= (RsslInt64)temp;
	}
	temp = snprintf(oBufPtr, (size_t)*oBufLen, "   %s\n", strbuf);
	oBufPtr += temp;
	*oBufLen -= (RsslInt64)temp;
	return (oBufPtr);
}

RSSL_API RsslRet rsslBufferToHexDump(const RsslBuffer* bufferToHexDump, RsslBuffer* hexDumpOutput, RsslUInt32 valuesPerLine, RsslError *error)
{
	RsslInt64		outputLen;
	char			buf[RSSL_HEXDUMP_LINE_LEN];
	char			buf1[RSSL_HEXDUMP_LINE_LEN];
	char			*hexPtr;
	char			*charPtr;
	char			*oBufPtr;
	char			*iBufCursor = bufferToHexDump->data;
	unsigned char	byte;
	RsslUInt32				position = 0;
	RsslInt32				curbyte = 0;
	RsslInt32				eobyte = 0;
	RsslUInt32	bufferNeeded;

	/* null checks */
	if (rtrUnlikely(RSSL_NULL_PTR(bufferToHexDump, "rsslBufferToHexDump", "bufferToHexDump", error)))
		return RSSL_RET_FAILURE;

	if (rtrUnlikely(RSSL_NULL_PTR(hexDumpOutput, "rsslBufferToHexDump", "hexDumpOutput", error)))
		return RSSL_RET_FAILURE;

	if (valuesPerLine == 0)
	{
		_rsslSetError(error, NULL, RSSL_RET_FAILURE, 0);
		snprintf(error->text, MAX_RSSL_ERROR_TEXT, "<%s:%d> rsslBufferToHexDump() Error: 0002 Invalid argument value of 0 for valuesPerLine.\n", __FILE__, __LINE__);
		return RSSL_RET_FAILURE;
	}

	/* Max is 70 due to length of buf and buf1 */
	if (valuesPerLine > 70)
		valuesPerLine = 70;
	else if (valuesPerLine & 0x01)
		valuesPerLine--;

	bufferNeeded = rsslCalculateHexDumpOutputSize(bufferToHexDump, valuesPerLine);

	/* Check to make sure it will fit. */
	if (bufferNeeded > hexDumpOutput->length)
	{
		_rsslSetError(error, NULL, RSSL_RET_FAILURE, 0);
		snprintf(error->text, MAX_RSSL_ERROR_TEXT, "<%s:%d> rsslBufferToHexDump() Error: 0020 Cannot fit formatted hex dump output into output buffer of size(%d)\n", __FILE__, __LINE__, hexDumpOutput->length);
		return RSSL_RET_BUFFER_TOO_SMALL;
	}
	*hexDumpOutput->data = '\0';
	oBufPtr = hexDumpOutput->data;
	outputLen = hexDumpOutput->length;


	hexPtr = buf;
	charPtr = buf1;
	while (position < bufferToHexDump->length)
	{
		byte = *iBufCursor++;
		hexPtr += snprintf(hexPtr, (size_t)(RSSL_HEXDUMP_LINE_LEN - (hexPtr - buf)), (eobyte & 1 ? "%2.2x " : "%2.2x"), byte);
		*charPtr++ = (byte >= ' ' && byte < 0x7f) ? byte : '.';
		eobyte ^= 1;
		position++;
		curbyte++;

		if ((position % valuesPerLine) == 0)
		{
			*hexPtr = *charPtr = '\0';
			oBufPtr = rssl_startNewLineBuffer(oBufPtr, buf, buf1, curbyte, valuesPerLine, &outputLen);
			hexPtr = buf;
			charPtr = buf1;
			curbyte = 0;
			eobyte = 0;
		}
		fflush(stdout);
	}
	if ((position % valuesPerLine) != 0)
	{
		*hexPtr = *charPtr = '\0';
		oBufPtr = rssl_startNewLineBuffer(oBufPtr, buf, buf1, curbyte, valuesPerLine, &outputLen);
		hexPtr = buf;
		charPtr = buf1;
		position = 0;
		curbyte = 0;
		eobyte = 0;
	}

	if (outputLen < 0)
	{
		_rsslSetError(error, NULL, RSSL_RET_BUFFER_TOO_SMALL, 0);
		snprintf(error->text, MAX_RSSL_ERROR_TEXT, "<%s:%d> rsslBufferToHexDump() Error: 0020 Cannot fit formatted hex dump output into output buffer of size (%d).\n", __FILE__, __LINE__, hexDumpOutput->length);
		return RSSL_RET_BUFFER_TOO_SMALL;
	}
	else
	{
		/* outputLen is the remaining unused amount in the buffer, and the previous check covers any potential underflow condition.
		Since outputLen starts out as hexDumpOutput->length, it will always be less than hexDumOutput->length */
		hexDumpOutput->length = hexDumpOutput->length - (RsslUInt32)outputLen;
	}

	return RSSL_RET_SUCCESS;
}


static char * rssl_startNewLineRawBuffer(
	char	*oBufPtr,
	char	*hexbuf,
	int		cursor,
	int		vperline,
	RsslInt *oBufLen )
{
	/* We have pre-checked input buffer size to ensure that there is enough space in the buffer. */
	short totCursor = (vperline * 2) + (vperline / 2);
	short curCursor = (cursor * 2) + (cursor / 2);
	int temp;

	temp = snprintf(oBufPtr, (size_t)*oBufLen, "%s\n", hexbuf);
	oBufPtr += temp;
	*oBufLen -= (RsslInt64)temp;

	while (curCursor++ < totCursor)
	{
		temp = snprintf(oBufPtr, (size_t)*oBufLen, " ");
		oBufPtr += temp;
		*oBufLen -= (RsslInt64)temp;
	}

	return (oBufPtr);
}

RSSL_API RsslRet rsslBufferToRawHexDump(const RsslBuffer* bufferToHexDump, RsslBuffer* hexDumpOutput, RsslUInt32 valuesPerLine, RsslError *error)
{
	RsslInt64		outputLen;
	char			buf[RSSL_HEXDUMP_LINE_LEN];
	char			*hexPtr;
	char			*oBufPtr;
	char			*iBufCursor = bufferToHexDump->data;
	unsigned char	byte;
	RsslUInt32				position = 0;
	RsslInt32				curbyte = 0;
	RsslInt32				eobyte = 0;
	RsslUInt32	bufferNeeded;
	
	/* null checks */
	if (rtrUnlikely(RSSL_NULL_PTR(bufferToHexDump, "rsslBufferToRawHexDump", "bufferToHexDump", error)))
		return RSSL_RET_FAILURE;

	if (rtrUnlikely(RSSL_NULL_PTR(hexDumpOutput, "rsslBufferToRawHexDump", "hexDumpOutput", error)))
		return RSSL_RET_FAILURE;

	if (valuesPerLine == 0)
	{
		_rsslSetError(error, NULL, RSSL_RET_FAILURE, 0);
		snprintf(error->text, MAX_RSSL_ERROR_TEXT, "<%s:%d> rsslBufferToRawHexDump() Error: 0002 Invalid argument value of 0 for valuesPerLine.\n", __FILE__, __LINE__);
		return RSSL_RET_FAILURE;
	}

	/* Max is 70 due to length of buf */
	if (valuesPerLine > 70)
		valuesPerLine = 70;
	else if (valuesPerLine & 0x01)
		valuesPerLine--;

	bufferNeeded = rsslCalculateHexDumpOutputSize(bufferToHexDump, valuesPerLine);

	/* Check to make sure it will fit. */
	if (bufferNeeded > hexDumpOutput->length)
	{
		_rsslSetError(error, NULL, RSSL_RET_FAILURE, 0);
		snprintf(error->text, MAX_RSSL_ERROR_TEXT, "<%s:%d> rsslBufferToRawHexDump() Error: 0020 Cannot fit formatted hex dump output into output buffer of size(%d)\n", __FILE__, __LINE__, hexDumpOutput->length);
		return RSSL_RET_BUFFER_TOO_SMALL;
	}
	*hexDumpOutput->data = '\0';
	oBufPtr = hexDumpOutput->data;
	outputLen = hexDumpOutput->length;


	hexPtr = buf;
	while (position < bufferToHexDump->length)
	{
		byte = *iBufCursor++;
		hexPtr += snprintf(hexPtr, (size_t)(RSSL_HEXDUMP_LINE_LEN - (hexPtr - buf)), (eobyte & 1 ? "%2.2x " : "%2.2x"), byte);
		eobyte ^= 1;
		position++;
		curbyte++;

		if ((position % valuesPerLine) == 0)
		{
			oBufPtr = rssl_startNewLineRawBuffer(oBufPtr, buf, curbyte, valuesPerLine, &outputLen);
			hexPtr = buf;
			curbyte = 0;
			eobyte = 0;
		}
		fflush(stdout);
	}
	if ((position % valuesPerLine) != 0)
	{
		oBufPtr = rssl_startNewLineRawBuffer(oBufPtr, buf, curbyte, valuesPerLine, &outputLen);
		hexPtr = buf;
		position = 0;
		curbyte = 0;
		eobyte = 0;
	}
	
	if (outputLen < 0)
	{
		_rsslSetError(error, NULL, RSSL_RET_BUFFER_TOO_SMALL, 0);
		snprintf(error->text, MAX_RSSL_ERROR_TEXT, "<%s:%d> rsslBufferToRawHexDump() Error: 0020 Cannot fit formatted hex dump output into output buffer of size (%d).\n", __FILE__, __LINE__, hexDumpOutput->length);			
		return RSSL_RET_BUFFER_TOO_SMALL;
	}
	else
	{
		/* outputLen is the remaining unused amount in the buffer, and the previous check covers any potential underflow condition.
			Since outputLen starts out as hexDumpOutput->length, it will always be less than hexDumOutput->length */
		hexDumpOutput->length = hexDumpOutput->length - (RsslUInt32)outputLen;
	}

	return RSSL_RET_SUCCESS;
}

RSSL_API RsslRet rsslDumpBuffer(RsslChannel *channel, RsslUInt32 protocolType, RsslBuffer* buffer, RsslError *error)
{
	rsslChannelImpl *rsslChnlImpl = 0;
	rsslBufferImpl *rsslBufImpl = 0;
	RsslRet ret = RSSL_RET_SUCCESS;

	if (rtrUnlikely(!initialized))
	{
		_rsslSetError(error, channel, RSSL_RET_INIT_NOT_INITIALIZED, 0);
		snprintf(error->text, MAX_RSSL_ERROR_TEXT, "<%s:%d> rsslDumpBuffer() Error: 0001 RSSL not initialized.\n", __FILE__, __LINE__);
		return RSSL_RET_INIT_NOT_INITIALIZED;
	}

	if (rtrUnlikely(RSSL_NULL_PTR(channel, "rsslDumpBuffer", "channel", error)))
		return RSSL_RET_FAILURE;

	if (rtrUnlikely(RSSL_NULL_PTR(buffer, "rsslDumpBuffer", "buffer", error)))
		return RSSL_RET_FAILURE;

	/* valid cases are a buffer with length was passed in, or it is a packed buffer and
	   a 0 length buffer is passed in - this signifys that nothing is written into the last portion of the buffer */
	if (rtrLikely((buffer->length > 0) || ((buffer->length == 0) && (((rsslBufferImpl*)buffer)->packingOffset > 0))))
	{
		rsslChnlImpl = (rsslChannelImpl*)channel;
		rsslBufImpl = (rsslBufferImpl*)buffer;

		if (rtrUnlikely( (rsslChnlImpl->traceOptionsInfo.traceOptions.traceFlags & RSSL_TRACE_DUMP)  && (rsslChnlImpl->traceOptionsInfo.traceOptions.traceFlags & (RSSL_TRACE_TO_FILE_ENABLE | RSSL_TRACE_TO_STDOUT))) )
		{
			_rsslTraceStartMsg(rsslChnlImpl, protocolType, buffer, &ret, traceDump, error);
		
			_rsslTraceEndMsg(rsslChnlImpl, &ret, RSSL_TRUE);
		
			_rsslTraceClosed(rsslChnlImpl, &ret);
		}

		return ret;
	}
	else
	{
		/* trying to write empty buffer */
		_rsslSetError(error, channel, RSSL_RET_FAILURE, 0);
		snprintf(error->text, MAX_RSSL_ERROR_TEXT, "<%s:%d> rsslWrite() Error: 0009 Buffer of length zero cannot be dumped\n", __FILE__, __LINE__);

		return RSSL_RET_FAILURE;
	}
}

RSSL_API ripcSSLApiFuncs* rsslGetOpenSSLAPIFuncs(RsslError* error)
{
	ripcSSLApiFuncs* tmp;
	mutexFuncs.staticMutexLock();
	tmp = ipcGetOpenSSLAPIFuncs(error);
	mutexFuncs.staticMutexUnlock();

	return tmp;
}

RSSL_API ripcCryptoApiFuncs* rsslGetOpenSSLCryptoFuncs(RsslError* error)
{
	ripcCryptoApiFuncs* tmp;
	mutexFuncs.staticMutexLock();
	tmp = ipcGetOpenSSLCryptoFuncs(error);
	mutexFuncs.staticMutexUnlock();

	return tmp;
}
