/*
 * This source code is provided under the Apache 2.0 license and is provided
 * AS IS with no warranty or guarantee of fit for purpose.  See the project's 
 * LICENSE.md for details. 
 * Copyright (C) 2019 LSEG. All rights reserved.
*/

#ifndef BUFFER_POOL_H
#define BUFFER_POOL_H

#include "rtr/rsslQueue.h"
#include "rtr/rsslTypes.h"
#include "rtr/rsslErrorInfo.h"

typedef struct SliceableBuffer SliceableBuffer;

typedef struct
{
	RsslQueue			_bufferPool;
	RsslUInt32			_maxFragmentSize;
	RsslQueue			_appBuffers;
	RsslQueue			_intBuffers;
	RsslUInt32			_appBufferLimit;
} BufferPool;

typedef struct
{
	RsslBuffer			buffer;					/* Must be first member. */
	RsslQueueLink		_qLink;
	RsslUInt32			_maxLength;
	SliceableBuffer*	_pSliceableBuffer;
} PoolBuffer;

/* Initializes the buffer pool. */
RsslRet initBufferPool(BufferPool *pBufferPool,
		RsslUInt32 bufferCount,
		RsslUInt32 maxFragmentSize,
		RsslUInt32 appBufferLimit,
		RsslErrorInfo *pErrorInfo);

/* Retrieves a buffer from the pool.
 * Set isAppBuffer to true if the buffer is requested by the application. */
RsslRet bufferPoolGet(BufferPool *pBufferPool, PoolBuffer *pBuffer, RsslUInt32 length,
		RsslBool isAppBuffer, RsslBool isTranslationBuffer, RsslErrorInfo *pErrorInfo);

/* Trims any unused length from the buffer */
void bufferPoolTrimUnusedLength(BufferPool *pBufferPool, PoolBuffer *pBuffer);

/* Releases RsslBuffer memory to the buffer pool. */
void bufferPoolRelease(BufferPool *pBufferPool, PoolBuffer *pBuffer);

/* Retrieves a number of buffers in use */
RsslUInt bufferPoolGetUsed(BufferPool* pBufferPool);

/* Cleans up a buffer pool */
void bufferPoolCleanup(BufferPool *pBufferPool);

#endif
