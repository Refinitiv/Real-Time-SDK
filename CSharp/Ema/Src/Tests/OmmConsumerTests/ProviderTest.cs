/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2023-2025 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

using System.Collections.Generic;
using System.Threading;

using LSEG.Eta.Codec;
using LSEG.Eta.Rdm;
using LSEG.Eta.ValueAdd.Rdm;
using LSEG.Eta.ValueAdd.Reactor;
using System;

using LSEG.Eta.Transports;
using System.Net.Sockets;
using Xunit.Abstractions;

namespace LSEG.Ema.Access.Tests.OmmConsumerTests
{
    internal class ProviderTest : IProviderCallback
    {
        private Thread? dispatchingThread;

        private CancellationTokenSource providerCts = new CancellationTokenSource();
        private CancellationToken token;

        private readonly List<Socket> RegisterSocketList = new List<Socket>();

        ReactorAcceptOptions m_AcceptOptions = new ReactorAcceptOptions();

        ProviderRole m_ProviderRole = new ProviderRole();

        LoginHandler m_LoginHandler;

        DirectoryHandler m_DirectoryHandler;
        DictionaryHandler m_DictionaryHandler;

        public MarketItemHandler MarketItemHandler;

        public static DataDictionary DataDictionary = new DataDictionary();

        private static void LoadDictionary()
        {
            if (DataDictionary.LoadEnumTypeDictionary("../../../ComplexTypeTests/enumtype.def", out _) < 0)
            {
                Console.WriteLine($"Unable to load enum dictionary.");
                Assert.True(false);
            }

            if (DataDictionary.LoadFieldDictionary("../../../ComplexTypeTests/RDMFieldDictionary", out _) < 0)
            {
                Console.WriteLine($"Unable to load enum dictionary.");
                Assert.True(false);
            }
        }

        /// <summary>
        /// Queue of events received from calling dispatch.
        /// </summary>
        private Queue<TestReactorEvent> m_EventQueue = new Queue<TestReactorEvent>();

        /* A default service that can be used when setting up a connection. */
        public static Service DefaultService { get; private set; }

        /* A second default service that can be used when setting up a connection. */
        public static Service DefaultService2 { get; private set; }

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

        private int bindPort;

        /// <summary>
        /// A port to use when binding servers. Incremented with each bind.
        /// </summary>
        static int m_PortToBind = 19123;

        static ReaderWriterLockSlim PortLock = new ReaderWriterLockSlim(LockRecursionPolicy.NoRecursion);

        private DecodeIterator m_DecodeIterator = new DecodeIterator();

        ITestOutputHelper m_Output;

        static ProviderTest()
        {
            LoadDictionary();

            DefaultService = new();
            DefaultService.Clear();
            DefaultService.HasInfo = true;
            DefaultService.HasState = true;
            DefaultService.ServiceId = 1;

            DefaultService.Info.ServiceName.Data("DEFAULT_SERVICE");

            DefaultService.Info.CapabilitiesList = new List<long>()
            {
                (long)DomainType.DICTIONARY, (long)DomainType.MARKET_PRICE,
                (long)DomainType.MARKET_BY_ORDER, (long)DomainType.SYMBOL_LIST,
                (long)DomainType.SYSTEM
            };

            DefaultService.Info.HasQos = true;
            DefaultService.Info.QosList = new List<Qos>();

            DefaultService.Info.HasDictionariesUsed = true;
            DefaultService.Info.DictionariesUsedList.Add("RWFFld");
            DefaultService.Info.DictionariesUsedList.Add("RWFEnum");

            DefaultService.Info.HasDictionariesProvided = true;
            DefaultService.Info.DictionariesProvidedList.Add("RWFFld");
            DefaultService.Info.DictionariesProvidedList.Add("RWFEnum");

            Qos qos = new();
            qos.Clear();
            qos.Timeliness(QosTimeliness.DELAYED);
            qos.Rate(QosRates.JIT_CONFLATED);
            qos.IsDynamic = false;
            qos.TimeInfo(0);
            qos.RateInfo(0);
            DefaultService.Info.QosList.Add(qos);

            qos.Clear();
            qos.Timeliness(QosTimeliness.REALTIME);
            qos.Rate(QosRates.TICK_BY_TICK);
            qos.IsDynamic = false;
            qos.TimeInfo(0);
            qos.RateInfo(0);
            DefaultService.Info.QosList.Add(qos);

            DefaultService.State.HasAcceptingRequests = true;
            DefaultService.State.AcceptingRequests = 1;
            DefaultService.State.ServiceStateVal = 1;

            DefaultService2 = new();
            DefaultService2.Clear();
            DefaultService2.HasInfo = true;
            DefaultService2.HasState = true;
            DefaultService2.ServiceId = 2;

            DefaultService2.Info.ServiceName.Data("DEFAULT_SERVICE2");

            DefaultService2.Info.CapabilitiesList = new List<long>()
            {
                (long)DomainType.DICTIONARY, (long)DomainType.MARKET_PRICE,
                (long)DomainType.MARKET_BY_ORDER, (long)DomainType.SYMBOL_LIST,
                (long)DomainType.SYSTEM
            };

            DefaultService2.Info.HasQos = true;
            DefaultService2.Info.QosList = new List<Qos>();
            qos.Clear();
            qos.Timeliness(QosTimeliness.DELAYED);
            qos.Rate(QosRates.JIT_CONFLATED);
            qos.IsDynamic = false;
            qos.TimeInfo(0);
            qos.RateInfo(0);
            DefaultService2.Info.QosList.Add(qos);

            qos.Clear();
            qos.Timeliness(QosTimeliness.REALTIME);
            qos.Rate(QosRates.TICK_BY_TICK);
            qos.IsDynamic = false;
            qos.TimeInfo(0);
            qos.RateInfo(0);
            DefaultService2.Info.QosList.Add(qos);

            DefaultService2.State.HasAcceptingRequests = true;
            DefaultService2.State.AcceptingRequests = 1;
            DefaultService2.State.ServiceStateVal = 1;
        }

        private static int GetServerPort()
        {
            PortLock.EnterWriteLock();
            m_PortToBind++;
            PortLock.ExitWriteLock();
            return m_PortToBind;
        }

#pragma warning disable CS8618 
        public ProviderTest(ProviderSessionOptions opts, ITestOutputHelper output)
#pragma warning restore CS8618
        {
            ProviderSessionOptions = opts;

            m_LoginHandler = new LoginHandler(this, output);
            m_DirectoryHandler = new DirectoryHandler(this);

            MarketItemHandler = new MarketItemHandler(m_LoginHandler, ProviderSessionOptions);
            if (ProviderSessionOptions.SendDictionaryResp)
            {
                m_DictionaryHandler = new DictionaryHandler(this);
            }

            m_AcceptOptions.AcceptOptions.UserSpecObject = this;
            m_ProviderRole.ChannelEventCallback = this;
            m_ProviderRole.DefaultMsgCallback = this;
            m_ProviderRole.DictionaryMsgCallback = this;
            m_ProviderRole.DirectoryMsgCallback = this;
            m_ProviderRole.LoginMsgCallback = this;

            bindPort = GetServerPort();

            m_Output = output;
        }

        public bool Initialize(ReactorOptions reactorOptions)
        {
            m_Reactor = Reactor.CreateReactor(reactorOptions, out _);

            if(m_Reactor is null)
            {
                return false;
            }

            Bind(ProviderSessionOptions);

            token = providerCts.Token;

            dispatchingThread = new Thread(new ThreadStart(Run));
            dispatchingThread.Name = "PROVIDER";
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

                if(Server == null)
                    port = GetServerPort();
            }

            RegisterSocketList.Add(Server.Socket);
        }

        private void Run()
        {
            ReactorDispatchOptions dispatchOptions = new ReactorDispatchOptions();

            while (!token.IsCancellationRequested
                   && m_Reactor != null)
            {
                if (m_Reactor.Dispatch(dispatchOptions, out var errorInfo) == ReactorReturnCode.FAILURE)
                {
                    Assert.Fail($"Reactor.Dispatch failed with {errorInfo}");
                    //threadRunning = false;
                    break;
                }

                if (Server?.Socket != null && Server!.Socket.Poll(1, SelectMode.SelectRead))
                {
                    if (m_Reactor!.Accept(Server!, m_AcceptOptions, m_ProviderRole, out _) == ReactorReturnCode.FAILURE)
                    {
                        break;
                    }
                }
            }
        }

        public ReactorCallbackReturnCode DefaultMsgCallback(ReactorMsgEvent reactorEvent)
        {
            m_EventQueue.Enqueue(new TestReactorEvent(TestReactorEventType.MSG, reactorEvent));

            IMsg msg = reactorEvent.Msg!;
            ReactorChannel reactorChannel = reactorEvent.ReactorChannel!;

            // clear decode iterator
            m_DecodeIterator.Clear();
            // set buffer and version info
            if (msg.EncodedDataBody.Data() != null)
            {
                m_DecodeIterator.SetBufferAndRWFVersion(msg.EncodedDataBody, reactorChannel.MajorVersion, reactorChannel.MinorVersion);
            }

            if(ProviderSessionOptions.SendMarketDataItemReject)
            {
                if(MarketItemHandler.SendItemRequestReject(reactorChannel, msg.StreamId, msg.DomainType, ItemRejectReason.STREAM_ALREADY_IN_USE, false) != ReactorReturnCode.SUCCESS)
                {
                    return ReactorCallbackReturnCode.FAILURE;
                }

                return ReactorCallbackReturnCode.SUCCESS;
            }

            switch (msg.DomainType)
            {
                case (int)DomainType.MARKET_PRICE:
                    if (MarketItemHandler.ProcessRequest(reactorChannel, (Eta.Codec.Msg)msg, m_DecodeIterator) != ReactorReturnCode.SUCCESS)
                    {
                        return ReactorCallbackReturnCode.FAILURE;
                    }
                    break;
                default:
                    switch (msg.MsgClass)
                    {
                        case MsgClasses.REQUEST:
                            if (MarketItemHandler.SendItemRequestReject(reactorChannel, msg.StreamId, msg.DomainType, ItemRejectReason.DOMAIN_NOT_SUPPORTED, false) != ReactorReturnCode.SUCCESS)
                                return ReactorCallbackReturnCode.FAILURE;
                            break;
                        case MsgClasses.CLOSE:
                            m_Output.WriteLine($"[ProviderTest] Received close message with streamId={msg.StreamId} and unsupported Domain '{msg.DomainType}'");
                            break;
                        case MsgClasses.GENERIC:
                            if (MarketItemHandler.ProcessGenericMsg(reactorChannel, (Eta.Codec.Msg)msg, m_DecodeIterator) != ReactorReturnCode.SUCCESS)
                            {
                                return ReactorCallbackReturnCode.FAILURE;
                            }
                            break;
                        case MsgClasses.POST:

                            break;
                        default:
                            m_Output.WriteLine($"[ProviderTest] Received unhandled Msg Class: {MsgClasses.ToString(msg.MsgClass)} with streamId={msg.StreamId} and unsupported Domain '{msg.DomainType}'");
                            break;
                    }
                    break;
            }


            return ReactorCallbackReturnCode.SUCCESS;
        }

        public ReactorCallbackReturnCode RdmDictionaryMsgCallback(RDMDictionaryMsgEvent dictionaryMsgEvent)
        {
            m_EventQueue.Enqueue(new TestReactorEvent(TestReactorEventType.DICTIONARY_MSG, dictionaryMsgEvent));

            if (ProviderSessionOptions.SendDictionaryResp)
            {
                Assert.Equal(ReactorReturnCode.SUCCESS, m_DictionaryHandler.HandleDictionaryMsgEvent(dictionaryMsgEvent));
            }

            return ReactorCallbackReturnCode.SUCCESS;
        }

        public ReactorCallbackReturnCode RdmDirectoryMsgCallback(RDMDirectoryMsgEvent directoryMsgEvent)
        {
            m_EventQueue.Enqueue(new TestReactorEvent(TestReactorEventType.DIRECTORY_MSG, directoryMsgEvent));

            if (ProviderSessionOptions.SendDirectoryResp)
            {
                Assert.Equal(ReactorReturnCode.SUCCESS, m_DirectoryHandler.HandleDirectoryMsgEvent(directoryMsgEvent));
            }

            return ReactorCallbackReturnCode.SUCCESS;
        }

        public ReactorCallbackReturnCode RdmLoginMsgCallback(RDMLoginMsgEvent loginEvent)
        {
            m_EventQueue.Enqueue(new TestReactorEvent(TestReactorEventType.LOGIN_MSG, loginEvent));

            Assert.Equal(ReactorReturnCode.SUCCESS,m_LoginHandler.HandleLoginMsgEvent(loginEvent));

            return ReactorCallbackReturnCode.SUCCESS;
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
            {
                Assert.Equal(m_ReactorChannel, evt.ReactorChannel);
            }
            else
                m_ReactorChannel = evt.ReactorChannel;

            m_EventQueue.Enqueue(new TestReactorEvent(TestReactorEventType.CHANNEL_EVENT, evt));
            return ReactorCallbackReturnCode.SUCCESS;
        }
    }
}
