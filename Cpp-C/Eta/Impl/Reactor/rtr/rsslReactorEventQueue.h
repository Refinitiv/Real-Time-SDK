/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2015,2018-2020,2024 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

#ifndef _RTR_RSSL_EVENT_QUEUE_H
#define _RTR_RSSL_EVENT_QUEUE_H

#include "rtr/rsslReactorEventsImpl.h"
#include "rtr/rsslRDMMsg.h"
#include "rtr/rsslEventSignal.h"
#include "rtr/rsslThread.h"

#include <stdlib.h>

#ifdef WIN32
#include <stdio.h>
#include <io.h>
#include <fcntl.h>
#include <process.h>
#include <math.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif

/* Event Queue */

typedef struct _RsslReactorEventQueueGroup RsslReactorEventQueueGroup;

/* RsslReactorEventQueue
 * Queue of RsslReactorEvents */
typedef struct
{
	RsslQueue eventPool;
	RsslQueue eventQueue;
	RsslMutex eventPoolLock;
	RsslMutex eventQueueLock;
	RsslReactorEventImpl *pLastEvent;

	RsslReactorEventQueueGroup *pParentGroup;
	RsslQueueLink readyEventQueueLink;
	RsslBool isInActiveEventQueueGroup;
} RsslReactorEventQueue;

/* RsslReactorEventQueueGroup
 * Maintains a group of event queues that are "active" (they have an event that can be dispatched).
 * If at least one queue is in the group, the file descriptor for the group(eventSignal) will be triggered. */
struct _RsslReactorEventQueueGroup
{
	RsslQueue readyEventQueueGroup;
	RsslMutex lock;
	RsslEventSignal eventSignal;
};

RTR_C_INLINE RsslRet rsslInitReactorEventQueueGroup(RsslReactorEventQueueGroup *pList)
{
	if (!rsslInitEventSignal(&pList->eventSignal))
		return RSSL_RET_FAILURE;

	RSSL_MUTEX_INIT(&pList->lock);

	rsslInitQueue(&pList->readyEventQueueGroup);

	return RSSL_RET_SUCCESS;
}

/* rsslReactorEventQueueGroupShift
 * "Rotates" the queues in the group, by taking the first queue, moving it to the back, and returning it. */
RTR_C_INLINE RsslReactorEventQueue* rsslReactorEventQueueGroupShift(RsslReactorEventQueueGroup *pQueueList)
{
	RsslQueueLink *pLink;
	RSSL_MUTEX_LOCK(&pQueueList->lock);

	pLink = rsslQueueRemoveFirstLink(&pQueueList->readyEventQueueGroup);
	if (pLink) rsslQueueAddLinkToBack(&pQueueList->readyEventQueueGroup, pLink);
	RSSL_MUTEX_UNLOCK(&pQueueList->lock);

	return pLink ? RSSL_QUEUE_LINK_TO_OBJECT(RsslReactorEventQueue, readyEventQueueLink, pLink) : NULL;
}

RTR_C_INLINE int rsslGetEventQueueGroupSignalFD(RsslReactorEventQueueGroup *pQueueList)
{
	return rsslGetEventSignalFD(&pQueueList->eventSignal);
}


/* Removes the event queue from its parent's event queue list if appropriate.
 * Resets the event queue list's signal if appropriate. */
/* Ownership of pQueue->eventQueueLock is assumed */
RTR_C_INLINE RsslRet rsslReactorEventQueueSetInactive(RsslReactorEventQueue *pQueue)
{
	RsslUInt32 count;

	RSSL_MUTEX_LOCK(&pQueue->pParentGroup->lock);

	if (!pQueue->isInActiveEventQueueGroup) return (RSSL_MUTEX_UNLOCK(&pQueue->pParentGroup->lock), RSSL_RET_SUCCESS);

	/* Add to parent list of active queues */
	rsslQueueRemoveLink(&pQueue->pParentGroup->readyEventQueueGroup, &pQueue->readyEventQueueLink);
	pQueue->isInActiveEventQueueGroup = RSSL_FALSE;

	count = rsslQueueGetElementCount(&pQueue->pParentGroup->readyEventQueueGroup);
	if (count == 0)
	{
		/* List is now empty; need to reset queue list descriptor */
		int ret;
		ret = rsslResetEventSignal(&pQueue->pParentGroup->eventSignal); /* Read pointer of event */

		if (ret < 0)
			return (RSSL_MUTEX_UNLOCK(&pQueue->pParentGroup->lock), RSSL_RET_FAILURE);
	}

	RSSL_MUTEX_UNLOCK(&pQueue->pParentGroup->lock);
	
	return RSSL_RET_SUCCESS;
}

/* rsslCleanupReactorEventQueueGroup 
 * Cleans up an RsslReactorEventQueueGroup. */
RTR_C_INLINE RsslRet rsslCleanupReactorEventQueueGroup(RsslReactorEventQueueGroup *pList)
{

	rsslCleanupEventSignal(&pList->eventSignal);

	RSSL_MUTEX_DESTROY(&pList->lock);

	return RSSL_RET_SUCCESS;
}

/* rsslInitReactorEventQueue 
 * Initializes an RsslReactorEventQueue.
 */
RTR_C_INLINE RsslRet rsslInitReactorEventQueue(RsslReactorEventQueue *pQueue, int poolSize, RsslReactorEventQueueGroup *pParentGroup)
{
	int i;

	if (!pParentGroup) return RSSL_RET_INVALID_ARGUMENT;

	memset(pQueue, 0, sizeof(RsslReactorEventQueue));

	RSSL_MUTEX_INIT(&pQueue->eventPoolLock);
	RSSL_MUTEX_INIT(&pQueue->eventQueueLock);

	rsslInitQueue(&pQueue->eventPool);
	rsslInitQueue(&pQueue->eventQueue);

	pQueue->pParentGroup = pParentGroup;

	for (i = 0; i < poolSize; ++i)
	{
		RsslReactorEventImpl *pNewEvent = (RsslReactorEventImpl*)malloc(sizeof(RsslReactorEventImpl));
		if (pNewEvent)
		{
			rsslClearReactorEventImpl(pNewEvent);
			rsslInitQueueLink(&pNewEvent->base.eventQueueLink);
			rsslQueueAddLinkToBack(&pQueue->eventPool, &pNewEvent->base.eventQueueLink);
		}
	}

	return RSSL_RET_SUCCESS;
}

/* rsslCleanupReactorEventQueue
 * Cleans up an RsslReactorEventQueue */
RTR_C_INLINE RsslRet rsslCleanupReactorEventQueue(RsslReactorEventQueue *pQueue)
{
	RsslQueueLink *pLink;
	RsslReactorEventImpl *pEvent;

	while ((pLink = rsslQueueRemoveFirstLink(&pQueue->eventQueue)))
	{
		pEvent = RSSL_QUEUE_LINK_TO_OBJECT(RsslReactorEventImpl, base.eventQueueLink, pLink);
		free(pEvent);
	}

	while ((pLink = rsslQueueRemoveFirstLink(&pQueue->eventPool)))
	{
		pEvent = RSSL_QUEUE_LINK_TO_OBJECT(RsslReactorEventImpl, base.eventQueueLink, pLink);
		free(pEvent);
	}

	if (pQueue->pLastEvent)
	{
		free(pQueue->pLastEvent);
		pQueue->pLastEvent = 0;
	}

	RSSL_MUTEX_DESTROY(&pQueue->eventPoolLock);
	RSSL_MUTEX_DESTROY(&pQueue->eventQueueLock);

	return RSSL_RET_SUCCESS;
}

RTR_C_INLINE RsslReactorEventImpl *rsslReactorEventQueueGetFromPool(RsslReactorEventQueue *pQueue)
{
	RsslReactorEventImpl *pEvent;
	RsslQueueLink *pLink;

	RSSL_MUTEX_LOCK(&pQueue->eventPoolLock);
	if ( (pLink = rsslQueueRemoveFirstLink(&pQueue->eventPool)))
		pEvent = RSSL_QUEUE_LINK_TO_OBJECT(RsslReactorEventImpl, base.eventQueueLink, pLink);
	else
	{
		pEvent = (RsslReactorEventImpl*)malloc(sizeof(RsslReactorEventImpl));
		if (pEvent)
		{
			rsslClearReactorEventImpl(pEvent);
			rsslInitQueueLink(&pEvent->base.eventQueueLink);
		}
	}
	RSSL_MUTEX_UNLOCK(&pQueue->eventPoolLock);

	return pEvent;
}


/* This should not be run if the event has alredy been placed into an event queue. */
RTR_C_INLINE void rsslReactorEventQueueReturnToPool(RsslReactorEventImpl *pEvent, RsslReactorEventQueue *pQueue, RsslInt32 poolSize)
{
	RSSL_MUTEX_LOCK(&pQueue->eventPoolLock);

	if (poolSize == -1 || (RsslInt32)pQueue->eventPool.count < poolSize)
	{
		rsslQueueAddLinkToBack(&pQueue->eventPool, &pEvent->base.eventQueueLink);
	}
	else 
	{
		free(pEvent);
	}

	RSSL_MUTEX_UNLOCK(&pQueue->eventPoolLock);
}

RTR_C_INLINE RsslRet rsslReactorEventQueuePut(RsslReactorEventQueue *pQueue, RsslReactorEventImpl *pEvent)
{
	RsslUInt32 count;

	RSSL_MUTEX_LOCK(&pQueue->eventQueueLock);

	rsslQueueAddLinkToBack(&pQueue->eventQueue, &pEvent->base.eventQueueLink);

	count = rsslQueueGetElementCount(&pQueue->eventQueue);
	if (count == 1)
	{
		/* May need to trigger parent EventQueueGroup */
		RsslUInt32 count;

		RSSL_MUTEX_LOCK(&pQueue->pParentGroup->lock);

		if (pQueue->isInActiveEventQueueGroup) return (RSSL_MUTEX_UNLOCK(&pQueue->pParentGroup->lock), RSSL_MUTEX_UNLOCK(&pQueue->eventQueueLock), RSSL_RET_SUCCESS);

		/* Add to parent list of active queues */
		rsslQueueAddLinkToBack(&pQueue->pParentGroup->readyEventQueueGroup, &pQueue->readyEventQueueLink);
		pQueue->isInActiveEventQueueGroup = RSSL_TRUE;

		count = rsslQueueGetElementCount(&pQueue->pParentGroup->readyEventQueueGroup);
		if (count == 1)
		{
			/* List was previously empty; Need to trigger queue list descriptor */
			int ret;

			ret = rsslSetEventSignal(&pQueue->pParentGroup->eventSignal); /* Read pointer of event */

			if (ret < 0)
			{
				RSSL_MUTEX_UNLOCK(&pQueue->pParentGroup->lock);
				RSSL_MUTEX_UNLOCK(&pQueue->eventQueueLock);
				return RSSL_RET_FAILURE;
			}

		}

		RSSL_MUTEX_UNLOCK(&pQueue->pParentGroup->lock);
	}

	RSSL_MUTEX_UNLOCK(&pQueue->eventQueueLock);

	return RSSL_RET_SUCCESS;
}

RTR_C_INLINE RsslReactorEventImpl* rsslReactorEventQueueGet(RsslReactorEventQueue *pQueue, RsslInt32 poolSize, RsslRet *pRet)
{
	RsslRet count;
	RsslReactorEventImpl *pEvent;
	RsslQueueLink *pLink;

	if (pQueue->pLastEvent)
	{
		/* Return previous event to pool */

		rsslReactorEventQueueReturnToPool(pQueue->pLastEvent, pQueue, poolSize);

		pQueue->pLastEvent = 0;
	}

	RSSL_MUTEX_LOCK(&pQueue->eventQueueLock);

	count = rsslQueueGetElementCount(&pQueue->eventQueue);

	if(count)
	{
		pLink = rsslQueueRemoveFirstLink(&pQueue->eventQueue);
		pEvent = RSSL_QUEUE_LINK_TO_OBJECT(RsslReactorEventImpl, base.eventQueueLink, pLink);
		if (count == 1)
		{
			/* May need to reset parent EventQueueGroup. */

			RSSL_MUTEX_LOCK(&pQueue->pParentGroup->lock);

			if (pQueue->isInActiveEventQueueGroup) 
			{
				RsslUInt32 parentQueueCount;

				/* Remove from parent list of active queues */
				rsslQueueRemoveLink(&pQueue->pParentGroup->readyEventQueueGroup, &pQueue->readyEventQueueLink);
				pQueue->isInActiveEventQueueGroup = RSSL_FALSE;

				parentQueueCount = rsslQueueGetElementCount(&pQueue->pParentGroup->readyEventQueueGroup);
				if (parentQueueCount == 0)
				{
					/* List is now empty; need to reset queue list descriptor */
					int ret;
					ret = rsslResetEventSignal(&pQueue->pParentGroup->eventSignal); /* Read pointer of event */

					if (ret < 0)
					{
						RSSL_MUTEX_UNLOCK(&pQueue->pParentGroup->lock);
						RSSL_MUTEX_UNLOCK(&pQueue->eventQueueLock);
						*pRet = RSSL_RET_FAILURE;
						return NULL;
					}
				}
			}

			RSSL_MUTEX_UNLOCK(&pQueue->pParentGroup->lock);
		}
		--count;
	}
	else
		pEvent = NULL;

	RSSL_MUTEX_UNLOCK(&pQueue->eventQueueLock);


	pQueue->pLastEvent = pEvent;
	*pRet = count;
	return pEvent;
}

#ifdef __cplusplus
};
#endif

#endif
