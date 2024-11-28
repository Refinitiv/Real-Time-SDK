/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2023-2024 LSEG. All rights reserved.     
 *|-----------------------------------------------------------------------------
 */

using LSEG.Ema.Access.Tests.OmmConsumerTests;
using LSEG.Ema.Rdm;
using LSEG.Eta.Codec;
using LSEG.Eta.Transports;
using LSEG.Eta.ValueAdd.Rdm;
using LSEG.Eta.ValueAdd.Reactor;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Net;
using System.Threading;
using Xunit.Abstractions;
using static LSEG.Ema.Access.Tests.OmmConsumerTests.ModifyIOCtlTest;
using static LSEG.Eta.Rdm.Directory;


namespace LSEG.Ema.Access.Tests.OmmNiProviderTests
{
    public class NiProviderTest : IDisposable
    {
        public void Dispose()
        {
            EtaGlobalPoolTestUtil.Clear();
        }

        ITestOutputHelper m_Output;

        public NiProviderTest(ITestOutputHelper output)
        {
            m_Output = output;
        }

        private static readonly string EMA_FILE_PATH = "../../../OmmNiProviderTests/EmaConfigNiProviderTest.xml";

        [Fact]
        public void LoginRequestTimeoutTest()
        {
            OmmNiProviderConfig config = new OmmNiProviderConfig(EMA_FILE_PATH);
            OmmException? exception = null;
            OmmProvider? provider = null;

            try
            {
                provider = new OmmProvider(config);
            }
            catch (OmmException ommException)
            {
                exception = ommException;
                Assert.True(exception.Type == OmmException.ExceptionType.OmmInvalidUsageException);
                Assert.StartsWith("login failed (timed out after waiting 5000 milliseconds", exception.Message);
            }

            Assert.NotNull(exception);
            Assert.Null(provider);
        }

        [Fact]
        public void LoginRejectByProviderTest()
        {
            ProviderSessionOptions providerSessionOpts = new ProviderSessionOptions();
            providerSessionOpts.SendLoginReject = true;
            ADHSimulator adhSimulator = new ADHSimulator(providerSessionOpts, m_Output);
            ReactorOptions reactorOptions = new ReactorOptions();
            adhSimulator.Initialize(reactorOptions);

            string hostString = $"localhost:{adhSimulator.ServerPort}";

            m_Output.WriteLine($"Connect with {hostString}");

            OmmNiProviderConfig config = new OmmNiProviderConfig();
            OmmException? exception = null;
            OmmProvider? provider = null;

            try
            {
                provider = new OmmProvider(config.Host(hostString));
            }
            catch (OmmException ommException)
            {
                exception = ommException;
                Assert.True(exception.Type == OmmException.ExceptionType.OmmInvalidUsageException);
                m_Output.WriteLine($"exception.Message = {exception.Message}");
                Assert.Contains("Login request rejected for stream id 1 - Force logout by Provider", exception.Message);
            }

            Assert.NotNull(exception);
            Assert.Null(provider);
            adhSimulator.UnInitialize();
        }

        [Fact]
        public void LoginAndPublishDefaultSourceDirectoryTest()
        {
            ProviderSessionOptions providerSessionOpts = new ProviderSessionOptions();
            providerSessionOpts.SendLoginReject = false;
            ADHSimulator adhSimulator = new ADHSimulator(providerSessionOpts, m_Output);
            ReactorOptions reactorOptions = new ReactorOptions();
            adhSimulator.Initialize(reactorOptions);

            string hostString = $"localhost:{adhSimulator.ServerPort}";

            m_Output.WriteLine($"Connect with {hostString}");

            OmmNiProviderConfig config = new OmmNiProviderConfig();
            OmmException? exception = null;
            OmmProvider? provider = null;

            try
            {
                provider = new OmmProvider(config.Host(hostString));
                m_Output.WriteLine("Created OmmProvider successfully");
            }
            catch (OmmException ommException)
            {
                exception = ommException;
            }

            Assert.Null(exception);

            // Waits until the simulator receives the directory message.
            Thread.Sleep(1000);

            DirectoryCache expectedCache = new();
            // This will generate the default config
            EmaServiceConfig tmpService = new EmaServiceConfig(true, true);
            expectedCache.AddService(tmpService.Service);

            // Sets by EMA's NiProvider before submitting to ADH.
            expectedCache.DirectoryRefresh.ClearCache = true;
            expectedCache.DirectoryRefresh.Filter = ServiceFilterFlags.INFO | ServiceFilterFlags.STATE;

            // Checks the hard coded source directory in ADHSimulator
            DirectoryRefresh sourceDirectoryCache = adhSimulator.DirectoryCache;
            Assert.Equal(expectedCache.DirectoryRefresh.ToString(), sourceDirectoryCache.ToString());

            Assert.Null(exception);
            provider?.Uninitialize();
            adhSimulator.UnInitialize();
        }

        [Fact]
        public void LoginAndPublishMarketDataForDefaultSourceDirectoryTest()
        {
            ProviderSessionOptions providerSessionOpts = new ProviderSessionOptions();
            providerSessionOpts.SendLoginReject = false;
            ADHSimulator adhSimulator = new ADHSimulator(providerSessionOpts, m_Output);
            ReactorOptions reactorOptions = new ReactorOptions();
            adhSimulator.Initialize(reactorOptions);

            string hostString = $"localhost:{adhSimulator.ServerPort}";

            m_Output.WriteLine($"Connect with {hostString}");

            OmmNiProviderConfig config = new OmmNiProviderConfig();
            OmmException? exception = null;
            OmmProvider? provider = null;

            try
            {
                provider = new OmmProvider(config.Host(hostString));
                m_Output.WriteLine("Created OmmProvider successfully");
            }
            catch (OmmException ommException)
            {
                exception = ommException;
            }

            Assert.Null(exception);

            // Waits until the simulator receives the directory message.
            Thread.Sleep(1000);

            // Remove the channel and login events
            adhSimulator.EventQueue.Clear();

            long itemHandle = 5;

            provider?.Submit(new RefreshMsg().ServiceName("NI_PUB").Name("IBM.N")
                .State(OmmState.StreamStates.OPEN, OmmState.DataStates.OK, OmmState.StatusCodes.NONE, "UnSolicited Refresh Completed")
                .MarkForClear().Payload(new FieldList()
                    .AddReal(22, 3990, OmmReal.MagnitudeTypes.EXPONENT_NEG_2)
                    .AddReal(25, 3994, OmmReal.MagnitudeTypes.EXPONENT_NEG_2)
                    .AddReal(30, 0, OmmReal.MagnitudeTypes.EXPONENT_0)
                    .AddReal(31, 19, OmmReal.MagnitudeTypes.EXPONENT_0)
                    .MarkForClear().Complete())
                .MarkForClear().Complete(true).ClearCache(true), itemHandle);

            Thread.Sleep(1000);

            Assert.Single(adhSimulator.EventQueue);

            TestReactorEvent testReactorEvent = adhSimulator.EventQueue.Dequeue();

            Assert.Equal(TestReactorEventType.MSG, testReactorEvent.EventType);
            ReactorMsgEvent msgEvent = (ReactorMsgEvent)testReactorEvent.ReactorEvent;

            RefreshMsg decodeRefreshMsg = new();

            decodeRefreshMsg.Decode(msgEvent.Msg!, Codec.MajorVersion(), Codec.MinorVersion(),
                adhSimulator.DictionaryHandler.DataDictionary);

            int expectedStreamId = -1;

            Assert.Equal(expectedStreamId, decodeRefreshMsg.StreamId());
            Assert.True(decodeRefreshMsg.HasName);
            Assert.Equal("IBM.N", decodeRefreshMsg.Name());
            Assert.True(decodeRefreshMsg.HasServiceId);
            Assert.Equal(0, decodeRefreshMsg.ServiceId());
            Assert.Equal(OmmState.StreamStates.OPEN, decodeRefreshMsg.State().StreamState);
            Assert.Equal(OmmState.DataStates.OK, decodeRefreshMsg.State().DataState);
            Assert.Equal(OmmState.StatusCodes.NONE, decodeRefreshMsg.State().StatusCode);
            Assert.True(decodeRefreshMsg.MarkForClear().Complete());
            Assert.True(decodeRefreshMsg.ClearCache());
            Assert.False(decodeRefreshMsg.Solicited());
            Assert.Equal(DataType.DataTypes.FIELD_LIST, decodeRefreshMsg.MarkForClear().Payload().DataType);

            FieldList fieldList = decodeRefreshMsg.MarkForClear().Payload().FieldList();

            var iterator = fieldList.GetEnumerator();
            Assert.True(iterator.MoveNext());
            FieldEntry entry = iterator.Current;
            Assert.Equal(22, entry.FieldId);
            Assert.Equal(39.90, entry.OmmRealValue().AsDouble());

            Assert.True(iterator.MoveNext());
            entry = iterator.Current;
            Assert.Equal(25, entry.FieldId);
            Assert.Equal(39.94, entry.OmmRealValue().AsDouble());

            Assert.True(iterator.MoveNext());
            entry = iterator.Current;
            Assert.Equal(30, entry.FieldId);
            Assert.Equal(0, entry.OmmRealValue().AsDouble());

            Assert.True(iterator.MoveNext());
            entry = iterator.Current;
            Assert.Equal(31, entry.FieldId);
            Assert.Equal(19, entry.OmmRealValue().AsDouble());

            Assert.False(iterator.MoveNext());

            Assert.Empty(adhSimulator.EventQueue); // There is no message in the Event queue.

            provider?.Submit(new UpdateMsg().ServiceName("NI_PUB").Name("IBM.N")
                            .MarkForClear().Payload(new FieldList()
                                .AddReal(22, 3391 + 5, OmmReal.MagnitudeTypes.EXPONENT_NEG_2)
                                .AddReal(30, 10 + 5, OmmReal.MagnitudeTypes.EXPONENT_0)
                                .MarkForClear().Complete()), itemHandle);

            Thread.Sleep(1000);

            Assert.Single(adhSimulator.EventQueue);

            testReactorEvent = adhSimulator.EventQueue.Dequeue();

            Assert.Equal(TestReactorEventType.MSG, testReactorEvent.EventType);
            msgEvent = (ReactorMsgEvent)testReactorEvent.ReactorEvent;

            UpdateMsg decodeUpdateMsg = new();

            decodeUpdateMsg.Decode(msgEvent.Msg!, Codec.MajorVersion(), Codec.MinorVersion(),
                adhSimulator.DictionaryHandler.DataDictionary);

            Assert.Equal(expectedStreamId, decodeUpdateMsg.StreamId());
            Assert.True(decodeUpdateMsg.HasName);
            Assert.Equal("IBM.N", decodeUpdateMsg.Name());
            Assert.True(decodeUpdateMsg.HasServiceId);
            Assert.Equal(0, decodeUpdateMsg.ServiceId());
            Assert.Equal(DataType.DataTypes.FIELD_LIST, decodeUpdateMsg.MarkForClear().Payload().DataType);

            fieldList = decodeUpdateMsg.MarkForClear().Payload().FieldList();

            iterator = fieldList.GetEnumerator();
            Assert.True(iterator.MoveNext());
            entry = iterator.Current;
            Assert.Equal(22, entry.FieldId);
            Assert.Equal(33.96, entry.OmmRealValue().AsDouble());

            Assert.True(iterator.MoveNext());
            entry = iterator.Current;
            Assert.Equal(30, entry.FieldId);
            Assert.Equal(15, entry.OmmRealValue().AsDouble());

            Assert.False(iterator.MoveNext());

            Assert.Empty(adhSimulator.EventQueue); // There is no message in the Event queue.

            provider?.Submit(new UpdateMsg().ServiceId(0).Name("IBM.N")
                            .MarkForClear().Payload(new FieldList()
                                .AddReal(22, 3391 + 9, OmmReal.MagnitudeTypes.EXPONENT_NEG_2)
                                .AddReal(30, 10 + 10, OmmReal.MagnitudeTypes.EXPONENT_0)
                                .MarkForClear().Complete()), itemHandle);

            Thread.Sleep(1000);

            Assert.Single(adhSimulator.EventQueue);

            testReactorEvent = adhSimulator.EventQueue.Dequeue();

            Assert.Equal(TestReactorEventType.MSG, testReactorEvent.EventType);
            msgEvent = (ReactorMsgEvent)testReactorEvent.ReactorEvent;

            decodeUpdateMsg.Decode(msgEvent.Msg!, Codec.MajorVersion(), Codec.MinorVersion(),
                adhSimulator.DictionaryHandler.DataDictionary);

            Assert.Equal(expectedStreamId, decodeUpdateMsg.StreamId());
            Assert.True(decodeUpdateMsg.HasName);
            Assert.Equal("IBM.N", decodeUpdateMsg.Name());
            Assert.True(decodeUpdateMsg.HasServiceId);
            Assert.Equal(0, decodeUpdateMsg.ServiceId());
            Assert.Equal(DataType.DataTypes.FIELD_LIST, decodeUpdateMsg.MarkForClear().Payload().DataType);

            fieldList = decodeUpdateMsg.MarkForClear().Payload().FieldList();

            iterator = fieldList.GetEnumerator();
            Assert.True(iterator.MoveNext());
            entry = iterator.Current;
            Assert.Equal(22, entry.FieldId);
            Assert.Equal(34.0, entry.OmmRealValue().AsDouble());

            Assert.True(iterator.MoveNext());
            entry = iterator.Current;
            Assert.Equal(30, entry.FieldId);
            Assert.Equal(20, entry.OmmRealValue().AsDouble());

            Assert.False(iterator.MoveNext());

            Assert.Empty(adhSimulator.EventQueue); // There is no message in the Event queue.

            Assert.Null(exception);
            provider?.Uninitialize();
            adhSimulator.UnInitialize();
        }

        [Fact]
        public void LoginAndPublishMarketDataWithInvalidServiceIdOrNameTest()
        {
            ProviderSessionOptions providerSessionOpts = new ProviderSessionOptions();
            providerSessionOpts.SendLoginReject = false;
            ADHSimulator adhSimulator = new ADHSimulator(providerSessionOpts, m_Output);
            ReactorOptions reactorOptions = new ReactorOptions();
            adhSimulator.Initialize(reactorOptions);

            string hostString = $"localhost:{adhSimulator.ServerPort}";

            m_Output.WriteLine($"Connect with {hostString}");

            OmmNiProviderConfig config = new OmmNiProviderConfig();
            OmmException? exception = null;
            OmmProvider? provider = null;

            try
            {
                provider = new OmmProvider(config.Host(hostString));
                m_Output.WriteLine("Created OmmProvider successfully");
            }
            catch (OmmException execp)
            {
                exception = execp;
            }

            Assert.Null(exception);

            // Waits until the simulator receives the directory message.
            Thread.Sleep(1000);

            // Remove the channel and login events
            adhSimulator.EventQueue.Clear();

            long itemHandle = 5;

            try
            {
                provider?.Submit(new RefreshMsg().ServiceName("UNKNOWN_SERVICE_NAME").Name("IBM.N")
                    .State(OmmState.StreamStates.OPEN, OmmState.DataStates.OK, OmmState.StatusCodes.NONE, "UnSolicited Refresh Completed")
                    .MarkForClear().Payload(new FieldList()
                        .AddReal(22, 3990, OmmReal.MagnitudeTypes.EXPONENT_NEG_2)
                        .AddReal(25, 3994, OmmReal.MagnitudeTypes.EXPONENT_NEG_2)
                        .AddReal(30, 0, OmmReal.MagnitudeTypes.EXPONENT_0)
                        .AddReal(31, 19, OmmReal.MagnitudeTypes.EXPONENT_0)
                        .MarkForClear().Complete())
                    .MarkForClear().Complete(true).ClearCache(true), itemHandle);
            }
            catch (OmmException execp)
            {
                exception = execp;
                Assert.Equal(OmmException.ExceptionType.OmmInvalidUsageException,
                    execp.Type);
                Assert.Equal("Attempt to submit initial RefreshMsg with service name of UNKNOWN_SERVICE_NAME that was not included in" +
                    " the SourceDirectory. Dropping this RefreshMsg.", execp.Message);
            }

            Assert.NotNull(exception);
            exception = null;

            try
            {
                provider?.Submit(new RefreshMsg().ServiceId(999).Name("IBM.N")
                    .State(OmmState.StreamStates.OPEN, OmmState.DataStates.OK, OmmState.StatusCodes.NONE, "UnSolicited Refresh Completed")
                    .MarkForClear().Payload(new FieldList()
                        .AddReal(22, 3990, OmmReal.MagnitudeTypes.EXPONENT_NEG_2)
                        .AddReal(25, 3994, OmmReal.MagnitudeTypes.EXPONENT_NEG_2)
                        .AddReal(30, 0, OmmReal.MagnitudeTypes.EXPONENT_0)
                        .AddReal(31, 19, OmmReal.MagnitudeTypes.EXPONENT_0)
                        .MarkForClear().Complete())
                    .MarkForClear().Complete(true).ClearCache(true), itemHandle);
            }
            catch (OmmException execp)
            {
                exception = execp;
                Assert.Equal(OmmException.ExceptionType.OmmInvalidUsageException,
                    execp.Type);
                Assert.Equal("Attempt to submit initial RefreshMsg with service id of 999 that was not included in" +
                    " the SourceDirectory. Dropping this RefreshMsg.", execp.Message);
            }

            Assert.NotNull(exception);
            provider?.Uninitialize();
            adhSimulator.UnInitialize();
        }

        [Fact]
        public void LoginAndPublishUpdateMsgWithoutServiceNameAndIdTest()
        {
            ProviderSessionOptions providerSessionOpts = new ProviderSessionOptions();
            providerSessionOpts.SendLoginReject = false;
            ADHSimulator adhSimulator = new ADHSimulator(providerSessionOpts, m_Output);
            ReactorOptions reactorOptions = new ReactorOptions();
            adhSimulator.Initialize(reactorOptions);

            string hostString = $"localhost:{adhSimulator.ServerPort}";

            m_Output.WriteLine($"Connect with {hostString}");

            OmmNiProviderConfig config = new OmmNiProviderConfig();
            OmmException? exception = null;
            OmmProvider? provider = null;

            try
            {
                provider = new OmmProvider(config.Host(hostString));
                m_Output.WriteLine("Created OmmProvider successfully");
            }
            catch (OmmException execp)
            {
                exception = execp;
            }

            Assert.Null(exception);

            // Waits until the simulator receives the directory message.
            Thread.Sleep(1000);

            // Remove the channel and login events
            adhSimulator.EventQueue.Clear();

            long itemHandle = 5;

            try
            {
                provider?.Submit(new UpdateMsg().Name("IBM.N")
                .MarkForClear().Payload(new FieldList()
                    .AddReal(22, 3391 + 9, OmmReal.MagnitudeTypes.EXPONENT_NEG_2)
                    .AddReal(30, 10 + 10, OmmReal.MagnitudeTypes.EXPONENT_0)
                    .MarkForClear().Complete()), itemHandle);
            }
            catch (OmmException execp)
            {
                exception = execp;
                Assert.Equal(OmmException.ExceptionType.OmmInvalidUsageException,
                    execp.Type);
                Assert.Equal("Attempt to submit initial UpdateMsg without service name or id. Dropping this UpdateMsg.", execp.Message);
            }

            Assert.NotNull(exception);

            provider?.Uninitialize();
            adhSimulator.UnInitialize();
        }

        [Fact]
        public void LoginAndPublishSourceDirectoryFromFileConfigTest()
        {
            ProviderSessionOptions providerSessionOpts = new ProviderSessionOptions();
            providerSessionOpts.SendLoginReject = false;
            ADHSimulator adhSimulator = new ADHSimulator(providerSessionOpts, m_Output);
            ReactorOptions reactorOptions = new ReactorOptions();
            adhSimulator.Initialize(reactorOptions);

            string hostString = $"localhost:{adhSimulator.ServerPort}";

            m_Output.WriteLine($"Connect with {hostString}");

            OmmNiProviderConfig config = new OmmNiProviderConfig(EMA_FILE_PATH);
            OmmException? exception = null;
            OmmProvider? provider = null;

            try
            {
                provider = new OmmProvider(config.Host(hostString));
                m_Output.WriteLine("Created OmmProvider successfully");
            }
            catch (OmmException ommException)
            {
                exception = ommException;
            }

            Assert.Null(exception);

            // Waits until the simulator receives the directory message.
            Thread.Sleep(1000);

            DirectoryRefresh expectedDirectoryRefresh = new();
            Service service = new();
            {
                service.Info.ServiceName.Data("TEST_NI_PUB");
                service.ServiceId = 11;

                service.Info.HasIsSource = true;
                service.Info.IsSource = 0;

                service.Info.HasSupportQosRange = true;
                service.Info.SupportsQosRange = 0;

                service.Action = MapEntryActions.ADD;

                service.HasInfo = true;
                service.Info.Action = FilterEntryActions.SET;

                service.Info.HasVendor = true;
                service.Info.Vendor.Data("company name");

                service.Info.CapabilitiesList.Add(Ema.Rdm.EmaRdm.MMT_MARKET_PRICE);
                service.Info.CapabilitiesList.Add(Ema.Rdm.EmaRdm.MMT_MARKET_BY_ORDER);
                service.Info.CapabilitiesList.Add(Ema.Rdm.EmaRdm.MMT_MARKET_MAKER);

                service.Info.HasQos = true;
                Qos qos = new Qos();
                qos.Rate(QosRates.TICK_BY_TICK);
                qos.Timeliness(QosTimeliness.REALTIME);
                service.Info.QosList.Add(qos);

                Qos qos2 = new Qos();
                qos2.Rate(QosRates.JIT_CONFLATED);
                qos2.Timeliness(QosTimeliness.DELAYED_UNKNOWN);
                service.Info.QosList.Add(qos2);

                service.Info.HasDictionariesProvided = true;
                service.Info.DictionariesProvidedList.Add(DictionaryCallbackClient<IOmmProviderClient>.DICTIONARY_RWFFID);
                service.Info.DictionariesProvidedList.Add(DictionaryCallbackClient<IOmmProviderClient>.DICTIONARY_RWFENUM);
                service.Info.DictionariesProvidedList.Add(DictionaryCallbackClient<IOmmProviderClient>.DICTIONARY_RWFFID);
                service.Info.DictionariesProvidedList.Add(DictionaryCallbackClient<IOmmProviderClient>.DICTIONARY_RWFENUM);

                service.Info.HasDictionariesUsed = true;
                service.Info.DictionariesUsedList.Add(DictionaryCallbackClient<IOmmProviderClient>.DICTIONARY_RWFFID);
                service.Info.DictionariesUsedList.Add(DictionaryCallbackClient<IOmmProviderClient>.DICTIONARY_RWFENUM);

                service.Info.HasItemList = true;
                service.Info.ItemList.Data("#.itemlist");

                service.Info.HasAcceptingConsStatus = true;
                service.Info.AcceptConsumerStatus = 0;

                service.Info.HasSupportOOBSnapshots = true;
                service.Info.SupportsOOBSnapshots = 0;

                service.HasState = true;
                service.State.Action = FilterEntryActions.SET;
                service.State.ServiceStateVal = 1;
                service.State.HasAcceptingRequests = true;
                service.State.AcceptingRequests = 1;
                service.State.HasStatus = true;
                service.State.Status.StreamState(StreamStates.OPEN);
                service.State.Status.DataState(DataStates.OK);
                service.State.Status.Code(StateCodes.NONE);
                service.State.Status.Text().Data("Status Text 1");

                service.HasLoad = true;
                service.Load.HasOpenLimit = true;
                service.Load.OpenLimit = 4294967295;
                service.Load.HasOpenWindow = true;
                service.Load.OpenWindow = 4294967295;
                service.Load.HasLoadFactor = true;
                service.Load.LoadFactor = 65535;
            }

            Service service2 = new();
            {
                service2.Info.ServiceName.Data("NI_PUB");
                service2.ServiceId = 0; // Generated by API

                service2.Info.HasIsSource = true;
                service2.Info.IsSource = 0;

                service2.Info.HasSupportQosRange = true;
                service2.Info.SupportsQosRange = 0;

                service2.Action = MapEntryActions.ADD;

                service2.HasInfo = true;
                service2.Info.Action = FilterEntryActions.SET;

                service2.Info.HasVendor = true;
                service2.Info.Vendor.Data("company name 2");

                service2.Info.CapabilitiesList.Add(Ema.Rdm.EmaRdm.MMT_MARKET_PRICE);
                service2.Info.CapabilitiesList.Add(Ema.Rdm.EmaRdm.MMT_MARKET_BY_ORDER);

                service2.Info.HasQos = true;
                Qos qos = new Qos();
                qos.Rate(QosRates.TICK_BY_TICK);
                qos.Timeliness(QosTimeliness.REALTIME);
                service2.Info.QosList.Add(qos);

                service2.Info.HasDictionariesProvided = true;
                service2.Info.DictionariesProvidedList.Add(DictionaryCallbackClient<IOmmProviderClient>.DICTIONARY_RWFFID);
                service2.Info.DictionariesProvidedList.Add(DictionaryCallbackClient<IOmmProviderClient>.DICTIONARY_RWFENUM);

                service2.Info.HasDictionariesUsed = true;
                service2.Info.DictionariesUsedList.Add(DictionaryCallbackClient<IOmmProviderClient>.DICTIONARY_RWFFID);
                service2.Info.DictionariesUsedList.Add(DictionaryCallbackClient<IOmmProviderClient>.DICTIONARY_RWFENUM);

                service2.Info.HasItemList = true;
                service2.Info.ItemList.Data("#.itemlist");

                service2.Info.HasAcceptingConsStatus = true;
                service2.Info.AcceptConsumerStatus = 0;

                service2.Info.HasSupportOOBSnapshots = true;
                service2.Info.SupportsOOBSnapshots = 0;

                service2.HasState = true;
                service2.State.Action = FilterEntryActions.SET;
                service2.State.ServiceStateVal = 1;
                service2.State.HasAcceptingRequests = true;
                service2.State.AcceptingRequests = 1;
                service2.State.HasStatus = true;
                service2.State.Status.StreamState(StreamStates.OPEN);
                service2.State.Status.DataState(DataStates.OK);
                service2.State.Status.Code(StateCodes.NONE);
                service2.State.Status.Text().Data("Status Text 2");

                service2.HasLoad = true;
                service2.Load.HasOpenLimit = true;
                service2.Load.OpenLimit = 4294967295;
                service2.Load.HasOpenWindow = true;
                service2.Load.OpenWindow = 4294967295;
                service2.Load.HasLoadFactor = true;
                service2.Load.LoadFactor = 65535;
            }

            expectedDirectoryRefresh.ServiceList.Add(service);
            expectedDirectoryRefresh.ServiceList.Add(service2);

            var refreshState = new State();
            refreshState.StreamState(StreamStates.OPEN);
            refreshState.DataState(DataStates.OK);
            refreshState.Code(StateCodes.NONE);
            refreshState.Text().Data("Source Directory Refresh Completed");
            expectedDirectoryRefresh.State = refreshState;

            // Sets by EMA's NiProvider before submitting to ADH.
            expectedDirectoryRefresh.ClearCache = true;
            expectedDirectoryRefresh.Filter = ServiceFilterFlags.INFO | ServiceFilterFlags.STATE;

            // Checks the hard coded source directory in ADHSimulator
            DirectoryRefresh sourceDirectoryCache = adhSimulator.DirectoryCache;

            Assert.Equal(expectedDirectoryRefresh.ToString(), sourceDirectoryCache.ToString());

            Assert.Null(exception);
            provider?.Uninitialize();
            adhSimulator.UnInitialize();
        }

        [Fact]
        public void LoginAndPublishSourceDirectoryFromUserTest()
        {
            ProviderSessionOptions providerSessionOpts = new ProviderSessionOptions();
            providerSessionOpts.SendLoginReject = false;
            ADHSimulator adhSimulator = new ADHSimulator(providerSessionOpts, m_Output);
            ReactorOptions reactorOptions = new ReactorOptions();
            adhSimulator.Initialize(reactorOptions);

            string hostString = $"localhost:{adhSimulator.ServerPort}";

            m_Output.WriteLine($"Connect with {hostString}");

            OmmNiProviderConfig config = new OmmNiProviderConfig();
            OmmException? exception = null;
            OmmProvider? provider = null;

            try
            {
                provider = new OmmProvider(config.Host(hostString).AdminControlDirectory(OmmNiProviderConfig.AdminControlMode.USER_CONTROL));
                m_Output.WriteLine("Created OmmProvider successfully");

                ulong serviceId = 555;
                long sourceDirectoryHandle = 1;
                RefreshMsg refresh = new();
                Map map = new();

                map.AddKeyUInt(serviceId, MapAction.ADD, new FilterList()
                        .AddEntry(EmaRdm.SERVICE_INFO_FILTER, FilterAction.SET, new ElementList()
                            .AddAscii(EmaRdm.ENAME_NAME, "USER_NI_PUB")
                            .AddAscii(EmaRdm.ENAME_VENDOR, "test user company")
                            .AddArray(EmaRdm.ENAME_CAPABILITIES, new OmmArray()
                                .AddUInt(EmaRdm.MMT_MARKET_PRICE)
                                .AddUInt(EmaRdm.MMT_MARKET_BY_PRICE)
                                .MarkForClear().Complete())
                            .AddArray(EmaRdm.ENAME_DICTIONARYS_USED, new OmmArray()
                                .AddAscii("RWFFld_user_used")
                                .AddAscii("RWFEnum_user_used")
                                .MarkForClear().Complete())
                            .AddArray(EmaRdm.ENAME_DICTIONARYS_PROVIDED, new OmmArray()
                                .AddAscii("RWFFld_user_provided")
                                .AddAscii("RWFEnum_user_provided")
                                .MarkForClear().Complete())
                            .AddUInt(EmaRdm.ENAME_LINK_STATE, 0)
                            .AddUInt(EmaRdm.ENAME_SUPPS_QOS_RANGE, 0)
                            .AddArray(EmaRdm.ENAME_QOS, new OmmArray()
                                .AddQos(OmmQos.Timelinesses.REALTIME, OmmQos.Rates.TICK_BY_TICK)
                                .AddQos(OmmQos.Timelinesses.INEXACT_DELAYED, OmmQos.Rates.JUST_IN_TIME_CONFLATED)
                                .MarkForClear().Complete())
                            .AddAscii(EmaRdm.ENAME_ITEM_LIST, "#.itemlist_user")
                            .AddUInt(EmaRdm.ENAME_ACCEPTING_CONS_STATUS, 0)
                            .AddUInt(EmaRdm.ENAME_SUPPS_OOB_SNAPSHOTS, 0)
                            .MarkForClear().Complete())
                        .AddEntry(EmaRdm.SERVICE_STATE_ID, FilterAction.SET, new ElementList()
                            .AddUInt(EmaRdm.ENAME_SVC_STATE, EmaRdm.SERVICE_UP)
                            .AddUInt(EmaRdm.ENAME_ACCEPTING_REQS, 1)
                            .AddState(EmaRdm.ENAME_STATUS, OmmState.StreamStates.OPEN, OmmState.DataStates.OK, OmmState.StatusCodes.NONE, "User Submitted Status Text 2")
                            .MarkForClear().Complete())
                        .AddEntry(EmaRdm.SERVICE_LOAD_ID, FilterAction.SET, new ElementList()
                            .AddUInt(EmaRdm.ENAME_OPEN_LIMIT, 100)
                            .AddUInt(EmaRdm.ENAME_OPEN_WINDOW, 50)
                            .AddUInt(EmaRdm.ENAME_LOAD_FACT, 30)
                            .MarkForClear().Complete())
                        .MarkForClear().Complete())
                    .MarkForClear().Complete();

                provider.Submit(refresh.DomainType(EmaRdm.MMT_DIRECTORY).Filter(EmaRdm.SERVICE_INFO_FILTER | EmaRdm.SERVICE_STATE_FILTER).ClearCache(true)
                    .MarkForClear().Payload(map).State(OmmState.StreamStates.OPEN, OmmState.DataStates.OK, OmmState.StatusCodes.NONE, "Source Directory Refresh Completed")
                    .MarkForClear().Complete(true), sourceDirectoryHandle);

            }
            catch (OmmException ommException)
            {
                exception = ommException;
            }

            Assert.Null(exception);

            // Waits until the simulator receives the directory message.
            Thread.Sleep(3000);

            DirectoryRefresh expectedDirectoryRefresh = new();
            Service service = new();
            {
                service.Info.ServiceName.Data("USER_NI_PUB");
                service.ServiceId = 555;

                service.Info.HasIsSource = true;
                service.Info.IsSource = 0;

                service.Info.HasSupportQosRange = true;
                service.Info.SupportsQosRange = 0;

                service.Action = MapEntryActions.ADD;

                service.HasInfo = true;
                service.Info.Action = FilterEntryActions.SET;

                service.Info.HasVendor = true;
                service.Info.Vendor.Data("test user company");

                service.Info.CapabilitiesList.Add(Ema.Rdm.EmaRdm.MMT_MARKET_PRICE);
                service.Info.CapabilitiesList.Add(Ema.Rdm.EmaRdm.MMT_MARKET_BY_PRICE);

                service.Info.HasQos = true;
                Qos qos = new Qos();
                qos.Rate(QosRates.TICK_BY_TICK);
                qos.Timeliness(QosTimeliness.REALTIME);
                service.Info.QosList.Add(qos);
                Qos qos2 = new Qos();
                qos2.Rate(QosRates.JIT_CONFLATED);
                qos2.Timeliness(QosTimeliness.DELAYED_UNKNOWN);
                service.Info.QosList.Add(qos2);

                service.Info.HasDictionariesProvided = true;
                service.Info.DictionariesProvidedList.Add("RWFFld_user_provided");
                service.Info.DictionariesProvidedList.Add("RWFEnum_user_provided");

                service.Info.HasDictionariesUsed = true;
                service.Info.DictionariesUsedList.Add("RWFFld_user_used");
                service.Info.DictionariesUsedList.Add("RWFEnum_user_used");

                service.Info.HasItemList = true;
                service.Info.ItemList.Data("#.itemlist_user");

                service.Info.HasAcceptingConsStatus = true;
                service.Info.AcceptConsumerStatus = 0;

                service.Info.HasSupportOOBSnapshots = true;
                service.Info.SupportsOOBSnapshots = 0;

                service.HasState = true;
                service.State.Action = FilterEntryActions.SET;
                service.State.ServiceStateVal = 1;
                service.State.HasAcceptingRequests = true;
                service.State.AcceptingRequests = 1;
                service.State.HasStatus = true;
                service.State.Status.StreamState(StreamStates.OPEN);
                service.State.Status.DataState(DataStates.OK);
                service.State.Status.Code(StateCodes.NONE);
                service.State.Status.Text().Data("User Submitted Status Text 2");

                service.HasLoad = true;
                service.Load.HasOpenLimit = true;
                service.Load.OpenLimit = 100;
                service.Load.HasOpenWindow = true;
                service.Load.OpenWindow = 50;
                service.Load.HasLoadFactor = true;
                service.Load.LoadFactor = 30;
            }
            expectedDirectoryRefresh.ServiceList.Add(service);

            var refreshState = new State();
            refreshState.StreamState(StreamStates.OPEN);
            refreshState.DataState(DataStates.OK);
            refreshState.Code(StateCodes.NONE);
            refreshState.Text().Data("Source Directory Refresh Completed");
            expectedDirectoryRefresh.State = refreshState;

            // Sets by EMA's NiProvider before submitting to ADH.
            expectedDirectoryRefresh.ClearCache = true;
            expectedDirectoryRefresh.Filter = ServiceFilterFlags.INFO | ServiceFilterFlags.STATE;

            // Checks the hard coded source directory in ADHSimulator
            DirectoryRefresh sourceDirectoryCache = adhSimulator.DirectoryCache;

            Assert.Equal(expectedDirectoryRefresh.ToString(), sourceDirectoryCache.ToString());


            Assert.Null(exception);
            provider?.Uninitialize();
            adhSimulator.UnInitialize();
        }

        [Theory]
        [InlineData(false)]
        [InlineData(true)]
        public void RegisterAndUnRegisterLoginReqTest(bool userDispatch = false)
        {
            ProviderSessionOptions providerSessionOpts = new ProviderSessionOptions();
            providerSessionOpts.SendLoginReject = false;
            ADHSimulator adhSimulator = new ADHSimulator(providerSessionOpts, m_Output);
            ReactorOptions reactorOptions = new ReactorOptions();
            adhSimulator.Initialize(reactorOptions);

            string hostString = $"localhost:{adhSimulator.ServerPort}";

            m_Output.WriteLine($"Connect with {hostString}");

            OmmNiProviderConfig config = new OmmNiProviderConfig();
            OmmException? exception = null;
            OmmProvider? provider = null;

            OmmProviderItemClientTest providerClient;

            try
            {
                config.Host(hostString);

                if (userDispatch)
                    config.OperationModel(OmmNiProviderConfig.OperationModelMode.USER_DISPATCH);

                provider = new OmmProvider(config.Host(hostString));

                providerClient = new OmmProviderItemClientTest(provider);

                RequestMsg requestMsg = new();
                requestMsg.DomainType(EmaRdm.MMT_LOGIN);

                long handle = provider.RegisterClient(requestMsg, providerClient, this);

                Assert.True(handle != 0);

                /* Checks the expected RefreshMsg from the login request */
                providerClient.RefreshMsgHandler = (refreshMsg, consEvent) =>
                {
                    Assert.Equal(handle, consEvent.Handle);

                    Assert.Equal(DataType.DataTypes.NO_DATA, refreshMsg.MarkForClear().Payload().DataType);

                    Assert.True(refreshMsg.HasMsgKey);
                    Assert.True(refreshMsg.HasNameType);
                    Assert.Equal((int)Eta.Rdm.Login.UserIdTypes.NAME, refreshMsg.NameType());

                    Assert.True(refreshMsg.HasName);
                    Assert.Equal(Environment.UserName, refreshMsg.Name());

                    Assert.Equal(OmmState.StatusCodes.NONE, refreshMsg.State().StatusCode);
                    Assert.Equal(OmmState.DataStates.OK, refreshMsg.State().DataState);
                    Assert.Equal(OmmState.StreamStates.OPEN, refreshMsg.State().StreamState);
                    Assert.Equal("Login accepted by host localhost", refreshMsg.State().StatusText);

                    Assert.True(refreshMsg.Solicited());

                    Assert.True(refreshMsg.HasAttrib);
                    Assert.Equal(DataType.DataTypes.ELEMENT_LIST, refreshMsg.Attrib().DataType);

                    ElementList elementList = refreshMsg.Attrib().ElementList();

                    var enumerator = elementList.GetEnumerator();
                    Assert.True(enumerator.MoveNext());
                    var entry = enumerator.Current;

                    Assert.Equal(DataType.DataTypes.ASCII, entry.LoadType);
                    Assert.Equal("ApplicationId", entry.Name);
                    Assert.Equal("256", entry.OmmAsciiValue().Value);

                    Assert.True(enumerator.MoveNext());
                    entry = enumerator.Current;

                    Assert.Equal(DataType.DataTypes.ASCII, entry.LoadType);
                    Assert.Equal("ApplicationName", entry.Name);
                    Assert.Equal("ETA Provider Test", entry.OmmAsciiValue().Value);

                    Assert.True(enumerator.MoveNext());
                    entry = enumerator.Current;

                    string hostName = Dns.GetHostName();
                    string position = Dns.GetHostAddresses(hostName).Where(ip => ip.AddressFamily == System.Net.Sockets.AddressFamily.InterNetwork).FirstOrDefault() + "/"
                            + hostName;

                    Assert.Equal(DataType.DataTypes.ASCII, entry.LoadType);
                    Assert.Equal("Position", entry.Name);
                    Assert.Equal(position, entry.OmmAsciiValue().Value);

                    Assert.True(enumerator.MoveNext());
                    entry = enumerator.Current;

                    Assert.Equal(DataType.DataTypes.UINT, entry.LoadType);
                    Assert.Equal("SingleOpen", entry.Name);
                    Assert.Equal<ulong>(0, entry.OmmUIntValue().Value);

                    Assert.True(enumerator.MoveNext());
                    entry = enumerator.Current;

                    Assert.Equal(DataType.DataTypes.UINT, entry.LoadType);
                    Assert.Equal("SupportOMMPost", entry.Name);
                    Assert.Equal<ulong>(1, entry.OmmUIntValue().Value);

                    Assert.True(enumerator.MoveNext());
                    entry = enumerator.Current;

                    Assert.Equal(DataType.DataTypes.UINT, entry.LoadType);
                    Assert.Equal("SupportBatchRequests", entry.Name);
                    Assert.Equal<ulong>(1, entry.OmmUIntValue().Value);

                    Assert.False(enumerator.MoveNext());
                };

                if (userDispatch)
                {
                    provider.Dispatch(DispatchTimeout.INFINITE_WAIT);
                }
                else
                {
                    Thread.Sleep(1000);
                }

                Assert.Equal(1, providerClient.ReceivedOnAll);
                Assert.Equal(1, providerClient.ReceivedOnRefresh);
                Assert.Equal(0, providerClient.ReceivedOnStatus);

                provider.Unregister(handle);
            }
            catch (OmmException ommException)
            {
                exception = ommException;
            }

            Assert.Null(exception);
            provider?.Uninitialize();
            adhSimulator.UnInitialize();
        }

        [Theory]
        [InlineData(false)]
        [InlineData(true)]
        public void GetChanelInformationTest(bool userDispatch = false)
        {
            ProviderSessionOptions providerSessionOpts = new ProviderSessionOptions();
            providerSessionOpts.SendLoginReject = false;
            ADHSimulator adhSimulator = new ADHSimulator(providerSessionOpts, m_Output);
            ReactorOptions reactorOptions = new ReactorOptions();
            adhSimulator.Initialize(reactorOptions);

            string hostString = $"localhost:{adhSimulator.ServerPort}";

            m_Output.WriteLine($"Connect with {hostString}");

            OmmNiProviderConfig config = new OmmNiProviderConfig();
            OmmException? exception = null;
            OmmProvider? provider = null;

            OmmProviderItemClientTest providerClient;

            string expectedConnectedComponentInfo = Transport.QueryVersion().ProductInternalVersion();
            string expectedChannelInfoStr =
                $"hostname: localhost{NewLine}\tIP address: not available for OmmNiProvider connections{NewLine}\tport: {adhSimulator.ServerPort}" +
                $"{NewLine}\tconnected component info: {expectedConnectedComponentInfo}{NewLine}\tchannel state: active{NewLine}\tconnection type: SOCKET{NewLine}\tprotocol type: Rssl wire format" +
                $"{NewLine}\tmajor version: 14{NewLine}\tminor version: 1{NewLine}\tping timeout: 60{NewLine}\tmax fragmentation size: 6142{NewLine}\tmax output buffers: 100{NewLine}\tguaranteed output buffers: 100" +
                $"{NewLine}\tnumber input buffers: 100{NewLine}\tsystem send buffer size: 0{NewLine}\tsystem receive buffer size: 0{NewLine}\tcompression type: none{NewLine}\tcompression threshold: 0" +
                $"{NewLine}\tencryptionProtocol: None";

            try
            {
                config.Host(hostString);

                if (userDispatch)
                    config.OperationModel(OmmNiProviderConfig.OperationModelMode.USER_DISPATCH);

                provider = new OmmProvider(config.Host(hostString));

                ChannelInformation channelInfo = new ChannelInformation();

                provider.ChannelInformation(channelInfo);

                Assert.Equal(expectedChannelInfoStr, channelInfo.ToString());

                providerClient = new OmmProviderItemClientTest(provider);

                RequestMsg requestMsg = new();
                requestMsg.DomainType(EmaRdm.MMT_LOGIN);

                long handle = provider.RegisterClient(requestMsg, providerClient, this);

                Assert.True(handle != 0);

                /* Checks the expected RefreshMsg from the login request */
                providerClient.RefreshMsgHandler = (refreshMsg, consEvent) =>
                {
                    Assert.Equal(expectedChannelInfoStr, consEvent.ChannelInformation().ToString());
                };

                if (userDispatch)
                {
                    provider.Dispatch(DispatchTimeout.INFINITE_WAIT);
                }
                else
                {
                    Thread.Sleep(1000);
                }

                Assert.Equal(1, providerClient.ReceivedOnAll);
                Assert.Equal(1, providerClient.ReceivedOnRefresh);
                Assert.Equal(0, providerClient.ReceivedOnStatus);

                provider.Unregister(handle);
            }
            catch (OmmException ommException)
            {
                exception = ommException;
            }

            Assert.Null(exception);
            provider?.Uninitialize();
            adhSimulator.UnInitialize();
        }

        [Fact]
        public void SubmitRefreshWithLoginHandle()
        {
            ProviderSessionOptions providerSessionOpts = new ProviderSessionOptions();
            providerSessionOpts.SendLoginReject = false;
            ADHSimulator adhSimulator = new ADHSimulator(providerSessionOpts, m_Output);
            ReactorOptions reactorOptions = new ReactorOptions();
            adhSimulator.Initialize(reactorOptions);

            string hostString = $"localhost:{adhSimulator.ServerPort}";

            m_Output.WriteLine($"Connect with {hostString}");

            OmmNiProviderConfig config = new OmmNiProviderConfig();
            OmmException? exception = null;
            OmmProvider? provider = null;

            OmmProviderItemClientTest providerClient;
            long loginhandle = 0;

            try
            {
                config.Host(hostString);

                config.OperationModel(OmmNiProviderConfig.OperationModelMode.USER_DISPATCH);

                provider = new OmmProvider(config.Host(hostString));

                providerClient = new OmmProviderItemClientTest(provider);

                RequestMsg requestMsg = new();
                requestMsg.DomainType(EmaRdm.MMT_LOGIN);

                loginhandle = provider.RegisterClient(requestMsg, providerClient, this);

                Assert.True(loginhandle != 0);

                provider.Dispatch(DispatchTimeout.INFINITE_WAIT);

                Assert.Equal(1, providerClient.ReceivedOnAll);
                Assert.Equal(1, providerClient.ReceivedOnRefresh);
                Assert.Equal(0, providerClient.ReceivedOnStatus);

                // Submit a refresh message using the login handle
                provider.Submit(new RefreshMsg().ServiceName("NI_PUB").Name("IBM.N")
                .State(OmmState.StreamStates.OPEN, OmmState.DataStates.OK, OmmState.StatusCodes.NONE, "UnSolicited Refresh Completed")
                .MarkForClear().Payload(new FieldList()
                    .AddReal(22, 3990, OmmReal.MagnitudeTypes.EXPONENT_NEG_2)
                    .AddReal(25, 3994, OmmReal.MagnitudeTypes.EXPONENT_NEG_2)
                    .AddReal(30, 0, OmmReal.MagnitudeTypes.EXPONENT_0)
                    .AddReal(31, 19, OmmReal.MagnitudeTypes.EXPONENT_0)
                    .MarkForClear().Complete())
                .MarkForClear().Complete(true).ClearCache(true), loginhandle);

                provider.Unregister(loginhandle);
            }
            catch (OmmException ommException)
            {
                exception = ommException;
                Assert.Equal(OmmException.ExceptionType.OmmInvalidHandleException, exception.Type);
                Assert.Equal("Attempt to submit( RefreshMsg ) using a registered handle.", exception.Message);
            }

            Assert.NotNull(exception);
            exception = null;

            // Able to submit after the login stream is unregistered.
            provider!.Unregister(loginhandle);

            try
            {
                // Submit a refresh message using the login handle
                provider!.Submit(new RefreshMsg().ServiceName("NI_PUB").Name("IBM.N")
                .State(OmmState.StreamStates.OPEN, OmmState.DataStates.OK, OmmState.StatusCodes.NONE, "UnSolicited Refresh Completed")
                .MarkForClear().Payload(new FieldList()
                    .AddReal(22, 3990, OmmReal.MagnitudeTypes.EXPONENT_NEG_2)
                    .AddReal(25, 3994, OmmReal.MagnitudeTypes.EXPONENT_NEG_2)
                    .AddReal(30, 0, OmmReal.MagnitudeTypes.EXPONENT_0)
                    .AddReal(31, 19, OmmReal.MagnitudeTypes.EXPONENT_0)
                    .MarkForClear().Complete())
                .MarkForClear().Complete(true).ClearCache(true), loginhandle);
            }
            catch (OmmException ommException)
            {
                exception = ommException;
            }

            Assert.Null(exception);

            provider?.Uninitialize();
            adhSimulator.UnInitialize();
        }

        [Theory]
        [InlineData(false)]
        [InlineData(true)]
        public void RegisterAndUnRegisterDictionaryReqsTest(bool userDispatch = false)
        {
            ProviderSessionOptions providerSessionOpts = new ProviderSessionOptions();
            providerSessionOpts.SendLoginReject = false;
            providerSessionOpts.SendDictionaryResp = true;
            ADHSimulator adhSimulator = new ADHSimulator(providerSessionOpts, m_Output);
            ReactorOptions reactorOptions = new ReactorOptions();
            adhSimulator.Initialize(reactorOptions);

            string hostString = $"localhost:{adhSimulator.ServerPort}";

            m_Output.WriteLine($"Connect with {hostString}");

            OmmNiProviderConfig config = new OmmNiProviderConfig();
            OmmProvider? provider = null;

            OmmProviderItemClientTest providerFldClient;
            OmmProviderItemClientTest providerEnumClient;

            try
            {
                config.Host(hostString);

                if (userDispatch)
                    config.OperationModel(OmmNiProviderConfig.OperationModelMode.USER_DISPATCH);

                provider = new OmmProvider(config.Host(hostString));

                providerFldClient = new OmmProviderItemClientTest(provider);

                RequestMsg requestMsg = new();
                requestMsg.DomainType(EmaRdm.MMT_DICTIONARY).Name("RWFFld").ServiceName("NI_PUB")
                    .Filter(EmaRdm.DICTIONARY_NORMAL);

                long fldHandle = provider.RegisterClient(requestMsg, providerFldClient, this);

                Assert.True(fldHandle != 0);

                bool fldComplelte = false, enumComplete = false;

                /* Checks the expected RefreshMsg from the RWFFld request */
                providerFldClient.RefreshMsgHandler = (refreshMsg, consEvent) =>
                {
                    Assert.Equal(fldHandle, consEvent.Handle);

                    Assert.Equal(DataType.DataTypes.SERIES, refreshMsg.MarkForClear().Payload().DataType);

                    Assert.True(refreshMsg.HasMsgKey);
                    Assert.False(refreshMsg.HasNameType);

                    Assert.True(refreshMsg.HasName);
                    Assert.Equal("RWFFld", refreshMsg.Name());

                    Assert.True(refreshMsg.HasServiceName);
                    Assert.Equal("NI_PUB", refreshMsg.ServiceName());

                    Assert.Equal(OmmState.StatusCodes.NONE, refreshMsg.State().StatusCode);
                    Assert.Equal(OmmState.DataStates.OK, refreshMsg.State().DataState);
                    Assert.Equal(OmmState.StreamStates.OPEN, refreshMsg.State().StreamState);

                    Assert.True(refreshMsg.Solicited());

                    fldComplelte = refreshMsg.MarkForClear().Complete();
                };

                providerEnumClient = new OmmProviderItemClientTest(provider);

                requestMsg.Clear();
                requestMsg.DomainType(EmaRdm.MMT_DICTIONARY).Name("RWFEnum").ServiceName("NI_PUB")
                    .Filter(EmaRdm.DICTIONARY_NORMAL);

                long enumHandle = provider.RegisterClient(requestMsg, providerEnumClient, this);

                Assert.True(enumHandle != 0);

                /* Checks the expected RefreshMsg from the RWFFld request */
                providerEnumClient.RefreshMsgHandler = (refreshMsg, consEvent) =>
                {
                    Assert.Equal(enumHandle, consEvent.Handle);

                    Assert.Equal(DataType.DataTypes.SERIES, refreshMsg.MarkForClear().Payload().DataType);

                    Assert.True(refreshMsg.HasMsgKey);
                    Assert.False(refreshMsg.HasNameType);

                    Assert.True(refreshMsg.HasName);
                    Assert.Equal("RWFEnum", refreshMsg.Name());

                    Assert.True(refreshMsg.HasServiceName);
                    Assert.Equal("NI_PUB", refreshMsg.ServiceName());

                    Assert.Equal(OmmState.StatusCodes.NONE, refreshMsg.State().StatusCode);
                    Assert.Equal(OmmState.DataStates.OK, refreshMsg.State().DataState);
                    Assert.Equal(OmmState.StreamStates.OPEN, refreshMsg.State().StreamState);

                    Assert.True(refreshMsg.Solicited());
                    enumComplete = refreshMsg.MarkForClear().Complete();
                };

                if (userDispatch)
                {
                    for (int i = 0; i < 50; i++)
                        provider.Dispatch(1000000);
                }
                else
                {
                    Thread.Sleep(10000);
                }

                Assert.True(fldComplelte && enumComplete);

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

        /// <summary>
        /// Various combinations of IOCtl codes and their new values used for testing.
        /// </summary>
        /// Each entry in this list corresponds to a separate test run.
        public static IEnumerable<object[]> IOCtlSettings => new List<object[]>
        {
            new object[]
            {
                new IOCtlSetting[]
                {
                    new IOCtlSetting() { Code = IOCtlCode.MAX_NUM_BUFFERS, Setting = -10, IsValid = false,
                        ErrorCode = OmmInvalidUsageException.ErrorCodes.FAILURE }
                }
            },

            new object[]
            {
                new IOCtlSetting[]
                {
                    new IOCtlSetting() { Code = IOCtlCode.MAX_NUM_BUFFERS, Setting = 100 }
                }
            },

            new object[]
            {
                new IOCtlSetting[]
                {
                    new IOCtlSetting() { Code = IOCtlCode.MAX_NUM_BUFFERS, Setting = 200 },
                    new IOCtlSetting() { Code = IOCtlCode.NUM_GUARANTEED_BUFFERS, Setting = 200 }
                }
            },

            new object[]
            {
                new IOCtlSetting[]
                {
                    new IOCtlSetting() {Code = IOCtlCode.HIGH_WATER_MARK, Setting = 10 }
                }
            },

            new object[]
            {
                new IOCtlSetting[]
                {
                    new IOCtlSetting() {
                        Code = IOCtlCode.MAX_NUM_BUFFERS,
                        Setting = -10,
                        IsValid = false,
                        Message = "Failed to modify I/O option = MAX_NUM_BUFFERS. Reason: FAILURE.",
                        ErrorCode = OmmInvalidUsageException.ErrorCodes.FAILURE
                    },
                    new IOCtlSetting() {Code = IOCtlCode.NUM_GUARANTEED_BUFFERS, Setting = 10 },
                    new IOCtlSetting() {Code = IOCtlCode.HIGH_WATER_MARK, Setting = 10 },
                    new IOCtlSetting() {Code = IOCtlCode.SYSTEM_READ_BUFFERS, Setting = 10 },
                    new IOCtlSetting() {Code = IOCtlCode.SYSTEM_WRITE_BUFFERS, Setting = 10 },
                    new IOCtlSetting() {Code = IOCtlCode.COMPRESSION_THRESHOLD, Setting = 10, IsValid = false }
                }
            },

            new object[]
            {
                new IOCtlSetting[]
                {
                    // The compression must be enabled between provider and consumer in order
                    // to modify the COMPRESSION_THRESHOLD code. The code below is valid as it
                    // is greater than the default value for LZ4_COMPRESSION_THRESHOLD.
                    new IOCtlSetting() {
                        Code = IOCtlCode.COMPRESSION_THRESHOLD,
                        Setting = 350,
                        IsValid = true
                    }
                }
            }
        };

        /// <summary>
        /// Modifying IOCtl codes as provided in the <paramref name="settings"/>.
        /// </summary>
        /// <param name="settings">Array of <see cref="IOCtlSetting"/> entries defining IOCtl modifications</param>
        [Theory]
        [MemberData(nameof(IOCtlSettings))]
        public void ModifyIOCtlCodes_Test(IOCtlSetting[] settings)
        {
            ProviderSessionOptions providerSessionOpts = new ProviderSessionOptions();
            providerSessionOpts.SendLoginReject = false;
            providerSessionOpts.CompressionType = Eta.Transports.CompressionType.LZ4;
            ADHSimulator adhSimulator = new ADHSimulator(providerSessionOpts, m_Output);
            ReactorOptions reactorOptions = new ReactorOptions();
            adhSimulator.Initialize(reactorOptions);

            string hostString = $"localhost:{adhSimulator.ServerPort}";

            m_Output.WriteLine($"Connect with {hostString}");

            OmmNiProviderConfig config = new OmmNiProviderConfig();
            OmmException? exception = null;
            OmmProvider? provider = null;

            try
            {
                provider = new OmmProvider(config.Host(hostString));
                m_Output.WriteLine("Created OmmProvider successfully");

                // modify IOCtl codes as defined in test parameters
                foreach (IOCtlSetting setting in settings)
                {
                    m_Output.WriteLine($"Modify IOCtl: {setting.Code}, new value: {setting.Setting}");
                    // some settings are known to be invalid and are expected to raise an exception
                    if (setting.IsValid)
                    {
                        provider.ModifyIOCtl(setting.Code, setting.Setting);
                    }
                    else
                    {
                        OmmInvalidUsageException ex = Assert.Throws<OmmInvalidUsageException>(
                            () => provider.ModifyIOCtl(setting.Code, setting.Setting));

                        Assert.NotEmpty(ex.Message);
                        if (setting.Message is not null)
                            Assert.Contains(setting.Message, ex.Message);

                        if (setting.ErrorCode is not null)
                            Assert.Equal(setting.ErrorCode, ex.ErrorCode);
                    }
                }
            }
            catch (OmmException ommException)
            {
                exception = ommException;
            }

            Assert.Null(exception);
            provider?.Uninitialize();
            adhSimulator.UnInitialize();
        }

        class OmmProviderErrorClientTest : IOmmProviderErrorClient
        {
            public List<OmmInvalidHandleException> InvalidHandleExceptionList { get; private set; } = new();

            public List<OmmInvalidUsageException> InvalidUsageExceptionList { get; private set; } = new();

            public void OnInvalidHandle(long handle, string text)
            {
                InvalidHandleExceptionList.Add(new OmmInvalidHandleException(handle, text));
            }

            public void OnInvalidUsage(string text, int errorCode)
            {
                InvalidUsageExceptionList.Add(new OmmInvalidUsageException(text, errorCode));
            }
        }

        [Fact]
        public void NotifyWithIOmmProviderErrorClientTest()
        {
            ProviderSessionOptions providerSessionOpts = new ProviderSessionOptions();
            providerSessionOpts.SendLoginReject = false;
            ADHSimulator adhSimulator = new ADHSimulator(providerSessionOpts, m_Output);
            ReactorOptions reactorOptions = new ReactorOptions();
            adhSimulator.Initialize(reactorOptions);

            string hostString = $"localhost:{adhSimulator.ServerPort}";

            m_Output.WriteLine($"Connect with {hostString}");

            OmmNiProviderConfig config = new OmmNiProviderConfig();
            OmmException? exception = null;
            OmmProvider? provider = null;

            long invalidHandle = 100;

            try
            {
                config.Host(hostString);

                var providerErrorClient = new OmmProviderErrorClientTest();

                provider = new OmmProvider(config.Host(hostString), providerErrorClient);

                long handle = provider.RegisterClient(null!, null!);

                Assert.Equal(0, handle);

                provider.Submit(new GenericMsg(), invalidHandle);

                provider.Reissue(new RequestMsg(), invalidHandle);

                provider.Reissue(new RequestMsg().DomainType(EmaRdm.MMT_LOGIN), invalidHandle);

                /* Checks OmmInvalidUsageException */
                Assert.True(providerErrorClient.InvalidUsageExceptionList.Count == 2);
                Assert.Equal("A derived class of IOmmProviderClient is not set",
                    providerErrorClient.InvalidUsageExceptionList[0].Message);
                Assert.Equal(OmmInvalidUsageException.ErrorCodes.INVALID_ARGUMENT,
                    providerErrorClient.InvalidUsageExceptionList[0].ErrorCode);

                Assert.Equal("OMM Non-Interactive provider supports reissuing LOGIN and DICTIONARY domain type only.",
                    providerErrorClient.InvalidUsageExceptionList[1].Message);
                Assert.Equal(OmmInvalidUsageException.ErrorCodes.INVALID_ARGUMENT,
                    providerErrorClient.InvalidUsageExceptionList[1].ErrorCode);

                /* Checks OmmInvalidHandleException */
                Assert.True(providerErrorClient.InvalidHandleExceptionList.Count == 2);
                Assert.Equal(invalidHandle, providerErrorClient.InvalidHandleExceptionList[0].Handle);
                Assert.Equal("Attempt to submit GenericMsg on stream that is not open yet. Handle = 100.",
                    providerErrorClient.InvalidHandleExceptionList[0].Message);

                Assert.Equal(invalidHandle, providerErrorClient.InvalidHandleExceptionList[1].Handle);
                Assert.StartsWith("Attempt to use invalid Handle on Reissue(). Instance name='DefaultEmaNiProvider_",
                    providerErrorClient.InvalidHandleExceptionList[1].Message);
            }
            catch (OmmException ommException)
            {
                exception = ommException;
            }

            Assert.Null(exception);

            provider?.Uninitialize();
            adhSimulator.UnInitialize();
        }
    }
}
