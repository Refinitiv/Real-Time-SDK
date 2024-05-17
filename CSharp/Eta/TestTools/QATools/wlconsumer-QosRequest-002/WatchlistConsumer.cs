/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.Md for details.                  --
 *|           Copyright (C) 2023 Refinitiv. All rights reserved.         --
 *|-----------------------------------------------------------------------------
 */

namespace LSEG.Eta.ValueAdd.WatchlistConsumer;
using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Net;
using System.Net.Sockets;

using LSEG.Eta.Codec;
using LSEG.Eta.Common;
using LSEG.Eta.Example.Common;
using LSEG.Eta.Rdm;
using LSEG.Eta.Transports;
using LSEG.Eta.ValueAdd.Rdm;
using LSEG.Eta.ValueAdd.Reactor;

using static LSEG.Eta.Rdm.Dictionary;
using static LSEG.Eta.ValueAdd.WatchlistConsumer.WatchlistConsumerConfig;
using static Rdm.LoginMsgType;
using ItemInfo = WatchlistConsumerConfig.ItemInfo;

/// <summary>
/// <p>
/// This is a main class to run the ETA Value Add WatchlistConsumer application.
/// </p>
/// <H2>Summary</H2>
/// <p>
/// This is the main file for the WatchlistConsumer application.  It is a single-threaded
/// client application that utilizes the ETA Reactor's watchlist to provide recovery of data.
/// The main consumer file provides the callback for channel events and
/// the default callback for processing RsslMsgs. The main function
/// Initializes the ETA Reactor, makes the desired connections, and
/// dispatches for events.
/// This application makes use of the RDM namespace for easier decoding of Login &amp; Source Directory
/// messages.
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
/// The RDMFieldDictionary and enumtype.Def files could be located in the
/// directory of execution or this application will request dictionary from
/// provider.
/// </p>
/// <p>
/// Arguments:
/// </p>
/// <ul>
/// <li>-h Server host name. Default is <i>localhost</i>.
/// <li>-p Server port number. Default is <i>14002</i>.
/// <li>-u Login user name. Default is system user name.
/// <li>-s Service name. Default is <i>DIRECT_FEED</i>.
/// <li>-mp Market Price domain item name. Default is <i>TRI.N</i>. The user can
/// specify multiple -mp instances, where each occurrence is associated with a
/// single item. For example, specifying -mp TRI -mp GOOG will provide content
/// for two MarketPrice items.
/// <li>-mbo Market By Order domain item name. No default. The user can specify
/// multiple -mbo instances, where each occurrence is associated with a single
/// item.
/// <li>-mbp market By Price domain item name. No default. The user can specify
/// multiple -mbp instances, where each occurrence is associated with a single
/// item.
/// <li>-yc Yield Curve domain item name. No default. The user can specify
/// multiple -yc instances, where each occurrence is associated with a
/// single item.
/// <li>-view viewFlag. Default is false. If true, each request will use a basic
/// dynamic view.
/// <li>-post postFlag. Default is false. If true, the application will attempt
/// to send post messages on the first requested Market Price item (i.E.,
/// on-stream).
/// <li>-offpost offPostFlag. Default is false. If true, the application will
/// attempt to send post messages on the login stream (i.E., off-stream).
/// <li>-publisherInfo publisherInfoFlag. Default is false. If true, the application will
/// attempt to send post messages with publisher ID and publisher address.
/// <li>-snapshot snapShotFlag. If true, each request will be non-streaming (i.E.
/// snapshot). Default is false (i.E. streaming requests).
/// <li>-sl symbolListName. symbolListName is optional without a default. If -sl
/// is specified without a ListName the itemList from the source directory
/// response will be used.
/// <li>-sld Requests item on the Symbol List domain and data streams for items on that list.
/// <li>-x Provides XML tracing of messages.
/// <li>-c Connection Type used (Socket or encrypted).
/// Default is <i>Socket</i>.
/// <li>-runTime run time. Default is 600 seconds. Controls the time the
/// application will run before exiting, in seconds.
/// <li>-proxy proxyFlag. if provided, the application will attempt
/// to make a connection through a proxy server 
/// <li>-ph Proxy host name.
/// <li>-pp Proxy port number.
/// <li>-plogin User name on proxy server.
/// <li>-ppasswd Password on proxy server.
/// <li>-pdomain Proxy Domain.
/// <li>-krbfile Proxy KRB file.
/// <li>-keyfile keystore file for encryption.
/// <li>-keypasswd keystore password for encryption.
/// <li>-tunnel (optional) enables consumer to open tunnel stream and send basic text messages
/// <li>-at Specifies the Authentication Token. If this is present, the login user name type will be Login.UserIdTypes.AUTHN_TOKEN.
/// <li>-ax Specifies the Authentication Extended information.
/// <li>-aid Specifies the Application ID.
/// <li>-sessionMgnt (optional) Enable Session Management in the reactor.
/// <li>-l (optional) Specifies a location to get an endpoint from service endpoint information. Defaults to us-east-1.
/// <li>-query (optional) Queries RDP service discovery to get an endpoint according to a specified connection type and location.
/// <li>-clientId Specifies the client Id for Refinitiv login V2, or specifies a unique ID with login V1 for applications making the request to EDP token service, this is also known as AppKey generated using an AppGenerator.
/// <li>-clientSecret Specifies the associated client Secret with a provided clientId for V2 logins.
/// <li>-jwkFile Specifies the file containing the JWK encoded key for V2 JWT logins.
/// <li>-tokenURL Specifies the token URL for V2 token oauthclientcreds grant type.
/// <li>-restEnableLog (optional) Enable REST logging message.
/// <li>-restLogFileName Set REST logging output stream.
/// <li>-rtt enables rtt support by a consumer. If provider make distribution of RTT messages, consumer will return back them. In another case, consumer will ignore them.
/// </ul>
/// </summary>
public class WatchlistConsumer : IConsumerCallback, IReactorServiceEndpointEventCallback
{
    private const string FIELD_DICTIONARY_DOWNLOAD_NAME = "RWFFld";
    private const string ENUM_TABLE_DOWNLOAD_NAME = "RWFEnum";
    private const int FIELD_DICTIONARY_STREAM_ID = 3;
    private const int ENUM_DICTIONARY_STREAM_ID = 4;

    private Reactor? m_Reactor;
    private readonly ReactorOptions m_ReactorOptions = new();
    private readonly ReactorDispatchOptions m_DispatchOptions = new();
    private WatchlistConsumerConfig m_WatchlistConsumerConfig;

    private System.DateTime m_Runtime;

    private readonly ReactorSubmitOptions m_SubmitOptions = new();
    private readonly ReactorServiceDiscoveryOptions m_ReactorServiceDiscoveryOptions = new();
    private readonly ReactorOAuthCredential m_ReactorOAuthCredential = new();
    private readonly List<ChannelInfo> m_ChnlInfoList = new();

    private readonly TimeSpan m_Closetime = TimeSpan.FromSeconds(10);
    private System.DateTime m_CloseRunTime;
    private bool m_CloseHandled;
    private readonly ItemDecoder m_ItemDecoder = new();
    private bool m_ItemsRequested = false;

    private readonly EncodeIterator m_EncIter = new();

    private readonly List<int> m_ViewFieldList = new();
    private bool m_FieldDictionaryLoaded;
    private bool m_EnumDictionaryLoaded;

    private readonly ICloseMsg m_CloseMsg = new Msg();

    private readonly ItemRequest m_ItemRequest = new();
    private Eta.Codec.Buffer? m_Payload;

    private FileStream? m_FileStream;

    private readonly Dictionary<int, ReactorChannel> m_SocketFdValueMap = new();
    private readonly List<Socket> m_Sockets = new();

    // APIQA: adding newItemRequested which keeps track of number of "new" items
    // (requested after initla request) requested
    private int m_NewItemRequested = 0;

    //APIQA:  adding a variable to count updates received
    private long m_UpdatesReceived;
    private int m_UpdateCount;

    // APIQA
    private int m_E1Counter = 0;
    private int m_E2Counter = 0;
    private int m_E3Counter = 0;
    private int m_E4Counter = 0;
    private int m_E5Counter = 0;
    private List<EventCounter> m_AllEventCounters = new();

    // END APIQA

    // APIQA:
    private readonly List<string> m_ViewElementNameList = new();
    private readonly List<int> m_ReissueViewFieldList = new();

    public WatchlistConsumer()
    {
        m_DispatchOptions.SetMaxMessages(1000);
        m_WatchlistConsumerConfig = new WatchlistConsumerConfig();
        m_WatchlistConsumerConfig.AddCommandLineArgs();
    }

    /// <summary>
    /// Initializes the Watchlist consumer application.
    /// </summary>
    /// <param name="args">Command line args.</param>
    internal void Init(string[] args)
    {
        if (!m_WatchlistConsumerConfig.Init(args))
        {
            Console.Error.WriteLine("\nError loading command line arguments:\n");
            Console.Error.WriteLine(CommandLine.OptionHelpString());
            Environment.Exit((int)CodecReturnCode.FAILURE);
        }

        m_ItemDecoder.Init();

        // display product version information
        Console.WriteLine(Transport.QueryVersion());
        Console.WriteLine("WatchlistConsumer initializing...");

        m_Runtime = System.DateTime.Now + TimeSpan.FromSeconds(m_WatchlistConsumerConfig.Runtime);
        m_CloseRunTime = System.DateTime.Now + TimeSpan.FromSeconds(m_WatchlistConsumerConfig.Runtime) + m_Closetime;

        // enable Reactor XML tracing if specified
        if (m_WatchlistConsumerConfig.EnableXmlTracing)
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
        if ((errorInfo is not null && errorInfo.Code != ReactorReturnCode.SUCCESS) || m_Reactor is null)
        {
            Console.WriteLine("Reactor.CreateReactor() failed: " + errorInfo?.ToString());
            Environment.Exit((int)ReactorReturnCode.FAILURE);
        }

        m_Sockets.Add(m_Reactor.EventSocket!);

        /* create channel info, initialize channel info, and connect channels
		 * for each connection specified */
        foreach (var connectionArg in m_WatchlistConsumerConfig.ConnectionList)
        {
            // create channel info
            ChannelInfo chnlInfo = new()
            {
                ConnectionArg = connectionArg
            };

            // initialize channel info
            InitChannelInfo(chnlInfo);

            // connect channel
            ReactorReturnCode ret;
            if ((ret = m_Reactor!.Connect(chnlInfo.ConnectOptions, chnlInfo.ConsumerRole, out var eInfo)) < ReactorReturnCode.SUCCESS)
            {
                Console.WriteLine("Reactor.Connect() failed with return code: " + ret + " error = " + eInfo?.Error.Text);
                Environment.Exit((int)ReactorReturnCode.FAILURE);
            }

            // add to ChannelInfo list
            m_ChnlInfoList.Add(chnlInfo);

            // APIQA
            m_AllEventCounters = m_WatchlistConsumerConfig.GetEventCounters();
            // END APIQA
        }
    }

    /// <summary>
    /// Runs the Watchlist consumer application.
    /// </summary>
    internal void Run()
    {
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

            if (System.DateTime.Now >= m_Runtime && !m_CloseHandled)
            {
                Console.WriteLine("Consumer run-time expired, close now...");
                HandleClose();
                m_CloseHandled = true;
            }
            else if (System.DateTime.Now >= m_CloseRunTime)
            {
                Console.WriteLine("Consumer closetime expired, shutdown reactor.");
                break;
            }
            if (!m_CloseHandled)
            {
                HandlePosting();

                // send login reissue if login reissue time has passed
                foreach (var chnlInfo in m_ChnlInfoList)
                {
                    if (chnlInfo.CanSendLoginReissue &&
                        System.DateTime.Now >= chnlInfo.LoginReissueTime)
                    {
                        LoginRequest loginRequest = chnlInfo.ConsumerRole.RdmLoginRequest!;
                        m_SubmitOptions.Clear();
                        var ret = chnlInfo!.ReactorChannel!.Submit(loginRequest, m_SubmitOptions, out var errorInfo);
                        if (ret != ReactorReturnCode.SUCCESS)
                        {
                            Console.WriteLine("Login reissue failed. Error: " + errorInfo?.Error.Text);
                        }
                        else
                        {
                            Console.WriteLine("Login reissue sent");
                        }
                        chnlInfo!.CanSendLoginReissue = false;
                    }
                }
            }

            if (m_CloseHandled)
                break;
        }
    }



    //APIQA edited
    //APIQA: Requesting new item
    CodecReturnCode RequestNewItem(Reactor reactor, ReactorChannel channel, bool existingItem, bool isPrivate, bool isSnap, bool isView, int viewId, bool isMsgKeyInUpdates, string name)
    {
        CodecReturnCode ret = CodecReturnCode.SUCCESS;
        m_ItemRequest.Clear();

        m_ItemRequest.AddItem(name);
        Console.WriteLine("\n\n-----------------APIQA: Requesting new item " + name);

        m_ItemRequest.DomainType = 6;//domaintype 6 is market price

        m_ItemRequest.StreamId = m_WatchlistConsumerConfig.LastInitialStreamId + m_NewItemRequested;

        if (!isSnap)
        {
            m_ItemRequest.HasStreaming = true;
        }

        // Also populate itemInfo
        if (!existingItem)
        {
            ItemInfo itemInfo = new()
            {
                StreamId = m_ItemRequest.StreamId,
                Domain = m_ItemRequest.DomainType,
                Name = name,
                //added specifying more attributes to itemInfo
                IsPrivateStream = isPrivate,
                IsView = isView,
                IsSnapshot = isSnap,
                IsMsgKeyInUpdates = isMsgKeyInUpdates,
                ViewId = viewId
            };
            m_WatchlistConsumerConfig.ItemList.Add(itemInfo);
        }

        // Encodeing VIEW
        m_Payload = new Eta.Codec.Buffer(); // move it to the top to share 
        m_Payload.Data(new ByteBuffer(1024));

        if (m_Payload == null)
        {
            return CodecReturnCode.FAILURE;
        }

        m_EncIter.Clear();
        m_EncIter.SetBufferAndRWFVersion(m_Payload, channel.MajorVersion, channel.MinorVersion);

        //added enabling private stream for new item
        if (isPrivate)
        {
            m_ItemRequest.HasPrivateStream = true;
        }

        if (isView)//added selectively making it a view item or not
        {
            m_ItemRequest.HasView = true;
            Console.WriteLine("------------------APIQA: StreamID: " + m_ItemRequest.StreamId + " Setting VIEW");

            if (viewId == 0)
            {
                Console.WriteLine("------------------APIQA: StreamID: " + m_ItemRequest.StreamId + " Setting VIEW 1 [FIDS: 2, 4, 38]");
                m_ViewFieldList.Add(2);
                m_ViewFieldList.Add(4);
                m_ViewFieldList.Add(38);
                m_ViewElementNameList.Add("RDNDISPLAY");
                m_ViewElementNameList.Add("RDN_EXCHID");
                m_ViewElementNameList.Add("DIVPAYDATE");

            }
            if (viewId == 1)
            {
                Console.WriteLine("------------------APIQA: StreamID: " + m_ItemRequest.StreamId + " Setting VIEW 1 [FIDS: 6, 22]");
                m_ViewFieldList.Add(6);
                m_ViewFieldList.Add(22);
                m_ViewElementNameList.Add("TRDPRC_1");
                m_ViewElementNameList.Add("BID");
            }
            if (viewId == 2)
            {
                Console.WriteLine("------------------APIQA: StreamID: " + m_ItemRequest.StreamId + " Setting VIEW 2 [FIDS 25,32]");
                m_ViewFieldList.Add(25);
                m_ViewFieldList.Add(32);
                m_ViewElementNameList.Add("ASK");
                m_ViewElementNameList.Add("VOL ACCUMULATED");
            }
            if (viewId == 3)
            {
                Console.WriteLine("------------------APIQA: StreamID: " + m_ItemRequest.StreamId + " Setting VIEW 3 [FIDS 25,32,267]");
                m_ViewFieldList.Add(22);
                m_ViewFieldList.Add(32);
                m_ViewFieldList.Add(267);
                m_ViewElementNameList.Add("BID");
                m_ViewElementNameList.Add("VOL ACCUMULATED");
                m_ViewElementNameList.Add("ASK_TIME");
            }
            if (viewId == 4)
            {
                Console.WriteLine("------------------APIQA: StreamID: " + m_ItemRequest.StreamId + " Setting VIEW 4 [FIDS 11,38,267] on streamId");
                m_ViewFieldList.Add(11);
                m_ViewFieldList.Add(38);
                m_ViewFieldList.Add(267);
                m_ViewElementNameList.Add("NETCHNG_1");
                m_ViewElementNameList.Add("DIVPAYDATE");
                m_ViewElementNameList.Add("ASK_TIME");
            }
            //APIQA: added more viewIds
            if (viewId == 5)
            {
                Console.WriteLine("------------------APIQA: StreamID: " + m_ItemRequest.StreamId + " Setting VIEW 5 [FIDS 2,32]");
                m_ViewFieldList.Add(2);
                m_ViewFieldList.Add(32);
                m_ViewElementNameList.Add("RDNDISPLAY");
                m_ViewElementNameList.Add("VOL ACCUMULATED");
            }
            if (viewId == 6)
            {
                Console.WriteLine("------------------APIQA: StreamID: " + m_ItemRequest.StreamId + " Setting VIEW 6 [FIDS 4,7,8]");
                m_ViewFieldList.Add(4);
                m_ViewFieldList.Add(7);
                m_ViewFieldList.Add(8);
                m_ViewElementNameList.Add("RDN_EXCHID");
                m_ViewElementNameList.Add("TRDPRC_2");
                m_ViewElementNameList.Add("TRDPRC_3");
            }
            if (viewId == 7)
            {
                Console.WriteLine("------------------APIQA: StreamID: " + m_ItemRequest.StreamId + " Setting VIEW 7 [FIDS 0-99]");
                m_ViewFieldList.Add(1);
                m_ViewFieldList.Add(2);
                m_ViewFieldList.Add(3);
                m_ViewFieldList.Add(4);
                m_ViewFieldList.Add(5);
                m_ViewFieldList.Add(6);
                m_ViewFieldList.Add(7);
                m_ViewFieldList.Add(8);
                m_ViewFieldList.Add(9);
                m_ViewFieldList.Add(10);
                m_ViewFieldList.Add(11);
                m_ViewFieldList.Add(12);
                m_ViewFieldList.Add(13);
                m_ViewFieldList.Add(14);
                m_ViewFieldList.Add(15);
                m_ViewFieldList.Add(16);
                m_ViewFieldList.Add(17);
                m_ViewFieldList.Add(18);
                m_ViewFieldList.Add(19);
                m_ViewFieldList.Add(20);
                m_ViewFieldList.Add(21);
                m_ViewFieldList.Add(22);
                m_ViewFieldList.Add(23);
                m_ViewFieldList.Add(24);
                m_ViewFieldList.Add(25);
                m_ViewFieldList.Add(26);
                m_ViewFieldList.Add(27);
                m_ViewFieldList.Add(28);
                m_ViewFieldList.Add(29);
                m_ViewFieldList.Add(30);
                m_ViewFieldList.Add(31);
                m_ViewFieldList.Add(32);
                m_ViewFieldList.Add(33);
                m_ViewFieldList.Add(34);
                m_ViewFieldList.Add(35);
                m_ViewFieldList.Add(36);
                m_ViewFieldList.Add(37);
                m_ViewFieldList.Add(38);
                m_ViewFieldList.Add(39);
                m_ViewFieldList.Add(40);
                m_ViewFieldList.Add(41);
                m_ViewFieldList.Add(42);
                m_ViewFieldList.Add(43);
                m_ViewFieldList.Add(44);
                m_ViewFieldList.Add(45);
                m_ViewFieldList.Add(46);
                m_ViewFieldList.Add(47);
                m_ViewFieldList.Add(48);
                m_ViewFieldList.Add(49);
                m_ViewFieldList.Add(50);
                m_ViewFieldList.Add(51);
                m_ViewFieldList.Add(52);
                m_ViewFieldList.Add(53);
                m_ViewFieldList.Add(54);
                m_ViewFieldList.Add(55);
                m_ViewFieldList.Add(56);
                m_ViewFieldList.Add(57);
                m_ViewFieldList.Add(58);
                m_ViewFieldList.Add(59);
                m_ViewFieldList.Add(60);
                m_ViewFieldList.Add(61);
                m_ViewFieldList.Add(62);
                m_ViewFieldList.Add(63);
                m_ViewFieldList.Add(64);
                m_ViewFieldList.Add(65);
                m_ViewFieldList.Add(66);
                m_ViewFieldList.Add(67);
                m_ViewFieldList.Add(68);
                m_ViewFieldList.Add(69);
                m_ViewFieldList.Add(70);
                m_ViewFieldList.Add(71);
                m_ViewFieldList.Add(72);
                m_ViewFieldList.Add(73);
                m_ViewFieldList.Add(74);
                m_ViewFieldList.Add(75);
                m_ViewFieldList.Add(76);
                m_ViewFieldList.Add(77);
                m_ViewFieldList.Add(78);
                m_ViewFieldList.Add(79);
                m_ViewFieldList.Add(80);
                m_ViewFieldList.Add(81);
                m_ViewFieldList.Add(82);
                m_ViewFieldList.Add(83);
                m_ViewFieldList.Add(84);
                m_ViewFieldList.Add(85);
                m_ViewFieldList.Add(86);
                m_ViewFieldList.Add(87);
                m_ViewFieldList.Add(88);
                m_ViewFieldList.Add(89);
                m_ViewFieldList.Add(90);
                m_ViewFieldList.Add(91);
                m_ViewFieldList.Add(92);
                m_ViewFieldList.Add(93);
                m_ViewFieldList.Add(94);
                m_ViewFieldList.Add(95);
                m_ViewFieldList.Add(96);
                m_ViewFieldList.Add(97);
                m_ViewFieldList.Add(98);
                m_ViewFieldList.Add(99);
                m_ViewElementNameList.Add("BID");
                m_ViewElementNameList.Add("ASK");
                m_ViewElementNameList.Add("TRD");
                m_ViewElementNameList.Add("VOLUME");
            }
            if (!m_WatchlistConsumerConfig.IsElementView)
            {
                m_ItemRequest.ViewFields = m_ViewFieldList;
            }
            else
            {
                m_ItemRequest.ViewElementNameList = m_ViewElementNameList;
            }
            ret = m_ItemRequest.Encode(m_EncIter);
            if (ret < CodecReturnCode.SUCCESS)
            {
                Console.WriteLine("RequestItem.Encode() failed");
                return ret;
            }
            m_ItemRequest.RequestMsg.EncodedDataBody = m_Payload;
        }
        else
        {
            m_ItemRequest.Encode();
        }
        m_SubmitOptions.Clear();
        if (m_WatchlistConsumerConfig.ServiceName != null)
        {
            m_SubmitOptions.ServiceName = m_WatchlistConsumerConfig.ServiceName;
        }
        //APIQA
        if (isMsgKeyInUpdates)
        {
            m_ItemRequest.RequestMsg.ApplyMsgKeyInUpdates();
            Console.WriteLine("------------APIQA: Now MsgKeyInUpdates is applied");
        }
        var submitReturnCode = channel.Submit(m_ItemRequest.RequestMsg, m_SubmitOptions, out var errorInfo);
        if (submitReturnCode < ReactorReturnCode.SUCCESS)
        {
            Console.WriteLine("\nReactorChannel.Submit() failed: " + ret + "(" + errorInfo!.Error.Text + ")\n");
            Environment.Exit((int)ReactorReturnCode.FAILURE);
        }
        m_NewItemRequested++;
        Console.WriteLine("APIQA: New item requested: PrivateStream: " + isPrivate + ", Snapshot: " + isSnap);
        return CodecReturnCode.SUCCESS;
    }

    // APIQA: Reissue item
    private CodecReturnCode ReissueItem(Reactor reactor, ReactorChannel channel, int reissueType, int reissueViewId, int itemIndex)
    {
        CodecReturnCode ret = CodecReturnCode.SUCCESS;

        string? nameOfItem = m_WatchlistConsumerConfig.GetNameOfItem(itemIndex);
        if (itemIndex >= m_WatchlistConsumerConfig.ItemList.Count)
        {
            Console.WriteLine("\n\n-----------------APIQA: <Item Not found in itemlist> Invalid Reissue item " + nameOfItem + ", Reissue Type: " + reissueType + ", reissueViewId: " + reissueViewId);
            return CodecReturnCode.FAILURE;

        }
        ItemInfo itInfo = m_WatchlistConsumerConfig.ItemList[itemIndex];
        if (itInfo == null || !itInfo.Name!.Equals(nameOfItem, StringComparison.InvariantCultureIgnoreCase))
        {
            Console.WriteLine("\n\n-----------------APIQA: <Item Not found in itemlist> Invalid Reissue item " + nameOfItem + ", Reissue Type: " + reissueType + ", reissueViewId: " + reissueViewId);
            return CodecReturnCode.FAILURE;
        }

        m_ItemRequest.Clear();

        m_ItemRequest.AddItem(nameOfItem);
        m_ItemRequest.StreamId = itInfo.StreamId;
        m_ItemRequest.DomainType = itInfo.Domain;

        if (reissueType == 1)
        {
            if (!m_WatchlistConsumerConfig.IsSnapshot)
                m_ItemRequest.HasStreaming = true;
            Console.WriteLine("\n\n-----------------APIQA: Reissue item " + nameOfItem + " streamid " + m_ItemRequest.StreamId + ", View Reissue Type: " + reissueType + ", reissueViewId: " + reissueViewId);
            // Encoding VIEW
            m_Payload = new (); // move it to the top to share 
            m_Payload.Data(new ByteBuffer(1024));

            if (m_Payload == null)
            {
                return CodecReturnCode.FAILURE;
            }
            m_EncIter.Clear();
            m_EncIter.SetBufferAndRWFVersion(m_Payload, channel.MajorVersion, channel.MinorVersion);
            m_ItemRequest.HasView = true;
            m_ReissueViewFieldList.Clear();
            if (reissueViewId == 0)
            {
                Console.WriteLine("------------------APIQA: reissue StreamID: " + m_ItemRequest.StreamId + " Setting VIEW 0 [FIDS: 2, 4, 38]");
                m_ReissueViewFieldList.Add(2);
                m_ReissueViewFieldList.Add(4);
                m_ReissueViewFieldList.Add(38);
            }
            else if (reissueViewId == 1)
            {
                Console.WriteLine("------------------APIQA: reissue StreamID: " + m_ItemRequest.StreamId + " Setting VIEW 1 [FIDS 6, 22]");
                m_ReissueViewFieldList.Add(6);
                m_ReissueViewFieldList.Add(22);
            }
            else if (reissueViewId == 2)
            {
                Console.WriteLine("------------------APIQA: reissue StreamID: " + m_ItemRequest.StreamId + " Setting VIEW 2 [FIDS 25,32]");
                m_ReissueViewFieldList.Add(25);
                m_ReissueViewFieldList.Add(32);
            }
            else if (reissueViewId == 3)
            {
                Console.WriteLine("------------------APIQA: reissue StreamID: " + m_ItemRequest.StreamId + " Setting VIEW 3 [FIDS 25,32,267]");
                m_ReissueViewFieldList.Add(22);
                m_ReissueViewFieldList.Add(32);
                m_ReissueViewFieldList.Add(267);
            }
            else if (reissueViewId == 4)
            {
                Console.WriteLine("------------------APIQA: reissue StreamID: " + m_ItemRequest.StreamId + " Setting VIEW 4 [FIDS 11,38,267");
                m_ReissueViewFieldList.Add(11);
                m_ReissueViewFieldList.Add(38);
                m_ReissueViewFieldList.Add(267);
            }
            else if (reissueViewId == 5)
            {
                Console.WriteLine("------------------APIQA: reissue StreamID: " + m_ItemRequest.StreamId + " Setting VIEW 5 [FIDS 2,32]");
                m_ReissueViewFieldList.Add(2);
                m_ReissueViewFieldList.Add(32);
            }
            else if (reissueViewId == 6)
            {
                Console.WriteLine("------------------APIQA: reissue StreamID: " + m_ItemRequest.StreamId + " Setting VIEW 6 [FIDS 4,7,8");
                m_ReissueViewFieldList.Add(4);
                m_ReissueViewFieldList.Add(7);
                m_ReissueViewFieldList.Add(8);
            }
            else if (reissueViewId == 7)
            {
                Console.WriteLine("------------------APIQA: reissue StreamID: " + m_ItemRequest.StreamId + " Setting VIEW 7 [FIDS 0-99");
                int vIdx = 0;
                for (vIdx = 1; vIdx < 100; vIdx++)
                    m_ReissueViewFieldList.Add(vIdx);
            }
            m_ItemRequest.ViewFields = m_ReissueViewFieldList;
            ret = m_ItemRequest.Encode(m_EncIter);
            if (ret < CodecReturnCode.SUCCESS)
            {
                Console.WriteLine("\n\n-----------------APIQA: Failed to encode View Reissue for item " + nameOfItem + ", Reissue Type: " + reissueType + ", reissueViewId: " + reissueViewId);
                return ret;
            }
            m_ItemRequest.RequestMsg.EncodedDataBody = m_Payload;
        }
        else if (reissueType == 2)
        { // Pause
            Console.WriteLine("-----------------APIQA: reissue for item " + nameOfItem + " streamid " + m_ItemRequest.StreamId + ", Reissue Type: " + reissueType + ", Setting PAUSE flags ");
            if (!m_WatchlistConsumerConfig.EnableSnapshot)
                m_ItemRequest.HasStreaming = true;
            m_ItemRequest.HasPause = true;
            //          m_ItemRequest.ApplyNoRefresh();			
            m_ItemRequest.Encode();
        }
        else if (reissueType == 3)
        { // Resume
            if (!m_WatchlistConsumerConfig.EnableSnapshot)
                m_ItemRequest.HasStreaming = true;
            Console.WriteLine("-----------------APIQA: reissue for item " + nameOfItem + " streamid " + m_ItemRequest.StreamId + ", Reissue Type: " + reissueType + ", Setting Resume/Streamin ");
            Console.WriteLine("-----------------APIQA: Setting RESUME/Streaming flags on item reissue for user streamid " + m_ItemRequest.StreamId);
            m_ItemRequest.HasStreaming = true;
            m_ItemRequest.HasPause = false;
            m_ItemRequest.Encode();
        }
        else
        {
            Console.WriteLine("\n\n-----------------APIQA: Item " + nameOfItem + "<UNSUPPORTED/INVALID> Reissue Type: " + reissueType + ", reissueViewId: " + reissueViewId);
            return CodecReturnCode.FAILURE;
        }


        Console.WriteLine(m_ItemRequest.ToString());

        m_SubmitOptions.Clear();
        if (m_WatchlistConsumerConfig.ServiceName != null)
        {
            m_SubmitOptions.ServiceName = m_WatchlistConsumerConfig.ServiceName;
        }
        var r = channel.Submit(m_ItemRequest.RequestMsg, m_SubmitOptions, out var errorInfo);
        if (r < ReactorReturnCode.SUCCESS)
        {
            Console.WriteLine("\nReactorChannel.Submit() failed: " + ret + "(" + errorInfo!.Error.Text + ")\n");
        }

        return CodecReturnCode.SUCCESS;
    }
    //END APIQA: Requesting new item

    /// <summary>
    /// Requests the desired items.
    /// </summary>
    private CodecReturnCode RequestItems(ReactorChannel channel)
    {
        if (m_ItemsRequested)
            return CodecReturnCode.SUCCESS;

        m_ItemsRequested = true;

        // APIQA
        Console.WriteLine("\n\n Requesting first " + m_WatchlistConsumerConfig.ItemList.Count + " items.");
        // END APIQA

        for (int itemListIndex = 0; itemListIndex < m_WatchlistConsumerConfig.ItemList.Count; ++itemListIndex)
        {
            CodecReturnCode ret = CodecReturnCode.SUCCESS;

            m_ItemRequest.Clear();

            // APIQA
            /*
            if (!m_WatchlistConsumerConfig.EnableSnapshot)
                m_ItemRequest.HasStreaming = true;
            */
            // END APIQA

            m_ItemRequest.AddItem(m_WatchlistConsumerConfig.ItemList[itemListIndex].Name!);

            var domainType = m_WatchlistConsumerConfig.ItemList[itemListIndex].Domain;

            m_ItemRequest.DomainType = domainType;
            m_ItemRequest.StreamId = m_WatchlistConsumerConfig.ItemList[itemListIndex].StreamId;

            // APIQA:  Added a check on item itself - altered if
            if ((!m_WatchlistConsumerConfig.EnableSnapshot) && (!m_WatchlistConsumerConfig.ItemList[itemListIndex].IsSnapshot))
            {
                Console.WriteLine("------------------APIQA: StreamID: " + m_ItemRequest.StreamId + " Setting STREAMING flag");
                m_ItemRequest.HasStreaming = true;
            }

            if (m_WatchlistConsumerConfig.ItemList[itemListIndex].IsPrivateStream)
            {
                Console.WriteLine("------------------APIQA: StreamID: " + m_ItemRequest.StreamId + " Setting PRIVATE flag");
                m_ItemRequest.HasPrivateStream = true;
            }
            if (m_WatchlistConsumerConfig.ItemList[itemListIndex].IsMsgKeyInUpdates)
            {
                Console.WriteLine("------------------APIQA: StreamID: " + m_ItemRequest.StreamId + " Setting MSG_KEY_IN_UPDATES flag");
                m_ItemRequest.RequestMsg.ApplyMsgKeyInUpdates();
            }
            // END APIQA

            if (m_WatchlistConsumerConfig.ItemList[itemListIndex].IsPrivateStream)
                m_ItemRequest.HasPrivateStream = true;

            //APIQA
            if (m_WatchlistConsumerConfig.HasReqQos)
            {
                m_ItemRequest.HasQos = true;
                m_ItemRequest.HasWorstQos = true;
                if (m_WatchlistConsumerConfig.QosDynamic)
                    m_ItemRequest.Qos.IsDynamic = m_WatchlistConsumerConfig.QosDynamic;
                else
                    m_ItemRequest.Qos.IsDynamic = false;
                m_ItemRequest.Qos.Rate(m_WatchlistConsumerConfig.QosRate);
                m_ItemRequest.Qos.Timeliness(m_WatchlistConsumerConfig.QosTimeliness);
                m_ItemRequest.Qos.TimeInfo(m_WatchlistConsumerConfig.QosTimeInfo);
                m_ItemRequest.Qos.RateInfo(m_WatchlistConsumerConfig.QosRateInfo);
                m_ItemRequest.WorstQos.Rate(m_WatchlistConsumerConfig.WorstQosRate);
                m_ItemRequest.WorstQos.Timeliness(m_WatchlistConsumerConfig.WorstQosTimeliness);
                m_ItemRequest.WorstQos.TimeInfo(m_WatchlistConsumerConfig.WorstQosTimeInfo);
                m_ItemRequest.WorstQos.RateInfo(m_WatchlistConsumerConfig.WorstQosRateInfo);
            }
            //END APIQA
            // API QA QOS 0253
            if (itemListIndex == 0)
            {
                m_ItemRequest.HasQos = true;
                m_ItemRequest.Qos.IsDynamic = false;
                m_ItemRequest.Qos.Rate(3);
                m_ItemRequest.Qos.RateInfo(11);
                m_ItemRequest.Qos.Timeliness(3);
                m_ItemRequest.Qos.TimeInfo(11);             
            } else if (itemListIndex == 1)
            {
                m_ItemRequest.HasQos = true;
                m_ItemRequest.HasWorstQos = true;
                m_ItemRequest.Qos.IsDynamic = true;
                m_ItemRequest.Qos.Rate(3);
                m_ItemRequest.Qos.Timeliness(3);
                m_ItemRequest.Qos.RateInfo(9);
                m_ItemRequest.Qos.TimeInfo(9);
                m_ItemRequest.WorstQos.Rate(3);
                m_ItemRequest.WorstQos.Timeliness(3);
                m_ItemRequest.WorstQos.RateInfo(6000);
                m_ItemRequest.WorstQos.TimeInfo(6000);
            }
            //End API QA QOS 0253
            if (domainType == (int)DomainType.SYMBOL_LIST && m_WatchlistConsumerConfig.ItemList[itemListIndex].SymbolListData)
            {
                m_ItemRequest.IsSymbolListData = true;
                m_Payload = new();
                m_Payload.Data(new ByteBuffer(1024));

                m_EncIter.Clear();
                m_EncIter.SetBufferAndRWFVersion(m_Payload, channel.MajorVersion, channel.MinorVersion);

                ret = m_ItemRequest.Encode(m_EncIter);

                if (ret < CodecReturnCode.SUCCESS)
                {
                    Console.WriteLine("RequestItem.Encode() failed");
                    return ret;
                }

                m_ItemRequest.RequestMsg.EncodedDataBody = m_Payload;
            }
            // APIQA
            else if (domainType == (int)DomainType.MARKET_PRICE && (m_WatchlistConsumerConfig.EnableView || m_WatchlistConsumerConfig.ItemList[itemListIndex].IsView))
            // END APIQA
            {
                m_Payload = new();// move it to the top to share
                m_Payload.Data(new ByteBuffer(1024));

                m_EncIter.Clear();
                m_EncIter.SetBufferAndRWFVersion(m_Payload, channel.MajorVersion, channel.MinorVersion);

                m_ItemRequest.HasView = true;
                // APIQA:  Added contents of view 
                if (m_WatchlistConsumerConfig.ItemList[itemListIndex].ViewId == 0)
                {
                    Console.WriteLine("------------------APIQA: StreamID: " + m_ItemRequest.StreamId + " Setting VIEW 0 [FIDS 2,4,38]");
                    m_ViewFieldList.Add(2);
                    m_ViewFieldList.Add(4);
                    m_ViewFieldList.Add(38);
                    m_ViewElementNameList.Add("RDNDISPLAY");
                    m_ViewElementNameList.Add("RDN_EXCHID");
                    m_ViewElementNameList.Add("DIVPAYDATE");
                }
                if (m_WatchlistConsumerConfig.ItemList[itemListIndex].ViewId == 1)
                {
                    Console.WriteLine("------------------APIQA: StreamID: " + m_ItemRequest.StreamId + " Setting VIEW 1 [FIDS 6,22]");
                    m_ViewFieldList.Add(6);
                    m_ViewFieldList.Add(22);
                    Console.WriteLine("------------------APIQA: StreamID: " + m_ItemRequest.StreamId + " Setting VIEW 1 [TRDPRC_1, BID]");
                    m_ViewElementNameList.Add("TRDPRC_1");
                    m_ViewElementNameList.Add("BID");
                }
                if (m_WatchlistConsumerConfig.ItemList[itemListIndex].ViewId == 2)
                {
                    Console.WriteLine("------------------APIQA: StreamID: " + m_ItemRequest.StreamId + " Setting VIEW 2 [FIDS 25,32]");
                    m_ViewFieldList.Add(25);
                    m_ViewFieldList.Add(32);
                    Console.WriteLine("------------------APIQA: StreamID: " + m_ItemRequest.StreamId + " Setting VIEW 2 [ASK, VOL ACCUMULATED]");
                    m_ViewElementNameList.Add("ASK");
                    m_ViewElementNameList.Add("VOL ACCUMULATED");
                }
                if (m_WatchlistConsumerConfig.ItemList[itemListIndex].ViewId == 3)
                {
                    Console.WriteLine("------------------APIQA: StreamID: " + m_ItemRequest.StreamId + " Setting VIEW 3 [FIDS 22,32,267]");
                    m_ViewFieldList.Add(22);
                    m_ViewFieldList.Add(32);
                    m_ViewFieldList.Add(267);
                    Console.WriteLine("------------------APIQA: StreamID: " + m_ItemRequest.StreamId + " Setting VIEW 3 [BID, VOL ACCUMULATED, ASK_TIME]");
                    m_ViewElementNameList.Add("BID");
                    m_ViewElementNameList.Add("VOL ACCUMULATED");
                    m_ViewElementNameList.Add("ASK_TIME");
                }
                if (m_WatchlistConsumerConfig.ItemList[itemListIndex].ViewId == 4)
                {
                    Console.WriteLine("------------------APIQA: StreamID: " + m_ItemRequest.StreamId + " Setting VIEW 4 [FIDS 11,38,267] on streamId");
                    m_ViewFieldList.Add(11);
                    m_ViewFieldList.Add(38);
                    m_ViewFieldList.Add(267);
                    m_ViewElementNameList.Add("NETCHNG_1");
                    m_ViewElementNameList.Add("DIVPAYDATE");
                    m_ViewElementNameList.Add("ASK_TIME");
                    Console.WriteLine("------------------APIQA: StreamID: " + m_ItemRequest.StreamId + " Setting VIEW 4 [NETCHNG_1, DIVPAYDATE, ASK_TIME]");
                }
                if (m_WatchlistConsumerConfig.ItemList[itemListIndex].ViewId == 5)
                {
                    Console.WriteLine("------------------APIQA: StreamID: " + m_ItemRequest.StreamId + " Setting VIEW 5 [FIDS 2,32]");
                    m_ViewFieldList.Add(2);
                    m_ViewFieldList.Add(32);
                    m_ViewElementNameList.Add("RDNDISPLAY");
                    m_ViewElementNameList.Add("VOL ACCUMULATED");
                }
                if (m_WatchlistConsumerConfig.ItemList[itemListIndex].ViewId == 6)
                {
                    Console.WriteLine("------------------APIQA: StreamID: " + m_ItemRequest.StreamId + " Setting VIEW 6 [FIDS 4,7,8]");
                    m_ViewFieldList.Add(4);
                    m_ViewFieldList.Add(7);
                    m_ViewFieldList.Add(8);
                    m_ViewElementNameList.Add("RDN_EXCHID");
                    m_ViewElementNameList.Add("TRDPRC_2");
                    m_ViewElementNameList.Add("TRDPRC_3");
                }
                if (m_WatchlistConsumerConfig.ItemList[itemListIndex].ViewId == 7)
                {
                    Console.WriteLine("------------------APIQA: StreamID: " + m_ItemRequest.StreamId + " Setting VIEW 0 [FIDS 0-99]");
                    m_ViewFieldList.Add(1);
                    m_ViewFieldList.Add(2);
                    m_ViewFieldList.Add(3);
                    m_ViewFieldList.Add(4);
                    m_ViewFieldList.Add(5);
                    m_ViewFieldList.Add(6);
                    m_ViewFieldList.Add(7);
                    m_ViewFieldList.Add(8);
                    m_ViewFieldList.Add(9);
                    m_ViewFieldList.Add(10);
                    m_ViewFieldList.Add(11);
                    m_ViewFieldList.Add(12);
                    m_ViewFieldList.Add(13);
                    m_ViewFieldList.Add(14);
                    m_ViewFieldList.Add(15);
                    m_ViewFieldList.Add(16);
                    m_ViewFieldList.Add(17);
                    m_ViewFieldList.Add(18);
                    m_ViewFieldList.Add(19);
                    m_ViewFieldList.Add(20);
                    m_ViewFieldList.Add(21);
                    m_ViewFieldList.Add(22);
                    m_ViewFieldList.Add(23);
                    m_ViewFieldList.Add(24);
                    m_ViewFieldList.Add(25);
                    m_ViewFieldList.Add(26);
                    m_ViewFieldList.Add(27);
                    m_ViewFieldList.Add(28);
                    m_ViewFieldList.Add(29);
                    m_ViewFieldList.Add(30);
                    m_ViewFieldList.Add(31);
                    m_ViewFieldList.Add(32);
                    m_ViewFieldList.Add(33);
                    m_ViewFieldList.Add(34);
                    m_ViewFieldList.Add(35);
                    m_ViewFieldList.Add(36);
                    m_ViewFieldList.Add(37);
                    m_ViewFieldList.Add(38);
                    m_ViewFieldList.Add(39);
                    m_ViewFieldList.Add(40);
                    m_ViewFieldList.Add(41);
                    m_ViewFieldList.Add(42);
                    m_ViewFieldList.Add(43);
                    m_ViewFieldList.Add(44);
                    m_ViewFieldList.Add(45);
                    m_ViewFieldList.Add(46);
                    m_ViewFieldList.Add(47);
                    m_ViewFieldList.Add(48);
                    m_ViewFieldList.Add(49);
                    m_ViewFieldList.Add(50);
                    m_ViewFieldList.Add(51);
                    m_ViewFieldList.Add(52);
                    m_ViewFieldList.Add(53);
                    m_ViewFieldList.Add(54);
                    m_ViewFieldList.Add(55);
                    m_ViewFieldList.Add(56);
                    m_ViewFieldList.Add(57);
                    m_ViewFieldList.Add(58);
                    m_ViewFieldList.Add(59);
                    m_ViewFieldList.Add(60);
                    m_ViewFieldList.Add(61);
                    m_ViewFieldList.Add(62);
                    m_ViewFieldList.Add(63);
                    m_ViewFieldList.Add(64);
                    m_ViewFieldList.Add(65);
                    m_ViewFieldList.Add(66);
                    m_ViewFieldList.Add(67);
                    m_ViewFieldList.Add(68);
                    m_ViewFieldList.Add(69);
                    m_ViewFieldList.Add(70);
                    m_ViewFieldList.Add(71);
                    m_ViewFieldList.Add(72);
                    m_ViewFieldList.Add(73);
                    m_ViewFieldList.Add(74);
                    m_ViewFieldList.Add(75);
                    m_ViewFieldList.Add(76);
                    m_ViewFieldList.Add(77);
                    m_ViewFieldList.Add(78);
                    m_ViewFieldList.Add(79);
                    m_ViewFieldList.Add(80);
                    m_ViewFieldList.Add(81);
                    m_ViewFieldList.Add(82);
                    m_ViewFieldList.Add(83);
                    m_ViewFieldList.Add(84);
                    m_ViewFieldList.Add(85);
                    m_ViewFieldList.Add(86);
                    m_ViewFieldList.Add(87);
                    m_ViewFieldList.Add(88);
                    m_ViewFieldList.Add(89);
                    m_ViewFieldList.Add(90);
                    m_ViewFieldList.Add(91);
                    m_ViewFieldList.Add(92);
                    m_ViewFieldList.Add(93);
                    m_ViewFieldList.Add(94);
                    m_ViewFieldList.Add(95);
                    m_ViewFieldList.Add(96);
                    m_ViewFieldList.Add(97);
                    m_ViewFieldList.Add(98);
                    m_ViewFieldList.Add(99);
                    m_ViewElementNameList.Add("BID");
                    m_ViewElementNameList.Add("ASK");
                    m_ViewElementNameList.Add("TRD");
                    m_ViewElementNameList.Add("VOLUME");
                }
                // END APIQA:  Added contents of view

                if (!m_WatchlistConsumerConfig.IsElementView)
                {
                    m_ItemRequest.ViewFields = m_ViewFieldList;
                }
                else
                {
                    m_ItemRequest.ViewElementNameList = m_ViewElementNameList;
                }
				ret = m_ItemRequest.Encode(m_EncIter);
                if (ret < CodecReturnCode.SUCCESS)
                {
                    Console.WriteLine("RequestItem.Encode() failed");
                    return ret;
                }
                m_ItemRequest.RequestMsg.EncodedDataBody = m_Payload;
            }
            else
            {
                m_ItemRequest.Encode();
            }

            m_SubmitOptions.Clear();
            if (m_WatchlistConsumerConfig.ServiceName != null)
            {
                m_SubmitOptions.ServiceName = m_WatchlistConsumerConfig.ServiceName;
            }

            var subRet = channel.Submit(m_ItemRequest.RequestMsg, m_SubmitOptions, out var errorInfo);
            if (subRet < ReactorReturnCode.SUCCESS)
            {
                Console.WriteLine("\nReactorChannel.Submit() failed: " + ret + "(" + errorInfo?.Error.Text + ")\n");
                Environment.Exit((int)ReactorReturnCode.FAILURE);
            }
        }
        return CodecReturnCode.SUCCESS;
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
            ServiceName = m_WatchlistConsumerConfig.ServiceName
        };

        if(channel.Submit(msg, submitOptions, out var errorInfo) != ReactorReturnCode.SUCCESS)
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
                    
                    // APIQA: make possible for all intiial request to be done after
                    // src dir refresh
                    if (m_FieldDictionaryLoaded && m_EnumDictionaryLoaded || m_ItemDecoder.FieldDictionaryLoadedFromFile && m_ItemDecoder.EnumTypeDictionaryLoadedFromFile)
                    {
                        if (!m_WatchlistConsumerConfig.DelayInitialRequest)
                            RequestItems(reactorEvent.ReactorChannel);
                        if (m_WatchlistConsumerConfig.ReqItemBeforeLogin)
                            break;
                    }
                    // END APIQA
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

                    // APIQA
                    m_E3Counter++;
                    if (m_AllEventCounters.Count > 0)
                    {
                        for (int i = 0; i < m_AllEventCounters.Count; ++i)
                        {
                            if (m_AllEventCounters[i].Delay == m_E3Counter && m_AllEventCounters[i].EventType == 3)
                            {
                                int startIdx = m_AllEventCounters[i].StartIdx;
                                int endIdx = m_AllEventCounters[i].EndIdx;
                                for (int index = startIdx; index < endIdx; ++index)
                                {
                                    bool isSnap = m_WatchlistConsumerConfig.GetMPItemInfo(index, "isSnapshot");
                                    bool isView = m_WatchlistConsumerConfig.GetMPItemInfo(index, "isView");
                                    bool isPrivate = m_WatchlistConsumerConfig.GetMPItemInfo(index, "isPrivate");
                                    bool isMsgKeyInUpdates = m_WatchlistConsumerConfig.GetMPItemInfo(index, "isMsgKeyInUpdates");
                                    int vID = m_WatchlistConsumerConfig.GetViewId(index);
                                    string nameOfItem = m_WatchlistConsumerConfig.GetNameOfItem(index)!;
                                    RequestNewItem(m_Reactor!, reactorEvent.ReactorChannel!, false, isPrivate, isSnap, isView, vID, isMsgKeyInUpdates, nameOfItem);
                                }
                                m_AllEventCounters.RemoveAt(i);
                                break;
                            }
                        }
                    }
                    // END APIQA
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

        item = m_WatchlistConsumerConfig.GetItemInfo(msg.StreamId);

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
                // APIQA
                if (refreshMsg.CheckHasMsgKey())
                {
                    Console.WriteLine("------------APIQA: DefaultMsgCallback RefreshMsg --  ItemName from MsgKey: " + refreshMsg.MsgKey.Name.ToString() + "; ServiceId from MsgKey: " + refreshMsg.MsgKey.ServiceId + "; nameType from MsgKey: " + refreshMsg.MsgKey.NameType + "\n\n");
                }

                m_E5Counter++;
                if (m_AllEventCounters.Count > 0)
                {
                    for (int i = 0; i < m_AllEventCounters.Count; ++i)
                    {
                        if (m_AllEventCounters[i].Delay == m_E5Counter && m_AllEventCounters[i].EventType == 5)
                        {
                            int startIdx = m_AllEventCounters[i].StartIdx;
                            int endIdx = m_AllEventCounters[i].EndIdx;
                            for (int index = startIdx; index < endIdx; ++index)
                            {
                                bool isSnap = m_WatchlistConsumerConfig.GetMPItemInfo(index, "isSnapshot");
                                bool isView = m_WatchlistConsumerConfig.GetMPItemInfo(index, "isView");
                                bool isPrivate;
                                isPrivate = m_WatchlistConsumerConfig.GetMPItemInfo(index, "isPrivate");
                                bool isMsgKeyInUpdates = m_WatchlistConsumerConfig.GetMPItemInfo(index, "isMsgKeyInUpdates");
                                int vID = m_WatchlistConsumerConfig.GetViewId(index);
                                string nameOfItem = m_WatchlistConsumerConfig.GetNameOfItem(index)!;
                                RequestNewItem(m_Reactor!, reactorEvent.ReactorChannel!, false, isPrivate, isSnap, isView, vID, isMsgKeyInUpdates, nameOfItem);
                            }
                            m_AllEventCounters.RemoveAt(i);
                            break;
                        }
                    }
                }

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
                // APIQA
                if (updateMsg.CheckHasMsgKey())
                {
                    Console.WriteLine("------------APIQA: DefaultMsgCallback UpdateMsg --  ItemName from MsgKey: " + updateMsg.MsgKey.Name.ToString() + "; ServiceId from MsgKey: " + updateMsg.MsgKey.ServiceId + "; nameType from MsgKey: " + updateMsg.MsgKey.NameType + "\n\n");
                }
                m_ItemDecoder.DecodeDataBody(reactorEvent.ReactorChannel!, updateMsg);
                //API QA
                if (m_WatchlistConsumerConfig.LoginPauseAndResume)
                {
                    m_UpdatesReceived++;
                    // APIQA: Manufacture a login reissue:
                    if (m_UpdatesReceived == 3)
                    {
                        LoginRequest loginRequest = chnlInfo!.ConsumerRole.RdmLoginRequest!;
                        loginRequest.Pause = true;
                        loginRequest.NoRefresh = true;
                        m_SubmitOptions.Clear();
                        if (chnlInfo.ReactorChannel!.Submit(loginRequest, m_SubmitOptions, out var errorInfo) != ReactorReturnCode.SUCCESS)
                        {
                            Console.WriteLine("------------APIQA: attempted login reissue failed. Error: " + errorInfo!.Error.Text);
                        }
                        else
                        {
                            Console.WriteLine("------------APIQA: PAUSE-ALL login reissue done.");
                        }
                    }
                }
                if (m_WatchlistConsumerConfig.ReissueDirEvery5Updates)
                {
                    m_UpdatesReceived++;
                    if (((m_UpdatesReceived % 5) == 0) && (m_UpdatesReceived <= 60))
                    {
                        DirectoryRequest directoryRequest = chnlInfo!.ConsumerRole.RdmDirectoryRequest!;
                        chnlInfo!.ConsumerRole.RdmDirectoryRequest!.Filter = Directory.ServiceFilterFlags.INFO |
                                                Directory.ServiceFilterFlags.STATE | Directory.ServiceFilterFlags.GROUP | Directory.ServiceFilterFlags.LINK;
                        m_SubmitOptions.Clear();
                        if (chnlInfo.ReactorChannel!.Submit(directoryRequest, m_SubmitOptions, out var errorInfo) != ReactorReturnCode.SUCCESS)
                        {
                            Console.WriteLine("------------APIQA: attempted SRC DIR reissue failed. Error: " + errorInfo!.Error.Text);
                        }
                        else
                        {
                            Console.WriteLine("------------APIQA: SRC DIR reissue done.");
                        }
                    }
                }
                if (m_WatchlistConsumerConfig.ReissueDirWithSID != 0)
                {
                    m_UpdatesReceived++;
                    if (m_UpdatesReceived == 5)
                    {
                        DirectoryRequest directoryRequest = new();
                        m_SubmitOptions.Clear();
                        directoryRequest.StreamId = 2;
                        directoryRequest.Filter = Directory.ServiceFilterFlags.INFO |
                                    Directory.ServiceFilterFlags.STATE | Directory.ServiceFilterFlags.GROUP;
                        directoryRequest.HasServiceId = true;
                        directoryRequest.ServiceId = m_WatchlistConsumerConfig.ReissueDirWithSID;
                        Console.WriteLine("Try to reissue Directory with serviceId : " + m_WatchlistConsumerConfig.ReissueDirWithSID);
                        if (chnlInfo!.ReactorChannel!.Submit(directoryRequest, m_SubmitOptions, out var errorInfo) != ReactorReturnCode.SUCCESS)
                        {
                            Console.WriteLine("------------APIQA: attempted SRC DIR reissue failed. Error: " + errorInfo!.Error.Text);
                        }
                        else
                        {
                            Console.WriteLine("------------APIQA: SRC DIR reissue done.");
                        }
                    }
                }

                //END APIQA
                m_E2Counter++;
                m_E4Counter++; // For reissue
                if (m_AllEventCounters.Count > 0)
                {
                    for (int i = 0; i < m_AllEventCounters.Count; ++i)
                    {
                        if (m_AllEventCounters[i].Delay == m_E2Counter && m_AllEventCounters[i].EventType == 2)
                        {
                            int startIdx = m_AllEventCounters[i].StartIdx;
                            int endIdx = m_AllEventCounters[i].EndIdx;
                            for (int index = startIdx; index < endIdx; ++index)
                            {
                                bool isSnap = m_WatchlistConsumerConfig.GetMPItemInfo(index, "isSnapshot");
                                bool isView = m_WatchlistConsumerConfig.GetMPItemInfo(index, "isView");
                                bool isPrivate;
                                isPrivate = m_WatchlistConsumerConfig.GetMPItemInfo(index, "isPrivate");
                                bool isMsgKeyInUpdates = m_WatchlistConsumerConfig.GetMPItemInfo(index, "isMsgKeyInUpdates");
                                int vID = m_WatchlistConsumerConfig.GetViewId(index);
                                string nameOfItem = m_WatchlistConsumerConfig.GetNameOfItem(index)!;
                                RequestNewItem(m_Reactor!, reactorEvent.ReactorChannel!, false, isPrivate, isSnap, isView, vID, isMsgKeyInUpdates, nameOfItem);
                            }
                            m_AllEventCounters.RemoveAt(i);
                            break;
                        }

                        // Check for reissue
                        if (m_AllEventCounters[i].Delay == m_E4Counter && m_AllEventCounters[i].EventType == 4)
                        {
                            int startIdx = m_AllEventCounters[i].StartIdx;
                            int endIdx = m_AllEventCounters[i].EndIdx;
                            int reissueType = m_AllEventCounters[i].ReissueType;
                            int reissueVid = m_AllEventCounters[i].ReissueViewId;
                            for (int index = startIdx; index < endIdx; ++index)
                            {
                                ReissueItem(m_Reactor!, reactorEvent.ReactorChannel!, reissueType, reissueVid, index);
                            }
                            m_AllEventCounters.RemoveAt(i);
                            break;
                        }
                    }
                }
                // END APIQA
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
                    chnlInfo.CanSendLoginReissue = m_WatchlistConsumerConfig.EnableSessionManagement ? false : true;
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
                // APIQA:  Send a login close
                if (m_WatchlistConsumerConfig.LoginCloseAfterLoginStatus)
                {
                    m_SubmitOptions.Clear();
                    m_CloseMsg.Clear();
                    m_CloseMsg.StreamId = loginStatus.StreamId;
                    m_CloseMsg.MsgClass = MsgClasses.CLOSE;
                    m_CloseMsg.DomainType = (int)DomainType.LOGIN;
                    if (chnlInfo.ReactorChannel!.Submit((Msg)m_CloseMsg, m_SubmitOptions, out _) != ReactorReturnCode.SUCCESS)
                        Console.WriteLine("Close login Stream of " + loginStatus.StreamId + " Failed.");
                    else
                        Console.WriteLine("----------------APIQA: Closed login stream of " + loginStatus.StreamId + " Failed.");
                }
                // END APIQA
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
                // APIQA
                Console.Write("---------APIQA: ServiceList: ");
                Console.Write("[{0}]", string.Join(", ", directoryRefresh.ServiceList));
                Console.WriteLine("\n\n");
                // END APIQA
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
                // APIQA
                if (!m_ItemsRequested && m_WatchlistConsumerConfig.DelayInitialRequest)
                {
                    if (m_FieldDictionaryLoaded && m_EnumDictionaryLoaded || m_ItemDecoder.FieldDictionaryLoadedFromFile && m_ItemDecoder.EnumTypeDictionaryLoadedFromFile)
                    {
                        RequestItems(reactorEvent.ReactorChannel!);
                    }
                }
                if (m_WatchlistConsumerConfig.ReqItemBeforeLogin)
                {
                    if (m_FieldDictionaryLoaded && m_EnumDictionaryLoaded || m_ItemDecoder.FieldDictionaryLoadedFromFile && m_ItemDecoder.EnumTypeDictionaryLoadedFromFile)
                    {
                        RequestItems(reactorEvent.ReactorChannel!);
                    }
                }
                // END APIQA
                break;

            case DirectoryMsgType.UPDATE:
                DirectoryUpdate? directoryUpdate = reactorEvent.DirectoryMsg?.DirectoryUpdate;

                serviceName = chnlInfo!.ConnectionArg.Service;
                Console.WriteLine("Received Source Directory Update");
                Console.WriteLine(directoryUpdate?.ToString());

                Console.WriteLine("Domain: " + DomainTypes.ToString(DomainType.SOURCE));
                Console.WriteLine($"Stream: {reactorEvent.Msg?.StreamId} Msg Class: {MsgClasses.ToString(MsgClasses.UPDATE)}");

                // APIQA:  Added this line
                Console.Write("---------APIQA: ServiceList: ");
                Console.Write("[{0}]", string.Join(", ", directoryUpdate!.ServiceList));
                Console.WriteLine("\n\n");
                // END APIQA:  Added this line

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

                // APIQA
                if (m_WatchlistConsumerConfig.LoginPauseAndResume)
                {
                    m_UpdateCount++;
                    if (m_UpdateCount == 3)
                    {
                        // Do a Login RESUME
                        LoginRequest loginRequest = chnlInfo.ConsumerRole.RdmLoginRequest!;
                        loginRequest.Flags &= ~LoginRequestFlags.PAUSE_ALL;
                        loginRequest.Flags |= LoginRequestFlags.NO_REFRESH;
                        m_SubmitOptions.Clear();
                        if (chnlInfo.ReactorChannel!.Submit(loginRequest, m_SubmitOptions, out var errorInfo) != ReactorReturnCode.SUCCESS)
                        {
                            Console.WriteLine("------------APIQA: attempted login reissue failed. Error: " + errorInfo!.Error.Text);
                        }
                        else
                        {
                            Console.WriteLine("------------APIQA: RESUME-ALL login reissue done.");
                        }
                    }
                }
                // End APIQA

                // APIQA
                m_E1Counter++;
                if (m_AllEventCounters.Count > 0)
                {
                    for (int i = 0; i < m_AllEventCounters.Count; ++i)
                    {
                        if (m_AllEventCounters[i].Delay == m_E1Counter && m_AllEventCounters[i].EventType == 1)
                        {
                            int startIdx = m_AllEventCounters[i].StartIdx;
                            int endIdx = m_AllEventCounters[i].EndIdx;
                            for (int index = startIdx; index < endIdx; ++index)
                            {
                                bool isSnap = m_WatchlistConsumerConfig.GetMPItemInfo(index, "isSnapshot");
                                bool isView = m_WatchlistConsumerConfig.GetMPItemInfo(index, "isView");
                                bool isPrivate = m_WatchlistConsumerConfig.GetMPItemInfo(index, "isPrivate");
                                bool isMsgKeyInUpdates = m_WatchlistConsumerConfig.GetMPItemInfo(index, "isMsgKeyInUpdates");
                                int vID;
                                vID = m_WatchlistConsumerConfig.GetViewId(index);
                                string nameOfItem = m_WatchlistConsumerConfig.GetNameOfItem(index)!;
                                RequestNewItem(m_Reactor!, reactorEvent.ReactorChannel!, false, isPrivate, isSnap, isView, vID, isMsgKeyInUpdates, nameOfItem);
                            }
                            m_AllEventCounters.RemoveAt(i);
                            break;
                        }
                    }
                }
                // END APIQA
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
                        Console.WriteLine("Decoding Field Dictionary failed");
                        chnlInfo?.ReactorChannel?.Close(out _);
                    }
                }
                else if (dictionaryRefresh.StreamId == chnlInfo.EnumDictionaryStreamId)
                {
                    if (chnlInfo.Dictionary.DecodeEnumTypeDictionary(chnlInfo.DIter, VerbosityValues.VERBOSE, out var errorInfo) == CodecReturnCode.SUCCESS)
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
                        Console.WriteLine("Decoding EnumType Dictionary failed: " + errorInfo.Text);
                        chnlInfo?.ReactorChannel?.Close(out _);
                    }
                }
                else
                {
                    Console.WriteLine("Received unexpected dictionary message on stream " + dictionaryRefresh.StreamId);
                }

                if (m_FieldDictionaryLoaded && m_EnumDictionaryLoaded)
                    RequestItems(chnlInfo?.ReactorChannel!);

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

    public ReactorCallbackReturnCode ReactorServiceEndpointEventCallback(ReactorServiceEndpointEvent reactorEvent)
    {
        if (reactorEvent.ReactorErrorInfo.Code == ReactorReturnCode.SUCCESS)
        {
            string? endPoint = null;
            string? port = null;
            List<ReactorServiceEndpointInfo> serviceEndpointInfoList = reactorEvent.ServiceEndpointInfoList!;

            foreach (var info in serviceEndpointInfoList)
            {
                if (info.LocationList.Count >= 2 && m_WatchlistConsumerConfig.Location != null &&
                        info.LocationList[0].StartsWith(m_WatchlistConsumerConfig.Location)) // Get an endpoint that provides auto failover for the specified location
                {
                    endPoint = info?.EndPoint;
                    port = info?.Port;
                    break;
                }
                // Try to get backups and keep looking for main case. Keep only the first item met.
                else if (info.LocationList.Count > 0 && m_WatchlistConsumerConfig.Location != null &&
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

    private bool IsRequestedServiceUp(ChannelInfo chnlInfo)
    {
        return chnlInfo.HasServiceInfo &&
                chnlInfo.ServiceInfo.HasState && (!chnlInfo.ServiceInfo.State.HasAcceptingRequests ||
                                                         chnlInfo.ServiceInfo.State.AcceptingRequests == 1) && chnlInfo.ServiceInfo.State.ServiceStateVal == 1;
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

        if (m_WatchlistConsumerConfig.PublisherId != null && m_WatchlistConsumerConfig.PublisherAddress != null)
            chnlInfo.PostHandler.SetPublisherInfo(m_WatchlistConsumerConfig.PublisherId, m_WatchlistConsumerConfig.PublisherAddress);
    }

    /// <summary>
    /// On and off stream posting if enabled
    /// </summary>
    private void HandlePosting()
    {
        foreach (var chnlInfo in m_ChnlInfoList)
        {
            if (chnlInfo.LoginRefresh == null ||
                chnlInfo.ServiceInfo == null ||
                chnlInfo.ReactorChannel == null ||
                chnlInfo.ReactorChannel.State != ReactorChannelState.READY ||
                !IsRequestedServiceUp(chnlInfo))
            {
                continue;
            }

            chnlInfo.PostItemName.Clear();

            if (chnlInfo.PostHandler.EnableOnStreamPost)
            {
                WatchlistConsumerConfig.ItemInfo? postingItem = null;

                // Find a first MarketPrice item
                // If found, send on-stream posts on it.
                for (int i = 0; i < m_WatchlistConsumerConfig.ItemCount; i++)
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

                chnlInfo.PostHandler.StreamId = postingItem.StreamId;
                chnlInfo.PostHandler.PostItemName.Data(postingItem.Name);
                chnlInfo.PostHandler.ServiceId = chnlInfo.ServiceInfo.ServiceId;
                chnlInfo.PostHandler.Dictionary = chnlInfo.Dictionary;

                CodecReturnCode ret = chnlInfo.PostHandler.HandlePosts(chnlInfo.ReactorChannel, out var errorInfo);
                if (ret < CodecReturnCode.SUCCESS)
                    Console.WriteLine("Error posting onstream: " + errorInfo!.Error.Text);
            }
            if (chnlInfo.PostHandler.EnableOffStreamPost)
            {
                chnlInfo.PostHandler.StreamId = chnlInfo.LoginRefresh.StreamId;
                chnlInfo.PostHandler.PostItemName.Data("OFFPOST");
                chnlInfo.PostHandler.ServiceId = chnlInfo.ServiceInfo.ServiceId;
                chnlInfo.PostHandler.Dictionary = chnlInfo.Dictionary;
                CodecReturnCode ret = chnlInfo.PostHandler.HandlePosts(chnlInfo.ReactorChannel, out var errorInfo);
                if (ret < CodecReturnCode.SUCCESS)
                    Console.WriteLine("Error posting offstream: " + errorInfo!.Error.Text);
            }
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
        if (!string.IsNullOrEmpty(m_WatchlistConsumerConfig.UserName))
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
        // APIQA
        int singleOpenArg = m_WatchlistConsumerConfig.SingleOpen;
        Console.WriteLine("\nAPIQA: SingleOpen set to " + singleOpenArg);
        chnlInfo.ConsumerRole.RdmLoginRequest.LoginAttrib.SingleOpen = singleOpenArg;
        int allowSuspectArg = m_WatchlistConsumerConfig.AllowSuspectData;
        chnlInfo.ConsumerRole.RdmLoginRequest!.LoginAttrib.HasAllowSuspectData = true;
        Console.WriteLine("APIQA: Allow Suspect Data set to " + allowSuspectArg);
        chnlInfo.ConsumerRole.RdmLoginRequest!.LoginAttrib.AllowSuspectData = allowSuspectArg;
        // END APIQA
        // APIQA
        if (m_WatchlistConsumerConfig.ReqDirWithSID != 0)
        {
            chnlInfo.ConsumerRole.RdmDirectoryRequest!.HasServiceId = true;
            chnlInfo.ConsumerRole.RdmDirectoryRequest.ServiceId = m_WatchlistConsumerConfig.ReqDirWithSID;
        }
        // End APIQA
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

        // APIQA
        chnlInfo.PostHandler.PostMultipart = m_WatchlistConsumerConfig.EnablePostMultipart;
        // END APIQA

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
                Console.WriteLine($"Error: {errorInfo?.Code} Text: " + errorInfo!.Error.Text);
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


    // APIQA:
    private void CloseLoginStream(ChannelInfo chnlInfo)
    {
        /* encode item close */
        m_SubmitOptions.Clear();
        m_CloseMsg.Clear();
        m_CloseMsg.MsgClass = MsgClasses.CLOSE;
        m_CloseMsg.StreamId = 1;
        m_CloseMsg.DomainType = (int)DomainType.LOGIN;
        m_CloseMsg.ContainerType = DataTypes.NO_DATA;

        if (chnlInfo.ReactorChannel!.Submit((Msg)m_CloseMsg, m_SubmitOptions, out _) != ReactorReturnCode.SUCCESS)
        {
            Console.WriteLine("\n\n----------------APIQA: Closing login stream Failed.\n");
        }
        else
            Console.WriteLine("\n\n----------------APIQA: Closd LOGIN stream.\n");
    }
    // END APIQA:

    private void CloseItemStreams(ChannelInfo chnlInfo)
    {
        // have offstream posting post close status
        if (chnlInfo.ShouldOffStreamPost)
        {
            chnlInfo.PostHandler.StreamId = chnlInfo.LoginRefresh.StreamId;
            chnlInfo.PostHandler.PostItemName.Data("OFFPOST");
            chnlInfo.PostHandler.ServiceId = chnlInfo.ServiceInfo.ServiceId;
            chnlInfo.PostHandler.Dictionary = chnlInfo.Dictionary;
            if (chnlInfo.ReactorChannel is not null)
                chnlInfo.PostHandler.CloseOffStreamPost(chnlInfo.ReactorChannel, out _);
        }

        for (int itemListIndex = 0; itemListIndex < m_WatchlistConsumerConfig.ItemCount; ++itemListIndex)
        {
            int domainType = (int)m_WatchlistConsumerConfig.ItemList[itemListIndex].Domain;
            int streamId = m_WatchlistConsumerConfig.ItemList[itemListIndex].StreamId;

            /* encode item close */
            m_CloseMsg.Clear();
            m_CloseMsg.MsgClass = MsgClasses.CLOSE;
            m_CloseMsg.StreamId = streamId;
            m_CloseMsg.DomainType = domainType;
            m_CloseMsg.ContainerType = DataTypes.NO_DATA;

            if (chnlInfo?.ReactorChannel?.Submit((Msg)m_CloseMsg, m_SubmitOptions, out _) != ReactorReturnCode.SUCCESS)
            {
                Console.WriteLine("Close itemStream of " + streamId + " Failed.");
            }
        }
    }

    /// <summary>
    /// Uninitializes the Value Add consumer application.
    /// </summary>
    internal void Uninitialize()
    {
        Console.WriteLine("Consumer unitializing and exiting...");

        foreach (var chnlInfo in m_ChnlInfoList)
        {
            // close items streams
            CloseItemStreams(chnlInfo);

            // close ReactorChannel
            if (chnlInfo.ReactorChannel != null)
            {
                chnlInfo?.ReactorChannel?.Close(out _);
            }
        }

        // shutdown reactor
        m_Reactor?.Shutdown(out _);
    }

    private void HandleClose()
    {
        Console.WriteLine("Consumer closes streams...");

        foreach (var chnlInfo in m_ChnlInfoList)
        {
            //APIQA
            if (m_WatchlistConsumerConfig.TestOnlyLoginClose)
            {
                CloseLoginStream(chnlInfo);
                Console.WriteLine("-----------APIQA: Closing only consumer login stream when run time expires\n\n");
            }
            // End APIQA
            CloseItemStreams(chnlInfo);
        }
    }

    private static void SetEncryptedConfiguration(ConnectOptions options)
    {
        options.EncryptionOpts.EncryptionProtocolFlags = EncryptionProtocolFlags.ENC_TLSV1_2;
        options.EncryptionOpts.EncryptedProtocol = ConnectionType.SOCKET;
    }

    /// <summary>
    /// Only BASIC authentication is supported: HTTPproxyUsername, HTTPproxyPasswd
    /// </summary>
    /// <param name="options">Connect options</param>
    private void SetCredentials(ConnectOptions options)
    {
        string? proxyUsername = m_WatchlistConsumerConfig.ProxyUsername;
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
}
