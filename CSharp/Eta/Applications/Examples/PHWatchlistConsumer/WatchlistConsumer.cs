/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2025-2026 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Net;
using System.Net.Sockets;
using System.Text;
using LSEG.Eta.Codec;
using LSEG.Eta.Example.Common;
using LSEG.Eta.Rdm;
using LSEG.Eta.Transports;
using LSEG.Eta.ValueAdd.Rdm;
using LSEG.Eta.ValueAdd.Reactor;
using static LSEG.Eta.Rdm.Dictionary;
using static LSEG.Eta.ValueAdd.WatchlistConsumer.WatchlistConsumerConfig;
using EncryptionProtocolFlags = LSEG.Eta.ValueAdd.WatchlistConsumer.WatchlistConsumerConfig.EncryptionProtocolFlags;

namespace LSEG.Eta.ValueAdd.WatchlistConsumer;
/// <summary>
/// <p>
/// This is a main class to run the ETA Value Add WatchlistConsumer application.
/// </p>
/// <H2>Summary</H2>
/// <p>
/// This is the main file for the WatchlistConsumer application.  It is a single-threaded
/// client application that utilizes the ETA Reactor's watchlist using a json config input.
/// The main consumer file provides the callback for channel events and
/// the default callback for processing RsslMsgs. The main function
/// Initializes the ETA Reactor, makes the desired connections, and
/// dispatches for events.
/// This application makes use of the RDM namespace for easier decoding of Login &amp; Source Directory
/// messages.
/// </p>
/// <p>
/// This application supports consuming Level I Market Price, Level II Market By Price, and
/// Level II Market By Order data.
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
/// <li>-x Provides XML tracing of messages.
/// <li>-runTime run time. Default is 600 seconds. Controls the time the
/// application will run before exiting, in seconds.
/// <li>-u Login user name. Default is system user name.
/// <li>-passwd Password for the user name.
/// <li>-clientId Specifies a unique ID for application making the request to RDP token service, also known as AppKey generated using an AppGenerator.
/// <li>-at Specifies the Authentication Token. If this is present, the login user name type will be Login.UserIdTypes.AUTHN_TOKEN.
/// <li>-ax Specifies the Authentication Extended information.
/// <li>-aid Specifies the Application ID.
/// <li>-clientId Specifies the client Id for Refinitiv login V2, or specifies a unique ID with login V1 for applications making the request to EDP token service, this is also known as AppKey generated using an AppGenerator.
/// <li>-clientSecret Specifies the associated client Secret with a provided clientId for V2 logins.
/// <li>-jwkFile Specifies the file containing the JWK encoded key for V2 JWT logins.
/// <li>-tokenURLV2 Specifies the token URL for V2 token oauthclientcreds grant type.
/// <li>-configJson Json Configuration file.
/// <li>-reconnectLimit Reconnection attempt count
/// <li>-restProxyHost Proxy host name. Used for Rest requests only: service discovery, auth
/// <li>-restProxyPort Proxy port.Used for Rest requests only: service discovery, auth
/// <li>-restProxyUserName Proxy user name.Used for Rest requests only: service discovery, auth
/// <li>-restProxyPasswd Proxy password.Used for Rest requests only: service discovery, auth
/// <li>Reconnection options:
/// <ul>
/// <li>-reconnectCount Specifies the maximum number of reconnection attempts.
/// <li>-reconnectMinDelay Specifies minimum delay (in milliseconds) between reconnection attempts.
/// <li>-reconnectMaxDelay Specifies maximum delay(in milliseconds) between reconnection attempts.
/// </ul>
/// </li>
/// <li>Options for IOCtl calls:
/// <ul>
/// <ul>-fallBackInterval specifies time interval(in second) in application before Ad Hoc Fallback function is invoked.O indicates that function won't be invoked.
/// <li>-ioctlInterval specifies time interval(in second) before IOCtl function is invoked.O indicates that function won't be invoked.
/// <li>-ioctlEnablePH enables Preferred host feature.
/// <li>-ioctlConnectListIndex specifies the preferred host as the index in the connection list.
/// <li>-ioctlDetectionTimeInterval specifies time interval(in second) to switch over to a preferred host. 0 indicates that the detection time interval is disabled.
/// <li>-ioctlDetectionTimeSchedule specifies Cron time format to switch over to a preferred host.
/// </ul>
/// </li>
/// </summary>
public class WatchlistConsumer : IConsumerCallback
{
    private const string FIELD_DICTIONARY_DOWNLOAD_NAME = "RWFFld";
    private const string ENUM_TABLE_DOWNLOAD_NAME = "RWFEnum";
    private const int FIELD_DICTIONARY_STREAM_ID = 3;
    private const int ENUM_DICTIONARY_STREAM_ID = 4;
    private const int GUARANTEED_OUTPUT_BUFFERS = 1000;
    private const string TIMESTAMP_FORMAT = "yyyy-MM-dd HH:mm:ss.fff";
    private Reactor.Reactor? m_Reactor;
    private readonly ReactorOptions m_ReactorOptions = new();
    private readonly ReactorDispatchOptions m_DispatchOptions = new();
    private readonly WatchlistConsumerConfig m_WatchlistConsumerConfig;

    private System.DateTime m_Runtime;

    private readonly ReactorSubmitOptions m_SubmitOptions = new();
    private readonly ReactorOAuthCredential m_ReactorOAuthCredential = new();

    private readonly TimeSpan m_Closetime = TimeSpan.FromSeconds(10);
    private System.DateTime m_CloseRunTime;
    private readonly ItemDecoder m_ItemDecoder = new();
    private bool m_ItemsRequested = false;

    private bool m_FieldDictionaryLoaded;
    private bool m_EnumDictionaryLoaded;

    private System.DateTime m_IoctlTime;
    private bool m_IsIOCtlCalled;
    private System.DateTime m_FallbackTime;
    private bool m_IsFallbackCalled;

    private readonly ICloseMsg m_CloseMsg = new Msg();

    private readonly ItemRequest m_ItemRequest = new();

    private readonly Dictionary<int, ReactorChannel> m_SocketFdValueMap = new();
    private readonly Dictionary<string, Service> m_ServiceMap = new();
    private readonly List<Socket> m_Sockets = new();

    private readonly ChannelInfo m_chnlInfo = new();

    private static ReactorChannelInfo m_ReactorChannelInfo = new();
    public WatchlistConsumer(WatchlistConsumerConfig config)
    {
        m_DispatchOptions.SetMaxMessages(1000);
        m_WatchlistConsumerConfig = config;
    }

    /// <summary>
    /// Initializes the Watchlist consumer application.
    /// </summary>
    /// <param name="args">Command line args.</param>
    internal void Init()
    {
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
        SetRestProxy(m_ReactorOptions.RestProxyOptions);

        m_ReactorOptions.EnableRestLogStream = m_WatchlistConsumerConfig.EnableRestLogging;

        if (!string.IsNullOrEmpty(m_WatchlistConsumerConfig.RestLogFileName))
        {
            try
            {
                var m_FileStream = new FileStream(m_WatchlistConsumerConfig.RestLogFileName, FileMode.Create);
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
        if ((errorInfo is not null && errorInfo.Code != ReactorReturnCode.SUCCESS) || m_Reactor is null)
        {
            Console.WriteLine("Reactor.CreateReactor() failed: " + errorInfo?.ToString());
            Environment.Exit((int)ReactorReturnCode.FAILURE);
        }

        m_Sockets.Add(m_Reactor.EventSocket!);
        m_chnlInfo.ConnectOptions = ReadReactorConnectOptionsFromConfig(m_WatchlistConsumerConfig);
        m_chnlInfo.ConnectOptions.ConnectionList.ForEach(c => c.ConnectOptions.UserSpecObject = m_chnlInfo);
        // initialize channel info
        InitChannelInfo(m_chnlInfo);

        // Initialize timers for ioctl and fallback calls
        m_IoctlTime = System.DateTime.Now + TimeSpan.FromSeconds(m_WatchlistConsumerConfig.IoctlInterval);
        m_FallbackTime = System.DateTime.Now + TimeSpan.FromSeconds(m_WatchlistConsumerConfig.FallBackInterval);

        // connect channel
        ReactorReturnCode ret;
        if ((ret = m_Reactor!.Connect(m_chnlInfo.ConnectOptions, m_chnlInfo.ConsumerRole, out var eInfo)) < ReactorReturnCode.SUCCESS)
        {
            Console.WriteLine("Reactor.Connect() failed with return code: " + ret + " error = " + eInfo?.Error.Text);
            Environment.Exit((int)ReactorReturnCode.FAILURE);
        }
    }

    private void SetRestProxy(ProxyOptions proxyOptions)
    {
        proxyOptions.ProxyHostName = m_WatchlistConsumerConfig.RestProxyHost;
        proxyOptions.ProxyPort = m_WatchlistConsumerConfig.RestProxyPort;
        proxyOptions.ProxyUserName = m_WatchlistConsumerConfig.RestProxyUsername;
        proxyOptions.ProxyPassword = m_WatchlistConsumerConfig.RestProxyPassword;
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

            System.DateTime currentTime = System.DateTime.Now;
            if (m_WatchlistConsumerConfig.IoctlInterval > 0 && currentTime >= m_IoctlTime && !m_IsIOCtlCalled)
            {
                ReactorChannel? rc = m_chnlInfo.ReactorChannel;

                if (rc != null && rc.State != ReactorChannelState.DOWN_RECONNECTING && m_WatchlistConsumerConfig.IoctlOverridesConfigPreferredHost)
                {
                    ModifyIOCtl(rc);
                    m_IsIOCtlCalled = true;
                }
            }

            if (m_WatchlistConsumerConfig.FallBackInterval > 0 && currentTime >= m_FallbackTime && !m_IsFallbackCalled)
            {
                ReactorChannel? rc = m_chnlInfo.ReactorChannel;
                if (rc != null)
                {
                    FallbackPreferredHost(rc);
                    m_IsFallbackCalled = true;
                }
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

            if (System.DateTime.Now >= m_Runtime)
            {
                Console.WriteLine("Consumer run-time expired, close now...");
                break;
            }
            else if (System.DateTime.Now >= m_CloseRunTime)
            {
                Console.WriteLine("Consumer closetime expired, shutdown reactor.");
                break;
            }
            if (m_chnlInfo.isChannelClosed)
            {
                break;
            }
        }
    }

    private void ModifyIOCtl(ReactorChannel reactorChannel)
    {
        ReactorPreferredHostOptions mergedPreferredHostOptions = new();
        PreferredHost? jsonPreferredHostOptions = m_WatchlistConsumerConfig.Config?.PreferredHost;

        mergedPreferredHostOptions.EnablePreferredHostOptions = m_WatchlistConsumerConfig.IoctlEnablePH
            ?? m_WatchlistConsumerConfig.Config?.PreferredHost?.EnablePH 
            ?? m_WatchlistConsumerConfig.IoctlConnectListIndex.HasValue || m_WatchlistConsumerConfig.IoctlDetectionTimeSchedule is not null;

        mergedPreferredHostOptions.ConnectionListIndex = m_WatchlistConsumerConfig.IoctlConnectListIndex
            ?? jsonPreferredHostOptions?.ConnectListIndex ?? 0;

        mergedPreferredHostOptions.DetectionTimeInterval = (uint)(m_WatchlistConsumerConfig.IoctlDetectionTimeInterval
            ?? jsonPreferredHostOptions?.DetectionTimeInterval ?? 0);

        mergedPreferredHostOptions.DetectionTimeSchedule = m_WatchlistConsumerConfig.IoctlDetectionTimeSchedule
            ?? jsonPreferredHostOptions?.DetectionTimeSchedule ?? string.Empty;

        if (reactorChannel.IOCtl(ReactorChannelIOCtlCode.PREFERRED_HOST_OPTIONS, mergedPreferredHostOptions, out var errorInfo) != ReactorReturnCode.SUCCESS)
        {
            Console.WriteLine("channel.IOCtl() was failed: " + errorInfo?.ToString());
        }
        else
        {
            Console.WriteLine("channel.IOCtl() was successful");
        }
    }

    private static void FallbackPreferredHost(ReactorChannel reactorChannel)
    {
        if (reactorChannel.FallbackPreferredHost(out var errorInfo) != ReactorReturnCode.SUCCESS)
        {
            Console.WriteLine("channel.FallbackPreferredHost() was failed: " + errorInfo?.Error.Text);
        }
        else
        {
            Console.WriteLine("channel.FallbackPreferredHost() was successful");
        }
    }

    /// <summary>
    /// Requests the desired items.
    /// </summary>
    private CodecReturnCode RequestItems(ReactorChannel channel)
    {
        if (m_ItemsRequested)
            return CodecReturnCode.SUCCESS;

        m_ItemsRequested = true;

        for (int itemListIndex = 0; itemListIndex < m_WatchlistConsumerConfig.ItemList.Count; ++itemListIndex)
        {
            m_ItemRequest.Clear();

            m_ItemRequest.HasStreaming = true;

            m_ItemRequest.AddItem(m_WatchlistConsumerConfig.ItemList[itemListIndex].Name!);

            var domainType = m_WatchlistConsumerConfig.ItemList[itemListIndex].Domain;

            m_ItemRequest.DomainType = domainType;
            m_ItemRequest.StreamId = m_WatchlistConsumerConfig.ItemList[itemListIndex].StreamId;

            m_ItemRequest.Encode();
            m_SubmitOptions.Clear();
            m_SubmitOptions.ServiceName = m_WatchlistConsumerConfig.ItemList[itemListIndex].ServiceName;
            if (m_WatchlistConsumerConfig.ServiceName != null)
            {
                m_SubmitOptions.ServiceName = m_WatchlistConsumerConfig.ServiceName;
            }

            var subRet = channel.Submit(m_ItemRequest.RequestMsg, m_SubmitOptions, out var errorInfo);
            if (subRet < ReactorReturnCode.SUCCESS)
            {
                Console.WriteLine("\nReactorChannel.Submit() failed: " + subRet + "(" + errorInfo?.Error.Text + ")\n");
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

    private static void DumpTimestamp() =>
        Console.WriteLine("<!-- " + System.DateTime.UtcNow.ToString(TIMESTAMP_FORMAT) + " (UTC) -->");

    public ReactorCallbackReturnCode ReactorChannelEventCallback(ReactorChannelEvent reactorEvent)
    {
        ChannelInfo? chnlInfo = (ChannelInfo)reactorEvent.ReactorChannel!.UserSpecObj!;
        DumpTimestamp();
        switch (reactorEvent.EventType)
        {
            case ReactorChannelEventType.CHANNEL_UP:
                {
                    if (reactorEvent.ReactorChannel.Channel != null)
                        Console.WriteLine("Channel Up Event: " + FormatReactorChannel(reactorEvent.ReactorChannel));
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
                        Console.WriteLine("Channel Ready Event: " + FormatReactorChannel(reactorEvent.ReactorChannel));
                    else
                        Console.WriteLine("Channel Ready Event");

                    reactorEvent.ReactorChannel?.Info(m_ReactorChannelInfo, out var _);
                    PrintPreferredHostInfo(m_ReactorChannelInfo.PreferredHostInfo);

                    break;
                }
            case ReactorChannelEventType.CHANNEL_OPENED:
                {
                    // set ReactorChannel on ChannelInfo, again need this?
                    chnlInfo.ReactorChannel = reactorEvent.ReactorChannel;

                    if (m_FieldDictionaryLoaded && m_EnumDictionaryLoaded
                        || m_ItemDecoder.FieldDictionaryLoadedFromFile && m_ItemDecoder.EnumTypeDictionaryLoadedFromFile)
                        RequestItems(reactorEvent.ReactorChannel);
                    else
                        RequestDictionaries(reactorEvent.ReactorChannel, chnlInfo);

                    break;
                }
            case ReactorChannelEventType.CHANNEL_DOWN_RECONNECTING:
                {
                    if (!reactorEvent.ReactorChannel?.Socket?.SafeHandle.IsClosed ?? false)
                        Console.WriteLine("\nConnection down reconnecting: " + FormatReactorChannel(reactorEvent.ReactorChannel!));
                    else
                        Console.WriteLine("\nConnection down reconnecting");

                    if (!string.IsNullOrEmpty(reactorEvent.ReactorErrorInfo?.Error.Text))
                        Console.WriteLine("\tError text: " + reactorEvent.ReactorErrorInfo.Error.Text + "\n");

                    reactorEvent.ReactorChannel?.Info(m_ReactorChannelInfo, out var _);
                    PrintPreferredHostInfo(m_ReactorChannelInfo.PreferredHostInfo);

                    // allow Reactor to perform connection recovery

                    // unregister selectableChannel from Selector
                    if (reactorEvent.ReactorChannel?.Socket != null)
                    {
                        UnregisterSocket(reactorEvent.ReactorChannel.Socket);
                    }

                    m_ItemsRequested = false;

                    // reset canSendLoginReissue flag
                    chnlInfo.CanSendLoginReissue = false;
                    break;
                }
            case ReactorChannelEventType.CHANNEL_DOWN:
                {
                    if (reactorEvent.ReactorChannel!.Socket != null)
                        Console.WriteLine("\nConnection down: " + reactorEvent.ReactorChannel.Socket.Handle.ToInt32());
                    else
                        Console.WriteLine("\nConnection down");

                    if (reactorEvent.ReactorErrorInfo != null && reactorEvent.ReactorErrorInfo.Error.Text != null)
                        Console.WriteLine("    Error text: " + reactorEvent.ReactorErrorInfo.Error.Text + "\n");

                    CloseConnection(chnlInfo);
                    break;
                }
            case ReactorChannelEventType.WARNING:
                Console.WriteLine("Received ReactorChannel WARNING event.");
                if (reactorEvent.ReactorErrorInfo != null && reactorEvent.ReactorErrorInfo.Error.Text != null)
                    Console.WriteLine("    Error text: " + reactorEvent.ReactorErrorInfo.Error.Text + "\n");

                break;
            case ReactorChannelEventType.PREFERRED_HOST_STARTING_FALLBACK:
                Console.WriteLine($"Received ReactorChannel PREFERRED_HOST_STARTING_FALLBACK event.");
                if (reactorEvent.ReactorErrorInfo != null && reactorEvent.ReactorErrorInfo.Error.Text != null)
                    Console.WriteLine("    Error text: " + reactorEvent.ReactorErrorInfo.Error.Text + "\n");

                reactorEvent.ReactorChannel?.Info(m_ReactorChannelInfo, out var _);
                PrintPreferredHostInfo(m_ReactorChannelInfo.PreferredHostInfo);

                break;
            case ReactorChannelEventType.PREFERRED_HOST_COMPLETE:
				Console.WriteLine($"Received ReactorChannel PREFERRED_HOST_COMPLETE event.");
				if (reactorEvent.ReactorErrorInfo != null && reactorEvent.ReactorErrorInfo.Error.Text != null)
					Console.WriteLine("    Error text: " + reactorEvent.ReactorErrorInfo.Error.Text + "\n");

                reactorEvent.ReactorChannel?.Info(m_ReactorChannelInfo, out var _);
                PrintPreferredHostInfo(m_ReactorChannelInfo.PreferredHostInfo);
                break;
            case ReactorChannelEventType.PREFERRED_HOST_NO_FALLBACK:
                Console.WriteLine($"Received ReactorChannel PREFERRED_HOST_NO_FALLBACK event.");
                if (reactorEvent.ReactorErrorInfo != null && reactorEvent.ReactorErrorInfo.Error.Text != null)
                    Console.WriteLine("    Error text: " + reactorEvent.ReactorErrorInfo.Error.Text + "\n");

                reactorEvent.ReactorChannel?.Info(m_ReactorChannelInfo, out var _);
                PrintPreferredHostInfo(m_ReactorChannelInfo.PreferredHostInfo);
                break;
            default:
                {
                    Console.WriteLine("Unknown channel event!\n");
                    return ReactorCallbackReturnCode.SUCCESS;
                }
        }

        return ReactorCallbackReturnCode.SUCCESS;
    }

    private static string FormatReactorChannel(ReactorChannel reactorChannel)
           => $"Channel {reactorChannel.Socket!.Handle.ToInt32()} Host: {reactorChannel.HostName}:{reactorChannel.Port}";


    private void PrintPreferredHostInfo(ReactorPreferredHostInfo reactorPreferredHostInfo)
    {
        if (!reactorPreferredHostInfo.IsPreferredHostEnabled)
        {
            return;
        }

        StringBuilder stringBuilder = new(256);
        stringBuilder.Append("\nPreferredHostInfo:")
            .Append("\n\tPreferredHostEnabled=").Append(reactorPreferredHostInfo.IsPreferredHostEnabled)
            .Append("\n\tDetectionTimeSchedule='").Append(reactorPreferredHostInfo.DetectionTimeSchedule).Append('\'')
            .Append("\n\tDetectionTimeInterval=").Append(reactorPreferredHostInfo.DetectionTimeInterval)
            .Append("\n\tConnectionListIndex=").Append(reactorPreferredHostInfo.ConnectionListIndex)
            .Append("\n");

        Console.WriteLine(stringBuilder);
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
        WatchlistConsumerConfig.ItemInfo? item;

        ChannelInfo? chnlInfo = (ChannelInfo?)reactorEvent.ReactorChannel?.UserSpecObj;

        var msg = reactorEvent.Msg;

        if (msg == null)
        {
            /* The message is not present because an error occurred while decoding it. Print
            * the error and close the channel. If desired, the un-decoded message buffer
            * is available in reactorEvent.TransportBuffer(). */

            Console.Write("defaultMsgCallback: {0}({1})\n", reactorEvent.ReactorErrorInfo.Error.Text, reactorEvent.ReactorErrorInfo.Location);

            CloseConnection(chnlInfo, reactorEvent.ReactorChannel);
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
                    }
                }
                else if (item != null)
                {
                    itemName = item?.Name?.ToString();
                }

                Console.WriteLine($"DefaultMsgCallback Refresh ItemName: {GetItemName()} Domain: {DomainTypes.ToString(refreshMsg.DomainType)}, StreamId: {refreshMsg.StreamId}");

                Console.WriteLine("                      State: " + refreshMsg.State);

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

        return ReactorCallbackReturnCode.SUCCESS;
    }

    public ReactorCallbackReturnCode RdmLoginMsgCallback(RDMLoginMsgEvent reactorEvent)
    {
        var chnlInfo = (ChannelInfo)reactorEvent.ReactorChannel!.UserSpecObj!;
        var msgType = reactorEvent.LoginMsg!.LoginMsgType;

        switch (msgType)
        {
            case LoginMsgType.REFRESH:
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
                }
                if (reactorEvent.LoginMsg.LoginRefresh.State.StreamState() != StreamStates.OPEN)
                {
                    Console.WriteLine("Login attempt failed");
                    CloseConnection(chnlInfo);
                }
                break;

            case LoginMsgType.STATUS:
                LoginStatus loginStatus = reactorEvent.LoginMsg.LoginStatus!;
                Console.WriteLine($"Domain: {DomainTypes.ToString(DomainType.LOGIN)}, StreamId: {reactorEvent.LoginMsg.LoginStatus?.StreamId}");
                Console.WriteLine("Received Login StatusMsg");
                if (loginStatus.HasState)

                {
                    Console.WriteLine("	" + loginStatus.State);
                }
                if (loginStatus.HasUserName)
                    Console.WriteLine(" UserName: " + loginStatus.UserName.ToString());
                if (loginStatus.State.StreamState() != StreamStates.OPEN)
                {
                    Console.WriteLine("Login attempt failed");
                    CloseConnection(chnlInfo);
                }
                break;

            case LoginMsgType.RTT:
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

        if (chnlInfo == null)
        {
            Console.WriteLine("ChannelInfo is null in passed reactorEvent.ReactorChannel.UserSpecObj");
            return ReactorCallbackReturnCode.FAILURE;
        }

        switch (msgType)
        {
            case DirectoryMsgType.REFRESH:
                DirectoryRefresh directoryRefresh = reactorEvent.DirectoryMsg!.DirectoryRefresh!;
                Console.WriteLine("Domain: " + DomainTypes.ToString(DomainType.SOURCE));
                Console.WriteLine("Stream: " + reactorEvent.DirectoryMsg.StreamId + " Msg Class: " + MsgClasses.ToString(MsgClasses.REFRESH));
                Console.WriteLine(directoryRefresh.State.ToString());

                serviceList = directoryRefresh.ServiceList;

                foreach (var service in serviceList)
                {
                    if (service.Info.ServiceName.ToString() != null)
                    {
                        if (m_ServiceMap.TryGetValue(service.Info.ServiceName.ToString(), out Service? tmpService))
                        {
                            if (service.Action == MapEntryActions.DELETE && service.ServiceId == chnlInfo.ServiceInfo.ServiceId)
                            {
                                chnlInfo.ServiceInfo.Action = MapEntryActions.DELETE;
                            }

                            bool updateServiceInfo = false;
                            Console.WriteLine("Received serviceName: " + service.Info.ServiceName + "\n");
                            // update service cache - assume cache is built with previous refresh message
                            if (service.ServiceId == tmpService!.ServiceId)
                            {
                                updateServiceInfo = true;
                            }

                            if (updateServiceInfo)
                            {
                                // update serviceInfo associated with requested service name
                                if (service.Copy(tmpService) < CodecReturnCode.SUCCESS)
                                {
                                    Console.WriteLine("Service.copy() failure");
                                    Uninitialize();
                                    Environment.Exit((int)ReactorReturnCode.FAILURE);
                                }
                            }
                        }
                        else
                        {
                            Service newService = new();

                            service.Copy(newService);

                            m_ServiceMap.Add(newService.Info.ServiceName.ToString(), newService);
                        }
                    }
                }

                break;

            case DirectoryMsgType.UPDATE:
                DirectoryUpdate? directoryUpdate = reactorEvent.DirectoryMsg?.DirectoryUpdate;

                Console.WriteLine("Received Source Directory Update");
                Console.WriteLine(directoryUpdate?.ToString());

                Console.WriteLine("Domain: " + DomainTypes.ToString(DomainType.SOURCE));
                Console.WriteLine($"Stream: {reactorEvent.Msg?.StreamId} Msg Class: {MsgClasses.ToString(MsgClasses.UPDATE)}");

                serviceList = directoryUpdate?.ServiceList;


                if (serviceList is not null)
                    foreach (var service in serviceList)
                    {
                        if (m_ServiceMap.TryGetValue(service.Info.ServiceName.ToString(), out Service? tmpService))
                        {
                            if (service.Action == MapEntryActions.DELETE && service.ServiceId == chnlInfo.ServiceInfo.ServiceId)
                            {
                                chnlInfo.ServiceInfo.Action = MapEntryActions.DELETE;
                            }

                            bool updateServiceInfo = false;
                            Console.WriteLine("Received serviceName: " + service.Info.ServiceName + "\n");
                            // update service cache - assume cache is built with previous refresh message
                            if (service.ServiceId == tmpService.ServiceId)
                            {
                                updateServiceInfo = true;
                            }

                            if (updateServiceInfo)
                            {
                                // update serviceInfo associated with requested service name
                                if (service.Copy(tmpService) < CodecReturnCode.SUCCESS)
                                {
                                    Console.WriteLine("Service.Copy() failure");
                                    Uninitialize();
                                    Environment.Exit((int)ReactorReturnCode.FAILURE);
                                }
                            }
                        }
                        else
                        {
                            Service newService = new();

                            service.Copy(newService);

                            m_ServiceMap.Add(newService.Info.ServiceName.ToString(), newService);
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

        if (chnlInfo == null)
        {
            Console.WriteLine("ChannelInfo is null in passed reactorEvent.ReactorChannel.UserSpecObj");
            return ReactorCallbackReturnCode.FAILURE;
        }
        if (chnlInfo!.ReactorChannel == null)
        {
            Console.WriteLine("ReactorChannel is null in passed ChannelInfo");
            return ReactorCallbackReturnCode.FAILURE;
        }

        // initialize dictionary
        chnlInfo.Dictionary ??= new();

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
                            CloseConnection(chnlInfo);
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
                        CloseConnection(chnlInfo);
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
                        CloseConnection(chnlInfo);
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



    private void CloseConnection(ChannelInfo? chnlInfo, ReactorChannel? reactorChannel = null)
    {
        ReactorChannel? reactorChannelToClose = chnlInfo?.ReactorChannel ?? reactorChannel;
        // unregister selectableChannel from Selector
        if (reactorChannelToClose?.Socket != null)
        {
            UnregisterSocket(reactorChannelToClose.Socket);
        }
        if (chnlInfo is not null)
        {
            Console.WriteLine("Consumer closes streams...");
            CloseItemStreams(chnlInfo);
        }
        // close ReactorChannel
        if (reactorChannelToClose is not null && reactorChannelToClose.Close(out var closeError) != ReactorReturnCode.SUCCESS)
        {
            Console.WriteLine("ReactorChannel.Close() failed: " + closeError!.Error.Text);
        }
        if (chnlInfo is not null)
        {
            chnlInfo.isChannelClosed = true;
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
        }

        if (!string.IsNullOrEmpty(m_WatchlistConsumerConfig.Password))
        {
            chnlInfo.ConsumerRole.RdmLoginRequest?.Password.Data(m_WatchlistConsumerConfig.Password);
            if (chnlInfo.ConsumerRole.RdmLoginRequest is not null)
                chnlInfo.ConsumerRole.RdmLoginRequest.HasPassword = true;
        }

        if (!string.IsNullOrEmpty(m_WatchlistConsumerConfig.ClientId))
        {
            m_ReactorOAuthCredential.ClientId.Data(m_WatchlistConsumerConfig.ClientId);
            chnlInfo.ConsumerRole.ReactorOAuthCredential = m_ReactorOAuthCredential;
            m_ReactorOAuthCredential.ClientId.Data(m_WatchlistConsumerConfig.ClientId);
            m_ReactorOAuthCredential.ClientSecret.Data(m_WatchlistConsumerConfig.ClientSecret);
            if (!string.IsNullOrEmpty(m_WatchlistConsumerConfig.ClientSecret))
            {
                m_ReactorOAuthCredential.ClientSecret.Data(m_WatchlistConsumerConfig.ClientSecret);
                m_ReactorOAuthCredential.ClientSecret.Data(m_WatchlistConsumerConfig.ClientSecret);
            }
        }

        try
        {
            if (!string.IsNullOrEmpty(m_WatchlistConsumerConfig.JwkFile))
            {
                string jwkData = File.ReadAllText(m_WatchlistConsumerConfig.JwkFile);
                m_ReactorOAuthCredential.ClientJwk.Data(jwkData);
                chnlInfo.ConsumerRole.ReactorOAuthCredential = m_ReactorOAuthCredential;
                m_ReactorOAuthCredential.ClientJwk.Data(jwkData);
            }
        }
        catch (Exception e)
        {
            Console.WriteLine("Error loading JWK file: " + e.Message);
            Console.WriteLine();
            Console.WriteLine(CommandLine.OptionHelpString());
            Console.WriteLine("Consumer exits...");
            Environment.Exit((int)CodecReturnCode.FAILURE);
        }

        if (!string.IsNullOrEmpty(m_WatchlistConsumerConfig.TokenScope))
        {
            m_ReactorOAuthCredential.TokenScope.Data(m_WatchlistConsumerConfig.TokenScope);
            chnlInfo.ConsumerRole.ReactorOAuthCredential = m_ReactorOAuthCredential;
        }

        if (!string.IsNullOrEmpty(m_WatchlistConsumerConfig.Audience))
        {
            m_ReactorOAuthCredential.Audience.Data(m_WatchlistConsumerConfig.Audience);
            chnlInfo.ConsumerRole.ReactorOAuthCredential = m_ReactorOAuthCredential;
        }

        /* Passing data to OAuth callback */
        m_ReactorOAuthCredential.UserSpecObj = m_ReactorOAuthCredential;

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

        if (m_ItemDecoder.FieldDictionaryLoadedFromFile == true &&
            m_ItemDecoder.EnumTypeDictionaryLoadedFromFile == true)
        {
            chnlInfo.Dictionary = m_ItemDecoder.Dictionary;
        }
    }

    private void CloseItemStreams(ChannelInfo chnlInfo)
    {
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
            if (chnlInfo.ReactorChannel is not null)
            if (chnlInfo!.ReactorChannel!.Submit((Msg)m_CloseMsg, m_SubmitOptions, out var erroInfo) != ReactorReturnCode.SUCCESS)
            {
                Console.WriteLine($"Close itemStream of {streamId} failed: {erroInfo?.Error.Text}");
            }
        }
    }

    /// <summary>
    /// Uninitializes the Value Add consumer application.
    /// </summary>
    internal void Uninitialize()
    {
        Console.WriteLine("Consumer unitializing and exiting...");
        CloseConnection(m_chnlInfo);

        // shutdown reactor
        m_Reactor?.Shutdown(out _);
    }

    private void HandleClose()
    {
        Console.WriteLine("Consumer closes streams...");
        CloseItemStreams(m_chnlInfo);
    }

    private static ReactorConnectOptions ReadReactorConnectOptionsFromConfig(WatchlistConsumerConfig config)
    {
        ReactorConnectOptions reactorConnectOpts = new();
        reactorConnectOpts.SetReconnectAttempLimit(config.ReconnectAttempLimit);
        if (config.Config is not null)
        {
            foreach (var conn in config.Config.ConnectionList)
            {
                ReactorConnectInfo reactorConnectInfo = new()
                {
                    EnableSessionManagement = conn.SessionMgnt ?? false,
                    Location = conn.Location ?? string.Empty,
                };
                reactorConnectInfo.ConnectOptions.UnifiedNetworkInfo.Address = conn.Host ?? string.Empty;
                reactorConnectInfo.ConnectOptions.UnifiedNetworkInfo.ServiceName = conn.Port?.ToString() ?? string.Empty;
                reactorConnectInfo.ConnectOptions.UnifiedNetworkInfo.InterfaceName = conn.InterfaceName ?? string.Empty;
                reactorConnectInfo.ConnectOptions.ConnectionType = (ConnectionType)(conn.ConnType ?? (int)ConnectionType.SOCKET);
                reactorConnectInfo.ConnectOptions.EncryptionOpts.EncryptedProtocol = (ConnectionType)(conn.EncryptedConnType ?? (int)ConnectionType.SOCKET);
                if (conn.EncryptionProtocolFlags is not null)
                {
                    if ((conn.EncryptionProtocolFlags! & EncryptionProtocolFlags.TLSv1_2) == EncryptionProtocolFlags.TLSv1_2)
                    {
                        reactorConnectInfo.ConnectOptions.EncryptionOpts.EncryptionProtocolFlags |= Transports.EncryptionProtocolFlags.ENC_TLSV1_2;
                    }
                    if ((conn.EncryptionProtocolFlags! & EncryptionProtocolFlags.TLSv1_3) == EncryptionProtocolFlags.TLSv1_3)
                    {
                        reactorConnectInfo.ConnectOptions.EncryptionOpts.EncryptionProtocolFlags |= Transports.EncryptionProtocolFlags.ENC_TLSV1_3;
                    }
                }
                reactorConnectInfo.ConnectOptions.ProxyOptions.ProxyPort = conn.ProxyPort ?? string.Empty;
                reactorConnectInfo.ConnectOptions.ProxyOptions.ProxyPassword = conn.ProxyPassword ?? string.Empty;
                reactorConnectInfo.ConnectOptions.ProxyOptions.ProxyUserName = conn.ProxyUserName ?? string.Empty;
                reactorConnectInfo.ConnectOptions.ProxyOptions.ProxyHostName = conn.ProxyHost ?? string.Empty;
                reactorConnectInfo.ConnectOptions.MajorVersion = Codec.Codec.MajorVersion();
                reactorConnectInfo.ConnectOptions.MinorVersion = Codec.Codec.MinorVersion();
                reactorConnectInfo.ConnectOptions.GuaranteedOutputBuffers = GUARANTEED_OUTPUT_BUFFERS;
                reactorConnectOpts.ConnectionList.Add(reactorConnectInfo);
            }
            reactorConnectOpts.SetReconnectMinDelay(config.ReconnectMinDelay);
            reactorConnectOpts.SetReconnectMaxDelay(config.ReconnectMaxDelay);
            var preferredHostOptions = reactorConnectOpts.PreferredHostOptions;
            preferredHostOptions.EnablePreferredHostOptions = config.Config.PreferredHost?.EnablePH ?? config.Config.PreferredHost?.ConnectListIndex is not null;
            preferredHostOptions.ConnectionListIndex = config.Config.PreferredHost?.ConnectListIndex ?? 0;
            preferredHostOptions.DetectionTimeInterval = (uint)(config.Config.PreferredHost?.DetectionTimeInterval ?? 0);
            preferredHostOptions.DetectionTimeSchedule = config.Config.PreferredHost?.DetectionTimeSchedule ?? string.Empty;
        }

        return reactorConnectOpts;
    }
}
