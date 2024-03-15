/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2023-2024 Refinitiv. All rights reserved.         --
 *|-----------------------------------------------------------------------------
 */

using LSEG.Ema.Access;
using LSEG.Ema.PerfTools.Common;
using System;

namespace LSEG.Ema.PerfTools.ConsPerf
{
    public class EmaConsPerfConfig
    {
        private string? _configString;
        private const int DEFAULT_THREAD_COUNT = 1;
        private int MaxThreads;


        /* APPLICATION configuration */
        public int SteadyStateTime;           /* Time application runs before exiting. */
        public int DelaySteadyStateCalc;          /* Time before the latency is calculated. */
        public int TicksPerSec;               /* Main loop ticks per second */
        public int ThreadCount;               /* Number of threads that handle connections. */

        public string? ItemFilename;           /* File of names to use when requesting items. */
        public string? MsgFilename;            /* File of data to use for message payloads. */

        public bool LogLatencyToFile;      /* Whether to log update latency information to a file. */
        public string? LatencyLogFilename;     /* Name of the latency log file. */
        public string? SummaryFilename;        /* Name of the summary log file. */
        public string? StatsFilename;          /* Name of the statistics log file. */
        public int WriteStatsInterval;        /* Controls how often statistics are written. */
        public bool DisplayStats;          /* Controls whether stats appear on the screen. */

        public int ItemRequestsPerSec;        /* Rate at which the consumer will send out item requests. */

        public bool RequestSnapshots;      /* Whether to request all items as snapshots. */

        public string? Username;               /* Username used when logging in. */
        public string? ServiceName;            /* Name of service to request items from. */
        public bool UseServiceId;          /* set service id on each request instead of setting service name on each request. */
        public int ItemRequestCount;          /* Number of items to request. See -itemCount. */
        public int CommonItemCount;           /* Number of items common to all connections, if using multiple connections. */
        public int PostsPerSec;               /* Number of posts to send per second. */
        public int LatencyPostsPerSec;        /* Number of latency posts to send per second. */
        public int GenMsgsPerSec;             /* Number of generic msgs to send per second. */
        public int LatencyGenMsgsPerSec;      /* Number of latency generic msgs to send per second. */

        public int RequestsPerTick;           /* Number of requests to send per tick */
        public int RequestsPerTickRemainder;  /* The remainder of number of requests to send per tick */

        public bool UseUserDispatch;          /* Use the EMA  USER_DISPATCH model instead of the EMA API_DISPATCH  model. */
        public bool DowncastDecoding;      /* Turn on the EMA data load downcast feature during decoding response payload. */
        public string? ConsumerName;           /* Name of the Consumer component in EmaConfig.xml. See -consumerName. */

        public string? Keyfile;                /* Keyfile for encryption */
        public string? _keypasswd;              /* Keyfile password */

        public uint EncryptionProtocol; /* Tls version. */

        internal EmaConsPerfConfig()
        {
            CommandLine.ProgName("emajConsPerf");
            CommandLine.AddOption("steadyStateTime", 300, "Time consumer will run the steady-state portion of the test. Also used as a timeout during the startup-state portion");
            CommandLine.AddOption("delaySteadyStateCalc", 0, "Time consumer will wait before calculate the latency (milliseconds)");
            CommandLine.AddOption("tickRate", 1000, "Ticks per second");
            CommandLine.AddOption("threads", DEFAULT_THREAD_COUNT, "Number of threads that handle connections");
            CommandLine.AddOption("itemFile", "350k.xml", "Name of the file to get item names from");
            CommandLine.AddOption("msgFile", "MsgData.xml", "Name of the file that specifies the data content in messages");
            CommandLine.AddOption("latencyFile", "", "Base name of file for logging latency");
            CommandLine.AddOption("summaryFile", "ConsSummary.out", "Name of file for logging summary info");
            CommandLine.AddOption("statsFile", "ConsStats", "Base name of file for logging periodic statistics");
            CommandLine.AddOption("writeStatsInterval", 5, "Controls how often stats are written to the file");
            CommandLine.AddOption("noDisplayStats", false, "Stop printout of stats to screen");
            CommandLine.AddOption("requestRate", 13500, "Rate at which to request items");
            CommandLine.AddOption("snapshot", false, "Snapshot test, request all items as non-streaming");
            CommandLine.AddOption("uname", "", "Username to use in login request");
            CommandLine.AddOption("serviceName", "DIRECT_FEED", "Name of service to request items from");
            CommandLine.AddOption("useServiceId", false, "set service id on each request instead of setting service name on each request");
            CommandLine.AddOption("itemCount", 100000, "Number of items to request");
            CommandLine.AddOption("commonItemCount", 0, "Number of items common to all consumers, if using multiple connections");
            CommandLine.AddOption("postingRate", 0, "Rate at which to send post messages");
            CommandLine.AddOption("postingLatencyRate", 0, "Rate at which to send latency post messages");
            CommandLine.AddOption("genericMsgRate", 0, "Rate at which to send generic messages");
            CommandLine.AddOption("genericMsgLatencyRate", 0, "Rate at which to send latency generic messages");
            CommandLine.AddOption("useUserDispatch", false, "Use the EMA USER_DISPATCH model instead of the EMA API_DISPATCH model for sending and receiving");
            CommandLine.AddOption("downcastDecoding", false, "Turn on the EMA data load downcast feature during decoding response payload");
            CommandLine.AddOption("consumerName", "", "Name of the Consumer component in config file EmaConfig.xml that will be usd to configure connection.");
            CommandLine.AddOption("spTLSv1.2", false, "Specifies that TLSv1.2 can be used for an encrypted connection");
            CommandLine.AddOption("spTLSv1.3", false, "Specifies that TLSv1.3 can be used for an encrypted connection");
        }

        /// <summary>
        /// Parses command-line arguments to fill in the application's configuration structures.
        /// </summary>
        /// <param name="args">command line arguments</param>
        /// <param name="maxThreads">maximum number of threads</param>
        public void Init(string[] args, int maxThreads)
        {
            try
            {
                CommandLine.ParseArgs(args);
            }
            catch (Exception ile)
            {
                Console.Error.WriteLine("Error loading command line arguments:\t");
                Console.Error.WriteLine(ile.Message);
                Console.Error.WriteLine();
                Console.Error.WriteLine(CommandLine.OptionHelpString());
                Environment.Exit(-1);
            }

            MaxThreads = maxThreads;
            MsgFilename = CommandLine.Value("msgFile");
            ItemFilename = CommandLine.Value("itemFile");
            LatencyLogFilename = CommandLine.Value("latencyFile");
            LogLatencyToFile = LatencyLogFilename != null && LatencyLogFilename.Length > 0;
            SummaryFilename = CommandLine.Value("summaryFile");
            StatsFilename = CommandLine.Value("statsFile");
            Username = CommandLine.Value("uname");
            ServiceName = CommandLine.Value("serviceName");
            UseServiceId = CommandLine.BoolValue("useServiceId");
            DisplayStats = !CommandLine.BoolValue("noDisplayStats");
            RequestSnapshots = CommandLine.BoolValue("snapshot");
            UseUserDispatch = CommandLine.BoolValue("useUserDispatch");
            DowncastDecoding = CommandLine.BoolValue("downcastDecoding");

            ConsumerName = CommandLine.Value("consumerName");

            EncryptionProtocol = EmaConfig.EncryptedTLSProtocolFlags.NONE;
            if (CommandLine.BoolValue("spTLSv1.2"))
                EncryptionProtocol |= EmaConfig.EncryptedTLSProtocolFlags.TLSv1_2;
            if (CommandLine.BoolValue("spTLSv1.3"))
                EncryptionProtocol |= EmaConfig.EncryptedTLSProtocolFlags.TLSv1_3;

            try
            {
                SteadyStateTime = CommandLine.IntValue("steadyStateTime");
                DelaySteadyStateCalc = CommandLine.IntValue("delaySteadyStateCalc");
                TicksPerSec = CommandLine.IntValue("tickRate");
                ThreadCount = CommandLine.IntValue("threads");
                WriteStatsInterval = CommandLine.IntValue("writeStatsInterval");
                ItemRequestsPerSec = CommandLine.IntValue("requestRate");
                ItemRequestCount = CommandLine.IntValue("itemCount");
                CommonItemCount = CommandLine.IntValue("commonItemCount");
                PostsPerSec = CommandLine.IntValue("postingRate");
                LatencyPostsPerSec = CommandLine.IntValue("postingLatencyRate");
                GenMsgsPerSec = CommandLine.IntValue("genericMsgRate");
                LatencyGenMsgsPerSec = CommandLine.IntValue("genericMsgLatencyRate");
            }
            catch (Exception ile)
            {
                Console.Error.WriteLine("Invalid argument, number expected.\t");
                Console.Error.WriteLine(ile.Message);
                Console.Error.WriteLine();
                Console.Error.WriteLine(CommandLine.OptionHelpString());
                Environment.Exit(-1);
            }

            if (TicksPerSec < 1)
            {
                Console.Error.WriteLine("Config Error: Tick rate cannot be less than 1.");
                Console.Error.WriteLine(CommandLine.OptionHelpString());
                Environment.Exit(-1);
            }

            if (ItemRequestsPerSec < TicksPerSec)
            {
                Console.Error.WriteLine("Config Error: Item Request Rate cannot be less than tick rate.\n");
                Console.Error.WriteLine(CommandLine.OptionHelpString());
                Environment.Exit(-1);
            }

            if (PostsPerSec < TicksPerSec && PostsPerSec != 0)
            {
                Console.Error.WriteLine("Config Error: Post Rate cannot be less than tick rate(unless it is zero).\n");
                Console.Error.WriteLine(CommandLine.OptionHelpString());
                Environment.Exit(-1);
            }

            if (LatencyPostsPerSec > PostsPerSec)
            {
                Console.Error.WriteLine("Config Error: Latency Post Rate cannot be greater than total posting rate.\n");
                Console.Error.WriteLine(CommandLine.OptionHelpString());
                Environment.Exit(-1);
            }

            if (LatencyPostsPerSec > TicksPerSec)
            {
                Console.Error.WriteLine("Config Error: Latency Post Rate cannot be greater than tick rate.\n");
                Console.Error.WriteLine(CommandLine.OptionHelpString());
                Environment.Exit(-1);
            }

            if (PostsPerSec > 0 && RequestSnapshots)
            {
                Console.Error.WriteLine("Config Error: Configured to post while requesting snapshots.\n");
                Console.Error.WriteLine(CommandLine.OptionHelpString());
                Environment.Exit(-1);
            }

            if (GenMsgsPerSec < TicksPerSec && GenMsgsPerSec != 0)
            {
                Console.Error.WriteLine("Config Error: Generic Msg Rate cannot be less than tick rate(unless it is zero).\n");
                Console.Error.WriteLine(CommandLine.OptionHelpString());
                Environment.Exit(-1);
            }

            if (LatencyGenMsgsPerSec > GenMsgsPerSec)
            {
                Console.Error.WriteLine("Config Error: Latency Generic Msg Rate cannot be greater than total generic msg rate.\n");
                Console.Error.WriteLine(CommandLine.OptionHelpString());
                Environment.Exit(-1);
            }

            if (LatencyGenMsgsPerSec > TicksPerSec)
            {
                Console.Error.WriteLine("Config Error: Latency Generic Msg Rate cannot be greater than tick rate.\n");
                Console.Error.WriteLine(CommandLine.OptionHelpString());
                Environment.Exit(-1);
            }

            if (GenMsgsPerSec > 0 && RequestSnapshots)
            {
                Console.Error.WriteLine("Config Error: Configured to send generic msgs while requesting snapshots.\n");
                Console.Error.WriteLine(CommandLine.OptionHelpString());
                Environment.Exit(-1);
            }

            if (CommonItemCount > ItemRequestCount)
            {
                Console.Error.WriteLine("Config Error: Common item count is greater than total item count.\n");
                Console.Error.WriteLine(CommandLine.OptionHelpString());
                Environment.Exit(-1);
            }

            if (WriteStatsInterval < 1)
            {
                Console.Error.WriteLine("Config error: Write Stats Interval cannot be less than 1.\n");
                Console.Error.WriteLine(CommandLine.OptionHelpString());
                Environment.Exit(-1);
            }

            if (ThreadCount > MaxThreads)
            {
                Console.Error.WriteLine("Config error: Thread count cannot be greater than " + MaxThreads + ".\n");
                Console.Error.WriteLine(CommandLine.OptionHelpString());
                Environment.Exit(-1);
            }

            if (DelaySteadyStateCalc < 0 || DelaySteadyStateCalc > 30000)
            {
                Console.Error.WriteLine("Config error: Time before the latency is calculated should not be less than 0 or greater than 30000.\n");
                Console.Error.WriteLine(CommandLine.OptionHelpString());
                Environment.Exit(-1);
            }

            if ((DelaySteadyStateCalc / 1000) > SteadyStateTime)
            {
                Console.Error.WriteLine("Config Error: Time before the latency is calculated should be less than Steady State Time.\n");
                Console.Error.WriteLine(CommandLine.OptionHelpString());
                Environment.Exit(-1);
            }

            RequestsPerTick = ItemRequestsPerSec / TicksPerSec;

            RequestsPerTickRemainder = ItemRequestsPerSec % TicksPerSec;

            CreateConfigString();
        }

        /* helper enum to make EncryptionProtocol string */
        private enum EncryptedTLSProtocolFlags
        {
            TLS_DEFAULT = 0x00,
            TLS_V1_2 = 0x04,
            TLS_V1_3 = 0x08,
            TLS_ALL = 0x0C
        }

        /* Create config string. */
        private void CreateConfigString()
        {
            string useOperationModelUsageString;

            if (UseUserDispatch)
            {
                useOperationModelUsageString = "USER_DISPATCH";
            }
            else
            {
                useOperationModelUsageString = "API_DISPATCH";
            }

            _configString = "--- TEST INPUTS ---\n\n" +
                    "       Steady State Time: " + SteadyStateTime + " sec\n" +
                    " Delay Steady State Time: " + DelaySteadyStateCalc + " msec\n" +
                    "                 Service: " + ServiceName + "\n" +
                    "            UseServiceId: " + (UseServiceId ? "Yes" : "No") + "\n" +
                    "            Thread Count: " + ThreadCount + "\n" +
                    "                Username: " + (Username!.Length > 0 ? Username : "(use system login name)") + "\n" +
                    "              Item Count: " + ItemRequestCount + "\n" +
                    "       Common Item Count: " + CommonItemCount + "\n" +
                    "            Request Rate: " + ItemRequestsPerSec + "\n" +
                    "       Request Snapshots: " + (RequestSnapshots ? "Yes" : "No") + "\n" +
                    "            Posting Rate: " + PostsPerSec + "\n" +
                    "    Latency Posting Rate: " + LatencyPostsPerSec + "\n" +
                    "        Generic Msg Rate: " + GenMsgsPerSec + "\n" +
                    "Generic Msg Latency Rate: " + LatencyGenMsgsPerSec + "\n" +
                    "               Item File: " + ItemFilename + "\n" +
                    "               Data File: " + MsgFilename + "\n" +
                    "            Summary File: " + SummaryFilename + "\n" +
                    "              Stats File: " + StatsFilename + "\n" +
                    "        Latency Log File: " + (LatencyLogFilename != null && LatencyLogFilename!.Length > 0 ? LatencyLogFilename : "(none)") + "\n" +
                    "               Tick Rate: " + TicksPerSec + "\n" +
                    "        DowncastDecoding: " + (DowncastDecoding ? "True" : "False") + "\n" +
                    "    OperationModel Usage: " + useOperationModelUsageString + "\n" +
                    "       Security Protocol: " + (EncryptedTLSProtocolFlags)EncryptionProtocol + "\n";
        }

        public override string ToString()
        {
            return _configString ?? "";
        }

    }
}
