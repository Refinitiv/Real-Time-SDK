/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2023 Refinitiv. All rights reserved.              --
 *|-----------------------------------------------------------------------------
 */

using System.Diagnostics;

namespace LSEG.Ema.Access
{
    /// <summary>
    /// Provides utility functions for EMA library.
    /// </summary>
    internal static class EmaUtil
    {
        static EmaUtil()
        {
            TicksPerSecond = Stopwatch.Frequency;
            TicksPerMillisecond = Stopwatch.Frequency / 1000.0;
            TicksPerMicrosecond = Stopwatch.Frequency / 1000000.0;
            TicksPerNanosecond = Stopwatch.Frequency / 1000000000.0;
        }

        private static double TicksPerSecond { get;  set; }

        private static double TicksPerMillisecond { get;  set; }

        private static double TicksPerMicrosecond { get;  set; }

        private static double TicksPerNanosecond { get;  set; }

        /// <summary>
        /// Gets the current value of time in seconds
        /// </summary>
        /// <returns>The current time value in seconds</returns>
        public static long GetSeconds()
        {
            return (long)(Stopwatch.GetTimestamp() / TicksPerSecond);
        }

        /// <summary>
        /// Gets the current value of time in milliseconds
        /// </summary>
        /// <returns>The current time value in milliseconds</returns>
        public static long GetMilliseconds()
        {
            return (long)(Stopwatch.GetTimestamp() / TicksPerMillisecond);
        }

        /// <summary>
        /// Gets the current value of time in microseconds
        /// </summary>
        /// <returns>The current time value in microseconds</returns>
        public static long GetMicroseconds()
        {
            return (long)(Stopwatch.GetTimestamp() / TicksPerMicrosecond);
        }

        /// <summary>
        /// Gets the current value of time in nanoseconds
        /// </summary>
        /// <returns>The current time value in nanoseconds</returns>
        public static long GetNanoseconds()
        {
            return (long)(Stopwatch.GetTimestamp() / TicksPerNanosecond);
        }
    }
}
