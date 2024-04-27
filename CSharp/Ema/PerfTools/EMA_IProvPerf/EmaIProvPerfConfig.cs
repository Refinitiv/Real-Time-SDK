/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2024 Refinitiv. All rights reserved.              --
 *|-----------------------------------------------------------------------------
 */

using LSEG.Ema.PerfTools.Common;

namespace LSEG.Ema.PerfTools.EMA_IProvPerf
{
    public class EmaIProvPerfConfig : BaseProviderPerfConfig
    {
        public const string APPLICATION_NAME = "EMAProvPerf";
        public const string APPLICATION_ID = "256";
        private const string DEFAULT_PERF_PROVIDER_CONFIG_PREFIX = "Perf_Provider_";
        private const string DEFAULT_PERF_IPROVIDER_SUMMARY_FILENAME = "IProvSummary.out";
        private const string DEFAULT_PERF_IPROVIDER_STATS_FILENAME = "IProvStats";

        public int genMsgsPerTick;              // Total generic rate per tick
        public int genMsgsPerTickRemainder;     // gen msgs per tick remainder
        public int genMsgsPerSec;               // Total generic msg rate per second(includes latency generic msgs).
        public int latencyGenMsgRate;           // Total latency generic msg rate per second

        public string? latencyFilename;          // Latency file
        public bool logLatencyToFile;

        public EmaIProvPerfConfig()
        {
            CommandLine.ProgName("IProvPerf");
            CommandLine.AddOption("latencyUpdateRate", 10, "Latency update rate per second (can specify \"all\" to send latency in every update");
            CommandLine.AddOption("genericMsgRate", 0, "Generic Msg rate per second");
            CommandLine.AddOption("genericMsgLatencyRate", 0, "Latency Generic Msg rate per second (can specify \"all\" to send latency in every generic msg");
            CommandLine.AddOption("latencyFile", "IProvLatency.out", "name of file for logging latency info");
        }

        protected override void FillProperties()
        {
            base.FillProperties();
            SummaryFilename = CommandLine.Value("summaryFile");
            StatsFilename = CommandLine.Value("statsFile");
            latencyFilename = CommandLine.Value("latencyFile");
            logLatencyToFile = latencyFilename != null && latencyFilename.Length != 0;
            genMsgsPerSec = CommandLine.IntValue("genericMsgRate");

            string? latencyGenMsgRate = CommandLine.Value("genericMsgLatencyRate");

            if ("all".Equals(latencyGenMsgRate))
            {
                this.latencyGenMsgRate = ALWAYS_SEND_LATENCY_GENMSG;
            }
            else if (latencyGenMsgRate != null)
            {
                this.latencyGenMsgRate = int.Parse(latencyGenMsgRate);
            }

            if (this.latencyGenMsgRate > TicksPerSec)
            {
                Console.Error.WriteLine("Config Error: Latency Generic Message Rate cannot be greater than tick rate.\n");
                Console.WriteLine(CommandLine.OptionHelpString());
                Environment.Exit(-1);
            }

            if (genMsgsPerSec != 0 && genMsgsPerSec < TicksPerSec)
            {
                Console.Error.WriteLine("Config Error: Generic message rate cannot be less than total ticks per second (unless it is zero).\n\n");
                Console.WriteLine(CommandLine.OptionHelpString());
                Environment.Exit(-1);
            }

            genMsgsPerTick = genMsgsPerSec / TicksPerSec;
            genMsgsPerTickRemainder = genMsgsPerSec % TicksPerSec;
        }

        protected override void CreateConfigString()
        {
            this.m_ConfigString = "--- TEST INPUTS ---\n\n" +
                    "                Run Time: " + RunTime + "\n" +
                    "         useUserDispatch: " + (UseUserDispatch ? "Yes" : "No") + "\n" +
                    "                 Threads: " + ThreadCount + "\n" +
                    "            Summary File: " + SummaryFilename + "\n" +
                    "        Latency Log File: " + ((latencyFilename == null || latencyFilename.Length == 0) ? "(none)" : latencyFilename + "\n") +
                    "    Write Stats Interval: " + WriteStatsInterval + "\n" +
                    "           Display Stats: " + (DisplayStats ? "Yes" : "No") + "\n" +
                    "               Tick Rate: " + TicksPerSec + "\n" +
                    "             Update Rate: " + UpdatesPerSec + "\n" +
                    "     Latency Update Rate: " + LatencyUpdateRate + "\n" +
                    "        Generic Msg Rate: " + genMsgsPerSec + "\n" +
                    "Latency Generic Msg Rate: " + latencyGenMsgRate + "\n" +
                    "      Refresh Burst Size: " + RefreshBurstSize + "\n" +
                    "               Data File: " + MsgFilename + "\n" +
                    "          Max pack count: " + MessagePackingCount + "\n" +
                    "     Packing buffer size: " + MessagePackingBufferSize + "\n";
        }

        protected override string DefaultProviderName()
        {
            return DEFAULT_PERF_PROVIDER_CONFIG_PREFIX;
        }

        protected override string DefaultStatsFileName()
        {
            return DEFAULT_PERF_IPROVIDER_STATS_FILENAME;
        }

        protected override string DefaultSummaryFileName()
        {
            return DEFAULT_PERF_IPROVIDER_SUMMARY_FILENAME;
        }

        /// <summary>
        /// Determines whether to log update latency information to a file.
        /// </summary>
        /// <returns>true if update latency has to be logged to file, false otherwise</returns>
        public bool LogLatencyToFile()
        {
            return latencyFilename != null && latencyFilename.Length > 0;
        }
    }
}
