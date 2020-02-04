/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright Thomson Reuters 2019. All rights reserved.            --
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
// TODO static void(*rsslSocketDumpInFunc)(const char *functionName, char *buffer, RsslUInt32 length, RsslSocket socketId) = 0;
// TODO static void(*rsslSocketDumpOutFunc)(const char *functionName, char *buffer, RsslUInt32 length, RsslSocket socketId) = 0;

// TODO void(*ripcDumpInFunc)(const char *functionName, char *buf, RsslUInt32 len, RsslUInt64 opaque) = 0;
// TODO void(*ripcDumpOutFunc)(const char *functionName, char *buf, RsslUInt32 len, RsslUInt64 opaque) = 0;

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
			// TODO if ((rsslChnlImpl->debugFlags & RSSL_DEBUG_RSSL_DUMP_IN) && (rsslChnlImpl->returnBuffer.length ))
			// TODO (*(rsslSocketDumpInFunc))(__FUNCTION__, rsslChnlImpl->returnBuffer.data, rsslChnlImpl->returnBuffer.length, rsslChnlImpl->Channel.socketId);

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
				// TODO if ((rsslChnlImpl->debugFlags & RSSL_DEBUG_RSSL_DUMP_IN) && (rsslChnlImpl->returnBuffer.length))
				// TODO (*(rsslSocketDumpInFunc))(__FUNCTION__, rsslChnlImpl->returnBuffer.data, rsslChnlImpl->returnBuffer.length, rsslChnlImpl->Channel.socketId);

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

	if (IPC_NULL_PTR(rsslSocketChannel, "rsslWebSocketWrite", "rsslSocketChannel", error))
		return RSSL_RET_FAILURE;

	_DEBUG_TRACE_WS_WRITE("fd "SOCKET_PRINT_TYPE" bImp len %d pOfset %d totLn %u\n",
							rsslSocketChannel->stream, rsslBufImpl->buffer.length, 
							rsslBufImpl->packingOffset, rsslBufImpl->totalLength)

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

		/* chained message - this means that I allocated the memory.
		   I have to break it into its respective ripc messages,
		   set the sizes accordingly, and then free the memory on a successful write */

		tempSize = rsslBufImpl->buffer.length - rsslBufImpl->writeCursor;

		/* This while loop support the WRITE_CALL_AGAIN case as well*/
		while (tempSize > 0)
		{
			if (ripcBuffer)
			{
				if (firstFragment)
				{
					firstFragment = 0;
					ripcBuffer->buffer[0] = '[';
					pBuffer = ripcBuffer->buffer + 1;
					ripcBuffer->length = 1;
					copyUserMsgSize = rsslChnlImpl->maxMsgSize - 1; /* To account for '[' at the beginning */
				}
				else
				{
					pBuffer = ripcBuffer->buffer;
					rsslBufImpl->fragmentationFlag = BUFFER_IMPL_SUBSEQ_FRAG_HEADER;
					copyUserMsgSize = rsslChnlImpl->maxMsgSize - 1; /* To account for ']' at the end */
				}

				if ( tempSize >= copyUserMsgSize)
				{
					/* use rsslChnlImpl->maxMsgSize here - because of tunneling, server side ripcBuffer->maxLen can have
						some additional bytes available there that we shouldnt be using */
					MemCopyByInt(pBuffer, rsslBufImpl->buffer.data + rsslBufImpl->writeCursor, copyUserMsgSize);
					tempSize -= copyUserMsgSize;
					rsslBufImpl->writeCursor += copyUserMsgSize;
					ripcBuffer->length += copyUserMsgSize;
					ripcBuffer->priority = rsslBufImpl->priority;

					if (tempSize == 0)
					{
						rsslBufImpl->fragmentationFlag = BUFFER_IMPL_LAST_FRAG_HEADER;
						pBuffer[copyUserMsgSize] = ']';
						ripcBuffer->length += 1;
					}
				}
				else
				{
					copyUserMsgSize = tempSize;
					/* this will all actually fit in one message */
					MemCopyByInt(pBuffer, rsslBufImpl->buffer.data + rsslBufImpl->writeCursor, tempSize);
					rsslBufImpl->writeCursor += tempSize;
					ripcBuffer->length = tempSize;
					ripcBuffer->priority = rsslBufImpl->priority;
					tempSize -= tempSize;
						
					if (tempSize == 0)
					{
						rsslBufImpl->fragmentationFlag = BUFFER_IMPL_LAST_FRAG_HEADER;
						pBuffer[copyUserMsgSize] = ']';
						ripcBuffer->length += 1;
					}
				}

				retVal = rwsWriteWebSocket(rsslSocketChannel, rsslBufImpl, writeFlags, (RsslInt32*)&outBytes, (RsslInt32*)&uncompOutBytes, (writeFlags & RSSL_WRITE_DIRECT_SOCKET_WRITE) != 0, error);

				totalOutBytes += outBytes;
				outBytes = 0;
				totalUncompOutBytes += uncompOutBytes;
				uncompOutBytes = 0;

				if (tempSize == 0 || retVal == RSSL_RET_FAILURE)
					break;
			}

			copyUserMsgSize = rsslChnlImpl->maxMsgSize;

			/* Created an individual subsequent fragmented buffer */
			ripcBuffer = (void*)rwsDataBuffer(rsslSocketChannel, tempSize >= copyUserMsgSize ? copyUserMsgSize : (tempSize + 1) , error);
			if (ripcBuffer == NULL)
			{
				/* call flush and try again, then return error */
				rsslFlush(&rsslChnlImpl->Channel, error);

				ripcBuffer = (void*)rwsDataBuffer(rsslSocketChannel, tempSize >= copyUserMsgSize ? copyUserMsgSize : (tempSize + 1), error);
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
			_rsslFree(rsslBufImpl->pOriginMem);
			rsslBufImpl->buffer.length = 0;
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
		rsslBufImpl->pOriginMem = rsslBufImpl->buffer.data;
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

RSSL_RSSL_SOCKET_IMPL_FAST(RsslRet) rsslWebSocketCloseChannel(rsslChannelImpl* rsslChnlImpl, RsslError *error)
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


