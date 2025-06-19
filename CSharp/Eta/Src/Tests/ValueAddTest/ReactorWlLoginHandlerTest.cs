/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2023-2024 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

using System;
using System.Threading;

using Xunit;
using Xunit.Categories;
using Xunit.Abstractions;

using LSEG.Eta.Codec;
using LSEG.Eta.ValueAdd.Rdm;
using LSEG.Eta.ValueAdd.Reactor;
using LSEG.Eta.Rdm;
using LSEG.Eta.Common;

namespace LSEG.Eta.ValuedAdd.Tests;

[Collection("ValueAdded")]
public class ReactorWlLoginHandlerTest
{
    private readonly ITestOutputHelper output;

    public ReactorWlLoginHandlerTest(ITestOutputHelper output)
    {
        this.output = output;
    }

    private void TearDownConsumerAndProvider(TestReactor consumerReactor, TestReactor providerReactor)
    {
        consumerReactor.Close();
        providerReactor.Close();
    }

    #region Mock handlers
    internal class MockDirectoryHandler : IWlDirectoryHandler
    {
        public void Init() { }

        public WlServiceCache ServiceCache { get => throw new NotImplementedException(); set => throw new NotImplementedException(); }

        public void AddPendingRequest(WlStream wlStream)
        {
            throw new System.NotImplementedException();
        }

        public ReactorReturnCode CallbackUserWithMsg(string location, IMsg msg, WlRequest wlRequest, out ReactorErrorInfo errorInfo)
        {
            throw new System.NotImplementedException();
        }

        public ReactorReturnCode CallbackUserWithMsgBase(string location, IMsg msg, IRdmMsg msgBase, WlRequest wlRequest, out ReactorErrorInfo errorInfo)
        {
            throw new System.NotImplementedException();
        }

        public void ChannelDown()
        {
            throw new System.NotImplementedException();
        }

        public void ChannelUp(out ReactorErrorInfo errorInfo)
        {
            errorInfo = null;
        }

        public void Clear()
        {
            throw new System.NotImplementedException();
        }

        public void DeleteAllServices(bool isChannelDown, out ReactorErrorInfo errorInfo)
        {
            throw new NotImplementedException();
        }

        public ReactorReturnCode Dispatch(out ReactorErrorInfo errorInfo)
        {
            errorInfo = null;
            return ReactorReturnCode.SUCCESS;
        }

        public ReactorReturnCode LoginStreamClosed()
        {
            throw new System.NotImplementedException();
        }

        public ReactorReturnCode LoginStreamOpen(out ReactorErrorInfo errorInfo)
        {
            throw new System.NotImplementedException();
        }

        public void OnRequestTimeout(WlStream stream)
        {
            throw new NotImplementedException();
        }

        public ReactorReturnCode ReadMsg(WlStream wlStream, DecodeIterator decodeIt, IMsg msg, out ReactorErrorInfo errorInfo)
        {
            throw new System.NotImplementedException();
        }

        public int ServiceId(string serviceName)
        {
            throw new System.NotImplementedException();
        }

        public ReactorReturnCode SubmitMsg(WlRequest request, IMsg msg, ReactorSubmitOptions submitOptions, out ReactorErrorInfo errorInfo)
        {
            throw new System.NotImplementedException();
        }

        public ReactorReturnCode SubmitRequest(WlRequest request, IRequestMsg requestMsg, bool isReissue, ReactorSubmitOptions submitOptions, out ReactorErrorInfo errorInfo)
        {
            throw new System.NotImplementedException();
        }
    }

    internal class MockItemHandler : IWlItemHandler
    {
        public void AddPendingRequest(WlStream wlStream)
        {
            throw new System.NotImplementedException();
        }

        public ReactorReturnCode CallbackUserWithMsg(string location, IMsg msg, WlRequest wlRequest, out ReactorErrorInfo errorInfo)
        {
            throw new System.NotImplementedException();
        }

        public ReactorReturnCode CallbackUserWithMsgBase(string location, IMsg msg, IRdmMsg msgBase, WlRequest wlRequest, out ReactorErrorInfo errorInfo)
        {
            throw new System.NotImplementedException();
        }

        public void ChannelDown()
        {
            throw new System.NotImplementedException();
        }

        public void ChannelUp(out ReactorErrorInfo errorInfo)
        {
            errorInfo = null;
        }

        public void Clear()
        {
            throw new System.NotImplementedException();
        }

        public ReactorReturnCode Dispatch(out ReactorErrorInfo errorInfo)
        {
            errorInfo = null;
            return ReactorReturnCode.SUCCESS;
        }

        public void Init(ConsumerRole consumerRole)
        {
        }

        public bool IsRequestRecoverable(WlRequest wlRequest, int streamState)
        {
            throw new System.NotImplementedException();
        }

        public ReactorReturnCode LoginStreamClosed(State state)
        {
            throw new System.NotImplementedException();
        }

        public ReactorReturnCode LoginStreamOpen(out ReactorErrorInfo errorInfo)
        {
            throw new System.NotImplementedException();
        }

        public void OnRequestTimeout(WlStream stream)
        {
            throw new NotImplementedException();
        }

        public ReactorReturnCode PauseAll()
        {
            throw new System.NotImplementedException();
        }

        public ReactorReturnCode ReadMsg(WlStream wlStream, DecodeIterator decodeIt, IMsg msg, out ReactorErrorInfo errorInfo)
        {
            throw new System.NotImplementedException();
        }

        public ReactorReturnCode ResumeAll()
        {
            throw new System.NotImplementedException();
        }

        public ReactorReturnCode SubmitMsg(WlRequest request, IMsg msg, ReactorSubmitOptions submitOptions, out ReactorErrorInfo errorInfo)
        {
            throw new System.NotImplementedException();
        }

        public ReactorReturnCode SubmitRequest(WlRequest request, IRequestMsg requestMsg, bool isReissue, ReactorSubmitOptions submitOptions, out ReactorErrorInfo errorInfo)
        {
            errorInfo = null;
            return ReactorReturnCode.SUCCESS;
        }
    }

    #endregion

    [Fact]
    [Category("Unit")]
    [Category("Reactor")]
    public void LoginPauseResumeTest_Socket()
    {
        LoginPauseResume(false, null);
    }

    private void LoginPauseResume(bool isWebsocket, string protocolList)
    {
        ReactorSubmitOptions submitOptions = new();
        TestReactorEvent reactorEvent;
        ReactorMsgEvent msgEvent;
        Msg msg = new Msg();
        IRequestMsg requestMsg = (IRequestMsg)new Msg();
        IRequestMsg receivedRequestMsg;
        IRefreshMsg refreshMsg = (IRefreshMsg)msg;
        IRefreshMsg receivedRefreshMsg;
        int providerStreamId;

        /* Create reactors. */
        TestReactor consumerReactor = new TestReactor();
        TestReactor providerReactor = new TestReactor();

        /* Create consumer. */
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

        /* Create provider. */
        Provider provider = new Provider(providerReactor);
        ProviderRole providerRole = (ProviderRole)provider.ReactorRole;
        providerRole.ChannelEventCallback = provider;
        providerRole.LoginMsgCallback = provider;
        providerRole.DirectoryMsgCallback = provider;
        providerRole.DictionaryMsgCallback = provider;
        providerRole.DefaultMsgCallback = provider;

        /* Connect the consumer and provider. Setup login & directory streams automatically. */
        ConsumerProviderSessionOptions opts = new ConsumerProviderSessionOptions();
        opts.SetupDefaultLoginStream = true;
        opts.SetupDefaultDirectoryStream = true;

        provider.Bind(opts);

        TestReactor.OpenSession(consumer, provider, opts);

        /* submit a item request messages */
        requestMsg.Clear();
        requestMsg.MsgClass = MsgClasses.REQUEST;
        requestMsg.StreamId = 5;
        requestMsg.DomainType = (int)DomainType.MARKET_PRICE;
        requestMsg.ApplyStreaming();
        requestMsg.MsgKey.ApplyHasName();
        requestMsg.MsgKey.Name.Data("JPM.N");

        requestMsg.ApplyHasPriority();
        requestMsg.Priority.Count = 11;
        requestMsg.Priority.PriorityClass = 22;
        submitOptions.Clear();
        submitOptions.ServiceName = Provider.DefaultService.Info.ServiceName.ToString();
        Assert.True(consumer.SubmitAndDispatch((Msg)requestMsg, submitOptions) >= ReactorReturnCode.SUCCESS);

        /* Provider receives requests. */
        providerReactor.Dispatch(1);
        reactorEvent = providerReactor.PollEvent();
        Assert.Equal(TestReactorEventType.MSG, reactorEvent.EventType);
        msgEvent = (ReactorMsgEvent)reactorEvent.ReactorEvent;
        Assert.Equal(MsgClasses.REQUEST, msgEvent.Msg.MsgClass);

        receivedRequestMsg = (IRequestMsg)msgEvent.Msg;
        Assert.Equal(3, receivedRequestMsg.StreamId);
        Assert.True(receivedRequestMsg.MsgKey.CheckHasServiceId());
        Assert.True(receivedRequestMsg.CheckStreaming());
        Assert.Equal(Provider.DefaultService.ServiceId, receivedRequestMsg.MsgKey.ServiceId);
        Assert.True(receivedRequestMsg.MsgKey.CheckHasName());
        Assert.Equal("JPM.N", receivedRequestMsg.MsgKey.Name.ToString());
        Assert.Equal((int)DomainType.MARKET_PRICE, receivedRequestMsg.DomainType);
        Assert.True(receivedRequestMsg.CheckHasPriority());
        Assert.Equal(11, receivedRequestMsg.Priority.Count);
        Assert.Equal(22, receivedRequestMsg.Priority.PriorityClass);
        providerStreamId = receivedRequestMsg.StreamId;

        /* Provider sends refresh .*/
        refreshMsg.Clear();
        refreshMsg.MsgClass = MsgClasses.REFRESH;
        refreshMsg.DomainType = (int)DomainType.MARKET_PRICE;
        refreshMsg.StreamId = providerStreamId;
        refreshMsg.ContainerType = DataTypes.NO_DATA;
        refreshMsg.ApplyRefreshComplete();
        refreshMsg.ApplyHasMsgKey();
        refreshMsg.MsgKey.ApplyHasServiceId();
        refreshMsg.MsgKey.ServiceId = Provider.DefaultService.ServiceId;
        refreshMsg.MsgKey.ApplyHasName();
        refreshMsg.MsgKey.Name.Data("JPM.N");
        refreshMsg.State.StreamState(StreamStates.OPEN);
        refreshMsg.State.DataState(DataStates.OK);
        refreshMsg.ApplySolicited();

        Assert.True(provider.SubmitAndDispatch((Msg)refreshMsg, submitOptions) >= ReactorReturnCode.SUCCESS);

        consumerReactor.Dispatch(1);
        reactorEvent = consumerReactor.PollEvent();
        Assert.Equal(TestReactorEventType.MSG, reactorEvent.EventType);
        msgEvent = (ReactorMsgEvent)reactorEvent.ReactorEvent;
        Assert.Equal(MsgClasses.REFRESH, msgEvent.Msg.MsgClass);

        receivedRefreshMsg = (IRefreshMsg)msgEvent.Msg;
        Assert.True(receivedRefreshMsg.CheckHasMsgKey());
        Assert.True(receivedRefreshMsg.MsgKey.CheckHasServiceId());
        Assert.Equal(Provider.DefaultService.ServiceId, receivedRefreshMsg.MsgKey.ServiceId);
        Assert.True(receivedRefreshMsg.MsgKey.CheckHasName());
        Assert.Equal((int)DomainType.MARKET_PRICE, receivedRefreshMsg.DomainType);
        Assert.Equal(DataTypes.NO_DATA, receivedRefreshMsg.ContainerType);
        Assert.Equal(StreamStates.OPEN, receivedRefreshMsg.State.StreamState());
        Assert.Equal(DataStates.OK, receivedRefreshMsg.State.DataState());
        Assert.NotNull(msgEvent.StreamInfo);
        Assert.NotNull(msgEvent.StreamInfo.ServiceName);
        Assert.Equal(msgEvent.StreamInfo.ServiceName, Provider.DefaultService.Info.ServiceName.ToString());

        // consumer reissues login with pause
        LoginRequest loginRequest = consumerRole.RdmLoginRequest;
        Assert.NotNull(loginRequest);
        loginRequest.Pause = true;
        loginRequest.NoRefresh = true;
        submitOptions.Clear();
        Assert.True(consumer.SubmitAndDispatch(loginRequest, submitOptions) >= ReactorReturnCode.SUCCESS);

        /* Provider receives request. */
        providerReactor.Dispatch(1);
        reactorEvent = providerReactor.PollEvent();
        Assert.Equal(TestReactorEventType.LOGIN_MSG, reactorEvent.EventType);
        msgEvent = (ReactorMsgEvent)reactorEvent.ReactorEvent;
        Assert.Equal(MsgClasses.REQUEST, msgEvent.Msg.MsgClass);

        receivedRequestMsg = (IRequestMsg)msgEvent.Msg;
        Assert.Equal(1, receivedRequestMsg.StreamId); // stream id should be same as first request
        // Login paused and item would be paused too

        // check that login handler is awaiting resume all
        WlLoginHandler loginHandler = (WlLoginHandler)consumerReactor.ComponentList[0].ReactorChannel.Watchlist.LoginHandler;
        Assert.True(loginHandler.m_AwaitingResumeAll);
        // all items should be paused
        int pausedCount = ((WlItemHandler)consumerReactor.ComponentList[0].ReactorChannel.Watchlist.ItemHandler).StreamDlList.Peek().NumPausedRequestsCount;
        int itemCount = ((WlItemHandler)consumerReactor.ComponentList[0].ReactorChannel.Watchlist.ItemHandler).StreamDlList.Peek().UserRequestDlList.Count();
        Assert.Equal(1, pausedCount);
        Assert.Equal(1, itemCount);

        // consumer reissues login with resume
        loginRequest.Flags &= ~LoginRequestFlags.PAUSE_ALL;
        submitOptions.Clear();
        Assert.True(consumer.SubmitAndDispatch(loginRequest, submitOptions) >= ReactorReturnCode.SUCCESS);

        /* Provider receives request. */
        providerReactor.Dispatch(1);
        reactorEvent = providerReactor.PollEvent();
        Assert.Equal(TestReactorEventType.LOGIN_MSG, reactorEvent.EventType);
        msgEvent = (ReactorMsgEvent)reactorEvent.ReactorEvent;
        Assert.Equal(MsgClasses.REQUEST, msgEvent.Msg.MsgClass);

        receivedRequestMsg = (IRequestMsg)msgEvent.Msg;
        Assert.Equal(1, receivedRequestMsg.StreamId); // stream id should be same as first request

        // check that login handler is not awaiting resume all
        loginHandler = (WlLoginHandler)consumerReactor.ComponentList[0].ReactorChannel.Watchlist.LoginHandler;
        Assert.False(loginHandler.m_AwaitingResumeAll);
        // all items should be resumed
        pausedCount = ((WlItemHandler)consumerReactor.ComponentList[0].ReactorChannel.Watchlist.ItemHandler).StreamDlList.Peek().NumPausedRequestsCount;
        itemCount = ((WlItemHandler)consumerReactor.ComponentList[0].ReactorChannel.Watchlist.ItemHandler).StreamDlList.Peek().UserRequestDlList.Count();
        Assert.Equal(0, pausedCount);
        Assert.Equal(1, itemCount);

        TestReactorComponent.CloseSession(consumer, provider);
        TearDownConsumerAndProvider(consumerReactor, providerReactor);
    }

    [Fact]
    [Category("Unit")]
    [Category("Reactor")]
    public void LoginPauseOnFirstRequestResumeTest()
    {
        ReactorSubmitOptions submitOptions = new ReactorSubmitOptions();
        TestReactorEvent reactorEvent;
        ReactorMsgEvent msgEvent;
        Msg msg = new Msg();
        IRequestMsg requestMsg = (IRequestMsg)new Msg();
        IRequestMsg receivedRequestMsg;
        IRefreshMsg refreshMsg = (IRefreshMsg)msg;
        IRefreshMsg receivedRefreshMsg;
        int providerStreamId;

        /* Create reactors. */
        TestReactor consumerReactor = new TestReactor();
        TestReactor providerReactor = new TestReactor();

        /* Create consumer. */
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

        /* Create provider. */
        Provider provider = new Provider(providerReactor);
        ProviderRole providerRole = (ProviderRole)provider.ReactorRole;
        providerRole.ChannelEventCallback = provider;
        providerRole.LoginMsgCallback = provider;
        providerRole.DirectoryMsgCallback = provider;
        providerRole.DictionaryMsgCallback = provider;
        providerRole.DefaultMsgCallback = provider;

        /* Connect the consumer and provider. Setup login & directory streams automatically. */
        ConsumerProviderSessionOptions opts = new ConsumerProviderSessionOptions();
        opts.SetupDefaultLoginStream = true;
        opts.SetupDefaultDirectoryStream = true;

        provider.Bind(opts);
        TestReactor.OpenSession(consumer, provider, opts);

        // submit a item request messages
        requestMsg.Clear();
        requestMsg.MsgClass = MsgClasses.REQUEST;
        requestMsg.StreamId = 5;
        requestMsg.DomainType = (int)DomainType.MARKET_PRICE;
        requestMsg.ApplyStreaming();
        requestMsg.MsgKey.ApplyHasName();
        requestMsg.MsgKey.Name.Data("JPM.N");

        requestMsg.ApplyHasPriority();
        requestMsg.ApplyPause();
        requestMsg.Priority.Count = 11;
        requestMsg.Priority.PriorityClass = 22;
        submitOptions.Clear();
        submitOptions.ServiceName = Provider.DefaultService.Info.ServiceName.ToString();
        Assert.True(consumer.SubmitAndDispatch((Msg)requestMsg, submitOptions) >= ReactorReturnCode.SUCCESS);

        /* Provider receives requests. */
        providerReactor.Dispatch(1);
        reactorEvent = providerReactor.PollEvent();
        Assert.Equal(TestReactorEventType.MSG, reactorEvent.EventType);
        msgEvent = (ReactorMsgEvent)reactorEvent.ReactorEvent;
        Assert.Equal(MsgClasses.REQUEST, msgEvent.Msg.MsgClass);

        receivedRequestMsg = (IRequestMsg)msgEvent.Msg;
        Assert.Equal(3, receivedRequestMsg.StreamId);
        Assert.True(receivedRequestMsg.MsgKey.CheckHasServiceId());
        Assert.True(receivedRequestMsg.CheckStreaming());
        Assert.Equal(Provider.DefaultService.ServiceId, receivedRequestMsg.MsgKey.ServiceId);
        Assert.True(receivedRequestMsg.MsgKey.CheckHasName());
        Assert.Equal("JPM.N", receivedRequestMsg.MsgKey.Name.ToString());
        Assert.Equal((int)DomainType.MARKET_PRICE, receivedRequestMsg.DomainType);
        Assert.True(receivedRequestMsg.CheckHasPriority());
        Assert.Equal(11, receivedRequestMsg.Priority.Count);
        Assert.Equal(22, receivedRequestMsg.Priority.PriorityClass);
        providerStreamId = receivedRequestMsg.StreamId;

        /* Provider sends refresh .*/
        refreshMsg.Clear();
        refreshMsg.MsgClass = MsgClasses.REFRESH;
        refreshMsg.DomainType = (int)DomainType.MARKET_PRICE;
        refreshMsg.StreamId = providerStreamId;
        refreshMsg.ContainerType = DataTypes.NO_DATA;
        refreshMsg.ApplyRefreshComplete();
        refreshMsg.ApplyHasMsgKey();
        refreshMsg.MsgKey.ApplyHasServiceId();
        refreshMsg.MsgKey.ServiceId = Provider.DefaultService.ServiceId;
        refreshMsg.MsgKey.ApplyHasName();
        refreshMsg.MsgKey.Name.Data("JPM.N");
        refreshMsg.State.StreamState(StreamStates.OPEN);
        refreshMsg.State.DataState(DataStates.OK);
        refreshMsg.ApplySolicited();

        Assert.True(provider.SubmitAndDispatch((Msg)refreshMsg, submitOptions) >= ReactorReturnCode.SUCCESS);

        consumerReactor.Dispatch(1);
        reactorEvent = consumerReactor.PollEvent();
        Assert.Equal(TestReactorEventType.MSG, reactorEvent.EventType);
        msgEvent = (ReactorMsgEvent)reactorEvent.ReactorEvent;
        Assert.Equal(MsgClasses.REFRESH, msgEvent.Msg.MsgClass);

        receivedRefreshMsg = (IRefreshMsg)msgEvent.Msg;
        Assert.True(receivedRefreshMsg.CheckHasMsgKey());
        Assert.True(receivedRefreshMsg.MsgKey.CheckHasServiceId());
        Assert.Equal(Provider.DefaultService.ServiceId, receivedRefreshMsg.MsgKey.ServiceId);
        Assert.True(receivedRefreshMsg.MsgKey.CheckHasName());
        Assert.Equal((int)DomainType.MARKET_PRICE, receivedRefreshMsg.DomainType);
        Assert.Equal(DataTypes.NO_DATA, receivedRefreshMsg.ContainerType);
        Assert.Equal(StreamStates.OPEN, receivedRefreshMsg.State.StreamState());
        Assert.Equal(DataStates.OK, receivedRefreshMsg.State.DataState());
        Assert.NotNull(msgEvent.StreamInfo);
        Assert.NotNull(msgEvent.StreamInfo.ServiceName);
        Assert.Equal(msgEvent.StreamInfo.ServiceName, Provider.DefaultService.Info.ServiceName.ToString());

        // consumer reissues request with resume
        IRequestMsg reissueRequestMsg = (IRequestMsg)new Msg();
        reissueRequestMsg.ApplyNoRefresh();

        reissueRequestMsg.MsgClass = MsgClasses.REQUEST;
        reissueRequestMsg.StreamId = 5;
        reissueRequestMsg.DomainType = (int)DomainType.MARKET_PRICE;
        reissueRequestMsg.ApplyStreaming();
        reissueRequestMsg.MsgKey.ApplyHasName();
        reissueRequestMsg.MsgKey.Name.Data("JPM.N");

        submitOptions.Clear();
        submitOptions.ServiceName = Provider.DefaultService.Info.ServiceName.ToString();
        Assert.True(consumer.SubmitAndDispatch((Msg)reissueRequestMsg, submitOptions) >= ReactorReturnCode.SUCCESS);

        /* Provider receives request. */
        providerReactor.Dispatch(1);
        reactorEvent = providerReactor.PollEvent();
        Assert.Equal(TestReactorEventType.MSG, reactorEvent.EventType);
        msgEvent = (ReactorMsgEvent)reactorEvent.ReactorEvent;
        Assert.Equal(MsgClasses.REQUEST, msgEvent.Msg.MsgClass);

        receivedRequestMsg = (IRequestMsg)msgEvent.Msg;
        Assert.Equal(3, receivedRequestMsg.StreamId); // stream id should be same as first request

        // check that login handler is not awaiting resume all
        WlLoginHandler loginHandler = (WlLoginHandler)consumerReactor.ComponentList[0].ReactorChannel.Watchlist.LoginHandler;
        Assert.False(loginHandler.m_AwaitingResumeAll);
        // all items should be resumed
        int pausedCount = ((WlItemHandler)consumerReactor.ComponentList[0].ReactorChannel.Watchlist.ItemHandler).StreamDlList.Peek().NumPausedRequestsCount;
        int itemCount = ((WlItemHandler)consumerReactor.ComponentList[0].ReactorChannel.Watchlist.ItemHandler).StreamDlList.Peek().UserRequestDlList.Count();
        Assert.Equal(0, pausedCount);
        Assert.Equal(1, itemCount);

        TestReactorComponent.CloseSession(consumer, provider);
        TearDownConsumerAndProvider(consumerReactor, providerReactor);
    }

    [Fact]
    [Category("Unit")]
    [Category("Reactor")]
    public void LoginPauseResumeTokenTest_Socket()
    {
        LoginPauseResumeToken(false, null);
    }

    private void LoginPauseResumeToken(bool isWebsocket, string protocolList)
    {
        ReactorSubmitOptions submitOptions = new ReactorSubmitOptions();
        TestReactorEvent reactorEvent;
        ReactorMsgEvent msgEvent;
        Msg msg = new Msg();
        IRequestMsg requestMsg = (IRequestMsg)new Msg();
        IRequestMsg receivedRequestMsg;
        IRefreshMsg refreshMsg = (IRefreshMsg)msg;
        IRefreshMsg receivedRefreshMsg;
        int providerStreamId;
        string authenticationToken1 = "authenticationToken1";
        string authenticationToken2 = "authenticationToken2";

        /* Create reactors. */
        TestReactor consumerReactor = new TestReactor();
        TestReactor providerReactor = new TestReactor();

        /* Create consumer. */
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

        /* Create provider. */
        Provider provider = new Provider(providerReactor);
        ProviderRole providerRole = (ProviderRole)provider.ReactorRole;
        providerRole.ChannelEventCallback = provider;
        providerRole.LoginMsgCallback = provider;
        providerRole.DirectoryMsgCallback = provider;
        providerRole.DictionaryMsgCallback = provider;
        providerRole.DefaultMsgCallback = provider;

        /* add authToken to login request */
        consumerRole.RdmLoginRequest.UserNameType = Login.UserIdTypes.AUTHN_TOKEN;
        Codec.Buffer authTokenBuffer = new Codec.Buffer();
        authTokenBuffer.Data(authenticationToken1);
        consumerRole.RdmLoginRequest.UserName = authTokenBuffer;

        /* Connect the consumer and provider. Setup login & directory streams automatically. */
        ConsumerProviderSessionOptions opts = new ConsumerProviderSessionOptions();
        opts.SetupDefaultLoginStream = true;
        opts.SetupDefaultDirectoryStream = true;

        provider.Bind(opts);

        TestReactor.OpenSession(consumer, provider, opts);

        // submit a item request messages
        requestMsg.Clear();
        requestMsg.MsgClass = MsgClasses.REQUEST;
        requestMsg.StreamId = 5;
        requestMsg.DomainType = (int)DomainType.MARKET_PRICE;
        requestMsg.ApplyStreaming();
        requestMsg.MsgKey.ApplyHasName();
        requestMsg.MsgKey.Name.Data("JPM.N");

        requestMsg.ApplyHasPriority();
        requestMsg.Priority.Count = 11;
        requestMsg.Priority.PriorityClass = 22;
        submitOptions.Clear();
        submitOptions.ServiceName = Provider.DefaultService.Info.ServiceName.ToString();
        Assert.True(consumer.SubmitAndDispatch((Msg)requestMsg, submitOptions) >= ReactorReturnCode.SUCCESS);

        /* Provider receives requests. */
        providerReactor.Dispatch(1);
        reactorEvent = providerReactor.PollEvent();
        Assert.Equal(TestReactorEventType.MSG, reactorEvent.EventType);
        msgEvent = (ReactorMsgEvent)reactorEvent.ReactorEvent;
        Assert.Equal(MsgClasses.REQUEST, msgEvent.Msg.MsgClass);

        receivedRequestMsg = (IRequestMsg)msgEvent.Msg;
        Assert.Equal(3, receivedRequestMsg.StreamId);
        Assert.True(receivedRequestMsg.MsgKey.CheckHasServiceId());
        Assert.True(receivedRequestMsg.CheckStreaming());
        Assert.Equal(Provider.DefaultService.ServiceId, receivedRequestMsg.MsgKey.ServiceId);
        Assert.True(receivedRequestMsg.MsgKey.CheckHasName());
        Assert.Equal("JPM.N", receivedRequestMsg.MsgKey.Name.ToString());
        Assert.Equal((int)DomainType.MARKET_PRICE, receivedRequestMsg.DomainType);
        Assert.True(receivedRequestMsg.CheckHasPriority());
        Assert.Equal(11, receivedRequestMsg.Priority.Count);
        Assert.Equal(22, receivedRequestMsg.Priority.PriorityClass);
        providerStreamId = receivedRequestMsg.StreamId;

        /* Provider sends refresh .*/
        refreshMsg.Clear();
        refreshMsg.MsgClass = MsgClasses.REFRESH;
        refreshMsg.DomainType = (int)DomainType.MARKET_PRICE;
        refreshMsg.StreamId = providerStreamId;
        refreshMsg.ContainerType = DataTypes.NO_DATA;
        refreshMsg.ApplyRefreshComplete();
        refreshMsg.ApplyHasMsgKey();
        refreshMsg.MsgKey.ApplyHasServiceId();
        refreshMsg.MsgKey.ServiceId = Provider.DefaultService.ServiceId;
        refreshMsg.MsgKey.ApplyHasName();
        refreshMsg.MsgKey.Name.Data("JPM.N");
        refreshMsg.State.StreamState(StreamStates.OPEN);
        refreshMsg.State.DataState(DataStates.OK);
        refreshMsg.ApplySolicited();

        Assert.True(provider.SubmitAndDispatch((Msg)refreshMsg, submitOptions) >= ReactorReturnCode.SUCCESS);

        consumerReactor.Dispatch(1);
        reactorEvent = consumerReactor.PollEvent();
        Assert.Equal(TestReactorEventType.MSG, reactorEvent.EventType);
        msgEvent = (ReactorMsgEvent)reactorEvent.ReactorEvent;
        Assert.Equal(MsgClasses.REFRESH, msgEvent.Msg.MsgClass);

        receivedRefreshMsg = (IRefreshMsg)msgEvent.Msg;
        Assert.True(receivedRefreshMsg.CheckHasMsgKey());
        Assert.True(receivedRefreshMsg.MsgKey.CheckHasServiceId());
        Assert.Equal(Provider.DefaultService.ServiceId, receivedRefreshMsg.MsgKey.ServiceId);
        Assert.True(receivedRefreshMsg.MsgKey.CheckHasName());
        Assert.Equal((int)DomainType.MARKET_PRICE, receivedRefreshMsg.DomainType);
        Assert.Equal(DataTypes.NO_DATA, receivedRefreshMsg.ContainerType);
        Assert.Equal(StreamStates.OPEN, receivedRefreshMsg.State.StreamState());
        Assert.Equal(DataStates.OK, receivedRefreshMsg.State.DataState());
        Assert.NotNull(msgEvent.StreamInfo);
        Assert.NotNull(msgEvent.StreamInfo.ServiceName);
        Assert.Equal(msgEvent.StreamInfo.ServiceName, Provider.DefaultService.Info.ServiceName.ToString());

        // consumer reissues login with pause
        LoginRequest loginRequest = consumerRole.RdmLoginRequest;
        Assert.NotNull(loginRequest);
        loginRequest.Pause = true;
        loginRequest.NoRefresh = true;
        submitOptions.Clear();
        Assert.True(consumer.SubmitAndDispatch(loginRequest, submitOptions) >= ReactorReturnCode.SUCCESS);

        /* Provider receives request. */
        providerReactor.Dispatch(1);
        reactorEvent = providerReactor.PollEvent();
        Assert.Equal(TestReactorEventType.LOGIN_MSG, reactorEvent.EventType);
        msgEvent = (ReactorMsgEvent)reactorEvent.ReactorEvent;
        Assert.Equal(MsgClasses.REQUEST, msgEvent.Msg.MsgClass);

        receivedRequestMsg = (IRequestMsg)msgEvent.Msg;
        Assert.Equal(1, receivedRequestMsg.StreamId); // stream id should be same as first request
                                                      // Login paused and item would be paused too

        // check that login handler is awaiting resume all
        WlLoginHandler loginHandler = (WlLoginHandler)consumerReactor.ComponentList[0].ReactorChannel.Watchlist.LoginHandler;
        Assert.True(loginHandler.m_AwaitingResumeAll);
        // all items should be paused
        int pausedCount = ((WlItemHandler)consumerReactor.ComponentList[0].ReactorChannel.Watchlist.ItemHandler).StreamDlList.Peek().NumPausedRequestsCount;
        int itemCount = ((WlItemHandler)consumerReactor.ComponentList[0].ReactorChannel.Watchlist.ItemHandler).StreamDlList.Peek().UserRequestDlList.Count();
        Assert.Equal(1, pausedCount);
        Assert.Equal(1, itemCount);

        // consumer reissues login with resume with token change
        authTokenBuffer.Data(authenticationToken2);
        loginRequest.UserName = authTokenBuffer;
        loginRequest.Flags &= ~LoginRequestFlags.PAUSE_ALL;
        submitOptions.Clear();
        Assert.True(consumer.SubmitAndDispatch(loginRequest, submitOptions) >= ReactorReturnCode.SUCCESS);

        /* Provider receives request. */
        providerReactor.Dispatch(1);
        reactorEvent = providerReactor.PollEvent();
        Assert.Equal(TestReactorEventType.LOGIN_MSG, reactorEvent.EventType);
        msgEvent = (ReactorMsgEvent)reactorEvent.ReactorEvent;
        Assert.Equal(MsgClasses.REQUEST, msgEvent.Msg.MsgClass);

        receivedRequestMsg = (IRequestMsg)msgEvent.Msg;
        Assert.Equal(1, receivedRequestMsg.StreamId); // stream id should be same as first request

        // check that login handler is still awaiting resume all since token change doesn't result in resume
        loginHandler = (WlLoginHandler)consumerReactor.ComponentList[0].ReactorChannel.Watchlist.LoginHandler;
        Assert.True(loginHandler.m_AwaitingResumeAll);
        // all items should still be paused
        pausedCount = ((WlItemHandler)consumerReactor.ComponentList[0].ReactorChannel.Watchlist.ItemHandler).StreamDlList.Peek().NumPausedRequestsCount;
        itemCount = ((WlItemHandler)consumerReactor.ComponentList[0].ReactorChannel.Watchlist.ItemHandler).StreamDlList.Peek().UserRequestDlList.Count();
        Assert.Equal(1, pausedCount);
        Assert.Equal(1, itemCount);

        // consumer reissues login with resume again with same token
        loginRequest.Flags &= ~LoginRequestFlags.PAUSE_ALL;
        submitOptions.Clear();
        Assert.True(consumer.SubmitAndDispatch(loginRequest, submitOptions) >= ReactorReturnCode.SUCCESS);

        /* Provider receives request. */
        providerReactor.Dispatch(1);
        reactorEvent = providerReactor.PollEvent();
        Assert.Equal(TestReactorEventType.LOGIN_MSG, reactorEvent.EventType);
        msgEvent = (ReactorMsgEvent)reactorEvent.ReactorEvent;
        Assert.Equal(MsgClasses.REQUEST, msgEvent.Msg.MsgClass);

        receivedRequestMsg = (IRequestMsg)msgEvent.Msg;
        Assert.Equal(1, receivedRequestMsg.StreamId); // stream id should be same as first request

        // check that login handler is not awaiting resume all
        loginHandler = (WlLoginHandler)consumerReactor.ComponentList[0].ReactorChannel.Watchlist.LoginHandler;
        Assert.False(loginHandler.m_AwaitingResumeAll);
        // all items should be resumed
        pausedCount = ((WlItemHandler)consumerReactor.ComponentList[0].ReactorChannel.Watchlist.ItemHandler).StreamDlList.Peek().NumPausedRequestsCount;
        itemCount = ((WlItemHandler)consumerReactor.ComponentList[0].ReactorChannel.Watchlist.ItemHandler).StreamDlList.Peek().UserRequestDlList.Count();
        Assert.Equal(0, pausedCount);
        Assert.Equal(1, itemCount);

        TestReactorComponent.CloseSession(consumer, provider);
        TearDownConsumerAndProvider(consumerReactor, providerReactor);
    }

    [Fact]
    [Category("Unit")]
    [Category("Reactor")]
    public void LoginClosedRecoverTest_SingleOpenOn()
    {
        LoginClosedRecoverTest(true);
    }

    [Fact]
    [Category("Unit")]
    [Category("Reactor")]
    public void LoginClosedRecoverTest_SingleOpenOff()
    {
        LoginClosedRecoverTest(false);
    }


    private void LoginClosedRecoverTest(bool singleOpen)
    {
        /* Tests behavior in response to receiving login closed/recover.
         * We should see:
         * - Watchlist disconnects in response to receiving this
         *   (and unless the login stream was previously established, ensure
         *   the connection can backoff).
         * - Tests include:
         *   - Provider sending ClosedRecover login StatusMsg/RefreshMsg in response to request.
         *   - As above, but after sending an Open/Ok login refresh first.  */

        ReactorSubmitOptions submitOptions = new ReactorSubmitOptions();
        TestReactorEvent reactorEvent;
        ReactorMsgEvent msgEvent;
        Msg msg = new Msg();
        IRequestMsg requestMsg = (IRequestMsg)msg;
        LoginRefresh loginRefresh = new LoginRefresh(), recvLoginRefresh;
        LoginStatus loginStatus = new LoginStatus(), recvLoginStatus;
        DirectoryRequest recvDirectoryRequest;
        DirectoryUpdate recvDirectoryUpdate;
        RDMLoginMsgEvent loginMsgEvent;
        RDMDirectoryMsgEvent directoryMsgEvent;
        IStatusMsg recvStatusMsg;
        ReactorChannelEvent channelEvent;
        LoginRequest recvLoginRequest;

        int reconnectMinDelay = 1000, reconnectMaxDelay = 3000;
        long expectedReconnectDelayTimeMs;
        long maxExpectedReconnectDelay = 300;
        System.DateTime startTimeMs;
        long deviationTimeMs;

        /* Create reactors. */
        TestReactor consumerReactor = new TestReactor();
        TestReactor providerReactor = new TestReactor();

        /* Create consumer. */
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
        consumerRole.WatchlistOptions.RequestTimeout = 3000;
        consumerRole.RdmLoginRequest.HasAttrib = true;
        consumerRole.RdmLoginRequest.LoginAttrib.HasSingleOpen = true;
        consumerRole.RdmLoginRequest.LoginAttrib.SingleOpen = singleOpen ? 1 : 0;

        /* Create provider. */
        Provider provider = new Provider(providerReactor);
        ProviderRole providerRole = (ProviderRole)provider.ReactorRole;
        providerRole.ChannelEventCallback = provider;
        providerRole.LoginMsgCallback = provider;
        providerRole.DirectoryMsgCallback = provider;
        providerRole.DictionaryMsgCallback = provider;
        providerRole.DefaultMsgCallback = provider;

        /* Connect the consumer and provider. Setup login & directory streams automatically. */
        ConsumerProviderSessionOptions opts = new ConsumerProviderSessionOptions();
        opts.SetupDefaultLoginStream = true;
        opts.SetupDefaultDirectoryStream = true;
        opts.ReconnectAttemptLimit = -1;
        opts.ReconnectMinDelay = TimeSpan.FromMilliseconds(reconnectMinDelay);
        opts.ReconnectMaxDelay = TimeSpan.FromMilliseconds(reconnectMaxDelay);

        provider.Bind(opts);
        TestReactor.OpenSession(consumer, provider, opts);

        /* Consumer sends request. */
        requestMsg.Clear();
        requestMsg.MsgClass = MsgClasses.REQUEST;
        requestMsg.StreamId = 5;
        requestMsg.DomainType = (int)DomainType.MARKET_PRICE;
        requestMsg.ApplyStreaming();
        requestMsg.MsgKey.ApplyHasName();
        requestMsg.MsgKey.Name.Data("TRI.N");
        submitOptions.Clear();
        submitOptions.ServiceName = Provider.DefaultService.Info.ServiceName.ToString();
        Assert.True(consumer.SubmitAndDispatch((Msg)requestMsg, submitOptions) >= ReactorReturnCode.SUCCESS);

        /* Provider receives request. */
        providerReactor.Dispatch(1);
        reactorEvent = providerReactor.PollEvent();
        Assert.Equal(TestReactorEventType.MSG, reactorEvent.EventType);
        msgEvent = (ReactorMsgEvent)reactorEvent.ReactorEvent;
        Assert.Equal(MsgClasses.REQUEST, msgEvent.Msg.MsgClass);

        /* Do two loops of sending login ClosedRecover status from the provider.
         * Make sure the Reactor is backing off the reconnect time
         * on the second attempt. */

        int provLoginStreamId = provider.DefaultSessionLoginStreamId;
        for (int i = 0; i < 2; ++i)
        {
            /* Calculate delay time (doubles for each iteration) */
            expectedReconnectDelayTimeMs = reconnectMinDelay;
            for (int j = 0; j < i; ++j)
                expectedReconnectDelayTimeMs *= 2;

            /* Provider sends login closed-recover. */
            loginStatus.Clear();
            loginStatus.StreamId = provLoginStreamId;
            loginStatus.HasState = true;
            loginStatus.State.StreamState(StreamStates.CLOSED_RECOVER);
            loginStatus.State.DataState(DataStates.SUSPECT);
            submitOptions.Clear();

            Assert.True(provider.SubmitAndDispatch(loginStatus, submitOptions) >= ReactorReturnCode.SUCCESS);

            consumerReactor.Dispatch(i == 0 ? 4 : 2);

            /* Consumer receives open/suspect login status. */
            reactorEvent = consumerReactor.PollEvent();
            Assert.Equal(TestReactorEventType.LOGIN_MSG, reactorEvent.EventType);
            loginMsgEvent = (RDMLoginMsgEvent)reactorEvent.ReactorEvent;
            Assert.Equal(LoginMsgType.STATUS, loginMsgEvent.LoginMsg.LoginMsgType);
            recvLoginStatus = loginMsgEvent.LoginMsg.LoginStatus;
            Assert.Equal(consumer.DefaultSessionLoginStreamId, recvLoginStatus.StreamId);
            Assert.True(recvLoginStatus.HasState);
            Assert.Equal(StreamStates.OPEN, recvLoginStatus.State.StreamState());
            Assert.Equal(DataStates.SUSPECT, recvLoginStatus.State.DataState());

            /* Consumer receives channel-down event. */
            reactorEvent = consumerReactor.PollEvent();
            Assert.Equal(TestReactorEventType.CHANNEL_EVENT, reactorEvent.EventType);
            channelEvent = (ReactorChannelEvent)reactorEvent.ReactorEvent;
            Assert.Equal(ReactorChannelEventType.CHANNEL_DOWN_RECONNECTING, channelEvent.EventType);

            if (i == 0)
            {
                /* Consumer receives directory update. */
                reactorEvent = consumerReactor.PollEvent();
                Assert.Equal(TestReactorEventType.DIRECTORY_MSG, reactorEvent.EventType);
                directoryMsgEvent = (RDMDirectoryMsgEvent)reactorEvent.ReactorEvent;
                Assert.Equal(DirectoryMsgType.UPDATE, directoryMsgEvent.DirectoryMsg.DirectoryMsgType);
                recvDirectoryUpdate = directoryMsgEvent.DirectoryMsg.DirectoryUpdate;
                Assert.Equal(consumer.DefaultSessionDirectoryStreamId, recvDirectoryUpdate.StreamId);

                /* Consumer receives item status (Open vs. ClosedRecover, depending on single-open setting) */
                reactorEvent = consumerReactor.PollEvent();
                Assert.Equal(TestReactorEventType.MSG, reactorEvent.EventType);
                msgEvent = (ReactorMsgEvent)reactorEvent.ReactorEvent;
                recvStatusMsg = (IStatusMsg)msgEvent.Msg;
                Assert.Equal(5, recvStatusMsg.StreamId);
                Assert.True(recvStatusMsg.CheckHasState());
                Assert.Equal((singleOpen ? StreamStates.OPEN : StreamStates.CLOSED_RECOVER),
                              recvStatusMsg.State.StreamState());
            }

            startTimeMs = System.DateTime.Now;

            /* Provider receives channel-down event. */
            providerReactor.Dispatch(1);
            reactorEvent = providerReactor.PollEvent();
            Assert.Equal(TestReactorEventType.CHANNEL_EVENT, reactorEvent.EventType);
            channelEvent = (ReactorChannelEvent)reactorEvent.ReactorEvent;
            Assert.Equal(ReactorChannelEventType.CHANNEL_DOWN, channelEvent.EventType);

            provider.CloseChannel();

            /* Wait for the reconnect. */
            providerReactor.Accept(opts, provider, TimeSpan.FromMilliseconds(expectedReconnectDelayTimeMs + 1000));

            /* Consumer receives channel-up event (and should internally push out login request). */
            consumerReactor.Dispatch(1);
            reactorEvent = consumerReactor.PollEvent();
            Assert.Equal(TestReactorEventType.CHANNEL_EVENT, reactorEvent.EventType);
            channelEvent = (ReactorChannelEvent)reactorEvent.ReactorEvent;
            Assert.Equal(ReactorChannelEventType.CHANNEL_UP, channelEvent.EventType);

            /* Provider receives channel-up event. */
            providerReactor.Dispatch(3);
            reactorEvent = providerReactor.PollEvent();
            Assert.Equal(TestReactorEventType.CHANNEL_EVENT, reactorEvent.EventType);
            channelEvent = (ReactorChannelEvent)reactorEvent.ReactorEvent;
            Assert.Equal(ReactorChannelEventType.CHANNEL_UP, channelEvent.EventType);

            deviationTimeMs = (long)Math.Abs((reactorEvent.NanoTime - startTimeMs).TotalMilliseconds - expectedReconnectDelayTimeMs);
            Assert.True(deviationTimeMs < maxExpectedReconnectDelay, $"Reconnection delay off by {deviationTimeMs} ms.");

            /* Provider receives channel-ready event. */
            reactorEvent = providerReactor.PollEvent();
            Assert.Equal(TestReactorEventType.CHANNEL_EVENT, reactorEvent.EventType);
            channelEvent = (ReactorChannelEvent)reactorEvent.ReactorEvent;
            Assert.Equal(ReactorChannelEventType.CHANNEL_READY, channelEvent.EventType);

            /* Provider receives relogin. */
            reactorEvent = providerReactor.PollEvent();
            Assert.Equal(TestReactorEventType.LOGIN_MSG, reactorEvent.EventType);
            loginMsgEvent = (RDMLoginMsgEvent)reactorEvent.ReactorEvent;
            Assert.Equal(LoginMsgType.REQUEST, loginMsgEvent.LoginMsg.LoginMsgType);
            recvLoginRequest = loginMsgEvent.LoginMsg.LoginRequest;
            provLoginStreamId = recvLoginRequest.StreamId;
        }

        /* Do two loops of sending login refresh, THEN ClosedRecover status from the provider.
         * Make sure the Reactor is NOT backing off the reconnect time
         * on the second attempt. */
        for (int i = 0; i < 2; ++i)
        {
            /* Calculate delay time. */
            expectedReconnectDelayTimeMs = reconnectMinDelay;

            /* Provider sends login refresh. */
            loginRefresh.Clear();
            loginRefresh.Solicited = true;
            loginRefresh.StreamId = provLoginStreamId;
            loginRefresh.State.StreamState(StreamStates.OPEN);
            loginRefresh.State.DataState(DataStates.OK);
            submitOptions.Clear();
            Assert.True(provider.SubmitAndDispatch(loginRefresh, submitOptions) >= ReactorReturnCode.SUCCESS);

            /* Consumer receives login refresh .*/
            consumerReactor.Dispatch(1);
            reactorEvent = consumerReactor.PollEvent();

            Assert.Equal(TestReactorEventType.LOGIN_MSG, reactorEvent.EventType);
            loginMsgEvent = (RDMLoginMsgEvent)reactorEvent.ReactorEvent;
            Assert.Equal(LoginMsgType.REFRESH, loginMsgEvent.LoginMsg.LoginMsgType);
            recvLoginRefresh = loginMsgEvent.LoginMsg.LoginRefresh;
            Assert.Equal(consumer.DefaultSessionLoginStreamId, recvLoginRefresh.StreamId);
            Assert.True(recvLoginRefresh.Solicited);
            Assert.Equal(StreamStates.OPEN, recvLoginRefresh.State.StreamState());
            Assert.Equal(DataStates.OK, recvLoginRefresh.State.DataState());

            /* Provider receives directory request. */
            providerReactor.Dispatch(1);
            reactorEvent = providerReactor.PollEvent();

            Assert.Equal(TestReactorEventType.DIRECTORY_MSG, reactorEvent.EventType);
            directoryMsgEvent = (RDMDirectoryMsgEvent)reactorEvent.ReactorEvent;
            recvDirectoryRequest = directoryMsgEvent.DirectoryMsg.DirectoryRequest;
            Assert.Equal((int)DirectoryMsgType.REQUEST, recvDirectoryRequest.MsgClass);

            /* Provider sends login closed-recover. */
            loginStatus.Clear();
            loginStatus.StreamId = provLoginStreamId;
            loginStatus.HasState = true;
            loginStatus.State.StreamState(StreamStates.CLOSED_RECOVER);
            loginStatus.State.DataState(DataStates.SUSPECT);
            submitOptions.Clear();

            Assert.True(provider.SubmitAndDispatch(loginStatus, submitOptions) >= ReactorReturnCode.SUCCESS);
            consumerReactor.Dispatch(2);

            /* Consumer receives open/suspect login status. */
            reactorEvent = consumerReactor.PollEvent();
            Assert.Equal(TestReactorEventType.LOGIN_MSG, reactorEvent.EventType);
            loginMsgEvent = (RDMLoginMsgEvent)reactorEvent.ReactorEvent;
            Assert.Equal(LoginMsgType.STATUS, loginMsgEvent.LoginMsg.LoginMsgType);
            recvLoginStatus = loginMsgEvent.LoginMsg.LoginStatus;
            Assert.Equal(consumer.DefaultSessionLoginStreamId, recvLoginStatus.StreamId);
            Assert.True(recvLoginStatus.HasState);
            Assert.Equal(StreamStates.OPEN, recvLoginStatus.State.StreamState());
            Assert.Equal(DataStates.SUSPECT, recvLoginStatus.State.DataState());

            /* Consumer receives channel-down event. */
            reactorEvent = consumerReactor.PollEvent();
            Assert.Equal(TestReactorEventType.CHANNEL_EVENT, reactorEvent.EventType);
            channelEvent = (ReactorChannelEvent)reactorEvent.ReactorEvent;
            Assert.Equal(ReactorChannelEventType.CHANNEL_DOWN_RECONNECTING, channelEvent.EventType);

            startTimeMs = System.DateTime.Now;

            providerReactor.Dispatch(1);

            /* Provider receives channel-down event. */
            reactorEvent = providerReactor.PollEvent();
            Assert.Equal(TestReactorEventType.CHANNEL_EVENT, reactorEvent.EventType);
            channelEvent = (ReactorChannelEvent)reactorEvent.ReactorEvent;
            Assert.Equal(ReactorChannelEventType.CHANNEL_DOWN, channelEvent.EventType);

            provider.CloseChannel();

            /* Wait for the reconnect. */
            providerReactor.Accept(opts, provider, TimeSpan.FromMilliseconds(expectedReconnectDelayTimeMs + 1000));

            /* Consumer receives channel-upreactorEvent(and should internally push out login request). */
            consumerReactor.Dispatch(1);
            reactorEvent = consumerReactor.PollEvent();
            Assert.Equal(TestReactorEventType.CHANNEL_EVENT, reactorEvent.EventType);
            channelEvent = (ReactorChannelEvent)reactorEvent.ReactorEvent;
            Assert.Equal(ReactorChannelEventType.CHANNEL_UP, channelEvent.EventType);

            /* Provider receives channel-up event. */
            providerReactor.Dispatch(3);
            reactorEvent = providerReactor.PollEvent();
            Assert.Equal(TestReactorEventType.CHANNEL_EVENT, reactorEvent.EventType);
            channelEvent = (ReactorChannelEvent)reactorEvent.ReactorEvent;
            Assert.Equal(ReactorChannelEventType.CHANNEL_UP, channelEvent.EventType);

            deviationTimeMs = (long)Math.Abs((reactorEvent.NanoTime - startTimeMs).TotalMilliseconds - expectedReconnectDelayTimeMs);
            Assert.True(deviationTimeMs < maxExpectedReconnectDelay, $"Reconnection delay off by {deviationTimeMs} ms.");

            /* Provider receives channel-ready event. */
            reactorEvent = providerReactor.PollEvent();
            Assert.Equal(TestReactorEventType.CHANNEL_EVENT, reactorEvent.EventType);
            channelEvent = (ReactorChannelEvent)reactorEvent.ReactorEvent;
            Assert.Equal(ReactorChannelEventType.CHANNEL_READY, channelEvent.EventType);

            /* Provider receives relogin. */
            reactorEvent = providerReactor.PollEvent();
            Assert.Equal(TestReactorEventType.LOGIN_MSG, reactorEvent.EventType);
            loginMsgEvent = (RDMLoginMsgEvent)reactorEvent.ReactorEvent;
            Assert.Equal(LoginMsgType.REQUEST, loginMsgEvent.LoginMsg.LoginMsgType);
            recvLoginRequest = loginMsgEvent.LoginMsg.LoginRequest;
            provLoginStreamId = recvLoginRequest.StreamId;
        }

        /*** Same test as above, but with ClosedRecover Login Refresh instead of status. ***/

        /* Send a login refresh to establish the stream (and reset the reconnect timer). */
        loginRefresh.Clear();
        loginRefresh.Solicited = true;
        loginRefresh.StreamId = provLoginStreamId;
        loginRefresh.State.StreamState(StreamStates.OPEN);
        loginRefresh.State.DataState(DataStates.OK);
        submitOptions.Clear();
        Assert.True(provider.SubmitAndDispatch(loginRefresh, submitOptions) >= ReactorReturnCode.SUCCESS);

        /* Consumer receives login refresh .*/
        consumerReactor.Dispatch(1);
        reactorEvent = consumerReactor.PollEvent();

        Assert.Equal(TestReactorEventType.LOGIN_MSG, reactorEvent.EventType);
        loginMsgEvent = (RDMLoginMsgEvent)reactorEvent.ReactorEvent;
        Assert.Equal(LoginMsgType.REFRESH, loginMsgEvent.LoginMsg.LoginMsgType);
        recvLoginRefresh = loginMsgEvent.LoginMsg.LoginRefresh;
        Assert.Equal(consumer.DefaultSessionLoginStreamId, recvLoginRefresh.StreamId);
        Assert.True(recvLoginRefresh.Solicited);
        Assert.Equal(StreamStates.OPEN, recvLoginRefresh.State.StreamState());
        Assert.Equal(DataStates.OK, recvLoginRefresh.State.DataState());

        /* Provider receives directory request. */
        providerReactor.Dispatch(1);
        reactorEvent = providerReactor.PollEvent();
        Assert.Equal(TestReactorEventType.DIRECTORY_MSG, reactorEvent.EventType);
        directoryMsgEvent = (RDMDirectoryMsgEvent)reactorEvent.ReactorEvent;
        recvDirectoryRequest = (DirectoryRequest)directoryMsgEvent.DirectoryMsg.DirectoryRequest;
        Assert.Equal((int)DirectoryMsgType.REQUEST, recvDirectoryRequest.MsgClass);

        /* Do two loops of sending login ClosedRecover from the provider.
         * Make sure the Reactor is backing off the reconnect time
         * on the second attempt. */

        for (int i = 0; i < 2; ++i)
        {
            /* Calculate delay time (doubles for each iteration) */
            expectedReconnectDelayTimeMs = reconnectMinDelay;
            for (int j = 0; j < i; ++j)
                expectedReconnectDelayTimeMs *= 2;

            /* Provider sends login closed-recover. */
            loginRefresh.Clear();
            loginRefresh.StreamId = provLoginStreamId;
            loginRefresh.State.StreamState(StreamStates.CLOSED_RECOVER);
            loginRefresh.State.DataState(DataStates.SUSPECT);
            submitOptions.Clear();
            Assert.True(provider.SubmitAndDispatch(loginRefresh, submitOptions) >= ReactorReturnCode.SUCCESS);

            consumerReactor.Dispatch(2);

            /* Consumer receives open/suspect login refresh. */
            reactorEvent = consumerReactor.PollEvent();
            Assert.Equal(TestReactorEventType.LOGIN_MSG, reactorEvent.EventType);
            loginMsgEvent = (RDMLoginMsgEvent)reactorEvent.ReactorEvent;
            Assert.Equal(LoginMsgType.REFRESH, loginMsgEvent.LoginMsg.LoginMsgType);
            recvLoginRefresh = loginMsgEvent.LoginMsg.LoginRefresh;
            Assert.Equal(consumer.DefaultSessionLoginStreamId, recvLoginRefresh.StreamId);
            Assert.Equal(StreamStates.OPEN, recvLoginRefresh.State.StreamState());
            Assert.Equal(DataStates.SUSPECT, recvLoginRefresh.State.DataState());

            /* Consumer receives channel-down event. */
            reactorEvent = consumerReactor.PollEvent();
            Assert.Equal(TestReactorEventType.CHANNEL_EVENT, reactorEvent.EventType);
            channelEvent = (ReactorChannelEvent)reactorEvent.ReactorEvent;
            Assert.Equal(ReactorChannelEventType.CHANNEL_DOWN_RECONNECTING, channelEvent.EventType);

            startTimeMs = System.DateTime.Now;

            /* Provider receives channel-down event. */
            providerReactor.Dispatch(1);
            reactorEvent = providerReactor.PollEvent();
            Assert.Equal(TestReactorEventType.CHANNEL_EVENT, reactorEvent.EventType);
            channelEvent = (ReactorChannelEvent)reactorEvent.ReactorEvent;
            Assert.Equal(ReactorChannelEventType.CHANNEL_DOWN, channelEvent.EventType);

            provider.CloseChannel();

            /* Wait for the reconnect. */
            providerReactor.Accept(opts, provider, TimeSpan.FromMilliseconds(expectedReconnectDelayTimeMs + 1000));

            /* Consumer receives channel-upreactorEvent(and should internally push out login request). */
            consumerReactor.Dispatch(1);
            reactorEvent = consumerReactor.PollEvent();
            Assert.Equal(TestReactorEventType.CHANNEL_EVENT, reactorEvent.EventType);
            channelEvent = (ReactorChannelEvent)reactorEvent.ReactorEvent;
            Assert.Equal(ReactorChannelEventType.CHANNEL_UP, channelEvent.EventType);

            /* Provider receives channel-up event. */
            providerReactor.Dispatch(3);

            reactorEvent = providerReactor.PollEvent();
            Assert.Equal(TestReactorEventType.CHANNEL_EVENT, reactorEvent.EventType);
            channelEvent = (ReactorChannelEvent)reactorEvent.ReactorEvent;
            Assert.Equal(ReactorChannelEventType.CHANNEL_UP, channelEvent.EventType);

            deviationTimeMs = (long)Math.Abs((reactorEvent.NanoTime - startTimeMs).TotalMilliseconds - expectedReconnectDelayTimeMs);
            Assert.True(deviationTimeMs < maxExpectedReconnectDelay, $"Reconnection delay off by {deviationTimeMs} ms.");

            /* Provider receives channel-ready event. */
            reactorEvent = providerReactor.PollEvent();
            Assert.Equal(TestReactorEventType.CHANNEL_EVENT, reactorEvent.EventType);
            channelEvent = (ReactorChannelEvent)reactorEvent.ReactorEvent;
            Assert.Equal(ReactorChannelEventType.CHANNEL_READY, channelEvent.EventType);

            /* Provider receives relogin. */
            reactorEvent = providerReactor.PollEvent();
            Assert.Equal(TestReactorEventType.LOGIN_MSG, reactorEvent.EventType);
            loginMsgEvent = (RDMLoginMsgEvent)reactorEvent.ReactorEvent;
            Assert.Equal(LoginMsgType.REQUEST, loginMsgEvent.LoginMsg.LoginMsgType);
            recvLoginRequest = loginMsgEvent.LoginMsg.LoginRequest;
            provLoginStreamId = recvLoginRequest.StreamId;
        }

        /* Do two loops of sending login refresh, THEN ClosedRecover from the provider.
         * Make sure the Reactor is NOT backing off the reconnect time
         * on the second attempt. */
        for (int i = 0; i < 2; ++i)
        {
            /* Calculate delay time. */
            expectedReconnectDelayTimeMs = reconnectMinDelay;

            /* Provider sends login refresh. */
            loginRefresh.Clear();
            loginRefresh.Solicited = true;
            loginRefresh.StreamId = provLoginStreamId;
            loginRefresh.State.StreamState(StreamStates.OPEN);
            loginRefresh.State.DataState(DataStates.OK);
            submitOptions.Clear();
            Assert.True(provider.SubmitAndDispatch(loginRefresh, submitOptions) >= ReactorReturnCode.SUCCESS);

            /* Consumer receives login refresh .*/
            consumerReactor.Dispatch(1);
            reactorEvent = consumerReactor.PollEvent();
            Assert.Equal(TestReactorEventType.LOGIN_MSG, reactorEvent.EventType);
            loginMsgEvent = (RDMLoginMsgEvent)reactorEvent.ReactorEvent;
            Assert.Equal(LoginMsgType.REFRESH, loginMsgEvent.LoginMsg.LoginMsgType);
            recvLoginRefresh = loginMsgEvent.LoginMsg.LoginRefresh;
            Assert.Equal(consumer.DefaultSessionLoginStreamId, recvLoginRefresh.StreamId);
            Assert.True(recvLoginRefresh.Solicited);
            Assert.Equal(StreamStates.OPEN, recvLoginRefresh.State.StreamState());
            Assert.Equal(DataStates.OK, recvLoginRefresh.State.DataState());

            /* Provider receives directory request. */
            providerReactor.Dispatch(1);
            reactorEvent = providerReactor.PollEvent();
            Assert.Equal(TestReactorEventType.DIRECTORY_MSG, reactorEvent.EventType);
            directoryMsgEvent = (RDMDirectoryMsgEvent)reactorEvent.ReactorEvent;
            recvDirectoryRequest = directoryMsgEvent.DirectoryMsg.DirectoryRequest;
            // Assert.Equal(DirectoryMsgType.REQUEST, recvDirectoryRequest.rdmMsgType());

            /* Provider sends login closed-recover. */
            loginRefresh.Clear();
            loginRefresh.StreamId = provLoginStreamId;
            loginRefresh.State.StreamState(StreamStates.CLOSED_RECOVER);
            loginRefresh.State.DataState(DataStates.SUSPECT);
            submitOptions.Clear();
            Assert.True(provider.SubmitAndDispatch(loginRefresh, submitOptions) >= ReactorReturnCode.SUCCESS);

            consumerReactor.Dispatch(2);

            /* Consumer receives open/suspect login refresh. */
            reactorEvent = consumerReactor.PollEvent();
            Assert.Equal(TestReactorEventType.LOGIN_MSG, reactorEvent.EventType);
            loginMsgEvent = (RDMLoginMsgEvent)reactorEvent.ReactorEvent;
            Assert.Equal(LoginMsgType.REFRESH, loginMsgEvent.LoginMsg.LoginMsgType);
            recvLoginRefresh = loginMsgEvent.LoginMsg.LoginRefresh;
            Assert.Equal(consumer.DefaultSessionLoginStreamId, recvLoginRefresh.StreamId);
            Assert.Equal(StreamStates.OPEN, recvLoginRefresh.State.StreamState());
            Assert.Equal(DataStates.SUSPECT, recvLoginRefresh.State.DataState());

            /* Consumer receives channel-down event. */
            reactorEvent = consumerReactor.PollEvent();
            Assert.Equal(TestReactorEventType.CHANNEL_EVENT, reactorEvent.EventType);
            channelEvent = (ReactorChannelEvent)reactorEvent.ReactorEvent;
            Assert.Equal(ReactorChannelEventType.CHANNEL_DOWN_RECONNECTING, channelEvent.EventType);

            startTimeMs = System.DateTime.Now;

            providerReactor.Dispatch(1);

            /* Provider receives channel-down event. */
            reactorEvent = providerReactor.PollEvent();
            Assert.Equal(TestReactorEventType.CHANNEL_EVENT, reactorEvent.EventType);
            channelEvent = (ReactorChannelEvent)reactorEvent.ReactorEvent;
            Assert.Equal(ReactorChannelEventType.CHANNEL_DOWN, channelEvent.EventType);

            provider.CloseChannel();

            /* Wait for the reconnect. */
            providerReactor.Accept(opts, provider, TimeSpan.FromMilliseconds(expectedReconnectDelayTimeMs + 1000));

            /* Consumer receives channel-upreactorEvent(and should internally push out login request). */
            consumerReactor.Dispatch(1);
            reactorEvent = consumerReactor.PollEvent();
            Assert.Equal(TestReactorEventType.CHANNEL_EVENT, reactorEvent.EventType);
            channelEvent = (ReactorChannelEvent)reactorEvent.ReactorEvent;
            Assert.Equal(ReactorChannelEventType.CHANNEL_UP, channelEvent.EventType);

            /* Provider receives channel-up event. */
            providerReactor.Dispatch(3);
            reactorEvent = providerReactor.PollEvent();
            Assert.Equal(TestReactorEventType.CHANNEL_EVENT, reactorEvent.EventType);
            channelEvent = (ReactorChannelEvent)reactorEvent.ReactorEvent;
            Assert.Equal(ReactorChannelEventType.CHANNEL_UP, channelEvent.EventType);

            deviationTimeMs = (long)Math.Abs((reactorEvent.NanoTime - startTimeMs).TotalMilliseconds - expectedReconnectDelayTimeMs);
            Assert.True(deviationTimeMs < maxExpectedReconnectDelay, $"Reconnection delay off by {deviationTimeMs} ms.");

            /* Provider receives channel-ready event. */
            reactorEvent = providerReactor.PollEvent();
            Assert.Equal(TestReactorEventType.CHANNEL_EVENT, reactorEvent.EventType);
            channelEvent = (ReactorChannelEvent)reactorEvent.ReactorEvent;
            Assert.Equal(ReactorChannelEventType.CHANNEL_READY, channelEvent.EventType);

            /* Provider receives relogin. */
            reactorEvent = providerReactor.PollEvent();
            Assert.Equal(TestReactorEventType.LOGIN_MSG, reactorEvent.EventType);
            loginMsgEvent = (RDMLoginMsgEvent)reactorEvent.ReactorEvent;
            Assert.Equal(LoginMsgType.REQUEST, loginMsgEvent.LoginMsg.LoginMsgType);
            recvLoginRequest = loginMsgEvent.LoginMsg.LoginRequest;
            provLoginStreamId = recvLoginRequest.StreamId;
        }

        TestReactorComponent.CloseSession(consumer, provider);
        TearDownConsumerAndProvider(consumerReactor, providerReactor);
    }

    [Fact]
    [Category("Unit")]
    [Category("Reactor")]
    public void LoginCloseFromCallbackTest_Socket()
    {
        LoginCloseFromCallback(false, null);
    }

    private void LoginCloseFromCallback(bool isWebsocket, string protocolList)
    {

        for (int i = 0; i < 2; ++i)
        {
            ReactorSubmitOptions submitOptions = new ReactorSubmitOptions();
            TestReactorEvent reactorEvent;
            RDMLoginMsgEvent loginMsgEvent;
            LoginStatus receivedLoginStatus;
            int provLoginStreamId;

            /* Create reactors. */
            TestReactor consumerReactor = new TestReactor();
            TestReactor providerReactor = new TestReactor();

            /* Create consumer. */
            Consumer consumer = new CloseLoginStreamFromCallbackConsumer(consumerReactor);
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
            Provider provider = new Provider(providerReactor);
            ProviderRole providerRole = (ProviderRole)provider.ReactorRole;
            providerRole.ChannelEventCallback = provider;
            providerRole.LoginMsgCallback = provider;
            providerRole.DirectoryMsgCallback = provider;
            providerRole.DictionaryMsgCallback = provider;
            providerRole.DefaultMsgCallback = provider;

            /* Connect the consumer and provider. */
            ConsumerProviderSessionOptions opts = new ConsumerProviderSessionOptions();
            opts.SetupDefaultLoginStream = false;
            opts.SetupDefaultDirectoryStream = false;
            opts.NumStatusEvents = 1;

            provider.Bind(opts);

            TestReactor.OpenSession(consumer, provider, opts);

            /* Provider receives login request. */
            provider.TestReactor.Dispatch(1);
            reactorEvent = provider.TestReactor.PollEvent();
            Assert.Equal(TestReactorEventType.LOGIN_MSG, reactorEvent.EventType);
            loginMsgEvent = (RDMLoginMsgEvent)reactorEvent.ReactorEvent;
            Assert.Equal(LoginMsgType.REQUEST, loginMsgEvent.LoginMsg.LoginMsgType);
            LoginRequest receivedLoginRequest = loginMsgEvent.LoginMsg.LoginRequest;
            provLoginStreamId = receivedLoginRequest.StreamId;

            /* Provider sends login refresh. */
            LoginStatus loginStatus = new LoginStatus();

            loginStatus.Clear();
            loginStatus.HasState = true;
            loginStatus.StreamId = provLoginStreamId;

            if (i == 0)
                loginStatus.State.StreamState(StreamStates.OPEN);
            else
                loginStatus.State.StreamState(StreamStates.CLOSED);

            loginStatus.State.DataState(DataStates.SUSPECT);
            loginStatus.State.Code(StateCodes.NOT_ENTITLED);
            loginStatus.State.Text().Data("Not permissioned.");

            submitOptions.Clear();
            Assert.True(provider.SubmitAndDispatch(loginStatus, submitOptions) >= ReactorReturnCode.SUCCESS);

            /* Consumer receives login status. */
            consumerReactor.Dispatch(1);
            reactorEvent = consumerReactor.PollEvent();
            Assert.Equal(TestReactorEventType.LOGIN_MSG, reactorEvent.EventType);
            loginMsgEvent = (RDMLoginMsgEvent)reactorEvent.ReactorEvent;
            Assert.Equal(LoginMsgType.STATUS, loginMsgEvent.LoginMsg.LoginMsgType);
            receivedLoginStatus = loginMsgEvent.LoginMsg.LoginStatus;
            Assert.Equal(consumerRole.RdmLoginRequest.StreamId, receivedLoginStatus.StreamId);
            Assert.True(receivedLoginStatus.HasState);

            if (i == 0)
                Assert.Equal(StreamStates.OPEN, receivedLoginStatus.State.StreamState());
            else
                Assert.Equal(StreamStates.CLOSED, receivedLoginStatus.State.StreamState());

            Assert.Equal(DataStates.SUSPECT, receivedLoginStatus.State.DataState());

            /* Provider receives login close if stream was was open. */
            providerReactor.Dispatch(1);
            reactorEvent = providerReactor.PollEvent();
            Assert.Equal(TestReactorEventType.LOGIN_MSG, reactorEvent.EventType);
            loginMsgEvent = (RDMLoginMsgEvent)reactorEvent.ReactorEvent;
            Assert.Equal(LoginMsgType.CLOSE, loginMsgEvent.LoginMsg.LoginMsgType);
            Assert.Equal(provLoginStreamId, loginMsgEvent.LoginMsg.StreamId);

            TestReactorComponent.CloseSession(consumer, provider);
            TearDownConsumerAndProvider(consumerReactor, providerReactor);
        }
    }

    /* Used by closeFromCallbackTest. */
    class CloseLoginStreamFromCallbackConsumer : Consumer
    {
        public CloseLoginStreamFromCallbackConsumer(TestReactor testReactor) : base(testReactor)
        { }

        public override ReactorCallbackReturnCode ReactorChannelEventCallback(ReactorChannelEvent evt)
        {
            ReactorSubmitOptions submitOptions = new ReactorSubmitOptions();
            Msg msg = new Msg();
            IRequestMsg requestMsg = (IRequestMsg)msg;

            base.ReactorChannelEventCallback(evt);

            if (evt.EventType == ReactorChannelEventType.CHANNEL_OPENED)
            {
                /* Consumer sends item request. */
                requestMsg.Clear();
                requestMsg.MsgClass = MsgClasses.REQUEST;
                requestMsg.StreamId = 6;
                requestMsg.DomainType = (int)DomainType.MARKET_PRICE;
                requestMsg.ApplyStreaming();
                requestMsg.MsgKey.ApplyHasName();
                requestMsg.MsgKey.Name.Data("TRI.N");

                submitOptions.Clear();
                submitOptions.ServiceName = Provider.DefaultService.Info.ServiceName.ToString();
                Assert.True(Submit((Msg)requestMsg, submitOptions) >= ReactorReturnCode.SUCCESS);
            }

            return ReactorCallbackReturnCode.SUCCESS;
        }

        public override ReactorCallbackReturnCode RdmLoginMsgCallback(RDMLoginMsgEvent evt)
        {
            ICloseMsg closeMsg = (ICloseMsg)new Msg();
            ReactorSubmitOptions submitOptions = new ReactorSubmitOptions();

            base.RdmLoginMsgCallback(evt);

            Assert.Equal(LoginMsgType.STATUS, evt.LoginMsg.LoginMsgType);
            closeMsg.Clear();
            closeMsg.MsgClass = MsgClasses.CLOSE;
            closeMsg.StreamId = 1;
            closeMsg.DomainType = (int)DomainType.LOGIN;
            submitOptions.Clear();
            Assert.True(Submit((Msg)closeMsg, submitOptions) >= ReactorReturnCode.SUCCESS);

            return ReactorCallbackReturnCode.SUCCESS;
        }
    }

    [Fact]
    [Category("Unit")]
    [Category("Reactor")]
    public void LoginReissue_Scenario_A_Test()
    {
        string test = "LoginReissue_Scenario_A_Test()";
        output.WriteLine("\n" + test + " Running...");
        output.WriteLine("/*   CONS                 WatchList                 PROV\n" +
                         "   1) |    Cons Request[0] -> |                       |\n" +
                         "   2) |                       |    Prov Request[0] -> |\n" +
                         "   3) |    Cons Request[1] -> |                       |\n" +
                         "   4) |                       | <- Prov Refresh[0]    |\n" +
                         "   5) | <- Cons Refresh[0]    |                       |\n" +
                         "   6) |                       |    Prov Request[1] -> |\n" +
                         "   7) |                       | <- Prov Refresh[1]    |\n" +
                         "   8) | <- Cons Refresh[1]    |                       |\n" +
                         "*/");

        int loginStreamId;
        TestReactorEvent reactorEvent;
        RDMLoginMsgEvent loginMsgEvent;
        ReactorSubmitOptions submitOptions = new ReactorSubmitOptions();

        LoginRequest[] consRequest = { null, null };
        LoginRefresh[] consRefresh = { null, null };
        LoginRequest[] provRequest = { null, null };
        LoginRefresh[] provRefresh = { null, null };

        // Data arrays - index [0] is used for the initial request and refresh, the other
        // indices are for the subsequent reissue requests and refreshes.
        string[] userNames = { "userName_0", "userName_1" };
        string[] authenticationTokens = { "authenticationToken_0", "authenticationToken_1" };
        string[] authenticationExts = { "authenticationExt_0", "authenticationExt_1" };
        string[] authenticationExtResps = { "authenticationExtResp_0", "authenticationExtResp_1", };
        long[] authenticationTTReissues = { 123123000, 123123001 };

        Login.UserIdTypes[] userNameTypes = { Login.UserIdTypes.NAME, Login.UserIdTypes.EMAIL_ADDRESS,
            Login.UserIdTypes.TOKEN, Login.UserIdTypes.COOKIE, Login.UserIdTypes.AUTHN_TOKEN
        };

        foreach (Login.UserIdTypes userNameType in userNameTypes)
        {
            output.WriteLine($"{test} loop: userNameType = " + userNameType);

            // Create reactors
            TestReactor consumerReactor = new TestReactor();
            TestReactor providerReactor = new TestReactor();
            consumerReactor.Reactor.m_ReactorOptions.XmlTracing = true;
            providerReactor.Reactor.m_ReactorOptions.XmlTracing = true;

            // Create consumer and the initial login request message with data using index [0]
            Consumer consumer = new Consumer(consumerReactor);
            ConsumerRole consumerRole = (ConsumerRole)consumer.ReactorRole;
            consumerRole.InitDefaultRDMLoginRequest();
            consumerRole.InitDefaultRDMDirectoryRequest();

            output.WriteLine($"{test} 1) Consumer creating login request[0]");
            consRequest[0] = consumerRole.RdmLoginRequest;
            loginStreamId = consRequest[0].StreamId;
            consRequest[0].HasUserNameType = true;
            consRequest[0].UserNameType = userNameType;

            if (userNameType == Login.UserIdTypes.AUTHN_TOKEN)
            {
                consRequest[0].UserName.Data(authenticationTokens[0]);
                consRequest[0].HasAuthenticationExtended = true;
                consRequest[0].AuthenticationExtended.Data(authenticationExts[0]);
            }
            else
            {
                consRequest[0].UserName.Data(userNames[0]);
            }

            consumerRole.ChannelEventCallback = consumer;
            consumerRole.LoginMsgCallback = consumer;
            consumerRole.DirectoryMsgCallback = consumer;
            consumerRole.DictionaryMsgCallback = consumer;
            consumerRole.DefaultMsgCallback = consumer;
            consumerRole.WatchlistOptions.EnableWatchlist = true;
            consumerRole.WatchlistOptions.ChannelOpenEventCallback = consumer;

            // Create provider.
            Provider provider = new Provider(providerReactor);
            ProviderRole providerRole = (ProviderRole)provider.ReactorRole;
            providerRole.ChannelEventCallback = provider;
            providerRole.LoginMsgCallback = provider;
            providerRole.DirectoryMsgCallback = provider;
            providerRole.DictionaryMsgCallback = provider;
            providerRole.DefaultMsgCallback = provider;

            // Connect the consumer and provider. Disable the automatic setup of login & directory streams.
            ConsumerProviderSessionOptions opts = new ConsumerProviderSessionOptions();
            opts.SetupDefaultLoginStream = false;
            opts.SetupDefaultDirectoryStream = false;
            provider.Bind(opts);
            output.WriteLine($"{test} 1) Consumer sending login request[0]");
            TestReactor.OpenSession(consumer, provider, opts);
            output.WriteLine($"{test} 1) Consumer sent login request[0]");

            output.WriteLine($"{test} 2) Provider dispatching, expects login request[0]");
            provider.TestReactor.Dispatch(1);
            reactorEvent = provider.TestReactor.PollEvent();
            Assert.Equal(TestReactorEventType.LOGIN_MSG, reactorEvent.EventType);
            loginMsgEvent = (RDMLoginMsgEvent)reactorEvent.ReactorEvent;
            Assert.Equal(LoginMsgType.REQUEST, loginMsgEvent.LoginMsg.LoginMsgType);
            output.WriteLine($"{test} 2) Provider received login request[0]");

            provRequest[0] = loginMsgEvent.LoginMsg.LoginRequest;
            Assert.Equal(provRequest[0].StreamId, loginStreamId);
            Assert.True(provRequest[0].HasUserNameType);
            Assert.Equal(provRequest[0].UserNameType, userNameType);

            if (userNameType == Login.UserIdTypes.AUTHN_TOKEN)
            {
                Assert.Equal(provRequest[0].UserName.ToString(), authenticationTokens[0]);
                Assert.True(provRequest[0].HasAuthenticationExtended);
                Assert.Equal(provRequest[0].AuthenticationExtended.ToString(), authenticationExts[0]);
            }
            else
                Assert.Equal(provRequest[0].UserName.ToString(), userNames[0]);
            output.WriteLine($"{test} 2) Provider validated login request[0]");

            output.WriteLine($"{test} 3) Consumer creating login request[1]");
            consumerRole.InitDefaultRDMLoginRequest();
            consRequest[1] = consumerRole.RdmLoginRequest;
            Assert.NotNull(consRequest[1]);
            consRequest[1].HasUserNameType = true;
            consRequest[1].UserNameType = userNameType;
            if (userNameType == Login.UserIdTypes.AUTHN_TOKEN)
            {
                consRequest[1].UserName.Data(authenticationTokens[1]);
                consRequest[1].HasAuthenticationExtended = true;
                consRequest[1].AuthenticationExtended.Data(authenticationExts[1]);
            }
            else if (userNameType == Login.UserIdTypes.TOKEN)
                consRequest[1].UserName.Data(userNames[1]);
            else
                consRequest[1].UserName.Data(userNames[0]);

            output.WriteLine($"{test} 3) Consumer sending login request[1]");
            submitOptions.Clear();
            Assert.True(consumer.SubmitAndDispatch(consRequest[1], submitOptions) >= ReactorReturnCode.SUCCESS);
            output.WriteLine($"{test} 3) Consumer sent login request[1]");

            provider.TestReactor.Dispatch(0);
            output.WriteLine($"{test} 3.1) Confirmed Watchlist did not send request[1] before receiving refresh[0]");

            output.WriteLine($"{test} 4) Provider creating login refresh[0]");
            provRefresh[0] = new LoginRefresh();
            provRefresh[0].StreamId = loginStreamId;
            provRefresh[0].Solicited = true;
            provRefresh[0].HasUserNameType = true;
            provRefresh[0].UserNameType = userNameType;
            if (userNameType == Login.UserIdTypes.AUTHN_TOKEN)
            {
                provRefresh[0].HasUserName = true;
                provRefresh[0].UserName.Data(userNames[0]);
                provRefresh[0].HasAuthenicationTTReissue = true;
                provRefresh[0].AuthenticationTTReissue = authenticationTTReissues[0];
                provRefresh[0].HasAuthenticationExtendedResp = true;
                provRefresh[0].AuthenticationExtendedResp.Data(authenticationExtResps[0]);
            }
            else
            {
                provRefresh[0].HasUserName = true;
                provRefresh[0].UserName.Data(userNames[0]);
            }
            provRefresh[0].State.StreamState(StreamStates.OPEN);
            provRefresh[0].State.DataState(DataStates.OK);

            output.WriteLine($"{test} 4) Provider sending login refresh[0]");
            submitOptions.Clear();
            Assert.True(provider.SubmitAndDispatch(provRefresh[0], submitOptions) >= ReactorReturnCode.SUCCESS);
            output.WriteLine($"{test} 4) Provider sent login refresh[0]: {provRefresh[0]}");

            output.WriteLine($"{test} 5) Consumer dispatching, expects login refresh[0] (if so, will send Directory request)");
            consumer.TestReactor.Dispatch(1);
            reactorEvent = consumer.TestReactor.PollEvent();
            Assert.Equal(TestReactorEventType.LOGIN_MSG, reactorEvent.EventType);
            loginMsgEvent = (RDMLoginMsgEvent)reactorEvent.ReactorEvent;
            Assert.Equal(LoginMsgType.REFRESH, loginMsgEvent.LoginMsg.LoginMsgType);
            Assert.Equal(MsgClasses.REFRESH, loginMsgEvent.Msg.MsgClass);
            output.WriteLine($"{test} 5) Consumer received login refresh[0]");

            consRefresh[0] = loginMsgEvent.LoginMsg.LoginRefresh;
            output.WriteLine($"refresh: {consRefresh[0]}");
            Assert.Equal(consRefresh[0].StreamId, loginStreamId);
            Assert.Equal(StreamStates.OPEN, consRefresh[0].State.StreamState());
            Assert.Equal(DataStates.OK, consRefresh[0].State.DataState());
            Assert.True(consRefresh[0].HasUserName); // ASSERTION FAIL
            Assert.True(consRefresh[0].HasUserNameType);
            if (userNameType == Login.UserIdTypes.AUTHN_TOKEN)
            {
                Assert.True(consRefresh[0].UserName.Data().ReadByte() == 0x0);
                Assert.True(consRefresh[0].HasAuthenicationTTReissue);
                Assert.Equal(consRefresh[0].AuthenticationTTReissue, authenticationTTReissues[0]);
                Assert.True(consRefresh[0].HasAuthenticationExtendedResp);
                Assert.Equal(consRefresh[0].AuthenticationExtendedResp.ToString(), authenticationExtResps[0]);
            }
            else
                Assert.Equal(consRefresh[0].UserName.ToString(), userNames[0]);
            output.WriteLine($"{test} 5) Consumer validated login refresh[0]");

            output.WriteLine($"{test} 6) Provider dispatching, expects Login request[1] and Directory request[1]");
            provider.TestReactor.Dispatch(2);
            // validate Login request[1]
            reactorEvent = providerReactor.PollEvent();
            Assert.Equal(TestReactorEventType.LOGIN_MSG, reactorEvent.EventType);
            loginMsgEvent = (RDMLoginMsgEvent)reactorEvent.ReactorEvent;
            Assert.Equal(LoginMsgType.REQUEST, loginMsgEvent.LoginMsg.LoginMsgType);
            LoginRequest loginRequest = loginMsgEvent.LoginMsg.LoginRequest;
            Assert.Equal(loginRequest.StreamId, loginStreamId);
            Assert.True(loginRequest.HasUserNameType);
            Assert.Equal(loginRequest.UserNameType, userNameType);
            if (userNameType == Login.UserIdTypes.AUTHN_TOKEN)
            {
                Assert.Equal(loginRequest.UserName.ToString(), authenticationTokens[1]);
                Assert.True(loginRequest.HasAuthenticationExtended);
                Assert.Equal(loginRequest.AuthenticationExtended.ToString(), authenticationExts[1]);
            }
            else if (userNameType == Login.UserIdTypes.TOKEN)
                Assert.Equal(loginRequest.UserName.ToString(), userNames[1]);
            else
                Assert.Equal(loginRequest.UserName.ToString(), userNames[0]);
            output.WriteLine($"{test} 6) Provider validated login request[1]");

            // validate Directory request[1]
            reactorEvent = provider.TestReactor.PollEvent();
            Assert.Equal(TestReactorEventType.DIRECTORY_MSG, reactorEvent.EventType);
            output.WriteLine($"{test} 6) Provider received Directory request[1]");
            output.WriteLine($"{test} 6) Provider does not validate Directory request[1]");

            output.WriteLine($"{test} 7) Prov creating login refresh[1]");
            provRefresh[1] = new LoginRefresh();
            provRefresh[1].Clear();
            provRefresh[1].StreamId = loginStreamId;
            provRefresh[1].Solicited = true;
            provRefresh[1].HasUserNameType = true;
            provRefresh[1].UserNameType = userNameType;
            if (userNameType == Login.UserIdTypes.AUTHN_TOKEN)
            {
                provRefresh[1].UserName.Data(userNames[1]);
                provRefresh[1].HasAuthenicationTTReissue = true;
                provRefresh[1].AuthenticationTTReissue = authenticationTTReissues[1];
                provRefresh[1].HasAuthenticationExtendedResp = true;
                provRefresh[1].AuthenticationExtendedResp.Data(authenticationExtResps[1]);
            }
            else
            {
                provRefresh[1].HasUserName = true;
                provRefresh[1].UserName.Data(userNames[1]);
            }
            provRefresh[1].State.StreamState(StreamStates.OPEN);
            provRefresh[1].State.DataState(DataStates.OK);

            output.WriteLine($"{test} 7) Prov sending login refresh[1]");
            submitOptions.Clear();
            Assert.True(provider.SubmitAndDispatch(provRefresh[1], submitOptions) >= ReactorReturnCode.SUCCESS);
            output.WriteLine($"{test} 7) Prov sent login refresh[1]");

            output.WriteLine($"{test} 8) Consumer dispatching, expects login refresh[1]");
            consumer.TestReactor.Dispatch(1);
            reactorEvent = consumer.TestReactor.PollEvent();
            Assert.Equal(TestReactorEventType.LOGIN_MSG, reactorEvent.EventType);
            loginMsgEvent = (RDMLoginMsgEvent)reactorEvent.ReactorEvent;
            Assert.Equal(LoginMsgType.REFRESH, loginMsgEvent.LoginMsg.LoginMsgType);
            Assert.Equal(MsgClasses.REFRESH, loginMsgEvent.Msg.MsgClass);
            output.WriteLine($"{test} 8) Consumer received login refresh[1]");

            consRefresh[1] = loginMsgEvent.LoginMsg.LoginRefresh;
            Assert.Equal(consRefresh[1].StreamId, loginStreamId);
            Assert.Equal(StreamStates.OPEN, consRefresh[1].State.StreamState());
            Assert.Equal(DataStates.OK, consRefresh[1].State.DataState());
            Assert.True(consRefresh[1].HasUserName);
            Assert.True(consRefresh[1].HasUserNameType);
            if (userNameType == Login.UserIdTypes.AUTHN_TOKEN)
            {
                Assert.True(consRefresh[1].UserName.Data().ReadByte() == 0x0);
                Assert.True(consRefresh[1].HasAuthenicationTTReissue);
                Assert.Equal(consRefresh[1].AuthenticationTTReissue, authenticationTTReissues[1]);
                Assert.True(consRefresh[1].HasAuthenticationExtendedResp);
                Assert.Equal(consRefresh[1].AuthenticationExtendedResp.ToString(), authenticationExtResps[1]);
            }
            else
                Assert.Equal(consRefresh[1].UserName.ToString(), userNames[1]);
            output.WriteLine($"{test} 8) Consumer validated login refresh[1]");
        }
        output.WriteLine($"{test} Done{NewLine}");
    }

    [Fact]
    [Category("Unit")]
    [Category("Reactor")]
    public void LoginReissue_Scenario_B_Test()
    {
        string test = "LoginReissue_Scenario_B_Test()";
        output.WriteLine("\n" + test + " Running...");
        output.WriteLine("/*   CONS                 WatchList                 PROV\n" +
                           "   1) |    Cons Request[0] -> |                       |\n" +
                           "   2) |                       |    Prov Request[0] -> |\n" +
                           "   3) |    Cons Request[1] -> |                       |\n" +
                           "   4) |    Cons Request[2] -> |                       |\n" +
                           "   5) |                       | <- Prov Refresh[0]    |\n" +
                           "   6) | <- Cons Refresh[0]    |                       |\n" +
                           "   7) |                       |    Prov Request[2] -> |\n" +
                           "   8) |                       | <- Prov Refresh[2]    |\n" +
                           "   9) | <- Cons Refresh[2]    |                       |\n" +
                           "*/");
        int loginStreamId;
        TestReactorEvent reactorEvent;
        RDMLoginMsgEvent loginMsgEvent;
        ReactorSubmitOptions submitOptions = new ReactorSubmitOptions();

        LoginRequest[] consRequest = { null, null, null };
        LoginRefresh[] consRefresh = { null, null, null };
        LoginRequest[] provRequest = { null, null, null };
        LoginRefresh[] provRefresh = { null, null, null };

        // Data arrays - index [0] is used for the initial request and refresh, the other
        // indices are for the subsequent reissue requests and refreshes.
        string[] userNames = { "userName_0", "userName_1", "userName_2" };
        string[] authenticationTokens = { "authenticationToken_0", "authenticationToken_1", "authenticationToken_2" };
        string[] authenticationExts = { "authenticationExt_0", "authenticationExt_1", "authenticationExt_2" };
        string[] authenticationExtResps = { "authenticationExtResp_0", "authenticationExtResp_1", "authenticationExtResp_2" };
        long[] authenticationTTReissues = { 123123000, 123123001, 123123002 };

        Login.UserIdTypes[] userNameTypes = { Login.UserIdTypes.NAME, Login.UserIdTypes.EMAIL_ADDRESS,
            Login.UserIdTypes.TOKEN, Login.UserIdTypes.COOKIE, Login.UserIdTypes.AUTHN_TOKEN
        };
        foreach (Login.UserIdTypes userNameType in userNameTypes)
        {
            output.WriteLine($"{test} loop: userNameType = " + userNameType);

            // Create reactors
            TestReactor consumerReactor = new TestReactor();
            TestReactor providerReactor = new TestReactor();
            consumerReactor.Reactor.m_ReactorOptions.XmlTracing = true;
            providerReactor.Reactor.m_ReactorOptions.XmlTracing = true;

            // Create consumer and the initial login request message with data using index [0]
            Consumer consumer = new Consumer(consumerReactor);
            ConsumerRole consumerRole = (ConsumerRole)consumer.ReactorRole;
            consumerRole.InitDefaultRDMLoginRequest();
            consumerRole.InitDefaultRDMDirectoryRequest();

            output.WriteLine($"{test} 1) Consumer creating login request[0]");
            consRequest[0] = consumerRole.RdmLoginRequest;
            loginStreamId = consRequest[0].StreamId;
            consRequest[0].HasUserNameType = true;
            consRequest[0].UserNameType = userNameType;

            if (userNameType == Login.UserIdTypes.AUTHN_TOKEN)
            {
                consRequest[0].UserName.Data(authenticationTokens[0]);
                consRequest[0].HasAuthenticationExtended = true;
                consRequest[0].AuthenticationExtended.Data(authenticationExts[0]);
            }
            else
                consRequest[0].UserName.Data(userNames[0]);

            consumerRole.ChannelEventCallback = consumer;
            consumerRole.LoginMsgCallback = consumer;
            consumerRole.DirectoryMsgCallback = consumer;
            consumerRole.DictionaryMsgCallback = consumer;
            consumerRole.DefaultMsgCallback = consumer;
            consumerRole.WatchlistOptions.EnableWatchlist = true;
            consumerRole.WatchlistOptions.ChannelOpenEventCallback = consumer;

            // Create provider.
            Provider provider = new Provider(providerReactor);
            ProviderRole providerRole = (ProviderRole)provider.ReactorRole;
            providerRole.ChannelEventCallback = provider;
            providerRole.LoginMsgCallback = provider;
            providerRole.DirectoryMsgCallback = provider;
            providerRole.DictionaryMsgCallback = provider;
            providerRole.DefaultMsgCallback = provider;

            // Connect the consumer and provider. Disable the automatic setup of login & directory streams.
            ConsumerProviderSessionOptions opts = new ConsumerProviderSessionOptions();
            opts.SetupDefaultLoginStream = false;
            opts.SetupDefaultDirectoryStream = false;
            provider.Bind(opts);
            output.WriteLine($"{test} 1) Consumer sending login request[0]");
            TestReactor.OpenSession(consumer, provider, opts);
            output.WriteLine($"{test} 1) Consumer sent login request[0]");

            output.WriteLine($"{test} 2) Provider dispatching, expects login request[0]");
            provider.TestReactor.Dispatch(1);
            reactorEvent = provider.TestReactor.PollEvent();
            Assert.Equal(TestReactorEventType.LOGIN_MSG, reactorEvent.EventType);
            loginMsgEvent = (RDMLoginMsgEvent)reactorEvent.ReactorEvent;
            Assert.Equal(LoginMsgType.REQUEST, loginMsgEvent.LoginMsg.LoginMsgType);
            output.WriteLine($"{test} 2) Provider received login request[0]");

            provRequest[0] = loginMsgEvent.LoginMsg.LoginRequest;
            Assert.Equal(provRequest[0].StreamId, loginStreamId);
            Assert.True(provRequest[0].HasUserNameType);
            Assert.Equal(provRequest[0].UserNameType, userNameType);

            if (userNameType == Login.UserIdTypes.AUTHN_TOKEN)
            {
                Assert.Equal(provRequest[0].UserName.ToString(), authenticationTokens[0]);
                Assert.True(provRequest[0].HasAuthenticationExtended);
                Assert.Equal(provRequest[0].AuthenticationExtended.ToString(), authenticationExts[0]);
            }
            else
                Assert.Equal(provRequest[0].UserName.ToString(), userNames[0]);
            output.WriteLine($"{test} 2) Provider validated login request[0]");

            output.WriteLine($"{test} 3) Consumer creating login request[1]");
            consumerRole.InitDefaultRDMLoginRequest();
            consRequest[1] = consumerRole.RdmLoginRequest;
            Assert.NotNull(consRequest[1]);
            consRequest[1].HasUserNameType = true;
            consRequest[1].UserNameType = userNameType;
            if (userNameType == Login.UserIdTypes.AUTHN_TOKEN)
            {
                consRequest[1].UserName.Data(authenticationTokens[1]);
                consRequest[1].HasAuthenticationExtended = true;
                consRequest[1].AuthenticationExtended.Data(authenticationExts[1]);
            }
            else if (userNameType == Login.UserIdTypes.TOKEN)
                consRequest[1].UserName.Data(userNames[1]);
            else
                consRequest[1].UserName.Data(userNames[0]);

            output.WriteLine($"{test} 3) Consumer sending login request[1]");
            submitOptions.Clear();
            Assert.True(consumer.SubmitAndDispatch(consRequest[1], submitOptions) >= ReactorReturnCode.SUCCESS);
            output.WriteLine($"{test} 3) Consumer sent login request[1]");

            provider.TestReactor.Dispatch(0);
            output.WriteLine($"{test} 3.1) Confirmed Watchlist did not send request[1] before receiving refresh[0]");

            output.WriteLine($"{test} 4) Consumer creating login request[2]");
            consumerRole.InitDefaultRDMLoginRequest();
            consRequest[2] = consumerRole.RdmLoginRequest;
            Assert.NotNull(consRequest[2]);
            consRequest[2].HasUserNameType = true;
            consRequest[2].UserNameType = userNameType;
            if (userNameType == Login.UserIdTypes.AUTHN_TOKEN)
            {
                consRequest[2].UserName.Data(authenticationTokens[2]);
                consRequest[2].HasAuthenticationExtended = true;
                consRequest[2].AuthenticationExtended.Data(authenticationExts[2]);
            }
            else if (userNameType == Login.UserIdTypes.TOKEN)
                consRequest[2].UserName.Data(userNames[2]);
            else
                consRequest[2].UserName.Data(userNames[0]);

            output.WriteLine($"{test} 4) Consumer sending login request[2]");
            submitOptions.Clear();
            Assert.True(consumer.SubmitAndDispatch(consRequest[2], submitOptions) >= ReactorReturnCode.SUCCESS);
            output.WriteLine($"{test} 4) Consumer sent login request[2]");

            provider.TestReactor.Dispatch(0);
            output.WriteLine($"{test} 4.1) Confirmed Watchlist did not send request[1] before receiving refresh[0]");

            output.WriteLine($"{test} 5) Provider creating login refresh[0]");
            provRefresh[0] = new LoginRefresh();
            provRefresh[0].Clear();
            provRefresh[0].StreamId = loginStreamId;
            provRefresh[0].Solicited = true;
            provRefresh[0].HasUserNameType = true;
            provRefresh[0].UserNameType = userNameType;
            if (userNameType == Login.UserIdTypes.AUTHN_TOKEN)
            {
                provRefresh[0].UserName.Data(userNames[0]);
                provRefresh[0].HasAuthenicationTTReissue = true;
                provRefresh[0].AuthenticationTTReissue = authenticationTTReissues[0];
                provRefresh[0].HasAuthenticationExtendedResp = true;
                provRefresh[0].AuthenticationExtendedResp.Data(authenticationExtResps[0]);
            }
            else
            {
                provRefresh[0].HasUserName = true;
                provRefresh[0].UserName.Data(userNames[0]);
            }
            provRefresh[0].State.StreamState(StreamStates.OPEN);
            provRefresh[0].State.DataState(DataStates.OK);

            output.WriteLine($"{test} 5) Provider sending login refresh[0]");
            submitOptions.Clear();
            Assert.True(provider.SubmitAndDispatch(provRefresh[0], submitOptions) >= ReactorReturnCode.SUCCESS);
            output.WriteLine($"{test} 5) Provider sent login refresh[0]");

            output.WriteLine($"{test} 6) Consumer dispatching, expects login refresh[0] (if so, will send Directory request)");
            consumer.TestReactor.Dispatch(1);
            reactorEvent = consumer.TestReactor.PollEvent();
            Assert.Equal(TestReactorEventType.LOGIN_MSG, reactorEvent.EventType);
            loginMsgEvent = (RDMLoginMsgEvent)reactorEvent.ReactorEvent;
            Assert.Equal(LoginMsgType.REFRESH, loginMsgEvent.LoginMsg.LoginMsgType);
            Assert.Equal(MsgClasses.REFRESH, loginMsgEvent.Msg.MsgClass);
            output.WriteLine($"{test} 6) Consumer received login refresh[0]");

            consRefresh[0] = loginMsgEvent.LoginMsg.LoginRefresh;
            Assert.Equal(consRefresh[0].StreamId, loginStreamId);
            Assert.Equal(StreamStates.OPEN, consRefresh[0].State.StreamState());
            Assert.Equal(DataStates.OK, consRefresh[0].State.DataState());
            Assert.True(consRefresh[0].HasUserName);
            Assert.True(consRefresh[0].HasUserNameType);
            if (userNameType == Login.UserIdTypes.AUTHN_TOKEN)
            {
                Assert.True(consRefresh[0].UserName.Data().ReadByte() == 0x0);
                Assert.True(consRefresh[0].HasAuthenicationTTReissue);
                Assert.Equal(consRefresh[0].AuthenticationTTReissue, authenticationTTReissues[0]);
                Assert.True(consRefresh[0].HasAuthenticationExtendedResp);
                Assert.Equal(consRefresh[0].AuthenticationExtendedResp.ToString(), authenticationExtResps[0]);
            }
            else
                Assert.Equal(consRefresh[0].UserName.ToString(), userNames[0]);
            output.WriteLine($"{test} 6) Consumer validated login refresh[0]");

            output.WriteLine($"{test} 7) Provider dispatching, expects Login Request[2] and Directory request[1]");

            provider.TestReactor.Dispatch(2);
            // validate Login request[2]
            reactorEvent = providerReactor.PollEvent();
            Assert.Equal(TestReactorEventType.LOGIN_MSG, reactorEvent.EventType);
            loginMsgEvent = (RDMLoginMsgEvent)reactorEvent.ReactorEvent;
            Assert.Equal(LoginMsgType.REQUEST, loginMsgEvent.LoginMsg.LoginMsgType);
            LoginRequest loginRequest = loginMsgEvent.LoginMsg.LoginRequest;
            Assert.Equal(loginRequest.StreamId, loginStreamId);
            Assert.True(loginRequest.HasUserNameType);
            Assert.Equal(loginRequest.UserNameType, userNameType);
            if (userNameType == Login.UserIdTypes.AUTHN_TOKEN)
            {
                Assert.Equal(loginRequest.UserName.ToString(), authenticationTokens[2]);
                Assert.True(loginRequest.HasAuthenticationExtended);
                Assert.Equal(loginRequest.AuthenticationExtended.ToString(), authenticationExts[2]);
            }
            else if (userNameType == Login.UserIdTypes.TOKEN)
                Assert.Equal(loginRequest.UserName.ToString(), userNames[2]);
            else
                Assert.Equal(loginRequest.UserName.ToString(), userNames[0]);
            output.WriteLine($"{test} 4.1) Provider validated login request[2]");

            // validate Directory request[1]
            reactorEvent = provider.TestReactor.PollEvent();
            Assert.Equal(TestReactorEventType.DIRECTORY_MSG, reactorEvent.EventType);
            output.WriteLine($"{test} 7) Provider received Directory request[1]");
            output.WriteLine($"{test} 7) Provider does not validate Directory request[1]");

            output.WriteLine($"{test} 8) Prov creating login refresh[1]");
            provRefresh[1] = new LoginRefresh();
            provRefresh[1].Clear();
            provRefresh[1].StreamId = loginStreamId;
            provRefresh[1].Solicited = true;
            provRefresh[1].HasUserNameType = true;
            provRefresh[1].UserNameType = userNameType;
            if (userNameType == Login.UserIdTypes.AUTHN_TOKEN)
            {
                provRefresh[1].UserName.Data(userNames[1]);
                provRefresh[1].HasAuthenicationTTReissue = true;
                provRefresh[1].AuthenticationTTReissue = authenticationTTReissues[1];
                provRefresh[1].HasAuthenticationExtendedResp = true;
                provRefresh[1].AuthenticationExtendedResp.Data(authenticationExtResps[1]);
            }
            else
            {
                provRefresh[1].HasUserName = true;
                provRefresh[1].UserName.Data(userNames[1]);
            }
            provRefresh[1].State.StreamState(StreamStates.OPEN);
            provRefresh[1].State.DataState(DataStates.OK);

            output.WriteLine($"{test} 8) Prov sending login refresh[1]");
            submitOptions.Clear();
            Assert.True(provider.SubmitAndDispatch(provRefresh[1], submitOptions) >= ReactorReturnCode.SUCCESS);
            output.WriteLine($"{test} 8) Prov sent login refresh[1]");

            output.WriteLine($"{test} 9) Consumer dispatching, expects login refresh[1]");
            consumer.TestReactor.Dispatch(1);
            reactorEvent = consumer.TestReactor.PollEvent();
            Assert.Equal(TestReactorEventType.LOGIN_MSG, reactorEvent.EventType);
            loginMsgEvent = (RDMLoginMsgEvent)reactorEvent.ReactorEvent;
            Assert.Equal(LoginMsgType.REFRESH, loginMsgEvent.LoginMsg.LoginMsgType);
            Assert.Equal(MsgClasses.REFRESH, loginMsgEvent.Msg.MsgClass);
            output.WriteLine($"{test} 9) Consumer received login refresh[1]");

            consRefresh[1] = loginMsgEvent.LoginMsg.LoginRefresh;
            Assert.Equal(consRefresh[1].StreamId, loginStreamId);
            Assert.Equal(StreamStates.OPEN, consRefresh[1].State.StreamState());
            Assert.Equal(DataStates.OK, consRefresh[1].State.DataState());
            Assert.True(consRefresh[1].HasUserName);
            Assert.True(consRefresh[1].HasUserNameType);
            if (userNameType == Login.UserIdTypes.AUTHN_TOKEN)
            {
                Assert.True(consRefresh[1].UserName.Data().ReadByte() == 0x0);
                Assert.True(consRefresh[1].HasAuthenicationTTReissue);
                Assert.Equal(consRefresh[1].AuthenticationTTReissue, authenticationTTReissues[1]);
                Assert.True(consRefresh[1].HasAuthenticationExtendedResp);
                Assert.Equal(consRefresh[1].AuthenticationExtendedResp.ToString(), authenticationExtResps[1]);
            }
            else
                Assert.Equal(consRefresh[1].UserName.ToString(), userNames[1]);
            output.WriteLine($"{test} 9) Consumer validated login refresh[1]");

            TestReactorComponent.CloseSession(consumer, provider);
            TearDownConsumerAndProvider(consumerReactor, providerReactor);
        }
        output.WriteLine($"{test} Done{NewLine}");
    }

    [Fact]
    [Category("Unit")]
    [Category("Reactor")]
    public void LoginReissue_Scenario_C_Test()
    {
        string test = "LoginReissueTest_Scenario_C_Test()";
        output.WriteLine($"{NewLine}{test} Running...");
        int loginStreamId;
        TestReactorEvent reactorEvent;
        RDMLoginMsgEvent loginMsgEvent;
        ReactorErrorInfo errorInfo;
        ReactorSubmitOptions submitOptions = new ReactorSubmitOptions();

        LoginRequest[] consRequest = { null, null, null, null };
        LoginRefresh[] consRefresh = { null, null, null, null };
        LoginRequest[] provRequest = { null, null, null, null };
        LoginRefresh[] provRefresh = { null, null, null, null };

        // Data arrays - index [0] is used for the initial request and refresh, the other
        // indices are for the subsequent reissue requests and refreshes.
        string[] userNames = { "userName_0", "userName_1", "userName_2", "userName_3" };
        string[] authenticationTokens = { "authenticationToken_0", "authenticationToken_1", "authenticationToken_2", "authenticationToken_3" };
        string[] authenticationExts = { "authenticationExt_0", "authenticationExt_1", "authenticationExt_2", "authenticationExt_3" };
        string[] authenticationExtResps = { "authenticationExtResp_0", "authenticationExtResp_1", "authenticationExtResp_2", "authenticationExtResp_3" };
        long[] authenticationTTReissues = { 123123000, 123123001, 123123002, 123123003 };

        // Run a test for each userNameType
        Login.UserIdTypes[] userNameTypes = { Login.UserIdTypes.NAME, Login.UserIdTypes.EMAIL_ADDRESS,
            Login.UserIdTypes.TOKEN, Login.UserIdTypes.COOKIE, Login.UserIdTypes.AUTHN_TOKEN
        };
        foreach (Login.UserIdTypes userNameType in userNameTypes)
        {
            bool reissueSuccess = true;
            output.WriteLine($"{test} loop: userNameType = {userNameType}");

            // Create reactors.
            TestReactor consumerReactor = new TestReactor();
            TestReactor providerReactor = new TestReactor();
            consumerReactor.Reactor.m_ReactorOptions.XmlTracing = true;
            providerReactor.Reactor.m_ReactorOptions.XmlTracing = true;

            // Create consumer.
            Consumer consumer = new Consumer(consumerReactor);
            ConsumerRole consumerRole = (ConsumerRole)consumer.ReactorRole;
            consumerRole.InitDefaultRDMLoginRequest();
            consumerRole.InitDefaultRDMDirectoryRequest();

            output.WriteLine($"{test} 1) Consumer creating login request[0]");
            consRequest[0] = consumerRole.RdmLoginRequest;
            consRequest[0].HasUserNameType = true;
            consRequest[0].UserNameType = userNameType;
            if (userNameType == Login.UserIdTypes.AUTHN_TOKEN)
            {
                // Setting the userName on a login request with userNameType = AUTHN_TOKEN
                // is allowed, however it will be ignored
                consRequest[0].UserName.Data(userNames[0]);
                consRequest[0].UserName.Data(authenticationTokens[0]);
                consRequest[0].HasAuthenticationExtended = true;
                consRequest[0].AuthenticationExtended.Data(authenticationExts[0]);
            }
            else
                consRequest[0].UserName.Data(userNames[0]);
            consumerRole.ChannelEventCallback = consumer;
            consumerRole.LoginMsgCallback = consumer;
            consumerRole.DirectoryMsgCallback = consumer;
            consumerRole.DictionaryMsgCallback = consumer;
            consumerRole.DefaultMsgCallback = consumer;
            consumerRole.WatchlistOptions.EnableWatchlist = true;
            consumerRole.WatchlistOptions.ChannelOpenEventCallback = consumer;

            // Create provider.
            Provider provider = new Provider(providerReactor);
            ProviderRole providerRole = (ProviderRole)provider.ReactorRole;
            providerRole.ChannelEventCallback = provider;
            providerRole.LoginMsgCallback = provider;
            providerRole.DirectoryMsgCallback = provider;
            providerRole.DictionaryMsgCallback = provider;
            providerRole.DefaultMsgCallback = provider;

            // Connect the consumer and provider. Disable the automatic setup of login & directory streams.
            ConsumerProviderSessionOptions opts = new ConsumerProviderSessionOptions();
            opts.SetupDefaultLoginStream = false;
            opts.SetupDefaultDirectoryStream = false;

            provider.Bind(opts);

            output.WriteLine($"{test} 1) Consumer sending login request[0]");
            TestReactor.OpenSession(consumer, provider, opts);
            output.WriteLine($"{test} 1) Consumer sent login request[0]");

            output.WriteLine($"{test} 2) Provider dispatching, expects login request[0]");

            provider.TestReactor.Dispatch(1);
            reactorEvent = provider.TestReactor.PollEvent();

            Assert.Equal(TestReactorEventType.LOGIN_MSG, reactorEvent.EventType);
            loginMsgEvent = (RDMLoginMsgEvent)reactorEvent.ReactorEvent;
            Assert.Equal(LoginMsgType.REQUEST, loginMsgEvent.LoginMsg.LoginMsgType);
            output.WriteLine($"{test} 2) Provider received login request[0]");

            provRequest[0] = (LoginRequest)loginMsgEvent.LoginMsg.LoginRequest;
            loginStreamId = provRequest[0].StreamId;
            Assert.True(provRequest[0].HasUserNameType);
            Assert.Equal(provRequest[0].UserNameType, userNameType);
            if (userNameType == Login.UserIdTypes.AUTHN_TOKEN)
            {
                // The userName on any login request with userNameType = AUTHN_TOKEN has no data
                Assert.Equal(provRequest[0].UserName.ToString(), authenticationTokens[0]);
                Assert.True(provRequest[0].HasAuthenticationExtended);
                Assert.Equal(provRequest[0].AuthenticationExtended.ToString(), authenticationExts[0]);
            }
            else
                Assert.Equal(provRequest[0].UserName.ToString(), userNames[0]);
            output.WriteLine($"{test} 2) Provider validated login request[0]");

            output.WriteLine($"{test} 3) Provider creating login refresh[0]");
            provRefresh[0] = new LoginRefresh();
            provRefresh[0].Clear();
            provRefresh[0].StreamId = loginStreamId;
            provRefresh[0].Solicited = true;
            provRefresh[0].HasUserName = true;
            provRefresh[0].HasUserNameType = true;
            provRefresh[0].UserNameType = userNameType;
            if (userNameType == Login.UserIdTypes.AUTHN_TOKEN)
            {
                provRefresh[0].UserName.Data(userNames[0]);
                provRefresh[0].HasAuthenicationTTReissue = true;
                provRefresh[0].AuthenticationTTReissue = authenticationTTReissues[0];
                provRefresh[0].HasAuthenticationExtendedResp = true;
                provRefresh[0].AuthenticationExtendedResp.Data(authenticationExtResps[0]);
            }
            else
                provRefresh[0].UserName.Data(userNames[0]);
            provRefresh[0].State.StreamState(StreamStates.OPEN);
            provRefresh[0].State.DataState(DataStates.OK);

            output.WriteLine($"{test} 3) Provider sending login refresh[0]");
            submitOptions.Clear();
            Assert.True(provider.SubmitAndDispatch(provRefresh[0], submitOptions) >= ReactorReturnCode.SUCCESS);
            output.WriteLine($"{test} 3) Provider sent login refresh[0]");

            output.WriteLine($"{test} 4) Consumer dispatching, expects login refresh[0] (if so, will send Directory request)");
            consumer.TestReactor.Dispatch(1);
            reactorEvent = consumer.TestReactor.PollEvent();
            Assert.Equal(TestReactorEventType.LOGIN_MSG, reactorEvent.EventType);
            loginMsgEvent = (RDMLoginMsgEvent)reactorEvent.ReactorEvent;
            Assert.Equal(LoginMsgType.REFRESH, loginMsgEvent.LoginMsg.LoginMsgType);
            Assert.Equal(MsgClasses.REFRESH, loginMsgEvent.Msg.MsgClass);
            output.WriteLine($"{test} 4) Consumer received login refresh[0]");

            consRefresh[0] = (LoginRefresh)loginMsgEvent.LoginMsg.LoginRefresh;
            Assert.Equal(consRefresh[0].StreamId, loginStreamId);
            Assert.Equal(StreamStates.OPEN, consRefresh[0].State.StreamState());
            Assert.Equal(DataStates.OK, consRefresh[0].State.DataState());
            Assert.True(consRefresh[0].HasUserName);
            Assert.True(consRefresh[0].HasUserNameType);
            if (userNameType == Login.UserIdTypes.AUTHN_TOKEN)
            {
                Assert.True(consRefresh[0].UserName.Data().ReadByte() == 0x0);
                Assert.True(consRefresh[0].HasAuthenicationTTReissue);
                Assert.Equal(consRefresh[0].AuthenticationTTReissue, authenticationTTReissues[0]);
                Assert.True(consRefresh[0].HasAuthenticationExtendedResp);
                Assert.Equal(consRefresh[0].AuthenticationExtendedResp.ToString(), authenticationExtResps[0]);
            }
            else
                Assert.Equal(consRefresh[0].UserName.ToString(), userNames[0]);
            output.WriteLine($"{test} 4) Consumer validated login refresh[0]");

            // Send a number of reissue requests and corresponding refreshes,
            // using the for loop counter as the request/refresh data index.
            for (int k = 1; k <= 3; k++)
            {
                output.WriteLine($"{test} {4 * k + 1}) Consumer creating login request[{k}]");
                consumerRole.InitDefaultRDMLoginRequest();
                consRequest[k] = consumerRole.RdmLoginRequest;
                Assert.NotNull(consRequest[k]);
                consRequest[k].HasUserNameType = true;
                consRequest[k].UserNameType = userNameType;

                bool matchRequestedUserName = (k == 2);
                bool mismatchUserNameTypes = (k == 3);

                if (userNameType == Login.UserIdTypes.AUTHN_TOKEN
                    || userNameType == Login.UserIdTypes.TOKEN)
                {
                    if (userNameType == Login.UserIdTypes.AUTHN_TOKEN)
                    {
                        if (matchRequestedUserName == true)
                            consRequest[k].UserName.Data(authenticationTokens[0]);
                        else
                            consRequest[k].UserName.Data(authenticationTokens[k]);

                        consRequest[k].HasAuthenticationExtended = true;
                        consRequest[k].AuthenticationExtended.Data(authenticationExts[k]);
                    }
                    else if (userNameType == Login.UserIdTypes.TOKEN)
                    {
                        if (matchRequestedUserName == true)
                            consRequest[k].UserName.Data(userNames[0]);
                        else
                            consRequest[k].UserName.Data(userNames[k]);
                    }
                    if (mismatchUserNameTypes == true)
                    {
                        if (userNameType == Login.UserIdTypes.AUTHN_TOKEN)
                            consRequest[k].UserNameType = Login.UserIdTypes.TOKEN;
                        else if (userNameType == Login.UserIdTypes.TOKEN)
                            consRequest[k].UserNameType = Login.UserIdTypes.AUTHN_TOKEN;

                        output.WriteLine($"{test} {4 * k + 1}) Consumer sending login request[{k}], mismatchUserNameTypes");
                        submitOptions.Clear();
                        Assert.True(consumer.ReactorChannel.Submit(consRequest[k], submitOptions, out errorInfo) == ReactorReturnCode.INVALID_USAGE);
                        Assert.Equal("Login userNameType does not match existing request", errorInfo.Error.Text);

                        output.WriteLine($"{test} {4 * k + 1}) Consumer sent login request[{k}], mismatchUserNameTypes");
                        reissueSuccess = false;
                    }
                    else
                    {
                        output.WriteLine($"{test} {4 * k + 1}) Consumer sending login request[{k}]");
                        submitOptions.Clear();
                        Assert.True(consumer.SubmitAndDispatch(consRequest[k], submitOptions) >= ReactorReturnCode.SUCCESS);
                        output.WriteLine($"{test} {4 * k + 1}) Consumer sent login request[{k}]");
                    }
                }
                else
                {
                    if (matchRequestedUserName == true)
                    {
                        consRequest[k].UserName.Data(userNames[0]);
                        submitOptions.Clear();
                        if (mismatchUserNameTypes == true)
                        {
                            consRequest[k].UserNameType = Login.UserIdTypes.TOKEN;
                            output.WriteLine($"{test} {4 * k + 1}) Consumer sending login request[{k}], mismatchUserNameTypes");
                            Assert.True(consumer.ReactorChannel.Submit(consRequest[k], submitOptions, out errorInfo) == ReactorReturnCode.INVALID_USAGE);
                            Assert.Equal("Login userNameType does not match existing request", errorInfo.Error.Text);
                            output.WriteLine($"{test} {4 * k + 1}) Consumer sent login request[{k}], mismatchUserNameTypes");
                            reissueSuccess = false;
                        }
                        else
                        {
                            output.WriteLine($"{test} {4 * k + 1}) Consumer sending login request[{k}]");
                            Assert.True(consumer.SubmitAndDispatch(consRequest[k], submitOptions) >= ReactorReturnCode.SUCCESS);
                            output.WriteLine($"{test} {4 * k + 1} ) Consumer sent login request[{k}]");
                        }
                    }
                    else
                    {
                        consRequest[k].UserName.Data(userNames[k]);
                        submitOptions.Clear();
                        if (mismatchUserNameTypes == true)
                        {
                            consRequest[k].UserNameType = Login.UserIdTypes.AUTHN_TOKEN;
                            output.WriteLine($"{test} {4 * k + 1}) Consumer sending login request[{k}], mismatchUserNameTypes");
                            Assert.True(consumer.ReactorChannel.Submit(consRequest[k], submitOptions, out errorInfo) == ReactorReturnCode.INVALID_USAGE);
                            Assert.Equal("Login userNameType does not match existing request", errorInfo.Error.Text);
                            output.WriteLine($"{test} {4 * k + 1}) Consumer sent login request[{k}], mismatchUserNameTypes");
                            reissueSuccess = false;
                        }
                        else
                        {
                            consRequest[k].UserName.Data(userNames[0]);
                            output.WriteLine($"{test} {4 * k + 1}) Consumer sending login request[{k}]");
                            Assert.True(consumer.SubmitAndDispatch(consRequest[k], submitOptions) >= ReactorReturnCode.SUCCESS);
                            output.WriteLine($"{test} {4 * k + 1}) Consumer sent login request[{k}]");
                        }
                    }
                }

                if (reissueSuccess == true)
                {
                    // Provider receives login reissue request (and directory request), then verifies data using index [k].
                    if (k == 1)
                    {
                        output.WriteLine($"{test} {5 * k + 1}) Provider dispatching, expects Directory and login request[{k}]");
                        provider.TestReactor.Dispatch(2);
                        reactorEvent = provider.TestReactor.PollEvent();
                        Assert.Equal(TestReactorEventType.DIRECTORY_MSG, reactorEvent.EventType);
                        output.WriteLine($"{test} {5 * k + 1}) Provider received Directory request");
                    }
                    else
                    {
                        output.WriteLine($"{test} {5 * k + 1}) Provider dispatching, expects login request[{k}]");
                        provider.TestReactor.Dispatch(1);
                    }

                    reactorEvent = provider.TestReactor.PollEvent();
                    Assert.Equal(TestReactorEventType.LOGIN_MSG, reactorEvent.EventType);
                    loginMsgEvent = (RDMLoginMsgEvent)reactorEvent.ReactorEvent;
                    Assert.Equal(LoginMsgType.REQUEST, loginMsgEvent.LoginMsg.LoginMsgType);
                    output.WriteLine($"{test} {5 * k + 1}) Provider received login request[{k}]");

                    provRequest[k] = loginMsgEvent.LoginMsg.LoginRequest;
                    loginStreamId = provRequest[k].StreamId;
                    Assert.True(provRequest[k].HasUserNameType);
                    Assert.Equal(provRequest[k].UserNameType, userNameType);
                    if (userNameType == Login.UserIdTypes.AUTHN_TOKEN)
                    {
                        if (matchRequestedUserName == true)
                            Assert.Equal(provRequest[k].UserName.ToString(), authenticationTokens[0]);
                        else
                            Assert.Equal(provRequest[k].UserName.ToString(), authenticationTokens[k]);
                        Assert.True(provRequest[k].HasAuthenticationExtended);
                        Assert.Equal(provRequest[k].AuthenticationExtended.ToString(), authenticationExts[k]);
                    }
                    else if (userNameType == Login.UserIdTypes.TOKEN)
                    {
                        if (matchRequestedUserName == true)
                            Assert.Equal(provRequest[k].UserName.ToString(), userNames[0]);
                        else
                            Assert.Equal(provRequest[k].UserName.ToString(), userNames[k]);
                    }
                    output.WriteLine($"{test} {5 * k + 1}) Provider validated login request[{k}]");

                    output.WriteLine($"{test} {6 * k + 1}) Provider creating login refresh[{k}]");
                    provRefresh[k] = new LoginRefresh();
                    provRefresh[k].Clear();
                    provRefresh[k].StreamId = loginStreamId;
                    provRefresh[k].Solicited = true;
                    provRefresh[k].HasUserName = true;
                    provRefresh[k].HasUserNameType = true;
                    provRefresh[k].UserNameType = userNameType;
                    if (userNameType == Login.UserIdTypes.AUTHN_TOKEN)
                    {
                        provRefresh[k].UserName.Data(userNames[k]);
                        provRefresh[k].HasAuthenicationTTReissue = true;
                        provRefresh[k].AuthenticationTTReissue = authenticationTTReissues[k];
                        provRefresh[k].HasAuthenticationExtendedResp = true;
                        provRefresh[k].AuthenticationExtendedResp.Data(authenticationExtResps[k]);
                    }
                    else if (userNameType == Login.UserIdTypes.TOKEN)
                        provRefresh[k].UserName.Data(userNames[k]);
                    else
                        provRefresh[k].UserName.Data(userNames[0]);
                    provRefresh[k].State.StreamState(StreamStates.OPEN);
                    provRefresh[k].State.DataState(DataStates.OK);

                    output.WriteLine($"{test} {6 * k + 1}) Provider sending login refresh[{k}]");
                    submitOptions.Clear();
                    Assert.True(provider.SubmitAndDispatch(provRefresh[k], submitOptions) >= ReactorReturnCode.SUCCESS);
                    output.WriteLine($"{test} {6 * k + 1}) Provider sent login refresh[{k}]");

                    output.WriteLine($"{test} {7 * k + 1}) Consumer dispatching, expects login refresh[{k}]");

                    consumer.TestReactor.Dispatch(1);
                    reactorEvent = consumer.TestReactor.PollEvent();

                    Assert.Equal(TestReactorEventType.LOGIN_MSG, reactorEvent.EventType);
                    loginMsgEvent = (RDMLoginMsgEvent)reactorEvent.ReactorEvent;
                    Assert.Equal(LoginMsgType.REFRESH, loginMsgEvent.LoginMsg.LoginMsgType);
                    Assert.Equal(MsgClasses.REFRESH, loginMsgEvent.Msg.MsgClass);
                    output.WriteLine($"{test} {7 * k + 1}) Consumer received login refresh[{k}]");

                    consRefresh[k] = loginMsgEvent.LoginMsg.LoginRefresh;
                    Assert.Equal(consRefresh[k].StreamId, loginStreamId);
                    Assert.Equal(StreamStates.OPEN, consRefresh[k].State.StreamState());
                    Assert.Equal(DataStates.OK, consRefresh[k].State.DataState());
                    Assert.True(consRefresh[k].HasUserName);
                    Assert.True(consRefresh[k].HasUserNameType);
                    if (userNameType == Login.UserIdTypes.AUTHN_TOKEN)
                    {
                        Assert.True(consRefresh[k].UserName.Data().ReadByte() == 0x0);
                        Assert.True(consRefresh[k].HasAuthenicationTTReissue = true);
                        Assert.Equal(consRefresh[k].AuthenticationTTReissue, authenticationTTReissues[k]);
                        Assert.True(consRefresh[k].HasAuthenticationExtendedResp);
                        Assert.Equal(consRefresh[k].AuthenticationExtendedResp.ToString(), authenticationExtResps[k]);
                    }
                    else if (userNameType == Login.UserIdTypes.TOKEN)
                        Assert.Equal(consRefresh[k].UserName.ToString(), userNames[k]);
                    else
                        Assert.Equal(consRefresh[k].UserName.ToString(), userNames[0]);
                    output.WriteLine($"{test} {7 * k + 1}) Consumer validated login refresh[{k}]");
                }
            }
        }
        output.WriteLine($"{test} Done{NewLine}");
    }

    [Fact]
    [Category("Unit")]
    [Category("Reactor")]
    public void LoginClosedStatusTest_Socket()
    {
        LoginClosedStatus(false, null);
    }

    private void LoginClosedStatus(bool isWebsocket, string protocolList)
    {

        /* Test closing login stream from provider. Test that an item requested is also considered closed when this happens. */

        ReactorSubmitOptions submitOptions = new ReactorSubmitOptions();
        TestReactorEvent reactorEvent;
        ReactorMsgEvent msgEvent;
        IRequestMsg requestMsg = (IRequestMsg)new Msg();
        IRequestMsg receivedRequestMsg;
        IRefreshMsg refreshMsg = (IRefreshMsg)new Msg();
        IRefreshMsg receivedRefreshMsg;
        IStatusMsg receivedStatusMsg;
        RDMLoginMsgEvent loginMsgEvent;
        IUpdateMsg updateMsg = (IUpdateMsg)new Msg();
        ICloseMsg closeMsg = (ICloseMsg)new Msg();
        LoginStatus loginStatus = new LoginStatus();
        LoginStatus receivedLoginStatus;
        int providerStreamId;

        /* Create reactors. */
        TestReactor consumerReactor = new TestReactor();
        TestReactor providerReactor = new TestReactor();

        /* Create consumer. */
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

        /* Create provider. */
        Provider provider = new Provider(providerReactor);
        ProviderRole providerRole = (ProviderRole)provider.ReactorRole;
        providerRole.ChannelEventCallback = provider;
        providerRole.LoginMsgCallback = provider;
        providerRole.DirectoryMsgCallback = provider;
        providerRole.DictionaryMsgCallback = provider;
        providerRole.DefaultMsgCallback = provider;

        /* Connect the consumer and provider. Setup login & directory streams automatically. */
        ConsumerProviderSessionOptions opts = new ConsumerProviderSessionOptions();
        opts.SetupDefaultLoginStream = true;
        opts.SetupDefaultDirectoryStream = true;
        opts.ReconnectAttemptLimit = -1;

        provider.Bind(opts);

        TestReactor.OpenSession(consumer, provider, opts);

        /* Consumer sends request. */
        requestMsg.Clear();
        requestMsg.MsgClass = MsgClasses.REQUEST;
        requestMsg.StreamId = 5;
        requestMsg.DomainType = (int)DomainType.MARKET_PRICE;
        requestMsg.ApplyStreaming();
        requestMsg.MsgKey.ApplyHasName();
        requestMsg.MsgKey.Name.Data("TRI.N");
        submitOptions.Clear();
        submitOptions.ServiceName = Provider.DefaultService.Info.ServiceName.ToString();
        Assert.True(consumer.SubmitAndDispatch((Msg)requestMsg, submitOptions) >= ReactorReturnCode.SUCCESS);

        /* Provider receives request. */
        providerReactor.Dispatch(1);
        reactorEvent = providerReactor.PollEvent();
        Assert.Equal(TestReactorEventType.MSG, reactorEvent.EventType);
        msgEvent = (ReactorMsgEvent)reactorEvent.ReactorEvent;
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

        /* Provider sends redirecting StatusMsg .*/
        refreshMsg.Clear();
        refreshMsg.MsgClass = MsgClasses.REFRESH;
        refreshMsg.DomainType = (int)DomainType.MARKET_PRICE;
        refreshMsg.StreamId = providerStreamId;
        refreshMsg.ContainerType = DataTypes.NO_DATA;
        refreshMsg.ApplyHasMsgKey();
        refreshMsg.MsgKey.ApplyHasServiceId();
        refreshMsg.MsgKey.ServiceId = Provider.DefaultService.ServiceId;
        refreshMsg.MsgKey.ApplyHasName();
        refreshMsg.MsgKey.Name.Data("TRI.N");
        refreshMsg.State.StreamState(StreamStates.OPEN);
        refreshMsg.State.DataState(DataStates.OK);
        refreshMsg.ApplySolicited();

        Assert.True(provider.SubmitAndDispatch((Msg)refreshMsg, submitOptions) >= ReactorReturnCode.SUCCESS);

        /* Consumer receives refresh. */
        consumerReactor.Dispatch(1);
        reactorEvent = consumerReactor.PollEvent();
        Assert.Equal(TestReactorEventType.MSG, reactorEvent.EventType);
        msgEvent = (ReactorMsgEvent)reactorEvent.ReactorEvent;
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
        Assert.NotNull(msgEvent.StreamInfo);
        Assert.NotNull(msgEvent.StreamInfo.ServiceName);
        Assert.Equal(msgEvent.StreamInfo.ServiceName, Provider.DefaultService.Info.ServiceName.ToString());

        /* Provider sends login Closed status. */
        loginStatus.Clear();
        loginStatus.StreamId = provider.DefaultSessionLoginStreamId;
        loginStatus.HasState = true;
        loginStatus.State.StreamState(StreamStates.CLOSED);
        loginStatus.State.DataState(DataStates.SUSPECT);
        submitOptions.Clear();
        Assert.True(provider.SubmitAndDispatch(loginStatus, submitOptions) >= ReactorReturnCode.SUCCESS);

        /* Consumer receives closed/suspect login status. */
        consumerReactor.Dispatch(3);
        reactorEvent = consumerReactor.PollEvent();
        Assert.Equal(TestReactorEventType.LOGIN_MSG, reactorEvent.EventType);
        loginMsgEvent = (RDMLoginMsgEvent)reactorEvent.ReactorEvent;
        Assert.Equal(LoginMsgType.STATUS, loginMsgEvent.LoginMsg.LoginMsgType);
        receivedLoginStatus = (LoginStatus)loginMsgEvent.LoginMsg.LoginStatus;
        Assert.Equal(consumer.DefaultSessionLoginStreamId, receivedLoginStatus.StreamId);
        Assert.True(receivedLoginStatus.HasState);
        Assert.Equal(StreamStates.CLOSED, receivedLoginStatus.State.StreamState());
        Assert.Equal(DataStates.SUSPECT, receivedLoginStatus.State.DataState());

        /* Consumer receives status for TRI. */
        reactorEvent = consumerReactor.PollEvent();
        Assert.Equal(TestReactorEventType.MSG, reactorEvent.EventType);
        msgEvent = (ReactorMsgEvent)reactorEvent.ReactorEvent;
        Assert.Equal(MsgClasses.STATUS, msgEvent.Msg.MsgClass);

        receivedStatusMsg = (IStatusMsg)msgEvent.Msg;
        Assert.Equal(5, receivedStatusMsg.StreamId);
        Assert.Equal((int)DomainType.MARKET_PRICE, receivedStatusMsg.DomainType);
        Assert.Equal(DataTypes.NO_DATA, receivedStatusMsg.ContainerType);
        Assert.Equal(StreamStates.CLOSED, receivedStatusMsg.State.StreamState());
        Assert.Equal(DataStates.SUSPECT, receivedStatusMsg.State.DataState());

        Assert.NotNull(msgEvent.StreamInfo);
        Assert.NotNull(msgEvent.StreamInfo.ServiceName);
        Assert.Equal(msgEvent.StreamInfo.ServiceName, Provider.DefaultService.Info.ServiceName.ToString());

        /* Consumer receives update for Directory to delete services. */
        reactorEvent = consumerReactor.PollEvent();
        Assert.Equal(TestReactorEventType.DIRECTORY_MSG, reactorEvent.EventType);
        msgEvent = (ReactorMsgEvent)reactorEvent.ReactorEvent;
        Assert.Equal(MsgClasses.UPDATE, msgEvent.Msg.MsgClass);

        /* Provider sends an update for TRI. */
        updateMsg.Clear();
        updateMsg.MsgClass = MsgClasses.UPDATE;
        updateMsg.StreamId = providerStreamId;
        updateMsg.DomainType = (int)DomainType.MARKET_PRICE;
        updateMsg.ContainerType = DataTypes.NO_DATA;
        Assert.True(provider.SubmitAndDispatch((Msg)updateMsg, submitOptions) >= ReactorReturnCode.SUCCESS);

        /* Consumer does not receive it since the item is closed. */
        consumerReactor.Dispatch(0);

        /* Provider doesn't receive close for the update. */
        providerReactor.Dispatch(0);

        /* Consumer closes login stream. */
        closeMsg.Clear();
        closeMsg.MsgClass = MsgClasses.CLOSE;
        closeMsg.StreamId = consumer.DefaultSessionLoginStreamId;
        closeMsg.DomainType = (int)DomainType.LOGIN;
        Assert.True(consumer.SubmitAndDispatch((Msg)closeMsg, submitOptions) >= ReactorReturnCode.SUCCESS);

        /* Provider receives nothing. */
        providerReactor.Dispatch(0);

        TestReactorComponent.CloseSession(consumer, provider);
        TearDownConsumerAndProvider(consumerReactor, providerReactor);
    }

    [Fact]
    [Category("Unit")]
    [Category("Reactor")]
    public void LoginReissue_SunnyTest_Socket()
    {
        LoginReissue_Sunny(false, null);
    }

    private void LoginReissue_Sunny(bool isWebsocket, string protocolList)
    {
        string test = "LoginReissue_SunnyTest()";
        output.WriteLine($"\n{test} Running...");
        output.WriteLine("/*   CONS                 WatchList                 PROV\n" +
                "   1) |    Cons Request[0] -> |                       |\n" +
                "   2) |                       |    Prov Request[0] -> |\n" +
                "   3) |                       | <- Prov Refresh[0]    |\n" +
                "   4) | <- Cons Refresh[0]    |                       |\n" +
                "   5) |    User Request[1]    |                       |\n" +
                "   6) |    Cons Request[1] -> |                       |\n" +
                "   7) |                       |    Prov Request[1] -> |\n" +
                "   8) |                       | <- Prov Refresh[1]    |\n" +
                "   9) | <- Cons Refresh[1]    |                       |\n" +
                "*/");

        int loginStreamId;
        TestReactorEvent reactorEvent;
        RDMLoginMsgEvent loginMsgEvent;
        ReactorSubmitOptions submitOptions = new ReactorSubmitOptions();

        LoginRequest[] consRequest = { null, null };
        LoginRefresh[] consRefresh = { null, null };
        LoginRequest[] provRequest = { null, null };
        LoginRefresh[] provRefresh = { null, null };

        // Data arrays - index [0] is used for the initial request and refresh, the other
        // indices are for the subsequent reissue requests and refreshes.
        string[] userNames = { "userName_0", "userName_1" };
        string[] authenticationTokens = { "authenticationToken_0", "authenticationToken_1" };
        string[] authenticationExts = { "authenticationExt_0", "authenticationExt_1" };
        string[] authenticationExtResps = { "authenticationExtResp_0", "authenticationExtResp_1" };
        long[] authenticationTTReissues = { 123123000, 123123001 };

        Login.UserIdTypes[] userNameTypes = { Login.UserIdTypes.NAME, Login.UserIdTypes.EMAIL_ADDRESS,
            Login.UserIdTypes.TOKEN, Login.UserIdTypes.COOKIE, Login.UserIdTypes.AUTHN_TOKEN
        };
        foreach (Login.UserIdTypes userNameType in userNameTypes)
        {
            output.WriteLine($"{test} loop: userNameType = " + userNameType);

            // Create reactors
            TestReactor consumerReactor = new TestReactor();
            TestReactor providerReactor = new TestReactor();
            consumerReactor.Reactor.m_ReactorOptions.XmlTracing = true;
            providerReactor.Reactor.m_ReactorOptions.XmlTracing = true;

            // Create consumer and the initial login request message with data using index [0]
            Consumer consumer = new Consumer(consumerReactor);
            ConsumerRole consumerRole = (ConsumerRole)consumer.ReactorRole;
            consumerRole.InitDefaultRDMLoginRequest();
            consumerRole.InitDefaultRDMDirectoryRequest();

            output.WriteLine($"{test} 1) Consumer creating login request[0]");
            consRequest[0] = consumerRole.RdmLoginRequest;
            loginStreamId = consRequest[0].StreamId;
            consRequest[0].HasUserNameType = true;
            consRequest[0].UserNameType = userNameType;

            if (userNameType == Login.UserIdTypes.AUTHN_TOKEN)
            {
                consRequest[0].UserName.Data(authenticationTokens[0]);
                consRequest[0].HasAuthenticationExtended = true;
                consRequest[0].AuthenticationExtended.Data(authenticationExts[0]);
            }
            else
                consRequest[0].UserName.Data(userNames[0]);

            consumerRole.ChannelEventCallback = consumer;
            consumerRole.LoginMsgCallback = consumer;
            consumerRole.DirectoryMsgCallback = consumer;
            consumerRole.DictionaryMsgCallback = consumer;
            consumerRole.DefaultMsgCallback = consumer;
            consumerRole.WatchlistOptions.EnableWatchlist = true;
            consumerRole.WatchlistOptions.ChannelOpenEventCallback = consumer;

            // Create provider.
            Provider provider = new Provider(providerReactor);
            ProviderRole providerRole = (ProviderRole)provider.ReactorRole;
            providerRole.ChannelEventCallback = provider;
            providerRole.LoginMsgCallback = provider;
            providerRole.DirectoryMsgCallback = provider;
            providerRole.DictionaryMsgCallback = provider;
            providerRole.DefaultMsgCallback = provider;

            // Connect the consumer and provider. Disable the automatic setup of login & directory streams.
            ConsumerProviderSessionOptions opts = new ConsumerProviderSessionOptions();
            opts.SetupDefaultLoginStream = true;
            opts.SetupDefaultDirectoryStream = true;
            opts.ReconnectAttemptLimit = -1;

            provider.Bind(opts);

            output.WriteLine($"{test} 1) Consumer sending login request[0]");
            TestReactor.OpenSession(consumer, provider, opts);
            output.WriteLine($"{test} 1) Consumer sent login request[0]");

            provider.TestReactor.Dispatch(0);
            output.WriteLine($"{test} 2,3,4) Provider and consumer already dispatched request[0]");

            output.WriteLine($"{test} 5) Consumer creating login request[1]");
            consumerRole.InitDefaultRDMLoginRequest();
            consRequest[1] = consumerRole.RdmLoginRequest;
            Assert.NotNull(consRequest[1]);
            consRequest[1].HasUserNameType = true;
            consRequest[1].UserNameType = userNameType;
            if (userNameType == Login.UserIdTypes.AUTHN_TOKEN)
            {
                consRequest[1].UserName.Data(authenticationTokens[1]);
                consRequest[1].HasAuthenticationExtended = true;
                consRequest[1].AuthenticationExtended.Data(authenticationExts[1]);
            }
            else if (userNameType == Login.UserIdTypes.TOKEN)
                consRequest[1].UserName.Data(userNames[1]);
            else
                consRequest[1].UserName.Data(userNames[0]);

            output.WriteLine($"{test} 6) Consumer sending login request[1]");
            submitOptions.Clear();
            Assert.True(consumer.SubmitAndDispatch(consRequest[1], submitOptions) >= ReactorReturnCode.SUCCESS);
            output.WriteLine($"{test} 6) Consumer sent login request[1]");

            output.WriteLine($"{test} 7) Provider dispatching, expects login request[1]");
            provider.TestReactor.Dispatch(1);
            reactorEvent = provider.TestReactor.PollEvent();
            Assert.Equal(TestReactorEventType.LOGIN_MSG, reactorEvent.EventType);
            loginMsgEvent = (RDMLoginMsgEvent)reactorEvent.ReactorEvent;
            Assert.Equal(LoginMsgType.REQUEST, loginMsgEvent.LoginMsg.LoginMsgType);

            provRequest[1] = loginMsgEvent.LoginMsg.LoginRequest;
            Assert.Equal(provRequest[1].StreamId, loginStreamId);
            Assert.True(provRequest[1].HasUserNameType);
            Assert.Equal(provRequest[1].UserNameType, userNameType);

            if (userNameType == Login.UserIdTypes.AUTHN_TOKEN)
            {
                Assert.Equal(provRequest[1].UserName.ToString(), authenticationTokens[1]);
                Assert.True(provRequest[1].HasAuthenticationExtended);
                Assert.Equal(provRequest[1].AuthenticationExtended.ToString(), authenticationExts[1]);
            }
            else if (userNameType == Login.UserIdTypes.TOKEN)
                Assert.Equal(provRequest[1].UserName.ToString(), userNames[1]);
            else if (userNameType != Login.UserIdTypes.COOKIE
                && (protocolList != null && protocolList.Contains("json"))) /* JSON converter doesn't write name when nameType is COOKIE */
                Assert.Equal(provRequest[1].UserName.ToString(), userNames[0]);

            output.WriteLine($"{test} 7) Provider validated login request[1]");

            output.WriteLine($"{test} 8) Provider creating login refresh[1]");
            provRefresh[1] = new LoginRefresh();
            provRefresh[1].Clear();
            provRefresh[1].StreamId = loginStreamId;
            provRefresh[1].Solicited = true;
            provRefresh[1].HasUserNameType = true;
            provRefresh[1].UserNameType = userNameType;
            if (userNameType == Login.UserIdTypes.AUTHN_TOKEN)
            {
                provRefresh[1].UserName.Data(userNames[1]);
                provRefresh[1].HasAuthenicationTTReissue = true;
                provRefresh[1].AuthenticationTTReissue = authenticationTTReissues[1];
                provRefresh[1].HasAuthenticationExtendedResp = true;
                provRefresh[1].AuthenticationExtendedResp.Data(authenticationExtResps[1]);
            }
            else if (userNameType == Login.UserIdTypes.TOKEN)
            {
                provRefresh[1].HasUserName = true;
                provRefresh[1].UserName.Data(userNames[1]);
            }
            else
            {
                provRefresh[1].HasUserName = true;
                provRefresh[1].UserName.Data(userNames[0]);
            }
            provRefresh[1].State.StreamState(StreamStates.OPEN);
            provRefresh[1].State.DataState(DataStates.OK);

            output.WriteLine($"{test} 8) Provider sending login refresh[1]");
            submitOptions.Clear();
            Assert.True(provider.SubmitAndDispatch(provRefresh[1], submitOptions) >= ReactorReturnCode.SUCCESS);
            output.WriteLine($"{test} 8) Provider sent login refresh[1]");

            output.WriteLine($"{test} 9) Consumer dispatching, expects login refresh[1]");
            consumer.TestReactor.Dispatch(1);
            reactorEvent = consumer.TestReactor.PollEvent();
            Assert.Equal(TestReactorEventType.LOGIN_MSG, reactorEvent.EventType);
            loginMsgEvent = (RDMLoginMsgEvent)reactorEvent.ReactorEvent;
            Assert.Equal(LoginMsgType.REFRESH, loginMsgEvent.LoginMsg.LoginMsgType);
            Assert.Equal(MsgClasses.REFRESH, loginMsgEvent.Msg.MsgClass);
            output.WriteLine($"{test} 9) Consumer received login refresh[1]");

            consRefresh[1] = loginMsgEvent.LoginMsg.LoginRefresh;
            Assert.Equal(consRefresh[1].StreamId, loginStreamId);
            Assert.Equal(StreamStates.OPEN, consRefresh[1].State.StreamState());
            Assert.Equal(DataStates.OK, consRefresh[1].State.DataState());
            Assert.True(consRefresh[1].HasUserName);
            Assert.True(consRefresh[1].HasUserNameType);
            if (userNameType == Login.UserIdTypes.AUTHN_TOKEN)
            {
                Assert.True(consRefresh[1].UserName.Data().ReadByte() == 0x0);
                Assert.True(consRefresh[1].HasAuthenicationTTReissue);
                Assert.Equal(consRefresh[1].AuthenticationTTReissue, authenticationTTReissues[1]);
                Assert.True(consRefresh[1].HasAuthenticationExtendedResp);
                Assert.Equal(consRefresh[1].AuthenticationExtendedResp.ToString(), authenticationExtResps[1]);
            }
            else if (userNameType == Login.UserIdTypes.TOKEN)
                Assert.Equal(consRefresh[1].UserName.ToString(), userNames[1]);
            else if (userNameType != Login.UserIdTypes.COOKIE
                     && (protocolList != null && protocolList.Contains("json"))) /* JSON converter doesn't write name when nameType is COOKIE */
                Assert.Equal(consRefresh[1].UserName.ToString(), userNames[0]);
            output.WriteLine($"{test} 9) Consumer validated login refresh[0]");

            TestReactorComponent.CloseSession(consumer, provider);
            TearDownConsumerAndProvider(consumerReactor, providerReactor);
        }
        output.WriteLine($"{test} Done{NewLine}");

    }

    [Fact]
    [Category("Unit")]
    [Category("Reactor")]
    public void LoginReissueWithConnectionRecovery_BeforeLoginSubmitTest_Socket()
    {
        LoginReissueWithConnectionRecovery_BeforeLoginSubmit(false, null);
    }

    private void LoginReissueWithConnectionRecovery_BeforeLoginSubmit(bool isWebsocket, string protocolList)
    {
        string test = "loginReissueWithConnectionRecovery_BeforeLoginSubmitTest()";
        output.WriteLine(NewLine + test + " Running...");
        output.WriteLine("/*   CONS                 WatchList                 PROV\n" +
                "   1) |    Cons Request[0] -> |                       |\n" +
                "   2) |                       |    Prov Request[0] -> |\n" +
                "   3) |                       | <- Prov Refresh[0]    |\n" +
                "   4) | <- Cons Refresh[0]    |                       |\n" +
                "   X) |                       |     Disconnect        |\n" +
                "   5) |    User Request[1]    |                       |\n" +
                "   R) |    Recovery           |                       |\n" +
                "   6) |    Cons Request[1] -> |                       |\n" +
                "   7) |                       |    Prov Request[1] -> |\n" +
                "   8) |                       | <- Prov Refresh[1]    |\n" +
                "   9) | <- Cons Refresh[1]    |                       |\n" +
                "*/");

        int loginStreamId;
        TestReactorEvent reactorEvent;
        RDMLoginMsgEvent loginMsgEvent;
        ReactorSubmitOptions submitOptions = new ReactorSubmitOptions();

        LoginRequest[] consRequest = { null, null };
        LoginRefresh[] consRefresh = { null, null };
        LoginRequest[] provRequest = { null, null };
        LoginRefresh[] provRefresh = { null, null };

        // Data arrays - index [0] is used for the initial request and refresh, the other
        // indices are for the subsequent reissue requests and refreshes.
        string[] userNames = { "userName_0", "userName_1" };
        string[] authenticationTokens = { "authenticationToken_0", "authenticationToken_1" };
        string[] authenticationExts = { "authenticationExt_0", "authenticationExt_1" };
        string[] authenticationExtResps = { "authenticationExtResp_0", "authenticationExtResp_1" };
        long[] authenticationTTReissues = { 123123000, 123123001 };

        Login.UserIdTypes[] userNameTypes = { Login.UserIdTypes.NAME, Login.UserIdTypes.EMAIL_ADDRESS,
            Login.UserIdTypes.TOKEN, Login.UserIdTypes.COOKIE, Login.UserIdTypes.AUTHN_TOKEN
        };
        foreach (Login.UserIdTypes userNameType in userNameTypes)
        {
            output.WriteLine($"{test} loop: userNameType = " + userNameType);

            // Create reactors
            TestReactor consumerReactor = new TestReactor();
            TestReactor providerReactor = new TestReactor();
            consumerReactor.Reactor.m_ReactorOptions.XmlTracing = true;
            providerReactor.Reactor.m_ReactorOptions.XmlTracing = true;

            // Create consumer and the initial login request message with data using index [0]
            Consumer consumer = new Consumer(consumerReactor);
            ConsumerRole consumerRole = (ConsumerRole)consumer.ReactorRole;
            consumerRole.InitDefaultRDMLoginRequest();
            consumerRole.InitDefaultRDMDirectoryRequest();

            output.WriteLine($"{test} 1) Consumer creating login request[0]");
            consRequest[0] = consumerRole.RdmLoginRequest;
            loginStreamId = consRequest[0].StreamId;
            consRequest[0].HasUserNameType = true;
            consRequest[0].UserNameType = userNameType;

            if (userNameType == Login.UserIdTypes.AUTHN_TOKEN)
            {
                consRequest[0].UserName.Data(authenticationTokens[0]);
                consRequest[0].HasAuthenticationExtended = true;
                consRequest[0].AuthenticationExtended.Data(authenticationExts[0]);
            }
            else
                consRequest[0].UserName.Data(userNames[0]);

            consumerRole.ChannelEventCallback = consumer;
            consumerRole.LoginMsgCallback = consumer;
            consumerRole.DirectoryMsgCallback = consumer;
            consumerRole.DictionaryMsgCallback = consumer;
            consumerRole.DefaultMsgCallback = consumer;
            consumerRole.WatchlistOptions.EnableWatchlist = true;
            consumerRole.WatchlistOptions.ChannelOpenEventCallback = consumer;

            // Create provider.
            Provider provider = new Provider(providerReactor);
            ProviderRole providerRole = (ProviderRole)provider.ReactorRole;
            providerRole.ChannelEventCallback = provider;
            providerRole.LoginMsgCallback = provider;
            providerRole.DirectoryMsgCallback = provider;
            providerRole.DictionaryMsgCallback = provider;
            providerRole.DefaultMsgCallback = provider;

            // Connect the consumer and provider. Disable the automatic setup of login & directory streams.
            ConsumerProviderSessionOptions opts = new ConsumerProviderSessionOptions();
            opts.SetupDefaultLoginStream = true;
            opts.SetupDefaultDirectoryStream = true;
            opts.ReconnectAttemptLimit = -1;

            provider.Bind(opts);

            output.WriteLine($"{test} 1) Consumer sending login request[0]");
            TestReactor.OpenSession(consumer, provider, opts);
            output.WriteLine($"{test} 1) Consumer sent login request[0]");

            provider.TestReactor.Dispatch(0);
            output.WriteLine($"{test} 2,3,4) Provider and consumer already dispatched request[0]");

            output.WriteLine($"{test} X) Prov disconnect");
            /* Disconnect provider. */
            provider.CloseChannel();

            consumerReactor.Dispatch(3);
            output.WriteLine($"{test} X.1) Consumer receives channel disconnect");
            reactorEvent = consumer.TestReactor.PollEvent();
            Assert.Equal(TestReactorEventType.CHANNEL_EVENT, reactorEvent.EventType);
            ReactorChannelEvent channelEvent = (ReactorChannelEvent)reactorEvent.ReactorEvent;
            Assert.Equal(ReactorChannelEventType.CHANNEL_DOWN_RECONNECTING, channelEvent.EventType);

            output.WriteLine($"{test} X.2) Consumer receives status disconnect");
            reactorEvent = consumer.TestReactor.PollEvent();
            Assert.Equal(TestReactorEventType.LOGIN_MSG, reactorEvent.EventType);
            loginMsgEvent = (RDMLoginMsgEvent)reactorEvent.ReactorEvent;
            Assert.Equal(LoginMsgType.STATUS, loginMsgEvent.LoginMsg.LoginMsgType);

            output.WriteLine($"{test} X.3) Consumer receives DIRECTORY_MSG");
            reactorEvent = consumer.TestReactor.PollEvent();
            Assert.Equal(TestReactorEventType.DIRECTORY_MSG, reactorEvent.EventType);


            output.WriteLine($"{test} 5) User creating login request[1]");
            consumerRole.InitDefaultRDMLoginRequest();
            consRequest[1] = consumerRole.RdmLoginRequest;
            Assert.NotNull(consRequest[1]);
            consRequest[1].HasUserNameType = true;
            consRequest[1].UserNameType = userNameType;
            if (userNameType == Login.UserIdTypes.AUTHN_TOKEN)
            {
                consRequest[1].UserName.Data(authenticationTokens[1]);
                consRequest[1].HasAuthenticationExtended = true;
                consRequest[1].AuthenticationExtended.Data(authenticationExts[1]);
            }
            else if (userNameType == Login.UserIdTypes.TOKEN)
                consRequest[1].UserName.Data(userNames[1]);
            else if (userNameType != Login.UserIdTypes.COOKIE
                     && (protocolList != null && protocolList.Contains("json")))
                consRequest[1].UserName.Data(userNames[0]);

            output.WriteLine($"{test} 5) Consumer sending login request[1]");
            submitOptions.Clear();
            Assert.True(consumer.SubmitAndDispatch(consRequest[1], submitOptions) >= ReactorReturnCode.SUCCESS);
            output.WriteLine($"{test} 5) Consumer sent login request[1]");

            provider.TestReactor.Dispatch(0);
            output.WriteLine($"{test} N) provider should not receive anything before recovery");

            output.WriteLine($"{test}R) Consumer recovery");
            TestReactor.OpenSession(consumer, provider, opts, true);

            output.WriteLine($"{test}R.0) Provider recovery - still nothing at provider side");
            provider.TestReactor.Dispatch(0);

            output.WriteLine($"{test} 6) Provider recovery - consumer resending");
            Assert.True(consumer.SubmitAndDispatch(consRequest[1], submitOptions) >= ReactorReturnCode.SUCCESS);
            output.WriteLine($"{test} 7) Provider dispatching, expects login request[1]");
            provider.TestReactor.Dispatch(1);

            reactorEvent = provider.TestReactor.PollEvent();
            Assert.Equal(TestReactorEventType.LOGIN_MSG, reactorEvent.EventType);
            loginMsgEvent = (RDMLoginMsgEvent)reactorEvent.ReactorEvent;
            Assert.Equal(LoginMsgType.REQUEST, loginMsgEvent.LoginMsg.LoginMsgType);

            provRequest[1] = loginMsgEvent.LoginMsg.LoginRequest;
            Assert.Equal(provRequest[1].StreamId, loginStreamId);
            Assert.True(provRequest[1].HasUserNameType);
            Assert.Equal(provRequest[1].UserNameType, userNameType);

            if (userNameType == Login.UserIdTypes.AUTHN_TOKEN)
            {
                Assert.Equal(provRequest[1].UserName.ToString(), authenticationTokens[1]);
                Assert.True(provRequest[1].HasAuthenticationExtended);
                Assert.Equal(provRequest[1].AuthenticationExtended.ToString(), authenticationExts[1]);
            }
            else if (userNameType == Login.UserIdTypes.TOKEN)
                Assert.Equal(provRequest[1].UserName.ToString(), userNames[1]);
            else if (userNameType != Login.UserIdTypes.COOKIE
                     && (protocolList != null && protocolList.Contains("json")))
                Assert.Equal(provRequest[1].UserName.ToString(), userNames[0]);

            output.WriteLine($"{test} 7) Provider validated login request[1]");

            output.WriteLine($"{test} 8) Provider creating login refresh[1]");
            provRefresh[1] = new LoginRefresh();
            provRefresh[1].Clear();
            provRefresh[1].StreamId = loginStreamId;
            provRefresh[1].Solicited = true;
            provRefresh[1].HasUserNameType = true;
            provRefresh[1].UserNameType = userNameType;
            if (userNameType == Login.UserIdTypes.AUTHN_TOKEN)
            {
                provRefresh[1].UserName.Data(userNames[1]);
                provRefresh[1].HasAuthenicationTTReissue = true;
                provRefresh[1].AuthenticationTTReissue = authenticationTTReissues[1];
                provRefresh[1].HasAuthenticationExtendedResp = true;
                provRefresh[1].AuthenticationExtendedResp.Data(authenticationExtResps[1]);
            }
            else if (userNameType == Login.UserIdTypes.TOKEN)
            {
                provRefresh[1].HasUserName = true;
                provRefresh[1].UserName.Data(userNames[1]);
            }
            else
            {
                provRefresh[1].HasUserName = true;
                provRefresh[1].UserName.Data(userNames[0]);
            }
            provRefresh[1].State.StreamState(StreamStates.OPEN);
            provRefresh[1].State.DataState(DataStates.OK);

            output.WriteLine($"{test} 8) Provider sending login refresh[1]");
            submitOptions.Clear();
            Assert.True(provider.SubmitAndDispatch(provRefresh[1], submitOptions) >= ReactorReturnCode.SUCCESS);
            output.WriteLine($"{test} 8) Provider sent login refresh[1]");

            output.WriteLine($"{test} 9) Consumer dispatching, expects login refresh[1]");
            consumer.TestReactor.Dispatch(1);
            reactorEvent = consumer.TestReactor.PollEvent();
            Assert.Equal(TestReactorEventType.LOGIN_MSG, reactorEvent.EventType);
            loginMsgEvent = (RDMLoginMsgEvent)reactorEvent.ReactorEvent;
            Assert.Equal(LoginMsgType.REFRESH, loginMsgEvent.LoginMsg.LoginMsgType);
            Assert.Equal(MsgClasses.REFRESH, loginMsgEvent.LoginMsg.LoginRefresh.MsgClass);
            output.WriteLine($"{test} 9) Consumer received login refresh[1]");

            consRefresh[1] = loginMsgEvent.LoginMsg.LoginRefresh;
            Assert.Equal(consRefresh[1].StreamId, loginStreamId);
            Assert.Equal(StreamStates.OPEN, consRefresh[1].State.StreamState());
            Assert.Equal(DataStates.OK, consRefresh[1].State.DataState());
            Assert.True(consRefresh[1].HasUserName);
            Assert.True(consRefresh[1].HasUserNameType);
            if (userNameType == Login.UserIdTypes.AUTHN_TOKEN)
            {
                Assert.True(consRefresh[1].UserName.Data().ReadByte() == 0x0);
                Assert.True(consRefresh[1].HasAuthenicationTTReissue);
                Assert.Equal(consRefresh[1].AuthenticationTTReissue, authenticationTTReissues[1]);
                Assert.True(consRefresh[1].HasAuthenticationExtendedResp);
                Assert.Equal(consRefresh[1].AuthenticationExtendedResp.ToString(), authenticationExtResps[1]);
            }
            else if (userNameType == Login.UserIdTypes.TOKEN)
                Assert.Equal(consRefresh[1].UserName.ToString(), userNames[1]);
            else if (userNameType != Login.UserIdTypes.COOKIE && (protocolList != null && protocolList.Contains("json")))
                Assert.Equal(consRefresh[1].UserName.ToString(), userNames[0]);
            output.WriteLine($"{test} 9) Consumer validated login refresh[1]");

            TestReactorComponent.CloseSession(consumer, provider);
            TearDownConsumerAndProvider(consumerReactor, providerReactor);
        }
        output.WriteLine($"{test} Done{NewLine}");
    }

    [Fact]
    [Category("Unit")]
    [Category("Reactor")]
    public void LoginReissueWithConnectionRecovery_AfterLoginSubmitTest_Socket()
    {
        LoginReissueWithConnectionRecovery_AfterLoginSubmit(false, null);
    }

    private void LoginReissueWithConnectionRecovery_AfterLoginSubmit(bool isWebsocket, string protocolList)
    {
        string test = "loginReissueWithConnectionRecovery_AfterLoginSubmitTest()";
        output.WriteLine(NewLine + test + " Running...");
        output.WriteLine("/*   CONS                 WatchList                 PROV\n" +
                "   1) |    Cons Request[0] -> |                       |\n" +
                "   2) |                       |    Prov Request[0] -> |\n" +
                "   3) |                       | <- Prov Refresh[0]    |\n" +
                "   4) | <- Cons Refresh[0]    |                       |\n" +
                "   5) |    User Request[1]    |                       |\n" +
                "   6) |    Cons Request[1] -> |                       |\n" +
                "   7) |                       |    Prov Request[1] -> |\n" +
                "   X) |                       |     Disconnect        |\n" +
                "   R) |    Recovery           |                       |\n" +
                "   8) |    Cons Request[1] -> |                       |\n" +
                "   9) |                       |    Prov Request[1] -> |\n" +
                "  10) |                       | <- Prov Refresh[1]    |\n" +
                "  11) | <- Cons Refresh[1]    |                       |\n" +
                "*/");

        int loginStreamId;
        TestReactorEvent reactorEvent;
        RDMLoginMsgEvent loginMsgEvent;
        ReactorSubmitOptions submitOptions = new ReactorSubmitOptions();

        LoginRequest[] consRequest = { null, null };
        LoginRefresh[] consRefresh = { null, null };
        LoginRequest[] provRequest = { null, null };
        LoginRefresh[] provRefresh = { null, null };

        // Data arrays - index [0] is used for the initial request and refresh, the other
        // indices are for the subsequent reissue requests and refreshes.
        string[] userNames = { "userName_0", "userName_1" };
        string[] authenticationTokens = { "authenticationToken_0", "authenticationToken_1" };
        string[] authenticationExts = { "authenticationExt_0", "authenticationExt_1" };
        string[] authenticationExtResps = { "authenticationExtResp_0", "authenticationExtResp_1" };
        long[] authenticationTTReissues = { 123123000, 123123001 };

        Login.UserIdTypes[] userNameTypes = { Login.UserIdTypes.NAME, Login.UserIdTypes.EMAIL_ADDRESS,
            Login.UserIdTypes.TOKEN, Login.UserIdTypes.COOKIE, Login.UserIdTypes.AUTHN_TOKEN
        };
        foreach (Login.UserIdTypes userNameType in userNameTypes)
        {
            output.WriteLine($"{test} loop: userNameType = " + userNameType);

            // Create reactors
            TestReactor consumerReactor = new TestReactor();
            TestReactor providerReactor = new TestReactor();
            consumerReactor.Reactor.m_ReactorOptions.XmlTracing = true;
            providerReactor.Reactor.m_ReactorOptions.XmlTracing = true;

            // Create consumer and the initial login request message with data using index [0]
            Consumer consumer = new Consumer(consumerReactor);
            ConsumerRole consumerRole = (ConsumerRole)consumer.ReactorRole;
            consumerRole.InitDefaultRDMLoginRequest();
            consumerRole.InitDefaultRDMDirectoryRequest();

            output.WriteLine($"{test} 1) Consumer creating login request[0]");
            consRequest[0] = consumerRole.RdmLoginRequest;
            loginStreamId = consRequest[0].StreamId;
            consRequest[0].HasUserNameType = true;
            consRequest[0].UserNameType = userNameType;

            if (userNameType == Login.UserIdTypes.AUTHN_TOKEN)
            {
                consRequest[0].UserName.Data(authenticationTokens[0]);
                consRequest[0].HasAuthenticationExtended = true;
                consRequest[0].AuthenticationExtended.Data(authenticationExts[0]);
            }
            else
                consRequest[0].UserName.Data(userNames[0]);

            consumerRole.ChannelEventCallback = consumer;
            consumerRole.LoginMsgCallback = consumer;
            consumerRole.DirectoryMsgCallback = consumer;
            consumerRole.DictionaryMsgCallback = consumer;
            consumerRole.DefaultMsgCallback = consumer;
            consumerRole.WatchlistOptions.EnableWatchlist = true;
            consumerRole.WatchlistOptions.ChannelOpenEventCallback = consumer;

            // Create provider.
            Provider provider = new Provider(providerReactor);
            ProviderRole providerRole = (ProviderRole)provider.ReactorRole;
            providerRole.ChannelEventCallback = provider;
            providerRole.LoginMsgCallback = provider;
            providerRole.DirectoryMsgCallback = provider;
            providerRole.DictionaryMsgCallback = provider;
            providerRole.DefaultMsgCallback = provider;

            // Connect the consumer and provider. Disable the automatic setup of login & directory streams.
            ConsumerProviderSessionOptions opts = new ConsumerProviderSessionOptions();
            opts.SetupDefaultLoginStream = true;
            opts.SetupDefaultDirectoryStream = true;
            opts.ReconnectAttemptLimit = -1;

            provider.Bind(opts);

            output.WriteLine($"{test} 1) Consumer sending login request[0]");
            TestReactor.OpenSession(consumer, provider, opts);
            output.WriteLine($"{test} 1) Consumer sent login request[0]");

            provider.TestReactor.Dispatch(0);
            output.WriteLine($"{test} 2,3,4) Provider and consumer already dispatched request[0]");

            output.WriteLine($"{test} 5) User creating login request[1]");
            consumerRole.InitDefaultRDMLoginRequest();
            consRequest[1] = consumerRole.RdmLoginRequest;
            Assert.NotNull(consRequest[1]);
            consRequest[1].HasUserNameType = true;
            consRequest[1].UserNameType = userNameType;
            if (userNameType == Login.UserIdTypes.AUTHN_TOKEN)
            {
                consRequest[1].UserName.Data(authenticationTokens[1]);
                consRequest[1].HasAuthenticationExtended = true;
                consRequest[1].AuthenticationExtended.Data(authenticationExts[1]);
            }
            else if (userNameType == Login.UserIdTypes.TOKEN)
                consRequest[1].UserName.Data(userNames[1]);
            else
                consRequest[1].UserName.Data(userNames[0]);

            output.WriteLine($"{test} 6) Consumer sending login request[1]");
            submitOptions.Clear();
            Assert.True(consumer.SubmitAndDispatch(consRequest[1], submitOptions) >= ReactorReturnCode.SUCCESS);
            output.WriteLine($"{test} 6) Consumer sent login request[1]");


            output.WriteLine($"{test} 7) Provider dispatching, expects login request[1]");
            provider.TestReactor.Dispatch(1);

            reactorEvent = provider.TestReactor.PollEvent();
            Assert.Equal(TestReactorEventType.LOGIN_MSG, reactorEvent.EventType);
            loginMsgEvent = (RDMLoginMsgEvent)reactorEvent.ReactorEvent;
            Assert.Equal(LoginMsgType.REQUEST, loginMsgEvent.LoginMsg.LoginMsgType);

            provRequest[1] = loginMsgEvent.LoginMsg.LoginRequest;
            Assert.Equal(provRequest[1].StreamId, loginStreamId);
            Assert.True(provRequest[1].HasUserNameType);
            Assert.Equal(provRequest[1].UserNameType, userNameType);

            if (userNameType == Login.UserIdTypes.AUTHN_TOKEN)
            {
                Assert.Equal(provRequest[1].UserName.ToString(), authenticationTokens[1]);
                Assert.True(provRequest[1].HasAuthenticationExtended);
                Assert.Equal(provRequest[1].AuthenticationExtended.ToString(), authenticationExts[1]);
            }
            else if (userNameType == Login.UserIdTypes.TOKEN)
                Assert.Equal(provRequest[1].UserName.ToString(), userNames[1]);
            else if (userNameType != Login.UserIdTypes.COOKIE && (protocolList != null && protocolList.Contains("json")))
                Assert.Equal(provRequest[1].UserName.ToString(), userNames[0]);

            output.WriteLine($"{test} 7) Provider validated login request[1]");

            output.WriteLine($"{test} X) Prov disconnect");
            /* Disconnect provider. */
            provider.CloseChannel();

            consumerReactor.Dispatch(3);

            output.WriteLine($"{test} X.1) Consumer receives channel disconnect");
            reactorEvent = consumer.TestReactor.PollEvent();
            Assert.Equal(TestReactorEventType.CHANNEL_EVENT, reactorEvent.EventType);
            ReactorChannelEvent channelEvent = (ReactorChannelEvent)reactorEvent.ReactorEvent;
            Assert.Equal(ReactorChannelEventType.CHANNEL_DOWN_RECONNECTING, channelEvent.EventType);

            output.WriteLine($"{test} X.2) Consumer receives status disconnect");
            reactorEvent = consumer.TestReactor.PollEvent();
            Assert.Equal(TestReactorEventType.LOGIN_MSG, reactorEvent.EventType);
            loginMsgEvent = (RDMLoginMsgEvent)reactorEvent.ReactorEvent;
            Assert.Equal(LoginMsgType.STATUS, loginMsgEvent.LoginMsg.LoginMsgType);

            output.WriteLine($"{test} X.3) Consumer receives DIRECTORY_MSG");
            reactorEvent = consumer.TestReactor.PollEvent();
            Assert.Equal(TestReactorEventType.DIRECTORY_MSG, reactorEvent.EventType);

            provider.TestReactor.Dispatch(0);
            output.WriteLine($"{test} X) provider should not receive anything before recovery");

            output.WriteLine($"{test}R) Consumer recovery");
            TestReactor.OpenSession(consumer, provider, opts, true);

            output.WriteLine($"{test}R.0) Provider recovery - still nothing at provider side");
            provider.TestReactor.Dispatch(0);

            output.WriteLine($"{test} 8) Provider recovery - consumer resending");
            Assert.True(consumer.SubmitAndDispatch(consRequest[1], submitOptions) >= ReactorReturnCode.SUCCESS);


            output.WriteLine($"{test} 9) Provider dispatching, expects login request[1]");
            provider.TestReactor.Dispatch(1);

            reactorEvent = provider.TestReactor.PollEvent();
            Assert.Equal(TestReactorEventType.LOGIN_MSG, reactorEvent.EventType);
            loginMsgEvent = (RDMLoginMsgEvent)reactorEvent.ReactorEvent;
            Assert.Equal(LoginMsgType.REQUEST, loginMsgEvent.LoginMsg.LoginMsgType);

            provRequest[1] = loginMsgEvent.LoginMsg.LoginRequest;
            Assert.Equal(provRequest[1].StreamId, loginStreamId);
            Assert.True(provRequest[1].HasUserNameType);
            Assert.Equal(provRequest[1].UserNameType, userNameType);

            if (userNameType == Login.UserIdTypes.AUTHN_TOKEN)
            {
                Assert.Equal(provRequest[1].UserName.ToString(), authenticationTokens[1]);
                Assert.True(provRequest[1].HasAuthenticationExtended);
                Assert.Equal(provRequest[1].AuthenticationExtended.ToString(), authenticationExts[1]);
            }
            else if (userNameType == Login.UserIdTypes.TOKEN)
                Assert.Equal(provRequest[1].UserName.ToString(), userNames[1]);
            else if (userNameType != Login.UserIdTypes.COOKIE
                     && (protocolList != null && protocolList.Contains("json")))
                Assert.Equal(provRequest[1].UserName.ToString(), userNames[0]);

            output.WriteLine($"{test} 9) Provider validated login request[1]");

            output.WriteLine($"{test} 10) Provider creating login refresh[1]");
            provRefresh[1] = new LoginRefresh();
            provRefresh[1].Clear();
            provRefresh[1].StreamId = loginStreamId;
            provRefresh[1].Solicited = true;
            provRefresh[1].HasUserNameType = true;
            provRefresh[1].UserNameType = userNameType;
            if (userNameType == Login.UserIdTypes.AUTHN_TOKEN)
            {
                provRefresh[1].UserName.Data(userNames[1]);
                provRefresh[1].HasAuthenicationTTReissue = true;
                provRefresh[1].AuthenticationTTReissue = authenticationTTReissues[1];
                provRefresh[1].HasAuthenticationExtendedResp = true;
                provRefresh[1].AuthenticationExtendedResp.Data(authenticationExtResps[1]);
            }
            else if (userNameType == Login.UserIdTypes.TOKEN)
            {
                provRefresh[1].HasUserName = true;
                provRefresh[1].UserName.Data(userNames[1]);
            }
            else
            {
                provRefresh[1].HasUserName = true;
                provRefresh[1].UserName.Data(userNames[0]);
            }
            provRefresh[1].State.StreamState(StreamStates.OPEN);
            provRefresh[1].State.DataState(DataStates.OK);

            output.WriteLine($"{test} 10) Provider sending login refresh[1]");
            submitOptions.Clear();
            Assert.True(provider.SubmitAndDispatch(provRefresh[1], submitOptions) >= ReactorReturnCode.SUCCESS);
            output.WriteLine($"{test} 10) Provider sent login refresh[1]");

            output.WriteLine($"{test} 11) Consumer dispatching, expects login refresh[1]");

            consumer.TestReactor.Dispatch(1);
            reactorEvent = consumer.TestReactor.PollEvent();

            Assert.Equal(TestReactorEventType.LOGIN_MSG, reactorEvent.EventType);
            loginMsgEvent = (RDMLoginMsgEvent)reactorEvent.ReactorEvent;
            Assert.Equal(LoginMsgType.REFRESH, loginMsgEvent.LoginMsg.LoginMsgType);
            Assert.Equal(MsgClasses.REFRESH, loginMsgEvent.LoginMsg.LoginRefresh.MsgClass);
            output.WriteLine($"{test} 11) Consumer received login refresh[1]");

            consRefresh[1] = loginMsgEvent.LoginMsg.LoginRefresh;
            Assert.Equal(consRefresh[1].StreamId, loginStreamId);
            Assert.Equal(StreamStates.OPEN, consRefresh[1].State.StreamState());
            Assert.Equal(DataStates.OK, consRefresh[1].State.DataState());
            Assert.True(consRefresh[1].HasUserName);
            Assert.True(consRefresh[1].HasUserNameType);
            if (userNameType == Login.UserIdTypes.AUTHN_TOKEN)
            {
                Assert.True(consRefresh[1].UserName.Data().ReadByte() == 0x0);
                Assert.True(consRefresh[1].HasAuthenicationTTReissue);
                Assert.Equal(consRefresh[1].AuthenticationTTReissue, authenticationTTReissues[1]);
                Assert.True(consRefresh[1].HasAuthenticationExtendedResp);
                Assert.Equal(consRefresh[1].AuthenticationExtendedResp.ToString(), authenticationExtResps[1]);
            }
            else if (userNameType == Login.UserIdTypes.TOKEN)
                Assert.Equal(consRefresh[1].UserName.ToString(), userNames[1]);
            else if (userNameType != Login.UserIdTypes.COOKIE && (protocolList != null && protocolList.Contains("json")))
                Assert.Equal(consRefresh[1].UserName.ToString(), userNames[0]);
            output.WriteLine($"{test} 11) Consumer validated login refresh[1]");

            TestReactorComponent.CloseSession(consumer, provider);
            TearDownConsumerAndProvider(consumerReactor, providerReactor);
        }
        output.WriteLine($"{test} Done{NewLine}");
    }


    [Fact]
    [Category("Unit")]
    [Category("Reactor")]
    public void SubmitOffstreamPostOnItemRefeshTest_Socket()
    {

        SubmitOffstreamPostOnItemRefesh(false, null);
    }

    private void SubmitOffstreamPostOnItemRefesh(bool isWebsocket, string protocolList)
    {

        ReactorSubmitOptions submitOptions = new ReactorSubmitOptions();
        TestReactorEvent reactorEvent;
        ReactorMsgEvent msgEvent;
        Msg msg = new Msg();
        IRequestMsg requestMsg = (IRequestMsg)msg;
        IRequestMsg receivedRequestMsg;
        IRefreshMsg refreshMsg = (IRefreshMsg)msg;
        IRefreshMsg receivedRefreshMsg;
        int providerStreamId;

        /* Create reactors. */
        TestReactor consumerReactor = new TestReactor();
        TestReactor providerReactor = new TestReactor();

        /* Create consumer. */
        Consumer consumer = new OffPostFromDefaultMsgCallbackConsumer(consumerReactor);
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
        Provider provider = new Provider(providerReactor);
        ProviderRole providerRole = (ProviderRole)provider.ReactorRole;
        providerRole.ChannelEventCallback = provider;
        providerRole.LoginMsgCallback = provider;
        providerRole.DirectoryMsgCallback = provider;
        providerRole.DictionaryMsgCallback = provider;
        providerRole.DefaultMsgCallback = provider;

        /* Connect the consumer and provider. Setup login & directory streams automatically. */
        ConsumerProviderSessionOptions opts = new ConsumerProviderSessionOptions();
        opts.SetupDefaultLoginStream = true;
        opts.SetupDefaultDirectoryStream = true;

        provider.Bind(opts);

        TestReactor.OpenSession(consumer, provider, opts);

        /* Consumer sends request. */
        requestMsg.Clear();
        requestMsg.MsgClass = MsgClasses.REQUEST;
        requestMsg.StreamId = 5;
        requestMsg.DomainType = (int)DomainType.MARKET_PRICE;
        requestMsg.ApplyStreaming();
        requestMsg.MsgKey.ApplyHasName();
        requestMsg.MsgKey.Name.Data("TRI.N");
        submitOptions.Clear();
        submitOptions.ServiceName = Provider.DefaultService.Info.ServiceName.ToString();
        Assert.True(consumer.SubmitAndDispatch((Msg)requestMsg, submitOptions) >= ReactorReturnCode.SUCCESS);

        /* Provider receives request. */
        providerReactor.Dispatch(1);
        reactorEvent = providerReactor.PollEvent();
        Assert.Equal(TestReactorEventType.MSG, reactorEvent.EventType);
        msgEvent = (ReactorMsgEvent)reactorEvent.ReactorEvent;
        Assert.Equal(MsgClasses.REQUEST, msgEvent.Msg.MsgClass);

        receivedRequestMsg = (IRequestMsg)msgEvent.Msg;
        Assert.True(receivedRequestMsg.MsgKey.CheckHasServiceId());
        Assert.True(receivedRequestMsg.CheckStreaming());
        Assert.False(receivedRequestMsg.CheckNoRefresh());
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
        refreshMsg.ApplySolicited();
        refreshMsg.DomainType = (int)DomainType.MARKET_PRICE;
        refreshMsg.StreamId = providerStreamId;
        refreshMsg.ContainerType = DataTypes.NO_DATA;
        refreshMsg.ApplyHasMsgKey();
        refreshMsg.MsgKey.ApplyHasServiceId();
        refreshMsg.MsgKey.ServiceId = Provider.DefaultService.ServiceId;
        refreshMsg.MsgKey.ApplyHasName();
        refreshMsg.MsgKey.Name.Data("TRI.N");
        refreshMsg.ApplyRefreshComplete();
        Codec.Buffer groupId = new();
        groupId.Data("1234431");
        refreshMsg.GroupId = groupId;
        refreshMsg.State.StreamState(StreamStates.OPEN);
        refreshMsg.State.DataState(DataStates.OK);

        Assert.True(provider.SubmitAndDispatch((Msg)refreshMsg, submitOptions) >= ReactorReturnCode.SUCCESS);

        /* Consumer receives refresh. */
        consumerReactor.Dispatch(1);
        reactorEvent = consumerReactor.PollEvent();
        Assert.Equal(TestReactorEventType.MSG, reactorEvent.EventType);
        msgEvent = (ReactorMsgEvent)reactorEvent.ReactorEvent;
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
        Assert.NotNull(msgEvent.StreamInfo);
        Assert.NotNull(msgEvent.StreamInfo.ServiceName);
        Assert.Equal(msgEvent.StreamInfo.ServiceName, Provider.DefaultService.Info.ServiceName.ToString());

        /* Provider receives offstream post. */
        IPostMsg offstreamPost;
        providerReactor.Dispatch(1);
        reactorEvent = providerReactor.PollEvent();
        Assert.Equal(TestReactorEventType.MSG, reactorEvent.EventType);
        msgEvent = (ReactorMsgEvent)reactorEvent.ReactorEvent;
        Assert.Equal(MsgClasses.POST, msgEvent.Msg.MsgClass);
        Assert.Equal(consumerRole.RdmLoginRequest.StreamId, msgEvent.Msg.StreamId);
        offstreamPost = (IPostMsg)msgEvent.Msg;

        // provider sends post ACK to consumer
        IAckMsg ackMsg = (IAckMsg)new Msg();
        ackMsg.MsgClass = MsgClasses.ACK;
        ackMsg.StreamId = consumerRole.RdmLoginRequest.StreamId;
        ackMsg.DomainType = offstreamPost.DomainType;
        ackMsg.AckId = offstreamPost.PostId;
        ackMsg.ApplyHasNakCode();
        ackMsg.NakCode = NakCodes.NONE;
        ackMsg.ApplyHasSeqNum();
        ackMsg.SeqNum = offstreamPost.SeqNum;
        Assert.True(provider.SubmitAndDispatch((Msg)ackMsg, submitOptions) >= ReactorReturnCode.SUCCESS);

        /* Consumer receives ACK. */
        consumerReactor.Dispatch(1);
        reactorEvent = consumerReactor.PollEvent();
        Assert.Equal(TestReactorEventType.MSG, reactorEvent.EventType);
        msgEvent = (ReactorMsgEvent)reactorEvent.ReactorEvent;
        Assert.Equal(MsgClasses.ACK, msgEvent.Msg.MsgClass);

        // extract refresh message from post and send to consumer
        DecodeIterator dIter = new DecodeIterator();
        dIter.SetBufferAndRWFVersion(offstreamPost.EncodedDataBody, msgEvent.ReactorChannel.MajorVersion, msgEvent.ReactorChannel.MinorVersion);
        Msg extractedMsg = new Msg();
        Assert.Equal(CodecReturnCode.SUCCESS, extractedMsg.Decode(dIter));
        if (protocolList == null || (!protocolList.Contains("json"))) /* JSON Converter uses some default values for Refresh message, hence flags do not coincide */
            Assert.Equal(0, extractedMsg.Flags);
        extractedMsg.StreamId = 3;
        Assert.True(provider.SubmitAndDispatch(extractedMsg, submitOptions) >= ReactorReturnCode.SUCCESS);

        if (protocolList == null || (!protocolList.Contains("json")))
        {
            /* Consumer receives extracted refresh message. */
            consumerReactor.Dispatch(1);
            reactorEvent = consumerReactor.PollEvent();
            Assert.Equal(TestReactorEventType.MSG, reactorEvent.EventType);
            msgEvent = (ReactorMsgEvent)reactorEvent.ReactorEvent;
            Assert.Equal(MsgClasses.REFRESH, msgEvent.Msg.MsgClass);
            Assert.Equal(0, msgEvent.Msg.Flags);
        }

        // sleep for 7 seconds and no more consumer events should be received
        try
        {
            Thread.Sleep(7000);
        }
        catch (Exception) { }

        consumerReactor.Dispatch(0);

        TestReactorComponent.CloseSession(consumer, provider);
        TearDownConsumerAndProvider(consumerReactor, providerReactor);
    }

    /* Used by submitOffstreamPostOnItemRefeshTest. */
    class OffPostFromDefaultMsgCallbackConsumer : Consumer
    {
        public OffPostFromDefaultMsgCallbackConsumer(TestReactor testReactor) : base(testReactor)
        {
        }


        public override ReactorCallbackReturnCode DefaultMsgCallback(ReactorMsgEvent reactorEvent)
        {
            base.DefaultMsgCallback(reactorEvent);

            IMsg msg = reactorEvent.Msg;

            switch (msg.MsgClass)
            {
                case MsgClasses.REFRESH:
                    // if solicited, send off stream post message with refresh message as payload
                    if ((msg.Flags & RefreshMsgFlags.SOLICITED) > 0)
                    {
                        IPostMsg postMsg = (IPostMsg)new Msg();
                        postMsg.Clear();
                        postMsg.MsgClass = MsgClasses.POST;
                        postMsg.StreamId = 1;
                        postMsg.DomainType = msg.DomainType;
                        postMsg.ContainerType = DataTypes.MSG;
                        postMsg.ApplyAck();
                        postMsg.ApplyHasPostId();
                        postMsg.PostId = 1;
                        postMsg.ApplyHasSeqNum();
                        postMsg.SeqNum = 1;
                        postMsg.ApplyHasMsgKey();
                        postMsg.MsgKey.ApplyHasServiceId();
                        postMsg.MsgKey.ApplyHasName();
                        postMsg.MsgKey.ApplyHasNameType();
                        postMsg.MsgKey.ServiceId = msg.MsgKey.ServiceId;
                        postMsg.MsgKey.NameType = 1;
                        postMsg.MsgKey.Name = msg.MsgKey.Name;

                        msg.Flags &= ~RefreshMsgFlags.SOLICITED;

                        EncodeIterator eIter = new EncodeIterator();
                        Codec.Buffer buffer = new();
                        buffer.Data(new ByteBuffer(1024));
                        eIter.SetBufferAndRWFVersion(buffer, reactorEvent.ReactorChannel.MajorVersion, reactorEvent.ReactorChannel.MinorVersion);
                        // reset all flags and set stream id to 0 to simulate bad refresh message
                        msg.Flags = 0;
                        msg.StreamId = 0;
                        msg.Encode(eIter);
                        postMsg.EncodedDataBody = buffer;

                        ReactorSubmitOptions submitOptions = new ReactorSubmitOptions();
                        ReactorErrorInfo errorInfo = new ReactorErrorInfo();
                        if ((reactorEvent.ReactorChannel.Submit((Msg)postMsg, submitOptions, out errorInfo)) != ReactorReturnCode.SUCCESS)
                        {
                            Assert.Fail("defaultMsgCallback() submit post failed");
                        }
                    }
                    break;
                default:
                    break;
            }

            return ReactorCallbackReturnCode.SUCCESS;
        }
    }
}