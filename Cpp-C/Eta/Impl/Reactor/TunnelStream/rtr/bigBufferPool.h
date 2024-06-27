/*
 * This source code is provided under the Apache 2.0 license and is provided
 * AS IS with no warranty or guarantee of fit for purpose.  See the project's 
 * LICENSE.md for details. 
 * Copyright (C) 2019 LSEG. All rights reserved.
*/

#ifndef BIG_BUFFER_POOL_H
#define BIG_BUFFER_POOL_H

#include "rtr/rsslQueue.h"
#include "rtr/rsslTypes.h"
#include "rtr/rsslErrorInfo.h"
#include "rtr/tunnelStreamImpl.h"

/* Initializes the big buffer pool. */
void bigBufferPoolInit(BigBufferPool *pBigBufferPool, RsslUInt fragmentSize, RsslUInt numBuffers);

/* Gets a big buffer from the pool. */
PoolBuffer* bigBufferPoolGet(BigBufferPool *pBigBufferPool, RsslUInt32 size, RsslErrorInfo *pErrorInfo);

/* Releases big buffer memory to the pool. */
void bigBufferPoolRelease(BigBufferPool *pBigBufferPool, PoolBuffer *pPoolBuffer);

/* Retrieves a number of buffers in use. */
RsslUInt bigBufferPoolGetUsed(BigBufferPool* pBigBufferPool);

/* Cleans up the big buffer pool. */
void bigBufferPoolCleanup(BigBufferPool *pBigBufferPool);

#endif
