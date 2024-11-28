/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2023-2024 LSEG. All rights reserved.     
 *|-----------------------------------------------------------------------------
 */

using System.Collections.Generic;

using Xunit;
using LSEG.Eta.ValueAdd.Reactor;
using LSEG.Eta.ValueAdd.Rdm;
using LSEG.Eta.Codec;
using System;
using System.Net.Sockets;

namespace LSEG.Eta.ValuedAdd.Tests;

/// <summary>
/// This class represents a single Reactor.
/// </summary>
///
/// <remarks>
/// It providers simple ways to connect components (such as a consumer and
/// provider) and dispatch for events. The dispatched events are copied
/// (including any underlying data such as messages) and stored into an event
/// queue. You can then retrieve those events and test that they are correct.
/// </remarks>
///
public class TestReactor
{
    /** Controls whether reactor does XML tracing. */
    public static bool EnableReactorXmlTracing = false;

    public List<TestReactorComponent> ComponentList { get => m_ComponentList; }


    /// <summary>
    /// The associated reactor.
    /// </summary>
    private Reactor m_Reactor;

    public Reactor Reactor { get => m_Reactor; }

    /// <summary>
    /// Queue of events received from calling dispatch.
    /// </summary>
    internal Queue<TestReactorEvent> m_EventQueue;

    /// <summary>
    /// List of components associated with this reactor.
    /// </summary>
    List<TestReactorComponent> m_ComponentList;

    private List<Socket> m_ReadSocketList;


    /// <summary>
    /// Creates a TestReactor.
    /// </summary>
    public TestReactor()
    {

        ReactorOptions reactorOptions = new();

        m_EventQueue = new Queue<TestReactorEvent>();
        m_ComponentList = new List<TestReactorComponent>();

        if (EnableReactorXmlTracing)
            reactorOptions.XmlTracing = true;

        m_Reactor = Reactor.CreateReactor(reactorOptions, out _);

        m_ReadSocketList = new();
    }

    /// <summary>
    /// Creates a TestReactor.
    /// </summary>
    /// <param name="reactorOptions"></param>
    public TestReactor(ReactorOptions reactorOptions)
    {

        m_EventQueue = new Queue<TestReactorEvent>();
        m_ComponentList = new List<TestReactorComponent>();

        if (EnableReactorXmlTracing)
            reactorOptions.XmlTracing = true;

        m_Reactor = Reactor.CreateReactor(reactorOptions, out var errorInfo);

        m_ReadSocketList = new();
    }


    /// <summary>
    /// Calls dispatch on the component's Reactor, and will store received events for
    /// later review.
    /// </summary>
    ///
    /// The test will verify that the exact number of events is received, and will fail if fewer
    /// or more are received.
    ///
    /// <param name="expectedEventCount">The exact number of events that should be received.</param>
    ///
    public void Dispatch(int expectedEventCount)
    {
        Dispatch(expectedEventCount, false);
    }

    /// <summary>
    /// Calls dispatch on the component's Reactor, and will store received events for
    /// later review.
    /// </summary>
    ///
    /// The test will verify that the exact number of events is received, and will fail if
    /// fewer or more are received.
    ///
    /// Used for forced close connection from client, use {@link #dispatch(int)} for ordinary scenarios
    ///
    /// <param name="expectedEventCount">The exact number of events that should be received.</param>
    /// <param name="channelWasClosed">true if client forcible have closed channel</param>
    ///
    public void Dispatch(int expectedEventCount, bool channelWasClosed)
    {
        TimeSpan timeout = (expectedEventCount > 0)
            ? TimeSpan.FromMilliseconds(1000)
            : TimeSpan.FromMilliseconds(200);

        Dispatch(expectedEventCount, timeout, channelWasClosed);
    }


    /// Waits for notification on the component's Reactor, and calls dispatch when
    /// triggered.  It will store any received events for later review.
    ///
    /// Waiting for notification stops once the expected number of events is received
    /// (unless that number is zero, in which case it waits for the full specified time).
    ///
    /// <param name="expectedEventCount">The exact number of events that should be received.</param>
    /// <param name="timeoutMsec">The maximum time to wait for all events.</param>
    ///
    public void Dispatch(int expectedEventCount, TimeSpan timeout)
    {
        Dispatch(expectedEventCount, timeout, false);
    }

    /// Waits for notification on the component's Reactor, and calls dispatch when
    /// triggered. It will store any received events for later review.
    ///
    /// Waiting for notification stops once the expected number of events is received
    /// (unless that number is zero, in which case it waits for the full specified time).
    ///
    /// Used for forced close connection from client, use {@link #dispatch(int)} for
    /// ordinary scenarios
    ///
    /// <param name="expectedEventCount">The exact number of events that should be received.</param>
    /// <param name="timeoutMsec">The maximum time to wait for all events.</param>
    /// <param name="channelWasClosed">true if client forcible have closed channel</param>
    ///
    public void Dispatch(int expectedEventCount, TimeSpan timeout, bool channelWasClosed)
    {
        int selectRet = 0;
        System.DateTime currentTime, stopTime;
        ReactorReturnCode lastDispatchRet = 0;

        /* Ensure no events were missed from previous calls to dispatch.
         * But don't check event queue size when expectedEventCount is set to -1 */
        if (expectedEventCount != -1)
            Assert.Empty(m_EventQueue);

        currentTime = System.DateTime.Now;

        stopTime = currentTime + timeout;

        do
        {
            if (lastDispatchRet == 0)
            {
                TimeSpan selectTime;

                var eventSocket = m_Reactor.EventSocket;

                //check if channel still exists and opened
                if (eventSocket == null)
                {
                    if (channelWasClosed)
                        return;
                    else
                        TestUtil.Fail("No selectable channel exists");
                }
                m_ReadSocketList.Add(m_Reactor.EventSocket);

                foreach (TestReactorComponent component in m_ComponentList)
                {
                    if (component.ReactorChannel != null
                        && component.IsReactorChannelUp
                        && component.ReactorChannel.Socket != null)
                    {
                        m_ReadSocketList.Add(component.ReactorChannel.Socket);
                    }
                }
                selectTime = (stopTime - currentTime);

                if (m_ReadSocketList.Count == 0)
                    selectRet = 1;
                else
                {
                    m_ReadSocketList.RemoveAll(socket => socket.SafeHandle.IsClosed);
                    try
                    {
                        if (selectTime > TimeSpan.Zero)
                        {
                            Socket.Select(m_ReadSocketList, null, null, (int)selectTime.TotalMilliseconds * 1000);
                        }
                        else
                        {
                            Socket.Select(m_ReadSocketList, null, null, 0);
                        }
                    }
                    catch (ObjectDisposedException)
                    {
                        m_ReadSocketList.RemoveAll(socket => socket.SafeHandle.IsClosed);
                    }             

                    selectRet = m_ReadSocketList.Count;
                }
            }
            else
                selectRet = 1;

            if (selectRet > 0)
            {
                ReactorDispatchOptions dispatchOpts = new();

                do
                {
                    dispatchOpts.Clear();
                    lastDispatchRet = m_Reactor.Dispatch(dispatchOpts, out var dispatchErrorInfo);
                    Assert.True(lastDispatchRet >= 0,
                        $"Dispatch failed: {lastDispatchRet} ({dispatchErrorInfo?.Location} -- {dispatchErrorInfo?.Error?.Text})");
                }
                while (lastDispatchRet > 0);
            }

            currentTime = System.DateTime.Now;

            /* If we've hit our expected number of events, drop the stopping time to at
             * most 100ms from now.
             *
             * Keep dispatching a short time to ensure no unexpected events are received,
             * and that any internal flush events are processed. */
            if (expectedEventCount > 0
                && m_EventQueue.Count == expectedEventCount)
            {
                System.DateTime stopTimeNew = currentTime + TimeSpan.FromMilliseconds(100);
                if ((stopTime - stopTimeNew) > TimeSpan.Zero)
                {
                    stopTime = stopTimeNew;
                }
            }

        }
        while (currentTime < stopTime);

        /* Don't check event queue size when expectedEventCount is set to -1 */
        if (expectedEventCount != -1)
            Assert.Equal(expectedEventCount, m_EventQueue.Count);
    }


    /// <summary>
    /// Stores the received channel event, and updates the relevant component's channel information.
    /// </summary>
    public ReactorCallbackReturnCode HandleChannelEvent(ReactorChannelEvent evt)
    {
        TestReactorComponent component = (TestReactorComponent)evt.ReactorChannel.UserSpecObj;

        switch (evt.EventType)
        {
            case ReactorChannelEventType.CHANNEL_OPENED:
                Assert.False(component.IsReactorChannelUp);
                break;
            case ReactorChannelEventType.CHANNEL_UP:
                component.IsReactorChannelUp = true;
                break;
            case ReactorChannelEventType.CHANNEL_DOWN:
            case ReactorChannelEventType.CHANNEL_DOWN_RECONNECTING:
                if (component.IsReactorChannelUp)
                {
                    /* If channel was up, the selectableChannel should be present. */
                    Assert.NotNull(component.ReactorChannel.Socket);

                    /* Cancel key. */
                    m_ReadSocketList.Remove(component.ReactorChannel.Socket);
                }

                component.IsReactorChannelUp = false;
                break;
            case ReactorChannelEventType.CHANNEL_READY:
            case ReactorChannelEventType.FD_CHANGE:
            case ReactorChannelEventType.WARNING:
                Assert.NotNull(component.ReactorChannel);
                break;
            default:
                TestUtil.Fail("Unhandled ReactorChannelEventType.");
                break;

        }

        if (component.ReactorChannel != null)
            Assert.Equal(component.ReactorChannel, evt.ReactorChannel);
        else
            component.ReactorChannel = evt.ReactorChannel;

        m_EventQueue.Enqueue(new TestReactorEvent(TestReactorEventType.CHANNEL_EVENT, evt));
        return ReactorCallbackReturnCode.SUCCESS;
    }

    /// <summary>
    /// Stores a login message event.
    /// </summary>
    public ReactorCallbackReturnCode HandleLoginMsgEvent(RDMLoginMsgEvent evt)
    {
        m_EventQueue.Enqueue(new TestReactorEvent(TestReactorEventType.LOGIN_MSG, evt));
        return ReactorCallbackReturnCode.SUCCESS;
    }


    /// <summary>
    /// Stores a directory message event.
    /// </summary>
    public ReactorCallbackReturnCode HandleDirectoryMsgEvent(RDMDirectoryMsgEvent evt)
    {
        m_EventQueue.Enqueue(new TestReactorEvent(TestReactorEventType.DIRECTORY_MSG, evt));
        return ReactorCallbackReturnCode.SUCCESS;
    }

    /// <summary>
    /// Stores a dictionary message event.
    /// </summary>
    public ReactorCallbackReturnCode HandleDictionaryMsgEvent(RDMDictionaryMsgEvent evt)
    {
        m_EventQueue.Enqueue(new TestReactorEvent(TestReactorEventType.DICTIONARY_MSG, evt));
        return ReactorCallbackReturnCode.SUCCESS;
    }

    /// <summary>
    /// Stores a message event.
    /// </summary>
    public ReactorCallbackReturnCode HandleDefaultMsgEvent(ReactorMsgEvent evt)
    {
        m_EventQueue.Enqueue(new TestReactorEvent(TestReactorEventType.MSG, evt));
        return ReactorCallbackReturnCode.SUCCESS;
    }

    /// <summary>
    /// Retrieves an event from the list of events received from a dispatch call.
    /// </summary>
    public TestReactorEvent PollEvent()
    {
        return m_EventQueue.Dequeue();
    }

    /// <summary>
    /// Adds a component to the Reactor.
    /// </summary>
    public void AddComponent(TestReactorComponent component)
    {
        Assert.DoesNotContain(component, m_ComponentList);
        m_ComponentList.Add(component);
    }

    /// <summary>
    /// Adds a component's server to the selector.
    /// </summary>
    public void RegisterComponentServer(TestReactorComponent component)
    {
        Assert.NotNull(component.Server);
        m_ReadSocketList.Add(component.Server.Socket);
    }


    /// <summary>
    /// Removes a component from the Reactor.
    /// </summary>
    public void RemoveComponent(TestReactorComponent component)
    {
        Assert.Contains(component, m_ComponentList);
        m_ComponentList.Remove(component);
    }


    /// <summary>
    /// Associates a component with this reactor and opens a connection.
    /// </summary>
    internal void Connect(ConsumerProviderSessionOptions opts, TestReactorComponent component, int portNumber)
    {
        ReactorConnectOptions connectOpts = new();
        ReactorConnectInfo connectInfo = new();

        connectOpts.Clear();
        connectOpts.ConnectionList.Add(connectInfo);
        connectOpts.ConnectionList[0].ConnectOptions.MajorVersion = Codec.Codec.MajorVersion();
        connectOpts.ConnectionList[0].ConnectOptions.MinorVersion = Codec.Codec.MinorVersion();
        connectOpts.ConnectionList[0].ConnectOptions.ConnectionType = opts.ConnectionType;
        connectOpts.ConnectionList[0].ConnectOptions.UserSpecObject = component;
        connectOpts.ConnectionList[0].ConnectOptions.CompressionType = opts.CompressionType;
        connectOpts.ConnectionList[0].ConnectOptions.NumInputBuffers = 20;

        connectOpts.SetReconnectAttempLimit(opts.ReconnectAttemptLimit);
        connectOpts.SetReconnectMinDelay((int)opts.ReconnectMinDelay.TotalMilliseconds);
        connectOpts.SetReconnectMaxDelay((int)opts.ReconnectMaxDelay.TotalMilliseconds);
        connectOpts.ConnectionList[0].ConnectOptions.PingTimeout = (int)opts.PingTimeout.TotalSeconds;
        connectOpts.ConnectionList[0].SetInitTimeout((int)opts.ConsumerChannelInitTimeout.TotalSeconds);

        connectOpts.ConnectionList[0].ConnectOptions.UnifiedNetworkInfo.Address = "localhost";
        connectOpts.ConnectionList[0].ConnectOptions.UnifiedNetworkInfo.ServiceName = portNumber.ToString();

        if(opts.SysSendBufSize > 0)
            connectOpts.ConnectionList[0].ConnectOptions.SysSendBufSize = opts.SysSendBufSize;

        if (opts.SysRecvBufSize > 0)
            connectOpts.ConnectionList[0].ConnectOptions.SysRecvBufSize = opts.SysRecvBufSize;

        ReactorReturnCode connectCode = m_Reactor.Connect(connectOpts, component.ReactorRole, out var connectError);
        Assert.True(ReactorReturnCode.SUCCESS == connectCode,
            $"Connect failed: {connectCode} ({connectError?.Location} -- {connectError?.Error?.Text})");

        /* Clear ReactorConnectOptions after connecting -- this tests whether the Reactor
         * is properly saving the options. */
        connectOpts.Clear();

    }

    /// <summary>
    /// Associates a component with this reactor and accepts a connection.
    /// </summary>
    internal void Accept(ConsumerProviderSessionOptions opts, TestReactorComponent component)
    {
        Accept(opts, component, TimeSpan.FromMilliseconds(5000));
    }

    /// <summary>
    /// Associates a component with this reactor and accepts a connection.
    /// </summary>
    internal void Accept(ConsumerProviderSessionOptions opts, TestReactorComponent component, TimeSpan timeout)
    {
        ReactorConnectOptions connectOpts = new();

        Assert.NotNull(component.Server);
        Assert.Null(component.ReactorChannel);

        if(m_ReadSocketList.Count == 0)
        {
            m_ReadSocketList.Add(component.Server.Socket);
        }

        /* Wait for server channel to trigger. */
        System.DateTime stopTime = System.DateTime.Now + timeout;

        do
        {
            try
            {
                Socket.Select(m_ReadSocketList, null, null, (int)(stopTime - System.DateTime.Now).TotalMilliseconds * 1000);
                Assert.NotEmpty(m_ReadSocketList);

                Dispatch(0, TimeSpan.FromMilliseconds(100));

                foreach (var key in m_ReadSocketList)
                {
                    if (key == m_Reactor.EventSocket)
                    {
                        /* Reactor's channel triggered. Should get no events. */
                        Dispatch(0, TimeSpan.FromMilliseconds(100));
                    }
                    else if (key == component.Server.Socket)
                    {
                        m_ReadSocketList.Clear();

                        ReactorAcceptOptions acceptOpts = new();

                        /* Accept connection. */
                        // acceptOpts.Clear();
                        acceptOpts.AcceptOptions.UserSpecObject = component;
                        Assert.Equal(ReactorReturnCode.SUCCESS,
                                     m_Reactor.Accept(component.Server, acceptOpts, component.ReactorRole, out _));
                        return;
                    }
                }
            }
            catch (Exception e)
            {
                TestUtil.Fail($"Caught I/O exception: {e.Message}");
            }
        }
        while (System.DateTime.Now < stopTime);

        TestUtil.Fail("Server did not receive accept notification.");
    }

    /// <summary>
    /// Connects a Consumer and Provider component to each other.
    /// </summary>
    public static void OpenSession(Consumer consumer, Provider provider, ConsumerProviderSessionOptions opts)
    {
        OpenSession(consumer, provider, opts, false);
    }

    /// <summary>
    /// Connects a Consumer and Provider component to each other.
    /// </summary>
    public static void OpenSession(Consumer consumer, Provider provider, ConsumerProviderSessionOptions opts, bool recoveringChannel)
    {
        TestReactorEvent evt;
        ReactorChannelEvent channelEvent;
        RDMLoginMsgEvent loginMsgEvent;
        RDMDirectoryMsgEvent directoryMsgEvent;
        ConsumerRole consumerRole = (ConsumerRole)consumer.ReactorRole;
        ReactorSubmitOptions submitOptions = new();

        if (!recoveringChannel)
            consumer.TestReactor.Connect(opts, consumer, provider.ServerPort);

        /* Preset login message required if automatically setting up login stream. */
        Assert.True(opts.SetupDefaultLoginStream == false || consumerRole.RdmLoginRequest != null);

        /* Preset directory message required, or watchlist must be enabled, if automatically setting up directory stream */
        Assert.True(opts.SetupDefaultDirectoryStream == false
            || consumerRole.WatchlistOptions.EnableWatchlist == true
            || consumerRole.RdmDirectoryRequest != null);

        if (consumerRole.WatchlistOptions.EnableWatchlist)
        {
            foreach (var component in consumer.TestReactor.ComponentList)
            {
                if (opts.LoginHandler != null)
                    component.ReactorChannel.Watchlist.LoginHandler = opts.LoginHandler;
                if (opts.DirectoryHandler != null)
                    component.ReactorChannel.Watchlist.DirectoryHandler = opts.DirectoryHandler;
                if (opts.ItemHandler != null)
                    component.ReactorChannel.Watchlist.ItemHandler = opts.ItemHandler;
            }
        }
        /* If watchlist enabled, should get ChannelOpenCallback */
        if (consumerRole.WatchlistOptions.EnableWatchlist
            && consumerRole.WatchlistOptions.ChannelOpenEventCallback != null
            && recoveringChannel == false)
        {
            evt = consumer.TestReactor.PollEvent();
            Assert.Equal(TestReactorEventType.CHANNEL_EVENT, evt.EventType);
            channelEvent = (ReactorChannelEvent)evt.ReactorEvent;
            Assert.Equal(ReactorChannelEventType.CHANNEL_OPENED, channelEvent.EventType);
        }
        else
        {
            consumer.TestReactor.Dispatch(0);
        }

        provider.TestReactor.Accept(opts, provider);

        /* Provider receives channel-up/channel-ready */
        if (recoveringChannel)
        {
            provider.TestReactor.Dispatch(2, TimeSpan.FromSeconds(6));
        }
        else
        {
            provider.TestReactor.Dispatch(2);
        }

        evt = provider.TestReactor.PollEvent();
        Assert.Equal(TestReactorEventType.CHANNEL_EVENT, evt.EventType);
        channelEvent = (ReactorChannelEvent)evt.ReactorEvent;
        Assert.Equal(ReactorChannelEventType.CHANNEL_UP, channelEvent.EventType);

        evt = provider.TestReactor.PollEvent();
        Assert.Equal(TestReactorEventType.CHANNEL_EVENT, evt.EventType);
        channelEvent = (ReactorChannelEvent)evt.ReactorEvent;
        Assert.Equal(ReactorChannelEventType.CHANNEL_READY, channelEvent.EventType);

        /* Consumer receives channel-up and any status events due the watchlist
         * items submitted in channel open callback. */

        if (consumerRole.RdmLoginRequest == null)
            consumer.TestReactor.Dispatch(2 + opts.NumStatusEvents);
        else
            consumer.TestReactor.Dispatch(1 + opts.NumStatusEvents);
        for (int i = 0; i < opts.NumStatusEvents; i++)
        {
            evt = consumer.TestReactor.PollEvent();
            Assert.Equal(TestReactorEventType.MSG, evt.EventType);
            ReactorMsgEvent msgEvent = (ReactorMsgEvent)evt.ReactorEvent;
            Assert.Equal(MsgClasses.STATUS, msgEvent.Msg.MsgClass);
        }
        evt = consumer.TestReactor.PollEvent();
        Assert.Equal(TestReactorEventType.CHANNEL_EVENT, evt.EventType);
        channelEvent = (ReactorChannelEvent)evt.ReactorEvent;
        Assert.Equal(ReactorChannelEventType.CHANNEL_UP, channelEvent.EventType);

        if (consumerRole.RdmLoginRequest == null)
        {
            /* Consumer receives channel-ready, then we're done. */
            //consumer.TestReactor.Dispatch(1);
            evt = consumer.TestReactor.PollEvent();
            Assert.Equal(TestReactorEventType.CHANNEL_EVENT, evt.EventType);
            channelEvent = (ReactorChannelEvent)evt.ReactorEvent;
            Assert.Equal(ReactorChannelEventType.CHANNEL_READY, channelEvent.EventType);

            provider.TestReactor.Dispatch(0);

            return;
        }

        if (!opts.SetupDefaultLoginStream)
            return;

        /* Provider receives login request. */
        provider.TestReactor.Dispatch(1);
        evt = provider.TestReactor.PollEvent();
        Assert.Equal(TestReactorEventType.LOGIN_MSG, evt.EventType);
        loginMsgEvent = (RDMLoginMsgEvent)evt.ReactorEvent;
        Assert.Equal(LoginMsgType.REQUEST, loginMsgEvent.LoginMsg.LoginMsgType);

        /* Provider sends a default login refresh. */
        LoginRequest loginRequest = new();
        LoginRefresh loginRefresh = new();

        loginRefresh.Clear();
        loginRefresh.Solicited = true;
        loginRefresh.UserNameType = loginRequest.UserNameType;
        loginRefresh.HasUserName = !string.IsNullOrEmpty(loginRequest.UserName.ToString());
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

        /* Consumer receives login refresh. */
        if (consumerRole.RdmDirectoryRequest == null && consumerRole.WatchlistOptions.EnableWatchlist == false)
            consumer.TestReactor.Dispatch(2);
        else
            consumer.TestReactor.Dispatch(1);

        /* Save the stream ID used by each component to open the login stream (may be
         * different if the watchlist is enabled). */
        consumer.DefaultSessionLoginStreamId = consumerRole.RdmLoginRequest.StreamId;
        provider.DefaultSessionLoginStreamId = loginRequest.StreamId;

        evt = consumer.TestReactor.PollEvent();
        Assert.Equal(TestReactorEventType.LOGIN_MSG, evt.EventType);
        loginMsgEvent = (RDMLoginMsgEvent)evt.ReactorEvent;
        Assert.Equal(LoginMsgType.REFRESH, loginMsgEvent.LoginMsg.LoginMsgType);

        if (consumerRole.RdmDirectoryRequest == null && consumerRole.WatchlistOptions.EnableWatchlist == false)
        {
            /* Consumer receives channel-ready. */
            evt = consumer.TestReactor.PollEvent();
            Assert.Equal(TestReactorEventType.CHANNEL_EVENT, evt.EventType);
            channelEvent = (ReactorChannelEvent)evt.ReactorEvent;
            Assert.Equal(ReactorChannelEventType.CHANNEL_READY, channelEvent.EventType);

            provider.TestReactor.Dispatch(0);

            /* when watchlist is not enabled, no directory exchange occurs. We're done. */
            if (consumerRole.WatchlistOptions.EnableWatchlist == false)
            {
                return;
            }
        }

        if (opts.SetupDefaultDirectoryStream == false && consumerRole.WatchlistOptions.EnableWatchlist == false)
            return;

        /* Provider receives directory request. */
        provider.TestReactor.Dispatch(1);
        evt = provider.TestReactor.PollEvent();
        Assert.Equal(TestReactorEventType.DIRECTORY_MSG, evt.EventType);
        directoryMsgEvent = (RDMDirectoryMsgEvent)evt.ReactorEvent;
        Assert.Equal(DirectoryMsgType.REQUEST, directoryMsgEvent.DirectoryMsg.DirectoryMsgType);

        /* Provider sends a default directory refresh. */
        DirectoryRequest directoryRequest = directoryMsgEvent.DirectoryMsg.DirectoryRequest;
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
        Service service2 = new();
        Provider.DefaultService.Copy(service);
        Provider.DefaultService2.Copy(service2);

        // Apply OpenWindow to the service if one is specified.
        if (opts.OpenWindow >= 0)
        {
            service.HasLoad = true;
            service.Load.HasOpenWindow = true;
            service.Load.OpenWindow = opts.OpenWindow;
        }

        directoryRefresh.ServiceList.Add(service);
        if (opts.SetupSecondDefaultDirectoryStream)
        {
            directoryRefresh.ServiceList.Add(service2);
        }
        submitOptions.Clear();
        Assert.True(provider.SubmitAndDispatch(directoryRefresh, submitOptions) >= ReactorReturnCode.SUCCESS);

        if (opts.SetupDefaultDirectoryStream == true)
            consumer.TestReactor.Dispatch(2);
        else
            consumer.TestReactor.Dispatch(1);

        /* Consumer receives directory refresh. */
        evt = consumer.TestReactor.PollEvent();
        if (opts.SetupDefaultDirectoryStream == true)
        {
            Assert.Equal(TestReactorEventType.DIRECTORY_MSG, evt.EventType);
            directoryMsgEvent = (RDMDirectoryMsgEvent)evt.ReactorEvent;
            if (!recoveringChannel || consumerRole.WatchlistOptions.EnableWatchlist == false)
                Assert.Equal(DirectoryMsgType.REFRESH, directoryMsgEvent.DirectoryMsg.DirectoryMsgType);
            else
                Assert.Equal(DirectoryMsgType.UPDATE, directoryMsgEvent.DirectoryMsg.DirectoryMsgType);

            /* Save the stream ID used by each component to open the directory stream (may
             * be different if the watchlist is enabled). */
            consumer.DefaultSessionDirectoryStreamId = consumerRole.RdmDirectoryRequest.StreamId;
            provider.DefaultSessionDirectoryStreamId = directoryRequest.StreamId;

            /* Consumer receives channel-ready. */
            evt = consumer.TestReactor.PollEvent();
            Assert.Equal(TestReactorEventType.CHANNEL_EVENT, evt.EventType);
            channelEvent = (ReactorChannelEvent)evt.ReactorEvent;
            Assert.Equal(ReactorChannelEventType.CHANNEL_READY, channelEvent.EventType);
        }
        else // only channel event comes in this case
        {
            /* Consumer receives channel-ready. */
            Assert.Equal(TestReactorEventType.CHANNEL_EVENT, evt.EventType);
            channelEvent = (ReactorChannelEvent)evt.ReactorEvent;
            Assert.Equal(ReactorChannelEventType.CHANNEL_READY, channelEvent.EventType);
        }
    }

    /// <summary>
    /// Cleans up the TestReactor's resources (e.g. its reactor)
    /// </summary>
    public void Close()
    {
        if (m_Reactor != null)
        {
            Assert.Equal(ReactorReturnCode.SUCCESS, m_Reactor.Shutdown(out _));
            m_Reactor = null;
        }

        if (m_EventQueue != null)
            m_EventQueue.Clear();
    }
}
