/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2023-2024 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

using LSEG.Ema.PerfTools.Common;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace LSEG.Ema.PerfTools.ConsPerf
{
    public class ConsumerThreadInfo
    {
        /// <summary>
        /// ID saved from thread creation.
        /// </summary>
        public long ThreadId;

        /// <summary>
        /// Queue of timestamp information, collected periodically by the Main thread.
        /// </summary>
        public TimeRecordQueue LatencyRecords;

        /// <summary>
        /// Queue of timestamp information(for posts), collected periodically by the Main thread.
        /// </summary>
        public TimeRecordQueue PostLatencyRecords;

        /// <summary>
        /// Queue of timestamp information(for generic messages), collected periodically by the Main thread.
        /// </summary>
        public TimeRecordQueue GenMsgLatencyRecords;

        /// <summary>
        /// Index into the item list at which item requests unique to this consumer start.
        /// </summary>
        public int ItemListUniqueIndex;

        /// <summary>
        /// Number of item requests to make.
        /// </summary>
        public int ItemListCount;

        /// <summary>
        /// Other stats, collected periodically by the Main thread.
        /// </summary>
        public ConsumerStats? Stats;

        /// <summary>
        /// File for logging stats for this connection.
        /// </summary>
        public string? StatsFile;

        /// <summary>
        /// File writer for logging stats for this connection.
        /// </summary>
        public StreamWriter? StatsFileWriter;

        /// <summary>
        /// File for logging latency for this connection.
        /// </summary>
        public string? LatencyLogFile;

        /// <summary>
        /// File writer for logging latency for this connection.
        /// </summary>
        public StreamWriter? LatencyLogFileWriter;

        /// <summary>
        /// Signals thread to Shutdown.
        /// </summary>
        public volatile bool Shutdown;

        /// <summary>
        /// Acknowledges thread is Shutdown.
        /// </summary>
        public volatile bool ShutdownAck;

        public ConsumerThreadInfo()
        {
            LatencyRecords = new TimeRecordQueue();
            PostLatencyRecords = new TimeRecordQueue();
            GenMsgLatencyRecords = new TimeRecordQueue();
            Stats = new ConsumerStats();
        }


        /// <summary>
        /// Submit a time record.
        /// </summary>
        /// <param name="recordQueue">the record queue</param>
        /// <param name="startTime">the start time</param>
        /// <param name="endTime">the end time</param>
        /// <param name="ticks">the ticks</param>
        /// <returns></returns>
        public int TimeRecordSubmit(TimeRecordQueue recordQueue, long startTime, long endTime, long ticks)
        {
            recordQueue.RecordTracker.SubmitTimeRecord(startTime, endTime, ticks);

            return 0;
        }

        public void Cleanup()
        {
            LatencyRecords.Cleanup();
            LatencyRecords.Cleanup();
            GenMsgLatencyRecords.Cleanup();
        }
    }
}
