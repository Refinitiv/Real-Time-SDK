/*
 * This source code is provided under the Apache 2.0 license and is provided
 * AS IS with no warranty or guarantee of fit for purpose.  See the project's 
 * LICENSE.md for details. 
 * Copyright Thomson Reuters 2015. All rights reserved.
*/

#include "rtr/bigBufferPool.h"
#include "rtr/rsslRetCodes.h"
#include <stdlib.h>
#include <assert.h>

static TunnelBufferImpl* createBigBuffer(RsslUInt bufferSize, RsslUInt userSize, RsslUInt8 poolIndex);

void bigBufferPoolInit(BigBufferPool *pBigBufferPool, RsslUInt fragmentSize, RsslUInt numBuffers)
{
	int i;

	pBigBufferPool->_maxPool = 0;
	pBigBufferPool->_currentNumBuffers = 0;

    // this pool should be created after the tunnel stream fragment size is known
    pBigBufferPool->_fragmentSize = fragmentSize;
    pBigBufferPool->_maxSize = fragmentSize * 2;
	pBigBufferPool->_maxNumBuffers = numBuffers;

	for (i = 0; i < NUM_POOLS; i++)
	{
		rsslInitQueue(&pBigBufferPool->_pools[i]);
	}
}

PoolBuffer* bigBufferPoolGet(BigBufferPool *pBigBufferPool, RsslUInt32 size, RsslErrorInfo *pErrorInfo)
{
	TunnelBufferImpl *pBuffer = NULL;
	RsslUInt poolSize = 0;
	RsslUInt8 poolIndex = 0;

	if (pBigBufferPool->_currentNumBuffers < pBigBufferPool->_maxNumBuffers)
	{
		pBigBufferPool->_currentNumBuffers++;
		// determine which pool to use
		poolSize = pBigBufferPool->_fragmentSize * 2;
		while (size > poolSize)
		{
			poolSize = poolSize * 2;
			poolIndex++;
		}

		if (poolSize > pBigBufferPool->_maxSize)
		{
			pBigBufferPool->_maxSize = poolSize;
			pBigBufferPool->_maxPool = poolIndex;
			// create a buffer of this size
			pBuffer = createBigBuffer(poolSize, size, poolIndex);
		}
		else // The size is smaller then max, so traverse through pools to find available buffer
		{
			RsslUInt8 i;
			for (i = poolIndex; i <= pBigBufferPool->_maxPool; i++)
			{
				RsslQueueLink *pQueueLink;
				
				if ((pQueueLink = rsslQueuePeekFront(&pBigBufferPool->_pools[i])) != NULL)
				{
					pBuffer = RSSL_QUEUE_LINK_TO_OBJECT(TunnelBufferImpl, _tbpLink, pQueueLink);

					// reset big buffer
					pBuffer->_poolBuffer.buffer.data = pBuffer->_startPos;
    				pBuffer->_poolBuffer.buffer.length = size;
					pBuffer->_fragmentationInProgress = RSSL_FALSE;
    				pBuffer->_totalMsgLength = 0;
    				pBuffer->_bytesRemainingToSend = 0;
    				pBuffer->_lastFragmentId = 0;
    				pBuffer->_messageId = 0;
    				pBuffer->_containerType = 0;

					rsslQueueRemoveLink(&pBigBufferPool->_pools[i], pQueueLink);

					break;
				}
			}
		}

		// There was no available buffer, create new
		if (pBuffer == NULL)
		{
			pBuffer = createBigBuffer(poolSize, size, poolIndex);
			if (pBuffer == NULL)
			{
				rsslSetErrorInfo(pErrorInfo, RSSL_EIC_FAILURE, 
					RSSL_RET_FAILURE, __FILE__, __LINE__, "Unable to allocate big buffer for tunnel stream.");
			}
		}
	}
	else // max number of big buffers reached, return NULL buffer
	{
		rsslSetErrorInfo(pErrorInfo, RSSL_EIC_FAILURE, 
			RSSL_RET_FAILURE, __FILE__, __LINE__, "Max number of big buffers limit reached");
	}

	return &pBuffer->_poolBuffer;
}

static TunnelBufferImpl* createBigBuffer(RsslUInt bufferSize, RsslUInt userSize, RsslUInt8 poolIndex)
{
	TunnelBufferImpl *pTSBuffer = (TunnelBufferImpl *)malloc(sizeof(TunnelBufferImpl));

	if (pTSBuffer)
	{
		tunnelBufferImplClear(pTSBuffer);
		pTSBuffer->_isBigBuffer = RSSL_TRUE;
		pTSBuffer->_poolBuffer.buffer.data = (char *)malloc(bufferSize);
		pTSBuffer->_poolBuffer.buffer.length = (RsslUInt32)userSize;
		pTSBuffer->_startPos = pTSBuffer->_poolBuffer.buffer.data;
		pTSBuffer->_bigBufferPoolIndex = poolIndex;

		if (!pTSBuffer->_poolBuffer.buffer.data)
		{
			free(pTSBuffer);
			pTSBuffer = NULL;
		}
	}

	return pTSBuffer;
}

void bigBufferPoolRelease(BigBufferPool *pBigBufferPool, PoolBuffer *pPoolBuffer)
{
	TunnelBufferImpl *pTSBuffer = (TunnelBufferImpl *)pPoolBuffer;

	// return to pool
	RsslUInt8 poolIndex = pTSBuffer->_bigBufferPoolIndex;

	rsslQueueAddLinkToBack(&pBigBufferPool->_pools[poolIndex], &pTSBuffer->_tbpLink);

	pBigBufferPool->_currentNumBuffers--;
}

void bigBufferPoolCleanup(BigBufferPool *pBigBufferPool)
{
	RsslUInt8 i;
	RsslUInt32 count = 0;
	pBigBufferPool->_maxSize = 0;
	pBigBufferPool->_maxPool = 0;
	pBigBufferPool->_fragmentSize = 0;
	pBigBufferPool->_maxNumBuffers = 0;
	pBigBufferPool->_currentNumBuffers = 0;
	for (i = 0; i < NUM_POOLS; i++)
	{
		RsslQueueLink *pQueueLink;
		while((pQueueLink = rsslQueueRemoveFirstLink(&pBigBufferPool->_pools[i])) != NULL)
		{
			TunnelBufferImpl *pBufferImpl = RSSL_QUEUE_LINK_TO_OBJECT(TunnelBufferImpl, _tbpLink, pQueueLink);
			if (pBufferImpl != NULL)
			{
				free(pBufferImpl->_startPos);
				free(pBufferImpl);
			}
		}
	}
}
