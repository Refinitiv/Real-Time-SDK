/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2024 LSEG. All rights reserved.     
 *|-----------------------------------------------------------------------------
 */

namespace LSEG.Ema.PerfTools.Common
{
    public class ProviderThreadStats
    {
        public const long NOT_DEFINED = -1;
        /// <summary>
        /// The count of Request messages
        /// </summary>
        public CountStat RequestCount { get; set; }
        /// <summary>
        /// The count of Refresh messages
        /// </summary>
        public CountStat RefreshCount { get; set; }

        /// <summary>
        /// The count of Item Refresh messages
        /// </summary>
        public CountStat ItemRefreshCount { get; set; }

        /// <summary>
        /// The count of Update messages
        /// </summary>
        public CountStat UpdateCount { get; set; }
        /// <summary>
        /// The count of Close messages
        /// </summary>
        public CountStat CloseCount { get; set; }
        /// <summary>
        /// The count of Post messages
        /// </summary>
        public CountStat PostCount { get; set; }
        /// <summary>
        /// The count of Status messages
        /// </summary>
        public CountStat StatusCount { get; set; }
        /// <summary>
        /// The number of sent messages
        /// </summary>
        public CountStat MsgSentCount { get; set; }
        /// <summary>
        /// The number of times the provider was out of buffers to send messages
        /// </summary>
        public CountStat OutOfBuffersCount { get; set; }

        /// <summary>
        /// The count of Packed Update messages
        /// </summary>
        public CountStat UpdatePackedMsgCount { get; set; }

        public TimeRecordQueue GenMsgLatencyRecords { get; set; }
        public long InactiveTime { get; set; }
        public long FirstGenMsgSentTime { get; set; }
        public long FirstGenMsgRecvTime { get; set; }
        
        /// <summary>
        /// Number of generic msgs sent.
        /// </summary>
        public CountStat GenMsgSentCount { get; set; }
        /// <summary>
        /// Number of generic msgs received.
        /// </summary>
        public CountStat GenMsgRecvCount { get; set; }
        /// <summary>
        /// Number of latency generic msgs sent.
        /// </summary>
        public CountStat LatencyGenMsgSentCount { get; set; }
        /// <summary>
        /// Generic msg latency statistics (recorded by stats thread).
        /// </summary>
        public ValueStatistics IntervalGenMsgLatencyStats { get; set; }
        /// <summary>
        /// Generic msg latency statistics.
        /// </summary>
        public ValueStatistics GenMsgLatencyStats { get; set; }

        public ProviderThreadStats()
        {
            FirstGenMsgRecvTime = NOT_DEFINED;
            FirstGenMsgSentTime = NOT_DEFINED;

            GenMsgSentCount = new CountStat();
            GenMsgRecvCount = new CountStat();
            LatencyGenMsgSentCount = new CountStat();
            IntervalGenMsgLatencyStats = new ValueStatistics();
            GenMsgLatencyStats = new ValueStatistics();
            IntervalGenMsgLatencyStats.Clear();
            GenMsgLatencyStats.Clear();

            RequestCount = new CountStat();
            RefreshCount = new CountStat();
            ItemRefreshCount = new CountStat();
            UpdateCount = new CountStat();
            CloseCount = new CountStat();
            PostCount = new CountStat();
            StatusCount = new CountStat();
            MsgSentCount = new CountStat();
            OutOfBuffersCount = new CountStat();
            GenMsgLatencyRecords = new TimeRecordQueue();
            UpdatePackedMsgCount = new CountStat();
        }

        public void TimeRecordSubmit(long startTime, long endTime, long ticks)
        {
            GenMsgLatencyRecords.RecordTracker.SubmitTimeRecord(startTime, endTime, ticks);
        }
    }
}
