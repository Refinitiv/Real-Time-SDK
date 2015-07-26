/*
 * This source code is provided under the Apache 2.0 license and is provided
 * AS IS with no warranty or guarantee of fit for purpose.  See the project's 
 * LICENSE.md for details. 
 * Copyright Thomson Reuters 2015. All rights reserved.
*/

/*
//
//  author:        Robert Bonaguro
//  created:       September 30, 2004
//  version:       1.0
//
//  description:
//	These functions provide Nano-second, Mirco-Second and
//  Milli-Second access to timers. These timers are not
//  based on the system clock and are therefore not adjusted
//  when the system time is changed.
//
*/
#ifdef LINUX

#ifndef __rssl_gettime_h
#define __rssl_gettime_h

#ifdef _WIN32
typedef __int64 rssl_time_value;
#else
typedef long long rssl_time_value;
#endif

#if defined(__cplusplus)
extern "C" {
#endif

	/* These functions get the time. They can be used
	 * across processes on the same machine since they
	 * are relative to the cpu clock ticks. These
	 * times are not affected by changes in the system
	 * time.
	 */
extern rssl_time_value rssl_gettime_nano();
extern rssl_time_value rssl_gettime_micro();
extern rssl_time_value rssl_gettime_milli();


	/* These functions get the time relative to the time
	 * any of the functions was first called. They should
	 * be used for timing within a process since they
	 * can only make sense within the process. The main
	 * goal is to cut down on the size of the returned
	 * values since we are substracting each actual
	 * time from some base time. These functions
	 * are all based on the above time which are based
	 * on clock ticks.
	 */
extern rssl_time_value rtr_getreltime_nano();
extern rssl_time_value rtr_getreltime_micro();
extern rssl_time_value rtr_getreltime_milli();


	/* These functions return the number of clock ticks
	 * per time interval (nano, micro & milli). They can
	 * be used across processes on the same machine since
	 * they are relative to the cpu clock ticks.
	 */
extern double rssl_getticks_per_nano();
extern double rssl_getticks_per_micro();
extern double rssl_getticks_per_milli();


	/* This function returns the current number of clock
	 * ticks since some arbitrary time in the past. Use
	 * along with rssl_getticks_per_*() to convert the
	 * clock ticks into a time value. e.g.
	 * rssl_gettime_micro() = rssl_getticks() / rssl_getticks_per_micro()
	 * Splitting up the tick count from tick frequency 
	 * allows for better performance by removing the division.
	 */
extern rssl_time_value rssl_getticks();


#if defined(__cplusplus)
}
#endif

#endif

#endif

