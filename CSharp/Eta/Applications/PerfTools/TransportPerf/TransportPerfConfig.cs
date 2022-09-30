/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2022 Refinitiv. All rights reserved.              --
 *|-----------------------------------------------------------------------------
 */


using System;
using System.Runtime.InteropServices;
using Refinitiv.Eta.Example.Common;
using Refinitiv.Eta.Transports;

namespace Refinitiv.Eta.PerfTools.TransportPerf
{
    /// <summary>
    /// Configures the TransportPerf application.
    /// </summary>
    public class TransportPerfConfig
    {
        /// <summary>
        /// Types of TransportPerf
        /// </summary>
        public enum ApplicationType
        {
            SERVER = 1,
            CLIENT = 2
        }

        private static string? m_ConfigString;

        /// <summary>
        /// Gets time application runs before exiting
        /// </summary>
        public static int RunTime { get; private set; }

        /// <summary>
        /// Gets number of threads that handle connections
        /// </summary>
        public static int ThreadCount { get; private set; }

        /// <summary>
        /// Gets ype of application(server, client)
        /// </summary>
        public static ApplicationType AppType { get; private set; }


        private static string? m_AppTypeString;

        /// <summary>
        /// Gets whether to reflect received messages instead of generating them.
        /// </summary>
        public static bool ReflectMsgs { get; private set; }

        /// <summary>
        /// Gets whether the application will continually read rather than using notification.
        /// Messages cannot be sent in this mode.
        /// </summary>
        public static bool BusyRead { get; private set; }

        /// <summary>
        /// Gets type of connection
        /// </summary>
        public static Transports.ConnectionType ConnectionType { get; private set; }
        private static string? m_ConnectionTypeString;

        /// <summary>
        /// Gets host name
        /// </summary>
        public static string? HostName { get; private set; }

        /// <summary>
        /// Gets port number
        /// </summary>
        public static string? PortNo { get; private set; }

        /// <summary>
        /// Gets name of network interface to bind to
        /// </summary>
        public static string? InterfaceName { get; private set; }

        /// <summary>
        /// Gets whether the Nagle's algorithm is enabled
        /// </summary>
        public static bool TcpNoDelay { get; private set; }

        /// <summary>
        /// Gets guaranteed output buffers.
        /// </summary>
        public static int GuaranteedOutputBuffers { get; private set; }

        /// <summary>
        /// Gets max fragment size
        /// </summary>
        public static int MaxFragmentSize { get; private set; }

        /// <summary>
        /// Gets the point tha causes ETA to automatically flush.
        /// </summary>
        public static int HighWaterMark { get; private set; }

        /// <summary>
        /// Gets system Send Buffer Size.
        /// </summary>
        public static int SendBufSize { get; private set; }

        /// <summary>
        /// Gets system Receive Buffer Size.
        /// </summary>
        public static int RecvBufSize { get; private set; }

        /// <summary>
        /// Gets name of summary file
        /// </summary>
        public static string? SummaryFileName { get; private set; }

        /// <summary>
        /// Gets how often statistics are written
        /// </summary>
        public static int WriteStatsInterval { get; private set; }

        /// <summary>
        /// Gets whether stats appear on the screen
        /// </summary>
        public static bool DisplayStats { get; private set; }

        /// <summary>
        /// Gets type of compression to use, if any
        /// </summary>
        public static Transports.CompressionType CompressionType { get; private set; }
        private static string? m_CompressionTypeString;

        /// <summary>
        /// Gets compression level, optional depending on compression algorithm used
        /// </summary>
        public static int CompressionLevel { get; private set; }

        /// <summary>
        /// Gets whether to support proxy connection
        /// </summary>
        public static bool Proxy { get; private set; }

        /// <summary>
        /// Gets proxy hostname or address
        /// </summary>
        public static string? ProxyHost { get; private set; }

        /// <summary>
        /// Gets proxy port
        /// </summary>
        public static int ProxyPort { get; private set; }

        /// <summary>
        /// Gets proxy username
        /// </summary>
        public static string? ProxyUserName { get; private set; }

        /// <summary>
        /// Gets proxy password
        /// </summary>
        public static string? ProxyPasswd { get; private set; }

        public static string DIR_SEPERATOR = RuntimeInformation.IsOSPlatform(OSPlatform.Windows) ?
           "\\" : "/";

        static TransportPerfConfig()
        {
            try
            {
                string folderPath = Environment.GetFolderPath(Environment.SpecialFolder.UserProfile);

                SummaryFileName = $"{folderPath}{DIR_SEPERATOR}TransportSummary_{new Random().NextInt64()}.out";
            }
            catch (Exception)
            {
                SummaryFileName = "TransportSummary.out";
            }

            CommandLine.ProgName("TransportPerf");
            CommandLine.AddOption("runTime", 300, "Runtime of the application, in seconds");
            CommandLine.AddOption("summaryFile", SummaryFileName, "Name of file for logging summary info");
            CommandLine.AddOption("writeStatsInterval", 5, "Controls how often stats are written to the file");
            CommandLine.AddOption("noDisplayStats", false, "Stop printout of stats to screen");
            CommandLine.AddOption("threads", 1, "Number of transport threads to create");
            CommandLine.AddOption("connType", "socket", "Type of connection(\"socket\", \"encrypted (client only)\")");
            CommandLine.AddOption("reflectMsgs", false, "Reflect received messages back, rather than generating our own");
            CommandLine.AddOption("outputBufs", 5000, "Number of output buffers(configures GuaranteedOutputBuffers in BindOptions/ConnectOptions)");
            CommandLine.AddOption("maxFragmentSize", 6144, " Max size of buffers(configures MaxFragmentSize in BindOptions/ConnectOptions)");
            CommandLine.AddOption("sendBufSize", 0, "System Send Buffer Size(configures SysSendBufSize in BindOptions/ConnectOptions)");
            CommandLine.AddOption("recvBufSize", 0, "System Receive Buffer Size(configures sysRecvBufSize in BindOptions/ConnectOptions)");
            CommandLine.AddOption("compressionType", "none", "Type of compression to use(\"none\", \"zlib\", \"lz4\")");
            CommandLine.AddOption("compressionLevel", 6, "Level of compression");
            CommandLine.AddOption("highWaterMark", 0, "Sets the point that causes ETA to automatically flush");
            CommandLine.AddOption("if", "", "Name of network interface to use");
            CommandLine.AddOption("h", "localhost", "Name of host for socket-based connections");
            CommandLine.AddOption("p", "14002", "Port number for socket-based connections");
            CommandLine.AddOption("tcpDelay", false, "Turns off tcp_nodelay in BindOptions, enabling Nagle's");
            CommandLine.AddOption("appType", "server", "Type of application(server, client)");
            CommandLine.AddOption("busyRead", false, "Continually read instead of using notification");
            CommandLine.AddOption("proxy", false, "Use proxy connection");
            CommandLine.AddOption("ph", "localhost", "Address of proxy host");
            CommandLine.AddOption("pp", "14002", "Port number for proxy server");
            CommandLine.AddOption("plogin", "John.Doe", "Login id on proxy server");
            CommandLine.AddOption("ppasswd", "", "Password on proxy server");
            // TransportThreadConfig
            CommandLine.AddOption("tickRate", 1000, "Ticks per second");
            CommandLine.AddOption("pack", 1, "Number of messages packed in a buffer(when count > 1, Channel.packBuffer() is used)");
            CommandLine.AddOption("msgRate", 100000, "Message rate per second");
            CommandLine.AddOption("msgSize", 76, "Size of messages to send");
            CommandLine.AddOption("latencyMsgRate", 10, "Latency Message rate (can specify \"all\" to send it as every msg)");
            CommandLine.AddOption("directWrite", false, "Sets direct socket write flag when using Channel.write()");
            CommandLine.AddOption("latencyFile", "", "Base name of file for logging latency");
            CommandLine.AddOption("statsFile", "TransportStats", "Base name of file for logging periodic statistics");

        }

        private TransportPerfConfig()
        {
        }

        /// <summary>
        /// Parses command-line arguments to fill in the application's configuration
        /// </summary>
        /// <param name="args">The command line arguments</param>
        public static void Init(string[] args)
        {
            try
            {
                CommandLine.ParseArgs(args);
            }
            catch (Exception exp)
            {
                Console.WriteLine("Error loading command line arguments:\t");
                Console.WriteLine(exp.Message);
                Console.WriteLine();
                Console.WriteLine(CommandLine.OptionHelpString());
                Environment.Exit(-1);
            }

            try
            {
                RunTime = CommandLine.IntValue("runTime");
                SummaryFileName = CommandLine.Value("summaryFile");
                TransportThreadConfig.StatsFileName = CommandLine.Value("statsFile");
                WriteStatsInterval = CommandLine.IntValue("writeStatsInterval");
                DisplayStats = !CommandLine.BoolValue("noDisplayStats");
                ThreadCount = CommandLine.IntValue("threads");
                if (ThreadCount < 1)
                {
                    ThreadCount = -1;
                    Console.WriteLine(CommandLine.OptionHelpString());
                    Environment.Exit(-1);
                }
                m_ConnectionTypeString = CommandLine.Value("connType");
                if (m_ConnectionTypeString!.Equals("socket"))
                {
                    ConnectionType = ConnectionType.SOCKET;
                }
                else if (m_ConnectionTypeString!.Equals("encrypted"))
                {
                    ConnectionType = ConnectionType.ENCRYPTED;
                }
                else
                {
                    Console.WriteLine("Config Error: Unknown connectionType. Valid types are \"socket\", \"encrypted\"\n");
                    Console.WriteLine(CommandLine.OptionHelpString());
                    Environment.Exit(-1);
                }
                m_AppTypeString = CommandLine.Value("appType");
                if (m_AppTypeString!.Equals("server"))
                {
                    AppType = ApplicationType.SERVER;
                }
                else if (m_AppTypeString.Equals("client"))
                {
                    AppType = ApplicationType.CLIENT;
                }
                else
                {
                    Console.WriteLine(CommandLine.OptionHelpString());
                    Environment.Exit(-1);
                }
                ReflectMsgs = CommandLine.BoolValue("reflectMsgs");
                TransportThreadConfig.MsgSize = CommandLine.IntValue("msgSize");
                BusyRead = CommandLine.BoolValue("busyRead");
                GuaranteedOutputBuffers = CommandLine.IntValue("outputBufs");
                MaxFragmentSize = CommandLine.IntValue("maxFragmentSize");
                SendBufSize = CommandLine.IntValue("sendBufSize");
                RecvBufSize = CommandLine.IntValue("recvBufSize");
                HighWaterMark = CommandLine.IntValue("highWaterMark");
                TransportThreadConfig.LatencyLogFileName = CommandLine.Value("latencyFile");
                if (!string.IsNullOrEmpty(TransportThreadConfig.LatencyLogFileName))
                {
                    TransportThreadConfig.LogLatencyToFile = true;
                }
                m_CompressionTypeString = CommandLine.Value("compressionType");
                if (m_CompressionTypeString!.Equals("none"))
                {
                    CompressionType = CompressionType.NONE;
                }
                else if (m_CompressionTypeString.Equals("zlib"))
                {
                    CompressionType = CompressionType.ZLIB;
                }
                else if (m_CompressionTypeString.Equals("lz4"))
                {
                    CompressionType = CompressionType.LZ4;
                }
                else
                {
                    Console.WriteLine(CommandLine.OptionHelpString());
                    Environment.Exit(-1);
                }
                CompressionLevel = CommandLine.IntValue("compressionLevel");
                InterfaceName = CommandLine.Value("if");
                PortNo = CommandLine.Value("p");
                HostName = CommandLine.Value("h");

                TcpNoDelay = !CommandLine.BoolValue("tcpDelay");

                TransportThreadConfig.MsgsPerSec = CommandLine.IntValue("msgRate");
                if ("all".Equals(CommandLine.Value("latencyMsgRate")))
                    TransportThreadConfig.LatencyMsgsPerSec = TransportThreadConfig.ALWAYS_SEND_LATENCY_MSG;
                else
                    TransportThreadConfig.LatencyMsgsPerSec = CommandLine.IntValue("latencyMsgRate");
                TransportThreadConfig.TicksPerSec =CommandLine.IntValue("tickRate");
                TransportThreadConfig.TotalBuffersPerPack = CommandLine.IntValue("pack");
                if (CommandLine.BoolValue("directWrite"))
                {
                    TransportThreadConfig.WriteFlags |= WriteFlags.DIRECT_SOCKET_WRITE;
                }

                Proxy = CommandLine.BoolValue("proxy");

                ProxyHost = CommandLine.Value("ph");
                ProxyPort = CommandLine.IntValue("pp");
                ProxyUserName = CommandLine.Value("plogin");
                ProxyPasswd = CommandLine.Value("ppasswd");
            }
            catch (Exception exp)
            {
                Console.WriteLine("Invalid argument.\t");
                Console.WriteLine(exp.Message);
                Console.WriteLine();
                Console.WriteLine(CommandLine.OptionHelpString());
                Environment.Exit(-1);
            }

            if (ThreadCount > 8)
            {
                Console.WriteLine("Config error: Thread count cannot be greater than 8.");
                Console.WriteLine(CommandLine.OptionHelpString());
                Environment.Exit(-1);
            }

            if (ThreadCount < 0)
            {
                Console.WriteLine("Config error: Thread count cannot be less than 0.");
                Console.WriteLine(CommandLine.OptionHelpString());
                Environment.Exit(-1);
            }

            if (TransportThreadConfig.TicksPerSec < 1)
            {
                Console.WriteLine("Config Error: Tick rate cannot be less than 1.\n");
                Console.WriteLine(CommandLine.OptionHelpString());
                Environment.Exit(-1);
            }

            if (SendBufSize < 0)
            {
                Console.WriteLine("Config error: Send Buffer Size cannot be less than 0.");
                Console.WriteLine(CommandLine.OptionHelpString());
                Environment.Exit(-1);
            }

            if (RecvBufSize < 0)
            {
                Console.WriteLine("Config error: Receive Buffer Size cannot be less than 0.");
                Console.WriteLine(CommandLine.OptionHelpString());
                Environment.Exit(-1);
            }

            /* Conditions */
            if (WriteStatsInterval < 1)
            {
                Console.WriteLine("Config error: Write Stats Interval cannot be less than 1.\n");
                Console.WriteLine(CommandLine.OptionHelpString());
                Environment.Exit(-1);
            }

            if (GuaranteedOutputBuffers < 0)
            {
                Console.WriteLine("Config Error: Cannot specify less than 0 guarenteed output buffers.\n\n");
                Console.WriteLine(CommandLine.OptionHelpString());
                Environment.Exit(-1);
            }

            if (TransportThreadConfig.TotalBuffersPerPack > 1 && ReflectMsgs)
            {
                Console.WriteLine("Config Error: Reflection does not pack messages.\n");
                Console.WriteLine(CommandLine.OptionHelpString());
                Environment.Exit(-1);
            }

            if (TransportThreadConfig.LatencyMsgsPerSec > TransportThreadConfig.MsgsPerSec)
            {
                Console.WriteLine("Config Error: Latency msg rate cannot be greater than total msg rate. \n\n");
                Console.WriteLine(CommandLine.OptionHelpString());
                Environment.Exit(-1);
            }

            if (TransportThreadConfig.LatencyMsgsPerSec > TransportThreadConfig.TicksPerSec)
            {
                Console.WriteLine("Config Error: Latency msg rate cannot be greater than total ticks per second. \n\n");
                Console.WriteLine(CommandLine.OptionHelpString());
                Environment.Exit(-1);
            }

            if (TransportThreadConfig.MsgsPerSec != 0 && TransportThreadConfig.MsgsPerSec < TransportThreadConfig.TicksPerSec)
            {
                Console.WriteLine("Config Error: Update rate cannot be less than total ticks per second(unless it is zero).\n\n");
                Console.WriteLine(CommandLine.OptionHelpString());
                Environment.Exit(-1);
            }

            /* Must have room for sending both the timestamp and sequence number. */
            if (TransportThreadConfig.MsgSize < 16)
            {
                Console.WriteLine("Config Error: Message size must be at least 16.\n\n");
                Console.WriteLine(CommandLine.OptionHelpString());
                Environment.Exit(-1);
            }

            if (TransportThreadConfig.TotalBuffersPerPack < 1)
            {
                Console.WriteLine("Config Error: Cannot specify less than 1 buffer per pack.\n\n");
                Console.WriteLine(CommandLine.OptionHelpString());
                Environment.Exit(-1);
            }

            /* Determine msg rates on per-tick basis */
            TransportThreadConfig.MsgsPerTick = TransportThreadConfig.MsgsPerSec / TransportThreadConfig.TicksPerSec;
            TransportThreadConfig.MsgsPerTickRemainder = TransportThreadConfig.MsgsPerSec % TransportThreadConfig.TicksPerSec;

            CreateConfigString();
        }

        private static void CreateConfigString()
        {
            m_ConfigString = "--- TEST INPUTS ---\n\n" +
                    "                Runtime: " + RunTime + " sec\n" +
                    "        Connection Type: " + ConnectionType + "\n" +
                                                  UnicastConfig() +
                    "               App Type: " + (AppType == ApplicationType.SERVER ? "server" : "client") + "\n" +
                    "           Thread Count: " + ThreadCount + "\n" +
                    "              Busy Read: " + (BusyRead ? "Yes" : "No") + "\n" +
                    "               Msg Size: " + TransportThreadConfig.MsgSize + "\n" +
                    "               Msg Rate: " + ((ReflectMsgs) ? "0(reflecting)" : TransportThreadConfig.MsgsPerSec) + "\n" +
                    "       Latency Msg Rate: " + ((ReflectMsgs) ? "0(reflecting)" : TransportThreadConfig.LatencyMsgsPerSec) + "\n" +
                    "         Output Buffers: " + GuaranteedOutputBuffers + "\n" +
                    "      Max Fragment Size: " + MaxFragmentSize + "\n" +
                    "       Send Buffer Size: " + SendBufSize + ((SendBufSize > 0) ? " bytes" : "(use default)") + "\n" +
                    "       Recv Buffer Size: " + RecvBufSize + ((RecvBufSize > 0) ? " bytes" : "(use default)") + "\n" +
                    "        High Water Mark: " + HighWaterMark + ((HighWaterMark > 0) ? " bytes" : "(use default)") + "\n" +
                    "       Compression Type: " + CompressionType + "\n" +
                    "      Compression Level: " + CompressionLevel + "\n" +
                    "         Interface Name: " + (!string.IsNullOrEmpty(InterfaceName) ? InterfaceName : "(use default)") + "\n" +
                    "            Tcp_NoDelay: " + (TcpNoDelay ? "Yes" : "No") + "\n" +
                    "              Tick Rate: " + TransportThreadConfig.TicksPerSec + "\n" +
                    "      Use Direct Writes: " + ((TransportThreadConfig.WriteFlags & WriteFlags.DIRECT_SOCKET_WRITE) > 0 ? "Yes" : "No") + "\n" +
                    "       Latency Log File: " + (!string.IsNullOrEmpty(TransportThreadConfig.LatencyLogFileName) ? TransportThreadConfig.LatencyLogFileName : "(none)") + "\n" +
                    "           Summary File: " + SummaryFileName + "\n" +
                    "             Stats File: " + TransportThreadConfig.StatsFileName + "\n" +
                    "   Write Stats Interval: " + WriteStatsInterval + "\n" +
                    "          Display Stats: " + (DisplayStats ? "Yes" : "No") + "\n" +
                    "                Packing: " + (TransportThreadConfig.TotalBuffersPerPack <= 1 ? "No" : "Yes(" + TransportThreadConfig.TotalBuffersPerPack + " per pack)") + "\n";

            if (Proxy)
            {
                m_ConfigString += ProxyConfig();
            }
        }

        private static string UnicastConfig()
        {
            return "               Hostname: " + HostName + "\n" +
                   "                   Port: " + PortNo;
        }

        private static string ProxyConfig()
        {
            String proxyString = "";
            proxyString = "         Proxy Hostname: " + ProxyHost + "\n" +
                          "             Proxy Port: " + ProxyPort + "\n" +
                          "         Proxy UserName: " + ProxyUserName + "\n" +
                          "         Proxy Password: " + ProxyPasswd + "\n";
            return proxyString;
        }

        /// <summary>
        /// Converts configuration parameters to a string.
        /// </summary>
        /// <returns>The string</returns>
        public static string? ConfigString()
        {
            return m_ConfigString;
        }
    }
}
