/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2022 Refinitiv. All rights reserved.              --
 *|-----------------------------------------------------------------------------
 */

namespace Refinitiv.Eta.PerfTools.Common
{
    /// <summary>
    /// Statistics associated with a ProviderThread
    /// </summary>
    public class ProviderThreadStats
    {
        public long InactiveTime { get; set; }
        public long FirstGenMsgSentTime { get; set; }
        public long FirstGenMsgRecvTime { get; set; }
        /// <summary>
        /// Number of generic msgs sent
        /// </summary>
        public CountStat GenMsgSentCount { get; set; } = new CountStat();
        /// <summary>
        /// Number of generic msgs received
        /// </summary>
        public CountStat GenMsgRecvCount { get; set; } = new CountStat();
        /// <summary>
        /// Number of latency generic msgs sent
        /// </summary>
        public CountStat LatencyGenMsgSentCount { get; set; } = new CountStat();
        /// <summary>
        /// Generic msg latency statistics (recorded by stats thread)
        /// </summary>
        public ValueStatistics IntervalGenMsgLatencyStats { get; set; } = new ValueStatistics();
        /// <summary>
        /// Generic msg latency statistics
        /// </summary>
        public ValueStatistics GenMsgLatencyStats { get; set; } = new ValueStatistics(); 

        public ValueStatistics RefreshBufLenStats { get; set; } = new ValueStatistics();
        public ValueStatistics UpdateBufLenStats { get; set; } = new ValueStatistics();
        public ValueStatistics GenMsgBufLenStats { get; set; } = new ValueStatistics();

        public ProviderThreadStats()
        {
            IntervalGenMsgLatencyStats.Clear();
            GenMsgLatencyStats.Clear();
            RefreshBufLenStats.Clear();
            UpdateBufLenStats.Clear();
            GenMsgBufLenStats.Clear();
        }
    }
}
