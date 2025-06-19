/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2019,2021,2024 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

/* testUtils.h
 * Functions useful for multiple performance tools. */
#ifndef _TEST_UTILS_H
#define _TEST_UTILS_H

#include "rtr/rsslTypes.h"
#include "rtr/rsslRetCodes.h"
#include "rtr/rsslReal.h"
#include "rtr/rsslDateTime.h"
#include "rtr/rsslQos.h"
#include "rtr/rsslState.h"

#include <stdio.h>
#include <time.h>

#if defined(WIN32)
#include <windows.h>
#elif defined(Linux)
#include <sched.h>
#else
#include <sys/types.h>
#include <sys/processor.h>
#include <sys/procset.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif

#ifdef _WIN32
#ifdef _WIN64
#define SOCKET_PRINT_TYPE "%llu"	/* WIN64 */
#else
#define SOCKET_PRINT_TYPE "%u"	/* WIN32 */
#endif
#else
#define SOCKET_PRINT_TYPE "%d"  /* Linux */
#endif

/*** Convenience functions for checking function calls. Each prints out a message on failure, along with file and line information. ***/

/* Checks a condition.  Prints the text of the condition upon failure. */
#define CHECK(__cond) ((__cond) || (printf("Failure: %s\n - (%s, Line:%d)\n", (#__cond), __FILE__, __LINE__), 0))

/* Checks a condition.  Prints the text of the condition upon failure, along with the code returned in the RsslRet object. */
#define CHECK_RSSLRET(__cond, __rsslRet) ((__cond) || (printf("Failure: \"%s\"\n - returned %s\n - at %s, Line %d\n", (#__cond), rsslRetCodeToString(__rsslRet), __FILE__, __LINE__), 0))

/* Checks a condition.  Prints the text of the condition upon failure, along with the code and text returned in the RsslError structure. */
#define CHECK_RSSLERROR(__cond, __rsslError) ((__cond) || (printf("Failure: \"%s\"\n - at %s, Line %d\n - returned %s, \"%s\"\n\n", (#__cond), __FILE__, __LINE__, rsslRetCodeToString((__rsslError).rsslErrorId), (__rsslError).text), 0))

#ifdef WIN32
#define SLEEP(__seconds) (Sleep(__seconds * 1000))
#else
#define SLEEP(__seconds) (sleep(__seconds))
#endif

/* Prints the current time, in Coordinated Universal Time. */
RTR_C_INLINE void printCurrentTimeUTC(FILE *file)
{
	time_t statsTime;
	struct tm *pStatsTimeTm;

	time(&statsTime);
	pStatsTimeTm = gmtime(&statsTime);

	fprintf(file, "%d-%02d-%02d %02d:%02d:%02d",
			pStatsTimeTm->tm_year + 1900, pStatsTimeTm->tm_mon + 1, pStatsTimeTm->tm_mday,
			pStatsTimeTm->tm_hour, pStatsTimeTm->tm_min, pStatsTimeTm->tm_sec);
}

/* Binds the calling thread to the core with the given CPU ID. */
RTR_C_INLINE RsslRet bindThread(RsslInt32 cpuId)
{
#if defined(WIN32)
	DWORD cpuMask = 1 << cpuId;
	return (SetThreadAffinityMask(GetCurrentThread(), cpuMask) != 0) ? RSSL_RET_SUCCESS : RSSL_RET_FAILURE;
#elif defined(Linux)
	cpu_set_t cpuSet;	
	CPU_ZERO(&cpuSet);
	CPU_SET(cpuId, &cpuSet);

	return (sched_setaffinity(0, sizeof(cpu_set_t), &cpuSet) == 0) ? RSSL_RET_SUCCESS : RSSL_RET_FAILURE;
#else /* Solaris */
	return (processor_bind(P_LWPID, P_MYID, cpuId, NULL) == 0) ? RSSL_RET_SUCCESS : RSSL_RET_FAILURE;
#endif
}

/* A union of the RSSL primitive types. */
typedef union {
	RsslInt			intType;
	RsslUInt		uintType;
	RsslFloat		floatType;
	RsslDouble		doubleType;
	RsslReal		realType;
	RsslDate		dateType;
	RsslTime		timeType;
	RsslDateTime	dateTimeType;
	RsslQos			qosType;
	RsslState		stateType;
	RsslEnum		enumType;
	RsslBuffer		bufferType;
} RsslPrimitive;

#ifdef __cplusplus
};
#endif


#endif
