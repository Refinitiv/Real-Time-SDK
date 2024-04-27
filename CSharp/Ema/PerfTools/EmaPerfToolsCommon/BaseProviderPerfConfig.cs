/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2024 Refinitiv. All rights reserved.              --
 *|-----------------------------------------------------------------------------
 */

using LSEG.Ema.PerfTools.Common;
using System.Net;

namespace LSEG.Ema.PerfTools.Common
{
    public abstract class BaseProviderPerfConfig
    {
        public const int ALWAYS_SEND_LATENCY_UPDATE = -1;
        public const int ALWAYS_SEND_LATENCY_GENMSG = -1;

        private const string DEFAULT_POSITION_VALUE = "1.1.1.1/net";

        /// <summary>
        /// Configuration string which will be print after start
        /// </summary>
        public string m_ConfigString;
        /// <summary>
        /// prefix of provider name in the EMAConfig.xml
        /// </summary>
        public string? ProviderName { get; internal set; }
        /// <summary>
        /// Defines whenever provider name was specified
        /// </summary>
        public bool UseCustomName { get; internal set; }
        /// <summary>
        /// Total update rate per tick
        /// </summary>
        public int UpdatesPerTick { get; internal set; }  
        /// <summary>
        /// Updates per tick remainder
        /// </summary>
        public int UpdatesPerTickRemainder { get; internal set; } 
        /// <summary>
        /// Controls granularity of update bursts
        /// </summary>
        public int TicksPerSec { get; internal set; }
        /// <summary>
        /// Total update rate per second(includes latency updates).
        /// </summary>
        public int UpdatesPerSec { get; internal set; }
        /// <summary>
        /// Number of refreshes to send in a burst(controls granularity of time-checking)
        /// </summary>
        public int RefreshBurstSize { get; internal set; }
        /// <summary>
        /// Total latency update rate per second
        /// </summary>
        public int LatencyUpdateRate { get; internal set; }
        /// <summary>
        /// Data file. Describes the data to use when encoding messages.
        /// </summary>
        public string? MsgFilename { get; internal set; }
        /// <summary>
        /// Number of provider threads to create.
        /// </summary>
        public int ThreadCount { get; internal set; }
        /// <summary>
        /// Time application runs before exiting
        /// </summary>
        public int RunTime { get; internal set; }
        /// <summary>
        /// Controls how often statistics are written
        /// </summary>
        public int WriteStatsInterval { get; internal set; }
        /// <summary>
        /// Controls whether stats appear on the screen
        /// </summary>
        public bool DisplayStats { get; internal set; }
        /// <summary>
        /// Controls opportunity for manually dispatching of the incoming messages
        /// </summary>
        public bool UseUserDispatch { get; internal set; }
        /// <summary>
        /// Provider's Position
        /// </summary>
        public string Position { get; internal set; } = DEFAULT_POSITION_VALUE;
        /// <summary>
        /// Summary file
        /// </summary>
        public string? SummaryFilename { get; protected set; }
        /// <summary>
        /// Stats file
        /// </summary>
        public string? StatsFilename { get; protected set; }

        /// <summary>
        /// Server certificate file
        /// </summary>
        public string? CertificateFilename { get; internal set; }

        /// <summary>
        /// Server private key file
        /// </summary>
        public string? PrivateKeyFilename { get; internal set; }

        /// <summary>
        /// Count of how many messages to pack in a single PackedMsg
        /// </summary>
        public int MessagePackingCount { get; internal set; }

        /// <summary>
        /// Sets the buffer size for PackedMsg(in bytes)
        /// </summary>
        public int MessagePackingBufferSize { get; internal set; }

        public BaseProviderPerfConfig()
        {
            CommandLine.AddOption("providerName", DefaultProviderName(), "Base name of provider configuration in EmaConfig.xml");
            CommandLine.AddOption("refreshBurstSize", 10, "Refreshes");
            CommandLine.AddOption("tickRate", 1000, "Ticks per second");
            CommandLine.AddOption("updateRate", 100000, "Update rate per second");
            CommandLine.AddOption("noDisplayStats", false, "Stop printout of stats to screen");
            CommandLine.AddOption("threads", 1, "Amount of executable threads");
            CommandLine.AddOption("runTime", 360, "Runtime of the application, in seconds");

            CommandLine.AddOption("msgFile", "MsgData.xml", "Name of the file that specifies the data content in messages");
            CommandLine.AddOption("writeStatsInterval", 5, "Controls how ofthen stats are written to the file.");

            CommandLine.AddOption("useUserDispatch", false, "True if user wants to dispatch EMA manually");

            CommandLine.AddOption("summaryFile", DefaultSummaryFileName(), "Name for logging summary info");
            CommandLine.AddOption("statsFile", DefaultStatsFileName(), "Base name of file for logging periodic statistics");

            CommandLine.AddOption("cert", string.Empty, "File containing the server certificate for encryption");
            CommandLine.AddOption("key", string.Empty, "File containing the server private key for encryption");

            CommandLine.AddOption("maxPackCount", 1, "Maximum number of messages packed in a buffer(when count > 1, ETA PackBuffer() is used");
            CommandLine.AddOption("packBufSize", 6000, "If packing, sets size of buffer to use");

            m_ConfigString = string.Empty;
        }

        /// <summary>
        /// Parses command-line arguments to fill in the application's configuration structures.
        /// </summary>
        /// <param name="args">command line arguments</param>
        public void Init(string[] args)
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
                Console.Error.WriteLine(CommandLine.OptionHelpString());
                Environment.Exit(-1);
            }
            FillProperties();
            CreateConfigString();
        }

        protected virtual void FillProperties()
        {
            ProviderName = CommandLine.Value("providerName");
            UseCustomName = !DefaultProviderName().Equals(ProviderName);
            SummaryFilename = CommandLine.Value("summaryFile");
            StatsFilename = CommandLine.Value("statsFile");
            DisplayStats = !CommandLine.BoolValue("noDisplayStats");

            // Perf Test configuration
            MsgFilename = CommandLine.Value("msgFile");
            string? latencyUpdateRate = CommandLine.Value("latencyUpdateRate");
            try
            {
                RunTime = CommandLine.IntValue("runTime");
                WriteStatsInterval = CommandLine.IntValue("writeStatsInterval");
                ThreadCount = Math.Max(CommandLine.IntValue("threads"), 1);
                RefreshBurstSize = CommandLine.IntValue("refreshBurstSize");

                UpdatesPerSec = CommandLine.IntValue("updateRate");
                if ("all".Equals(latencyUpdateRate))
                {
                    LatencyUpdateRate = ALWAYS_SEND_LATENCY_UPDATE;
                }
                else if (latencyUpdateRate != null)
                {
                    LatencyUpdateRate = int.Parse(latencyUpdateRate);
                }

                TicksPerSec = CommandLine.IntValue("tickRate");
                UseUserDispatch = CommandLine.BoolValue("useUserDispatch");

                MessagePackingCount = CommandLine.IntValue("maxPackCount");
                MessagePackingBufferSize = CommandLine.IntValue("packBufSize");
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
                Console.WriteLine(CommandLine.OptionHelpString());
                Environment.Exit(-1);
            }

            if (LatencyUpdateRate > TicksPerSec)
            {
                Console.Error.WriteLine("Config Error: Latency Update Rate cannot be greater than tick rate.\n");
                Console.WriteLine(CommandLine.OptionHelpString());
                Environment.Exit(-1);
            }

            if (WriteStatsInterval < 1)
            {
                Console.Error.WriteLine("Config error: Write Stats Interval cannot be less than 1.");
                Console.WriteLine(CommandLine.OptionHelpString());
                Environment.Exit(-1);
            }

            if (ThreadCount > 8)
            {
                Console.Error.WriteLine("Config error: Thread count cannot be greater than 8.");
                Console.WriteLine(CommandLine.OptionHelpString());
                Environment.Exit(-1);
            }


            if (UpdatesPerSec != 0 && UpdatesPerSec < TicksPerSec)
            {
                Console.Error.WriteLine("Config Error: Update rate cannot be less than total ticks per second (unless it is zero).\n\n");
                Console.WriteLine(CommandLine.OptionHelpString());
                Environment.Exit(-1);
            }

            if (MessagePackingCount < 1)
            {
                Console.Error.WriteLine("Config error: Message Packing Count cannot be less than 1.");
                Console.WriteLine(CommandLine.OptionHelpString());
                Environment.Exit(-1);
            }

            if (MessagePackingBufferSize < 1)
            {
                Console.Error.WriteLine("Config error: Message Packing Buffer Size cannot be less than 1.");
                Console.WriteLine(CommandLine.OptionHelpString());
                Environment.Exit(-1);
            }

            UpdatesPerTick = UpdatesPerSec / TicksPerSec;
            UpdatesPerTickRemainder = UpdatesPerSec % TicksPerSec;

            InitPosition();
        }

        // Create config string.
        protected abstract void CreateConfigString();

        private void InitPosition()
        {
            try
            {
                Position = BitConverter.ToUInt32(Dns.GetHostAddresses(Dns.GetHostName())
						.Where(ip => ip.AddressFamily == System.Net.Sockets.AddressFamily.InterNetwork)!.FirstOrDefault()?.GetAddressBytes()) + "/"
                        + Dns.GetHostName();
            }
            catch (Exception)
            {
                Position = DEFAULT_POSITION_VALUE;
            }
        }

        protected abstract string DefaultProviderName();
        protected abstract string DefaultSummaryFileName();
        protected abstract string DefaultStatsFileName();

        /// <summary>
        /// Converts configuration parameters to a string with effective bindOptions values.
        /// </summary>
        /// <returns>String representation of the current config</returns>
        public override string ToString()
        {
            return m_ConfigString;
        }
    }
}
