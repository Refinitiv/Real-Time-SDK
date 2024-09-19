/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2024 LSEG. All rights reserved.     
 *|-----------------------------------------------------------------------------
 */

using LSEG.Ema.PerfTools.Common;

namespace LSEG.Ema.PerfTools.EMA_NiProvPerf
{
    public class EmaNiProviderPerfConfig : BaseProviderPerfConfig
    {
        private const string DEFAULT_PERF_PROVIDER_CONFIG_PREFIX = "Perf_NIProvider_";
        private const string DEFAULT_PERF_NIPROVIDER_SUMMARY_FILENAME = "NIProvSummary.out";
        private const string DEFAULT_PERF_NIPROVIDER_STATS_FILENAME = "NIProvStats";

        /// <summary>
        /// Number of items to request. See -itemCount.
        /// </summary>
        public int ItemRequestCount { get; internal set; }
        /// <summary>
        /// Number of items common to all providers, if using multiple connections.
        /// </summary>
        public int CommonItemCount { get; internal set; }
        /// <summary>
        /// Service ID which used by Non-Interactive provider
        /// </summary>
        public int ServiceId { get; internal set; }
        /// <summary>
        /// Service name which used by Non-Interactive provider
        /// </summary>
        public string? ServiceName { get; internal set; }
        /// <summary>
        /// Controls that application should use specified ServiceId
        /// </summary>
        public bool UseServiceId { get; internal set; }
        /// <summary>
        /// The name of the file that has Item information
        /// </summary>
        public string? ItemFilename { get; internal set; }

        public EmaNiProviderPerfConfig()
        {
            CommandLine.ProgName("NiProvPerf");
            CommandLine.AddOption("latencyUpdateRate", 10, "Latency update rate per second (can specify \"all\" to send latency in every update");
            CommandLine.AddOption("itemCount", 100000, "Number of items to publish non-interactively");
            CommandLine.AddOption("commonItemCount", 0, "Number of items common to all providers, if using multiple connections");
            CommandLine.AddOption("serviceName", "NI_PUB", "Name of the provided service");
            CommandLine.AddOption("serviceId", 1, "ID of the provided service");
            CommandLine.AddOption("useServiceId", false, "Flag defining whether the ServiceId should be used");
            CommandLine.AddOption("itemFile", "350k.xml", "Name of the file to get items from");
        }

        protected override void FillProperties()
        {
            base.FillProperties();
            ItemRequestCount = CommandLine.IntValue("itemCount");
            CommonItemCount = CommandLine.IntValue("commonItemCount");
            ServiceId = CommandLine.IntValue("serviceId");
            ServiceName = CommandLine.Value("serviceName");
            UseServiceId = CommandLine.BoolValue("useServiceId");
            ItemFilename = CommandLine.Value("itemFile");
        }

        protected override void CreateConfigString()
        {
            m_ConfigString = "--- TEST INPUTS ---\n\n" +
                    "                Run Time: " + RunTime + "\n" +
                    "         UseUserDispatch: " + (UseUserDispatch ? "Yes" : "No") + "\n" +
                    "                 Threads: " + ThreadCount + "\n" +
                    "            Summary File: " + SummaryFilename + "\n" +
                    "    Write Stats Interval: " + WriteStatsInterval + "\n" +
                    "           Display Stats: " + (DisplayStats ? "Yes" : "No") + "\n" +
                    "               Tick Rate: " + TicksPerSec + "\n" +
                    "             Update Rate: " + UpdatesPerSec + "\n" +
                    "     Latency Update Rate: " + LatencyUpdateRate + "\n" +
                    "      Item publish count: " + ItemRequestCount + "\n" +
                    "       Item common count: " + CommonItemCount + "\n" +
                    "               Data File: " + MsgFilename + "\n" +
                    "               Item File: " + ItemFilename + "\n" +
                    "              Service Id: " + (!UseServiceId ? "<not used>" : ServiceId) + "\n" +
                    "            Service Name: " + ServiceName + "\n" +
                    "          Max pack count: " + MessagePackingCount + "\n" +
                    "     Packing buffer size: " + MessagePackingBufferSize;
        }

        protected override string DefaultProviderName()
        {
            return DEFAULT_PERF_PROVIDER_CONFIG_PREFIX;
        }

        protected override string DefaultSummaryFileName()
        {
            return DEFAULT_PERF_NIPROVIDER_SUMMARY_FILENAME;
        }

        protected override string DefaultStatsFileName()
        {
            return DEFAULT_PERF_NIPROVIDER_STATS_FILENAME;
        }
    }
}
