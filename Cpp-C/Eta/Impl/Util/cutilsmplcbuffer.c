/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright Thomson Reuters 2018. All rights reserved.            --
 *|-----------------------------------------------------------------------------
 */

#include <stdio.h>
#include <malloc.h>

#include "rtr/cutilsmplcbuffer.h"

enum {
	rtr_smplcFreeMblk = 0x01
} rtr_smplcMsgbFlags;

rtr_msgb_t *rtr_smplcIntAllocD(rtr_bufferpool_t *pool, size_t size)
{
	void		*memory;
	rtr_msgb_t	*mblk=0;
	rtr_datab_t	*dblk;
	memory = malloc(sizeof(rtr_datab_t) + sizeof(rtr_msgb_t) + rtr_cbufferCppOverhead + size);
	if (memory)
	{
		dblk = (rtr_datab_t*)memory;
		mblk = (rtr_msgb_t*)((caddr_t)memory + sizeof(rtr_datab_t));

		dblk->base = (caddr_t)((caddr_t)mblk + sizeof(rtr_msgb_t) + rtr_cbufferCppOverhead);
		dblk->length = size;
		dblk->pool = pool;
		dblk->numRefs = 1;
		dblk->flags = 0;
		dblk->pad = 0;
		dblk->internal = 0;

		mblk->nextMsg = 0;
		mblk->buffer = dblk->base;
		mblk->length = 0;
		mblk->maxLength = dblk->length;
		mblk->datab = dblk;
		mblk->flags = 0;
		mblk->protocol = 0;
		mblk->fragOffset = 0;
		mblk->priority = 0;
		mblk->pool = pool;
		if (rtr_cbufferCppOverhead && rtr_cbufferCppInit)
		{
			mblk->internal = ((caddr_t)mblk + sizeof(rtr_msgb_t));
			mblk->internal = (void*)(*(rtr_cbufferCppInit))(mblk,mblk->internal);
		}
		else
			mblk->internal = 0;

		RTBUFFERPOOLLOCK(pool);
		pool->numBufs++;
		RTBUFFERPOOLUNLOCK(pool);
	}
	return(mblk);
}

rtr_msgb_t *rtr_smplcIntAllocM(rtr_bufferpool_t *pool)
{
	rtr_msgb_t	*mblk;
	mblk = (rtr_msgb_t*)malloc(sizeof(rtr_msgb_t) + rtr_cbufferCppOverhead);
	if (mblk)
	{
		mblk->nextMsg = 0;
		mblk->buffer = 0;
		mblk->length = 0;
		mblk->maxLength = 0;
		mblk->datab = 0;
		mblk->flags = rtr_smplcFreeMblk;
		mblk->protocol = 0;
		mblk->fragOffset = 0;
		mblk->priority = 0;
		mblk->pool = pool;
		if (rtr_cbufferCppOverhead && rtr_cbufferCppInit)
		{
			mblk->internal = ((caddr_t)mblk + sizeof(rtr_msgb_t));
			mblk->internal = (void*)(*(rtr_cbufferCppInit))(mblk,mblk->internal);
		}
		else
			mblk->internal = 0;
	}
	return(mblk);
}

int rtr_smplcFreePool( rtr_bufferpool_t *pool )
{
	if ((pool == 0) || (pool->internal == 0))
		return(-1);
	free(pool);
	return(1);
}

rtr_msgb_t *rtr_smplcAllocMsg(rtr_bufferpool_t *pool, size_t size)
{
	rtr_msgb_t	*mblk;
	if ((pool == 0) || (pool->internal == 0))
		return(0);
	mblk=rtr_smplcIntAllocD(pool,size);
	return(mblk);
}

rtr_msgb_t *rtr_smplcAllocMaxMsg(rtr_bufferpool_t *pool)
{
	return(0);
}

int rtr_smplcFreeMsg(rtr_msgb_t *mblk)
{
	int					doFree;

	if ((mblk == 0) || (mblk->pool == 0) || (mblk->datab == 0))
		return(-1);

	mblk->datab->numRefs--;

	doFree = (mblk->flags & rtr_smplcFreeMblk);

	if (mblk->datab->numRefs <= 0)
	{
		free(mblk->datab);
	}

	if (doFree)
	{
		free(mblk);
	}

	return(1);
}


int rtr_smplcSetUsed(rtr_bufferpool_t *pool, rtr_msgb_t *curmblk)
{
	if ((pool == 0) || (pool->internal == 0))
		return(0);

	return(1);
}

rtr_msgb_t *rtr_smplcDupMsg(rtr_bufferpool_t *pool, rtr_msgb_t *curmblk)
{
	rtr_msgb_t	*mblk=0;

	if ((pool == 0) || (curmblk == 0) || (pool->internal == 0))
		return(0);

	if ((mblk = rtr_smplcIntAllocM(pool)) == 0)
		return(0);

	mblk->nextMsg = 0;
	mblk->buffer = curmblk->buffer;
	mblk->length = curmblk->length;
	mblk->maxLength = curmblk->maxLength;
	mblk->datab = curmblk->datab;
	mblk->protocol = curmblk->protocol;
	mblk->fragOffset = curmblk->fragOffset;
	mblk->priority = curmblk->priority;
	mblk->datab->numRefs++;
	return(mblk);
}

rtr_msgb_t *rtr_smplcCopyMsg(rtr_bufferpool_t *pool, rtr_msgb_t *curmblk)
{
	rtr_msgb_t	*mblk=0;

	if ((pool == 0) || (curmblk == 0) || (pool->internal == 0))
		return(0);

	if ((mblk = rtr_smplcIntAllocD(pool,curmblk->maxLength)) == 0)
		return(0);

	mblk->nextMsg = 0;
	memcpy(mblk->buffer,curmblk->buffer,curmblk->length);
	mblk->length = curmblk->length;
	mblk->protocol = curmblk->protocol;
	mblk->fragOffset = curmblk->fragOffset;
	mblk->priority = curmblk->priority;
	return(mblk);
}

int rtr_smplcAddRef(rtr_bufferpool_t *pool)
{
	int retvalue;
	pool->numRefs++;
	retvalue = pool->numRefs;
	return(retvalue);
}

int rtr_smplcDropRef(rtr_bufferpool_t *pool)
{
	int retvalue;
	pool->numRefs--;
	retvalue = pool->numRefs;
	if (retvalue == 0)
		rtr_smplcFreePool(pool);
	return(retvalue);
}

int rtr_smplcFinishInit(rtr_bufferpool_t *pool,size_t bufSize)
{
	return(-1);
}

int rtr_smplcSetMaxBufs(rtr_bufferpool_t *pool,int bufSize)
{
	return(1);
}

rtr_bufferpool_t *rtr_smplcAllocatePool(RsslMutex *mutex)
{
	rtr_bufferpool_t *retpool;

	retpool = (rtr_bufferpool_t*)malloc(sizeof(rtr_bufferpool_t));
	if (retpool)
	{
		retpool->maxBufs = -1;
		retpool->numBufs = 0;
		retpool->numRefs = 1;
		retpool->maxBufSize = -1;
		retpool->initialized = 1;
		retpool->allocMsg = rtr_smplcAllocMsg;
		retpool->allocMaxMsg = rtr_smplcAllocMaxMsg;
		retpool->usedBytes = rtr_smplcSetUsed;
		retpool->dupMsg = rtr_smplcDupMsg;
		retpool->copyMsg = rtr_smplcCopyMsg;
		retpool->freeMsg = rtr_smplcFreeMsg;
		retpool->dropReference = rtr_smplcDropRef;
		retpool->addReference = rtr_smplcAddRef;
		retpool->setMaxBufs = rtr_smplcSetMaxBufs;
		retpool->finishInit = rtr_smplcFinishInit;
		retpool->internal = retpool;
		retpool->mutex = mutex;
	}
	return(retpool);
}

