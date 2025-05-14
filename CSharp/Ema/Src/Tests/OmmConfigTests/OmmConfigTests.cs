/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2023-2024 LSEG. All rights reserved.     
 *|-----------------------------------------------------------------------------
 */

using System;
using System.Collections.Generic;
using System.Linq;
using System.Net.Security;

using LSEG.Ema.Rdm;
using LSEG.Eta.Codec;
using LSEG.Eta.Rdm;
using LSEG.Eta.Transports;
using LSEG.Eta.ValueAdd.Rdm;
using LSEG.Eta.ValueAdd.Reactor;
using static LSEG.Ema.Access.EmaConfig;
using static LSEG.Eta.Rdm.Directory;
namespace LSEG.Ema.Access.Tests.OmmConfigTests;

public class OmmConfigTests : IDisposable
{
    public void Dispose()
    {
        EtaGlobalPoolTestUtil.Clear();
    }

    // Default objects to compare to in the below tests.
    private static readonly NiProviderConfig defaultNiProviderConfig = new();
    private static readonly IProviderConfig defaultIProviderConfig = new();
    private static readonly ServerConfig defaultServerConfig = new();
    private static readonly ClientChannelConfig defaultChannelConfig = new();
    private static readonly LoggerConfig defaultLoggerConfig = new();
    private static readonly DictionaryConfig defaultDictConfig = new();

    // Default Xml File load test
    // Creates EmaConfig.xml in the current working directory, and attempts to load it.

    // Verifies behavior for all of the enumeration string methods in:
    // DictionaryConfig
    // LoggerConfig
    // ClientChanelConfig
    [Fact]
    public void ConfigStringToEnumTest()
    {
        string testString;

        // ClientChannelConfig connection type and compression type
        Eta.Transports.ConnectionType outputConnType;

        testString = "RSSL_SOCKET";
        outputConnType = ClientChannelConfig.StringToConnectionType(testString);
        Assert.Equal(Eta.Transports.ConnectionType.SOCKET, outputConnType);

        testString = "RSSL_ENCRYPTED";
        outputConnType = ClientChannelConfig.StringToConnectionType(testString);
        Assert.Equal(Eta.Transports.ConnectionType.ENCRYPTED, outputConnType);

        // Check that lower case works
        testString = "rssl_socket";
        outputConnType = ClientChannelConfig.StringToConnectionType(testString);
        Assert.Equal(Eta.Transports.ConnectionType.SOCKET, outputConnType);

        testString = "rssl_encrypted";
        outputConnType = ClientChannelConfig.StringToConnectionType(testString);
        Assert.Equal(Eta.Transports.ConnectionType.ENCRYPTED, outputConnType);

        // Failure case with bad input string
        testString = "bad_conn_type";
        Assert.Throws<OmmInvalidConfigurationException>(() => outputConnType = ClientChannelConfig.StringToConnectionType(testString));

        // Test Compression Type
        Eta.Transports.CompressionType outputCompType;
        testString = "None";
        outputCompType = ClientChannelConfig.StringToCompressionType(testString);
        Assert.Equal(Eta.Transports.CompressionType.NONE, outputCompType);

        testString = "ZLib";
        outputCompType = ClientChannelConfig.StringToCompressionType(testString);
        Assert.Equal(Eta.Transports.CompressionType.ZLIB, outputCompType);

        testString = "LZ4";
        outputCompType = ClientChannelConfig.StringToCompressionType(testString);
        Assert.Equal(Eta.Transports.CompressionType.LZ4, outputCompType);

        // Failure case with bad input string
        testString = "bad_comp_type";
        Assert.Throws<OmmInvalidConfigurationException>(() => outputCompType = ClientChannelConfig.StringToCompressionType(testString));
     
        // LoggerConfig for LoggerLevel and LoggerType
        LoggerLevel outputLoggerLevel;

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

        // Failure case with bad input string
        testString = "bad_log_level";
        Assert.Throws<OmmInvalidConfigurationException>(() => outputLoggerLevel = LoggerConfig.StringToLoggerLevel(testString));


        LoggerType outputLoggerType;

        testString = "File";
        outputLoggerType = LoggerConfig.StringToLoggerType(testString);
        Assert.Equal(LoggerType.FILE, outputLoggerType);

        testString = "Stdout";
        outputLoggerType = LoggerConfig.StringToLoggerType(testString);
        Assert.Equal(LoggerType.STDOUT, outputLoggerType);

        // Failure case with bad input string
        testString = "bad_log_type";
        Assert.Throws<OmmInvalidConfigurationException>(() => outputLoggerType = LoggerConfig.StringToLoggerType(testString));

        // DictionaryConfig DictionaryType
        int outputDictionaryMode;

        testString = "FileDictionary";
        outputDictionaryMode = DictionaryConfig.StringToDictionaryMode(testString);
        Assert.Equal(EmaConfig.DictionaryTypeEnum.FILE, outputDictionaryMode);

        testString = "ChannelDictionary";
        outputDictionaryMode = DictionaryConfig.StringToDictionaryMode(testString);
        Assert.Equal(EmaConfig.DictionaryTypeEnum.CHANNEL, outputDictionaryMode);

        // Failure case with bad input string
        testString = "bad_dictionary_type";
        Assert.Throws<OmmInvalidConfigurationException>(() => outputDictionaryMode = DictionaryConfig.StringToDictionaryMode(testString));

    }

    // Verifies behavior for all of the enumeration string methods in:
    // DirectoryConfig
    [Fact]
    public void DirectoryConfigStringToEnumTest()
    {
        string testString;

        // DirectoryConfig StringToTimeliness
        uint outputTimeliness;

        testString = "RealTime";
        outputTimeliness = DirectoryConfig.StringToTimeliness(testString);
        Assert.Equal(OmmQos.Timelinesses.REALTIME, outputTimeliness);

        testString = "InexactDelayed";
        outputTimeliness = DirectoryConfig.StringToTimeliness(testString);

        // Failure case with bad input string
        testString = "bad_timeliness";
        Assert.Throws<OmmInvalidConfigurationException>(() => outputTimeliness = DirectoryConfig.StringToTimeliness(testString));


        // DirectoryConfig StringToRate
        uint outputRate;

        testString = "TickByTick";
        outputRate = DirectoryConfig.StringToRate(testString);
        Assert.Equal(OmmQos.Rates.TICK_BY_TICK, outputRate);

        testString = "JustInTimeConflated";
        outputRate = DirectoryConfig.StringToRate(testString);
        Assert.Equal(OmmQos.Rates.JUST_IN_TIME_CONFLATED, outputRate);

        // Failure case with bad input string
        testString = "bad_rate";
        Assert.Throws<OmmInvalidConfigurationException>(() => outputTimeliness = DirectoryConfig.StringToRate(testString));


        // DirectoryConfig StringtoCapability
        int capability;

        testString = "MMT_LOGIN";
        capability = DirectoryConfig.StringToCapability(testString);
        Assert.Equal(EmaConfig.CapabilitiesEnum.MMT_LOGIN, capability);

        testString = "MMT_DIRECTORY";
        capability = DirectoryConfig.StringToCapability(testString);
        Assert.Equal(EmaConfig.CapabilitiesEnum.MMT_DIRECTORY, capability);

        testString = "MMT_DICTIONARY";
        capability = DirectoryConfig.StringToCapability(testString);
        Assert.Equal(EmaConfig.CapabilitiesEnum.MMT_DICTIONARY, capability);

        testString = "MMT_MARKET_PRICE";
        capability = DirectoryConfig.StringToCapability(testString);
        Assert.Equal(EmaConfig.CapabilitiesEnum.MMT_MARKET_PRICE, capability);

        testString = "MMT_MARKET_BY_PRICE";
        capability = DirectoryConfig.StringToCapability(testString);
        Assert.Equal(EmaConfig.CapabilitiesEnum.MMT_MARKET_BY_PRICE, capability);

        testString = "MMT_MARKET_BY_ORDER";
        capability = DirectoryConfig.StringToCapability(testString);
        Assert.Equal(EmaConfig.CapabilitiesEnum.MMT_MARKET_BY_ORDER, capability);

        testString = "MMT_MARKET_MAKER";
        capability = DirectoryConfig.StringToCapability(testString);
        Assert.Equal(EmaConfig.CapabilitiesEnum.MMT_MARKET_MAKER, capability);

        testString = "MMT_SYMBOL_LIST";
        capability = DirectoryConfig.StringToCapability(testString);
        Assert.Equal(EmaConfig.CapabilitiesEnum.MMT_SYMBOL_LIST, capability);

        testString = "MMT_SERVICE_PROVIDER_STATUS";
        capability = DirectoryConfig.StringToCapability(testString);
        Assert.Equal(EmaConfig.CapabilitiesEnum.MMT_SERVICE_PROVIDER_STATUS, capability);

        testString = "MMT_HISTORY";
        capability = DirectoryConfig.StringToCapability(testString);
        Assert.Equal(EmaConfig.CapabilitiesEnum.MMT_HISTORY, capability);

        testString = "MMT_HEADLINE";
        capability = DirectoryConfig.StringToCapability(testString);
        Assert.Equal(EmaConfig.CapabilitiesEnum.MMT_HEADLINE, capability);

        testString = "MMT_STORY";
        capability = DirectoryConfig.StringToCapability(testString);
        Assert.Equal(EmaConfig.CapabilitiesEnum.MMT_STORY, capability);

        testString = "MMT_REPLAYHEADLINE";
        capability = DirectoryConfig.StringToCapability(testString);
        Assert.Equal(EmaConfig.CapabilitiesEnum.MMT_REPLAYHEADLINE, capability);

        testString = "MMT_REPLAYSTORY";
        capability = DirectoryConfig.StringToCapability(testString);
        Assert.Equal(EmaConfig.CapabilitiesEnum.MMT_REPLAYSTORY, capability);

        testString = "MMT_TRANSACTION";
        capability = DirectoryConfig.StringToCapability(testString);
        Assert.Equal(EmaConfig.CapabilitiesEnum.MMT_TRANSACTION, capability);

        testString = "MMT_YIELD_CURVE";
        capability = DirectoryConfig.StringToCapability(testString);
        Assert.Equal(EmaConfig.CapabilitiesEnum.MMT_YIELD_CURVE, capability);

        testString = "MMT_CONTRIBUTION";
        capability = DirectoryConfig.StringToCapability(testString);
        Assert.Equal(EmaConfig.CapabilitiesEnum.MMT_CONTRIBUTION, capability);

        testString = "MMT_PROVIDER_ADMIN";
        capability = DirectoryConfig.StringToCapability(testString);
        Assert.Equal(EmaConfig.CapabilitiesEnum.MMT_PROVIDER_ADMIN, capability);

        testString = "MMT_ANALYTICS";
        capability = DirectoryConfig.StringToCapability(testString);
        Assert.Equal(EmaConfig.CapabilitiesEnum.MMT_ANALYTICS, capability);

        testString = "MMT_REFERENCE";
        capability = DirectoryConfig.StringToCapability(testString);
        Assert.Equal(EmaConfig.CapabilitiesEnum.MMT_REFERENCE, capability);

        testString = "MMT_NEWS_TEXT_ANALYTICS";
        capability = DirectoryConfig.StringToCapability(testString);
        Assert.Equal(EmaConfig.CapabilitiesEnum.MMT_NEWS_TEXT_ANALYTICS, capability);

        testString = "MMT_SYSTEM";
        capability = DirectoryConfig.StringToCapability(testString);
        Assert.Equal(EmaConfig.CapabilitiesEnum.MMT_SYSTEM, capability);

        // Failure case with bad input string
        testString = "bad_capability";
        Assert.Throws<OmmInvalidConfigurationException>(() => capability = DirectoryConfig.StringToCapability(testString));


        // DirectoryConfig StringToStreamState
        int streamState;

        testString = "Open";
        streamState = DirectoryConfig.StringToStreamState(testString);
        Assert.Equal(OmmState.StreamStates.OPEN, streamState);

        testString = "NonStreaming";
        streamState = DirectoryConfig.StringToStreamState(testString);
        Assert.Equal(OmmState.StreamStates.NON_STREAMING, streamState);

        testString = "Closed";
        streamState = DirectoryConfig.StringToStreamState(testString);
        Assert.Equal(OmmState.StreamStates.CLOSED, streamState);

        testString = "ClosedRecover";
        streamState = DirectoryConfig.StringToStreamState(testString);
        Assert.Equal(OmmState.StreamStates.CLOSED_RECOVER, streamState);

        testString = "ClosedRedirected";
        streamState = DirectoryConfig.StringToStreamState(testString);
        Assert.Equal(OmmState.StreamStates.CLOSED_REDIRECTED, streamState);

        // Failure case with bad input string
        testString = "bad_state";
        Assert.Throws<OmmInvalidConfigurationException>(() => streamState = DirectoryConfig.StringToCapability(testString));
           
        // DirectoryConfig StringToDataState
        int dataState;

        testString = "NoChange";
        dataState = DirectoryConfig.StringToDataState(testString);
        Assert.Equal(OmmState.DataStates.NO_CHANGE, dataState);

        testString = "Ok";
        dataState = DirectoryConfig.StringToDataState(testString);
        Assert.Equal(OmmState.DataStates.OK, dataState);

        testString = "Suspect";
        dataState = DirectoryConfig.StringToDataState(testString);
        Assert.Equal(OmmState.DataStates.SUSPECT, dataState);

        // Failure case with bad input string
        testString = "bad_state";
        Assert.Throws<OmmInvalidConfigurationException>(() => dataState = DirectoryConfig.StringToCapability(testString));
            
        // DirectoryConfig StringToStatusCode
        int status;

        testString = "None";
        status = DirectoryConfig.StringToStatusCode(testString);
        Assert.Equal(OmmState.StatusCodes.NONE, status);

        testString = "NotFound";
        status = DirectoryConfig.StringToStatusCode(testString);
        Assert.Equal(OmmState.StatusCodes.NOT_FOUND, status);

        testString = "Timeout";
        status = DirectoryConfig.StringToStatusCode(testString);
        Assert.Equal(OmmState.StatusCodes.TIMEOUT, status);

        testString = "NotAuthorized";
        status = DirectoryConfig.StringToStatusCode(testString);
        Assert.Equal(OmmState.StatusCodes.NOT_AUTHORIZED, status);

        testString = "InvalidArgument";
        status = DirectoryConfig.StringToStatusCode(testString);
        Assert.Equal(OmmState.StatusCodes.INVALID_ARGUMENT, status);

        testString = "UsageError";
        status = DirectoryConfig.StringToStatusCode(testString);
        Assert.Equal(OmmState.StatusCodes.USAGE_ERROR, status);

        testString = "Preempted";
        status = DirectoryConfig.StringToStatusCode(testString);
        Assert.Equal(OmmState.StatusCodes.PREEMPTED, status);

        testString = "JustInTimeConflationStarted";
        status = DirectoryConfig.StringToStatusCode(testString);
        Assert.Equal(OmmState.StatusCodes.JUST_IN_TIME_CONFLATION_STARTED, status);

        testString = "TickByTickResumed";
        status = DirectoryConfig.StringToStatusCode(testString);
        Assert.Equal(OmmState.StatusCodes.TICK_BY_TICK_RESUMED, status);

        testString = "FailoverStarted";
        status = DirectoryConfig.StringToStatusCode(testString);
        Assert.Equal(OmmState.StatusCodes.FAILOVER_STARTED, status);

        testString = "FailoverCompleted";
        status = DirectoryConfig.StringToStatusCode(testString);
        Assert.Equal(OmmState.StatusCodes.FAILOVER_COMPLETED, status);

        testString = "GapDetected";
        status = DirectoryConfig.StringToStatusCode(testString);
        Assert.Equal(OmmState.StatusCodes.GAP_DETECTED, status);

        testString = "NoResources";
        status = DirectoryConfig.StringToStatusCode(testString);
        Assert.Equal(OmmState.StatusCodes.NO_RESOURCES, status);

        testString = "TooManyItems";
        status = DirectoryConfig.StringToStatusCode(testString);
        Assert.Equal(OmmState.StatusCodes.TOO_MANY_ITEMS, status);

        testString = "AlreadyOpen";
        status = DirectoryConfig.StringToStatusCode(testString);
        Assert.Equal(OmmState.StatusCodes.ALREADY_OPEN, status);

        testString = "SourceUnknown";
        status = DirectoryConfig.StringToStatusCode(testString);
        Assert.Equal(OmmState.StatusCodes.SOURCE_UNKNOWN, status);

        testString = "NotOpen";
        status = DirectoryConfig.StringToStatusCode(testString);
        Assert.Equal(OmmState.StatusCodes.NOT_OPEN, status);

        testString = "NonUpdatingItem";
        status = DirectoryConfig.StringToStatusCode(testString);
        Assert.Equal(OmmState.StatusCodes.NON_UPDATING_ITEM, status);

        testString = "UnsupportedViewType";
        status = DirectoryConfig.StringToStatusCode(testString);
        Assert.Equal(OmmState.StatusCodes.UNSUPPORTED_VIEW_TYPE, status);

        testString = "InvalidView";
        status = DirectoryConfig.StringToStatusCode(testString);
        Assert.Equal(OmmState.StatusCodes.INVALID_VIEW, status);

        testString = "FullViewProvided";
        status = DirectoryConfig.StringToStatusCode(testString);
        Assert.Equal(OmmState.StatusCodes.FULL_VIEW_PROVIDED, status);

        testString = "UnableToRequestAsBatch";
        status = DirectoryConfig.StringToStatusCode(testString);
        Assert.Equal(OmmState.StatusCodes.UNABLE_TO_REQUEST_AS_BATCH, status);

        testString = "NoBatchViewSupportInReq";
        status = DirectoryConfig.StringToStatusCode(testString);
        Assert.Equal(OmmState.StatusCodes.NO_BATCH_VIEW_SUPPORT_IN_REQ, status);

        testString = "ExceededMaxMountsPerUser";
        status = DirectoryConfig.StringToStatusCode(testString);
        Assert.Equal(OmmState.StatusCodes.EXCEEDED_MAX_MOUNTS_PER_USER, status);

        testString = "Error";
        status = DirectoryConfig.StringToStatusCode(testString);
        Assert.Equal(OmmState.StatusCodes.ERROR, status);

        testString = "DacsDown";
        status = DirectoryConfig.StringToStatusCode(testString);
        Assert.Equal(OmmState.StatusCodes.DACS_DOWN, status);

        testString = "UserUnknownToPermSys";
        status = DirectoryConfig.StringToStatusCode(testString);
        Assert.Equal(OmmState.StatusCodes.USER_UNKNOWN_TO_PERM_SYS, status);

        testString = "DacsMaxLoginsReached";
        status = DirectoryConfig.StringToStatusCode(testString);
        Assert.Equal(OmmState.StatusCodes.DACS_MAX_LOGINS_REACHED, status);

        testString = "DacsUserAccessToAppDenied";
        status = DirectoryConfig.StringToStatusCode(testString);
        Assert.Equal(OmmState.StatusCodes.DACS_USER_ACCESS_TO_APP_DENIED, status);

        testString = "GapFill";
        status = DirectoryConfig.StringToStatusCode(testString);
        Assert.Equal(OmmState.StatusCodes.GAP_FILL, status);

        testString = "AppAuthorizationFailed";
        status = DirectoryConfig.StringToStatusCode(testString);
        Assert.Equal(OmmState.StatusCodes.APP_AUTHORIZATION_FAILED, status);

        // Failure case with bad input string
        testString = "bad_status";
        Assert.Throws<OmmInvalidConfigurationException>(() => status = DirectoryConfig.StringToStatusCode(testString));
    }

    // Xml Config loading and parsing test
    // This loads a config that contains all elements used in the OmmNiProvider config, tests the external setter and getter methods,
    // and verifies that the Reactor ConnectInfo and Reactor Role generation methods work, ensuring that all config members are correctly set
    // in the Role and ConnectInfo objects.
    [Fact]
    public void NiProviderXmlConfigTest()
    {
        OmmNiProviderConfig niProviderConfig;
        NiProviderConfig testNiProvConfig;
        ClientChannelConfig testChannelConfig;
        LoggerConfig testLoggerConfig;
        DictionaryConfig testDictConfig;
        DirectoryConfig testDirectoryConfig;
        EmaServiceConfig testServiceConfig;
        Service? testService;
        OmmNiProviderConfigImpl copiedConfig;
        ReactorConnectOptions testConnOpts;
        NIProviderRole testRole;
        ReactorConnectInfo testConnInfo;

        niProviderConfig = new OmmNiProviderConfig("../../../OmmConfigTests/EmaTestConfig.xml");

        // Set adminControlDirectory to true
        niProviderConfig.AdminControlDirectory(OmmNiProviderConfig.AdminControlMode.API_CONTROL);

        OmmNiProviderConfigImpl niProvConfigImpl = niProviderConfig.OmmNiProvConfigImpl;

        // Check to make sure that there aren't any elements in the ConfigErrorLog
        Assert.Equal(0, niProvConfigImpl.ConfigErrorLog!.Count());

        // Loaded the Config, now make sure everything's in it.

        // There should be 2 Consumers, 2 ClientChannels, 1 Logger, and 1 Dictionary
        Assert.Equal(2, niProvConfigImpl.NiProviderConfigMap.Count);
        Assert.Equal(2, niProvConfigImpl.ClientChannelConfigMap.Count);
        Assert.Equal(2, niProvConfigImpl.LoggerConfigMap.Count);
        Assert.Equal(2, niProvConfigImpl.DictionaryConfigMap.Count);
        Assert.Single(niProvConfigImpl.DirectoryConfigMap);

        Assert.Equal("TestNiProv_1", niProvConfigImpl.FirstConfiguredNiProviderName);

        testNiProvConfig = niProvConfigImpl.NiProviderConfigMap["TestNiProv_1"];

        Assert.Equal("TestNiProv_1", testNiProvConfig.Name);
        Assert.Single(testNiProvConfig.ChannelSet);
        Assert.Equal("TestChannel_1", testNiProvConfig.ChannelSet[0]);
        Assert.Equal("TestLogger_1", testNiProvConfig.Logger);
        Assert.Equal("TestDirectory_1", testNiProvConfig.Directory);
        Assert.Equal((long)30, testNiProvConfig.LoginRequestTimeOut);
        Assert.Equal(40, testNiProvConfig.DispatchTimeoutApiThread);
        Assert.Equal((ulong)50, testNiProvConfig.ItemCountHint);
        Assert.Equal((int)60, testNiProvConfig.MaxDispatchCountApiThread);
        Assert.Equal((int)70, testNiProvConfig.MaxDispatchCountUserThread);
        Assert.False(testNiProvConfig.MergeSourceDirectoryStreams);
        Assert.Equal(110, testNiProvConfig.ReconnectAttemptLimit);
        Assert.Equal(1400, testNiProvConfig.ReconnectMaxDelay);
        Assert.Equal(1300, testNiProvConfig.ReconnectMinDelay);
        Assert.False(testNiProvConfig.RecoverUserSubmitSourceDirectory);
        Assert.False(testNiProvConfig.RefreshFirstRequired);
        Assert.False(testNiProvConfig.RemoveItemsOnDisconnect);
        Assert.Equal((ulong)140, testNiProvConfig.RequestTimeout);
        Assert.Equal((int)160, testNiProvConfig.ServiceCountHint);
        // XML tracing block
        Assert.True(testNiProvConfig.XmlTraceToStdout);
        Assert.False(testNiProvConfig.XmlTraceToFile);
        Assert.Equal((ulong)100_000_000, testNiProvConfig.XmlTraceMaxFileSize);
        Assert.Equal("EmaTrace", testNiProvConfig.XmlTraceFileName);
        Assert.False(testNiProvConfig.XmlTraceToMultipleFiles);
        Assert.True(testNiProvConfig.XmlTraceWrite);
        Assert.True(testNiProvConfig.XmlTraceRead);
        Assert.False(testNiProvConfig.XmlTracePing);


        // TestConsumer_2 has all defaults except for ChannelSet.
        testNiProvConfig = niProvConfigImpl.NiProviderConfigMap["TestNiProv_2"];

        Assert.Equal("TestNiProv_2", testNiProvConfig.Name);
        Assert.Equal(2, testNiProvConfig.ChannelSet.Count);
        Assert.Equal("TestChannel_1", testNiProvConfig.ChannelSet[0]);
        Assert.Equal("TestChannel_2", testNiProvConfig.ChannelSet[1]);
        Assert.Equal(defaultNiProviderConfig.Directory, testNiProvConfig.Directory);
        Assert.Equal(defaultNiProviderConfig.LoginRequestTimeOut, testNiProvConfig.LoginRequestTimeOut);
        Assert.Equal(defaultNiProviderConfig.DispatchTimeoutApiThread, testNiProvConfig.DispatchTimeoutApiThread);
        Assert.Equal(defaultNiProviderConfig.ItemCountHint, testNiProvConfig.ItemCountHint);
        Assert.Equal(defaultNiProviderConfig.MaxDispatchCountApiThread, testNiProvConfig.MaxDispatchCountApiThread);
        Assert.Equal(defaultNiProviderConfig.MaxDispatchCountUserThread, testNiProvConfig.MaxDispatchCountUserThread);
        Assert.Equal(defaultNiProviderConfig.MergeSourceDirectoryStreams, testNiProvConfig.MergeSourceDirectoryStreams);
        Assert.Equal(defaultNiProviderConfig.ReconnectAttemptLimit, testNiProvConfig.ReconnectAttemptLimit);
        Assert.Equal(defaultNiProviderConfig.ReconnectMaxDelay, testNiProvConfig.ReconnectMaxDelay);
        Assert.Equal(defaultNiProviderConfig.ReconnectMinDelay, testNiProvConfig.ReconnectMinDelay);
        Assert.Equal(defaultNiProviderConfig.RecoverUserSubmitSourceDirectory, testNiProvConfig.RecoverUserSubmitSourceDirectory);
        Assert.Equal(defaultNiProviderConfig.RefreshFirstRequired, testNiProvConfig.RefreshFirstRequired);
        Assert.Equal(defaultNiProviderConfig.RemoveItemsOnDisconnect, testNiProvConfig.RemoveItemsOnDisconnect);
        Assert.Equal(defaultNiProviderConfig.RequestTimeout, testNiProvConfig.RequestTimeout);
        Assert.Equal(defaultNiProviderConfig.ServiceCountHint, testNiProvConfig.ServiceCountHint);
        // XML tracing block
        Assert.Equal(defaultNiProviderConfig.XmlTraceToStdout, testNiProvConfig.XmlTraceToStdout);
        Assert.Equal(defaultNiProviderConfig.XmlTraceToFile, testNiProvConfig.XmlTraceToFile);
        Assert.Equal(defaultNiProviderConfig.XmlTraceMaxFileSize, testNiProvConfig.XmlTraceMaxFileSize);
        Assert.Equal(defaultNiProviderConfig.XmlTraceFileName, testNiProvConfig.XmlTraceFileName);
        Assert.Equal(defaultNiProviderConfig.XmlTraceToMultipleFiles, testNiProvConfig.XmlTraceToMultipleFiles);
        Assert.Equal(defaultNiProviderConfig.XmlTraceWrite, testNiProvConfig.XmlTraceWrite);
        Assert.Equal(defaultNiProviderConfig.XmlTraceRead, testNiProvConfig.XmlTraceRead);
        Assert.Equal(defaultNiProviderConfig.XmlTracePing, testNiProvConfig.XmlTracePing);


        testChannelConfig = niProvConfigImpl.ClientChannelConfigMap["TestChannel_1"];

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
        testChannelConfig = niProvConfigImpl.ClientChannelConfigMap["TestChannel_2"];

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

        testLoggerConfig = niProvConfigImpl.LoggerConfigMap["TestLogger_1"];

        Assert.Equal("TestLogger_1", testLoggerConfig.Name);
        Assert.Equal(LoggerType.STDOUT, testLoggerConfig.LoggerType);
        Assert.Equal(LoggerLevel.INFO, testLoggerConfig.LoggerSeverity);
        Assert.Equal("testLogFile1", testLoggerConfig.FileName);
        Assert.Equal((ulong)1, testLoggerConfig.IncludeDateInLoggerOutput);
        Assert.Equal((ulong)10, testLoggerConfig.NumberOfLogFiles);
        Assert.Equal((ulong)20, testLoggerConfig.MaxLogFileSize);

        // TestLogger_2 is all defaults
        testLoggerConfig = niProvConfigImpl.LoggerConfigMap["TestLogger_2"];

        Assert.Equal("TestLogger_2", testLoggerConfig.Name);
        Assert.Equal(defaultLoggerConfig.LoggerType, testLoggerConfig.LoggerType);
        Assert.Equal(defaultLoggerConfig.LoggerSeverity, testLoggerConfig.LoggerSeverity);
        Assert.Equal(defaultLoggerConfig.FileName, testLoggerConfig.FileName);
        Assert.Equal(defaultLoggerConfig.IncludeDateInLoggerOutput, testLoggerConfig.IncludeDateInLoggerOutput);
        Assert.Equal(defaultLoggerConfig.NumberOfLogFiles, testLoggerConfig.NumberOfLogFiles);
        Assert.Equal(defaultLoggerConfig.MaxLogFileSize, testLoggerConfig.MaxLogFileSize);

        testDictConfig = niProvConfigImpl.DictionaryConfigMap["TestDictionary_1"];

        Assert.Equal("TestDictionary_1", testDictConfig.Name);
        Assert.Equal(EmaConfig.DictionaryTypeEnum.FILE, testDictConfig.DictionaryType);
        Assert.Equal("testEnumFile1", testDictConfig.EnumTypeDefFileName);
        Assert.Equal("testEnumItem1", testDictConfig.EnumTypeDefItemName);
        Assert.Equal("testRdmFile1", testDictConfig.RdmFieldDictionaryFileName);
        Assert.Equal("testRdmItem1", testDictConfig.RdmFieldDictionaryItemName);

        // TestDictionary_2 is set to defaults
        testDictConfig = niProvConfigImpl.DictionaryConfigMap["TestDictionary_2"];

        Assert.Equal("TestDictionary_2", testDictConfig.Name);
        Assert.Equal(defaultDictConfig.DictionaryType, testDictConfig.DictionaryType);
        Assert.Equal(defaultDictConfig.EnumTypeDefFileName, testDictConfig.EnumTypeDefFileName);
        Assert.Equal(defaultDictConfig.EnumTypeDefItemName, testDictConfig.EnumTypeDefItemName);
        Assert.Equal(defaultDictConfig.RdmFieldDictionaryFileName, testDictConfig.RdmFieldDictionaryFileName);
        Assert.Equal(defaultDictConfig.RdmFieldDictionaryItemName, testDictConfig.RdmFieldDictionaryItemName);

        // Verify the Directory configuration here
        // TestDictionary_2 is set to defaults
        testDirectoryConfig = niProvConfigImpl.DirectoryConfigMap["TestDirectory_1"];

        Assert.Equal("TestDirectory_1", testDirectoryConfig.Name);
        Assert.Equal(2, testDirectoryConfig.ServiceMap.Count);

        testServiceConfig = testDirectoryConfig.ServiceMap["TestService_1"];

        // Check the Info filter
        Assert.True(testServiceConfig.Service.HasInfo);
        Assert.Equal("TestService_1", testServiceConfig.Service.Info.ServiceName.ToString());
        Assert.Equal(10, testServiceConfig.Service.ServiceId);
        Assert.True(testServiceConfig.Service.Info.HasVendor);
        Assert.Equal("RTSDK", testServiceConfig.Service.Info.Vendor.ToString());
        Assert.True(testServiceConfig.Service.Info.HasIsSource);
        Assert.Equal(1, testServiceConfig.Service.Info.IsSource);
        Assert.Equal(2, testServiceConfig.Service.Info.CapabilitiesList.Count);
        Assert.Equal(EmaRdm.MMT_MARKET_PRICE, testServiceConfig.Service.Info.CapabilitiesList[0]);
        Assert.Equal(129, testServiceConfig.Service.Info.CapabilitiesList[1]);
        Assert.True(testServiceConfig.Service.Info.HasAcceptingConsStatus);
        Assert.Equal(0, testServiceConfig.Service.Info.AcceptConsumerStatus);
        Assert.True(testServiceConfig.Service.Info.HasItemList);
        Assert.Equal("Items#", testServiceConfig.Service.Info.ItemList.ToString());
        // The dictionaries provided and used in the service will be empty at this point.
        Assert.False(testServiceConfig.Service.Info.HasDictionariesProvided);
        Assert.Empty(testServiceConfig.Service.Info.DictionariesProvidedList);
        Assert.Equal(2, testServiceConfig.DictionariesProvidedList.Count);
        Assert.Equal("TestDictionary_1", testServiceConfig.DictionariesProvidedList[0]);
        Assert.Equal("TestDictionary_2", testServiceConfig.DictionariesProvidedList[1]);
        Assert.False(testServiceConfig.Service.Info.HasDictionariesUsed);
        Assert.Empty(testServiceConfig.Service.Info.DictionariesUsedList);
        Assert.Equal(2, testServiceConfig.DictionariesUsedList.Count);
        Assert.Equal("TestDictionary_1", testServiceConfig.DictionariesUsedList[0]);
        Assert.Equal("TestDictionary_2", testServiceConfig.DictionariesUsedList[1]);
        Assert.True(testServiceConfig.Service.Info.HasQos);
        Assert.Equal(2, testServiceConfig.Service.Info.QosList.Count);
        Assert.Equal(QosTimeliness.REALTIME, testServiceConfig.Service.Info.QosList[0].Timeliness());
        Assert.Equal(0, testServiceConfig.Service.Info.QosList[0].TimeInfo());
        Assert.Equal(QosRates.TICK_BY_TICK, testServiceConfig.Service.Info.QosList[0].Rate());
        Assert.Equal(0, testServiceConfig.Service.Info.QosList[0].RateInfo());
        Assert.Equal(QosTimeliness.DELAYED, testServiceConfig.Service.Info.QosList[1].Timeliness());
        Assert.Equal(10, testServiceConfig.Service.Info.QosList[1].TimeInfo());
        Assert.Equal(QosRates.TIME_CONFLATED, testServiceConfig.Service.Info.QosList[1].Rate());
        Assert.Equal(20, testServiceConfig.Service.Info.QosList[1].RateInfo());
        Assert.True(testServiceConfig.Service.Info.HasSupportQosRange);
        Assert.Equal(1, testServiceConfig.Service.Info.SupportsQosRange);
        Assert.True(testServiceConfig.Service.Info.HasSupportOOBSnapshots);
        Assert.Equal(1, testServiceConfig.Service.Info.SupportsOOBSnapshots);
        // Check the state filter
        Assert.True(testServiceConfig.Service.HasState);
        Assert.Equal(0, testServiceConfig.Service.State.ServiceStateVal);
        Assert.True(testServiceConfig.Service.State.HasAcceptingRequests);
        Assert.Equal(1, testServiceConfig.Service.State.AcceptingRequests);
        Assert.True(testServiceConfig.Service.State.HasStatus);
        Assert.Equal(StreamStates.CLOSED, testServiceConfig.Service.State.Status.StreamState());
        Assert.Equal(DataStates.SUSPECT, testServiceConfig.Service.State.Status.DataState());
        Assert.Equal(OmmState.StatusCodes.NOT_AUTHORIZED, testServiceConfig.Service.State.Status.Code());
        Assert.Equal("TestText", testServiceConfig.Service.State.Status.Text().ToString());
        // Check the load filter
        Assert.True(testServiceConfig.Service.HasLoad);
        Assert.True(testServiceConfig.Service.Load.HasOpenLimit);
        Assert.Equal(100, testServiceConfig.Service.Load.OpenLimit);
        Assert.True(testServiceConfig.Service.Load.HasOpenWindow);
        Assert.Equal(110, testServiceConfig.Service.Load.OpenWindow);
        Assert.True(testServiceConfig.Service.Load.HasLoadFactor);
        Assert.Equal(120, testServiceConfig.Service.Load.LoadFactor);

        testServiceConfig = testDirectoryConfig.ServiceMap["TestService_2"];

        // This is EMA defaults.
        // Check the Info filter
        Assert.True(testServiceConfig.Service.HasInfo);
        Assert.Equal("TestService_2", testServiceConfig.Service.Info.ServiceName.ToString());
        Assert.Equal(15, testServiceConfig.Service.ServiceId);
        Assert.False(testServiceConfig.Service.Info.HasVendor);
        Assert.True(testServiceConfig.Service.Info.HasIsSource);
        Assert.Equal(0, testServiceConfig.Service.Info.IsSource);
        Assert.Single(testServiceConfig.Service.Info.CapabilitiesList);
        Assert.Equal(EmaRdm.MMT_MARKET_PRICE, testServiceConfig.Service.Info.CapabilitiesList[0]);
        Assert.True(testServiceConfig.Service.Info.HasAcceptingConsStatus);
        Assert.Equal(1, testServiceConfig.Service.Info.AcceptConsumerStatus);
        Assert.False(testServiceConfig.Service.Info.HasItemList);
        // The dictionaries provided and used in the service will be empty at this point.
        Assert.False(testServiceConfig.Service.Info.HasDictionariesProvided);
        Assert.Empty(testServiceConfig.Service.Info.DictionariesProvidedList);
        Assert.Single(testServiceConfig.DictionariesProvidedList);
        Assert.Equal("TestDictionary_1", testServiceConfig.DictionariesProvidedList[0]);
        Assert.False(testServiceConfig.Service.Info.HasDictionariesUsed);
        Assert.Empty(testServiceConfig.Service.Info.DictionariesUsedList);
        Assert.Single(testServiceConfig.DictionariesUsedList);
        Assert.Equal("TestDictionary_1", testServiceConfig.DictionariesUsedList[0]);
        Assert.True(testServiceConfig.Service.Info.HasQos);
        Assert.Empty(testServiceConfig.Service.Info.QosList);
        Assert.True(testServiceConfig.Service.Info.HasSupportOOBSnapshots);
        Assert.Equal(0, testServiceConfig.Service.Info.SupportsOOBSnapshots);
        // Check the state filter
        Assert.True(testServiceConfig.Service.HasState);
        Assert.Equal(1, testServiceConfig.Service.State.ServiceStateVal);
        Assert.True(testServiceConfig.Service.State.HasAcceptingRequests);
        Assert.Equal(0, testServiceConfig.Service.State.AcceptingRequests);
        Assert.True(testServiceConfig.Service.State.HasStatus);
        Assert.Equal(StreamStates.OPEN, testServiceConfig.Service.State.Status.StreamState());
        Assert.Equal(DataStates.OK, testServiceConfig.Service.State.Status.DataState());
        Assert.Equal(OmmState.StatusCodes.NONE, testServiceConfig.Service.State.Status.Code());
        Assert.Equal(0, testServiceConfig.Service.State.Status.Text().Length);
        // Check the load filter
        Assert.True(testServiceConfig.Service.HasLoad);
        Assert.False(testServiceConfig.Service.Load.HasOpenLimit);
        Assert.False(testServiceConfig.Service.Load.HasOpenWindow);
        Assert.False(testServiceConfig.Service.Load.HasLoadFactor);

        // Verify that the config works here
        niProvConfigImpl.VerifyConfiguration();

        copiedConfig = new OmmNiProviderConfigImpl(niProvConfigImpl);

        // This config should have one niProvider, logger and dictionary, and 1 client channel.  The configured services in the choosen directory will be placed into the DirectoryCache.
        Assert.Single(copiedConfig.NiProviderConfigMap);
        Assert.Single(copiedConfig.ClientChannelConfigMap);
        Assert.Single(copiedConfig.LoggerConfigMap);
        Assert.Single(copiedConfig.DirectoryConfigMap);
        Assert.Equal(2, copiedConfig.DictionaryConfigMap.Count);
        Assert.NotNull(copiedConfig.DirectoryCache);
        Assert.Equal(2, copiedConfig.DirectoryCache!.DirectoryRefresh.ServiceList.Count);

        testNiProvConfig = copiedConfig.NiProviderConfigMap["TestNiProv_1"];

        Assert.Equal("TestNiProv_1", testNiProvConfig.Name);
        Assert.Single(testNiProvConfig.ChannelSet);
        Assert.Equal("TestChannel_1", testNiProvConfig.ChannelSet[0]);
        Assert.Equal("TestLogger_1", testNiProvConfig.Logger);
        Assert.Equal("TestDirectory_1", testNiProvConfig.Directory);
        Assert.Equal((long)30, testNiProvConfig.LoginRequestTimeOut);
        Assert.Equal(40, testNiProvConfig.DispatchTimeoutApiThread);
        Assert.Equal((ulong)50, testNiProvConfig.ItemCountHint);
        Assert.Equal((int)60, testNiProvConfig.MaxDispatchCountApiThread);
        Assert.Equal((int)70, testNiProvConfig.MaxDispatchCountUserThread);
        Assert.False(testNiProvConfig.MergeSourceDirectoryStreams);
        Assert.Equal(110, testNiProvConfig.ReconnectAttemptLimit);
        Assert.Equal(1400, testNiProvConfig.ReconnectMaxDelay);
        Assert.Equal(1300, testNiProvConfig.ReconnectMinDelay);
        Assert.False(testNiProvConfig.RecoverUserSubmitSourceDirectory);
        Assert.False(testNiProvConfig.RefreshFirstRequired);
        Assert.False(testNiProvConfig.RemoveItemsOnDisconnect);
        Assert.Equal((ulong)140, testNiProvConfig.RequestTimeout);
        Assert.Equal((int)160, testNiProvConfig.ServiceCountHint);
        // XML tracing block
        Assert.True(testNiProvConfig.XmlTraceToStdout);
        Assert.False(testNiProvConfig.XmlTraceToFile);
        Assert.Equal((ulong)100_000_000, testNiProvConfig.XmlTraceMaxFileSize);
        Assert.Equal("EmaTrace", testNiProvConfig.XmlTraceFileName);
        Assert.False(testNiProvConfig.XmlTraceToMultipleFiles);
        Assert.True(testNiProvConfig.XmlTraceWrite);
        Assert.True(testNiProvConfig.XmlTraceRead);
        Assert.False(testNiProvConfig.XmlTracePing);


        testChannelConfig = copiedConfig.ClientChannelConfigMap["TestChannel_1"];

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


        // Check the directory refresh services generated by the copy constructor from the config
        testService = copiedConfig.DirectoryCache!.GetService(10);

        Assert.NotNull(testService);

        // Check the Info filter
        Assert.True(testService!.HasInfo);
        Assert.Equal("TestService_1", testService!.Info.ServiceName.ToString());
        Assert.Equal(10, testService!.ServiceId);
        Assert.True(testService!.Info.HasVendor);
        Assert.Equal("RTSDK", testService!.Info.Vendor.ToString());
        Assert.True(testService!.Info.HasIsSource);
        Assert.Equal(1, testService!.Info.IsSource);
        Assert.Equal(2, testService!.Info.CapabilitiesList.Count);
        Assert.Equal(EmaRdm.MMT_MARKET_PRICE, testService!.Info.CapabilitiesList[0]);
        Assert.Equal(129, testService!.Info.CapabilitiesList[1]);
        Assert.True(testService!.Info.HasAcceptingConsStatus);
        Assert.Equal(0, testService!.Info.AcceptConsumerStatus);
        Assert.True(testService!.Info.HasItemList);
        Assert.Equal("Items#", testService!.Info.ItemList.ToString());
        // The dictionaries provided and used in the service will be empty at this point.
        Assert.True(testService!.Info.HasDictionariesProvided);
        Assert.Equal(4, testService!.Info.DictionariesProvidedList.Count);
        Assert.Contains("testRdmItem1", testService!.Info.DictionariesProvidedList);
        Assert.Contains("testEnumItem1", testService!.Info.DictionariesProvidedList);
        Assert.Contains(defaultDictConfig.EnumTypeDefItemName, testService!.Info.DictionariesProvidedList);
        Assert.Contains(defaultDictConfig.RdmFieldDictionaryItemName, testService!.Info.DictionariesProvidedList);
        Assert.True(testService!.Info.HasDictionariesUsed);
        Assert.Equal(4, testService!.Info.DictionariesUsedList.Count);
        Assert.Contains("testRdmItem1", testService!.Info.DictionariesUsedList);
        Assert.Contains("testEnumItem1", testService!.Info.DictionariesUsedList);
        Assert.Contains(defaultDictConfig.EnumTypeDefItemName, testService!.Info.DictionariesUsedList);
        Assert.Contains(defaultDictConfig.RdmFieldDictionaryItemName, testService!.Info.DictionariesUsedList);
        Assert.True(testService!.Info.HasQos);
        Assert.Equal(2, testService!.Info.QosList.Count);
        Assert.Equal(QosTimeliness.REALTIME, testService!.Info.QosList[0].Timeliness());
        Assert.Equal(0, testService!.Info.QosList[0].TimeInfo());
        Assert.Equal(QosRates.TICK_BY_TICK, testService!.Info.QosList[0].Rate());
        Assert.Equal(0, testService!.Info.QosList[0].RateInfo());
        Assert.Equal(QosTimeliness.DELAYED, testService!.Info.QosList[1].Timeliness());
        Assert.Equal(10, testService!.Info.QosList[1].TimeInfo());
        Assert.Equal(QosRates.TIME_CONFLATED, testService!.Info.QosList[1].Rate());
        Assert.Equal(20, testService!.Info.QosList[1].RateInfo());
        Assert.True(testService!.Info.HasSupportQosRange);
        Assert.Equal(1, testService!.Info.SupportsQosRange);
        Assert.True(testService!.Info.HasSupportOOBSnapshots);
        Assert.Equal(1, testService!.Info.SupportsOOBSnapshots);
        // Check the state filter
        Assert.True(testService!.HasState);
        Assert.Equal(0, testService!.State.ServiceStateVal);
        Assert.True(testService!.State.HasAcceptingRequests);
        Assert.Equal(1, testService!.State.AcceptingRequests);
        Assert.True(testService!.State.HasStatus);
        Assert.Equal(StreamStates.CLOSED, testService!.State.Status.StreamState());
        Assert.Equal(DataStates.SUSPECT, testService!.State.Status.DataState());
        Assert.Equal(OmmState.StatusCodes.NOT_AUTHORIZED, testService!.State.Status.Code());
        Assert.Equal("TestText", testService!.State.Status.Text().ToString());
        // Check the load filter
        Assert.True(testService!.HasLoad);
        Assert.True(testService!.Load.HasOpenLimit);
        Assert.Equal(100, testService!.Load.OpenLimit);
        Assert.True(testService!.Load.HasOpenWindow);
        Assert.Equal(110, testService!.Load.OpenWindow);
        Assert.True(testService!.Load.HasLoadFactor);
        Assert.Equal(120, testService!.Load.LoadFactor);

        testService = copiedConfig.DirectoryCache!.GetService(15);

        Assert.NotNull(testService);
        // This is EMA defaults.
        // Check the Info filter
        Assert.True(testService!.HasInfo);
        Assert.Equal("TestService_2", testService!.Info.ServiceName.ToString());
        Assert.Equal(15, testService!.ServiceId);
        Assert.False(testService!.Info.HasVendor);
        Assert.True(testService!.Info.HasIsSource);
        Assert.Equal(0, testService!.Info.IsSource);
        Assert.Single(testService!.Info.CapabilitiesList);
        Assert.Equal(EmaRdm.MMT_MARKET_PRICE, testService!.Info.CapabilitiesList[0]);
        Assert.True(testService!.Info.HasAcceptingConsStatus);
        Assert.Equal(1, testService!.Info.AcceptConsumerStatus);
        Assert.False(testService!.Info.HasItemList);
        // The dictionaries provided and used in the service will be empty at this point.
        Assert.True(testService!.Info.HasDictionariesUsed);
        Assert.Equal(2, testService!.Info.DictionariesUsedList.Count);
        Assert.Contains("testRdmItem1", testService!.Info.DictionariesUsedList);
        Assert.Contains("testEnumItem1", testService!.Info.DictionariesUsedList);
        Assert.True(testService!.Info.HasDictionariesProvided);
        Assert.Equal(2, testService!.Info.DictionariesProvidedList.Count);
        Assert.Contains("testRdmItem1", testService!.Info.DictionariesProvidedList);
        Assert.Contains("testEnumItem1", testService!.Info.DictionariesProvidedList);
        Assert.True(testService!.Info.HasQos);
        // Default QoS list
        Assert.NotNull(testService!.Info.QosList[0]);
        Assert.Equal(QosTimeliness.REALTIME, testService!.Info.QosList[0].Timeliness());
        Assert.Equal(0, testService!.Info.QosList[0].TimeInfo());
        Assert.Equal(QosRates.TICK_BY_TICK, testService!.Info.QosList[0].Rate());
        Assert.Equal(0, testService!.Info.QosList[0].RateInfo());
        Assert.True(testService!.Info.HasSupportOOBSnapshots);
        Assert.Equal(0, testService!.Info.SupportsOOBSnapshots);
        // Check the state filter
        Assert.True(testService!.HasState);
        Assert.Equal(1, testService!.State.ServiceStateVal);
        Assert.True(testService!.State.HasAcceptingRequests);
        Assert.Equal(0, testService!.State.AcceptingRequests);
        Assert.True(testService!.State.HasStatus);
        Assert.Equal(StreamStates.OPEN, testService!.State.Status.StreamState());
        Assert.Equal(DataStates.OK, testService!.State.Status.DataState());
        Assert.Equal(OmmState.StatusCodes.NONE, testService!.State.Status.Code());
        Assert.Equal(0, testService!.State.Status.Text().Length);
        // Check the load filter
        Assert.True(testService!.HasLoad);
        Assert.False(testService!.Load.HasOpenLimit);
        Assert.False(testService!.Load.HasOpenWindow);
        Assert.False(testService!.Load.HasLoadFactor);

        // Generate the role, check the values in it
        // Any channel-related ioctrl config will not be generated into the role.
        testRole = copiedConfig.GenerateNiProviderRole();

        Assert.NotNull(testRole.RdmLoginRequest);
        Assert.NotNull(testRole.RdmDirectoryRefresh);

        // Check the directory refresh services generated by the copy constructor from the config
        for (int index = 0; index < testRole.RdmDirectoryRefresh!.ServiceList.Count; index++)
        {
            if (testRole.RdmDirectoryRefresh.ServiceList[index].ServiceId == 10)
            {
                testService = testRole.RdmDirectoryRefresh.ServiceList[index];
            }
        }

        Assert.NotNull(testService);

        // Check the Info filter
        Assert.True(testService!.HasInfo);
        Assert.Equal("TestService_1", testService!.Info.ServiceName.ToString());
        Assert.Equal(10, testService!.ServiceId);
        Assert.True(testService!.Info.HasVendor);
        Assert.Equal("RTSDK", testService!.Info.Vendor.ToString());
        Assert.True(testService!.Info.HasIsSource);
        Assert.Equal(1, testService!.Info.IsSource);
        Assert.Equal(2, testService!.Info.CapabilitiesList.Count);
        Assert.Equal(EmaRdm.MMT_MARKET_PRICE, testService!.Info.CapabilitiesList[0]);
        Assert.Equal(129, testService!.Info.CapabilitiesList[1]);
        Assert.True(testService!.Info.HasAcceptingConsStatus);
        Assert.Equal(0, testService!.Info.AcceptConsumerStatus);
        Assert.True(testService!.Info.HasItemList);
        Assert.Equal("Items#", testService!.Info.ItemList.ToString());
        // The dictionaries provided and used in the service will be empty at this point.
        Assert.True(testService!.Info.HasDictionariesProvided);
        Assert.Equal(4, testService!.Info.DictionariesProvidedList.Count);
        Assert.Contains("testRdmItem1", testService!.Info.DictionariesProvidedList);
        Assert.Contains("testEnumItem1", testService!.Info.DictionariesProvidedList);
        Assert.Contains(defaultDictConfig.EnumTypeDefItemName, testService!.Info.DictionariesProvidedList);
        Assert.Contains(defaultDictConfig.RdmFieldDictionaryItemName, testService!.Info.DictionariesProvidedList);
        Assert.True(testService!.Info.HasDictionariesUsed);
        Assert.Equal(4, testService!.Info.DictionariesUsedList.Count);
        Assert.Contains("testRdmItem1", testService!.Info.DictionariesUsedList);
        Assert.Contains("testEnumItem1", testService!.Info.DictionariesUsedList);
        Assert.Contains(defaultDictConfig.EnumTypeDefItemName, testService!.Info.DictionariesUsedList);
        Assert.Contains(defaultDictConfig.RdmFieldDictionaryItemName, testService!.Info.DictionariesUsedList);
        Assert.True(testService!.Info.HasQos);
        Assert.Equal(2, testService!.Info.QosList.Count);
        Assert.Equal(QosTimeliness.REALTIME, testService!.Info.QosList[0].Timeliness());
        Assert.Equal(0, testService!.Info.QosList[0].TimeInfo());
        Assert.Equal(QosRates.TICK_BY_TICK, testService!.Info.QosList[0].Rate());
        Assert.Equal(0, testService!.Info.QosList[0].RateInfo());
        Assert.Equal(QosTimeliness.DELAYED, testService!.Info.QosList[1].Timeliness());
        Assert.Equal(10, testService!.Info.QosList[1].TimeInfo());
        Assert.Equal(QosRates.TIME_CONFLATED, testService!.Info.QosList[1].Rate());
        Assert.Equal(20, testService!.Info.QosList[1].RateInfo());
        Assert.True(testService!.Info.HasSupportQosRange);
        Assert.Equal(1, testService!.Info.SupportsQosRange);
        Assert.True(testService!.Info.HasSupportOOBSnapshots);
        Assert.Equal(1, testService!.Info.SupportsOOBSnapshots);
        // Check the state filter
        Assert.True(testService!.HasState);
        Assert.Equal(0, testService!.State.ServiceStateVal);
        Assert.True(testService!.State.HasAcceptingRequests);
        Assert.Equal(1, testService!.State.AcceptingRequests);
        Assert.True(testService!.State.HasStatus);
        Assert.Equal(StreamStates.CLOSED, testService!.State.Status.StreamState());
        Assert.Equal(DataStates.SUSPECT, testService!.State.Status.DataState());
        Assert.Equal(OmmState.StatusCodes.NOT_AUTHORIZED, testService!.State.Status.Code());
        Assert.Equal("TestText", testService!.State.Status.Text().ToString());
        // Check the load filter
        Assert.True(testService!.HasLoad);
        Assert.True(testService!.Load.HasOpenLimit);
        Assert.Equal(100, testService!.Load.OpenLimit);
        Assert.True(testService!.Load.HasOpenWindow);
        Assert.Equal(110, testService!.Load.OpenWindow);
        Assert.True(testService!.Load.HasLoadFactor);
        Assert.Equal(120, testService!.Load.LoadFactor);

        // Check the directory refresh services generated by the copy constructor from the config
        for (int index = 0; index < testRole.RdmDirectoryRefresh!.ServiceList.Count; index++)
        {
            if (testRole.RdmDirectoryRefresh.ServiceList[index].ServiceId == 15)
            {
                testService = testRole.RdmDirectoryRefresh.ServiceList[index];
            }
        }

        Assert.NotNull(testService);
        // This is EMA defaults.
        // Check the Info filter
        Assert.True(testService!.HasInfo);
        Assert.Equal("TestService_2", testService!.Info.ServiceName.ToString());
        Assert.Equal(15, testService!.ServiceId);
        Assert.False(testService!.Info.HasVendor);
        Assert.True(testService!.Info.HasIsSource);
        Assert.Equal(0, testService!.Info.IsSource);
        Assert.Single(testService!.Info.CapabilitiesList);
        Assert.Equal(EmaRdm.MMT_MARKET_PRICE, testService!.Info.CapabilitiesList[0]);
        Assert.True(testService!.Info.HasAcceptingConsStatus);
        Assert.Equal(1, testService!.Info.AcceptConsumerStatus);
        Assert.False(testService!.Info.HasItemList);
        // The dictionaries provided and used in the service will be empty at this point.
        Assert.True(testService!.Info.HasDictionariesUsed);
        Assert.Equal(2, testService!.Info.DictionariesUsedList.Count);
        Assert.Contains("testRdmItem1", testService!.Info.DictionariesUsedList);
        Assert.Contains("testEnumItem1", testService!.Info.DictionariesUsedList);
        Assert.True(testService!.Info.HasDictionariesProvided);
        Assert.Equal(2, testService!.Info.DictionariesProvidedList.Count);
        Assert.Contains("testRdmItem1", testService!.Info.DictionariesProvidedList);
        Assert.Contains("testEnumItem1", testService!.Info.DictionariesProvidedList);
        Assert.True(testService!.Info.HasQos);
        // Default QoS list
        Assert.NotNull(testService!.Info.QosList[0]);
        Assert.Equal(QosTimeliness.REALTIME, testService!.Info.QosList[0].Timeliness());
        Assert.Equal(0, testService!.Info.QosList[0].TimeInfo());
        Assert.Equal(QosRates.TICK_BY_TICK, testService!.Info.QosList[0].Rate());
        Assert.Equal(0, testService!.Info.QosList[0].RateInfo());
        Assert.True(testService!.Info.HasSupportOOBSnapshots);
        Assert.Equal(0, testService!.Info.SupportsOOBSnapshots);
        // Check the state filter
        Assert.True(testService!.HasState);
        Assert.Equal(1, testService!.State.ServiceStateVal);
        Assert.True(testService!.State.HasAcceptingRequests);
        Assert.Equal(0, testService!.State.AcceptingRequests);
        Assert.True(testService!.State.HasStatus);
        Assert.Equal(StreamStates.OPEN, testService!.State.Status.StreamState());
        Assert.Equal(DataStates.OK, testService!.State.Status.DataState());
        Assert.Equal(OmmState.StatusCodes.NONE, testService!.State.Status.Code());
        Assert.Equal(0, testService!.State.Status.Text().Length);
        // Check the load filter
        Assert.True(testService!.HasLoad);
        Assert.False(testService!.Load.HasOpenLimit);
        Assert.False(testService!.Load.HasOpenWindow);
        Assert.False(testService!.Load.HasLoadFactor);


        testConnOpts = copiedConfig.GenerateReactorConnectOpts();

        Assert.Single(testConnOpts.ConnectionList);
        testConnInfo = testConnOpts.ConnectionList[0];

        Assert.Equal(testConnOpts.GetReconnectAttemptLimit(), copiedConfig.NiProviderConfig.ReconnectAttemptLimit);
        Assert.Equal(testConnOpts.GetReconnectMinDelay(), copiedConfig.NiProviderConfig.ReconnectMinDelay);
        Assert.Equal(testConnOpts.GetReconnectMaxDelay(), copiedConfig.NiProviderConfig.ReconnectMaxDelay);


        Assert.Equal(Eta.Transports.ConnectionType.ENCRYPTED, testConnInfo.ConnectOptions.ConnectionType);
        Assert.Equal(Eta.Transports.ConnectionType.ENCRYPTED, testConnInfo.ConnectOptions.EncryptionOpts.EncryptedProtocol);
        Assert.Equal(60, testConnInfo.ConnectOptions.PingTimeout);
        Assert.True(testConnInfo.EnableSessionManagement);
        Assert.Equal(20, testConnInfo.ConnectOptions.GuaranteedOutputBuffers);
        Assert.Equal(30, testChannelConfig.HighWaterMark);
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
        Assert.Equal(25000, testConnInfo.ConnectOptions.EncryptionOpts.AuthenticationTimeout);

        // OmmConsumerConfig reuse and ConsumerName override test
        // Sets the consumer Name to TestConsumer_2, which has a channelSet containing TestChannel_1 and TestChannel_2
        // Also sets some dummy V2 credentials to make sure they're flowing through the API.
        niProviderConfig.ProviderName("TestNiProv_2");
        niProviderConfig.UserName("TestUserName");
        niProviderConfig.Password("TestPassword");
        niProviderConfig.ApplicationId("TestAppId");
        niProviderConfig.ProxyHost("TestProxyHostOverride");
        niProviderConfig.ProxyPort("TestProxyPortOverride");
        niProviderConfig.ProxyUserName("TestProxyUsernameOverride");
        niProviderConfig.ProxyPassword("testProxyPasswordOverride");

        // Verify that the config works here
        niProvConfigImpl.VerifyConfiguration();

        copiedConfig = new OmmNiProviderConfigImpl(niProvConfigImpl);

        // This config should only have one of each config type.
        Assert.Single(copiedConfig.NiProviderConfigMap);
        Assert.Equal(2, copiedConfig.ClientChannelConfigMap.Count);
        Assert.Single(copiedConfig.LoggerConfigMap);
        Assert.Equal(2, copiedConfig.DictionaryConfigMap.Count);
        Assert.NotNull(copiedConfig.DirectoryCache);
        Assert.Equal(2, copiedConfig.DirectoryCache!.DirectoryRefresh.ServiceList.Count);

        // Verify that none of the method-set values have changed
        Assert.Equal(string.Empty, copiedConfig.HostName);
        Assert.Equal(string.Empty, copiedConfig.Port);
        Assert.Equal("TestUserName", copiedConfig.UserName);
        Assert.Equal("TestPassword", copiedConfig.Password);
        Assert.Equal("TestAppId", copiedConfig.ApplicationId);
        Assert.Equal("TestProxyHostOverride", copiedConfig.ProxyHost);
        Assert.Equal("TestProxyPortOverride", copiedConfig.ProxyPort);
        Assert.Equal("testProxyPasswordOverride", copiedConfig.ProxyPassword);
        Assert.Equal("TestProxyUsernameOverride", copiedConfig.ProxyUserName);

        // TestConsumer_2 has all defaults except for ChannelSet.
        testNiProvConfig = copiedConfig.NiProviderConfigMap["TestNiProv_2"];

        Assert.Equal("TestNiProv_2", testNiProvConfig.Name);
        Assert.Equal(2, testNiProvConfig.ChannelSet.Count);
        Assert.Equal("TestChannel_1", testNiProvConfig.ChannelSet[0]);
        Assert.Equal("TestChannel_2", testNiProvConfig.ChannelSet[1]);
        Assert.Equal("DefaultEmaLogger", testNiProvConfig.Logger);
        Assert.Equal("TestDirectory_1", testNiProvConfig.Directory);
        Assert.Equal(defaultNiProviderConfig.LoginRequestTimeOut, testNiProvConfig.LoginRequestTimeOut);
        Assert.Equal(defaultNiProviderConfig.DispatchTimeoutApiThread, testNiProvConfig.DispatchTimeoutApiThread);
        Assert.Equal(defaultNiProviderConfig.ItemCountHint, testNiProvConfig.ItemCountHint);
        Assert.Equal(defaultNiProviderConfig.MaxDispatchCountApiThread, testNiProvConfig.MaxDispatchCountApiThread);
        Assert.Equal(defaultNiProviderConfig.MaxDispatchCountUserThread, testNiProvConfig.MaxDispatchCountUserThread);
        Assert.Equal(defaultNiProviderConfig.MergeSourceDirectoryStreams, testNiProvConfig.MergeSourceDirectoryStreams);
        Assert.Equal(defaultNiProviderConfig.ReconnectAttemptLimit, testNiProvConfig.ReconnectAttemptLimit);
        Assert.Equal(defaultNiProviderConfig.ReconnectMaxDelay, testNiProvConfig.ReconnectMaxDelay);
        Assert.Equal(defaultNiProviderConfig.ReconnectMinDelay, testNiProvConfig.ReconnectMinDelay);
        Assert.Equal(defaultNiProviderConfig.RecoverUserSubmitSourceDirectory, testNiProvConfig.RecoverUserSubmitSourceDirectory);
        Assert.Equal(defaultNiProviderConfig.RefreshFirstRequired, testNiProvConfig.RefreshFirstRequired);
        Assert.Equal(defaultNiProviderConfig.RemoveItemsOnDisconnect, testNiProvConfig.RemoveItemsOnDisconnect);
        Assert.Equal(defaultNiProviderConfig.RequestTimeout, testNiProvConfig.RequestTimeout);
        Assert.Equal(defaultNiProviderConfig.ServiceCountHint, testNiProvConfig.ServiceCountHint);
        // XML tracing block
        Assert.Equal(defaultNiProviderConfig.XmlTraceToStdout, testNiProvConfig.XmlTraceToStdout);
        Assert.Equal(defaultNiProviderConfig.XmlTraceToFile, testNiProvConfig.XmlTraceToFile);
        Assert.Equal(defaultNiProviderConfig.XmlTraceMaxFileSize, testNiProvConfig.XmlTraceMaxFileSize);
        Assert.Equal(defaultNiProviderConfig.XmlTraceFileName, testNiProvConfig.XmlTraceFileName);
        Assert.Equal(defaultNiProviderConfig.XmlTraceToMultipleFiles, testNiProvConfig.XmlTraceToMultipleFiles);
        Assert.Equal(defaultNiProviderConfig.XmlTraceWrite, testNiProvConfig.XmlTraceWrite);
        Assert.Equal(defaultNiProviderConfig.XmlTraceRead, testNiProvConfig.XmlTraceRead);
        Assert.Equal(defaultNiProviderConfig.XmlTracePing, testNiProvConfig.XmlTracePing);

        Assert.Same(testNiProvConfig, copiedConfig.NiProviderConfig);

        testChannelConfig = copiedConfig.ClientChannelConfigMap["TestChannel_1"];

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
        Assert.Equal("TestProxyHostOverride", testChannelConfig.ConnectInfo.ConnectOptions.ProxyOptions.ProxyHostName);
        Assert.Equal("TestProxyPortOverride", testChannelConfig.ConnectInfo.ConnectOptions.ProxyOptions.ProxyPort);
        Assert.Equal("testProxyPasswordOverride", testChannelConfig.ConnectInfo.ConnectOptions.ProxyOptions.ProxyPassword);
        Assert.Equal("TestProxyUsernameOverride", testChannelConfig.ConnectInfo.ConnectOptions.ProxyOptions.ProxyUserName);
        Assert.True(testChannelConfig.ConnectInfo.ConnectOptions.TcpOpts.TcpNoDelay);
        Assert.Equal(EncryptionProtocolFlags.ENC_TLSV1_2, testChannelConfig.ConnectInfo.ConnectOptions.EncryptionOpts.EncryptionProtocolFlags);
        Assert.Equal(25000, testChannelConfig.ConnectInfo.ConnectOptions.EncryptionOpts.AuthenticationTimeout);

        // TestChannel_2 is the default
        testChannelConfig = copiedConfig.ClientChannelConfigMap["TestChannel_2"];

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
        Assert.Equal("testProxyPasswordOverride", testChannelConfig.ConnectInfo.ConnectOptions.ProxyOptions.ProxyPassword);
        Assert.Equal("TestProxyUsernameOverride", testChannelConfig.ConnectInfo.ConnectOptions.ProxyOptions.ProxyUserName);
        Assert.Equal(defaultChannelConfig.ConnectInfo.ConnectOptions.TcpOpts.TcpNoDelay, testChannelConfig.ConnectInfo.ConnectOptions.TcpOpts.TcpNoDelay);
        Assert.Equal(defaultChannelConfig.ConnectInfo.ConnectOptions.EncryptionOpts.EncryptionProtocolFlags, testChannelConfig.ConnectInfo.ConnectOptions.EncryptionOpts.EncryptionProtocolFlags);
        Assert.False(defaultChannelConfig.CompressionThresholdSet);

        testService = copiedConfig.DirectoryCache!.GetService(15);

        Assert.NotNull(testService);
        // This is EMA defaults.
        // Check the Info filter
        Assert.True(testService!.HasInfo);
        Assert.Equal("TestService_2", testService!.Info.ServiceName.ToString());
        Assert.Equal(15, testService!.ServiceId);
        Assert.False(testService!.Info.HasVendor);
        Assert.True(testService!.Info.HasIsSource);
        Assert.Equal(0, testService!.Info.IsSource);
        Assert.Single(testService!.Info.CapabilitiesList);
        Assert.Equal(EmaRdm.MMT_MARKET_PRICE, testService!.Info.CapabilitiesList[0]);
        Assert.True(testService!.Info.HasAcceptingConsStatus);
        Assert.Equal(1, testService!.Info.AcceptConsumerStatus);
        Assert.False(testService!.Info.HasItemList);
        // The dictionaries provided and used in the service will be empty at this point.
        Assert.True(testService!.Info.HasDictionariesUsed);
        Assert.Equal(2, testService!.Info.DictionariesUsedList.Count);
        Assert.Contains("testRdmItem1", testService!.Info.DictionariesUsedList);
        Assert.Contains("testEnumItem1", testService!.Info.DictionariesUsedList);
        Assert.True(testService!.Info.HasDictionariesProvided);
        Assert.Equal(2, testService!.Info.DictionariesProvidedList.Count);
        Assert.Contains("testRdmItem1", testService!.Info.DictionariesProvidedList);
        Assert.Contains("testEnumItem1", testService!.Info.DictionariesProvidedList);
        Assert.True(testService!.Info.HasQos);
        // Default QoS list
        Assert.NotNull(testService!.Info.QosList[0]);
        Assert.Equal(QosTimeliness.REALTIME, testService!.Info.QosList[0].Timeliness());
        Assert.Equal(0, testService!.Info.QosList[0].TimeInfo());
        Assert.Equal(QosRates.TICK_BY_TICK, testService!.Info.QosList[0].Rate());
        Assert.Equal(0, testService!.Info.QosList[0].RateInfo());
        Assert.True(testService!.Info.HasSupportOOBSnapshots);
        Assert.Equal(0, testService!.Info.SupportsOOBSnapshots);
        // Check the state filter
        Assert.True(testService!.HasState);
        Assert.Equal(1, testService!.State.ServiceStateVal);
        Assert.True(testService!.State.HasAcceptingRequests);
        Assert.Equal(0, testService!.State.AcceptingRequests);
        Assert.True(testService!.State.HasStatus);
        Assert.Equal(StreamStates.OPEN, testService!.State.Status.StreamState());
        Assert.Equal(DataStates.OK, testService!.State.Status.DataState());
        Assert.Equal(OmmState.StatusCodes.NONE, testService!.State.Status.Code());
        Assert.Equal(0, testService!.State.Status.Text().Length);
        // Check the load filter
        Assert.True(testService!.HasLoad);
        Assert.False(testService!.Load.HasOpenLimit);
        Assert.False(testService!.Load.HasOpenWindow);
        Assert.False(testService!.Load.HasLoadFactor);

        // Check the directory refresh services generated by the copy constructor from the config
        testService = copiedConfig.DirectoryCache!.GetService(10);

        Assert.NotNull(testService);

        // Check the Info filter
        Assert.True(testService!.HasInfo);
        Assert.Equal("TestService_1", testService!.Info.ServiceName.ToString());
        Assert.Equal(10, testService!.ServiceId);
        Assert.True(testService!.Info.HasVendor);
        Assert.Equal("RTSDK", testService!.Info.Vendor.ToString());
        Assert.True(testService!.Info.HasIsSource);
        Assert.Equal(1, testService!.Info.IsSource);
        Assert.Equal(2, testService!.Info.CapabilitiesList.Count);
        Assert.Equal(EmaRdm.MMT_MARKET_PRICE, testService!.Info.CapabilitiesList[0]);
        Assert.Equal(129, testService!.Info.CapabilitiesList[1]);
        Assert.True(testService!.Info.HasAcceptingConsStatus);
        Assert.Equal(0, testService!.Info.AcceptConsumerStatus);
        Assert.True(testService!.Info.HasItemList);
        Assert.Equal("Items#", testService!.Info.ItemList.ToString());
        // The dictionaries provided and used in the service will be empty at this point.
        Assert.True(testService!.Info.HasDictionariesProvided);
        Assert.Equal(4, testService!.Info.DictionariesProvidedList.Count);
        Assert.Contains("testRdmItem1", testService!.Info.DictionariesProvidedList);
        Assert.Contains("testEnumItem1", testService!.Info.DictionariesProvidedList);
        Assert.Contains(defaultDictConfig.EnumTypeDefItemName, testService!.Info.DictionariesProvidedList);
        Assert.Contains(defaultDictConfig.RdmFieldDictionaryItemName, testService!.Info.DictionariesProvidedList);
        Assert.True(testService!.Info.HasDictionariesUsed);
        Assert.Equal(4, testService!.Info.DictionariesUsedList.Count);
        Assert.Contains("testRdmItem1", testService!.Info.DictionariesUsedList);
        Assert.Contains("testEnumItem1", testService!.Info.DictionariesUsedList);
        Assert.Contains(defaultDictConfig.EnumTypeDefItemName, testService!.Info.DictionariesUsedList);
        Assert.Contains(defaultDictConfig.RdmFieldDictionaryItemName, testService!.Info.DictionariesUsedList);
        Assert.True(testService!.Info.HasQos);
        Assert.Equal(2, testService!.Info.QosList.Count);
        Assert.Equal(QosTimeliness.REALTIME, testService!.Info.QosList[0].Timeliness());
        Assert.Equal(0, testService!.Info.QosList[0].TimeInfo());
        Assert.Equal(QosRates.TICK_BY_TICK, testService!.Info.QosList[0].Rate());
        Assert.Equal(0, testService!.Info.QosList[0].RateInfo());
        Assert.Equal(QosTimeliness.DELAYED, testService!.Info.QosList[1].Timeliness());
        Assert.Equal(10, testService!.Info.QosList[1].TimeInfo());
        Assert.Equal(QosRates.TIME_CONFLATED, testService!.Info.QosList[1].Rate());
        Assert.Equal(20, testService!.Info.QosList[1].RateInfo());
        Assert.True(testService!.Info.HasSupportQosRange);
        Assert.Equal(1, testService!.Info.SupportsQosRange);
        Assert.True(testService!.Info.HasSupportOOBSnapshots);
        Assert.Equal(1, testService!.Info.SupportsOOBSnapshots);
        // Check the state filter
        Assert.True(testService!.HasState);
        Assert.Equal(0, testService!.State.ServiceStateVal);
        Assert.True(testService!.State.HasAcceptingRequests);
        Assert.Equal(1, testService!.State.AcceptingRequests);
        Assert.True(testService!.State.HasStatus);
        Assert.Equal(StreamStates.CLOSED, testService!.State.Status.StreamState());
        Assert.Equal(DataStates.SUSPECT, testService!.State.Status.DataState());
        Assert.Equal(OmmState.StatusCodes.NOT_AUTHORIZED, testService!.State.Status.Code());
        Assert.Equal("TestText", testService!.State.Status.Text().ToString());
        // Check the load filter
        Assert.True(testService!.HasLoad);
        Assert.True(testService!.Load.HasOpenLimit);
        Assert.Equal(100, testService!.Load.OpenLimit);
        Assert.True(testService!.Load.HasOpenWindow);
        Assert.Equal(110, testService!.Load.OpenWindow);
        Assert.True(testService!.Load.HasLoadFactor);
        Assert.Equal(120, testService!.Load.LoadFactor);

        // Generate the role, check the values in it
        // Any channel-related ioctrl config will not be generated into the role.
        testRole = copiedConfig.GenerateNiProviderRole();

        Assert.NotNull(testRole.RdmLoginRequest);
        Assert.NotNull(testRole.RdmDirectoryRefresh);

        // Check the directory refresh services generated by the copy constructor from the config
        for (int index = 0; index < testRole.RdmDirectoryRefresh!.ServiceList.Count; index++)
        {
            if (testRole.RdmDirectoryRefresh.ServiceList[index].ServiceId == 10)
            {
                testService = testRole.RdmDirectoryRefresh.ServiceList[index];
            }
        }

        Assert.NotNull(testService);

        // Check the Info filter
        Assert.True(testService!.HasInfo);
        Assert.Equal("TestService_1", testService!.Info.ServiceName.ToString());
        Assert.Equal(10, testService!.ServiceId);
        Assert.True(testService!.Info.HasVendor);
        Assert.Equal("RTSDK", testService!.Info.Vendor.ToString());
        Assert.True(testService!.Info.HasIsSource);
        Assert.Equal(1, testService!.Info.IsSource);
        Assert.Equal(2, testService!.Info.CapabilitiesList.Count);
        Assert.Equal(EmaRdm.MMT_MARKET_PRICE, testService!.Info.CapabilitiesList[0]);
        Assert.Equal(129, testService!.Info.CapabilitiesList[1]);
        Assert.True(testService!.Info.HasAcceptingConsStatus);
        Assert.Equal(0, testService!.Info.AcceptConsumerStatus);
        Assert.True(testService!.Info.HasItemList);
        Assert.Equal("Items#", testService!.Info.ItemList.ToString());
        // The dictionaries provided and used in the service will be empty at this point.
        Assert.True(testService!.Info.HasDictionariesProvided);
        Assert.Equal(4, testService!.Info.DictionariesProvidedList.Count);
        Assert.Contains("testRdmItem1", testService!.Info.DictionariesProvidedList);
        Assert.Contains("testEnumItem1", testService!.Info.DictionariesProvidedList);
        Assert.Contains(defaultDictConfig.EnumTypeDefItemName, testService!.Info.DictionariesProvidedList);
        Assert.Contains(defaultDictConfig.RdmFieldDictionaryItemName, testService!.Info.DictionariesProvidedList);
        Assert.True(testService!.Info.HasDictionariesUsed);
        Assert.Equal(4, testService!.Info.DictionariesUsedList.Count);
        Assert.Contains("testRdmItem1", testService!.Info.DictionariesUsedList);
        Assert.Contains("testEnumItem1", testService!.Info.DictionariesUsedList);
        Assert.Contains(defaultDictConfig.EnumTypeDefItemName, testService!.Info.DictionariesUsedList);
        Assert.Contains(defaultDictConfig.RdmFieldDictionaryItemName, testService!.Info.DictionariesUsedList);
        Assert.True(testService!.Info.HasQos);
        Assert.Equal(2, testService!.Info.QosList.Count);
        Assert.Equal(QosTimeliness.REALTIME, testService!.Info.QosList[0].Timeliness());
        Assert.Equal(0, testService!.Info.QosList[0].TimeInfo());
        Assert.Equal(QosRates.TICK_BY_TICK, testService!.Info.QosList[0].Rate());
        Assert.Equal(0, testService!.Info.QosList[0].RateInfo());
        Assert.Equal(QosTimeliness.DELAYED, testService!.Info.QosList[1].Timeliness());
        Assert.Equal(10, testService!.Info.QosList[1].TimeInfo());
        Assert.Equal(QosRates.TIME_CONFLATED, testService!.Info.QosList[1].Rate());
        Assert.Equal(20, testService!.Info.QosList[1].RateInfo());
        Assert.True(testService!.Info.HasSupportQosRange);
        Assert.Equal(1, testService!.Info.SupportsQosRange);
        Assert.True(testService!.Info.HasSupportOOBSnapshots);
        Assert.Equal(1, testService!.Info.SupportsOOBSnapshots);
        // Check the state filter
        Assert.True(testService!.HasState);
        Assert.Equal(0, testService!.State.ServiceStateVal);
        Assert.True(testService!.State.HasAcceptingRequests);
        Assert.Equal(1, testService!.State.AcceptingRequests);
        Assert.True(testService!.State.HasStatus);
        Assert.Equal(StreamStates.CLOSED, testService!.State.Status.StreamState());
        Assert.Equal(DataStates.SUSPECT, testService!.State.Status.DataState());
        Assert.Equal(OmmState.StatusCodes.NOT_AUTHORIZED, testService!.State.Status.Code());
        Assert.Equal("TestText", testService!.State.Status.Text().ToString());
        // Check the load filter
        Assert.True(testService!.HasLoad);
        Assert.True(testService!.Load.HasOpenLimit);
        Assert.Equal(100, testService!.Load.OpenLimit);
        Assert.True(testService!.Load.HasOpenWindow);
        Assert.Equal(110, testService!.Load.OpenWindow);
        Assert.True(testService!.Load.HasLoadFactor);
        Assert.Equal(120, testService!.Load.LoadFactor);

        // Check the directory refresh services generated by the copy constructor from the config
        for (int index = 0; index < testRole.RdmDirectoryRefresh!.ServiceList.Count; index++)
        {
            if (testRole.RdmDirectoryRefresh.ServiceList[index].ServiceId == 15)
            {
                testService = testRole.RdmDirectoryRefresh.ServiceList[index];
            }
        }

        Assert.NotNull(testService);
        // This is EMA defaults.
        // Check the Info filter
        Assert.True(testService!.HasInfo);
        Assert.Equal("TestService_2", testService!.Info.ServiceName.ToString());
        Assert.Equal(15, testService!.ServiceId);
        Assert.False(testService!.Info.HasVendor);
        Assert.True(testService!.Info.HasIsSource);
        Assert.Equal(0, testService!.Info.IsSource);
        Assert.Single(testService!.Info.CapabilitiesList);
        Assert.Equal(EmaRdm.MMT_MARKET_PRICE, testService!.Info.CapabilitiesList[0]);
        Assert.True(testService!.Info.HasAcceptingConsStatus);
        Assert.Equal(1, testService!.Info.AcceptConsumerStatus);
        Assert.False(testService!.Info.HasItemList);
        // The dictionaries provided and used in the service will be empty at this point.
        Assert.True(testService!.Info.HasDictionariesUsed);
        Assert.Equal(2, testService!.Info.DictionariesUsedList.Count);
        Assert.Contains("testRdmItem1", testService!.Info.DictionariesUsedList);
        Assert.Contains("testEnumItem1", testService!.Info.DictionariesUsedList);
        Assert.True(testService!.Info.HasDictionariesProvided);
        Assert.Equal(2, testService!.Info.DictionariesProvidedList.Count);
        Assert.Contains("testRdmItem1", testService!.Info.DictionariesProvidedList);
        Assert.Contains("testEnumItem1", testService!.Info.DictionariesProvidedList);
        Assert.True(testService!.Info.HasQos);
        // Default QoS list
        Assert.NotNull(testService!.Info.QosList[0]);
        Assert.Equal(QosTimeliness.REALTIME, testService!.Info.QosList[0].Timeliness());
        Assert.Equal(0, testService!.Info.QosList[0].TimeInfo());
        Assert.Equal(QosRates.TICK_BY_TICK, testService!.Info.QosList[0].Rate());
        Assert.Equal(0, testService!.Info.QosList[0].RateInfo());
        Assert.True(testService!.Info.HasSupportOOBSnapshots);
        Assert.Equal(0, testService!.Info.SupportsOOBSnapshots);
        // Check the state filter
        Assert.True(testService!.HasState);
        Assert.Equal(1, testService!.State.ServiceStateVal);
        Assert.True(testService!.State.HasAcceptingRequests);
        Assert.Equal(0, testService!.State.AcceptingRequests);
        Assert.True(testService!.State.HasStatus);
        Assert.Equal(StreamStates.OPEN, testService!.State.Status.StreamState());
        Assert.Equal(DataStates.OK, testService!.State.Status.DataState());
        Assert.Equal(OmmState.StatusCodes.NONE, testService!.State.Status.Code());
        Assert.Equal(0, testService!.State.Status.Text().Length);
        // Check the load filter
        Assert.True(testService!.HasLoad);
        Assert.False(testService!.Load.HasOpenLimit);
        Assert.False(testService!.Load.HasOpenWindow);
        Assert.False(testService!.Load.HasLoadFactor);

        // Generate the connection options, verify that the channel set is correct.
        testConnOpts = copiedConfig.GenerateReactorConnectOpts();

        Assert.Equal(2, testConnOpts.ConnectionList.Count);
        testConnInfo = testConnOpts.ConnectionList[0];

        Assert.Equal(testConnOpts.GetReconnectAttemptLimit(), copiedConfig.NiProviderConfig.ReconnectAttemptLimit);
        Assert.Equal(testConnOpts.GetReconnectMinDelay(), copiedConfig.NiProviderConfig.ReconnectMinDelay);
        Assert.Equal(testConnOpts.GetReconnectMaxDelay(), copiedConfig.NiProviderConfig.ReconnectMaxDelay);


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

        // OmmConsumerConfig reuse and ConsumerName override test
        // Sets the consumer Name to TestConsumer_2, which has a channelSet containing TestChannel_1 and TestChannel_2
        // Also sets some dummy V2 credentials to make sure they're flowing through the API.
        niProviderConfig.ProviderName("TestNiProv_2");
        niProviderConfig.Host("TestHostOverride:TestPortOverride");
        niProviderConfig.UserName("TestUserName");
        niProviderConfig.Password("TestPassword");
        niProviderConfig.ApplicationId("TestAppId");
        niProviderConfig.ProxyHost("TestProxyHostOverride");
        niProviderConfig.ProxyPort("TestProxyPortOverride");
        niProviderConfig.ProxyUserName("TestProxyUsernameOverride");
        niProviderConfig.ProxyPassword("testProxyPasswordOverride");
        niProviderConfig.AdminControlDirectory(OmmNiProviderConfig.AdminControlMode.USER_CONTROL);

        // Verify that the config works here
        niProvConfigImpl.VerifyConfiguration();

        copiedConfig = new OmmNiProviderConfigImpl(niProvConfigImpl);

        // This config should only have one of each config type.
        Assert.Single(copiedConfig.NiProviderConfigMap);
        Assert.Single(copiedConfig.ClientChannelConfigMap);
        Assert.Single(copiedConfig.LoggerConfigMap);
        Assert.Empty(copiedConfig.DictionaryConfigMap);
        Assert.NotNull(copiedConfig.DirectoryCache);
        Assert.Empty(copiedConfig.DirectoryCache!.DirectoryRefresh.ServiceList);

        // Verify that none of the method-set values have changed
        Assert.Equal("TestHostOverride", copiedConfig.HostName);
        Assert.Equal("TestPortOverride", copiedConfig.Port);
        Assert.Equal("TestUserName", copiedConfig.UserName);
        Assert.Equal("TestPassword", copiedConfig.Password);
        Assert.Equal("TestAppId", copiedConfig.ApplicationId);
        Assert.Equal("TestProxyHostOverride", copiedConfig.ProxyHost);
        Assert.Equal("TestProxyPortOverride", copiedConfig.ProxyPort);
        Assert.Equal("testProxyPasswordOverride", copiedConfig.ProxyPassword);
        Assert.Equal("TestProxyUsernameOverride", copiedConfig.ProxyUserName);

        // TestConsumer_2 has all defaults except for ChannelSet.
        testNiProvConfig = copiedConfig.NiProviderConfigMap["TestNiProv_2"];

        Assert.Equal("TestNiProv_2", testNiProvConfig.Name);
        Assert.Single(testNiProvConfig.ChannelSet);
        Assert.Equal("TestChannel_1", testNiProvConfig.ChannelSet[0]);
        Assert.Equal(defaultNiProviderConfig.Directory, testNiProvConfig.Directory);
        Assert.Equal(defaultNiProviderConfig.LoginRequestTimeOut, testNiProvConfig.LoginRequestTimeOut);
        Assert.Equal(defaultNiProviderConfig.DispatchTimeoutApiThread, testNiProvConfig.DispatchTimeoutApiThread);
        Assert.Equal(defaultNiProviderConfig.ItemCountHint, testNiProvConfig.ItemCountHint);
        Assert.Equal(defaultNiProviderConfig.MaxDispatchCountApiThread, testNiProvConfig.MaxDispatchCountApiThread);
        Assert.Equal(defaultNiProviderConfig.MaxDispatchCountUserThread, testNiProvConfig.MaxDispatchCountUserThread);
        Assert.Equal(defaultNiProviderConfig.MergeSourceDirectoryStreams, testNiProvConfig.MergeSourceDirectoryStreams);
        Assert.Equal(defaultNiProviderConfig.ReconnectAttemptLimit, testNiProvConfig.ReconnectAttemptLimit);
        Assert.Equal(defaultNiProviderConfig.ReconnectMaxDelay, testNiProvConfig.ReconnectMaxDelay);
        Assert.Equal(defaultNiProviderConfig.ReconnectMinDelay, testNiProvConfig.ReconnectMinDelay);
        Assert.Equal(defaultNiProviderConfig.RecoverUserSubmitSourceDirectory, testNiProvConfig.RecoverUserSubmitSourceDirectory);
        Assert.Equal(defaultNiProviderConfig.RefreshFirstRequired, testNiProvConfig.RefreshFirstRequired);
        Assert.Equal(defaultNiProviderConfig.RemoveItemsOnDisconnect, testNiProvConfig.RemoveItemsOnDisconnect);
        Assert.Equal(defaultNiProviderConfig.RequestTimeout, testNiProvConfig.RequestTimeout);
        Assert.Equal(defaultNiProviderConfig.ServiceCountHint, testNiProvConfig.ServiceCountHint);
        // XML tracing block
        Assert.Equal(defaultNiProviderConfig.XmlTraceToStdout, testNiProvConfig.XmlTraceToStdout);
        Assert.Equal(defaultNiProviderConfig.XmlTraceToFile, testNiProvConfig.XmlTraceToFile);
        Assert.Equal(defaultNiProviderConfig.XmlTraceMaxFileSize, testNiProvConfig.XmlTraceMaxFileSize);
        Assert.Equal(defaultNiProviderConfig.XmlTraceFileName, testNiProvConfig.XmlTraceFileName);
        Assert.Equal(defaultNiProviderConfig.XmlTraceToMultipleFiles, testNiProvConfig.XmlTraceToMultipleFiles);
        Assert.Equal(defaultNiProviderConfig.XmlTraceWrite, testNiProvConfig.XmlTraceWrite);
        Assert.Equal(defaultNiProviderConfig.XmlTraceRead, testNiProvConfig.XmlTraceRead);
        Assert.Equal(defaultNiProviderConfig.XmlTracePing, testNiProvConfig.XmlTracePing);
        Assert.Same(testNiProvConfig, copiedConfig.NiProviderConfig);

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
        Assert.True(testChannelConfig.CompressionThresholdSet);
        Assert.Equal(555, testChannelConfig.CompressionThreshold);
        Assert.True(testChannelConfig.DirectWrite);
        Assert.Equal("TestHostOverride", testChannelConfig.ConnectInfo.ConnectOptions.UnifiedNetworkInfo.Address);
        Assert.Equal("TestPortOverride", testChannelConfig.ConnectInfo.ConnectOptions.UnifiedNetworkInfo.ServiceName);
        Assert.Equal("TestProxyHostOverride", testChannelConfig.ConnectInfo.ConnectOptions.ProxyOptions.ProxyHostName);
        Assert.Equal("TestProxyPortOverride", testChannelConfig.ConnectInfo.ConnectOptions.ProxyOptions.ProxyPort);
        Assert.Equal("testProxyPasswordOverride", testChannelConfig.ConnectInfo.ConnectOptions.ProxyOptions.ProxyPassword);
        Assert.Equal("TestProxyUsernameOverride", testChannelConfig.ConnectInfo.ConnectOptions.ProxyOptions.ProxyUserName);
        Assert.True(testChannelConfig.ConnectInfo.ConnectOptions.TcpOpts.TcpNoDelay);
        Assert.Equal(EncryptionProtocolFlags.ENC_TLSV1_2, testChannelConfig.ConnectInfo.ConnectOptions.EncryptionOpts.EncryptionProtocolFlags);
        Assert.Equal(25000, testChannelConfig.ConnectInfo.ConnectOptions.EncryptionOpts.AuthenticationTimeout);


        // Generate the role, check the values in it
        // Any channel-related ioctrl config will not be generated into the role.
        testRole = copiedConfig.GenerateNiProviderRole();

        Assert.NotNull(testRole.RdmLoginRequest);
        Assert.Null(testRole.RdmDirectoryRefresh);

        // Generate the connection options, verify that the channel set is correct.
        testConnOpts = copiedConfig.GenerateReactorConnectOpts();

        Assert.Single(testConnOpts.ConnectionList);
        testConnInfo = testConnOpts.ConnectionList[0];

        Assert.Equal(testConnOpts.GetReconnectAttemptLimit(), copiedConfig.NiProviderConfig.ReconnectAttemptLimit);
        Assert.Equal(testConnOpts.GetReconnectMinDelay(), copiedConfig.NiProviderConfig.ReconnectMinDelay);
        Assert.Equal(testConnOpts.GetReconnectMaxDelay(), copiedConfig.NiProviderConfig.ReconnectMaxDelay);


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
        Assert.Equal("TestProxyUsernameOverride", testChannelConfig.ConnectInfo.ConnectOptions.ProxyOptions.ProxyUserName);
        Assert.Equal("testProxyPasswordOverride", testChannelConfig.ConnectInfo.ConnectOptions.ProxyOptions.ProxyPassword);
        Assert.True(testConnInfo.ConnectOptions.TcpOpts.TcpNoDelay);
        Assert.Equal(EncryptionProtocolFlags.ENC_TLSV1_2, testConnInfo.ConnectOptions.EncryptionOpts.EncryptionProtocolFlags);
    }

    // This test will a blank config and verify that a default-only generated config is created.
    [Fact]
    public void NiProviderDefaultConfigTest()
    {
        OmmNiProviderConfig niProviderConfig;
        NiProviderConfig testNiProvConfig;
        ClientChannelConfig testChannelConfig;
        LoggerConfig testLoggerConfig;
        OmmNiProviderConfigImpl copiedConfig;
        Service? testService;
        DirectoryConfig testDirectoryConfig;
        EmaServiceConfig testServiceConfig;
        DictionaryConfig testDictionaryConfig;

        // Load a blank config so we can be sure that everything added is from the programmtic config
        // This doesn't use the default behavior(loading EmaConfig.xml) because an EmaConfig.xml file may be present in the directory for other example apps
        niProviderConfig = new OmmNiProviderConfig("../../../OmmConfigTests/EmaBlankConfig.xml");

        OmmNiProviderConfigImpl niProvConfigImpl = niProviderConfig.OmmNiProvConfigImpl;

        // Loaded the Config, now make sure everything's in it.

        // There should be nothing in here
        Assert.Empty(niProvConfigImpl.NiProviderConfigMap);
        Assert.Empty(niProvConfigImpl.ClientChannelConfigMap);
        Assert.Empty(niProvConfigImpl.LoggerConfigMap);
        Assert.Empty(niProvConfigImpl.DictionaryConfigMap);

        // Verify should succeed
        niProvConfigImpl.VerifyConfiguration();

        copiedConfig = new OmmNiProviderConfigImpl(niProvConfigImpl);

        // There should be one default-generated consumer, and one default-generated channel in the maps
        // Logger and Dictionary are set in copiedConfig.Logger and copiedConfig.Dictionary, respectively
        Assert.Single(copiedConfig.NiProviderConfigMap);
        Assert.Single(copiedConfig.ClientChannelConfigMap);
        Assert.Single(copiedConfig.LoggerConfigMap);
        Assert.Single(copiedConfig.DictionaryConfigMap);
        Assert.NotNull(copiedConfig.DirectoryCache);
        Assert.Single(copiedConfig.DirectoryCache!.DirectoryRefresh.ServiceList);

        // DefaultEmaConsumer has all defaults and generated names
        testNiProvConfig = copiedConfig.NiProviderConfigMap["DefaultEmaNiProvider"];

        Assert.Same(copiedConfig.NiProviderConfig, copiedConfig.NiProviderConfigMap["DefaultEmaNiProvider"]);
        Assert.Equal(defaultNiProviderConfig.Directory, testNiProvConfig.Directory);
        Assert.Equal(defaultNiProviderConfig.LoginRequestTimeOut, testNiProvConfig.LoginRequestTimeOut);
        Assert.Equal(defaultNiProviderConfig.DispatchTimeoutApiThread, testNiProvConfig.DispatchTimeoutApiThread);
        Assert.Equal(defaultNiProviderConfig.ItemCountHint, testNiProvConfig.ItemCountHint);
        Assert.Equal(defaultNiProviderConfig.MaxDispatchCountApiThread, testNiProvConfig.MaxDispatchCountApiThread);
        Assert.Equal(defaultNiProviderConfig.MaxDispatchCountUserThread, testNiProvConfig.MaxDispatchCountUserThread);
        Assert.Equal(defaultNiProviderConfig.MergeSourceDirectoryStreams, testNiProvConfig.MergeSourceDirectoryStreams);
        Assert.Equal(defaultNiProviderConfig.ReconnectAttemptLimit, testNiProvConfig.ReconnectAttemptLimit);
        Assert.Equal(defaultNiProviderConfig.ReconnectMaxDelay, testNiProvConfig.ReconnectMaxDelay);
        Assert.Equal(defaultNiProviderConfig.ReconnectMinDelay, testNiProvConfig.ReconnectMinDelay);
        Assert.Equal(defaultNiProviderConfig.RecoverUserSubmitSourceDirectory, testNiProvConfig.RecoverUserSubmitSourceDirectory);
        Assert.Equal(defaultNiProviderConfig.RefreshFirstRequired, testNiProvConfig.RefreshFirstRequired);
        Assert.Equal(defaultNiProviderConfig.RemoveItemsOnDisconnect, testNiProvConfig.RemoveItemsOnDisconnect);
        Assert.Equal(defaultNiProviderConfig.RequestTimeout, testNiProvConfig.RequestTimeout);
        Assert.Equal(defaultNiProviderConfig.ServiceCountHint, testNiProvConfig.ServiceCountHint);
        // XML tracing block
        Assert.Equal(defaultNiProviderConfig.XmlTraceToStdout, testNiProvConfig.XmlTraceToStdout);
        Assert.Equal(defaultNiProviderConfig.XmlTraceToFile, testNiProvConfig.XmlTraceToFile);
        Assert.Equal(defaultNiProviderConfig.XmlTraceMaxFileSize, testNiProvConfig.XmlTraceMaxFileSize);
        Assert.Equal(defaultNiProviderConfig.XmlTraceFileName, testNiProvConfig.XmlTraceFileName);
        Assert.Equal(defaultNiProviderConfig.XmlTraceToMultipleFiles, testNiProvConfig.XmlTraceToMultipleFiles);
        Assert.Equal(defaultNiProviderConfig.XmlTraceWrite, testNiProvConfig.XmlTraceWrite);
        Assert.Equal(defaultNiProviderConfig.XmlTraceRead, testNiProvConfig.XmlTraceRead);
        Assert.Equal(defaultNiProviderConfig.XmlTracePing, testNiProvConfig.XmlTracePing);


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
        Assert.Equal("14003", testChannelConfig.ConnectInfo.ConnectOptions.UnifiedNetworkInfo.ServiceName);
        Assert.Equal(defaultChannelConfig.ConnectInfo.ConnectOptions.GuaranteedOutputBuffers, testChannelConfig.ConnectInfo.ConnectOptions.GuaranteedOutputBuffers);
        Assert.Equal(defaultChannelConfig.ConnectInfo.ConnectOptions.ProxyOptions.ProxyHostName, testChannelConfig.ConnectInfo.ConnectOptions.ProxyOptions.ProxyHostName);
        Assert.Equal(defaultChannelConfig.ConnectInfo.ConnectOptions.ProxyOptions.ProxyPort, testChannelConfig.ConnectInfo.ConnectOptions.ProxyOptions.ProxyPort);
        Assert.Equal(defaultChannelConfig.ConnectInfo.ConnectOptions.TcpOpts.TcpNoDelay, testChannelConfig.ConnectInfo.ConnectOptions.TcpOpts.TcpNoDelay);
        Assert.Equal(defaultChannelConfig.ConnectInfo.ConnectOptions.EncryptionOpts.EncryptionProtocolFlags, testChannelConfig.ConnectInfo.ConnectOptions.EncryptionOpts.EncryptionProtocolFlags);

        // The logger will be generated and contain all defaults
        testLoggerConfig = copiedConfig.LoggerConfigMap["DefaultEmaLogger"];

        Assert.Same(copiedConfig.LoggerConfig, copiedConfig.LoggerConfigMap["DefaultEmaLogger"]);
        Assert.Equal("DefaultEmaLogger", testLoggerConfig.Name);
        Assert.Equal(defaultLoggerConfig.LoggerType, testLoggerConfig.LoggerType);
        Assert.Equal(defaultLoggerConfig.LoggerSeverity, testLoggerConfig.LoggerSeverity);
        Assert.Equal(defaultLoggerConfig.FileName, testLoggerConfig.FileName);
        Assert.Equal(defaultLoggerConfig.IncludeDateInLoggerOutput, testLoggerConfig.IncludeDateInLoggerOutput);
        Assert.Equal(defaultLoggerConfig.NumberOfLogFiles, testLoggerConfig.NumberOfLogFiles);
        Assert.Equal(defaultLoggerConfig.MaxLogFileSize, testLoggerConfig.MaxLogFileSize);

        testService = copiedConfig.DirectoryCache.GetService(0);

        Assert.NotNull(testService);

        Assert.Equal("NI_PUB", testService!.Info.ServiceName.ToString());
        Assert.Equal(0, testService!.ServiceId);
        Assert.True(testService!.HasInfo);
        Assert.True(testService!.HasState);
        Assert.True(testService!.State.HasAcceptingRequests);
        Assert.Equal(0, testService!.State.AcceptingRequests);
        Assert.True(testService!.Info.HasSupportQosRange);
        Assert.Equal(0, testService!.Info.SupportsQosRange);
        Assert.Equal(MapEntryActions.ADD, testService!.Action);
        Assert.Equal(FilterEntryActions.SET, testService!.Info.Action);
        Assert.True(testService!.Info.HasVendor);
        Assert.True(string.IsNullOrEmpty(testService!.Info.Vendor.ToString()));
        Assert.True(testService!.Info.HasIsSource);
        Assert.Equal(0, testService!.Info.IsSource);
        Assert.Contains(Ema.Rdm.EmaRdm.MMT_MARKET_PRICE, testService!.Info.CapabilitiesList);
        Assert.Contains(Ema.Rdm.EmaRdm.MMT_MARKET_BY_ORDER, testService!.Info.CapabilitiesList);
        Assert.Contains(Ema.Rdm.EmaRdm.MMT_MARKET_BY_PRICE, testService!.Info.CapabilitiesList);
        Assert.Contains(Ema.Rdm.EmaRdm.MMT_MARKET_MAKER, testService!.Info.CapabilitiesList);
        Assert.True(testService!.Info.HasQos);
        Assert.Single(testService!.Info.QosList);
        Assert.Equal(QosRates.TICK_BY_TICK, testService!.Info.QosList[0].Rate());
        Assert.Equal(QosTimeliness.REALTIME, testService!.Info.QosList[0].Timeliness());
        Assert.True(testService!.Info.HasDictionariesUsed);
        Assert.Equal(2, testService!.Info.DictionariesUsedList.Count);
        Assert.Contains(DictionaryCallbackClient<IOmmProviderClient>.DICTIONARY_RWFFID, testService!.Info.DictionariesUsedList);
        Assert.Contains(DictionaryCallbackClient<IOmmProviderClient>.DICTIONARY_RWFENUM, testService!.Info.DictionariesUsedList);
        Assert.True(testService!.Info.HasItemList);
        Assert.True(string.IsNullOrEmpty(testService!.Info.ItemList.ToString()));
        Assert.True(testService!.Info.HasAcceptingConsStatus);
        Assert.Equal(1, testService!.Info.AcceptConsumerStatus);
        Assert.True(testService!.Info.HasSupportOOBSnapshots);
        Assert.Equal(0, testService!.Info.SupportsOOBSnapshots);
        Assert.Equal(FilterEntryActions.SET, testService!.State.Action);
        Assert.Equal(1, testService!.State.ServiceStateVal);

        Assert.Equal(Directory.ServiceFilterFlags.INFO | Directory.ServiceFilterFlags.STATE, copiedConfig.DirectoryCache.DirectoryRefresh.Filter);

        // Get the directoryconfig for DefaultEmaDirectory
        testDirectoryConfig = copiedConfig.DirectoryConfigMap["DefaultEmaDirectory"];

        Assert.Single(testDirectoryConfig.ServiceMap);

        testServiceConfig = testDirectoryConfig.ServiceMap["NI_PUB"];

        // This should be the same object as above.
        Assert.Same(testService, testServiceConfig.Service);
        Assert.Contains("DefaultEmaDictionary", testServiceConfig.DictionariesUsedList);
        Assert.Empty(testServiceConfig.DictionariesProvidedList);

        testDictionaryConfig = copiedConfig.DictionaryConfigMap["DefaultEmaDictionary"];

        Assert.Equal("./enumtype.def", testDictionaryConfig.EnumTypeDefFileName);
        Assert.Equal("RWFEnum", testDictionaryConfig.EnumTypeDefItemName);
        Assert.Equal("./RDMFieldDictionary", testDictionaryConfig.RdmFieldDictionaryFileName);
        Assert.Equal("RWFFld", testDictionaryConfig.RdmFieldDictionaryItemName);
    }

    // This test will load different failure scenarios for the verification method.
    // The positive test cases were already covered in the XmlConfigTest
    [Fact]
    public void NiProviderVerificationFailureTest()
    {
        OmmNiProviderConfig niProvConfig;
        NiProviderConfig tmpNiProvConfig_1;

        ClientChannelConfig tmpChannelConfig_1;
        ClientChannelConfig tmpChannelConfig_2;
        ClientChannelConfig tmpChannelConfig_3;

        LoggerConfig tmpLoggerConfig_1;
        LoggerConfig tmpLoggerConfig_2;

        DictionaryConfig tmpDictionaryConfig_1;
        DictionaryConfig tmpDictionaryConfig_2;

        DirectoryConfig tmpDirectoryConfig_1;
        DirectoryConfig tmpDirectoryConfig_2;

        EmaServiceConfig tmpService_1;
        EmaServiceConfig tmpService_2;


        // Load a blank config so we can be sure that everything added is from the programmtic config
        // This doesn't use the default behavior(loading EmaConfig.xml) because an EmaConfig.xml file may be present in the directory for other example apps
        niProvConfig = new OmmNiProviderConfig("../../../OmmConfigTests/EmaBlankConfig.xml");

        OmmNiProviderConfigImpl niProvConfigImpl = niProvConfig.OmmNiProvConfigImpl;

        // Loaded the Config, now make sure everything's in it.

        // There should be nothing in here
        Assert.Empty(niProvConfigImpl.NiProviderConfigMap);
        Assert.Empty(niProvConfigImpl.ClientChannelConfigMap);
        Assert.Empty(niProvConfigImpl.LoggerConfigMap);
        Assert.Empty(niProvConfigImpl.DictionaryConfigMap);

        // Verify should succeed
        niProvConfigImpl.VerifyConfiguration();

        // Set a non-existant consumer name
        niProvConfigImpl.NiProviderName = "bad_niprov";

        // This should fail.
        Assert.Throws<OmmInvalidConfigurationException>(() => niProvConfigImpl.VerifyConfiguration());

        // Unset the ConsumerName and test the DefaultConsumer string
        niProvConfigImpl.NiProviderName = string.Empty;
        niProvConfigImpl.DefaultNiProvider = "bad_niprov";

        // This should fail.
        Assert.Throws<OmmInvalidConfigurationException>(() => niProvConfigImpl.VerifyConfiguration());

        niProvConfigImpl.DefaultNiProvider = string.Empty;

        tmpNiProvConfig_1 = new NiProviderConfig();

        tmpNiProvConfig_1.Name = "niProv_1";
        tmpNiProvConfig_1.ChannelSet.Add("Bad_channel_1");

        niProvConfigImpl.NiProviderConfigMap.Add(tmpNiProvConfig_1.Name, tmpNiProvConfig_1);

        // This should fail.
        Assert.Throws<OmmInvalidConfigurationException>(() => niProvConfigImpl.VerifyConfiguration());

        // Add a couple of channels, make sure it still fails.
        tmpChannelConfig_1 = new ClientChannelConfig();
        tmpChannelConfig_1.Name = "ConfigChannel_1";
        niProvConfigImpl.ClientChannelConfigMap.Add(tmpChannelConfig_1.Name, tmpChannelConfig_1);

        tmpChannelConfig_2 = new ClientChannelConfig();
        tmpChannelConfig_2.Name = "ConfigChannel_2";
        niProvConfigImpl.ClientChannelConfigMap.Add(tmpChannelConfig_2.Name, tmpChannelConfig_2);

        tmpChannelConfig_3 = new ClientChannelConfig();
        tmpChannelConfig_3.Name = "ConfigChannel_3";
        niProvConfigImpl.ClientChannelConfigMap.Add(tmpChannelConfig_3.Name, tmpChannelConfig_3);

        // This should fail.
        Assert.Throws<OmmInvalidConfigurationException>(() => niProvConfigImpl.VerifyConfiguration());

        // Clear the channelSet, add 3 channels with the failure in the middle.
        tmpNiProvConfig_1.ChannelSet.Clear();
        tmpNiProvConfig_1.ChannelSet.Add("ConfigChannel_1");
        tmpNiProvConfig_1.ChannelSet.Add("bad_channel");
        tmpNiProvConfig_1.ChannelSet.Add("ConfigChannel_3");

        // This should fail.
        Assert.Throws<OmmInvalidConfigurationException>(() => niProvConfigImpl.VerifyConfiguration());

        // Clear the channelSet, add 3 channels with the failure at the end.
        tmpNiProvConfig_1.ChannelSet.Clear();
        tmpNiProvConfig_1.ChannelSet.Add("ConfigChannel_1");
        tmpNiProvConfig_1.ChannelSet.Add("ConfigChannel_2");
        tmpNiProvConfig_1.ChannelSet.Add("bad_channel");

        // This should fail.
        Assert.Throws<OmmInvalidConfigurationException>(() => niProvConfigImpl.VerifyConfiguration());

        // Clear the channelSet, set 3 good channels, make sure it succeeds
        tmpNiProvConfig_1.ChannelSet.Clear();
        tmpNiProvConfig_1.ChannelSet.Add("ConfigChannel_1");
        tmpNiProvConfig_1.ChannelSet.Add("ConfigChannel_2");
        tmpNiProvConfig_1.ChannelSet.Add("ConfigChannel_3");

        niProvConfigImpl.VerifyConfiguration();

        // Add a logger mismatch
        tmpNiProvConfig_1.Logger = "bad_logger";

        // This should fail.
        Assert.Throws<OmmInvalidConfigurationException>(() => niProvConfigImpl.VerifyConfiguration());

        // Add a couple of loggers and make sure it's still a mismatch

        tmpLoggerConfig_1 = new LoggerConfig();
        tmpLoggerConfig_1.Name = "ConfigLogger_1";
        niProvConfigImpl.LoggerConfigMap.Add(tmpLoggerConfig_1.Name, tmpLoggerConfig_1);

        tmpLoggerConfig_2 = new LoggerConfig();
        tmpLoggerConfig_2.Name = "ConfigLogger_2";
        niProvConfigImpl.LoggerConfigMap.Add(tmpLoggerConfig_2.Name, tmpLoggerConfig_2);

        tmpNiProvConfig_1.Logger = "bad_logger";

        // This should fail.
        Assert.Throws<OmmInvalidConfigurationException>(() => niProvConfigImpl.VerifyConfiguration());

        // Set the logger correctly and make sure it verifies

        tmpNiProvConfig_1.Logger = "ConfigLogger_2";

        niProvConfigImpl.VerifyConfiguration();

        // Add in the dictionaries and make sure it still works - these are all going by the default directory.
        tmpDictionaryConfig_1 = new DictionaryConfig();
        tmpDictionaryConfig_1.Name = "ConfigDictionary_1";
        niProvConfigImpl.DictionaryConfigMap.Add(tmpDictionaryConfig_1.Name, tmpDictionaryConfig_1);

        tmpDictionaryConfig_2 = new DictionaryConfig();
        tmpDictionaryConfig_2.Name = "ConfigDictionary_2";
        niProvConfigImpl.DictionaryConfigMap.Add(tmpDictionaryConfig_2.Name, tmpDictionaryConfig_2);

        niProvConfigImpl.VerifyConfiguration();

        // Setup the services and directory configs.
        tmpService_1 = new EmaServiceConfig(true, false);
        tmpService_1.Service.Info.ServiceName.Data("ConfigService_1");
        tmpService_1.Service.ServiceId = 5;

        tmpService_2 = new EmaServiceConfig(true, false);
        tmpService_2.Service.Info.ServiceName.Data("ConfigService_2");
        tmpService_1.Service.ServiceId = 20;

        tmpDirectoryConfig_1 = new DirectoryConfig();
        tmpDirectoryConfig_1.Name = "ConfigDirectory_1";
        tmpDirectoryConfig_1.ServiceMap.Add(tmpService_1.Service.Info.ServiceName.ToString(), tmpService_1);
        niProvConfigImpl.DirectoryConfigMap.Add(tmpDirectoryConfig_1.Name, tmpDirectoryConfig_1);

        tmpDirectoryConfig_2 = new DirectoryConfig();
        tmpDirectoryConfig_2.Name = "ConfigDirectory_2";
        tmpDirectoryConfig_2.ServiceMap.Add(tmpService_1.Service.Info.ServiceName.ToString(), tmpService_1);
        tmpDirectoryConfig_2.ServiceMap.Add(tmpService_2.Service.Info.ServiceName.ToString(), tmpService_2);
        niProvConfigImpl.DirectoryConfigMap.Add(tmpDirectoryConfig_2.Name, tmpDirectoryConfig_2);


        // This should succeed
        niProvConfigImpl.VerifyConfiguration();

        // Set the default directory name to invalid
        niProvConfigImpl.DefaultDirectory = "badDirectory_1";

        // This should fail.
        Assert.Throws<OmmInvalidConfigurationException>(() => niProvConfigImpl.VerifyConfiguration());

        // re-set the default directory name, and set an invalid directory in the main NiProviderConfig
        niProvConfigImpl.DefaultDirectory = string.Empty;
        tmpNiProvConfig_1.Directory = "bad_directory_1";

        // This should fail.
        Assert.Throws<OmmInvalidConfigurationException>(() => niProvConfigImpl.VerifyConfiguration());

        // Set the niProv directory to a correct one.
        niProvConfigImpl.DefaultDirectory = string.Empty;
        tmpNiProvConfig_1.Directory = string.Empty;

        // Add in a bad dictionary name to service_2's used list
        tmpService_2.DictionariesUsedList.Clear();
        tmpService_2.DictionariesUsedList.Add("badDictionary");
        tmpService_2.DictionariesUsedList.Add("ConfigDictionary_1");
        tmpService_2.DictionariesUsedList.Add("ConfigDictionary_2");
        tmpService_2.DictionariesProvidedList.Clear();
        tmpService_2.DictionariesProvidedList.Add("ConfigDictionary_1");
        tmpService_2.DictionariesProvidedList.Add("ConfigDictionary_2");

        // This should fail.
        Assert.Throws<OmmInvalidConfigurationException>(() => niProvConfigImpl.VerifyConfiguration());

        // Add in a bad dictionary name to service_2's used list
        tmpService_2.DictionariesUsedList.Clear();
        tmpService_2.DictionariesUsedList.Add("ConfigDictionary_1");
        tmpService_2.DictionariesUsedList.Add("badDictionary");
        tmpService_2.DictionariesUsedList.Add("ConfigDictionary_2");
        tmpService_2.DictionariesProvidedList.Clear();
        tmpService_2.DictionariesProvidedList.Add("ConfigDictionary_1");
        tmpService_2.DictionariesProvidedList.Add("ConfigDictionary_2");

        // This should fail.
        Assert.Throws<OmmInvalidConfigurationException>(() => niProvConfigImpl.VerifyConfiguration());

        // Add in a bad dictionary name to service_2's used list
        tmpService_2.DictionariesUsedList.Clear();
        tmpService_2.DictionariesUsedList.Add("ConfigDictionary_1");
        tmpService_2.DictionariesUsedList.Add("ConfigDictionary_2");
        tmpService_2.DictionariesUsedList.Add("badDictionary");
        tmpService_2.DictionariesProvidedList.Clear();
        tmpService_2.DictionariesProvidedList.Add("ConfigDictionary_1");
        tmpService_2.DictionariesProvidedList.Add("ConfigDictionary_2");

        // This should fail.
        Assert.Throws<OmmInvalidConfigurationException>(() => niProvConfigImpl.VerifyConfiguration());

        // Add in a bad dictionary name to service_2's provided list
        tmpService_2.DictionariesUsedList.Clear();
        tmpService_2.DictionariesUsedList.Add("ConfigDictionary_1");
        tmpService_2.DictionariesUsedList.Add("ConfigDictionary_2");
        tmpService_2.DictionariesProvidedList.Clear();
        tmpService_2.DictionariesProvidedList.Add("badDictionary");
        tmpService_2.DictionariesProvidedList.Add("ConfigDictionary_1");
        tmpService_2.DictionariesProvidedList.Add("ConfigDictionary_2");

        // This should fail.
        Assert.Throws<OmmInvalidConfigurationException>(() => niProvConfigImpl.VerifyConfiguration());

        // Add in a bad dictionary name to service_2's provided list
        tmpService_2.DictionariesUsedList.Clear();
        tmpService_2.DictionariesUsedList.Add("ConfigDictionary_1");
        tmpService_2.DictionariesUsedList.Add("ConfigDictionary_2");
        tmpService_2.DictionariesProvidedList.Clear();
        tmpService_2.DictionariesProvidedList.Add("ConfigDictionary_1");
        tmpService_2.DictionariesProvidedList.Add("badDictionary");
        tmpService_2.DictionariesProvidedList.Add("ConfigDictionary_2");

        // This should fail.
        Assert.Throws<OmmInvalidConfigurationException>(() => niProvConfigImpl.VerifyConfiguration());

        // Set the dictionaries to correct
        tmpService_2.DictionariesUsedList.Clear();
        tmpService_2.DictionariesUsedList.Add("ConfigDictionary_1");
        tmpService_2.DictionariesUsedList.Add("ConfigDictionary_2");
        tmpService_2.DictionariesProvidedList.Clear();
        tmpService_2.DictionariesProvidedList.Add("ConfigDictionary_1");
        tmpService_2.DictionariesProvidedList.Add("ConfigDictionary_2");

        niProvConfigImpl.VerifyConfiguration();

        // set service_2's dictionaries to correct.
        tmpService_2.DictionariesUsedList.Clear();
        tmpService_2.DictionariesUsedList.Add("ConfigDictionary_1");
        tmpService_2.DictionariesUsedList.Add("ConfigDictionary_2");
        tmpService_2.DictionariesProvidedList.Clear();
        tmpService_2.DictionariesProvidedList.Add("ConfigDictionary_1");
        tmpService_2.DictionariesProvidedList.Add("ConfigDictionary_2");

        // Set both services to the same service Id
        tmpService_1.Service.ServiceId = 10;
        tmpService_2.Service.ServiceId = 10;

        // This should fail.
        Assert.Throws<OmmInvalidConfigurationException>(() => niProvConfigImpl.VerifyConfiguration());
    }

    [Fact]
    public void NiProviderProgrammaticConfigTest()
    {
        OmmNiProviderConfig niProvConfig;
        NiProviderConfig testNiProvConfig;
        ClientChannelConfig testChannelConfig;
        LoggerConfig testLoggerConfig;
        DictionaryConfig testDictConfig;
        DirectoryConfig testDirectoryConfig;
        EmaServiceConfig testServiceConfig;

        // Top level map
        Map outerMap = new Map();
        // Middle map for consumers, channels, loggers, dictionaries
        Map innerMap = new Map();
        // Outer element list 
        ElementList encodeGroupList = new ElementList();
        ElementList encodeObjectList = new ElementList();

        // Service Map
        Map serviceMap = new Map();
        ElementList serviceElementList = new ElementList();
        ElementList serviceFilterList = new ElementList();
        ElementList innerServiceElementList = new ElementList();
        Series qosSeries = new Series();
        OmmArray serviceArray = new OmmArray();

        // Load a blank config so we can be sure that everything added is from the programmtic config
        // This doesn't use the default behavior(loading EmaConfig.xml) because an EmaConfig.xml file may be present in the directory for other example apps
        niProvConfig = new OmmNiProviderConfig("../../../OmmConfigTests/EmaBlankConfig.xml");

        OmmNiProviderConfigImpl niProvConfigImpl = niProvConfig.OmmNiProvConfigImpl;

        // Loaded the Config, now make sure everything's in it.

        // There should be nothing in there.
        Assert.Empty(niProvConfigImpl.NiProviderConfigMap);
        Assert.Empty(niProvConfigImpl.ClientChannelConfigMap);
        Assert.Empty(niProvConfigImpl.LoggerConfigMap);
        Assert.Empty(niProvConfigImpl.DictionaryConfigMap);

        outerMap.Clear();

        innerMap.Clear();

        encodeGroupList.Clear();

        encodeObjectList.Clear();

        // Encode ProgNiProvider_1
        encodeObjectList.AddAscii("Channel", "ProgChannel_1")
            .AddAscii("Logger", "ProgLogger_1")
            .AddAscii("Directory", "ProgDirectory_1")
            .AddUInt("LoginRequestTimeOut", 2030)
            .AddInt("DispatchTimeoutApiThread", 2040)
            .AddUInt("ItemCountHint", 2050)
            .AddUInt("MaxDispatchCountApiThread", 2060)
            .AddUInt("MaxDispatchCountUserThread", 2070)
            .AddUInt("MergeSourceDirectoryStreams", 0)
            .AddUInt("RecoverUserSubmitSourceDirectory", 0)
            .AddUInt("RefreshFirstRequired", 0)
            .AddUInt("RemoveItemsOnDisconnect", 0)
            .AddUInt("RequestTimeout", 2140)
            .AddInt("ReconnectAttemptLimit", 2100)
            .AddInt("ReconnectMaxDelay", 2120)
            .AddInt("ReconnectMinDelay", 2110)
            .AddUInt("ServiceCountHint", 2160)
            // XML tracing block
            .AddUInt("XmlTraceToStdout", 1)
            .AddUInt("XmlTraceToFile", 0)
            .AddUInt("XmlTraceMaxFileSize", (ulong)10_000_000)
            .AddAscii("XmlTraceFileName", "EmaTrace")
            .AddUInt("XmlTraceToMultipleFiles", 0)
            .AddUInt("XmlTraceWrite", 1)
            .AddUInt("XmlTraceRead", 1)
            .AddUInt("XmlTracePing", 0)
            .MarkForClear().Complete();

        innerMap.AddKeyAscii("ProgNiProvider_1", MapAction.ADD, encodeObjectList);

        encodeObjectList.Clear();

        encodeObjectList.AddAscii("ChannelSet", "ProgChannel_1, ProgChannel_2")
            .MarkForClear().Complete();

        innerMap.AddKeyAscii("ProgNiProvider_2", MapAction.ADD, encodeObjectList);

        innerMap.MarkForClear().Complete();

        encodeGroupList.Clear();

        encodeGroupList.AddAscii("DefaultNiProvider", "ProgNiProvider_1")
            .AddMap("NiProviderList", innerMap)
            .MarkForClear().Complete();

        outerMap.AddKeyAscii("NiProviderGroup", MapAction.ADD, encodeGroupList);


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
            .MarkForClear().Complete();

        innerMap.AddKeyAscii("ProgChannel_1", MapAction.ADD, encodeObjectList);


        // Second channel is all defaults, so add an empty Element List
        encodeObjectList.Clear();
        encodeObjectList.MarkForClear().Complete();

        innerMap.AddKeyAscii("ProgChannel_2", MapAction.ADD, encodeObjectList)
            .MarkForClear().Complete();

        encodeGroupList.AddMap("ChannelList", innerMap)
            .MarkForClear().Complete();

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
            .MarkForClear().Complete();

        innerMap.AddKeyAscii("ProgLogger_1", MapAction.ADD, encodeObjectList);

        // Blank logger config
        encodeObjectList.Clear();
        encodeObjectList.MarkForClear().Complete();

        innerMap.AddKeyAscii("ProgLogger_2", MapAction.ADD, encodeObjectList)
            .MarkForClear().Complete();

        encodeGroupList.AddMap("LoggerList", innerMap)
            .MarkForClear().Complete();

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
            .MarkForClear().Complete();

        innerMap.AddKeyAscii("ProgDictionary_1", MapAction.ADD, encodeObjectList);

        encodeObjectList.Clear();
        encodeObjectList.MarkForClear().Complete();

        innerMap.AddKeyAscii("ProgDictionary_2", MapAction.ADD, encodeObjectList)
            .MarkForClear().Complete();

        encodeGroupList.AddMap("DictionaryList", innerMap)
            .MarkForClear().Complete();

        outerMap.AddKeyAscii("DictionaryGroup", MapAction.ADD, encodeGroupList);

        // Encode a couple of directories.
        // Directory Group element list
        encodeGroupList.Clear();
        // DirectoryList map
        innerMap.Clear();
        // Services map
        serviceMap.Clear();
        // element list for the filter entries(info, state, etc.)
        serviceFilterList.Clear();
        // Filter elements element list
        serviceElementList.Clear();

        // Additional elements that need to be encoded.
        qosSeries.Clear();
        serviceArray.Clear();
        innerServiceElementList.Clear();

        // Default directory of progDirectory.
        encodeGroupList.AddAscii("DefaultDirectory", "progDirectory_1");

        // Start encoding the first directory here, starting with the first service
        // Encode the InfoFilter
        serviceElementList.AddUInt("ServiceId", 11)
                            .AddAscii("Vendor", "progVendor")
                            .AddUInt("IsSource", 1);

        serviceArray.AddUInt(130)
                    .AddUInt(5)
                    .MarkForClear().Complete();

        serviceElementList.AddArray("Capabilities", serviceArray)
                            .AddUInt("AcceptingConsumerStatus", 1)
                            .AddAscii("ItemList", "progItem");

        serviceArray.Clear();
        serviceArray.AddAscii("ProgDictionary_1")
                    .AddAscii("ProgDictionary_2")
                    .MarkForClear().Complete();
        serviceElementList.AddArray("DictionariesProvided", serviceArray);

        serviceArray.Clear();
        serviceArray.AddAscii("ProgDictionary_1")
                    .AddAscii("ProgDictionary_2")
                    .MarkForClear().Complete();
        serviceElementList.AddArray("DictionariesUsed", serviceArray);

        qosSeries.Clear();
        innerServiceElementList.Clear();
        innerServiceElementList.AddAscii("Timeliness", "Timeliness::InexactDelayed")
                                .AddAscii("Rate", "Rate::JustInTimeConflated")
                                .MarkForClear().Complete();

        qosSeries.AddEntry(innerServiceElementList);

        innerServiceElementList.Clear();
        innerServiceElementList.AddUInt("Timeliness", 100)
                                .AddUInt("Rate", 200)
                                .MarkForClear().Complete();
        qosSeries.AddEntry(innerServiceElementList)
                    .MarkForClear().Complete();

        serviceElementList.AddSeries("QoS", qosSeries);

        serviceElementList.AddUInt("SupportsQoSRange", 1)
                            .AddUInt("SupportsOutOfBandSnapshots", 1)
                            .MarkForClear().Complete();

        serviceFilterList.AddElementList("InfoFilter", serviceElementList);

        // Encode the State filter
        serviceElementList.Clear();

        serviceElementList.AddUInt("ServiceState", 0)
                            .AddUInt("AcceptingRequests", 1);

        innerServiceElementList.Clear();
        innerServiceElementList.AddAscii("StreamState", "StreamState::ClosedRedirected")
                                .AddAscii("DataState", "DataState::Suspect")
                                .AddAscii("StatusCode", "StatusCode::NotOpen")
                                .AddAscii("StatusText", "ProgStatusText")
                                .MarkForClear().Complete();

        serviceElementList.AddElementList("Status", innerServiceElementList)
                            .MarkForClear().Complete();

        serviceFilterList.AddElementList("StateFilter", serviceElementList);

        // Encode the Load Filter
        serviceElementList.Clear();

        serviceElementList.AddUInt("OpenLimit", 500)
                            .AddUInt("OpenWindow", 600)
                            .AddUInt("LoadFactor", 700)
                            .MarkForClear().Complete();
        serviceFilterList.AddElementList("LoadFilter", serviceElementList)
                            .MarkForClear().Complete();

        serviceMap.AddKeyAscii("ProgService_1", MapAction.ADD, serviceFilterList);

        // Encode an all-default service, except for service Id and capabilities
        serviceFilterList.Clear();

        serviceElementList.Clear();
        serviceArray.Clear();
        serviceElementList.AddUInt("ServiceId", 12)
                            .AddUInt("IsSource", 1);

        serviceArray.AddAscii("MMT_HISTORY")
                    .MarkForClear().Complete();

        serviceElementList.AddArray("Capabilities", serviceArray)
                            .MarkForClear().Complete();
        serviceFilterList.AddElementList("InfoFilter", serviceElementList)
                        .MarkForClear().Complete();

        serviceMap.AddKeyAscii("ProgService_2", MapAction.ADD, serviceFilterList)
                    .MarkForClear().Complete();

        innerMap.AddKeyAscii("ProgDirectory_1", MapAction.ADD, serviceMap)
                .MarkForClear().Complete();

        encodeGroupList.AddMap("DirectoryList", innerMap)
                        .MarkForClear().Complete();

        outerMap.AddKeyAscii("DirectoryGroup", MapAction.ADD, encodeGroupList);

        outerMap.MarkForClear().Complete();

        niProvConfigImpl.Config(outerMap);

        Assert.Equal(2, niProvConfigImpl.NiProviderConfigMap.Count);
        Assert.Equal(2, niProvConfigImpl.ClientChannelConfigMap.Count);
        Assert.Equal(2, niProvConfigImpl.LoggerConfigMap.Count);
        Assert.Equal(2, niProvConfigImpl.DictionaryConfigMap.Count);
        Assert.Single(niProvConfigImpl.DirectoryConfigMap);

        Assert.Empty(niProvConfigImpl.ConfigErrorLog!.ErrorList);

        testNiProvConfig = niProvConfigImpl.NiProviderConfigMap["ProgNiProvider_1"];

        Assert.Equal("ProgNiProvider_1", testNiProvConfig.Name);
        Assert.Single(testNiProvConfig.ChannelSet);
        Assert.Equal("ProgChannel_1", testNiProvConfig.ChannelSet[0]);
        Assert.Equal("ProgLogger_1", testNiProvConfig.Logger);
        Assert.Equal("ProgDirectory_1", testNiProvConfig.Directory);
        Assert.Equal((long)2030, testNiProvConfig.LoginRequestTimeOut);
        Assert.Equal(2040, testNiProvConfig.DispatchTimeoutApiThread);
        Assert.Equal((ulong)2050, testNiProvConfig.ItemCountHint);
        Assert.Equal((int)2060, testNiProvConfig.MaxDispatchCountApiThread);
        Assert.Equal((int)2070, testNiProvConfig.MaxDispatchCountUserThread);
        Assert.False(testNiProvConfig.MergeSourceDirectoryStreams);
        Assert.Equal(2100, testNiProvConfig.ReconnectAttemptLimit);
        Assert.Equal(2120, testNiProvConfig.ReconnectMaxDelay);
        Assert.Equal(2110, testNiProvConfig.ReconnectMinDelay);
        Assert.False(testNiProvConfig.RecoverUserSubmitSourceDirectory);
        Assert.False(testNiProvConfig.RefreshFirstRequired);
        Assert.False(testNiProvConfig.RemoveItemsOnDisconnect);
        Assert.Equal((ulong)2140, testNiProvConfig.RequestTimeout);
        Assert.Equal((int)2160, testNiProvConfig.ServiceCountHint);
        // XML tracing block
        Assert.True(testNiProvConfig.XmlTraceToStdout);
        Assert.False(testNiProvConfig.XmlTraceToFile);
        Assert.Equal((ulong)10_000_000, testNiProvConfig.XmlTraceMaxFileSize);
        Assert.Equal("EmaTrace", testNiProvConfig.XmlTraceFileName);
        Assert.False(testNiProvConfig.XmlTraceToMultipleFiles);
        Assert.True(testNiProvConfig.XmlTraceWrite);
        Assert.True(testNiProvConfig.XmlTraceRead);
        Assert.False(testNiProvConfig.XmlTracePing);


        // TestConsumer_2 has all defaults except for ChannelSet.
        testNiProvConfig = niProvConfigImpl.NiProviderConfigMap["ProgNiProvider_2"];

        Assert.Equal("ProgNiProvider_2", testNiProvConfig.Name);
        Assert.Equal(2, testNiProvConfig.ChannelSet.Count);
        Assert.Equal("ProgChannel_1", testNiProvConfig.ChannelSet[0]);
        Assert.Equal("ProgChannel_2", testNiProvConfig.ChannelSet[1]);
        Assert.Equal(defaultNiProviderConfig.Directory, testNiProvConfig.Directory);
        Assert.Equal(defaultNiProviderConfig.LoginRequestTimeOut, testNiProvConfig.LoginRequestTimeOut);
        Assert.Equal(defaultNiProviderConfig.DispatchTimeoutApiThread, testNiProvConfig.DispatchTimeoutApiThread);
        Assert.Equal(defaultNiProviderConfig.ItemCountHint, testNiProvConfig.ItemCountHint);
        Assert.Equal(defaultNiProviderConfig.MaxDispatchCountApiThread, testNiProvConfig.MaxDispatchCountApiThread);
        Assert.Equal(defaultNiProviderConfig.MaxDispatchCountUserThread, testNiProvConfig.MaxDispatchCountUserThread);
        Assert.Equal(defaultNiProviderConfig.MergeSourceDirectoryStreams, testNiProvConfig.MergeSourceDirectoryStreams);
        Assert.Equal(defaultNiProviderConfig.ReconnectAttemptLimit, testNiProvConfig.ReconnectAttemptLimit);
        Assert.Equal(defaultNiProviderConfig.ReconnectMaxDelay, testNiProvConfig.ReconnectMaxDelay);
        Assert.Equal(defaultNiProviderConfig.ReconnectMinDelay, testNiProvConfig.ReconnectMinDelay);
        Assert.Equal(defaultNiProviderConfig.RecoverUserSubmitSourceDirectory, testNiProvConfig.RecoverUserSubmitSourceDirectory);
        Assert.Equal(defaultNiProviderConfig.RefreshFirstRequired, testNiProvConfig.RefreshFirstRequired);
        Assert.Equal(defaultNiProviderConfig.RemoveItemsOnDisconnect, testNiProvConfig.RemoveItemsOnDisconnect);
        Assert.Equal(defaultNiProviderConfig.RequestTimeout, testNiProvConfig.RequestTimeout);
        Assert.Equal(defaultNiProviderConfig.ServiceCountHint, testNiProvConfig.ServiceCountHint);
        // XML tracing block
        Assert.Equal(defaultNiProviderConfig.XmlTraceToStdout, testNiProvConfig.XmlTraceToStdout);
        Assert.Equal(defaultNiProviderConfig.XmlTraceToFile, testNiProvConfig.XmlTraceToFile);
        Assert.Equal(defaultNiProviderConfig.XmlTraceMaxFileSize, testNiProvConfig.XmlTraceMaxFileSize);
        Assert.Equal(defaultNiProviderConfig.XmlTraceFileName, testNiProvConfig.XmlTraceFileName);
        Assert.Equal(defaultNiProviderConfig.XmlTraceToMultipleFiles, testNiProvConfig.XmlTraceToMultipleFiles);
        Assert.Equal(defaultNiProviderConfig.XmlTraceWrite, testNiProvConfig.XmlTraceWrite);
        Assert.Equal(defaultNiProviderConfig.XmlTraceRead, testNiProvConfig.XmlTraceRead);
        Assert.Equal(defaultNiProviderConfig.XmlTracePing, testNiProvConfig.XmlTracePing);


        testChannelConfig = niProvConfigImpl.ClientChannelConfigMap["ProgChannel_1"];

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
        testChannelConfig = niProvConfigImpl.ClientChannelConfigMap["ProgChannel_2"];

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

        testLoggerConfig = niProvConfigImpl.LoggerConfigMap["ProgLogger_1"];

        Assert.Equal("ProgLogger_1", testLoggerConfig.Name);
        Assert.Equal(LoggerType.STDOUT, testLoggerConfig.LoggerType);
        Assert.Equal(LoggerLevel.INFO, testLoggerConfig.LoggerSeverity);
        Assert.Equal("ProgLogFile", testLoggerConfig.FileName);
        Assert.Equal((ulong)1, testLoggerConfig.IncludeDateInLoggerOutput);
        Assert.Equal((ulong)20, testLoggerConfig.NumberOfLogFiles);
        Assert.Equal((ulong)100, testLoggerConfig.MaxLogFileSize);

        // ProgLogger_2 is all defaults
        testLoggerConfig = niProvConfigImpl.LoggerConfigMap["ProgLogger_2"];

        Assert.Equal("ProgLogger_2", testLoggerConfig.Name);
        Assert.Equal(defaultLoggerConfig.LoggerType, testLoggerConfig.LoggerType);
        Assert.Equal(defaultLoggerConfig.LoggerSeverity, testLoggerConfig.LoggerSeverity);
        Assert.Equal(defaultLoggerConfig.FileName, testLoggerConfig.FileName);
        Assert.Equal(defaultLoggerConfig.IncludeDateInLoggerOutput, testLoggerConfig.IncludeDateInLoggerOutput);
        Assert.Equal(defaultLoggerConfig.NumberOfLogFiles, testLoggerConfig.NumberOfLogFiles);
        Assert.Equal(defaultLoggerConfig.MaxLogFileSize, testLoggerConfig.MaxLogFileSize);

        testDictConfig = niProvConfigImpl.DictionaryConfigMap["ProgDictionary_1"];

        Assert.Equal("ProgDictionary_1", testDictConfig.Name);
        Assert.Equal(EmaConfig.DictionaryTypeEnum.FILE, testDictConfig.DictionaryType);
        Assert.Equal("ProgEnumFile", testDictConfig.EnumTypeDefFileName);
        Assert.Equal("ProgEnumItem", testDictConfig.EnumTypeDefItemName);
        Assert.Equal("ProgFieldFile", testDictConfig.RdmFieldDictionaryFileName);
        Assert.Equal("ProgFieldItem", testDictConfig.RdmFieldDictionaryItemName);

        // ProgDictionary_1 is set to defaults
        testDictConfig = niProvConfigImpl.DictionaryConfigMap["ProgDictionary_2"];

        Assert.Equal("ProgDictionary_2", testDictConfig.Name);
        Assert.Equal(defaultDictConfig.DictionaryType, testDictConfig.DictionaryType);
        Assert.Equal(defaultDictConfig.EnumTypeDefFileName, testDictConfig.EnumTypeDefFileName);
        Assert.Equal(defaultDictConfig.EnumTypeDefItemName, testDictConfig.EnumTypeDefItemName);
        Assert.Equal(defaultDictConfig.RdmFieldDictionaryFileName, testDictConfig.RdmFieldDictionaryFileName);
        Assert.Equal(defaultDictConfig.RdmFieldDictionaryItemName, testDictConfig.RdmFieldDictionaryItemName);

        // Verify the Directory configuration here
        // TestDictionary_2 is set to defaults
        testDirectoryConfig = niProvConfigImpl.DirectoryConfigMap["ProgDirectory_1"];

        Assert.Equal("ProgDirectory_1", testDirectoryConfig.Name);
        Assert.Equal(2, testDirectoryConfig.ServiceMap.Count);

        testServiceConfig = testDirectoryConfig.ServiceMap["ProgService_1"];

        // Check the Info filter
        Assert.True(testServiceConfig.Service.HasInfo);
        Assert.Equal("ProgService_1", testServiceConfig.Service.Info.ServiceName.ToString());
        Assert.Equal(11, testServiceConfig.Service.ServiceId);
        Assert.True(testServiceConfig.Service.Info.HasVendor);
        Assert.Equal("progVendor", testServiceConfig.Service.Info.Vendor.ToString());
        Assert.True(testServiceConfig.Service.Info.HasIsSource);
        Assert.Equal(1, testServiceConfig.Service.Info.IsSource);
        Assert.Equal(2, testServiceConfig.Service.Info.CapabilitiesList.Count);
        Assert.Equal(130, testServiceConfig.Service.Info.CapabilitiesList[0]);
        Assert.Equal(5, testServiceConfig.Service.Info.CapabilitiesList[1]);
        Assert.True(testServiceConfig.Service.Info.HasAcceptingConsStatus);
        Assert.Equal(1, testServiceConfig.Service.Info.AcceptConsumerStatus);
        Assert.True(testServiceConfig.Service.Info.HasItemList);
        Assert.Equal("progItem", testServiceConfig.Service.Info.ItemList.ToString());
        // The dictionaries provided and used in the service will be empty at this point.
        Assert.False(testServiceConfig.Service.Info.HasDictionariesProvided);
        Assert.Empty(testServiceConfig.Service.Info.DictionariesProvidedList);
        Assert.Equal(2, testServiceConfig.DictionariesProvidedList.Count);
        Assert.Equal("ProgDictionary_1", testServiceConfig.DictionariesProvidedList[0]);
        Assert.Equal("ProgDictionary_2", testServiceConfig.DictionariesProvidedList[1]);
        Assert.False(testServiceConfig.Service.Info.HasDictionariesUsed);
        Assert.Empty(testServiceConfig.Service.Info.DictionariesUsedList);
        Assert.Equal(2, testServiceConfig.DictionariesUsedList.Count);
        Assert.Equal("ProgDictionary_1", testServiceConfig.DictionariesUsedList[0]);
        Assert.Equal("ProgDictionary_2", testServiceConfig.DictionariesUsedList[1]);
        Assert.True(testServiceConfig.Service.Info.HasQos);
        Assert.Equal(2, testServiceConfig.Service.Info.QosList.Count);
        Assert.Equal(QosTimeliness.DELAYED_UNKNOWN, testServiceConfig.Service.Info.QosList[0].Timeliness());
        Assert.Equal(0, testServiceConfig.Service.Info.QosList[0].TimeInfo());
        Assert.Equal(QosRates.JIT_CONFLATED, testServiceConfig.Service.Info.QosList[0].Rate());
        Assert.Equal(0, testServiceConfig.Service.Info.QosList[0].RateInfo());
        Assert.Equal(QosTimeliness.DELAYED, testServiceConfig.Service.Info.QosList[1].Timeliness());
        Assert.Equal(100, testServiceConfig.Service.Info.QosList[1].TimeInfo());
        Assert.Equal(QosRates.TIME_CONFLATED, testServiceConfig.Service.Info.QosList[1].Rate());
        Assert.Equal(200, testServiceConfig.Service.Info.QosList[1].RateInfo());
        Assert.True(testServiceConfig.Service.Info.HasSupportQosRange);
        Assert.Equal(1, testServiceConfig.Service.Info.SupportsQosRange);
        Assert.True(testServiceConfig.Service.Info.HasSupportOOBSnapshots);
        Assert.Equal(1, testServiceConfig.Service.Info.SupportsOOBSnapshots);
        // Check the state filter
        Assert.True(testServiceConfig.Service.HasState);
        Assert.Equal(0, testServiceConfig.Service.State.ServiceStateVal);
        Assert.True(testServiceConfig.Service.State.HasAcceptingRequests);
        Assert.Equal(1, testServiceConfig.Service.State.AcceptingRequests);
        Assert.True(testServiceConfig.Service.State.HasStatus);
        Assert.Equal(StreamStates.REDIRECTED, testServiceConfig.Service.State.Status.StreamState());
        Assert.Equal(DataStates.SUSPECT, testServiceConfig.Service.State.Status.DataState());
        Assert.Equal(OmmState.StatusCodes.NOT_OPEN, testServiceConfig.Service.State.Status.Code());
        Assert.Equal("ProgStatusText", testServiceConfig.Service.State.Status.Text().ToString());
        // Check the load filter
        Assert.True(testServiceConfig.Service.HasLoad);
        Assert.True(testServiceConfig.Service.Load.HasOpenLimit);
        Assert.Equal(500, testServiceConfig.Service.Load.OpenLimit);
        Assert.True(testServiceConfig.Service.Load.HasOpenWindow);
        Assert.Equal(600, testServiceConfig.Service.Load.OpenWindow);
        Assert.True(testServiceConfig.Service.Load.HasLoadFactor);
        Assert.Equal(700, testServiceConfig.Service.Load.LoadFactor);

        testServiceConfig = testDirectoryConfig.ServiceMap["ProgService_2"];

        // This is EMA defaults.
        // Check the Info filter
        Assert.True(testServiceConfig.Service.HasInfo);
        Assert.Equal("ProgService_2", testServiceConfig.Service.Info.ServiceName.ToString());
        Assert.Equal(12, testServiceConfig.Service.ServiceId);
        Assert.False(testServiceConfig.Service.Info.HasVendor);
        Assert.True(testServiceConfig.Service.Info.HasIsSource);
        Assert.Equal(1, testServiceConfig.Service.Info.IsSource);
        Assert.Single(testServiceConfig.Service.Info.CapabilitiesList);
        Assert.Equal(EmaRdm.MMT_HISTORY, testServiceConfig.Service.Info.CapabilitiesList[0]);
        Assert.True(testServiceConfig.Service.Info.HasAcceptingConsStatus);
        Assert.Equal(1, testServiceConfig.Service.Info.AcceptConsumerStatus);
        Assert.False(testServiceConfig.Service.Info.HasItemList);
        // The dictionaries provided and used in the service will be empty at this point.
        Assert.False(testServiceConfig.Service.Info.HasDictionariesProvided);
        Assert.Empty(testServiceConfig.Service.Info.DictionariesProvidedList);
        Assert.Empty(testServiceConfig.DictionariesProvidedList);
        Assert.False(testServiceConfig.Service.Info.HasDictionariesUsed);
        Assert.Empty(testServiceConfig.Service.Info.DictionariesUsedList);
        Assert.Empty(testServiceConfig.DictionariesUsedList);
        Assert.True(testServiceConfig.Service.Info.HasQos);
        Assert.Empty(testServiceConfig.Service.Info.QosList);
        Assert.True(testServiceConfig.Service.Info.HasSupportOOBSnapshots);
        Assert.Equal(0, testServiceConfig.Service.Info.SupportsOOBSnapshots);
        // Check the state filter
        Assert.True(testServiceConfig.Service.HasState);
        Assert.Equal(1, testServiceConfig.Service.State.ServiceStateVal);
        Assert.True(testServiceConfig.Service.State.HasAcceptingRequests);
        Assert.Equal(0, testServiceConfig.Service.State.AcceptingRequests);
        Assert.True(testServiceConfig.Service.State.HasStatus);
        Assert.Equal(StreamStates.OPEN, testServiceConfig.Service.State.Status.StreamState());
        Assert.Equal(DataStates.OK, testServiceConfig.Service.State.Status.DataState());
        Assert.Equal(OmmState.StatusCodes.NONE, testServiceConfig.Service.State.Status.Code());
        Assert.Equal(0, testServiceConfig.Service.State.Status.Text().Length);
        // Check the load filter
        Assert.True(testServiceConfig.Service.HasLoad);
        Assert.False(testServiceConfig.Service.Load.HasOpenLimit);
        Assert.False(testServiceConfig.Service.Load.HasOpenWindow);
        Assert.False(testServiceConfig.Service.Load.HasLoadFactor);

        // Verify that the config works here
        niProvConfigImpl.VerifyConfiguration();
    }

    [Fact]
    public void NiProviderProgrammaticConfigOverlayTest()
    {
        OmmNiProviderConfig niProvConfig;
        NiProviderConfig testNiProvConfig;
        ClientChannelConfig testChannelConfig;
        LoggerConfig testLoggerConfig;
        DictionaryConfig testDictConfig;
        DirectoryConfig testDirectoryConfig;
        EmaServiceConfig testServiceConfig;

        // Top level map
        Map outerMap = new Map();
        // Middle map for consumers, channels, loggers, dictionaries
        Map innerMap = new Map();
        // Outer element list 
        ElementList encodeGroupList = new ElementList();
        ElementList encodeObjectList = new ElementList();

        // Service Map
        Map serviceMap = new Map();
        ElementList serviceElementList = new ElementList();
        ElementList serviceFilterList = new ElementList();
        ElementList innerServiceElementList = new ElementList();
        Series qosSeries = new Series();
        OmmArray serviceArray = new OmmArray();

        // Load a blank config so we can be sure that everything added is from the programmtic config
        // This doesn't use the default behavior(loading EmaConfig.xml) because an EmaConfig.xml file may be present in the directory for other example apps
        niProvConfig = new OmmNiProviderConfig("../../../OmmConfigTests/EmaBlankConfig.xml");

        OmmNiProviderConfigImpl niProvConfigImpl = niProvConfig.OmmNiProvConfigImpl;

        // Loaded the Config, now make sure everything's in it.

        // There should be nothing in there.
        Assert.Empty(niProvConfigImpl.NiProviderConfigMap);
        Assert.Empty(niProvConfigImpl.ClientChannelConfigMap);
        Assert.Empty(niProvConfigImpl.LoggerConfigMap);
        Assert.Empty(niProvConfigImpl.DictionaryConfigMap);

        outerMap.Clear();

        innerMap.Clear();

        encodeGroupList.Clear();

        encodeObjectList.Clear();

        // Encode ProgNiProvider_1
        //
        encodeObjectList.AddAscii("Channel", "ProgChannel_1")
            .AddAscii("Logger", "ProgLogger_1")
            .AddAscii("Directory", "ProgDirectory_1")
            .AddUInt("LoginRequestTimeOut", 2030)
            .AddInt("DispatchTimeoutApiThread", 2040)
            .AddUInt("ItemCountHint", 2050)
            .AddUInt("MaxDispatchCountApiThread", 2060)
            .AddUInt("MaxDispatchCountUserThread", 2070)
            .AddUInt("MergeSourceDirectoryStreams", 0)
            .AddUInt("RecoverUserSubmitSourceDirectory", 0)
            .AddUInt("RefreshFirstRequired", 0)
            .AddUInt("RemoveItemsOnDisconnect", 0)
            .AddUInt("RequestTimeout", 2140)
            .AddInt("ReconnectAttemptLimit", 2100)
            .AddInt("ReconnectMaxDelay", 2120)
            .AddInt("ReconnectMinDelay", 2110)
            .AddUInt("ServiceCountHint", 2160)
            // XML tracing block
            .AddUInt("XmlTraceToStdout", 0)
            .AddUInt("XmlTraceToFile", 1)
            .AddUInt("XmlTraceMaxFileSize", (ulong)5_000_000)
            .AddAscii("XmlTraceFileName", "NotEmaTrace")
            .AddUInt("XmlTraceToMultipleFiles", 1)
            .AddUInt("XmlTraceWrite", 0)
            .AddUInt("XmlTraceRead", 0)
            .AddUInt("XmlTracePing", 1)
            .MarkForClear().Complete();

        innerMap.AddKeyAscii("ProgNiProvider_1", MapAction.ADD, encodeObjectList);

        encodeObjectList.Clear();

        encodeObjectList.AddAscii("ChannelSet", "ProgChannel_1, ProgChannel_2")
            .MarkForClear().Complete();

        innerMap.AddKeyAscii("ProgNiProvider_2", MapAction.ADD, encodeObjectList);

        innerMap.MarkForClear().Complete();

        encodeGroupList.Clear();

        encodeGroupList.AddAscii("DefaultNiProvider", "ProgNiProvider_1")
            .AddMap("NiProviderList", innerMap)
            .MarkForClear().Complete();

        outerMap.AddKeyAscii("NiProviderGroup", MapAction.ADD, encodeGroupList);


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
            .MarkForClear().Complete();

        innerMap.AddKeyAscii("ProgChannel_1", MapAction.ADD, encodeObjectList);


        // Second channel is all defaults, so add an empty Element List
        encodeObjectList.Clear();
        encodeObjectList.MarkForClear().Complete();

        innerMap.AddKeyAscii("ProgChannel_2", MapAction.ADD, encodeObjectList)
            .MarkForClear().Complete();

        encodeGroupList.AddMap("ChannelList", innerMap)
            .MarkForClear().Complete();

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
            .MarkForClear().Complete();

        innerMap.AddKeyAscii("ProgLogger_1", MapAction.ADD, encodeObjectList);

        // Blank logger config
        encodeObjectList.Clear();
        encodeObjectList.MarkForClear().Complete();

        innerMap.AddKeyAscii("ProgLogger_2", MapAction.ADD, encodeObjectList)
            .MarkForClear().Complete();

        encodeGroupList.AddMap("LoggerList", innerMap)
            .MarkForClear().Complete();

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
            .MarkForClear().Complete();

        innerMap.AddKeyAscii("ProgDictionary_1", MapAction.ADD, encodeObjectList);

        encodeObjectList.Clear();
        encodeObjectList.MarkForClear().Complete();

        innerMap.AddKeyAscii("ProgDictionary_2", MapAction.ADD, encodeObjectList)
            .MarkForClear().Complete();

        encodeGroupList.AddMap("DictionaryList", innerMap)
            .MarkForClear().Complete();

        outerMap.AddKeyAscii("DictionaryGroup", MapAction.ADD, encodeGroupList);

        // Encode a couple of directories.
        // Directory Group element list
        encodeGroupList.Clear();
        // DirectoryList map
        innerMap.Clear();
        // Services map
        serviceMap.Clear();
        // element list for the filter entries(info, state, etc.)
        serviceFilterList.Clear();
        // Filter elements element list
        serviceElementList.Clear();

        // Additional elements that need to be encoded.
        qosSeries.Clear();
        serviceArray.Clear();
        innerServiceElementList.Clear();

        // Default directory of progDirectory.
        encodeGroupList.AddAscii("DefaultDirectory", "progDirectory_1");

        // Start encoding the first directory here, starting with the first service
        // Encode the InfoFilter
        serviceElementList.AddUInt("ServiceId", 11)
                            .AddAscii("Vendor", "progVendor")
                            .AddUInt("IsSource", 1);

        serviceArray.AddUInt(130)
                    .AddUInt(5)
                    .MarkForClear().Complete();

        serviceElementList.AddArray("Capabilities", serviceArray)
                            .AddUInt("AcceptingConsumerStatus", 1)
                            .AddAscii("ItemList", "progItem");

        serviceArray.Clear();
        serviceArray.AddAscii("ProgDictionary_1")
                    .AddAscii("ProgDictionary_2")
                    .MarkForClear().Complete();
        serviceElementList.AddArray("DictionariesProvided", serviceArray);

        serviceArray.Clear();
        serviceArray.AddAscii("ProgDictionary_1")
                    .AddAscii("ProgDictionary_2")
                    .MarkForClear().Complete();
        serviceElementList.AddArray("DictionariesUsed", serviceArray);

        qosSeries.Clear();
        innerServiceElementList.Clear();
        innerServiceElementList.AddAscii("Timeliness", "Timeliness::InexactDelayed")
                                .AddAscii("Rate", "Rate::JustInTimeConflated")
                                .MarkForClear().Complete();

        qosSeries.AddEntry(innerServiceElementList);

        innerServiceElementList.Clear();
        innerServiceElementList.AddUInt("Timeliness", 100)
                                .AddUInt("Rate", 200)
                                .MarkForClear().Complete();
        qosSeries.AddEntry(innerServiceElementList)
                    .MarkForClear().Complete();

        serviceElementList.AddSeries("QoS", qosSeries);

        serviceElementList.AddUInt("SupportsQoSRange", 1)
                            .AddUInt("SupportsOutOfBandSnapshots", 1)
                            .MarkForClear().Complete();

        serviceFilterList.AddElementList("InfoFilter", serviceElementList);

        // Encode the State filter
        serviceElementList.Clear();

        serviceElementList.AddUInt("ServiceState", 0)
                            .AddUInt("AcceptingRequests", 1);

        innerServiceElementList.Clear();
        innerServiceElementList.AddAscii("StreamState", "StreamState::ClosedRedirected")
                                .AddAscii("DataState", "DataState::Suspect")
                                .AddAscii("StatusCode", "StatusCode::NotOpen")
                                .AddAscii("StatusText", "ProgStatusText")
                                .MarkForClear().Complete();

        serviceElementList.AddElementList("Status", innerServiceElementList)
                            .MarkForClear().Complete();

        serviceFilterList.AddElementList("StateFilter", serviceElementList);

        // Encode the Load Filter
        serviceElementList.Clear();

        serviceElementList.AddUInt("OpenLimit", 500)
                            .AddUInt("OpenWindow", 600)
                            .AddUInt("LoadFactor", 700)
                            .MarkForClear().Complete();
        serviceFilterList.AddElementList("LoadFilter", serviceElementList)
                            .MarkForClear().Complete();

        serviceMap.AddKeyAscii("ProgService_1", MapAction.ADD, serviceFilterList);

        // Encode an all-default service, except for service Id and capabilities
        serviceFilterList.Clear();

        serviceElementList.Clear();
        serviceArray.Clear();
        serviceElementList.AddUInt("ServiceId", 12)
                            .AddUInt("IsSource", 1);

        serviceArray.AddAscii("MMT_HISTORY")
                    .MarkForClear().Complete();

        serviceElementList.AddArray("Capabilities", serviceArray)
                            .MarkForClear().Complete();
        serviceFilterList.AddElementList("InfoFilter", serviceElementList)
                        .MarkForClear().Complete();

        serviceMap.AddKeyAscii("ProgService_2", MapAction.ADD, serviceFilterList)
                    .MarkForClear().Complete();

        innerMap.AddKeyAscii("ProgDirectory_1", MapAction.ADD, serviceMap)
                .MarkForClear().Complete();

        encodeGroupList.AddMap("DirectoryList", innerMap)
                        .MarkForClear().Complete();

        outerMap.AddKeyAscii("DirectoryGroup", MapAction.ADD, encodeGroupList);

        outerMap.MarkForClear().Complete();

        niProvConfigImpl.Config(outerMap);

        Assert.Equal(2, niProvConfigImpl.NiProviderConfigMap.Count);
        Assert.Equal(2, niProvConfigImpl.ClientChannelConfigMap.Count);
        Assert.Equal(2, niProvConfigImpl.LoggerConfigMap.Count);
        Assert.Equal(2, niProvConfigImpl.DictionaryConfigMap.Count);
        Assert.Single(niProvConfigImpl.DirectoryConfigMap);

        Assert.Empty(niProvConfigImpl.ConfigErrorLog!.ErrorList);

        testNiProvConfig = niProvConfigImpl.NiProviderConfigMap["ProgNiProvider_1"];

        Assert.Equal("ProgNiProvider_1", testNiProvConfig.Name);
        Assert.Single(testNiProvConfig.ChannelSet);
        Assert.Equal("ProgChannel_1", testNiProvConfig.ChannelSet[0]);
        Assert.Equal("ProgLogger_1", testNiProvConfig.Logger);
        Assert.Equal("ProgDirectory_1", testNiProvConfig.Directory);
        Assert.Equal((long)2030, testNiProvConfig.LoginRequestTimeOut);
        Assert.Equal(2040, testNiProvConfig.DispatchTimeoutApiThread);
        Assert.Equal((ulong)2050, testNiProvConfig.ItemCountHint);
        Assert.Equal((int)2060, testNiProvConfig.MaxDispatchCountApiThread);
        Assert.Equal((int)2070, testNiProvConfig.MaxDispatchCountUserThread);
        Assert.False(testNiProvConfig.MergeSourceDirectoryStreams);
        Assert.Equal(2100, testNiProvConfig.ReconnectAttemptLimit);
        Assert.Equal(2120, testNiProvConfig.ReconnectMaxDelay);
        Assert.Equal(2110, testNiProvConfig.ReconnectMinDelay);
        Assert.False(testNiProvConfig.RecoverUserSubmitSourceDirectory);
        Assert.False(testNiProvConfig.RefreshFirstRequired);
        Assert.False(testNiProvConfig.RemoveItemsOnDisconnect);
        Assert.Equal((ulong)2140, testNiProvConfig.RequestTimeout);
        Assert.Equal((int)2160, testNiProvConfig.ServiceCountHint);
        // XML tracing block
        Assert.False(testNiProvConfig.XmlTraceToStdout);
        Assert.True(testNiProvConfig.XmlTraceToFile);
        Assert.Equal((ulong)5_000_000, testNiProvConfig.XmlTraceMaxFileSize);
        Assert.Equal("NotEmaTrace", testNiProvConfig.XmlTraceFileName);
        Assert.True(testNiProvConfig.XmlTraceToMultipleFiles);
        Assert.False(testNiProvConfig.XmlTraceWrite);
        Assert.False(testNiProvConfig.XmlTraceRead);
        Assert.True(testNiProvConfig.XmlTracePing);


        // TestConsumer_2 has all defaults except for ChannelSet.
        testNiProvConfig = niProvConfigImpl.NiProviderConfigMap["ProgNiProvider_2"];

        Assert.Equal("ProgNiProvider_2", testNiProvConfig.Name);
        Assert.Equal(2, testNiProvConfig.ChannelSet.Count);
        Assert.Equal("ProgChannel_1", testNiProvConfig.ChannelSet[0]);
        Assert.Equal("ProgChannel_2", testNiProvConfig.ChannelSet[1]);
        Assert.Equal(defaultNiProviderConfig.Directory, testNiProvConfig.Directory);
        Assert.Equal(defaultNiProviderConfig.LoginRequestTimeOut, testNiProvConfig.LoginRequestTimeOut);
        Assert.Equal(defaultNiProviderConfig.DispatchTimeoutApiThread, testNiProvConfig.DispatchTimeoutApiThread);
        Assert.Equal(defaultNiProviderConfig.ItemCountHint, testNiProvConfig.ItemCountHint);
        Assert.Equal(defaultNiProviderConfig.MaxDispatchCountApiThread, testNiProvConfig.MaxDispatchCountApiThread);
        Assert.Equal(defaultNiProviderConfig.MaxDispatchCountUserThread, testNiProvConfig.MaxDispatchCountUserThread);
        Assert.Equal(defaultNiProviderConfig.MergeSourceDirectoryStreams, testNiProvConfig.MergeSourceDirectoryStreams);
        Assert.Equal(defaultNiProviderConfig.ReconnectAttemptLimit, testNiProvConfig.ReconnectAttemptLimit);
        Assert.Equal(defaultNiProviderConfig.ReconnectMaxDelay, testNiProvConfig.ReconnectMaxDelay);
        Assert.Equal(defaultNiProviderConfig.ReconnectMinDelay, testNiProvConfig.ReconnectMinDelay);
        Assert.Equal(defaultNiProviderConfig.RecoverUserSubmitSourceDirectory, testNiProvConfig.RecoverUserSubmitSourceDirectory);
        Assert.Equal(defaultNiProviderConfig.RefreshFirstRequired, testNiProvConfig.RefreshFirstRequired);
        Assert.Equal(defaultNiProviderConfig.RemoveItemsOnDisconnect, testNiProvConfig.RemoveItemsOnDisconnect);
        Assert.Equal(defaultNiProviderConfig.RequestTimeout, testNiProvConfig.RequestTimeout);
        Assert.Equal(defaultNiProviderConfig.ServiceCountHint, testNiProvConfig.ServiceCountHint);
  
        testChannelConfig = niProvConfigImpl.ClientChannelConfigMap["ProgChannel_1"];

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
        testChannelConfig = niProvConfigImpl.ClientChannelConfigMap["ProgChannel_2"];

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

        testLoggerConfig = niProvConfigImpl.LoggerConfigMap["ProgLogger_1"];

        Assert.Equal("ProgLogger_1", testLoggerConfig.Name);
        Assert.Equal(LoggerType.STDOUT, testLoggerConfig.LoggerType);
        Assert.Equal(LoggerLevel.INFO, testLoggerConfig.LoggerSeverity);
        Assert.Equal("ProgLogFile", testLoggerConfig.FileName);
        Assert.Equal((ulong)1, testLoggerConfig.IncludeDateInLoggerOutput);
        Assert.Equal((ulong)20, testLoggerConfig.NumberOfLogFiles);
        Assert.Equal((ulong)100, testLoggerConfig.MaxLogFileSize);

        // ProgLogger_2 is all defaults
        testLoggerConfig = niProvConfigImpl.LoggerConfigMap["ProgLogger_2"];

        Assert.Equal("ProgLogger_2", testLoggerConfig.Name);
        Assert.Equal(defaultLoggerConfig.LoggerType, testLoggerConfig.LoggerType);
        Assert.Equal(defaultLoggerConfig.LoggerSeverity, testLoggerConfig.LoggerSeverity);
        Assert.Equal(defaultLoggerConfig.FileName, testLoggerConfig.FileName);
        Assert.Equal(defaultLoggerConfig.IncludeDateInLoggerOutput, testLoggerConfig.IncludeDateInLoggerOutput);
        Assert.Equal(defaultLoggerConfig.NumberOfLogFiles, testLoggerConfig.NumberOfLogFiles);
        Assert.Equal(defaultLoggerConfig.MaxLogFileSize, testLoggerConfig.MaxLogFileSize);

        testDictConfig = niProvConfigImpl.DictionaryConfigMap["ProgDictionary_1"];

        Assert.Equal("ProgDictionary_1", testDictConfig.Name);
        Assert.Equal(EmaConfig.DictionaryTypeEnum.FILE, testDictConfig.DictionaryType);
        Assert.Equal("ProgEnumFile", testDictConfig.EnumTypeDefFileName);
        Assert.Equal("ProgEnumItem", testDictConfig.EnumTypeDefItemName);
        Assert.Equal("ProgFieldFile", testDictConfig.RdmFieldDictionaryFileName);
        Assert.Equal("ProgFieldItem", testDictConfig.RdmFieldDictionaryItemName);

        // ProgDictionary_1 is set to defaults
        testDictConfig = niProvConfigImpl.DictionaryConfigMap["ProgDictionary_2"];

        Assert.Equal("ProgDictionary_2", testDictConfig.Name);
        Assert.Equal(defaultDictConfig.DictionaryType, testDictConfig.DictionaryType);
        Assert.Equal(defaultDictConfig.EnumTypeDefFileName, testDictConfig.EnumTypeDefFileName);
        Assert.Equal(defaultDictConfig.EnumTypeDefItemName, testDictConfig.EnumTypeDefItemName);
        Assert.Equal(defaultDictConfig.RdmFieldDictionaryFileName, testDictConfig.RdmFieldDictionaryFileName);
        Assert.Equal(defaultDictConfig.RdmFieldDictionaryItemName, testDictConfig.RdmFieldDictionaryItemName);

        // Verify the Directory configuration here
        // TestDictionary_2 is set to defaults
        testDirectoryConfig = niProvConfigImpl.DirectoryConfigMap["ProgDirectory_1"];

        Assert.Equal("ProgDirectory_1", testDirectoryConfig.Name);
        Assert.Equal(2, testDirectoryConfig.ServiceMap.Count);

        testServiceConfig = testDirectoryConfig.ServiceMap["ProgService_1"];

        // Check the Info filter
        Assert.True(testServiceConfig.Service.HasInfo);
        Assert.Equal("ProgService_1", testServiceConfig.Service.Info.ServiceName.ToString());
        Assert.Equal(11, testServiceConfig.Service.ServiceId);
        Assert.True(testServiceConfig.Service.Info.HasVendor);
        Assert.Equal("progVendor", testServiceConfig.Service.Info.Vendor.ToString());
        Assert.True(testServiceConfig.Service.Info.HasIsSource);
        Assert.Equal(1, testServiceConfig.Service.Info.IsSource);
        Assert.Equal(2, testServiceConfig.Service.Info.CapabilitiesList.Count);
        Assert.Equal(130, testServiceConfig.Service.Info.CapabilitiesList[0]);
        Assert.Equal(5, testServiceConfig.Service.Info.CapabilitiesList[1]);
        Assert.True(testServiceConfig.Service.Info.HasAcceptingConsStatus);
        Assert.Equal(1, testServiceConfig.Service.Info.AcceptConsumerStatus);
        Assert.True(testServiceConfig.Service.Info.HasItemList);
        Assert.Equal("progItem", testServiceConfig.Service.Info.ItemList.ToString());
        // The dictionaries provided and used in the service will be empty at this point.
        Assert.False(testServiceConfig.Service.Info.HasDictionariesProvided);
        Assert.Empty(testServiceConfig.Service.Info.DictionariesProvidedList);
        Assert.Equal(2, testServiceConfig.DictionariesProvidedList.Count);
        Assert.Equal("ProgDictionary_1", testServiceConfig.DictionariesProvidedList[0]);
        Assert.Equal("ProgDictionary_2", testServiceConfig.DictionariesProvidedList[1]);
        Assert.False(testServiceConfig.Service.Info.HasDictionariesUsed);
        Assert.Empty(testServiceConfig.Service.Info.DictionariesUsedList);
        Assert.Equal(2, testServiceConfig.DictionariesUsedList.Count);
        Assert.Equal("ProgDictionary_1", testServiceConfig.DictionariesUsedList[0]);
        Assert.Equal("ProgDictionary_2", testServiceConfig.DictionariesUsedList[1]);
        Assert.True(testServiceConfig.Service.Info.HasQos);
        Assert.Equal(2, testServiceConfig.Service.Info.QosList.Count);
        Assert.Equal(QosTimeliness.DELAYED_UNKNOWN, testServiceConfig.Service.Info.QosList[0].Timeliness());
        Assert.Equal(0, testServiceConfig.Service.Info.QosList[0].TimeInfo());
        Assert.Equal(QosRates.JIT_CONFLATED, testServiceConfig.Service.Info.QosList[0].Rate());
        Assert.Equal(0, testServiceConfig.Service.Info.QosList[0].RateInfo());
        Assert.Equal(QosTimeliness.DELAYED, testServiceConfig.Service.Info.QosList[1].Timeliness());
        Assert.Equal(100, testServiceConfig.Service.Info.QosList[1].TimeInfo());
        Assert.Equal(QosRates.TIME_CONFLATED, testServiceConfig.Service.Info.QosList[1].Rate());
        Assert.Equal(200, testServiceConfig.Service.Info.QosList[1].RateInfo());
        Assert.True(testServiceConfig.Service.Info.HasSupportQosRange);
        Assert.Equal(1, testServiceConfig.Service.Info.SupportsQosRange);
        Assert.True(testServiceConfig.Service.Info.HasSupportOOBSnapshots);
        Assert.Equal(1, testServiceConfig.Service.Info.SupportsOOBSnapshots);
        // Check the state filter
        Assert.True(testServiceConfig.Service.HasState);
        Assert.Equal(0, testServiceConfig.Service.State.ServiceStateVal);
        Assert.True(testServiceConfig.Service.State.HasAcceptingRequests);
        Assert.Equal(1, testServiceConfig.Service.State.AcceptingRequests);
        Assert.True(testServiceConfig.Service.State.HasStatus);
        Assert.Equal(StreamStates.REDIRECTED, testServiceConfig.Service.State.Status.StreamState());
        Assert.Equal(DataStates.SUSPECT, testServiceConfig.Service.State.Status.DataState());
        Assert.Equal(OmmState.StatusCodes.NOT_OPEN, testServiceConfig.Service.State.Status.Code());
        Assert.Equal("ProgStatusText", testServiceConfig.Service.State.Status.Text().ToString());
        // Check the load filter
        Assert.True(testServiceConfig.Service.HasLoad);
        Assert.True(testServiceConfig.Service.Load.HasOpenLimit);
        Assert.Equal(500, testServiceConfig.Service.Load.OpenLimit);
        Assert.True(testServiceConfig.Service.Load.HasOpenWindow);
        Assert.Equal(600, testServiceConfig.Service.Load.OpenWindow);
        Assert.True(testServiceConfig.Service.Load.HasLoadFactor);
        Assert.Equal(700, testServiceConfig.Service.Load.LoadFactor);

        testServiceConfig = testDirectoryConfig.ServiceMap["ProgService_2"];

        // This is EMA defaults.
        // Check the Info filter
        Assert.True(testServiceConfig.Service.HasInfo);
        Assert.Equal("ProgService_2", testServiceConfig.Service.Info.ServiceName.ToString());
        Assert.Equal(12, testServiceConfig.Service.ServiceId);
        Assert.False(testServiceConfig.Service.Info.HasVendor);
        Assert.True(testServiceConfig.Service.Info.HasIsSource);
        Assert.Equal(1, testServiceConfig.Service.Info.IsSource);
        Assert.Single(testServiceConfig.Service.Info.CapabilitiesList);
        Assert.Equal(EmaRdm.MMT_HISTORY, testServiceConfig.Service.Info.CapabilitiesList[0]);
        Assert.True(testServiceConfig.Service.Info.HasAcceptingConsStatus);
        Assert.Equal(1, testServiceConfig.Service.Info.AcceptConsumerStatus);
        Assert.False(testServiceConfig.Service.Info.HasItemList);
        // The dictionaries provided and used in the service will be empty at this point.
        Assert.False(testServiceConfig.Service.Info.HasDictionariesProvided);
        Assert.Empty(testServiceConfig.Service.Info.DictionariesProvidedList);
        Assert.Empty(testServiceConfig.DictionariesProvidedList);
        Assert.False(testServiceConfig.Service.Info.HasDictionariesUsed);
        Assert.Empty(testServiceConfig.Service.Info.DictionariesUsedList);
        Assert.Empty(testServiceConfig.DictionariesUsedList);
        Assert.True(testServiceConfig.Service.Info.HasQos);
        Assert.Empty(testServiceConfig.Service.Info.QosList);
        Assert.True(testServiceConfig.Service.Info.HasSupportOOBSnapshots);
        Assert.Equal(0, testServiceConfig.Service.Info.SupportsOOBSnapshots);
        // Check the state filter
        Assert.True(testServiceConfig.Service.HasState);
        Assert.Equal(1, testServiceConfig.Service.State.ServiceStateVal);
        Assert.True(testServiceConfig.Service.State.HasAcceptingRequests);
        Assert.Equal(0, testServiceConfig.Service.State.AcceptingRequests);
        Assert.True(testServiceConfig.Service.State.HasStatus);
        Assert.Equal(StreamStates.OPEN, testServiceConfig.Service.State.Status.StreamState());
        Assert.Equal(DataStates.OK, testServiceConfig.Service.State.Status.DataState());
        Assert.Equal(OmmState.StatusCodes.NONE, testServiceConfig.Service.State.Status.Code());
        Assert.Equal(0, testServiceConfig.Service.State.Status.Text().Length);
        // Check the load filter
        Assert.True(testServiceConfig.Service.HasLoad);
        Assert.False(testServiceConfig.Service.Load.HasOpenLimit);
        Assert.False(testServiceConfig.Service.Load.HasOpenWindow);
        Assert.False(testServiceConfig.Service.Load.HasLoadFactor);

        // Verify that the config works here
        niProvConfigImpl.VerifyConfiguration();


        // Overlay with changes to all of the _1 elements
        outerMap.Clear();

        innerMap.Clear();

        encodeGroupList.Clear();

        encodeObjectList.Clear();

        // Encode ProgNiProvider_1
        //
        encodeObjectList.AddAscii("ChannelSet", "ProgChannel_1, ProgChannel_2")
            .AddAscii("Logger", "ProgLogger_1")
            .AddAscii("Directory", "ProgDirectory_1")
            .AddUInt("LoginRequestTimeOut", 3030)
            .AddInt("DispatchTimeoutApiThread", 3040)
            .AddUInt("ItemCountHint", 3050)
            .AddUInt("MaxDispatchCountApiThread", 3060)
            .AddUInt("MaxDispatchCountUserThread", 3070)
            .AddUInt("MergeSourceDirectoryStreams", 1)
            .AddUInt("RecoverUserSubmitSourceDirectory", 1)
            .AddUInt("RefreshFirstRequired", 1)
            .AddUInt("RemoveItemsOnDisconnect", 1)
            .AddUInt("RequestTimeout", 3140)
            .AddInt("ReconnectAttemptLimit", 3100)
            .AddInt("ReconnectMaxDelay", 3120)
            .AddInt("ReconnectMinDelay", 3110)
            .AddUInt("ServiceCountHint", 3160)
            // XML tracing block
            .AddUInt("XmlTraceToStdout", 0)
            .AddUInt("XmlTraceToFile", 1)
            .AddUInt("XmlTraceMaxFileSize", (ulong)5_000_000)
            .AddAscii("XmlTraceFileName", "NotEmaTrace")
            .AddUInt("XmlTraceToMultipleFiles", 1)
            .AddUInt("XmlTraceWrite", 0)
            .AddUInt("XmlTraceRead", 0)
            .AddUInt("XmlTracePing", 1)
            .MarkForClear().Complete();

        innerMap.AddKeyAscii("ProgNiProvider_1", MapAction.ADD, encodeObjectList);

        encodeObjectList.Clear();

        encodeObjectList.AddAscii("ChannelSet", "ProgChannel_1, ProgChannel_2")
            .MarkForClear().Complete();

        innerMap.AddKeyAscii("ProgNiProvider_2", MapAction.ADD, encodeObjectList);

        innerMap.MarkForClear().Complete();

        encodeGroupList.Clear();

        encodeGroupList.AddAscii("DefaultNiProvider", "ProgNiProvider_1")
            .AddMap("NiProviderList", innerMap)
            .MarkForClear().Complete();

        outerMap.AddKeyAscii("NiProviderGroup", MapAction.ADD, encodeGroupList);


        // Start encoding the channel connection information
        encodeGroupList.Clear();
        innerMap.Clear();
        encodeObjectList.Clear();

        encodeObjectList.AddEnum("ChannelType", ConnectionTypeEnum.SOCKET)
            .AddEnum("EncryptedProtocolType", ConnectionTypeEnum.SOCKET)
            .AddUInt("ConnectionPingTimeout", 2000)
            .AddUInt("EnableSessionManagement", 0)
            .AddUInt("GuaranteedOutputBuffers", 2010)
            .AddUInt("HighWaterMark", 2020)
            .AddUInt("InitializationTimeout", 2030)
            .AddUInt("AuthenticationTimeout", 2115)
            .AddAscii("InterfaceName", "NewProgInterface_1")
            .AddAscii("Location", "NewProgLocation_1")
            .AddUInt("NumInputBuffers", 2040)
            .AddUInt("ServiceDiscoveryRetryCount", 2050)
            .AddUInt("SysRecvBufSize", 2060)
            .AddUInt("SysSendBufSize", 2070)
            .AddEnum("CompressionType", CompressionTypeEnum.ZLIB)
            .AddUInt("DirectWrite", 0)
            .AddAscii("Host", "NewProgHost_1")
            .AddAscii("Port", "NewProgPort_1")
            .AddAscii("ProxyHost", "NewProgProxyHost_1")
            .AddAscii("ProxyPort", "NewProgProxyPort_1")
            .AddUInt("TcpNodelay", 0)
            .AddUInt("SecurityProtocol", EncryptedTLSProtocolFlags.TLSv1_2 | 0x01)
            .AddUInt("CompressionThreshold", 1666)
            .MarkForClear().Complete();

        innerMap.AddKeyAscii("ProgChannel_1", MapAction.ADD, encodeObjectList);


        // Second channel is all defaults, so add an empty Element List
        encodeObjectList.Clear();
        encodeObjectList.MarkForClear().Complete();

        innerMap.AddKeyAscii("ProgChannel_2", MapAction.ADD, encodeObjectList)
            .MarkForClear().Complete();

        encodeGroupList.AddMap("ChannelList", innerMap)
            .MarkForClear().Complete();

        outerMap.AddKeyAscii("ChannelGroup", MapAction.ADD, encodeGroupList);

        // Start encoding the Logger information
        encodeGroupList.Clear();
        innerMap.Clear();
        encodeObjectList.Clear();

        encodeObjectList.AddAscii("FileName", "NewProgLogFile")
            .AddUInt("IncludeDateInLoggerOutput", 0)
            .AddUInt("NumberOfLogFiles", 120)
            .AddUInt("MaxLogFileSize", 1100)
            .AddEnum("LoggerSeverity", LoggerLevelEnum.WARNING)
            .AddEnum("LoggerType", LoggerTypeEnum.FILE)
            .MarkForClear().Complete();

        innerMap.AddKeyAscii("ProgLogger_1", MapAction.ADD, encodeObjectList);

        // Blank logger config
        encodeObjectList.Clear();
        encodeObjectList.MarkForClear().Complete();

        innerMap.AddKeyAscii("ProgLogger_2", MapAction.ADD, encodeObjectList)
            .MarkForClear().Complete();

        encodeGroupList.AddMap("LoggerList", innerMap)
            .MarkForClear().Complete();

        outerMap.AddKeyAscii("LoggerGroup", MapAction.ADD, encodeGroupList);

        // Start encoding the Dictionary information
        encodeGroupList.Clear();
        innerMap.Clear();
        encodeObjectList.Clear();

        encodeObjectList.AddAscii("EnumTypeDefFileName", "NewProgEnumFile")
            .AddAscii("EnumTypeDefItemName", "NewProgEnumItem")
            .AddAscii("RdmFieldDictionaryFileName", "NewProgFieldFile")
            .AddAscii("RdmFieldDictionaryItemName", "NewProgFieldItem")
            .AddEnum("DictionaryType", DictionaryTypeEnum.CHANNEL)
            .MarkForClear().Complete();

        innerMap.AddKeyAscii("ProgDictionary_1", MapAction.ADD, encodeObjectList);

        encodeObjectList.Clear();
        encodeObjectList.MarkForClear().Complete();

        innerMap.AddKeyAscii("ProgDictionary_2", MapAction.ADD, encodeObjectList)
            .MarkForClear().Complete();

        encodeGroupList.AddMap("DictionaryList", innerMap)
            .MarkForClear().Complete();

        outerMap.AddKeyAscii("DictionaryGroup", MapAction.ADD, encodeGroupList);

        // Encode a couple of directories.
        // Directory Group element list
        encodeGroupList.Clear();
        // DirectoryList map
        innerMap.Clear();
        // Services map
        serviceMap.Clear();
        // element list for the filter entries(info, state, etc.)
        serviceFilterList.Clear();
        // Filter elements element list
        serviceElementList.Clear();

        // Additional elements that need to be encoded.
        qosSeries.Clear();
        serviceArray.Clear();
        innerServiceElementList.Clear();

        // Default directory of progDirectory.
        encodeGroupList.AddAscii("DefaultDirectory", "progDirectory_1");

        // Start encoding the first directory here, starting with the first service
        // Encode the InfoFilter
        serviceElementList.AddUInt("ServiceId", 111)
                            .AddAscii("Vendor", "newProgVendor")
                            .AddUInt("IsSource", 0);

        serviceArray.AddUInt(131)
                    .AddUInt(6)
                    .MarkForClear().Complete();

        serviceElementList.AddArray("Capabilities", serviceArray)
                            .AddUInt("AcceptingConsumerStatus", 0)
                            .AddAscii("ItemList", "newProgItem");

        serviceArray.Clear();
        serviceArray.AddAscii("ProgDictionary_1")
                    .MarkForClear().Complete();
        serviceElementList.AddArray("DictionariesProvided", serviceArray);

        serviceArray.Clear();
        serviceArray.AddAscii("ProgDictionary_1")
                    .MarkForClear().Complete();
        serviceElementList.AddArray("DictionariesUsed", serviceArray);

        qosSeries.Clear();
        innerServiceElementList.Clear();
        innerServiceElementList.AddAscii("Timeliness", "Timeliness::RealTime")
                                .AddAscii("Rate", "Rate::TickByTick")
                                .MarkForClear().Complete();

        qosSeries.AddEntry(innerServiceElementList);

        innerServiceElementList.Clear();
        innerServiceElementList.AddUInt("Timeliness", 1100)
                                .AddUInt("Rate", 1200)
                                .MarkForClear().Complete();
        qosSeries.AddEntry(innerServiceElementList)
                    .MarkForClear().Complete();

        serviceElementList.AddSeries("QoS", qosSeries);

        serviceElementList.AddUInt("SupportsQoSRange", 0)
                            .AddUInt("SupportsOutOfBandSnapshots", 0)
                            .MarkForClear().Complete();

        serviceFilterList.AddElementList("InfoFilter", serviceElementList);

        // Encode the State filter
        serviceElementList.Clear();

        serviceElementList.AddUInt("ServiceState", 1)
                            .AddUInt("AcceptingRequests", 0);

        innerServiceElementList.Clear();
        innerServiceElementList.AddAscii("StreamState", "StreamState::Closed")
                                .AddAscii("DataState", "DataState::Ok")
                                .AddAscii("StatusCode", "StatusCode::AlreadyOpen")
                                .AddAscii("StatusText", "NewProgStatusText")
                                .MarkForClear().Complete();

        serviceElementList.AddElementList("Status", innerServiceElementList)
                            .MarkForClear().Complete();

        serviceFilterList.AddElementList("StateFilter", serviceElementList);

        // Encode the Load Filter
        serviceElementList.Clear();

        serviceElementList.AddUInt("OpenLimit", 1500)
                            .AddUInt("OpenWindow", 1600)
                            .AddUInt("LoadFactor", 1700)
                            .MarkForClear().Complete();
        serviceFilterList.AddElementList("LoadFilter", serviceElementList)
                            .MarkForClear().Complete();

        serviceMap.AddKeyAscii("ProgService_1", MapAction.ADD, serviceFilterList);

        // Encode an all-default service, except for service Id and capabilities
        serviceFilterList.Clear();

        serviceElementList.Clear();
        serviceArray.Clear();
        serviceElementList.AddUInt("ServiceId", 112)
                            .AddUInt("IsSource", 0);

        serviceArray.AddAscii("MMT_STORY")
                    .MarkForClear().Complete();

        serviceElementList.AddArray("Capabilities", serviceArray)
                            .MarkForClear().Complete();
        serviceFilterList.AddElementList("InfoFilter", serviceElementList)
                        .MarkForClear().Complete();

        serviceMap.AddKeyAscii("ProgService_2", MapAction.ADD, serviceFilterList)
                    .MarkForClear().Complete();

        innerMap.AddKeyAscii("ProgDirectory_1", MapAction.ADD, serviceMap)
                .MarkForClear().Complete();

        encodeGroupList.AddMap("DirectoryList", innerMap)
                        .MarkForClear().Complete();

        outerMap.AddKeyAscii("DirectoryGroup", MapAction.ADD, encodeGroupList);

        outerMap.MarkForClear().Complete();

        niProvConfigImpl.Config(outerMap);

        Assert.Equal(2, niProvConfigImpl.NiProviderConfigMap.Count);
        Assert.Equal(2, niProvConfigImpl.ClientChannelConfigMap.Count);
        Assert.Equal(2, niProvConfigImpl.LoggerConfigMap.Count);
        Assert.Equal(2, niProvConfigImpl.DictionaryConfigMap.Count);
        Assert.Single(niProvConfigImpl.DirectoryConfigMap);

        Assert.Empty(niProvConfigImpl.ConfigErrorLog!.ErrorList);

        testNiProvConfig = niProvConfigImpl.NiProviderConfigMap["ProgNiProvider_1"];

        Assert.Equal("ProgNiProvider_1", testNiProvConfig.Name);
        Assert.Equal(2, testNiProvConfig.ChannelSet.Count);
        Assert.Equal("ProgChannel_1", testNiProvConfig.ChannelSet[0]);
        Assert.Equal("ProgChannel_2", testNiProvConfig.ChannelSet[1]);
        Assert.Equal("ProgLogger_1", testNiProvConfig.Logger);
        Assert.Equal("ProgDirectory_1", testNiProvConfig.Directory);
        Assert.Equal((long)3030, testNiProvConfig.LoginRequestTimeOut);
        Assert.Equal(3040, testNiProvConfig.DispatchTimeoutApiThread);
        Assert.Equal((ulong)3050, testNiProvConfig.ItemCountHint);
        Assert.Equal((int)3060, testNiProvConfig.MaxDispatchCountApiThread);
        Assert.Equal((int)3070, testNiProvConfig.MaxDispatchCountUserThread);
        Assert.True(testNiProvConfig.MergeSourceDirectoryStreams);
        Assert.Equal(3100, testNiProvConfig.ReconnectAttemptLimit);
        Assert.Equal(3120, testNiProvConfig.ReconnectMaxDelay);
        Assert.Equal(3110, testNiProvConfig.ReconnectMinDelay);
        Assert.True(testNiProvConfig.RecoverUserSubmitSourceDirectory);
        Assert.True(testNiProvConfig.RefreshFirstRequired);
        Assert.True(testNiProvConfig.RemoveItemsOnDisconnect);
        Assert.Equal((ulong)3140, testNiProvConfig.RequestTimeout);
        Assert.Equal((int)3160, testNiProvConfig.ServiceCountHint);
        // XML tracing block
        Assert.False(testNiProvConfig.XmlTraceToStdout);
        Assert.True(testNiProvConfig.XmlTraceToFile);
        Assert.Equal((ulong)5_000_000, testNiProvConfig.XmlTraceMaxFileSize);
        Assert.Equal("NotEmaTrace", testNiProvConfig.XmlTraceFileName);
        Assert.True(testNiProvConfig.XmlTraceToMultipleFiles);
        Assert.False(testNiProvConfig.XmlTraceWrite);
        Assert.False(testNiProvConfig.XmlTraceRead);
        Assert.True(testNiProvConfig.XmlTracePing);


        // TestConsumer_2 has all defaults except for ChannelSet.
        testNiProvConfig = niProvConfigImpl.NiProviderConfigMap["ProgNiProvider_2"];

        Assert.Equal("ProgNiProvider_2", testNiProvConfig.Name);
        Assert.Equal(2, testNiProvConfig.ChannelSet.Count);
        Assert.Equal("ProgChannel_1", testNiProvConfig.ChannelSet[0]);
        Assert.Equal("ProgChannel_2", testNiProvConfig.ChannelSet[1]);
        Assert.Equal(defaultNiProviderConfig.Directory, testNiProvConfig.Directory);
        Assert.Equal(defaultNiProviderConfig.LoginRequestTimeOut, testNiProvConfig.LoginRequestTimeOut);
        Assert.Equal(defaultNiProviderConfig.DispatchTimeoutApiThread, testNiProvConfig.DispatchTimeoutApiThread);
        Assert.Equal(defaultNiProviderConfig.ItemCountHint, testNiProvConfig.ItemCountHint);
        Assert.Equal(defaultNiProviderConfig.MaxDispatchCountApiThread, testNiProvConfig.MaxDispatchCountApiThread);
        Assert.Equal(defaultNiProviderConfig.MaxDispatchCountUserThread, testNiProvConfig.MaxDispatchCountUserThread);
        Assert.Equal(defaultNiProviderConfig.MergeSourceDirectoryStreams, testNiProvConfig.MergeSourceDirectoryStreams);
        Assert.Equal(defaultNiProviderConfig.ReconnectAttemptLimit, testNiProvConfig.ReconnectAttemptLimit);
        Assert.Equal(defaultNiProviderConfig.ReconnectMaxDelay, testNiProvConfig.ReconnectMaxDelay);
        Assert.Equal(defaultNiProviderConfig.ReconnectMinDelay, testNiProvConfig.ReconnectMinDelay);
        Assert.Equal(defaultNiProviderConfig.RecoverUserSubmitSourceDirectory, testNiProvConfig.RecoverUserSubmitSourceDirectory);
        Assert.Equal(defaultNiProviderConfig.RefreshFirstRequired, testNiProvConfig.RefreshFirstRequired);
        Assert.Equal(defaultNiProviderConfig.RemoveItemsOnDisconnect, testNiProvConfig.RemoveItemsOnDisconnect);
        Assert.Equal(defaultNiProviderConfig.RequestTimeout, testNiProvConfig.RequestTimeout);
        Assert.Equal(defaultNiProviderConfig.ServiceCountHint, testNiProvConfig.ServiceCountHint);

        testChannelConfig = niProvConfigImpl.ClientChannelConfigMap["ProgChannel_1"];

        Assert.Equal("ProgChannel_1", testChannelConfig.Name);
        Assert.Equal(Eta.Transports.ConnectionType.SOCKET, testChannelConfig.ConnectInfo.ConnectOptions.ConnectionType);
        Assert.Equal(Eta.Transports.ConnectionType.SOCKET, testChannelConfig.ConnectInfo.ConnectOptions.EncryptionOpts.EncryptedProtocol);
        Assert.Equal(2, testChannelConfig.ConnectInfo.ConnectOptions.PingTimeout);
        Assert.False(testChannelConfig.ConnectInfo.EnableSessionManagement);
        Assert.Equal(2010, testChannelConfig.ConnectInfo.ConnectOptions.GuaranteedOutputBuffers);
        Assert.Equal(2020, testChannelConfig.HighWaterMark);
        Assert.Equal(2030, testChannelConfig.ConnectInfo.GetInitTimeout());
        Assert.Equal(2115, testChannelConfig.ConnectInfo.ConnectOptions.EncryptionOpts.AuthenticationTimeout);
        Assert.Equal("NewProgInterface_1", testChannelConfig.ConnectInfo.ConnectOptions.UnifiedNetworkInfo.InterfaceName);
        Assert.Equal("NewProgLocation_1", testChannelConfig.ConnectInfo.Location);
        Assert.Equal(2040, testChannelConfig.ConnectInfo.ConnectOptions.NumInputBuffers);
        Assert.Equal(2050, testChannelConfig.ConnectInfo.ServiceDiscoveryRetryCount);
        Assert.Equal(2060, testChannelConfig.ConnectInfo.ConnectOptions.SysRecvBufSize);
        Assert.Equal(2070, testChannelConfig.ConnectInfo.ConnectOptions.SysSendBufSize);
        Assert.Equal(Eta.Transports.CompressionType.ZLIB, testChannelConfig.ConnectInfo.ConnectOptions.CompressionType);
        Assert.False(testChannelConfig.DirectWrite);
        Assert.Equal("NewProgHost_1", testChannelConfig.ConnectInfo.ConnectOptions.UnifiedNetworkInfo.Address);
        Assert.Equal("NewProgPort_1", testChannelConfig.ConnectInfo.ConnectOptions.UnifiedNetworkInfo.ServiceName);
        Assert.Equal("NewProgProxyHost_1", testChannelConfig.ConnectInfo.ConnectOptions.ProxyOptions.ProxyHostName);
        Assert.Equal("NewProgProxyPort_1", testChannelConfig.ConnectInfo.ConnectOptions.ProxyOptions.ProxyPort);
        Assert.False(testChannelConfig.ConnectInfo.ConnectOptions.TcpOpts.TcpNoDelay);
        Assert.Equal(EncryptionProtocolFlags.ENC_TLSV1_2,
            testChannelConfig.ConnectInfo.ConnectOptions.EncryptionOpts.EncryptionProtocolFlags &
            EncryptionProtocolFlags.ENC_TLSV1_2);
        Assert.True(testChannelConfig.CompressionThresholdSet);
        Assert.Equal(1666, testChannelConfig.CompressionThreshold);

        // ProgChannel_2 is the default
        testChannelConfig = niProvConfigImpl.ClientChannelConfigMap["ProgChannel_2"];

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

        testLoggerConfig = niProvConfigImpl.LoggerConfigMap["ProgLogger_1"];

        Assert.Equal("ProgLogger_1", testLoggerConfig.Name);
        Assert.Equal(LoggerType.FILE, testLoggerConfig.LoggerType);
        Assert.Equal(LoggerLevel.WARNING, testLoggerConfig.LoggerSeverity);
        Assert.Equal("NewProgLogFile", testLoggerConfig.FileName);
        Assert.Equal((ulong)0, testLoggerConfig.IncludeDateInLoggerOutput);
        Assert.Equal((ulong)120, testLoggerConfig.NumberOfLogFiles);
        Assert.Equal((ulong)1100, testLoggerConfig.MaxLogFileSize);

        // ProgLogger_2 is all defaults
        testLoggerConfig = niProvConfigImpl.LoggerConfigMap["ProgLogger_2"];

        Assert.Equal("ProgLogger_2", testLoggerConfig.Name);
        Assert.Equal(defaultLoggerConfig.LoggerType, testLoggerConfig.LoggerType);
        Assert.Equal(defaultLoggerConfig.LoggerSeverity, testLoggerConfig.LoggerSeverity);
        Assert.Equal(defaultLoggerConfig.FileName, testLoggerConfig.FileName);
        Assert.Equal(defaultLoggerConfig.IncludeDateInLoggerOutput, testLoggerConfig.IncludeDateInLoggerOutput);
        Assert.Equal(defaultLoggerConfig.NumberOfLogFiles, testLoggerConfig.NumberOfLogFiles);
        Assert.Equal(defaultLoggerConfig.MaxLogFileSize, testLoggerConfig.MaxLogFileSize);

        testDictConfig = niProvConfigImpl.DictionaryConfigMap["ProgDictionary_1"];

        Assert.Equal("ProgDictionary_1", testDictConfig.Name);
        Assert.Equal(EmaConfig.DictionaryTypeEnum.CHANNEL, testDictConfig.DictionaryType);
        Assert.Equal("NewProgEnumFile", testDictConfig.EnumTypeDefFileName);
        Assert.Equal("NewProgEnumItem", testDictConfig.EnumTypeDefItemName);
        Assert.Equal("NewProgFieldFile", testDictConfig.RdmFieldDictionaryFileName);
        Assert.Equal("NewProgFieldItem", testDictConfig.RdmFieldDictionaryItemName);

        // ProgDictionary_1 is set to defaults
        testDictConfig = niProvConfigImpl.DictionaryConfigMap["ProgDictionary_2"];

        Assert.Equal("ProgDictionary_2", testDictConfig.Name);
        Assert.Equal(defaultDictConfig.DictionaryType, testDictConfig.DictionaryType);
        Assert.Equal(defaultDictConfig.EnumTypeDefFileName, testDictConfig.EnumTypeDefFileName);
        Assert.Equal(defaultDictConfig.EnumTypeDefItemName, testDictConfig.EnumTypeDefItemName);
        Assert.Equal(defaultDictConfig.RdmFieldDictionaryFileName, testDictConfig.RdmFieldDictionaryFileName);
        Assert.Equal(defaultDictConfig.RdmFieldDictionaryItemName, testDictConfig.RdmFieldDictionaryItemName);

        // Verify the Directory configuration here
        // TestDictionary_2 is set to defaults
        testDirectoryConfig = niProvConfigImpl.DirectoryConfigMap["ProgDirectory_1"];

        Assert.Equal("ProgDirectory_1", testDirectoryConfig.Name);
        Assert.Equal(2, testDirectoryConfig.ServiceMap.Count);

        testServiceConfig = testDirectoryConfig.ServiceMap["ProgService_1"];

        // Check the Info filter
        Assert.True(testServiceConfig.Service.HasInfo);
        Assert.Equal("ProgService_1", testServiceConfig.Service.Info.ServiceName.ToString());
        Assert.Equal(111, testServiceConfig.Service.ServiceId);
        Assert.True(testServiceConfig.Service.Info.HasVendor);
        Assert.Equal("newProgVendor", testServiceConfig.Service.Info.Vendor.ToString());
        Assert.True(testServiceConfig.Service.Info.HasIsSource);
        Assert.Equal(0, testServiceConfig.Service.Info.IsSource);
        Assert.Equal(2, testServiceConfig.Service.Info.CapabilitiesList.Count);
        Assert.Equal(131, testServiceConfig.Service.Info.CapabilitiesList[0]);
        Assert.Equal(6, testServiceConfig.Service.Info.CapabilitiesList[1]);
        Assert.True(testServiceConfig.Service.Info.HasAcceptingConsStatus);
        Assert.Equal(0, testServiceConfig.Service.Info.AcceptConsumerStatus);
        Assert.True(testServiceConfig.Service.Info.HasItemList);
        Assert.Equal("newProgItem", testServiceConfig.Service.Info.ItemList.ToString());
        // The dictionaries provided and used in the service will be empty at this point.
        Assert.False(testServiceConfig.Service.Info.HasDictionariesProvided);
        Assert.Empty(testServiceConfig.Service.Info.DictionariesProvidedList);
        Assert.Single(testServiceConfig.DictionariesProvidedList);
        Assert.Equal("ProgDictionary_1", testServiceConfig.DictionariesProvidedList[0]);
        Assert.False(testServiceConfig.Service.Info.HasDictionariesUsed);
        Assert.Empty(testServiceConfig.Service.Info.DictionariesUsedList);
        Assert.Single(testServiceConfig.DictionariesUsedList);
        Assert.Equal("ProgDictionary_1", testServiceConfig.DictionariesUsedList[0]);
        Assert.True(testServiceConfig.Service.Info.HasQos);
        Assert.Equal(2, testServiceConfig.Service.Info.QosList.Count);
        Assert.Equal(QosTimeliness.REALTIME, testServiceConfig.Service.Info.QosList[0].Timeliness());
        Assert.Equal(0, testServiceConfig.Service.Info.QosList[0].TimeInfo());
        Assert.Equal(QosRates.TICK_BY_TICK, testServiceConfig.Service.Info.QosList[0].Rate());
        Assert.Equal(0, testServiceConfig.Service.Info.QosList[0].RateInfo());
        Assert.Equal(QosTimeliness.DELAYED, testServiceConfig.Service.Info.QosList[1].Timeliness());
        Assert.Equal(1100, testServiceConfig.Service.Info.QosList[1].TimeInfo());
        Assert.Equal(QosRates.TIME_CONFLATED, testServiceConfig.Service.Info.QosList[1].Rate());
        Assert.Equal(1200, testServiceConfig.Service.Info.QosList[1].RateInfo());
        Assert.True(testServiceConfig.Service.Info.HasSupportQosRange);
        Assert.Equal(0, testServiceConfig.Service.Info.SupportsQosRange);
        Assert.True(testServiceConfig.Service.Info.HasSupportOOBSnapshots);
        Assert.Equal(0, testServiceConfig.Service.Info.SupportsOOBSnapshots);
        // Check the state filter
        Assert.True(testServiceConfig.Service.HasState);
        Assert.Equal(1, testServiceConfig.Service.State.ServiceStateVal);
        Assert.True(testServiceConfig.Service.State.HasAcceptingRequests);
        Assert.Equal(0, testServiceConfig.Service.State.AcceptingRequests);
        Assert.True(testServiceConfig.Service.State.HasStatus);
        Assert.Equal(StreamStates.CLOSED, testServiceConfig.Service.State.Status.StreamState());
        Assert.Equal(DataStates.OK, testServiceConfig.Service.State.Status.DataState());
        Assert.Equal(OmmState.StatusCodes.ALREADY_OPEN, testServiceConfig.Service.State.Status.Code());
        Assert.Equal("NewProgStatusText", testServiceConfig.Service.State.Status.Text().ToString());
        // Check the load filter
        Assert.True(testServiceConfig.Service.HasLoad);
        Assert.True(testServiceConfig.Service.Load.HasOpenLimit);
        Assert.Equal(1500, testServiceConfig.Service.Load.OpenLimit);
        Assert.True(testServiceConfig.Service.Load.HasOpenWindow);
        Assert.Equal(1600, testServiceConfig.Service.Load.OpenWindow);
        Assert.True(testServiceConfig.Service.Load.HasLoadFactor);
        Assert.Equal(1700, testServiceConfig.Service.Load.LoadFactor);

        testServiceConfig = testDirectoryConfig.ServiceMap["ProgService_2"];

        // This is EMA defaults.
        // Check the Info filter
        Assert.True(testServiceConfig.Service.HasInfo);
        Assert.Equal("ProgService_2", testServiceConfig.Service.Info.ServiceName.ToString());
        Assert.Equal(112, testServiceConfig.Service.ServiceId);
        Assert.False(testServiceConfig.Service.Info.HasVendor);
        Assert.True(testServiceConfig.Service.Info.HasIsSource);
        Assert.Equal(0, testServiceConfig.Service.Info.IsSource);
        Assert.Single(testServiceConfig.Service.Info.CapabilitiesList);
        Assert.Equal(EmaRdm.MMT_STORY, testServiceConfig.Service.Info.CapabilitiesList[0]);
        Assert.True(testServiceConfig.Service.Info.HasAcceptingConsStatus);
        Assert.Equal(1, testServiceConfig.Service.Info.AcceptConsumerStatus);
        Assert.False(testServiceConfig.Service.Info.HasItemList);
        // The dictionaries provided and used in the service will be empty at this point.
        Assert.False(testServiceConfig.Service.Info.HasDictionariesProvided);
        Assert.Empty(testServiceConfig.Service.Info.DictionariesProvidedList);
        Assert.Empty(testServiceConfig.DictionariesProvidedList);
        Assert.False(testServiceConfig.Service.Info.HasDictionariesUsed);
        Assert.Empty(testServiceConfig.Service.Info.DictionariesUsedList);
        Assert.Empty(testServiceConfig.DictionariesUsedList);
        Assert.True(testServiceConfig.Service.Info.HasQos);
        Assert.Empty(testServiceConfig.Service.Info.QosList);
        Assert.True(testServiceConfig.Service.Info.HasSupportOOBSnapshots);
        Assert.Equal(0, testServiceConfig.Service.Info.SupportsOOBSnapshots);
        // Check the state filter
        Assert.True(testServiceConfig.Service.HasState);
        Assert.Equal(1, testServiceConfig.Service.State.ServiceStateVal);
        Assert.True(testServiceConfig.Service.State.HasAcceptingRequests);
        Assert.Equal(0, testServiceConfig.Service.State.AcceptingRequests);
        Assert.True(testServiceConfig.Service.State.HasStatus);
        Assert.Equal(StreamStates.OPEN, testServiceConfig.Service.State.Status.StreamState());
        Assert.Equal(DataStates.OK, testServiceConfig.Service.State.Status.DataState());
        Assert.Equal(OmmState.StatusCodes.NONE, testServiceConfig.Service.State.Status.Code());
        Assert.Equal(0, testServiceConfig.Service.State.Status.Text().Length);
        // Check the load filter
        Assert.True(testServiceConfig.Service.HasLoad);
        Assert.False(testServiceConfig.Service.Load.HasOpenLimit);
        Assert.False(testServiceConfig.Service.Load.HasOpenWindow);
        Assert.False(testServiceConfig.Service.Load.HasLoadFactor);

        niProvConfigImpl.VerifyConfiguration();
    }

    // Tests the addadminmsg functionality.
    [Fact]
    public void NiProviderAddAdminMsgTest()
    {
        OmmNiProviderConfig niProviderConfig;
        OmmNiProviderConfigImpl copiedConfig;
        NIProviderRole testRole;

        try
        {
            niProviderConfig = new OmmNiProviderConfig("../../../OmmConfigTests/EmaTestConfig.xml");
        }
        catch (Exception excp)
        {
           Assert.Fail(excp.Message);
            return;
        }

        OmmNiProviderConfigImpl consConfigImpl = niProviderConfig.OmmNiProvConfigImpl;

        // Generate a consumer role to ensuure that everything that should be null is null
        // Copy the config
        copiedConfig = new OmmNiProviderConfigImpl(niProviderConfig.OmmNiProvConfigImpl);

        testRole = copiedConfig.GenerateNiProviderRole();

        // Both should be not null because default config is API driven for directories.
        Assert.NotNull(testRole.RdmLoginRequest);
        Assert.NotNull(testRole.RdmDirectoryRefresh);


        RequestMsg reqMsg = new RequestMsg();
        reqMsg.Name("TestName").DomainType((int)DomainType.LOGIN).Attrib(
            new ElementList().AddAscii("ApplicationId", "100")
                .AddAscii("Position", "TestPosition")
                .AddAscii("Password", "TestPassword")
                .AddAscii("ApplicationName", "TestEMAApp")
                .MarkForClear().Complete()
        ).MarkForClear().EncodeComplete();

        try
        {
            niProviderConfig.AddAdminMsg(reqMsg);
        }
        catch (Exception excp)
        {
           Assert.Fail(excp.Message);
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

        niProviderConfig.AddAdminMsg(reqMsg);

        // Copy the config
        copiedConfig = new OmmNiProviderConfigImpl(niProviderConfig.OmmNiProvConfigImpl);

        testRole = copiedConfig.GenerateNiProviderRole();

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

        // Now set all of the method-based options to something different and verify that they get set in the login.
        niProviderConfig.UserName("MethodUser");
        niProviderConfig.ApplicationId("MethodId");
        niProviderConfig.Position("MethodPosition");
        niProviderConfig.Password("MethodPassword");

        // Copy the config and generate the role.
        copiedConfig = new OmmNiProviderConfigImpl(niProviderConfig.OmmNiProvConfigImpl);

        testRole = copiedConfig.GenerateNiProviderRole();

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


        // Setup the absolute bare minimum, avoiding setting values that are pre-populated by EMA
        DirectoryRefresh directoryRefresh = new DirectoryRefresh();
        Service directoryService = new Service();

        directoryService.Action = MapEntryActions.ADD;
        directoryService.ServiceId = 5;
        directoryService.HasInfo = true;
        directoryService.Info.Action = FilterEntryActions.SET;
        directoryService.Info.ServiceName.Data("addMsgService");
        directoryService.Info.CapabilitiesList.Add(20);
        directoryService.Info.CapabilitiesList.Add(50);
        directoryService.Info.HasIsSource = true;
        directoryService.Info.IsSource = 1;

        directoryRefresh.ServiceList.Add(directoryService);

        directoryRefresh.StreamId = 2;
        directoryRefresh.Filter = ServiceFilterFlags.INFO;

        directoryRefresh.State.StreamState(StreamStates.OPEN);
        directoryRefresh.State.DataState(DataStates.OK);

        // Setup the refresh message.  This is so we can directly compare the directoryRefresh we're providing with the internal directoryRefresh.
        RefreshMsg refreshMsg = new RefreshMsg();
        refreshMsg.DomainType((int)DomainType.SOURCE);

        refreshMsg.Encoder!.AcquireEncodeIterator();

        Assert.Equal(CodecReturnCode.SUCCESS, directoryRefresh.Encode(refreshMsg.Encoder!.m_encodeIterator!));

        refreshMsg.Encoder!.m_containerComplete = true;
        refreshMsg.MarkForClear();
        niProviderConfig.AddAdminMsg(refreshMsg);

        // Verify the configuration
        niProviderConfig.OmmNiProvConfigImpl.VerifyConfiguration();

        // Copy the config
        copiedConfig = new OmmNiProviderConfigImpl(niProviderConfig.OmmNiProvConfigImpl);

        testRole = copiedConfig.GenerateNiProviderRole();

        // RDM Login should be set, and the directory refresh should also be set with the above settings
        Assert.NotNull(testRole.RdmLoginRequest);
        Assert.NotNull(testRole.RdmDirectoryRefresh);

        Assert.Equal(2, testRole.RdmDirectoryRefresh!.StreamId);
        Assert.Equal(ServiceFilterFlags.INFO, testRole.RdmDirectoryRefresh!.Filter);

        Assert.Single(testRole.RdmDirectoryRefresh!.ServiceList);
        Assert.Equal(MapEntryActions.ADD, testRole.RdmDirectoryRefresh!.ServiceList[0].Action);
        Assert.Equal(5, testRole.RdmDirectoryRefresh!.ServiceList[0].ServiceId);
        Assert.True(testRole.RdmDirectoryRefresh!.ServiceList[0].HasInfo);
        Assert.Equal(FilterEntryActions.SET, testRole.RdmDirectoryRefresh!.ServiceList[0].Info.Action);
        Assert.Equal(2, testRole.RdmDirectoryRefresh!.ServiceList[0].Info.CapabilitiesList.Count);
        Assert.Equal(20, testRole.RdmDirectoryRefresh!.ServiceList[0].Info.CapabilitiesList[0]);
        Assert.Equal(50, testRole.RdmDirectoryRefresh!.ServiceList[0].Info.CapabilitiesList[1]);
        Assert.True(testRole.RdmDirectoryRefresh!.ServiceList[0].Info.HasIsSource);
        Assert.Equal(1, testRole.RdmDirectoryRefresh!.ServiceList[0].Info.IsSource);

        // Finally, set the admin control to USER_CONTROL, and make sure that there isn't a defined directory refresh
        niProviderConfig.AdminControlDirectory(OmmNiProviderConfig.AdminControlMode.USER_CONTROL);

        // Copy the config and generate the role.
        copiedConfig = new OmmNiProviderConfigImpl(niProviderConfig.OmmNiProvConfigImpl);

        testRole = copiedConfig.GenerateNiProviderRole();

        // RDM Login should be set, but directory refresh should be null, and there should be nothing in the directory cache.
        Assert.NotNull(testRole.RdmLoginRequest);
        Assert.Null(testRole.RdmDirectoryRefresh);

        Assert.Empty(copiedConfig.DirectoryCache!.DirectoryRefresh.ServiceList);
    }

    // Xml Config loading and parsing test
    // This loads a config that contains all elements used in the OmmNiProvider config, tests the external setter and getter methods,
    // and verifies that the Reactor ConnectInfo and Reactor Role generation methods work, ensuring that all config members are correctly set
    // in the Role and ConnectInfo objects.
    [Fact]
    public void IProviderXmlConfigTest()
    {
        OmmIProviderConfig iProviderConfig;
        IProviderConfig testIProvConfig;
        ServerConfig testServerConfig;
        LoggerConfig testLoggerConfig;
        DictionaryConfig testDictConfig;
        DirectoryConfig testDirectoryConfig;
        EmaServiceConfig testServiceConfig;
        Service? testService;
        OmmIProviderConfigImpl copiedConfig;

        iProviderConfig = new OmmIProviderConfig("../../../OmmConfigTests/EmaTestConfig.xml");

        // Set adminControlDirectory to true
        iProviderConfig.AdminControlDirectory(OmmIProviderConfig.AdminControlMode.API_CONTROL);

        OmmIProviderConfigImpl iProvConfigImpl = iProviderConfig.OmmIProvConfigImpl;

        // Check to make sure that there aren't any elements in the ConfigErrorLog
        Assert.Equal(0, iProvConfigImpl.ConfigErrorLog!.Count());

        // Loaded the Config, now make sure everything's in it.

        // There should be 2 Consumers, 2 ClientChannels, 1 Logger, and 1 Dictionary
        Assert.Equal(2, iProvConfigImpl.IProviderConfigMap.Count);
        Assert.Equal(2, iProvConfigImpl.ServerConfigMap.Count);
        Assert.Equal(2, iProvConfigImpl.LoggerConfigMap.Count);
        Assert.Equal(2, iProvConfigImpl.DictionaryConfigMap.Count);
        Assert.Single(iProvConfigImpl.DirectoryConfigMap);

        Assert.Equal("TestIProv_1", iProvConfigImpl.FirstConfiguredIProvider);

        testIProvConfig = iProvConfigImpl.IProviderConfigMap["TestIProv_1"];

        Assert.Equal("TestIProv_1", testIProvConfig.Name);
        Assert.Equal("TestServer_1", testIProvConfig.Server);
        Assert.Equal("TestLogger_1", testIProvConfig.Logger);
        Assert.Equal("TestDirectory_1", testIProvConfig.Directory);
        Assert.Equal((long)20, testIProvConfig.DispatchTimeoutApiThread);
        Assert.Equal((ulong)30, testIProvConfig.ItemCountHint);
        Assert.Equal((int)40, testIProvConfig.MaxDispatchCountApiThread);
        Assert.Equal((int)50, testIProvConfig.MaxDispatchCountUserThread);
        Assert.False(testIProvConfig.RefreshFirstRequired);
        Assert.Equal((ulong)70, testIProvConfig.RequestTimeout);
        Assert.Equal((int)80, testIProvConfig.ServiceCountHint);
        Assert.True(testIProvConfig.XmlTraceToStdout);
        Assert.True(testIProvConfig.AcceptDirMessageWithoutMinFilters);
        Assert.True(testIProvConfig.AcceptMessageSameKeyButDiffStream);
        Assert.True(testIProvConfig.AcceptMessageThatChangesService);
        Assert.True(testIProvConfig.AcceptMessageWithoutAcceptingRequests);
        Assert.True(testIProvConfig.AcceptMessageWithoutBeingLogin);
        Assert.True(testIProvConfig.AcceptMessageWithoutQosInRange);
        Assert.True(testIProvConfig.EnforceAckIDValidation);
        Assert.Equal((int)90, testIProvConfig.EnumTypeFragmentSize);
        Assert.Equal((int)100, testIProvConfig.FieldDictionaryFragmentSize);

        // TestConsumer_2 has all defaults except for ChannelSet.
        testIProvConfig = iProvConfigImpl.IProviderConfigMap["TestIProv_2"];

        Assert.Equal("TestIProv_2", testIProvConfig.Name);
        Assert.Equal("TestServer_2", testIProvConfig.Server);
        Assert.Equal(defaultIProviderConfig.Logger, testIProvConfig.Logger);
        Assert.Equal(defaultIProviderConfig.Directory, testIProvConfig.Directory);
        Assert.Equal(defaultIProviderConfig.DispatchTimeoutApiThread, testIProvConfig.DispatchTimeoutApiThread);
        Assert.Equal(defaultIProviderConfig.ItemCountHint, testIProvConfig.ItemCountHint);
        Assert.Equal(defaultIProviderConfig.MaxDispatchCountApiThread, testIProvConfig.MaxDispatchCountApiThread);
        Assert.Equal(defaultIProviderConfig.MaxDispatchCountUserThread, testIProvConfig.MaxDispatchCountUserThread);
        Assert.Equal(defaultIProviderConfig.RefreshFirstRequired, testIProvConfig.RefreshFirstRequired);
        Assert.Equal(defaultIProviderConfig.RequestTimeout, testIProvConfig.RequestTimeout);
        Assert.Equal(defaultIProviderConfig.ServiceCountHint, testIProvConfig.ServiceCountHint);
        // XML tracing block
        Assert.Equal(defaultIProviderConfig.XmlTraceToStdout, testIProvConfig.XmlTraceToStdout);
        Assert.Equal(defaultIProviderConfig.XmlTraceToFile, testIProvConfig.XmlTraceToFile);
        Assert.Equal(defaultIProviderConfig.XmlTraceMaxFileSize, testIProvConfig.XmlTraceMaxFileSize);
        Assert.Equal(defaultIProviderConfig.XmlTraceFileName, testIProvConfig.XmlTraceFileName);
        Assert.Equal(defaultIProviderConfig.XmlTraceToMultipleFiles, testIProvConfig.XmlTraceToMultipleFiles);
        Assert.Equal(defaultIProviderConfig.XmlTraceWrite, testIProvConfig.XmlTraceWrite);
        Assert.Equal(defaultIProviderConfig.XmlTraceRead, testIProvConfig.XmlTraceRead);
        Assert.Equal(defaultIProviderConfig.XmlTracePing, testIProvConfig.XmlTracePing);
        Assert.Equal(defaultIProviderConfig.AcceptDirMessageWithoutMinFilters, testIProvConfig.AcceptDirMessageWithoutMinFilters);
        Assert.Equal(defaultIProviderConfig.AcceptMessageSameKeyButDiffStream, testIProvConfig.AcceptMessageSameKeyButDiffStream);
        Assert.Equal(defaultIProviderConfig.AcceptMessageThatChangesService, testIProvConfig.AcceptMessageThatChangesService);
        Assert.Equal(defaultIProviderConfig.AcceptMessageWithoutAcceptingRequests, testIProvConfig.AcceptMessageWithoutAcceptingRequests);
        Assert.Equal(defaultIProviderConfig.AcceptMessageWithoutBeingLogin, testIProvConfig.AcceptMessageWithoutBeingLogin);
        Assert.Equal(defaultIProviderConfig.AcceptMessageWithoutQosInRange, testIProvConfig.AcceptMessageWithoutQosInRange);
        Assert.Equal(defaultIProviderConfig.EnforceAckIDValidation, testIProvConfig.EnforceAckIDValidation);
        Assert.Equal(defaultIProviderConfig.EnumTypeFragmentSize, testIProvConfig.EnumTypeFragmentSize);
        Assert.Equal(defaultIProviderConfig.FieldDictionaryFragmentSize, testIProvConfig.FieldDictionaryFragmentSize);

        testServerConfig = iProvConfigImpl.ServerConfigMap["TestServer_1"];

        Assert.Equal("TestServer_1", testServerConfig.Name);
        Assert.Equal(10, testServerConfig.BindOptions.MinPingTimeout);
        Assert.Equal(20, testServerConfig.BindOptions.PingTimeout);
        Assert.True(testServerConfig.CompressionThresholdSet);
        Assert.Equal(30, testServerConfig.CompressionThreshold);
        Assert.Equal(Eta.Transports.CompressionType.ZLIB, testServerConfig.BindOptions.CompressionType);
        Assert.True(testServerConfig.DirectWrite);
        Assert.Equal(40, testServerConfig.BindOptions.GuaranteedOutputBuffers);
        Assert.Equal(50, testServerConfig.HighWaterMark);
        Assert.Equal(60, testServerConfig.InitializationTimeout);
        Assert.Equal("testInterface", testServerConfig.BindOptions.InterfaceName);
        Assert.Equal(70, testServerConfig.BindOptions.MaxFragmentSize);
        Assert.Equal(80, testServerConfig.BindOptions.NumInputBuffers);
        Assert.Equal("testPort1", testServerConfig.BindOptions.ServiceName);
        Assert.Equal(Eta.Transports.ConnectionType.ENCRYPTED, testServerConfig.BindOptions.ConnectionType);
        Assert.Equal(90, testServerConfig.BindOptions.SysRecvBufSize);
        Assert.Equal(100, testServerConfig.BindOptions.SysSendBufSize);
        Assert.False(testServerConfig.BindOptions.TcpOpts.TcpNoDelay);
        Assert.Equal("testCert", testServerConfig.BindOptions.BindEncryptionOpts.ServerCertificate);
        Assert.Equal("testKey", testServerConfig.BindOptions.BindEncryptionOpts.ServerPrivateKey);
        Assert.Equal(4, testServerConfig.BindOptions.BindEncryptionOpts.TlsCipherSuites.Count());
        Assert.Contains(TlsCipherSuite.TLS_RSA_WITH_NULL_MD5, testServerConfig.BindOptions.BindEncryptionOpts.TlsCipherSuites);
        Assert.Contains(TlsCipherSuite.TLS_RSA_WITH_NULL_SHA, testServerConfig.BindOptions.BindEncryptionOpts.TlsCipherSuites);
        Assert.Contains(TlsCipherSuite.TLS_RSA_EXPORT_WITH_RC4_40_MD5, testServerConfig.BindOptions.BindEncryptionOpts.TlsCipherSuites);
        Assert.Contains(TlsCipherSuite.TLS_RSA_WITH_RC4_128_SHA, testServerConfig.BindOptions.BindEncryptionOpts.TlsCipherSuites);
        Assert.Equal(EncryptionProtocolFlags.ENC_TLSV1_2, testServerConfig.BindOptions.BindEncryptionOpts.EncryptionProtocolFlags);
        Assert.Equal(15000, testServerConfig.BindOptions.BindEncryptionOpts.AuthenticationTimeout);

        // TestChannel_2 is the default
        testServerConfig = iProvConfigImpl.ServerConfigMap["TestServer_2"];

        Assert.Equal("TestServer_2", testServerConfig.Name);
        Assert.Equal(defaultServerConfig.BindOptions.MinPingTimeout, testServerConfig.BindOptions.MinPingTimeout);
        Assert.Equal(defaultServerConfig.BindOptions.PingTimeout, testServerConfig.BindOptions.PingTimeout);
        Assert.Equal(defaultServerConfig.CompressionThresholdSet, testServerConfig.CompressionThresholdSet);
        Assert.Equal(defaultServerConfig.CompressionThreshold, testServerConfig.CompressionThreshold);
        Assert.Equal(defaultServerConfig.BindOptions.CompressionType, testServerConfig.BindOptions.CompressionType);
        Assert.Equal(defaultServerConfig.DirectWrite, testServerConfig.DirectWrite);
        Assert.Equal(defaultServerConfig.BindOptions.GuaranteedOutputBuffers, testServerConfig.BindOptions.GuaranteedOutputBuffers);
        Assert.Equal(defaultServerConfig.HighWaterMark, testServerConfig.HighWaterMark);
        Assert.Equal(defaultServerConfig.InitializationTimeout, testServerConfig.InitializationTimeout);
        Assert.Equal(defaultServerConfig.BindOptions.InterfaceName, testServerConfig.BindOptions.InterfaceName);
        Assert.Equal(defaultServerConfig.BindOptions.MaxFragmentSize, testServerConfig.BindOptions.MaxFragmentSize);
        Assert.Equal(defaultServerConfig.BindOptions.NumInputBuffers, testServerConfig.BindOptions.NumInputBuffers);
        Assert.Equal(defaultServerConfig.BindOptions.ServiceName, testServerConfig.BindOptions.ServiceName);
        Assert.Equal(defaultServerConfig.BindOptions.ConnectionType, testServerConfig.BindOptions.ConnectionType);
        Assert.Equal(defaultServerConfig.BindOptions.SysRecvBufSize, testServerConfig.BindOptions.SysRecvBufSize);
        Assert.Equal(defaultServerConfig.BindOptions.SysSendBufSize, testServerConfig.BindOptions.SysSendBufSize);
        Assert.Equal(defaultServerConfig.BindOptions.TcpOpts.TcpNoDelay, testServerConfig.BindOptions.TcpOpts.TcpNoDelay);
        Assert.Equal(defaultServerConfig.BindOptions.BindEncryptionOpts.ServerCertificate, testServerConfig.BindOptions.BindEncryptionOpts.ServerCertificate);
        Assert.Equal(defaultServerConfig.BindOptions.BindEncryptionOpts.ServerPrivateKey, testServerConfig.BindOptions.BindEncryptionOpts.ServerPrivateKey);
        Assert.Null(testServerConfig.BindOptions.BindEncryptionOpts.TlsCipherSuites);
        Assert.Equal(defaultServerConfig.BindOptions.BindEncryptionOpts.EncryptionProtocolFlags, testServerConfig.BindOptions.BindEncryptionOpts.EncryptionProtocolFlags);
        Assert.Equal(10000, testServerConfig.BindOptions.BindEncryptionOpts.AuthenticationTimeout); // Default value

        testLoggerConfig = iProvConfigImpl.LoggerConfigMap["TestLogger_1"];

        Assert.Equal("TestLogger_1", testLoggerConfig.Name);
        Assert.Equal(LoggerType.STDOUT, testLoggerConfig.LoggerType);
        Assert.Equal(LoggerLevel.INFO, testLoggerConfig.LoggerSeverity);
        Assert.Equal("testLogFile1", testLoggerConfig.FileName);
        Assert.Equal((ulong)1, testLoggerConfig.IncludeDateInLoggerOutput);
        Assert.Equal((ulong)10, testLoggerConfig.NumberOfLogFiles);
        Assert.Equal((ulong)20, testLoggerConfig.MaxLogFileSize);

        testLoggerConfig = iProvConfigImpl.LoggerConfigMap["TestLogger_2"];

        Assert.Equal("TestLogger_2", testLoggerConfig.Name);
        Assert.Equal(defaultLoggerConfig.LoggerType, testLoggerConfig.LoggerType);
        Assert.Equal(defaultLoggerConfig.LoggerSeverity, testLoggerConfig.LoggerSeverity);
        Assert.Equal(defaultLoggerConfig.FileName, testLoggerConfig.FileName);
        Assert.Equal(defaultLoggerConfig.IncludeDateInLoggerOutput, testLoggerConfig.IncludeDateInLoggerOutput);
        Assert.Equal(defaultLoggerConfig.NumberOfLogFiles, testLoggerConfig.NumberOfLogFiles);
        Assert.Equal(defaultLoggerConfig.MaxLogFileSize, testLoggerConfig.MaxLogFileSize);

        testDictConfig = iProvConfigImpl.DictionaryConfigMap["TestDictionary_1"];

        Assert.Equal("TestDictionary_1", testDictConfig.Name);
        Assert.Equal(EmaConfig.DictionaryTypeEnum.FILE, testDictConfig.DictionaryType);
        Assert.Equal("testEnumFile1", testDictConfig.EnumTypeDefFileName);
        Assert.Equal("testEnumItem1", testDictConfig.EnumTypeDefItemName);
        Assert.Equal("testRdmFile1", testDictConfig.RdmFieldDictionaryFileName);
        Assert.Equal("testRdmItem1", testDictConfig.RdmFieldDictionaryItemName);

        // TestDictionary_2 is set to defaults
        testDictConfig = iProvConfigImpl.DictionaryConfigMap["TestDictionary_2"];

        Assert.Equal("TestDictionary_2", testDictConfig.Name);
        Assert.Equal(defaultDictConfig.DictionaryType, testDictConfig.DictionaryType);
        Assert.Equal(defaultDictConfig.EnumTypeDefFileName, testDictConfig.EnumTypeDefFileName);
        Assert.Equal(defaultDictConfig.EnumTypeDefItemName, testDictConfig.EnumTypeDefItemName);
        Assert.Equal(defaultDictConfig.RdmFieldDictionaryFileName, testDictConfig.RdmFieldDictionaryFileName);
        Assert.Equal(defaultDictConfig.RdmFieldDictionaryItemName, testDictConfig.RdmFieldDictionaryItemName);

        // Verify the Directory configuration here
        // TestDictionary_2 is set to defaults
        testDirectoryConfig = iProvConfigImpl.DirectoryConfigMap["TestDirectory_1"];

        Assert.Equal("TestDirectory_1", testDirectoryConfig.Name);
        Assert.Equal(2, testDirectoryConfig.ServiceMap.Count);

        testServiceConfig = testDirectoryConfig.ServiceMap["TestService_1"];

        // Check the Info filter
        Assert.True(testServiceConfig.Service.HasInfo);
        Assert.Equal("TestService_1", testServiceConfig.Service.Info.ServiceName.ToString());
        Assert.Equal(10, testServiceConfig.Service.ServiceId);
        Assert.True(testServiceConfig.Service.Info.HasVendor);
        Assert.Equal("RTSDK", testServiceConfig.Service.Info.Vendor.ToString());
        Assert.True(testServiceConfig.Service.Info.HasIsSource);
        Assert.Equal(1, testServiceConfig.Service.Info.IsSource);
        Assert.Equal(2, testServiceConfig.Service.Info.CapabilitiesList.Count);
        Assert.Equal(EmaRdm.MMT_MARKET_PRICE, testServiceConfig.Service.Info.CapabilitiesList[0]);
        Assert.Equal(129, testServiceConfig.Service.Info.CapabilitiesList[1]);
        Assert.True(testServiceConfig.Service.Info.HasAcceptingConsStatus);
        Assert.Equal(0, testServiceConfig.Service.Info.AcceptConsumerStatus);
        Assert.True(testServiceConfig.Service.Info.HasItemList);
        Assert.Equal("Items#", testServiceConfig.Service.Info.ItemList.ToString());
        // The dictionaries provided and used in the service will be empty at this point.
        Assert.False(testServiceConfig.Service.Info.HasDictionariesProvided);
        Assert.Empty(testServiceConfig.Service.Info.DictionariesProvidedList);
        Assert.Equal(2, testServiceConfig.DictionariesProvidedList.Count);
        Assert.Equal("TestDictionary_1", testServiceConfig.DictionariesProvidedList[0]);
        Assert.Equal("TestDictionary_2", testServiceConfig.DictionariesProvidedList[1]);
        Assert.False(testServiceConfig.Service.Info.HasDictionariesUsed);
        Assert.Empty(testServiceConfig.Service.Info.DictionariesUsedList);
        Assert.Equal(2, testServiceConfig.DictionariesUsedList.Count);
        Assert.Equal("TestDictionary_1", testServiceConfig.DictionariesUsedList[0]);
        Assert.Equal("TestDictionary_2", testServiceConfig.DictionariesUsedList[1]);
        Assert.True(testServiceConfig.Service.Info.HasQos);
        Assert.Equal(2, testServiceConfig.Service.Info.QosList.Count);
        Assert.Equal(QosTimeliness.REALTIME, testServiceConfig.Service.Info.QosList[0].Timeliness());
        Assert.Equal(0, testServiceConfig.Service.Info.QosList[0].TimeInfo());
        Assert.Equal(QosRates.TICK_BY_TICK, testServiceConfig.Service.Info.QosList[0].Rate());
        Assert.Equal(0, testServiceConfig.Service.Info.QosList[0].RateInfo());
        Assert.Equal(QosTimeliness.DELAYED, testServiceConfig.Service.Info.QosList[1].Timeliness());
        Assert.Equal(10, testServiceConfig.Service.Info.QosList[1].TimeInfo());
        Assert.Equal(QosRates.TIME_CONFLATED, testServiceConfig.Service.Info.QosList[1].Rate());
        Assert.Equal(20, testServiceConfig.Service.Info.QosList[1].RateInfo());
        Assert.True(testServiceConfig.Service.Info.HasSupportQosRange);
        Assert.Equal(1, testServiceConfig.Service.Info.SupportsQosRange);
        Assert.True(testServiceConfig.Service.Info.HasSupportOOBSnapshots);
        Assert.Equal(1, testServiceConfig.Service.Info.SupportsOOBSnapshots);
        // Check the state filter
        Assert.True(testServiceConfig.Service.HasState);
        Assert.Equal(0, testServiceConfig.Service.State.ServiceStateVal);
        Assert.True(testServiceConfig.Service.State.HasAcceptingRequests);
        Assert.Equal(1, testServiceConfig.Service.State.AcceptingRequests);
        Assert.True(testServiceConfig.Service.State.HasStatus);
        Assert.Equal(StreamStates.CLOSED, testServiceConfig.Service.State.Status.StreamState());
        Assert.Equal(DataStates.SUSPECT, testServiceConfig.Service.State.Status.DataState());
        Assert.Equal(OmmState.StatusCodes.NOT_AUTHORIZED, testServiceConfig.Service.State.Status.Code());
        Assert.Equal("TestText", testServiceConfig.Service.State.Status.Text().ToString());
        // Check the load filter
        Assert.True(testServiceConfig.Service.HasLoad);
        Assert.True(testServiceConfig.Service.Load.HasOpenLimit);
        Assert.Equal(100, testServiceConfig.Service.Load.OpenLimit);
        Assert.True(testServiceConfig.Service.Load.HasOpenWindow);
        Assert.Equal(110, testServiceConfig.Service.Load.OpenWindow);
        Assert.True(testServiceConfig.Service.Load.HasLoadFactor);
        Assert.Equal(120, testServiceConfig.Service.Load.LoadFactor);

        testServiceConfig = testDirectoryConfig.ServiceMap["TestService_2"];

        // This is EMA defaults.
        // Check the Info filter
        Assert.True(testServiceConfig.Service.HasInfo);
        Assert.Equal("TestService_2", testServiceConfig.Service.Info.ServiceName.ToString());
        Assert.Equal(15, testServiceConfig.Service.ServiceId);
        Assert.False(testServiceConfig.Service.Info.HasVendor);
        Assert.True(testServiceConfig.Service.Info.HasIsSource);
        Assert.Equal(0, testServiceConfig.Service.Info.IsSource);
        Assert.Single(testServiceConfig.Service.Info.CapabilitiesList);
        Assert.Equal(EmaRdm.MMT_MARKET_PRICE, testServiceConfig.Service.Info.CapabilitiesList[0]);
        Assert.True(testServiceConfig.Service.Info.HasAcceptingConsStatus);
        Assert.Equal(1, testServiceConfig.Service.Info.AcceptConsumerStatus);
        Assert.False(testServiceConfig.Service.Info.HasItemList);
        // The dictionaries provided and used in the service will be empty at this point.
        Assert.False(testServiceConfig.Service.Info.HasDictionariesProvided);
        Assert.Empty(testServiceConfig.Service.Info.DictionariesProvidedList);
        Assert.Single(testServiceConfig.DictionariesProvidedList);
        Assert.Equal("TestDictionary_1", testServiceConfig.DictionariesProvidedList[0]);
        Assert.False(testServiceConfig.Service.Info.HasDictionariesUsed);
        Assert.Empty(testServiceConfig.Service.Info.DictionariesUsedList);
        Assert.Single(testServiceConfig.DictionariesUsedList);
        Assert.Equal("TestDictionary_1", testServiceConfig.DictionariesUsedList[0]);
        Assert.True(testServiceConfig.Service.Info.HasQos);
        Assert.Empty(testServiceConfig.Service.Info.QosList);
        Assert.True(testServiceConfig.Service.Info.HasSupportOOBSnapshots);
        Assert.Equal(0, testServiceConfig.Service.Info.SupportsOOBSnapshots);
        // Check the state filter
        Assert.True(testServiceConfig.Service.HasState);
        Assert.Equal(1, testServiceConfig.Service.State.ServiceStateVal);
        Assert.True(testServiceConfig.Service.State.HasAcceptingRequests);
        Assert.Equal(1, testServiceConfig.Service.State.AcceptingRequests);
        Assert.True(testServiceConfig.Service.State.HasStatus);
        Assert.Equal(StreamStates.OPEN, testServiceConfig.Service.State.Status.StreamState());
        Assert.Equal(DataStates.OK, testServiceConfig.Service.State.Status.DataState());
        Assert.Equal(OmmState.StatusCodes.NONE, testServiceConfig.Service.State.Status.Code());
        Assert.Equal(0, testServiceConfig.Service.State.Status.Text().Length);
        // Check the load filter
        Assert.True(testServiceConfig.Service.HasLoad);
        Assert.False(testServiceConfig.Service.Load.HasOpenLimit);
        Assert.False(testServiceConfig.Service.Load.HasOpenWindow);
        Assert.False(testServiceConfig.Service.Load.HasLoadFactor);

        // Verify that the config works here
        iProvConfigImpl.VerifyConfiguration();

        copiedConfig = new OmmIProviderConfigImpl(iProvConfigImpl);

        // This config should have one niProvider, logger and dictionary, and 1 server.  The configured services in the choosen directory will be placed into the DirectoryCache.
        Assert.Single(copiedConfig.IProviderConfigMap);
        Assert.Single(copiedConfig.ServerConfigMap);
        Assert.Single(copiedConfig.LoggerConfigMap);
        Assert.Single(copiedConfig.DirectoryConfigMap);
        Assert.Equal(2, copiedConfig.DictionaryConfigMap.Count);
        Assert.NotNull(copiedConfig.DirectoryCache);
        Assert.Equal(2, copiedConfig.DirectoryCache!.DirectoryRefresh.ServiceList.Count);

        testIProvConfig = copiedConfig.IProviderConfigMap["TestIProv_1"];

        Assert.Equal("TestIProv_1", testIProvConfig.Name);
        Assert.Equal("TestServer_1", testIProvConfig.Server);
        Assert.Equal("TestLogger_1", testIProvConfig.Logger);
        Assert.Equal("TestDirectory_1", testIProvConfig.Directory);
        Assert.Equal((long)20, testIProvConfig.DispatchTimeoutApiThread);
        Assert.Equal((ulong)30, testIProvConfig.ItemCountHint);
        Assert.Equal((int)40, testIProvConfig.MaxDispatchCountApiThread);
        Assert.Equal((int)50, testIProvConfig.MaxDispatchCountUserThread);
        Assert.False(testIProvConfig.RefreshFirstRequired);
        Assert.Equal((ulong)70, testIProvConfig.RequestTimeout);
        Assert.Equal((int)80, testIProvConfig.ServiceCountHint);
        // XML tracing block
        Assert.True(testIProvConfig.XmlTraceToStdout);
        Assert.False(testIProvConfig.XmlTraceToFile);
        Assert.Equal((ulong)100_000_000, testIProvConfig.XmlTraceMaxFileSize);
        Assert.Equal("EmaTrace", testIProvConfig.XmlTraceFileName);
        Assert.False(testIProvConfig.XmlTraceToMultipleFiles);
        Assert.True(testIProvConfig.XmlTraceWrite);
        Assert.True(testIProvConfig.XmlTraceRead);
        Assert.False(testIProvConfig.XmlTracePing);
        Assert.True(testIProvConfig.AcceptDirMessageWithoutMinFilters);
        Assert.True(testIProvConfig.AcceptMessageSameKeyButDiffStream);
        Assert.True(testIProvConfig.AcceptMessageThatChangesService);
        Assert.True(testIProvConfig.AcceptMessageWithoutAcceptingRequests);
        Assert.True(testIProvConfig.AcceptMessageWithoutBeingLogin);
        Assert.True(testIProvConfig.AcceptMessageWithoutQosInRange);
        Assert.True(testIProvConfig.EnforceAckIDValidation);
        Assert.Equal((int)90, testIProvConfig.EnumTypeFragmentSize);
        Assert.Equal((int)100, testIProvConfig.FieldDictionaryFragmentSize);

        testServerConfig = copiedConfig.ServerConfigMap["TestServer_1"];

        Assert.Same(copiedConfig.ServerConfig, testServerConfig);
        Assert.Equal("TestServer_1", testServerConfig.Name);
        Assert.Equal(10, testServerConfig.BindOptions.MinPingTimeout);
        Assert.Equal(20, testServerConfig.BindOptions.PingTimeout);
        Assert.True(testServerConfig.CompressionThresholdSet);
        Assert.Equal(30, testServerConfig.CompressionThreshold);
        Assert.Equal(Eta.Transports.CompressionType.ZLIB, testServerConfig.BindOptions.CompressionType);
        Assert.True(testServerConfig.DirectWrite);
        Assert.Equal(40, testServerConfig.BindOptions.GuaranteedOutputBuffers);
        Assert.Equal(50, testServerConfig.HighWaterMark);
        Assert.Equal(60, testServerConfig.InitializationTimeout);
        Assert.Equal("testInterface", testServerConfig.BindOptions.InterfaceName);
        Assert.Equal(70, testServerConfig.BindOptions.MaxFragmentSize);
        Assert.Equal(80, testServerConfig.BindOptions.NumInputBuffers);
        Assert.Equal("testPort1", testServerConfig.BindOptions.ServiceName);
        Assert.Equal(Eta.Transports.ConnectionType.ENCRYPTED, testServerConfig.BindOptions.ConnectionType);
        Assert.Equal(90, testServerConfig.BindOptions.SysRecvBufSize);
        Assert.Equal(100, testServerConfig.BindOptions.SysSendBufSize);
        Assert.False(testServerConfig.BindOptions.TcpOpts.TcpNoDelay);
        Assert.Equal("testCert", testServerConfig.BindOptions.BindEncryptionOpts.ServerCertificate);
        Assert.Equal("testKey", testServerConfig.BindOptions.BindEncryptionOpts.ServerPrivateKey);
        Assert.Equal(4, testServerConfig.BindOptions.BindEncryptionOpts.TlsCipherSuites.Count());
        Assert.Contains(TlsCipherSuite.TLS_RSA_WITH_NULL_MD5, testServerConfig.BindOptions.BindEncryptionOpts.TlsCipherSuites);
        Assert.Contains(TlsCipherSuite.TLS_RSA_WITH_NULL_SHA, testServerConfig.BindOptions.BindEncryptionOpts.TlsCipherSuites);
        Assert.Contains(TlsCipherSuite.TLS_RSA_EXPORT_WITH_RC4_40_MD5, testServerConfig.BindOptions.BindEncryptionOpts.TlsCipherSuites);
        Assert.Contains(TlsCipherSuite.TLS_RSA_WITH_RC4_128_SHA, testServerConfig.BindOptions.BindEncryptionOpts.TlsCipherSuites);
        Assert.Equal(EncryptionProtocolFlags.ENC_TLSV1_2, testServerConfig.BindOptions.BindEncryptionOpts.EncryptionProtocolFlags);

        // Check the directory refresh services generated by the copy constructor from the config
        testService = copiedConfig.DirectoryCache!.GetService(10);

        Assert.NotNull(testService);

        // Check the Info filter
        Assert.True(testService!.HasInfo);
        Assert.Equal("TestService_1", testService!.Info.ServiceName.ToString());
        Assert.Equal(10, testService!.ServiceId);
        Assert.True(testService!.Info.HasVendor);
        Assert.Equal("RTSDK", testService!.Info.Vendor.ToString());
        Assert.True(testService!.Info.HasIsSource);
        Assert.Equal(1, testService!.Info.IsSource);
        Assert.Equal(2, testService!.Info.CapabilitiesList.Count);
        Assert.Equal(EmaRdm.MMT_MARKET_PRICE, testService!.Info.CapabilitiesList[0]);
        Assert.Equal(129, testService!.Info.CapabilitiesList[1]);
        Assert.True(testService!.Info.HasAcceptingConsStatus);
        Assert.Equal(0, testService!.Info.AcceptConsumerStatus);
        Assert.True(testService!.Info.HasItemList);
        Assert.Equal("Items#", testService!.Info.ItemList.ToString());
        // The dictionaries provided and used in the service will be empty at this point.
        Assert.True(testService!.Info.HasDictionariesProvided);
        Assert.Equal(4, testService!.Info.DictionariesProvidedList.Count);
        Assert.Contains("testRdmItem1", testService!.Info.DictionariesProvidedList);
        Assert.Contains("testEnumItem1", testService!.Info.DictionariesProvidedList);
        Assert.Contains(defaultDictConfig.EnumTypeDefItemName, testService!.Info.DictionariesProvidedList);
        Assert.Contains(defaultDictConfig.RdmFieldDictionaryItemName, testService!.Info.DictionariesProvidedList);
        Assert.True(testService!.Info.HasDictionariesUsed);
        Assert.Equal(4, testService!.Info.DictionariesUsedList.Count);
        Assert.Contains("testRdmItem1", testService!.Info.DictionariesUsedList);
        Assert.Contains("testEnumItem1", testService!.Info.DictionariesUsedList);
        Assert.Contains(defaultDictConfig.EnumTypeDefItemName, testService!.Info.DictionariesUsedList);
        Assert.Contains(defaultDictConfig.RdmFieldDictionaryItemName, testService!.Info.DictionariesUsedList);
        Assert.True(testService!.Info.HasQos);
        Assert.Equal(2, testService!.Info.QosList.Count);
        Assert.Equal(QosTimeliness.REALTIME, testService!.Info.QosList[0].Timeliness());
        Assert.Equal(0, testService!.Info.QosList[0].TimeInfo());
        Assert.Equal(QosRates.TICK_BY_TICK, testService!.Info.QosList[0].Rate());
        Assert.Equal(0, testService!.Info.QosList[0].RateInfo());
        Assert.Equal(QosTimeliness.DELAYED, testService!.Info.QosList[1].Timeliness());
        Assert.Equal(10, testService!.Info.QosList[1].TimeInfo());
        Assert.Equal(QosRates.TIME_CONFLATED, testService!.Info.QosList[1].Rate());
        Assert.Equal(20, testService!.Info.QosList[1].RateInfo());
        Assert.True(testService!.Info.HasSupportQosRange);
        Assert.Equal(1, testService!.Info.SupportsQosRange);
        Assert.True(testService!.Info.HasSupportOOBSnapshots);
        Assert.Equal(1, testService!.Info.SupportsOOBSnapshots);
        // Check the state filter
        Assert.True(testService!.HasState);
        Assert.Equal(0, testService!.State.ServiceStateVal);
        Assert.True(testService!.State.HasAcceptingRequests);
        Assert.Equal(1, testService!.State.AcceptingRequests);
        Assert.True(testService!.State.HasStatus);
        Assert.Equal(StreamStates.CLOSED, testService!.State.Status.StreamState());
        Assert.Equal(DataStates.SUSPECT, testService!.State.Status.DataState());
        Assert.Equal(OmmState.StatusCodes.NOT_AUTHORIZED, testService!.State.Status.Code());
        Assert.Equal("TestText", testService!.State.Status.Text().ToString());
        // Check the load filter
        Assert.True(testService!.HasLoad);
        Assert.True(testService!.Load.HasOpenLimit);
        Assert.Equal(100, testService!.Load.OpenLimit);
        Assert.True(testService!.Load.HasOpenWindow);
        Assert.Equal(110, testService!.Load.OpenWindow);
        Assert.True(testService!.Load.HasLoadFactor);
        Assert.Equal(120, testService!.Load.LoadFactor);

        testService = copiedConfig.DirectoryCache!.GetService(15);

        Assert.NotNull(testService);
        // This is EMA defaults.
        // Check the Info filter
        Assert.True(testService!.HasInfo);
        Assert.Equal("TestService_2", testService!.Info.ServiceName.ToString());
        Assert.Equal(15, testService!.ServiceId);
        Assert.False(testService!.Info.HasVendor);
        Assert.True(testService!.Info.HasIsSource);
        Assert.Equal(0, testService!.Info.IsSource);
        Assert.Single(testService!.Info.CapabilitiesList);
        Assert.Equal(EmaRdm.MMT_MARKET_PRICE, testService!.Info.CapabilitiesList[0]);
        Assert.True(testService!.Info.HasAcceptingConsStatus);
        Assert.Equal(1, testService!.Info.AcceptConsumerStatus);
        Assert.False(testService!.Info.HasItemList);
        // The dictionaries provided and used in the service will be empty at this point.
        Assert.True(testService!.Info.HasDictionariesUsed);
        Assert.Equal(2, testService!.Info.DictionariesUsedList.Count);
        Assert.Contains("testRdmItem1", testService!.Info.DictionariesUsedList);
        Assert.Contains("testEnumItem1", testService!.Info.DictionariesUsedList);
        Assert.True(testService!.Info.HasDictionariesProvided);
        Assert.Equal(2, testService!.Info.DictionariesProvidedList.Count);
        Assert.Contains("testRdmItem1", testService!.Info.DictionariesProvidedList);
        Assert.Contains("testEnumItem1", testService!.Info.DictionariesProvidedList);
        Assert.True(testService!.Info.HasQos);
        // Default QoS list
        Assert.NotNull(testService!.Info.QosList[0]);
        Assert.Equal(QosTimeliness.REALTIME, testService!.Info.QosList[0].Timeliness());
        Assert.Equal(0, testService!.Info.QosList[0].TimeInfo());
        Assert.Equal(QosRates.TICK_BY_TICK, testService!.Info.QosList[0].Rate());
        Assert.Equal(0, testService!.Info.QosList[0].RateInfo());
        Assert.True(testService!.Info.HasSupportOOBSnapshots);
        Assert.Equal(0, testService!.Info.SupportsOOBSnapshots);
        // Check the state filter
        Assert.True(testService!.HasState);
        Assert.Equal(1, testService!.State.ServiceStateVal);
        Assert.True(testService!.State.HasAcceptingRequests);
        Assert.Equal(1, testService!.State.AcceptingRequests);
        Assert.True(testService!.State.HasStatus);
        Assert.Equal(StreamStates.OPEN, testService!.State.Status.StreamState());
        Assert.Equal(DataStates.OK, testService!.State.Status.DataState());
        Assert.Equal(OmmState.StatusCodes.NONE, testService!.State.Status.Code());
        Assert.Equal(0, testService!.State.Status.Text().Length);
        // Check the load filter
        Assert.True(testService!.HasLoad);
        Assert.False(testService!.Load.HasOpenLimit);
        Assert.False(testService!.Load.HasOpenWindow);
        Assert.False(testService!.Load.HasLoadFactor);

        
        // OmmConsumerConfig reuse and ConsumerName override test
        // Sets the consumer Name to TestConsumer_2, which has a channelSet containing TestChannel_1 and TestChannel_2
        // Also sets dummy encryption options to make sure they flow through.
        iProviderConfig.ProviderName("TestIProv_2");
        iProviderConfig.ServerCertificate("testMethodCert_1");
        iProviderConfig.ServerPrivateKey("testMethodKey_1");
        // Dummy encryption cipher to make sure it flows through.
        List<TlsCipherSuite> tlsCiphers = new List<TlsCipherSuite>();
        tlsCiphers.Add(TlsCipherSuite.TLS_RSA_WITH_NULL_SHA);
        iProviderConfig.TlsCipherSuites(tlsCiphers);
        iProviderConfig.Port("testMethodPort");

        // Verify that the config works here
        iProvConfigImpl.VerifyConfiguration();

        copiedConfig = new OmmIProviderConfigImpl(iProvConfigImpl);

        // This config should only have one of each config type.
        Assert.Single(copiedConfig.IProviderConfigMap);
        Assert.Single(copiedConfig.ServerConfigMap);
        Assert.Single(copiedConfig.LoggerConfigMap);
        Assert.Equal(2, copiedConfig.DictionaryConfigMap.Count);
        Assert.NotNull(copiedConfig.DirectoryCache);
        Assert.Equal(2, copiedConfig.DirectoryCache!.DirectoryRefresh.ServiceList.Count);

        // Verify that the server port has changed
        Assert.Equal("testMethodPort", copiedConfig.ServerPort);


        // TestConsumer_2 has all defaults except for ChannelSet.
        testIProvConfig = copiedConfig.IProviderConfigMap["TestIProv_2"];

        Assert.Equal("TestIProv_2", testIProvConfig.Name);
        Assert.Equal("TestServer_2", testIProvConfig.Server);
        Assert.Equal("DefaultEmaLogger", testIProvConfig.Logger);
        Assert.Equal("TestDirectory_1", testIProvConfig.Directory);
        Assert.Equal(defaultIProviderConfig.DispatchTimeoutApiThread, testIProvConfig.DispatchTimeoutApiThread);
        Assert.Equal(defaultIProviderConfig.ItemCountHint, testIProvConfig.ItemCountHint);
        Assert.Equal(defaultIProviderConfig.MaxDispatchCountApiThread, testIProvConfig.MaxDispatchCountApiThread);
        Assert.Equal(defaultIProviderConfig.MaxDispatchCountUserThread, testIProvConfig.MaxDispatchCountUserThread);
        Assert.Equal(defaultIProviderConfig.RefreshFirstRequired, testIProvConfig.RefreshFirstRequired);
        Assert.Equal(defaultIProviderConfig.RequestTimeout, testIProvConfig.RequestTimeout);
        Assert.Equal(defaultIProviderConfig.ServiceCountHint, testIProvConfig.ServiceCountHint);
        // XML tracing block
        Assert.Equal(defaultIProviderConfig.XmlTraceToStdout, testIProvConfig.XmlTraceToStdout);
        Assert.Equal(defaultIProviderConfig.XmlTraceToFile, testIProvConfig.XmlTraceToFile);
        Assert.Equal(defaultIProviderConfig.XmlTraceMaxFileSize, testIProvConfig.XmlTraceMaxFileSize);
        Assert.Equal(defaultIProviderConfig.XmlTraceFileName, testIProvConfig.XmlTraceFileName);
        Assert.Equal(defaultIProviderConfig.XmlTraceToMultipleFiles, testIProvConfig.XmlTraceToMultipleFiles);
        Assert.Equal(defaultIProviderConfig.XmlTraceWrite, testIProvConfig.XmlTraceWrite);
        Assert.Equal(defaultIProviderConfig.XmlTraceRead, testIProvConfig.XmlTraceRead);
        Assert.Equal(defaultIProviderConfig.XmlTracePing, testIProvConfig.XmlTracePing);
        Assert.Equal(defaultIProviderConfig.AcceptDirMessageWithoutMinFilters, testIProvConfig.AcceptDirMessageWithoutMinFilters);
        Assert.Equal(defaultIProviderConfig.AcceptMessageSameKeyButDiffStream, testIProvConfig.AcceptMessageSameKeyButDiffStream);
        Assert.Equal(defaultIProviderConfig.AcceptMessageThatChangesService, testIProvConfig.AcceptMessageThatChangesService);
        Assert.Equal(defaultIProviderConfig.AcceptMessageWithoutAcceptingRequests, testIProvConfig.AcceptMessageWithoutAcceptingRequests);
        Assert.Equal(defaultIProviderConfig.AcceptMessageWithoutBeingLogin, testIProvConfig.AcceptMessageWithoutBeingLogin);
        Assert.Equal(defaultIProviderConfig.AcceptMessageWithoutQosInRange, testIProvConfig.AcceptMessageWithoutQosInRange);
        Assert.Equal(defaultIProviderConfig.EnforceAckIDValidation, testIProvConfig.EnforceAckIDValidation);
        Assert.Equal(defaultIProviderConfig.EnumTypeFragmentSize, testIProvConfig.EnumTypeFragmentSize);
        Assert.Equal(defaultIProviderConfig.FieldDictionaryFragmentSize, testIProvConfig.FieldDictionaryFragmentSize);
        Assert.Same(testIProvConfig, copiedConfig.IProviderConfig);

        // TestChannel_2 is the default
        testServerConfig = copiedConfig.ServerConfigMap["TestServer_2"];

        Assert.Equal("TestServer_2", testServerConfig.Name);
        Assert.Equal(defaultServerConfig.BindOptions.MinPingTimeout, testServerConfig.BindOptions.MinPingTimeout);
        Assert.Equal(defaultServerConfig.BindOptions.PingTimeout, testServerConfig.BindOptions.PingTimeout);
        Assert.Equal(defaultServerConfig.CompressionThresholdSet, testServerConfig.CompressionThresholdSet);
        Assert.Equal(defaultServerConfig.CompressionThreshold, testServerConfig.CompressionThreshold);
        Assert.Equal(defaultServerConfig.BindOptions.CompressionType, testServerConfig.BindOptions.CompressionType);
        Assert.Equal(defaultServerConfig.DirectWrite, testServerConfig.DirectWrite);
        Assert.Equal(defaultServerConfig.BindOptions.GuaranteedOutputBuffers, testServerConfig.BindOptions.GuaranteedOutputBuffers);
        Assert.Equal(defaultServerConfig.HighWaterMark, testServerConfig.HighWaterMark);
        Assert.Equal(defaultServerConfig.InitializationTimeout, testServerConfig.InitializationTimeout);
        Assert.Equal(defaultServerConfig.BindOptions.InterfaceName, testServerConfig.BindOptions.InterfaceName);
        Assert.Equal(defaultServerConfig.BindOptions.MaxFragmentSize, testServerConfig.BindOptions.MaxFragmentSize);
        Assert.Equal(defaultServerConfig.BindOptions.NumInputBuffers, testServerConfig.BindOptions.NumInputBuffers);
        Assert.Equal("testMethodPort", testServerConfig.BindOptions.ServiceName);
        Assert.Equal(defaultServerConfig.BindOptions.ConnectionType, testServerConfig.BindOptions.ConnectionType);
        Assert.Equal(defaultServerConfig.BindOptions.SysRecvBufSize, testServerConfig.BindOptions.SysRecvBufSize);
        Assert.Equal(defaultServerConfig.BindOptions.SysSendBufSize, testServerConfig.BindOptions.SysSendBufSize);
        Assert.Equal(defaultServerConfig.BindOptions.TcpOpts.TcpNoDelay, testServerConfig.BindOptions.TcpOpts.TcpNoDelay);
        Assert.Equal("testMethodCert_1", testServerConfig.BindOptions.BindEncryptionOpts.ServerCertificate);
        Assert.Equal("testMethodKey_1", testServerConfig.BindOptions.BindEncryptionOpts.ServerPrivateKey);
        Assert.Single(testServerConfig.BindOptions.BindEncryptionOpts.TlsCipherSuites);
        Assert.Contains(TlsCipherSuite.TLS_RSA_WITH_NULL_SHA, testServerConfig.BindOptions.BindEncryptionOpts.TlsCipherSuites);
        Assert.Equal(defaultServerConfig.BindOptions.BindEncryptionOpts.EncryptionProtocolFlags, testServerConfig.BindOptions.BindEncryptionOpts.EncryptionProtocolFlags);
        Assert.Same(testServerConfig, copiedConfig.ServerConfig);

        testService = copiedConfig.DirectoryCache!.GetService(15);

        Assert.NotNull(testService);
        // This is EMA defaults.
        // Check the Info filter
        Assert.True(testService!.HasInfo);
        Assert.Equal("TestService_2", testService!.Info.ServiceName.ToString());
        Assert.Equal(15, testService!.ServiceId);
        Assert.False(testService!.Info.HasVendor);
        Assert.True(testService!.Info.HasIsSource);
        Assert.Equal(0, testService!.Info.IsSource);
        Assert.Single(testService!.Info.CapabilitiesList);
        Assert.Equal(EmaRdm.MMT_MARKET_PRICE, testService!.Info.CapabilitiesList[0]);
        Assert.True(testService!.Info.HasAcceptingConsStatus);
        Assert.Equal(1, testService!.Info.AcceptConsumerStatus);
        Assert.False(testService!.Info.HasItemList);
        // The dictionaries provided and used in the service will be empty at this point.
        Assert.True(testService!.Info.HasDictionariesUsed);
        Assert.Equal(2, testService!.Info.DictionariesUsedList.Count);
        Assert.Contains("testRdmItem1", testService!.Info.DictionariesUsedList);
        Assert.Contains("testEnumItem1", testService!.Info.DictionariesUsedList);
        Assert.True(testService!.Info.HasDictionariesProvided);
        Assert.Equal(2, testService!.Info.DictionariesProvidedList.Count);
        Assert.Contains("testRdmItem1", testService!.Info.DictionariesProvidedList);
        Assert.Contains("testEnumItem1", testService!.Info.DictionariesProvidedList);
        Assert.True(testService!.Info.HasQos);
        // Default QoS list
        Assert.NotNull(testService!.Info.QosList[0]);
        Assert.Equal(QosTimeliness.REALTIME, testService!.Info.QosList[0].Timeliness());
        Assert.Equal(0, testService!.Info.QosList[0].TimeInfo());
        Assert.Equal(QosRates.TICK_BY_TICK, testService!.Info.QosList[0].Rate());
        Assert.Equal(0, testService!.Info.QosList[0].RateInfo());
        Assert.True(testService!.Info.HasSupportOOBSnapshots);
        Assert.Equal(0, testService!.Info.SupportsOOBSnapshots);
        // Check the state filter
        Assert.True(testService!.HasState);
        Assert.Equal(1, testService!.State.ServiceStateVal);
        Assert.True(testService!.State.HasAcceptingRequests);
        Assert.Equal(1, testService!.State.AcceptingRequests);
        Assert.True(testService!.State.HasStatus);
        Assert.Equal(StreamStates.OPEN, testService!.State.Status.StreamState());
        Assert.Equal(DataStates.OK, testService!.State.Status.DataState());
        Assert.Equal(OmmState.StatusCodes.NONE, testService!.State.Status.Code());
        Assert.Equal(0, testService!.State.Status.Text().Length);
        // Check the load filter
        Assert.True(testService!.HasLoad);
        Assert.False(testService!.Load.HasOpenLimit);
        Assert.False(testService!.Load.HasOpenWindow);
        Assert.False(testService!.Load.HasLoadFactor);

        // Check the directory refresh services generated by the copy constructor from the config
        testService = copiedConfig.DirectoryCache!.GetService(10);

        Assert.NotNull(testService);

        // Check the Info filter
        Assert.True(testService!.HasInfo);
        Assert.Equal("TestService_1", testService!.Info.ServiceName.ToString());
        Assert.Equal(10, testService!.ServiceId);
        Assert.True(testService!.Info.HasVendor);
        Assert.Equal("RTSDK", testService!.Info.Vendor.ToString());
        Assert.True(testService!.Info.HasIsSource);
        Assert.Equal(1, testService!.Info.IsSource);
        Assert.Equal(2, testService!.Info.CapabilitiesList.Count);
        Assert.Equal(EmaRdm.MMT_MARKET_PRICE, testService!.Info.CapabilitiesList[0]);
        Assert.Equal(129, testService!.Info.CapabilitiesList[1]);
        Assert.True(testService!.Info.HasAcceptingConsStatus);
        Assert.Equal(0, testService!.Info.AcceptConsumerStatus);
        Assert.True(testService!.Info.HasItemList);
        Assert.Equal("Items#", testService!.Info.ItemList.ToString());
        // The dictionaries provided and used in the service will be empty at this point.
        Assert.True(testService!.Info.HasDictionariesProvided);
        Assert.Equal(4, testService!.Info.DictionariesProvidedList.Count);
        Assert.Contains("testRdmItem1", testService!.Info.DictionariesProvidedList);
        Assert.Contains("testEnumItem1", testService!.Info.DictionariesProvidedList);
        Assert.Contains(defaultDictConfig.EnumTypeDefItemName, testService!.Info.DictionariesProvidedList);
        Assert.Contains(defaultDictConfig.RdmFieldDictionaryItemName, testService!.Info.DictionariesProvidedList);
        Assert.True(testService!.Info.HasDictionariesUsed);
        Assert.Equal(4, testService!.Info.DictionariesUsedList.Count);
        Assert.Contains("testRdmItem1", testService!.Info.DictionariesUsedList);
        Assert.Contains("testEnumItem1", testService!.Info.DictionariesUsedList);
        Assert.Contains(defaultDictConfig.EnumTypeDefItemName, testService!.Info.DictionariesUsedList);
        Assert.Contains(defaultDictConfig.RdmFieldDictionaryItemName, testService!.Info.DictionariesUsedList);
        Assert.True(testService!.Info.HasQos);
        Assert.Equal(2, testService!.Info.QosList.Count);
        Assert.Equal(QosTimeliness.REALTIME, testService!.Info.QosList[0].Timeliness());
        Assert.Equal(0, testService!.Info.QosList[0].TimeInfo());
        Assert.Equal(QosRates.TICK_BY_TICK, testService!.Info.QosList[0].Rate());
        Assert.Equal(0, testService!.Info.QosList[0].RateInfo());
        Assert.Equal(QosTimeliness.DELAYED, testService!.Info.QosList[1].Timeliness());
        Assert.Equal(10, testService!.Info.QosList[1].TimeInfo());
        Assert.Equal(QosRates.TIME_CONFLATED, testService!.Info.QosList[1].Rate());
        Assert.Equal(20, testService!.Info.QosList[1].RateInfo());
        Assert.True(testService!.Info.HasSupportQosRange);
        Assert.Equal(1, testService!.Info.SupportsQosRange);
        Assert.True(testService!.Info.HasSupportOOBSnapshots);
        Assert.Equal(1, testService!.Info.SupportsOOBSnapshots);
        // Check the state filter
        Assert.True(testService!.HasState);
        Assert.Equal(0, testService!.State.ServiceStateVal);
        Assert.True(testService!.State.HasAcceptingRequests);
        Assert.Equal(1, testService!.State.AcceptingRequests);
        Assert.True(testService!.State.HasStatus);
        Assert.Equal(StreamStates.CLOSED, testService!.State.Status.StreamState());
        Assert.Equal(DataStates.SUSPECT, testService!.State.Status.DataState());
        Assert.Equal(OmmState.StatusCodes.NOT_AUTHORIZED, testService!.State.Status.Code());
        Assert.Equal("TestText", testService!.State.Status.Text().ToString());
        // Check the load filter
        Assert.True(testService!.HasLoad);
        Assert.True(testService!.Load.HasOpenLimit);
        Assert.Equal(100, testService!.Load.OpenLimit);
        Assert.True(testService!.Load.HasOpenWindow);
        Assert.Equal(110, testService!.Load.OpenWindow);
        Assert.True(testService!.Load.HasLoadFactor);
        Assert.Equal(120, testService!.Load.LoadFactor);
    }

    [Fact]
    public void IProviderProgrammaticConfigTest()
    {
        OmmIProviderConfig iProvConfig;
        IProviderConfig testIProvConfig;
        ServerConfig testServerConfig;
        LoggerConfig testLoggerConfig;
        DictionaryConfig testDictConfig;
        DirectoryConfig testDirectoryConfig;
        EmaServiceConfig testServiceConfig;

        // Top level map
        Map outerMap = new Map();
        // Middle map for consumers, channels, loggers, dictionaries
        Map innerMap = new Map();
        // Outer element list 
        ElementList encodeGroupList = new ElementList();
        ElementList encodeObjectList = new ElementList();

        // Service Map
        Map serviceMap = new Map();
        ElementList serviceElementList = new ElementList();
        ElementList serviceFilterList = new ElementList();
        ElementList innerServiceElementList = new ElementList();
        Series qosSeries = new Series();
        OmmArray serviceArray = new OmmArray();

        // Load a blank config so we can be sure that everything added is from the programmtic config
        // This doesn't use the default behavior(loading EmaConfig.xml) because an EmaConfig.xml file may be present in the directory for other example apps
        iProvConfig = new OmmIProviderConfig("../../../OmmConfigTests/EmaBlankConfig.xml");

        OmmIProviderConfigImpl iProvConfigImpl = iProvConfig.OmmIProvConfigImpl;

        // Loaded the Config, now make sure everything's in it.

        // There should be nothing in there.
        Assert.Empty(iProvConfigImpl.IProviderConfigMap);
        Assert.Empty(iProvConfigImpl.ServerConfigMap);
        Assert.Empty(iProvConfigImpl.LoggerConfigMap);
        Assert.Empty(iProvConfigImpl.DictionaryConfigMap);
        Assert.Empty(iProvConfigImpl.DirectoryConfigMap);

        outerMap.Clear();

        innerMap.Clear();

        encodeGroupList.Clear();

        encodeObjectList.Clear();

        // Encode ProgIProvider_1
        //
        encodeObjectList.AddAscii("Server", "ProgServer_1")
            .AddAscii("Logger", "ProgLogger_1")
            .AddAscii("Directory", "ProgDirectory_1")
            .AddInt("DispatchTimeoutApiThread", 2020)
            .AddUInt("ItemCountHint", 2030)
            .AddUInt("MaxDispatchCountApiThread", 2040)
            .AddUInt("MaxDispatchCountUserThread", 2050)
            .AddUInt("RefreshFirstRequired", 0)
            .AddUInt("RequestTimeout", 2060)
            .AddUInt("ServiceCountHint", 2070)
            // XML tracing block
            .AddUInt("XmlTraceToStdout", 1)
            .AddUInt("XmlTraceToFile", 0)
            .AddUInt("XmlTraceMaxFileSize", (ulong)10_000_000)
            .AddAscii("XmlTraceFileName", "EmaTrace")
            .AddUInt("XmlTraceToMultipleFiles", 0)
            .AddUInt("XmlTraceWrite", 1)
            .AddUInt("XmlTraceRead", 1)
            .AddUInt("XmlTracePing", 0)
            .AddUInt("AcceptDirMessageWithoutMinFilters", 1)
            .AddUInt("AcceptMessageSameKeyButDiffStream", 1)
            .AddUInt("AcceptMessageThatChangesService", 1)
            .AddUInt("AcceptMessageWithoutAcceptingRequests", 1)
            .AddUInt("AcceptMessageWithoutBeingLogin", 1)
            .AddUInt("AcceptMessageWithoutQosInRange", 1)
            .AddUInt("EnforceAckIDValidation", 1)
            .AddUInt("EnumTypeFragmentSize", 2080)
            .AddUInt("FieldDictionaryFragmentSize", 2090)
            .MarkForClear().Complete();

        innerMap.AddKeyAscii("ProgIProvider_1", MapAction.ADD, encodeObjectList);

        encodeObjectList.Clear();

        encodeObjectList.AddAscii("Server", "ProgServer_2")
            .MarkForClear().Complete();

        innerMap.AddKeyAscii("ProgIProvider_2", MapAction.ADD, encodeObjectList);

        innerMap.MarkForClear().Complete();

        encodeGroupList.Clear();

        encodeGroupList.AddAscii("DefaultIProvider", "ProgIProvider_1")
            .AddMap("IProviderList", innerMap)
            .MarkForClear().Complete();

        outerMap.AddKeyAscii("IProviderGroup", MapAction.ADD, encodeGroupList);


        // Start encoding the channel connection information
        encodeGroupList.Clear();
        innerMap.Clear();
        encodeObjectList.Clear();

        encodeObjectList.AddEnum("ServerType", ConnectionTypeEnum.ENCRYPTED)
            .AddUInt("ConnectionMinPingTimeout", 3010000)
            .AddUInt("ConnectionPingTimeout", 3020000)
            .AddUInt("CompressionThreshold", 3030)
            .AddEnum("CompressionType", CompressionTypeEnum.LZ4)
            .AddUInt("DirectWrite", 1)
            .AddUInt("GuaranteedOutputBuffers", 3040)
            .AddUInt("HighWaterMark", 3050)
            .AddUInt("InitializationTimeout", 3060)
            .AddAscii("InterfaceName", "ProgInterface_1")
            .AddUInt("MaxFragmentSize", 3070)
            .AddUInt("NumInputBuffers", 3080)
            .AddAscii("Port", "ProgPort_1")
            .AddUInt("SysRecvBufSize", 3090)
            .AddUInt("SysSendBufSize", 3100)
            .AddUInt("TcpNodelay", 1)
            .AddAscii("ServerCert", "ProgCert_1")
            .AddAscii("ServerPrivateKey", "ProgKey_1")
            .AddUInt("SecurityProtocol", 6)
            .AddAscii("CipherSuite", "TLS_RSA_WITH_NULL_MD5, TLS_RSA_WITH_NULL_SHA, 3, 5")
            .AddUInt("AuthenticationTimeout", 30000)
            .MarkForClear().Complete();

        innerMap.AddKeyAscii("ProgServer_1", MapAction.ADD, encodeObjectList);


        // Second channel is all defaults, so add an empty Element List
        encodeObjectList.Clear();
        encodeObjectList.MarkForClear().Complete();

        innerMap.AddKeyAscii("ProgServer_2", MapAction.ADD, encodeObjectList)
            .MarkForClear().Complete();

        encodeGroupList.AddMap("ServerList", innerMap)
            .MarkForClear().Complete();

        outerMap.AddKeyAscii("ServerGroup", MapAction.ADD, encodeGroupList);

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
            .MarkForClear().Complete();

        innerMap.AddKeyAscii("ProgLogger_1", MapAction.ADD, encodeObjectList);

        // Blank logger config
        encodeObjectList.Clear();
        encodeObjectList.MarkForClear().Complete();

        innerMap.AddKeyAscii("ProgLogger_2", MapAction.ADD, encodeObjectList)
            .MarkForClear().Complete();

        encodeGroupList.AddMap("LoggerList", innerMap)
            .MarkForClear().Complete();

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
            .MarkForClear().Complete();

        innerMap.AddKeyAscii("ProgDictionary_1", MapAction.ADD, encodeObjectList);

        encodeObjectList.Clear();
        encodeObjectList.MarkForClear().Complete();

        innerMap.AddKeyAscii("ProgDictionary_2", MapAction.ADD, encodeObjectList)
            .MarkForClear().Complete();

        encodeGroupList.AddMap("DictionaryList", innerMap)
            .MarkForClear().Complete();

        outerMap.AddKeyAscii("DictionaryGroup", MapAction.ADD, encodeGroupList);

        // Encode a couple of directories.
        // Directory Group element list
        encodeGroupList.Clear();
        // DirectoryList map
        innerMap.Clear();
        // Services map
        serviceMap.Clear();
        // element list for the filter entries(info, state, etc.)
        serviceFilterList.Clear();
        // Filter elements element list
        serviceElementList.Clear();

        // Additional elements that need to be encoded.
        qosSeries.Clear();
        serviceArray.Clear();
        innerServiceElementList.Clear();

        // Default directory of progDirectory.
        encodeGroupList.AddAscii("DefaultDirectory", "progDirectory_1");

        // Start encoding the first directory here, starting with the first service
        // Encode the InfoFilter
        serviceElementList.AddUInt("ServiceId", 11)
                            .AddAscii("Vendor", "progVendor")
                            .AddUInt("IsSource", 1);

        serviceArray.AddUInt(130)
                    .AddUInt(5)
                    .MarkForClear().Complete();

        serviceElementList.AddArray("Capabilities", serviceArray)
                            .AddUInt("AcceptingConsumerStatus", 1)
                            .AddAscii("ItemList", "progItem");

        serviceArray.Clear();
        serviceArray.AddAscii("ProgDictionary_1")
                    .AddAscii("ProgDictionary_2")
                    .MarkForClear().Complete();
        serviceElementList.AddArray("DictionariesProvided", serviceArray);

        serviceArray.Clear();
        serviceArray.AddAscii("ProgDictionary_1")
                    .AddAscii("ProgDictionary_2")
                    .MarkForClear().Complete();
        serviceElementList.AddArray("DictionariesUsed", serviceArray);

        qosSeries.Clear();
        innerServiceElementList.Clear();
        innerServiceElementList.AddAscii("Timeliness", "Timeliness::InexactDelayed")
                                .AddAscii("Rate", "Rate::JustInTimeConflated")
                                .MarkForClear().Complete();

        qosSeries.AddEntry(innerServiceElementList);

        innerServiceElementList.Clear();
        innerServiceElementList.AddUInt("Timeliness", 100)
                                .AddUInt("Rate", 200)
                                .MarkForClear().Complete();
        qosSeries.AddEntry(innerServiceElementList)
                    .MarkForClear().Complete();

        serviceElementList.AddSeries("QoS", qosSeries);

        serviceElementList.AddUInt("SupportsQoSRange", 1)
                            .AddUInt("SupportsOutOfBandSnapshots", 1)
                            .MarkForClear().Complete();

        serviceFilterList.AddElementList("InfoFilter", serviceElementList);

        // Encode the State filter
        serviceElementList.Clear();

        serviceElementList.AddUInt("ServiceState", 0)
                            .AddUInt("AcceptingRequests", 1);

        innerServiceElementList.Clear();
        innerServiceElementList.AddAscii("StreamState", "StreamState::ClosedRedirected")
                                .AddAscii("DataState", "DataState::Suspect")
                                .AddAscii("StatusCode", "StatusCode::NotOpen")
                                .AddAscii("StatusText", "ProgStatusText")
                                .MarkForClear().Complete();

        serviceElementList.AddElementList("Status", innerServiceElementList)
                            .MarkForClear().Complete();

        serviceFilterList.AddElementList("StateFilter", serviceElementList);

        // Encode the Load Filter
        serviceElementList.Clear();

        serviceElementList.AddUInt("OpenLimit", 500)
                            .AddUInt("OpenWindow", 600)
                            .AddUInt("LoadFactor", 700)
                            .MarkForClear().Complete();
        serviceFilterList.AddElementList("LoadFilter", serviceElementList)
                            .MarkForClear().Complete();

        serviceMap.AddKeyAscii("ProgService_1", MapAction.ADD, serviceFilterList);

        // Encode an all-default service, except for service Id and capabilities
        serviceFilterList.Clear();

        serviceElementList.Clear();
        serviceArray.Clear();
        serviceElementList.AddUInt("ServiceId", 12)
                            .AddUInt("IsSource", 1);

        serviceArray.AddAscii("MMT_HISTORY")
                    .MarkForClear().Complete();

        serviceElementList.AddArray("Capabilities", serviceArray)
                            .MarkForClear().Complete();
        serviceFilterList.AddElementList("InfoFilter", serviceElementList)
                        .MarkForClear().Complete();

        serviceMap.AddKeyAscii("ProgService_2", MapAction.ADD, serviceFilterList)
                    .MarkForClear().Complete();

        innerMap.AddKeyAscii("ProgDirectory_1", MapAction.ADD, serviceMap)
                .MarkForClear().Complete();

        encodeGroupList.AddMap("DirectoryList", innerMap)
                        .MarkForClear().Complete();

        outerMap.AddKeyAscii("DirectoryGroup", MapAction.ADD, encodeGroupList);

        outerMap.MarkForClear().Complete();

        try
        {
            iProvConfigImpl.Config(outerMap);
        }
        catch (Exception excp)
        {
           Assert.Fail(excp.Message);
            return;
        }

        Assert.Equal(2, iProvConfigImpl.IProviderConfigMap.Count);
        Assert.Equal(2, iProvConfigImpl.ServerConfigMap.Count);
        Assert.Equal(2, iProvConfigImpl.LoggerConfigMap.Count);
        Assert.Equal(2, iProvConfigImpl.DictionaryConfigMap.Count);
        Assert.Single(iProvConfigImpl.DirectoryConfigMap);

        Assert.Empty(iProvConfigImpl.ConfigErrorLog!.ErrorList);


        try
        {
            testIProvConfig = iProvConfigImpl.IProviderConfigMap["ProgIProvider_1"];
        }
        catch (Exception excp)
        {
           Assert.Fail(excp.Message);
            return;
        }

        Assert.Equal("ProgIProvider_1", testIProvConfig.Name);
        Assert.Equal("ProgServer_1", testIProvConfig.Server);
        Assert.Equal("ProgLogger_1", testIProvConfig.Logger);
        Assert.Equal("ProgDirectory_1", testIProvConfig.Directory);
        Assert.Equal((long)2020, testIProvConfig.DispatchTimeoutApiThread);
        Assert.Equal((ulong)2030, testIProvConfig.ItemCountHint);
        Assert.Equal((int)2040, testIProvConfig.MaxDispatchCountApiThread);
        Assert.Equal((int)2050, testIProvConfig.MaxDispatchCountUserThread);
        Assert.False(testIProvConfig.RefreshFirstRequired);
        Assert.Equal((ulong)2060, testIProvConfig.RequestTimeout);
        Assert.Equal((int)2070, testIProvConfig.ServiceCountHint);
        // XML tracing block
        Assert.True(testIProvConfig.XmlTraceToStdout);
        Assert.False(testIProvConfig.XmlTraceToFile);
        Assert.Equal((ulong)10_000_000, testIProvConfig.XmlTraceMaxFileSize);
        Assert.Equal("EmaTrace", testIProvConfig.XmlTraceFileName);
        Assert.False(testIProvConfig.XmlTraceToMultipleFiles);
        Assert.True(testIProvConfig.XmlTraceWrite);
        Assert.True(testIProvConfig.XmlTraceRead);
        Assert.False(testIProvConfig.XmlTracePing);
        Assert.True(testIProvConfig.AcceptDirMessageWithoutMinFilters);
        Assert.True(testIProvConfig.AcceptMessageSameKeyButDiffStream);
        Assert.True(testIProvConfig.AcceptMessageThatChangesService);
        Assert.True(testIProvConfig.AcceptMessageWithoutAcceptingRequests);
        Assert.True(testIProvConfig.AcceptMessageWithoutBeingLogin);
        Assert.True(testIProvConfig.AcceptMessageWithoutQosInRange);
        Assert.True(testIProvConfig.EnforceAckIDValidation);
        Assert.Equal((int)2080, testIProvConfig.EnumTypeFragmentSize);
        Assert.Equal((int)2090, testIProvConfig.FieldDictionaryFragmentSize);



        // ProgIProvider_2 has all defaults except for ChannelSet.
        testIProvConfig = iProvConfigImpl.IProviderConfigMap["ProgIProvider_2"];

        Assert.Equal("ProgIProvider_2", testIProvConfig.Name);
        Assert.Equal("ProgServer_2", testIProvConfig.Server);
        Assert.Equal(defaultIProviderConfig.Logger, testIProvConfig.Logger);
        Assert.Equal(defaultIProviderConfig.Directory, testIProvConfig.Directory);
        Assert.Equal(defaultIProviderConfig.DispatchTimeoutApiThread, testIProvConfig.DispatchTimeoutApiThread);
        Assert.Equal(defaultIProviderConfig.ItemCountHint, testIProvConfig.ItemCountHint);
        Assert.Equal(defaultIProviderConfig.MaxDispatchCountApiThread, testIProvConfig.MaxDispatchCountApiThread);
        Assert.Equal(defaultIProviderConfig.MaxDispatchCountUserThread, testIProvConfig.MaxDispatchCountUserThread);
        Assert.Equal(defaultIProviderConfig.RefreshFirstRequired, testIProvConfig.RefreshFirstRequired);
        Assert.Equal(defaultIProviderConfig.RequestTimeout, testIProvConfig.RequestTimeout);
        Assert.Equal(defaultIProviderConfig.ServiceCountHint, testIProvConfig.ServiceCountHint);
        // XML tracing block
        Assert.Equal(defaultIProviderConfig.XmlTraceToStdout, testIProvConfig.XmlTraceToStdout);
        Assert.Equal(defaultIProviderConfig.XmlTraceToFile, testIProvConfig.XmlTraceToFile);
        Assert.Equal(defaultIProviderConfig.XmlTraceMaxFileSize, testIProvConfig.XmlTraceMaxFileSize);
        Assert.Equal(defaultIProviderConfig.XmlTraceFileName, testIProvConfig.XmlTraceFileName);
        Assert.Equal(defaultIProviderConfig.XmlTraceToMultipleFiles, testIProvConfig.XmlTraceToMultipleFiles);
        Assert.Equal(defaultIProviderConfig.XmlTraceWrite, testIProvConfig.XmlTraceWrite);
        Assert.Equal(defaultIProviderConfig.XmlTraceRead, testIProvConfig.XmlTraceRead);
        Assert.Equal(defaultIProviderConfig.XmlTracePing, testIProvConfig.XmlTracePing);
        Assert.Equal(defaultIProviderConfig.AcceptDirMessageWithoutMinFilters, testIProvConfig.AcceptDirMessageWithoutMinFilters);
        Assert.Equal(defaultIProviderConfig.AcceptMessageSameKeyButDiffStream, testIProvConfig.AcceptMessageSameKeyButDiffStream);
        Assert.Equal(defaultIProviderConfig.AcceptMessageThatChangesService, testIProvConfig.AcceptMessageThatChangesService);
        Assert.Equal(defaultIProviderConfig.AcceptMessageWithoutAcceptingRequests, testIProvConfig.AcceptMessageWithoutAcceptingRequests);
        Assert.Equal(defaultIProviderConfig.AcceptMessageWithoutBeingLogin, testIProvConfig.AcceptMessageWithoutBeingLogin);
        Assert.Equal(defaultIProviderConfig.AcceptMessageWithoutQosInRange, testIProvConfig.AcceptMessageWithoutQosInRange);
        Assert.Equal(defaultIProviderConfig.EnforceAckIDValidation, testIProvConfig.EnforceAckIDValidation);
        Assert.Equal(defaultIProviderConfig.EnumTypeFragmentSize, testIProvConfig.EnumTypeFragmentSize);
        Assert.Equal(defaultIProviderConfig.FieldDictionaryFragmentSize, testIProvConfig.FieldDictionaryFragmentSize);

        testServerConfig = iProvConfigImpl.ServerConfigMap["ProgServer_1"];

        Assert.Equal("ProgServer_1", testServerConfig.Name);
        Assert.Equal(3010, testServerConfig.BindOptions.MinPingTimeout);
        Assert.Equal(3020, testServerConfig.BindOptions.PingTimeout);
        Assert.True(testServerConfig.CompressionThresholdSet);
        Assert.Equal(3030, testServerConfig.CompressionThreshold);
        Assert.Equal(Eta.Transports.CompressionType.LZ4, testServerConfig.BindOptions.CompressionType);
        Assert.True(testServerConfig.DirectWrite);
        Assert.Equal(3040, testServerConfig.BindOptions.GuaranteedOutputBuffers);
        Assert.Equal(3050, testServerConfig.HighWaterMark);
        Assert.Equal(3060, testServerConfig.InitializationTimeout);
        Assert.Equal("ProgInterface_1", testServerConfig.BindOptions.InterfaceName);
        Assert.Equal(3070, testServerConfig.BindOptions.MaxFragmentSize);
        Assert.Equal(3080, testServerConfig.BindOptions.NumInputBuffers);
        Assert.Equal("ProgPort_1", testServerConfig.BindOptions.ServiceName);
        Assert.Equal(Eta.Transports.ConnectionType.ENCRYPTED, testServerConfig.BindOptions.ConnectionType);
        Assert.Equal(3090, testServerConfig.BindOptions.SysRecvBufSize);
        Assert.Equal(3100, testServerConfig.BindOptions.SysSendBufSize);
        Assert.True(testServerConfig.BindOptions.TcpOpts.TcpNoDelay);
        Assert.Equal("ProgCert_1", testServerConfig.BindOptions.BindEncryptionOpts.ServerCertificate);
        Assert.Equal("ProgKey_1", testServerConfig.BindOptions.BindEncryptionOpts.ServerPrivateKey);
        Assert.Equal(4, testServerConfig.BindOptions.BindEncryptionOpts.TlsCipherSuites.Count());
        Assert.Contains(TlsCipherSuite.TLS_RSA_WITH_NULL_MD5, testServerConfig.BindOptions.BindEncryptionOpts.TlsCipherSuites);
        Assert.Contains(TlsCipherSuite.TLS_RSA_WITH_NULL_SHA, testServerConfig.BindOptions.BindEncryptionOpts.TlsCipherSuites);
        Assert.Contains(TlsCipherSuite.TLS_RSA_EXPORT_WITH_RC4_40_MD5, testServerConfig.BindOptions.BindEncryptionOpts.TlsCipherSuites);
        Assert.Contains(TlsCipherSuite.TLS_RSA_WITH_RC4_128_SHA, testServerConfig.BindOptions.BindEncryptionOpts.TlsCipherSuites);
        Assert.Equal(EncryptionProtocolFlags.ENC_TLSV1_2, testServerConfig.BindOptions.BindEncryptionOpts.EncryptionProtocolFlags & EncryptionProtocolFlags.ENC_TLSV1_2);
        Assert.Equal(30000, testServerConfig.BindOptions.BindEncryptionOpts.AuthenticationTimeout);

       // ProgChannel_2 is the default
       testServerConfig = iProvConfigImpl.ServerConfigMap["ProgServer_2"];

        Assert.Equal("ProgServer_2", testServerConfig.Name);
        Assert.Equal(defaultServerConfig.BindOptions.MinPingTimeout, testServerConfig.BindOptions.MinPingTimeout);
        Assert.Equal(defaultServerConfig.BindOptions.PingTimeout, testServerConfig.BindOptions.PingTimeout);
        Assert.Equal(defaultServerConfig.CompressionThresholdSet, testServerConfig.CompressionThresholdSet);
        Assert.Equal(defaultServerConfig.CompressionThreshold, testServerConfig.CompressionThreshold);
        Assert.Equal(defaultServerConfig.BindOptions.CompressionType, testServerConfig.BindOptions.CompressionType);
        Assert.Equal(defaultServerConfig.DirectWrite, testServerConfig.DirectWrite);
        Assert.Equal(defaultServerConfig.BindOptions.GuaranteedOutputBuffers, testServerConfig.BindOptions.GuaranteedOutputBuffers);
        Assert.Equal(defaultServerConfig.HighWaterMark, testServerConfig.HighWaterMark);
        Assert.Equal(defaultServerConfig.InitializationTimeout, testServerConfig.InitializationTimeout);
        Assert.Equal(defaultServerConfig.BindOptions.InterfaceName, testServerConfig.BindOptions.InterfaceName);
        Assert.Equal(defaultServerConfig.BindOptions.MaxFragmentSize, testServerConfig.BindOptions.MaxFragmentSize);
        Assert.Equal(defaultServerConfig.BindOptions.NumInputBuffers, testServerConfig.BindOptions.NumInputBuffers);
        Assert.Equal(defaultServerConfig.BindOptions.ServiceName, testServerConfig.BindOptions.ServiceName);
        Assert.Equal(defaultServerConfig.BindOptions.ConnectionType, testServerConfig.BindOptions.ConnectionType);
        Assert.Equal(defaultServerConfig.BindOptions.SysRecvBufSize, testServerConfig.BindOptions.SysRecvBufSize);
        Assert.Equal(defaultServerConfig.BindOptions.SysSendBufSize, testServerConfig.BindOptions.SysSendBufSize);
        Assert.Equal(defaultServerConfig.BindOptions.TcpOpts.TcpNoDelay, testServerConfig.BindOptions.TcpOpts.TcpNoDelay);
        Assert.Equal(defaultServerConfig.BindOptions.BindEncryptionOpts.ServerCertificate, testServerConfig.BindOptions.BindEncryptionOpts.ServerCertificate);
        Assert.Equal(defaultServerConfig.BindOptions.BindEncryptionOpts.ServerPrivateKey, testServerConfig.BindOptions.BindEncryptionOpts.ServerPrivateKey);
        Assert.Null(testServerConfig.BindOptions.BindEncryptionOpts.TlsCipherSuites);
        Assert.Equal(defaultServerConfig.BindOptions.BindEncryptionOpts.EncryptionProtocolFlags, testServerConfig.BindOptions.BindEncryptionOpts.EncryptionProtocolFlags);
        Assert.Equal(10000, testServerConfig.BindOptions.BindEncryptionOpts.AuthenticationTimeout); // Test with deafult value

        testLoggerConfig = iProvConfigImpl.LoggerConfigMap["ProgLogger_1"];

        Assert.Equal("ProgLogger_1", testLoggerConfig.Name);
        Assert.Equal(LoggerType.STDOUT, testLoggerConfig.LoggerType);
        Assert.Equal(LoggerLevel.INFO, testLoggerConfig.LoggerSeverity);
        Assert.Equal("ProgLogFile", testLoggerConfig.FileName);
        Assert.Equal((ulong)1, testLoggerConfig.IncludeDateInLoggerOutput);
        Assert.Equal((ulong)20, testLoggerConfig.NumberOfLogFiles);
        Assert.Equal((ulong)100, testLoggerConfig.MaxLogFileSize);

        // ProgLogger_2 is all defaults
        testLoggerConfig = iProvConfigImpl.LoggerConfigMap["ProgLogger_2"];

        Assert.Equal("ProgLogger_2", testLoggerConfig.Name);
        Assert.Equal(defaultLoggerConfig.LoggerType, testLoggerConfig.LoggerType);
        Assert.Equal(defaultLoggerConfig.LoggerSeverity, testLoggerConfig.LoggerSeverity);
        Assert.Equal(defaultLoggerConfig.FileName, testLoggerConfig.FileName);
        Assert.Equal(defaultLoggerConfig.IncludeDateInLoggerOutput, testLoggerConfig.IncludeDateInLoggerOutput);
        Assert.Equal(defaultLoggerConfig.NumberOfLogFiles, testLoggerConfig.NumberOfLogFiles);
        Assert.Equal(defaultLoggerConfig.MaxLogFileSize, testLoggerConfig.MaxLogFileSize);

        testDictConfig = iProvConfigImpl.DictionaryConfigMap["ProgDictionary_1"];

        Assert.Equal("ProgDictionary_1", testDictConfig.Name);
        Assert.Equal(EmaConfig.DictionaryTypeEnum.FILE, testDictConfig.DictionaryType);
        Assert.Equal("ProgEnumFile", testDictConfig.EnumTypeDefFileName);
        Assert.Equal("ProgEnumItem", testDictConfig.EnumTypeDefItemName);
        Assert.Equal("ProgFieldFile", testDictConfig.RdmFieldDictionaryFileName);
        Assert.Equal("ProgFieldItem", testDictConfig.RdmFieldDictionaryItemName);

        // ProgDictionary_1 is set to defaults
        testDictConfig = iProvConfigImpl.DictionaryConfigMap["ProgDictionary_2"];

        Assert.Equal("ProgDictionary_2", testDictConfig.Name);
        Assert.Equal(defaultDictConfig.DictionaryType, testDictConfig.DictionaryType);
        Assert.Equal(defaultDictConfig.EnumTypeDefFileName, testDictConfig.EnumTypeDefFileName);
        Assert.Equal(defaultDictConfig.EnumTypeDefItemName, testDictConfig.EnumTypeDefItemName);
        Assert.Equal(defaultDictConfig.RdmFieldDictionaryFileName, testDictConfig.RdmFieldDictionaryFileName);
        Assert.Equal(defaultDictConfig.RdmFieldDictionaryItemName, testDictConfig.RdmFieldDictionaryItemName);

        // Verify the Directory configuration here
        // TestDictionary_2 is set to defaults
        testDirectoryConfig = iProvConfigImpl.DirectoryConfigMap["ProgDirectory_1"];

        Assert.Equal("ProgDirectory_1", testDirectoryConfig.Name);
        Assert.Equal(2, testDirectoryConfig.ServiceMap.Count);

        testServiceConfig = testDirectoryConfig.ServiceMap["ProgService_1"];

        // Check the Info filter
        Assert.True(testServiceConfig.Service.HasInfo);
        Assert.Equal("ProgService_1", testServiceConfig.Service.Info.ServiceName.ToString());
        Assert.Equal(11, testServiceConfig.Service.ServiceId);
        Assert.True(testServiceConfig.Service.Info.HasVendor);
        Assert.Equal("progVendor", testServiceConfig.Service.Info.Vendor.ToString());
        Assert.True(testServiceConfig.Service.Info.HasIsSource);
        Assert.Equal(1, testServiceConfig.Service.Info.IsSource);
        Assert.Equal(2, testServiceConfig.Service.Info.CapabilitiesList.Count);
        Assert.Equal(130, testServiceConfig.Service.Info.CapabilitiesList[0]);
        Assert.Equal(5, testServiceConfig.Service.Info.CapabilitiesList[1]);
        Assert.True(testServiceConfig.Service.Info.HasAcceptingConsStatus);
        Assert.Equal(1, testServiceConfig.Service.Info.AcceptConsumerStatus);
        Assert.True(testServiceConfig.Service.Info.HasItemList);
        Assert.Equal("progItem", testServiceConfig.Service.Info.ItemList.ToString());
        // The dictionaries provided and used in the service will be empty at this point.
        Assert.False(testServiceConfig.Service.Info.HasDictionariesProvided);
        Assert.Empty(testServiceConfig.Service.Info.DictionariesProvidedList);
        Assert.Equal(2, testServiceConfig.DictionariesProvidedList.Count);
        Assert.Equal("ProgDictionary_1", testServiceConfig.DictionariesProvidedList[0]);
        Assert.Equal("ProgDictionary_2", testServiceConfig.DictionariesProvidedList[1]);
        Assert.False(testServiceConfig.Service.Info.HasDictionariesUsed);
        Assert.Empty(testServiceConfig.Service.Info.DictionariesUsedList);
        Assert.Equal(2, testServiceConfig.DictionariesUsedList.Count);
        Assert.Equal("ProgDictionary_1", testServiceConfig.DictionariesUsedList[0]);
        Assert.Equal("ProgDictionary_2", testServiceConfig.DictionariesUsedList[1]);
        Assert.True(testServiceConfig.Service.Info.HasQos);
        Assert.Equal(2, testServiceConfig.Service.Info.QosList.Count);
        Assert.Equal(QosTimeliness.DELAYED_UNKNOWN, testServiceConfig.Service.Info.QosList[0].Timeliness());
        Assert.Equal(0, testServiceConfig.Service.Info.QosList[0].TimeInfo());
        Assert.Equal(QosRates.JIT_CONFLATED, testServiceConfig.Service.Info.QosList[0].Rate());
        Assert.Equal(0, testServiceConfig.Service.Info.QosList[0].RateInfo());
        Assert.Equal(QosTimeliness.DELAYED, testServiceConfig.Service.Info.QosList[1].Timeliness());
        Assert.Equal(100, testServiceConfig.Service.Info.QosList[1].TimeInfo());
        Assert.Equal(QosRates.TIME_CONFLATED, testServiceConfig.Service.Info.QosList[1].Rate());
        Assert.Equal(200, testServiceConfig.Service.Info.QosList[1].RateInfo());
        Assert.True(testServiceConfig.Service.Info.HasSupportQosRange);
        Assert.Equal(1, testServiceConfig.Service.Info.SupportsQosRange);
        Assert.True(testServiceConfig.Service.Info.HasSupportOOBSnapshots);
        Assert.Equal(1, testServiceConfig.Service.Info.SupportsOOBSnapshots);
        // Check the state filter
        Assert.True(testServiceConfig.Service.HasState);
        Assert.Equal(0, testServiceConfig.Service.State.ServiceStateVal);
        Assert.True(testServiceConfig.Service.State.HasAcceptingRequests);
        Assert.Equal(1, testServiceConfig.Service.State.AcceptingRequests);
        Assert.True(testServiceConfig.Service.State.HasStatus);
        Assert.Equal(StreamStates.REDIRECTED, testServiceConfig.Service.State.Status.StreamState());
        Assert.Equal(DataStates.SUSPECT, testServiceConfig.Service.State.Status.DataState());
        Assert.Equal(OmmState.StatusCodes.NOT_OPEN, testServiceConfig.Service.State.Status.Code());
        Assert.Equal("ProgStatusText", testServiceConfig.Service.State.Status.Text().ToString());
        // Check the load filter
        Assert.True(testServiceConfig.Service.HasLoad);
        Assert.True(testServiceConfig.Service.Load.HasOpenLimit);
        Assert.Equal(500, testServiceConfig.Service.Load.OpenLimit);
        Assert.True(testServiceConfig.Service.Load.HasOpenWindow);
        Assert.Equal(600, testServiceConfig.Service.Load.OpenWindow);
        Assert.True(testServiceConfig.Service.Load.HasLoadFactor);
        Assert.Equal(700, testServiceConfig.Service.Load.LoadFactor);

        testServiceConfig = testDirectoryConfig.ServiceMap["ProgService_2"];

        // This is EMA defaults.
        // Check the Info filter
        Assert.True(testServiceConfig.Service.HasInfo);
        Assert.Equal("ProgService_2", testServiceConfig.Service.Info.ServiceName.ToString());
        Assert.Equal(12, testServiceConfig.Service.ServiceId);
        Assert.False(testServiceConfig.Service.Info.HasVendor);
        Assert.True(testServiceConfig.Service.Info.HasIsSource);
        Assert.Equal(1, testServiceConfig.Service.Info.IsSource);
        Assert.Single(testServiceConfig.Service.Info.CapabilitiesList);
        Assert.Equal(EmaRdm.MMT_HISTORY, testServiceConfig.Service.Info.CapabilitiesList[0]);
        Assert.True(testServiceConfig.Service.Info.HasAcceptingConsStatus);
        Assert.Equal(1, testServiceConfig.Service.Info.AcceptConsumerStatus);
        Assert.False(testServiceConfig.Service.Info.HasItemList);
        // The dictionaries provided and used in the service will be empty at this point.
        Assert.False(testServiceConfig.Service.Info.HasDictionariesProvided);
        Assert.Empty(testServiceConfig.Service.Info.DictionariesProvidedList);
        Assert.Empty(testServiceConfig.DictionariesProvidedList);
        Assert.False(testServiceConfig.Service.Info.HasDictionariesUsed);
        Assert.Empty(testServiceConfig.Service.Info.DictionariesUsedList);
        Assert.Empty(testServiceConfig.DictionariesUsedList);
        Assert.True(testServiceConfig.Service.Info.HasQos);
        Assert.Empty(testServiceConfig.Service.Info.QosList);
        Assert.True(testServiceConfig.Service.Info.HasSupportOOBSnapshots);
        Assert.Equal(0, testServiceConfig.Service.Info.SupportsOOBSnapshots);
        // Check the state filter
        Assert.True(testServiceConfig.Service.HasState);
        Assert.Equal(1, testServiceConfig.Service.State.ServiceStateVal);
        Assert.True(testServiceConfig.Service.State.HasAcceptingRequests);
        Assert.Equal(1, testServiceConfig.Service.State.AcceptingRequests);
        Assert.True(testServiceConfig.Service.State.HasStatus);
        Assert.Equal(StreamStates.OPEN, testServiceConfig.Service.State.Status.StreamState());
        Assert.Equal(DataStates.OK, testServiceConfig.Service.State.Status.DataState());
        Assert.Equal(OmmState.StatusCodes.NONE, testServiceConfig.Service.State.Status.Code());
        Assert.Equal(0, testServiceConfig.Service.State.Status.Text().Length);
        // Check the load filter
        Assert.True(testServiceConfig.Service.HasLoad);
        Assert.False(testServiceConfig.Service.Load.HasOpenLimit);
        Assert.False(testServiceConfig.Service.Load.HasOpenWindow);
        Assert.False(testServiceConfig.Service.Load.HasLoadFactor);

        // Verify that the config works here
        iProvConfigImpl.VerifyConfiguration();
    }

    [Fact]
    public void IProviderProgrammaticConfigOverlayTest()
    {
        OmmIProviderConfig iProvConfig;
        IProviderConfig testIProvConfig;
        ServerConfig testServerConfig;
        LoggerConfig testLoggerConfig;
        DictionaryConfig testDictConfig;
        DirectoryConfig testDirectoryConfig;
        EmaServiceConfig testServiceConfig;

        // Top level map
        Map outerMap = new Map();
        // Middle map for consumers, channels, loggers, dictionaries
        Map innerMap = new Map();
        // Outer element list 
        ElementList encodeGroupList = new ElementList();
        ElementList encodeObjectList = new ElementList();

        // Service Map
        Map serviceMap = new Map();
        ElementList serviceElementList = new ElementList();
        ElementList serviceFilterList = new ElementList();
        ElementList innerServiceElementList = new ElementList();
        Series qosSeries = new Series();
        OmmArray serviceArray = new OmmArray();

        // Load a blank config so we can be sure that everything added is from the programmtic config
        // This doesn't use the default behavior(loading EmaConfig.xml) because an EmaConfig.xml file may be present in the directory for other example apps
        iProvConfig = new OmmIProviderConfig("../../../OmmConfigTests/EmaBlankConfig.xml");

        OmmIProviderConfigImpl iProvConfigImpl = iProvConfig.OmmIProvConfigImpl;

        // Loaded the Config, now make sure everything's in it.

        // There should be nothing in there.
        Assert.Empty(iProvConfigImpl.IProviderConfigMap);
        Assert.Empty(iProvConfigImpl.ServerConfigMap);
        Assert.Empty(iProvConfigImpl.LoggerConfigMap);
        Assert.Empty(iProvConfigImpl.DictionaryConfigMap);

        outerMap.Clear();

        innerMap.Clear();

        encodeGroupList.Clear();

        encodeObjectList.Clear();

        // Encode ProgNiProvider_1
        //
        encodeObjectList.AddAscii("Server", "ProgServer_1")
            .AddAscii("Logger", "ProgLogger_1")
            .AddAscii("Directory", "ProgDirectory_1")
            .AddInt("DispatchTimeoutApiThread", 2020)
            .AddUInt("ItemCountHint", 2030)
            .AddUInt("MaxDispatchCountApiThread", 2040)
            .AddUInt("MaxDispatchCountUserThread", 2050)
            .AddUInt("RefreshFirstRequired", 0)
            .AddUInt("RequestTimeout", 2060)
            .AddUInt("ServiceCountHint", 2070)
            // XML trace params have values differing from defaults
            .AddUInt("XmlTraceToStdout", 0)
            .AddUInt("XmlTraceToFile", 1)
            .AddUInt("XmlTraceMaxFileSize", (ulong)5_000_000)
            .AddAscii("XmlTraceFileName", "NotEmaTrace")
            .AddUInt("XmlTraceToMultipleFiles", 1)
            .AddUInt("XmlTraceWrite", 0)
            .AddUInt("XmlTraceRead", 0)
            .AddUInt("XmlTracePing", 1)
            .AddUInt("AcceptDirMessageWithoutMinFilters", 1)
            .AddUInt("AcceptMessageSameKeyButDiffStream", 1)
            .AddUInt("AcceptMessageThatChangesService", 1)
            .AddUInt("AcceptMessageWithoutAcceptingRequests", 1)
            .AddUInt("AcceptMessageWithoutBeingLogin", 1)
            .AddUInt("AcceptMessageWithoutQosInRange", 1)
            .AddUInt("EnforceAckIDValidation", 1)
            .AddUInt("EnumTypeFragmentSize", 2080)
            .AddUInt("FieldDictionaryFragmentSize", 2090)
            .MarkForClear().Complete();

        innerMap.AddKeyAscii("ProgIProvider_1", MapAction.ADD, encodeObjectList);

        encodeObjectList.Clear();

        encodeObjectList.AddAscii("Server", "ProgServer_2")
            .MarkForClear().Complete();

        innerMap.AddKeyAscii("ProgIProvider_2", MapAction.ADD, encodeObjectList);

        innerMap.MarkForClear().Complete();

        encodeGroupList.Clear();

        encodeGroupList.AddAscii("DefaultIProvider", "ProgIProvider_1")
            .AddMap("IProviderList", innerMap)
            .MarkForClear().Complete();

        outerMap.AddKeyAscii("IProviderGroup", MapAction.ADD, encodeGroupList);


        // Start encoding the channel connection information
        encodeGroupList.Clear();
        innerMap.Clear();
        encodeObjectList.Clear();

        encodeObjectList.AddEnum("ServerType", ConnectionTypeEnum.ENCRYPTED)
            .AddUInt("ConnectionMinPingTimeout", 3010000)
            .AddUInt("ConnectionPingTimeout", 3020000)
            .AddUInt("CompressionThreshold", 3030)
            .AddEnum("CompressionType", CompressionTypeEnum.ZLIB)
            .AddUInt("DirectWrite", 1)
            .AddUInt("GuaranteedOutputBuffers", 3040)
            .AddUInt("HighWaterMark", 3050)
            .AddUInt("InitializationTimeout", 3060)
            .AddAscii("InterfaceName", "ProgInterface_1")
            .AddUInt("MaxFragmentSize", 3070)
            .AddUInt("NumInputBuffers", 3080)
            .AddAscii("Port", "ProgPort_1")
            .AddUInt("SysRecvBufSize", 3090)
            .AddUInt("SysSendBufSize", 3100)
            .AddUInt("TcpNodelay", 1)
            .AddAscii("ServerCert", "ProgCert_1")
            .AddAscii("ServerPrivateKey", "ProgKey_1")
            .AddUInt("SecurityProtocol", 6)
            .AddAscii("CipherSuite", "TLS_RSA_WITH_NULL_MD5, TLS_RSA_WITH_NULL_SHA, 3, 5")
            .MarkForClear().Complete();

        innerMap.AddKeyAscii("ProgServer_1", MapAction.ADD, encodeObjectList);


        // Second channel is all defaults, so add an empty Element List
        encodeObjectList.Clear();
        encodeObjectList.MarkForClear().Complete();

        innerMap.AddKeyAscii("ProgServer_2", MapAction.ADD, encodeObjectList)
            .MarkForClear().Complete();

        encodeGroupList.AddMap("ServerList", innerMap)
            .MarkForClear().Complete();

        outerMap.AddKeyAscii("ServerGroup", MapAction.ADD, encodeGroupList);

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
            .MarkForClear().Complete();

        innerMap.AddKeyAscii("ProgLogger_1", MapAction.ADD, encodeObjectList);

        // Blank logger config
        encodeObjectList.Clear();
        encodeObjectList.MarkForClear().Complete();

        innerMap.AddKeyAscii("ProgLogger_2", MapAction.ADD, encodeObjectList)
            .MarkForClear().Complete();

        encodeGroupList.AddMap("LoggerList", innerMap)
            .MarkForClear().Complete();

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
            .MarkForClear().Complete();

        innerMap.AddKeyAscii("ProgDictionary_1", MapAction.ADD, encodeObjectList);

        encodeObjectList.Clear();
        encodeObjectList.MarkForClear().Complete();

        innerMap.AddKeyAscii("ProgDictionary_2", MapAction.ADD, encodeObjectList)
            .MarkForClear().Complete();

        encodeGroupList.AddMap("DictionaryList", innerMap)
            .MarkForClear().Complete();

        outerMap.AddKeyAscii("DictionaryGroup", MapAction.ADD, encodeGroupList);

        // Encode a couple of directories.
        // Directory Group element list
        encodeGroupList.Clear();
        // DirectoryList map
        innerMap.Clear();
        // Services map
        serviceMap.Clear();
        // element list for the filter entries(info, state, etc.)
        serviceFilterList.Clear();
        // Filter elements element list
        serviceElementList.Clear();

        // Additional elements that need to be encoded.
        qosSeries.Clear();
        serviceArray.Clear();
        innerServiceElementList.Clear();

        // Default directory of progDirectory.
        encodeGroupList.AddAscii("DefaultDirectory", "progDirectory_1");

        // Start encoding the first directory here, starting with the first service
        // Encode the InfoFilter
        serviceElementList.AddUInt("ServiceId", 11)
                            .AddAscii("Vendor", "progVendor")
                            .AddUInt("IsSource", 1);

        serviceArray.AddUInt(130)
                    .AddUInt(5)
                    .MarkForClear().Complete();

        serviceElementList.AddArray("Capabilities", serviceArray)
                            .AddUInt("AcceptingConsumerStatus", 1)
                            .AddAscii("ItemList", "progItem");

        serviceArray.Clear();
        serviceArray.AddAscii("ProgDictionary_1")
                    .AddAscii("ProgDictionary_2")
                    .MarkForClear().Complete();
        serviceElementList.AddArray("DictionariesProvided", serviceArray);

        serviceArray.Clear();
        serviceArray.AddAscii("ProgDictionary_1")
                    .AddAscii("ProgDictionary_2")
                    .MarkForClear().Complete();
        serviceElementList.AddArray("DictionariesUsed", serviceArray);

        qosSeries.Clear();
        innerServiceElementList.Clear();
        innerServiceElementList.AddAscii("Timeliness", "Timeliness::InexactDelayed")
                                .AddAscii("Rate", "Rate::JustInTimeConflated")
                                .MarkForClear().Complete();

        qosSeries.AddEntry(innerServiceElementList);

        innerServiceElementList.Clear();
        innerServiceElementList.AddUInt("Timeliness", 100)
                                .AddUInt("Rate", 200)
                                .MarkForClear().Complete();
        qosSeries.AddEntry(innerServiceElementList)
                    .MarkForClear().Complete();

        serviceElementList.AddSeries("QoS", qosSeries);

        serviceElementList.AddUInt("SupportsQoSRange", 1)
                            .AddUInt("SupportsOutOfBandSnapshots", 1)
                            .MarkForClear().Complete();

        serviceFilterList.AddElementList("InfoFilter", serviceElementList);

        // Encode the State filter
        serviceElementList.Clear();

        serviceElementList.AddUInt("ServiceState", 0)
                            .AddUInt("AcceptingRequests", 1);

        innerServiceElementList.Clear();
        innerServiceElementList.AddAscii("StreamState", "StreamState::ClosedRedirected")
                                .AddAscii("DataState", "DataState::Suspect")
                                .AddAscii("StatusCode", "StatusCode::NotOpen")
                                .AddAscii("StatusText", "ProgStatusText")
                                .MarkForClear().Complete();

        serviceElementList.AddElementList("Status", innerServiceElementList)
                            .MarkForClear().Complete();

        serviceFilterList.AddElementList("StateFilter", serviceElementList);

        // Encode the Load Filter
        serviceElementList.Clear();

        serviceElementList.AddUInt("OpenLimit", 500)
                            .AddUInt("OpenWindow", 600)
                            .AddUInt("LoadFactor", 700)
                            .MarkForClear().Complete();
        serviceFilterList.AddElementList("LoadFilter", serviceElementList)
                            .MarkForClear().Complete();

        serviceMap.AddKeyAscii("ProgService_1", MapAction.ADD, serviceFilterList);

        // Encode an all-default service, except for service Id and capabilities
        serviceFilterList.Clear();

        serviceElementList.Clear();
        serviceArray.Clear();
        serviceElementList.AddUInt("ServiceId", 12)
                            .AddUInt("IsSource", 1);

        serviceArray.AddAscii("MMT_HISTORY")
                    .MarkForClear().Complete();

        serviceElementList.AddArray("Capabilities", serviceArray)
                            .MarkForClear().Complete();
        serviceFilterList.AddElementList("InfoFilter", serviceElementList)
                        .MarkForClear().Complete();

        serviceMap.AddKeyAscii("ProgService_2", MapAction.ADD, serviceFilterList)
                    .MarkForClear().Complete();

        innerMap.AddKeyAscii("ProgDirectory_1", MapAction.ADD, serviceMap)
                .MarkForClear().Complete();

        encodeGroupList.AddMap("DirectoryList", innerMap)
                        .MarkForClear().Complete();

        outerMap.AddKeyAscii("DirectoryGroup", MapAction.ADD, encodeGroupList);

        outerMap.MarkForClear().Complete();

        // Apply the config and verify the contents
        iProvConfigImpl.Config(outerMap);

        Assert.Equal(2, iProvConfigImpl.IProviderConfigMap.Count);
        Assert.Equal(2, iProvConfigImpl.ServerConfigMap.Count);
        Assert.Equal(2, iProvConfigImpl.LoggerConfigMap.Count);
        Assert.Equal(2, iProvConfigImpl.DictionaryConfigMap.Count);
        Assert.Single(iProvConfigImpl.DirectoryConfigMap);

        Assert.Empty(iProvConfigImpl.ConfigErrorLog!.ErrorList);

        testIProvConfig = iProvConfigImpl.IProviderConfigMap["ProgIProvider_1"];

        Assert.Equal("ProgIProvider_1", testIProvConfig.Name);
        Assert.Equal("ProgServer_1", testIProvConfig.Server);
        Assert.Equal("ProgLogger_1", testIProvConfig.Logger);
        Assert.Equal("ProgDirectory_1", testIProvConfig.Directory);
        Assert.Equal((long)2020, testIProvConfig.DispatchTimeoutApiThread);
        Assert.Equal((ulong)2030, testIProvConfig.ItemCountHint);
        Assert.Equal((int)2040, testIProvConfig.MaxDispatchCountApiThread);
        Assert.Equal((int)2050, testIProvConfig.MaxDispatchCountUserThread);
        Assert.False(testIProvConfig.RefreshFirstRequired);
        Assert.Equal((ulong)2060, testIProvConfig.RequestTimeout);
        Assert.Equal((int)2070, testIProvConfig.ServiceCountHint);
        // XML tracing block
        Assert.False(testIProvConfig.XmlTraceToStdout);
        Assert.True(testIProvConfig.XmlTraceToFile);
        Assert.Equal((ulong)5_000_000, testIProvConfig.XmlTraceMaxFileSize);
        Assert.Equal("NotEmaTrace", testIProvConfig.XmlTraceFileName);
        Assert.True(testIProvConfig.XmlTraceToMultipleFiles);
        Assert.False(testIProvConfig.XmlTraceWrite);
        Assert.False(testIProvConfig.XmlTraceRead);
        Assert.True(testIProvConfig.XmlTracePing);
        Assert.True(testIProvConfig.AcceptDirMessageWithoutMinFilters);
        Assert.True(testIProvConfig.AcceptMessageSameKeyButDiffStream);
        Assert.True(testIProvConfig.AcceptMessageThatChangesService);
        Assert.True(testIProvConfig.AcceptMessageWithoutAcceptingRequests);
        Assert.True(testIProvConfig.AcceptMessageWithoutBeingLogin);
        Assert.True(testIProvConfig.AcceptMessageWithoutQosInRange);
        Assert.True(testIProvConfig.EnforceAckIDValidation);
        Assert.Equal((int)2080, testIProvConfig.EnumTypeFragmentSize);
        Assert.Equal((int)2090, testIProvConfig.FieldDictionaryFragmentSize);



        // ProgIProvider_2 has all defaults except for ChannelSet.
        testIProvConfig = iProvConfigImpl.IProviderConfigMap["ProgIProvider_2"];

        Assert.Equal("ProgIProvider_2", testIProvConfig.Name);
        Assert.Equal("ProgServer_2", testIProvConfig.Server);
        Assert.Equal(defaultIProviderConfig.Logger, testIProvConfig.Logger);
        Assert.Equal(defaultIProviderConfig.Directory, testIProvConfig.Directory);
        Assert.Equal(defaultIProviderConfig.DispatchTimeoutApiThread, testIProvConfig.DispatchTimeoutApiThread);
        Assert.Equal(defaultIProviderConfig.ItemCountHint, testIProvConfig.ItemCountHint);
        Assert.Equal(defaultIProviderConfig.MaxDispatchCountApiThread, testIProvConfig.MaxDispatchCountApiThread);
        Assert.Equal(defaultIProviderConfig.MaxDispatchCountUserThread, testIProvConfig.MaxDispatchCountUserThread);
        Assert.Equal(defaultIProviderConfig.RefreshFirstRequired, testIProvConfig.RefreshFirstRequired);
        Assert.Equal(defaultIProviderConfig.RequestTimeout, testIProvConfig.RequestTimeout);
        Assert.Equal(defaultIProviderConfig.ServiceCountHint, testIProvConfig.ServiceCountHint);
        // XML tracing block
        Assert.Equal(defaultIProviderConfig.XmlTraceToStdout, testIProvConfig.XmlTraceToStdout);
        Assert.Equal(defaultIProviderConfig.XmlTraceToFile, testIProvConfig.XmlTraceToFile);
        Assert.Equal(defaultIProviderConfig.XmlTraceMaxFileSize, testIProvConfig.XmlTraceMaxFileSize);
        Assert.Equal(defaultIProviderConfig.XmlTraceFileName, testIProvConfig.XmlTraceFileName);
        Assert.Equal(defaultIProviderConfig.XmlTraceToMultipleFiles, testIProvConfig.XmlTraceToMultipleFiles);
        Assert.Equal(defaultIProviderConfig.XmlTraceWrite, testIProvConfig.XmlTraceWrite);
        Assert.Equal(defaultIProviderConfig.XmlTraceRead, testIProvConfig.XmlTraceRead);
        Assert.Equal(defaultIProviderConfig.XmlTracePing, testIProvConfig.XmlTracePing);
        Assert.Equal(defaultIProviderConfig.AcceptDirMessageWithoutMinFilters, testIProvConfig.AcceptDirMessageWithoutMinFilters);
        Assert.Equal(defaultIProviderConfig.AcceptMessageSameKeyButDiffStream, testIProvConfig.AcceptMessageSameKeyButDiffStream);
        Assert.Equal(defaultIProviderConfig.AcceptMessageThatChangesService, testIProvConfig.AcceptMessageThatChangesService);
        Assert.Equal(defaultIProviderConfig.AcceptMessageWithoutAcceptingRequests, testIProvConfig.AcceptMessageWithoutAcceptingRequests);
        Assert.Equal(defaultIProviderConfig.AcceptMessageWithoutBeingLogin, testIProvConfig.AcceptMessageWithoutBeingLogin);
        Assert.Equal(defaultIProviderConfig.AcceptMessageWithoutQosInRange, testIProvConfig.AcceptMessageWithoutQosInRange);
        Assert.Equal(defaultIProviderConfig.EnforceAckIDValidation, testIProvConfig.EnforceAckIDValidation);
        Assert.Equal(defaultIProviderConfig.EnumTypeFragmentSize, testIProvConfig.EnumTypeFragmentSize);
        Assert.Equal(defaultIProviderConfig.FieldDictionaryFragmentSize, testIProvConfig.FieldDictionaryFragmentSize);

        testServerConfig = iProvConfigImpl.ServerConfigMap["ProgServer_1"];

        Assert.Equal("ProgServer_1", testServerConfig.Name);
        Assert.Equal(3010, testServerConfig.BindOptions.MinPingTimeout);
        Assert.Equal(3020, testServerConfig.BindOptions.PingTimeout);
        Assert.True(testServerConfig.CompressionThresholdSet);
        Assert.Equal(3030, testServerConfig.CompressionThreshold);
        Assert.Equal(Eta.Transports.CompressionType.ZLIB, testServerConfig.BindOptions.CompressionType);
        Assert.True(testServerConfig.DirectWrite);
        Assert.Equal(3040, testServerConfig.BindOptions.GuaranteedOutputBuffers);
        Assert.Equal(3050, testServerConfig.HighWaterMark);
        Assert.Equal(3060, testServerConfig.InitializationTimeout);
        Assert.Equal("ProgInterface_1", testServerConfig.BindOptions.InterfaceName);
        Assert.Equal(3070, testServerConfig.BindOptions.MaxFragmentSize);
        Assert.Equal(3080, testServerConfig.BindOptions.NumInputBuffers);
        Assert.Equal("ProgPort_1", testServerConfig.BindOptions.ServiceName);
        Assert.Equal(Eta.Transports.ConnectionType.ENCRYPTED, testServerConfig.BindOptions.ConnectionType);
        Assert.Equal(3090, testServerConfig.BindOptions.SysRecvBufSize);
        Assert.Equal(3100, testServerConfig.BindOptions.SysSendBufSize);
        Assert.True(testServerConfig.BindOptions.TcpOpts.TcpNoDelay);
        Assert.Equal("ProgCert_1", testServerConfig.BindOptions.BindEncryptionOpts.ServerCertificate);
        Assert.Equal("ProgKey_1", testServerConfig.BindOptions.BindEncryptionOpts.ServerPrivateKey);
        Assert.Equal(4, testServerConfig.BindOptions.BindEncryptionOpts.TlsCipherSuites.Count());
        Assert.Contains(TlsCipherSuite.TLS_RSA_WITH_NULL_MD5, testServerConfig.BindOptions.BindEncryptionOpts.TlsCipherSuites);
        Assert.Contains(TlsCipherSuite.TLS_RSA_WITH_NULL_SHA, testServerConfig.BindOptions.BindEncryptionOpts.TlsCipherSuites);
        Assert.Contains(TlsCipherSuite.TLS_RSA_EXPORT_WITH_RC4_40_MD5, testServerConfig.BindOptions.BindEncryptionOpts.TlsCipherSuites);
        Assert.Contains(TlsCipherSuite.TLS_RSA_WITH_RC4_128_SHA, testServerConfig.BindOptions.BindEncryptionOpts.TlsCipherSuites);
        Assert.Equal(EncryptionProtocolFlags.ENC_TLSV1_2, testServerConfig.BindOptions.BindEncryptionOpts.EncryptionProtocolFlags & EncryptionProtocolFlags.ENC_TLSV1_2);

        // ProgChannel_2 is the default
        testServerConfig = iProvConfigImpl.ServerConfigMap["ProgServer_2"];

        Assert.Equal("ProgServer_2", testServerConfig.Name);
        Assert.Equal(defaultServerConfig.BindOptions.MinPingTimeout, testServerConfig.BindOptions.MinPingTimeout);
        Assert.Equal(defaultServerConfig.BindOptions.PingTimeout, testServerConfig.BindOptions.PingTimeout);
        Assert.Equal(defaultServerConfig.CompressionThresholdSet, testServerConfig.CompressionThresholdSet);
        Assert.Equal(defaultServerConfig.CompressionThreshold, testServerConfig.CompressionThreshold);
        Assert.Equal(defaultServerConfig.BindOptions.CompressionType, testServerConfig.BindOptions.CompressionType);
        Assert.Equal(defaultServerConfig.DirectWrite, testServerConfig.DirectWrite);
        Assert.Equal(defaultServerConfig.BindOptions.GuaranteedOutputBuffers, testServerConfig.BindOptions.GuaranteedOutputBuffers);
        Assert.Equal(defaultServerConfig.HighWaterMark, testServerConfig.HighWaterMark);
        Assert.Equal(defaultServerConfig.InitializationTimeout, testServerConfig.InitializationTimeout);
        Assert.Equal(defaultServerConfig.BindOptions.InterfaceName, testServerConfig.BindOptions.InterfaceName);
        Assert.Equal(defaultServerConfig.BindOptions.MaxFragmentSize, testServerConfig.BindOptions.MaxFragmentSize);
        Assert.Equal(defaultServerConfig.BindOptions.NumInputBuffers, testServerConfig.BindOptions.NumInputBuffers);
        Assert.Equal(defaultServerConfig.BindOptions.ServiceName, testServerConfig.BindOptions.ServiceName);
        Assert.Equal(defaultServerConfig.BindOptions.ConnectionType, testServerConfig.BindOptions.ConnectionType);
        Assert.Equal(defaultServerConfig.BindOptions.SysRecvBufSize, testServerConfig.BindOptions.SysRecvBufSize);
        Assert.Equal(defaultServerConfig.BindOptions.SysSendBufSize, testServerConfig.BindOptions.SysSendBufSize);
        Assert.Equal(defaultServerConfig.BindOptions.TcpOpts.TcpNoDelay, testServerConfig.BindOptions.TcpOpts.TcpNoDelay);
        Assert.Equal(defaultServerConfig.BindOptions.BindEncryptionOpts.ServerCertificate, testServerConfig.BindOptions.BindEncryptionOpts.ServerCertificate);
        Assert.Equal(defaultServerConfig.BindOptions.BindEncryptionOpts.ServerPrivateKey, testServerConfig.BindOptions.BindEncryptionOpts.ServerPrivateKey);
        Assert.Null(testServerConfig.BindOptions.BindEncryptionOpts.TlsCipherSuites);
        Assert.Equal(defaultServerConfig.BindOptions.BindEncryptionOpts.EncryptionProtocolFlags, testServerConfig.BindOptions.BindEncryptionOpts.EncryptionProtocolFlags);

        testLoggerConfig = iProvConfigImpl.LoggerConfigMap["ProgLogger_1"];

        Assert.Equal("ProgLogger_1", testLoggerConfig.Name);
        Assert.Equal(LoggerType.STDOUT, testLoggerConfig.LoggerType);
        Assert.Equal(LoggerLevel.INFO, testLoggerConfig.LoggerSeverity);
        Assert.Equal("ProgLogFile", testLoggerConfig.FileName);
        Assert.Equal((ulong)1, testLoggerConfig.IncludeDateInLoggerOutput);
        Assert.Equal((ulong)20, testLoggerConfig.NumberOfLogFiles);
        Assert.Equal((ulong)100, testLoggerConfig.MaxLogFileSize);

        // ProgLogger_2 is all defaults
        testLoggerConfig = iProvConfigImpl.LoggerConfigMap["ProgLogger_2"];

        Assert.Equal("ProgLogger_2", testLoggerConfig.Name);
        Assert.Equal(defaultLoggerConfig.LoggerType, testLoggerConfig.LoggerType);
        Assert.Equal(defaultLoggerConfig.LoggerSeverity, testLoggerConfig.LoggerSeverity);
        Assert.Equal(defaultLoggerConfig.FileName, testLoggerConfig.FileName);
        Assert.Equal(defaultLoggerConfig.IncludeDateInLoggerOutput, testLoggerConfig.IncludeDateInLoggerOutput);
        Assert.Equal(defaultLoggerConfig.NumberOfLogFiles, testLoggerConfig.NumberOfLogFiles);
        Assert.Equal(defaultLoggerConfig.MaxLogFileSize, testLoggerConfig.MaxLogFileSize);

        testDictConfig = iProvConfigImpl.DictionaryConfigMap["ProgDictionary_1"];

        Assert.Equal("ProgDictionary_1", testDictConfig.Name);
        Assert.Equal(EmaConfig.DictionaryTypeEnum.FILE, testDictConfig.DictionaryType);
        Assert.Equal("ProgEnumFile", testDictConfig.EnumTypeDefFileName);
        Assert.Equal("ProgEnumItem", testDictConfig.EnumTypeDefItemName);
        Assert.Equal("ProgFieldFile", testDictConfig.RdmFieldDictionaryFileName);
        Assert.Equal("ProgFieldItem", testDictConfig.RdmFieldDictionaryItemName);

        // ProgDictionary_1 is set to defaults
        testDictConfig = iProvConfigImpl.DictionaryConfigMap["ProgDictionary_2"];

        Assert.Equal("ProgDictionary_2", testDictConfig.Name);
        Assert.Equal(defaultDictConfig.DictionaryType, testDictConfig.DictionaryType);
        Assert.Equal(defaultDictConfig.EnumTypeDefFileName, testDictConfig.EnumTypeDefFileName);
        Assert.Equal(defaultDictConfig.EnumTypeDefItemName, testDictConfig.EnumTypeDefItemName);
        Assert.Equal(defaultDictConfig.RdmFieldDictionaryFileName, testDictConfig.RdmFieldDictionaryFileName);
        Assert.Equal(defaultDictConfig.RdmFieldDictionaryItemName, testDictConfig.RdmFieldDictionaryItemName);

        // Verify the Directory configuration here
        // TestDictionary_2 is set to defaults
        testDirectoryConfig = iProvConfigImpl.DirectoryConfigMap["ProgDirectory_1"];

        Assert.Equal("ProgDirectory_1", testDirectoryConfig.Name);
        Assert.Equal(2, testDirectoryConfig.ServiceMap.Count);

        testServiceConfig = testDirectoryConfig.ServiceMap["ProgService_1"];

        // Check the Info filter
        Assert.True(testServiceConfig.Service.HasInfo);
        Assert.Equal("ProgService_1", testServiceConfig.Service.Info.ServiceName.ToString());
        Assert.Equal(11, testServiceConfig.Service.ServiceId);
        Assert.True(testServiceConfig.Service.Info.HasVendor);
        Assert.Equal("progVendor", testServiceConfig.Service.Info.Vendor.ToString());
        Assert.True(testServiceConfig.Service.Info.HasIsSource);
        Assert.Equal(1, testServiceConfig.Service.Info.IsSource);
        Assert.Equal(2, testServiceConfig.Service.Info.CapabilitiesList.Count);
        Assert.Equal(130, testServiceConfig.Service.Info.CapabilitiesList[0]);
        Assert.Equal(5, testServiceConfig.Service.Info.CapabilitiesList[1]);
        Assert.True(testServiceConfig.Service.Info.HasAcceptingConsStatus);
        Assert.Equal(1, testServiceConfig.Service.Info.AcceptConsumerStatus);
        Assert.True(testServiceConfig.Service.Info.HasItemList);
        Assert.Equal("progItem", testServiceConfig.Service.Info.ItemList.ToString());
        // The dictionaries provided and used in the service will be empty at this point.
        Assert.False(testServiceConfig.Service.Info.HasDictionariesProvided);
        Assert.Empty(testServiceConfig.Service.Info.DictionariesProvidedList);
        Assert.Equal(2, testServiceConfig.DictionariesProvidedList.Count);
        Assert.Equal("ProgDictionary_1", testServiceConfig.DictionariesProvidedList[0]);
        Assert.Equal("ProgDictionary_2", testServiceConfig.DictionariesProvidedList[1]);
        Assert.False(testServiceConfig.Service.Info.HasDictionariesUsed);
        Assert.Empty(testServiceConfig.Service.Info.DictionariesUsedList);
        Assert.Equal(2, testServiceConfig.DictionariesUsedList.Count);
        Assert.Equal("ProgDictionary_1", testServiceConfig.DictionariesUsedList[0]);
        Assert.Equal("ProgDictionary_2", testServiceConfig.DictionariesUsedList[1]);
        Assert.True(testServiceConfig.Service.Info.HasQos);
        Assert.Equal(2, testServiceConfig.Service.Info.QosList.Count);
        Assert.Equal(QosTimeliness.DELAYED_UNKNOWN, testServiceConfig.Service.Info.QosList[0].Timeliness());
        Assert.Equal(0, testServiceConfig.Service.Info.QosList[0].TimeInfo());
        Assert.Equal(QosRates.JIT_CONFLATED, testServiceConfig.Service.Info.QosList[0].Rate());
        Assert.Equal(0, testServiceConfig.Service.Info.QosList[0].RateInfo());
        Assert.Equal(QosTimeliness.DELAYED, testServiceConfig.Service.Info.QosList[1].Timeliness());
        Assert.Equal(100, testServiceConfig.Service.Info.QosList[1].TimeInfo());
        Assert.Equal(QosRates.TIME_CONFLATED, testServiceConfig.Service.Info.QosList[1].Rate());
        Assert.Equal(200, testServiceConfig.Service.Info.QosList[1].RateInfo());
        Assert.True(testServiceConfig.Service.Info.HasSupportQosRange);
        Assert.Equal(1, testServiceConfig.Service.Info.SupportsQosRange);
        Assert.True(testServiceConfig.Service.Info.HasSupportOOBSnapshots);
        Assert.Equal(1, testServiceConfig.Service.Info.SupportsOOBSnapshots);
        // Check the state filter
        Assert.True(testServiceConfig.Service.HasState);
        Assert.Equal(0, testServiceConfig.Service.State.ServiceStateVal);
        Assert.True(testServiceConfig.Service.State.HasAcceptingRequests);
        Assert.Equal(1, testServiceConfig.Service.State.AcceptingRequests);
        Assert.True(testServiceConfig.Service.State.HasStatus);
        Assert.Equal(StreamStates.REDIRECTED, testServiceConfig.Service.State.Status.StreamState());
        Assert.Equal(DataStates.SUSPECT, testServiceConfig.Service.State.Status.DataState());
        Assert.Equal(OmmState.StatusCodes.NOT_OPEN, testServiceConfig.Service.State.Status.Code());
        Assert.Equal("ProgStatusText", testServiceConfig.Service.State.Status.Text().ToString());
        // Check the load filter
        Assert.True(testServiceConfig.Service.HasLoad);
        Assert.True(testServiceConfig.Service.Load.HasOpenLimit);
        Assert.Equal(500, testServiceConfig.Service.Load.OpenLimit);
        Assert.True(testServiceConfig.Service.Load.HasOpenWindow);
        Assert.Equal(600, testServiceConfig.Service.Load.OpenWindow);
        Assert.True(testServiceConfig.Service.Load.HasLoadFactor);
        Assert.Equal(700, testServiceConfig.Service.Load.LoadFactor);

        testServiceConfig = testDirectoryConfig.ServiceMap["ProgService_2"];

        // This is EMA defaults.
        // Check the Info filter
        Assert.True(testServiceConfig.Service.HasInfo);
        Assert.Equal("ProgService_2", testServiceConfig.Service.Info.ServiceName.ToString());
        Assert.Equal(12, testServiceConfig.Service.ServiceId);
        Assert.False(testServiceConfig.Service.Info.HasVendor);
        Assert.True(testServiceConfig.Service.Info.HasIsSource);
        Assert.Equal(1, testServiceConfig.Service.Info.IsSource);
        Assert.Single(testServiceConfig.Service.Info.CapabilitiesList);
        Assert.Equal(EmaRdm.MMT_HISTORY, testServiceConfig.Service.Info.CapabilitiesList[0]);
        Assert.True(testServiceConfig.Service.Info.HasAcceptingConsStatus);
        Assert.Equal(1, testServiceConfig.Service.Info.AcceptConsumerStatus);
        Assert.False(testServiceConfig.Service.Info.HasItemList);
        // The dictionaries provided and used in the service will be empty at this point.
        Assert.False(testServiceConfig.Service.Info.HasDictionariesProvided);
        Assert.Empty(testServiceConfig.Service.Info.DictionariesProvidedList);
        Assert.Empty(testServiceConfig.DictionariesProvidedList);
        Assert.False(testServiceConfig.Service.Info.HasDictionariesUsed);
        Assert.Empty(testServiceConfig.Service.Info.DictionariesUsedList);
        Assert.Empty(testServiceConfig.DictionariesUsedList);
        Assert.True(testServiceConfig.Service.Info.HasQos);
        Assert.Empty(testServiceConfig.Service.Info.QosList);
        Assert.True(testServiceConfig.Service.Info.HasSupportOOBSnapshots);
        Assert.Equal(0, testServiceConfig.Service.Info.SupportsOOBSnapshots);
        // Check the state filter
        Assert.True(testServiceConfig.Service.HasState);
        Assert.Equal(1, testServiceConfig.Service.State.ServiceStateVal);
        Assert.True(testServiceConfig.Service.State.HasAcceptingRequests);
        Assert.Equal(1, testServiceConfig.Service.State.AcceptingRequests);
        Assert.True(testServiceConfig.Service.State.HasStatus);
        Assert.Equal(StreamStates.OPEN, testServiceConfig.Service.State.Status.StreamState());
        Assert.Equal(DataStates.OK, testServiceConfig.Service.State.Status.DataState());
        Assert.Equal(OmmState.StatusCodes.NONE, testServiceConfig.Service.State.Status.Code());
        Assert.Equal(0, testServiceConfig.Service.State.Status.Text().Length);
        // Check the load filter
        Assert.True(testServiceConfig.Service.HasLoad);
        Assert.False(testServiceConfig.Service.Load.HasOpenLimit);
        Assert.False(testServiceConfig.Service.Load.HasOpenWindow);
        Assert.False(testServiceConfig.Service.Load.HasLoadFactor);

        // Verify that the config works here
        iProvConfigImpl.VerifyConfiguration();

        // Overlay with changes to all of the _1 elements
        outerMap.Clear();

        innerMap.Clear();

        encodeGroupList.Clear();

        encodeObjectList.Clear();

        // Encode ProgNiProvider_1
        //
        encodeObjectList.AddAscii("Server", "ProgServer_1")
        .AddAscii("Logger", "ProgLogger_1")
        .AddAscii("Directory", "ProgDirectory_1")
        .AddInt("DispatchTimeoutApiThread", 3020)
        .AddUInt("ItemCountHint", 3030)
        .AddUInt("MaxDispatchCountApiThread", 3040)
        .AddUInt("MaxDispatchCountUserThread", 3050)
        .AddUInt("RefreshFirstRequired", 0)
        .AddUInt("RequestTimeout", 3060)
        .AddUInt("ServiceCountHint", 3070)
        // XML tracing block
        .AddUInt("XmlTraceToStdout", 0)
        .AddUInt("XmlTraceToFile", 1)
        .AddUInt("XmlTraceMaxFileSize", (ulong)5_000_000)
        .AddAscii("XmlTraceFileName", "NotEmaTrace")
        .AddUInt("XmlTraceToMultipleFiles", 1)
        .AddUInt("XmlTraceWrite", 0)
        .AddUInt("XmlTraceRead", 0)
        .AddUInt("XmlTracePing", 1)
        .AddUInt("AcceptDirMessageWithoutMinFilters", 0)
        .AddUInt("AcceptMessageSameKeyButDiffStream", 0)
        .AddUInt("AcceptMessageThatChangesService", 0)
        .AddUInt("AcceptMessageWithoutAcceptingRequests", 0)
        .AddUInt("AcceptMessageWithoutBeingLogin", 0)
        .AddUInt("AcceptMessageWithoutQosInRange", 0)
        .AddUInt("EnforceAckIDValidation", 0)
        .AddUInt("EnumTypeFragmentSize", 3080)
        .AddUInt("FieldDictionaryFragmentSize", 3090)
        .MarkForClear().Complete();

        innerMap.AddKeyAscii("ProgIProvider_1", MapAction.ADD, encodeObjectList);

        encodeObjectList.Clear();

        encodeObjectList.AddAscii("Server", "ProgServer_2")
            .MarkForClear().Complete();

        innerMap.AddKeyAscii("ProgIProvider_2", MapAction.ADD, encodeObjectList);

        innerMap.MarkForClear().Complete();

        encodeGroupList.Clear();

        encodeGroupList.AddAscii("DefaultIProvider", "ProgIProvider_1")
            .AddMap("IProviderList", innerMap)
            .MarkForClear().Complete();

        outerMap.AddKeyAscii("IProviderGroup", MapAction.ADD, encodeGroupList);


        // Start encoding the channel connection information
        encodeGroupList.Clear();
        innerMap.Clear();
        encodeObjectList.Clear();

        encodeObjectList.AddEnum("ServerType", ConnectionTypeEnum.SOCKET)
            .AddUInt("ConnectionMinPingTimeout", 4010000)
            .AddUInt("ConnectionPingTimeout", 4020000)
            .AddUInt("CompressionThreshold", 4030)
            .AddEnum("CompressionType", CompressionTypeEnum.LZ4)
            .AddUInt("DirectWrite", 0)
            .AddUInt("GuaranteedOutputBuffers", 4040)
            .AddUInt("HighWaterMark", 4050)
            .AddUInt("InitializationTimeout", 4060)
            .AddAscii("InterfaceName", "ProgInterface_2")
            .AddUInt("MaxFragmentSize", 4070)
            .AddUInt("NumInputBuffers", 4080)
            .AddAscii("Port", "ProgPort_2")
            .AddUInt("SysRecvBufSize", 4090)
            .AddUInt("SysSendBufSize", 4100)
            .AddUInt("TcpNodelay", 0)
            .AddAscii("ServerCert", "ProgCert_2")
            .AddAscii("ServerPrivateKey", "ProgKey_2")
            .AddUInt("SecurityProtocol", 6)
            .AddAscii("CipherSuite", "TLS_RSA_WITH_NULL_MD5")
            .MarkForClear().Complete();

        innerMap.AddKeyAscii("ProgServer_1", MapAction.ADD, encodeObjectList);


        // Second channel is all defaults, so add an empty Element List
        encodeObjectList.Clear();
        encodeObjectList.MarkForClear().Complete();

        innerMap.AddKeyAscii("ProgServer_2", MapAction.ADD, encodeObjectList)
            .MarkForClear().Complete();

        encodeGroupList.AddMap("ServerList", innerMap)
            .MarkForClear().Complete();

        outerMap.AddKeyAscii("ServerGroup", MapAction.ADD, encodeGroupList);

        // Start encoding the Logger information
        encodeGroupList.Clear();
        innerMap.Clear();
        encodeObjectList.Clear();

        encodeObjectList.AddAscii("FileName", "NewProgLogFile")
            .AddUInt("IncludeDateInLoggerOutput", 0)
            .AddUInt("NumberOfLogFiles", 120)
            .AddUInt("MaxLogFileSize", 1100)
            .AddEnum("LoggerSeverity", LoggerLevelEnum.WARNING)
            .AddEnum("LoggerType", LoggerTypeEnum.FILE)
            .MarkForClear().Complete();

        innerMap.AddKeyAscii("ProgLogger_1", MapAction.ADD, encodeObjectList);

        // Blank logger config
        encodeObjectList.Clear();
        encodeObjectList.MarkForClear().Complete();

        innerMap.AddKeyAscii("ProgLogger_2", MapAction.ADD, encodeObjectList)
            .MarkForClear().Complete();

        encodeGroupList.AddMap("LoggerList", innerMap)
            .MarkForClear().Complete();

        outerMap.AddKeyAscii("LoggerGroup", MapAction.ADD, encodeGroupList);

        // Start encoding the Dictionary information
        encodeGroupList.Clear();
        innerMap.Clear();
        encodeObjectList.Clear();

        encodeObjectList.AddAscii("EnumTypeDefFileName", "NewProgEnumFile")
            .AddAscii("EnumTypeDefItemName", "NewProgEnumItem")
            .AddAscii("RdmFieldDictionaryFileName", "NewProgFieldFile")
            .AddAscii("RdmFieldDictionaryItemName", "NewProgFieldItem")
            .AddEnum("DictionaryType", DictionaryTypeEnum.CHANNEL)
            .MarkForClear().Complete();

        innerMap.AddKeyAscii("ProgDictionary_1", MapAction.ADD, encodeObjectList);

        encodeObjectList.Clear();
        encodeObjectList.MarkForClear().Complete();

        innerMap.AddKeyAscii("ProgDictionary_2", MapAction.ADD, encodeObjectList)
            .MarkForClear().Complete();

        encodeGroupList.AddMap("DictionaryList", innerMap)
            .MarkForClear().Complete();

        outerMap.AddKeyAscii("DictionaryGroup", MapAction.ADD, encodeGroupList);

        // Encode a couple of directories.
        // Directory Group element list
        encodeGroupList.Clear();
        // DirectoryList map
        innerMap.Clear();
        // Services map
        serviceMap.Clear();
        // element list for the filter entries(info, state, etc.)
        serviceFilterList.Clear();
        // Filter elements element list
        serviceElementList.Clear();

        // Additional elements that need to be encoded.
        qosSeries.Clear();
        serviceArray.Clear();
        innerServiceElementList.Clear();

        // Default directory of progDirectory.
        encodeGroupList.AddAscii("DefaultDirectory", "progDirectory_1");

        // Start encoding the first directory here, starting with the first service
        // Encode the InfoFilter
        serviceElementList.AddUInt("ServiceId", 111)
                            .AddAscii("Vendor", "newProgVendor")
                            .AddUInt("IsSource", 0);

        serviceArray.AddUInt(131)
                    .AddUInt(6)
                    .MarkForClear().Complete();

        serviceElementList.AddArray("Capabilities", serviceArray)
                            .AddUInt("AcceptingConsumerStatus", 0)
                            .AddAscii("ItemList", "newProgItem");

        serviceArray.Clear();
        serviceArray.AddAscii("ProgDictionary_1")
                    .MarkForClear().Complete();
        serviceElementList.AddArray("DictionariesProvided", serviceArray);

        serviceArray.Clear();
        serviceArray.AddAscii("ProgDictionary_1")
                    .MarkForClear().Complete();
        serviceElementList.AddArray("DictionariesUsed", serviceArray);

        qosSeries.Clear();
        innerServiceElementList.Clear();
        innerServiceElementList.AddAscii("Timeliness", "Timeliness::RealTime")
                                .AddAscii("Rate", "Rate::TickByTick")
                                .MarkForClear().Complete();

        qosSeries.AddEntry(innerServiceElementList);

        innerServiceElementList.Clear();
        innerServiceElementList.AddUInt("Timeliness", 1100)
                                .AddUInt("Rate", 1200)
                                .MarkForClear().Complete();
        qosSeries.AddEntry(innerServiceElementList)
                    .MarkForClear().Complete();

        serviceElementList.AddSeries("QoS", qosSeries);

        serviceElementList.AddUInt("SupportsQoSRange", 0)
                            .AddUInt("SupportsOutOfBandSnapshots", 0)
                            .MarkForClear().Complete();

        serviceFilterList.AddElementList("InfoFilter", serviceElementList);

        // Encode the State filter
        serviceElementList.Clear();

        serviceElementList.AddUInt("ServiceState", 1)
                            .AddUInt("AcceptingRequests", 0);

        innerServiceElementList.Clear();
        innerServiceElementList.AddAscii("StreamState", "StreamState::Closed")
                                .AddAscii("DataState", "DataState::Ok")
                                .AddAscii("StatusCode", "StatusCode::AlreadyOpen")
                                .AddAscii("StatusText", "NewProgStatusText")
                                .MarkForClear().Complete();

        serviceElementList.AddElementList("Status", innerServiceElementList)
                            .MarkForClear().Complete();

        serviceFilterList.AddElementList("StateFilter", serviceElementList);

        // Encode the Load Filter
        serviceElementList.Clear();

        serviceElementList.AddUInt("OpenLimit", 1500)
                            .AddUInt("OpenWindow", 1600)
                            .AddUInt("LoadFactor", 1700)
                            .MarkForClear().Complete();
        serviceFilterList.AddElementList("LoadFilter", serviceElementList)
                            .MarkForClear().Complete();

        serviceMap.AddKeyAscii("ProgService_1", MapAction.ADD, serviceFilterList);

        // Encode an all-default service, except for service Id and capabilities
        serviceFilterList.Clear();

        serviceElementList.Clear();
        serviceArray.Clear();
        serviceElementList.AddUInt("ServiceId", 112)
                            .AddUInt("IsSource", 0);

        serviceArray.AddAscii("MMT_STORY")
                    .MarkForClear().Complete();

        serviceElementList.AddArray("Capabilities", serviceArray)
                            .MarkForClear().Complete();
        serviceFilterList.AddElementList("InfoFilter", serviceElementList)
                        .MarkForClear().Complete();

        serviceMap.AddKeyAscii("ProgService_2", MapAction.ADD, serviceFilterList)
                    .MarkForClear().Complete();

        innerMap.AddKeyAscii("ProgDirectory_1", MapAction.ADD, serviceMap)
                .MarkForClear().Complete();

        encodeGroupList.AddMap("DirectoryList", innerMap)
                        .MarkForClear().Complete();

        outerMap.AddKeyAscii("DirectoryGroup", MapAction.ADD, encodeGroupList);

        outerMap.MarkForClear().Complete();

        // Apply and verify the overlaid config
        iProvConfigImpl.Config(outerMap);

        Assert.Equal(2, iProvConfigImpl.IProviderConfigMap.Count);
        Assert.Equal(2, iProvConfigImpl.ServerConfigMap.Count);
        Assert.Equal(2, iProvConfigImpl.LoggerConfigMap.Count);
        Assert.Equal(2, iProvConfigImpl.DictionaryConfigMap.Count);
        Assert.Single(iProvConfigImpl.DirectoryConfigMap);

        Assert.Empty(iProvConfigImpl.ConfigErrorLog!.ErrorList);

        testIProvConfig = iProvConfigImpl.IProviderConfigMap["ProgIProvider_1"];

        Assert.Equal("ProgIProvider_1", testIProvConfig.Name);
        Assert.Equal("ProgServer_1", testIProvConfig.Server);
        Assert.Equal("ProgLogger_1", testIProvConfig.Logger);
        Assert.Equal("ProgDirectory_1", testIProvConfig.Directory);
        Assert.Equal((long)3020, testIProvConfig.DispatchTimeoutApiThread);
        Assert.Equal((ulong)3030, testIProvConfig.ItemCountHint);
        Assert.Equal((int)3040, testIProvConfig.MaxDispatchCountApiThread);
        Assert.Equal((int)3050, testIProvConfig.MaxDispatchCountUserThread);
        Assert.False(testIProvConfig.RefreshFirstRequired);
        Assert.Equal((ulong)3060, testIProvConfig.RequestTimeout);
        Assert.Equal((int)3070, testIProvConfig.ServiceCountHint);
        // XML tracing block
        Assert.False(testIProvConfig.XmlTraceToStdout);
        Assert.True(testIProvConfig.XmlTraceToFile);
        Assert.Equal((ulong)5_000_000, testIProvConfig.XmlTraceMaxFileSize);
        Assert.Equal("NotEmaTrace", testIProvConfig.XmlTraceFileName);
        Assert.True(testIProvConfig.XmlTraceToMultipleFiles);
        Assert.False(testIProvConfig.XmlTraceWrite);
        Assert.False(testIProvConfig.XmlTraceRead);
        Assert.True(testIProvConfig.XmlTracePing);
        Assert.False(testIProvConfig.AcceptDirMessageWithoutMinFilters);
        Assert.False(testIProvConfig.AcceptMessageSameKeyButDiffStream);
        Assert.False(testIProvConfig.AcceptMessageThatChangesService);
        Assert.False(testIProvConfig.AcceptMessageWithoutAcceptingRequests);
        Assert.False(testIProvConfig.AcceptMessageWithoutBeingLogin);
        Assert.False(testIProvConfig.AcceptMessageWithoutQosInRange);
        Assert.False(testIProvConfig.EnforceAckIDValidation);
        Assert.Equal((int)3080, testIProvConfig.EnumTypeFragmentSize);
        Assert.Equal((int)3090, testIProvConfig.FieldDictionaryFragmentSize);



        // ProgIProvider_2 has all defaults except for ChannelSet.
        testIProvConfig = iProvConfigImpl.IProviderConfigMap["ProgIProvider_2"];

        Assert.Equal("ProgIProvider_2", testIProvConfig.Name);
        Assert.Equal("ProgServer_2", testIProvConfig.Server);
        Assert.Equal(defaultIProviderConfig.Logger, testIProvConfig.Logger);
        Assert.Equal(defaultIProviderConfig.Directory, testIProvConfig.Directory);
        Assert.Equal(defaultIProviderConfig.DispatchTimeoutApiThread, testIProvConfig.DispatchTimeoutApiThread);
        Assert.Equal(defaultIProviderConfig.ItemCountHint, testIProvConfig.ItemCountHint);
        Assert.Equal(defaultIProviderConfig.MaxDispatchCountApiThread, testIProvConfig.MaxDispatchCountApiThread);
        Assert.Equal(defaultIProviderConfig.MaxDispatchCountUserThread, testIProvConfig.MaxDispatchCountUserThread);
        Assert.Equal(defaultIProviderConfig.RefreshFirstRequired, testIProvConfig.RefreshFirstRequired);
        Assert.Equal(defaultIProviderConfig.RequestTimeout, testIProvConfig.RequestTimeout);
        Assert.Equal(defaultIProviderConfig.ServiceCountHint, testIProvConfig.ServiceCountHint);
        // XML tracing block
        Assert.Equal(defaultIProviderConfig.XmlTraceToStdout, testIProvConfig.XmlTraceToStdout);
        Assert.Equal(defaultIProviderConfig.XmlTraceToFile, testIProvConfig.XmlTraceToFile);
        Assert.Equal(defaultIProviderConfig.XmlTraceMaxFileSize, testIProvConfig.XmlTraceMaxFileSize);
        Assert.Equal(defaultIProviderConfig.XmlTraceFileName, testIProvConfig.XmlTraceFileName);
        Assert.Equal(defaultIProviderConfig.XmlTraceToMultipleFiles, testIProvConfig.XmlTraceToMultipleFiles);
        Assert.Equal(defaultIProviderConfig.XmlTraceWrite, testIProvConfig.XmlTraceWrite);
        Assert.Equal(defaultIProviderConfig.XmlTraceRead, testIProvConfig.XmlTraceRead);
        Assert.Equal(defaultIProviderConfig.XmlTracePing, testIProvConfig.XmlTracePing);
        Assert.Equal(defaultIProviderConfig.AcceptDirMessageWithoutMinFilters, testIProvConfig.AcceptDirMessageWithoutMinFilters);
        Assert.Equal(defaultIProviderConfig.AcceptMessageSameKeyButDiffStream, testIProvConfig.AcceptMessageSameKeyButDiffStream);
        Assert.Equal(defaultIProviderConfig.AcceptMessageThatChangesService, testIProvConfig.AcceptMessageThatChangesService);
        Assert.Equal(defaultIProviderConfig.AcceptMessageWithoutAcceptingRequests, testIProvConfig.AcceptMessageWithoutAcceptingRequests);
        Assert.Equal(defaultIProviderConfig.AcceptMessageWithoutBeingLogin, testIProvConfig.AcceptMessageWithoutBeingLogin);
        Assert.Equal(defaultIProviderConfig.AcceptMessageWithoutQosInRange, testIProvConfig.AcceptMessageWithoutQosInRange);
        Assert.Equal(defaultIProviderConfig.EnforceAckIDValidation, testIProvConfig.EnforceAckIDValidation);
        Assert.Equal(defaultIProviderConfig.EnumTypeFragmentSize, testIProvConfig.EnumTypeFragmentSize);
        Assert.Equal(defaultIProviderConfig.FieldDictionaryFragmentSize, testIProvConfig.FieldDictionaryFragmentSize);

        testServerConfig = iProvConfigImpl.ServerConfigMap["ProgServer_1"];

        Assert.Equal("ProgServer_1", testServerConfig.Name);
        Assert.Equal(4010, testServerConfig.BindOptions.MinPingTimeout);
        Assert.Equal(4020, testServerConfig.BindOptions.PingTimeout);
        Assert.True(testServerConfig.CompressionThresholdSet);
        Assert.Equal(4030, testServerConfig.CompressionThreshold);
        Assert.Equal(Eta.Transports.CompressionType.LZ4, testServerConfig.BindOptions.CompressionType);
        Assert.False(testServerConfig.DirectWrite);
        Assert.Equal(4040, testServerConfig.BindOptions.GuaranteedOutputBuffers);
        Assert.Equal(4050, testServerConfig.HighWaterMark);
        Assert.Equal(4060, testServerConfig.InitializationTimeout);
        Assert.Equal("ProgInterface_2", testServerConfig.BindOptions.InterfaceName);
        Assert.Equal(4070, testServerConfig.BindOptions.MaxFragmentSize);
        Assert.Equal(4080, testServerConfig.BindOptions.NumInputBuffers);
        Assert.Equal("ProgPort_2", testServerConfig.BindOptions.ServiceName);
        Assert.Equal(Eta.Transports.ConnectionType.SOCKET, testServerConfig.BindOptions.ConnectionType);
        Assert.Equal(4090, testServerConfig.BindOptions.SysRecvBufSize);
        Assert.Equal(4100, testServerConfig.BindOptions.SysSendBufSize);
        Assert.False(testServerConfig.BindOptions.TcpOpts.TcpNoDelay);
        Assert.Equal("ProgCert_2", testServerConfig.BindOptions.BindEncryptionOpts.ServerCertificate);
        Assert.Equal("ProgKey_2", testServerConfig.BindOptions.BindEncryptionOpts.ServerPrivateKey);
        Assert.Single(testServerConfig.BindOptions.BindEncryptionOpts.TlsCipherSuites);
        Assert.Contains(TlsCipherSuite.TLS_RSA_WITH_NULL_MD5, testServerConfig.BindOptions.BindEncryptionOpts.TlsCipherSuites);
        Assert.Equal(EncryptionProtocolFlags.ENC_TLSV1_2, testServerConfig.BindOptions.BindEncryptionOpts.EncryptionProtocolFlags & EncryptionProtocolFlags.ENC_TLSV1_2);

        // ProgServer_2 is the default
        testServerConfig = iProvConfigImpl.ServerConfigMap["ProgServer_2"];

        Assert.Equal("ProgServer_2", testServerConfig.Name);
        Assert.Equal(defaultServerConfig.BindOptions.MinPingTimeout, testServerConfig.BindOptions.MinPingTimeout);
        Assert.Equal(defaultServerConfig.BindOptions.PingTimeout, testServerConfig.BindOptions.PingTimeout);
        Assert.Equal(defaultServerConfig.CompressionThresholdSet, testServerConfig.CompressionThresholdSet);
        Assert.Equal(defaultServerConfig.CompressionThreshold, testServerConfig.CompressionThreshold);
        Assert.Equal(defaultServerConfig.BindOptions.CompressionType, testServerConfig.BindOptions.CompressionType);
        Assert.Equal(defaultServerConfig.DirectWrite, testServerConfig.DirectWrite);
        Assert.Equal(defaultServerConfig.BindOptions.GuaranteedOutputBuffers, testServerConfig.BindOptions.GuaranteedOutputBuffers);
        Assert.Equal(defaultServerConfig.HighWaterMark, testServerConfig.HighWaterMark);
        Assert.Equal(defaultServerConfig.InitializationTimeout, testServerConfig.InitializationTimeout);
        Assert.Equal(defaultServerConfig.BindOptions.InterfaceName, testServerConfig.BindOptions.InterfaceName);
        Assert.Equal(defaultServerConfig.BindOptions.MaxFragmentSize, testServerConfig.BindOptions.MaxFragmentSize);
        Assert.Equal(defaultServerConfig.BindOptions.NumInputBuffers, testServerConfig.BindOptions.NumInputBuffers);
        Assert.Equal(defaultServerConfig.BindOptions.ServiceName, testServerConfig.BindOptions.ServiceName);
        Assert.Equal(defaultServerConfig.BindOptions.ConnectionType, testServerConfig.BindOptions.ConnectionType);
        Assert.Equal(defaultServerConfig.BindOptions.SysRecvBufSize, testServerConfig.BindOptions.SysRecvBufSize);
        Assert.Equal(defaultServerConfig.BindOptions.SysSendBufSize, testServerConfig.BindOptions.SysSendBufSize);
        Assert.Equal(defaultServerConfig.BindOptions.TcpOpts.TcpNoDelay, testServerConfig.BindOptions.TcpOpts.TcpNoDelay);
        Assert.Equal(defaultServerConfig.BindOptions.BindEncryptionOpts.ServerCertificate, testServerConfig.BindOptions.BindEncryptionOpts.ServerCertificate);
        Assert.Equal(defaultServerConfig.BindOptions.BindEncryptionOpts.ServerPrivateKey, testServerConfig.BindOptions.BindEncryptionOpts.ServerPrivateKey);
        Assert.Null(testServerConfig.BindOptions.BindEncryptionOpts.TlsCipherSuites);
        Assert.Equal(defaultServerConfig.BindOptions.BindEncryptionOpts.EncryptionProtocolFlags, testServerConfig.BindOptions.BindEncryptionOpts.EncryptionProtocolFlags);

        testLoggerConfig = iProvConfigImpl.LoggerConfigMap["ProgLogger_1"];

        Assert.Equal("ProgLogger_1", testLoggerConfig.Name);
        Assert.Equal(LoggerType.FILE, testLoggerConfig.LoggerType);
        Assert.Equal(LoggerLevel.WARNING, testLoggerConfig.LoggerSeverity);
        Assert.Equal("NewProgLogFile", testLoggerConfig.FileName);
        Assert.Equal((ulong)0, testLoggerConfig.IncludeDateInLoggerOutput);
        Assert.Equal((ulong)120, testLoggerConfig.NumberOfLogFiles);
        Assert.Equal((ulong)1100, testLoggerConfig.MaxLogFileSize);

        // ProgLogger_2 is all defaults
        testLoggerConfig = iProvConfigImpl.LoggerConfigMap["ProgLogger_2"];

        Assert.Equal("ProgLogger_2", testLoggerConfig.Name);
        Assert.Equal(defaultLoggerConfig.LoggerType, testLoggerConfig.LoggerType);
        Assert.Equal(defaultLoggerConfig.LoggerSeverity, testLoggerConfig.LoggerSeverity);
        Assert.Equal(defaultLoggerConfig.FileName, testLoggerConfig.FileName);
        Assert.Equal(defaultLoggerConfig.IncludeDateInLoggerOutput, testLoggerConfig.IncludeDateInLoggerOutput);
        Assert.Equal(defaultLoggerConfig.NumberOfLogFiles, testLoggerConfig.NumberOfLogFiles);
        Assert.Equal(defaultLoggerConfig.MaxLogFileSize, testLoggerConfig.MaxLogFileSize);

        testDictConfig = iProvConfigImpl.DictionaryConfigMap["ProgDictionary_1"];

        Assert.Equal("ProgDictionary_1", testDictConfig.Name);
        Assert.Equal(EmaConfig.DictionaryTypeEnum.CHANNEL, testDictConfig.DictionaryType);
        Assert.Equal("NewProgEnumFile", testDictConfig.EnumTypeDefFileName);
        Assert.Equal("NewProgEnumItem", testDictConfig.EnumTypeDefItemName);
        Assert.Equal("NewProgFieldFile", testDictConfig.RdmFieldDictionaryFileName);
        Assert.Equal("NewProgFieldItem", testDictConfig.RdmFieldDictionaryItemName);

        // ProgDictionary_1 is set to defaults
        testDictConfig = iProvConfigImpl.DictionaryConfigMap["ProgDictionary_2"];

        Assert.Equal("ProgDictionary_2", testDictConfig.Name);
        Assert.Equal(defaultDictConfig.DictionaryType, testDictConfig.DictionaryType);
        Assert.Equal(defaultDictConfig.EnumTypeDefFileName, testDictConfig.EnumTypeDefFileName);
        Assert.Equal(defaultDictConfig.EnumTypeDefItemName, testDictConfig.EnumTypeDefItemName);
        Assert.Equal(defaultDictConfig.RdmFieldDictionaryFileName, testDictConfig.RdmFieldDictionaryFileName);
        Assert.Equal(defaultDictConfig.RdmFieldDictionaryItemName, testDictConfig.RdmFieldDictionaryItemName);

        // Verify the Directory configuration here
        // TestDictionary_2 is set to defaults
        testDirectoryConfig = iProvConfigImpl.DirectoryConfigMap["ProgDirectory_1"];

        Assert.Equal("ProgDirectory_1", testDirectoryConfig.Name);
        Assert.Equal(2, testDirectoryConfig.ServiceMap.Count);

        testServiceConfig = testDirectoryConfig.ServiceMap["ProgService_1"];

        // Check the Info filter
        Assert.True(testServiceConfig.Service.HasInfo);
        Assert.Equal("ProgService_1", testServiceConfig.Service.Info.ServiceName.ToString());
        Assert.Equal(111, testServiceConfig.Service.ServiceId);
        Assert.True(testServiceConfig.Service.Info.HasVendor);
        Assert.Equal("newProgVendor", testServiceConfig.Service.Info.Vendor.ToString());
        Assert.True(testServiceConfig.Service.Info.HasIsSource);
        Assert.Equal(0, testServiceConfig.Service.Info.IsSource);
        Assert.Equal(2, testServiceConfig.Service.Info.CapabilitiesList.Count);
        Assert.Equal(131, testServiceConfig.Service.Info.CapabilitiesList[0]);
        Assert.Equal(6, testServiceConfig.Service.Info.CapabilitiesList[1]);
        Assert.True(testServiceConfig.Service.Info.HasAcceptingConsStatus);
        Assert.Equal(0, testServiceConfig.Service.Info.AcceptConsumerStatus);
        Assert.True(testServiceConfig.Service.Info.HasItemList);
        Assert.Equal("newProgItem", testServiceConfig.Service.Info.ItemList.ToString());
        // The dictionaries provided and used in the service will be empty at this point.
        Assert.False(testServiceConfig.Service.Info.HasDictionariesProvided);
        Assert.Empty(testServiceConfig.Service.Info.DictionariesProvidedList);
        Assert.Single(testServiceConfig.DictionariesProvidedList);
        Assert.Equal("ProgDictionary_1", testServiceConfig.DictionariesProvidedList[0]);
        Assert.False(testServiceConfig.Service.Info.HasDictionariesUsed);
        Assert.Empty(testServiceConfig.Service.Info.DictionariesUsedList);
        Assert.Single(testServiceConfig.DictionariesUsedList);
        Assert.Equal("ProgDictionary_1", testServiceConfig.DictionariesUsedList[0]);
        Assert.True(testServiceConfig.Service.Info.HasQos);
        Assert.Equal(2, testServiceConfig.Service.Info.QosList.Count);
        Assert.Equal(QosTimeliness.REALTIME, testServiceConfig.Service.Info.QosList[0].Timeliness());
        Assert.Equal(0, testServiceConfig.Service.Info.QosList[0].TimeInfo());
        Assert.Equal(QosRates.TICK_BY_TICK, testServiceConfig.Service.Info.QosList[0].Rate());
        Assert.Equal(0, testServiceConfig.Service.Info.QosList[0].RateInfo());
        Assert.Equal(QosTimeliness.DELAYED, testServiceConfig.Service.Info.QosList[1].Timeliness());
        Assert.Equal(1100, testServiceConfig.Service.Info.QosList[1].TimeInfo());
        Assert.Equal(QosRates.TIME_CONFLATED, testServiceConfig.Service.Info.QosList[1].Rate());
        Assert.Equal(1200, testServiceConfig.Service.Info.QosList[1].RateInfo());
        Assert.True(testServiceConfig.Service.Info.HasSupportQosRange);
        Assert.Equal(0, testServiceConfig.Service.Info.SupportsQosRange);
        Assert.True(testServiceConfig.Service.Info.HasSupportOOBSnapshots);
        Assert.Equal(0, testServiceConfig.Service.Info.SupportsOOBSnapshots);
        // Check the state filter
        Assert.True(testServiceConfig.Service.HasState);
        Assert.Equal(1, testServiceConfig.Service.State.ServiceStateVal);
        Assert.True(testServiceConfig.Service.State.HasAcceptingRequests);
        Assert.Equal(0, testServiceConfig.Service.State.AcceptingRequests);
        Assert.True(testServiceConfig.Service.State.HasStatus);
        Assert.Equal(StreamStates.CLOSED, testServiceConfig.Service.State.Status.StreamState());
        Assert.Equal(DataStates.OK, testServiceConfig.Service.State.Status.DataState());
        Assert.Equal(OmmState.StatusCodes.ALREADY_OPEN, testServiceConfig.Service.State.Status.Code());
        Assert.Equal("NewProgStatusText", testServiceConfig.Service.State.Status.Text().ToString());
        // Check the load filter
        Assert.True(testServiceConfig.Service.HasLoad);
        Assert.True(testServiceConfig.Service.Load.HasOpenLimit);
        Assert.Equal(1500, testServiceConfig.Service.Load.OpenLimit);
        Assert.True(testServiceConfig.Service.Load.HasOpenWindow);
        Assert.Equal(1600, testServiceConfig.Service.Load.OpenWindow);
        Assert.True(testServiceConfig.Service.Load.HasLoadFactor);
        Assert.Equal(1700, testServiceConfig.Service.Load.LoadFactor);

        testServiceConfig = testDirectoryConfig.ServiceMap["ProgService_2"];

        // This is EMA defaults.
        // Check the Info filter
        Assert.True(testServiceConfig.Service.HasInfo);
        Assert.Equal("ProgService_2", testServiceConfig.Service.Info.ServiceName.ToString());
        Assert.Equal(112, testServiceConfig.Service.ServiceId);
        Assert.False(testServiceConfig.Service.Info.HasVendor);
        Assert.True(testServiceConfig.Service.Info.HasIsSource);
        Assert.Equal(0, testServiceConfig.Service.Info.IsSource);
        Assert.Single(testServiceConfig.Service.Info.CapabilitiesList);
        Assert.Equal(EmaRdm.MMT_STORY, testServiceConfig.Service.Info.CapabilitiesList[0]);
        Assert.True(testServiceConfig.Service.Info.HasAcceptingConsStatus);
        Assert.Equal(1, testServiceConfig.Service.Info.AcceptConsumerStatus);
        Assert.False(testServiceConfig.Service.Info.HasItemList);
        // The dictionaries provided and used in the service will be empty at this point.
        Assert.False(testServiceConfig.Service.Info.HasDictionariesProvided);
        Assert.Empty(testServiceConfig.Service.Info.DictionariesProvidedList);
        Assert.Empty(testServiceConfig.DictionariesProvidedList);
        Assert.False(testServiceConfig.Service.Info.HasDictionariesUsed);
        Assert.Empty(testServiceConfig.Service.Info.DictionariesUsedList);
        Assert.Empty(testServiceConfig.DictionariesUsedList);
        Assert.True(testServiceConfig.Service.Info.HasQos);
        Assert.Empty(testServiceConfig.Service.Info.QosList);
        Assert.True(testServiceConfig.Service.Info.HasSupportOOBSnapshots);
        Assert.Equal(0, testServiceConfig.Service.Info.SupportsOOBSnapshots);
        // Check the state filter
        Assert.True(testServiceConfig.Service.HasState);
        Assert.Equal(1, testServiceConfig.Service.State.ServiceStateVal);
        Assert.True(testServiceConfig.Service.State.HasAcceptingRequests);
        Assert.Equal(1, testServiceConfig.Service.State.AcceptingRequests);
        Assert.True(testServiceConfig.Service.State.HasStatus);
        Assert.Equal(StreamStates.OPEN, testServiceConfig.Service.State.Status.StreamState());
        Assert.Equal(DataStates.OK, testServiceConfig.Service.State.Status.DataState());
        Assert.Equal(OmmState.StatusCodes.NONE, testServiceConfig.Service.State.Status.Code());
        Assert.Equal(0, testServiceConfig.Service.State.Status.Text().Length);
        // Check the load filter
        Assert.True(testServiceConfig.Service.HasLoad);
        Assert.False(testServiceConfig.Service.Load.HasOpenLimit);
        Assert.False(testServiceConfig.Service.Load.HasOpenWindow);
        Assert.False(testServiceConfig.Service.Load.HasLoadFactor);

        // Verify that the config works here
        iProvConfigImpl.VerifyConfiguration();
    }

    // Tests the addadminmsg functionality.
    [Fact]
    public void IProviderAddAdminMsgTest()
    {
        OmmIProviderConfig iProviderConfig;
        OmmIProviderConfigImpl copiedConfig;
        DirectoryConfig testDirectoryConfig;
        EmaServiceConfig testServiceConfig;

        iProviderConfig = new OmmIProviderConfig("../../../OmmConfigTests/EmaTestConfig.xml");

        // Generate a consumer role to ensuure that everything that should be null is null
        // Copy the config
        copiedConfig = new OmmIProviderConfigImpl(iProviderConfig.OmmIProvConfigImpl);

        // There should be a two Services in the directory cache
        Assert.NotNull(copiedConfig.DirectoryCache);
        Assert.Equal(2, copiedConfig.DirectoryCache!.DirectoryRefresh.ServiceList.Count);


        // Setup the absolute bare minimum, avoiding setting values that are pre-populated by EMA
        DirectoryRefresh directoryRefresh = new DirectoryRefresh();
        Service directoryService = new Service();

        directoryService.Action = MapEntryActions.ADD;
        directoryService.ServiceId = 5;
        directoryService.HasInfo = true;
        directoryService.Info.Action = FilterEntryActions.SET;
        directoryService.Info.ServiceName.Data("addMsgService");
        directoryService.Info.CapabilitiesList.Add(20);
        directoryService.Info.CapabilitiesList.Add(50);
        directoryService.Info.HasIsSource = true;
        directoryService.Info.IsSource = 1;

        directoryRefresh.ServiceList.Add(directoryService);

        directoryRefresh.StreamId = 2;
        directoryRefresh.Filter = ServiceFilterFlags.INFO;

        directoryRefresh.State.StreamState(StreamStates.OPEN);
        directoryRefresh.State.DataState(DataStates.OK);

        // Setup the refresh message.  This is so we can directly compare the directoryRefresh we're providing with the internal directoryRefresh.
        RefreshMsg refreshMsg = new RefreshMsg();
        refreshMsg.DomainType((int)DomainType.SOURCE);

        refreshMsg.Encoder!.AcquireEncodeIterator();

        Assert.Equal(CodecReturnCode.SUCCESS, directoryRefresh.Encode(refreshMsg.Encoder!.m_encodeIterator!));

        refreshMsg.Encoder!.m_containerComplete = true;

        iProviderConfig.AddAdminMsg(refreshMsg);

        // Verify the configuration
        iProviderConfig.OmmIProvConfigImpl.VerifyConfiguration();

        // Copy the config
        copiedConfig = new OmmIProviderConfigImpl(iProviderConfig.OmmIProvConfigImpl);

        Assert.NotNull(copiedConfig.DirectoryCache);
        Assert.Single(copiedConfig.DirectoryCache!.DirectoryRefresh.ServiceList);
        Assert.Single(copiedConfig.DirectoryConfigMap);

        testDirectoryConfig = copiedConfig.DirectoryConfigMap["AddAdminDirectory"];

        Assert.Single(testDirectoryConfig.ServiceMap);

        testServiceConfig = copiedConfig.DirectoryConfigMap["AddAdminDirectory"].ServiceMap["addMsgService"];

        DirectoryRefresh copiedRefresh = copiedConfig.DirectoryCache!.DirectoryRefresh;

        Assert.Equal(2, copiedRefresh.StreamId);
        Assert.Equal(ServiceFilterFlags.INFO, copiedRefresh.Filter);

        Assert.Single(copiedRefresh.ServiceList);
        Assert.Equal(MapEntryActions.ADD, copiedRefresh.ServiceList[0].Action);
        Assert.Equal(5, copiedRefresh.ServiceList[0].ServiceId);
        Assert.True(copiedRefresh.ServiceList[0].HasInfo);
        Assert.Equal(FilterEntryActions.SET, copiedRefresh.ServiceList[0].Info.Action);
        Assert.Equal(2, copiedRefresh.ServiceList[0].Info.CapabilitiesList.Count);
        Assert.Equal(20, copiedRefresh.ServiceList[0].Info.CapabilitiesList[0]);
        Assert.Equal(50, copiedRefresh.ServiceList[0].Info.CapabilitiesList[1]);
        Assert.True(copiedRefresh.ServiceList[0].Info.HasIsSource);
        Assert.Equal(1, copiedRefresh.ServiceList[0].Info.IsSource);

        // Check to see if the service in the copied Refresh is the same as the one in the directory service map
        Assert.Same(testServiceConfig.Service, copiedRefresh.ServiceList[0]);

        // Finally, set the admin control to USER_CONTROL, and make sure that there isn't a defined directory refresh
        iProviderConfig.AdminControlDirectory(OmmIProviderConfig.AdminControlMode.USER_CONTROL);

        // Copy the config and generate the role.
        copiedConfig = new OmmIProviderConfigImpl(iProviderConfig.OmmIProvConfigImpl);

        Assert.NotNull(copiedConfig.DirectoryCache);
        Assert.Empty(copiedConfig.DirectoryCache!.DirectoryRefresh.ServiceList);
        Assert.Empty(copiedConfig.DirectoryConfigMap);
    }

    // This test will a blank config and verify that a default-only generated config is created.
    [Fact]
    public void IProviderDefaultConfigTest()
    {
        OmmIProviderConfig iProviderConfig;
        IProviderConfig testIProvConfig;
        ServerConfig testServerConfig;
        LoggerConfig testLoggerConfig;
        OmmIProviderConfigImpl copiedConfig;
        Service? testService;
        DirectoryConfig testDirectoryConfig;
        EmaServiceConfig testServiceConfig;
        DictionaryConfig testDictionaryConfig;

        // Load a blank config so we can be sure that everything added is from the programmtic config
        // This doesn't use the default behavior(loading EmaConfig.xml) because an EmaConfig.xml file may be present in the directory for other example apps
        iProviderConfig = new OmmIProviderConfig("../../../OmmConfigTests/EmaBlankConfig.xml");

        OmmIProviderConfigImpl iProvConfigImpl = iProviderConfig.OmmIProvConfigImpl;

        // Loaded the Config, now make sure everything's in it.

        // There should be nothing in here
        Assert.Empty(iProvConfigImpl.IProviderConfigMap);
        Assert.Empty(iProvConfigImpl.ServerConfigMap);
        Assert.Empty(iProvConfigImpl.LoggerConfigMap);
        Assert.Empty(iProvConfigImpl.DictionaryConfigMap);

        // Verify should succeed
        iProvConfigImpl.VerifyConfiguration();

        copiedConfig = new OmmIProviderConfigImpl(iProvConfigImpl);

        // There should be one default-generated consumer, and one default-generated channel in the maps
        // Logger and Dictionary are set in copiedConfig.Logger and copiedConfig.Dictionary, respectively
        Assert.Single(copiedConfig.IProviderConfigMap);
        Assert.Single(copiedConfig.ServerConfigMap);
        Assert.Single(copiedConfig.LoggerConfigMap);
        Assert.Single(copiedConfig.DictionaryConfigMap);
        Assert.NotNull(copiedConfig.DirectoryCache);
        Assert.Single(copiedConfig.DirectoryCache!.DirectoryRefresh.ServiceList);

        // DefaultEmaConsumer has all defaults and generated names
        testIProvConfig = copiedConfig.IProviderConfigMap["DefaultEmaIProvider"];

        Assert.Same(copiedConfig.IProviderConfig, copiedConfig.IProviderConfigMap["DefaultEmaIProvider"]);
        Assert.Equal(defaultIProviderConfig.Directory, testIProvConfig.Directory);
        Assert.Equal(defaultIProviderConfig.DispatchTimeoutApiThread, testIProvConfig.DispatchTimeoutApiThread);
        Assert.Equal(defaultIProviderConfig.ItemCountHint, testIProvConfig.ItemCountHint);
        Assert.Equal("DefaultEmaLogger", testIProvConfig.Logger);
        Assert.Equal(defaultIProviderConfig.MaxDispatchCountApiThread, testIProvConfig.MaxDispatchCountApiThread);
        Assert.Equal(defaultIProviderConfig.MaxDispatchCountUserThread, testIProvConfig.MaxDispatchCountUserThread);
        Assert.Equal(defaultIProviderConfig.RefreshFirstRequired, testIProvConfig.RefreshFirstRequired);
        Assert.Equal(defaultIProviderConfig.RequestTimeout, testIProvConfig.RequestTimeout);
        Assert.Equal(defaultIProviderConfig.ServiceCountHint, testIProvConfig.ServiceCountHint);
        // XML tracing block
        Assert.Equal(defaultIProviderConfig.XmlTraceToStdout, testIProvConfig.XmlTraceToStdout);
        Assert.Equal(defaultIProviderConfig.XmlTraceToFile, testIProvConfig.XmlTraceToFile);
        Assert.Equal(defaultIProviderConfig.XmlTraceMaxFileSize, testIProvConfig.XmlTraceMaxFileSize);
        Assert.Equal(defaultIProviderConfig.XmlTraceFileName, testIProvConfig.XmlTraceFileName);
        Assert.Equal(defaultIProviderConfig.XmlTraceToMultipleFiles, testIProvConfig.XmlTraceToMultipleFiles);
        Assert.Equal(defaultIProviderConfig.XmlTraceWrite, testIProvConfig.XmlTraceWrite);
        Assert.Equal(defaultIProviderConfig.XmlTraceRead, testIProvConfig.XmlTraceRead);
        Assert.Equal(defaultIProviderConfig.XmlTracePing, testIProvConfig.XmlTracePing);
        Assert.Equal("DefaultEmaServer", testIProvConfig.Server);
        Assert.Equal(defaultIProviderConfig.AcceptDirMessageWithoutMinFilters, testIProvConfig.AcceptDirMessageWithoutMinFilters);
        Assert.Equal(defaultIProviderConfig.AcceptMessageSameKeyButDiffStream, testIProvConfig.AcceptMessageSameKeyButDiffStream);
        Assert.Equal(defaultIProviderConfig.AcceptMessageThatChangesService, testIProvConfig.AcceptMessageThatChangesService);
        Assert.Equal(defaultIProviderConfig.AcceptMessageWithoutAcceptingRequests, testIProvConfig.AcceptMessageWithoutAcceptingRequests);
        Assert.Equal(defaultIProviderConfig.AcceptMessageWithoutBeingLogin, testIProvConfig.AcceptMessageWithoutBeingLogin);
        Assert.Equal(defaultIProviderConfig.AcceptMessageWithoutQosInRange, testIProvConfig.AcceptMessageWithoutQosInRange);
        Assert.Equal(defaultIProviderConfig.EnforceAckIDValidation, testIProvConfig.EnforceAckIDValidation);
        Assert.Equal(defaultIProviderConfig.EnumTypeFragmentSize, testIProvConfig.EnumTypeFragmentSize);
        Assert.Equal(defaultIProviderConfig.FieldDictionaryFragmentSize, testIProvConfig.FieldDictionaryFragmentSize);


        // DefaultEmaChannel is the generated default channel
        testServerConfig = copiedConfig.ServerConfigMap["DefaultEmaServer"];

        Assert.Equal("DefaultEmaServer", testServerConfig.Name);
        Assert.Equal(defaultServerConfig.DirectWrite, testServerConfig.DirectWrite);
        Assert.Equal(defaultServerConfig.InitializationTimeout, testServerConfig.InitializationTimeout);
        Assert.Equal(defaultServerConfig.CompressionThresholdSet, testServerConfig.CompressionThresholdSet);
        Assert.Equal(defaultServerConfig.CompressionThreshold, testServerConfig.CompressionThreshold);
        // Make sure all the bind options match.
        Assert.Equal(defaultServerConfig.BindOptions.ComponentVersion, testServerConfig.BindOptions.ComponentVersion);
        Assert.Equal(defaultServerConfig.BindOptions.ServiceName, testServerConfig.BindOptions.ServiceName);
        Assert.Equal(defaultServerConfig.BindOptions.InterfaceName, testServerConfig.BindOptions.InterfaceName);
        Assert.Equal(defaultServerConfig.BindOptions.CompressionType, testServerConfig.BindOptions.CompressionType);
        Assert.Equal(defaultServerConfig.BindOptions.CompressionLevel, testServerConfig.BindOptions.CompressionLevel);
        Assert.Equal(defaultServerConfig.BindOptions.ForceCompression, testServerConfig.BindOptions.ForceCompression);
        Assert.Equal(defaultServerConfig.BindOptions.ServerBlocking, testServerConfig.BindOptions.ServerBlocking);
        Assert.Equal(defaultServerConfig.BindOptions.ChannelIsBlocking, testServerConfig.BindOptions.ChannelIsBlocking);
        Assert.Equal(defaultServerConfig.BindOptions.ClientToServerPings, testServerConfig.BindOptions.ClientToServerPings);
        Assert.Equal(defaultServerConfig.BindOptions.ConnectionType, testServerConfig.BindOptions.ConnectionType);
        Assert.Equal(defaultServerConfig.BindOptions.PingTimeout, testServerConfig.BindOptions.PingTimeout);
        Assert.Equal(defaultServerConfig.BindOptions.MinPingTimeout, testServerConfig.BindOptions.MinPingTimeout);
        Assert.Equal(defaultServerConfig.BindOptions.MaxFragmentSize, testServerConfig.BindOptions.MaxFragmentSize);
        Assert.Equal(defaultServerConfig.BindOptions.MaxOutputBuffers, testServerConfig.BindOptions.MaxOutputBuffers);
        Assert.Equal(defaultServerConfig.BindOptions.GuaranteedOutputBuffers, testServerConfig.BindOptions.GuaranteedOutputBuffers);
        Assert.Equal(defaultServerConfig.BindOptions.NumInputBuffers, testServerConfig.BindOptions.NumInputBuffers);
        Assert.Equal(defaultServerConfig.BindOptions.SharedPoolSize, testServerConfig.BindOptions.SharedPoolSize);
        Assert.Equal(defaultServerConfig.BindOptions.SharedPoolLock, testServerConfig.BindOptions.SharedPoolLock);
        Assert.Equal(defaultServerConfig.BindOptions.MajorVersion, testServerConfig.BindOptions.MajorVersion);
        Assert.Equal(defaultServerConfig.BindOptions.MinorVersion, testServerConfig.BindOptions.MinorVersion);
        Assert.Equal(defaultServerConfig.BindOptions.ProtocolType, testServerConfig.BindOptions.ProtocolType);
        Assert.Equal(defaultServerConfig.BindOptions.SysSendBufSize, testServerConfig.BindOptions.SysSendBufSize);
        Assert.Equal(defaultServerConfig.BindOptions.SysRecvBufSize, testServerConfig.BindOptions.SysRecvBufSize);
        Assert.Equal(defaultServerConfig.BindOptions.MinorVersion, testServerConfig.BindOptions.MinorVersion);
        Assert.Equal(defaultServerConfig.BindOptions.UserSpecObject, testServerConfig.BindOptions.UserSpecObject);
        Assert.Equal(defaultServerConfig.BindOptions.TcpOpts.TcpNoDelay, testServerConfig.BindOptions.TcpOpts.TcpNoDelay);

        // The logger will be generated and contain all defaults
        testLoggerConfig = copiedConfig.LoggerConfigMap["DefaultEmaLogger"];

        Assert.Same(copiedConfig.LoggerConfig, copiedConfig.LoggerConfigMap["DefaultEmaLogger"]);
        Assert.Equal("DefaultEmaLogger", testLoggerConfig.Name);
        Assert.Equal(defaultLoggerConfig.LoggerType, testLoggerConfig.LoggerType);
        Assert.Equal(defaultLoggerConfig.LoggerSeverity, testLoggerConfig.LoggerSeverity);
        Assert.Equal(defaultLoggerConfig.FileName, testLoggerConfig.FileName);
        Assert.Equal(defaultLoggerConfig.IncludeDateInLoggerOutput, testLoggerConfig.IncludeDateInLoggerOutput);
        Assert.Equal(defaultLoggerConfig.NumberOfLogFiles, testLoggerConfig.NumberOfLogFiles);
        Assert.Equal(defaultLoggerConfig.MaxLogFileSize, testLoggerConfig.MaxLogFileSize);

        testService = copiedConfig.DirectoryCache.GetService(1);

        Assert.NotNull(testService);

        Assert.Equal("DIRECT_FEED", testService!.Info.ServiceName.ToString());
        Assert.Equal(1, testService!.ServiceId);
        Assert.True(testService!.HasInfo);
        Assert.True(testService!.HasState);
        Assert.True(testService!.State.HasAcceptingRequests);
        Assert.Equal(1, testService!.State.AcceptingRequests);
        Assert.True(testService!.Info.HasSupportQosRange);
        Assert.Equal(1, testService!.Info.SupportsQosRange);
        Assert.Equal(MapEntryActions.ADD, testService!.Action);
        Assert.Equal(FilterEntryActions.SET, testService!.Info.Action);
        Assert.True(testService!.Info.HasVendor);
        Assert.True(string.IsNullOrEmpty(testService!.Info.Vendor.ToString()));
        Assert.True(testService!.Info.HasIsSource);
        Assert.Equal(0, testService!.Info.IsSource);
        Assert.Contains(Ema.Rdm.EmaRdm.MMT_MARKET_PRICE, testService!.Info.CapabilitiesList);
        Assert.Contains(Ema.Rdm.EmaRdm.MMT_MARKET_BY_ORDER, testService!.Info.CapabilitiesList);
        Assert.Contains(Ema.Rdm.EmaRdm.MMT_MARKET_BY_PRICE, testService!.Info.CapabilitiesList);
        Assert.Contains(Ema.Rdm.EmaRdm.MMT_MARKET_MAKER, testService!.Info.CapabilitiesList);
        Assert.Contains(Ema.Rdm.EmaRdm.MMT_DICTIONARY, testService!.Info.CapabilitiesList);
        Assert.True(testService!.Info.HasQos);
        Assert.Single(testService!.Info.QosList);
        Assert.Equal(QosRates.TICK_BY_TICK, testService!.Info.QosList[0].Rate());
        Assert.Equal(QosTimeliness.REALTIME, testService!.Info.QosList[0].Timeliness());
        Assert.True(testService!.Info.HasDictionariesProvided);
        Assert.Equal(2, testService!.Info.DictionariesProvidedList.Count);
        Assert.Contains(DictionaryCallbackClient<IOmmProviderClient>.DICTIONARY_RWFFID, testService!.Info.DictionariesProvidedList);
        Assert.Contains(DictionaryCallbackClient<IOmmProviderClient>.DICTIONARY_RWFENUM, testService!.Info.DictionariesProvidedList);
        Assert.True(testService!.Info.HasDictionariesUsed);
        Assert.Equal(2, testService!.Info.DictionariesUsedList.Count);
        Assert.Contains(DictionaryCallbackClient<IOmmProviderClient>.DICTIONARY_RWFFID, testService!.Info.DictionariesUsedList);
        Assert.Contains(DictionaryCallbackClient<IOmmProviderClient>.DICTIONARY_RWFENUM, testService!.Info.DictionariesUsedList);
        Assert.True(testService!.Info.HasItemList);
        Assert.True(string.IsNullOrEmpty(testService!.Info.ItemList.ToString()));
        Assert.True(testService!.Info.HasAcceptingConsStatus);
        Assert.Equal(1, testService!.Info.AcceptConsumerStatus);
        Assert.True(testService!.Info.HasSupportOOBSnapshots);
        Assert.Equal(0, testService!.Info.SupportsOOBSnapshots);
        Assert.Equal(FilterEntryActions.SET, testService!.State.Action);
        Assert.Equal(1, testService!.State.ServiceStateVal);

        Assert.Equal(Directory.ServiceFilterFlags.INFO | Directory.ServiceFilterFlags.STATE, copiedConfig.DirectoryCache.DirectoryRefresh.Filter);

        // Get the directoryconfig for DefaultEmaDirectory
        testDirectoryConfig = copiedConfig.DirectoryConfigMap["DefaultEmaDirectory"];

        Assert.Single(testDirectoryConfig.ServiceMap);

        testServiceConfig = testDirectoryConfig.ServiceMap["DIRECT_FEED"];

        // This should be the same object as above.
        Assert.Same(testService, testServiceConfig.Service);
        Assert.Contains("DefaultEmaDictionary", testServiceConfig.DictionariesUsedList);
        Assert.Contains("DefaultEmaDictionary", testServiceConfig.DictionariesProvidedList);

        testDictionaryConfig = copiedConfig.DictionaryConfigMap["DefaultEmaDictionary"];

        Assert.Equal("./enumtype.def", testDictionaryConfig.EnumTypeDefFileName);
        Assert.Equal("RWFEnum", testDictionaryConfig.EnumTypeDefItemName);
        Assert.Equal("./RDMFieldDictionary", testDictionaryConfig.RdmFieldDictionaryFileName);
        Assert.Equal("RWFFld", testDictionaryConfig.RdmFieldDictionaryItemName);

        copiedConfig.VerifyConfiguration();
    }

    // This test will load different failure scenarios for the verification method.
    // The positive test cases were already covered in the XmlConfigTest
    [Fact]
    public void IProviderVerificationFailureTest()
    {
        OmmIProviderConfig iProvConfig;
        IProviderConfig tmpIProvConfig_1;

        ServerConfig serverConfig_1;
        ServerConfig serverConfig_2;

        LoggerConfig tmpLoggerConfig_1;
        LoggerConfig tmpLoggerConfig_2;

        DictionaryConfig tmpDictionaryConfig_1;
        DictionaryConfig tmpDictionaryConfig_2;

        DirectoryConfig tmpDirectoryConfig_1;
        DirectoryConfig tmpDirectoryConfig_2;

        EmaServiceConfig tmpService_1;
        EmaServiceConfig tmpService_2;


        // Load a blank config so we can be sure that everything added is from the programmtic config
        // This doesn't use the default behavior(loading EmaConfig.xml) because an EmaConfig.xml file may be present in the directory for other example apps
        iProvConfig = new OmmIProviderConfig("../../../OmmConfigTests/EmaBlankConfig.xml");

        OmmIProviderConfigImpl iProvConfigImpl = iProvConfig.OmmIProvConfigImpl;

        // Loaded the Config, now make sure everything's in it.

        // There should be nothing in here
        Assert.Empty(iProvConfigImpl.IProviderConfigMap);
        Assert.Empty(iProvConfigImpl.ServerConfigMap);
        Assert.Empty(iProvConfigImpl.LoggerConfigMap);
        Assert.Empty(iProvConfigImpl.DictionaryConfigMap);

        // Verify should succeed
        iProvConfigImpl.VerifyConfiguration();

        // Set a non-existant consumer name
        iProvConfigImpl.IProviderName = "bad_iprov";

        // This should fail.
        Assert.Throws<OmmInvalidConfigurationException>(() => iProvConfigImpl.VerifyConfiguration());

        // Unset the ConsumerName and test the DefaultConsumer string
        iProvConfigImpl.IProviderName = string.Empty;
        iProvConfigImpl.DefaultIProvider = "bad_niprov";

        // This should fail.
        Assert.Throws<OmmInvalidConfigurationException>(() => iProvConfigImpl.VerifyConfiguration());

        iProvConfigImpl.DefaultIProvider = string.Empty;

        tmpIProvConfig_1 = new IProviderConfig();

        tmpIProvConfig_1.Name = "iProv_1";
        tmpIProvConfig_1.Server = "badServer_1";

        iProvConfigImpl.IProviderConfigMap.Add(tmpIProvConfig_1.Name, tmpIProvConfig_1);

        // This should fail.
        Assert.Throws<OmmInvalidConfigurationException>(() => iProvConfigImpl.VerifyConfiguration());

        // Add a couple of servers, make sure it still fails
        serverConfig_1 = new ServerConfig();
        serverConfig_1.Name = "ConfigServer_1";
        iProvConfigImpl.ServerConfigMap.Add(serverConfig_1.Name, serverConfig_1);

        serverConfig_2 = new ServerConfig();
        serverConfig_2.Name = "ConfigServer_2";
        iProvConfigImpl.ServerConfigMap.Add(serverConfig_2.Name, serverConfig_2);

        // This should fail.
        Assert.Throws<OmmInvalidConfigurationException>(() => iProvConfigImpl.VerifyConfiguration());

        // Set the server to a valid value.
        tmpIProvConfig_1.Server = "ConfigServer_2";

        iProvConfigImpl.VerifyConfiguration();

        // Add a logger mismatch
        tmpIProvConfig_1.Logger = "bad_logger";

        // This should fail.
        Assert.Throws<OmmInvalidConfigurationException>(() => iProvConfigImpl.VerifyConfiguration());

        // Add a couple of loggers and make sure it's still a mismatch

        tmpLoggerConfig_1 = new LoggerConfig();
        tmpLoggerConfig_1.Name = "ConfigLogger_1";
        iProvConfigImpl.LoggerConfigMap.Add(tmpLoggerConfig_1.Name, tmpLoggerConfig_1);

        tmpLoggerConfig_2 = new LoggerConfig();
        tmpLoggerConfig_2.Name = "ConfigLogger_2";
        iProvConfigImpl.LoggerConfigMap.Add(tmpLoggerConfig_2.Name, tmpLoggerConfig_2);

        tmpIProvConfig_1.Logger = "bad_logger";

        // This should fail.
        Assert.Throws<OmmInvalidConfigurationException>(() => iProvConfigImpl.VerifyConfiguration());

        // Set the logger correctly and make sure it verifies
        tmpIProvConfig_1.Logger = "ConfigLogger_2";

        iProvConfigImpl.VerifyConfiguration();

        // Add in the dictionaries and make sure it still works - these are all going by the default directory.
        tmpDictionaryConfig_1 = new DictionaryConfig();
        tmpDictionaryConfig_1.Name = "ConfigDictionary_1";
        iProvConfigImpl.DictionaryConfigMap.Add(tmpDictionaryConfig_1.Name, tmpDictionaryConfig_1);

        tmpDictionaryConfig_2 = new DictionaryConfig();
        tmpDictionaryConfig_2.Name = "ConfigDictionary_2";
        iProvConfigImpl.DictionaryConfigMap.Add(tmpDictionaryConfig_2.Name, tmpDictionaryConfig_2);

        iProvConfigImpl.VerifyConfiguration();

        // Setup the services and directory configs.
        tmpService_1 = new EmaServiceConfig(true, false);
        tmpService_1.Service.Info.ServiceName.Data("ConfigService_1");
        tmpService_1.Service.ServiceId = 5;

        tmpService_2 = new EmaServiceConfig(true, false);
        tmpService_2.Service.Info.ServiceName.Data("ConfigService_2");
        tmpService_1.Service.ServiceId = 20;

        tmpDirectoryConfig_1 = new DirectoryConfig();
        tmpDirectoryConfig_1.Name = "ConfigDirectory_1";
        tmpDirectoryConfig_1.ServiceMap.Add(tmpService_1.Service.Info.ServiceName.ToString(), tmpService_1);
        iProvConfigImpl.DirectoryConfigMap.Add(tmpDirectoryConfig_1.Name, tmpDirectoryConfig_1);

        tmpDirectoryConfig_2 = new DirectoryConfig();
        tmpDirectoryConfig_2.Name = "ConfigDirectory_2";
        tmpDirectoryConfig_2.ServiceMap.Add(tmpService_1.Service.Info.ServiceName.ToString(), tmpService_1);
        tmpDirectoryConfig_2.ServiceMap.Add(tmpService_2.Service.Info.ServiceName.ToString(), tmpService_2);
        iProvConfigImpl.DirectoryConfigMap.Add(tmpDirectoryConfig_2.Name, tmpDirectoryConfig_2);


        // This should succeed
        iProvConfigImpl.VerifyConfiguration();

        // Set the default directory name to invalid
        iProvConfigImpl.DefaultDirectory = "badDirectory_1";

        // This should fail.
        Assert.Throws<OmmInvalidConfigurationException>(() => iProvConfigImpl.VerifyConfiguration());

        // re-set the default directory name, and set an invalid directory in the main NiProviderConfig
        iProvConfigImpl.DefaultDirectory = string.Empty;
        tmpIProvConfig_1.Directory = "bad_directory_1";

        // This should fail.
        Assert.Throws<OmmInvalidConfigurationException>(() => iProvConfigImpl.VerifyConfiguration());

        // Set the niProv directory to a correct one.
        iProvConfigImpl.DefaultDirectory = string.Empty;
        tmpIProvConfig_1.Directory = string.Empty;

        // Add in a bad dictionary name to service_2's used list
        tmpService_2.DictionariesUsedList.Clear();
        tmpService_2.DictionariesUsedList.Add("badDictionary");
        tmpService_2.DictionariesUsedList.Add("ConfigDictionary_1");
        tmpService_2.DictionariesUsedList.Add("ConfigDictionary_2");
        tmpService_2.DictionariesProvidedList.Clear();
        tmpService_2.DictionariesProvidedList.Add("ConfigDictionary_1");
        tmpService_2.DictionariesProvidedList.Add("ConfigDictionary_2");

        // This should fail.
        Assert.Throws<OmmInvalidConfigurationException>(() => iProvConfigImpl.VerifyConfiguration());

        // Add in a bad dictionary name to service_2's used list
        tmpService_2.DictionariesUsedList.Clear();
        tmpService_2.DictionariesUsedList.Add("ConfigDictionary_1");
        tmpService_2.DictionariesUsedList.Add("badDictionary");
        tmpService_2.DictionariesUsedList.Add("ConfigDictionary_2");
        tmpService_2.DictionariesProvidedList.Clear();
        tmpService_2.DictionariesProvidedList.Add("ConfigDictionary_1");
        tmpService_2.DictionariesProvidedList.Add("ConfigDictionary_2");

        // This should fail.
        Assert.Throws<OmmInvalidConfigurationException>(() => iProvConfigImpl.VerifyConfiguration());

        // Add in a bad dictionary name to service_2's used list
        tmpService_2.DictionariesUsedList.Clear();
        tmpService_2.DictionariesUsedList.Add("ConfigDictionary_1");
        tmpService_2.DictionariesUsedList.Add("ConfigDictionary_2");
        tmpService_2.DictionariesUsedList.Add("badDictionary");
        tmpService_2.DictionariesProvidedList.Clear();
        tmpService_2.DictionariesProvidedList.Add("ConfigDictionary_1");
        tmpService_2.DictionariesProvidedList.Add("ConfigDictionary_2");

        // This should fail.
        Assert.Throws<OmmInvalidConfigurationException>(() => iProvConfigImpl.VerifyConfiguration());

        // Add in a bad dictionary name to service_2's provided list
        tmpService_2.DictionariesUsedList.Clear();
        tmpService_2.DictionariesUsedList.Add("ConfigDictionary_1");
        tmpService_2.DictionariesUsedList.Add("ConfigDictionary_2");
        tmpService_2.DictionariesProvidedList.Clear();
        tmpService_2.DictionariesProvidedList.Add("badDictionary");
        tmpService_2.DictionariesProvidedList.Add("ConfigDictionary_1");
        tmpService_2.DictionariesProvidedList.Add("ConfigDictionary_2");

        // This should fail.
        Assert.Throws<OmmInvalidConfigurationException>(() => iProvConfigImpl.VerifyConfiguration());

        // Add in a bad dictionary name to service_2's provided list
        tmpService_2.DictionariesUsedList.Clear();
        tmpService_2.DictionariesUsedList.Add("ConfigDictionary_1");
        tmpService_2.DictionariesUsedList.Add("ConfigDictionary_2");
        tmpService_2.DictionariesProvidedList.Clear();
        tmpService_2.DictionariesProvidedList.Add("ConfigDictionary_1");
        tmpService_2.DictionariesProvidedList.Add("badDictionary");
        tmpService_2.DictionariesProvidedList.Add("ConfigDictionary_2");

        // This should fail.
        Assert.Throws<OmmInvalidConfigurationException>(() => iProvConfigImpl.VerifyConfiguration());

        // Add in a bad dictionary name to service_2's provided list
        tmpService_2.DictionariesUsedList.Clear();
        tmpService_2.DictionariesUsedList.Add("ConfigDictionary_1");
        tmpService_2.DictionariesUsedList.Add("ConfigDictionary_2");
        tmpService_2.DictionariesProvidedList.Clear();
        tmpService_2.DictionariesProvidedList.Add("ConfigDictionary_1");
        tmpService_2.DictionariesProvidedList.Add("ConfigDictionary_2");
        tmpService_2.DictionariesProvidedList.Add("badDictionary");

        // This should fail.
        Assert.Throws<OmmInvalidConfigurationException>(() => iProvConfigImpl.VerifyConfiguration());

        // set service_2's dictionaries to correct.
        tmpService_2.DictionariesUsedList.Clear();
        tmpService_2.DictionariesUsedList.Add("ConfigDictionary_1");
        tmpService_2.DictionariesUsedList.Add("ConfigDictionary_2");
        tmpService_2.DictionariesProvidedList.Clear();
        tmpService_2.DictionariesProvidedList.Add("ConfigDictionary_1");
        tmpService_2.DictionariesProvidedList.Add("ConfigDictionary_2");

        // This should succeed
        iProvConfigImpl.VerifyConfiguration();

        // Set both services to the same service Id
        tmpService_1.Service.ServiceId = 10;
        tmpService_2.Service.ServiceId = 10;

        // This should fail.
        Assert.Throws<OmmInvalidConfigurationException>(() => iProvConfigImpl.VerifyConfiguration());

    }
}

