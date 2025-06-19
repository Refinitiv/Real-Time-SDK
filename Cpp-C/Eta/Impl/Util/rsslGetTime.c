/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2020,2024 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

#include "rtr/rsslGetTime.h"

static int rsslGetTimeIsInitialized=0;
static RsslDouble rsslTicksPerNano=0;
static RsslDouble rsslTicksPerMicro=0;
static RsslDouble rsslTicksPerMilli=0;


/* Linux uses the monotonic clock to get time.  Here, ticks are the same as the nansecond time. */
#if defined(Linux)

#include <time.h>
#include <errno.h>
#include <sys/time.h>

RSSL_API RsslDouble rsslGetTicksPerNano()
{
	return (RsslDouble)1;
}

RSSL_API RsslDouble rsslGetTicksPerMicro()
{
	return (RsslDouble)1000;
}

RSSL_API RsslDouble rsslGetTicksPerMilli()
{
	return (RsslDouble)1000000;
}

RSSL_API RsslTimeValue rsslGetTicks()
{
	return rsslGetTimeNano();
}


RSSL_API RsslTimeValue rsslGetTimeNano()
{
	RsslTimeValue	ret;
	struct timespec	ts;

	if ( clock_gettime(CLOCK_MONOTONIC,&ts) == 0)
	{
		ret = (RsslTimeValue)ts.tv_sec * 1000000000ULL + (RsslTimeValue)ts.tv_nsec;
	}
	else
	{
		struct timeval tv;
		gettimeofday(&tv,0);
		ret = (RsslTimeValue)tv.tv_sec * 1000000000ULL + (RsslTimeValue)tv.tv_usec * 1000ULL;
	}
	return(ret);
}

RSSL_API RsslTimeValue rsslGetTimeMicro()
{
	RsslTimeValue	ret;
	struct timespec	ts;

	if ( clock_gettime(CLOCK_MONOTONIC,&ts) == 0)
	{
		ret = (RsslTimeValue)ts.tv_sec * 1000000ULL + (RsslTimeValue)ts.tv_nsec / 1000ULL;
	}
	else
	{
		struct timeval tv;
		gettimeofday(&tv,0);
		ret = (RsslTimeValue)tv.tv_sec * 1000000ULL + (RsslTimeValue)tv.tv_usec;
	}
	return(ret);
}

RSSL_API RsslTimeValue rsslGetTimeMilli()
{
	RsslTimeValue	ret;
	struct timespec	ts;

	if ( clock_gettime(CLOCK_MONOTONIC,&ts) == 0)
	{
		ret = (RsslTimeValue)ts.tv_sec * 1000ULL + (RsslTimeValue)ts.tv_nsec / 1000000ULL;
	}
	else
	{
		struct timeval tv;
		gettimeofday(&tv,0);
		ret = (RsslTimeValue)tv.tv_sec * 1000ULL + (RsslTimeValue)tv.tv_usec / 1000ULL;
	}
	return(ret);
}

#else /* Windows uses the performance counter.  The first time any of these functions are called, we need to calculate the clock frequency */
#include <Windows.h>

static void rsslGetTimeInitialize()
{

	if (!rsslGetTimeIsInitialized)
	{
		LARGE_INTEGER	m_Freq;

		/* Get the number of clock ticks per second */
		QueryPerformanceFrequency(&m_Freq);

		/* Calculate the clock tick frequencies */
		rsslTicksPerNano = (RsslDouble)m_Freq.QuadPart/1000000000.0;
		rsslTicksPerMicro = (RsslDouble)m_Freq.QuadPart/1000000.0;
		rsslTicksPerMilli = (RsslDouble)m_Freq.QuadPart/1000.0;

		rsslGetTimeIsInitialized = 1;
	}
}

RSSL_API RsslDouble rsslGetTicksPerNano()
{
	if (!rsslGetTimeIsInitialized)
		rsslGetTimeInitialize();
	return rsslTicksPerNano;
}

RSSL_API RsslDouble rsslGetTicksPerMicro()
{
	if (!rsslGetTimeIsInitialized)
		rsslGetTimeInitialize();
	return rsslTicksPerMicro;
}

RSSL_API RsslDouble rsslGetTicksPerMilli()
{
	if (!rsslGetTimeIsInitialized)
		rsslGetTimeInitialize();
	return rsslTicksPerMilli;
}

RSSL_API RsslTimeValue rsslGetTicks()
{
	LARGE_INTEGER	m_Counter;

	if (!rsslGetTimeIsInitialized)
		rsslGetTimeInitialize();

	if (!QueryPerformanceCounter(&m_Counter))
		return((RsslTimeValue)0);

	return((RsslTimeValue)m_Counter.QuadPart);
}

RSSL_API RsslTimeValue rsslGetTimeNano()
{
	LARGE_INTEGER	m_Counter;
	double			ret;

	if (!rsslGetTimeIsInitialized)
		rsslGetTimeInitialize();

	if ( !QueryPerformanceCounter(&m_Counter) )
		return( (RsslTimeValue)0 );

	ret = (double)m_Counter.QuadPart / rsslTicksPerNano;
	return( (RsslTimeValue)ret );
}

RSSL_API RsslTimeValue rsslGetTimeMicro()
{
	LARGE_INTEGER	m_Counter;
	double			ret;

	if (!rsslGetTimeIsInitialized)
		rsslGetTimeInitialize();

	if ( !QueryPerformanceCounter(&m_Counter) )
		return( (RsslTimeValue)0 );

	ret = (double)m_Counter.QuadPart / rsslTicksPerMicro;
	return( (RsslTimeValue)ret );
}

RSSL_API RsslTimeValue rsslGetTimeMilli()
{
	LARGE_INTEGER	m_Counter;
	double			ret;

	if (!rsslGetTimeIsInitialized)
		rsslGetTimeInitialize();

	if ( !QueryPerformanceCounter(&m_Counter) )
		return( (RsslTimeValue)0 );

	ret = (double)m_Counter.QuadPart / rsslTicksPerMilli;
	return( (RsslTimeValue)ret );
}

#endif
