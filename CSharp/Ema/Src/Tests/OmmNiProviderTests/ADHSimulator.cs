/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2023 Refinitiv. All rights reserved.              --
 *|-----------------------------------------------------------------------------
 */

using System.Collections.Generic;
using System.Net.Sockets;
using System.Threading;

using Xunit.Abstractions;

using LSEG.Ema.Access.Tests.OmmConsumerTests;
using LSEG.Eta.Codec;
using LSEG.Eta.Transports;
using LSEG.Eta.ValueAdd.Rdm;
using LSEG.Eta.ValueAdd.Reactor;
using System.Dynamic;
using System;

namespace LSEG.Ema.Access.Tests.OmmNiProviderTests
{
    internal class ADHSimulator : IProviderCallback
    {
        OmmConsumerTests.LoginHandler m_LoginHandler;
        internal OmmConsumerTests.DictionaryHandler DictionaryHandler { get; private set; }

        public OmmConsumerTests.MarketItemHandler m_MarketItemHandler;

        private Thread? dispatchingThread;

        private CancellationTokenSource providerCts = new CancellationTokenSource();
        private CancellationToken token;

        private readonly List<Socket> RegisterSocketList = new List<Socket>();

        ReactorAcceptOptions m_AcceptOptions = new ReactorAcceptOptions();

        ProviderRole m_ProviderRole = new ProviderRole();

        public static DataDictionary DataDictionary = new DataDictionary();

        private Eta.ValueAdd.Rdm.DirectoryMsg m_DirectoryCache;

        private static void LoadDictionary()
        {
            if (DataDictionary.LoadEnumTypeDictionary("../../../ComplexTypeTests/enumtype.def", out _) < 0)
            {
                Assert.Fail("Unable to load enum dictionary.");
            }

            if (DataDictionary.LoadFieldDictionary("../../../ComplexTypeTests/RDMFieldDictionary", out _) < 0)
            {
                Assert.Fail("Unable to load enum dictionary.");
            }
        }

        static ADHSimulator()
        {
            LoadDictionary();
        }

        static ReaderWriterLockSlim PortLock = new ReaderWriterLockSlim(LockRecursionPolicy.NoRecursion);

        /// <summary>
        /// Queue of events received from calling dispatch.
        /// </summary>
        internal Queue<TestReactorEvent> EventQueue { get; private set; } = new Queue<TestReactorEvent>();


        /// <summary>
        /// Reactor channel associated with this component, if connected.
        /// </summary>
        protected ReactorChannel? m_ReactorChannel;

        /// <summary>
        /// Whether the reactor channel associated with this component is up.
        /// </summary>
        protected bool m_ReactorChannelIsUp;

        /// <summary>
        /// ReactorRole associated with this component.
        /// </summary>
        public ProviderRole? ReactorRole { get; set; }

        /// <summary>
        /// Server associated with this component, if any.
        /// </summary>
        public IServer? Server { get; set; }

        public Reactor? m_Reactor { get; private set; }

        /// <summary>
        /// Returns the port of the component's server.
        /// </summary>
        public int ServerPort
        {
            get
            {
                Assert.NotNull(Server);
                return Server!.PortNumber;
            }
        }

        public ProviderSessionOptions ProviderSessionOptions { get; private set; }

        public DirectoryRefresh DirectoryCache { get => m_DirectoryCache.DirectoryRefresh!; }

        private int bindPort;

        /// <summary>
        /// A port to use when binding servers. Incremented with each bind.
        /// </summary>
        static int m_PortToBind = 16005;

        ITestOutputHelper m_Output;

        internal ADHSimulator(ProviderSessionOptions providerSessionOptions, ITestOutputHelper output)
        {
            m_LoginHandler = new OmmConsumerTests.LoginHandler(providerSessionOptions);

            ProviderSessionOptions = providerSessionOptions;

            DictionaryHandler = new OmmConsumerTests.DictionaryHandler(providerSessionOptions);

            m_MarketItemHandler = new OmmConsumerTests.MarketItemHandler(m_LoginHandler, providerSessionOptions);

            m_AcceptOptions.AcceptOptions.UserSpecObject = this;
            m_ProviderRole.ChannelEventCallback = this;
            m_ProviderRole.DefaultMsgCallback = this;
            m_ProviderRole.DictionaryMsgCallback = this;
            m_ProviderRole.DirectoryMsgCallback = this;
            m_ProviderRole.LoginMsgCallback = this;

            bindPort = GetServerPort();

            m_Output = output;

            m_DirectoryCache = new DirectoryMsg();
            m_DirectoryCache.DirectoryMsgType = DirectoryMsgType.REFRESH;
        }

        private static int GetServerPort()
        {
            PortLock.EnterWriteLock();
            m_PortToBind++;
            PortLock.ExitWriteLock();
            return m_PortToBind;
        }

        public bool Initialize(ReactorOptions reactorOptions)
        {
            m_Reactor = Reactor.CreateReactor(reactorOptions, out _);

            if (m_Reactor is null)
            {
                return false;
            }

            Bind(ProviderSessionOptions);

            token = providerCts.Token;

            dispatchingThread = new Thread(new ThreadStart(Run));
            dispatchingThread.Name = "NIPROVIDER";
            dispatchingThread.Start();

            return true;
        }

        public void UnInitialize()
        {
            if (dispatchingThread?.IsAlive == true)
            {
                providerCts.Cancel();

                // threadRunning = false;
                dispatchingThread?.Join();
            }
            providerCts.Dispose();

            if (m_ReactorChannel != null)
            {
                m_Reactor?.CloseChannel(m_ReactorChannel, out _);
            }

            m_Reactor?.Shutdown(out _);
        }

        private void Bind(ProviderSessionOptions opts)
        {
            BindOptions bindOpts = new();
            bindOpts.Clear();
            bindOpts.MajorVersion = Codec.MajorVersion();
            bindOpts.MinorVersion = Codec.MinorVersion();
            bindOpts.PingTimeout = (int)opts.PingTimeout.TotalSeconds;
            bindOpts.MinPingTimeout = (int)opts.PingTimeout.TotalSeconds;
            bindOpts.GuaranteedOutputBuffers = opts.NumOfGuaranteedBuffers;

            if (opts.CompressionType != Eta.Transports.CompressionType.NONE)
            {
                bindOpts.ForceCompression = true;
                bindOpts.CompressionType = opts.CompressionType;
            }

            int port = bindPort; // Tries with the initial port from object creation;

            // allow multiple tests to run at the same time, if the port is in use it might mean that
            // another parallel test is using this port, so just try to get another port
            while (Server == null)
            {
                bindOpts.ServiceName = port.ToString();
                Server = Transport.Bind(bindOpts, out _);

                if (Server == null)
                    port = GetServerPort();
            }

            RegisterSocketList.Add(Server.Socket);
        }

        string m_IProviderPort = string.Empty;

        private void Run()
        {
            ReactorDispatchOptions dispatchOptions = new ReactorDispatchOptions();

            while (!token.IsCancellationRequested
                   && m_Reactor is not null)
            {
                if (m_Reactor.Dispatch(dispatchOptions, out var errorInfo) == ReactorReturnCode.FAILURE)
                {
                    Assert.Fail($"Reactor.Dispatch failed with {errorInfo}");
                    break;
                }

                if (Server?.Socket != null && Server!.Socket.Poll(1, SelectMode.SelectRead))
                {
                    if (m_Reactor!.Accept(Server!, m_AcceptOptions, m_ProviderRole, out _) == ReactorReturnCode.FAILURE)
                    {
                        break;
                    }
                }

                string iprovPort = Interlocked.Exchange(ref m_IProviderPort, string.Empty);
                if (iprovPort.Length != 0)
                {
                    // establish connection to the Interactive Provider
                    ReactorConnectInfo connectInfo = new ReactorConnectInfo();
                    connectInfo.ConnectOptions.MajorVersion = Eta.Codec.Codec.MajorVersion();
                    connectInfo.ConnectOptions.MinorVersion = Eta.Codec.Codec.MinorVersion();
                    connectInfo.ConnectOptions.ConnectionType = Eta.Transports.ConnectionType.SOCKET;
                    connectInfo.ConnectOptions.UnifiedNetworkInfo.ServiceName = iprovPort;
                    connectInfo.ConnectOptions.UnifiedNetworkInfo.Address = "localhost";
                    connectInfo.ConnectOptions.UserSpecObject = this;

                    ReactorConnectOptions connectOpts = new ();
                    connectOpts.ConnectionList.Add(connectInfo);

                    ConsumerRole consumerRole = new ConsumerRole()
                    {
                        DefaultMsgCallback = this,
                        ChannelEventCallback = this,
                        LoginMsgCallback = this,
                        DirectoryMsgCallback = this,
                        DictionaryMsgCallback = this
                    };

                    consumerRole.InitDefaultRDMLoginRequest();
                    consumerRole.InitDefaultRDMDirectoryRequest();

                    ReactorReturnCode ret;
                    if ((ret = m_Reactor.Connect(connectOpts, consumerRole, out var connectErrorInfo)) < ReactorReturnCode.SUCCESS)
                    {
                        Console.WriteLine($"Reactor.Connect failed with return code: {ret} error = {connectErrorInfo?.Error.Text}");
                        Assert.True(false);
                    }
                    Assert.False(ret < ReactorReturnCode.SUCCESS);
                }
            }
        }
        
        /// <see cref=IProviderDictionaryTests.DownloadFromADH_Test(OmmIProviderConfig.AdminControlMode)" />
        public void IProviderLaunched(string port)
        {
            // notification for when interactive Provider is initialized and can accept connection from ADH
            Interlocked.Exchange(ref m_IProviderPort, port);
        }

        public ReactorCallbackReturnCode ReactorChannelEventCallback(ReactorChannelEvent evt)
        {
            switch (evt.EventType)
            {
                case ReactorChannelEventType.CHANNEL_OPENED:
                    Assert.False(m_ReactorChannelIsUp);
                    break;
                case ReactorChannelEventType.CHANNEL_UP:
                    m_ReactorChannelIsUp = true;
                    break;
                case ReactorChannelEventType.CHANNEL_DOWN:
                case ReactorChannelEventType.CHANNEL_DOWN_RECONNECTING:
                    if (m_ReactorChannelIsUp)
                    {
                        /* If channel was up, the selectableChannel should be present. */
                        Assert.NotNull(m_ReactorChannel!.Socket);

                        /* Cancel key. */
                        if (m_ReactorChannel!.Socket != null)
                        {
                            RegisterSocketList.Remove(m_ReactorChannel!.Socket);
                        }
                    }

                    m_ReactorChannelIsUp = false;

                    m_ReactorChannel = null;

                    break;
                case ReactorChannelEventType.CHANNEL_READY:
                case ReactorChannelEventType.FD_CHANGE:
                case ReactorChannelEventType.WARNING:
                    Assert.NotNull(m_ReactorChannel);
                    break;
                default:
                    EmaTestUtils.Fail("Unhandled ReactorChannelEventType.");
                    break;

            }

            if (m_ReactorChannel != null)
                Assert.Equal(m_ReactorChannel, evt.ReactorChannel);
            else
                m_ReactorChannel = evt.ReactorChannel;

            EventQueue.Enqueue(new TestReactorEvent(TestReactorEventType.CHANNEL_EVENT, evt));
            return ReactorCallbackReturnCode.SUCCESS;
        }

        public ReactorCallbackReturnCode DefaultMsgCallback(ReactorMsgEvent msgEvent)
        {
            IMsg msg = msgEvent.Msg!;
            ReactorChannel reactorChannel = msgEvent.ReactorChannel!;

            if (ProviderSessionOptions.SendMarketDataItemReject)
            {
                if (m_MarketItemHandler.SendItemRequestReject(reactorChannel, msg.StreamId, msg.DomainType, ItemRejectReason.STREAM_ALREADY_IN_USE, false) != ReactorReturnCode.SUCCESS)
                {
                    return ReactorCallbackReturnCode.FAILURE;
                }

                return ReactorCallbackReturnCode.SUCCESS;
            }
            else
            {
                EventQueue.Enqueue(new TestReactorEvent(TestReactorEventType.MSG, msgEvent));
            }

            return ReactorCallbackReturnCode.SUCCESS;
        }

        public ReactorCallbackReturnCode RdmLoginMsgCallback(RDMLoginMsgEvent loginEvent)
        {
            EventQueue.Enqueue(new TestReactorEvent(TestReactorEventType.LOGIN_MSG, loginEvent));

            Assert.Equal(ReactorReturnCode.SUCCESS, m_LoginHandler.HandleLoginMsgEvent(loginEvent));

            return ReactorCallbackReturnCode.SUCCESS;
        }

        public ReactorCallbackReturnCode RdmDirectoryMsgCallback(RDMDirectoryMsgEvent directoryMsgEvent)
        {
            switch (directoryMsgEvent.DirectoryMsg!.DirectoryMsgType)
            {
                case Eta.ValueAdd.Rdm.DirectoryMsgType.REFRESH:
                    {
                        m_DirectoryCache.DirectoryMsgType = Eta.ValueAdd.Rdm.DirectoryMsgType.REFRESH;

                        directoryMsgEvent.DirectoryMsg.DirectoryRefresh!.Copy(m_DirectoryCache.DirectoryRefresh!);

                        break;
                    }
                case Eta.ValueAdd.Rdm.DirectoryMsgType.UPDATE:
                    {
                        break;
                    }
                case Eta.ValueAdd.Rdm.DirectoryMsgType.STATUS:
                    {
                        break;
                    }
            }

            return ReactorCallbackReturnCode.SUCCESS;
        }

        public ReactorCallbackReturnCode RdmDictionaryMsgCallback(RDMDictionaryMsgEvent msgEvent)
        {
            EventQueue.Enqueue(new TestReactorEvent(TestReactorEventType.DICTIONARY_MSG, msgEvent));

            if (ProviderSessionOptions.SendDictionaryResp)
            {
                Assert.Equal(ReactorReturnCode.SUCCESS, DictionaryHandler?.HandleDictionaryMsgEvent(msgEvent));
            }

            return ReactorCallbackReturnCode.SUCCESS;
        }
    }
}
