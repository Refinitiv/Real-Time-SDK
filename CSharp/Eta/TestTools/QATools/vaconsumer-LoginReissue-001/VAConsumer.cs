/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2023-2024 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

using System.Net.Sockets;

using LSEG.Eta.Codec;
using LSEG.Eta.Example.VACommon;
using LSEG.Eta.Rdm;
using LSEG.Eta.Transports;
using LSEG.Eta.ValueAdd.Rdm;
using LSEG.Eta.ValueAdd.Reactor;

namespace LSEG.Eta.ValueAdd.Consumer
{
    /// <summary>
    /// This is a main class to run the ETA Value Add Consumer application.
    /// </summary>
    ///
    /// <H2>Summary</H2>
    /// <p>
    /// The purpose of this application is to demonstrate consuming data from
    /// an OMM Provider using Value Add components. It is a single-threaded
    /// client application.
    /// </p>
    /// <p>
    /// The consumer application implements callbacks that process information
    /// received by the provider. It creates the Reactor, creates the desired
    /// connections, then dispatches from the Reactor for events and messages.
    /// Once it has received the event indicating that the channel is ready,
    /// it will make the desired item requests (snapshot or streaming) to a
    /// provider and appropriately processes the responses. The resulting decoded
    /// responses from the provided are displayed on the console.
    /// </p>
    /// <p>
    /// This application supports consuming Level I Market Price, Level II Market By
    /// Order, Level II Market By Price and Yield Curve. This application can optionally
    /// perform on-stream and off-stream posting for Level I Market Price content. The
    /// item name used for an off-stream post is "OFFPOST". For simplicity, the off-stream
    /// post item name is not configurable, but users can modify the code if desired.
    /// </p>
    /// <p>
    /// If multiple item requests are specified on the command line for the same domain and
    /// the provider supports batch requests, this application will send the item requests
    /// as a single Batch request.
    /// </p>
    /// <p>
    /// If supported by the provider and the application requests view use, a dynamic
    /// view will be requested with all Level I Market Price requests. For simplicity,
    /// this view is not configurable but users can modify the code to change the
    /// requested view.
    /// </p>
    /// <p>
    /// This application supports a symbol list request. The symbol list name is optional.
    /// If the user does not provide a symbol list name, the name is taken from the source
    /// directory response.
    /// </p>
    /// <p>
    /// This application is intended as a basic usage example. Some of the design choices
    /// were made to favor simplicity and readability over performance. This application
    /// is not intended to be used for measuring performance. This application uses
    /// Value Add and shows how using Value Add simplifies the writing of ETA
    /// applications. Because Value Add is a layer on top of ETA, you may see a
    /// slight decrease in performance compared to writing applications directly to
    /// the ETA interfaces.
    /// </p>
    /// <H2>Setup Environment</H2>
    /// <p>
    /// The RDMFieldDictionary and enumtype.def files could be located in the
    /// directory of execution or this application will request dictionary from
    /// provider.
    /// </p>
    ///
    /// <H2>Running the application:</H2>
    /// <p>
    /// Change directory to the project directory and issue the following <i>dotnet</i> command:
    /// <p>
    /// dotnet run<br/>
    /// or:<br/>
    /// dotnet run -- [arguments]<br/>
    /// <br/>
    ///
    /// Arguments are listed below.
    /// </p>
    /// <ul>
    /// <li>-c specifies a connection to open and a list of items to request:
    /// <ul>
    ///  <li>hostname:        Hostname of provider to connect to
    ///  <li>port:            Port of provider to connect to
    ///  <li>service:         Name of service to request items from on this connection
    ///  <li>domain:itemName: Domain and name of an item to request
    ///       <br>A comma-separated list of these may be specified.
    ///       <br>The domain may be any of: mp(MarketPrice), mbo(MarketByOrder), mbp(MarketByPrice), yc(YieldCurve), sl(SymbolList)
    ///       <br>The domain may also be any of the private stream domains: mpps(MarketPrice PS), mbops(MarketByOrder PS), mbpps(MarketByPrice PS), ycps(YieldCurve PS)
    ///       <br>Example Usage: -c localhost:14002 DIRECT_FEED mp:TRI,mp:GOOG,mpps:FB,mbo:MSFT,mbpps:IBM,sl
    ///       <br>&nbsp;&nbsp;(for SymbolList requests, a name can be optionally specified)
    ///  </li>
    /// </ul>
    /// </li>
    /// <li>-uname changes the username used when logging into the provider
    ///
    /// <li>-passwd changes the password used when logging into the provider
    ///
    /// <li>-view specifies each request using a basic dynamic view
    ///
    /// <li>-post specifies that the application should attempt to send post messages on
    ///           the first requested Market Price item
    ///
    /// <li>-offpost specifies that the application should attempt to send post messages
    ///              on the login stream (i.e., off-stream)
    ///
    /// <li>-publisherInfo specifies that the application provides its own user id and address
    ///
    /// <li>-snapshot specifies each request using non-streaming
    ///
    /// <li>-x provides an XML trace of messages
    ///
    /// <li>-runtime adjusts the running time of the application
    ///
    /// <li>-aid Specifies the Application ID.
    ///
    /// <li>-rtt enables rtt support by a consumer. If provider make distribution of RTT
    ///          messages, consumer will return back them. In another case, consumer will
    ///          ignore them.
    /// </ul>
    ///
    public class VAConsumer : IConsumerCallback , IReactorAuthTokenEventCallback, IReactorOAuthCredentialEventCallback
    {
        private const string FIELD_DICTIONARY_FILE_NAME = "RDMFieldDictionary";
        private const string ENUM_TABLE_FILE_NAME = "enumtype.def";

        /// default server host name
        private const string DEFAULT_SRVR_HOSTNAME = "localhost";

        /// default server port number
        private const string DEFAULT_SRVR_PORT_NO = "14002";

        /// default service name
        private const string DEFAULT_SERVICE_NAME = "DIRECT_FEED";

        /// default item name
        private const string DEFAULT_ITEM_NAME = "TRI.N";

        /// default item name 2
        private const string DEFAULT_ITEM_NAME2 = ".DJI";

        private Reactor.Reactor? m_Reactor;
        private ReactorOptions m_ReactorOptions = new();
        private ReactorDispatchOptions m_DispatchOptions = new();
        private ReactorOAuthCredential oAuthCredential = new();

        private ConsumerCmdLineParser m_ConsumerCmdLineParser = new();

        private System.DateTime m_Runtime;

        private Error error = new();    // error information

        private DataDictionary m_Dictionary;

        private bool fieldDictionaryLoadedFromFile;
        private bool enumTypeDictionaryLoadedFromFile;

        List<ChannelInfo> chnlInfoList = new List<ChannelInfo>();
        List<Socket> m_ReadSockets = new List<Socket>();

        private TimeSpan m_Closetime;
        private System.DateTime m_CloseRuntime;
        bool m_CloseHandled;

        // APIQA: Adding variables
        int eventCounter = 0;
        int srcdirEventCounter = 0;
        // END APIQA

        private ReactorSubmitOptions m_SubmitOptions = new();

        private FileStream? m_FileStream;

        /// <summary>
        /// Map Socket file descriptor to the corresponding ReactorChannel.
        /// It is needed in C# version of the app because there is no way to attach
        /// payload to Sockets in Select method.
        /// </summary>
        private Dictionary<Int32, ReactorChannel> m_SocketFdValueMap = new();

        public VAConsumer()
        {
            m_Dictionary = new();
            m_DispatchOptions.SetMaxMessages(1);
            m_Closetime = TimeSpan.FromSeconds(10);
        }

        /// <summary>
        /// Initializes the Value Add consumer application.
        /// </summary>
        /// <param name="args"></param>
        private void Init(string[] args)
        {
            // parse command line
            if (!m_ConsumerCmdLineParser.ParseArgs(args))
            {
                Console.Error.WriteLine("\nError loading command line arguments:\n");
                m_ConsumerCmdLineParser.PrintUsage();
                Environment.Exit((int)CodecReturnCode.FAILURE);
            }

            // add default connections to arguments if none specified
            if (m_ConsumerCmdLineParser.ConnectionList.Count == 0 &&
                !m_ConsumerCmdLineParser.EnableSessionMgnt)
            {
                // first connection - localhost:14002 DIRECT_FEED mp:TRI.N
                List<ItemArg> itemList = new List<ItemArg>();
                ItemArg itemArg = new ItemArg(DomainType.MARKET_PRICE, DEFAULT_ITEM_NAME, false);
                itemList.Add(itemArg);
                ConnectionArg connectionArg = new ConnectionArg(ConnectionType.SOCKET,
                                                                DEFAULT_SERVICE_NAME,
                                                                DEFAULT_SRVR_HOSTNAME,
                                                                DEFAULT_SRVR_PORT_NO,
                                                                itemList);
                m_ConsumerCmdLineParser.ConnectionList.Add(connectionArg);

                // second connection - localhost:14002 DIRECT_FEED mp:TRI.N mp:.DJI
                List<ItemArg> itemList2 = new List<ItemArg>();
                ItemArg itemArg2 = new ItemArg(DomainType.MARKET_PRICE, DEFAULT_ITEM_NAME2, false);
                itemList2.Add(itemArg);
                itemList2.Add(itemArg2);
                ConnectionArg connectionArg2 = new ConnectionArg(ConnectionType.SOCKET,
                                                                 DEFAULT_SERVICE_NAME,
                                                                 DEFAULT_SRVR_HOSTNAME,
                                                                 DEFAULT_SRVR_PORT_NO,
                                                                 itemList2);
                m_ConsumerCmdLineParser.ConnectionList.Add(connectionArg2);
            }

            // display product version information
            Console.WriteLine("Consumer initializing...");
            Console.WriteLine($"Codec version: {Codec.Codec.MajorVersion()}.{Codec.Codec.MinorVersion()}");

            m_Runtime = System.DateTime.Now + m_ConsumerCmdLineParser.Runtime;
            m_CloseRuntime = System.DateTime.Now + (m_ConsumerCmdLineParser.Runtime + m_Closetime);

            // load dictionary
            LoadDictionary();

            // enable Reactor XML tracing if specified
            if (m_ConsumerCmdLineParser.EnableXmlTracing)
            {
                m_ReactorOptions.XmlTracing = true;
            }

            if(!string.IsNullOrEmpty(m_ConsumerCmdLineParser.TokenURLV2))
            {
                m_ReactorOptions.SetTokenServiceURL(m_ConsumerCmdLineParser.TokenURLV2);
            }

            if (m_ConsumerCmdLineParser.EnableRestLogging)
            {
                m_ReactorOptions.EnableRestLogStream = m_ConsumerCmdLineParser.EnableRestLogging;
            }

            if (!string.IsNullOrEmpty(m_ConsumerCmdLineParser.RestLoggingFileName))
            {
                try
                {
                    m_FileStream = new FileStream(m_ConsumerCmdLineParser.RestLoggingFileName, FileMode.Create);
                    m_ReactorOptions.RestLogOutputStream = m_FileStream;
                }
                catch (Exception ex)
                {
                    Console.WriteLine($"Failed to create a FileStream with error text: {ex.Message}");
                    Environment.Exit((int)ReactorReturnCode.FAILURE);
                }
            }

            // create reactor
            m_Reactor = Reactor.Reactor.CreateReactor(m_ReactorOptions, out var errorInfo);
            if ((errorInfo != null
                 && errorInfo.Code != ReactorReturnCode.SUCCESS)
                || m_Reactor == null)
            {
                Console.WriteLine("CreateReactor() failed: " + errorInfo?.ToString());
                Environment.Exit((int)ReactorReturnCode.FAILURE);
            }

            m_ReadSockets.Add(m_Reactor.EventSocket!);

            /* create channel info, initialize channel info, and connect channels
             * for each connection specified */
            foreach (ConnectionArg connectionArg in m_ConsumerCmdLineParser.ConnectionList)
            {
                // create channel info
                ChannelInfo chnlInfo = new ChannelInfo(connectionArg);

                // initialize channel info
                InitChannelInfo(chnlInfo);

                // connect channel
                ReactorReturnCode ret;
                if ((ret = m_Reactor.Connect(chnlInfo.ConnectOptions, chnlInfo.ConsumerRole, out var connectErrorInfo)) < ReactorReturnCode.SUCCESS)
                {
                    Console.WriteLine($"Reactor.Connect failed with return code: {ret} error = {connectErrorInfo?.Error.Text}");
                    Environment.Exit((int)ReactorReturnCode.FAILURE);
                }

                // add to ChannelInfo list
                chnlInfoList.Add(chnlInfo);
            }
        }

        /// <summary>
        /// Runs the Value Add consumer application.
        /// </summary>
        private void Run()
        {
            int selectTime = 1000 * 1000; // 1 second in microseconds
            while (true)
            {
                List<Socket> sockReadList = new List<Socket>(m_ReadSockets);
                List<Socket> sockErrList = new List<Socket>(m_ReadSockets);

                try
                {
                    Socket.Select(sockReadList, null, sockErrList, selectTime);
                }
                catch (Exception e)
                {
                    Console.WriteLine($"Select failed: {e.Message}");
                    Environment.Exit((int)ReactorReturnCode.FAILURE);
                }

                // nothing to read
                if (sockReadList.Count > 0)
                {
                    foreach (var sock in sockReadList)
                    {
                        if (sock == m_Reactor!.EventSocket)
                        {
                            // dispatch until no more messages
                            m_DispatchOptions.ReactorChannel = null;
                            ReactorReturnCode ret;
                            ReactorErrorInfo? errorInfo;
                            while ((ret = m_Reactor.Dispatch(m_DispatchOptions, out errorInfo)) > ReactorReturnCode.SUCCESS)
                            { }

                            // Graceful shutdown if Dispatch fails
                            if (ret != ReactorReturnCode.SUCCESS)
                            {
                                Console.WriteLine(errorInfo?.Error.Text);
                                Uninitialize();
                                Environment.Exit((int)ReactorReturnCode.FAILURE);
                            }
                        }
                        else
                        {
                            // retrieve associated reactor channel and dispatch on that channel
                            ReactorChannel reactorChnl = GetChannelBySocketFd(sock.Handle.ToInt32());

                            ReactorReturnCode ret;
                            ReactorErrorInfo? errorInfo;

                            // dispatch until no more messages
                            m_DispatchOptions.ReactorChannel = reactorChnl;
                            while ((ret = m_Reactor.Dispatch(m_DispatchOptions, out errorInfo)) > ReactorReturnCode.SUCCESS)
                            { }

                            if (ret != ReactorReturnCode.SUCCESS)
                            {
                                if (reactorChnl.State != ReactorChannelState.CLOSED
                                    && reactorChnl.State != ReactorChannelState.DOWN_RECONNECTING)
                                {
                                    Console.WriteLine($"ReactorChannel dispatch failed: {ret} ({errorInfo?.Error?.Text})");
                                    Uninitialize();
                                    Environment.Exit((int)ReactorReturnCode.FAILURE);
                                }
                            }
                        }
                    }
                }

                // Handle run-time
                if (System.DateTime.Now >= m_Runtime && !m_CloseHandled)
                {
                    Console.WriteLine("Consumer run-time expired, close now...");
                    HandleClose();
                    m_CloseHandled = true;
                }
                else if (System.DateTime.Now >= m_CloseRuntime)
                {
                    Console.WriteLine("Consumer closetime expired, shutdown reactor.");
                    break;
                }
                if (!m_CloseHandled)
                {
                    HandlePosting();
                    HandleTunnelStream();

                    // send login reissue if login reissue time has passed
                    foreach (ChannelInfo chnlInfo in chnlInfoList)
                    {
                        if (chnlInfo.ReactorChannel == null
                            || (chnlInfo.ReactorChannel.State != ReactorChannelState.UP
                                && chnlInfo.ReactorChannel.State != ReactorChannelState.READY))
                        {
                            continue;
                        }

                        if (chnlInfo.CanSendLoginReissue
                            && (m_ConsumerCmdLineParser.EnableSessionMgnt == false)
                            && System.DateTime.Now >= chnlInfo.LoginReissueTime)
                        {
                            LoginRequest loginRequest = chnlInfo.ConsumerRole.RdmLoginRequest!;
                            m_SubmitOptions.Clear();
                            if (chnlInfo.ReactorChannel.Submit(loginRequest, m_SubmitOptions, out var errorInfo) != ReactorReturnCode.SUCCESS)
                            {
                                Console.WriteLine("Login reissue failed. Error: " + errorInfo?.Error.Text);
                            }
                            else
                            {
                                Console.WriteLine("Login reissue sent");
                            }
                            chnlInfo.CanSendLoginReissue = false;
                        }
                    }
                }

                if (m_CloseHandled)
                    break;
            }
        }


        public ReactorCallbackReturnCode ReactorChannelEventCallback(ReactorChannelEvent evt)
        {
            ChannelInfo? chnlInfo = evt.ReactorChannel?.UserSpecObj as ChannelInfo;
            if (chnlInfo == null)
                return ReactorCallbackReturnCode.FAILURE;

            switch (evt.EventType)
            {
                case ReactorChannelEventType.CHANNEL_UP:
                    {
                        if (evt.ReactorChannel?.Socket != null)
                            Console.WriteLine("Channel Up Event: " + evt.ReactorChannel.Socket.Handle.ToInt32());
                        else
                            Console.WriteLine("Channel Up Event");

                        // register selector with channel event's reactorChannel
                        RegisterChannel(evt.ReactorChannel!);

                        break;
                    }
                case ReactorChannelEventType.FD_CHANGE:
                    {
                        int fdOldSocketId = evt.ReactorChannel!.OldSocket!.Handle.ToInt32();
                        int fdSocketId = evt.ReactorChannel!.Socket!.Handle.ToInt32();

                        Console.WriteLine($"Channel Change - Old Channel: {fdOldSocketId} New Channel: {fdSocketId}");

                        // cancel old reactorChannel select
                        UnregisterSocket(evt.ReactorChannel.OldSocket);

                        // register selector with channel event's new reactorChannel
                        RegisterChannel(evt.ReactorChannel);

                        break;
                    }
                case ReactorChannelEventType.CHANNEL_READY:
                    {
                        // set ReactorChannel on ChannelInfo
                        chnlInfo.ReactorChannel = evt.ReactorChannel;
                        if (evt.ReactorChannel?.Socket != null)
                            Console.WriteLine("Channel Ready Event: " + evt.ReactorChannel.Socket.Handle.ToInt32());
                        else
                            Console.WriteLine("Channel Ready Event");

                        if (IsRequestedServiceUp(chnlInfo))
                        {
                            CheckAndInitPostingSupport(chnlInfo);

                            if (!chnlInfo.ItemWatchList.IsEmpty)
                            {
                                chnlInfo.ItemWatchList.Clear();
                            }

                            SendMPRequests(chnlInfo);
                            SendMBORequests(chnlInfo);
                            SendMBPRequests(chnlInfo);
                            SendSymbolListRequests(chnlInfo);
                            SendYieldCurveRequests(chnlInfo);
                            chnlInfo.RequestsSent = true;
                        }

                        break;
                    }
                case ReactorChannelEventType.CHANNEL_DOWN_RECONNECTING:
                    {
                        if (evt.ReactorChannel?.Socket != null)
                            Console.WriteLine("\nConnection down reconnecting: Channel " + evt.ReactorChannel.Socket.Handle.ToInt32());
                        else
                            Console.WriteLine("\nConnection down reconnecting");

                        if (evt.ReactorErrorInfo != null && evt.ReactorErrorInfo.Error.Text != null)
                            Console.WriteLine("\tError text: " + evt.ReactorErrorInfo.Error.Text + "\n");

                        // allow Reactor to perform connection recovery

                        // unregister selectableChannel from Selector
                        if (evt.ReactorChannel?.Socket != null)
                        {
                            UnregisterSocket(evt.ReactorChannel.Socket);
                        }

                        // reset dictionary if not loaded from file
                        if (!fieldDictionaryLoadedFromFile
                            && !enumTypeDictionaryLoadedFromFile)
                        {
                            chnlInfo.Dictionary?.Clear();
                        }

                        // reset item request(s) sent flag
                        chnlInfo.RequestsSent = false;

                        // reset hasServiceInfo flag
                        chnlInfo.HasServiceInfo = false;

                        // reset canSendLoginReissue flag
                        chnlInfo.CanSendLoginReissue = false;

                        SetItemState(chnlInfo, StreamStates.CLOSED_RECOVER, DataStates.SUSPECT, StateCodes.NONE);

                        break;
                    }
                case ReactorChannelEventType.CHANNEL_DOWN:
                    {
                        if (evt.ReactorChannel!.Socket != null)
                            Console.WriteLine("\nConnection down: Channel " + evt.ReactorChannel.Socket.Handle.ToInt32());
                        else
                            Console.WriteLine("\nConnection down");

                        if (evt.ReactorErrorInfo != null && evt.ReactorErrorInfo.Error.Text != null)
                            Console.WriteLine("    Error text: " + evt.ReactorErrorInfo.Error.Text + "\n");

                        // unregister selectableChannel from Selector
                        if (evt.ReactorChannel!.Socket != null)
                        {
                            m_ReadSockets.Remove(evt.ReactorChannel.Socket);
                            m_SocketFdValueMap.Remove(evt.ReactorChannel.Socket.Handle.ToInt32());
                        }

                        // close ReactorChannel
                        if (chnlInfo.ReactorChannel != null)
                        {
                            chnlInfo.ReactorChannel.Close(out _);
                        }
                        break;
                    }
                case ReactorChannelEventType.WARNING:
                    Console.WriteLine("Received ReactorChannel WARNING event.");
                    if (evt.ReactorErrorInfo != null && evt.ReactorErrorInfo.Error.Text != null)
                        Console.WriteLine("    Error text: " + evt.ReactorErrorInfo.Error.Text + "\n");

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
            ChannelInfo? chnlInfo = evt.ReactorChannel?.UserSpecObj as ChannelInfo;
            Msg? msg = evt.Msg as Msg;

            if (msg == null)
            {
                /* The message is not present because an error occurred while decoding it. Print
                 * the error and close the channel. If desired, the un-decoded message buffer
                 * is available in event.transportBuffer(). */
                Console.WriteLine("DefaultMsgCallback: {0}({1})",
                    evt.ReactorErrorInfo.Error.Text, evt.ReactorErrorInfo.Location);

                // unregister selectableChannel from Selector
                if (evt.ReactorChannel!.Socket != null)
                {
                    m_ReadSockets.Remove(evt.ReactorChannel.Socket);
                    m_SocketFdValueMap.Remove(evt.ReactorChannel.Socket.Handle.ToInt32());
                }

                // close ReactorChannel
                if (chnlInfo!.ReactorChannel != null)
                {
                    chnlInfo.ReactorChannel.Close(out _);
                }
                return ReactorCallbackReturnCode.SUCCESS;
            }

            // set response message
            chnlInfo!.ResponseMsg = msg;

            // set-up decode iterator if message has message body
            if (msg.EncodedDataBody != null
                && msg.EncodedDataBody.Data() != null)
            {
                // clear decode iterator
                chnlInfo.DecodeIter.Clear();

                // set buffer and version info
                chnlInfo.DecodeIter.SetBufferAndRWFVersion(msg.EncodedDataBody,
                    evt.ReactorChannel!.MajorVersion,
                    evt.ReactorChannel.MinorVersion);
            }

            ProcessResponse(chnlInfo);

            return ReactorCallbackReturnCode.SUCCESS;
        }

        public ReactorCallbackReturnCode RdmLoginMsgCallback(RDMLoginMsgEvent evt)
        {
            ChannelInfo? chnlInfo = evt.ReactorChannel?.UserSpecObj as ChannelInfo;
            LoginMsgType msgType = evt.LoginMsg!.LoginMsgType;

            if (chnlInfo == null)
                return ReactorCallbackReturnCode.FAILURE;

            switch (msgType)
            {
                case LoginMsgType.REFRESH:
                    Console.WriteLine("Received Login Refresh for Username: " + evt.LoginMsg.LoginRefresh!.UserName);
                    Console.WriteLine(evt.LoginMsg.ToString());

                    // save loginRefresh
                    evt.LoginMsg.LoginRefresh.Copy(chnlInfo.LoginRefresh);

                    // set login stream id in MarketPriceHandler and YieldCurveHandler
                    chnlInfo.MarketPriceHandler.LoginStreamId = evt.LoginMsg.LoginRefresh.StreamId;
                    chnlInfo.YieldCurveHandler.LoginStreamId = evt.LoginMsg.LoginRefresh.StreamId;

                    // get login reissue time from authenticationTTReissue
                    if (chnlInfo.LoginRefresh.HasAuthenicationTTReissue)
                    {
                        chnlInfo.LoginReissueTime = DateTimeOffset.FromUnixTimeSeconds(chnlInfo.LoginRefresh.AuthenticationTTReissue).DateTime;
                        chnlInfo.CanSendLoginReissue = true;
                    }
                    break;

                case LoginMsgType.STATUS:
                    LoginStatus loginStatus = evt.LoginMsg.LoginStatus!;
                    Console.WriteLine("Received Login StatusMsg");
                    if (loginStatus.HasState)
                    {
                        Console.WriteLine("\t" + loginStatus.State);
                    }
                    break;

                case LoginMsgType.RTT:
                    LoginRTT loginRTT = evt.LoginMsg.LoginRTT!;
                    Console.Write("\nReceived login RTT message from Provider {0}.\n",
                        evt.ReactorChannel!.Socket!.Handle.ToInt32());
                    Console.Write("\tTicks: {0}\n", Math.Round((double)loginRTT.Ticks / 1000));
                    if (loginRTT.HasRTLatency)
                    {
                        Console.Write("\tLast Latency: {0}\n", Math.Round((double)loginRTT.RTLatency / 1000));
                    }
                    if (loginRTT.HasTCPRetrans)
                    {
                        Console.Write("\tProvider side TCP Retransmissions: {0}\n", loginRTT.TCPRetrans);
                    }
                    Console.WriteLine("RTT Response sent to provider by reactor.\n");
                    break;

                default:
                    Console.WriteLine($"Received Unhandled Login Msg Type: {msgType}");
                    break;
            }

            return ReactorCallbackReturnCode.SUCCESS;
        }

        public ReactorCallbackReturnCode RdmDirectoryMsgCallback(RDMDirectoryMsgEvent evt)
        {
            ChannelInfo? chnlInfo = evt.ReactorChannel?.UserSpecObj as ChannelInfo;
            DirectoryMsgType msgType = evt.DirectoryMsg!.DirectoryMsgType;

            if (chnlInfo == null)
                return ReactorCallbackReturnCode.FAILURE;

            switch (msgType)
            {
                case DirectoryMsgType.REFRESH:
                    DirectoryRefresh directoryRefresh = evt.DirectoryMsg.DirectoryRefresh!;
                    ProcessServiceRefresh(directoryRefresh, chnlInfo);
                    if (chnlInfo.ServiceInfo.Action == MapEntryActions.DELETE)
                    {
                        error.Text = "RdmDirectoryMsgCallback(): DirectoryRefresh Failed: directory service is deleted";
                        return ReactorCallbackReturnCode.SUCCESS;
                    }
                    break;
                case DirectoryMsgType.UPDATE:
                    DirectoryUpdate directoryUpdate = evt.DirectoryMsg.DirectoryUpdate!;
                    ProcessServiceUpdate(directoryUpdate, chnlInfo);
                    if (chnlInfo.ServiceInfo.Action == MapEntryActions.DELETE)
                    {
                        error.Text = "RdmDirectoryMsgCallback(): DirectoryUpdate Failed: directory service is deleted";
                        return ReactorCallbackReturnCode.SUCCESS;
                    }
                    if (IsRequestedServiceUp(chnlInfo) && !chnlInfo.RequestsSent)
                    {
                        CheckAndInitPostingSupport(chnlInfo);

                        chnlInfo.ItemWatchList.Clear();

                        SendMPRequests(chnlInfo);
                        SendMBORequests(chnlInfo);
                        SendMBPRequests(chnlInfo);
                        SendSymbolListRequests(chnlInfo);
                        SendYieldCurveRequests(chnlInfo);
                        chnlInfo.RequestsSent = true;
                    }
                    // APIQA:
                    srcdirEventCounter++;
                    if (srcdirEventCounter == 1)
                    {
                        // Do a RESUME
                        LoginRequest loginRequest = chnlInfo.ConsumerRole.RdmLoginRequest!;
                        loginRequest.Flags = loginRequest.Flags & ~LoginRequestFlags.PAUSE_ALL;
                        loginRequest.Flags = loginRequest.Flags & ~LoginRequestFlags.NO_REFRESH;
                        m_SubmitOptions.Clear();
                        if (chnlInfo.ReactorChannel.Submit(loginRequest, m_SubmitOptions, out var errorInfo) != ReactorReturnCode.SUCCESS)
                        {
                            Console.WriteLine("APIQA: Submit failed when attempting to send RESUME ALL. Error: " + errorInfo.Error.Text);
                        }
                        else
                        {
                            Console.WriteLine("APIQA: sending RESUME ALL");
                        }
                    }
                    // END APIQA:

                    break;
                case DirectoryMsgType.CLOSE:
                    Console.WriteLine("Received Source Directory Close");
                    break;
                case DirectoryMsgType.STATUS:
                    DirectoryStatus directoryStatus = evt.DirectoryMsg.DirectoryStatus!;
                    Console.WriteLine("\nReceived Source Directory StatusMsg");
                    if (directoryStatus.HasState)
                    {
                        Console.WriteLine("\t" + directoryStatus.State);
                    }
                    break;
                default:
                    Console.WriteLine("Received Unhandled Source Directory Msg Type: " + msgType);
                    break;
            }

            return ReactorCallbackReturnCode.SUCCESS;
        }

        public ReactorCallbackReturnCode RdmDictionaryMsgCallback(RDMDictionaryMsgEvent evt)
        {
            ChannelInfo? chnlInfo = evt.ReactorChannel?.UserSpecObj as ChannelInfo;
            DictionaryMsgType msgType = evt.DictionaryMsg!.DictionaryMsgType;

            if (chnlInfo == null)
                return ReactorCallbackReturnCode.FAILURE;

            // initialize dictionary
            if (chnlInfo.Dictionary == null)
            {
                chnlInfo.Dictionary = m_Dictionary;
            }

            switch (msgType)
            {
                case DictionaryMsgType.REFRESH:
                    DictionaryRefresh dictionaryRefresh = evt.DictionaryMsg.DictionaryRefresh!;

                    if (dictionaryRefresh.HasInfo)
                    {
                        /* The first part of a dictionary refresh should contain information about its type.
                         * Save this information and use it as subsequent parts arrive. */
                        switch (dictionaryRefresh.DictionaryType)
                        {
                            case Dictionary.Types.FIELD_DEFINITIONS:
                                chnlInfo.FieldDictionaryStreamId = dictionaryRefresh.StreamId;
                                break;
                            case Dictionary.Types.ENUM_TABLES:
                                chnlInfo.EnumDictionaryStreamId = dictionaryRefresh.StreamId;
                                break;
                            default:
                                Console.WriteLine($"Unknown dictionary type {dictionaryRefresh.DictionaryType} from message on stream {dictionaryRefresh.StreamId}");
                                chnlInfo.ReactorChannel!.Close(out _);
                                return ReactorCallbackReturnCode.SUCCESS;
                        }
                    }

                    /* decode dictionary response */

                    // clear decode iterator
                    chnlInfo.DecodeIter.Clear();

                    // set buffer and version info
                    chnlInfo.DecodeIter.SetBufferAndRWFVersion(dictionaryRefresh.DataBody,
                                evt.ReactorChannel!.MajorVersion,
                                evt.ReactorChannel.MinorVersion);

                    Console.WriteLine("Received Dictionary Response: " + dictionaryRefresh.DictionaryName);

                    if (dictionaryRefresh.StreamId == chnlInfo.FieldDictionaryStreamId)
                    {
                        if (chnlInfo.Dictionary.DecodeFieldDictionary(chnlInfo.DecodeIter, Dictionary.VerbosityValues.VERBOSE, out _)
                            == CodecReturnCode.SUCCESS)
                        {
                            if (dictionaryRefresh.RefreshComplete)
                            {
                                Console.WriteLine($"Field Dictionary complete.");
                            }
                        }
                        else
                        {
                            Console.WriteLine("Decoding Field Dictionary failed: " + error.Text);
                            chnlInfo.ReactorChannel!.Close(out _);
                        }
                    }
                    else if (dictionaryRefresh.StreamId == chnlInfo.EnumDictionaryStreamId)
                    {
                        if (chnlInfo.Dictionary.DecodeEnumTypeDictionary(chnlInfo.DecodeIter, Dictionary.VerbosityValues.VERBOSE, out _)
                            == CodecReturnCode.SUCCESS)
                        {
                            if (dictionaryRefresh.RefreshComplete)
                            {
                                Console.WriteLine("EnumType Dictionary complete.");
                            }
                        }
                        else
                        {
                            Console.WriteLine("Decoding EnumType Dictionary failed: " + error.Text);
                            chnlInfo.ReactorChannel!.Close(out _);
                        }
                    }
                    else
                    {
                        Console.WriteLine("Received unexpected dictionary message on stream " + dictionaryRefresh.StreamId);
                    }
                    break;
                case DictionaryMsgType.STATUS:
                    Console.WriteLine("Received Dictionary StatusMsg");
                    break;
                default:
                    Console.WriteLine("Received Unhandled Dictionary Msg Type: " + msgType);
                    break;
            }

            return ReactorCallbackReturnCode.SUCCESS;
        }

        public ReactorCallbackReturnCode ReactorAuthTokenEventCallback(ReactorAuthTokenEvent reactorAuthTokenEvent)
        {
            if(reactorAuthTokenEvent.ReactorErrorInfo.Code != ReactorReturnCode.SUCCESS)
            {
                System.Console.WriteLine($"Retrive an access token failed. Text: {reactorAuthTokenEvent.ReactorErrorInfo}");
            }
            else
            {
                System.Console.WriteLine($"Access token information: {reactorAuthTokenEvent.ReactorAuthTokenInfo}");
            }

            return ReactorCallbackReturnCode.SUCCESS;
        }

        public ReactorCallbackReturnCode ReactorOAuthCredentialEventCallback(ReactorOAuthCredentialEvent reactorOAuthCredentialEvent)
        {
            ReactorOAuthCredentialRenewalOptions renewalOptions = new ReactorOAuthCredentialRenewalOptions();
            ReactorOAuthCredentialRenewal reactorOAuthCredentialRenewal = new ReactorOAuthCredentialRenewal();
            ReactorOAuthCredential? reactorOAuthCredential = (ReactorOAuthCredential?)reactorOAuthCredentialEvent.UserSpecObj;

            if (reactorOAuthCredential is not null)
            {
                renewalOptions.RenewalModes = ReactorOAuthCredentialRenewalModes.CLIENT_SECRET;
                reactorOAuthCredentialRenewal.ClientSecret.Data(reactorOAuthCredential.ClientSecret.ToString());

                reactorOAuthCredentialEvent.Reactor!.SubmitOAuthCredentialRenewal(renewalOptions, reactorOAuthCredentialRenewal, out _);
            }

            return ReactorCallbackReturnCode.SUCCESS;
        }

        private void ProcessServiceRefresh(DirectoryRefresh directoryRefresh, ChannelInfo? chnlInfo)
        {
            if (chnlInfo == null)
                return;

            string serviceName = chnlInfo.ConnectionArg.Service;
            Console.WriteLine("Received Source Directory Refresh");
            Console.WriteLine(directoryRefresh.ToString());
            foreach (Service service in directoryRefresh.ServiceList)
            {
                if (service.Action == MapEntryActions.DELETE
                    && service.ServiceId == chnlInfo.ServiceInfo.ServiceId)
                {
                    chnlInfo.ServiceInfo.Action = MapEntryActions.DELETE;
                }

                if (service.Info.ServiceName.ToString() != null)
                {
                    Console.WriteLine($"Received ServiceName: {service.Info.ServiceName}\n");
                    // cache service requested by the application
                    if (service.Info.ServiceName.ToString().Equals(serviceName))
                    {
                        // save serviceInfo associated with requested service name
                        if (service.Copy(chnlInfo.ServiceInfo) < CodecReturnCode.SUCCESS)
                        {
                            Console.WriteLine("Service.Copy() failure");
                            Uninitialize();
                            Environment.Exit((int)ReactorReturnCode.FAILURE);
                        }
                        chnlInfo.HasServiceInfo = true;
                        SetItemState(chnlInfo, service.State.Status.StreamState(), service.State.Status.DataState(),
                                service.State.Status.Code());
                    }
                }
            }
        }

        private void ProcessServiceUpdate(DirectoryUpdate directoryUpdate, ChannelInfo chnlInfo)
        {
            string serviceName = chnlInfo.ConnectionArg.Service;

            Console.WriteLine("Received Source Directory Update");
            Console.WriteLine(directoryUpdate.ToString());

            foreach (Service service in directoryUpdate.ServiceList)
            {
                if (service.Action == MapEntryActions.DELETE
                    && service.ServiceId == chnlInfo.ServiceInfo.ServiceId)
                {
                    chnlInfo.ServiceInfo.Action = MapEntryActions.DELETE;
                }

                bool updateServiceInfo = false;
                if (service.Info.ServiceName.ToString() != null)
                {
                    Console.WriteLine($"Received ServiceName: {service.Info.ServiceName}\n");
                    // update service cache - assume cache is built with previous refresh message
                    if (service.Info.ServiceName.ToString().Equals(serviceName)
                        || service.ServiceId == chnlInfo.ServiceInfo.ServiceId)
                    {
                        updateServiceInfo = true;
                    }
                }
                else
                {
                    if (service.ServiceId == chnlInfo.ServiceInfo.ServiceId)
                    {
                        updateServiceInfo = true;
                    }
                }

                if (updateServiceInfo)
                {
                    // update serviceInfo associated with requested service name
                    if (service.Copy(chnlInfo.ServiceInfo) < CodecReturnCode.SUCCESS)
                    {
                        Console.WriteLine("Service.Copy() failure");
                        Uninitialize();
                        Environment.Exit((int)ReactorReturnCode.FAILURE);
                    }
                    chnlInfo.HasServiceInfo = true;
                    SetItemState(chnlInfo, service.State.Status.StreamState(), service.State.Status.DataState(),
                                 service.State.Status.Code());
                }
            }
        }

        public bool IsRequestedServiceUp(ChannelInfo chnlInfo)
        {
            return chnlInfo.HasServiceInfo
                && chnlInfo.ServiceInfo.HasState
                && (!chnlInfo.ServiceInfo.State.HasAcceptingRequests
                    || chnlInfo.ServiceInfo.State.AcceptingRequests == 1)
                && chnlInfo.ServiceInfo.State.ServiceStateVal == 1;
        }

        private void CheckAndInitPostingSupport(ChannelInfo chnlInfo)
        {
            if (!(chnlInfo.ShouldOnStreamPost || chnlInfo.ShouldOffStreamPost))
                return;

            // set up posting if its enabled

            // ensure that provider supports posting - if not, disable posting
            if (!chnlInfo.LoginRefresh.HasFeatures
                || !chnlInfo.LoginRefresh.SupportedFeatures.HasSupportPost
                || chnlInfo.LoginRefresh.SupportedFeatures.SupportOMMPost == 0)
            {
                // provider does not support posting, disable it
                chnlInfo.ShouldOffStreamPost = false;
                chnlInfo.ShouldOnStreamPost = false;
                chnlInfo.PostHandler.EnableOnstreamPost = false;
                chnlInfo.PostHandler.EnableOffstreamPost = false;
                Console.WriteLine("Connected Provider does not support OMM Posting.  Disabling Post functionality.");
                return;
            }

            if (m_ConsumerCmdLineParser.PublisherId != null
                && m_ConsumerCmdLineParser.PublisherAddress != null)
            {
                chnlInfo.PostHandler.SetPublisherInfo(m_ConsumerCmdLineParser.PublisherId, m_ConsumerCmdLineParser.PublisherAddress);
            }

            // This sets up our basic timing so post messages will be sent
            // periodically
            chnlInfo.PostHandler.InitPostHandler();
        }

        // on and off stream posting if enabled
        private void HandlePosting()
        {
            foreach (ChannelInfo chnlInfo in chnlInfoList)
            {
                if (chnlInfo.LoginRefresh == null
                    || chnlInfo.ServiceInfo == null
                    || chnlInfo.ReactorChannel == null
                    || chnlInfo.ReactorChannel.State != ReactorChannelState.READY)
                {
                    continue;
                }

                if (chnlInfo.PostHandler.EnableOnstreamPost)
                {
                    chnlInfo.PostItemName.Clear();
                    int postStreamId = chnlInfo.MarketPriceHandler.GetFirstItem(chnlInfo.PostItemName);
                    if (postStreamId == 0 || chnlInfo.PostItemName.Length == 0)
                        return;

                    chnlInfo.PostHandler.StreamId = postStreamId;
                    chnlInfo.PostHandler.ServiceId = chnlInfo.ServiceInfo.ServiceId;
                    chnlInfo.PostHandler.Dictionary = chnlInfo.Dictionary;
                    chnlInfo.PostHandler.PostItemName.Data(chnlInfo.PostItemName.Data(),
                        chnlInfo.PostItemName.Position, chnlInfo.PostItemName.Length);

                    ReactorReturnCode ret = chnlInfo.PostHandler.HandlePosts(chnlInfo.ReactorChannel, out var errorInfo);
                    if (ret < ReactorReturnCode.SUCCESS)
                        Console.WriteLine("Error posting onstream: " + errorInfo?.ToString());
                }
                if (chnlInfo.PostHandler.EnableOffstreamPost)
                {
                    chnlInfo.PostHandler.StreamId = chnlInfo.LoginRefresh.StreamId;
                    chnlInfo.PostHandler.PostItemName.Data("OFFPOST");
                    chnlInfo.PostHandler.ServiceId = chnlInfo.ServiceInfo.ServiceId;
                    chnlInfo.PostHandler.Dictionary = chnlInfo.Dictionary;

                    ReactorReturnCode ret = chnlInfo.PostHandler.HandlePosts(chnlInfo.ReactorChannel, out var errorInfo);
                    if (ret < ReactorReturnCode.SUCCESS)
                        Console.WriteLine("Error posting offstream: " + errorInfo?.ToString());
                }
            }
        }

        private void HandleTunnelStream()
        {
            foreach (ChannelInfo chnlInfo in chnlInfoList)
            {
                if (chnlInfo.LoginRefresh == null ||
                    chnlInfo.ServiceInfo == null ||
                    chnlInfo.ReactorChannel == null ||
                    chnlInfo.ReactorChannel.State != ReactorChannelState.READY)
                {
                    continue;
                }
            }
        }

        private void ProcessResponse(ChannelInfo chnlInfo)
        {
            switch ((DomainType)chnlInfo.ResponseMsg.DomainType)
            {
                case DomainType.MARKET_PRICE:
                    Console.WriteLine($"(Channel {chnlInfo.ReactorChannel!.Socket!.Handle.ToInt32()}):");
                    ProcessMarketPriceResp(chnlInfo);
                    // APIQA: Send login reissue for PAUSE_ALL
                    eventCounter++;
                    if (eventCounter == 3)
                    {
                        LoginRequest loginRequest = chnlInfo.ConsumerRole.RdmLoginRequest;
                        loginRequest.Flags |= LoginRequestFlags.PAUSE_ALL;
                        loginRequest.Flags |= LoginRequestFlags.NO_REFRESH;
                        m_SubmitOptions.Clear();
                        if ((chnlInfo.ReactorChannel.Submit(loginRequest, m_SubmitOptions, out var errorInfo)) != ReactorReturnCode.SUCCESS)
                        {
                            Console.WriteLine("APIQA: Submit failed when attempting to send PAUSE ALL. Error: " + errorInfo.Error.Text);
                        }
                        else
                        {
                            Console.WriteLine("APIQA: sending PAUSE ALL");
                        }
                    }
                    // END APIQA
                    break;
                case DomainType.MARKET_BY_ORDER:
                    Console.WriteLine($"(Channel {chnlInfo.ReactorChannel!.Socket!.Handle.ToInt32()}):");
                    ProcessMarketByOrderResp(chnlInfo);
                    break;
                case DomainType.MARKET_BY_PRICE:
                    Console.WriteLine($"(Channel {chnlInfo.ReactorChannel!.Socket!.Handle.ToInt32()}):");
                    ProcessMarketByPriceResp(chnlInfo);
                    break;
                case DomainType.SYMBOL_LIST:
                    Console.WriteLine($"(Channel {chnlInfo.ReactorChannel!.Socket!.Handle.ToInt32()}):");
                    ProcessSymbolListResp(chnlInfo);
                    break;
                case DomainType.YIELD_CURVE:
                    Console.WriteLine($"(Channel {chnlInfo.ReactorChannel!.Socket!.Handle.ToInt32()}):");
                    ProcessYieldCurveResp(chnlInfo);
                    break;
                default:
                    Console.WriteLine($"Unhandled Domain Type: {chnlInfo.ResponseMsg.DomainType}");
                    break;
            }
        }

        private void ProcessSymbolListResp(ChannelInfo chnlInfo)
        {
            if (chnlInfo.SymbolListHandler.ProcessResponse(chnlInfo.ResponseMsg, chnlInfo.DecodeIter, chnlInfo.Dictionary, out var errorInfo)
                != ReactorReturnCode.SUCCESS)
            {
                Console.WriteLine(errorInfo?.Error.Text);
                Uninitialize();
                Environment.Exit((int)ReactorReturnCode.FAILURE);
            }
        }

        private void ProcessMarketByPriceResp(ChannelInfo chnlInfo)
        {
            if (chnlInfo.MarketByPriceHandler.ProcessResponse(chnlInfo.ResponseMsg, chnlInfo.DecodeIter, chnlInfo.Dictionary, out var errorInfo)
                != ReactorReturnCode.SUCCESS)
            {
                Console.WriteLine(errorInfo?.Error.Text);
                Uninitialize();
                Environment.Exit((int)ReactorReturnCode.FAILURE);
            }
        }

        private void ProcessMarketByOrderResp(ChannelInfo chnlInfo)
        {
            if (chnlInfo.MarketByOrderHandler.ProcessResponse(chnlInfo.ResponseMsg, chnlInfo.DecodeIter, chnlInfo.Dictionary, out var errorInfo)
                != ReactorReturnCode.SUCCESS)
            {
                Console.WriteLine(errorInfo?.Error.Text);
                Uninitialize();
                Environment.Exit((int)ReactorReturnCode.FAILURE);
            }
        }

        private void ProcessMarketPriceResp(ChannelInfo chnlInfo)
        {
            if (chnlInfo.MarketPriceHandler.ProcessResponse(chnlInfo.ResponseMsg, chnlInfo.DecodeIter, chnlInfo.Dictionary, out var errorInfo)
                != ReactorReturnCode.SUCCESS)
            {
                Console.WriteLine(errorInfo?.Error.Text);
                Uninitialize();
                Environment.Exit((int)ReactorReturnCode.FAILURE);
            }
        }

        private void ProcessYieldCurveResp(ChannelInfo chnlInfo)
        {
            if (chnlInfo.YieldCurveHandler.ProcessResponse(chnlInfo.ResponseMsg, chnlInfo.DecodeIter, chnlInfo.Dictionary, out var errorInfo)
                != ReactorReturnCode.SUCCESS)
            {
                Console.WriteLine(errorInfo?.Error.Text);
                Uninitialize();
                Environment.Exit((int)ReactorReturnCode.FAILURE);
            }
        }

        /// <summary>
        /// Load dictionary from file.
        /// </summary>
        void LoadDictionary()
        {
            m_Dictionary.Clear();
            if (m_Dictionary.LoadFieldDictionary(FIELD_DICTIONARY_FILE_NAME, out var dictError) < 0)
            {
                Console.WriteLine("Unable to load field dictionary.  Will attempt to download from provider.\n\tText: "
                                   + dictError.Text);
            }
            else
            {
                fieldDictionaryLoadedFromFile = true;
            }

            if (m_Dictionary.LoadEnumTypeDictionary(ENUM_TABLE_FILE_NAME, out var enumError) < 0)
            {
                Console.WriteLine("Unable to load enum dictionary.  Will attempt to download from provider.\n\tText: "
                                   + enumError.Text);
            }
            else
            {
                enumTypeDictionaryLoadedFromFile = true;
            }
        }

        private void InitChannelInfo(ChannelInfo chnlInfo)
        {
            // set up consumer role
            chnlInfo.ConsumerRole.DefaultMsgCallback = this;
            chnlInfo.ConsumerRole.ChannelEventCallback = this;
            chnlInfo.ConsumerRole.LoginMsgCallback = this;
            chnlInfo.ConsumerRole.DirectoryMsgCallback = this;
            if (!fieldDictionaryLoadedFromFile
                || !enumTypeDictionaryLoadedFromFile)
            {
                chnlInfo.ConsumerRole.DictionaryMsgCallback = this;
            }

            // initialize consumer role to default
            chnlInfo.ConsumerRole.InitDefaultRDMLoginRequest();
            chnlInfo.ConsumerRole.InitDefaultRDMDirectoryRequest();

            // use command line login user name if specified
            if (!string.IsNullOrEmpty(m_ConsumerCmdLineParser.UserName))
            {
                LoginRequest loginRequest = chnlInfo.ConsumerRole.RdmLoginRequest!;
                loginRequest.UserName.Data(m_ConsumerCmdLineParser.UserName);
            }
            if (m_ConsumerCmdLineParser.Passwd != null)
            {
                LoginRequest loginRequest = chnlInfo.ConsumerRole.RdmLoginRequest!;
                loginRequest.Password.Data(m_ConsumerCmdLineParser.Passwd);
                loginRequest.HasPassword = true;
            }
            if(!string.IsNullOrEmpty(m_ConsumerCmdLineParser.ClientId))
            {
                oAuthCredential.ClientId.Data(m_ConsumerCmdLineParser.ClientId);

                if(!string.IsNullOrEmpty(m_ConsumerCmdLineParser.ClientSecret))
                {
                    oAuthCredential.ClientSecret.Data(m_ConsumerCmdLineParser.ClientSecret);

                    /* Specified the IReactorOAuthCredentialEventCallback to get sensitive information as needed to authorize with the token service. */
                    oAuthCredential.ReactorOAuthCredentialEventCallback = this;
                }
            }

            if (!string.IsNullOrEmpty(m_ConsumerCmdLineParser.TokenScope))
            {
                oAuthCredential.TokenScope.Data(m_ConsumerCmdLineParser.TokenScope);
            }

            oAuthCredential.UserSpecObj = oAuthCredential;
            chnlInfo.ConsumerRole.ReactorOAuthCredential = oAuthCredential;

            // use command line authentication token and extended authentication information if specified
            if (!string.IsNullOrEmpty(m_ConsumerCmdLineParser.AuthenticationToken))
            {
                LoginRequest loginRequest = chnlInfo.ConsumerRole.RdmLoginRequest!;
                loginRequest.UserNameType = Login.UserIdTypes.AUTHN_TOKEN;
                loginRequest.UserName.Data(m_ConsumerCmdLineParser.AuthenticationToken);

                if (!string.IsNullOrEmpty(m_ConsumerCmdLineParser.AuthenticationExtended))
                {
                    loginRequest.HasAuthenticationExtended = true;
                    loginRequest.AuthenticationExtended.Data(m_ConsumerCmdLineParser.AuthenticationExtended);
                }
            }

            // use command line application id if specified
            if (!string.IsNullOrEmpty(m_ConsumerCmdLineParser.ApplicationId))
            {
                LoginRequest loginRequest = chnlInfo.ConsumerRole.RdmLoginRequest!;
                loginRequest.LoginAttrib.ApplicationId.Data(m_ConsumerCmdLineParser.ApplicationId);
            }

            if (m_ConsumerCmdLineParser.EnableRtt)
            {
                chnlInfo.ConsumerRole.RdmLoginRequest!.LoginAttrib.HasSupportRoundTripLatencyMonitoring = true;
            }

            // if unable to load from file, enable consumer to download dictionary
            if (fieldDictionaryLoadedFromFile == false
                || enumTypeDictionaryLoadedFromFile == false)
            {
                chnlInfo.ConsumerRole.DictionaryDownloadMode = DictionaryDownloadMode.FIRST_AVAILABLE;
                m_Dictionary = new(); //drop the old dictionary
            }

            chnlInfo.Dictionary = m_Dictionary;

            chnlInfo.ShouldOffStreamPost = m_ConsumerCmdLineParser.EnableOffpost;
            // this application requires at least one market price item to be
            // requested for on-stream posting to be performed
            chnlInfo.ShouldOnStreamPost = m_ConsumerCmdLineParser.EnablePost;
            if (chnlInfo.ShouldOnStreamPost)
            {
                bool mpItemFound = chnlInfo.ConnectionArg.ItemList?.Any(itemArg => itemArg.Domain == DomainType.MARKET_PRICE)
                    ?? false;

                if (!mpItemFound)
                {
                    Console.WriteLine("\nPosting will not be performed for this channel as no Market Price items were requested");
                    chnlInfo.ShouldOnStreamPost = false;
                }
            }

            chnlInfo.PostHandler.EnableOnstreamPost = chnlInfo.ShouldOnStreamPost;
            chnlInfo.PostHandler.EnableOffstreamPost = chnlInfo.ShouldOffStreamPost;
            chnlInfo.MarketPriceHandler.SnapshotRequest = m_ConsumerCmdLineParser.EnableSnapshot;
            chnlInfo.MarketByOrderHandler.SnapshotRequest = m_ConsumerCmdLineParser.EnableSnapshot;
            chnlInfo.MarketByPriceHandler.SnapshotRequest = m_ConsumerCmdLineParser.EnableSnapshot;
            chnlInfo.YieldCurveHandler.SnapshotRequest = m_ConsumerCmdLineParser.EnableSnapshot;
            chnlInfo.SymbolListHandler.SnapshotRequest = m_ConsumerCmdLineParser.EnableSnapshot;
            chnlInfo.MarketPriceHandler.ViewRequest = m_ConsumerCmdLineParser.EnableView;
            // create item lists from those specified on command line
            CreateItemLists(chnlInfo);

            // set up reactor connect options
            chnlInfo.ConnectOptions.SetReconnectAttempLimit(-1); // attempt to recover forever
            chnlInfo.ConnectOptions.SetReconnectMinDelay(1000); // 1 second minimum
            chnlInfo.ConnectOptions.SetReconnectMaxDelay(60000); // 60 second maximum
            chnlInfo.ConnectOptions.ConnectionList[0].ConnectOptions.MajorVersion = Codec.Codec.MajorVersion();
            chnlInfo.ConnectOptions.ConnectionList[0].ConnectOptions.MinorVersion = Codec.Codec.MinorVersion();
            chnlInfo.ConnectOptions.ConnectionList[0].ConnectOptions.ConnectionType = chnlInfo.ConnectionArg.ConnectionType;

            if (m_ConsumerCmdLineParser.EnableSessionMgnt)
            {
                chnlInfo.ConnectOptions.ConnectionList[0].EnableSessionManagement = true;
                chnlInfo.ConnectOptions.ConnectionList[0].ReactorAuthTokenEventCallback = this;
                chnlInfo.ConnectOptions.ConnectionList[0].ConnectOptions.EncryptionOpts.EncryptedProtocol = ConnectionType.SOCKET;
            }

            chnlInfo.ConnectOptions.ConnectionList[0].ConnectOptions.UnifiedNetworkInfo.ServiceName = chnlInfo.ConnectionArg.Port ?? String.Empty;
            chnlInfo.ConnectOptions.ConnectionList[0].ConnectOptions.UnifiedNetworkInfo.Address = chnlInfo.ConnectionArg.Hostname ?? String.Empty;

            chnlInfo.ConnectOptions.ConnectionList[0].ConnectOptions.UserSpecObject = chnlInfo;
            chnlInfo.ConnectOptions.ConnectionList[0].ConnectOptions.GuaranteedOutputBuffers = 1000;
            // add backup connection if specified
            if (m_ConsumerCmdLineParser.BackupHostname != null
                && m_ConsumerCmdLineParser.BackupPort != null)
            {
                ReactorConnectInfo connectInfo = new();
                chnlInfo.ConnectOptions.ConnectionList.Add(connectInfo);
                chnlInfo.ConnectOptions.ConnectionList[1].ConnectOptions.MajorVersion = Codec.Codec.MajorVersion();
                chnlInfo.ConnectOptions.ConnectionList[1].ConnectOptions.MinorVersion = Codec.Codec.MinorVersion();
                chnlInfo.ConnectOptions.ConnectionList[1].ConnectOptions.ConnectionType = chnlInfo.ConnectionArg.ConnectionType;
                chnlInfo.ConnectOptions.ConnectionList[1].ConnectOptions.UnifiedNetworkInfo.ServiceName = m_ConsumerCmdLineParser.BackupPort;
                chnlInfo.ConnectOptions.ConnectionList[1].ConnectOptions.UnifiedNetworkInfo.Address = m_ConsumerCmdLineParser.BackupHostname;
                chnlInfo.ConnectOptions.ConnectionList[1].ConnectOptions.UserSpecObject = chnlInfo;
                chnlInfo.ConnectOptions.ConnectionList[1].ConnectOptions.GuaranteedOutputBuffers = 1000;

                if (m_ConsumerCmdLineParser.EnableSessionMgnt)
                {
                    chnlInfo.ConnectOptions.ConnectionList[1].EnableSessionManagement = true;

                    chnlInfo.ConnectOptions.ConnectionList[1].ReactorAuthTokenEventCallback = this;

                    ConnectOptions cOpt = chnlInfo.ConnectOptions.ConnectionList[1].ConnectOptions;
                    cOpt.ConnectionType = ConnectionType.ENCRYPTED;
                    cOpt.EncryptionOpts.EncryptedProtocol = ConnectionType.SOCKET;
                }
            }

            // handler encrypted connection
            chnlInfo.ShouldEnableEncrypted = m_ConsumerCmdLineParser.EnableEncrypted;

            if(chnlInfo.ShouldEnableEncrypted)
            {
                ConnectOptions cOpt = chnlInfo.ConnectOptions.ConnectionList[0].ConnectOptions;
                cOpt.ConnectionType = ConnectionType.ENCRYPTED;
                cOpt.EncryptionOpts.EncryptedProtocol = m_ConsumerCmdLineParser.EncryptedProtocolType;
            }

            /* Setup proxy if configured */
            if (m_ConsumerCmdLineParser.EnableProxy)
            {
                string? proxyHostName = m_ConsumerCmdLineParser.ProxyHostname;
                if (String.IsNullOrEmpty(proxyHostName))
                {
                    Console.Error.WriteLine("Error: Proxy hostname not provided.");
                    Environment.Exit((int)CodecReturnCode.FAILURE);
                }
                string? proxyPort = m_ConsumerCmdLineParser.ProxyPort;
                if (String.IsNullOrEmpty(proxyPort))
                {
                    Console.Error.WriteLine("Error: Proxy port number not provided.");
                    Environment.Exit((int)CodecReturnCode.FAILURE);
                }

                chnlInfo.ConnectOptions.ConnectionList[0].ConnectOptions.ProxyOptions.ProxyHostName = proxyHostName;
                chnlInfo.ConnectOptions.ConnectionList[0].ConnectOptions.ProxyOptions.ProxyPort = proxyPort;
                chnlInfo.ConnectOptions.ConnectionList[0].ConnectOptions.ProxyOptions.ProxyUserName = m_ConsumerCmdLineParser.ProxyUsername;
                chnlInfo.ConnectOptions.ConnectionList[0].ConnectOptions.ProxyOptions.ProxyPassword = m_ConsumerCmdLineParser.ProxyPasswd;
            }
        }

        private void SetItemState(ChannelInfo chnlInfo, int streamState, int dataState, int stateCode)
        {
            foreach (var entry in chnlInfo.ItemWatchList)
            {
                entry.Value.ItemState.StreamState(streamState);
                entry.Value.ItemState.DataState(dataState);
                entry.Value.ItemState.Code(stateCode);
            }
        }


        private void CreateItemLists(ChannelInfo chnlInfo)
        {
            // add specified items to item watch list
            if (chnlInfo.ConnectionArg.ItemList != null)
            {
                foreach (ItemArg itemArg in chnlInfo.ConnectionArg.ItemList)
                {
                    switch (itemArg.Domain)
                    {
                        case DomainType.MARKET_PRICE:
                            if (!itemArg.EnablePrivateStream)
                                chnlInfo.mpItemList.Add(itemArg.ItemName!);
                            else
                                chnlInfo.mppsItemList.Add(itemArg.ItemName!);
                            break;
                        case DomainType.MARKET_BY_ORDER:
                            if (!itemArg.EnablePrivateStream)
                                chnlInfo.mboItemList.Add(itemArg.ItemName!);
                            else
                                chnlInfo.mbopsItemList.Add(itemArg.ItemName!);
                            break;
                        case DomainType.MARKET_BY_PRICE:
                            if (!itemArg.EnablePrivateStream)
                                chnlInfo.mbpItemList.Add(itemArg.ItemName!);
                            else
                                chnlInfo.mbppsItemList.Add(itemArg.ItemName!);
                            break;
                        case DomainType.YIELD_CURVE:
                            if (!itemArg.EnablePrivateStream)
                                chnlInfo.ycItemList.Add(itemArg.ItemName!);
                            else
                                chnlInfo.ycpsItemList.Add(itemArg.ItemName!);
                            break;
                        case DomainType.SYMBOL_LIST:
                            chnlInfo.slItemList.Add(itemArg.ItemName!);
                            break;
                        default:
                            break;
                    }
                }
            }
        }

        private void SendSymbolListRequests(ChannelInfo chnlInfo)
        {
            if (chnlInfo.slItemList.Count == 0)
                return;

            if (!chnlInfo.ServiceInfo.HasInfo)
            {
                Uninitialize();
                Environment.Exit((int)ReactorReturnCode.FAILURE);
            }

            var info = chnlInfo.ServiceInfo.Info;
            if (info.QosList.Count > 0)
            {
                Qos qos = info.QosList[0];
                chnlInfo.SymbolListHandler.Qos.IsDynamic = qos.IsDynamic;
                chnlInfo.SymbolListHandler.Qos.Rate(qos.Rate());
                chnlInfo.SymbolListHandler.Qos.Timeliness(qos.Timeliness());
            }
            else
            {
                chnlInfo.SymbolListHandler.Qos.IsDynamic = false;
                chnlInfo.SymbolListHandler.Qos.Rate(QosRates.TICK_BY_TICK);
                chnlInfo.SymbolListHandler.Qos.Timeliness(QosTimeliness.REALTIME);
            }
            chnlInfo.SymbolListHandler.Capabilities.AddRange(info.CapabilitiesList);
            chnlInfo.SymbolListHandler.ServiceId = chnlInfo.ServiceInfo.ServiceId;

            string cmdSLName = chnlInfo.slItemList[0];
            if (cmdSLName == null)
            {
                chnlInfo.SymbolListHandler.SymbolListName.Data(info.ItemList.Data(), info.ItemList.Position, info.ItemList.Length);
            }
            else
            {
                chnlInfo.SymbolListHandler.SymbolListName.Data(cmdSLName);
            }

            if (chnlInfo.SymbolListHandler.SendRequest(chnlInfo.ReactorChannel!, out var errorInfo) != ReactorReturnCode.SUCCESS)
            {
                if (chnlInfo.ReactorChannel!.State != ReactorChannelState.CLOSED
                    && chnlInfo.ReactorChannel.State != ReactorChannelState.DOWN_RECONNECTING)
                {
                    Console.WriteLine(errorInfo?.Error.Text);
                    Uninitialize();
                    Environment.Exit((int)ReactorReturnCode.FAILURE);
                }
            }
        }

        private void SendMBPRequests(ChannelInfo chnlInfo)
        {
            if (chnlInfo.MarketByPriceHandler.SendItemRequests(chnlInfo.ReactorChannel!, chnlInfo.mbpItemList, false, chnlInfo.LoginRefresh,
                chnlInfo.ServiceInfo, out var errorInfo) != ReactorReturnCode.SUCCESS)
            {
                if (chnlInfo.ReactorChannel!.State != ReactorChannelState.CLOSED
                    && chnlInfo.ReactorChannel!.State != ReactorChannelState.DOWN_RECONNECTING)
                {
                    Console.WriteLine(errorInfo?.Error.Text);
                    Uninitialize();
                    Environment.Exit((int)ReactorReturnCode.FAILURE);
                }
            }

            if (chnlInfo.mbppsItemList.Count > 0 && !chnlInfo.mbppsRequestSent)
            {
                if (chnlInfo.MarketByPriceHandler.SendItemRequests(chnlInfo.ReactorChannel!, chnlInfo.mbppsItemList, true, chnlInfo.LoginRefresh,
                    chnlInfo.ServiceInfo, out errorInfo) != ReactorReturnCode.SUCCESS)
                {
                    if (chnlInfo.ReactorChannel!.State != ReactorChannelState.CLOSED &&
                        chnlInfo.ReactorChannel.State != ReactorChannelState.DOWN_RECONNECTING)
                    {
                        Console.WriteLine(errorInfo?.Error.Text);
                        Uninitialize();
                        Environment.Exit((int)ReactorReturnCode.FAILURE);
                    }
                }
                chnlInfo.mbppsRequestSent = true;
            }
        }

        private void SendMBORequests(ChannelInfo chnlInfo)
        {
            if (chnlInfo.MarketByOrderHandler.SendItemRequests(chnlInfo.ReactorChannel!, chnlInfo.mboItemList, false, chnlInfo.LoginRefresh,
                chnlInfo.ServiceInfo, out var errorInfo) != ReactorReturnCode.SUCCESS)
            {
                if (chnlInfo.ReactorChannel!.State != ReactorChannelState.CLOSED &&
                    chnlInfo.ReactorChannel.State != ReactorChannelState.DOWN_RECONNECTING)
                {
                    Console.WriteLine(errorInfo?.Error.Text);
                    Uninitialize();
                    Environment.Exit((int)ReactorReturnCode.FAILURE);
                }
            }

            if (chnlInfo.mbopsItemList.Count > 0 && !chnlInfo.mbopsRequestSent)
            {
                if (chnlInfo.MarketByOrderHandler.SendItemRequests(chnlInfo.ReactorChannel!, chnlInfo.mbopsItemList, true, chnlInfo.LoginRefresh,
                    chnlInfo.ServiceInfo, out errorInfo) != ReactorReturnCode.SUCCESS)
                {
                    if (chnlInfo.ReactorChannel!.State != ReactorChannelState.CLOSED &&
                        chnlInfo.ReactorChannel.State != ReactorChannelState.DOWN_RECONNECTING)
                    {
                        Console.WriteLine(errorInfo!.Error.Text);
                        Uninitialize();
                        Environment.Exit((int)ReactorReturnCode.FAILURE);
                    }
                }
                chnlInfo.mbopsRequestSent = true;
            }
        }

        private void SendMPRequests(ChannelInfo chnlInfo)
        {
            if (chnlInfo.MarketPriceHandler.SendItemRequests(chnlInfo.ReactorChannel!, chnlInfo.mpItemList, false,
                    chnlInfo.LoginRefresh, chnlInfo.ServiceInfo, out var errorInfo)
                != ReactorReturnCode.SUCCESS)
            {
                if (chnlInfo.ReactorChannel!.State != ReactorChannelState.CLOSED &&
                    chnlInfo.ReactorChannel.State != ReactorChannelState.DOWN_RECONNECTING)
                {
                    Console.WriteLine(errorInfo?.Error.Text);
                    Uninitialize();
                    Environment.Exit((int)ReactorReturnCode.FAILURE);
                }
            }

            if (chnlInfo.mppsItemList.Count > 0 && !chnlInfo.mppsRequestSent)
            {
                if (chnlInfo.MarketPriceHandler.SendItemRequests(chnlInfo.ReactorChannel!, chnlInfo.mppsItemList, true,
                        chnlInfo.LoginRefresh, chnlInfo.ServiceInfo, out errorInfo)
                    != ReactorReturnCode.SUCCESS)
                {
                    if (chnlInfo.ReactorChannel!.State != ReactorChannelState.CLOSED &&
                        chnlInfo.ReactorChannel.State != ReactorChannelState.DOWN_RECONNECTING)
                    {
                        Console.WriteLine(errorInfo?.Error.Text);
                        Uninitialize();
                        Environment.Exit((int)ReactorReturnCode.FAILURE);
                    }
                }
                chnlInfo.mppsRequestSent = true;
            }
        }

        private void SendYieldCurveRequests(ChannelInfo chnlInfo)
        {
            if (chnlInfo.YieldCurveHandler.SendItemRequests(chnlInfo.ReactorChannel!, chnlInfo.ycItemList, false, chnlInfo.LoginRefresh,
                chnlInfo.ServiceInfo, out var errorInfo) != ReactorReturnCode.SUCCESS)
            {
                if (chnlInfo.ReactorChannel!.State != ReactorChannelState.CLOSED &&
                    chnlInfo.ReactorChannel.State != ReactorChannelState.DOWN_RECONNECTING)
                {
                    Console.WriteLine(errorInfo?.Error.Text);
                    Uninitialize();
                    Environment.Exit((int)ReactorReturnCode.FAILURE);
                }
            }

            if (chnlInfo.ycpsItemList.Count > 0 && !chnlInfo.ycpsRequestSent)
            {
                if (chnlInfo.YieldCurveHandler.SendItemRequests(chnlInfo.ReactorChannel!, chnlInfo.ycpsItemList, true, chnlInfo.LoginRefresh,
                    chnlInfo.ServiceInfo, out errorInfo) != ReactorReturnCode.SUCCESS)
                {
                    if (chnlInfo.ReactorChannel!.State != ReactorChannelState.CLOSED &&
                        chnlInfo.ReactorChannel.State != ReactorChannelState.DOWN_RECONNECTING)
                    {
                        Console.WriteLine(errorInfo?.Error.Text);
                        Uninitialize();
                        Environment.Exit((int)ReactorReturnCode.FAILURE);
                    }
                }
                chnlInfo.ycpsRequestSent = true;
            }
        }

        private void CloseItemStreams(ChannelInfo chnlInfo)
        {
            // have offstream posting post close status
            if (chnlInfo.ShouldOffStreamPost)
            {
                chnlInfo.PostHandler.StreamId = chnlInfo.LoginRefresh.StreamId;
                chnlInfo.PostHandler.PostItemName.Data("OFFPOST");
                chnlInfo.PostHandler.ServiceId = chnlInfo.ServiceInfo.ServiceId;
                chnlInfo.PostHandler.Dictionary = chnlInfo.Dictionary;
                chnlInfo.PostHandler.CloseOffStreamPost(chnlInfo.ReactorChannel!, out _);
            }

            // close item streams if opened
            chnlInfo.MarketPriceHandler.CloseStreams(chnlInfo.ReactorChannel!, out _);
            chnlInfo.MarketByOrderHandler.CloseStreams(chnlInfo.ReactorChannel!, out _);
            chnlInfo.MarketByPriceHandler.CloseStreams(chnlInfo.ReactorChannel!, out _);
            chnlInfo.SymbolListHandler.CloseStream(chnlInfo.ReactorChannel!, out _);
            chnlInfo.YieldCurveHandler.CloseStreams(chnlInfo.ReactorChannel!, out _);
        }

        /* Uninitializes the Value Add consumer application. */
        private void Uninitialize()
        {
            Console.WriteLine("Consumer unitializing and exiting...");

            foreach (ChannelInfo chnlInfo in chnlInfoList)
            {
                // close items streams
                CloseItemStreams(chnlInfo);

                // close ReactorChannel
                if (chnlInfo.ReactorChannel != null)
                {
                    chnlInfo.ReactorChannel.Close(out _);
                }
            }

            if(m_FileStream != null)
            {
                m_FileStream.Dispose();
            }

            // shutdown reactor
            m_Reactor?.Shutdown(out _);
        }

        private void RegisterChannel(ReactorChannel chan)
        {
            m_ReadSockets.Add(chan.Socket!);
            m_SocketFdValueMap.Add(chan.Socket!.Handle.ToInt32(), chan);
        }

        private void UnregisterSocket(Socket sock)
        {
            m_ReadSockets.Remove(sock);
            m_SocketFdValueMap.Remove(sock.Handle.ToInt32());
        }

        private ReactorChannel GetChannelBySocketFd(int fd)
        {
            return m_SocketFdValueMap[fd];
        }

        private void HandleClose()
        {
            Console.WriteLine("Consumer closes streams...");

            foreach (ChannelInfo chnlInfo in chnlInfoList)
            {
                CloseItemStreams(chnlInfo);
            }
        }

        public static void Main(string[] args)
        {
            VAConsumer consumer = new VAConsumer();
            consumer.Init(args);
            consumer.Run();
            consumer.Uninitialize();
            Environment.Exit(0);
        }
    }
}
