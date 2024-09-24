/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2023 LSEG. All rights reserved.     
 *|-----------------------------------------------------------------------------
 */

using System;
using System.Collections.Generic;
using System.Threading;

using Xunit;
using Xunit.Abstractions;
using Xunit.Categories;

using LSEG.Eta.Codec;
using LSEG.Eta.ValueAdd.Reactor;
using LSEG.Eta.Rdm;
using LSEG.Eta.ValueAdd.Rdm;
using LSEG.Eta.Example.Common;

using static LSEG.Eta.Rdm.Directory;

namespace LSEG.Eta.ValuedAdd.Tests;


/// <summary>
/// Implements and tests basic request-response operations similar to those that can be
/// executed manually using Value Add Consumer and Provider training applications.
/// </summary>
///
[Collection("ValueAdded")]
public class ReactorBasicItemsDomainTest : IDisposable
{
    #region Common variables and methods

    /// Capture output from tests
    private readonly ITestOutputHelper output;

    private const string DICTIONARY_FILE_NAME = "../../../../Src/Tests/test_FieldDictionary";
    private const string ENUM_TYPE_FILE_NAME = "../../../../Src/Tests/test_enumtype.def";

    private const string USER_NAME = "User's Name";
    private const string USER_PASSWORD = "Their password";
    private const string LOGIN_ACCEPTED = "Login accepted by test host";
    private const string APPLICATION_ID = "256";
    private const string APPLICATION_NAME = "ETA Test Provider";

    private const string VENDOR = "LSEG";
    private const string FIELD_DICTIONARY_NAME = "RWFFld";
    private const string ENUM_TYPE_DICTIONARY_NAME = "RWFEnum";
    private const string LINK_NAME = "ETA Provider Link";
    private const int OPEN_LIMIT = 10;


    private static readonly List<string> DEFAULT_VIEW_FIELD_LIST = new List<string> { "6", "22", "25", "32" };

    private DataDictionary m_Dictionary = new();


    // init
    public ReactorBasicItemsDomainTest(ITestOutputHelper output)
    {
        this.output = output;

        m_Dictionary.Clear();

        Assert.Equal(CodecReturnCode.SUCCESS,
            m_Dictionary.LoadFieldDictionary(DICTIONARY_FILE_NAME, out _));

        Assert.Equal(CodecReturnCode.SUCCESS,
            m_Dictionary.LoadEnumTypeDictionary(ENUM_TYPE_FILE_NAME, out _));
    }

    // tearDown
    public void Dispose()
    {
        m_Dictionary.Clear();
        m_Dictionary = null;
    }

    private void TearDownConsumerAndProvider(TestReactor consumerReactor, TestReactor providerReactor)
    {
        consumerReactor.Close();
        providerReactor.Close();
    }

    #endregion

    #region Basic Login

    /// <summary>
    /// Perform basic login request-response
    /// </summary>
    ///
    /// This test method follows the "normal" path: a Consumer sends LoginRequest
    /// message to Provider, provider inspects it and sends LoginRefresh in
    /// responce. Consumer receives it and inspects.
    ///
    [Fact]
    public void TestBasicLogin()
    {
        ReactorSubmitOptions submitOptions = new ReactorSubmitOptions();
        submitOptions.Clear();

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

        Provider provider = new Provider(providerReactor);
        ProviderRole providerRole = (ProviderRole)provider.ReactorRole;
        providerRole.ChannelEventCallback = provider;
        providerRole.LoginMsgCallback = provider;
        providerRole.DirectoryMsgCallback = provider;
        providerRole.DictionaryMsgCallback = provider;
        providerRole.DefaultMsgCallback = provider;

        // Connect the consumer and provider. Setup login & directory streams automatically.
        ConsumerProviderSessionOptions opts = new ConsumerProviderSessionOptions();
        opts.SetupDefaultLoginStream = true;
        opts.SetupDefaultDirectoryStream = true;

        provider.Bind(opts);

        TestReactor.OpenSession(consumer, provider, opts);

        consumerRole.RdmLoginRequest.HasUserNameType = true;
        consumerRole.RdmLoginRequest.UserNameType = Login.UserIdTypes.NAME;
        consumerRole.RdmLoginRequest.UserName.Data(USER_NAME);
        consumerRole.RdmLoginRequest.HasPassword = true;
        consumerRole.RdmLoginRequest.Password.Data(USER_PASSWORD);

        // Login request is sent from the Consumer to Provider
        Assert.True(consumer.SubmitAndDispatch(consumerRole.RdmLoginRequest, submitOptions) >= ReactorReturnCode.SUCCESS);

        // Provider receives Login request and inspects it before sending response
        providerReactor.Dispatch(1);
        TestReactorEvent evt = providerReactor.PollEvent();

        Assert.Equal(TestReactorEventType.LOGIN_MSG, evt.EventType);
        ReactorMsgEvent msgEvent = (ReactorMsgEvent)evt.ReactorEvent;
        Assert.Equal(MsgClasses.REQUEST, msgEvent.Msg.MsgClass);

        RDMLoginMsgEvent loginEvent = msgEvent as RDMLoginMsgEvent;
        Assert.NotNull(loginEvent);
        Assert.NotNull(loginEvent.LoginMsg);

        var rLoginRequest = loginEvent.LoginMsg.LoginRequest;
        Assert.NotNull(rLoginRequest);

        // check that user name and password match our expectations
        Assert.Equal(USER_NAME, rLoginRequest.UserName.ToString());
        Assert.Equal(USER_PASSWORD, rLoginRequest.Password.ToString());

        // and if they do, send Login Refresh in response
        LoginRefresh loginRefresh = new LoginRefresh();
        loginRefresh.Clear();
        loginRefresh.StreamId = rLoginRequest.StreamId;
        loginRefresh.HasUserName = true;
        loginRefresh.UserName.Data(rLoginRequest.UserName.Data());
        loginRefresh.HasUserNameType = rLoginRequest.HasUserNameType;
        loginRefresh.UserNameType = rLoginRequest.UserNameType;
        loginRefresh.State.Code(StateCodes.NONE);
        loginRefresh.State.DataState(DataStates.OK);
        loginRefresh.State.StreamState(StreamStates.OPEN);
        loginRefresh.State.Text().Data(LOGIN_ACCEPTED);
        loginRefresh.Solicited = true;
        loginRefresh.HasAttrib = true;
        loginRefresh.LoginAttrib.HasApplicationId = true;
        loginRefresh.LoginAttrib.ApplicationId.Data(APPLICATION_ID);
        loginRefresh.LoginAttrib.HasApplicationName = true;
        loginRefresh.LoginAttrib.ApplicationName.Data(APPLICATION_NAME);
        loginRefresh.SupportedFeatures.HasSupportPost = true;
        loginRefresh.SupportedFeatures.SupportOMMPost = 1;

        if (rLoginRequest.HasAttrib && rLoginRequest.LoginAttrib.HasPosition)
        {
            loginRefresh.LoginAttrib.HasPosition = true;
            loginRefresh.LoginAttrib.Position.Data(rLoginRequest.LoginAttrib.Position.Data(),
                rLoginRequest.LoginAttrib.Position.Position, rLoginRequest.LoginAttrib.Position.Length);
        }

        // this provider does not support singleOpen behavior
        loginRefresh.LoginAttrib.HasSingleOpen = true;
        loginRefresh.LoginAttrib.SingleOpen = 0;

        // this provider supports batch requests
        loginRefresh.HasFeatures = true;
        loginRefresh.SupportedFeatures.HasSupportBatchRequests = true;
        loginRefresh.SupportedFeatures.SupportBatchRequests = 1;

        loginRefresh.SupportedFeatures.HasSupportPost = true;
        loginRefresh.SupportedFeatures.SupportOMMPost = 1;

        // keep default values for remaining options

        // Login Response (Refresh) is sent from Provider to the Consumer
        Assert.True(provider.SubmitAndDispatch(loginRefresh, submitOptions) >= ReactorReturnCode.SUCCESS);

        // Consumer receives Login Response (Refresh) and inspects it
        consumerReactor.Dispatch(1);
        evt = consumerReactor.PollEvent();

        Assert.Equal(TestReactorEventType.LOGIN_MSG, evt.EventType);
        msgEvent = (ReactorMsgEvent)evt.ReactorEvent;
        Assert.Equal(MsgClasses.REFRESH, msgEvent.Msg.MsgClass);

        loginEvent = msgEvent as RDMLoginMsgEvent;
        Assert.NotNull(loginEvent);
        Assert.NotNull(loginEvent.LoginMsg);

        var rLoginRefresh = loginEvent.LoginMsg.LoginRefresh;
        Assert.NotNull(rLoginRefresh);

        Assert.Equal(rLoginRequest.StreamId, rLoginRefresh.StreamId);
        Assert.True(rLoginRefresh.HasUserName);
        Assert.Equal(USER_NAME, rLoginRefresh.UserName.ToString());
        Assert.True(rLoginRefresh.HasUserNameType);
        Assert.Equal(rLoginRequest.UserNameType, rLoginRefresh.UserNameType);
        Assert.Equal(StateCodes.NONE, rLoginRefresh.State.Code());
        Assert.Equal(DataStates.OK, rLoginRefresh.State.DataState());
        Assert.Equal(StreamStates.OPEN, rLoginRefresh.State.StreamState());
        Assert.Equal(LOGIN_ACCEPTED, rLoginRefresh.State.Text().ToString());
        Assert.True(rLoginRefresh.Solicited);
        Assert.True(rLoginRefresh.HasAttrib);
        Assert.True(rLoginRefresh.LoginAttrib.HasApplicationId);
        Assert.Equal(APPLICATION_ID, rLoginRefresh.LoginAttrib.ApplicationId.ToString());
        Assert.True(rLoginRefresh.LoginAttrib.HasApplicationName);
        Assert.Equal(APPLICATION_NAME, rLoginRefresh.LoginAttrib.ApplicationName.ToString());

        if (rLoginRequest.HasAttrib && rLoginRequest.LoginAttrib.HasPosition)
        {
            Assert.True(rLoginRefresh.LoginAttrib.HasPosition);
            Assert.Equal(consumerRole.RdmLoginRequest.LoginAttrib.Position.ToString(),
                rLoginRefresh.LoginAttrib.Position.ToString());
        }

        // this provider does not support singleOpen behavior
        Assert.True(rLoginRefresh.LoginAttrib.HasSingleOpen);
        Assert.Equal(0, rLoginRefresh.LoginAttrib.SingleOpen);

        // this provider supports batch requests
        Assert.True(rLoginRefresh.HasFeatures);
        Assert.True(rLoginRefresh.SupportedFeatures.HasSupportBatchRequests);
        Assert.Equal(1, rLoginRefresh.SupportedFeatures.SupportBatchRequests);

        Assert.True(rLoginRefresh.SupportedFeatures.HasSupportPost);
        Assert.Equal(1, rLoginRefresh.SupportedFeatures.SupportOMMPost);

        // Login Request was sent by the Consumer, received by Provider, and Login Refresh
        // message was sent in response. Consumer received this response and checked that
        // it matches expectations.

        // Test is over, clean up
        TestReactorComponent.CloseSession(consumer, provider);
        TearDownConsumerAndProvider(consumerReactor, providerReactor);
    }

    #endregion

    #region Basic Directory and Dictionary requests

    [Fact]
    public void TestBasicDictionaryDirectoryRequest()
    {
        ReactorSubmitOptions submitOptions = new ReactorSubmitOptions();
        submitOptions.Clear();

        TestReactor consumerReactor = new TestReactor();
        TestReactor providerReactor = new TestReactor();

        Consumer consumer = new Consumer(consumerReactor);
        Provider provider = new Provider(providerReactor);

        BasicDictionaryDirectoryRequestDo(submitOptions, consumerReactor, providerReactor, consumer, provider, out _);

        // Test is over, clean up
        TestReactorComponent.CloseSession(consumer, provider);
        TearDownConsumerAndProvider(consumerReactor, providerReactor);
    }

    #endregion

    #region Basic Item Requests

    /// Perform a simple randezvous: consumer establishes a channel to the provider and
    /// sends a MarketPriceRequest (from the VAConsumer example application), provider
    /// receives it and sends a corresponding response.
    [Fact]
    public void TestBasicItemRequest()
    {
        ReactorSubmitOptions submitOptions = new ReactorSubmitOptions();
        submitOptions.Clear();

        TestReactor consumerReactor = new TestReactor();
        TestReactor providerReactor = new TestReactor();
        LoginRefresh loginInfo = new();

        Consumer consumer = new Consumer(consumerReactor);
        Provider provider = new Provider(providerReactor);

        // Preform basic setup, including directory and dictionary downloads
        BasicDictionaryDirectoryRequestDo(submitOptions, consumerReactor, providerReactor, consumer, provider, out var service);

        // Send simple Market Price Request
        MarketPriceRequest marketPriceRequest = new();
        GenerateMarketPriceRequest(marketPriceRequest, true, service, loginInfo, viewFieldList: DEFAULT_VIEW_FIELD_LIST);

        marketPriceRequest.ItemNames.Add("ITEM1");

        Assert.True(consumer.SubmitAndDispatch(marketPriceRequest, submitOptions) >= ReactorReturnCode.SUCCESS);

        // Provider has to handle it and send Market Price Refresh in response, but first check the message
        providerReactor.Dispatch(1);
        TestReactorEvent evt = providerReactor.PollEvent();
        Assert.Equal(TestReactorEventType.MSG, evt.EventType);
        ReactorMsgEvent msgEvent = (ReactorMsgEvent)evt.ReactorEvent;
        Assert.Equal(MsgClasses.REQUEST, msgEvent.Msg.MsgClass);
        Assert.Equal((int)DomainType.MARKET_PRICE, msgEvent.Msg.DomainType);
        Assert.NotNull(msgEvent.Msg.MsgKey);
        Assert.True(msgEvent.Msg.MsgKey.CheckHasName());
        Assert.NotNull(msgEvent.Msg.MsgKey.Name);
        Assert.False(string.IsNullOrEmpty(msgEvent.Msg.MsgKey.Name.ToString()));
        // msgKey.name Required in initial request, otherwise optional. Specifies the name of the requested item.
        Assert.Equal("ITEM1", msgEvent.Msg.MsgKey.Name.ToString());

        MarketPriceRefresh marketPriceRefresh = new();
        MarketPriceItem marketPriceItem = new MarketPriceItem();
        EncodeMarketPriceRefresh(marketPriceRefresh, m_Dictionary, marketPriceItem, "ITEM");

        Assert.True(provider.SubmitAndDispatch(marketPriceRefresh, submitOptions) >= ReactorReturnCode.SUCCESS);

        // now consumer has the market price refresh message and has to handle it
        consumerReactor.Dispatch(1);
        evt = consumerReactor.PollEvent();
        Assert.NotNull(evt);
        msgEvent = (ReactorMsgEvent)evt.ReactorEvent;
        Assert.NotNull(msgEvent.Msg);
        Assert.Equal((int)DomainType.MARKET_PRICE, msgEvent.Msg.DomainType);

        // Test is over, clean up
        TestReactorComponent.CloseSession(consumer, provider);
        TearDownConsumerAndProvider(consumerReactor, providerReactor);
    }

    /// <summary>
    /// There is a difference in how a batch request is handled.
    /// </summary>
    [Fact]
    public void TestBasicBatchItemRequest()
    {
        ReactorSubmitOptions submitOptions = new ReactorSubmitOptions();
        submitOptions.Clear();

        TestReactor consumerReactor = new TestReactor();
        TestReactor providerReactor = new TestReactor();
        LoginRefresh loginInfo = new();

        Consumer consumer = new Consumer(consumerReactor);
        Provider provider = new Provider(providerReactor);

        // Preform basic setup, including directory and dictionary downloads
        BasicDictionaryDirectoryRequestDo(submitOptions, consumerReactor, providerReactor, consumer, provider, out var service);

        // Send simple Market Price Request
        MarketPriceRequest marketPriceRequest = new();
        GenerateMarketPriceRequest(marketPriceRequest, true, service, loginInfo, viewFieldList: DEFAULT_VIEW_FIELD_LIST);

        // requesting information for more than one item makes it into a batch request
        marketPriceRequest.ItemNames.Add("ITEM1");
        marketPriceRequest.ItemNames.Add("ITEM2");

        Assert.True(consumer.SubmitAndDispatch(marketPriceRequest, submitOptions) >= ReactorReturnCode.SUCCESS);

        // Provider has to handle it and send Market Price Refresh in response, but first check the message
        providerReactor.Dispatch(1);
        TestReactorEvent evt = providerReactor.PollEvent();
        Assert.Equal(TestReactorEventType.MSG, evt.EventType);
        ReactorMsgEvent msgEvent = (ReactorMsgEvent)evt.ReactorEvent;
        Msg msg = msgEvent.Msg as Msg;
        Assert.NotNull(msg);
        Assert.Equal(MsgClasses.REQUEST, msg.MsgClass);
        Assert.Equal((int)DomainType.MARKET_PRICE, msg.DomainType);
        Assert.NotNull(msg.MsgKey);
        Assert.False(msg.MsgKey.CheckHasName());
        Assert.True(msg.CheckHasBatch());

        // Now Provider proceeds to submit refresh messages for requested items

        ReactorChannel reactorChannel = msgEvent.ReactorChannel;
        Assert.NotNull(reactorChannel);

        // this will decode the request message similar how VAProvider handles
        // a batch request, will check that the items are there
        var items = DecodeBatchRequest(reactorChannel, msg, false);
        Assert.Equal(2, items.Count);
        Assert.NotNull(items.Find(item => item.Equals("ITEM1")));
        Assert.NotNull(items.Find(item => item.Equals("ITEM2")));

        IStatusMsg statusMsg = new Msg();
        statusMsg.MsgClass = MsgClasses.STATUS;
        // Provider closes the original request stream, item refresh responses
        // are sent on their individual streams
        statusMsg.StreamId = msg.StreamId;
        statusMsg.DomainType = msg.DomainType;
        statusMsg.ContainerType = Codec.DataTypes.NO_DATA;
        statusMsg.Flags = StatusMsgFlags.HAS_STATE;
        statusMsg.State.StreamState(StreamStates.CLOSED);
        statusMsg.State.DataState(DataStates.NO_CHANGE);
        statusMsg.State.Code(StateCodes.NONE);
        statusMsg.State.Text().Data("Stream closed for batch");

        Assert.True(provider.SubmitAndDispatch((Msg)statusMsg, submitOptions) >= ReactorReturnCode.SUCCESS);

        // now consumer has the market price refresh message and has to handle it
        consumerReactor.Dispatch(1);
        evt = consumerReactor.PollEvent();
        Assert.NotNull(evt);
        msgEvent = (ReactorMsgEvent)evt.ReactorEvent;
        Assert.NotNull(msgEvent.Msg);
        Assert.Equal((int)DomainType.MARKET_PRICE, msgEvent.Msg.DomainType);
        Assert.Equal(MsgClasses.STATUS, msgEvent.Msg.MsgClass);
        Assert.Equal(marketPriceRequest.StreamId, msgEvent.Msg.StreamId);

        // Test is over, clean up
        TestReactorComponent.CloseSession(consumer, provider);
        TearDownConsumerAndProvider(consumerReactor, providerReactor);
    }

    /// <summary>
    /// Extracts requested items list from a batch item request.
    /// </summary>
    private List<string> DecodeBatchRequest(ReactorChannel chnl, Msg msg, bool isPrivateStream)
    {
        output.WriteLine($"Received batch item request (streamId={msg.StreamId}) on domain {msg.DomainType}");

        DecodeIterator dIter = new();
        dIter.Clear();
        if (msg.EncodedDataBody.Data() != null)
        {
            dIter.SetBufferAndRWFVersion(msg.EncodedDataBody, chnl.MajorVersion, chnl.MinorVersion);
        }

        ElementList elementList = new ElementList();
        ElementEntry elementEntry = new ElementEntry();
        Codec.Array array = new();
        ArrayEntry arrayEntry = new();
        Codec.Buffer batchReqName = new Codec.Buffer();
        batchReqName.Data(":ItemList");

        // The payload of a batch request contains an elementList
        CodecReturnCode ret = elementList.Decode(dIter, null);
        Assert.False(ret < CodecReturnCode.SUCCESS);

        List<string> items = new();
        // The list of items being requested is in an elementList entry with the
        // element name of ":ItemList"
        while ((ret = elementEntry.Decode(dIter)) != CodecReturnCode.END_OF_CONTAINER)
        {
            if (elementEntry.Name.Equals(batchReqName))
            {
                // The list of items names is in an array
                ret = array.Decode(dIter);
                Assert.False(ret < CodecReturnCode.SUCCESS);

                while ((ret = arrayEntry.Decode(dIter)) != CodecReturnCode.END_OF_CONTAINER)
                {
                    Assert.False(ret < CodecReturnCode.SUCCESS);

                    //all of the items requested have the same key. They use
                    //the key of the batch request.
                    //The only difference is the name
                    msg.MsgKey.Flags = msg.MsgKey.Flags | MsgKeyFlags.HAS_NAME;
                    msg.MsgKey.Name = arrayEntry.EncodedData;

                    // this is the place where a Provider would handle requested item
                    // i.e. send a market price refresh in response
                    items.Add(msg.MsgKey.Name.ToString());
                }
            }
        }
        return items;
    }

    #endregion

    #region Basic batch request test

    [Fact]
    public void BatchRequestTest_Socket()
    {
        // Test a simple batch request/refresh exchange without Watchlist service.
        ReactorSubmitOptions submitOptions = new();
        TestReactorEvent evt;
        ReactorMsgEvent msgEvent;
        Msg msg = new();
        IRequestMsg requestMsg = (IRequestMsg)msg;
        IRequestMsg receivedRequestMsg;
        IRefreshMsg refreshMsg = (IRefreshMsg)msg;
        IRefreshMsg receivedRefreshMsg;
        int providerStreamId;

        // Create reactors.
        TestReactor consumerReactor = new TestReactor();
        TestReactor providerReactor = new TestReactor();

        // Create consumer.
        Consumer consumer = new Consumer(consumerReactor);
        ConsumerRole consumerRole = (ConsumerRole)consumer.ReactorRole;
        consumerRole.InitDefaultRDMLoginRequest();
        consumerRole.InitDefaultRDMDirectoryRequest();
        consumerRole.ChannelEventCallback = consumer;
        consumerRole.LoginMsgCallback = consumer;
        consumerRole.DirectoryMsgCallback = consumer;
        consumerRole.DictionaryMsgCallback = consumer;
        consumerRole.DefaultMsgCallback = consumer;

        // Create provider.
        Provider provider = new Provider(providerReactor);
        ProviderRole providerRole = (ProviderRole)provider.ReactorRole;
        providerRole.ChannelEventCallback = provider;
        providerRole.LoginMsgCallback = provider;
        providerRole.DirectoryMsgCallback = provider;
        providerRole.DictionaryMsgCallback = provider;
        providerRole.DefaultMsgCallback = provider;

        // Connect the consumer and provider. Setup login & directory streams automatically.
        ConsumerProviderSessionOptions opts = new();
        opts.SetupDefaultLoginStream = true;
        opts.SetupDefaultDirectoryStream = true;

        provider.Bind(opts);

        TestReactor.OpenSession(consumer, provider, opts);

        // Consumer sends request.
        requestMsg.Clear();
        requestMsg.MsgClass = MsgClasses.REQUEST;
        requestMsg.StreamId = 5;
        requestMsg.DomainType = (int)DomainType.MARKET_PRICE;
        msg.ContainerType = Codec.DataTypes.ELEMENT_LIST;
        requestMsg.ApplyStreaming();
        requestMsg.ApplyHasBatch();

        EncodeBatchWithView(consumer.ReactorChannel, requestMsg, null);

        submitOptions.Clear();
        submitOptions.ServiceName = Provider.DefaultService.Info.ServiceName.ToString();
        Assert.True(consumer.Submit((Msg)requestMsg, submitOptions) >= ReactorReturnCode.SUCCESS);

        // note: When Watchlist is enabled, status message with closed batch request can
        // be received immediately and dispatched here

        // Provider receives request.
        providerReactor.Dispatch(1);
        evt = providerReactor.PollEvent();
        Assert.Equal(TestReactorEventType.MSG, evt.EventType);
        msgEvent = (ReactorMsgEvent)evt.ReactorEvent;
        Assert.Equal(MsgClasses.REQUEST, msgEvent.Msg.MsgClass);

        receivedRequestMsg = (IRequestMsg)msgEvent.Msg;
        Assert.True(receivedRequestMsg.CheckStreaming());
        Assert.False(receivedRequestMsg.CheckNoRefresh());
        // requested items in batch requested are decoded
        Assert.False(receivedRequestMsg.MsgKey.CheckHasName());

        ReactorChannel reactorChannel = msgEvent.ReactorChannel;
        Assert.NotNull(reactorChannel);

        var items = DecodeBatchRequest(reactorChannel, msg, false);

        Assert.Equal(2, items.Count);

        // note: Watchlist splits batch request into individual requests.
        // Without Watchlist we have to deal with a single request

        Assert.Equal((int)DomainType.MARKET_PRICE, receivedRequestMsg.DomainType);
        Assert.Equal(Codec.DataTypes.ELEMENT_LIST, receivedRequestMsg.ContainerType);

        // Before sending data on requested items Provider responds on the same stream as
        // the original batch request.  The stream on which the batch request was sent
        // (i.e., the 'batch stream') then closes, because all additional responses are
        // provided on individual streams.
        IStatusMsg statusMsg = new Msg();
        statusMsg.MsgClass = MsgClasses.STATUS;
        statusMsg.StreamId = receivedRequestMsg.StreamId;
        statusMsg.DomainType = msg.DomainType;
        statusMsg.ContainerType = Codec.DataTypes.NO_DATA;
        statusMsg.Flags = StatusMsgFlags.HAS_STATE;
        statusMsg.State.StreamState(StreamStates.CLOSED);
        statusMsg.State.DataState(DataStates.OK);

        Assert.True(provider.Submit((Msg)statusMsg, submitOptions) >= ReactorReturnCode.SUCCESS);

        // Consumer recieves the close the 'batch stream' message
        consumerReactor.Dispatch(1);
        evt = consumerReactor.PollEvent();
        Assert.Equal(TestReactorEventType.MSG, evt.EventType);
        msgEvent = (ReactorMsgEvent)evt.ReactorEvent;
        Assert.Equal(MsgClasses.STATUS, msgEvent.Msg.MsgClass);
        Assert.Equal(5, msgEvent.Msg.StreamId);
        Assert.NotEqual(0, msgEvent.Msg.Flags & StatusMsgFlags.HAS_STATE);

        // Now Provider proceeds to submit refresh messages for requested items

        // Assign consecutive stream IDs for each item starting with the stream following
        // the one the batch request was made on
        providerStreamId = receivedRequestMsg.StreamId++;

        // Provider sends refresh for the first item
        refreshMsg.Clear();
        refreshMsg.MsgClass = MsgClasses.REFRESH;
        refreshMsg.DomainType = (int)DomainType.MARKET_PRICE;
        refreshMsg.StreamId = providerStreamId;
        refreshMsg.ContainerType = Codec.DataTypes.NO_DATA;
        // indicate that it is a single-part (atomic) refresh
        refreshMsg.ApplyRefreshComplete();
        refreshMsg.ApplyHasMsgKey();
        refreshMsg.MsgKey.ApplyHasServiceId();
        refreshMsg.MsgKey.ServiceId = Provider.DefaultService.ServiceId;
        refreshMsg.MsgKey.ApplyHasName();
        refreshMsg.MsgKey.Name.Data("TRI.N");
        refreshMsg.State.StreamState(StreamStates.OPEN);
        refreshMsg.State.DataState(DataStates.OK);

        Assert.True(provider.SubmitAndDispatch((Msg)refreshMsg, submitOptions) >= ReactorReturnCode.SUCCESS);

        // Consumer receives refresh for the first item
        consumerReactor.Dispatch(1);

        // Received Refresh for TRI.N
        evt = consumerReactor.PollEvent();
        Assert.Equal(TestReactorEventType.MSG, evt.EventType);
        msgEvent = (ReactorMsgEvent)evt.ReactorEvent;
        Assert.Equal(MsgClasses.REFRESH, msgEvent.Msg.MsgClass);

        // Received refresh message with item TRI.N
        receivedRefreshMsg = (IRefreshMsg)msgEvent.Msg;
        Assert.True(receivedRefreshMsg.CheckRefreshComplete());
        Assert.Equal("TRI.N", receivedRefreshMsg.MsgKey.Name.ToString());
        Assert.Equal((int)DomainType.MARKET_PRICE, receivedRefreshMsg.DomainType);
        Assert.Equal(Codec.DataTypes.NO_DATA, receivedRefreshMsg.ContainerType);
        Assert.Equal(StreamStates.OPEN, receivedRefreshMsg.State.StreamState());
        Assert.Equal(DataStates.OK, receivedRefreshMsg.State.DataState());
        Assert.False(receivedRefreshMsg.CheckSolicited());

        // Provider processes request for IBM.N - since we don't have Watchlist,
        // the request message remains the same
        Assert.Equal(TestReactorEventType.MSG, evt.EventType);
        msgEvent = (ReactorMsgEvent)evt.ReactorEvent;
        Assert.Equal(MsgClasses.REFRESH, msgEvent.Msg.MsgClass);

        receivedRequestMsg = (IRequestMsg)msgEvent.Msg;
        Assert.False(receivedRequestMsg.CheckNoRefresh());
        Assert.Equal((int)DomainType.MARKET_PRICE, receivedRequestMsg.DomainType);
        Assert.Equal(Codec.DataTypes.NO_DATA, receivedRequestMsg.ContainerType);

        providerStreamId = receivedRequestMsg.StreamId++;

        // Provider sends refresh for the second item
        refreshMsg.Clear();
        refreshMsg.MsgClass = MsgClasses.REFRESH;
        refreshMsg.DomainType = (int)DomainType.MARKET_PRICE;
        refreshMsg.StreamId = providerStreamId;
        refreshMsg.ContainerType = Codec.DataTypes.NO_DATA;
        // indicate that it is a single-part (atomic) refresh
        refreshMsg.ApplyRefreshComplete();
        refreshMsg.ApplyHasMsgKey();
        refreshMsg.MsgKey.ApplyHasServiceId();
        refreshMsg.MsgKey.ServiceId = Provider.DefaultService.ServiceId;
        refreshMsg.MsgKey.ApplyHasName();
        refreshMsg.MsgKey.Name.Data("IBM.N");
        refreshMsg.State.StreamState(StreamStates.OPEN);
        refreshMsg.State.DataState(DataStates.OK);

        Assert.True(provider.SubmitAndDispatch((Msg)refreshMsg, submitOptions) >= ReactorReturnCode.SUCCESS);

        // Consumer receives refresh for IBM.N
        consumerReactor.Dispatch(1);
        evt = consumerReactor.PollEvent();
        Assert.Equal(TestReactorEventType.MSG, evt.EventType);
        msgEvent = (ReactorMsgEvent)evt.ReactorEvent;
        Assert.Equal(MsgClasses.REFRESH, msgEvent.Msg.MsgClass);

        // Received refresh message with item IBM.N
        receivedRefreshMsg = (IRefreshMsg)msgEvent.Msg;
        Assert.True(receivedRefreshMsg.CheckRefreshComplete());
        Assert.Equal("IBM.N", receivedRefreshMsg.MsgKey.Name.ToString());
        Assert.Equal((int)DomainType.MARKET_PRICE, receivedRefreshMsg.DomainType);
        Assert.Equal(Codec.DataTypes.NO_DATA, receivedRefreshMsg.ContainerType);
        Assert.Equal(StreamStates.OPEN, receivedRefreshMsg.State.StreamState());
        Assert.Equal(DataStates.OK, receivedRefreshMsg.State.DataState());
        // consumer did not explicitly ask for this refresh
        Assert.False(receivedRefreshMsg.CheckSolicited());

        TestReactorComponent.CloseSession(consumer, provider);
        TearDownConsumerAndProvider(consumerReactor, providerReactor);
    }

    #endregion

    #region  Request Symbol List test
    [Fact]
    public void RequestSymbolListTest_Socket()
    {
        // Test a batch request/refresh exchange that has a symbolList without Watchlist service.
        ReactorSubmitOptions submitOptions = new();
        TestReactorEvent evt;
        ReactorMsgEvent msgEvent;
        Msg msg = new();
        IRequestMsg requestMsg = (IRequestMsg)msg;
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

        // Consumer sends request.
        requestMsg.Clear();
        requestMsg.MsgClass = MsgClasses.REQUEST;
        requestMsg.StreamId = 5;
        requestMsg.DomainType = (int)DomainType.SYMBOL_LIST;
        requestMsg.ContainerType = Codec.DataTypes.ELEMENT_LIST;
        requestMsg.ApplyHasQos();
        requestMsg.Qos.Rate(QosRates.TICK_BY_TICK);
        requestMsg.Qos.Timeliness(QosTimeliness.REALTIME);
        requestMsg.ApplyHasPriority();
        requestMsg.Priority.PriorityClass = 1;
        requestMsg.Priority.Count = 1;
        requestMsg.ApplyStreaming();
        requestMsg.ApplyHasBatch();

        EncodeSymbolList(consumer.ReactorChannel, requestMsg, true);

        submitOptions.Clear();
        submitOptions.ServiceName = Provider.DefaultService.Info.ServiceName.ToString();
        Assert.True(consumer.Submit((Msg)requestMsg, submitOptions) >= ReactorReturnCode.SUCCESS);

        // note: When Watchlist is enabled, status message with closed batch request can
        // be received immediately and dispatched here

        // Provider receives request. Since we don't have a Watchlist, the request is received as is in a single event
        providerReactor.Dispatch(1);
        evt = providerReactor.PollEvent();
        Assert.Equal(TestReactorEventType.MSG, evt.EventType);
        msgEvent = (ReactorMsgEvent)evt.ReactorEvent;
        Assert.Equal(MsgClasses.REQUEST, msgEvent.Msg.MsgClass);

        receivedRequestMsg = (IRequestMsg)msgEvent.Msg;
        Assert.True(receivedRequestMsg.CheckStreaming());
        Assert.False(receivedRequestMsg.CheckNoRefresh());
        Assert.False(receivedRequestMsg.MsgKey.CheckHasName());
        Assert.Equal((int)DomainType.SYMBOL_LIST, receivedRequestMsg.DomainType);

        // Get each requested item name We will assign consecutive stream IDs for each
        // item starting with the stream following the one the batch request was made on
        providerStreamId = receivedRequestMsg.StreamId++;

        // Provider sends refresh
        refreshMsg.Clear();
        refreshMsg.MsgClass = MsgClasses.REFRESH;
        refreshMsg.DomainType = (int)DomainType.SYMBOL_LIST;
        refreshMsg.StreamId = providerStreamId;
        refreshMsg.ContainerType = Codec.DataTypes.NO_DATA;
        // indicate that it is a single-part (atomic) refresh
        refreshMsg.ApplyRefreshComplete();
        refreshMsg.ApplyHasMsgKey();
        refreshMsg.MsgKey.ApplyHasServiceId();
        refreshMsg.MsgKey.ServiceId = Provider.DefaultService.ServiceId;
        refreshMsg.MsgKey.ApplyHasName();
        refreshMsg.MsgKey.Name.Data("TRI.N");
        refreshMsg.State.StreamState(StreamStates.OPEN);
        refreshMsg.State.DataState(DataStates.OK);

        Assert.True(provider.Submit((Msg)refreshMsg, submitOptions) >= ReactorReturnCode.SUCCESS);

        // Consumer receives refresh.
        consumerReactor.Dispatch(1);

        // Received Refresh for TRI.N
        evt = consumerReactor.PollEvent();
        Assert.Equal(TestReactorEventType.MSG, evt.EventType);
        msgEvent = (ReactorMsgEvent)evt.ReactorEvent;
        Assert.Equal(MsgClasses.REFRESH, msgEvent.Msg.MsgClass);

        // Received refresh message for TRI.N
        receivedRefreshMsg = (IRefreshMsg)msgEvent.Msg;
        Assert.True(receivedRefreshMsg.CheckRefreshComplete());
        Assert.Equal("TRI.N", receivedRefreshMsg.MsgKey.Name.ToString());
        Assert.Equal((int)DomainType.SYMBOL_LIST, receivedRefreshMsg.DomainType);
        Assert.Equal(Codec.DataTypes.NO_DATA, receivedRefreshMsg.ContainerType);
        Assert.Equal(StreamStates.OPEN, receivedRefreshMsg.State.StreamState());
        Assert.Equal(DataStates.OK, receivedRefreshMsg.State.DataState());
        Assert.False(receivedRefreshMsg.CheckSolicited());

        // Provider processes request for IBM.N - since we don't have Watchlist,
        // the request message remains the same
        receivedRequestMsg = (IRequestMsg)msgEvent.Msg;
        Assert.False(receivedRequestMsg.CheckNoRefresh());
        Assert.Equal((int)DomainType.SYMBOL_LIST, receivedRequestMsg.DomainType);

        providerStreamId = receivedRequestMsg.StreamId++;

        // Provider sends refresh
        refreshMsg.Clear();
        refreshMsg.MsgClass = MsgClasses.REFRESH;
        refreshMsg.DomainType = (int)DomainType.SYMBOL_LIST;
        refreshMsg.StreamId = providerStreamId;
        refreshMsg.ContainerType = Codec.DataTypes.NO_DATA;
        // indicate that it is a single-part (atomic) refresh
        refreshMsg.ApplyRefreshComplete();
        refreshMsg.ApplyHasMsgKey();
        refreshMsg.MsgKey.ApplyHasServiceId();
        refreshMsg.MsgKey.ServiceId = Provider.DefaultService.ServiceId;
        refreshMsg.MsgKey.ApplyHasName();
        refreshMsg.MsgKey.Name.Data("IBM.N");
        refreshMsg.State.StreamState(StreamStates.OPEN);
        refreshMsg.State.DataState(DataStates.OK);

        Assert.True(provider.SubmitAndDispatch((Msg)refreshMsg, submitOptions) >= ReactorReturnCode.SUCCESS);

        // Consumer receives refresh for IBM.N
        consumerReactor.Dispatch(1);
        evt = consumerReactor.PollEvent();
        Assert.Equal(TestReactorEventType.MSG, evt.EventType);
        msgEvent = (ReactorMsgEvent)evt.ReactorEvent;
        Assert.Equal(MsgClasses.REFRESH, msgEvent.Msg.MsgClass);

        // Received refresh message for IBM.N
        receivedRefreshMsg = (IRefreshMsg)msgEvent.Msg;
        Assert.Equal("IBM.N", receivedRefreshMsg.MsgKey.Name.ToString());
        Assert.Equal((int)DomainType.SYMBOL_LIST, receivedRefreshMsg.DomainType);
        Assert.Equal(Codec.DataTypes.NO_DATA, receivedRefreshMsg.ContainerType);
        Assert.Equal(StreamStates.OPEN, receivedRefreshMsg.State.StreamState());
        Assert.Equal(DataStates.OK, receivedRefreshMsg.State.DataState());
        Assert.True(receivedRefreshMsg.CheckRefreshComplete());
        Assert.False(receivedRefreshMsg.CheckSolicited());

        TestReactorComponent.CloseSession(consumer, provider);
        TearDownConsumerAndProvider(consumerReactor, providerReactor);
    }

    #endregion

    #region Posting, ack/nack from provider

    /// <summary> Helper class used by SubmitPostOnItemRefreshTest.</summary>
    class PostFromDefaultMsgCallbackConsumer : Consumer
    {
        public PostFromDefaultMsgCallbackConsumer(TestReactor testReactor) : base(testReactor)
        {
        }

        public override ReactorCallbackReturnCode DefaultMsgCallback(ReactorMsgEvent evt)
        {
            base.DefaultMsgCallback(evt);

            IMsg msg = evt.Msg;

            switch (msg.MsgClass)
            {
                case MsgClasses.REFRESH:
                    // send post message
                    IPostMsg postMsg = (IPostMsg)new Msg();
                    postMsg.Clear();
                    postMsg.MsgClass = MsgClasses.POST;
                    postMsg.StreamId = msg.StreamId;
                    postMsg.DomainType = msg.DomainType;
                    postMsg.ContainerType = Codec.DataTypes.NO_DATA;

                    ReactorSubmitOptions submitOptions = new();
                    Assert.Equal(ReactorReturnCode.SUCCESS, evt.ReactorChannel.Submit((Msg)postMsg, submitOptions, out _));

                    break;
                default:
                    break;
            }

            return ReactorCallbackReturnCode.SUCCESS;
        }
    }

    [Fact]
    public void SubmitPostOnItemRefreshTest_Socket()
    {
        ReactorSubmitOptions submitOptions = new();
        TestReactorEvent evt;
        ReactorMsgEvent msgEvent;
        Msg msg = new();
        IRequestMsg requestMsg = (IRequestMsg)msg;
        IRequestMsg receivedRequestMsg;
        IRefreshMsg refreshMsg = (IRefreshMsg)msg;
        IRefreshMsg receivedRefreshMsg;

        /* Create reactors. */
        TestReactor consumerReactor = new TestReactor();
        TestReactor providerReactor = new TestReactor();

        /* Create consumer. */
        Consumer consumer = new PostFromDefaultMsgCallbackConsumer(consumerReactor);
        ConsumerRole consumerRole = (ConsumerRole)consumer.ReactorRole;
        consumerRole.InitDefaultRDMLoginRequest();
        consumerRole.InitDefaultRDMDirectoryRequest();
        consumerRole.ChannelEventCallback = consumer;
        consumerRole.LoginMsgCallback = consumer;
        consumerRole.DirectoryMsgCallback = consumer;
        consumerRole.DictionaryMsgCallback = consumer;
        consumerRole.DefaultMsgCallback = consumer;

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
        evt = providerReactor.PollEvent();
        Assert.Equal(TestReactorEventType.MSG, evt.EventType);
        msgEvent = (ReactorMsgEvent)evt.ReactorEvent;
        Assert.Equal(MsgClasses.REQUEST, msgEvent.Msg.MsgClass);

        receivedRequestMsg = (IRequestMsg)msgEvent.Msg;
        Assert.True(receivedRequestMsg.CheckStreaming());
        Assert.False(receivedRequestMsg.CheckNoRefresh());
        Assert.True(receivedRequestMsg.MsgKey.CheckHasName());
        Assert.Equal("TRI.N", receivedRequestMsg.MsgKey.Name.ToString());
        Assert.Equal((int)DomainType.MARKET_PRICE, receivedRequestMsg.DomainType);

        // Provider sends refresh
        refreshMsg.Clear();
        refreshMsg.MsgClass = MsgClasses.REFRESH;
        refreshMsg.DomainType = (int)DomainType.MARKET_PRICE;
        refreshMsg.StreamId = receivedRequestMsg.StreamId;
        refreshMsg.ContainerType = Codec.DataTypes.NO_DATA;
        // indicate that it is a single-part (atomic) refresh
        refreshMsg.ApplyRefreshComplete();
        refreshMsg.ApplyHasMsgKey();
        refreshMsg.MsgKey.ApplyHasServiceId();
        refreshMsg.MsgKey.ServiceId = Provider.DefaultService.ServiceId;
        refreshMsg.MsgKey.ApplyHasName();
        refreshMsg.MsgKey.Name.Data("TRI.N");
        Codec.Buffer groupId = new();
        groupId.Data("1234431");
        refreshMsg.GroupId = groupId;
        refreshMsg.State.StreamState(StreamStates.OPEN);
        refreshMsg.State.DataState(DataStates.OK);

        Assert.True(provider.SubmitAndDispatch((Msg)refreshMsg, submitOptions) >= ReactorReturnCode.SUCCESS);

        // Consumer receives refresh.
        consumerReactor.Dispatch(1);
        evt = consumerReactor.PollEvent();
        Assert.Equal(TestReactorEventType.MSG, evt.EventType);
        msgEvent = (ReactorMsgEvent)evt.ReactorEvent;
        Assert.Equal(MsgClasses.REFRESH, msgEvent.Msg.MsgClass);

        receivedRefreshMsg = (IRefreshMsg)msgEvent.Msg;
        Assert.True(receivedRefreshMsg.CheckHasMsgKey());
        Assert.True(receivedRefreshMsg.MsgKey.CheckHasServiceId());
        Assert.True(receivedRefreshMsg.CheckRefreshComplete());
        Assert.Equal(Provider.DefaultService.ServiceId, receivedRefreshMsg.MsgKey.ServiceId);
        Assert.True(receivedRefreshMsg.MsgKey.CheckHasName());
        Assert.Equal("TRI.N", receivedRefreshMsg.MsgKey.Name.ToString());
        Assert.Equal((int)DomainType.MARKET_PRICE, receivedRefreshMsg.DomainType);
        Assert.Equal(Codec.DataTypes.NO_DATA, receivedRefreshMsg.ContainerType);
        Assert.Equal(StreamStates.OPEN, receivedRefreshMsg.State.StreamState());
        Assert.Equal(DataStates.OK, receivedRefreshMsg.State.DataState());

        // Provider receives post.
        providerReactor.Dispatch(1);
        evt = providerReactor.PollEvent();
        msgEvent = (ReactorMsgEvent)evt.ReactorEvent;
        Assert.Equal(MsgClasses.POST, msgEvent.Msg.MsgClass);
        IPostMsg receivedPostMsg = (IPostMsg)msgEvent.Msg;
        Assert.Equal(requestMsg.StreamId, receivedPostMsg.StreamId);
        Assert.Equal((int)DomainType.MARKET_PRICE, receivedPostMsg.DomainType);
        Assert.Equal(Codec.DataTypes.NO_DATA, receivedPostMsg.ContainerType);

        TestReactorComponent.CloseSession(consumer, provider);
        TearDownConsumerAndProvider(consumerReactor, providerReactor);
    }

    /// <summary> Helper class used by <c cref="SubmitOffstreamPostOnItemRefreshTest_Socket()"/>.</summary>
    class OffPostFromDefaultMsgCallbackConsumer : Consumer
    {
        public OffPostFromDefaultMsgCallbackConsumer(TestReactor testReactor) : base(testReactor)
        {
        }


        public override ReactorCallbackReturnCode DefaultMsgCallback(ReactorMsgEvent evt)
        {
            base.DefaultMsgCallback(evt);

            IMsg msg = evt.Msg;

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
                        postMsg.ContainerType = Codec.DataTypes.MSG;
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

                        msg.Flags = msg.Flags & ~RefreshMsgFlags.SOLICITED;

                        EncodeIterator eIter = new();
                        Codec.Buffer buffer = new();
                        buffer.Data(new Common.ByteBuffer(1024));
                        eIter.SetBufferAndRWFVersion(buffer, evt.ReactorChannel.MajorVersion, evt.ReactorChannel.MinorVersion);
                        // reset all flags and set stream id to 0 to simulate bad refresh message
                        msg.Flags = 0;
                        msg.StreamId = 0;
                        msg.Encode(eIter);
                        postMsg.EncodedDataBody = buffer;

                        ReactorSubmitOptions submitOptions = new();
                        ReactorErrorInfo errorInfo = new();
                        Assert.Equal(ReactorReturnCode.SUCCESS, evt.ReactorChannel.Submit((Msg)postMsg, submitOptions, out _));

                    }
                    break;
                default:
                    break;
            }

            return ReactorCallbackReturnCode.SUCCESS;
        }
    }

    [Fact]
    public void SubmitOffstreamPostOnItemRefreshTest_Socket()
    {
        ReactorSubmitOptions submitOptions = new();
        TestReactorEvent evt;
        ReactorMsgEvent msgEvent;
        Msg msg = new();
        IRequestMsg requestMsg = (IRequestMsg)msg;
        IRequestMsg receivedRequestMsg;
        IRefreshMsg refreshMsg = (IRefreshMsg)msg;
        IRefreshMsg receivedRefreshMsg;

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
        evt = providerReactor.PollEvent();
        Assert.Equal(TestReactorEventType.MSG, evt.EventType);
        msgEvent = (ReactorMsgEvent)evt.ReactorEvent;
        Assert.Equal(MsgClasses.REQUEST, msgEvent.Msg.MsgClass);

        receivedRequestMsg = (IRequestMsg)msgEvent.Msg;
        Assert.True(receivedRequestMsg.CheckStreaming());
        Assert.False(receivedRequestMsg.CheckNoRefresh());
        Assert.True(receivedRequestMsg.MsgKey.CheckHasName());
        Assert.Equal("TRI.N", receivedRequestMsg.MsgKey.Name.ToString());
        Assert.Equal((int)DomainType.MARKET_PRICE, receivedRequestMsg.DomainType);

        /* Provider sends refresh .*/
        refreshMsg.Clear();
        refreshMsg.MsgClass = MsgClasses.REFRESH;
        refreshMsg.ApplySolicited();
        refreshMsg.DomainType = (int)DomainType.MARKET_PRICE;
        refreshMsg.StreamId = receivedRequestMsg.StreamId;
        refreshMsg.ContainerType = Codec.DataTypes.NO_DATA;
        // indicate that it is a single-part (atomic) refresh
        refreshMsg.ApplyRefreshComplete();
        refreshMsg.ApplyHasMsgKey();
        refreshMsg.MsgKey.ApplyHasServiceId();
        refreshMsg.MsgKey.ServiceId = Provider.DefaultService.ServiceId;
        refreshMsg.MsgKey.ApplyHasName();
        refreshMsg.MsgKey.Name.Data("TRI.N");
        Codec.Buffer groupId = new();
        groupId.Data("1234431");
        refreshMsg.GroupId = groupId;
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
        Assert.True(receivedRefreshMsg.CheckRefreshComplete());
        Assert.True(receivedRefreshMsg.CheckHasMsgKey());
        Assert.True(receivedRefreshMsg.MsgKey.CheckHasName());
        Assert.Equal("TRI.N", receivedRefreshMsg.MsgKey.Name.ToString());
        Assert.Equal((int)DomainType.MARKET_PRICE, receivedRefreshMsg.DomainType);
        Assert.Equal(Codec.DataTypes.NO_DATA, receivedRefreshMsg.ContainerType);
        Assert.Equal(StreamStates.OPEN, receivedRefreshMsg.State.StreamState());
        Assert.Equal(DataStates.OK, receivedRefreshMsg.State.DataState());

        /* Provider receives offstream post. */
        IPostMsg offstreamPost;
        providerReactor.Dispatch(1);
        evt = providerReactor.PollEvent();
        Assert.Equal(TestReactorEventType.MSG, evt.EventType);
        msgEvent = (ReactorMsgEvent)evt.ReactorEvent;
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
        evt = consumerReactor.PollEvent();
        Assert.Equal(TestReactorEventType.MSG, evt.EventType);
        msgEvent = (ReactorMsgEvent)evt.ReactorEvent;
        Assert.Equal(MsgClasses.ACK, msgEvent.Msg.MsgClass);

        // extract refresh message from post and send to consumer
        DecodeIterator dIter = new();
        dIter.SetBufferAndRWFVersion(offstreamPost.EncodedDataBody,
            msgEvent.ReactorChannel.MajorVersion, msgEvent.ReactorChannel.MinorVersion);
        Msg extractedMsg = new Msg();
        Assert.Equal(CodecReturnCode.SUCCESS, extractedMsg.Decode(dIter));

        Assert.Equal(0, extractedMsg.Flags);
        extractedMsg.StreamId = 3;
        Assert.True(provider.SubmitAndDispatch((Msg)extractedMsg, submitOptions) >= ReactorReturnCode.SUCCESS);

        /* Consumer receives extracted refresh message. */
        consumerReactor.Dispatch(1);
        evt = consumerReactor.PollEvent();
        Assert.Equal(TestReactorEventType.MSG, evt.EventType);
        msgEvent = (ReactorMsgEvent)evt.ReactorEvent;
        Assert.Equal(MsgClasses.REFRESH, msgEvent.Msg.MsgClass);
        Assert.Equal(0, msgEvent.Msg.Flags);

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


    #endregion

    #region Private and helper methods

    private void EncodeSymbolList(ReactorChannel rc, IRequestMsg msg, bool hasBehaviors)
    {
        Codec.Buffer buf = new();
        buf.Data(new Common.ByteBuffer(1024));
        EncodeIterator encodeIter = new();

        /* clear encode iterator */
        encodeIter.Clear();

        /* encode message */
        encodeIter.SetBufferAndRWFVersion(buf, rc.MajorVersion, rc.MinorVersion);

        // encode payload

        ElementList eList = new();
        ElementEntry eEntry = new();

        eList.ApplyHasStandardData();
        Assert.Equal(CodecReturnCode.SUCCESS, eList.EncodeInit(encodeIter, null, 0));

        if (hasBehaviors)
        {
            ElementList behaviorList = new();
            ElementEntry dataStreamEntry = new();
            UInt tempUInt = new();

            eEntry.Clear();
            eEntry.Name.Data(":SymbolListBehaviors");
            eEntry.DataType = Codec.DataTypes.ELEMENT_LIST;

            Assert.Equal(CodecReturnCode.SUCCESS, eEntry.EncodeInit(encodeIter, 0));

            behaviorList.Clear();
            behaviorList.ApplyHasStandardData();

            Assert.Equal(CodecReturnCode.SUCCESS, behaviorList.EncodeInit(encodeIter, null, 0));

            dataStreamEntry.Clear();
            dataStreamEntry.Name.Data(":DataStreams");
            dataStreamEntry.DataType = Codec.DataTypes.UINT;
            tempUInt.Value(SymbolList.SymbolListDataStreamRequestFlags.SYMBOL_LIST_DATA_STREAMS);
            Assert.Equal(CodecReturnCode.SUCCESS, dataStreamEntry.Encode(encodeIter, tempUInt));

            Assert.Equal(CodecReturnCode.SUCCESS, behaviorList.EncodeComplete(encodeIter, true));

            Assert.Equal(CodecReturnCode.SUCCESS, eEntry.EncodeComplete(encodeIter, true));
        }

        Assert.Equal(CodecReturnCode.SUCCESS, eList.EncodeComplete(encodeIter, true));

        msg.EncodedDataBody = buf;
    }

    private void EncodeBatchWithView(ReactorChannel rc, IRequestMsg msg, List<int> fieldIdList)
    {
        Codec.Buffer buf = new();
        buf.Data(new Common.ByteBuffer(1024));
        EncodeIterator encodeIter = new();

        /* clear encode iterator */
        encodeIter.Clear();

        /* encode message */
        encodeIter.SetBufferAndRWFVersion(buf, rc.MajorVersion, rc.MinorVersion);

        // encode payload

        ElementList eList = new();
        ElementEntry eEntry = new();
        Codec.Array elementArray = new();
        ArrayEntry ae = new();
        Codec.Buffer itemName = new();

        eList.ApplyHasStandardData();
        Assert.Equal(CodecReturnCode.SUCCESS, eList.EncodeInit(encodeIter, null, 0));

        // encode Batch
        eEntry.Name = ElementNames.BATCH_ITEM_LIST;
        eEntry.DataType = Codec.DataTypes.ARRAY;
        Assert.Equal(CodecReturnCode.SUCCESS, eEntry.EncodeInit(encodeIter, 0));

        /* Encode the array of requested item names */
        elementArray.PrimitiveType = Codec.DataTypes.ASCII_STRING;
        elementArray.ItemLength = 0;

        Assert.Equal(CodecReturnCode.SUCCESS, elementArray.EncodeInit(encodeIter));
        itemName.Data("TRI.N");
        Assert.Equal(CodecReturnCode.SUCCESS, ae.Encode(encodeIter, itemName));
        itemName.Data("IBM.N");
        Assert.Equal(CodecReturnCode.SUCCESS, ae.Encode(encodeIter, itemName));

        Assert.Equal(CodecReturnCode.SUCCESS, elementArray.EncodeComplete(encodeIter, true));

        Assert.Equal(CodecReturnCode.SUCCESS, eEntry.EncodeComplete(encodeIter, true));

        if (fieldIdList != null)
        {
            UInt tempUInt = new();
            Codec.Int tempInt = new();

            // encode view request
            eEntry.Clear();
            eEntry.Name = ElementNames.VIEW_TYPE;
            eEntry.DataType = Codec.DataTypes.UINT;

            tempUInt.Value(ViewTypes.FIELD_ID_LIST);

            Assert.Equal(CodecReturnCode.SUCCESS, eEntry.Encode(encodeIter, tempUInt));

            eEntry.Clear();
            eEntry.Name = ElementNames.VIEW_DATA;
            eEntry.DataType = Codec.DataTypes.ARRAY;
            Assert.Equal(CodecReturnCode.SUCCESS, eEntry.EncodeInit(encodeIter, 0));
            elementArray.Clear();
            elementArray.PrimitiveType = Codec.DataTypes.INT;
            elementArray.ItemLength = 0;

            Assert.Equal(CodecReturnCode.SUCCESS, elementArray.EncodeInit(encodeIter));

            foreach (var viewField in fieldIdList)
            {
                ae.Clear();
                tempInt.Value(viewField);
                Assert.Equal(CodecReturnCode.SUCCESS, ae.Encode(encodeIter, tempInt));
            }
            Assert.Equal(CodecReturnCode.SUCCESS, elementArray.EncodeComplete(encodeIter, true));
            Assert.Equal(CodecReturnCode.SUCCESS, eEntry.EncodeComplete(encodeIter, true));
        }

        Assert.Equal(CodecReturnCode.SUCCESS, eList.EncodeComplete(encodeIter, true));

        msg.EncodedDataBody = buf;
    }

    private void GenerateMarketPriceRequest(MarketPriceRequest marketPriceRequest, bool isPrivateStream, Service srcDirInfo,
        LoginRefresh loginInfo = null, List<string> viewFieldList = null)
    {
        marketPriceRequest.Clear();

        //if (!m_SnapshotRequested)
        //    marketPriceRequest.Streaming = true;

        marketPriceRequest.HasServiceId = true;
        marketPriceRequest.ServiceId = srcDirInfo.ServiceId;
        marketPriceRequest.HasPriority = true;
        marketPriceRequest.Priority = (1, 1);

        if (srcDirInfo.Info.QosList.Count > 0)
        {
            marketPriceRequest.HasQos = true;
            marketPriceRequest.Qos.IsDynamic = false;
            marketPriceRequest.Qos.TimeInfo(srcDirInfo.Info.QosList[0].TimeInfo());
            marketPriceRequest.Qos.Timeliness(srcDirInfo.Info.QosList[0].Timeliness());
            marketPriceRequest.Qos.RateInfo(srcDirInfo.Info.QosList[0].RateInfo());
            marketPriceRequest.Qos.Rate(srcDirInfo.Info.QosList[0].Rate());
        }
        if (isPrivateStream)
            marketPriceRequest.PrivateStream = true;

        if (loginInfo.HasFeatures
            && loginInfo.SupportedFeatures.HasSupportViewRequests
            && loginInfo.SupportedFeatures.SupportViewRequests == 1
            // && m_ViewRequested
            )
        {
            marketPriceRequest.HasView = true;
            marketPriceRequest.ViewFields.AddRange(viewFieldList ?? DEFAULT_VIEW_FIELD_LIST);
        }
    }

    private void EncodeMarketPriceRefresh(MarketPriceRefresh marketPriceRefresh, DataDictionary dictionary,
        MarketPriceItem marketItem, string itemName,
        int serviceId = -1, int streamId = -1,
        bool isStreaming = false, bool isSolicited = false, bool isPrivateStream = false)
    {
        marketPriceRefresh.Clear();
        marketPriceRefresh.DataDictionary = dictionary;
        if (isStreaming)
        {
            marketPriceRefresh.State.StreamState(StreamStates.OPEN);
        }
        else
        {
            marketPriceRefresh.State.StreamState(StreamStates.NON_STREAMING);
        }
        marketPriceRefresh.State.DataState(DataStates.OK);
        marketPriceRefresh.State.Code(StateCodes.NONE);
        marketPriceRefresh.State.Text().Data("Item Refresh Completed");
        // indicate that it is a single-part (atomic) refresh
        marketPriceRefresh.RefreshComplete = true;
        marketPriceRefresh.HasQos = true;

        if (isSolicited)
        {
            marketPriceRefresh.Solicited = true;

            // clear cache for solicited refresh messages.
            marketPriceRefresh.ClearCache = true;
        }

        if (isPrivateStream)
        {
            marketPriceRefresh.PrivateStream = true;
        }

        // Service Id
        marketPriceRefresh.HasServiceId = true;
        marketPriceRefresh.ServiceId = serviceId;

        // ItemName
        marketPriceRefresh.ItemName.Data(itemName);

        // Qos
        marketPriceRefresh.Qos.IsDynamic = false;
        marketPriceRefresh.Qos.Rate(QosRates.TICK_BY_TICK);
        marketPriceRefresh.Qos.Timeliness(QosTimeliness.REALTIME);

        // StreamId
        marketPriceRefresh.StreamId = streamId;
        marketPriceRefresh.MarketPriceItem = marketItem;
    }


    private void BasicDictionaryDirectoryRequestDo(ReactorSubmitOptions submitOptions,
        TestReactor consumerReactor, TestReactor providerReactor,
        Consumer consumer, Provider provider, out Service service)
    {
        ConsumerRole consumerRole = (ConsumerRole)consumer.ReactorRole;
        consumerRole.InitDefaultRDMLoginRequest();
        consumerRole.InitDefaultRDMDirectoryRequest();
        consumerRole.ChannelEventCallback = consumer;
        consumerRole.LoginMsgCallback = consumer;
        consumerRole.DirectoryMsgCallback = consumer;
        consumerRole.DictionaryMsgCallback = consumer;
        consumerRole.DefaultMsgCallback = consumer;
        // Consumer requests a Dictionary from Provider by defining dictionary name
        // and the corresponding download mode in the ConsumerRole supplied to Reactor
        consumerRole.DictionaryDownloadMode = DictionaryDownloadMode.FIRST_AVAILABLE;
        consumerRole.FieldDictionaryName.Data(FIELD_DICTIONARY_NAME);
        consumerRole.RdmDirectoryRequest = new DirectoryRequest();
        consumerRole.InitDefaultRDMDirectoryRequest();
        consumerRole.InitDefaultRDMEnumDictionaryRequest();
        consumerRole.InitDefaultRDMFieldDictionaryRequest();

        ProviderRole providerRole = (ProviderRole)provider.ReactorRole;
        providerRole.ChannelEventCallback = provider;
        providerRole.LoginMsgCallback = provider;
        providerRole.DirectoryMsgCallback = provider;
        providerRole.DictionaryMsgCallback = provider;
        providerRole.DefaultMsgCallback = provider;

        // Connect the consumer and provider. Setup login & directory streams automatically.
        ConsumerProviderSessionOptions opts = new ConsumerProviderSessionOptions();
        opts.SetupDefaultLoginStream = true;
        opts.SetupDefaultDirectoryStream = false;

        provider.Bind(opts);

        TestReactor.OpenSession(consumer, provider, opts);

        // ConsumerRole requested a directory, provide it
        providerReactor.Dispatch(1);
        TestReactorEvent evt = providerReactor.PollEvent();

        Assert.Equal(TestReactorEventType.DIRECTORY_MSG, evt.EventType);
        ReactorMsgEvent msgEvent = (ReactorMsgEvent)evt.ReactorEvent;
        Assert.Equal(MsgClasses.REQUEST, msgEvent.Msg.MsgClass);

        RDMDirectoryMsgEvent dirEvent = msgEvent as RDMDirectoryMsgEvent;
        Assert.NotNull(dirEvent);

        DirectoryMsg dirMsg = dirEvent.DirectoryMsg;
        Assert.NotNull(dirMsg);
        Assert.Equal(DirectoryMsgType.REQUEST, dirMsg.DirectoryMsgType);

        DirectoryRequest dirRequest = dirMsg.DirectoryRequest;
        Assert.NotNull(dirRequest);

        HandleDirectoryRequest(dirRequest, provider, submitOptions, out service);

        // Handle Directory Refresh that the Consumer received in response to the Directory Request
        consumerReactor.Dispatch(1);
        evt = consumerReactor.PollEvent();
        dirEvent = evt.ReactorEvent as RDMDirectoryMsgEvent;
        Assert.NotNull(dirEvent);
        Assert.Equal(DirectoryMsgType.REFRESH, dirEvent.DirectoryMsg.DirectoryMsgType);
        DirectoryRefresh dirRefresh = dirEvent.DirectoryMsg.DirectoryRefresh;
        Assert.NotNull(dirRefresh);
        Assert.NotEmpty(dirRefresh.ServiceList);

        // Consumer also sent two Dictionary requests
        providerReactor.Dispatch(2);
        evt = providerReactor.PollEvent();
        Assert.NotNull(evt);
        Assert.Equal(TestReactorEventType.DICTIONARY_MSG, evt.EventType);
        RDMDictionaryMsgEvent dictEvent = evt.ReactorEvent as RDMDictionaryMsgEvent;
        Assert.NotNull(dictEvent);
        Assert.Equal(DictionaryMsgType.REQUEST, dictEvent.DictionaryMsg.DictionaryMsgType);
        DictionaryRequest dictRequest = dictEvent.DictionaryMsg.DictionaryRequest;
        string dictionaryName1 = dictRequest.DictionaryName.ToString();

        evt = providerReactor.PollEvent();
        Assert.NotNull(evt);
        Assert.Equal(TestReactorEventType.DICTIONARY_MSG, evt.EventType);
        dictEvent = evt.ReactorEvent as RDMDictionaryMsgEvent;
        Assert.NotNull(dictEvent);
        Assert.Equal(DictionaryMsgType.REQUEST, dictEvent.DictionaryMsg.DictionaryMsgType);
        dictRequest = dictEvent.DictionaryMsg.DictionaryRequest;
        string dictionaryName2 = dictRequest.DictionaryName.ToString();

        Assert.True(FIELD_DICTIONARY_NAME.Equals(dictionaryName1)
            || FIELD_DICTIONARY_NAME.Equals(dictionaryName2));

        Assert.True(ENUM_TYPE_DICTIONARY_NAME.Equals(dictionaryName1)
            || ENUM_TYPE_DICTIONARY_NAME.Equals(dictionaryName2));
    }

    /// <summary>
    /// Sends directory refresh message depending on the given directory request.
    /// </summary>
    ///
    /// <remarks>
    /// This is essentially the same code as is used in VAProvider's
    /// DirectoryHandler.SendRefresh method.
    /// </remarks>
    private void HandleDirectoryRequest(DirectoryRequest srcDirReqInfo, Provider provider, ReactorSubmitOptions submitOptions, out Service service)
    {
        int ServiceId = 1234;
        string ServiceName = "INDIRECT_FEED";
        DirectoryRefresh directoryRefresh = new DirectoryRefresh();
        service = new();

        // encode source directory request
        directoryRefresh.Clear();
        directoryRefresh.StreamId = srcDirReqInfo.StreamId;

        // clear cache
        directoryRefresh.ClearCache = true;
        directoryRefresh.Solicited = true;

        // state information for response message
        directoryRefresh.State.Clear();
        directoryRefresh.State.StreamState(StreamStates.OPEN);
        directoryRefresh.State.DataState(DataStates.OK);
        directoryRefresh.State.Code(StateCodes.NONE);
        directoryRefresh.State.Text().Data("Source Directory Refresh Completed");

        // attribInfo information for response message
        directoryRefresh.Filter = srcDirReqInfo.Filter;

        // populate the Service
        service.Clear();
        service.Action = MapEntryActions.ADD;

        // set the service Id (map key)
        service.ServiceId = ServiceId;

        if ((srcDirReqInfo.Filter & ServiceFilterFlags.INFO) != 0)
        {
            service.HasInfo = true;
            service.Info.Action = FilterEntryActions.SET;

            // vendor
            service.Info.HasVendor = true;
            service.Info.Vendor.Data(VENDOR);

            // service name - required
            service.Info.ServiceName.Data(ServiceName);


            // Qos Range is not supported
            service.Info.HasSupportQosRange = true;
            service.Info.SupportsQosRange = 0;

            // capabilities - required
            service.Info.CapabilitiesList.Add((long)DomainType.MARKET_PRICE);
            service.Info.CapabilitiesList.Add((long)DomainType.MARKET_BY_ORDER);
            service.Info.CapabilitiesList.Add((long)DomainType.MARKET_BY_PRICE);
            service.Info.CapabilitiesList.Add((long)DomainType.DICTIONARY);
            service.Info.CapabilitiesList.Add((long)DomainType.SYMBOL_LIST);
            service.Info.CapabilitiesList.Add((long)DomainType.SYSTEM);

            // qos
            service.Info.HasQos = true;
            Qos qos = new Qos();
            qos.Rate(QosRates.TICK_BY_TICK);
            qos.Timeliness(QosTimeliness.REALTIME);
            service.Info.QosList.Add(qos);

            // dictionary used
            service.Info.HasDictionariesUsed = true;
            service.Info.DictionariesUsedList.Add(FIELD_DICTIONARY_NAME);
            service.Info.DictionariesUsedList.Add(ENUM_TYPE_DICTIONARY_NAME);

            // dictionary provided
            service.Info.HasDictionariesProvided = true;
            service.Info.DictionariesProvidedList.Add(FIELD_DICTIONARY_NAME);
            service.Info.DictionariesProvidedList.Add(ENUM_TYPE_DICTIONARY_NAME);

            // isSource = Service is provided directly from original publisher
            service.Info.HasIsSource = true;
            service.Info.IsSource = 1;

            // itemList - Name of SymbolList that includes all of the items that
            // the publisher currently provides.
            service.Info.HasItemList = true;
            service.Info.ItemList.Data("_ETA_ITEM_LIST");

            // accepting customer status = no
            service.Info.HasAcceptingConsStatus = true;
            service.Info.AcceptConsumerStatus = 1;

            // supports out of band snapshots = no
            service.Info.HasSupportOOBSnapshots = true;
            service.Info.SupportsOOBSnapshots = 0;
        }

        if ((srcDirReqInfo.Filter & ServiceFilterFlags.STATE) != 0)
        {
            service.HasState = true;
            service.State.Action = FilterEntryActions.SET;

            // service state
            service.State.ServiceStateVal = 1;

            // accepting requests
            service.State.HasAcceptingRequests = true;
            service.State.AcceptingRequests = 1;
        }

        if ((srcDirReqInfo.Filter & ServiceFilterFlags.LOAD) != 0)
        {
            service.HasLoad = true;
            service.Load.Action = FilterEntryActions.SET;

            // open limit
            service.Load.HasOpenLimit = true;
            service.Load.OpenLimit = OPEN_LIMIT;
        }

        if ((srcDirReqInfo.Filter & ServiceFilterFlags.LINK) != 0)
        {
            service.HasLink = true;
            service.Link.Action = FilterEntryActions.SET;

            ServiceLink serviceLink = new ServiceLink();

            // link name - Map Entry Key
            serviceLink.Name.Data(LINK_NAME);

            // link type
            serviceLink.HasType = true;
            serviceLink.Type = LinkTypes.INTERACTIVE;

            // link text
            serviceLink.HasText = true;
            serviceLink.Text.Data("Link state is up");
            service.Link.LinkList.Add(serviceLink);
        }

        directoryRefresh.ServiceList.Add(service);

        // send source directory request
        Assert.True(provider.SubmitAndDispatch(directoryRefresh, submitOptions) >= ReactorReturnCode.SUCCESS);
    }

    #endregion
}
