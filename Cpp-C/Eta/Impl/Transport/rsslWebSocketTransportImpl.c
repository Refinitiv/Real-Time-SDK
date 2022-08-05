/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2020 Refinitiv. All rights reserved.              --
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
#endif


#include <setjmp.h>
#include <ctype.h>
#include <stdint.h>

#include "rtr/ripchttp.h"
#include "rtr/rsslCurlJIT.h"
#include "curl/curl.h"

#include "rtr/rwsutils.h"

/* global debug function pointers */
static void(*rsslWebSocketDumpInFunc)(const char *functionName, char *buffer, RsslUInt32 length, RsslSocket socketId) = 0;
static void(*rsslWebSocketDumpOutFunc)(const char *functionName, char *buffer, RsslUInt32 length, RsslSocket socketId) = 0;

void(*webSocketDumpInFunc)(const char *functionName, char *buf, RsslUInt32 len, RsslUInt64 opaque) = 0;
void(*webSocketDumpOutFunc)(const char *functionName, char *buf, RsslUInt32 len, RsslUInt64 opaque) = 0;

RTR_C_INLINE void rsslWebSocketDumpInFuncImpl(const char* functionName, char* buffer, RsslUInt32 length, RsslSocket socketId, RsslChannel* pChannel);
RTR_C_INLINE void rsslWebSocketDumpOutFuncImpl(const char* functionName, char* buffer, RsslUInt32 length, RsslSocket socketId, RsslChannel* pChannel);

/* List of the WebSocket transport function entry points for the different protocol types */
RsslSocketDumpFuncs rsslWebSocketDumpFuncs[MAX_PROTOCOL_TYPES];

extern RsslInt32 rwsDbgFuncs( void(*dumpIn)(const char *functionName, char *buf, RsslUInt32 len, RsslUInt64 opaque),
                       void(*dumpOut)(const char *functionName, char *buf, RsslUInt32 len, RsslUInt64 opaque));

extern RsslInt32 rwsDbgFuncsEx(RsslDebugFunctionsExOpts* pOpts);

/************************************
 * START PUBLIC ABSTRACTED FUNCTIONS
 ************************************/

/* rssl Web Socket Read */
RSSL_RSSL_SOCKET_IMPL_FAST(RsslBuffer*) rsslWebSocketRead(rsslChannelImpl* rsslChnlImpl, RsslReadOutArgs *readOutArgs, RsslRet *readRet, RsslError *error)
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

	_DEBUG_TRACE_WS_READ("fd "SOCKET_PRINT_TYPE" \n", rsslSocketChannel->stream)

	if (IPC_NULL_PTR(rsslSocketChannel, "rsslWebSocketRead", "rsslSocketChannel", error))
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

	/* This can only happen in the threaded case */
	if (rsslSocketChannel->workState & RIPC_INT_READ_THR)
	{
		ipcReadRet = RSSL_RET_READ_WOULD_BLOCK;

		ripcBuffer = 0;
	}
	else
	{
		rsslSocketChannel->workState |= RIPC_INT_READ_THR;

		ripcBuffer = (rtr_msgb_t *)rwsReadWebSocket(rsslSocketChannel, &ipcReadRet, &ripcMoreData, &inBytes, &uncompInBytes, &packing, error);
		
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
		/* message is one fragment */
		if (ripcBuffer->length > 0)
		{
			/* im not the memory owner */
			rsslChnlImpl->returnBufferOwner = 0;

			/* check for packing */
			// Currently there is no support for unpacking JSON messages from an array
			rsslChnlImpl->returnBuffer.length = (RsslUInt32)ripcBuffer->length;
			rsslChnlImpl->returnBuffer.data = ripcBuffer->buffer;
		}
		else
		{
			/* received a ping with no payload */
			returnNull = 1;
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
			if ((rsslChnlImpl->debugFlags & RSSL_DEBUG_RSSL_DUMP_IN) && (!returnNull))
			{
				rsslWebSocketDumpInFuncImpl(__FUNCTION__, rsslChnlImpl->returnBuffer.data, rsslChnlImpl->returnBuffer.length,
					rsslChnlImpl->Channel.socketId, &rsslChnlImpl->Channel);
			}

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
				if ((rsslChnlImpl->debugFlags & RSSL_DEBUG_RSSL_DUMP_IN) && (!returnNull))
				{
					rsslWebSocketDumpInFuncImpl(__FUNCTION__, rsslChnlImpl->returnBuffer.data, rsslChnlImpl->returnBuffer.length,
						rsslChnlImpl->Channel.socketId, &rsslChnlImpl->Channel);
				}

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

char* increaseSizeAndRealloc(char* pOldDataBuffer, RsslUInt32 originalBufferLength, size_t newSize, RsslError* error)
{
	char* ptempBuffer = NULL;
	if (newSize > 0)
	{
		ptempBuffer = (char*)malloc(newSize);
	}

	if (ptempBuffer == NULL)
	{
		_rsslSetError(error, NULL, RSSL_RET_FAILURE, errno);
		snprintf(error->text, MAX_RSSL_ERROR_TEXT,
			"<%s:%d> Could not allocate memory for WS compression buffer.",
			__FILE__, __LINE__);
		return(0);
	}

	memcpy(ptempBuffer, pOldDataBuffer, originalBufferLength);
	_rsslFree(pOldDataBuffer);
	return ptempBuffer;
}

/* rssl Socket Write */
RSSL_RSSL_SOCKET_IMPL_FAST(RsslRet) rsslWebSocketWrite(rsslChannelImpl *rsslChnlImpl, rsslBufferImpl *rsslBufImpl, RsslWriteInArgs *writeInArgs,
	RsslWriteOutArgs *writeOutArgs, RsslError *error)
{
	RsslInt32 retVal = RSSL_RET_FAILURE;
	RsslUInt32 outBytes = 0;
	RsslUInt32 uncompOutBytes = 0;
	RsslUInt32 totalOutBytes = 0;
	RsslUInt32 totalUncompOutBytes = 0;
	RsslUInt32 writeFlags = writeInArgs->writeInFlags;
	rtr_msgb_t	*ripcBuffer = (rtr_msgb_t*)(rsslBufImpl->bufferInfo);

	RsslSocketChannel *rsslSocketChannel = (RsslSocketChannel*)rsslChnlImpl->transportInfo;
	rwsSession_t* wsSess = 0;

	if (IPC_NULL_PTR(rsslSocketChannel, "rsslWebSocketWrite", "rsslSocketChannel", error))
		return RSSL_RET_FAILURE;

	wsSess = (rwsSession_t*)rsslSocketChannel->rwsSession;

	if (IPC_NULL_PTR(wsSess, "rsslWebSocketWrite", "wsSess", error))
		return RSSL_RET_FAILURE;

	_DEBUG_TRACE_WS_WRITE("fd "SOCKET_PRINT_TYPE" bImp len %d pOfset %d totLn %u\n",
							rsslSocketChannel->stream, rsslBufImpl->buffer.length, 
							rsslBufImpl->packingOffset, rsslBufImpl->totalLength)

	/* Always set the HIGH flush priority */
	rsslBufImpl->priority = RSSL_HIGH_PRIORITY;

	/* check if we are doing fragmentation */
	if (ripcBuffer && (!(rsslBufImpl->fragmentationFlag)) && (rsslBufImpl->writeCursor == 0))
	{
		/* no fragmentation */
		/* packed case */
		if (rsslBufImpl->packingOffset > 1)
		{
			/* if the length is zero, then there is no message at the end. */
			/* We can take out the space we allocated for its length */
			if (rsslBufImpl->buffer.length == 0)
			{
				rsslBufImpl->packingOffset -= 1;
			}
			else
			{
				ripcBuffer->buffer[rsslBufImpl->packingOffset-1] = ',';
				rsslBufImpl->packingOffset += rsslBufImpl->buffer.length;
			}
			ripcBuffer->length = rsslBufImpl->packingOffset;		/* the packing offset is the entire length of everything in the buffer we want to send */
		}
		else
			ripcBuffer->length = rsslBufImpl->buffer.length + rsslBufImpl->packingOffset;
		
		ripcBuffer->buffer[ripcBuffer->length] = ']';
		ripcBuffer->length += 1;

		ripcBuffer->priority = rsslBufImpl->priority;

		if ((rsslChnlImpl->debugFlags & RSSL_DEBUG_RSSL_DUMP_OUT) && (ripcBuffer->length))
		{
			rsslWebSocketDumpOutFuncImpl(__FUNCTION__, ripcBuffer->buffer, (RsslUInt32)ripcBuffer->length,
				rsslChnlImpl->Channel.socketId, &rsslChnlImpl->Channel);
		}

		retVal = rwsWriteWebSocket(rsslSocketChannel, rsslBufImpl, writeFlags, 
								(RsslInt32*)&outBytes, (RsslInt32*)&uncompOutBytes, 
								(writeFlags & RSSL_WRITE_DIRECT_SOCKET_WRITE) != 0, error);

		totalOutBytes += outBytes;
		outBytes = 0;
		totalUncompOutBytes += uncompOutBytes;
		uncompOutBytes = 0;
	}
	else
	{
		/* fragmentation */
		RsslUInt32 tempSize = 0;
		RsslUInt32 copyUserMsgSize = 0;
		RsslBool firstFragment = rsslBufImpl->fragmentationFlag == BUFFER_IMPL_FIRST_FRAG_HEADER ? 1 : 0;
		char* pBuffer = 0;
		char* pDataBuffer = 0;

		/* chained message - this means that I allocated the memory.
		   I have to break it into its respective ripc messages,
		   set the sizes accordingly, and then free the memory on a successful write */

		/* Compressed the entire message when writting the first fragementation message. */
		if (wsSess->comp.outCompression == RSSL_COMP_ZLIB && wsSess->deflate)
		{
			if (firstFragment && (rsslBufImpl->memoryAllocationOffset != 0))
			{
				ripcCompBuffer	compBuf;
				int compressedBufferLength = rsslBufImpl->buffer.length + 256; /* Added additional bytes to avoid unnessary memory reallocation.*/
				int retVal;
				int compressedBytesOut = 0;
				rsslBufImpl->compressedBuffer.data = (char*)_rsslMalloc(compressedBufferLength);

				if (rsslBufImpl->compressedBuffer.data == NULL)
				{
					_rsslSetError(error, &rsslChnlImpl->Channel, RSSL_RET_BUFFER_NO_BUFFERS, 0);
					snprintf(error->text, MAX_RSSL_ERROR_TEXT,
						"<%s:%d> Error: 0016 rsslWebSocketWrite() Cannot allocate memory of size %d for compressed buffer.\n", __FILE__, __LINE__, compressedBufferLength);
					return RSSL_RET_FAILURE;
				}

				rsslBufImpl->compressedBuffer.length = (RsslUInt32)compressedBufferLength;

				rsslBufImpl->buffer.data[rsslBufImpl->buffer.length] = ']'; /* To account for ']' at the end */
				rsslBufImpl->buffer.length += 2;

				rsslBufImpl->buffer.data -= rsslBufImpl->memoryAllocationOffset;
				rsslBufImpl->memoryAllocationOffset = 0;

				compBuf.next_in = rsslBufImpl->buffer.data;
				compBuf.avail_in = (RsslUInt32)rsslBufImpl->buffer.length;
				compBuf.next_out = rsslBufImpl->compressedBuffer.data;
				compBuf.avail_out = (unsigned long)(rsslBufImpl->compressedBuffer.length);

				if ((retVal = (*(wsSess->comp.outCompFuncs->compress)) (wsSess->comp.c_stream_out, &compBuf,
					(wsSess->comp.flags & RWS_COMPF_DEFLATE_NO_OUTBOUND_CONTEXT) ? 1 : 0,
					error)) < 0)
				{
					_rsslSetError(error, NULL, RSSL_RET_FAILURE, 0);
					snprintf((error->text), MAX_RSSL_ERROR_TEXT,
						"<%s:%d> Compression failed for outbound WS payload",
						__FUNCTION__, __LINE__);

					_rsslFree(rsslBufImpl->compressedBuffer.data);
					rsslBufImpl->compressedBuffer.data = 0;

					return RSSL_RET_FAILURE;
				}

				totalUncompOutBytes = compBuf.bytes_in_used;
				compressedBytesOut += compBuf.bytes_out_used;

				if (retVal > 0 && compBuf.avail_out == 0)
				{
					char* pTempBuffer = NULL;
					RsslUInt32 newSize = 0;
					RsslUInt32 previousSize = rsslBufImpl->compressedBuffer.length;
					do
					{
						newSize = rsslBufImpl->compressedBuffer.length * 2;
						if ((pTempBuffer = increaseSizeAndRealloc(rsslBufImpl->compressedBuffer.data, rsslBufImpl->compressedBuffer.length,
							(size_t)newSize, error)) == 0)
						{
							_rsslFree(rsslBufImpl->compressedBuffer.data);
							rsslBufImpl->compressedBuffer.data = 0;

							return RSSL_RET_FAILURE;
						}
						else
						{
							rsslBufImpl->compressedBuffer.data = pTempBuffer;
							rsslBufImpl->compressedBuffer.length = newSize;
						}

						compBuf.next_out = rsslBufImpl->compressedBuffer.data + previousSize;
						compBuf.avail_out = (unsigned long)(newSize - previousSize);

						if ((*(wsSess->comp.outCompFuncs->compress)) (wsSess->comp.c_stream_out, &compBuf,
							0, /* This buffer is for holding the remainder of the compressed message. Don't reset context. */
							error) < 0 ||
							compBuf.avail_out == 0)
						{
							_rsslFree(rsslBufImpl->compressedBuffer.data);
							rsslBufImpl->compressedBuffer.data = 0;

							_rsslSetError(error, NULL, RSSL_RET_FAILURE, 0);
							snprintf((error->text), MAX_RSSL_ERROR_TEXT,
								"<%s:%d> Compression failed for outbound WS payload", __FUNCTION__, __LINE__);
							return RSSL_RET_FAILURE;
						}
					} while (retVal > 0 && compBuf.avail_out == 0);

					totalUncompOutBytes += compBuf.bytes_in_used;
					compressedBytesOut += compBuf.bytes_out_used;
				}

				rsslBufImpl->compressedBuffer.length = compressedBytesOut;
				rsslBufImpl->compressedBuffer.length -= 4;
			}
			else
			{
				/* Sets to the orignal buffer length for the WRITE_CALL_AGAIN case. */
				totalUncompOutBytes = rsslBufImpl->buffer.length;
			}

			tempSize = rsslBufImpl->compressedBuffer.length - rsslBufImpl->writeCursor;
			pDataBuffer = rsslBufImpl->compressedBuffer.data;
		}
		else
		{
			if (firstFragment && (rsslBufImpl->memoryAllocationOffset != 0))
			{
				rsslBufImpl->buffer.data[rsslBufImpl->buffer.length] = ']'; /* To account for ']' at the end */
				rsslBufImpl->buffer.length += 2;

				rsslBufImpl->buffer.data -= rsslBufImpl->memoryAllocationOffset;
				rsslBufImpl->memoryAllocationOffset = 0;
			}

			tempSize = rsslBufImpl->buffer.length - rsslBufImpl->writeCursor;
			pDataBuffer = rsslBufImpl->buffer.data;
		}

		/* This while loop support the WRITE_CALL_AGAIN case as well*/
		while (tempSize > 0)
		{
			if (ripcBuffer)
			{
				if (firstFragment)
				{
					firstFragment = 0;
					pBuffer = ripcBuffer->buffer;
					copyUserMsgSize = rsslChnlImpl->maxMsgSize;
				}
				else
				{
					pBuffer = ripcBuffer->buffer;
					rsslBufImpl->fragmentationFlag = BUFFER_IMPL_SUBSEQ_FRAG_HEADER;
					copyUserMsgSize = rsslChnlImpl->maxMsgSize;
				}

				if ( tempSize >= copyUserMsgSize)
				{
					/* use rsslChnlImpl->maxMsgSize here - because of tunneling, server side ripcBuffer->maxLen can have
						some additional bytes available there that we shouldnt be using */
					MemCopyByInt(pBuffer, pDataBuffer + rsslBufImpl->writeCursor, copyUserMsgSize);
					tempSize -= copyUserMsgSize;
					rsslBufImpl->writeCursor += copyUserMsgSize;
					ripcBuffer->length += copyUserMsgSize;
					ripcBuffer->priority = rsslBufImpl->priority;

					if (tempSize == 0)
					{
						if (rsslBufImpl->fragmentationFlag == BUFFER_IMPL_FIRST_FRAG_HEADER)
						{
							rsslBufImpl->fragmentationFlag = BUFFER_IMPL_ONLY_ONE_FRAG_MSG;
						}
						else
						{
							rsslBufImpl->fragmentationFlag = BUFFER_IMPL_LAST_FRAG_HEADER;
						}
					}
				}
				else
				{
					copyUserMsgSize = tempSize;

					/* this will all actually fit in one message */
					MemCopyByInt(pBuffer, pDataBuffer + rsslBufImpl->writeCursor, copyUserMsgSize);
					rsslBufImpl->writeCursor += copyUserMsgSize;
					ripcBuffer->length = copyUserMsgSize;
					ripcBuffer->priority = rsslBufImpl->priority;
					tempSize -= copyUserMsgSize;
						
					if (rsslBufImpl->fragmentationFlag == BUFFER_IMPL_FIRST_FRAG_HEADER)
					{
						rsslBufImpl->fragmentationFlag = BUFFER_IMPL_ONLY_ONE_FRAG_MSG;
					}
					else
					{
						rsslBufImpl->fragmentationFlag = BUFFER_IMPL_LAST_FRAG_HEADER;
					}
				}

				if ((rsslChnlImpl->debugFlags & RSSL_DEBUG_RSSL_DUMP_OUT) && (ripcBuffer->length))
				{
					rsslWebSocketDumpOutFuncImpl(__FUNCTION__, ripcBuffer->buffer, (RsslUInt32)ripcBuffer->length,
						rsslChnlImpl->Channel.socketId, &rsslChnlImpl->Channel);
				}

				retVal = rwsWriteWebSocket(rsslSocketChannel, rsslBufImpl, writeFlags, (RsslInt32*)&outBytes, (RsslInt32*)&uncompOutBytes, (writeFlags & RSSL_WRITE_DIRECT_SOCKET_WRITE) != 0, error);

				/* Ensure to move on to next message after writing the first message. */
				if (rsslBufImpl->fragmentationFlag == BUFFER_IMPL_FIRST_FRAG_HEADER)
				{
					rsslBufImpl->fragmentationFlag = BUFFER_IMPL_SUBSEQ_FRAG_HEADER;
				}

				totalOutBytes += outBytes;
				outBytes = 0;
				totalUncompOutBytes += uncompOutBytes;
				uncompOutBytes = 0;

				if (tempSize == 0 || retVal == RSSL_RET_FAILURE)
					break;
			}

			copyUserMsgSize = rsslChnlImpl->maxMsgSize;

			/* Created an individual subsequent fragmented buffer */
			ripcBuffer = (void*)rwsDataBuffer(rsslSocketChannel, tempSize >= copyUserMsgSize ? copyUserMsgSize : tempSize , error);
			if (ripcBuffer == NULL)
			{
				/* call flush and try again, then return error */
				rsslFlush(&rsslChnlImpl->Channel, error);

				ripcBuffer = (void*)rwsDataBuffer(rsslSocketChannel, tempSize >= copyUserMsgSize ? copyUserMsgSize : tempSize, error);
				if (ripcBuffer == NULL)
				{
					/* return error here */
					error->channel = &rsslChnlImpl->Channel;
					rsslBufImpl->bufferInfo = NULL; /* Set to NULL to get a new buffer for write call again */

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
			if (rsslBufImpl->memoryAllocationOffset != 0)
			{
				rsslBufImpl->buffer.data -= rsslBufImpl->memoryAllocationOffset;
				rsslBufImpl->memoryAllocationOffset = 0;
			}
			_rsslFree(rsslBufImpl->buffer.data);
			rsslBufImpl->buffer.length = 0;
			_rsslFree(rsslBufImpl->compressedBuffer.data);
			rsslBufImpl->compressedBuffer.length = 0;
			ripcBuffer->priority = rsslBufImpl->priority;
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
RSSL_RSSL_SOCKET_IMPL_FAST(rsslBufferImpl*) rsslWebSocketGetBuffer(rsslChannelImpl *rsslChnlImpl, RsslUInt32 size, RsslBool packedBuffer, RsslError *error)
{
	rtr_msgb_t *ipcBuf=0;
	rsslBufferImpl *rsslBufImpl=0;
	RsslInt32 maxOutputMsgs;
	RsslCompTypes compression;
	RsslUInt8	fragmentation = 0;
	RsslSocketChannel*	rsslSocketChannel = (RsslSocketChannel*)rsslChnlImpl->transportInfo;
	rwsSession_t* wsSess = (rwsSession_t*)rsslSocketChannel->rwsSession;

	wsSess = (rwsSession_t*)rsslSocketChannel->rwsSession;

	if (IPC_NULL_PTR(wsSess, "rsslWebSocketGetBuffer", "wsSess", error))
		return NULL;

	_DEBUG_TRACE_BUFFER("size %d packedBuf %s max %u\n", size, (packedBuffer?"true":"false"), rsslChnlImpl->maxMsgSize)

	if (size >= UINT32_MAX - 2) {
		_rsslSetError(error, &rsslChnlImpl->Channel, RSSL_RET_FAILURE, 0);
		snprintf(error->text, MAX_RSSL_ERROR_TEXT, 
				"<%s:%d> rsslWebSocketGetBuffer() Error: 0010 Invalid buffer size specified.\n", 
					__FILE__, __LINE__);
		return NULL;
	}

	if (size <= rsslChnlImpl->maxMsgSize)
	{
		size += 2; // Make sure there is room for the opening and closing brackets
		// Since this is a JSON2 message it does not matter if it is packed or a single
		//  large message,  In both cases there will be at least 2 bytes added to the payload
		//  to enclose the JSON message(s) with '[' ']'
		//  Any additional packed messages with add a byte for each packed message from adding
		//  ',' to delineate the packed messages into a JSON array format
		//  [<message_1>,<message_2>,<message_3>]

		ipcBuf = (rtr_msgb_t *)rwsDataBuffer(rsslSocketChannel, size, error);
	}
	else
	{
		/* must be fragmented case */
		RsslInt32 usedBuf;

		IPC_MUTEX_LOCK(rsslSocketChannel);

		/* Return the number of guaranteed buffers used + pool buffers used */
		usedBuf = rsslSocketChannel->guarBufPool->numRegBufsUsed + rsslSocketChannel->guarBufPool->numPoolBufs;
		maxOutputMsgs = rsslSocketChannel->guarBufPool->maxPoolBufs + rsslSocketChannel->guarBufPool->bufpool.maxBufs;
		compression = rsslSocketChannel->outCompression;

		IPC_MUTEX_UNLOCK(rsslSocketChannel);

		/* fragmentation */
		/* the top DataBuffer statement is for cases where our maxGuarMsgs is larger - the reason we need to
		do this check is for compression.  We need to keep a few guaranteed buffers available in the case
		that we compress - ripc needs to be able to get a new buffer to compress into. */

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

			ipcBuf = (rtr_msgb_t *)rwsDataBuffer(rsslSocketChannel, rsslChnlImpl->maxMsgSize, error);
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
				"<%s:%d> Error: 0016 rsslWebSocketGetBuffer() Cannot allocate memory of size %d for buffer.\n", __FILE__, __LINE__, size);

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

		rsslBufImpl->buffer.data = (char*)_rsslMalloc(size + 9);

		if (rsslBufImpl->buffer.data == NULL)
		{
			_rsslSetError(error, &rsslChnlImpl->Channel, RSSL_RET_BUFFER_NO_BUFFERS, 0);
			snprintf(error->text, MAX_RSSL_ERROR_TEXT,
				"<%s:%d> Error: 0016 rsslSocketGetBuffer() Cannot allocate memory of size %d for buffer.\n", __FILE__, __LINE__, size);
			/* return ripc buffer */
			ipcReleaseDataBuffer(rsslSocketChannel, ipcBuf, error);

			return NULL;
		}

		rsslBufImpl->buffer.data[0] = '[';
		rsslBufImpl->memoryAllocationOffset = 1;
		rsslBufImpl->buffer.data += rsslBufImpl->memoryAllocationOffset;

		/* set me as owner */
		rsslBufImpl->owner = 1;
		rsslBufImpl->buffer.length = size;
	}
	else
	{
		_rsslBufferMap(rsslBufImpl, ipcBuf);

		rsslBufImpl->buffer.data[rsslBufImpl->packingOffset] = '[';

		/* Regardless if packing or not, the JSON data message is
		 * enclosed with brackets and rsslBufImpl->packingOffset will be used
		 * to account for the data message offset */
		rsslBufImpl->packingOffset += 1;
		rsslBufImpl->buffer.data = rsslBufImpl->buffer.data + rsslBufImpl->packingOffset;

		// Subtract one for the opening bracket here
		rsslBufImpl->buffer.length = size - 2;
	}

	_DEBUG_TRACE_BUFFER(" bImp  len %d pOfset %d totLn %u\n",
			rsslBufImpl->buffer.length, rsslBufImpl->packingOffset, rsslBufImpl->totalLength)

	rsslBufImpl->totalLength = size;
	
	return rsslBufImpl;
}

/* rssl Socket PackBuffer */
RSSL_RSSL_SOCKET_IMPL_FAST(RsslBuffer*) rsslWebSocketPackBuffer(rsslChannelImpl *rsslChnlImpl, rsslBufferImpl *rsslBufImpl, RsslError *error)
{
	RsslUInt16 bufLength = rsslBufImpl->buffer.length;
	rtr_msgb_t	*ripcBuffer = rsslBufImpl->bufferInfo;
	RsslSocketChannel *rsslSocketChannel = (RsslSocketChannel*)rsslChnlImpl->transportInfo;

	if (IPC_NULL_PTR(rsslSocketChannel, "rsslWebSocketPackBuffer", "rsslSocketChannel", error))
		return 0;

	_DEBUG_TRACE_BUFFER("bImp  len %d pOff %d totLn %u\n", 
			rsslBufImpl->buffer.length, rsslBufImpl->packingOffset, rsslBufImpl->totalLength)

	/* make sure there is room for another message in the buffer plus the ',' */
	if ((rsslBufImpl->packingOffset + rsslBufImpl->buffer.length + 1) < rsslBufImpl->totalLength)
	{
		/* buf = "[<data blk>"
		 *  pktOf--^   Here is no need to add a ',' with first data block packed */
		if (rsslBufImpl->packingOffset > 1)
			ripcBuffer->buffer[rsslBufImpl->packingOffset-1] = ',';

		/* buf = "[<data blk>.."
		 *  pktOf--^  -->   --^ Move ->packingOffset 1 past the position for
		 *                        the next separator ','  or ending ']' */
		/* This will move ->packingOffset to where the next message would start */
		rsslBufImpl->packingOffset += rsslBufImpl->buffer.length + 1;

		rsslBufImpl->buffer.data = ripcBuffer->buffer + rsslBufImpl->packingOffset;
		rsslBufImpl->buffer.length = rsslBufImpl->totalLength - rsslBufImpl->packingOffset;
	}
	else	/* There isn't enough room left to store another message (and its length) */
	{
		rsslBufImpl->buffer.length = 0;		/* Tell them there is no room left */
		rsslBufImpl->buffer.data = 0;		/* set this to zero. The alternative 
											 * (which we do not like) is to point 
											 * beyond the buffer */
	}

	return (&(rsslBufImpl->buffer));
}

/* rssl Socket Ping */
RSSL_RSSL_SOCKET_IMPL_FAST(RsslRet) rsslWebSocketPing(rsslChannelImpl *rsslChnlImpl, RsslError *error)
{
	RsslRet retVal;
	RsslBuffer pingBuffer = {17, (char*)"[{\"Type\":\"Ping\"}]"};

	_DEBUG_TRACE_WS_WRITE("Here \n")

	/* no buffer passed in */
	/* use ripcWrtHeader - should write and flush */
	retVal = rwsSendPingData(((RsslSocketChannel*)rsslChnlImpl->transportInfo), &pingBuffer, error);
	if (retVal < RSSL_RET_SUCCESS)
	{
		error->channel = &rsslChnlImpl->Channel;
	}

	return retVal;
}

RsslRet rsslWebSocketCloseChannel(rsslChannelImpl* rsslChnlImpl, RsslError *error)
{
	RsslSocketChannel *rsslSocketChannel = (RsslSocketChannel*)rsslChnlImpl->transportInfo;

	if (IPC_NULL_PTR(rsslSocketChannel, "rsslWebSocketCloseChannel", "rsslSocketChannel", error))
		return 0;

	/* Send WebSocket close frame before closing the socket channel only when the channel is still active */
	if (rsslChnlImpl->Channel.state == RSSL_CH_STATE_ACTIVE)
	{
		rwsSendWsClose(rsslSocketChannel, RWS_CFSC_NORMAL_CLOSE, error);
	}

	return rsslSocketCloseChannel(rsslChnlImpl, error);
}

void rsslReleaseWebSocketServer(void *server)
{
	rwsServer_t *wsServ;

	if (server)
	{
		 wsServ = (rwsServer_t*)server;
		rwsReleaseServer(wsServ);
		_rsslFree(server);
	}
}

void rsslReleaseWebSocketSession(void *session)
{
	rwsSession_t *wsSess;

	if (session)
	{
		wsSess = (rwsSession_t*)session;
		rwsReleaseSession(wsSess);
		_rsslFree(session);
	}
}

void rsslWebSocketDumpInFuncImpl(const char* functionName, char* buffer, RsslUInt32 length, RsslSocket socketId, RsslChannel* pChannel)
{
	if (rsslWebSocketDumpInFunc)
	{
		(*(rsslWebSocketDumpInFunc))(functionName, buffer, length, socketId);
	}
	if (rsslWebSocketDumpFuncs[pChannel->protocolType].dumpRsslIn)
	{
		(*(rsslWebSocketDumpFuncs[pChannel->protocolType].dumpRsslIn))(functionName, buffer, length, pChannel);
	}
}

void rsslWebSocketDumpOutFuncImpl(const char* functionName, char* buffer, RsslUInt32 length, RsslSocket socketId, RsslChannel* pChannel)
{
	if (rsslWebSocketDumpOutFunc)
	{
		(*(rsslWebSocketDumpOutFunc))(functionName, buffer, length, socketId);
	}
	if (rsslWebSocketDumpFuncs[pChannel->protocolType].dumpRsslOut)
	{
		(*(rsslWebSocketDumpFuncs[pChannel->protocolType].dumpRsslOut))(functionName, buffer, length, pChannel);
	}
}

/* Sets Socket debug dump functions */
RsslRet rsslSetWebSocketDebugFunctions(
	void(*dumpIpcIn)(const char *functionName, char *buffer, RsslUInt32 length, RsslUInt64 opaque),
	void(*dumpIpcOut)(const char *functionName, char *buffer, RsslUInt32 length, RsslUInt64 opaque),
	void(*dumpRsslIn)(const char *functionName, char *buffer, RsslUInt32 length, RsslSocket socketId),
	void(*dumpRsslOut)(const char *functionName, char *buffer, RsslUInt32 length, RsslSocket socketId),
	RsslError *error)
{
	RsslRet retVal = RSSL_RET_SUCCESS;
	RsslInt32	tempRetVal = 0;

	if ((dumpRsslIn && rsslWebSocketDumpInFunc) || (dumpRsslOut && rsslWebSocketDumpOutFunc))
	{
		/* set error message */
		_rsslSetError(error, NULL, RSSL_RET_FAILURE, 0);
		snprintf(error->text, MAX_RSSL_ERROR_TEXT, "<%s:%d> rsslSetWebSocketDebugFunctions() Cannot set socket Rssl dump functions.\n", __FILE__, __LINE__);

		retVal = RSSL_RET_FAILURE;
	}
	else
	{
		rsslWebSocketDumpInFunc = dumpRsslIn;
		rsslWebSocketDumpOutFunc = dumpRsslOut;
		retVal = RSSL_RET_SUCCESS;
	}

	tempRetVal = rwsDbgFuncs(dumpIpcIn, dumpIpcOut);

	if ((tempRetVal < RSSL_RET_SUCCESS) && (retVal < RSSL_RET_SUCCESS))
	{
		/* set error message */
		_rsslSetError(error, NULL, RSSL_RET_FAILURE, 0);
		snprintf(error->text, MAX_RSSL_ERROR_TEXT, "<%s:%d> rsslSetWebSocketDebugFunctions() Cannot set socket Rssl and WebSocket dump functions.\n", __FILE__, __LINE__);

		retVal = RSSL_RET_FAILURE;
	}
	else if (tempRetVal < RSSL_RET_SUCCESS)
	{
		/* set error message */
		_rsslSetError(error, NULL, RSSL_RET_FAILURE, 0);
		snprintf(error->text, MAX_RSSL_ERROR_TEXT, "<%s:%d> rsslSetWebSocketDebugFunctions() Cannot set socket IPC WebSocket functions.\n", __FILE__, __LINE__);

		retVal = RSSL_RET_FAILURE;
	}

	return retVal;
}

RsslRet rsslSetWebSocketDebugFunctionsEx(RsslDebugFunctionsExOpts* pOpts, RsslError* error)
{
	RsslRet retVal = RSSL_RET_SUCCESS;
	RsslInt32	tempRetVal = 0;

	if ((pOpts->dumpRsslIn && rsslWebSocketDumpFuncs[pOpts->protocolType].dumpRsslIn)
		|| (pOpts->dumpRsslOut && rsslWebSocketDumpFuncs[pOpts->protocolType].dumpRsslOut))
	{
		/* set error message */
		_rsslSetError(error, NULL, RSSL_RET_FAILURE, 0);
		snprintf(error->text, MAX_RSSL_ERROR_TEXT, "<%s:%d> rsslSetWebSocketDebugFunctionsEx() Cannot set socket Rssl dump functions.\n", __FILE__, __LINE__);

		retVal = RSSL_RET_FAILURE;
	}
	else
	{
		rsslWebSocketDumpFuncs[pOpts->protocolType].dumpRsslIn = pOpts->dumpRsslIn;
		rsslWebSocketDumpFuncs[pOpts->protocolType].dumpRsslOut = pOpts->dumpRsslOut;
		retVal = RSSL_RET_SUCCESS;
	}

	tempRetVal = rwsDbgFuncsEx(pOpts);

	if ((tempRetVal < RSSL_RET_SUCCESS) && (retVal < RSSL_RET_SUCCESS))
	{
		/* set error message */
		_rsslSetError(error, NULL, RSSL_RET_FAILURE, 0);
		snprintf(error->text, MAX_RSSL_ERROR_TEXT, "<%s:%d> rsslSetWebSocketDebugFunctionsEx() Cannot set socket Rssl and WebSocket dump functions.\n", __FILE__, __LINE__);

		retVal = RSSL_RET_FAILURE;
	}
	else if (tempRetVal < RSSL_RET_SUCCESS)
	{
		/* set error message */
		_rsslSetError(error, NULL, RSSL_RET_FAILURE, 0);
		snprintf(error->text, MAX_RSSL_ERROR_TEXT, "<%s:%d> rsslSetWebSocketDebugFunctionsEx() Cannot set socket IPC WebSocket functions.\n", __FILE__, __LINE__);

		retVal = RSSL_RET_FAILURE;
	}

	return retVal;
}
