/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2022-2023 Refinitiv. All rights reserved.              --
 *|-----------------------------------------------------------------------------
 */

using System;
using LSEG.Eta.Codec;
using LSEG.Eta.Example.Common;
using LSEG.Eta.Transports;

namespace LSEG.Eta.PerfTools.Common
{

    /// <summary>
    /// Provides configuration option for the non-interactive provider performance
    /// application.
    /// </summary>
    public class NIProvPerfConfig
    {
        private static readonly int DEFAULT_THREAD_COUNT = 1;
        private static readonly int ALWAYS_SEND_LATENCY_UPDATE = -1;

        #region APPLICATION configuration

        /// <summary>Time application runs before exiting.</summary>
        public static TimeSpan RunTime { get; private set; }

        /// <summary>Main loop ticks per second.</summary>
        public static int TicksPerSec => ProviderPerfConfig.TicksPerSec;

        /// <summary>Number of threads that handle connections.</summary>
        public static int ThreadCount => ProviderPerfConfig.ThreadCount;

        /// <summary>Item List file. Provides a list of items to open.</summary>
        public static string ItemFilename => ProviderPerfConfig.ItemFilename!;

        /// <summary>Data file. Describes the data to use when encoding messages.</summary>
        public static string MsgFilename => ProviderPerfConfig.MsgFilename!;

        /// <summary>Name of the summary log file.</summary>
        public static string? SummaryFilename { get; private set; }

        /// <summary>Name of the statistics log file.</summary>
        public static string StatsFilename => ProviderPerfConfig.StatsFilename!;

        /// <summary>Controls how often statistics are written.</summary>
        public static int WriteStatsInterval { get; private set; }

        /// <summary>Controls whether stats appear on the screen.</summary>
        public static bool DisplayStats { get; private set; }

        /// <summary>How many messages are packed into a given buffer.</summary>
        public static int TotalBuffersPerPack => ProviderPerfConfig.TotalBuffersPerPack;

        /// <summary>Size of packable buffer, if packing.</summary>
        public static int PackingBufferLength => ProviderPerfConfig.PackingBufferLength;

        /// <summary>Total refresh rate per second.</summary>
        public static int RefreshBurstSize => ProviderPerfConfig.RefreshBurstSize;

        #endregion

        #region CONNECTION configuration

        /// <summary>Type of connection.</summary>
        public static ConnectionType ConnectionType { get; private set; }

        /// <summary>Type of encrypted connection.</summary>
        public static ConnectionType EncryptedConnectionType { get; private set; }

        /// <summary>hostName, if using Channel.connect().</summary>
        public static string? HostName { get; private set; }

        /// <summary>Port number.</summary>
        public static string? PortNo { get; private set; }

        /// <summary>Name of interface.</summary>
        public static string? InterfaceName { get; private set; }

        /// <summary>Guaranteed Output Buffers.</summary>
        public static int GuaranteedOutputBuffers { get; private set; }

        /// <summary>System Send Buffer Size.</summary>
        public static int SendBufSize { get; private set; }

        /// <summary>System Receive Buffer Size.</summary>
        public static int RecvBufSize { get; private set; }

        /// <summary>System Send timeout</summary>
		public static int SendTimeout { get; set; }

        /// <summary>System Receive timeout</summary>
        public static int RecvTimeout { get; set; }

        /// <summary>The point that causes ETA to automatically flush.</summary>
        public static int HighWaterMark { get; private set; }

        /// <summary>The fragment size.</summary>
        public static int MaxFragmentSize { get; private set; }

        /// <summary>Enable/Disable Nagle's algorithm.</summary>
        public static bool TcpNoDelay { get; private set; }

        /// <summary>Username used when logging in.</summary>
        public static string? Username { get; private set; }

        /// <summary>Number of items to publish.</summary>
        public static int ItemPublishCount { get; private set; }

        /// <summary>Number of items common to all connections, if using multiple connections.</summary>
        public static int CommonItemCount { get; private set; }

        /// <summary>Use the VA Reactor instead of the ETA Channel for sending and receiving.</summary>
        public static bool UseReactor;

        /// <summary>Number of updates to send per second.</summary>
        public static int UpdatesPerSec => ProviderPerfConfig.UpdatesPerSec;

        /// <summary>Number of latency updates to send per second.</summary>
        public static int LatencyUpdateRate => ProviderPerfConfig.LatencyUpdateRate;

        /// <summary>ID of the provided service.</summary>
        public static int ServiceId => ProviderPerfConfig.ServiceId;

        /// <summary>Name of the provided service.</summary>
        public static string ServiceName => ProviderPerfConfig.ServiceName!;

        /// <summary>Advertised OpenLimit (set to 0 to not provide this).</summary>
        public static int OpenLimit => ProviderPerfConfig.OpenLimit;

        /// <summary>Converts configuration parameters to a string.</summary>
        public static string? ConvertToString { get; private set; }

        public static readonly EncryptionProtocolCommandLineArg EncryptionProtocol = new();

        #endregion

        static NIProvPerfConfig()
        {
            CommandLine.AddOption("runTime", 360, "Time application runs before exiting");
            CommandLine.AddOption("summaryFile", "NIProvSummary.out", "Name of file for logging summary info");
            CommandLine.AddOption("writeStatsInterval", 5, "Controls how often stats are written to the file");
            CommandLine.AddOption("noDisplayStats", false, "Stop printout of stats to screen");
            CommandLine.AddOption("connType", "socket", "Type of connection (\"socket\" or \"encrypted\")");
            CommandLine.AddOption("encryptedConnectionType", "socket", "Type of encrypted connection (\"socket\")");
            CommandLine.AddOption("h", "localhost", "Name of host for socket connection");
            CommandLine.AddOption("p", "14003", "Port number for socket connection");
            CommandLine.AddOption("if", "", "Name of network interface to use");
            CommandLine.AddOption("uname", "", "Username to use in login request");
            CommandLine.AddOption("tcpDelay", false, "Turns off tcp_nodelay in ConnectOptions, enabling Nagle's");
            CommandLine.AddOption("outputBufs", 5000, "Number of output buffers(configures guaranteedOutputBuffers in ETA connection)");
            CommandLine.AddOption("maxFragmentSize", 6144, "Maximum Fragment Size");
            CommandLine.AddOption("sendBufSize", 0, "System Send Buffer Size(configures sysSendBufSize in ConnectOptions)");
            CommandLine.AddOption("recvBufSize", 0, "System Receive Buffer Size(configures sysRecvBufSize in ConnectOptions)");
            CommandLine.AddOption("sendTimeout", 0, "System Send timeout (configures SendTimeout in ConnectOptions)");
            CommandLine.AddOption("recvTimeout", 0, "System Receive timeout (configures ReceiveTimeout in ConnectOptions)");
            CommandLine.AddOption("highWaterMark", 0, "Sets the point that causes ETA to automatically flush");
            CommandLine.AddOption("itemCount", 100_000, "Number of items to publish non-interactively");
            CommandLine.AddOption("commonItemCount", 0, "Number of items common to all providers, if using multiple connections");
            CommandLine.AddOption("reactor", false, "Use the VA Reactor instead of the ETA Channel for sending and receiving");

            // ProviderPerfConfig
            CommandLine.AddOption("threads", DEFAULT_THREAD_COUNT, "Number of non-interactive provider threads to create");
            CommandLine.AddOption("statsFile", "NIProvStats", "Base name of file for logging periodic statistics");
            CommandLine.AddOption("itemFile", "350k.xml", "Name of the file to get item names from");
            CommandLine.AddOption("msgFile", "MsgData.xml", "Name of the file that specifies the data content in messages");
            CommandLine.AddOption("updateRate", 100_000, "Total update rate per second(includes latency updates)");
            CommandLine.AddOption("refreshBurstSize", 10, "Number of refreshes to send in a burst(controls granularity of time-checking)");
            CommandLine.AddOption("latencyUpdateRate", 10, "Total latency update rate per second");
            CommandLine.AddOption("tickRate", 1000, "Ticks per second");
            CommandLine.AddOption("maxPackCount", 1, "Number of messages packed into a given buffer");
            CommandLine.AddOption("packBufSize", 6000, "Size of packable buffer, if packing");
            CommandLine.AddOption("directWrite", false, "Turns on direct write flag");
            CommandLine.AddOption("serviceName", "NI_PUB", "Name of the provided service");
            CommandLine.AddOption("serviceId", 1, "ID of the provided service");
        }

        /// NIProvPerfConfig cannot be instantiated
        private NIProvPerfConfig()
        {

        }

        /// <summary>Parses command-line arguments to fill in the application's configuration structures.</summary>
        ///
        /// <param name="args">the args</param>
        public static void Init(string[] args)
        {
            try
            {
                CommandLine.ParseArgs(args);
            }
            catch (Exception e)
            {
                Console.Error.WriteLine("Error loading command line arguments:\t");
                Console.Error.WriteLine(e.Message);
                Console.Error.WriteLine();
                Console.WriteLine(CommandLine.OptionHelpString());
                Environment.Exit((int)PerfToolsReturnCode.FAILURE);
            }

            // Don't need to provide an open limit. Our encode logic will skip the OpenLimit entry if it is set to zero.
            ProviderPerfConfig.OpenLimit = 0;

            CommandLine.ProgName("NIProvPerf");
            ProviderPerfConfig.ItemFilename = CommandLine.Value("itemFile");
            ProviderPerfConfig.MsgFilename = CommandLine.Value("msgFile");
            ProviderPerfConfig.StatsFilename = CommandLine.Value("statsFile");
            SummaryFilename = CommandLine.Value("summaryFile");
            DisplayStats = !CommandLine.BoolValue("noDisplayStats");
            HostName = CommandLine.Value("h");
            PortNo = CommandLine.Value("p");
            InterfaceName = CommandLine.Value("if");
            Username = CommandLine.Value("uname");
            TcpNoDelay = !CommandLine.BoolValue("tcpDelay");
            ProviderPerfConfig.DirectWrite = CommandLine.BoolValue("directWrite");
            ProviderPerfConfig.ServiceName = CommandLine.Value("serviceName");
            UseReactor = CommandLine.BoolValue("reactor");
            string? encryptedConnectionType = CommandLine.Value("encryptedConnectionType");
            if (!string.IsNullOrEmpty(encryptedConnectionType))
            {
                switch (encryptedConnectionType)
                {
                    case "socket" :
                        EncryptedConnectionType = ConnectionType.SOCKET;
                        break;
                    default:
                        Console.Error.WriteLine("Config Error: Only socket encrypted connection type is supported.");
                        Console.WriteLine(CommandLine.OptionHelpString());
                        Environment.Exit(-1);
                        break;
                }
            }

            try
            {
                RunTime = TimeSpan.FromSeconds(CommandLine.IntValue("runTime"));
                ProviderPerfConfig.TicksPerSec = CommandLine.IntValue("tickRate");
                ProviderPerfConfig.ThreadCount = CommandLine.IntValue("threads") > 1 ? CommandLine.IntValue("threads") : 1;
                WriteStatsInterval = CommandLine.IntValue("writeStatsInterval");
                ProviderPerfConfig.TotalBuffersPerPack = CommandLine.IntValue("maxPackCount");
                ProviderPerfConfig.PackingBufferLength = CommandLine.IntValue("packBufSize");
                ProviderPerfConfig.RefreshBurstSize = CommandLine.IntValue("refreshBurstSize");
                ProviderPerfConfig.UpdatesPerSec = CommandLine.IntValue("updateRate");

                if ("all".Equals(CommandLine.Value("latencyUpdateRate")))
                    ProviderPerfConfig.LatencyUpdateRate = ALWAYS_SEND_LATENCY_UPDATE;
                else
                    ProviderPerfConfig.LatencyUpdateRate = CommandLine.IntValue("latencyUpdateRate");

                string? connectionType = CommandLine.Value("connType");
                if (!"socket".Equals(connectionType) && !"encrypted".Equals(connectionType))
                {
                    Console.Error.WriteLine("Config Error: Only socket and encrypted connection types are supported.");
                    Console.WriteLine(CommandLine.OptionHelpString());
                    Environment.Exit(-1);
                }
                if ("socket".Equals(connectionType))
                {
                    ConnectionType = ConnectionType.SOCKET;
                }
                else if ("encrypted".Equals(connectionType))
                {
                    ConnectionType = ConnectionType.ENCRYPTED;
                }

                GuaranteedOutputBuffers = CommandLine.IntValue("outputBufs");
                MaxFragmentSize = CommandLine.IntValue("maxFragmentSize");
                SendBufSize = CommandLine.IntValue("sendBufSize");
                RecvBufSize = CommandLine.IntValue("recvBufSize");
                SendTimeout = CommandLine.IntValue("sendTimeout");
                RecvTimeout = CommandLine.IntValue("recvTimeout");
                HighWaterMark = CommandLine.IntValue("highWaterMark");
                ItemPublishCount = CommandLine.IntValue("itemCount");
                CommonItemCount = CommandLine.IntValue("commonItemCount");
                ProviderPerfConfig.ServiceId = CommandLine.IntValue("serviceId");
            }
            catch (Exception e)
            {
                Console.Error.WriteLine("Invalid argument: \t");
                Console.Error.WriteLine(e.Message);
                Console.Error.WriteLine();
                Console.WriteLine(CommandLine.OptionHelpString());
                Environment.Exit(-1);
            }

            // Check conditions

            if (ProviderPerfConfig.ThreadCount < 1)
            {
                Console.Error.WriteLine("Config Error: Thread count cannot be less than 1.\n");
                Console.WriteLine(CommandLine.OptionHelpString());
                Environment.Exit(-1);
            }

            if (ProviderPerfConfig.TicksPerSec < 1)
            {
                Console.Error.WriteLine("Config Error: Tick rate cannot be less than 1.\n");
                Console.WriteLine(CommandLine.OptionHelpString());
                Environment.Exit(-1);
            }

            if (CommonItemCount > ItemPublishCount / ProviderPerfConfig.ThreadCount)
            {
                Console.Error.WriteLine($"Config Error: Common item count ({CommonItemCount}) " +
                    $"is greater than total item count per thread ({ItemPublishCount / ProviderPerfConfig.ThreadCount}).\n");
                Console.WriteLine(CommandLine.OptionHelpString());
                Environment.Exit(-1);
            }

            if (CommonItemCount > ItemPublishCount)
            {
                Console.Error.WriteLine("Config Error: Common item count is greater than total item count.\n");
                Console.WriteLine(CommandLine.OptionHelpString());
                Environment.Exit(-1);
            }

            if (ProviderPerfConfig.LatencyUpdateRate > ProviderPerfConfig.UpdatesPerSec)
            {
                Console.Error.WriteLine("Config Error: Latency update rate cannot be greater than total update rate. \n\n");
                Environment.Exit(-1);
            }

            if (ProviderPerfConfig.LatencyUpdateRate > ProviderPerfConfig.TicksPerSec)
            {
                Console.Error.WriteLine("Config Error: Latency update rate cannot be greater than total ticks per second. \n\n");
                Environment.Exit(-1);
            }

            if (ProviderPerfConfig.UpdatesPerSec != 0 && ProviderPerfConfig.UpdatesPerSec < ProviderPerfConfig.TicksPerSec)
            {
                Console.Error.WriteLine("Config Error: Update rate cannot be less than total ticks per second(unless it is zero).\n\n");
                Environment.Exit(-1);
            }

            if (ProviderPerfConfig.TotalBuffersPerPack < 1)
            {
                Console.Error.WriteLine("Config Error: Cannot specify less than 1 buffer per pack.\n\n");
                Environment.Exit(-1);
            }

            if (ProviderPerfConfig.TotalBuffersPerPack > 1 && ProviderPerfConfig.PackingBufferLength == 0)
            {
                Console.Error.WriteLine("Config Error: -maxPackCount set but -packBufSize is zero.\n\n");
                Environment.Exit(-1);
            }

            ProviderPerfConfig.UpdatesPerTick = ProviderPerfConfig.UpdatesPerSec / ProviderPerfConfig.TicksPerSec;
            ProviderPerfConfig.UpdatesPerTickRemainder = ProviderPerfConfig.UpdatesPerSec % ProviderPerfConfig.TicksPerSec;

            CreateConfigString();
        }

        /// <summary>Create config string.</summary>
        private static void CreateConfigString()
        {
            string connectionString = $"               Hostname: {HostName}\n" +
                                      $"                   Port: {PortNo}\n";

            ConvertToString = "--- TEST INPUTS ---\n\n" +
                "               Run Time: " + RunTime.TotalSeconds + " sec\n" +
                "        Connection Type: " + CommandLine.Value("connType") + "\n" +
                           connectionString +
                "         Output Buffers: " + GuaranteedOutputBuffers + "\n" +
                "      Max Fragment Size: " + MaxFragmentSize + "\n" +
                "       Send Buffer Size: " + SendBufSize + ((SendBufSize > 0) ? " bytes" : "(use default)") + "\n" +
                "       Recv Buffer Size: " + RecvBufSize + ((RecvBufSize > 0) ? " bytes" : "(use default)") + "\n" +
                "           Send Timeout: " + SendTimeout + ((SendTimeout > 0) ? " ms" : "(use default)") + "\n" +
                "           Recv Timeout: " + RecvTimeout + ((RecvTimeout > 0) ? " ms" : "(use default)") + "\n" +
                "        High Water Mark: " + HighWaterMark + ((HighWaterMark > 0) ? " bytes" : "(use default)") + "\n" +
                "         Interface Name: " + (InterfaceName != null && InterfaceName.Length > 0 ? InterfaceName : "(use default)") + "\n" +
                "               Username: " + (Username != null && Username.Length > 0 ? Username : "(use system login name)") + "\n" +
                "            Tcp_NoDelay: " + (TcpNoDelay ? "Yes" : "No") + "\n" +
                "             Item Count: " + ItemPublishCount + "\n" +
                "      Common Item Count: " + CommonItemCount + "\n" +
                "              Tick Rate: " + ProviderPerfConfig.TicksPerSec + "\n" +
                "      Use Direct Writes: " + (ProviderPerfConfig.DirectWrite ? "Yes" : "No") + "\n" +
                "           Summary File: " + SummaryFilename + "\n" +
                "             Stats File: " + ProviderPerfConfig.StatsFilename + "\n" +
                "   Write Stats Interval: " + WriteStatsInterval + "\n" +
                "          Display Stats: " + DisplayStats + "\n" +
                "            Update Rate: " + ProviderPerfConfig.UpdatesPerSec + "\n" +
                "    Latency Update Rate: " + ((ProviderPerfConfig.LatencyUpdateRate >= 0) ? ProviderPerfConfig.LatencyUpdateRate : ProviderPerfConfig.UpdatesPerSec) + "\n" +
                "     Refresh Burst Size: " + ProviderPerfConfig.RefreshBurstSize + "\n" +
                "              Item File: " + ProviderPerfConfig.ItemFilename + "\n" +
                "              Data File: " + ProviderPerfConfig.MsgFilename + "\n" +
                "                Packing: " + ((ProviderPerfConfig.TotalBuffersPerPack > 1) ? "Yes(max " + ProviderPerfConfig.TotalBuffersPerPack + " per pack, " + ProviderPerfConfig.PackingBufferLength + " buffer size)" : "No") + "\n" +
                "             Service ID: " + ProviderPerfConfig.ServiceId + "\n" +
                "           Service Name: " + ProviderPerfConfig.ServiceName + "\n" +
                "            Use Reactor: " + (UseReactor ? "Yes" : "No") + "\n\n";
        }
    }
}
