/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2023-2024 LSEG. All rights reserved.     
 *|-----------------------------------------------------------------------------
 */

using LSEG.Ema.Rdm;
using LSEG.Eta.Codec;
using LSEG.Eta.ValueAdd.Reactor;
using System;
using System.Threading;
using Xunit.Abstractions;

namespace LSEG.Ema.Access.Tests.OmmConsumerTests;

/// testing the <see cref="Access.DictionaryCallbackClient"/>
public class DictionaryCallbackClientTests : IDisposable
{
    public void Dispose()
    {
        EtaGlobalPoolTestUtil.Clear();
    }

    private const int TEST_TIMEOUT_MS = 10_000;

    private ITestOutputHelper output;

    public DictionaryCallbackClientTests(ITestOutputHelper output)
    {
        this.output = output;
    }


    /// With default options <see cref="OmmConsumer"/> downloads Dictionaries from
    /// Provider or throws an exception if it can't
    [Theory]
    [InlineData(true)]
    [InlineData(false)]
    public void CreateValidOmmConsumerWithProvidedDictionary_Test(bool sendDictionaryResponse)
    {
        ProviderSessionOptions providerSessionOpts = new ProviderSessionOptions()
        {
            // whether Provider is going to respond for the Dictionary requests
            SendDictionaryResp = sendDictionaryResponse
        };
        providerSessionOpts.SendLoginReject = false;
        ProviderTest providerTest = new ProviderTest(providerSessionOpts, output);
        ReactorOptions reactorOptions = new ReactorOptions();
        providerTest.Initialize(reactorOptions);

        string hostString = $"localhost:{providerTest.ServerPort}";

        output.WriteLine($"Connect with {hostString}");

        OmmConsumerConfig config = new OmmConsumerConfig();
        OmmConsumer? consumer = null;

        try
        {
            consumer = new OmmConsumer(config.Host(hostString));
            Assert.True(sendDictionaryResponse, "Make sure exception is thrown only when expected");
        }
        catch (OmmInvalidUsageException ex)
        {
            Assert.False(sendDictionaryResponse, $"Exception during initialization not expected: {ex}");
            Assert.Equal(OmmInvalidUsageException.ErrorCodes.DICTIONARY_REQUEST_TIME_OUT,
                ex.ErrorCode);
            Assert.Contains("Dictionary retrieval failed (timed out after waiting", ex.Message);
        }

        consumer?.Uninitialize();
        providerTest.UnInitialize();
    }


    /// The <see cref="OmmConsumer"/> can be configured to load Dictionaries from the
    /// filesystem
    [Theory]
    [InlineData(true)]
    [InlineData(false)]
    public void CreateValidOmmConsumerWithLocalDictionary_Test(bool correctPaths)
    {
        ProviderSessionOptions providerSessionOpts = new ProviderSessionOptions()
        {
            // ensure that Provider won't send a Dictionary response
            SendDictionaryResp = false
        };
        providerSessionOpts.SendLoginReject = false;
        ProviderTest providerTest = new ProviderTest(providerSessionOpts, output);
        ReactorOptions reactorOptions = new ReactorOptions();
        providerTest.Initialize(reactorOptions);

        string hostString = $"localhost:{providerTest.ServerPort}";

        output.WriteLine($"Connect with {hostString}");

        OmmConsumerConfig config = new OmmConsumerConfig("../../../OmmConsumerTests/EmaDictionariesConfig.xml");

        config.OmmConsConfigImpl.DictionaryConfig.IsLocalDictionary = true;
        OmmConsumerConfigImpl configImpl = config.OmmConsConfigImpl;

        if (correctPaths)
        {
            DictionaryConfig dictConfig = configImpl.DictionaryConfigMap["TestDictionary_1"];

            dictConfig.EnumTypeDefFileName = DictionaryHandler.ENUM_TABLE_FILENAME;
            dictConfig.RdmFieldDictionaryFileName = DictionaryHandler.FIELD_DICTIONARY_FILENAME;

        }
        configImpl.HostName = "localhost";
        configImpl.Port = providerTest.ServerPort.ToString();
        config.Host(hostString);

        OmmConsumer? consumer = null;

        try
        {
            consumer = new OmmConsumer(config);
            Assert.True(correctPaths, "Make sure exception is thrown only when expected");
        }
        catch (System.Exception ex)
        {
            Assert.False(correctPaths, $"Exception during initialization not expected: {ex}");
            OmmInvalidUsageException? ommException = ex as OmmInvalidUsageException;
            Assert.NotNull(ommException);
            Assert.Equal(OmmInvalidUsageException.ErrorCodes.FAILURE,
                ommException!.ErrorCode);
            Assert.Contains("Unable to load RDMFieldDictionary from file named RDMTestDictionary.file",
                ex.Message);
        }

        consumer?.Uninitialize();
        providerTest.UnInitialize();
    }


    /// <summary>
    /// Register clients with Dictionary requests and check that the dictionaries are
    /// successfully downloaded.
    /// </summary>
    /// <param name="userDispatch"></param>
    /// <param name="serviceName">whether service name is specified for the registered
    ///   request. when specified: dictionaries are downloaded from Provider; when not
    ///   specified: callback method receives locally cached dicionaries</param>
    [Theory]
    [InlineData(false, true)]
    [InlineData(false, false)]
    public void SimpleDictionaryRequest_Test(bool userDispatch = false, bool serviceName = true)
    {
        output.WriteLine($"SimpleDictionaryRequest_Test() userDispatch={userDispatch} serviceName={serviceName} BEGIN");

        ProviderSessionOptions providerSessionOpts = new ProviderSessionOptions()
        {
            SendDictionaryResp = true
        };

        ProviderTest providerTest = new ProviderTest(providerSessionOpts, output);
        ReactorOptions reactorOptions = new ReactorOptions();
        providerTest.Initialize(reactorOptions);

        string hostString = $"localhost:{providerTest.ServerPort}";

        OmmConsumerConfig config = new OmmConsumerConfig();
        OmmConsumer? consumer = null;
        OmmConsumerDictionaryClientTest consumerClient;

        Rdm.DataDictionary dataDictionary = new();

        config.Host(hostString);

        if (userDispatch)
            config.OperationModel(OmmConsumerConfig.OperationModelMode.USER_DISPATCH);

        consumer = new OmmConsumer(config.Host(hostString));
        // note: EMA should have Dictionary info by this moment

        consumerClient = new OmmConsumerDictionaryClientTest(consumer);

        bool fldDictComplete = false;
        bool enumTypeComplete = false;
        int handledRefreshes = 0;
        bool extractFidType = false;
        bool extractEnumType = false;

        /* Checks the expected RefreshMsg from the dictionary request and caches
        * received dictionaries into dataDictionary */
        consumerClient.RefreshMsgHandler = (refreshMsg, consEvent) =>
        {
            bool complete = refreshMsg.MarkForClear().Complete();

            switch (refreshMsg.MarkForClear().Payload().DataType)
            {
                case DataTypes.SERIES:
                    if (refreshMsg.Name().Equals("RWFFld"))
                    {
                        if (!extractFidType)
                        {
                            Rdm.DataDictionary dataDictionaryTemp = new();
                            Assert.Equal(1, dataDictionaryTemp.ExtractDictionaryType(refreshMsg.MarkForClear().Payload().Series()));
                            extractFidType = true;
                        }

                        dataDictionary.DecodeFieldDictionary(refreshMsg.MarkForClear().Payload().Series(), EmaRdm.DICTIONARY_NORMAL);
                        fldDictComplete = complete;
                    }
                    else if (refreshMsg.Name().Equals("RWFEnum"))
                    {
                        if (!extractEnumType)
                        {
                            Rdm.DataDictionary dataDictionaryTemp = new();
                            Assert.Equal(2, dataDictionaryTemp.ExtractDictionaryType(refreshMsg.MarkForClear().Payload().Series()));
                            extractEnumType = true;
                        }

                        dataDictionary.DecodeEnumTypeDictionary(refreshMsg.MarkForClear().Payload().Series(), EmaRdm.DICTIONARY_NORMAL);
                        enumTypeComplete = complete;
                    }
                    break;

                default:
                    break;
            }

            handledRefreshes++;
        };

        // these are additional requests by the application
        RequestMsg requestMsg = new();
        requestMsg.DomainType((int)Eta.Rdm.DomainType.DICTIONARY)
            .Name("RWFFld")
            .Filter(EmaRdm.DICTIONARY_NORMAL)
            .InterestAfterRefresh(true);
        if (serviceName)
            requestMsg.ServiceName("DEFAULT_SERVICE");
        long fldHandle = consumer.RegisterClient(requestMsg, consumerClient, this);

        requestMsg.Clear();
        requestMsg.DomainType((int)Eta.Rdm.DomainType.DICTIONARY)
            .Name("RWFEnum")
            .Filter(EmaRdm.DICTIONARY_NORMAL)
            .InterestAfterRefresh(true);
        if (serviceName)
            requestMsg.ServiceName("DEFAULT_SERVICE");
        long enumHandle = consumer.RegisterClient(requestMsg, consumerClient, this);

        if (userDispatch)
        {
            consumer.Dispatch(DispatchTimeout.INFINITE_WAIT);
            Assert.True(false);
        }
        else
        {
            Thread.Sleep(TEST_TIMEOUT_MS);
        }

        Assert.True(fldDictComplete);
        Assert.True(enumTypeComplete);
        Assert.True(dataDictionary.IsFieldDictionaryLoaded);
        Assert.True(dataDictionary.IsEnumTypeDefLoaded);
        Assert.Equal(-32768, dataDictionary.MinFid);
        Assert.Equal(32767, dataDictionary.MaxFid);
        Assert.Equal(15052, dataDictionary.Entries().Count);
        Assert.Equal(575, dataDictionary.EnumTables().Count);

        Assert.Equal("PERMISSION", dataDictionary.Entry(1).DdeAcronym);
        Assert.Equal("MIN_FID_DDE", dataDictionary.Entry(-32768).DdeAcronym);

        // finally compare received data dictionaries to the dictionaries loaded locally
        // from the same files
        Eta.Codec.DataDictionary localDictionary = new();
        Assert.Equal(CodecReturnCode.SUCCESS,
            localDictionary.LoadFieldDictionary(DictionaryHandler.FIELD_DICTIONARY_FILENAME, out var fieldError));
        Assert.Equal(CodecReturnCode.SUCCESS,
            localDictionary.LoadEnumTypeDictionary(DictionaryHandler.ENUM_TABLE_FILENAME, out var enumError));

        DataDictionaryTests.AssertEqualDataDictionary(localDictionary, dataDictionary, true);

        consumer.Unregister(fldHandle);
        consumer.Unregister(enumHandle);

        consumer?.Uninitialize();
        providerTest.UnInitialize();

        output.WriteLine($"SimpleDictionaryRequest_Test() userDispatch={userDispatch} DONE");
    }
}
