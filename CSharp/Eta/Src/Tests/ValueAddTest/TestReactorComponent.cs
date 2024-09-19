/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2023 LSEG. All rights reserved.     
 *|-----------------------------------------------------------------------------
 */

using Xunit;

using LSEG.Eta.Codec;
using LSEG.Eta.Transports;
using LSEG.Eta.ValueAdd.Rdm;
using LSEG.Eta.ValueAdd.Reactor;

namespace LSEG.Eta.ValuedAdd.Tests;

/** A component represents a consumer, provider, etc. on the network (note the Consumer and Provider subclasses). */
public abstract class TestReactorComponent
{
    /// <summary>
    /// Reactor channel associated with this component, if connected.
    /// </summary>
    protected ReactorChannel m_ReactorChannel;

    /// <summary>
    /// Whether the reactor channel associated with this component is up.
    /// </summary>
    protected bool m_ReactorChannelIsUp;

    /// <summary>
    /// ReactorRole associated with this component.
    /// </summary>
    public ReactorRole ReactorRole { get; set; }

    /// <summary>
    /// Server associated with this component, if any.
    /// </summary>
    public IServer Server { get; set; }

    /// <summary>
    /// Returns the port of the component's server.
    /// </summary>
    public int ServerPort
    {
        get
        {
            Assert.NotNull(Server);
            return Server.PortNumber;
        }
    }

    /// <summary>
    /// Reactor associated with this component.
    /// </summary>
    public TestReactor TestReactor { get; set; }

    int m_DefaultSessionLoginStreamId;
    bool m_DefaultSessionLoginStreamIdIsSet;
    int m_DefaultSessionDirectoryStreamId;
    bool m_DefaultSessionDirectoryStreamIdIsSet;

    /// <summary>
    /// A port to use when binding servers. Incremented with each bind.
    /// </summary>
    static int m_PortToBind = 16123;

    /// <summary>
    /// A port to use when binding servers. Incremented with each bind.
    /// </summary>
    static int m_PortToBindForReconnectTest = 16000;

    /** Returns the port the next bind call will use. Useful for testing reconnection
     * to servers that won't be bound until later in a test. */
    static int nextReservedServerPort()
    {
        return m_PortToBindForReconnectTest;
    }

    protected TestReactorComponent(TestReactor testReactor)
    {
        TestReactor = testReactor;
        TestReactor.AddComponent(this);
    }

    public ReactorChannel ReactorChannel
    {
        get => m_ReactorChannel;
        set
        {
            m_ReactorChannel = value;
        }
    }


    public bool IsReactorChannelUp
    {
        get => m_ReactorChannelIsUp;
        set
        {
            m_ReactorChannelIsUp = value;
        }
    }

    /// <summary>
    /// Stores the login stream ID for this component.
    /// </summary>
    ///
    /// <remarks>
    /// Used if a login stream is automatically setup as part of opening a session.
    /// If a login stream was automatically opened as part of opening a session,
    /// returns the ID of that stream for this component.
    /// </remarks>
    ///
    public int DefaultSessionLoginStreamId
    {
        get
        {
            Assert.True(m_DefaultSessionLoginStreamIdIsSet);
            return m_DefaultSessionLoginStreamId;
        }
        set
        {
            m_DefaultSessionLoginStreamIdIsSet = true;
            m_DefaultSessionLoginStreamId = value;
        }
    }

    /// <summary>
    /// If a directory stream was automatically opened as part of
    /// opening a session, returns the ID of that stream for this component.
    /// </summary>
    ///
    /// <remarks>
    /// Stores the directory stream ID for this component. Used if a directory
    /// stream is automatically setup as part of opening a session.
    /// </remarks>
    ///
    public int DefaultSessionDirectoryStreamId
    {
        get
        {
            Assert.True(m_DefaultSessionDirectoryStreamIdIsSet);
            return m_DefaultSessionDirectoryStreamId;
        }

        set
        {
            m_DefaultSessionDirectoryStreamIdIsSet = true;
            m_DefaultSessionDirectoryStreamId = value;
        }
    }

    public void BindForReconnectTest(ConsumerProviderSessionOptions opts)
    {
        BindOptions bindOpts = new();
        bindOpts.Clear();
        bindOpts.MajorVersion = Codec.Codec.MajorVersion();
        bindOpts.MinorVersion = Codec.Codec.MinorVersion();
        bindOpts.ServiceName = (m_PortToBindForReconnectTest++).ToString();
        bindOpts.PingTimeout = (int)opts.PingTimeout.TotalSeconds;
        bindOpts.MinPingTimeout = (int)opts.PingTimeout.TotalSeconds;
        Server = Transport.Bind(bindOpts, out var bindError);
        Assert.True(Server != null,
                    $"bind failed: {bindError.ErrorId} ({bindError.Text})");

        TestReactor.RegisterComponentServer(this);
    }


    public void Bind(ConsumerProviderSessionOptions opts)
    {
        BindOptions bindOpts = new();
        bindOpts.Clear();
        bindOpts.MajorVersion = Codec.Codec.MajorVersion();
        bindOpts.MinorVersion = Codec.Codec.MinorVersion();
        bindOpts.PingTimeout = (int)opts.PingTimeout.TotalSeconds;
        bindOpts.MinPingTimeout = (int)opts.PingTimeout.TotalSeconds;
        bindOpts.GuaranteedOutputBuffers = opts.NumOfGuaranteedBuffers;
        bindOpts.CompressionType = opts.CompressionType;

        // allow multiple tests to run at the same time, if the port is in use it might mean that
        // another parallel test is using this port, so just try to get another port
        while (Server == null)
        {
            bindOpts.ServiceName = (m_PortToBind++).ToString();
            Server = Transport.Bind(bindOpts, out _);
        }

        TestReactor.RegisterComponentServer(this);
    }

    /// <summary>
    /// Sends a Msg to the component's channel.
    /// </summary>
    ///
    public ReactorReturnCode Submit(Msg msg, ReactorSubmitOptions submitOptions)
    {
        return Submit(msg, submitOptions, false);
    }


    public ReactorReturnCode Submit(Msg msg, ReactorSubmitOptions submitOptions, bool expectFailure)
    {
        ReactorReturnCode ret = m_ReactorChannel.Submit(msg, submitOptions, out var errorInfo);

        if (!expectFailure)
        {
            Assert.True(ret >= ReactorReturnCode.SUCCESS,
                $"submit failed: {ret} ({errorInfo?.Location} -- {errorInfo?.Error?.Text})");
        }

        return ret;
    }


    /// <summary>
    /// Sends a Msg to the component's channel, and dispatches to ensure no events are
    /// received and any internal flush events are processed.
    /// </summary>
    ///
    public ReactorReturnCode SubmitAndDispatch(Msg msg, ReactorSubmitOptions submitOptions)
    {
        return SubmitAndDispatch(msg, submitOptions, false);
    }


    /// <summary>
    /// Sends a Msg to the component's channel, and dispatches. Allowing any unprocessed messages left
    /// </summary>
    public ReactorReturnCode SubmitAndDispatch(Msg msg, ReactorSubmitOptions submitOptions, bool expectFailures)
    {
        ReactorReturnCode ret = Submit(msg, submitOptions, expectFailures);
        TestReactor.Dispatch(expectFailures ? -1 : 0);
        return ret;
    }


    /// <summary>
    /// Sends an RDM message to the component's channel.
    /// </summary>
    public ReactorReturnCode Submit(MsgBase msg, ReactorSubmitOptions submitOptions)
    {
        ReactorReturnCode ret = m_ReactorChannel.Submit(msg, submitOptions, out var errorInfo);

        Assert.True(ret >= ReactorReturnCode.SUCCESS,
            $"submit failed: {ret} ({errorInfo?.Location} -- {errorInfo?.Error?.Text})");

        return ret;
    }


    /** Sends an RDM message to the component's channel, and dispatches to ensure no
     * events are received and any internal flush events are processed. */
    public ReactorReturnCode SubmitAndDispatch(MsgBase msg, ReactorSubmitOptions submitOptions)
    {
        ReactorReturnCode ret = Submit(msg, submitOptions);
        TestReactor.Dispatch(0);
        return ret;
    }

    /// <summary>
    /// Disconnect a consumer and provider component and clean them up.
    /// </summary>
    public static void CloseSession(Consumer consumer, Provider provider)
    {
        CloseSession(consumer, provider, false);
    }


    /// <summary>
    /// Disconnect a consumer and provider component and clean them up.
    /// Do additional checks to not fail on dirty client disonnection.
    /// </summary>
    public static void CloseSession(Consumer consumer, Provider provider, bool expectConsumerFailure)
    {
        /* Make sure there's nothing left in the dispatch queue. */
        consumer.TestReactor.Dispatch(0, expectConsumerFailure);
        provider.TestReactor.Dispatch(0);

        consumer.Close();
        provider.Close();
    }

    /// <summary>
    /// Closes the component's channel.
    /// </summary>
    internal void CloseChannel()
    {
        if (ReactorChannelState.CLOSED != m_ReactorChannel.State)
        {
            Assert.Equal(ReactorReturnCode.SUCCESS, m_ReactorChannel.Close(out _));
        }
        m_ReactorChannelIsUp = false;
        m_ReactorChannel = null;
    }


    /// <summary>
    /// Close a component and remove it from its associated TestReactor.
    /// Closes any associated server and reactor channel.
    /// </summary>
    public void Close()
    {
        Assert.NotNull(TestReactor);
        if (Server != null)
        {
            Assert.Equal(TransportReturnCode.SUCCESS, Server.Close(out _));
            Server = null;
        }

        if (m_ReactorChannel != null)
            CloseChannel();

        TestReactor.RemoveComponent(this);
        TestReactor = null;
    }

    public ReactorReturnCode GetChannelInfo(ReactorChannelInfo reactorChannelInfo)
    {
        if(m_ReactorChannelIsUp && m_ReactorChannel != null)
        {
            return m_ReactorChannel.Info(reactorChannelInfo, out _);
        }
        else
        {
            return ReactorReturnCode.FAILURE;
        }
    }
}
