/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2019 Refinitiv. All rights reserved.            --
 *|-----------------------------------------------------------------------------
 */

#include <stdio.h>
#include <malloc.h>

#include "rtr/cutildfltcbuffer.h"

#if !defined(WIN32) && !defined(__OS2__)
#include <sys/param.h>
#endif

#ifndef PAGESIZE
#define PAGESIZE 4096
#endif

#define SIPC_MEMALIGN 4


#define fromMyPool(dblk,realpool) \
	( ((dblk)->pool == &((realpool)->bufpool)) ? 1 : 0)

#define notFromMyPool(dblk,realpool) \
	( ((dblk)->pool == &((realpool)->bufpool)) ? 1 : 0)


int rtr_cbufferCppOverhead = 0;
void *(*rtr_cbufferCppInit)(rtr_msgb_t*,void*) = 0;


size_t rtr_dfltcAlignBytes( size_t bytes, size_t alignment )
{
	size_t x = bytes + alignment - 1;
	x -= (x % alignment);
	return(x);
}


static int rtr_dfltcRemovePool( rtr_dfltcbufferpool_t *pool )
{
	rtr_datab_t	*dblk;
	rtr_msgb_t	*mblk;
	RsslQueueLink	*pLink;

	while ((pLink = rsslQueueRemoveLastLink(&(pool->freeList))) != 0)
	{
		dblk = RSSL_QUEUE_LINK_TO_OBJECT(rtr_datab_t, link, pLink);
		free(dblk);
	}

	pool->bufpool.numBufs = 0;

#ifdef _DFLTC_BUFFER_DEBUG
	pool->numFreeDblks = 0;
	pool->numFreeMblks = 0;
	pool->numUsedDblks = 0;
	pool->numUsedMblks = 0;
#endif

	if (pool->curDblk)
	{
		if (fromMyPool(pool->curDblk,pool))
			free(pool->curDblk);
		else
		{
			rtrBufferFree(pool->curDblk->pool,(rtr_msgb_t*)pool->curDblk->internal);
			pool->numPoolBufs--;
		}

		pool->curDblk = 0;
	}
	pool->nextChar = 0;

	while ((pLink =rsslQueueRemoveLastLink(&(pool->sharedPoolMblks))) != 0)
	{
		mblk = RSSL_QUEUE_LINK_TO_OBJECT(rtr_msgb_t, link, pLink);
		rtrBufferFree(mblk->pool,mblk);
		pool->numPoolBufs--;
	}

	while ((pLink = rsslQueueRemoveLastLink(&(pool->usedList))) != 0)
	{
		dblk = RSSL_QUEUE_LINK_TO_OBJECT(rtr_datab_t, link, pLink);
		free(dblk);
	}

	/* Allocated message blocks list does not have an associated structure.  The allocated memory block 
	   starts with a RsslQueueLink header. */
	while ((pLink = (RsslQueueLink*)rsslQueueRemoveLastLink(&(pool->allocatedMblks))) != 0)
	{
		free(pLink);
	}

	rsslInitQueue(&(pool->freeMsgList));

	return(0);
}

static int rtr_dfltcRemoveFromPool( rtr_dfltcbufferpool_t *pool, int numBufs )
{
	rtr_datab_t	*dblk;
	RsslQueueLink *pLink;
	while (numBufs-- > 0)
	{
		pLink =rsslQueueRemoveLastLink(&(pool->freeList));
		if (pLink)
		{
			dblk = RSSL_QUEUE_LINK_TO_OBJECT(rtr_datab_t, link, pLink);
#ifdef _DFLTC_BUFFER_DEBUG
			pool->numFreeDblks--;
			pool->numFreeMblks--;
#endif
			free(dblk);
			pool->bufpool.numBufs--;
		}
		else
			return(-1);
	}
	return(0);
}

static int rtr_dfltcIncreasePool( rtr_dfltcbufferpool_t *pool, int bufs )
{
	void		*memory;
	rtr_datab_t	*dblk;
	rtr_msgb_t	*mblk;
	int			numBufs=0;

	while (numBufs < bufs)
	{
		memory = malloc(sizeof(rtr_datab_t) + sizeof(rtr_msgb_t) + rtr_cbufferCppOverhead + pool->bufpool.maxBufSize);
		if (memory)
		{
			dblk = (rtr_datab_t*)memory;
			mblk = (rtr_msgb_t*)((caddr_t)memory + sizeof(rtr_datab_t));

			rsslInitQueueLink(&(dblk->link));
			dblk->base = (caddr_t)((caddr_t)mblk + sizeof(rtr_msgb_t) + rtr_cbufferCppOverhead);
			dblk->length = pool->bufpool.maxBufSize;
			dblk->pool = &(pool->bufpool);
			dblk->numRefs = 0;
			dblk->flags = 0;
			dblk->pad = 0;
			dblk->internal = 0;

			rsslInitQueueLink(&(mblk->link));
			mblk->nextMsg = 0;
			mblk->buffer = 0;
			mblk->length = 0;
			mblk->maxLength = 0;
			mblk->datab = 0;
			mblk->flags = 0;
			mblk->protocol = 0;
			mblk->protocolHdr = 0;
			mblk->protocolHdrLength = 0;
			mblk->fragOffset = 0;
			mblk->priority = 0;
			if (rtr_cbufferCppOverhead && rtr_cbufferCppInit)
			{
				mblk->internal = ((caddr_t)mblk + sizeof(rtr_msgb_t));
				mblk->internal = (void*)(*(rtr_cbufferCppInit))(mblk,mblk->internal);
			}
			else
				mblk->internal = 0;
			mblk->pool = &(pool->bufpool);

			rsslQueueAddLinkToBack(&(pool->freeList),&(dblk->link));

			numBufs++;
#ifdef _DFLTC_BUFFER_DEBUG
			pool->numFreeDblks++;
			pool->numFreeMblks++;
#endif
			pool->bufpool.numBufs++;
		}
		else
		{
			if (numBufs > 0)
				rtr_dfltcRemoveFromPool( pool, numBufs );
			return(-1);
		}
	}
	return(numBufs);
}

static int rtr_dfltcIncreaseMblks( rtr_dfltcbufferpool_t *pool, int bufs )
{
	rtr_msgb_t	*mblk;
	void		*memory;
	void		*memEnd;
	size_t		bytesNeeded = sizeof(RsslQueueLink) + (bufs * (sizeof(rtr_msgb_t) + rtr_cbufferCppOverhead));
	int			numAdded=-1;

	bytesNeeded=rtr_dfltcAlignBytes(bytesNeeded,PAGESIZE);

	if ( bytesNeeded < (sizeof(rtr_msgb_t) + sizeof(RsslQueueLink)) )
		return(-1);

	memory = malloc(bytesNeeded);
	if (memory)
	{
		/* the mblock list is a queue header followed by an rtr_msgb_t structure, then the memory for the block
		   There is no formal structure for this object. The list of allocated blocksed is managed by the 
		   allocatedMblks list. */
		RsslQueueLink	*memlink = (RsslQueueLink*)memory;
		rsslInitQueueLink(memlink);
		rsslQueueAddLinkToBack(&(pool->allocatedMblks),memlink);

		numAdded = 0;
		memEnd = (caddr_t)memory + bytesNeeded;
		memory = (caddr_t)memory + sizeof(RsslQueueLink);

		while ( (void*)((caddr_t)memory + sizeof(rtr_msgb_t) + rtr_cbufferCppOverhead ) < memEnd)
		{
			mblk = (rtr_msgb_t*)memory;
			rsslInitQueueLink(&(mblk->link));
			mblk->nextMsg = 0;
			mblk->buffer = 0;
			mblk->length = 0;
			mblk->maxLength = 0;
			mblk->datab = 0;
			mblk->flags = rtr_dfltcMsgbPutInFreeList;
			mblk->protocol = 0;
			mblk->protocolHdr = 0;
			mblk->protocolHdrLength = 0;
			mblk->fragOffset = 0;
			mblk->priority = 0;
			mblk->pool = &(pool->bufpool);

			if (rtr_cbufferCppOverhead && rtr_cbufferCppInit)
			{
				memory = (caddr_t)memory + sizeof(rtr_msgb_t);
				mblk->internal = (void*)(*(rtr_cbufferCppInit))(mblk,memory);
				memory = (caddr_t)memory + rtr_cbufferCppOverhead;
			}
			else
			{
				memory = (caddr_t)memory + sizeof(rtr_msgb_t);
				mblk->internal = 0;
			}
			rsslQueueAddLinkToBack(&(pool->freeMsgList),&(mblk->link));

			numAdded++;
#ifdef _DFLTC_BUFFER_DEBUG
			pool->numFreeMblks++;
#endif
		}
	}
	return(numAdded);
}

static int rtr_dfltcFreePool( rtr_bufferpool_t *pool )
{
	rtr_dfltcbufferpool_t	*rtr_dfltcpool;

	rtr_dfltcpool = (rtr_dfltcbufferpool_t*)pool->internal;

	if (rtr_dfltcpool->sharedPool)
		rtrBufferPoolDropRef(rtr_dfltcpool->sharedPool);

	rtr_dfltcRemovePool(rtr_dfltcpool);
	free(rtr_dfltcpool);

	return(0);
}

static rtr_msgb_t *getFreeMblk(rtr_dfltcbufferpool_t* rtr_dfltcpool)
{
	rtr_msgb_t	*mblk=0;
	RsslQueueLink *pLink = 0;

	pLink = rsslQueueRemoveFirstLink(&(rtr_dfltcpool->freeMsgList));
	if (pLink == 0)
	{
		/* If empty, allocate more memory and get the first block out of the free message list */
		if (rtr_dfltcIncreaseMblks(rtr_dfltcpool, rtr_dfltcpool->increaseBufRate) == -1)
			return NULL;

		pLink = rsslQueueRemoveFirstLink(&(rtr_dfltcpool->freeMsgList));
	}

	mblk = RSSL_QUEUE_LINK_TO_OBJECT(rtr_msgb_t, link, pLink);
	return(mblk);
}

static rtr_msgb_t *rtr_dfltcIntAllocMsg(rtr_dfltcbufferpool_t* rtr_dfltcpool, size_t size)
{
	rtr_datab_t	*dblk;
	rtr_msgb_t	*mblk=0;
	size_t			bytesLeft;
	int			attempt=0;
	RsslQueueLink *pLink;

	while (mblk == 0)
	{
		/* Check to see if there is a free data segment */
		if ((dblk = rtr_dfltcpool->curDblk) == 0)
		{
			pLink = rsslQueueRemoveFirstLink(&(rtr_dfltcpool->freeList));
			
			if (pLink != 0)
			{
				dblk = RSSL_QUEUE_LINK_TO_OBJECT(rtr_datab_t, link, pLink);
#ifdef _DFLTC_BUFFER_DEBUG
				rtr_dfltcpool->numFreeDblks--;
				rtr_dfltcpool->numUsedDblks++;
#endif
			}
			else if (attempt == 0)
			{
				int increase = rtr_dfltcpool->increaseBufRate;
				/* Attempt to increase the pool size once. */
				attempt++;
				if ((rtr_dfltcpool->bufpool.numBufs + increase) > rtr_dfltcpool->bufpool.maxBufs)
					increase = rtr_dfltcpool->bufpool.maxBufs - rtr_dfltcpool->bufpool.numBufs;

				if (increase > 0)
					rtr_dfltcIncreasePool(rtr_dfltcpool,increase);
				/* We should have allocated the memory here, go back to the begininng of the while loop to pull 
				   the allocated blocks. */
				continue;
			}
			else if ((rtr_dfltcpool->sharedPool) &&
				(rtr_dfltcpool->numPoolBufs < rtr_dfltcpool->maxPoolBufs ))
			{
				/* Pull from the shared pool, if present. */
				mblk = rtrBufferAllocMax(rtr_dfltcpool->sharedPool);
				if (mblk)
				{
					rsslQueueAddLinkToBack(&(rtr_dfltcpool->sharedPoolMblks),&(mblk->link));
					dblk = mblk->datab;
					dblk->internal = mblk;
					rtr_dfltcpool->numPoolBufs++;
					mblk = 0;
				}
				else
					return(0);
			}
			else
				return(0);

			rtr_dfltcpool->curDblk = dblk;
			rtr_dfltcpool->nextChar = dblk->base;
		}

		bytesLeft = dblk->length - (rtr_dfltcpool->nextChar - dblk->base);
		/* Check to see if there's space in the data segment for the requested message size */
		if (size <= bytesLeft)
		{
			mblk = 0;
			if (dblk->numRefs == 0)
			{
				/* The first message in the data block immediately follows the rtr_datab_t structure. */
				mblk = (rtr_msgb_t*)((caddr_t)dblk + sizeof(rtr_datab_t));
			}
			else if ((pLink = rsslQueueRemoveFirstLink(&(rtr_dfltcpool->freeMsgList))) == 0)
			{
				/* There are no available free messages in this data block, so reserve a new message block in the memory pool*/
				rtr_dfltcIncreaseMblks(rtr_dfltcpool,rtr_dfltcpool->increaseBufRate);
				pLink = rsslQueueRemoveFirstLink(&(rtr_dfltcpool->freeMsgList));
				if (pLink == 0)
				{
					/* Allocation failed, so return NULL */
					return 0;
				}
			}
			if(!mblk)
				mblk = RSSL_QUEUE_LINK_TO_OBJECT(rtr_msgb_t, link, pLink);
	
			mblk->nextMsg = 0;
			mblk->buffer = rtr_dfltcpool->nextChar;
			mblk->length = 0;
			mblk->maxLength = size;
			mblk->datab = dblk;
			mblk->protocol = 0;
			mblk->protocolHdr = 0;
			mblk->protocolHdrLength = 0;
			mblk->fragOffset = 0;
			mblk->priority = 0;

			dblk->numRefs++;
			rtr_dfltcpool->nextChar += size;
	
#ifdef _DFLTC_BUFFER_DEBUG
			rtr_dfltcpool->numFreeMblks--;
			rtr_dfltcpool->numUsedMblks++;
#endif

			if (rtr_dfltcpool->numRegBufsUsed == 0)
			{
				rtr_dfltcpool->numRegBufsUsed++;
				if (rtr_dfltcpool->numRegBufsUsed > rtr_dfltcpool->peakNumBufsUsed)
					rtr_dfltcpool->peakNumBufsUsed = rtr_dfltcpool->numRegBufsUsed;
			}

		}
		else
		{
			/* There isn't enough space in the current data block, so add it to the used data block list, and 
			   go back to the top of the loop and attempt to grab a new one from the pool */
			if (fromMyPool(dblk,rtr_dfltcpool))
			{
				rsslQueueAddLinkToBack(&(rtr_dfltcpool->usedList),&(dblk->link));
				rtr_dfltcpool->numRegBufsUsed++;
				if (rtr_dfltcpool->numRegBufsUsed > rtr_dfltcpool->peakNumBufsUsed)
					rtr_dfltcpool->peakNumBufsUsed = rtr_dfltcpool->numRegBufsUsed;
			}
			rtr_dfltcpool->curDblk = 0;
		}
	}
	return(mblk);
}

static int rtr_dfltcIntFreeMsg(rtr_dfltcbufferpool_t *rtr_dfltcpool,rtr_msgb_t *mblk)
{
	rtr_datab_t			*dblk;
	rtr_msgb_t			*nmblk;

	while (mblk)
	{
		dblk = mblk->datab;
		nmblk = mblk->nextMsg;
		if (dblk)
		{
			dblk->numRefs--;

			if (fromMyPool(dblk,rtr_dfltcpool))
			{
				if (dblk->numRefs <= 0)
				{
					if (rtr_dfltcpool->curDblk == dblk)
					{
						rtr_dfltcpool->nextChar = rtr_dfltcpool->curDblk->base;
					}
					else
					{
#ifdef _DFLTC_BUFFER_DEBUG
						rtr_dfltcpool->numFreeDblks++;
						rtr_dfltcpool->numUsedDblks--;
#endif
						dblk->internal = 0;
						rsslQueueRemoveLink(&(rtr_dfltcpool->usedList),&(dblk->link));
						rtr_dfltcpool->numRegBufsUsed--;
						rsslQueueAddLinkToBack(&(rtr_dfltcpool->freeList),&(dblk->link));
					}
				}
			}
			else
			{
				if (dblk->numRefs == 1)
				{
					rtr_msgb_t *shmblk = (rtr_msgb_t*)dblk->internal;
					rsslQueueRemoveLink(&(rtr_dfltcpool->sharedPoolMblks),&(shmblk->link));
					rtr_dfltcpool->numPoolBufs--;
					rtrBufferFree(shmblk->pool,shmblk);
					mblk->datab = 0;

						/* If this is the current data block being
						 * used in this pool then clear this state.
						 */
					if (rtr_dfltcpool->curDblk == dblk)
						rtr_dfltcpool->curDblk = 0;
				}
			}
		}

#ifdef _DFLTC_BUFFER_DEBUG
		rtr_dfltcpool->numFreeMblks++;
		rtr_dfltcpool->numUsedMblks--;
#endif
	
		if (mblk->flags & rtr_dfltcMsgbPutInFreeList) 
		{
			rsslQueueAddLinkToBack(&(rtr_dfltcpool->freeMsgList),&(mblk->link));
		}
		mblk = nmblk;
	}

	return(1);
}

rtr_msgb_t *rtr_dfltcAllocMsg(rtr_bufferpool_t *pool, size_t size)
{
	rtr_dfltcbufferpool_t	*rtr_dfltcpool;
	rtr_msgb_t			*mblk=0;

	RTBUFFERPOOLLOCK(pool);
	rtr_dfltcpool = (rtr_dfltcbufferpool_t*)pool->internal;

	if (size <= pool->maxBufSize)
	{
		mblk = rtr_dfltcIntAllocMsg(rtr_dfltcpool,size);
	}
	else
	{
		size_t			bytesLeft=size;
		size_t			bytesToGet;
		rtr_msgb_t	*curmblk,*nmblk;

		while (bytesLeft > 0)
		{
			if (bytesLeft > pool->maxBufSize)
				bytesToGet = pool->maxBufSize;
			else
				bytesToGet = bytesLeft;

			if ((nmblk = rtr_dfltcIntAllocMsg(rtr_dfltcpool,bytesToGet)) == 0)
			{
				if (mblk != 0)
					rtr_dfltcIntFreeMsg(rtr_dfltcpool,mblk);
				mblk = 0;
				break;
			}

			if (mblk == 0)
				mblk = nmblk;
			else
				curmblk->nextMsg = nmblk;

			curmblk = nmblk;
			bytesLeft -= bytesToGet;
		}
	}
	RTBUFFERPOOLUNLOCK(pool);

	return(mblk);
}
/* This function pulls messages from the shared pool.  */
rtr_msgb_t *rtr_dfltcAllocMaxMsg(rtr_bufferpool_t *pool)
{
	rtr_dfltcbufferpool_t	*rtr_dfltcpool;
	rtr_msgb_t			*mblk=0;
	rtr_datab_t			*dblk=0;
	int					attempt=0;
	RsslQueueLink		*pLink = 0;

	RTBUFFERPOOLLOCK(pool);
	rtr_dfltcpool = (rtr_dfltcbufferpool_t*)pool->internal;

	while (dblk == 0)
	{
		pLink = rsslQueueRemoveFirstLink(&(rtr_dfltcpool->freeList));
		if (pLink != 0)
		{
			dblk = RSSL_QUEUE_LINK_TO_OBJECT(rtr_datab_t, link, pLink);
				/* Put the data block in the used list. */
			rsslQueueAddLinkToBack(&(rtr_dfltcpool->usedList),&(dblk->link));
			rtr_dfltcpool->numRegBufsUsed++;
			if (rtr_dfltcpool->numRegBufsUsed > rtr_dfltcpool->peakNumBufsUsed)
				rtr_dfltcpool->peakNumBufsUsed = rtr_dfltcpool->numRegBufsUsed;

#ifdef _DFLTC_BUFFER_DEBUG
			rtr_dfltcpool->numFreeDblks--;
			rtr_dfltcpool->numUsedDblks++;
#endif
		}
		else if (attempt == 0)
		{
			int increase = rtr_dfltcpool->increaseBufRate;
			attempt++;
			if ((rtr_dfltcpool->bufpool.numBufs + increase) > rtr_dfltcpool->bufpool.maxBufs)
				increase = rtr_dfltcpool->bufpool.maxBufs - rtr_dfltcpool->bufpool.numBufs;

			if (increase > 0)
				rtr_dfltcIncreasePool(rtr_dfltcpool,increase);

			continue;
		}
		else if ((rtr_dfltcpool->sharedPool) &&
			(rtr_dfltcpool->numPoolBufs < rtr_dfltcpool->maxPoolBufs ))
		{
			mblk = rtrBufferAllocMax(rtr_dfltcpool->sharedPool);
			if (mblk)
			{
				rsslQueueAddLinkToBack(&(rtr_dfltcpool->sharedPoolMblks),&(mblk->link));
				dblk = mblk->datab;
				dblk->internal = mblk;
				rtr_dfltcpool->numPoolBufs++;
				mblk = 0;
			}
			else
			{
				RTBUFFERPOOLUNLOCK(pool);
				return(0);
			}
		}
		else
		{
			RTBUFFERPOOLUNLOCK(pool);
			return(0);
		}
	}

	if ((pLink = rsslQueueRemoveLastLink(&(rtr_dfltcpool->freeMsgList))) == 0)
	{
		rtr_dfltcIncreaseMblks(rtr_dfltcpool,rtr_dfltcpool->increaseBufRate);
		pLink = rsslQueueRemoveLastLink(&(rtr_dfltcpool->freeMsgList));
		if (pLink == 0)
		{
			rtr_msgb_t *shmblk = (rtr_msgb_t*)dblk->internal;
			rsslQueueRemoveLink(&(rtr_dfltcpool->sharedPoolMblks),&(shmblk->link));
			rtr_dfltcpool->numPoolBufs--;
			rtrBufferFree(shmblk->pool,shmblk);
			RTBUFFERPOOLUNLOCK(pool);
			return(0);
		}
	}

	mblk = RSSL_QUEUE_LINK_TO_OBJECT(rtr_msgb_t, link, pLink);

	mblk->nextMsg = 0;
	mblk->buffer = dblk->base;
	mblk->length = 0;
	mblk->maxLength = dblk->length;
	mblk->datab = dblk;
	mblk->protocol = 0;
	mblk->protocolHdr = 0;
	mblk->protocolHdrLength = 0;
	mblk->fragOffset = 0;
	mblk->priority = 0;
	dblk->numRefs++;
#ifdef _DFLTC_BUFFER_DEBUG
	rtr_dfltcpool->numFreeMblks--;
	rtr_dfltcpool->numUsedMblks++;
#endif
	RTBUFFERPOOLUNLOCK(pool);
	return(mblk);
}

int rtr_dfltcSetUsed(rtr_bufferpool_t *pool, rtr_msgb_t *curmblk)
{
	rtr_dfltcbufferpool_t	*rtr_dfltcpool;

	RTBUFFERPOOLLOCK(pool);
	rtr_dfltcpool = (rtr_dfltcbufferpool_t*)pool->internal;

	if (rtr_dfltcpool->curDblk)
	{
		while (curmblk)
		{
			if (curmblk->length != curmblk->maxLength)
			{
				if ((curmblk->buffer + curmblk->maxLength) == rtr_dfltcpool->nextChar)
				{
					rtr_dfltcpool->nextChar -= (curmblk->maxLength - curmblk->length);
					curmblk->maxLength = curmblk->length;
				}
			}
			curmblk = curmblk->nextMsg;
		}
	}
	RTBUFFERPOOLUNLOCK(pool);
	return(1);
}

rtr_msgb_t *rtr_dfltcDupMsg(rtr_bufferpool_t *pool, rtr_msgb_t *curmblk)
{
	rtr_dfltcbufferpool_t	*rtr_dfltcpool;
	rtr_msgb_t			*retmblk=0;
	rtr_msgb_t			*newmblk=0;
	rtr_msgb_t			*lastmblk=0;
	RsslQueueLink		*pLink=0;

	RTBUFFERPOOLLOCK(pool);
	rtr_dfltcpool = (rtr_dfltcbufferpool_t*)pool->internal;

	while (curmblk)
	{
		if ((pLink = rsslQueueRemoveFirstLink(&(rtr_dfltcpool->freeMsgList))) == 0)		{
			rtr_dfltcIncreaseMblks(rtr_dfltcpool,rtr_dfltcpool->increaseBufRate);
			pLink = rsslQueueRemoveFirstLink(&(rtr_dfltcpool->freeMsgList));
			if (pLink == 0)
			{
				if (retmblk)
					rtr_dfltcIntFreeMsg(rtr_dfltcpool,retmblk);
				retmblk = 0;
				break;
			}
		}

		newmblk = RSSL_QUEUE_LINK_TO_OBJECT(rtr_msgb_t, link, pLink);

		newmblk->nextMsg = 0;
		newmblk->buffer = curmblk->buffer;
		newmblk->length = curmblk->length;
		newmblk->maxLength = curmblk->maxLength;
		newmblk->datab = curmblk->datab;
		newmblk->protocol = curmblk->protocol;
		newmblk->protocolHdr = curmblk->protocolHdr;
		newmblk->protocolHdrLength = curmblk->protocolHdrLength;
		newmblk->fragOffset = curmblk->fragOffset;
		newmblk->priority = curmblk->priority;
	
		newmblk->datab->numRefs++;

#ifdef _DFLTC_BUFFER_DEBUG
		rtr_dfltcpool->numFreeMblks--;
		rtr_dfltcpool->numUsedMblks++;
#endif

		if (retmblk == 0)
			retmblk = newmblk;
		else
			lastmblk->nextMsg = newmblk;

		lastmblk = newmblk;
		curmblk = curmblk->nextMsg;
	}
	RTBUFFERPOOLUNLOCK(pool);
	return(retmblk);
}

rtr_msgb_t *rtr_dfltcCopyMsg(rtr_bufferpool_t *pool, rtr_msgb_t *curmblk)
{
	rtr_dfltcbufferpool_t	*rtr_dfltcpool;
	rtr_msgb_t			*retmblk=0;
	rtr_msgb_t			*newmblk=0;
	rtr_msgb_t			*lastmblk=0;

	RTBUFFERPOOLLOCK(pool);
	rtr_dfltcpool = (rtr_dfltcbufferpool_t*)pool->internal;

	while (curmblk)
	{
		if ((newmblk = rtr_dfltcIntAllocMsg(rtr_dfltcpool,curmblk->length)) == 0)
		{
			if (retmblk)
				rtr_dfltcIntFreeMsg(rtr_dfltcpool,retmblk);
			retmblk = 0;
			break;
		}

		newmblk->length = curmblk->length;
		memcpy(newmblk->buffer,curmblk->buffer,curmblk->length);

		if (retmblk == 0)
			retmblk = newmblk;
		else
			lastmblk->nextMsg = newmblk;

		lastmblk = newmblk;
		curmblk = curmblk->nextMsg;
	}
	RTBUFFERPOOLUNLOCK(pool);
	return(retmblk);
}

int rtr_dfltcFreeMsg(rtr_msgb_t *mblk)
{
	rtr_dfltcbufferpool_t	*rtr_dfltcpool;
	int					retval;

	RTBUFFERPOOLLOCK(mblk->pool);
	rtr_dfltcpool = (rtr_dfltcbufferpool_t*)mblk->pool->internal;
	retval = rtr_dfltcIntFreeMsg(rtr_dfltcpool,mblk);
	RTBUFFERPOOLUNLOCK(mblk->pool);

	return(retval);
}

int rtr_dfltcSetMaxBufs(rtr_bufferpool_t *pool, int newValue )
{
	rtr_dfltcbufferpool_t	*rtr_dfltcpool;
	int	change;
	int retval = 1;

	RTBUFFERPOOLLOCK(pool);
	rtr_dfltcpool = (rtr_dfltcbufferpool_t*)pool->internal;

	if ( pool->maxBufs < newValue )
	{
		change = newValue - pool->maxBufs;
		if (rtr_dfltcIncreasePool(rtr_dfltcpool,change) < 0)
			retval = -1;
		else
			pool->maxBufs = newValue;
	}
	else
	{
		if (pool->numBufs > pool->maxBufs)
		{
			rtr_dfltcRemoveFromPool(rtr_dfltcpool,(pool->numBufs - pool->maxBufs));
		}
		pool->maxBufs = newValue;
	}
	RTBUFFERPOOLUNLOCK(pool);
	return(retval);
}

int rtr_dfltcSetMaxSharedBufs(rtr_bufferpool_t *pool, int newValue )
{
	rtr_dfltcbufferpool_t	*rtr_dfltcpool;

	RTBUFFERPOOLLOCK(pool);
	rtr_dfltcpool = (rtr_dfltcbufferpool_t*)pool->internal;

	if (newValue >= 0)
		rtr_dfltcpool->maxPoolBufs = newValue;

	RTBUFFERPOOLUNLOCK(pool);

	return(1);
}

int rtr_dfltcResetPeakNumBufs(rtr_bufferpool_t *pool)
{
	rtr_dfltcbufferpool_t	*rtr_dfltcpool;

	RTBUFFERPOOLLOCK(pool);

	rtr_dfltcpool = (rtr_dfltcbufferpool_t*)pool->internal;

	rtr_dfltcpool->peakNumBufsUsed = rtr_dfltcpool->numRegBufsUsed;

	RTBUFFERPOOLUNLOCK(pool);

	return(1);
}

int rtr_countFreeList(rtr_bufferpool_t *pool)
{
	rtr_dfltcbufferpool_t	*rtr_dfltcpool=(rtr_dfltcbufferpool_t*)pool->internal;
	int			cnt=0;

	RTBUFFERPOOLLOCK(pool);
	cnt = rtr_dfltcpool->freeList.count;
	RTBUFFERPOOLUNLOCK(pool);
	return(cnt);
}

int rtr_dfltcAddRef(rtr_bufferpool_t *pool)
{
	int retvalue;

	RTBUFFERPOOLLOCK(pool);
	pool->numRefs++;
	retvalue = pool->numRefs;
	RTBUFFERPOOLUNLOCK(pool);
	return(retvalue);
}

int rtr_dfltcDropRef(rtr_bufferpool_t *pool)
{
	int retvalue;
	RsslMutex *mtx = pool->mutex;

	RTBUFFERPOOLLOCK(pool);
	pool->numRefs--;
	retvalue = pool->numRefs;
	if (retvalue == 0)
		rtr_dfltcFreePool(pool);

	if (mtx)
	  (void) RSSL_MUTEX_UNLOCK(mtx);

	return(retvalue);
}

rtr_dfltcbufferpool_t *rtr_dfltcAllocatePool( int init_bufs, int max_bufs, int increase_bufs,
									size_t bufSize, rtr_bufferpool_t *sharedPool, int maxPoolBufs, RsslMutex *mutex )
{
	rtr_dfltcbufferpool_t *retpool=rtr_dfltcAllocPool(init_bufs,max_bufs,increase_bufs,sharedPool,maxPoolBufs,mutex);

	if (retpool)
	{
		if (rtr_dfltcSetBufSize(retpool,bufSize) < 0)
		{
			rtr_dfltcFreePool(&(retpool->bufpool));
			retpool = 0;
		}
	}
	return(retpool);
}

int rtr_dfltcFinishInit(rtr_bufferpool_t *pool, size_t maxBufSize)
{
	rtr_dfltcbufferpool_t	*rtr_dfltcpool;
	int	retval=0;
	int init_bufs;

	RTBUFFERPOOLLOCK(pool);
	rtr_dfltcpool = (rtr_dfltcbufferpool_t*)pool->internal;

	if (pool->initialized)
	{
		RTBUFFERPOOLUNLOCK(pool);
		return(-1);
	}

	pool->maxBufSize = maxBufSize;

	init_bufs = pool->numBufs;
	if (init_bufs < 0)
		init_bufs = 0;
	pool->numBufs = 0;


	if (rtr_dfltcpool->sharedPool &&
		(rtr_dfltcpool->sharedPool->maxBufSize < pool->maxBufSize) )
	{
		retval = -1;
		pool->maxBufSize = -1;
	}
	else if (rtr_dfltcIncreasePool(rtr_dfltcpool,init_bufs) < 0)
	{
		retval = -1;
		pool->maxBufSize = -1;
	}
	else if (rtr_dfltcIncreaseMblks(rtr_dfltcpool,init_bufs) < 0)
	{
		retval = -1;
		pool->maxBufSize = -1;
	}
	else
		pool->initialized = 1;

	RTBUFFERPOOLUNLOCK(pool);

	return(retval);
}

rtr_dfltcbufferpool_t *rtr_dfltcAllocPool(	int init_bufs, int max_bufs,int increase_bufs,
									rtr_bufferpool_t *sharedPool, int maxPoolBufs,
									RsslMutex *mutex )
{
	rtr_dfltcbufferpool_t *retpool;

	retpool = (rtr_dfltcbufferpool_t*)malloc(sizeof(rtr_dfltcbufferpool_t));
	if (retpool)
	{
		rsslInitQueue(&(retpool->freeList));
		rsslInitQueue(&(retpool->usedList));
		rsslInitQueue(&(retpool->freeMsgList));
		rsslInitQueue(&(retpool->allocatedMblks));
		rsslInitQueue(&(retpool->sharedPoolMblks));
		retpool->nextChar = 0;
		retpool->curDblk = 0;
#ifdef _DFLTC_BUFFER_DEBUG
		retpool->numFreeDblks = 0;
		retpool->numFreeMblks = 0;
		retpool->numUsedDblks = 0;
		retpool->numUsedMblks = 0;
#endif
		retpool->sharedPool = sharedPool;
		retpool->maxPoolBufs = maxPoolBufs;
		retpool->increaseBufRate = increase_bufs;
		retpool->numPoolBufs = 0;
		retpool->numRegBufsUsed = 0;
		retpool->peakNumBufsUsed =0;
		retpool->bufpool.maxBufs = max_bufs;
		retpool->bufpool.numBufs = init_bufs;
		retpool->bufpool.numRefs = 1;
		retpool->bufpool.maxBufSize = -1;
		retpool->bufpool.initialized = 0;
		retpool->bufpool.allocMsg = rtr_dfltcAllocMsg;
		retpool->bufpool.allocMaxMsg = rtr_dfltcAllocMaxMsg;
		retpool->bufpool.usedBytes = rtr_dfltcSetUsed;
		retpool->bufpool.dupMsg = rtr_dfltcDupMsg;
		retpool->bufpool.copyMsg = rtr_dfltcCopyMsg;
		retpool->bufpool.freeMsg = rtr_dfltcFreeMsg;
		retpool->bufpool.dropReference = rtr_dfltcDropRef;
		retpool->bufpool.addReference = rtr_dfltcAddRef;
		retpool->bufpool.setMaxBufs = rtr_dfltcSetMaxBufs;
		retpool->bufpool.finishInit = rtr_dfltcFinishInit;
		retpool->bufpool.internal = retpool;
		retpool->bufpool.mutex = mutex;

		if (sharedPool)
			rtrBufferPoolAddRef(sharedPool);
	}
	return(retpool);
}

int rtr_dfltcSetBufSize( rtr_dfltcbufferpool_t *pool, size_t bufSize )
{
	int	retval=0;
	int init_bufs;

	RTBUFFERPOOLLOCK(&(pool->bufpool));

	pool->bufpool.maxBufSize = bufSize;
	init_bufs = pool->bufpool.numBufs;
	if (init_bufs < 0)
		init_bufs = 0;
	pool->bufpool.numBufs = 0;

	if (pool->sharedPool &&
		(pool->sharedPool->maxBufSize < pool->bufpool.maxBufSize) )
	{
		retval = -1;
		pool->bufpool.maxBufSize = -1;
	}
	else if (rtr_dfltcIncreasePool(pool,init_bufs) < 0)
	{
		retval = -1;
		pool->bufpool.maxBufSize = -1;
	}
	else if (rtr_dfltcIncreaseMblks(pool,init_bufs) < 0)
	{
		retval = -1;
		pool->bufpool.maxBufSize = -1;
	}
	else
		pool->bufpool.initialized = 1;

	RTBUFFERPOOLUNLOCK(&(pool->bufpool));

	return(retval);
}
