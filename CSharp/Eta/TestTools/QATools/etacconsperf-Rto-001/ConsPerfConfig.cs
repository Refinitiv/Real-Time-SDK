/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2022-2023 Refinitiv. All rights reserved.              --
 *|-----------------------------------------------------------------------------
 */

using LSEG.Eta.Example.Common;
using LSEG.Eta.PerfTools.Common;
using LSEG.Eta.Transports;

namespace LSEG.Eta.PerfTools.ConsPerf
{
	/// <summary>
	/// Provides configuration that is not specific to any particular handler.
	/// </summary>
	public class ConsPerfConfig
    {
		public const int DEFAULT_THREAD_COUNT = 1;
		//APIQA
		private static string DEFAULT_HOST = "localhost";
		private static string DEFAULT_PORT = "14002";
		//END APIQA
		public string? ConfigString { get; set; }
		public int MaxThreads { get; set; }

		/* APPLICATION configuration */
		/// <summary>
		/// Time application runs before exiting
		/// </summary>
		public int SteadyStateTime { get; set; }
		/// <summary>
		/// Time before the latency is calculated
		/// </summary>
		public int DelaySteadyStateCalc { get; set; }
		/// <summary>
		/// Main loop ticks per second
		/// </summary>
		public int TicksPerSec { get; set; }
		/// <summary>
		/// Number of threads that handle connections
		/// </summary>
		public int ThreadCount { get; set; }
		/// <summary>
		/// File of names to use when requesting items
		/// </summary>
		public string? ItemFilename { get; set; }
		/// <summary>
		/// File of data to use for message payloads
		/// </summary>
		public string? MsgFilename { get; set; }

		/// <summary>
		/// Whether to log update latency information to a file
		/// </summary>
		public bool LogLatencyToFile { get; set; }
		/// <summary>
		/// Name of the latency log file
		/// </summary>
		public string? LatencyLogFilename { get; set; }
		/// <summary>
		/// Name of the summary log file
		/// </summary>
		public string? SummaryFilename { get; set; }
		/// <summary>
		/// Name of the statistics log file
		/// </summary>
		public string? StatsFilename { get; set; }
		/// <summary>
		/// Controls how often statistics are written
		/// </summary>
		public int WriteStatsInterval { get; set; }
		/// <summary>
		/// Controls whether stats appear on the screen
		/// </summary>
		public bool DisplayStats { get; set; }
		/// <summary>
		/// Rate at which the consumer will send out item requests
		/// </summary>
		public int ItemRequestsPerSec { get; set; }

		/* CONNECTION configuration */
		/// <summary>
		/// Type of connection
		/// </summary>
		public ConnectionType ConnectionType { get; set; }
		/// <summary>
		/// HostName, if using Transport.connect()
		/// </summary>
		public string? HostName { get; set; }
		/// <summary>
		/// Port number
		/// </summary>
		public string? PortNo { get; set; }
		/// <summary>
		/// Name of interface
		/// </summary>
		public string? InterfaceName { get; set; }
		/// <summary>
		/// Guaranteed Output Buffers
		/// </summary>
		public int GuaranteedOutputBuffers { get; set; }
		/// <summary>
		/// Input Buffers
		/// </summary>
		public int NumInputBuffers { get; set; }
		/// <summary>
		/// System Send Buffer Size
		/// </summary>
		public int SendBufSize { get; set; }
		/// <summary>
		/// System Receive Buffer Size
		/// </summary>
		public int RecvBufSize { get; set; }
		/// <summary>
		/// System Send timeout
		/// </summary>
		public int SendTimeout { get; set; }
		/// <summary>
		/// System Receive timeout
		/// </summary>
		public int RecvTimeout { get; set; }
		/// <summary>
		/// Sets the point which will cause ETA to automatically flush
		/// </summary>
		public int HighWaterMark { get; set; }
		/// <summary>
		/// Enable/Disable Nagle's algorithm
		/// </summary>
		public bool TcpNoDelay { get; set; }
		/// <summary>
		/// Determines whether to request all items as snapshots
		/// </summary>
		public bool RequestSnapshots { get; set; }

		/// <summary>
		/// Username used when logging in
		/// </summary>
		public string? Username { get; set; }
		/// <summary>
		/// Name of service to request items from
		/// </summary>
		public string? ServiceName { get; set; }

		//APIQA
		private bool UseReactor { get; set; }         /* Use the VA Reactor instead of the ETA Channel for sending and receiving. */
		private string? ClientId { get; set; }               /* Client id */
		private string? Password { get; set; }               /* Password */
		private string? ClientSecret { get; set; }           /* Client secret */
		private string? ProxyHostName { get; set; }          /* Address or hostname of the HTTP proxy server */
		private string? ProxyPort { get; set; }              /* Port Number of the HTTP proxy server. */
		private bool Proxy { get; set; }                 /* Proxy. */
		private string? Location { get; set; }               /* Location endpoint */
		private bool SessionMgnt { get; set; }           /* (optional) Enable Session Management in the reactor. */
		//END APIQA
		/// <summary>
		/// Number of items to request. See -itemCount
		/// </summary>
		public int ItemRequestCount { get; set; }
		/// <summary>
		/// Number of items common to all connections, if using multiple connections
		/// </summary>
		public int CommonItemCount { get; set; }
		/// <summary>
		/// Number of posts to send per second
		/// </summary>
		public int PostsPerSec { get; set; }
		/// <summary>
		/// Number of latency posts to send per second
		/// </summary>
		public int LatencyPostsPerSec { get; set; }
		/// <summary>
		/// Number of generic msgs to send per second
		/// </summary>
		public int GenMsgsPerSec { get; set; }
		/// <summary>
		/// Number of latency generic msgs to send per second
		/// </summary>
		public int LatencyGenMsgsPerSec { get; set; }

		/// <summary>
		/// Number of requests to send per tick
		/// </summary>
		public int RequestsPerTick { get; set; }
		/// <summary>
		/// The remainder of number of requests to send per tick
		/// </summary>
		public int RequestsPerTickRemainder { get; set; }

        /// <summary>
        /// Use the VA Reactor instead of the ETA Channel for sending and receiving.
        /// </summary>
        public bool UseReactor;

		/// <summary>
		/// If set, the application will continually read rather than using notification
		/// </summary>
		public bool BusyRead { get; set; }

		public void AddDefaultOptionValues()
        {
			CommandLine.ProgName("ConsPerf");
			CommandLine.AddOption("steadyStateTime", 300, "Time consumer will run the steady-state portion of the test. Also used as a timeout during the startup-state portion");
			CommandLine.AddOption("delaySteadyStateCalc", 30000, "Time consumer will wait before calculate the latency (milliseconds)");
			CommandLine.AddOption("tickRate", 1000, "Ticks per second");
			CommandLine.AddOption("threads", DEFAULT_THREAD_COUNT, "Number of threads that handle connections");
			CommandLine.AddOption("itemFile", "350k.xml", "Name of the file to get item names from");
			CommandLine.AddOption("msgFile", "MsgData.xml", "Name of the file that specifies the data content in messages");
			CommandLine.AddOption("latencyFile", "", "Base name of file for logging latency");
			CommandLine.AddOption("summaryFile", "ConsSummary.out", "Name of file for logging summary info");
			CommandLine.AddOption("statsFile", "ConsStats", "Base name of file for logging periodic statistics");
			CommandLine.AddOption("writeStatsInterval", 5, "Controls how often stats are written to the file");
			CommandLine.AddOption("noDisplayStats", false, "Stop printout of stats to screen");
			CommandLine.AddOption("requestRate", 500000, "Rate at which to request items");
			CommandLine.AddOption("connType", "socket", "Type of connection");
			//APIQA
			CommandLine.AddOption("h", DEFAULT_HOST, "Name of host to connect to");
			CommandLine.AddOption("p", DEFAULT_PORT, "Port number to connect to");
			//END APIQA
			CommandLine.AddOption("if", "", "Name of network interface to use");
			CommandLine.AddOption("outputBufs", 5000, "Number of output buffers(configures guaranteedOutputBuffers in ETA connection)");
			CommandLine.AddOption("inputBufs", 15, "Number of input buffers(configures numInputBufs in ETA connection)");
			CommandLine.AddOption("sendBufSize", 0, "System Send Buffer Size(configures sysSendBufSize in ConnectOptions)");
			CommandLine.AddOption("recvBufSize", 0, "System Receive Buffer Size(configures sysRecvBufSize in ConnectOptions)");
			CommandLine.AddOption("sendTimeout", 0, "System Send timeout (configures sendTimeout in ConnectOptions)");
			CommandLine.AddOption("recvTimeout", 0, "System Receive timeout (configures receiveTimeout in ConnectOptions)");
			CommandLine.AddOption("highWaterMark", 0, "Sets the point that causes ETA to automatically flush");
			CommandLine.AddOption("tcpDelay", false, "Turns off tcp_nodelay in BindOptions, enabling Nagle's");
			CommandLine.AddOption("snapshot", false, "Snapshot test, request all items as non-streaming");
			CommandLine.AddOption("uname", "", "Username to use in login request");
			CommandLine.AddOption("serviceName", "DIRECT_FEED", "Name of service to request items from");
			//APIQA
			CommandLine.AddOption("itemCount", 1, "Number of items to request");
			//END APIQA
			CommandLine.AddOption("commonItemCount", 0, "Number of items common to all consumers, if using multiple connections");
			CommandLine.AddOption("postingRate", 0, "Rate at which to send post messages");
			CommandLine.AddOption("postingLatencyRate", 0, "Rate at which to send latency post messages");
			CommandLine.AddOption("genericMsgRate", 0, "Rate at which to send generic messages");
			CommandLine.AddOption("genericMsgLatencyRate", 0, "Rate at which to send latency generic messages");
			CommandLine.AddOption("busyRead", false, "If set, the application will continually read rather than using notification.");
			//APIQA
			CommandLine.AddOption("reactor", false, "Use the VA Reactor instead of the ETA Channel for sending and receiving");
			CommandLine.AddOption("location", "", "Specify location endpoint");
			CommandLine.AddOption("password", "", "Password to use in login request");
			CommandLine.AddOption("clientId", "", "ClientId to use in login request");
			CommandLine.AddOption("clientSecret", "", "ClientId to use in login request");
			CommandLine.AddOption("proxy", "Specifies that the application will make a connection through a proxy server, default is false");
			CommandLine.AddOption("ph", "", "Proxy server host name");
			CommandLine.AddOption("pp", "", "Proxy port number");
			CommandLine.AddOption("sessionMgnt", "(optional) Enable Session Management in the reactor.");
			//ENDAPIQA

		}

		/// <summary>
		/// Parses command-line arguments to fill in the application's configuration structures.
		/// </summary>
		/// <param name="args">Command line arguments</param>
		/// <param name="maxThreads">The maximum number of threads</param>
		public void Init(string[] args, int maxThreads)
		{
			AddDefaultOptionValues();

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

			MaxThreads = maxThreads;
			MsgFilename = CommandLine.Value("msgFile");
			ItemFilename = CommandLine.Value("itemFile");
			LatencyLogFilename = CommandLine.Value("latencyFile");
			LogLatencyToFile = !string.IsNullOrEmpty(LatencyLogFilename);
			SummaryFilename = CommandLine.Value("summaryFile");
			StatsFilename = CommandLine.Value("statsFile");
			HostName = CommandLine.Value("h");
			PortNo = CommandLine.Value("p");
			InterfaceName = CommandLine.Value("if");
			Username = CommandLine.Value("uname");
			ServiceName = CommandLine.Value("serviceName");
			DisplayStats = !CommandLine.BoolValue("noDisplayStats");
			TcpNoDelay = !CommandLine.BoolValue("tcpDelay");
			RequestSnapshots = CommandLine.BoolValue("snapshot");
			BusyRead = CommandLine.BoolValue("busyRead");
			//APIQA
			UseReactor = CommandLine.BoolValue("reactor");
			ClientId = CommandLine.Value("clientId");
			Password = CommandLine.Value("password");
			ClientSecret = CommandLine.Value("clientSecret");
			ProxyHostName = CommandLine.Value("ph");
			ProxyPort = CommandLine.Value("pp");
			Proxy = CommandLine.BoolValue("proxy");
			SessionMgnt = CommandLine.BoolValue("sessionMgnt");
			Location = CommandLine.Value("location");
			//END APIQA
			try
			{
				SteadyStateTime = CommandLine.IntValue("steadyStateTime");
				DelaySteadyStateCalc = CommandLine.IntValue("delaySteadyStateCalc");
				TicksPerSec = CommandLine.IntValue("tickRate");
				ThreadCount = CommandLine.IntValue("threads");
				WriteStatsInterval = CommandLine.IntValue("writeStatsInterval");
				ItemRequestsPerSec = CommandLine.IntValue("requestRate");
				if ("socket".Equals(CommandLine.Value("connType")))
				{
					ConnectionType = ConnectionType.SOCKET;
				}
				else if ("encrypted".Equals(CommandLine.Value("connType")))
				{
					ConnectionType = ConnectionType.ENCRYPTED;
				}
				else
				{
					Console.Error.WriteLine("Config Error: Only socket, websocket or encrypted connection type is supported.\n");
					Console.Error.WriteLine(CommandLine.OptionHelpString());
					Environment.Exit((int)PerfToolsReturnCode.FAILURE);
				}
				GuaranteedOutputBuffers = CommandLine.IntValue("outputBufs");
				NumInputBuffers = CommandLine.IntValue("inputBufs");
				SendBufSize = CommandLine.IntValue("sendBufSize");
				RecvBufSize = CommandLine.IntValue("recvBufSize");
				SendTimeout = CommandLine.IntValue("sendTimeout");
				RecvTimeout = CommandLine.IntValue("recvTimeout");
				HighWaterMark = CommandLine.IntValue("highWaterMark");
				ItemRequestCount = CommandLine.IntValue("itemCount");
				CommonItemCount = CommandLine.IntValue("commonItemCount");
				PostsPerSec = CommandLine.IntValue("postingRate");
				LatencyPostsPerSec = CommandLine.IntValue("postingLatencyRate");
				GenMsgsPerSec = CommandLine.IntValue("genericMsgRate");
				LatencyGenMsgsPerSec = CommandLine.IntValue("genericMsgLatencyRate");
			}
			catch (FormatException ile)
			{
				Console.Error.WriteLine("Invalid argument, number expected.\t");
				Console.Error.WriteLine(ile.Message);
				Console.Error.WriteLine();
				Console.WriteLine(CommandLine.OptionHelpString());
				Environment.Exit((int)PerfToolsReturnCode.FAILURE);
			}
			//APIQA
			if (UseReactor && Username!.Length == 0)
			{
				Console.Error.WriteLine("Config Error: Username must be provided");
				Console.WriteLine(CommandLine.OptionHelpString());
				Environment.Exit((int)PerfToolsReturnCode.FAILURE);
			}

			if (UseReactor && ClientId!.Length == 0)
			{
				Console.Error.WriteLine("Config Error: clientId must be provided");
				Console.WriteLine(CommandLine.OptionHelpString());
				Environment.Exit((int)PerfToolsReturnCode.FAILURE);
			}

			if (UseReactor && Password!.Length == 0)
			{
				Console.Error.WriteLine("Config Error: Password must be provided");
				Console.WriteLine(CommandLine.OptionHelpString());
				Environment.Exit((int)PerfToolsReturnCode.FAILURE);
			}


			if (SessionMgnt && PortNo!.Equals(DEFAULT_PORT) && HostName!.Equals(DEFAULT_HOST))
			{
				PortNo = "";
				HostName = "";
			}

			if (Proxy)
			{
				if (ProxyHostName!.Length == 0)
				{
					Console.Error.WriteLine("Config Error: Proxy hostname not provided.\"");
					Console.WriteLine(CommandLine.OptionHelpString());
					Environment.Exit((int)PerfToolsReturnCode.FAILURE);
				}

				if (ProxyPort!.Length == 0)
				{
					Console.Error.WriteLine("Config Error: Proxy port not provided.\"");
					Console.WriteLine(CommandLine.OptionHelpString());
					Environment.Exit((int)PerfToolsReturnCode.FAILURE);
				}

			}

			//END APIQA


			if (ConnectionType != ConnectionType.SOCKET && ConnectionType != ConnectionType.ENCRYPTED)
			{
				Console.Error.WriteLine("Config Error: Application only supports SOCKET, ENCRYPTED connection types.\n");
				Console.WriteLine(CommandLine.OptionHelpString());
				Environment.Exit((int)PerfToolsReturnCode.FAILURE);
			}

			if (TicksPerSec < 1)
			{
				Console.Error.WriteLine("Config Error: Tick rate cannot be less than 1.");
				Console.WriteLine(CommandLine.OptionHelpString());
				Environment.Exit((int)PerfToolsReturnCode.FAILURE);
			}

			if (ItemRequestsPerSec < TicksPerSec)
			{
				Console.Error.WriteLine("Config Error: Item Request Rate cannot be less than tick rate.\n");
				Console.WriteLine(CommandLine.OptionHelpString());
				Environment.Exit((int)PerfToolsReturnCode.FAILURE);
			}

			if (PostsPerSec < TicksPerSec && PostsPerSec != 0)
			{
				Console.Error.WriteLine("Config Error: Post Rate cannot be less than tick rate(unless it is zero).\n");
				Console.WriteLine(CommandLine.OptionHelpString());
				Environment.Exit((int)PerfToolsReturnCode.FAILURE);
			}

			if (LatencyPostsPerSec > PostsPerSec)
			{
				Console.Error.WriteLine("Config Error: Latency Post Rate cannot be greater than total posting rate.\n");
				Console.WriteLine(CommandLine.OptionHelpString());
				Environment.Exit((int)PerfToolsReturnCode.FAILURE);
			}

			if (LatencyPostsPerSec > TicksPerSec)
			{
				Console.Error.WriteLine("Config Error: Latency Post Rate cannot be greater than tick rate.\n");
				Console.WriteLine(CommandLine.OptionHelpString());
				Environment.Exit((int)PerfToolsReturnCode.FAILURE);
			}

			if (PostsPerSec > 0 && RequestSnapshots)
			{
				Console.Error.WriteLine("Config Error: Configured to post while requesting snapshots.\n");
				Console.WriteLine(CommandLine.OptionHelpString());
				Environment.Exit((int)PerfToolsReturnCode.FAILURE);
			}

			if (GenMsgsPerSec < TicksPerSec && GenMsgsPerSec != 0)
			{
				Console.Error.WriteLine("Config Error: Generic Msg Rate cannot be less than tick rate(unless it is zero).\n");
				Console.WriteLine(CommandLine.OptionHelpString());
				Environment.Exit((int)PerfToolsReturnCode.FAILURE);
			}

			if (LatencyGenMsgsPerSec > GenMsgsPerSec)
			{
				Console.Error.WriteLine("Config Error: Latency Generic Msg Rate cannot be greater than total generic msg rate.\n");
				Console.WriteLine(CommandLine.OptionHelpString());
				Environment.Exit((int)PerfToolsReturnCode.FAILURE);
			}

			if (LatencyGenMsgsPerSec > TicksPerSec)
			{
				Console.Error.WriteLine("Config Error: Latency Generic Msg Rate cannot be greater than tick rate.\n");
				Console.WriteLine(CommandLine.OptionHelpString());
				Environment.Exit((int)PerfToolsReturnCode.FAILURE);
			}

			if (GenMsgsPerSec > 0 && RequestSnapshots)
			{
				Console.Error.WriteLine("Config Error: Configured to send generic msgs while requesting snapshots.\n");
				Console.WriteLine(CommandLine.OptionHelpString());
				Environment.Exit((int)PerfToolsReturnCode.FAILURE);
			}

			if (CommonItemCount > ItemRequestCount)
			{
				Console.Error.WriteLine("Config Error: Common item count is greater than total item count.\n");
				Console.WriteLine(CommandLine.OptionHelpString());
				Environment.Exit((int)PerfToolsReturnCode.FAILURE);
			}

			if (WriteStatsInterval < 1)
			{
				Console.Error.WriteLine("Config error: Write Stats Interval cannot be less than 1.\n");
				Console.WriteLine(CommandLine.OptionHelpString());
				Environment.Exit((int)PerfToolsReturnCode.FAILURE);
			}

			if (ThreadCount > MaxThreads)
			{
				Console.Error.WriteLine("Config error: Thread count cannot be greater than " + MaxThreads + ".\n");
				Console.WriteLine(CommandLine.OptionHelpString());
				Environment.Exit((int)PerfToolsReturnCode.FAILURE);
			}

			if (DelaySteadyStateCalc < 0 || DelaySteadyStateCalc > 30000)
			{
				Console.Error.WriteLine("Config error: Time before the latency is calculated should not be less than 0 or greater than 30000.\n");
				Console.WriteLine(CommandLine.OptionHelpString());
				Environment.Exit((int)PerfToolsReturnCode.FAILURE);
			}

			if ((DelaySteadyStateCalc / 1000) > SteadyStateTime)
			{
				Console.Error.WriteLine("Config Error: Time before the latency is calculated should be less than Steady State Time.\n");
				Console.WriteLine(CommandLine.OptionHelpString());
				Environment.Exit((int)PerfToolsReturnCode.FAILURE);
			}

			RequestsPerTick = ItemRequestsPerSec / TicksPerSec;

			RequestsPerTickRemainder = ItemRequestsPerSec % TicksPerSec;

			CreateConfigString();
		}

		private void CreateConfigString()
		{
            string reactorWatchlistUsageString;

            if (UseReactor)
            {
                reactorWatchlistUsageString = "Reactor";
            }
            else
            {
                reactorWatchlistUsageString = "None";
            }

			ConfigString = "--- TEST INPUTS ---\n\n" +
				"          Steady State Time: " + SteadyStateTime + " sec\n" +
				"    Delay Steady State Time: " + DelaySteadyStateCalc + " msec\n" +
				"            Connection Type: " + (ConnectionType == ConnectionType.SOCKET ? "SOCKET" : "ENCRYPTED") + "\n" +
				"                   Hostname: " + HostName + "\n" +
				"                       Port: " + PortNo + "\n" +
				//APIQA
				"                   ClientId: " + (ClientId!.Length > 0 ? "<removed from displaying>" : "null") + "\n" +
				"                   Password: " + (Password!.Length > 0 ? "<removed from displaying>" : "null") + "\n" +
				"   	        ClientSecret: " + (ClientSecret.Length > 0 ? "<removed from displaying>" : "unspecified") + "\n" +
				"            Proxy Host Name: " + ProxyHostName + "\n" +
				"                 Proxy Port: " + ProxyPort + "\n" +
				//END APIQA
				"                    Service: " + ServiceName + "\n" +
				"               Thread Count: " + ThreadCount + "\n" +
				"             Output Buffers: " + GuaranteedOutputBuffers + "\n" +
				"              Input Buffers: " + NumInputBuffers + "\n" +
				"           Send Buffer Size: " + SendBufSize + ((SendBufSize > 0) ? " bytes" : "(use default)") + "\n" +
				"           Recv Buffer Size: " + RecvBufSize + ((RecvBufSize > 0) ? " bytes" : "(use default)") + "\n" +
				"               Send Timeout: " + SendTimeout + ((SendTimeout > 0) ? " ms" : "(use default)") + "\n" +
				"            Receive Timeout: " + RecvTimeout + ((RecvTimeout > 0) ? " ms" : "(use default)") + "\n" +
				"            High Water Mark: " + HighWaterMark + ((HighWaterMark > 0) ? " bytes" : "(use default)") + "\n" +
				"             Interface Name: " + (InterfaceName != null && InterfaceName.Length > 0 ? InterfaceName : "(use default)") + "\n" +
				"                Tcp_NoDelay: " + (TcpNoDelay ? "Yes" : "No") + "\n" +
				"                   Username: " + (Username != null && Username.Length > 0 ? Username : "(use system login name)") + "\n" +
				"                 Item Count: " + ItemRequestCount + "\n" +
				"          Common Item Count: " + CommonItemCount + "\n" +
				"               Request Rate: " + ItemRequestsPerSec + "\n" +
				"          Request Snapshots: " + (RequestSnapshots ? "Yes" : "No") + "\n" +
				"               Posting Rate: " + PostsPerSec + "\n" +
				"       Latency Posting Rate: " + LatencyPostsPerSec + "\n" +
				"           Generic Msg Rate: " + GenMsgsPerSec + "\n" +
				"   Generic Msg Latency Rate: " + LatencyGenMsgsPerSec + "\n" +
				"                  Item File: " + ItemFilename + "\n" +
				"                  Data File: " + MsgFilename + "\n" +
				"               Summary File: " + SummaryFilename + "\n" +
				"                 Stats File: " + StatsFilename + "\n" +
				"           Latency Log File: " + (LatencyLogFilename != null && LatencyLogFilename.Length > 0 ? LatencyLogFilename : "(none)") + "\n" +
				"                  Tick Rate: " + TicksPerSec + "\n" +
				"                  Busy Read: " + (BusyRead ? "Yes" : "No") + "\n";
		}

		public override string? ToString()
        {
			return ConfigString;
        }
	}
}
