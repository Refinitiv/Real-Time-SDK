/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2019 Refinitiv. All rights reserved.            --
 *|-----------------------------------------------------------------------------
 */

#ifndef	__cbuffer_h
#define	__cbuffer_h

#ifdef DEV_SVR4
#include <sys/types.h>
#endif

#ifdef Linux
#include <sys/types.h>
#endif

#include "rtr/rsslQueue.h"
#include "rtr/platform.h"
#include "rtr/rsslThread.h"

#ifdef __cplusplus
extern "C" {
#endif


typedef struct rtr_bufferpool {
	struct rtr_msgb	*(*allocMsg)(struct rtr_bufferpool *pool,size_t size);
						/* Allocate a message of 'size' and return
						 * a rtr_msgb_t that represents that message.
						 */
	struct rtr_msgb	*(*allocMaxMsg)(struct rtr_bufferpool *pool);
						/* Allocate a message of 'maxBufSize' and return
						 * a rtr_msgb_t that represents that message.
						 * Guarantee the rtr_datab_t this message refers
						 * is solely owned by this rtr_msgb_t.
						 */
	int				(*usedBytes)(struct rtr_bufferpool *pool,struct rtr_msgb* mblk);
						/* Set the actually number of used bytes in
						 * the mblk as opposed to the number of bytes
						 * requested in allocMsg(). Must be less than,
						 * or equal to, the number of requested bytes.
						 */
	struct rtr_msgb	*(*dupMsg)(struct rtr_bufferpool *pool,struct rtr_msgb * mblk);
						/* Duplicate the current mblk while refering to
						 * the same rtr_datab_t. A new rtr_msgb will be returned
						 * that is the same as mblk and refers to the exact
						 * same rtr_datab_t.
						 */
	struct rtr_msgb	*(*copyMsg)(struct rtr_bufferpool *pool,struct rtr_msgb * mblk);
						/* Make a copy of mblk. This also makes a copy of the
						 * underlying rtr_datab_t.
						 */
	int				(*freeMsg)(struct rtr_msgb* mblk);
						/* Return the mblk to the pool. */
	int				(*addReference)(struct rtr_bufferpool *pool);
						/* Add a reference to the pool. Returns
						 * the new number of refernces. */
	int				(*dropReference)(struct rtr_bufferpool *pool);
						/* Drop a reference from the pool. Delete
						 * pool if goes to zero. Returns the new
						 * number of references (0 implies pool deleted).
						 */
	int				(*setMaxBufs)(struct rtr_bufferpool *pool,int num);
						/* Set the maximum number of buffers. */
	int				(*finishInit)(struct rtr_bufferpool *pool,size_t maxBufSize);
	RsslMutex		*mutex;
	size_t			maxBufSize;	/* Maximum buffer size */
	int				maxBufs;	/* Maximum number of buffers */
	int				numBufs;	/* Current number of buffers allocated (not necessarily used) */
	unsigned short	numRefs;	/* Number of references to this pool */
	unsigned short	initialized;/* Has this pool been fully initialized */
	void			*internal;	/* Internal, never touch */
} rtr_bufferpool_t;

#define RTBUFFERPOOLLOCK(pool) \
  if ((pool)->mutex) (void) RSSL_MUTEX_LOCK((pool)->mutex);

#define RTBUFFERPOOLUNLOCK(pool) \
  if ((pool)->mutex) (void) RSSL_MUTEX_UNLOCK((pool)->mutex);

typedef struct rtr_datab {
	RsslQueueLink	link;		/* Internal, never touch */
	rtr_bufferpool_t	*pool;		/* Pool associated with this message */
	caddr_t			base;		/* Base memory point for data block */
	size_t			length;		/* Maximum length of data block */
	unsigned short	numRefs;	/* Reference counting */
	unsigned char	flags;
	unsigned char	pad;
	void			*internal;	/* Internal, never touch */
} rtr_datab_t;

typedef struct rtr_msgb {
	RsslQueueLink	link;		/* May be used for list operations by owner */
	rtr_bufferpool_t	*pool;		/* Pool associated with this message */
	void			*internal;	/* Internal, never touch */
	struct rtr_msgb	*nextMsg;	/* Next message block in chain */
	caddr_t			buffer;		/* Base memory pointer for message block */
	size_t			length;		/* Length of this message block */
	size_t			maxLength;	/* Maximum length of this message block */
	struct rtr_datab	*datab;		/* Data block this message refers */
	unsigned short	flags;
	unsigned short	protocol;
	unsigned short  fragOffset;
	int				priority;   /* which priority queue to write to */
	void			*local;		/* Local storage for however owns the rtr_msgb_t */
	unsigned short	protocolHdr;
	unsigned short	protocolHdrLength;		/* Maximum length of data block */
} rtr_msgb_t;


#define rtrBufferAlloc(pool,size) ((*((pool)->allocMsg))(pool,size))
#define rtrBufferAllocMax(pool) ((*((pool)->allocMaxMsg))(pool))
#define rtrBufferSetUsed(pool,msg) ((*((pool)->usedBytes))(pool,msg))
#define rtrBufferDup(pool,msg) ((*((pool)->dupMsg))(pool,msg))
#define rtrBufferCopy(pool,msg) ((*((pool)->copyMsg))(pool,msg))
#define rtrBufferFree(pool,msg) ((*((pool)->freeMsg))(msg))

#define rtrBufferPoolSetMaxBufs(pool,numBufs) ((*((pool)->setMaxBufs))(pool,numBufs))
#define rtrBufferPoolAddRef(pool) ((*((pool)->addReference))(pool))
#define rtrBufferPoolDropRef(pool) ((*((pool)->dropReference))(pool))
#define rtrBufferPoolFinishInit(pool,bufsize) ((*((pool)->finishInit))(pool,bufsize))



extern int rtr_cbufferCppOverhead;
extern void *(*rtr_cbufferCppInit)(rtr_msgb_t*,void*);



#ifdef __cplusplus
} /* extern "C" */
#endif


#endif
