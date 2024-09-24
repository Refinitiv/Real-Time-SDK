/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.Md for details.
 *|           Copyright (C) 2023 LSEG. All rights reserved.     
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
using LSEG.Eta.Example.VACommon;
using LSEG.Eta.Rdm;
using LSEG.Eta.Transports;
using LSEG.Eta.ValueAdd.Rdm;
using LSEG.Eta.ValueAdd.Reactor;

using static LSEG.Eta.Rdm.Dictionary;
using static Rdm.LoginMsgType;

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
/// <li>-clientId Specifies the client Id for V2 authentication OR specifies a unique ID, also known as AppKey generated by an AppGenerator, for V1 authentication usedwhen connecting to Real-Time Optimized. 
/// <li>-clientSecret Specifies the associated client Secret with a provided clientId for V2 logins.
/// <li>-jwkFile Specifies the file containing the JWK encoded key for V2 JWT logins.
/// <li>-tokenURL Specifies the token URL for V2 token oauthclientcreds grant type.
/// <li>-restEnableLog (optional) Enable REST logging message.
/// <li>-restLogFileName Set REST logging output stream.
/// <li>-rtt enables rtt support by a consumer. If provider make distribution of RTT messages, consumer will return back them. In another case, consumer will ignore them.
/// </ul>
/// </summary>
/// 


//APIQA
class WatchlistConsumer
{
    WatchlistConsumerConfig? m_WatchlistConsumerConfig;
    private System.DateTime? m_Runtime;

    private List<ConsumerCallbackThread> m_ConsumerList = new List<ConsumerCallbackThread>();
    private List<ChannelInfo> m_ChanInfoList = new List<ChannelInfo>();
    bool m_ShutDown = false;

    /* Initializes the Value Add consumer application. */
    public void Init(String[] args)
    {
        m_WatchlistConsumerConfig = new WatchlistConsumerConfig();
        m_WatchlistConsumerConfig.AddCommandLineArgs();

        if (!m_WatchlistConsumerConfig.Init(args))
        {
            Console.Error.WriteLine("\nError loading command line arguments:\n");
            Console.Error.WriteLine(CommandLine.OptionHelpString());
            Environment.Exit((int)CodecReturnCode.FAILURE);
        }

        // display product version information
        Console.WriteLine(Codec.MajorVersion() + "." + Codec.MinorVersion());
        Console.WriteLine("WatchlistConsumer initializing...");

        m_Runtime = System.DateTime.Now + +TimeSpan.FromSeconds(m_WatchlistConsumerConfig.Runtime * 1000);

        /* create per reactor/watchlist, per channel info per thread which initializes channel info, and connect channels
         * for each connection specified */
        foreach (ConnectionArg connectionArg in m_WatchlistConsumerConfig.ConnectionList)
        {
            // create channel info
            ChannelInfo chnlInfo = new ChannelInfo();
            chnlInfo.ConnectionArg = connectionArg;
            ConnectOptions connectOptions = chnlInfo.ConnectInfo.ConnectOptions;
            connectOptions.UserSpecObject = chnlInfo;

            ConsumerCallbackThread consumerCallbackThread = new ConsumerCallbackThread(chnlInfo, this);
            Thread thread = new Thread(new ThreadStart(consumerCallbackThread.Run));
            thread.Start();

            //wait when the thread starts
            while (!thread.IsBackground);

            m_ConsumerList.Add(consumerCallbackThread);
            m_ChanInfoList.Add(chnlInfo);
        }

        DistributeItemRequest();
    }

    void DistributeItemRequest()
    {
        List<WatchlistConsumerConfig.ItemInfo> itemList = m_WatchlistConsumerConfig!.ItemList;
        int numOfItems = itemList.Count();
        int numOfConsumer = m_ConsumerList.Count();
        int maxItemPerConsumer = numOfItems / numOfConsumer;
        int consumerIndex = 0;
        List<WatchlistConsumerConfig.ItemInfo>? itemsPerConsumer = null;
        for (int i = 0; i < numOfItems; i++)
        {
            if (i % maxItemPerConsumer == 0)
            {
                if (consumerIndex < numOfConsumer)
                {
                    itemsPerConsumer = m_ConsumerList[consumerIndex]!.m_ConsumerRequestThread!.m_ItemInfoList;
                }
                else
                    return;

                consumerIndex++;
            }

            //itemsPerConsumer!.Add(itemList[i]);
        }
        return;
    }

    /** Shutdown Consumer */
    public void Shutdown()
    {
        m_ShutDown = true;
    }

    public WatchlistConsumerConfig? WatchlistConsumerConfig()
    {
        return m_WatchlistConsumerConfig;
    }

    /* Runs the Value Add consumer application. */
    public void Run()
    {
        // main statistics polling thread here
        while (!m_ShutDown)
        {
            try
            {
                Thread.Sleep(1000);
            }
            catch (Exception e)
            {
                Console.WriteLine("Thread.sleep(1000) failed: {0}", e.Message);
                Environment.Exit((int)CodecReturnCode.FAILURE);
            }

            // Handle run-time
            if (System.DateTime.Now >= m_Runtime)
            {
                Console.WriteLine("MultithreadConsumer run-time expired, close now...");
                break;
            }
        }

        foreach (ConsumerCallbackThread consumer in m_ConsumerList)
        {
            consumer.m_ShutDown = (true);
        }

        try
        {
            Thread.Sleep(5000);
        }
        catch (Exception e)
        {
            Console.WriteLine("Thread.sleep(1000) failed: {0}", e.Message);
            Environment.Exit((int)CodecReturnCode.FAILURE);
        }
    }

}
