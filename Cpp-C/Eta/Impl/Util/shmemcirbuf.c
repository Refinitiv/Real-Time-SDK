/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright Thomson Reuters 2018. All rights reserved.            --
 *|-----------------------------------------------------------------------------
 */
 
#include "rtr/shmemcirbuf.h"


void RTRShmCirBufServerInit( rtrShmCirBuf* cBuf,
							 rtrUInt32 numBuffers,
							 rtrUInt32 maxBufSize,
							 rtrShmSeg* shMemSeg )
{
	char *start,*end;
	size_t totalSize = numBuffers * maxBufSize;

	start = rtrShmBytesReserve( shMemSeg, totalSize );
	end = start + totalSize;
	cBuf->start = cBuf->write = cBuf->read = RTR_SHM_MAKE_OFFSET(shMemSeg->base,start);
	cBuf->end = RTR_SHM_MAKE_OFFSET(shMemSeg->base,end);
	cBuf->maxBufSize = maxBufSize;
	cBuf->numBuffers = numBuffers;
};

void RTRShmCirBufClientInit( rtrShmCirBuf* circBufClient, rtrShmCirBuf* circBufServer )
{
	circBufClient->start = circBufServer->start;
	circBufClient->end = circBufServer->end;
	circBufClient->read = circBufServer->write;
	circBufClient->maxBufSize = circBufServer->maxBufSize;
	circBufClient->numBuffers = circBufServer->numBuffers;
};


