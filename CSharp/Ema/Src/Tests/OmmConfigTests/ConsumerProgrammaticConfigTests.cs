/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2024 Refinitiv. All rights reserved.        --
 *|-----------------------------------------------------------------------------
 */

using LSEG.Eta.Rdm;
using LSEG.Eta.Transports;
using LSEG.Eta.ValueAdd.Reactor;
using System.Collections.Generic;
using static LSEG.Ema.Access.EmaConfig;
using static LSEG.Ema.Access.Tests.OmmConfigTests.ConfigTestsUtils;
using static LSEG.Eta.Rdm.Directory;

namespace LSEG.Ema.Access.Tests.OmmConfigTests
{
    public class ConsumerProgrammaticConfigTests
    {
        private static readonly ConsumerConfig defaultConsConfig = new ();
        private static readonly ClientChannelConfig defaultChannelConfig = new ();
        private static readonly LoggerConfig defaultLoggerConfig = new ();
        private static readonly DictionaryConfig defaultDictConfig = new ();

        [Fact]
        public void RestProxyParamsLoadTest()
        {
            // Arrange
            var expectedProxyHost = "some-proxy.com";
            var expectedProxyPort = "1234";
            var consumerConfig = LoadEmaBlankConfig();
            var configImpl = consumerConfig.OmmConsConfigImpl;

            var emaConfigMap = new Map();
            var groupList = new ElementList();
            var innerMap = new Map();
            var objectPropertiesList = new ElementList();

            emaConfigMap
                .Clear()
                .AddKeyAscii("ConsumerGroup", MapAction.ADD,
                    groupList
                        .Clear()
                        .AddAscii("DefaultConsumer", "ProgConsumer_1")
                        .AddMap("ConsumerList",
                            innerMap
                                .Clear()
                                .AddKeyAscii("ProgConsumer_1", MapAction.ADD,
                                    objectPropertiesList
                                        .Clear()
                                        .AddAscii("RestProxyHostName", expectedProxyHost)
                                        .AddAscii("RestProxyPort", expectedProxyPort)
                                        .Complete())
                                .Complete())
                        .Complete())
                .Complete();

            // Act
            configImpl.Config(emaConfigMap);

            // Assert
            var namedConsumerConfig = configImpl.ConsumerConfigMap[configImpl.DefaultConsumer];
            Assert.Equal(expectedProxyHost, namedConsumerConfig.RestProxyHostName);
            Assert.Equal(expectedProxyPort, namedConsumerConfig.RestProxyPort);
        }

        // OmmConsumerConfig.Host() test
        // Tests the Host string parsing with the following scenarios:
        // ""(blank) => Default Hostname(localhost) and default port(14002)
        // [Hostname] => Configured Hostname and default port
        // [Hostname]: => Configured Hostname and default port
        // :[Port] => Default Hostname and Port
        // [Hostname]:[Port] => Configured Hostname and Configured Port
        [Fact]
        public void StringToHostMethodTest()
        {
            // Blank path, will not load any Xml here.
            OmmConsumerConfigImpl configImpl = new OmmConsumerConfigImpl("");

            configImpl.Clear();

            Assert.Equal(string.Empty, configImpl.HostName);
            Assert.Equal(string.Empty, configImpl.Port);

            // Blank input
            configImpl.Host("");
            Assert.Equal("localhost", configImpl.HostName);
            Assert.Equal("14002", configImpl.Port);

            configImpl.Clear();

            // [Hostname]
            configImpl.Host("testHost");
            Assert.Equal("testHost", configImpl.HostName);
            Assert.Equal("14002", configImpl.Port);
            configImpl.Clear();

            // [Hostname]:
            configImpl.Host("testHost:");
            Assert.Equal("testHost", configImpl.HostName);
            Assert.Equal("14002", configImpl.Port);
            configImpl.Clear();

            // :[Port]
            configImpl.Host(":12345");
            Assert.Equal("localhost", configImpl.HostName);
            Assert.Equal("12345", configImpl.Port);
            configImpl.Clear();

            // [Hostname]:[Port]
            configImpl.Host("testHost:12345");
            Assert.Equal("testHost", configImpl.HostName);
            Assert.Equal("12345", configImpl.Port);
            configImpl.Clear();
        }


        [Fact]
        public void ConsumerProgrammaticConfigTest()
        {
            OmmConsumerConfig consumerConfig;
            ConsumerConfig testConsConfig;
            ClientChannelConfig testChannelConfig;
            LoggerConfig testLoggerConfig;
            DictionaryConfig testDictConfig;

            // Top level map
            Map outerMap = new Map();
            // Middle map for consumers, channels, loggers, dictionaries
            Map innerMap = new Map();
            // Outer element list 
            ElementList encodeGroupList = new ElementList();
            ElementList encodeObjectList = new ElementList();

            // Load a blank config so we can be sure that everything added is from the programmtic config
            // This doesn't use the default behavior(loading EmaConfig.xml) because an EmaConfig.xml file may be present in the directory for other example apps
            consumerConfig = LoadEmaTestConfig();


            OmmConsumerConfigImpl consConfigImpl = consumerConfig.OmmConsConfigImpl;

            // Loaded the Config, now make sure everything's in it.

            // There should be 2 Consumers, 2 ClientChannels, 2 Loggers, and 2 Dictionaries
            Assert.Equal(2, consConfigImpl.ConsumerConfigMap.Count);
            Assert.Equal(2, consConfigImpl.ClientChannelConfigMap.Count);
            Assert.Equal(2, consConfigImpl.LoggerConfigMap.Count);
            Assert.Equal(2, consConfigImpl.DictionaryConfigMap.Count);


            outerMap.Clear();

            innerMap.Clear();

            encodeGroupList.Clear();

            encodeObjectList.Clear();

            // Encode ProgrammaticConsumer_1
            //
            encodeObjectList.AddAscii("Channel", "ProgChannel_1")
                .AddAscii("Dictionary", "ProgDictionary_1")
                .AddAscii("Logger", "ProgLogger_1")
                .AddUInt("DictionaryRequestTimeOut", 2000)
                .AddUInt("DirectoryRequestTimeOut", 2010)
                .AddUInt("LoginRequestTimeOut", 2030)
                .AddInt("DispatchTimeoutApiThread", 2040)
                .AddUInt("EnableRtt", 1)
                .AddUInt("ItemCountHint", 2050)
                .AddUInt("MaxDispatchCountApiThread", 2060)
                .AddUInt("MaxDispatchCountUserThread", 2070)
                .AddUInt("MaxOutstandingPosts", 2080)
                .AddUInt("MsgKeyInUpdates", 0)
                .AddUInt("ObeyOpenWindow", 0)
                .AddUInt("PostAckTimeout", 2090)
                .AddInt("ReconnectAttemptLimit", 2100)
                .AddInt("ReconnectMaxDelay", 2120)
                .AddInt("ReconnectMinDelay", 2110)
                .AddUInt("RequestTimeout", 2130)
                .AddUInt("RestEnableLog", 1)
                .AddUInt("RestEnableLogViaCallback", 1)
                .AddAscii("RestLogFileName", "ProgRestLog")
                .AddUInt("ServiceCountHint", 2140)
                .AddUInt("RestRequestTimeOut", 2150)
                // XML tracing block
                .AddUInt("XmlTraceToStdout", 1)
                .AddUInt("XmlTraceToFile", 0)
                .AddUInt("XmlTraceMaxFileSize", (ulong)10_000_000)
                .AddAscii("XmlTraceFileName", "EmaTrace")
                .AddUInt("XmlTraceToMultipleFiles", 0)
                .AddUInt("XmlTraceWrite", 1)
                .AddUInt("XmlTraceRead", 1)
                .AddUInt("XmlTracePing", 0)
                .Complete();

            innerMap.AddKeyAscii("ProgConsumer_1", MapAction.ADD, encodeObjectList);

            encodeObjectList.Clear();

            encodeObjectList.AddAscii("ChannelSet", "ProgChannel_1, ProgChannel_2")
                .Complete();

            innerMap.AddKeyAscii("ProgConsumer_2", MapAction.ADD, encodeObjectList);

            innerMap.Complete();

            encodeGroupList.Clear();

            encodeGroupList.AddAscii("DefaultConsumer", "ProgConsumer_1")
                .AddMap("ConsumerList", innerMap)
                .Complete();

            outerMap.AddKeyAscii("ConsumerGroup", MapAction.ADD, encodeGroupList);


            // Start encoding the channel connection information
            encodeGroupList.Clear();
            innerMap.Clear();
            encodeObjectList.Clear();

            encodeObjectList.AddEnum("ChannelType", ConnectionTypeEnum.ENCRYPTED)
                .AddEnum("EncryptedProtocolType", ConnectionTypeEnum.ENCRYPTED)
                .AddUInt("ConnectionPingTimeout", 1000)
                .AddUInt("EnableSessionManagement", 1)
                .AddUInt("GuaranteedOutputBuffers", 1010)
                .AddUInt("HighWaterMark", 1020)
                .AddUInt("InitializationTimeout", 1030)
                .AddUInt("AuthenticationTimeout", 15000)
                .AddAscii("InterfaceName", "ProgInterface_1")
                .AddAscii("Location", "ProgLocation_1")
                .AddUInt("NumInputBuffers", 1040)
                .AddUInt("ServiceDiscoveryRetryCount", 1050)
                .AddUInt("SysRecvBufSize", 1060)
                .AddUInt("SysSendBufSize", 1070)
                .AddEnum("CompressionType", CompressionTypeEnum.LZ4)
                .AddUInt("DirectWrite", 1)
                .AddAscii("Host", "ProgHost_1")
                .AddAscii("Port", "ProgPort_1")
                .AddAscii("ProxyHost", "ProgProxyHost_1")
                .AddAscii("ProxyPort", "ProgProxyPort_1")
                .AddUInt("TcpNodelay", 1)
                .AddUInt("SecurityProtocol", EncryptedTLSProtocolFlags.TLSv1_2 | 0x01)
                .AddUInt("CompressionThreshold", 666)
                .Complete();

            innerMap.AddKeyAscii("ProgChannel_1", MapAction.ADD, encodeObjectList);


            // Second channel is all defaults, so add an empty Element List
            encodeObjectList.Clear();
            encodeObjectList.Complete();

            innerMap.AddKeyAscii("ProgChannel_2", MapAction.ADD, encodeObjectList)
                .Complete();

            encodeGroupList.AddMap("ChannelList", innerMap)
                .Complete();

            outerMap.AddKeyAscii("ChannelGroup", MapAction.ADD, encodeGroupList);

            // Start encoding the Logger information
            encodeGroupList.Clear();
            innerMap.Clear();
            encodeObjectList.Clear();

            encodeObjectList.AddAscii("FileName", "ProgLogFile")
                .AddUInt("IncludeDateInLoggerOutput", 1)
                .AddUInt("NumberOfLogFiles", 20)
                .AddUInt("MaxLogFileSize", 100)
                .AddEnum("LoggerSeverity", LoggerLevelEnum.INFO)
                .AddEnum("LoggerType", LoggerTypeEnum.STDOUT)
                .Complete();

            innerMap.AddKeyAscii("ProgLogger_1", MapAction.ADD, encodeObjectList);

            // Blank logger config
            encodeObjectList.Clear();
            encodeObjectList.Complete();

            innerMap.AddKeyAscii("ProgLogger_2", MapAction.ADD, encodeObjectList)
                .Complete();

            encodeGroupList.AddMap("LoggerList", innerMap)
                .Complete();

            outerMap.AddKeyAscii("LoggerGroup", MapAction.ADD, encodeGroupList);

            // Start encoding the Dictionary information
            encodeGroupList.Clear();
            innerMap.Clear();
            encodeObjectList.Clear();

            encodeObjectList.AddAscii("EnumTypeDefFileName", "ProgEnumFile")
                .AddAscii("EnumTypeDefItemName", "ProgEnumItem")
                .AddAscii("RdmFieldDictionaryFileName", "ProgFieldFile")
                .AddAscii("RdmFieldDictionaryItemName", "ProgFieldItem")
                .AddEnum("DictionaryType", DictionaryTypeEnum.FILE)
                .Complete();

            innerMap.AddKeyAscii("ProgDictionary_1", MapAction.ADD, encodeObjectList);

            encodeObjectList.Clear();
            encodeObjectList.Complete();

            innerMap.AddKeyAscii("ProgDictionary_2", MapAction.ADD, encodeObjectList)
                .Complete();

            encodeGroupList.AddMap("DictionaryList", innerMap)
                .Complete();

            outerMap.AddKeyAscii("DictionaryGroup", MapAction.ADD, encodeGroupList);

            outerMap.Complete();

            consConfigImpl.Config(outerMap);

            Assert.Equal(4, consConfigImpl.ConsumerConfigMap.Count);
            Assert.Equal(4, consConfigImpl.ClientChannelConfigMap.Count);
            Assert.Equal(4, consConfigImpl.LoggerConfigMap.Count);
            Assert.Equal(4, consConfigImpl.DictionaryConfigMap.Count);

            testConsConfig = consConfigImpl.ConsumerConfigMap["ProgConsumer_1"];

            Assert.Equal("ProgConsumer_1", testConsConfig.Name);
            Assert.Single(testConsConfig.ChannelSet);
            Assert.Equal("ProgChannel_1", testConsConfig.ChannelSet[0]);
            Assert.Equal("ProgLogger_1", testConsConfig.Logger);
            Assert.Equal("ProgDictionary_1", testConsConfig.Dictionary);
            Assert.Equal((long)2000, testConsConfig.DictionaryRequestTimeOut);
            Assert.Equal((long)2010, testConsConfig.DirectoryRequestTimeOut);
            Assert.Equal((long)2030, testConsConfig.LoginRequestTimeOut);
            Assert.Equal(2040, testConsConfig.DispatchTimeoutApiThread);
            Assert.True(testConsConfig.EnableRtt);
            Assert.Equal((ulong)2050, testConsConfig.ItemCountHint);
            Assert.Equal((int)2060, testConsConfig.MaxDispatchCountApiThread);
            Assert.Equal((int)2070, testConsConfig.MaxDispatchCountUserThread);
            Assert.Equal((ulong)2080, testConsConfig.MaxOutstandingPosts);
            Assert.False(testConsConfig.MsgKeyInUpdates);
            Assert.False(testConsConfig.ObeyOpenWindow);
            Assert.Equal((ulong)2090, testConsConfig.PostAckTimeout);
            Assert.Equal(2100, testConsConfig.ReconnectAttemptLimit);
            Assert.Equal(2120, testConsConfig.ReconnectMaxDelay);
            Assert.Equal(2110, testConsConfig.ReconnectMinDelay);
            Assert.Equal((ulong)2130, testConsConfig.RequestTimeout);
            Assert.True(testConsConfig.RestEnableLog);
            Assert.True(testConsConfig.RestEnableLogViaCallback);
            Assert.Equal("ProgRestLog", testConsConfig.RestLogFileName);
            Assert.Equal((ulong)2150, testConsConfig.RestRequestTimeOut);
            Assert.Equal((int)2140, testConsConfig.ServiceCountHint);
            // XML tracing block
            Assert.True(testConsConfig.XmlTraceToStdout);
            Assert.False(testConsConfig.XmlTraceToFile);
            Assert.Equal((ulong)10_000_000, testConsConfig.XmlTraceMaxFileSize);
            Assert.Equal("EmaTrace", testConsConfig.XmlTraceFileName);
            Assert.False(testConsConfig.XmlTraceToMultipleFiles);
            Assert.True(testConsConfig.XmlTraceWrite);
            Assert.True(testConsConfig.XmlTraceRead);
            Assert.False(testConsConfig.XmlTracePing);


            // ProgConsumer_2 has all defaults except for ChannelSet.
            testConsConfig = consConfigImpl.ConsumerConfigMap["ProgConsumer_2"];


            Assert.Equal("ProgConsumer_2", testConsConfig.Name);
            Assert.Equal(2, testConsConfig.ChannelSet.Count);
            Assert.Equal("ProgChannel_1", testConsConfig.ChannelSet[0]);
            Assert.Equal("ProgChannel_2", testConsConfig.ChannelSet[1]);
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
            // XML tracing block
            Assert.Equal(defaultConsConfig.XmlTraceToStdout, testConsConfig.XmlTraceToStdout);
            Assert.Equal(defaultConsConfig.XmlTraceToFile, testConsConfig.XmlTraceToFile);
            Assert.Equal(defaultConsConfig.XmlTraceMaxFileSize, testConsConfig.XmlTraceMaxFileSize);
            Assert.Equal(defaultConsConfig.XmlTraceFileName, testConsConfig.XmlTraceFileName);
            Assert.Equal(defaultConsConfig.XmlTraceToMultipleFiles, testConsConfig.XmlTraceToMultipleFiles);
            Assert.Equal(defaultConsConfig.XmlTraceWrite, testConsConfig.XmlTraceWrite);
            Assert.Equal(defaultConsConfig.XmlTraceRead, testConsConfig.XmlTraceRead);
            Assert.Equal(defaultConsConfig.XmlTracePing, testConsConfig.XmlTracePing);

            testChannelConfig = consConfigImpl.ClientChannelConfigMap["ProgChannel_1"];

            Assert.Equal("ProgChannel_1", testChannelConfig.Name);
            Assert.Equal(Eta.Transports.ConnectionType.ENCRYPTED, testChannelConfig.ConnectInfo.ConnectOptions.ConnectionType);
            Assert.Equal(Eta.Transports.ConnectionType.ENCRYPTED, testChannelConfig.ConnectInfo.ConnectOptions.EncryptionOpts.EncryptedProtocol);
            Assert.Equal(1, testChannelConfig.ConnectInfo.ConnectOptions.PingTimeout);
            Assert.True(testChannelConfig.ConnectInfo.EnableSessionManagement);
            Assert.Equal(1010, testChannelConfig.ConnectInfo.ConnectOptions.GuaranteedOutputBuffers);
            Assert.Equal(1020, testChannelConfig.HighWaterMark);
            Assert.Equal(1030, testChannelConfig.ConnectInfo.GetInitTimeout());
            Assert.Equal(15000, testChannelConfig.ConnectInfo.ConnectOptions.EncryptionOpts.AuthenticationTimeout);
            Assert.Equal("ProgInterface_1", testChannelConfig.ConnectInfo.ConnectOptions.UnifiedNetworkInfo.InterfaceName);
            Assert.Equal("ProgLocation_1", testChannelConfig.ConnectInfo.Location);
            Assert.Equal(1040, testChannelConfig.ConnectInfo.ConnectOptions.NumInputBuffers);
            Assert.Equal(1050, testChannelConfig.ConnectInfo.ServiceDiscoveryRetryCount);
            Assert.Equal(1060, testChannelConfig.ConnectInfo.ConnectOptions.SysRecvBufSize);
            Assert.Equal(1070, testChannelConfig.ConnectInfo.ConnectOptions.SysSendBufSize);
            Assert.Equal(Eta.Transports.CompressionType.LZ4, testChannelConfig.ConnectInfo.ConnectOptions.CompressionType);
            Assert.True(testChannelConfig.DirectWrite);
            Assert.Equal("ProgHost_1", testChannelConfig.ConnectInfo.ConnectOptions.UnifiedNetworkInfo.Address);
            Assert.Equal("ProgPort_1", testChannelConfig.ConnectInfo.ConnectOptions.UnifiedNetworkInfo.ServiceName);
            Assert.Equal("ProgProxyHost_1", testChannelConfig.ConnectInfo.ConnectOptions.ProxyOptions.ProxyHostName);
            Assert.Equal("ProgProxyPort_1", testChannelConfig.ConnectInfo.ConnectOptions.ProxyOptions.ProxyPort);
            Assert.True(testChannelConfig.ConnectInfo.ConnectOptions.TcpOpts.TcpNoDelay);
            Assert.Equal(EncryptionProtocolFlags.ENC_TLSV1_2,
                testChannelConfig.ConnectInfo.ConnectOptions.EncryptionOpts.EncryptionProtocolFlags &
                EncryptionProtocolFlags.ENC_TLSV1_2);
            Assert.True(testChannelConfig.CompressionThresholdSet);
            Assert.Equal(666, testChannelConfig.CompressionThreshold);

            // ProgChannel_2 is the default
            testChannelConfig = consConfigImpl.ClientChannelConfigMap["ProgChannel_2"];


            Assert.Equal("ProgChannel_2", testChannelConfig.Name);
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
            Assert.Equal(defaultChannelConfig.CompressionThresholdSet, testChannelConfig.CompressionThresholdSet);

            testLoggerConfig = consConfigImpl.LoggerConfigMap["ProgLogger_1"];

            Assert.Equal("ProgLogger_1", testLoggerConfig.Name);
            Assert.Equal(LoggerType.STDOUT, testLoggerConfig.LoggerType);
            Assert.Equal(LoggerLevel.INFO, testLoggerConfig.LoggerSeverity);
            Assert.Equal("ProgLogFile", testLoggerConfig.FileName);
            Assert.Equal((ulong)1, testLoggerConfig.IncludeDateInLoggerOutput);
            Assert.Equal((ulong)20, testLoggerConfig.NumberOfLogFiles);
            Assert.Equal((ulong)100, testLoggerConfig.MaxLogFileSize);

            // ProgLogger_2 is all defaults
            testLoggerConfig = consConfigImpl.LoggerConfigMap["ProgLogger_2"];

            Assert.Equal("ProgLogger_2", testLoggerConfig.Name);
            Assert.Equal(defaultLoggerConfig.LoggerType, testLoggerConfig.LoggerType);
            Assert.Equal(defaultLoggerConfig.LoggerSeverity, testLoggerConfig.LoggerSeverity);
            Assert.Equal(defaultLoggerConfig.FileName, testLoggerConfig.FileName);
            Assert.Equal(defaultLoggerConfig.IncludeDateInLoggerOutput, testLoggerConfig.IncludeDateInLoggerOutput);
            Assert.Equal(defaultLoggerConfig.NumberOfLogFiles, testLoggerConfig.NumberOfLogFiles);
            Assert.Equal(defaultLoggerConfig.MaxLogFileSize, testLoggerConfig.MaxLogFileSize);

            testDictConfig = consConfigImpl.DictionaryConfigMap["ProgDictionary_1"];

            Assert.Equal("ProgDictionary_1", testDictConfig.Name);
            Assert.Equal(EmaConfig.DictionaryTypeEnum.FILE, testDictConfig.DictionaryType);
            Assert.Equal("ProgEnumFile", testDictConfig.EnumTypeDefFileName);
            Assert.Equal("ProgEnumItem", testDictConfig.EnumTypeDefItemName);
            Assert.Equal("ProgFieldFile", testDictConfig.RdmFieldDictionaryFileName);
            Assert.Equal("ProgFieldItem", testDictConfig.RdmFieldDictionaryItemName);

            // ProgDictionary_1 is set to defaults
            testDictConfig = consConfigImpl.DictionaryConfigMap["ProgDictionary_2"];

            Assert.Equal("ProgDictionary_2", testDictConfig.Name);
            Assert.Equal(defaultDictConfig.DictionaryType, testDictConfig.DictionaryType);
            Assert.Equal(defaultDictConfig.EnumTypeDefFileName, testDictConfig.EnumTypeDefFileName);
            Assert.Equal(defaultDictConfig.EnumTypeDefItemName, testDictConfig.EnumTypeDefItemName);
            Assert.Equal(defaultDictConfig.RdmFieldDictionaryFileName, testDictConfig.RdmFieldDictionaryFileName);
            Assert.Equal(defaultDictConfig.RdmFieldDictionaryItemName, testDictConfig.RdmFieldDictionaryItemName);

            // Verify that the config works here
            consConfigImpl.VerifyConfiguration();
        }


        // This tests the programmatic overlay functionality.
        // The full test config is loaded, and all elements are overwritten by the programmatic config.
        [Fact]
        public void ConsumerProgrammaticOverlayTest()
        {
            OmmConsumerConfig consumerConfig;
            ConsumerConfig testConsConfig;
            ClientChannelConfig testChannelConfig;
            LoggerConfig testLoggerConfig;
            DictionaryConfig testDictConfig;

            // Top level map
            Map outerMap = new Map();
            // Middle map for consumers, channels, loggers, dictionaries
            Map innerMap = new Map();
            // Outer element list 
            ElementList encodeGroupList = new ElementList();
            ElementList encodeObjectList = new ElementList();

            // Load the fully set copnfig so we have a config to overlay information into
            // This doesn't use the default behavior(loading EmaConfig.xml) because an EmaConfig.xml file may be present in the directory for other example apps
            consumerConfig = LoadEmaTestConfig();

            OmmConsumerConfigImpl consConfigImpl = consumerConfig.OmmConsConfigImpl;

            // Loaded the Config, now make sure everything's in it.

            // There should be 2 Consumers, 2 ClientChannels, 1 Logger, and 1 Dictionary
            Assert.Equal(2, consConfigImpl.ConsumerConfigMap.Count);
            Assert.Equal(2, consConfigImpl.ClientChannelConfigMap.Count);
            Assert.Equal(2, consConfigImpl.LoggerConfigMap.Count);
            Assert.Equal(2, consConfigImpl.DictionaryConfigMap.Count);

            outerMap.Clear();

            innerMap.Clear();

            encodeGroupList.Clear();

            encodeObjectList.Clear();

            // Encode ProgrammaticConsumer_1
            //
            encodeObjectList.AddAscii("Channel", "ProgChannel_1")
                .AddAscii("Dictionary", "ProgDictionary_1")
                .AddAscii("Logger", "ProgLogger_1")
                .AddUInt("DictionaryRequestTimeOut", 2000)
                .AddUInt("DirectoryRequestTimeOut", 2010)
                .AddUInt("LoginRequestTimeOut", 2030)
                .AddInt("DispatchTimeoutApiThread", 2040)
                .AddUInt("EnableRtt", 0)
                .AddUInt("ItemCountHint", 2050)
                .AddUInt("MaxDispatchCountApiThread", 2060)
                .AddUInt("MaxDispatchCountUserThread", 2070)
                .AddUInt("MaxOutstandingPosts", 2080)
                .AddUInt("MsgKeyInUpdates", 0)
                .AddUInt("ObeyOpenWindow", 0)
                .AddUInt("PostAckTimeout", 2090)
                .AddInt("ReconnectAttemptLimit", 2100)
                .AddInt("ReconnectMaxDelay", 2120)
                .AddInt("ReconnectMinDelay", 2110)
                .AddUInt("RequestTimeout", 2130)
                .AddUInt("RestEnableLog", 0)
                .AddUInt("RestEnableLogViaCallback", 0)
                .AddAscii("RestLogFileName", "ProgRestLog")
                .AddUInt("ServiceCountHint", 2140)
                .AddUInt("RestRequestTimeOut", 2150)
                // XML trace params have values differing from defaults
                .AddUInt("XmlTraceToStdout", 0)
                .AddUInt("XmlTraceToFile", 1)
                .AddUInt("XmlTraceMaxFileSize", (ulong)5_000_000)
                .AddAscii("XmlTraceFileName", "NotEmaTrace")
                .AddUInt("XmlTraceToMultipleFiles", 1)
                .AddUInt("XmlTraceWrite", 0)
                .AddUInt("XmlTraceRead", 0)
                .AddUInt("XmlTracePing", 1)
                .Complete();

            innerMap.AddKeyAscii("TestConsumer_1", MapAction.ADD, encodeObjectList);

            encodeObjectList.Clear();

            encodeObjectList.AddAscii("ChannelSet", "ProgChannel_1, ProgChannel_2")
                .Complete();

            innerMap.AddKeyAscii("TestConsumer_2", MapAction.ADD, encodeObjectList);

            innerMap.Complete();

            encodeGroupList.Clear();

            encodeGroupList.AddAscii("DefaultConsumer", "TestConsumer_1")
                .AddMap("ConsumerList", innerMap)
                .Complete();

            outerMap.AddKeyAscii("ConsumerGroup", MapAction.ADD, encodeGroupList);


            // Start encoding the channel connection information
            encodeGroupList.Clear();
            innerMap.Clear();
            encodeObjectList.Clear();

            encodeObjectList.AddEnum("ChannelType", ConnectionTypeEnum.ENCRYPTED)
                .AddEnum("EncryptedProtocolType", ConnectionTypeEnum.ENCRYPTED)
                .AddUInt("ConnectionPingTimeout", 1000)
                .AddUInt("EnableSessionManagement", 0)
                .AddUInt("GuaranteedOutputBuffers", 1010)
                .AddUInt("HighWaterMark", 1020)
                .AddUInt("InitializationTimeout", 1030)
                .AddAscii("InterfaceName", "ProgInterface_1")
                .AddAscii("Location", "ProgLocation_1")
                .AddUInt("NumInputBuffers", 1040)
                .AddUInt("ServiceDiscoveryRetryCount", 1050)
                .AddUInt("SysRecvBufSize", 1060)
                .AddUInt("SysSendBufSize", 1070)
                .AddEnum("CompressionType", CompressionTypeEnum.LZ4)
                .AddUInt("DirectWrite", 1)
                .AddAscii("Host", "ProgHost_1")
                .AddAscii("Port", "ProgPort_1")
                .AddAscii("ProxyHost", "ProgProxyHost_1")
                .AddAscii("ProxyPort", "ProgProxyPort_1")
                .AddUInt("TcpNodelay", 0)
                .AddUInt("SecurityProtocol", EncryptedTLSProtocolFlags.TLSv1_2 | 0x01)
                .Complete();

            innerMap.AddKeyAscii("TestChannel_1", MapAction.ADD, encodeObjectList);


            // Second channel is all defaults, so add an empty Element List
            encodeObjectList.Clear();
            encodeObjectList.Complete();

            innerMap.AddKeyAscii("TestChannel_2", MapAction.ADD, encodeObjectList)
                .Complete();

            encodeGroupList.AddMap("ChannelList", innerMap)
                .Complete();

            outerMap.AddKeyAscii("ChannelGroup", MapAction.ADD, encodeGroupList);

            // Start encoding the Logger information
            encodeGroupList.Clear();
            innerMap.Clear();
            encodeObjectList.Clear();

            encodeObjectList.AddAscii("FileName", "ProgLogFile")
                .AddUInt("IncludeDateInLoggerOutput", 1)
                .AddUInt("NumberOfLogFiles", 20)
                .AddUInt("MaxLogFileSize", 100)
                .AddEnum("LoggerSeverity", LoggerLevelEnum.ERROR)
                .AddEnum("LoggerType", LoggerTypeEnum.FILE)
                .Complete();

            innerMap.AddKeyAscii("TestLogger_1", MapAction.ADD, encodeObjectList);

            // Blank logger config
            encodeObjectList.Clear();
            encodeObjectList.Complete();

            innerMap.AddKeyAscii("TestLogger_2", MapAction.ADD, encodeObjectList)
                .Complete();

            encodeGroupList.AddMap("LoggerList", innerMap)
                .Complete();

            outerMap.AddKeyAscii("LoggerGroup", MapAction.ADD, encodeGroupList);

            // Start encoding the Dictionary information
            encodeGroupList.Clear();
            innerMap.Clear();
            encodeObjectList.Clear();

            encodeObjectList.AddAscii("EnumTypeDefFileName", "ProgEnumFile")
                .AddAscii("EnumTypeDefItemName", "ProgEnumItem")
                .AddAscii("RdmFieldDictionaryFileName", "ProgFieldFile")
                .AddAscii("RdmFieldDictionaryItemName", "ProgFieldItem")
                .AddEnum("DictionaryType", DictionaryTypeEnum.CHANNEL)
                .Complete();

            innerMap.AddKeyAscii("TestDictionary_1", MapAction.ADD, encodeObjectList);

            encodeObjectList.Clear();
            encodeObjectList.Complete();

            innerMap.AddKeyAscii("TestDictionary_2", MapAction.ADD, encodeObjectList)
                .Complete();

            encodeGroupList.AddMap("DictionaryList", innerMap)
                .Complete();

            outerMap.AddKeyAscii("DictionaryGroup", MapAction.ADD, encodeGroupList);

            outerMap.Complete();

            consConfigImpl.Config(outerMap);


            Assert.Equal(3, consConfigImpl.ConsumerConfigMap.Count);
            Assert.Equal(2, consConfigImpl.ClientChannelConfigMap.Count);
            Assert.Equal(2, consConfigImpl.LoggerConfigMap.Count);
            Assert.Equal(2, consConfigImpl.DictionaryConfigMap.Count);

            testConsConfig = consConfigImpl.ConsumerConfigMap["TestConsumer_1"];

            Assert.Equal("TestConsumer_1", testConsConfig.Name);
            Assert.Single(testConsConfig.ChannelSet);
            Assert.Equal("ProgChannel_1", testConsConfig.ChannelSet[0]);
            Assert.Equal("ProgLogger_1", testConsConfig.Logger);
            Assert.Equal("ProgDictionary_1", testConsConfig.Dictionary);
            Assert.Equal((long)2000, testConsConfig.DictionaryRequestTimeOut);
            Assert.Equal((long)2010, testConsConfig.DirectoryRequestTimeOut);
            Assert.Equal((long)2030, testConsConfig.LoginRequestTimeOut);
            Assert.Equal(2040, testConsConfig.DispatchTimeoutApiThread);
            Assert.False(testConsConfig.EnableRtt);
            Assert.Equal((ulong)2050, testConsConfig.ItemCountHint);
            Assert.Equal((int)2060, testConsConfig.MaxDispatchCountApiThread);
            Assert.Equal((int)2070, testConsConfig.MaxDispatchCountUserThread);
            Assert.Equal((ulong)2080, testConsConfig.MaxOutstandingPosts);
            Assert.False(testConsConfig.MsgKeyInUpdates);
            Assert.False(testConsConfig.ObeyOpenWindow);
            Assert.Equal((ulong)2090, testConsConfig.PostAckTimeout);
            Assert.Equal(2100, testConsConfig.ReconnectAttemptLimit);
            Assert.Equal(2120, testConsConfig.ReconnectMaxDelay);
            Assert.Equal(2110, testConsConfig.ReconnectMinDelay);
            Assert.Equal((ulong)2130, testConsConfig.RequestTimeout);
            Assert.False(testConsConfig.RestEnableLog);
            Assert.False(testConsConfig.RestEnableLogViaCallback);
            Assert.Equal("ProgRestLog", testConsConfig.RestLogFileName);
            Assert.Equal((ulong)2150, testConsConfig.RestRequestTimeOut);
            Assert.Equal((int)2140, testConsConfig.ServiceCountHint);
            // XML tracing block
            Assert.False(testConsConfig.XmlTraceToStdout);
            Assert.True(testConsConfig.XmlTraceToFile);
            Assert.Equal((ulong)5_000_000, testConsConfig.XmlTraceMaxFileSize);
            Assert.Equal("NotEmaTrace", testConsConfig.XmlTraceFileName);
            Assert.True(testConsConfig.XmlTraceToMultipleFiles);
            Assert.False(testConsConfig.XmlTraceWrite);
            Assert.False(testConsConfig.XmlTraceRead);
            Assert.True(testConsConfig.XmlTracePing);


            // ProgConsumer_2 has all defaults except for ChannelSet.
            testConsConfig = consConfigImpl.ConsumerConfigMap["TestConsumer_2"];

            Assert.Equal("TestConsumer_2", testConsConfig.Name);
            Assert.Equal(2, testConsConfig.ChannelSet.Count);
            Assert.Equal("ProgChannel_1", testConsConfig.ChannelSet[0]);
            Assert.Equal("ProgChannel_2", testConsConfig.ChannelSet[1]);
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
            // XML tracing block
            Assert.Equal(defaultConsConfig.XmlTraceToStdout, testConsConfig.XmlTraceToStdout);
            Assert.Equal(defaultConsConfig.XmlTraceToFile, testConsConfig.XmlTraceToFile);
            Assert.Equal(defaultConsConfig.XmlTraceMaxFileSize, testConsConfig.XmlTraceMaxFileSize);
            Assert.Equal(defaultConsConfig.XmlTraceFileName, testConsConfig.XmlTraceFileName);
            Assert.Equal(defaultConsConfig.XmlTraceToMultipleFiles, testConsConfig.XmlTraceToMultipleFiles);
            Assert.Equal(defaultConsConfig.XmlTraceWrite, testConsConfig.XmlTraceWrite);
            Assert.Equal(defaultConsConfig.XmlTraceRead, testConsConfig.XmlTraceRead);
            Assert.Equal(defaultConsConfig.XmlTracePing, testConsConfig.XmlTracePing);


            testChannelConfig = consConfigImpl.ClientChannelConfigMap["TestChannel_1"];

            Assert.Equal("TestChannel_1", testChannelConfig.Name);
            Assert.Equal(Eta.Transports.ConnectionType.ENCRYPTED, testChannelConfig.ConnectInfo.ConnectOptions.ConnectionType);
            Assert.Equal(Eta.Transports.ConnectionType.ENCRYPTED, testChannelConfig.ConnectInfo.ConnectOptions.EncryptionOpts.EncryptedProtocol);
            Assert.Equal(1, testChannelConfig.ConnectInfo.ConnectOptions.PingTimeout);
            Assert.False(testChannelConfig.ConnectInfo.EnableSessionManagement);
            Assert.Equal(1010, testChannelConfig.ConnectInfo.ConnectOptions.GuaranteedOutputBuffers);
            Assert.Equal(1020, testChannelConfig.HighWaterMark);
            Assert.Equal(1030, testChannelConfig.ConnectInfo.GetInitTimeout());
            Assert.Equal("ProgInterface_1", testChannelConfig.ConnectInfo.ConnectOptions.UnifiedNetworkInfo.InterfaceName);
            Assert.Equal("ProgLocation_1", testChannelConfig.ConnectInfo.Location);
            Assert.Equal(1040, testChannelConfig.ConnectInfo.ConnectOptions.NumInputBuffers);
            Assert.Equal(1050, testChannelConfig.ConnectInfo.ServiceDiscoveryRetryCount);
            Assert.Equal(1060, testChannelConfig.ConnectInfo.ConnectOptions.SysRecvBufSize);
            Assert.Equal(1070, testChannelConfig.ConnectInfo.ConnectOptions.SysSendBufSize);
            Assert.Equal(Eta.Transports.CompressionType.LZ4, testChannelConfig.ConnectInfo.ConnectOptions.CompressionType);
            Assert.True(testChannelConfig.DirectWrite);
            Assert.Equal("ProgHost_1", testChannelConfig.ConnectInfo.ConnectOptions.UnifiedNetworkInfo.Address);
            Assert.Equal("ProgPort_1", testChannelConfig.ConnectInfo.ConnectOptions.UnifiedNetworkInfo.ServiceName);
            Assert.Equal("ProgProxyHost_1", testChannelConfig.ConnectInfo.ConnectOptions.ProxyOptions.ProxyHostName);
            Assert.Equal("ProgProxyPort_1", testChannelConfig.ConnectInfo.ConnectOptions.ProxyOptions.ProxyPort);
            Assert.False(testChannelConfig.ConnectInfo.ConnectOptions.TcpOpts.TcpNoDelay);
            Assert.Equal(EncryptionProtocolFlags.ENC_TLSV1_2,
                testChannelConfig.ConnectInfo.ConnectOptions.EncryptionOpts.EncryptionProtocolFlags &
                EncryptionProtocolFlags.ENC_TLSV1_2);

            // ProgChannel_2 is the default
            testChannelConfig = consConfigImpl.ClientChannelConfigMap["TestChannel_2"];


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

            testLoggerConfig = consConfigImpl.LoggerConfigMap["TestLogger_1"];

            Assert.Equal("TestLogger_1", testLoggerConfig.Name);
            Assert.Equal(LoggerType.FILE, testLoggerConfig.LoggerType);
            Assert.Equal(LoggerLevel.ERROR, testLoggerConfig.LoggerSeverity);
            Assert.Equal("ProgLogFile", testLoggerConfig.FileName);
            Assert.Equal((ulong)1, testLoggerConfig.IncludeDateInLoggerOutput);
            Assert.Equal((ulong)20, testLoggerConfig.NumberOfLogFiles);
            Assert.Equal((ulong)100, testLoggerConfig.MaxLogFileSize);

            // TestLogger_2 is all defaults
            testLoggerConfig = consConfigImpl.LoggerConfigMap["TestLogger_2"];

            Assert.Equal("TestLogger_2", testLoggerConfig.Name);
            Assert.Equal(defaultLoggerConfig.LoggerType, testLoggerConfig.LoggerType);
            Assert.Equal(defaultLoggerConfig.LoggerSeverity, testLoggerConfig.LoggerSeverity);
            Assert.Equal(defaultLoggerConfig.FileName, testLoggerConfig.FileName);
            Assert.Equal(defaultLoggerConfig.IncludeDateInLoggerOutput, testLoggerConfig.IncludeDateInLoggerOutput);
            Assert.Equal(defaultLoggerConfig.NumberOfLogFiles, testLoggerConfig.NumberOfLogFiles);
            Assert.Equal(defaultLoggerConfig.MaxLogFileSize, testLoggerConfig.MaxLogFileSize);

            testDictConfig = consConfigImpl.DictionaryConfigMap["TestDictionary_1"];

            Assert.Equal("TestDictionary_1", testDictConfig.Name);
            Assert.Equal(EmaConfig.DictionaryTypeEnum.CHANNEL, testDictConfig.DictionaryType);
            Assert.Equal("ProgEnumFile", testDictConfig.EnumTypeDefFileName);
            Assert.Equal("ProgEnumItem", testDictConfig.EnumTypeDefItemName);
            Assert.Equal("ProgFieldFile", testDictConfig.RdmFieldDictionaryFileName);
            Assert.Equal("ProgFieldItem", testDictConfig.RdmFieldDictionaryItemName);

            // ProgDictionary_1 is set to defaults
            testDictConfig = consConfigImpl.DictionaryConfigMap["TestDictionary_2"];

            Assert.Equal("TestDictionary_2", testDictConfig.Name);
            Assert.Equal(defaultDictConfig.DictionaryType, testDictConfig.DictionaryType);
            Assert.Equal(defaultDictConfig.EnumTypeDefFileName, testDictConfig.EnumTypeDefFileName);
            Assert.Equal(defaultDictConfig.EnumTypeDefItemName, testDictConfig.EnumTypeDefItemName);
            Assert.Equal(defaultDictConfig.RdmFieldDictionaryFileName, testDictConfig.RdmFieldDictionaryFileName);
            Assert.Equal(defaultDictConfig.RdmFieldDictionaryItemName, testDictConfig.RdmFieldDictionaryItemName);
        }


        // This test will load different failure scenarios for the verification method.
        // The positive test cases were already covered in the XmlConfigTest
        [Fact]
        public void ConsumerVerificationFailureTest()
        {
            OmmConsumerConfig consumerConfig;
            ConsumerConfig tmpConsumerConfig;

            ClientChannelConfig tmpChannelConfig_1;
            ClientChannelConfig tmpChannelConfig_2;
            ClientChannelConfig tmpChannelConfig_3;

            LoggerConfig tmpLoggerConfig_1;
            LoggerConfig tmpLoggerConfig_2;


            DictionaryConfig tmpDictionaryConfig_1;
            DictionaryConfig tmpDictionaryConfig_2;


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


            // Set a non-existant consumer name
            consConfigImpl.ConsumerName = "bad_consumer";

            // This should fail.
            Assert.Throws<OmmInvalidConfigurationException>(() => consConfigImpl.VerifyConfiguration());

            // Unset the ConsumerName and test the DefaultConsumer string
            consConfigImpl.ConsumerName = string.Empty;
            consConfigImpl.DefaultConsumer = "bad_consumer";

            // This should fail.
            Assert.Throws<OmmInvalidConfigurationException>(() => consConfigImpl.VerifyConfiguration());

            consConfigImpl.DefaultConsumer = string.Empty;

            tmpConsumerConfig = new ConsumerConfig();

            tmpConsumerConfig.Name = "consumer_1";
            tmpConsumerConfig.ChannelSet.Add("Bad_channel_1");

            consConfigImpl.ConsumerConfigMap.Add(tmpConsumerConfig.Name, tmpConsumerConfig);

            // This should fail.
            Assert.Throws<OmmInvalidConfigurationException>(() => consConfigImpl.VerifyConfiguration());

            // Add a couple of channels, make sure it still fails.
            tmpChannelConfig_1 = new ClientChannelConfig();
            tmpChannelConfig_1.Name = "ConfigChannel_1";
            consConfigImpl.ClientChannelConfigMap.Add(tmpChannelConfig_1.Name, tmpChannelConfig_1);

            tmpChannelConfig_2 = new ClientChannelConfig();
            tmpChannelConfig_2.Name = "ConfigChannel_2";
            consConfigImpl.ClientChannelConfigMap.Add(tmpChannelConfig_2.Name, tmpChannelConfig_2);

            tmpChannelConfig_3 = new ClientChannelConfig();
            tmpChannelConfig_3.Name = "ConfigChannel_3";
            consConfigImpl.ClientChannelConfigMap.Add(tmpChannelConfig_3.Name, tmpChannelConfig_3);

            // This should fail.
            Assert.Throws<OmmInvalidConfigurationException>(() => consConfigImpl.VerifyConfiguration());

            // Clear the channelSet, add 3 channels with the failure in the middle.
            tmpConsumerConfig.ChannelSet.Clear();
            tmpConsumerConfig.ChannelSet.Add("ConfigChannel_1");
            tmpConsumerConfig.ChannelSet.Add("bad_channel");
            tmpConsumerConfig.ChannelSet.Add("ConfigChannel_3");

            // This should fail.
            Assert.Throws<OmmInvalidConfigurationException>(() => consConfigImpl.VerifyConfiguration());

            // Clear the channelSet, add 3 channels with the failure at the end.
            tmpConsumerConfig.ChannelSet.Clear();
            tmpConsumerConfig.ChannelSet.Add("ConfigChannel_1");
            tmpConsumerConfig.ChannelSet.Add("ConfigChannel_2");
            tmpConsumerConfig.ChannelSet.Add("bad_channel");

            // This should fail.
            Assert.Throws<OmmInvalidConfigurationException>(() => consConfigImpl.VerifyConfiguration());

            // Clear the channelSet, set 3 good channels, make sure it succeeds
            tmpConsumerConfig.ChannelSet.Clear();
            tmpConsumerConfig.ChannelSet.Add("ConfigChannel_1");
            tmpConsumerConfig.ChannelSet.Add("ConfigChannel_2");
            tmpConsumerConfig.ChannelSet.Add("ConfigChannel_3");

            consConfigImpl.VerifyConfiguration();

            // Add a logger mismatch

            tmpConsumerConfig.Logger = "bad_logger";

            // This should fail.
            Assert.Throws<OmmInvalidConfigurationException>(() => consConfigImpl.VerifyConfiguration());

            // Add a couple of loggers and make sure it's still a mismatch

            tmpLoggerConfig_1 = new LoggerConfig();
            tmpLoggerConfig_1.Name = "ConfigLogger_1";
            consConfigImpl.LoggerConfigMap.Add(tmpLoggerConfig_1.Name, tmpLoggerConfig_1);

            tmpLoggerConfig_2 = new LoggerConfig();
            tmpLoggerConfig_2.Name = "ConfigLogger_2";
            consConfigImpl.LoggerConfigMap.Add(tmpLoggerConfig_2.Name, tmpLoggerConfig_2);

            tmpConsumerConfig.Logger = "bad_logger";

            // This should fail.
            Assert.Throws<OmmInvalidConfigurationException>(() => consConfigImpl.VerifyConfiguration());

            // Set the logger correctly and make sure it verifies
            tmpConsumerConfig.Logger = "ConfigLogger_2";

            // This should fail.
            consConfigImpl.VerifyConfiguration();

            // Set the dictionary to incorrect value
            tmpConsumerConfig.Dictionary = "bad_dictionary";

            // This should fail.
            Assert.Throws<OmmInvalidConfigurationException>(() => consConfigImpl.VerifyConfiguration());

            // Add in the dictionaries and make sure it still fails.
            tmpDictionaryConfig_1 = new DictionaryConfig();
            tmpDictionaryConfig_1.Name = "ConfigDictionary_1";
            consConfigImpl.DictionaryConfigMap.Add(tmpDictionaryConfig_1.Name, tmpDictionaryConfig_1);

            tmpDictionaryConfig_2 = new DictionaryConfig();
            tmpDictionaryConfig_2.Name = "ConfigDictionary_2";
            consConfigImpl.DictionaryConfigMap.Add(tmpDictionaryConfig_2.Name, tmpDictionaryConfig_2);

            // This should fail.
            Assert.Throws<OmmInvalidConfigurationException>(() => consConfigImpl.VerifyConfiguration());
        }


        // Tests the addadminmsg functionality.
        [Fact]
        public void ConsumerAddAdminMsgTest()
        {
            OmmConsumerConfig consumerConfig;
            OmmConsumerConfigImpl copiedConfig;
            ConsumerRole testRole;
            ConfigError? error;

            consumerConfig = new OmmConsumerConfig("../../../OmmConfigTests/EmaTestConfig.xml");

            OmmConsumerConfigImpl consConfigImpl = consumerConfig.OmmConsConfigImpl;

            // Generate a consumer role to ensuure that everything that should be null is null
            // Copy the config
            copiedConfig = new OmmConsumerConfigImpl(consumerConfig.OmmConsConfigImpl);

            testRole = copiedConfig.GenerateConsumerRole();

            Assert.NotNull(testRole.RdmLoginRequest);
            Assert.Null(testRole.RdmDirectoryRequest);
            Assert.Null(testRole.RdmEnumTypeDictionaryRequest);
            Assert.Null(testRole.RdmFieldDictionaryRequest);


            RequestMsg reqMsg = new RequestMsg();
            reqMsg.Name("TestName").DomainType((int)DomainType.LOGIN).Attrib(
                new ElementList().AddAscii("ApplicationId", "100")
                    .AddAscii("Position", "TestPosition")
                    .AddAscii("Password", "TestPassword")
                    .AddAscii("ApplicationName", "TestEMAApp")
                    .Complete()
            ).EncodeComplete();

            consumerConfig.AddAdminMsg(reqMsg);

            Assert.Equal("TestName", consConfigImpl.AdminLoginRequest.UserName.ToString());

            Assert.True(consConfigImpl.AdminLoginRequest.HasAttrib);

            Assert.True(consConfigImpl.AdminLoginRequest.LoginAttrib.HasApplicationId);
            Assert.Equal("100", consConfigImpl.AdminLoginRequest.LoginAttrib.ApplicationId.ToString());

            Assert.True(consConfigImpl.AdminLoginRequest.LoginAttrib.HasApplicationName);
            Assert.Equal("TestEMAApp", consConfigImpl.AdminLoginRequest.LoginAttrib.ApplicationName.ToString());

            Assert.True(consConfigImpl.AdminLoginRequest.LoginAttrib.HasPosition);
            Assert.Equal("TestPosition", consConfigImpl.AdminLoginRequest.LoginAttrib.Position.ToString());

            Assert.True(consConfigImpl.AdminLoginRequest.HasPassword);
            Assert.Equal("TestPassword", consConfigImpl.AdminLoginRequest.Password.ToString());

            // There should be no logged errors in the log
            Assert.Equal(0, consConfigImpl.ConfigErrorLog?.Count());

            reqMsg.Clear();

            // Test a Source directory request without a filter and without a streaming request.
            // This should succeed, with the resulting directory request having a filter with everything, no serviceId specified, and with Streaming unset.
            // There should also be two logged warnings in the ConfigErrorLog
            reqMsg.DomainType((int)DomainType.SOURCE).InterestAfterRefresh(false).EncodeComplete();

            consumerConfig.AddAdminMsg(reqMsg);

            Assert.NotNull(consConfigImpl.AdminDirectoryRequest);

            Assert.Equal(ServiceFilterFlags.ALL_FILTERS, consConfigImpl.AdminDirectoryRequest?.Filter);

            Assert.False(consConfigImpl.AdminDirectoryRequest?.HasServiceId);

            // There should be two logged error in the log, both with a WARNING error level
            Assert.Equal(2, consConfigImpl.ConfigErrorLog?.Count());


            for (int i = 0; i < consConfigImpl.ConfigErrorLog?.Count(); ++i)
            {
                error = consConfigImpl.ConfigErrorLog?.ErrorList[i];
                Assert.NotNull(error);
                Assert.Equal(LoggerLevel.WARNING, error?.Level);
            }

            // Clear the error log for next failure tests
            consConfigImpl.ConfigErrorLog?.Clear();

            // Set a directory request with a filter and a serviceId
            reqMsg.Clear();
            reqMsg.DomainType((int)DomainType.SOURCE).Filter((int)ServiceFilterFlags.DATA).ServiceId(150).EncodeComplete();

            consumerConfig.AddAdminMsg(reqMsg);

            Assert.NotNull(consConfigImpl.AdminDirectoryRequest);

            Assert.Equal(ServiceFilterFlags.DATA, consConfigImpl.AdminDirectoryRequest?.Filter);

            Assert.True(consConfigImpl.AdminDirectoryRequest?.HasServiceId);

            Assert.Equal(150, consConfigImpl.AdminDirectoryRequest?.ServiceId);

            // There should be no logged errors in the log
            Assert.Equal(0, consConfigImpl.ConfigErrorLog?.Count());

            // Test Dictionary
            // No set name, so error in log
            reqMsg.Clear();
            reqMsg.DomainType((int)DomainType.DICTIONARY).EncodeComplete();

            consumerConfig.AddAdminMsg(reqMsg);

            Assert.Null(consConfigImpl.AdminEnumDictionaryRequest);
            Assert.Null(consConfigImpl.AdminFieldDictionaryRequest);

            // There should be no logged errors in the log
            Assert.Equal(1, consConfigImpl.ConfigErrorLog?.Count());

            // Clear the error log for next failure tests
            consConfigImpl.ConfigErrorLog?.Clear();

            // set name, no service name or service id, so error in log
            reqMsg.Clear();
            reqMsg.Name("BadDictName").DomainType((int)DomainType.DICTIONARY).EncodeComplete();

            consumerConfig.AddAdminMsg(reqMsg);

            Assert.Null(consConfigImpl.AdminEnumDictionaryRequest);
            Assert.Null(consConfigImpl.AdminFieldDictionaryRequest);

            // There should be no logged errors in the log
            Assert.Equal(1, consConfigImpl.ConfigErrorLog?.Count());

            // Clear the error log for next failure tests
            consConfigImpl.ConfigErrorLog?.Clear();

            // set name, set service id, no filter, so error in log
            reqMsg.Clear();
            reqMsg.Name("BadDictName").ServiceId(10).DomainType((int)DomainType.DICTIONARY).EncodeComplete();

            consumerConfig.AddAdminMsg(reqMsg);

            Assert.Null(consConfigImpl.AdminEnumDictionaryRequest);
            Assert.Null(consConfigImpl.AdminFieldDictionaryRequest);

            // There should be no logged errors in the log
            Assert.Equal(1, consConfigImpl.ConfigErrorLog?.Count());

            // Clear the error log for next failure tests
            consConfigImpl.ConfigErrorLog?.Clear();

            // set name, set service id, filter, set no_refresh so error in log
            reqMsg.Clear();
            reqMsg.Name("BadDictName").ServiceId(10).Filter(Dictionary.VerbosityValues.VERBOSE).InitialImage(false).DomainType((int)DomainType.DICTIONARY).EncodeComplete();

            consumerConfig.AddAdminMsg(reqMsg);

            Assert.Null(consConfigImpl.AdminEnumDictionaryRequest);
            Assert.Null(consConfigImpl.AdminFieldDictionaryRequest);

            // There should be no logged errors in the log
            Assert.Equal(1, consConfigImpl.ConfigErrorLog?.Count());

            // Clear the error log for next failure tests
            consConfigImpl.ConfigErrorLog?.Clear();

            // set name, set service id, filter, set no_refresh so error in log
            reqMsg.Clear();
            reqMsg.Name("BadDictName").ServiceId(10).Filter(Dictionary.VerbosityValues.VERBOSE).InitialImage(false).DomainType((int)DomainType.DICTIONARY).EncodeComplete();

            consumerConfig.AddAdminMsg(reqMsg);

            Assert.Null(consConfigImpl.AdminEnumDictionaryRequest);
            Assert.Null(consConfigImpl.AdminFieldDictionaryRequest);

            // There should be no logged errors in the log
            Assert.Equal(1, consConfigImpl.ConfigErrorLog?.Count());

            // Clear the error log for next failure tests
            consConfigImpl.ConfigErrorLog?.Clear();

            // bad dictionary name, set service id, filter, set no_refresh so error in log
            reqMsg.Clear();
            reqMsg.Name("BadDictName").ServiceId(10).Filter(Dictionary.VerbosityValues.VERBOSE).DomainType((int)DomainType.DICTIONARY).EncodeComplete();

            consumerConfig.AddAdminMsg(reqMsg);

            Assert.Null(consConfigImpl.AdminEnumDictionaryRequest);
            Assert.Null(consConfigImpl.AdminFieldDictionaryRequest);

            // There should be no logged errors in the log
            Assert.Equal(1, consConfigImpl.ConfigErrorLog?.Count());

            // Clear the error log for next failure tests
            consConfigImpl.ConfigErrorLog?.Clear();

            // correct dictionary name, set service id, filter, set no_refresh so error in log
            reqMsg.Clear();
            reqMsg.Name("RWFFld").ServiceId(10).Filter(Dictionary.VerbosityValues.VERBOSE).DomainType((int)DomainType.DICTIONARY).EncodeComplete();

            consumerConfig.AddAdminMsg(reqMsg);


            Assert.Null(consConfigImpl.AdminEnumDictionaryRequest);
            Assert.NotNull(consConfigImpl.AdminFieldDictionaryRequest);

            Assert.Equal("RWFFld", consConfigImpl.AdminFieldDictionaryRequest?.DictionaryName.ToString());
            Assert.Equal(Dictionary.VerbosityValues.VERBOSE, consConfigImpl.AdminFieldDictionaryRequest?.Verbosity);
            Assert.Equal(10, consConfigImpl.AdminFieldDictionaryRequest?.ServiceId);


            // There should be no logged errors in the log
            Assert.Equal(0, consConfigImpl.ConfigErrorLog?.Count());

            // Clear the error log for next failure tests
            consConfigImpl.ConfigErrorLog?.Clear();

            // Set field dictionary request to null.
            consConfigImpl.AdminFieldDictionaryRequest = null;

            // correct enum dictionary name, set service id, filter, set no_refresh so error in log
            reqMsg.Clear();
            reqMsg.Name("RWFEnum").ServiceId(10).Filter(Dictionary.VerbosityValues.VERBOSE).DomainType((int)DomainType.DICTIONARY).EncodeComplete();

            consumerConfig.AddAdminMsg(reqMsg);

            Assert.NotNull(consConfigImpl.AdminEnumDictionaryRequest);
            Assert.Null(consConfigImpl.AdminFieldDictionaryRequest);

            Assert.Equal("RWFEnum", consConfigImpl.AdminEnumDictionaryRequest?.DictionaryName.ToString());
            Assert.Equal(Dictionary.VerbosityValues.VERBOSE, consConfigImpl.AdminEnumDictionaryRequest?.Verbosity);
            Assert.Equal(10, consConfigImpl.AdminEnumDictionaryRequest?.ServiceId);


            // There should be no logged errors in the log
            Assert.Equal(0, consConfigImpl.ConfigErrorLog?.Count());

            // Clear the error log for next failure tests
            consConfigImpl.ConfigErrorLog?.Clear();

            // Add the RWFFld request back in.
            reqMsg.Clear();
            reqMsg.Name("RWFFld").ServiceId(10).Filter(Dictionary.VerbosityValues.VERBOSE).DomainType((int)DomainType.DICTIONARY).EncodeComplete();

            consumerConfig.AddAdminMsg(reqMsg);

            Assert.NotNull(consConfigImpl.AdminEnumDictionaryRequest);
            Assert.Equal("RWFEnum", consConfigImpl.AdminEnumDictionaryRequest?.DictionaryName.ToString());
            Assert.Equal(Dictionary.VerbosityValues.VERBOSE, consConfigImpl.AdminEnumDictionaryRequest?.Verbosity);
            Assert.Equal(10, consConfigImpl.AdminEnumDictionaryRequest?.ServiceId);

            Assert.NotNull(consConfigImpl.AdminFieldDictionaryRequest);
            Assert.Equal("RWFFld", consConfigImpl.AdminFieldDictionaryRequest?.DictionaryName.ToString());
            Assert.Equal(Dictionary.VerbosityValues.VERBOSE, consConfigImpl.AdminFieldDictionaryRequest?.Verbosity);
            Assert.Equal(10, consConfigImpl.AdminFieldDictionaryRequest?.ServiceId);


            // Copy the config
            copiedConfig = new OmmConsumerConfigImpl(consumerConfig.OmmConsConfigImpl);

            testRole = copiedConfig.GenerateConsumerRole();

            Assert.NotNull(testRole.RdmLoginRequest);
            Assert.Equal("TestName", testRole.RdmLoginRequest?.UserName.ToString());
            Assert.True(testRole.RdmLoginRequest?.HasAttrib);
            Assert.True(testRole.RdmLoginRequest?.LoginAttrib.HasApplicationId);
            Assert.Equal("100", testRole.RdmLoginRequest?.LoginAttrib.ApplicationId.ToString());
            Assert.True(consConfigImpl.AdminLoginRequest.LoginAttrib.HasApplicationName);
            Assert.Equal("TestEMAApp", testRole.RdmLoginRequest?.LoginAttrib.ApplicationName.ToString());
            Assert.True(consConfigImpl.AdminLoginRequest.LoginAttrib.HasPosition);
            Assert.Equal("TestPosition", testRole.RdmLoginRequest?.LoginAttrib.Position.ToString());
            Assert.True(consConfigImpl.AdminLoginRequest.HasPassword);
            Assert.Equal("TestPassword", testRole.RdmLoginRequest?.Password.ToString());

            Assert.NotNull(testRole.RdmDirectoryRequest);
            Assert.Equal(ServiceFilterFlags.DATA, testRole.RdmDirectoryRequest?.Filter);
            Assert.True(testRole.RdmDirectoryRequest?.HasServiceId);
            Assert.Equal(150, testRole.RdmDirectoryRequest?.ServiceId);

            Assert.NotNull(testRole.RdmEnumTypeDictionaryRequest);
            Assert.Equal("RWFEnum", testRole.RdmEnumTypeDictionaryRequest?.DictionaryName.ToString());
            Assert.Equal(Dictionary.VerbosityValues.VERBOSE, testRole.RdmEnumTypeDictionaryRequest?.Verbosity);
            Assert.Equal(10, testRole.RdmEnumTypeDictionaryRequest?.ServiceId);

            Assert.NotNull(testRole.RdmFieldDictionaryRequest);
            Assert.Equal("RWFFld", testRole.RdmFieldDictionaryRequest?.DictionaryName.ToString());
            Assert.Equal(Dictionary.VerbosityValues.VERBOSE, testRole.RdmFieldDictionaryRequest?.Verbosity);
            Assert.Equal(10, testRole.RdmFieldDictionaryRequest?.ServiceId);

            // Now set all of the method-based options to something different and verify that they get set in the login.
            consumerConfig.UserName("MethodUser");
            consumerConfig.ApplicationId("MethodId");
            consumerConfig.Position("MethodPosition");
            consumerConfig.Password("MethodPassword");

            copiedConfig = new OmmConsumerConfigImpl(consumerConfig.OmmConsConfigImpl);

            testRole = copiedConfig.GenerateConsumerRole();

            Assert.NotNull(testRole.RdmLoginRequest);
            Assert.Equal("MethodUser", testRole.RdmLoginRequest?.UserName.ToString());
            Assert.True(testRole.RdmLoginRequest?.HasAttrib);
            Assert.True(testRole.RdmLoginRequest?.LoginAttrib.HasApplicationId);
            Assert.Equal("MethodId", testRole.RdmLoginRequest?.LoginAttrib.ApplicationId.ToString());
            Assert.True(consConfigImpl.AdminLoginRequest.LoginAttrib.HasApplicationName);
            Assert.Equal("TestEMAApp", testRole.RdmLoginRequest?.LoginAttrib.ApplicationName.ToString());
            Assert.True(consConfigImpl.AdminLoginRequest.LoginAttrib.HasPosition);
            Assert.Equal("MethodPosition", testRole.RdmLoginRequest?.LoginAttrib.Position.ToString());
            Assert.True(consConfigImpl.AdminLoginRequest.HasPassword);
            Assert.Equal("MethodPassword", testRole.RdmLoginRequest?.Password.ToString());

            Assert.NotNull(testRole.RdmDirectoryRequest);
            Assert.Equal(ServiceFilterFlags.DATA, testRole.RdmDirectoryRequest?.Filter);
            Assert.True(testRole.RdmDirectoryRequest?.HasServiceId);
            Assert.Equal(150, testRole.RdmDirectoryRequest?.ServiceId);

            Assert.NotNull(testRole.RdmEnumTypeDictionaryRequest);
            Assert.Equal("RWFEnum", testRole.RdmEnumTypeDictionaryRequest?.DictionaryName.ToString());
            Assert.Equal(Dictionary.VerbosityValues.VERBOSE, testRole.RdmEnumTypeDictionaryRequest?.Verbosity);
            Assert.Equal(10, testRole.RdmEnumTypeDictionaryRequest?.ServiceId);

            Assert.NotNull(testRole.RdmFieldDictionaryRequest);
            Assert.Equal("RWFFld", testRole.RdmFieldDictionaryRequest?.DictionaryName.ToString());
            Assert.Equal(Dictionary.VerbosityValues.VERBOSE, testRole.RdmFieldDictionaryRequest?.Verbosity);
            Assert.Equal(10, testRole.RdmFieldDictionaryRequest?.ServiceId);

        }
    }
}
