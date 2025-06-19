/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2016,2019-2020,2024 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

#include "GetTime.h"

using namespace refinitiv::ema::access;

#ifdef WIN32 
#include <Windows.h>
#else 
#include <time.h>
#include <sys/time.h>
#endif
#ifdef WIN32

LARGE_INTEGER _initFreq()
{
	LARGE_INTEGER f;
	QueryPerformanceFrequency( &f );
	return f;
}

const LARGE_INTEGER _FREQ = _initFreq();
#endif

const double GetTime::_TICKS_PER_SECOND = GetTime::_initTicksPerSecond();
const double GetTime::_TICKS_PER_MILLI = GetTime::_initTicksPerMilli();
const double GetTime::_TICKS_PER_MICRO = GetTime::_initTicksPerMicro();
const double GetTime::_TICKS_PER_NANO = GetTime::_initTicksPerNano();

#if defined (Linux) || defined (LINUX)

double GetTime::_initTicksPerSecond()
{
	return 1000000000.0;
}

double GetTime::_initTicksPerMilli()
{
	return 1000000.0;
}

double GetTime::_initTicksPerMicro()
{
	return 1000.0;
}

double GetTime::_initTicksPerNano()
{
	return 1.0;
}

UInt64 GetTime::getSeconds()
{
	UInt64 ret;
	struct timespec	ts;

	if ( clock_gettime( CLOCK_MONOTONIC, &ts ) == 0 )
	{
		ret = (UInt64) ts.tv_sec + (UInt64) ts.tv_nsec / 1000000000ULL;
	}
	else
	{
		struct timeval tv;
		gettimeofday( &tv, 0 );
		ret = (UInt64) tv.tv_sec + (UInt64) tv.tv_usec / 1000000ULL;
	}

	return ret;
}

UInt64 GetTime::getMillis()
{
	UInt64	ret;
	struct timespec	ts;

	if ( clock_gettime( CLOCK_MONOTONIC, &ts ) == 0 )
	{
		ret = (UInt64) ts.tv_sec * 1000ULL + (UInt64) ts.tv_nsec / 1000000ULL;
	}
	else
	{
		struct timeval tv;
		gettimeofday( &tv, 0 );
		ret = (UInt64) tv.tv_sec * 1000ULL + (UInt64) tv.tv_usec / 1000ULL;
	}

	return ret;
}

UInt64 GetTime::getMicros()
{
	UInt64 ret;
	struct timespec	ts;

	if ( clock_gettime( CLOCK_MONOTONIC, &ts ) == 0 )
	{
		ret = (UInt64) ts.tv_sec * 1000000ULL + (UInt64) ts.tv_nsec / 1000ULL;
	}
	else
	{
		struct timeval tv;
		gettimeofday( &tv, 0 );
		ret = (UInt64) tv.tv_sec * 1000000ULL + (UInt64) tv.tv_usec;
	}

	return ret;
}

UInt64 GetTime::getNanos()
{
	UInt64 ret;
	struct timespec	ts;

	if ( clock_gettime( CLOCK_MONOTONIC, &ts ) == 0 )
	{
		ret = (UInt64) ts.tv_sec * 1000000000ULL + (UInt64) ts.tv_nsec;
	}
	else
	{
		struct timeval tv;
		gettimeofday( &tv, 0 );
		ret = (UInt64) tv.tv_sec * 1000000000ULL + (UInt64) tv.tv_usec * 1000ULL;
	}

	return ret;
}

UInt64 GetTime::getTicks()
{
	UInt64 ret;
	struct timespec	ts;

	if ( clock_gettime( CLOCK_MONOTONIC, &ts ) == 0 )
	{
		ret = (UInt64) ts.tv_sec * 1000000000ULL + (UInt64) ts.tv_nsec;
	}
	else
	{
		struct timeval tv;
		gettimeofday( &tv, 0 );
		ret = (UInt64) tv.tv_sec * 1000000000ULL + (UInt64) tv.tv_usec * 1000ULL;
	}

	return ret;
}

double GetTime::ticksPerSecond() { return _TICKS_PER_SECOND; }
double GetTime::ticksPerMilli() { return _TICKS_PER_MILLI; }
double GetTime::ticksPerMicro() { return _TICKS_PER_MICRO; }
double GetTime::ticksPerNano() { return _TICKS_PER_NANO; }

#endif


#ifdef WIN32

double GetTime::_initTicksPerSecond()
{
	return (double) _FREQ.QuadPart;
}

double GetTime::_initTicksPerMilli()
{
	return (double) _FREQ.QuadPart / 1000.0;
}

double GetTime::_initTicksPerMicro()
{
	return (double) _FREQ.QuadPart / 1000000.0;
}

double GetTime::_initTicksPerNano()
{
	return (double) _FREQ.QuadPart / 1000000000.0;
}

UInt64 GetTime::getSeconds()
{
	LARGE_INTEGER m_Counter;
	double ret;

	if ( !QueryPerformanceCounter( &m_Counter ) )
		return( (UInt64) 0 );

	ret = (double) m_Counter.QuadPart / _TICKS_PER_SECOND;
	
	return (UInt64) ret;
}

UInt64 GetTime::getMillis()
{
	LARGE_INTEGER m_Counter;
	double ret;

	if ( !QueryPerformanceCounter( &m_Counter ) )
		return( (UInt64) 0 );
	
	ret = (double) m_Counter.QuadPart / _TICKS_PER_MILLI;
	
	return (UInt64) ret;
}

UInt64 GetTime::getMicros()
{
	LARGE_INTEGER m_Counter;
	double ret;

	if ( !QueryPerformanceCounter( &m_Counter ) )
		return( (UInt64) 0 );
	
	ret = (double) m_Counter.QuadPart / _TICKS_PER_MICRO;
	
	return (UInt64) ret;
}

UInt64 GetTime::getNanos()
{
	LARGE_INTEGER m_Counter;
	double ret;

	if ( !QueryPerformanceCounter( &m_Counter ) )
		return( (UInt64) 0 );
	
	ret = (double) m_Counter.QuadPart / _TICKS_PER_NANO;
	
	return (UInt64) ret;
}

UInt64 GetTime::getTicks()
{
	LARGE_INTEGER m_Counter;

	if ( !QueryPerformanceCounter( &m_Counter ) )
		return( (UInt64) 0 );

	return (UInt64) m_Counter.QuadPart;
}

double GetTime::ticksPerNano()
{
	return _TICKS_PER_NANO;
}

double GetTime::ticksPerMicro()
{
	return _TICKS_PER_MICRO;
}

double GetTime::ticksPerMilli()
{
	return _TICKS_PER_MILLI;
}

double GetTime::ticksPerSecond()
{
	return _TICKS_PER_SECOND;
}

#endif
