/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.Md for details.                  --
 *|           Copyright (C) 2022-2023 Refinitiv. All rights reserved.              --
 *|-----------------------------------------------------------------------------
 */

using System;
using System.Collections.Generic;
using System.IO;
using System.Net.Sockets;

using LSEG.Eta.Codec;
using LSEG.Eta.Example.VACommon;
using LSEG.Eta.Rdm;
using LSEG.Eta.Transports;
using LSEG.Eta.ValueAdd.Rdm;
using LSEG.Eta.ValueAdd.Reactor;

namespace LSEG.Eta.ValueAdd.VANiProvider
{
    /// <summary>
    /// This is a main class to run ETA ValueAdd NIProvider application. The purpose of this application 
    /// is to non-interactively provide Level I Market Price and Level 2 Market By Order data to an Advanced Data Hub (ADH). 
    /// It is a single-threaded client application using ValueAdd components.
    /// 
    /// This class is responsible for the following:
    /// <ul>
    /// <li>Initialize and set command line options.
    /// <li>Load Dictionary from file.
    /// <li>Create a Reactor ETA Channel. It responds to messages through its Login, 
    /// Directory, Dictionary and default message callbacks.
    /// <li>Connect to the ADH provider, send item refreshes, then send item updates.
    /// </ul>
    /// <p>
    /// This class is also a call back for all events from Consumer/ADH. 
    /// It dispatches events to domain specific handlers.
    /// </p>
    /// <p>
    /// This application is intended as a basic usage example. 
    /// Some of the design choices were made to favor simplicity and readability over performance. 
    /// It is not intended to be used for measuring performance. 
    /// This application uses ValueAdd and shows how using ValueAdd simplifies 
    /// the writing of ETA applications. Because ValueAdd is a layer on top of ETA, you may see a 
    /// slight decrease in performance compared to writing applications directly to the ETA interfaces.
    /// </p>
    /// <H2>Setup Environment</H2>
    /// <p>
    /// The RDMFieldDictionary and enumtype.def files must be located 
    /// in the directory of execution.
    /// </p>
    /// </summary>
    public class VANiProvider : INiProviderCallback, IDictionaryMsgCallback
    {
        private Reactor.Reactor? m_Reactor;
        private ReactorOptions m_ReactorOptions = new ReactorOptions();
        private ReactorErrorInfo? m_ErrorInfo = null;
        private ReactorDispatchOptions m_DispatchOptions = new ReactorDispatchOptions();
        private ReactorSubmitOptions m_SubmitOptions = new ReactorSubmitOptions();
        private NIProviderCmdLineParser m_CommandLineParser = new NIProviderCmdLineParser();

        private long m_Runtime;

        private const int SEND_INTERVAL = 1000; // send content every 1000 milliseconds    

        private const string m_DefaultSrvrHostname = "localhost";   
        private const string m_DefaultSrvrPortNo = "14003";   
        private const string m_DefaultServiceName = "NI_PUB";  
        private const int m_DefaultServiceId = 5;
        private const string m_DefaultItemName = "TRI.N";
        private const string m_DefaultItemName1 = "IBM.N";
        private const EncryptionProtocolFlags m_DefaultEncryptionProtocolFlags = EncryptionProtocolFlags.ENC_NONE;

        private DataDictionary m_Dictionary;

        private const string FIELD_DICTIONARY_FILE_NAME = "RDMFieldDictionary";
        private const string ENUM_TABLE_FILE_NAME = "enumtype.def";    

        private bool m_RefreshesSent;

        List<ChannelInfo> m_ChnlInfoList = new List<ChannelInfo>();

        private Dictionary<Socket, ReactorChannel> m_SocketChannelMap = new Dictionary<Socket, ReactorChannel>();
        private List<Socket> m_Sockets = new List<Socket>();
        private List<Socket> m_CurrentSockets = new List<Socket>();

        public VANiProvider()
        {
            m_Dictionary = new DataDictionary();
            m_DispatchOptions.SetMaxMessages(1);
        }

        private void Init(string[] args)
        {
            // parse command line
            if (!m_CommandLineParser.ParseArgs(args))
            {
                Console.Error.WriteLine("\nError loading command line arguments:\n");
                m_CommandLineParser.PrintUsage();
                Environment.Exit((int)ReactorReturnCode.FAILURE);
            }

            // add default connections to arguments if none specified
            if (m_CommandLineParser.ConnectionList.Count == 0)
            {
                // first connection - localhost:14003 NI_PUB mp:TRI.N
                List<ItemArg> itemList = new List<ItemArg>();
                ItemArg itemArg = new ItemArg(DomainType.MARKET_PRICE, m_DefaultItemName, false);
                itemList.Add(itemArg);
                ItemArg itemArg1 = new ItemArg(DomainType.MARKET_PRICE, m_DefaultItemName1, false);
                itemList.Add(itemArg1);

                ItemArg itemArg2 = new ItemArg(DomainType.MARKET_BY_ORDER, m_DefaultItemName, false);
                itemList.Add(itemArg2);
                ItemArg itemArg3 = new ItemArg(DomainType.MARKET_BY_ORDER, m_DefaultItemName1, false);
                itemList.Add(itemArg3);
                ConnectionArg connectionArg = new ConnectionArg(ConnectionType.SOCKET,
                                                                m_DefaultServiceName,
                                                                m_DefaultSrvrHostname,
                                                                m_DefaultSrvrPortNo,
                                                                itemList,
                                                                m_DefaultEncryptionProtocolFlags);
                m_CommandLineParser.ConnectionList.Add(connectionArg);
            }

            Console.WriteLine("NIProvider initializing...");

            m_Runtime = DateTimeOffset.Now.ToUnixTimeMilliseconds() + m_CommandLineParser.Runtime * 1000;

            // load dictionary
            DictionaryDownloadMode dictionaryDownloadMode = LoadDictionary();

            // enable Reactor XML tracing if specified
            m_ReactorOptions.XmlTracing = m_CommandLineParser.EnableXmlTracing;

            // create reactor
            m_Reactor = Reactor.Reactor.CreateReactor(m_ReactorOptions, out m_ErrorInfo);
            if (m_Reactor is null || (m_ErrorInfo != null && m_ErrorInfo.Code != ReactorReturnCode.SUCCESS))
            {
                Console.WriteLine($"CreateReactor() failed: {m_ErrorInfo?.ToString()}");
                Environment.Exit((int)ReactorReturnCode.FAILURE);
            }

            m_Sockets.Add(m_Reactor.EventSocket!);

            // create channel info
            ChannelInfo chnlInfo = new ChannelInfo();
            chnlInfo.ConnectionArg = m_CommandLineParser.ConnectionList[0];
            chnlInfo.NiProviderRole.DictionaryDownloadMode = dictionaryDownloadMode;

            // initialize channel info
            InitChannelInfo(chnlInfo);

            ReactorReturnCode ret;

            if ((ret = m_Reactor.Connect(chnlInfo.ConnectOptions, chnlInfo.NiProviderRole, out m_ErrorInfo)) < ReactorReturnCode.SUCCESS)
            {
                Console.WriteLine($"Reactor.Connect failed with return code: {ret}, error = {(m_ErrorInfo != null ? m_ErrorInfo.Error.Text : "")}");
                Environment.Exit((int)ReactorReturnCode.FAILURE);
            }

            // add to ChannelInfo list
            m_ChnlInfoList.Add(chnlInfo);

        }

        private void InitChannelInfo(ChannelInfo chnlInfo)
        {
            // set up niprovider role   
            chnlInfo.NiProviderRole.ChannelEventCallback = this;
            chnlInfo.NiProviderRole.LoginMsgCallback = this;
            chnlInfo.NiProviderRole.DefaultMsgCallback = this;
            chnlInfo.NiProviderRole.DictionaryMsgCallback = this;

            chnlInfo.MarketPriceHandler = new MarketPriceHandler(chnlInfo.ItemWatchList, m_Dictionary);
            chnlInfo.MarketByOrderHandler = new MarketByOrderHandler(chnlInfo.ItemWatchList, m_Dictionary);

            // initialize niprovider role to default
            String serviceName = chnlInfo.ConnectionArg!.Service;
            chnlInfo.NiProviderRole.InitDefaultRDMLoginRequest();
            chnlInfo.NiProviderRole.InitDefaultRDMDirectoryRefresh(serviceName, m_DefaultServiceId);

            // use command line login user name if specified
            if (m_CommandLineParser.UserName != null && !m_CommandLineParser.UserName.Equals(""))
            {
                LoginRequest loginRequest = chnlInfo.NiProviderRole.RdmLoginRequest!;
                loginRequest.UserName.Data(m_CommandLineParser.UserName);
            }

            // use command line authentication token and extended authentication information if specified
            if (m_CommandLineParser.AuthenticationToken != null && !m_CommandLineParser.AuthenticationToken.Equals(""))
            {
                LoginRequest loginRequest = chnlInfo.NiProviderRole.RdmLoginRequest!;
                loginRequest.UserNameType = Login.UserIdTypes.AUTHN_TOKEN;
                loginRequest.UserName.Data(m_CommandLineParser.AuthenticationToken);

                if (m_CommandLineParser.AuthenticationExtended != null && !m_CommandLineParser.AuthenticationExtended.Equals(""))
                {
                    loginRequest.HasAuthenticationExtended = true;
                    loginRequest.AuthenticationExtended.Data(m_CommandLineParser.AuthenticationExtended);
                }
            }

            // use command line application id if specified
            if (!string.IsNullOrEmpty(m_CommandLineParser.ApplicationId))
            {
                LoginRequest loginRequest = chnlInfo.NiProviderRole.RdmLoginRequest!;
                loginRequest.LoginAttrib.ApplicationId.Data(m_CommandLineParser.ApplicationId);
            }

            CreateItemLists(chnlInfo);

            // set up reactor connect options
            chnlInfo.ConnectOptions.SetReconnectAttempLimit(-1); // attempt to recover forever
            chnlInfo.ConnectOptions.SetReconnectMinDelay(1000); // 1 second minimum
            chnlInfo.ConnectOptions.SetReconnectMaxDelay(60000); // 60 second maximum

            ConnectionType connectionType = chnlInfo.ConnectionArg.ConnectionType;

            if (connectionType == ConnectionType.SOCKET)
            {
                chnlInfo.ConnectOptions.ConnectionList[0].ConnectOptions.MajorVersion = Codec.Codec.MajorVersion();
                chnlInfo.ConnectOptions.ConnectionList[0].ConnectOptions.MinorVersion = Codec.Codec.MinorVersion();
                chnlInfo.ConnectOptions.ConnectionList[0].ConnectOptions.ConnectionType = connectionType;
                chnlInfo.ConnectOptions.ConnectionList[0].ConnectOptions.UnifiedNetworkInfo.ServiceName = chnlInfo.ConnectionArg.Port;
                chnlInfo.ConnectOptions.ConnectionList[0].ConnectOptions.UnifiedNetworkInfo.Address = chnlInfo.ConnectionArg.Hostname;
                chnlInfo.ConnectOptions.ConnectionList[0].ConnectOptions.UserSpecObject = chnlInfo;
            }
            // add backup connection if specified
            if (m_CommandLineParser.HasBackupConfig)
            {
                connectionType = m_CommandLineParser.BackupConnectionType;

                if (connectionType == ConnectionType.SOCKET && m_CommandLineParser.BackupHostname != null && m_CommandLineParser.BackupPort != null)
                {

                    ReactorConnectInfo connectInfo = new ReactorConnectInfo();
                    chnlInfo.ConnectOptions.ConnectionList.Add(connectInfo);
                    chnlInfo.ConnectOptions.ConnectionList[1].ConnectOptions.MajorVersion =Codec.Codec.MajorVersion();
                    chnlInfo.ConnectOptions.ConnectionList[1].ConnectOptions.MinorVersion = Codec.Codec.MinorVersion();
                    chnlInfo.ConnectOptions.ConnectionList[1].ConnectOptions.ConnectionType = connectionType;
                    chnlInfo.ConnectOptions.ConnectionList[1].ConnectOptions.UnifiedNetworkInfo.ServiceName = m_CommandLineParser.BackupPort;
                    chnlInfo.ConnectOptions.ConnectionList[1].ConnectOptions.UnifiedNetworkInfo.Address = m_CommandLineParser.BackupHostname;
                    chnlInfo.ConnectOptions.ConnectionList[1].ConnectOptions.UserSpecObject = chnlInfo;
                }
            }
        }

        private void CreateItemLists(ChannelInfo chnlInfo)
        {
            // add specified items to item watch list
            foreach (ItemArg itemArg in chnlInfo.ConnectionArg!.ItemList)
            {
                switch (itemArg.Domain)
                {
                    case DomainType.MARKET_PRICE:
                        if (!itemArg.EnablePrivateStream)
                        {
                            chnlInfo.MpItemList.Add(itemArg.ItemName!);
                        }
                        else
                        {
                            chnlInfo.MppsItemList.Add(itemArg.ItemName!);
                        }
                        break;
                    case DomainType.MARKET_BY_ORDER:
                        if (!itemArg.EnablePrivateStream)
                        {
                            chnlInfo.MboItemList.Add(itemArg.ItemName!);
                        }
                        else
                        {
                            chnlInfo.MbopsItemList.Add(itemArg.ItemName!);
                        }
                        break;
                    case DomainType.MARKET_BY_PRICE:
                        Console.WriteLine("Does not support publishing MarketByPrice items.");
                        Environment.Exit((int)ReactorReturnCode.FAILURE);
                        break;
                    default:
                        break;
                }
            }
        }

        private DictionaryDownloadMode LoadDictionary()
        {
            m_Dictionary.Clear();
            if (m_Dictionary.LoadFieldDictionary(FIELD_DICTIONARY_FILE_NAME, out CodecError fdError) < 0)
            {
                Console.WriteLine($"Unable to load field dictionary. Will attempt to download from provider.\n\tText: {fdError?.Text}");
                return DictionaryDownloadMode.FIRST_AVAILABLE;
            }

            if (m_Dictionary.LoadEnumTypeDictionary(ENUM_TABLE_FILE_NAME, out CodecError etError) < 0)
            {
                Console.WriteLine($"Unable to load enum dictionary. Will attempt to download from provider.\n\tText: {etError?.Text}");
                return DictionaryDownloadMode.FIRST_AVAILABLE;
            }
            return DictionaryDownloadMode.NONE;
        }

        private void Run()
        {
            int selectTime = 1000;
            ReactorReturnCode ret;
            long currentTime = DateTimeOffset.Now.ToUnixTimeMilliseconds();
            long nextSendTime = currentTime + SEND_INTERVAL;
            while (true)
            {
                try
                {
                    if (m_Sockets.Count > 0)
                    {
                        m_CurrentSockets.Clear();
                        m_CurrentSockets.AddRange(m_Sockets);

                        Socket.Select(m_CurrentSockets, null, null, selectTime * 1000);
                    }
                }
                catch (IOException e)
                {
                    Console.WriteLine($"Select failed: {e.Message}");
                    Environment.Exit((int)ReactorReturnCode.FAILURE);
                }

                // send login reissue if login reissue time has passed
                foreach (ChannelInfo chnlInfo in m_ChnlInfoList)
                {
                    if (chnlInfo.ReactorChannel == null || (chnlInfo.ReactorChannel.State != ReactorChannelState.UP && chnlInfo.ReactorChannel.State != ReactorChannelState.READY))
                    {
                        continue;
                    }

                    if (chnlInfo.CanSendLoginReissue && DateTimeOffset.Now.ToUnixTimeMilliseconds() >= chnlInfo.LoginReissueTime)
                    {
                        LoginRequest? loginRequest = chnlInfo.NiProviderRole.RdmLoginRequest;
                        m_SubmitOptions.Clear();
                        if (chnlInfo.ReactorChannel.Submit(loginRequest!, m_SubmitOptions, out m_ErrorInfo) != ReactorReturnCode.SUCCESS)
                        {
                            Console.WriteLine($"Login reissue failed. Error: {(m_ErrorInfo != null ? m_ErrorInfo.Error.Text : "")}");
                        }
                        else
                        {
                            Console.WriteLine("Login reissue sent");
                        }
                        chnlInfo.CanSendLoginReissue = false;
                    }
                }

                // Handle run-time
                if (DateTimeOffset.Now.ToUnixTimeMilliseconds() >= m_Runtime)
                {
                    Console.WriteLine("NIProvider run-time expired...");
                    break;
                }

                if (m_CurrentSockets.Count > 0)
                {
                    foreach (var socket in m_CurrentSockets)
                    {
                        if (socket == m_Reactor!.EventSocket)
                        {
                            m_DispatchOptions.ReactorChannel = null;
                            while ((ret = m_Reactor.Dispatch(m_DispatchOptions, out m_ErrorInfo)) > 0) { }
                            if (ret == ReactorReturnCode.FAILURE)
                            {
                                Console.WriteLine($"Failed to dispatch reactor events: {(m_ErrorInfo != null ? m_ErrorInfo.Error.Text : "")}");
                                Uninitialize();
                                Environment.Exit((int)ReactorReturnCode.FAILURE);
                            }
                        } 
                        else
                        {
                            var reactorChannel = m_SocketChannelMap[socket];
                            m_DispatchOptions.ReactorChannel = reactorChannel;
                            while ((ret = m_Reactor.Dispatch(m_DispatchOptions, out m_ErrorInfo)) > 0) { }
                            if (ret == ReactorReturnCode.FAILURE)
                            {
                                if (reactorChannel.State != ReactorChannelState.CLOSED && reactorChannel.State != ReactorChannelState.DOWN_RECONNECTING)
                                {
                                    Console.WriteLine($"ReactorChannel dispatch failed: {(m_ErrorInfo != null ? m_ErrorInfo.Error.Text : "")}");
                                    Uninitialize();
                                    Environment.Exit((int)ReactorReturnCode.FAILURE);
                                }
                            }
                        }
                    }
                }

                if (currentTime >= nextSendTime)
                {
                    foreach (ChannelInfo chnlInfo in m_ChnlInfoList)
                    {
                        if (chnlInfo.ReactorChannel != null 
                                && chnlInfo.ReactorChannel.State == ReactorChannelState.READY
                                && chnlInfo.LoginRefresh.State.StreamState() == StreamStates.OPEN 
                                && chnlInfo.LoginRefresh.State.DataState() == DataStates.OK)
                        {
                            if (m_RefreshesSent) // send updates since refreshes already sent
                            {
                                SendItemUpdates(chnlInfo);
                            }
                            else // send refreshes first
                            {
                                SendItemRefreshes(chnlInfo);
                                m_RefreshesSent = true;
                            }
                        }
                        nextSendTime += SEND_INTERVAL;
                    }
                }
                currentTime = DateTimeOffset.Now.ToUnixTimeMilliseconds();
            }
        }

        private void SendItemUpdates(ChannelInfo chnlInfo)
        {
            if (chnlInfo.MarketPriceHandler!.SendItemUpdates(chnlInfo.ReactorChannel!, out m_ErrorInfo) != ReactorReturnCode.SUCCESS)
            {
                if (chnlInfo.ReactorChannel!.State != ReactorChannelState.CLOSED 
                    && chnlInfo.ReactorChannel.State != ReactorChannelState.DOWN_RECONNECTING)
                {
                    Console.WriteLine($"Failed to send MarketPrice item updates: {(m_ErrorInfo != null ? m_ErrorInfo.Error.Text : "")}");
                    Uninitialize();
                    Environment.Exit((int)ReactorReturnCode.FAILURE);
                }
            }

            if (chnlInfo.MarketByOrderHandler!.SendItemUpdates(chnlInfo.ReactorChannel!, out m_ErrorInfo) != ReactorReturnCode.SUCCESS)
            {
                if (chnlInfo.ReactorChannel!.State != ReactorChannelState.CLOSED &&
                    chnlInfo.ReactorChannel.State != ReactorChannelState.DOWN_RECONNECTING)
                {
                    Console.WriteLine($"Failed to send MarketByOrder item updates: {(m_ErrorInfo != null ? m_ErrorInfo.Error.Text : "")}");
                    Uninitialize();
                    Environment.Exit((int)ReactorReturnCode.FAILURE);
                }
            }
        }

        private void SendItemRefreshes(ChannelInfo chnlInfo)
        {
            if (chnlInfo.MarketPriceHandler!.SendItemRefreshes(chnlInfo.ReactorChannel!, chnlInfo.MpItemList, m_DefaultServiceId, out m_ErrorInfo) != ReactorReturnCode.SUCCESS)
            {
                if (chnlInfo.ReactorChannel!.State != ReactorChannelState.CLOSED && chnlInfo.ReactorChannel.State != ReactorChannelState.DOWN_RECONNECTING)
                {
                    Console.WriteLine($"Failed to send MarketPrice item refreshes: {(m_ErrorInfo != null ? m_ErrorInfo.Error.Text : "")}");
                    Uninitialize();
                    Environment.Exit((int)ReactorReturnCode.FAILURE);
                }
            }

            if (chnlInfo.MarketByOrderHandler!.SendItemRefreshes(chnlInfo.ReactorChannel!, chnlInfo.MboItemList, m_DefaultServiceId, out m_ErrorInfo) != ReactorReturnCode.SUCCESS)
            {
                if (chnlInfo.ReactorChannel!.State != ReactorChannelState.CLOSED && chnlInfo.ReactorChannel.State != ReactorChannelState.DOWN_RECONNECTING)
                {
                    Console.WriteLine($"Failed to send MarketByOrder item refreshes: {(m_ErrorInfo != null ? m_ErrorInfo.Error.Text : "")}");
                    Uninitialize();
                    Environment.Exit((int)ReactorReturnCode.FAILURE);
                }
            }
        }

        private void Uninitialize()
        {
            Console.WriteLine("NIProvider unitializing and exiting...");

            foreach (ChannelInfo chnlInfo in m_ChnlInfoList)
            {
                // close items streams
                CloseItemStreams(chnlInfo);

                // close ReactorChannel
                if (chnlInfo.ReactorChannel != null)
                {
                    if (chnlInfo.ReactorChannel.Close(out m_ErrorInfo) < ReactorReturnCode.SUCCESS)
                    {
                        Console.WriteLine($"Failed to close ReactorChannel {chnlInfo.ReactorChannel}: {(m_ErrorInfo != null ? m_ErrorInfo.Error.Text : "")}");
                    }
                }
            }

            // shutdown reactor
            m_Reactor!.Shutdown(out m_ErrorInfo);
        }

        private void CloseItemStreams(ChannelInfo chnlInfo)
        {
            // close item streams if opened
            if (chnlInfo.ReactorChannel is not null)
            {
                chnlInfo.MarketPriceHandler!.CloseStreams(chnlInfo.ReactorChannel);
                chnlInfo.MarketByOrderHandler!.CloseStreams(chnlInfo.ReactorChannel);
            }
        }

        public ReactorCallbackReturnCode ReactorChannelEventCallback(ReactorChannelEvent reactorEvent)
        {
            ChannelInfo? chnlInfo = (ChannelInfo?)reactorEvent.ReactorChannel!.UserSpecObj;

            switch (reactorEvent.EventType)
            {
                case ReactorChannelEventType.CHANNEL_UP:

                    m_SocketChannelMap.Add(reactorEvent.ReactorChannel.Socket!, reactorEvent.ReactorChannel);
                    m_Sockets.Add(reactorEvent.ReactorChannel.Socket!);

                    int rcvBufSize = 65535;
                    int sendBufSize = 65535;
                    /* Change size of send/receive buffer since it's small by default on some platforms */
                    if (reactorEvent.ReactorChannel.IOCtl(IOCtlCode.SYSTEM_WRITE_BUFFERS, sendBufSize, out m_ErrorInfo) != ReactorReturnCode.SUCCESS)
                    {
                        Console.WriteLine($"channel.IOCtl() failed: {m_ErrorInfo?.Error.Text}");
                        return ReactorCallbackReturnCode.SUCCESS;
                    }
    
                    if (reactorEvent.ReactorChannel.IOCtl(IOCtlCode.SYSTEM_READ_BUFFERS, rcvBufSize, out m_ErrorInfo) != ReactorReturnCode.SUCCESS)
                    {
                        Console.WriteLine($"channel.IOCtl() failed: {m_ErrorInfo?.Error.Text}");
                        return ReactorCallbackReturnCode.SUCCESS;
                    }
                    break;
                case ReactorChannelEventType.FD_CHANGE:

                    m_SocketChannelMap.Remove(reactorEvent.ReactorChannel.OldSocket!);
                    m_Sockets.Remove(reactorEvent.ReactorChannel.OldSocket!);

                    Console.WriteLine($"Channel Change - Old Channel: {reactorEvent.ReactorChannel.OldSocket!.Handle.ToInt64()}\n" +
                        $" New Channel: {reactorEvent.ReactorChannel.Socket!.Handle.ToInt64()}");
                    break;
                case ReactorChannelEventType.CHANNEL_READY:
                    
                    chnlInfo!.ReactorChannel = reactorEvent.ReactorChannel;
                    Console.WriteLine("\nConnection is ready, starting publishing");
                    break;
                case ReactorChannelEventType.CHANNEL_DOWN_RECONNECTING:
                    
                    if (reactorEvent.ReactorChannel.Socket != null)
                    {
                        Console.WriteLine($"\nConnection down reconnecting: Channel {reactorEvent.ReactorChannel.Socket.Handle.ToInt64()}");
                    }
                    else
                    {
                        Console.WriteLine("\nConnection down reconnecting");
                    }
    
                    if (reactorEvent.ReactorErrorInfo != null && reactorEvent.ReactorErrorInfo.Error.Text != null)
                    {
                        Console.WriteLine($"    Error text: {reactorEvent.ReactorErrorInfo.Error.Text}\n");
                    }

                    m_RefreshesSent = false;
                    chnlInfo!.CanSendLoginReissue = false;
  
                    if (reactorEvent.ReactorChannel.Socket != null)
                    {
                        m_SocketChannelMap.Remove(reactorEvent.ReactorChannel.Socket);
                        m_Sockets.Remove(reactorEvent.ReactorChannel.Socket);
                    }

                    if (chnlInfo.NiProviderRole.DictionaryDownloadMode == DictionaryDownloadMode.FIRST_AVAILABLE) 
                    {
                        if (m_Dictionary != null) 
                        {
                            m_Dictionary.Clear();
                        }
                    }
                    break;
                case ReactorChannelEventType.CHANNEL_DOWN:
                    if (reactorEvent.ReactorChannel.Socket != null)
                        Console.WriteLine($"\nConnection down: Channel {reactorEvent.ReactorChannel.Socket.Handle.ToInt64()}");
                    else
                        Console.WriteLine("\nConnection down");
    
                    if (reactorEvent.ReactorErrorInfo != null && reactorEvent.ReactorErrorInfo.Error.Text != null)
                        Console.WriteLine($"    Error text: {reactorEvent.ReactorErrorInfo.Error.Text}\n");

                    if (reactorEvent.ReactorChannel.Socket != null)
                    {
                        m_SocketChannelMap.Remove(reactorEvent.ReactorChannel.Socket);
                        m_Sockets.Remove(reactorEvent.ReactorChannel.Socket);
                    }

                    // close ReactorChannel
                    if (chnlInfo!.ReactorChannel != null)
                    {
                        chnlInfo.ReactorChannel.Close(out m_ErrorInfo);
                    }
                    break;
                default:
                    break;
            }

            return ReactorCallbackReturnCode.SUCCESS;
        }

        public ReactorCallbackReturnCode DefaultMsgCallback(ReactorMsgEvent msgEvent)
        {
            if (msgEvent.Msg == null)
            {   
                Console.WriteLine($"DefaultMsgCallback received error: {msgEvent.ReactorErrorInfo.Error.Text} at {msgEvent.ReactorErrorInfo.Location}\n");
            }
            return ReactorCallbackReturnCode.SUCCESS;
        }

        public ReactorCallbackReturnCode RdmLoginMsgCallback(RDMLoginMsgEvent loginEvent)
        {
            ChannelInfo? chnlInfo = (ChannelInfo?)loginEvent.ReactorChannel!.UserSpecObj;
            LoginMsg? loginMsg = loginEvent.LoginMsg;

            if (loginMsg == null)
            {
                Console.WriteLine($"LoginMsgCallback received error: {loginEvent.ReactorErrorInfo.Error.Text} at {loginEvent.ReactorErrorInfo.Location}\n");
           
                return ReactorCallbackReturnCode.SUCCESS;
            }

            LoginMsgType msgType = loginEvent.LoginMsg!.LoginMsgType;
            switch (msgType)
            {
                case LoginMsgType.REFRESH:
                    Console.WriteLine($"Received Login Refresh for Username: {loginEvent.LoginMsg!.LoginRefresh!.UserName}");
                    Console.WriteLine(loginEvent.LoginMsg.ToString());

                    // save loginRefresh
                    loginEvent.LoginMsg.LoginRefresh.Copy(chnlInfo!.LoginRefresh);

                    // get login reissue time from authenticationTTReissue
                    if (chnlInfo.LoginRefresh.HasAuthenicationTTReissue)
                    {
                        chnlInfo.LoginReissueTime = chnlInfo.LoginRefresh.AuthenticationTTReissue * 1000;
                        chnlInfo.CanSendLoginReissue = true;
                    }
                    break;
                case LoginMsgType.STATUS:
                    LoginStatus loginStatus = loginEvent.LoginMsg.LoginStatus!;
                    Console.WriteLine("Received Login StatusMsg");
                    if (loginStatus.HasState)
                    {
                        Console.WriteLine("	" + loginStatus.State);
                    }
                    break;
                default:
                Console.WriteLine($"Received Unhandled Login Msg Type: {msgType}");
        break;
    }
        return ReactorCallbackReturnCode.SUCCESS;
    }

        public ReactorCallbackReturnCode RdmDictionaryMsgCallback(RDMDictionaryMsgEvent msgEvent)
        {
            ChannelInfo? chnlInfo = (ChannelInfo?)msgEvent.ReactorChannel!.UserSpecObj;
            DictionaryMsgType msgType = msgEvent.DictionaryMsg!.DictionaryMsgType;

            switch (msgType)
            {
                case DictionaryMsgType.REFRESH:
                    DictionaryRefresh dictionaryRefresh = msgEvent.DictionaryMsg.DictionaryRefresh!;

                    if (dictionaryRefresh.HasInfo)
                    {
                        /* The first part of a dictionary refresh should contain information about its type.
                         * Save this information and use it as subsequent parts arrive. */
                        switch (dictionaryRefresh.DictionaryType)
                        {
                            case Dictionary.Types.FIELD_DEFINITIONS:
                                chnlInfo!.fieldDictionaryStreamId = dictionaryRefresh.StreamId;
                                break;
                            case Dictionary.Types.ENUM_TABLES:
                                chnlInfo!.enumDictionaryStreamId = dictionaryRefresh.StreamId;
                                break;
                            default:
                                Console.WriteLine($"Unknown dictionary type {dictionaryRefresh.DictionaryType} from message on stream {dictionaryRefresh.StreamId}");
                                chnlInfo!.ReactorChannel!.Close(out m_ErrorInfo);
                                return ReactorCallbackReturnCode.SUCCESS;
                        }
                    }

                    // decode dictionary response

                    // clear decode iterator
                    chnlInfo!.DecodeIter.Clear();

                    // set buffer and version info
                    chnlInfo.DecodeIter.SetBufferAndRWFVersion(dictionaryRefresh.DataBody,
                            msgEvent.ReactorChannel.MajorVersion,
                            msgEvent.ReactorChannel.MinorVersion);

                    Console.WriteLine($"Received Dictionary Response: {dictionaryRefresh.DictionaryName}");

                    if (dictionaryRefresh.StreamId == chnlInfo.fieldDictionaryStreamId)
                    {
                        if (m_Dictionary.DecodeFieldDictionary(chnlInfo.DecodeIter, Dictionary.VerbosityValues.VERBOSE, out CodecError error) == CodecReturnCode.SUCCESS)
                        {
                            if (dictionaryRefresh.RefreshComplete)
                            {
                                Console.WriteLine("Field Dictionary complete.");
                            }
                        }
                        else
                        {
                            Console.WriteLine($"Decoding Field Dictionary failed, error: {error?.Text}");
                            chnlInfo.ReactorChannel!.Close(out m_ErrorInfo);
                        }
                    }
                    else if (dictionaryRefresh.StreamId == chnlInfo.enumDictionaryStreamId)
                    {
                        if (m_Dictionary.DecodeEnumTypeDictionary(chnlInfo.DecodeIter, Dictionary.VerbosityValues.VERBOSE, out CodecError error) == CodecReturnCode.SUCCESS)
                        {
                            if (dictionaryRefresh.RefreshComplete)
                            {
                                Console.WriteLine("EnumType Dictionary complete.");
                            }
                        }
                        else
                        {
                            Console.WriteLine($"Decoding EnumType Dictionary failed: {error.Text}");
                            chnlInfo.ReactorChannel!.Close(out m_ErrorInfo);
                        }
                    }
                    else
                    {
                        Console.WriteLine($"Received unexpected dictionary message on stream {dictionaryRefresh.StreamId}");
                    }
                    break;
                case DictionaryMsgType.STATUS:
                    Console.WriteLine("Received Dictionary StatusMsg");
                    break;
                default:
                    Console.WriteLine($"Received Unhandled Dictionary Msg Type: {msgType}");
                    break;
            }
            return ReactorCallbackReturnCode.SUCCESS;
        }

        public static void Main(string[] args) 
        {
            VANiProvider niprovider = new VANiProvider();
            niprovider.Init(args);
            niprovider.Run();
            niprovider.Uninitialize();
            Environment.Exit(0);
        }
    }
}
