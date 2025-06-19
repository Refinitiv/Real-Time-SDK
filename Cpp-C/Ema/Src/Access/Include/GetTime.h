/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2020,2024 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

#ifndef __refinitiv_ema_access_GetTime_h
#define __refinitiv_ema_access_GetTime_h

/**
	@class refinitiv::ema::access::GetTime GetTime.h "Access/Include/GetTime.h"
	@brief GetTime class encapsulates functionality for efficiently obtaining relative time in various standard units of time.

	Interface for efficiently obtaining relative time in various standard units of time.
	Interfaces providing units of seconds, milli seconds, micro seconds, and nano seconds are provided.
	Depending on the platform, the implementation uses clock_gettime(), if available otherwise gettimeofday(), on Linux,
	and QueryPerformanceCounter() on Windows.

	Note that using the nano seconds interface does not necessarily provide nano second resolution or granularity.
	The actual resolution will vary depending on platform and other hardware variables.
	An interface for units of ticks is also provided and gives the time in the smallest units of time and
	the highest resolution available for any given platform.
	However, the actual resolution for any interface can only be determined empirically.

	Interfaces for obtaining the number of ticks per standard unit of time are provided.
	Interfaces for obtaining the absolute time in standard units from two relative times in ticks are provided.

	Note taking the inverse of the number of ticks per standard unit of time can be used to
	determine the upper bound of the actual resolution in the same units of time.

*/

#include "Common.h"

namespace refinitiv {

namespace ema {

namespace access {

/**
	\typedef TimeValue
	@brief represents TimeValue as a 64-bit unsigned integer
*/
typedef UInt64 TimeValue;

class EMA_ACCESS_API GetTime
{
public :

	///@name Accessors
	//@{
	/** Retrieve current time value in seconds.
		@return current time value in seconds
	*/
	static TimeValue getSeconds();

	//@{
	/** Retrieve current time value in milliseconds.
		@return current time value in milliseconds
	*/
	static TimeValue getMillis();

	//@{
	/** Retrieve current time value in microseconds.
		@return current time value in microseconds
	*/
	static TimeValue getMicros();

	//@{
	/** Retrieve current time value in nanoseconds.
		@return current time value in nanoseconds
	*/
	static TimeValue getNanos();

	//@{
	/** Retrieve current time value in CPU ticks. For Operating Systems that support monotonic clock implementations, this is in nanoseconds.
		@return current time value in CPU ticks
	*/
	static TimeValue getTicks();

	//@{
	/** Retrieve number of ticks per second.  This can be used with GetTime::getTicks().
		@return number of ticks per second
	*/
	static double ticksPerSecond();

	//@{
	/** Retrieve number of ticks per millisecond.  This can be used with GetTime::getTicks().
		@return number of ticks per millisecond
	*/
	static double ticksPerMilli();

	//@{
	/** Retrieve number of ticks per microsecond.  This can be used with GetTime::getTicks().
		@return number of ticks per microsecond
	*/
	static double ticksPerMicro();

	//@{
	/** Retrieve number of ticks per nanosecond.  This can be used with GetTime::getTicks().
		@return number of ticks per nanosecond
	*/
	static double ticksPerNano();
	//@}

private:

	static double			_initTicksPerSecond();
	static double			_initTicksPerMilli();
	static double			_initTicksPerMicro();
	static double			_initTicksPerNano();

	static const double		_TICKS_PER_SECOND;
	static const double		_TICKS_PER_MILLI;
	static const double		_TICKS_PER_MICRO;
	static const double		_TICKS_PER_NANO;
};

}

}

}

#endif // __refinitiv_ema_access_GetTime_h

