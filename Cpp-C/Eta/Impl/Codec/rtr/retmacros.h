/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2019 LSEG. All rights reserved.                 --
 *|-----------------------------------------------------------------------------
 */

#ifndef _RTR_RSSL_RET_MACROS_H
#define _RTR_RSSL_RET_MACROS_H

#include "rtr/rsslRetCodes.h"

#include <stdio.h>
#include <stdlib.h>

#ifdef __cplusplus
extern "C" {
#endif
#ifndef _KERNEL


RTR_C_ALWAYS_INLINE void rsslfail(const char* cond, const char* msg, const char* file, int line)
{
	static char fmt[] = "Fatal Error: %s, %s (file %s, line %d)\n";
	fprintf(stderr, fmt, cond, msg, file, line);
	abort();
}

#else

#include <sys/debug.h>

RTR_C_ALWAYS_INLINE void rsslfail(const char* cond, const char* msg, const char* file, int line)
{
	char err[2500];
	snprintf(err, 2500, "%s, %s", cond, msg);
	assfail(err,__FILE__,__LINE__);
}

#endif


/* RTRFAIL prints a failure message and exits, even if not debugging */
#define RSSLFAIL(cond, msg)			rsslfail(cond, msg, __FILE__, __LINE__)

#if defined(NDEBUG) && !defined(_RSSL_RELEASE_ASSERT_) 
	#define _RSSL_ASSERT(COND, MSG)
	#define RSSL_ASSERT(COND, MSG)
#else
	#if defined(__STDC__) || defined(_MSC_VER)
		#define RSSL_ASSERT(COND, MSG) (void)((COND) || (RSSLFAIL(#COND, #MSG), 0))
		#define _RSSL_ASSERT(COND, MSG) (void)((COND) || (RSSLFAIL(#COND, #MSG), 0))
	#else
		#define RSSL_ASSERT(COND, MSG) (void)((COND) || (RSSLFAIL("COND", "MSG"), 0))
		#define _RSSL_ASSERT(COND, MSG) (void)((COND) || (RSSLFAIL("COND","MSG"), 0))
	#endif
#endif


#define RSSL_CHECK(c,b) _RSSL_ASSERT(!(c))

#include <stdio.h>


#define RSSL_CHECKOFFBUFFER(pos, buf) RSSL_CHECKSIZE((pos), (buf).data + (buf).length)

#define RSSL_CHECKRET( r )	if (r < 0) return(RSSL_RET_FAILURE);
#define RSSL_CHECKSIZE(a,size) if (a > size) return(RSSL_RET_INCOMPLETE_DATA);

#ifdef __cplusplus
}
#endif

#endif

