/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2018-2019,2024,2026 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

#ifndef __rsslalloc_h
#define __rsslalloc_h


#include "rtr/os.h"
#include "rtr/rsslTypes.h"

#include <sys/types.h>
#include <stdlib.h>

#include <string.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Function pointer types used to abstract the underlying memory allocator.
 * These allow the allocation routines used throughout RSSL to be overridden
 * (e.g. by rsslTransportUnitTest) to test allocation failure handling or
 * to track/instrument memory usage, while defaulting to the standard
 * malloc/realloc/free based implementations below. */
typedef void* (*RsslMallocFunc)(size_t size);
typedef void* (*RsslReallocFunc)(char *buf, size_t origSize, size_t newSize);
typedef void  (*RsslFreeFunc)(void *mem);

/* Default (built-in) allocator implementations. */
RTR_C_ALWAYS_INLINE void* _rsslMallocDefault(size_t size)
{
	return(malloc(size));
}

RTR_C_ALWAYS_INLINE void* _rsslReallocDefault(char *buf, size_t origSize, size_t newSize)
{
	void *newBuf=malloc(newSize);
	if (newBuf)
	{
		memcpy(newBuf, buf, origSize);
		free(buf);
	}
	return (newBuf);
}

RTR_C_ALWAYS_INLINE void _rsslFreeDefault(void *mem)
{
	free(mem);
}

/* Function pointers used for all memory allocation/deallocation within RSSL.
 * These default to the built-in implementations above (set in rsslImpl.c),
 * but can be overwritten - e.g. by rsslTransportUnitTest - to substitute a
 * custom allocator. */
RSSL_API extern RsslMallocFunc  rsslMallocFunc;
RSSL_API extern RsslReallocFunc rsslReallocFunc;
RSSL_API extern RsslFreeFunc    rsslFreeFunc;

RTR_C_ALWAYS_INLINE void* _rsslMalloc(size_t size)
{
	return (*rsslMallocFunc)(size);
}

RTR_C_ALWAYS_INLINE void* _rsslRealloc(char *buf, size_t origSize, size_t newSize)
{
	return (*rsslReallocFunc)(buf, origSize, newSize);
}

RTR_C_ALWAYS_INLINE void _rsslFree(void *mem)
{
	(*rsslFreeFunc)(mem);
}


#ifdef __cplusplus
} /* extern "C" */
#endif

#endif
