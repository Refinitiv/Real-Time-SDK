/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2022-2023 LSEG. All rights reserved.     
 *|-----------------------------------------------------------------------------
 */

using System;
using System.Collections.Generic;
using System.Net.Sockets;
using System.Threading;

using LSEG.Eta.Common;
using LSEG.Eta.Codec;
using LSEG.Eta.Example.Common;
using LSEG.Eta.PerfTools.Common;
using LSEG.Eta.Rdm;
using LSEG.Eta.Transports;
using LSEG.Eta.ValueAdd.Rdm;
using LSEG.Eta.ValueAdd.Reactor;

using ProviderSession = LSEG.Eta.PerfTools.Common.ProviderSession;

namespace LSEG.Eta.PerfTools.ProvPerf
{
    /// <summary>
    /// Interactive provider implementation of the provider thread.
    /// Handles accepting of new channels, processing of incoming messages
    /// and sending of message bursts.
    /// </summary>
    public class IProviderThread : ProviderThread, IReactorChannelEventCallback,
        IDefaultMsgCallback, IRDMLoginMsgCallback, IDirectoryMsgCallback, IDictionaryMsgCallback
    {
        private const string applicationName = "ProvPerf";
        private const string applicationId = "256";

        private IProvDirectoryProvider m_DirectoryProvider;     // Source directory requests handler
        private LoginProvider m_LoginProvider;                  // Login requests handler
        private DictionaryProvider m_DictionaryProvider;        // Dictionary requests handler
        private ItemRequestHandler m_ItemRequestHandler;        // Iterm requests handler
        private DecodeIterator m_DecodeIter;                    // Decode iterator

        private ChannelInfo m_ChannelInfo;                      // Active channel information
        private ChannelHandler m_ChannelHandler;                // Channel handler.

        private Msg m_tmpMsg;

        public volatile int ConnectionCount;                    //Number of client sessions currently connected.

        /// <summary>
        /// Use the VA Reactor instead of the ETA Channel for sending and receiving
        /// </summary>
        private Reactor? m_Reactor;

        /// <summary>
        /// Use the VA Reactor instead of the ETA Channel for sending and receiving
        /// </summary>
        private ProviderRole m_ProviderRole;

        /// <summary>
        /// Use the VA Reactor instead of the ETA Channel for sending and receiving
        /// </summary>
        private ReactorOptions m_ReactorOptions;

        /// <summary>
        /// Use the VA Reactor instead of the ETA Channel for sending and receiving
        /// </summary>
        private ReactorAcceptOptions m_AcceptOptions;

        /// <summary>
        /// Use the VA Reactor instead of the ETA Channel for sending and receiving
        /// </summary>
        private ReactorDispatchOptions m_DispatchOptions;

        /// <summary>
        /// Use the VA Reactor instead of the ETA Channel for sending and receiving
        /// </summary>
        private ReactorChannelInfo m_ReactorChannnelInfo;

        /// <summary>
        /// Use the VA Reactor instead of the ETA Channel for sending and receiving
        /// </summary>
        private bool m_ReactorInitialized;

        /// <summary>
        /// Maps connected client socket to the corresponding reactor channel to find the
        /// channel depending on "active" socket after Select call.
        /// </summary>
        private Dictionary<Socket, ReactorChannel> m_SocketChannelMap = new Dictionary<Socket, ReactorChannel>();

        private ReactorChannel? m_CallbackFailedChannel;

        /// <summary>
        /// Instantiates a new i provider thread.
        /// </summary>
        /// <param name="xmlMsgData">the xml msg data</param>
        public IProviderThread(XmlMsgData xmlMsgData) : base(xmlMsgData)
        {
            m_DecodeIter = new DecodeIterator();
            m_tmpMsg = new Msg();
            m_LoginProvider = new LoginProvider();
            m_DirectoryProvider = new IProvDirectoryProvider();
            m_DictionaryProvider = new DictionaryProvider();
            m_ChannelInfo = new ChannelInfo();
            m_ItemRequestHandler = new ItemRequestHandler();
            m_ChannelHandler = new ChannelHandler(this);
            m_ProviderRole = new ProviderRole();
            m_ReactorOptions = new ReactorOptions();
            m_AcceptOptions = new ReactorAcceptOptions();
            m_DispatchOptions = new ReactorDispatchOptions();
            m_ReactorChannnelInfo = new ReactorChannelInfo();

            InitTimeFunctions();
        }

        /// <summary>
        /// Handles newly accepted client channel.
        /// </summary>
        /// <param name="channel">the accepted channel</param>
        public void AcceptNewChannel(IChannel channel)
        {
            ProviderSession provSession = new ProviderSession(m_XmlMsgData, m_ItemEncoder);
            ++ConnectionCount;
            m_ChannelHandler.AddChannel(channel, provSession, true);
        }

        /// <summary>
        /// Process the channel active event for the interactive provider application.
        /// This causes some additional channel configuration and a print out of the channel configuration.
        /// </summary>
        /// <param name="clientChannelInfo">the client channel info</param>
        /// <param name="error"><see cref="Error"/> instance that carries error details in case of failure</param>
        /// <returns><see cref="PerfToolsReturnCode"/> indicating the status of the operation</returns>
        public PerfToolsReturnCode ProcessActiveChannel(ClientChannelInfo clientChannelInfo, out Error? error)
        {
            TransportReturnCode transportReturnCode;
            if (ProviderPerfConfig.HighWaterMark > 0)
            {
                transportReturnCode = clientChannelInfo.Channel!.IOCtl(IOCtlCode.HIGH_WATER_MARK, ProviderPerfConfig.HighWaterMark, out error);
                if (transportReturnCode != TransportReturnCode.SUCCESS)
                    return PerfToolsReturnCode.FAILURE;
            }

            transportReturnCode = clientChannelInfo.Channel!.Info(m_ChannelInfo, out error);
            if (transportReturnCode != TransportReturnCode.SUCCESS)
                return PerfToolsReturnCode.FAILURE;

            Console.WriteLine($"Client channel active. Channel Info: {m_ChannelInfo.ToString()}\n");

            int count = m_ChannelInfo.ComponentInfoList == null ? 0 : m_ChannelInfo.ComponentInfoList.Count;
            if (count == 0)
                Console.WriteLine("(No component info)");
            else
            {
                for (int i = 0; i < count; ++i)
                {
                    Console.WriteLine(m_ChannelInfo.ComponentInfoList![i].ComponentVersion);
                    if (i < count - 1)
                        Console.Write(", ");
                }
            }

            Console.WriteLine("\n\n");

            // Check that we can successfully pack, if packing messages
            if (ProviderPerfConfig.TotalBuffersPerPack > 1 && ProviderPerfConfig.PackingBufferLength > m_ChannelInfo.MaxFragmentSize)
            {
                Console.Write($"Error (Channel {clientChannelInfo.Channel}): MaxFragmentSize {m_ChannelInfo.MaxFragmentSize} is too small for packing buffer size {ProviderPerfConfig.PackingBufferLength}\n");
                error = new Error()
                {
                    Text = $"Error (Channel {clientChannelInfo.Channel}): MaxFragmentSize {m_ChannelInfo.MaxFragmentSize} is too small for packing buffer size {ProviderPerfConfig.PackingBufferLength}\n"
                };
                return PerfToolsReturnCode.FAILURE;
            }

            ProviderSession provSession = (ProviderSession)clientChannelInfo.UserSpec!;
            PerfToolsReturnCode perfToolsReturnCode = provSession.PrintEstimatedMsgSizes(out error);
            if (perfToolsReturnCode != PerfToolsReturnCode.SUCCESS)
                return perfToolsReturnCode;

            provSession.TimeActivated = (long)GetTime.GetMicroseconds();

            error = null;
            return PerfToolsReturnCode.SUCCESS;
        }

        /// <summary>
        /// Method called by ChannelHandler when a channel is closed
        /// </summary>
        /// <param name="channelHandler">the channel handler</param>
        /// <param name="clientChannelInfo">the client channel info</param>
        /// <param name="error">error information</param>
        /// <returns><see cref="PerfToolsReturnCode"/> value indicating the status of the operation</returns>
        public PerfToolsReturnCode ProcessInactiveChannel(ChannelHandler channelHandler, ClientChannelInfo clientChannelInfo, Error error)
        {
            long inactiveTime = (long)GetTime.GetMicroseconds();
            channelHandler.ProviderThread.ProvThreadInfo!.Stats.InactiveTime = inactiveTime;
            Console.WriteLine($"ProcessInactiveChannel ({inactiveTime})");

            m_ChannelHandler.ChannelLock.EnterWriteLock();
            --ConnectionCount;
            m_ChannelHandler.ChannelLock.ExitWriteLock();

            if (ProviderPerfConfig.UseReactor) // use ETA VA Reactor
            {
                m_SocketChannelMap.Remove(clientChannelInfo.Channel!.Socket);
            }

            Console.WriteLine($"Channel Closed: {error?.Text}");

            ProviderSession? provSession = (ProviderSession?)clientChannelInfo.UserSpec;
            provSession?.Cleanup();

            return PerfToolsReturnCode.SUCCESS;
        }

        /// <summary>
        /// Method called by ChannelHandler when ETA read() returns a buffer
        /// </summary>
        /// <param name="channelHandler">the channel handler</param>
        /// <param name="clientChannelInfo">the client channel info</param>
        /// <param name="msgBuf">the msg buffer</param>
        /// <param name="error"><see cref="Error"/> instance with error details in case of failure</param>
        /// <returns><see cref="PerfToolsReturnCode"/> value indicating the status of the operation</returns>
        public PerfToolsReturnCode ProcessMsg(ChannelHandler channelHandler, ClientChannelInfo clientChannelInfo, ITransportBuffer msgBuf, out Error? error)
        {
            ProviderThread providerThread = channelHandler.ProviderThread;
            IChannel channel = clientChannelInfo.Channel!;
            CodecReturnCode codecReturnCode;

            // clear decode iterator
            m_DecodeIter.Clear();
            m_tmpMsg.Clear();

            codecReturnCode = m_DecodeIter.SetBufferAndRWFVersion(msgBuf, channel.MajorVersion, channel.MinorVersion);

            if (codecReturnCode != CodecReturnCode.SUCCESS)
            {
                Console.Error.WriteLine($"DecodeIterator.SetBufferAndRWFVersion() failed with return code:  {codecReturnCode.GetAsString()}");
                error = new Error()
                {
                    Text = $"DecodeIterator.SetBufferAndRWFVersion() failed with return code:  {codecReturnCode.GetAsString()}"
                };
                return PerfToolsReturnCode.FAILURE;
            }

            codecReturnCode = m_tmpMsg.Decode(m_DecodeIter);
            if (codecReturnCode != CodecReturnCode.SUCCESS)
            {
                Console.Error.WriteLine($"Msg.Decode() failed with return code:  {codecReturnCode.GetAsString()}");
                error = new Error()
                {
                    Text = $"Msg.Decode() failed with return code: {codecReturnCode.GetAsString()}"
                };
                return PerfToolsReturnCode.FAILURE;
            }

            TransportReturnCode transportReturnCode;
            switch (m_tmpMsg.DomainType)
            {
                case (int)DomainType.LOGIN:
                    transportReturnCode = m_LoginProvider.ProcessMsg(channelHandler, clientChannelInfo, m_tmpMsg, m_DecodeIter, out error);
                    break;
                case (int)DomainType.SOURCE:
                    transportReturnCode = m_DirectoryProvider.ProcessMsg(channelHandler, clientChannelInfo, m_tmpMsg, m_DecodeIter, out error);
                    break;
                case (int)DomainType.DICTIONARY:
                    transportReturnCode = m_DictionaryProvider.ProcessMsg(channelHandler, clientChannelInfo, m_tmpMsg, m_DecodeIter, out error);
                    break;
                case (int)DomainType.MARKET_PRICE:
                    if (m_XmlMsgData.UpdateCount > 0)
                        transportReturnCode = m_ItemRequestHandler.ProcessMsg(providerThread, (ProviderSession)clientChannelInfo.UserSpec!, m_tmpMsg, m_DirectoryProvider.OpenLimit, m_DirectoryProvider.ServiceId, m_DirectoryProvider.Qos, m_DecodeIter, out error);
                    else
                        transportReturnCode = m_ItemRequestHandler.SendRequestReject(providerThread, (ProviderSession)clientChannelInfo.UserSpec!, m_tmpMsg, ItemRejectReason.DOMAIN_NOT_SUPPORTED, out error);
                    break;
                default:
                    transportReturnCode = m_ItemRequestHandler.SendRequestReject(providerThread, (ProviderSession)clientChannelInfo.UserSpec!, m_tmpMsg, ItemRejectReason.DOMAIN_NOT_SUPPORTED, out error);
                    break;
            }

            if (transportReturnCode > TransportReturnCode.SUCCESS)
            {
                // The method sent a message and indicated that we need to flush.
                m_ChannelHandler.RequestFlush(clientChannelInfo);
            }

            return transportReturnCode < TransportReturnCode.SUCCESS ? PerfToolsReturnCode.FAILURE : PerfToolsReturnCode.SUCCESS;
        }

        /// <summary>
        /// Interactive provider thread's run method
        /// </summary>
        public override void Run()
        {
            // create reactor
            if (ProviderPerfConfig.UseReactor) // use ETA VA Reactor
            {
                m_ReactorOptions.Clear();
                Reactor? reactor;
                if ((reactor = Reactor.CreateReactor(m_ReactorOptions, out var errorInfo)) == null)
                {
                    Console.Error.WriteLine("Reactor creation failed: {0}", errorInfo?.Error.Text);
                    Environment.Exit((int)ReactorReturnCode.FAILURE);
                }
                else
                {
                    m_Reactor = reactor;
                }
            }

            m_DirectoryProvider.ServiceName = ProviderPerfConfig.ServiceName;
            m_DirectoryProvider.ServiceId = ProviderPerfConfig.ServiceId;
            m_DirectoryProvider.OpenLimit = ProviderPerfConfig.OpenLimit;
            m_DirectoryProvider.InitService(m_XmlMsgData);

            m_LoginProvider.InitDefaultPosition();
            m_LoginProvider.ApplicationId = applicationId;
            m_LoginProvider.ApplicationName = applicationName;

            if (!m_DictionaryProvider.LoadDictionary(out m_Error!))
            {
                Console.WriteLine($"Failed loading dictionary, error: {m_Error?.Text}");
                Environment.Exit((int)PerfToolsReturnCode.FAILURE);
            }

            ProvThreadInfo!.Dictionary = m_DictionaryProvider.Dictionary;

            m_ReactorInitialized = true;

            // Determine update rates on per-tick basis
            double nextTickTime = InitNextTickTime!();

            List<Socket> readSocketList = new List<Socket>();

            // this is the main loop
            while (!Shutdown)
            {
                if (!ProviderPerfConfig.UseReactor) // use ETA Channel
                {
                    if (nextTickTime <= CurrentTime!())
                    {
                        nextTickTime = NextTickTime!(nextTickTime);
                        SendMsgBurst(nextTickTime);
                        m_ChannelHandler.ProcessNewChannels();
                    }
                    m_ChannelHandler.ReadChannels(nextTickTime, out m_Error!);
                    if (m_Error is not null && !string.IsNullOrEmpty(m_Error.Text))
                    {
                        Console.WriteLine($"Error while handling channels: {m_Error?.Text}");
                    }
                    m_ChannelHandler.CheckPings();
                }
                else // use ETA VA Reactor
                {
                    if (nextTickTime <= CurrentTime!())
                    {
                        nextTickTime = NextTickTime!(nextTickTime);
                        SendMsgBurst(nextTickTime);
                    }

                    readSocketList.Add(m_Reactor!.EventSocket!);
                    readSocketList.AddRange(m_SocketChannelMap.Keys);

                    // set select time
                    try
                    {
                        // number of microseconds to wait for socket events before next tick
                        int selTime = (int)SelectTime!(nextTickTime);

                        Socket.Select(readSocketList, null, null, selTime);
                    }
                    catch (Exception e1)
                    {
                        Console.Error.WriteLine("Error while waiting for socket events: " + e1.Message);
                        Environment.Exit((int)ReactorReturnCode.FAILURE);
                    }

                    // nothing to read or write
                    if (readSocketList.Count == 0)
                        continue;

                    foreach (var sock in readSocketList)
                    {
                        if (!sock.SafeHandle.IsClosed && sock.Available == 0)
                            continue;

                        ReactorReturnCode ret;
                        ReactorErrorInfo? dispatchError;

                        ReactorChannel? reactorChnl;
                        m_SocketChannelMap.TryGetValue(sock, out reactorChnl);
                        m_DispatchOptions.ReactorChannel = reactorChnl;
                        /* read until no more to read */
                        while ((ret = m_Reactor.Dispatch(m_DispatchOptions, out dispatchError)) > 0 && !Shutdown) { }

                        if (m_CallbackFailedChannel != null)
                        {
                            if (m_SocketChannelMap.ContainsKey(m_CallbackFailedChannel!.Channel!.Socket))
                                m_SocketChannelMap.Remove(m_CallbackFailedChannel!.Channel!.Socket);
                            ProviderSession provSession = (ProviderSession)m_CallbackFailedChannel!.UserSpecObj!;
                            m_ChannelHandler.CloseChannel(provSession.ClientChannelInfo!, m_Error!);
                            m_CallbackFailedChannel = null;
                        }

                        if (ret == ReactorReturnCode.FAILURE)
                        {
                            if (reactorChnl!.State != ReactorChannelState.CLOSED &&
                                reactorChnl.State != ReactorChannelState.DOWN_RECONNECTING)
                            {
                                Console.Error.WriteLine("ReactorChannel dispatch failed: " + dispatchError?.ToString());
                                reactorChnl.Close(out _);
                                Environment.Exit((int)CodecReturnCode.FAILURE);
                            }
                        }
                    }
                    m_DispatchOptions.ReactorChannel = null;
                    readSocketList.Clear();
                }
            }

            ShutdownAck = true;
        }

        /// <summary>
        /// Cleans provider thread
        /// </summary>
        public override void Cleanup()
        {
            m_ChannelHandler.Cleanup();
        }

        /// <summary>
        /// Send refreshes and updates to open channels.
        /// If an operation on channel returns unrecoverable error, the channel is closed.
        /// </summary>
        /// <param name="stopTime">the time until which the messages are being sent</param>
        private void SendMsgBurst(double stopTime)
        {
            foreach (ClientChannelInfo clientChannelInfo in m_ChannelHandler.ActiveChannelList.Values)
            {
                ProviderSession providerSession = (ProviderSession)clientChannelInfo.UserSpec!;

                // The application corrects for ticks that don't finish before the time
                // that the next update burst should start. But don't do this correction
                // for new channels.
                if (providerSession.TimeActivated == 0)
                {
                    continue;
                }

                TransportReturnCode ret = TransportReturnCode.SUCCESS;

                // Send burst of updates
                if (ProviderPerfConfig.UpdatesPerSec != 0 && providerSession.UpdateItemList.Count() != 0)
                {
                    ret = SendUpdateBurst(providerSession, out m_Error!);
                    if (ret > TransportReturnCode.SUCCESS)
                    {
                        // Need to flush
                        m_ChannelHandler.RequestFlush(clientChannelInfo);
                    }
                }

                // Send burst of generic messages
                if (ProviderPerfConfig.GenMsgsPerSec != 0 && providerSession.GenMsgItemList.Count() != 0)
                {
                    ret = SendGenMsgBurst(providerSession, out m_Error!);
                    if (ret > TransportReturnCode.SUCCESS)
                    {
                        // Need to flush
                        m_ChannelHandler.RequestFlush(clientChannelInfo);
                    }
                }

                // Use remaining time in the tick to send refreshes.
                while (ret >= TransportReturnCode.SUCCESS && providerSession.RefreshItemList.Count() != 0 && CurrentTime!() < stopTime)
                {
                    ret = SendRefreshBurst(providerSession, out m_Error!);
                }

                if (ret < TransportReturnCode.SUCCESS)
                {
                    switch (ret)
                    {
                        case TransportReturnCode.NO_BUFFERS:
                            m_ChannelHandler.RequestFlush(clientChannelInfo);
                            break;
                        default:
                            if (Thread.CurrentThread.IsAlive)
                            {
                                Console.WriteLine($"Failure while writing message bursts: {m_Error?.Text} ({ret})\n");
                                if (!ProviderPerfConfig.UseReactor)
                                {
                                    m_ChannelHandler.CloseChannel(clientChannelInfo, new Error()); //Failed to send an update. Remove this client
                                }
                                else
                                {
                                    Console.WriteLine("Channel Closed.");

                                    long inactiveTime = (long)GetTime.GetMicroseconds();
                                    ProvThreadInfo!.Stats.InactiveTime = inactiveTime;

                                    --ConnectionCount;

                                    // unregister selectableChannel from Selector
                                    try
                                    {
                                        m_SocketChannelMap.Remove(clientChannelInfo.ReactorChannel!.Socket!);
                                    }
                                    catch (Exception) { } // channel may be null so ignore

                                    if (providerSession.ClientChannelInfo!.ParentQueue!.Count > 0)
                                    {
                                        providerSession.ClientChannelInfo.ParentQueue.TryRemove(clientChannelInfo.ID, out _);
                                    }

                                    clientChannelInfo.ReactorChannel!.Close(out _); //Failed to send an update. Remove this client
                                }
                            }
                            break;
                    }
                }
                else if (ret > TransportReturnCode.SUCCESS)
                {
                    // need to flush
                    m_ChannelHandler.RequestFlush(clientChannelInfo);
                }
            }
        }

        public ReaderWriterLockSlim HandlerLock()
        {
            return m_ChannelHandler.ChannelLock;
        }

        internal ReactorReturnCode AcceptNewReactorChannel(IServer server, out ReactorErrorInfo? errorInfo)
        {
            while (!m_ReactorInitialized)
            {
                try
                {
                    Thread.Sleep(10);
                }
                catch (Exception e)
                {
                    Console.Error.WriteLine("AcceptNewReactorChannel() Thread.Sleep() exception: " + e.Message);
                }
            }

            // create provider session
            ProviderSession provSession = new ProviderSession(m_XmlMsgData, m_ItemEncoder);
            ClientChannelInfo ccInfo = new ClientChannelInfo();
            provSession.Init(ccInfo);
            ccInfo.UserSpec = provSession;
            provSession.ProviderThread = this;
            ++ConnectionCount;

            // initialize provider role
            m_ProviderRole.ChannelEventCallback = this;
            m_ProviderRole.DefaultMsgCallback = this;
            m_ProviderRole.LoginMsgCallback = this;
            m_ProviderRole.DirectoryMsgCallback = this;
            m_ProviderRole.DictionaryMsgCallback = this;

            Console.WriteLine("Accepting new Reactor connection...");

            m_AcceptOptions.Clear();
            m_AcceptOptions.AcceptOptions.UserSpecObject = provSession;

            return m_Reactor!.Accept(server, m_AcceptOptions, m_ProviderRole, out errorInfo);
        }

        #region Reactor callbacks
        public ReactorCallbackReturnCode ReactorChannelEventCallback(ReactorChannelEvent evt)
        {
            ReactorChannel reactorChannel = evt.ReactorChannel!;
            ProviderSession provSession = (ProviderSession)reactorChannel.UserSpecObj!;

            switch (evt.EventType)
            {
                case ReactorChannelEventType.CHANNEL_UP:
                    {
                        // set the high water mark if configured
                        if (ProviderPerfConfig.HighWaterMark > 0)
                        {
                            if (reactorChannel.IOCtl(IOCtlCode.HIGH_WATER_MARK, ProviderPerfConfig.HighWaterMark, out _) != ReactorReturnCode.SUCCESS)
                            {
                                Console.WriteLine("ReactorChannel.ioctl() failed");
                                reactorChannel.Close(out _);
                            }
                        }

                        // register selector with channel event's reactorChannel
                        m_SocketChannelMap.Add(reactorChannel.Socket!, reactorChannel);

                        /* retrieve and print out channel information */
                        if (reactorChannel.Info(m_ReactorChannnelInfo, out var infoError) != ReactorReturnCode.SUCCESS)
                        {
                            Console.WriteLine("ReactorChannel.Info() failed: " + infoError?.ToString());
                            reactorChannel.Close(out _);
                        }
                        Console.WriteLine("Channel active. " + m_ReactorChannnelInfo.ChannelInfo.ToString());

                        /* Check that we can successfully pack, if packing messages. */
                        if (ProviderPerfConfig.TotalBuffersPerPack > 1
                            && ProviderPerfConfig.PackingBufferLength > m_ReactorChannnelInfo.ChannelInfo.MaxFragmentSize)
                        {
                            Console.Error.WriteLine("Error(Channel {0}): MaxFragmentSize {1} is too small for packing buffer size {2}",
                                    reactorChannel.Socket, m_ReactorChannnelInfo.ChannelInfo.MaxFragmentSize,
                                    ProviderPerfConfig.PackingBufferLength);
                            Environment.Exit(-1);
                        }

                        provSession.ClientChannelInfo!.ReactorChannel = reactorChannel;
                        provSession.ClientChannelInfo.Channel = reactorChannel.Channel;
                        provSession.ClientChannelInfo.ParentQueue = m_ChannelHandler.ActiveChannelList;
                        provSession.ClientChannelInfo.ParentQueue.TryAdd(provSession.ClientChannelInfo.ID, provSession.ClientChannelInfo);

                        provSession.TimeActivated = (long)GetTime.GetMicroseconds();

                        break;
                    }
                case ReactorChannelEventType.CHANNEL_READY:
                    {
                        if (provSession.PrintEstimatedMsgSizes(out var error) != PerfToolsReturnCode.SUCCESS)
                        {
                            Console.WriteLine($"Error: {error?.Text}");
                            reactorChannel.Close(out _);
                        }

                        break;
                    }
                case ReactorChannelEventType.FD_CHANGE:
                    {
                        Console.WriteLine("Channel Change - Old Channel: "
                                + evt.ReactorChannel!.OldSocket + " New Channel: "
                                + evt.ReactorChannel.Socket);

                        // cancel old reactorChannel select
                        m_SocketChannelMap.Remove(evt.ReactorChannel.OldSocket!);

                        // register selector with channel event's new reactorChannel
                        m_SocketChannelMap.Add(evt.ReactorChannel.Socket!, evt.ReactorChannel);

                        break;
                    }
                case ReactorChannelEventType.CHANNEL_DOWN:
                    {
                        Console.WriteLine("Channel Closed.");

                        long inactiveTime = (long)GetTime.GetMicroseconds();
                        ProvThreadInfo!.Stats.InactiveTime = inactiveTime;

                        --ConnectionCount;

                        // unregister selectableChannel from Selector
                        m_SocketChannelMap.Remove(evt.ReactorChannel!.Socket!);

                        if (provSession.ClientChannelInfo!.ReactorChannel != null
                            && provSession.ClientChannelInfo.ParentQueue!.Count > 0)
                        {
                            provSession.ClientChannelInfo.ParentQueue.TryRemove(provSession.ClientChannelInfo.ID, out _);
                        }

                        // close ReactorChannel
                        if (reactorChannel != null)
                        {
                            reactorChannel.Close(out _);
                        }

                        break;
                    }
                case ReactorChannelEventType.WARNING:
                    Console.WriteLine("Received ReactorChannel WARNING event\n");
                    break;
                default:
                    {
                        Console.WriteLine("Unknown channel event!\n");
                        return ReactorCallbackReturnCode.SUCCESS;
                    }
            }

            return ReactorCallbackReturnCode.SUCCESS;
        }

        public ReactorCallbackReturnCode DefaultMsgCallback(ReactorMsgEvent evt)
        {
            ReactorChannel reactorChannel = evt.ReactorChannel!;

            ProcessMessage(reactorChannel, (Msg)evt.Msg!);

            return ReactorCallbackReturnCode.SUCCESS;
        }

        public ReactorCallbackReturnCode RdmLoginMsgCallback(RDMLoginMsgEvent evt)
        {
            ReactorChannel reactorChannel = evt.ReactorChannel!;
            ProviderSession provSession = (ProviderSession)reactorChannel.UserSpecObj!;

            LoginMsg loginMsg = evt.LoginMsg!;

            switch (loginMsg.LoginMsgType)
            {
                case LoginMsgType.REQUEST:
                    //send login response
                    LoginRequest loginRequest = loginMsg.LoginRequest!;
                    loginRequest.Copy(m_LoginProvider.LoginRequest);
                    m_LoginProvider.SendRefreshReactor(provSession.ClientChannelInfo!, out _);
                    break;
                case LoginMsgType.CLOSE:
                    Console.WriteLine("Received Login Close for streamId " + loginMsg.LoginClose!.StreamId);
                    break;
                default:
                    Console.WriteLine("Received Unhandled Login Msg Class: " + evt.Msg!.MsgClass);
                    break;
            }

            return ReactorCallbackReturnCode.SUCCESS;
        }

        public ReactorCallbackReturnCode RdmDirectoryMsgCallback(RDMDirectoryMsgEvent evt)
        {
            ReactorChannel reactorChannel = evt.ReactorChannel!;
            ProviderSession provSession = (ProviderSession)reactorChannel.UserSpecObj!;

            DirectoryMsg directoryMsg = evt.DirectoryMsg!;

            switch (directoryMsg.DirectoryMsgType)
            {
                case DirectoryMsgType.REQUEST:
                    DirectoryRequest directoryRequest = directoryMsg.DirectoryRequest!;
                    directoryRequest.Copy(m_DirectoryProvider.DirectoryRequest);
                    Console.WriteLine("Received Source Directory Request");
                    // send source directory response
                    if (m_DirectoryProvider.SendRefreshReactor(provSession.ClientChannelInfo, out var reactorErrorInfo) < ReactorReturnCode.SUCCESS)
                    {
                        Console.WriteLine($"Failure sending Directory refresh: {reactorErrorInfo?.Error?.Text}");
                        m_CallbackFailedChannel = reactorChannel;
                    }
                    break;
                case DirectoryMsgType.CLOSE:
                    Console.WriteLine("Received Directory Close for streamId " + directoryMsg.StreamId);
                    break;
                default:
                    Console.Error.WriteLine("Received unhandled Source Directory msg type: " + evt.Msg!.MsgClass);
                    return ReactorCallbackReturnCode.FAILURE;
            }

            return ReactorCallbackReturnCode.SUCCESS;
        }

        public ReactorCallbackReturnCode RdmDictionaryMsgCallback(RDMDictionaryMsgEvent evt)
        {
            ReactorChannel reactorChannel = evt.ReactorChannel!;
            ProviderSession provSession = (ProviderSession)reactorChannel.UserSpecObj!;

            DictionaryMsg dictionaryMsg = evt.DictionaryMsg!;

            switch (dictionaryMsg.DictionaryMsgType)
            {
                case DictionaryMsgType.REQUEST:
                    DictionaryRequest dictionaryRequest = dictionaryMsg.DictionaryRequest!;
                    dictionaryRequest.Copy(m_DictionaryProvider.DictionaryRequest);
                    Console.WriteLine("Received Dictionary Request for DictionaryName: " + dictionaryRequest.DictionaryName);
                    if (m_DictionaryProvider.FieldDictionaryDownloadName.Equals(dictionaryRequest.DictionaryName))
                    {
                        if (m_DictionaryProvider.SendFieldDictionaryResponseReactor(provSession.ClientChannelInfo, out var errorInfo) < ReactorReturnCode.SUCCESS)
                        {
                            Console.WriteLine($"Failure sending RDMFieldDictionary: {errorInfo?.Error?.Text}");
                            m_CallbackFailedChannel = reactorChannel;
                        }
                    }
                    else if (m_DictionaryProvider.EnumTypeDictionaryDownloadName.Equals(dictionaryRequest.DictionaryName))
                    {
                        if (m_DictionaryProvider.SendEnumTypeDictionaryResponseReactor(provSession.ClientChannelInfo, out var errorInfo) < ReactorReturnCode.SUCCESS)
                        {
                            Console.WriteLine($"Failure sending EnumType Dictionary: {errorInfo?.Error?.Text}");
                            m_CallbackFailedChannel = reactorChannel;
                        }
                    }
                    else
                    {
                        m_DictionaryProvider.SendRequestRejectReactor(provSession.ClientChannelInfo, dictionaryMsg.StreamId,
                            DictionaryRejectReason.UNKNOWN_DICTIONARY_NAME, out _);
                    }
                    break;
                case DictionaryMsgType.CLOSE:
                    Console.WriteLine("Received Dictionary Close for streamId " + dictionaryMsg.StreamId);
                    break;

                default:
                    Console.WriteLine("Received unhandled Source Directory msg type: " + evt.Msg!.MsgClass);
                    break;
            }

            return ReactorCallbackReturnCode.SUCCESS;
        }

        void ProcessMessage(ReactorChannel reactorChannel, Msg msg)
        {
            ProviderSession provSession = (ProviderSession)reactorChannel.UserSpecObj!;
            ProviderThread providerThread = provSession.ProviderThread!;

            m_DecodeIter.Clear();

            if (msg.EncodedDataBody != null && msg.EncodedDataBody.Data() != null)
            {
                m_DecodeIter.SetBufferAndRWFVersion(msg.EncodedDataBody, reactorChannel.MajorVersion, reactorChannel.MinorVersion);
            }

            switch (msg.DomainType)
            {
                case (int)DomainType.MARKET_PRICE:
                    if (m_XmlMsgData.UpdateCount > 0)
                        m_ItemRequestHandler.ProcessMsg(providerThread, provSession, msg,
                            m_DirectoryProvider.OpenLimit, m_DirectoryProvider.ServiceId, m_DirectoryProvider.Qos,
                            m_DecodeIter, out var errorInfo);
                    else
                        m_ItemRequestHandler.SendRequestReject(providerThread, provSession, msg, ItemRejectReason.DOMAIN_NOT_SUPPORTED, out _);
                    break;
                default:
                    m_ItemRequestHandler.SendRequestReject(providerThread, provSession, msg, ItemRejectReason.DOMAIN_NOT_SUPPORTED, out _);
                    break;
            }
        }
        #endregion
    }
}
