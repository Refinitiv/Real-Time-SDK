/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2022-2023 LSEG. All rights reserved.     
 *|-----------------------------------------------------------------------------
 */

using LSEG.Eta.Common;
using LSEG.Eta.Transports;

namespace LSEG.Eta.PerfTools.TransportPerf
{
    /// <summary>
    /// Provides the global configuration for TransportThreads.
    /// </summary>
    internal class TransportThreadConfig
    {
        public const int ALWAYS_SEND_LATENCY_MSG = -1;

        /// <summary>
        /// Gets or sets to control granularity of msg bursts
        /// </summary>
        /// <remarks>
        /// How they must be sized to match the desired msg rate
        /// </remarks>
        public static int TicksPerSec { get;set; }

        /// <summary>
        /// Gets or sets how many messages are packed into a given buffer
        /// </summary>
        public static int TotalBuffersPerPack { get;set; }

        /// <summary>
        /// Gets or sets total msg rate per second(includes latency msgs).
        /// </summary>
        public static int MsgsPerSec { get;set; }

        /// <summary>
        /// Gets or sets total latency msg rate per second.
        /// </summary>
        public static int LatencyMsgsPerSec { get;set; }

        /// <summary>
        /// Gets or sets size of messages to send.
        /// </summary>
        public static int MsgSize { get;set; }

        /// <summary>
        /// Gets or sets flags to use when calling <see cref="IChannel.Write(ITransportBuffer, WriteArgs, out Error)"/>
        /// </summary>
        public static WriteFlags WriteFlags { get; set; }

        /// <summary>
        /// Gets or sets messages per tick. 
        /// </summary>
        public static int MsgsPerTick { get;set; }

        /// <summary>
        /// Gets or sets messages per tick (remainder).
        /// </summary>
        public static int MsgsPerTickRemainder { get;set; }

        /// <summary>
        /// Gets or sets whether ping timeouts should be monitored.
        /// </summary>
        public static bool CheckPings { get; set; } = true;

        /// <summary>
        /// Gets or sets name of the statistics log file.
        /// </summary>
        public static string? StatsFileName { get; set; }

        /// <summary>
        /// Gets or sets whether to log latency information to a file.
        /// </summary>
        public static bool LogLatencyToFile { get; set; }

        /// <summary>
        /// Gets or sets name of the latency log file.
        /// </summary>
        public static string? LatencyLogFileName { get; set; }
    }
}
