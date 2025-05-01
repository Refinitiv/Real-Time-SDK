/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2024-2025 LSEG. All rights reserved.              --
 *|-----------------------------------------------------------------------------
 */


using LSEG.Eta.Transports;
using LSEG.Eta.ValueAdd.Reactor;
using System.Collections.Generic;
using static LSEG.Ema.Access.Tests.OmmConfigTests.ConfigTestsUtils;

namespace LSEG.Ema.Access.Tests.OmmConfigTests
{
    public class ConsumerXmlConfigTests
    {
        private static readonly ClientChannelConfig defaultChannelConfig = new();
        private static readonly LoggerConfig defaultLoggerConfig = new();
        private static readonly DictionaryConfig defaultDictConfig = new();
        private static readonly ConsumerConfig defaultConsConfig = new();

        public static IEnumerable<object[]> GetTestObjects()
        {
            OmmConsumerConfig consumerConfig;
            OmmConsumerConfigImpl consConfigImplOriginal;
            OmmConsumerConfigImpl consConfigImplClone;
            consumerConfig = LoadEmaTestConfig();
            consConfigImplOriginal = consumerConfig.OmmConsConfigImpl;
            consConfigImplOriginal.VerifyConfiguration();
            consConfigImplClone = new OmmConsumerConfigImpl(consConfigImplOriginal);
            yield return new object[] { consConfigImplOriginal };
            yield return new object[] { consConfigImplClone };
        }

        [Fact]
        public void LoadConfigFromFileTest()
        {
            OmmConsumerConfigImpl consConfigImpl = LoadEmaTestConfig().OmmConsConfigImpl;
            Assert.Equal(0, consConfigImpl.ConfigErrorLog!.Count());
            Assert.Equal(2, consConfigImpl.ConsumerConfigMap.Count);
            Assert.Equal(2, consConfigImpl.ClientChannelConfigMap.Count);
            Assert.Equal(2, consConfigImpl.LoggerConfigMap.Count);
            Assert.Equal(2, consConfigImpl.DictionaryConfigMap.Count);
        }

        [Theory]
        [MemberData(nameof(GetTestObjects))]
        internal void ValidateConfigFromFileTest(OmmConsumerConfigImpl consConfigImpl)
        {
            consConfigImpl.VerifyConfiguration();
        }

        [Theory]
        [MemberData(nameof(GetTestObjects))]
        internal void ConsumerWithNoDefaultsTest(OmmConsumerConfigImpl consConfigImpl)
        {
            Assert.True(consConfigImpl.ConsumerConfigMap.ContainsKey("TestConsumer"));
            var testConsConfig = consConfigImpl.ConsumerConfigMap["TestConsumer"];

            Assert.Equal("TestConsumer", testConsConfig.Name);
            Assert.Single(testConsConfig.ChannelSet);
            Assert.Equal("TestChannel_1", testConsConfig.ChannelSet[0]);
            Assert.Equal("TestLogger_1", testConsConfig.Logger);
            Assert.Equal("TestDictionary_1", testConsConfig.Dictionary);
            Assert.Equal(10, testConsConfig.DictionaryRequestTimeOut);
            Assert.Equal(20, testConsConfig.DirectoryRequestTimeOut);
            Assert.Equal(30, testConsConfig.LoginRequestTimeOut);
            Assert.Equal(-1, testConsConfig.DispatchTimeoutApiThread);
            Assert.True(testConsConfig.EnableRtt);
            Assert.Equal((ulong)50, testConsConfig.ItemCountHint);
            Assert.Equal(60, testConsConfig.MaxDispatchCountApiThread);
            Assert.Equal(70, testConsConfig.MaxDispatchCountUserThread);
            Assert.Equal((ulong)80, testConsConfig.MaxOutstandingPosts);
            Assert.False(testConsConfig.MsgKeyInUpdates);
            Assert.False(testConsConfig.ObeyOpenWindow);
            Assert.Equal((ulong)100, testConsConfig.PostAckTimeout);
            Assert.Equal(-1, testConsConfig.ReconnectAttemptLimit);
            Assert.Equal(1400, testConsConfig.ReconnectMaxDelay);
            Assert.Equal(1300, testConsConfig.ReconnectMinDelay);
            Assert.Equal((ulong)140, testConsConfig.RequestTimeout);
            Assert.True(testConsConfig.RestEnableLog);
            Assert.True(testConsConfig.RestEnableLogViaCallback);
            Assert.Equal("testRestLog", testConsConfig.RestLogFileName);
            Assert.Equal((ulong)150, testConsConfig.RestRequestTimeOut);
            Assert.Equal(160, testConsConfig.ServiceCountHint);
            // XML tracing block
            Assert.True(testConsConfig.XmlTraceToStdout);
            Assert.Equal("proxy.local", testConsConfig.RestProxyHostName);
            Assert.Equal("3128", testConsConfig.RestProxyPort);
            Assert.False(testConsConfig.XmlTraceToFile);
            Assert.Equal((ulong)100_000_000, testConsConfig.XmlTraceMaxFileSize);
            Assert.Equal("EmaTrace", testConsConfig.XmlTraceFileName);
            Assert.False(testConsConfig.XmlTraceToMultipleFiles);
            Assert.True(testConsConfig.XmlTraceWrite);
            Assert.True(testConsConfig.XmlTraceRead);
            Assert.False(testConsConfig.XmlTracePing);
        }

        [Fact]
        public void ConsumerWithDefaultsExceptChannelSetTest()
        {
            OmmConsumerConfigImpl consConfigImpl = LoadEmaTestConfig().OmmConsConfigImpl;
            Assert.True(consConfigImpl.ConsumerConfigMap.ContainsKey("TestConsumer_2"));
            var testConsConfig = consConfigImpl.ConsumerConfigMap["TestConsumer_2"];

            Assert.Equal("TestConsumer_2", testConsConfig.Name);
            Assert.Equal(2, testConsConfig.ChannelSet.Count);
            Assert.Equal("TestChannel_1", testConsConfig.ChannelSet[0]);
            Assert.Equal("TestChannel_2", testConsConfig.ChannelSet[1]);  
        }

        private static void AssertConsumerConfigIsDefault(ConsumerConfig testConsConfig)
        {
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
            Assert.Equal(defaultConsConfig.RestProxyHostName, testConsConfig.RestProxyHostName);
            Assert.Equal(defaultConsConfig.RestProxyPort, testConsConfig.RestProxyPort);
            // XML tracing block
            Assert.Equal(defaultConsConfig.XmlTraceFileName, testConsConfig.XmlTraceFileName);
            Assert.Equal(defaultConsConfig.XmlTraceToStdout, testConsConfig.XmlTraceToStdout);
            Assert.Equal(defaultConsConfig.XmlTraceToFile, testConsConfig.XmlTraceToFile);
            Assert.Equal(defaultConsConfig.XmlTraceMaxFileSize, testConsConfig.XmlTraceMaxFileSize);
            Assert.Equal(defaultConsConfig.XmlTraceToMultipleFiles, testConsConfig.XmlTraceToMultipleFiles);
            Assert.Equal(defaultConsConfig.XmlTraceWrite, testConsConfig.XmlTraceWrite);
            Assert.Equal(defaultConsConfig.XmlTraceRead, testConsConfig.XmlTraceRead);
            Assert.Equal(defaultConsConfig.XmlTracePing, testConsConfig.XmlTracePing);
        }

        [Theory]
        [MemberData(nameof(GetTestObjects))]
        internal void ChannelWithNondefaultsTest(OmmConsumerConfigImpl consConfigImpl)
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
            OmmConsumerConfigImpl consConfigImpl = LoadEmaTestConfig().OmmConsConfigImpl;
            var testChannelConfig = consConfigImpl.ClientChannelConfigMap["TestChannel_2"];
            Assert.Equal("TestChannel_2", testChannelConfig.Name);
            AssertChannelConfigIsDefault(testChannelConfig);
        }

        private static void AssertChannelConfigIsDefault(ClientChannelConfig testChannelConfig)
        {
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

        [Theory]
        [MemberData(nameof(GetTestObjects))]
        internal void LoggerWithNonDefaultsTest(OmmConsumerConfigImpl consConfigImpl)
        {
            Assert.True(consConfigImpl.LoggerConfigMap.ContainsKey("TestLogger_1"));
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
            OmmConsumerConfigImpl consConfigImpl = LoadEmaTestConfig().OmmConsConfigImpl;
            var testLoggerConfig = consConfigImpl.LoggerConfigMap["TestLogger_2"];
            Assert.Equal("TestLogger_2", testLoggerConfig.Name);
            AssertLoggerConfigIsDefault(testLoggerConfig);
        }

        private static void AssertLoggerConfigIsDefault(LoggerConfig testLoggerConfig)
        {
            Assert.Equal(defaultLoggerConfig.LoggerType, testLoggerConfig.LoggerType);
            Assert.Equal(defaultLoggerConfig.LoggerSeverity, testLoggerConfig.LoggerSeverity);
            Assert.Equal(defaultLoggerConfig.FileName, testLoggerConfig.FileName);
            Assert.Equal(defaultLoggerConfig.IncludeDateInLoggerOutput, testLoggerConfig.IncludeDateInLoggerOutput);
            Assert.Equal(defaultLoggerConfig.NumberOfLogFiles, testLoggerConfig.NumberOfLogFiles);
            Assert.Equal(defaultLoggerConfig.MaxLogFileSize, testLoggerConfig.MaxLogFileSize);
        }

        [Theory]
        [MemberData(nameof(GetTestObjects))]
        internal void DictionaryWithNonDefaultsTest(OmmConsumerConfigImpl consConfigImpl)
        {
            Assert.True(consConfigImpl.DictionaryConfigMap.ContainsKey("TestDictionary_1"));
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
            OmmConsumerConfigImpl consConfigImpl = LoadEmaTestConfig().OmmConsConfigImpl;
            var testDictConfig = consConfigImpl.DictionaryConfigMap["TestDictionary_2"];
            AssertDictionaryIsDefualt(testDictConfig);
        }

        private static void AssertDictionaryIsDefualt(DictionaryConfig testDictConfig)
        {
            Assert.Equal(defaultDictConfig.DictionaryType, testDictConfig.DictionaryType);
            Assert.Equal(defaultDictConfig.EnumTypeDefFileName, testDictConfig.EnumTypeDefFileName);
            Assert.Equal(defaultDictConfig.EnumTypeDefItemName, testDictConfig.EnumTypeDefItemName);
            Assert.Equal(defaultDictConfig.RdmFieldDictionaryFileName, testDictConfig.RdmFieldDictionaryFileName);
            Assert.Equal(defaultDictConfig.RdmFieldDictionaryItemName, testDictConfig.RdmFieldDictionaryItemName);
        }

        // Xml Config loading and parsing test
        // This loads a config that contains all elements used in the OmmConsumer config, tests the external setter and getter methods,
        // and verifies that the Reactor ConnectInfo and Reactor Role generation methods work, ensuring that all config members are correctly set
        // in the Role and ConnectInfo objects.
        [Fact]
        public void ConsumerXmlConfigTest()
        {
            OmmConsumerConfig consumerConfig;
            ConsumerConfig testConsConfig;
            ClientChannelConfig testChannelConfig;
            LoggerConfig testLoggerConfig;
            DictionaryConfig testDictConfig;
            OmmConsumerConfigImpl copiedConfig;
            ReactorConnectOptions testConnOpts;
            ConsumerRole testRole;
            ReactorConnectInfo testConnInfo;

            consumerConfig = new OmmConsumerConfig("../../../OmmConfigTests/EmaTestConfig.xml");

            OmmConsumerConfigImpl consConfigImpl = consumerConfig.OmmConsConfigImpl;
            Assert.Equal("TestConsumer", consConfigImpl.FirstConfiguredConsumerName);
            copiedConfig = new OmmConsumerConfigImpl(consConfigImpl);
            // This config should only have one of each config type.
            Assert.Single(copiedConfig.ConsumerConfigMap);
            Assert.Single(copiedConfig.ClientChannelConfigMap);
            Assert.Single(copiedConfig.LoggerConfigMap);
            Assert.Single(copiedConfig.DictionaryConfigMap);

            // Verify that none of the method-set values have changed
            Assert.Equal(string.Empty, copiedConfig.HostName);
            Assert.Equal(string.Empty, copiedConfig.Port);
            Assert.Equal(string.Empty, copiedConfig.UserName);
            Assert.Equal(string.Empty, copiedConfig.Password);
            Assert.Equal(string.Empty, copiedConfig.ApplicationId);
            Assert.Equal(string.Empty, copiedConfig.ClientId);
            Assert.Equal(string.Empty, copiedConfig.ClientSecret);
            Assert.Equal(string.Empty, copiedConfig.ClientJwk);
            Assert.Equal(string.Empty, copiedConfig.Audience);
            Assert.Equal(string.Empty, copiedConfig.TokenScope);
            Assert.Equal(string.Empty, copiedConfig.TokenUrlV2);
            Assert.Equal(string.Empty, copiedConfig.ServiceDiscoveryUrl);
            Assert.Equal(string.Empty, copiedConfig.ProxyHost);
            Assert.Equal(string.Empty, copiedConfig.ProxyPort);
            Assert.Equal(string.Empty, copiedConfig.ProxyPassword);
            Assert.Equal(string.Empty, copiedConfig.ProxyUserName);

            // Generate the role, check the values in it
            // RTT is set when the Login message is passed into the role, so that is not set here.
            // Also, any channel-related ioctrl config will not be generated into the role.
            testRole = copiedConfig.GenerateConsumerRole();

            Assert.Null(testRole.ReactorOAuthCredential);

            Assert.True(testRole.WatchlistOptions.EnableWatchlist);
            Assert.Equal(copiedConfig.ConsumerConfig.ItemCountHint, testRole.WatchlistOptions.ItemCountHint);
            Assert.Equal(copiedConfig.ConsumerConfig.ObeyOpenWindow, testRole.WatchlistOptions.ObeyOpenWindow);
            Assert.Equal(copiedConfig.ConsumerConfig.PostAckTimeout, testRole.WatchlistOptions.PostAckTimeout);
            Assert.Equal(copiedConfig.ConsumerConfig.RequestTimeout, testRole.WatchlistOptions.RequestTimeout);
            Assert.Equal(copiedConfig.ConsumerConfig.MaxOutstandingPosts, testRole.WatchlistOptions.MaxOutstandingPosts);

            // Generate the connection options.
            testConnOpts = copiedConfig.GenerateReactorConnectOpts();

            Assert.Single(testConnOpts.ConnectionList);
            testConnInfo = testConnOpts.ConnectionList[0];

            Assert.Equal(copiedConfig.ConsumerConfig.ReconnectAttemptLimit, testConnOpts.GetReconnectAttemptLimit());
            Assert.Equal(copiedConfig.ConsumerConfig.ReconnectMinDelay, testConnOpts.GetReconnectMinDelay());
            Assert.Equal(copiedConfig.ConsumerConfig.ReconnectMaxDelay, testConnOpts.GetReconnectMaxDelay());

            Assert.Equal(Eta.Transports.ConnectionType.ENCRYPTED, testConnInfo.ConnectOptions.ConnectionType);
            Assert.Equal(Eta.Transports.ConnectionType.ENCRYPTED, testConnInfo.ConnectOptions.EncryptionOpts.EncryptedProtocol);
            Assert.Equal(60, testConnInfo.ConnectOptions.PingTimeout);
            Assert.True(testConnInfo.EnableSessionManagement);
            Assert.Equal(20, testConnInfo.ConnectOptions.GuaranteedOutputBuffers);
            Assert.Equal(40, testConnInfo.GetInitTimeout());
            Assert.Equal("testInterface", testConnInfo.ConnectOptions.UnifiedNetworkInfo.InterfaceName);
            Assert.Equal("testLocation", testConnInfo.Location);
            Assert.Equal(50, testConnInfo.ConnectOptions.NumInputBuffers);
            Assert.Equal(60, testConnInfo.ServiceDiscoveryRetryCount);
            Assert.Equal(70, testConnInfo.ConnectOptions.SysRecvBufSize);
            Assert.Equal(80, testConnInfo.ConnectOptions.SysSendBufSize);
            Assert.Equal(Eta.Transports.CompressionType.ZLIB, testConnInfo.ConnectOptions.CompressionType);
            Assert.Equal("testChannel1", testConnInfo.ConnectOptions.UnifiedNetworkInfo.Address);
            Assert.Equal("testPort1", testConnInfo.ConnectOptions.UnifiedNetworkInfo.ServiceName);
            Assert.Equal("proxyHost1", testConnInfo.ConnectOptions.ProxyOptions.ProxyHostName);
            Assert.Equal("proxyPort1", testConnInfo.ConnectOptions.ProxyOptions.ProxyPort);
            Assert.True(testConnInfo.ConnectOptions.TcpOpts.TcpNoDelay);
            Assert.Equal(EncryptionProtocolFlags.ENC_TLSV1_2, testConnInfo.ConnectOptions.EncryptionOpts.EncryptionProtocolFlags);

            // OmmConsumerConfig reuse and ConsumerName override test
            // Sets the consumer Name to TestConsumer_2, which has a channelSet containing TestChannel_1 and TestChannel_2
            // Also sets some dummy V2 credentials to make sure they're flowing through the API.
            consumerConfig.ConsumerName("TestConsumer_2");
            consumerConfig.UserName("TestUserName");
            consumerConfig.Password("TestPassword");
            consumerConfig.ApplicationId("TestAppId");
            consumerConfig.ApplicationName("TestAppName");
            consumerConfig.ClientId("TestClientId");
            consumerConfig.ClientSecret("TestClientSecret");
            consumerConfig.ClientJwk("TestClientJwk");
            consumerConfig.Audience("TestAudience");
            consumerConfig.TokenScope("TestTokenScope");
            consumerConfig.TokenUrlV2("TestTokenURL");
            consumerConfig.ServiceDiscoveryUrl("TestServiceDiscoveryUrl");
            consumerConfig.ProxyHost("TestProxyHostOverride");
            consumerConfig.ProxyPort("TestProxyPortOverride");
            consumerConfig.ProxyUserName("TestProxyUsernameOverride");
            consumerConfig.ProxyPassword("testProxyPasswordOverride");

            // Verify that the config works here
            consConfigImpl.VerifyConfiguration();

            copiedConfig = new OmmConsumerConfigImpl(consConfigImpl);

            // This config should have one consumer, logger and dictionary, and 2 client channels.
            Assert.Single(copiedConfig.ConsumerConfigMap);
            Assert.Equal(2, copiedConfig.ClientChannelConfigMap.Count);
            Assert.Single(copiedConfig.LoggerConfigMap);
            Assert.Single(copiedConfig.DictionaryConfigMap);

            // Verify that none of the method-set values have changed
            Assert.Equal(string.Empty, copiedConfig.HostName);
            Assert.Equal(string.Empty, copiedConfig.Port);
            Assert.Equal("TestUserName", copiedConfig.UserName);
            Assert.Equal("TestPassword", copiedConfig.Password);
            Assert.Equal("TestAppId", copiedConfig.ApplicationId);
            Assert.Equal("TestAppName", copiedConfig.ApplicationName);
            Assert.Equal("TestClientId", copiedConfig.ClientId);
            Assert.Equal("TestClientSecret", copiedConfig.ClientSecret);
            Assert.Equal("TestClientJwk", copiedConfig.ClientJwk);
            Assert.Equal("TestAudience", copiedConfig.Audience);
            Assert.Equal("TestTokenScope", copiedConfig.TokenScope);
            Assert.Equal("TestTokenURL", copiedConfig.TokenUrlV2);
            Assert.Equal("TestServiceDiscoveryUrl", copiedConfig.ServiceDiscoveryUrl);
            Assert.Equal("TestProxyHostOverride", copiedConfig.ProxyHost);
            Assert.Equal("TestProxyPortOverride", copiedConfig.ProxyPort);
            Assert.Equal("testProxyPasswordOverride", copiedConfig.ProxyPassword);
            Assert.Equal("TestProxyUsernameOverride", copiedConfig.ProxyUserName);

            // TestChannel_2 is the default
            testChannelConfig = copiedConfig.ClientChannelConfigMap["TestChannel_2"];

            // Check that DefaultEmaLogger exists and that the object in the map is the same as what's set on the OmmConsumerConfigImpl
            Assert.True(copiedConfig.LoggerConfigMap.ContainsKey("DefaultEmaLogger"));
            Assert.Same(copiedConfig.LoggerConfig, copiedConfig.LoggerConfigMap["DefaultEmaLogger"]);
            testLoggerConfig = copiedConfig.LoggerConfigMap["DefaultEmaLogger"];

            Assert.Equal("DefaultEmaLogger", testLoggerConfig.Name);
            Assert.Equal(defaultLoggerConfig.LoggerType, testLoggerConfig.LoggerType);
            Assert.Equal(defaultLoggerConfig.LoggerSeverity, testLoggerConfig.LoggerSeverity);
            Assert.Equal(defaultLoggerConfig.FileName, testLoggerConfig.FileName);
            Assert.Equal(defaultLoggerConfig.IncludeDateInLoggerOutput, testLoggerConfig.IncludeDateInLoggerOutput);
            Assert.Equal(defaultLoggerConfig.NumberOfLogFiles, testLoggerConfig.NumberOfLogFiles);
            Assert.Equal(defaultLoggerConfig.MaxLogFileSize, testLoggerConfig.MaxLogFileSize);

            // Check that DefaultEmaDictionary exists and that the object in the map is the same as what's set on the OmmConsumerConfigImpl
            Assert.True(copiedConfig.DictionaryConfigMap.ContainsKey("DefaultEmaDictionary"));
            Assert.Same(copiedConfig.DictionaryConfig, copiedConfig.DictionaryConfigMap["DefaultEmaDictionary"]);
            testDictConfig = copiedConfig.DictionaryConfigMap["DefaultEmaDictionary"];

            Assert.Equal("DefaultEmaDictionary", testDictConfig.Name);
            AssertDictionaryIsDefualt(testDictConfig);

            // Generate the role, check the values in it
            // RTT is set when the Login message is passed into the role, so that is not set here.
            // Also, any channel-related ioctrl config will not be generated into the role.
            testRole = copiedConfig.GenerateConsumerRole();

            Assert.Equal(defaultDictConfig.RdmFieldDictionaryItemName, testRole.FieldDictionaryName.ToString());
            Assert.Equal(defaultDictConfig.EnumTypeDefItemName, testRole.EnumTypeDictionaryName.ToString());

            Assert.NotNull(testRole.ReactorOAuthCredential);
            Assert.Equal("TestClientId", testRole.ReactorOAuthCredential!.ClientId.ToString());
            Assert.Equal("TestClientSecret", testRole.ReactorOAuthCredential.ClientSecret.ToString());
            Assert.Equal("TestClientJwk", testRole.ReactorOAuthCredential.ClientJwk.ToString());
            Assert.Equal("TestAudience", testRole.ReactorOAuthCredential.Audience.ToString());
            Assert.Equal("TestTokenScope", testRole.ReactorOAuthCredential.TokenScope.ToString());

            Assert.Empty(copiedConfig.ClientSecret);
            Assert.Empty(copiedConfig.ClientJwk);

            Assert.True(testRole.WatchlistOptions.EnableWatchlist);
            Assert.Equal(defaultConsConfig.ItemCountHint, testRole.WatchlistOptions.ItemCountHint);
            Assert.Equal(defaultConsConfig.ObeyOpenWindow, testRole.WatchlistOptions.ObeyOpenWindow);
            Assert.Equal(defaultConsConfig.PostAckTimeout, testRole.WatchlistOptions.PostAckTimeout);
            Assert.Equal(defaultConsConfig.RequestTimeout, testRole.WatchlistOptions.RequestTimeout);
            Assert.Equal(defaultConsConfig.MaxOutstandingPosts, testRole.WatchlistOptions.MaxOutstandingPosts);

            // Generate the connection options, verify that the channel set is correct.
            testConnOpts = copiedConfig.GenerateReactorConnectOpts();

            Assert.Equal(2, testConnOpts.ConnectionList.Count);
            testConnInfo = testConnOpts.ConnectionList[0];

            Assert.Equal(testConnOpts.GetReconnectAttemptLimit(), copiedConfig.ConsumerConfig.ReconnectAttemptLimit);
            Assert.Equal(testConnOpts.GetReconnectMinDelay(), copiedConfig.ConsumerConfig.ReconnectMinDelay);
            Assert.Equal(testConnOpts.GetReconnectMaxDelay(), copiedConfig.ConsumerConfig.ReconnectMaxDelay);

            Assert.Equal(Eta.Transports.ConnectionType.ENCRYPTED, testConnInfo.ConnectOptions.ConnectionType);
            Assert.Equal(Eta.Transports.ConnectionType.ENCRYPTED, testConnInfo.ConnectOptions.EncryptionOpts.EncryptedProtocol);
            Assert.Equal(60, testConnInfo.ConnectOptions.PingTimeout);
            Assert.True(testConnInfo.EnableSessionManagement);
            Assert.Equal(20, testConnInfo.ConnectOptions.GuaranteedOutputBuffers);
            Assert.Equal(40, testConnInfo.GetInitTimeout());
            Assert.Equal("testInterface", testConnInfo.ConnectOptions.UnifiedNetworkInfo.InterfaceName);
            Assert.Equal("testLocation", testConnInfo.Location);
            Assert.Equal(50, testConnInfo.ConnectOptions.NumInputBuffers);
            Assert.Equal(60, testConnInfo.ServiceDiscoveryRetryCount);
            Assert.Equal(70, testConnInfo.ConnectOptions.SysRecvBufSize);
            Assert.Equal(80, testConnInfo.ConnectOptions.SysSendBufSize);
            Assert.Equal(Eta.Transports.CompressionType.ZLIB, testConnInfo.ConnectOptions.CompressionType);
            Assert.Equal("testChannel1", testConnInfo.ConnectOptions.UnifiedNetworkInfo.Address);
            Assert.Equal("testPort1", testConnInfo.ConnectOptions.UnifiedNetworkInfo.ServiceName);
            Assert.Equal("TestProxyHostOverride", testConnInfo.ConnectOptions.ProxyOptions.ProxyHostName);
            Assert.Equal("TestProxyPortOverride", testConnInfo.ConnectOptions.ProxyOptions.ProxyPort);
            Assert.Equal("TestProxyUsernameOverride", testChannelConfig.ConnectInfo.ConnectOptions.ProxyOptions.ProxyUserName);
            Assert.Equal("testProxyPasswordOverride", testChannelConfig.ConnectInfo.ConnectOptions.ProxyOptions.ProxyPassword);
            Assert.True(testConnInfo.ConnectOptions.TcpOpts.TcpNoDelay);
            Assert.Equal(EncryptionProtocolFlags.ENC_TLSV1_2, testConnInfo.ConnectOptions.EncryptionOpts.EncryptionProtocolFlags);

            // Check the second channel, which should be all defaults.
            testConnInfo = testConnOpts.ConnectionList[1];

            Assert.Equal(defaultChannelConfig.ConnectInfo.ConnectOptions.ConnectionType, testConnInfo.ConnectOptions.ConnectionType);
            Assert.Equal(defaultChannelConfig.ConnectInfo.ConnectOptions.EncryptionOpts.EncryptedProtocol, testConnInfo.ConnectOptions.EncryptionOpts.EncryptedProtocol);
            Assert.Equal(defaultChannelConfig.ConnectInfo.ConnectOptions.PingTimeout, testConnInfo.ConnectOptions.PingTimeout);
            Assert.Equal(defaultChannelConfig.ConnectInfo.EnableSessionManagement, testConnInfo.EnableSessionManagement);
            Assert.Equal(defaultChannelConfig.ConnectInfo.ConnectOptions.GuaranteedOutputBuffers, testConnInfo.ConnectOptions.GuaranteedOutputBuffers);
            Assert.Equal(defaultChannelConfig.ConnectInfo.GetInitTimeout(), testConnInfo.GetInitTimeout());
            Assert.Equal(defaultChannelConfig.ConnectInfo.ConnectOptions.UnifiedNetworkInfo.InterfaceName, testConnInfo.ConnectOptions.UnifiedNetworkInfo.InterfaceName);
            Assert.Equal(defaultChannelConfig.ConnectInfo.Location, testConnInfo.Location);
            Assert.Equal(defaultChannelConfig.ConnectInfo.ConnectOptions.NumInputBuffers, testConnInfo.ConnectOptions.NumInputBuffers);
            Assert.Equal(defaultChannelConfig.ConnectInfo.ServiceDiscoveryRetryCount, testConnInfo.ServiceDiscoveryRetryCount);
            Assert.Equal(defaultChannelConfig.ConnectInfo.ConnectOptions.SysRecvBufSize, testConnInfo.ConnectOptions.SysRecvBufSize);
            Assert.Equal(defaultChannelConfig.ConnectInfo.ConnectOptions.SysSendBufSize, testConnInfo.ConnectOptions.SysSendBufSize);
            Assert.Equal(defaultChannelConfig.ConnectInfo.ConnectOptions.CompressionType, testConnInfo.ConnectOptions.CompressionType);
            Assert.Equal(defaultChannelConfig.ConnectInfo.ConnectOptions.UnifiedNetworkInfo.Address, testConnInfo.ConnectOptions.UnifiedNetworkInfo.Address);
            Assert.Equal(defaultChannelConfig.ConnectInfo.ConnectOptions.UnifiedNetworkInfo.ServiceName, testConnInfo.ConnectOptions.UnifiedNetworkInfo.ServiceName);
            Assert.Equal(defaultChannelConfig.ConnectInfo.ConnectOptions.GuaranteedOutputBuffers, testConnInfo.ConnectOptions.GuaranteedOutputBuffers);
            Assert.Equal("TestProxyHostOverride", testConnInfo.ConnectOptions.ProxyOptions.ProxyHostName);
            Assert.Equal("TestProxyPortOverride", testConnInfo.ConnectOptions.ProxyOptions.ProxyPort);
            Assert.Equal("TestProxyUsernameOverride", testConnInfo.ConnectOptions.ProxyOptions.ProxyUserName);
            Assert.Equal("testProxyPasswordOverride", testConnInfo.ConnectOptions.ProxyOptions.ProxyPassword);
            Assert.Equal(defaultChannelConfig.ConnectInfo.ConnectOptions.TcpOpts.TcpNoDelay, testConnInfo.ConnectOptions.TcpOpts.TcpNoDelay);
            Assert.Equal(defaultChannelConfig.ConnectInfo.ConnectOptions.EncryptionOpts.EncryptionProtocolFlags, testConnInfo.ConnectOptions.EncryptionOpts.EncryptionProtocolFlags);

            // Final re-use test, set the hostname and port, username and password.  Make sure all of this flows through. setting a single channel with all defaults except for port and host
            // with socket connection type.
            // Already tested the method that parses the host string, so we don't need to test all the variants here.
            consumerConfig.Host("TestHostOverride:TestPortOverride");

            // Verify that the config works here
            consConfigImpl.VerifyConfiguration();

            copiedConfig = new OmmConsumerConfigImpl(consConfigImpl);

            // This config should have one consumer, logger and dictionary, and 1 client channel.
            Assert.Single(copiedConfig.ConsumerConfigMap);
            Assert.Single(copiedConfig.ClientChannelConfigMap);
            Assert.Single(copiedConfig.LoggerConfigMap);
            Assert.Single(copiedConfig.DictionaryConfigMap);

            // Verify that none of the method-set values have changed
            Assert.Equal("TestHostOverride", copiedConfig.HostName);
            Assert.Equal("TestPortOverride", copiedConfig.Port);
            Assert.Equal("TestUserName", copiedConfig.UserName);
            Assert.Equal("TestPassword", copiedConfig.Password);
            Assert.Equal("TestAppId", copiedConfig.ApplicationId);
            Assert.Equal("TestClientId", copiedConfig.ClientId);
            Assert.Equal("TestClientSecret", copiedConfig.ClientSecret);
            Assert.Equal("TestClientJwk", copiedConfig.ClientJwk);
            Assert.Equal("TestAudience", copiedConfig.Audience);
            Assert.Equal("TestTokenScope", copiedConfig.TokenScope);
            Assert.Equal("TestTokenURL", copiedConfig.TokenUrlV2);
            Assert.Equal("TestServiceDiscoveryUrl", copiedConfig.ServiceDiscoveryUrl);
            Assert.Equal("TestProxyHostOverride", copiedConfig.ProxyHost);
            Assert.Equal("TestProxyPortOverride", copiedConfig.ProxyPort);
            Assert.Equal("testProxyPasswordOverride", copiedConfig.ProxyPassword);
            Assert.Equal("TestProxyUsernameOverride", copiedConfig.ProxyUserName);

            // Verify that the ConsumerName has been properly set to the defaultEmaConsumer, that the copied Config has the TestConsumer in the consumer config map, and that 
            // the ConsumerConfig object referenced by ConsumerConfig is the same as in the map.
            Assert.Equal("TestConsumer_2", copiedConfig.ConsumerName);
            Assert.True(copiedConfig.ConsumerConfigMap.ContainsKey("TestConsumer_2"));
            Assert.Same(copiedConfig.ConsumerConfig, copiedConfig.ConsumerConfigMap["TestConsumer_2"]);

            testConsConfig = copiedConfig.ConsumerConfigMap["TestConsumer_2"];

            Assert.Equal("TestConsumer_2", testConsConfig.Name);
            Assert.Single(testConsConfig.ChannelSet);
            Assert.Equal("TestChannel_1", testConsConfig.ChannelSet[0]);
            Assert.Equal("DefaultEmaLogger", testConsConfig.Logger);
            Assert.Equal("DefaultEmaDictionary", testConsConfig.Dictionary);
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
            // XML tracing block
            Assert.Equal(defaultConsConfig.XmlTraceFileName, testConsConfig.XmlTraceFileName);
            Assert.Equal(defaultConsConfig.XmlTraceToStdout, testConsConfig.XmlTraceToStdout);
            Assert.Equal(defaultConsConfig.XmlTraceToFile, testConsConfig.XmlTraceToFile);
            Assert.Equal(defaultConsConfig.XmlTraceMaxFileSize, testConsConfig.XmlTraceMaxFileSize);
            Assert.Equal(defaultConsConfig.XmlTraceToMultipleFiles, testConsConfig.XmlTraceToMultipleFiles);
            Assert.Equal(defaultConsConfig.XmlTraceWrite, testConsConfig.XmlTraceWrite);
            Assert.Equal(defaultConsConfig.XmlTraceRead, testConsConfig.XmlTraceRead);
            Assert.Equal(defaultConsConfig.XmlTracePing, testConsConfig.XmlTracePing);

            // DefaultEmaChannel should be all defaults except for the host, port, and proxy information
            testChannelConfig = copiedConfig.ClientChannelConfigMap["TestChannel_1"];

            Assert.Equal("TestChannel_1", testChannelConfig.Name);
            Assert.Equal(Eta.Transports.ConnectionType.SOCKET, testChannelConfig.ConnectInfo.ConnectOptions.ConnectionType);
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
            Assert.True(testChannelConfig.DirectWrite);
            Assert.Equal("TestHostOverride", testChannelConfig.ConnectInfo.ConnectOptions.UnifiedNetworkInfo.Address);
            Assert.Equal("TestPortOverride", testChannelConfig.ConnectInfo.ConnectOptions.UnifiedNetworkInfo.ServiceName);
            Assert.Equal("TestProxyHostOverride", testChannelConfig.ConnectInfo.ConnectOptions.ProxyOptions.ProxyHostName);
            Assert.Equal("TestProxyPortOverride", testChannelConfig.ConnectInfo.ConnectOptions.ProxyOptions.ProxyPort);
            Assert.Equal("TestProxyUsernameOverride", testChannelConfig.ConnectInfo.ConnectOptions.ProxyOptions.ProxyUserName);
            Assert.Equal("testProxyPasswordOverride", testChannelConfig.ConnectInfo.ConnectOptions.ProxyOptions.ProxyPassword);
            Assert.True(testChannelConfig.ConnectInfo.ConnectOptions.TcpOpts.TcpNoDelay);
            Assert.Equal(EncryptionProtocolFlags.ENC_TLSV1_2, testChannelConfig.ConnectInfo.ConnectOptions.EncryptionOpts.EncryptionProtocolFlags);

            // Check that DefaultEmaLogger exists and that the object in the map is the same as what's set on the OmmConsumerConfigImpl
            Assert.True(copiedConfig.LoggerConfigMap.ContainsKey("DefaultEmaLogger"));
            Assert.Same(copiedConfig.LoggerConfig, copiedConfig.LoggerConfigMap["DefaultEmaLogger"]);

            testLoggerConfig = copiedConfig.LoggerConfigMap["DefaultEmaLogger"];

            Assert.Equal("DefaultEmaLogger", testLoggerConfig.Name);
            AssertLoggerConfigIsDefault(testLoggerConfig);

            // Check that DefaultEmaDictionary exists and that the object in the map is the same as what's set on the OmmConsumerConfigImpl
            Assert.True(copiedConfig.DictionaryConfigMap.ContainsKey("DefaultEmaDictionary"));
            Assert.Same(copiedConfig.DictionaryConfig, copiedConfig.DictionaryConfigMap["DefaultEmaDictionary"]);
            testDictConfig = copiedConfig.DictionaryConfigMap["DefaultEmaDictionary"];

            Assert.Equal("DefaultEmaDictionary", testDictConfig.Name);
            AssertDictionaryIsDefualt(testDictConfig);

            // Generate the role, check the values in it
            // RTT is set when the Login message is passed into the role, so that is not set here.
            // Also, any channel-related ioctrl config will not be generated into the role.
            testRole = copiedConfig.GenerateConsumerRole();

            Assert.Equal(defaultDictConfig.RdmFieldDictionaryItemName, testRole.FieldDictionaryName.ToString());
            Assert.Equal(defaultDictConfig.EnumTypeDefItemName, testRole.EnumTypeDictionaryName.ToString());

            Assert.NotNull(testRole.ReactorOAuthCredential);
            Assert.Equal("TestClientId", testRole.ReactorOAuthCredential!.ClientId.ToString());
            Assert.Equal("TestClientSecret", testRole.ReactorOAuthCredential.ClientSecret.ToString());
            Assert.Equal("TestClientJwk", testRole.ReactorOAuthCredential.ClientJwk.ToString());
            Assert.Equal("TestAudience", testRole.ReactorOAuthCredential.Audience.ToString());
            Assert.Equal("TestTokenScope", testRole.ReactorOAuthCredential.TokenScope.ToString());

            Assert.Empty(copiedConfig.ClientSecret);
            Assert.Empty(copiedConfig.ClientJwk);

            Assert.True(testRole.WatchlistOptions.EnableWatchlist);
            Assert.Equal(defaultConsConfig.ItemCountHint, testRole.WatchlistOptions.ItemCountHint);
            Assert.Equal(defaultConsConfig.ObeyOpenWindow, testRole.WatchlistOptions.ObeyOpenWindow);
            Assert.Equal(defaultConsConfig.PostAckTimeout, testRole.WatchlistOptions.PostAckTimeout);
            Assert.Equal(defaultConsConfig.RequestTimeout, testRole.WatchlistOptions.RequestTimeout);
            Assert.Equal(defaultConsConfig.MaxOutstandingPosts, testRole.WatchlistOptions.MaxOutstandingPosts);

            // Generate the connection options, verify that the channel set is correct.
            testConnOpts = copiedConfig.GenerateReactorConnectOpts();

            Assert.Single(testConnOpts.ConnectionList);
            testConnInfo = testConnOpts.ConnectionList[0];

            Assert.Equal(testConnOpts.GetReconnectAttemptLimit(), copiedConfig.ConsumerConfig.ReconnectAttemptLimit);
            Assert.Equal(testConnOpts.GetReconnectMinDelay(), copiedConfig.ConsumerConfig.ReconnectMinDelay);
            Assert.Equal(testConnOpts.GetReconnectMaxDelay(), copiedConfig.ConsumerConfig.ReconnectMaxDelay);

            Assert.Equal(Eta.Transports.ConnectionType.SOCKET, testConnInfo.ConnectOptions.ConnectionType);
            Assert.Equal(Eta.Transports.ConnectionType.ENCRYPTED, testConnInfo.ConnectOptions.EncryptionOpts.EncryptedProtocol);
            Assert.Equal(60, testConnInfo.ConnectOptions.PingTimeout);
            Assert.True(testConnInfo.EnableSessionManagement);
            Assert.Equal(20, testConnInfo.ConnectOptions.GuaranteedOutputBuffers);
            Assert.Equal(40, testConnInfo.GetInitTimeout());
            Assert.Equal("testInterface", testConnInfo.ConnectOptions.UnifiedNetworkInfo.InterfaceName);
            Assert.Equal("testLocation", testConnInfo.Location);
            Assert.Equal(50, testConnInfo.ConnectOptions.NumInputBuffers);
            Assert.Equal(60, testConnInfo.ServiceDiscoveryRetryCount);
            Assert.Equal(70, testConnInfo.ConnectOptions.SysRecvBufSize);
            Assert.Equal(80, testConnInfo.ConnectOptions.SysSendBufSize);
            Assert.Equal(Eta.Transports.CompressionType.ZLIB, testConnInfo.ConnectOptions.CompressionType);
            Assert.Equal("TestHostOverride", testConnInfo.ConnectOptions.UnifiedNetworkInfo.Address);
            Assert.Equal("TestPortOverride", testConnInfo.ConnectOptions.UnifiedNetworkInfo.ServiceName);
            Assert.Equal("TestProxyHostOverride", testConnInfo.ConnectOptions.ProxyOptions.ProxyHostName);
            Assert.Equal("TestProxyPortOverride", testConnInfo.ConnectOptions.ProxyOptions.ProxyPort);
            Assert.Equal("TestProxyUsernameOverride", testConnInfo.ConnectOptions.ProxyOptions.ProxyUserName);
            Assert.Equal("testProxyPasswordOverride", testConnInfo.ConnectOptions.ProxyOptions.ProxyPassword);
            Assert.True(testConnInfo.ConnectOptions.TcpOpts.TcpNoDelay);
            Assert.Equal(EncryptionProtocolFlags.ENC_TLSV1_2, testConnInfo.ConnectOptions.EncryptionOpts.EncryptionProtocolFlags);
        }

        // This test will a blank config and verify that a default-only generated config is created.
        [Fact]
        public void ConsumerDefaultConfigTest()
        {
            OmmConsumerConfig consumerConfig;
            ConsumerConfig testConsConfig;
            ClientChannelConfig testChannelConfig;
            LoggerConfig testLoggerConfig;
            DictionaryConfig testDictionaryConfig;
            OmmConsumerConfigImpl copiedConfig;

            // Load a blank config so we can be sure that everything added is from the programmtic config
            // This doesn't use the default behavior(loading EmaConfig.xml) because an EmaConfig.xml file may be present in the directory for other example apps
            consumerConfig = new OmmConsumerConfig("../../../OmmConfigTests/EmaBlankConfig.xml");

            OmmConsumerConfigImpl consConfigImpl = consumerConfig.OmmConsConfigImpl;

            // Loaded the Config, now make sure everything's in it.

            // There should be nothing in here
            Assert.Empty(consConfigImpl.ConsumerConfigMap);
            Assert.Empty(consConfigImpl.ClientChannelConfigMap);
            Assert.Empty(consConfigImpl.LoggerConfigMap);
            Assert.Empty(consConfigImpl.DictionaryConfigMap);

            // Verify should succeed
            consConfigImpl.VerifyConfiguration();

            copiedConfig = new OmmConsumerConfigImpl(consConfigImpl);


            // There should be one default-generated consumer, and one default-generated channel in the maps
            // Logger and Dictionary are set in copiedConfig.Logger and copiedConfig.Dictionary, respectively
            Assert.Single(copiedConfig.ConsumerConfigMap);
            Assert.Single(copiedConfig.ClientChannelConfigMap);
            Assert.Single(copiedConfig.LoggerConfigMap);
            Assert.Single(copiedConfig.DictionaryConfigMap);

            // DefaultEmaConsumer has all defaults and generated names
            testConsConfig = copiedConfig.ConsumerConfigMap["DefaultEmaConsumer"];

            Assert.Same(copiedConfig.ConsumerConfig, copiedConfig.ConsumerConfigMap["DefaultEmaConsumer"]);
            Assert.Equal("DefaultEmaConsumer", testConsConfig.Name);
            Assert.Single(testConsConfig.ChannelSet);
            Assert.Equal("DefaultEmaChannel", testConsConfig.ChannelSet[0]);
            Assert.Equal("DefaultEmaLogger", testConsConfig.Logger);
            Assert.Equal("DefaultEmaDictionary", testConsConfig.Dictionary);
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
            // XML tracing block
            Assert.Equal(defaultConsConfig.XmlTraceToStdout, testConsConfig.XmlTraceToStdout);
            Assert.Equal(defaultConsConfig.XmlTraceToFile, testConsConfig.XmlTraceToFile);
            Assert.Equal(defaultConsConfig.XmlTraceMaxFileSize, testConsConfig.XmlTraceMaxFileSize);
            Assert.Equal(defaultConsConfig.XmlTraceFileName, testConsConfig.XmlTraceFileName);
            Assert.Equal(defaultConsConfig.XmlTraceToMultipleFiles, testConsConfig.XmlTraceToMultipleFiles);
            Assert.Equal(defaultConsConfig.XmlTraceWrite, testConsConfig.XmlTraceWrite);
            Assert.Equal(defaultConsConfig.XmlTraceRead, testConsConfig.XmlTraceRead);
            Assert.Equal(defaultConsConfig.XmlTracePing, testConsConfig.XmlTracePing);


            // DefaultEmaChannel is the generated default channel
            testChannelConfig = copiedConfig.ClientChannelConfigMap["DefaultEmaChannel"];

            Assert.Equal("DefaultEmaChannel", testChannelConfig.Name);
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
            Assert.Equal("localhost", testChannelConfig.ConnectInfo.ConnectOptions.UnifiedNetworkInfo.Address);
            Assert.Equal("14002", testChannelConfig.ConnectInfo.ConnectOptions.UnifiedNetworkInfo.ServiceName);
            Assert.Equal(defaultChannelConfig.ConnectInfo.ConnectOptions.GuaranteedOutputBuffers, testChannelConfig.ConnectInfo.ConnectOptions.GuaranteedOutputBuffers);
            Assert.Equal(defaultChannelConfig.ConnectInfo.ConnectOptions.ProxyOptions.ProxyHostName, testChannelConfig.ConnectInfo.ConnectOptions.ProxyOptions.ProxyHostName);
            Assert.Equal(defaultChannelConfig.ConnectInfo.ConnectOptions.ProxyOptions.ProxyPort, testChannelConfig.ConnectInfo.ConnectOptions.ProxyOptions.ProxyPort);
            Assert.Equal(defaultChannelConfig.ConnectInfo.ConnectOptions.TcpOpts.TcpNoDelay, testChannelConfig.ConnectInfo.ConnectOptions.TcpOpts.TcpNoDelay);
            Assert.Equal(defaultChannelConfig.ConnectInfo.ConnectOptions.EncryptionOpts.EncryptionProtocolFlags, testChannelConfig.ConnectInfo.ConnectOptions.EncryptionOpts.EncryptionProtocolFlags);
            Assert.False(defaultChannelConfig.CompressionThresholdSet);

            // The logger will be generated and contain all defaults
            testLoggerConfig = copiedConfig.LoggerConfigMap["DefaultEmaLogger"];

            Assert.Same(copiedConfig.LoggerConfig, copiedConfig.LoggerConfigMap["DefaultEmaLogger"]);
            Assert.Equal("DefaultEmaLogger", testLoggerConfig.Name);
            AssertLoggerConfigIsDefault(testLoggerConfig);

            // The Dictionary will be generated and contain all defaults
            testDictionaryConfig = copiedConfig.DictionaryConfigMap["DefaultEmaDictionary"];

            Assert.Same(copiedConfig.DictionaryConfig, copiedConfig.DictionaryConfigMap["DefaultEmaDictionary"]);
            Assert.Equal("DefaultEmaDictionary", testDictionaryConfig.Name);
            AssertDictionaryIsDefualt(testDictionaryConfig);
        }
    }
}
