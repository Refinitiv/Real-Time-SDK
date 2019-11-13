/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2019 Refinitiv. All rights reserved.            --
 *|-----------------------------------------------------------------------------
 */

#ifndef RTR_GLOBAL_DEFS
#define RTR_GLOBAL_DEFS

typedef unsigned int RTRBOOL; 
#define RTRTRUE 1
#define RTRFALSE 0


#ifdef __cplusplus
extern "C" {
#endif
	void rtrfail(const char* msg, const char* file, int line);
#ifdef __cplusplus
}
#endif


/* RTRFAIL prints a failure message and exits, even if not debugging */
#define RTRFAIL(msg)			rtrfail(msg, __FILE__, __LINE__)

#if defined(__STDC__) || defined(_MSC_VER)
#define RTRASSERT(EX)	(void)((EX) || (RTRFAIL(#EX), 0))
#else
#define RTRASSERT(EX)	(void)((EX) || (RTRFAIL("EX"), 0))
#endif

#ifdef RTDEBUG
#  define RTPRECONDITION(a)		RTRASSERT(a) 
#  define RTPOSTCONDITION(a)	RTRASSERT(a)
#else
#  define RTPRECONDITION(a)
#  define RTPOSTCONDITION(a)
#endif

#define   rtr_offsetof(s, m)  (int)(&(((s *)0)->m))

#endif
