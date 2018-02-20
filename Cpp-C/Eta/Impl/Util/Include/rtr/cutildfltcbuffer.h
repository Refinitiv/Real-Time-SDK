/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright Thomson Reuters 2018. All rights reserved.            --
 *|-----------------------------------------------------------------------------
 */

#ifndef	__rtr_dfltcbuffer_h
#define	__rtr_dfltcbuffer_h

#include "rtr/cutilcbuffer.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct rtr_dfltcbufferpool {
	rtr_bufferpool_t	bufpool;
	RsslQueue	freeList;
	RsslQueue	freeMsgList;
	RsslQueue	usedList;
	RsslQueue	allocatedMblks;
	rtr_datab_t		*curDblk;
	caddr_t			nextChar;
	int				increaseBufRate;
	rtr_bufferpool_t	*sharedPool;
	int				maxPoolBufs;
	int				numPoolBufs;
	int				numRegBufsUsed; /* Current number of buffers used */
	int				peakNumBufsUsed; /* Peak number of buffers used */
	RsslQueue	sharedPoolMblks;
#ifdef _DFLTC_BUFFER_DEBUG
	unsigned int numFreeDblks;
	unsigned int numFreeMblks;
	unsigned int numUsedDblks;
	unsigned int numUsedMblks;
#endif
} rtr_dfltcbufferpool_t;

enum rtr_dfltcMsgbFlags
{
	rtr_dfltcMsgbPutInFreeList = 0x01
};


#define rtr_dfltcSetUsedLast(mypool,curmblk) \
	( (	((mypool)->curDblk == (curmblk)->datab) && \
		 (((curmblk)->buffer + (curmblk)->maxLength) == (mypool)->nextChar )) ? \
	 ((mypool)->nextChar -= ((curmblk)->maxLength - (curmblk)->length), \
	 (curmblk)->maxLength = (curmblk)->length, 1 ): 0 )


	/* Align 'bytes' on alignment */
extern size_t rtr_dfltcAlignBytes( size_t bytes, size_t alignment );

	/* Default buffer pool to initialize */
extern rtr_dfltcbufferpool_t *rtr_dfltcAllocatePool(
			int initBufs, int maxBufs, int increaseBufs,
			size_t bufSize, rtr_bufferpool_t *sharedPool,
			int maxPoolBufs, RsslMutex *mutex );

	/* Used this function in conjunction with rtr_dfltcSetBufSize when
	 * the buffer size is not pre known.
	 */
extern rtr_dfltcbufferpool_t *rtr_dfltcAllocPool(
			int initBufs, int max_bufs,int increase_bufs,
			rtr_bufferpool_t *sharedPool, int maxPoolBufs,
			RsslMutex *mutex );
extern int rtr_dfltcSetBufSize( rtr_dfltcbufferpool_t *pool, size_t bufSize );

extern int rtr_dfltcAddRef(rtr_bufferpool_t *pool);
extern int rtr_dfltcDropRef(rtr_bufferpool_t *pool);

extern rtr_msgb_t *rtr_dfltcAllocMsg(rtr_bufferpool_t *pool, size_t size);
extern rtr_msgb_t *rtr_dfltcAllocMaxMsg(rtr_bufferpool_t *pool);
extern rtr_msgb_t *rtr_dfltcDupMsg(rtr_bufferpool_t *pool, rtr_msgb_t *curmblk);
extern rtr_msgb_t *rtr_dfltcCopyMsg(rtr_bufferpool_t *pool, rtr_msgb_t *curmblk);

extern int rtr_dfltcFreeMsg(rtr_msgb_t *mblk);
extern int rtr_dfltcSetUsed(rtr_bufferpool_t *pool, rtr_msgb_t *curmblk);
extern int rtr_dfltcSetMaxBufs(rtr_bufferpool_t *pool, int newValue );
extern int rtr_dfltcSetMaxSharedBufs(rtr_bufferpool_t *pool, int newValue );
extern int rtr_dfltcResetPeakNumBufs(rtr_bufferpool_t *pool);


#ifdef __cplusplus
} /* extern "C" */
#endif


#endif
