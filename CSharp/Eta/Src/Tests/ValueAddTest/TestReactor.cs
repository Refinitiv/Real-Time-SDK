/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2023-2025 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

using System.Collections.Generic;

using Xunit;
using LSEG.Eta.ValueAdd.Reactor;
using LSEG.Eta.ValueAdd.Rdm;
using LSEG.Eta.Codec;
using System;
using System.Linq;
using System.Net.Sockets;

using static LSEG.Eta.Tests.ValueAddTest.TestUtil;

namespace LSEG.Eta.Tests.ValueAddTest;

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
public class TestReactor : IDisposable
{
    private static readonly Dictionary<Type, int> m_MsgTypeToMsgClass = new()
    {
        { typeof(IAckMsg), MsgClasses.ACK},
        { typeof(ICloseMsg), MsgClasses.CLOSE},
        { typeof(IGenericMsg), MsgClasses.GENERIC},
        { typeof(IPostMsg), MsgClasses.POST },
        { typeof(IRefreshMsg), MsgClasses.REFRESH },
        { typeof(IRequestMsg), MsgClasses.REQUEST },
        { typeof(IStatusMsg), MsgClasses.STATUS },
        { typeof(IUpdateMsg), MsgClasses.UPDATE },
    };

    /** Controls whether reactor does XML tracing. */
    public static bool EnableReactorXmlTracing = false;

    public List<TestReactorComponent> ComponentList { get => m_ComponentList; }

    /// <summary>
    /// Default timeout for <see cref="Dispatch(int)"/> & <see cref="Dispatch(int, bool)"/> methods,
    /// when expected event count is unlimited (i.e. -1).
    /// </summary>
    public TimeSpan DefaultDispatchTimeout { get; set; } = TimeSpan.FromMilliseconds(200);

    /// <summary>
    /// Default timeout for <see cref="Dispatch(int)"/> & <see cref="Dispatch(int, bool)"/> methods,
    /// when expected event count is specified.
    /// </summary>
    public TimeSpan DefaultDispatchTimeoutForEventCount { get; set; } = TimeSpan.FromSeconds(10);


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

    #region Utility methods

    private string GetEventQueueDump(bool showCount = true) =>
        @$"Event queue contents{(showCount ? $" ({m_EventQueue.Count} element(s))" : "")}:
{string.Join("," + Environment.NewLine, m_EventQueue.Select(x => $" - {x?.ToString() ?? "<null>"}"))}
--------";

    #endregion

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
    /// <remarks>
    /// The test will verify that the exact number of events is received, and will fail if fewer
    /// or more are received.
    /// </remarks>
    /// <param name="expectedEventCount">The exact number of events that should be received.</param>
    public TestReactor Dispatch(int expectedEventCount) =>
        Dispatch(expectedEventCount, false);

    /// <summary>
    /// Calls dispatch on the component's Reactor, and will store received events for
    /// later review.
    /// </summary>
    /// <remarks>
    /// The test will verify that the exact number of events is received, and will fail if
    /// fewer or more are received.
    ///
    /// Used for forced close connection from client, use <see cref="Dispatch(int)"/> for ordinary scenarios
    /// </remarks>
    /// <param name="expectedEventCount">The exact number of events that should be received.</param>
    /// <param name="channelWasClosed">true if client forcible have closed channel</param>
    public TestReactor Dispatch(int expectedEventCount, bool channelWasClosed)
    {
        TimeSpan timeout = (expectedEventCount > 0)
            ? DefaultDispatchTimeoutForEventCount
            : DefaultDispatchTimeout;

        return Dispatch(expectedEventCount, timeout, channelWasClosed);
    }


    /// <summary>
    /// Waits for notification on the component's Reactor, and calls dispatch when
    /// triggered.  It will store any received events for later review.
    /// </summary>
    /// <remarks>
    /// Waiting for notification stops once the expected number of events is received
    /// (unless that number is zero, in which case it waits for the full specified time).
    /// </remarks>
    /// <param name="expectedEventCount">The exact number of events that should be received.</param>
    /// <param name="timeout">The maximum time to wait for all events.</param>
    public TestReactor Dispatch(int expectedEventCount, TimeSpan timeout) =>
        Dispatch(expectedEventCount, timeout, false);

    /// <summary>
    /// Waits for notification on the component's Reactor, and calls dispatch when
    /// triggered. It will store any received events for later review.
    /// </summary>
    /// <remarks>
    /// Waiting for notification stops once the expected number of events is received
    /// (unless that number is zero, in which case it waits for the full specified time).
    ///
    /// Used for forced close connection from client, use <see cref="Dispatch(int)"/> for
    /// ordinary scenarios
    /// <remarks>
    /// <param name="expectedEventCount">The exact number of events that should be received.</param>
    /// <param name="timeout">The maximum time to wait for all events.</param>
    /// <param name="channelWasClosed">true if client forcible have closed channel</param>
    public TestReactor Dispatch(int expectedEventCount, TimeSpan timeout, bool channelWasClosed)
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
                        return this;
                    else
                        TestUtil.Fail("No selectable channel exists");
                }
                m_ReadSocketList.Add(m_Reactor.EventSocket);

                foreach (TestReactorComponent component in m_ComponentList)
                {
                    if (component.ReactorChannel != null
                        && component.IsReactorChannelUp
                        && component.ReactorChannel.Socket != null
                        && !component.ReactorChannel.Socket.SafeHandle.IsClosed)
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
                    Assert.True(lastDispatchRet >= ReactorReturnCode.SUCCESS,
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
            Assert.True(expectedEventCount == m_EventQueue.Count, $@"
Failure:  Event queue size differ
Expected: {expectedEventCount}
Actual:   {m_EventQueue.Count}
{GetEventQueueDump(showCount: false)}");

        return this;
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
            case ReactorChannelEventType.PREFERRED_HOST_COMPLETE:
            case ReactorChannelEventType.PREFERRED_HOST_STARTING_FALLBACK:
            case ReactorChannelEventType.PREFERRED_HOST_NO_FALLBACK:
                Assert.NotNull(component.ReactorChannel);
                break;
            default:
                TestUtil.Fail($"Unhandled ReactorChannelEventType: {evt.EventType}.");
                break;

        }

        // ReactorChannel may be different when current Provider has been contacted again.
        // So this check is only relevant for Consumer.
        if (component.ReactorChannel != null && component.ReactorRole.Type == ReactorRoleType.CONSUMER)
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

    public TEvent PollReactorEvent<TEvent>()
        where TEvent : ReactorEvent
    {
        var @event = PollEvent();
        Assert.IsAssignableFrom<TEvent>(@event.ReactorEvent);
        return (TEvent)@event.ReactorEvent;
    }

    public TMsg PollMsgEvent<TMsg>()
        where TMsg : IMsg
    {
        var msgEvent = PollReactorEvent<ReactorMsgEvent>();

        if (m_MsgTypeToMsgClass.TryGetValue(typeof(TMsg), out var msgClass) &&
                msgClass != msgEvent.Msg.MsgClass)
            Assert.Fail($@"
Failure: Wrong message class
Expected: {MsgClasses.ToString(msgClass)}
Actual:   {MsgClasses.ToString(msgEvent.Msg.MsgClass)}
Message XML: {msgEvent.Msg.ToXml()}");

        return (TMsg) msgEvent.Msg;
    }

    public TestReactor AssertReactorChannelEvent(ReactorChannelEventType type) =>
        AssertEventType(type, (ReactorChannelEvent evt) => evt.EventType);

    public TestReactor AssertLoginMsgEvent(LoginMsgType type) =>
        AssertEventType(type, (RDMLoginMsgEvent evt) => evt.LoginMsg.LoginMsgType);

    public TestReactor AssertDirectoryMsgEvent(DirectoryMsgType type) =>
        AssertEventType(type, (RDMDirectoryMsgEvent evt) => evt.DirectoryMsg.DirectoryMsgType);

    private TestReactor AssertEventType<TEvent, TEventType>(TEventType type, Func<TEvent, TEventType> getEventType)
        where TEvent : ReactorEvent
    {
        var reactorEvent = PollReactorEvent<TEvent>();
        var actualEventType = getEventType(reactorEvent);
        Assert.True(type.Equals(actualEventType), $@"
Failure:  Wrong reactor event type
Expected: {type}
Actual:   {actualEventType}
{GetEventQueueDump()}");

        return this;
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

        connectOpts.Clear();
        MapToReactorConnectOptions(connectOpts, opts, component, new[] {portNumber});

        ConnectChecked(connectOpts, component.ReactorRole);
    }

    /// <summary>
    /// Associates a component with this reactor and opens a connection.
    /// </summary>
    internal void Connect(ConsumerProviderSessionOptions opts, TestReactorComponent component, int[] portNumbers)
    {
        if (portNumbers.Length == 0)
            throw new ArgumentException("At least 1 port number must be specified", nameof(portNumbers));

        ReactorConnectOptions connectOpts = new();

        connectOpts.Clear();
        MapToReactorConnectOptions(connectOpts, opts, component, portNumbers);

        ConnectChecked(connectOpts, component.ReactorRole);
    }

    private static void MapToReactorConnectOptions(ReactorConnectOptions connectOpts, ConsumerProviderSessionOptions opts, TestReactorComponent component, int[] portNumbers)
    {
        connectOpts.SetReconnectAttempLimit(opts.ReconnectAttemptLimit);
        connectOpts.SetReconnectMinDelay((int)opts.ReconnectMinDelay.TotalMilliseconds);
        connectOpts.SetReconnectMaxDelay((int)opts.ReconnectMaxDelay.TotalMilliseconds);

        opts.PreferredHostOptions?.Copy(connectOpts.PreferredHostOptions);

        foreach (var portNumber in portNumbers)
        {
            ReactorConnectInfo connectInfo = new()
            {
                ConnectOptions = {
                    MajorVersion = Codec.Codec.MajorVersion(),
                    MinorVersion = Codec.Codec.MinorVersion(),
                    ConnectionType = opts.ConnectionType,
                    UserSpecObject = component,
                    CompressionType = opts.CompressionType,
                    NumInputBuffers = 20,
                    UnifiedNetworkInfo = {
                        Address = "localhost",
                        ServiceName = portNumber.ToString(),
                    },
                },
            };

            connectInfo.ConnectOptions.PingTimeout = (int)opts.PingTimeout.TotalSeconds;
            connectInfo.SetInitTimeout((int)opts.ConsumerChannelInitTimeout.TotalSeconds);

            if (opts.SysSendBufSize > 0)
                connectInfo.ConnectOptions.SysSendBufSize = opts.SysSendBufSize;

            if (opts.SysRecvBufSize > 0)
                connectInfo.ConnectOptions.SysRecvBufSize = opts.SysRecvBufSize;

            connectOpts.ConnectionList.Add(connectInfo);
        }
    }

    private void ConnectChecked(ReactorConnectOptions connectOpts, ReactorRole reactorRole)
    {
        ReactorReturnCode connectCode = m_Reactor.Connect(connectOpts, reactorRole, out var connectError);
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
        //Assert.Null(component.ReactorChannel);

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
                        AssertSuccess((out ReactorErrorInfo err) => m_Reactor.Accept(component.Server, acceptOpts, component.ReactorRole, out err));
                        return;
                    }
                }
            }
            catch (Exception e) when (e is SocketException || e is ObjectDisposedException)
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
    [Obsolete("Use TestReactorSession instead")]
    public static TestReactorSession OpenSession(Consumer consumer, Provider provider, ConsumerProviderSessionOptions opts)
        => OpenSession(consumer, provider, opts, false);

    /// <summary>
    /// Connects a Consumer and Provider component to each other.
    /// </summary>
    [Obsolete("Use TestReactorSession instead")]
    public static TestReactorSession OpenSession(Consumer consumer, Provider provider, ConsumerProviderSessionOptions opts, bool recoveringChannel)
        => TestReactorSession.OpenSession(consumer, provider, opts, recoveringChannel);

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

    public void Dispose() => Close();
}
