/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2022-2023 Refinitiv. All rights reserved.              --
 *|-----------------------------------------------------------------------------
 */

using System;
using System.Collections.Generic;
using System.Net.Sockets;
using LSEG.Eta.Common;
using LSEG.Eta.Codec;
using LSEG.Eta.Rdm;
using LSEG.Eta.Example.Common;
using LSEG.Eta.Transports;

namespace LSEG.Eta.Example.Provider
{
    /// <summary>
    /// This is the main class for the ETA .NET Provider application. It is a
    /// single-threaded server application. The application uses either the operating
    /// parameters entered by the user or a default set of parameters.
    /// <para>
    /// The purpose of this application is to interactively provide Level I Market Price,
    /// Level II Market By Order, Level II Market By Price, and Symbol List data to
    /// one or more consumers. It allowed, it requests dictionary from the adh.
    /// </para>
    /// <para>
    /// It is a single-threaded server application. First the application initializes
    /// the ETA transport and binds the server. If the dictionary files are in the path
    /// it loads dictionary information from the RDMFieldDictionary and enumtype.def files.
    /// Finally, it processes login, source directory, dictionary, market price,
    /// market by order, market by price, and symbol list requests from consumers and sends
    /// the appropriate responses.
    /// </para>
    /// <para>
    /// Level II Market By Price refresh messages are sent as multi-part messages. An
    /// update message is sent between each part of the multi-part refresh message.
    /// </para>
    /// <para>
    /// Dictionary requests are supported by this application. If the Login Request
    /// indicates the presence of the dictionary request feature and the dictionaries are
    /// not setup on startup, this application sends dictionary requests for both
    /// field dictionary and enumtype dictionary. The responses to the dictionary requests
    /// are processed as well.
    /// </para>
    /// <para>
    /// Batch requests are supported by this application. The login response message
    /// indicates that batch support is present. Batch requests are accepted and a stream
    /// is opened for each item in the batch request.
    /// </para>
    /// <para>
    /// Posting requests are supported by this application for items that have already
    /// been opened by a consumer. On-stream and off-stream posts are accepted and sent
    /// out to any consumer that has the item open. Off-stream posts for items that
    /// have not already been opened by a consumer are rejected (in this example).
    /// </para>
    /// <para>
    /// Private stream requests are also supported by this application. All items requested
    /// with the private stream flag set in the request message result in the private
    /// stream flag set in the applicable response messages. If a request is received
    /// without the private stream flag set for the item name of "RES-DS", this application
    /// redirects the consumer to open the "RES-DS" item on a private stream instead
    /// of a normal stream.
    /// </para>
    /// <para>
    /// Symbol List requests are expected to use a symbol list name of "_ETA_ITEM_LIST".
    /// The symbol list name is provided in the source directory response for the consumer
    /// to use.
    /// </para>
    /// Arguments are listed below.
    /// <list type="bullet">
    /// <item>
    /// <description>-id Service id. Default is 1</description>
    /// </item>
    /// <item>
    /// <description>-p Server port number. Default is 14002</description>
    /// </item>
    /// <item>
    /// <description>-s Service name. Default is DIRECT_FEED</description>
    /// </item>
    /// <item>
    /// <description>-x Provides XML tracing of messages.</description>
    /// </item>
    /// <item>
    /// <description>-runtime run time. Default is 1200 seconds. Controls the time the
    /// application will run before exiting, in seconds.
    /// </description>
    /// </item>
    /// <item>
    /// <description>-rtt application (provider) supports calculation of Round Trip Latency</description>
    /// </item> 
    /// </list>
    /// </summary>
    public class Provider : IReceivedMsgCallback
    {
        private ProviderSession m_ProviderSession;
        private DecodeIterator m_DIter = new DecodeIterator();
        private Msg m_ReceivedMsg = new Msg();
        private UnSupportedMsgHandler m_UnSupportedMsgHandler;
        private ProviderDictionaryHandler m_DictionaryHandler;
        private ProviderDirectoryHandler m_DirectoryHandler;
        private ProviderLoginHandler m_LoginHandler;
        private ItemHandler m_ItemHandler;

        private long m_Runtime;

        private const int UPDATE_INTERVAL = 1;
        private long publishTime = 0;

        /* default server port number */
        private static readonly string defaultSrvrPortNo = "14002";

        /* default service name */
        private static readonly string defaultServiceName = "DIRECT_FEED";

        /* default run time in seconds */
        private static readonly string defaultRuntime = "1200"; // seconds

        public static readonly int CLIENT_SESSION_INIT_TIMEOUT = 30; // seconds

        public Provider()
        {
            m_ProviderSession = new ProviderSession();
            m_UnSupportedMsgHandler = new UnSupportedMsgHandler(m_ProviderSession);
            m_DictionaryHandler = new ProviderDictionaryHandler(m_ProviderSession);
            m_DirectoryHandler = new ProviderDirectoryHandler(m_ProviderSession);
            m_LoginHandler = new ProviderLoginHandler(m_ProviderSession);
            m_ItemHandler = new ItemHandler(m_ProviderSession, m_DictionaryHandler, m_LoginHandler);
        }


        static void Main(string[] args)
        {
            Provider provider = new Provider();
            provider.Init(args);
            provider.Run();
            provider.UnInit();
        }

        /// <summary>
        /// Parses command line arguments, initializes provider session which creates
        /// listening socket. It also initializes Login, Directory, Dictionary and
        /// Item Handlers.
        /// </summary>
        /// <param name="args">command line arguments</param>
        public void Init(String[] args)
        {
            /* process command line args */
            AddCommandLineArgs();
            try
            {
                CommandLine.ParseArgs(args);
            }
            catch (Exception exp)
            {
                Console.WriteLine("Error loading command line arguments:\t");
                Console.WriteLine(exp.Message);
                Console.WriteLine();
                Console.WriteLine(CommandLine.OptionHelpString());
                Environment.Exit((int)CodecReturnCode.FAILURE);
            }
            Console.WriteLine("connectionType: " + CommandLine.Value("c"));
            Console.WriteLine("portNo: " + CommandLine.Value("p"));
            Console.WriteLine("interfaceName: " + CommandLine.Value("i"));
            Console.WriteLine("serviceName: " + CommandLine.Value("s"));
            Console.WriteLine("serviceId: " + CommandLine.Value("id"));
            Console.WriteLine("enableRTT: " + CommandLine.Value("rtt"));
            string? connectionType = CommandLine.Value("c");

            if (!m_DictionaryHandler.LoadDictionary(out CodecError? codecError))
            {
                /* if no local dictionary found maybe we can request it from ADH */
                Console.WriteLine("Local dictionary not available, will try to request it from ADH if it supports the Provider Dictionary Download\n");
            }

            // get bind options from the provider session
            BindOptions bindOptions = m_ProviderSession.BindOptions;

            // set the connection parameters on the bind options
            bindOptions.ServiceName = CommandLine.Value("p");
            bindOptions.InterfaceName = CommandLine.Value("i");
            if (connectionType != null && connectionType.Equals("encrypted"))
            {
                bindOptions.ConnectionType = ConnectionType.ENCRYPTED;
                bindOptions.BindEncryptionOpts.ServerCertificate = CommandLine.Value("cert");
                bindOptions.BindEncryptionOpts.ServerPrivateKey = CommandLine.Value("keyfile");
            }

            TransportReturnCode ret = m_ProviderSession.Init(false, out Error? error);
            if (ret != TransportReturnCode.SUCCESS)
            {
                Console.WriteLine($"Failed initializing server, error: {error?.Text}");
               Environment.Exit((int)TransportReturnCode.FAILURE);
            }

            // enable XML tracing
            if (CommandLine.BoolValue("x"))
            {
                m_ProviderSession.EnableXmlTrace(m_DictionaryHandler.Dictionary);
            }

            m_LoginHandler.Init();
            m_DirectoryHandler.Init();
            m_DirectoryHandler.ServiceName = CommandLine.Value("s");
            m_ItemHandler.Init();
            try
            {
                m_LoginHandler.EnableRtt = CommandLine.BoolValue("rtt");
                m_DirectoryHandler.ServiceId = CommandLine.IntValue("id");
                m_ItemHandler.ServiceId = CommandLine.IntValue("id");
                m_Runtime = (System.DateTime.Now.Ticks / TimeSpan.TicksPerMillisecond) + CommandLine.IntValue("runtime") * 1000;
            }
            catch (Exception exp)
            {
                Console.WriteLine("Invalid argument, number expected.\t");
                Console.WriteLine(exp.Message);
                Environment.Exit(-1);
            }
        }

        private static void AddCommandLineArgs()
        {
            CommandLine.ProgName("ETA Provider");
            CommandLine.AddOption("p", defaultSrvrPortNo, "Server port number");
            CommandLine.AddOption("s", defaultServiceName, "Service name");
            CommandLine.AddOption("i", defaultValue: null!, "Interface name");
            CommandLine.AddOption("runtime", defaultRuntime, "Program runtime in seconds");
            CommandLine.AddOption("id", "1", "Service id");
            CommandLine.AddOption("x", "Provides XML tracing of messages.");
            CommandLine.AddOption("rtt", false, "Provider supports calculation of Round Trip Latency");
            CommandLine.AddOption("c", defaultValue: null!, "Provider connection type.  Either \"socket\" or \"encrypted\"");
            CommandLine.AddOption("cert", defaultValue: null!, "The server certificate file");
            CommandLine.AddOption("keyfile", defaultValue: null!, "The server private key file");
        }

        private void HandleRuntime()
        {
            // get current time
            long currentTime = System.DateTime.Now.Ticks / TimeSpan.TicksPerMillisecond;

            if (currentTime >= m_Runtime)
            {
                // send close status messages to all streams on all channels
                foreach (ClientSessionInfo clientSessionInfo in m_ProviderSession.ClientSessions)
                {
                    if ((clientSessionInfo != null) &&
                        (clientSessionInfo.ClientChannel != null &&
                         clientSessionInfo.ClientChannel.Socket != null &&
                         clientSessionInfo.ClientChannel.State != ChannelState.INACTIVE))
                    {
                        // send close status messages to all item streams
                        CodecReturnCode ret = m_ItemHandler.SendCloseStatusMsgs(clientSessionInfo.ClientChannel, out Error? error);
                        if (ret != CodecReturnCode.SUCCESS)
                            Console.WriteLine($"Failed sending item close, error: {error?.Text}");

                        // send close status message to source directory stream
                        ret = m_DirectoryHandler.SendCloseStatus(clientSessionInfo.ClientChannel, out error);
                        if (ret != CodecReturnCode.SUCCESS)
                            Console.WriteLine($"Failed sending directory close, error: {error?.Text}");

                        // send close status messages to dictionary streams
                        ret = m_DictionaryHandler.SendCloseStatusMsgs(clientSessionInfo.ClientChannel, out error);
                        if (ret != CodecReturnCode.SUCCESS)
                            Console.WriteLine($"Failed sending dictionary close: {error?.Text}");

                        // flush before exiting
                        if (clientSessionInfo.ClientChannel != null && clientSessionInfo.ClientChannel.Socket != null)
                        {
                            bool isWritable = clientSessionInfo.ClientChannel.Socket.Poll(10, System.Net.Sockets.SelectMode.SelectWrite);

                            if (isWritable)
                            {
                                int flushRet = 1;
                                while (flushRet > (int)TransportReturnCode.SUCCESS)
                                {
                                    flushRet = (int)clientSessionInfo.ClientChannel.Flush(out error);
                                }
                                if (flushRet < (int)TransportReturnCode.SUCCESS)
                                {
                                    Console.WriteLine($"ClientChannel.Flush() failed with return code {ret} and text {error?.Text}");
                                }
                            }
                        }
                    }
                }
                Console.WriteLine("provider run-time expired...");
                UnInit();
                Environment.Exit(0);
            }

        }

        /// <summary>
        /// Main loop polls socket events from server socket. Accepts new client
        /// connections and reads requests from already established client
        /// connection. Checks for runtime expiration. If there is no activity on the
        /// socket, periodically sends item updates to connected client sessions that
        /// has requested market price items.
        /// </summary>
        public void Run()
        {
            CodecReturnCode ret;
            List<Socket> readList = new List<Socket>(ProviderSession.CLIENT_SESSIONS_LIMIT);
            List<Socket> writeList = new List<Socket>(ProviderSession.CLIENT_SESSIONS_LIMIT);
            SelectElement selectElement;

            //APIQA
            int updateCount = 0;
            //END APIQA

            // main loop
            while (true)
            {
                try
                {
                    readList.Clear();
                    writeList.Clear();

                    if(m_ProviderSession.ServerSocket != null)
                    {
                        // Register to accept incoming client connections.
                        readList.Add(m_ProviderSession.ServerSocket);
                    }

                    for(int i =0; i < m_ProviderSession.ClientSessionSocketList.Count; i++)
                    {
                        selectElement = m_ProviderSession.ClientSessionSocketList[i];

                        if (selectElement.Mode == Common.SelectMode.READ)
                        {
                            readList.Add(selectElement.Socket!);
                        }
                        else
                        {
                            if (selectElement.Mode == (Common.SelectMode.READ | Common.SelectMode.WRITE))
                            {
                                readList.Add(selectElement.Socket!);
                                writeList.Add(selectElement.Socket!);
                            }
                        }
                    }

                    if (readList.Count > 0 || writeList.Count > 0)
                    {
                        Socket.Select(readList, writeList, null, UPDATE_INTERVAL * 200 * 1000);
                    }
                }
                catch (Exception exp)
                {
                    Console.Write(exp.Message);
                    CleanupAndExit();
                }

                if (publishTime < (System.DateTime.Now.Ticks / TimeSpan.TicksPerMillisecond))
                {
                    /* Send market price updates for each connected channel */
                    m_ItemHandler.UpdateItemInfo();

                    foreach (ClientSessionInfo clientSessionInfo in m_ProviderSession.ClientSessions)
                    {
                        if ((clientSessionInfo != null) &&
                            (clientSessionInfo.ClientChannel != null &&
                             clientSessionInfo.ClientChannel.Socket != null &&
                             clientSessionInfo.ClientChannel.State != ChannelState.INACTIVE))
                        {
                            m_LoginHandler.ProceedLoginRttMessage(clientSessionInfo.ClientChannel, out Error? error);
                            //APIQA
                            if (updateCount <= 10)
                            {
                                ret = m_ItemHandler.SendItemUpdates(clientSessionInfo.ClientChannel, out error);
                                if (ret != CodecReturnCode.SUCCESS)
                                {
                                    Console.WriteLine(error!.Text);
                                    ProcessChannelClose(clientSessionInfo.ClientChannel);
                                    m_ProviderSession.RemoveClientSessionForChannel(clientSessionInfo.ClientChannel);
                                    RemoveInactiveSessions();
                                }
                                updateCount = updateCount + 1;
                            }
                            //END APIQA
                        }
                    }
                    publishTime = (System.DateTime.Now.Ticks / TimeSpan.TicksPerMillisecond) + 1000;
                }

                if (readList.Count != 0 || writeList.Count != 0)
                {
                    CheckTimeout();

                    Socket socket;
                    Error? error;
                    for(int i = 0; i < readList.Count; i++)
                    {
                        socket = readList[i];
                        if(m_ProviderSession.ServerSocket == socket)
                        {
                            TransportReturnCode transportRet = m_ProviderSession.HandleNewClientSession(m_ProviderSession.Server!, out error);
                            if (transportRet != TransportReturnCode.SUCCESS)
                            {
                                Console.WriteLine($"accept error, text: {error?.Text}");
                                continue;
                            }
                        }
                        else
                        {
                            IChannel? channel = m_ProviderSession.GetClientChannel(socket);
                            if (channel != null)
                            {
                                TransportReturnCode transportRet = m_ProviderSession.Read(channel, out error, this);
                                if (transportRet != TransportReturnCode.SUCCESS)
                                {
                                   Console.WriteLine($"read error, text: {error?.Text}");
                                    continue;
                                }
                            }
                        }
                    }

                    for (int i = 0; i < writeList.Count; i++)
                    {
                        socket = writeList[i];
                        IChannel? channel = m_ProviderSession.GetClientChannel(socket);
                        if (channel != null && channel.State == ChannelState.ACTIVE)
                        {
                            m_ProviderSession.Flush(socket, out error);
                        }
                    }
                }

                /* Handle pings */
                m_ProviderSession.HandlePings();

                /* Handle run-time */
                HandleRuntime();
            }
        }

        public void ProcessChannelClose(IChannel? channel)
        {
            if (channel != null)
            {
                m_ItemHandler.CloseRequests(channel);
                m_DictionaryHandler.CloseRequests(channel);
                m_DirectoryHandler.CloseRequest(channel);
                m_LoginHandler.CloseRequestAndRtt(channel);
            }
        }

        /// <summary>
        /// Call back for socket read for client messages.
        /// </summary>
        /// <param name="channel">The channel for the message</param>
        /// <param name="msgBuf">The client message buffer</param>
        public void ProcessReceivedMsg(IChannel channel, ITransportBuffer msgBuf)
        {
            CodecReturnCode ret;

            /* clear decode iterator */
            m_DIter.Clear();

            /* set buffer and version info */
            ret = m_DIter.SetBufferAndRWFVersion(msgBuf, channel.MajorVersion, channel.MinorVersion);

            Error? error;
            if (ret != CodecReturnCode.SUCCESS)
            {
                Console.WriteLine($"DecodeIterator.SetBufferAndRWFVersion() failed with return code: {ret.GetAsString()}");
                ProcessChannelClose(channel);
                m_ProviderSession.RemoveClientSessionForChannel(channel);
                RemoveInactiveSessions();

            }

            ret = m_ReceivedMsg.Decode(m_DIter);
            if (ret != CodecReturnCode.SUCCESS)
            {
                Console.WriteLine($"RequestMsg.decode() failed with return code: {ret.GetAsString()} on SessionData {channel.Socket} Size {msgBuf.Data.Limit - msgBuf.Data.Position}");
                ProcessChannelClose(channel);
                m_ProviderSession.RemoveClientSessionForChannel(channel);
                RemoveInactiveSessions();
            }

            switch (m_ReceivedMsg.DomainType)
            {
                case (int)DomainType.LOGIN:
                    {
                        if (m_LoginHandler.ProcessRequest(channel, m_ReceivedMsg, m_DIter, out error) != 0)
                        {
                            Console.WriteLine($"Failed processing login request, error: {error?.Text}");
                            ProcessChannelClose(channel);
                            m_ProviderSession.RemoveClientSessionForChannel(channel);
                            RemoveInactiveSessions();
                        }

                        // request dictionary from ADH if not available locally
                        if (!m_DictionaryHandler.IsDictionaryReady)
                        {
                            LoginRequestInfo? loginReqInfo = m_LoginHandler.FindLoginRequestInfo(channel);

                            if (loginReqInfo != null)
                            {
                                if (loginReqInfo.LoginRequest.HasAttrib &&
                                    loginReqInfo.LoginRequest.LoginAttrib.HasProviderSupportDictDownload &&
                                    loginReqInfo.LoginRequest.LoginAttrib.SupportProviderDictionaryDownload == 1)
                                {
                                    CodecReturnCode requestStatus = m_DictionaryHandler.SendDictionaryRequests(channel, out error, m_DirectoryHandler.ServiceId);
                                    if (requestStatus == CodecReturnCode.SUCCESS)
                                    {
                                        Console.WriteLine("Sent Dictionary Request\n");
                                    }
                                    else
                                    {
                                        Console.WriteLine($"Dictionary could not be downloaded, unable to send the request to the connection, error: {error?.Text}");
                                        ProcessChannelClose(channel);
                                        m_ProviderSession.RemoveClientSessionForChannel(channel);
                                        RemoveInactiveSessions();
                                    }
                                }
                                else
                                {
                                    Console.WriteLine("Dictionary could not be downloaded, the connection does not support Provider Dictionary Download");
                                    ProcessChannelClose(channel);
                                    m_ProviderSession.RemoveClientSessionForChannel(channel);
                                    RemoveInactiveSessions();
                                }
                            }
                            else
                            {
                                Console.WriteLine($"Could not find login request information for {channel.Socket}");
                                ProcessChannelClose(channel);
                                m_ProviderSession.RemoveClientSessionForChannel(channel);
                                RemoveInactiveSessions();
                            }
                        }
                        break;
                    }

                case (int)DomainType.SOURCE:
                    if (m_DirectoryHandler.ProcessRequest(channel, m_ReceivedMsg, m_DIter, out error) != 0)
                    {
                        Console.WriteLine($"Failed processing directory request, error: {error?.Text}");
                        ProcessChannelClose(channel);
                        m_ProviderSession.RemoveClientSessionForChannel(channel);
                        RemoveInactiveSessions();
                    }
                    break;
                case (int)DomainType.DICTIONARY:
                    if (m_DictionaryHandler.ProcessMessage(channel, m_ReceivedMsg, m_DIter, out error) != 0)
                    {
                        Console.WriteLine($"Failed processing dictionary message, error: {error?.Text}");
                        ProcessChannelClose(channel);
                        m_ProviderSession.RemoveClientSessionForChannel(channel);
                        RemoveInactiveSessions();
                    }
                    break;

                case (int)DomainType.MARKET_PRICE:
                case (int)DomainType.MARKET_BY_ORDER:
                case (int)DomainType.MARKET_BY_PRICE:
                case (int)DomainType.SYMBOL_LIST:
                    if (m_ItemHandler.ProcessRequest(channel, m_ReceivedMsg, m_DIter, out error) != 0)
                    {
                        Console.WriteLine($"Failed processing item request, error: {error?.Text}");
                        ProcessChannelClose(channel);
                        m_ProviderSession.RemoveClientSessionForChannel(channel);
                        RemoveInactiveSessions(); ;
                    }
                    break;
                default:
                    if (m_UnSupportedMsgHandler.ProcessRequest(channel, m_ReceivedMsg, out error) != 0)
                    {
                        Console.WriteLine($"Failed processing unhandled request message. error: {error?.Text}");
                        ProcessChannelClose(channel);
                        m_ProviderSession.RemoveClientSessionForChannel(channel);
                        RemoveInactiveSessions();
                    }
                    break;
            }
        }

        private void CheckTimeout()
        {
            foreach (ClientSessionInfo clientSessionInfo in m_ProviderSession.ClientSessions)
            {
                if (clientSessionInfo != null &&
                    clientSessionInfo.ClientChannel != null &&
                    clientSessionInfo.ClientChannel.State == ChannelState.INITIALIZING)
                {
                    long currentTime = (System.DateTime.Now.Ticks / TimeSpan.TicksPerMillisecond);
                    if ((currentTime - clientSessionInfo.StartTime) > CLIENT_SESSION_INIT_TIMEOUT * 1000)
                    {
                        Console.WriteLine($"Provider close clientSesson due to timeout of initialization {clientSessionInfo.ClientChannel.Socket} curTime= {currentTime} startTime = {clientSessionInfo.StartTime}");
                        m_ProviderSession.RemoveClientSessionForChannel(clientSessionInfo.ClientChannel);
                        RemoveInactiveSessions();
                    }
                }
            }
        }

        private void RemoveInactiveSessions()
        {
            foreach (ClientSessionInfo clientSessionInfo in m_ProviderSession.ClientSessions)
            {
                if (clientSessionInfo != null &&
                    clientSessionInfo.ClientChannel != null &&
                    clientSessionInfo.ClientChannel.State == ChannelState.INACTIVE && clientSessionInfo.StartTime > 0)
                {
                    Console.WriteLine("Provider close clientSesson due to inactive state ");
                    ProviderSession.RemoveInactiveClientSessionForChannel(clientSessionInfo);
                }
            }
        }

        private void UnInit()
        {
            m_ProviderSession.UnInit();
        }

        private void CleanupAndExit()
        {
            m_ProviderSession.UnInit();
            Environment.Exit((int)TransportReturnCode.FAILURE);
        }
    }
}
