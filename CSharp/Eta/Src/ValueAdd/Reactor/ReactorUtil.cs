/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2023-2026 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

using System.Diagnostics;

namespace LSEG.Eta.ValueAdd.Reactor
{
    internal static class ReactorUtil
    {
        private static readonly DateTime _initialDT;
        private static readonly long _initialTimeMs;

        static ReactorUtil()
        {
            TicksPerSecond = Stopwatch.Frequency;
            TicksPerMilliSecond = Stopwatch.Frequency / 1000.0;

            // Attempt to pricisely map current date time with system timer timestamp
            // using average timestamp value between before & after DateTime.Now
            var nowMs = GetCurrentTimeMilliSecond();
            _initialDT = DateTime.Now;
            _initialTimeMs = nowMs + (GetCurrentTimeMilliSecond() - nowMs) / 2;
        }

        public static double TicksPerSecond { get; private set; }

        public static double TicksPerMilliSecond { get; private set; }

        public static long GetCurrentTimeMilliSecond() => (long)(Stopwatch.GetTimestamp() / TicksPerMilliSecond);

        public static long GetCurrentTimeSecond() => (long)(Stopwatch.GetTimestamp() / TicksPerSecond);

        /// <summary>
        /// Converts given <paramref name="timeMs"/> from milliseconds to <see cref="DateTime"/> value.
        /// </summary>
        /// <param name="timeMs">Time in milliseconds retrieved from <see cref="GetCurrentTimeMilliSecond"/> or derived from that value.</param>
        /// <returns>Returns date time value corresponding to passed millisecond time.</returns>
        /// <remarks>
        /// Simple <see cref="DateTime.DateTime(long)"/> cannot be used for conversion of values from <see cref="GetCurrentTimeMilliSecond"/>.
        /// Because <see cref="GetCurrentTimeMilliSecond"/> uses high resolution timer which at least on Windows returns time relative to system start instead of Unix epoch required by <see cref="DateTime.DateTime(long)"/> .
        /// </remarks>
        public static DateTime ConvertMilliSecondTimeToDateTime(long timeMs) => _initialDT.AddMilliseconds(timeMs - _initialTimeMs);

        /// <summary>
        /// Converts given <paramref name="dateTime"/> from <see cref="DateTime"/> to milliseconds value.
        /// </summary>
        /// <param name="dateTime">local date time to convert to milliseconds.</param>
        /// <returns>Returns time in milliseconds similar to <see cref="GetCurrentTimeMilliSecond"/>.</returns>
        /// <remarks>See <see cref="ConvertMilliSecondTimeToDateTime(long)"/> remarks for further explanation.</remarks>
        public static long ConvertDateTimeToMilliSecondTime(DateTime dateTime) => (long)Math.Round(_initialTimeMs + (dateTime - _initialDT).TotalMilliseconds, MidpointRounding.AwayFromZero);
    }
}
