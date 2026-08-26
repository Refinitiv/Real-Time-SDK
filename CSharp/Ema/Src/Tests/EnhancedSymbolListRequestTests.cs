/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2026 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

using System;
using System.Collections.Generic;
using System.Linq;
using System.Runtime.InteropServices;
using System.Text;
using System.Threading.Tasks;
using LSEG.Ema.Access.Tests.OmmConsumerTests;
using LSEG.Ema.Access.Tests.RequestRouting;
using LSEG.Ema.Rdm;
using LSEG.Eta.Codec;
using LSEG.Eta.Common;
using LSEG.Eta.Rdm;

using static LSEG.Ema.Access.Tests.OmmConfigTests.ConfigTestsUtils;


namespace LSEG.Ema.Access.Tests
{
    public class EnhancedSymbolListRequestTests
    {
        private static readonly string EmaConfigFileLocation = BASE_TEST_CONFIG_PATH + "/RequestRouting/EmaConfigTest.xml";

        ITestOutputHelper output;

        public EnhancedSymbolListRequestTests(ITestOutputHelper output)
        {
            this.output = output;
        }

        [Fact]
        public void SubmitSymbolListReqMsgTest()
        {
            OmmIProviderConfig providerConfig = new(EmaConfigFileLocation);
            ProviderTestOptions providerTestOptions = new ProviderTestOptions();
            ProviderTestClient providerClient = new ProviderTestClient(output, providerTestOptions);

            OmmProvider ommProvider = new OmmProvider(providerConfig.Port("19001"), providerClient);

            ConsumerTestOptions options = new();
            options.GetChannelInformation = true;
            using ConsumerTestClient consumerClient = new ConsumerTestClient(output, options);

            OmmConsumer? consumer = null;

            try
            {
                ElementList payload = new ElementList();
                ElementEntry entry = new ElementEntry();

                ElementList eePayload = new ElementList();
                ElementEntry eeEntry = new ElementEntry();

                eePayload.AddUInt(":DataStreams", SymbolList.SymbolListDataStreamRequestFlags.SYMBOL_LIST_DATA_STREAMS);
                eePayload.Complete();

                payload.AddElementList(":SymbolListBehaviors", eePayload);
                payload.Complete();

                RequestMsg reqMsg = new RequestMsg();
                reqMsg.DomainType(EmaRdm.MMT_SYMBOL_LIST).ServiceName("DIRECT_FEED").Name(".AV.N").Payload(payload);

                consumer = new OmmConsumer(new OmmConsumerConfig(EmaConfigFileLocation).ConsumerName("Consumer_1"), consumerClient);

                OmmConsumerImpl ommConsumerImpl = consumer.m_OmmConsumerImpl!;

                Thread.Sleep(1500);

                Msg msg = consumerClient.WaitForMessage<Msg>();
                msg.MarkForClear();

                long handle = consumer.RegisterClient(reqMsg, consumerClient);

                Thread.Sleep(3000);

                CheckStandardSymbolListResponse(consumerClient);

                reqMsg.MarkForClear();
                payload.MarkForClear();
                eePayload.MarkForClear();

            }
            finally
            {
                consumer?.Uninitialize();
                if (ommProvider != null) ommProvider.Uninitialize();
            }
        }

        [Fact]
        public void SubmitSymbolListReqMsgTest_SameRequestAsUser()
        {
            OmmIProviderConfig providerConfig = new(EmaConfigFileLocation);
            ProviderTestOptions providerTestOptions = new ProviderTestOptions();
            ProviderTestClient providerClient = new ProviderTestClient(output, providerTestOptions);

            OmmProvider ommProvider = new OmmProvider(providerConfig.Port("19001"), providerClient);

            ConsumerTestOptions options = new();
            options.GetChannelInformation = true;
            using ConsumerTestClient consumerClient = new ConsumerTestClient(output, options);

            OmmConsumer? consumer = null;

            try
            {
                ElementList payload = new ElementList();
                ElementEntry entry = new ElementEntry();

                ElementList eePayload = new ElementList();
                ElementEntry eeEntry = new ElementEntry();

                eePayload.AddUInt(":DataStreams", SymbolList.SymbolListDataStreamRequestFlags.SYMBOL_LIST_DATA_STREAMS);
                eePayload.Complete();

                payload.AddElementList(":SymbolListBehaviors", eePayload);
                payload.Complete();

                RequestMsg reqMsg = new RequestMsg();
                reqMsg.DomainType(EmaRdm.MMT_SYMBOL_LIST).ServiceName("DIRECT_FEED").Name(".AV.N").Payload(payload);

                consumer = new OmmConsumer(new OmmConsumerConfig(EmaConfigFileLocation).ConsumerName("Consumer_1"), consumerClient);

                OmmConsumerImpl ommConsumerImpl = consumer.m_OmmConsumerImpl!;

                Thread.Sleep(1500);

                Msg msg = consumerClient.WaitForMessage<Msg>();
                msg.MarkForClear();

                long handle = consumer.RegisterClient(reqMsg, consumerClient);

                Thread.Sleep(3000);

                //CheckStandardSymbolListResponse(consumerClient);

                reqMsg.MarkForClear();
                payload.MarkForClear();
                eePayload.MarkForClear();

                long itemAHandle = consumerClient.NameHandleMap["itemA"];

                reqMsg = new RequestMsg();
                reqMsg.DomainType(EmaRdm.MMT_MARKET_PRICE).ServiceName("DIRECT_FEED").Name("itemA");

                long customerItemAHandle = consumer.RegisterClient(reqMsg, consumerClient);

                Thread.Sleep(1500);

                consumer.Unregister(itemAHandle);

                Thread.Sleep(1500);

                consumer.Unregister(customerItemAHandle);

            }
            finally
            {
                consumer?.Uninitialize();
                if (ommProvider != null) ommProvider.Uninitialize();
            }
        }

        [Fact]
        public void TestRequestingSymbolListItem_MarketPriceItemsRecovery()
        {
            /*
                Consumer application requests Symbol List item, receives Refresh for it.
                API automatically requests Market Price items provided in Symbol List Refresh, consumer gets Refreshes for them.
                Consumer successfully unregisters one of these Market Price items and the Symbol List item itself.
                Then Provider goes down and up again. The Consumer application successfully recovers the two remaining
                Market Price items, but not the Symbol List and Market Price items consumer unregistered.
            */

            OmmIProviderConfig providerConfig = new(EmaConfigFileLocation);
            ProviderTestOptions providerTestOptions = new ProviderTestOptions();
            ProviderTestClient providerClient = new ProviderTestClient(output, providerTestOptions);

            OmmProvider ommProvider = new OmmProvider(providerConfig.Port("19001"), providerClient);

            ConsumerTestOptions options = new();
            options.GetChannelInformation = true;
            using ConsumerTestClient consumerClient = new ConsumerTestClient(output, options);

            OmmConsumer? consumer = null;

            try
            {
                ElementList payload = new ElementList();
                ElementEntry entry = new ElementEntry();

                ElementList eePayload = new ElementList();
                ElementEntry eeEntry = new ElementEntry();

                eePayload.AddUInt(":DataStreams", SymbolList.SymbolListDataStreamRequestFlags.SYMBOL_LIST_DATA_STREAMS);
                eePayload.Complete();

                payload.AddElementList(":SymbolListBehaviors", eePayload);
                payload.Complete();

                RequestMsg reqMsg = new RequestMsg();
                reqMsg.DomainType(EmaRdm.MMT_SYMBOL_LIST).ServiceName("DIRECT_FEED").Name(".AV.N").Payload(payload);

                consumer = new OmmConsumer(new OmmConsumerConfig(EmaConfigFileLocation).ConsumerName("Consumer_1"), consumerClient);

                OmmConsumerImpl ommConsumerImpl = consumer.m_OmmConsumerImpl!;

                Thread.Sleep(1500);

                Msg msg = consumerClient.WaitForMessage<Msg>();
                msg.MarkForClear();

                long itemHandle = consumer.RegisterClient(reqMsg, consumerClient);

                Thread.Sleep(3000);

                CheckStandardSymbolListResponse(consumerClient);

                int count = providerClient.QueueSize();
                for (int i = 0; i < count; i++) { msg = providerClient.PopMessage(); msg.MarkForClear(); }

                long itemBHandle = consumerClient.NameHandleMap["itemB"];
                consumer.Unregister(itemBHandle);

                Thread.Sleep(1000);

                Assert.Equal(1, providerClient.QueueSize());
                Msg message = providerClient.PopMessage();
                Assert.True(message is RequestMsg);
                RequestMsg requestMsg = (RequestMsg)message;
                Assert.Equal("itemB", requestMsg.Name());

                consumer.Unregister(itemHandle);

                ommProvider.Uninitialize();

                providerClient = new ProviderTestClient(output, providerTestOptions);
                ommProvider = new OmmProvider(providerConfig.Port("19001"), providerClient);

                Thread.Sleep(8000);

                HashSet<string> names = new();
                count = providerClient.QueueSize();
                for (int i = 0; i < count; i++)
                {
                    message = providerClient.PopMessage();
                    if (message is RequestMsg)
                    {
                        requestMsg = (RequestMsg)message;
                        if (requestMsg.HasName
                                && (requestMsg.Name().Equals("itemA") || requestMsg.Name().Equals("itemB")
                                || requestMsg.Name().Equals("itemC") || requestMsg.Name().Equals(".AV.N")))
                        {
                            names.Add(requestMsg.Name());
                        }
                    }
                    message.MarkForClear();
                }

                Assert.Equal(2, names.Count);
                Assert.DoesNotContain("itemB", names);
                Assert.DoesNotContain(".AV.N", names);

                long itemAHandle = consumerClient.NameHandleMap["itemA"];
                consumer.Unregister(itemAHandle);

                Thread.Sleep(1000);

                Assert.Equal(1, providerClient.QueueSize());
                message = providerClient.PopMessage();
                Assert.True(message is RequestMsg);
                requestMsg = (RequestMsg)message;
                Assert.Equal("itemA", requestMsg.Name());
                message.MarkForClear();

                reqMsg.MarkForClear();
                payload.MarkForClear();
                eePayload.MarkForClear();
                message.MarkForClear();
                requestMsg.MarkForClear();
            }
            finally
            {
                consumer?.Uninitialize();
                if (ommProvider != null) ommProvider.Uninitialize();
            }
        }

        [Fact]
        public void TestRequestingSymbolListItem_SymbolListAndMarketPriceItemsRecovery()
        {
            /*
                Consumer application requests Symbol List item, receives Refresh for it.
                API automatically requests Market Price items provided in Symbol List Refresh, consumer gets Refreshes for them.
                Consumer successfully unregisters one of these Market Price items.
                Then Provider goes down and up again.
                The Consumer application successfully recovers all four items - the Symbol List item and the three
                Market Price items associated with it (the two Market Price items that were not unregistered before
                Provider restart should not be requested twice!).
             */

            OmmIProviderConfig providerConfig = new(EmaConfigFileLocation);
            ProviderTestOptions providerTestOptions = new ProviderTestOptions();
            ProviderTestClient providerClient = new ProviderTestClient(output, providerTestOptions);

            OmmProvider ommProvider = new OmmProvider(providerConfig.Port("19001"), providerClient);

            ConsumerTestOptions options = new();
            options.GetChannelInformation = true;
            using ConsumerTestClient consumerClient = new ConsumerTestClient(output, options);

            OmmConsumer? consumer = null;

            try
            {
                ElementList payload = new ElementList();
                ElementEntry entry = new ElementEntry();

                ElementList eePayload = new ElementList();
                ElementEntry eeEntry = new ElementEntry();

                eePayload.AddUInt(":DataStreams", SymbolList.SymbolListDataStreamRequestFlags.SYMBOL_LIST_DATA_STREAMS);
                eePayload.Complete();

                payload.AddElementList(":SymbolListBehaviors", eePayload);
                payload.Complete();

                RequestMsg reqMsg = new RequestMsg();
                reqMsg.DomainType(EmaRdm.MMT_SYMBOL_LIST).ServiceName("DIRECT_FEED").Name(".AV.N").Payload(payload);

                consumer = new OmmConsumer(new OmmConsumerConfig(EmaConfigFileLocation).ConsumerName("Consumer_1"), consumerClient);

                OmmConsumerImpl ommConsumerImpl = consumer.m_OmmConsumerImpl!;

                Thread.Sleep(1500);

                Msg msg = consumerClient.WaitForMessage<Msg>();
                msg.MarkForClear();

                int count = providerClient.QueueSize();
                for (int i = 0; i < count; i++) { msg = providerClient.PopMessage(); msg.MarkForClear(); }

                long itemHandle = consumer.RegisterClient(reqMsg, consumerClient);

                Thread.Sleep(3000);

                reqMsg.MarkForClear();
                payload.MarkForClear();
                eePayload.MarkForClear();

                Assert.Equal(4, providerClient.QueueSize());

                Msg message = providerClient.PopMessage();

                Assert.True(message is RequestMsg);
                RequestMsg requestMsg = (RequestMsg)message;
                message.MarkForClear();

                Assert.Equal("DIRECT_FEED", requestMsg.ServiceName());
                Assert.Equal(".AV.N", requestMsg.Name());

                HashSet<string> names = new HashSet<string>();
                for (int i = 0; i < 3; i++)
                {
                    message = providerClient.PopMessage();
                    requestMsg = (RequestMsg)message;
                    Assert.Equal("DIRECT_FEED", requestMsg.ServiceName());
                    Assert.True("itemA".Equals(requestMsg.Name()) || "itemB".Equals(requestMsg.Name()) || "itemC".Equals(requestMsg.Name()));
                    names.Add(requestMsg.Name());
                    message.MarkForClear();
                }
                Assert.Equal(3, names.Count);

                Assert.Equal(4, consumerClient.QueueSize());

                message = consumerClient.PopMessage();
                Assert.True(message is RefreshMsg);

                RefreshMsg refresh = (RefreshMsg)message;
                Assert.True(refresh.HasName);
                Assert.Equal(".AV.N", refresh.Name());

                message.MarkForClear();

                names.Clear();
                for (int i = 0; i < 3; i++)
                {
                    message = consumerClient.PopMessage();
                    Assert.True(message is RefreshMsg);

                    refresh = (RefreshMsg)message;
                    Assert.True(refresh.HasName);
                    Assert.True("itemA".Equals(refresh.Name()) || "itemB".Equals(refresh.Name()) || "itemC".Equals(refresh.Name()));
                    names.Add(refresh.Name());

                    message.MarkForClear();
                }
                Assert.Equal(3, names.Count);

                long itemBHandle = consumerClient.NameHandleMap["itemB"];
                consumer.Unregister(itemBHandle);

                Thread.Sleep(1000);

                Assert.Equal(1, providerClient.QueueSize());
                message = providerClient.PopMessage();
                Assert.True(message is RequestMsg);
                requestMsg = (RequestMsg)message;
                Assert.Equal("itemB", requestMsg.Name());
                message.MarkForClear();

                ommProvider.Uninitialize();

                providerClient = new ProviderTestClient(output, providerTestOptions);
                ommProvider = new OmmProvider(providerConfig.Port("19001"), providerClient);

                Thread.Sleep(8000);

                names.Clear();
                count = providerClient.QueueSize();
                for (int i = 0; i < count; i++)
                {
                    message = providerClient.PopMessage();
                    if (message is RequestMsg)
                    {
                        requestMsg = (RequestMsg)message;
                        if (requestMsg.HasName
                                && (requestMsg.Name().Equals("itemA") || requestMsg.Name().Equals("itemB")
                                || requestMsg.Name().Equals("itemC") || requestMsg.Name().Equals(".AV.N")))
                        {
                            names.Add(requestMsg.Name());
                        }
                    }
                    message.MarkForClear();
                }
                Assert.Equal(4, names.Count);

                long itemAHandle = consumerClient.NameHandleMap["itemA"];
                consumer.Unregister(itemAHandle);

                Thread.Sleep(1000);

                Assert.Equal(1, providerClient.QueueSize());
                message = providerClient.PopMessage();
                Assert.True(message is RequestMsg);
                requestMsg = (RequestMsg)message;
                Assert.Equal("itemA", requestMsg.Name());

                message.MarkForClear();

                Assert.NotNull(consumer);
            }
            catch (Exception e)
            {
                output.WriteLine(e.Message);
                output.WriteLine(e.StackTrace ?? "Exception: null stack trace");
                Assert.False(true);
            }
            finally
            {
                output.WriteLine("Uninitializing...");

                if (consumer != null) consumer.Uninitialize();
                if (ommProvider != null) ommProvider.Uninitialize();
            }
        }

        [Fact]
        public void TestRequestingSymbolListItem_SymbolListAndMarketPriceItemsRecovery_PrivateStream()
        {
            /*
                Consumer requests symbol list item on private stream, receives Refresh for Symbol List with 3 Market Price items.
                Consumer unregisters one item. After that provider restarts. Consumer doesn't recover Symbol List item since it is private
                but recovers the remaining market price items
            */

            OmmIProviderConfig providerConfig = new(EmaConfigFileLocation);
            ProviderTestOptions providerTestOptions = new ProviderTestOptions();
            ProviderTestClient providerClient = new ProviderTestClient(output, providerTestOptions);

            OmmProvider ommProvider = new OmmProvider(providerConfig.Port("19001"), providerClient);

            ConsumerTestOptions options = new();
            options.GetChannelInformation = true;
            using ConsumerTestClient consumerClient = new ConsumerTestClient(output, options);

            OmmConsumer? consumer = null;

            try
            {
                ElementList payload = new ElementList();
                ElementEntry entry = new ElementEntry();

                ElementList eePayload = new ElementList();
                ElementEntry eeEntry = new ElementEntry();

                eePayload.AddUInt(":DataStreams", SymbolList.SymbolListDataStreamRequestFlags.SYMBOL_LIST_DATA_STREAMS);
                eePayload.Complete();

                payload.AddElementList(":SymbolListBehaviors", eePayload);
                payload.Complete();

                RequestMsg reqMsg = new RequestMsg();
                reqMsg.DomainType(EmaRdm.MMT_SYMBOL_LIST).ServiceName("DIRECT_FEED").Name(".AV.N").Payload(payload).PrivateStream(true);

                consumer = new OmmConsumer(new OmmConsumerConfig(EmaConfigFileLocation).ConsumerName("Consumer_1"), consumerClient);

                OmmConsumerImpl ommConsumerImpl = consumer.m_OmmConsumerImpl!;

                Thread.Sleep(1500);

                Msg msg = consumerClient.WaitForMessage<Msg>();
                msg.MarkForClear();

                int count = providerClient.QueueSize();
                for (int i = 0; i < count; i++) { msg = providerClient.PopMessage(); msg.MarkForClear(); }

                long itemHandle = consumer.RegisterClient(reqMsg, consumerClient);

                Thread.Sleep(3000);

                reqMsg.MarkForClear();
                payload.MarkForClear();
                eePayload.MarkForClear();

                Assert.Equal(4, providerClient.QueueSize());

                Msg message = providerClient.PopMessage();

                Assert.True(message is RequestMsg);
                RequestMsg requestMsg = (RequestMsg)message;
                message.MarkForClear();

                Assert.Equal("DIRECT_FEED", requestMsg.ServiceName());
                Assert.Equal(".AV.N", requestMsg.Name());

                HashSet<string> names = new HashSet<string>();
                for (int i = 0; i < 3; i++)
                {
                    message = providerClient.PopMessage();
                    requestMsg = (RequestMsg)message;
                    Assert.Equal("DIRECT_FEED", requestMsg.ServiceName());
                    Assert.True("itemA".Equals(requestMsg.Name()) || "itemB".Equals(requestMsg.Name()) || "itemC".Equals(requestMsg.Name()));
                    names.Add(requestMsg.Name());
                    message.MarkForClear();
                }
                Assert.Equal(3, names.Count);

                Assert.Equal(4, consumerClient.QueueSize());

                message = consumerClient.PopMessage();
                Assert.True(message is RefreshMsg);

                RefreshMsg refresh = (RefreshMsg)message;
                Assert.True(refresh.HasName);
                Assert.Equal(".AV.N", refresh.Name());

                message.MarkForClear();

                names.Clear();
                for (int i = 0; i < 3; i++)
                {
                    message = consumerClient.PopMessage();
                    Assert.True(message is RefreshMsg);

                    refresh = (RefreshMsg)message;
                    Assert.True(refresh.HasName);
                    Assert.True("itemA".Equals(refresh.Name()) || "itemB".Equals(refresh.Name()) || "itemC".Equals(refresh.Name()));
                    names.Add(refresh.Name());

                    message.MarkForClear();
                }
                Assert.Equal(3, names.Count);

                long itemBHandle = consumerClient.NameHandleMap["itemB"];
                consumer.Unregister(itemBHandle);

                Thread.Sleep(1000);

                Assert.Equal(1, providerClient.QueueSize());
                message = providerClient.PopMessage();
                Assert.True(message is RequestMsg);
                requestMsg = (RequestMsg)message;
                Assert.Equal("itemB", requestMsg.Name());

                message.MarkForClear();

                ommProvider.Uninitialize();

                providerClient = new ProviderTestClient(output, providerTestOptions);
                ommProvider = new OmmProvider(providerConfig.Port("19001"), providerClient);

                Thread.Sleep(4000);

                names.Clear();
                count = providerClient.QueueSize();
                for (int i = 0; i < count; i++)
                {
                    message = providerClient.PopMessage();
                    if (message is RequestMsg)
                    {
                        requestMsg = (RequestMsg)message;
                        if (requestMsg.HasName
                                && (requestMsg.Name().Equals("itemA") || requestMsg.Name().Equals("itemB")
                                || requestMsg.Name().Equals("itemC") || requestMsg.Name().Equals(".AV.N")))
                        {
                            names.Add(requestMsg.Name());
                        }
                    }
                    message.MarkForClear();
                }
                Assert.Equal(2, names.Count); // private stream is not recovered

                long itemAHandle = consumerClient.NameHandleMap["itemA"];
                consumer.Unregister(itemAHandle);

                Thread.Sleep(1000);

                Assert.Equal(1, providerClient.QueueSize());
                message = providerClient.PopMessage();
                Assert.True(message is RequestMsg);
                requestMsg = (RequestMsg)message;
                Assert.Equal("itemA", requestMsg.Name());

                message.MarkForClear();

                Assert.NotNull(consumer);
            }
            catch (Exception e)
            {
                output.WriteLine(e.Message);
                output.WriteLine(e.StackTrace ?? "Exception: null stack trace");
                Assert.False(true);
            }
            finally
            {
                output.WriteLine("Uninitializing...");

                if (consumer != null) consumer.Uninitialize();
                if (ommProvider != null) ommProvider.Uninitialize();
            }
        }

        [Fact]
        public void TestRequestingTwoSymbolListItems_SameItemListInRefresh()
        {
            /*
                Consumer sends two Symbol List requests for different items (service is the same).
                Market Price items are the same in Refreshes for both Symbol List items.
                Consumer application should receive Market Price item refreshes only for one set of Market Price items.
             */
            OmmIProviderConfig providerConfig = new(EmaConfigFileLocation);
            ProviderTestOptions providerTestOptions = new ProviderTestOptions();
            ProviderTestClient providerClient = new ProviderTestClient(output, providerTestOptions);

            OmmProvider ommProvider = new OmmProvider(providerConfig.Port("19001"), providerClient);

            ConsumerTestOptions options = new();
            options.GetChannelInformation = true;
            using ConsumerTestClient consumerClient = new ConsumerTestClient(output, options);

            OmmConsumer? consumer = null;

            try
            {
                ElementList payload = new ElementList();
                ElementEntry entry = new ElementEntry();

                ElementList eePayload = new ElementList();
                ElementEntry eeEntry = new ElementEntry();

                eePayload.AddUInt(":DataStreams", SymbolList.SymbolListDataStreamRequestFlags.SYMBOL_LIST_DATA_STREAMS);
                eePayload.Complete();

                payload.AddElementList(":SymbolListBehaviors", eePayload);
                payload.Complete();

                RequestMsg reqMsg = new RequestMsg();
                reqMsg.DomainType(EmaRdm.MMT_SYMBOL_LIST).ServiceName("DIRECT_FEED").Name(".AV.N").Payload(payload);

                consumer = new OmmConsumer(new OmmConsumerConfig(EmaConfigFileLocation).ConsumerName("Consumer_1"), consumerClient);

                OmmConsumerImpl ommConsumerImpl = consumer.m_OmmConsumerImpl!;

                Thread.Sleep(1500);

                Msg msg = consumerClient.WaitForMessage<Msg>();
                msg.MarkForClear();

                int count = providerClient.QueueSize();
                for (int i = 0; i < count; i++) { msg = providerClient.PopMessage(); msg.MarkForClear(); }

                long itemHandle = consumer.RegisterClient(reqMsg, consumerClient);
                long itemHandle2 = consumer.RegisterClient(reqMsg.Clear().Name(".BV.N").DomainType(EmaRdm.MMT_SYMBOL_LIST).ServiceName("DIRECT_FEED").Payload(payload), consumerClient);

                Thread.Sleep(3000);

                reqMsg.MarkForClear();
                payload.MarkForClear();
                eePayload.MarkForClear();

                Assert.Equal(5, providerClient.QueueSize());

                Msg message;
                RequestMsg requestMsg;

                HashSet<string> slNames = new();
                for (int i = 0; i < 2; i++)
                {
                    message = providerClient.PopMessage();
                    Assert.True(message is RequestMsg);
                    requestMsg = (RequestMsg)message;

                    Assert.Equal("DIRECT_FEED", requestMsg.ServiceName());
                    Assert.True(".AV.N".Equals(requestMsg.Name()) || ".BV.N".Equals(requestMsg.Name()));

                    slNames.Add(requestMsg.Name());

                    message.MarkForClear();
                }
                Assert.Equal(2, slNames.Count);

                HashSet<String> names = new HashSet<String>();
                for (int i = 0; i < 3; i++)
                {
                    message = providerClient.PopMessage();
                    Assert.True(message is RequestMsg);
                    requestMsg = (RequestMsg)message;
                    Assert.Equal("DIRECT_FEED", requestMsg.ServiceName());
                    Assert.True("itemA".Equals(requestMsg.Name()) || "itemB".Equals(requestMsg.Name()) || "itemC".Equals(requestMsg.Name()));
                    names.Add(requestMsg.Name());

                    message.MarkForClear();
                }
                Assert.Equal(3, names.Count);

                Assert.Equal(5, consumerClient.QueueSize());

                slNames.Clear();
                names.Clear();
                RefreshMsg refresh;

                for (int i = 0; i < 5; i++)
                {
                    message = consumerClient.PopMessage();
                    Assert.True(message is RefreshMsg);

                    refresh = (RefreshMsg)message;
                    Assert.True(refresh.HasName);

                    if (refresh.DomainType() == (int)DomainType.SYMBOL_LIST)
                    {
                        Assert.True(".AV.N".Equals(refresh.Name()) || ".BV.N".Equals(refresh.Name()));
                        slNames.Add(refresh.Name());
                    }
                    else if (refresh.DomainType() == (int)DomainType.MARKET_PRICE)
                    {
                        Assert.True("itemA".Equals(refresh.Name()) || "itemB".Equals(refresh.Name()) || "itemC".Equals(refresh.Name()));
                        names.Add(refresh.Name());
                    }
                    message.MarkForClear();
                }

                Assert.Equal(3, names.Count);
                Assert.Equal(2, slNames.Count);

                Assert.NotNull(consumer);
            }
            catch (Exception e)
            {
                output.WriteLine(e.Message);
                output.WriteLine(e.StackTrace ?? "Exception: null stack trace");
                Assert.False(true);
            }
            finally
            {
                output.WriteLine("Uninitializing...");

                if (consumer != null) consumer.Uninitialize();
                if (ommProvider != null) ommProvider.Uninitialize();
            }
        }

        [Fact]
        public void TestRequestingTwoSymbolListItems_DifferentServiceNames()
        {
            /*
                Consumer requests two symbol list items from different services DIRECT_FEED and DIRECT_FEED1.
                Consumer application should automatically receive two sets of Market Price items for both Symbol List requests
                even though they have all characteristics (name, Qos) the same apart from the service.
            */
            OmmIProviderConfig providerConfig = new(EmaConfigFileLocation);
            ProviderTestOptions providerTestOptions = new ProviderTestOptions();
            ProviderTestClient providerClient = new ProviderTestClient(output, providerTestOptions);

            OmmProvider ommProvider = new OmmProvider(providerConfig.Port("19001").ProviderName("Provider_8"), providerClient);

            ConsumerTestOptions options = new();
            options.GetChannelInformation = true;
            using ConsumerTestClient consumerClient = new ConsumerTestClient(output, options);

            OmmConsumer? consumer = null;

            try
            {
                ElementList payload = new ElementList();
                ElementEntry entry = new ElementEntry();

                ElementList eePayload = new ElementList();
                ElementEntry eeEntry = new ElementEntry();

                eePayload.AddUInt(":DataStreams", SymbolList.SymbolListDataStreamRequestFlags.SYMBOL_LIST_DATA_STREAMS);
                eePayload.Complete();

                payload.AddElementList(":SymbolListBehaviors", eePayload);
                payload.Complete();

                RequestMsg reqMsg = new RequestMsg();
                reqMsg.DomainType(EmaRdm.MMT_SYMBOL_LIST).ServiceName("DIRECT_FEED").Name(".AV.N").Payload(payload);

                consumer = new OmmConsumer(new OmmConsumerConfig(EmaConfigFileLocation).ConsumerName("Consumer_1"), consumerClient);

                OmmConsumerImpl ommConsumerImpl = consumer.m_OmmConsumerImpl!;

                Thread.Sleep(1500);

                Msg msg = consumerClient.WaitForMessage<Msg>();
                msg.MarkForClear();

                int count = providerClient.QueueSize();
                for (int i = 0; i < count; i++) { msg = providerClient.PopMessage(); msg.MarkForClear(); }

                long itemHandle = consumer.RegisterClient(reqMsg, consumerClient);
                long itemHandle2 = consumer.RegisterClient(reqMsg.Clear().Name(".BV.N").DomainType(EmaRdm.MMT_SYMBOL_LIST).ServiceName("DIRECT_FEED_2").Payload(payload), consumerClient);

                Thread.Sleep(3000);

                reqMsg.MarkForClear();
                payload.MarkForClear();
                eePayload.MarkForClear();

                Assert.Equal(8, providerClient.QueueSize()); // we should receive item requests for 3 MARKET_PRICE items for serviceId 1 and serviceId 2 + 2 symbol list requests

                Msg message;
                RequestMsg requestMsg;

                HashSet<string> slNames = new();
                int[] mpNames = new int[3];
                for (int i = 0; i < 8; i++)
                {
                    message = providerClient.PopMessage();
                    Assert.True(message is RequestMsg);
                    requestMsg = (RequestMsg)message;

                    if (requestMsg.DomainType() == (int)DomainType.SYMBOL_LIST)
                    {
                        Assert.True(".AV.N".Equals(requestMsg.Name()) || ".BV.N".Equals(requestMsg.Name()));
                        slNames.Add(requestMsg.Name());
                    }
                    else if (requestMsg.DomainType() == (int)DomainType.MARKET_PRICE)
                    {
                        Assert.True("itemA".Equals(requestMsg.Name()) || "itemB".Equals(requestMsg.Name()) || "itemC".Equals(requestMsg.Name()));
                        switch (requestMsg.Name())
                        {
                            case "itemA": mpNames[0] += 1; break;
                            case "itemB": mpNames[1] += 1; break;
                            case "itemC": mpNames[2] += 1; break;
                            default: Assert.False(true); break;
                        }
                    }

                    Assert.True("DIRECT_FEED".Equals(requestMsg.ServiceName()) || "DIRECT_FEED_2".Equals(requestMsg.ServiceName()));

                    message.MarkForClear();
                }
                Assert.Equal(2, slNames.Count);

                for (int i = 0; i < 3; i++) Assert.Equal(2, mpNames[i]);
                Assert.Equal(8, consumerClient.QueueSize());

                for (int i = 0; i < 3; i++) mpNames[i] = 0;
                slNames.Clear();

                RefreshMsg refresh;

                for (int i = 0; i < 8; i++)
                {
                    message = consumerClient.PopMessage();
                    Assert.True(message is RefreshMsg);

                    refresh = (RefreshMsg)message;
                    Assert.True(refresh.HasName);

                    if (refresh.DomainType() == (int)DomainType.SYMBOL_LIST)
                    {
                        Assert.True(".AV.N".Equals(refresh.Name()) || ".BV.N".Equals(refresh.Name()));
                        slNames.Add(refresh.Name());
                    }
                    else if (refresh.DomainType() == (int)DomainType.MARKET_PRICE)
                    {
                        Assert.True("itemA".Equals(refresh.Name()) || "itemB".Equals(refresh.Name()) || "itemC".Equals(refresh.Name()));
                        switch (refresh.Name())
                        {
                            case "itemA": mpNames[0] += 1; break;
                            case "itemB": mpNames[1] += 1; break;
                            case "itemC": mpNames[2] += 1; break;
                            default: Assert.False(true); break;
                        }
                    }

                    Assert.True("DIRECT_FEED".Equals(refresh.ServiceName()) || "DIRECT_FEED_2".Equals(refresh.ServiceName()));

                    message.MarkForClear();
                }

                Assert.Equal(2, slNames.Count);
                for (int i = 0; i < 3; i++) Assert.Equal(2, mpNames[i]);

                Assert.NotNull(consumer);
            }
            catch (Exception e)
            {
                output.WriteLine(e.Message);
                output.WriteLine(e.StackTrace ?? "Exception: null stack trace");
                Assert.False(true);
            }
            finally
            {
                output.WriteLine("Uninitializing...");

                if (consumer != null) consumer.Uninitialize();
                if (ommProvider != null) ommProvider.Uninitialize();
            }
        }

        [Fact]
        public void TestRequestingSymbolListItem_SymbolListAndMarketPriceItemsReceived_SendGenericMsgOnMarketPriceStream()
        {
            /*
                When Consumer requests Symbol List item and API automatically opens streams for Market Price items from
                Symbol List Refresh, the Consumer and Provider should be able to successfully send Generic messages on the
                Market Price streams.
             */

            OmmIProviderConfig providerConfig = new(EmaConfigFileLocation);
            ProviderTestOptions providerTestOptions = new ProviderTestOptions();
            ProviderTestClient providerClient = new ProviderTestClient(output, providerTestOptions);

            OmmProvider ommProvider = new OmmProvider(providerConfig.Port("19001"), providerClient);

            ConsumerTestOptions options = new();
            options.GetChannelInformation = true;
            using ConsumerTestClient consumerClient = new ConsumerTestClient(output, options);

            OmmConsumer? consumer = null;

            try
            {
                ElementList payload = new ElementList();
                ElementEntry entry = new ElementEntry();

                ElementList eePayload = new ElementList();
                ElementEntry eeEntry = new ElementEntry();

                eePayload.AddUInt(":DataStreams", SymbolList.SymbolListDataStreamRequestFlags.SYMBOL_LIST_DATA_STREAMS);
                eePayload.Complete();

                payload.AddElementList(":SymbolListBehaviors", eePayload);
                payload.Complete();

                RequestMsg reqMsg = new RequestMsg();
                reqMsg.DomainType(EmaRdm.MMT_SYMBOL_LIST).ServiceName("DIRECT_FEED").Name(".AV.N").Payload(payload);

                consumer = new OmmConsumer(new OmmConsumerConfig(EmaConfigFileLocation).ConsumerName("Consumer_1"), consumerClient);

                OmmConsumerImpl ommConsumerImpl = consumer.m_OmmConsumerImpl!;

                Thread.Sleep(1500);

                Msg msg = consumerClient.WaitForMessage<Msg>();
                msg.MarkForClear();

                int count = providerClient.QueueSize();
                for (int i = 0; i < count; i++) { msg = providerClient.PopMessage(); msg.MarkForClear(); }

                long itemHandle = consumer.RegisterClient(reqMsg, consumerClient);

                reqMsg.MarkForClear();
                payload.MarkForClear();
                eePayload.MarkForClear();

                Thread.Sleep(3000);

                Assert.Equal(4, providerClient.QueueSize());

                Msg message = providerClient.PopMessage();

                Assert.True(message is RequestMsg);
                RequestMsg requestMsg = (RequestMsg)message;

                Assert.Equal("DIRECT_FEED", requestMsg.ServiceName());
                Assert.Equal(".AV.N", requestMsg.Name());

                requestMsg.MarkForClear();

                int itemBStreamId = 0;

                HashSet<string> names = new HashSet<string>();
                for (int i = 0; i < 3; i++)
                {
                    message = providerClient.PopMessage();
                    requestMsg = (RequestMsg)message;
                    Assert.Equal("DIRECT_FEED", requestMsg.ServiceName());
                    Assert.True("itemA".Equals(requestMsg.Name()) || "itemB".Equals(requestMsg.Name()) || "itemC".Equals(requestMsg.Name()));
                    names.Add(requestMsg.Name());

                    if ("itemB".Equals(requestMsg.Name())) itemBStreamId = requestMsg.StreamId();

                    message.MarkForClear();
                }
                Assert.Equal(3, names.Count);
                Assert.True(itemBStreamId != 0);

                Assert.Equal(4, consumerClient.QueueSize());

                message = consumerClient.PopMessage();
                Assert.True(message is RefreshMsg);

                RefreshMsg refresh = (RefreshMsg)message;
                Assert.True(refresh.HasName);
                Assert.Equal(".AV.N", refresh.Name());

                message.MarkForClear();

                names.Clear();
                for (int i = 0; i < 3; i++)
                {
                    message = consumerClient.PopMessage();
                    Assert.True(message is RefreshMsg);

                    refresh = (RefreshMsg)message;
                    Assert.True(refresh.HasName);
                    Assert.True("itemA".Equals(refresh.Name()) || "itemB".Equals(refresh.Name()) || "itemC".Equals(refresh.Name()));
                    names.Add(refresh.Name());

                    message.MarkForClear();
                }
                Assert.Equal(3, names.Count);

                long itemBHandle = consumerClient.NameHandleMap["itemB"];
                consumer.Submit(new GenericMsg().Name("itemB").Complete(true), itemBHandle);

                Thread.Sleep(1000);

                Assert.Equal(1, providerClient.QueueSize());

                message = providerClient.PopMessage();
                Assert.True(message is GenericMsg);
                GenericMsg genericMsg = (GenericMsg)message;
                Assert.Equal("itemB", genericMsg.Name());
                Assert.Equal(itemBStreamId, genericMsg.StreamId());

                message.MarkForClear();

                long provHandleA = providerClient.RetriveItemHandle("itemA");

                if (provHandleA != 0)
                {
                    ommProvider.Submit(new GenericMsg().Name("Prov_itemA").Complete(true), provHandleA);
                }

                Thread.Sleep(1000);

                Assert.Equal(1, consumerClient.QueueSize());
                message = consumerClient.PopMessage();
                Assert.True(message is GenericMsg);

                message.MarkForClear();

                Assert.NotNull(consumer);
            }
            catch (Exception e)
            {
                output.WriteLine(e.Message);
                output.WriteLine(e.StackTrace ?? "Exception: null stack trace");
                Assert.False(true);
            }
            finally
            {
                output.WriteLine("Uninitializing...");

                if (consumer != null) consumer.Uninitialize();
                if (ommProvider != null) ommProvider.Uninitialize();
            }
        }

        [Fact]
        public void TestRequestingSymbolListItem_SymbolListAndMarketPriceItemsRecovery_MPItemRequestTimesOutAndIsRecovered()
        {
            /*
                Consumer requests Symbol List item. API automatically requests the Market Price items from the Refresh message.
                Provider sends Refreshes only for two out of three Market Price items requested. The Consumer receives these Refreshes and
                closes the streams for all currently open items (except for item without Refresh since EMA still doesn't know
                anything about it). After some time the remaining item times out. The API tries to recover it by sending another request,
                Provider sends Refresh, Consumer application receives it.
             */
            OmmIProviderConfig providerConfig = new(EmaConfigFileLocation);
            ProviderTestOptions providerTestOptions = new ProviderTestOptions();

            providerTestOptions.SendItemRefreshMap = new ();
            providerTestOptions.SendItemRefreshMap.Add("itemB", false);

            ProviderTestClient providerClient = new ProviderTestClient(output, providerTestOptions);

            OmmProvider ommProvider = new OmmProvider(providerConfig.Port("19001"), providerClient);

            ConsumerTestOptions options = new();
            options.GetChannelInformation = true;
            using ConsumerTestClient consumerClient = new ConsumerTestClient(output, options);

            OmmConsumer? consumer = null;

            try
            {
                ElementList payload = new ElementList();
                ElementEntry entry = new ElementEntry();

                ElementList eePayload = new ElementList();
                ElementEntry eeEntry = new ElementEntry();

                eePayload.AddUInt(":DataStreams", SymbolList.SymbolListDataStreamRequestFlags.SYMBOL_LIST_DATA_STREAMS);
                eePayload.Complete();

                payload.AddElementList(":SymbolListBehaviors", eePayload);
                payload.Complete();

                RequestMsg reqMsg = new RequestMsg();
                reqMsg.DomainType(EmaRdm.MMT_SYMBOL_LIST).ServiceName("DIRECT_FEED").Name(".AV.N").Payload(payload);

                consumer = new OmmConsumer(new OmmConsumerConfig(EmaConfigFileLocation).ConsumerName("Consumer_1_1"), consumerClient);

                OmmConsumerImpl ommConsumerImpl = consumer.m_OmmConsumerImpl!;

                Thread.Sleep(1500);

                Msg msg = consumerClient.WaitForMessage<Msg>();
                msg.MarkForClear();

                int count = providerClient.QueueSize();
                for (int i = 0; i < count; i++) { msg = providerClient.PopMessage(); msg.MarkForClear(); }

                long itemHandle = consumer.RegisterClient(reqMsg, consumerClient);

                Thread.Sleep(3000);

                reqMsg.MarkForClear();
                payload.MarkForClear();
                eePayload.MarkForClear();

                Assert.Equal(4, providerClient.QueueSize());

                Msg message = providerClient.PopMessage();

                Assert.True(message is RequestMsg);
                RequestMsg requestMsg = (RequestMsg)message;

                message.MarkForClear();

                Assert.Equal("DIRECT_FEED", requestMsg.ServiceName());
                Assert.Equal(".AV.N", requestMsg.Name());

                HashSet<string> names = new();
                for (int i = 0; i < 3; i++)
                {
                    message = providerClient.PopMessage();
                    requestMsg = (RequestMsg)message;
                    Assert.Equal("DIRECT_FEED", requestMsg.ServiceName());
                    Assert.True("itemA".Equals(requestMsg.Name()) || "itemB".Equals(requestMsg.Name()) || "itemC".Equals(requestMsg.Name()));
                    names.Add(requestMsg.Name());

                    message.MarkForClear();
                }
                Assert.Equal(3, names.Count);

                Assert.Equal(3, consumerClient.QueueSize());

                message = consumerClient.PopMessage();
                Assert.True(message is RefreshMsg);

                RefreshMsg refresh = (RefreshMsg)message;
                Assert.True(refresh.HasName);
                Assert.Equal(".AV.N", refresh.Name());

                message.MarkForClear();

                names.Clear();
                for (int i = 0; i < 2; i++)
                {
                    message = consumerClient.PopMessage();
                    Assert.True(message is RefreshMsg);

                    refresh = (RefreshMsg)message;
                    Assert.True(refresh.HasName);
                    Assert.True("itemA".Equals(refresh.Name()) || "itemB".Equals(refresh.Name()) || "itemC".Equals(refresh.Name()));
                    names.Add(refresh.Name());

                    message.MarkForClear();
                }
                Assert.Equal(2, names.Count);

                long itemAHandle = consumerClient.NameHandleMap["itemA"];
                consumer.Unregister(itemAHandle);

                long itemCHandle = consumerClient.NameHandleMap["itemC"];
                consumer.Unregister(itemCHandle);

                consumer.Unregister(itemHandle);

                Thread.Sleep(1000);

                providerTestOptions.SendItemRefreshMap = null;

                Thread.Sleep(9000);

                Assert.Equal(5, providerClient.QueueSize()); // Close msgs for .AV.N, itemA, itemC, Close for itemB since it has expired and ReqMsg for itemB since it was not closed by the application and we want to recover it
                int[] foundMsgs = new int[4];
                for (int i = 0; i < 5; i++)
                {
                    message = providerClient.PopMessage();
                    Assert.True(message is RequestMsg);
                    reqMsg = (RequestMsg)message;
                    if (reqMsg.HasName)
                    {
                        switch (reqMsg.Name())
                        {
                            case ".AV.N": foundMsgs[0] += 1; break;
                            case "itemA": foundMsgs[1] += 1; break;
                            case "itemB": foundMsgs[2] += 1; break;
                            case "itemC": foundMsgs[3] += 1; break;
                            default: break;
                        }
                    }

                    message.MarkForClear();
                }
                Assert.Equal(1, foundMsgs[0]);
                Assert.Equal(1, foundMsgs[1]);
                Assert.Equal(2, foundMsgs[2]);
                Assert.Equal(1, foundMsgs[3]);

                Assert.Equal(2, consumerClient.QueueSize()); // Open/Suspect status for itemB since it timed out, Refresh for itemB.

                bool statusFound = false;
                bool refreshFound = false;

                for (int i = 0; i < 2; i++)
                {
                    message = consumerClient.PopMessage();
                    Assert.True(message is RefreshMsg || message is StatusMsg);
                    if (message is RefreshMsg)
                    {
                        refreshFound = true;
                    }
                    if (message is StatusMsg)
                    {
                        statusFound = true;
                    }

                    message.MarkForClear();
                }

                Assert.True(refreshFound && statusFound);

                Assert.NotNull(consumer);
            }
            catch (Exception e)
            {
                output.WriteLine(e.Message);
                output.WriteLine(e.StackTrace ?? "Exception: null stack trace");
                Assert.False(true);
            }
            finally
            {
                output.WriteLine("Uninitializing...");

                if (consumer != null) consumer.Uninitialize();
                if (ommProvider != null) ommProvider.Uninitialize();
            }
        }

        [Fact]
        public void TestRequestingSymbolListItem_ProviderClosesMarketPriceItem_ConsumerShouldDoFine()
        {
            /*
                Consumer requests Symbol List item. API automatically requests the Market Price items from the Refresh message.
                Provider sends Refreshes for the three Market Price items requested automatically. The Consumer receives these Refreshes.
                The Provider sends Status CLOSED message for one of the Market Price items. Consumer gets Status CLOSED message,
                no error occurs.
             */
            OmmIProviderConfig providerConfig = new(EmaConfigFileLocation);
            ProviderTestOptions providerTestOptions = new ProviderTestOptions();
            ProviderTestClient providerClient = new ProviderTestClient(output, providerTestOptions);

            OmmProvider ommProvider = new OmmProvider(providerConfig.Port("19001"), providerClient);

            ConsumerTestOptions options = new();
            options.GetChannelInformation = true;
            using ConsumerTestClient consumerClient = new ConsumerTestClient(output, options);

            OmmConsumer? consumer = null;

            try
            {
                ElementList payload = new ElementList();
                ElementEntry entry = new ElementEntry();

                ElementList eePayload = new ElementList();
                ElementEntry eeEntry = new ElementEntry();

                eePayload.AddUInt(":DataStreams", SymbolList.SymbolListDataStreamRequestFlags.SYMBOL_LIST_DATA_STREAMS);
                eePayload.Complete();

                payload.AddElementList(":SymbolListBehaviors", eePayload);
                payload.Complete();

                RequestMsg reqMsg = new RequestMsg();
                reqMsg.DomainType(EmaRdm.MMT_SYMBOL_LIST).ServiceName("DIRECT_FEED").Name(".AV.N").Payload(payload);

                consumer = new OmmConsumer(new OmmConsumerConfig(EmaConfigFileLocation).ConsumerName("Consumer_1"), consumerClient);

                OmmConsumerImpl ommConsumerImpl = consumer.m_OmmConsumerImpl!;

                Thread.Sleep(1500);

                Msg msg = consumerClient.WaitForMessage<Msg>();
                msg.MarkForClear();

                int count = providerClient.QueueSize();
                for (int i = 0; i < count; i++) { msg = providerClient.PopMessage(); msg.MarkForClear(); }

                long itemHandle = consumer.RegisterClient(reqMsg, consumerClient);

                Thread.Sleep(3000);

                reqMsg.MarkForClear();
                payload.MarkForClear();
                eePayload.MarkForClear();

                Assert.Equal(4, providerClient.QueueSize());

                Msg message = providerClient.PopMessage();

                Assert.True(message is RequestMsg);
                RequestMsg requestMsg = (RequestMsg)message;

                Assert.Equal("DIRECT_FEED", requestMsg.ServiceName());
                Assert.Equal(".AV.N", requestMsg.Name());

                message.MarkForClear();

                HashSet<string> names = new HashSet<string>();
                for (int i = 0; i < 3; i++)
                {
                    message = providerClient.PopMessage();
                    requestMsg = (RequestMsg)message;
                    Assert.Equal("DIRECT_FEED", requestMsg.ServiceName());
                    Assert.True("itemA".Equals(requestMsg.Name()) || "itemB".Equals(requestMsg.Name()) || "itemC".Equals(requestMsg.Name()));
                    names.Add(requestMsg.Name());

                    message.MarkForClear();
                }
                Assert.Equal(3, names.Count);

                Assert.Equal(4, consumerClient.QueueSize());

                message = consumerClient.PopMessage();
                Assert.True(message is RefreshMsg);

                RefreshMsg refresh = (RefreshMsg)message;
                Assert.True(refresh.HasName);
                Assert.Equal(".AV.N", refresh.Name());

                message.MarkForClear();

                names.Clear();
                int itemAStreamId = 0;
                for (int i = 0; i < 3; i++)
                {
                    message = consumerClient.PopMessage();
                    Assert.True(message is RefreshMsg);

                    refresh = (RefreshMsg)message;
                    Assert.True(refresh.HasName);
                    Assert.True("itemA".Equals(refresh.Name()) || "itemB".Equals(refresh.Name()) || "itemC".Equals(refresh.Name()));
                    names.Add(refresh.Name());
                    if ("itemA".Equals(refresh.Name())) itemAStreamId = refresh.StreamId();

                    message.MarkForClear();
                }
                Assert.Equal(3, names.Count);

                StatusMsg status = new StatusMsg().State(OmmState.StreamStates.CLOSED, OmmState.DataStates.SUSPECT);

                long handle = providerClient.RetriveItemHandle("itemA");
                ommProvider.Submit(status, handle);

                Thread.Sleep(1000);

                status.MarkForClear();

                Assert.Equal(1, consumerClient.QueueSize());

                message = consumerClient.PopMessage();
                Assert.True(message is StatusMsg);
                Assert.Equal(itemAStreamId, message.StreamId());

                message.MarkForClear();

                Assert.NotNull(consumer);
            }
            catch (Exception e)
            {
                output.WriteLine(e.Message);
                output.WriteLine(e.StackTrace ?? "Exception: null stack trace");
                Assert.False(true);
            }
            finally
            {
                output.WriteLine("Uninitializing...");

                if (consumer != null) consumer.Uninitialize();
                if (ommProvider != null) ommProvider.Uninitialize();
            }
        }

        [Fact]
        public void TestSymbolListRequest_UnregisterSymbolListBeforeMarketPriceRefreshesArrive()
        {
            /*
                Consumer requests Symbol List item. API automatically requests the Market Price items from the Refresh message.
                Provider sends Refresh message for Symbol List request, but desn't send Market Price Refreshes at once.
                Before getting Refreshes for Market Price items, Consumer unregisters Symbol List item.
                Then Provider sends Market Price Refreshes. Consumer should receive Refreshes for Market Price items.
            */

            OmmIProviderConfig providerConfig = new(EmaConfigFileLocation);
            ProviderTestOptions providerTestOptions = new ProviderTestOptions();
            ProviderTestClient providerClient = new ProviderTestClient(output, providerTestOptions);

            OmmProvider ommProvider = new OmmProvider(providerConfig.Port("19001"), providerClient);

            ConsumerTestOptions options = new();
            options.GetChannelInformation = true;
            using ConsumerTestClient consumerClient = new ConsumerTestClient(output, options);

            OmmConsumer? consumer = null;

            try
            {
                ElementList payload = new ElementList();
                ElementEntry entry = new ElementEntry();

                ElementList eePayload = new ElementList();
                ElementEntry eeEntry = new ElementEntry();

                eePayload.AddUInt(":DataStreams", SymbolList.SymbolListDataStreamRequestFlags.SYMBOL_LIST_DATA_STREAMS);
                eePayload.Complete();

                payload.AddElementList(":SymbolListBehaviors", eePayload);
                payload.Complete();

                RequestMsg reqMsg = new RequestMsg();
                reqMsg.DomainType(EmaRdm.MMT_SYMBOL_LIST).ServiceName("DIRECT_FEED").Name(".AV.N").Payload(payload);

                consumer = new OmmConsumer(new OmmConsumerConfig(EmaConfigFileLocation).ConsumerName("Consumer_1"), consumerClient);

                OmmConsumerImpl ommConsumerImpl = consumer.m_OmmConsumerImpl!;

                Thread.Sleep(1500);

                Msg msg = consumerClient.WaitForMessage<Msg>();
                msg.MarkForClear();

                int count = providerClient.QueueSize();
                for (int i = 0; i < count; i++) { msg = providerClient.PopMessage(); msg.MarkForClear(); }

                providerTestOptions.WaitBeforeSendingItemRefresh = 5000;
                long itemHandle = consumer.RegisterClient(reqMsg, consumerClient);

                Thread.Sleep(3000);

                reqMsg.MarkForClear();
                payload.MarkForClear();
                eePayload.MarkForClear();

                Msg message;

                Assert.Equal(1, consumerClient.QueueSize());

                message = consumerClient.PopMessage();
                Assert.True(message is RefreshMsg);

                RefreshMsg refresh = (RefreshMsg)message;
                Assert.True(refresh.HasName);
                Assert.Equal(".AV.N", refresh.Name());

                message.MarkForClear();

                consumer.Unregister(itemHandle);

                Thread.Sleep(7000);

                Assert.Equal(3, consumerClient.QueueSize());
                HashSet<string> names = new HashSet<string>();
                for (int i = 0; i < 3; i++)
                {
                    message = consumerClient.PopMessage();
                    Assert.True(message is RefreshMsg);
                    refresh = (RefreshMsg)message;
                    Assert.True(refresh.HasName);
                    Assert.True("itemA".Equals(refresh.Name()) || "itemB".Equals(refresh.Name()) || "itemC".Equals(refresh.Name()));
                    names.Add(refresh.Name());

                    message.MarkForClear();
                }
                Assert.Equal(3, names.Count);

                Assert.NotNull(consumer);
            }
            catch (Exception e)
            {
                output.WriteLine(e.Message);
                output.WriteLine(e.StackTrace ?? "Exception: null stack trace");
                Assert.False(true);
            }
            finally
            {
                output.WriteLine("Uninitializing...");

                if (consumer != null) consumer.Uninitialize();
                if (ommProvider != null) ommProvider.Uninitialize();
            }
        }

        [Fact]
        public void TestSymbolListRequest_ReissueSymbolListAndMarketPriceRequests()
        {
            /*
                Consumer requests Symbol List item. API automatically requests the Market Price items from the Refresh message.
                Provider sends Refreshes for the three Market Price items requested automatically. The Consumer receives these Refreshes.
                The Provider sends Status CLOSED message for one of the Market Price items. Consumer gets Status CLOSED message,
                no error occurs.
             */
            OmmIProviderConfig providerConfig = new(EmaConfigFileLocation);
            ProviderTestOptions providerTestOptions = new ProviderTestOptions();
            ProviderTestClient providerClient = new ProviderTestClient(output, providerTestOptions);

            OmmProvider ommProvider = new OmmProvider(providerConfig.Port("19001"), providerClient);

            ConsumerTestOptions options = new();
            options.GetChannelInformation = true;
            using ConsumerTestClient consumerClient = new ConsumerTestClient(output, options);

            OmmConsumer? consumer = null;

            try
            {
                ElementList payload = new ElementList();
                ElementEntry entry = new ElementEntry();

                ElementList eePayload = new ElementList();
                ElementEntry eeEntry = new ElementEntry();

                eePayload.AddUInt(":DataStreams", SymbolList.SymbolListDataStreamRequestFlags.SYMBOL_LIST_DATA_STREAMS);
                eePayload.Complete();

                payload.AddElementList(":SymbolListBehaviors", eePayload);
                payload.Complete();

                RequestMsg reqMsg = new RequestMsg();
                reqMsg.DomainType(EmaRdm.MMT_SYMBOL_LIST).ServiceName("DIRECT_FEED").Name(".AV.N").Payload(payload);

                consumer = new OmmConsumer(new OmmConsumerConfig(EmaConfigFileLocation).ConsumerName("Consumer_1"), consumerClient);

                OmmConsumerImpl ommConsumerImpl = consumer.m_OmmConsumerImpl!;

                Thread.Sleep(1500);

                Msg msg = consumerClient.WaitForMessage<Msg>();
                msg.MarkForClear();

                int count = providerClient.QueueSize();
                for (int i = 0; i < count; i++) { msg = providerClient.PopMessage(); msg.MarkForClear(); }

                long itemHandle = consumer.RegisterClient(reqMsg, consumerClient);

                Thread.Sleep(3000);

                reqMsg.MarkForClear();
                payload.MarkForClear();
                eePayload.MarkForClear();

                Assert.Equal(4, providerClient.QueueSize());

                Msg message = providerClient.PopMessage();

                Assert.True(message is RequestMsg);
                RequestMsg requestMsg = (RequestMsg)message;

                Assert.Equal("DIRECT_FEED", requestMsg.ServiceName());
                Assert.Equal(".AV.N", requestMsg.Name());

                message.MarkForClear();

                HashSet<string> names = new HashSet<string>();
                for (int i = 0; i < 3; i++)
                {
                    message = providerClient.PopMessage();
                    requestMsg = (RequestMsg)message;
                    Assert.Equal("DIRECT_FEED", requestMsg.ServiceName());
                    Assert.True("itemA".Equals(requestMsg.Name()) || "itemB".Equals(requestMsg.Name()) || "itemC".Equals(requestMsg.Name()));
                    names.Add(requestMsg.Name());

                    message.MarkForClear();
                }
                Assert.Equal(3, names.Count);

                Assert.Equal(4, consumerClient.QueueSize());

                message = consumerClient.PopMessage();
                Assert.True(message is RefreshMsg);

                RefreshMsg refresh = (RefreshMsg)message;
                Assert.True(refresh.HasName);
                Assert.Equal(".AV.N", refresh.Name());

                message.MarkForClear();

                names.Clear();
                int itemAStreamId = 0;

                for (int i = 0; i < 3; i++)
                {
                    message = consumerClient.PopMessage();
                    Assert.True(message is RefreshMsg);

                    refresh = (RefreshMsg)message;
                    Assert.True(refresh.HasName);
                    Assert.True("itemA".Equals(refresh.Name()) || "itemB".Equals(refresh.Name()) || "itemC".Equals(refresh.Name()));
                    names.Add(refresh.Name());
                    if ("itemA".Equals(refresh.Name())) itemAStreamId = refresh.StreamId();

                    message.MarkForClear();
                }
                Assert.Equal(3, names.Count);

                consumer.RegisterClient(reqMsg, consumerClient);

                Thread.Sleep(2000);
                Assert.NotNull(consumer);
            }
            catch (Exception e)
            {
                output.WriteLine(e.Message);
                output.WriteLine(e.StackTrace ?? "Exception: null stack trace");
                Assert.False(true);
            }
            finally
            {
                output.WriteLine("Uninitializing...");

                if (consumer != null) consumer.Uninitialize();
                if (ommProvider != null) ommProvider.Uninitialize();
            }
        }

        private void CheckStandardSymbolListResponse(ConsumerTestClient consumerClient)
        {
            HashSet<string> names = new();
            HashSet<int> streamIds = new();
            bool foundSLRefresh = false;

            for (int i = 0; i < 4; i++)
            {
                RefreshMsg message = consumerClient.WaitForMessage<RefreshMsg>();
                if (message.DomainType() == (int)DomainType.SYMBOL_LIST)
                {
                    foundSLRefresh = true;
                    Assert.True(message.StreamId() > 0);
                    if (message.HasServiceName) Assert.Equal("DIRECT_FEED", message.ServiceName());
                }
                else if (message.DomainType() == (int)DomainType.MARKET_PRICE)
                {
                    names.Add(message.Name());
                    streamIds.Add(message.StreamId());

                    Assert.True("itemA".Equals(message.Name()) || "itemB".Equals(message.Name()) || "itemC".Equals(message.Name()));
                }
                message.MarkForClear();
            }

            Assert.True(names.Contains("itemA") && names.Contains("itemB") && names.Contains("itemC"));
            Assert.True(streamIds.Contains(-1) && streamIds.Contains(-2) && streamIds.Contains(-3));
            Assert.True(foundSLRefresh);
        }


        // More advanced scenarios

        [Fact]
        public void TestReconnectionInChannelSet()
        {
            OmmIProviderConfig providerConfig = new(EmaConfigFileLocation);
            ProviderTestOptions providerTestOptions = new ProviderTestOptions();

            ProviderTestClient providerClient_1 = new ProviderTestClient(output, providerTestOptions);
            OmmProvider ommProvider_1 = new OmmProvider(providerConfig.Port("19001"), providerClient_1);

            ProviderTestClient providerClient_2 = new ProviderTestClient(output, providerTestOptions);
            OmmProvider ommProvider_2 = new OmmProvider(providerConfig.Port("19002"), providerClient_2);

            ProviderTestClient providerClient_3 = new ProviderTestClient(output, providerTestOptions);
            OmmProvider ommProvider_3 = new OmmProvider(providerConfig.Port("19003"), providerClient_3);

            ConsumerTestOptions options = new();
            options.GetChannelInformation = true;
            using ConsumerTestClient consumerClient = new ConsumerTestClient(output, options);

            OmmConsumer? consumer = null;

            try
            {
                ElementList payload = new ElementList();
                ElementEntry entry = new ElementEntry();

                ElementList eePayload = new ElementList();
                ElementEntry eeEntry = new ElementEntry();

                eePayload.AddUInt(":DataStreams", SymbolList.SymbolListDataStreamRequestFlags.SYMBOL_LIST_DATA_STREAMS);
                eePayload.Complete();

                payload.AddElementList(":SymbolListBehaviors", eePayload);
                payload.Complete();

                RequestMsg reqMsg = new RequestMsg();
                reqMsg.DomainType(EmaRdm.MMT_SYMBOL_LIST).ServiceName("DIRECT_FEED").Name(".AV.N").Payload(payload);

                consumer = new OmmConsumer(new OmmConsumerConfig(EmaConfigFileLocation).ConsumerName("Consumer_1_3"), consumerClient);

                OmmConsumerImpl ommConsumerImpl = consumer.m_OmmConsumerImpl!;

                Thread.Sleep(1500);

                Msg msg = consumerClient.WaitForMessage<Msg>();
                msg.MarkForClear();

                int count = providerClient_1.QueueSize();
                for (int i = 0; i < count; i++) { msg = providerClient_1.PopMessage(); msg.MarkForClear(); }

                long itemHandle = consumer.RegisterClient(reqMsg, consumerClient);

                Thread.Sleep(3000);

                reqMsg.MarkForClear();
                payload.MarkForClear();
                eePayload.MarkForClear();

                count = consumerClient.QueueSize();
                for (int i = 0; i < count; i++) { msg = consumerClient.PopMessage(); msg.MarkForClear(); }

                output.WriteLine(">>>> Killing provider 1");
                ommProvider_1.Uninitialize();

                Thread.Sleep(6000);

                HashSet<string> itemNames = new();
                HashSet<int> closedStreamIds = new();

                count = consumerClient.QueueSize();
                for (int i = 0; i < count; i++)
                {
                    Msg message = consumerClient.PopMessage();
                    //ChannelInformation chInfo = consumerClient.PopChannelInfo();

                    if (message is RefreshMsg)
                    {
                        RefreshMsg refresh = (RefreshMsg)message;
                        if (refresh.HasName
                                && (refresh.Name().Equals(".AV.N")
                                || refresh.Name().Equals("itemA")
                                || refresh.Name().Equals("itemB")
                                || refresh.Name().Equals("itemC")))
                        {
                            if (refresh.Name().Equals(".AV.N")) Assert.Equal(5, refresh.StreamId());
                            itemNames.Add(refresh.Name());
                        }
                    }
                    else if (message is StatusMsg)
                    {
                        StatusMsg status = (StatusMsg)message;
                        Assert.True(status.HasState);
                        if (status.StreamId() < 0)
                        {
                            Assert.Contains("Individual item", status.State().StatusText);
                            Assert.Equal(OmmState.StreamStates.CLOSED, status.State().StreamState);
                            closedStreamIds.Add(status.StreamId());
                        }
                    }

                    message.MarkForClear();
                }
                Assert.True(itemNames.Contains("itemA") && itemNames.Contains("itemB") && itemNames.Contains("itemC"));
                Assert.True(closedStreamIds.Contains(-1) && closedStreamIds.Contains(-2) && closedStreamIds.Contains(-3));
            }
            catch (Exception e)
            {
                output.WriteLine(e.Message);
                output.WriteLine(e.StackTrace ?? "Exception: null stack trace");
                Assert.True(false);
            }
            finally
            {
                output.WriteLine("Uninitializing...");
                if (consumer != null) consumer.Uninitialize();

                if (ommProvider_1 != null) ommProvider_1.Uninitialize();
                if (ommProvider_2 != null) ommProvider_2.Uninitialize();
                if (ommProvider_3 != null) ommProvider_3.Uninitialize();
            }
        }

        [Fact]
        public void TestReconnectionInChannelSet_RequestSameItemAsInSymbolList()
        {
            OmmIProviderConfig providerConfig = new(EmaConfigFileLocation);
            ProviderTestOptions providerTestOptions = new ProviderTestOptions();
            providerTestOptions.RespondToReissue = true;

            ProviderTestClient providerClient_1 = new ProviderTestClient(output, providerTestOptions);
            OmmProvider ommProvider_1 = new OmmProvider(providerConfig.Port("19001"), providerClient_1);

            ProviderTestClient providerClient_2 = new ProviderTestClient(output, providerTestOptions);
            OmmProvider ommProvider_2 = new OmmProvider(providerConfig.Port("19002"), providerClient_2);

            ProviderTestClient providerClient_3 = new ProviderTestClient(output, providerTestOptions);
            OmmProvider ommProvider_3 = new OmmProvider(providerConfig.Port("19003"), providerClient_3);

            ConsumerTestOptions options = new();
            options.GetChannelInformation = true;
            using ConsumerTestClient consumerClient = new ConsumerTestClient(output, options);

            OmmConsumer? consumer = null;

            try
            {
                ElementList payload = new ElementList();
                ElementEntry entry = new ElementEntry();

                ElementList eePayload = new ElementList();
                ElementEntry eeEntry = new ElementEntry();

                eePayload.AddUInt(":DataStreams", SymbolList.SymbolListDataStreamRequestFlags.SYMBOL_LIST_DATA_STREAMS);
                eePayload.Complete();

                payload.AddElementList(":SymbolListBehaviors", eePayload);
                payload.Complete();

                consumer = new OmmConsumer(new OmmConsumerConfig(EmaConfigFileLocation).ConsumerName("Consumer_1_3"), consumerClient);

                RequestMsg reqMsg = new RequestMsg();
                reqMsg.DomainType(EmaRdm.MMT_MARKET_PRICE).ServiceName("DIRECT_FEED").Name("itemA");

                OmmConsumerImpl ommConsumerImpl = consumer.m_OmmConsumerImpl!;

                Thread.Sleep(1500);

                Msg msg = consumerClient.WaitForMessage<Msg>();
                msg.MarkForClear();

                int count = providerClient_1.QueueSize();
                for (int i = 0; i < count; i++) { msg = providerClient_1.PopMessage(); msg.MarkForClear(); }

                long itemAHandle = consumer.RegisterClient(reqMsg, consumerClient);

                reqMsg.MarkForClear();

                Thread.Sleep(2000);

                reqMsg = new RequestMsg();
                reqMsg.DomainType(EmaRdm.MMT_SYMBOL_LIST).ServiceName("DIRECT_FEED").Name(".AV.N").Payload(payload);

                long itemHandle = consumer.RegisterClient(reqMsg, consumerClient);

                Thread.Sleep(3000);

                reqMsg.MarkForClear();
                payload.MarkForClear();
                eePayload.MarkForClear();

                count = consumerClient.QueueSize();
                for (int i = 0; i < count; i++) { msg = consumerClient.PopMessage(); msg.MarkForClear(); }

                output.WriteLine(">>>> Killing provider 1");
                ommProvider_1.Uninitialize();

                Thread.Sleep(6000);

                HashSet<string> itemNames = new();
                HashSet<int> closedStreamIds = new();
                int refreshCount = 0;
                int openStatusCount = 0;

                count = consumerClient.QueueSize();
                for (int i = 0; i < count; i++)
                {
                    Msg message = consumerClient.PopMessage();
                    //ChannelInformation chInfo = consumerClient.PopChannelInfo();

                    if (message is RefreshMsg)
                    {
                        RefreshMsg refresh = (RefreshMsg)message;
                        if (refresh.HasName
                                && (refresh.Name().Equals(".AV.N")
                                || refresh.Name().Equals("itemA")
                                || refresh.Name().Equals("itemB")
                                || refresh.Name().Equals("itemC")))
                        {
                            if (refresh.Name().Equals(".AV.N")) Assert.Equal(6, refresh.StreamId());
                            itemNames.Add(refresh.Name());
                            refreshCount++;
                        }
                    }
                    else if (message is StatusMsg)
                    {
                        StatusMsg status = (StatusMsg)message;
                        Assert.True(status.HasState);
                        if (status.DomainType() == EmaRdm.MMT_SYMBOL_LIST)
                        {
                            Assert.Equal(OmmState.StreamStates.OPEN, status.State().StreamState);
                            Assert.Equal(OmmState.DataStates.SUSPECT, status.State().DataState);

                            openStatusCount++;
                        }
                        else if (status.DomainType() == EmaRdm.MMT_MARKET_PRICE)
                        {
                            if (status.StreamId() < 0)
                            {
                                Assert.Contains("Individual item", status.State().StatusText);
                                Assert.Equal(OmmState.StreamStates.CLOSED, status.State().StreamState);
                                closedStreamIds.Add(status.StreamId());
                            }
                            else
                            {
                                Assert.Equal(OmmState.StreamStates.OPEN, status.State().StreamState);
                                Assert.Equal(OmmState.DataStates.SUSPECT, status.State().DataState);

                                Assert.True("AV.N".Equals(status.Name()) || "itemA".Equals(status.Name()));
                                openStatusCount++;
                            }
                        }                        
                    }

                    message.MarkForClear();
                }
                Assert.True(itemNames.Contains("itemA") && itemNames.Contains("itemB") && itemNames.Contains("itemC"));
                Assert.True(closedStreamIds.Contains(-1) && closedStreamIds.Contains(-2) && closedStreamIds.Contains(-3));
                Assert.Equal(5, refreshCount);
                Assert.Equal(2, openStatusCount);
            }
            catch (Exception e)
            {
                output.WriteLine(e.Message);
                output.WriteLine(e.StackTrace ?? "Exception: null stack trace");
                Assert.True(false);
            }
            finally
            {
                output.WriteLine("Uninitializing...");
                if (consumer != null) consumer.Uninitialize();

                if (ommProvider_1 != null) ommProvider_1.Uninitialize();
                if (ommProvider_2 != null) ommProvider_2.Uninitialize();
                if (ommProvider_3 != null) ommProvider_3.Uninitialize();
            }
        }

        [Fact]
        public void TestRequestingSingleSymbolListItemWithRequestedConnectionDownWithDisableSessionEnhancedItemRecovery()
        {
            OmmIProviderConfig providerConfig = new(EmaConfigFileLocation);
            ProviderTestOptions providerTestOptions = new ProviderTestOptions();

            ProviderTestClient providerClient_1 = new ProviderTestClient(output, providerTestOptions);
            OmmProvider ommProvider_1 = new OmmProvider(providerConfig.Port("19001"), providerClient_1);

            ProviderTestClient providerClient_4 = new ProviderTestClient(output, providerTestOptions);
            OmmProvider ommProvider_4 = new OmmProvider(providerConfig.Port("19004"), providerClient_4);


            ConsumerTestOptions options = new();
            options.GetChannelInformation = true;
            using ConsumerTestClient consumerClient = new ConsumerTestClient(output, options);

            OmmConsumer? consumer = null;

            try
            {
                ElementList payload = new ElementList();
                ElementEntry entry = new ElementEntry();

                ElementList eePayload = new ElementList();
                ElementEntry eeEntry = new ElementEntry();

                eePayload.AddUInt(":DataStreams", SymbolList.SymbolListDataStreamRequestFlags.SYMBOL_LIST_DATA_STREAMS);
                eePayload.Complete();

                payload.AddElementList(":SymbolListBehaviors", eePayload);
                payload.Complete();

                RequestMsg reqMsg = new RequestMsg();
                reqMsg.DomainType(EmaRdm.MMT_SYMBOL_LIST).ServiceName("DIRECT_FEED").Name(".AV.N").Payload(payload);

                consumer = new OmmConsumer(new OmmConsumerConfig(EmaConfigFileLocation).ConsumerName("Consumer_9"), consumerClient);

                OmmConsumerImpl ommConsumerImpl = consumer.m_OmmConsumerImpl!;

                Thread.Sleep(1500);

                Msg msg = consumerClient.WaitForMessage<Msg>();
                msg.MarkForClear();

                int count = consumerClient.QueueSize();
                for (int i = 0; i < count; i++) { msg = consumerClient.PopMessage(); msg.MarkForClear(); }

                count = providerClient_1.QueueSize();
                for (int i = 0; i < count; i++) { msg = providerClient_1.PopMessage(); msg.MarkForClear(); }

                long itemHandle = consumer.RegisterClient(reqMsg, consumerClient);

                Thread.Sleep(3000);

                reqMsg.MarkForClear();
                payload.MarkForClear();
                eePayload.MarkForClear();

                /* Checks provider that receives the item request. */
                Msg message;
                RequestMsg requestMsg;

                Assert.Equal(4, providerClient_1.QueueSize());
                for (int i = 0; i < 4; i++)
                {
                    message = providerClient_1.PopMessage();
                    Assert.True(message is RequestMsg);

                    message.MarkForClear();
                }

                Assert.Equal(4, consumerClient.QueueSize());
                for (int i = 0; i < 4; i++)
                {
                    message = consumerClient.PopMessage();
                    Assert.True(message is RefreshMsg);

                    message.MarkForClear();
                }

                Thread.Sleep(1000);

                output.WriteLine(">>>> Provider 1 goes down <<<<");
                /* Force channel down on the first provider */
                ommProvider_1.Uninitialize();

                Thread.Sleep(15000);

                HashSet<int> statusStreamIds = new();
                HashSet<string> refreshNames = new();
                StatusMsg status;
                RefreshMsg refresh;

                Assert.Equal(11, consumerClient.QueueSize());

                for (int i = 0; i < 11; i++)
                {
                    message = consumerClient.PopMessage();
                    if (message is StatusMsg)
                    {
                        status = (StatusMsg)message;
                        if (status.DomainType() == (int)DomainType.MARKET_PRICE)
                        {
                            statusStreamIds.Add(status.StreamId());
                            Assert.True(status.State().StreamState == OmmState.StreamStates.CLOSED);
                            Assert.True(status.State().DataState == OmmState.DataStates.SUSPECT);
                        }
                        else if (status.DomainType() == (int)DomainType.SYMBOL_LIST)
                        {
                            statusStreamIds.Add(status.StreamId());
                            Assert.True(status.State().StreamState == OmmState.StreamStates.OPEN);
                            Assert.True(status.State().DataState == OmmState.DataStates.SUSPECT);
                        }
                    }
                    else if (message is RefreshMsg)
                    {
                        refresh = (RefreshMsg)message;
                        if (refresh.DomainType() == (int)DomainType.MARKET_PRICE || refresh.DomainType() == (int)DomainType.SYMBOL_LIST)
                        {
                            Assert.True(refresh.HasName);
                            refreshNames.Add(refresh.Name());
                            Assert.True(".AV.N".Equals(refresh.Name())
                                    || "itemA".Equals(refresh.Name()) || "itemB".Equals(refresh.Name()) || "itemC".Equals(refresh.Name()));
                        }

                    }

                    message.MarkForClear();
                }

                Assert.Equal(4, statusStreamIds.Count);
                Assert.Equal(4, refreshNames.Count);

                Assert.Equal(4, providerClient_4.QueueSize());
                for (int i = 0; i < 4; i++)
                {
                    message = providerClient_4.PopMessage();
                    Assert.True(message is RequestMsg);
                    requestMsg = (RequestMsg)message;
                    Assert.True(".AV.N".Equals(requestMsg.Name())
                            || "itemA".Equals(requestMsg.Name()) || "itemB".Equals(requestMsg.Name()) || "itemC".Equals(requestMsg.Name()));

                    message.MarkForClear();
                }

                output.WriteLine(">>>> Provider 1 goes up again <<<<");
                providerClient_1 = new ProviderTestClient(output, providerTestOptions);
                ommProvider_1 = new OmmProvider(providerConfig.Port("19001"), providerClient_1);

                Thread.Sleep(3000);

                Assert.Equal(0, consumerClient.QueueSize());
                Assert.Equal(0, providerClient_1.QueueSize());
            }
            catch (Exception e)
            {
                output.WriteLine(e.StackTrace ?? "Exception: null stack trace");
                Assert.False(true);
            }
            finally
            {
                output.WriteLine("Uninitializing...");

                if (consumer != null) consumer.Uninitialize();
                ommProvider_1.Uninitialize();
                ommProvider_4.Uninitialize();
            }
        }

        [Fact]
        public void TestRequestingSingleSymbolListItemWithRequestedConnectionDownWithEnableSessionEnhancedItemRecovery()
        {
            OmmIProviderConfig providerConfig = new(EmaConfigFileLocation);
            ProviderTestOptions providerTestOptions = new ProviderTestOptions();

            ProviderTestClient providerClient_1 = new ProviderTestClient(output, providerTestOptions);
            OmmProvider ommProvider_1 = new OmmProvider(providerConfig.Port("19001"), providerClient_1);

            ProviderTestClient providerClient_4 = new ProviderTestClient(output, providerTestOptions);
            OmmProvider ommProvider_4 = new OmmProvider(providerConfig.Port("19004"), providerClient_4);


            ConsumerTestOptions options = new();
            options.GetChannelInformation = true;
            using ConsumerTestClient consumerClient = new ConsumerTestClient(output, options);

            OmmConsumer? consumer = null;

            try
            {
                ElementList payload = new ElementList();
                ElementEntry entry = new ElementEntry();

                ElementList eePayload = new ElementList();
                ElementEntry eeEntry = new ElementEntry();

                eePayload.AddUInt(":DataStreams", SymbolList.SymbolListDataStreamRequestFlags.SYMBOL_LIST_DATA_STREAMS);
                eePayload.Complete();

                payload.AddElementList(":SymbolListBehaviors", eePayload);
                payload.Complete();

                RequestMsg reqMsg = new RequestMsg();
                reqMsg.DomainType(EmaRdm.MMT_SYMBOL_LIST).ServiceName("DIRECT_FEED").Name(".AV.N").Payload(payload);

                consumer = new OmmConsumer(new OmmConsumerConfig(EmaConfigFileLocation).ConsumerName("Consumer_12"), consumerClient);

                OmmConsumerImpl ommConsumerImpl = consumer.m_OmmConsumerImpl!;

                Thread.Sleep(1500);

                Msg msg = consumerClient.WaitForMessage<Msg>();
                msg.MarkForClear();

                int count = consumerClient.QueueSize();
                for (int i = 0; i < count; i++) { msg = consumerClient.PopMessage(); msg.MarkForClear(); }

                count = providerClient_1.QueueSize();
                for (int i = 0; i < count; i++) { msg = providerClient_1.PopMessage(); msg.MarkForClear(); }

                long itemHandle = consumer.RegisterClient(reqMsg, consumerClient);

                Thread.Sleep(3000);

                reqMsg.MarkForClear();
                payload.MarkForClear();
                eePayload.MarkForClear();

                /* Checks provider that receives the item request. */
                Msg message;
                RequestMsg requestMsg;

                Assert.Equal(4, providerClient_1.QueueSize());
                for (int i = 0; i < 4; i++)
                {
                    message = providerClient_1.PopMessage();
                    Assert.True(message is RequestMsg);

                    message.MarkForClear();
                }

                Assert.Equal(4, consumerClient.QueueSize());
                for (int i = 0; i < 4; i++)
                {
                    message = consumerClient.PopMessage();
                    Assert.True(message is RefreshMsg);

                    message.MarkForClear();
                }

                Thread.Sleep(2000);

                output.WriteLine(">>>>>> Provider 1 goes down\n");
                ommProvider_1.Uninitialize();

                Thread.Sleep(15000);

                HashSet<int> statusStreamIds = new();
                HashSet<string> refreshNames = new();
                StatusMsg status;
                RefreshMsg refresh;

                count = consumerClient.QueueSize();

                for (int i = 0; i < count; i++)
                {
                    message = consumerClient.PopMessage();
                    if (message is StatusMsg)
                    {
                        status = (StatusMsg)message;
                        if (status.DomainType() == (int)DomainType.SYMBOL_LIST)
                        {
                            statusStreamIds.Add(status.StreamId());
                            Assert.True(status.State().StreamState == OmmState.StreamStates.OPEN);
                            Assert.True(status.State().DataState == OmmState.DataStates.SUSPECT);
                        }
                        else if (status.DomainType() == (int)DomainType.MARKET_PRICE)
                        {
                            statusStreamIds.Add(status.StreamId());
                            Assert.True(status.State().StreamState == OmmState.StreamStates.CLOSED);
                            Assert.True(status.State().DataState == OmmState.DataStates.SUSPECT);
                        }
                    }
                    else if (message is RefreshMsg)
                    {
                        refresh = (RefreshMsg)message;
                        if (refresh.DomainType() == (int)DomainType.MARKET_PRICE || refresh.DomainType() == (int)DomainType.SYMBOL_LIST)
                        {
                            Assert.True(refresh.HasName);
                            refreshNames.Add(refresh.Name());
                            Assert.True(".AV.N".Equals(refresh.Name())
                                    || "itemA".Equals(refresh.Name()) || "itemB".Equals(refresh.Name()) || "itemC".Equals(refresh.Name()));
                        }
                    }

                    message.MarkForClear();
                }

                Assert.Equal(4, statusStreamIds.Count);
                Assert.Equal(4, refreshNames.Count);

                Assert.Equal(4, providerClient_4.QueueSize());
                for (int i = 0; i < 4; i++)
                {
                    message = providerClient_4.PopMessage();
                    Assert.True(message is RequestMsg);
                    requestMsg = (RequestMsg)message;
                    Assert.True(".AV.N".Equals(requestMsg.Name())
                            || "itemA".Equals(requestMsg.Name()) || "itemB".Equals(requestMsg.Name()) || "itemC".Equals(requestMsg.Name()));

                    message.MarkForClear();
                }

                providerClient_1 = new ProviderTestClient(output, providerTestOptions);
                ommProvider_1 = new OmmProvider(providerConfig.Port("19001"), providerClient_1);

                Thread.Sleep(3000);

                Assert.Equal(0, consumerClient.QueueSize());
                Assert.Equal(0, providerClient_1.QueueSize());
            }
            catch (Exception e)
            {
                output.WriteLine(e.StackTrace ?? "Exception: null stack trace");
                Assert.False(true);

            }
            finally
            {
                output.WriteLine("Uninitializing...");

                if (consumer != null) consumer.Uninitialize();
                ommProvider_1.Uninitialize();
                ommProvider_4.Uninitialize();
            }
        }

        [Fact]
        public void TestRequestingSingleSLItemWithRequestedConnectionDownWithDisableSessionEnhancedItemRecoveryAndChannelIsClosed()
        {
            OmmIProviderConfig providerConfig = new(EmaConfigFileLocation);
            ProviderTestOptions providerTestOptions = new ProviderTestOptions();

            ProviderTestClient providerClient_1 = new ProviderTestClient(output, providerTestOptions);
            OmmProvider ommProvider_1 = new OmmProvider(providerConfig.Port("19001"), providerClient_1);

            ProviderTestClient providerClient_4 = new ProviderTestClient(output, providerTestOptions);
            OmmProvider ommProvider_4 = new OmmProvider(providerConfig.Port("19004"), providerClient_4);


            ConsumerTestOptions options = new();
            options.GetChannelInformation = true;
            using ConsumerTestClient consumerClient = new ConsumerTestClient(output, options);

            OmmConsumer? consumer = null;

            try
            {
                ElementList payload = new ElementList();
                ElementEntry entry = new ElementEntry();

                ElementList eePayload = new ElementList();
                ElementEntry eeEntry = new ElementEntry();

                eePayload.AddUInt(":DataStreams", SymbolList.SymbolListDataStreamRequestFlags.SYMBOL_LIST_DATA_STREAMS);
                eePayload.Complete();

                payload.AddElementList(":SymbolListBehaviors", eePayload);
                payload.Complete();

                RequestMsg reqMsg = new RequestMsg();
                reqMsg.DomainType(EmaRdm.MMT_SYMBOL_LIST).ServiceName("DIRECT_FEED").Name(".AV.N").Payload(payload);

                consumer = new OmmConsumer(new OmmConsumerConfig(EmaConfigFileLocation).ConsumerName("Consumer_9"), consumerClient);

                OmmConsumerImpl ommConsumerImpl = consumer.m_OmmConsumerImpl!;

                Thread.Sleep(1500);

                Msg msg = consumerClient.WaitForMessage<Msg>();
                msg.MarkForClear();

                int count = consumerClient.QueueSize();
                for (int i = 0; i < count; i++) { msg = consumerClient.PopMessage(); msg.MarkForClear(); }

                count = providerClient_1.QueueSize();
                for (int i = 0; i < count; i++) { msg = providerClient_1.PopMessage(); msg.MarkForClear(); }

                long itemHandle = consumer.RegisterClient(reqMsg, consumerClient);

                Thread.Sleep(3000);

                reqMsg.MarkForClear();
                payload.MarkForClear();
                eePayload.MarkForClear();

                HashSet<string> itemNames = new();
                HashSet<int> itemStreamIds = new();
                count = consumerClient.QueueSize();
                for (int i = 0; i < count; i++)
                {
                    Msg message = consumerClient.PopMessage();
                    ChannelInformation chInfo = consumerClient.PopChannelInfo();
                    if (message is RefreshMsg)
                    {
                        RefreshMsg refresh = (RefreshMsg)message;
                        if (refresh.HasName
                                && (refresh.Name().Equals(".AV.N")
                                || refresh.Name().Equals("itemA")
                                || refresh.Name().Equals("itemB")
                                || refresh.Name().Equals("itemC")))
                        {
                            if (refresh.Name().Equals(".AV.N")) Assert.Equal(5, refresh.StreamId());
                            itemNames.Add(refresh.Name());
                            itemStreamIds.Add(refresh.StreamId());
                        }
                    }

                    message.MarkForClear();
                }
                Assert.Equal(4, itemNames.Count);


                Thread.Sleep(1000);

                output.WriteLine(" >>>> Killing provider_1");
                /* Force channel down on the first provider of Connection_1*/
                ommProvider_1.Uninitialize();

                /* Wait until the subscribed channel is closed */
                Thread.Sleep(15000);

                int msgCount = consumerClient.QueueSize();
                HashSet<string> names = new();
                StatusMsg status;
                int closedItemsCount = 0;

                for (int i = 0; i < msgCount; i++)
                {
                    Msg message = consumerClient.PopMessage();
                    ChannelInformation chInfo = consumerClient.PopChannelInfo();
                    if (message.DomainType() == (int)DomainType.MARKET_PRICE || message.DomainType() == (int)DomainType.SYMBOL_LIST)
                    {
                        if (message is RefreshMsg)
                        {
                            Assert.True(message.HasName);
                            RefreshMsg refresh = (RefreshMsg)message;
                            if (".AV.N".Equals(refresh.Name()) || "itemA".Equals(refresh.Name()) || "itemB".Equals(refresh.Name()) || "itemC".Equals(refresh.Name()))
                                names.Add(refresh.Name());
                        }
                        else if (message is StatusMsg)
                        {
                            status = (StatusMsg)message;
                            if (status.State().StatusText.Contains("Individual item from Symbol List closed due to server change."))
                                closedItemsCount++;
                        }
                    }

                    message.MarkForClear();
                }
                Assert.Equal(4, names.Count);
                Assert.Equal(3, closedItemsCount);

                count = providerClient_4.QueueSize();
                for (int i = 0; i < count; i++) { msg = providerClient_4.PopMessage(); msg.MarkForClear(); }

                consumer.Unregister(itemHandle);

                Thread.Sleep(3000);

                Assert.True(providerClient_4.QueueSize() > 0); // provider_4 received close message
            }
            catch (Exception e)
            {
                output.WriteLine(e.StackTrace ?? "Exception: null stack trace");
                Assert.False(true);
            }
            finally
            {
                output.WriteLine("Uninitializing...");

                if (consumer != null) consumer.Uninitialize();
                ommProvider_1.Uninitialize();
                ommProvider_4.Uninitialize();
            }
        }

        [Fact]
        public void TestMultiConnectionSymbolListRequestsToDifferentServers()
        {
            OmmIProviderConfig providerConfig = new(EmaConfigFileLocation);
            ProviderTestOptions providerTestOptions = new ProviderTestOptions();

            ProviderTestClient providerClient_1 = new ProviderTestClient(output, providerTestOptions);
            OmmProvider ommProvider_1 = new OmmProvider(providerConfig.Port("19001").ProviderName("Provider_1"), providerClient_1);

            ProviderTestClient providerClient_4 = new ProviderTestClient(output, providerTestOptions);
            OmmProvider ommProvider_4 = new OmmProvider(providerConfig.Port("19004").ProviderName("Provider_3"), providerClient_4);

            ProviderTestClient providerClient_5 = new ProviderTestClient(output, providerTestOptions);
            OmmProvider ommProvider_5 = new OmmProvider(providerConfig.Port("19005").ProviderName("Provider_3"), providerClient_5);


            ConsumerTestOptions options = new();
            options.GetChannelInformation = true;
            using ConsumerTestClient consumerClient = new ConsumerTestClient(output, options);

            OmmConsumer? consumer = null;

            try
            {
                string serviceName = "DIRECT_FEED";
                string serviceName2 = "DIRECT_FEED_2";
                string itemName = ".AV.N";

                ElementList payload = new ElementList();
                ElementEntry entry = new ElementEntry();

                ElementList eePayload = new ElementList();
                ElementEntry eeEntry = new ElementEntry();

                eePayload.AddUInt(":DataStreams", SymbolList.SymbolListDataStreamRequestFlags.SYMBOL_LIST_DATA_STREAMS);
                eePayload.Complete();

                payload.AddElementList(":SymbolListBehaviors", eePayload);
                payload.Complete();

                RequestMsg reqMsg = new RequestMsg().DomainType(EmaRdm.MMT_SYMBOL_LIST).ServiceName(serviceName).Name(".AV.N").Payload(payload);
                RequestMsg reqMsg2 = new RequestMsg().DomainType(EmaRdm.MMT_SYMBOL_LIST).ServiceName(serviceName2).Name(itemName).Payload(payload);

                consumer = new OmmConsumer(new OmmConsumerConfig(EmaConfigFileLocation).ConsumerName("Consumer_55"), consumerClient);

                OmmConsumerImpl ommConsumerImpl = consumer.m_OmmConsumerImpl!;

                Thread.Sleep(1500);

                Msg msg;
                ChannelInformation chInfo;

                int count = consumerClient.QueueSize();
                for (int i = 0; i < count; i++) 
                { 
                    msg = consumerClient.PopMessage(); msg.MarkForClear();
                    chInfo = consumerClient.PopChannelInfo();
                }

                count = providerClient_1.QueueSize();
                for (int i = 0; i < count; i++) { msg = providerClient_1.PopMessage(); msg.MarkForClear(); }

                count = providerClient_4.QueueSize();
                for (int i = 0; i < count; i++) { msg = providerClient_4.PopMessage(); msg.MarkForClear(); }

                output.WriteLine(">>>>>>> Requesting items...\n");
                long itemHandle = consumer.RegisterClient(reqMsg, consumerClient);

                Thread.Sleep(2500);

                Msg message;
                
                HashSet<string> itemNames = new();
                HashSet<int> itemStreamIds = new();
                count = consumerClient.QueueSize();
                for (int i = 0; i < count; i++)
                {
                    message = consumerClient.PopMessage();
                    chInfo = consumerClient.PopChannelInfo();
                    if (chInfo.ChannelName.Equals("Channel_1"))
                    {
                        if (message is RefreshMsg)
                        {
                            RefreshMsg refresh = (RefreshMsg)message;
                            if (refresh.HasName
                                    && (refresh.Name().Equals(".AV.N")
                                    || refresh.Name().Equals("itemA")
                                    || refresh.Name().Equals("itemB")
                                    || refresh.Name().Equals("itemC")))
                            {
                                if (refresh.Name().Equals(".AV.N")) Assert.Equal(5, refresh.StreamId());
                                itemNames.Add(refresh.Name());
                                itemStreamIds.Add(refresh.StreamId());
                            }
                        }
                    }

                    message.MarkForClear();
                }
                Assert.Equal(4, itemNames.Count);
                Assert.True(itemStreamIds.Contains(-1) && itemStreamIds.Contains(-2) && itemStreamIds.Contains(-3));

                itemNames.Clear();
                itemStreamIds.Clear();

                long itemHandle2 = consumer.RegisterClient(reqMsg2, consumerClient);

                Thread.Sleep(5500);

                count = consumerClient.QueueSize();
                for (int i = 0; i < count; i++)
                {
                    message = consumerClient.PopMessage();
                    chInfo = consumerClient.PopChannelInfo();
                    if (chInfo.ChannelName.Equals("Channel_4"))
                    {
                        if (message is RefreshMsg)
                        {
                            RefreshMsg refresh = (RefreshMsg)message;
                            if (refresh.HasName
                                    && (refresh.Name().Equals(".AV.N")
                                    || refresh.Name().Equals("itemA")
                                    || refresh.Name().Equals("itemB")
                                    || refresh.Name().Equals("itemC")))
                            {
                                if (refresh.Name().Equals(".AV.N")) Assert.Equal(6, refresh.StreamId());
                                itemNames.Add(refresh.Name());
                                itemStreamIds.Add(refresh.StreamId());
                            }
                        }
                    }

                    message.MarkForClear();
                }
                Assert.Equal(4, itemNames.Count);
                Assert.True(itemStreamIds.Contains(-4) && itemStreamIds.Contains(-5) && itemStreamIds.Contains(-6));

                reqMsg.MarkForClear();
                reqMsg2.MarkForClear();
                payload.MarkForClear();
                eePayload.MarkForClear();

                count = providerClient_1.QueueSize();
                for (int i = 0; i < count; i++) { msg = providerClient_1.PopMessage(); msg.MarkForClear(); }

                count = providerClient_4.QueueSize();
                for (int i = 0; i < count; i++) { msg = providerClient_4.PopMessage(); msg.MarkForClear(); }

                consumer.Unregister(itemHandle);

                Thread.Sleep(1500);

                Assert.True(providerClient_1.QueueSize() > 0);
                Assert.True(providerClient_4.QueueSize() == 0);

                count = providerClient_1.QueueSize();
                for (int i = 0; i < count; i++) { msg = providerClient_1.PopMessage(); msg.MarkForClear(); }

                consumer.Unregister(itemHandle2);

                Thread.Sleep(1500);

                Assert.True(providerClient_4.QueueSize() > 0);
                Assert.True(providerClient_1.QueueSize() == 0);

                count = providerClient_4.QueueSize();
                for (int i = 0; i < count; i++) { msg = providerClient_4.PopMessage(); msg.MarkForClear(); }
            }
            catch (Exception ex)
            {
                output.WriteLine(ex.StackTrace ?? "Exception: null stack trace");
                Assert.False(true);
            }
            finally
            {
                output.WriteLine(">>>>> Uninitializing...");
                Assert.NotNull(consumer);

                if (consumer != null) consumer.Uninitialize();
                if (ommProvider_1 != null) ommProvider_1.Uninitialize();
                if (ommProvider_4 != null) ommProvider_4.Uninitialize();
                if (ommProvider_5 != null) ommProvider_5.Uninitialize();
            }
        }

        [Fact]
        public void TestMultiConnectionsItemRequestsWithServiceListNameButConcreteServicesAreNotAvaliableThenConcreteServiceIsAdded()
        {
            OmmIProviderConfig providerConfig = new(EmaConfigFileLocation);
            ProviderTestOptions providerTestOptions = new ProviderTestOptions();

            ProviderTestClient providerClient_1 = new ProviderTestClient(output, providerTestOptions);
            OmmProvider ommProvider_1 = new OmmProvider(providerConfig.Port("19001"), providerClient_1);

            ProviderTestClient providerClient_4 = new ProviderTestClient(output, providerTestOptions);
            OmmProvider ommProvider_4 = new OmmProvider(providerConfig.Port("19004"), providerClient_4);


            ConsumerTestOptions options = new();
            options.GetChannelInformation = true;
            using ConsumerTestClient consumerClient = new ConsumerTestClient(output, options);

            OmmConsumer? consumer = null;

            try
            {
                ElementList payload = new ElementList();
                ElementEntry entry = new ElementEntry();

                ElementList eePayload = new ElementList();
                ElementEntry eeEntry = new ElementEntry();

                eePayload.AddUInt(":DataStreams", SymbolList.SymbolListDataStreamRequestFlags.SYMBOL_LIST_DATA_STREAMS);
                eePayload.Complete();

                payload.AddElementList(":SymbolListBehaviors", eePayload);
                payload.Complete();

                ServiceList serviceList = new ServiceList("SVG1");

                serviceList.ConcreteServiceList.Add("UNKNOWN_SERVICE");
                serviceList.ConcreteServiceList.Add("DIRECT_FEED2");

                consumer = new OmmConsumer(new OmmConsumerConfig(EmaConfigFileLocation).ConsumerName("Consumer_9").AddServiceList(serviceList), consumerClient);

                OmmConsumerImpl ommConsumerImpl = consumer.m_OmmConsumerImpl!;  

                Thread.Sleep(1500);

                Msg msg = consumerClient.WaitForMessage<Msg>();
                msg.MarkForClear();

                int count = consumerClient.QueueSize();
                for (int i = 0; i < count; i++) { msg = consumerClient.PopMessage(); msg.MarkForClear(); }

                count = providerClient_1.QueueSize();
                for (int i = 0; i < count; i++) { msg = providerClient_1.PopMessage(); msg.MarkForClear(); }

                RequestMsg reqMsg = new RequestMsg();
                reqMsg.DomainType(EmaRdm.MMT_SYMBOL_LIST).ServiceListName("SVG1").Name(".AV.N").Payload(payload);

                long itemHandle = consumer.RegisterClient(reqMsg, consumerClient);

                Thread.Sleep(3000);

                reqMsg.MarkForClear();
                payload.MarkForClear();
                eePayload.MarkForClear();

                Assert.Equal(1, consumerClient.QueueSize());

                Msg message = consumerClient.PopMessage();

                StatusMsg statusMsg = (StatusMsg)message;

                Assert.Equal("SVG1", statusMsg.ServiceName());
                Assert.Equal(".AV.N", statusMsg.Name());
                Assert.Equal(OmmState.StreamStates.OPEN, statusMsg.State().StreamState);
                Assert.Equal(OmmState.DataStates.SUSPECT, statusMsg.State().DataState);
                Assert.Equal(OmmState.StatusCodes.NONE, statusMsg.State().StatusCode);
                Assert.Equal("No matching service present.", statusMsg.State().StatusText);

                message.MarkForClear();

                // Provider send source directory update message to add the DIRECT_FEED2 service
                OmmArray capablities = new OmmArray();
                capablities.AddUInt(EmaRdm.MMT_MARKET_PRICE);
                capablities.AddUInt(EmaRdm.MMT_MARKET_BY_PRICE);
                capablities.AddUInt(EmaRdm.MMT_SYMBOL_LIST);
                capablities.Complete();
                OmmArray dictionaryUsed = new OmmArray();
                dictionaryUsed.AddAscii("RWFFld");
                dictionaryUsed.AddAscii("RWFEnum");
                dictionaryUsed.Complete();

                ElementList serviceInfoId = new ElementList();

                serviceInfoId.AddAscii(EmaRdm.ENAME_NAME, "DIRECT_FEED2");
                serviceInfoId.AddArray(EmaRdm.ENAME_CAPABILITIES, capablities);
                serviceInfoId.AddArray(EmaRdm.ENAME_DICTIONARYS_USED, dictionaryUsed);

                ElementList serviceStateId = new ElementList();
                serviceStateId.AddUInt(EmaRdm.ENAME_SVC_STATE, EmaRdm.SERVICE_UP);

                FilterList filterList = new FilterList();
                serviceInfoId.Complete();
                filterList.AddEntry(EmaRdm.SERVICE_INFO_ID, FilterAction.SET, serviceInfoId);
                serviceStateId.Complete();
                filterList.AddEntry(EmaRdm.SERVICE_STATE_ID, FilterAction.SET, serviceStateId);
                filterList.Complete();

                Map map = new Map();
                map.AddKeyUInt(2, MapAction.ADD, filterList);
                map.Complete();

                UpdateMsg updateMsg = new UpdateMsg();
                ommProvider_1.Submit(updateMsg.DomainType(EmaRdm.MMT_DIRECTORY).Filter(EmaRdm.SERVICE_INFO_FILTER | EmaRdm.SERVICE_STATE_FILTER).Payload(map), 0);

                Thread.Sleep(5000);

                updateMsg.MarkForClear();
                map.MarkForClear();
                capablities.MarkForClear();
                dictionaryUsed.MarkForClear();
                serviceInfoId.MarkForClear(); 
                serviceStateId.MarkForClear();
                filterList.MarkForClear();

                Assert.Equal(4, providerClient_1.QueueSize());

                message = providerClient_1.PopMessage();

                Assert.True(message is RequestMsg);

                RequestMsg requestMsg = (RequestMsg)message;

                Assert.Equal("DIRECT_FEED2", requestMsg.ServiceName());
                Assert.Equal(".AV.N", requestMsg.Name());
                Assert.Equal(2, requestMsg.ServiceId());

                message.MarkForClear();

                for (int i = 0; i < 3; i++)
                {
                    message = providerClient_1.PopMessage();
                    Assert.True(message is RequestMsg);
                    requestMsg = (RequestMsg)message;
                    Assert.Equal("DIRECT_FEED2", requestMsg.ServiceName());
                    Assert.True("itemA".Equals(requestMsg.Name()) || "itemB".Equals(requestMsg.Name()) || "itemC".Equals(requestMsg.Name()));
                    Assert.Equal(2, requestMsg.ServiceId());

                    message.MarkForClear();
                }

                Assert.Equal(4, consumerClient.QueueSize());
                message = consumerClient.PopMessage();

                RefreshMsg refreshMsg = (RefreshMsg)message;
                Assert.Equal("SVG1", refreshMsg.ServiceName());
                Assert.Equal(".AV.N", refreshMsg.Name());
                Assert.Equal(32767, refreshMsg.ServiceId());
                Assert.Equal(OmmState.StreamStates.OPEN, refreshMsg.State().StreamState);
                Assert.Equal(OmmState.DataStates.OK, refreshMsg.State().DataState);
                Assert.Equal(OmmState.StatusCodes.NONE, (int)refreshMsg.State().Code);
                Assert.True(refreshMsg.Solicited());

                message.MarkForClear();

                for (int i = 0; i < 3; i++)
                {
                    message = consumerClient.PopMessage();
                    Assert.True(message is RefreshMsg);
                    refreshMsg = (RefreshMsg)message;
                    Assert.Equal("SVG1", refreshMsg.ServiceName());
                    Assert.True("itemA".Equals(refreshMsg.Name()) || "itemB".Equals(refreshMsg.Name()) || "itemC".Equals(refreshMsg.Name()));
                    Assert.Equal(32767, refreshMsg.ServiceId());

                    message.MarkForClear();
                }

                consumer.Unregister(itemHandle);
            }
            catch (Exception e)
            {
                output.WriteLine(e.StackTrace ?? "Exception: null stack trace");
                Assert.False(true);
            }
            finally
            {
                output.WriteLine("Uninitializing...");

                if (consumer != null) consumer.Uninitialize();
                if (ommProvider_1 != null) ommProvider_1.Uninitialize();
                if (ommProvider_4 != null) ommProvider_4.Uninitialize();
            }
        }

        [Fact]
        public void TestMultiConnectionItemGroupClosedRecoverableWithServiceList()
        {
            OmmIProviderConfig providerConfig = new(EmaConfigFileLocation);
            ProviderTestOptions providerTestOptions = new ProviderTestOptions();

            ProviderTestClient providerClient_1 = new ProviderTestClient(output, providerTestOptions);
            OmmProvider ommProvider_1 = new OmmProvider(providerConfig.Port("19001").ProviderName("Provider_1"), providerClient_1);

            ProviderTestClient providerClient_4 = new ProviderTestClient(output, providerTestOptions);
            OmmProvider ommProvider_4 = new OmmProvider(providerConfig.Port("19004").ProviderName("Provider_3"), providerClient_4);

            ConsumerTestOptions options = new();
            options.GetChannelInformation = true;
            using ConsumerTestClient consumerClient = new ConsumerTestClient(output, options);

            OmmConsumer? consumer = null;

            try
            {
                ElementList payload = new ElementList();
                ElementEntry entry = new ElementEntry();

                ElementList eePayload = new ElementList();
                ElementEntry eeEntry = new ElementEntry();

                eePayload.AddUInt(":DataStreams", SymbolList.SymbolListDataStreamRequestFlags.SYMBOL_LIST_DATA_STREAMS);
                eePayload.Complete();

                payload.AddElementList(":SymbolListBehaviors", eePayload);
                payload.Complete();

                ServiceList serviceList = new ServiceList("SVG1");

                serviceList.ConcreteServiceList.Add("DIRECT_FEED");
                serviceList.ConcreteServiceList.Add("DIRECT_FEED_2");

                RequestMsg reqMsg = new RequestMsg();
                reqMsg.DomainType(EmaRdm.MMT_SYMBOL_LIST).ServiceListName("SVG1").Name(".AV.N").Payload(payload);

                consumer = new OmmConsumer(new OmmConsumerConfig(EmaConfigFileLocation).ConsumerName("Consumer_9").AddServiceList(serviceList), consumerClient);

                OmmConsumerImpl ommConsumerImpl = consumer.m_OmmConsumerImpl!;

                Thread.Sleep(1500);

                Msg msg = consumerClient.WaitForMessage<Msg>();
                msg.MarkForClear();

                int count = consumerClient.QueueSize();
                for (int i = 0; i < count; i++) { msg = consumerClient.PopMessage(); msg.MarkForClear(); }

                count = providerClient_1.QueueSize();
                for (int i = 0; i < count; i++) { msg = providerClient_1.PopMessage(); msg.MarkForClear(); }

                long itemHandle = consumer.RegisterClient(reqMsg, consumerClient);

                Thread.Sleep(3000);

                reqMsg.MarkForClear();
                payload.MarkForClear();
                eePayload.MarkForClear();

                Assert.NotNull(consumer.m_OmmConsumerImpl);
                ConsumerSession<IOmmConsumerClient>? consumerSession = consumer.m_OmmConsumerImpl.ConsumerSession;
                Assert.NotNull(consumerSession);
                Assert.NotNull(consumerSession.ServiceListDict);
                int serviceId = consumerSession.ServiceListDict["SVG1"].ServiceId;

                Assert.Equal(4, consumerClient.QueueSize());
                Msg message = consumerClient.PopMessage();

                RefreshMsg refreshMsg = (RefreshMsg)message;
                Assert.Equal(".AV.N", refreshMsg.Name());
                Assert.Equal(OmmState.StreamStates.OPEN, refreshMsg.State().StreamState);
                Assert.Equal(OmmState.DataStates.OK, refreshMsg.State().DataState);
                Assert.Equal(OmmState.StatusCodes.NONE, (int)refreshMsg.State().Code);
                Assert.True(refreshMsg.Solicited());

                message.MarkForClear();

                for (int i = 0; i < 3; i++)
                {
                    message = consumerClient.PopMessage();
                    
                    Assert.True(message is RefreshMsg);
                    refreshMsg = (RefreshMsg)message;
                    Assert.True("itemA".Equals(refreshMsg.Name()) || "itemB".Equals(refreshMsg.Name()) || "itemC".Equals(refreshMsg.Name()));

                    message.MarkForClear();
                }

                /* Send item recoverable status from the first provider */
                long providerItemHandle = providerClient_1.RetriveItemHandle(".AV.N");

                Assert.True(providerItemHandle > 0);

                ElementList serviceState = new ElementList();
                serviceState.AddUInt(EmaRdm.ENAME_SVC_STATE, EmaRdm.SERVICE_DOWN);
                serviceState.AddUInt(EmaRdm.ENAME_ACCEPTING_REQS, 1);
                serviceState.AddState(EmaRdm.ENAME_STATUS, OmmState.StreamStates.OPEN, OmmState.DataStates.SUSPECT, OmmState.StatusCodes.NONE);
                serviceState.Complete();

                FilterList filterListEnc = new FilterList();
                filterListEnc.AddEntry(EmaRdm.SERVICE_STATE_ID, FilterAction.UPDATE, serviceState);
                filterListEnc.Complete();

                Map map = new Map();
                map.AddKeyUInt(1, MapAction.UPDATE, filterListEnc);
                map.Complete();

                UpdateMsg updateMsg = new UpdateMsg();
                ommProvider_1.Submit(updateMsg.DomainType(EmaRdm.MMT_DIRECTORY).Filter(EmaRdm.SERVICE_STATE_FILTER).Payload(map), 0);   // use 0 item handle to fan-out to all subscribers

                StatusMsg statusMsg = new StatusMsg();
                ommProvider_1.Submit(statusMsg.DomainType(EmaRdm.MMT_SYMBOL_LIST)
                                .State(OmmState.StreamStates.CLOSED_RECOVER, OmmState.DataStates.SUSPECT, OmmState.StatusCodes.NONE, "Item temporary closed"), providerItemHandle);


                Thread.Sleep(12000); // Wait until consumer receives the item closed recoverable status message.

                serviceState.MarkForClear();
                filterListEnc.MarkForClear();
                map.MarkForClear();
                updateMsg.MarkForClear();

                count = consumerClient.QueueSize();

                int closedItemsCount = 0;
                for (int i = 0; i < count; i++)
                {
                    message = consumerClient.PopMessage();
                    if (message is StatusMsg)
                    {
                        StatusMsg status = (StatusMsg)message;
                        if (status.State().StatusText.Contains("This individual item of the Symbol List"))
                        {
                            closedItemsCount++;
                        }
                        else if (status.Name().Equals(".AV.N"))
                        {
                            Assert.Equal("SVG1", status.ServiceName());
                            Assert.Equal(serviceId, status.ServiceId());
                            Assert.Equal(OmmState.StreamStates.OPEN, status.State().StreamState);
                            Assert.Equal(OmmState.DataStates.SUSPECT, status.State().DataState);
                            Assert.Equal(OmmState.StatusCodes.NONE, status.State().StatusCode);
                        }
                    }
                    else if (message is RefreshMsg)
                    {
                        RefreshMsg refresh = (RefreshMsg)message;
                        Assert.Equal("SVG1", refreshMsg.ServiceName());
                        Assert.Equal(serviceId, refreshMsg.ServiceId());
                        Assert.Equal(OmmState.StreamStates.OPEN, refreshMsg.State().StreamState);
                        Assert.Equal(OmmState.DataStates.OK, refreshMsg.State().DataState);
                        Assert.Equal(OmmState.StatusCodes.NONE, (int)refreshMsg.State().Code);

                        Assert.True(".AV.N".Equals(refreshMsg.Name())
                                || "itemA".Equals(refreshMsg.Name()) || "itemB".Equals(refreshMsg.Name()) || "itemC".Equals(refreshMsg.Name()));
                    }

                    message.MarkForClear();
                }
                Assert.Equal(3, closedItemsCount);

                Thread.Sleep(2000);

                consumer.Unregister(itemHandle);

                Thread.Sleep(1000);
            }
            catch (Exception e)
            {
                output.WriteLine(e.StackTrace ?? "Exception: null stack trace");
                Assert.False(true);
            }
            finally
            {
                output.WriteLine("Uninitializing...");

                if (consumer != null) consumer.Uninitialize();
                if (ommProvider_1 != null) ommProvider_1.Uninitialize();
                if (ommProvider_4 != null) ommProvider_4.Uninitialize();
            }
        }

        [Fact]
        public void TestSymbolListRecoveryReconnectWithChannelList()
        {
            OmmIProviderConfig providerConfig = new(EmaConfigFileLocation);
            ProviderTestOptions providerTestOptions = new ProviderTestOptions();

            ProviderTestClient providerClient_1 = new ProviderTestClient(output, providerTestOptions);
            OmmProvider ommProvider_1 = new OmmProvider(providerConfig.Port("19001").ProviderName("Provider_1"), providerClient_1);

            ProviderTestClient providerClient_2 = new ProviderTestClient(output, providerTestOptions);
            OmmProvider ommProvider_2 = new OmmProvider(providerConfig.Port("19002").ProviderName("Provider_1"), providerClient_2);

            ProviderTestClient providerClient_3 = new ProviderTestClient(output, providerTestOptions);
            OmmProvider ommProvider_3 = new OmmProvider(providerConfig.Port("19003").ProviderName("Provider_1"), providerClient_3);

            ConsumerTestOptions options = new();
            options.GetChannelInformation = true;
            using ConsumerTestClient consumerClient = new ConsumerTestClient(output, options);

            OmmConsumer? consumer = null;

            try
            {
                ElementList payload = new ElementList();
                ElementEntry entry = new ElementEntry();

                ElementList eePayload = new ElementList();
                ElementEntry eeEntry = new ElementEntry();

                eePayload.AddUInt(":DataStreams", SymbolList.SymbolListDataStreamRequestFlags.SYMBOL_LIST_DATA_STREAMS);
                eePayload.Complete();

                payload.AddElementList(":SymbolListBehaviors", eePayload);
                payload.Complete();

                consumer = new OmmConsumer(new OmmConsumerConfig(EmaConfigFileLocation).ConsumerName("Consumer_1_2"), consumerClient);

                OmmConsumerImpl ommConsumerImpl = consumer.m_OmmConsumerImpl!;

                Thread.Sleep(1500);

                Msg msg = consumerClient.WaitForMessage<Msg>();
                msg.MarkForClear();

                int count = consumerClient.QueueSize();
                for (int i = 0; i < count; i++) { msg = consumerClient.PopMessage(); msg.MarkForClear(); }

                RequestMsg reqMsg = new RequestMsg();
                reqMsg.DomainType(EmaRdm.MMT_SYMBOL_LIST).ServiceName("DIRECT_FEED").Name(".AV.N").Payload(payload);

                consumer.RegisterClient(reqMsg, consumerClient);

                Thread.Sleep(2500);

                reqMsg.MarkForClear();
                payload.MarkForClear();
                eePayload.MarkForClear();

                HashSet<int> negativeStreamIds = new();
                negativeStreamIds.Add(-1);
                negativeStreamIds.Add(-2);
                negativeStreamIds.Add(-3);

                for (int i = 0; i < 4; i++)
                {
                    Msg message = consumerClient.PopMessage();
                    Assert.True(message is RefreshMsg);
                    RefreshMsg refresh = (RefreshMsg)message;

                    if (refresh.DomainType() == (int)DomainType.SYMBOL_LIST)
                    {
                        Assert.Equal(5, refresh.StreamId());
                        Assert.Equal(".AV.N", refresh.Name());
                    }
                    else if (refresh.DomainType() == (int)DomainType.MARKET_PRICE) Assert.Contains(refresh.StreamId(), negativeStreamIds);

                    message.MarkForClear();
                }

                output.WriteLine("\n >>> Killing provider_1 \n");

                ommProvider_1.Uninitialize();

                Thread.Sleep(10000);

                count = consumerClient.QueueSize();
                int statusClosedCount = 0;
                int refreshCount = 0;
                for (int i = 0; i < count; i++)
                {
                    Msg message = consumerClient.PopMessage();
                    if (message is StatusMsg)
                    {
                        StatusMsg status = (StatusMsg)message;
                        if (status.State().StatusText.Contains("Individual item from Symbol List closed due to server change"))
                        {
                            Assert.True(status.StreamId() < 0);
                            Assert.True(status.DomainType() == (int)DomainType.MARKET_PRICE);
                            statusClosedCount++;
                        }
                    }
                    else if (message is RefreshMsg)
                    {
                        RefreshMsg refresh = (RefreshMsg)message;
                        if (refresh.DomainType() == (int)DomainType.MARKET_PRICE || refresh.DomainType() == (int)DomainType.SYMBOL_LIST)
                        {
                            Assert.True(refresh.HasName);
                            Assert.True(refresh.Name().Equals(".AV.N")
                                    || refresh.Name().Equals("itemA")
                                    || refresh.Name().Equals("itemB")
                                    || refresh.Name().Equals("itemC"));
                            refreshCount++;
                        }
                    }
                    message.MarkForClear();
                }
                Assert.Equal(3, statusClosedCount);
                Assert.Equal(4, refreshCount);

                output.WriteLine("\n >>> Killing provider_2 \n");

                ommProvider_2.Uninitialize();

                Thread.Sleep(10000);

                count = consumerClient.QueueSize();
                statusClosedCount = 0;
                refreshCount = 0;

                for (int i = 0; i < count; i++)
                {
                    Msg message = consumerClient.PopMessage();
                    if (message is StatusMsg)
                    {
                        StatusMsg status = (StatusMsg)message;
                        if (status.State().StatusText.Contains("Individual item from Symbol List closed due to server change"))
                        {
                            Assert.True(status.StreamId() < 0);
                            Assert.True(status.DomainType() == (int)DomainType.MARKET_PRICE);
                            statusClosedCount++;
                        }
                    }
                    else if (message is RefreshMsg)
                    {
                        RefreshMsg refresh = (RefreshMsg)message;
                        if (refresh.DomainType() == (int)DomainType.MARKET_PRICE || refresh.DomainType() == (int)DomainType.SYMBOL_LIST)
                        {
                            Assert.True(refresh.HasName);
                            Assert.True(refresh.Name().Equals(".AV.N")
                                    || refresh.Name().Equals("itemA")
                                    || refresh.Name().Equals("itemB")
                                    || refresh.Name().Equals("itemC"));
                            refreshCount++;
                        }  
                    }
                    message.MarkForClear();
                }
                Assert.Equal(3, statusClosedCount);
                Assert.Equal(4, refreshCount);

                providerClient_1 = new ProviderTestClient(output, providerTestOptions);
                ommProvider_1 = new OmmProvider(providerConfig.Port("19001").ProviderName("Provider_1"), providerClient_1);

                providerClient_2 = new ProviderTestClient(output, providerTestOptions);
                ommProvider_2 = new OmmProvider(providerConfig.Port("19002").ProviderName("Provider_1"), providerClient_2);

                output.WriteLine("\n >>> Killing provider_3 \n");

                ommProvider_3.Uninitialize();

                Thread.Sleep(10000);

                count = consumerClient.QueueSize();
                statusClosedCount = 0;
                refreshCount = 0;

                for (int i = 0; i < count; i++)
                {
                    Msg message = consumerClient.PopMessage();
                    if (message is StatusMsg)
                    {
                        StatusMsg status = (StatusMsg)message;
                        if (status.State().StatusText.Contains("Individual item from Symbol List closed due to server change"))
                        {
                            Assert.True(status.StreamId() < 0);
                            Assert.True(status.DomainType() == (int)DomainType.MARKET_PRICE);
                            statusClosedCount++;
                        }
                    }
                    else if (message is RefreshMsg)
                    {
                        RefreshMsg refresh = (RefreshMsg)message;
                        if (refresh.DomainType() == (int)DomainType.MARKET_PRICE || refresh.DomainType() == (int)DomainType.SYMBOL_LIST)
                        {
                            Assert.True(refresh.HasName);
                            Assert.True(refresh.Name().Equals(".AV.N")
                                    || refresh.Name().Equals("itemA")
                                    || refresh.Name().Equals("itemB")
                                    || refresh.Name().Equals("itemC"));
                            refreshCount++;
                        }   
                    }
                    message.MarkForClear();
                }
                Assert.Equal(3, statusClosedCount);
                Assert.Equal(4, refreshCount);
            }
            catch (Exception e)
            {
                output.WriteLine(e.StackTrace ?? "Exception: null stack trace");
                Assert.False(true);
            }
            finally
            {
                output.WriteLine("Uninitializing...");

                if (consumer != null) consumer.Uninitialize();
                if (ommProvider_1 != null) ommProvider_1.Uninitialize();
                if (ommProvider_2 != null) ommProvider_2.Uninitialize();
                if (ommProvider_3 != null) ommProvider_3.Uninitialize();
            }
        }

        // Modified Multiconnection/Preferred Host tests

        [Fact]
        public void SingleConnectionFallbackToPreferredChannelOnChannelListUponDetectionIntervalTest()
        {
            OmmConsumer? consumer = null;
            ConsumerTestOptions consumerOption = new();
            ProviderTestOptions providerTestOptions = new();
            providerTestOptions.SupportStandby = true;
            providerTestOptions.SendRefreshAttrib = true;

            ProviderTestClient providerClient_1 = new(output, providerTestOptions);
            ProviderTestClient providerClient_2 = new(output, providerTestOptions);
            ProviderTestClient providerClient_3 = new(output, providerTestOptions);

            OmmIProviderConfig config = new OmmIProviderConfig(EmaConfigFileLocation);

            consumerOption.GetChannelInformation = true;
            consumerOption.GetSessionChannelInfo = false;
            using ConsumerTestClient consumerClient = new(output, consumerOption);


            // Channel_1
            OmmProvider ommprovider_1 = new(config.Port("19001").ProviderName("Provider_1"), providerClient_1);

            // Channel_2 /* This is preferred host */
            OmmProvider? ommprovider_2 = null;

            // Channel_3
            OmmProvider ommprovider_3 = new(config.Port("19003").ProviderName("Provider_1"), providerClient_3);

            try
            {
                ConsumerTestOptions options = new();
                options.GetChannelInformation = true;

                consumer = new OmmConsumer(new OmmConsumerConfig(EmaConfigFileLocation).ConsumerName("Consumer_13"), consumerClient);

                string serviceName = "DIRECT_FEED";
                string itemName = ".AV.N";

                ElementList payload = new ElementList();
                ElementEntry entry = new ElementEntry();

                ElementList eePayload = new ElementList();
                ElementEntry eeEntry = new ElementEntry();

                eePayload.AddUInt(":DataStreams", SymbolList.SymbolListDataStreamRequestFlags.SYMBOL_LIST_DATA_STREAMS);
                eePayload.Complete();

                payload.AddElementList(":SymbolListBehaviors", eePayload);
                payload.Complete();

                RequestMsg reqMsg = new();
                long itemHandle = consumer.RegisterClient(reqMsg.Name(itemName).ServiceName(serviceName).DomainType((int)DomainType.SYMBOL_LIST).Payload(payload), consumerClient);

                payload.MarkForClear();
                payload.MarkForClear();
                eePayload.MarkForClear();
                reqMsg.MarkForClear();

                StatusMsg statusMsg = consumerClient.WaitForMessage<StatusMsg>();

                Assert.Equal(1, statusMsg.StreamId());
                Assert.Equal(EmaRdm.MMT_LOGIN, statusMsg.DomainType());
                Assert.Equal("Open / Suspect / None / 'channel down'", statusMsg.State().ToString());
                ChannelInformation channelInfo = consumerClient.PopChannelInfo();
                Assert.Equal("Channel_2", channelInfo.ChannelName);
                Assert.Equal(ChannelState.INACTIVE, channelInfo.ChannelState);

                statusMsg.MarkForClear();

                RefreshMsg refreshMsg = consumerClient.WaitForMessage<RefreshMsg>();

                Assert.Equal(1, refreshMsg.StreamId());
                Assert.Equal(EmaRdm.MMT_LOGIN, refreshMsg.DomainType());
                Assert.Equal("Open / Ok / None / 'Login accepted'", refreshMsg.State().ToString());
                Assert.True(refreshMsg.Solicited());
                Assert.True(refreshMsg.Complete());
                Assert.True(refreshMsg.HasMsgKey);
                Assert.Equal(DataTypes.NO_DATA, refreshMsg.Payload().DataType);
                Assert.Equal(DataTypes.ELEMENT_LIST, refreshMsg.Attrib().DataType);
                channelInfo = consumerClient.PopChannelInfo();
                Assert.Equal("Channel_1", channelInfo.ChannelName);
                Assert.Equal(ChannelState.ACTIVE, channelInfo.ChannelState);
                Assert.True(channelInfo.PreferredHostInfo!.IsPreferredHostEnabled);
                Assert.Equal(7, channelInfo.PreferredHostInfo!.DetectionTimeInterval);
                Assert.Equal("", channelInfo.PreferredHostInfo!.DetectionTimeSchedule);
                Assert.Equal("Channel_2", channelInfo.PreferredHostInfo!.ChannelName);
                Assert.True(6 >= channelInfo.PreferredHostInfo!.RemainingDetectionTime);

                refreshMsg.MarkForClear();

                //Checks the market price item refresh from the Channel_1
                refreshMsg = consumerClient.WaitForMessage<RefreshMsg>();

                Assert.Equal(5, refreshMsg.StreamId());
                Assert.Equal(EmaRdm.MMT_SYMBOL_LIST, refreshMsg.DomainType());
                Assert.Equal("Open / Ok / None / 'Refresh Completed'", refreshMsg.State().ToString());
                Assert.True(refreshMsg.Complete());
                Assert.True(refreshMsg.Solicited());
                Assert.True(refreshMsg.HasName);
                Assert.Equal(itemName, refreshMsg.Name());
                Assert.True(refreshMsg.HasServiceId);
                Assert.True(refreshMsg.HasServiceName);
                Assert.Equal(serviceName, refreshMsg.ServiceName());
               // Assert.Equal(DataTypes.FIELD_LIST, refreshMsg.Payload().DataType);
                channelInfo = consumerClient.PopChannelInfo();
                Assert.Equal("Channel_1", channelInfo.ChannelName);
                Assert.Equal(ChannelState.ACTIVE, channelInfo.ChannelState);
                Assert.True(channelInfo.PreferredHostInfo!.IsPreferredHostEnabled);
                Assert.Equal(7, channelInfo.PreferredHostInfo!.DetectionTimeInterval);
                Assert.Equal("", channelInfo.PreferredHostInfo!.DetectionTimeSchedule);
                Assert.Equal("Channel_2", channelInfo.PreferredHostInfo!.ChannelName);
                Assert.True(6 >= channelInfo.PreferredHostInfo!.RemainingDetectionTime);

                refreshMsg.MarkForClear();

                for (int i = 0; i < 3; i++)
                {
                    //Checks the market price item refresh from the Channel_1
                    refreshMsg = consumerClient.WaitForMessage<RefreshMsg>();

                    //Assert.Equal(5, refreshMsg.StreamId());
                    Assert.Equal(EmaRdm.MMT_MARKET_PRICE, refreshMsg.DomainType());
                    //Assert.Equal("Open / Ok / None / 'Refresh Completed'", refreshMsg.State().ToString());
                    //Assert.True(refreshMsg.Complete());
                    //Assert.True(refreshMsg.Solicited());
                    Assert.True(refreshMsg.HasName);
                    //Assert.Equal(itemName, refreshMsg.Name());
                    Assert.True(refreshMsg.HasServiceId);
                    Assert.True(refreshMsg.HasServiceName);
                    Assert.Equal(serviceName, refreshMsg.ServiceName());
                    Assert.Equal(DataTypes.FIELD_LIST, refreshMsg.Payload().DataType);
                    channelInfo = consumerClient.PopChannelInfo();
                    Assert.Equal("Channel_1", channelInfo.ChannelName);
                    Assert.Equal(ChannelState.ACTIVE, channelInfo.ChannelState);
                    Assert.True(channelInfo.PreferredHostInfo!.IsPreferredHostEnabled);
                    Assert.Equal(7, channelInfo.PreferredHostInfo!.DetectionTimeInterval);
                    Assert.Equal("", channelInfo.PreferredHostInfo!.DetectionTimeSchedule);
                    Assert.Equal("Channel_2", channelInfo.PreferredHostInfo!.ChannelName);
                    Assert.True(6 >= channelInfo.PreferredHostInfo!.RemainingDetectionTime);

                    refreshMsg.MarkForClear();
                }

                // Checks for PH START and COMPLETE events
                /* Checks login status messages */
                statusMsg = consumerClient.WaitForMessage<StatusMsg>();

                Assert.Equal(1, statusMsg.StreamId());
                Assert.Equal(EmaRdm.MMT_LOGIN, statusMsg.DomainType());
                Assert.Equal("Open / Ok / PreferredHostStartingFallback / 'preferred host starting fallback'", statusMsg.State().ToString());
                Assert.True(statusMsg.HasMsgKey);
                Assert.Equal(DataTypes.NO_DATA, statusMsg.Payload().DataType);
                channelInfo = consumerClient.PopChannelInfo();
                Assert.Equal("Channel_1", channelInfo.ChannelName);
                Assert.Equal(ChannelState.ACTIVE, channelInfo.ChannelState);
                Assert.True(channelInfo.PreferredHostInfo!.IsPreferredHostEnabled);
                Assert.Equal(7, channelInfo.PreferredHostInfo!.DetectionTimeInterval);
                Assert.Equal("", channelInfo.PreferredHostInfo!.DetectionTimeSchedule);
                Assert.Equal("Channel_2", channelInfo.PreferredHostInfo!.ChannelName);
                Assert.True(6 >= channelInfo.PreferredHostInfo!.RemainingDetectionTime);

                statusMsg.MarkForClear();

                statusMsg = consumerClient.WaitForMessage<StatusMsg>();

                Assert.Equal(1, statusMsg.StreamId());
                Assert.Equal(EmaRdm.MMT_LOGIN, statusMsg.DomainType());
                Assert.Equal("Open / Ok / PreferredHostComplete / 'preferred host complete'", statusMsg.State().ToString());
                Assert.True(statusMsg.HasMsgKey);
                Assert.Equal(DataTypes.NO_DATA, statusMsg.Payload().DataType);
                channelInfo = consumerClient.PopChannelInfo();
                Assert.Equal("Channel_1", channelInfo.ChannelName);
                Assert.Equal(ChannelState.ACTIVE, channelInfo.ChannelState);
                Assert.True(channelInfo.PreferredHostInfo!.IsPreferredHostEnabled);
                Assert.Equal(7, channelInfo.PreferredHostInfo!.DetectionTimeInterval);
                Assert.Equal("", channelInfo.PreferredHostInfo!.DetectionTimeSchedule);
                Assert.Equal("Channel_2", channelInfo.PreferredHostInfo!.ChannelName);
                Assert.True(6 >= channelInfo.PreferredHostInfo!.RemainingDetectionTime);

                statusMsg.MarkForClear();

                output.WriteLine("Bring up the preferred channel.");

                // Start the provider for Channel_2 */
                ommprovider_2 = new OmmProvider(config.Port("19002").ProviderName("Provider_1"), providerClient_2);

                // The fallback should happen by the detection time interval
                output.WriteLine("Fallback by the detection time interval to connect to the preferred channel.");

                statusMsg = consumerClient.WaitForMessage<StatusMsg>();
                
                Assert.Equal(1, statusMsg.StreamId());
                Assert.Equal(EmaRdm.MMT_LOGIN, statusMsg.DomainType());
                Assert.Equal("Open / Ok / PreferredHostStartingFallback / 'preferred host starting fallback'", statusMsg.State().ToString());
                Assert.True(statusMsg.HasMsgKey);
                Assert.Equal(DataTypes.NO_DATA, statusMsg.Payload().DataType);
                Assert.Equal(DataTypes.NO_DATA, statusMsg.Attrib().DataType);
                channelInfo = consumerClient.PopChannelInfo();
                Assert.Equal("Channel_1", channelInfo.ChannelName);
                Assert.Equal(ChannelState.ACTIVE, channelInfo.ChannelState);
                Assert.True(channelInfo.PreferredHostInfo!.IsPreferredHostEnabled);
                Assert.Equal(7, channelInfo.PreferredHostInfo!.DetectionTimeInterval);
                Assert.Equal("", channelInfo.PreferredHostInfo!.DetectionTimeSchedule);
                Assert.Equal("Channel_2", channelInfo.PreferredHostInfo!.ChannelName);

                statusMsg.MarkForClear();

                bool foundSymbolListStatus = false;
                bool foundLoginStatus = false;
                int mpCount = 0;

                for (int i = 0; i < 5; i++)
                {
                    statusMsg = consumerClient.WaitForMessage<StatusMsg>();
                    if (statusMsg.DomainType() == EmaRdm.MMT_MARKET_PRICE || statusMsg.DomainType() == EmaRdm.MMT_SYMBOL_LIST)
                    {
                        if (statusMsg.DomainType() == EmaRdm.MMT_MARKET_PRICE)
                        {
                            Assert.True(statusMsg.StreamId() == -1 || statusMsg.StreamId() == -2 || statusMsg.StreamId() == -3);
                            Assert.Equal(EmaRdm.MMT_MARKET_PRICE, statusMsg.DomainType());
                            Assert.True(statusMsg.HasState);
                            Assert.Equal("Closed / Suspect / None / 'Individual item from Symbol List closed due to server change.'", statusMsg.State().ToString());
                            Assert.True(statusMsg.HasName);
                            Assert.True("itemA".Equals(statusMsg.Name()) || "itemB".Equals(statusMsg.Name()) || "itemC".Equals(statusMsg.Name()));
                            mpCount++;
                        }
                        else if (statusMsg.DomainType() == EmaRdm.MMT_SYMBOL_LIST)
                        {
                            Assert.Equal(5, statusMsg.StreamId());
                            Assert.True(statusMsg.HasState);
                            Assert.Equal("Open / Suspect / None / 'channel down.'", statusMsg.State().ToString());
                            Assert.True(statusMsg.HasName);
                            Assert.Equal(itemName, statusMsg.Name());
                            foundSymbolListStatus = true;
                        }
                        Assert.True(statusMsg.HasServiceId);
                        Assert.True(statusMsg.HasServiceName);
                        Assert.Equal(serviceName, statusMsg.ServiceName());
                        Assert.Equal(DataTypes.NO_DATA, statusMsg.Payload().DataType);
                        channelInfo = consumerClient.PopChannelInfo();
                        Assert.Equal("Channel_1", channelInfo.ChannelName);
                        Assert.Equal(ChannelState.INACTIVE, channelInfo.ChannelState);
                    }
                    else if (statusMsg.DomainType() == EmaRdm.MMT_LOGIN)
                    {
                        Assert.Equal(1, statusMsg.StreamId());
                        Assert.Equal(EmaRdm.MMT_LOGIN, statusMsg.DomainType());
                        Assert.Equal("Open / Suspect / None / ''", statusMsg.State().ToString());
                        Assert.False(statusMsg.HasMsgKey);
                        Assert.Equal(DataTypes.NO_DATA, statusMsg.Payload().DataType);
                        Assert.Equal(DataTypes.NO_DATA, statusMsg.Attrib().DataType);
                        channelInfo = consumerClient.PopChannelInfo();
                        Assert.Equal("Channel_1", channelInfo.ChannelName);
                        Assert.Equal(ChannelState.INACTIVE, channelInfo.ChannelState);
                        foundLoginStatus = true;
                    }

                    statusMsg.MarkForClear();
                }
                Assert.True(foundSymbolListStatus);
                Assert.True(foundLoginStatus);
                Assert.True(mpCount == 3);

                statusMsg = consumerClient.WaitForMessage<StatusMsg>();

                Assert.Equal(1, statusMsg.StreamId());
                Assert.Equal(EmaRdm.MMT_LOGIN, statusMsg.DomainType());
                Assert.Equal("Open / Suspect / PreferredHostComplete / 'preferred host complete'", statusMsg.State().ToString());
                Assert.True(statusMsg.HasMsgKey);
                Assert.Equal(DataTypes.NO_DATA, statusMsg.Payload().DataType);
                channelInfo = consumerClient.PopChannelInfo();
                Assert.Equal("Channel_2", channelInfo.ChannelName);
                Assert.Equal(ChannelState.ACTIVE, channelInfo.ChannelState);
                Assert.True(channelInfo.PreferredHostInfo!.IsPreferredHostEnabled);
                Assert.Equal(7, channelInfo.PreferredHostInfo!.DetectionTimeInterval);
                Assert.Equal("", channelInfo.PreferredHostInfo!.DetectionTimeSchedule);
                Assert.Equal("Channel_2", channelInfo.PreferredHostInfo!.ChannelName);

                statusMsg.MarkForClear();

                refreshMsg = consumerClient.WaitForMessage<RefreshMsg>();

                Assert.Equal(1, refreshMsg.StreamId());
                Assert.Equal(EmaRdm.MMT_LOGIN, refreshMsg.DomainType());
                Assert.Equal("Open / Ok / None / 'Login accepted'", refreshMsg.State().ToString());
                Assert.True(refreshMsg.Solicited());
                Assert.True(refreshMsg.Complete());
                Assert.True(refreshMsg.HasMsgKey);
                Assert.Equal(DataTypes.NO_DATA, refreshMsg.Payload().DataType);
                Assert.Equal(DataTypes.ELEMENT_LIST, refreshMsg.Attrib().DataType);
                channelInfo = consumerClient.PopChannelInfo();
                Assert.Equal("Channel_2", channelInfo.ChannelName);
                Assert.Equal(ChannelState.ACTIVE, channelInfo.ChannelState);
                Assert.True(channelInfo.PreferredHostInfo!.IsPreferredHostEnabled);
                Assert.Equal("Channel_2", channelInfo.PreferredHostInfo?.ChannelName);

                refreshMsg.MarkForClear();

                /* Checks login status messages */
                statusMsg = consumerClient.WaitForMessage<StatusMsg>();

                Assert.Equal(1, statusMsg.StreamId());
                Assert.Equal(EmaRdm.MMT_LOGIN, statusMsg.DomainType());
                Assert.Equal("Open / Ok / None / 'channel up'", statusMsg.State().ToString());
                Assert.True(statusMsg.HasMsgKey);
                Assert.Equal(DataTypes.NO_DATA, statusMsg.Payload().DataType);
                channelInfo = consumerClient.PopChannelInfo();
                Assert.Equal("Channel_2", channelInfo.ChannelName);
                Assert.Equal(ChannelState.ACTIVE, channelInfo.ChannelState);
                Assert.True(channelInfo.PreferredHostInfo!.IsPreferredHostEnabled);
                Assert.Equal(7, channelInfo.PreferredHostInfo!.DetectionTimeInterval);
                Assert.Equal("", channelInfo.PreferredHostInfo!.DetectionTimeSchedule);
                Assert.Equal("Channel_2", channelInfo.PreferredHostInfo!.ChannelName);

                statusMsg.MarkForClear();

                //Checks the market price item refresh from the starting channel of WSB-G1 after the fallback is trigger 
                refreshMsg = consumerClient.WaitForMessage<RefreshMsg>();

                Assert.Equal(5, refreshMsg.StreamId());
                Assert.Equal(EmaRdm.MMT_SYMBOL_LIST, refreshMsg.DomainType());
                Assert.Equal("Open / Ok / None / 'Refresh Completed'", refreshMsg.State().ToString());
                Assert.True(refreshMsg.Complete());
                Assert.True(refreshMsg.Solicited());
                Assert.True(refreshMsg.HasName);
                Assert.Equal(itemName, refreshMsg.Name());
                Assert.True(refreshMsg.HasServiceId);
                Assert.True(refreshMsg.HasServiceName);
                Assert.Equal(serviceName, refreshMsg.ServiceName());
                Assert.Equal(DataTypes.MAP, refreshMsg.Payload().DataType);
                channelInfo = consumerClient.PopChannelInfo();
                Assert.Equal("Channel_2", channelInfo.ChannelName);
                Assert.Equal(ChannelState.ACTIVE, channelInfo.ChannelState);
                Assert.True(channelInfo.PreferredHostInfo!.IsPreferredHostEnabled);
                Assert.Equal(7, channelInfo.PreferredHostInfo!.DetectionTimeInterval);
                Assert.Equal("", channelInfo.PreferredHostInfo!.DetectionTimeSchedule);
                Assert.Equal("Channel_2", channelInfo.PreferredHostInfo!.ChannelName);

                refreshMsg.MarkForClear();

                for (int i = 0; i < 3; i++)
                {
                    //Checks the market price item refresh from the Channel_1
                    refreshMsg = consumerClient.WaitForMessage<RefreshMsg>();

                    //Assert.Equal(5, refreshMsg.StreamId());
                    Assert.Equal(EmaRdm.MMT_MARKET_PRICE, refreshMsg.DomainType());
                    //Assert.Equal("Open / Ok / None / 'Refresh Completed'", refreshMsg.State().ToString());
                    Assert.True(refreshMsg.Complete());
                    //Assert.True(refreshMsg.Solicited());
                    Assert.True(refreshMsg.HasName);
                    //Assert.Equal(itemName, refreshMsg.Name());
                    Assert.True(refreshMsg.HasServiceId);
                    Assert.True(refreshMsg.HasServiceName);
                    Assert.Equal(serviceName, refreshMsg.ServiceName());
                    Assert.Equal(DataTypes.FIELD_LIST, refreshMsg.Payload().DataType);
                    channelInfo = consumerClient.PopChannelInfo();
                    Assert.Equal("Channel_2", channelInfo.ChannelName);
                    Assert.Equal(ChannelState.ACTIVE, channelInfo.ChannelState);
                    Assert.True(channelInfo.PreferredHostInfo!.IsPreferredHostEnabled);
                    Assert.Equal(7, channelInfo.PreferredHostInfo!.DetectionTimeInterval);
                    Assert.Equal("", channelInfo.PreferredHostInfo!.DetectionTimeSchedule);
                    Assert.Equal("Channel_2", channelInfo.PreferredHostInfo!.ChannelName);

                    refreshMsg.MarkForClear();
                }

                // Checks for PH NO FALLBACK event
                /* Checks login status messages */
                statusMsg = consumerClient.WaitForMessage<StatusMsg>();

                Assert.Equal(1, statusMsg.StreamId());
                Assert.Equal(EmaRdm.MMT_LOGIN, statusMsg.DomainType());
                Assert.Equal("Open / Ok / PreferredHostNoFallback / 'preferred host no fallback'", statusMsg.State().ToString());
                Assert.True(statusMsg.HasMsgKey);
                Assert.Equal(DataTypes.NO_DATA, statusMsg.Payload().DataType);
                channelInfo = consumerClient.PopChannelInfo();
                Assert.Equal("Channel_2", channelInfo.ChannelName);
                Assert.Equal(ChannelState.ACTIVE, channelInfo.ChannelState);
                Assert.True(channelInfo.PreferredHostInfo!.IsPreferredHostEnabled);
                Assert.Equal(7, channelInfo.PreferredHostInfo!.DetectionTimeInterval);
                Assert.Equal("", channelInfo.PreferredHostInfo!.DetectionTimeSchedule);
                Assert.Equal("Channel_2", channelInfo.PreferredHostInfo!.ChannelName);

                statusMsg.MarkForClear();

                // There should be a preferred host event
                Assert.Equal(0, consumerClient.QueueSize());

                consumer.Unregister(itemHandle);
            }
            finally
            {
                consumer?.Uninitialize();

                ommprovider_1?.Uninitialize();
                ommprovider_2?.Uninitialize();
                ommprovider_3?.Uninitialize();
            }
        }

        [Fact]
        public void MultiConnectionsFallbackToPreferredChannelOnChannelListUponDetectionIntervalTest()
        {
            OmmConsumer? consumer = null;
            ConsumerTestOptions consumerOption = new();
            ProviderTestOptions providerTestOptions = new();
            providerTestOptions.SupportStandby = true;
            providerTestOptions.SendRefreshAttrib = true;

            ProviderTestClient providerClient = new(output, providerTestOptions);
            providerClient.Name = "Provider_1";
            ProviderTestClient providerClient2 = new(output, providerTestOptions);
            providerClient2.Name = "Provider_2";

            ProviderTestClient providerClient3 = new(output, providerTestOptions);
            providerClient3.Name = "Provider_3";
            ProviderTestClient providerClient4 = new(output, providerTestOptions);
            providerClient3.Name = "Provider_4";

            OmmIProviderConfig config = new OmmIProviderConfig(EmaConfigFileLocation);

            consumerOption.GetChannelInformation = true;
            consumerOption.GetSessionChannelInfo = true;
            using ConsumerTestClient consumerClient = new(output, consumerOption);


            // Channel_1
            OmmProvider ommprovider = new(config.Port("19001").ProviderName("Provider_1"), providerClient);

            // Channel_2 /* This is preferred host for Connection_5 */
            OmmProvider? ommprovider2 = null;

            // Channel_4
            OmmProvider ommprovider3 = new(config.Port("19004").ProviderName("Provider_1"), providerClient3);

            // Channel_5 /* This is preferred host for Connection_6 */
            OmmProvider? ommprovider4 = null;

            try
            {
                ConsumerTestOptions options = new();
                options.GetChannelInformation = true;

                consumer = new OmmConsumer(new OmmConsumerConfig(EmaConfigFileLocation).ConsumerName("Consumer_16"), consumerClient);

                string serviceName = "DIRECT_FEED";
                string itemName = ".AV.N";

                ElementList payload = new ElementList();
                ElementEntry entry = new ElementEntry();

                ElementList eePayload = new ElementList();
                ElementEntry eeEntry = new ElementEntry();

                eePayload.AddUInt(":DataStreams", SymbolList.SymbolListDataStreamRequestFlags.SYMBOL_LIST_DATA_STREAMS);
                eePayload.Complete();

                payload.AddElementList(":SymbolListBehaviors", eePayload);
                payload.Complete();

                RequestMsg reqMsg = new();
                long itemHandle = consumer.RegisterClient(reqMsg.Name(itemName).ServiceName(serviceName).DomainType(EmaRdm.MMT_SYMBOL_LIST).Payload(payload), consumerClient);

                payload.MarkForClear();
                payload.MarkForClear();
                eePayload.MarkForClear();
                reqMsg.MarkForClear();

                StatusMsg statusMsg = consumerClient.WaitForMessage<StatusMsg>().MarkForClear();

                Assert.Equal(1, statusMsg.StreamId());
                Assert.Equal(EmaRdm.MMT_LOGIN, statusMsg.DomainType());
                Assert.Equal("Open / Suspect / None / 'session channel down reconnecting'", statusMsg.State().ToString());
                ChannelInformation channelInfo = consumerClient.PopChannelInfo();
                Assert.Equal("Channel_9", channelInfo.ChannelName);
                Assert.Equal("Connection_5", channelInfo.SessionChannelName);
                Assert.Equal(ChannelState.INACTIVE, channelInfo.ChannelState);
                Assert.Null(channelInfo.PreferredHostInfo);

                statusMsg = consumerClient.WaitForMessage<StatusMsg>().MarkForClear();

                channelInfo = consumerClient.PopChannelInfo();
                if (channelInfo.ChannelName.Equals("Channel_10"))
                {
                    Assert.Equal(1, statusMsg.StreamId());
                    Assert.Equal(EmaRdm.MMT_LOGIN, statusMsg.DomainType());
                    Assert.Equal("Open / Suspect / None / 'session channel down reconnecting'", statusMsg.State().ToString());
                    Assert.Equal("Channel_10", channelInfo.ChannelName);
                    Assert.Equal("Connection_6", channelInfo.SessionChannelName);
                    Assert.Equal(ChannelState.INACTIVE, channelInfo.ChannelState);
                    Assert.Null(channelInfo.PreferredHostInfo);

                    statusMsg = consumerClient.WaitForMessage<StatusMsg>().MarkForClear();
                    Assert.Equal(1, statusMsg.StreamId());
                    Assert.Equal(EmaRdm.MMT_LOGIN, statusMsg.DomainType());
                    Assert.Equal("Open / Suspect / None / 'session channel up'", statusMsg.State().ToString());
                    channelInfo = consumerClient.PopChannelInfo();
                    Assert.Equal("Channel_1", channelInfo.ChannelName);
                    Assert.Equal("Connection_5", channelInfo.SessionChannelName);
                    Assert.Equal(ChannelState.ACTIVE, channelInfo.ChannelState);
                    Assert.NotNull(channelInfo.PreferredHostInfo);
                    Assert.Equal(7, channelInfo.PreferredHostInfo!.DetectionTimeInterval);
                    Assert.Equal("", channelInfo.PreferredHostInfo!.DetectionTimeSchedule);
                    Assert.Equal("Channel_9", channelInfo.PreferredHostInfo!.ChannelName);
                }
                else if (channelInfo.ChannelName.Equals("Channel_1"))
                {
                    Assert.Equal(1, statusMsg.StreamId());
                    Assert.Equal(EmaRdm.MMT_LOGIN, statusMsg.DomainType());
                    Assert.Equal("Open / Suspect / None / 'session channel up'", statusMsg.State().ToString());
                    Assert.Equal("Channel_1", channelInfo.ChannelName);
                    Assert.Equal("Connection_5", channelInfo.SessionChannelName);
                    Assert.Equal(ChannelState.ACTIVE, channelInfo.ChannelState);
                    Assert.NotNull(channelInfo.PreferredHostInfo);
                    Assert.Equal(7, channelInfo.PreferredHostInfo!.DetectionTimeInterval);
                    Assert.Equal("", channelInfo.PreferredHostInfo!.DetectionTimeSchedule);
                    Assert.Equal("Channel_9", channelInfo.PreferredHostInfo!.ChannelName);

                    statusMsg = consumerClient.WaitForMessage<StatusMsg>().MarkForClear();

                    Assert.Equal(1, statusMsg.StreamId());
                    Assert.Equal(EmaRdm.MMT_LOGIN, statusMsg.DomainType());
                    Assert.Equal("Open / Suspect / None / 'session channel down reconnecting'", statusMsg.State().ToString());
                    channelInfo = consumerClient.PopChannelInfo();
                    Assert.Equal("Channel_10", channelInfo.ChannelName);
                    Assert.Equal("Connection_6", channelInfo.SessionChannelName);
                    Assert.Equal(ChannelState.INACTIVE, channelInfo.ChannelState);
                    Assert.Null(channelInfo.PreferredHostInfo);
                }


                statusMsg = consumerClient.WaitForMessage<StatusMsg>().MarkForClear();
                Assert.Equal(1, statusMsg.StreamId());
                Assert.Equal(EmaRdm.MMT_LOGIN, statusMsg.DomainType());
                Assert.Equal("Open / Suspect / None / 'session channel up'", statusMsg.State().ToString());
                channelInfo = consumerClient.PopChannelInfo();
                Assert.Equal("Channel_4", channelInfo.ChannelName);
                Assert.Equal("Connection_6", channelInfo.SessionChannelName);
                Assert.Equal(ChannelState.ACTIVE, channelInfo.ChannelState);
                Assert.NotNull(channelInfo.PreferredHostInfo);
                Assert.Equal(7, channelInfo.PreferredHostInfo!.DetectionTimeInterval);
                Assert.Equal("", channelInfo.PreferredHostInfo!.DetectionTimeSchedule);
                Assert.Equal("Channel_10", channelInfo.PreferredHostInfo!.ChannelName);

                RefreshMsg refreshMsg = consumerClient.WaitForMessage<RefreshMsg>().MarkForClear();

                Assert.Equal(1, refreshMsg.StreamId());
                Assert.Equal(EmaRdm.MMT_LOGIN, refreshMsg.DomainType());
                Assert.Equal("Open / Ok / None / 'Login accepted'", refreshMsg.State().ToString());
                Assert.True(refreshMsg.Solicited());
                Assert.True(refreshMsg.Complete());
                Assert.True(refreshMsg.HasMsgKey);
                Assert.Equal(DataTypes.NO_DATA, refreshMsg.Payload().DataType);
                Assert.Equal(DataTypes.ELEMENT_LIST, refreshMsg.Attrib().DataType);
                channelInfo = consumerClient.PopChannelInfo();
                Assert.Equal("Channel_4", channelInfo.ChannelName);
                Assert.Equal("Connection_6", channelInfo.SessionChannelName);
                Assert.Equal(ChannelState.ACTIVE, channelInfo.ChannelState);
                Assert.NotNull(channelInfo.PreferredHostInfo);
                Assert.Equal(7, channelInfo.PreferredHostInfo!.DetectionTimeInterval);
                Assert.Equal("", channelInfo.PreferredHostInfo!.DetectionTimeSchedule);
                Assert.Equal("Channel_10", channelInfo.PreferredHostInfo!.ChannelName);


                //Checks the market price item refresh from the Channel_1
                refreshMsg = consumerClient.WaitForMessage<RefreshMsg>().MarkForClear();

                Assert.Equal(5, refreshMsg.StreamId());
                Assert.Equal(EmaRdm.MMT_SYMBOL_LIST, refreshMsg.DomainType());
                Assert.Equal("Open / Ok / None / 'Refresh Completed'", refreshMsg.State().ToString());
                Assert.True(refreshMsg.Complete());
                Assert.True(refreshMsg.Solicited());
                Assert.True(refreshMsg.HasName);
                Assert.Equal(itemName, refreshMsg.Name());
                Assert.True(refreshMsg.HasServiceId);
                Assert.True(refreshMsg.HasServiceName);
                Assert.Equal(serviceName, refreshMsg.ServiceName());
                Assert.Equal(DataTypes.MAP, refreshMsg.Payload().DataType);
                channelInfo = consumerClient.PopChannelInfo();
                Assert.Equal("Channel_1", channelInfo.ChannelName);
                Assert.Equal("Connection_5", channelInfo.SessionChannelName);
                Assert.Equal(ChannelState.ACTIVE, channelInfo.ChannelState);
                Assert.NotNull(channelInfo.PreferredHostInfo);
                Assert.Equal(7, channelInfo.PreferredHostInfo!.DetectionTimeInterval);
                Assert.Equal("", channelInfo.PreferredHostInfo!.DetectionTimeSchedule);
                Assert.Equal("Channel_9", channelInfo.PreferredHostInfo!.ChannelName);

                refreshMsg.MarkForClear();

                for (int i = 0; i < 3; i++)
                {
                    //Checks the market price item refresh from the Channel_1
                    refreshMsg = consumerClient.WaitForMessage<RefreshMsg>();

                    //Assert.Equal(5, refreshMsg.StreamId());
                    Assert.Equal(EmaRdm.MMT_MARKET_PRICE, refreshMsg.DomainType());
                    Assert.Equal("Open / Ok / None / 'Refresh Completed'", refreshMsg.State().ToString());
                    //Assert.True(refreshMsg.Complete());
                    //Assert.True(refreshMsg.Solicited());
                    Assert.True(refreshMsg.HasName);
                    //Assert.Equal(itemName, refreshMsg.Name());
                    Assert.True(refreshMsg.HasServiceId);
                    Assert.True(refreshMsg.HasServiceName);
                    Assert.Equal(serviceName, refreshMsg.ServiceName());
                    Assert.Equal(DataTypes.FIELD_LIST, refreshMsg.Payload().DataType);
                    channelInfo = consumerClient.PopChannelInfo();
                    Assert.Equal("Channel_1", channelInfo.ChannelName);
                    Assert.Equal("Connection_5", channelInfo.SessionChannelName);
                    Assert.Equal(ChannelState.ACTIVE, channelInfo.ChannelState);
                    Assert.NotNull(channelInfo.PreferredHostInfo);
                    Assert.Equal(7, channelInfo.PreferredHostInfo!.DetectionTimeInterval);
                    Assert.Equal("", channelInfo.PreferredHostInfo!.DetectionTimeSchedule);
                    Assert.Equal("Channel_9", channelInfo.PreferredHostInfo!.ChannelName);

                    refreshMsg.MarkForClear();
                }

                Thread.Sleep(9000);

                // Checks for PH START and COMPLETE events

                /* Checks login status messages */
                statusMsg = consumerClient.WaitForMessage<StatusMsg>().MarkForClear();

                Assert.Equal(1, statusMsg.StreamId());
                Assert.Equal(EmaRdm.MMT_LOGIN, statusMsg.DomainType());
                Assert.Equal("Open / Ok / PreferredHostStartingFallback / 'preferred host starting fallback'", statusMsg.State().ToString());
                Assert.True(statusMsg.HasMsgKey);
                Assert.Equal(DataTypes.NO_DATA, statusMsg.Payload().DataType);
                channelInfo = consumerClient.PopChannelInfo();
                Assert.Equal("Channel_1", channelInfo.ChannelName);
                Assert.Equal("Connection_5", channelInfo.SessionChannelName);
                Assert.Equal(ChannelState.ACTIVE, channelInfo.ChannelState);
                Assert.NotNull(channelInfo.PreferredHostInfo);
                Assert.Equal(7, channelInfo.PreferredHostInfo!.DetectionTimeInterval);
                Assert.Equal("", channelInfo.PreferredHostInfo!.DetectionTimeSchedule);
                Assert.Equal("Channel_9", channelInfo.PreferredHostInfo!.ChannelName);

                statusMsg = consumerClient.WaitForMessage<StatusMsg>().MarkForClear();

                Assert.Equal(1, statusMsg.StreamId());
                Assert.Equal(EmaRdm.MMT_LOGIN, statusMsg.DomainType());
                Assert.Equal("Open / Ok / PreferredHostComplete / 'preferred host complete'", statusMsg.State().ToString());
                Assert.True(statusMsg.HasMsgKey);
                Assert.Equal(DataTypes.NO_DATA, statusMsg.Payload().DataType);
                channelInfo = consumerClient.PopChannelInfo();
                Assert.Equal("Channel_1", channelInfo.ChannelName);
                Assert.Equal("Connection_5", channelInfo.SessionChannelName);
                Assert.Equal(ChannelState.ACTIVE, channelInfo.ChannelState);
                Assert.NotNull(channelInfo.PreferredHostInfo);
                Assert.Equal(7, channelInfo.PreferredHostInfo!.DetectionTimeInterval);
                Assert.Equal("", channelInfo.PreferredHostInfo!.DetectionTimeSchedule);
                Assert.Equal("Channel_9", channelInfo.PreferredHostInfo!.ChannelName);

                /* Checks login status messages */
                statusMsg = consumerClient.WaitForMessage<StatusMsg>().MarkForClear();

                Assert.Equal(1, statusMsg.StreamId());
                Assert.Equal(EmaRdm.MMT_LOGIN, statusMsg.DomainType());
                Assert.Equal("Open / Ok / PreferredHostStartingFallback / 'preferred host starting fallback'", statusMsg.State().ToString());
                Assert.True(statusMsg.HasMsgKey);
                Assert.Equal(DataTypes.NO_DATA, statusMsg.Payload().DataType);
                channelInfo = consumerClient.PopChannelInfo();
                Assert.Equal("Channel_4", channelInfo.ChannelName);
                Assert.Equal("Connection_6", channelInfo.SessionChannelName);
                Assert.Equal(ChannelState.ACTIVE, channelInfo.ChannelState);
                Assert.NotNull(channelInfo.PreferredHostInfo);
                Assert.Equal(7, channelInfo.PreferredHostInfo!.DetectionTimeInterval);
                Assert.Equal("", channelInfo.PreferredHostInfo!.DetectionTimeSchedule);
                Assert.Equal("Channel_10", channelInfo.PreferredHostInfo!.ChannelName);

                statusMsg = consumerClient.WaitForMessage<StatusMsg>().MarkForClear();

                Assert.Equal(1, statusMsg.StreamId());
                Assert.Equal(EmaRdm.MMT_LOGIN, statusMsg.DomainType());
                Assert.Equal("Open / Ok / PreferredHostComplete / 'preferred host complete'", statusMsg.State().ToString());
                Assert.True(statusMsg.HasMsgKey);
                Assert.Equal(DataTypes.NO_DATA, statusMsg.Payload().DataType);
                channelInfo = consumerClient.PopChannelInfo();
                Assert.Equal("Channel_4", channelInfo.ChannelName);
                Assert.Equal("Connection_6", channelInfo.SessionChannelName);
                Assert.Equal(ChannelState.ACTIVE, channelInfo.ChannelState);
                Assert.Equal(7, channelInfo.PreferredHostInfo!.DetectionTimeInterval);
                Assert.Equal("", channelInfo.PreferredHostInfo!.DetectionTimeSchedule);
                Assert.Equal("Channel_10", channelInfo.PreferredHostInfo!.ChannelName);

                output.WriteLine("Bring up the preferred channel.");

                // Start the provider for Channel_9 */
                ommprovider2 = new OmmProvider(config.Port("19009").ProviderName("Provider_1"), providerClient2);

                // Start the provider for Channel_10 */
                ommprovider4 = new OmmProvider(config.Port("19010").ProviderName("Provider_1"), providerClient2);

                // The fallback should happen by the detection time interval
                output.WriteLine("Fallback by the detection time interval to connect to the preferred channel.");

                Thread.Sleep(7000);

                statusMsg = consumerClient.WaitForMessage<StatusMsg>().MarkForClear();
                Assert.Equal(1, statusMsg.StreamId());
                Assert.Equal(EmaRdm.MMT_LOGIN, statusMsg.DomainType());
                Assert.Equal("Open / Ok / PreferredHostStartingFallback / 'preferred host starting fallback'", statusMsg.State().ToString());
                Assert.True(statusMsg.HasMsgKey);
                Assert.Equal(DataTypes.NO_DATA, statusMsg.Payload().DataType);
                Assert.Equal(DataTypes.NO_DATA, statusMsg.Attrib().DataType);
                channelInfo = consumerClient.PopChannelInfo();
                Assert.Equal("Channel_1", channelInfo.ChannelName);
                Assert.Equal("Connection_5", channelInfo.SessionChannelName);
                Assert.Equal(ChannelState.ACTIVE, channelInfo.ChannelState);
                Assert.Equal(7, channelInfo.PreferredHostInfo!.DetectionTimeInterval);
                Assert.Equal("", channelInfo.PreferredHostInfo!.DetectionTimeSchedule);
                Assert.Equal("Channel_9", channelInfo.PreferredHostInfo!.ChannelName);

                bool foundSymbolListStatus = false;
                bool foundLoginStatus = false;
                int mpCount = 0;

                for (int i = 0; i < 5; i++)
                {
                    statusMsg = consumerClient.WaitForMessage<StatusMsg>();
                    if (statusMsg.DomainType() == EmaRdm.MMT_MARKET_PRICE || statusMsg.DomainType() == EmaRdm.MMT_SYMBOL_LIST)
                    {
                        if (statusMsg.DomainType() == EmaRdm.MMT_MARKET_PRICE)
                        {
                            Assert.True(statusMsg.StreamId() == -1 || statusMsg.StreamId() == -2 || statusMsg.StreamId() == -3);
                            Assert.Equal(EmaRdm.MMT_MARKET_PRICE, statusMsg.DomainType());
                            Assert.True(statusMsg.HasState);
                            Assert.Equal("Closed / Suspect / None / 'Individual item from Symbol List closed due to server change.'", statusMsg.State().ToString());
                            Assert.True(statusMsg.HasName);
                            Assert.True("itemA".Equals(statusMsg.Name()) || "itemB".Equals(statusMsg.Name()) || "itemC".Equals(statusMsg.Name()));
                            mpCount++;
                        }
                        else if (statusMsg.DomainType() == EmaRdm.MMT_SYMBOL_LIST)
                        {
                            Assert.Equal(5, statusMsg.StreamId());
                            Assert.True(statusMsg.HasState);
                            Assert.Equal("Open / Suspect / None / 'channel down.'", statusMsg.State().ToString());
                            Assert.True(statusMsg.HasName);
                            Assert.Equal(itemName, statusMsg.Name());
                            foundSymbolListStatus = true;
                        }
                        Assert.True(statusMsg.HasServiceId);
                        Assert.True(statusMsg.HasServiceName);
                        Assert.Equal(serviceName, statusMsg.ServiceName());
                        Assert.Equal(DataTypes.NO_DATA, statusMsg.Payload().DataType);
                        channelInfo = consumerClient.PopChannelInfo();
                        Assert.Equal("Channel_1", channelInfo.ChannelName);
                        Assert.Equal("Connection_5", channelInfo.SessionChannelName);
                        Assert.Equal(ChannelState.INACTIVE, channelInfo.ChannelState);
                    }
                    else if (statusMsg.DomainType() == EmaRdm.MMT_LOGIN)
                    {
                        Assert.Equal(1, statusMsg.StreamId());
                        Assert.Equal(EmaRdm.MMT_LOGIN, statusMsg.DomainType());
                        Assert.Equal("Open / Ok / None / 'session channel down reconnecting'", statusMsg.State().ToString());
                        Assert.True(statusMsg.HasMsgKey);
                        Assert.Equal(DataTypes.NO_DATA, statusMsg.Payload().DataType);
                        Assert.Equal(DataTypes.NO_DATA, statusMsg.Attrib().DataType);
                        channelInfo = consumerClient.PopChannelInfo();
                        Assert.Equal("Channel_1", channelInfo.ChannelName);
                        Assert.Equal("Connection_5", channelInfo.SessionChannelName);
                        Assert.Equal(ChannelState.INACTIVE, channelInfo.ChannelState);
                        foundLoginStatus = true;
                    }

                    statusMsg.MarkForClear();
                }
                Assert.True(foundSymbolListStatus);
                Assert.True(foundLoginStatus);
                Assert.True(mpCount == 3);

                statusMsg = consumerClient.WaitForMessage<StatusMsg>().MarkForClear();

                Assert.Equal(1, statusMsg.StreamId());
                Assert.Equal(EmaRdm.MMT_LOGIN, statusMsg.DomainType());
                Assert.Equal("Open / Ok / PreferredHostComplete / 'preferred host complete'", statusMsg.State().ToString());
                Assert.True(statusMsg.HasMsgKey);
                Assert.Equal(DataTypes.NO_DATA, statusMsg.Payload().DataType);
                channelInfo = consumerClient.PopChannelInfo();
                Assert.Equal("Channel_9", channelInfo.ChannelName);
                Assert.Equal("Connection_5", channelInfo.SessionChannelName);
                Assert.Equal(ChannelState.ACTIVE, channelInfo.ChannelState);
                Assert.Equal(7, channelInfo.PreferredHostInfo!.DetectionTimeInterval);
                Assert.Equal("", channelInfo.PreferredHostInfo!.DetectionTimeSchedule);
                Assert.Equal("Channel_9", channelInfo.PreferredHostInfo!.ChannelName);

                statusMsg.MarkForClear();

                refreshMsg = consumerClient.WaitForMessage<RefreshMsg>().MarkForClear();

                Assert.Equal(1, refreshMsg.StreamId());
                Assert.Equal(EmaRdm.MMT_LOGIN, refreshMsg.DomainType());
                Assert.Equal("Open / Ok / None / 'Login accepted'", refreshMsg.State().ToString());
                Assert.True(refreshMsg.Solicited());
                Assert.True(refreshMsg.Complete());
                Assert.True(refreshMsg.HasMsgKey);
                Assert.Equal(DataTypes.NO_DATA, refreshMsg.Payload().DataType);
                Assert.Equal(DataTypes.ELEMENT_LIST, refreshMsg.Attrib().DataType);
                channelInfo = consumerClient.PopChannelInfo();
                Assert.Equal("Channel_9", channelInfo.ChannelName);
                Assert.Equal("Connection_5", channelInfo.SessionChannelName);
                Assert.Equal(ChannelState.ACTIVE, channelInfo.ChannelState);
                Assert.Equal(7, channelInfo.PreferredHostInfo!.DetectionTimeInterval);
                Assert.Equal("", channelInfo.PreferredHostInfo!.DetectionTimeSchedule);
                Assert.Equal("Channel_9", channelInfo.PreferredHostInfo!.ChannelName);

                refreshMsg.MarkForClear();

                statusMsg = consumerClient.WaitForMessage<StatusMsg>().MarkForClear();

                Assert.Equal(1, statusMsg.StreamId());
                Assert.Equal(EmaRdm.MMT_LOGIN, statusMsg.DomainType());
                Assert.Equal("Open / Ok / None / 'session channel up'", statusMsg.State().ToString());
                Assert.True(statusMsg.HasMsgKey);
                Assert.Equal(DataTypes.NO_DATA, statusMsg.Payload().DataType);
                channelInfo = consumerClient.PopChannelInfo();
                Assert.Equal("Channel_9", channelInfo.ChannelName);
                Assert.Equal("Connection_5", channelInfo.SessionChannelName);
                Assert.Equal(ChannelState.ACTIVE, channelInfo.ChannelState);
                Assert.Equal(7, channelInfo.PreferredHostInfo!.DetectionTimeInterval);
                Assert.Equal("", channelInfo.PreferredHostInfo!.DetectionTimeSchedule);
                Assert.Equal("Channel_9", channelInfo.PreferredHostInfo!.ChannelName);

                statusMsg.MarkForClear();

                refreshMsg = consumerClient.WaitForMessage<RefreshMsg>().MarkForClear();

                Assert.Equal(5, refreshMsg.StreamId());
                Assert.Equal(EmaRdm.MMT_SYMBOL_LIST, refreshMsg.DomainType());
                Assert.Equal("Open / Ok / None / 'Refresh Completed'", refreshMsg.State().ToString());
                Assert.True(refreshMsg.Complete());
                Assert.True(refreshMsg.Solicited());
                Assert.True(refreshMsg.HasName);
                Assert.Equal(itemName, refreshMsg.Name());
                Assert.True(refreshMsg.HasServiceId);
                Assert.True(refreshMsg.HasServiceName);
                Assert.Equal(serviceName, refreshMsg.ServiceName());
                Assert.Equal(DataTypes.MAP, refreshMsg.Payload().DataType);
                channelInfo = consumerClient.PopChannelInfo();
                Assert.Equal("Channel_9", channelInfo.ChannelName);
                Assert.Equal("Connection_5", channelInfo.SessionChannelName);
                Assert.Equal(ChannelState.ACTIVE, channelInfo.ChannelState);
                Assert.Equal(7, channelInfo.PreferredHostInfo!.DetectionTimeInterval);
                Assert.Equal("", channelInfo.PreferredHostInfo!.DetectionTimeSchedule);
                Assert.Equal("Channel_9", channelInfo.PreferredHostInfo!.ChannelName);

                refreshMsg.MarkForClear();

                for (int i = 0; i < 3; i++)
                {
                    //Checks the market price item refresh from the Channel_1
                    refreshMsg = consumerClient.WaitForMessage<RefreshMsg>();

                    //Assert.Equal(5, refreshMsg.StreamId());
                    Assert.Equal(EmaRdm.MMT_MARKET_PRICE, refreshMsg.DomainType());
                    Assert.Equal("Open / Ok / None / 'Refresh Completed'", refreshMsg.State().ToString());
                    //Assert.True(refreshMsg.Complete());
                    //Assert.True(refreshMsg.Solicited());
                    Assert.True(refreshMsg.HasName);
                    //Assert.Equal(itemName, refreshMsg.Name());
                    Assert.True(refreshMsg.HasServiceId);
                    Assert.True(refreshMsg.HasServiceName);
                    Assert.Equal(serviceName, refreshMsg.ServiceName());
                    Assert.Equal(DataTypes.FIELD_LIST, refreshMsg.Payload().DataType);
                    channelInfo = consumerClient.PopChannelInfo();
                    Assert.Equal("Channel_9", channelInfo.ChannelName);
                    Assert.Equal("Connection_5", channelInfo.SessionChannelName);
                    Assert.Equal(ChannelState.ACTIVE, channelInfo.ChannelState);
                    Assert.NotNull(channelInfo.PreferredHostInfo);
                    Assert.Equal(7, channelInfo.PreferredHostInfo!.DetectionTimeInterval);
                    Assert.Equal("", channelInfo.PreferredHostInfo!.DetectionTimeSchedule);
                    Assert.Equal("Channel_9", channelInfo.PreferredHostInfo!.ChannelName);

                    refreshMsg.MarkForClear();
                }

                statusMsg = consumerClient.WaitForMessage<StatusMsg>().MarkForClear();
                Assert.Equal(1, statusMsg.StreamId());
                Assert.Equal(EmaRdm.MMT_LOGIN, statusMsg.DomainType());
                Assert.Equal("Open / Ok / PreferredHostStartingFallback / 'preferred host starting fallback'", statusMsg.State().ToString());
                Assert.True(statusMsg.HasMsgKey);
                Assert.Equal(DataTypes.NO_DATA, statusMsg.Payload().DataType);
                Assert.Equal(DataTypes.NO_DATA, statusMsg.Attrib().DataType);
                channelInfo = consumerClient.PopChannelInfo();
                Assert.Equal("Channel_4", channelInfo.ChannelName);
                Assert.Equal("Connection_6", channelInfo.SessionChannelName);
                Assert.Equal(ChannelState.ACTIVE, channelInfo.ChannelState);
                Assert.Equal(7, channelInfo.PreferredHostInfo!.DetectionTimeInterval);
                Assert.Equal("", channelInfo.PreferredHostInfo!.DetectionTimeSchedule);
                Assert.Equal("Channel_10", channelInfo.PreferredHostInfo!.ChannelName);

                statusMsg = consumerClient.WaitForMessage<StatusMsg>().MarkForClear();
                Assert.Equal(1, statusMsg.StreamId());
                Assert.Equal(EmaRdm.MMT_LOGIN, statusMsg.DomainType());
                Assert.Equal("Open / Ok / None / 'session channel down reconnecting'", statusMsg.State().ToString());
                Assert.True(statusMsg.HasMsgKey);
                Assert.Equal(DataTypes.NO_DATA, statusMsg.Payload().DataType);
                Assert.Equal(DataTypes.NO_DATA, statusMsg.Attrib().DataType);
                channelInfo = consumerClient.PopChannelInfo();
                Assert.Equal("Channel_4", channelInfo.ChannelName);
                Assert.Equal("Connection_6", channelInfo.SessionChannelName);
                Assert.Equal(ChannelState.INACTIVE, channelInfo.ChannelState);

                statusMsg = consumerClient.WaitForMessage<StatusMsg>().MarkForClear();

                Assert.Equal(1, statusMsg.StreamId());
                Assert.Equal(EmaRdm.MMT_LOGIN, statusMsg.DomainType());
                Assert.Equal("Open / Ok / PreferredHostComplete / 'preferred host complete'", statusMsg.State().ToString());
                Assert.True(statusMsg.HasMsgKey);
                Assert.Equal(DataTypes.NO_DATA, statusMsg.Payload().DataType);
                channelInfo = consumerClient.PopChannelInfo();
                Assert.Equal("Channel_10", channelInfo.ChannelName);
                Assert.Equal("Connection_6", channelInfo.SessionChannelName);
                Assert.Equal(ChannelState.ACTIVE, channelInfo.ChannelState);
                Assert.Equal(7, channelInfo.PreferredHostInfo!.DetectionTimeInterval);
                Assert.Equal("", channelInfo.PreferredHostInfo!.DetectionTimeSchedule);
                Assert.Equal("Channel_10", channelInfo.PreferredHostInfo!.ChannelName);

                refreshMsg = consumerClient.WaitForMessage<RefreshMsg>().MarkForClear();

                Assert.Equal(1, refreshMsg.StreamId());
                Assert.Equal(EmaRdm.MMT_LOGIN, refreshMsg.DomainType());
                Assert.Equal("Open / Ok / None / 'Login accepted'", refreshMsg.State().ToString());
                Assert.True(refreshMsg.Solicited());
                Assert.True(refreshMsg.Complete());
                Assert.True(refreshMsg.HasMsgKey);
                Assert.Equal(DataTypes.NO_DATA, refreshMsg.Payload().DataType);
                Assert.Equal(DataTypes.ELEMENT_LIST, refreshMsg.Attrib().DataType);
                channelInfo = consumerClient.PopChannelInfo();
                Assert.Equal("Channel_10", channelInfo.ChannelName);
                Assert.Equal("Connection_6", channelInfo.SessionChannelName);
                Assert.Equal(ChannelState.ACTIVE, channelInfo.ChannelState);
                Assert.Equal(7, channelInfo.PreferredHostInfo!.DetectionTimeInterval);
                Assert.Equal("", channelInfo.PreferredHostInfo!.DetectionTimeSchedule);
                Assert.Equal("Channel_10", channelInfo.PreferredHostInfo!.ChannelName);

                // Checks login status messages
                statusMsg = consumerClient.WaitForMessage<StatusMsg>().MarkForClear();

                Assert.Equal(1, statusMsg.StreamId());
                Assert.Equal(EmaRdm.MMT_LOGIN, statusMsg.DomainType());
                Assert.Equal("Open / Ok / None / 'session channel up'", statusMsg.State().ToString());
                Assert.True(statusMsg.HasMsgKey);
                Assert.Equal(DataTypes.NO_DATA, statusMsg.Payload().DataType);
                channelInfo = consumerClient.PopChannelInfo();
                Assert.Equal("Channel_10", channelInfo.ChannelName);
                Assert.Equal("Connection_6", channelInfo.SessionChannelName);
                Assert.Equal(ChannelState.ACTIVE, channelInfo.ChannelState);
                Assert.Equal(7, channelInfo.PreferredHostInfo!.DetectionTimeInterval);
                Assert.Equal("", channelInfo.PreferredHostInfo!.DetectionTimeSchedule);
                Assert.Equal("Channel_10", channelInfo.PreferredHostInfo!.ChannelName);

                Thread.Sleep(5000);

                // Checks for PH NO FALLBACK event

                /* Checks login status messages */
                statusMsg = consumerClient.WaitForMessage<StatusMsg>().MarkForClear();

                Assert.Equal(1, statusMsg.StreamId());
                Assert.Equal(EmaRdm.MMT_LOGIN, statusMsg.DomainType());
                Assert.Equal("Open / Ok / PreferredHostNoFallback / 'preferred host no fallback'", statusMsg.State().ToString());
                Assert.True(statusMsg.HasMsgKey);
                Assert.Equal(DataTypes.NO_DATA, statusMsg.Payload().DataType);
                channelInfo = consumerClient.PopChannelInfo();
                Assert.Equal("Channel_9", channelInfo.ChannelName);
                Assert.Equal("Connection_5", channelInfo.SessionChannelName);
                Assert.Equal(ChannelState.ACTIVE, channelInfo.ChannelState);
                Assert.Equal(7, channelInfo.PreferredHostInfo!.DetectionTimeInterval);
                Assert.Equal("", channelInfo.PreferredHostInfo!.DetectionTimeSchedule);
                Assert.Equal("Channel_9", channelInfo.PreferredHostInfo!.ChannelName);

                //Checks login status messages
                statusMsg = consumerClient.WaitForMessage<StatusMsg>().MarkForClear();

                Assert.Equal(1, statusMsg.StreamId());
                Assert.Equal(EmaRdm.MMT_LOGIN, statusMsg.DomainType());
                Assert.Equal("Open / Ok / PreferredHostNoFallback / 'preferred host no fallback'", statusMsg.State().ToString());
                Assert.True(statusMsg.HasMsgKey);
                Assert.Equal(DataTypes.NO_DATA, statusMsg.Payload().DataType);
                channelInfo = consumerClient.PopChannelInfo();
                Assert.Equal("Channel_10", channelInfo.ChannelName);
                Assert.Equal("Connection_6", channelInfo.SessionChannelName);
                Assert.Equal(ChannelState.ACTIVE, channelInfo.ChannelState);
                Assert.Equal(7, channelInfo.PreferredHostInfo!.DetectionTimeInterval);
                Assert.Equal("", channelInfo.PreferredHostInfo!.DetectionTimeSchedule);
                Assert.Equal("Channel_10", channelInfo.PreferredHostInfo!.ChannelName);

                // There should be any message and channel events at this time.
                Assert.Equal(0, consumerClient.QueueSize());

                consumer.Unregister(itemHandle);
            }
            finally
            {
                consumer?.Uninitialize();
                ommprovider?.Uninitialize();
                ommprovider2?.Uninitialize();
                ommprovider3?.Uninitialize();
                ommprovider4?.Uninitialize();
            }
        }
    }
}
