/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2015,2018-2019,2024 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

#ifndef _RSSL_QUEUE_H
#define _RSSL_QUEUE_H

#include <stddef.h>
#include "rtr/rsslTypes.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct _RsslQueueLink {
	struct _RsslQueueLink *prev, *next;
} RsslQueueLink;


typedef struct {
	RsslQueueLink head;
	RsslUInt32 count;
	RsslQueueLink *iterNext;
} RsslQueue;

RTR_C_INLINE void rsslInitQueueLink(RsslQueueLink *pLink)
{
	pLink->prev = 0;
	pLink->next = 0;
}

RTR_C_INLINE void rsslInitQueue(RsslQueue *pQueue)
{
	pQueue->head.prev = &pQueue->head;
	pQueue->head.next = &pQueue->head;
	pQueue->count = 0;
	pQueue->iterNext = NULL;
}

RTR_C_INLINE RsslBool rsslQueueLinkInAList(RsslQueueLink *pLink)
{
	return (pLink->prev || pLink->next) ? RSSL_TRUE : RSSL_FALSE;
}

RTR_C_INLINE void rsslQueueAddLinkToBack(RsslQueue *pQueue, RsslQueueLink *pLink)
{
	pLink->next = &pQueue->head;
	pLink->prev = pQueue->head.prev;
	pQueue->head.prev = pLink;
	pLink->prev->next = pLink;
	++pQueue->count;
}

RTR_C_INLINE void rsslQueueAddLinkToFront(RsslQueue *pQueue, RsslQueueLink *pLink)
{
	pLink->prev = &pQueue->head;
	pLink->next = pQueue->head.next;
	pQueue->head.next = pLink;
	pLink->next->prev = pLink;
	++pQueue->count;
}

RTR_C_INLINE void rsslQueueInsertAfter(RsslQueue *pQueue, RsslQueueLink *pCurrentLink,
		RsslQueueLink *pNewLink)
{
	pNewLink->prev = pCurrentLink;
	pNewLink->next = pCurrentLink->next;
	pCurrentLink->next->prev = pNewLink;
	pCurrentLink->next = pNewLink;
	++pQueue->count;
}

RTR_C_INLINE void rsslQueuePrepend(RsslQueue *pQueue, RsslQueue *pSourceQueue)
{
	if (pSourceQueue->head.next == &pSourceQueue->head) return;

	/* Link last element of source queue to first element of this queue */
	pSourceQueue->head.prev->next = pQueue->head.next;
	pQueue->head.next->prev = pSourceQueue->head.prev;

	/* Link last element of source queue to head of this queue */
	pQueue->head.next = pSourceQueue->head.next;
	pSourceQueue->head.next->prev = &pQueue->head;

	pQueue->count += pSourceQueue->count;
	rsslInitQueue(pSourceQueue);
}

RTR_C_INLINE void rsslQueueAppend(RsslQueue *pQueue, RsslQueue *pSourceQueue)
{
	if (pSourceQueue->head.next == &pSourceQueue->head) return;

	/* Link last element of this queue to first element of source queue */
	pQueue->head.prev->next = pSourceQueue->head.next;
	pSourceQueue->head.next->prev = pQueue->head.prev;

	/* Link last element of source queue to end of this queue */
	pQueue->head.prev = pSourceQueue->head.prev;
	pSourceQueue->head.prev->next = &pQueue->head;

	pQueue->count += pSourceQueue->count;
	rsslInitQueue(pSourceQueue);
}

RTR_C_INLINE RsslQueueLink* rsslQueuePeekFront(RsslQueue *pQueue)
{
	return (pQueue->head.next != &pQueue->head) ? pQueue->head.next : NULL;
}

RTR_C_INLINE RsslQueueLink* rsslQueuePeekBack(RsslQueue *pQueue)
{
	return (pQueue->head.prev != &pQueue->head) ? pQueue->head.prev : NULL;
}

RTR_C_INLINE RsslQueueLink* rsslQueuePeekNext(RsslQueue *pQueue, RsslQueueLink *pLink)
{
	return (pLink->next != &pQueue->head) ? pLink->next : NULL;
}

RTR_C_INLINE RsslQueueLink* rsslQueuePeekPrev(RsslQueue *pQueue, RsslQueueLink *pLink)
{
	return (pLink->prev != &pQueue->head) ? pLink->prev : NULL;
}

RTR_C_INLINE void rsslQueueRemoveLink(RsslQueue *pQueue, RsslQueueLink *pLink)
{
	if (pLink == pQueue->iterNext)
		pQueue->iterNext = rsslQueuePeekNext(pQueue, pQueue->iterNext);

	pLink->next->prev = pLink->prev;
	pLink->prev->next = pLink->next;
	pLink->next = 0;
	pLink->prev = 0;
	--pQueue->count;
}

RTR_C_INLINE RsslQueueLink* rsslQueueRemoveFirstLink(RsslQueue *pQueue)
{
	RsslQueueLink *pLink = rsslQueuePeekFront(pQueue);
	if (pLink) rsslQueueRemoveLink(pQueue, pLink);
	return pLink;
}

RTR_C_INLINE RsslQueueLink* rsslQueueRemoveLastLink(RsslQueue *pQueue)
{
	RsslQueueLink *pLink = rsslQueuePeekBack(pQueue);
	if (pLink) rsslQueueRemoveLink(pQueue, pLink);
	return pLink;
}

RTR_C_INLINE RsslQueueLink* rsslQueuePeekNextCircular(RsslQueue *pQueue, RsslQueueLink *pLink)
{
	return (pLink->next != &pQueue->head) ? pLink->next : rsslQueuePeekFront(pQueue);
}

RTR_C_INLINE RsslUInt32 rsslQueueGetElementCount(RsslQueue *pQueue)
{
	return pQueue->count;
}

RTR_C_INLINE RsslQueueLink* rsslQueueStart(RsslQueue *pQueue)
{
	RsslQueueLink *pHead = rsslQueuePeekFront(pQueue);
	if (pHead)
		pQueue->iterNext = rsslQueuePeekNext(pQueue, pHead);
	else
		pQueue->iterNext = NULL;

	return pHead;
}

RTR_C_INLINE RsslQueueLink *rsslQueueForth(RsslQueue *pQueue)
{
	RsslQueueLink *iterNext = pQueue->iterNext;

	if (pQueue->iterNext)
		pQueue->iterNext = rsslQueuePeekNext(pQueue, pQueue->iterNext);

	return iterNext;
}


/*** Queue management helper functions ***/

/* RSSL_QUEUE_LINK_TO_OBJECT
 * Takes a RsslQueueLink pointer from an RsslQueue and returns a pointer to the object that has that link.
 * __objectType -- the type of the object.
 * __link -- The name of the link member on the object.
 * __pLink -- Pointer the RsslQueueLink of the object.
 *
 * Example use:
 * typedef struct 
 * {
 * 	RsslQueueLink queueLink1;
 * } MyObjectType;
 * 
 * RsslQueueLink *pLink;
 * MyObjectType *pMyObject;
 * (...)
 * pMyObject = RSSL_QUEUE_LINK_TO_OBJECT(MyObjectType, queueLink, pLink);
 * 
 */
#define RSSL_QUEUE_LINK_TO_OBJECT(__objectType, __link, __pLink) ((__objectType*)((char*)__pLink - offsetof(__objectType, __link)))

/* RSSL_QUEUE_FOR_EACH_LINK
 * 
 * "for" loop for the RsslQueue.  Designed so that while iterating, the current element can be safely removed.
 * __pQueue - Queue to iterate over.
 * __pTmpLink - Temporary pointer to the next element after the current one. Do not use this inside your loop.
 * __pLink - The current element of the loop iteration.
 *
 * Example use:
 *
 *   RsslQueue myQueue;
 *   RsslQueueLink *pTmpLink, *pLink;
 *   MyElem *pElement;
 *     (...)
 *   RSSL_QUEUE_FOR_EACH_LINK(&myQueue, pTmpLink, pLink)
 *   {
 *      pElement = RSSL_QUEUE_LINK_TO_OBJECT(QueueElem, queueLink, pLink);
 *      if (pElement->needToRemove)
 *         rsslQueueRemoveLink(&myQueue, pLink);
 *   }
 */
#define RSSL_QUEUE_FOR_EACH_LINK(__pQueue, __pLink) \
	for ((__pLink) = rsslQueueStart((__pQueue)); (__pLink) != NULL; (__pLink) = rsslQueueForth((__pQueue)))



#ifdef __cplusplus
}
#endif

#endif
