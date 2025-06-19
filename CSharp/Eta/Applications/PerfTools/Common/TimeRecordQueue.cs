/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2023-2024 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

using System.Collections.Concurrent;

namespace LSEG.Eta.PerfTools.Common
{
    /// <summary>
    /// Stores a queue and pool of TimeRecord objects. This class along with
    /// <see cref="TimeRecord"/> are used to collect individual time differences
    /// for statistical calculation in a thread-safe manner -- one thread can
    /// store information by adding to the records queue and another can retrieve
    /// the information from the records queue and do any desired calculation.
    /// </summary>
    public class TimeRecordQueue
    {
        /// <summary>
        /// Gets queue of submitted TimeRecord objects.
        /// </summary>
        public ConcurrentQueue<TimeRecord> Pools { get; private set; } = new ConcurrentQueue<TimeRecord>();

        /// <summary>
        /// Gets queue of submitted TimeRecord objects.
        /// </summary>
        public ConcurrentQueue<TimeRecord> Records { get; private set; } = new ConcurrentQueue<TimeRecord>();

        public TimeRecordQueue()
        {
            for (int i = 0; i < 1000; i++)
            {
                Pools.Enqueue(new TimeRecord());
            }
        }

        /// <summary>
        /// Cleans up TimeRecordQueue.
        /// </summary>
        public void Cleanup()
        {
            while (!Records.IsEmpty)
            {
                if (Records.TryDequeue(out TimeRecord? rec))
                {
                    Pools.Enqueue(rec);
                }
            }
        }
    }
}
