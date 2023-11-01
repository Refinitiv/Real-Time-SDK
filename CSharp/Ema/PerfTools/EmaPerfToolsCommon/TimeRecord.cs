/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2023 Refinitiv. All rights reserved.              --
 *|-----------------------------------------------------------------------------
 */

namespace LSEG.Ema.PerfTools.Common
{
    /// <summary>
    /// Stores time information. This class along with <see cref="TimeRecordQueue"/> are
    /// used to collect individual time differences for statistical calculation in a
    /// thread-safe manner -- one thread can store information by adding to the records
    /// queue and another can retrieve the information from the records queue and do any
    /// desired calculation.
    /// </summary>
    public class TimeRecord
    {
        /// <summary>
        /// Gets or sets recorded start time.
        /// </summary>
        public long StartTime { get; set; }

        /// <summary>
        /// Gets or sets recorded end time.
        /// </summary>
        public long EndTime { get; set; }

        /// <summary>
        /// Gets or sets units per microsecond.
        /// </summary>
        public long Ticks { get; set; }
    }
}
