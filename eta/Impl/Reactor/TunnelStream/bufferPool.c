/*
 * This source code is provided under the Apache 2.0 license and is provided
 * AS IS with no warranty or guarantee of fit for purpose.  See the project's 
 * LICENSE.md for details. 
 * Copyright Thomson Reuters 2015. All rights reserved.
*/

#include "rtr/bufferPool.h"
#include "rtr/rsslRetCodes.h"
#include <stdlib.h>
#include <assert.h>

struct SliceableBuffer
{
	RsslQueueLink	_qLink;			/* Pool link */
	int				_bufferCount;	/* Number of RsslBuffers using this fragment. */
	char			*_pData;			/* Start of buffer space. */
	char			*_pCurPos;		/* Current unused buffer space. */
	RsslQueue		*_pParentQueue;
};

static SliceableBuffer* _bufferPoolAddSliceableBuffer(BufferPool *pBufferPool,
		RsslErrorInfo *pErrorInfo)
{
	/* Add 7 in case decoders try to bytewswap it (the leftover bytes will be correctly discarded). */
	SliceableBuffer *pSliceableBuffer = (SliceableBuffer*)malloc(sizeof(SliceableBuffer) + pBufferPool->_maxFragmentSize + 7);

	if (pSliceableBuffer == NULL)
		rsslSetErrorInfo(pErrorInfo, RSSL_EIC_FAILURE, RSSL_RET_FAILURE, 
				__FILE__, __LINE__, "Failed to allocate buffer for pool.");

	memset(pSliceableBuffer, 0, sizeof(SliceableBuffer));
	pSliceableBuffer->_pCurPos = pSliceableBuffer->_pData = (char*)pSliceableBuffer + sizeof(SliceableBuffer);
	return pSliceableBuffer;
}

RsslRet initBufferPool(BufferPool *pBufferPool,
		RsslUInt32 bufferCount,
		RsslUInt32 maxFragmentSize,
		RsslUInt32 appBufferLimit,
		RsslErrorInfo *pErrorInfo)
{
	RsslUInt32 i;

	rsslInitQueue(&pBufferPool->_bufferPool);
	rsslInitQueue(&pBufferPool->_appBuffers);
	rsslInitQueue(&pBufferPool->_intBuffers);

	pBufferPool->_maxFragmentSize = maxFragmentSize;
	pBufferPool->_appBufferLimit = appBufferLimit;

	for(i = 0; i < bufferCount; ++i)
	{
		SliceableBuffer *pSliceableBuffer;
		if ((pSliceableBuffer = _bufferPoolAddSliceableBuffer(pBufferPool, pErrorInfo))
			   	== NULL)
		{
			bufferPoolCleanup(pBufferPool);
			return RSSL_RET_FAILURE;
		}

		rsslQueueAddLinkToBack(&pBufferPool->_bufferPool,
				&pSliceableBuffer->_qLink);
	}

	return RSSL_RET_SUCCESS;
}

RsslRet bufferPoolGet(BufferPool *pBufferPool, PoolBuffer *pBuffer, RsslUInt32 length,
		RsslBool isAppBuffer, RsslBool isTranslationBuffer, RsslErrorInfo *pErrorInfo)
{
	RsslQueueLink *pLink;
	SliceableBuffer *pCurBuffer;
	RsslQueue *pBufferQueue;

	if (length > pBufferPool->_maxFragmentSize)
	{
		rsslSetErrorInfo(pErrorInfo, RSSL_EIC_FAILURE, RSSL_RET_INVALID_ARGUMENT, 
				__FILE__, __LINE__, "Buffer size is too large");
		return RSSL_RET_INVALID_ARGUMENT;
	}

	if (isAppBuffer)
		pBufferQueue = &pBufferPool->_appBuffers;
	else
		pBufferQueue = &pBufferPool->_intBuffers;

	pLink = rsslQueuePeekBack(pBufferQueue);

	if (pLink != NULL)
		pCurBuffer = RSSL_QUEUE_LINK_TO_OBJECT(SliceableBuffer, _qLink, pLink);
	else
		pCurBuffer = NULL;

	if (pCurBuffer == NULL || pBufferPool->_maxFragmentSize - 
			(pCurBuffer->_pCurPos - pCurBuffer->_pData) < length)
	{
		/* No current buffer or buffer does not have room; add another one. */
		if (isAppBuffer && !isTranslationBuffer && rsslQueueGetElementCount(&pBufferPool->_appBuffers)
				>= pBufferPool->_appBufferLimit)
		{
			rsslSetErrorInfo(pErrorInfo, RSSL_EIC_FAILURE, RSSL_RET_FAILURE, 
					__FILE__, __LINE__, "No buffers are available from the buffer pool.");
			return RSSL_RET_FAILURE;
		}

		pLink = rsslQueueRemoveFirstLink(&pBufferPool->_bufferPool);

		if (pLink == NULL)
		{
			if ((pCurBuffer = _bufferPoolAddSliceableBuffer(pBufferPool, pErrorInfo))
					== NULL)
				return RSSL_RET_FAILURE;
		}
		else
			pCurBuffer = RSSL_QUEUE_LINK_TO_OBJECT(SliceableBuffer, _qLink, pLink);

		pCurBuffer->_pCurPos = pCurBuffer->_pData;
		pCurBuffer->_bufferCount = 0;
		pCurBuffer->_pParentQueue = pBufferQueue;
		rsslQueueAddLinkToBack(pBufferQueue, &pCurBuffer->_qLink);
	}

	++pCurBuffer->_bufferCount;
	pBuffer->buffer.data = pCurBuffer->_pCurPos;
	pBuffer->buffer.length = pBuffer->_maxLength = length;
	pBuffer->_pSliceableBuffer = pCurBuffer;
	pCurBuffer->_pCurPos += length;
	
	return RSSL_RET_SUCCESS;
}

void bufferPoolTrimUnusedLength(BufferPool *pBufferPool, PoolBuffer *pBuffer)
{
	SliceableBuffer *pSliceableBuffer = pBuffer->_pSliceableBuffer;
	if (pBuffer->buffer.data + pBuffer->_maxLength == pSliceableBuffer->_pCurPos)
		pSliceableBuffer->_pCurPos = pBuffer->buffer.data + pBuffer->buffer.length;
}

void bufferPoolRelease(BufferPool *pBufferPool, PoolBuffer *pBuffer)
{
	SliceableBuffer *pSliceableBuffer = pBuffer->_pSliceableBuffer;

	pBuffer->_pSliceableBuffer = NULL;
	assert(pSliceableBuffer->_bufferCount > 0);

	--pSliceableBuffer->_bufferCount;
	if (pSliceableBuffer->_bufferCount == 0)
	{
		rsslQueueRemoveLink(pSliceableBuffer->_pParentQueue, &pSliceableBuffer->_qLink);
		rsslQueueAddLinkToBack(&pBufferPool->_bufferPool, &pSliceableBuffer->_qLink);
	}
}

void bufferPoolCleanup(BufferPool *pBufferPool)
{
	RsslQueueLink *pLink;

	while ((pLink = rsslQueueRemoveFirstLink(&pBufferPool->_bufferPool)) 
			!= NULL)
	{
		SliceableBuffer *pSliceableBuffer =
			RSSL_QUEUE_LINK_TO_OBJECT(SliceableBuffer, _qLink, pLink);
		free(pSliceableBuffer);
	}

	while ((pLink = rsslQueueRemoveFirstLink(&pBufferPool->_appBuffers)) 
			!= NULL)
	{
		SliceableBuffer *pSliceableBuffer =
			RSSL_QUEUE_LINK_TO_OBJECT(SliceableBuffer, _qLink, pLink);
		free(pSliceableBuffer);
	}

	while ((pLink = rsslQueueRemoveFirstLink(&pBufferPool->_intBuffers)) 
			!= NULL)
	{
		SliceableBuffer *pSliceableBuffer =
			RSSL_QUEUE_LINK_TO_OBJECT(SliceableBuffer, _qLink, pLink);
		free(pSliceableBuffer);
	}

	pBufferPool->_maxFragmentSize = 0;
}

