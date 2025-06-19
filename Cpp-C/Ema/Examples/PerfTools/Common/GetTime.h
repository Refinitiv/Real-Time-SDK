/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2016,2019-2020,2024 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

#ifndef _GETTIME_H
#define _GETTIME_H

// Interface for efficiently obtaining relative time in various standard units of time.
// Interfaces providing units of seconds, milli seconds, micro seconds, and nano seconds are provided.
// Depending on the platform, the implementation uses clock_gettime(), if available otherwise gettimeofday(), on Linux, 
// and QueryPerformanceCounter() on Windows.

// Note that using the nano seconds interface does not necessarily provide nano second resolution or granularity.
// The actual resolution will vary depending on platform and other hardware variables.
// An interface for units of ticks is also provided and gives the time in the smallest units of time and 
// the highest resolution available for any given platform.
// However, the actual resolution for any interface can only be determined empirically.

// Interfaces for obtaining the number of ticks per standard unit of time are provided.
// Interfaces for obtaining the absolute time in standard units from two relative times in ticks are provided.

// Note taking the inverse of the number of ticks per standard unit of time can be used to 
// determine the upper bound of the actual resolution in the same units of time.

#include "Ema.h"
#if defined(WIN32)
#include <Windows.h>
#define PerfLargeInteger LARGE_INTEGER
#else
#define PerfLargeInteger  refinitiv::ema::access::Int64
#endif

typedef refinitiv::ema::access::UInt64 PerfTimeValue;
typedef refinitiv::ema::access::UInt64 TICKS;

namespace perftool {

namespace common {

class GetTime
{
public:

	static PerfTimeValue getTimeSeconds();
	static PerfTimeValue  getTimeMilli();
	static PerfTimeValue  getTimeMicro();
	static PerfTimeValue   getTimeNano();
	static PerfTimeValue   getTicks();

	static double ticksPerSecond();
	static double ticksPerMilli();
	static double ticksPerMicro();
	static double ticksPerNano();

	static double diffSeconds(TICKS t1, TICKS t2) {return (t2-t1) / ticksPerSecond();}
	static double diffMillis(TICKS t1, TICKS t2)  {return (t2-t1) / ticksPerMilli();}
	static double diffMicros(TICKS t1, TICKS t2)  {return (t2-t1) / ticksPerMicro();}
	static double diffNanos(TICKS t1, TICKS t2)   {return (t2-t1) / ticksPerNano();}
	static TICKS  diffTicks(TICKS t1, TICKS t2)   {return (t2-t1);}
	static void initFreq();
	static bool getTimeIsInitialized();
private:
	static double initTicksPerSecond();
	static double initTicksPerMilli();
	static double initTicksPerMicro();
	static double initTicksPerNano();

	static const double TICKS_PER_SECOND;
	static const double TICKS_PER_MILLI;
	static const double TICKS_PER_MICRO;
	static const double TICKS_PER_NANO;
	static PerfLargeInteger FREQ;
	static bool IsTimeInitialized;
};


inline bool GetTime::getTimeIsInitialized()
{
	return GetTime::IsTimeInitialized;
}

} // common

} // perftool

#endif //_GETTIME_H

