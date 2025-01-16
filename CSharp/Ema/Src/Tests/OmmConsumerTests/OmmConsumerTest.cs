/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2023-2025 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

using LSEG.Eta.Codec;
using LSEG.Eta.Rdm;
using LSEG.Eta.ValueAdd.Reactor;
using System;
using System.Linq;
using System.Net;
using System.Threading;
using Xunit.Abstractions;
using static LSEG.Eta.Rdm.Directory;

using LSEG.Ema.Rdm;
using System.Collections.Generic;

namespace LSEG.Ema.Access.Tests.OmmConsumerTests
{

    public class OmmConsumerTest : IDisposable
    {
        public void Dispose()
        {
            EtaGlobalPoolTestUtil.Clear();
        }

        ITestOutputHelper output;

        public OmmConsumerTest(ITestOutputHelper output)
        {
            this.output = output;

        }

        internal static void CheckRefreshPayload(ProviderTest providerTest, int itemIndex, RefreshMsg refreshMsg, 
            string itemName, State? expectedState = null, HashSet<int>? fidSet = null)
        {
            Assert.Equal((int)DomainType.MARKET_PRICE, refreshMsg.DomainType());
            Assert.True(refreshMsg.ClearCache());
            Assert.True(refreshMsg.MarkForClear().Complete());
            Assert.True(refreshMsg.Solicited());

            if (expectedState is null)
            {
                Assert.Equal(StreamStates.OPEN, refreshMsg.State().StreamState);
                Assert.Equal(DataStates.OK, refreshMsg.State().DataState);
                Assert.Equal(StateCodes.NONE, refreshMsg.State().StatusCode);
                Assert.Equal("Item Refresh Completed", refreshMsg.State().StatusText);
            }
            else
            {
                Assert.Equal(expectedState.StreamState(), refreshMsg.State().StreamState);
                Assert.Equal(expectedState.DataState(), refreshMsg.State().DataState);
                Assert.Equal(expectedState.Code(), refreshMsg.State().StatusCode);
                Assert.Equal(expectedState.Text().ToString(), refreshMsg.State().StatusText);
            }

            Assert.True(refreshMsg.HasMsgKey);
            Assert.True(refreshMsg.HasName);
            Assert.Equal(itemName, refreshMsg.Name());
            Assert.True(refreshMsg.HasNameType);
            Assert.Equal(InstrumentNameTypes.RIC, refreshMsg.NameType());
            Assert.True(refreshMsg.HasServiceId);
            Assert.Equal(ProviderTest.DefaultService.ServiceId, refreshMsg.ServiceId());
            Assert.True(refreshMsg.HasServiceName);
            Assert.Equal(ProviderTest.DefaultService.Info.ServiceName.ToString(), refreshMsg.ServiceName());

            MarketPriceItem marketPriceItem = providerTest.MarketItemHandler
                .MarketPriceItems[itemIndex];

            Assert.Equal(DataType.DataTypes.FIELD_LIST, refreshMsg.MarkForClear().Payload().DataType);

            FieldList fieldList = refreshMsg.MarkForClear().Payload().FieldList();
            var fieldIt = fieldList.GetEnumerator();
            FieldEntry fieldEntry;

            if (fidSet is null)
            {
                Assert.True(fieldIt.MoveNext());
                fieldEntry = fieldIt.Current;

                Assert.Equal(MarketPriceItem.RDNDISPLAY_FID, fieldEntry.FieldId);
                Assert.Equal(marketPriceItem.RDNDISPLAY, (int)fieldEntry.UIntValue());
            }

            if (fidSet is null)
            {
                Assert.True(fieldIt.MoveNext());
                fieldEntry = fieldIt.Current;

                Assert.Equal(MarketPriceItem.RDN_EXCHID_FID, fieldEntry.FieldId);
                Assert.Equal(marketPriceItem.RDN_EXCHID, fieldEntry.EnumValue());
            }

            Assert.True(fieldIt.MoveNext());
            fieldEntry = fieldIt.Current;

            Assert.Equal(MarketPriceItem.DIVPAYDATE_FID, fieldEntry.FieldId);
            Assert.Equal(marketPriceItem.DIVPAYDATE.ToString(), fieldEntry.OmmDateValue().ToString());

            Assert.True(fieldIt.MoveNext());
            fieldEntry = fieldIt.Current;

            Assert.Equal(MarketPriceItem.TRDPRC_1_FID, fieldEntry.FieldId);
            Assert.Equal(marketPriceItem.TRDPRC_1.ToString(), fieldEntry.OmmRealValue().ToString());

            Assert.True(fieldIt.MoveNext());
            fieldEntry = fieldIt.Current;

            Assert.Equal(MarketPriceItem.BID_FID, fieldEntry.FieldId);
            Assert.Equal(marketPriceItem.BID.ToString(), fieldEntry.OmmRealValue().ToString());

            Assert.True(fieldIt.MoveNext());
            fieldEntry = fieldIt.Current;

            Assert.Equal(MarketPriceItem.ASK_FID, fieldEntry.FieldId);
            Assert.Equal(marketPriceItem.ASK.ToString(), fieldEntry.OmmRealValue().ToString());

            Assert.True(fieldIt.MoveNext());
            fieldEntry = fieldIt.Current;

            Assert.Equal(MarketPriceItem.ASK_TIME_FID, fieldEntry.FieldId);
            Assert.Equal(marketPriceItem.ASK_TIME.Time().ToString(), fieldEntry.OmmTimeValue().ToString());

            Assert.False(fieldIt.MoveNext());
        }

        private static void CheckUpdatePayload(ProviderTest providerTest, int itemIndex, UpdateMsg updateMsg)
        {
            MarketPriceItem marketPriceItem = providerTest.MarketItemHandler
                .MarketPriceItems[itemIndex];

            FieldList fieldList = updateMsg.MarkForClear().Payload().FieldList();

            var fieldIt = fieldList.GetEnumerator();

            Assert.True(fieldIt.MoveNext());
            var fieldEntry = fieldIt.Current;

            Assert.Equal(MarketPriceItem.TRDPRC_1_FID, fieldEntry.FieldId);
            Assert.Equal(marketPriceItem.TRDPRC_1.ToString(), fieldEntry.OmmRealValue().ToString());

            Assert.True(fieldIt.MoveNext());
            fieldEntry = fieldIt.Current;

            Assert.Equal(MarketPriceItem.BID_FID, fieldEntry.FieldId);
            Assert.Equal(marketPriceItem.BID.ToString(), fieldEntry.OmmRealValue().ToString());

            Assert.True(fieldIt.MoveNext());
            fieldEntry = fieldIt.Current;

            Assert.Equal(MarketPriceItem.ASK_FID, fieldEntry.FieldId);
            Assert.Equal(marketPriceItem.ASK.ToString(), fieldEntry.OmmRealValue().ToString());

            Assert.False(fieldIt.MoveNext());
        }

        [Fact]
        public void SecondUninitializeTest()
        {
            // Arrange
            ProviderSessionOptions providerSessionOpts = new()
            {
                SendLoginReject = false,
                SendDictionaryResp = true
            };
            ProviderTest providerTest = new(providerSessionOpts, output);
            ReactorOptions reactorOptions = new();
            providerTest.Initialize(reactorOptions);

            string hostString = $"localhost:{providerTest.ServerPort}";

            output.WriteLine($"Connect with {hostString}");

            OmmConsumerConfig config = new();
            OmmConsumer consumer = new(config.Host(hostString));
            string consumerNameInitial = consumer.ConsumerName;

            consumer.Uninitialize();
            providerTest.UnInitialize();

            // Act/Assert
            consumer.Uninitialize();
            // No exception should be at this point

            Assert.Equal(consumerNameInitial, consumer.ConsumerName);
        }

        [Fact]
        public void LoginRequestTimeoutTest()
        {
            OmmConsumerConfig config = new OmmConsumerConfig();
            OmmException? exception = null;
            OmmConsumer? consumer = null;

            try
            {
                consumer = new OmmConsumer(config.Host("unknownhost:15999"));
            }
            catch (OmmException ommException)
            {
                exception = ommException;
                Assert.True(exception.Type == OmmException.ExceptionType.OmmInvalidUsageException);
                Assert.StartsWith("login failed (timed out after waiting 45000 milliseconds) for unknownhost:15999)", exception.Message);
            }

            Assert.NotNull(exception);
            Assert.Null(consumer);
        }

        [Fact]
        public void LoginRejectByProviderTest()
        {
            ProviderSessionOptions providerSessionOpts = new ProviderSessionOptions();
            providerSessionOpts.SendLoginReject = true;
            ProviderTest providerTest = new ProviderTest(providerSessionOpts, output);
            ReactorOptions reactorOptions = new ReactorOptions();
            providerTest.Initialize(reactorOptions);

            string hostString = $"localhost:{providerTest.ServerPort}";

            output.WriteLine($"Connect with {hostString}");

            OmmConsumerConfig config = new OmmConsumerConfig();
            OmmException? exception = null;
            OmmConsumer? consumer = null;

            try
            {
                consumer = new OmmConsumer(config.Host(hostString));
            }
            catch (OmmException ommException)
            {
                exception = ommException;
                Assert.True(exception.Type == OmmException.ExceptionType.OmmInvalidUsageException);
                output.WriteLine($"exception.Message = {exception.Message}");
                Assert.Contains("Login request rejected for stream id 1 - Force logout by Provider", exception.Message);
            }

            Assert.NotNull(exception);
            Assert.Null(consumer);
            providerTest.UnInitialize();
        }

        [Fact]
        public void LoginRejectByProviderWithClientTest()
        {
            ProviderSessionOptions providerSessionOpts = new ProviderSessionOptions();
            providerSessionOpts.SendLoginReject = true;
            ProviderTest providerTest = new ProviderTest(providerSessionOpts, output);
            ReactorOptions reactorOptions = new ReactorOptions();
            providerTest.Initialize(reactorOptions);

            string hostString = $"localhost:{providerTest.ServerPort}";

            output.WriteLine($"Connect with {hostString}");

            OmmConsumerConfig config = new OmmConsumerConfig();
            OmmException? exception = null;
            OmmConsumer? consumer = null;
            OmmConsumerItemClientTest consumerClient = new();

            try
            {
                consumerClient.StatusMsgHandler = (statusMsg, consEvent) =>
                {
                    Assert.Equal(DataType.DataTypes.NO_DATA, statusMsg.MarkForClear().Payload().DataType);

                    Assert.True(statusMsg.HasState);
                    Assert.Equal(OmmState.StreamStates.CLOSED, statusMsg.State().StreamState);
                    Assert.Equal(OmmState.DataStates.SUSPECT, statusMsg.State().DataState);
                    Assert.Equal(OmmState.StatusCodes.USAGE_ERROR, statusMsg.State().StatusCode);
                    Assert.Equal("Login request rejected for stream id 1 - Force logout by Provider", statusMsg.State().StatusText);
                };

                consumer = new OmmConsumer(config.Host(hostString), consumerClient, this);
            }
            catch (OmmException ommException)
            {
                exception = ommException;
                Assert.True(exception.Type == OmmException.ExceptionType.OmmInvalidUsageException);
                output.WriteLine($"exception.Message = {exception.Message}");
                Assert.Contains("Login request rejected for stream id 1 - Force logout by Provider", exception.Message);
            }

            Assert.Equal(1, consumerClient.ReceivedOnAll);
            Assert.Equal(0, consumerClient.ReceivedOnRefresh);
            Assert.Equal(1, consumerClient.ReceivedOnStatus);
            Assert.Equal(0, consumerClient.ReceivedOnUpdate);

            Assert.NotNull(exception);
            Assert.Null(consumer);
            providerTest.UnInitialize();
        }

        [Fact]
        public void DirectorRequestTimeoutTest()
        {
            ProviderSessionOptions providerSessionOpts = new ProviderSessionOptions();
            providerSessionOpts.SendDirectoryResp = false;
            ProviderTest providerTest = new ProviderTest(providerSessionOpts, output);
            ReactorOptions reactorOptions = new ReactorOptions();
            providerTest.Initialize(reactorOptions);

            string hostString = $"localhost:{providerTest.ServerPort}";

            output.WriteLine($"Connect with {hostString}");

            OmmConsumerConfig config = new OmmConsumerConfig();
            OmmException? exception = null;
            OmmConsumer? consumer = null;

            try
            {
                consumer = new OmmConsumer(config.Host(hostString));
            }
            catch (OmmException ommException)
            {
                exception = ommException;
                Assert.True(exception.Type == OmmException.ExceptionType.OmmInvalidUsageException);
                Assert.StartsWith("directory retrieval failed (timed out after waiting 45000 milliseconds) for", exception.Message);
            }

            Assert.NotNull(exception);
            Assert.Null(consumer);
            providerTest.UnInitialize();
        }

        [Fact]
        public void CreateValidOmmConsumerTest() // Receives both login and directory refersh messages.
        {
            ProviderSessionOptions providerSessionOpts = new ProviderSessionOptions();
            providerSessionOpts.SendLoginReject = false;
            providerSessionOpts.SendDictionaryResp = true;
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
            }
            catch (OmmException ommException)
            {
                Assert.Fail($"Exception during initialization not expected: {ommException}");
            }

            consumer?.Uninitialize();
            providerTest.UnInitialize();
        }

        [Theory]
        [InlineData(false)]
        [InlineData(true)]
        public void RegisterAndUnRegisterSourceDirectoryReqTest(bool userDispatch = false) // Receives both login and directory refersh messages.
        {
            ProviderSessionOptions providerSessionOpts = new ProviderSessionOptions();
            providerSessionOpts.SendDictionaryResp = true;
            ProviderTest providerTest = new ProviderTest(providerSessionOpts, output);
            ReactorOptions reactorOptions = new ReactorOptions();
            providerTest.Initialize(reactorOptions);

            string hostString = $"localhost:{providerTest.ServerPort}";

            output.WriteLine($"userDispatch = {userDispatch}");
            output.WriteLine($"Connect with {hostString}");


            OmmConsumerConfig config = new OmmConsumerConfig();
            OmmException? exception = null;
            OmmConsumer? consumer = null;

            OmmConsumerItemClientTest consumerClient;

            try
            {
                config.Host(hostString);

                if (userDispatch)
                    config.OperationModel(OmmConsumerConfig.OperationModelMode.USER_DISPATCH);

                consumer = new OmmConsumer(config.Host(hostString));

                output.WriteLine("Finished creating an OmmConsumer");

                consumerClient = new OmmConsumerItemClientTest(consumer);

                RequestMsg requestMsg = new();
                requestMsg.DomainType(EmaRdm.MMT_DIRECTORY).ServiceName("DEFAULT_SERVICE");

                long handle = consumer.RegisterClient(requestMsg, consumerClient, this);

                Assert.True(handle > 0);

                output.WriteLine("Finished sending request");

                /* Checks the expected RefreshMsg from the source directory request */
                consumerClient.RefreshMsgHandler = (refreshMsg, consEvent) =>
                {
                    Assert.Equal(handle, consEvent.Handle);

                    Assert.Equal(DataType.DataTypes.MAP, refreshMsg.MarkForClear().Payload().DataType);

                    Map map = refreshMsg.MarkForClear().Payload().Map();

                    foreach (var mapEntry in map)
                    {
                        if (mapEntry.LoadType == DataType.DataTypes.FILTER_LIST)
                        {
                            FilterList filterList = mapEntry.FilterList();

                            foreach (var filterEntry in filterList)
                            {
                                int filterId = filterEntry.FilterId;

                                switch (filterId)
                                {
                                    case ServiceFilterFlags.INFO:
                                        {
                                            Assert.Equal(DataType.DataTypes.ELEMENT_LIST, filterEntry.LoadType);
                                            ElementList elementList = filterEntry.ElementList();

                                            var enumerator = elementList.GetEnumerator();
                                            Assert.True(enumerator.MoveNext());

                                            var entry = enumerator.Current;

                                            Assert.Equal(DataType.DataTypes.ASCII, entry.LoadType);
                                            Assert.Equal("Name", entry.Name);
                                            Assert.Equal(ProviderTest.DefaultService.Info.ServiceName.ToString(), entry.OmmAsciiValue().Value);

                                            Assert.True(enumerator.MoveNext());
                                            entry = enumerator.Current;

                                            Assert.Equal(DataType.DataTypes.ARRAY, entry.LoadType);
                                            Assert.Equal("Capabilities", entry.Name);
                                            OmmArray array = entry.OmmArrayValue();

                                            var arrayIt = array.GetEnumerator();

                                            foreach (var capability in ProviderTest.DefaultService.Info.CapabilitiesList)
                                            {
                                                Assert.True(arrayIt.MoveNext());
                                                var arrayEntry = arrayIt.Current;

                                                Assert.Equal(DataType.DataTypes.UINT, arrayEntry.LoadType);
                                                Assert.Equal(capability, (long)arrayEntry.OmmUIntValue().Value);
                                            }
                                            Assert.False(arrayIt.MoveNext());

                                            Assert.True(enumerator.MoveNext());
                                            entry = enumerator.Current;

                                            Assert.Equal(DataType.DataTypes.ARRAY, entry.LoadType);
                                            Assert.Equal("DictionariesProvided", entry.Name);
                                            array = entry.OmmArrayValue();

                                            arrayIt = array.GetEnumerator();
                                            foreach (var dictName in ProviderTest.DefaultService.Info.DictionariesProvidedList)
                                            {
                                                Assert.True(arrayIt.MoveNext());
                                                var arrayEntry = arrayIt.Current;

                                                Assert.Equal(DataType.DataTypes.ASCII, arrayEntry.LoadType);
                                                Assert.Equal(dictName, arrayEntry.OmmAsciiValue().Value);
                                            }
                                            Assert.False(arrayIt.MoveNext());

                                            Assert.True(enumerator.MoveNext());
                                            entry = enumerator.Current;

                                            Assert.Equal(DataType.DataTypes.ARRAY, entry.LoadType);
                                            Assert.Equal("DictionariesUsed", entry.Name);
                                            array = entry.OmmArrayValue();

                                            arrayIt = array.GetEnumerator();
                                            foreach (var dictName in ProviderTest.DefaultService.Info.DictionariesProvidedList)
                                            {
                                                Assert.True(arrayIt.MoveNext());
                                                var arrayEntry = arrayIt.Current;

                                                Assert.Equal(DataType.DataTypes.ASCII, arrayEntry.LoadType);
                                                Assert.Equal(dictName, arrayEntry.OmmAsciiValue().Value);
                                            }
                                            Assert.False(arrayIt.MoveNext());

                                            Assert.True(enumerator.MoveNext());
                                            entry = enumerator.Current;

                                            Assert.Equal(DataType.DataTypes.ARRAY, entry.LoadType);
                                            Assert.Equal("QoS", entry.Name);
                                            array = entry.OmmArrayValue();

                                            arrayIt = array.GetEnumerator();
                                            foreach (var qos in ProviderTest.DefaultService.Info.QosList)
                                            {
                                                Assert.True(arrayIt.MoveNext());
                                                var arrayEntry = arrayIt.Current;

                                                Assert.Equal(DataType.DataTypes.QOS, arrayEntry.LoadType);
                                                Assert.Equal(qos.Rate(), EmaTestUtils.QosRate(arrayEntry.OmmQosValue().Rate));
                                                Assert.Equal(qos.Timeliness(), EmaTestUtils.QosTimeliness(arrayEntry.OmmQosValue().Timeliness));
                                            }
                                            Assert.False(arrayIt.MoveNext());

                                            Assert.False(enumerator.MoveNext());
                                            break;
                                        }

                                    case ServiceFilterFlags.STATE:
                                        {
                                            Assert.Equal(DataType.DataTypes.ELEMENT_LIST, filterEntry.LoadType);
                                            ElementList elementList = filterEntry.ElementList();

                                            var enumerator = elementList.GetEnumerator();
                                            Assert.True(enumerator.MoveNext());

                                            var entry = enumerator.Current;

                                            Assert.Equal(DataType.DataTypes.UINT, entry.LoadType);
                                            Assert.Equal("ServiceState", entry.Name);
                                            Assert.Equal(ProviderTest.DefaultService.State.ServiceStateVal, (long)entry.OmmUIntValue().Value);

                                            Assert.True(enumerator.MoveNext());
                                            entry = enumerator.Current;

                                            Assert.Equal(DataType.DataTypes.UINT, entry.LoadType);
                                            Assert.Equal("AcceptingRequests", entry.Name);
                                            Assert.Equal(ProviderTest.DefaultService.State.AcceptingRequests, (long)entry.OmmUIntValue().Value);

                                            Assert.False(enumerator.MoveNext());

                                            break;
                                        }
                                    default:
                                        {
                                            Assert.True(false);
                                            break;
                                        }
                                }
                            }
                        }
                    }

                };

                if (userDispatch)
                {
                    output.WriteLine("Start user dispatching");
                    consumer.Dispatch(DispatchTimeout.INFINITE_WAIT);
                    output.WriteLine("End user's dispatching");
                }
                else
                {
                    Thread.Sleep(1000);
                }

                Assert.Equal(1, consumerClient.ReceivedOnAll);
                Assert.Equal(1, consumerClient.ReceivedOnRefresh);
                Assert.Equal(0, consumerClient.ReceivedOnStatus);
                Assert.Equal(0, consumerClient.ReceivedOnUpdate);

                consumer.Unregister(handle);
            }
            catch (OmmException ommException)
            {
                exception = ommException;
            }

            Assert.Null(exception);

            consumer?.Uninitialize();
            providerTest.UnInitialize();
        }

        [Theory]
        [InlineData(false)]
        [InlineData(true)]
        public void RegisterAndUnRegisterLoginReqTest(bool userDispatch = false) // Receives both login and directory refersh messages.
        {
            ProviderSessionOptions providerSessionOpts = new ProviderSessionOptions();
            providerSessionOpts.SendDictionaryResp = true;
            ProviderTest providerTest = new ProviderTest(providerSessionOpts, output);
            ReactorOptions reactorOptions = new ReactorOptions();
            providerTest.Initialize(reactorOptions);

            string hostString = $"localhost:{providerTest.ServerPort}";

            output.WriteLine($"Connect with {hostString}");

            OmmConsumerConfig config = new OmmConsumerConfig();
            OmmException? exception = null;
            OmmConsumer? consumer = null;

            OmmConsumerItemClientTest consumerClient;

            try
            {
                config.Host(hostString);

                if (userDispatch)
                    config.OperationModel(OmmConsumerConfig.OperationModelMode.USER_DISPATCH);

                consumer = new OmmConsumer(config.Host(hostString));

                consumerClient = new OmmConsumerItemClientTest(consumer);

                RequestMsg requestMsg = new();
                requestMsg.DomainType(EmaRdm.MMT_LOGIN);

                long handle = consumer.RegisterClient(requestMsg, consumerClient, this);

                Assert.True(handle > 0);

                /* Checks the expected RefreshMsg from the login request */
                consumerClient.RefreshMsgHandler = (refreshMsg, consEvent) =>
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
                    consumer.Dispatch(DispatchTimeout.INFINITE_WAIT);
                }
                else
                {
                    Thread.Sleep(1000);
                }

                Assert.Equal(1, consumerClient.ReceivedOnAll);
                Assert.Equal(1, consumerClient.ReceivedOnRefresh);
                Assert.Equal(0, consumerClient.ReceivedOnStatus);
                Assert.Equal(0, consumerClient.ReceivedOnUpdate);

                consumer.Unregister(handle);
            }
            catch (OmmException ommException)
            {
                exception = ommException;
            }

            Assert.Null(exception);

            consumer?.Uninitialize();
            providerTest.UnInitialize();
        }

        [Theory]
        [InlineData(false)]
        [InlineData(true)]
        public void RegisterSingleMarketDataRequestTest(bool userDispatch = false)
        {
            ProviderSessionOptions providerSessionOpts = new ProviderSessionOptions();
            providerSessionOpts.SendDictionaryResp = true;
            ProviderTest providerTest = new ProviderTest(providerSessionOpts, output);
            ReactorOptions reactorOptions = new ReactorOptions();
            providerTest.Initialize(reactorOptions);

            string hostString = $"localhost:{providerTest.ServerPort}";

            output.WriteLine($"userDispatch = {userDispatch}");
            output.WriteLine($"Connect with {hostString}");


            OmmConsumerConfig config = new OmmConsumerConfig();
            OmmException? exception = null;
            OmmConsumer? consumer = null;

            OmmConsumerItemClientTest consumerClient;

            try
            {
                config.Host(hostString);

                if (userDispatch)
                    config.OperationModel(OmmConsumerConfig.OperationModelMode.USER_DISPATCH);

                consumer = new OmmConsumer(config.Host(hostString));

                output.WriteLine("Finished creating an OmmConsumer");

                consumerClient = new OmmConsumerItemClientTest(consumer);

                string itemName = "ItemName";

                RequestMsg requestMsg = new();
                requestMsg.DomainType(EmaRdm.MMT_MARKET_PRICE).ServiceName(ProviderTest.DefaultService.Info.ServiceName.ToString())
                    .Name(itemName).InterestAfterRefresh(true);

                long handle = consumer.RegisterClient(requestMsg, consumerClient, this);

                Assert.True(handle > 0);

                /* Checks the expected RefreshMsg from the market data request */
                consumerClient.RefreshMsgHandler = (refreshMsg, consEvent) =>
                {
                    Assert.Equal(handle, consEvent.Handle);
                    Assert.Same(this, consEvent.Closure);

                    CheckRefreshPayload(providerTest, 0, refreshMsg, itemName);
                };

                consumerClient.UpdateMsgHandler = (updateMsg, consEvent) =>
                {
                    Assert.Equal(handle, consEvent.Handle);
                    Assert.Same(this, consEvent.Closure);
                    Assert.Equal(DataType.DataTypes.FIELD_LIST, updateMsg.MarkForClear().Payload().DataType);

                    CheckUpdatePayload(providerTest, 0, updateMsg);
                };

                if (userDispatch)
                {
                    consumer.Dispatch(DispatchTimeout.INFINITE_WAIT);
                    consumer.Dispatch(DispatchTimeout.INFINITE_WAIT);
                }
                else
                {
                    Thread.Sleep(10000);
                }

                Assert.Equal(2, consumerClient.ReceivedOnAll);
                Assert.Equal(1, consumerClient.ReceivedOnRefresh);
                Assert.Equal(0, consumerClient.ReceivedOnStatus);
                Assert.Equal(1, consumerClient.ReceivedOnUpdate);

                consumer.Unregister(handle);

            }
            catch (OmmException ommException)
            {
                exception = ommException;
            }

            Assert.Null(exception);

            consumer?.Uninitialize();
            providerTest.UnInitialize();
        }

        [Theory]
        [InlineData(false)]
        [InlineData(true)]
        public void RegisterSingleMarketData_InvalidServiceNameTest(bool userDispatch = false)
        {
            ProviderSessionOptions providerSessionOpts = new ProviderSessionOptions();
            providerSessionOpts.SendDictionaryResp = true;
            ProviderTest providerTest = new ProviderTest(providerSessionOpts, output);
            ReactorOptions reactorOptions = new ReactorOptions();
            providerTest.Initialize(reactorOptions);

            string hostString = $"localhost:{providerTest.ServerPort}";

            output.WriteLine($"userDispatch = {userDispatch}");
            output.WriteLine($"Connect with {hostString}");


            OmmConsumerConfig config = new OmmConsumerConfig();
            OmmException? exception = null;
            OmmConsumer? consumer = null;

            OmmConsumerItemClientTest consumerClient;

            try
            {
                config.Host(hostString);

                if (userDispatch)
                    config.OperationModel(OmmConsumerConfig.OperationModelMode.USER_DISPATCH);

                consumer = new OmmConsumer(config.Host(hostString));

                output.WriteLine("Finished creating an OmmConsumer");

                consumerClient = new OmmConsumerItemClientTest(consumer);

                string itemName = "ItemName";

                RequestMsg requestMsg = new();
                requestMsg.DomainType(EmaRdm.MMT_MARKET_PRICE).ServiceName("INVALID_NAME")
                    .Name(itemName);

                long handle = consumer.RegisterClient(requestMsg, consumerClient, this);

                Assert.True(handle > 0);

                consumerClient.StatusMsgHandler = (statusMsg, consEvent) =>
                {
                    Assert.Equal(handle, consEvent.Handle);
                    Assert.Equal(DataType.DataTypes.NO_DATA, statusMsg.MarkForClear().Payload().DataType);

                    Assert.True(statusMsg.HasState);
                    Assert.Equal(OmmState.StreamStates.OPEN, statusMsg.State().StreamState);
                    Assert.Equal(OmmState.DataStates.SUSPECT, statusMsg.State().DataState);
                    Assert.Equal(OmmState.StatusCodes.NONE, statusMsg.State().StatusCode);
                    Assert.Equal("Service not available", statusMsg.State().StatusText);
                };

                if (userDispatch)
                {
                    consumer.Dispatch(DispatchTimeout.INFINITE_WAIT);
                }
                else
                {
                    Thread.Sleep(5000);
                }

                Assert.Equal(1, consumerClient.ReceivedOnAll);
                Assert.Equal(0, consumerClient.ReceivedOnRefresh);
                Assert.Equal(1, consumerClient.ReceivedOnStatus);
                Assert.Equal(0, consumerClient.ReceivedOnUpdate);

                consumer.Unregister(handle);
            }
            catch(OmmException ommException)
            {
                exception = ommException;
            }

            Assert.Null(exception);

            consumer?.Uninitialize();
            providerTest.UnInitialize();
        }

        [Theory]
        [InlineData(false)]
        [InlineData(true)]
        public void RegisterSingleMarketData_UnknownDomainNameTest(bool userDispatch = false)
        {
            ProviderSessionOptions providerSessionOpts = new ProviderSessionOptions();
            providerSessionOpts.SendDictionaryResp = true;
            ProviderTest providerTest = new ProviderTest(providerSessionOpts, output);
            ReactorOptions reactorOptions = new ReactorOptions();
            providerTest.Initialize(reactorOptions);

            string hostString = $"localhost:{providerTest.ServerPort}";

            output.WriteLine($"userDispatch = {userDispatch}");
            output.WriteLine($"Connect with {hostString}");


            OmmConsumerConfig config = new OmmConsumerConfig();
            OmmException? exception = null;
            OmmConsumer? consumer = null;

            OmmConsumerItemClientTest consumerClient;

            try
            {
                config.Host(hostString);

                if (userDispatch)
                    config.OperationModel(OmmConsumerConfig.OperationModelMode.USER_DISPATCH);

                consumer = new OmmConsumer(config.Host(hostString));

                output.WriteLine("Finished creating an OmmConsumer");

                consumerClient = new OmmConsumerItemClientTest(consumer);

                string itemName = "ItemName";

                RequestMsg requestMsg = new();
                requestMsg.DomainType(155).ServiceId(1).Name(itemName);

                long handle = consumer.RegisterClient(requestMsg, consumerClient, this);

                Assert.True(handle > 0);

                consumerClient.StatusMsgHandler = (statusMsg, consEvent) =>
                {
                    Assert.Equal(handle, consEvent.Handle);
                    Assert.Equal(DataType.DataTypes.NO_DATA, statusMsg.MarkForClear().Payload().DataType);

                    Assert.True(statusMsg.HasState);
                    Assert.Equal(OmmState.StreamStates.OPEN, statusMsg.State().StreamState);
                    Assert.Equal(OmmState.DataStates.SUSPECT, statusMsg.State().DataState);
                    Assert.Equal(OmmState.StatusCodes.NONE, statusMsg.State().StatusCode);
                    Assert.Equal("Capability not supported", statusMsg.State().StatusText);
                };

                if (userDispatch)
                {
                    consumer.Dispatch(DispatchTimeout.INFINITE_WAIT);
                }
                else
                {
                    Thread.Sleep(5000);
                }

                Assert.Equal(1, consumerClient.ReceivedOnAll);
                Assert.Equal(0, consumerClient.ReceivedOnRefresh);
                Assert.Equal(1, consumerClient.ReceivedOnStatus);
                Assert.Equal(0, consumerClient.ReceivedOnUpdate);

                consumer.Unregister(handle);
            }
            catch (OmmException ommException)
            {
                exception = ommException;
            }

            Assert.Null(exception);

            consumer?.Uninitialize();
            providerTest.UnInitialize();
        }

        [Theory]
        [InlineData(false)]
        [InlineData(true)]
        public void RegisterSingleMarketData_RequestTimeoutTest(bool userDispatch = false)
        {
            ProviderSessionOptions providerSessionOpts = new ProviderSessionOptions();
            providerSessionOpts.SendDictionaryResp = true;
            providerSessionOpts.SendMarketDataItemResp = false;
            ProviderTest providerTest = new ProviderTest(providerSessionOpts, output);
            ReactorOptions reactorOptions = new ReactorOptions();
            providerTest.Initialize(reactorOptions);

            string hostString = $"localhost:{providerTest.ServerPort}";

            output.WriteLine($"userDispatch = {userDispatch}");
            output.WriteLine($"Connect with {hostString}");


            OmmConsumerConfig config = new OmmConsumerConfig();
            OmmException? exception = null;
            OmmConsumer? consumer = null;

            OmmConsumerItemClientTest consumerClient;

            try
            {
                config.Host(hostString);

                if (userDispatch)
                    config.OperationModel(OmmConsumerConfig.OperationModelMode.USER_DISPATCH);

                consumer = new OmmConsumer(config.Host(hostString));

                output.WriteLine("Finished creating an OmmConsumer");

                consumerClient = new OmmConsumerItemClientTest(consumer);

                string itemName = "ItemName";

                RequestMsg requestMsg = new();
                requestMsg.DomainType(EmaRdm.MMT_MARKET_PRICE).ServiceId(1).Name(itemName);

                long handle = consumer.RegisterClient(requestMsg, consumerClient, this);

                Assert.True(handle > 0);

                consumerClient.StatusMsgHandler = (statusMsg, consEvent) =>
                {
                    Assert.Equal(handle, consEvent.Handle);
                    Assert.Equal(DataType.DataTypes.NO_DATA, statusMsg.MarkForClear().Payload().DataType);

                    Assert.True(statusMsg.HasState);
                    Assert.Equal(OmmState.StreamStates.OPEN, statusMsg.State().StreamState);
                    Assert.Equal(OmmState.DataStates.SUSPECT, statusMsg.State().DataState);
                    Assert.Equal(OmmState.StatusCodes.NONE, statusMsg.State().StatusCode);
                    Assert.Equal("Request timeout", statusMsg.State().StatusText);
                };

                if (userDispatch)
                {
                    consumer.Dispatch(DispatchTimeout.INFINITE_WAIT);
                }
                else
                {
                    Thread.Sleep(20000);
                }

                Assert.Equal(1, consumerClient.ReceivedOnAll);
                Assert.Equal(0, consumerClient.ReceivedOnRefresh);
                Assert.Equal(1, consumerClient.ReceivedOnStatus);
                Assert.Equal(0, consumerClient.ReceivedOnUpdate);

                consumer.Unregister(handle);
            }
            catch (OmmException ommException)
            {
                exception = ommException;
            }

            Assert.Null(exception);

            consumer?.Uninitialize();
            providerTest.UnInitialize();
        }

        [Theory]
        [InlineData(false)]
        [InlineData(true)]
        public void RegisterSingleMarketData_RejectItemReqTest(bool userDispatch = false)
        {
            ProviderSessionOptions providerSessionOpts = new ProviderSessionOptions();
            providerSessionOpts.SendDictionaryResp = true;
            providerSessionOpts.SendMarketDataItemReject = true;
            ProviderTest providerTest = new ProviderTest(providerSessionOpts, output);
            ReactorOptions reactorOptions = new ReactorOptions();
            providerTest.Initialize(reactorOptions);

            string hostString = $"localhost:{providerTest.ServerPort}";

            output.WriteLine($"userDispatch = {userDispatch}");
            output.WriteLine($"Connect with {hostString}");
            
            OmmConsumerConfig config = new OmmConsumerConfig();
            OmmException? exception = null;
            OmmConsumer? consumer = null;

            OmmConsumerItemClientTest consumerClient;

            try
            {
                config.Host(hostString);

                if (userDispatch)
                    config.OperationModel(OmmConsumerConfig.OperationModelMode.USER_DISPATCH);

                consumer = new OmmConsumer(config.Host(hostString));

                output.WriteLine("Finished creating an OmmConsumer");

                consumerClient = new OmmConsumerItemClientTest(consumer);

                string itemName = "ItemName";

                RequestMsg requestMsg = new();
                requestMsg.DomainType(EmaRdm.MMT_MARKET_PRICE).ServiceId(1).Name(itemName);

                long handle = consumer.RegisterClient(requestMsg, consumerClient, this);

                Assert.True(handle > 0);

                consumerClient.StatusMsgHandler = (statusMsg, consEvent) =>
                {
                    Assert.Equal(handle, consEvent.Handle);
                    Assert.Equal(DataType.DataTypes.NO_DATA, statusMsg.MarkForClear().Payload().DataType);

                    Assert.True(statusMsg.HasState);
                    Assert.Equal(OmmState.StreamStates.CLOSED, statusMsg.State().StreamState);
                    Assert.Equal(OmmState.DataStates.SUSPECT, statusMsg.State().DataState);
                    Assert.Equal(OmmState.StatusCodes.USAGE_ERROR, statusMsg.State().StatusCode);
                    Assert.Equal("Item request rejected for stream id 5- stream already in use with a different key", statusMsg.State().StatusText);
                };

                if (userDispatch)
                {
                    consumer.Dispatch(DispatchTimeout.INFINITE_WAIT);
                }
                else
                {
                    Thread.Sleep(5000);
                }

                Assert.Equal(1, consumerClient.ReceivedOnAll);
                Assert.Equal(0, consumerClient.ReceivedOnRefresh);
                Assert.Equal(1, consumerClient.ReceivedOnStatus);
                Assert.Equal(0, consumerClient.ReceivedOnUpdate);

                consumer.Unregister(handle);
            }
            catch (OmmException ommException)
            {
                exception = ommException;
            }

            Assert.Null(exception);

            consumer?.Uninitialize();
            providerTest.UnInitialize();
        }

        [Fact]
        public void RegisterBatchRequestWithEmptyListTest()
        {
            var exception = Assert.ThrowsAny<OmmException>(() =>
            {
                string[] itemNames = { "", "", "", "", "", "" };

                ElementList payload = new ElementList();
                OmmArray array = new OmmArray();
                array.AddAscii(itemNames[0]).AddAscii(itemNames[1]).AddAscii(itemNames[2])
                    .AddAscii(itemNames[3]).AddAscii(itemNames[4]).AddAscii(itemNames[5]).MarkForClear().Complete();
                payload.AddArray(EmaRdm.ENAME_BATCH_ITEM_LIST, array).MarkForClear().Complete();

                RequestMsg requestMsg = new();
                requestMsg.DomainType(EmaRdm.MMT_MARKET_PRICE).ServiceId(1)
                    .MarkForClear().Payload(payload);
            });

            Assert.Equal("RequestMsgEncoder.CheckBatchView: Batch request item list is blank.", exception!.Message);
        }

        [Theory]
        [InlineData(false)]
        [InlineData(true)]
        public void RegisterBatchMarketDataItemTest(bool userDispatch = false)
        {
            ProviderSessionOptions providerSessionOpts = new ProviderSessionOptions();
            providerSessionOpts.SendMarketDataItemUpdate = false;
            providerSessionOpts.SendDictionaryResp = true;
            ProviderTest providerTest = new ProviderTest(providerSessionOpts, output);

            ReactorOptions reactorOptions = new ReactorOptions();
            providerTest.Initialize(reactorOptions);

            string hostString = $"localhost:{providerTest.ServerPort}";

            output.WriteLine($"userDispatch = {userDispatch}");
            output.WriteLine($"Connect with {hostString}");

            OmmConsumerConfig config = new OmmConsumerConfig();
            OmmException? exception = null;
            OmmConsumer? consumer = null;

            OmmConsumerItemClientTest consumerClient;

            try
            {
                config.Host(hostString);

                if (userDispatch)
                    config.OperationModel(OmmConsumerConfig.OperationModelMode.USER_DISPATCH);

                consumer = new OmmConsumer(config.Host(hostString));

                consumerClient = new OmmConsumerItemClientTest(consumer);

                string[] itemNames = { "LSG", "TRI", "ABC", "ABC", "TRI", "LSG" };

                ElementList payload = new ElementList();
                OmmArray array = new OmmArray();
                array.AddAscii(itemNames[0]).AddAscii(itemNames[1]).AddAscii(itemNames[2])
                    .AddAscii(itemNames[3]).AddAscii(itemNames[4]).AddAscii(itemNames[5]).MarkForClear().Complete();
                payload.AddArray(EmaRdm.ENAME_BATCH_ITEM_LIST, array).MarkForClear().Complete();

                RequestMsg requestMsg = new();
                requestMsg.DomainType(EmaRdm.MMT_MARKET_PRICE).ServiceId(1)
                    .MarkForClear().Payload(payload);

                long handle = consumer.RegisterClient(requestMsg, consumerClient, this);

                Assert.True(handle > 0);

                List<long> HandleList = new();

                long count = 1;

                // Added expected item handles to the list
                foreach(var item in itemNames)
                {
                    HandleList.Add(handle + count++);
                }

                consumerClient.RefreshMsgHandler = (refreshMsg, consEvent) =>
                {
                    if(consumerClient.ReceivedOnRefresh <= 2)
                        CheckRefreshPayload(providerTest, 0, refreshMsg, itemNames[0]);
                    else if (consumerClient.ReceivedOnRefresh <= 4)
                        CheckRefreshPayload(providerTest, 1, refreshMsg, itemNames[1]);
                    else if (consumerClient.ReceivedOnRefresh <= 6)
                        CheckRefreshPayload(providerTest, 2, refreshMsg, itemNames[2]);
                };

                consumerClient.UpdateMsgHandler = (updateMsg, consEvent) =>
                {
                    if (consumerClient.ReceivedOnRefresh <= 2)
                        CheckUpdatePayload(providerTest, 0, updateMsg);
                    else if (consumerClient.ReceivedOnRefresh <= 4)
                        CheckUpdatePayload(providerTest, 1, updateMsg);
                    else if (consumerClient.ReceivedOnRefresh <= 6)
                        CheckUpdatePayload(providerTest, 2, updateMsg);
                };

                consumerClient.StatusMsgHandler = (statusMsg, consEvent) =>
                {
                    // Checks with the batch's handle only.
                    Assert.Equal(handle, consEvent.Handle);
                    Assert.Equal(DataType.DataTypes.NO_DATA, statusMsg.MarkForClear().Payload().DataType);

                    Assert.True(statusMsg.HasState);
                    Assert.Equal(OmmState.StreamStates.CLOSED, statusMsg.State().StreamState);
                    Assert.Equal(OmmState.DataStates.OK, statusMsg.State().DataState);
                    Assert.Equal(OmmState.StatusCodes.NONE, statusMsg.State().StatusCode);
                    Assert.Equal("Stream closed for batch", statusMsg.State().StatusText);
                };

                if (userDispatch)
                {
                    consumer.Dispatch(DispatchTimeout.INFINITE_WAIT);
                    consumer.Dispatch(DispatchTimeout.INFINITE_WAIT);
                    consumer.Dispatch(DispatchTimeout.INFINITE_WAIT);
                }
                else
                {
                    Thread.Sleep(10000);
                }

                // Provider receives only 3 market data requests
                Assert.Equal(3, providerTest.MarketItemHandler.MarketPriceItemIndex);

                Assert.Equal(7, consumerClient.ReceivedOnAll);
                Assert.Equal(6, consumerClient.ReceivedOnRefresh);
                Assert.Equal(1, consumerClient.ReceivedOnStatus);
                Assert.Equal(0, consumerClient.ReceivedOnUpdate);

                foreach (var itemHandle in HandleList)
                {
                    consumer.Unregister(itemHandle);
                }
            }
            catch (OmmException ommException)
            {
                exception = ommException;
            }

            Assert.Null(exception);

            consumer?.Uninitialize();
            providerTest.UnInitialize();
        }


        [Theory]
        [InlineData(false)]
        [InlineData(true)]
        public void RegisterViewMarketDataItemTest(bool userDispatch = false)
        {
            ProviderSessionOptions providerSessionOpts = new ProviderSessionOptions();
            providerSessionOpts.SendDictionaryResp = true;
            providerSessionOpts.SendMarketDataItemUpdate = true;
            ProviderTest providerTest = new ProviderTest(providerSessionOpts, output);
            ReactorOptions reactorOptions = new ReactorOptions();
            providerTest.Initialize(reactorOptions);

            string hostString = $"localhost:{providerTest.ServerPort}";

            output.WriteLine($"userDispatch = {userDispatch}");
            output.WriteLine($"Connect with {hostString}");

            OmmConsumerConfig config = new OmmConsumerConfig();
            OmmException? exception = null;
            OmmConsumer? consumer = null;

            OmmConsumerItemClientTest consumerClient;

            try
            {
                config.Host(hostString);

                if (userDispatch)
                    config.OperationModel(OmmConsumerConfig.OperationModelMode.USER_DISPATCH);

                consumer = new OmmConsumer(config.Host(hostString));

                consumerClient = new OmmConsumerItemClientTest(consumer);

                ElementList payload = new ElementList();
                OmmArray array = new OmmArray();
                //array.FixedWidth = 2;
                array.AddInt(MarketPriceItem.BID_FID).AddInt(MarketPriceItem.ASK_FID).MarkForClear().Complete();

                payload.AddUInt(EmaRdm.ENAME_VIEW_TYPE, 1)
                    .AddArray(EmaRdm.ENAME_VIEW_DATA, array).MarkForClear().Complete();

                RequestMsg requestMsg = new();
                requestMsg.DomainType(EmaRdm.MMT_MARKET_PRICE).ServiceId(1).Name("ItemName")
                    .MarkForClear().Payload(payload).InterestAfterRefresh(true);

                long handle = consumer.RegisterClient(requestMsg, consumerClient, this);

                Assert.True(handle > 0);

                consumerClient.RefreshMsgHandler = (refreshMsg, consEvent) =>
                {
                    Assert.Equal((int)DomainType.MARKET_PRICE, refreshMsg.DomainType());
                    Assert.True(refreshMsg.ClearCache());
                    Assert.True(refreshMsg.MarkForClear().Complete());
                    Assert.True(refreshMsg.Solicited());
                    Assert.Equal(StreamStates.OPEN, refreshMsg.State().StreamState);
                    Assert.Equal(DataStates.OK, refreshMsg.State().DataState);
                    Assert.Equal(StateCodes.NONE, refreshMsg.State().StatusCode);
                    Assert.Equal("Item Refresh Completed", refreshMsg.State().StatusText);

                    Assert.True(refreshMsg.HasMsgKey);
                    Assert.True(refreshMsg.HasName);
                    Assert.Equal("ItemName", refreshMsg.Name());
                    Assert.True(refreshMsg.HasNameType);
                    Assert.Equal(InstrumentNameTypes.RIC, refreshMsg.NameType());
                    Assert.True(refreshMsg.HasServiceId);
                    Assert.Equal(ProviderTest.DefaultService.ServiceId, refreshMsg.ServiceId());
                    Assert.True(refreshMsg.HasServiceName);
                    Assert.Equal(ProviderTest.DefaultService.Info.ServiceName.ToString(), refreshMsg.ServiceName());

                    MarketPriceItem marketPriceItem = providerTest.MarketItemHandler
                        .MarketPriceItems[0];

                    Assert.Equal(DataType.DataTypes.FIELD_LIST, refreshMsg.MarkForClear().Payload().DataType);

                    FieldList fieldList = refreshMsg.MarkForClear().Payload().FieldList();
                    var fieldIt = fieldList.GetEnumerator();

                    Assert.True(fieldIt.MoveNext());
                    var fieldEntry = fieldIt.Current;

                    Assert.Equal(MarketPriceItem.BID_FID, fieldEntry.FieldId);
                    Assert.Equal(marketPriceItem.BID.ToString(), fieldEntry.OmmRealValue().ToString());

                    Assert.True(fieldIt.MoveNext());
                    fieldEntry = fieldIt.Current;

                    Assert.Equal(MarketPriceItem.ASK_FID, fieldEntry.FieldId);
                    Assert.Equal(marketPriceItem.ASK.ToString(), fieldEntry.OmmRealValue().ToString());

                    Assert.False(fieldIt.MoveNext());
                };

                consumerClient.UpdateMsgHandler = (updateMsg, consEvent) =>
                {
                    MarketPriceItem marketPriceItem = providerTest.MarketItemHandler
                           .MarketPriceItems[0];

                    FieldList fieldList = updateMsg.MarkForClear().Payload().FieldList();

                    var fieldIt = fieldList.GetEnumerator();

                    Assert.True(fieldIt.MoveNext());
                    var fieldEntry = fieldIt.Current;

                    Assert.Equal(MarketPriceItem.BID_FID, fieldEntry.FieldId);
                    Assert.Equal(marketPriceItem.BID.ToString(), fieldEntry.OmmRealValue().ToString());

                    Assert.True(fieldIt.MoveNext());
                    fieldEntry = fieldIt.Current;

                    Assert.Equal(MarketPriceItem.ASK_FID, fieldEntry.FieldId);
                    Assert.Equal(marketPriceItem.ASK.ToString(), fieldEntry.OmmRealValue().ToString());

                    Assert.False(fieldIt.MoveNext());
                };

                if (userDispatch)
                {
                    consumer.Dispatch(DispatchTimeout.INFINITE_WAIT);
                    consumer.Dispatch(DispatchTimeout.INFINITE_WAIT);
                }
                else
                {
                    Thread.Sleep(8000);
                }

                // Provider receives only 1 market data requests
                Assert.Equal(1, providerTest.MarketItemHandler.MarketPriceItemIndex);

                Assert.Equal(2, consumerClient.ReceivedOnAll);
                Assert.Equal(1, consumerClient.ReceivedOnRefresh);
                Assert.Equal(0, consumerClient.ReceivedOnStatus);
                Assert.Equal(1, consumerClient.ReceivedOnUpdate);

                consumer.Unregister(handle);
            }
            catch (OmmException ommException)
            {
                exception = ommException;
            }

            Assert.Null(exception);

            consumer?.Uninitialize();
            providerTest.UnInitialize();
        }

        [Fact]
        public void SubmitAndReissueWithInvalidHandleTest()
        {
            ProviderSessionOptions providerSessionOpts = new ProviderSessionOptions();
            providerSessionOpts.SendDictionaryResp = true;
            providerSessionOpts.SendMarketDataItemUpdate = true;
            ProviderTest providerTest = new ProviderTest(providerSessionOpts, output);
            ReactorOptions reactorOptions = new ReactorOptions();
            providerTest.Initialize(reactorOptions);

            string hostString = $"localhost:{providerTest.ServerPort}";

            output.WriteLine($"Connect with {hostString}");

            OmmConsumerConfig config = new OmmConsumerConfig();
            OmmException? exception = null;
            OmmConsumer? consumer = null;
            long invalidHandle = 100;
            int numOfExcpetions = 0;

            try
            {
                config.Host(hostString);

                consumer = new OmmConsumer(config.Host(hostString));

                consumer.Submit(new GenericMsg(), invalidHandle);

            }
            catch (OmmException ommException1)
            {
                Assert.Equal(OmmException.ExceptionType.OmmInvalidHandleException, ommException1.Type);
                Assert.True(ommException1 is OmmInvalidHandleException);
                numOfExcpetions++;

                OmmInvalidHandleException invalidHandleExcep = (OmmInvalidHandleException)ommException1;

                Assert.StartsWith("Attempt to use invalid Handle on Submit(GenericMsg)", invalidHandleExcep.Message);

                exception = ommException1;

                try
                {
                    consumer!.Submit(new PostMsg(), invalidHandle);
                }
                catch (OmmException ommException2)
                {
                    Assert.Equal(OmmException.ExceptionType.OmmInvalidHandleException, ommException2.Type);
                    Assert.True(ommException2 is OmmInvalidHandleException);
                    numOfExcpetions++;

                   invalidHandleExcep = (OmmInvalidHandleException)ommException2;

                    Assert.StartsWith("Attempt to use invalid Handle on Submit(PostMsg)", invalidHandleExcep.Message);

                    exception = ommException2;
                }

                try
                {
                    consumer!.Reissue(new RequestMsg(), invalidHandle);
                }
                catch (OmmException ommException3)
                {
                    Assert.Equal(OmmException.ExceptionType.OmmInvalidHandleException, ommException3.Type);
                    Assert.True(ommException3 is OmmInvalidHandleException);
                    numOfExcpetions++;

                    invalidHandleExcep = (OmmInvalidHandleException)ommException3;

                    Assert.StartsWith("Attempt to use invalid Handle on Reissue()", invalidHandleExcep.Message);

                    exception = ommException3;
                }
            }

            Assert.NotNull(exception);
            Assert.Equal(3, numOfExcpetions);

            consumer?.Uninitialize();
            providerTest.UnInitialize();
        }

        class OmmConsumerErrorClientTest : IOmmConsumerErrorClient
        {
            public List<OmmInvalidHandleException> InvalidHandleExceptionList { get; private set; } = new();

            public List<OmmInvalidUsageException> InvalidUsageExceptionList { get; private set; }  = new();

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
        public void NotifyWithIOmmConsumerErrorClientTest()
        {
            ProviderSessionOptions providerSessionOpts = new ProviderSessionOptions();
            providerSessionOpts.SendMarketDataItemUpdate = true;
            providerSessionOpts.SendDictionaryResp = true;
            ProviderTest providerTest = new ProviderTest(providerSessionOpts, output);
            ReactorOptions reactorOptions = new ReactorOptions();
            providerTest.Initialize(reactorOptions);

            string hostString = $"localhost:{providerTest.ServerPort}";

            output.WriteLine($"Connect with {hostString}");

            OmmConsumerConfig config = new OmmConsumerConfig();
            OmmException? exception = null;
            OmmConsumer? consumer = null;
            long invalidHandle = 100;

            try
            {
                var consumerErrorClient = new OmmConsumerErrorClientTest();

                config.Host(hostString);

                consumer = new OmmConsumer(config.Host(hostString), consumerErrorClient);

                long handle = consumer.RegisterClient(null!, null!);

                Assert.Equal(0, handle);

                consumer.Submit(new GenericMsg(), invalidHandle);

                consumer.Submit(new PostMsg(), invalidHandle);

                consumer.Reissue(new RequestMsg(), invalidHandle);

                /* Checks OmmInvalidUsageException */
                Assert.True(consumerErrorClient.InvalidUsageExceptionList.Count == 1);
                Assert.Equal("A derived class of IOmmConsumerClient is not set",
                    consumerErrorClient.InvalidUsageExceptionList[0].Message);
                Assert.Equal(OmmInvalidUsageException.ErrorCodes.INVALID_ARGUMENT,
                    consumerErrorClient.InvalidUsageExceptionList[0].ErrorCode);

                /* Checks OmmInvalidHandleException */
                Assert.True(consumerErrorClient.InvalidHandleExceptionList.Count == 3);
                Assert.Equal(invalidHandle, consumerErrorClient.InvalidHandleExceptionList[0].Handle);
                Assert.StartsWith("Attempt to use invalid Handle on Submit(GenericMsg)", consumerErrorClient.InvalidHandleExceptionList[0].Message);

                Assert.Equal(invalidHandle, consumerErrorClient.InvalidHandleExceptionList[1].Handle);
                Assert.StartsWith("Attempt to use invalid Handle on Submit(PostMsg)", consumerErrorClient.InvalidHandleExceptionList[1].Message);

                Assert.Equal(invalidHandle, consumerErrorClient.InvalidHandleExceptionList[2].Handle);
                Assert.StartsWith("Attempt to use invalid Handle on Reissue()", consumerErrorClient.InvalidHandleExceptionList[2].Message);
            }
            catch (OmmException ommException)
            {

                exception = ommException; 
            }

            Assert.Null(exception);

            consumer?.Uninitialize();
            providerTest.UnInitialize();
        }

        [Fact]
        public void ReissueViewChangeTest()
        {
            ProviderSessionOptions providerSessionOpts = new ProviderSessionOptions();
            providerSessionOpts.SendDictionaryResp = true;
            providerSessionOpts.SendMarketDataItemUpdate = false;
            ProviderTest providerTest = new ProviderTest(providerSessionOpts, output);
            ReactorOptions reactorOptions = new ReactorOptions();
            providerTest.Initialize(reactorOptions);

            string hostString = $"localhost:{providerTest.ServerPort}";
            output.WriteLine($"Connect with {hostString}");

            OmmConsumerConfig config = new OmmConsumerConfig();
            OmmException? exception = null;
            OmmConsumer? consumer = null;

            OmmConsumerItemClientTest consumerClient;

            try
            {
                config.Host(hostString);

                config.OperationModel(OmmConsumerConfig.OperationModelMode.USER_DISPATCH);

                consumer = new OmmConsumer(config.Host(hostString));

                consumerClient = new OmmConsumerItemClientTest(consumer);

                ElementList payload = new ElementList();
                OmmArray array = new OmmArray();
                array.AddInt(MarketPriceItem.BID_FID).AddInt(MarketPriceItem.ASK_FID).MarkForClear().Complete();

                payload.AddUInt(EmaRdm.ENAME_VIEW_TYPE, 1)
                    .AddArray(EmaRdm.ENAME_VIEW_DATA, array).MarkForClear().Complete();

                RequestMsg requestMsg = new();
                requestMsg.DomainType(EmaRdm.MMT_MARKET_PRICE)
                    .ServiceId(ProviderTest.DefaultService.ServiceId).Name("ItemName")
                    .MarkForClear().Payload(payload);

                long handle = consumer.RegisterClient(requestMsg, consumerClient, this);
                int sequenceOfRefresh = 1;

                Assert.True(handle != 0);

                consumerClient.RefreshMsgHandler = (refreshMsg, consEvent) =>
                {
                    Assert.Equal((int)DomainType.MARKET_PRICE, refreshMsg.DomainType());
                    Assert.True(refreshMsg.ClearCache());
                    Assert.True(refreshMsg.MarkForClear().Complete());
                    Assert.True(refreshMsg.Solicited());
                    Assert.Equal(StreamStates.OPEN, refreshMsg.State().StreamState);
                    Assert.Equal(DataStates.OK, refreshMsg.State().DataState);
                    Assert.Equal(StateCodes.NONE, refreshMsg.State().StatusCode);
                    Assert.Equal("Item Refresh Completed", refreshMsg.State().StatusText);

                    Assert.True(refreshMsg.HasMsgKey);
                    Assert.True(refreshMsg.HasName);
                    Assert.Equal("ItemName", refreshMsg.Name());
                    Assert.True(refreshMsg.HasNameType);
                    Assert.Equal(InstrumentNameTypes.RIC, refreshMsg.NameType());
                    Assert.True(refreshMsg.HasServiceId);
                    Assert.Equal(ProviderTest.DefaultService.ServiceId, refreshMsg.ServiceId());
                    Assert.True(refreshMsg.HasServiceName);
                    Assert.Equal(ProviderTest.DefaultService.Info.ServiceName.ToString(), refreshMsg.ServiceName());

                    MarketPriceItem marketPriceItem = providerTest.MarketItemHandler
                        .MarketPriceItems[0];

                    Assert.Equal(DataType.DataTypes.FIELD_LIST, refreshMsg.MarkForClear().Payload().DataType);

                    FieldList fieldList = refreshMsg.MarkForClear().Payload().FieldList();
                    var fieldIt = fieldList.GetEnumerator();

                    Assert.True(fieldIt.MoveNext());
                    var fieldEntry = fieldIt.Current;

                    Assert.Equal(MarketPriceItem.BID_FID, fieldEntry.FieldId);
                    Assert.Equal(marketPriceItem.BID.ToString(), fieldEntry.OmmRealValue().ToString());

                    if (sequenceOfRefresh == 1) // Remove ASK_FID from the reissue to change view.
                    {
                        Assert.True(fieldIt.MoveNext());
                        fieldEntry = fieldIt.Current;

                        Assert.Equal(MarketPriceItem.ASK_FID, fieldEntry.FieldId);
                        Assert.Equal(marketPriceItem.ASK.ToString(), fieldEntry.OmmRealValue().ToString());
                    }

                    Assert.False(fieldIt.MoveNext());

                    sequenceOfRefresh++;
                };

                consumer.Dispatch(DispatchTimeout.INFINITE_WAIT);

                payload.Clear();
                array.Clear();
                array.AddInt(MarketPriceItem.BID_FID).MarkForClear().Complete();

                payload.AddUInt(EmaRdm.ENAME_VIEW_TYPE, 1)
                    .AddArray(EmaRdm.ENAME_VIEW_DATA, array).MarkForClear().Complete();

                requestMsg.Clear();
                requestMsg.DomainType(EmaRdm.MMT_MARKET_PRICE).MarkForClear().Payload(payload);

                consumer.Reissue(requestMsg, handle);

                consumer.Dispatch(DispatchTimeout.INFINITE_WAIT);

                // Provider receives 2 market data requests
                Assert.Equal(2, providerTest.MarketItemHandler.MarketPriceItemIndex);

                Assert.Equal(2, consumerClient.ReceivedOnAll);
                Assert.Equal(2, consumerClient.ReceivedOnRefresh);
                Assert.Equal(0, consumerClient.ReceivedOnStatus);
                Assert.Equal(0, consumerClient.ReceivedOnUpdate);

                consumer.Unregister(handle);
            }
            catch (OmmException ommException)
            {
                exception = ommException;
            }

            Assert.Null(exception);

            consumer?.Uninitialize();
            providerTest.UnInitialize();
        }

        [Fact]
        public void SubmitOnStreamPostMsgTest()
        {
            ProviderSessionOptions providerSessionOpts = new ProviderSessionOptions();
            providerSessionOpts.SendDictionaryResp = true;
            providerSessionOpts.SendMarketDataItemUpdate = false;
            ProviderTest providerTest = new ProviderTest(providerSessionOpts, output);
            ReactorOptions reactorOptions = new ReactorOptions();
            providerTest.Initialize(reactorOptions);

            string hostString = $"localhost:{providerTest.ServerPort}";
            output.WriteLine($"Connect with {hostString}");

            OmmConsumerConfig config = new OmmConsumerConfig();
            OmmException? exception = null;
            OmmConsumer? consumer = null;

            OmmConsumerItemClientTest consumerClient;

            try
            {
                config.Host(hostString);

                config.OperationModel(OmmConsumerConfig.OperationModelMode.USER_DISPATCH);

                consumer = new OmmConsumer(config.Host(hostString));

                output.WriteLine("Finished creating an OmmConsumer");

                consumerClient = new OmmConsumerItemClientTest(consumer);

                string itemName = "ItemName";

                RequestMsg requestMsg = new();
                requestMsg.DomainType(EmaRdm.MMT_MARKET_PRICE).ServiceId(ProviderTest.DefaultService.ServiceId)
                    .Name(itemName);

                long handle = consumer.RegisterClient(requestMsg, consumerClient, this);

                Assert.True(handle != 0);

                /* Checks the expected RefreshMsg from the market data request */
                consumerClient.RefreshMsgHandler = (refreshMsg, consEvent) =>
                {
                    Assert.Equal(handle, consEvent.Handle);
                    Assert.Same(this, consEvent.Closure);

                    CheckRefreshPayload(providerTest, 0, refreshMsg, itemName);
                };


                consumer.Dispatch(DispatchTimeout.INFINITE_WAIT);

                Assert.Equal(1, consumerClient.ReceivedOnAll);
                Assert.Equal(1, consumerClient.ReceivedOnRefresh);
                Assert.Equal(0, consumerClient.ReceivedOnStatus);
                Assert.Equal(0, consumerClient.ReceivedOnUpdate);

                // Submit Post message to the provider side.
                {
                    ElementList elementList = new();
                    elementList.AddInt("element1", 555).AddUInt("element2", 666).MarkForClear().Complete();
                    UpdateMsg updateMsg = new();
                    updateMsg.DomainType(EmaRdm.MMT_MARKET_PRICE).Name("updateMsg").MarkForClear().Payload(elementList);

                    consumer.Submit(new PostMsg().PostId(1).DomainType(EmaRdm.MMT_MARKET_PRICE).Name("postMsg")
                        .ServiceId(ProviderTest.DefaultService.ServiceId)
                        .SolicitAck(true).MarkForClear().Complete(true).MarkForClear().Payload(updateMsg), handle);

                    consumerClient.AckMsgHandler = (ackMsg, consEvent) =>
                    {
                        Assert.Equal(handle, consEvent.Handle);

                        // Check ack message from provider side
                        Assert.Equal(EmaRdm.MMT_MARKET_PRICE, ackMsg.DomainType());
                        Assert.True(ackMsg.HasName);
                        Assert.Equal("postMsg", ackMsg.Name());

                        Assert.Equal(DataType.DataTypes.UPDATE_MSG, ackMsg.MarkForClear().Payload().DataType);

                        UpdateMsg updateMsg = ackMsg.MarkForClear().Payload().UpdateMsg();
                        Assert.True(updateMsg.HasName);
                        Assert.Equal("updateMsg", updateMsg.Name());
                        Assert.Equal(EmaRdm.MMT_MARKET_PRICE, updateMsg.DomainType());

                        ElementList elementList = updateMsg.MarkForClear().Payload().ElementList();

                        var elementListIt = elementList.GetEnumerator();

                        Assert.True(elementListIt.MoveNext());
                        var elementEntry = elementListIt.Current;

                        Assert.Equal("element1", elementEntry.Name);
                        Assert.Equal(555, elementEntry.IntValue());

                        Assert.True(elementListIt.MoveNext());
                        elementEntry = elementListIt.Current;

                        Assert.Equal("element2", elementEntry.Name);
                        Assert.Equal((ulong)666, elementEntry.UIntValue());

                        Assert.False(elementListIt.MoveNext());
                    };

                    // Waiting for provider to send generic message back.
                    consumer.Dispatch(DispatchTimeout.INFINITE_WAIT);

                    Assert.Equal(2, consumerClient.ReceivedOnAll);
                    Assert.Equal(1, consumerClient.ReceivedOnAck);
                }

                consumer.Unregister(handle);

            }
            catch (OmmException ommException)
            {
                exception = ommException;
            }

            Assert.Null(exception);

            consumer?.Uninitialize();
            providerTest.UnInitialize();
        }


        [Fact]
        public void SubmitOffStreamPostMsgTest()
        {
            ProviderSessionOptions providerSessionOpts = new ProviderSessionOptions();
            providerSessionOpts.SendDictionaryResp = true;
            providerSessionOpts.SendMarketDataItemUpdate = false;
            ProviderTest providerTest = new ProviderTest(providerSessionOpts, output);
            ReactorOptions reactorOptions = new ReactorOptions();
            providerTest.Initialize(reactorOptions);

            string hostString = $"localhost:{providerTest.ServerPort}";
            output.WriteLine($"Connect with {hostString}");

            OmmConsumerConfig config = new OmmConsumerConfig();
            OmmException? exception = null;
            OmmConsumer? consumer = null;

            OmmConsumerItemClientTest consumerClient;

            try
            {
                config.Host(hostString);

                config.OperationModel(OmmConsumerConfig.OperationModelMode.USER_DISPATCH);

                consumer = new OmmConsumer(config.Host(hostString));

                output.WriteLine("Finished creating an OmmConsumer");

                consumerClient = new OmmConsumerItemClientTest(consumer);

                RequestMsg requestMsg = new();
                requestMsg.DomainType(EmaRdm.MMT_LOGIN);

                long loginHandle = consumer.RegisterClient(requestMsg, consumerClient, this);

                Assert.True(loginHandle != 0);

                /* Checks the expected RefreshMsg from the market data request */
                consumerClient.RefreshMsgHandler = (refreshMsg, consEvent) =>
                {
                    Assert.Equal(loginHandle, consEvent.Handle);
                    Assert.Same(this, consEvent.Closure);
                    Assert.Equal((int)DomainType.LOGIN, refreshMsg.DomainType());

                    Assert.True(refreshMsg.HasName);
                    Assert.Equal(Environment.UserName, refreshMsg.Name());

                    Assert.Equal(OmmState.StatusCodes.NONE, refreshMsg.State().StatusCode);
                    Assert.Equal(OmmState.DataStates.OK, refreshMsg.State().DataState);
                    Assert.Equal(OmmState.StreamStates.OPEN, refreshMsg.State().StreamState);
                };


                consumer.Dispatch(DispatchTimeout.INFINITE_WAIT);

                Assert.Equal(1, consumerClient.ReceivedOnAll);
                Assert.Equal(1, consumerClient.ReceivedOnRefresh);
                Assert.Equal(0, consumerClient.ReceivedOnStatus);
                Assert.Equal(0, consumerClient.ReceivedOnUpdate);

                // Submit Post message to the provider side.
                {
                    ElementList elementList = new();
                    elementList.AddInt("element1", 555).AddUInt("element2", 666).MarkForClear().Complete();
                    UpdateMsg updateMsg = new();
                    updateMsg.DomainType(EmaRdm.MMT_MARKET_PRICE).Name("updateMsg").MarkForClear().Payload(elementList);

                    consumer.Submit(new PostMsg().PostId(1).DomainType(EmaRdm.MMT_MARKET_PRICE).Name("postMsg")
                        .ServiceId(ProviderTest.DefaultService.ServiceId)
                        .SolicitAck(true).MarkForClear().Payload(updateMsg).MarkForClear().Complete(true), loginHandle);

                    consumerClient.AckMsgHandler = (ackMsg, consEvent) =>
                    {
                        Assert.Equal(loginHandle, consEvent.Handle);

                        // Check ack message from provider side
                        Assert.Equal(EmaRdm.MMT_MARKET_PRICE, ackMsg.DomainType());
                        Assert.True(ackMsg.HasName);
                        Assert.Equal("postMsg", ackMsg.Name());
                        Assert.Equal(1, ackMsg.StreamId());

                        Assert.Equal(DataType.DataTypes.UPDATE_MSG, ackMsg.MarkForClear().Payload().DataType);

                        UpdateMsg updateMsg = ackMsg.MarkForClear().Payload().UpdateMsg();
                        Assert.True(updateMsg.HasName);
                        Assert.Equal("updateMsg", updateMsg.Name());
                        Assert.Equal(EmaRdm.MMT_MARKET_PRICE, updateMsg.DomainType());

                        ElementList elementList = updateMsg.MarkForClear().Payload().ElementList();

                        var elementListIt = elementList.GetEnumerator();

                        Assert.True(elementListIt.MoveNext());
                        var elementEntry = elementListIt.Current;

                        Assert.Equal("element1", elementEntry.Name);
                        Assert.Equal(555, elementEntry.IntValue());

                        Assert.True(elementListIt.MoveNext());
                        elementEntry = elementListIt.Current;

                        Assert.Equal("element2", elementEntry.Name);
                        Assert.Equal((ulong)666, elementEntry.UIntValue());

                        Assert.False(elementListIt.MoveNext());
                    };

                    // Waiting for provider to send generic message back.
                    consumer.Dispatch(DispatchTimeout.INFINITE_WAIT);

                    Assert.Equal(2, consumerClient.ReceivedOnAll);
                    Assert.Equal(1, consumerClient.ReceivedOnAck);
                }

                consumer.Unregister(loginHandle);

            }
            catch (OmmException ommException)
            {
                exception = ommException;
            }

            Assert.Null(exception);

            consumer?.Uninitialize();
            providerTest.UnInitialize();
        }

        [Fact]
        public void SubmitGenericMsgTest()
        {
            ProviderSessionOptions providerSessionOpts = new ProviderSessionOptions();
            providerSessionOpts.SendDictionaryResp = true;
            providerSessionOpts.SendMarketDataItemUpdate = false;
            ProviderTest providerTest = new ProviderTest(providerSessionOpts, output);
            ReactorOptions reactorOptions = new ReactorOptions();
            providerTest.Initialize(reactorOptions);

            string hostString = $"localhost:{providerTest.ServerPort}";
            output.WriteLine($"Connect with {hostString}");

            OmmConsumerConfig config = new OmmConsumerConfig();
            OmmException? exception = null;
            OmmConsumer? consumer = null;

            OmmConsumerItemClientTest consumerClient;

            try
            {
                config.Host(hostString);

                config.OperationModel(OmmConsumerConfig.OperationModelMode.USER_DISPATCH);

                consumer = new OmmConsumer(config.Host(hostString));

                output.WriteLine("Finished creating an OmmConsumer");

                consumerClient = new OmmConsumerItemClientTest(consumer);

                string itemName = "ItemName";

                RequestMsg requestMsg = new();
                requestMsg.DomainType(EmaRdm.MMT_MARKET_PRICE).ServiceName(ProviderTest.DefaultService.Info.ServiceName.ToString())
                    .Name(itemName).InterestAfterRefresh(true);

                long handle = consumer.RegisterClient(requestMsg, consumerClient, this);

                Assert.True(handle > 0);

                /* Checks the expected RefreshMsg from the market data request */
                consumerClient.RefreshMsgHandler = (refreshMsg, consEvent) =>
                {
                    Assert.Equal(handle, consEvent.Handle);

                    CheckRefreshPayload(providerTest, 0, refreshMsg, itemName);
                };


                consumer.Dispatch(DispatchTimeout.INFINITE_WAIT);

                Assert.Equal(1, consumerClient.ReceivedOnAll);
                Assert.Equal(1, consumerClient.ReceivedOnRefresh);
                Assert.Equal(0, consumerClient.ReceivedOnStatus);
                Assert.Equal(0, consumerClient.ReceivedOnUpdate);

                // Submit generic message to the provider side.
                {
                    ElementList elementList = new();
                    elementList.AddInt("element1", 555).AddUInt("element2", 666).MarkForClear().Complete();
                    consumer.Submit(new GenericMsg().DomainType(200).Name("genericMsg").MarkForClear().Payload(elementList),
                        handle);

                    consumerClient.GenericMsgHandler = (genericMsg, consEvent) =>
                    {
                        Assert.Equal(handle, consEvent.Handle);
                        Assert.Same(this, consEvent.Closure);

                        Assert.Equal(200, genericMsg.DomainType());
                        Assert.Equal("genericMsg", genericMsg.Name());

                        // Check generic message from provider side
                        Assert.Equal(DataType.DataTypes.ELEMENT_LIST, genericMsg.MarkForClear().Payload().DataType);

                        ElementList elementList = genericMsg.MarkForClear().Payload().ElementList();

                        var elementListIt = elementList.GetEnumerator();

                        Assert.True(elementListIt.MoveNext());
                        var elementEntry = elementListIt.Current;

                        Assert.Equal("element1", elementEntry.Name);
                        Assert.Equal(555, elementEntry.IntValue());

                        Assert.True(elementListIt.MoveNext());
                        elementEntry = elementListIt.Current;

                        Assert.Equal("element2", elementEntry.Name);
                        Assert.Equal((ulong)666, elementEntry.UIntValue());

                        Assert.False(elementListIt.MoveNext());
                    };

                    // Waiting for provider to send generic message back.
                    consumer.Dispatch(DispatchTimeout.INFINITE_WAIT);

                    Assert.Equal(2, consumerClient.ReceivedOnAll);
                    Assert.Equal(1, consumerClient.ReceivedOnGeneric);
                }

                consumer.Unregister(handle);

            }
            catch (OmmException ommException)
            {
                exception = ommException;
            }

            Assert.Null(exception);

            consumer?.Uninitialize();
            providerTest.UnInitialize();
        }

        [Theory]
        [InlineData(false)]
        [InlineData(true)]
        public void RegisterMarketDataStreamingAndSnapshotRequestsTest(bool userDispatch = false)
        {
            ProviderSessionOptions providerSessionOpts = new ProviderSessionOptions();
            providerSessionOpts.SendMarketDataItemUpdate = true;
            providerSessionOpts.SendDictionaryResp = true;
            ProviderTest providerTest = new ProviderTest(providerSessionOpts, output);
            ReactorOptions reactorOptions = new ReactorOptions();
            providerTest.Initialize(reactorOptions);

            string hostString = $"localhost:{providerTest.ServerPort}";

            output.WriteLine($"userDispatch = {userDispatch}");
            output.WriteLine($"Connect with {hostString}");


            OmmConsumerConfig config = new OmmConsumerConfig();
            OmmException? exception = null;
            OmmConsumer? consumer = null;

            OmmConsumerItemClientTest consumerClient;
            int refreshIndex = 0;
            State m_State = new();

            try
            {
                config.Host(hostString);

                if (userDispatch)
                    config.OperationModel(OmmConsumerConfig.OperationModelMode.USER_DISPATCH);

                consumer = new OmmConsumer(config.Host(hostString));

                output.WriteLine("Finished creating an OmmConsumer");

                consumerClient = new OmmConsumerItemClientTest(consumer);

                string[] itemNames = { "ItemName", "ItemName2" };

                RequestMsg requestMsg = new();
                requestMsg.DomainType(EmaRdm.MMT_MARKET_PRICE).ServiceName(ProviderTest.DefaultService.Info.ServiceName.ToString())
                    .Name(itemNames[0]).InterestAfterRefresh(true);

                RequestMsg requestMsg2 = new();
                requestMsg2.DomainType(EmaRdm.MMT_MARKET_PRICE).ServiceName(ProviderTest.DefaultService.Info.ServiceName.ToString())
                   .Name(itemNames[1]).InterestAfterRefresh(false);

                long handle = consumer.RegisterClient(requestMsg, consumerClient, this);

                Assert.True(handle > 0);

                long handle2 = consumer.RegisterClient(requestMsg2, consumerClient, this);

                Assert.True(handle2 > 0);

                /* Checks the expected RefreshMsg from the market data request */
                consumerClient.RefreshMsgHandler = (refreshMsg, consEvent) =>
                {
                    Assert.Same(this, consEvent.Closure);

                    if (refreshIndex == 0)
                    {
                        Assert.Equal(handle, consEvent.Handle);
                        m_State.StreamState(StreamStates.OPEN);
                        m_State.DataState(DataStates.OK);
                        m_State.Code(StateCodes.NONE);
                        m_State.Text().Data("Item Refresh Completed");
                    }
                    else
                    {
                        Assert.Equal(handle2, consEvent.Handle);
                        m_State.StreamState(StreamStates.NON_STREAMING);
                        m_State.DataState(DataStates.OK);
                        m_State.Code(StateCodes.NONE);
                        m_State.Text().Data("Item Refresh Completed");
                    }

                    CheckRefreshPayload(providerTest, refreshIndex, refreshMsg, itemNames[refreshIndex], m_State);

                    ++refreshIndex;
                };

                consumerClient.UpdateMsgHandler = (updateMsg, consEvent) =>
                {
                    Assert.Equal(handle, consEvent.Handle);
                    Assert.Same(this, consEvent.Closure);
                    Assert.Equal(DataType.DataTypes.FIELD_LIST, updateMsg.MarkForClear().Payload().DataType);

                    CheckUpdatePayload(providerTest, 0, updateMsg);
                };

                if (userDispatch)
                {
                    consumer.Dispatch(DispatchTimeout.INFINITE_WAIT);
                    consumer.Dispatch(DispatchTimeout.INFINITE_WAIT);
                    consumer.Dispatch(5000);
                }
                else
                {
                    Thread.Sleep(12000);
                }

                Assert.Equal(3, consumerClient.ReceivedOnAll);
                Assert.Equal(2, consumerClient.ReceivedOnRefresh);
                Assert.Equal(0, consumerClient.ReceivedOnStatus);
                Assert.Equal(1, consumerClient.ReceivedOnUpdate);

                consumer.Unregister(handle);
                consumer.Unregister(handle2);
            }
            catch (OmmException ommException)
            {
                exception = ommException;
            }

            Assert.Null(exception);

            consumer?.Uninitialize();
            providerTest.UnInitialize();
        }
    }
}
