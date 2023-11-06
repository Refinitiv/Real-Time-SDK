/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2023 Refinitiv. All rights reserved.              --
 *|-----------------------------------------------------------------------------
 */

using System;
using System.Collections.Generic;
using System.Security.Cryptography;
using LSEG.Eta.Codec;
using LSEG.Eta.Common;
using LSEG.Eta.Rdm;
using LSEG.Eta.Transports;
using LSEG.Eta.ValueAdd.Reactor;
using NLog.LayoutRenderers;
using static LSEG.Ema.Access.DictionaryConfig;
using static LSEG.Ema.Access.EmaConfig;
using static LSEG.Eta.Rdm.Directory;

namespace LSEG.Ema.Access.Tests.OmmConsumerConfigTests;


public class ConsumerConfigTests
{
    // Default objects to compare to in the below tests.
    private static ConsumerConfig defaultConsConfig = new ConsumerConfig();
    private static ClientChannelConfig defaultChannelConfig = new ClientChannelConfig();
    private static LoggerConfig defaultLoggerConfig = new LoggerConfig();
    private static DictionaryConfig defaultDictConfig = new DictionaryConfig();

    // Default Xml File load test
    // Creates EmaConfig.xml in the current working directory, and attempts to load it.

    // Verifies behavior for all of the enumeration string methods in:
    // DictionaryConfig
    // LoggerConfig
    // ClientChanelConfig
    [Fact]
    public void ConfigStringToEnumTest()
    {
        string testString = string.Empty;

        // ClientChannelConfig connection type and compression type
        ConnectionType outputConnType;
        try
        {
            testString = "RSSL_SOCKET";
            outputConnType = ClientChannelConfig.StringToConnectionType(testString);
            Assert.Equal(ConnectionType.SOCKET, outputConnType);

            testString = "RSSL_ENCRYPTED";
            outputConnType = ClientChannelConfig.StringToConnectionType(testString);
            Assert.Equal(ConnectionType.ENCRYPTED, outputConnType);

            // Check that lower case works
            testString = "rssl_socket";
            outputConnType = ClientChannelConfig.StringToConnectionType(testString);
            Assert.Equal(ConnectionType.SOCKET, outputConnType);

            testString = "rssl_encrypted";
            outputConnType = ClientChannelConfig.StringToConnectionType(testString);
            Assert.Equal(ConnectionType.ENCRYPTED, outputConnType);
        }
        catch(OmmInvalidConfigurationException)
        {
            // Fail here
            Assert.False(true);
        }

        try
        {
            // Failure case with bad input string
            testString = "bad_conn_type";
            outputConnType = ClientChannelConfig.StringToConnectionType(testString);
            // Fail here
            Assert.False(true);
        }
        catch (OmmInvalidConfigurationException)
        {
            // Received OmmInvalidConfigurationException, so pass here.
            Assert.True(true);
        }

        CompressionType outputCompType;
        try
        {
            testString = "None";
            outputCompType = ClientChannelConfig.StringToCompressionType(testString);
            Assert.Equal(CompressionType.NONE, outputCompType);

            testString = "ZLib";
            outputCompType = ClientChannelConfig.StringToCompressionType(testString);
            Assert.Equal(CompressionType.ZLIB, outputCompType);

            testString = "LZ4";
            outputCompType = ClientChannelConfig.StringToCompressionType(testString);
            Assert.Equal(CompressionType.LZ4, outputCompType);
        }
        catch (OmmInvalidConfigurationException)
        {
            // Fail here
            Assert.False(true);
        }

        try
        {
            // Failure case with bad input string
            testString = "bad_comp_type";
            outputCompType = ClientChannelConfig.StringToCompressionType(testString);
            // Fail here
            Assert.False(true);
        }
        catch (OmmInvalidConfigurationException)
        {
            // Received OmmInvalidConfigurationException, so pass here.
            Assert.True(true);
        }

        // LoggerConfig for LoggerLevel and LoggerType
        LoggerLevel outputLoggerLevel;
        try
        {
            testString = "Trace";
            outputLoggerLevel = LoggerConfig.StringToLoggerLevel(testString);
            Assert.Equal(LoggerLevel.TRACE, outputLoggerLevel);

            testString = "Verbose";
            outputLoggerLevel = LoggerConfig.StringToLoggerLevel(testString);
            Assert.Equal(LoggerLevel.TRACE, outputLoggerLevel);

            testString = "Info";
            outputLoggerLevel = LoggerConfig.StringToLoggerLevel(testString);
            Assert.Equal(LoggerLevel.INFO, outputLoggerLevel);

            testString = "Success";
            outputLoggerLevel = LoggerConfig.StringToLoggerLevel(testString);
            Assert.Equal(LoggerLevel.INFO, outputLoggerLevel);

            testString = "Warning";
            outputLoggerLevel = LoggerConfig.StringToLoggerLevel(testString);
            Assert.Equal(LoggerLevel.WARNING, outputLoggerLevel);

            testString = "Error";
            outputLoggerLevel = LoggerConfig.StringToLoggerLevel(testString);
            Assert.Equal(LoggerLevel.ERROR, outputLoggerLevel);

            testString = "NoLogMsg";
            outputLoggerLevel = LoggerConfig.StringToLoggerLevel(testString);
            Assert.Equal(LoggerLevel.OFF, outputLoggerLevel);
        }
        catch (OmmInvalidConfigurationException)
        {
            // Fail here
            Assert.False(true);
        }

        bool catchException = false;
        try
        {
            testString = "Debug";
            outputLoggerLevel = LoggerConfig.StringToLoggerLevel(testString);
            Assert.Equal(LoggerLevel.DEBUG, outputLoggerLevel);
        }
        catch (OmmInvalidConfigurationException exp)
        {
            catchException = true;
            Assert.Equal("Logger Severity: Debug not recognized. Acceptable inputs: \"Trace\"," +
                " \"Info\" or \"Success\", \"Warning\", \"Error\" or \"Verbose\", \"NoLogMsg\".", exp.Message);
        }
        Assert.True(catchException);

        try
        {
            // Failure case with bad input string
            testString = "bad_log_level";
            outputLoggerLevel = LoggerConfig.StringToLoggerLevel(testString);
            // Fail here
            Assert.False(true);
        }
        catch (OmmInvalidConfigurationException)
        {
            // Received OmmInvalidConfigurationException, so pass here.
            Assert.True(true);
        }

        LoggerType outputLoggerType;
        try
        {
            testString = "File";
            outputLoggerType = LoggerConfig.StringToLoggerType(testString);
            Assert.Equal(LoggerType.FILE, outputLoggerType);

            testString = "Stdout";
            outputLoggerType = LoggerConfig.StringToLoggerType(testString);
            Assert.Equal(LoggerType.STDOUT, outputLoggerType);
        }
        catch (OmmInvalidConfigurationException)
        {
            // Fail here
            Assert.False(true);
        }

        try
        {
            // Failure case with bad input string
            testString = "bad_log_type";
            outputLoggerType = LoggerConfig.StringToLoggerType(testString);
            // Fail here
            Assert.False(true);
        }
        catch (OmmInvalidConfigurationException)
        {
            // Received OmmInvalidConfigurationException, so pass here.
            Assert.True(true);
        }

        // DictionaryConfig DictionaryType
        int outputDictionaryMode;
        try
        {
            testString = "FileDictionary";
            outputDictionaryMode = DictionaryConfig.StringToDictionaryMode(testString);
            Assert.Equal(EmaConfig.DictionaryTypeEnum.FILE, outputDictionaryMode);

            testString = "ChannelDictionary";
            outputDictionaryMode = DictionaryConfig.StringToDictionaryMode(testString);
            Assert.Equal(EmaConfig.DictionaryTypeEnum.CHANNEL, outputDictionaryMode);
        }
        catch (OmmInvalidConfigurationException)
        {
            // Fail here
            Assert.False(true);
        }

        try
        {
            // Failure case with bad input string
            testString = "bad_dictionary_type";
            outputDictionaryMode = DictionaryConfig.StringToDictionaryMode(testString);
            // Fail here
            Assert.False(true);
        }
        catch (OmmInvalidConfigurationException)
        {
            // Received OmmInvalidConfigurationException, so pass here.
            Assert.True(true);
        }
    }

    // OmmConsumerConfig.Host() test
    // Tests the Host string parsing with the following scenarios:
    // ""(blank) => Default Hostname(localhost) and default port(14002)
    // [Hostname] => Configured Hostname and default port
    // [Hostname]: => Configured Hostname and default port
    // :[Port] => Default Hostname and Port
    // [Hostname]:[Port] => Configured Hostname and Configured Port
    [Fact]
    public void HostMethodTest()
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

    // Xml Config loading and parsing test
    // This loads a config that contains all elements used in the OmmConsumer config, tests the external setter and getter methods,
    // and verifies that the Reactor ConnectInfo and Reactor Role generation methods work, ensuring that all config members are correctly set
    // in the Role and ConnectInfo objects.
    [Fact]
    public void XmlConfigTest()
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

        try
        {
            consumerConfig = new OmmConsumerConfig("../../../OmmConsumerConfigTests/EmaTestConfig.xml");
        }
        catch(OmmInvalidConfigurationException)
        {
            Assert.False(true);
            return;
        }

        OmmConsumerConfigImpl consConfigImpl = consumerConfig.OmmConsConfigImpl;

        // Check to make sure that there aren't any elements in the ConfigErrorLog
        Assert.Equal(0,consConfigImpl.ConfigErrorLog!.Count());

        // Loaded the Config, now make sure everything's in it.

        // There should be 2 Consumers, 2 ClientChannels, 1 Logger, and 1 Dictionary
        Assert.Equal(2, consConfigImpl.ConsumerConfigMap.Count);
        Assert.Equal(2, consConfigImpl.ClientChannelConfigMap.Count);
        Assert.Equal(2, consConfigImpl.LoggerConfigMap.Count);
        Assert.Equal(2, consConfigImpl.DictionaryConfigMap.Count);

        try
        {
            testConsConfig = consConfigImpl.ConsumerConfigMap["TestConsumer"];
        }
        catch
        {
            Assert.False(true);
            return;
        }

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


        // TestConsumer_2 has all defaults except for ChannelSet.

        try
        {
            testConsConfig = consConfigImpl.ConsumerConfigMap["TestConsumer_2"];
        }
        catch
        {
            Assert.False(true);
            return;
        }

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

        try
        {
            testChannelConfig = consConfigImpl.ClientChannelConfigMap["TestChannel_1"];
        }
        catch
        {
            Assert.False(true);
            return;
        }

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

        // TestChannel_2 is the default
        try
        {
            testChannelConfig = consConfigImpl.ClientChannelConfigMap["TestChannel_2"];
        }
        catch
        {
            Assert.False(true);
            return;
        }

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

        try
        {
            testLoggerConfig = consConfigImpl.LoggerConfigMap["TestLogger_1"];
        }
        catch
        {
            Assert.False(true);
            return;
        }

        Assert.Equal("TestLogger_1", testLoggerConfig.Name);
        Assert.Equal(LoggerType.STDOUT, testLoggerConfig.LoggerType);
        Assert.Equal(LoggerLevel.INFO, testLoggerConfig.LoggerSeverity);
        Assert.Equal("testLogFile1", testLoggerConfig.FileName);
        Assert.Equal((ulong)1, testLoggerConfig.IncludeDateInLoggerOutput);
        Assert.Equal((ulong)10, testLoggerConfig.NumberOfLogFiles);
        Assert.Equal((ulong)20, testLoggerConfig.MaxLogFileSize);

        // TestLogger_2 is all defaults
        try
        {
            testLoggerConfig = consConfigImpl.LoggerConfigMap["TestLogger_2"];
        }
        catch
        {
            Assert.False(true);
            return;
        }

        Assert.Equal("TestLogger_2", testLoggerConfig.Name);
        Assert.Equal(defaultLoggerConfig.LoggerType, testLoggerConfig.LoggerType);
        Assert.Equal(defaultLoggerConfig.LoggerSeverity, testLoggerConfig.LoggerSeverity);
        Assert.Equal(defaultLoggerConfig.FileName, testLoggerConfig.FileName);
        Assert.Equal(defaultLoggerConfig.IncludeDateInLoggerOutput, testLoggerConfig.IncludeDateInLoggerOutput);
        Assert.Equal(defaultLoggerConfig.NumberOfLogFiles, testLoggerConfig.NumberOfLogFiles);
        Assert.Equal(defaultLoggerConfig.MaxLogFileSize, testLoggerConfig.MaxLogFileSize);

        try
        {
            testDictConfig = consConfigImpl.DictionaryConfigMap["TestDictionary_1"];
        }
        catch
        {
            Assert.False(true);
            return;
        }

        Assert.Equal("TestDictionary_1", testDictConfig.Name);
        Assert.Equal(EmaConfig.DictionaryTypeEnum.FILE, testDictConfig.DictionaryType);
        Assert.Equal("testEnumFile1", testDictConfig.EnumTypeDefFileName);
        Assert.Equal("testEnumItem1", testDictConfig.EnumTypeDefItemName);
        Assert.Equal("testRdmFile1", testDictConfig.RdmFieldDictionaryFileName);
        Assert.Equal("testRdmItem1", testDictConfig.RdmFieldDictionaryItemName);

        // TestDictionary_2 is set to defaults
        try
        {
            testDictConfig = consConfigImpl.DictionaryConfigMap["TestDictionary_2"];
        }
        catch
        {
            Assert.False(true);
            return;
        }

        Assert.Equal("TestDictionary_2", testDictConfig.Name);
        Assert.Equal(defaultDictConfig.DictionaryType, testDictConfig.DictionaryType);
        Assert.Equal(defaultDictConfig.EnumTypeDefFileName, testDictConfig.EnumTypeDefFileName);
        Assert.Equal(defaultDictConfig.EnumTypeDefItemName, testDictConfig.EnumTypeDefItemName);
        Assert.Equal(defaultDictConfig.RdmFieldDictionaryFileName, testDictConfig.RdmFieldDictionaryFileName);
        Assert.Equal(defaultDictConfig.RdmFieldDictionaryItemName, testDictConfig.RdmFieldDictionaryItemName);

        // Verify that the config works here
        try
        {
            consConfigImpl.VerifyConfiguration();
        }
        catch
        {
            Assert.False(true);
            return;
        }

        try
        {
            copiedConfig = new OmmConsumerConfigImpl(consConfigImpl);
        }
        catch
        {
            Assert.False(true);
            return;
        }

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

        // Verify that the ConsumerName has been properly set, that the copied Config has the TestConsumer in the consumer config map, and that 
        // the ConsumerConfig object referenced by ConsumerConfig is the same as in the map.
        Assert.Equal("TestConsumer", copiedConfig.ConsumerName);
        Assert.True(copiedConfig.ConsumerConfigMap.ContainsKey("TestConsumer"));
        Assert.Same(copiedConfig.ConsumerConfig, copiedConfig.ConsumerConfigMap["TestConsumer"]);

        testConsConfig = copiedConfig.ConsumerConfig;

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

        // Check to see if the channel is correct.
        try
        {
            testChannelConfig = copiedConfig.ClientChannelConfigMap["TestChannel_1"];
        }
        catch
        {
            Assert.False(true);
            return;
        }

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
        Assert.True(testChannelConfig.DirectWrite);
        Assert.Equal("testChannel1", testChannelConfig.ConnectInfo.ConnectOptions.UnifiedNetworkInfo.Address);
        Assert.Equal("testPort1", testChannelConfig.ConnectInfo.ConnectOptions.UnifiedNetworkInfo.ServiceName);
        Assert.Equal("proxyHost1", testChannelConfig.ConnectInfo.ConnectOptions.ProxyOptions.ProxyHostName);
        Assert.Equal("proxyPort1", testChannelConfig.ConnectInfo.ConnectOptions.ProxyOptions.ProxyPort);
        Assert.True(testChannelConfig.ConnectInfo.ConnectOptions.TcpOpts.TcpNoDelay);
        Assert.Equal(EncryptionProtocolFlags.ENC_TLSV1_2, testChannelConfig.ConnectInfo.ConnectOptions.EncryptionOpts.EncryptionProtocolFlags);


        // Check that TestLogger exists and that the object in the map is the same as what's set on the OmmConsumerConfigImpl
        Assert.True(copiedConfig.LoggerConfigMap.ContainsKey("TestLogger_1"));
        Assert.Same(copiedConfig.LoggerConfig, copiedConfig.LoggerConfigMap["TestLogger_1"]);
        try
        {
            testLoggerConfig = copiedConfig.LoggerConfigMap["TestLogger_1"];
        }
        catch
        {
            Assert.False(true);
            return;
        }

        Assert.Equal("TestLogger_1", testLoggerConfig.Name);
        Assert.Equal(LoggerType.STDOUT, testLoggerConfig.LoggerType);
        Assert.Equal(LoggerLevel.INFO, testLoggerConfig.LoggerSeverity);
        Assert.Equal("testLogFile1", testLoggerConfig.FileName);
        Assert.Equal((ulong)1, testLoggerConfig.IncludeDateInLoggerOutput);
        Assert.Equal((ulong)10, testLoggerConfig.NumberOfLogFiles);
        Assert.Equal((ulong)20, testLoggerConfig.MaxLogFileSize);


        // Check that TestDictionary_1 exists and that the object in the map is the same as what's set on the OmmConsumerConfigImpl
        Assert.True(copiedConfig.DictionaryConfigMap.ContainsKey("TestDictionary_1"));
        Assert.Same(copiedConfig.DictionaryConfig, copiedConfig.DictionaryConfigMap["TestDictionary_1"]);
        try
        {
            testDictConfig = copiedConfig.DictionaryConfigMap["TestDictionary_1"];
        }
        catch
        {
            Assert.False(true);
            return;
        }

        Assert.Equal("TestDictionary_1", testDictConfig.Name);
        Assert.Equal(EmaConfig.DictionaryTypeEnum.FILE, testDictConfig.DictionaryType);
        Assert.Equal("testEnumFile1", testDictConfig.EnumTypeDefFileName);
        Assert.Equal("testEnumItem1", testDictConfig.EnumTypeDefItemName);
        Assert.Equal("testRdmFile1", testDictConfig.RdmFieldDictionaryFileName);
        Assert.Equal("testRdmItem1", testDictConfig.RdmFieldDictionaryItemName);

        // Generate the role, check the values in it
        // RTT is set when the Login message is passed into the role, so that is not set here.
        // Also, any channel-related ioctrl config will not be generated into the role.
        testRole = copiedConfig.GenerateConsumerRole();

        Assert.Equal(testDictConfig.RdmFieldDictionaryItemName, testRole.FieldDictionaryName.ToString());
        Assert.Equal(testDictConfig.EnumTypeDefItemName, testRole.EnumTypeDictionaryName.ToString());

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
        try
        {
            consConfigImpl.VerifyConfiguration();
        }
        catch
        {
            Assert.False(true);
            return;
        }

        try
        {
            copiedConfig = new OmmConsumerConfigImpl(consConfigImpl);
        }
        catch
        {
            Assert.False(true);
            return;
        }

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

        // Verify that the ConsumerName has been properly set, that the copied Config has the TestConsumer in the consumer config map, and that 
        // the ConsumerConfig object referenced by ConsumerConfig is the same as in the map.
        Assert.Equal("TestConsumer_2", copiedConfig.ConsumerName);
        Assert.True(copiedConfig.ConsumerConfigMap.ContainsKey("TestConsumer_2"));
        Assert.Same(copiedConfig.ConsumerConfig, copiedConfig.ConsumerConfigMap["TestConsumer_2"]);

        try
        {
            testConsConfig = copiedConfig.ConsumerConfigMap["TestConsumer_2"];
        }
        catch
        {
            Assert.False(true);
            return;
        }

        Assert.Equal("TestConsumer_2", testConsConfig.Name);
        Assert.Equal(2, testConsConfig.ChannelSet.Count);
        Assert.Equal("TestChannel_1", testConsConfig.ChannelSet[0]);
        Assert.Equal("TestChannel_2", testConsConfig.ChannelSet[1]);
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
        Assert.Equal(defaultConsConfig.XmlTraceDump, testConsConfig.XmlTraceDump);
        Assert.Equal(defaultConsConfig.XmlTraceFileName, testConsConfig.XmlTraceFileName);
        Assert.Equal(defaultConsConfig.XmlTraceToStdout, testConsConfig.XmlTraceToStdout);

        try
        {
            testChannelConfig = copiedConfig.ClientChannelConfigMap["TestChannel_1"];
        }
        catch
        {
            Assert.False(true);
            return;
        }

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
        Assert.True(testChannelConfig.DirectWrite);
        Assert.Equal("testChannel1", testChannelConfig.ConnectInfo.ConnectOptions.UnifiedNetworkInfo.Address);
        Assert.Equal("testPort1", testChannelConfig.ConnectInfo.ConnectOptions.UnifiedNetworkInfo.ServiceName);
        Assert.Equal("TestProxyHostOverride", testChannelConfig.ConnectInfo.ConnectOptions.ProxyOptions.ProxyHostName);
        Assert.Equal("TestProxyPortOverride", testChannelConfig.ConnectInfo.ConnectOptions.ProxyOptions.ProxyPort);
        Assert.Equal("TestProxyUsernameOverride", testChannelConfig.ConnectInfo.ConnectOptions.ProxyOptions.ProxyUserName);
        Assert.Equal("testProxyPasswordOverride", testChannelConfig.ConnectInfo.ConnectOptions.ProxyOptions.ProxyPassword);
        Assert.True(testChannelConfig.ConnectInfo.ConnectOptions.TcpOpts.TcpNoDelay);
        Assert.Equal(EncryptionProtocolFlags.ENC_TLSV1_2, testChannelConfig.ConnectInfo.ConnectOptions.EncryptionOpts.EncryptionProtocolFlags);

        // TestChannel_2 is the default
        try
        {
            testChannelConfig = copiedConfig.ClientChannelConfigMap["TestChannel_2"];
        }
        catch
        {
            Assert.False(true);
            return;
        }

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
        Assert.Equal("TestProxyHostOverride", testChannelConfig.ConnectInfo.ConnectOptions.ProxyOptions.ProxyHostName);
        Assert.Equal("TestProxyPortOverride", testChannelConfig.ConnectInfo.ConnectOptions.ProxyOptions.ProxyPort);
        Assert.Equal("TestProxyUsernameOverride", testChannelConfig.ConnectInfo.ConnectOptions.ProxyOptions.ProxyUserName);
        Assert.Equal("testProxyPasswordOverride", testChannelConfig.ConnectInfo.ConnectOptions.ProxyOptions.ProxyPassword);
        Assert.Equal(defaultChannelConfig.ConnectInfo.ConnectOptions.TcpOpts.TcpNoDelay, testChannelConfig.ConnectInfo.ConnectOptions.TcpOpts.TcpNoDelay);
        Assert.Equal(defaultChannelConfig.ConnectInfo.ConnectOptions.EncryptionOpts.EncryptionProtocolFlags, testChannelConfig.ConnectInfo.ConnectOptions.EncryptionOpts.EncryptionProtocolFlags);


        // Check that DefaultEmaLogger exists and that the object in the map is the same as what's set on the OmmConsumerConfigImpl
        Assert.True(copiedConfig.LoggerConfigMap.ContainsKey("DefaultEmaLogger"));
        Assert.Same(copiedConfig.LoggerConfig, copiedConfig.LoggerConfigMap["DefaultEmaLogger"]);
        try
        {
            testLoggerConfig = copiedConfig.LoggerConfigMap["DefaultEmaLogger"];
        }
        catch
        {
            Assert.False(true);
            return;
        }

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
        try
        {
            testDictConfig = copiedConfig.DictionaryConfigMap["DefaultEmaDictionary"];
        }
        catch
        {
            Assert.False(true);
            return;
        }

        Assert.Equal("DefaultEmaDictionary", testDictConfig.Name);
        Assert.Equal(defaultDictConfig.DictionaryType, testDictConfig.DictionaryType);
        Assert.Equal(defaultDictConfig.EnumTypeDefFileName, testDictConfig.EnumTypeDefFileName);
        Assert.Equal(defaultDictConfig.EnumTypeDefItemName, testDictConfig.EnumTypeDefItemName);
        Assert.Equal(defaultDictConfig.RdmFieldDictionaryFileName, testDictConfig.RdmFieldDictionaryFileName);
        Assert.Equal(defaultDictConfig.RdmFieldDictionaryItemName, testDictConfig.RdmFieldDictionaryItemName);

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
        try
        {
            consConfigImpl.VerifyConfiguration();
        }
        catch
        {
            Assert.False(true);
            return;
        }

        try
        {
            copiedConfig = new OmmConsumerConfigImpl(consConfigImpl);
        }
        catch
        {
            Assert.False(true);
            return;
        }

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

        try
        {
            testConsConfig = copiedConfig.ConsumerConfigMap["TestConsumer_2"];
        }
        catch
        {
            Assert.False(true);
            return;
        }

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
        Assert.Equal(defaultConsConfig.XmlTraceDump, testConsConfig.XmlTraceDump);
        Assert.Equal(defaultConsConfig.XmlTraceFileName, testConsConfig.XmlTraceFileName); 
        Assert.Equal(defaultConsConfig.XmlTraceToStdout, testConsConfig.XmlTraceToStdout);

        // DefaultEmaChannel should be all defaults except for the host, port, and proxy information
        try
        {
            testChannelConfig = copiedConfig.ClientChannelConfigMap["TestChannel_1"];
        }
        catch
        {
            Assert.False(true);
            return;
        }

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
        try
        {
            testLoggerConfig = copiedConfig.LoggerConfigMap["DefaultEmaLogger"];
        }
        catch
        {
            Assert.False(true);
            return;
        }

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
        try
        {
            testDictConfig = copiedConfig.DictionaryConfigMap["DefaultEmaDictionary"];
        }
        catch
        {
            Assert.False(true);
            return;
        }

        Assert.Equal("DefaultEmaDictionary", testDictConfig.Name);
        Assert.Equal(defaultDictConfig.DictionaryType, testDictConfig.DictionaryType);
        Assert.Equal(defaultDictConfig.EnumTypeDefFileName, testDictConfig.EnumTypeDefFileName);
        Assert.Equal(defaultDictConfig.EnumTypeDefItemName, testDictConfig.EnumTypeDefItemName);
        Assert.Equal(defaultDictConfig.RdmFieldDictionaryFileName, testDictConfig.RdmFieldDictionaryFileName);
        Assert.Equal(defaultDictConfig.RdmFieldDictionaryItemName, testDictConfig.RdmFieldDictionaryItemName);

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


    [Fact]
    public void ProgrammaticConfigTest()
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
        try
        {
            consumerConfig = new OmmConsumerConfig("../../../OmmConsumerConfigTests/EmaBlankConfig.xml");
        }
        catch (OmmInvalidConfigurationException)
        {
            Assert.False(true);
            return;
        }

        OmmConsumerConfigImpl consConfigImpl = consumerConfig.OmmConsConfigImpl;

        // Loaded the Config, now make sure everything's in it.

        // There should be 2 Consumers, 2 ClientChannels, 1 Logger, and 1 Dictionary
        Assert.Empty(consConfigImpl.ConsumerConfigMap);
        Assert.Empty(consConfigImpl.ClientChannelConfigMap);
        Assert.Empty(consConfigImpl.LoggerConfigMap);
        Assert.Empty(consConfigImpl.DictionaryConfigMap);

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
            .AddUInt("XmlTraceDump", 1)
            .AddUInt("XmlTraceToStdout", 1)
            //.AddAscii("XmlTraceFileName", "ProgXmlFile")
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
            .AddUInt("AuthenticationTimeout", 15)
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

        try
        {
            consConfigImpl.Config(outerMap);
        }
        catch (Exception excp)
        {
            Assert.False(true, excp.Message);
            return;
        }

        Assert.Equal(2, consConfigImpl.ConsumerConfigMap.Count);
        Assert.Equal(2, consConfigImpl.ClientChannelConfigMap.Count);
        Assert.Equal(2, consConfigImpl.LoggerConfigMap.Count);
        Assert.Equal(2, consConfigImpl.DictionaryConfigMap.Count);

        try
        {
            testConsConfig = consConfigImpl.ConsumerConfigMap["ProgConsumer_1"];
        }
        catch
        {
            Assert.False(true);
            return;
        }

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
        //Assert.Equal((ulong)1, testConsConfig.XmlTraceDump);
       //Assert.Equal("ProgXmlFile", testConsConfig.XmlTraceFileName);
        Assert.True(testConsConfig.XmlTraceToStdout);


        // ProgConsumer_2 has all defaults except for ChannelSet.

        try
        {
            testConsConfig = consConfigImpl.ConsumerConfigMap["ProgConsumer_2"];
        }
        catch
        {
            Assert.False(true);
            return;
        }

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
        Assert.Equal(defaultConsConfig.XmlTraceDump, testConsConfig.XmlTraceDump);
        Assert.Equal(defaultConsConfig.XmlTraceFileName, testConsConfig.XmlTraceFileName);
        Assert.Equal(defaultConsConfig.XmlTraceToStdout, testConsConfig.XmlTraceToStdout);

        try
        {
            testChannelConfig = consConfigImpl.ClientChannelConfigMap["ProgChannel_1"];
        }
        catch
        {
            Assert.False(true);
            return;
        }

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
        try
        {
            testChannelConfig = consConfigImpl.ClientChannelConfigMap["ProgChannel_2"];
        }
        catch
        {
            Assert.False(true);
            return;
        }

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

        try
        {
            testLoggerConfig = consConfigImpl.LoggerConfigMap["ProgLogger_1"];
        }
        catch
        {
            Assert.False(true);
            return;
        }

        Assert.Equal("ProgLogger_1", testLoggerConfig.Name);
        Assert.Equal(LoggerType.STDOUT, testLoggerConfig.LoggerType);
        Assert.Equal(LoggerLevel.INFO, testLoggerConfig.LoggerSeverity);
        Assert.Equal("ProgLogFile", testLoggerConfig.FileName);
        Assert.Equal((ulong)1, testLoggerConfig.IncludeDateInLoggerOutput);
        Assert.Equal((ulong)20, testLoggerConfig.NumberOfLogFiles);
        Assert.Equal((ulong)100, testLoggerConfig.MaxLogFileSize);

        // ProgLogger_2 is all defaults
        try
        {
            testLoggerConfig = consConfigImpl.LoggerConfigMap["ProgLogger_2"];
        }
        catch
        {
            Assert.False(true);
            return;
        }

        Assert.Equal("ProgLogger_2", testLoggerConfig.Name);
        Assert.Equal(defaultLoggerConfig.LoggerType, testLoggerConfig.LoggerType);
        Assert.Equal(defaultLoggerConfig.LoggerSeverity, testLoggerConfig.LoggerSeverity);
        Assert.Equal(defaultLoggerConfig.FileName, testLoggerConfig.FileName);
        Assert.Equal(defaultLoggerConfig.IncludeDateInLoggerOutput, testLoggerConfig.IncludeDateInLoggerOutput);
        Assert.Equal(defaultLoggerConfig.NumberOfLogFiles, testLoggerConfig.NumberOfLogFiles);
        Assert.Equal(defaultLoggerConfig.MaxLogFileSize, testLoggerConfig.MaxLogFileSize);

        try
        {
            testDictConfig = consConfigImpl.DictionaryConfigMap["ProgDictionary_1"];
        }
        catch
        {
            Assert.False(true);
            return;
        }

        Assert.Equal("ProgDictionary_1", testDictConfig.Name);
        Assert.Equal(EmaConfig.DictionaryTypeEnum.FILE, testDictConfig.DictionaryType);
        Assert.Equal("ProgEnumFile", testDictConfig.EnumTypeDefFileName);
        Assert.Equal("ProgEnumItem", testDictConfig.EnumTypeDefItemName);
        Assert.Equal("ProgFieldFile", testDictConfig.RdmFieldDictionaryFileName);
        Assert.Equal("ProgFieldItem", testDictConfig.RdmFieldDictionaryItemName);

        // ProgDictionary_1 is set to defaults
        try
        {
            testDictConfig = consConfigImpl.DictionaryConfigMap["ProgDictionary_2"];
        }
        catch
        {
            Assert.False(true);
            return;
        }

        Assert.Equal("ProgDictionary_2", testDictConfig.Name);
        Assert.Equal(defaultDictConfig.DictionaryType, testDictConfig.DictionaryType);
        Assert.Equal(defaultDictConfig.EnumTypeDefFileName, testDictConfig.EnumTypeDefFileName);
        Assert.Equal(defaultDictConfig.EnumTypeDefItemName, testDictConfig.EnumTypeDefItemName);
        Assert.Equal(defaultDictConfig.RdmFieldDictionaryFileName, testDictConfig.RdmFieldDictionaryFileName);
        Assert.Equal(defaultDictConfig.RdmFieldDictionaryItemName, testDictConfig.RdmFieldDictionaryItemName);

        // Verify that the config works here
        try
        {
            consConfigImpl.VerifyConfiguration();
        }
        catch
        {
            Assert.False(true);
            return;
        }
    }



    // This tests the programmatic overlay functionality.
    // The full test config is loaded, and all elements are overwritten by the programmatic config.
    [Fact]
    public void ProgrammaticOverlayTest()
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
        try
        {
            consumerConfig = new OmmConsumerConfig("../../../OmmConsumerConfigTests/EmaBlankConfig.xml");
        }
        catch (OmmInvalidConfigurationException)
        {
            Assert.False(true);
            return;
        }

        OmmConsumerConfigImpl consConfigImpl = consumerConfig.OmmConsConfigImpl;

        // Loaded the Config, now make sure everything's in it.

        // There should be nothing in here
        Assert.Empty(consConfigImpl.ConsumerConfigMap);
        Assert.Empty(consConfigImpl.ClientChannelConfigMap);
        Assert.Empty(consConfigImpl.LoggerConfigMap);
        Assert.Empty(consConfigImpl.DictionaryConfigMap);

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
            .AddUInt("XmlTraceDump", 0)
            .AddUInt("XmlTraceToStdout", 0)
            //.AddAscii("XmlTraceFileName", "ProgXmlFile")
            .Complete();

        innerMap.AddKeyAscii("TestConsumer_1", MapAction.ADD, encodeObjectList);

        encodeObjectList.Clear();

        encodeObjectList.AddAscii("ChannelSet", "ProgChannel_1, ProgChannel_2")
            .Complete();

        innerMap.AddKeyAscii("TestConsumer_2", MapAction.ADD, encodeObjectList);

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

        try
        {
            consConfigImpl.Config(outerMap);
        }
        catch (Exception excp)
        {
            Assert.False(true, excp.Message);
            return;
        }

        Assert.Equal(2, consConfigImpl.ConsumerConfigMap.Count);
        Assert.Equal(2, consConfigImpl.ClientChannelConfigMap.Count);
        Assert.Equal(2, consConfigImpl.LoggerConfigMap.Count);
        Assert.Equal(2, consConfigImpl.DictionaryConfigMap.Count);

        try
        {
            testConsConfig = consConfigImpl.ConsumerConfigMap["TestConsumer_1"];
        }
        catch
        {
            Assert.False(true);
            return;
        }

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
        Assert.Equal((ulong)0, testConsConfig.XmlTraceDump);
        //Assert.Equal("ProgXmlFile", testConsConfig.XmlTraceFileName);
        Assert.False(testConsConfig.XmlTraceToStdout);


        // ProgConsumer_2 has all defaults except for ChannelSet.

        try
        {
            testConsConfig = consConfigImpl.ConsumerConfigMap["TestConsumer_2"];
        }
        catch
        {
            Assert.False(true);
            return;
        }

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
        Assert.Equal(defaultConsConfig.XmlTraceDump, testConsConfig.XmlTraceDump);
        Assert.Equal(defaultConsConfig.XmlTraceFileName, testConsConfig.XmlTraceFileName);
        Assert.Equal(defaultConsConfig.XmlTraceToStdout, testConsConfig.XmlTraceToStdout);

        try
        {
            testChannelConfig = consConfigImpl.ClientChannelConfigMap["TestChannel_1"];
        }
        catch
        {
            Assert.False(true);
            return;
        }

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
        try
        {
            testChannelConfig = consConfigImpl.ClientChannelConfigMap["TestChannel_2"];
        }
        catch
        {
            Assert.False(true);
            return;
        }

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

        try
        {
            testLoggerConfig = consConfigImpl.LoggerConfigMap["TestLogger_1"];
        }
        catch
        {
            Assert.False(true);
            return;
        }

        Assert.Equal("TestLogger_1", testLoggerConfig.Name);
        Assert.Equal(LoggerType.FILE, testLoggerConfig.LoggerType);
        Assert.Equal(LoggerLevel.ERROR, testLoggerConfig.LoggerSeverity);
        Assert.Equal("ProgLogFile", testLoggerConfig.FileName);
        Assert.Equal((ulong)1, testLoggerConfig.IncludeDateInLoggerOutput);
        Assert.Equal((ulong)20, testLoggerConfig.NumberOfLogFiles);
        Assert.Equal((ulong)100, testLoggerConfig.MaxLogFileSize);

        // TestLogger_2 is all defaults
        try
        {
            testLoggerConfig = consConfigImpl.LoggerConfigMap["TestLogger_2"];
        }
        catch
        {
            Assert.False(true);
            return;
        }

        Assert.Equal("TestLogger_2", testLoggerConfig.Name);
        Assert.Equal(defaultLoggerConfig.LoggerType, testLoggerConfig.LoggerType);
        Assert.Equal(defaultLoggerConfig.LoggerSeverity, testLoggerConfig.LoggerSeverity);
        Assert.Equal(defaultLoggerConfig.FileName, testLoggerConfig.FileName);
        Assert.Equal(defaultLoggerConfig.IncludeDateInLoggerOutput, testLoggerConfig.IncludeDateInLoggerOutput);
        Assert.Equal(defaultLoggerConfig.NumberOfLogFiles, testLoggerConfig.NumberOfLogFiles);
        Assert.Equal(defaultLoggerConfig.MaxLogFileSize, testLoggerConfig.MaxLogFileSize);

        try
        {
            testDictConfig = consConfigImpl.DictionaryConfigMap["TestDictionary_1"];
        }
        catch
        {
            Assert.False(true);
            return;
        }

        Assert.Equal("TestDictionary_1", testDictConfig.Name);
        Assert.Equal(EmaConfig.DictionaryTypeEnum.CHANNEL, testDictConfig.DictionaryType);
        Assert.Equal("ProgEnumFile", testDictConfig.EnumTypeDefFileName);
        Assert.Equal("ProgEnumItem", testDictConfig.EnumTypeDefItemName);
        Assert.Equal("ProgFieldFile", testDictConfig.RdmFieldDictionaryFileName);
        Assert.Equal("ProgFieldItem", testDictConfig.RdmFieldDictionaryItemName);

        // ProgDictionary_1 is set to defaults
        try
        {
            testDictConfig = consConfigImpl.DictionaryConfigMap["TestDictionary_2"];
        }
        catch
        {
            Assert.False(true);
            return;
        }

        Assert.Equal("TestDictionary_2", testDictConfig.Name);
        Assert.Equal(defaultDictConfig.DictionaryType, testDictConfig.DictionaryType);
        Assert.Equal(defaultDictConfig.EnumTypeDefFileName, testDictConfig.EnumTypeDefFileName);
        Assert.Equal(defaultDictConfig.EnumTypeDefItemName, testDictConfig.EnumTypeDefItemName);
        Assert.Equal(defaultDictConfig.RdmFieldDictionaryFileName, testDictConfig.RdmFieldDictionaryFileName);
        Assert.Equal(defaultDictConfig.RdmFieldDictionaryItemName, testDictConfig.RdmFieldDictionaryItemName);

        // Verify that the config works here
        try
        {
            consConfigImpl.VerifyConfiguration();
            // Expected that VerifyConfiguration throws an exception
            Assert.False(true);
        }
        catch
        {
           // Expected failure here.
         
        }
    }

    // This test will a blank config and verify that a default-only generated config is created.
    [Fact]
    public void DefaultConfigTest()
    {
        OmmConsumerConfig consumerConfig;
        ConsumerConfig testConsConfig;
        ClientChannelConfig testChannelConfig;
        LoggerConfig testLoggerConfig;
        DictionaryConfig testDictionaryConfig;
        OmmConsumerConfigImpl copiedConfig;

        // Load a blank config so we can be sure that everything added is from the programmtic config
        // This doesn't use the default behavior(loading EmaConfig.xml) because an EmaConfig.xml file may be present in the directory for other example apps
        try
        {
            consumerConfig = new OmmConsumerConfig("../../../OmmConsumerConfigTests/EmaBlankConfig.xml");
        }
        catch (OmmInvalidConfigurationException)
        {
            Assert.False(true);
            return;
        }

        OmmConsumerConfigImpl consConfigImpl = consumerConfig.OmmConsConfigImpl;

        // Loaded the Config, now make sure everything's in it.

        // There should be nothing in here
        Assert.Empty(consConfigImpl.ConsumerConfigMap);
        Assert.Empty(consConfigImpl.ClientChannelConfigMap);
        Assert.Empty(consConfigImpl.LoggerConfigMap);
        Assert.Empty(consConfigImpl.DictionaryConfigMap);

        // Verify should succeed
        try
        {
            consConfigImpl.VerifyConfiguration();
        }
        catch (OmmInvalidConfigurationException)
        {
            Assert.False(true);
            return;
        }

        try
        {
            copiedConfig = new OmmConsumerConfigImpl(consConfigImpl);
        }
        catch (OmmInvalidConfigurationException)
        {
            Assert.False(true);
            return;
        }

        // There should be one default-generated consumer, and one default-generated channel in the maps
        // Logger and Dictionary are set in copiedConfig.Logger and copiedConfig.Dictionary, respectively
        Assert.Single(copiedConfig.ConsumerConfigMap);
        Assert.Single(copiedConfig.ClientChannelConfigMap);
        Assert.Single(copiedConfig.LoggerConfigMap);
        Assert.Single(copiedConfig.DictionaryConfigMap);

        // DefaultEmaConsumer has all defaults and generated names
        try
        {
            testConsConfig = copiedConfig.ConsumerConfigMap["DefaultEmaConsumer"];
        }
        catch
        {
            Assert.False(true);
            return;
        }

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
        Assert.Equal(defaultConsConfig.XmlTraceDump, testConsConfig.XmlTraceDump);
        Assert.Equal(defaultConsConfig.XmlTraceFileName, testConsConfig.XmlTraceFileName);
        Assert.Equal(defaultConsConfig.XmlTraceToStdout, testConsConfig.XmlTraceToStdout);

        // DefaultEmaChannel is the generated default channel
        try
        {
            testChannelConfig = copiedConfig.ClientChannelConfigMap["DefaultEmaChannel"];
        }
        catch
        {
            Assert.False(true);
            return;
        }

        defaultChannelConfig.ConnectInfo.ConnectOptions.UnifiedNetworkInfo.Address = ClientChannelConfig.DefaultHost;
        defaultChannelConfig.ConnectInfo.ConnectOptions.UnifiedNetworkInfo.ServiceName = ClientChannelConfig.DefaultPort;

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
        Assert.Equal(defaultChannelConfig.ConnectInfo.ConnectOptions.UnifiedNetworkInfo.Address, testChannelConfig.ConnectInfo.ConnectOptions.UnifiedNetworkInfo.Address);
        Assert.Equal(defaultChannelConfig.ConnectInfo.ConnectOptions.UnifiedNetworkInfo.ServiceName, testChannelConfig.ConnectInfo.ConnectOptions.UnifiedNetworkInfo.ServiceName);
        Assert.Equal(defaultChannelConfig.ConnectInfo.ConnectOptions.GuaranteedOutputBuffers, testChannelConfig.ConnectInfo.ConnectOptions.GuaranteedOutputBuffers);
        Assert.Equal(defaultChannelConfig.ConnectInfo.ConnectOptions.ProxyOptions.ProxyHostName, testChannelConfig.ConnectInfo.ConnectOptions.ProxyOptions.ProxyHostName);
        Assert.Equal(defaultChannelConfig.ConnectInfo.ConnectOptions.ProxyOptions.ProxyPort, testChannelConfig.ConnectInfo.ConnectOptions.ProxyOptions.ProxyPort);
        Assert.Equal(defaultChannelConfig.ConnectInfo.ConnectOptions.TcpOpts.TcpNoDelay, testChannelConfig.ConnectInfo.ConnectOptions.TcpOpts.TcpNoDelay);
        Assert.Equal(defaultChannelConfig.ConnectInfo.ConnectOptions.EncryptionOpts.EncryptionProtocolFlags, testChannelConfig.ConnectInfo.ConnectOptions.EncryptionOpts.EncryptionProtocolFlags);

        // The logger will be generated and contain all defaults
        try
        {
            testLoggerConfig = copiedConfig.LoggerConfigMap["DefaultEmaLogger"];
        }
        catch
        {
            Assert.False(true);
            return;
        }

        Assert.Same(copiedConfig.LoggerConfig, copiedConfig.LoggerConfigMap["DefaultEmaLogger"]);
        Assert.Equal("DefaultEmaLogger", testLoggerConfig.Name);
        Assert.Equal(defaultLoggerConfig.LoggerType, testLoggerConfig.LoggerType);
        Assert.Equal(defaultLoggerConfig.LoggerSeverity, testLoggerConfig.LoggerSeverity);
        Assert.Equal(defaultLoggerConfig.FileName, testLoggerConfig.FileName);
        Assert.Equal(defaultLoggerConfig.IncludeDateInLoggerOutput, testLoggerConfig.IncludeDateInLoggerOutput);
        Assert.Equal(defaultLoggerConfig.NumberOfLogFiles, testLoggerConfig.NumberOfLogFiles);
        Assert.Equal(defaultLoggerConfig.MaxLogFileSize, testLoggerConfig.MaxLogFileSize);

        // The Dictionary will be generated and contain all defaults
        try
        {
            testDictionaryConfig = copiedConfig.DictionaryConfigMap["DefaultEmaDictionary"];
        }
        catch
        {
            Assert.False(true);
            return;
        }

        Assert.Same(copiedConfig.DictionaryConfig, copiedConfig.DictionaryConfigMap["DefaultEmaDictionary"]);
        Assert.Equal("DefaultEmaDictionary", testDictionaryConfig.Name);
        Assert.Equal(defaultDictConfig.DictionaryType, testDictionaryConfig.DictionaryType);
        Assert.Equal(defaultDictConfig.EnumTypeDefFileName, testDictionaryConfig.EnumTypeDefFileName);
        Assert.Equal(defaultDictConfig.EnumTypeDefItemName, testDictionaryConfig.EnumTypeDefItemName);
        Assert.Equal(defaultDictConfig.RdmFieldDictionaryFileName, testDictionaryConfig.RdmFieldDictionaryFileName);
        Assert.Equal(defaultDictConfig.RdmFieldDictionaryItemName, testDictionaryConfig.RdmFieldDictionaryItemName);

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
        try
        {
            consumerConfig = new OmmConsumerConfig("../../../OmmConsumerConfigTests/EmaBlankConfig.xml");
        }
        catch (OmmInvalidConfigurationException)
        {
            Assert.False(true);
            return;
        }

        OmmConsumerConfigImpl consConfigImpl = consumerConfig.OmmConsConfigImpl;

        // Loaded the Config, now make sure everything's in it.

        // There should be nothing in here
        Assert.Empty(consConfigImpl.ConsumerConfigMap);
        Assert.Empty(consConfigImpl.ClientChannelConfigMap);
        Assert.Empty(consConfigImpl.LoggerConfigMap);
        Assert.Empty(consConfigImpl.DictionaryConfigMap);

        // Verify should succeed
        try
        {
            consConfigImpl.VerifyConfiguration();
        }
        catch (OmmInvalidConfigurationException)
        {
            Assert.False(true);
            return;
        }

        // Set a non-existant consumer name
        consConfigImpl.ConsumerName = "bad_consumer";

        try
        {
            consConfigImpl.VerifyConfiguration();
            Assert.False(true);
        }
        catch (OmmInvalidConfigurationException e)
        {
            Assert.True(true, e.Message);
        }


        // Unset the ConsumerName and test the DefaultConsumer string
        consConfigImpl.ConsumerName = string.Empty;
        consConfigImpl.DefaultConsumer = "bad_consumer";

        try
        {
            consConfigImpl.VerifyConfiguration();
            Assert.False(true);
        }
        catch (OmmInvalidConfigurationException e)
        {
            Assert.True(true, e.Message);
        }

        consConfigImpl.DefaultConsumer = string.Empty;

        tmpConsumerConfig = new ConsumerConfig();

        tmpConsumerConfig.Name = "consumer_1";
        tmpConsumerConfig.ChannelSet.Add("Bad_channel_1");
        
        consConfigImpl.ConsumerConfigMap.Add(tmpConsumerConfig.Name, tmpConsumerConfig);

        try
        {
            consConfigImpl.VerifyConfiguration();
            Assert.False(true);
        }
        catch (OmmInvalidConfigurationException e)
        {
            Assert.True(true, e.Message);
        }

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

        try
        {
            consConfigImpl.VerifyConfiguration();
            Assert.False(true);
        }
        catch (OmmInvalidConfigurationException e)
        {
            Assert.True(true, e.Message);
        }

        // Clear the channelSet, add 3 channels with the failure in the middle.
        tmpConsumerConfig.ChannelSet.Clear();
        tmpConsumerConfig.ChannelSet.Add("ConfigChannel_1");
        tmpConsumerConfig.ChannelSet.Add("bad_channel");
        tmpConsumerConfig.ChannelSet.Add("ConfigChannel_3");

        try
        {
            consConfigImpl.VerifyConfiguration();
            Assert.False(true);
        }
        catch (OmmInvalidConfigurationException e)
        {
            Assert.True(true, e.Message);
        }

        // Clear the channelSet, add 3 channels with the failure at the end.
        tmpConsumerConfig.ChannelSet.Clear();
        tmpConsumerConfig.ChannelSet.Add("ConfigChannel_1");
        tmpConsumerConfig.ChannelSet.Add("ConfigChannel_2");
        tmpConsumerConfig.ChannelSet.Add("bad_channel");

        try
        {
            consConfigImpl.VerifyConfiguration();
            Assert.False(true);
        }
        catch (OmmInvalidConfigurationException e)
        {
            Assert.True(true, e.Message);
        }

        // Clear the channelSet, set 3 good channels, make sure it succeeds
        tmpConsumerConfig.ChannelSet.Clear();
        tmpConsumerConfig.ChannelSet.Add("ConfigChannel_1");
        tmpConsumerConfig.ChannelSet.Add("ConfigChannel_2");
        tmpConsumerConfig.ChannelSet.Add("ConfigChannel_3");

        try
        {
            consConfigImpl.VerifyConfiguration();
            Assert.True(true);
        }
        catch (OmmInvalidConfigurationException e)
        {
            Assert.False(true, e.Message);
        }

        // Add a logger mismatch

        tmpConsumerConfig.Logger = "bad_logger";

        try
        {
            consConfigImpl.VerifyConfiguration();
            Assert.False(true);
        }
        catch (OmmInvalidConfigurationException e)
        {
            Assert.True(true, e.Message);
        }

        // Add a couple of loggers and make sure it's still a mismatch

        tmpLoggerConfig_1 = new LoggerConfig();
        tmpLoggerConfig_1.Name = "ConfigLogger_1";
        consConfigImpl.LoggerConfigMap.Add(tmpLoggerConfig_1.Name, tmpLoggerConfig_1);

        tmpLoggerConfig_2 = new LoggerConfig();
        tmpLoggerConfig_2.Name = "ConfigLogger_2";
        consConfigImpl.LoggerConfigMap.Add(tmpLoggerConfig_2.Name, tmpLoggerConfig_2);

        tmpConsumerConfig.Logger = "bad_logger";

        try
        {
            consConfigImpl.VerifyConfiguration();
            Assert.False(true);
        }
        catch (OmmInvalidConfigurationException e)
        {
            Assert.True(true, e.Message);
        }

        // Set the logger correctly and make sure it verifies

        tmpConsumerConfig.Logger = "ConfigLogger_2";

        try
        {
            consConfigImpl.VerifyConfiguration();
            Assert.True(true);
        }
        catch (OmmInvalidConfigurationException e)
        {
            Assert.False(true, e.Message);
        }

        // Set the dictionary to incorrect value

        tmpConsumerConfig.Dictionary = "bad_dictionary";

        try
        {
            consConfigImpl.VerifyConfiguration();
            Assert.False(true);
        }
        catch (OmmInvalidConfigurationException e)
        {
            Assert.True(true, e.Message);
        }

        // Add in the dictionaries and make sure it still fails.
        tmpDictionaryConfig_1 = new DictionaryConfig();
        tmpDictionaryConfig_1.Name = "ConfigDictionary_1";
        consConfigImpl.DictionaryConfigMap.Add(tmpDictionaryConfig_1.Name, tmpDictionaryConfig_1);

        tmpDictionaryConfig_2 = new DictionaryConfig();
        tmpDictionaryConfig_2.Name = "ConfigDictionary_2";
        consConfigImpl.DictionaryConfigMap.Add(tmpDictionaryConfig_2.Name, tmpDictionaryConfig_2);

        try
        {
            consConfigImpl.VerifyConfiguration();
            Assert.False(true);
        }
        catch (OmmInvalidConfigurationException e)
        {
            Assert.True(true, e.Message);
        }
    }

    // Tests the addadminmsg functionality.
    [Fact]
    public void AddAdminMsgTest()
    {
        OmmConsumerConfig consumerConfig;
        OmmConsumerConfigImpl copiedConfig;
        ConsumerRole testRole;
        ConfigError? error;

        try
        {
            consumerConfig = new OmmConsumerConfig("../../../OmmConsumerConfigTests/EmaTestConfig.xml");
        }
        catch (OmmInvalidConfigurationException)
        {
            Assert.False(true);
            return;
        }

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

        try
        {
            consumerConfig.AddAdminMsg(reqMsg);
        }
        catch (OmmInvalidConfigurationException)
        {
            Assert.False(true);
            return;
        }

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

        try
        {
            consumerConfig.AddAdminMsg(reqMsg);
        }
        catch (OmmInvalidConfigurationException)
        {
            Assert.False(true);
            return;
        }

        Assert.NotNull(consConfigImpl.AdminDirectoryRequest);

        Assert.Equal(ServiceFilterFlags.ALL_FILTERS, consConfigImpl.AdminDirectoryRequest?.Filter);

        Assert.False(consConfigImpl.AdminDirectoryRequest?.HasServiceId);

        // There should be two logged error in the log, both with a WARNING error level
        Assert.Equal(2, consConfigImpl.ConfigErrorLog?.Count());

        
        for(int i = 0; i < consConfigImpl.ConfigErrorLog?.Count(); ++i)
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

        try
        {
            consumerConfig.AddAdminMsg(reqMsg);
        }
        catch (OmmInvalidConfigurationException)
        {
            Assert.False(true);
            return;
        }

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

        try
        {
            consumerConfig.AddAdminMsg(reqMsg);
        }
        catch (OmmInvalidConfigurationException)
        {
            Assert.False(true);
            return;
        }

        Assert.Null(consConfigImpl.AdminEnumDictionaryRequest);
        Assert.Null(consConfigImpl.AdminFieldDictionaryRequest);

        // There should be no logged errors in the log
        Assert.Equal(1, consConfigImpl.ConfigErrorLog?.Count());

        // Clear the error log for next failure tests
        consConfigImpl.ConfigErrorLog?.Clear();

        // set name, no service name or service id, so error in log
        reqMsg.Clear();
        reqMsg.Name("BadDictName").DomainType((int)DomainType.DICTIONARY).EncodeComplete();

        try
        {
            consumerConfig.AddAdminMsg(reqMsg);
        }
        catch (OmmInvalidConfigurationException)
        {
            Assert.False(true);
            return;
        }

        Assert.Null(consConfigImpl.AdminEnumDictionaryRequest);
        Assert.Null(consConfigImpl.AdminFieldDictionaryRequest);

        // There should be no logged errors in the log
        Assert.Equal(1, consConfigImpl.ConfigErrorLog?.Count());

        // Clear the error log for next failure tests
        consConfigImpl.ConfigErrorLog?.Clear();

        // set name, set service id, no filter, so error in log
        reqMsg.Clear();
        reqMsg.Name("BadDictName").ServiceId(10).DomainType((int)DomainType.DICTIONARY).EncodeComplete();

        try
        {
            consumerConfig.AddAdminMsg(reqMsg);
        }
        catch (OmmInvalidConfigurationException)
        {
            Assert.False(true);
            return;
        }

        Assert.Null(consConfigImpl.AdminEnumDictionaryRequest);
        Assert.Null(consConfigImpl.AdminFieldDictionaryRequest);

        // There should be no logged errors in the log
        Assert.Equal(1, consConfigImpl.ConfigErrorLog?.Count());

        // Clear the error log for next failure tests
        consConfigImpl.ConfigErrorLog?.Clear();

        // set name, set service id, filter, set no_refresh so error in log
        reqMsg.Clear();
        reqMsg.Name("BadDictName").ServiceId(10).Filter(Dictionary.VerbosityValues.VERBOSE).InitialImage(false).DomainType((int)DomainType.DICTIONARY).EncodeComplete();

        try
        {
            consumerConfig.AddAdminMsg(reqMsg);
        }
        catch (OmmInvalidConfigurationException)
        {
            Assert.False(true);
            return;
        }

        Assert.Null(consConfigImpl.AdminEnumDictionaryRequest);
        Assert.Null(consConfigImpl.AdminFieldDictionaryRequest);

        // There should be no logged errors in the log
        Assert.Equal(1, consConfigImpl.ConfigErrorLog?.Count());

        // Clear the error log for next failure tests
        consConfigImpl.ConfigErrorLog?.Clear();

        // set name, set service id, filter, set no_refresh so error in log
        reqMsg.Clear();
        reqMsg.Name("BadDictName").ServiceId(10).Filter(Dictionary.VerbosityValues.VERBOSE).InitialImage(false).DomainType((int)DomainType.DICTIONARY).EncodeComplete();

        try
        {
            consumerConfig.AddAdminMsg(reqMsg);
        }
        catch (OmmInvalidConfigurationException)
        {
            Assert.False(true);
            return;
        }

        Assert.Null(consConfigImpl.AdminEnumDictionaryRequest);
        Assert.Null(consConfigImpl.AdminFieldDictionaryRequest);

        // There should be no logged errors in the log
        Assert.Equal(1, consConfigImpl.ConfigErrorLog?.Count());

        // Clear the error log for next failure tests
        consConfigImpl.ConfigErrorLog?.Clear();

        // bad dictionary name, set service id, filter, set no_refresh so error in log
        reqMsg.Clear();
        reqMsg.Name("BadDictName").ServiceId(10).Filter(Dictionary.VerbosityValues.VERBOSE).DomainType((int)DomainType.DICTIONARY).EncodeComplete();

        try
        {
            consumerConfig.AddAdminMsg(reqMsg);
        }
        catch (OmmInvalidConfigurationException)
        {
            Assert.False(true);
            return;
        }

        Assert.Null(consConfigImpl.AdminEnumDictionaryRequest);
        Assert.Null(consConfigImpl.AdminFieldDictionaryRequest);

        // There should be no logged errors in the log
        Assert.Equal(1, consConfigImpl.ConfigErrorLog?.Count());

        // Clear the error log for next failure tests
        consConfigImpl.ConfigErrorLog?.Clear();

        // correct dictionary name, set service id, filter, set no_refresh so error in log
        reqMsg.Clear();
        reqMsg.Name("RWFFld").ServiceId(10).Filter(Dictionary.VerbosityValues.VERBOSE).DomainType((int)DomainType.DICTIONARY).EncodeComplete();

        try
        {
            consumerConfig.AddAdminMsg(reqMsg);
        }
        catch (OmmInvalidConfigurationException)
        {
            Assert.False(true);
            return;
        }

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

        try
        {
            consumerConfig.AddAdminMsg(reqMsg);
        }
        catch (OmmInvalidConfigurationException)
        {
            Assert.False(true);
            return;
        }

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

        try
        {
            consumerConfig.AddAdminMsg(reqMsg);
        }
        catch (OmmInvalidConfigurationException)
        {
            Assert.False(true);
            return;
        }

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

