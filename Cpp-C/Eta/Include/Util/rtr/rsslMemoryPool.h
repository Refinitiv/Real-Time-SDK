/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2015,2019,2024 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

#ifndef RSSL_MEMORY_POOL_H
#define RSSL_MEMORY_POOL_H

#include "rtr/rsslQueue.h"
#include "rtr/rsslErrorInfo.h"
#include <assert.h>
#include <malloc.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Creates a pool of fixed-size memory blocks. */

typedef struct
{
	int			blockSize;
	RsslQueue	blocks;
} RsslMemoryPool;

typedef struct
{
	RsslQueueLink qlPool;
} RsslMemoryBlock;

/* Initializes a pool. */
RTR_C_INLINE RsslRet rsslMemoryPoolInit(RsslMemoryPool *pPool, int blockSize, int blockCount,
		RsslErrorInfo *pErrorInfo);

/* Cleans up a pool. */
RTR_C_INLINE void rsslMemoryPoolCleanup(RsslMemoryPool *pPool);

/* Retrieves a memory block from the pool. */
RTR_C_INLINE void *rsslMemoryPoolGet(RsslMemoryPool *pPool, RsslErrorInfo *pErrorInfo);

/* Returns a memory block to the pool. */
RTR_C_INLINE void rsslMemoryPoolPut(RsslMemoryPool *pPool, void *pMemory);

RTR_C_INLINE RsslRet rsslMemoryPoolInit(RsslMemoryPool *pPool, int blockSize, int blockCount,
		RsslErrorInfo *pErrorInfo)
{
	int i;

	assert(blockSize > sizeof(RsslMemoryBlock));

	rsslInitQueue(&pPool->blocks);
	pPool->blockSize = blockSize;

	for(i = 0; i < blockCount; ++i)
	{
		RsslMemoryBlock *pBlock = (RsslMemoryBlock*)malloc(blockSize);

		if (!pBlock)
		{
			rsslSetErrorInfo(pErrorInfo, RSSL_EIC_FAILURE, RSSL_RET_FAILURE, __FILE__, __LINE__, 
					"Memory allocation failure.");
			rsslMemoryPoolCleanup(pPool);
			return RSSL_RET_FAILURE;
		}

		rsslQueueAddLinkToBack(&pPool->blocks, &pBlock->qlPool);
	}

	return RSSL_RET_SUCCESS;
}

RTR_C_INLINE void rsslMemoryPoolCleanup(RsslMemoryPool *pPool)
{
	RsslQueueLink *pLink;
	while (pLink = rsslQueueRemoveFirstLink(&pPool->blocks))
		free(RSSL_QUEUE_LINK_TO_OBJECT(RsslMemoryBlock, qlPool, pLink));
}

RTR_C_INLINE void *rsslMemoryPoolGet(RsslMemoryPool *pPool, RsslErrorInfo *pErrorInfo)
{
	RsslQueueLink *pLink;

	if (pLink = rsslQueueRemoveFirstLink(&pPool->blocks))
		return (void*)RSSL_QUEUE_LINK_TO_OBJECT(RsslMemoryBlock, qlPool, pLink);
	else
	{
		void *pBlock = (void*)malloc(pPool->blockSize);

		if (!pBlock)
		{
			rsslSetErrorInfo(pErrorInfo, RSSL_EIC_FAILURE, RSSL_RET_FAILURE, __FILE__, __LINE__, 
					"Memory allocation failure.");
			return NULL;
		}

		return pBlock;
	}
}

RTR_C_INLINE void rsslMemoryPoolPut(RsslMemoryPool *pPool, void *pMemory)
{
	rsslQueueAddLinkToBack(&pPool->blocks, &((RsslMemoryBlock*)pMemory)->qlPool);
}

#ifdef __cplusplus
}
#endif

#endif
