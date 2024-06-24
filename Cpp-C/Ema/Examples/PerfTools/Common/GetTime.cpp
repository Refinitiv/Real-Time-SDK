///*|-----------------------------------------------------------------------------
// *|            This source code is provided under the Apache 2.0 license
// *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
// *|                See the project's LICENSE.md for details.
// *|           Copyright (C) 2019 LSEG. All rights reserved.                 --
///*|-----------------------------------------------------------------------------

#include "GetTime.h"

using namespace perftool::common;

#if defined(_WIN32)
	#include <Windows.h>
	// Initialize Windows specific static variable
PerfLargeInteger GetTime::FREQ = {0, 0};
bool GetTime::IsTimeInitialized = false;
void GetTime::initFreq()
{
	QueryPerformanceFrequency(&GetTime::FREQ);
	GetTime::IsTimeInitialized = true;
}

#endif // defined(_WIN32)

// Initialize static members
const double GetTime::TICKS_PER_SECOND = GetTime::initTicksPerSecond();
const double GetTime::TICKS_PER_MILLI  = GetTime::initTicksPerMilli();
const double GetTime::TICKS_PER_MICRO  = GetTime::initTicksPerMicro();
const double GetTime::TICKS_PER_NANO   = GetTime::initTicksPerNano();


// Use the get high resolution time function call.
// This call uses the high resolution timer which is the non-adjustable high-res clock for the system.
// It is not in any way correlated to the time of day.

// Use the clock_gettime to access the clock.
// It is not in any way corrleted to the time of day.
#if defined(Linux) || defined(LINUX)

#include <time.h>
#include <errno.h>
#include <sys/time.h>

double GetTime::initTicksPerSecond()
{
	return 1000000000.0;
}

double GetTime::initTicksPerMilli()
{
	return 1000000.0;
}

double GetTime::initTicksPerMicro()
{
	return 1000.0;
}

double GetTime::initTicksPerNano()
{
	return 1.0;
}

PerfTimeValue GetTime::getTimeSeconds()
{
	PerfTimeValue	ret;
	struct timespec	ts;

	if ( clock_gettime(CLOCK_MONOTONIC,&ts) == 0)
	{
		ret = (PerfTimeValue)ts.tv_sec + (PerfTimeValue)ts.tv_nsec / 1000000000ULL;
	}
	else
	{
		struct timeval tv;
		gettimeofday(&tv,0);
		ret = (PerfTimeValue)tv.tv_sec + (PerfTimeValue)tv.tv_usec / 1000000ULL;
	}
	return(ret);
}

PerfTimeValue GetTime::getTimeMilli()
{
	PerfTimeValue	ret;
	struct timespec	ts;

	if ( clock_gettime(CLOCK_MONOTONIC,&ts) == 0)
	{
		ret = (PerfTimeValue)ts.tv_sec * 1000ULL + (PerfTimeValue)ts.tv_nsec / 1000000ULL;
	}
	else
	{
		struct timeval tv;
		gettimeofday(&tv,0);
		ret = (PerfTimeValue)tv.tv_sec * 1000ULL + (PerfTimeValue)tv.tv_usec / 1000ULL;
	}
	return(ret);
}

PerfTimeValue GetTime::getTimeMicro()
{
	PerfTimeValue	ret;
	struct timespec	ts;

	if ( clock_gettime(CLOCK_MONOTONIC,&ts) == 0)
	{
		ret = (PerfTimeValue)ts.tv_sec * 1000000ULL + (PerfTimeValue)ts.tv_nsec / 1000ULL;
	}
	else
	{
		struct timeval tv;
		gettimeofday(&tv,0);
		ret = (PerfTimeValue)tv.tv_sec * 1000000ULL + (PerfTimeValue)tv.tv_usec;
	}
	return(ret);
}

PerfTimeValue GetTime::getTimeNano()
{
	PerfTimeValue	ret;
	struct timespec	ts;

	if ( clock_gettime(CLOCK_MONOTONIC,&ts) == 0)
	{
		ret = (PerfTimeValue)ts.tv_sec * 1000000000ULL + (PerfTimeValue)ts.tv_nsec;
	}
	else
	{
		struct timeval tv;
		gettimeofday(&tv,0);
		ret = (PerfTimeValue)tv.tv_sec * 1000000000ULL + (PerfTimeValue)tv.tv_usec * 1000ULL;
	}
	return(ret);
}

TICKS GetTime::getTicks()
{
	TICKS	ret;
	struct timespec	ts;

	if(clock_gettime(CLOCK_MONOTONIC,&ts) == 0)
	{
		ret = (TICKS)ts.tv_sec * 1000000000ULL + (TICKS)ts.tv_nsec;
	}
	else
	{
		struct timeval tv;
		gettimeofday(&tv,0);
		ret = (TICKS)tv.tv_sec * 1000000000ULL + (TICKS)tv.tv_usec * 1000ULL;
	}
	return(ret);
}

double GetTime::ticksPerSecond(){ return TICKS_PER_SECOND; }
double GetTime::ticksPerMilli()	{ return TICKS_PER_MILLI; }
double GetTime::ticksPerMicro()	{ return TICKS_PER_MICRO; }
double GetTime::ticksPerNano()	{ return TICKS_PER_NANO; }

#endif // defined(Linux) || defined(LINUX)


// Use the cpu clock tick and cpu frequency to calculate the time.
// It is not in any way correlated to the time of day.
#if defined(_WIN32)

double GetTime::initTicksPerSecond()
{
	if(!GetTime::IsTimeInitialized)
		GetTime::initFreq();

	return (double) FREQ.QuadPart;
}

double GetTime::initTicksPerMilli()
{
	if(!GetTime::IsTimeInitialized)
		GetTime::initFreq();

	return (double) FREQ.QuadPart/1000.0;
}

double GetTime::initTicksPerMicro()
{
	if(!GetTime::IsTimeInitialized)
		GetTime::initFreq();

	return (double) FREQ.QuadPart/1000000.0;
}

double GetTime::initTicksPerNano()
{
	if(!GetTime::IsTimeInitialized)
		GetTime::initFreq();

	return (double) FREQ.QuadPart/1000000000.0;
}

PerfTimeValue GetTime::getTimeSeconds()
{
	LARGE_INTEGER	m_Counter;
	double			ret;
	if ( !QueryPerformanceCounter(&m_Counter) )
		return( (PerfTimeValue)0 );
	ret = (double)m_Counter.QuadPart / TICKS_PER_SECOND;
	return( (PerfTimeValue)ret );
}

PerfTimeValue GetTime::getTimeMilli()
{
	LARGE_INTEGER	m_Counter;
	double			ret;
	if ( !QueryPerformanceCounter(&m_Counter) )
		return( (PerfTimeValue)0 );
	ret = (double)m_Counter.QuadPart / TICKS_PER_MILLI;
	return( (PerfTimeValue)ret );
}

PerfTimeValue GetTime::getTimeMicro()
{
	LARGE_INTEGER	m_Counter;
	double			ret;
	if ( !QueryPerformanceCounter(&m_Counter) )
		return( (PerfTimeValue)0 );
	ret = (double)m_Counter.QuadPart / TICKS_PER_MICRO;
	return( (PerfTimeValue)ret );
}

PerfTimeValue GetTime::getTimeNano()
{
	LARGE_INTEGER	m_Counter;
	double			ret;
	if ( !QueryPerformanceCounter(&m_Counter) )
		return( (PerfTimeValue)0 );
	ret = (double)m_Counter.QuadPart / TICKS_PER_NANO;
	return( (PerfTimeValue)ret );
}

TICKS GetTime::getTicks()
{
	LARGE_INTEGER	m_Counter;
	if ( !QueryPerformanceCounter(&m_Counter) )
		return( (TICKS)0 );
	return( (TICKS)m_Counter.QuadPart );
}

double GetTime::ticksPerNano()
{
	return TICKS_PER_NANO;
}

double GetTime::ticksPerMicro()
{
	return TICKS_PER_MICRO;
}

double GetTime::ticksPerMilli()
{
	return TICKS_PER_MILLI;
}

double GetTime::ticksPerSecond()
{
	return TICKS_PER_SECOND;
}

#endif // defined(_WIN32)
