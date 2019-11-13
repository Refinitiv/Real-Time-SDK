/*
 * This source code is provided under the Apache 2.0 license and is provided
 * AS IS with no warranty or guarantee of fit for purpose.  See the project's 
 * LICENSE.md for details. 
 * Copyright (C) 2019 Refinitiv. All rights reserved.
*/

/* rotatingQueue.h
 * A queue that allows for iterating over the queue in a circular fashion. */

#ifndef _ROTATING_QUEUE_H
#define _ROTATING_QUEUE_H

#include "rtr/rsslQueue.h"

#include <assert.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Rotating queue.  
   - Allows easy inserting/removal and round-robin iteration over time.  */ 
typedef struct
{
	RsslQueue		_queue;			/* Internal queue. */
	RsslQueueLink	*_pCurrentLink;	/* Current position in the queue. */
} RotatingQueue;

/* Initializes a RotatingQueue struct. */
RTR_C_INLINE void initRotatingQueue(RotatingQueue *pQueue)
{
	rsslInitQueue(&pQueue->_queue);
	pQueue->_pCurrentLink = NULL;
}

/* Insert a link into the queue. */
RTR_C_INLINE void rotatingQueueInsert(RotatingQueue *pQueue, RsslQueueLink *pLink)
{
	assert(pQueue);
	assert(pLink);

	if (rsslQueueGetElementCount(&pQueue->_queue) == 0)
		pQueue->_pCurrentLink = pLink;

	rsslQueueAddLinkToBack(&pQueue->_queue, pLink);
}


/* Moves to the next link in the queue(moving back to the start when it hits the end). */
RTR_C_INLINE RsslQueueLink *rotatingQueueNext(RotatingQueue *pQueue)
{
	assert(pQueue);

	if (rsslQueueGetElementCount(&pQueue->_queue))
	{
		pQueue->_pCurrentLink = rsslQueuePeekNextCircular(&pQueue->_queue, pQueue->_pCurrentLink);
		return pQueue->_pCurrentLink;
	}
	else
		return NULL;
}

RTR_C_INLINE RsslUInt32 rotatingQueueGetCount(RotatingQueue *pQueue)
{
	assert(pQueue);
	return rsslQueueGetElementCount(&pQueue->_queue);
}

RTR_C_INLINE RsslQueueLink *rotatingQueuePeekFrontAsList(RotatingQueue *pQueue)
{
	assert(pQueue);
	return rsslQueuePeekFront(&pQueue->_queue);
}

/* Moves to next link in the queue(without rotating back to the start, use with rotatingQueueRestart to iterate over the whole queue) */
RTR_C_INLINE RsslQueueLink *rotatingQueuePeekNextAsList(RotatingQueue *pQueue, RsslQueueLink *pLink)
{
	assert(pQueue);
	assert(pLink);

	return rsslQueuePeekNext(&pQueue->_queue, pLink);
}

/* Removes a link from the queue, moving the current link if necessary. */
RTR_C_INLINE void rotatingQueueRemove(RotatingQueue *pQueue, RsslQueueLink *pLink)
{
	RsslQueueLink *pNextLink;
	
	assert(pQueue);
	assert(pLink);
	assert(pQueue->_pCurrentLink);

	pNextLink = rsslQueuePeekNextCircular(&pQueue->_queue, pQueue->_pCurrentLink);

	rsslQueueRemoveLink(&pQueue->_queue, pLink);

	if (rsslQueueGetElementCount(&pQueue->_queue) == 0) /* It was the only item in the queue */
		pQueue->_pCurrentLink = NULL;
	else if (pLink == pQueue->_pCurrentLink) /* Current item is this link, so move */
		pQueue->_pCurrentLink = pNextLink;

}

#ifdef __cplusplus
};
#endif

#endif

