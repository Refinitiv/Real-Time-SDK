/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.Md for details.                  --
 *|           Copyright (C) 2022 Refinitiv. All rights reserved.              --
 *|-----------------------------------------------------------------------------
 */

using Refinitiv.Eta.Codec;
using Refinitiv.Eta.Example.Common;
using Refinitiv.Eta.Rdm;
using Refinitiv.Eta.Transports;
using Refinitiv.Eta.Transports.Interfaces;
using Refinitiv.Eta.ValueAdd.Rdm;
using Refinitiv.Eta.ValueAdd.Reactor;
using static Refinitiv.Eta.Rdm.Directory;
using System.Net.Sockets;

namespace Refinitiv.Eta.ValueAdd.Provider
{
    public class VAProvider : IRDMLoginMsgCallback, IDirectoryMsgCallback, IDictionaryMsgCallback, IReactorChannelEventCallback, IDefaultMsgCallback
    {
        // client sessions over this limit gets rejected with NAK mount
        const int NUM_CLIENT_SESSIONS = 5;

        private const int UPDATE_INTERVAL = 500000;

        private BindOptions m_BindOptions = new BindOptions();        
        private ReactorOptions m_ReactorOptions = new ReactorOptions();
        private ReactorAcceptOptions m_ReactorAcceptOptions = new ReactorAcceptOptions();
        private ProviderRole m_ProviderRole = new ProviderRole();
        private ReactorErrorInfo? m_ErrorInfo = null;
        private ReactorDispatchOptions m_DispatchOptions = new ReactorDispatchOptions();
        private ProviderCmdLineParser m_ProviderCmdLineParser = new ProviderCmdLineParser();
        private Error error = new Error();
        private DecodeIterator m_DecodeIterator = new DecodeIterator();
        private DictionaryHandler m_DictionaryHandler;
        private DirectoryHandler m_DirectoryHandler;
        private LoginHandler m_LoginHandler;
        private ItemHandler m_ItemHandler;
        private Dictionary<ReactorChannel, Int64> m_SocketFdValueMap = new Dictionary<ReactorChannel, Int64>();
        private Dictionary<Socket, ReactorChannel> m_SocketChannelMap = new Dictionary<Socket, ReactorChannel>();

        private IServer? m_Server;
        private Reactor.Reactor? m_Reactor;

        private int m_ClientSessionCount = 0;
        List<ReactorChannel> m_ReactorChannelList = new List<ReactorChannel>();
        List<Socket> m_SocketList = new List<Socket>();
        List<Socket> m_CurrentSocketList = new List<Socket>();

        private string? portNo;
        private string? serviceName;
        private int serviceId;
        private long runtime;

        private long m_CloseTime;
        private long m_CloseRunTime;
        bool m_CloseHandled;

        private const string defaultSrvrPortNo = "14002";
        private const string defaultServiceName = "DIRECT_FEED";
        private const int defaultServiceId = 1;

        public VAProvider()
        {
            m_DictionaryHandler = new DictionaryHandler();
            m_DirectoryHandler = new DirectoryHandler();
            m_LoginHandler = new LoginHandler();
            m_ItemHandler = new ItemHandler(m_DictionaryHandler, m_LoginHandler);
            m_ProviderRole.ChannelEventCallback = this;
            m_ProviderRole.DefaultMsgCallback = this;
            m_ProviderRole.DictionaryMsgCallback = this;
            m_ProviderRole.DirectoryMsgCallback = this;
            m_ProviderRole.LoginMsgCallback = this;
            m_CloseTime = 10;
        }

        private void Init(string[] args)
        {
            // parse command line
            if (!m_ProviderCmdLineParser.ParseArgs(args))
            {
                Console.Error.WriteLine("\nError loading command line arguments:\n");
                m_ProviderCmdLineParser.PrintUsage();
                Environment.Exit((int)ReactorReturnCode.FAILURE);
            }

            if (m_ProviderCmdLineParser.PortNo != null)
            {
                portNo = m_ProviderCmdLineParser.PortNo;
            }
            else
            {
                portNo = defaultSrvrPortNo;
            }
            if (m_ProviderCmdLineParser.ServiceName != null)
            {
                serviceName = m_ProviderCmdLineParser.ServiceName;
            }
            else
            {
                serviceName = defaultServiceName;
            }
            if (m_ProviderCmdLineParser.ServiceId != 0)
            {
                serviceId = m_ProviderCmdLineParser.ServiceId;
            }
            else
            {
                serviceId = defaultServiceId;
            }

            runtime = (System.DateTime.Now.Ticks / TimeSpan.TicksPerMillisecond) + (m_ProviderCmdLineParser.Runtime * 1000);
            m_CloseRunTime = (System.DateTime.Now.Ticks / TimeSpan.TicksPerMillisecond) + (m_ProviderCmdLineParser.Runtime + m_CloseTime) * 1000;

            Console.WriteLine($"ConnectionType: {(m_ProviderCmdLineParser.ConnectionType == ConnectionType.SOCKET ? "socket" : "encrypted")}");
            Console.WriteLine($"portNo: {portNo}");
            Console.WriteLine($"interfaceName: {m_ProviderCmdLineParser.InterfaceName}");
            Console.WriteLine($"serviceName: {serviceName}");
            Console.WriteLine($"serviceId: {serviceId}");
            Console.WriteLine($"enableRTT: {m_ProviderCmdLineParser.EnableRtt}");

            if (m_ProviderCmdLineParser.ConnectionType == ConnectionType.ENCRYPTED)
            {
                Console.WriteLine($"Certificate: {m_ProviderCmdLineParser.KeyCertificate}");
                Console.WriteLine($"Key: {m_ProviderCmdLineParser.PrivateKey}");
            }

            // load dictionary
            if (!m_DictionaryHandler.LoadDictionary())
            {
                Console.WriteLine("Error loading dictionary...");
                Environment.Exit((int)ReactorReturnCode.FAILURE);
            }

            // enable Reactor XML tracing if specified
            if (m_ProviderCmdLineParser.EnableXmlTracing)
            {
                m_ReactorOptions.XmlTracing = true;
            }

            // create reactor
            m_Reactor = Reactor.Reactor.CreateReactor(m_ReactorOptions, out m_ErrorInfo);
            if (m_ErrorInfo != null && m_ErrorInfo.Code != ReactorReturnCode.SUCCESS)
            {
                Console.WriteLine($"CreateReactor() failed: {m_ErrorInfo.ToString()}");
                Environment.Exit((int)ReactorReturnCode.FAILURE);
            }

            m_SocketList.Add(m_Reactor!.EventSocket!);

            // bind server
            if (m_ProviderCmdLineParser.ConnectionType == ConnectionType.ENCRYPTED)
            {
                m_BindOptions.ConnectionType = ConnectionType.ENCRYPTED;
                m_BindOptions.BindEncryptionOpts.ServerCertificate = m_ProviderCmdLineParser.KeyCertificate;
                m_BindOptions.BindEncryptionOpts.ServerPrivateKey = m_ProviderCmdLineParser.PrivateKey;
            }
            m_BindOptions.GuaranteedOutputBuffers = 1500;
            m_BindOptions.MajorVersion = Codec.Codec.MajorVersion();
            m_BindOptions.MinorVersion = Codec.Codec.MinorVersion();
            m_BindOptions.ProtocolType = Codec.Codec.RWF_PROTOCOL_TYPE;
            m_BindOptions.ServiceName = portNo;
            m_BindOptions.InterfaceName = m_ProviderCmdLineParser.InterfaceName;

            m_Server = Transport.Bind(m_BindOptions, out error);
            if (m_Server == null)
            {
                Console.WriteLine($"Error initializing server: {(error != null ? error.Text : "")}");
                Environment.Exit((int)ReactorReturnCode.FAILURE);
            }

            Console.WriteLine($"\nServer bound on port {m_Server.PortNumber}");
            m_SocketList.Add(m_Server.Socket);

            // initialize handlers
            m_LoginHandler.Init();
            m_LoginHandler.EnableRtt = m_ProviderCmdLineParser.EnableRtt;
            m_DirectoryHandler.Init();
            m_DirectoryHandler.ServiceName = serviceName;
            m_ItemHandler.Init();
            m_DirectoryHandler.ServiceId = serviceId;
            m_ItemHandler.ServiceId = serviceId;
        }

        public ReactorCallbackReturnCode ReactorChannelEventCallback(ReactorChannelEvent reactorEvent)
        {
            ReactorChannel? reactorChannel = reactorEvent.ReactorChannel;
            switch (reactorEvent.EventType)
            {
                case ReactorChannelEventType.CHANNEL_UP:
                    {
                        // A channel that we have requested via Reactor.Accept() has come up.
                        // Register selector so we can be notified to start calling dispatch() for
                        // this channel
                        Console.WriteLine("\nConnection up!");
                        Console.WriteLine($"New client on Channel {reactorChannel!.Channel!}");

                        // register selector with channel event's reactorChannel
                        m_SocketChannelMap.Add(reactorChannel.Channel!.Socket, reactorChannel);
                        m_SocketList.Add(reactorChannel.Channel.Socket);

                        int rcvBufSize = 65535;
                        int sendBufSize = 65535;
                        // Change size of send/receive buffer since it's small by default on some platforms
                        if (reactorChannel.IOCtl(IOCtlCode.SYSTEM_WRITE_BUFFERS, sendBufSize, out m_ErrorInfo) != ReactorReturnCode.SUCCESS)
                        {
                            Console.WriteLine($"ReactorChannel.IOCtl() failed: {(m_ErrorInfo != null ? m_ErrorInfo.Error.Text : "")}");
                            return ReactorCallbackReturnCode.SUCCESS;
                        }

                        if (reactorChannel.IOCtl(IOCtlCode.SYSTEM_READ_BUFFERS, rcvBufSize, out m_ErrorInfo) != ReactorReturnCode.SUCCESS)
                        {
                            Console.WriteLine($"ReactorChannel.IOCtl() failed: {(m_ErrorInfo != null ? m_ErrorInfo.Error.Text : "")}");
                            return ReactorCallbackReturnCode.SUCCESS;
                        }
                        break;
                    }
                case ReactorChannelEventType.CHANNEL_READY:
                    // The channel has exchanged the messages necessary to setup the connection
                    // and is now ready for general use. For an RDM Provider, this normally immediately
                    // follows the CHANNEL_UP event.
                    m_ReactorChannelList.Add(reactorChannel!);

                    //define new socket fd value
                    m_SocketFdValueMap.Add(reactorChannel!, reactorChannel!.Channel!.Socket.Handle.ToInt64());
                    break;
                case ReactorChannelEventType.FD_CHANGE:
                    // The notifier representing the ReactorChannel has been changed.
                    Console.WriteLine("Channel Change...");

                    m_SocketFdValueMap.Remove(reactorChannel!);
                    m_SocketFdValueMap.Add(reactorChannel!, reactorChannel!.Channel!.Socket.Handle.ToInt64());
                    m_SocketChannelMap.Remove(reactorChannel.OldSocket!);
                    m_SocketChannelMap.Add(reactorChannel.Socket!, reactorChannel);
                    m_SocketList.Remove(reactorChannel.OldSocket!);
                    m_SocketList.Add(reactorChannel.Socket!);
                    break;
                case ReactorChannelEventType.CHANNEL_DOWN:
                    {
                        if (reactorEvent.ReactorChannel!.Channel != null)
                            Console.WriteLine($"\nConnection down: Channel {reactorEvent.ReactorChannel}");
                        else
                        Console.WriteLine("\nConnection down");

                        if (reactorEvent.ReactorErrorInfo != null && reactorEvent.ReactorErrorInfo.Error.Text != null)
                            Console.WriteLine($"	Error text: {reactorEvent.ReactorErrorInfo.Error.Text}\n");

                        // send close status messages to all item streams
                        m_ItemHandler.SendCloseStatusMsgs(reactorChannel!, out m_ErrorInfo);

                        // send close status message to source directory stream
                        m_DirectoryHandler.SendCloseStatus(reactorChannel!, out m_ErrorInfo);

                        // send close status messages to dictionary streams
                        m_DictionaryHandler.SendCloseStatusMsgs(reactorChannel!, out m_ErrorInfo);

                        // It is important to make sure that no more interface calls are made using the channel after
                        // calling ReactorChannel.Close(). Because this application is single-threaded, it is safe
                        // to call it inside callback functions.
                        RemoveClientSessionForChannel(reactorChannel);
                        break;
                    }
                case ReactorChannelEventType.WARNING:
                    Console.WriteLine("Received ReactorChannel WARNING event\n");
                    break;
                default:
                    Console.WriteLine("Unknown channel event!\n");
                    CleanupAndExit();
                    break;
            }

            return ReactorCallbackReturnCode.SUCCESS;
        }

        public ReactorCallbackReturnCode DefaultMsgCallback(ReactorMsgEvent reactorEvent)
        {
            Msg msg = (Msg)reactorEvent.Msg!;
            ReactorChannel reactorChannel = reactorEvent.ReactorChannel!;

            if (msg == null)
            {
                Console.WriteLine($"DefaultMsgCallback() received error: {reactorEvent.ReactorErrorInfo.Error.Text} at {reactorEvent.ReactorErrorInfo.Location}\n");
                RemoveClientSessionForChannel(reactorChannel);
                return ReactorCallbackReturnCode.SUCCESS;
            }

            // clear decode iterator
            m_DecodeIterator.Clear();
            // set buffer and version info
            if (msg.EncodedDataBody.Data() != null)
            {
                m_DecodeIterator.SetBufferAndRWFVersion(msg.EncodedDataBody, reactorChannel.MajorVersion, reactorChannel.MinorVersion);
            }

            switch (msg.DomainType)
            {
                case (int)DomainType.MARKET_PRICE:
                case (int)DomainType.MARKET_BY_ORDER:
                case (int)DomainType.MARKET_BY_PRICE:
                case (int)DomainType.YIELD_CURVE:
                case (int)DomainType.SYMBOL_LIST:
                    if (m_ItemHandler.ProcessRequest(reactorChannel, msg, m_DecodeIterator, out m_ErrorInfo) != ReactorReturnCode.SUCCESS)
                    {
                        RemoveClientSessionForChannel(reactorChannel);
                        break;
                    }
                    break;
                default:
                    switch (msg.MsgClass)
                    {
                        case MsgClasses.REQUEST:
                            if (m_ItemHandler.SendItemRequestReject(reactorChannel, msg.StreamId, msg.DomainType, ItemRejectReason.DOMAIN_NOT_SUPPORTED, false, out m_ErrorInfo) != ReactorReturnCode.SUCCESS)
                                RemoveClientSessionForChannel(reactorChannel);
                            break;
                        case MsgClasses.CLOSE:
                            Console.WriteLine($"Received close message with streamId={msg.StreamId} and unsupported Domain '{msg.DomainType}'");
                            break;
                        default:
                            Console.WriteLine($"Received unhandled Msg Class: {MsgClasses.ToString(msg.MsgClass)} with streamId={msg.StreamId} and unsupported Domain '{msg.DomainType}'");
                            break;
                    }
                    break;
            }

            return ReactorCallbackReturnCode.SUCCESS;
        }

        public ReactorCallbackReturnCode RdmLoginMsgCallback(RDMLoginMsgEvent reactorEvent)
        {
            LoginMsg? loginMsg = reactorEvent.LoginMsg;
            ReactorChannel? reactorChannel = reactorEvent.ReactorChannel;

            if (loginMsg == null)
            {
                Console.WriteLine($"RdmLoginMsgCallback() received error: {reactorEvent.ReactorErrorInfo.Error.Text} at {reactorEvent.ReactorErrorInfo.Location}\n");

                if (reactorEvent.Msg != null)
                {
                    if (m_LoginHandler.SendRequestReject(reactorChannel!, reactorEvent.Msg.StreamId, LoginRejectReason.LOGIN_RDM_DECODER_FAILED, out m_ErrorInfo) != ReactorReturnCode.SUCCESS)
                    {
                        RemoveClientSessionForChannel(reactorChannel);
                    }
                    return ReactorCallbackReturnCode.SUCCESS;
                }
                else
                {
                    RemoveClientSessionForChannel(reactorChannel);
                    return ReactorCallbackReturnCode.SUCCESS;
                }
            }

            switch (loginMsg.LoginMsgType)
            {
                case LoginMsgType.REQUEST:
                    {
                        LoginRequest loginRequest = loginMsg.LoginRequest!;

                        if (m_LoginHandler.GetLoginRequestInfo(reactorChannel!, loginRequest) == null)
                        {
                            RemoveClientSessionForChannel(reactorChannel);
                            break;
                        }

                        Console.WriteLine($"\nReceived Login Request for Username: {loginRequest.UserName}");

                        if (m_LoginHandler.SendRefresh(reactorChannel!, loginRequest, out m_ErrorInfo) != ReactorReturnCode.SUCCESS)
                        {
                            RemoveClientSessionForChannel(reactorChannel);
                        }
                        break;
                    }
                case LoginMsgType.CLOSE:
                    Console.WriteLine($"\nReceived Login Close for StreamId {loginMsg.LoginClose!.StreamId}");
                    m_LoginHandler.CloseStream(loginMsg.LoginClose.StreamId);
                    break;
                case LoginMsgType.RTT:
                    LoginRTT? loginRTT = loginMsg.LoginRTT!;
                    Console.WriteLine($"Received login RTT message from Consumer {m_SocketFdValueMap[reactorEvent.ReactorChannel!]}.\n");
                    Console.WriteLine($"\tRTT Tick value is {loginRTT.Ticks}\n");
                    if (loginRTT.HasTCPRetrans) 
                    {
                        Console.WriteLine($"\tConsumer side TCP retransmissions: {loginRTT.TCPRetrans}\n");
                    }
                    long calculatedRtt = loginRTT.CalculateRTTLatency();
                    LoginRTT storedLoginRtt = m_LoginHandler.GetLoginRtt(reactorChannel!)!.LoginRtt;
                    loginRTT.Copy(storedLoginRtt);
                    Console.WriteLine($"\tLast RTT message latency is {calculatedRtt}.\n\n");
                    break;
                default:
                    Console.WriteLine($"\nReceived unhandled login msg type: {loginMsg.LoginMsgType}");
                    break;
            }

            return ReactorCallbackReturnCode.SUCCESS;
        }

        public ReactorCallbackReturnCode RdmDirectoryMsgCallback(RDMDirectoryMsgEvent reactorEvent)
        {
            DirectoryMsg directoryMsg = reactorEvent.DirectoryMsg!;
            ReactorChannel? reactorChannel = reactorEvent.ReactorChannel;

            if (directoryMsg == null)
            {
                Console.WriteLine($"DirectoryMsgCallback() received error: {reactorEvent.ReactorErrorInfo.Error.Text} at {reactorEvent.ReactorErrorInfo.Location}\n");

                if (reactorEvent.Msg != null)
                {
                    if (m_DirectoryHandler.SendRequestReject(reactorChannel!, reactorEvent.Msg.StreamId, DirectoryRejectReason.DIRECTORY_RDM_DECODER_FAILED, out m_ErrorInfo) != ReactorReturnCode.SUCCESS)
                    {
                        RemoveClientSessionForChannel(reactorChannel);
                    }
                    return ReactorCallbackReturnCode.SUCCESS;
                }
                else
                {
                    RemoveClientSessionForChannel(reactorChannel);
                    return ReactorCallbackReturnCode.SUCCESS;
                }
            }

            switch (directoryMsg.DirectoryMsgType)
            {
                case DirectoryMsgType.REQUEST:
                    {
                        DirectoryRequest directoryRequest = directoryMsg.DirectoryRequest!;

                        // Reject any request that does not request at least the Info, State, and Group filters
                        if (((directoryRequest.Filter & ServiceFilterFlags.INFO) == 0) ||
                            ((directoryRequest.Filter & ServiceFilterFlags.STATE) == 0) ||
                            ((directoryRequest.Filter & ServiceFilterFlags.GROUP) == 0))
                        {
                            if (m_DirectoryHandler.SendRequestReject(reactorChannel!, reactorEvent.Msg!.StreamId, DirectoryRejectReason.INCORRECT_FILTER_FLAGS, out m_ErrorInfo) != ReactorReturnCode.SUCCESS)
                            RemoveClientSessionForChannel(reactorChannel);
                            break;
                        }

                        if (m_DirectoryHandler.GetDirectoryRequest(reactorChannel!, directoryRequest) == null)
                        {
                            RemoveClientSessionForChannel(reactorChannel);
                            break;
                        }

                        Console.WriteLine("\nReceived Source Directory Request");

                        // send source directory response
                        if (m_DirectoryHandler.SendRefresh(reactorChannel!, directoryRequest, out m_ErrorInfo) != ReactorReturnCode.SUCCESS)
                            RemoveClientSessionForChannel(reactorChannel);
                        break;
                    }
                case DirectoryMsgType.CLOSE:
                    {
                        Console.WriteLine($"\nReceived Source Directory Close for StreamId {directoryMsg.StreamId}");

                        // close source directory stream
                        m_DirectoryHandler.CloseStream(directoryMsg.StreamId);
                        break;
                    }
                default:
                    Console.WriteLine($"\nReceived unhandled Source Directory msg type: {directoryMsg.DirectoryMsgType}");
                    break;
            }

            return ReactorCallbackReturnCode.SUCCESS;
        }

        public ReactorCallbackReturnCode RdmDictionaryMsgCallback(RDMDictionaryMsgEvent reactorEvent)
        {
            ReactorReturnCode ret;
            DictionaryMsg dictionaryMsg = reactorEvent.DictionaryMsg!;
            ReactorChannel? reactorChannel = reactorEvent.ReactorChannel;

            if(dictionaryMsg == null)
            {
                Console.WriteLine($"DictionaryMsgCallback() received error: {reactorEvent.ReactorErrorInfo.Error.Text} at {reactorEvent.ReactorErrorInfo.Location}\n");
                if (reactorEvent.Msg != null)
                {
                    if (m_DictionaryHandler.SendRequestReject(reactorChannel!, reactorEvent.Msg.StreamId, DictionaryRejectReason.DICTIONARY_RDM_DECODER_FAILED, out m_ErrorInfo) != ReactorReturnCode.SUCCESS)
                    RemoveClientSessionForChannel(reactorChannel);

                    return ReactorCallbackReturnCode.SUCCESS;
                }
                else
                {
                    RemoveClientSessionForChannel(reactorChannel);
                    return ReactorCallbackReturnCode.SUCCESS;
                }
            }

            switch (dictionaryMsg.DictionaryMsgType)
            {
                case DictionaryMsgType.REQUEST:
                    {
                        DictionaryRequest dictionaryRequest = dictionaryMsg.DictionaryRequest!;

                        if (m_DictionaryHandler.GetDictionaryRequestInfo(reactorChannel!, dictionaryRequest) == null)
                        {
                            RemoveClientSessionForChannel(reactorChannel);
                            break;
                        }

                        Console.WriteLine($"\nReceived Dictionary Request for DictionaryName: {dictionaryRequest.DictionaryName}");

                        if (DictionaryHandler.FieldDictionaryDownloadName.Equals(dictionaryRequest.DictionaryName))
                        {
                            // Name matches field dictionary. Send the field dictionary refresh
                            if ((ret = m_DictionaryHandler.SendFieldDictionaryResponse(reactorChannel!, dictionaryRequest, out m_ErrorInfo)) != ReactorReturnCode.SUCCESS)
                            {
                                Console.WriteLine($"SendFieldDictionaryResponse() failed: {ret}");
                                RemoveClientSessionForChannel(reactorChannel);
                            }
                        }
                        else if (DictionaryHandler.EnumTypeDictionaryDownloadName.Equals(dictionaryRequest.DictionaryName))
                        {
                            // Name matches the enum types dictionary. Send the enum types dictionary refresh
                            if ((ret = m_DictionaryHandler.SendEnumTypeDictionaryResponse(reactorChannel!, dictionaryRequest, out m_ErrorInfo)) != ReactorReturnCode.SUCCESS)
                            {
                                Console.WriteLine($"SendEnumTypeDictionaryResponse() failed: {ret}");
                                RemoveClientSessionForChannel(reactorChannel);
                            }
                        }
                        else
                        {
                            if (m_DictionaryHandler.SendRequestReject(reactorChannel!, reactorEvent.Msg!.StreamId, DictionaryRejectReason.UNKNOWN_DICTIONARY_NAME, out m_ErrorInfo) != ReactorReturnCode.SUCCESS)
                            {
                                RemoveClientSessionForChannel(reactorChannel);
                            }
                        }
                        break;
                    }
                case DictionaryMsgType.CLOSE:
                    Console.WriteLine($"\nReceived Dictionary Close for StreamId {dictionaryMsg.StreamId}");

                    /* close dictionary stream */
                    m_DictionaryHandler.CloseStream(dictionaryMsg.StreamId);
                    break;
                default:
                    Console.WriteLine($"\nReceived Unhandled Dictionary Msg Type: {dictionaryMsg.DictionaryMsgType}");
                    break;
            }

            return ReactorCallbackReturnCode.SUCCESS;
        }

        private void SendCloseStatusMessages()
        {
            // send close status messages to all streams on all channels
            foreach (ReactorChannel reactorChnl in m_ReactorChannelList)
            {
                if (reactorChnl != null)
                {
                    // send close status messages to all item streams 
                    if (m_ItemHandler.SendCloseStatusMsgs(reactorChnl, out m_ErrorInfo) != ReactorReturnCode.SUCCESS)
                        Console.WriteLine($"Error sending item close: {m_ErrorInfo?.Error.Text}");               

                    // send close status message to source directory stream
                    if (m_DirectoryHandler.SendCloseStatus(reactorChnl, out m_ErrorInfo) != ReactorReturnCode.SUCCESS)
                        Console.WriteLine($"Error sending directory close: {m_ErrorInfo.Error.Text}");

                    // send close status messages to dictionary streams
                    if (m_DictionaryHandler.SendCloseStatusMsgs(reactorChnl, out m_ErrorInfo) != ReactorReturnCode.SUCCESS)
                        Console.WriteLine($"Error sending dictionary close: {m_ErrorInfo?.Error.Text}");
                }
            }
        }

        private void RemoveClientSessionForChannel(ReactorChannel? reactorChannel)
        {
            if (reactorChannel != null)
            {
                m_ReactorChannelList.Remove(reactorChannel);
                m_DictionaryHandler.CloseStream(reactorChannel);
                m_DirectoryHandler.CloseStream(reactorChannel);
                m_LoginHandler.CloseStream(reactorChannel);
                m_ItemHandler.CloseStream(reactorChannel);
                m_SocketChannelMap.Remove(reactorChannel.Socket!);
                m_SocketList.Remove(reactorChannel.Socket!);
                reactorChannel.Close(out ReactorErrorInfo? errorInfo);
                if (errorInfo != null)
                {
                    Console.WriteLine($"Error closing Reactor channel: {errorInfo.Error.Text}");
                }
                m_SocketFdValueMap.Remove(reactorChannel);
                m_ClientSessionCount--;
            }           
        }

        private void Uninitialize()
        {
            // shutdown reactor
            if (m_Reactor!.Shutdown(out m_ErrorInfo) != ReactorReturnCode.SUCCESS)
            {
                Console.WriteLine($"Error shutting down Reactor: {(error != null ? error.Text : "")}");
            }
        }

        private void CleanupAndExit()
        {
            Uninitialize();
            Environment.Exit((int)ReactorReturnCode.FAILURE);
        }

        private void Run()
        {
            ReactorReturnCode ret;
            // main loop
            while (true)
            {
                m_CurrentSocketList.Clear();
                m_CurrentSocketList.AddRange(m_SocketList);
                try
                {
                    if (m_CurrentSocketList.Count > 0)
                    {
                        Socket.Select(m_CurrentSocketList, null, null, UPDATE_INTERVAL);
                    }
                }
                catch (Exception e)
                {
                    Console.WriteLine($"Failure polling for socket operations: {e.Message}");
                    CleanupAndExit();
                }

                // nothing to read
                if (m_CurrentSocketList.Count == 0)
                {
                    // Send market price updates for each connected channel
                    m_ItemHandler.UpdateItemInfo();
                    foreach (ReactorChannel reactorChnl in m_ReactorChannelList)
                    {
                        if (reactorChnl != null && reactorChnl.State == ReactorChannelState.READY)
                        {
                            // try to send rtt message
                            ret = m_LoginHandler.SendRTT(reactorChnl, out m_ErrorInfo);
                            if (ret != ReactorReturnCode.SUCCESS)
                            {
                                if (reactorChnl.State != ReactorChannelState.CLOSED &&
                                    reactorChnl.State != ReactorChannelState.DOWN)
                                {
                                    Console.WriteLine(m_ErrorInfo != null ? m_ErrorInfo.Error.Text : $"Failed to send LoginRTT message, return code: {ret}");
                                    CleanupAndExit();
                                }
                            }

                            // process market price updates
                            ret = m_ItemHandler.SendItemUpdates(reactorChnl, out m_ErrorInfo);
                            if (ret != ReactorReturnCode.SUCCESS)
                            {
                                if (reactorChnl.State != ReactorChannelState.CLOSED &&
                                    reactorChnl.State != ReactorChannelState.DOWN)
                                {
                                    Console.WriteLine(m_ErrorInfo != null ? m_ErrorInfo.Error.Text : $"Failed to send item updates, return code: {ret}");
                                    CleanupAndExit();
                                }
                            }
                        }
                    }
                }
                else // notification occurred
                {
                    foreach (var socket in m_CurrentSocketList)
                    {
                        if (socket == m_Server!.Socket) // accept new connection
                        {
                            m_ClientSessionCount++;
                            m_ReactorAcceptOptions.Clear();
                            m_ReactorAcceptOptions.AcceptOptions.UserSpecObject = m_Server;
                            if (m_ClientSessionCount <= NUM_CLIENT_SESSIONS)
                            {
                                m_ReactorAcceptOptions.AcceptOptions.NakMount = false;
                            }
                            else
                            {
                                m_ReactorAcceptOptions.AcceptOptions.NakMount = true;
                            }
                            if (m_Reactor!.Accept(m_Server, m_ReactorAcceptOptions, m_ProviderRole, out m_ErrorInfo) == ReactorReturnCode.FAILURE)
                            {
                                Console.WriteLine($"Reactor accept error, text: {(m_ErrorInfo != null ? m_ErrorInfo.Error.Text : "")}");
                                CleanupAndExit();
                            }
                        }
                        else if (socket == m_Reactor!.EventSocket) // read from client reactor channel
                        {
                            m_DispatchOptions.ReactorChannel = null;
                            while ((ret = m_Reactor.Dispatch(m_DispatchOptions, out m_ErrorInfo)) > 0) { }
                            if (ret == ReactorReturnCode.FAILURE)
                            {
                                Console.WriteLine($"Failed to dispatch reactor event: {(m_ErrorInfo != null ? m_ErrorInfo.Error.Text : "")}");
                                CleanupAndExit();
                            }
                        } 
                        else // Reactor EventSocket
                        {
                            // retrieve associated reactor channel and dispatch on that channel
                            ReactorChannel reactorChnl = m_SocketChannelMap[socket];
                            m_DispatchOptions.ReactorChannel = reactorChnl;
                            // dispatch until no more messages
                            while ((ret = m_Reactor.Dispatch(m_DispatchOptions, out m_ErrorInfo)) > 0) { }
                            if (ret == ReactorReturnCode.FAILURE)
                            {
                                if (reactorChnl.State != ReactorChannelState.CLOSED &&
                                    reactorChnl.State != ReactorChannelState.DOWN)
                                {
                                    Console.WriteLine($"Failed to dispatch channel: {(m_ErrorInfo != null ? m_ErrorInfo.Error.Text : "")}");
                                    CleanupAndExit();
                                }
                            }
                        }
                    }
                }

                // Handle run-time
                if ((System.DateTime.Now.Ticks / TimeSpan.TicksPerMillisecond) >= runtime && !m_CloseHandled)
                {
                    Console.WriteLine("Provider run-time expired, closing the application...");
                    SendCloseStatusMessages();
                    m_CloseHandled = true;
                }
                else if ((System.DateTime.Now.Ticks / TimeSpan.TicksPerMillisecond) >= m_CloseRunTime)
                {
                    Console.WriteLine("Provider run-time expired...");
                    break;
                }
            }
        }

        public static void Main(string[] args)
        {
            VAProvider provider = new VAProvider();
            provider.Init(args);
            provider.Run();
            provider.Uninitialize();
            Environment.Exit((int)ReactorReturnCode.SUCCESS);
        }
    }
}
