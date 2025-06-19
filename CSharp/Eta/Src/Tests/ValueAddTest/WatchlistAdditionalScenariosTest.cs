/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2023-2024 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

using System;
using System.Collections.Generic;

using Xunit;
using LSEG.Eta.Codec;
using LSEG.Eta.ValueAdd.Reactor;
using LSEG.Eta.Rdm;
using LSEG.Eta.ValueAdd.Rdm;
using Buffer = LSEG.Eta.Codec.Buffer;
using LSEG.Eta.Common;

namespace LSEG.Eta.ValuedAdd.Tests
{
    public class WatchlistAdditionalScenariosTest
    {
        private const long ALL_FILTERS = Directory.ServiceFilterFlags.INFO |
        Directory.ServiceFilterFlags.STATE |
        Directory.ServiceFilterFlags.GROUP |
        Directory.ServiceFilterFlags.DATA |
        Directory.ServiceFilterFlags.LINK |
        Directory.ServiceFilterFlags.LOAD;

        [Fact]
        public void WatchlistMiscBigPostMsgTest()
        {
            /* Tests watchlist internal handling of WRITE_CALL_AGAIN by sending a really big message. */

            ReactorSubmitOptions submitOptions = new();
            TestReactorEvent evt;
            ReactorMsgEvent msgEvent;
            Msg msg = new();
            IRequestMsg requestMsg = msg;
            IRequestMsg receivedRequestMsg;
            IRefreshMsg refreshMsg = msg;
            IRefreshMsg receivedRefreshMsg;
            int providerStreamId;
            IPostMsg postMsg = new Msg();

            /* Create reactors. */
            TestReactor consumerReactor = new();
            TestReactor providerReactor = new();

            /* Create consumer. */
            Consumer consumer = new(consumerReactor);
            ConsumerRole consumerRole = (ConsumerRole)consumer.ReactorRole;
            consumerRole.InitDefaultRDMLoginRequest();
            consumerRole.InitDefaultRDMDirectoryRequest();
            consumerRole.ChannelEventCallback = consumer;
            consumerRole.LoginMsgCallback = consumer;
            consumerRole.DirectoryMsgCallback = consumer;
            consumerRole.DictionaryMsgCallback = consumer;
            consumerRole.DefaultMsgCallback = consumer;
            consumerRole.WatchlistOptions.EnableWatchlist = true;
            consumerRole.WatchlistOptions.ChannelOpenEventCallback = consumer;
            consumerRole.WatchlistOptions.RequestTimeout = 3000;

            /* Create provider. */
            Provider provider = new(providerReactor);
            ProviderRole providerRole = (ProviderRole)provider.ReactorRole;
            providerRole.ChannelEventCallback = provider;
            providerRole.LoginMsgCallback = provider;
            providerRole.DirectoryMsgCallback = provider;
            providerRole.DictionaryMsgCallback = provider;
            providerRole.DefaultMsgCallback = provider;

            /* Connect the consumer and provider. Setup login & directory streams automatically. */
            ConsumerProviderSessionOptions opts = new();
            opts.SetupDefaultLoginStream = true;
            opts.SetupDefaultDirectoryStream = true;

            provider.Bind(opts);

            opts.SysSendBufSize = 3 * 1024 * 1024;
            TestReactor.OpenSession(consumer, provider, opts);

            /* Request first item. */
            requestMsg.Clear();
            requestMsg.MsgClass = MsgClasses.REQUEST;
            requestMsg.StreamId = 5;
            requestMsg.DomainType = (int)DomainType.MARKET_PRICE;
            requestMsg.MsgKey.ApplyHasName();
            requestMsg.MsgKey.Name.Data("TRI.N");
            requestMsg.ApplyStreaming();
            requestMsg.ApplyHasQos();
            requestMsg.Qos.Timeliness(QosTimeliness.REALTIME);
            requestMsg.Qos.Rate(QosRates.TICK_BY_TICK);

            submitOptions.Clear();
            submitOptions.ServiceName = Provider.DefaultService.Info.ServiceName.ToString();
            Assert.True(consumer.SubmitAndDispatch((Msg)requestMsg, submitOptions) >= ReactorReturnCode.SUCCESS);

            /* Provider receives request. */
            providerReactor.Dispatch(1);
            evt = providerReactor.PollEvent();
            Assert.Equal(TestReactorEventType.MSG, evt.EventType);
            msgEvent = (ReactorMsgEvent)evt.ReactorEvent;
            Assert.Equal(MsgClasses.REQUEST, msgEvent.Msg.MsgClass);

            receivedRequestMsg = (IRequestMsg)msgEvent.Msg;
            Assert.True(receivedRequestMsg.MsgKey.CheckHasServiceId());
            Assert.True(receivedRequestMsg.CheckStreaming());
            Assert.False(receivedRequestMsg.CheckNoRefresh());
            Assert.True(receivedRequestMsg.CheckHasPriority()); /* Provides the default priority*/
            Assert.Equal(1, receivedRequestMsg.Priority.PriorityClass);
            Assert.Equal(1, receivedRequestMsg.Priority.Count);
            Assert.Equal(Provider.DefaultService.ServiceId, receivedRequestMsg.MsgKey.ServiceId);
            Assert.True(receivedRequestMsg.MsgKey.CheckHasName());
            Assert.Equal("TRI.N", receivedRequestMsg.MsgKey.Name.ToString());
            Assert.Equal((int)DomainType.MARKET_PRICE, receivedRequestMsg.DomainType);
            Assert.True(receivedRequestMsg.CheckHasPriority());
            Assert.Equal(1, receivedRequestMsg.Priority.Count);

            providerStreamId = receivedRequestMsg.StreamId;

            /* Provider sends refresh .*/
            refreshMsg.Clear();
            refreshMsg.MsgClass = MsgClasses.REFRESH;
            refreshMsg.DomainType = (int)DomainType.MARKET_PRICE;
            refreshMsg.StreamId = providerStreamId;
            refreshMsg.ContainerType = DataTypes.NO_DATA;
            refreshMsg.ApplyHasMsgKey();
            refreshMsg.MsgKey.ApplyHasServiceId();
            refreshMsg.ApplyRefreshComplete();
            refreshMsg.ApplySolicited();
            refreshMsg.MsgKey.ServiceId = Provider.DefaultService.ServiceId;
            refreshMsg.MsgKey.ApplyHasName();
            refreshMsg.MsgKey.Name.Data("TRI.N");
            refreshMsg.State.StreamState(StreamStates.OPEN);
            refreshMsg.State.DataState(DataStates.OK);

            Assert.True(provider.SubmitAndDispatch((Msg)refreshMsg, submitOptions) >= ReactorReturnCode.SUCCESS);

            /* Consumer receives refresh. */
            consumerReactor.Dispatch(1);
            evt = consumerReactor.PollEvent();
            Assert.Equal(TestReactorEventType.MSG, evt.EventType);
            msgEvent = (ReactorMsgEvent)evt.ReactorEvent;
            Assert.Equal(MsgClasses.REFRESH, msgEvent.Msg.MsgClass);

            receivedRefreshMsg = (IRefreshMsg)msgEvent.Msg;
            Assert.True(receivedRefreshMsg.CheckHasMsgKey());
            Assert.True(receivedRefreshMsg.MsgKey.CheckHasServiceId());
            Assert.Equal(Provider.DefaultService.ServiceId, receivedRefreshMsg.MsgKey.ServiceId);
            Assert.True(receivedRefreshMsg.MsgKey.CheckHasName());
            Assert.Equal("TRI.N", receivedRefreshMsg.MsgKey.Name.ToString());
            Assert.Equal((int)DomainType.MARKET_PRICE, receivedRefreshMsg.DomainType);
            Assert.Equal(DataTypes.NO_DATA, receivedRefreshMsg.ContainerType);
            Assert.Equal(StreamStates.OPEN, receivedRefreshMsg.State.StreamState());
            Assert.Equal(DataStates.OK, receivedRefreshMsg.State.DataState());
            Assert.Equal(5, receivedRefreshMsg.StreamId);
            Assert.NotNull(msgEvent.StreamInfo);
            Assert.NotNull(msgEvent.StreamInfo.ServiceName);
            Assert.Equal(msgEvent.StreamInfo.ServiceName, Provider.DefaultService.Info.ServiceName.ToString());

            ReactorChannelInfo conChannelInfo = new();

            Assert.True(consumer.GetChannelInfo(conChannelInfo) == ReactorReturnCode.SUCCESS);

            int msgSize = (conChannelInfo.ChannelInfo.MaxFragmentSize *
                conChannelInfo.ChannelInfo.MaxOutputBuffers) * 8;

            ByteBuffer postByteBuffer = new(msgSize);
            Buffer dataBuffer = new();
            dataBuffer.Data(postByteBuffer);

            /* Sends a post message. */
            postMsg.Clear();
            postMsg.MsgClass = MsgClasses.POST;
            postMsg.StreamId = 5;
            postMsg.DomainType = (int)DomainType.MARKET_PRICE;
            postMsg.ContainerType = DataTypes.OPAQUE;
            postMsg.ApplyPostComplete();
            postMsg.ApplyHasMsgKey();
            postMsg.EncodedDataBody = dataBuffer;

            submitOptions.Clear();
            submitOptions.ServiceName = Provider.DefaultService.Info.ServiceName.ToString();

            Assert.True(consumer.SubmitAndDispatch((Msg)postMsg, submitOptions) >= ReactorReturnCode.SUCCESS);

            do
            {
                consumerReactor.Dispatch(-1);
                providerReactor.Dispatch(-1);

            } while (providerReactor.m_EventQueue.Count == 0);

            /* Provider receives post. */
            evt = providerReactor.PollEvent();
            Assert.Equal(TestReactorEventType.MSG, evt.EventType);
            msgEvent = (ReactorMsgEvent)evt.ReactorEvent;
            Assert.Equal(MsgClasses.POST, msgEvent.Msg.MsgClass);
            IPostMsg receivedPostMsg = (IPostMsg)msgEvent.Msg;
            Assert.True(receivedPostMsg.CheckHasMsgKey());
            Assert.True(receivedPostMsg.MsgKey.CheckHasServiceId());
            Assert.Equal(Provider.DefaultService.ServiceId, receivedPostMsg.MsgKey.ServiceId);
            Assert.Equal((int)DomainType.MARKET_PRICE, receivedPostMsg.DomainType);
            Assert.Equal(providerStreamId, receivedPostMsg.StreamId);
            Assert.Equal(msgSize, receivedPostMsg.EncodedDataBody.Length);

            TestReactorComponent.CloseSession(consumer, provider);
            consumerReactor.Close();
            providerReactor.Close();
        }

        [Fact]
        public void WatchlistMiscBigGenericMsgTest()
        {
            /* Tests watchlist internal handling of WRITE_CALL_AGAIN by sending a really big message. */

            ReactorSubmitOptions submitOptions = new();
            TestReactorEvent evt;
            ReactorMsgEvent msgEvent;
            Msg msg = new();
            IRequestMsg requestMsg = msg;
            IRequestMsg receivedRequestMsg;
            IRefreshMsg refreshMsg = msg;
            IRefreshMsg receivedRefreshMsg;
            int providerStreamId;
            IGenericMsg genericMsg = new Msg();

            /* Create reactors. */
            TestReactor consumerReactor = new();
            TestReactor providerReactor = new();

            /* Create consumer. */
            Consumer consumer = new(consumerReactor);
            ConsumerRole consumerRole = (ConsumerRole)consumer.ReactorRole;
            consumerRole.InitDefaultRDMLoginRequest();
            consumerRole.InitDefaultRDMDirectoryRequest();
            consumerRole.ChannelEventCallback = consumer;
            consumerRole.LoginMsgCallback = consumer;
            consumerRole.DirectoryMsgCallback = consumer;
            consumerRole.DictionaryMsgCallback = consumer;
            consumerRole.DefaultMsgCallback = consumer;
            consumerRole.WatchlistOptions.EnableWatchlist = true;
            consumerRole.WatchlistOptions.ChannelOpenEventCallback = consumer;
            consumerRole.WatchlistOptions.RequestTimeout = 3000;

            /* Create provider. */
            Provider provider = new(providerReactor);
            ProviderRole providerRole = (ProviderRole)provider.ReactorRole;
            providerRole.ChannelEventCallback = provider;
            providerRole.LoginMsgCallback = provider;
            providerRole.DirectoryMsgCallback = provider;
            providerRole.DictionaryMsgCallback = provider;
            providerRole.DefaultMsgCallback = provider;

            /* Connect the consumer and provider. Setup login & directory streams automatically. */
            ConsumerProviderSessionOptions opts = new();
            opts.SetupDefaultLoginStream = true;
            opts.SetupDefaultDirectoryStream = true;

            provider.Bind(opts);

            opts.SysSendBufSize = 3 * 1024 * 1024;
            TestReactor.OpenSession(consumer, provider, opts);

            /* Request first item. */
            requestMsg.Clear();
            requestMsg.MsgClass = MsgClasses.REQUEST;
            requestMsg.StreamId = 5;
            requestMsg.DomainType = (int)DomainType.MARKET_PRICE;
            requestMsg.MsgKey.ApplyHasName();
            requestMsg.MsgKey.Name.Data("TRI.N");
            requestMsg.ApplyStreaming();
            requestMsg.ApplyHasQos();
            requestMsg.Qos.Timeliness(QosTimeliness.REALTIME);
            requestMsg.Qos.Rate(QosRates.TICK_BY_TICK);

            submitOptions.Clear();
            submitOptions.ServiceName = Provider.DefaultService.Info.ServiceName.ToString();
            Assert.True(consumer.SubmitAndDispatch((Msg)requestMsg, submitOptions) >= ReactorReturnCode.SUCCESS);

            /* Provider receives request. */
            providerReactor.Dispatch(1);
            evt = providerReactor.PollEvent();
            Assert.Equal(TestReactorEventType.MSG, evt.EventType);
            msgEvent = (ReactorMsgEvent)evt.ReactorEvent;
            Assert.Equal(MsgClasses.REQUEST, msgEvent.Msg.MsgClass);

            receivedRequestMsg = (IRequestMsg)msgEvent.Msg;
            Assert.True(receivedRequestMsg.MsgKey.CheckHasServiceId());
            Assert.True(receivedRequestMsg.CheckStreaming());
            Assert.False(receivedRequestMsg.CheckNoRefresh());
            Assert.True(receivedRequestMsg.CheckHasPriority()); /* Provides the default priority*/
            Assert.Equal(1, receivedRequestMsg.Priority.PriorityClass);
            Assert.Equal(1, receivedRequestMsg.Priority.Count);
            Assert.Equal(Provider.DefaultService.ServiceId, receivedRequestMsg.MsgKey.ServiceId);
            Assert.True(receivedRequestMsg.MsgKey.CheckHasName());
            Assert.Equal("TRI.N", receivedRequestMsg.MsgKey.Name.ToString());
            Assert.Equal((int)DomainType.MARKET_PRICE, receivedRequestMsg.DomainType);
            Assert.True(receivedRequestMsg.CheckHasPriority());
            Assert.Equal(1, receivedRequestMsg.Priority.Count);

            providerStreamId = receivedRequestMsg.StreamId;

            /* Provider sends refresh .*/
            refreshMsg.Clear();
            refreshMsg.MsgClass = MsgClasses.REFRESH;
            refreshMsg.DomainType = (int)DomainType.MARKET_PRICE;
            refreshMsg.StreamId = providerStreamId;
            refreshMsg.ContainerType = DataTypes.NO_DATA;
            refreshMsg.ApplyHasMsgKey();
            refreshMsg.MsgKey.ApplyHasServiceId();
            refreshMsg.ApplyRefreshComplete();
            refreshMsg.ApplySolicited();
            refreshMsg.MsgKey.ServiceId = Provider.DefaultService.ServiceId;
            refreshMsg.MsgKey.ApplyHasName();
            refreshMsg.MsgKey.Name.Data("TRI.N");
            refreshMsg.State.StreamState(StreamStates.OPEN);
            refreshMsg.State.DataState(DataStates.OK);

            Assert.True(provider.SubmitAndDispatch((Msg)refreshMsg, submitOptions) >= ReactorReturnCode.SUCCESS);

            /* Consumer receives refresh. */
            consumerReactor.Dispatch(1);
            evt = consumerReactor.PollEvent();
            Assert.Equal(TestReactorEventType.MSG, evt.EventType);
            msgEvent = (ReactorMsgEvent)evt.ReactorEvent;
            Assert.Equal(MsgClasses.REFRESH, msgEvent.Msg.MsgClass);

            receivedRefreshMsg = (IRefreshMsg)msgEvent.Msg;
            Assert.True(receivedRefreshMsg.CheckHasMsgKey());
            Assert.True(receivedRefreshMsg.MsgKey.CheckHasServiceId());
            Assert.Equal(Provider.DefaultService.ServiceId, receivedRefreshMsg.MsgKey.ServiceId);
            Assert.True(receivedRefreshMsg.MsgKey.CheckHasName());
            Assert.Equal("TRI.N", receivedRefreshMsg.MsgKey.Name.ToString());
            Assert.Equal((int)DomainType.MARKET_PRICE, receivedRefreshMsg.DomainType);
            Assert.Equal(DataTypes.NO_DATA, receivedRefreshMsg.ContainerType);
            Assert.Equal(StreamStates.OPEN, receivedRefreshMsg.State.StreamState());
            Assert.Equal(DataStates.OK, receivedRefreshMsg.State.DataState());
            Assert.Equal(5, receivedRefreshMsg.StreamId);
            Assert.NotNull(msgEvent.StreamInfo);
            Assert.NotNull(msgEvent.StreamInfo.ServiceName);
            Assert.Equal(msgEvent.StreamInfo.ServiceName, Provider.DefaultService.Info.ServiceName.ToString());

            ReactorChannelInfo conChannelInfo = new();

            Assert.True(consumer.GetChannelInfo(conChannelInfo) == ReactorReturnCode.SUCCESS);

            int msgSize = (conChannelInfo.ChannelInfo.MaxFragmentSize *
                conChannelInfo.ChannelInfo.MaxOutputBuffers) * 8;

            ByteBuffer postByteBuffer = new(msgSize);
            Buffer dataBuffer = new();
            dataBuffer.Data(postByteBuffer);

            /* Sends a big generic message. */
            genericMsg.Clear();
            genericMsg.MsgClass = MsgClasses.GENERIC;
            genericMsg.StreamId = 5;
            genericMsg.DomainType = (int)DomainType.MARKET_PRICE;
            genericMsg.ContainerType = DataTypes.OPAQUE;
            genericMsg.ApplyMessageComplete();
            genericMsg.ApplyHasMsgKey();
            genericMsg.EncodedDataBody = dataBuffer;

            submitOptions.Clear();
            submitOptions.ServiceName = Provider.DefaultService.Info.ServiceName.ToString();

            Assert.True(consumer.SubmitAndDispatch((Msg)genericMsg, submitOptions) >= ReactorReturnCode.SUCCESS);

            do
            {
                consumerReactor.Dispatch(-1);
                providerReactor.Dispatch(-1);

            } while (providerReactor.m_EventQueue.Count == 0);

            /* Provider receives post. */
            evt = providerReactor.PollEvent();
            Assert.Equal(TestReactorEventType.MSG, evt.EventType);
            msgEvent = (ReactorMsgEvent)evt.ReactorEvent;
            Assert.Equal(MsgClasses.GENERIC, msgEvent.Msg.MsgClass);
            IGenericMsg receivedGenericMsg = (IGenericMsg)msgEvent.Msg;
            Assert.True(receivedGenericMsg.CheckMessageComplete());
            Assert.True(receivedGenericMsg.CheckHasMsgKey());
            Assert.True(receivedGenericMsg.MsgKey.CheckHasServiceId());
            Assert.Equal(Provider.DefaultService.ServiceId, receivedGenericMsg.MsgKey.ServiceId);
            Assert.Equal((int)DomainType.MARKET_PRICE, receivedGenericMsg.DomainType);
            Assert.Equal(providerStreamId, receivedGenericMsg.StreamId);
            Assert.Equal(msgSize, receivedGenericMsg.EncodedDataBody.Length);

            TestReactorComponent.CloseSession(consumer, provider);
            consumerReactor.Close();
            providerReactor.Close();
        }

        private void WatchlistTwoItemsPostWithAck(bool expectedTimeout)
        {
            ReactorSubmitOptions submitOptions = new();
            TestReactorEvent evt;
            ReactorMsgEvent msgEvent;
            Msg msg = new();
            IRequestMsg requestMsg = msg;
            IRequestMsg receivedRequestMsg;
            IRefreshMsg refreshMsg = msg;
            IRefreshMsg receivedRefreshMsg;
            int providerStreamId;
            IPostMsg postMsg = new Msg();

            /* Create reactors. */
            TestReactor consumerReactor = new();
            TestReactor providerReactor = new();

            /* Create consumer. */
            Consumer consumer = new(consumerReactor);
            ConsumerRole consumerRole = (ConsumerRole)consumer.ReactorRole;
            consumerRole.InitDefaultRDMLoginRequest();
            consumerRole.InitDefaultRDMDirectoryRequest();
            consumerRole.ChannelEventCallback = consumer;
            consumerRole.LoginMsgCallback = consumer;
            consumerRole.DirectoryMsgCallback = consumer;
            consumerRole.DictionaryMsgCallback = consumer;
            consumerRole.DefaultMsgCallback = consumer;
            consumerRole.WatchlistOptions.EnableWatchlist = true;
            consumerRole.WatchlistOptions.ChannelOpenEventCallback = consumer;
            consumerRole.WatchlistOptions.RequestTimeout = 3000;
            consumerRole.WatchlistOptions.PostAckTimeout = 2000;

            /* Create provider. */
            Provider provider = new(providerReactor);
            ProviderRole providerRole = (ProviderRole)provider.ReactorRole;
            providerRole.ChannelEventCallback = provider;
            providerRole.LoginMsgCallback = provider;
            providerRole.DirectoryMsgCallback = provider;
            providerRole.DictionaryMsgCallback = provider;
            providerRole.DefaultMsgCallback = provider;

            /* Connect the consumer and provider. Setup login & directory streams automatically. */
            ConsumerProviderSessionOptions opts = new();
            opts.SetupDefaultLoginStream = true;
            opts.SetupDefaultDirectoryStream = true;

            provider.Bind(opts);

            TestReactor.OpenSession(consumer, provider, opts);

            int[] streamIdList = { 5, 6 };

            /* Request first item. */
            requestMsg.Clear();
            requestMsg.StreamId = streamIdList[0];
            requestMsg.MsgClass = MsgClasses.REQUEST;
            requestMsg.DomainType = (int)DomainType.MARKET_PRICE;
            requestMsg.ApplyStreaming();
            requestMsg.ApplyHasQos();
            requestMsg.Qos.Timeliness(QosTimeliness.REALTIME);
            requestMsg.Qos.Rate(QosRates.TICK_BY_TICK);

            submitOptions.Clear();
            submitOptions.ServiceName = Provider.DefaultService.Info.ServiceName.ToString();
            submitOptions.RequestMsgOptions.UserSpecObj = "7777";
            Assert.True(consumer.SubmitAndDispatch((Msg)requestMsg, submitOptions) >= ReactorReturnCode.SUCCESS);

            /* Provider receives request. */
            providerReactor.Dispatch(1);
            evt = providerReactor.PollEvent();
            Assert.Equal(TestReactorEventType.MSG, evt.EventType);
            msgEvent = (ReactorMsgEvent)evt.ReactorEvent;
            Assert.Equal(MsgClasses.REQUEST, msgEvent.Msg.MsgClass);
            receivedRequestMsg = (IRequestMsg)msgEvent.Msg;
            Assert.True(receivedRequestMsg.CheckStreaming());
            Assert.True(receivedRequestMsg.CheckHasPriority());
            Assert.Equal(1, receivedRequestMsg.Priority.PriorityClass);
            Assert.Equal(1, receivedRequestMsg.Priority.Count);
            Assert.False(receivedRequestMsg.CheckNoRefresh());
            providerStreamId = receivedRequestMsg.StreamId;

            /* Request second item. */
            requestMsg.StreamId = streamIdList[1];
            submitOptions.Clear();
            submitOptions.ServiceName = Provider.DefaultService.Info.ServiceName.ToString();
            submitOptions.RequestMsgOptions.UserSpecObj = "8888";
            Assert.True(consumer.SubmitAndDispatch((Msg)requestMsg, submitOptions) >= ReactorReturnCode.SUCCESS);

            /* Provider receives request -- same stream, no refresh, updated priority. */
            providerReactor.Dispatch(1);
            evt = providerReactor.PollEvent();
            Assert.Equal(TestReactorEventType.MSG, evt.EventType);
            msgEvent = (ReactorMsgEvent)evt.ReactorEvent;
            Assert.Equal(MsgClasses.REQUEST, msgEvent.Msg.MsgClass);
            receivedRequestMsg = (IRequestMsg)msgEvent.Msg;
            Assert.True(receivedRequestMsg.CheckStreaming());
            Assert.True(receivedRequestMsg.CheckHasPriority());
            Assert.Equal(1, receivedRequestMsg.Priority.PriorityClass);
            Assert.Equal(2, receivedRequestMsg.Priority.Count);
            Assert.True(receivedRequestMsg.CheckNoRefresh());
            Assert.Equal(providerStreamId, receivedRequestMsg.StreamId);

            refreshMsg.Clear();
            refreshMsg.MsgClass = MsgClasses.REFRESH;
            refreshMsg.DomainType = (int)DomainType.MARKET_PRICE;
            refreshMsg.StreamId = providerStreamId;
            refreshMsg.ContainerType = DataTypes.NO_DATA;
            refreshMsg.ApplyRefreshComplete();
            refreshMsg.ApplySolicited();
            refreshMsg.ApplyClearCache();
            refreshMsg.State.StreamState(StreamStates.OPEN);
            refreshMsg.State.DataState(DataStates.OK);
            requestMsg.ApplyHasQos();
            refreshMsg.Qos.Timeliness(QosTimeliness.REALTIME);
            refreshMsg.Qos.Rate(QosRates.TICK_BY_TICK);

            submitOptions.Clear();
            Assert.True(provider.SubmitAndDispatch((Msg)refreshMsg, submitOptions) >= ReactorReturnCode.SUCCESS);

            /* Consumer receives refresh. */
            consumerReactor.Dispatch(2);
            evt = consumerReactor.PollEvent();
            Assert.Equal(TestReactorEventType.MSG, evt.EventType);
            msgEvent = (ReactorMsgEvent)evt.ReactorEvent;
            Assert.Equal(MsgClasses.REFRESH, msgEvent.Msg.MsgClass);
            receivedRefreshMsg = (IRefreshMsg)msgEvent.Msg;
            Assert.Equal((int)DomainType.MARKET_PRICE, receivedRefreshMsg.DomainType);
            Assert.Equal(DataTypes.NO_DATA, receivedRefreshMsg.ContainerType);
            Assert.Equal(StreamStates.OPEN, receivedRefreshMsg.State.StreamState());
            Assert.Equal(DataStates.OK, receivedRefreshMsg.State.DataState());
            Assert.Equal(streamIdList[0], receivedRefreshMsg.StreamId);

            evt = consumerReactor.PollEvent();
            Assert.Equal(TestReactorEventType.MSG, evt.EventType);
            msgEvent = (ReactorMsgEvent)evt.ReactorEvent;
            Assert.Equal(MsgClasses.REFRESH, msgEvent.Msg.MsgClass);
            receivedRefreshMsg = (IRefreshMsg)msgEvent.Msg;
            Assert.Equal((int)DomainType.MARKET_PRICE, receivedRefreshMsg.DomainType);
            Assert.Equal(DataTypes.NO_DATA, receivedRefreshMsg.ContainerType);
            Assert.Equal(StreamStates.OPEN, receivedRefreshMsg.State.StreamState());
            Assert.Equal(DataStates.OK, receivedRefreshMsg.State.DataState());
            Assert.Equal(streamIdList[1], receivedRefreshMsg.StreamId);

            /* Consumer sends incomplete post, with sequence number. */
            postMsg.Clear();
            postMsg.MsgClass = MsgClasses.POST;
            postMsg.StreamId = streamIdList[0];
            postMsg.DomainType = (int)DomainType.MARKET_PRICE;
            postMsg.ContainerType = DataTypes.NO_DATA;
            postMsg.ApplyHasPostId();
            postMsg.ApplyAck();
            postMsg.ApplyHasSeqNum();
            postMsg.PostId = 55;
            postMsg.SeqNum = 88;

            submitOptions.Clear();
            Assert.True(consumer.SubmitAndDispatch((Msg)postMsg, submitOptions) >= ReactorReturnCode.SUCCESS);

            /* Provider receives post. */
            providerReactor.Dispatch(1);
            evt = providerReactor.PollEvent();
            Assert.Equal(TestReactorEventType.MSG, evt.EventType);
            msgEvent = (ReactorMsgEvent)evt.ReactorEvent;
            Assert.Equal(MsgClasses.POST, msgEvent.Msg.MsgClass);
            IPostMsg receivedPostMsg = (IPostMsg)msgEvent.Msg;
            Assert.Equal((int)DomainType.MARKET_PRICE, receivedPostMsg.DomainType);
            Assert.Equal(DataTypes.NO_DATA, receivedPostMsg.ContainerType);
            Assert.False(receivedPostMsg.CheckPostComplete());
            Assert.True(receivedPostMsg.CheckHasPostId());
            Assert.True(receivedPostMsg.CheckHasSeqNum());
            Assert.False(receivedPostMsg.CheckHasMsgKey());
            Assert.Equal(55, receivedPostMsg.PostId);
            Assert.Equal(88, receivedPostMsg.SeqNum);
            Assert.Equal(providerStreamId, receivedPostMsg.StreamId);

            /* Provider sends acknowledgement. */
            IAckMsg ackMsg = new Msg();
            ackMsg.MsgClass = MsgClasses.ACK;
            ackMsg.StreamId = providerStreamId;
            ackMsg.DomainType = (int)DomainType.MARKET_PRICE;
            ackMsg.ContainerType = DataTypes.NO_DATA;
            ackMsg.ApplyHasSeqNum();
            ackMsg.AckId = postMsg.PostId;
            ackMsg.SeqNum = postMsg.SeqNum;

            submitOptions.Clear();
            Assert.True(provider.SubmitAndDispatch((Msg)ackMsg, submitOptions) >= ReactorReturnCode.SUCCESS);

            /* Consumer receives ack, only on the first stream. */
            consumerReactor.Dispatch(1);
            evt = consumerReactor.PollEvent();
            Assert.Equal(TestReactorEventType.MSG, evt.EventType);
            msgEvent = (ReactorMsgEvent)evt.ReactorEvent;
            Assert.Equal(MsgClasses.ACK, msgEvent.Msg.MsgClass);
            IAckMsg receivedAckMsg = (IAckMsg)msgEvent.Msg;
            Assert.Equal(streamIdList[0], receivedAckMsg.StreamId);
            Assert.Equal((int)DomainType.MARKET_PRICE, receivedAckMsg.DomainType);
            Assert.Equal(DataTypes.NO_DATA, receivedAckMsg.ContainerType);
            Assert.True(receivedAckMsg.CheckHasSeqNum());
            Assert.Equal(postMsg.PostId, receivedAckMsg.AckId);
            Assert.Equal(postMsg.SeqNum, receivedAckMsg.SeqNum);
            Assert.NotNull(msgEvent.StreamInfo.ServiceName);
            Assert.Equal(Provider.DefaultService.Info.ServiceName.ToString(), msgEvent.StreamInfo.ServiceName);
            Assert.Equal("7777", msgEvent.StreamInfo.UserSpec);

            /* Consumer sends complete post, with ack ID & sequence number. */
            postMsg.Clear();
            postMsg.MsgClass = MsgClasses.POST;
            postMsg.StreamId = streamIdList[0];
            postMsg.DomainType = (int)DomainType.MARKET_PRICE;
            postMsg.ContainerType = DataTypes.NO_DATA;
            postMsg.ApplyPostComplete();
            postMsg.ApplyHasPostId();
            postMsg.ApplyAck();
            postMsg.ApplyHasSeqNum();
            postMsg.PostId = 55;
            postMsg.SeqNum = 88;
            submitOptions.Clear();
            Assert.True(consumer.SubmitAndDispatch((Msg)postMsg, submitOptions) >= ReactorReturnCode.SUCCESS);

            /* Provider receives post. */
            providerReactor.Dispatch(1);
            evt = providerReactor.PollEvent();
            Assert.Equal(TestReactorEventType.MSG, evt.EventType);
            msgEvent = (ReactorMsgEvent)evt.ReactorEvent;
            Assert.Equal(MsgClasses.POST, msgEvent.Msg.MsgClass);
            receivedPostMsg = (IPostMsg)msgEvent.Msg;
            Assert.Equal((int)DomainType.MARKET_PRICE, receivedPostMsg.DomainType);
            Assert.Equal(DataTypes.NO_DATA, receivedPostMsg.ContainerType);
            Assert.True(receivedPostMsg.CheckPostComplete());
            Assert.True(receivedPostMsg.CheckHasPostId());
            Assert.True(receivedPostMsg.CheckHasSeqNum());
            Assert.False(receivedPostMsg.CheckHasMsgKey());
            Assert.Equal(55, receivedPostMsg.PostId);
            Assert.Equal(88, receivedPostMsg.SeqNum);
            Assert.Equal(providerStreamId, receivedPostMsg.StreamId);

            /* Provider sends acknowledgement. */
            ackMsg.Clear();
            ackMsg.MsgClass = MsgClasses.ACK;
            ackMsg.StreamId = providerStreamId;
            ackMsg.DomainType = (int)DomainType.MARKET_PRICE;
            ackMsg.ContainerType = DataTypes.NO_DATA;
            ackMsg.ApplyHasSeqNum();
            ackMsg.AckId = postMsg.PostId;
            ackMsg.SeqNum = postMsg.SeqNum;

            submitOptions.Clear();
            Assert.True(provider.SubmitAndDispatch((Msg)ackMsg, submitOptions) >= ReactorReturnCode.SUCCESS);

            /* Consumer receives ack, only on the first stream. */
            consumerReactor.Dispatch(1);
            evt = consumerReactor.PollEvent();
            Assert.Equal(TestReactorEventType.MSG, evt.EventType);
            msgEvent = (ReactorMsgEvent)evt.ReactorEvent;
            Assert.Equal(MsgClasses.ACK, msgEvent.Msg.MsgClass);
            receivedAckMsg = (IAckMsg)msgEvent.Msg;
            Assert.Equal(streamIdList[0], receivedAckMsg.StreamId);
            Assert.Equal((int)DomainType.MARKET_PRICE, receivedAckMsg.DomainType);
            Assert.Equal(DataTypes.NO_DATA, receivedAckMsg.ContainerType);
            Assert.True(receivedAckMsg.CheckHasSeqNum());
            Assert.Equal(postMsg.PostId, receivedAckMsg.AckId);
            Assert.Equal(postMsg.SeqNum, receivedAckMsg.SeqNum);
            Assert.NotNull(msgEvent.StreamInfo.ServiceName);
            Assert.Equal(Provider.DefaultService.Info.ServiceName.ToString(), msgEvent.StreamInfo.ServiceName);
            Assert.Equal("7777", msgEvent.StreamInfo.UserSpec);

            /* Consumer sends complete post, without sequence number. */
            postMsg.Clear();
            postMsg.MsgClass = MsgClasses.POST;
            postMsg.StreamId = streamIdList[0];
            postMsg.DomainType = (int)DomainType.MARKET_PRICE;
            postMsg.ContainerType = DataTypes.NO_DATA;
            postMsg.ApplyPostComplete();
            postMsg.ApplyHasPostId();
            postMsg.ApplyAck();
            postMsg.PostId = 55;
            submitOptions.Clear();
            Assert.True(consumer.SubmitAndDispatch((Msg)postMsg, submitOptions) >= ReactorReturnCode.SUCCESS);

            /* Provider receives post. */
            providerReactor.Dispatch(1);
            evt = providerReactor.PollEvent();
            Assert.Equal(TestReactorEventType.MSG, evt.EventType);
            msgEvent = (ReactorMsgEvent)evt.ReactorEvent;
            Assert.Equal(MsgClasses.POST, msgEvent.Msg.MsgClass);
            receivedPostMsg = (IPostMsg)msgEvent.Msg;
            Assert.Equal((int)DomainType.MARKET_PRICE, receivedPostMsg.DomainType);
            Assert.Equal(DataTypes.NO_DATA, receivedPostMsg.ContainerType);
            Assert.True(receivedPostMsg.CheckHasPostId());
            Assert.False(receivedPostMsg.CheckHasMsgKey());
            Assert.Equal(55, receivedPostMsg.PostId);
            Assert.Equal(providerStreamId, receivedPostMsg.StreamId);

            /* Provider sends acknowledgement. */
            ackMsg.Clear();
            ackMsg.MsgClass = MsgClasses.ACK;
            ackMsg.StreamId = providerStreamId;
            ackMsg.DomainType = (int)DomainType.MARKET_PRICE;
            ackMsg.ContainerType = DataTypes.NO_DATA;
            ackMsg.AckId = postMsg.PostId;

            submitOptions.Clear();
            Assert.True(provider.SubmitAndDispatch((Msg)ackMsg, submitOptions) >= ReactorReturnCode.SUCCESS);

            /* Consumer receives ack, only on the first stream. */
            consumerReactor.Dispatch(1);
            evt = consumerReactor.PollEvent();
            Assert.Equal(TestReactorEventType.MSG, evt.EventType);
            msgEvent = (ReactorMsgEvent)evt.ReactorEvent;
            Assert.Equal(MsgClasses.ACK, msgEvent.Msg.MsgClass);
            receivedAckMsg = (IAckMsg)msgEvent.Msg;
            Assert.Equal(streamIdList[0], receivedAckMsg.StreamId);
            Assert.Equal((int)DomainType.MARKET_PRICE, receivedAckMsg.DomainType);
            Assert.Equal(DataTypes.NO_DATA, receivedAckMsg.ContainerType);
            Assert.False(receivedAckMsg.CheckHasSeqNum());
            Assert.Equal(postMsg.PostId, receivedAckMsg.AckId);
            Assert.NotNull(msgEvent.StreamInfo.ServiceName);
            Assert.Equal(Provider.DefaultService.Info.ServiceName.ToString(), msgEvent.StreamInfo.ServiceName);
            Assert.Equal("7777", msgEvent.StreamInfo.UserSpec);

            /* Not expect to receive a post timeout */
            if (expectedTimeout == false)
            {
                /* Consumer sends post, with ack ID & sequence number on the first stream. */
                postMsg.Clear();
                postMsg.MsgClass = MsgClasses.POST;
                postMsg.StreamId = streamIdList[0];
                postMsg.DomainType = (int)DomainType.MARKET_PRICE;
                postMsg.ContainerType = DataTypes.NO_DATA;
                postMsg.ApplyPostComplete();
                postMsg.ApplyHasPostId();
                postMsg.ApplyAck();
                postMsg.ApplyHasSeqNum();
                postMsg.PostId = 55;
                postMsg.SeqNum = 88;

                submitOptions.Clear();
                Assert.True(consumer.SubmitAndDispatch((Msg)postMsg, submitOptions) >= ReactorReturnCode.SUCCESS);

                /* Provider receives post. */
                providerReactor.Dispatch(1);
                evt = providerReactor.PollEvent();
                Assert.Equal(TestReactorEventType.MSG, evt.EventType);
                msgEvent = (ReactorMsgEvent)evt.ReactorEvent;
                Assert.Equal(MsgClasses.POST, msgEvent.Msg.MsgClass);
                receivedPostMsg = (IPostMsg)msgEvent.Msg;
                Assert.Equal((int)DomainType.MARKET_PRICE, receivedPostMsg.DomainType);
                Assert.Equal(DataTypes.NO_DATA, receivedPostMsg.ContainerType);
                Assert.True(receivedPostMsg.CheckPostComplete());
                Assert.True(receivedPostMsg.CheckHasPostId());
                Assert.True(receivedPostMsg.CheckHasSeqNum());
                Assert.False(receivedPostMsg.CheckHasMsgKey());
                Assert.Equal(55, receivedPostMsg.PostId);
                Assert.Equal(88, receivedPostMsg.SeqNum);
                Assert.Equal(providerStreamId, receivedPostMsg.StreamId);

                /* Provider sends acknowledgement. */
                ackMsg.Clear();
                ackMsg.MsgClass = MsgClasses.ACK;
                ackMsg.StreamId = providerStreamId;
                ackMsg.DomainType = (int)DomainType.MARKET_PRICE;
                ackMsg.ContainerType = DataTypes.NO_DATA;
                ackMsg.ApplyHasSeqNum();
                ackMsg.AckId = postMsg.PostId;
                ackMsg.SeqNum = postMsg.SeqNum;

                submitOptions.Clear();
                Assert.True(provider.SubmitAndDispatch((Msg)ackMsg, submitOptions) >= ReactorReturnCode.SUCCESS);

                /* Consumer receives ack, only on the first stream. */
                consumerReactor.Dispatch(1);
                evt = consumerReactor.PollEvent();
                Assert.Equal(TestReactorEventType.MSG, evt.EventType);
                msgEvent = (ReactorMsgEvent)evt.ReactorEvent;
                Assert.Equal(MsgClasses.ACK, msgEvent.Msg.MsgClass);
                receivedAckMsg = (IAckMsg)msgEvent.Msg;
                Assert.Equal(streamIdList[0], receivedAckMsg.StreamId);
                Assert.Equal((int)DomainType.MARKET_PRICE, receivedAckMsg.DomainType);
                Assert.Equal(DataTypes.NO_DATA, receivedAckMsg.ContainerType);
                Assert.True(receivedAckMsg.CheckHasSeqNum());
                Assert.Equal(postMsg.PostId, receivedAckMsg.AckId);
                Assert.Equal(postMsg.SeqNum, receivedAckMsg.SeqNum);
                Assert.NotNull(msgEvent.StreamInfo.ServiceName);
                Assert.Equal(Provider.DefaultService.Info.ServiceName.ToString(), msgEvent.StreamInfo.ServiceName);
                Assert.Equal("7777", msgEvent.StreamInfo.UserSpec);

                /* Consumer receives no timeout nack. */
                consumerReactor.Dispatch(0, new TimeSpan(0, 0, 0, 0, 2500));
                Assert.True(0 == consumerReactor.m_EventQueue.Count);
            }
            else
            {
                /* Consumer sends complete post, without sequence number. */
                postMsg.Clear();
                postMsg.MsgClass = MsgClasses.POST;
                postMsg.StreamId = streamIdList[0];
                postMsg.DomainType = (int)DomainType.MARKET_PRICE;
                postMsg.ContainerType = DataTypes.NO_DATA;
                postMsg.ApplyPostComplete();
                postMsg.ApplyHasPostId();
                postMsg.ApplyAck();
                postMsg.PostId = 55;

                submitOptions.Clear();
                Assert.True(consumer.SubmitAndDispatch((Msg)postMsg, submitOptions) >= ReactorReturnCode.SUCCESS);

                /* Provider receives post(doesn't respond). */
                providerReactor.Dispatch(1);
                evt = providerReactor.PollEvent();
                Assert.Equal(TestReactorEventType.MSG, evt.EventType);
                msgEvent = (ReactorMsgEvent)evt.ReactorEvent;
                Assert.Equal(MsgClasses.POST, msgEvent.Msg.MsgClass);
                receivedPostMsg = (IPostMsg)msgEvent.Msg;
                Assert.Equal((int)DomainType.MARKET_PRICE, receivedPostMsg.DomainType);
                Assert.Equal(DataTypes.NO_DATA, receivedPostMsg.ContainerType);
                Assert.True(receivedPostMsg.CheckPostComplete());
                Assert.True(receivedPostMsg.CheckHasPostId());
                Assert.False(receivedPostMsg.CheckHasMsgKey());
                Assert.Equal(55, receivedPostMsg.PostId);
                Assert.Equal(providerStreamId, receivedPostMsg.StreamId);

                /* Expected to receive the timeout nack from the watchlist */
                consumerReactor.Dispatch(1, new TimeSpan(0, 0, 0, 0, 2500));

                /* Consumer receives ack, only on the first stream. */
                evt = consumerReactor.PollEvent();
                Assert.Equal(TestReactorEventType.MSG, evt.EventType);
                msgEvent = (ReactorMsgEvent)evt.ReactorEvent;
                Assert.Equal(MsgClasses.ACK, msgEvent.Msg.MsgClass);
                receivedAckMsg = (IAckMsg)msgEvent.Msg;
                Assert.Equal(streamIdList[0], receivedAckMsg.StreamId);
                Assert.Equal((int)DomainType.MARKET_PRICE, receivedAckMsg.DomainType);
                Assert.Equal(DataTypes.NO_DATA, receivedAckMsg.ContainerType);
                Assert.True(receivedAckMsg.CheckHasNakCode());
                Assert.True(receivedAckMsg.CheckHasText());
                Assert.Equal(postMsg.PostId, receivedAckMsg.AckId);
                Assert.Equal(NakCodes.NO_RESPONSE, receivedAckMsg.NakCode);
                Assert.Equal($"No Ack received for PostMsg with postId = {postMsg.PostId}",
                    receivedAckMsg.Text.ToString());
                Assert.NotNull(msgEvent.StreamInfo.ServiceName);
                Assert.Equal(Provider.DefaultService.Info.ServiceName.ToString(), msgEvent.StreamInfo.ServiceName);
                Assert.Equal("7777", msgEvent.StreamInfo.UserSpec);

                /* Consumer sends complete post, with ack ID & sequence number. */
                postMsg.Clear();
                postMsg.MsgClass = MsgClasses.POST;
                postMsg.StreamId = streamIdList[0];
                postMsg.DomainType = (int)DomainType.MARKET_PRICE;
                postMsg.ContainerType = DataTypes.NO_DATA;
                postMsg.ApplyPostComplete();
                postMsg.ApplyHasPostId();
                postMsg.ApplyHasSeqNum();
                postMsg.ApplyAck();
                postMsg.PostId = 55;
                postMsg.SeqNum = 99;

                submitOptions.Clear();
                Assert.True(consumer.SubmitAndDispatch((Msg)postMsg, submitOptions) >= ReactorReturnCode.SUCCESS);

                /* Provider receives post(doesn't respond). */
                providerReactor.Dispatch(1);
                evt = providerReactor.PollEvent();
                Assert.Equal(TestReactorEventType.MSG, evt.EventType);
                msgEvent = (ReactorMsgEvent)evt.ReactorEvent;
                Assert.Equal(MsgClasses.POST, msgEvent.Msg.MsgClass);
                receivedPostMsg = (IPostMsg)msgEvent.Msg;
                Assert.Equal((int)DomainType.MARKET_PRICE, receivedPostMsg.DomainType);
                Assert.Equal(DataTypes.NO_DATA, receivedPostMsg.ContainerType);
                Assert.True(receivedPostMsg.CheckPostComplete());
                Assert.True(receivedPostMsg.CheckHasPostId());
                Assert.True(receivedPostMsg.CheckHasSeqNum());
                Assert.False(receivedPostMsg.CheckHasMsgKey());
                Assert.Equal(55, receivedPostMsg.PostId);
                Assert.Equal(99, receivedPostMsg.SeqNum);
                Assert.Equal(providerStreamId, receivedPostMsg.StreamId);

                /* Expected to receive the timeout nack from the watchlist */
                consumerReactor.Dispatch(1, new TimeSpan(0, 0, 0, 0, 2500));

                /* Consumer receives ack, only on the first stream. */
                evt = consumerReactor.PollEvent();
                Assert.Equal(TestReactorEventType.MSG, evt.EventType);
                msgEvent = (ReactorMsgEvent)evt.ReactorEvent;
                Assert.Equal(MsgClasses.ACK, msgEvent.Msg.MsgClass);
                receivedAckMsg = (IAckMsg)msgEvent.Msg;
                Assert.Equal(streamIdList[0], receivedAckMsg.StreamId);
                Assert.Equal((int)DomainType.MARKET_PRICE, receivedAckMsg.DomainType);
                Assert.Equal(DataTypes.NO_DATA, receivedAckMsg.ContainerType);
                Assert.True(receivedAckMsg.CheckHasSeqNum());
                Assert.True(receivedAckMsg.CheckHasNakCode());
                Assert.True(receivedAckMsg.CheckHasText());
                Assert.Equal(postMsg.PostId, receivedAckMsg.AckId);
                Assert.Equal(postMsg.SeqNum, receivedAckMsg.SeqNum);
                Assert.Equal(NakCodes.NO_RESPONSE, receivedAckMsg.NakCode);
                Assert.Equal($"No Ack received for PostMsg with postId = {postMsg.PostId}",
                    receivedAckMsg.Text.ToString());
                Assert.NotNull(msgEvent.StreamInfo.ServiceName);
                Assert.Equal(Provider.DefaultService.Info.ServiceName.ToString(), msgEvent.StreamInfo.ServiceName);
                Assert.Equal("7777", msgEvent.StreamInfo.UserSpec);
            }

            /* Close first item */
            ICloseMsg closeMsg = new Msg();
            closeMsg.MsgClass = MsgClasses.CLOSE;
            closeMsg.StreamId = streamIdList[0];
            closeMsg.DomainType = (int)DomainType.MARKET_PRICE;

            submitOptions.Clear();
            Assert.True(consumer.SubmitAndDispatch((Msg)closeMsg, submitOptions) >= ReactorReturnCode.SUCCESS);

            /* Provider receives request -- same stream, no refresh, updated priority. */
            providerReactor.Dispatch(1);
            evt = providerReactor.PollEvent();
            Assert.Equal(TestReactorEventType.MSG, evt.EventType);
            msgEvent = (ReactorMsgEvent)evt.ReactorEvent;
            Assert.Equal(MsgClasses.REQUEST, msgEvent.Msg.MsgClass);
            receivedRequestMsg = (IRequestMsg)msgEvent.Msg;
            Assert.True(receivedRequestMsg.CheckStreaming());
            Assert.True(receivedRequestMsg.CheckHasPriority());
            Assert.Equal(1, receivedRequestMsg.Priority.PriorityClass);
            Assert.Equal(1, receivedRequestMsg.Priority.Count);
            Assert.True(receivedRequestMsg.CheckNoRefresh());
            Assert.Equal(providerStreamId, receivedRequestMsg.StreamId);

            /* Close second item. */
            closeMsg = new Msg();
            closeMsg.MsgClass = MsgClasses.CLOSE;
            closeMsg.StreamId = streamIdList[1];
            closeMsg.DomainType = (int)DomainType.MARKET_PRICE;

            submitOptions.Clear();
            Assert.True(consumer.SubmitAndDispatch((Msg)closeMsg, submitOptions) >= ReactorReturnCode.SUCCESS);

            /* Provider receives close. */
            providerReactor.Dispatch(1);
            evt = providerReactor.PollEvent();
            Assert.Equal(TestReactorEventType.MSG, evt.EventType);
            msgEvent = (ReactorMsgEvent)evt.ReactorEvent;
            Assert.Equal(MsgClasses.CLOSE, msgEvent.Msg.MsgClass);
            ICloseMsg receivedCloseMsg = (ICloseMsg)msgEvent.Msg;
            Assert.Equal(providerStreamId, receivedCloseMsg.StreamId);

            TestReactorComponent.CloseSession(consumer, provider);
            consumerReactor.Close();
            providerReactor.Close();
        }

        [Fact]
        public void WatchlistTwoItemsPostWithAckTest()
        {
            WatchlistTwoItemsPostWithAck(false);
        }

        [Fact]
        public void WatchlistTwoItemsPostWithAck_TimeoutTest()
        {
            WatchlistTwoItemsPostWithAck(true);
        }

        [Fact]
        public void WatchlistPostWithAck_OffStream_TimeoutTest()
        {
            ReactorSubmitOptions submitOptions = new();
            TestReactorEvent evt;
            ReactorMsgEvent msgEvent;
            Msg msg = new();
            IRequestMsg requestMsg = msg;
            IRefreshMsg refreshMsg = msg;
            IPostMsg postMsg = new Msg();
            IAckMsg ackMsg = new Msg();
            IAckMsg receivedAckMsg;
            IPostMsg receivedPostMsg;

            /* Create reactors. */
            TestReactor consumerReactor = new();
            TestReactor providerReactor = new();

            /* Create consumer. */
            Consumer consumer = new(consumerReactor);
            ConsumerRole consumerRole = (ConsumerRole)consumer.ReactorRole;
            consumerRole.InitDefaultRDMLoginRequest();
            consumerRole.InitDefaultRDMDirectoryRequest();
            consumerRole.ChannelEventCallback = consumer;
            consumerRole.LoginMsgCallback = consumer;
            consumerRole.DirectoryMsgCallback = consumer;
            consumerRole.DictionaryMsgCallback = consumer;
            consumerRole.DefaultMsgCallback = consumer;
            consumerRole.WatchlistOptions.EnableWatchlist = true;
            consumerRole.WatchlistOptions.ChannelOpenEventCallback = consumer;
            consumerRole.WatchlistOptions.PostAckTimeout = 2000;

            /* Create provider. */
            Provider provider = new(providerReactor);
            ProviderRole providerRole = (ProviderRole)provider.ReactorRole;
            providerRole.ChannelEventCallback = provider;
            providerRole.LoginMsgCallback = provider;
            providerRole.DirectoryMsgCallback = provider;
            providerRole.DictionaryMsgCallback = provider;
            providerRole.DefaultMsgCallback = provider;

            /* Connect the consumer and provider. Setup login & directory streams automatically. */
            ConsumerProviderSessionOptions opts = new();
            opts.SetupDefaultLoginStream = true;
            opts.SetupDefaultDirectoryStream = true;

            provider.Bind(opts);

            TestReactor.OpenSession(consumer, provider, opts);

            /* Consumer sends incomplete post, with sequence number. */
            postMsg.MsgClass = MsgClasses.POST;
            postMsg.StreamId = consumer.DefaultSessionLoginStreamId;
            postMsg.DomainType = (int)DomainType.MARKET_PRICE;
            postMsg.ContainerType = DataTypes.NO_DATA;
            postMsg.ApplyHasPostId();
            postMsg.ApplyAck();
            postMsg.ApplyHasSeqNum();
            postMsg.PostId = 55;
            postMsg.SeqNum = 88;

            /* Adds message key as it is needed for the off stream posting */
            postMsg.ApplyHasMsgKey();
            postMsg.MsgKey.ApplyHasName();
            Buffer itemName = new();
            itemName.Data("itemname");
            postMsg.MsgKey.Name = itemName;

            submitOptions.Clear();
            Assert.True(consumer.SubmitAndDispatch((Msg)postMsg, submitOptions) >= ReactorReturnCode.SUCCESS);

            /* Provider receives post (doesn't respond). */
            providerReactor.Dispatch(1);
            evt = providerReactor.PollEvent();
            Assert.Equal(TestReactorEventType.MSG, evt.EventType);
            msgEvent = (ReactorMsgEvent)evt.ReactorEvent;
            Assert.Equal(MsgClasses.POST, msgEvent.Msg.MsgClass);
            receivedPostMsg = (IPostMsg)msgEvent.Msg;
            Assert.Equal((int)DomainType.MARKET_PRICE, receivedPostMsg.DomainType);
            Assert.Equal(DataTypes.NO_DATA, receivedPostMsg.ContainerType);
            Assert.False(receivedPostMsg.CheckPostComplete());
            Assert.True(receivedPostMsg.CheckHasPostId());
            Assert.True(receivedPostMsg.CheckHasSeqNum());
            Assert.True(receivedPostMsg.CheckHasMsgKey());
            Assert.True(receivedPostMsg.MsgKey.CheckHasName());
            Assert.Equal(itemName.ToString(), receivedPostMsg.MsgKey.Name.ToString());
            Assert.Equal(postMsg.PostId, receivedPostMsg.PostId);
            Assert.Equal(postMsg.SeqNum, receivedPostMsg.SeqNum);
            Assert.Equal(provider.DefaultSessionLoginStreamId, receivedPostMsg.StreamId);

            /* Consumer receives nack. */
            consumerReactor.Dispatch(1, new TimeSpan(0, 0, 0, 0, 2500));
            evt = consumerReactor.PollEvent();
            Assert.Equal(TestReactorEventType.MSG, evt.EventType);
            msgEvent = (ReactorMsgEvent)evt.ReactorEvent;
            Assert.Equal(MsgClasses.ACK, msgEvent.Msg.MsgClass);
            receivedAckMsg = (IAckMsg)msgEvent.Msg;
            Assert.Equal(consumer.DefaultSessionLoginStreamId, receivedAckMsg.StreamId);
            Assert.Equal((int)DomainType.MARKET_PRICE, receivedAckMsg.DomainType);
            Assert.Equal(DataTypes.NO_DATA, receivedAckMsg.ContainerType);
            Assert.True(receivedAckMsg.CheckHasSeqNum());
            Assert.True(receivedAckMsg.CheckHasNakCode());
            Assert.True(receivedAckMsg.CheckHasText());
            Assert.Equal(postMsg.PostId, receivedAckMsg.AckId);
            Assert.Equal(postMsg.SeqNum, receivedAckMsg.SeqNum);
            Assert.Equal(NakCodes.NO_RESPONSE, receivedAckMsg.NakCode);
            Assert.Equal($"No Ack received for PostMsg with postId = {postMsg.PostId}",
                receivedAckMsg.Text.ToString());
            Assert.Null(msgEvent.StreamInfo.ServiceName);

            TestReactorComponent.CloseSession(consumer, provider);
            consumerReactor.Close();
            providerReactor.Close();
        }

        [Fact]
        public void WatchlistTwoItems_UnackedPostsTest()
        {
            ReactorSubmitOptions submitOptions = new();
            TestReactorEvent evt;
            ReactorMsgEvent msgEvent;
            Msg msg = new();
            IRequestMsg requestMsg = msg;
            IRefreshMsg refreshMsg = msg;
            IPostMsg postMsg = new Msg();
            IAckMsg ackMsg = new Msg();
            IPostMsg receivedPostMsg;

            /* Create reactors. */
            TestReactor consumerReactor = new();
            TestReactor providerReactor = new();

            /* Create consumer. */
            Consumer consumer = new(consumerReactor);
            ConsumerRole consumerRole = (ConsumerRole)consumer.ReactorRole;
            consumerRole.InitDefaultRDMLoginRequest();
            consumerRole.InitDefaultRDMDirectoryRequest();
            consumerRole.ChannelEventCallback = consumer;
            consumerRole.LoginMsgCallback = consumer;
            consumerRole.DirectoryMsgCallback = consumer;
            consumerRole.DictionaryMsgCallback = consumer;
            consumerRole.DefaultMsgCallback = consumer;
            consumerRole.WatchlistOptions.EnableWatchlist = true;
            consumerRole.WatchlistOptions.ChannelOpenEventCallback = consumer;
            consumerRole.WatchlistOptions.PostAckTimeout = 2000;

            /* Create provider. */
            Provider provider = new(providerReactor);
            ProviderRole providerRole = (ProviderRole)provider.ReactorRole;
            providerRole.ChannelEventCallback = provider;
            providerRole.LoginMsgCallback = provider;
            providerRole.DirectoryMsgCallback = provider;
            providerRole.DictionaryMsgCallback = provider;
            providerRole.DefaultMsgCallback = provider;

            /* Connect the consumer and provider. Setup login & directory streams automatically. */
            ConsumerProviderSessionOptions opts = new();
            opts.SetupDefaultLoginStream = true;
            opts.SetupDefaultDirectoryStream = true;

            provider.Bind(opts);

            TestReactor.OpenSession(consumer, provider, opts);

            /* Tests that unacknowledged post messages are properly cleaned up on stream close. */

            int consumerStreamId = 5;
            /* Request first item. */
            requestMsg.Clear();
            requestMsg.StreamId = consumerStreamId;
            requestMsg.MsgClass = MsgClasses.REQUEST;
            requestMsg.DomainType = (int)DomainType.MARKET_PRICE;
            requestMsg.ContainerType = DataTypes.NO_DATA;
            requestMsg.ApplyStreaming();
            requestMsg.ApplyHasQos();
            requestMsg.Qos.Timeliness(QosTimeliness.REALTIME);
            requestMsg.Qos.Rate(QosRates.TICK_BY_TICK);

            submitOptions.Clear();
            submitOptions.ServiceName = Provider.DefaultService.Info.ServiceName.ToString();
            submitOptions.RequestMsgOptions.UserSpecObj = "9999";
            Assert.True(consumer.SubmitAndDispatch((Msg)requestMsg, submitOptions) >= ReactorReturnCode.SUCCESS);

            /* Provider receives request. */
            providerReactor.Dispatch(1);
            evt = providerReactor.PollEvent();
            Assert.Equal(TestReactorEventType.MSG, evt.EventType);
            msgEvent = (ReactorMsgEvent)evt.ReactorEvent;
            Assert.Equal(MsgClasses.REQUEST, msgEvent.Msg.MsgClass);
            IRequestMsg receivedRequestMsg = (IRequestMsg)msgEvent.Msg;
            Assert.True(receivedRequestMsg.CheckStreaming());
            Assert.True(receivedRequestMsg.CheckHasPriority());
            Assert.Equal(1, receivedRequestMsg.Priority.PriorityClass);
            Assert.Equal(1, receivedRequestMsg.Priority.Count);
            Assert.False(receivedRequestMsg.CheckNoRefresh());
            int providerStreamId = receivedRequestMsg.StreamId;

            /* Provider sends refresh. */
            refreshMsg.Clear();
            refreshMsg.MsgClass = MsgClasses.REFRESH;
            refreshMsg.DomainType = (int)DomainType.MARKET_PRICE;
            refreshMsg.StreamId = providerStreamId;
            refreshMsg.ContainerType = DataTypes.NO_DATA;
            refreshMsg.ApplyRefreshComplete();
            refreshMsg.ApplySolicited();
            refreshMsg.ApplyClearCache();
            refreshMsg.State.StreamState(StreamStates.OPEN);
            refreshMsg.State.DataState(DataStates.OK);
            requestMsg.ApplyHasQos();
            refreshMsg.Qos.Timeliness(QosTimeliness.REALTIME);
            refreshMsg.Qos.Rate(QosRates.TICK_BY_TICK);

            submitOptions.Clear();
            Assert.True(provider.SubmitAndDispatch((Msg)refreshMsg, submitOptions) >= ReactorReturnCode.SUCCESS);

            /* Consumer receives refresh. */
            consumerReactor.Dispatch(1);
            evt = consumerReactor.PollEvent();
            Assert.Equal(TestReactorEventType.MSG, evt.EventType);
            msgEvent = (ReactorMsgEvent)evt.ReactorEvent;
            Assert.Equal(MsgClasses.REFRESH, msgEvent.Msg.MsgClass);
            IRefreshMsg receivedRefreshMsg = (IRefreshMsg)msgEvent.Msg;
            Assert.Equal((int)DomainType.MARKET_PRICE, receivedRefreshMsg.DomainType);
            Assert.Equal(DataTypes.NO_DATA, receivedRefreshMsg.ContainerType);
            Assert.Equal(StreamStates.OPEN, receivedRefreshMsg.State.StreamState());
            Assert.Equal(DataStates.OK, receivedRefreshMsg.State.DataState());
            Assert.Equal(consumerStreamId, receivedRefreshMsg.StreamId);

            /* Consumer sends incomplete post, with sequence number. */
            postMsg.Clear();
            postMsg.MsgClass = MsgClasses.POST;
            postMsg.StreamId = consumerStreamId;
            postMsg.DomainType = (int)DomainType.MARKET_PRICE;
            postMsg.ContainerType = DataTypes.NO_DATA;
            postMsg.ApplyHasPostId();
            postMsg.ApplyAck();
            postMsg.ApplyHasSeqNum();
            postMsg.PostId = 55;
            postMsg.SeqNum = 88;

            submitOptions.Clear();
            Assert.True(consumer.SubmitAndDispatch((Msg)postMsg, submitOptions) >= ReactorReturnCode.SUCCESS);

            /* Provider receives post. */
            providerReactor.Dispatch(1);
            evt = providerReactor.PollEvent();
            Assert.Equal(TestReactorEventType.MSG, evt.EventType);
            msgEvent = (ReactorMsgEvent)evt.ReactorEvent;
            Assert.Equal(MsgClasses.POST, msgEvent.Msg.MsgClass);
            receivedPostMsg = (IPostMsg)msgEvent.Msg;
            Assert.Equal((int)DomainType.MARKET_PRICE, receivedPostMsg.DomainType);
            Assert.Equal(DataTypes.NO_DATA, receivedPostMsg.ContainerType);
            Assert.False(receivedPostMsg.CheckPostComplete());
            Assert.True(receivedPostMsg.CheckHasPostId());
            Assert.True(receivedPostMsg.CheckHasSeqNum());
            Assert.False(receivedPostMsg.CheckHasMsgKey());
            Assert.Equal(55, receivedPostMsg.PostId);
            Assert.Equal(88, receivedPostMsg.SeqNum);
            Assert.Equal(providerStreamId, receivedPostMsg.StreamId);

            /* Consumer sends complete post, with ack ID & sequence number. */
            postMsg.Clear();
            postMsg.MsgClass = MsgClasses.POST;
            postMsg.StreamId = consumerStreamId;
            postMsg.DomainType = (int)DomainType.MARKET_PRICE;
            postMsg.ContainerType = DataTypes.NO_DATA;
            postMsg.ApplyPostComplete();
            postMsg.ApplyHasPostId();
            postMsg.ApplyAck();
            postMsg.ApplyHasSeqNum();
            postMsg.PostId = 55;
            postMsg.SeqNum = 89;

            submitOptions.Clear();
            Assert.True(consumer.SubmitAndDispatch((Msg)postMsg, submitOptions) >= ReactorReturnCode.SUCCESS);

            /* Provider receives post. */
            providerReactor.Dispatch(1);
            evt = providerReactor.PollEvent();
            Assert.Equal(TestReactorEventType.MSG, evt.EventType);
            msgEvent = (ReactorMsgEvent)evt.ReactorEvent;
            Assert.Equal(MsgClasses.POST, msgEvent.Msg.MsgClass);
            receivedPostMsg = (IPostMsg)msgEvent.Msg;
            Assert.Equal((int)DomainType.MARKET_PRICE, receivedPostMsg.DomainType);
            Assert.Equal(DataTypes.NO_DATA, receivedPostMsg.ContainerType);
            Assert.True(receivedPostMsg.CheckPostComplete());
            Assert.True(receivedPostMsg.CheckHasPostId());
            Assert.True(receivedPostMsg.CheckHasSeqNum());
            Assert.False(receivedPostMsg.CheckHasMsgKey());
            Assert.Equal(55, receivedPostMsg.PostId);
            Assert.Equal(89, receivedPostMsg.SeqNum);
            Assert.Equal(providerStreamId, receivedPostMsg.StreamId);

            /* Provider sends closed-recover. */
            IStatusMsg statusMsg = new Msg();
            statusMsg.MsgClass = MsgClasses.STATUS;
            statusMsg.StreamId = providerStreamId;
            statusMsg.DomainType = (int)DomainType.MARKET_PRICE;
            statusMsg.ContainerType = DataTypes.NO_DATA;
            statusMsg.ApplyHasState();
            statusMsg.State.StreamState(StreamStates.CLOSED_RECOVER);
            statusMsg.State.DataState(DataStates.SUSPECT);

            submitOptions.Clear();
            submitOptions.ServiceName = Provider.DefaultService.Info.ServiceName.ToString();
            Assert.True(provider.SubmitAndDispatch((Msg)statusMsg, submitOptions) >= ReactorReturnCode.SUCCESS);

            /* Consumer receives Open/Suspect status. */
            consumerReactor.Dispatch(1);
            evt = consumerReactor.PollEvent();
            Assert.Equal(TestReactorEventType.MSG, evt.EventType);
            msgEvent = (ReactorMsgEvent)evt.ReactorEvent;
            Assert.Equal(MsgClasses.STATUS, msgEvent.Msg.MsgClass);
            IStatusMsg receivedStatusMsg = (IStatusMsg)msgEvent.Msg;
            Assert.Equal((int)DomainType.MARKET_PRICE, receivedStatusMsg.DomainType);
            Assert.True(receivedStatusMsg.CheckHasState());
            Assert.Equal(DataTypes.NO_DATA, receivedStatusMsg.ContainerType);
            Assert.Equal(StreamStates.OPEN, receivedStatusMsg.State.StreamState());
            Assert.Equal(DataStates.SUSPECT, receivedStatusMsg.State.DataState());
            Assert.Equal(consumerStreamId, receivedStatusMsg.StreamId);

            /* Ensure that Consumer receives no timeout nack on stream close. */
            consumerReactor.Dispatch(0, new TimeSpan(0, 0, 0, 0, 2500));
            Assert.True(0 == consumerReactor.m_EventQueue.Count);

            /* Watchlist send a request message to recover the stream. */
            providerReactor.Dispatch(1);
            evt = providerReactor.PollEvent();
            Assert.Equal(TestReactorEventType.MSG, evt.EventType);
            msgEvent = (ReactorMsgEvent)evt.ReactorEvent;
            Assert.Equal(MsgClasses.REQUEST, msgEvent.Msg.MsgClass);
            receivedRequestMsg = (IRequestMsg)msgEvent.Msg;
            Assert.True(receivedRequestMsg.CheckStreaming());
            Assert.True(receivedRequestMsg.CheckHasPriority());
            Assert.Equal(1, receivedRequestMsg.Priority.PriorityClass);
            Assert.Equal(1, receivedRequestMsg.Priority.Count);
            Assert.False(receivedRequestMsg.CheckNoRefresh());

            TestReactorComponent.CloseSession(consumer, provider);
            consumerReactor.Close();
            providerReactor.Close();
        }

        [Fact]
        public void WatchlistTwoItems_UnackedPosts_SamePostIdSeqNum_Test()
        {
            ReactorSubmitOptions submitOptions = new();
            TestReactorEvent evt;
            ReactorMsgEvent msgEvent;
            Msg msg = new();
            IRequestMsg requestMsg = msg;
            IRefreshMsg refreshMsg = msg;
            IPostMsg postMsg = new Msg();
            IAckMsg ackMsg = new Msg();
            IPostMsg receivedPostMsg;

            /* Create reactors. */
            TestReactor consumerReactor = new();
            TestReactor providerReactor = new();

            /* Create consumer. */
            Consumer consumer = new(consumerReactor);
            ConsumerRole consumerRole = (ConsumerRole)consumer.ReactorRole;
            consumerRole.InitDefaultRDMLoginRequest();
            consumerRole.InitDefaultRDMDirectoryRequest();
            consumerRole.ChannelEventCallback = consumer;
            consumerRole.LoginMsgCallback = consumer;
            consumerRole.DirectoryMsgCallback = consumer;
            consumerRole.DictionaryMsgCallback = consumer;
            consumerRole.DefaultMsgCallback = consumer;
            consumerRole.WatchlistOptions.EnableWatchlist = true;
            consumerRole.WatchlistOptions.ChannelOpenEventCallback = consumer;
            consumerRole.WatchlistOptions.PostAckTimeout = 2000;

            /* Create provider. */
            Provider provider = new(providerReactor);
            ProviderRole providerRole = (ProviderRole)provider.ReactorRole;
            providerRole.ChannelEventCallback = provider;
            providerRole.LoginMsgCallback = provider;
            providerRole.DirectoryMsgCallback = provider;
            providerRole.DictionaryMsgCallback = provider;
            providerRole.DefaultMsgCallback = provider;

            /* Connect the consumer and provider. Setup login & directory streams automatically. */
            ConsumerProviderSessionOptions opts = new();
            opts.SetupDefaultLoginStream = true;
            opts.SetupDefaultDirectoryStream = true;

            provider.Bind(opts);

            TestReactor.OpenSession(consumer, provider, opts);

            /* Tests that unacknowledged post messages are properly cleaned up on stream close even though they had same postId, seqNum. */

            int consumerStreamId = 5;
            /* Request first item. */
            requestMsg.Clear();
            requestMsg.StreamId = consumerStreamId;
            requestMsg.MsgClass = MsgClasses.REQUEST;
            requestMsg.DomainType = (int)DomainType.MARKET_PRICE;
            requestMsg.ContainerType = DataTypes.NO_DATA;
            requestMsg.ApplyStreaming();
            requestMsg.ApplyHasQos();
            requestMsg.Qos.Timeliness(QosTimeliness.REALTIME);
            requestMsg.Qos.Rate(QosRates.TICK_BY_TICK);

            submitOptions.Clear();
            submitOptions.ServiceName = Provider.DefaultService.Info.ServiceName.ToString();
            submitOptions.RequestMsgOptions.UserSpecObj = "9999";
            Assert.True(consumer.SubmitAndDispatch((Msg)requestMsg, submitOptions) >= ReactorReturnCode.SUCCESS);

            /* Provider receives request. */
            providerReactor.Dispatch(1);
            evt = providerReactor.PollEvent();
            Assert.Equal(TestReactorEventType.MSG, evt.EventType);
            msgEvent = (ReactorMsgEvent)evt.ReactorEvent;
            Assert.Equal(MsgClasses.REQUEST, msgEvent.Msg.MsgClass);
            IRequestMsg receivedRequestMsg = (IRequestMsg)msgEvent.Msg;
            Assert.True(receivedRequestMsg.CheckStreaming());
            Assert.True(receivedRequestMsg.CheckHasPriority());
            Assert.Equal(1, receivedRequestMsg.Priority.PriorityClass);
            Assert.Equal(1, receivedRequestMsg.Priority.Count);
            Assert.False(receivedRequestMsg.CheckNoRefresh());
            int providerStreamId = receivedRequestMsg.StreamId;

            /* Provider sends refresh. */
            refreshMsg.Clear();
            refreshMsg.MsgClass = MsgClasses.REFRESH;
            refreshMsg.DomainType = (int)DomainType.MARKET_PRICE;
            refreshMsg.StreamId = providerStreamId;
            refreshMsg.ContainerType = DataTypes.NO_DATA;
            refreshMsg.ApplyRefreshComplete();
            refreshMsg.ApplySolicited();
            refreshMsg.ApplyClearCache();
            refreshMsg.State.StreamState(StreamStates.OPEN);
            refreshMsg.State.DataState(DataStates.OK);
            requestMsg.ApplyHasQos();
            refreshMsg.Qos.Timeliness(QosTimeliness.REALTIME);
            refreshMsg.Qos.Rate(QosRates.TICK_BY_TICK);

            submitOptions.Clear();
            Assert.True(provider.SubmitAndDispatch((Msg)refreshMsg, submitOptions) >= ReactorReturnCode.SUCCESS);

            /* Consumer receives refresh. */
            consumerReactor.Dispatch(1);
            evt = consumerReactor.PollEvent();
            Assert.Equal(TestReactorEventType.MSG, evt.EventType);
            msgEvent = (ReactorMsgEvent)evt.ReactorEvent;
            Assert.Equal(MsgClasses.REFRESH, msgEvent.Msg.MsgClass);
            IRefreshMsg receivedRefreshMsg = (IRefreshMsg)msgEvent.Msg;
            Assert.Equal((int)DomainType.MARKET_PRICE, receivedRefreshMsg.DomainType);
            Assert.Equal(DataTypes.NO_DATA, receivedRefreshMsg.ContainerType);
            Assert.Equal(StreamStates.OPEN, receivedRefreshMsg.State.StreamState());
            Assert.Equal(DataStates.OK, receivedRefreshMsg.State.DataState());
            Assert.Equal(consumerStreamId, receivedRefreshMsg.StreamId);

            /* Consumer sends incomplete post, with sequence number. */
            postMsg.Clear();
            postMsg.MsgClass = MsgClasses.POST;
            postMsg.StreamId = consumerStreamId;
            postMsg.DomainType = (int)DomainType.MARKET_PRICE;
            postMsg.ContainerType = DataTypes.NO_DATA;
            postMsg.ApplyHasPostId();
            postMsg.ApplyAck();
            postMsg.ApplyHasSeqNum();
            postMsg.PostId = 55;
            postMsg.SeqNum = 88;

            submitOptions.Clear();
            Assert.True(consumer.SubmitAndDispatch((Msg)postMsg, submitOptions) >= ReactorReturnCode.SUCCESS);

            /* Provider receives post. */
            providerReactor.Dispatch(1);
            evt = providerReactor.PollEvent();
            Assert.Equal(TestReactorEventType.MSG, evt.EventType);
            msgEvent = (ReactorMsgEvent)evt.ReactorEvent;
            Assert.Equal(MsgClasses.POST, msgEvent.Msg.MsgClass);
            receivedPostMsg = (IPostMsg)msgEvent.Msg;
            Assert.Equal((int)DomainType.MARKET_PRICE, receivedPostMsg.DomainType);
            Assert.Equal(DataTypes.NO_DATA, receivedPostMsg.ContainerType);
            Assert.False(receivedPostMsg.CheckPostComplete());
            Assert.True(receivedPostMsg.CheckHasPostId());
            Assert.True(receivedPostMsg.CheckHasSeqNum());
            Assert.False(receivedPostMsg.CheckHasMsgKey());
            Assert.Equal(55, receivedPostMsg.PostId);
            Assert.Equal(88, receivedPostMsg.SeqNum);
            Assert.Equal(providerStreamId, receivedPostMsg.StreamId);

            /* Consumer sends complete post, with ack ID & sequence number. */
            postMsg.Clear();
            postMsg.MsgClass = MsgClasses.POST;
            postMsg.StreamId = consumerStreamId;
            postMsg.DomainType = (int)DomainType.MARKET_PRICE;
            postMsg.ContainerType = DataTypes.NO_DATA;
            postMsg.ApplyPostComplete();
            postMsg.ApplyHasPostId();
            postMsg.ApplyAck();
            postMsg.ApplyHasSeqNum();
            postMsg.PostId = 55;
            postMsg.SeqNum = 88;

            submitOptions.Clear();
            Assert.True(consumer.SubmitAndDispatch((Msg)postMsg, submitOptions) >= ReactorReturnCode.SUCCESS);

            /* Provider receives post. */
            providerReactor.Dispatch(1);
            evt = providerReactor.PollEvent();
            Assert.Equal(TestReactorEventType.MSG, evt.EventType);
            msgEvent = (ReactorMsgEvent)evt.ReactorEvent;
            Assert.Equal(MsgClasses.POST, msgEvent.Msg.MsgClass);
            receivedPostMsg = (IPostMsg)msgEvent.Msg;
            Assert.Equal((int)DomainType.MARKET_PRICE, receivedPostMsg.DomainType);
            Assert.Equal(DataTypes.NO_DATA, receivedPostMsg.ContainerType);
            Assert.True(receivedPostMsg.CheckPostComplete());
            Assert.True(receivedPostMsg.CheckHasPostId());
            Assert.True(receivedPostMsg.CheckHasSeqNum());
            Assert.False(receivedPostMsg.CheckHasMsgKey());
            Assert.Equal(55, receivedPostMsg.PostId);
            Assert.Equal(88, receivedPostMsg.SeqNum);
            Assert.Equal(providerStreamId, receivedPostMsg.StreamId);

            /* Provider sends closed-recover. */
            IStatusMsg statusMsg = new Msg();
            statusMsg.MsgClass = MsgClasses.STATUS;
            statusMsg.StreamId = providerStreamId;
            statusMsg.DomainType = (int)DomainType.MARKET_PRICE;
            statusMsg.ContainerType = DataTypes.NO_DATA;
            statusMsg.ApplyHasState();
            statusMsg.State.StreamState(StreamStates.CLOSED_RECOVER);
            statusMsg.State.DataState(DataStates.SUSPECT);

            submitOptions.Clear();
            submitOptions.ServiceName = Provider.DefaultService.Info.ServiceName.ToString();
            Assert.True(provider.SubmitAndDispatch((Msg)statusMsg, submitOptions) >= ReactorReturnCode.SUCCESS);

            /* Consumer receives Open/Suspect status. */
            consumerReactor.Dispatch(1);
            evt = consumerReactor.PollEvent();
            Assert.Equal(TestReactorEventType.MSG, evt.EventType);
            msgEvent = (ReactorMsgEvent)evt.ReactorEvent;
            Assert.Equal(MsgClasses.STATUS, msgEvent.Msg.MsgClass);
            IStatusMsg receivedStatusMsg = (IStatusMsg)msgEvent.Msg;
            Assert.Equal((int)DomainType.MARKET_PRICE, receivedStatusMsg.DomainType);
            Assert.True(receivedStatusMsg.CheckHasState());
            Assert.Equal(DataTypes.NO_DATA, receivedStatusMsg.ContainerType);
            Assert.Equal(StreamStates.OPEN, receivedStatusMsg.State.StreamState());
            Assert.Equal(DataStates.SUSPECT, receivedStatusMsg.State.DataState());
            Assert.Equal(consumerStreamId, receivedStatusMsg.StreamId);

            /* Ensure that Consumer receives no timeout nack on stream close. */
            consumerReactor.Dispatch(0, new TimeSpan(0, 0, 0, 0, 2500));
            Assert.True(0 == consumerReactor.m_EventQueue.Count);

            /* Watchlist send a request message to recover the stream. */
            providerReactor.Dispatch(1);
            evt = providerReactor.PollEvent();
            Assert.Equal(TestReactorEventType.MSG, evt.EventType);
            msgEvent = (ReactorMsgEvent)evt.ReactorEvent;
            Assert.Equal(MsgClasses.REQUEST, msgEvent.Msg.MsgClass);
            receivedRequestMsg = (IRequestMsg)msgEvent.Msg;
            Assert.True(receivedRequestMsg.CheckStreaming());
            Assert.True(receivedRequestMsg.CheckHasPriority());
            Assert.Equal(1, receivedRequestMsg.Priority.PriorityClass);
            Assert.Equal(1, receivedRequestMsg.Priority.Count);
            Assert.False(receivedRequestMsg.CheckNoRefresh());

            TestReactorComponent.CloseSession(consumer, provider);
            consumerReactor.Close();
            providerReactor.Close();
        }

        [Fact]
        public void WatchlistTwoItems_GenericMessageTest()
        {
            ReactorSubmitOptions submitOptions = new();
            TestReactorEvent evt;
            ReactorMsgEvent msgEvent;
            Msg msg = new();
            IRequestMsg requestMsg = msg;
            IRefreshMsg refreshMsg = msg;
            IGenericMsg genericMsg = msg;
            IGenericMsg receivedGenericMsg;

            /* Connect the consumer and provider. Setup login & directory streams automatically. */
            ConsumerProviderSessionOptions opts = new();
            opts.SetupDefaultLoginStream = true;
            opts.SetupDefaultDirectoryStream = true;

            SetupSession(opts, false, 0, false, 0, out var consumerReactor, out var providerReactor, out var consumer, out var provider);

            /* Try sending a generic message in each direction. */

            int[] consStreamIds = { 5, 6 };
            /* Request first item. */
            requestMsg.Clear();
            requestMsg.StreamId = consStreamIds[0];
            requestMsg.MsgClass = MsgClasses.REQUEST;
            requestMsg.DomainType = (int)DomainType.MARKET_PRICE;
            requestMsg.ContainerType = DataTypes.NO_DATA;
            requestMsg.ApplyStreaming();
            requestMsg.ApplyHasQos();
            requestMsg.Qos.Timeliness(QosTimeliness.REALTIME);
            requestMsg.Qos.Rate(QosRates.TICK_BY_TICK);

            submitOptions.Clear();
            submitOptions.ServiceName = Provider.DefaultService.Info.ServiceName.ToString();
            submitOptions.RequestMsgOptions.UserSpecObj = "9999";
            Assert.True(consumer.SubmitAndDispatch((Msg)requestMsg, submitOptions) >= ReactorReturnCode.SUCCESS);

            /* Provider receives request. */
            providerReactor.Dispatch(1);
            evt = providerReactor.PollEvent();
            Assert.Equal(TestReactorEventType.MSG, evt.EventType);
            msgEvent = (ReactorMsgEvent)evt.ReactorEvent;
            Assert.Equal(MsgClasses.REQUEST, msgEvent.Msg.MsgClass);
            IRequestMsg receivedRequestMsg = (IRequestMsg)msgEvent.Msg;
            Assert.True(receivedRequestMsg.CheckStreaming());
            Assert.True(receivedRequestMsg.CheckHasPriority());
            Assert.Equal(1, receivedRequestMsg.Priority.PriorityClass);
            Assert.Equal(1, receivedRequestMsg.Priority.Count);
            Assert.False(receivedRequestMsg.CheckNoRefresh());
            int providerStreamId = receivedRequestMsg.StreamId;

            /* Request second item. */
            requestMsg.StreamId = consStreamIds[1];

            submitOptions.Clear();
            submitOptions.ServiceName = Provider.DefaultService.Info.ServiceName.ToString();
            submitOptions.RequestMsgOptions.UserSpecObj = "5555";
            Assert.True(consumer.SubmitAndDispatch((Msg)requestMsg, submitOptions) >= ReactorReturnCode.SUCCESS);

            /* Provider receives request -- same stream, no refresh, updated priority. */
            providerReactor.Dispatch(1);
            evt = providerReactor.PollEvent();
            Assert.Equal(TestReactorEventType.MSG, evt.EventType);
            msgEvent = (ReactorMsgEvent)evt.ReactorEvent;
            Assert.Equal(MsgClasses.REQUEST, msgEvent.Msg.MsgClass);
            receivedRequestMsg = (IRequestMsg)msgEvent.Msg;
            Assert.True(receivedRequestMsg.CheckStreaming());
            Assert.True(receivedRequestMsg.CheckHasPriority());
            Assert.Equal(1, receivedRequestMsg.Priority.PriorityClass);
            Assert.Equal(2, receivedRequestMsg.Priority.Count);
            Assert.True(receivedRequestMsg.CheckNoRefresh());
            Assert.Equal(providerStreamId, receivedRequestMsg.StreamId);

            /* Provider sends refresh. */
            refreshMsg.Clear();
            refreshMsg.MsgClass = MsgClasses.REFRESH;
            refreshMsg.DomainType = (int)DomainType.MARKET_PRICE;
            refreshMsg.StreamId = providerStreamId;
            refreshMsg.ContainerType = DataTypes.NO_DATA;
            refreshMsg.ApplyRefreshComplete();
            refreshMsg.ApplySolicited();
            refreshMsg.ApplyClearCache();
            refreshMsg.State.StreamState(StreamStates.OPEN);
            refreshMsg.State.DataState(DataStates.OK);
            requestMsg.ApplyHasQos();
            refreshMsg.Qos.Timeliness(QosTimeliness.REALTIME);
            refreshMsg.Qos.Rate(QosRates.TICK_BY_TICK);

            submitOptions.Clear();
            Assert.True(provider.SubmitAndDispatch((Msg)refreshMsg, submitOptions) >= ReactorReturnCode.SUCCESS);

            /* Consumer receives refresh. */
            consumerReactor.Dispatch(2);
            evt = consumerReactor.PollEvent();
            Assert.Equal(TestReactorEventType.MSG, evt.EventType);
            msgEvent = (ReactorMsgEvent)evt.ReactorEvent;
            Assert.Equal(MsgClasses.REFRESH, msgEvent.Msg.MsgClass);
            IRefreshMsg receivedRefreshMsg = (IRefreshMsg)msgEvent.Msg;
            Assert.Equal((int)DomainType.MARKET_PRICE, receivedRefreshMsg.DomainType);
            Assert.Equal(DataTypes.NO_DATA, receivedRefreshMsg.ContainerType);
            Assert.Equal(StreamStates.OPEN, receivedRefreshMsg.State.StreamState());
            Assert.Equal(DataStates.OK, receivedRefreshMsg.State.DataState());
            Assert.Equal(consStreamIds[0], receivedRefreshMsg.StreamId);

            evt = consumerReactor.PollEvent();
            Assert.Equal(TestReactorEventType.MSG, evt.EventType);
            msgEvent = (ReactorMsgEvent)evt.ReactorEvent;
            Assert.Equal(MsgClasses.REFRESH, msgEvent.Msg.MsgClass);
            receivedRefreshMsg = (IRefreshMsg)msgEvent.Msg;
            Assert.Equal((int)DomainType.MARKET_PRICE, receivedRefreshMsg.DomainType);
            Assert.Equal(DataTypes.NO_DATA, receivedRefreshMsg.ContainerType);
            Assert.Equal(StreamStates.OPEN, receivedRefreshMsg.State.StreamState());
            Assert.Equal(DataStates.OK, receivedRefreshMsg.State.DataState());
            Assert.Equal(consStreamIds[1], receivedRefreshMsg.StreamId);

            /* Consumer requests directory. */
            DirectoryRequest directoryRequest = new();

            directoryRequest.Filter = Directory.ServiceFilterFlags.INFO | Directory.ServiceFilterFlags.STATE | Directory.ServiceFilterFlags.GROUP;
            directoryRequest.StreamId = 7;
            directoryRequest.Streaming = true;

            submitOptions.Clear();
            submitOptions.ServiceName = Provider.DefaultService.Info.ServiceName.ToString();
            submitOptions.RequestMsgOptions.UserSpecObj = "77775555";
            Assert.True(consumer.Submit((MsgBase)directoryRequest, submitOptions) >= ReactorReturnCode.SUCCESS);

            /* Consumer receives directory refresh. */
            consumerReactor.Dispatch(1);
            evt = consumerReactor.PollEvent();
            Assert.Equal(TestReactorEventType.DIRECTORY_MSG, evt.EventType);
            RDMDirectoryMsgEvent directoryMsgEvent = (RDMDirectoryMsgEvent)evt.ReactorEvent;
            Assert.Equal(DirectoryMsgType.REFRESH, directoryMsgEvent.DirectoryMsg.DirectoryMsgType);
            DirectoryRefresh directoryRefresh = directoryMsgEvent.DirectoryMsg.DirectoryRefresh;
            Assert.Equal(7, directoryRefresh.StreamId);
            Assert.Equal(StreamStates.OPEN, directoryRefresh.State.StreamState());
            Assert.Equal(DataStates.OK, directoryRefresh.State.DataState());
            Assert.NotNull(directoryMsgEvent.StreamInfo.UserSpec);
            Assert.Equal("77775555", directoryMsgEvent.StreamInfo.UserSpec);
            List<Service> serviceList = directoryRefresh.ServiceList;
            Assert.Equal(serviceList[0].ServiceId, Provider.DefaultService.ServiceId);
            Assert.Equal(MapEntryActions.ADD, serviceList[0].Action);

            /* Consumer requests second directory request. */
            directoryRequest.StreamId = 8;

            submitOptions.Clear();
            submitOptions.ServiceName = Provider.DefaultService.Info.ServiceName.ToString();
            submitOptions.RequestMsgOptions.UserSpecObj = "77776666";
            Assert.True(consumer.Submit((MsgBase)directoryRequest, submitOptions) >= ReactorReturnCode.SUCCESS);

            /* Consumer receives second directory refresh. */
            consumerReactor.Dispatch(1);
            evt = consumerReactor.PollEvent();
            Assert.Equal(TestReactorEventType.DIRECTORY_MSG, evt.EventType);
            directoryMsgEvent = (RDMDirectoryMsgEvent)evt.ReactorEvent;
            Assert.Equal(DirectoryMsgType.REFRESH, directoryMsgEvent.DirectoryMsg.DirectoryMsgType);
            directoryRefresh = directoryMsgEvent.DirectoryMsg.DirectoryRefresh;
            Assert.Equal(8, directoryRefresh.StreamId);
            Assert.Equal(StreamStates.OPEN, directoryRefresh.State.StreamState());
            Assert.Equal(DataStates.OK, directoryRefresh.State.DataState());
            Assert.NotNull(directoryMsgEvent.StreamInfo.UserSpec);
            Assert.Equal("77776666", directoryMsgEvent.StreamInfo.UserSpec);
            serviceList = directoryRefresh.ServiceList;
            Assert.Equal(serviceList[0].ServiceId, Provider.DefaultService.ServiceId);
            Assert.Equal(MapEntryActions.ADD, serviceList[0].Action);

            /* Send a generic message on the first item stream. */
            genericMsg.Clear();
            genericMsg.MsgClass = MsgClasses.GENERIC;
            genericMsg.StreamId = consStreamIds[0];
            genericMsg.DomainType = (int)DomainType.MARKET_PRICE;
            genericMsg.ContainerType = DataTypes.NO_DATA;
            genericMsg.ApplyMessageComplete();

            submitOptions.Clear();
            Assert.True(consumer.Submit((Msg)genericMsg, submitOptions) >= ReactorReturnCode.SUCCESS);

            /* Provider receives generic message. */
            providerReactor.Dispatch(1);
            evt = providerReactor.PollEvent();
            Assert.Equal(TestReactorEventType.MSG, evt.EventType);
            msgEvent = (ReactorMsgEvent)evt.ReactorEvent;
            Assert.Equal(MsgClasses.GENERIC, msgEvent.Msg.MsgClass);
            receivedGenericMsg = (IGenericMsg)msgEvent.Msg;
            Assert.True(receivedGenericMsg.CheckMessageComplete());
            Assert.Equal(providerStreamId, receivedGenericMsg.StreamId);

            /* Provider sends generic message back. */
            genericMsg.Clear();
            genericMsg.MsgClass = MsgClasses.GENERIC;
            genericMsg.StreamId = providerStreamId;
            genericMsg.DomainType = (int)DomainType.MARKET_PRICE;
            genericMsg.ContainerType = DataTypes.NO_DATA;
            genericMsg.ApplyMessageComplete();

            submitOptions.Clear();
            Assert.True(provider.Submit((Msg)genericMsg, submitOptions) >= ReactorReturnCode.SUCCESS);

            /* Consumer receives two generic messages(one on each request). */
            consumerReactor.Dispatch(2);

            bool firstStreamGenericMsg = false;
            bool secondStreamGenericMsg = false;
            for (int i = 0; i < 2; i++)
            {
                evt = consumerReactor.PollEvent();
                Assert.Equal(TestReactorEventType.MSG, evt.EventType);
                msgEvent = (ReactorMsgEvent)evt.ReactorEvent;
                Assert.Equal(MsgClasses.GENERIC, msgEvent.Msg.MsgClass);
                receivedGenericMsg = (IGenericMsg)msgEvent.Msg;
                Assert.True(receivedGenericMsg.CheckMessageComplete());

                if (receivedGenericMsg.StreamId == consStreamIds[0])
                {
                    Assert.False(firstStreamGenericMsg);
                    firstStreamGenericMsg = true;
                }
                else if (receivedGenericMsg.StreamId == consStreamIds[1])
                {
                    Assert.False(secondStreamGenericMsg);
                    secondStreamGenericMsg = true;
                }
                else
                    Assert.False(true);
            }

            Assert.True(firstStreamGenericMsg && secondStreamGenericMsg);
            Assert.True(consumerReactor.m_EventQueue.Count == 0);

            /* Now try receiving a generic message on the login stream. */

            /* Provider sends generic message . */
            genericMsg.Clear();
            genericMsg.MsgClass = MsgClasses.GENERIC;
            genericMsg.StreamId = provider.DefaultSessionLoginStreamId;
            genericMsg.DomainType = (int)DomainType.LOGIN;
            genericMsg.ContainerType = DataTypes.NO_DATA;
            genericMsg.ApplyMessageComplete();

            submitOptions.Clear();
            Assert.True(provider.SubmitAndDispatch((Msg)genericMsg, submitOptions) >= ReactorReturnCode.SUCCESS);

            /* Consumer receives generic message. */
            consumerReactor.Dispatch(1);
            evt = consumerReactor.PollEvent();
            Assert.Equal(TestReactorEventType.MSG, evt.EventType);
            msgEvent = (ReactorMsgEvent)evt.ReactorEvent;
            Assert.Equal(MsgClasses.GENERIC, msgEvent.Msg.MsgClass);
            receivedGenericMsg = (IGenericMsg)msgEvent.Msg;
            Assert.Equal(1, receivedGenericMsg.StreamId);
            Assert.Equal((int)DomainType.LOGIN, receivedGenericMsg.DomainType);
            Assert.True(receivedGenericMsg.CheckMessageComplete());

            /* Now try receiving a generic message on directory streams. */
            /* Provider sends generic message back. */
            genericMsg.Clear();
            genericMsg.MsgClass = MsgClasses.GENERIC;
            genericMsg.StreamId = provider.DefaultSessionDirectoryStreamId;
            genericMsg.DomainType = (int)DomainType.SOURCE;
            genericMsg.ContainerType = DataTypes.NO_DATA;
            genericMsg.ApplyMessageComplete();

            submitOptions.Clear();
            Assert.True(provider.SubmitAndDispatch((Msg)genericMsg, submitOptions) >= ReactorReturnCode.SUCCESS);

            /* Consumer receives generic message on both directory streams */
            consumerReactor.Dispatch(3);

            firstStreamGenericMsg = false;
            secondStreamGenericMsg = false;
            for (int i = 0; i < 3; i++)
            {
                evt = consumerReactor.PollEvent();
                Assert.Equal(TestReactorEventType.MSG, evt.EventType);
                msgEvent = (ReactorMsgEvent)evt.ReactorEvent;
                Assert.Equal(MsgClasses.GENERIC, msgEvent.Msg.MsgClass);
                receivedGenericMsg = (IGenericMsg)msgEvent.Msg;
                Assert.True(receivedGenericMsg.CheckMessageComplete());
                Assert.Equal((int)DomainType.SOURCE, receivedGenericMsg.DomainType);

                if (receivedGenericMsg.StreamId == consumer.DefaultSessionDirectoryStreamId)
                {
                    // Receives the default directory stream
                }
                else if (receivedGenericMsg.StreamId == 7)
                {
                    Assert.False(firstStreamGenericMsg);
                    firstStreamGenericMsg = true;
                }
                else if (receivedGenericMsg.StreamId == 8)
                {
                    Assert.False(secondStreamGenericMsg);
                    secondStreamGenericMsg = true;
                }
                else
                    Assert.False(true);
            }

            Assert.True(firstStreamGenericMsg && secondStreamGenericMsg);
            Assert.True(consumerReactor.m_EventQueue.Count == 0);
            Assert.True(providerReactor.m_EventQueue.Count == 0);

            /* Close first item. */
            ICloseMsg closeMsg = new Msg();
            closeMsg.MsgClass = MsgClasses.CLOSE;
            closeMsg.StreamId = consStreamIds[0];
            closeMsg.DomainType = (int)DomainType.MARKET_PRICE;

            submitOptions.Clear();
            submitOptions.ServiceName = Provider.DefaultService.Info.ServiceName.ToString();
            Assert.True(consumer.SubmitAndDispatch((Msg)closeMsg, submitOptions) >= ReactorReturnCode.SUCCESS);

            Assert.True(consumerReactor.m_EventQueue.Count == 0);

            /* Provider receives request -- same stream, no refresh, updated priority. */
            providerReactor.Dispatch(1);
            evt = providerReactor.PollEvent();
            Assert.Equal(TestReactorEventType.MSG, evt.EventType);
            msgEvent = (ReactorMsgEvent)evt.ReactorEvent;
            Assert.Equal(MsgClasses.REQUEST, msgEvent.Msg.MsgClass);
            receivedRequestMsg = (IRequestMsg)msgEvent.Msg;
            Assert.True(receivedRequestMsg.CheckStreaming());
            Assert.True(receivedRequestMsg.CheckHasPriority());
            Assert.Equal(1, receivedRequestMsg.Priority.PriorityClass);
            Assert.Equal(1, receivedRequestMsg.Priority.Count);
            Assert.True(receivedRequestMsg.CheckNoRefresh());
            Assert.Equal(providerStreamId, receivedRequestMsg.StreamId);

            /* Close second item. */
            closeMsg.StreamId = consStreamIds[1];
            closeMsg.DomainType = (int)DomainType.MARKET_PRICE;

            submitOptions.Clear();
            Assert.True(consumer.SubmitAndDispatch((Msg)closeMsg, submitOptions) >= ReactorReturnCode.SUCCESS);

            /* Provider receives close. */
            providerReactor.Dispatch(1);
            evt = providerReactor.PollEvent();
            Assert.Equal(TestReactorEventType.MSG, evt.EventType);
            msgEvent = (ReactorMsgEvent)evt.ReactorEvent;
            Assert.Equal(MsgClasses.CLOSE, msgEvent.Msg.MsgClass);
            Assert.Equal(providerStreamId, receivedRequestMsg.StreamId);

            TestReactorComponent.CloseSession(consumer, provider);
            consumerReactor.Close();
            providerReactor.Close();
        }

        [Fact]
        public void WatchlistThreeItemsServiceNameAndIdTest()
        {
            ReactorSubmitOptions submitOptions = new();
            TestReactorEvent evt;
            ReactorMsgEvent msgEvent;
            Msg msg = new();
            IRequestMsg requestMsg = msg;
            IRefreshMsg refreshMsg = msg;

            /* Connect the consumer and provider. Setup login & directory streams automatically. */
            ConsumerProviderSessionOptions opts = new();
            opts.SetupDefaultLoginStream = true;
            opts.SetupDefaultDirectoryStream = true;

            SetupSession(opts, false, 0, false, 0, out var consumerReactor, out var providerReactor, out var consumer, out var provider);

            int[] consStreamIds = { 5, 6, 7 };

            /* Request first item. */
            requestMsg.Clear();
            requestMsg.StreamId = consStreamIds[0];
            requestMsg.MsgClass = MsgClasses.REQUEST;
            requestMsg.DomainType = (int)DomainType.MARKET_PRICE;
            requestMsg.ContainerType = DataTypes.NO_DATA;
            requestMsg.MsgKey.ApplyHasServiceId();
            requestMsg.ApplyStreaming();
            requestMsg.ApplyHasQos();
            requestMsg.MsgKey.ServiceId = Provider.DefaultService.ServiceId;
            requestMsg.Qos.Timeliness(QosTimeliness.REALTIME);
            requestMsg.Qos.Rate(QosRates.TICK_BY_TICK);

            submitOptions.Clear();
            Assert.True(consumer.SubmitAndDispatch((Msg)requestMsg, submitOptions) >= ReactorReturnCode.SUCCESS);

            /* Provider receives request. */
            providerReactor.Dispatch(1);
            evt = providerReactor.PollEvent();
            Assert.Equal(TestReactorEventType.MSG, evt.EventType);
            msgEvent = (ReactorMsgEvent)evt.ReactorEvent;
            Assert.Equal(MsgClasses.REQUEST, msgEvent.Msg.MsgClass);
            IRequestMsg receivedRequestMsg = (IRequestMsg)msgEvent.Msg;
            Assert.True(receivedRequestMsg.CheckStreaming());
            Assert.True(receivedRequestMsg.CheckHasPriority());
            Assert.Equal(1, receivedRequestMsg.Priority.PriorityClass);
            Assert.Equal(1, receivedRequestMsg.Priority.Count);
            Assert.False(receivedRequestMsg.CheckNoRefresh());
            Assert.True(receivedRequestMsg.MsgKey.CheckHasServiceId());
            Assert.Equal(Provider.DefaultService.ServiceId, receivedRequestMsg.MsgKey.ServiceId);
            int providerStreamId = receivedRequestMsg.StreamId;

            /* Request second item. */
            requestMsg.StreamId = consStreamIds[1];

            submitOptions.Clear();
            Assert.True(consumer.SubmitAndDispatch((Msg)requestMsg, submitOptions) >= ReactorReturnCode.SUCCESS);

            /* Provider receives request -- same stream, no refresh, updated priority. */
            providerReactor.Dispatch(1);
            evt = providerReactor.PollEvent();
            Assert.Equal(TestReactorEventType.MSG, evt.EventType);
            msgEvent = (ReactorMsgEvent)evt.ReactorEvent;
            Assert.Equal(MsgClasses.REQUEST, msgEvent.Msg.MsgClass);
            receivedRequestMsg = (IRequestMsg)msgEvent.Msg;
            Assert.True(receivedRequestMsg.CheckStreaming());
            Assert.True(receivedRequestMsg.CheckHasPriority());
            Assert.Equal(1, receivedRequestMsg.Priority.PriorityClass);
            Assert.Equal(2, receivedRequestMsg.Priority.Count);
            Assert.True(receivedRequestMsg.CheckNoRefresh());
            Assert.True(receivedRequestMsg.MsgKey.CheckHasServiceId());
            Assert.Equal(Provider.DefaultService.ServiceId, receivedRequestMsg.MsgKey.ServiceId);
            Assert.Equal(providerStreamId, receivedRequestMsg.StreamId);

            /* Request thrid item, using service name */
            requestMsg.StreamId = consStreamIds[2];
            requestMsg.MsgKey.Flags &= ~MsgKeyFlags.HAS_SERVICE_ID;

            submitOptions.Clear();
            submitOptions.ServiceName = Provider.DefaultService.Info.ServiceName.ToString();
            Assert.True(consumer.SubmitAndDispatch((Msg)requestMsg, submitOptions) >= ReactorReturnCode.SUCCESS);

            /* Provider receives request -- same stream, no refresh, updated priority. */
            providerReactor.Dispatch(1);
            evt = providerReactor.PollEvent();
            Assert.Equal(TestReactorEventType.MSG, evt.EventType);
            msgEvent = (ReactorMsgEvent)evt.ReactorEvent;
            Assert.Equal(MsgClasses.REQUEST, msgEvent.Msg.MsgClass);
            receivedRequestMsg = (IRequestMsg)msgEvent.Msg;
            Assert.True(receivedRequestMsg.CheckStreaming());
            Assert.True(receivedRequestMsg.CheckHasPriority());
            Assert.Equal(1, receivedRequestMsg.Priority.PriorityClass);
            Assert.Equal(3, receivedRequestMsg.Priority.Count);
            Assert.True(receivedRequestMsg.CheckNoRefresh());
            Assert.True(receivedRequestMsg.MsgKey.CheckHasServiceId());
            Assert.Equal(Provider.DefaultService.ServiceId, receivedRequestMsg.MsgKey.ServiceId);
            Assert.Equal(providerStreamId, receivedRequestMsg.StreamId);

            /* Close first item. */
            ICloseMsg closeMsg = new Msg();
            closeMsg.MsgClass = MsgClasses.CLOSE;
            closeMsg.StreamId = consStreamIds[0];
            closeMsg.DomainType = (int)DomainType.MARKET_PRICE;

            submitOptions.Clear();
            Assert.True(consumer.SubmitAndDispatch((Msg)closeMsg, submitOptions) >= ReactorReturnCode.SUCCESS);
            Assert.True(consumerReactor.m_EventQueue.Count == 0);

            /* Provider receives request -- same stream, no refresh, updated priority. */
            providerReactor.Dispatch(1);
            evt = providerReactor.PollEvent();
            Assert.Equal(TestReactorEventType.MSG, evt.EventType);
            msgEvent = (ReactorMsgEvent)evt.ReactorEvent;
            Assert.Equal(MsgClasses.REQUEST, msgEvent.Msg.MsgClass);
            receivedRequestMsg = (IRequestMsg)msgEvent.Msg;
            Assert.True(receivedRequestMsg.CheckStreaming());
            Assert.True(receivedRequestMsg.CheckHasPriority());
            Assert.Equal(1, receivedRequestMsg.Priority.PriorityClass);
            Assert.Equal(2, receivedRequestMsg.Priority.Count);
            Assert.True(receivedRequestMsg.CheckNoRefresh());
            Assert.Equal(providerStreamId, receivedRequestMsg.StreamId);

            /* Close second item. */
            closeMsg.StreamId = consStreamIds[1];
            closeMsg.DomainType = (int)DomainType.MARKET_PRICE;

            submitOptions.Clear();
            Assert.True(consumer.SubmitAndDispatch((Msg)closeMsg, submitOptions) >= ReactorReturnCode.SUCCESS);
            Assert.True(consumerReactor.m_EventQueue.Count == 0);

            /* Provider receives request -- same stream, no refresh, updated priority. */
            providerReactor.Dispatch(1);
            evt = providerReactor.PollEvent();
            Assert.Equal(TestReactorEventType.MSG, evt.EventType);
            msgEvent = (ReactorMsgEvent)evt.ReactorEvent;
            Assert.Equal(MsgClasses.REQUEST, msgEvent.Msg.MsgClass);
            receivedRequestMsg = (IRequestMsg)msgEvent.Msg;
            Assert.True(receivedRequestMsg.CheckStreaming());
            Assert.True(receivedRequestMsg.CheckHasPriority());
            Assert.Equal(1, receivedRequestMsg.Priority.PriorityClass);
            Assert.Equal(1, receivedRequestMsg.Priority.Count);
            Assert.True(receivedRequestMsg.CheckNoRefresh());
            Assert.Equal(providerStreamId, receivedRequestMsg.StreamId);

            /* Close third item. */
            closeMsg.StreamId = consStreamIds[2];
            closeMsg.DomainType = (int)DomainType.MARKET_PRICE;

            submitOptions.Clear();
            Assert.True(consumer.SubmitAndDispatch((Msg)closeMsg, submitOptions) >= ReactorReturnCode.SUCCESS);
            Assert.True(consumerReactor.m_EventQueue.Count == 0);

            /* Provider receives close. */
            providerReactor.Dispatch(1);
            evt = providerReactor.PollEvent();
            Assert.Equal(TestReactorEventType.MSG, evt.EventType);
            msgEvent = (ReactorMsgEvent)evt.ReactorEvent;
            Assert.Equal(MsgClasses.CLOSE, msgEvent.Msg.MsgClass);
            Assert.Equal(providerStreamId, receivedRequestMsg.StreamId);

            TestReactorComponent.CloseSession(consumer, provider);
            consumerReactor.Close();
            providerReactor.Close();
        }

        class WatchlistAggregationTest_TwoItems_Callback_CloseByStreamId : Consumer
        {
            private int m_streamID;
            private bool sendCloseMsg = true; // Send close request by default

            public WatchlistAggregationTest_TwoItems_Callback_CloseByStreamId(TestReactor testReactor, int streamId) :
                base(testReactor)
            {
                m_streamID = streamId;
            }

            public override ReactorCallbackReturnCode DefaultMsgCallback(ReactorMsgEvent evt)
            {
                base.DefaultMsgCallback(evt);

                if (sendCloseMsg)
                {
                    /* Closes the specified stream ID */
                    ICloseMsg closeMsg = new Msg();
                    closeMsg.MsgClass = MsgClasses.CLOSE;
                    ReactorSubmitOptions submitOptions = new();
                    closeMsg.StreamId = m_streamID;
                    closeMsg.DomainType = (int)DomainType.MARKET_PRICE;

                    Assert.True(Submit((Msg)closeMsg, submitOptions) >= ReactorReturnCode.SUCCESS);

                    sendCloseMsg = false;
                }

                return ReactorCallbackReturnCode.SUCCESS;
            }
        }

        [Fact]
        public void WatchlistTwoItemsCloseFirstAndSecondInCallbackTest()
        {
            ReactorSubmitOptions submitOptions = new();
            TestReactorEvent evt;
            ReactorMsgEvent msgEvent;
            Msg msg = new();
            IRequestMsg requestMsg = msg;
            IRequestMsg receivedRequestMsg;
            IRefreshMsg refreshMsg = msg;
            IRefreshMsg receivedRefreshMsg;
            int providerStreamId;
            int[] streamIdList = { 5, 6 };

            for (int i = 0; i < streamIdList.Length; i++)
            {
                /* Create reactors. */
                TestReactor consumerReactor = new();
                TestReactor providerReactor = new();

                /* Create consumer. */
                Consumer consumer = new WatchlistAggregationTest_TwoItems_Callback_CloseByStreamId(consumerReactor, streamIdList[i]);
                ConsumerRole consumerRole = (ConsumerRole)consumer.ReactorRole;
                consumerRole.InitDefaultRDMLoginRequest();
                consumerRole.InitDefaultRDMDirectoryRequest();
                consumerRole.ChannelEventCallback = consumer;
                consumerRole.LoginMsgCallback = consumer;
                consumerRole.DirectoryMsgCallback = consumer;
                consumerRole.DictionaryMsgCallback = consumer;
                consumerRole.DefaultMsgCallback = consumer;
                consumerRole.WatchlistOptions.EnableWatchlist = true;
                consumerRole.WatchlistOptions.ChannelOpenEventCallback = consumer;
                consumerRole.WatchlistOptions.RequestTimeout = 3000;

                /* Create provider. */
                Provider provider = new(providerReactor);
                ProviderRole providerRole = (ProviderRole)provider.ReactorRole;
                providerRole.ChannelEventCallback = provider;
                providerRole.LoginMsgCallback = provider;
                providerRole.DirectoryMsgCallback = provider;
                providerRole.DictionaryMsgCallback = provider;
                providerRole.DefaultMsgCallback = provider;

                /* Connect the consumer and provider. Setup login & directory streams automatically. */
                ConsumerProviderSessionOptions opts = new();
                opts.SetupDefaultLoginStream = true;
                opts.SetupDefaultDirectoryStream = true;

                provider.Bind(opts);
                TestReactor.OpenSession(consumer, provider, opts);

                /* Request first item. */
                requestMsg.Clear();
                requestMsg.StreamId = streamIdList[0];
                requestMsg.MsgClass = MsgClasses.REQUEST;
                requestMsg.DomainType = (int)DomainType.MARKET_PRICE;
                requestMsg.ApplyStreaming();
                requestMsg.ApplyHasQos();
                requestMsg.Qos.Timeliness(QosTimeliness.REALTIME);
                requestMsg.Qos.Rate(QosRates.TICK_BY_TICK);

                submitOptions.Clear();
                submitOptions.ServiceName = Provider.DefaultService.Info.ServiceName.ToString();
                Assert.True(consumer.SubmitAndDispatch((Msg)requestMsg, submitOptions) >= ReactorReturnCode.SUCCESS);

                /* Provider receives request. */
                providerReactor.Dispatch(1);
                evt = providerReactor.PollEvent();
                Assert.Equal(TestReactorEventType.MSG, evt.EventType);
                msgEvent = (ReactorMsgEvent)evt.ReactorEvent;
                Assert.Equal(MsgClasses.REQUEST, msgEvent.Msg.MsgClass);
                receivedRequestMsg = (IRequestMsg)msgEvent.Msg;

                providerStreamId = receivedRequestMsg.StreamId;

                /* Request second item with the same request message but different stream ID. */
                requestMsg.StreamId = streamIdList[1];

                submitOptions.Clear();
                submitOptions.ServiceName = Provider.DefaultService.Info.ServiceName.ToString();
                Assert.True(consumer.SubmitAndDispatch((Msg)requestMsg, submitOptions) >= ReactorReturnCode.SUCCESS);

                /* Provider receives request -- same stream, no refresh, updated priority. */
                providerReactor.Dispatch(1);
                evt = providerReactor.PollEvent();
                Assert.Equal(TestReactorEventType.MSG, evt.EventType);
                msgEvent = (ReactorMsgEvent)evt.ReactorEvent;
                Assert.Equal(MsgClasses.REQUEST, msgEvent.Msg.MsgClass);
                receivedRequestMsg = (IRequestMsg)msgEvent.Msg;
                Assert.True(receivedRequestMsg.CheckHasPriority());
                Assert.Equal(1, receivedRequestMsg.Priority.PriorityClass);
                Assert.Equal(2, receivedRequestMsg.Priority.Count);
                Assert.True(receivedRequestMsg.CheckNoRefresh());

                /* Provider sends refresh. */
                refreshMsg.Clear();
                refreshMsg.MsgClass = MsgClasses.REFRESH;
                refreshMsg.DomainType = (int)DomainType.MARKET_PRICE;
                refreshMsg.StreamId = providerStreamId;
                refreshMsg.ContainerType = DataTypes.NO_DATA;
                refreshMsg.ApplyRefreshComplete();
                refreshMsg.ApplySolicited();
                refreshMsg.ApplyClearCache();
                refreshMsg.State.StreamState(StreamStates.OPEN);
                refreshMsg.State.DataState(DataStates.OK);
                requestMsg.ApplyHasQos();
                refreshMsg.Qos.Timeliness(QosTimeliness.REALTIME);
                refreshMsg.Qos.Rate(QosRates.TICK_BY_TICK);

                submitOptions.Clear();
                Assert.True(provider.SubmitAndDispatch((Msg)refreshMsg, submitOptions) >= ReactorReturnCode.SUCCESS);

                if (i == 0)
                {
                    /* Consumer receives refresh. */
                    consumerReactor.Dispatch(2);
                    evt = consumerReactor.PollEvent();
                    Assert.Equal(TestReactorEventType.MSG, evt.EventType);
                    msgEvent = (ReactorMsgEvent)evt.ReactorEvent;
                    Assert.Equal(MsgClasses.REFRESH, msgEvent.Msg.MsgClass);
                    receivedRefreshMsg = (IRefreshMsg)msgEvent.Msg;
                    Assert.Equal((int)DomainType.MARKET_PRICE, receivedRefreshMsg.DomainType);
                    Assert.Equal(DataTypes.NO_DATA, receivedRefreshMsg.ContainerType);
                    Assert.Equal(StreamStates.OPEN, receivedRefreshMsg.State.StreamState());
                    Assert.Equal(DataStates.OK, receivedRefreshMsg.State.DataState());
                    Assert.Equal(streamIdList[0], receivedRefreshMsg.StreamId);

                    evt = consumerReactor.PollEvent();
                    Assert.Equal(TestReactorEventType.MSG, evt.EventType);
                    msgEvent = (ReactorMsgEvent)evt.ReactorEvent;
                    Assert.Equal(MsgClasses.REFRESH, msgEvent.Msg.MsgClass);
                    receivedRefreshMsg = (IRefreshMsg)msgEvent.Msg;
                    Assert.Equal((int)DomainType.MARKET_PRICE, receivedRefreshMsg.DomainType);
                    Assert.Equal(DataTypes.NO_DATA, receivedRefreshMsg.ContainerType);
                    Assert.Equal(StreamStates.OPEN, receivedRefreshMsg.State.StreamState());
                    Assert.Equal(DataStates.OK, receivedRefreshMsg.State.DataState());
                    Assert.Equal(streamIdList[1], receivedRefreshMsg.StreamId);
                }
                else
                {
                    /* Consumer receives refresh. */
                    consumerReactor.Dispatch(1);
                    evt = consumerReactor.PollEvent();
                    Assert.Equal(TestReactorEventType.MSG, evt.EventType);
                    msgEvent = (ReactorMsgEvent)evt.ReactorEvent;
                    Assert.Equal(MsgClasses.REFRESH, msgEvent.Msg.MsgClass);
                    receivedRefreshMsg = (IRefreshMsg)msgEvent.Msg;
                    Assert.Equal((int)DomainType.MARKET_PRICE, receivedRefreshMsg.DomainType);
                    Assert.Equal(DataTypes.NO_DATA, receivedRefreshMsg.ContainerType);
                    Assert.Equal(StreamStates.OPEN, receivedRefreshMsg.State.StreamState());
                    Assert.Equal(DataStates.OK, receivedRefreshMsg.State.DataState());
                    Assert.Equal(streamIdList[0], receivedRefreshMsg.StreamId);
                }
                

                /* Provider receives request -- same stream, no refresh, updated priority. */
                providerReactor.Dispatch(1);
                evt = providerReactor.PollEvent();
                Assert.Equal(TestReactorEventType.MSG, evt.EventType);
                msgEvent = (ReactorMsgEvent)evt.ReactorEvent;
                Assert.Equal(MsgClasses.REQUEST, msgEvent.Msg.MsgClass);
                receivedRequestMsg = (IRequestMsg)msgEvent.Msg;
                Assert.True(receivedRequestMsg.CheckHasPriority());
                Assert.Equal(1, receivedRequestMsg.Priority.PriorityClass);
                Assert.Equal(1, receivedRequestMsg.Priority.Count);
                Assert.True(receivedRequestMsg.CheckNoRefresh());
                Assert.Equal(providerStreamId, receivedRequestMsg.StreamId);

                TestReactorComponent.CloseSession(consumer, provider);
                consumerReactor.Close();
                providerReactor.Close();
            }
        }

        internal class ConsumerCallback : IConsumerCallback
        {
            private Consumer consumer;
            private int count = 0;

            public ConsumerCallback(Consumer consumer)
            {
                this.consumer = consumer;
            }

            public ReactorCallbackReturnCode DefaultMsgCallback(ReactorMsgEvent msgEvent)
            {
                return consumer.DefaultMsgCallback(msgEvent);
            }

            public ReactorCallbackReturnCode RdmDictionaryMsgCallback(RDMDictionaryMsgEvent msgEvent)
            {
                return ReactorCallbackReturnCode.SUCCESS;
            }

            public ReactorCallbackReturnCode RdmDirectoryMsgCallback(RDMDirectoryMsgEvent directoryMsgEvent)
            {
                if (count++ == 0)
                    return consumer.RdmDirectoryMsgCallback(directoryMsgEvent);
                return ReactorCallbackReturnCode.RAISE;
            }

            public ReactorCallbackReturnCode RdmLoginMsgCallback(RDMLoginMsgEvent loginEvent)
            {
                return ReactorCallbackReturnCode.SUCCESS;
            }

            public ReactorCallbackReturnCode ReactorChannelEventCallback(ReactorChannelEvent evt)
            {
                return ReactorCallbackReturnCode.SUCCESS;
            }
        }

        [Fact]
        public void WatchlistDirectoryNoRdmCallbacksTest()
        {
            ReactorSubmitOptions submitOptions = new ReactorSubmitOptions();
            TestReactorEvent evt;
            ReactorMsgEvent msgEvent;

            TestReactor consumerReactor = new TestReactor();
            TestReactor providerReactor = new TestReactor();

            Consumer consumer = new Consumer(consumerReactor);
            ConsumerRole consumerRole = (ConsumerRole)consumer.ReactorRole;
            consumerRole.InitDefaultRDMLoginRequest();
            consumerRole.InitDefaultRDMDirectoryRequest();
            consumerRole.ChannelEventCallback = consumer;
            consumerRole.LoginMsgCallback = consumer;
            consumerRole.DirectoryMsgCallback = new ConsumerCallback(consumer);
            consumerRole.DictionaryMsgCallback = consumer;
            consumerRole.DefaultMsgCallback = consumer;
            consumerRole.WatchlistOptions.EnableWatchlist = true;
            consumerRole.WatchlistOptions.ChannelOpenEventCallback = consumer;

            Provider provider = new Provider(providerReactor);
            ProviderRole providerRole = (ProviderRole)provider.ReactorRole;
            providerRole.ChannelEventCallback = provider;
            providerRole.LoginMsgCallback = provider;
            providerRole.DirectoryMsgCallback = provider;
            providerRole.DictionaryMsgCallback = provider;
            providerRole.DefaultMsgCallback = provider;

            ConsumerProviderSessionOptions opts = new ConsumerProviderSessionOptions();
            opts.SetupDefaultLoginStream = true;
            opts.SetupDefaultDirectoryStream = true;

            provider.Bind(opts);
            TestReactor.OpenSession(consumer, provider, opts);

            var directoryRequest = WlDirectoryHandlerTests.CreateDirectoryRequest(Provider.DefaultService.ServiceId,
                Directory.ServiceFilterFlags.INFO | Directory.ServiceFilterFlags.STATE, 5);

            submitOptions.Clear();
            Assert.True(consumer.Submit(directoryRequest, submitOptions) >= ReactorReturnCode.SUCCESS);

            consumerReactor.Dispatch(1);

            evt = consumerReactor.PollEvent();

            Assert.Equal(TestReactorEventType.MSG, evt.EventType);
            msgEvent = (ReactorMsgEvent)evt.ReactorEvent;
            Assert.Equal(MsgClasses.REFRESH, msgEvent.Msg.MsgClass);
            Assert.Equal(5, msgEvent.Msg.StreamId);

            TestReactorComponent.CloseSession(consumer, provider);
            consumerReactor.Close();
            providerReactor.Close();
        }

        [Fact]
        public void WatchlistDirectoryServiceUpdateLinkFilterOnlyTest()
        {
            ReactorSubmitOptions submitOptions = new ReactorSubmitOptions();
            TestReactorEvent evt;
            ReactorMsgEvent msgEvent;

            ConsumerProviderSessionOptions opts = new ConsumerProviderSessionOptions();
            opts.SetupDefaultLoginStream = true;
            opts.SetupDefaultDirectoryStream = true;

            SetupSession(opts, false, 0, false, 0, out var consumerReactor, out var providerReactor, out var consumer, out var provider);

            var directoryRequest = WlDirectoryHandlerTests.CreateDirectoryRequest(Provider.DefaultService.ServiceId,
                Directory.ServiceFilterFlags.INFO |
                Directory.ServiceFilterFlags.LINK |
                Directory.ServiceFilterFlags.LOAD |
                Directory.ServiceFilterFlags.DATA |
                Directory.ServiceFilterFlags.STATE,
                5);

            submitOptions.Clear();
            Assert.True(consumer.Submit(directoryRequest, submitOptions) >= ReactorReturnCode.SUCCESS);

            consumerReactor.Dispatch(1);
            consumerReactor.PollEvent(); // Get response to user request on user stream 5

            DirectoryUpdate directoryUpdate = new DirectoryUpdate();
            directoryUpdate.StreamId = 2;
            directoryUpdate.Filter = ALL_FILTERS;
            Service service = new Service();
            service.ServiceId = Provider.DefaultService.ServiceId;
            directoryUpdate.ServiceList.Add(service);
            service.Action = MapEntryActions.UPDATE;
            service.Flags = ServiceFlags.HAS_INFO | ServiceFlags.HAS_STATE | ServiceFlags.HAS_LOAD | ServiceFlags.HAS_LINK;

            service.Info.ServiceName = Provider.DefaultService.Info.ServiceName;
            service.Info.CapabilitiesList = new List<long>() { (int)DomainType.HEADLINE, (int)DomainType.REPLAYHEADLINE };
            service.State.ServiceStateVal = 0;
            service.State.Action = FilterEntryActions.UPDATE;
            service.Load.Flags = ServiceLoadFlags.HAS_OPEN_WINDOW;
            service.Load.OpenWindow = 55555;

            ServiceLink link = new ServiceLink();
            link.Name.Data("fish");
            link.LinkState = 1;

            service.Link.LinkList.Add(link);
            service.Link.Action = FilterEntryActions.SET;

            submitOptions.Clear();
            Assert.True(provider.SubmitAndDispatch(directoryUpdate, submitOptions) >= ReactorReturnCode.SUCCESS);

            consumerReactor.Dispatch(2);

            evt = consumerReactor.PollEvent(); // DirectoryUpdate for default Directory request

            Assert.Equal(TestReactorEventType.DIRECTORY_MSG, evt.EventType);
            msgEvent = (ReactorMsgEvent)evt.ReactorEvent;
            Assert.Equal(MsgClasses.UPDATE, msgEvent.Msg.MsgClass);

            var dirEvent = msgEvent as RDMDirectoryMsgEvent;
            Assert.NotNull(dirEvent);

            var dirMsg = dirEvent.DirectoryMsg;
            Assert.NotNull(dirMsg);
            Assert.Equal(DirectoryMsgType.UPDATE, dirMsg.DirectoryMsgType);

            DirectoryUpdate updateMsg = dirMsg.DirectoryUpdate;
            Assert.Equal(2, updateMsg.StreamId);

            evt = consumerReactor.PollEvent(); // DirectoryUpdate for user Directory request (streamId == 5)

            Assert.Equal(TestReactorEventType.DIRECTORY_MSG, evt.EventType);
            msgEvent = (ReactorMsgEvent)evt.ReactorEvent;
            Assert.Equal(MsgClasses.UPDATE, msgEvent.Msg.MsgClass);

            dirEvent = msgEvent as RDMDirectoryMsgEvent;
            Assert.NotNull(dirEvent);

            dirMsg = dirEvent.DirectoryMsg;
            Assert.NotNull(dirMsg);
            Assert.Equal(DirectoryMsgType.UPDATE, dirMsg.DirectoryMsgType);

            updateMsg = dirMsg.DirectoryUpdate;
            Assert.Equal(5, updateMsg.StreamId);
            Assert.Single(updateMsg.ServiceList);
            Assert.Equal(2, updateMsg.ServiceList[0].Info.CapabilitiesList.Count);
            Assert.Equal((int)DomainType.HEADLINE, updateMsg.ServiceList[0].Info.CapabilitiesList[0]);
            Assert.Equal((int)DomainType.REPLAYHEADLINE, updateMsg.ServiceList[0].Info.CapabilitiesList[1]);
            Assert.Equal(FilterEntryActions.UPDATE, updateMsg.ServiceList[0].State.Action);
            Assert.Equal(0, updateMsg.ServiceList[0].State.ServiceStateVal);
            Assert.True(updateMsg.ServiceList[0].HasLink);
            Assert.Equal(MapEntryActions.ADD, updateMsg.ServiceList[0].Link.LinkList[0].Action);
            Assert.Equal("fish", updateMsg.ServiceList[0].Link.LinkList[0].Name.ToString());
            Assert.Equal(1, updateMsg.ServiceList[0].Link.LinkList[0].LinkState);
            Assert.Equal(FilterEntryActions.SET, updateMsg.ServiceList[0].Link.Action);
            Assert.True(updateMsg.ServiceList[0].HasLoad);

            directoryRequest.Filter = Directory.ServiceFilterFlags.LINK; // Consumer requests Directory again
            submitOptions.Clear();
            Assert.True(consumer.Submit(directoryRequest, submitOptions) >= ReactorReturnCode.SUCCESS);

            consumerReactor.Dispatch(1);

            evt = consumerReactor.PollEvent(); // DirectoryRefresh for user Directory request (streamId == 5)

            Assert.Equal(TestReactorEventType.DIRECTORY_MSG, evt.EventType);
            msgEvent = (ReactorMsgEvent)evt.ReactorEvent;
            Assert.Equal(MsgClasses.REFRESH, msgEvent.Msg.MsgClass);

            dirEvent = msgEvent as RDMDirectoryMsgEvent;
            Assert.NotNull(dirEvent);

            dirMsg = dirEvent.DirectoryMsg;
            Assert.NotNull(dirMsg);
            Assert.Equal(DirectoryMsgType.REFRESH, dirMsg.DirectoryMsgType);

            var refreshMsg = dirMsg.DirectoryRefresh;
            Assert.Equal(5, refreshMsg.StreamId);
            Assert.Single(refreshMsg.ServiceList);
            Assert.True(refreshMsg.ServiceList[0].HasLink);
            Assert.Equal(MapEntryActions.ADD, refreshMsg.ServiceList[0].Link.LinkList[0].Action);
            Assert.Equal("fish", refreshMsg.ServiceList[0].Link.LinkList[0].Name.ToString());
            Assert.Equal(1, refreshMsg.ServiceList[0].Link.LinkList[0].LinkState);
            Assert.Equal(FilterEntryActions.SET, refreshMsg.ServiceList[0].Link.Action);

            TestReactorComponent.CloseSession(consumer, provider);
            consumerReactor.Close();
            providerReactor.Close();
        }

        [Fact]
        public void WatchlistDirectoryServiceUpdatePartialFilterTest()
        {
            ReactorSubmitOptions submitOptions = new ReactorSubmitOptions();
            TestReactorEvent evt;
            ReactorMsgEvent msgEvent;

            ConsumerProviderSessionOptions opts = new ConsumerProviderSessionOptions();
            opts.SetupDefaultLoginStream = true;
            opts.SetupDefaultDirectoryStream = true;

            SetupSession(opts, false, 0, false, 0, out var consumerReactor, out var providerReactor, out var consumer, out var provider);

            var directoryRequest = WlDirectoryHandlerTests.CreateDirectoryRequest(Provider.DefaultService.ServiceId,
                Directory.ServiceFilterFlags.INFO |
                Directory.ServiceFilterFlags.STATE |
                Directory.ServiceFilterFlags.GROUP,
                5);

            submitOptions.Clear();
            Assert.True(consumer.Submit(directoryRequest, submitOptions) >= ReactorReturnCode.SUCCESS);

            consumerReactor.Dispatch(1);
            consumerReactor.PollEvent(); // Get response to user request on user stream 5

            Buffer group0 = new Buffer();
            group0.Data("00");
            Buffer group1 = new Buffer();
            group1.Data("01");
            Buffer text = new Buffer();
            text.Data("some random String");

            DirectoryUpdate directoryUpdate = new DirectoryUpdate();
            directoryUpdate.StreamId = 2;
            directoryUpdate.Filter = ALL_FILTERS;
            Service service = new Service();
            service.ServiceId = Provider.DefaultService.ServiceId;
            directoryUpdate.ServiceList.Add(service);
            service.Action = MapEntryActions.UPDATE;
            service.Flags = ServiceFlags.HAS_INFO | ServiceFlags.HAS_STATE | ServiceFlags.HAS_LOAD | ServiceFlags.HAS_LINK;

            service.Info.ServiceName = Provider.DefaultService.Info.ServiceName;
            service.Info.CapabilitiesList = new List<long>() { (int)DomainType.HEADLINE, (int)DomainType.REPLAYHEADLINE };
            service.State.ServiceStateVal = 0;
            service.Load.Flags = ServiceLoadFlags.HAS_OPEN_WINDOW;
            service.Load.OpenWindow = 55555;

            ServiceGroup serviceGroup0 = new ServiceGroup();
            serviceGroup0.Action = FilterEntryActions.SET;
            serviceGroup0.Flags = ServiceGroupFlags.HAS_STATUS;
            serviceGroup0.Group = group1;
            serviceGroup0.Status.StreamState(StreamStates.OPEN);
            serviceGroup0.Status.DataState(DataStates.SUSPECT);
            serviceGroup0.Status.Code(StateCodes.NO_RESOURCES);
            serviceGroup0.Status.Text(text);

            service.GroupStateList.Add(serviceGroup0);

            ServiceGroup serviceGroup1 = new ServiceGroup();
            serviceGroup1.Flags = ServiceGroupFlags.HAS_MERGED_TO_GROUP;
            serviceGroup1.Action = FilterEntryActions.SET;
            serviceGroup1.MergedToGroup = group0;
            serviceGroup1.Group = group1;

            service.GroupStateList.Add(serviceGroup1);

            submitOptions.Clear();
            Assert.True(provider.SubmitAndDispatch(directoryUpdate, submitOptions) >= ReactorReturnCode.SUCCESS);

            consumerReactor.Dispatch(2);
            consumerReactor.PollEvent(); // DirectoryUpdate for default Directory request

            evt = consumerReactor.PollEvent(); // DirectoryUpdate for user Directory request (streamId == 5)

            Assert.Equal(TestReactorEventType.DIRECTORY_MSG, evt.EventType);
            msgEvent = (ReactorMsgEvent)evt.ReactorEvent;
            Assert.Equal(MsgClasses.UPDATE, msgEvent.Msg.MsgClass);

            var dirEvent = msgEvent as RDMDirectoryMsgEvent;
            Assert.NotNull(dirEvent);

            var dirMsg = dirEvent.DirectoryMsg;
            Assert.NotNull(dirMsg);
            Assert.Equal(DirectoryMsgType.UPDATE, dirMsg.DirectoryMsgType);

            var updateMsg = dirMsg.DirectoryUpdate;
            Assert.Equal(5, updateMsg.StreamId);
            Assert.Equal(Directory.ServiceFilterFlags.INFO | Directory.ServiceFilterFlags.STATE | Directory.ServiceFilterFlags.GROUP, updateMsg.Filter);
            Assert.Single(updateMsg.ServiceList);
            Assert.Equal(2, updateMsg.ServiceList[0].Info.CapabilitiesList.Count);
            Assert.Equal((int)DomainType.HEADLINE, updateMsg.ServiceList[0].Info.CapabilitiesList[0]);
            Assert.Equal((int)DomainType.REPLAYHEADLINE, updateMsg.ServiceList[0].Info.CapabilitiesList[1]);
            Assert.Equal(FilterEntryActions.SET, updateMsg.ServiceList[0].State.Action);
            Assert.Equal(MapEntryActions.UPDATE, updateMsg.ServiceList[0].Action);
            Assert.Equal(0, updateMsg.ServiceList[0].State.ServiceStateVal);
            Assert.False(updateMsg.ServiceList[0].HasLink);
            Assert.False(updateMsg.ServiceList[0].HasLoad);
            Assert.Equal(2, updateMsg.ServiceList[0].GroupStateList.Count);
            Assert.Equal(FilterEntryActions.SET, updateMsg.ServiceList[0].GroupStateList[0].Action);
            Assert.Equal(ServiceGroupFlags.HAS_STATUS, updateMsg.ServiceList[0].GroupStateList[0].Flags);
            Assert.Equal(StreamStates.OPEN, updateMsg.ServiceList[0].GroupStateList[0].Status.StreamState());
            Assert.Equal(DataStates.SUSPECT, updateMsg.ServiceList[0].GroupStateList[0].Status.DataState());
            Assert.Equal(text, updateMsg.ServiceList[0].GroupStateList[0].Status.Text());
            Assert.Equal(FilterEntryActions.SET, updateMsg.ServiceList[0].GroupStateList[1].Action);
            Assert.Equal(ServiceGroupFlags.HAS_MERGED_TO_GROUP, updateMsg.ServiceList[0].GroupStateList[1].Flags);
            Assert.Equal(group0, updateMsg.ServiceList[0].GroupStateList[1].MergedToGroup);
            Assert.Equal(group1, updateMsg.ServiceList[0].GroupStateList[1].Group);

            submitOptions.Clear();
            Assert.True(consumer.Submit(directoryRequest, submitOptions) >= ReactorReturnCode.SUCCESS);

            consumerReactor.Dispatch(1);
            evt = consumerReactor.PollEvent(); // Get response to user request on user stream 5

            Assert.Equal(TestReactorEventType.DIRECTORY_MSG, evt.EventType);
            msgEvent = (ReactorMsgEvent)evt.ReactorEvent;
            Assert.Equal(MsgClasses.REFRESH, msgEvent.Msg.MsgClass);

            dirEvent = msgEvent as RDMDirectoryMsgEvent;
            Assert.NotNull(dirEvent);

            dirMsg = dirEvent.DirectoryMsg;
            Assert.NotNull(dirMsg);
            Assert.Equal(DirectoryMsgType.REFRESH, dirMsg.DirectoryMsgType);

            var refreshMsg = dirMsg.DirectoryRefresh;
            Assert.Equal(5, refreshMsg.StreamId);
            Assert.Single(refreshMsg.ServiceList);
            Assert.Equal(Directory.ServiceFilterFlags.INFO | Directory.ServiceFilterFlags.STATE | Directory.ServiceFilterFlags.GROUP, refreshMsg.Filter);
            Assert.Equal(2, refreshMsg.ServiceList[0].Info.CapabilitiesList.Count);
            Assert.Equal(0, refreshMsg.ServiceList[0].State.ServiceStateVal);
            Assert.Equal(FilterEntryActions.SET, refreshMsg.ServiceList[0].State.Action);
            Assert.Empty(refreshMsg.ServiceList[0].GroupStateList);

            TestReactorComponent.CloseSession(consumer, provider);
            consumerReactor.Close();
            providerReactor.Close();
        }

        [Fact]
        public void WatchlistDirectoryRsslMsgCloseTest()
        {
            ReactorSubmitOptions submitOptions = new ReactorSubmitOptions();

            ConsumerProviderSessionOptions opts = new ConsumerProviderSessionOptions();
            opts.SetupDefaultLoginStream = true;
            opts.SetupDefaultDirectoryStream = true;

            SetupSession(opts, false, 0, false, 0, out var consumerReactor, out var providerReactor, out var consumer, out var provider);

            DirectoryClose directoryClose = new DirectoryClose();
            directoryClose.StreamId = 2;
            submitOptions.Clear();
            Assert.True(consumer.Submit(directoryClose, submitOptions) >= ReactorReturnCode.SUCCESS);

            providerReactor.Dispatch(0);

            DirectoryUpdate directoryUpdate = new DirectoryUpdate();
            directoryUpdate.StreamId = 2;
            directoryUpdate.Filter = ALL_FILTERS;
            Service service = new Service();
            service.ServiceId = Provider.DefaultService.ServiceId;
            directoryUpdate.ServiceList.Add(service);
            service.Action = MapEntryActions.UPDATE;
            service.Flags = ServiceFlags.HAS_INFO | ServiceFlags.HAS_STATE | ServiceFlags.HAS_LOAD | ServiceFlags.HAS_LINK;

            service.Info.ServiceName = Provider.DefaultService.Info.ServiceName;
            service.Info.CapabilitiesList = new List<long>() { (int)DomainType.HEADLINE, (int)DomainType.REPLAYHEADLINE };
            service.State.ServiceStateVal = 0;
            service.Load.Flags = ServiceLoadFlags.HAS_OPEN_WINDOW;
            service.Load.OpenWindow = 55555;

            submitOptions.Clear();
            Assert.True(provider.SubmitAndDispatch(directoryUpdate, submitOptions) >= ReactorReturnCode.SUCCESS);

            consumerReactor.Dispatch(0); // User receives no messages because the stream associated with the default request is closed

            TestReactorComponent.CloseSession(consumer, provider);
            consumerReactor.Close();
            providerReactor.Close();
        }

        [Fact]
        public void WatchlistDirectoryDeleteServiceTest()
        {
            ReactorSubmitOptions submitOptions = new ReactorSubmitOptions();
            TestReactorEvent evt;
            ReactorMsgEvent msgEvent;

            ConsumerProviderSessionOptions opts = new ConsumerProviderSessionOptions();
            opts.SetupDefaultLoginStream = true;
            opts.SetupDefaultDirectoryStream = true;

            SetupSession(opts, false, 0, false, 0, out var consumerReactor, out var providerReactor, out var consumer, out var provider);

            DirectoryUpdate directoryUpdate = new DirectoryUpdate();
            directoryUpdate.StreamId = 2;
            directoryUpdate.Filter = ALL_FILTERS;

            Service service1 = new Service();

            int service1Id = 111;
            service1.ServiceId = service1Id;
            service1.Action = MapEntryActions.DELETE;
            directoryUpdate.ServiceList.Add(service1);

            submitOptions.Clear();
            Assert.True(provider.SubmitAndDispatch(directoryUpdate, submitOptions) >= ReactorReturnCode.SUCCESS);

            consumerReactor.Dispatch(1);
            evt = consumerReactor.PollEvent();

            Assert.Equal(TestReactorEventType.DIRECTORY_MSG, evt.EventType);
            msgEvent = (ReactorMsgEvent)evt.ReactorEvent;
            Assert.Equal(MsgClasses.UPDATE, msgEvent.Msg.MsgClass);

            var dirEvent = msgEvent as RDMDirectoryMsgEvent;
            Assert.NotNull(dirEvent);

            var dirMsg = dirEvent.DirectoryMsg;
            Assert.NotNull(dirMsg);
            Assert.Equal(DirectoryMsgType.UPDATE, dirMsg.DirectoryMsgType);

            var updateMsg = dirMsg.DirectoryUpdate;
            Assert.Equal(2, updateMsg.StreamId);
            Assert.Single(updateMsg.ServiceList);
            Assert.Equal(service1Id, updateMsg.ServiceList[0].ServiceId);
            Assert.Equal(MapEntryActions.DELETE, updateMsg.ServiceList[0].Action);

            directoryUpdate.Clear();
            directoryUpdate.StreamId = 2;
            Service service2 = new Service();
            service1.Clear();
            directoryUpdate.ServiceList.Add(service1);
            directoryUpdate.ServiceList.Add(service2);
            service1.ServiceId = service1Id;
            int service2Id = 222;
            service2.ServiceId = service2Id;
            service1.Action = MapEntryActions.ADD;
            service2.Action = MapEntryActions.ADD;
            service1.Flags = ServiceFlags.HAS_INFO;
            service2.Flags = ServiceFlags.HAS_INFO;
            service1.Info.Flags = ServiceInfoFlags.HAS_DICTS_PROVIDED | ServiceInfoFlags.HAS_VENDOR | ServiceInfoFlags.HAS_DICTS_USED;
            service2.Info.Flags = ServiceInfoFlags.HAS_DICTS_PROVIDED | ServiceInfoFlags.HAS_DICTS_USED;
            string service1Name = "ZUREK_SOUP";
            service1.Info.ServiceName.Data(service1Name);
            string service2Name = "CZARNINA_SOUP";
            service2.Info.ServiceName.Data(service2Name);
            string vendor = "Poland";
            var capabilitiesList = new List<long>() { (long)DomainType.DICTIONARY,
            (long)DomainType.MARKET_PRICE,
            (long)DomainType.MARKET_BY_ORDER,
            (long)DomainType.SYMBOL_LIST
            };

            service1.Info.CapabilitiesList = capabilitiesList;
            service2.Info.CapabilitiesList = capabilitiesList;
            service1.Info.Vendor.Data(vendor);
            service2.Info.Vendor.Data(vendor);

            var dictionariesProvidedList = new List<string>() { "RWFFld", "Polish", "Klingon", "StrongBadian" };
            var dictionariesUsedList = new List<string>() { "Spock", "Yelled", "KHAAAAAN", "And", "I", "Facepalmed" };
            service1.Info.DictionariesProvidedList = dictionariesProvidedList;
            service1.Info.DictionariesUsedList = dictionariesUsedList;
            service2.Info.DictionariesProvidedList = dictionariesProvidedList;
            service2.Info.DictionariesUsedList = dictionariesUsedList;

            submitOptions.Clear();
            Assert.True(provider.SubmitAndDispatch(directoryUpdate, submitOptions) >= ReactorReturnCode.SUCCESS);

            consumerReactor.Dispatch(1);
            evt = consumerReactor.PollEvent();

            Assert.Equal(TestReactorEventType.DIRECTORY_MSG, evt.EventType);
            msgEvent = (ReactorMsgEvent)evt.ReactorEvent;
            Assert.Equal(MsgClasses.UPDATE, msgEvent.Msg.MsgClass);

            dirEvent = msgEvent as RDMDirectoryMsgEvent;
            Assert.NotNull(dirEvent);

            dirMsg = dirEvent.DirectoryMsg;
            Assert.NotNull(dirMsg);
            Assert.Equal(DirectoryMsgType.UPDATE, dirMsg.DirectoryMsgType);

            updateMsg = dirMsg.DirectoryUpdate;
            Assert.Equal(2, updateMsg.StreamId);
            Assert.Equal(2, updateMsg.ServiceList.Count);
            int s1 = 0;
            int s2 = 0;
            if (updateMsg.ServiceList[0].ServiceId == service1Id)
            {
                s1 = 0;
                s2 = 1;
            }
            else if (updateMsg.ServiceList[1].ServiceId == service1Id)
            {
                s1 = 1;
                s2 = 0;
            }

            Assert.Equal(ServiceInfoFlags.HAS_DICTS_PROVIDED | ServiceInfoFlags.HAS_VENDOR | ServiceInfoFlags.HAS_DICTS_USED, updateMsg.ServiceList[s1].Info.Flags);
            Assert.Equal(ServiceInfoFlags.HAS_DICTS_PROVIDED | ServiceInfoFlags.HAS_DICTS_USED, updateMsg.ServiceList[s2].Info.Flags);
            Assert.Equal(MapEntryActions.ADD, updateMsg.ServiceList[s1].Action);
            Assert.Equal(MapEntryActions.ADD, updateMsg.ServiceList[s2].Action);
            Assert.Equal(service1Name, updateMsg.ServiceList[s1].Info.ServiceName.ToString());
            Assert.Equal(service2Name, updateMsg.ServiceList[s2].Info.ServiceName.ToString());
            Assert.Equal(capabilitiesList.Count, updateMsg.ServiceList[s1].Info.CapabilitiesList.Count);
            Assert.Equal(capabilitiesList.Count, updateMsg.ServiceList[s2].Info.CapabilitiesList.Count);

            directoryUpdate.Clear();
            directoryUpdate.StreamId = 2;
            service1.Clear();
            service1.ServiceId = service1Id;
            service1.Action = MapEntryActions.DELETE;
            directoryUpdate.ServiceList.Add(service1);

            submitOptions.Clear();
            Assert.True(provider.SubmitAndDispatch(directoryUpdate, submitOptions) >= ReactorReturnCode.SUCCESS);

            consumerReactor.Dispatch(1);
            evt = consumerReactor.PollEvent();

            Assert.Equal(TestReactorEventType.DIRECTORY_MSG, evt.EventType);
            msgEvent = (ReactorMsgEvent)evt.ReactorEvent;
            Assert.Equal(MsgClasses.UPDATE, msgEvent.Msg.MsgClass);

            dirEvent = msgEvent as RDMDirectoryMsgEvent;
            Assert.NotNull(dirEvent);

            dirMsg = dirEvent.DirectoryMsg;
            Assert.NotNull(dirMsg);
            Assert.Equal(DirectoryMsgType.UPDATE, dirMsg.DirectoryMsgType);

            updateMsg = dirMsg.DirectoryUpdate;
            Assert.Equal(2, updateMsg.StreamId);
            Assert.Single(updateMsg.ServiceList);
            Assert.Equal(service1Id, updateMsg.ServiceList[0].ServiceId);
            Assert.Equal(MapEntryActions.DELETE, updateMsg.ServiceList[0].Action);

            TestReactorComponent.CloseSession(consumer, provider);
            consumerReactor.Close();
            providerReactor.Close();
        }

        [Fact]
        public void WatchlistDirectoryBigDirectoryTest()
        {
            ReactorSubmitOptions submitOptions = new ReactorSubmitOptions();
            TestReactorEvent evt;
            ReactorMsgEvent msgEvent;

            ConsumerProviderSessionOptions opts = new ConsumerProviderSessionOptions();
            opts.SetupDefaultLoginStream = true;
            opts.SetupDefaultDirectoryStream = true;

            SetupSession(opts, false, 0, false, 0, out var consumerReactor, out var providerReactor, out var consumer, out var provider);

            string vendor = "Ferengi, Bajoran, Tamarian, Orion, Andorii, Borg, Vulcan, Rihannsu, Federation, Denobula, Rigelian, Talaxian";
            var capabilitiesList = new List<long>() { (long)DomainType.DICTIONARY,
            (long)DomainType.MARKET_PRICE,
            (long)DomainType.MARKET_BY_ORDER,
            (long)DomainType.SYMBOL_LIST
        };
            var dictionariesProvidedList = new List<string>() { "British Virgin Islands", "Cardassian", "Klingon", "StrongBadian", "Turkmenistan", "Liechtenstein", "Negara Brunei Darussalam" };
            var dictionariesUsedList = new List<string>() { "British Virgin Islands", "Romulan", "Klingon", "StrongBadian", "Turkmenistan", "Liechtenstein", "Negara Brunei Darussalam" };
            var itemList = "phasers, photon topedoes, communicator, replicator, holodeck, datapad, android, visor, Nacelle, anti-matter, tribble, Earl Grey Tea";
            var flags = ServiceInfoFlags.HAS_VENDOR | ServiceInfoFlags.HAS_DICTS_PROVIDED | ServiceInfoFlags.HAS_DICTS_USED | ServiceInfoFlags.HAS_ITEM_LIST;

            DirectoryUpdate directoryUpdate = new DirectoryUpdate();
            directoryUpdate.StreamId = 2;
            for (int i = 0; i < 20; i++)
            {
                Service service = new Service();
                service.ServiceId = i + 3;
                service.Flags = ServiceFlags.HAS_INFO | ServiceFlags.HAS_LOAD;
                service.Info.ServiceName.Data($"Star Trek Next Generation Episode {i}");
                service.Action = MapEntryActions.ADD;
                service.Info.Flags = flags;
                service.Info.CapabilitiesList = capabilitiesList;
                service.Info.DictionariesProvidedList = dictionariesProvidedList;
                service.Info.DictionariesUsedList = dictionariesUsedList;
                service.Info.ItemList.Data(itemList);
                service.Info.Vendor.Data(vendor);
                service.Load.Flags = ServiceLoadFlags.HAS_LOAD_FACTOR;
                service.Load.LoadFactor = 111;

                directoryUpdate.ServiceList.Add(service);
            }

            submitOptions.Clear();
            Assert.True(provider.SubmitAndDispatch(directoryUpdate, submitOptions) >= ReactorReturnCode.SUCCESS);

            consumerReactor.Dispatch(1);
            evt = consumerReactor.PollEvent();

            Assert.Equal(TestReactorEventType.DIRECTORY_MSG, evt.EventType);
            msgEvent = (ReactorMsgEvent)evt.ReactorEvent;
            Assert.Equal(MsgClasses.UPDATE, msgEvent.Msg.MsgClass);

            var dirEvent = msgEvent as RDMDirectoryMsgEvent;
            Assert.NotNull(dirEvent);

            var dirMsg = dirEvent.DirectoryMsg;
            Assert.NotNull(dirMsg);
            Assert.Equal(DirectoryMsgType.UPDATE, dirMsg.DirectoryMsgType);

            var updateMsg = dirMsg.DirectoryUpdate;
            Assert.Equal(2, updateMsg.StreamId);
            Assert.Equal(directoryUpdate.ServiceList.Count, updateMsg.ServiceList.Count);
            for (int i = 0; i < 20; i++)
            {
                Assert.Equal(flags, updateMsg.ServiceList[i].Info.Flags);
                Assert.Equal(capabilitiesList, updateMsg.ServiceList[i].Info.CapabilitiesList);
                Assert.Equal(dictionariesProvidedList, updateMsg.ServiceList[i].Info.DictionariesProvidedList);
                Assert.Equal(dictionariesUsedList, updateMsg.ServiceList[i].Info.DictionariesUsedList);
            }

            TestReactorComponent.CloseSession(consumer, provider);
            consumerReactor.Close();
            providerReactor.Close();
        }

        [Fact]
        public void WatchlistMiscAdminMsgsTest()
        {
            ReactorSubmitOptions submitOptions = new ReactorSubmitOptions();
            TestReactorEvent evt;

            TestReactor consumerReactor = new TestReactor();
            TestReactor providerReactor = new TestReactor();

            Consumer consumer = new Consumer(consumerReactor);
            ConsumerRole consumerRole = (ConsumerRole)consumer.ReactorRole;
            consumerRole.ChannelEventCallback = consumer;
            consumerRole.LoginMsgCallback = consumer;
            consumerRole.DirectoryMsgCallback = consumer;
            consumerRole.DictionaryMsgCallback = consumer;
            consumerRole.DefaultMsgCallback = consumer;
            consumerRole.WatchlistOptions.EnableWatchlist = true;
            consumerRole.WatchlistOptions.ChannelOpenEventCallback = consumer;

            Provider provider = new Provider(providerReactor);
            ProviderRole providerRole = (ProviderRole)provider.ReactorRole;
            providerRole.ChannelEventCallback = provider;
            providerRole.LoginMsgCallback = provider;
            providerRole.DirectoryMsgCallback = provider;
            providerRole.DictionaryMsgCallback = provider;
            providerRole.DefaultMsgCallback = provider;

            ConsumerProviderSessionOptions opts = new ConsumerProviderSessionOptions();
            opts.SetupDefaultLoginStream = false;
            opts.SetupDefaultDirectoryStream = false;

            provider.Bind(opts);

            ReactorChannelEvent channelEvent;
            RDMLoginMsgEvent loginMsgEvent;
            RDMDirectoryMsgEvent directoryMsgEvent;

            consumer.TestReactor.Connect(opts, consumer, provider.ServerPort);
            evt = consumer.TestReactor.PollEvent();
            Assert.Equal(TestReactorEventType.CHANNEL_EVENT, evt.EventType);
            channelEvent = (ReactorChannelEvent)evt.ReactorEvent;
            Assert.Equal(ReactorChannelEventType.CHANNEL_OPENED, channelEvent.EventType);

            provider.TestReactor.Accept(opts, provider);
            provider.TestReactor.Dispatch(2);

            evt = provider.TestReactor.PollEvent();
            Assert.Equal(TestReactorEventType.CHANNEL_EVENT, evt.EventType);
            channelEvent = (ReactorChannelEvent)evt.ReactorEvent;
            Assert.Equal(ReactorChannelEventType.CHANNEL_UP, channelEvent.EventType);

            evt = provider.TestReactor.PollEvent();
            Assert.Equal(TestReactorEventType.CHANNEL_EVENT, evt.EventType);
            channelEvent = (ReactorChannelEvent)evt.ReactorEvent;
            Assert.Equal(ReactorChannelEventType.CHANNEL_READY, channelEvent.EventType);

            consumer.TestReactor.Dispatch(2);

            evt = consumer.TestReactor.PollEvent();
            Assert.Equal(TestReactorEventType.CHANNEL_EVENT, evt.EventType);
            channelEvent = (ReactorChannelEvent)evt.ReactorEvent;
            Assert.Equal(ReactorChannelEventType.CHANNEL_UP, channelEvent.EventType);

            evt = consumer.TestReactor.PollEvent();
            Assert.Equal(TestReactorEventType.CHANNEL_EVENT, evt.EventType);
            channelEvent = (ReactorChannelEvent)evt.ReactorEvent;
            Assert.Equal(ReactorChannelEventType.CHANNEL_READY, channelEvent.EventType);

            provider.TestReactor.Dispatch(0);

            LoginRequest loginRequest = new LoginRequest();
            loginRequest.InitDefaultRequest(1);

            submitOptions.Clear();
            Assert.True(consumer.SubmitAndDispatch(loginRequest, submitOptions) >= ReactorReturnCode.SUCCESS);

            /* Provider receives login request. */
            provider.TestReactor.Dispatch(1);
            evt = provider.TestReactor.PollEvent();
            Assert.Equal(TestReactorEventType.LOGIN_MSG, evt.EventType);
            loginMsgEvent = (RDMLoginMsgEvent)evt.ReactorEvent;
            Assert.Equal(LoginMsgType.REQUEST, loginMsgEvent.LoginMsg.LoginMsgType);

            LoginRefresh loginRefresh = new LoginRefresh();

            loginRefresh.Clear();
            loginRefresh.Solicited = true;
            loginRefresh.UserName = loginRequest.UserName;
            loginRefresh.StreamId = loginRequest.StreamId;
            loginRefresh.HasFeatures = true;
            loginRefresh.SupportedFeatures.HasSupportOptimizedPauseResume = true;
            loginRefresh.SupportedFeatures.SupportOptimizedPauseResume = 1;
            loginRefresh.SupportedFeatures.HasSupportViewRequests = true;
            loginRefresh.SupportedFeatures.SupportViewRequests = 1;
            loginRefresh.SupportedFeatures.HasSupportPost = true;
            loginRefresh.SupportedFeatures.SupportOMMPost = 1;

            // required for RequestSymbolListTest_Socket to correctly mimic real-life scenario
            loginRefresh.SupportedFeatures.HasSupportEnhancedSymbolList = true;
            loginRefresh.SupportedFeatures.SupportEnhancedSymbolList = 1;

            loginRefresh.State.StreamState(StreamStates.OPEN);
            loginRefresh.State.DataState(DataStates.OK);
            loginRefresh.State.Code(StateCodes.NONE);
            loginRefresh.State.Text().Data("Login OK");

            submitOptions.Clear();
            Assert.True(provider.SubmitAndDispatch(loginRefresh, submitOptions) >= ReactorReturnCode.SUCCESS);

            consumer.TestReactor.Dispatch(1);

            evt = consumer.TestReactor.PollEvent();
            Assert.Equal(TestReactorEventType.LOGIN_MSG, evt.EventType);
            loginMsgEvent = (RDMLoginMsgEvent)evt.ReactorEvent;
            Assert.Equal(LoginMsgType.REFRESH, loginMsgEvent.LoginMsg.LoginMsgType);

            DirectoryRequest directoryRequest = new DirectoryRequest();
            directoryRequest.StreamId = 2;
            directoryRequest.Filter = ALL_FILTERS;

            submitOptions.Clear();
            Assert.True(consumer.Submit(directoryRequest, submitOptions) >= ReactorReturnCode.SUCCESS);

            /* Provider receives directory request. */
            provider.TestReactor.Dispatch(1);
            evt = provider.TestReactor.PollEvent();
            Assert.Equal(TestReactorEventType.DIRECTORY_MSG, evt.EventType);
            directoryMsgEvent = (RDMDirectoryMsgEvent)evt.ReactorEvent;
            Assert.Equal(DirectoryMsgType.REQUEST, directoryMsgEvent.DirectoryMsg.DirectoryMsgType);

            DirectoryRefresh directoryRefresh = new DirectoryRefresh();

            directoryRefresh.Clear();
            directoryRefresh.StreamId = directoryRequest.StreamId;
            directoryRefresh.Filter = directoryRequest.Filter;
            directoryRefresh.Solicited = true;
            directoryRefresh.ClearCache = true;
            directoryRefresh.State.StreamState(StreamStates.OPEN);
            directoryRefresh.State.DataState(DataStates.OK);
            directoryRefresh.State.Code(StateCodes.NONE);
            directoryRefresh.State.Text().Data("Source Directory Refresh Complete");

            Service service = new();
            Service service2 = new();
            Provider.DefaultService.Copy(service);
            Provider.DefaultService2.Copy(service2);

            directoryRefresh.ServiceList.Add(service);
            directoryRefresh.ServiceList.Add(service2);

            submitOptions.Clear();
            Assert.True(provider.SubmitAndDispatch(directoryRefresh, submitOptions) >= ReactorReturnCode.SUCCESS);

            consumer.TestReactor.Dispatch(1);
            evt = consumer.TestReactor.PollEvent();
            Assert.Equal(TestReactorEventType.DIRECTORY_MSG, evt.EventType);
            directoryMsgEvent = (RDMDirectoryMsgEvent)evt.ReactorEvent;
            Assert.Equal(DirectoryMsgType.REFRESH, directoryMsgEvent.DirectoryMsg.DirectoryMsgType);

            TestReactorComponent.CloseSession(consumer, provider);
            consumerReactor.Close();
            providerReactor.Close();
        }

        [Fact]
        public void WatchlistDirectoryDuplicateServiceNameTest()
        {
            ReactorSubmitOptions submitOptions = new ReactorSubmitOptions();
            TestReactorEvent evt;
            ReactorMsgEvent msgEvent;

            ConsumerProviderSessionOptions opts = new ConsumerProviderSessionOptions();
            opts.SetupDefaultLoginStream = true;
            opts.SetupDefaultDirectoryStream = true;
            opts.SetupSecondDefaultDirectoryStream = true;
            opts.ReconnectAttemptLimit = 3;

            SetupSession(opts, false, 0, false, 0, out var consumerReactor, out var providerReactor, out var consumer, out var provider);

            DirectoryUpdate directoryUpdate = new DirectoryUpdate();
            directoryUpdate.StreamId = 2;
            Service service = new Service();
            service.Flags = ServiceFlags.HAS_INFO;
            service.Info.ServiceName = Provider.DefaultService.Info.ServiceName;
            service.ServiceId = Provider.DefaultService.ServiceId + 1;

            directoryUpdate.ServiceList.Add(service);

            submitOptions.Clear();
            Assert.True(provider.SubmitAndDispatch(directoryUpdate, submitOptions) >= ReactorReturnCode.SUCCESS);

            consumerReactor.Dispatch(3);
            evt = consumerReactor.PollEvent();

            Assert.Equal(TestReactorEventType.CHANNEL_EVENT, evt.EventType);
            var channelEvent = (ReactorChannelEvent)evt.ReactorEvent;
            Assert.Equal(ReactorChannelEventType.CHANNEL_DOWN_RECONNECTING, channelEvent.EventType);

            evt = consumerReactor.PollEvent();
            Assert.Equal(TestReactorEventType.LOGIN_MSG, evt.EventType);
            msgEvent = (ReactorMsgEvent)evt.ReactorEvent;
            Assert.Equal(MsgClasses.STATUS, msgEvent.Msg.MsgClass);

            evt = consumerReactor.PollEvent();
            Assert.Equal(TestReactorEventType.DIRECTORY_MSG, evt.EventType);
            msgEvent = (ReactorMsgEvent)evt.ReactorEvent;
            var dirEvent = msgEvent as RDMDirectoryMsgEvent;
            Assert.NotNull(dirEvent);

            var dirMsg = dirEvent.DirectoryMsg;
            Assert.NotNull(dirMsg);
            Assert.Equal(DirectoryMsgType.UPDATE, dirMsg.DirectoryMsgType);

            providerReactor.Dispatch(1); // provider receives channel down
            evt = providerReactor.PollEvent();
            Assert.Equal(TestReactorEventType.CHANNEL_EVENT, evt.EventType);

            provider.CloseChannel();

            TestReactor.OpenSession(consumer, provider, opts, true); // connection is  successfully recovered

            TestReactorComponent.CloseSession(consumer, provider);
            consumerReactor.Close();
            providerReactor.Close();
        }

        [Fact]
        public void WatchlistThreeItems_OnePrivateTest()
        {
            ReactorSubmitOptions submitOptions = new ReactorSubmitOptions();
            TestReactorEvent evt;
            ReactorMsgEvent msgEvent;
            IRequestMsg requestMsg = (IRequestMsg)new Msg();
            IRequestMsg receivedRequestMsg;
            int providerStreamId;
            int providerPrivateStreamId;
            int testUserSpecObj = 997;

            /* Connect the consumer and provider. Setup login & directory streams automatically. */
            ConsumerProviderSessionOptions opts = new ConsumerProviderSessionOptions();
            opts.SetupDefaultLoginStream = true;
            opts.SetupDefaultDirectoryStream = true;

            SetupSession(opts, false, 0, false, 0, out var consumerReactor, out var providerReactor, out var consumer, out var provider);

            /* Consumer sends streaming request. */
            requestMsg.Clear();
            requestMsg.MsgClass = MsgClasses.REQUEST;
            requestMsg.StreamId = 5;
            requestMsg.DomainType = (int)DomainType.MARKET_PRICE;
            requestMsg.ApplyStreaming();
            requestMsg.MsgKey.ApplyHasName();
            requestMsg.MsgKey.Name.Data("TRI.N");
            requestMsg.ApplyHasQos();
            requestMsg.Qos.Timeliness(QosTimeliness.REALTIME);
            requestMsg.Qos.Rate(QosRates.TICK_BY_TICK);
            submitOptions.Clear();
            submitOptions.ServiceName = Provider.DefaultService.Info.ServiceName.ToString();
            submitOptions.RequestMsgOptions.UserSpecObj = testUserSpecObj;
            Assert.True(consumer.Submit((Msg)requestMsg, submitOptions) >= ReactorReturnCode.SUCCESS);

            providerReactor.Dispatch(1);

            /* Provider receives request. */
            evt = providerReactor.PollEvent();
            Assert.Equal(TestReactorEventType.MSG, evt.EventType);
            msgEvent = (ReactorMsgEvent)evt.ReactorEvent;
            Assert.Equal(MsgClasses.REQUEST, msgEvent.Msg.MsgClass);
            receivedRequestMsg = (IRequestMsg)msgEvent.Msg;
            Assert.True(receivedRequestMsg.MsgKey.CheckHasServiceId());
            Assert.True(receivedRequestMsg.CheckStreaming());
            Assert.False(receivedRequestMsg.CheckNoRefresh());
            Assert.Equal(Provider.DefaultService.ServiceId, receivedRequestMsg.MsgKey.ServiceId);
            Assert.True(receivedRequestMsg.MsgKey.CheckHasName());
            Assert.Equal("TRI.N", receivedRequestMsg.MsgKey.Name.ToString());
            Assert.Equal((int)DomainType.MARKET_PRICE, receivedRequestMsg.DomainType);
            providerStreamId = receivedRequestMsg.StreamId;

            /* Consumer sends streaming request for same item . */
            requestMsg.StreamId = 6;
            submitOptions.Clear();
            submitOptions.ServiceName = Provider.DefaultService.Info.ServiceName.ToString();
            submitOptions.RequestMsgOptions.UserSpecObj = testUserSpecObj;
            Assert.True(consumer.SubmitAndDispatch((Msg)requestMsg, submitOptions) >= ReactorReturnCode.SUCCESS);

            providerReactor.Dispatch(1);

            /* Provider receives priority change due to second request. */
            evt = providerReactor.PollEvent();
            Assert.Equal(TestReactorEventType.MSG, evt.EventType);
            msgEvent = (ReactorMsgEvent)evt.ReactorEvent;
            Assert.Equal(MsgClasses.REQUEST, msgEvent.Msg.MsgClass);
            receivedRequestMsg = (IRequestMsg)msgEvent.Msg;
            Assert.True(receivedRequestMsg.MsgKey.CheckHasServiceId());
            Assert.True(receivedRequestMsg.CheckStreaming());
            Assert.Equal(Provider.DefaultService.ServiceId, receivedRequestMsg.MsgKey.ServiceId);
            Assert.True(receivedRequestMsg.MsgKey.CheckHasName());
            Assert.Equal("TRI.N", receivedRequestMsg.MsgKey.Name.ToString());
            Assert.True(receivedRequestMsg.CheckHasPriority());
            Assert.Equal(1, receivedRequestMsg.Priority.PriorityClass);
            Assert.Equal(2, receivedRequestMsg.Priority.Count);
            Assert.Equal((int)DomainType.MARKET_PRICE, receivedRequestMsg.DomainType);
            Assert.Equal(providerStreamId, receivedRequestMsg.StreamId);

            /* Consumer sends private streaming request for same item . */
            requestMsg.StreamId = 7;
            requestMsg.Flags |= RequestMsgFlags.PRIVATE_STREAM;
            submitOptions.Clear();
            submitOptions.ServiceName = Provider.DefaultService.Info.ServiceName.ToString();
            submitOptions.RequestMsgOptions.UserSpecObj = testUserSpecObj;
            Assert.True(consumer.SubmitAndDispatch((Msg)requestMsg, submitOptions) >= ReactorReturnCode.SUCCESS);

            providerReactor.Dispatch(1);

            /* Provider receives request. */
            evt = providerReactor.PollEvent();
            Assert.Equal(TestReactorEventType.MSG, evt.EventType);
            msgEvent = (ReactorMsgEvent)evt.ReactorEvent;
            Assert.Equal(MsgClasses.REQUEST, msgEvent.Msg.MsgClass);
            receivedRequestMsg = (IRequestMsg)msgEvent.Msg;
            Assert.True(receivedRequestMsg.MsgKey.CheckHasServiceId());
            Assert.True(receivedRequestMsg.CheckStreaming());
            Assert.True(receivedRequestMsg.CheckPrivateStream());
            Assert.False(receivedRequestMsg.CheckNoRefresh());
            Assert.False(receivedRequestMsg.CheckHasPriority());
            Assert.Equal(Provider.DefaultService.ServiceId, receivedRequestMsg.MsgKey.ServiceId);
            Assert.True(receivedRequestMsg.MsgKey.CheckHasName());
            Assert.Equal("TRI.N", receivedRequestMsg.MsgKey.Name.ToString());
            Assert.Equal((int)DomainType.MARKET_PRICE, receivedRequestMsg.DomainType);
            providerPrivateStreamId = receivedRequestMsg.StreamId;

            ICloseMsg closeMsg = new Msg();
            closeMsg.StreamId = 5;

            closeMsg.MsgClass = MsgClasses.CLOSE;
            closeMsg.DomainType = (int)DomainType.MARKET_PRICE;
            submitOptions.Clear();
            Assert.True(consumer.SubmitAndDispatch((Msg)closeMsg, submitOptions) >= ReactorReturnCode.SUCCESS);

            providerReactor.Dispatch(1);

            /* Provider receives priority change due to second request. */
            evt = providerReactor.PollEvent();
            Assert.Equal(TestReactorEventType.MSG, evt.EventType);
            msgEvent = (ReactorMsgEvent)evt.ReactorEvent;
            Assert.Equal(MsgClasses.REQUEST, msgEvent.Msg.MsgClass);
            receivedRequestMsg = (IRequestMsg)msgEvent.Msg;
            Assert.True(receivedRequestMsg.MsgKey.CheckHasName());
            Assert.True(receivedRequestMsg.CheckHasPriority());
            Assert.True(receivedRequestMsg.CheckNoRefresh());
            Assert.Equal(1, receivedRequestMsg.Priority.PriorityClass);
            Assert.Equal(1, receivedRequestMsg.Priority.Count);
            Assert.Equal((int)DomainType.MARKET_PRICE, receivedRequestMsg.DomainType);
            Assert.Equal(providerStreamId, receivedRequestMsg.StreamId);

            closeMsg.StreamId = 7;
            submitOptions.Clear();
            Assert.True(consumer.SubmitAndDispatch((Msg)closeMsg, submitOptions) >= ReactorReturnCode.SUCCESS);

            providerReactor.Dispatch(1);

            evt = providerReactor.PollEvent();
            Assert.Equal(TestReactorEventType.MSG, evt.EventType);
            msgEvent = (ReactorMsgEvent)evt.ReactorEvent;
            Assert.Equal(MsgClasses.CLOSE, msgEvent.Msg.MsgClass);
            var receivedCloseMsg = (ICloseMsg)msgEvent.Msg;
            Assert.Equal(providerPrivateStreamId, receivedCloseMsg.StreamId);

            closeMsg.StreamId = 6;
            submitOptions.Clear();
            Assert.True(consumer.SubmitAndDispatch((Msg)closeMsg, submitOptions) >= ReactorReturnCode.SUCCESS);

            providerReactor.Dispatch(1);

            evt = providerReactor.PollEvent();
            Assert.Equal(TestReactorEventType.MSG, evt.EventType);
            msgEvent = (ReactorMsgEvent)evt.ReactorEvent;
            Assert.Equal(MsgClasses.CLOSE, msgEvent.Msg.MsgClass);
            receivedCloseMsg = (ICloseMsg)msgEvent.Msg;
            Assert.Equal(providerStreamId, receivedCloseMsg.StreamId);

            TestReactorComponent.CloseSession(consumer, provider);
            consumerReactor.Close();
            providerReactor.Close();
        }

        [Fact]
        public void WatchlistOneItem_ClosedRecoverTest()
        {
            ReactorSubmitOptions submitOptions = new ReactorSubmitOptions();
            TestReactorEvent evt;
            ReactorMsgEvent msgEvent;

            ConsumerProviderSessionOptions opts = new ConsumerProviderSessionOptions();
            opts.SetupDefaultLoginStream = true;
            opts.SetupDefaultDirectoryStream = true;

            SetupSession(opts, false, 0, false, 0, out var consumerReactor, out var providerReactor, out var consumer, out var provider);

            IRequestMsg requestMsg = new Msg();
            /* Consumer sends streaming request. */
            requestMsg.Clear();
            requestMsg.MsgClass = MsgClasses.REQUEST;
            requestMsg.StreamId = 5;
            requestMsg.DomainType = (int)DomainType.MARKET_PRICE;
            requestMsg.ApplyStreaming();
            requestMsg.MsgKey.ApplyHasName();
            requestMsg.MsgKey.Name.Data("TRI.N");
            requestMsg.ApplyHasQos();
            requestMsg.Qos.Timeliness(QosTimeliness.REALTIME);
            requestMsg.Qos.Rate(QosRates.TICK_BY_TICK);
            submitOptions.Clear();
            submitOptions.ServiceName = Provider.DefaultService.Info.ServiceName.ToString();
            Assert.True(consumer.Submit((Msg)requestMsg, submitOptions) >= ReactorReturnCode.SUCCESS);

            providerReactor.Dispatch(1);

            /* Provider receives request. */
            evt = providerReactor.PollEvent();
            Assert.Equal(TestReactorEventType.MSG, evt.EventType);
            msgEvent = (ReactorMsgEvent)evt.ReactorEvent;
            Assert.Equal(MsgClasses.REQUEST, msgEvent.Msg.MsgClass);
            IRequestMsg receivedRequestMsg = (IRequestMsg)msgEvent.Msg;
            Assert.True(receivedRequestMsg.MsgKey.CheckHasServiceId());
            Assert.True(receivedRequestMsg.CheckStreaming());
            Assert.False(receivedRequestMsg.CheckNoRefresh());
            Assert.Equal(Provider.DefaultService.ServiceId, receivedRequestMsg.MsgKey.ServiceId);
            Assert.True(receivedRequestMsg.MsgKey.CheckHasName());
            Assert.Equal("TRI.N", receivedRequestMsg.MsgKey.Name.ToString());
            Assert.Equal((int)DomainType.MARKET_PRICE, receivedRequestMsg.DomainType);
            int providerStreamId = receivedRequestMsg.StreamId;

            IStatusMsg statusMsg = new Msg();
            statusMsg.StreamId = providerStreamId;
            statusMsg.MsgClass = MsgClasses.STATUS;
            statusMsg.DomainType = (int)DomainType.MARKET_PRICE;
            statusMsg.ApplyHasState();
            statusMsg.State.StreamState(StreamStates.CLOSED_RECOVER);
            statusMsg.State.DataState(DataStates.SUSPECT);
            submitOptions.Clear();
            submitOptions.ServiceName = Provider.DefaultService.Info.ServiceName.ToString();
            Assert.True(provider.Submit((Msg)statusMsg, submitOptions) >= ReactorReturnCode.SUCCESS);

            consumerReactor.Dispatch(1);

            evt = consumerReactor.PollEvent();
            msgEvent = (ReactorMsgEvent)evt.ReactorEvent;
            Assert.Equal(MsgClasses.STATUS, msgEvent.Msg.MsgClass);
            Assert.True(((IStatusMsg)msgEvent.Msg).CheckHasState());
            Assert.Equal(StreamStates.OPEN, ((IStatusMsg)msgEvent.Msg).State.StreamState());
            Assert.Equal(DataStates.SUSPECT, ((IStatusMsg)msgEvent.Msg).State.DataState());

            providerReactor.Dispatch(1);

            // Provider receives request once again
            evt = providerReactor.PollEvent();
            Assert.Equal(TestReactorEventType.MSG, evt.EventType);
            msgEvent = (ReactorMsgEvent)evt.ReactorEvent;
            Assert.Equal(MsgClasses.REQUEST, msgEvent.Msg.MsgClass);
            receivedRequestMsg = (IRequestMsg)msgEvent.Msg;
            Assert.True(receivedRequestMsg.MsgKey.CheckHasServiceId());
            Assert.True(receivedRequestMsg.CheckStreaming());
            Assert.False(receivedRequestMsg.CheckNoRefresh());
            Assert.Equal(Provider.DefaultService.ServiceId, receivedRequestMsg.MsgKey.ServiceId);
            Assert.True(receivedRequestMsg.MsgKey.CheckHasName());
            Assert.Equal("TRI.N", receivedRequestMsg.MsgKey.Name.ToString());
            Assert.Equal((int)DomainType.MARKET_PRICE, receivedRequestMsg.DomainType);

            TestReactorComponent.CloseSession(consumer, provider);
            consumerReactor.Close();
            providerReactor.Close();
        }

        [Fact]
        public void WatchlistOneItem_ClearCacheTest()
        {
            ReactorSubmitOptions submitOptions = new ReactorSubmitOptions();
            TestReactorEvent evt;
            ReactorMsgEvent msgEvent;

            ConsumerProviderSessionOptions opts = new ConsumerProviderSessionOptions();
            opts.SetupDefaultLoginStream = true;
            opts.SetupDefaultDirectoryStream = true;

            SetupSession(opts, false, 0, false, 0, out var consumerReactor, out var providerReactor, out var consumer, out var provider);

            // Consumer sends first request
            IRequestMsg requestMsg = new Msg();
            requestMsg.Clear();
            requestMsg.MsgClass = MsgClasses.REQUEST;
            requestMsg.StreamId = 5;
            requestMsg.DomainType = (int)DomainType.MARKET_PRICE;
            requestMsg.ApplyStreaming();
            requestMsg.MsgKey.ApplyHasName();
            requestMsg.MsgKey.Name.Data("TRI.N");
            requestMsg.ApplyHasQos();
            requestMsg.Qos.Timeliness(QosTimeliness.REALTIME);
            requestMsg.Qos.Rate(QosRates.TICK_BY_TICK);
            submitOptions.Clear();
            submitOptions.ServiceName = Provider.DefaultService.Info.ServiceName.ToString();
            Assert.True(consumer.SubmitAndDispatch((Msg)requestMsg, submitOptions) >= ReactorReturnCode.SUCCESS);

            // Consumer sends second request
            requestMsg.Clear();
            requestMsg.MsgClass = MsgClasses.REQUEST;
            requestMsg.StreamId = 6;
            requestMsg.DomainType = (int)DomainType.MARKET_PRICE;
            requestMsg.ApplyStreaming();
            requestMsg.MsgKey.ApplyHasName();
            requestMsg.MsgKey.Name.Data("GOOG.L");
            requestMsg.ApplyHasQos();
            requestMsg.Qos.Timeliness(QosTimeliness.REALTIME);
            requestMsg.Qos.Rate(QosRates.TICK_BY_TICK);
            submitOptions.Clear();
            submitOptions.ServiceName = Provider.DefaultService.Info.ServiceName.ToString();
            Assert.True(consumer.SubmitAndDispatch((Msg)requestMsg, submitOptions) >= ReactorReturnCode.SUCCESS);

            providerReactor.Dispatch(2);
            providerReactor.PollEvent();
            providerReactor.PollEvent();

            // Provider resends directory refresh with ClearCache flag
            DirectoryRefresh directoryRefresh = new DirectoryRefresh();

            directoryRefresh.Clear();
            directoryRefresh.StreamId = 2;
            directoryRefresh.Filter = ALL_FILTERS;
            directoryRefresh.Solicited = true;
            directoryRefresh.ClearCache = true;
            directoryRefresh.State.StreamState(StreamStates.OPEN);
            directoryRefresh.State.DataState(DataStates.OK);
            directoryRefresh.State.Code(StateCodes.NONE);
            directoryRefresh.State.Text().Data("Source Directory Refresh Complete");

            Service service = new();
            Provider.DefaultService.Copy(service);
            directoryRefresh.ServiceList.Add(service);

            submitOptions.Clear();
            Assert.True(provider.Submit(directoryRefresh, submitOptions) >= ReactorReturnCode.SUCCESS);

            // Consumer should get Open/Suspect statuses for items and DirectoryRefresh
            // for the default Reactor Directory request initialized for Consumer role
            consumer.TestReactor.Dispatch(3);

            evt = consumer.TestReactor.PollEvent();
            Assert.Equal(TestReactorEventType.MSG, evt.EventType);
            msgEvent = (ReactorMsgEvent)evt.ReactorEvent;
            Assert.Equal(5, msgEvent.Msg.StreamId);
            Assert.Equal(MsgClasses.STATUS, msgEvent.Msg.MsgClass);
            IStatusMsg statusMsg = (IStatusMsg)msgEvent.Msg;
            Assert.Equal(StreamStates.OPEN, statusMsg.State.StreamState());
            Assert.Equal(DataStates.SUSPECT, statusMsg.State.DataState());

            evt = consumer.TestReactor.PollEvent();
            Assert.Equal(TestReactorEventType.MSG, evt.EventType);
            msgEvent = (ReactorMsgEvent)evt.ReactorEvent;
            Assert.Equal(6, msgEvent.Msg.StreamId);
            Assert.Equal(MsgClasses.STATUS, msgEvent.Msg.MsgClass);
            statusMsg = (IStatusMsg)msgEvent.Msg;
            Assert.Equal(StreamStates.OPEN, statusMsg.State.StreamState());
            Assert.Equal(DataStates.SUSPECT, statusMsg.State.DataState());

            evt = consumer.TestReactor.PollEvent();
            Assert.Equal(TestReactorEventType.DIRECTORY_MSG, evt.EventType);
            msgEvent = (ReactorMsgEvent)evt.ReactorEvent;
            Assert.Equal(2, msgEvent.Msg.StreamId);
            Assert.Equal(MsgClasses.UPDATE, msgEvent.Msg.MsgClass);

            providerReactor.Dispatch(2);

            evt = providerReactor.PollEvent();
            Assert.Equal(TestReactorEventType.MSG, evt.EventType);
            msgEvent = (ReactorMsgEvent)evt.ReactorEvent;
            Assert.Equal(MsgClasses.REQUEST, msgEvent.Msg.MsgClass);

            evt = providerReactor.PollEvent();
            Assert.Equal(TestReactorEventType.MSG, evt.EventType);
            msgEvent = (ReactorMsgEvent)evt.ReactorEvent;
            Assert.Equal(MsgClasses.REQUEST, msgEvent.Msg.MsgClass);

            TestReactorComponent.CloseSession(consumer, provider);
            consumerReactor.Close();
            providerReactor.Close();
        }

        [Fact]
        public void WatchlistOneItem_QosChangeTest()
        {
            ReactorSubmitOptions submitOptions = new ReactorSubmitOptions();
            TestReactorEvent evt;
            ReactorMsgEvent msgEvent;

            ConsumerProviderSessionOptions opts = new ConsumerProviderSessionOptions();
            opts.SetupDefaultLoginStream = true;
            opts.SetupDefaultDirectoryStream = true;

            SetupSession(opts, false, 0, false, 0, out var consumerReactor, out var providerReactor, out var consumer, out var provider);

            DirectoryUpdate directoryUpdate = new DirectoryUpdate();
            directoryUpdate.StreamId = 2;
            directoryUpdate.Filter = ALL_FILTERS;
            Service service = new Service();
            Provider.DefaultService.Copy(service);
            directoryUpdate.ServiceList.Add(service);
            service.Action = MapEntryActions.UPDATE;
            service.Info.Flags = service.Info.Flags | ServiceInfoFlags.HAS_QOS;
            Qos qos = new Qos();
            qos.Timeliness(QosTimeliness.REALTIME);
            qos.Rate(QosRates.TICK_BY_TICK);
            service.Info.QosList.Add(qos);

            submitOptions.Clear();
            Assert.True(provider.Submit(directoryUpdate, submitOptions) >= ReactorReturnCode.SUCCESS);

            consumerReactor.Dispatch(1);
            evt = consumerReactor.PollEvent();
            Assert.Equal(TestReactorEventType.DIRECTORY_MSG, evt.EventType);
            msgEvent = (ReactorMsgEvent)evt.ReactorEvent;
            Assert.Equal(MsgClasses.UPDATE, msgEvent.Msg.MsgClass);

            // Consumer sends a request
            IRequestMsg requestMsg = new Msg();
            requestMsg.Clear();
            requestMsg.MsgClass = MsgClasses.REQUEST;
            requestMsg.StreamId = 5;
            requestMsg.DomainType = (int)DomainType.MARKET_PRICE;
            requestMsg.ApplyStreaming();
            requestMsg.MsgKey.ApplyHasName();
            requestMsg.MsgKey.Name.Data("TRI.N");
            requestMsg.ApplyHasQos();
            requestMsg.Qos.Timeliness(QosTimeliness.REALTIME);
            requestMsg.Qos.Rate(QosRates.TICK_BY_TICK);
            submitOptions.Clear();
            submitOptions.ServiceName = Provider.DefaultService.Info.ServiceName.ToString();
            Assert.True(consumer.SubmitAndDispatch((Msg)requestMsg, submitOptions) >= ReactorReturnCode.SUCCESS);

            providerReactor.Dispatch(1);
            evt = providerReactor.PollEvent();
            Assert.Equal(TestReactorEventType.MSG, evt.EventType);
            msgEvent = (ReactorMsgEvent)evt.ReactorEvent;
            Assert.Equal(MsgClasses.REQUEST, msgEvent.Msg.MsgClass);
            int providerStreamId = msgEvent.Msg.StreamId;

            /*** Recover to No QoS ***/
            /* Provider changes QoS to nothing. */
            service.Info.QosList.Clear();
            submitOptions.Clear();
            Assert.True(provider.Submit(directoryUpdate, submitOptions) >= ReactorReturnCode.SUCCESS);

            consumerReactor.Dispatch(1);
            evt = consumerReactor.PollEvent();
            Assert.Equal(TestReactorEventType.DIRECTORY_MSG, evt.EventType);
            msgEvent = (ReactorMsgEvent)evt.ReactorEvent;
            Assert.Equal(MsgClasses.UPDATE, msgEvent.Msg.MsgClass);

            IStatusMsg statusMsg = new Msg();
            statusMsg.StreamId = providerStreamId;
            statusMsg.MsgClass = MsgClasses.STATUS;
            statusMsg.DomainType = (int)DomainType.MARKET_PRICE;
            statusMsg.Flags = StatusMsgFlags.HAS_STATE;
            statusMsg.State.StreamState(StreamStates.CLOSED_RECOVER);
            statusMsg.State.DataState(DataStates.SUSPECT);
            statusMsg.State.Code(StateCodes.NO_RESOURCES);

            submitOptions.Clear();
            submitOptions.ServiceName = Provider.DefaultService.Info.ServiceName.ToString();
            Assert.True(provider.SubmitAndDispatch((Msg)statusMsg, submitOptions) >= ReactorReturnCode.SUCCESS);

            consumerReactor.Dispatch(2);

            /* Consumer receives Open/Suspect status from provider. */
            evt = consumerReactor.PollEvent();
            Assert.Equal(TestReactorEventType.MSG, evt.EventType);
            msgEvent = (ReactorMsgEvent)evt.ReactorEvent;
            Assert.Equal(MsgClasses.STATUS, msgEvent.Msg.MsgClass);
            Assert.Equal(5, msgEvent.Msg.StreamId);
            IStatusMsg receivedStatus = (IStatusMsg)msgEvent.Msg;
            Assert.True(receivedStatus.CheckHasState());
            Assert.Equal(StreamStates.OPEN, receivedStatus.State.StreamState());
            Assert.Equal(DataStates.SUSPECT, receivedStatus.State.DataState());
            Assert.Equal(StateCodes.NO_RESOURCES, receivedStatus.State.Code());

            /* Consumer receives Open/Suspect status from recovery attempt. */
            evt = consumerReactor.PollEvent();
            Assert.Equal(TestReactorEventType.MSG, evt.EventType);
            msgEvent = (ReactorMsgEvent)evt.ReactorEvent;
            Assert.Equal(MsgClasses.STATUS, msgEvent.Msg.MsgClass);
            Assert.Equal(5, msgEvent.Msg.StreamId);
            receivedStatus = (IStatusMsg)msgEvent.Msg;
            Assert.True(receivedStatus.CheckHasState());
            Assert.Equal(StreamStates.OPEN, receivedStatus.State.StreamState());
            Assert.Equal(DataStates.SUSPECT, receivedStatus.State.DataState());
            Assert.Equal(StateCodes.NONE, receivedStatus.State.Code());

            // Provider receives nothing yet.
            providerReactor.Dispatch(0);

            /*** Recover to Delayed QoS ***/
            /* Provider adds QoS */
            qos.Clear();
            qos.Timeliness(QosTimeliness.REALTIME);
            qos.Rate(QosRates.TICK_BY_TICK);
            service.Info.QosList.Clear();
            service.Info.QosList.Add(qos);

            submitOptions.Clear();
            Assert.True(provider.Submit(directoryUpdate, submitOptions) >= ReactorReturnCode.SUCCESS);

            consumerReactor.Dispatch(1);
            evt = consumerReactor.PollEvent();
            Assert.Equal(TestReactorEventType.DIRECTORY_MSG, evt.EventType);
            msgEvent = (ReactorMsgEvent)evt.ReactorEvent;
            Assert.Equal(MsgClasses.UPDATE, msgEvent.Msg.MsgClass);

            /* Provider receives request again. */
            providerReactor.Dispatch(1);
            evt = providerReactor.PollEvent();
            Assert.Equal(TestReactorEventType.MSG, evt.EventType);
            msgEvent = (ReactorMsgEvent)evt.ReactorEvent;
            Assert.Equal(MsgClasses.REQUEST, msgEvent.Msg.MsgClass);
            Assert.Equal("TRI.N", msgEvent.Msg.MsgKey.Name.ToString());

            TestReactorComponent.CloseSession(consumer, provider);
            consumerReactor.Close();
            providerReactor.Close();
        }

        [Fact]
        public void WatchlistTwoItems_OpenWindowTest()
        {
            ReactorSubmitOptions submitOptions = new ReactorSubmitOptions();
            TestReactorEvent evt;
            ReactorMsgEvent msgEvent;

            ConsumerProviderSessionOptions opts = new ConsumerProviderSessionOptions();
            opts.SetupDefaultLoginStream = true;
            opts.SetupDefaultDirectoryStream = true;

            SetupSession(opts, false, 0, false, 0, out var consumerReactor, out var providerReactor, out var consumer, out var provider);

            DirectoryUpdate directoryUpdate = new DirectoryUpdate();
            directoryUpdate.StreamId = 2;
            directoryUpdate.Filter = ALL_FILTERS;
            Service service = new Service();
            Provider.DefaultService.Copy(service);
            directoryUpdate.ServiceList.Add(service);
            service.Action = MapEntryActions.UPDATE;
            service.HasInfo = true;
            service.HasLoad = true;
            service.Info.Flags = service.Info.Flags;
            service.Load.Flags = ServiceLoadFlags.HAS_OPEN_WINDOW;
            service.Load.OpenWindow = 1;

            submitOptions.Clear();
            Assert.True(provider.Submit(directoryUpdate, submitOptions) >= ReactorReturnCode.SUCCESS);

            consumerReactor.Dispatch(1);
            evt = consumerReactor.PollEvent();
            Assert.Equal(TestReactorEventType.DIRECTORY_MSG, evt.EventType);
            msgEvent = (ReactorMsgEvent)evt.ReactorEvent;
            Assert.Equal(MsgClasses.UPDATE, msgEvent.Msg.MsgClass);

            // Consumer sends a request
            IRequestMsg requestMsg = new Msg();
            requestMsg.Clear();
            requestMsg.MsgClass = MsgClasses.REQUEST;
            requestMsg.StreamId = 5;
            requestMsg.DomainType = (int)DomainType.MARKET_PRICE;
            requestMsg.ApplyStreaming();
            requestMsg.MsgKey.ApplyHasIdentifier();
            requestMsg.MsgKey.Identifier = 5;
            requestMsg.ApplyHasQos();
            requestMsg.Qos.Timeliness(QosTimeliness.REALTIME);
            requestMsg.Qos.Rate(QosRates.TICK_BY_TICK);

            submitOptions.Clear();
            submitOptions.ServiceName = Provider.DefaultService.Info.ServiceName.ToString();
            Assert.True(consumer.SubmitAndDispatch((Msg)requestMsg, submitOptions) >= ReactorReturnCode.SUCCESS);

            requestMsg.StreamId = 6;
            requestMsg.MsgKey.Identifier = 6;

            submitOptions.Clear();
            submitOptions.ServiceName = Provider.DefaultService.Info.ServiceName.ToString();
            Assert.True(consumer.SubmitAndDispatch((Msg)requestMsg, submitOptions) >= ReactorReturnCode.SUCCESS);

            providerReactor.Dispatch(1);
            evt = providerReactor.PollEvent();
            Assert.Equal(TestReactorEventType.MSG, evt.EventType);
            msgEvent = (ReactorMsgEvent)evt.ReactorEvent;
            Assert.Equal(MsgClasses.REQUEST, msgEvent.Msg.MsgClass);
            int providerStreamId1 = msgEvent.Msg.StreamId;

            IRefreshMsg refreshMsg = new Msg();
            refreshMsg.Clear();
            refreshMsg.MsgClass = MsgClasses.REFRESH;
            refreshMsg.DomainType = (int)DomainType.MARKET_PRICE;
            refreshMsg.StreamId = msgEvent.Msg.StreamId;
            refreshMsg.ContainerType = DataTypes.NO_DATA;
            refreshMsg.ApplyHasMsgKey();
            refreshMsg.MsgKey.ApplyHasServiceId();
            refreshMsg.ApplyRefreshComplete();
            refreshMsg.ApplySolicited();
            refreshMsg.MsgKey.ServiceId = Provider.DefaultService.ServiceId;
            refreshMsg.State.StreamState(StreamStates.OPEN);
            refreshMsg.State.DataState(DataStates.OK);
            refreshMsg.ApplyClearCache();

            submitOptions.Clear();
            Assert.True(provider.SubmitAndDispatch((Msg)refreshMsg, submitOptions) >= ReactorReturnCode.SUCCESS);

            consumerReactor.Dispatch(1);
            evt = consumerReactor.PollEvent();
            Assert.Equal(TestReactorEventType.MSG, evt.EventType);
            msgEvent = (ReactorMsgEvent)evt.ReactorEvent;
            Assert.Equal(MsgClasses.REFRESH, msgEvent.Msg.MsgClass);
            Assert.Equal(5, msgEvent.Msg.StreamId);

            providerReactor.Dispatch(1);
            evt = providerReactor.PollEvent();
            Assert.Equal(TestReactorEventType.MSG, evt.EventType);
            msgEvent = (ReactorMsgEvent)evt.ReactorEvent;
            Assert.Equal(MsgClasses.REQUEST, msgEvent.Msg.MsgClass);
            Assert.NotEqual(providerStreamId1, msgEvent.Msg.StreamId);

            TestReactorComponent.CloseSession(consumer, provider);
            consumerReactor.Close();
            providerReactor.Close();
        }

        [Fact]
        public void WatchlistSnapshotBeforeStreaming_MultipartTest()
        {
            ReactorSubmitOptions submitOptions = new();
            TestReactorEvent evt;
            ReactorMsgEvent msgEvent;
            Msg msg = new();
            IRequestMsg requestMsg = msg;
            IRefreshMsg refreshMsg = msg;
            IGenericMsg genericMsg = msg;
            IRefreshMsg receivedRefreshMsg;

            /* Connect the consumer and provider. Setup login & directory streams automatically. */
            ConsumerProviderSessionOptions opts = new();
            opts.SetupDefaultLoginStream = true;
            opts.SetupDefaultDirectoryStream = true;

            SetupSession(opts, false, 0, false, 0, out var consumerReactor, out var providerReactor, out var consumer, out var provider, 15000);

            /* These tests cases regarding multipart refreshes, such as interleaving updates
             * and unsolicited refreshes. */

            /* NOTE: For now, the behavior for unsolicited refreshes in these cases is that they
             * should be treated like updates. */

            int[] consStreamIds = { 5, 6 };

            /* Request first item as a snapshot. */
            requestMsg.Clear();
            requestMsg.StreamId = consStreamIds[0];
            requestMsg.MsgClass = MsgClasses.REQUEST;
            requestMsg.DomainType = (int)DomainType.MARKET_PRICE;
            requestMsg.ContainerType = DataTypes.NO_DATA;
            requestMsg.ApplyHasQos();
            requestMsg.Qos.Timeliness(QosTimeliness.REALTIME);
            requestMsg.Qos.Rate(QosRates.TICK_BY_TICK);

            submitOptions.Clear();
            submitOptions.ServiceName = Provider.DefaultService.Info.ServiceName.ToString();
            submitOptions.RequestMsgOptions.UserSpecObj = "1111";
            Assert.True(consumer.SubmitAndDispatch((Msg)requestMsg, submitOptions) >= ReactorReturnCode.SUCCESS);

            /* Provider receives request. */
            providerReactor.Dispatch(1);
            evt = providerReactor.PollEvent();
            Assert.Equal(TestReactorEventType.MSG, evt.EventType);
            msgEvent = (ReactorMsgEvent)evt.ReactorEvent;
            Assert.Equal(MsgClasses.REQUEST, msgEvent.Msg.MsgClass);
            IRequestMsg receivedRequestMsg = (IRequestMsg)msgEvent.Msg;
            Assert.False(receivedRequestMsg.CheckStreaming());
            Assert.True(receivedRequestMsg.CheckHasPriority());
            Assert.Equal(1, receivedRequestMsg.Priority.PriorityClass);
            Assert.Equal(1, receivedRequestMsg.Priority.Count);
            Assert.False(receivedRequestMsg.CheckNoRefresh());
            int providerStreamId = receivedRequestMsg.StreamId;

            /* Request second item as streaming. */
            requestMsg.StreamId = consStreamIds[1];
            requestMsg.ApplyStreaming();

            submitOptions.Clear();
            submitOptions.ServiceName = Provider.DefaultService.Info.ServiceName.ToString();
            submitOptions.RequestMsgOptions.UserSpecObj = "2222";
            Assert.True(consumer.SubmitAndDispatch((Msg)requestMsg, submitOptions) >= ReactorReturnCode.SUCCESS);

            /* Provider receives nothing. */
            providerReactor.Dispatch(0, new TimeSpan(0, 0, 0, 0, 500));
            Assert.True(providerReactor.m_EventQueue.Count == 0);

            /* Provider sends UNSOLICTED refresh part. */
            refreshMsg.Clear();
            refreshMsg.MsgClass = MsgClasses.REFRESH;
            refreshMsg.DomainType = (int)DomainType.MARKET_PRICE;
            refreshMsg.StreamId = providerStreamId;
            refreshMsg.ContainerType = DataTypes.NO_DATA;
            refreshMsg.ApplyClearCache();
            refreshMsg.State.StreamState(StreamStates.NON_STREAMING);
            refreshMsg.State.DataState(DataStates.OK);
            requestMsg.ApplyHasQos();
            refreshMsg.Qos.Timeliness(QosTimeliness.REALTIME);
            refreshMsg.Qos.Rate(QosRates.TICK_BY_TICK);

            submitOptions.Clear();
            Assert.True(provider.SubmitAndDispatch((Msg)refreshMsg, submitOptions) >= ReactorReturnCode.SUCCESS);

            /* Consumer does NOT see this. */
            consumerReactor.Dispatch(0, new TimeSpan(0, 0, 0, 0, 500));
            Assert.True(consumerReactor.m_EventQueue.Count == 0);

            /* Provider sends refresh part. */
            refreshMsg.Clear();
            refreshMsg.MsgClass = MsgClasses.REFRESH;
            refreshMsg.DomainType = (int)DomainType.MARKET_PRICE;
            refreshMsg.StreamId = providerStreamId;
            refreshMsg.ContainerType = DataTypes.NO_DATA;
            refreshMsg.ApplyClearCache();
            refreshMsg.ApplySolicited();
            refreshMsg.State.StreamState(StreamStates.NON_STREAMING);
            refreshMsg.State.DataState(DataStates.OK);
            requestMsg.ApplyHasQos();
            refreshMsg.Qos.Timeliness(QosTimeliness.REALTIME);
            refreshMsg.Qos.Rate(QosRates.TICK_BY_TICK);

            submitOptions.Clear();
            Assert.True(provider.SubmitAndDispatch((Msg)refreshMsg, submitOptions) >= ReactorReturnCode.SUCCESS);

            /* Consumer receives refresh. */
            consumerReactor.Dispatch(1);
            evt = consumerReactor.PollEvent();
            Assert.Equal(TestReactorEventType.MSG, evt.EventType);
            msgEvent = (ReactorMsgEvent)evt.ReactorEvent;
            Assert.Equal(MsgClasses.REFRESH, msgEvent.Msg.MsgClass);
            receivedRefreshMsg = (IRefreshMsg)msgEvent.Msg;
            Assert.True(receivedRefreshMsg.CheckSolicited());
            Assert.Equal(consStreamIds[0], receivedRefreshMsg.StreamId);
            Assert.Equal((int)DomainType.MARKET_PRICE, receivedRefreshMsg.DomainType);
            Assert.Equal(DataTypes.NO_DATA, receivedRefreshMsg.ContainerType);
            Assert.Equal(StreamStates.NON_STREAMING, receivedRefreshMsg.State.StreamState());
            Assert.Equal(DataStates.OK, receivedRefreshMsg.State.DataState());
            Assert.Equal("1111", msgEvent.StreamInfo.UserSpec);

            /* Provider sends update. */
            IUpdateMsg updateMsg = new Msg();
            updateMsg.MsgClass = MsgClasses.UPDATE;
            updateMsg.StreamId = providerStreamId;
            updateMsg.DomainType = (int)DomainType.MARKET_PRICE;
            updateMsg.ContainerType = DataTypes.NO_DATA;

            submitOptions.Clear();
            Assert.True(provider.SubmitAndDispatch((Msg)updateMsg, submitOptions) >= ReactorReturnCode.SUCCESS);

            /* Consumer receives update. */
            consumerReactor.Dispatch(1);
            evt = consumerReactor.PollEvent();
            Assert.Equal(TestReactorEventType.MSG, evt.EventType);
            msgEvent = (ReactorMsgEvent)evt.ReactorEvent;
            Assert.Equal(MsgClasses.UPDATE, msgEvent.Msg.MsgClass);
            IUpdateMsg receivedUpdateMsg = (IUpdateMsg)msgEvent.Msg;
            Assert.Equal(consStreamIds[0], receivedUpdateMsg.StreamId);
            Assert.Equal("1111", msgEvent.StreamInfo.UserSpec);

            /* Provider sends refresh part. */
            refreshMsg.Clear();
            refreshMsg.MsgClass = MsgClasses.REFRESH;
            refreshMsg.DomainType = (int)DomainType.MARKET_PRICE;
            refreshMsg.StreamId = providerStreamId;
            refreshMsg.ContainerType = DataTypes.NO_DATA;
            refreshMsg.ApplySolicited();
            refreshMsg.State.StreamState(StreamStates.NON_STREAMING);
            refreshMsg.State.DataState(DataStates.OK);
            requestMsg.ApplyHasQos();
            refreshMsg.Qos.Timeliness(QosTimeliness.REALTIME);
            refreshMsg.Qos.Rate(QosRates.TICK_BY_TICK);

            submitOptions.Clear();
            Assert.True(provider.SubmitAndDispatch((Msg)refreshMsg, submitOptions) >= ReactorReturnCode.SUCCESS);

            /* Consumer receives refresh. */
            consumerReactor.Dispatch(1);
            evt = consumerReactor.PollEvent();
            Assert.Equal(TestReactorEventType.MSG, evt.EventType);
            msgEvent = (ReactorMsgEvent)evt.ReactorEvent;
            Assert.Equal(MsgClasses.REFRESH, msgEvent.Msg.MsgClass);
            receivedRefreshMsg = (IRefreshMsg)msgEvent.Msg;
            Assert.True(receivedRefreshMsg.CheckSolicited());
            Assert.Equal(consStreamIds[0], receivedRefreshMsg.StreamId);
            Assert.Equal((int)DomainType.MARKET_PRICE, receivedRefreshMsg.DomainType);
            Assert.Equal(DataTypes.NO_DATA, receivedRefreshMsg.ContainerType);
            Assert.Equal(StreamStates.NON_STREAMING, receivedRefreshMsg.State.StreamState());
            Assert.Equal(DataStates.OK, receivedRefreshMsg.State.DataState());
            Assert.Equal("1111", msgEvent.StreamInfo.UserSpec);

            /* Provider sends UNSOLICTED refresh part. */
            refreshMsg.Clear();
            refreshMsg.MsgClass = MsgClasses.REFRESH;
            refreshMsg.DomainType = (int)DomainType.MARKET_PRICE;
            refreshMsg.StreamId = providerStreamId;
            refreshMsg.ContainerType = DataTypes.NO_DATA;
            refreshMsg.State.StreamState(StreamStates.NON_STREAMING);
            refreshMsg.State.DataState(DataStates.OK);
            requestMsg.ApplyHasQos();
            refreshMsg.Qos.Timeliness(QosTimeliness.REALTIME);
            refreshMsg.Qos.Rate(QosRates.TICK_BY_TICK);

            submitOptions.Clear();
            Assert.True(provider.SubmitAndDispatch((Msg)refreshMsg, submitOptions) >= ReactorReturnCode.SUCCESS);

            /* Consumer DOES see this refresh. */
            consumerReactor.Dispatch(1);
            evt = consumerReactor.PollEvent();
            Assert.Equal(TestReactorEventType.MSG, evt.EventType);
            msgEvent = (ReactorMsgEvent)evt.ReactorEvent;
            Assert.Equal(MsgClasses.REFRESH, msgEvent.Msg.MsgClass);
            receivedRefreshMsg = (IRefreshMsg)msgEvent.Msg;
            Assert.False(receivedRefreshMsg.CheckSolicited());
            Assert.Equal(consStreamIds[0], receivedRefreshMsg.StreamId);
            Assert.Equal((int)DomainType.MARKET_PRICE, receivedRefreshMsg.DomainType);
            Assert.Equal(DataTypes.NO_DATA, receivedRefreshMsg.ContainerType);
            Assert.Equal(StreamStates.NON_STREAMING, receivedRefreshMsg.State.StreamState());
            Assert.Equal(DataStates.OK, receivedRefreshMsg.State.DataState());
            Assert.Equal("1111", msgEvent.StreamInfo.UserSpec);

            /* Provider sends final refresh part. */
            refreshMsg.Clear();
            refreshMsg.MsgClass = MsgClasses.REFRESH;
            refreshMsg.DomainType = (int)DomainType.MARKET_PRICE;
            refreshMsg.StreamId = providerStreamId;
            refreshMsg.ContainerType = DataTypes.NO_DATA;
            refreshMsg.State.StreamState(StreamStates.NON_STREAMING);
            refreshMsg.State.DataState(DataStates.OK);
            requestMsg.ApplyHasQos();
            refreshMsg.Qos.Timeliness(QosTimeliness.REALTIME);
            refreshMsg.Qos.Rate(QosRates.TICK_BY_TICK);
            refreshMsg.ApplySolicited();
            refreshMsg.ApplyRefreshComplete();

            submitOptions.Clear();
            Assert.True(provider.SubmitAndDispatch((Msg)refreshMsg, submitOptions) >= ReactorReturnCode.SUCCESS);

            /* Consumer receives refresh. */
            consumerReactor.Dispatch(1);
            evt = consumerReactor.PollEvent();
            Assert.Equal(TestReactorEventType.MSG, evt.EventType);
            msgEvent = (ReactorMsgEvent)evt.ReactorEvent;
            Assert.Equal(MsgClasses.REFRESH, msgEvent.Msg.MsgClass);
            receivedRefreshMsg = (IRefreshMsg)msgEvent.Msg;
            Assert.True(receivedRefreshMsg.CheckSolicited());
            Assert.True(receivedRefreshMsg.CheckRefreshComplete());
            Assert.Equal(consStreamIds[0], receivedRefreshMsg.StreamId);
            Assert.Equal((int)DomainType.MARKET_PRICE, receivedRefreshMsg.DomainType);
            Assert.Equal(DataTypes.NO_DATA, receivedRefreshMsg.ContainerType);
            Assert.Equal(StreamStates.NON_STREAMING, receivedRefreshMsg.State.StreamState());
            Assert.Equal(DataStates.OK, receivedRefreshMsg.State.DataState());
            Assert.Equal("1111", msgEvent.StreamInfo.UserSpec);

            /* Provider receives streaming request. */
            providerReactor.Dispatch(1);
            evt = providerReactor.PollEvent();
            Assert.Equal(TestReactorEventType.MSG, evt.EventType);
            msgEvent = (ReactorMsgEvent)evt.ReactorEvent;
            Assert.Equal(MsgClasses.REQUEST, msgEvent.Msg.MsgClass);
            receivedRequestMsg = (IRequestMsg)msgEvent.Msg;
            Assert.True(receivedRequestMsg.CheckStreaming());
            Assert.True(receivedRequestMsg.CheckHasPriority());
            Assert.Equal(1, receivedRequestMsg.Priority.PriorityClass);
            Assert.Equal(1, receivedRequestMsg.Priority.Count);
            Assert.False(receivedRequestMsg.CheckNoRefresh());
            providerStreamId = receivedRequestMsg.StreamId;

            /* Provider sends UNSOLICITED refresh part. */
            refreshMsg.Clear();
            refreshMsg.MsgClass = MsgClasses.REFRESH;
            refreshMsg.DomainType = (int)DomainType.MARKET_PRICE;
            refreshMsg.StreamId = providerStreamId;
            refreshMsg.ContainerType = DataTypes.NO_DATA;
            refreshMsg.State.StreamState(StreamStates.OPEN);
            refreshMsg.State.DataState(DataStates.OK);
            requestMsg.ApplyHasQos();
            refreshMsg.Qos.Timeliness(QosTimeliness.REALTIME);
            refreshMsg.Qos.Rate(QosRates.TICK_BY_TICK);
            refreshMsg.ApplyClearCache();

            submitOptions.Clear();
            Assert.True(provider.SubmitAndDispatch((Msg)refreshMsg, submitOptions) >= ReactorReturnCode.SUCCESS);

            /* Consumer does NOT see this since first part hasn't arrived. */
            consumerReactor.Dispatch(0, new TimeSpan(0, 0, 0, 0, 500));
            Assert.True(consumerReactor.m_EventQueue.Count == 0);

            /* Provider sends refresh part. */
            refreshMsg.Clear();
            refreshMsg.MsgClass = MsgClasses.REFRESH;
            refreshMsg.DomainType = (int)DomainType.MARKET_PRICE;
            refreshMsg.StreamId = providerStreamId;
            refreshMsg.ContainerType = DataTypes.NO_DATA;
            refreshMsg.State.StreamState(StreamStates.OPEN);
            refreshMsg.State.DataState(DataStates.OK);
            requestMsg.ApplyHasQos();
            refreshMsg.Qos.Timeliness(QosTimeliness.REALTIME);
            refreshMsg.Qos.Rate(QosRates.TICK_BY_TICK);
            refreshMsg.ApplyClearCache();
            refreshMsg.ApplySolicited();

            submitOptions.Clear();
            Assert.True(provider.SubmitAndDispatch((Msg)refreshMsg, submitOptions) >= ReactorReturnCode.SUCCESS);

            /* Consumer receives refresh. */
            consumerReactor.Dispatch(1);
            evt = consumerReactor.PollEvent();
            Assert.Equal(TestReactorEventType.MSG, evt.EventType);
            msgEvent = (ReactorMsgEvent)evt.ReactorEvent;
            Assert.Equal(MsgClasses.REFRESH, msgEvent.Msg.MsgClass);
            receivedRefreshMsg = (IRefreshMsg)msgEvent.Msg;
            Assert.True(receivedRefreshMsg.CheckSolicited());
            Assert.Equal(consStreamIds[1], receivedRefreshMsg.StreamId);
            Assert.Equal((int)DomainType.MARKET_PRICE, receivedRefreshMsg.DomainType);
            Assert.Equal(DataTypes.NO_DATA, receivedRefreshMsg.ContainerType);
            Assert.Equal(StreamStates.OPEN, receivedRefreshMsg.State.StreamState());
            Assert.Equal(DataStates.OK, receivedRefreshMsg.State.DataState());
            Assert.Equal("2222", msgEvent.StreamInfo.UserSpec);

            /* Provider sends update. */
            updateMsg.Clear();
            updateMsg.MsgClass = MsgClasses.UPDATE;
            updateMsg.StreamId = providerStreamId;
            updateMsg.DomainType = (int)DomainType.MARKET_PRICE;
            updateMsg.ContainerType = DataTypes.NO_DATA;

            submitOptions.Clear();
            Assert.True(provider.SubmitAndDispatch((Msg)updateMsg, submitOptions) >= ReactorReturnCode.SUCCESS);

            /* Consumer receives update. */
            consumerReactor.Dispatch(1);
            evt = consumerReactor.PollEvent();
            Assert.Equal(TestReactorEventType.MSG, evt.EventType);
            msgEvent = (ReactorMsgEvent)evt.ReactorEvent;
            Assert.Equal(MsgClasses.UPDATE, msgEvent.Msg.MsgClass);
            receivedUpdateMsg = (IUpdateMsg)msgEvent.Msg;
            Assert.Equal(consStreamIds[1], receivedUpdateMsg.StreamId);
            Assert.Equal("2222", msgEvent.StreamInfo.UserSpec);

            /* Provider sends UNSOLICTED refresh part. */
            refreshMsg.Clear();
            refreshMsg.MsgClass = MsgClasses.REFRESH;
            refreshMsg.DomainType = (int)DomainType.MARKET_PRICE;
            refreshMsg.StreamId = providerStreamId;
            refreshMsg.ContainerType = DataTypes.NO_DATA;
            refreshMsg.State.StreamState(StreamStates.OPEN);
            refreshMsg.State.DataState(DataStates.OK);
            requestMsg.ApplyHasQos();
            refreshMsg.Qos.Timeliness(QosTimeliness.REALTIME);
            refreshMsg.Qos.Rate(QosRates.TICK_BY_TICK);

            submitOptions.Clear();
            Assert.True(provider.SubmitAndDispatch((Msg)refreshMsg, submitOptions) >= ReactorReturnCode.SUCCESS);

            /* Consumer DOES see this refresh. */
            consumerReactor.Dispatch(1);
            evt = consumerReactor.PollEvent();
            Assert.Equal(TestReactorEventType.MSG, evt.EventType);
            msgEvent = (ReactorMsgEvent)evt.ReactorEvent;
            Assert.Equal(MsgClasses.REFRESH, msgEvent.Msg.MsgClass);
            receivedRefreshMsg = (IRefreshMsg)msgEvent.Msg;
            Assert.False(receivedRefreshMsg.CheckSolicited());
            Assert.Equal(consStreamIds[1], receivedRefreshMsg.StreamId);
            Assert.Equal((int)DomainType.MARKET_PRICE, receivedRefreshMsg.DomainType);
            Assert.Equal(DataTypes.NO_DATA, receivedRefreshMsg.ContainerType);
            Assert.Equal(StreamStates.OPEN, receivedRefreshMsg.State.StreamState());
            Assert.Equal(DataStates.OK, receivedRefreshMsg.State.DataState());
            Assert.Equal("2222", msgEvent.StreamInfo.UserSpec);

            /* Provider sends refresh part. */
            refreshMsg.Clear();
            refreshMsg.MsgClass = MsgClasses.REFRESH;
            refreshMsg.DomainType = (int)DomainType.MARKET_PRICE;
            refreshMsg.StreamId = providerStreamId;
            refreshMsg.ContainerType = DataTypes.NO_DATA;
            refreshMsg.State.StreamState(StreamStates.OPEN);
            refreshMsg.State.DataState(DataStates.OK);
            requestMsg.ApplyHasQos();
            refreshMsg.Qos.Timeliness(QosTimeliness.REALTIME);
            refreshMsg.Qos.Rate(QosRates.TICK_BY_TICK);
            refreshMsg.ApplySolicited();

            submitOptions.Clear();
            Assert.True(provider.SubmitAndDispatch((Msg)refreshMsg, submitOptions) >= ReactorReturnCode.SUCCESS);

            /* Consumer receives refresh. */
            consumerReactor.Dispatch(1);
            evt = consumerReactor.PollEvent();
            Assert.Equal(TestReactorEventType.MSG, evt.EventType);
            msgEvent = (ReactorMsgEvent)evt.ReactorEvent;
            Assert.Equal(MsgClasses.REFRESH, msgEvent.Msg.MsgClass);
            receivedRefreshMsg = (IRefreshMsg)msgEvent.Msg;
            Assert.True(receivedRefreshMsg.CheckSolicited());
            Assert.Equal(consStreamIds[1], receivedRefreshMsg.StreamId);
            Assert.Equal((int)DomainType.MARKET_PRICE, receivedRefreshMsg.DomainType);
            Assert.Equal(DataTypes.NO_DATA, receivedRefreshMsg.ContainerType);
            Assert.Equal(StreamStates.OPEN, receivedRefreshMsg.State.StreamState());
            Assert.Equal(DataStates.OK, receivedRefreshMsg.State.DataState());
            Assert.Equal("2222", msgEvent.StreamInfo.UserSpec);

            /* Provider sends final refresh part. */
            refreshMsg.Clear();
            refreshMsg.MsgClass = MsgClasses.REFRESH;
            refreshMsg.DomainType = (int)DomainType.MARKET_PRICE;
            refreshMsg.StreamId = providerStreamId;
            refreshMsg.ContainerType = DataTypes.NO_DATA;
            refreshMsg.State.StreamState(StreamStates.OPEN);
            refreshMsg.State.DataState(DataStates.OK);
            requestMsg.ApplyHasQos();
            refreshMsg.Qos.Timeliness(QosTimeliness.REALTIME);
            refreshMsg.Qos.Rate(QosRates.TICK_BY_TICK);
            refreshMsg.ApplySolicited();
            refreshMsg.ApplyRefreshComplete();

            submitOptions.Clear();
            Assert.True(provider.SubmitAndDispatch((Msg)refreshMsg, submitOptions) >= ReactorReturnCode.SUCCESS);

            /* Consumer receives refresh. */
            consumerReactor.Dispatch(1);
            evt = consumerReactor.PollEvent();
            Assert.Equal(TestReactorEventType.MSG, evt.EventType);
            msgEvent = (ReactorMsgEvent)evt.ReactorEvent;
            Assert.Equal(MsgClasses.REFRESH, msgEvent.Msg.MsgClass);
            receivedRefreshMsg = (IRefreshMsg)msgEvent.Msg;
            Assert.True(receivedRefreshMsg.CheckSolicited());
            Assert.True(receivedRefreshMsg.CheckRefreshComplete());
            Assert.Equal(consStreamIds[1], receivedRefreshMsg.StreamId);
            Assert.Equal((int)DomainType.MARKET_PRICE, receivedRefreshMsg.DomainType);
            Assert.Equal(DataTypes.NO_DATA, receivedRefreshMsg.ContainerType);
            Assert.Equal(StreamStates.OPEN, receivedRefreshMsg.State.StreamState());
            Assert.Equal(DataStates.OK, receivedRefreshMsg.State.DataState());
            Assert.Equal("2222", msgEvent.StreamInfo.UserSpec);

            /* Provider sends update. */
            updateMsg.Clear();
            updateMsg.MsgClass = MsgClasses.UPDATE;
            updateMsg.StreamId = providerStreamId;
            updateMsg.DomainType = (int)DomainType.MARKET_PRICE;
            updateMsg.ContainerType = DataTypes.NO_DATA;

            submitOptions.Clear();
            Assert.True(provider.SubmitAndDispatch((Msg)updateMsg, submitOptions) >= ReactorReturnCode.SUCCESS);

            /* Consumer receives update. */
            consumerReactor.Dispatch(1);
            evt = consumerReactor.PollEvent();
            Assert.Equal(TestReactorEventType.MSG, evt.EventType);
            msgEvent = (ReactorMsgEvent)evt.ReactorEvent;
            Assert.Equal(MsgClasses.UPDATE, msgEvent.Msg.MsgClass);
            receivedUpdateMsg = (IUpdateMsg)msgEvent.Msg;
            Assert.Equal(consStreamIds[1], receivedUpdateMsg.StreamId);
            Assert.Equal("2222", msgEvent.StreamInfo.UserSpec);

            TestReactorComponent.CloseSession(consumer, provider);
            consumerReactor.Close();
            providerReactor.Close();
        }

        [Fact]
        public void WatchlistTwoItems_RequestTimeoutTest()
        {
            ReactorSubmitOptions submitOptions = new ReactorSubmitOptions();
            TestReactorEvent evt;
            ReactorMsgEvent msgEvent;

            TestReactor consumerReactor = new TestReactor();
            TestReactor providerReactor = new TestReactor();

            Consumer consumer = new Consumer(consumerReactor);
            ConsumerRole consumerRole = (ConsumerRole)consumer.ReactorRole;
            consumerRole.ChannelEventCallback = consumer;
            consumerRole.LoginMsgCallback = consumer;
            consumerRole.DirectoryMsgCallback = consumer;
            consumerRole.DictionaryMsgCallback = consumer;
            consumerRole.DefaultMsgCallback = consumer;
            consumerRole.WatchlistOptions.EnableWatchlist = true;
            consumerRole.WatchlistOptions.ChannelOpenEventCallback = consumer;
            consumerRole.WatchlistOptions.RequestTimeout = 3000;

            Provider provider = new Provider(providerReactor);
            ProviderRole providerRole = (ProviderRole)provider.ReactorRole;
            providerRole.ChannelEventCallback = provider;
            providerRole.LoginMsgCallback = provider;
            providerRole.DirectoryMsgCallback = provider;
            providerRole.DictionaryMsgCallback = provider;
            providerRole.DefaultMsgCallback = provider;

            ConsumerProviderSessionOptions opts = new ConsumerProviderSessionOptions();
            opts.SetupDefaultLoginStream = false;
            opts.SetupDefaultDirectoryStream = false;
            opts.ReconnectAttemptLimit = 3;

            provider.Bind(opts);

            RDMLoginMsgEvent loginMsgEvent;
            RDMDirectoryMsgEvent directoryMsgEvent;

            consumer.TestReactor.Connect(opts, consumer, provider.ServerPort);
            consumer.TestReactor.PollEvent();

            provider.TestReactor.Accept(opts, provider);
            provider.TestReactor.Dispatch(2);
            provider.TestReactor.PollEvent();
            provider.TestReactor.PollEvent();

            consumer.TestReactor.Dispatch(2);
            consumer.TestReactor.PollEvent();
            consumer.TestReactor.PollEvent();

            provider.TestReactor.Dispatch(0);

            LoginRequest loginRequest = new LoginRequest();
            loginRequest.InitDefaultRequest(1);
            loginRequest.HasAttrib = true;
            loginRequest.LoginAttrib.HasSingleOpen = true;
            loginRequest.LoginAttrib.SingleOpen = 1;

            submitOptions.Clear();
            Assert.True(consumer.SubmitAndDispatch(loginRequest, submitOptions) >= ReactorReturnCode.SUCCESS);

            provider.TestReactor.Dispatch(1); // Provider receives login request but doesn't respond
            provider.TestReactor.PollEvent();

            /*** Test login request timeout. ***/
            /* Wait for login response. Consumer should get open/suspect after a couple seconds. */
            consumer.TestReactor.Dispatch(1, new TimeSpan(0, 0, 5));

            evt = consumer.TestReactor.PollEvent();
            Assert.Equal(TestReactorEventType.LOGIN_MSG, evt.EventType);
            loginMsgEvent = (RDMLoginMsgEvent)evt.ReactorEvent;
            Assert.Equal(LoginMsgType.STATUS, loginMsgEvent.LoginMsg.LoginMsgType);
            Assert.Equal(1, loginMsgEvent.LoginMsg.StreamId);
            Assert.True(loginMsgEvent.LoginMsg.LoginStatus.HasState);
            Assert.Equal(StreamStates.OPEN, loginMsgEvent.LoginMsg.LoginStatus.State.StreamState());
            Assert.Equal(DataStates.SUSPECT, loginMsgEvent.LoginMsg.LoginStatus.State.DataState());
            Assert.Equal(StateCodes.TIMEOUT, loginMsgEvent.LoginMsg.LoginStatus.State.Code());

            provider.TestReactor.Dispatch(2, new TimeSpan(0, 0, 5));

            evt = provider.TestReactor.PollEvent();
            Assert.Equal(TestReactorEventType.LOGIN_MSG, evt.EventType);
            loginMsgEvent = (RDMLoginMsgEvent)evt.ReactorEvent;
            Assert.Equal(LoginMsgType.CLOSE, loginMsgEvent.LoginMsg.LoginMsgType);

            evt = provider.TestReactor.PollEvent();
            Assert.Equal(TestReactorEventType.LOGIN_MSG, evt.EventType);
            loginMsgEvent = (RDMLoginMsgEvent)evt.ReactorEvent;
            Assert.Equal(LoginMsgType.REQUEST, loginMsgEvent.LoginMsg.LoginMsgType);

            LoginRefresh loginRefresh = new LoginRefresh();

            loginRefresh.Clear();
            loginRefresh.Solicited = true;
            loginRefresh.UserName = loginRequest.UserName;
            loginRefresh.StreamId = loginRequest.StreamId;
            loginRefresh.HasFeatures = true;
            loginRefresh.SupportedFeatures.HasSupportOptimizedPauseResume = true;
            loginRefresh.SupportedFeatures.SupportOptimizedPauseResume = 1;
            loginRefresh.SupportedFeatures.HasSupportViewRequests = true;
            loginRefresh.SupportedFeatures.SupportViewRequests = 1;
            loginRefresh.SupportedFeatures.HasSupportPost = true;
            loginRefresh.SupportedFeatures.SupportOMMPost = 1;

            loginRefresh.SupportedFeatures.HasSupportEnhancedSymbolList = true;
            loginRefresh.SupportedFeatures.SupportEnhancedSymbolList = 1;

            loginRefresh.State.StreamState(StreamStates.OPEN);
            loginRefresh.State.DataState(DataStates.OK);
            loginRefresh.State.Code(StateCodes.NONE);
            loginRefresh.State.Text().Data("Login OK");

            submitOptions.Clear();
            Assert.True(provider.SubmitAndDispatch(loginRefresh, submitOptions) >= ReactorReturnCode.SUCCESS);

            consumer.TestReactor.Dispatch(1);

            evt = consumer.TestReactor.PollEvent();
            Assert.Equal(TestReactorEventType.LOGIN_MSG, evt.EventType);

            /*** Test directory request timeout. ***/
            /* Provider receives directory request. */
            provider.TestReactor.Dispatch(1);
            evt = provider.TestReactor.PollEvent();
            Assert.Equal(TestReactorEventType.DIRECTORY_MSG, evt.EventType);
            directoryMsgEvent = (RDMDirectoryMsgEvent)evt.ReactorEvent;
            Assert.Equal(DirectoryMsgType.REQUEST, directoryMsgEvent.DirectoryMsg.DirectoryMsgType);

            /* Wait for directory response. Consumer should get nothing. */
            consumer.TestReactor.Dispatch(0, new TimeSpan(0, 0, 5));

            provider.TestReactor.Dispatch(2);
            evt = provider.TestReactor.PollEvent();
            Assert.Equal(TestReactorEventType.DIRECTORY_MSG, evt.EventType);
            directoryMsgEvent = (RDMDirectoryMsgEvent)evt.ReactorEvent;
            Assert.Equal(DirectoryMsgType.CLOSE, directoryMsgEvent.DirectoryMsg.DirectoryMsgType);

            evt = provider.TestReactor.PollEvent();
            Assert.Equal(TestReactorEventType.DIRECTORY_MSG, evt.EventType);
            directoryMsgEvent = (RDMDirectoryMsgEvent)evt.ReactorEvent;
            Assert.Equal(DirectoryMsgType.REQUEST, directoryMsgEvent.DirectoryMsg.DirectoryMsgType);

            DirectoryRefresh directoryRefresh = new DirectoryRefresh();

            directoryRefresh.Clear();
            directoryRefresh.StreamId = 2;
            directoryRefresh.Filter = ALL_FILTERS;
            directoryRefresh.Solicited = true;
            directoryRefresh.ClearCache = true;
            directoryRefresh.State.StreamState(StreamStates.OPEN);
            directoryRefresh.State.DataState(DataStates.OK);
            directoryRefresh.State.Code(StateCodes.NONE);
            directoryRefresh.State.Text().Data("Source Directory Refresh Complete");

            Service service = new();
            Provider.DefaultService.Copy(service);
            directoryRefresh.ServiceList.Add(service);

            submitOptions.Clear();
            Assert.True(provider.SubmitAndDispatch(directoryRefresh, submitOptions) >= ReactorReturnCode.SUCCESS);

            consumer.TestReactor.Dispatch(0);

            /*** Test item request timeout ***/
            /* Consumer sends streaming request. */
            IRequestMsg requestMsg = new Msg();
            requestMsg.Clear();
            requestMsg.MsgClass = MsgClasses.REQUEST;
            requestMsg.StreamId = 5;
            requestMsg.DomainType = (int)DomainType.MARKET_PRICE;
            requestMsg.ApplyStreaming();
            requestMsg.MsgKey.ApplyHasName();
            requestMsg.MsgKey.Name.Data("TRI.N");
            requestMsg.ApplyHasQos();
            requestMsg.Qos.Timeliness(QosTimeliness.REALTIME);
            requestMsg.Qos.Rate(QosRates.TICK_BY_TICK);
            submitOptions.Clear();
            submitOptions.ServiceName = Provider.DefaultService.Info.ServiceName.ToString();
            Assert.True(consumer.SubmitAndDispatch((Msg)requestMsg, submitOptions) >= ReactorReturnCode.SUCCESS);

            /* Provider receives request and sends nothing */
            providerReactor.Dispatch(1);
            evt = providerReactor.PollEvent();

            Assert.Equal(TestReactorEventType.MSG, evt.EventType);
            msgEvent = (ReactorMsgEvent)evt.ReactorEvent;
            Assert.Equal(MsgClasses.REQUEST, msgEvent.Msg.MsgClass);
            int providerStreamId = msgEvent.Msg.StreamId;

            /* Consumer sends streaming request. */
            requestMsg.Clear();
            requestMsg.MsgClass = MsgClasses.REQUEST;
            requestMsg.StreamId = 6;
            requestMsg.DomainType = (int)DomainType.MARKET_PRICE;
            requestMsg.ApplyStreaming();
            requestMsg.MsgKey.ApplyHasName();
            requestMsg.MsgKey.Name.Data("TRI.N");
            requestMsg.ApplyHasQos();
            requestMsg.Qos.Timeliness(QosTimeliness.REALTIME);
            requestMsg.Qos.Rate(QosRates.TICK_BY_TICK);
            submitOptions.Clear();
            submitOptions.ServiceName = Provider.DefaultService.Info.ServiceName.ToString();
            Assert.True(consumer.SubmitAndDispatch((Msg)requestMsg, submitOptions) >= ReactorReturnCode.SUCCESS);

            /* Provider receives priority change */
            providerReactor.Dispatch(1);
            providerReactor.PollEvent();

            /* Wait for item refresh. Consumer should get Open/Suspect after a couple seconds. */
            consumer.TestReactor.Dispatch(2, new TimeSpan(0, 0, 5));

            evt = consumer.TestReactor.PollEvent();
            Assert.Equal(TestReactorEventType.MSG, evt.EventType);
            msgEvent = (ReactorMsgEvent)evt.ReactorEvent;
            Assert.Equal(5, msgEvent.Msg.StreamId);
            Assert.Equal(MsgClasses.STATUS, msgEvent.Msg.MsgClass);
            IStatusMsg statusMsg = (IStatusMsg)msgEvent.Msg;
            Assert.Equal(StreamStates.OPEN, statusMsg.State.StreamState());
            Assert.Equal(DataStates.SUSPECT, statusMsg.State.DataState());

            evt = consumer.TestReactor.PollEvent();
            Assert.Equal(TestReactorEventType.MSG, evt.EventType);
            msgEvent = (ReactorMsgEvent)evt.ReactorEvent;
            Assert.Equal(6, msgEvent.Msg.StreamId);
            Assert.Equal(MsgClasses.STATUS, msgEvent.Msg.MsgClass);
            statusMsg = (IStatusMsg)msgEvent.Msg;
            Assert.Equal(StreamStates.OPEN, statusMsg.State.StreamState());
            Assert.Equal(DataStates.SUSPECT, statusMsg.State.DataState());

            /* Provider receives CloseMsg and a new RequestMsg. */
            providerReactor.Dispatch(2);

            evt = providerReactor.PollEvent();
            Assert.Equal(TestReactorEventType.MSG, evt.EventType);
            msgEvent = (ReactorMsgEvent)evt.ReactorEvent;
            Assert.Equal(providerStreamId, msgEvent.Msg.StreamId);
            Assert.Equal(MsgClasses.CLOSE, msgEvent.Msg.MsgClass);

            evt = providerReactor.PollEvent();
            Assert.Equal(TestReactorEventType.MSG, evt.EventType);
            msgEvent = (ReactorMsgEvent)evt.ReactorEvent;
            Assert.Equal(MsgClasses.REQUEST, msgEvent.Msg.MsgClass);
            requestMsg = (IRequestMsg)msgEvent.Msg;
            Assert.True(requestMsg.CheckHasPriority());
            Assert.Equal(1, requestMsg.Priority.PriorityClass);
            Assert.Equal(2, requestMsg.Priority.Count);

            TestReactorComponent.CloseSession(consumer, provider);
            consumerReactor.Close();
            providerReactor.Close();
        }

        [Fact]
        public void WatchlistReissuePriorityChangeAndNewRefreshTest()
        {
            ReactorSubmitOptions submitOptions = new();
            TestReactorEvent evt;
            ReactorMsgEvent msgEvent;
            Msg msg = new();
            IRequestMsg requestMsg = msg;
            IRefreshMsg refreshMsg = msg;

            /* Connect the consumer and provider. Setup login & directory streams automatically. */
            ConsumerProviderSessionOptions opts = new();
            opts.SetupDefaultLoginStream = true;
            opts.SetupDefaultDirectoryStream = true;

            SetupSession(opts, false, 0, false, 0, out var consumerReactor, out var providerReactor, out var consumer, out var provider, 15000);

            int[] consStreamIds = { 5, 6 };

            /* Consumer requests item. */
            requestMsg.Clear();
            requestMsg.StreamId = consStreamIds[0];
            requestMsg.MsgClass = MsgClasses.REQUEST;
            requestMsg.DomainType = (int)DomainType.MARKET_PRICE;
            requestMsg.ContainerType = DataTypes.NO_DATA;
            requestMsg.ApplyStreaming();
            requestMsg.ApplyHasQos();
            requestMsg.Qos.Timeliness(QosTimeliness.REALTIME);
            requestMsg.Qos.Rate(QosRates.TICK_BY_TICK);

            submitOptions.Clear();
            submitOptions.ServiceName = Provider.DefaultService.Info.ServiceName.ToString();
            submitOptions.RequestMsgOptions.UserSpecObj = "1111";
            Assert.True(consumer.SubmitAndDispatch((Msg)requestMsg, submitOptions) >= ReactorReturnCode.SUCCESS);

            /* Provider receives request. */
            providerReactor.Dispatch(1);
            evt = providerReactor.PollEvent();
            Assert.Equal(TestReactorEventType.MSG, evt.EventType);
            msgEvent = (ReactorMsgEvent)evt.ReactorEvent;
            Assert.Equal(MsgClasses.REQUEST, msgEvent.Msg.MsgClass);
            IRequestMsg receivedRequestMsg = (IRequestMsg)msgEvent.Msg;
            Assert.True(receivedRequestMsg.CheckStreaming());
            Assert.True(receivedRequestMsg.CheckHasPriority());
            Assert.Equal(1, receivedRequestMsg.Priority.PriorityClass);
            Assert.Equal(1, receivedRequestMsg.Priority.Count);
            Assert.False(receivedRequestMsg.CheckNoRefresh());
            int providerStreamId = receivedRequestMsg.StreamId;

            /* Consumer reissues item. */
            requestMsg.Clear();
            requestMsg.StreamId = consStreamIds[0];
            requestMsg.MsgClass = MsgClasses.REQUEST;
            requestMsg.DomainType = (int)DomainType.MARKET_PRICE;
            requestMsg.ContainerType = DataTypes.NO_DATA;
            requestMsg.ApplyStreaming();
            requestMsg.ApplyHasQos();
            requestMsg.ApplyNoRefresh();
            requestMsg.Qos.Timeliness(QosTimeliness.REALTIME);
            requestMsg.Qos.Rate(QosRates.TICK_BY_TICK);
            requestMsg.ApplyHasPriority();
            requestMsg.Priority.PriorityClass = 5;
            requestMsg.Priority.Count = 6;

            submitOptions.Clear();
            submitOptions.ServiceName = Provider.DefaultService.Info.ServiceName.ToString();
            Assert.True(consumer.SubmitAndDispatch((Msg)requestMsg, submitOptions) >= ReactorReturnCode.SUCCESS);

            /* Provider receives request -- same stream, no refresh, updated priority. */
            providerReactor.Dispatch(1);
            evt = providerReactor.PollEvent();
            Assert.Equal(TestReactorEventType.MSG, evt.EventType);
            msgEvent = (ReactorMsgEvent)evt.ReactorEvent;
            Assert.Equal(MsgClasses.REQUEST, msgEvent.Msg.MsgClass);
            receivedRequestMsg = (IRequestMsg)msgEvent.Msg;
            Assert.True(receivedRequestMsg.CheckStreaming());
            Assert.True(receivedRequestMsg.CheckHasPriority());
            Assert.Equal(5, receivedRequestMsg.Priority.PriorityClass);
            Assert.Equal(6, receivedRequestMsg.Priority.Count);
            Assert.True(receivedRequestMsg.CheckNoRefresh());
            Assert.Equal(providerStreamId, receivedRequestMsg.StreamId);

            /* Provider sends refresh. */
            refreshMsg.Clear();
            refreshMsg.MsgClass = MsgClasses.REFRESH;
            refreshMsg.DomainType = (int)DomainType.MARKET_PRICE;
            refreshMsg.StreamId = providerStreamId;
            refreshMsg.ContainerType = DataTypes.NO_DATA;
            refreshMsg.ApplyRefreshComplete();
            refreshMsg.ApplySolicited();
            refreshMsg.ApplyClearCache();
            refreshMsg.State.StreamState(StreamStates.OPEN);
            refreshMsg.State.DataState(DataStates.OK);
            requestMsg.ApplyHasQos();
            refreshMsg.Qos.Timeliness(QosTimeliness.REALTIME);
            refreshMsg.Qos.Rate(QosRates.TICK_BY_TICK);

            submitOptions.Clear();
            Assert.True(provider.SubmitAndDispatch((Msg)refreshMsg, submitOptions) >= ReactorReturnCode.SUCCESS);

            /* Consumer receives refresh. */
            consumerReactor.Dispatch(1);
            evt = consumerReactor.PollEvent();
            Assert.Equal(TestReactorEventType.MSG, evt.EventType);
            msgEvent = (ReactorMsgEvent)evt.ReactorEvent;
            Assert.Equal(MsgClasses.REFRESH, msgEvent.Msg.MsgClass);
            IRefreshMsg receivedRefreshMsg = (IRefreshMsg)msgEvent.Msg;
            Assert.Equal((int)DomainType.MARKET_PRICE, receivedRefreshMsg.DomainType);
            Assert.Equal(DataTypes.NO_DATA, receivedRefreshMsg.ContainerType);
            Assert.Equal(StreamStates.OPEN, receivedRefreshMsg.State.StreamState());
            Assert.Equal(DataStates.OK, receivedRefreshMsg.State.DataState());
            Assert.Equal(consStreamIds[0], receivedRefreshMsg.StreamId);

            /* Consumer reissues with new image. */
            requestMsg.Clear();
            requestMsg.StreamId = consStreamIds[0];
            requestMsg.MsgClass = MsgClasses.REQUEST;
            requestMsg.DomainType = (int)DomainType.MARKET_PRICE;
            requestMsg.ContainerType = DataTypes.NO_DATA;
            requestMsg.ApplyStreaming();
            requestMsg.ApplyHasQos();
            requestMsg.Qos.Timeliness(QosTimeliness.REALTIME);
            requestMsg.Qos.Rate(QosRates.TICK_BY_TICK);

            submitOptions.Clear();
            submitOptions.ServiceName = Provider.DefaultService.Info.ServiceName.ToString();
            Assert.True(consumer.SubmitAndDispatch((Msg)requestMsg, submitOptions) >= ReactorReturnCode.SUCCESS);

            /* Provider receives request. */
            providerReactor.Dispatch(1);
            evt = providerReactor.PollEvent();
            Assert.Equal(TestReactorEventType.MSG, evt.EventType);
            msgEvent = (ReactorMsgEvent)evt.ReactorEvent;
            Assert.Equal(MsgClasses.REQUEST, msgEvent.Msg.MsgClass);
            receivedRequestMsg = (IRequestMsg)msgEvent.Msg;
            Assert.True(receivedRequestMsg.CheckStreaming());
            Assert.False(receivedRequestMsg.CheckHasPriority());
            Assert.False(receivedRequestMsg.CheckNoRefresh());
            Assert.Equal(providerStreamId, receivedRequestMsg.StreamId);

            /* Provider sends refresh. */
            refreshMsg.Clear();
            refreshMsg.MsgClass = MsgClasses.REFRESH;
            refreshMsg.DomainType = (int)DomainType.MARKET_PRICE;
            refreshMsg.StreamId = providerStreamId;
            refreshMsg.ContainerType = DataTypes.NO_DATA;
            refreshMsg.ApplyRefreshComplete();
            refreshMsg.ApplySolicited();
            refreshMsg.ApplyClearCache();
            refreshMsg.State.StreamState(StreamStates.OPEN);
            refreshMsg.State.DataState(DataStates.OK);
            requestMsg.ApplyHasQos();
            refreshMsg.Qos.Timeliness(QosTimeliness.REALTIME);
            refreshMsg.Qos.Rate(QosRates.TICK_BY_TICK);

            submitOptions.Clear();
            Assert.True(provider.SubmitAndDispatch((Msg)refreshMsg, submitOptions) >= ReactorReturnCode.SUCCESS);

            /* Consumer receives refresh. */
            consumerReactor.Dispatch(1);
            evt = consumerReactor.PollEvent();
            Assert.Equal(TestReactorEventType.MSG, evt.EventType);
            msgEvent = (ReactorMsgEvent)evt.ReactorEvent;
            Assert.Equal(MsgClasses.REFRESH, msgEvent.Msg.MsgClass);
            receivedRefreshMsg = (IRefreshMsg)msgEvent.Msg;
            Assert.Equal((int)DomainType.MARKET_PRICE, receivedRefreshMsg.DomainType);
            Assert.Equal(DataTypes.NO_DATA, receivedRefreshMsg.ContainerType);
            Assert.Equal(StreamStates.OPEN, receivedRefreshMsg.State.StreamState());
            Assert.Equal(DataStates.OK, receivedRefreshMsg.State.DataState());
            Assert.Equal(consStreamIds[0], receivedRefreshMsg.StreamId);

            /* Consumer closes item. */
            ICloseMsg closeMsg = msg;
            closeMsg.MsgClass = MsgClasses.CLOSE;
            closeMsg.StreamId = consStreamIds[0];

            submitOptions.Clear();
            Assert.True(consumer.SubmitAndDispatch((Msg)closeMsg, submitOptions) >= ReactorReturnCode.SUCCESS);

            /* Provider receives close. */
            providerReactor.Dispatch(1);
            evt = providerReactor.PollEvent();
            Assert.Equal(TestReactorEventType.MSG, evt.EventType);
            msgEvent = (ReactorMsgEvent)evt.ReactorEvent;
            Assert.Equal(MsgClasses.CLOSE, msgEvent.Msg.MsgClass);
            ICloseMsg receivedCloseMsg = (ICloseMsg)msgEvent.Msg;
            Assert.Equal(providerStreamId, receivedCloseMsg.StreamId);

            TestReactorComponent.CloseSession(consumer, provider);
            consumerReactor.Close();
            providerReactor.Close();
        }

        [Fact]
        public void WatchlistTwoItems_FieldViewFromMsgBufferTest()
        {
            ReactorSubmitOptions submitOptions = new();
            TestReactorEvent evt;
            ReactorMsgEvent msgEvent;
            Msg msg = new();
            IRequestMsg requestMsg = msg;
            IRefreshMsg refreshMsg = msg;

            /* Connect the consumer and provider. Setup login & directory streams automatically. */
            ConsumerProviderSessionOptions opts = new();
            opts.SetupDefaultLoginStream = true;
            opts.SetupDefaultDirectoryStream = true;

            SetupSession(opts, false, 0, false, 0, out var consumerReactor, out var providerReactor, out var consumer, out var provider, 15000);

            List<int> view1List = new List<int> { 2, 6, 7, 2 };
            List<int> view1_1List = new List<int> { 3, 22 };
            List<int> view2List = new List<int> { 6, 7 };

            List<int> providerView1List = new List<int> { 2, 6, 7 };
            List<int> providerView1_1List = new List<int> { 3, 22 };
            List<int> providerView2List = new List<int> { 6, 7 };
            List<int> combinedViewList = new List<int> { 3, 6, 7, 22 };

            int[] consStreamIds = { 5, 6 };

            /* Request first item. */
            requestMsg.Clear();
            requestMsg.StreamId = consStreamIds[0];
            requestMsg.MsgClass = MsgClasses.REQUEST;
            requestMsg.DomainType = (int)DomainType.MARKET_PRICE;
            requestMsg.ContainerType = DataTypes.NO_DATA;
            requestMsg.ApplyStreaming();
            requestMsg.ApplyHasQos();
            requestMsg.Qos.Timeliness(QosTimeliness.REALTIME);
            requestMsg.Qos.Rate(QosRates.TICK_BY_TICK);
            requestMsg.ApplyHasView();
            WatchlistItemDomainsTest.EncodeViewFieldIdList(consumer.ReactorChannel, view1List, requestMsg);

            submitOptions.Clear();
            submitOptions.ServiceName = Provider.DefaultService.Info.ServiceName.ToString();
            submitOptions.RequestMsgOptions.UserSpecObj = "1111";
            Assert.True(consumer.SubmitAndDispatch((Msg)requestMsg, submitOptions) >= ReactorReturnCode.SUCCESS);

            /* Provider receives request. */
            providerReactor.Dispatch(1);
            evt = providerReactor.PollEvent();
            Assert.Equal(TestReactorEventType.MSG, evt.EventType);
            msgEvent = (ReactorMsgEvent)evt.ReactorEvent;
            Assert.Equal(MsgClasses.REQUEST, msgEvent.Msg.MsgClass);
            IRequestMsg receivedRequestMsg = (IRequestMsg)msgEvent.Msg;
            Assert.True(receivedRequestMsg.CheckStreaming());
            Assert.True(receivedRequestMsg.CheckHasPriority());
            Assert.Equal(1, receivedRequestMsg.Priority.PriorityClass);
            Assert.Equal(1, receivedRequestMsg.Priority.Count);
            Assert.False(receivedRequestMsg.CheckNoRefresh());
            Assert.True(WatchlistItemDomainsTest.CheckHasCorrectView(provider, receivedRequestMsg, providerView1List));
            int providerStreamId = receivedRequestMsg.StreamId;

            /* Provider sends refresh, satisfying first view. */
            refreshMsg.Clear();
            refreshMsg.MsgClass = MsgClasses.REFRESH;
            refreshMsg.DomainType = (int)DomainType.MARKET_PRICE;
            refreshMsg.StreamId = providerStreamId;
            refreshMsg.ContainerType = DataTypes.NO_DATA;
            refreshMsg.ApplyRefreshComplete();
            refreshMsg.ApplySolicited();
            refreshMsg.ApplyClearCache();
            refreshMsg.State.StreamState(StreamStates.OPEN);
            refreshMsg.State.DataState(DataStates.OK);
            requestMsg.ApplyHasQos();
            refreshMsg.Qos.Timeliness(QosTimeliness.REALTIME);
            refreshMsg.Qos.Rate(QosRates.TICK_BY_TICK);

            submitOptions.Clear();
            Assert.True(provider.SubmitAndDispatch((Msg)refreshMsg, submitOptions) >= ReactorReturnCode.SUCCESS);

            /* Consumer receives refresh. */
            consumerReactor.Dispatch(1);
            evt = consumerReactor.PollEvent();
            Assert.Equal(TestReactorEventType.MSG, evt.EventType);
            msgEvent = (ReactorMsgEvent)evt.ReactorEvent;
            Assert.Equal(MsgClasses.REFRESH, msgEvent.Msg.MsgClass);
            IRefreshMsg receivedRefreshMsg = (IRefreshMsg)msgEvent.Msg;
            Assert.Equal((int)DomainType.MARKET_PRICE, receivedRefreshMsg.DomainType);
            Assert.Equal(DataTypes.NO_DATA, receivedRefreshMsg.ContainerType);
            Assert.Equal(StreamStates.OPEN, receivedRefreshMsg.State.StreamState());
            Assert.Equal(DataStates.OK, receivedRefreshMsg.State.DataState());
            Assert.Equal(consStreamIds[0], receivedRefreshMsg.StreamId);

            /* Reissue first item with new view. */
            requestMsg.Clear();
            requestMsg.StreamId = consStreamIds[0];
            requestMsg.MsgClass = MsgClasses.REQUEST;
            requestMsg.DomainType = (int)DomainType.MARKET_PRICE;
            requestMsg.ContainerType = DataTypes.NO_DATA;
            requestMsg.ApplyStreaming();
            requestMsg.ApplyHasQos();
            requestMsg.Qos.Timeliness(QosTimeliness.REALTIME);
            requestMsg.Qos.Rate(QosRates.TICK_BY_TICK);
            requestMsg.ApplyHasView();
            WatchlistItemDomainsTest.EncodeViewFieldIdList(consumer.ReactorChannel, view1_1List, requestMsg);

            submitOptions.Clear();
            submitOptions.ServiceName = Provider.DefaultService.Info.ServiceName.ToString();
            submitOptions.RequestMsgOptions.UserSpecObj = "1111";
            Assert.True(consumer.SubmitAndDispatch((Msg)requestMsg, submitOptions) >= ReactorReturnCode.SUCCESS);

            /* Provider receives request. */
            providerReactor.Dispatch(1);
            evt = providerReactor.PollEvent();
            Assert.Equal(TestReactorEventType.MSG, evt.EventType);
            msgEvent = (ReactorMsgEvent)evt.ReactorEvent;
            Assert.Equal(MsgClasses.REQUEST, msgEvent.Msg.MsgClass);
            receivedRequestMsg = (IRequestMsg)msgEvent.Msg;
            Assert.True(receivedRequestMsg.CheckStreaming());
            Assert.False(receivedRequestMsg.CheckHasPriority());
            Assert.False(receivedRequestMsg.CheckNoRefresh());
            Assert.Equal(providerStreamId, receivedRequestMsg.StreamId);
            Assert.True(WatchlistItemDomainsTest.CheckHasCorrectView(provider, receivedRequestMsg, providerView1_1List));

            /* Request second item. */
            requestMsg.StreamId = consStreamIds[1];
            WatchlistItemDomainsTest.EncodeViewFieldIdList(consumer.ReactorChannel, view2List, requestMsg);

            submitOptions.Clear();
            submitOptions.ServiceName = Provider.DefaultService.Info.ServiceName.ToString();
            submitOptions.RequestMsgOptions.UserSpecObj = "2222";
            Assert.True(consumer.SubmitAndDispatch((Msg)requestMsg, submitOptions) >= ReactorReturnCode.SUCCESS);

            /* Second request shouldn't arrive yet. */
            providerReactor.Dispatch(0, new TimeSpan(0, 0, 0, 0, 500));
            Assert.True(0 == providerReactor.m_EventQueue.Count);

            /* Provider sends refresh, satisfying first view. */
            refreshMsg.Clear();
            refreshMsg.MsgClass = MsgClasses.REFRESH;
            refreshMsg.DomainType = (int)DomainType.MARKET_PRICE;
            refreshMsg.StreamId = providerStreamId;
            refreshMsg.ContainerType = DataTypes.NO_DATA;
            refreshMsg.ApplyRefreshComplete();
            refreshMsg.ApplySolicited();
            refreshMsg.ApplyClearCache();
            refreshMsg.State.StreamState(StreamStates.OPEN);
            refreshMsg.State.DataState(DataStates.OK);
            requestMsg.ApplyHasQos();
            refreshMsg.Qos.Timeliness(QosTimeliness.REALTIME);
            refreshMsg.Qos.Rate(QosRates.TICK_BY_TICK);

            submitOptions.Clear();
            Assert.True(provider.SubmitAndDispatch((Msg)refreshMsg, submitOptions) >= ReactorReturnCode.SUCCESS);

            /* Consumer receives refresh. */
            consumerReactor.Dispatch(1);
            evt = consumerReactor.PollEvent();
            Assert.Equal(TestReactorEventType.MSG, evt.EventType);
            msgEvent = (ReactorMsgEvent)evt.ReactorEvent;
            Assert.Equal(MsgClasses.REFRESH, msgEvent.Msg.MsgClass);
            receivedRefreshMsg = (IRefreshMsg)msgEvent.Msg;
            Assert.Equal((int)DomainType.MARKET_PRICE, receivedRefreshMsg.DomainType);
            Assert.Equal(DataTypes.NO_DATA, receivedRefreshMsg.ContainerType);
            Assert.Equal(StreamStates.OPEN, receivedRefreshMsg.State.StreamState());
            Assert.Equal(DataStates.OK, receivedRefreshMsg.State.DataState());
            Assert.Equal(consStreamIds[0], receivedRefreshMsg.StreamId);

            /* Provider receives request -- same stream, need refresh, updated priority. */
            providerReactor.Dispatch(1);
            evt = providerReactor.PollEvent();
            Assert.Equal(TestReactorEventType.MSG, evt.EventType);
            msgEvent = (ReactorMsgEvent)evt.ReactorEvent;
            Assert.Equal(MsgClasses.REQUEST, msgEvent.Msg.MsgClass);
            receivedRequestMsg = (IRequestMsg)msgEvent.Msg;
            Assert.True(receivedRequestMsg.CheckStreaming());
            Assert.True(receivedRequestMsg.CheckHasPriority());
            Assert.Equal(1, receivedRequestMsg.Priority.PriorityClass);
            Assert.Equal(2, receivedRequestMsg.Priority.Count);
            Assert.False(receivedRequestMsg.CheckNoRefresh());
            Assert.Equal(providerStreamId, receivedRequestMsg.StreamId);
            Assert.True(receivedRequestMsg.CheckHasView());
            Assert.True(WatchlistItemDomainsTest.CheckHasCorrectView(provider, receivedRequestMsg, combinedViewList));

            /* Close first item. */
            ICloseMsg closeMsg = new Msg();
            closeMsg.MsgClass = MsgClasses.CLOSE;
            closeMsg.StreamId = consStreamIds[0];
            closeMsg.DomainType = (int)DomainType.MARKET_PRICE;

            submitOptions.Clear();
            Assert.True(consumer.SubmitAndDispatch((Msg)closeMsg, submitOptions) >= ReactorReturnCode.SUCCESS);

            /* Provider receives priority and view changes as the watchlist hasn't received the previous view. */
            providerReactor.Dispatch(1);
            evt = providerReactor.PollEvent();
            Assert.Equal(TestReactorEventType.MSG, evt.EventType);
            msgEvent = (ReactorMsgEvent)evt.ReactorEvent;
            Assert.Equal(MsgClasses.REQUEST, msgEvent.Msg.MsgClass);
            receivedRequestMsg = (IRequestMsg)msgEvent.Msg;
            Assert.True(receivedRequestMsg.CheckStreaming());
            Assert.True(receivedRequestMsg.CheckHasPriority());
            Assert.Equal(1, receivedRequestMsg.Priority.PriorityClass);
            Assert.Equal(1, receivedRequestMsg.Priority.Count);
            Assert.False(receivedRequestMsg.CheckNoRefresh());
            Assert.True(receivedRequestMsg.CheckHasView());
            Assert.True(WatchlistItemDomainsTest.CheckHasCorrectView(provider, receivedRequestMsg, providerView2List));
            Assert.Equal(providerStreamId, receivedRequestMsg.StreamId);

            /* Provider sends refresh, satisfying second view. */
            refreshMsg.Clear();
            refreshMsg.MsgClass = MsgClasses.REFRESH;
            refreshMsg.DomainType = (int)DomainType.MARKET_PRICE;
            refreshMsg.StreamId = providerStreamId;
            refreshMsg.ContainerType = DataTypes.NO_DATA;
            refreshMsg.ApplyRefreshComplete();
            refreshMsg.ApplySolicited();
            refreshMsg.ApplyClearCache();
            refreshMsg.State.StreamState(StreamStates.OPEN);
            refreshMsg.State.DataState(DataStates.OK);
            requestMsg.ApplyHasQos();
            refreshMsg.Qos.Timeliness(QosTimeliness.REALTIME);
            refreshMsg.Qos.Rate(QosRates.TICK_BY_TICK);

            submitOptions.Clear();
            Assert.True(provider.SubmitAndDispatch((Msg)refreshMsg, submitOptions) >= ReactorReturnCode.SUCCESS);

            consumerReactor.Dispatch(1);
            evt = consumerReactor.PollEvent();
            Assert.Equal(TestReactorEventType.MSG, evt.EventType);
            msgEvent = (ReactorMsgEvent)evt.ReactorEvent;
            Assert.Equal(MsgClasses.REFRESH, msgEvent.Msg.MsgClass);
            receivedRefreshMsg = (IRefreshMsg)msgEvent.Msg;
            Assert.Equal((int)DomainType.MARKET_PRICE, receivedRefreshMsg.DomainType);
            Assert.Equal(DataTypes.NO_DATA, receivedRefreshMsg.ContainerType);
            Assert.Equal(StreamStates.OPEN, receivedRefreshMsg.State.StreamState());
            Assert.Equal(DataStates.OK, receivedRefreshMsg.State.DataState());
            Assert.Equal(consStreamIds[1], receivedRefreshMsg.StreamId);

            /* Close second item. */
            closeMsg.StreamId = consStreamIds[1]; ;

            submitOptions.Clear();
            Assert.True(consumer.SubmitAndDispatch((Msg)closeMsg, submitOptions) >= ReactorReturnCode.SUCCESS);

            /* Provider receives close. */
            providerReactor.Dispatch(1);
            evt = providerReactor.PollEvent();
            Assert.Equal(TestReactorEventType.MSG, evt.EventType);
            msgEvent = (ReactorMsgEvent)evt.ReactorEvent;
            Assert.Equal(MsgClasses.CLOSE, msgEvent.Msg.MsgClass);
            ICloseMsg receivedCloseMsg = (ICloseMsg)msgEvent.Msg;
            Assert.Equal(providerStreamId, receivedCloseMsg.StreamId);

            TestReactorComponent.CloseSession(consumer, provider);
            consumerReactor.Close();
            providerReactor.Close();
        }

        [Fact]
        public void WatchlistOneItem_ServiceChangeTest()
        {
            ReactorSubmitOptions submitOptions = new ReactorSubmitOptions();
            TestReactorEvent evt;
            ReactorMsgEvent msgEvent;

            ConsumerProviderSessionOptions opts = new ConsumerProviderSessionOptions();
            opts.SetupDefaultLoginStream = true;
            opts.SetupDefaultDirectoryStream = true;

            SetupSession(opts, false, 0, false, 0, out var consumerReactor, out var providerReactor, out var consumer, out var provider);

            // Consumer sends a request
            IRequestMsg requestMsg = new Msg();
            requestMsg.Clear();
            requestMsg.MsgClass = MsgClasses.REQUEST;
            requestMsg.StreamId = 5;
            requestMsg.DomainType = (int)DomainType.MARKET_PRICE;
            requestMsg.ApplyStreaming();
            requestMsg.MsgKey.ApplyHasName();
            requestMsg.MsgKey.Name.Data("TRI.N");
            requestMsg.ApplyHasQos();
            requestMsg.Qos.Timeliness(QosTimeliness.REALTIME);
            requestMsg.Qos.Rate(QosRates.TICK_BY_TICK);
            submitOptions.Clear();
            submitOptions.ServiceName = Provider.DefaultService.Info.ServiceName.ToString();
            Assert.True(consumer.SubmitAndDispatch((Msg)requestMsg, submitOptions) >= ReactorReturnCode.SUCCESS);

            providerReactor.Dispatch(1);
            evt = providerReactor.PollEvent();
            Assert.Equal(TestReactorEventType.MSG, evt.EventType);
            msgEvent = (ReactorMsgEvent)evt.ReactorEvent;
            Assert.Equal(MsgClasses.REQUEST, msgEvent.Msg.MsgClass);
            int providerStreamId = msgEvent.Msg.StreamId;

            DirectoryUpdate directoryUpdate = new DirectoryUpdate();
            directoryUpdate.StreamId = 2;
            directoryUpdate.Filter = ALL_FILTERS;
            Service service = new Service();
            Provider.DefaultService.Copy(service);
            directoryUpdate.ServiceList.Add(service);
            service.Action = MapEntryActions.UPDATE;
            service.Info.ServiceName.Data("SecretAgentMan");

            submitOptions.Clear();
            Assert.True(provider.Submit(directoryUpdate, submitOptions) >= ReactorReturnCode.SUCCESS);

            consumerReactor.Dispatch(1);
            evt = consumerReactor.PollEvent();
            Assert.Equal(TestReactorEventType.DIRECTORY_MSG, evt.EventType);
            msgEvent = (ReactorMsgEvent)evt.ReactorEvent;
            Assert.Equal(MsgClasses.UPDATE, msgEvent.Msg.MsgClass);

            IStatusMsg statusMsg = new Msg();
            statusMsg.StreamId = providerStreamId;
            statusMsg.MsgClass = MsgClasses.STATUS;
            statusMsg.DomainType = (int)DomainType.MARKET_PRICE;
            statusMsg.Flags = StatusMsgFlags.HAS_STATE;
            statusMsg.State.StreamState(StreamStates.CLOSED_RECOVER);
            statusMsg.State.DataState(DataStates.SUSPECT);

            submitOptions.Clear();
            Assert.True(provider.SubmitAndDispatch((Msg)statusMsg, submitOptions) >= ReactorReturnCode.SUCCESS);

            consumerReactor.Dispatch(1);

            // Consumer receives Open/Suspect status from provider.
            evt = consumerReactor.PollEvent();
            Assert.Equal(TestReactorEventType.MSG, evt.EventType);
            msgEvent = (ReactorMsgEvent)evt.ReactorEvent;
            Assert.Equal(MsgClasses.STATUS, msgEvent.Msg.MsgClass);
            Assert.Equal(5, msgEvent.Msg.StreamId);
            IStatusMsg receivedStatus = (IStatusMsg)msgEvent.Msg;
            Assert.True(receivedStatus.CheckHasState());
            Assert.Equal(StreamStates.OPEN, receivedStatus.State.StreamState());
            Assert.Equal(DataStates.SUSPECT, receivedStatus.State.DataState());

            // Provider receives nothing yet.
            providerReactor.Dispatch(0);

            // Provider changes service name back
            directoryUpdate.Clear();
            directoryUpdate.StreamId = 2;
            directoryUpdate.Filter = ALL_FILTERS;
            service = new Service();
            Provider.DefaultService.Copy(service);
            directoryUpdate.ServiceList.Add(service);
            service.Action = MapEntryActions.UPDATE;

            submitOptions.Clear();
            Assert.True(provider.Submit(directoryUpdate, submitOptions) >= ReactorReturnCode.SUCCESS);

            // Consumer receives an update for Directory
            consumerReactor.Dispatch(1);
            evt = consumerReactor.PollEvent();
            Assert.Equal(TestReactorEventType.DIRECTORY_MSG, evt.EventType);
            msgEvent = (ReactorMsgEvent)evt.ReactorEvent;
            Assert.Equal(MsgClasses.UPDATE, msgEvent.Msg.MsgClass);

            // Provider receives a request again
            providerReactor.Dispatch(1);
            evt = providerReactor.PollEvent();
            Assert.Equal(TestReactorEventType.MSG, evt.EventType);
            msgEvent = (ReactorMsgEvent)evt.ReactorEvent;
            Assert.Equal(MsgClasses.REQUEST, msgEvent.Msg.MsgClass);

            TestReactorComponent.CloseSession(consumer, provider);
            consumerReactor.Close();
            providerReactor.Close();
        }

        [Fact]
        public void WatchlistOneItem_ClosedRecover_PrivateStreamTest()
        {
            ReactorSubmitOptions submitOptions = new ReactorSubmitOptions();
            TestReactorEvent evt;
            ReactorMsgEvent msgEvent;

            ConsumerProviderSessionOptions opts = new ConsumerProviderSessionOptions();
            opts.SetupDefaultLoginStream = true;
            opts.SetupDefaultDirectoryStream = true;

            SetupSession(opts, false, 0, false, 0, out var consumerReactor, out var providerReactor, out var consumer, out var provider);

            // Consumer sends a request
            IRequestMsg requestMsg = new Msg();
            requestMsg.Clear();
            requestMsg.MsgClass = MsgClasses.REQUEST;
            requestMsg.StreamId = 5;
            requestMsg.DomainType = (int)DomainType.MARKET_PRICE;
            requestMsg.ApplyStreaming();
            requestMsg.ApplyPrivateStream();
            requestMsg.MsgKey.ApplyHasName();
            requestMsg.MsgKey.Name.Data("TRI.N");
            requestMsg.ApplyHasQos();
            requestMsg.Qos.Timeliness(QosTimeliness.REALTIME);
            requestMsg.Qos.Rate(QosRates.TICK_BY_TICK);
            submitOptions.Clear();
            submitOptions.ServiceName = Provider.DefaultService.Info.ServiceName.ToString();
            Assert.True(consumer.SubmitAndDispatch((Msg)requestMsg, submitOptions) >= ReactorReturnCode.SUCCESS);

            // Provider receives a request
            providerReactor.Dispatch(1);
            evt = providerReactor.PollEvent();
            Assert.Equal(TestReactorEventType.MSG, evt.EventType);
            msgEvent = (ReactorMsgEvent)evt.ReactorEvent;
            Assert.Equal(MsgClasses.REQUEST, msgEvent.Msg.MsgClass);
            int providerStreamId = msgEvent.Msg.StreamId;

            // Provider sends closed/recover
            IStatusMsg statusMsg = new Msg();
            statusMsg.StreamId = providerStreamId;
            statusMsg.MsgClass = MsgClasses.STATUS;
            statusMsg.DomainType = (int)DomainType.MARKET_PRICE;
            statusMsg.Flags = StatusMsgFlags.HAS_STATE | StatusMsgFlags.PRIVATE_STREAM;
            statusMsg.State.StreamState(StreamStates.CLOSED_RECOVER);
            statusMsg.State.DataState(DataStates.SUSPECT);

            submitOptions.Clear();
            Assert.True(provider.SubmitAndDispatch((Msg)statusMsg, submitOptions) >= ReactorReturnCode.SUCCESS);

            consumerReactor.Dispatch(1);

            // Consumer receives ClosedRecover/Suspect status from provider.
            evt = consumerReactor.PollEvent();
            Assert.Equal(TestReactorEventType.MSG, evt.EventType);
            msgEvent = (ReactorMsgEvent)evt.ReactorEvent;
            Assert.Equal(MsgClasses.STATUS, msgEvent.Msg.MsgClass);
            Assert.Equal(5, msgEvent.Msg.StreamId);
            IStatusMsg receivedStatus = (IStatusMsg)msgEvent.Msg;
            Assert.True(receivedStatus.CheckPrivateStream());
            Assert.True(receivedStatus.CheckHasState());
            Assert.Equal(StreamStates.CLOSED_RECOVER, receivedStatus.State.StreamState());
            Assert.Equal(DataStates.SUSPECT, receivedStatus.State.DataState());

            // Provider receives no more messages
            providerReactor.Dispatch(0);

            TestReactorComponent.CloseSession(consumer, provider);
            consumerReactor.Close();
            providerReactor.Close();
        }

        [Fact]
        public void WatchlistOneItem_ClosedRecover_SingleOpenOffTest()
        {
            ReactorSubmitOptions submitOptions = new ReactorSubmitOptions();
            TestReactorEvent evt;
            ReactorMsgEvent msgEvent;

            ConsumerProviderSessionOptions opts = new ConsumerProviderSessionOptions();
            opts.SetupDefaultLoginStream = true;
            opts.SetupDefaultDirectoryStream = true;

            SetupSession(opts, true, 0, false, 0, out var consumerReactor, out var providerReactor, out var consumer, out var provider);

            // Consumer sends a request from nonexistent service
            IRequestMsg requestMsg = new Msg();
            requestMsg.Clear();
            requestMsg.MsgClass = MsgClasses.REQUEST;
            requestMsg.StreamId = 5;
            requestMsg.DomainType = (int)DomainType.MARKET_PRICE;
            requestMsg.ApplyStreaming();
            requestMsg.MsgKey.ApplyHasName();
            requestMsg.MsgKey.Name.Data("TRI.N");
            requestMsg.ApplyHasQos();
            requestMsg.Qos.Timeliness(QosTimeliness.REALTIME);
            requestMsg.Qos.Rate(QosRates.TICK_BY_TICK);

            submitOptions.Clear();
            submitOptions.ServiceName = "MLB";
            Assert.True(consumer.Submit((Msg)requestMsg, submitOptions) >= ReactorReturnCode.SUCCESS);

            consumerReactor.Dispatch(1);

            // Consumer receives ClosedRecover/Suspect status.
            evt = consumerReactor.PollEvent();
            Assert.Equal(TestReactorEventType.MSG, evt.EventType);
            msgEvent = (ReactorMsgEvent)evt.ReactorEvent;
            Assert.Equal(MsgClasses.STATUS, msgEvent.Msg.MsgClass);
            Assert.Equal(5, msgEvent.Msg.StreamId);
            IStatusMsg receivedStatus = (IStatusMsg)msgEvent.Msg;
            Assert.True(receivedStatus.CheckHasState());
            Assert.Equal(StreamStates.CLOSED_RECOVER, receivedStatus.State.StreamState());
            Assert.Equal(DataStates.SUSPECT, receivedStatus.State.DataState());

            requestMsg.Clear();
            requestMsg.MsgClass = MsgClasses.REQUEST;
            requestMsg.StreamId = 5;
            requestMsg.DomainType = (int)DomainType.MARKET_PRICE;
            requestMsg.ApplyStreaming();
            requestMsg.MsgKey.ApplyHasName();
            requestMsg.MsgKey.Name.Data("TRI.N");
            requestMsg.ApplyHasQos();
            requestMsg.Qos.Timeliness(QosTimeliness.REALTIME);
            requestMsg.Qos.Rate(QosRates.TICK_BY_TICK);

            submitOptions.Clear();
            submitOptions.ServiceName = Provider.DefaultService.Info.ServiceName.ToString();
            Assert.True(consumer.SubmitAndDispatch((Msg)requestMsg, submitOptions) >= ReactorReturnCode.SUCCESS);

            // Provider receives a request
            providerReactor.Dispatch(1);
            evt = providerReactor.PollEvent();
            Assert.Equal(TestReactorEventType.MSG, evt.EventType);
            msgEvent = (ReactorMsgEvent)evt.ReactorEvent;
            Assert.Equal(MsgClasses.REQUEST, msgEvent.Msg.MsgClass);
            int providerStreamId = msgEvent.Msg.StreamId;

            // Provider sends closed/recover
            IStatusMsg statusMsg = new Msg();
            statusMsg.StreamId = providerStreamId;
            statusMsg.MsgClass = MsgClasses.STATUS;
            statusMsg.DomainType = (int)DomainType.MARKET_PRICE;
            statusMsg.Flags = StatusMsgFlags.HAS_STATE | StatusMsgFlags.PRIVATE_STREAM;
            statusMsg.State.StreamState(StreamStates.CLOSED_RECOVER);
            statusMsg.State.DataState(DataStates.SUSPECT);

            submitOptions.Clear();
            Assert.True(provider.SubmitAndDispatch((Msg)statusMsg, submitOptions) >= ReactorReturnCode.SUCCESS);

            consumerReactor.Dispatch(1);

            // Consumer receives ClosedRecover/Suspect status from provider.
            evt = consumerReactor.PollEvent();
            Assert.Equal(TestReactorEventType.MSG, evt.EventType);
            msgEvent = (ReactorMsgEvent)evt.ReactorEvent;
            Assert.Equal(MsgClasses.STATUS, msgEvent.Msg.MsgClass);
            Assert.Equal(5, msgEvent.Msg.StreamId);
            receivedStatus = (IStatusMsg)msgEvent.Msg;
            Assert.True(receivedStatus.CheckPrivateStream());
            Assert.True(receivedStatus.CheckHasState());
            Assert.Equal(StreamStates.CLOSED_RECOVER, receivedStatus.State.StreamState());
            Assert.Equal(DataStates.SUSPECT, receivedStatus.State.DataState());

            // Provider receives no more messages
            providerReactor.Dispatch(0);

            TestReactorComponent.CloseSession(consumer, provider);
            consumerReactor.Close();
            providerReactor.Close();
        }

        [Fact]
        public void WatchlistOneItem_AllowSuspectDataOffTest()
        {
            ReactorSubmitOptions submitOptions = new ReactorSubmitOptions();
            TestReactorEvent evt;
            ReactorMsgEvent msgEvent;

            ConsumerProviderSessionOptions opts = new ConsumerProviderSessionOptions();
            opts.SetupDefaultLoginStream = true;
            opts.SetupDefaultDirectoryStream = true;

            SetupSession(opts, true, 0, true, 0, out var consumerReactor, out var providerReactor, out var consumer, out var provider);

            IRequestMsg requestMsg = new Msg();
            requestMsg.Clear();
            requestMsg.MsgClass = MsgClasses.REQUEST;
            requestMsg.StreamId = 5;
            requestMsg.DomainType = (int)DomainType.MARKET_PRICE;
            requestMsg.ApplyStreaming();
            requestMsg.MsgKey.ApplyHasName();
            requestMsg.MsgKey.Name.Data("TRI.N");
            requestMsg.ApplyHasQos();
            requestMsg.Qos.Timeliness(QosTimeliness.REALTIME);
            requestMsg.Qos.Rate(QosRates.TICK_BY_TICK);

            submitOptions.Clear();
            submitOptions.ServiceName = Provider.DefaultService.Info.ServiceName.ToString();
            Assert.True(consumer.Submit((Msg)requestMsg, submitOptions) >= ReactorReturnCode.SUCCESS);

            // Provider receives a request
            providerReactor.Dispatch(1);
            evt = providerReactor.PollEvent();
            Assert.Equal(TestReactorEventType.MSG, evt.EventType);
            msgEvent = (ReactorMsgEvent)evt.ReactorEvent;
            Assert.Equal(MsgClasses.REQUEST, msgEvent.Msg.MsgClass);
            int providerStreamId = msgEvent.Msg.StreamId;

            // Provider sends open/suspect
            IStatusMsg statusMsg = new Msg();
            statusMsg.StreamId = providerStreamId;
            statusMsg.MsgClass = MsgClasses.STATUS;
            statusMsg.DomainType = (int)DomainType.MARKET_PRICE;
            statusMsg.Flags = StatusMsgFlags.HAS_STATE;
            statusMsg.State.StreamState(StreamStates.OPEN);
            statusMsg.State.DataState(DataStates.SUSPECT);

            submitOptions.Clear();
            Assert.True(provider.SubmitAndDispatch((Msg)statusMsg, submitOptions) >= ReactorReturnCode.SUCCESS);

            consumerReactor.Dispatch(1);

            // Consumer receives ClosedRecover/Suspect status.
            evt = consumerReactor.PollEvent();
            Assert.Equal(TestReactorEventType.MSG, evt.EventType);
            msgEvent = (ReactorMsgEvent)evt.ReactorEvent;
            Assert.Equal(MsgClasses.STATUS, msgEvent.Msg.MsgClass);
            Assert.Equal(5, msgEvent.Msg.StreamId);
            var receivedStatus = (IStatusMsg)msgEvent.Msg;
            Assert.True(receivedStatus.CheckHasState());
            Assert.Equal(StreamStates.CLOSED_RECOVER, receivedStatus.State.StreamState());
            Assert.Equal(DataStates.SUSPECT, receivedStatus.State.DataState());

            // Provider receives CLOSE request
            providerReactor.Dispatch(1);
            evt = providerReactor.PollEvent();
            Assert.Equal(TestReactorEventType.MSG, evt.EventType);
            msgEvent = (ReactorMsgEvent)evt.ReactorEvent;
            Assert.Equal(MsgClasses.CLOSE, msgEvent.Msg.MsgClass);
            Assert.Equal((int)DomainType.MARKET_PRICE, msgEvent.Msg.DomainType);
            Assert.Equal(providerStreamId, msgEvent.Msg.StreamId);

            TestReactorComponent.CloseSession(consumer, provider);
            consumerReactor.Close();
            providerReactor.Close();
        }

        [Fact]
        public void WatchlistTwoItems_ClosedRecoverFromServiceStateTest()
        {
            ReactorSubmitOptions submitOptions = new ReactorSubmitOptions();
            TestReactorEvent evt;
            ReactorMsgEvent msgEvent;

            ConsumerProviderSessionOptions opts = new ConsumerProviderSessionOptions();
            opts.SetupDefaultLoginStream = true;
            opts.SetupDefaultDirectoryStream = true;

            SetupSession(opts, true, 1, false, 0, out var consumerReactor, out var providerReactor, out var consumer, out var provider);

            // Request first item
            IRequestMsg requestMsg = new Msg();
            requestMsg.Clear();
            requestMsg.MsgClass = MsgClasses.REQUEST;
            requestMsg.StreamId = 5;
            requestMsg.DomainType = (int)DomainType.MARKET_PRICE;
            requestMsg.ApplyStreaming();
            requestMsg.MsgKey.ApplyHasName();
            requestMsg.MsgKey.Name.Data("TRI.N");
            requestMsg.ApplyHasQos();
            requestMsg.Qos.Timeliness(QosTimeliness.REALTIME);
            requestMsg.Qos.Rate(QosRates.TICK_BY_TICK);
            submitOptions.Clear();
            submitOptions.ServiceName = Provider.DefaultService.Info.ServiceName.ToString();
            Assert.True(consumer.Submit((Msg)requestMsg, submitOptions) >= ReactorReturnCode.SUCCESS);

            // Request second item
            requestMsg.Clear();
            requestMsg.MsgClass = MsgClasses.REQUEST;
            requestMsg.StreamId = 6;
            requestMsg.DomainType = (int)DomainType.MARKET_PRICE;
            requestMsg.ApplyStreaming();
            requestMsg.MsgKey.ApplyHasName();
            requestMsg.MsgKey.Name.Data("IBM.N");
            requestMsg.ApplyHasQos();
            requestMsg.Qos.Timeliness(QosTimeliness.REALTIME);
            requestMsg.Qos.Rate(QosRates.TICK_BY_TICK);
            submitOptions.Clear();
            submitOptions.ServiceName = Provider.DefaultService.Info.ServiceName.ToString();
            Assert.True(consumer.Submit((Msg)requestMsg, submitOptions) >= ReactorReturnCode.SUCCESS);

            consumerReactor.Dispatch(0);

            providerReactor.Dispatch(2);

            evt = providerReactor.PollEvent();
            Assert.Equal(TestReactorEventType.MSG, evt.EventType);
            msgEvent = (ReactorMsgEvent)evt.ReactorEvent;
            Assert.Equal(MsgClasses.REQUEST, msgEvent.Msg.MsgClass);
            var reqMsg = (IRequestMsg)msgEvent.Msg;
            Assert.True(reqMsg.MsgKey.CheckHasName());
            Assert.Equal("TRI.N", reqMsg.MsgKey.Name.ToString());
            int providerStreamId1 = msgEvent.Msg.StreamId;

            evt = providerReactor.PollEvent();
            Assert.Equal(TestReactorEventType.MSG, evt.EventType);
            msgEvent = (ReactorMsgEvent)evt.ReactorEvent;
            Assert.Equal(MsgClasses.REQUEST, msgEvent.Msg.MsgClass);
            reqMsg = (IRequestMsg)msgEvent.Msg;
            Assert.True(reqMsg.MsgKey.CheckHasName());
            Assert.Equal("IBM.N", reqMsg.MsgKey.Name.ToString());
            int providerStreamId2 = msgEvent.Msg.StreamId;

            DirectoryUpdate directoryUpdate = new DirectoryUpdate();
            directoryUpdate.StreamId = 2;
            Service service = new Service();
            Provider.DefaultService.Copy(service);
            service.Flags |= ServiceFlags.HAS_STATE;
            service.State.HasStatus = true;
            service.State.Status.StreamState(StreamStates.CLOSED_RECOVER);
            service.State.Status.DataState(DataStates.SUSPECT);
            service.Action = MapEntryActions.UPDATE;

            directoryUpdate.ServiceList.Add(service);

            submitOptions.Clear();
            Assert.True(provider.SubmitAndDispatch(directoryUpdate, submitOptions) >= ReactorReturnCode.SUCCESS);

            // Consumer receives Open/Suspect status messages and directory update for the initial Directory request
            consumerReactor.Dispatch(3);

            evt = consumerReactor.PollEvent();
            Assert.Equal(TestReactorEventType.MSG, evt.EventType);
            msgEvent = (ReactorMsgEvent)evt.ReactorEvent;
            Assert.Equal(MsgClasses.STATUS, msgEvent.Msg.MsgClass);
            Assert.Equal(5, msgEvent.Msg.StreamId);
            var receivedStatus = (IStatusMsg)msgEvent.Msg;
            Assert.True(receivedStatus.CheckHasState());
            Assert.Equal(StreamStates.OPEN, receivedStatus.State.StreamState());
            Assert.Equal(DataStates.SUSPECT, receivedStatus.State.DataState());

            evt = consumerReactor.PollEvent();
            Assert.Equal(TestReactorEventType.MSG, evt.EventType);
            msgEvent = (ReactorMsgEvent)evt.ReactorEvent;
            Assert.Equal(MsgClasses.STATUS, msgEvent.Msg.MsgClass);
            Assert.Equal(6, msgEvent.Msg.StreamId);
            receivedStatus = (IStatusMsg)msgEvent.Msg;
            Assert.True(receivedStatus.CheckHasState());
            Assert.Equal(StreamStates.OPEN, receivedStatus.State.StreamState());
            Assert.Equal(DataStates.SUSPECT, receivedStatus.State.DataState());

            evt = consumerReactor.PollEvent();
            Assert.Equal(TestReactorEventType.DIRECTORY_MSG, evt.EventType);
            msgEvent = (ReactorMsgEvent)evt.ReactorEvent;
            Assert.Equal(MsgClasses.UPDATE, msgEvent.Msg.MsgClass);

            // provider receives requests again
            providerReactor.Dispatch(2);

            evt = providerReactor.PollEvent();
            Assert.Equal(TestReactorEventType.MSG, evt.EventType);
            msgEvent = (ReactorMsgEvent)evt.ReactorEvent;
            Assert.Equal(MsgClasses.REQUEST, msgEvent.Msg.MsgClass);
            reqMsg = (IRequestMsg)msgEvent.Msg;
            Assert.True(reqMsg.MsgKey.CheckHasName());
            Assert.Equal("TRI.N", reqMsg.MsgKey.Name.ToString());

            evt = providerReactor.PollEvent();
            Assert.Equal(TestReactorEventType.MSG, evt.EventType);
            msgEvent = (ReactorMsgEvent)evt.ReactorEvent;
            Assert.Equal(MsgClasses.REQUEST, msgEvent.Msg.MsgClass);
            reqMsg = (IRequestMsg)msgEvent.Msg;
            Assert.True(reqMsg.MsgKey.CheckHasName());
            Assert.Equal("IBM.N", reqMsg.MsgKey.Name.ToString());

            TestReactorComponent.CloseSession(consumer, provider);
            consumerReactor.Close();
            providerReactor.Close();
        }

        [Fact]
        public void WatchlistOneItem_ClosedRecoverFromGroupStateTest()
        {
            ReactorSubmitOptions submitOptions = new ReactorSubmitOptions();
            TestReactorEvent evt;
            ReactorMsgEvent msgEvent;

            ConsumerProviderSessionOptions opts = new ConsumerProviderSessionOptions();
            opts.SetupDefaultLoginStream = true;
            opts.SetupDefaultDirectoryStream = true;

            SetupSession(opts, true, 1, false, 0, out var consumerReactor, out var providerReactor, out var consumer, out var provider);

            // Request first item
            IRequestMsg requestMsg = new Msg();
            requestMsg.Clear();
            requestMsg.MsgClass = MsgClasses.REQUEST;
            requestMsg.StreamId = 5;
            requestMsg.DomainType = (int)DomainType.MARKET_PRICE;
            requestMsg.ApplyStreaming();
            requestMsg.MsgKey.ApplyHasName();
            requestMsg.MsgKey.Name.Data("TRI.N");
            requestMsg.ApplyHasQos();
            requestMsg.Qos.Timeliness(QosTimeliness.REALTIME);
            requestMsg.Qos.Rate(QosRates.TICK_BY_TICK);
            submitOptions.Clear();
            submitOptions.ServiceName = Provider.DefaultService.Info.ServiceName.ToString();
            Assert.True(consumer.Submit((Msg)requestMsg, submitOptions) >= ReactorReturnCode.SUCCESS);

            providerReactor.Dispatch(1);

            evt = providerReactor.PollEvent();
            Assert.Equal(TestReactorEventType.MSG, evt.EventType);
            msgEvent = (ReactorMsgEvent)evt.ReactorEvent;
            Assert.Equal(MsgClasses.REQUEST, msgEvent.Msg.MsgClass);
            var reqMsg = (IRequestMsg)msgEvent.Msg;
            Assert.True(reqMsg.MsgKey.CheckHasName());
            Assert.Equal("TRI.N", reqMsg.MsgKey.Name.ToString());

            // Provider sends item refresh
            IRefreshMsg refreshMsg = new Msg();
            refreshMsg.StreamId = reqMsg.StreamId;
            refreshMsg.DomainType = (int)DomainType.MARKET_PRICE;
            refreshMsg.MsgClass = MsgClasses.REFRESH;
            refreshMsg.ApplyHasMsgKey();
            refreshMsg.MsgKey.ApplyHasServiceId();
            refreshMsg.MsgKey.ServiceId = Provider.DefaultService.ServiceId;
            refreshMsg.Flags = RefreshMsgFlags.CLEAR_CACHE | RefreshMsgFlags.HAS_QOS
                | RefreshMsgFlags.REFRESH_COMPLETE | RefreshMsgFlags.SOLICITED;
            refreshMsg.Qos.Timeliness(QosTimeliness.REALTIME);
            refreshMsg.Qos.Rate(QosRates.TICK_BY_TICK);
            refreshMsg.State.StreamState(StreamStates.OPEN);
            refreshMsg.State.DataState(DataStates.OK);
            Buffer groupId = new Buffer();
            groupId.Data("group1");
            refreshMsg.GroupId = groupId;

            submitOptions.Clear();
            Assert.True(provider.SubmitAndDispatch((Msg)refreshMsg, submitOptions) >= ReactorReturnCode.SUCCESS);

            consumerReactor.Dispatch(1);
            evt = consumerReactor.PollEvent();
            Assert.Equal(TestReactorEventType.MSG, evt.EventType);
            msgEvent = (ReactorMsgEvent)evt.ReactorEvent;
            Assert.Equal(MsgClasses.REFRESH, msgEvent.Msg.MsgClass);
            var refMsg = (IRefreshMsg)msgEvent.Msg;
            Assert.Equal((int)DomainType.MARKET_PRICE, refMsg.DomainType);
            Assert.Equal(RefreshMsgFlags.CLEAR_CACHE | RefreshMsgFlags.HAS_QOS
                | RefreshMsgFlags.REFRESH_COMPLETE | RefreshMsgFlags.SOLICITED, refMsg.Flags);
            Assert.Equal(StreamStates.OPEN, refMsg.State.StreamState());
            Assert.Equal(DataStates.OK, refMsg.State.DataState());
            Assert.Equal(QosTimeliness.REALTIME, refMsg.Qos.Timeliness());
            Assert.Equal(QosRates.TICK_BY_TICK, refMsg.Qos.Rate());

            // Provider sends closed-recover via group
            DirectoryUpdate directoryUpdate = new DirectoryUpdate();
            directoryUpdate.StreamId = 2;
            Service service = new Service();
            Provider.DefaultService.Copy(service);
            ServiceGroup serviceGroup = new ServiceGroup();
            serviceGroup.HasStatus = true;
            serviceGroup.Status.StreamState(StreamStates.CLOSED_RECOVER);
            serviceGroup.Status.DataState(DataStates.SUSPECT);
            serviceGroup.Group = groupId;
            service.GroupStateList.Add(serviceGroup);
            service.Action = MapEntryActions.UPDATE;

            directoryUpdate.ServiceList.Add(service);

            submitOptions.Clear();
            Assert.True(provider.SubmitAndDispatch(directoryUpdate, submitOptions) >= ReactorReturnCode.SUCCESS);

            // Consumer receives Open/Suspect status message and directory update for the initial Directory request
            consumerReactor.Dispatch(2);

            evt = consumerReactor.PollEvent();
            Assert.Equal(TestReactorEventType.MSG, evt.EventType);
            msgEvent = (ReactorMsgEvent)evt.ReactorEvent;
            Assert.Equal(MsgClasses.STATUS, msgEvent.Msg.MsgClass);
            Assert.Equal(5, msgEvent.Msg.StreamId);
            var receivedStatus = (IStatusMsg)msgEvent.Msg;
            Assert.True(receivedStatus.CheckHasState());
            Assert.Equal(StreamStates.OPEN, receivedStatus.State.StreamState());
            Assert.Equal(DataStates.SUSPECT, receivedStatus.State.DataState());

            evt = consumerReactor.PollEvent();
            Assert.Equal(TestReactorEventType.DIRECTORY_MSG, evt.EventType);
            msgEvent = (ReactorMsgEvent)evt.ReactorEvent;
            Assert.Equal(MsgClasses.UPDATE, msgEvent.Msg.MsgClass);

            // provider receives request again
            providerReactor.Dispatch(1);

            evt = providerReactor.PollEvent();
            Assert.Equal(TestReactorEventType.MSG, evt.EventType);
            msgEvent = (ReactorMsgEvent)evt.ReactorEvent;
            Assert.Equal(MsgClasses.REQUEST, msgEvent.Msg.MsgClass);
            reqMsg = (IRequestMsg)msgEvent.Msg;
            Assert.True(reqMsg.MsgKey.CheckHasName());
            Assert.Equal("TRI.N", reqMsg.MsgKey.Name.ToString());

            TestReactorComponent.CloseSession(consumer, provider);
            consumerReactor.Close();
            providerReactor.Close();
        }

        [Fact]
        public void WatchlistPrivateStreamWithPayloadAndExtHeaderTest()
        {
            ReactorSubmitOptions submitOptions = new ReactorSubmitOptions();
            TestReactorEvent evt;
            ReactorMsgEvent msgEvent;

            ConsumerProviderSessionOptions opts = new ConsumerProviderSessionOptions();
            opts.SetupDefaultLoginStream = true;
            opts.SetupDefaultDirectoryStream = true;

            SetupSession(opts, true, 1, false, 0, out var consumerReactor, out var providerReactor, out var consumer, out var provider);

            int providerItemStream;
            IRequestMsg requestMsg = new Msg();
            Buffer dataBody = new Buffer();
            dataBody.Data("Spagetti");
            Buffer dataBody2 = new Buffer();
            dataBody2.Data("Dinner");
            Buffer requestHeader = new Buffer();
            requestHeader.Data("Turkey");
            Buffer requestHeader2 = new Buffer();
            requestHeader2.Data("Sub");

            for (int i = 0; i < 3; i++)
            {
                bool testDataBody = false;
                bool testExtHeader = false;

                switch (i)
                {
                    case 0: testDataBody = true; testExtHeader = false; break;
                    case 1: testDataBody = false; testExtHeader = true; break;
                    case 2: testDataBody = true; testExtHeader = true; break;
                    default: testDataBody = false; testExtHeader = false; break;
                }

                requestMsg.Clear();
                requestMsg.MsgClass = MsgClasses.REQUEST;
                requestMsg.StreamId = 5;
                requestMsg.DomainType = (int)DomainType.MARKET_PRICE;
                requestMsg.ApplyStreaming();
                requestMsg.MsgKey.ApplyHasName();
                requestMsg.MsgKey.Name.Data("TRI.N");
                requestMsg.ApplyHasQos();
                requestMsg.Qos.Timeliness(QosTimeliness.REALTIME);
                requestMsg.Qos.Rate(QosRates.TICK_BY_TICK);
                requestMsg.ApplyPrivateStream();

                if (testDataBody)
                {
                    requestMsg.ContainerType = DataTypes.OPAQUE;
                    requestMsg.EncodedDataBody = dataBody;
                }

                if (testExtHeader)
                {
                    requestMsg.ApplyHasExtendedHdr();
                    requestMsg.ExtendedHeader = requestHeader;
                }

                submitOptions.Clear();
                submitOptions.ServiceName = Provider.DefaultService.Info.ServiceName.ToString();
                Assert.True(consumer.SubmitAndDispatch((Msg)requestMsg, submitOptions) >= ReactorReturnCode.SUCCESS);

                // Provider receives request
                providerReactor.Dispatch(1);
                evt = providerReactor.PollEvent();
                Assert.Equal(TestReactorEventType.MSG, evt.EventType);
                msgEvent = (ReactorMsgEvent)evt.ReactorEvent;
                Assert.Equal(MsgClasses.REQUEST, msgEvent.Msg.MsgClass);
                var reqMsg = (IRequestMsg)msgEvent.Msg;
                Assert.True(reqMsg.MsgKey.CheckHasName());
                Assert.Equal("TRI.N", reqMsg.MsgKey.Name.ToString());
                Assert.Equal(testExtHeader, reqMsg.CheckHasExtendedHdr());
                if (testExtHeader)
                {
                    Assert.Equal(requestHeader, reqMsg.ExtendedHeader);
                }
                if (testDataBody)
                {
                    Assert.Equal(DataTypes.OPAQUE, reqMsg.ContainerType);
                    Assert.Equal(dataBody, reqMsg.EncodedDataBody);
                }
                else
                {
                    Assert.Equal(DataTypes.NO_DATA, reqMsg.ContainerType);
                }

                providerItemStream = reqMsg.StreamId;

                requestMsg.Clear();
                requestMsg.MsgClass = MsgClasses.REQUEST;
                requestMsg.StreamId = 5;
                requestMsg.DomainType = (int)DomainType.MARKET_PRICE;
                requestMsg.ApplyStreaming();
                requestMsg.MsgKey.ApplyHasName();
                requestMsg.MsgKey.Name.Data("TRI.N");
                requestMsg.ApplyHasQos();
                requestMsg.Qos.Timeliness(QosTimeliness.REALTIME);
                requestMsg.Qos.Rate(QosRates.TICK_BY_TICK);
                requestMsg.ApplyPrivateStream();

                if (testDataBody)
                {
                    requestMsg.ContainerType = DataTypes.OPAQUE;
                    requestMsg.EncodedDataBody = dataBody2;
                }

                if (testExtHeader)
                {
                    requestMsg.ApplyHasExtendedHdr();
                    requestMsg.ExtendedHeader = requestHeader2;
                }

                submitOptions.Clear();
                submitOptions.ServiceName = Provider.DefaultService.Info.ServiceName.ToString();
                Assert.True(consumer.SubmitAndDispatch((Msg)requestMsg, submitOptions) >= ReactorReturnCode.SUCCESS);

                // Provider receives reissue
                providerReactor.Dispatch(1);
                evt = providerReactor.PollEvent();
                Assert.Equal(TestReactorEventType.MSG, evt.EventType);
                msgEvent = (ReactorMsgEvent)evt.ReactorEvent;
                Assert.Equal(MsgClasses.REQUEST, msgEvent.Msg.MsgClass);
                reqMsg = (IRequestMsg)msgEvent.Msg;
                Assert.True(reqMsg.MsgKey.CheckHasName());
                Assert.Equal("TRI.N", reqMsg.MsgKey.Name.ToString());
                Assert.Equal(testExtHeader, reqMsg.CheckHasExtendedHdr());
                if (testExtHeader)
                {
                    Assert.Equal(requestHeader2, reqMsg.ExtendedHeader);
                }

                if (testDataBody)
                {
                    Assert.Equal(DataTypes.OPAQUE, reqMsg.ContainerType);
                    Assert.Equal(dataBody2, reqMsg.EncodedDataBody);
                }
                else
                {
                    Assert.Equal(DataTypes.NO_DATA, reqMsg.ContainerType);
                }
                Assert.Equal(providerItemStream, reqMsg.StreamId);

                // Close item
                ICloseMsg closeMsg = new Msg();
                closeMsg.StreamId = 5;
                closeMsg.DomainType = (int)DomainType.MARKET_PRICE;
                closeMsg.MsgClass = MsgClasses.CLOSE;
                submitOptions.Clear();
                Assert.True(consumer.Submit((Msg)closeMsg, submitOptions) >= ReactorReturnCode.SUCCESS);

                providerReactor.Dispatch(1);
                evt = providerReactor.PollEvent();
                Assert.Equal(TestReactorEventType.MSG, evt.EventType);
                msgEvent = (ReactorMsgEvent)evt.ReactorEvent;
                Assert.Equal(MsgClasses.CLOSE, msgEvent.Msg.MsgClass);
                Assert.Equal(providerItemStream, msgEvent.Msg.StreamId);
            }

            TestReactorComponent.CloseSession(consumer, provider);
            consumerReactor.Close();
            providerReactor.Close();
        }

        private void WatchlistOneItem_LoginClosedRecover(bool singleOpen)
        {
            ReactorSubmitOptions submitOptions = new ReactorSubmitOptions();
            TestReactorEvent evt;
            ReactorMsgEvent msgEvent;
            LoginStatus loginStatus = new LoginStatus();
            LoginStatus statusMsg;
            RDMLoginMsgEvent loginMsgEvent;
            ReactorChannelEvent channelEvent;

            ConsumerProviderSessionOptions opts = new ConsumerProviderSessionOptions();
            opts.SetupDefaultLoginStream = true;
            opts.SetupDefaultDirectoryStream = true;
            opts.ReconnectMinDelay = new TimeSpan(0, 0, 1);
            opts.ReconnectMaxDelay = new TimeSpan(0, 0, 3);
            opts.ReconnectAttemptLimit = 5;

            SetupSession(opts, true, singleOpen ? 1 : 0, false, 0, out var consumerReactor, out var providerReactor, out var consumer, out var provider);

            IRequestMsg requestMsg = new Msg();
            requestMsg.Clear();
            requestMsg.MsgClass = MsgClasses.REQUEST;
            requestMsg.StreamId = 5;
            requestMsg.DomainType = (int)DomainType.MARKET_PRICE;
            requestMsg.ApplyStreaming();
            requestMsg.MsgKey.ApplyHasName();
            requestMsg.MsgKey.Name.Data("TRI.N");
            requestMsg.ApplyHasQos();
            requestMsg.Qos.Timeliness(QosTimeliness.REALTIME);
            requestMsg.Qos.Rate(QosRates.TICK_BY_TICK);

            submitOptions.Clear();
            submitOptions.ServiceName = Provider.DefaultService.Info.ServiceName.ToString();
            Assert.True(consumer.SubmitAndDispatch((Msg)requestMsg, submitOptions) >= ReactorReturnCode.SUCCESS);

            // Provider receives reissue
            providerReactor.Dispatch(1);
            evt = providerReactor.PollEvent();
            Assert.Equal(TestReactorEventType.MSG, evt.EventType);
            msgEvent = (ReactorMsgEvent)evt.ReactorEvent;
            Assert.Equal(MsgClasses.REQUEST, msgEvent.Msg.MsgClass);
            var reqMsg = (IRequestMsg)msgEvent.Msg;
            Assert.True(reqMsg.MsgKey.CheckHasName());
            Assert.Equal("TRI.N", reqMsg.MsgKey.Name.ToString());

            /* Make sure we back off on reconnecting if we don't get past login. */
            for (int i = 0; i < 2; ++i)
            {
                loginStatus.Clear();
                loginStatus.StreamId = 1;
                loginStatus.HasState = true;
                loginStatus.State.StreamState(StreamStates.CLOSED_RECOVER);
                loginStatus.State.DataState(DataStates.SUSPECT);

                submitOptions.Clear();
                Assert.True(provider.SubmitAndDispatch(loginStatus, submitOptions) >= ReactorReturnCode.SUCCESS);

                // Consumer receives LoginStatus from callback,
                // Channel event for channel down, Directory update message for the default Reactor Directory request and 
                consumerReactor.Dispatch(4 - 2 * i);

                evt = consumerReactor.PollEvent();
                Assert.Equal(TestReactorEventType.LOGIN_MSG, evt.EventType);
                loginMsgEvent = (RDMLoginMsgEvent)evt.ReactorEvent;
                Assert.Equal(MsgClasses.STATUS, loginMsgEvent.Msg.MsgClass);
                Assert.Equal(1, loginMsgEvent.Msg.StreamId);
                statusMsg = loginMsgEvent.LoginMsg.LoginStatus;
                Assert.True(statusMsg.HasState);
                Assert.Equal(StreamStates.OPEN, statusMsg.State.StreamState());
                Assert.Equal(DataStates.SUSPECT, statusMsg.State.DataState());

                evt = consumerReactor.PollEvent();
                Assert.Equal(TestReactorEventType.CHANNEL_EVENT, evt.EventType);
                channelEvent = (ReactorChannelEvent)evt.ReactorEvent;
                Assert.Equal(ReactorChannelEventType.CHANNEL_DOWN_RECONNECTING, channelEvent.EventType);

                if (i == 0)
                {
                    /* First time, consumer receives directory update 
                     * and item status (Open/Suspect or ClosedRecover/Suspect based on Single-Open setting). */
                    evt = consumerReactor.PollEvent();
                    Assert.Equal(TestReactorEventType.DIRECTORY_MSG, evt.EventType);
                    msgEvent = (ReactorMsgEvent)evt.ReactorEvent;
                    Assert.Equal(MsgClasses.UPDATE, msgEvent.Msg.MsgClass);

                    evt = consumerReactor.PollEvent();
                    Assert.Equal(TestReactorEventType.MSG, evt.EventType);
                    msgEvent = (ReactorMsgEvent)evt.ReactorEvent;
                    Assert.Equal(MsgClasses.STATUS, msgEvent.Msg.MsgClass);
                    Assert.Equal(5, msgEvent.Msg.StreamId);
                    var codecStatusMsg = (IStatusMsg)msgEvent.Msg;
                    Assert.True(codecStatusMsg.CheckHasState());
                    Assert.Equal(singleOpen ? StreamStates.OPEN : StreamStates.CLOSED_RECOVER, codecStatusMsg.State.StreamState()); // Adjust depending on SingleOpen
                    Assert.Equal(DataStates.SUSPECT, codecStatusMsg.State.DataState());
                }

                providerReactor.Dispatch(1);
                evt = providerReactor.PollEvent();
                Assert.Equal(TestReactorEventType.CHANNEL_EVENT, evt.EventType); // provider receives channel down event
                channelEvent = (ReactorChannelEvent)evt.ReactorEvent;
                Assert.Equal(ReactorChannelEventType.CHANNEL_DOWN, channelEvent.EventType);

                provider.CloseChannel();

                // Recover connection
                provider.TestReactor.Accept(opts, provider, new TimeSpan(0, 0, 1 * (int)Math.Pow(2, i)));

                providerReactor.Dispatch(2);
                evt = providerReactor.PollEvent();
                Assert.Equal(TestReactorEventType.CHANNEL_EVENT, evt.EventType); // provider receives channel up event
                channelEvent = (ReactorChannelEvent)evt.ReactorEvent;
                Assert.Equal(ReactorChannelEventType.CHANNEL_UP, channelEvent.EventType);
                evt = providerReactor.PollEvent();
                Assert.Equal(TestReactorEventType.CHANNEL_EVENT, evt.EventType); // provider receives channel ready event
                channelEvent = (ReactorChannelEvent)evt.ReactorEvent;
                Assert.Equal(ReactorChannelEventType.CHANNEL_READY, channelEvent.EventType);

                consumerReactor.Dispatch(1);
                evt = consumerReactor.PollEvent();
                Assert.Equal(TestReactorEventType.CHANNEL_EVENT, evt.EventType); // consumer receives channel up event
                channelEvent = (ReactorChannelEvent)evt.ReactorEvent;
                Assert.Equal(ReactorChannelEventType.CHANNEL_UP, channelEvent.EventType);

                providerReactor.Dispatch(1); // provider receives Login request again
                evt = providerReactor.PollEvent();
                Assert.Equal(TestReactorEventType.LOGIN_MSG, evt.EventType);
                loginMsgEvent = (RDMLoginMsgEvent)evt.ReactorEvent;
                Assert.Equal(MsgClasses.REQUEST, loginMsgEvent.Msg.MsgClass);
                Assert.Equal(1, loginMsgEvent.Msg.StreamId);
            }

            /*** Now, make sure we don't back off if we DO get past login 
             * (same test as above, except provider accepts the login request before closing it). 
             * Do two attempts.  ***/

            // Provider sends LoginRefresh with OPEN/OK state
            LoginRefresh loginRefresh = new LoginRefresh();
            loginRefresh.StreamId = 1;
            loginRefresh.Solicited = true;
            loginRefresh.SupportedFeatures.HasSupportOptimizedPauseResume = true;
            loginRefresh.SupportedFeatures.SupportOptimizedPauseResume = 1;
            loginRefresh.State.StreamState(StreamStates.OPEN);
            loginRefresh.State.DataState(DataStates.OK);

            submitOptions.Clear();
            Assert.True(provider.Submit(loginRefresh, submitOptions) >= ReactorReturnCode.SUCCESS);

            // Provider sends LoginStatus with CLOSED_RECOVER/SUSPECT state
            loginStatus.Clear();
            loginStatus.StreamId = 1;
            loginStatus.HasState = true;
            loginStatus.State.StreamState(StreamStates.CLOSED_RECOVER);
            loginStatus.State.DataState(DataStates.SUSPECT);

            submitOptions.Clear();
            Assert.True(provider.SubmitAndDispatch(loginStatus, submitOptions) >= ReactorReturnCode.SUCCESS);

            consumerReactor.Dispatch(3);

            // Consumer receives Login Refresh and Status messages
            evt = consumerReactor.PollEvent();
            Assert.Equal(TestReactorEventType.LOGIN_MSG, evt.EventType);
            loginMsgEvent = (RDMLoginMsgEvent)evt.ReactorEvent;
            Assert.Equal(MsgClasses.REFRESH, loginMsgEvent.Msg.MsgClass);
            Assert.Equal(1, loginMsgEvent.Msg.StreamId);
            var refreshMsg = loginMsgEvent.LoginMsg.LoginRefresh;
            Assert.Equal(StreamStates.OPEN, refreshMsg.State.StreamState());
            Assert.Equal(DataStates.OK, refreshMsg.State.DataState());

            evt = consumerReactor.PollEvent();
            Assert.Equal(TestReactorEventType.LOGIN_MSG, evt.EventType);
            loginMsgEvent = (RDMLoginMsgEvent)evt.ReactorEvent;
            Assert.Equal(MsgClasses.STATUS, loginMsgEvent.Msg.MsgClass);
            Assert.Equal(1, loginMsgEvent.Msg.StreamId);
            statusMsg = loginMsgEvent.LoginMsg.LoginStatus;
            Assert.Equal(StreamStates.OPEN, statusMsg.State.StreamState());
            Assert.Equal(DataStates.SUSPECT, statusMsg.State.DataState());

            // Consumer receives channel down reconnecting event
            evt = consumerReactor.PollEvent();
            Assert.Equal(TestReactorEventType.CHANNEL_EVENT, evt.EventType);
            channelEvent = (ReactorChannelEvent)evt.ReactorEvent;
            Assert.Equal(ReactorChannelEventType.CHANNEL_DOWN_RECONNECTING, channelEvent.EventType);

            providerReactor.Dispatch(2);
            evt = providerReactor.PollEvent();
            Assert.Equal(TestReactorEventType.DIRECTORY_MSG, evt.EventType); // provider receives Directory request for default Directory request

            evt = providerReactor.PollEvent();
            Assert.Equal(TestReactorEventType.CHANNEL_EVENT, evt.EventType); // provider receives channel down event
            channelEvent = (ReactorChannelEvent)evt.ReactorEvent;
            Assert.Equal(ReactorChannelEventType.CHANNEL_DOWN, channelEvent.EventType);

            provider.CloseChannel();

            // Recover connection
            provider.TestReactor.Accept(opts, provider, new TimeSpan(0, 0, 1));

            providerReactor.Dispatch(2);
            evt = providerReactor.PollEvent();
            Assert.Equal(TestReactorEventType.CHANNEL_EVENT, evt.EventType); // provider receives channel up event
            channelEvent = (ReactorChannelEvent)evt.ReactorEvent;
            Assert.Equal(ReactorChannelEventType.CHANNEL_UP, channelEvent.EventType);
            evt = providerReactor.PollEvent();
            Assert.Equal(TestReactorEventType.CHANNEL_EVENT, evt.EventType); // provider receives channel ready event
            channelEvent = (ReactorChannelEvent)evt.ReactorEvent;
            Assert.Equal(ReactorChannelEventType.CHANNEL_READY, channelEvent.EventType);

            consumerReactor.Dispatch(1);
            evt = consumerReactor.PollEvent();
            Assert.Equal(TestReactorEventType.CHANNEL_EVENT, evt.EventType); // consumer receives channel up event
            channelEvent = (ReactorChannelEvent)evt.ReactorEvent;
            Assert.Equal(ReactorChannelEventType.CHANNEL_UP, channelEvent.EventType);

            providerReactor.Dispatch(1); // provider receives Login request again
            evt = providerReactor.PollEvent();
            Assert.Equal(TestReactorEventType.LOGIN_MSG, evt.EventType);
            loginMsgEvent = (RDMLoginMsgEvent)evt.ReactorEvent;
            Assert.Equal(MsgClasses.REQUEST, loginMsgEvent.Msg.MsgClass);
            Assert.Equal(1, loginMsgEvent.Msg.StreamId);

            /*** On the second attempt, exchange directory request/refresh through 
             * before sending login status. ***/
            loginRefresh.Clear();
            loginRefresh.StreamId = 1;
            loginRefresh.Solicited = true;
            loginRefresh.SupportedFeatures.HasSupportOptimizedPauseResume = true;
            loginRefresh.SupportedFeatures.SupportOptimizedPauseResume = 1;
            loginRefresh.State.StreamState(StreamStates.OPEN);
            loginRefresh.State.DataState(DataStates.OK);

            submitOptions.Clear();
            Assert.True(provider.Submit(loginRefresh, submitOptions) >= ReactorReturnCode.SUCCESS);

            consumerReactor.Dispatch(1);
            evt = consumerReactor.PollEvent();
            Assert.Equal(TestReactorEventType.LOGIN_MSG, evt.EventType);
            loginMsgEvent = (RDMLoginMsgEvent)evt.ReactorEvent;
            Assert.Equal(MsgClasses.REFRESH, loginMsgEvent.Msg.MsgClass);
            Assert.Equal(1, loginMsgEvent.Msg.StreamId);
            refreshMsg = loginMsgEvent.LoginMsg.LoginRefresh;
            Assert.Equal(StreamStates.OPEN, refreshMsg.State.StreamState());
            Assert.Equal(DataStates.OK, refreshMsg.State.DataState());

            providerReactor.Dispatch(1);
            evt = providerReactor.PollEvent();
            Assert.Equal(TestReactorEventType.DIRECTORY_MSG, evt.EventType);
            msgEvent = (ReactorMsgEvent)evt.ReactorEvent;
            Assert.Equal(MsgClasses.REQUEST, msgEvent.Msg.MsgClass);

            // Provider sends LoginStatus with CLOSED_RECOVER/SUSPECT state
            loginStatus.Clear();
            loginStatus.StreamId = 1;
            loginStatus.HasState = true;
            loginStatus.State.StreamState(StreamStates.CLOSED_RECOVER);
            loginStatus.State.DataState(DataStates.SUSPECT);

            submitOptions.Clear();
            Assert.True(provider.SubmitAndDispatch(loginStatus, submitOptions) >= ReactorReturnCode.SUCCESS);

            consumerReactor.Dispatch(2);

            // Consumer receives Login Status message
            evt = consumerReactor.PollEvent();
            Assert.Equal(TestReactorEventType.LOGIN_MSG, evt.EventType);
            loginMsgEvent = (RDMLoginMsgEvent)evt.ReactorEvent;
            Assert.Equal(MsgClasses.STATUS, loginMsgEvent.Msg.MsgClass);
            Assert.Equal(1, loginMsgEvent.Msg.StreamId);
            statusMsg = loginMsgEvent.LoginMsg.LoginStatus;
            Assert.Equal(StreamStates.OPEN, statusMsg.State.StreamState());
            Assert.Equal(DataStates.SUSPECT, statusMsg.State.DataState());

            // Consumer receives channel down reconnecting event
            evt = consumerReactor.PollEvent();
            Assert.Equal(TestReactorEventType.CHANNEL_EVENT, evt.EventType);
            channelEvent = (ReactorChannelEvent)evt.ReactorEvent;
            Assert.Equal(ReactorChannelEventType.CHANNEL_DOWN_RECONNECTING, channelEvent.EventType);

            providerReactor.Dispatch(1);
            evt = providerReactor.PollEvent();
            Assert.Equal(TestReactorEventType.CHANNEL_EVENT, evt.EventType); // provider receives channel down event
            channelEvent = (ReactorChannelEvent)evt.ReactorEvent;
            Assert.Equal(ReactorChannelEventType.CHANNEL_DOWN, channelEvent.EventType);

            provider.CloseChannel();

            // Recover connection
            provider.TestReactor.Accept(opts, provider, new TimeSpan(0, 0, 1));

            providerReactor.Dispatch(2);
            evt = providerReactor.PollEvent();
            Assert.Equal(TestReactorEventType.CHANNEL_EVENT, evt.EventType); // provider receives channel up event
            evt = providerReactor.PollEvent();
            Assert.Equal(TestReactorEventType.CHANNEL_EVENT, evt.EventType); // provider receives channel ready event

            consumerReactor.Dispatch(1);
            evt = consumerReactor.PollEvent();
            Assert.Equal(TestReactorEventType.CHANNEL_EVENT, evt.EventType); // consumer receives channel up event

            providerReactor.Dispatch(1); // provider receives Login request again
            evt = providerReactor.PollEvent();
            Assert.Equal(TestReactorEventType.LOGIN_MSG, evt.EventType);
            loginMsgEvent = (RDMLoginMsgEvent)evt.ReactorEvent;
            Assert.Equal(MsgClasses.REQUEST, loginMsgEvent.Msg.MsgClass);
            Assert.Equal(1, loginMsgEvent.Msg.StreamId);

            loginRefresh.Clear();
            loginRefresh.StreamId = 1;
            loginRefresh.Solicited = true;
            loginRefresh.SupportedFeatures.HasSupportOptimizedPauseResume = true;
            loginRefresh.SupportedFeatures.SupportOptimizedPauseResume = 1;
            loginRefresh.State.StreamState(StreamStates.OPEN);
            loginRefresh.State.DataState(DataStates.OK);

            submitOptions.Clear();
            Assert.True(provider.Submit(loginRefresh, submitOptions) >= ReactorReturnCode.SUCCESS);

            consumerReactor.Dispatch(1);
            evt = consumerReactor.PollEvent();
            Assert.Equal(TestReactorEventType.LOGIN_MSG, evt.EventType);
            loginMsgEvent = (RDMLoginMsgEvent)evt.ReactorEvent;
            Assert.Equal(MsgClasses.REFRESH, loginMsgEvent.Msg.MsgClass);
            Assert.Equal(1, loginMsgEvent.Msg.StreamId);
            refreshMsg = loginMsgEvent.LoginMsg.LoginRefresh;
            Assert.Equal(StreamStates.OPEN, refreshMsg.State.StreamState());
            Assert.Equal(DataStates.OK, refreshMsg.State.DataState());

            providerReactor.Dispatch(1);
            evt = providerReactor.PollEvent();
            Assert.Equal(TestReactorEventType.DIRECTORY_MSG, evt.EventType);

            DirectoryRefresh directoryRefresh = new DirectoryRefresh();

            directoryRefresh.Clear();
            directoryRefresh.StreamId = ((ReactorMsgEvent)evt.ReactorEvent).Msg.StreamId;
            directoryRefresh.Filter = ALL_FILTERS;
            directoryRefresh.Solicited = true;
            directoryRefresh.ClearCache = true;
            directoryRefresh.State.StreamState(StreamStates.OPEN);
            directoryRefresh.State.DataState(DataStates.OK);
            directoryRefresh.State.Code(StateCodes.NONE);
            directoryRefresh.State.Text().Data("Source Directory Refresh Complete");

            Service service = new();
            Service service2 = new();
            Provider.DefaultService.Copy(service);
            Provider.DefaultService2.Copy(service2);

            directoryRefresh.ServiceList.Add(service);
            directoryRefresh.ServiceList.Add(service2);

            submitOptions.Clear();
            Assert.True(provider.SubmitAndDispatch(directoryRefresh, submitOptions) >= ReactorReturnCode.SUCCESS);

            consumerReactor.Dispatch(2);
            evt = consumerReactor.PollEvent();
            Assert.Equal(TestReactorEventType.DIRECTORY_MSG, evt.EventType);
            msgEvent = (ReactorMsgEvent)evt.ReactorEvent;
            Assert.Equal(MsgClasses.UPDATE, msgEvent.Msg.MsgClass); // We have already received the first refresh during initial session setup

            evt = consumerReactor.PollEvent();
            Assert.Equal(TestReactorEventType.CHANNEL_EVENT, evt.EventType); // Consumer receives channel ready event
            channelEvent = (ReactorChannelEvent)evt.ReactorEvent;
            Assert.Equal(ReactorChannelEventType.CHANNEL_READY, channelEvent.EventType);

            if (singleOpen) // if SingleOpen is true, provider receives request again
            {
                providerReactor.Dispatch(1);
                evt = providerReactor.PollEvent();
                Assert.Equal(TestReactorEventType.MSG, evt.EventType);
            }

            TestReactorComponent.CloseSession(consumer, provider);
            consumerReactor.Close();
            providerReactor.Close();
        }

        [Fact]
        public void WatchlistOneItem_LoginClosedRecoverSingleOpenOnTest()
        {
            WatchlistOneItem_LoginClosedRecover(true);
        }

        [Fact]
        public void WatchlistOneItem_LoginClosedRecoverSingleOpenOffTest()
        {
            WatchlistOneItem_LoginClosedRecover(false);
        }

        [Fact]
        public void WatchlistOneItem_LoginClosedTest()
        {
            ReactorSubmitOptions submitOptions = new ReactorSubmitOptions();
            TestReactorEvent evt;
            ReactorMsgEvent msgEvent;

            ConsumerProviderSessionOptions opts = new ConsumerProviderSessionOptions();
            opts.SetupDefaultLoginStream = true;
            opts.SetupDefaultDirectoryStream = true;

            SetupSession(opts, true, 1, false, 0, out var consumerReactor, out var providerReactor, out var consumer, out var provider);

            // Request first item
            IRequestMsg requestMsg = new Msg();
            requestMsg.Clear();
            requestMsg.MsgClass = MsgClasses.REQUEST;
            requestMsg.StreamId = 5;
            requestMsg.DomainType = (int)DomainType.MARKET_PRICE;
            requestMsg.ApplyStreaming();
            requestMsg.MsgKey.ApplyHasName();
            requestMsg.MsgKey.Name.Data("TRI.N");
            requestMsg.ApplyHasQos();
            requestMsg.Qos.Timeliness(QosTimeliness.REALTIME);
            requestMsg.Qos.Rate(QosRates.TICK_BY_TICK);
            submitOptions.Clear();
            submitOptions.ServiceName = Provider.DefaultService.Info.ServiceName.ToString();
            Assert.True(consumer.Submit((Msg)requestMsg, submitOptions) >= ReactorReturnCode.SUCCESS);

            providerReactor.Dispatch(1);

            evt = providerReactor.PollEvent();
            Assert.Equal(TestReactorEventType.MSG, evt.EventType);
            msgEvent = (ReactorMsgEvent)evt.ReactorEvent;
            Assert.Equal(MsgClasses.REQUEST, msgEvent.Msg.MsgClass);
            var reqMsg = (IRequestMsg)msgEvent.Msg;
            Assert.True(reqMsg.MsgKey.CheckHasName());
            Assert.Equal("TRI.N", reqMsg.MsgKey.Name.ToString());

            LoginStatus loginStatus = new LoginStatus();
            loginStatus.StreamId = 1;
            loginStatus.HasState = true;
            loginStatus.State.StreamState(StreamStates.CLOSED);
            loginStatus.State.DataState(DataStates.SUSPECT);

            submitOptions.Clear();
            Assert.True(provider.Submit(loginStatus, submitOptions) >= ReactorReturnCode.SUCCESS);

            consumerReactor.Dispatch(3);
            evt = consumerReactor.PollEvent();
            Assert.Equal(TestReactorEventType.LOGIN_MSG, evt.EventType);
            var loginMsgEvent = (RDMLoginMsgEvent)evt.ReactorEvent;
            Assert.Equal(MsgClasses.STATUS, loginMsgEvent.Msg.MsgClass);
            var loginStatusRdm = loginMsgEvent.LoginMsg.LoginStatus;
            Assert.True(loginStatusRdm.HasState);
            Assert.Equal(StreamStates.CLOSED, loginStatusRdm.State.StreamState());
            Assert.Equal(DataStates.SUSPECT, loginStatusRdm.State.DataState());

            evt = consumerReactor.PollEvent();
            Assert.Equal(TestReactorEventType.MSG, evt.EventType);
            msgEvent = (ReactorMsgEvent)evt.ReactorEvent;
            Assert.Equal(MsgClasses.STATUS, msgEvent.Msg.MsgClass);
            var status = (IStatusMsg)msgEvent.Msg;
            Assert.True(status.CheckHasState());
            Assert.Equal(StreamStates.CLOSED, status.State.StreamState());
            Assert.Equal(DataStates.SUSPECT, status.State.DataState());

            evt = consumerReactor.PollEvent();
            Assert.Equal(TestReactorEventType.DIRECTORY_MSG, evt.EventType);
            msgEvent = (ReactorMsgEvent)evt.ReactorEvent;
            Assert.Equal(MsgClasses.UPDATE, msgEvent.Msg.MsgClass);

            consumerReactor.Dispatch(0);
            providerReactor.Dispatch(0);

            /*** Make sure we can log back in, and that the items are not recovered. ***/

            submitOptions.Clear();
            Assert.True(consumer.SubmitAndDispatch(((ConsumerRole)consumerReactor.ComponentList[0].ReactorRole).RdmLoginRequest, submitOptions) >= ReactorReturnCode.SUCCESS);

            providerReactor.Dispatch(1);
            evt = providerReactor.PollEvent();
            Assert.Equal(TestReactorEventType.LOGIN_MSG, evt.EventType);
            loginMsgEvent = (RDMLoginMsgEvent)evt.ReactorEvent;
            Assert.Equal(MsgClasses.REQUEST, loginMsgEvent.Msg.MsgClass);

            LoginRefresh loginRefresh = new LoginRefresh();
            loginRefresh.Clear();
            loginRefresh.StreamId = 1;
            loginRefresh.Solicited = true;
            loginRefresh.SupportedFeatures.HasSupportOptimizedPauseResume = true;
            loginRefresh.SupportedFeatures.SupportOptimizedPauseResume = 1;
            loginRefresh.State.StreamState(StreamStates.OPEN);
            loginRefresh.State.DataState(DataStates.OK);

            submitOptions.Clear();
            Assert.True(provider.SubmitAndDispatch(loginRefresh, submitOptions) >= ReactorReturnCode.SUCCESS);

            consumerReactor.Dispatch(1);
            evt = consumerReactor.PollEvent();
            Assert.Equal(TestReactorEventType.LOGIN_MSG, evt.EventType);
            loginMsgEvent = (RDMLoginMsgEvent)evt.ReactorEvent;
            Assert.Equal(MsgClasses.REFRESH, loginMsgEvent.Msg.MsgClass);

            providerReactor.Dispatch(1);
            evt = providerReactor.PollEvent();
            Assert.Equal(TestReactorEventType.DIRECTORY_MSG, evt.EventType);

            DirectoryRefresh directoryRefresh = new DirectoryRefresh();

            directoryRefresh.Clear();
            directoryRefresh.StreamId = ((ReactorMsgEvent)evt.ReactorEvent).Msg.StreamId;
            directoryRefresh.Filter = ALL_FILTERS;
            directoryRefresh.Solicited = true;
            directoryRefresh.ClearCache = true;
            directoryRefresh.State.StreamState(StreamStates.OPEN);
            directoryRefresh.State.DataState(DataStates.OK);
            directoryRefresh.State.Code(StateCodes.NONE);
            directoryRefresh.State.Text().Data("Source Directory Refresh Complete");

            Service service = new();
            Service service2 = new();
            Provider.DefaultService.Copy(service);
            Provider.DefaultService2.Copy(service2);

            directoryRefresh.ServiceList.Add(service);
            directoryRefresh.ServiceList.Add(service2);

            submitOptions.Clear();
            Assert.True(provider.SubmitAndDispatch(directoryRefresh, submitOptions) >= ReactorReturnCode.SUCCESS);

            consumerReactor.Dispatch(1);
            evt = consumerReactor.PollEvent();
            Assert.Equal(TestReactorEventType.DIRECTORY_MSG, evt.EventType);
            msgEvent = (ReactorMsgEvent)evt.ReactorEvent;
            Assert.Equal(MsgClasses.UPDATE, msgEvent.Msg.MsgClass);

            submitOptions.Clear();
            submitOptions.ServiceName = Provider.DefaultService.Info.ServiceName.ToString();
            Assert.True(consumer.Submit((Msg)requestMsg, submitOptions) >= ReactorReturnCode.SUCCESS);

            providerReactor.Dispatch(1);
            evt = providerReactor.PollEvent();
            Assert.Equal(TestReactorEventType.MSG, evt.EventType);
            msgEvent = (ReactorMsgEvent)evt.ReactorEvent;
            Assert.Equal(MsgClasses.REQUEST, msgEvent.Msg.MsgClass);
            reqMsg = (IRequestMsg)msgEvent.Msg;
            Assert.True(reqMsg.MsgKey.CheckHasName());
            Assert.Equal("TRI.N", reqMsg.MsgKey.Name.ToString());

            TestReactorComponent.CloseSession(consumer, provider);
            consumerReactor.Close();
            providerReactor.Close();
        }

        [Fact]
        public void WatchlistOneItem_LoginClosed_DirectoryTest()
        {
            ReactorSubmitOptions submitOptions = new ReactorSubmitOptions();
            TestReactorEvent evt;

            TestReactor consumerReactor = new TestReactor();
            TestReactor providerReactor = new TestReactor();

            Consumer consumer = new Consumer(consumerReactor);
            ConsumerRole consumerRole = (ConsumerRole)consumer.ReactorRole;
            consumerRole.ChannelEventCallback = consumer;
            consumerRole.LoginMsgCallback = consumer;
            consumerRole.DirectoryMsgCallback = consumer;
            consumerRole.DictionaryMsgCallback = consumer;
            consumerRole.DefaultMsgCallback = consumer;
            consumerRole.WatchlistOptions.EnableWatchlist = true;
            consumerRole.WatchlistOptions.ChannelOpenEventCallback = consumer;
            consumerRole.WatchlistOptions.RequestTimeout = 2500;

            Provider provider = new Provider(providerReactor);
            ProviderRole providerRole = (ProviderRole)provider.ReactorRole;
            providerRole.ChannelEventCallback = provider;
            providerRole.LoginMsgCallback = provider;
            providerRole.DirectoryMsgCallback = provider;
            providerRole.DictionaryMsgCallback = provider;
            providerRole.DefaultMsgCallback = provider;

            ConsumerProviderSessionOptions opts = new ConsumerProviderSessionOptions();
            opts.SetupDefaultLoginStream = false;
            opts.SetupDefaultDirectoryStream = false;

            provider.Bind(opts);

            RDMLoginMsgEvent loginMsgEvent;
            RDMDirectoryMsgEvent directoryMsgEvent;

            consumer.TestReactor.Connect(opts, consumer, provider.ServerPort);
            consumer.TestReactor.PollEvent();

            provider.TestReactor.Accept(opts, provider);
            provider.TestReactor.Dispatch(2);
            provider.TestReactor.PollEvent();
            provider.TestReactor.PollEvent();

            consumer.TestReactor.Dispatch(2);
            consumer.TestReactor.PollEvent();
            consumer.TestReactor.PollEvent();

            provider.TestReactor.Dispatch(0);

            LoginRequest loginRequest = new LoginRequest();
            loginRequest.InitDefaultRequest(1);
            loginRequest.HasAttrib = true;
            loginRequest.LoginAttrib.HasSingleOpen = true;
            loginRequest.LoginAttrib.SingleOpen = 1;

            submitOptions.Clear();
            Assert.True(consumer.SubmitAndDispatch(loginRequest, submitOptions) >= ReactorReturnCode.SUCCESS);

            provider.TestReactor.Dispatch(1);
            evt = provider.TestReactor.PollEvent();
            Assert.Equal(TestReactorEventType.LOGIN_MSG, evt.EventType);
            loginMsgEvent = (RDMLoginMsgEvent)evt.ReactorEvent;
            Assert.Equal(LoginMsgType.REQUEST, loginMsgEvent.LoginMsg.LoginMsgType);

            LoginRefresh loginRefresh = new LoginRefresh();

            loginRefresh.Clear();
            loginRefresh.Solicited = true;
            loginRefresh.UserName = loginRequest.UserName;
            loginRefresh.StreamId = loginRequest.StreamId;
            loginRefresh.HasFeatures = true;
            loginRefresh.SupportedFeatures.HasSupportOptimizedPauseResume = true;
            loginRefresh.SupportedFeatures.SupportOptimizedPauseResume = 1;
            loginRefresh.SupportedFeatures.HasSupportViewRequests = true;
            loginRefresh.SupportedFeatures.SupportViewRequests = 1;
            loginRefresh.SupportedFeatures.HasSupportPost = true;
            loginRefresh.SupportedFeatures.SupportOMMPost = 1;

            loginRefresh.SupportedFeatures.HasSupportEnhancedSymbolList = true;
            loginRefresh.SupportedFeatures.SupportEnhancedSymbolList = 1;

            loginRefresh.State.StreamState(StreamStates.OPEN);
            loginRefresh.State.DataState(DataStates.OK);
            loginRefresh.State.Code(StateCodes.NONE);
            loginRefresh.State.Text().Data("Login OK");

            submitOptions.Clear();
            Assert.True(provider.SubmitAndDispatch(loginRefresh, submitOptions) >= ReactorReturnCode.SUCCESS);

            consumer.TestReactor.Dispatch(1);
            evt = consumer.TestReactor.PollEvent();
            Assert.Equal(TestReactorEventType.LOGIN_MSG, evt.EventType);
            loginMsgEvent = (RDMLoginMsgEvent)evt.ReactorEvent;
            Assert.Equal(MsgClasses.REFRESH, loginMsgEvent.Msg.MsgClass);

            provider.TestReactor.Dispatch(1);
            evt = provider.TestReactor.PollEvent();
            Assert.Equal(TestReactorEventType.DIRECTORY_MSG, evt.EventType);
            directoryMsgEvent = (RDMDirectoryMsgEvent)evt.ReactorEvent;
            Assert.Equal(DirectoryMsgType.REQUEST, directoryMsgEvent.DirectoryMsg.DirectoryMsgType);

            LoginStatus loginStatus = new LoginStatus();
            loginStatus.StreamId = 1;
            loginStatus.HasState = true;
            loginStatus.State.StreamState(StreamStates.CLOSED);
            loginStatus.State.DataState(DataStates.SUSPECT);

            submitOptions.Clear();
            Assert.True(provider.SubmitAndDispatch(loginStatus, submitOptions) >= ReactorReturnCode.SUCCESS);

            consumerReactor.Dispatch(1);
            evt = consumerReactor.PollEvent();
            Assert.Equal(TestReactorEventType.LOGIN_MSG, evt.EventType);
            loginMsgEvent = (RDMLoginMsgEvent)evt.ReactorEvent;
            Assert.Equal(MsgClasses.STATUS, loginMsgEvent.Msg.MsgClass);
            var loginStatusRdm = loginMsgEvent.LoginMsg.LoginStatus;
            Assert.True(loginStatusRdm.HasState);
            Assert.Equal(StreamStates.CLOSED, loginStatusRdm.State.StreamState());
            Assert.Equal(DataStates.SUSPECT, loginStatusRdm.State.DataState());

            // Ensure that no timeouts fire.
            consumerReactor.Dispatch(0, new TimeSpan(0, 0, 3));

            providerReactor.Dispatch(0);

            // Consumer logs back in
            submitOptions.Clear();
            Assert.True(consumer.SubmitAndDispatch(loginRequest, submitOptions) >= ReactorReturnCode.SUCCESS);

            provider.TestReactor.Dispatch(1);
            evt = provider.TestReactor.PollEvent();
            Assert.Equal(TestReactorEventType.LOGIN_MSG, evt.EventType);
            loginMsgEvent = (RDMLoginMsgEvent)evt.ReactorEvent;
            Assert.Equal(LoginMsgType.REQUEST, loginMsgEvent.LoginMsg.LoginMsgType);

            submitOptions.Clear();
            Assert.True(provider.SubmitAndDispatch(loginRefresh, submitOptions) >= ReactorReturnCode.SUCCESS);

            consumer.TestReactor.Dispatch(1);
            evt = consumer.TestReactor.PollEvent();
            Assert.Equal(TestReactorEventType.LOGIN_MSG, evt.EventType);

            provider.TestReactor.Dispatch(1);
            evt = provider.TestReactor.PollEvent();
            Assert.Equal(TestReactorEventType.DIRECTORY_MSG, evt.EventType);
            directoryMsgEvent = (RDMDirectoryMsgEvent)evt.ReactorEvent;
            Assert.Equal(DirectoryMsgType.REQUEST, directoryMsgEvent.DirectoryMsg.DirectoryMsgType);

            TestReactorComponent.CloseSession(consumer, provider);
            consumerReactor.Close();
            providerReactor.Close();
        }

        private void WatchlistOneItem_LoginCloseRequest(bool useRssl)
        {
            ReactorSubmitOptions submitOptions = new ReactorSubmitOptions();
            TestReactorEvent evt;
            ReactorMsgEvent msgEvent;

            ConsumerProviderSessionOptions opts = new ConsumerProviderSessionOptions();
            opts.SetupDefaultLoginStream = true;
            opts.SetupDefaultDirectoryStream = true;

            SetupSession(opts, true, 1, false, 0, out var consumerReactor, out var providerReactor, out var consumer, out var provider);

            // Request first item
            IRequestMsg requestMsg = new Msg();
            requestMsg.Clear();
            requestMsg.MsgClass = MsgClasses.REQUEST;
            requestMsg.StreamId = 5;
            requestMsg.DomainType = (int)DomainType.MARKET_PRICE;
            requestMsg.ApplyStreaming();
            requestMsg.MsgKey.ApplyHasName();
            requestMsg.MsgKey.Name.Data("TRI.N");
            requestMsg.ApplyHasQos();
            requestMsg.Qos.Timeliness(QosTimeliness.REALTIME);
            requestMsg.Qos.Rate(QosRates.TICK_BY_TICK);
            submitOptions.Clear();
            submitOptions.ServiceName = Provider.DefaultService.Info.ServiceName.ToString();
            Assert.True(consumer.Submit((Msg)requestMsg, submitOptions) >= ReactorReturnCode.SUCCESS);

            providerReactor.Dispatch(1);

            evt = providerReactor.PollEvent();
            Assert.Equal(TestReactorEventType.MSG, evt.EventType);
            msgEvent = (ReactorMsgEvent)evt.ReactorEvent;
            Assert.Equal(MsgClasses.REQUEST, msgEvent.Msg.MsgClass);
            var reqMsg = (IRequestMsg)msgEvent.Msg;
            Assert.True(reqMsg.MsgKey.CheckHasName());
            Assert.Equal("TRI.N", reqMsg.MsgKey.Name.ToString());
            int providerItemStream = reqMsg.StreamId;

            if (useRssl)
            {
                Msg closeMsg = new Msg();
                closeMsg.DomainType = (int)DomainType.LOGIN;
                closeMsg.MsgClass = MsgClasses.CLOSE;
                closeMsg.StreamId = 1;

                submitOptions.Clear();
                Assert.True(consumer.Submit(closeMsg, submitOptions) >= ReactorReturnCode.SUCCESS);
            }
            else
            {
                LoginClose loginClose = new LoginClose();
                loginClose.StreamId = 1;

                submitOptions.Clear();
                Assert.True(consumer.Submit(loginClose, submitOptions) >= ReactorReturnCode.SUCCESS);
            }

            // consumer calls back to the user with item stream status msg immediately after login close was submitted 
            Assert.True(consumerReactor.m_EventQueue.Count > 0);
            evt = consumerReactor.PollEvent();
            Assert.Equal(TestReactorEventType.MSG, evt.EventType);
            msgEvent = (ReactorMsgEvent)evt.ReactorEvent;
            Assert.Equal(MsgClasses.STATUS, msgEvent.Msg.MsgClass);
            Assert.Equal(5, msgEvent.Msg.StreamId);

            evt = consumerReactor.PollEvent(); // delete services update for the default Reactor directory request
            Assert.Equal(TestReactorEventType.DIRECTORY_MSG, evt.EventType);
            msgEvent = (ReactorMsgEvent)evt.ReactorEvent;
            Assert.Equal(MsgClasses.UPDATE, msgEvent.Msg.MsgClass);

            consumerReactor.Dispatch(0);

            IRefreshMsg refreshMsg = new Msg();
            refreshMsg.DomainType = (int)DomainType.MARKET_PRICE;
            refreshMsg.MsgClass = MsgClasses.REFRESH;
            refreshMsg.StreamId = providerItemStream;
            refreshMsg.ApplyHasMsgKey();
            refreshMsg.ApplyClearCache();
            refreshMsg.ApplyRefreshComplete();
            refreshMsg.ApplyHasQos();
            refreshMsg.MsgKey.ApplyHasName();
            refreshMsg.MsgKey.Name.Data("TRI.N");
            refreshMsg.MsgKey.ApplyHasServiceId();
            refreshMsg.MsgKey.ServiceId = Provider.DefaultService.ServiceId;
            refreshMsg.Qos.Timeliness(QosTimeliness.REALTIME);
            refreshMsg.Qos.Rate(QosRates.TICK_BY_TICK);
            refreshMsg.ApplySolicited();
            refreshMsg.ContainerType = DataTypes.NO_DATA;
            refreshMsg.State.StreamState(StreamStates.OPEN);
            refreshMsg.State.DataState(DataStates.OK);

            submitOptions.Clear();
            Assert.True(provider.Submit((Msg)refreshMsg, submitOptions) >= ReactorReturnCode.SUCCESS);

            providerReactor.Dispatch(1);
            evt = providerReactor.PollEvent();
            Assert.Equal(TestReactorEventType.LOGIN_MSG, evt.EventType);
            msgEvent = (ReactorMsgEvent)evt.ReactorEvent;
            Assert.Equal(MsgClasses.CLOSE, msgEvent.Msg.MsgClass);

            // Consumer should not get the refresh
            consumerReactor.Dispatch(0);
            // Provider should not get any messages in response
            providerReactor.Dispatch(0);

            TestReactorComponent.CloseSession(consumer, provider);
            consumerReactor.Close();
            providerReactor.Close();
        }

        [Fact]
        public void WatchlistOneItem_LoginCloseRequestRsslTest()
        {
            WatchlistOneItem_LoginCloseRequest(true);
        }

        [Fact]
        public void WatchlistOneItem_LoginCloseRequestRdmTest()
        {
            WatchlistOneItem_LoginCloseRequest(false);
        }

        [Fact]
        public void WatchlistOneItem_DirectoryClosedRecoverTest()
        {
            ReactorSubmitOptions submitOptions = new ReactorSubmitOptions();
            TestReactorEvent evt;
            ReactorMsgEvent msgEvent;

            ConsumerProviderSessionOptions opts = new ConsumerProviderSessionOptions();
            opts.SetupDefaultLoginStream = true;
            opts.SetupDefaultDirectoryStream = true;
            opts.ReconnectAttemptLimit = 3;

            SetupSession(opts, true, 1, false, 0, out var consumerReactor, out var providerReactor, out var consumer, out var provider);

            // Request first item
            IRequestMsg requestMsg = new Msg();
            requestMsg.Clear();
            requestMsg.MsgClass = MsgClasses.REQUEST;
            requestMsg.StreamId = 5;
            requestMsg.DomainType = (int)DomainType.MARKET_PRICE;
            requestMsg.ApplyStreaming();
            requestMsg.MsgKey.ApplyHasName();
            requestMsg.MsgKey.Name.Data("TRI.N");
            requestMsg.ApplyHasQos();
            requestMsg.Qos.Timeliness(QosTimeliness.REALTIME);
            requestMsg.Qos.Rate(QosRates.TICK_BY_TICK);
            submitOptions.Clear();
            submitOptions.ServiceName = Provider.DefaultService.Info.ServiceName.ToString();
            Assert.True(consumer.Submit((Msg)requestMsg, submitOptions) >= ReactorReturnCode.SUCCESS);

            providerReactor.Dispatch(1);

            evt = providerReactor.PollEvent();
            Assert.Equal(TestReactorEventType.MSG, evt.EventType);
            msgEvent = (ReactorMsgEvent)evt.ReactorEvent;
            Assert.Equal(MsgClasses.REQUEST, msgEvent.Msg.MsgClass);
            var reqMsg = (IRequestMsg)msgEvent.Msg;
            Assert.True(reqMsg.MsgKey.CheckHasName());
            Assert.Equal("TRI.N", reqMsg.MsgKey.Name.ToString());

            DirectoryStatus directoryStatus = new DirectoryStatus();
            directoryStatus.StreamId = 2;
            directoryStatus.HasState = true;
            directoryStatus.HasFilter = true;
            directoryStatus.State.StreamState(StreamStates.CLOSED_RECOVER);
            directoryStatus.State.DataState(DataStates.SUSPECT);
            directoryStatus.Filter = ALL_FILTERS;

            submitOptions.Clear();
            Assert.True(provider.SubmitAndDispatch(directoryStatus, submitOptions) >= ReactorReturnCode.SUCCESS);

            consumerReactor.Dispatch(2);
            evt = consumerReactor.PollEvent(); // directory update to DELETE service
            Assert.Equal(TestReactorEventType.DIRECTORY_MSG, evt.EventType);
            msgEvent = (ReactorMsgEvent)evt.ReactorEvent;
            Assert.Equal(MsgClasses.UPDATE, msgEvent.Msg.MsgClass);

            evt = consumerReactor.PollEvent();
            Assert.Equal(TestReactorEventType.MSG, evt.EventType);
            msgEvent = (ReactorMsgEvent)evt.ReactorEvent;
            Assert.Equal(MsgClasses.STATUS, msgEvent.Msg.MsgClass);
            var statusMsg = (IStatusMsg)msgEvent.Msg;
            Assert.True(statusMsg.CheckHasState());
            Assert.Equal(StreamStates.OPEN, statusMsg.State.StreamState());
            Assert.Equal(DataStates.SUSPECT, statusMsg.State.DataState());

            providerReactor.Dispatch(1); // Provider gets Directory Request
            evt = providerReactor.PollEvent();
            Assert.Equal(TestReactorEventType.DIRECTORY_MSG, evt.EventType);
            msgEvent = (ReactorMsgEvent)evt.ReactorEvent;
            Assert.Equal(MsgClasses.REQUEST, msgEvent.Msg.MsgClass);

            DirectoryRefresh directoryRefresh = new DirectoryRefresh();

            directoryRefresh.Clear();
            directoryRefresh.StreamId = ((ReactorMsgEvent)evt.ReactorEvent).Msg.StreamId;
            directoryRefresh.Filter = ALL_FILTERS;
            directoryRefresh.Solicited = true;
            directoryRefresh.ClearCache = true;
            directoryRefresh.State.StreamState(StreamStates.OPEN);
            directoryRefresh.State.DataState(DataStates.OK);
            directoryRefresh.State.Code(StateCodes.NONE);
            directoryRefresh.State.Text().Data("Source Directory Refresh Complete");

            Service service = new();
            Service service2 = new();
            Provider.DefaultService.Copy(service);
            Provider.DefaultService2.Copy(service2);

            directoryRefresh.ServiceList.Add(service);
            directoryRefresh.ServiceList.Add(service2);

            submitOptions.Clear();
            Assert.True(provider.SubmitAndDispatch(directoryRefresh, submitOptions) >= ReactorReturnCode.SUCCESS);

            consumerReactor.Dispatch(1);
            evt = consumerReactor.PollEvent();
            Assert.Equal(TestReactorEventType.DIRECTORY_MSG, evt.EventType);
            msgEvent = (ReactorMsgEvent)evt.ReactorEvent;
            Assert.Equal(MsgClasses.UPDATE, msgEvent.Msg.MsgClass);

            providerReactor.Dispatch(1);
            evt = providerReactor.PollEvent();
            Assert.Equal(TestReactorEventType.MSG, evt.EventType);
            msgEvent = (ReactorMsgEvent)evt.ReactorEvent;
            Assert.Equal(MsgClasses.REQUEST, msgEvent.Msg.MsgClass);
            reqMsg = (IRequestMsg)msgEvent.Msg;
            Assert.True(reqMsg.MsgKey.CheckHasName());
            Assert.Equal("TRI.N", reqMsg.MsgKey.Name.ToString());
        }

        [Fact]
        public void WatchlistUnknownStreamTest()
        {
            ReactorSubmitOptions submitOptions = new ReactorSubmitOptions();
            TestReactorEvent evt;
            ReactorMsgEvent msgEvent;

            ConsumerProviderSessionOptions opts = new ConsumerProviderSessionOptions();
            opts.SetupDefaultLoginStream = true;
            opts.SetupDefaultDirectoryStream = true;
            opts.ReconnectAttemptLimit = 3;

            SetupSession(opts, true, 1, false, 0, out var consumerReactor, out var providerReactor, out var consumer, out var provider);

            Msg statusMsg = new Msg();
            statusMsg.MsgClass = MsgClasses.STATUS;
            statusMsg.DomainType = (int)DomainType.MARKET_PRICE;
            statusMsg.StreamId = 5; // unknown stream
            statusMsg.ApplyHasState();
            statusMsg.State.StreamState(StreamStates.OPEN);
            statusMsg.State.DataState(DataStates.SUSPECT);
            submitOptions.Clear();
            Assert.True(provider.SubmitAndDispatch(statusMsg, submitOptions) >= ReactorReturnCode.SUCCESS);

            consumerReactor.Dispatch(0); // consumer receives nothing

            providerReactor.Dispatch(0); // provider receives no messages

            statusMsg.State.StreamState(StreamStates.CLOSED_RECOVER);
            submitOptions.Clear();
            Assert.True(provider.SubmitAndDispatch(statusMsg, submitOptions) >= ReactorReturnCode.SUCCESS);

            consumerReactor.Dispatch(0); // consumer receives nothing but sends CLOSE message to the unknown stream

            providerReactor.Dispatch(1); // consumer's watchlist sends CLOSE message
            evt = providerReactor.PollEvent();
            Assert.Equal(TestReactorEventType.MSG, evt.EventType);
            msgEvent = (ReactorMsgEvent)evt.ReactorEvent;
            Assert.Equal(MsgClasses.CLOSE, msgEvent.Msg.MsgClass);
            Assert.Equal((int)DomainType.MARKET_PRICE, msgEvent.Msg.DomainType);
            Assert.Equal(5, msgEvent.Msg.StreamId);

            TestReactorComponent.CloseSession(consumer, provider);
            consumerReactor.Close();
            providerReactor.Close();
        }

        private void SetupSession(ConsumerProviderSessionOptions opts,
            bool hasSingleOpen, int singleOpen, bool hasAllowSuspectData, int allowSuspectData,
            out TestReactor consumerReactor, out TestReactor providerReactor,
            out Consumer consumer, out Provider provider, uint requestTimeout = 3000)
        {
            consumerReactor = new TestReactor();
            providerReactor = new TestReactor();

            consumer = new Consumer(consumerReactor);
            ConsumerRole consumerRole = (ConsumerRole)consumer.ReactorRole;
            consumerRole.InitDefaultRDMLoginRequest();
            consumerRole.RdmLoginRequest.HasAttrib = true;
            if (hasSingleOpen)
            {
                consumerRole.RdmLoginRequest.LoginAttrib.HasSingleOpen = true;
                consumerRole.RdmLoginRequest.LoginAttrib.SingleOpen = singleOpen;
            }
            if (hasAllowSuspectData)
            {
                consumerRole.RdmLoginRequest.LoginAttrib.HasAllowSuspectData = true;
                consumerRole.RdmLoginRequest.LoginAttrib.AllowSuspectData = allowSuspectData;
            }

            consumerRole.InitDefaultRDMDirectoryRequest();
            consumerRole.ChannelEventCallback = consumer;
            consumerRole.LoginMsgCallback = consumer;
            consumerRole.DirectoryMsgCallback = consumer;
            consumerRole.DictionaryMsgCallback = consumer;
            consumerRole.DefaultMsgCallback = consumer;
            consumerRole.WatchlistOptions.EnableWatchlist = true;
            consumerRole.WatchlistOptions.ChannelOpenEventCallback = consumer;
            consumerRole.WatchlistOptions.RequestTimeout = requestTimeout;

            provider = new Provider(providerReactor);
            ProviderRole providerRole = (ProviderRole)provider.ReactorRole;
            providerRole.ChannelEventCallback = provider;
            providerRole.LoginMsgCallback = provider;
            providerRole.DirectoryMsgCallback = provider;
            providerRole.DictionaryMsgCallback = provider;
            providerRole.DefaultMsgCallback = provider;

            provider.Bind(opts);
            TestReactor.OpenSession(consumer, provider, opts);
        }

        [Fact]
        public void WatchlistTwoItems_ElementViewFromMsgBufferTest()
        {
            ReactorSubmitOptions submitOptions = new();
            TestReactorEvent evt;
            ReactorMsgEvent msgEvent;
            Msg msg = new();
            IRequestMsg requestMsg = msg;
            IRefreshMsg refreshMsg = msg;

            /* Create reactors. */
            TestReactor consumerReactor = new();
            TestReactor providerReactor = new();

            /* Create consumer. */
            Consumer consumer = new(consumerReactor);
            ConsumerRole consumerRole = (ConsumerRole)consumer.ReactorRole;
            consumerRole.InitDefaultRDMLoginRequest();
            consumerRole.InitDefaultRDMDirectoryRequest();
            consumerRole.ChannelEventCallback = consumer;
            consumerRole.LoginMsgCallback = consumer;
            consumerRole.DirectoryMsgCallback = consumer;
            consumerRole.DictionaryMsgCallback = consumer;
            consumerRole.DefaultMsgCallback = consumer;
            consumerRole.WatchlistOptions.EnableWatchlist = true;
            consumerRole.WatchlistOptions.ChannelOpenEventCallback = consumer;
            consumerRole.WatchlistOptions.RequestTimeout = 15000;

            /* Create provider. */
            Provider provider = new(providerReactor);
            ProviderRole providerRole = (ProviderRole)provider.ReactorRole;
            providerRole.ChannelEventCallback = provider;
            providerRole.LoginMsgCallback = provider;
            providerRole.DirectoryMsgCallback = provider;
            providerRole.DictionaryMsgCallback = provider;
            providerRole.DefaultMsgCallback = provider;

            /* Connect the consumer and provider. Setup login & directory streams automatically. */
            ConsumerProviderSessionOptions opts = new();
            opts.SetupDefaultLoginStream = true;
            opts.SetupDefaultDirectoryStream = true;

            provider.Bind(opts);

            TestReactor.OpenSession(consumer, provider, opts);

            List<string> view1List = new List<string> { "1111", "4444", "5555" };
            List<string> view1_1List = new List<string> { "1111", "4444" };
            List<string> view2List = new List<string> { "4444", "3333", "3333" };

            List<string> combinedViewList = new List<string> { "1111", "3333", "4444" };

            int[] consStreamIds = { 5, 6 };

            /* Request first item. */
            requestMsg.Clear();
            requestMsg.StreamId = consStreamIds[0];
            requestMsg.MsgClass = MsgClasses.REQUEST;
            requestMsg.DomainType = (int)DomainType.MARKET_PRICE;
            requestMsg.ContainerType = DataTypes.NO_DATA;
            requestMsg.ApplyStreaming();
            requestMsg.ApplyHasQos();
            requestMsg.Qos.Timeliness(QosTimeliness.REALTIME);
            requestMsg.Qos.Rate(QosRates.TICK_BY_TICK);
            requestMsg.ApplyHasView();
            WatchlistItemDomainsTest.EncodeViewElementNameList(consumer.ReactorChannel, view1List, requestMsg);

            submitOptions.Clear();
            submitOptions.ServiceName = Provider.DefaultService.Info.ServiceName.ToString();
            submitOptions.RequestMsgOptions.UserSpecObj = "1111";
            Assert.True(consumer.SubmitAndDispatch((Msg)requestMsg, submitOptions) >= ReactorReturnCode.SUCCESS);

            /* Provider receives request. */
            providerReactor.Dispatch(1);
            evt = providerReactor.PollEvent();
            Assert.Equal(TestReactorEventType.MSG, evt.EventType);
            msgEvent = (ReactorMsgEvent)evt.ReactorEvent;
            Assert.Equal(MsgClasses.REQUEST, msgEvent.Msg.MsgClass);
            IRequestMsg receivedRequestMsg = (IRequestMsg)msgEvent.Msg;
            Assert.True(receivedRequestMsg.CheckStreaming());
            Assert.True(receivedRequestMsg.CheckHasPriority());
            Assert.Equal(1, receivedRequestMsg.Priority.PriorityClass);
            Assert.Equal(1, receivedRequestMsg.Priority.Count);
            Assert.False(receivedRequestMsg.CheckNoRefresh());
            Assert.True(WatchlistItemDomainsTest.CheckHasCorrectViewEname(provider, receivedRequestMsg, view1List));
            int providerStreamId = receivedRequestMsg.StreamId;

            /* Provider sends refresh, satisfying first view. */
            refreshMsg.Clear();
            refreshMsg.MsgClass = MsgClasses.REFRESH;
            refreshMsg.DomainType = (int)DomainType.MARKET_PRICE;
            refreshMsg.StreamId = providerStreamId;
            refreshMsg.ContainerType = DataTypes.NO_DATA;
            refreshMsg.ApplyRefreshComplete();
            refreshMsg.ApplySolicited();
            refreshMsg.ApplyClearCache();
            refreshMsg.State.StreamState(StreamStates.OPEN);
            refreshMsg.State.DataState(DataStates.OK);
            requestMsg.ApplyHasQos();
            refreshMsg.Qos.Timeliness(QosTimeliness.REALTIME);
            refreshMsg.Qos.Rate(QosRates.TICK_BY_TICK);

            submitOptions.Clear();
            Assert.True(provider.SubmitAndDispatch((Msg)refreshMsg, submitOptions) >= ReactorReturnCode.SUCCESS);

            /* Consumer receives refresh. */
            consumerReactor.Dispatch(1);
            evt = consumerReactor.PollEvent();
            Assert.Equal(TestReactorEventType.MSG, evt.EventType);
            msgEvent = (ReactorMsgEvent)evt.ReactorEvent;
            Assert.Equal(MsgClasses.REFRESH, msgEvent.Msg.MsgClass);
            IRefreshMsg receivedRefreshMsg = (IRefreshMsg)msgEvent.Msg;
            Assert.Equal((int)DomainType.MARKET_PRICE, receivedRefreshMsg.DomainType);
            Assert.Equal(DataTypes.NO_DATA, receivedRefreshMsg.ContainerType);
            Assert.Equal(StreamStates.OPEN, receivedRefreshMsg.State.StreamState());
            Assert.Equal(DataStates.OK, receivedRefreshMsg.State.DataState());
            Assert.Equal(consStreamIds[0], receivedRefreshMsg.StreamId);

            /* Reissue first item with new view. */
            requestMsg.Clear();
            requestMsg.StreamId = consStreamIds[0];
            requestMsg.MsgClass = MsgClasses.REQUEST;
            requestMsg.DomainType = (int)DomainType.MARKET_PRICE;
            requestMsg.ContainerType = DataTypes.NO_DATA;
            requestMsg.ApplyStreaming();
            requestMsg.ApplyHasQos();
            requestMsg.Qos.Timeliness(QosTimeliness.REALTIME);
            requestMsg.Qos.Rate(QosRates.TICK_BY_TICK);
            requestMsg.ApplyHasView();
            WatchlistItemDomainsTest.EncodeViewElementNameList(consumer.ReactorChannel, view1_1List, requestMsg);

            submitOptions.Clear();
            submitOptions.ServiceName = Provider.DefaultService.Info.ServiceName.ToString();
            submitOptions.RequestMsgOptions.UserSpecObj = "1111";
            Assert.True(consumer.SubmitAndDispatch((Msg)requestMsg, submitOptions) >= ReactorReturnCode.SUCCESS);

            /* Provider receives request. */
            providerReactor.Dispatch(1);
            evt = providerReactor.PollEvent();
            Assert.Equal(TestReactorEventType.MSG, evt.EventType);
            msgEvent = (ReactorMsgEvent)evt.ReactorEvent;
            Assert.Equal(MsgClasses.REQUEST, msgEvent.Msg.MsgClass);
            receivedRequestMsg = (IRequestMsg)msgEvent.Msg;
            Assert.True(receivedRequestMsg.CheckStreaming());
            Assert.False(receivedRequestMsg.CheckHasPriority());
            Assert.False(receivedRequestMsg.CheckNoRefresh());
            Assert.Equal(providerStreamId, receivedRequestMsg.StreamId);
            Assert.True(WatchlistItemDomainsTest.CheckHasCorrectViewEname(provider, receivedRequestMsg, view1_1List));

            /* Request second item. */
            requestMsg.StreamId = consStreamIds[1];
            WatchlistItemDomainsTest.EncodeViewElementNameList(consumer.ReactorChannel, view2List, requestMsg);

            submitOptions.Clear();
            submitOptions.ServiceName = Provider.DefaultService.Info.ServiceName.ToString();
            submitOptions.RequestMsgOptions.UserSpecObj = "2222";
            Assert.True(consumer.SubmitAndDispatch((Msg)requestMsg, submitOptions) >= ReactorReturnCode.SUCCESS);

            /* Second request shouldn't arrive yet. */
            providerReactor.Dispatch(0, new TimeSpan(0, 0, 0, 0, 500));
            Assert.True(0 == providerReactor.m_EventQueue.Count);

            /* Provider sends refresh, satisfying first view. */
            refreshMsg.Clear();
            refreshMsg.MsgClass = MsgClasses.REFRESH;
            refreshMsg.DomainType = (int)DomainType.MARKET_PRICE;
            refreshMsg.StreamId = providerStreamId;
            refreshMsg.ContainerType = DataTypes.NO_DATA;
            refreshMsg.ApplyRefreshComplete();
            refreshMsg.ApplySolicited();
            refreshMsg.ApplyClearCache();
            refreshMsg.State.StreamState(StreamStates.OPEN);
            refreshMsg.State.DataState(DataStates.OK);
            requestMsg.ApplyHasQos();
            refreshMsg.Qos.Timeliness(QosTimeliness.REALTIME);
            refreshMsg.Qos.Rate(QosRates.TICK_BY_TICK);

            submitOptions.Clear();
            Assert.True(provider.SubmitAndDispatch((Msg)refreshMsg, submitOptions) >= ReactorReturnCode.SUCCESS);

            /* Consumer receives refresh. */
            consumerReactor.Dispatch(1);
            evt = consumerReactor.PollEvent();
            Assert.Equal(TestReactorEventType.MSG, evt.EventType);
            msgEvent = (ReactorMsgEvent)evt.ReactorEvent;
            Assert.Equal(MsgClasses.REFRESH, msgEvent.Msg.MsgClass);
            receivedRefreshMsg = (IRefreshMsg)msgEvent.Msg;
            Assert.Equal((int)DomainType.MARKET_PRICE, receivedRefreshMsg.DomainType);
            Assert.Equal(DataTypes.NO_DATA, receivedRefreshMsg.ContainerType);
            Assert.Equal(StreamStates.OPEN, receivedRefreshMsg.State.StreamState());
            Assert.Equal(DataStates.OK, receivedRefreshMsg.State.DataState());
            Assert.Equal(consStreamIds[0], receivedRefreshMsg.StreamId);

            /* Provider receives request -- same stream, need refresh, updated priority. */
            providerReactor.Dispatch(1);
            evt = providerReactor.PollEvent();
            Assert.Equal(TestReactorEventType.MSG, evt.EventType);
            msgEvent = (ReactorMsgEvent)evt.ReactorEvent;
            Assert.Equal(MsgClasses.REQUEST, msgEvent.Msg.MsgClass);
            receivedRequestMsg = (IRequestMsg)msgEvent.Msg;
            Assert.True(receivedRequestMsg.CheckStreaming());
            Assert.True(receivedRequestMsg.CheckHasPriority());
            Assert.Equal(1, receivedRequestMsg.Priority.PriorityClass);
            Assert.Equal(2, receivedRequestMsg.Priority.Count);
            Assert.False(receivedRequestMsg.CheckNoRefresh());
            Assert.Equal(providerStreamId, receivedRequestMsg.StreamId);
            Assert.True(receivedRequestMsg.CheckHasView());
            Assert.True(WatchlistItemDomainsTest.CheckHasCorrectViewEname(provider, receivedRequestMsg, combinedViewList));

            /* Close first item. */
            ICloseMsg closeMsg = new Msg();
            closeMsg.MsgClass = MsgClasses.CLOSE;
            closeMsg.StreamId = consStreamIds[0];
            closeMsg.DomainType = (int)DomainType.MARKET_PRICE;

            submitOptions.Clear();
            Assert.True(consumer.SubmitAndDispatch((Msg)closeMsg, submitOptions) >= ReactorReturnCode.SUCCESS);

            /* Provider receives priority and view changes as the watchlist hasn't received the previous view. */
            providerReactor.Dispatch(1);
            evt = providerReactor.PollEvent();
            Assert.Equal(TestReactorEventType.MSG, evt.EventType);
            msgEvent = (ReactorMsgEvent)evt.ReactorEvent;
            Assert.Equal(MsgClasses.REQUEST, msgEvent.Msg.MsgClass);
            receivedRequestMsg = (IRequestMsg)msgEvent.Msg;
            Assert.True(receivedRequestMsg.CheckStreaming());
            Assert.True(receivedRequestMsg.CheckHasPriority());
            Assert.Equal(1, receivedRequestMsg.Priority.PriorityClass);
            Assert.Equal(1, receivedRequestMsg.Priority.Count);
            Assert.False(receivedRequestMsg.CheckNoRefresh());
            Assert.True(receivedRequestMsg.CheckHasView());
            Assert.True(WatchlistItemDomainsTest.CheckHasCorrectViewEname(provider, receivedRequestMsg, view2List));
            Assert.Equal(providerStreamId, receivedRequestMsg.StreamId);

            /* Provider sends refresh, satisfying second view. */
            refreshMsg.Clear();
            refreshMsg.MsgClass = MsgClasses.REFRESH;
            refreshMsg.DomainType = (int)DomainType.MARKET_PRICE;
            refreshMsg.StreamId = providerStreamId;
            refreshMsg.ContainerType = DataTypes.NO_DATA;
            refreshMsg.ApplyRefreshComplete();
            refreshMsg.ApplySolicited();
            refreshMsg.ApplyClearCache();
            refreshMsg.State.StreamState(StreamStates.OPEN);
            refreshMsg.State.DataState(DataStates.OK);
            requestMsg.ApplyHasQos();
            refreshMsg.Qos.Timeliness(QosTimeliness.REALTIME);
            refreshMsg.Qos.Rate(QosRates.TICK_BY_TICK);

            submitOptions.Clear();
            Assert.True(provider.SubmitAndDispatch((Msg)refreshMsg, submitOptions) >= ReactorReturnCode.SUCCESS);

            consumerReactor.Dispatch(1);
            evt = consumerReactor.PollEvent();
            Assert.Equal(TestReactorEventType.MSG, evt.EventType);
            msgEvent = (ReactorMsgEvent)evt.ReactorEvent;
            Assert.Equal(MsgClasses.REFRESH, msgEvent.Msg.MsgClass);
            receivedRefreshMsg = (IRefreshMsg)msgEvent.Msg;
            Assert.Equal((int)DomainType.MARKET_PRICE, receivedRefreshMsg.DomainType);
            Assert.Equal(DataTypes.NO_DATA, receivedRefreshMsg.ContainerType);
            Assert.Equal(StreamStates.OPEN, receivedRefreshMsg.State.StreamState());
            Assert.Equal(DataStates.OK, receivedRefreshMsg.State.DataState());
            Assert.Equal(consStreamIds[1], receivedRefreshMsg.StreamId);

            /* Close second item. */
            closeMsg.StreamId = consStreamIds[1]; ;

            submitOptions.Clear();
            Assert.True(consumer.SubmitAndDispatch((Msg)closeMsg, submitOptions) >= ReactorReturnCode.SUCCESS);

            /* Provider receives close. */
            providerReactor.Dispatch(1);
            evt = providerReactor.PollEvent();
            Assert.Equal(TestReactorEventType.MSG, evt.EventType);
            msgEvent = (ReactorMsgEvent)evt.ReactorEvent;
            Assert.Equal(MsgClasses.CLOSE, msgEvent.Msg.MsgClass);
            ICloseMsg receivedCloseMsg = (ICloseMsg)msgEvent.Msg;
            Assert.Equal(providerStreamId, receivedCloseMsg.StreamId);

            TestReactorComponent.CloseSession(consumer, provider);
            consumerReactor.Close();
            providerReactor.Close();
        }

        [Fact]
        public void WatchlistEmptyViewTest()
        {
            ReactorSubmitOptions submitOptions = new ReactorSubmitOptions();
            TestReactorEvent evt;
            ReactorMsgEvent msgEvent;

            TestReactor consumerReactor = new TestReactor();
            TestReactor providerReactor = new TestReactor();

            Consumer consumer = new Consumer(consumerReactor);
            ConsumerRole consumerRole = (ConsumerRole)consumer.ReactorRole;
            consumerRole.InitDefaultRDMLoginRequest();
            consumerRole.InitDefaultRDMDirectoryRequest();
            consumerRole.ChannelEventCallback = consumer;
            consumerRole.LoginMsgCallback = consumer;
            consumerRole.DirectoryMsgCallback = consumer;
            consumerRole.DictionaryMsgCallback = consumer;
            consumerRole.DefaultMsgCallback = consumer;
            consumerRole.WatchlistOptions.EnableWatchlist = true;
            consumerRole.WatchlistOptions.ChannelOpenEventCallback = consumer;

            Provider provider = new Provider(providerReactor);
            ProviderRole providerRole = (ProviderRole)provider.ReactorRole;
            providerRole.ChannelEventCallback = provider;
            providerRole.LoginMsgCallback = provider;
            providerRole.DirectoryMsgCallback = provider;
            providerRole.DictionaryMsgCallback = provider;
            providerRole.DefaultMsgCallback = provider;

            ConsumerProviderSessionOptions opts = new ConsumerProviderSessionOptions();
            opts.SetupDefaultLoginStream = true;
            opts.SetupDefaultDirectoryStream = true;

            provider.Bind(opts);
            TestReactor.OpenSession(consumer, provider, opts);

            List<int> view1List = new List<int> { 2, 6, 7 };

            int[] consStreamIds = { 5, 6 };

            /* Request item. */
            IRequestMsg requestMsg = new Msg();
            requestMsg.Clear();
            requestMsg.MsgClass = MsgClasses.REQUEST;
            requestMsg.StreamId = consStreamIds[0];
            requestMsg.DomainType = (int)DomainType.MARKET_PRICE;
            requestMsg.ApplyStreaming();
            requestMsg.ApplyHasQos();
            requestMsg.Qos.Timeliness(QosTimeliness.REALTIME);
            requestMsg.Qos.Rate(QosRates.TICK_BY_TICK);
            requestMsg.ApplyHasView();
            WatchlistItemDomainsTest.EncodeViewFieldIdList(consumer.ReactorChannel, view1List, requestMsg);

            submitOptions.Clear();
            submitOptions.ServiceName = Provider.DefaultService.Info.ServiceName.ToString();
            Assert.True(consumer.SubmitAndDispatch((Msg)requestMsg, submitOptions) >= ReactorReturnCode.SUCCESS);

            /* Provider receives request. */
            providerReactor.Dispatch(1);
            evt = providerReactor.PollEvent();
            Assert.Equal(TestReactorEventType.MSG, evt.EventType);
            msgEvent = (ReactorMsgEvent)evt.ReactorEvent;
            Assert.Equal(MsgClasses.REQUEST, msgEvent.Msg.MsgClass);
            IRequestMsg receivedRequestMsg = (IRequestMsg)msgEvent.Msg;
            Assert.True(receivedRequestMsg.CheckStreaming());
            Assert.True(receivedRequestMsg.CheckHasPriority());
            Assert.Equal(1, receivedRequestMsg.Priority.PriorityClass);
            Assert.Equal(1, receivedRequestMsg.Priority.Count);
            Assert.False(receivedRequestMsg.CheckNoRefresh());
            int providerStreamId = receivedRequestMsg.StreamId;
            Assert.True(WatchlistItemDomainsTest.CheckHasCorrectView(provider, receivedRequestMsg, view1List));

            /* Provider sends refresh, satisfying first view. */
            IRefreshMsg refreshMsg = new Msg();
            refreshMsg.MsgClass = MsgClasses.REFRESH;
            refreshMsg.DomainType = (int)DomainType.MARKET_PRICE;
            refreshMsg.StreamId = providerStreamId;
            refreshMsg.ContainerType = DataTypes.NO_DATA;
            refreshMsg.ApplyRefreshComplete();
            refreshMsg.ApplySolicited();
            refreshMsg.ApplyClearCache();
            refreshMsg.State.StreamState(StreamStates.OPEN);
            refreshMsg.State.DataState(DataStates.OK);
            refreshMsg.ApplyHasQos();
            refreshMsg.Qos.Timeliness(QosTimeliness.REALTIME);
            refreshMsg.Qos.Rate(QosRates.TICK_BY_TICK);

            submitOptions.Clear();
            Assert.True(provider.SubmitAndDispatch((Msg)refreshMsg, submitOptions) >= ReactorReturnCode.SUCCESS);

            /* Consumer receives refresh. */
            consumerReactor.Dispatch(1);
            evt = consumerReactor.PollEvent();
            Assert.Equal(TestReactorEventType.MSG, evt.EventType);
            msgEvent = (ReactorMsgEvent)evt.ReactorEvent;
            Assert.Equal(MsgClasses.REFRESH, msgEvent.Msg.MsgClass);
            IRefreshMsg receivedRefreshMsg = (IRefreshMsg)msgEvent.Msg;
            Assert.Equal((int)DomainType.MARKET_PRICE, receivedRefreshMsg.DomainType);
            Assert.Equal(DataTypes.NO_DATA, receivedRefreshMsg.ContainerType);
            Assert.Equal(StreamStates.OPEN, receivedRefreshMsg.State.StreamState());
            Assert.Equal(DataStates.OK, receivedRefreshMsg.State.DataState());
            Assert.Equal(consStreamIds[0], receivedRefreshMsg.StreamId);

            /* Request second item with the same view. */
            requestMsg.StreamId = consStreamIds[1];

            submitOptions.Clear();
            submitOptions.ServiceName = Provider.DefaultService.Info.ServiceName.ToString();
            Assert.True(consumer.SubmitAndDispatch((Msg)requestMsg, submitOptions) >= ReactorReturnCode.SUCCESS);

            /* Provider receives request. */
            providerReactor.Dispatch(1);
            evt = providerReactor.PollEvent();
            Assert.Equal(TestReactorEventType.MSG, evt.EventType);
            msgEvent = (ReactorMsgEvent)evt.ReactorEvent;
            Assert.Equal(MsgClasses.REQUEST, msgEvent.Msg.MsgClass);
            receivedRequestMsg = (IRequestMsg)msgEvent.Msg;
            Assert.True(receivedRequestMsg.CheckStreaming());
            Assert.True(receivedRequestMsg.CheckHasPriority());
            Assert.Equal(1, receivedRequestMsg.Priority.PriorityClass);
            Assert.Equal(2, receivedRequestMsg.Priority.Count);
            Assert.False(receivedRequestMsg.CheckNoRefresh());

            /* View flag but no change in view. */
            Assert.True(receivedRequestMsg.CheckHasView());
            Assert.True(WatchlistItemDomainsTest.CheckHasCorrectView(provider, receivedRequestMsg, view1List));
            Assert.Equal(providerStreamId, receivedRequestMsg.StreamId);

            /* Provider sends refresh, satisfying new view. */
            refreshMsg.Clear();
            refreshMsg.MsgClass = MsgClasses.REFRESH;
            refreshMsg.DomainType = (int)DomainType.MARKET_PRICE;
            refreshMsg.StreamId = providerStreamId;
            refreshMsg.ContainerType = DataTypes.NO_DATA;
            refreshMsg.ApplyRefreshComplete();
            refreshMsg.ApplySolicited();
            refreshMsg.ApplyClearCache();
            refreshMsg.State.StreamState(StreamStates.OPEN);
            refreshMsg.State.DataState(DataStates.OK);
            refreshMsg.ApplyHasQos();
            refreshMsg.Qos.Timeliness(QosTimeliness.REALTIME);
            refreshMsg.Qos.Rate(QosRates.TICK_BY_TICK);

            submitOptions.Clear();
            Assert.True(provider.SubmitAndDispatch((Msg)refreshMsg, submitOptions) >= ReactorReturnCode.SUCCESS);

            /* Consumer receives refresh for both user streams. */
            consumerReactor.Dispatch(2);
            evt = consumerReactor.PollEvent();
            Assert.Equal(TestReactorEventType.MSG, evt.EventType);
            msgEvent = (ReactorMsgEvent)evt.ReactorEvent;
            Assert.Equal(MsgClasses.REFRESH, msgEvent.Msg.MsgClass);
            receivedRefreshMsg = (IRefreshMsg)msgEvent.Msg;
            Assert.Equal((int)DomainType.MARKET_PRICE, receivedRefreshMsg.DomainType);
            Assert.Equal(DataTypes.NO_DATA, receivedRefreshMsg.ContainerType);
            Assert.Equal(StreamStates.OPEN, receivedRefreshMsg.State.StreamState());
            Assert.Equal(DataStates.OK, receivedRefreshMsg.State.DataState());
            Assert.Equal(consStreamIds[0], receivedRefreshMsg.StreamId);

            evt = consumerReactor.PollEvent();
            Assert.Equal(TestReactorEventType.MSG, evt.EventType);
            msgEvent = (ReactorMsgEvent)evt.ReactorEvent;
            Assert.Equal(MsgClasses.REFRESH, msgEvent.Msg.MsgClass);
            receivedRefreshMsg = (IRefreshMsg)msgEvent.Msg;
            Assert.Equal((int)DomainType.MARKET_PRICE, receivedRefreshMsg.DomainType);
            Assert.Equal(DataTypes.NO_DATA, receivedRefreshMsg.ContainerType);
            Assert.Equal(StreamStates.OPEN, receivedRefreshMsg.State.StreamState());
            Assert.Equal(DataStates.OK, receivedRefreshMsg.State.DataState());
            Assert.Equal(consStreamIds[1], receivedRefreshMsg.StreamId);

            /* Reissue the second item with empty view. */
            requestMsg.Clear();
            requestMsg.MsgClass = MsgClasses.REQUEST;
            requestMsg.StreamId = consStreamIds[1];
            requestMsg.DomainType = (int)DomainType.MARKET_PRICE;
            requestMsg.ApplyStreaming();
            requestMsg.ApplyHasQos();
            requestMsg.Qos.Timeliness(QosTimeliness.REALTIME);
            requestMsg.Qos.Rate(QosRates.TICK_BY_TICK);
            requestMsg.ApplyHasView();

            List<int> emptyViewList = new();

            WatchlistItemDomainsTest.EncodeViewFieldIdList(consumer.ReactorChannel, emptyViewList, requestMsg);

            submitOptions.Clear();
            submitOptions.ServiceName = Provider.DefaultService.Info.ServiceName.ToString();
            Assert.True(consumer.SubmitAndDispatch((Msg)requestMsg, submitOptions) >= ReactorReturnCode.SUCCESS);

            /* Provider receives request. */
            providerReactor.Dispatch(1);
            evt = providerReactor.PollEvent();
            Assert.Equal(TestReactorEventType.MSG, evt.EventType);
            msgEvent = (ReactorMsgEvent)evt.ReactorEvent;
            Assert.Equal(MsgClasses.REQUEST, msgEvent.Msg.MsgClass);
            receivedRequestMsg = (IRequestMsg)msgEvent.Msg;
            Assert.True(receivedRequestMsg.CheckStreaming());
            Assert.False(receivedRequestMsg.CheckHasPriority());
            Assert.False(receivedRequestMsg.CheckNoRefresh());

            /* View flag and empty view. */
            Assert.True(receivedRequestMsg.CheckHasView());
            Assert.True(WatchlistItemDomainsTest.CheckHasCorrectView(provider, receivedRequestMsg, view1List));
            Assert.Equal(providerStreamId, receivedRequestMsg.StreamId);

            TestReactorComponent.CloseSession(consumer, provider);
            consumerReactor.Close();
            providerReactor.Close();
        }

        [Fact]
        public void WatchlistTwoItems_SnapshotViewTest()
        {
            ReactorSubmitOptions submitOptions = new ReactorSubmitOptions();
            TestReactorEvent evt;
            ReactorMsgEvent msgEvent;

            TestReactor consumerReactor = new TestReactor();
            TestReactor providerReactor = new TestReactor();

            Consumer consumer = new Consumer(consumerReactor);
            ConsumerRole consumerRole = (ConsumerRole)consumer.ReactorRole;
            consumerRole.InitDefaultRDMLoginRequest();
            consumerRole.InitDefaultRDMDirectoryRequest();
            consumerRole.ChannelEventCallback = consumer;
            consumerRole.LoginMsgCallback = consumer;
            consumerRole.DirectoryMsgCallback = consumer;
            consumerRole.DictionaryMsgCallback = consumer;
            consumerRole.DefaultMsgCallback = consumer;
            consumerRole.WatchlistOptions.EnableWatchlist = true;
            consumerRole.WatchlistOptions.ChannelOpenEventCallback = consumer;

            Provider provider = new Provider(providerReactor);
            ProviderRole providerRole = (ProviderRole)provider.ReactorRole;
            providerRole.ChannelEventCallback = provider;
            providerRole.LoginMsgCallback = provider;
            providerRole.DirectoryMsgCallback = provider;
            providerRole.DictionaryMsgCallback = provider;
            providerRole.DefaultMsgCallback = provider;

            ConsumerProviderSessionOptions opts = new ConsumerProviderSessionOptions();
            opts.SetupDefaultLoginStream = true;
            opts.SetupDefaultDirectoryStream = true;

            provider.Bind(opts);
            TestReactor.OpenSession(consumer, provider, opts);

            List<int> view1List = new List<int> { 2, 6, 7 };
            List<int> view2List = new List<int> { 3, 22 };

            int[] consStreamIds = { 5, 6 };

            /* Request first snapshot item. */
            IRequestMsg requestMsg = new Msg();
            requestMsg.StreamId = consStreamIds[0];
            requestMsg.MsgClass = MsgClasses.REQUEST;
            requestMsg.DomainType = (int)DomainType.MARKET_PRICE;
            requestMsg.ContainerType = DataTypes.NO_DATA;
            requestMsg.ApplyHasQos();
            requestMsg.Qos.Timeliness(QosTimeliness.REALTIME);
            requestMsg.Qos.Rate(QosRates.TICK_BY_TICK);
            requestMsg.ApplyHasView();
            WatchlistItemDomainsTest.EncodeViewFieldIdList(consumer.ReactorChannel, view1List, requestMsg);

            submitOptions.Clear();
            submitOptions.ServiceName = Provider.DefaultService.Info.ServiceName.ToString();
            Assert.True(consumer.SubmitAndDispatch((Msg)requestMsg, submitOptions) >= ReactorReturnCode.SUCCESS);

            /* Provider receives first snapshot request. */
            providerReactor.Dispatch(1);
            evt = providerReactor.PollEvent();
            Assert.Equal(TestReactorEventType.MSG, evt.EventType);
            msgEvent = (ReactorMsgEvent)evt.ReactorEvent;
            Assert.Equal(MsgClasses.REQUEST, msgEvent.Msg.MsgClass);
            IRequestMsg receivedRequestMsg = (IRequestMsg)msgEvent.Msg;
            Assert.False(receivedRequestMsg.CheckStreaming());
            Assert.True(receivedRequestMsg.CheckHasPriority());
            Assert.Equal(1, receivedRequestMsg.Priority.PriorityClass);
            Assert.Equal(1, receivedRequestMsg.Priority.Count);
            Assert.False(receivedRequestMsg.CheckNoRefresh());
            Assert.True(WatchlistItemDomainsTest.CheckHasCorrectView(provider, requestMsg, view1List));
            int providerStreamId = receivedRequestMsg.StreamId;

            /* consumer sends second snapshot request with diffrent view. */
            requestMsg.Clear();
            requestMsg.StreamId = consStreamIds[1];
            requestMsg.MsgClass = MsgClasses.REQUEST;
            requestMsg.DomainType = (int)DomainType.MARKET_PRICE;
            requestMsg.ContainerType = DataTypes.NO_DATA;
            requestMsg.ApplyHasQos();
            requestMsg.Qos.Timeliness(QosTimeliness.REALTIME);
            requestMsg.Qos.Rate(QosRates.TICK_BY_TICK);
            requestMsg.ApplyHasView();
            WatchlistItemDomainsTest.EncodeViewFieldIdList(consumer.ReactorChannel, view2List, requestMsg);

            submitOptions.Clear();
            submitOptions.ServiceName = Provider.DefaultService.Info.ServiceName.ToString();
            Assert.True(consumer.SubmitAndDispatch((Msg)requestMsg, submitOptions) >= ReactorReturnCode.SUCCESS);

            /* Provider receives nothing yet. */
            providerReactor.Dispatch(0, new TimeSpan(0, 0, 0, 0, 500));
            Assert.True(providerReactor.m_EventQueue.Count == 0);

            /* Provider sends refresh, satisfying first view. */
            IRefreshMsg refreshMsg = new Msg();
            refreshMsg.MsgClass = MsgClasses.REFRESH;
            refreshMsg.DomainType = (int)DomainType.MARKET_PRICE;
            refreshMsg.StreamId = providerStreamId;
            refreshMsg.ContainerType = DataTypes.NO_DATA;
            refreshMsg.ApplyRefreshComplete();
            refreshMsg.ApplySolicited();
            refreshMsg.ApplyClearCache();
            refreshMsg.State.StreamState(StreamStates.NON_STREAMING);
            refreshMsg.State.DataState(DataStates.OK);
            refreshMsg.ApplyHasQos();
            refreshMsg.Qos.Timeliness(QosTimeliness.REALTIME);
            refreshMsg.Qos.Rate(QosRates.TICK_BY_TICK);

            submitOptions.Clear();
            Assert.True(provider.SubmitAndDispatch((Msg)refreshMsg, submitOptions) >= ReactorReturnCode.SUCCESS);

            /* Consumer receives refresh for the first item. */
            consumerReactor.Dispatch(1);
            evt = consumerReactor.PollEvent();
            Assert.Equal(TestReactorEventType.MSG, evt.EventType);
            msgEvent = (ReactorMsgEvent)evt.ReactorEvent;
            Assert.Equal(MsgClasses.REFRESH, msgEvent.Msg.MsgClass);
            IRefreshMsg receivedRefreshMsg = (IRefreshMsg)msgEvent.Msg;
            Assert.Equal((int)DomainType.MARKET_PRICE, receivedRefreshMsg.DomainType);
            Assert.Equal(DataTypes.NO_DATA, receivedRefreshMsg.ContainerType);
            Assert.Equal(StreamStates.NON_STREAMING, receivedRefreshMsg.State.StreamState());
            Assert.Equal(DataStates.OK, receivedRefreshMsg.State.DataState());
            Assert.Equal(consStreamIds[0], receivedRefreshMsg.StreamId);

            /* Provider receives second snapshot request. */
            providerReactor.Dispatch(1);
            evt = providerReactor.PollEvent();
            Assert.Equal(TestReactorEventType.MSG, evt.EventType);
            msgEvent = (ReactorMsgEvent)evt.ReactorEvent;
            Assert.Equal(MsgClasses.REQUEST, msgEvent.Msg.MsgClass);
            receivedRequestMsg = (IRequestMsg)msgEvent.Msg;
            Assert.False(receivedRequestMsg.CheckStreaming());
            Assert.True(receivedRequestMsg.CheckHasPriority());
            Assert.Equal(1, receivedRequestMsg.Priority.PriorityClass);
            Assert.Equal(1, receivedRequestMsg.Priority.Count);
            Assert.False(receivedRequestMsg.CheckNoRefresh());
            Assert.True(WatchlistItemDomainsTest.CheckHasCorrectView(provider, requestMsg, view2List));
            int providerStreamId2 = receivedRequestMsg.StreamId;

            TestReactorComponent.CloseSession(consumer, provider);
            consumerReactor.Close();
            providerReactor.Close();
        }

        [Fact]
        public void WatchlistSnapshotBeforeStreaming_ViewTest()
        {
            ReactorSubmitOptions submitOptions = new ReactorSubmitOptions();
            TestReactorEvent evt;
            ReactorMsgEvent msgEvent;

            TestReactor consumerReactor = new TestReactor();
            TestReactor providerReactor = new TestReactor();

            Consumer consumer = new Consumer(consumerReactor);
            ConsumerRole consumerRole = (ConsumerRole)consumer.ReactorRole;
            consumerRole.InitDefaultRDMLoginRequest();
            consumerRole.InitDefaultRDMDirectoryRequest();
            consumerRole.ChannelEventCallback = consumer;
            consumerRole.LoginMsgCallback = consumer;
            consumerRole.DirectoryMsgCallback = consumer;
            consumerRole.DictionaryMsgCallback = consumer;
            consumerRole.DefaultMsgCallback = consumer;
            consumerRole.WatchlistOptions.EnableWatchlist = true;
            consumerRole.WatchlistOptions.ChannelOpenEventCallback = consumer;

            Provider provider = new Provider(providerReactor);
            ProviderRole providerRole = (ProviderRole)provider.ReactorRole;
            providerRole.ChannelEventCallback = provider;
            providerRole.LoginMsgCallback = provider;
            providerRole.DirectoryMsgCallback = provider;
            providerRole.DictionaryMsgCallback = provider;
            providerRole.DefaultMsgCallback = provider;

            ConsumerProviderSessionOptions opts = new ConsumerProviderSessionOptions();
            opts.SetupDefaultLoginStream = true;
            opts.SetupDefaultDirectoryStream = true;

            provider.Bind(opts);
            TestReactor.OpenSession(consumer, provider, opts);

            List<int> view1List = new List<int> { 2, 6, 7 };
            List<int> view2List = new List<int> { 6, 7 };

            int[] consStreamIds = { 5, 6 };

            /* Request first item as a snapshot. */
            IRequestMsg requestMsg = new Msg();
            requestMsg.StreamId = consStreamIds[0];
            requestMsg.MsgClass = MsgClasses.REQUEST;
            requestMsg.DomainType = (int)DomainType.MARKET_PRICE;
            requestMsg.ContainerType = DataTypes.NO_DATA;
            requestMsg.ApplyHasQos();
            requestMsg.Qos.Timeliness(QosTimeliness.REALTIME);
            requestMsg.Qos.Rate(QosRates.TICK_BY_TICK);
            requestMsg.ApplyHasView();
            WatchlistItemDomainsTest.EncodeViewFieldIdList(consumer.ReactorChannel, view1List, requestMsg);

            submitOptions.Clear();
            submitOptions.ServiceName = Provider.DefaultService.Info.ServiceName.ToString();
            submitOptions.RequestMsgOptions.UserSpecObj = "1111";
            Assert.True(consumer.SubmitAndDispatch((Msg)requestMsg, submitOptions) >= ReactorReturnCode.SUCCESS);

            /* Provider receives request. */
            providerReactor.Dispatch(1);
            evt = providerReactor.PollEvent();
            Assert.Equal(TestReactorEventType.MSG, evt.EventType);
            msgEvent = (ReactorMsgEvent)evt.ReactorEvent;
            Assert.Equal(MsgClasses.REQUEST, msgEvent.Msg.MsgClass);
            IRequestMsg receivedRequestMsg = (IRequestMsg)msgEvent.Msg;
            Assert.False(receivedRequestMsg.CheckStreaming());
            Assert.True(receivedRequestMsg.CheckHasPriority());
            Assert.Equal(1, receivedRequestMsg.Priority.PriorityClass);
            Assert.Equal(1, receivedRequestMsg.Priority.Count);
            Assert.False(receivedRequestMsg.CheckNoRefresh());
            int providerStreamId = receivedRequestMsg.StreamId;
            Assert.True(WatchlistItemDomainsTest.CheckHasCorrectView(provider, receivedRequestMsg, view1List));

            /* Request second item as streaming. */
            requestMsg.StreamId = consStreamIds[1];
            requestMsg.ApplyStreaming();
            WatchlistItemDomainsTest.EncodeViewFieldIdList(consumer.ReactorChannel, view2List, requestMsg);

            submitOptions.Clear();
            submitOptions.ServiceName = Provider.DefaultService.Info.ServiceName.ToString();
            Assert.True(consumer.SubmitAndDispatch((Msg)requestMsg, submitOptions) >= ReactorReturnCode.SUCCESS);

            /* Provider receives nothing. */
            providerReactor.Dispatch(0, new TimeSpan(0, 0, 0, 0, 500));
            Assert.True(providerReactor.m_EventQueue.Count == 0);

            /* Provider sends refresh. */
            IRefreshMsg refreshMsg = new Msg();
            refreshMsg.MsgClass = MsgClasses.REFRESH;
            refreshMsg.DomainType = (int)DomainType.MARKET_PRICE;
            refreshMsg.StreamId = providerStreamId;
            refreshMsg.ContainerType = DataTypes.NO_DATA;
            refreshMsg.ApplyRefreshComplete();
            refreshMsg.ApplySolicited();
            refreshMsg.ApplyClearCache();
            refreshMsg.State.StreamState(StreamStates.OPEN);
            refreshMsg.State.DataState(DataStates.OK);
            refreshMsg.ApplyHasQos();
            refreshMsg.Qos.Timeliness(QosTimeliness.REALTIME);
            refreshMsg.Qos.Rate(QosRates.TICK_BY_TICK);

            submitOptions.Clear();
            Assert.True(provider.SubmitAndDispatch((Msg)refreshMsg, submitOptions) >= ReactorReturnCode.SUCCESS);

            providerReactor.Dispatch(0, new TimeSpan(0, 0, 0, 0, 500));
            Assert.True(providerReactor.m_EventQueue.Count == 0);

            /* Consumer receives refresh. */
            consumerReactor.Dispatch(1);
            evt = consumerReactor.PollEvent();
            Assert.Equal(TestReactorEventType.MSG, evt.EventType);
            msgEvent = (ReactorMsgEvent)evt.ReactorEvent;
            Assert.Equal(MsgClasses.REFRESH, msgEvent.Msg.MsgClass);
            IRefreshMsg receivedRefreshMsg = (IRefreshMsg)msgEvent.Msg;
            Assert.Equal((int)DomainType.MARKET_PRICE, receivedRefreshMsg.DomainType);
            Assert.Equal(DataTypes.NO_DATA, receivedRefreshMsg.ContainerType);
            Assert.Equal(StreamStates.NON_STREAMING, receivedRefreshMsg.State.StreamState());
            Assert.Equal(DataStates.OK, receivedRefreshMsg.State.DataState());
            Assert.Equal(consStreamIds[0], receivedRefreshMsg.StreamId);
            Assert.NotNull(msgEvent.StreamInfo.UserSpec);
            Assert.Equal("1111", msgEvent.StreamInfo.UserSpec);

            /* Provider receives streaming request. */
            providerReactor.Dispatch(1);
            evt = providerReactor.PollEvent();
            Assert.Equal(TestReactorEventType.MSG, evt.EventType);
            msgEvent = (ReactorMsgEvent)evt.ReactorEvent;
            Assert.Equal(MsgClasses.REQUEST, msgEvent.Msg.MsgClass);
            receivedRequestMsg = (IRequestMsg)msgEvent.Msg;
            Assert.True(receivedRequestMsg.CheckStreaming());
            Assert.True(receivedRequestMsg.CheckHasPriority());
            Assert.Equal(1, receivedRequestMsg.Priority.PriorityClass);
            Assert.Equal(1, receivedRequestMsg.Priority.Count);
            Assert.False(receivedRequestMsg.CheckNoRefresh());
            Assert.Equal(providerStreamId, receivedRequestMsg.StreamId);
            Assert.True(WatchlistItemDomainsTest.CheckHasCorrectView(provider, receivedRequestMsg, view2List));

            TestReactorComponent.CloseSession(consumer, provider);
            consumerReactor.Close();
            providerReactor.Close();
        }

        [Fact]
        public void WatchlistTwoItems_ViewOnOffTest()
        {
            ReactorSubmitOptions submitOptions = new ReactorSubmitOptions();
            TestReactorEvent evt;
            ReactorMsgEvent msgEvent;

            TestReactor consumerReactor = new TestReactor();
            TestReactor providerReactor = new TestReactor();

            Consumer consumer = new Consumer(consumerReactor);
            ConsumerRole consumerRole = (ConsumerRole)consumer.ReactorRole;
            consumerRole.InitDefaultRDMLoginRequest();
            consumerRole.InitDefaultRDMDirectoryRequest();
            consumerRole.ChannelEventCallback = consumer;
            consumerRole.LoginMsgCallback = consumer;
            consumerRole.DirectoryMsgCallback = consumer;
            consumerRole.DictionaryMsgCallback = consumer;
            consumerRole.DefaultMsgCallback = consumer;
            consumerRole.WatchlistOptions.EnableWatchlist = true;
            consumerRole.WatchlistOptions.ChannelOpenEventCallback = consumer;

            Provider provider = new Provider(providerReactor);
            ProviderRole providerRole = (ProviderRole)provider.ReactorRole;
            providerRole.ChannelEventCallback = provider;
            providerRole.LoginMsgCallback = provider;
            providerRole.DirectoryMsgCallback = provider;
            providerRole.DictionaryMsgCallback = provider;
            providerRole.DefaultMsgCallback = provider;

            ConsumerProviderSessionOptions opts = new ConsumerProviderSessionOptions();
            opts.SetupDefaultLoginStream = true;
            opts.SetupDefaultDirectoryStream = true;

            provider.Bind(opts);
            TestReactor.OpenSession(consumer, provider, opts);

            List<int> view1List = new List<int> { 2, 6, 7 };
            List<int> view2List = new List<int> { 6, 7 };

            int[] consStreamIds = { 5, 6 };

            /* Test how aggregation of items can remove/restore
             * a view. */

            /* Request first item. */
            IRequestMsg requestMsg = new Msg();
            requestMsg.StreamId = consStreamIds[0];
            requestMsg.MsgClass = MsgClasses.REQUEST;
            requestMsg.DomainType = (int)DomainType.MARKET_PRICE;
            requestMsg.ContainerType = DataTypes.NO_DATA;
            requestMsg.ApplyStreaming();
            requestMsg.ApplyHasQos();
            requestMsg.Qos.Timeliness(QosTimeliness.REALTIME);
            requestMsg.Qos.Rate(QosRates.TICK_BY_TICK);
            requestMsg.ApplyHasView();
            WatchlistItemDomainsTest.EncodeViewFieldIdList(consumer.ReactorChannel, view1List, requestMsg);

            submitOptions.Clear();
            submitOptions.ServiceName = Provider.DefaultService.Info.ServiceName.ToString();
            Assert.True(consumer.SubmitAndDispatch((Msg)requestMsg, submitOptions) >= ReactorReturnCode.SUCCESS);

            /* Provider receives request. */
            providerReactor.Dispatch(1);
            evt = providerReactor.PollEvent();
            Assert.Equal(TestReactorEventType.MSG, evt.EventType);
            msgEvent = (ReactorMsgEvent)evt.ReactorEvent;
            Assert.Equal(MsgClasses.REQUEST, msgEvent.Msg.MsgClass);
            IRequestMsg receivedRequestMsg = (IRequestMsg)msgEvent.Msg;
            Assert.True(receivedRequestMsg.CheckStreaming());
            Assert.True(receivedRequestMsg.CheckHasPriority());
            Assert.Equal(1, receivedRequestMsg.Priority.PriorityClass);
            Assert.Equal(1, receivedRequestMsg.Priority.Count);
            Assert.False(receivedRequestMsg.CheckNoRefresh());
            Assert.True(receivedRequestMsg.CheckHasView());
            int providerStreamId = receivedRequestMsg.StreamId;
            Assert.True(WatchlistItemDomainsTest.CheckHasCorrectView(provider, receivedRequestMsg, view1List));

            /* Provider sends refresh, satisfying first view. */
            IRefreshMsg refreshMsg = new Msg();
            refreshMsg.MsgClass = MsgClasses.REFRESH;
            refreshMsg.DomainType = (int)DomainType.MARKET_PRICE;
            refreshMsg.StreamId = providerStreamId;
            refreshMsg.ContainerType = DataTypes.NO_DATA;
            refreshMsg.ApplyRefreshComplete();
            refreshMsg.ApplySolicited();
            refreshMsg.ApplyClearCache();
            refreshMsg.State.StreamState(StreamStates.OPEN);
            refreshMsg.State.DataState(DataStates.OK);
            refreshMsg.ApplyHasQos();
            refreshMsg.Qos.Timeliness(QosTimeliness.REALTIME);
            refreshMsg.Qos.Rate(QosRates.TICK_BY_TICK);

            submitOptions.Clear();
            Assert.True(provider.SubmitAndDispatch((Msg)refreshMsg, submitOptions) >= ReactorReturnCode.SUCCESS);

            /* Consumer receives refresh. */
            consumerReactor.Dispatch(1);
            evt = consumerReactor.PollEvent();
            Assert.Equal(TestReactorEventType.MSG, evt.EventType);
            msgEvent = (ReactorMsgEvent)evt.ReactorEvent;
            Assert.Equal(MsgClasses.REFRESH, msgEvent.Msg.MsgClass);
            IRefreshMsg receivedRefreshMsg = (IRefreshMsg)msgEvent.Msg;
            Assert.Equal((int)DomainType.MARKET_PRICE, receivedRefreshMsg.DomainType);
            Assert.Equal(DataTypes.NO_DATA, receivedRefreshMsg.ContainerType);
            Assert.Equal(StreamStates.OPEN, receivedRefreshMsg.State.StreamState());
            Assert.Equal(DataStates.OK, receivedRefreshMsg.State.DataState());
            Assert.Equal(consStreamIds[0], receivedRefreshMsg.StreamId);

            /* Request second item without view. */
            requestMsg.Clear();
            requestMsg.StreamId = consStreamIds[1];
            requestMsg.MsgClass = MsgClasses.REQUEST;
            requestMsg.DomainType = (int)DomainType.MARKET_PRICE;
            requestMsg.ContainerType = DataTypes.NO_DATA;
            requestMsg.ApplyStreaming();
            requestMsg.ApplyHasQos();
            requestMsg.Qos.Timeliness(QosTimeliness.REALTIME);
            requestMsg.Qos.Rate(QosRates.TICK_BY_TICK);

            submitOptions.Clear();
            submitOptions.ServiceName = Provider.DefaultService.Info.ServiceName.ToString();
            Assert.True(consumer.SubmitAndDispatch((Msg)requestMsg, submitOptions) >= ReactorReturnCode.SUCCESS);

            /* Provider receives request. View should be removed. */
            providerReactor.Dispatch(1);
            evt = providerReactor.PollEvent();
            Assert.Equal(TestReactorEventType.MSG, evt.EventType);
            msgEvent = (ReactorMsgEvent)evt.ReactorEvent;
            Assert.Equal(MsgClasses.REQUEST, msgEvent.Msg.MsgClass);
            receivedRequestMsg = (IRequestMsg)msgEvent.Msg;
            Assert.True(receivedRequestMsg.CheckStreaming());
            Assert.True(receivedRequestMsg.CheckHasPriority());
            Assert.Equal(1, receivedRequestMsg.Priority.PriorityClass);
            Assert.Equal(2, receivedRequestMsg.Priority.Count);
            Assert.False(receivedRequestMsg.CheckNoRefresh());
            Assert.False(receivedRequestMsg.CheckHasView());
            Assert.Equal(providerStreamId, receivedRequestMsg.StreamId);

            /* Provider sends refresh, satisfying view. */
            refreshMsg.Clear();
            refreshMsg.MsgClass = MsgClasses.REFRESH;
            refreshMsg.DomainType = (int)DomainType.MARKET_PRICE;
            refreshMsg.StreamId = providerStreamId;
            refreshMsg.ContainerType = DataTypes.NO_DATA;
            refreshMsg.ApplyRefreshComplete();
            refreshMsg.ApplySolicited();
            refreshMsg.ApplyClearCache();
            refreshMsg.State.StreamState(StreamStates.OPEN);
            refreshMsg.State.DataState(DataStates.OK);
            refreshMsg.ApplyHasQos();
            refreshMsg.Qos.Timeliness(QosTimeliness.REALTIME);
            refreshMsg.Qos.Rate(QosRates.TICK_BY_TICK);

            submitOptions.Clear();
            Assert.True(provider.SubmitAndDispatch((Msg)refreshMsg, submitOptions) >= ReactorReturnCode.SUCCESS);

            /* Consumer receives refresh on both streams (unsolicited on one). */
            consumerReactor.Dispatch(2);
            evt = consumerReactor.PollEvent();
            Assert.Equal(TestReactorEventType.MSG, evt.EventType);
            msgEvent = (ReactorMsgEvent)evt.ReactorEvent;
            Assert.Equal(MsgClasses.REFRESH, msgEvent.Msg.MsgClass);
            receivedRefreshMsg = (IRefreshMsg)msgEvent.Msg;
            Assert.Equal((int)DomainType.MARKET_PRICE, receivedRefreshMsg.DomainType);
            Assert.Equal(DataTypes.NO_DATA, receivedRefreshMsg.ContainerType);
            Assert.Equal(StreamStates.OPEN, receivedRefreshMsg.State.StreamState());
            Assert.Equal(DataStates.OK, receivedRefreshMsg.State.DataState());
            Assert.False(receivedRefreshMsg.CheckSolicited());
            Assert.Equal(consStreamIds[0], receivedRefreshMsg.StreamId);

            evt = consumerReactor.PollEvent();
            Assert.Equal(TestReactorEventType.MSG, evt.EventType);
            msgEvent = (ReactorMsgEvent)evt.ReactorEvent;
            Assert.Equal(MsgClasses.REFRESH, msgEvent.Msg.MsgClass);
            receivedRefreshMsg = (IRefreshMsg)msgEvent.Msg;
            Assert.Equal((int)DomainType.MARKET_PRICE, receivedRefreshMsg.DomainType);
            Assert.Equal(DataTypes.NO_DATA, receivedRefreshMsg.ContainerType);
            Assert.Equal(StreamStates.OPEN, receivedRefreshMsg.State.StreamState());
            Assert.Equal(DataStates.OK, receivedRefreshMsg.State.DataState());
            Assert.True(receivedRefreshMsg.CheckSolicited());
            Assert.Equal(consStreamIds[1], receivedRefreshMsg.StreamId);

            TestReactorComponent.CloseSession(consumer, provider);
            consumerReactor.Close();
            providerReactor.Close();
        }

        [Fact]
        public void WatchlistTwoItems_ViewMixtureTest()
        {
            ReactorSubmitOptions submitOptions = new ReactorSubmitOptions();
            TestReactorEvent evt;
            ReactorMsgEvent msgEvent;

            TestReactor consumerReactor = new TestReactor();
            TestReactor providerReactor = new TestReactor();

            Consumer consumer = new Consumer(consumerReactor);
            ConsumerRole consumerRole = (ConsumerRole)consumer.ReactorRole;
            consumerRole.InitDefaultRDMLoginRequest();
            consumerRole.InitDefaultRDMDirectoryRequest();
            consumerRole.ChannelEventCallback = consumer;
            consumerRole.LoginMsgCallback = consumer;
            consumerRole.DirectoryMsgCallback = consumer;
            consumerRole.DictionaryMsgCallback = consumer;
            consumerRole.DefaultMsgCallback = consumer;
            consumerRole.WatchlistOptions.EnableWatchlist = true;
            consumerRole.WatchlistOptions.ChannelOpenEventCallback = consumer;

            Provider provider = new Provider(providerReactor);
            ProviderRole providerRole = (ProviderRole)provider.ReactorRole;
            providerRole.ChannelEventCallback = provider;
            providerRole.LoginMsgCallback = provider;
            providerRole.DirectoryMsgCallback = provider;
            providerRole.DictionaryMsgCallback = provider;
            providerRole.DefaultMsgCallback = provider;

            ConsumerProviderSessionOptions opts = new ConsumerProviderSessionOptions();
            opts.SetupDefaultLoginStream = true;
            opts.SetupDefaultDirectoryStream = true;

            provider.Bind(opts);
            TestReactor.OpenSession(consumer, provider, opts);

            List<int> view1List = new List<int> { 2, 3 };

            List<string> enameView1List = new List<string> { "FID2", "FID3" };

            int[] consStreamIds = { 5, 6 };

            /* Request first item. */
            IRequestMsg requestMsg = new Msg();
            requestMsg.StreamId = consStreamIds[0];
            requestMsg.MsgClass = MsgClasses.REQUEST;
            requestMsg.DomainType = (int)DomainType.MARKET_PRICE;
            requestMsg.ApplyStreaming();
            requestMsg.ApplyHasQos();
            requestMsg.Qos.Timeliness(QosTimeliness.REALTIME);
            requestMsg.Qos.Rate(QosRates.TICK_BY_TICK);
            requestMsg.ApplyHasView();
            WatchlistItemDomainsTest.EncodeViewFieldIdList(consumer.ReactorChannel, view1List, requestMsg);

            submitOptions.Clear();
            submitOptions.ServiceName = Provider.DefaultService.Info.ServiceName.ToString();
            Assert.True(consumer.SubmitAndDispatch((Msg)requestMsg, submitOptions) >= ReactorReturnCode.SUCCESS);

            /* Provider receives request with field view. */
            providerReactor.Dispatch(1);
            evt = providerReactor.PollEvent();
            Assert.Equal(TestReactorEventType.MSG, evt.EventType);
            msgEvent = (ReactorMsgEvent)evt.ReactorEvent;
            Assert.Equal(MsgClasses.REQUEST, msgEvent.Msg.MsgClass);
            IRequestMsg receivedRequestMsg = (IRequestMsg)msgEvent.Msg;
            Assert.True(receivedRequestMsg.CheckStreaming());
            Assert.True(receivedRequestMsg.CheckHasPriority());
            Assert.Equal(1, receivedRequestMsg.Priority.PriorityClass);
            Assert.Equal(1, receivedRequestMsg.Priority.Count);
            Assert.False(receivedRequestMsg.CheckNoRefresh());
            Assert.True(WatchlistItemDomainsTest.CheckHasCorrectView(provider, receivedRequestMsg, view1List));
            int providerStreamId = receivedRequestMsg.StreamId;

            /* Request second item with element name view. */
            requestMsg.StreamId = consStreamIds[1];
            WatchlistItemDomainsTest.EncodeViewElementNameList(consumer.ReactorChannel, enameView1List, requestMsg);

            submitOptions.Clear();
            submitOptions.ServiceName = Provider.DefaultService.Info.ServiceName.ToString();
            Assert.True(consumer.Submit((Msg)requestMsg, submitOptions) >= ReactorReturnCode.SUCCESS);

            /* Second request is rejected for trying to mix views. */
            consumerReactor.Dispatch(1);
            evt = consumerReactor.PollEvent();
            Assert.Equal(TestReactorEventType.MSG, evt.EventType);
            msgEvent = (ReactorMsgEvent)evt.ReactorEvent;
            Assert.Equal(MsgClasses.STATUS, msgEvent.Msg.MsgClass);
            IStatusMsg receivedStatusMsg = (IStatusMsg)msgEvent.Msg;
            Assert.Equal((int)DomainType.MARKET_PRICE, receivedStatusMsg.DomainType);
            Assert.Equal(DataTypes.NO_DATA, receivedStatusMsg.ContainerType);
            Assert.True(receivedStatusMsg.CheckHasState());
            Assert.Equal(StreamStates.CLOSED, receivedStatusMsg.State.StreamState());
            Assert.Equal(DataStates.SUSPECT, receivedStatusMsg.State.DataState());
            Assert.Equal(StateCodes.USAGE_ERROR, receivedStatusMsg.State.Code());
            Assert.Equal(consStreamIds[1], receivedStatusMsg.StreamId);

            /* Provider sends refresh, satisfying first view. */
            IRefreshMsg refreshMsg = new Msg();
            refreshMsg.MsgClass = MsgClasses.REFRESH;
            refreshMsg.DomainType = (int)DomainType.MARKET_PRICE;
            refreshMsg.StreamId = providerStreamId;
            refreshMsg.ContainerType = DataTypes.NO_DATA;
            refreshMsg.ApplyRefreshComplete();
            refreshMsg.ApplySolicited();
            refreshMsg.ApplyClearCache();
            refreshMsg.State.StreamState(StreamStates.OPEN);
            refreshMsg.State.DataState(DataStates.OK);
            refreshMsg.ApplyHasQos();
            refreshMsg.Qos.Timeliness(QosTimeliness.REALTIME);
            refreshMsg.Qos.Rate(QosRates.TICK_BY_TICK);

            submitOptions.Clear();
            Assert.True(provider.SubmitAndDispatch((Msg)refreshMsg, submitOptions) >= ReactorReturnCode.SUCCESS);

            /* Consumer receives refresh for the first view. */
            consumerReactor.Dispatch(1);
            evt = consumerReactor.PollEvent();
            Assert.Equal(TestReactorEventType.MSG, evt.EventType);
            msgEvent = (ReactorMsgEvent)evt.ReactorEvent;
            Assert.Equal(MsgClasses.REFRESH, msgEvent.Msg.MsgClass);
            IRefreshMsg receivedRefreshMsg = (IRefreshMsg)msgEvent.Msg;
            Assert.Equal((int)DomainType.MARKET_PRICE, receivedRefreshMsg.DomainType);
            Assert.Equal(DataTypes.NO_DATA, receivedRefreshMsg.ContainerType);
            Assert.Equal(StreamStates.OPEN, receivedRefreshMsg.State.StreamState());
            Assert.Equal(DataStates.OK, receivedRefreshMsg.State.DataState());
            Assert.Equal(consStreamIds[0], receivedRefreshMsg.StreamId);

            /* Close first item. */
            ICloseMsg closeMsg = new Msg();
            closeMsg.MsgClass = MsgClasses.CLOSE;
            closeMsg.StreamId = consStreamIds[0];

            submitOptions.Clear();
            Assert.True(consumer.SubmitAndDispatch((Msg)closeMsg, submitOptions) >= ReactorReturnCode.SUCCESS);

            /* Provider receives close. */
            providerReactor.Dispatch(1);
            evt = providerReactor.PollEvent();
            Assert.Equal(TestReactorEventType.MSG, evt.EventType);
            msgEvent = (ReactorMsgEvent)evt.ReactorEvent;
            Assert.Equal(MsgClasses.CLOSE, msgEvent.Msg.MsgClass);
            ICloseMsg receivedCloseMsg = (ICloseMsg)msgEvent.Msg;
            Assert.Equal(providerStreamId, receivedCloseMsg.StreamId);

            TestReactorComponent.CloseSession(consumer, provider);
            consumerReactor.Close();
            providerReactor.Close();
        }

        [Fact]
        public void WatchlistTwoItemsInMsgBuffer_ViewMixtureTest()
        {
            ReactorSubmitOptions submitOptions = new ReactorSubmitOptions();
            TestReactorEvent evt;
            ReactorMsgEvent msgEvent;

            TestReactor consumerReactor = new TestReactor();
            TestReactor providerReactor = new TestReactor();

            Consumer consumer = new Consumer(consumerReactor);
            ConsumerRole consumerRole = (ConsumerRole)consumer.ReactorRole;
            consumerRole.InitDefaultRDMLoginRequest();
            consumerRole.InitDefaultRDMDirectoryRequest();
            consumerRole.ChannelEventCallback = consumer;
            consumerRole.LoginMsgCallback = consumer;
            consumerRole.DirectoryMsgCallback = consumer;
            consumerRole.DictionaryMsgCallback = consumer;
            consumerRole.DefaultMsgCallback = consumer;
            consumerRole.WatchlistOptions.EnableWatchlist = true;
            consumerRole.WatchlistOptions.ChannelOpenEventCallback = consumer;

            Provider provider = new Provider(providerReactor);
            ProviderRole providerRole = (ProviderRole)provider.ReactorRole;
            providerRole.ChannelEventCallback = provider;
            providerRole.LoginMsgCallback = provider;
            providerRole.DirectoryMsgCallback = provider;
            providerRole.DictionaryMsgCallback = provider;
            providerRole.DefaultMsgCallback = provider;

            ConsumerProviderSessionOptions opts = new ConsumerProviderSessionOptions();
            opts.SetupDefaultLoginStream = true;
            opts.SetupDefaultDirectoryStream = true;

            provider.Bind(opts);
            TestReactor.OpenSession(consumer, provider, opts);

            List<int> view1List = new List<int> { 2, 3 };

            List<string> enameView1List = new List<string> { "FID2", "FID3" };

            int[] consStreamIds = { 5, 6 };

            /* Request first item. */
            IRequestMsg requestMsg = new Msg();
            requestMsg.StreamId = consStreamIds[0];
            requestMsg.MsgClass = MsgClasses.REQUEST;
            requestMsg.DomainType = (int)DomainType.MARKET_PRICE;
            requestMsg.ContainerType = DataTypes.NO_DATA;
            requestMsg.ApplyStreaming();
            requestMsg.ApplyHasQos();
            requestMsg.Qos.Timeliness(QosTimeliness.REALTIME);
            requestMsg.Qos.Rate(QosRates.TICK_BY_TICK);
            requestMsg.ApplyHasView();
            WatchlistItemDomainsTest.EncodeViewFieldIdList(consumer.ReactorChannel, view1List, requestMsg);

            submitOptions.Clear();
            submitOptions.ServiceName = Provider.DefaultService.Info.ServiceName.ToString();
            Assert.True(consumer.SubmitAndDispatch((Msg)requestMsg, submitOptions) >= ReactorReturnCode.SUCCESS);

            /* Provider receives request with field view. */
            providerReactor.Dispatch(1);
            evt = providerReactor.PollEvent();
            Assert.Equal(TestReactorEventType.MSG, evt.EventType);
            msgEvent = (ReactorMsgEvent)evt.ReactorEvent;
            Assert.Equal(MsgClasses.REQUEST, msgEvent.Msg.MsgClass);
            IRequestMsg receivedRequestMsg = (IRequestMsg)msgEvent.Msg;
            Assert.True(receivedRequestMsg.CheckStreaming());
            Assert.True(receivedRequestMsg.CheckHasPriority());
            Assert.Equal(1, receivedRequestMsg.Priority.PriorityClass);
            Assert.Equal(1, receivedRequestMsg.Priority.Count);
            Assert.False(receivedRequestMsg.CheckNoRefresh());
            Assert.True(WatchlistItemDomainsTest.CheckHasCorrectView(provider, receivedRequestMsg, view1List));
            int providerStreamId = receivedRequestMsg.StreamId;

            /* Request second item with element view. */
            requestMsg.StreamId = consStreamIds[1];
            requestMsg.ApplyHasView();
            WatchlistItemDomainsTest.EncodeViewElementNameList(consumer.ReactorChannel, enameView1List, requestMsg);

            submitOptions.Clear();
            submitOptions.ServiceName = Provider.DefaultService.Info.ServiceName.ToString();
            Assert.True(consumer.Submit((Msg)requestMsg, submitOptions) >= ReactorReturnCode.SUCCESS);

            /* Second request is rejected for trying to mix views. */
            consumerReactor.Dispatch(1);
            evt = consumerReactor.PollEvent();
            Assert.Equal(TestReactorEventType.MSG, evt.EventType);
            msgEvent = (ReactorMsgEvent)evt.ReactorEvent;
            Assert.Equal(MsgClasses.STATUS, msgEvent.Msg.MsgClass);
            IStatusMsg receivedStatusMsg = (IStatusMsg)msgEvent.Msg;
            Assert.Equal((int)DomainType.MARKET_PRICE, receivedStatusMsg.DomainType);
            Assert.Equal(DataTypes.NO_DATA, receivedStatusMsg.ContainerType);
            Assert.True(receivedStatusMsg.CheckHasState());
            Assert.Equal(StreamStates.CLOSED, receivedStatusMsg.State.StreamState());
            Assert.Equal(DataStates.SUSPECT, receivedStatusMsg.State.DataState());
            Assert.Equal(StateCodes.USAGE_ERROR, receivedStatusMsg.State.Code());
            Assert.Equal(consStreamIds[1], receivedStatusMsg.StreamId);

            /* Provider sends refresh, satisfying first view. */
            IRefreshMsg refreshMsg = new Msg();
            refreshMsg.MsgClass = MsgClasses.REFRESH;
            refreshMsg.DomainType = (int)DomainType.MARKET_PRICE;
            refreshMsg.StreamId = providerStreamId;
            refreshMsg.ContainerType = DataTypes.NO_DATA;
            refreshMsg.ApplyRefreshComplete();
            refreshMsg.ApplySolicited();
            refreshMsg.ApplyClearCache();
            refreshMsg.State.StreamState(StreamStates.OPEN);
            refreshMsg.State.DataState(DataStates.OK);
            refreshMsg.ApplyHasQos();
            refreshMsg.Qos.Timeliness(QosTimeliness.REALTIME);
            refreshMsg.Qos.Rate(QosRates.TICK_BY_TICK);

            submitOptions.Clear();
            Assert.True(provider.SubmitAndDispatch((Msg)refreshMsg, submitOptions) >= ReactorReturnCode.SUCCESS);

            /* Consumer receives refresh. */
            consumerReactor.Dispatch(1);
            evt = consumerReactor.PollEvent();
            Assert.Equal(TestReactorEventType.MSG, evt.EventType);
            msgEvent = (ReactorMsgEvent)evt.ReactorEvent;
            Assert.Equal(MsgClasses.REFRESH, msgEvent.Msg.MsgClass);
            IRefreshMsg receivedRefreshMsg = (IRefreshMsg)msgEvent.Msg;
            Assert.Equal((int)DomainType.MARKET_PRICE, receivedRefreshMsg.DomainType);
            Assert.Equal(DataTypes.NO_DATA, receivedRefreshMsg.ContainerType);
            Assert.Equal(StreamStates.OPEN, receivedRefreshMsg.State.StreamState());
            Assert.Equal(DataStates.OK, receivedRefreshMsg.State.DataState());
            Assert.Equal(consStreamIds[0], receivedRefreshMsg.StreamId);

            /* Close first item. */
            ICloseMsg closeMsg = new Msg();
            closeMsg.MsgClass = MsgClasses.CLOSE;
            closeMsg.StreamId = consStreamIds[0];

            submitOptions.Clear();
            Assert.True(consumer.SubmitAndDispatch((Msg)closeMsg, submitOptions) >= ReactorReturnCode.SUCCESS);

            /* Provider receives close. */
            providerReactor.Dispatch(1);
            evt = providerReactor.PollEvent();
            Assert.Equal(TestReactorEventType.MSG, evt.EventType);
            msgEvent = (ReactorMsgEvent)evt.ReactorEvent;
            Assert.Equal(MsgClasses.CLOSE, msgEvent.Msg.MsgClass);
            ICloseMsg receivedCloseMsg = (ICloseMsg)msgEvent.Msg;
            Assert.Equal(providerStreamId, receivedCloseMsg.StreamId);

            TestReactorComponent.CloseSession(consumer, provider);
            consumerReactor.Close();
            providerReactor.Close();
        }

        [Fact]
        public void WatchlistThreeItemsInMsgBuffer_BatchTest()
        {
            ReactorSubmitOptions submitOptions = new ReactorSubmitOptions();
            TestReactorEvent evt;
            ReactorMsgEvent msgEvent;

            TestReactor consumerReactor = new TestReactor();
            TestReactor providerReactor = new TestReactor();

            Consumer consumer = new Consumer(consumerReactor);
            ConsumerRole consumerRole = (ConsumerRole)consumer.ReactorRole;
            consumerRole.InitDefaultRDMLoginRequest();
            consumerRole.InitDefaultRDMDirectoryRequest();
            consumerRole.ChannelEventCallback = consumer;
            consumerRole.LoginMsgCallback = consumer;
            consumerRole.DirectoryMsgCallback = consumer;
            consumerRole.DictionaryMsgCallback = consumer;
            consumerRole.DefaultMsgCallback = consumer;
            consumerRole.WatchlistOptions.EnableWatchlist = true;
            consumerRole.WatchlistOptions.ChannelOpenEventCallback = consumer;

            Provider provider = new Provider(providerReactor);
            ProviderRole providerRole = (ProviderRole)provider.ReactorRole;
            providerRole.ChannelEventCallback = provider;
            providerRole.LoginMsgCallback = provider;
            providerRole.DirectoryMsgCallback = provider;
            providerRole.DictionaryMsgCallback = provider;
            providerRole.DefaultMsgCallback = provider;

            ConsumerProviderSessionOptions opts = new ConsumerProviderSessionOptions();
            opts.SetupDefaultLoginStream = true;
            opts.SetupDefaultDirectoryStream = true;

            provider.Bind(opts);
            TestReactor.OpenSession(consumer, provider, opts);

            List<string> itemList = new List<string>() { "TRI.N", "TRI.N", "WJI" };

            int[] domains = { (int)DomainType.MARKET_PRICE, (int)DomainType.SYMBOL_LIST, (int)DomainType.MARKET_BY_ORDER };

            int[] consStreamIds = { 5, 6, 7, 8 };

            IRequestMsg requestMsg = new Msg();
            IRefreshMsg refreshMsg = new Msg();
            IStatusMsg receivedStatusMsg;
            IRequestMsg receivedRequestMsg;
            IRefreshMsg receivedRefreshMsg;

            for (int i = 0; i < 3; i++)
            {
                /* Request items. */
                requestMsg.Clear();
                requestMsg.StreamId = consStreamIds[0];
                requestMsg.MsgClass = MsgClasses.REQUEST;
                requestMsg.DomainType = domains[i];
                requestMsg.ApplyStreaming();
                requestMsg.ApplyHasQos();
                requestMsg.Qos.Timeliness(QosTimeliness.REALTIME);
                requestMsg.Qos.Rate(QosRates.TICK_BY_TICK);
                requestMsg.ApplyHasBatch();
                WatchlistItemDomainsTest.EncodeBatchWithView(consumer.ReactorChannel, requestMsg, itemList, null);

                submitOptions.Clear();
                submitOptions.ServiceName = Provider.DefaultService.Info.ServiceName.ToString();
                submitOptions.RequestMsgOptions.UserSpecObj = "77777";
                Assert.True(consumer.Submit((Msg)requestMsg, submitOptions) >= ReactorReturnCode.SUCCESS);

                /* Consumer receives close ack for batch. */
                consumerReactor.Dispatch(1);
                evt = consumerReactor.PollEvent();
                Assert.Equal(TestReactorEventType.MSG, evt.EventType);
                msgEvent = (ReactorMsgEvent)evt.ReactorEvent;
                Assert.Equal(MsgClasses.STATUS, msgEvent.Msg.MsgClass);

                // Received status message with closed batch stream
                receivedStatusMsg = (IStatusMsg)msgEvent.Msg;
                Assert.Equal(domains[i], receivedStatusMsg.DomainType);
                Assert.True(receivedStatusMsg.CheckHasState());
                Assert.Equal(DataTypes.NO_DATA, receivedStatusMsg.ContainerType);
                Assert.Equal(StreamStates.CLOSED, receivedStatusMsg.State.StreamState());
                Assert.Equal(DataStates.OK, receivedStatusMsg.State.DataState());
                Assert.Equal(consStreamIds[0], receivedStatusMsg.StreamId);
                Assert.Equal("Stream closed for batch", receivedStatusMsg.State.Text().ToString());
                Assert.Equal("77777", msgEvent.StreamInfo.UserSpec);

                /* No additional consumer event */
                consumerReactor.Dispatch(0);

                /* Provider receives request for TRI.N. */
                providerReactor.Dispatch(2);
                evt = providerReactor.PollEvent();
                Assert.Equal(TestReactorEventType.MSG, evt.EventType);
                msgEvent = (ReactorMsgEvent)evt.ReactorEvent;
                Assert.Equal(MsgClasses.REQUEST, msgEvent.Msg.MsgClass);
                receivedRequestMsg = (IRequestMsg)msgEvent.Msg;
                Assert.True(receivedRequestMsg.CheckStreaming());
                Assert.True(receivedRequestMsg.CheckHasPriority());
                Assert.Equal(1, receivedRequestMsg.Priority.PriorityClass);
                Assert.Equal(2, receivedRequestMsg.Priority.Count);
                Assert.False(receivedRequestMsg.CheckNoRefresh());
                Assert.True(receivedRequestMsg.MsgKey.CheckHasName());
                Assert.Equal(itemList[0], receivedRequestMsg.MsgKey.Name.ToString());
                int providerTriStreamId = receivedRequestMsg.StreamId;

                /* Provider receives request for WJI. */
                evt = providerReactor.PollEvent();
                Assert.Equal(TestReactorEventType.MSG, evt.EventType);
                msgEvent = (ReactorMsgEvent)evt.ReactorEvent;
                Assert.Equal(MsgClasses.REQUEST, msgEvent.Msg.MsgClass);
                receivedRequestMsg = (IRequestMsg)msgEvent.Msg;
                Assert.True(receivedRequestMsg.CheckStreaming());
                Assert.True(receivedRequestMsg.CheckHasPriority());
                Assert.Equal(1, receivedRequestMsg.Priority.PriorityClass);
                Assert.Equal(1, receivedRequestMsg.Priority.Count);
                Assert.False(receivedRequestMsg.CheckNoRefresh());
                Assert.True(receivedRequestMsg.MsgKey.CheckHasName());
                Assert.Equal(itemList[2], receivedRequestMsg.MsgKey.Name.ToString());
                int providerWjiStreamId = receivedRequestMsg.StreamId;

                /* Provider sends refreshes */
                refreshMsg.Clear();
                refreshMsg.MsgClass = MsgClasses.REFRESH;
                refreshMsg.DomainType = domains[i];
                refreshMsg.StreamId = providerTriStreamId;
                refreshMsg.ContainerType = DataTypes.NO_DATA;
                refreshMsg.ApplyRefreshComplete();
                refreshMsg.ApplySolicited();
                refreshMsg.ApplyClearCache();
                refreshMsg.State.StreamState(StreamStates.OPEN);
                refreshMsg.State.DataState(DataStates.OK);
                refreshMsg.ApplyHasQos();
                refreshMsg.Qos.Timeliness(QosTimeliness.REALTIME);
                refreshMsg.Qos.Rate(QosRates.TICK_BY_TICK);

                submitOptions.Clear();
                Assert.True(provider.SubmitAndDispatch((Msg)refreshMsg, submitOptions) >= ReactorReturnCode.SUCCESS);

                refreshMsg.Clear();
                refreshMsg.MsgClass = MsgClasses.REFRESH;
                refreshMsg.DomainType = domains[i];
                refreshMsg.StreamId = providerWjiStreamId;
                refreshMsg.ContainerType = DataTypes.NO_DATA;
                refreshMsg.ApplyRefreshComplete();
                refreshMsg.ApplySolicited();
                refreshMsg.ApplyClearCache();
                refreshMsg.State.StreamState(StreamStates.OPEN);
                refreshMsg.State.DataState(DataStates.OK);
                refreshMsg.ApplyHasQos();
                refreshMsg.Qos.Timeliness(QosTimeliness.REALTIME);
                refreshMsg.Qos.Rate(QosRates.TICK_BY_TICK);

                submitOptions.Clear();
                Assert.True(provider.SubmitAndDispatch((Msg)refreshMsg, submitOptions) >= ReactorReturnCode.SUCCESS);

                /* Consumer receives refreshes. */
                consumerReactor.Dispatch(3);
                evt = consumerReactor.PollEvent();
                Assert.Equal(TestReactorEventType.MSG, evt.EventType);
                msgEvent = (ReactorMsgEvent)evt.ReactorEvent;
                Assert.Equal(MsgClasses.REFRESH, msgEvent.Msg.MsgClass);
                receivedRefreshMsg = (IRefreshMsg)msgEvent.Msg;
                Assert.Equal(domains[i], receivedRefreshMsg.DomainType);
                Assert.Equal(DataTypes.NO_DATA, receivedRefreshMsg.ContainerType);
                Assert.Equal(StreamStates.OPEN, receivedRefreshMsg.State.StreamState());
                Assert.Equal(DataStates.OK, receivedRefreshMsg.State.DataState());
                Assert.Equal(consStreamIds[1], receivedRefreshMsg.StreamId);
                Assert.Equal("77777", msgEvent.StreamInfo.UserSpec);
                Assert.Equal(Provider.DefaultService.Info.ServiceName.ToString(), msgEvent.StreamInfo.ServiceName);

                evt = consumerReactor.PollEvent();
                Assert.Equal(TestReactorEventType.MSG, evt.EventType);
                msgEvent = (ReactorMsgEvent)evt.ReactorEvent;
                Assert.Equal(MsgClasses.REFRESH, msgEvent.Msg.MsgClass);
                receivedRefreshMsg = (IRefreshMsg)msgEvent.Msg;
                Assert.Equal(domains[i], receivedRefreshMsg.DomainType);
                Assert.Equal(DataTypes.NO_DATA, receivedRefreshMsg.ContainerType);
                Assert.Equal(StreamStates.OPEN, receivedRefreshMsg.State.StreamState());
                Assert.Equal(DataStates.OK, receivedRefreshMsg.State.DataState());
                Assert.Equal(consStreamIds[2], receivedRefreshMsg.StreamId);
                Assert.Equal("77777", msgEvent.StreamInfo.UserSpec);
                Assert.Equal(Provider.DefaultService.Info.ServiceName.ToString(), msgEvent.StreamInfo.ServiceName);

                evt = consumerReactor.PollEvent();
                Assert.Equal(TestReactorEventType.MSG, evt.EventType);
                msgEvent = (ReactorMsgEvent)evt.ReactorEvent;
                Assert.Equal(MsgClasses.REFRESH, msgEvent.Msg.MsgClass);
                receivedRefreshMsg = (IRefreshMsg)msgEvent.Msg;
                Assert.Equal(domains[i], receivedRefreshMsg.DomainType);
                Assert.Equal(DataTypes.NO_DATA, receivedRefreshMsg.ContainerType);
                Assert.Equal(StreamStates.OPEN, receivedRefreshMsg.State.StreamState());
                Assert.Equal(DataStates.OK, receivedRefreshMsg.State.DataState());
                Assert.Equal(consStreamIds[3], receivedRefreshMsg.StreamId);
                Assert.Equal("77777", msgEvent.StreamInfo.UserSpec);
                Assert.Equal(Provider.DefaultService.Info.ServiceName.ToString(), msgEvent.StreamInfo.ServiceName);

                /* Close first item. */
                ICloseMsg closeMsg = new Msg();
                closeMsg.MsgClass = MsgClasses.CLOSE;
                closeMsg.StreamId = consStreamIds[1];
                closeMsg.DomainType = domains[i];

                submitOptions.Clear();
                Assert.True(consumer.SubmitAndDispatch((Msg)closeMsg, submitOptions) >= ReactorReturnCode.SUCCESS);

                /* Provider receives request -- same stream, no refresh, updated priority. */
                providerReactor.Dispatch(1);
                evt = providerReactor.PollEvent();
                Assert.Equal(TestReactorEventType.MSG, evt.EventType);
                msgEvent = (ReactorMsgEvent)evt.ReactorEvent;
                Assert.Equal(MsgClasses.REQUEST, msgEvent.Msg.MsgClass);
                receivedRequestMsg = (IRequestMsg)msgEvent.Msg;
                Assert.True(receivedRequestMsg.CheckStreaming());
                Assert.True(receivedRequestMsg.CheckHasPriority());
                Assert.Equal(1, receivedRequestMsg.Priority.PriorityClass);
                Assert.Equal(1, receivedRequestMsg.Priority.Count);
                Assert.True(receivedRequestMsg.CheckNoRefresh());
                Assert.Equal(providerTriStreamId, receivedRequestMsg.StreamId);

                /* Close second item. */
                closeMsg.MsgClass = MsgClasses.CLOSE;
                closeMsg.StreamId = consStreamIds[2];
                closeMsg.DomainType = domains[i];

                submitOptions.Clear();
                Assert.True(consumer.SubmitAndDispatch((Msg)closeMsg, submitOptions) >= ReactorReturnCode.SUCCESS);

                providerReactor.Dispatch(1);
                evt = providerReactor.PollEvent();
                Assert.Equal(TestReactorEventType.MSG, evt.EventType);
                msgEvent = (ReactorMsgEvent)evt.ReactorEvent;
                Assert.Equal(MsgClasses.CLOSE, msgEvent.Msg.MsgClass);
                ICloseMsg receivedCloseMsg = (ICloseMsg)msgEvent.Msg;
                Assert.Equal(providerTriStreamId, receivedCloseMsg.StreamId);

                /* Close third item. */
                closeMsg.MsgClass = MsgClasses.CLOSE;
                closeMsg.StreamId = consStreamIds[3];
                closeMsg.DomainType = domains[i];

                submitOptions.Clear();
                Assert.True(consumer.SubmitAndDispatch((Msg)closeMsg, submitOptions) >= ReactorReturnCode.SUCCESS);

                providerReactor.Dispatch(1);
                evt = providerReactor.PollEvent();
                Assert.Equal(TestReactorEventType.MSG, evt.EventType);
                msgEvent = (ReactorMsgEvent)evt.ReactorEvent;
                Assert.Equal(MsgClasses.CLOSE, msgEvent.Msg.MsgClass);
                receivedCloseMsg = (ICloseMsg)msgEvent.Msg;
                Assert.Equal(providerWjiStreamId, receivedCloseMsg.StreamId);
            }

            TestReactorComponent.CloseSession(consumer, provider);
            consumerReactor.Close();
            providerReactor.Close();
        }

        [Fact]
        public void WatchlistThreeItemsInMsgBuffer_BatchWithViewTest()
        {
            ReactorSubmitOptions submitOptions = new ReactorSubmitOptions();
            TestReactorEvent evt;
            ReactorMsgEvent msgEvent;

            TestReactor consumerReactor = new TestReactor();
            TestReactor providerReactor = new TestReactor();

            Consumer consumer = new Consumer(consumerReactor);
            ConsumerRole consumerRole = (ConsumerRole)consumer.ReactorRole;
            consumerRole.InitDefaultRDMLoginRequest();
            consumerRole.InitDefaultRDMDirectoryRequest();
            consumerRole.ChannelEventCallback = consumer;
            consumerRole.LoginMsgCallback = consumer;
            consumerRole.DirectoryMsgCallback = consumer;
            consumerRole.DictionaryMsgCallback = consumer;
            consumerRole.DefaultMsgCallback = consumer;
            consumerRole.WatchlistOptions.EnableWatchlist = true;
            consumerRole.WatchlistOptions.ChannelOpenEventCallback = consumer;

            Provider provider = new Provider(providerReactor);
            ProviderRole providerRole = (ProviderRole)provider.ReactorRole;
            providerRole.ChannelEventCallback = provider;
            providerRole.LoginMsgCallback = provider;
            providerRole.DirectoryMsgCallback = provider;
            providerRole.DictionaryMsgCallback = provider;
            providerRole.DefaultMsgCallback = provider;

            ConsumerProviderSessionOptions opts = new ConsumerProviderSessionOptions();
            opts.SetupDefaultLoginStream = true;
            opts.SetupDefaultDirectoryStream = true;

            provider.Bind(opts);
            TestReactor.OpenSession(consumer, provider, opts);

            List<string> itemList = new List<string>() { "TRI.N", "TRI.N", ".DJI" };

            List<int> view1List = new List<int>() { 2, 6, 7 };

            int[] domains = { (int)DomainType.MARKET_PRICE, (int)DomainType.SYMBOL_LIST, (int)DomainType.MARKET_BY_ORDER };

            int[] consStreamIds = { 5, 6, 7, 8 };

            IRequestMsg requestMsg = new Msg();
            IRefreshMsg refreshMsg = new Msg();
            IStatusMsg receivedStatusMsg;
            IRequestMsg receivedRequestMsg;
            IRefreshMsg receivedRefreshMsg;

            for (int i = 0; i < 3; i++)
            {
                /* Request items. */
                requestMsg.Clear();
                requestMsg.StreamId = consStreamIds[0];
                requestMsg.MsgClass = MsgClasses.REQUEST;
                requestMsg.DomainType = domains[i];
                requestMsg.ApplyStreaming();
                requestMsg.ApplyHasQos();
                requestMsg.Qos.Timeliness(QosTimeliness.REALTIME);
                requestMsg.Qos.Rate(QosRates.TICK_BY_TICK);
                requestMsg.ApplyHasBatch();
                requestMsg.ApplyHasView();
                WatchlistItemDomainsTest.EncodeBatchWithView(consumer.ReactorChannel, requestMsg, itemList, view1List);

                submitOptions.Clear();
                submitOptions.ServiceName = Provider.DefaultService.Info.ServiceName.ToString();
                submitOptions.RequestMsgOptions.UserSpecObj = "77777";
                Assert.True(consumer.Submit((Msg)requestMsg, submitOptions) >= ReactorReturnCode.SUCCESS);

                /* Consumer receives close ack for batch. */
                consumerReactor.Dispatch(1);
                evt = consumerReactor.PollEvent();
                Assert.Equal(TestReactorEventType.MSG, evt.EventType);
                msgEvent = (ReactorMsgEvent)evt.ReactorEvent;
                Assert.Equal(MsgClasses.STATUS, msgEvent.Msg.MsgClass);

                // Received status message with closed batch stream
                receivedStatusMsg = (IStatusMsg)msgEvent.Msg;
                Assert.Equal(domains[i], receivedStatusMsg.DomainType);
                Assert.True(receivedStatusMsg.CheckHasState());
                Assert.Equal(DataTypes.NO_DATA, receivedStatusMsg.ContainerType);
                Assert.Equal(StreamStates.CLOSED, receivedStatusMsg.State.StreamState());
                Assert.Equal(DataStates.OK, receivedStatusMsg.State.DataState());
                Assert.Equal(consStreamIds[0], receivedStatusMsg.StreamId);
                Assert.Equal("Stream closed for batch", receivedStatusMsg.State.Text().ToString());
                Assert.Equal("77777", msgEvent.StreamInfo.UserSpec);

                /* No additional consumer event */
                consumerReactor.Dispatch(0);

                /* Provider receives request for TRI.N. */
                providerReactor.Dispatch(2);
                evt = providerReactor.PollEvent();
                Assert.Equal(TestReactorEventType.MSG, evt.EventType);
                msgEvent = (ReactorMsgEvent)evt.ReactorEvent;
                Assert.Equal(MsgClasses.REQUEST, msgEvent.Msg.MsgClass);
                receivedRequestMsg = (IRequestMsg)msgEvent.Msg;
                Assert.True(receivedRequestMsg.CheckStreaming());
                Assert.True(receivedRequestMsg.CheckHasPriority());
                Assert.Equal(1, receivedRequestMsg.Priority.PriorityClass);
                Assert.Equal(2, receivedRequestMsg.Priority.Count);
                Assert.False(receivedRequestMsg.CheckNoRefresh());
                Assert.True(receivedRequestMsg.MsgKey.CheckHasName());
                Assert.Equal(itemList[0], receivedRequestMsg.MsgKey.Name.ToString());
                int providerTriStreamId = receivedRequestMsg.StreamId;
                WatchlistItemDomainsTest.CheckHasCorrectView(provider, receivedRequestMsg, view1List);

                /* Provider receives request for .DJI. */
                evt = providerReactor.PollEvent();
                Assert.Equal(TestReactorEventType.MSG, evt.EventType);
                msgEvent = (ReactorMsgEvent)evt.ReactorEvent;
                Assert.Equal(MsgClasses.REQUEST, msgEvent.Msg.MsgClass);
                receivedRequestMsg = (IRequestMsg)msgEvent.Msg;
                Assert.True(receivedRequestMsg.CheckStreaming());
                Assert.True(receivedRequestMsg.CheckHasPriority());
                Assert.Equal(1, receivedRequestMsg.Priority.PriorityClass);
                Assert.Equal(1, receivedRequestMsg.Priority.Count);
                Assert.False(receivedRequestMsg.CheckNoRefresh());
                Assert.True(receivedRequestMsg.MsgKey.CheckHasName());
                Assert.Equal(itemList[2], receivedRequestMsg.MsgKey.Name.ToString());
                int providerDjiStreamId = receivedRequestMsg.StreamId;
                WatchlistItemDomainsTest.CheckHasCorrectView(provider, receivedRequestMsg, view1List);

                /* Provider sends refreshes */
                refreshMsg.Clear();
                refreshMsg.MsgClass = MsgClasses.REFRESH;
                refreshMsg.DomainType = domains[i];
                refreshMsg.StreamId = providerTriStreamId;
                refreshMsg.ContainerType = DataTypes.NO_DATA;
                refreshMsg.ApplyRefreshComplete();
                refreshMsg.ApplySolicited();
                refreshMsg.ApplyClearCache();
                refreshMsg.State.StreamState(StreamStates.OPEN);
                refreshMsg.State.DataState(DataStates.OK);
                refreshMsg.ApplyHasQos();
                refreshMsg.Qos.Timeliness(QosTimeliness.REALTIME);
                refreshMsg.Qos.Rate(QosRates.TICK_BY_TICK);

                submitOptions.Clear();
                Assert.True(provider.SubmitAndDispatch((Msg)refreshMsg, submitOptions) >= ReactorReturnCode.SUCCESS);

                refreshMsg.Clear();
                refreshMsg.MsgClass = MsgClasses.REFRESH;
                refreshMsg.DomainType = domains[i];
                refreshMsg.StreamId = providerDjiStreamId;
                refreshMsg.ContainerType = DataTypes.NO_DATA;
                refreshMsg.ApplyRefreshComplete();
                refreshMsg.ApplySolicited();
                refreshMsg.ApplyClearCache();
                refreshMsg.State.StreamState(StreamStates.OPEN);
                refreshMsg.State.DataState(DataStates.OK);
                refreshMsg.ApplyHasQos();
                refreshMsg.Qos.Timeliness(QosTimeliness.REALTIME);
                refreshMsg.Qos.Rate(QosRates.TICK_BY_TICK);

                submitOptions.Clear();
                Assert.True(provider.SubmitAndDispatch((Msg)refreshMsg, submitOptions) >= ReactorReturnCode.SUCCESS);

                /* Consumer receives refreshes. */
                consumerReactor.Dispatch(3);
                evt = consumerReactor.PollEvent();
                Assert.Equal(TestReactorEventType.MSG, evt.EventType);
                msgEvent = (ReactorMsgEvent)evt.ReactorEvent;
                Assert.Equal(MsgClasses.REFRESH, msgEvent.Msg.MsgClass);
                receivedRefreshMsg = (IRefreshMsg)msgEvent.Msg;
                Assert.Equal(domains[i], receivedRefreshMsg.DomainType);
                Assert.Equal(DataTypes.NO_DATA, receivedRefreshMsg.ContainerType);
                Assert.Equal(StreamStates.OPEN, receivedRefreshMsg.State.StreamState());
                Assert.Equal(DataStates.OK, receivedRefreshMsg.State.DataState());
                Assert.Equal(consStreamIds[1], receivedRefreshMsg.StreamId);
                Assert.Equal("77777", msgEvent.StreamInfo.UserSpec);
                Assert.Equal(Provider.DefaultService.Info.ServiceName.ToString(), msgEvent.StreamInfo.ServiceName);

                evt = consumerReactor.PollEvent();
                Assert.Equal(TestReactorEventType.MSG, evt.EventType);
                msgEvent = (ReactorMsgEvent)evt.ReactorEvent;
                Assert.Equal(MsgClasses.REFRESH, msgEvent.Msg.MsgClass);
                receivedRefreshMsg = (IRefreshMsg)msgEvent.Msg;
                Assert.Equal(domains[i], receivedRefreshMsg.DomainType);
                Assert.Equal(DataTypes.NO_DATA, receivedRefreshMsg.ContainerType);
                Assert.Equal(StreamStates.OPEN, receivedRefreshMsg.State.StreamState());
                Assert.Equal(DataStates.OK, receivedRefreshMsg.State.DataState());
                Assert.Equal(consStreamIds[2], receivedRefreshMsg.StreamId);
                Assert.Equal("77777", msgEvent.StreamInfo.UserSpec);
                Assert.Equal(Provider.DefaultService.Info.ServiceName.ToString(), msgEvent.StreamInfo.ServiceName);

                evt = consumerReactor.PollEvent();
                Assert.Equal(TestReactorEventType.MSG, evt.EventType);
                msgEvent = (ReactorMsgEvent)evt.ReactorEvent;
                Assert.Equal(MsgClasses.REFRESH, msgEvent.Msg.MsgClass);
                receivedRefreshMsg = (IRefreshMsg)msgEvent.Msg;
                Assert.Equal(domains[i], receivedRefreshMsg.DomainType);
                Assert.Equal(DataTypes.NO_DATA, receivedRefreshMsg.ContainerType);
                Assert.Equal(StreamStates.OPEN, receivedRefreshMsg.State.StreamState());
                Assert.Equal(DataStates.OK, receivedRefreshMsg.State.DataState());
                Assert.Equal(consStreamIds[3], receivedRefreshMsg.StreamId);
                Assert.Equal("77777", msgEvent.StreamInfo.UserSpec);
                Assert.Equal(Provider.DefaultService.Info.ServiceName.ToString(), msgEvent.StreamInfo.ServiceName);

                /* Close first item. */
                ICloseMsg closeMsg = new Msg();
                closeMsg.MsgClass = MsgClasses.CLOSE;
                closeMsg.StreamId = consStreamIds[1];
                closeMsg.DomainType = domains[i];

                submitOptions.Clear();
                Assert.True(consumer.SubmitAndDispatch((Msg)closeMsg, submitOptions) >= ReactorReturnCode.SUCCESS);

                /* Provider receives request -- same stream, no refresh, updated priority. */
                providerReactor.Dispatch(1);
                evt = providerReactor.PollEvent();
                Assert.Equal(TestReactorEventType.MSG, evt.EventType);
                msgEvent = (ReactorMsgEvent)evt.ReactorEvent;
                Assert.Equal(MsgClasses.REQUEST, msgEvent.Msg.MsgClass);
                receivedRequestMsg = (IRequestMsg)msgEvent.Msg;
                Assert.True(receivedRequestMsg.CheckStreaming());
                Assert.True(receivedRequestMsg.CheckHasPriority());
                Assert.Equal(1, receivedRequestMsg.Priority.PriorityClass);
                Assert.Equal(1, receivedRequestMsg.Priority.Count);
                Assert.True(receivedRequestMsg.CheckNoRefresh());
                Assert.Equal(providerTriStreamId, receivedRequestMsg.StreamId);

                /* Close second item. */
                closeMsg.MsgClass = MsgClasses.CLOSE;
                closeMsg.StreamId = consStreamIds[2];
                closeMsg.DomainType = domains[i];

                submitOptions.Clear();
                Assert.True(consumer.SubmitAndDispatch((Msg)closeMsg, submitOptions) >= ReactorReturnCode.SUCCESS);

                providerReactor.Dispatch(1);
                evt = providerReactor.PollEvent();
                Assert.Equal(TestReactorEventType.MSG, evt.EventType);
                msgEvent = (ReactorMsgEvent)evt.ReactorEvent;
                Assert.Equal(MsgClasses.CLOSE, msgEvent.Msg.MsgClass);
                ICloseMsg receivedCloseMsg = (ICloseMsg)msgEvent.Msg;
                Assert.Equal(providerTriStreamId, receivedCloseMsg.StreamId);

                /* Close third item. */
                closeMsg.MsgClass = MsgClasses.CLOSE;
                closeMsg.StreamId = consStreamIds[3];
                closeMsg.DomainType = domains[i];

                submitOptions.Clear();
                Assert.True(consumer.SubmitAndDispatch((Msg)closeMsg, submitOptions) >= ReactorReturnCode.SUCCESS);

                providerReactor.Dispatch(1);
                evt = providerReactor.PollEvent();
                Assert.Equal(TestReactorEventType.MSG, evt.EventType);
                msgEvent = (ReactorMsgEvent)evt.ReactorEvent;
                Assert.Equal(MsgClasses.CLOSE, msgEvent.Msg.MsgClass);
                receivedCloseMsg = (ICloseMsg)msgEvent.Msg;
                Assert.Equal(providerDjiStreamId, receivedCloseMsg.StreamId);
            }

            TestReactorComponent.CloseSession(consumer, provider);
            consumerReactor.Close();
            providerReactor.Close();
        }

        [Fact]
        public void WatchlistOneItem_QosRangeTest()
        {
            ReactorSubmitOptions submitOptions = new();
            TestReactorEvent evt;
            ReactorMsgEvent msgEvent;
            Msg msg = new();
            IRequestMsg requestMsg = msg;

            /* Create reactors. */
            TestReactor consumerReactor = new();
            TestReactor providerReactor = new();

            /* Create consumer. */
            Consumer consumer = new(consumerReactor);
            ConsumerRole consumerRole = (ConsumerRole)consumer.ReactorRole;
            consumerRole.InitDefaultRDMLoginRequest();
            consumerRole.InitDefaultRDMDirectoryRequest();
            consumerRole.ChannelEventCallback = consumer;
            consumerRole.LoginMsgCallback = consumer;
            consumerRole.DirectoryMsgCallback = consumer;
            consumerRole.DictionaryMsgCallback = consumer;
            consumerRole.DefaultMsgCallback = consumer;
            consumerRole.WatchlistOptions.EnableWatchlist = true;
            consumerRole.WatchlistOptions.ChannelOpenEventCallback = consumer;
            consumerRole.WatchlistOptions.RequestTimeout = 15000;

            /* Create provider. */
            Provider provider = new(providerReactor);
            ProviderRole providerRole = (ProviderRole)provider.ReactorRole;
            providerRole.ChannelEventCallback = provider;
            providerRole.LoginMsgCallback = provider;
            providerRole.DirectoryMsgCallback = provider;
            providerRole.DictionaryMsgCallback = provider;
            providerRole.DefaultMsgCallback = provider;

            /* Connect the consumer and provider. Setup login & directory streams automatically. */
            ConsumerProviderSessionOptions opts = new();
            opts.SetupDefaultLoginStream = true;
            opts.SetupDefaultDirectoryStream = true;

            provider.Bind(opts);

            TestReactor.OpenSession(consumer, provider, opts);

            /* Request first item. */
            requestMsg.Clear();
            requestMsg.StreamId = 5;
            requestMsg.MsgClass = MsgClasses.REQUEST;
            requestMsg.DomainType = (int)DomainType.MARKET_PRICE;
            requestMsg.ContainerType = DataTypes.NO_DATA;
            requestMsg.ApplyStreaming();
            requestMsg.ApplyHasQos();
            requestMsg.Qos.Timeliness(QosTimeliness.REALTIME);
            requestMsg.Qos.Rate(QosRates.TICK_BY_TICK);
            requestMsg.ApplyHasWorstQos();
            requestMsg.WorstQos.Timeliness(QosTimeliness.DELAYED);
            requestMsg.WorstQos.Rate(QosRates.JIT_CONFLATED);

            submitOptions.Clear();
            submitOptions.ServiceName = Provider.DefaultService.Info.ServiceName.ToString();
            Assert.True(consumer.SubmitAndDispatch((Msg)requestMsg, submitOptions) >= ReactorReturnCode.SUCCESS);

            /* Provider receives request. */
            providerReactor.Dispatch(1);
            evt = providerReactor.PollEvent();
            Assert.Equal(TestReactorEventType.MSG, evt.EventType);
            msgEvent = (ReactorMsgEvent)evt.ReactorEvent;
            Assert.Equal(MsgClasses.REQUEST, msgEvent.Msg.MsgClass);
            IRequestMsg receivedRequestMsg = (IRequestMsg)msgEvent.Msg;
            Assert.True(receivedRequestMsg.CheckStreaming());
            Assert.True(receivedRequestMsg.CheckHasPriority());
            Assert.Equal(1, receivedRequestMsg.Priority.PriorityClass);
            Assert.Equal(1, receivedRequestMsg.Priority.Count);
            Assert.False(receivedRequestMsg.CheckNoRefresh());
            Assert.True(receivedRequestMsg.CheckHasQos());
            Assert.False(receivedRequestMsg.CheckHasWorstQos());
            Assert.Equal(QosTimeliness.REALTIME, receivedRequestMsg.Qos.Timeliness());
            Assert.Equal(QosRates.TICK_BY_TICK, receivedRequestMsg.Qos.Rate());
            int providerStreamId = receivedRequestMsg.StreamId;

            /* Provider sends closed status message. */
            IStatusMsg statusMsg = new Msg();
            statusMsg.MsgClass = MsgClasses.STATUS;
            statusMsg.StreamId = providerStreamId;
            statusMsg.DomainType = (int)DomainType.MARKET_PRICE;
            statusMsg.ContainerType = DataTypes.NO_DATA;
            statusMsg.ApplyHasState();
            statusMsg.State.StreamState(StreamStates.CLOSED);
            statusMsg.State.DataState(DataStates.SUSPECT);

            submitOptions.Clear();
            Assert.True(provider.SubmitAndDispatch((Msg)statusMsg, submitOptions) >= ReactorReturnCode.SUCCESS);

            /* Consumer receives Closed status. */
            consumerReactor.Dispatch(1);
            evt = consumerReactor.PollEvent();
            Assert.Equal(TestReactorEventType.MSG, evt.EventType);
            msgEvent = (ReactorMsgEvent)evt.ReactorEvent;
            Assert.Equal(MsgClasses.STATUS, msgEvent.Msg.MsgClass);
            IStatusMsg receivedStatusMsg = (IStatusMsg)msgEvent.Msg;
            Assert.Equal((int)DomainType.MARKET_PRICE, receivedStatusMsg.DomainType);
            Assert.Equal(DataTypes.NO_DATA, receivedStatusMsg.ContainerType);
            Assert.True(receivedStatusMsg.CheckHasState());
            Assert.Equal(StreamStates.CLOSED, receivedStatusMsg.State.StreamState());
            Assert.Equal(DataStates.SUSPECT, receivedStatusMsg.State.DataState());
            Assert.Equal(5, receivedStatusMsg.StreamId);

            /* Provider receives nothing. */
            providerReactor.Dispatch(0);

            TestReactorComponent.CloseSession(consumer, provider);
            consumerReactor.Close();
            providerReactor.Close();
        }

        [Fact]
        public void WatchlistOneItem_NoCapabilityTest()
        {
            ReactorSubmitOptions submitOptions = new();
            TestReactorEvent evt;
            ReactorMsgEvent msgEvent;
            Msg msg = new();
            IRequestMsg requestMsg = msg;

            /* Create reactors. */
            TestReactor consumerReactor = new();
            TestReactor providerReactor = new();

            /* Create consumer. */
            Consumer consumer = new(consumerReactor);
            ConsumerRole consumerRole = (ConsumerRole)consumer.ReactorRole;
            consumerRole.InitDefaultRDMLoginRequest();
            consumerRole.InitDefaultRDMDirectoryRequest();
            consumerRole.ChannelEventCallback = consumer;
            consumerRole.LoginMsgCallback = consumer;
            consumerRole.DirectoryMsgCallback = consumer;
            consumerRole.DictionaryMsgCallback = consumer;
            consumerRole.DefaultMsgCallback = consumer;
            consumerRole.WatchlistOptions.EnableWatchlist = true;
            consumerRole.WatchlistOptions.ChannelOpenEventCallback = consumer;
            consumerRole.WatchlistOptions.RequestTimeout = 15000;

            /* Create provider. */
            Provider provider = new(providerReactor);
            ProviderRole providerRole = (ProviderRole)provider.ReactorRole;
            providerRole.ChannelEventCallback = provider;
            providerRole.LoginMsgCallback = provider;
            providerRole.DirectoryMsgCallback = provider;
            providerRole.DictionaryMsgCallback = provider;
            providerRole.DefaultMsgCallback = provider;

            /* Connect the consumer and provider. Setup login & directory streams automatically. */
            ConsumerProviderSessionOptions opts = new();
            opts.SetupDefaultLoginStream = true;
            opts.SetupDefaultDirectoryStream = true;

            provider.Bind(opts);

            TestReactor.OpenSession(consumer, provider, opts);

            /* Request item on some unsupported domain. 
             * * (The chosen domain is 0, so if this test ever appears to fail -- it may be because
             * 0 was added to the default service capabilities).*/

            /* Request first item. */
            requestMsg.Clear();
            requestMsg.StreamId = 5;
            requestMsg.MsgClass = MsgClasses.REQUEST;
            requestMsg.DomainType = 0;
            requestMsg.ContainerType = DataTypes.NO_DATA;
            requestMsg.ApplyStreaming();
            requestMsg.ApplyHasQos();
            requestMsg.Qos.Timeliness(QosTimeliness.REALTIME);
            requestMsg.Qos.Rate(QosRates.TICK_BY_TICK);
            requestMsg.ApplyHasWorstQos();
            requestMsg.WorstQos.Timeliness(QosTimeliness.DELAYED);
            requestMsg.WorstQos.Rate(QosRates.JIT_CONFLATED);

            submitOptions.Clear();
            submitOptions.ServiceName = Provider.DefaultService.Info.ServiceName.ToString();
            Assert.True(consumer.Submit((Msg)requestMsg, submitOptions) >= ReactorReturnCode.SUCCESS);

            /* Consumer receives Open/Suspect status. */
            consumerReactor.Dispatch(1);
            evt = consumerReactor.PollEvent();
            Assert.Equal(TestReactorEventType.MSG, evt.EventType);
            msgEvent = (ReactorMsgEvent)evt.ReactorEvent;
            Assert.Equal(MsgClasses.STATUS, msgEvent.Msg.MsgClass);
            IStatusMsg receivedStatusMsg = (IStatusMsg)msgEvent.Msg;
            Assert.Equal(0, receivedStatusMsg.DomainType);
            Assert.True(receivedStatusMsg.CheckHasState());
            Assert.Equal(DataTypes.NO_DATA, receivedStatusMsg.ContainerType);
            Assert.Equal(StreamStates.OPEN, receivedStatusMsg.State.StreamState());
            Assert.Equal(DataStates.SUSPECT, receivedStatusMsg.State.DataState());
            Assert.Equal(5, receivedStatusMsg.StreamId);

            /* Provider receives nothing. */
            providerReactor.Dispatch(0);

            TestReactorComponent.CloseSession(consumer, provider);
            consumerReactor.Close();
            providerReactor.Close();
        }

        [Fact]
        public void WatchlistOneItem_StaticQosTest()
        {
            ReactorSubmitOptions submitOptions = new();
            TestReactorEvent evt;
            ReactorMsgEvent msgEvent;
            Msg msg = new();
            IRequestMsg requestMsg = msg;

            /* Create reactors. */
            TestReactor consumerReactor = new();
            TestReactor providerReactor = new();

            /* Create consumer. */
            Consumer consumer = new(consumerReactor);
            ConsumerRole consumerRole = (ConsumerRole)consumer.ReactorRole;
            consumerRole.InitDefaultRDMLoginRequest();
            consumerRole.InitDefaultRDMDirectoryRequest();
            consumerRole.ChannelEventCallback = consumer;
            consumerRole.LoginMsgCallback = consumer;
            consumerRole.DirectoryMsgCallback = consumer;
            consumerRole.DictionaryMsgCallback = consumer;
            consumerRole.DefaultMsgCallback = consumer;
            consumerRole.WatchlistOptions.EnableWatchlist = true;
            consumerRole.WatchlistOptions.ChannelOpenEventCallback = consumer;
            consumerRole.WatchlistOptions.RequestTimeout = 15000;

            /* Create provider. */
            Provider provider = new(providerReactor);
            ProviderRole providerRole = (ProviderRole)provider.ReactorRole;
            providerRole.ChannelEventCallback = provider;
            providerRole.LoginMsgCallback = provider;
            providerRole.DirectoryMsgCallback = provider;
            providerRole.DictionaryMsgCallback = provider;
            providerRole.DefaultMsgCallback = provider;

            /* Connect the consumer and provider. Setup login & directory streams automatically. */
            ConsumerProviderSessionOptions opts = new();
            opts.SetupDefaultLoginStream = true;
            opts.SetupDefaultDirectoryStream = true;

            provider.Bind(opts);

            TestReactor.OpenSession(consumer, provider, opts);

            /* Request first item. */
            requestMsg.Clear();
            requestMsg.StreamId = 5;
            requestMsg.MsgClass = MsgClasses.REQUEST;
            requestMsg.DomainType = (int)DomainType.MARKET_PRICE;
            requestMsg.ContainerType = DataTypes.NO_DATA;
            requestMsg.ApplyStreaming();
            requestMsg.ApplyHasQos();
            requestMsg.Qos.Timeliness(QosTimeliness.REALTIME);
            requestMsg.Qos.Rate(QosRates.TICK_BY_TICK);
            requestMsg.ApplyHasWorstQos();
            requestMsg.WorstQos.Timeliness(QosTimeliness.DELAYED);
            requestMsg.WorstQos.Rate(QosRates.JIT_CONFLATED);

            submitOptions.Clear();
            submitOptions.ServiceName = Provider.DefaultService.Info.ServiceName.ToString();
            Assert.True(consumer.SubmitAndDispatch((Msg)requestMsg, submitOptions) >= ReactorReturnCode.SUCCESS);

            /* Provider receives request. */
            providerReactor.Dispatch(1);
            evt = providerReactor.PollEvent();
            Assert.Equal(TestReactorEventType.MSG, evt.EventType);
            msgEvent = (ReactorMsgEvent)evt.ReactorEvent;
            Assert.Equal(MsgClasses.REQUEST, msgEvent.Msg.MsgClass);
            IRequestMsg receivedRequestMsg = (IRequestMsg)msgEvent.Msg;
            Assert.True(receivedRequestMsg.CheckStreaming());
            Assert.True(receivedRequestMsg.CheckHasPriority());
            Assert.Equal(1, receivedRequestMsg.Priority.PriorityClass);
            Assert.Equal(1, receivedRequestMsg.Priority.Count);
            Assert.False(receivedRequestMsg.CheckNoRefresh());
            Assert.True(receivedRequestMsg.CheckHasQos());
            Assert.False(receivedRequestMsg.CheckHasWorstQos());
            Assert.Equal(QosTimeliness.REALTIME, receivedRequestMsg.Qos.Timeliness());
            Assert.Equal(QosRates.TICK_BY_TICK, receivedRequestMsg.Qos.Rate());
            int providerStreamId = receivedRequestMsg.StreamId;

            /* Provider sends refresh. */
            IRefreshMsg refreshMsg = new Msg();
            refreshMsg.MsgClass = MsgClasses.REFRESH;
            refreshMsg.DomainType = (int)DomainType.MARKET_PRICE;
            refreshMsg.StreamId = providerStreamId;
            refreshMsg.ContainerType = DataTypes.NO_DATA;
            refreshMsg.ApplyRefreshComplete();
            refreshMsg.ApplySolicited();
            refreshMsg.ApplyClearCache();
            refreshMsg.ApplyHasMsgKey();
            refreshMsg.MsgKey.ApplyHasServiceId();
            refreshMsg.MsgKey.ServiceId = Provider.DefaultService.ServiceId;
            refreshMsg.State.StreamState(StreamStates.OPEN);
            refreshMsg.State.DataState(DataStates.OK);
            refreshMsg.ApplyHasQos();
            refreshMsg.Qos.Timeliness(QosTimeliness.REALTIME);
            refreshMsg.Qos.Rate(QosRates.TICK_BY_TICK);
            Buffer groupId = new Buffer();
            groupId.Data("1234431");
            refreshMsg.GroupId = groupId;

            submitOptions.Clear();
            Assert.True(provider.SubmitAndDispatch((Msg)refreshMsg, submitOptions) >= ReactorReturnCode.SUCCESS);

            /* Provider sends closed. */
            IStatusMsg statusMsg = new Msg();
            statusMsg.MsgClass = MsgClasses.STATUS;
            statusMsg.StreamId = providerStreamId;
            statusMsg.DomainType = (int)DomainType.MARKET_PRICE;
            statusMsg.ContainerType = DataTypes.NO_DATA;
            statusMsg.ApplyHasState();
            statusMsg.State.StreamState(StreamStates.CLOSED);
            statusMsg.State.DataState(DataStates.SUSPECT);

            submitOptions.Clear();
            Assert.True(provider.SubmitAndDispatch((Msg)statusMsg, submitOptions) >= ReactorReturnCode.SUCCESS);

            /* Consumer receives refresh. */
            consumerReactor.Dispatch(2);
            evt = consumerReactor.PollEvent();
            Assert.Equal(TestReactorEventType.MSG, evt.EventType);
            msgEvent = (ReactorMsgEvent)evt.ReactorEvent;
            Assert.Equal(MsgClasses.REFRESH, msgEvent.Msg.MsgClass);
            IRefreshMsg receivedRefreshMsg = (IRefreshMsg)msgEvent.Msg;
            Assert.Equal((int)DomainType.MARKET_PRICE, receivedRefreshMsg.DomainType);
            Assert.Equal(DataTypes.NO_DATA, receivedRefreshMsg.ContainerType);
            Assert.Equal(StreamStates.OPEN, receivedRefreshMsg.State.StreamState());
            Assert.Equal(DataStates.OK, receivedRefreshMsg.State.DataState());
            Assert.Equal(5, receivedRefreshMsg.StreamId);
            Assert.True(receivedRefreshMsg.GroupId.Equals(groupId));

            /* Consumer receives Closed status. */
            evt = consumerReactor.PollEvent();
            Assert.Equal(TestReactorEventType.MSG, evt.EventType);
            msgEvent = (ReactorMsgEvent)evt.ReactorEvent;
            Assert.Equal(MsgClasses.STATUS, msgEvent.Msg.MsgClass);
            IStatusMsg receivedStatusMsg = (IStatusMsg)msgEvent.Msg;
            Assert.Equal((int)DomainType.MARKET_PRICE, receivedStatusMsg.DomainType);
            Assert.Equal(DataTypes.NO_DATA, receivedStatusMsg.ContainerType);
            Assert.True(receivedStatusMsg.CheckHasState());
            Assert.Equal(StreamStates.CLOSED, receivedStatusMsg.State.StreamState());
            Assert.Equal(DataStates.SUSPECT, receivedStatusMsg.State.DataState());
            Assert.Equal(5, receivedStatusMsg.StreamId);

            TestReactorComponent.CloseSession(consumer, provider);
            consumerReactor.Close();
            providerReactor.Close();
        }

        [Fact]
        public void WatchlistOneItem_RedirectToPrivateStreamTest()
        {
            ReactorSubmitOptions submitOptions = new();
            TestReactorEvent evt;
            ReactorMsgEvent msgEvent;
            Msg msg = new();
            IRequestMsg requestMsg = msg;

            /* Create reactors. */
            TestReactor consumerReactor = new();
            TestReactor providerReactor = new();

            /* Create consumer. */
            Consumer consumer = new(consumerReactor);
            ConsumerRole consumerRole = (ConsumerRole)consumer.ReactorRole;
            consumerRole.InitDefaultRDMLoginRequest();
            consumerRole.InitDefaultRDMDirectoryRequest();
            consumerRole.ChannelEventCallback = consumer;
            consumerRole.LoginMsgCallback = consumer;
            consumerRole.DirectoryMsgCallback = consumer;
            consumerRole.DictionaryMsgCallback = consumer;
            consumerRole.DefaultMsgCallback = consumer;
            consumerRole.WatchlistOptions.EnableWatchlist = true;
            consumerRole.WatchlistOptions.ChannelOpenEventCallback = consumer;
            consumerRole.WatchlistOptions.RequestTimeout = 15000;

            /* Create provider. */
            Provider provider = new(providerReactor);
            ProviderRole providerRole = (ProviderRole)provider.ReactorRole;
            providerRole.ChannelEventCallback = provider;
            providerRole.LoginMsgCallback = provider;
            providerRole.DirectoryMsgCallback = provider;
            providerRole.DictionaryMsgCallback = provider;
            providerRole.DefaultMsgCallback = provider;

            /* Connect the consumer and provider. Setup login & directory streams automatically. */
            ConsumerProviderSessionOptions opts = new();
            opts.SetupDefaultLoginStream = true;
            opts.SetupDefaultDirectoryStream = true;

            provider.Bind(opts);

            TestReactor.OpenSession(consumer, provider, opts);

            Buffer itemNameBuf = new Buffer();
            itemNameBuf.Data("RTRSY.O");

            /* Request first item. */
            requestMsg.Clear();
            requestMsg.StreamId = 5;
            requestMsg.MsgClass = MsgClasses.REQUEST;
            requestMsg.DomainType = (int)DomainType.MARKET_PRICE;
            requestMsg.ContainerType = DataTypes.NO_DATA;
            requestMsg.ApplyStreaming();
            requestMsg.ApplyHasQos();
            requestMsg.Qos.Timeliness(QosTimeliness.REALTIME);
            requestMsg.Qos.Rate(QosRates.TICK_BY_TICK);
            requestMsg.MsgKey.ApplyHasName();
            requestMsg.MsgKey.Name = itemNameBuf;

            submitOptions.Clear();
            submitOptions.ServiceName = Provider.DefaultService.Info.ServiceName.ToString();
            Assert.True(consumer.SubmitAndDispatch((Msg)requestMsg, submitOptions) >= ReactorReturnCode.SUCCESS);

            /* Provider receives request. */
            providerReactor.Dispatch(1);
            evt = providerReactor.PollEvent();
            Assert.Equal(TestReactorEventType.MSG, evt.EventType);
            msgEvent = (ReactorMsgEvent)evt.ReactorEvent;
            Assert.Equal(MsgClasses.REQUEST, msgEvent.Msg.MsgClass);
            IRequestMsg receivedRequestMsg = (IRequestMsg)msgEvent.Msg;
            Assert.True(receivedRequestMsg.CheckStreaming());
            Assert.True(receivedRequestMsg.CheckHasPriority());
            Assert.Equal(1, receivedRequestMsg.Priority.PriorityClass);
            Assert.Equal(1, receivedRequestMsg.Priority.Count);
            Assert.False(receivedRequestMsg.CheckNoRefresh());
            Assert.True(receivedRequestMsg.MsgKey.CheckHasServiceId());
            Assert.Equal(Provider.DefaultService.ServiceId, receivedRequestMsg.MsgKey.ServiceId);
            Assert.True(itemNameBuf.Equals(receivedRequestMsg.MsgKey.Name));
            int providerStreamId = receivedRequestMsg.StreamId;

            /* Provider sends redirect. */
            IStatusMsg statusMsg = new Msg();
            statusMsg.MsgClass = MsgClasses.STATUS;
            statusMsg.StreamId = providerStreamId;
            statusMsg.DomainType = (int)DomainType.MARKET_PRICE;
            statusMsg.ContainerType = DataTypes.NO_DATA;
            statusMsg.ApplyPrivateStream();
            statusMsg.ApplyHasState();
            statusMsg.State.StreamState(StreamStates.REDIRECTED);
            statusMsg.State.DataState(DataStates.OK);

            submitOptions.Clear();
            Assert.True(provider.SubmitAndDispatch((Msg)statusMsg, submitOptions) >= ReactorReturnCode.SUCCESS);

            /* Consumer receives redirect, with no key. */
            consumerReactor.Dispatch(1);
            evt = consumerReactor.PollEvent();
            Assert.Equal(TestReactorEventType.MSG, evt.EventType);
            msgEvent = (ReactorMsgEvent)evt.ReactorEvent;
            Assert.Equal(MsgClasses.STATUS, msgEvent.Msg.MsgClass);
            IStatusMsg receivedStatusMsg = (IStatusMsg)msgEvent.Msg;
            Assert.Equal((int)DomainType.MARKET_PRICE, receivedStatusMsg.DomainType);
            Assert.Equal(DataTypes.NO_DATA, receivedStatusMsg.ContainerType);
            Assert.True(receivedStatusMsg.CheckPrivateStream());
            Assert.True(receivedStatusMsg.CheckHasState());
            Assert.Equal(StreamStates.REDIRECTED, receivedStatusMsg.State.StreamState());
            Assert.Equal(DataStates.OK, receivedStatusMsg.State.DataState());
            Assert.Equal(5, receivedStatusMsg.StreamId);

            TestReactorComponent.CloseSession(consumer, provider);
            consumerReactor.Close();
            providerReactor.Close();
        }

        [Fact]
        public void WatchlistOneItem_RedirectToPrivateStream_WithKeyTest()
        {
            ReactorSubmitOptions submitOptions = new();
            TestReactorEvent evt;
            ReactorMsgEvent msgEvent;
            Msg msg = new();
            IRequestMsg requestMsg = msg;

            Buffer oldItemName = new Buffer();
            oldItemName.Data("RTRSY.O");

            Buffer newItemName = new Buffer();
            newItemName.Data("TRI.N");

            MsgKey newMsgKey;

            /* Create reactors. */
            TestReactor consumerReactor = new();
            TestReactor providerReactor = new();

            /* Create consumer. */
            Consumer consumer = new(consumerReactor);
            ConsumerRole consumerRole = (ConsumerRole)consumer.ReactorRole;
            consumerRole.InitDefaultRDMLoginRequest();
            consumerRole.InitDefaultRDMDirectoryRequest();
            consumerRole.ChannelEventCallback = consumer;
            consumerRole.LoginMsgCallback = consumer;
            consumerRole.DirectoryMsgCallback = consumer;
            consumerRole.DictionaryMsgCallback = consumer;
            consumerRole.DefaultMsgCallback = consumer;
            consumerRole.WatchlistOptions.EnableWatchlist = true;
            consumerRole.WatchlistOptions.ChannelOpenEventCallback = consumer;
            consumerRole.WatchlistOptions.RequestTimeout = 15000;

            /* Create provider. */
            Provider provider = new(providerReactor);
            ProviderRole providerRole = (ProviderRole)provider.ReactorRole;
            providerRole.ChannelEventCallback = provider;
            providerRole.LoginMsgCallback = provider;
            providerRole.DirectoryMsgCallback = provider;
            providerRole.DictionaryMsgCallback = provider;
            providerRole.DefaultMsgCallback = provider;

            /* Connect the consumer and provider. Setup login & directory streams automatically. */
            ConsumerProviderSessionOptions opts = new();
            opts.SetupDefaultLoginStream = true;
            opts.SetupDefaultDirectoryStream = true;

            provider.Bind(opts);

            TestReactor.OpenSession(consumer, provider, opts);

            /* Request first item. */
            requestMsg.Clear();
            requestMsg.StreamId = 5;
            requestMsg.MsgClass = MsgClasses.REQUEST;
            requestMsg.DomainType = (int)DomainType.MARKET_PRICE;
            requestMsg.ContainerType = DataTypes.NO_DATA;
            requestMsg.ApplyStreaming();
            requestMsg.ApplyHasQos();
            requestMsg.Qos.Timeliness(QosTimeliness.REALTIME);
            requestMsg.Qos.Rate(QosRates.TICK_BY_TICK);
            requestMsg.MsgKey.ApplyHasName();
            requestMsg.MsgKey.Name = oldItemName;

            submitOptions.Clear();
            submitOptions.ServiceName = Provider.DefaultService.Info.ServiceName.ToString();
            Assert.True(consumer.SubmitAndDispatch((Msg)requestMsg, submitOptions) >= ReactorReturnCode.SUCCESS);

            /* Provider receives request. */
            providerReactor.Dispatch(1);
            evt = providerReactor.PollEvent();
            Assert.Equal(TestReactorEventType.MSG, evt.EventType);
            msgEvent = (ReactorMsgEvent)evt.ReactorEvent;
            Assert.Equal(MsgClasses.REQUEST, msgEvent.Msg.MsgClass);
            IRequestMsg receivedRequestMsg = (IRequestMsg)msgEvent.Msg;
            Assert.True(receivedRequestMsg.CheckStreaming());
            Assert.True(receivedRequestMsg.CheckHasPriority());
            Assert.Equal(1, receivedRequestMsg.Priority.PriorityClass);
            Assert.Equal(1, receivedRequestMsg.Priority.Count);
            Assert.False(receivedRequestMsg.CheckNoRefresh());
            Assert.True(receivedRequestMsg.MsgKey.CheckHasServiceId());
            Assert.Equal(Provider.DefaultService.ServiceId, receivedRequestMsg.MsgKey.ServiceId);
            Assert.True(oldItemName.Equals(receivedRequestMsg.MsgKey.Name));
            int providerStreamId = receivedRequestMsg.StreamId;

            /* Provider sends redirect. */
            IStatusMsg statusMsg = new Msg();
            statusMsg.MsgClass = MsgClasses.STATUS;
            statusMsg.StreamId = providerStreamId;
            statusMsg.DomainType = (int)DomainType.MARKET_PRICE;
            statusMsg.ContainerType = DataTypes.NO_DATA;
            statusMsg.ApplyPrivateStream();
            statusMsg.ApplyHasState();
            statusMsg.State.StreamState(StreamStates.REDIRECTED);
            statusMsg.State.DataState(DataStates.OK);
            statusMsg.ApplyHasMsgKey();
            statusMsg.MsgKey.ApplyHasName();
            statusMsg.MsgKey.ApplyHasServiceId();
            statusMsg.MsgKey.Name = newItemName;
            statusMsg.MsgKey.ServiceId = Provider.DefaultService.ServiceId;

            submitOptions.Clear();
            Assert.True(provider.SubmitAndDispatch((Msg)statusMsg, submitOptions) >= ReactorReturnCode.SUCCESS);

            /* Consumer receives Open/Suspect status, with new key info. */
            consumerReactor.Dispatch(1);
            evt = consumerReactor.PollEvent();
            Assert.Equal(TestReactorEventType.MSG, evt.EventType);
            msgEvent = (ReactorMsgEvent)evt.ReactorEvent;
            Assert.Equal(MsgClasses.STATUS, msgEvent.Msg.MsgClass);
            IStatusMsg receivedStatusMsg = (IStatusMsg)msgEvent.Msg;
            Assert.Equal((int)DomainType.MARKET_PRICE, receivedStatusMsg.DomainType);
            Assert.Equal(DataTypes.NO_DATA, receivedStatusMsg.ContainerType);
            Assert.True(receivedStatusMsg.CheckPrivateStream());
            Assert.True(receivedStatusMsg.CheckHasState());
            Assert.Equal(StreamStates.REDIRECTED, receivedStatusMsg.State.StreamState());
            Assert.Equal(DataStates.OK, receivedStatusMsg.State.DataState());
            Assert.Equal(5, receivedStatusMsg.StreamId);
            newMsgKey = receivedStatusMsg.MsgKey;
            Assert.True(newMsgKey.CheckHasServiceId());
            Assert.True(newMsgKey.CheckHasName());
            Assert.Equal(Provider.DefaultService.ServiceId, newMsgKey.ServiceId);
            Assert.True(newItemName.Equals(newMsgKey.Name));

            TestReactorComponent.CloseSession(consumer, provider);
            consumerReactor.Close();
            providerReactor.Close();
        }

        [Fact]
        public void WatchlistSymbolListTest_BigListTest()
        {
            ReactorSubmitOptions submitOptions = new();
            TestReactorEvent evt;
            ReactorMsgEvent msgEvent;
            Msg msg = new();
            IRequestMsg requestMsg = msg;

            /* Create reactors. */
            TestReactor consumerReactor = new();
            TestReactor providerReactor = new();

            /* Create consumer. */
            Consumer consumer = new(consumerReactor);
            ConsumerRole consumerRole = (ConsumerRole)consumer.ReactorRole;
            consumerRole.InitDefaultRDMLoginRequest();
            consumerRole.InitDefaultRDMDirectoryRequest();
            consumerRole.ChannelEventCallback = consumer;
            consumerRole.LoginMsgCallback = consumer;
            consumerRole.DirectoryMsgCallback = consumer;
            consumerRole.DictionaryMsgCallback = consumer;
            consumerRole.DefaultMsgCallback = consumer;
            consumerRole.WatchlistOptions.EnableWatchlist = true;
            consumerRole.WatchlistOptions.ChannelOpenEventCallback = consumer;
            consumerRole.WatchlistOptions.RequestTimeout = 15000;

            /* Create provider. */
            Provider provider = new(providerReactor);
            ProviderRole providerRole = (ProviderRole)provider.ReactorRole;
            providerRole.ChannelEventCallback = provider;
            providerRole.LoginMsgCallback = provider;
            providerRole.DirectoryMsgCallback = provider;
            providerRole.DictionaryMsgCallback = provider;
            providerRole.DefaultMsgCallback = provider;

            /* Connect the consumer and provider. Setup login & directory streams automatically. */
            ConsumerProviderSessionOptions opts = new();
            opts.SetupDefaultLoginStream = true;
            opts.SetupDefaultDirectoryStream = true;

            provider.Bind(opts);

            SymbolAction[] symbolList = new SymbolAction[1000];

            Buffer symbolListName = new Buffer();
            symbolListName.Data("NUMB3RS");

            /* Fill symbol list with names. */
            for (int i = 0; i < symbolList.Length; ++i)
            {
                symbolList[i] = new SymbolAction();
                symbolList[i].ItemName.Data($"{i}");
                symbolList[i].Action = MapEntryActions.ADD;
            }

            TestReactor.OpenSession(consumer, provider, opts);

            /* Set the open window so we can better control this loop. */
            DirectoryUpdate directoryUpdate = new DirectoryUpdate();
            directoryUpdate.Clear();
            directoryUpdate.StreamId = provider.DefaultSessionDirectoryStreamId;
            directoryUpdate.Filter = Directory.ServiceFilterFlags.INFO | Directory.ServiceFilterFlags.STATE |
                Directory.ServiceFilterFlags.LOAD | Directory.ServiceFilterFlags.LINK;
            Service service = new Service();
            service.ServiceId = Provider.DefaultService.ServiceId;
            service.HasLoad = true;
            service.Load.HasOpenWindow = true;
            service.Load.OpenWindow = 50;
            service.Action = MapEntryActions.UPDATE;
            directoryUpdate.ServiceList.Add(service);

            submitOptions.Clear();
            Assert.True(provider.SubmitAndDispatch(directoryUpdate, submitOptions) >= ReactorReturnCode.SUCCESS);

            consumerReactor.Dispatch(0);

            int[] consStreamIds = { 5, 6 };

            /* Request symbol list. */
            requestMsg.Clear();
            requestMsg.StreamId = consStreamIds[0];
            requestMsg.MsgClass = MsgClasses.REQUEST;
            requestMsg.DomainType = (int)DomainType.SYMBOL_LIST;
            requestMsg.ContainerType = DataTypes.ELEMENT_LIST;
            requestMsg.ApplyStreaming();
            requestMsg.ApplyHasQos();
            requestMsg.Qos.Timeliness(QosTimeliness.REALTIME);
            requestMsg.Qos.Rate(QosRates.TICK_BY_TICK);
            requestMsg.MsgKey.ApplyHasName();
            requestMsg.MsgKey.Name = symbolListName;

            Buffer payload = new();
            payload.Data(new ByteBuffer(1024));

            EncodeIterator encIter = new EncodeIterator();
            encIter.Clear();
            encIter.SetBufferAndRWFVersion(payload, consumer.ReactorChannel.MajorVersion,
                    consumer.ReactorChannel.MajorVersion);

            ElementList elementList = new();
            elementList.Clear();
            elementList.ApplyHasStandardData();

            CodecReturnCode ret = elementList.EncodeInit(encIter, null, 0);
            Assert.True(ret >= CodecReturnCode.SUCCESS);

            WatchlistItemDomainsTest.EncodeSymbolListBehaviorsElement(encIter, SymbolList.SymbolListDataStreamRequestFlags.SYMBOL_LIST_DATA_STREAMS);
            ret = elementList.EncodeComplete(encIter, true);
            Assert.True(ret >= CodecReturnCode.SUCCESS);
            requestMsg.EncodedDataBody = payload;

            submitOptions.Clear();
            submitOptions.ServiceName = Provider.DefaultService.Info.ServiceName.ToString();
            Assert.True(consumer.SubmitAndDispatch((Msg)requestMsg, submitOptions) >= ReactorReturnCode.SUCCESS);

            /* Provider receives request. */
            providerReactor.Dispatch(1);
            evt = providerReactor.PollEvent();
            Assert.Equal(TestReactorEventType.MSG, evt.EventType);
            msgEvent = (ReactorMsgEvent)evt.ReactorEvent;
            Assert.Equal(MsgClasses.REQUEST, msgEvent.Msg.MsgClass);
            IRequestMsg receivedRequestMsg = (IRequestMsg)msgEvent.Msg;
            Assert.Equal((int)DomainType.SYMBOL_LIST, receivedRequestMsg.DomainType);
            Assert.True(receivedRequestMsg.CheckStreaming());
            Assert.True(receivedRequestMsg.CheckHasPriority());
            Assert.Equal(1, receivedRequestMsg.Priority.PriorityClass);
            Assert.Equal(1, receivedRequestMsg.Priority.Count);
            Assert.False(receivedRequestMsg.CheckNoRefresh());
            Assert.True(receivedRequestMsg.CheckHasQos());
            Assert.Equal(QosTimeliness.REALTIME, receivedRequestMsg.Qos.Timeliness());
            Assert.Equal(QosRates.TICK_BY_TICK, receivedRequestMsg.Qos.Rate());
            Assert.True(receivedRequestMsg.MsgKey.CheckHasName());
            Assert.True(symbolListName.Equals(receivedRequestMsg.MsgKey.Name));
            int providerStreamId = receivedRequestMsg.StreamId;

            /* Provider sends refresh. */
            IRefreshMsg refreshMsg = new Msg();
            refreshMsg.MsgClass = MsgClasses.REFRESH;
            refreshMsg.DomainType = (int)DomainType.SYMBOL_LIST;
            refreshMsg.StreamId = providerStreamId;
            refreshMsg.ContainerType = DataTypes.MAP;
            refreshMsg.ApplyRefreshComplete();
            refreshMsg.ApplySolicited();
            refreshMsg.ApplyClearCache();
            refreshMsg.ApplyHasMsgKey();
            refreshMsg.State.StreamState(StreamStates.OPEN);
            refreshMsg.State.DataState(DataStates.OK);
            refreshMsg.ApplyHasQos();
            refreshMsg.Qos.Timeliness(QosTimeliness.REALTIME);
            refreshMsg.Qos.Rate(QosRates.TICK_BY_TICK);
            refreshMsg.MsgKey.ApplyHasServiceId();
            refreshMsg.MsgKey.ServiceId = Provider.DefaultService.ServiceId;

            Buffer dataBodyBuf = new Buffer();
            dataBodyBuf.Data(new ByteBuffer(8192));

            WatchlistItemDomainsTest.EncodeSymbolListDataBody(provider.ReactorChannel, dataBodyBuf, symbolList);
            refreshMsg.EncodedDataBody = dataBodyBuf;

            submitOptions.Clear();
            Assert.True(provider.SubmitAndDispatch((Msg)refreshMsg, submitOptions) >= ReactorReturnCode.SUCCESS);

            /* Consumer receives refresh. */
            consumerReactor.Dispatch(1);
            evt = consumerReactor.PollEvent();
            Assert.Equal(TestReactorEventType.MSG, evt.EventType);
            msgEvent = (ReactorMsgEvent)evt.ReactorEvent;
            Assert.Equal(MsgClasses.REFRESH, msgEvent.Msg.MsgClass);
            IRefreshMsg receivedRefreshMsg = (IRefreshMsg)msgEvent.Msg;
            Assert.Equal((int)DomainType.SYMBOL_LIST, receivedRefreshMsg.DomainType);
            Assert.Equal(DataTypes.MAP, receivedRefreshMsg.ContainerType);
            Assert.Equal(StreamStates.OPEN, receivedRefreshMsg.State.StreamState());
            Assert.Equal(DataStates.OK, receivedRefreshMsg.State.DataState());
            Assert.Equal(consStreamIds[0], receivedRefreshMsg.StreamId);

            /* Exchange requests/refreshes. */
            int providerRequests = 0;
            int consumerRefreshes = 0;
            while (consumerRefreshes < symbolList.Length)
            {
                providerReactor.Dispatch(-1);

                do
                {
                    int providerItemStream;

                    /* Provider receives request. */
                    evt = providerReactor.PollEvent();
                    Assert.Equal(TestReactorEventType.MSG, evt.EventType);
                    msgEvent = (ReactorMsgEvent)evt.ReactorEvent;
                    Assert.Equal(MsgClasses.REQUEST, msgEvent.Msg.MsgClass);
                    receivedRequestMsg = (IRequestMsg)msgEvent.Msg;
                    Assert.True(receivedRequestMsg.CheckStreaming());
                    Assert.True(receivedRequestMsg.CheckHasPriority());
                    Assert.Equal((int)DomainType.MARKET_PRICE, receivedRequestMsg.DomainType);
                    Assert.Equal(1, receivedRequestMsg.Priority.PriorityClass);
                    Assert.Equal(1, receivedRequestMsg.Priority.Count);
                    Assert.False(receivedRequestMsg.CheckNoRefresh());
                    Assert.True(receivedRequestMsg.CheckHasQos());
                    Assert.Equal(QosTimeliness.REALTIME, receivedRequestMsg.Qos.Timeliness());
                    Assert.Equal(QosRates.TICK_BY_TICK, receivedRequestMsg.Qos.Rate());
                    Assert.True(receivedRequestMsg.MsgKey.CheckHasName());
                    Assert.True(symbolList[providerRequests].ItemName.Equals(receivedRequestMsg.MsgKey.Name));
                    providerItemStream = receivedRequestMsg.StreamId;

                    /* Provider sends refresh. */
                    refreshMsg.Clear();
                    refreshMsg.MsgClass = MsgClasses.REFRESH;
                    refreshMsg.DomainType = (int)DomainType.MARKET_PRICE;
                    refreshMsg.StreamId = providerItemStream;
                    refreshMsg.ContainerType = DataTypes.NO_DATA;
                    refreshMsg.ApplyRefreshComplete();
                    refreshMsg.ApplySolicited();
                    refreshMsg.ApplyClearCache();
                    refreshMsg.ApplyHasMsgKey();
                    refreshMsg.State.StreamState(StreamStates.OPEN);
                    refreshMsg.State.DataState(DataStates.OK);
                    refreshMsg.ApplyHasQos();
                    refreshMsg.Qos.Timeliness(QosTimeliness.REALTIME);
                    refreshMsg.Qos.Rate(QosRates.TICK_BY_TICK);
                    refreshMsg.MsgKey.ApplyHasServiceId();
                    refreshMsg.MsgKey.ServiceId = Provider.DefaultService.ServiceId;
                    refreshMsg.MsgKey.ApplyHasName();
                    refreshMsg.MsgKey.Name = symbolList[providerRequests].ItemName;

                    submitOptions.Clear();
                    Assert.True(provider.Submit((Msg)refreshMsg, submitOptions) >= ReactorReturnCode.SUCCESS);

                    ++providerRequests;

                } while (providerReactor.m_EventQueue.Count > 0);


                providerReactor.Dispatch(0);

                consumerReactor.Dispatch(-1);

                do
                {
                    /* Consumer receives refresh. */
                    evt = consumerReactor.PollEvent();
                    Assert.Equal(TestReactorEventType.MSG, evt.EventType);
                    msgEvent = (ReactorMsgEvent)evt.ReactorEvent;
                    Assert.Equal(MsgClasses.REFRESH, msgEvent.Msg.MsgClass);
                    receivedRefreshMsg = (IRefreshMsg)msgEvent.Msg;
                    Assert.True(receivedRefreshMsg.StreamId < 0);
                    Assert.Equal((int)DomainType.MARKET_PRICE, receivedRefreshMsg.DomainType);
                    Assert.Equal(DataTypes.NO_DATA, receivedRefreshMsg.ContainerType);
                    Assert.Equal(StreamStates.OPEN, receivedRefreshMsg.State.StreamState());
                    Assert.Equal(DataStates.OK, receivedRefreshMsg.State.DataState());
                    Assert.True(receivedRefreshMsg.CheckHasMsgKey());
                    Assert.True(symbolList[consumerRefreshes].ItemName.Equals(receivedRefreshMsg.MsgKey.Name));
                    ++consumerRefreshes;
                } while (consumerReactor.m_EventQueue.Count > 0);

            }

            Assert.Equal(symbolList.Length, providerRequests);
            Assert.Equal(symbolList.Length, consumerRefreshes);

            TestReactorComponent.CloseSession(consumer, provider);
            consumerReactor.Close();
            providerReactor.Close();
        }

        [Fact]
        public void WatchlistSymbolListTest_TwoSymbols_FlagsFromMsgBufferTest()
        {
            ReactorSubmitOptions submitOptions = new();
            TestReactorEvent evt;
            ReactorMsgEvent msgEvent;
            Msg msg = new();
            IRequestMsg requestMsg = msg;

            /* Create reactors. */
            TestReactor consumerReactor = new();
            TestReactor providerReactor = new();

            /* Create consumer. */
            Consumer consumer = new(consumerReactor);
            ConsumerRole consumerRole = (ConsumerRole)consumer.ReactorRole;
            consumerRole.InitDefaultRDMLoginRequest();
            consumerRole.InitDefaultRDMDirectoryRequest();
            consumerRole.ChannelEventCallback = consumer;
            consumerRole.LoginMsgCallback = consumer;
            consumerRole.DirectoryMsgCallback = consumer;
            consumerRole.DictionaryMsgCallback = consumer;
            consumerRole.DefaultMsgCallback = consumer;
            consumerRole.WatchlistOptions.EnableWatchlist = true;
            consumerRole.WatchlistOptions.ChannelOpenEventCallback = consumer;
            consumerRole.WatchlistOptions.RequestTimeout = 15000;

            /* Create provider. */
            Provider provider = new(providerReactor);
            ProviderRole providerRole = (ProviderRole)provider.ReactorRole;
            providerRole.ChannelEventCallback = provider;
            providerRole.LoginMsgCallback = provider;
            providerRole.DirectoryMsgCallback = provider;
            providerRole.DictionaryMsgCallback = provider;
            providerRole.DefaultMsgCallback = provider;

            /* Connect the consumer and provider. Setup login & directory streams automatically. */
            ConsumerProviderSessionOptions opts = new();
            opts.SetupDefaultLoginStream = true;
            opts.SetupDefaultDirectoryStream = true;

            provider.Bind(opts);

            SymbolAction[] symbolList = new SymbolAction[2];
            Buffer symbolListName = new Buffer();
            symbolListName.Data("symbolist name");

            List<int> view1List = new List<int>() { 2, 6, 7 };

            /* Consumer requests a symbol list that contains two items and specifies it wants data streams.  
             * * These items should therefore be automatically requested by the watchlist. In this test,
             * * closing the symbol list does NOT automatically close the items. */

            TestReactor.OpenSession(consumer, provider, opts);

            /* Request symbol list. */
            int[] consStreamIds = { 5, 6 };

            /* Request symbol list. */
            requestMsg.Clear();
            requestMsg.StreamId = consStreamIds[0];
            requestMsg.MsgClass = MsgClasses.REQUEST;
            requestMsg.DomainType = (int)DomainType.SYMBOL_LIST;
            requestMsg.ContainerType = DataTypes.ELEMENT_LIST;
            requestMsg.ApplyStreaming();
            requestMsg.ApplyHasQos();
            requestMsg.Qos.Timeliness(QosTimeliness.REALTIME);
            requestMsg.Qos.Rate(QosRates.TICK_BY_TICK);
            requestMsg.MsgKey.ApplyHasName();
            requestMsg.MsgKey.Name = symbolListName;

            Buffer payload = new();
            payload.Data(new ByteBuffer(1024));

            EncodeIterator encIter = new EncodeIterator();
            encIter.Clear();
            encIter.SetBufferAndRWFVersion(payload, consumer.ReactorChannel.MajorVersion,
                    consumer.ReactorChannel.MajorVersion);

            ElementList elementList = new();
            elementList.Clear();
            elementList.ApplyHasStandardData();

            CodecReturnCode ret = elementList.EncodeInit(encIter, null, 0);
            Assert.True(ret >= CodecReturnCode.SUCCESS);

            WatchlistItemDomainsTest.EncodeSymbolListBehaviorsElement(encIter, SymbolList.SymbolListDataStreamRequestFlags.SYMBOL_LIST_DATA_STREAMS);
            ret = elementList.EncodeComplete(encIter, true);
            Assert.True(ret >= CodecReturnCode.SUCCESS);
            requestMsg.EncodedDataBody = payload;

            submitOptions.Clear();
            submitOptions.ServiceName = Provider.DefaultService.Info.ServiceName.ToString();
            Assert.True(consumer.SubmitAndDispatch((Msg)requestMsg, submitOptions) >= ReactorReturnCode.SUCCESS);

            /* Provider receives request. */
            providerReactor.Dispatch(1);
            evt = providerReactor.PollEvent();
            Assert.Equal(TestReactorEventType.MSG, evt.EventType);
            msgEvent = (ReactorMsgEvent)evt.ReactorEvent;
            Assert.Equal(MsgClasses.REQUEST, msgEvent.Msg.MsgClass);
            IRequestMsg receivedRequestMsg = (IRequestMsg)msgEvent.Msg;
            Assert.Equal((int)DomainType.SYMBOL_LIST, receivedRequestMsg.DomainType);
            Assert.True(receivedRequestMsg.CheckStreaming());
            Assert.True(receivedRequestMsg.CheckHasPriority());
            Assert.Equal(1, receivedRequestMsg.Priority.PriorityClass);
            Assert.Equal(1, receivedRequestMsg.Priority.Count);
            Assert.False(receivedRequestMsg.CheckNoRefresh());
            Assert.True(receivedRequestMsg.CheckHasQos());
            Assert.Equal(QosTimeliness.REALTIME, receivedRequestMsg.Qos.Timeliness());
            Assert.Equal(QosRates.TICK_BY_TICK, receivedRequestMsg.Qos.Rate());
            Assert.True(receivedRequestMsg.MsgKey.CheckHasName());
            Assert.True(symbolListName.Equals(receivedRequestMsg.MsgKey.Name));
            int providerStreamId = receivedRequestMsg.StreamId;

            /* Provider sends refresh. */
            IRefreshMsg refreshMsg = new Msg();
            refreshMsg.MsgClass = MsgClasses.REFRESH;
            refreshMsg.DomainType = (int)DomainType.SYMBOL_LIST;
            refreshMsg.StreamId = providerStreamId;
            refreshMsg.ContainerType = DataTypes.MAP;
            refreshMsg.ApplyRefreshComplete();
            refreshMsg.ApplySolicited();
            refreshMsg.ApplyClearCache();
            refreshMsg.ApplyHasMsgKey();
            refreshMsg.State.StreamState(StreamStates.OPEN);
            refreshMsg.State.DataState(DataStates.OK);
            refreshMsg.ApplyHasQos();
            refreshMsg.Qos.Timeliness(QosTimeliness.REALTIME);
            refreshMsg.Qos.Rate(QosRates.TICK_BY_TICK);
            refreshMsg.MsgKey.ApplyHasServiceId();
            refreshMsg.MsgKey.ServiceId = Provider.DefaultService.ServiceId;

            Buffer dataBodyBuf = new Buffer();
            dataBodyBuf.Data(new ByteBuffer(1024));

            symbolList[0] = new SymbolAction();
            symbolList[0].ItemName.Data("ITEM1");
            symbolList[0].Action = MapEntryActions.ADD;

            symbolList[1] = new SymbolAction();
            symbolList[1].ItemName.Data("ITEM2");
            symbolList[1].Action = MapEntryActions.ADD;

            WatchlistItemDomainsTest.EncodeSymbolListDataBody(provider.ReactorChannel, dataBodyBuf, symbolList);
            refreshMsg.EncodedDataBody = dataBodyBuf;

            submitOptions.Clear();
            Assert.True(provider.Submit((Msg)refreshMsg, submitOptions) >= ReactorReturnCode.SUCCESS);

            /* Consumer receives refresh. */
            consumerReactor.Dispatch(1);
            evt = consumerReactor.PollEvent();
            Assert.Equal(TestReactorEventType.MSG, evt.EventType);
            msgEvent = (ReactorMsgEvent)evt.ReactorEvent;
            Assert.Equal(MsgClasses.REFRESH, msgEvent.Msg.MsgClass);
            IRefreshMsg receivedRefreshMsg = (IRefreshMsg)msgEvent.Msg;
            Assert.Equal((int)DomainType.SYMBOL_LIST, receivedRefreshMsg.DomainType);
            Assert.Equal(DataTypes.MAP, receivedRefreshMsg.ContainerType);
            Assert.Equal(StreamStates.OPEN, receivedRefreshMsg.State.StreamState());
            Assert.Equal(DataStates.OK, receivedRefreshMsg.State.DataState());
            Assert.Equal(consStreamIds[0], receivedRefreshMsg.StreamId);

            /* Provider receives requests for items(should come in order of list). */
            providerReactor.Dispatch(2);
            evt = providerReactor.PollEvent();
            Assert.Equal(TestReactorEventType.MSG, evt.EventType);
            msgEvent = (ReactorMsgEvent)evt.ReactorEvent;
            Assert.Equal(MsgClasses.REQUEST, msgEvent.Msg.MsgClass);
            receivedRequestMsg = (IRequestMsg)msgEvent.Msg;
            Assert.True(receivedRequestMsg.CheckStreaming());
            Assert.True(receivedRequestMsg.CheckHasPriority());
            Assert.Equal((int)DomainType.MARKET_PRICE, receivedRequestMsg.DomainType);
            Assert.Equal(1, receivedRequestMsg.Priority.PriorityClass);
            Assert.Equal(1, receivedRequestMsg.Priority.Count);
            Assert.False(receivedRequestMsg.CheckNoRefresh());
            Assert.True(receivedRequestMsg.CheckHasQos());
            Assert.Equal(QosTimeliness.REALTIME, receivedRequestMsg.Qos.Timeliness());
            Assert.Equal(QosRates.TICK_BY_TICK, receivedRequestMsg.Qos.Rate());
            Assert.True(receivedRequestMsg.MsgKey.CheckHasName());
            Assert.True(symbolList[0].ItemName.Equals(receivedRequestMsg.MsgKey.Name));
            int providerItem1Stream = receivedRequestMsg.StreamId;

            evt = providerReactor.PollEvent();
            Assert.Equal(TestReactorEventType.MSG, evt.EventType);
            msgEvent = (ReactorMsgEvent)evt.ReactorEvent;
            Assert.Equal(MsgClasses.REQUEST, msgEvent.Msg.MsgClass);
            receivedRequestMsg = (IRequestMsg)msgEvent.Msg;
            Assert.True(receivedRequestMsg.CheckStreaming());
            Assert.True(receivedRequestMsg.CheckHasPriority());
            Assert.Equal((int)DomainType.MARKET_PRICE, receivedRequestMsg.DomainType);
            Assert.Equal(1, receivedRequestMsg.Priority.PriorityClass);
            Assert.Equal(1, receivedRequestMsg.Priority.Count);
            Assert.False(receivedRequestMsg.CheckNoRefresh());
            Assert.True(receivedRequestMsg.CheckHasQos());
            Assert.Equal(QosTimeliness.REALTIME, receivedRequestMsg.Qos.Timeliness());
            Assert.Equal(QosRates.TICK_BY_TICK, receivedRequestMsg.Qos.Rate());
            Assert.True(receivedRequestMsg.MsgKey.CheckHasName());
            Assert.True(symbolList[1].ItemName.Equals(receivedRequestMsg.MsgKey.Name));
            int providerItem2Stream = receivedRequestMsg.StreamId;

            /* Provider sends refresh. */
            refreshMsg.Clear();
            refreshMsg.MsgClass = MsgClasses.REFRESH;
            refreshMsg.DomainType = (int)DomainType.MARKET_PRICE;
            refreshMsg.StreamId = providerItem1Stream;
            refreshMsg.ContainerType = DataTypes.NO_DATA;
            refreshMsg.ApplyRefreshComplete();
            refreshMsg.ApplySolicited();
            refreshMsg.ApplyClearCache();
            refreshMsg.ApplyHasMsgKey();
            refreshMsg.State.StreamState(StreamStates.OPEN);
            refreshMsg.State.DataState(DataStates.OK);
            refreshMsg.ApplyHasQos();
            refreshMsg.Qos.Timeliness(QosTimeliness.REALTIME);
            refreshMsg.Qos.Rate(QosRates.TICK_BY_TICK);
            refreshMsg.MsgKey.ApplyHasServiceId();
            refreshMsg.MsgKey.ServiceId = Provider.DefaultService.ServiceId;
            refreshMsg.MsgKey.ApplyHasName();
            refreshMsg.MsgKey.Name = symbolList[0].ItemName;

            submitOptions.Clear();
            Assert.True(provider.SubmitAndDispatch((Msg)refreshMsg, submitOptions) >= ReactorReturnCode.SUCCESS);

            refreshMsg.StreamId = providerItem2Stream;
            refreshMsg.MsgKey.Name = symbolList[1].ItemName;

            submitOptions.Clear();
            Assert.True(provider.SubmitAndDispatch((Msg)refreshMsg, submitOptions) >= ReactorReturnCode.SUCCESS);

            /* Consumer receives refreshes. */
            consumerReactor.Dispatch(2, new TimeSpan(0, 0, 1));
            evt = consumerReactor.PollEvent();
            Assert.Equal(TestReactorEventType.MSG, evt.EventType);
            msgEvent = (ReactorMsgEvent)evt.ReactorEvent;
            Assert.Equal(MsgClasses.REFRESH, msgEvent.Msg.MsgClass);
            receivedRefreshMsg = (IRefreshMsg)msgEvent.Msg;
            Assert.True(receivedRefreshMsg.StreamId < 0);
            Assert.Equal((int)DomainType.MARKET_PRICE, receivedRefreshMsg.DomainType);
            Assert.Equal(DataTypes.NO_DATA, receivedRefreshMsg.ContainerType);
            Assert.Equal(StreamStates.OPEN, receivedRefreshMsg.State.StreamState());
            Assert.Equal(DataStates.OK, receivedRefreshMsg.State.DataState());
            Assert.True(receivedRefreshMsg.CheckHasMsgKey());
            Assert.True(symbolList[0].ItemName.Equals(receivedRefreshMsg.MsgKey.Name));
            int consumerItem1Stream = receivedRefreshMsg.StreamId;

            evt = consumerReactor.PollEvent();
            Assert.Equal(TestReactorEventType.MSG, evt.EventType);
            msgEvent = (ReactorMsgEvent)evt.ReactorEvent;
            Assert.Equal(MsgClasses.REFRESH, msgEvent.Msg.MsgClass);
            receivedRefreshMsg = (IRefreshMsg)msgEvent.Msg;
            Assert.True(receivedRefreshMsg.StreamId < 0);
            Assert.Equal((int)DomainType.MARKET_PRICE, receivedRefreshMsg.DomainType);
            Assert.Equal(DataTypes.NO_DATA, receivedRefreshMsg.ContainerType);
            Assert.Equal(StreamStates.OPEN, receivedRefreshMsg.State.StreamState());
            Assert.Equal(DataStates.OK, receivedRefreshMsg.State.DataState());
            Assert.True(receivedRefreshMsg.CheckHasMsgKey());
            Assert.True(symbolList[1].ItemName.Equals(receivedRefreshMsg.MsgKey.Name));
            int consumerItem2Stream = receivedRefreshMsg.StreamId;

            /* Consumer adds a view to first request. */
            requestMsg.Clear();
            requestMsg.MsgClass = MsgClasses.REQUEST;
            requestMsg.StreamId = consumerItem1Stream;
            requestMsg.DomainType = (int)DomainType.MARKET_PRICE;
            requestMsg.ApplyStreaming();
            requestMsg.ApplyHasQos();
            requestMsg.Qos.Timeliness(QosTimeliness.REALTIME);
            requestMsg.Qos.Rate(QosRates.TICK_BY_TICK);
            requestMsg.ApplyHasView();
            WatchlistItemDomainsTest.EncodeViewFieldIdList(consumer.ReactorChannel, view1List, requestMsg);

            submitOptions.Clear();
            submitOptions.ServiceName = Provider.DefaultService.Info.ServiceName.ToString();
            Assert.True(consumer.SubmitAndDispatch((Msg)requestMsg, submitOptions) >= ReactorReturnCode.SUCCESS);

            /* Provider receives request with view. */
            providerReactor.Dispatch(1);
            evt = providerReactor.PollEvent();
            Assert.Equal(TestReactorEventType.MSG, evt.EventType);
            msgEvent = (ReactorMsgEvent)evt.ReactorEvent;
            Assert.Equal(MsgClasses.REQUEST, msgEvent.Msg.MsgClass);
            receivedRequestMsg = (IRequestMsg)msgEvent.Msg;
            Assert.True(receivedRequestMsg.CheckStreaming());
            Assert.False(receivedRequestMsg.CheckHasPriority());
            Assert.False(receivedRequestMsg.CheckNoRefresh());
            Assert.Equal(providerItem1Stream, receivedRequestMsg.StreamId);
            Assert.True(receivedRequestMsg.CheckHasView());
            Assert.True(WatchlistItemDomainsTest.CheckHasCorrectView(provider, receivedRequestMsg, view1List));

            /* Consumer closes symbol list. */
            ICloseMsg closeMsg = new Msg();
            closeMsg.MsgClass = MsgClasses.CLOSE;
            closeMsg.StreamId = consStreamIds[0];
            closeMsg.DomainType = (int)DomainType.SYMBOL_LIST;

            submitOptions.Clear();
            Assert.True(consumer.SubmitAndDispatch((Msg)closeMsg, submitOptions) >= ReactorReturnCode.SUCCESS);

            /* Provider receives close. */
            providerReactor.Dispatch(1);
            evt = providerReactor.PollEvent();
            Assert.Equal(TestReactorEventType.MSG, evt.EventType);
            msgEvent = (ReactorMsgEvent)evt.ReactorEvent;
            Assert.Equal(MsgClasses.CLOSE, msgEvent.Msg.MsgClass);
            ICloseMsg receivedCloseMsg = (ICloseMsg)msgEvent.Msg;
            Assert.Equal((int)DomainType.SYMBOL_LIST, receivedCloseMsg.DomainType);
            Assert.Equal(providerStreamId, receivedCloseMsg.StreamId);

            /* Providers receives nothing else. */
            providerReactor.Dispatch(0);

            /* Consumer closes item 1. */
            closeMsg.Clear();
            closeMsg.MsgClass = MsgClasses.CLOSE;
            closeMsg.StreamId = consumerItem1Stream;
            closeMsg.DomainType = (int)DomainType.MARKET_PRICE;

            submitOptions.Clear();
            Assert.True(consumer.SubmitAndDispatch((Msg)closeMsg, submitOptions) >= ReactorReturnCode.SUCCESS);

            /* Consumer closes item 2. */
            closeMsg.Clear();
            closeMsg.MsgClass = MsgClasses.CLOSE;
            closeMsg.StreamId = consumerItem2Stream;
            closeMsg.DomainType = (int)DomainType.MARKET_PRICE;

            submitOptions.Clear();
            Assert.True(consumer.SubmitAndDispatch((Msg)closeMsg, submitOptions) >= ReactorReturnCode.SUCCESS);

            /* Provider receives close of items. */
            providerReactor.Dispatch(2);
            evt = providerReactor.PollEvent();
            Assert.Equal(TestReactorEventType.MSG, evt.EventType);
            msgEvent = (ReactorMsgEvent)evt.ReactorEvent;
            Assert.Equal(MsgClasses.CLOSE, msgEvent.Msg.MsgClass);
            receivedCloseMsg = (ICloseMsg)msgEvent.Msg;
            Assert.Equal(providerItem1Stream, receivedCloseMsg.StreamId);
            Assert.Equal((int)DomainType.MARKET_PRICE, receivedCloseMsg.DomainType);

            evt = providerReactor.PollEvent();
            Assert.Equal(TestReactorEventType.MSG, evt.EventType);
            msgEvent = (ReactorMsgEvent)evt.ReactorEvent;
            Assert.Equal(MsgClasses.CLOSE, msgEvent.Msg.MsgClass);
            receivedCloseMsg = (ICloseMsg)msgEvent.Msg;
            Assert.Equal(providerItem2Stream, receivedCloseMsg.StreamId);
            Assert.Equal((int)DomainType.MARKET_PRICE, receivedCloseMsg.DomainType);

            TestReactorComponent.CloseSession(consumer, provider);
            consumerReactor.Close();
            providerReactor.Close();
        }

        [Fact]
        public void WatchlistLoginCredentialsUpdateTest()
        {
            ReactorSubmitOptions submitOptions = new();
            TestReactorEvent evt;

            TestReactor consumerReactor = new();
            TestReactor providerReactor = new();

            Consumer consumer = new(consumerReactor);
            ConsumerRole consumerRole = (ConsumerRole)consumer.ReactorRole;
            consumerRole.ChannelEventCallback = consumer;
            consumerRole.LoginMsgCallback = consumer;
            consumerRole.DirectoryMsgCallback = consumer;
            consumerRole.DictionaryMsgCallback = consumer;
            consumerRole.DefaultMsgCallback = consumer;
            consumerRole.WatchlistOptions.EnableWatchlist = true;
            consumerRole.WatchlistOptions.ChannelOpenEventCallback = consumer;

            Provider provider = new(providerReactor);
            ProviderRole providerRole = (ProviderRole)provider.ReactorRole;
            providerRole.ChannelEventCallback = provider;
            providerRole.LoginMsgCallback = provider;
            providerRole.DirectoryMsgCallback = provider;
            providerRole.DictionaryMsgCallback = provider;
            providerRole.DefaultMsgCallback = provider;

            ConsumerProviderSessionOptions opts = new();
            opts.SetupDefaultLoginStream = false;
            opts.SetupDefaultDirectoryStream = false;

            provider.Bind(opts);

            ReactorChannelEvent channelEvent;
            RDMLoginMsgEvent loginMsgEvent;
            RDMDirectoryMsgEvent directoryMsgEvent;

            string userToken = "RedPill";
            string userToken2 = "BluePill";
            string userToken3 = "Cookie";
            string userToken4 = "Spoon";

            /* Apart from the use of the USER_TOKEN username type, this test begins much the same
             * way as the standard framework.  The actual test of updating it is further below. */

            consumer.TestReactor.Connect(opts, consumer, provider.ServerPort);
            evt = consumer.TestReactor.PollEvent();
            Assert.Equal(TestReactorEventType.CHANNEL_EVENT, evt.EventType);
            channelEvent = (ReactorChannelEvent)evt.ReactorEvent;
            Assert.Equal(ReactorChannelEventType.CHANNEL_OPENED, channelEvent.EventType);

            provider.TestReactor.Accept(opts, provider);
            provider.TestReactor.Dispatch(2);

            evt = provider.TestReactor.PollEvent();
            Assert.Equal(TestReactorEventType.CHANNEL_EVENT, evt.EventType);
            channelEvent = (ReactorChannelEvent)evt.ReactorEvent;
            Assert.Equal(ReactorChannelEventType.CHANNEL_UP, channelEvent.EventType);

            evt = provider.TestReactor.PollEvent();
            Assert.Equal(TestReactorEventType.CHANNEL_EVENT, evt.EventType);
            channelEvent = (ReactorChannelEvent)evt.ReactorEvent;
            Assert.Equal(ReactorChannelEventType.CHANNEL_READY, channelEvent.EventType);

            consumer.TestReactor.Dispatch(2);

            evt = consumer.TestReactor.PollEvent();
            Assert.Equal(TestReactorEventType.CHANNEL_EVENT, evt.EventType);
            channelEvent = (ReactorChannelEvent)evt.ReactorEvent;
            Assert.Equal(ReactorChannelEventType.CHANNEL_UP, channelEvent.EventType);

            evt = consumer.TestReactor.PollEvent();
            Assert.Equal(TestReactorEventType.CHANNEL_EVENT, evt.EventType);
            channelEvent = (ReactorChannelEvent)evt.ReactorEvent;
            Assert.Equal(ReactorChannelEventType.CHANNEL_READY, channelEvent.EventType);

            provider.TestReactor.Dispatch(0);

            LoginRequest loginRequest = new();
            loginRequest.InitDefaultRequest(1);
            loginRequest.UserNameType = Login.UserIdTypes.TOKEN;
            loginRequest.UserName.Data(userToken);

            submitOptions.Clear();
            Assert.True(consumer.SubmitAndDispatch(loginRequest, submitOptions) >= ReactorReturnCode.SUCCESS);

            /* Provider receives login request. */
            provider.TestReactor.Dispatch(1);
            evt = provider.TestReactor.PollEvent();
            Assert.Equal(TestReactorEventType.LOGIN_MSG, evt.EventType);
            loginMsgEvent = (RDMLoginMsgEvent)evt.ReactorEvent;
            Assert.Equal(LoginMsgType.REQUEST, loginMsgEvent.LoginMsg.LoginMsgType);
            LoginRequest receivedLoginRequest = loginMsgEvent.LoginMsg.LoginRequest;
            Assert.True(receivedLoginRequest.HasUserNameType);
            Assert.Equal(Login.UserIdTypes.TOKEN, receivedLoginRequest.UserNameType);
            Assert.Equal(receivedLoginRequest.UserName.ToString(), userToken);
            int providerLoginStreamId = loginMsgEvent.LoginMsg.StreamId;

            LoginRefresh loginRefresh = new();

            loginRefresh.Clear();
            loginRefresh.Solicited = true;
            loginRefresh.HasUserName = true;
            loginRefresh.UserName = receivedLoginRequest.UserName;
            loginRefresh.StreamId = providerLoginStreamId;
            loginRefresh.HasUserNameType = true;
            loginRefresh.UserNameType = Login.UserIdTypes.TOKEN;
            loginRefresh.HasFeatures = true;
            loginRefresh.SupportedFeatures.HasSupportOptimizedPauseResume = true;
            loginRefresh.SupportedFeatures.SupportOptimizedPauseResume = 1;
            loginRefresh.State.StreamState(StreamStates.OPEN);
            loginRefresh.State.DataState(DataStates.OK);
            loginRefresh.State.Code(StateCodes.NONE);
            loginRefresh.State.Text().Data("Login OK");

            submitOptions.Clear();
            Assert.True(provider.SubmitAndDispatch(loginRefresh, submitOptions) >= ReactorReturnCode.SUCCESS);

            consumer.TestReactor.Dispatch(1);

            /* Consumer receives login response. */
            evt = consumer.TestReactor.PollEvent();
            Assert.Equal(TestReactorEventType.LOGIN_MSG, evt.EventType);
            loginMsgEvent = (RDMLoginMsgEvent)evt.ReactorEvent;
            Assert.Equal(LoginMsgType.REFRESH, loginMsgEvent.LoginMsg.LoginMsgType);
            Assert.Equal(1, loginMsgEvent.LoginMsg.StreamId);
            LoginRefresh receivedLoginRefresh = loginMsgEvent.LoginMsg.LoginRefresh;
            Assert.True(receivedLoginRefresh.HasUserName);
            Assert.True(receivedLoginRefresh.HasUserNameType);
            Assert.Equal(Login.UserIdTypes.TOKEN, receivedLoginRefresh.UserNameType);
            Assert.Equal(userToken, receivedLoginRefresh.UserName.ToString());

            DirectoryRequest directoryRequest = new DirectoryRequest();
            directoryRequest.StreamId = 2;
            directoryRequest.Filter = ALL_FILTERS;

            submitOptions.Clear();
            Assert.True(consumer.Submit(directoryRequest, submitOptions) >= ReactorReturnCode.SUCCESS);

            /* Provider receives directory request. */
            provider.TestReactor.Dispatch(1);
            evt = provider.TestReactor.PollEvent();
            Assert.Equal(TestReactorEventType.DIRECTORY_MSG, evt.EventType);
            directoryMsgEvent = (RDMDirectoryMsgEvent)evt.ReactorEvent;
            Assert.Equal(DirectoryMsgType.REQUEST, directoryMsgEvent.DirectoryMsg.DirectoryMsgType);

            DirectoryRefresh directoryRefresh = new();

            directoryRefresh.Clear();
            directoryRefresh.StreamId = directoryRequest.StreamId;
            directoryRefresh.Filter = directoryRequest.Filter;
            directoryRefresh.Solicited = true;
            directoryRefresh.ClearCache = true;
            directoryRefresh.State.StreamState(StreamStates.OPEN);
            directoryRefresh.State.DataState(DataStates.OK);
            directoryRefresh.State.Code(StateCodes.NONE);
            directoryRefresh.State.Text().Data("Source Directory Refresh Complete");

            Service service = new();
            Provider.DefaultService.Copy(service);

            directoryRefresh.ServiceList.Add(service);

            submitOptions.Clear();
            Assert.True(provider.SubmitAndDispatch(directoryRefresh, submitOptions) >= ReactorReturnCode.SUCCESS);

            consumer.TestReactor.Dispatch(1);
            evt = consumer.TestReactor.PollEvent();
            Assert.Equal(TestReactorEventType.DIRECTORY_MSG, evt.EventType);
            directoryMsgEvent = (RDMDirectoryMsgEvent)evt.ReactorEvent;
            Assert.Equal(DirectoryMsgType.REFRESH, directoryMsgEvent.DirectoryMsg.DirectoryMsgType);

            /* Consumer should receive no more messages. */
            consumerReactor.Dispatch(0);

            /*** Test of token changes starts here. ***/

            /* Consumer updates token. */
            loginRequest.Clear();
            loginRequest.InitDefaultRequest(1);
            loginRequest.UserNameType = Login.UserIdTypes.TOKEN;
            loginRequest.UserName.Data(userToken2);

            submitOptions.Clear();
            Assert.True(consumer.SubmitAndDispatch(loginRequest, submitOptions) >= ReactorReturnCode.SUCCESS);

            /* Consumer updates token again. */
            loginRequest.Clear();
            loginRequest.InitDefaultRequest(1);
            loginRequest.UserNameType = Login.UserIdTypes.TOKEN;
            loginRequest.UserName.Data(userToken3);

            submitOptions.Clear();
            Assert.True(consumer.SubmitAndDispatch(loginRequest, submitOptions) >= ReactorReturnCode.SUCCESS);

            /* Consumer updates token one more time. */
            loginRequest.Clear();
            loginRequest.InitDefaultRequest(1);
            loginRequest.UserNameType = Login.UserIdTypes.TOKEN;
            loginRequest.UserName.Data(userToken4);

            submitOptions.Clear();
            Assert.True(consumer.SubmitAndDispatch(loginRequest, submitOptions) >= ReactorReturnCode.SUCCESS);

            /* Provider receives first token update. */
            provider.TestReactor.Dispatch(1);
            evt = provider.TestReactor.PollEvent();
            Assert.Equal(TestReactorEventType.LOGIN_MSG, evt.EventType);
            loginMsgEvent = (RDMLoginMsgEvent)evt.ReactorEvent;
            Assert.Equal(LoginMsgType.REQUEST, loginMsgEvent.LoginMsg.LoginMsgType);
            Assert.Equal(providerLoginStreamId, loginMsgEvent.LoginMsg.StreamId);
            receivedLoginRequest = loginMsgEvent.LoginMsg.LoginRequest;
            Assert.True(receivedLoginRequest.HasUserNameType);
            Assert.Equal(Login.UserIdTypes.TOKEN, receivedLoginRequest.UserNameType);
            Assert.Equal(receivedLoginRequest.UserName.ToString(), userToken2);

            /* Provider responds to first token update. */
            loginRefresh.Clear();
            loginRefresh.Solicited = true;
            loginRefresh.HasUserName = true;
            loginRefresh.UserName = receivedLoginRequest.UserName;
            loginRefresh.StreamId = providerLoginStreamId;
            loginRefresh.HasUserNameType = true;
            loginRefresh.UserNameType = Login.UserIdTypes.TOKEN;
            loginRefresh.HasFeatures = true;
            loginRefresh.SupportedFeatures.HasSupportOptimizedPauseResume = true;
            loginRefresh.SupportedFeatures.SupportOptimizedPauseResume = 1;

            submitOptions.Clear();
            Assert.True(provider.SubmitAndDispatch(loginRefresh, submitOptions) >= ReactorReturnCode.SUCCESS);

            /* Consumer receives refresh with new token. */
            consumerReactor.Dispatch(1);
            evt = consumer.TestReactor.PollEvent();
            Assert.Equal(TestReactorEventType.LOGIN_MSG, evt.EventType);
            loginMsgEvent = (RDMLoginMsgEvent)evt.ReactorEvent;
            Assert.Equal(LoginMsgType.REFRESH, loginMsgEvent.LoginMsg.LoginMsgType);
            Assert.Equal(1, loginMsgEvent.LoginMsg.StreamId);
            receivedLoginRefresh = loginMsgEvent.LoginMsg.LoginRefresh;
            Assert.True(receivedLoginRefresh.HasUserName);
            Assert.True(receivedLoginRefresh.HasUserNameType);
            Assert.Equal(Login.UserIdTypes.TOKEN, receivedLoginRefresh.UserNameType);
            Assert.Equal(userToken2, receivedLoginRefresh.UserName.ToString());

            /* Provider does not receive second token update */

            /* Provider receives final token update. */
            provider.TestReactor.Dispatch(1);
            evt = provider.TestReactor.PollEvent();
            Assert.Equal(TestReactorEventType.LOGIN_MSG, evt.EventType);
            loginMsgEvent = (RDMLoginMsgEvent)evt.ReactorEvent;
            Assert.Equal(LoginMsgType.REQUEST, loginMsgEvent.LoginMsg.LoginMsgType);
            Assert.Equal(providerLoginStreamId, loginMsgEvent.LoginMsg.StreamId);
            receivedLoginRequest = loginMsgEvent.LoginMsg.LoginRequest;
            Assert.True(receivedLoginRequest.HasUserNameType);
            Assert.Equal(Login.UserIdTypes.TOKEN, receivedLoginRequest.UserNameType);
            Assert.Equal(receivedLoginRequest.UserName.ToString(), userToken4);

            /* Provider responds to final token update. */
            loginRefresh.Clear();
            loginRefresh.Solicited = true;
            loginRefresh.HasUserName = true;
            loginRefresh.UserName = receivedLoginRequest.UserName;
            loginRefresh.StreamId = providerLoginStreamId;
            loginRefresh.HasUserNameType = true;
            loginRefresh.UserNameType = Login.UserIdTypes.TOKEN;
            loginRefresh.HasFeatures = true;
            loginRefresh.SupportedFeatures.HasSupportOptimizedPauseResume = true;
            loginRefresh.SupportedFeatures.SupportOptimizedPauseResume = 1;

            submitOptions.Clear();
            Assert.True(provider.SubmitAndDispatch(loginRefresh, submitOptions) >= ReactorReturnCode.SUCCESS);

            /* Consumer receives refresh with final token. */
            consumerReactor.Dispatch(1);
            evt = consumer.TestReactor.PollEvent();
            Assert.Equal(TestReactorEventType.LOGIN_MSG, evt.EventType);
            loginMsgEvent = (RDMLoginMsgEvent)evt.ReactorEvent;
            Assert.Equal(LoginMsgType.REFRESH, loginMsgEvent.LoginMsg.LoginMsgType);
            Assert.Equal(1, loginMsgEvent.LoginMsg.StreamId);
            receivedLoginRefresh = loginMsgEvent.LoginMsg.LoginRefresh;
            Assert.True(receivedLoginRefresh.HasUserName);
            Assert.True(receivedLoginRefresh.HasUserNameType);
            Assert.Equal(Login.UserIdTypes.TOKEN, receivedLoginRefresh.UserNameType);
            Assert.Equal(userToken4, receivedLoginRefresh.UserName.ToString());

            TestReactorComponent.CloseSession(consumer, provider);
            consumerReactor.Close();
            providerReactor.Close();
        }

        [Fact]
        public void WatchlistLoginAuthenticationUpdateTest()
        {
            ReactorSubmitOptions submitOptions = new();
            TestReactorEvent evt;

            TestReactor consumerReactor = new();
            TestReactor providerReactor = new();

            Consumer consumer = new(consumerReactor);
            ConsumerRole consumerRole = (ConsumerRole)consumer.ReactorRole;
            consumerRole.ChannelEventCallback = consumer;
            consumerRole.LoginMsgCallback = consumer;
            consumerRole.DirectoryMsgCallback = consumer;
            consumerRole.DictionaryMsgCallback = consumer;
            consumerRole.DefaultMsgCallback = consumer;
            consumerRole.WatchlistOptions.EnableWatchlist = true;
            consumerRole.WatchlistOptions.ChannelOpenEventCallback = consumer;

            Provider provider = new(providerReactor);
            ProviderRole providerRole = (ProviderRole)provider.ReactorRole;
            providerRole.ChannelEventCallback = provider;
            providerRole.LoginMsgCallback = provider;
            providerRole.DirectoryMsgCallback = provider;
            providerRole.DictionaryMsgCallback = provider;
            providerRole.DefaultMsgCallback = provider;

            ConsumerProviderSessionOptions opts = new();
            opts.SetupDefaultLoginStream = false;
            opts.SetupDefaultDirectoryStream = false;

            provider.Bind(opts);

            ReactorChannelEvent channelEvent;
            RDMLoginMsgEvent loginMsgEvent;
            RDMDirectoryMsgEvent directoryMsgEvent;

            string userToken = "RedPill";
            string userToken2 = "BluePill";
            string userToken3 = "Cookie";
            string userToken4 = "Spoon";

            string extResp1 = "Candy";
            string extResp4 = "Steak";

            long TTReissue = 15000000;

            consumer.TestReactor.Connect(opts, consumer, provider.ServerPort);
            evt = consumer.TestReactor.PollEvent();
            Assert.Equal(TestReactorEventType.CHANNEL_EVENT, evt.EventType);
            channelEvent = (ReactorChannelEvent)evt.ReactorEvent;
            Assert.Equal(ReactorChannelEventType.CHANNEL_OPENED, channelEvent.EventType);

            provider.TestReactor.Accept(opts, provider);
            provider.TestReactor.Dispatch(2);

            evt = provider.TestReactor.PollEvent();
            Assert.Equal(TestReactorEventType.CHANNEL_EVENT, evt.EventType);
            channelEvent = (ReactorChannelEvent)evt.ReactorEvent;
            Assert.Equal(ReactorChannelEventType.CHANNEL_UP, channelEvent.EventType);

            evt = provider.TestReactor.PollEvent();
            Assert.Equal(TestReactorEventType.CHANNEL_EVENT, evt.EventType);
            channelEvent = (ReactorChannelEvent)evt.ReactorEvent;
            Assert.Equal(ReactorChannelEventType.CHANNEL_READY, channelEvent.EventType);

            consumer.TestReactor.Dispatch(2);

            evt = consumer.TestReactor.PollEvent();
            Assert.Equal(TestReactorEventType.CHANNEL_EVENT, evt.EventType);
            channelEvent = (ReactorChannelEvent)evt.ReactorEvent;
            Assert.Equal(ReactorChannelEventType.CHANNEL_UP, channelEvent.EventType);

            evt = consumer.TestReactor.PollEvent();
            Assert.Equal(TestReactorEventType.CHANNEL_EVENT, evt.EventType);
            channelEvent = (ReactorChannelEvent)evt.ReactorEvent;
            Assert.Equal(ReactorChannelEventType.CHANNEL_READY, channelEvent.EventType);

            provider.TestReactor.Dispatch(0);

            /* Consumer submits login. */
            LoginRequest loginRequest = new();
            loginRequest.InitDefaultRequest(1);
            loginRequest.UserNameType = Login.UserIdTypes.AUTHN_TOKEN;
            loginRequest.UserName.Data(userToken);

            submitOptions.Clear();
            Assert.True(consumer.SubmitAndDispatch(loginRequest, submitOptions) >= ReactorReturnCode.SUCCESS);

            /* Provider receives login request. */
            provider.TestReactor.Dispatch(1);
            evt = provider.TestReactor.PollEvent();
            Assert.Equal(TestReactorEventType.LOGIN_MSG, evt.EventType);
            loginMsgEvent = (RDMLoginMsgEvent)evt.ReactorEvent;
            Assert.Equal(LoginMsgType.REQUEST, loginMsgEvent.LoginMsg.LoginMsgType);
            LoginRequest receivedLoginRequest = loginMsgEvent.LoginMsg.LoginRequest;
            Assert.True(receivedLoginRequest.HasUserNameType);
            Assert.Equal(Login.UserIdTypes.AUTHN_TOKEN, receivedLoginRequest.UserNameType);
            Assert.Equal(receivedLoginRequest.UserName.ToString(), userToken);
            int providerLoginStreamId = loginMsgEvent.LoginMsg.StreamId;

            /* Provider sends login response. */
            LoginRefresh loginRefresh = new();
            loginRefresh.Clear();
            loginRefresh.Solicited = true;
            loginRefresh.HasAuthenticationExtendedResp = true;
            loginRefresh.AuthenticationExtendedResp.Data(extResp1);
            loginRefresh.HasAuthenicationTTReissue = true;
            loginRefresh.AuthenticationTTReissue = TTReissue;
            loginRefresh.StreamId = providerLoginStreamId;
            loginRefresh.HasUserNameType = true;
            loginRefresh.UserNameType = Login.UserIdTypes.AUTHN_TOKEN;
            loginRefresh.HasFeatures = true;
            loginRefresh.SupportedFeatures.HasSupportOptimizedPauseResume = true;
            loginRefresh.SupportedFeatures.SupportOptimizedPauseResume = 1;

            submitOptions.Clear();
            Assert.True(provider.SubmitAndDispatch(loginRefresh, submitOptions) >= ReactorReturnCode.SUCCESS);

            consumer.TestReactor.Dispatch(1);

            /* Consumer receives login response. */
            evt = consumer.TestReactor.PollEvent();
            Assert.Equal(TestReactorEventType.LOGIN_MSG, evt.EventType);
            loginMsgEvent = (RDMLoginMsgEvent)evt.ReactorEvent;
            Assert.Equal(LoginMsgType.REFRESH, loginMsgEvent.LoginMsg.LoginMsgType);
            Assert.Equal(1, loginMsgEvent.LoginMsg.StreamId);
            LoginRefresh receivedLoginRefresh = loginMsgEvent.LoginMsg.LoginRefresh;
            Assert.True(receivedLoginRefresh.HasUserNameType);
            Assert.Equal(Login.UserIdTypes.AUTHN_TOKEN, receivedLoginRefresh.UserNameType);
            Assert.True(receivedLoginRefresh.HasAuthenticationExtendedResp);
            Assert.Equal(extResp1, receivedLoginRefresh.AuthenticationExtendedResp.ToString());
            Assert.True(receivedLoginRefresh.HasAuthenicationTTReissue);
            Assert.Equal(TTReissue, receivedLoginRefresh.AuthenticationTTReissue);

            DirectoryRequest directoryRequest = new DirectoryRequest();
            directoryRequest.StreamId = 2;
            directoryRequest.Filter = ALL_FILTERS;

            submitOptions.Clear();
            Assert.True(consumer.Submit(directoryRequest, submitOptions) >= ReactorReturnCode.SUCCESS);

            /* Provider receives directory request. */
            provider.TestReactor.Dispatch(1);
            evt = provider.TestReactor.PollEvent();
            Assert.Equal(TestReactorEventType.DIRECTORY_MSG, evt.EventType);
            directoryMsgEvent = (RDMDirectoryMsgEvent)evt.ReactorEvent;
            Assert.Equal(DirectoryMsgType.REQUEST, directoryMsgEvent.DirectoryMsg.DirectoryMsgType);

            DirectoryRefresh directoryRefresh = new();

            directoryRefresh.Clear();
            directoryRefresh.StreamId = directoryRequest.StreamId;
            directoryRefresh.Filter = directoryRequest.Filter;
            directoryRefresh.Solicited = true;
            directoryRefresh.ClearCache = true;
            directoryRefresh.State.StreamState(StreamStates.OPEN);
            directoryRefresh.State.DataState(DataStates.OK);
            directoryRefresh.State.Code(StateCodes.NONE);
            directoryRefresh.State.Text().Data("Source Directory Refresh Complete");

            Service service = new();
            Provider.DefaultService.Copy(service);

            directoryRefresh.ServiceList.Add(service);

            submitOptions.Clear();
            Assert.True(provider.SubmitAndDispatch(directoryRefresh, submitOptions) >= ReactorReturnCode.SUCCESS);

            consumer.TestReactor.Dispatch(1);
            evt = consumer.TestReactor.PollEvent();
            Assert.Equal(TestReactorEventType.DIRECTORY_MSG, evt.EventType);
            directoryMsgEvent = (RDMDirectoryMsgEvent)evt.ReactorEvent;
            Assert.Equal(DirectoryMsgType.REFRESH, directoryMsgEvent.DirectoryMsg.DirectoryMsgType);

            /* Consumer should receive no more messages. */
            consumerReactor.Dispatch(0);

            /*** Test of token changes starts here. ***/

            /* Consumer updates token. */
            loginRequest.Clear();
            loginRequest.InitDefaultRequest(1);
            loginRequest.UserNameType = Login.UserIdTypes.AUTHN_TOKEN;
            loginRequest.UserName.Data(userToken2);

            submitOptions.Clear();
            Assert.True(consumer.SubmitAndDispatch(loginRequest, submitOptions) >= ReactorReturnCode.SUCCESS);

            /* Consumer updates token again. */
            loginRequest.Clear();
            loginRequest.InitDefaultRequest(1);
            loginRequest.UserNameType = Login.UserIdTypes.AUTHN_TOKEN;
            loginRequest.UserName.Data(userToken3);

            submitOptions.Clear();
            Assert.True(consumer.SubmitAndDispatch(loginRequest, submitOptions) >= ReactorReturnCode.SUCCESS);

            /* Consumer updates token one more time. */
            loginRequest.Clear();
            loginRequest.InitDefaultRequest(1);
            loginRequest.UserNameType = Login.UserIdTypes.AUTHN_TOKEN;
            loginRequest.UserName.Data(userToken4);

            submitOptions.Clear();
            Assert.True(consumer.SubmitAndDispatch(loginRequest, submitOptions) >= ReactorReturnCode.SUCCESS);

            /* Provider receives first token update. */
            provider.TestReactor.Dispatch(1);
            evt = provider.TestReactor.PollEvent();
            Assert.Equal(TestReactorEventType.LOGIN_MSG, evt.EventType);
            loginMsgEvent = (RDMLoginMsgEvent)evt.ReactorEvent;
            Assert.Equal(LoginMsgType.REQUEST, loginMsgEvent.LoginMsg.LoginMsgType);
            Assert.Equal(providerLoginStreamId, loginMsgEvent.LoginMsg.StreamId);
            receivedLoginRequest = loginMsgEvent.LoginMsg.LoginRequest;
            Assert.True(receivedLoginRequest.HasUserNameType);
            Assert.Equal(Login.UserIdTypes.AUTHN_TOKEN, receivedLoginRequest.UserNameType);
            Assert.Equal(receivedLoginRequest.UserName.ToString(), userToken2);

            /* Provider responds to first token update. */
            loginRefresh.Clear();
            loginRefresh.Solicited = true;
            loginRefresh.StreamId = providerLoginStreamId;
            loginRefresh.HasUserNameType = true;
            loginRefresh.UserNameType = Login.UserIdTypes.AUTHN_TOKEN;
            loginRefresh.HasFeatures = true;
            loginRefresh.SupportedFeatures.HasSupportOptimizedPauseResume = true;
            loginRefresh.SupportedFeatures.SupportOptimizedPauseResume = 1;
            loginRefresh.HasAuthenicationTTReissue = true;
            loginRefresh.AuthenticationTTReissue = TTReissue;

            submitOptions.Clear();
            Assert.True(provider.SubmitAndDispatch(loginRefresh, submitOptions) >= ReactorReturnCode.SUCCESS);

            /* Consumer receives refresh with new token. */
            consumerReactor.Dispatch(1);
            evt = consumer.TestReactor.PollEvent();
            Assert.Equal(TestReactorEventType.LOGIN_MSG, evt.EventType);
            loginMsgEvent = (RDMLoginMsgEvent)evt.ReactorEvent;
            Assert.Equal(LoginMsgType.REFRESH, loginMsgEvent.LoginMsg.LoginMsgType);
            Assert.Equal(1, loginMsgEvent.LoginMsg.StreamId);
            receivedLoginRefresh = loginMsgEvent.LoginMsg.LoginRefresh;
            Assert.True(receivedLoginRefresh.HasUserNameType);
            Assert.Equal(Login.UserIdTypes.AUTHN_TOKEN, receivedLoginRefresh.UserNameType);
            Assert.True(receivedLoginRefresh.HasAuthenicationTTReissue);
            Assert.True(receivedLoginRefresh.AuthenticationTTReissue == TTReissue);

            /* Provider does not receive second token update */

            /* Provider receives final token update. */
            provider.TestReactor.Dispatch(1);
            evt = provider.TestReactor.PollEvent();
            Assert.Equal(TestReactorEventType.LOGIN_MSG, evt.EventType);
            loginMsgEvent = (RDMLoginMsgEvent)evt.ReactorEvent;
            Assert.Equal(LoginMsgType.REQUEST, loginMsgEvent.LoginMsg.LoginMsgType);
            Assert.Equal(providerLoginStreamId, loginMsgEvent.LoginMsg.StreamId);
            receivedLoginRequest = loginMsgEvent.LoginMsg.LoginRequest;
            Assert.True(receivedLoginRequest.HasUserNameType);
            Assert.Equal(Login.UserIdTypes.AUTHN_TOKEN, receivedLoginRequest.UserNameType);
            Assert.Equal(receivedLoginRequest.UserName.ToString(), userToken4);

            /* Provider responds to final token update. */
            loginRefresh.Clear();
            loginRefresh.Solicited = true;
            loginRefresh.StreamId = providerLoginStreamId;
            loginRefresh.HasUserNameType = true;
            loginRefresh.UserNameType = Login.UserIdTypes.AUTHN_TOKEN;
            loginRefresh.HasFeatures = true;
            loginRefresh.SupportedFeatures.HasSupportOptimizedPauseResume = true;
            loginRefresh.SupportedFeatures.SupportOptimizedPauseResume = 1;
            loginRefresh.HasAuthenticationExtendedResp = true;
            loginRefresh.AuthenticationExtendedResp.Data(extResp4);

            submitOptions.Clear();
            Assert.True(provider.SubmitAndDispatch(loginRefresh, submitOptions) >= ReactorReturnCode.SUCCESS);

            /* Consumer receives refresh with final token. */
            consumerReactor.Dispatch(1);
            evt = consumer.TestReactor.PollEvent();
            Assert.Equal(TestReactorEventType.LOGIN_MSG, evt.EventType);
            loginMsgEvent = (RDMLoginMsgEvent)evt.ReactorEvent;
            Assert.Equal(LoginMsgType.REFRESH, loginMsgEvent.LoginMsg.LoginMsgType);
            Assert.Equal(1, loginMsgEvent.LoginMsg.StreamId);
            receivedLoginRefresh = loginMsgEvent.LoginMsg.LoginRefresh;
            Assert.True(receivedLoginRefresh.HasUserNameType);
            Assert.Equal(Login.UserIdTypes.AUTHN_TOKEN, receivedLoginRefresh.UserNameType);
            Assert.True(receivedLoginRefresh.HasAuthenticationExtendedResp);
            Assert.Equal(extResp4, receivedLoginRefresh.AuthenticationExtendedResp.ToString());

            TestReactorComponent.CloseSession(consumer, provider);
            consumerReactor.Close();
            providerReactor.Close();
        }

        class SendStreamingAndSnapshotRequestsWithViewsConsumer : Consumer
        {
            public SendStreamingAndSnapshotRequestsWithViewsConsumer(TestReactor testReactor) : base(testReactor)
            {
            }

            public override ReactorCallbackReturnCode ReactorChannelEventCallback(ReactorChannelEvent evt)
            {
                ReactorSubmitOptions submitOptions = new ReactorSubmitOptions();
                IRequestMsg requestMsg = new Msg();

                if (evt.EventType == ReactorChannelEventType.CHANNEL_OPENED)
                {
                    base.ReactorChannelEventCallback(evt);

                    List<int> view1List = new List<int> { 25, 32 };
                    List<int> view2List = new List<int> { 6, 22 };
                    List<int> combindView = new List<int> { 6, 22, 25, 32 };

                    int[] consStreamIds = { 5, 6 };

                    /* Request first streaming item. */
                    requestMsg.Clear();
                    requestMsg.StreamId = consStreamIds[0];
                    requestMsg.MsgClass = MsgClasses.REQUEST;
                    requestMsg.DomainType = (int)DomainType.MARKET_PRICE;
                    requestMsg.ContainerType = DataTypes.NO_DATA;
                    requestMsg.ApplyHasQos();
                    requestMsg.ApplyStreaming();
                    requestMsg.Qos.Timeliness(QosTimeliness.REALTIME);
                    requestMsg.Qos.Rate(QosRates.TICK_BY_TICK);
                    requestMsg.ApplyHasView();
                    WatchlistItemDomainsTest.EncodeViewFieldIdList(ReactorChannel, view1List, requestMsg);

                    submitOptions.Clear();
                    submitOptions.ServiceName = Provider.DefaultService.Info.ServiceName.ToString();
                    Assert.True(Submit((Msg)requestMsg, submitOptions) >= ReactorReturnCode.SUCCESS);

                    /* Request second snapshot item. */
                    requestMsg.Clear();
                    requestMsg.StreamId = consStreamIds[1];
                    requestMsg.MsgClass = MsgClasses.REQUEST;
                    requestMsg.DomainType = (int)DomainType.MARKET_PRICE;
                    requestMsg.ContainerType = DataTypes.NO_DATA;
                    requestMsg.ApplyHasQos();
                    requestMsg.Qos.Timeliness(QosTimeliness.REALTIME);
                    requestMsg.Qos.Rate(QosRates.TICK_BY_TICK);
                    requestMsg.ApplyHasView();
                    WatchlistItemDomainsTest.EncodeViewFieldIdList(ReactorChannel, view2List, requestMsg);

                    submitOptions.Clear();
                    submitOptions.ServiceName = Provider.DefaultService.Info.ServiceName.ToString();
                    Assert.True(Submit((Msg)requestMsg, submitOptions) >= ReactorReturnCode.SUCCESS);
                }
                else
                {
                    return base.ReactorChannelEventCallback(evt);
                }

                return ReactorCallbackReturnCode.SUCCESS;
            }
        }

        [Fact]
        public void WatchlistItemViewAggregationStreamAndSnapshotTest()
        {
            ReactorSubmitOptions submitOptions = new ReactorSubmitOptions();
            TestReactorEvent evt;
            ReactorMsgEvent msgEvent;

            TestReactor consumerReactor = new TestReactor();
            TestReactor providerReactor = new TestReactor();

            Consumer consumer = new SendStreamingAndSnapshotRequestsWithViewsConsumer(consumerReactor);
            ConsumerRole consumerRole = (ConsumerRole)consumer.ReactorRole;
            consumerRole.InitDefaultRDMLoginRequest();
            consumerRole.InitDefaultRDMDirectoryRequest();
            consumerRole.ChannelEventCallback = consumer;
            consumerRole.LoginMsgCallback = consumer;
            consumerRole.DirectoryMsgCallback = consumer;
            consumerRole.DictionaryMsgCallback = consumer;
            consumerRole.DefaultMsgCallback = consumer;
            consumerRole.WatchlistOptions.EnableWatchlist = true;
            consumerRole.WatchlistOptions.ChannelOpenEventCallback = consumer;

            Provider provider = new Provider(providerReactor);
            ProviderRole providerRole = (ProviderRole)provider.ReactorRole;
            providerRole.ChannelEventCallback = provider;
            providerRole.LoginMsgCallback = provider;
            providerRole.DirectoryMsgCallback = provider;
            providerRole.DictionaryMsgCallback = provider;
            providerRole.DefaultMsgCallback = provider;

            ConsumerProviderSessionOptions opts = new ConsumerProviderSessionOptions();
            opts.SetupDefaultLoginStream = true;
            opts.SetupDefaultDirectoryStream = true;

            provider.Bind(opts);

            opts.NumStatusEvents = 2; // set number of expected status message from request submitted in channel open callback
            TestReactor.OpenSession(consumer, provider, opts);

            int[] consStreamIds = { 5, 6 };

            List<int> view1List = new List<int> { 25, 32 };
            List<int> combindView = new List<int> { 6, 22, 25, 32 };

            /* Send streaming and snapshot requests with views on the CHANNEL_OPENED event*/

            /* Provider receives first snapshot request. */
            providerReactor.Dispatch(1);
            evt = providerReactor.PollEvent();
            Assert.Equal(TestReactorEventType.MSG, evt.EventType);
            msgEvent = (ReactorMsgEvent)evt.ReactorEvent;
            Assert.Equal(MsgClasses.REQUEST, msgEvent.Msg.MsgClass);
            IRequestMsg receivedRequestMsg = (IRequestMsg)msgEvent.Msg;
            Assert.True(receivedRequestMsg.CheckStreaming());
            Assert.True(receivedRequestMsg.CheckHasPriority());
            Assert.Equal(1, receivedRequestMsg.Priority.PriorityClass);
            Assert.Equal(1, receivedRequestMsg.Priority.Count);
            Assert.False(receivedRequestMsg.CheckNoRefresh());
            Assert.True(WatchlistItemDomainsTest.CheckHasCorrectView(provider, receivedRequestMsg, combindView));
            int providerStreamId = receivedRequestMsg.StreamId;

            /* Provider receives nothing yet. */
            providerReactor.Dispatch(0, new TimeSpan(0, 0, 0, 0, 500));
            Assert.True(providerReactor.m_EventQueue.Count == 0);

            /* Provider sends refresh, satisfying first view. */
            IRefreshMsg refreshMsg = new Msg();
            refreshMsg.MsgClass = MsgClasses.REFRESH;
            refreshMsg.DomainType = (int)DomainType.MARKET_PRICE;
            refreshMsg.StreamId = providerStreamId;
            refreshMsg.ContainerType = DataTypes.NO_DATA;
            refreshMsg.ApplyRefreshComplete();
            refreshMsg.ApplySolicited();
            refreshMsg.ApplyClearCache();
            refreshMsg.State.StreamState(StreamStates.OPEN);
            refreshMsg.State.DataState(DataStates.OK);
            refreshMsg.ApplyHasQos();
            refreshMsg.Qos.Timeliness(QosTimeliness.REALTIME);
            refreshMsg.Qos.Rate(QosRates.TICK_BY_TICK);

            submitOptions.Clear();
            Assert.True(provider.SubmitAndDispatch((Msg)refreshMsg, submitOptions) >= ReactorReturnCode.SUCCESS);

            /* Consumer receives refresh for the first item. */
            consumerReactor.Dispatch(2);
            evt = consumerReactor.PollEvent();
            Assert.Equal(TestReactorEventType.MSG, evt.EventType);
            msgEvent = (ReactorMsgEvent)evt.ReactorEvent;
            Assert.Equal(MsgClasses.REFRESH, msgEvent.Msg.MsgClass);
            IRefreshMsg receivedRefreshMsg = (IRefreshMsg)msgEvent.Msg;
            Assert.Equal((int)DomainType.MARKET_PRICE, receivedRefreshMsg.DomainType);
            Assert.Equal(DataTypes.NO_DATA, receivedRefreshMsg.ContainerType);
            Assert.Equal(StreamStates.OPEN, receivedRefreshMsg.State.StreamState());
            Assert.Equal(DataStates.OK, receivedRefreshMsg.State.DataState());
            Assert.Equal(consStreamIds[0], receivedRefreshMsg.StreamId);

            evt = consumerReactor.PollEvent();
            Assert.Equal(TestReactorEventType.MSG, evt.EventType);
            msgEvent = (ReactorMsgEvent)evt.ReactorEvent;
            Assert.Equal(MsgClasses.REFRESH, msgEvent.Msg.MsgClass);
            receivedRefreshMsg = (IRefreshMsg)msgEvent.Msg;
            Assert.Equal((int)DomainType.MARKET_PRICE, receivedRefreshMsg.DomainType);
            Assert.Equal(DataTypes.NO_DATA, receivedRefreshMsg.ContainerType);
            Assert.Equal(StreamStates.NON_STREAMING, receivedRefreshMsg.State.StreamState());
            Assert.Equal(DataStates.OK, receivedRefreshMsg.State.DataState());
            Assert.Equal(consStreamIds[1], receivedRefreshMsg.StreamId);

            /* Provider receives an addtional request to remove the snapshot's view. */
            providerReactor.Dispatch(1);
            evt = providerReactor.PollEvent();
            Assert.Equal(TestReactorEventType.MSG, evt.EventType);
            msgEvent = (ReactorMsgEvent)evt.ReactorEvent;
            Assert.Equal(MsgClasses.REQUEST, msgEvent.Msg.MsgClass);
            receivedRequestMsg = (IRequestMsg)msgEvent.Msg;
            Assert.True(receivedRequestMsg.CheckStreaming());
            Assert.True(receivedRequestMsg.CheckHasPriority());
            Assert.Equal(1, receivedRequestMsg.Priority.PriorityClass);
            Assert.Equal(1, receivedRequestMsg.Priority.Count);
            Assert.True(receivedRequestMsg.CheckNoRefresh());
            Assert.True(WatchlistItemDomainsTest.CheckHasCorrectView(provider, receivedRequestMsg, view1List));
            int providerStreamId2 = receivedRequestMsg.StreamId;

            TestReactorComponent.CloseSession(consumer, provider);
            consumerReactor.Close();
            providerReactor.Close();
        }
    }
}
 