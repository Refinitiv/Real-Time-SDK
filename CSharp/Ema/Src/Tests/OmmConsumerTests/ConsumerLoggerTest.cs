/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2023-2025 LSEG. All rights reserved.     
 *|-----------------------------------------------------------------------------
 */

using System;
using System.IO;
using LSEG.Eta.ValueAdd.Reactor;
using Xunit.Abstractions;

namespace LSEG.Ema.Access.Tests.OmmConsumerTests
{
    public class ConsumerLoggerTest
    {
        private static readonly string EMA_FILE_PATH = "../../../OmmConsumerTests/EmaConfigLoggerTest.xml";

        private static readonly string VERBOSE_LOG = $"{NewLine}TRACE|: loggerMsg{NewLine}    ClientName: Consumer_1_Verbose{NewLine}    Severity: Trace    Text:    " +
            $"Print out active configuration detail.{NewLine}\tConfiguredName: Consumer_1_Verbose{NewLine}\tInstanceName: Consumer_1_Verbose{NewLine}\tItemCountHint: 100000{NewLine}\t" +
            $"ServiceCountHint: 513{NewLine}\tMaxDispatchCountApiThread: 100{NewLine}\tMaxDispatchCountUserThread: 100{NewLine}\tDispatchTimeoutApiThread: 0{NewLine}\tRequestTimeout: 15000{NewLine}\t" +
            $"XmlTraceToStdout: False{NewLine}\tXmlTraceToFile: False{NewLine}\tXmlTraceMaxFileSize: 100000000{NewLine}\tXmlTraceFileName: EmaTrace{NewLine}\tXmlTraceToMultipleFiles: False{NewLine}\t" +
            $"XmlTraceWrite: True{NewLine}\tXmlTraceRead: True{NewLine}\tXmlTracePing: False{NewLine}\tObeyOpenWindow: True{NewLine}\tPostAckTimeout: 15000{NewLine}\tMaxOutstandingPosts: 100000{NewLine}\t" +
            $"DispatchMode: 2{NewLine}\tDispatchTimeoutApiThread: 0{NewLine}\tReconnectAttemptLimit: -1{NewLine}\tReconnectMinDelay: 1000{NewLine}\tReconnectMaxDelay: 5000{NewLine}\tMsgKeyInUpdates: True{NewLine}\t" +
            $"DirectoryRequestTimeOut: 45000{NewLine}\tDictionaryRequestTimeOut: 45000{NewLine}\tRestRequestTimeOut: 15000{NewLine}\tLoginRequestTimeOut: 3000{NewLine}loggerMsgEnd{NewLine}";

        private static readonly string VERBOSE_LOG_1 = $"{NewLine}TRACE|: loggerMsg{NewLine}    ClientName: Consumer_1_Verbose{NewLine}    Severity: Trace    Text:" +
            $"    Successfully created Reactor.{NewLine}loggerMsgEnd{NewLine}{NewLine}TRACE|: loggerMsg{NewLine}    ClientName: LoginCallbackClient{NewLine}    Severity: Trace" +
            $"    Text:    RDMLogin request message was populated with this info: LoginRequest: {NewLine}\tstreamId: 1{NewLine}\tuserName: user{NewLine}\tstreaming:" +
            $" true{NewLine}\tnameType: NAME{NewLine}\tapplicationId: 256{NewLine}\tapplicationName: ema{NewLine}\tposition:";

        private static readonly string VERBOSE_LOG_2 = $"    ClientName: DirectoryCallbackClient{NewLine}    Severity: Trace    Text:    RDMDirectoryRequest" +
            $" message was populated with Filter(s){NewLine}\tRDM_DIRECTORY_SERVICE_INFO_FILTER{NewLine}\tRDM_DIRECTORY_SERVICE_STATE_FILTER{NewLine}\tRDM_DIRECTORY_SERVICE_GROUP_FILTER" +
            $"{NewLine}\tRDM_DIRECTORY_SERVICE_LOAD_FILTER{NewLine}\tRDM_DIRECTORY_SERVICE_DATA_FILTER{NewLine}\tRDM_DIRECTORY_SERVICE_LINK_FILTER{NewLine}\trequesting all services{NewLine}" +
            $"loggerMsgEnd{NewLine}{NewLine}TRACE|: loggerMsg{NewLine}    ClientName: DictionaryCallbackClient{NewLine}    Severity: Trace    Text:    Successfully loaded local dictionaries:" +
            $" {NewLine}\tRDMFieldDictionary file named ../../../../../../etc/RDMFieldDictionary{NewLine}\tEnumTypeDef file named ../../../../../../etc/enumtype.def{NewLine}loggerMsgEnd{NewLine}{NewLine}" +
            $"TRACE|: loggerMsg{NewLine}    ClientName: ChannelCallbackClient{NewLine}    Severity: Trace    Text:    Created ChannelCallbackClient{NewLine}loggerMsgEnd{NewLine}{NewLine}TRACE|: loggerMsg{NewLine}" +
            $"    ClientName: ChannelCallbackClient{NewLine}    Severity: Trace    Text:    Attempt to connect using{NewLine}\t1] SOCKET{NewLine}\tChannel name Channel_1{NewLine}\tInstance name" +
            $" Consumer_1_Verbose{NewLine}\tReactor";

        private static readonly string VERBOSE_LOG_3 = $"CompressionType None{NewLine}\tTcpNodelay True{NewLine}\tEnableSessionMgnt False{NewLine}\tLocation us-east-1{NewLine}\t" +
            $"ReconnectAttemptLimit -1{NewLine}\tReconnectMinDelay 1000 msec{NewLine}\tReconnectMaxDelay 5000 msec{NewLine}\tGuaranteedOutputBuffers 5000{NewLine}\tNumInputBuffers 100{NewLine}\tSysRecvBufSize" +
            $" 0{NewLine}\tSysSendBufSize 0{NewLine}\tConnectionPingTimeout 30 sec{NewLine}\tInitializationTimeout 60 sec{NewLine}\tDirectWrite False{NewLine}{NewLine}loggerMsgEnd{NewLine}{NewLine}TRACE|: loggerMsg{NewLine}    ClientName: ChannelCallbackClient{NewLine}    " +
            $"Severity: Trace    Text:    Received ChannelOpened event on channel Channel_1{NewLine}\tInstance Name Consumer_1_Verbose{NewLine}loggerMsgEnd{NewLine}{NewLine}TRACE|: loggerMsg{NewLine}    " +
            $"ClientName: ChannelCallbackClient{NewLine}    Severity: Trace    Text:    Successfully created a Reactor and Channel(s){NewLine}\tChannel name(s) Channel_1{NewLine}\tInstance name " +
            $"Consumer_1_Verbose{NewLine}{NewLine}loggerMsgEnd{NewLine}{NewLine}INFO|: loggerMsg{NewLine}    ClientName: ChannelCallbackClient{NewLine}    Severity: Info    Text:    Received ChannelUp event on " +
            $"channel Channel_1{NewLine}\tInstance Name Consumer_1_Verbose{NewLine}\tComponent Version etacsharp";

        private static readonly string VERBOSE_LOG_4 = $"{NewLine}loggerMsgEnd" +
            $"{NewLine}{NewLine}TRACE|: loggerMsg{NewLine}    ClientName: LoginCallbackClient{NewLine}    Severity: Trace    Text:    RDMLogin stream was open with refresh message{NewLine}" +
            $"LoginRefresh: {NewLine}\tstreamId: 1{NewLine}\tname: user{NewLine}\tnameType: NAME{NewLine}\tState: Open/Ok/None - text: \"Login accepted by host localhost\"{NewLine}\tisSolicited:" +
            $" True{NewLine}\tapplicationId: 256{NewLine}\tapplicationName: ETA Provider Test{NewLine}\tposition:";

        private static readonly string VERBOSE_LOG_5 = $"{NewLine}\tsingleOpen: 1{NewLine}\tallowSuspectData: 1{NewLine}\tSupportBatchRequests: 1{NewLine}\tSupportOMMPost: 1{NewLine}" +
            $"\tSupportEnhancedSymbolList: 1{NewLine}{NewLine}{NewLine}loggerMsgEnd{NewLine}{NewLine}TRACE|: loggerMsg{NewLine}    ClientName: DirectoryCallbackClient{NewLine}    Severity: Trace" +
            $"    Text:    RDMDirectory stream state was open with refresh message {NewLine}\tState: Open/Ok/None - text: \"\"{NewLine}loggerMsgEnd{NewLine}{NewLine}TRACE|: loggerMsg{NewLine}" +
            $"    ClientName: ChannelCallbackClient{NewLine}    Severity: Trace    Text:    Received ChannelReady event on channel Channel_1{NewLine}\tInstance Name" +
            $" Consumer_1_Verbose{NewLine}loggerMsgEnd{NewLine}";

        private static readonly string SUCCESS_LOG = $"{NewLine}INFO|: loggerMsg{NewLine}    ClientName: ChannelCallbackClient{NewLine}    Severity: Info    Text:    Received ChannelUp event on" +
            $" channel Channel_1{NewLine}\tInstance Name Consumer_1_Success{NewLine}\tComponent Version";

        private static readonly string ERROR_LOG = $"{NewLine}ERROR|: loggerMsg{NewLine}    ClientName: Consumer_1_Error{NewLine}    Severity: Error    Text:    login failed (timed out after" +
            $" waiting 3000 milliseconds) for localhost:5555){NewLine}loggerMsgEnd{NewLine}{NewLine}ERROR|: loggerMsg{NewLine}    ClientName: ChannelCallbackClient{NewLine}    Severity: Error    Text:    Received ChannelDown" +
            $" event on channel Channel_1{NewLine}\tInstance Name Consumer_1_Error{NewLine}\tReactor";

        private static readonly string WARNING_LOG = $"{NewLine}WARN|: loggerMsg{NewLine}    ClientName: ChannelCallbackClient{NewLine}    Severity: Warning    Text:" +
            $"    Received ChannelDownReconnecting event on channel Channel_1{NewLine}\tInstance Name Consumer_1_Warning{NewLine}\tChannel is null{NewLine}\tError Id SUCCESS{NewLine}" +
            $"\tInternal sysError 0{NewLine}\tError Location Reactor.Connect{NewLine}\tError text DNS resolution failure for address \"Invalidhost.abc\" with error text \"No such host is known.\".";

        ITestOutputHelper output;

        public ConsumerLoggerTest(ITestOutputHelper output)
        {
            this.output = output;

        }

        [Theory]
        [InlineData("Verbose")]
        [InlineData("Success")]
        [InlineData("Warning")]
        [InlineData("Error")]
        [InlineData("NoLogMsg")]
        public void ConsumerFileLoggerTest(string logLevel)
        {
            ProviderSessionOptions providerSessionOpts = new ();
            providerSessionOpts.SendLoginReject = false;
            ProviderTest providerTest = new (providerSessionOpts, output);
            ReactorOptions reactorOptions = new ();
            providerTest.Initialize(reactorOptions);

            int serverPort = logLevel.Equals("Error") ? 5555 : providerTest.ServerPort;

            string hostString = $"localhost:{serverPort}";

            output.WriteLine($"Connect with {hostString}");

            OmmConsumerConfig config = new (EMA_FILE_PATH);
            config.UserName("user");

            OmmException? exception = null;
            OmmConsumer? consumer = null;

            try
            {
                config.ConsumerName($"Consumer_2_{logLevel}");

                // Skip generating an instance ID
                OmmBaseImpl<IOmmConsumerClient>.GENERATE_INSTANCE_ID = false;

                if (logLevel.Equals("Warning"))
                    consumer = new OmmConsumer(config.Host("Invalidhost.abc:14002"));
                else
                    consumer = new OmmConsumer(config.Host(hostString));
            }
            catch (OmmException ommException)
            {
                exception = ommException;
            }

            string? logOutput = null;
            int processId = System.Environment.ProcessId;
            string filePath = $"emaLog_{logLevel}_{processId}.log";

            if (logLevel.Equals("Error") || logLevel.Equals("Warning"))
            {
                Assert.NotNull(exception);

                logOutput = File.ReadAllText(filePath);
            }
            else if (logLevel.Equals("NoLogMsg"))
            {
                Assert.Null(exception);
                Assert.False(File.Exists(filePath));
                logOutput = string.Empty;
            }
            else
            {
                Assert.Null(exception);

                logOutput = File.ReadAllText(filePath);
            }

            Assert.NotNull(logOutput);

            if (logLevel.Equals("Success"))
            {
                Assert.Contains(SUCCESS_LOG.Replace("Consumer_1_Success", "Consumer_2_Success"), logOutput);
            }
            else if (logLevel.Equals("NoLogMsg"))
            {
                Assert.Equal(string.Empty, logOutput);
            }
            else if (logLevel.Equals("Error"))
            {
                Assert.StartsWith(ERROR_LOG.Replace("Consumer_1", "Consumer_2"), logOutput);
            }
            else if (logLevel.Equals("Verbose"))
            {
                Assert.Contains(VERBOSE_LOG.Replace("Consumer_1", "Consumer_2"), logOutput);
                Assert.Contains(VERBOSE_LOG_1.Replace("Consumer_1", "Consumer_2"), logOutput);
                Assert.Contains(VERBOSE_LOG_2.Replace("Consumer_1", "Consumer_2"), logOutput);
                Assert.Contains(VERBOSE_LOG_3.Replace("Consumer_1", "Consumer_2"), logOutput);
                Assert.Contains(VERBOSE_LOG_4.Replace("Consumer_1", "Consumer_2"), logOutput);
                Assert.Contains(VERBOSE_LOG_5.Replace("Consumer_1", "Consumer_2"), logOutput);
            }
            else if (logLevel.Equals("Warning"))
            {
                Assert.Contains(WARNING_LOG.Replace("Consumer_1", "Consumer_2"), logOutput);
            }

            consumer?.Uninitialize();
            providerTest.UnInitialize();
        }

        [Theory]
        [InlineData("Verbose")]
        [InlineData("Success")]
        [InlineData("Warning")]
        [InlineData("Error")]
        [InlineData("NoLogMsg")]
        public void ConsumerConsoleLoggerTest(string logLevel)
        {
            ProviderSessionOptions providerSessionOpts = new();
            providerSessionOpts.SendLoginReject = false;
            ProviderTest providerTest = new(providerSessionOpts, output);
            ReactorOptions reactorOptions = new();
            providerTest.Initialize(reactorOptions);

            int serverPort = logLevel.Equals("Error") ? 5555 : providerTest.ServerPort;

            string hostString = $"localhost:{serverPort}";

            output.WriteLine($"Connect with {hostString}");

            OmmConsumerConfig config = new(EMA_FILE_PATH);
            config.UserName("user");

            OmmException? exception = null;
            OmmConsumer? consumer = null;

            MemoryStream memoryStream = new (8192);
            StreamWriter streamWriter = new (memoryStream);
            System.Console.SetOut(streamWriter);

            try
            {
                config.ConsumerName($"Consumer_1_{logLevel}");
                
                // Skip generating an instance ID
                OmmBaseImpl<IOmmConsumerClient>.GENERATE_INSTANCE_ID = false;

                if (logLevel.Equals("Warning"))
                    consumer = new OmmConsumer(config.Host("Invalidhost.abc:14002"));
                else
                    consumer = new OmmConsumer(config.Host(hostString));
            }
            catch (OmmException ommException)
            {
                exception = ommException;
            }

            if (logLevel.Equals("Error") || logLevel.Equals("Warning"))
            {
                Assert.NotNull(exception);
            }
            else
            {
                Assert.Null(exception);
            }

            string logOutput = System.Text.Encoding.ASCII.GetString(memoryStream.GetBuffer(), 0, 
                (int)memoryStream.Length);

            Assert.NotNull(logOutput);

            if (logLevel.Equals("Success"))
            {
                Assert.Contains(SUCCESS_LOG, logOutput);
            }
            else if(logLevel.Equals("NoLogMsg"))
            {
                Assert.Equal(string.Empty, logOutput);
            }
            else if (logLevel.Equals("Error"))
            {
                Assert.StartsWith(ERROR_LOG, logOutput);
            }
            else if (logLevel.Equals("Verbose"))
            {
                Assert.Contains(VERBOSE_LOG, logOutput);
                Assert.Contains(VERBOSE_LOG_1, logOutput);
                Assert.Contains(VERBOSE_LOG_2, logOutput);
                Assert.Contains(VERBOSE_LOG_3, logOutput);
                Assert.Contains(VERBOSE_LOG_4, logOutput);
                Assert.Contains(VERBOSE_LOG_5, logOutput);
            }
            else if (logLevel.Equals("Warning"))
            {
                Assert.Contains(WARNING_LOG, logOutput);
            }

            consumer?.Uninitialize();
            providerTest.UnInitialize();
        }

        [Fact]
        public void ConsumerMultipleIncludedDateFilesLoggerTest()
        {
            ProviderSessionOptions providerSessionOpts = new();
            providerSessionOpts.SendLoginReject = false;
            ProviderTest providerTest = new(providerSessionOpts, output);
            ReactorOptions reactorOptions = new();
            providerTest.Initialize(reactorOptions);

            int serverPort = providerTest.ServerPort;

            string hostString = $"localhost:{serverPort}";

            output.WriteLine($"Connect with {hostString}");

            OmmConsumerConfig config = new(EMA_FILE_PATH);
            OmmException? exception = null;
            OmmConsumer? consumer = null;

            try
            {
                config.ConsumerName("Consumer_2_MultipleFiles");

                // Skip generating an instance ID
                OmmBaseImpl<IOmmConsumerClient>.GENERATE_INSTANCE_ID = false;

                consumer = new OmmConsumer(config.Host(hostString));
            }
            catch (OmmException ommException)
            {
                exception = ommException;
            }

            Assert.Null(exception);

            int processId = System.Environment.ProcessId;
            string filePath = $"emaLog_MultipleFiles_{processId}.log";
            string filePath0 = $"emaLog_MultipleFiles_{processId}.0.log";
            string filePath1 = $"emaLog_MultipleFiles_{processId}.1.log";

            consumer?.Uninitialize();
            providerTest.UnInitialize();

            Assert.True(File.Exists(filePath));
            Assert.True(File.Exists(filePath0));
            Assert.True(File.Exists(filePath1));

            string logOutput = File.ReadAllText(filePath);
            string logOutput0 = File.ReadAllText(filePath0);
            string logOutput1 = File.ReadAllText(filePath1);

            DateTime dateTime = DateTime.Now;

            string expectedDate = $"{dateTime.Year}-";
            expectedDate += (dateTime.Month < 10) ? $"0{dateTime.Month}-" : $"{dateTime.Month}-";
            expectedDate += (dateTime.Day < 10) ? $"0{dateTime.Day}" : $"{dateTime.Day}";

            Assert.Contains(expectedDate, logOutput1);
            Assert.Contains("RDMLogin stream was open with refresh message", logOutput1);
            Assert.Contains("Login accepted by host localhost", logOutput1);
            Assert.Contains(expectedDate, logOutput0);
            Assert.Contains("RDMDirectory stream state was open with refresh message", logOutput0);
            Assert.Contains("Received ChannelReady event on channel Channel_1", logOutput0);
            Assert.Contains(expectedDate, logOutput);
            Assert.Contains("Received Delete action for RDMService", logOutput);

            Assert.True(logOutput.Length > 0);
        }


        /// <summary>
        /// Make sure that the EMA Logger does not override NLog logger configuration for the
        /// application.
        /// </summary>
        /// <remarks>
        /// See GitHub issue #257.
        /// </remarks>
        [Fact]
        public void OverrideConfig_Test()
        {
            // Intercept standard output (Console) to the memory buffer to examine Logger
            // messages for expected log messages
            MemoryStream memoryStream = new(12 * 1024);
            StreamWriter streamWriter = new(memoryStream);
            Console.SetOut(streamWriter);

            // Application configures NLog
            NLog.Targets.DebugTarget target = new NLog.Targets.DebugTarget() { Name = "Debug" };
            target.Layout = "${message}";

            NLog.Config.LoggingConfiguration config = new NLog.Config.LoggingConfiguration();
            config.AddRuleForAllLevels(target);

            NLog.LogManager.Configuration = config;

            NLog.Logger logger = NLog.LogManager.GetLogger("Example");

            // Initialise mock provider
            ProviderSessionOptions providerSessionOpts = new();
            providerSessionOpts.SendLoginReject = false;
            ProviderTest providerTest = new(providerSessionOpts, output);
            ReactorOptions reactorOptions = new();
            providerTest.Initialize(reactorOptions);

            int serverPort = providerTest.ServerPort;

            string hostString = $"localhost:{serverPort}";

            // Initialize EMA Consumer
            OmmConsumerConfig consumerConfig = new OmmConsumerConfig(EMA_FILE_PATH);

            consumerConfig
                .UserName("user")
                .Host(hostString)
                .ConsumerName($"Consumer_1_Verbose");

            // run EMA Consumer
            {
                OmmConsumer consumer = new OmmConsumer(consumerConfig);

                // verify that the application can still log its messages to the configured output
                // and that it did not get overridden by the EMA LoggerClient
                logger.Debug("log message");
                logger.Debug("another log message");

                providerTest.UnInitialize();
                consumer.Uninitialize();
            }

            // both the consumer and provider are stopped, now examine the log output to
            // the Console and to the application-defined logger

            Assert.Equal(2, target.Counter);
            Assert.Equal("another log message", target.LastMessage);

            // verify that the EMA IProvider was still able to output log messages as
            // configured - to Console
            string logOutput = System.Text.Encoding.ASCII.GetString(memoryStream.GetBuffer(), 0,
                (int)memoryStream.Length);

            Assert.NotNull(logOutput);
            Assert.NotEmpty(logOutput);

            Assert.StartsWith($"{NewLine}TRACE", logOutput);
            Assert.Contains("Successfully created Reactor", logOutput);
            Assert.Contains("Print out active configuration detail.", logOutput);
        }
    }
}
