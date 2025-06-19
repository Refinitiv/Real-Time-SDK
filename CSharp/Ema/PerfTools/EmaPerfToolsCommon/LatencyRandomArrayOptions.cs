/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2023-2024 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

namespace LSEG.Ema.PerfTools.Common
{
    public class LatencyRandomArrayOptions
    {
        /// <summary>
        /// Gets or sets total messages sent per second.
        /// </summary>
        public int TotalMsgsPerSec { get; set; }

        /// <summary>
        /// Gets or sets total number of latency messages sent per second.
        /// </summary>
        public int LatencyMsgsPerSec { get; set; }

        /// <summary>
        /// Gets or sets bursts of messages sent per second by the user.
        /// </summary>
        public int TicksPerSec { get; set; }

        /// <summary>
        /// Gets or sets the number of elements in the array. Increasing this
        /// adds more random values to use (each individual array contains one
        /// second's worth of values).
        /// </summary>
        public int ArrayCount { get; set; }

        /// <summary>
        /// Clears the latency random array options.
        /// </summary>
        public void Clear()
        {
            TotalMsgsPerSec = 0;
            LatencyMsgsPerSec = 0;
            TicksPerSec = 0;
            ArrayCount = 0;
        }
    }
}
