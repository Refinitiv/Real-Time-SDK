/*
 * This source code is provided under the Apache 2.0 license and is provided
 * AS IS with no warranty or guarantee of fit for purpose.  See the project's 
 * LICENSE.md for details. 
 * Copyright Thomson Reuters 2015. All rights reserved.
*/

/* getTime.h
 * These functions provide Nano-second, Mirco-Second and Milli-Second access to timers. 
 * These timers are not based on a high-resolution non-decreasing time, not the system clock, and are 
 * not affected by changes to the clock. */

#ifndef _GETTIME_H
#define _GETTIME_H

#ifdef _WIN32
typedef unsigned __int64 TimeValue;
#else
typedef unsigned long long TimeValue;
#endif

#if defined(__cplusplus)
extern "C" {
#endif

/* Get current time value in nanoseconds. */
TimeValue getTimeNano();

/* Get current time value in microseconds. */
TimeValue getTimeMicro();

/* Get current time value in milliseconds. */
TimeValue getTimeMilli();

#if defined(__cplusplus)
}
#endif

#endif


