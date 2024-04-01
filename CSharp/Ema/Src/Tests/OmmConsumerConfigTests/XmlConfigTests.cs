using LSEG.Eta.Transports;
using static LSEG.Ema.Access.Tests.OmmConsumerConfigTests.ConfigTestsUtils;

namespace LSEG.Ema.Access.Tests.OmmConsumerConfigTests
{
    public class XmlConfigTests
    {
        private OmmConsumerConfig consumerConfig;
        private OmmConsumerConfigImpl consConfigImpl;

        public XmlConfigTests()
        {
            consumerConfig = LoadEmaTestConfig();
            consConfigImpl = consumerConfig.OmmConsConfigImpl;
        }

        [Fact]
        public void LoadConfigFromFileTest()
        {
            Assert.Equal(0, consConfigImpl.ConfigErrorLog!.Count());

            Assert.Equal(2, consConfigImpl.ConsumerConfigMap.Count);
            Assert.Equal(2, consConfigImpl.ClientChannelConfigMap.Count);
            Assert.Equal(2, consConfigImpl.LoggerConfigMap.Count);
            Assert.Equal(2, consConfigImpl.DictionaryConfigMap.Count);
        }

        [Fact]
        public void ValidateConfigFromFileTest()
        {
            consConfigImpl.VerifyConfiguration();
        }

        [Fact]
        public void ConsumerWithNoDefaultsTest()
        {
            var testConsConfig = consConfigImpl.ConsumerConfigMap["TestConsumer"];

            Assert.Equal("TestConsumer", testConsConfig.Name);
            Assert.Single(testConsConfig.ChannelSet);
            Assert.Equal("TestChannel_1", testConsConfig.ChannelSet[0]);
            Assert.Equal("TestLogger_1", testConsConfig.Logger);
            Assert.Equal("TestDictionary_1", testConsConfig.Dictionary);
            Assert.Equal((long)10, testConsConfig.DictionaryRequestTimeOut);
            Assert.Equal((long)20, testConsConfig.DirectoryRequestTimeOut);
            Assert.Equal((long)30, testConsConfig.LoginRequestTimeOut);
            Assert.Equal(40, testConsConfig.DispatchTimeoutApiThread);
            Assert.True(testConsConfig.EnableRtt);
            Assert.Equal((ulong)50, testConsConfig.ItemCountHint);
            Assert.Equal((int)60, testConsConfig.MaxDispatchCountApiThread);
            Assert.Equal((int)70, testConsConfig.MaxDispatchCountUserThread);
            Assert.Equal((ulong)80, testConsConfig.MaxOutstandingPosts);
            Assert.False(testConsConfig.MsgKeyInUpdates);
            Assert.False(testConsConfig.ObeyOpenWindow);
            Assert.Equal((ulong)100, testConsConfig.PostAckTimeout);
            Assert.Equal(110, testConsConfig.ReconnectAttemptLimit);
            Assert.Equal(1400, testConsConfig.ReconnectMaxDelay);
            Assert.Equal(1300, testConsConfig.ReconnectMinDelay);
            Assert.Equal((ulong)140, testConsConfig.RequestTimeout);
            Assert.True(testConsConfig.RestEnableLog);
            Assert.True(testConsConfig.RestEnableLogViaCallback);
            Assert.Equal("testRestLog", testConsConfig.RestLogFileName);
            Assert.Equal((ulong)150, testConsConfig.RestRequestTimeOut);
            Assert.Equal((int)160, testConsConfig.ServiceCountHint);
            //Assert.Equal((ulong)1, testConsConfig.XmlTraceDump);
            //Assert.Equal("testXmlTrace", testConsConfig.XmlTraceFileName);
            Assert.True(testConsConfig.XmlTraceToStdout);
            Assert.Equal("proxy.local", testConsConfig.RestProxyHostName);
            Assert.Equal("3128", testConsConfig.RestProxyPort);
        }

        [Fact]
        public void ConsumerWithDefaultsExceptChannelSetTest()
        {
            var testConsConfig = consConfigImpl.ConsumerConfigMap["TestConsumer_2"];
            var defaultConsConfig = new ConsumerConfig();

            Assert.Equal("TestConsumer_2", testConsConfig.Name);
            Assert.Equal(2, testConsConfig.ChannelSet.Count);
            Assert.Equal("TestChannel_1", testConsConfig.ChannelSet[0]);
            Assert.Equal("TestChannel_2", testConsConfig.ChannelSet[1]);
            Assert.Equal(defaultConsConfig.Logger, testConsConfig.Logger);
            Assert.Equal(defaultConsConfig.Dictionary, testConsConfig.Dictionary);
            Assert.Equal(defaultConsConfig.DictionaryRequestTimeOut, testConsConfig.DictionaryRequestTimeOut);
            Assert.Equal(defaultConsConfig.DictionaryRequestTimeOut, testConsConfig.DirectoryRequestTimeOut);
            Assert.Equal(defaultConsConfig.LoginRequestTimeOut, testConsConfig.LoginRequestTimeOut);
            Assert.Equal(defaultConsConfig.DispatchTimeoutApiThread, testConsConfig.DispatchTimeoutApiThread);
            Assert.Equal(defaultConsConfig.EnableRtt, testConsConfig.EnableRtt);
            Assert.Equal(defaultConsConfig.ItemCountHint, testConsConfig.ItemCountHint);
            Assert.Equal(defaultConsConfig.MaxDispatchCountApiThread, testConsConfig.MaxDispatchCountApiThread);
            Assert.Equal(defaultConsConfig.MaxDispatchCountUserThread, testConsConfig.MaxDispatchCountUserThread);
            Assert.Equal(defaultConsConfig.MaxOutstandingPosts, testConsConfig.MaxOutstandingPosts);
            Assert.Equal(defaultConsConfig.MsgKeyInUpdates, testConsConfig.MsgKeyInUpdates);
            Assert.Equal(defaultConsConfig.ObeyOpenWindow, testConsConfig.ObeyOpenWindow);
            Assert.Equal(defaultConsConfig.PostAckTimeout, testConsConfig.PostAckTimeout);
            Assert.Equal(defaultConsConfig.ReconnectAttemptLimit, testConsConfig.ReconnectAttemptLimit);
            Assert.Equal(defaultConsConfig.ReconnectMaxDelay, testConsConfig.ReconnectMaxDelay);
            Assert.Equal(defaultConsConfig.ReconnectMinDelay, testConsConfig.ReconnectMinDelay);
            Assert.Equal(defaultConsConfig.RequestTimeout, testConsConfig.RequestTimeout);
            Assert.Equal(defaultConsConfig.RestEnableLog, testConsConfig.RestEnableLog);
            Assert.Equal(defaultConsConfig.RestEnableLogViaCallback, testConsConfig.RestEnableLogViaCallback);
            Assert.Equal(defaultConsConfig.RestLogFileName, testConsConfig.RestLogFileName);
            Assert.Equal(defaultConsConfig.RestRequestTimeOut, testConsConfig.RestRequestTimeOut);
            Assert.Equal(defaultConsConfig.ServiceCountHint, testConsConfig.ServiceCountHint);
            Assert.Equal(defaultConsConfig.XmlTraceDump, testConsConfig.XmlTraceDump);
            Assert.Equal(defaultConsConfig.XmlTraceFileName, testConsConfig.XmlTraceFileName);
            Assert.Equal(defaultConsConfig.XmlTraceToStdout, testConsConfig.XmlTraceToStdout);
        }

        [Fact]
        public void ChannelWithNondefaultsTest()
        {
            var testChannelConfig = consConfigImpl.ClientChannelConfigMap["TestChannel_1"];

            Assert.Equal("TestChannel_1", testChannelConfig.Name);
            Assert.Equal(Eta.Transports.ConnectionType.ENCRYPTED, testChannelConfig.ConnectInfo.ConnectOptions.ConnectionType);
            Assert.Equal(Eta.Transports.ConnectionType.ENCRYPTED, testChannelConfig.ConnectInfo.ConnectOptions.EncryptionOpts.EncryptedProtocol);
            Assert.Equal(60, testChannelConfig.ConnectInfo.ConnectOptions.PingTimeout);
            Assert.True(testChannelConfig.ConnectInfo.EnableSessionManagement);
            Assert.Equal(20, testChannelConfig.ConnectInfo.ConnectOptions.GuaranteedOutputBuffers);
            Assert.Equal(30, testChannelConfig.HighWaterMark);
            Assert.Equal(40, testChannelConfig.ConnectInfo.GetInitTimeout());
            Assert.Equal("testInterface", testChannelConfig.ConnectInfo.ConnectOptions.UnifiedNetworkInfo.InterfaceName);
            Assert.Equal("testLocation", testChannelConfig.ConnectInfo.Location);
            Assert.Equal(50, testChannelConfig.ConnectInfo.ConnectOptions.NumInputBuffers);
            Assert.Equal(60, testChannelConfig.ConnectInfo.ServiceDiscoveryRetryCount);
            Assert.Equal(70, testChannelConfig.ConnectInfo.ConnectOptions.SysRecvBufSize);
            Assert.Equal(80, testChannelConfig.ConnectInfo.ConnectOptions.SysSendBufSize);
            Assert.Equal(Eta.Transports.CompressionType.ZLIB, testChannelConfig.ConnectInfo.ConnectOptions.CompressionType);
            Assert.True(testChannelConfig.CompressionThresholdSet);
            Assert.Equal(555, testChannelConfig.CompressionThreshold);
            Assert.True(testChannelConfig.DirectWrite);
            Assert.Equal("testChannel1", testChannelConfig.ConnectInfo.ConnectOptions.UnifiedNetworkInfo.Address);
            Assert.Equal("testPort1", testChannelConfig.ConnectInfo.ConnectOptions.UnifiedNetworkInfo.ServiceName);
            Assert.Equal("proxyHost1", testChannelConfig.ConnectInfo.ConnectOptions.ProxyOptions.ProxyHostName);
            Assert.Equal("proxyPort1", testChannelConfig.ConnectInfo.ConnectOptions.ProxyOptions.ProxyPort);
            Assert.True(testChannelConfig.ConnectInfo.ConnectOptions.TcpOpts.TcpNoDelay);
            Assert.Equal(EncryptionProtocolFlags.ENC_TLSV1_2, testChannelConfig.ConnectInfo.ConnectOptions.EncryptionOpts.EncryptionProtocolFlags);
            Assert.Equal(25000, testChannelConfig.ConnectInfo.ConnectOptions.EncryptionOpts.AuthenticationTimeout);
        }

        [Fact]
        public void ChannelWithDefaultsTest()
        {
            var testChannelConfig = consConfigImpl.ClientChannelConfigMap["TestChannel_2"];
            var defaultChannelConfig = new ClientChannelConfig();

            Assert.Equal("TestChannel_2", testChannelConfig.Name);
            Assert.Equal(defaultChannelConfig.ConnectInfo.ConnectOptions.ConnectionType, testChannelConfig.ConnectInfo.ConnectOptions.ConnectionType);
            Assert.Equal(defaultChannelConfig.ConnectInfo.ConnectOptions.EncryptionOpts.EncryptedProtocol, testChannelConfig.ConnectInfo.ConnectOptions.EncryptionOpts.EncryptedProtocol);
            Assert.Equal(defaultChannelConfig.ConnectInfo.ConnectOptions.PingTimeout, testChannelConfig.ConnectInfo.ConnectOptions.PingTimeout);
            Assert.Equal(defaultChannelConfig.ConnectInfo.EnableSessionManagement, testChannelConfig.ConnectInfo.EnableSessionManagement);
            Assert.Equal(defaultChannelConfig.ConnectInfo.ConnectOptions.GuaranteedOutputBuffers, testChannelConfig.ConnectInfo.ConnectOptions.GuaranteedOutputBuffers);
            Assert.Equal(defaultChannelConfig.HighWaterMark, testChannelConfig.HighWaterMark);
            Assert.Equal(defaultChannelConfig.ConnectInfo.GetInitTimeout(), testChannelConfig.ConnectInfo.GetInitTimeout());
            Assert.Equal(defaultChannelConfig.ConnectInfo.ConnectOptions.UnifiedNetworkInfo.InterfaceName, testChannelConfig.ConnectInfo.ConnectOptions.UnifiedNetworkInfo.InterfaceName);
            Assert.Equal(defaultChannelConfig.ConnectInfo.Location, testChannelConfig.ConnectInfo.Location);
            Assert.Equal(defaultChannelConfig.ConnectInfo.ConnectOptions.NumInputBuffers, testChannelConfig.ConnectInfo.ConnectOptions.NumInputBuffers);
            Assert.Equal(defaultChannelConfig.ConnectInfo.ServiceDiscoveryRetryCount, testChannelConfig.ConnectInfo.ServiceDiscoveryRetryCount);
            Assert.Equal(defaultChannelConfig.ConnectInfo.ConnectOptions.SysRecvBufSize, testChannelConfig.ConnectInfo.ConnectOptions.SysRecvBufSize);
            Assert.Equal(defaultChannelConfig.ConnectInfo.ConnectOptions.SysSendBufSize, testChannelConfig.ConnectInfo.ConnectOptions.SysSendBufSize);
            Assert.Equal(defaultChannelConfig.ConnectInfo.ConnectOptions.CompressionType, testChannelConfig.ConnectInfo.ConnectOptions.CompressionType);
            Assert.Equal(defaultChannelConfig.DirectWrite, testChannelConfig.DirectWrite);
            Assert.Equal(defaultChannelConfig.ConnectInfo.ConnectOptions.UnifiedNetworkInfo.Address, testChannelConfig.ConnectInfo.ConnectOptions.UnifiedNetworkInfo.Address);
            Assert.Equal(defaultChannelConfig.ConnectInfo.ConnectOptions.UnifiedNetworkInfo.ServiceName, testChannelConfig.ConnectInfo.ConnectOptions.UnifiedNetworkInfo.ServiceName);
            Assert.Equal(defaultChannelConfig.ConnectInfo.ConnectOptions.GuaranteedOutputBuffers, testChannelConfig.ConnectInfo.ConnectOptions.GuaranteedOutputBuffers);
            Assert.Equal(defaultChannelConfig.ConnectInfo.ConnectOptions.ProxyOptions.ProxyHostName, testChannelConfig.ConnectInfo.ConnectOptions.ProxyOptions.ProxyHostName);
            Assert.Equal(defaultChannelConfig.ConnectInfo.ConnectOptions.ProxyOptions.ProxyPort, testChannelConfig.ConnectInfo.ConnectOptions.ProxyOptions.ProxyPort);
            Assert.Equal(defaultChannelConfig.ConnectInfo.ConnectOptions.TcpOpts.TcpNoDelay, testChannelConfig.ConnectInfo.ConnectOptions.TcpOpts.TcpNoDelay);
            Assert.Equal(defaultChannelConfig.ConnectInfo.ConnectOptions.EncryptionOpts.EncryptionProtocolFlags, testChannelConfig.ConnectInfo.ConnectOptions.EncryptionOpts.EncryptionProtocolFlags);
            Assert.False(defaultChannelConfig.CompressionThresholdSet);
        }

        [Fact]
        public void LoggerWithNonDefaultsTest()
        {
            var testLoggerConfig = consConfigImpl.LoggerConfigMap["TestLogger_1"];

            Assert.Equal("TestLogger_1", testLoggerConfig.Name);
            Assert.Equal(LoggerType.STDOUT, testLoggerConfig.LoggerType);
            Assert.Equal(LoggerLevel.INFO, testLoggerConfig.LoggerSeverity);
            Assert.Equal("testLogFile1", testLoggerConfig.FileName);
            Assert.Equal((ulong)1, testLoggerConfig.IncludeDateInLoggerOutput);
            Assert.Equal((ulong)10, testLoggerConfig.NumberOfLogFiles);
            Assert.Equal((ulong)20, testLoggerConfig.MaxLogFileSize);
        }

        [Fact]
        public void LoggerWithDefaultsTest()
        {
            var testLoggerConfig = consConfigImpl.LoggerConfigMap["TestLogger_2"];
            var defaultLoggerConfig = new LoggerConfig();

            Assert.Equal("TestLogger_2", testLoggerConfig.Name);
            Assert.Equal(defaultLoggerConfig.LoggerType, testLoggerConfig.LoggerType);
            Assert.Equal(defaultLoggerConfig.LoggerSeverity, testLoggerConfig.LoggerSeverity);
            Assert.Equal(defaultLoggerConfig.FileName, testLoggerConfig.FileName);
            Assert.Equal(defaultLoggerConfig.IncludeDateInLoggerOutput, testLoggerConfig.IncludeDateInLoggerOutput);
            Assert.Equal(defaultLoggerConfig.NumberOfLogFiles, testLoggerConfig.NumberOfLogFiles);
            Assert.Equal(defaultLoggerConfig.MaxLogFileSize, testLoggerConfig.MaxLogFileSize);
        }

        [Fact]
        public void DictionaryWithNonDefaultsTest()
        {
            var testDictConfig = consConfigImpl.DictionaryConfigMap["TestDictionary_1"];

            Assert.Equal("TestDictionary_1", testDictConfig.Name);
            Assert.Equal(EmaConfig.DictionaryTypeEnum.FILE, testDictConfig.DictionaryType);
            Assert.Equal("testEnumFile1", testDictConfig.EnumTypeDefFileName);
            Assert.Equal("testEnumItem1", testDictConfig.EnumTypeDefItemName);
            Assert.Equal("testRdmFile1", testDictConfig.RdmFieldDictionaryFileName);
            Assert.Equal("testRdmItem1", testDictConfig.RdmFieldDictionaryItemName);
        }

        [Fact]
        public void DictionaryWithDefaultsTest()
        {
            var testDictConfig = consConfigImpl.DictionaryConfigMap["TestDictionary_2"];
            var defaultDictConfig = new DictionaryConfig();

            Assert.Equal("TestDictionary_2", testDictConfig.Name);
            Assert.Equal(defaultDictConfig.DictionaryType, testDictConfig.DictionaryType);
            Assert.Equal(defaultDictConfig.EnumTypeDefFileName, testDictConfig.EnumTypeDefFileName);
            Assert.Equal(defaultDictConfig.EnumTypeDefItemName, testDictConfig.EnumTypeDefItemName);
            Assert.Equal(defaultDictConfig.RdmFieldDictionaryFileName, testDictConfig.RdmFieldDictionaryFileName);
            Assert.Equal(defaultDictConfig.RdmFieldDictionaryItemName, testDictConfig.RdmFieldDictionaryItemName);
        }
    }
}
