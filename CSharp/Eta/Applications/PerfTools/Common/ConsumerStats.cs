/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2023-2024 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

namespace LSEG.Eta.PerfTools.Common
{
	/// <summary>
	/// Maintains counts and other values for measuring statistics on a consumer thread.
	/// </summary>
	public class ConsumerStats
    {
		/// <summary>
		/// Time at which first item request was made
		/// </summary>
		public long ImageRetrievalStartTime { get; set; }
		/// <summary>
		/// Time at which last item refresh was received
		/// </summary>
		public long ImageRetrievalEndTime { get; set; }
		/// <summary>
		/// Time at which steady-state latency started to calculate
		/// </summary>
		public long SteadyStateLatencyTime { get; set; }
		/// <summary>
		/// Time at which first item update was received
		/// </summary>
		public long FirstUpdateTime { get; set; }


		/// <summary>
		/// Number of item refreshes received
		/// </summary>
		public CountStat RefreshCount { get; set; } = new CountStat();
		/// <summary>
		/// Number of Refresh Complete and Data State OK
		/// </summary>
		public CountStat RefreshCompleteCount { get; set; } = new CountStat();
		/// <summary>
		/// Number of item updates received during startup
		/// </summary>
		public CountStat StartupUpdateCount { get; set; } = new CountStat();
		/// <summary>
		/// Number of item updates received during steady state
		/// </summary>
		public CountStat SteadyStateUpdateCount { get; set; } = new CountStat();
		/// <summary>
		/// Number of requests sent
		/// </summary>
		public CountStat RequestCount { get; set; } = new CountStat();
		/// <summary>
		/// Number of item status messages received
		/// </summary>
		public CountStat StatusCount { get; set; } = new CountStat();
		/// <summary>
		/// Number of posts not sent due to lack of buffers
		/// </summary>
		public CountStat PostOutOfBuffersCount { get; set; } = new CountStat();
		/// <summary>
		/// Number of posts sent
		/// </summary>
		public CountStat PostSentCount { get; set; } = new CountStat();
		/// <summary>
		/// Number of generic msgs sent
		/// </summary>
		public CountStat GenMsgSentCount { get; set; } = new CountStat();
		/// <summary>
		/// Number of generic msgs received
		/// </summary>
		public CountStat GenMsgRecvCount { get; set; } = new CountStat();
		/// <summary>
		/// Number of latency generic mesgs sent
		/// </summary>
		public CountStat LatencyGenMsgSentCount { get; set; } = new CountStat();
		/// <summary>
		/// Number of generic msgs not sent due to lack of buffers
		/// </summary>
		public CountStat GenMsgOutOfBuffersCount { get; set; } = new CountStat();


		/// <summary>
		/// Latency statistics (recorded by stats thread)
		/// </summary>
		public ValueStatistics IntervalLatencyStats { get; set; } = new ValueStatistics();
		/// <summary>
		/// Post latency statistics (recorded by stats thread)
		/// </summary>
		public ValueStatistics IntervalPostLatencyStats { get; set; } = new ValueStatistics();
		/// <summary>
		/// Generic msg latency statistics (recorded by stats thread)
		/// </summary>
		public ValueStatistics IntervalGenMsgLatencyStats { get; set; } = new ValueStatistics();
		/// <summary>
		/// Startup latency statistics
		/// </summary>
		public ValueStatistics StartupLatencyStats { get; set; } = new ValueStatistics();
		/// <summary>
		/// Steady-state latency statistics
		/// </summary>
		public ValueStatistics SteadyStateLatencyStats { get; set; } = new ValueStatistics();
		/// <summary>
		///Overall latency statistics
		/// </summary>
		public ValueStatistics OverallLatencyStats { get; set; } = new ValueStatistics();
		/// <summary>
		/// Posting latency statistics
		/// </summary>
		public ValueStatistics PostLatencyStats { get; set; } = new ValueStatistics();
		/// <summary>
		/// Generic msg latency statistics
		/// </summary>
		public ValueStatistics GenMsgLatencyStats { get; set; } = new ValueStatistics();
		/// <summary>
		/// Tunnel Buffer Usage statistics
		/// </summary>
		public ValueStatistics TunnelStreamBufUsageStats { get; set; } = new ValueStatistics();

		/// <summary>
		/// Stats thread sets this once it has recorded/printed this consumer's image retrieval time
		/// </summary>
		public bool ImageTimeRecorded { get; set; }
	}
}
