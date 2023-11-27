//APIQA
namespace LSEG.Eta.ValueAdd.WatchlistConsumer;

using LSEG.Eta.Codec;
using LSEG.Eta.Common;
using LSEG.Eta.Example.Common;
using LSEG.Eta.Rdm;
using LSEG.Eta.Transports;
using LSEG.Eta.ValueAdd.Rdm;
using LSEG.Eta.ValueAdd.Reactor;
using System;
using System.IO;
using System.Net;
using System.Net.Sockets;
using static LSEG.Eta.Rdm.Dictionary;
using static Rdm.LoginMsgType;

/**
 * <p>
 * This interface implements callbacks that process information
 * received by the provider. It creates the Reactor, creates the desired
 * connections, then dispatches from the Reactor for events and messages.
 * Once it has received the event indicating that the channel is ready,
 * it will trigger ConsumerRequestThread to send the desired item requests (snapshot or streaming) to a
 * provider. The resulting decoded responses from the provided are displayed on the console.
 * </p>
 */
class ConsumerCallbackThread : IConsumerCallback, IReactorServiceEndpointEventCallback
{
    private const string FIELD_DICTIONARY_DOWNLOAD_NAME = "RWFFld";
    private const string ENUM_TABLE_DOWNLOAD_NAME = "RWFEnum";
    private const string FIX_FIELD_DICTIONARY_FILE_NAME = "FDMFixFieldDictionary";
    private const string FIX_ENUM_TABLE_FILE_NAME = "FDMenumtypes.def";

    private const int FIELD_DICTIONARY_STREAM_ID = 3;
    private const int ENUM_DICTIONARY_STREAM_ID = 4;
    private const int MAX_MSG_SIZE = 1024;
    private const int TRANSPORT_BUFFER_SIZE_CLOSE = MAX_MSG_SIZE;

    private Reactor? m_Reactor;
    private FileStream? m_FileStream;
    private ChannelInfo? m_ChnlInfo;
    private WatchlistConsumer? m_WatchlistConsumer;
    public WatchlistConsumerConfig? m_WatchlistConsumerConfig;
    public ConsumerRequestThread? m_ConsumerRequestThread;
    private DataDictionary m_Dictionary;

    private ReactorOptions m_ReactorOptions = new ReactorOptions();
    private ReactorErrorInfo m_errorInfo = new ReactorErrorInfo();
    private ReactorDispatchOptions m_DispatchOptions = new ReactorDispatchOptions();
    private readonly ReactorServiceDiscoveryOptions m_ReactorServiceDiscoveryOptions = new();
    private readonly ReactorOAuthCredential m_ReactorOAuthCredential = new();
    private readonly ItemDecoder m_ItemDecoder = new();
    //private List<ChannelInfo> m_ChnlInfoList = new();
    private ReactorSubmitOptions m_SubmitOptions = new();
    private ICloseMsg m_CloseMsg = new Msg();
    private readonly List<Socket> m_Sockets = new();
    private readonly Dictionary<int, ReactorChannel> m_SocketFdValueMap = new();

    private bool m_CloseHandled = false;
    public bool m_ShutDown = false;
    public bool m_ItemsRequested = false;
    private bool m_FieldDictionaryLoaded;
    private bool m_EnumDictionaryLoaded;

    public ConsumerCallbackThread(ChannelInfo channelInfo, WatchlistConsumer consumer)
    {
        m_Dictionary = new();

        m_DispatchOptions.SetMaxMessages(1);

        m_ItemsRequested = false;

        m_ChnlInfo = channelInfo;
        m_WatchlistConsumer = consumer;
        m_WatchlistConsumerConfig = m_WatchlistConsumer.WatchlistConsumerConfig();
    }

    /* Initializes the Value Add consumer application. */
    private void Init()
    {
        m_ItemDecoder.Init();

        // enable Reactor XML tracing if specified
        if (m_WatchlistConsumerConfig!.EnableXmlTracing)
        {
            m_ReactorOptions.XmlTracing = true;
        }

        // Set the Token Generator URL locations, if specified

        if (!string.IsNullOrEmpty(m_WatchlistConsumerConfig.TokenUrlV2))
        {
            m_ReactorOptions.SetTokenServiceURL(m_WatchlistConsumerConfig.TokenUrlV2);
        }

        if (!string.IsNullOrEmpty(m_WatchlistConsumerConfig.ServiceDiscoveryURL))
        {
            m_ReactorOptions.SetServiceDiscoveryURL(m_WatchlistConsumerConfig.ServiceDiscoveryURL);
        }

        m_ReactorOptions.EnableRestLogStream = m_WatchlistConsumerConfig.EnableRestLogging;

        if (!string.IsNullOrEmpty(m_WatchlistConsumerConfig.RestLogFileName))
        {
            try
            {
                m_FileStream = new FileStream(m_WatchlistConsumerConfig.RestLogFileName, FileMode.Create);
                m_ReactorOptions.RestLogOutputStream = m_FileStream;
            }
            catch (Exception ex)
            {
                Console.WriteLine($"Failed to create a FileStream with error text: {ex.Message}");
                Environment.Exit((int)ReactorReturnCode.FAILURE);
            }
        }

        // create reactor
        m_Reactor = Reactor.CreateReactor(m_ReactorOptions, out var errorInfo);
        if (m_Reactor == null)
        {
            Console.WriteLine("createReactor() failed: " + errorInfo!.ToString());
            Environment.Exit((int)ReactorReturnCode.FAILURE);
        }

        m_Sockets.Add(m_Reactor!.EventSocket!);

        // initialize channel info
        InitChannelInfo(m_ChnlInfo!);

        // connect channel
        ReactorReturnCode ret;
        if ((ret = m_Reactor!.Connect(m_ChnlInfo!.ConnectOptions, m_ChnlInfo.ConsumerRole, out var eInfo)) < ReactorReturnCode.SUCCESS)
        {
            Console.WriteLine("Reactor.Connect() failed with return code: " + ret + " error = " + eInfo?.Error.Text);
            Environment.Exit((int)ReactorReturnCode.FAILURE);
        }

        new Thread(new ThreadStart((m_ConsumerRequestThread = new ConsumerRequestThread(m_ChnlInfo, this)).Run)).Start();
    }

    /* Runs the Value Add consumer application. */
    public void Run()
    {
        // initialize the test data from configuration and xml files
        Init();

        Thread.CurrentThread.IsBackground = true;

        int selectTime = 1000;
        while (true)
        {
            List<Socket> sockReadList = new(m_Sockets);
            List<Socket> sockErrList = new(m_Sockets);

            try
            {
                Socket.Select(sockReadList, null, sockErrList, selectTime);
            }
            catch (Exception e)
            {
                Console.WriteLine($"Socket.Select failed: {e.Message}");
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
                        ReactorErrorInfo? eInfo;
                        while ((ret = m_Reactor.Dispatch(m_DispatchOptions, out eInfo)) > ReactorReturnCode.SUCCESS)
                        { }

                        // Graceful shutdown if Dispatch fails
                        if (ret != ReactorReturnCode.SUCCESS)
                        {
                            Console.WriteLine(eInfo?.Error.Text);
                            Uninitialize();
                            Environment.Exit((int)ReactorReturnCode.FAILURE);
                        }
                    }
                    else
                    {
                        // retrieve associated reactor channel and dispatch on that channel
                        ReactorChannel reactorChnl = GetChannelBySocketFd(sock.Handle.ToInt32());

                        ReactorReturnCode ret;
                        ReactorErrorInfo? eInfo;

                        // dispatch until no more messages
                        m_DispatchOptions.ReactorChannel = reactorChnl;
                        while ((ret = m_Reactor.Dispatch(m_DispatchOptions, out eInfo)) > ReactorReturnCode.SUCCESS)
                        { }

                        if (ret != ReactorReturnCode.SUCCESS)
                        {
                            if (reactorChnl.State != ReactorChannelState.CLOSED
                                && reactorChnl.State != ReactorChannelState.DOWN_RECONNECTING)
                            {
                                Console.WriteLine($"Reactor.Dispatch() failed: {ret} ({eInfo?.Error?.Text})");
                                Uninitialize();
                                Environment.Exit((int)ReactorReturnCode.FAILURE);
                            }
                        }
                    }
                }
            }

            if (m_ShutDown && !m_CloseHandled)
            {
                HandlePosting();

                // send login reissue if login reissue time has passed
                if (m_ChnlInfo!.CanSendLoginReissue &&
                    System.DateTime.Now >= m_ChnlInfo!.LoginReissueTime)
                {
                    LoginRequest loginRequest = m_ChnlInfo!.ConsumerRole.RdmLoginRequest!;
                    m_SubmitOptions.Clear();
                    var ret = m_ChnlInfo!.ReactorChannel!.Submit(loginRequest, m_SubmitOptions, out var errorInfo);
                    if (ret != ReactorReturnCode.SUCCESS)
                    {
                        Console.WriteLine("Login reissue failed. Error: " + errorInfo?.Error.Text);
                    }
                    else
                    {
                        Console.WriteLine("Login reissue sent");
                    }
                    m_ChnlInfo!.CanSendLoginReissue = false;
                }
            }

            if (m_CloseHandled)
                break;
        }
    }

    private void InitChannelInfo(ChannelInfo chnlInfo)
    {
        // set up consumer role
        chnlInfo.ConsumerRole.DefaultMsgCallback = this;
        chnlInfo.ConsumerRole.ChannelEventCallback = this;
        chnlInfo.ConsumerRole.LoginMsgCallback = this;
        chnlInfo.ConsumerRole.DirectoryMsgCallback = this;
        chnlInfo.ConsumerRole.WatchlistOptions.EnableWatchlist = true;
        chnlInfo.ConsumerRole.WatchlistOptions.ItemCountHint = 4;
        chnlInfo.ConsumerRole.WatchlistOptions.MaxOutstandingPosts = 5;
        chnlInfo.ConsumerRole.WatchlistOptions.ObeyOpenWindow = true;
        chnlInfo.ConsumerRole.WatchlistOptions.ChannelOpenEventCallback = this;

        if (m_ItemDecoder.FieldDictionaryLoadedFromFile == false &&
            m_ItemDecoder.EnumTypeDictionaryLoadedFromFile == false)
        {
            chnlInfo.ConsumerRole.DictionaryMsgCallback = this;
        }

        // initialize consumer role to default
        chnlInfo.ConsumerRole.InitDefaultRDMLoginRequest();
        chnlInfo.ConsumerRole.InitDefaultRDMDirectoryRequest();

        // use command line login user name if specified
        if (!string.IsNullOrEmpty(m_WatchlistConsumerConfig!.UserName))
        {
            chnlInfo.ConsumerRole.RdmLoginRequest!.UserName.Data(m_WatchlistConsumerConfig.UserName);
            m_ReactorServiceDiscoveryOptions.ProxyUserName.Data(m_WatchlistConsumerConfig.UserName);
        }

        if (!string.IsNullOrEmpty(m_WatchlistConsumerConfig.Password))
        {
            chnlInfo.ConsumerRole.RdmLoginRequest?.Password.Data(m_WatchlistConsumerConfig.Password);
            if (chnlInfo.ConsumerRole.RdmLoginRequest is not null)
                chnlInfo.ConsumerRole.RdmLoginRequest.HasPassword = true;
            m_ReactorServiceDiscoveryOptions.ProxyPassword.Data(m_WatchlistConsumerConfig.Password);
        }

        if (!string.IsNullOrEmpty(m_WatchlistConsumerConfig.ClientId))
        {
            m_ReactorOAuthCredential.ClientId.Data(m_WatchlistConsumerConfig.ClientId);
            chnlInfo.ConsumerRole.ReactorOAuthCredential = m_ReactorOAuthCredential;
            m_ReactorServiceDiscoveryOptions.ClientId.Data(m_WatchlistConsumerConfig.ClientId);
            m_ReactorServiceDiscoveryOptions.ClientSecret.Data(m_WatchlistConsumerConfig.ClientSecret);
            if (!string.IsNullOrEmpty(m_WatchlistConsumerConfig.ClientSecret))
            {
                m_ReactorOAuthCredential.ClientSecret.Data(m_WatchlistConsumerConfig.ClientSecret);
                m_ReactorServiceDiscoveryOptions.ClientSecret.Data(m_WatchlistConsumerConfig.ClientSecret);
            }
        }

        if (!string.IsNullOrEmpty(m_WatchlistConsumerConfig.JwkFile))
        {
            try
            {
                m_ReactorOAuthCredential.ClientJwk.Data(m_WatchlistConsumerConfig.JwkFile);
                chnlInfo.ConsumerRole.ReactorOAuthCredential = m_ReactorOAuthCredential;
                m_ReactorServiceDiscoveryOptions.ClientJwk.Data(m_WatchlistConsumerConfig.JwkFile);
            }
            catch (Exception e)
            {
                Console.WriteLine("Error loading JWK file: " + e.Message);
                Console.WriteLine();
                Console.WriteLine(CommandLine.OptionHelpString());
                Console.WriteLine("Consumer exits...");
                Environment.Exit((int)CodecReturnCode.FAILURE);
            }
        }

        if (!string.IsNullOrEmpty(m_WatchlistConsumerConfig.TokenScope))
        {
            m_ReactorOAuthCredential.TokenScope.Data(m_WatchlistConsumerConfig.TokenScope);
            chnlInfo.ConsumerRole.ReactorOAuthCredential = m_ReactorOAuthCredential;
        }

        if (!string.IsNullOrEmpty(m_WatchlistConsumerConfig.Audience))
        {
            m_ReactorOAuthCredential.Audience.Data(m_WatchlistConsumerConfig.Audience);
            m_ReactorServiceDiscoveryOptions.Audience.Data(m_WatchlistConsumerConfig.Audience);
            chnlInfo.ConsumerRole.ReactorOAuthCredential = m_ReactorOAuthCredential;
        }

        if (!string.IsNullOrEmpty(m_WatchlistConsumerConfig.ProxyHostname))
        {
            m_ReactorServiceDiscoveryOptions.ProxyHostName.Data(m_WatchlistConsumerConfig.ProxyHostname);
        }

        if (!string.IsNullOrEmpty(m_WatchlistConsumerConfig.ProxyPort))
        {
            m_ReactorServiceDiscoveryOptions.ProxyPort.Data(m_WatchlistConsumerConfig.ProxyPort);
        }

        if (!string.IsNullOrEmpty(m_WatchlistConsumerConfig.ProxyUsername))
        {
            m_ReactorServiceDiscoveryOptions.ProxyUserName.Data(m_WatchlistConsumerConfig.ProxyUsername);
        }

        if (!string.IsNullOrEmpty(m_WatchlistConsumerConfig.ProxyPassword))
        {
            m_ReactorServiceDiscoveryOptions.ProxyPassword.Data(m_WatchlistConsumerConfig.ProxyPassword);
        }

        string localIPaddress = "localhost";
        string? localHostName;

        try
        {
            localHostName = Dns.GetHostAddresses(Dns.GetHostName())
                        .Where(ip => ip.AddressFamily == AddressFamily.InterNetwork)
                        .Select(a => a.ToString()).FirstOrDefault();
        }
        catch (SocketException)
        {
            localHostName = localIPaddress;
        }

        // use command line authentication token and extended authentication information if specified
        if (!string.IsNullOrEmpty(m_WatchlistConsumerConfig.AuthenticationToken))
        {
            chnlInfo.ConsumerRole.RdmLoginRequest!.UserNameType = Login.UserIdTypes.AUTHN_TOKEN;
            chnlInfo.ConsumerRole.RdmLoginRequest.UserName.Data(m_WatchlistConsumerConfig.AuthenticationToken);

            if (!string.IsNullOrEmpty(m_WatchlistConsumerConfig.AuthenticationExtended))
            {
                chnlInfo.ConsumerRole.RdmLoginRequest.HasAuthenticationExtended = true;
                chnlInfo.ConsumerRole.RdmLoginRequest.AuthenticationExtended.Data(m_WatchlistConsumerConfig.AuthenticationExtended);
            }
        }

        // use command line application id if specified
        if (!string.IsNullOrEmpty(m_WatchlistConsumerConfig.ApplicationId))
        {
            chnlInfo.ConsumerRole.RdmLoginRequest?.LoginAttrib.ApplicationId.Data(m_WatchlistConsumerConfig.ApplicationId);
        }

        chnlInfo.ConsumerRole.RdmLoginRequest!.LoginAttrib.HasSingleOpen = true;
        chnlInfo.ConsumerRole.RdmLoginRequest!.LoginAttrib.SingleOpen = 1;
        chnlInfo.ConsumerRole.RdmLoginRequest!.LoginAttrib.HasAllowSuspectData = true;
        chnlInfo.ConsumerRole.RdmLoginRequest!.LoginAttrib.AllowSuspectData = 1;

        if (m_WatchlistConsumerConfig.EnableRTT)
        {
            chnlInfo.ConsumerRole.RdmLoginRequest!.LoginAttrib.HasSupportRoundTripLatencyMonitoring = true;
        }

        if (m_ItemDecoder.FieldDictionaryLoadedFromFile == true &&
            m_ItemDecoder.EnumTypeDictionaryLoadedFromFile == true)
        {
            chnlInfo.Dictionary = m_ItemDecoder.Dictionary;
        }

        chnlInfo.ShouldOffStreamPost = m_WatchlistConsumerConfig.EnableOffpost;
        chnlInfo.ShouldOnStreamPost = m_WatchlistConsumerConfig.EnablePost;

        if (chnlInfo.ShouldOnStreamPost)
        {
            bool mpItemFound = false;
            if (chnlInfo.ConnectionArg.ItemList != null)
            {
                foreach (var itemArg in chnlInfo.ConnectionArg.ItemList)
                {
                    if (itemArg.Domain == DomainType.MARKET_PRICE)
                    {
                        mpItemFound = true;
                        break;
                    }
                }
            }
            if (mpItemFound == false)
            {
                Console.WriteLine("\nPosting will not be performed for this channel as no Market Price items were requested");
                chnlInfo.ShouldOnStreamPost = false;
            }
        }

        chnlInfo.PostHandler.EnableOnStreamPost = chnlInfo.ShouldOnStreamPost;
        chnlInfo.PostHandler.EnableOffStreamPost = chnlInfo.ShouldOffStreamPost;

        // This sets up our basic timing so post messages will be sent
        // periodically
        chnlInfo.PostHandler.InitPostHandler();

        // set up reactor connect options
        chnlInfo.ConnectOptions.SetReconnectAttempLimit(-1); // attempt to recover forever
        chnlInfo.ConnectOptions.SetReconnectMinDelay(500); // 0.5 second minimum
        chnlInfo.ConnectOptions.SetReconnectMaxDelay(3000); // 3 second maximum
        chnlInfo.ConnectOptions.ConnectionList[0].ConnectOptions.MajorVersion = Codec.MajorVersion();
        chnlInfo.ConnectOptions.ConnectionList[0].ConnectOptions.MinorVersion = Codec.MinorVersion();
        chnlInfo.ConnectOptions.ConnectionList[0].ConnectOptions.ConnectionType = chnlInfo.ConnectionArg.ConnectionType;
        chnlInfo.ConnectOptions.ConnectionList[0].ConnectOptions.UserSpecObject = chnlInfo;
        chnlInfo.ConnectOptions.ConnectionList[0].ConnectOptions.GuaranteedOutputBuffers = 1000;

        chnlInfo.ConnectOptions.ConnectionList[0].EnableSessionManagement = m_WatchlistConsumerConfig.EnableSessionManagement;

        // handler encrypted
        chnlInfo.ShouldEnableEncrypted = m_WatchlistConsumerConfig.EnableEncrypted;

        ConnectOptions cOpt = chnlInfo.ConnectOptions.ConnectionList[0].ConnectOptions;

        if (chnlInfo.ShouldEnableEncrypted)
        {
            cOpt.ConnectionType = ConnectionType.ENCRYPTED;
            cOpt.EncryptionOpts.EncryptedProtocol = chnlInfo.ConnectionArg.ConnectionType;
            SetEncryptedConfiguration(cOpt);
        }

        /* Setup proxy info */
        if (m_WatchlistConsumerConfig.EnableProxy)
        {
            string? proxyHostName = m_WatchlistConsumerConfig.ProxyHostname;
            if (proxyHostName == null)
            {
                Console.Error.WriteLine("Error: Proxy hostname not provided.");
                Environment.Exit((int)CodecReturnCode.FAILURE);
            }
            string? proxyPort = m_WatchlistConsumerConfig.ProxyPort;
            if (proxyPort == null)
            {
                Console.Error.WriteLine("Error: Proxy port number not provided.");
                Environment.Exit((int)CodecReturnCode.FAILURE);
            }

            chnlInfo.ConnectOptions.ConnectionList[0].ConnectOptions.ProxyOptions.ProxyHostName = proxyHostName;
            try
            {
                chnlInfo.ConnectOptions.ConnectionList[0].ConnectOptions.ProxyOptions.ProxyPort = proxyPort;
            }
            catch (Exception)
            {
                Console.Error.WriteLine("Error: Proxy port number not provided.");
                Environment.Exit((int)CodecReturnCode.FAILURE);
            }
            SetCredentials(chnlInfo.ConnectOptions.ConnectionList[0].ConnectOptions);
        }

        if (m_WatchlistConsumerConfig.QueryEndpoint)
        {
            if (m_WatchlistConsumerConfig.EnableEncrypted)
            {
                m_ReactorServiceDiscoveryOptions.Transport = ReactorDiscoveryTransportProtocol.RD_TP_TCP;
            }
            else
            {
                Console.WriteLine("Error: Invalid connection type for " +
                                   m_WatchlistConsumerConfig.ConnectionList[0].ConnectionType +
                                   " querying RDP service discovery, only encrypted supported");
                Environment.Exit((int)ReactorReturnCode.FAILURE);
            }

            m_ReactorServiceDiscoveryOptions.ReactorServiceEndpointEventCallback = this;

            if (m_Reactor!.QueryServiceDiscovery(m_ReactorServiceDiscoveryOptions, out var errorInfo) != ReactorReturnCode.SUCCESS)
            {
                Console.WriteLine("Error: " + errorInfo?.Code + " Text: " + errorInfo?.Error.Text);
                return;
            }
        }

        if (chnlInfo.ConnectionArg.Port != null)
        {
            chnlInfo.ConnectOptions.ConnectionList[0].ConnectOptions.UnifiedNetworkInfo.ServiceName = chnlInfo.ConnectionArg.Port;
        }

        if (chnlInfo.ConnectionArg.Hostname != null)
        {
            chnlInfo.ConnectOptions.ConnectionList[0].ConnectOptions.UnifiedNetworkInfo.Address = chnlInfo.ConnectionArg.Hostname;
        }

        chnlInfo.ConnectOptions.ConnectionList[0].ConnectOptions.UnifiedNetworkInfo.InterfaceName = chnlInfo.ConnectionArg.InterfaceName;

        if (m_WatchlistConsumerConfig.Location != null)
            chnlInfo.ConnectOptions.ConnectionList[0].Location = m_WatchlistConsumerConfig.Location;
    }

    public ReactorCallbackReturnCode ReactorServiceEndpointEventCallback(ReactorServiceEndpointEvent reactorEvent)
    {
        if (reactorEvent.ReactorErrorInfo.Code == ReactorReturnCode.SUCCESS)
        {
            string? endPoint = null;
            string? port = null;
            List<ReactorServiceEndpointInfo> serviceEndpointInfoList = reactorEvent.ServiceEndpointInfoList!;

            foreach (var info in serviceEndpointInfoList)
            {
                if (info.LocationList.Count >= 2 && m_WatchlistConsumerConfig!.Location != null &&
                        info.LocationList[0].StartsWith(m_WatchlistConsumerConfig.Location)) // Get an endpoint that provides auto failover for the specified location
                {
                    endPoint = info?.EndPoint;
                    port = info?.Port;
                    break;
                }
                // Try to get backups and keep looking for main case. Keep only the first item met.
                else if (info.LocationList.Count > 0 && m_WatchlistConsumerConfig!.Location != null &&
                        info.LocationList[0].StartsWith(m_WatchlistConsumerConfig.Location) &&
                        endPoint == null && port == null)
                {
                    endPoint = info.EndPoint;
                    port = info.Port;
                }
            }

            if (endPoint is not null && port is not null)
            {
                if (m_WatchlistConsumerConfig is not null)
                {
                    m_WatchlistConsumerConfig.ConnectionList[0].Hostname = endPoint;
                    m_WatchlistConsumerConfig.ConnectionList[0].Port = port;
                }
            }
        }
        else
        {
            Console.WriteLine("Error requesting Service Discovery Endpoint Information: " + reactorEvent.ReactorErrorInfo.ToString());
            Environment.Exit((int)ReactorReturnCode.FAILURE);
        }

        return ReactorCallbackReturnCode.SUCCESS;
    }

    public ReactorCallbackReturnCode ReactorChannelEventCallback(ReactorChannelEvent reactorEvent)
    {
        ChannelInfo? chnlInfo = (ChannelInfo)reactorEvent.ReactorChannel!.UserSpecObj!;

        switch (reactorEvent.EventType)
        {
            case ReactorChannelEventType.CHANNEL_UP:
                {
                    if (reactorEvent.ReactorChannel.Channel != null)
                        Console.WriteLine("Channel Up Event: " + reactorEvent.ReactorChannel.Channel.HostName);
                    else
                        Console.WriteLine("Channel Up Event");
                    // register selector with channel event's reactorChannel
                    RegisterChannel(reactorEvent.ReactorChannel!);
                    break;
                }
            case ReactorChannelEventType.FD_CHANGE:
                {
                    int fdOldSocketId = reactorEvent.ReactorChannel!.OldSocket!.Handle.ToInt32();
                    int fdSocketId = reactorEvent.ReactorChannel!.Socket!.Handle.ToInt32();

                    Console.WriteLine($"Channel Change - Old Channel: {fdOldSocketId} New Channel: {fdSocketId}");

                    // cancel old reactorChannel select
                    UnregisterSocket(reactorEvent.ReactorChannel.OldSocket);

                    // register selector with channel event's new reactorChannel
                    RegisterChannel(reactorEvent.ReactorChannel);
                    break;
                }
            case ReactorChannelEventType.CHANNEL_READY:
                {
                    if (reactorEvent.ReactorChannel.Channel != null)
                        Console.WriteLine("Channel Ready Event: " + reactorEvent.ReactorChannel.Channel);
                    else
                        Console.WriteLine("Channel Ready Event");

                    if (IsRequestedServiceUp(chnlInfo))
                    {
                        CheckAndInitPostingSupport(chnlInfo);
                    }
                    break;
                }
            case ReactorChannelEventType.CHANNEL_OPENED:
                {
                    // set ReactorChannel on ChannelInfo, again need this?
                    chnlInfo.ReactorChannel = reactorEvent.ReactorChannel;

                    if (m_FieldDictionaryLoaded && m_EnumDictionaryLoaded
                        || m_ItemDecoder.FieldDictionaryLoadedFromFile && m_ItemDecoder.EnumTypeDictionaryLoadedFromFile)
                        m_ItemsRequested = false;
                    else
                        RequestDictionaries(reactorEvent.ReactorChannel, chnlInfo);

                    break;
                }
            case ReactorChannelEventType.CHANNEL_DOWN_RECONNECTING:
                {
                    if (reactorEvent.ReactorChannel?.Socket != null)
                        Console.WriteLine("\nConnection down reconnecting: Channel " + reactorEvent.ReactorChannel.Socket.Handle.ToInt32());
                    else
                        Console.WriteLine("\nConnection down reconnecting");

                    if (!string.IsNullOrEmpty(reactorEvent.ReactorErrorInfo?.Error.Text))
                        Console.WriteLine("\tError text: " + reactorEvent.ReactorErrorInfo.Error.Text + "\n");

                    // allow Reactor to perform connection recovery

                    // unregister selectableChannel from Selector
                    if (reactorEvent.ReactorChannel?.Socket != null)
                    {
                        UnregisterSocket(reactorEvent.ReactorChannel.Socket);
                    }

                    // reset hasServiceInfo flag
                    chnlInfo.HasServiceInfo = false;
                    m_ItemsRequested = false;

                    // reset canSendLoginReissue flag
                    chnlInfo.CanSendLoginReissue = false;
                    break;
                }
            case ReactorChannelEventType.CHANNEL_DOWN:
                {
                    if (reactorEvent.ReactorChannel!.Socket != null)
                        Console.WriteLine("\nConnection down: Channel " + reactorEvent.ReactorChannel.Socket.Handle.ToInt32());
                    else
                        Console.WriteLine("\nConnection down");

                    if (reactorEvent.ReactorErrorInfo != null && reactorEvent.ReactorErrorInfo.Error.Text != null)
                        Console.WriteLine("    Error text: " + reactorEvent.ReactorErrorInfo.Error.Text + "\n");

                    // unregister selectableChannel from Selector
                    if (reactorEvent.ReactorChannel!.Socket != null)
                    {
                        m_Sockets.Remove(reactorEvent.ReactorChannel.Socket);
                        m_SocketFdValueMap.Remove(reactorEvent.ReactorChannel.Socket.Handle.ToInt32());
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
                if (reactorEvent.ReactorErrorInfo != null && reactorEvent.ReactorErrorInfo.Error.Text != null)
                    Console.WriteLine("    Error text: " + reactorEvent.ReactorErrorInfo.Error.Text + "\n");

                break;

            default:
                {
                    Console.WriteLine("Unknown channel event!\n");
                    return ReactorCallbackReturnCode.SUCCESS;
                }
        }

        return ReactorCallbackReturnCode.SUCCESS;
    }

    public ReactorCallbackReturnCode DefaultMsgCallback(ReactorMsgEvent reactorEvent)
    {
        string? itemName = null;
        string GetItemName() => itemName ?? "null";
        State? itemState = null;
        WatchlistConsumerConfig.ItemInfo? item;

        ChannelInfo? chnlInfo = (ChannelInfo?)reactorEvent.ReactorChannel?.UserSpecObj;

        var msg = reactorEvent.Msg;

        if (msg == null)
        {
            /* The message is not present because an error occurred while decoding it. Print
             * the error and close the channel. If desired, the un-decoded message buffer
             * is available in reactorEvent.TransportBuffer(). */

            Console.Write("defaultMsgCallback: {0}({1})\n", reactorEvent.ReactorErrorInfo.Error.Text, reactorEvent.ReactorErrorInfo.Location);

            // close ReactorChannel
            if (chnlInfo?.ReactorChannel != null)
            {
                chnlInfo.ReactorChannel.Close(out _);
            }
            return ReactorCallbackReturnCode.SUCCESS;
        }

        item = m_WatchlistConsumerConfig!.GetItemInfo(msg.StreamId);

        switch (msg.MsgClass)
        {
            case MsgClasses.REFRESH:

                IRefreshMsg refreshMsg = (IRefreshMsg)msg;
                if (refreshMsg.CheckHasMsgKey())
                {
                    if (refreshMsg.MsgKey.CheckHasName())
                    {
                        itemName = refreshMsg.MsgKey.Name.ToString(); // Buffer
                        if (item == null && refreshMsg.StreamId < 0)
                        {
                            m_WatchlistConsumerConfig.AddProvidedItemInfo(refreshMsg.StreamId, refreshMsg.MsgKey,
                                    refreshMsg.DomainType);
                        }
                    }
                }
                else if (item != null)
                {
                    itemName = item?.Name?.ToString();
                }

                Console.WriteLine($"DefaultMsgCallback Refresh ItemName: {GetItemName()} Domain: {DomainTypes.ToString(refreshMsg.DomainType)}, StreamId: {refreshMsg.StreamId}");

                Console.WriteLine("                      State: " + refreshMsg.State);

                itemState = refreshMsg.State;
                /* Decode data body according to its domain. */
                if (reactorEvent.ReactorChannel is not null)
                    m_ItemDecoder.DecodeDataBody(reactorEvent.ReactorChannel, (Msg)refreshMsg);
                break;

            case MsgClasses.UPDATE:

                IUpdateMsg updateMsg = (IUpdateMsg)msg;
                if (updateMsg.CheckHasMsgKey() && updateMsg.MsgKey.CheckHasName())
                {
                    itemName = updateMsg.MsgKey.Name.ToString();
                }
                else if (item != null)
                {
                    itemName = item.Name?.ToString();
                }

                Console.WriteLine($"DefaultMsgCallback Update ItemName: {GetItemName()} Domain: {DomainTypes.ToString(updateMsg.DomainType)}, StreamId: {updateMsg.StreamId}");

                /* Decode data body according to its domain. */
                if (reactorEvent.ReactorChannel is not null)
                    m_ItemDecoder.DecodeDataBody(reactorEvent.ReactorChannel, (Msg)updateMsg);
                break;

            case MsgClasses.STATUS:
                IStatusMsg statusMsg = (IStatusMsg)msg;
                if (statusMsg.CheckHasMsgKey())
                {
                    if (statusMsg.MsgKey.CheckHasName())
                    {
                        itemName = statusMsg.MsgKey.Name.ToString();
                        if (item != null && statusMsg.StreamId < 0)
                        {
                            m_WatchlistConsumerConfig.AddProvidedItemInfo(statusMsg.StreamId, statusMsg.MsgKey, statusMsg.DomainType);
                        }
                    }
                }
                else if (item != null)
                {
                    itemName = item.Name?.ToString();
                }

                Console.WriteLine($"DefaultMsgCallback Status -- ItemName: {GetItemName()} Domain: {DomainTypes.ToString(statusMsg.DomainType)}, StreamId: {statusMsg.StreamId}");

                if (statusMsg.CheckHasState())
                {
                    Console.WriteLine(statusMsg.State);

                    itemState = statusMsg.State;
                }

                break;

            case MsgClasses.ACK:

                IAckMsg ackMsg = (IAckMsg)msg;
                if (ackMsg.CheckHasMsgKey())
                {
                    if (ackMsg.MsgKey.CheckHasName())
                    {
                        itemName = ackMsg.MsgKey.Name.ToString();
                    }
                }
                else if (item != null)
                {
                    itemName = item.Name?.ToString();
                }
                Console.WriteLine($"DefaultMsgCallback Ack --  ItemName: {GetItemName()} Domain: {DomainTypes.ToString(ackMsg.DomainType)}, StreamId: {ackMsg.StreamId}");
                Console.WriteLine(" ackId: " + ackMsg.AckId);
                if (ackMsg.CheckHasSeqNum())
                {
                    Console.WriteLine(" seqNum: " + ackMsg.SeqNum);
                }
                if (ackMsg.CheckHasNakCode())
                {
                    Console.WriteLine(" nakCode: " + ackMsg.NakCode);
                }
                if (ackMsg.CheckHasText())
                {
                    Console.WriteLine(" text: " + ackMsg.Text);
                }
                break;

            default:
                Console.WriteLine("Received Unhandled Item Msg Class: " + msg.MsgClass);
                break;
        }

        if (itemState != null && item != null)
        {
            /* Check state of any provider-driven streams.
             * If the state indicates the item was closed, remove it from our list. */
            if (msg.StreamId < 0 && itemState.StreamState() != StreamStates.OPEN)
                m_WatchlistConsumerConfig.RemoveProvidedItemInfo(item);

            /* Update item state. */
            else
                itemState.Copy(item.State);
        }

        return ReactorCallbackReturnCode.SUCCESS;
    }

    public ReactorCallbackReturnCode RdmLoginMsgCallback(RDMLoginMsgEvent reactorEvent)
    {
        var chnlInfo = (ChannelInfo)reactorEvent.ReactorChannel!.UserSpecObj!;
        var msgType = reactorEvent.LoginMsg!.LoginMsgType;

        switch (msgType)
        {
            case REFRESH:
                Console.WriteLine("Received Login Refresh for Username: " + reactorEvent.LoginMsg.LoginRefresh!.UserName);
                Console.WriteLine(reactorEvent.LoginMsg.ToString());

                // save loginRefresh
                reactorEvent.LoginMsg.LoginRefresh.Copy(chnlInfo.LoginRefresh);

                Console.WriteLine($"Domain: {DomainTypes.ToString(DomainType.LOGIN)}, StreamId: {reactorEvent.LoginMsg.LoginRefresh!.StreamId}");

                Console.WriteLine(" State: " + chnlInfo.LoginRefresh.State);
                if (chnlInfo.LoginRefresh.HasUserName)
                    Console.WriteLine(" UserName: " + chnlInfo.LoginRefresh.UserName.ToString());

                // get login reissue time from authenticationTTReissue
                if (chnlInfo.LoginRefresh.HasAuthenicationTTReissue)
                {
                    chnlInfo.LoginReissueTime = DateTimeOffset.FromUnixTimeSeconds(chnlInfo.LoginRefresh.AuthenticationTTReissue).DateTime;
                    chnlInfo.CanSendLoginReissue = m_WatchlistConsumerConfig!.EnableSessionManagement ? false : true;
                }

                break;

            case STATUS:
                LoginStatus loginStatus = reactorEvent.LoginMsg.LoginStatus!;
                Console.WriteLine($"Domain: {DomainTypes.ToString(DomainType.LOGIN)}, StreamId: {reactorEvent.LoginMsg.LoginStatus?.StreamId}");
                Console.WriteLine("Received Login StatusMsg");
                if (loginStatus.HasState)

                {
                    Console.WriteLine("	" + loginStatus.State);
                }
                if (loginStatus.HasUserName)
                    Console.WriteLine(" UserName: " + loginStatus.UserName.ToString());

                break;

            case RTT:
                LoginRTT? loginRTT = reactorEvent.LoginMsg?.LoginRTT;
                var id = m_SocketFdValueMap.FirstOrDefault(kv => reactorEvent.ReactorChannel == kv.Value);
                Console.Write($"\nReceived login RTT message from Provider {id}.\n");
                Console.Write($"\tTicks: {loginRTT?.Ticks / 1000}u\n");
                if (loginRTT?.HasRTLatency ?? false)
                {
                    long calculatedRtt = loginRTT.CalculateRTTLatency(LoginRTT.TimeUnit.MICRO_SECONDS);
                    Console.Write($"\tLast Latency: {calculatedRtt}u\n");
                }
                if (loginRTT?.HasTCPRetrans ?? false)
                {
                    Console.Write("\tProvider side TCP Retransmissions: {0}u\n", loginRTT.TCPRetrans);
                }
                Console.Write("RTT Response sent to provider by watchlist.\n\n");
                break;

            default:
                Console.WriteLine("Received Unhandled Login Msg Type: " + msgType);
                break;
        }

        Console.WriteLine();

        return ReactorCallbackReturnCode.SUCCESS;
    }

    public ReactorCallbackReturnCode RdmDirectoryMsgCallback(RDMDirectoryMsgEvent reactorEvent)
    {
        ChannelInfo? chnlInfo = (ChannelInfo?)reactorEvent.ReactorChannel?.UserSpecObj;
        DirectoryMsgType msgType = reactorEvent.DirectoryMsg!.DirectoryMsgType;
        List<Service>? serviceList = null;

        switch (msgType)
        {
            case DirectoryMsgType.REFRESH:
                DirectoryRefresh directoryRefresh = reactorEvent.DirectoryMsg!.DirectoryRefresh!;
                Console.WriteLine("Domain: " + DomainTypes.ToString(DomainType.SOURCE));
                Console.WriteLine("Stream: " + reactorEvent.DirectoryMsg.StreamId + " Msg Class: " + MsgClasses.ToString(MsgClasses.REFRESH));
                Console.WriteLine(directoryRefresh.State.ToString());

                serviceList = directoryRefresh.ServiceList;
                string? serviceName = chnlInfo?.ConnectionArg.Service;

                foreach (var service in serviceList)
                {
                    if (service.Info.ServiceName.ToString() != null)
                    {
                        if (service.Info.ServiceName.ToString().Equals(serviceName))
                        {
                            // save serviceInfo associated with requested service name
                            if (service.Copy(chnlInfo!.ServiceInfo) < CodecReturnCode.SUCCESS)
                            {
                                Console.WriteLine("Service.Copy() failure");
                                Uninitialize();
                                Environment.Exit((int)ReactorReturnCode.FAILURE);
                            }
                            chnlInfo.HasServiceInfo = true;
                        }
                    }
                }
                break;

            case DirectoryMsgType.UPDATE:
                DirectoryUpdate? directoryUpdate = reactorEvent.DirectoryMsg?.DirectoryUpdate;

                serviceName = chnlInfo!.ConnectionArg.Service;
                Console.WriteLine("Received Source Directory Update");
                Console.WriteLine(directoryUpdate?.ToString());

                Console.WriteLine("Domain: " + DomainTypes.ToString(DomainType.SOURCE));
                Console.WriteLine($"Stream: {reactorEvent.Msg?.StreamId} Msg Class: {MsgClasses.ToString(MsgClasses.UPDATE)}");

                serviceList = directoryUpdate?.ServiceList;
                if (serviceList is not null)
                    foreach (var service in serviceList)
                    {
                        if (service.Action == MapEntryActions.DELETE && service.ServiceId == chnlInfo.ServiceInfo.ServiceId)
                        {
                            chnlInfo.ServiceInfo.Action = MapEntryActions.DELETE;
                        }

                        bool updateServiceInfo = false;
                        if (service.Info.ServiceName.ToString() != null)
                        {
                            Console.WriteLine("Received serviceName: " + service.Info.ServiceName + "\n");
                            // update service cache - assume cache is built with previous refresh message
                            if (service.Info.ServiceName.ToString().Equals(serviceName) ||
                                service.ServiceId == chnlInfo.ServiceInfo.ServiceId)
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
                        }
                    }

                break;

            case DirectoryMsgType.CLOSE:
                Console.WriteLine("Received Source Directory Close");
                break;

            case DirectoryMsgType.STATUS:
                DirectoryStatus? directoryStatus = reactorEvent.DirectoryMsg.DirectoryStatus;
                Console.WriteLine("Received Source Directory StatusMsg");
                Console.WriteLine("Domain: " + DomainTypes.ToString(DomainType.SOURCE));
                Console.WriteLine($"Stream: {reactorEvent.DirectoryMsg.DirectoryStatus?.StreamId} Msg Class: {MsgClasses.ToString(MsgClasses.STATUS)}");
                Console.WriteLine(directoryStatus?.State.ToString());
                if (directoryStatus?.HasState ?? false)
                {
                    Console.WriteLine("	" + directoryStatus.State);
                }
                break;

            default:
                Console.WriteLine("Received Unhandled Source Directory Msg Type: " + msgType);
                break;
        }

        /* Refresh and update messages contain updates to service information. */
        if (serviceList != null)
        {
            foreach (var service in serviceList)
            {
                Console.WriteLine($" Service = {service.ServiceId} Action: {MapEntryActionsExtensions.ToString(service.Action)}");
            }
        }

        Console.WriteLine("");

        return ReactorCallbackReturnCode.SUCCESS;
    }

    public ReactorCallbackReturnCode RdmDictionaryMsgCallback(RDMDictionaryMsgEvent reactorEvent)
    {
        ChannelInfo? chnlInfo = (ChannelInfo?)reactorEvent.ReactorChannel?.UserSpecObj;
        DictionaryMsgType? msgType = reactorEvent.DictionaryMsg?.DictionaryMsgType;

        // initialize dictionary
        if (chnlInfo is not null && chnlInfo.Dictionary == null)
        {
            chnlInfo.Dictionary = new();
        }

        switch (msgType)
        {
            case DictionaryMsgType.REFRESH:
                DictionaryRefresh dictionaryRefresh = reactorEvent.DictionaryMsg?.DictionaryRefresh!;

                if (dictionaryRefresh.HasInfo)
                {
                    /* The first part of a dictionary refresh should contain information about its type.
                     * Save this information and use it as subsequent parts arrive. */
                    switch (dictionaryRefresh.DictionaryType)
                    {
                        case Types.FIELD_DEFINITIONS:
                            m_FieldDictionaryLoaded = false;
                            chnlInfo!.FieldDictionaryStreamId = dictionaryRefresh.StreamId;
                            break;

                        case Types.ENUM_TABLES:
                            m_EnumDictionaryLoaded = false;
                            chnlInfo!.EnumDictionaryStreamId = dictionaryRefresh.StreamId;
                            break;

                        default:
                            Console.WriteLine($"Unknown dictionary type {dictionaryRefresh.DictionaryType} from message on stream {dictionaryRefresh.StreamId}");
                            chnlInfo?.ReactorChannel?.Close(out _);
                            return ReactorCallbackReturnCode.SUCCESS;
                    }
                }

                /* decode dictionary response */

                // clear decode iterator
                chnlInfo!.DIter.Clear();

                // set buffer and version info
                chnlInfo.DIter.SetBufferAndRWFVersion(dictionaryRefresh.DataBody,
                            reactorEvent.ReactorChannel!.MajorVersion,
                            reactorEvent.ReactorChannel.MinorVersion);

                Console.WriteLine("Received Dictionary Response: " + dictionaryRefresh.DictionaryName);

                if (dictionaryRefresh.StreamId == chnlInfo.FieldDictionaryStreamId)
                {
                    if (chnlInfo.Dictionary.DecodeFieldDictionary(chnlInfo.DIter, VerbosityValues.VERBOSE, out var decodeFieldError) == CodecReturnCode.SUCCESS)
                    {
                        if (dictionaryRefresh.RefreshComplete)
                        {
                            m_FieldDictionaryLoaded = true;
                            m_ItemDecoder.FieldDictionaryDownloadedFromNetwork = true;
                            m_ItemDecoder.Dictionary = chnlInfo.Dictionary;
                            Console.WriteLine("Field Dictionary complete.");
                        }
                    }
                    else
                    {
                        Console.WriteLine("Decoding Field Dictionary failed: " + decodeFieldError.Text);
                        if (chnlInfo!.ReactorChannel!.Close(out var closeError) != ReactorReturnCode.SUCCESS)
                        {
                            Console.WriteLine("ReactorChannel.Close() failed: " + closeError?.Error.Text);
                        }
                    }
                }
                else if (dictionaryRefresh.StreamId == chnlInfo.EnumDictionaryStreamId)
                {
                    if (chnlInfo.Dictionary.DecodeEnumTypeDictionary(chnlInfo.DIter, VerbosityValues.VERBOSE, out var decodeEnumError) == CodecReturnCode.SUCCESS)
                    {
                        if (dictionaryRefresh.RefreshComplete)
                        {
                            m_EnumDictionaryLoaded = true;
                            m_ItemDecoder.EnumTypeDictionaryDownloadedFromNetwork = true;
                            m_ItemDecoder.Dictionary = chnlInfo.Dictionary;
                            Console.WriteLine("EnumType Dictionary complete.");
                        }
                    }
                    else
                    {
                        Console.WriteLine("Decoding EnumType Dictionary failed: " + decodeEnumError.Text);

                        if (chnlInfo!.ReactorChannel!.Close(out var closeError) != ReactorReturnCode.SUCCESS)
                        {
                            Console.WriteLine("ReactorChannel.Close() failed: " + closeError?.Error.Text);
                        }
                    }
                }
                else
                {
                    Console.WriteLine("Received unexpected dictionary message on stream " + dictionaryRefresh.StreamId);
                }

                if (m_FieldDictionaryLoaded && m_EnumDictionaryLoaded)
                    m_ItemsRequested = false;

                break;

            case DictionaryMsgType.STATUS:
                DictionaryStatus dictionaryStatus = reactorEvent.DictionaryMsg?.DictionaryStatus!;

                if (dictionaryStatus.StreamId == chnlInfo?.FieldDictionaryStreamId)
                {
                    Console.WriteLine("Received Dictionary StatusMsg for RWFFld, streamId: " + chnlInfo.FieldDictionaryStreamId);
                }
                else if (dictionaryStatus.StreamId == chnlInfo?.EnumDictionaryStreamId)
                {
                    Console.WriteLine("Received Dictionary StatusMsg for RWFEnum, streamId: " + chnlInfo.EnumDictionaryStreamId);
                }
                if (dictionaryStatus.HasState)
                {
                    Console.WriteLine(dictionaryStatus.State);
                }
                break;

            default:
                Console.WriteLine("Received Unhandled Dictionary Msg Type: " + msgType);
                break;
        }

        Console.WriteLine("");

        return ReactorCallbackReturnCode.SUCCESS;
    }

    private void RequestDictionaries(ReactorChannel channel, ChannelInfo chnlInfo)
    {
        Msg msg = new()
        {
            MsgClass = MsgClasses.REQUEST
        };

        /* set-up message */
        msg.ApplyStreaming();
        msg.StreamId = FIELD_DICTIONARY_STREAM_ID;
        chnlInfo.FieldDictionaryStreamId = FIELD_DICTIONARY_STREAM_ID;
        msg.DomainType = (int)DomainType.DICTIONARY;
        msg.ContainerType = DataTypes.NO_DATA;
        msg.MsgKey.ApplyHasNameType();
        msg.MsgKey.ApplyHasName();
        msg.MsgKey.ApplyHasFilter();
        msg.MsgKey.Filter = VerbosityValues.NORMAL;
        msg.MsgKey.Name.Data(FIELD_DICTIONARY_DOWNLOAD_NAME);

        ReactorSubmitOptions submitOptions = new()
        {
            ServiceName = m_WatchlistConsumerConfig!.ServiceName
        };

        if (channel.Submit(msg, submitOptions, out var errorInfo) != ReactorReturnCode.SUCCESS)
        {
            Console.WriteLine("ReactorChannel.Submit() failed: " + errorInfo?.Error.Text);
        }

        msg.StreamId = ENUM_DICTIONARY_STREAM_ID;
        chnlInfo.EnumDictionaryStreamId = ENUM_DICTIONARY_STREAM_ID;
        msg.MsgKey.Name.Data(ENUM_TABLE_DOWNLOAD_NAME);

        if (channel.Submit(msg, submitOptions, out var eInfo) != ReactorReturnCode.SUCCESS)
        {
            Console.WriteLine("ReactorChannel.Submit() failed: " + eInfo?.Error.Text);
        }
    }
    private void CheckAndInitPostingSupport(ChannelInfo chnlInfo)
    {
        if (!(chnlInfo.ShouldOnStreamPost || chnlInfo.ShouldOffStreamPost))
            return;

        // set up posting if its enabled

        // ensure that provider supports posting - if not, disable posting
        if (!chnlInfo.LoginRefresh.HasFeatures ||
            !chnlInfo.LoginRefresh.SupportedFeatures.HasSupportPost ||
            chnlInfo.LoginRefresh.SupportedFeatures.SupportOMMPost == 0)
        {
            // provider does not support posting, disable it
            chnlInfo.ShouldOffStreamPost = false;
            chnlInfo.ShouldOnStreamPost = false;
            chnlInfo.PostHandler.EnableOnStreamPost = false;
            chnlInfo.PostHandler.EnableOffStreamPost = false;
            Console.WriteLine("Connected Provider does not support OMM Posting.  Disabling Post functionality.");
            return;
        }

        if (m_WatchlistConsumerConfig!.PublisherId != null && m_WatchlistConsumerConfig.PublisherAddress != null)
            chnlInfo.PostHandler.SetPublisherInfo(m_WatchlistConsumerConfig.PublisherId, m_WatchlistConsumerConfig.PublisherAddress);
    }

    private void RegisterChannel(ReactorChannel chan)
    {
        m_Sockets.Add(chan.Socket!);
        m_SocketFdValueMap.Add(chan.Socket!.Handle.ToInt32(), chan);
    }

    private void UnregisterSocket(Socket sock)
    {
        m_Sockets.Remove(sock);
        m_SocketFdValueMap.Remove(sock.Handle.ToInt32());
    }
    private ReactorChannel GetChannelBySocketFd(int fd) => m_SocketFdValueMap[fd];


    /// <summary>
    /// On and off stream posting if enabled
    /// </summary>
    private void HandlePosting()
    {
         m_ChnlInfo!.PostItemName.Clear();

         if (m_ChnlInfo.PostHandler.EnableOnStreamPost)
         {
             WatchlistConsumerConfig.ItemInfo? postingItem = null;

             // Find a first MarketPrice item
             // If found, send on-stream posts on it.
             for (int i = 0; i < m_WatchlistConsumerConfig!.ItemCount; i++)
             {
                 if (m_WatchlistConsumerConfig.ItemList[i].Domain == (int)DomainType.MARKET_PRICE)
                 {
                     postingItem = m_WatchlistConsumerConfig.ItemList[i];
                     if (m_WatchlistConsumerConfig.ItemList[i].State.StreamState() != StreamStates.OPEN ||
                        m_WatchlistConsumerConfig.ItemList[i].State.DataState() != DataStates.OK)
                     {
                         Console.WriteLine("No currently available Market Price streams to on-stream post to.  Will retry shortly.");
                         return;
                     }
                     break;
                 }
             }

             if (postingItem == null)
             {
                 Console.WriteLine("No currently available Market Price streams to on-stream post to.  Will retry shortly.\n");
                 return;
             }

             m_ChnlInfo.PostHandler.StreamId = postingItem.StreamId;
             m_ChnlInfo.PostHandler.PostItemName.Data(postingItem.Name);
             m_ChnlInfo.PostHandler.ServiceId = m_ChnlInfo.ServiceInfo.ServiceId;
             m_ChnlInfo.PostHandler.Dictionary = m_ChnlInfo.Dictionary;

             CodecReturnCode ret = m_ChnlInfo.PostHandler.HandlePosts(m_ChnlInfo!.ReactorChannel!, out var errorInfo);
             if (ret < CodecReturnCode.SUCCESS)
                 Console.WriteLine("Error posting onstream: " + errorInfo?.Error.Text);
         }
         if (m_ChnlInfo.PostHandler.EnableOffStreamPost)
         {
             m_ChnlInfo.PostHandler.StreamId = m_ChnlInfo.LoginRefresh.StreamId;
             m_ChnlInfo.PostHandler.PostItemName.Data("OFFPOST");
             m_ChnlInfo.PostHandler.ServiceId = m_ChnlInfo.ServiceInfo.ServiceId;
             m_ChnlInfo.PostHandler.Dictionary = m_ChnlInfo.Dictionary;
             CodecReturnCode ret = m_ChnlInfo.PostHandler.HandlePosts(m_ChnlInfo!.ReactorChannel!, out var errorInfo);
             if (ret < CodecReturnCode.SUCCESS)
                 Console.WriteLine("Error posting offstream: " + errorInfo?.Error.Text);
         }
    }


    private void CloseItemStreams(ChannelInfo? chnlInfo)
    {
        // have offstream posting post close status
        if (chnlInfo!.ShouldOffStreamPost)
        {
            chnlInfo.PostHandler.StreamId = chnlInfo.LoginRefresh.StreamId;
            chnlInfo.PostHandler.PostItemName.Data("OFFPOST");
            chnlInfo.PostHandler.ServiceId = chnlInfo.ServiceInfo.ServiceId;
            chnlInfo.PostHandler.Dictionary = chnlInfo.Dictionary;
            if (chnlInfo!.PostHandler!.CloseOffStreamPost(chnlInfo!.ReactorChannel!, out var errorInfo) != CodecReturnCode.SUCCESS)
            {
                Console.WriteLine("PostHandler.CloseOffStreamPost() failed: " + errorInfo?.Error.Text);
            }
        }

        for (int itemListIndex = 0; itemListIndex < m_WatchlistConsumerConfig!.ItemCount; ++itemListIndex)
        {
            int domainType = (int)m_WatchlistConsumerConfig.ItemList[itemListIndex].Domain;
            int streamId = m_WatchlistConsumerConfig.ItemList[itemListIndex].StreamId;

            /* encode item close */
            m_CloseMsg.Clear();
            m_CloseMsg.MsgClass = MsgClasses.CLOSE;
            m_CloseMsg.StreamId = streamId;
            m_CloseMsg.DomainType = domainType;
            m_CloseMsg.ContainerType = DataTypes.NO_DATA;
            if (chnlInfo!.ReactorChannel!.Submit((Msg)m_CloseMsg, m_SubmitOptions, out var erroInfo) != ReactorReturnCode.SUCCESS)
            {
                Console.WriteLine($"Close itemStream of {streamId} failed: {erroInfo?.Error.Text}");
            }
        }
    }

    internal void Uninitialize()
    {
        Console.WriteLine("Consumer unitializing and exiting...");

        // close items streams
        CloseItemStreams(m_ChnlInfo);

        // close ReactorChannel
        if (m_ChnlInfo!.ReactorChannel != null)
        {
            m_ChnlInfo?.ReactorChannel?.Close(out _);
        }

        m_FileStream?.Dispose();

        // shutdown reactor
        m_Reactor?.Shutdown(out _);
    }

    private static void SetEncryptedConfiguration(ConnectOptions options)
    {
        options.EncryptionOpts.EncryptionProtocolFlags = EncryptionProtocolFlags.ENC_TLSV1_2;
        options.EncryptionOpts.EncryptedProtocol = ConnectionType.SOCKET;
    }

    private void SetCredentials(ConnectOptions options)
    {
        string? proxyUsername = m_WatchlistConsumerConfig!.ProxyUsername;
        if (proxyUsername == null)
        {
            Console.Error.WriteLine("Error: Proxy username not provided.");
            Environment.Exit((int)CodecReturnCode.FAILURE);
        }

        string? proxyPasswd = m_WatchlistConsumerConfig.ProxyPassword;
        if (m_WatchlistConsumerConfig.ProxyPassword is null)
        {
            Console.Error.WriteLine("Error: Proxy password not provided.");
            Environment.Exit((int)CodecReturnCode.FAILURE);
        }

        options.ProxyOptions.ProxyUserName = proxyUsername;
        options.ProxyOptions.ProxyPassword = proxyPasswd;
    }

    public bool IsRequestedServiceUp(ChannelInfo? chnlInfo)
    {
        return chnlInfo!.HasServiceInfo &&
                chnlInfo.ServiceInfo.HasState && (!chnlInfo.ServiceInfo.State.HasAcceptingRequests ||
                                                         chnlInfo.ServiceInfo.State.AcceptingRequests == 1) && chnlInfo.ServiceInfo.State.ServiceStateVal == 1;
    }

    public bool IsDictionaryReady()
    {
        return (m_FieldDictionaryLoaded && m_EnumDictionaryLoaded) || (m_ItemDecoder.FieldDictionaryLoadedFromFile && m_ItemDecoder.EnumTypeDictionaryLoadedFromFile);
    }
}
