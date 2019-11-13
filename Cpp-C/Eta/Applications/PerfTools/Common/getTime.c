/*
 * This source code is provided under the Apache 2.0 license and is provided
 * AS IS with no warranty or guarantee of fit for purpose.  See the project's 
 * LICENSE.md for details. 
 * Copyright (C) 2019 Refinitiv. All rights reserved.
*/

#include "getTime.h"

static int getTimeIsInitialized=0;
static double ticksPerNano=0;
static double ticksPerMicro=0;
static double ticksPerMilli=0;

#if defined(SOLARIS2)
#include <sys/time.h>
TimeValue getTimeNano()
{
	return(gethrtime());
}

TimeValue getTimeMicro()
{
	return(gethrtime()/1000L);
}

TimeValue getTimeMilli()
{
	return(gethrtime()/1000000L);
}

#elif defined(Linux)

#include <time.h>
#include <errno.h>
#include <sys/time.h>

TimeValue getTimeNano()
{
	TimeValue	ret;
	struct timespec	ts;

	if ( clock_gettime(CLOCK_MONOTONIC,&ts) == 0)
	{
		ret = (TimeValue)ts.tv_sec * 1000000000ULL + (TimeValue)ts.tv_nsec;
	}
	else
	{
		struct timeval tv;
		gettimeofday(&tv,0);
		ret = (TimeValue)tv.tv_sec * 1000000000ULL + (TimeValue)tv.tv_usec * 1000ULL;
	}
	return(ret);
}

TimeValue getTimeMicro()
{
	TimeValue	ret;
	struct timespec	ts;

	if ( clock_gettime(CLOCK_MONOTONIC,&ts) == 0)
	{
		ret = (TimeValue)ts.tv_sec * 1000000ULL + (TimeValue)ts.tv_nsec / 1000ULL;
	}
	else
	{
		struct timeval tv;
		gettimeofday(&tv,0);
		ret = (TimeValue)tv.tv_sec * 1000000ULL + (TimeValue)tv.tv_usec;
	}
	return(ret);
}

TimeValue getTimeMilli()
{
	TimeValue	ret;
	struct timespec	ts;

	if ( clock_gettime(CLOCK_MONOTONIC,&ts) == 0)
	{
		ret = (TimeValue)ts.tv_sec * 1000ULL + (TimeValue)ts.tv_nsec / 1000000ULL;
	}
	else
	{
		struct timeval tv;
		gettimeofday(&tv,0);
		ret = (TimeValue)tv.tv_sec * 1000ULL + (TimeValue)tv.tv_usec / 1000ULL;
	}
	return(ret);
}

#else /* Windows */
#include <Windows.h>

static void getTimeInitialize()
{

	if (!getTimeIsInitialized)
	{
		LARGE_INTEGER	m_Freq;

		/* Get the number of clock ticks per second */
		QueryPerformanceFrequency(&m_Freq);

		/* Calculate the clock tick frequencies */
		ticksPerNano = (double)m_Freq.QuadPart/1000000000.0;
		ticksPerMicro = (double)m_Freq.QuadPart/1000000.0;
		ticksPerMilli = (double)m_Freq.QuadPart/1000.0;

		getTimeIsInitialized = 1;
	}
}

TimeValue getTimeNano()
{
	LARGE_INTEGER	m_Counter;
	double			ret;

	if (!getTimeIsInitialized)
		getTimeInitialize();

	if ( !QueryPerformanceCounter(&m_Counter) )
		return( (TimeValue)0 );

	ret = (double)m_Counter.QuadPart / ticksPerNano;
	return( (TimeValue)ret );
}

TimeValue getTimeMicro()
{
	LARGE_INTEGER	m_Counter;
	double			ret;

	if (!getTimeIsInitialized)
		getTimeInitialize();

	if ( !QueryPerformanceCounter(&m_Counter) )
		return( (TimeValue)0 );

	ret = (double)m_Counter.QuadPart / ticksPerMicro;
	return( (TimeValue)ret );
}

TimeValue getTimeMilli()
{
	LARGE_INTEGER	m_Counter;
	double			ret;

	if (!getTimeIsInitialized)
		getTimeInitialize();

	if ( !QueryPerformanceCounter(&m_Counter) )
		return( (TimeValue)0 );

	ret = (double)m_Counter.QuadPart / ticksPerMilli;
	return( (TimeValue)ret );
}

#endif
