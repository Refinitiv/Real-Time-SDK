/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2019 Refinitiv. All rights reserved.            --
 *|-----------------------------------------------------------------------------
 */

#include "rtr/shmemtrans.h"
#include "rtr/rsslAlloc.h"
#include "rtr/rsslErrors.h"
#include "rtr/custmem.h"
#include <stdio.h>
#include <malloc.h>
#include <assert.h>

 // for monitoring calls of initialization and releasing
static RsslInt32 countShmTransCreate = 0;
static RsslInt32 countShmTransDestroy = 0;

rtrShmTransServer *rtrShmTransCreate(rtrShmCreateOpts *createOpts, RsslError *error)
{
	rtrUInt32 i;
	rtrUInt32 segSize;
	char errBuff[__ERROR_LEN];
	rtrShmBuffer *bufPtr = 0;
	rtrShmTransServer *trans = 0;
	rtrUInt32 bufSize = (rtrUInt32)RTR_SHM_ALIGNBYTES((sizeof(rtrShmBuffer) + createOpts->maxBufSize)); /* sizeof(rtrShmBuffer) is diff on 64 vrs 32 bit machines */

	/* layout of server */
	/* we need to pack all the data types so they have the same layout on both 32 and 64 bit machines */
	/* 32 bit machines do 4 bytes alignment and 64 bit machines do 8 bytes alignment */

/* always do 8 byte alignment to ensure atomic reads/writes */
/* defined (COMPILE_64BITS) */
	segSize = (rtrUInt32)	(RTR_SHM_ALIGNBYTES(2*sizeof(rtrUInt16) + sizeof(rtrUInt32))  +						/* shmemVersion, flags, pingTimeout */
							 RTR_SHM_ALIGNBYTES(sizeof(rtrUInt32) + sizeof(rtrUInt32)) +						/* byteWritten, protocolType */
							 RTR_SHM_ALIGNBYTES(sizeof(rtrUInt32) + sizeof(rtrUInt32)) + 						/* majorVersion, minorVersion */
							 RTR_SHM_ALIGNBYTES(sizeof(rtrUInt8) + RSSL_SHM_COMPONENT_VERSION_SIZE) +			/* component version and its (1 byte) length */
							 RTR_SHM_ALIGNBYTES(sizeof(rtrSpinLock)) +					 						/* userLock */
							 RTR_SHM_ALIGNBYTES(sizeof(rtrInt64)) +												/* seqNumServer */
							 RTR_SHM_ALIGNBYTES(sizeof(rtrShmCirBuf)));											/* circularBufferServer */

	trans = (rtrShmTransServer*)_rsslMalloc(sizeof(rtrShmTransServer));

	if (!trans)
	{
		_rsslSetError(error, 0, RSSL_RET_FAILURE, 0);
		snprintf(error->text, MAX_RSSL_ERROR_TEXT, "<%s:%d> rtrShmTransCreate unable to allocate shared memory transport.\n", __FILE__, __LINE__);
		return NULL;
	}

	trans->namedPipe = 0;

	/* not sure why this takes the segment since its not created yet - doesnt do anything with it */
	if ((trans->controlMutex = rtrShmSegCreateMutex(&trans->shMemSeg,createOpts->shMemKey,0, errBuff)) == 0)
	{
		_rsslSetError(error, 0, RSSL_RET_FAILURE, 0);
		snprintf(error->text, MAX_RSSL_ERROR_TEXT, "<%s:%d> rtrShmTransCreate unable to create control mutex (%s).\n", __FILE__, __LINE__, errBuff);
		free(trans);
		return NULL;
	}

#if SHM_PIPE || SHM_NAMEDPIPE
	/* set up the pipe. We use the FD of the pipe as the bind FD we give back to the user */
	// TODO: check create/attach/destroy to see if we should be holding the control mutex while setting up and closing pipe
	if (rssl_pipe_create(&trans->_bindPipe) < 1)
	{
		_rsslSetError(error, 0, RSSL_RET_FAILURE, 0);
		snprintf(error->text, MAX_RSSL_ERROR_TEXT, "<%s:%d> rtrShmTransCreate unable to create shared pipe.\n", __FILE__, __LINE__);
		rtrReleaseMutex(trans->controlMutex);
		free(trans);
		return NULL;
	}

	/* write a byte into the pipe to trick user into calling accept */
	if ((rssl_pipe_write(&trans->_bindPipe, "1", 1)) < 1)
	{
		_rsslSetError(error, 0, RSSL_RET_FAILURE, 0);
		snprintf(error->text, MAX_RSSL_ERROR_TEXT, "<%s:%d> rtrShmTransCreate unable to write byte to pipe.\n", __FILE__, __LINE__);
		rtrShmTransDestroy(trans, error);
		return NULL;
	}
	trans->_bindFD = rssl_pipe_get_read_fd(&trans->_bindPipe);
#endif

#if SHM_NAMEDPIPE
	/* create the named pipe used to notify the reader */
	/* the named Pipe FD will be used as the accept FD once the user calls rsslAccept() */
	if ((trans->namedPipe = rtrShmSegCreateNamedPipe(createOpts->shMemKey,0, errBuff)) == 0)
	{
		_rsslSetError(error, 0, RSSL_RET_FAILURE, 0);
		snprintf(error->text, MAX_RSSL_ERROR_TEXT, "<%s:%d> rtrShmTransCreate unable to create named pipe (%s).\n", __FILE__, __LINE__, errBuff);
		free(trans);
		return NULL;
	}
#endif

#if SHM_SOCKET_TCP
	if ((trans->_bindFD = rtrShmSegBindSocketTCP(createOpts->shMemKey, errBuff, trans->serverBlocking)) == -1)
	{
		_rsslSetError(error, 0, RSSL_RET_FAILURE, 0);
		snprintf(error->text, MAX_RSSL_ERROR_TEXT, "<%s:%d> rtrShmTransCreate unable to bind TCP socket (%s).\n", __FILE__, __LINE__, errBuff);
		free(trans);
		return NULL;
	}

#endif

#if SHM_SOCKET_UDP
	if ((trans->_bindFD = rtrShmSegBindSocketUDP(createOpts->shMemKey, errBuff)) == -1)
	{
		_rsslSetError(error, 0, RSSL_RET_FAILURE, 0);
		snprintf(error->text, MAX_RSSL_ERROR_TEXT, "<%s:%d> rtrShmTransCreate unable to bind UDP socket (%s).\n", __FILE__, __LINE__, errBuff);
		free(trans);
		return NULL;
	}

#endif
	/* the control mutex is used for segment control (attach/create/destroy) */
	rtrWaitForMutex(trans->controlMutex);

	if (rtrShmSegCreate(&trans->shMemSeg,createOpts->shMemKey,segSize + (bufSize * createOpts->numBuffers), errBuff) < 0)
	{
		_rsslSetError(error, 0, RSSL_RET_FAILURE, 0);
		snprintf(error->text, MAX_RSSL_ERROR_TEXT, "<%s:%d> rtrShmTransCreate unable to create shared memory segment with key %s and size %d (%s).\n", __FILE__, __LINE__, createOpts->shMemKey, (segSize + (bufSize * createOpts->numBuffers)), errBuff);
		rtrReleaseMutex(trans->controlMutex);
		{
			RsslError errorDestroy;
			rtrShmTransDestroy(trans, &errorDestroy);
		}
		return NULL;
	}

/* always do 8 byte alignment to ensure atomic reads/writes */
/* defined (COMPILE_64BITS) */
	/* 64 bit platforms are 8 byte aligned. We want to pack for optimal memory use */
	trans->shmemVersion = (rtrUInt16*)rtrShmBytesReserve( &trans->shMemSeg, sizeof(rtrUInt64));
	trans->flags = (rtrUInt16*)((char *)trans->shmemVersion + sizeof(rtrUInt16));
	trans->pingTimeout = (rtrUInt32*)((char *)trans->shmemVersion + 2 * sizeof(rtrUInt16));

	trans->byteWritten = (rtrUInt32*)rtrShmBytesReserve( &trans->shMemSeg, sizeof(rtrUInt64));
	trans->protocolType = (rtrUInt32*)((char *)trans->byteWritten + sizeof(rtrUInt32));

	trans->majorVersion = (rtrUInt32*)rtrShmBytesReserve( &trans->shMemSeg, sizeof(rtrUInt64));
	trans->minorVersion = (rtrUInt32*)((char *)trans->majorVersion + sizeof(rtrUInt32));

	trans->serverComponentVersionLen = (rtrUInt8 *)rtrShmBytesReserve(&trans->shMemSeg, RSSL_SHM_COMPONENT_VERSION_SIZE + sizeof(rtrUInt8));
	trans->serverComponentVersion = (char *)(trans->serverComponentVersionLen + sizeof(rtrUInt8));

	trans->userLock = (rtrSpinLock*)rtrShmBytesReserve( &trans->shMemSeg, RTR_SHM_ALIGNBYTES(sizeof(rtrSpinLock)));		/* spinlock is 4 bytes on windows and linux */
	trans->seqNumServer = (RsslUInt64*)rtrShmBytesReserve(&trans->shMemSeg, sizeof(rtrInt64));
	trans->circularBufferServer = (rtrShmCirBuf*)rtrShmBytesReserve( &trans->shMemSeg, sizeof(rtrShmCirBuf) );

	*trans->shmemVersion = 1;	/* the version of this transport */
	*trans->flags = 0;
	*trans->pingTimeout  = createOpts->pingTimeout;
	*trans->protocolType = createOpts->protocolType;
	*trans->majorVersion = createOpts->majorVersion;
	*trans->minorVersion = createOpts->minorVersion;

	trans->serverBlocking = createOpts->serverBlocking;
	trans->channelsBlocking = createOpts->channelsBlocking;

	if (createOpts->serverToClientPing)
		*trans->flags |= RSSL_SHM_SERVER_PING_ENABLED;

	*trans->seqNumServer = RSSL_SHM_MIN_SEQ_NUM;

	RTRShmCirBufServerInit(trans->circularBufferServer, createOpts->numBuffers, bufSize, &trans->shMemSeg);
	
	/* initialize buffer maxLength here */
	for (i = 0; i < createOpts->numBuffers; i++)
	{
		bufPtr = (rtrShmBuffer *)RTRShmCirBufGetWriteBuf(trans->circularBufferServer, &trans->shMemSeg);
		bufPtr->maxLength = (RsslUInt16)(bufSize - sizeof(rtrShmBuffer));		/* don't count the header */
		RTRShmCirBufWritten(trans->circularBufferServer);
	}

	RTR_SLOCK_INIT(trans->userLock);
	rtrReleaseMutex(trans->controlMutex);
	*trans->byteWritten = 0;

	++countShmTransCreate;
	return trans;
}

int rtrShmTransAccept( rtrShmTransServer *trans, RsslError *error)
{
	char tempBuf[5];

	if (!trans)
	{
		_rsslSetError(error, 0, RSSL_RET_FAILURE, 0);
		snprintf(error->text, MAX_RSSL_ERROR_TEXT, "<%s:%d> rtrShmTransAccept failed due to no shared memory transport.\n", __FILE__, __LINE__);
		return -1;
	}

#if SHM_PIPE || SHM_NAMEDPIPE
	/* this function currently does not do much - we mainly use it to
	   trick the server into calling accept so it gets an rssl channel to use for rsslGetBuffer and rsslWrite */

	if (rssl_pipe_read(&trans->_bindPipe, tempBuf, 1) <= 0)
	{
		_rsslSetError(error, 0, RSSL_RET_FAILURE, 0);
		snprintf(error->text, MAX_RSSL_ERROR_TEXT, "<%s:%d> rtrShmTransAccept unable to read byte from pipe.\n", __FILE__, __LINE__);
		return -1;
	}
#endif

#if SHM_PIPE
	/* set up a pipe.  */
	/* we need to return an FD back to the accept thread that can be used by the customer in select() */
	if (rssl_pipe_create(&trans->_acceptPipe) < 1)
	{
		_rsslSetError(error, 0, RSSL_RET_FAILURE, 0);
		snprintf(error->text, MAX_RSSL_ERROR_TEXT, "<%s:%d> rsslUniShMemAccept unable to create accept pipe.", __FILE__, __LINE__);
		return -1;
	}
	/* write a byte to trigger the user to call rsslInitChannel */
	if (rssl_pipe_write(&trans->_acceptPipe, "1", 1) < 1)
	{
		_rsslSetError(error, 0, RSSL_RET_FAILURE, 0);
		snprintf(error->text, MAX_RSSL_ERROR_TEXT, "<%s:%d> rtrShmTransAccept unable to write byte to pipe (errno = %d).\n", __FILE__, __LINE__, errno);
		rssl_pipe_close(&trans->_acceptPipe);
		rtrReleaseMutex(trans->controlMutex);
		free(trans);
		return -1;
	}
	/* either end of the pipe will work for a write notifier, give them the read end in case someday we have to send them something */
	trans->_acceptFD = rssl_pipe_get_read_fd(&trans->_acceptPipe);
#endif

#if SHM_NAMEDPIPE
	/* open the pipe we use for notification */
	fprintf(stderr, "*** opening named pipe\n");
	if ((trans->_acceptFD = rtrShmSegAcceptNamedPipe( trans->namedPipe, errBuff )) == -1)
	{
		_rsslSetError(error, 0, RSSL_RET_FAILURE, 0);
		snprintf(error->text, MAX_RSSL_ERROR_TEXT, "<%s:%d> rsslUniShMemAccept unable to create accept named pipe (%s)", __FILE__, __LINE__, errBuff);
		return -1;
	}
	*trans->byteWritten = 0;
#endif

#if SHM_SOCKET_TCP
	/* open the socket we use for notification */
	if ((trans->_acceptFD = rtrShmSegAcceptSocketTCP( trans->_bindFD, errBuff, trans->channelsBlocking )) == -1)
	{
		_rsslSetError(error, 0, RSSL_RET_FAILURE, 0);
		snprintf(error->text, MAX_RSSL_ERROR_TEXT, "<%s:%d> rsslUniShMemAccept unable to create accept TCP socket (%s)", __FILE__, __LINE__, errBuff);
		return -1;
	}
	*trans->byteWritten = 0;
#endif

#if SHM_SOCKET_UDP
	cliLen = sizeof(trans->cliAddr);
	if (rtrShmSegAcceptSocketUDP( trans->_bindFD, &trans->cliAddr, &cliLen, errBuff ) == -1)
	{
		_rsslSetError(error, 0, RSSL_RET_FAILURE, 0);
		snprintf(error->text, MAX_RSSL_ERROR_TEXT, "<%s:%d> rsslUniShMemAccept unable to create accept UDP socket (%s)", __FILE__, __LINE__, errBuff);
		return -1;
	}
	trans->_acceptFD = trans->_bindFD;
	*trans->byteWritten = 0;
#endif

	return 0;
}

/* The server deletes the shmem seg when it exits */
/* however, the segment doesnt actually go away until all the clients detach from it. */
int rtrShmTransDestroy( void *trans, RsslError *error )
	{
		rtrShmTransServer *shmTransServerns = trans;

		if (shmTransServerns->controlMutex == 0)
	{
		_rsslSetError(error, 0, RSSL_RET_FAILURE, 0);
		snprintf(error->text, MAX_RSSL_ERROR_TEXT, "<%s:%d> rtrShmTransDestroy failed because control mutex is not set.\n", __FILE__, __LINE__);
		return -1;
	}

	rtrWaitForMutex(shmTransServerns->controlMutex);
	
	if (shmTransServerns->userLock)
	{
		RTR_SLOCK_DESTROY(shmTransServerns->userLock);
		shmTransServerns->userLock = 0;
	}

	if (shmTransServerns->flags != NULL)						/* flags are available when shmTransServerns was initialized only */
		*shmTransServerns->flags |= RSSL_SHM_SERVER_SHUTDOWN;	/* let the consumers know that the server is shutting down */
	rtrShmSegDestroy(&shmTransServerns->shMemSeg);

	rtrReleaseMutex(shmTransServerns->controlMutex);
	rtrShmSegDestroyMutex(shmTransServerns->controlMutex);
#ifdef SHM_NAMEDPIPE
	rtrShmSegDestroyNamedPipe(shmTransServerns->namedPipe);
#endif
	shmTransServerns->controlMutex = 0;

	/* close the pipe(s) */
	rssl_pipe_close(&shmTransServerns->_bindPipe);
	rssl_pipe_close(&shmTransServerns->_acceptPipe);
	free(trans);

	++countShmTransDestroy;
	return 0;
}


rtrShmTransClient* rtrShmTransAttach( rtrShmAttachOpts *attachOpts, RsslError *error )
{
	char *curLoc;
	char errBuff[__ERROR_LEN];
	rtrShmTransClient *trans = (rtrShmTransClient*)_rsslMalloc(sizeof(rtrShmTransClient));

	if (!trans)
	{
		_rsslSetError(error, 0, RSSL_RET_FAILURE, 0);
		snprintf(error->text, MAX_RSSL_ERROR_TEXT, "<%s:%d> rtrShmTransAttach unable to allocate memory for shared memory transport.\n", __FILE__, __LINE__);
		free(trans);
		return NULL;
	}
	trans->currentBuffer = 0;
	trans->seqNumServer = 0;
	trans->readBuffer = 0;
	trans->namedPipe = 0;

	if ((trans->controlMutex = rtrShmSegAttachMutex(&trans->shMemSeg,attachOpts->shMemKey,0, errBuff)) == 0)
	{
		_rsslSetError(error, 0, RSSL_RET_FAILURE, 0);
		snprintf(error->text, MAX_RSSL_ERROR_TEXT, "<%s:%d> rtrShmTransAttach unable to attach to control mutex (%s).\n", __FILE__, __LINE__, errBuff);
		free(trans);
		return NULL;
	}

#if SHM_PIPE
	/* set up the pipe. We dont need to be holding the control mutex while we do this */
	// TODO: check if we should be holding control mutex for pipe creation/deletion
	if (rssl_pipe_create(&trans->_pipe) < 1)
	{
		_rsslSetError(error, 0, RSSL_RET_FAILURE, 0);
		snprintf(error->text, MAX_RSSL_ERROR_TEXT, "<%s:%d> rtrShmTransAttach unable to create pipe.\n", __FILE__, __LINE__);
		rtrReleaseMutex(trans->controlMutex);
		free(trans);
		return NULL;
	}

	/* Very lazy pipe use - we will just write a byte
	   here now and this will keep the user constantly triggered to
	   call read - even if they are not doing blocking IO, they will be  */
	if (rssl_pipe_write(&trans->_pipe, "1", 1) < 1)
	{
		_rsslSetError(error, 0, RSSL_RET_FAILURE, 0);
		snprintf(error->text, MAX_RSSL_ERROR_TEXT, "<%s:%d> rtrShmTransAttach unable to write byte to pipe.\n", __FILE__, __LINE__);
		rssl_pipe_close(&trans->_pipe);
		rtrReleaseMutex(trans->controlMutex);
		free(trans);
		return NULL;
	}
	trans->clientFD = rssl_pipe_get_read_fd(&trans->_pipe);
#endif

	/* the control mutex is used for segment control (attach/create/destroy) */
	rtrWaitForMutex(trans->controlMutex);

	/* connect to shared memory segments */
	/* attach first because the attach code will read all of shared memory to get it into cache and mapped */
	if (rtrShmSegAttach(&trans->shMemSeg,attachOpts->shMemKey, errBuff) < 0)
	{
		_rsslSetError(error, 0, RSSL_RET_FAILURE, 0);
		snprintf(error->text, MAX_RSSL_ERROR_TEXT, "<%s:%d> rtrShmTransAttach unable to attach to shared memory segment with key %s (%s).\n", __FILE__, __LINE__,attachOpts->shMemKey, errBuff);
		rtrReleaseMutex(trans->controlMutex);
		free(trans);
		return NULL;
	}

	trans->readRetries = 0;
	trans->maxReaderRetryThreshhold = attachOpts->maxReaderRetryThreshhold;
	trans->blockingIO = attachOpts->blockingIO;
	curLoc = (char*)trans->shMemSeg.base + trans->shMemSeg.hdr->headerLen;

/* always do 8 byte alignment to ensure atomic reads/writes */
/* defined (COMPILE_64BITS) */
	/* 64 bit platforms are 8 byte aligned. Pack for optimal memory use */
	trans->shmemVersion = (rtrUInt16*)rtrShmBytesAttach( &curLoc, sizeof(rtrUInt64));
	trans->flags = (rtrUInt16*)((char *)trans->shmemVersion + sizeof(rtrUInt16));
	trans->pingTimeout = (rtrUInt32*)((char *)trans->shmemVersion + 2*sizeof(rtrUInt16));

	trans->byteWritten = (rtrUInt32*)rtrShmBytesAttach( &curLoc, RTR_SHM_ALIGNBYTES(sizeof(rtrUInt64)));
	trans->protocolType = (rtrUInt32*)((char *)trans->byteWritten + sizeof(rtrUInt32));

	trans->majorVersion = (rtrUInt32*)rtrShmBytesAttach( &curLoc, sizeof(rtrUInt64));
	trans->minorVersion = (rtrUInt32*)((char *)trans->majorVersion + sizeof(rtrUInt32));

	trans->serverComponentVersionLen = (rtrUInt8 *)rtrShmBytesAttach(&curLoc, RSSL_SHM_COMPONENT_VERSION_SIZE + sizeof(rtrUInt8));
	trans->serverComponentVersion = (char *)(trans->serverComponentVersionLen + sizeof(rtrUInt8));

	trans->userLock = (rtrSpinLock*) rtrShmBytesAttach( &curLoc, RTR_SHM_ALIGNBYTES(sizeof(rtrSpinLock)));		/* spinlock 4 bytes on win and Linux */
	trans->seqNumServer = (RsslUInt64*)rtrShmBytesAttach( &curLoc, sizeof(rtrInt64));
	trans->circularBufferServer = (rtrShmCirBuf*)rtrShmBytesAttach(&curLoc,sizeof(rtrShmCirBuf));


#if 0	/* for alignment debugging */
	printf("flags = %p(%x)\n", trans->flags, *trans->flags);
	printf("byteWritten = %p(%d)\n", trans->byteWritten, *trans->byteWritten);
	printf("pingTimeout = %p(%d)\n", trans->pingTimeout, *trans->pingTimeout);
	printf("protocolType = %p(%d)\n", trans->protocolType, *trans->protocolType);
	printf("userLock = %p\n", trans->userLock);
	printf("majorVersion = %p(%d)\n", trans->majorVersion, *trans->majorVersion);
	printf("minorVersion = %p(%d)\n", trans->minorVersion, *trans->minorVersion);
	printf("serverComponentVersionLen = %p(%d)\n", trans->serverComponentVersionLen, *trans->serverComponentVersionLen);
	printf("serverComponentVersion = %p\n", trans->serverComponentVersion);
	printf("seqNumServer = %p\n", trans->seqNumServer);
	printf("circularBufferServer = %p\n", trans->circularBufferServer);
#endif

	/*make sure the shmem seg we are reading is the right version */
	/* if its a newer version, then we shouldnt try to read it */
	if (*trans->shmemVersion != 1)
	{
		_rsslSetError(error, 0, RSSL_RET_FAILURE, 0);
		snprintf(error->text, MAX_RSSL_ERROR_TEXT, "<%s:%d> rtrShmTransAttach incompatible with newer shmem segment (version = %d).\n", __FILE__, __LINE__, *trans->shmemVersion);
		free(trans);
		return NULL;
	}

	/* allocate a buffer that we return to the client */
	/* do this malloc before the RTRShmCirBufClientInit. */
	/* if we did it after RTRShmCirBufClientInit, we would delay the return back to the client (and maybe cause them to get too far behind) */
	/* Do this outside of the userLock region so we dont hold up the writer */
	if ((trans->currentBuffer = (char *)_rsslMalloc(trans->circularBufferServer->maxBufSize+8)) == NULL)
	{
		rtrReleaseMutex(trans->controlMutex);
		_rsslSetError(error, 0, RSSL_RET_FAILURE, 0);
		snprintf(error->text, MAX_RSSL_ERROR_TEXT, "<%s:%d> rtrShmTransAttach failed to malloc buffer (size = %d).\n", __FILE__, __LINE__, trans->circularBufferServer->maxBufSize);
		_rsslFree(trans);
		return NULL;
	}

	/* the server holds the userlock mutex while it is writing. */
	/* we grab the mutex here so we get a consistent view of shmem while we are initializing */
	/* note that the server cannot write while we have the mutex, so dont hold it for long! */
	/* we want to do put off doing this until just before we return to the client */
	/* The server is busy writing and we dont want the client to fall too far behind before they start reading */
	RTR_SHTRANS_LOCK(trans->userLock);
	trans->seqNumClient = *trans->seqNumServer;
	RTRShmCirBufClientInit(&trans->circularBufferClient, trans->circularBufferServer);
	RTR_SHTRANS_UNLOCK(trans->userLock);

	rtrReleaseMutex(trans->controlMutex);

	/* attach to the notifier after the shared segment. When we attach to the notifier, the provider knows we are here and will start writing. */
	/* if we attach to the notifier first, then the provider would start writing to shared memory before we have attached. */
#if SHM_NAMEDPIPE
	/* attach to the named pipe used to notify the reader */
	if ((trans->namedPipe = rtrShmSegAttachNamedPipe(attachOpts->shMemKey,0, errBuff)) == 0)
	{
		_rsslSetError(error, 0, RSSL_RET_FAILURE, 0);
		snprintf(error->text, MAX_RSSL_ERROR_TEXT, "<%s:%d> rtrShmTransAttach unable to attach named pipe (%s).\n", __FILE__, __LINE__, errBuff);
		free(trans);
		return NULL;
	}
	trans->clientFD = trans->namedPipe->fd;
#endif

#if SHM_SOCKET_TCP
	/* attach to the socket used to notify the reader */
	if ((trans->clientFD = rtrShmSegAttachSocketTCP(attachOpts->shMemKey, errBuff, attachOpts->blockingIO)) == 0)
	{
		_rsslSetError(error, 0, RSSL_RET_FAILURE, 0);
		snprintf(error->text, MAX_RSSL_ERROR_TEXT, "<%s:%d> rtrShmTransAttach unable to attach to TCP socket (%s).\n", __FILE__, __LINE__, errBuff);
		free(trans);
		return NULL;
	}
#endif

#if SHM_SOCKET_UDP
	/* attach to the socket used to notify the reader */
	if ((trans->clientFD = rtrShmSegAttachSocketUDP(attachOpts->shMemKey, errBuff)) == 0)
	{
		_rsslSetError(error, 0, RSSL_RET_FAILURE, 0);
		snprintf(error->text, MAX_RSSL_ERROR_TEXT, "<%s:%d> rtrShmTransAttach unable to attach to UDP socket (%s).\n", __FILE__, __LINE__, errBuff);
		free(trans);
		return NULL;
	}
#endif

	/* initialize max sequence number lag for reader */
	if (attachOpts->maxReaderSeqNumLag > trans->circularBufferClient.numBuffers)
	{
		_rsslSetError(error, 0, RSSL_RET_FAILURE, 0);
		snprintf(error->text, MAX_RSSL_ERROR_TEXT, "<%s:%d> maxReaderSeqNumLag(%llu) greater than number of buffer(%d).\n", 
				__FILE__, __LINE__, attachOpts->maxReaderSeqNumLag, trans->circularBufferClient.numBuffers);
		rtrShmTransDetach(trans, error);
		return NULL;
	}

	if (attachOpts->maxReaderSeqNumLag > 0)
	{
		trans->maxReaderSeqNumLag = attachOpts->maxReaderSeqNumLag;
	}
	else
	{
		trans->maxReaderSeqNumLag = (trans->circularBufferClient.numBuffers * 3)/4;
	}

	if (*trans->protocolType != attachOpts->protocolType)
	{
		_rsslSetError(error, 0, RSSL_RET_FAILURE, 0);
		snprintf(error->text, MAX_RSSL_ERROR_TEXT, "<%s:%d> client protocol type(%d) incompatible with server(%d).\n", __FILE__, __LINE__, attachOpts->protocolType, *trans->protocolType);
		rtrShmTransDetach(trans, error);
		return NULL;
	}

	if (*trans->majorVersion != attachOpts->majorVersion)
	{
		_rsslSetError(error, 0, RSSL_RET_FAILURE, 0);
		snprintf(error->text, MAX_RSSL_ERROR_TEXT, "<%s:%d> client majorVersion(%d) incompatible with server(%d).\n", __FILE__, __LINE__, attachOpts->majorVersion, *trans->majorVersion);
		rtrShmTransDetach(trans, error);
		return NULL;
	}
	return trans;
}


int rtrShmTransDetach( rtrShmTransClient *trans, RsslError *error )
{
	rtrWaitForMutex(trans->controlMutex);

	trans->userLock = 0;
	trans->circularBufferServer = 0;
	trans->readBuffer = 0;

	rtrShmSegDetach(&trans->shMemSeg);

	rtrReleaseMutex(trans->controlMutex);

	rtrShmSegDestroyMutex(trans->controlMutex);

	trans->controlMutex = 0;
	if (trans->currentBuffer != 0) 
	{	
		_rsslFree (trans->currentBuffer);
		trans->currentBuffer = 0;
	}

#ifdef SHM_NAMEDPIPE
	rtrShmSegDetachNamedPipe(trans->namedPipe);
#endif
	trans->namedPipe = 0;
	rssl_pipe_close(&trans->_pipe);

	free(trans);
	return 0;
}


/* client uses to read */
rtrShmBuffer* rtrShmTransClientRead(rtrShmTransClient *trans, RsslChannel *chnl, RsslRet *readRet, RsslError *error)
{
	RsslUInt64 reader_seqnum_lag;

	if (rtrLikely(trans->readBuffer != 0))
	{
		/* we must release the last read buffer first */
		rtrShmTransRelBuffer(trans);
	}

	/* now get the next buffer and store it in our read buffer */
	/* if the user is not blocking just grab a buffer out of the queue and move on */
	/* if nothing is returned there is nothing to read */
	if (rtrLikely(!trans->blockingIO))
	{
		/* when data is available client seq num is less than server seq num */
		/* the writer is doing atomic increments of the seqNumServer */
		/* We want to avoid accessing this variable more than once per read so we dont slow down the increments */
		/* Also, the seqNumServer may change in between multiple accesses */
		reader_seqnum_lag = (RsslUInt64)(RTR_ATOMIC_READ64(trans->seqNumServer) - trans->seqNumClient);
		if (rtrLikely(reader_seqnum_lag != 0))
		{
			/*
			if (reader_seqnum_lag > 100)	printf("reader_seqnum_lag = %llu  seqNumClient = %llu\n", reader_seqnum_lag, trans->seqNumClient);
			*/
			/* return failure if reader is too slow */
			if (rtrUnlikely(reader_seqnum_lag > trans->maxReaderSeqNumLag))
			{
				_rsslSetError(error, 0, RSSL_RET_SLOW_READER, 0);
				snprintf(error->text, MAX_RSSL_ERROR_TEXT, "<%s:%d> rtrShmTransClientRead disconnected from shared memory because reader lags writer by %llu messages.\n", __FILE__, __LINE__, reader_seqnum_lag);
				chnl->state = RSSL_CH_STATE_CLOSED;
				return 0;
			}
			trans->readBuffer = (rtrShmBuffer*)RTRShmCirBufRead(&trans->circularBufferClient, &trans->shMemSeg);

			/* copy the buffer and its header to make sure we have given the client a buffer that wont be overwritten by the server*/
			MemCopyByInt((void *)trans->currentBuffer, (void *)trans->readBuffer, (trans->readBuffer->length + sizeof(rtrShmBuffer)) ); 
			trans->readBuffer = (rtrShmBuffer*)trans->currentBuffer;	/* return the copied buffer to the client */
			reader_seqnum_lag = (RsslUInt64)(RTR_ATOMIC_READ64(trans->seqNumServer) - trans->seqNumClient);
			if (rtrUnlikely(reader_seqnum_lag > (trans->circularBufferClient.numBuffers - 1)))	/* check if server passed us during the memcpy */
			{
				_rsslSetError(error, 0, RSSL_RET_SLOW_READER, 0);
				snprintf(error->text, MAX_RSSL_ERROR_TEXT, "<%s:%d> rtrShmTransClientRead disconnected from shared memory because reader lags writer by %llu messages.\n", __FILE__, __LINE__, reader_seqnum_lag);
				chnl->state = RSSL_CH_STATE_CLOSED;
				return 0;
			}
			*readRet = (RsslRet) reader_seqnum_lag;
		}
		else	/* nothing to read */
		{	
			/* set readBuffer to 0 if nothing available to read */
			trans->readBuffer = 0;
			if (rtrUnlikely(*trans->flags & RSSL_SHM_SERVER_SHUTDOWN))
			{
				_rsslSetError(error, 0, RSSL_RET_FAILURE, 0);
				snprintf(error->text, MAX_RSSL_ERROR_TEXT, "<%s:%d> rtrShmTransClientRead disconnected from shared memory because the provider terminated.\n", __FILE__, __LINE__);
				chnl->state = RSSL_CH_STATE_CLOSED;
				return 0;
			}
			_rsslSetError(error, chnl, RSSL_RET_READ_WOULD_BLOCK, 0);
		}
	}
	else
	{
		/* if the user is blocking, loop on the queue read */
		trans->readBuffer = 0;
	
		while (chnl->state == RSSL_CH_STATE_ACTIVE && trans->readBuffer == 0)
		{
			/* when data is available client seq num is less than server seq num */
			reader_seqnum_lag = (RsslUInt64)(*trans->seqNumServer - trans->seqNumClient);
			if (rtrLikely(reader_seqnum_lag > 0))
			{
				/* return failure if reader is too slow */
				if (reader_seqnum_lag > trans->maxReaderSeqNumLag)
				{
					_rsslSetError(error, 0, RSSL_RET_SLOW_READER, 0);
					snprintf(error->text, MAX_RSSL_ERROR_TEXT, "<%s:%d> rtrShmTransClientRead failed because reader lags writer by %llu messages.\n", __FILE__, __LINE__, reader_seqnum_lag);
					chnl->state = RSSL_CH_STATE_CLOSED;
					return 0;
				}
				*readRet = (RsslRet) reader_seqnum_lag;
				trans->readBuffer = (rtrShmBuffer*)RTRShmCirBufRead(&trans->circularBufferClient, &trans->shMemSeg);
			}
			else /* set readBuffer to 0 if nothing available to read */
			{
				trans->readBuffer = 0;
				if (*trans->flags & RSSL_SHM_SERVER_SHUTDOWN)
				{
					_rsslSetError(error, 0, RSSL_RET_FAILURE, 0);
					snprintf(error->text, MAX_RSSL_ERROR_TEXT, "<%s:%d> rtrShmTransClientRead disconnected from shared memory because the provider terminated.\n", __FILE__, __LINE__);
					chnl->state = RSSL_CH_STATE_CLOSED;
					return 0;
				}
			}
		}
		/* return 0 if channel no longer active */
		if (chnl->state != RSSL_CH_STATE_ACTIVE)
		{
			_rsslSetError(error, 0, RSSL_RET_FAILURE, 0);
			snprintf(error->text, MAX_RSSL_ERROR_TEXT, "<%s:%d> rtrShmTransClientRead failed due to channel is no longer active.\n", __FILE__, __LINE__);
			return 0;
		}
	}

	return trans->readBuffer;
}

RsslInt32 ripcGetCountShmTransCreate()
{
	return countShmTransCreate;
}

RsslInt32 ripcGetCountShmTransDestroy()
{
	return countShmTransDestroy;
}
