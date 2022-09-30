/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2022 Refinitiv. All rights reserved.              --
 *|-----------------------------------------------------------------------------
 */

using System.Diagnostics;

namespace Refinitiv.Eta.PerfTools.Common
{
    public static class GetTime
    {
        static GetTime()
        {
            TicksPerSecond = Stopwatch.Frequency;
            TicksPerMillisecond = Stopwatch.Frequency / 1000.0;
            TicksPerMicrosecond = Stopwatch.Frequency / 1000000.0;
            TicksPerNanosecond = Stopwatch.Frequency / 1000000000.0;
        }
        
        /// <summary>
        /// Gets number of ticks per second.
        /// </summary>
        public static double TicksPerSecond { get; private set; }

        /// <summary>
        /// Gets number of ticks per millisecond
        /// </summary>
        public static double TicksPerMillisecond { get; private set; }

        /// <summary>
        /// Gets number of ticks per microsecond
        /// </summary>
        public static double TicksPerMicrosecond { get; private set; }

        /// <summary>
        /// Gets number of ticks per nanosecond
        /// </summary>
        public static double TicksPerNanosecond { get; private set; }

        /// <summary>
        /// Gets the current value of time in seconds
        /// </summary>
        /// <returns>The current time value in seconds</returns>
        public static double GetSeconds()
        {
            return Stopwatch.GetTimestamp() / TicksPerSecond;
        }

        /// <summary>
        /// Gets the current value of time in milliseconds
        /// </summary>
        /// <returns>The current time value in milliseconds</returns>
        public static double GetMilliseconds()
        {
            return Stopwatch.GetTimestamp() / TicksPerMillisecond;
        }

        /// <summary>
        /// Gets the current value of time in microseconds
        /// </summary>
        /// <returns>The current time value in microseconds</returns>
        public static double GetMicroseconds()
        {
            return Stopwatch.GetTimestamp() / TicksPerMicrosecond;
        }

        /// <summary>
        /// Gets the current value of time in nanoseconds
        /// </summary>
        /// <returns>The current time value in nanoseconds</returns>
        public static double GetNanoseconds()
        {
            return Stopwatch.GetTimestamp() / TicksPerNanosecond;
        }

        /// <summary>
        /// Gets the current number of ticks
        /// </summary>
        /// <returns>A long integer representing the tick counter value</returns>
        public static long GetTicks()
        {
            return Stopwatch.GetTimestamp();
        }
    }
}
