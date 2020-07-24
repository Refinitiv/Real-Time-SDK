/*|-----------------------------------------------------------------------------
*|            This source code is provided under the Apache 2.0 license      --
*|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
*|                See the project's LICENSE.md for details.                  --
*|           Copyright (C) 2020 Refinitiv. All rights reserved.            --
*|-----------------------------------------------------------------------------
*/

/**
* @addtogroup RSSLGetTime
* @{
*/

/* These functions provide Nano-second, Mirco-Second and Milli-Second access to timers. 
 * These timers are based on a high-resolution non-decreasing time, not the system clock, and are 
 * not affected by changes to the clock. */

#ifndef _RSSL_GETTIME_H
#define _RSSL_GETTIME_H

#include "rtr/rsslTypes.h"

#ifdef _WIN32
typedef unsigned __int64 RsslTimeValue;
#else
typedef unsigned long long RsslTimeValue;
#endif

#if defined(__cplusplus)
extern "C" {
#endif

/**
 *  @brief Get current time value in CPU ticks. For Operating Systems that support monotonic clock implementations, this is in nanoseconds.
 */
RSSL_API RsslTimeValue rsslGetTicks();

/**
 *  @brief Get current time value in nanoseconds. 
 */
RSSL_API RsslTimeValue rsslGetTimeNano();

/**
 *  @brief Get current time value in microseconds. 
 */
RSSL_API RsslTimeValue rsslGetTimeMicro();

/**
 *  @brief Get current time value in milliseconds. 
 */
RSSL_API RsslTimeValue rsslGetTimeMilli();

/**
 *  @brief Get number of ticks per nanosecond.  This can be used with rsslGetTicks().
 */
RSSL_API RsslDouble rsslGetTicksPerNano();

/**
 *  @brief Get number of ticks per microsecond.  This can be used with rsslGetTicks().
 */ 
RSSL_API RsslDouble rsslGetTicksPerMicro();

/**
 *  @brief Get number of CPU ticks per millisecond. This can be used with rsslGetTicks().
 */
RSSL_API RsslDouble rsslGetTicksPerMilli();

#if defined(__cplusplus)
}
#endif

#endif


