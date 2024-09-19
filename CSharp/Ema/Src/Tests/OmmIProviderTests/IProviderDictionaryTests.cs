/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2024 LSEG. All rights reserved.     
 *|-----------------------------------------------------------------------------
 */

using System;
using System.IO;

using System.Threading;
using Xunit.Abstractions;

using LSEG.Eta.Codec;
using LSEG.Eta.ValueAdd.Rdm;
using LSEG.Eta.ValueAdd.Reactor;

using LSEG.Ema.Rdm;

using LSEG.Ema.Access.Tests.OmmConsumerTests;
using System.ComponentModel;
using LSEG.Eta.Rdm;
using LSEG.Ema.Access.Tests.OmmNiProviderTests;

namespace LSEG.Ema.Access.Tests.OmmIProviderTests;

/// <summary>
/// Tests for <see cref="Ema.Access.DictionaryHandler"/> of the EMA Interactive Provider.
/// </summary>
public class IProviderDictionaryTests
{
    ITestOutputHelper m_Output;

    // Test cases within a class are expected to be executed sequentially and therefore
    // can use the same port number, which is different from the port number used by test
    // cases in other classes
    private const string PROVIDER_PORT = "19102";

    public IProviderDictionaryTests(ITestOutputHelper output)
    {
        m_Output = output;
    }

    /// <summary>
    /// Verify that the initialization with the Dictionary domain requests for both cases:
    /// with the API controlled and the USER controlled options.
    /// </summary>
    [Theory]
    [Category("DictionaryHandler")]
    [InlineData(OmmIProviderConfig.AdminControlMode.USER_CONTROL)]
    [InlineData(OmmIProviderConfig.AdminControlMode.API_CONTROL)]
    public void InitializeAndUninitialize_Test(OmmIProviderConfig.AdminControlMode dictionaryOperationModelMode)
    {
        // Test case setup
        OmmIProviderConfig config = (dictionaryOperationModelMode == OmmIProviderConfig.AdminControlMode.API_CONTROL)
            ? new OmmIProviderConfig("../../../OmmIProviderTests/EmaConfigTest.xml")
            : new OmmIProviderConfig();

        OmmProvider? provider = null;
        OmmProviderItemClientTest providerClient = new();

        SimpleConsumerTest simpleConsumer = new SimpleConsumerTest(m_Output, PROVIDER_PORT);

        ulong serviceId = 1;
        string serviceName = "DIRECT_FEED";

        int dictionaryRequestsCount = 0;

        var dataDictionary = new Rdm.DataDictionary();

        dataDictionary.LoadFieldDictionary(DataDictionaryTests.FIELD_DICTIONARY_FILENAME);
        dataDictionary.LoadEnumTypeDictionary(DataDictionaryTests.ENUM_TABLE_FILENAME);

        // While Source Directory Handler is missing, provide a USER-generated response with dictionaries
        providerClient.ReqMsgHandler = (requestMsg, providerEvent) =>
        {
            switch (requestMsg.DomainType())
            {
                case EmaRdm.MMT_DIRECTORY:
                    SubmitDirectoryResponse(requestMsg, providerEvent, provider!, serviceId, serviceName);
                    break;

                case EmaRdm.MMT_DICTIONARY:
                    {
                        // Application receives Dictionary callback only when Dictionary operation
                        // is under USER_CONTROL
                        Assert.Equal(OmmIProviderConfig.AdminControlMode.USER_CONTROL, dictionaryOperationModelMode);

                        // Count requests, check for Enum and Field dictionary requests advertised
                        // above
                        ++dictionaryRequestsCount;

                        SubmitDictionaryResponse(requestMsg, providerEvent, dataDictionary);
                    }
                    break;

                case EmaRdm.MMT_LOGIN:
                    provider!.Submit(new RefreshMsg().StreamId(requestMsg.StreamId())
                        .DomainType(requestMsg.DomainType())
                        .Solicited(true), providerEvent.Handle);
                    break;

                default:
                    provider!.Submit(new RefreshMsg().StreamId(requestMsg.StreamId())
                        .DomainType(requestMsg.DomainType())
                        .Solicited(true), providerEvent.Handle);
                    break;
            }
        };

        // Test case execution
        try
        {
            // Initialize both Provider and the Consumer
            provider = new OmmProvider(config.Port(PROVIDER_PORT)
                .AdminControlDirectory(dictionaryOperationModelMode)
                .AdminControlDictionary(dictionaryOperationModelMode),
                providerClient);

            Assert.Contains("DefaultEmaIProvider", provider.ProviderName);

            InitializeSimpleConsumer(provider, providerClient, simpleConsumer);

            // The user-supplied providerClient handles dictionary requests only when the
            // USER_CONTROL mode is enabled
            if (OmmIProviderConfig.AdminControlMode.USER_CONTROL == dictionaryOperationModelMode)
                Assert.Equal(2, dictionaryRequestsCount);
        }
        finally
        {
            // Teardown

            simpleConsumer.UnInitialize();
            provider?.Uninitialize();
        }
    }

    /// <summary>
    /// case <see cref="DictionaryMsgType.REQUEST"/>
    /// </summary>
    /// <param name="dictionaryOperationModelMode"></param>
    [Theory]
    [InlineData(OmmIProviderConfig.AdminControlMode.USER_CONTROL)]
    [InlineData(OmmIProviderConfig.AdminControlMode.API_CONTROL)]
    public void SubmitDictionaryRequest_Test(OmmIProviderConfig.AdminControlMode dictionaryOperationModelMode)
    {
        // Test case setup
        OmmIProviderConfig config = (dictionaryOperationModelMode == OmmIProviderConfig.AdminControlMode.API_CONTROL)
            ? new OmmIProviderConfig("../../../OmmIProviderTests/EmaConfigTest.xml")
            : new OmmIProviderConfig();

        OmmProvider? provider = null;
        OmmProviderItemClientTest providerClient = new();

        SimpleConsumerTest simpleConsumer = new SimpleConsumerTest(m_Output, PROVIDER_PORT);

        ulong serviceId = 1;
        string serviceName = "DIRECT_FEED";

        int dictionaryRequestsCount = 0;

        var dataDictionary = new Rdm.DataDictionary();

        dataDictionary.LoadFieldDictionary(DataDictionaryTests.FIELD_DICTIONARY_FILENAME);
        dataDictionary.LoadEnumTypeDictionary(DataDictionaryTests.ENUM_TABLE_FILENAME);

        // While Source Directory Handler is missing, provide a USER-generated response with dictionaries
        providerClient.ReqMsgHandler = (requestMsg, providerEvent) =>
        {
            switch (requestMsg.DomainType())
            {
                case EmaRdm.MMT_DIRECTORY:
                    SubmitDirectoryResponse(requestMsg, providerEvent, provider!, serviceId, serviceName);
                    break;

                case EmaRdm.MMT_DICTIONARY:
                    // Application receives Dictionary callback only when Dictionary operation
                    // is under USER_CONTROL
                    Assert.Equal(OmmIProviderConfig.AdminControlMode.USER_CONTROL, dictionaryOperationModelMode);

                    // Count requests, check for Enum and Field dictionary requests advertised
                    // above
                    ++dictionaryRequestsCount;

                    SubmitDictionaryResponse(requestMsg, providerEvent, dataDictionary);
                    break;

                default:
                    provider!.Submit(new RefreshMsg().StreamId(requestMsg.StreamId())
                        .DomainType(requestMsg.DomainType())
                        .Solicited(true), providerEvent.Handle);
                    break;
            }
        };

        try
        {
            provider = new OmmProvider(config.Port(PROVIDER_PORT)
                .AdminControlDirectory(dictionaryOperationModelMode)
                .AdminControlDictionary(dictionaryOperationModelMode),
                providerClient);

            Assert.Contains("DefaultEmaIProvider", provider.ProviderName);

            TestReactorEvent reactorEvent;
            InitializeSimpleConsumer(provider, providerClient, simpleConsumer);

            {
                // Submit Dictionary request with invalid dictionary name, expect rejection message
                simpleConsumer.SubmitDictionaryRequest(streamId: simpleConsumer.DictionaryStreamId,
                    serviceId: simpleConsumer.DictionaryServiceId, serviceName, "DoesNotExist");

                Thread.Sleep(1000);

                Assert.Single(simpleConsumer.EventQueue);

                reactorEvent = simpleConsumer.EventQueue.Dequeue();
                Assert.Equal(TestReactorEventType.DICTIONARY_MSG, reactorEvent.EventType);
                RDMDictionaryMsgEvent dictEvent = (RDMDictionaryMsgEvent)reactorEvent.ReactorEvent;
                Assert.NotNull(dictEvent.DictionaryMsg);
                Assert.Equal(DictionaryMsgType.STATUS, dictEvent.DictionaryMsg!.DictionaryMsgType);

                DictionaryStatus dictStatus = dictEvent.DictionaryMsg!.DictionaryStatus!;
                Assert.Equal(DataStates.SUSPECT, dictStatus.State.DataState());
                Assert.Equal(StateCodes.ERROR, dictStatus.State.Code());
                Assert.Equal(StreamStates.CLOSED_RECOVER, dictStatus.State.StreamState());
                Assert.Contains("Dictionary request message rejected - the reqesting dictionary name",
                    dictStatus.State.Text().ToString());
            }

            {
                // Submit request with the valid dictionary name but for the invalid serviceId,
                // expect rejection message as both dictionary name and the ServiceId must match
                simpleConsumer.SubmitDictionaryRequest(streamId: simpleConsumer.DictionaryStreamId,
                    serviceId: (int)simpleConsumer.DictionaryServiceId + 100, serviceName, "RWFFld");

                Thread.Sleep(1000);

                Assert.Single(simpleConsumer.EventQueue);

                reactorEvent = simpleConsumer.EventQueue.Dequeue();
                Assert.Equal(TestReactorEventType.DICTIONARY_MSG, reactorEvent.EventType);
                RDMDictionaryMsgEvent dictEvent = (RDMDictionaryMsgEvent)reactorEvent.ReactorEvent;
                Assert.NotNull(dictEvent.DictionaryMsg);
                Assert.Equal(DictionaryMsgType.STATUS, dictEvent.DictionaryMsg!.DictionaryMsgType);

                DictionaryStatus dictStatus = dictEvent.DictionaryMsg!.DictionaryStatus!;
                Assert.Equal(DataStates.SUSPECT, dictStatus.State.DataState());
                Assert.Equal(StateCodes.ERROR, dictStatus.State.Code());
                Assert.Equal(StreamStates.CLOSED_RECOVER, dictStatus.State.StreamState());

                // note: handling of invalid streamId differs between two modes
                if (dictionaryOperationModelMode == OmmIProviderConfig.AdminControlMode.USER_CONTROL)
                {
                    Assert.Contains("Dictionary request message rejected - the service Id",
                        dictStatus.State.Text().ToString());
                }
                else
                {
                    Assert.Contains("Dictionary request message rejected - the reqesting dictionary name",
                        dictStatus.State.Text().ToString());
                }
            }

            {
                // Submit requests with the valid dictionary name and valid serviceId,
                // expect refresh message in response as both dictionary name and the ServiceId match
                simpleConsumer.DataDictionary.Clear();

                simpleConsumer.SubmitDictionaryRequest(streamId: simpleConsumer.DictionaryStreamId,
                    serviceId: (int)simpleConsumer.DictionaryServiceId, serviceName, "RWFFld");

                Thread.Sleep(1000);

                Assert.Empty(simpleConsumer.EventQueue);

                simpleConsumer.SubmitDictionaryRequest(streamId: simpleConsumer.DictionaryStreamId,
                    serviceId: (int)simpleConsumer.DictionaryServiceId, serviceName, "RWFEnum");

                Thread.Sleep(1000);

                Assert.Empty(simpleConsumer.EventQueue);
            }
        }
        finally
        {
            simpleConsumer.UnInitialize();
            provider?.Uninitialize();
        }
    }

    /// <summary>
    /// case DictionaryMsgType.CLOSE:
    /// </summary>
    /// <param name="dictionaryOperationModelMode"></param>
    [Theory]
    [InlineData(OmmIProviderConfig.AdminControlMode.USER_CONTROL)]
    [InlineData(OmmIProviderConfig.AdminControlMode.API_CONTROL)]
    public void SubmitDictionaryClose_Test(OmmIProviderConfig.AdminControlMode dictionaryOperationModelMode)
    {
        // Test case setup
        OmmIProviderConfig config = (dictionaryOperationModelMode == OmmIProviderConfig.AdminControlMode.API_CONTROL)
            ? new OmmIProviderConfig("../../../OmmIProviderTests/EmaConfigTest.xml")
            : new OmmIProviderConfig();

        OmmProvider? provider = null;
        OmmProviderItemClientTest providerClient = new();

        SimpleConsumerTest simpleConsumer = new SimpleConsumerTest(m_Output, PROVIDER_PORT);

        ulong serviceId = 1;
        string serviceName = "DIRECT_FEED";

        var dataDictionary = new Rdm.DataDictionary();

        dataDictionary.LoadFieldDictionary(DataDictionaryTests.FIELD_DICTIONARY_FILENAME);
        dataDictionary.LoadEnumTypeDictionary(DataDictionaryTests.ENUM_TABLE_FILENAME);

        // While Source Directory Handler is missing, provide a USER-generated response with dictionaries
        providerClient.ReqMsgHandler = (requestMsg, providerEvent) =>
        {
            switch (requestMsg.DomainType())
            {
                case EmaRdm.MMT_DIRECTORY:
                    SubmitDirectoryResponse(requestMsg, providerEvent, provider!, serviceId, serviceName);
                    break;

                case EmaRdm.MMT_DICTIONARY:
                    // Application receives Dictionary callback only when Dictionary operation
                    // is under USER_CONTROL
                    Assert.Equal(OmmIProviderConfig.AdminControlMode.USER_CONTROL, dictionaryOperationModelMode);

                    SubmitDictionaryResponse(requestMsg, providerEvent, dataDictionary);
                    break;

                default:
                    provider!.Submit(new RefreshMsg().StreamId(requestMsg.StreamId())
                        .DomainType(requestMsg.DomainType())
                        .Solicited(true), providerEvent.Handle);
                    break;
            }
        };

        try
        {
            m_Output.WriteLine("Creating Provider");

            provider = new OmmProvider(config.Port(PROVIDER_PORT)
                .AdminControlDirectory(dictionaryOperationModelMode)
                .AdminControlDictionary(dictionaryOperationModelMode),
                providerClient);

            Assert.Contains("DefaultEmaIProvider", provider.ProviderName);

            m_Output.WriteLine("Initializing Consumer");

            InitializeSimpleConsumer(provider, providerClient, simpleConsumer);

            m_Output.WriteLine("Initialized Consumer");

            {
                // Consumer's Reactor closes Dictionary streams after receiving them
                simpleConsumer.SubmitDictionaryClose(simpleConsumer.DictionaryStreamId);

                Thread.Sleep(1000);

                if (dictionaryOperationModelMode == OmmIProviderConfig.AdminControlMode.API_CONTROL)
                    // all CLOSE messages are handled within API, application doesn't get any callbacks
                    Assert.Equal(0, providerClient.ReceivedOnClose);
                else
                    // application gets two CLOSE callbacks invoked for both dictionaries
                    Assert.Equal(2, providerClient.ReceivedOnClose);
            }
        }
        finally
        {
            simpleConsumer.UnInitialize();
            provider?.Uninitialize();
        }
    }


    /// <summary>
    /// Case <see cref="DictionaryMsgType.REFRESH"/>
    /// </summary>
    ///
    /// <remarks>
    /// <para>
    /// OMM Provider can receive Dictionary REFRESH messages from ADH when it requests a
    /// Dictionary from that node, but not from the Consumer.</para>
    ///
    /// <para>
    /// Therefore DictionaryHandler is not expecting a Dictionary REFRESH message from the
    /// Consumer.</para>
    ///
    /// <para>
    /// This test case sends a Dictionary REFRESH message from the Consumer to Provider
    /// and verifies that this message is correctly discarded by monitoring Logger output
    /// to Console</para>
    ///
    /// </remarks>
    ///
    /// <param name="dictionaryOperationModelMode"></param>
    ///
    /// <seealso cref="ItemCallbackClient{T}.ProcessIProviderMsgCallback(ReactorMsgEvent, Eta.Codec.DataDictionary?)"/>
    ///
    [Theory]
    [InlineData(OmmIProviderConfig.AdminControlMode.USER_CONTROL)]
    [InlineData(OmmIProviderConfig.AdminControlMode.API_CONTROL)]
    public void SubmitDictionaryRefresh_Test(OmmIProviderConfig.AdminControlMode dictionaryOperationModelMode)
    {
        var enumsDictionary = new Eta.Codec.DataDictionary();
        enumsDictionary.LoadEnumTypeDictionary(DataDictionaryTests.ENUM_TABLE_FILENAME, out _);

        int invalidStreamId = 151;

        // Refresh message is not expected to be received from the Consumer. Check for the
        // Error message to be logged from ItemCallbackClient
        // ProcessIProviderMsgCallback(ReactorMsgEvent, DataDictionary?)  method
        InvalidMessageSubmitHelper(dictionaryOperationModelMode,
            (SimpleConsumerTest simpleConsumer) => simpleConsumer.SubmitDictionaryRefresh(invalidStreamId,
                serviceId: simpleConsumer.DictionaryServiceId, "DoesNotExist", enumsDictionary),
            $"Received an item event without a matching stream Id {invalidStreamId}");
    }

    /// <summary>
    /// case <see cref="DictionaryMsgType.STATUS"/>
    /// </summary>
    /// <remarks>
    /// Similar to Dictionary REFRESH message, Provider does not expect this message from
    /// the Consumer. And as with the case of REFRESH message, the Provider is expected to
    /// produce an error log message, which is intercepted into the memory buffer and
    /// examined in the end of the test case.
    /// </remarks>
    /// <param name="dictionaryOperationModelMode"></param>
    /// <seealso cref="SubmitDictionaryRefresh_Test(OmmIProviderConfig.AdminControlMode)"/>
    [Theory]
    [InlineData(OmmIProviderConfig.AdminControlMode.USER_CONTROL)]
    [InlineData(OmmIProviderConfig.AdminControlMode.API_CONTROL)]
    public void SubmitDictionaryStatus_Test(OmmIProviderConfig.AdminControlMode dictionaryOperationModelMode)
    {
        // Status message is not expected to be received from the Consumer. Check for the
        // Error message to be logged from ItemCallbackClient
        // ProcessIProviderMsgCallback(ReactorMsgEvent, DataDictionary?)  method
        InvalidMessageSubmitHelper(dictionaryOperationModelMode,
            (SimpleConsumerTest simpleConsumer) => simpleConsumer.SubmitDictionaryStatus(simpleConsumer.DictionaryStreamId),
            $"Received an item event without a matching stream Id");
    }

    // Consumer application sends a generic message with non-existence
    // stream Id. The dictionary handler sends a reject status message.
    [Theory]
    [InlineData(OmmIProviderConfig.AdminControlMode.USER_CONTROL)]
    [InlineData(OmmIProviderConfig.AdminControlMode.API_CONTROL)]
    public void SubmitDictionaryGeneric_Test(OmmIProviderConfig.AdminControlMode dictionaryOperationModelMode)
    {
        InvalidMessageSubmitHelper(dictionaryOperationModelMode,
            (SimpleConsumerTest simpleConsumer) => simpleConsumer.SubmitDictionaryStatus(simpleConsumer.DictionaryStreamId),
            $"Received an item event without a matching stream Id");
    }

    /// Consumer sends a POST message with non-existing stream Id. The dictionary handler
    /// sends a reject status message because POST message is an unsupported/unhandled
    /// Dictionary message type, but only when this message is the first Dictionary message
    /// received by Provider's Reactor.
    [Theory]
    [InlineData(OmmIProviderConfig.AdminControlMode.USER_CONTROL)]
    [InlineData(OmmIProviderConfig.AdminControlMode.API_CONTROL)]
    public void SubmitDictionaryPost_Test(OmmIProviderConfig.AdminControlMode dictionaryOperationModelMode)
    {
        // Intercept standard output (Console) to the memory buffer to examine Logger
        // messages (similar to ConsumerLoggerTest) for expected error reports
        MemoryStream memoryStream = new(12 * 1024);
        StreamWriter streamWriter = new(memoryStream);
        Console.SetOut(streamWriter);

        // Test case setup
        OmmIProviderConfig config = (dictionaryOperationModelMode == OmmIProviderConfig.AdminControlMode.API_CONTROL)
            ? new OmmIProviderConfig("../../../OmmIProviderTests/EmaConfigTest.xml")
            : new OmmIProviderConfig();

        ConfigureConsoleLoggerOutput(config);

        OmmProvider? provider = null;
        OmmProviderItemClientTest providerClient = new();

        SimpleConsumerTest simpleConsumer = new SimpleConsumerTest(m_Output, PROVIDER_PORT);

        ulong serviceId = 1;
        string serviceName = "DIRECT_FEED";

        int dictionaryRequestsCount = 0;

        var dataDictionary = new Rdm.DataDictionary();

        dataDictionary.LoadFieldDictionary(DataDictionaryTests.FIELD_DICTIONARY_FILENAME);
        dataDictionary.LoadEnumTypeDictionary(DataDictionaryTests.ENUM_TABLE_FILENAME);

        // While Source Directory Handler is missing, provide a USER-generated response with dictionaries
        providerClient.ReqMsgHandler = (requestMsg, providerEvent) =>
        {
            switch (requestMsg.DomainType())
            {
                case EmaRdm.MMT_DIRECTORY:
                    SubmitDirectoryResponse(requestMsg, providerEvent, provider!, serviceId, serviceName);
                    break;

                case EmaRdm.MMT_DICTIONARY:
                    {
                        // Application receives Dictionary callback only when Dictionary operation
                        // is under USER_CONTROL
                        Assert.Equal(OmmIProviderConfig.AdminControlMode.USER_CONTROL, dictionaryOperationModelMode);

                        // Count requests, check for Enum and Field dictionary requests advertised
                        // above
                        ++dictionaryRequestsCount;

                        SubmitDictionaryResponse(requestMsg, providerEvent, dataDictionary);
                    }
                    break;

                case EmaRdm.MMT_LOGIN:
                    provider!.Submit(new RefreshMsg().StreamId(requestMsg.StreamId())
                        .DomainType(requestMsg.DomainType())
                        .Solicited(true), providerEvent.Handle);
                    break;

                default:
                    provider!.Submit(new RefreshMsg().StreamId(requestMsg.StreamId())
                        .DomainType(requestMsg.DomainType())
                        .Solicited(true), providerEvent.Handle);
                    break;
            }
        };

        // Test case execution
        try
        {
            // Initialize both Provider and the Consumer
            provider = new OmmProvider(config.Port(PROVIDER_PORT)
                .AdminControlDirectory(dictionaryOperationModelMode)
                .AdminControlDictionary(dictionaryOperationModelMode),
                providerClient);

            Assert.Contains("DefaultEmaIProvider", provider.ProviderName);

            InitializeSimpleConsumer(provider, providerClient, simpleConsumer, initDictionaryRequest: false);

            int invalidStreamId = 191;
            {
                simpleConsumer.SubmitPostMsgOnStream(
                    invalidStreamId,
                    simpleConsumer.DictionaryServiceId,
                    "ITEM.NAME",
                    (int)DomainType.DICTIONARY,
                    123);
            }

            Thread.Sleep(1000);

            string logOutput = System.Text.Encoding.ASCII.GetString(memoryStream.GetBuffer(), 0,
                (int)memoryStream.Length);

            Assert.NotNull(logOutput);
            Assert.NotEmpty(logOutput);

            Assert.Contains("Rejected unhandled dictionary message type ", logOutput);
            Assert.Contains($"Stream Id {invalidStreamId}", logOutput);

            // examine dictionary reject message
            Assert.Single(simpleConsumer.EventQueue);

            TestReactorEvent reactorEvent = simpleConsumer.EventQueue.Dequeue();
            Assert.Equal(TestReactorEventType.DICTIONARY_MSG, reactorEvent.EventType);
            RDMDictionaryMsgEvent dictEvent = (RDMDictionaryMsgEvent)reactorEvent.ReactorEvent;

            Assert.NotNull(dictEvent.DictionaryMsg);
            Assert.Equal(DictionaryMsgType.STATUS, dictEvent.DictionaryMsg!.DictionaryMsgType);
            Assert.Equal(invalidStreamId, dictEvent.DictionaryMsg!.StreamId);
            DictionaryStatus dictionaryStatus = dictEvent.DictionaryMsg!.DictionaryStatus!;
            Assert.Contains("Dictionary message rejected - unhandled dictionary message type.",
                dictionaryStatus.State.ToString());
        }
        finally
        {
            // Teardown
            simpleConsumer.UnInitialize();
            provider?.Uninitialize();
        }
    }

    // EMA provider download dictionary information(RWFFld and RWFEnum)
    // from ADHSimulator. Please take a look at the
    // NiProviderTest.RegisterAndUnRegisterDictionaryReqsTest test case.
    [Theory]
    [InlineData(OmmIProviderConfig.AdminControlMode.USER_CONTROL)]
    [InlineData(OmmIProviderConfig.AdminControlMode.API_CONTROL)]
    public void DownloadFromADH_Test(OmmIProviderConfig.AdminControlMode dictionaryOperationModelMode)
    {
        m_Output.WriteLine("DownloadFromADH_Test BEGIN");

        ProviderSessionOptions providerSessionOpts = new ProviderSessionOptions();
        providerSessionOpts.SendLoginReject = false;
        providerSessionOpts.SendDictionaryResp = true;

        ADHSimulator adhSimulator = new ADHSimulator(providerSessionOpts, m_Output);
        ReactorOptions reactorOptions = new ReactorOptions();

        adhSimulator.Initialize(reactorOptions);

        string hostString = $"localhost:{adhSimulator.ServerPort}";

        m_Output.WriteLine($"Connect with {hostString}");

        OmmIProviderConfig config = new OmmIProviderConfig("../../../OmmIProviderTests/EmaConfigTest.xml")
            .ProviderName("EmaIProvider_RemoteDictionary")
            .Port(PROVIDER_PORT)
            .AdminControlDictionary(dictionaryOperationModelMode);

        OmmProvider? provider = null;
        OmmProviderItemClientTest providerClient = new();

        providerClient.RefreshMsgHandler = (RefreshMsg refreshMsg, IOmmProviderEvent providerEvent) =>
        {
            Assert.NotNull(refreshMsg);
            Assert.NotNull(providerEvent);
        };

        providerClient.ReqMsgHandler = (RequestMsg reqMsg, IOmmProviderEvent providerEvent) =>
        {
            Assert.NotNull(reqMsg);
            switch (reqMsg.DomainType())
            {
                case EmaRdm.MMT_LOGIN:
                    providerEvent.Provider.Submit(new RefreshMsg()
                        .DomainType(EmaRdm.MMT_LOGIN).Name(reqMsg.Name()).NameType(EmaRdm.USER_NAME)
                        .Complete(true).Solicited(true)
                        .State(OmmState.StreamStates.OPEN, OmmState.DataStates.OK, OmmState.StatusCodes.NONE, "Login accepted")
                        .Attrib(new ElementList().Complete()),
                        providerEvent.Handle);
                    break;

                default:
                    break;
            };
        };

        OmmProviderItemClientTest providerFldClient;
        OmmProviderItemClientTest providerEnumClient;

        try
        {
            provider = new OmmProvider(config, providerClient);

            Thread.Sleep(1000);

            adhSimulator.IProviderLaunched(PROVIDER_PORT);

            int countWaits = 10;

            while (providerClient.ReceivedOnAll == 0
                && --countWaits > 0)
            {
                Thread.Sleep(1000);
            }

            Assert.NotEmpty(adhSimulator.EventQueue);
            Assert.NotEqual(0, countWaits);

            providerFldClient = new OmmProviderItemClientTest(provider);

            RequestMsg requestMsg = new();
            requestMsg.DomainType(EmaRdm.MMT_DICTIONARY).Name("RWFFld").ServiceName("DIRECT_FEED")
                .Filter(EmaRdm.DICTIONARY_NORMAL);

            long fldHandle = provider.RegisterClient(requestMsg, providerFldClient, this);

            Assert.True(fldHandle != 0);

            bool fldComplelte = false, enumComplete = false;

            /* Checks the expected RefreshMsg from the RWFFld request */
            providerFldClient.RefreshMsgHandler = (refreshMsg, consEvent) =>
            {
                Assert.Equal(fldHandle, consEvent.Handle);

                Assert.Equal(DataType.DataTypes.SERIES, refreshMsg.Payload().DataType);

                Assert.True(refreshMsg.HasMsgKey);
                Assert.False(refreshMsg.HasNameType);

                Assert.True(refreshMsg.HasName);
                Assert.Equal("RWFFld", refreshMsg.Name());

                Assert.True(refreshMsg.HasServiceName);
                Assert.Equal("DIRECT_FEED", refreshMsg.ServiceName());

                Assert.Equal(OmmState.StatusCodes.NONE, refreshMsg.State().StatusCode);
                Assert.Equal(OmmState.DataStates.OK, refreshMsg.State().DataState);
                Assert.Equal(OmmState.StreamStates.OPEN, refreshMsg.State().StreamState);

                Assert.True(refreshMsg.Solicited());

                fldComplelte = refreshMsg.Complete();
            };

            providerEnumClient = new OmmProviderItemClientTest(provider);

            requestMsg.Clear();
            requestMsg.DomainType(EmaRdm.MMT_DICTIONARY).Name("RWFEnum").ServiceName("DIRECT_FEED")
                .Filter(EmaRdm.DICTIONARY_NORMAL);

            long enumHandle = provider.RegisterClient(requestMsg, providerEnumClient, this);

            Assert.True(enumHandle != 0);

            /* Checks the expected RefreshMsg from the RWFFld request */
            providerEnumClient.RefreshMsgHandler = (refreshMsg, consEvent) =>
            {
                Assert.Equal(enumHandle, consEvent.Handle);

                Assert.Equal(DataType.DataTypes.SERIES, refreshMsg.Payload().DataType);

                Assert.True(refreshMsg.HasMsgKey);
                Assert.False(refreshMsg.HasNameType);

                Assert.True(refreshMsg.HasName);
                Assert.Equal("RWFEnum", refreshMsg.Name());

                Assert.True(refreshMsg.HasServiceName);
                Assert.Equal("DIRECT_FEED", refreshMsg.ServiceName());

                Assert.Equal(OmmState.StatusCodes.NONE, refreshMsg.State().StatusCode);
                Assert.Equal(OmmState.DataStates.OK, refreshMsg.State().DataState);
                Assert.Equal(OmmState.StreamStates.OPEN, refreshMsg.State().StreamState);

                Assert.True(refreshMsg.Solicited());
                enumComplete = refreshMsg.Complete();
            };

            Thread.Sleep(5_000);

            Assert.True(fldComplelte);
            Assert.True(enumComplete);

            Assert.Equal(2, providerFldClient.ReceivedOnAll);
            Assert.Equal(2, providerFldClient.ReceivedOnRefresh);
            Assert.Equal(0, providerFldClient.ReceivedOnStatus);

            Assert.Equal(13, providerEnumClient.ReceivedOnAll);
            Assert.Equal(13, providerEnumClient.ReceivedOnRefresh);
            Assert.Equal(0, providerEnumClient.ReceivedOnStatus);

            provider.Unregister(fldHandle);
            provider.Unregister(enumHandle);
        }
        finally
        {
            provider?.Uninitialize();
            adhSimulator.UnInitialize();
        }
    }


    #region Helper functions

    private static void InitializeSimpleConsumer(OmmProvider provider, OmmProviderItemClientTest providerClient,
        SimpleConsumerTest simpleConsumer, bool initDictionaryRequest = true)
    {
        ReactorOptions reactorOptions = new ReactorOptions();

        simpleConsumer.Initialize(reactorOptions,
            connectOptions: null,
            initLoginRequest: true,
            initDirectoryRequest: true,
            initDictionaryRequest: initDictionaryRequest);

        Thread.Sleep(3_000);

        Assert.Equal(4, simpleConsumer.EventQueue.Count);

        var reactorEvent = simpleConsumer.EventQueue.Dequeue(); // 1.
        ReactorChannelEvent channelEvent = (ReactorChannelEvent)reactorEvent.ReactorEvent;
        Assert.Equal(ReactorChannelEventType.CHANNEL_UP, channelEvent.EventType);

        reactorEvent = simpleConsumer.EventQueue.Dequeue(); // 2.
        Assert.Equal(TestReactorEventType.LOGIN_MSG, reactorEvent.EventType);

        reactorEvent = simpleConsumer.EventQueue.Dequeue(); // 3.
        Assert.Equal(TestReactorEventType.DIRECTORY_MSG, reactorEvent.EventType);

        reactorEvent = simpleConsumer.EventQueue.Dequeue(); // 4.
        channelEvent = (ReactorChannelEvent)reactorEvent.ReactorEvent;
        Assert.Equal(ReactorChannelEventType.CHANNEL_READY, channelEvent.EventType);

        switch (((OmmServerBaseImpl)provider!.m_OmmProviderImpl).ConfigImpl.AdminControlDictionary)
        {
            case OmmIProviderConfig.AdminControlMode.USER_CONTROL:
                // Provider Client -- that is, the User-provided application, receives
                // Login, Dictionary (when Dictionary request is sent) and two additional
                // requests -- one for the fields and another for the enums dictionaries.
                if (initDictionaryRequest)
                    Assert.Equal(4, providerClient.ReceivedOnReqMsg);
                else
                    Assert.Equal(2, providerClient.ReceivedOnReqMsg);
                break;
            case OmmIProviderConfig.AdminControlMode.API_CONTROL:
                // These is 1 request for the USER-managed domain: Login
                Assert.Equal(1, providerClient.ReceivedOnReqMsg);
                break;
        }

        OmmServerBaseImpl serverBaseImpl = (OmmServerBaseImpl)provider!.m_OmmProviderImpl;
        Assert.Single(serverBaseImpl.ConnectedChannelList);

        // consumer gets dictionaries only when requested in the first place
        Assert.Equal(initDictionaryRequest, simpleConsumer.RDMFieldDictComplete);
        Assert.Equal(initDictionaryRequest, simpleConsumer.RDMEnumDictComplete);

        if (initDictionaryRequest)
            Assert.NotEqual(0, simpleConsumer.DictionaryStreamId);
        else
            Assert.Equal(0, simpleConsumer.DictionaryStreamId);
    }

    private static void SubmitDirectoryResponse(RequestMsg requestMsg, IOmmProviderEvent providerEvent, OmmProvider provider,
        ulong serviceId, string serviceName)
    {
        var payload = new Map();
        var service = new FilterList();
        service.AddEntry(EmaRdm.SERVICE_INFO_ID,
            FilterAction.SET,
            new ElementList()
                .AddAscii(EmaRdm.ENAME_NAME, serviceName)
                .AddArray(EmaRdm.ENAME_DICTIONARYS_USED, new OmmArray()
                    .AddAscii("RWFFld")
                    .AddAscii("RWFEnum").Complete())
                .AddArray(EmaRdm.ENAME_DICTIONARYS_PROVIDED, new OmmArray()
                    .AddAscii("RWFFld")
                    .AddAscii("RWFEnum").Complete())
                    .Complete())
               .AddEntry(EmaRdm.SERVICE_STATE_ID, FilterAction.SET,
            new ElementList()
                .AddUInt(EmaRdm.ENAME_SVC_STATE, EmaRdm.SERVICE_UP)
                .AddUInt(EmaRdm.ENAME_ACCEPTING_REQS, 1).Complete())
                .Complete();

        payload.AddKeyUInt(serviceId, MapAction.ADD, service).Complete();

        provider!.Submit(new RefreshMsg().StreamId(requestMsg.StreamId())
            .DomainType(requestMsg.DomainType()).Filter(EmaRdm.SERVICE_INFO_FILTER | EmaRdm.SERVICE_STATE_FILTER)
            .Complete(true).Solicited(true).Payload(payload), providerEvent.Handle);
    }

    private static void SubmitDictionaryResponse(RequestMsg requestMsg, IOmmProviderEvent providerEvent, Rdm.DataDictionary dataDictionary)
    {
        // A similar Dictionary request handler can be found in the 332 Interactive Provider
        // training example application 332_Dictionary_UserControl

        var series = new Series();

        RefreshMsg refreshMsg = new RefreshMsg();

        int fragmentationSize = 1024 * 1024;
        int currentValue;
        bool result = false;

        // Encode and send dictionary refresh messages in response
        if (requestMsg.Name().Equals("RWFFld"))
        {
            currentValue = dataDictionary.MinFid;

            while (!result)
            {
                currentValue = dataDictionary.EncodeFieldDictionary(series, currentValue, requestMsg.Filter(),
                    fragmentationSize);
                result = (currentValue == dataDictionary.MaxFid);

                providerEvent.Provider.Submit(refreshMsg.Name(requestMsg.Name())//.ServiceName(requestMsg.ServiceName())
                    .ServiceId(requestMsg.ServiceId())
                    .DomainType(EmaRdm.MMT_DICTIONARY).Filter(requestMsg.Filter()).Payload(series)
                    .Complete(result).Solicited(true),
                    providerEvent.Handle);
            }
        }
        else if (requestMsg.Name().Equals("RWFEnum"))
        {
            currentValue = 0;

            while (!result)
            {
                currentValue = dataDictionary.EncodeEnumTypeDictionary(series, currentValue, requestMsg.Filter(), fragmentationSize);
                result = (currentValue == dataDictionary.EnumTables().Count);

                providerEvent.Provider.Submit(refreshMsg.Name(requestMsg.Name())
                    .ServiceId(requestMsg.ServiceId())
                    .DomainType(EmaRdm.MMT_DICTIONARY).Filter(requestMsg.Filter()).Payload(series)
                    .Complete(result).Solicited(true),
                    providerEvent.Handle);

            }
        }
        else
        {
            // The consumer requested dictionary with an invalid name, emulate corresponding
            // Dictionary Status message in response
            providerEvent.Provider.Submit(new StatusMsg().Name(requestMsg.Name()).ServiceId(requestMsg.ServiceId())
                .State(OmmState.StreamStates.CLOSED_RECOVER, OmmState.DataStates.SUSPECT, OmmState.StatusCodes.ERROR,
                    $"Dictionary request message rejected - the reqesting dictionary name '{requestMsg.Name()}' not found.")
                .DomainType(EmaRdm.MMT_DICTIONARY),
                providerEvent.Handle);
        }
    }

    /// <summary>
    /// Modify EMA Provider configuration to make it output log message to Console. These
    /// messages are captured into the memoryStream to be analyzed for presence of
    /// expected error messages
    /// </summary>
    private static void ConfigureConsoleLoggerOutput(OmmIProviderConfig config)
    {
        var configImpl = config.OmmIProvConfigImpl;

        var testLogger = new LoggerConfig();
        testLogger.Name = "DictionaryLogger";
        testLogger.LoggerType = LoggerType.STDOUT;
        testLogger.LoggerSeverity = LoggerLevel.TRACE;

        var serverConfig = new ServerConfig();
        serverConfig.Name = "DictionaryServer";

        var providerConfig = new IProviderConfig();
        providerConfig.Logger = testLogger.Name;
        providerConfig.Server = serverConfig.Name;

        configImpl.DefaultIProvider = "DefaultEmaIProvider";

        configImpl.IProviderConfigMap[configImpl.DefaultIProvider] = providerConfig;

        configImpl.LoggerConfigMap[testLogger.Name] = testLogger;
        configImpl.ServerConfigMap[serverConfig.Name] = serverConfig;
    }

    private void InvalidMessageSubmitHelper(OmmIProviderConfig.AdminControlMode dictionaryOperationModelMode,
        Action<SimpleConsumerTest> messageSender, string expectedMessage)
    {
        // Intercept standard output (Console) to the memory buffer to examine Logger
        // messages (similar to ConsumerLoggerTest) for expected error reports
        MemoryStream memoryStream = new(12 * 1024);
        StreamWriter streamWriter = new(memoryStream);
        System.Console.SetOut(streamWriter);

        OmmIProviderConfig config = (dictionaryOperationModelMode == OmmIProviderConfig.AdminControlMode.API_CONTROL)
            ? new OmmIProviderConfig("../../../OmmIProviderTests/EmaConfigTest.xml")
            : new OmmIProviderConfig();

        OmmProvider? provider = null;
        OmmProviderItemClientTest providerClient = new();

        SimpleConsumerTest simpleConsumer = new SimpleConsumerTest(m_Output, PROVIDER_PORT);

        ulong serviceId = 1;
        string serviceName = "DIRECT_FEED";

        var dataDictionary = new Rdm.DataDictionary();

        dataDictionary.LoadFieldDictionary(DataDictionaryTests.FIELD_DICTIONARY_FILENAME);
        dataDictionary.LoadEnumTypeDictionary(DataDictionaryTests.ENUM_TABLE_FILENAME);

        // While Source Directory Handler is missing, provide a USER-generated response with dictionaries
        providerClient.ReqMsgHandler = (requestMsg, providerEvent) =>
        {
            switch (requestMsg.DomainType())
            {
                case EmaRdm.MMT_DIRECTORY:
                    SubmitDirectoryResponse(requestMsg, providerEvent, provider!, serviceId, serviceName);
                    break;

                case EmaRdm.MMT_DICTIONARY:
                    // Application receives Dictionary callback only when Dictionary operation
                    // is under USER_CONTROL
                    Assert.Equal(OmmIProviderConfig.AdminControlMode.USER_CONTROL, dictionaryOperationModelMode);

                    SubmitDictionaryResponse(requestMsg, providerEvent, dataDictionary);
                    break;

                default:
                    provider!.Submit(new RefreshMsg().StreamId(requestMsg.StreamId())
                        .DomainType(requestMsg.DomainType())
                        .Solicited(true), providerEvent.Handle);
                    break;
            }
        };

        ConfigureConsoleLoggerOutput(config);

        try
        {
            provider = new OmmProvider(config.Port(PROVIDER_PORT)
                .AdminControlDirectory(dictionaryOperationModelMode)
                .AdminControlDictionary(dictionaryOperationModelMode),
                providerClient);

            Assert.Contains("DefaultEmaIProvider", provider.ProviderName);

            InitializeSimpleConsumer(provider, providerClient, simpleConsumer);

            messageSender(simpleConsumer);

            Thread.Sleep(1000);

            string logOutput = System.Text.Encoding.ASCII.GetString(memoryStream.GetBuffer(), 0,
                (int)memoryStream.Length);

            Assert.NotNull(logOutput);
            Assert.NotEmpty(logOutput);

            // expected error message
            Assert.Contains(expectedMessage, logOutput);
        }
        finally
        {
            simpleConsumer.UnInitialize();
            provider?.Uninitialize();
        }
    }

    #endregion
}
