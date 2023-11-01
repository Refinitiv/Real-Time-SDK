using LSEG.Eta.Example.Common;
using LSEG.Eta.Transports;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace LSEG.Eta.PerfTools.Common
{
    public class ProviderPerfConfig
    {
        private const int ALWAYS_SEND_LATENCY_UPDATE = -1;
        private const int ALWAYS_SEND_LATENCY_GENMSG = -1;

        public static string? ConfigString { get; set; }
        public static int TicksPerSec { get; set; }                 // Controls granularity of update bursts (how they must be sized to match the desired update rate).
        public static int TotalBuffersPerPack { get; set; }         // How many messages are packed into a given buffer.
        public static int PackingBufferLength { get; set; }         // Size of packable buffer, if packing.
        public static int RefreshBurstSize { get; set; }            // Number of refreshes to send in a burst(controls granularity of time-checking)
        public static int UpdatesPerSec { get; set; }               // Total update rate per second(includes latency updates).
        public static int UpdatesPerTick { get; set; }              // Updates per tick
        public static int UpdatesPerTickRemainder { get; set; }     // Updates per tick (remainder)
        public static int GenMsgsPerSec { get; set; }               // Total generic msg rate per second(includes latency generic msgs).
        public static int GenMsgsPerTick { get; set; }              // Generic msgs per tick
        public static int GenMsgsPerTickRemainder { get; set; }     // Generic msgs per tick (remainder)

        public static int LatencyUpdateRate { get; set; }           // Total latency update rate per second
        public static int LatencyGenMsgRate { get; set; }           // Total latency generic msg rate per second
        public static string? ItemFilename { get; set; }             // Item List file. Provides a list of items to open.
        public static string? MsgFilename { get; set; }              // Data file. Describes the data to use when encoding messages.
        public static int ThreadCount { get; set; }                 // Number of provider threads to create.
        public static int RunTime { get; set; }                     // Time application runs before exiting

        public static string? ServiceName { get; set; }              // Name of the provided service
        public static int ServiceId { get; set; }                   // ID of the provided service
        public static int OpenLimit { get; set; }                   // Advertised OpenLimit (default is set to 0 to not use this)

        public static string? PortNo { get; set; }                   // Port number
        public static string? InterfaceName { get; set; }            // Network interface to bind to
        public static bool TcpNoDelay { get; set; }                 // TCP_NODELAY option for socket
        public static int GuaranteedOutputBuffers { get; set; }     // Guaranteed Output Buffers
        public static int MaxOutputBuffers { get; set; }            // Max Output Buffers
        public static int MaxFragmentSize { get; set; }             // Max fragment size
        public static int HighWaterMark { get; set; }               // High water mark
        public static int SendBufSize { get; set; }                 // System send buffer size
        public static int RecvBufSize { get; set; }                 // System receive buffer size
        public static int SendTimeout { get; set; }                 // System Send timeout
        public static int RecvTimeout { get; set; }                 // System Receive timeout
        public static string? SummaryFilename { get; set; }          // Summary file
        public static string? StatsFilename { get; set; }            // Stats file
        public static string? LatencyFilename { get; set; }          // Latency file
        public static int WriteStatsInterval { get; set; }          // Controls how often statistics are written
        public static bool DisplayStats { get; set; }               // Controls whether stats appear on the screen
        public static bool DirectWrite { get; set; }                // direct write enabled
        public static bool UseReactor { get; set; }                 // Use the VA Reactor instead of the ETA Channel for sending and receiving.
        public static ConnectionType ConnectionType { get; set; }   // Connection Type, either ConnectionTypes.SOCKET or ConnectionTypes.ENCRYPTED
        public static string? KeyFile { get; set; }
        public static string? Cert { get; set; }

        public static readonly EncryptionProtocolCommandLineArg EncryptionProtocol = new();

        /// <summary>Whether to log update latency information to a file</summary>
        public static bool LogLatencyToFile
        {
            get => !string.IsNullOrEmpty(LatencyFilename);
        }

        static ProviderPerfConfig()
        {
            CommandLine.ProgName("ProvPerf");
            CommandLine.AddOption("p", "14002", "Port number to connect to");
            CommandLine.AddOption("outputBufs", 50000, "Number of output buffers(configures guaranteedOutputBuffers in BindOptions)");
            CommandLine.AddOption("maxOutputBufs", 0, "Number of max output buffers, optional(configures maxOutputBuffers in BindOptions), value is recalculated by API to match expression: 5 < \"-outputBufs\" <= \"-maxOutputBufs\"");
            CommandLine.AddOption("maxFragmentSize", 6144, " Max size of buffers(configures maxFragmentSize in BindOptions)");
            CommandLine.AddOption("sendBufSize", 0, "System Send Buffer Size(configures sysSendBufSize in BindOptions)");
            CommandLine.AddOption("recvBufSize", 0, "System Receive Buffer Size(configures sysRecvBufSize in BindOptions)");
            CommandLine.AddOption("sendTimeout", 0, "System Send timeout (configures SendTimeout in AcceptOptions)");
            CommandLine.AddOption("recvTimeout", 0, "System Receive timeout (configures ReceiveTimeout in AcceptOptions)");
            CommandLine.AddOption("tcpDelay", false, "Turns off tcp_nodelay in BindOptions, enabling Nagle's");
            CommandLine.AddOption("highWaterMark", 0, "Sets the point that causes ETA to automatically flush");
            CommandLine.AddOption("if", "", "Name of network interface to use");
            CommandLine.AddOption("tickRate", 1000, "Ticks per second");
            CommandLine.AddOption("updateRate", 10000, "Update rate per second");
            CommandLine.AddOption("latencyUpdateRate", 10, "Latency update rate per second (can specify \"all\" to send latency in every update");
            CommandLine.AddOption("genericMsgRate", 0, "Generic Msg rate per second");
            CommandLine.AddOption("genericMsgLatencyRate", 0, "Latency Generic Msg rate per second (can specify \"all\" to send latency in every generic msg");
            CommandLine.AddOption("maxPackCount", 1, "Maximum number of messages packed in a buffer(when count > 1, eta PackBuffer() is used");
            CommandLine.AddOption("packBufSize", 6000, "If packing, sets size of buffer to use");
            CommandLine.AddOption("refreshBurstSize", 10, "Number of refreshes to send in a burst (controls granularity of time-checking)");
            CommandLine.AddOption("directWrite", false, "Sets direct socket write flag when using channel.write()");
            CommandLine.AddOption("serviceName", "DIRECT_FEED", "Service name");
            CommandLine.AddOption("serviceId", 1, "Service ID");
            CommandLine.AddOption("openLimit", 1000000, "Max number of items consumer may request per connection");
            CommandLine.AddOption("noDisplayStats", false, "Stop printout of stats to screen");

            CommandLine.AddOption("msgFile", "MsgData.xml", "Name of the file that specifies the data content in messages");
            CommandLine.AddOption("latencyFile", "ProvLatency.out", "Name of file for logging latency info");
            CommandLine.AddOption("summaryFile", "ProvSummary.out", "Name of file for logging summary info");
            CommandLine.AddOption("statsFile", "ProvStats", "Base name of file for logging periodic statistics");
            CommandLine.AddOption("writeStatsInterval", 5, "Controls how often stats are written to the file");
            CommandLine.AddOption("runTime", 360, "Runtime of the application, in seconds");
            CommandLine.AddOption("threads", 1, "Number of provider threads to create");
            CommandLine.AddOption("reactor", false, "Use the VA Reactor instead of the ETA Channel for sending and receiving");
            CommandLine.AddOption("pl", "", "List of supported WS sub-protocols in order of preference(',' | white space delineated)");
            CommandLine.AddOption("c", "", "Provider connection type.  Either \"socket\" or \"encrypted\"");
            CommandLine.AddOption("cert", defaultValue: "", "The server certificate file");
            CommandLine.AddOption("keyfile", defaultValue: "", "The server private key file");
        }

        /// <summary>
        ///
        /// </summary>
        /// <param name="args"></param>
        public static void Init(string[] args)
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
                Environment.Exit((int)PerfToolsReturnCode.FAILURE);
            }

            LatencyFilename = CommandLine.Value("latencyFile");
            SummaryFilename = CommandLine.Value("summaryFile");
            StatsFilename = CommandLine.Value("statsFile");
            DisplayStats = !CommandLine.BoolValue("noDisplayStats");
            InterfaceName = CommandLine.Value("if");
            PortNo = CommandLine.Value("p");
            TcpNoDelay = !CommandLine.BoolValue("tcpDelay");
            ServiceName = CommandLine.Value("serviceName");
            Cert = CommandLine.Value("cert");
            KeyFile = CommandLine.Value("keyfile");
            if (CommandLine.Value("c") == null || CommandLine.Value("c")!.Equals("socket"))
            {
                ConnectionType = ConnectionType.SOCKET;
            }
            else if (CommandLine.Value("c") != null && CommandLine.Value("c")!.Equals("encrypted"))
            {
                ConnectionType = ConnectionType.ENCRYPTED;
            }

            // Perf Test configuration
            MsgFilename = CommandLine.Value("msgFile");
            string? latencyUpdateRate = CommandLine.Value("latencyUpdateRate");
            string? latencyGenMsgRate = CommandLine.Value("genericMsgLatencyRate");
            DirectWrite = CommandLine.BoolValue("directWrite");
            UseReactor = CommandLine.BoolValue("reactor");
            try
            {
                RunTime = CommandLine.IntValue("runTime");
                WriteStatsInterval = CommandLine.IntValue("writeStatsInterval");
                ThreadCount = (CommandLine.IntValue("threads") > 1 ? CommandLine.IntValue("threads") : 1);
                GuaranteedOutputBuffers = CommandLine.IntValue("outputBufs");
                MaxOutputBuffers = CommandLine.IntValue("maxOutputBufs");
                MaxFragmentSize = CommandLine.IntValue("maxFragmentSize");
                ServiceId = CommandLine.IntValue("serviceId");
                OpenLimit = CommandLine.IntValue("openLimit");
                RefreshBurstSize = CommandLine.IntValue("refreshBurstSize");

                UpdatesPerSec = CommandLine.IntValue("updateRate");
                if ("all".Equals(latencyUpdateRate))
                    LatencyUpdateRate = ALWAYS_SEND_LATENCY_UPDATE;
                else
                    LatencyUpdateRate = int.Parse(latencyUpdateRate!);

                GenMsgsPerSec = CommandLine.IntValue("genericMsgRate");
                if ("all".Equals(latencyGenMsgRate))
                    LatencyGenMsgRate = ALWAYS_SEND_LATENCY_GENMSG;
                else
                    LatencyGenMsgRate = int.Parse(latencyGenMsgRate!);

                TicksPerSec = CommandLine.IntValue("tickRate");
                TotalBuffersPerPack = CommandLine.IntValue("maxPackCount");
                PackingBufferLength = CommandLine.IntValue("packBufSize");
                HighWaterMark = CommandLine.IntValue("highWaterMark");
                SendBufSize = CommandLine.IntValue("sendBufSize");
                RecvBufSize = CommandLine.IntValue("recvBufSize");
                SendTimeout = CommandLine.IntValue("sendTimeout");
                RecvTimeout = CommandLine.IntValue("recvTimeout");
            }
            catch (FormatException ile)
            {
                Console.Error.WriteLine("Invalid argument, number expected.\t");
                Console.Error.WriteLine(ile.Message);
                Console.Error.WriteLine();
                Console.Error.WriteLine(CommandLine.OptionHelpString());
                Environment.Exit((int)PerfToolsReturnCode.FAILURE);
            }

            if (TicksPerSec < 1)
            {
                Console.Error.WriteLine("Config Error: Tick rate cannot be less than 1.");
                Console.WriteLine(CommandLine.OptionHelpString());
                Environment.Exit((int)PerfToolsReturnCode.FAILURE);
            }

            if (LatencyUpdateRate > TicksPerSec)
            {
                Console.Error.WriteLine("Config Error: Latency Update Rate cannot be greater than tick rate.\n");
                Console.WriteLine(CommandLine.OptionHelpString());
                Environment.Exit((int)PerfToolsReturnCode.FAILURE);
            }

            if (LatencyGenMsgRate > TicksPerSec)
            {
                Console.Error.WriteLine("Config Error: Latency Generic Message Rate cannot be greater than tick rate.\n");
                Console.WriteLine(CommandLine.OptionHelpString());
                Environment.Exit((int)PerfToolsReturnCode.FAILURE);
            }

            if (WriteStatsInterval < 1)
            {
                Console.Error.WriteLine("Config error: Write Stats Interval cannot be less than 1.");
                Console.WriteLine(CommandLine.OptionHelpString());
                Environment.Exit((int)PerfToolsReturnCode.FAILURE);
            }

            if (ThreadCount > 8)
            {
                Console.Error.WriteLine("Config error: Thread count cannot be greater than 8.");
                Console.WriteLine(CommandLine.OptionHelpString());
                Environment.Exit((int)PerfToolsReturnCode.FAILURE);
            }

            if (TotalBuffersPerPack < 1)
            {
                Console.Error.WriteLine("Config error: Max Pack Count cannot be less than 1.");
                Console.WriteLine(CommandLine.OptionHelpString());
                Environment.Exit((int)PerfToolsReturnCode.FAILURE);
            }

            if (TotalBuffersPerPack > 1 && PackingBufferLength == 0)
            {
                Console.Error.WriteLine("Config Error: -maxPackCount set but -packBufSize is zero.\n\n");
                Environment.Exit((int)PerfToolsReturnCode.FAILURE);
            }

            if (UpdatesPerSec != 0 && UpdatesPerSec < TicksPerSec)
            {
                Console.Error.WriteLine("Config Error: Update rate cannot be less than total ticks per second (unless it is zero).\n\n");
                Console.WriteLine(CommandLine.OptionHelpString());
                Environment.Exit((int)PerfToolsReturnCode.FAILURE);
            }

            if (GenMsgsPerSec != 0 && GenMsgsPerSec < TicksPerSec)
            {
                Console.Error.WriteLine("Config Error: Generic message rate cannot be less than total ticks per second (unless it is zero).\n\n");
                Console.WriteLine(CommandLine.OptionHelpString());
                Environment.Exit((int)PerfToolsReturnCode.FAILURE);
            }

            UpdatesPerTick = UpdatesPerSec / TicksPerSec;
            UpdatesPerTickRemainder = UpdatesPerSec % TicksPerSec;

            GenMsgsPerTick = GenMsgsPerSec / TicksPerSec;
            GenMsgsPerTickRemainder = GenMsgsPerSec % TicksPerSec;

            CreateConfigString(null);
        }

        /// <summary>
        /// Converts configuration parameters to a string with effective bindOptions values
        /// </summary>
        /// <param name="bindOptions">the bind options to convert to string</param>
        /// <returns>the config string</returns>
        public static string? ConvertToString(BindOptions? bindOptions)
        {
            if (bindOptions != null)
            {
                CreateConfigString(bindOptions);
            }
            return ConfigString;
        }

        /// <summary>
        /// Converts configuration parameters to a string
        /// </summary>
        /// <returns>the configuration string</returns>
        public static string? ConvertToString()
        {
            return ConvertToString(null);
        }

        private static void CreateConfigString(BindOptions? bindOptions)
        {
            ConfigString = "--- TEST INPUTS ---\n\n" +
                "          Steady State Time: " + RunTime + " sec\n" +
                "                       Port: " + PortNo + "\n" +
                "			 Connection type: " + ConnectionType + "\n" +
                "               Thread Count: " + ThreadCount + "\n" +
                "             Output Buffers: " + GuaranteedOutputBuffers
                                                 + (bindOptions == null ? "" : "(effective = " + bindOptions.GuaranteedOutputBuffers + ")")
                                                 + "\n" +
                "         Max Output Buffers: " + MaxOutputBuffers + ((MaxOutputBuffers > 0) ? "" : "(use default)")
                                                 + (bindOptions == null ? "" : "(effective = " + bindOptions.MaxOutputBuffers + ")")
                                                 + "\n" +
                "          Max Fragment Size: " + MaxFragmentSize + "\n" +
                "           Send Buffer Size: " + SendBufSize + ((SendBufSize > 0) ? " bytes" : "(use default)") + "\n" +
                "           Recv Buffer Size: " + RecvBufSize + ((RecvBufSize > 0) ? " bytes" : "(use default)") + "\n" +
                "               Send Timeout: " + SendTimeout + ((SendTimeout > 0) ? " ms" : "(use default)") + "\n" +
                "            Receive Timeout: " + RecvTimeout + ((RecvTimeout > 0) ? " ms" : "(use default)") + "\n" +
                "             Interface Name: " + (InterfaceName != null && InterfaceName.Length > 0 ? InterfaceName : "(use default)") + "\n" +
                "                Tcp_NoDelay: " + (TcpNoDelay ? "Yes" : "No") + "\n" +
                "                  Tick Rate: " + TicksPerSec + "\n" +
                "          Use Direct Writes: " + (DirectWrite ? "Yes" : "No") + "\n" +
                "            High Water Mark: " + HighWaterMark + ((HighWaterMark > 0) ? " bytes" : "(use default)") + "\n" +
                "               Latency File: " + LatencyFilename + "\n" +
                "               Summary File: " + SummaryFilename + "\n" +
                "       Write Stats Interval: " + WriteStatsInterval + "\n" +
                "                 Stats File: " + StatsFilename + "\n" +
                "              Display Stats: " + DisplayStats + "\n" +
                "                Update Rate: " + UpdatesPerSec + "\n" +
                "        Latency Update Rate: " + LatencyUpdateRate + "\n" +
                "           Generic Msg Rate: " + GenMsgsPerSec + "\n" +
                "   Generic Msg Latency Rate: " + LatencyGenMsgRate + "\n" +
                "         Refresh Burst Size: " + RefreshBurstSize + "\n" +
                "                  Data File: " + MsgFilename + "\n" +
                "                    Packing: " + (TotalBuffersPerPack <= 1 ? "No" : "Yes(max " + TotalBuffersPerPack + " per pack, " + PackingBufferLength + " buffer size)") + "\n" +
                "               Service Name: " + ServiceName + "\n" +
                "                 Service ID: " + ServiceId + "\n" +
                "                 Open Limit: " + OpenLimit + "\n" +
                "                Use Reactor: " + (UseReactor ? "Yes" : "No") + "\n";
        }
    }
}
