/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2019 LSEG. All rights reserved.                 --
 *|-----------------------------------------------------------------------------
 */
 
#ifndef __RTR_SHMEM_CIRBUF
#define __RTR_SHMEM_CIRBUF

#ifdef __cplusplus
extern "C" {
#endif

#include "shmem.h"


typedef struct
{
	RTR_SHM_OFFSET	start;		/* Start of the buffer */
	RTR_SHM_OFFSET	end;		/* End of the buffer */
	RTR_SHM_OFFSET	write;		/* Current position to write into the buffer */
	RTR_SHM_OFFSET	read;		/* Current position to read from the buffer */
	rtrUInt32		maxBufSize;	/* size of the individual buffers */
	rtrUInt32		numBuffers;	/* number of buffers in the circular queue */
} rtrShmCirBuf;

void RTRShmCirBufServerInit( rtrShmCirBuf* cBuf, rtrUInt32 numBuffers, rtrUInt32 maxBufSize, rtrShmSeg* shMemSeg );
void RTRShmCirBufClientInit( rtrShmCirBuf* circBufClient, rtrShmCirBuf* circBufServer );


RTR_C_ALWAYS_INLINE char* RTRShmCirBufGetWriteBuf( rtrShmCirBuf* cBuf, rtrShmSeg* shMemSeg )
{
	return (char*)RTR_SHM_MAKE_PTR(shMemSeg->base,cBuf->write);
}

RTR_C_ALWAYS_INLINE void RTRShmCirBufWritten( rtrShmCirBuf* cBuf )
{
	cBuf->write += cBuf->maxBufSize;
	if (rtrUnlikely(cBuf->write >= cBuf->end))
		cBuf->write = cBuf->start;
}

RTR_C_ALWAYS_INLINE char* RTRShmCirBufRead( rtrShmCirBuf* cBuf, rtrShmSeg* shMemSeg )
{
	return (char*)RTR_SHM_MAKE_PTR(shMemSeg->base,cBuf->read);
}

RTR_C_ALWAYS_INLINE void RTRShmCirBufReadComplete( rtrShmCirBuf* cBuf )
{
	cBuf->read += cBuf->maxBufSize;
	if (rtrUnlikely(cBuf->read >= cBuf->end))
	{
		cBuf->read = cBuf->start;
	}
}


#ifdef __cplusplus
};
#endif

#endif
