/*
 *|-------------------------------------------------------------------------------
 *| This source code is provided under the Apache 2.0 license
 *| AS IS with no warranty or guarantee of fit for purpose.
 *| See LICENSE.md for details.
 *| Copyright (C) 2022-2024 LSEG. All rights reserved.                     --
 *|-------------------------------------------------------------------------------
 */

/*********************************************************************************
 * This is the ETA Consumer Training series of the ETA Training Suite
 * applications. The purpose of this application is to show step-by-step
 * training how to build a ETA OMM Consumer using the ETA Transport layer.
 *
 * Main CSharp source file for the ETA Consumer Training application. It is a
 * single-threaded client application.
 *
 *********************************************************************************
 * ETA Consumer Training Module 1a: Establish network communication
 *********************************************************************************
 *
 * Summary:
 *
 * In this module, the application initializes the ETA Transport and
 * connects the client. An OMM consumer application can establish a
 * connection to other OMM Interactive Provider applications, including
 * Real-Time Distribution Systems, Data Feed Direct,
 * and LSEG Real-Time.
 *
 * Detailed Descriptions:
 *
 * The first step of any ETA consumer application is to establish a
 * network connection with its peer component (i.e., another application
 * with which to interact). An OMM consumer typically creates an out-bound
 * connection to the well-known hostname and port of a server (Interactive
 * Provider or ADS). The consumer uses the Channel.connect() function to initiate
 * the connection and then uses the Channel.init() function to complete
 * channel initialization.
 *
 * For this simple training app, only a single channel/connection is used for
 * the entire life of this app.
 *********************************************************************************
 * Command line usage:
 *
 * ./ConsMod1a [-h <SrvrHostname>] [-p <SrvrPortNo>] [-i <InterfaceName>]
 * (runs with a default set of parameters (-h localhost -p 14002 -i ""))
 * (runs with specified set of parameters, all parameters are optional)
 *
 * Pressing the CTRL+C buttons terminates the program.
 *
 *********************************************************************************
 * ETA Consumer Training Module 1b: Ping (heartbeat) Management
 *********************************************************************************
 *
 * Summary:
 *
 * Ping or heartbeat messages indicate the continued presence of an application.
 * After the consumer connection is active, ping messages must be exchanged.
 * The negotiated ping timeout is retrieved using the GetChannelInfo() function.
 * The connection will be terminated if ping heartbeats are not sent or received
 * within the expected time frame.
 *
 * Detailed Descriptions:
 *
 * Ping or heartbeat messages are used to indicate the continued presence of
 * an application. These are typically only required when no other information
 * is being exchanged. For example, there may be long periods of time that
 * elapse between requests made from an OMM consumer application. In this
 * situation, the consumer would send periodic heartbeat messages to inform
 * the providing application that it is still alive. Because the provider
 * application is likely sending more frequent information, providing updates
 * on any streams the consumer has requested(like update for market price), it may not need to send
 * heartbeats as the other data is sufficient to announce its continued
 * presence. It is the responsibility of each connection to manage the sending
 * and receiving of heartbeat messages.
 *
 * Command line usage:
 *
 * ./ConsMod1b [-h <SrvrHostname>] [-p <SrvrPortNo>] [-i <InterfaceName>] [-r <Running Time>]
 * (runs with a default set of parameters (-h localhost -p 14002 -i "" -r 300))
 * (runs with specified set of parameters, all parameters are optional)
 *
 * Pressing the CTRL+C buttons terminates the program.
 *
 *********************************************************************************
 * ETA Consumer Training Module 1c: Reading and Writing Data
 *********************************************************************************
 *
 * Summary:
 *
 * When channel initialization is complete, the state of the channel
 * Channel.state() is TransportReturnCodes.ACTIVE, and applications can send
 * and receive data.
 *
 * Detailed Descriptions:
 *
 * When a client or server Channel.state() is ChannelState.ACTIVE, it is
 * possible for an application to receive data from the connection. The
 * arrival of this information is checked with Select method. The ETA
 * Transport reads information from the network as a byte stream, after
 * which it determines buffer boundaries and returns each buffer one by
 * one.
 *
 * When a client or server Channel.state() is ChannelState.ACTIVE, it is
 * possible for an application to write data to the connection. Writing
 * involves a several step process.
 *
 * Because the ETA Transport provides efficient buffer management, the user is
 * required to obtain a buffer from the ETA Transport buffer pool. This can be
 * the guaranteed output buffer pool associated with a Channel. After a buffer
 * is acquired, the user can populate the Buffer.Data and set the Buffer.Length
 * to the number of bytes referred to by data.
 *
 * If queued information cannot be passed to the network, a function is provided
 * to allow the application to continue attempts to flush data to the
 * connection. An pool mechanism can be used to help with determining when the
 * network is able to accept additional bytes for writing. The ETA Transport can
 * continue to queue data, even if the network is unable to write.
 *
 * Command line usage:
 *
 * ./ConsMod1c [-h <SrvrHostname>] [-p <SrvrPortNo>] [-i <InterfaceName>] [-r <Running Time>]
 * (runs with a default set of parameters (-h localhost -p 14002 -i "" -r 300))
 * (runs with specified set of parameters, all parameters are optional)
 *
 * Pressing the CTRL+C buttons terminates the program.
 *
 *********************************************************************************
 * ETA Consumer Training Module 2: Log in
 *********************************************************************************
 *
 * Summary:
 *
 * Applications authenticate using the Login domain model. An OMM consumer must
 * authenticate with a provider using a Login request prior to issuing any other
 * requests or opening any other streams. After receiving a Login request, an
 * Interactive Provider determines whether a user is permissioned to access the
 * system. The Interactive Provider sends back a Login response, indicating to
 * the consumer whether access is granted.
 *
 * Detailed Descriptions:
 *
 * After receiving a Login request, an Interactive Provider determines whether
 * a user is permissioned to access the system. The Interactive Provider sends
 * back a Login response, indicating to the consumer whether access is granted.
 *
 * a) If the application is denied, the Login stream is closed, and the
 *    consumer application cannot send additional requests.
 *
 * b) If the application is granted access, the Login response contains
 *    information about available features, such as Posting, Pause and Resume, and
 *    the use of Dynamic Views. The consumer application can use this information
 *    to tailor its interaction with the provider.
 *
 * Content is encoded and decoded using the ETA Message Package and the ETA
 * Data Package.
 *
 * Command line usage:
 *
 * ./ConsMod2 [-h <SrvrHostname>] [-p <SrvrPortNo>] [-i <InterfaceName>] [-r <Running Time>]
 * (runs with a default set of parameters (-h localhost -p 14002 -i "" -r 300))
 * (runs with specified set of parameters, all parameters are optional)
 *
 * Pressing the CTRL+C buttons terminates the program.
 *
 *********************************************************************************
 * ETA Consumer Training Module 3: Obtain Source Directory
 *********************************************************************************
 *
 * Summary:
 *
 * The Source Directory domain model conveys information about all available
 * services in the system. An OMM consumer typically requests a Source
 * Directory to retrieve information about available services and their
 * capabilities. This includes information about supported domain types, the
 * service's state, the quality of service (QoS), and any item group
 * information associated with the service.
 *
 * Detailed Descriptions:
 *
 * The Source Directory Info filter contains service name and serviceId
 * information for all available services. When an appropriate service is
 * discovered by the OMM consumer, it uses the serviceId associated with the
 * service on all subsequent requests to that service.
 *
 * The Source Directory State filter contains status information for service,
 * which informs the consumer whether the service is Up and available, or Down
 * and unavailable.
 *
 * The Source Directory Group filter conveys item group status information,
 * including information about group states, as well as the merging of groups.
 *
 * Content is encoded and decoded using the ETA Message Package and the ETA
 * Data Package.
 *
 * Command line usage:
 *
 * ./ConsMod2 [-h <SrvrHostname>] [-p <SrvrPortNo>] [-i <InterfaceName>] [-r <Running Time>] [-s <Service Name>]
 * (runs with a default set of parameters (-h localhost -p 14002 -i "" -r 300 -s DIRECT_FEED))
 * (runs with specified set of parameters, all parameters are optional)
 *
 * Pressing the CTRL+C buttons terminates the program.
 *
 *********************************************************************************
 * ETA Consumer Training Module 3: Obtain Source Directory
 *********************************************************************************
 *
 * Summary:
 *
 * The Source Directory domain model conveys information about all available
 * services in the system. An OMM consumer typically requests a Source
 * Directory to retrieve information about available services and their
 * capabilities. This includes information about supported domain types, the
 * service's state, the quality of service (QoS), and any item group
 * information associated with the service.
 *
 * Detailed Descriptions:
 *
 * The Source Directory Info filter contains service name and serviceId
 * information for all available services. When an appropriate service is
 * discovered by the OMM consumer, it uses the serviceId associated with the
 * service on all subsequent requests to that service.
 *
 * The Source Directory State filter contains status information for service,
 * which informs the consumer whether the service is Up and available, or Down
 * and unavailable.
 *
 * The Source Directory Group filter conveys item group status information,
 * including information about group states, as well as the merging of groups.
 *
 * Content is encoded and decoded using the ETA Message Package and the ETA
 * Data Package.
 *
 * Command line usage:
 *
 * ./ConsMod4 [-h <SrvrHostname>] [-p <SrvrPortNo>] [-i <InterfaceName>] [-r <Running Time>] [-s <Service Name>]
 * (runs with a default set of parameters (-h localhost -p 14002 -i "" -r 300 -s DIRECT_FEED))
 * (runs with specified set of parameters, all parameters are optional)
 *
 * Pressing the CTRL+C buttons terminates the program.
 *
 *********************************************************************************
 * ETA Consumer Training Module 5: Issue Item Requests
 *********************************************************************************
 *
 * Summary:
 *
 * After the consumer application successfully logs in and obtains Source
 * Directory and Dictionary information, it can request additional content.
 *
 * When issuing the request, the consuming application specifies the serviceId
 * of the desired service along with a streamId. Requests can be sent for any
 * domain using the formats defined in that domain model specification. In this
 * simple example, we show how to make a Market Price level I data Item request
 * to obtain the data from a provider.
 *
 * Detailed Descriptions:
 *
 * The Market Price domain provides access to Level I market information such as
 * trades, indicative quotes, and top-of-book quotes. All information is sent as
 * an FieldList. Field-value pairs contained in the field list include
 * information related to that item (i.e., net change, bid, ask, volume, high,
 * low, or last price).
 *
 * A Market Price request message is encoded and sent by OMM consumer
 * applications. The request specifies the name and attributes of an item in
 * which the consumer is interested. If a consumer wishes to receive updates, it
 * can make a "streaming" request by setting the RequestMsgFlags.STREAMING
 * flag. If the flag is not set, the consumer is requesting a "snapshot," and
 * the refresh should end the request.
 *
 * Market Price data is conveyed as an FieldList, where each FieldEntry
 * corresponds to a piece of information and its current value. The field list
 * should be decoded using its associated Field Dictionary, indicated by the
 * dictionaryId present in the field list.
 *
 * Similar to module_Dictionary, the main change is to add one more option in
 * the second main loop.
 *
 * Also added 4 more functions: SendMarketPriceItemRequest(),
 * ProcessMarketPriceItemResponse(), DecodeMarketPricePayload() and
 * CloseMarketPriceItemStream(). More details are in those functions.
 *
 * Content is encoded and decoded using the ETA Message Package and the ETA
 * Data Package.
 *
 * Command line usage:
 *
 * ./ConsMod5 [-h <SrvrHostname>] [-p <SrvrPortNo>] [-i <InterfaceName>] [-r <Running Time>] [-s <Service Name>] [-mp <Market Price Item Name>]
 * (runs with a default set of parameters (-h localhost -p 14002 -i "" -r 300 -s DIRECT_FEED -mp TRI))
 * (runs with specified set of parameters, all parameters are optional)
 *
 * Pressing the CTRL+C buttons terminates the program.
 *
 *********************************************************************************/

using System;
using System.Collections.Generic;
using System.Linq;
using System.Net.Sockets;

using LSEG.Eta.Transports;
using LSEG.Eta.Common;
using LSEG.Eta.Codec;
using LSEG.Eta.Rdm;
using LSEG.Eta.Example.Common;

using Buffer = LSEG.Eta.Codec.Buffer;
using Array = LSEG.Eta.Codec.Array;

namespace LSEG.Eta.Training.Consumer
{
    public class Module_5_ItemRequest
    {
        #region Global Variables

        static List<Socket> readSocketList = new();
        static List<Socket> writeSocketList = new();

        /* A channel to keep track of connection */
        static IChannel? channel;

        static long channelFDValue = -1;

        /* dictionary file name  */
        static string fieldDictionaryFileName = "RDMFieldDictionary";
        /* dictionary download name */
        static string dictionaryDownloadName = "RWFFld";

        /* enum table file name */
        static string enumTypeDictionaryFileName = "enumtype.def";
        /* enum table download name */
        static string enumTableDownloadName = "RWFEnum";

        const string FIELD_DICTIONARY_FILE_NAME = "RDMFieldDictionary";
        const string ENUM_TABLE_FILE_NAME = "enumtype.def";

        const int FIELD_DICTIONARY_STREAM_ID = 3;
        const int ENUM_TYPE_DICTIONARY_STREAM_ID = 4;
        const int MARKETPRICE_ITEM_STREAM_ID = 5;

        static UInt serviceDiscoveryInfo_serviceId = new();

        static bool serviceDiscoveryInfo_serviceNameFound = false;
        static bool serviceDiscoveryInfo_etalDMTDictionarySupported = false;
        static bool serviceDiscoveryInfo_etaDMTMarketPriceSupported = false;
        static bool serviceDiscoveryInfo_RDMFieldDictionaryProvided = false;
        static bool serviceDiscoveryInfo_enumtypeProvided = false;
        static int serviceDiscoveryInfo_ServiceState;
        static int serviceDiscoveryInfo_AcceptingRequests;
        static List<Qos> serviceDiscoveryInfo_qoS = new List<Qos>();

        static string serviceDiscoveryInfo_serviceName = String.Empty;

        static bool dictionariesLoadedInfo_fieldDictionaryLoaded = false;
        static bool dictionariesLoadedInfo_enumTypeDictionaryLoaded = false;
        static bool dictionariesLoadedInfo_fieldDictionaryFirstPart = true;
        static bool dictionariesLoadedInfo_enumTypeDictionaryFirstPart = true;
        static bool dictionariesLoadedInfo_isInitialized = false;

        static string marketPriceItemInfo_itemName = String.Empty;
        static State marketPriceItemInfo_itemState = new();

        #endregion

        public static void Main(string[] args)
        {
            #region Declare Variables

            TransportReturnCode retCode;

            /* Create error to keep track of any errors that may occur in Transport methods */
            Error error = new();

            /* Create and populate connect options to specify connection preferences */
            ConnectOptions cOpts = new();

            /* InProgInfo Information for the In Progress Connection State */
            InProgInfo inProgInfo = new();

            /*  Channel Info returned by GetChannelInfo call */
            ChannelInfo channelInfo = new();

            ITransportBuffer? msgBuf;

            System.DateTime currentTime;
            System.DateTime etaRuntime;
            /* use default runTime of 300 seconds */
            TimeSpan runTime = TimeSpan.FromSeconds(300);

            /* Iterator used for decoding throughout the application - we can clear it and reuse it instead of recreating it */
            DecodeIterator decodeIter = new();

            /* Create initialization arguments (InitArgs) to specify any initialization options for Transport */
            InitArgs initArgs = new();

            MarketPriceItem marketPriceItem = new();

            /* the default option parameters */
            /* connect to server running on same machine */
            string srvrHostname = "localhost";
            /* server is running on port number 14003 */
            string srvrPortNo = "14002";
            /* use default NIC network interface card to bind to for all inbound and outbound data */
            string interfaceName = "";
            /* default service name is "DIRECT_FEED" used in source directory handler */
            string serviceName = "DIRECT_FEED";
            /* default item name is "TRI" used in market price item request handler */
            string itemName = "TRI";

            #endregion

            /* User specifies options such as address, port, and interface from the command line.
             * User can have the flexibility of specifying any or all of the parameters in any order.
             */

            if (args.Length > 0)
            {
                int i = 0;

                while (i < args.Length)
                {
                    if ((args[i].Equals("-h")) == true)
                    {
                        i += 2;
                        srvrHostname = args[i - 1];
                    }
                    else if ((args[i].Equals("-p")) == true)
                    {
                        i += 2;
                        srvrPortNo = args[i - 1];
                    }
                    else if ((args[i].Equals("-i")) == true)
                    {
                        i += 2;
                        interfaceName = args[i - 1];
                    }
                    else if ((args[i].Equals("-r")) == true)
                    {
                        i += 2;
                        runTime = TimeSpan.FromSeconds(Int32.Parse(args[i - 1]));
                    }
                    else if ((args[i].Equals("-s")) == true)
                    {
                        i += 2;
                        serviceName = args[i - 1];
                    }
                    else if ((args[i].Equals("-mp")) == true)
                    {
                        i += 2;
                        itemName = args[i - 1];
                    }
                    else
                    {
                        Console.Write("Error: Unrecognized option: {0}\n\n", args[i]);
                        Console.Write("Usage: {0} or\n{0} [-h <SrvrHostname>] [-p <SrvrPortNo>] [-i <InterfaceName>] [-r <runTime>] [-s <ServiceName>] [-mp <ItemName>]\n",
                            System.AppDomain.CurrentDomain.FriendlyName);
                        Environment.Exit((int)TransportReturnCode.FAILURE);
                    }
                }
            }

            DataDictionary dictionary = new();

            /*************************************
             * Step 1) Initialize Transport *
             *************************************/
            /*********************************************************
             * Client/Consumer Application Lifecycle Major Step 1: Initialize ETA
             * Transport using Initialize The first ETA Transport function that an
             * application should call. This creates and initializes internal memory
             * and structures, as well as performing any bootstrapping for
             * underlying dependencies. The Initialize function also allows the user
             * to specify the locking model they want applied to the ETA Transport.
             *********************************************************/
            if (Transport.Initialize(initArgs, out error) != TransportReturnCode.SUCCESS)
            {
                Console.Write("Error ({0}) (errno: {1}): {2}", error.ErrorId, error.SysError, error.Text);
                Environment.Exit((int)TransportReturnCode.FAILURE);
            }

            currentTime = System.DateTime.Now;
            etaRuntime = currentTime + runTime;

            /* Set connect options */
            /* populate connect options, then pass to Connect function -
             * ETA Transport should already be initialized
             */
            /* use standard socket connection */
            cOpts.ConnectionType = ConnectionType.SOCKET; /* (0) Channel is a standard TCP socket connection type */
            cOpts.UnifiedNetworkInfo.Address = srvrHostname;
            cOpts.UnifiedNetworkInfo.ServiceName = srvrPortNo;
            cOpts.UnifiedNetworkInfo.InterfaceName = interfaceName;
            cOpts.Blocking = false;
            cOpts.PingTimeout = 60;
            cOpts.CompressionType = CompressionType.NONE;

            /* populate version and protocol with RWF information or protocol specific info */
            cOpts.ProtocolType = (Transports.ProtocolType)Codec.Codec.ProtocolType();  /* Protocol type definition for RWF */
            cOpts.MajorVersion = Codec.Codec.MajorVersion();
            cOpts.MinorVersion = Codec.Codec.MinorVersion();

            /* Initializes market price item fields. */
            marketPriceItem.InitFields();
            // isInUse = true;
            // ASK_TIME.localTime();
            // setSALTIME();
            // PERATIO = 5.00;

            // marketPriceItem.RDNDISPLAY = 100;
            // marketPriceItem.RDN_EXCHID = 155;
            // tempBuffer.data = (char *)"05/17/2013";
            // tempBuffer.length = (UInt32)strlen("05/17/2013");
            // DateStringToDate(&marketPriceItem.DIVPAYDATE, &tempBuffer);
            // marketPriceItem.TRDPRC_1 = 1.00;
            // marketPriceItem.BID = 0.99;
            // marketPriceItem.ASK = 1.03;
            // marketPriceItem.ACVOL_1 = 100000;
            // marketPriceItem.NETCHNG_1 = 2.15;

            /* Initialize item handler - initialize/reset market price item information */
            marketPriceItemInfo_itemName = itemName;

            /*********************************************************
             * For performance considerations, it is recommended to first load field and
             * enumerated dictionaries from local files, if they exist, at the earlier
             * stage of the consumer applications.
             *
             * When loading from local files, ETA offers several utility functions to load
             * and manage a properly-formatted field dictionary and enum type dictionary.
             *
             * Only make Market Price item request after both dictionaries are successfully
             * loaded from files.
             *
             * If at least one of the dictionaries fails to get loaded properly, it will
             * continue the code path of downloading the failed loaded dictionary or both
             * dictionaries (if neither exists in run-time path or loaded properly) from
             * provider
             *********************************************************/

            dictionary.Clear();
            CodecError codecError = new();

            /* load field dictionary from file - adds data from a Field Dictionary file to the DataDictionary */
            if (dictionary.LoadFieldDictionary(FIELD_DICTIONARY_FILE_NAME, out codecError) < 0)
            {
                Console.Write("Unable to load field dictionary.  Will attempt to download from provider.\n\tText: {0}\n ",
                    codecError.Text);

            }
            else
            {
                Console.Write("Successfully loaded field dictionary from (local) file.\n\n");
                dictionariesLoadedInfo_fieldDictionaryLoaded = true;
            }

            if (dictionary.LoadEnumTypeDictionary(ENUM_TABLE_FILE_NAME, out codecError) < 0)
            {
                Console.Write("Unable to load enum dictionary.  Will attempt to download from provider.\n\tText: {0}\n",
                    codecError.Text);
            }
            else
            {
                Console.Write("Successfully loaded enum type dictionary from (local) file.\n\n");
                dictionariesLoadedInfo_enumTypeDictionaryLoaded = true;
            }
            if ((dictionariesLoadedInfo_fieldDictionaryLoaded) && (dictionariesLoadedInfo_enumTypeDictionaryLoaded))
            {
                dictionariesLoadedInfo_isInitialized = true;
                Console.Write("ETA Consumer application has successfully loaded both dictionaries from (local) files.\n\n");
            }

            /**************************************************************************************************
             * Connect to Server and receive a channel
             **************************************************************************************************/
            /*********************************************************
             * Client/Consumer Application life cycle Major Step 2: Connect using Connect
             * (OS connection establishment handshake) Connect call Establishes an
             * outbound connection, which can leverage standard sockets.
             *
             * Returns an Channel that represents the connection to the user. In the event
             * of an error, NULL is returned and additional information can be found in
             * the Error structure.  Connection options are passed in via an
             * ConnectOptions structure.
             *********************************************************/
            if ((channel = Transport.Connect(cOpts, out error)) == null)
            {
                Console.Write("Error ({0}) (errno: {1}) encountered with Connect. Error Text : {2}\n",
                    error.ErrorId, error.SysError, error.Text);
                /* End application, uninitialize to clean up first */
                Transport.Uninitialize();
                Environment.Exit((int)TransportReturnCode.FAILURE);
            }

            channelFDValue = channel.Socket.Handle.ToInt64();
            Console.Write("Channel IPC descriptor = {0}\n", channelFDValue);

            /**************************************************************************************************
             * Loop 1) - Keep calling Channel.Init() until it has properly
             * connected us to the server
             **************************************************************************************************/
            /* Main loop for getting connection active and successful completion of the
             * initialization process The loop calls Select() to wait for avaliable data.
             *
             * Currently, the main loop would exit if an error condition is triggered or
             * Channel.State() transitions to ChannelState.ACTIVE.
             */

            /* Writeability in Socket.Select means that the socket is connected. Will be
             * set to false in the end of the cycle once the connection is
             * established.  */
            bool opConnect = true;
            while (channel.State != ChannelState.ACTIVE)
            {
                try
                {
                    PerformSocketSelect(opWrite: opConnect);

                    /* Check if channel is READ-able/WRITE-able */
                    if (readSocketList.Count > 0
                        || writeSocketList.Count > 0)
                    {
                        /****************************************************************************
                         * Step 3) Call Channel.Init() to progress channel initialization further.
                         *
                         * This method is called multiple times throughout the Loop 1, as it makes
                         * more progress towards channel initialization.
                         ***************************************************************************/
                        /*
                         * Internally, the ETA initialization process includes several actions.
                         *
                         * The initialization includes any necessary ETA connection handshake
                         * exchanges.  Compression, ping timeout, and versioning related negotiations
                         * also take place during the initialization process.
                         *
                         * This process involves exchanging several messages across the connection,
                         * and once all message exchanges have completed the Channel.State will
                         * transition. If the connection is accepted and all types of negotiations
                         * completed properly, the Channel.State will become ChannelState.ACTIVE.
                         *
                         * If the connection is rejected, either due to some kind of negotiation
                         * failure or because an Server rejected the connection by setting nakMount
                         * to true, the Channel.State will become ChannelState.CLOSED.
                         *
                         * Note:
                         *
                         * For both client and server channels, more than one call to InitChannel can
                         * be required to complete the channel initialization process.
                         */
                        if ((retCode = channel.Init(inProgInfo, out error)) < TransportReturnCode.SUCCESS)
                        {
                            Console.Write("Error ({0}) (errno: {1}) encountered with Init Channel fd={2}. Error Text: {3}\n",
                                error.ErrorId, error.SysError, channelFDValue, error.Text);
                            CloseChannelCleanUpAndExit(channel, TransportReturnCode.FAILURE, dictionary);
                        }
                        else
                        {
                            /* Deduce an action from return code of Channel.init() */
                            switch (retCode)
                            {
                                case TransportReturnCode.CHAN_INIT_IN_PROGRESS:
                                    {
                                        /* Switch to a new channel if required */

                                        if (inProgInfo.Flags == InProgFlags.SCKT_CHNL_CHANGE)
                                        {
                                            /* SCKT_CHNL_CHANGE indicates that a socketId change has occurred as a result of this call.
                                            *  New SocketFDValue need to be set to update current FD.
                                            *  Shifting from the old socket to the new one handled by the library itself.
                                            */

                                            channelFDValue = inProgInfo.NewSocket.Handle.ToInt64();
                                        }
                                        else
                                        {
                                            Console.Write("Channel {0} in progress...\n", channelFDValue);
                                        }
                                    }
                                    break;
                                /* channel connection becomes active!
                                 *
                                 * Once a connection is established and transitions to the ChannelState.ACTIVE state,
                                 * this Channel can be used for other transport operations.
                                 */
                                case TransportReturnCode.SUCCESS:
                                    {
                                        Console.Write("Channel on fd {0} is now active - reading and writing can begin.\n",
                                            channelFDValue);

                                        /* Populate information from channel */
                                        if ((retCode = channel.Info(channelInfo, out error)) != TransportReturnCode.SUCCESS)
                                        {
                                            Console.Write("Error ({0}) (errno: {1}) encountered with Init Channel fd={2}. Error Text: {3}\n",
                                                error.ErrorId, error.SysError, channelFDValue, error.Text);
                                            CloseChannelCleanUpAndExit(channel, TransportReturnCode.FAILURE, dictionary);
                                        }

                                        /* Print out basic channel info */
                                        Console.Write("\nChannel {0} active. Channel Info:\n"
                                            + "Max Fragment Size:           {1}\n"
                                            + "Output Buffers:              {2} Max, {3} Guaranteed\n"
                                            + "Input Buffers:               {4}\n"
                                            + "Send/Receive Buffer Sizes:   {5}/{6}\n"
                                            + "Ping Timeout:                {7}\n",
                                            channelFDValue,
                                            /* This is the max fragment size before fragmentation and reassembly is necessary. */
                                            channelInfo.MaxFragmentSize,
                                            /* This is the maximum number of output buffers available to the channel. */
                                            channelInfo.MaxOutputBuffers,
                                            /* This is the guaranteed number of output buffers available to the channel. */
                                            channelInfo.GuaranteedOutputBuffers,
                                            /* This is the number of input buffers available to the channel. */
                                            channelInfo.NumInputBuffers,
                                            /* This is the systems Send Buffer size. This reports the systems send buffer size
                                             * respective to the transport type being used (TCP, UDP, etc) */
                                            channelInfo.SysSendBufSize,
                                            /* This is the systems Receive Buffer size. This reports the systems receive buffer
                                             * size respective to the transport type being used (TCP, UDP, etc) */
                                            channelInfo.SysRecvBufSize,
                                            /* This is the value of the negotiated ping timeout */
                                            channelInfo.PingTimeout);

                                        Console.Write("Connected component version: ");
                                        if (channelInfo.ComponentInfoList.Count == 0)
                                            Console.WriteLine("(No component info)");
                                        else
                                        {
                                            Console.WriteLine(string.Join(", ",
                                                channelInfo.ComponentInfoList.Select(ci => ci.ComponentVersion)));
                                        }
                                    }
                                    break;
                                default: /* Error handling */
                                    {
                                        Console.Write("Bad return value fd={0} : <{1}>\n", channelFDValue, retCode);
                                        /* Closes channel, closes server, cleans up and exits the application. */
                                        CloseChannelCleanUpAndExit(channel, TransportReturnCode.FAILURE, dictionary);
                                    }
                                    break;
                            }
                        }
                    }
                    opConnect = !channel.Socket.Connected;
                }
                catch (Exception e1)
                {
                    Console.WriteLine("Exception: {0}\nStack trace:\n{1}", e1.Message, e1.StackTrace);
                    CloseChannelCleanUpAndExit(channel, TransportReturnCode.FAILURE, dictionary);
                }
            }

            /* Initialize ping management handler */
            InitPingManagement(channel);

            if ((retCode = SendLoginRequest(channel, channelInfo.MaxFragmentSize)) > TransportReturnCode.SUCCESS)
            {
                /* There is still data left to flush, leave our write notification enabled
                 * so we get called again.
                 *
                 * If everything wasn't flushed, it usually indicates that the TCP output
                 * buffer cannot accept more yet
                 */

                /* Flush is done by application */
            }
            else if (retCode < TransportReturnCode.SUCCESS)
            {
                Console.Write("Error ({0}) (errno: {1}) encountered with Init Channel fd={2}. Error Text: {3}\n",
                    error.ErrorId, error.SysError, channelFDValue, error.Text);
                /* Closes channel, cleans up and exits the application. */
                CloseChannelCleanUpAndExit(channel, TransportReturnCode.FAILURE, dictionary);
            }

            /****************************************************
             * Loop 2) Check ping operations until runtime ends *
             ****************************************************/
            /* Here were are using a new Main loop. An alternative design would be to
             * combine this Main loop with the Main loop for getting connection
             * active. Some bookkeeping would be required for that approach.
             */
            /* Add two functions at the end compared to module_1a:InitPingManagement() and
             * ProcessPingManagementHandler() */

            /* Main loop for message processing (reading data, writing data, and ping management, etc.)
             *
             * Currently, the only way to exit this Main loop is when an error condition
             * is triggered or after a predetermined run-time has elapsed.
             */
            while (true)
            {
                try
                {
                    PerformSocketSelect(opWrite: true);

                    /* Check if channel is READ-able */
                    if (readSocketList.Count > 0)
                    {
                        /* Initialize to a positive value for retCode in case we have more data that is available to read */
                        retCode = (TransportReturnCode)(TransportReturnCode.SUCCESS + 1);

                        /******************************************************
                         * Loop 3) Read and decode for all buffers in channel *
                         ******************************************************/
                        while (retCode > TransportReturnCode.SUCCESS)
                        {
                            /* Create read arguments (ReadArgs) to keep track of the statuses of any reads */
                            ReadArgs readArgs = new();

                            /**************************************************
                            * Step 4) Read message from channel into buffer *
                            **************************************************/
                            /*********************************************************
                             * Client/Consumer Application Life Cycle Major Step 4: Read
                             * using Read.
                             *
                             * The examples for executing read and write are shown in
                             * module_1c. Read provides the user with data received from
                             * the connection.
                             *
                             * This function expects the Channel to be in the active
                             * state. When data is available, an Buffer referring to the
                             * information is returned, which is valid until the next call
                             * to Read.
                             *
                             * A return code parameter passed into the function is used to
                             * convey error information as well as communicate whether
                             * there is additional information to read.
                             *********************************************************/

                            msgBuf = channel.Read(readArgs, out error);

                            if (msgBuf != null)
                            {
                                /* if a buffer is returned, we have data to process and code is success */

                                /* Processes a response from the channel/connection. This consists of performing a high
                                 * level decode of the message and then calling the applicable specific function for
                                 * further processing.
                                 */

                                /* No need to clear the message before we decode into it. ETA Decoding populates all
                                 * message members (and that is true for any decoding with ETA, you never need to clear
                                 * anything but the iterator)
                                 */
                                /* We have data to process */
                                /* Create message to represent buffer data */
                                Msg msg = new();

                                /* This ClearDecodeIterator clear iterator function should be used to achieve the best
                                 * performance while clearing the iterator.
                                 */
                                /* Clears members necessary for decoding and readies the iterator for reuse. You must
                                 * clear DecodeIterator before decoding content. For performance purposes, only those
                                 * members required for proper functionality are cleared.
                                 */
                                decodeIter.Clear();
                                /* Set the RWF version to decode with this iterator */
                                CodecReturnCode ret = decodeIter.SetBufferAndRWFVersion(msgBuf, channel.MajorVersion, channel.MinorVersion);
                                /* Associates the DecodeIterator with the Buffer from which to decode. */
                                if (ret != CodecReturnCode.SUCCESS)
                                {
                                    Console.Write("\nSetDecodeIteratorBuffer() failed with return code: {0}\n", ret);
                                    /* Closes channel, closes server, cleans up and exits the application. */
                                    CloseChannelCleanUpAndExit(channel, TransportReturnCode.FAILURE, dictionary);
                                }
                                /* decode contents into the Msg structure */
                                if ((ret = msg.Decode(decodeIter)) != CodecReturnCode.SUCCESS)
                                {
                                    Console.Write("Error ({0}) (errno: {1}) encountered with DecodeMsg.\n",
                                        ret.GetAsInfo(), ret);
                                    /* Closes channel, cleans up and exits the application. */
                                    CloseChannelCleanUpAndExit(channel, TransportReturnCode.FAILURE, dictionary);
                                }
                                /* After acquiring data in buffer, we can check what kind of information it is. It may
                                 * be login message, update or other info. This part will be fulfilled in latter
                                 * examples like module of login and source directory.
                                 */

                                /* Deduce an action based on the domain type of the message */
                                switch (msg.DomainType)
                                {
                                    /* (1) Login Message */
                                    case (int)DomainType.LOGIN:
                                        {
                                            /* (1) Login Message */
                                            /* main difference with last module_1c_read_and_write
                                            * add SendMessage(), SendLoginRequest(),ProcessLoginResponse(), CloseLoginStream()
                                            * and EtaGetBuffer() functions at the end.*/
                                            if (ProcessLoginResponse(msg, decodeIter) != TransportReturnCode.SUCCESS)
                                            {
                                                /* Login Failed and the application is denied - Could be one of the following 3 possibilities:
                                                 *
                                                 * - CLOSED_RECOVER (Stream State): (3) Closed, the applications may attempt to
                                                 *   re-open the stream later (can occur via either an RsslRefreshMsg or an
                                                 *   RsslStatusMsg), OR
                                                 *
                                                 * - CLOSED (Stream State): (4) Closed (indicates that the data is not available
                                                 *   on this service/connection and is not likely to become available), OR
                                                 *
                                                 * - SUSPECT (Data State): (2) Data is Suspect (similar to a stale data state,
                                                 *	 indicates that the health of some or all data associated with the stream is
                                                 *	 out of date or cannot be confirmed that it is current)
                                                 */

                                                /* Closes channel, cleans up and exits the application. */
                                                CloseChannelCleanUpAndExit(channel, TransportReturnCode.FAILURE, dictionary);
                                            }
                                            else
                                            {
                                                Console.Write("ETA Consumer application is granted access and has logged in successfully.\n\n");
                                                serviceDiscoveryInfo_serviceName = serviceName;
                                                /* After it is granted access, those common daily activities and requesting could be done here.
                                                 * But first we need the source directory.*/
                                                /* Send Source Directory request message */

                                                if ((retCode = SendSourceDirectoryRequest(channel, channelInfo.MaxFragmentSize)) > TransportReturnCode.SUCCESS)
                                                {
                                                    /* There is still data left to flush, leave our write notification enabled so
                                                     * we get called again.
                                                     *
                                                     * If everything wasn't flushed, it usually indicates that the TCP output
                                                     * buffer cannot accept more yet
                                                     */
                                                    /* flush is done by application */
                                                }
                                                else if (retCode < TransportReturnCode.SUCCESS)
                                                {
                                                    /* Closes channel, cleans up and exits the application. */
                                                    CloseChannelCleanUpAndExit(channel, TransportReturnCode.FAILURE, dictionary);
                                                }
                                            }
                                        }
                                        break;
                                    case (int)DomainType.SOURCE:
                                        {
                                            if (ProcessSourceDirectoryResponse(channel, msg, decodeIter) != TransportReturnCode.SUCCESS)
                                            {
                                                /* Closes channel, cleans up and exits the application. */
                                                CloseChannelCleanUpAndExit(channel, TransportReturnCode.FAILURE, dictionary);
                                            }
                                            else
                                            {
                                                Console.Write("ETA Consumer application has successfully received source directory information.\n\n");
                                            }

                                            /* exit app if service name entered by user cannot be found */
                                            if (!serviceDiscoveryInfo_serviceNameFound)
                                            {
                                                Console.Write("\nSource directory response does not contain service name: {0}. Exit app.\n", serviceName);
                                                /* Closes channel, cleans up and exits the application. */
                                                CloseChannelCleanUpAndExit(channel, TransportReturnCode.FAILURE, dictionary);
                                            }

                                            /* exit app if service we care about is NOT up and accepting requests */
                                            if ((serviceDiscoveryInfo_ServiceState != Rdm.Directory.ServiceStates.UP) || (serviceDiscoveryInfo_AcceptingRequests == 0))
                                            {
                                                Console.Write("\nService name: {0} is NOT up and accepting requests. Exit app.\n", serviceName);
                                                /* Closes channel, cleans up and exits the application. */
                                                CloseChannelCleanUpAndExit(channel, TransportReturnCode.FAILURE, dictionary);
                                            }
                                            /*
                                             * A consumer application can determine whether to load necessary dictionary
                                             * information from a local file or download the information from the provider if
                                             * available.
                                             *
                                             * - If loading from a file, ETA offers several utility functions to load and manage a
                                             *   properly-formatted field dictionary.
                                             *
                                             * - If downloading information, the application issues a request using the Dictionary
                                             *   domain model. The provider application should respond with a dictionary response,
                                             *   typically broken into a multi-part message. ETA offers several utility functions
                                             *   for encoding and decoding of the Dictionary domain content.
                                             *
                                             * In this simple app, we are trying to first loading necessary dictionary information
                                             * from a local file, if it exists.  For performance considerations, we first load
                                             * field and enumerated dictionaries from local files, if they exist, at the earlier
                                             * stage of the consumer applications.
                                             *
                                             * Otherwise, if loading dictionary information from a local file fails, we download
                                             * the necessary dictionary information from provider if available.
                                             */

                                            /*********************************************************
                                             * The Consumer app should already first tried to load field and enumerated
                                             * dictionaries from local files at the beginning of the app.
                                             *
                                             * Only make Market Price item request after both dictionaries are successfully loaded
                                             * from files.
                                             *
                                             * If at least one of the dictionaries fails to get loaded properly, it will continue
                                             * the code path of downloading the failed loaded dictionary or both dictionaries (if
                                             * neither exists in run-time path or loaded properly) from provider
                                             *********************************************************/

                                            /* Only make Market Price item request after both dictionaries are successfully loaded from files. */
                                            if (dictionariesLoadedInfo_fieldDictionaryLoaded
                                                && dictionariesLoadedInfo_enumTypeDictionaryLoaded)
                                            {
                                                /* check to see if the provider supports the Market Price Domain Type (MARKET_PRICE) */
                                                if (!serviceDiscoveryInfo_etaDMTMarketPriceSupported)
                                                {
                                                    Console.Write("\nDomainTypes.MARKET_PRICE Domain Type is NOT supported by the indicated provider. Exit app.\n");
                                                    /* Closes channel, cleans up and exits the application. */
                                                    CloseChannelCleanUpAndExit(channel, TransportReturnCode.FAILURE, dictionary);
                                                }

                                                /* For this simple example, send just 1 Market Price item request message */

                                                /* Send just 1 Market Price item request message */
                                                if ((retCode = SendMarketPriceItemRequest(channel, channelInfo.MaxFragmentSize)) > TransportReturnCode.SUCCESS)
                                                {
                                                    /* There is still data left to flush, leave our write notification
                                                     * enabled so we get called again.
                                                     *
                                                     * If everything wasn't flushed, it usually indicates that the TCP
                                                     * output buffer cannot accept more yet
                                                     */

                                                    /* flush is done by application */

                                                }
                                                else if (retCode < TransportReturnCode.SUCCESS)
                                                {
                                                    Console.WriteLine("\nDictionary download not supported by the indicated provider");
                                                    CloseChannelCleanUpAndExit(channel, TransportReturnCode.FAILURE, dictionary);
                                                }

                                                break;
                                            }

                                            /*********************************************************
                                             * Need to continue the code path to download at least 1 dictionary from provider
                                             * because at least 1 loading of either field and enumerated dictionaries from local
                                             * files failed
                                             *
                                             * If downloading information, the application issues a request using the Dictionary
                                             * domain model. The provider application should respond with a dictionary response,
                                             * typically broken into a multi-part message. ETA offers several utility functions
                                             * for encoding and decoding of the Dictionary domain content.
                                             *
                                             * Only make Market Price item request after both dictionaries are successfully
                                             * downloaded from provider or loaded from file already (in that case, would not
                                             * download for that dictionary).
                                             *********************************************************/

                                            /* clear the DataDictionary dictionary before first use/load
                                             */
                                            dictionary.Clear();

                                            /* Will attempt to download the Field Dictionary (RDMFieldDictionary) from provider. */
                                            if (!dictionariesLoadedInfo_fieldDictionaryLoaded)
                                            {
                                                /* check if Dictionary Domain Type is supported */
                                                if (!serviceDiscoveryInfo_etalDMTDictionarySupported)
                                                {
                                                    Console.Write("\nDictionary Domain Type is NOT supported. Exit app.\n");
                                                    /* Closes channel, cleans up and exits the application. */
                                                    CloseChannelCleanUpAndExit(channel, TransportReturnCode.FAILURE, dictionary);
                                                }

                                                /* check if RDMFieldDictionary is available for downloading */
                                                if (!serviceDiscoveryInfo_RDMFieldDictionaryProvided)
                                                {
                                                    Console.Write("\nRDMFieldDictionary is NOT available for downloading. Exit app.\n");
                                                    /* Closes channel, cleans up and exits the application. */
                                                    CloseChannelCleanUpAndExit(channel, TransportReturnCode.FAILURE, dictionary);
                                                }

                                                /* Send RDMFieldDictionary Dictionary request message */
                                                if ((retCode = SendDictionaryRequest(channel, dictionary, channelInfo.MaxFragmentSize, dictionaryDownloadName)) > TransportReturnCode.SUCCESS)
                                                {
                                                    /* There is still data left to flush, leave our write notification enabled so
                                                     * we get called again.
                                                     *
                                                     * If everything wasn't flushed, it usually indicates that the TCP output
                                                     * buffer cannot accept more yet
                                                     */

                                                    /* flush is done by application */
                                                }
                                                else if (retCode < TransportReturnCode.SUCCESS)
                                                {
                                                    /* Closes channel, cleans up and exits the application. */
                                                    CloseChannelCleanUpAndExit(channel, TransportReturnCode.FAILURE, dictionary);
                                                }
                                            }

                                            /* Will attempt to download Enumerated Types Dictionaries (enumtype.def) from provider. */
                                            if (!dictionariesLoadedInfo_enumTypeDictionaryLoaded)
                                            {
                                                /* check if Dictionary Domain Type is supported */
                                                if (!serviceDiscoveryInfo_etalDMTDictionarySupported)
                                                {
                                                    Console.Write("\nDictionary Domain Type is NOT supported. Exit app.\n");
                                                    /* Closes channel, cleans up and exits the application. */
                                                    CloseChannelCleanUpAndExit(channel, TransportReturnCode.FAILURE, dictionary);
                                                }

                                                /* check if enumtype.def is available for downloading */
                                                if (!serviceDiscoveryInfo_enumtypeProvided)
                                                {
                                                    Console.Write("\nenumtype.def is NOT available for downloading. Exit app.\n");
                                                    /* Closes channel, cleans up and exits the application. */
                                                    CloseChannelCleanUpAndExit(channel, TransportReturnCode.FAILURE, dictionary);
                                                }

                                                /* Send enumtype.def Dictionary request message */
                                                if ((retCode = SendDictionaryRequest(channel, dictionary, channelInfo.MaxFragmentSize, enumTableDownloadName)) > TransportReturnCode.SUCCESS)
                                                {
                                                    /* There is still data left to flush, leave our write notification enabled so
                                                     * we get called again.
                                                     *
                                                     * If everything wasn't flushed, it usually indicates that the TCP output
                                                     * buffer cannot accept more yet
                                                     */

                                                    /* flush is done by application */
                                                }
                                                else if (retCode < TransportReturnCode.SUCCESS)
                                                {
                                                    /* Closes channel, cleans up and exits the application. */
                                                    CloseChannelCleanUpAndExit(channel, TransportReturnCode.FAILURE, dictionary);
                                                }
                                            }
                                        }
                                        break;
                                    case (int)DomainType.DICTIONARY:
                                        {

                                            if ((retCode = ProcessDictionaryResponse(channel, msg, decodeIter, dictionary)) != TransportReturnCode.SUCCESS)
                                            {
                                                /* Closes channel, cleans up and exits the application. */
                                                CloseChannelCleanUpAndExit(channel, TransportReturnCode.FAILURE, dictionary);
                                            }
                                            else
                                            {
                                                /* Only make Market Price item request after both dictionaries are successfully downloaded from the provider. */
                                                if (dictionariesLoadedInfo_fieldDictionaryLoaded
                                                    && dictionariesLoadedInfo_enumTypeDictionaryLoaded)
                                                {
                                                    dictionariesLoadedInfo_isInitialized = true;
                                                    Console.Write("ETA Consumer application has successfully downloaded both dictionaries or successfully downloaded 1 dictionary if the other dictionary has already been loaded from (local) file successfully.\n\n");

                                                    /* check to see if the provider supports the Market Price Domain Type (MARKET_PRICE) */
                                                    if (!serviceDiscoveryInfo_etaDMTMarketPriceSupported)
                                                    {
                                                        Console.Write("\nDomainTypes.MARKET_PRICE Domain Type is NOT supported by the indicated provider. Exit app.\n");
                                                        /* Closes channel, cleans up and exits the application. */
                                                        CloseChannelCleanUpAndExit(channel, TransportReturnCode.FAILURE, dictionary);
                                                    }

                                                    /* For this simple example, send just 1 Market Price item request message */
                                                    if ((retCode = SendMarketPriceItemRequest(channel, channelInfo.MaxFragmentSize)) > TransportReturnCode.SUCCESS)
                                                    {
                                                        /* There is still data left to flush, leave our write notification enabled
                                                         * so we get called again.
                                                         *
                                                         * If everything wasn't flushed, it usually indicates that the TCP output
                                                         * buffer cannot accept more yet
                                                         */

                                                        /* flush is done by application */
                                                    }
                                                    else if (retCode < TransportReturnCode.SUCCESS)
                                                    {
                                                        CloseChannelCleanUpAndExit(channel, TransportReturnCode.FAILURE, dictionary);
                                                    }

                                                }
                                            }
                                        }
                                        break;
                                    case (int)DomainType.MARKET_PRICE:
                                        {
                                            if (ProcessMarketPriceItemResponse(channel, msg, decodeIter, dictionary) != TransportReturnCode.SUCCESS)
                                            {
                                                /* Closes channel, cleans up and exits the application. */
                                                CloseChannelCleanUpAndExit(channel, TransportReturnCode.FAILURE, dictionary);
                                            }
                                            else
                                            {
                                                Console.Write("\nETA Consumer application has successfully received Market Price item response.\n\n");
                                            }

                                        }
                                        break;
                                    default: /* Error handling */
                                        {
                                            Console.Write("Unhandled Domain Type: {0}\n", msg.DomainType);
                                        }
                                        break;
                                }

                                /* Acknowledge that a ping has been received */
                                receivedServerMsg = true;
                                Console.Write("Ping message has been received successfully from the server due to data message ...\n\n");
                            }
                            else
                            {
                                /* Deduce an action from the return code of Channel.Read() */
                                retCode = readArgs.ReadRetVal;
                                switch (retCode)
                                {
                                    /* Acknowledge that a ping has been received */
                                    case TransportReturnCode.READ_PING:
                                        /* Update ping monitor */
                                        /* set flag for server message received */
                                        receivedServerMsg = true;
                                        Console.Write("Ping message has been received successfully from the server due to ping message ...\n\n");
                                        break;

                                    /* Switch to a new channel if required */
                                    case TransportReturnCode.READ_FD_CHANGE:
                                        /* READ_FD_CHANGE indicates that a socketId change has occurred as a result of this call.
                                        *  New SocketFDValue need to be set to update current FD.
                                        *  Shifting from the old socket to the new one handled by the library itself.
                                        */

                                        channelFDValue = inProgInfo.NewSocket.Handle.ToInt64();
                                        break;

                                    case TransportReturnCode.READ_WOULD_BLOCK:
                                        /* Nothing to read */
                                        break;
                                    case TransportReturnCode.READ_IN_PROGRESS:
                                        /* Reading from multiple threads */
                                        break;
                                    case TransportReturnCode.INIT_NOT_INITIALIZED:
                                    case TransportReturnCode.FAILURE:
                                        Console.Write("Error ({0}) (errno: {1}) {1}\n", error.ErrorId, error.SysError, error.Text);
                                        CloseChannelCleanUpAndExit(channel, TransportReturnCode.FAILURE, dictionary);
                                        break;
                                    default: /* Error handling */

                                        if (retCode < 0)
                                        {
                                            Console.Write("Error ({0}) (errno: {1}) encountered with Read. Error Text: {2}\n",
                                                error.ErrorId, error.SysError, error.Text);
                                            /* Closes channel/connection, cleans up and exits the application. */
                                            CloseChannelCleanUpAndExit(channel, TransportReturnCode.FAILURE, dictionary);
                                        }
                                        break;
                                }
                            }
                        }
                    }

                    /* Check if channel is WRITE-able */
                    if (writeSocketList.Count > 0)
                    {
                        retCode = TransportReturnCode.FAILURE;

                        if ((retCode = channel.Flush(out error)) >= TransportReturnCode.SUCCESS)
                        {
                            /* retval == TransportReturnCode.SUCCESS - All was flushed. */
                            /* retval == TransportReturnCode.SUCCESS - There is still data left to flush.
                             * If everything wasn't flushed, it usually indicates that the TCP output buffer cannot accept more yet
                             */
                        }
                        else
                        {
                            switch (retCode)
                            {
                                case TransportReturnCode.SUCCESS:
                                    {
                                        /* Everything has been flushed, no data is left to send */

                                        Console.Write("All data has been successfuly flushed...\n");
                                    }
                                    break;
                                case TransportReturnCode.FAILURE:
                                default:
                                    Console.Write("Error ({0}) (errno: {1}) encountered with Init Channel fd={2}. Error Text: {3}\n",
                                                  error.ErrorId, error.SysError, channelFDValue, error.Text);
                                    CloseChannelCleanUpAndExit(channel, TransportReturnCode.FAILURE, dictionary);
                                    break;
                            }
                        }
                    }

                    /* Processing ping management handler */
                    if ((retCode = ProcessPingManagementHandler(channel)) >= TransportReturnCode.SUCCESS)
                    {
                        /* If everything wasn't flushed, it usually indicates that the TCP output buffer cannot accept more yet
                         *
                         * Flush is done by application */
                    }
                    else if (retCode < TransportReturnCode.SUCCESS)
                    {
                        /* Closes channel, cleans up and exits the application. */
                        CloseChannelCleanUpAndExit(channel, TransportReturnCode.FAILURE, dictionary);
                    }

                    /* get current time */
                    currentTime = System.DateTime.Now;

                    /* Handles the run-time for the ETA Consumer application. Here we exit the application after a predetermined time to run */
                    /* If the runtime has expired */
                    if (currentTime >= etaRuntime)
                    {
                        /* Closes all streams for the consumer after run-time has elapsed. */

                        if ((retCode = CloseMarketPriceItemStream(channel, channelInfo.MaxFragmentSize)) != TransportReturnCode.SUCCESS) /* (retval > _RET_SUCCESS) or (retval < _RET_SUCCESS) */
                        {
                            /* When you close Market Price item, we want to make a best effort to get this across the network as
                             * it will gracefully close the open Market Price item stream. If this cannot be flushed or failed,
                             * this application will just close the connection for simplicity.
                             */

                            /* Closes channel, cleans up and exits the application. */
                            CloseChannelCleanUpAndExit(channel, TransportReturnCode.FAILURE, dictionary);
                        }

                        /* Close Login stream */
                        /* Note that closing Login stream will automatically close all other streams at the provider */
                        if ((retCode = CloseLoginStream(channel, channelInfo.MaxFragmentSize)) < TransportReturnCode.SUCCESS)
                        {
                            /* When you close login, we want to make a best effort to get this across the network as it
                             * will gracefully close all open streams. If this cannot be flushed or failed, this
                             * application will just close the connection for simplicity.
                             */

                            /* Closes channel, cleans up and exits the application. */
                            CloseChannelCleanUpAndExit(channel, TransportReturnCode.FAILURE, dictionary);
                        }
                        else if (retCode > TransportReturnCode.SUCCESS)
                        {
                            /* Flush before exiting */
                            if (writeSocketList.Count > 0)
                            {
                                retCode = (TransportReturnCode)1;

                                /******************************************************
                                 * Loop 4) Flush all remaining buffers to channel *
                                 ******************************************************/
                                while (retCode > TransportReturnCode.SUCCESS)
                                {
                                    retCode = channel.Flush(out error);
                                }
                                if (retCode < TransportReturnCode.SUCCESS)
                                {
                                    Console.Write("Flush has failed with return code {0} - <{1}>\n", retCode, error.Text);
                                }
                            }
                        }
                        Console.Write("ETA Client run-time has expired...\n");
                        CloseChannelCleanUpAndExit(channel, TransportReturnCode.SUCCESS, dictionary);
                    }
                }
                catch (Exception e1)
                {
                    Console.WriteLine("Exception: {0}\nStack trace:\n{1}", e1.Message, e1.StackTrace);
                    CloseChannelCleanUpAndExit(channel, TransportReturnCode.FAILURE, dictionary);
                }
            }
        }

        private static void PerformSocketSelect(int timeOutUs = 1000 * 1000, bool opWrite = false)
        {
            readSocketList.Clear();
            writeSocketList.Clear();

            if (channel != null)
                readSocketList.Add(channel.Socket);
            if (channel != null && opWrite)
                writeSocketList.Add(channel.Socket);

            Socket.Select(readSocketList, writeSocketList, null, timeOutUs);
        }

        /// <summary>
        /// Closes channel and exits application.
        /// </summary>
        ///
        /// <param name="channel">Channel to be closed</param>
        /// <param name="code">if exit is due to errors/exceptions</param>
        public static void CloseChannelCleanUpAndExit(IChannel channel, TransportReturnCode code, DataDictionary dictionary)
        {
            bool isClosedAndClean = true;
            Error error = new();
            /*********************************************************
             * Client/Consumer Application Life Cycle Major Step 5: Close connection
             * using channel Close (OS connection release handshake) channel Close
             * closes the client based Channel. This will release any pool based
             * resources back to their respective pools, close the connection, and
             * perform any additional necessary cleanup.
             *********************************************************/

            if ((channel != null))
            {
                isClosedAndClean = channel.Close(out error) >= TransportReturnCode.SUCCESS;
            }

            /* when users are done, they should unload dictionaries to clean up memory */
            if (dictionary == null)
                Console.Write("\nNULL Dictionary pointer.\n");

            else if (!dictionariesLoadedInfo_isInitialized)
                Console.Write("\nNo need to delete dictionary - dictionary was not loaded yet.\n");

            else
                dictionary.Clear();


            /*********************************************************
             * Client/Consumer Application Life Cycle Major Step 6: Uninitialize ETA
             * Transport using Uninitialize The last ETA Transport function that an
             * application should call. This uninitializes internal data structures
             * and deletes any allocated memory.
             *********************************************************/

            /*
             * All ETA Transport use is complete, must uninitialize. The
             * uninitialization process allows for any heap allocated memory to be
             * cleaned up properly.
             */
            Transport.Uninitialize();

            if (isClosedAndClean)
            {
                Console.WriteLine("Consumer application has closed channel and has cleaned up successfully.");
            }
            else
            {
                Console.Write("Error ({0}) (errno: {1}): {2}\n", error.ErrorId, error.SysError, error.Text);
            }

            if (code == TransportReturnCode.SUCCESS)
            {
                Console.Write("\nETA Consumer Training Application successfully ended.\n");
            }

            Environment.Exit(0);
        }

        static TimeSpan pingTimeoutServer; /* server ping timeout */
        static TimeSpan pingTimeoutClient; /* client ping timeout */
        static System.DateTime nextReceivePingTime; /* time client should receive next message/ping from server */
        static System.DateTime nextSendPingTime; /* time to send next ping from client */
        static bool receivedServerMsg; /* flag for server message received */

        /// <summary>
        /// Initializes the ping times for etaChannel.
        /// </summary>
        ///
        /// <param name="channel">The channel for ping management info initialization</param>
        public static void InitPingManagement(IChannel channel)
        {
            /* get current time */
            System.DateTime currentTime = System.DateTime.Now;

            /* set ping timeout for local and remote pings */
            pingTimeoutClient = TimeSpan.FromSeconds(channel.PingTimeOut / 3);
            pingTimeoutServer = TimeSpan.FromSeconds(channel.PingTimeOut);

            /* set time to send next ping to remote connection */
            nextSendPingTime = currentTime + pingTimeoutClient;

            /* set time should receive next ping from remote connection */
            nextReceivePingTime = currentTime + pingTimeoutServer;
        }

        /// <summary>
        /// Processing ping management handler.
        /// </summary>
        ///
        /// <param name="channel">The channel for ping management processing</param>
        public static TransportReturnCode ProcessPingManagementHandler(IChannel channel)
        {
            /* Handles the ping processing for etaChannel.
             *
             * Sends a ping to the server if the next send ping time has arrived and
             * checks if a ping has been received from the server within the next receive
             * ping time.
             */
            TransportReturnCode retval = TransportReturnCode.SUCCESS;
            Error error = new();

            /* get current time */
            System.DateTime currentTime = System.DateTime.Now;

            /* handle client pings */
            if (currentTime >= nextSendPingTime)
            {
                /* send ping to server */
                /*********************************************************
                 * Client/Consumer Application Life Cycle Major Step 4: Ping using
                 * channel.Ping
                 *
                 * Attempts to write a heartbeat message on the connection. This function
                 * expects the Channel to be in the active state.
                 *
                 * If an application calls the Ping function while there are other bytes
                 * queued for output, the ETA Transport layer will suppress the heartbeat
                 * message and attempt to flush bytes to the network on the user's behalf.
                 *********************************************************/

                /* Ping use - this demonstrates sending of heartbeats */
                if ((retval = channel.Ping(out error)) > TransportReturnCode.SUCCESS)
                {
                    /* Indicates that queued data was sent as a heartbeat and there is still information
                     * internally queued by the transport.
                     *
                     * The Flush function must be called to continue attempting to pass the queued bytes to
                     * the connection. This information may still be queued because there is not sufficient
                     * space in the connections output buffer.
                     *
                     * An Select request can be used to indicate when the socketId has write availability.

                    /* Flush needs to be done by application */
                }
                else
                {
                    switch (retval)
                    {
                        case TransportReturnCode.SUCCESS:
                            {
                                /* Ping message has been sent successfully */
                                Console.Write("Ping message has been sent successfully to the server ...\n\n");
                            }
                            break;
                        case TransportReturnCode.FAILURE: /* fall through to default. */
                        default: /* Error handling */
                            {
                                Console.Write("Error ({0}) (errno: {1}) encountered with Ping(). Error Text:{2}\n",
                                    error.ErrorId, error.SysError, error.Text);
                                /* Closes channel/connection, cleans up and exits the application. */
                                return TransportReturnCode.FAILURE;
                            }
                    }
                }

                /* set time to send next ping from client */
                nextSendPingTime = currentTime + pingTimeoutClient;
            }

            /* handle server pings - an application should determine if data or pings have
             * been received, if not application should determine if pingTimeout has
             * elapsed, and if so connection should be closed
             */
            if (currentTime >= nextReceivePingTime)
            {
                /* Check if received message from remote (connection) since last time */
                if (receivedServerMsg)
                {
                    /* Reset flag for remote message received */
                    receivedServerMsg = false;

                    /* Set time should receive next message/ping from remote (connection)  */
                    nextReceivePingTime = currentTime + (pingTimeoutServer * 1000);
                }
                else /* lost contact with server */
                {
                    error ??= new Error();
                    /* Lost contact with remote (connection) */
                    error.Text = "Lost contact with connection...\n";
                    Console.Write("Error ({0}) (errno: {1}) {2}\n", error.ErrorId, error.SysError, error.Text);
                    return TransportReturnCode.FAILURE;
                }
            }
            return retval;
        }

        /// <summary>
        /// Sends a message buffer to the channel
        /// </summary>
        ///
        /// <param name="channel">the Channel to send the message buffer to</param>
        /// <param name="msgBuf">the buffer to be sent</param>
        ///
        /// <returns>status code</returns>
        public static TransportReturnCode SendMessage(IChannel channel, ITransportBuffer msgBuf)
        {
            Error error;
            TransportReturnCode retCode;
            WriteArgs writeArgs = new();

            writeArgs.Flags = WriteFlags.NO_FLAGS;
            /* send the request */

            /*********************************************************
             * Client/Consumer Application Life Cycle Major Step 4: Write using Writer
             *
             * rsslWriter performs any writing or queuing of data. This function expects
             * the channel to be in the active state and the buffer to be properly
             * populated, where length reflects the actual number of bytes used.
             *
             * This function allows for several modifications to be specified for this
             * call. Here we use WriteFlags.NO_FLAGS. For more information on other flag
             * enumeration such as WriteFlags.DO_NOT_COMPRESS or
             * WriteFlags.DIRECT_SOCKET_WRITE, see the ETA C developers guide for
             * rsslWrite Flag Enumeration Values supported by ETA Transport.
             *
             * The ETA Transport also supports writing data at different priority levels.
             *
             * The application can pass in two integer values used for reporting
             * information about the number of bytes that will be written.
             *
             * The uncompressedBytesWritten parameter will return the number of bytes to
             * be written, including any transport header overhead but not taking into
             * account any compression.
             *
             * The bytesWritten parameter will return the number of bytes to be written,
             * including any transport header overhead and taking into account any
             * compression.
             *
             * If compression is disabled, uncompressedBytesWritten and bytesWritten
             * should match.
             *
             * The number of bytes saved through the compression process can be calculated
             * by (bytesWritten - uncompressedBytesWritten).
             *
             * Note: Before passing a buffer to Write, it is required that the application
             * set length to the number of bytes actually used. This ensures that only the
             * required bytes are written to the network.
             *********************************************************/

            /* Now write the data - keep track of ETA Transport return code -
             *
             * Because positive values indicate bytes left to write, some negative
             * transport layer return codes still indicate success
             */

            /* this example writes buffer as high priority and no write modification flags */
            if ((retCode = channel.Write(msgBuf, writeArgs, out error)) == TransportReturnCode.WRITE_CALL_AGAIN)
            {
                /* (-10) Transport Success: channel.write is fragmenting the buffer and needs to be
                 * called again with the same buffer. This indicates that rsslWrite was unable to
                 * send all fragments with the current call and must continue fragmenting
                 */

                /* Large buffer is being split by transport, but out of output buffers. Schedule a
                 * call to channel.Flush() and then call the channel.write function again with this
                 * same exact buffer to continue the fragmentation process. Only release the buffer
                 * if not passing it to rsslWrite again. */

                /* call flush and write again - breaking out if the return code is something other
                 * than TransportReturnCodes.WRITE_CALL_AGAIN (write call again) */
                while (retCode == TransportReturnCode.WRITE_CALL_AGAIN)
                {
                    if ((retCode = channel.Flush(out error)) < TransportReturnCode.SUCCESS)
                    {
                        Console.Write("Flush has failed with return code {0} - <{1}>\n", retCode, error.Text);
                    }
                    retCode = channel.Write(msgBuf, writeArgs, out error);
                }

            }

            /* set write fd if there's still data queued */
            if (retCode > TransportReturnCode.SUCCESS)
            {
                /* The write was successful and there is more data queued in ETA Transport. The
                 * channel.Flush() method should be used to continue attempting to flush data to the
                 * connection. ETA will release buffer.
                 */

                /* Flush needs to be done by application */
            }
            else
            {
                switch (retCode)
                {
                    case TransportReturnCode.SUCCESS:
                        {
                            /* Successful write and all data has been passed to the connection */
                            /* Continue with next operations. ETA will release buffer.*/
                        }
                        break;
                    case TransportReturnCode.NO_BUFFERS:
                        {
                            channel.ReleaseBuffer(msgBuf, out error);
                        }
                        break;
                    case TransportReturnCode.WRITE_FLUSH_FAILED:
                        {
                            if (channel.State == ChannelState.CLOSED)
                            {
                                /* Channel is Closed - This is terminal. Treat as error, and buffer
                                 * must be released - fall through to default. */
                            }
                            else
                            {
                                /* Channel.Write() internally attempted to flush data to the
                                 * connection but was blocked. This is not a failure and the user
                                 * should not release their buffer. */

                                /* Successful write call, data is queued. The channel.Flush() method
                                 * should be used to continue attempting to flush data to the
                                 * connection. */

                                /* Set write fd if flush failed */
                                /* Flush needs to be done by application */

                                /* Channel is still open, but channel.Write() tried to flush internally and failed.
                                 * Return positive value so the caller knows there's bytes to flush.
                                 */

                                return (TransportReturnCode)1;
                            }
                            break;
                        }
                    case TransportReturnCode.FAILURE:
                    default:
                        {
                            Console.Write("Error ({0}) (errno: {1}) encountered with Init Channel fd={2}. Error Text: {3}\n",
                                error.ErrorId, error.SysError, channelFDValue, error.Text);
                            channel.ReleaseBuffer(msgBuf, out error);
                            return TransportReturnCode.FAILURE;
                        }
                }
            }

            return retCode;
        }

        public const int LOGIN_STREAM_ID = 1;

        /// <summary>
        /// Sends login request message to a channel
        /// </summary>
        ///
        /// <param name="channel">the Channel of connection</param>
        /// <param name="maxFragmentSize">maxFragmentSize in channelInfo</param>
        ///
        /// <returns>status code</returns>
        public static TransportReturnCode SendLoginRequest(IChannel channel, int maxFragmentSize)
        {
            CodecReturnCode retCode;
            Error error = new();
            ITransportBuffer? msgBuf;

            /* Populate and encode a requestMsg */
            IRequestMsg reqMsg = new Msg();

            /* ETA provides clear functions for its structures (e.g., encodeIterator.Clear()).
             *
             * These functions are tuned to be efficient and avoid initializing unnecessary
             * structure members, and allow for optimal structure use and reuse.
             */
            /* Iterator used for encoding throughout the application - we can clear it and reuse it instead of recreating it */
            EncodeIterator encIter = new();

            ElementList elementList = new();
            ElementEntry elementEntry = new();

            Buffer applicationId = new();
            Buffer applicationName = new();
            UInt applicationRole = new();

            elementList.Clear();
            elementEntry.Clear();

            string userName = "put user name here";
            Buffer userNameBuf = new();

            /* Get a buffer of the channel max fragment size */

            /* Obtains a non-packable buffer of the requested size from the ETA Transport guaranteed
             * buffer pool to write into for the Login request.
             *
             * For an outbound TransportBuffer (specifically one gotten from Channel.GetBuffer(),
             * like this one) length() initially does indeed indicate the number of bytes available,
             * but when the ByteBuffer's position is after the start (because the application has
             * written something, i.e. encoded some or all of a Login request message), it actually
             * returns the number of bytes between the start & current positions (basically, the
             * number of bytes encoded thus far).
             */
            /* EtaGetBuffer() is the utility function that does 2-pass (more robust) getting non-packable buffer. */
            if ((msgBuf = EtaGetBuffer(channel, maxFragmentSize, error)) == null)
            {
                /* Connection should be closed, return failure */
                /* Closes channel, cleans up and exits the application. */
                return TransportReturnCode.FAILURE;
            }
            /* if a buffer is returned, we can populate and write, encode an RsslMsg into the buffer */

            /* Encodes the Login request. */

            reqMsg.Clear();

            /* clear encode iterator for reuse - this should be used to achieve the best performance while clearing the iterator. */
            encIter.Clear();
            /* set version information of the connection on the encode iterator so proper versioning can be performed */
            /* set the buffer on an RsslEncodeIterator */
            if ((retCode = encIter.SetBufferAndRWFVersion(msgBuf, channel.MajorVersion, channel.MinorVersion)) < (CodecReturnCode)TransportReturnCode.SUCCESS)
            {
                channel.ReleaseBuffer(msgBuf, out error);
                Console.Write("\nrsslSetEncodeIteratorBuffer() failed for Login Request with return code: {0}\n", retCode);
                /* Closes channel, cleans up and exits the application. */
                return TransportReturnCode.FAILURE;
            }
            /* Create and initialize request message */
            reqMsg.MsgClass = MsgClasses.REQUEST; /* (1) Request Message */
            reqMsg.StreamId = LOGIN_STREAM_ID; /* StreamId */

            reqMsg.DomainType = (int)DomainType.LOGIN; /* (1) Login Message */
            reqMsg.ContainerType = DataTypes.NO_DATA; /* (128) No Data */
            /* The initial Login request must be streaming (i.e., a RequestMsgFlags.STREAMING flag is required). */
            reqMsg.Flags = RequestMsgFlags.STREAMING;
            /* set msgKey members */
            reqMsg.MsgKey.Flags = MsgKeyFlags.HAS_ATTRIB | MsgKeyFlags.HAS_NAME_TYPE | MsgKeyFlags.HAS_NAME;

            /* Username */
            userNameBuf.Data(userName);

            if ((userName = Environment.UserName) != null)
            {
                reqMsg.MsgKey.Name.Data(userName);
            }
            else
            {
                reqMsg.MsgKey.Name.Data("Unknown");
            }

            reqMsg.MsgKey.NameType = (int)Login.UserIdTypes.NAME; /* (1) Name */
            /* (133) Element List container type, used to represent content containing element name, dataType, and value triples.    */
            reqMsg.MsgKey.AttribContainerType = DataTypes.ELEMENT_LIST;

            /* encode message */

            /* since our msgKey has opaque that we want to encode, we need to use encode() */
            /* reqMsg.Encode() should return and inform us to encode our key opaque */
            if ((retCode = reqMsg.EncodeInit(encIter, 0)) < CodecReturnCode.SUCCESS)
            {
                return TransportReturnCode.FAILURE;
            }
            /* encode our msgKey opaque */

            /* encode the element list */
            elementList.Clear();

            elementList.Flags = (ElementListFlags.HAS_STANDARD_DATA);/* (0x08) The elementlist contains standard encoded content (e.g. not set defined).  */

            /* Begins encoding of an ElementList. */

            if ((retCode = elementList.EncodeInit(encIter, null, 0)) < CodecReturnCode.SUCCESS)
            {
                return TransportReturnCode.FAILURE;
            }

            /* ApplicationId */
            applicationId.Data("256");

            elementEntry.DataType = DataTypes.ASCII_STRING;
            elementEntry.Name = ElementNames.APPID;

            if ((retCode = elementEntry.Encode(encIter, applicationId)) < CodecReturnCode.SUCCESS)
            {
                channel.ReleaseBuffer(msgBuf, out error);
                Console.Write("elementEntry.Encode() failed for Login Request with return code: {0}\n", retCode);
                /* Closes channel, cleans up and exits the application. */
                return TransportReturnCode.FAILURE;
            }
            /* ApplicationName */
            applicationName.Data("ETA Consumer Training");

            elementEntry.DataType = DataTypes.ASCII_STRING;
            elementEntry.Name = ElementNames.APPNAME;

            if ((retCode = elementEntry.Encode(encIter, applicationName)) < CodecReturnCode.SUCCESS)
            {
                channel.ReleaseBuffer(msgBuf, out error);
                Console.Write("elementEntry.Encode() failed for Login Request with return code: {0}\n", retCode);
                /* Closes channel, cleans up and exits the application. */
                return TransportReturnCode.FAILURE;
            }

            /* Role */
            elementEntry.DataType = DataTypes.UINT;
            elementEntry.Name = ElementNames.ROLE;

            /* (1) Application logs in as a consumer */
            applicationRole.Value(Login.RoleTypes.CONS);

            if ((retCode = elementEntry.Encode(encIter, applicationRole)) < CodecReturnCode.SUCCESS)
            {
                channel.ReleaseBuffer(msgBuf, out error);
                Console.Write("elementEntry.Encode() failed for Login Request with return code: {0}\n", retCode);
                /* Closes channel, cleans up and exits the application. */
                return TransportReturnCode.FAILURE;
            }

            /* complete encode element list */
            if ((retCode = elementList.EncodeComplete(encIter, true)) < CodecReturnCode.SUCCESS)
            {
                channel.ReleaseBuffer(msgBuf, out error);
                Console.Write("elementList.EncodeComplete() failed for Login Request with return code: {0}\n", retCode);
                /* Closes channel, cleans up and exits the application. */
                return TransportReturnCode.FAILURE;
            }
            /* complete encode key */
            /* EncodeMsgKeyAttribComplete finishes our key opaque, so it should return and indicate
             * for us to encode our container/msg payload
             */
            if ((retCode = reqMsg.EncodeKeyAttribComplete(encIter, true)) < CodecReturnCode.SUCCESS)
            {
                channel.ReleaseBuffer(msgBuf, out error);
                Console.Write("reqMsg.EncodeKeyAttribComplete() failed for Login Request with return code: {0}\n", retCode);
                /* Closes channel, cleans up and exits the application. */
                return TransportReturnCode.FAILURE;
            }
            /* complete encode message */
            if ((retCode = reqMsg.EncodeComplete(encIter, true)) < CodecReturnCode.SUCCESS)
            {
                channel.ReleaseBuffer(msgBuf, out error);
                Console.Write("reqMsg.EncodeComplete() failed for Login Request with return code: {0}\n", retCode);
                /* Closes channel, cleans up and exits the application. */
                return TransportReturnCode.FAILURE;
            }

            /* send login request */
            if ((TransportReturnCode)(retCode = (CodecReturnCode)SendMessage(channel, msgBuf)) < TransportReturnCode.SUCCESS)
            {
                return TransportReturnCode.FAILURE;
            }
            else if ((TransportReturnCode)retCode > TransportReturnCode.SUCCESS)
            {
                /* There is still data left to flush, leave our write notification enabled so we get
                 * called again.
                 *
                 * If everything wasn't flushed, it usually indicates that the TCP output buffer
                 * cannot accept more yet
                 */

                /* Flush needs to be done by application */
            }

            return (TransportReturnCode)retCode;
        }


        /// <summary>
        /// Processes a login response
        /// </summary>
        ///
        /// <param name="msg">the partially decoded message</param>
        /// <param name="decIter">the decode iterator</param>
        /// <returns>status code</returns>
        public static TransportReturnCode ProcessLoginResponse(Msg msg, DecodeIterator decIter)
        {
            CodecReturnCode retCode;
            State pState;

            /* Switch cases depending on message class */
            switch (msg.MsgClass)
            {
                case MsgClasses.REFRESH:
                    {
                        IMsgKey? key;

                        ElementList elementList = new();
                        ElementEntry elementEntry = new();

                        /* check stream id */
                        Console.WriteLine("\nReceived Login Refresh Msg with Stream Id {0}\n", msg.StreamId);

                        /* check if it's a solicited refresh */
                        if ((msg.Flags & RefreshMsgFlags.SOLICITED) != 0)
                        {
                            Console.WriteLine("\nThe refresh msg is a solicited refresh (sent as a response to a request).\n");
                        }
                        else
                        {
                            Console.WriteLine("A refresh sent to inform a consumer of an upstream change in information (i.e., an unsolicited refresh).\n");
                        }

                        /* get key */
                        key = msg.MsgKey;

                        /* decode key opaque data */
                        if ((retCode = msg.DecodeKeyAttrib(decIter, key)) != CodecReturnCode.SUCCESS)
                        {
                            return TransportReturnCode.FAILURE;
                        }

                        /* decode element list */
                        if ((retCode = elementList.Decode(decIter, null)) == CodecReturnCode.SUCCESS)
                        {
                            /* decode each element entry in list */
                            while ((retCode = elementEntry.Decode(decIter)) != CodecReturnCode.END_OF_CONTAINER)
                            {
                                if (retCode == CodecReturnCode.SUCCESS)
                                {
                                    /* get login response information */
                                    /* ApplicationId */
                                    if (elementEntry.Name.Equals(ElementNames.APPID))
                                    {
                                        Console.Write("\tReceived Login Response for ApplicationId: {0}\n",
                                            elementEntry.EncodedData.ToString());
                                    }
                                    /* ApplicationName */
                                    else if (elementEntry.Name.Equals(ElementNames.APPNAME))
                                    {
                                        Console.Write("\tReceived Login Response for ApplicationName: {0}\n",
                                            elementEntry.EncodedData.ToString());
                                    }
                                    /* Position */
                                    else if (elementEntry.Name.Equals(ElementNames.POSITION))
                                    {
                                        Console.Write("\tReceived Login Response for Position: {0}\n",
                                            elementEntry.EncodedData.ToString());
                                    }
                                }
                                else
                                {
                                    return TransportReturnCode.FAILURE;
                                }
                            }
                        }
                        else
                        {
                            return TransportReturnCode.FAILURE;
                        }
                        /* get Username */
                        if (key != null)
                        {
                            Console.Write("\nReceived Login Response for Username: {0}\n", key.Name.ToString());
                        }
                        else
                        {
                            Console.Write("\nReceived Login Response for Username: Unknown\n");
                        }

                        /* get state information */
                        pState = ((IRefreshMsg)msg).State;
                        Console.Write("{0}\n\n", pState.ToString());

                        /* check if login okay and is solicited */
                        if (((msg.Flags & RefreshMsgFlags.SOLICITED) != 0)
                            && pState.StreamState() == StreamStates.OPEN
                            && pState.DataState() == DataStates.OK)
                        {
                            Console.Write("Login stream is OK and solicited\n");
                        }
                        else /* handle error cases */
                        {
                            if (pState.StreamState() == StreamStates.CLOSED_RECOVER)
                            {
                                /* Stream State is (3) Closed, the applications may attempt to re-open the
                                 * stream later (can occur via either an RsslRefreshMsg or an StatusMsg) */
                                Console.Write("Login stream is closed recover\n");
                                return TransportReturnCode.FAILURE;
                            }
                            else if (pState.StreamState() == StreamStates.CLOSED)
                            {
                                /* Stream State is (4) Closed (indicates that the data is not available on
                                 * this service/connection and is not likely to become available) */
                                Console.Write("Login attempt failed (stream closed)\n");
                                return TransportReturnCode.FAILURE;
                            }
                            else if (pState.StreamState() == StreamStates.OPEN
                                && pState.DataState() == DataStates.SUSPECT)
                            {
                                /* Stream State is (1) Open (typically implies that information will be
                                 * streaming, as information changes updated information will be sent on the
                                 * stream, after final RsslRefreshMsg or RsslStatusMsg)
                                 *
                                 * Data State is (2) Data is Suspect (similar to a stale data state, indicates
                                 * that the health of some or all data associated with the stream is out of
                                 * date or cannot be confirmed that it is current )
                                 */
                                Console.Write("Login stream is suspect\n");
                                return TransportReturnCode.FAILURE;
                            }
                        }
                    }
                    break;
                case MsgClasses.STATUS:/* (3) Status Message */
                    {
                        Console.Write("Received Login StatusMsg\n");
                        if ((msg.Flags & StatusMsgFlags.HAS_STATE) != 0)
                        {
                            /* get state information */
                            pState = ((IStatusMsg)msg).State;
                            Console.Write("{0}\n\n", pState.ToString());

                            /* handle error cases */
                            if (pState.StreamState() == StreamStates.CLOSED_RECOVER)
                            {
                                /* Stream State is (3) Closed, the applications may attempt to re-open the
                                 * stream later (can occur via either an RefreshMsg or an StatusMsg) */
                                Console.Write("Login stream is closed recover\n");
                                return TransportReturnCode.FAILURE;
                            }
                            else if (pState.StreamState() == StreamStates.CLOSED)
                            {
                                /* Stream State is (4) Closed (indicates that the data is not available on
                                 * this service/connection and is not likely to become available) */
                                Console.Write("Login attempt failed (stream closed)\n");
                                return TransportReturnCode.FAILURE;
                            }
                            else if (pState.StreamState() == StreamStates.OPEN && pState.DataState() == DataStates.SUSPECT)
                            {
                                /* Stream State is (1) Open (typically implies that information will be
                                 * streaming, as information changes updated information will be sent on the
                                 * stream, after final RefreshMsg or StatusMsg)
                                 *
                                 * Data State is (2) Data is Suspect (similar to a stale data state, indicates
                                 * that the health of some or all data associated with the stream is out of
                                 * date or cannot be confirmed that it is current )
                                 */
                                Console.Write("Login stream is suspect\n");
                                return TransportReturnCode.FAILURE;
                            }
                        }
                    }
                    break;
                case MsgClasses.UPDATE:/* (4) Update Message */
                    {
                        Console.WriteLine("Received Login Update\n");
                    }
                    break;
                default:/* Error handling */
                    {
                        Console.Write("Received Unhandled Login Msg Class: {0}\n", msg.MsgClass);
                        return TransportReturnCode.FAILURE;
                    }
            }

            return TransportReturnCode.SUCCESS;
        }


        /// <summary>
        /// Close the login stream
        /// </summary>
        ///
        /// <param name="channel">the Channel of connection</param>
        /// <param name="maxFragmentSize">the maximum fragment size before fragmentation</param>
        /// <returns>status code</returns>
        public static TransportReturnCode CloseLoginStream(IChannel channel, int maxFragmentSize)
        {
            CodecReturnCode retCode;
            Error error = new();
            ITransportBuffer? msgBuf;

            /* Consumer uses CloseMsg to indicate no further interest in an item stream
             * and to close the stream. */
            ICloseMsg closeMsg = new Msg();

            /* Get a buffer of the channel max fragment size */
            /* ETA provides clear functions for its structures (e.g.,
             * encodeIterator.Clear()).
             *
             * These functions are tuned to be efficient and avoid initializing
             * unnecessary structure members, and allow for optimal structure use and
             * reuse.
             */
            /* Iterator used for encoding throughout the application - we can clear it and reuse it instead of recreating it */
            EncodeIterator encIter = new();

            /* Obtains a non-packable buffer of the requested size from the ETA Transport
             * guaranteed buffer pool to write into for the Login close.
             *
             * For an outbound TransportBuffer (specifically one gotten from
             * Channel.GetBuffer(), like this one) length() initially does indeed indicate
             * the number of bytes available, but when the ByteBuffer's position is after
             * the start (because the application has written something, i.e. encoded some
             * or all of a Login request message), it actually returns the number of bytes
             * between the start & current positions (basically, the number of bytes
             * encoded thus far).
             */
            /* EtaGetBuffer() is the utility function that does 2-pass (more robust) getting non-packable buffer. */
            if ((msgBuf = EtaGetBuffer(channel, maxFragmentSize, error)) == null)
            {
                return TransportReturnCode.FAILURE;
            }

            /* if a buffer is returned, we can populate and write, encode a Msg into the buffer */

            /* Encodes the Login close. */

            closeMsg.Clear();
            /* clear encode iterator for reuse - this should be used to achieve the best
             * performance while clearing the iterator. */
            encIter.Clear();

            /* set version information of the connection on the encode iterator so proper
             * versioning can be performed */
            /* set the buffer on an EncodeIterator */
            if ((retCode = encIter.SetBufferAndRWFVersion(msgBuf, channel.MajorVersion, channel.MinorVersion)) < (CodecReturnCode)TransportReturnCode.SUCCESS)
            {
                channel.ReleaseBuffer(msgBuf, out error);
                Console.Write("\nSetEncodeIteratorBuffer() failed with return code: {0}\n", retCode);
                /* Closes channel, cleans up and exits the application. */
                return TransportReturnCode.FAILURE;
            }

            /* Create and initialize close message */
            closeMsg.MsgClass = MsgClasses.CLOSE; /* (5) Close Message */
            closeMsg.StreamId = LOGIN_STREAM_ID;
            closeMsg.DomainType = (int)DomainType.LOGIN; /* (1) Login Message */
            /* No payload associated with this close message */
            closeMsg.ContainerType = DataTypes.NO_DATA; /* (128) No Data */
            /* encode message */

            /* Since there is no payload, no need for Init/Complete as everything is in the msg header */
            /* Functions without a suffix of Init or Complete (e.g. EncodeMsg) perform
             * encoding within a single call, typically used for encoding simple types
             * like Integer or incorporating previously encoded data (referred to as
             * pre-encoded data).
             */

            if ((retCode = closeMsg.Encode(encIter)) < CodecReturnCode.SUCCESS)
            {
                channel.ReleaseBuffer(msgBuf, out error);
                Console.Write("Msg.Encode() failed with return code: {0}\n", retCode);
                return TransportReturnCode.FAILURE;
            }
            /* send login close */
            if ((TransportReturnCode)(retCode = (CodecReturnCode)SendMessage(channel, msgBuf)) < TransportReturnCode.SUCCESS)
            {
                /* login close fails */
                /* Closes channel, cleans up and exits the application. */
                return TransportReturnCode.FAILURE;
            }
            else if (retCode > (CodecReturnCode)TransportReturnCode.SUCCESS)
            {
                /* There is still data left to flush, leave our write notification enabled
                 * so we get called again.
                 *
                 * If everything wasn't flushed, it usually indicates that the TCP output
                 * buffer cannot accept more yet
                 */

                /* Flush needs to be done by application */
            }

            return (TransportReturnCode)retCode;
        }


        /// <summary>
        /// Performs two time pass to obtain buffer
        /// </summary>
        ///
        /// <param name="channel">the Channel of connection</param>
        /// <param name="size">size of requested buffer</param>
        /// <param name="error">tracks error info</param>
        /// <returns>obtained buffer</returns>
        public static ITransportBuffer? EtaGetBuffer(IChannel channel, int size, Error error)
        {
            TransportReturnCode retCode;

            ITransportBuffer? msgBuf;

            /* Retrieve a Buffer for use. */
            /* First check error */
            if ((msgBuf = channel.GetBuffer(size, false, out error)) == null)
            {
                if (error.ErrorId != TransportReturnCode.NO_BUFFERS)
                {
                    /* Check to see if this is just out of buffers or if it's unrecoverable */

                    /* it's unrecoverable Error */
                    Console.Write("Error ({0}) (errno: {1}) encountered with Channel.GetBuffer. Error Text: {2}\n",
                        error.ErrorId, error.SysError, error.Text);
                    /* Connection should be closed, return failure */
                    /* Closes channel, cleans up and exits the application. */

                    return null;
                }

                /* (-4) Transport Failure: There are no buffers available from the buffer
                 * pool, returned from GetBuffer.
                 *
                 * Use Ioctl to increase pool size or use Flush to flush data and return
                 * buffers to pool.
                 */

                /* The Flush function could be used to attempt to free buffers back to the pool */

                /* Flush and obtain buffer again */
                retCode = channel.Flush(out error);
                if (retCode < TransportReturnCode.SUCCESS)
                {
                    Console.Write("Channel.Flush() failed with return code {0} - <{1}>\n", retCode, error.Text);
                    /* Closes channel, cleans up and exits the application. */
                    return null;
                }

                if ((msgBuf = channel.GetBuffer(size, false, out error)) == null)
                {
                    Console.Write("Error ({0}) (errno: {1}) encountered with Channel.GetBuffer. Error Text: {2}\n",
                        error.ErrorId, error.SysError, error.Text);
                    return null;
                }

            }
            /* return  buffer to be filled in with valid memory */
            return msgBuf;
        }

        public const int SRCDIR_STREAM_ID = 2;

        public static TransportReturnCode SendSourceDirectoryRequest(IChannel channel, int maxMsgSize)
        {
            CodecReturnCode ret;
            Error error = new();
            ITransportBuffer? msgBuf;

            EncodeIterator encIter = new();

            IRequestMsg requestMsg = new Msg();

            /* get a buffer for the source directory request */
            if ((msgBuf = EtaGetBuffer(channel, maxMsgSize, error)) == null)
            {
                /* Connection should be closed, return failure */
                /* Closes channel, cleans up and exits the application. */

                return TransportReturnCode.FAILURE;
            }

            requestMsg.Clear();

            encIter.Clear();
            if ((ret = encIter.SetBufferAndRWFVersion(msgBuf, channel.MajorVersion, channel.MinorVersion)) < (CodecReturnCode)TransportReturnCode.SUCCESS)
            {
                channel.ReleaseBuffer(msgBuf, out error);
                Console.Write("\nSetEncodeIteratorBuffer() failed with return code: {0}\n", ret);
                /* Closes channel, cleans up and exits the application. */
                return TransportReturnCode.FAILURE;
            }

            /*encode source directory request*/
            requestMsg.MsgClass = MsgClasses.REQUEST;
            requestMsg.StreamId = SRCDIR_STREAM_ID;
            requestMsg.DomainType = (int)DomainType.SOURCE;
            requestMsg.ContainerType = DataTypes.NO_DATA;
            /* RequestMsgFlags.STREAMING flag set. When the flag is set, the request is
            * known as a "streaming" request, meaning that the refresh will be followed by
            * updates.
            */
            requestMsg.Flags = RequestMsgFlags.STREAMING | RequestMsgFlags.HAS_PRIORITY;

            /* A consumer can request information about all services by omitting serviceId
             * information
             *
             * If the consumer wishes to receive information about all services, the
             * consumer should not specify a serviceId (i.e., the consumer should not set
             * MsgKeyFlags.HAS_SERVICE_ID).
             */

            /* set members in msgKey */
            /* (0x0008) This MsgKey has a filter, contained in MsgKey.Filter.  */
            requestMsg.MsgKey.Flags = MsgKeyFlags.HAS_FILTER;

            /* Because the Source Directory domain uses an FilterList, a consumer can
             * indicate the specific source related information in which it is interested
             * via a msgKey.filter. Each bit-value represented in the filter corresponds
             * to an information set that can be provided in response messages.
             *
             * LSEG recommends that a consumer application minimally request Info,
             * State, and Group filters for the
             *
             * Source Directory:
             *
             * - The Info filter contains the service name and serviceId data for all
             *   available services. When an appropriate service is discovered by the OMM
             *   Consumer, the serviceId associated with the service is used on subsequent
             *   requests to that service.
             *
             * - The State filter contains status data for the service. Such data informs
             *   the Consumer whether the service is up (and available) or down (and
             *   unavailable).
             *
             * - The Group filter conveys any item group status information, including
             *   group states and as regards the merging of groups if applicable.
             */
            requestMsg.MsgKey.Filter = Rdm.Directory.ServiceFilterFlags.INFO
                | Rdm.Directory.ServiceFilterFlags.STATE
                | Rdm.Directory.ServiceFilterFlags.GROUP;

            ret = requestMsg.Encode(encIter);
            if (ret != CodecReturnCode.SUCCESS)
            {
                error.Text = "EncodeDirectoryRequest(): Failed <" + ret.GetAsString() + ">";
                return (TransportReturnCode)ret;
            }

            /* send source directory request */
            if ((ret = (CodecReturnCode)SendMessage(channel, msgBuf)) < (CodecReturnCode)TransportReturnCode.SUCCESS)
            {
                /* Closes channel, cleans up and exits the application. */
                return TransportReturnCode.FAILURE;
            }
            else if (ret > (CodecReturnCode)TransportReturnCode.SUCCESS)
            {
                /* There is still data left to flush, leave our write notification enabled
                 * so we get called again.
                 *
                 * If everything wasn't flushed, it usually indicates that the TCP output
                 * buffer cannot accept more yet
                 */

                /* flush needs to be done by application */
            }
            return (TransportReturnCode)ret;
        }

        public static TransportReturnCode ProcessSourceDirectoryResponse(IChannel chnl, Msg msg, DecodeIterator dIter)
        {
            CodecReturnCode retval;
            State pState;

            Buffer tempBuffer = new();

            Map map = new();
            MapEntry mapEntry = new();
            FilterList filterList = new();

            FilterEntry filterEntry = new();
            ElementEntry elementEntry = new();
            ElementList elementList = new();
            Array array = new();
            ArrayEntry arrayEntry = new();

            int arrayCount = 0;

            int serviceCount = 0;
            /* this keeps track of which service we are actually interested in, that is,
             * when we run into the serviceName requested by the application
             */
            int foundServiceIndex = -1;

            string serviceName = "DIRECT_FEED";
            /* create primitive value to have key decoded into */
            UInt serviceId = new();

            UInt capabilities = new();

            string dictionariesProvided;

            Qos qoS = new();
            List<Qos> QosBuf = new List<Qos>();
            bool foundQoS = false;

            /* The ServiceState and AcceptingRequests elements in the State filter entry
             * work together to indicate the ability of a particular service to provide data:
             */
            UInt serviceState = new();
            UInt acceptingRequests = new();

            State serviceStatus = new();

            IRefreshMsg refreshMsg = (IRefreshMsg)msg;
            IStatusMsg statusMsg = (IStatusMsg)msg;

            /* Switch cases depending on message class */
            switch (msg.MsgClass)
            {
                case MsgClasses.REFRESH:
                case MsgClasses.UPDATE:
                    {

                        /* decode source directory refresh or update */

                        if (msg.MsgClass == MsgClasses.REFRESH)
                            Console.Write("\nReceived Source Directory Refresh\n");
                        else /* (4) Update Message */
                        {
                            Console.Write("\nReceived Source Directory Update\n");

                            /* When displaying update information, we should also display the updateType information. */
                            Console.Write("UPDATE TYPE: {0}\n", UpdateEventTypes.ToString(((IUpdateMsg)msg).UpdateType));
                        }

                        /* decode contents into the map structure */
                        if ((retval = map.Decode(dIter)) < CodecReturnCode.SUCCESS)
                        {
                            /* decoding failure tends to be unrecoverable */
                            Console.Write("Error ({0}) (errno: {1}) encountered with DecodeMap.",
                                retval.GetAsInfo(), retval);
                            return (TransportReturnCode)retval;
                        }

                        /* The primitive type associated with each MapEntry key. */
                        /* source directory refresh or update key data type must be DataTypes.UINT */
                        if (map.KeyPrimitiveType != DataTypes.UINT)
                        {
                            Console.Write("Map has incorrect keyPrimitiveType");
                            return TransportReturnCode.FAILURE;
                        }

                        /* if summary data is present, invoking decoder for that type (instead of DecodeEntry)
                         * indicates to ETA that user wants to decode summary data
                         */
                        if ((map.Flags & MapFlags.HAS_SUMMARY_DATA) != 0)
                        {
                            /* Map summary data is present. Its type should be that of Map.containerType */
                            Console.Write("summary data is present. Its type should be that of Map.containerType\n");
                            /* Continue decoding ... */
                        }

                        while ((retval = mapEntry.Decode(dIter, serviceId)) != CodecReturnCode.END_OF_CONTAINER)
                        {
                            /* break out of decoding when predetermined max services (15) reached */
                            if (serviceCount == 15)
                            {
                                /* The decoding process typically runs until the end of each container,
                                 * indicated by CodecReturnCodes.END_OF_CONTAINER.
                                 *
                                 * This FinishDecodeEntries() function sets the application to skip remaining
                                 * entries in a container and continue the decoding process. This function
                                 * will skip past remaining entries in the container and perform necessary
                                 * synchronization between the content and iterator so that decoding can
                                 * continue.
                                 */
                                dIter.FinishDecodeEntries();
                                Console.Write("processSourceDirectoryResponse() maxServices limit reached - more services in message than memory can support\n");
                                break;
                            }

                            if (retval != CodecReturnCode.SUCCESS && retval != CodecReturnCode.BLANK_DATA)
                            {
                                /* decoding failure tends to be unrecoverable */
                                Console.Write("Error ({0}) (errno: {1}) encountered with DecodeMapEntry.",
                                    retval.GetAsInfo(), retval);
                                return (TransportReturnCode)retval;
                            }
                            else
                            {
                                if (msg.MsgClass == MsgClasses.REFRESH)
                                    Console.Write("\nReceived Source Directory Refresh for Decoded Service Id: {0}",
                                        serviceId.ToLong());
                                else /* (4) Update Message */
                                    Console.Write("\nReceived Source Directory Update for Decoded Service Id: {0}",
                                        serviceId.ToLong());

                                /* decode contents into the filter list structure */
                                if ((retval = filterList.Decode(dIter)) < CodecReturnCode.SUCCESS)
                                {
                                    /* decoding failure tends to be unrecoverable */
                                    Console.Write("Error ({0}) (errno: {1}) encountered with DecodeFilterList.",
                                        retval.GetAsInfo(), retval);
                                    return (TransportReturnCode)retval;
                                }

                                /* decode each filter entry until there are no more left.
                                 * Decodes an FilterEntry. This function expects the same DecodeIterator that was used with DecodeFilterList.
                                 * This populates encData with an encoded entry.
                                 */
                                while ((retval = filterEntry.Decode(dIter)) != CodecReturnCode.END_OF_CONTAINER)
                                {
                                    if (retval < CodecReturnCode.SUCCESS)
                                    {
                                        Console.Write("Error ({0}) (errno: {1}) encountered with DecodeFilterEntry.",
                                            retval.GetAsInfo(), retval);
                                        return (TransportReturnCode)retval;
                                    }

                                    else
                                    {
                                        /* Show how to use ContainerType.
                                         *
                                         * After DecodeFilterEntry function returns, the FilterList.containerType (or
                                         * FilterEntry.containerType if present) can invoke the correct contained type's
                                         * decode functions.
                                         *
                                         * If filterEntry.containerType is present, switch on that; otherwise switch on
                                         * filterList.containerType
                                         */
                                        int cType;
                                        if ((filterEntry.Flags & FilterEntryFlags.HAS_CONTAINER_TYPE) != 0)
                                            cType = filterEntry.ContainerType;
                                        else
                                            cType = filterList.ContainerType;
                                        switch (cType)
                                        {
                                            /* (137) Map container type, used to represent primitive type key - container type paired entries.   */
                                            case DataTypes.MAP:
                                                {
                                                    /* Continue decoding map entries. call DecodeMap function
                                                     * Write("DecodeFilterEntry: Continue decoding map entries.\n");
                                                     */
                                                }
                                                break;
                                            /* (133) Element List container type, used to represent content containing
                                             * element name, dataType, and value triples.  */
                                            case DataTypes.ELEMENT_LIST:
                                                {
                                                    /* For Source Directory response, we actually know it is DataTypes.ELEMENT_LIST up-front
                                                     * See code below in the next switch block: we are calling DecodeElementList to
                                                     * continue decoding element entries
                                                     */

                                                    /* Continue decoding element entries. call DecodeElementList function
                                                     * Write("DecodeFilterEntry: Continue decoding element entries.\n");
                                                     */
                                                }
                                                break;
                                            default: /* Error handling */
                                                {
                                                    Console.Write("\nUnkonwn ContainerType: {0}\n", cType);
                                                    return TransportReturnCode.FAILURE;
                                                }
                                        }
                                        /* decode source directory response information */
                                        switch (filterEntry.Id)
                                        {
                                            case Rdm.Directory.ServiceFilterIds.INFO: /* (1) Service Info Filter ID */
                                                {
                                                    Console.Write("\nDecoding Service Info Filter ID FilterListEntry\n");

                                                    /* decode element list - third parameter is 0 because we do not have set definitions in this example */
                                                    if ((retval = elementList.Decode(dIter, null)) < CodecReturnCode.SUCCESS)
                                                    {
                                                        /* decoding failure tends to be unrecoverable */
                                                        Console.Write("Error ({0}) (errno: {1}) encountered with DecodeElementList.",
                                                            retval.GetAsInfo(), retval);
                                                        return (TransportReturnCode)retval;
                                                    }
                                                    /* decode element list elements */
                                                    while ((retval = elementEntry.Decode(dIter)) != CodecReturnCode.END_OF_CONTAINER)
                                                    {
                                                        if (retval < CodecReturnCode.SUCCESS)
                                                        {
                                                            /* decoding failure tends to be unrecoverable */
                                                            Console.Write("Error ({0}) (errno: {1}) encountered with DecodeElementEntry.",
                                                                retval.GetAsInfo(), retval);
                                                            return (TransportReturnCode)retval;
                                                        }
                                                        else
                                                        {
                                                            /* get service general information */

                                                            /* Name */
                                                            if (elementEntry.Name.Equals(ElementNames.NAME))
                                                            {
                                                                if (msg.MsgClass == MsgClasses.REFRESH)
                                                                    Console.Write("\tReceived Source Directory Refresh for ServiceName: {0}\n",
                                                                        elementEntry.EncodedData.ToString());
                                                                else /* (4) Update Message */
                                                                    Console.Write("\tReceived Source Directory Update for ServiceName: {0}\n",
                                                                        elementEntry.EncodedData.ToString());

                                                                /* When an appropriate service is discovered by the OMM consumer,
                                                                 * it uses the serviceId associated with the service on all
                                                                 * subsequent requests to that service. Check if service name
                                                                 * received in response matches that entered by user if it does,
                                                                 * store the service id
                                                                 */
                                                                serviceName = elementEntry.EncodedData.ToString();

                                                                if (serviceName.Equals(serviceDiscoveryInfo_serviceName))
                                                                {
                                                                    /* serviceName requested by the application is FOUND */
                                                                    foundServiceIndex = serviceCount;

                                                                    Console.Write("\tService name: {0} ({1}) is discovered by the OMM consumer. \n",
                                                                        serviceName, serviceId.ToLong());
                                                                    serviceId.Copy(serviceDiscoveryInfo_serviceId);
                                                                    serviceDiscoveryInfo_serviceNameFound = true;
                                                                }
                                                            }
                                                            /* Capabilities */
                                                            else if (elementEntry.Name.Equals(ElementNames.CAPABILITIES))
                                                            {
                                                                /* decode into the array structure header */
                                                                if ((retval = array.Decode(dIter)) < CodecReturnCode.SUCCESS)
                                                                {
                                                                    /* decoding failure tends to be unrecoverable */
                                                                    Console.Write("Error {0} ({1}) encountered with DecodeArray.\n",
                                                                        retval.GetAsInfo(), retval);
                                                                    return (TransportReturnCode)retval;
                                                                }
                                                                while ((retval = arrayEntry.Decode(dIter)) != CodecReturnCode.END_OF_CONTAINER)
                                                                {
                                                                    /* break out of decoding array items when predetermined max capabilities (10) reached */
                                                                    if (arrayCount == 10)
                                                                    {
                                                                        /* The decoding process typically runs until the end of each
                                                                         * container, indicated by CodecReturnCodes.END_OF_CONTAINER.
                                                                         *
                                                                         * This FinishDecodeEntries function sets the application to skip
                                                                         * remaining entries in a container and continue the decoding
                                                                         * process. This function will skip past remaining entries in the
                                                                         * container and perform necessary synchronization between the
                                                                         * content and iterator so that decoding can continue.
                                                                         */
                                                                        dIter.FinishDecodeEntries();
                                                                        break;
                                                                    }

                                                                    if (retval == CodecReturnCode.SUCCESS)
                                                                    {
                                                                        retval = capabilities.Decode(dIter);
                                                                        if (retval != CodecReturnCode.SUCCESS && retval != CodecReturnCode.BLANK_DATA)
                                                                        {
                                                                            Console.Write("Error {0} ({1}) encountered with DecodeUInt().\n",
                                                                                retval.GetAsInfo(), retval);
                                                                            return (TransportReturnCode)retval;
                                                                        }
                                                                        if (msg.MsgClass == MsgClasses.REFRESH)
                                                                            Console.Write("\tReceived Source Directory Refresh for Decoded Capabilities[{0}]: {1}\n",
                                                                                arrayCount, capabilities.ToLong());
                                                                        else /* (4) Update Message */
                                                                            Console.Write("\tReceived Source Directory Update for Decoded Capabilities[{0}]: {1}\n",
                                                                                arrayCount, capabilities.ToLong());

                                                                        /* if advertising Dictionary domain type is supported */
                                                                        if ((DomainType)capabilities.ToLong() == DomainType.DICTIONARY)
                                                                        {
                                                                            Console.Write("\tDICTIONARY domain type is supported.\n");
                                                                            serviceDiscoveryInfo_etalDMTDictionarySupported = true;
                                                                        }

                                                                        /* if advertising MarketPrice domain type is supported */
                                                                        if ((DomainType)capabilities.ToLong() == DomainType.MARKET_PRICE)
                                                                        {
                                                                            Console.Write("\tMARKET_PRICE domain type is supported.\n");
                                                                            serviceDiscoveryInfo_etaDMTMarketPriceSupported = true;
                                                                        }

                                                                    }
                                                                    else if (retval != CodecReturnCode.BLANK_DATA)
                                                                    {
                                                                        /* decoding failure tends to be unrecoverable */
                                                                        Console.Write("Error {0} ({1}) encountered with DecodeArrayEntry.\n",
                                                                            retval.GetAsInfo(), retval);
                                                                        return (TransportReturnCode)retval;
                                                                    }
                                                                    arrayCount++;
                                                                }
                                                                arrayCount = 0;
                                                            }
                                                            /* DictionariesProvided */
                                                            else if (elementEntry.Name.Equals(ElementNames.DICTIONARIES_PROVIDED))
                                                            {
                                                                /* decode into the array structure header */
                                                                if ((retval = array.Decode(dIter)) < CodecReturnCode.SUCCESS)
                                                                {
                                                                    /* decoding failure tends to be unrecoverable */
                                                                    Console.Write("Error {0} ({1}) encountered with DecodeArray.\n",
                                                                        retval.GetAsInfo(), retval);
                                                                    return (TransportReturnCode)retval;
                                                                }
                                                                while ((retval = arrayEntry.Decode(dIter)) != CodecReturnCode.END_OF_CONTAINER)
                                                                {
                                                                    /* break out of decoding array items when predetermined max dictionaries (5) reached */
                                                                    if (arrayCount == 5)
                                                                    {
                                                                        /* The decoding process typically runs until the end of each
                                                                         * container, indicated by CodecReturnCodes.END_OF_CONTAINER.
                                                                         *
                                                                         * This FinishDecodeEntries function sets the application to skip
                                                                         * remaining entries in a container and continue the decoding
                                                                         * process. This function will skip past remaining entries in the
                                                                         * container and perform necessary synchronization between the
                                                                         * content and iterator so that decoding can continue.
                                                                         */
                                                                        dIter.FinishDecodeEntries();
                                                                        break;
                                                                    }

                                                                    if (retval == CodecReturnCode.SUCCESS)
                                                                    {
                                                                        if (msg.MsgClass == MsgClasses.REFRESH)
                                                                            Console.Write("\tReceived Source Directory Refresh for DictionariesProvided[{0}]: {1}\n",
                                                                                arrayCount, arrayEntry.EncodedData.ToString());
                                                                        else /* (4) Update Message */
                                                                            Console.Write("\tReceived Source Directory Update for DictionariesProvided[{0}]: {1}\n",
                                                                                arrayCount, arrayEntry.EncodedData.ToString());

                                                                        /* DictionariesProvided provide the dictionaries that are available for downloading */
                                                                        /* Our training ETA Consumer app only cares about RDMFieldDictionary and enumtype.def */
                                                                        dictionariesProvided = arrayEntry.EncodedData.ToString();

                                                                        if (dictionariesProvided.Equals(dictionaryDownloadName))
                                                                        {
                                                                            /* dictionary RDMFieldDictionary is available for downloading */
                                                                            Console.Write("\tDictionary Provided: {0} with filename: {1} \n",
                                                                                dictionariesProvided, fieldDictionaryFileName);
                                                                            serviceDiscoveryInfo_RDMFieldDictionaryProvided = true;
                                                                        }
                                                                        else if (dictionariesProvided.Equals(enumTableDownloadName))
                                                                        {
                                                                            /* dictionary enumtype.def is available for downloading */
                                                                            Console.Write("\tDictionary Provided: {0} with filename: {1} \n",
                                                                                enumTableDownloadName, enumTypeDictionaryFileName);
                                                                            serviceDiscoveryInfo_enumtypeProvided = true;
                                                                        }
                                                                    }
                                                                    else if (retval != CodecReturnCode.BLANK_DATA)
                                                                    {
                                                                        /* decoding failure tends to be unrecoverable */
                                                                        Console.Write("Error {0} ({1}) encountered with DecodeArrayEntry.\n",
                                                                            retval.GetAsInfo(), retval);
                                                                        return (TransportReturnCode)retval;
                                                                    }
                                                                    arrayCount++;
                                                                }
                                                                arrayCount = 0;
                                                            }

                                                            /* QoS */
                                                            else if (elementEntry.Name.Equals(ElementNames.QOS))
                                                            {
                                                                foundQoS = true;
                                                                /* decode into the array structure header */
                                                                if ((retval = array.Decode(dIter)) < CodecReturnCode.SUCCESS)
                                                                {
                                                                    /* decoding failure tends to be unrecoverable */
                                                                    Console.Write("Error {0} ({1}) encountered with DecodeArray.\n",
                                                                        retval.GetAsInfo(), retval);
                                                                    return (TransportReturnCode)retval;
                                                                }
                                                                while ((retval = arrayEntry.Decode(dIter)) != CodecReturnCode.END_OF_CONTAINER)
                                                                {
                                                                    /* break out of decoding array items when predetermined max QOS (5) reached */
                                                                    if (arrayCount == 5)
                                                                    {
                                                                        /* The decoding process typically runs until the end of each
                                                                         * container, indicated by CodecReturnCodes.END_OF_CONTAINER.
                                                                         *
                                                                         * This FinishDecodeEntries function sets the application to skip
                                                                         * remaining entries in a container and continue the decoding
                                                                         * process. This function will skip past remaining entries in the
                                                                         * container and perform necessary synchronization between the
                                                                         * content and iterator so that decoding can continue.
                                                                         */
                                                                        dIter.FinishDecodeEntries();
                                                                        break;
                                                                    }

                                                                    if (retval == CodecReturnCode.SUCCESS)
                                                                    {
                                                                        /* Obtain QoS information such as data timeliness (e.g. real time) and rate (e.g. tick-by-tick). */
                                                                        retval = qoS.Decode(dIter);
                                                                        if (retval != CodecReturnCode.SUCCESS && retval != CodecReturnCode.BLANK_DATA)
                                                                        {
                                                                            Console.Write("DecodeQos() failed with return code: {0}\n", retval);
                                                                            return (TransportReturnCode)retval;
                                                                        }
                                                                        else
                                                                        {

                                                                            QosBuf.Add(qoS);
                                                                            Console.Write("\tReceived {0}\n", QosBuf[0]);
                                                                        }
                                                                    }
                                                                    else if (retval != CodecReturnCode.BLANK_DATA)
                                                                    {
                                                                        /* decoding failure tends to be unrecoverable */
                                                                        Console.Write("Error {0} ({1}) encountered with DecodeArrayEntry.\n",
                                                                            retval.GetAsInfo(), retval);
                                                                        return (TransportReturnCode)retval;
                                                                    }
                                                                    arrayCount++;
                                                                }
                                                                arrayCount = 0;

                                                                /* if this is the serviceName that is requested by the application */
                                                                if (serviceCount == foundServiceIndex)
                                                                {
                                                                    /* Need to store the Source Directory QoS information */
                                                                    serviceDiscoveryInfo_qoS.Add(qoS);
                                                                }
                                                            }
                                                        }
                                                    }

                                                    /* if QoS was not send in the directory refresh message set it to the default values */
                                                    if (!foundQoS)
                                                    {
                                                        Console.Write("\tNot Received Source Directory Refresh for QoS\n");
                                                        Console.Write("\tSet default QoS: Realtime/TickByTick/Static\n");

                                                        for (arrayCount = 0; arrayCount < 5; arrayCount++)
                                                        {
                                                            serviceDiscoveryInfo_qoS[arrayCount].Timeliness(QosTimeliness.REALTIME);
                                                            serviceDiscoveryInfo_qoS[arrayCount].Rate(QosRates.TICK_BY_TICK);
                                                            serviceDiscoveryInfo_qoS[arrayCount].IsDynamic = false;
                                                        }
                                                    }
                                                }
                                                break;
                                            case Rdm.Directory.ServiceFilterIds.STATE: /* (2) Source State Filter ID */
                                                {
                                                    Console.Write("\nDecoding Source State Filter ID FilterListEntry\n");

                                                    /* decode element list - third parameter is 0 because we do not have set definitions in this example */
                                                    if ((retval = elementList.Decode(dIter, null)) < CodecReturnCode.SUCCESS)
                                                    {
                                                        /* decoding failure tends to be unrecoverable */
                                                        Console.Write("Error {0} ({1}) encountered with DecodeElementList.\n",
                                                            retval.GetAsInfo(), retval);
                                                        return (TransportReturnCode)retval;
                                                    }

                                                    /* decode element list elements */
                                                    while ((retval = elementEntry.Decode(dIter)) != CodecReturnCode.END_OF_CONTAINER)
                                                    {
                                                        if (retval < CodecReturnCode.SUCCESS)
                                                        {
                                                            /* decoding failure tends to be unrecoverable */
                                                            Console.Write("Error {0} ({1}) encountered with DecodeElementEntry.\n",
                                                                retval.GetAsInfo(), retval);
                                                            return (TransportReturnCode)retval;
                                                        }
                                                        else
                                                        {
                                                            /* get service state information */

                                                            /* The ServiceState and AcceptingRequests elements in the State filter entry
                                                             * work together to indicate the ability of a particular service to provide
                                                             * data:
                                                             *
                                                             * - ServiceState indicates whether the source of the data is accepting
                                                             *   requests.
                                                             *
                                                             * - AcceptingRequests indicates whether the immediate upstream provider (the
                                                             *   provider to which the consumer is directly connected) can accept new
                                                             *   requests and/or process reissue requests on already open streams.
                                                             */

                                                            /* for our training app, for the service we are intersted in to be considered
                                                             * really up, both ServiceState has to be Up (1), and AcceptingRequests has to
                                                             * be Yes (1). That way, New requests and reissue requests can be successfully
                                                             * processed.
                                                             */

                                                            /* ServiceState - Up(1), Down (0) */
                                                            /* RDM_DIRECTORY_SERVICE_STATE_DOWN -  (0) Service state down */
                                                            /* RDM_DIRECTORY_SERVICE_STATE_UP -    (1) Service state up */
                                                            if (elementEntry.Name.Equals(ElementNames.SVC_STATE))
                                                            {

                                                                retval = serviceState.Decode(dIter);
                                                                if (retval != CodecReturnCode.SUCCESS && retval != CodecReturnCode.BLANK_DATA)
                                                                {
                                                                    Console.Write("Error {0} ({1}) encountered with State.Decode().\n",
                                                                        retval.GetAsInfo(), retval);
                                                                    return (TransportReturnCode)retval;
                                                                }
                                                                if (msg.MsgClass == MsgClasses.REFRESH)
                                                                    Console.Write("\tReceived Source Directory Refresh for Decoded ServiceState: {0}\n", serviceState.ToLong());
                                                                else /* (4) Update Message */
                                                                    Console.Write("\tReceived Source Directory Update for Decoded ServiceState: {0}\n", serviceState.ToLong());

                                                                /* if this is the serviceName that is requested by the application */
                                                                if (serviceCount == foundServiceIndex)
                                                                {
                                                                    /* Need to track that service we care about is up */
                                                                    serviceDiscoveryInfo_ServiceState = (int)serviceState.ToLong();
                                                                }
                                                            }

                                                            /* AcceptingRequests - Yes (1), No (0) */
                                                            else if (elementEntry.Name.Equals(ElementNames.ACCEPTING_REQS))
                                                            {
                                                                retval = acceptingRequests.Decode(dIter);
                                                                if (retval != CodecReturnCode.SUCCESS && retval != CodecReturnCode.BLANK_DATA)
                                                                {
                                                                    Console.Write("Error {0} ({1}) encountered with DecodeUInt().\n",
                                                                        retval.GetAsInfo(), retval);
                                                                    return (TransportReturnCode)retval;
                                                                }
                                                                if (msg.MsgClass == MsgClasses.REFRESH)
                                                                    Console.Write("\tReceived Source Directory Refresh for Decoded AcceptingRequests: {0}\n",
                                                                        acceptingRequests.ToLong());
                                                                else /* (4) Update Message */
                                                                    Console.Write("\tReceived Source Directory Update for Decoded AcceptingRequests: {0}\n",
                                                                        acceptingRequests.ToLong());

                                                                /* if this is the serviceName that is requested by the application */
                                                                if (serviceCount == foundServiceIndex)
                                                                {
                                                                    /* Need to track that service we care about is accepting requests */
                                                                    serviceDiscoveryInfo_AcceptingRequests = (int)acceptingRequests.ToLong();
                                                                }
                                                            }

                                                            /* Status */
                                                            else if (elementEntry.Name.Equals(ElementNames.STATUS))
                                                            {
                                                                retval = serviceStatus.Decode(dIter);
                                                                if (retval != CodecReturnCode.SUCCESS && retval != CodecReturnCode.BLANK_DATA)
                                                                {
                                                                    Console.Write("Error decoding State.\n");
                                                                    return (TransportReturnCode)retval;
                                                                }
                                                                if (msg.MsgClass == MsgClasses.REFRESH)
                                                                    Console.Write("\tReceived Source Directory Refresh for Decoded State: {0} {1} {2} {3}\n",
                                                                        serviceStatus.StreamState(), serviceStatus.DataState(), serviceStatus.Code(), serviceStatus.Text().ToString());
                                                                else /* (4) Update Message */
                                                                    Console.Write("\tReceived Source Directory Update for Decoded State: {0} {1} {2} {3}\n",
                                                                        serviceStatus.StreamState(), serviceStatus.DataState(), serviceStatus.Code(), serviceStatus.Text().ToString());

                                                                tempBuffer.Data(serviceStatus.ToString());
                                                                Console.Write("{0}\n\n", tempBuffer.ToString());
                                                            }
                                                        }
                                                    }

                                                }
                                                break;
                                            case Rdm.Directory.ServiceFilterIds.GROUP: /* (3) Source Group Filter ID */
                                                {
                                                    Console.Write("\nDecoding Source Group Filter ID FilterListEntry\n");

                                                    /* decode element list - third parameter is 0 because we do not have set definitions in this example */
                                                    if ((retval = elementList.Decode(dIter, null)) < CodecReturnCode.SUCCESS)
                                                    {
                                                        /* decoding failure tends to be unrecoverable */
                                                        Console.Write("Error {0} ({1}) encountered with DecodeElementList.\n",
                                                            retval.GetAsInfo(), retval);
                                                        return (TransportReturnCode)retval;
                                                    }

                                                    /* decode element list elements */
                                                    while ((retval = elementEntry.Decode(dIter)) != CodecReturnCode.END_OF_CONTAINER)
                                                    {
                                                        if (retval < CodecReturnCode.SUCCESS)
                                                        {
                                                            /* decoding failure tends to be unrecoverable */
                                                            Console.Write("Error {0} ({1}) encountered with DecodeElementEntry.\n",
                                                                retval.GetAsInfo(), retval);
                                                            return (TransportReturnCode)retval;
                                                        }
                                                        else
                                                        {
                                                            /* get service group information */

                                                            /* Group */
                                                            if (elementEntry.Name.Equals(ElementNames.GROUP))
                                                            {
                                                                if (msg.MsgClass == MsgClasses.REFRESH)
                                                                    Console.Write("\tReceived Source Directory Refresh for Group: {0}\n",
                                                                        elementEntry.EncodedData);
                                                                else /* (4) Update Message */
                                                                    Console.Write("\tReceived Source Directory Update for Group: {0}\n",
                                                                        elementEntry.EncodedData);
                                                            }
                                                        }
                                                    }
                                                }
                                                break;
                                            case Rdm.Directory.ServiceFilterIds.LOAD: /* (4) Source Load Filter ID */
                                                {
                                                    Console.Write("\nDecoding Source Load Filter ID FilterListEntry not supported in this app\n");
                                                }
                                                break;
                                            case Rdm.Directory.ServiceFilterIds.DATA: /* (5) Source Data Filter ID */
                                                {
                                                    Console.Write("\nDecoding Source Data Filter ID FilterListEntry not supported in this app\n");
                                                }
                                                break;
                                            case Rdm.Directory.ServiceFilterIds.LINK: /* (6) Communication Link Filter ID */
                                                {
                                                    Console.Write("\nDecoding Communication Link Filter ID FilterListEntry not supported in this app\n");
                                                }
                                                break;
                                            default: /* Error handling */
                                                {
                                                    Console.Write("\nUnkonwn FilterListEntry filterID: {0}\n", filterEntry.Id);
                                                    return TransportReturnCode.FAILURE;
                                                }
                                        }
                                    }
                                }
                            }
                            serviceCount++;
                        }

                        if (msg.MsgClass == MsgClasses.REFRESH)
                        {
                            pState = refreshMsg.State;
                            Console.Write(" {0}\n\n", pState.ToString());
                        }
                    }
                    break;

                case MsgClasses.STATUS:
                    Console.Write("\nReceived Source Directory StatusMsg\n");
                    if ((statusMsg.Flags & StatusMsgFlags.HAS_STATE) != 0)
                    {
                        pState = statusMsg.State;
                        Console.Write("	{0}\n\n", pState.ToString());
                    }
                    break;

                case MsgClasses.CLOSE: /* (5) Close Message */
                    {
                        Console.Write("\nReceived Source Directory Close\n");
                    }
                    break;
                default:
                    {
                        Console.Write("Received Unhandled Source Directory Msg Class: " + msg.MsgClass);
                        return TransportReturnCode.FAILURE;
                    }
            }

            return TransportReturnCode.SUCCESS;
        }

        /// <summary>
        /// Processes a dictionary response. This consists of decoding the response.
        /// </summary>
        ///
        /// <remarks>
        /// etaChannelInfo - The channel management information including the dictionaries loaded information that is populated/updated
        /// msg - The partially decoded message
        /// decodeIter - The decode iterator
        /// dataDictionary - the dictionary used for decoding the field entry data
        /// </remarks>
        ///
        /// <param name="chnl">the chnl</param>
        /// <param name="msg">the msg</param>
        /// <param name="dIter">the d iter</param>
        /// <param name="dictionary">the dictionary</param>
        /// <returns>the CodecReturnCode</returns>
        public static TransportReturnCode ProcessDictionaryResponse(IChannel chnl, Msg msg, DecodeIterator dIter, DataDictionary dictionary)
        {
            CodecReturnCode ret = CodecReturnCode.SUCCESS;
            CodecError codecError = new();

            IMsgKey key;
            State pState;

            IRefreshMsg refreshMsg = (IRefreshMsg)msg;
            Int dictionaryType = new();

            switch (msg.MsgClass)
            {
                case MsgClasses.REFRESH:
                    /* decode dictionary response */

                    /* get key */
                    key = msg.MsgKey;

                    if (key != null)
                    {
                        Console.Write("Received Dictionary Response: {0}\n", key.Name.ToString());
                    }
                    else
                    {
                        Console.Write("Received Dictionary Response\n");
                    }

                    pState = refreshMsg.State;

                    Console.Write("{0}\n\n", pState.ToString());

                    /* The dictionary response is typically broken into a multi-part message due to large size of the dictionary */
                    if (((dictionariesLoadedInfo_fieldDictionaryFirstPart) && (msg.StreamId == FIELD_DICTIONARY_STREAM_ID))
                        || ((dictionariesLoadedInfo_enumTypeDictionaryFirstPart) && (msg.StreamId == ENUM_TYPE_DICTIONARY_STREAM_ID)))
                    {
                        /* The first part of a dictionary refresh should contain information about its type.
	    				 * Save this information and use it as subsequent parts arrive. */

                        /* Extracts the RDM Dictionary Type (RDMDictionaryTypes) information from an encoded
                         * dictionary. This can determine the specific dictionary decode function that should be used
                         * (e.g. for dictionary type of RDM_DICTIONARY_FIELD_DEFINITIONS, the user would next invoke the
                         * DecodeFieldDictionary function).
                         *
                         * This is expected to be called after DecodeMsg (where the Msg.domainType is
                         * DomainTypes.DICTIONARY), but before decoding the Msg.encDataBody payload.
                         */
                        if (dictionary.ExtractDictionaryType(dIter, dictionaryType, out codecError) != CodecReturnCode.SUCCESS)
                        {
                            Console.Write("ExtractDictionaryType() failed: {0}\n", codecError.Text);
                            return TransportReturnCode.FAILURE;
                        }
                        int dictionaryType_int = (int)dictionaryType.ToLong();
                        switch (dictionaryType_int)
                        {
                            case Dictionary.Types.FIELD_DEFINITIONS:
                                /* (1) Field Dictionary type, typically referring to an RDMFieldDictionary */
                                dictionariesLoadedInfo_fieldDictionaryFirstPart = false;
                                break;
                            case Dictionary.Types.ENUM_TABLES:
                                /* (2) Enumeration Dictionary type, typically referring to an enumtype.def */
                                dictionariesLoadedInfo_enumTypeDictionaryFirstPart = false;
                                break;
                            default:
                                codecError.Text = "Received unexpected dictionary message on stream " + msg.StreamId;
                                ret = CodecReturnCode.FAILURE;
                                break;
                        }
                    }

                    if (msg.StreamId == FIELD_DICTIONARY_STREAM_ID)
                    {
                        ret = dictionary.DecodeFieldDictionary(dIter, Dictionary.VerbosityValues.VERBOSE, out codecError);
                        if (ret != CodecReturnCode.SUCCESS)
                        {
                            break;
                        }

                        /* (0x0040) Indicates that this is the final part of a refresh. This flag should be set on both
                         *          single-part response messages, as well as the final message in a multi-part response
                         *          message sequence.
                         */
                        if ((refreshMsg.Flags & RefreshMsgFlags.REFRESH_COMPLETE) != 0)
                        {
                            dictionariesLoadedInfo_fieldDictionaryLoaded = true;
                            if (!dictionariesLoadedInfo_enumTypeDictionaryLoaded)
                                Console.Write("Field Dictionary complete, waiting for Enum Table...\n");

                            else
                                Console.Write("Field Dictionary complete.\n");
                        }

                    }
                    else if (msg.StreamId == ENUM_TYPE_DICTIONARY_STREAM_ID)
                    {
                        ret = dictionary.DecodeEnumTypeDictionary(dIter, Dictionary.VerbosityValues.VERBOSE, out codecError);
                        if (ret != CodecReturnCode.SUCCESS)
                        {
                            break;
                        }

                        if ((refreshMsg.Flags & RefreshMsgFlags.REFRESH_COMPLETE) != 0)
                        {
                            dictionariesLoadedInfo_enumTypeDictionaryLoaded = true;
                            if (!dictionariesLoadedInfo_fieldDictionaryLoaded)
                                Console.Write("Enumerated Types Dictionary complete, waiting for Field Dictionary...\n");
                        }
                    }
                    else
                    {
                        codecError.Text = "Received unexpected dictionary message on stream " + msg.StreamId;
                        ret = CodecReturnCode.FAILURE;
                        if (dictionariesLoadedInfo_fieldDictionaryLoaded && dictionariesLoadedInfo_enumTypeDictionaryLoaded)
                        {
                            Console.Write("Dictionary ready, requesting Market Price item...\n\n");

                            /* Ok to send Market Price item request now */
                        }
                    }
                    break;
                case MsgClasses.STATUS:
                    Console.WriteLine("Received StatusMsg for dictionary");
                    IStatusMsg statusMsg = (IStatusMsg)msg;
                    if (statusMsg.CheckHasState())
                    {
                        State ppState = statusMsg.State;

                        Console.Write("	{0}\n\n", ppState.ToString());

                    }
                    break;

                default:
                    Console.WriteLine("Received Unhandled Dictionary MsgClass: {0}", msg.MsgClass);
                    break;
            }
            if (ret != CodecReturnCode.SUCCESS)
            {
                Console.WriteLine("Error processing dictionary response, exit.");
                CloseChannelCleanUpAndExit(chnl, TransportReturnCode.FAILURE, dictionary);

            }
            return TransportReturnCode.SUCCESS;

        }

        /// <summary>
        /// Send dictionary request.
        /// </summary>
        ///
        /// <remarks>
        ///
        /// <para>This consists of getting a message buffer, setting the dictionary
        /// request information, encoding the dictionary request, and sending the
        /// dictionary request to the server.</para>
        ///
        /// <para>A Dictionary request message is encoded and sent by OMM consumer
        /// applications. Some data requires the use of a dictionary for encoding or
        /// decoding.</para>
        ///
        /// <para>This dictionary typically defines type and formatting information and
        /// directs the application as to how to encode or decode specific pieces of
        /// information.</para>
        ///
        /// <para>Content that uses the FieldList type requires the use of a field
        /// dictionary (usually the RDMFieldDictionary, though it could also be
        /// a user-defined or modified field dictionary).</para>
        ///
        /// <remarks>
        ///
        /// <param name="chnl">the chnl</param>
        /// <param name="dictionary">the dictionary</param>
        /// <param name="maxFragmentSize">the max fragment size</param>
        /// <param name="dictionaryName">the dictionary name</param>
        ///
        /// <returns>status code</returns>
        public static TransportReturnCode SendDictionaryRequest(IChannel chnl, DataDictionary dictionary, int maxFragmentSize, string dictionaryName)
        {
            Error error = new();

            CodecReturnCode ret;
            ITransportBuffer? msgBuf;
            IRequestMsg requestMsg = new Msg();

            /* ETA provides clear functions for its structures (e.g., encodeIterator.Clear()).
             *
             * These functions are tuned to be efficient and avoid initializing unnecessary
             * structure members, and allow for optimal structure use and reuse.
             */
            /* Iterator used for encoding throughout the application - we can clear it and
             * reuse it instead of recreating it */
            EncodeIterator encIter = new();

            /* Obtains a non-packable buffer of the requested size from the ETA Transport guaranteed
             * buffer pool to write into for the Login request.
             *
             * For an outbound TransportBuffer (specifically one gotten from Channel.GetBuffer(),
             * like this one) length() initially does indeed indicate the number of bytes available,
             * but when the ByteBuffer's position is after the start (because the application has
             * written something, i.e. encoded some or all of a Login request message), it actually
             * returns the number of bytes between the start & current positions (basically, the
             * number of bytes encoded thus far).
             */
            /* etaGetBuffer() is the utility function that does 2-pass (more robust) getting non-packable buffer. */
            if ((msgBuf = EtaGetBuffer(chnl, maxFragmentSize, error)) == null) /* first check Error */
            {
                /* Connection should be closed, return failure */
                /* Closes channel, cleans up and exits the application. */
                return TransportReturnCode.FAILURE;
            }

            /* if a buffer is returned, we can populate and write, encode an Msg into the buffer */

            /* Encodes the Dictionary request. */
            requestMsg.Clear();
            encIter.Clear();

            /* set the buffer on an EncodeIterator */
            if ((ret = encIter.SetBufferAndRWFVersion(msgBuf, chnl.MajorVersion, chnl.MinorVersion)) < CodecReturnCode.SUCCESS)
            {
                chnl.ReleaseBuffer(msgBuf, out error);
                Console.Write("\nSetEncodeIteratorBuffer() failed with return code: {0}\n", ret);
                /* Closes channel, cleans up and exits the application. */
                return TransportReturnCode.FAILURE;
            }

            requestMsg.MsgClass = MsgClasses.REQUEST;
            requestMsg.DomainType = (int)DomainType.DICTIONARY;
            requestMsg.ContainerType = DataTypes.NO_DATA;
            requestMsg.Flags = RequestMsgFlags.NONE;

            if (dictionaryName.Equals(dictionaryDownloadName))
            {
                requestMsg.StreamId = FIELD_DICTIONARY_STREAM_ID;
            }
            else if (dictionaryName.Equals(enumTableDownloadName))
            {
                requestMsg.StreamId = ENUM_TYPE_DICTIONARY_STREAM_ID;
            }
            else
            {
                Console.Write("\nSend Dictionary Request with Unknown Dictionary Name: {0}\n", dictionaryName);
                /* Closes channel, cleans up and exits the application. */
                return TransportReturnCode.FAILURE;
            }
            requestMsg.MsgKey.ApplyHasFilter();
            requestMsg.MsgKey.ApplyHasName();
            requestMsg.MsgKey.ApplyHasServiceId();

            requestMsg.MsgKey.Name.Data(dictionaryName);
            requestMsg.MsgKey.ServiceId = (int)serviceDiscoveryInfo_serviceId.ToLong();
            requestMsg.MsgKey.Filter = Dictionary.VerbosityValues.VERBOSE;

            /* encode message - populate message and encode it */
            if ((ret = requestMsg.Encode(encIter)) < CodecReturnCode.SUCCESS)
            {
                chnl.ReleaseBuffer(msgBuf, out error);
                Console.Write("EncodeMsg() failed with return code: {1}\n", ret);
                /* Closes channel, cleans up and exits the application. */
                return (TransportReturnCode)ret;
            }
            /* send dictionary request */
            if ((ret = (CodecReturnCode)SendMessage(chnl, msgBuf)) < CodecReturnCode.SUCCESS)
            {
                /* Closes channel, cleans up and exits the application. */
                return TransportReturnCode.FAILURE;
            }
            else if (ret > CodecReturnCode.SUCCESS)
            {
                /* There is still data left to flush, leave our write notification enabled so we get
                 * called again.
                 *
                 * If everything wasn't flushed, it usually indicates that the TCP output buffer
                 * cannot accept more yet
                 */

                /* flush needs to be done by application */
            }

            return (TransportReturnCode)ret;

        }

        /// <summary>
        /// Send market price item request.
        /// </summary>
        ///
        /// <param name="chnl">the chnl</param>
        /// <param name="maxFragmentSize">the max fragment size</param>
        ///
        /// <returns>the int</returns>
        public static TransportReturnCode SendMarketPriceItemRequest(IChannel chnl, int maxFragmentSize)
        {
            Error error = new();

            CodecReturnCode ret;
            ITransportBuffer? msgBuf;
            IRequestMsg requestMsg = (IRequestMsg)new Msg();

            /* Iterator used for encoding throughout the application - we can clear it and reuse it instead of recreating it */
            EncodeIterator encIter = new();

            /* EtaGetBuffer() is the utility function that does 2-pass (more robust) getting non-packable buffer. */
            if ((msgBuf = EtaGetBuffer(chnl, maxFragmentSize, error)) == null) /* first check Error */
            {
                /* Connection should be closed, return failure */
                /* Closes channel, cleans up and exits the application. */
                return TransportReturnCode.FAILURE;
            }

            /* if a buffer is returned, we can populate and write, encode an Msg into the buffer */

            /* Encodes the Dictionary request. */

            requestMsg.Clear();

            /* clear encode iterator for reuse - this should be used to achieve the best performance
             * while clearing the iterator. */
            encIter.Clear();

            /* set version information of the connection on the encode iterator so proper versioning
             * can be performed */

            /* set the buffer on an EncodeIterator */
            if ((ret = encIter.SetBufferAndRWFVersion(msgBuf, chnl.MajorVersion, chnl.MinorVersion)) < CodecReturnCode.SUCCESS)
            {
                chnl.ReleaseBuffer(msgBuf, out error);
                Console.Write("\nSetEncodeIteratorBuffer() failed with return code: {0}\n", ret);
                /* Closes channel, cleans up and exits the application. */
                return TransportReturnCode.FAILURE;
            }

            requestMsg.MsgClass = MsgClasses.REQUEST;
            requestMsg.StreamId = MARKETPRICE_ITEM_STREAM_ID;
            requestMsg.DomainType = (int)DomainType.MARKET_PRICE;
            /* No Batch request - since we are only showing one item in the itemList, it is a waste
             * of bandwidth to send a batch request */

            /* No View */
            requestMsg.ContainerType = DataTypes.NO_DATA;

            /* RequestMsgFlags.STREAMING flag set. When the flag is set, the request is known as a
             * "streaming" request, meaning that the refresh will be followed by updates.
             */
            requestMsg.ApplyHasQos();
            requestMsg.ApplyHasPriority();
            requestMsg.ApplyStreaming();

            /* (0x002) This RequestMsg has priority information, contained in
             *         RequestMsg.PriorityClass and RequestMsg.PriorityCount. This is used to
             *         indicate the importance of this stream.
             */
            requestMsg.Priority.PriorityClass = 1;
            requestMsg.Priority.Count = 1;

            /* (0x040) This RequestMsg contains quality of service information, contained in
             *         RequestMsg.Qos.  If only RequestMsg.Qos is present, this is the QoS that will
             *         satisfy the request.  If RequestMsg.Qos and RequestMsg.WorstQos are both
             *         present, this indicates that any QoS in that range will satisfy the
             *         request. '
             */
            /* copy the QoS information */
            serviceDiscoveryInfo_qoS[0].Copy(requestMsg.Qos);

            /* specify msgKey members */
            requestMsg.MsgKey.ApplyHasServiceId();
            requestMsg.MsgKey.ApplyHasNameType();
            requestMsg.MsgKey.ApplyHasName();
            /* msgKey.NameType Optional. When consuming from LSEG sources, typically set to
             * InstrumentNameTypes.RIC (the "Reuters Instrument Code"). If this is not specified,
             * msgKey.NameType defaults to RDM_INSTRUMENT_NAME_TYPE_RIC.
             */
            requestMsg.MsgKey.NameType = InstrumentNameTypes.RIC;
            /* msgKey.Name Required in initial request, otherwise optional. The name of the requested item. */
            requestMsg.MsgKey.Name.Data(marketPriceItemInfo_itemName);
            /* msgKey.ServiceId is Required. This should be the Id associated with the service from
             * which the consumer wishes to request the item.
             */
            requestMsg.MsgKey.ServiceId = ((int)serviceDiscoveryInfo_serviceId.ToLong());

            ret = requestMsg.Encode(encIter);
            if (ret < CodecReturnCode.SUCCESS)
            {
                chnl.ReleaseBuffer(msgBuf, out error);
                Console.Write("EncodeMsg() failed with return code: {0}\n", ret);
                return (TransportReturnCode)ret;
            }

            /* send dictionary request */
            if ((ret = (CodecReturnCode)SendMessage(chnl, msgBuf)) < CodecReturnCode.SUCCESS)
            {
                /* Closes channel, cleans up and exits the application. */
                return TransportReturnCode.FAILURE;
            }
            else if (ret > CodecReturnCode.SUCCESS)
            {
                /* There is still data left to flush, leave our write notification enabled so we get called again.
                 * If everything wasn't flushed, it usually indicates that the TCP output buffer cannot accept more yet
                 */

                /* flush needs to be done by application */
            }

            return (TransportReturnCode)ret;

        }

        /// <summary>
        /// Processes a market price response.
        /// </summary>
        ///
        /// <remarks>
        /// This consists of extracting the key, printing out the item name contained in the key,
        /// decoding the field list and field entry data.
        /// </remarks>
        ///
        /// <param name="chnl">the chnl</param>
        /// <param name="msg">the msg</param>
        /// <param name="dIter">the decode iterator</param>
        /// <param name="dictionary">the dictionary</param>
        /// <param name="error">the error</param>
        ///
        /// <returns>status code</returns>
        public static TransportReturnCode ProcessMarketPriceItemResponse(IChannel chnl, Msg msg, DecodeIterator dIter, DataDictionary dictionary)
        {
            /* The MsgKey contains a variety of attributes used to identify the contents flowing
             * within a particular stream.
             *
             * This information, in conjunction with DomainType and quality of service information,
             * can be used to uniquely identify a data stream.
             */
            IMsgKey key = new MsgKey();

            FieldList fieldList = new();

            CodecReturnCode ret;

            IRefreshMsg refreshMsg = (IRefreshMsg)msg;
            IUpdateMsg updateMsg = (IUpdateMsg)msg;

            switch (msg.MsgClass)
            {
                case MsgClasses.REFRESH: /* (2) Refresh Message */
                    {

                        Console.Write("Received Item RefreshMsg for stream {0}\n", refreshMsg.StreamId);
                        /* update our item state list if its a refresh, then process just like update */

                        marketPriceItemInfo_itemState.DataState(refreshMsg.State.DataState());
                        marketPriceItemInfo_itemState.StreamState(refreshMsg.State.StreamState());
                        Console.Write("{0}\n\n", refreshMsg.State.ToString());

                        /* refresh continued - process just like update */
                        goto case MsgClasses.UPDATE;
                    }
                case MsgClasses.UPDATE: /* (4) Update Message */
                    {
                        /* decode market price response for both Refresh Msg and Update Msg */

                        if (msg.MsgClass == MsgClasses.UPDATE)
                        {
                            Console.Write("\nReceived Item Update Msg for stream {0} \n", updateMsg.StreamId);
                            /* When displaying update information, we should also display the updateType information. */
                            /* Indicates domain-specific information about the type of content
                             * contained in this update.  Domain Models.
                             */
                            Console.Write("UPDATE TYPE: {0}\n", ((IUpdateMsg)msg).UpdateType);
                        }

                        /* get key */
                        key = msg.MsgKey;

                        /* print out item name from key if it has it */

                        if (key != null && key.CheckHasName())
                        {
                            Console.Write("\n{0}\nDOMAIN: {1}\n",
                                key.Name.ToString(), DomainTypes.ToString(msg.DomainType));

                        }
                        else /* cached item name*/
                        {
                            Console.Write("\n{0}\nDOMAIN: {1}\n",
                                marketPriceItemInfo_itemName, DomainTypes.ToString(msg.DomainType));
                        }

                        /* decode into the MarketPrice Payload field list structure */
                        ret = (CodecReturnCode)DecodeMarketPricePayload(fieldList, dIter, dictionary);

                        if (ret != CodecReturnCode.SUCCESS)
                        {
                            Console.WriteLine("DecodeFieldList() failed with return code: " + ret);
                            return (TransportReturnCode)ret;
                        }

                    }
                    break;
                case MsgClasses.STATUS: /* (3) Status Message */
                    {
                        IStatusMsg statusMsg = (IStatusMsg)msg;
                        Console.WriteLine("Received Item StatusMsg for stream " + msg.StreamId);

                        /* (0x020) Indicates that this StatusMsg has stream or group state
                        *          information, contained in StatusMsg.State.
                        */

                        if (statusMsg.CheckHasState())
                        {
                            Console.Write("{0}\n", statusMsg.State.ToString());

                            /* Update our state table with the new state */
                            marketPriceItemInfo_itemState.DataState(statusMsg.State.DataState());
                            marketPriceItemInfo_itemState.StreamState(statusMsg.State.StreamState());
                        }
                    }
                    break;
                case MsgClasses.ACK:
                    {
                        Console.WriteLine("Received AckMsg for stream " + msg.StreamId);

                        /* get key */
                        key = msg.MsgKey;

                        /* print out item name from key if it has it */

                        if (key != null && key.CheckHasName())
                        {
                            Console.Write("\n{0}\nDOMAIN: {1}\n",
                                key.Name.Data(), DomainTypes.ToString(msg.DomainType));

                        }
                        else
                        /*cached item name*/
                        {
                            Console.Write("\n{0}\nDOMAIN: {1}\n",
                                marketPriceItemInfo_itemName, DomainTypes.ToString(msg.DomainType));
                        }
                        IAckMsg ackMsg = (IAckMsg)msg;
                        Console.Write("\tackId={0}\n", ackMsg.AckId);

                        if (ackMsg.CheckHasSeqNum())
                            Console.Write("\tseqNum={0}\n", ackMsg.SeqNum);

                        if (ackMsg.CheckHasNakCode())
                            Console.Write("\tnakCode={0}\n", ackMsg.NakCode);

                        if (ackMsg.CheckHasText())
                            Console.Write("\ttext={0}\n", ackMsg.Text.Data());

                    }
                    break;
                default:/* Error handling */
                    Console.WriteLine("Received Unhandled Item Msg Class: " + msg.MsgClass);
                    break;
            }

            return TransportReturnCode.SUCCESS;
        }

        static UInt fidUIntValue = new();
        static Int fidIntValue = new();
        static Real fidRealValue = new();
        static LSEG.Eta.Codec.Float fidFloatValue = new();
        static LSEG.Eta.Codec.Double fidDoubleValue = new();
        static LSEG.Eta.Codec.Enum fidEnumValue = new();
        static LSEG.Eta.Codec.Date fidDateValue = new();
        static Time fidTimeValue = new();
        static LSEG.Eta.Codec.DateTime fidDateTimeValue = new();

        /// <summary>
        /// Decode market price payload.
        /// </summary>
        ///
        /// <param name="fieldList">the field list</param>
        /// <param name="dIter">the decode iterator</param>
        /// <param name="dictionary">the dictionary</param>
        ///
        /// <returns>status code</returns>
        public static TransportReturnCode DecodeMarketPricePayload(FieldList fieldList, DecodeIterator dIter, DataDictionary dictionary)
        {
            CodecReturnCode ret = 0;
            FieldEntry fEntry = new();
            /* (0) Unknown Data Type. This is only valid when decoding an RsslFieldEntry type that
             *     requires a dictionary look-up.
             *
             * If content is set defined, actual type enum will be present.
             */
            int dataType = DataTypes.UNKNOWN;

            fidUIntValue.Clear();
            fidIntValue.Clear();
            fidRealValue.Clear();
            fidFloatValue.Clear();
            fidDoubleValue.Clear();
            fidEnumValue.Clear();
            fidDateValue.Clear();
            fidTimeValue.Clear();
            fidDateTimeValue.Clear();

            Qos fidQosValue = new();
            State fidStateValue = new();
            /*get dictionary entry*/
            IDictionaryEntry dictionaryEntry = dictionary.Entry(fEntry.FieldId);

            /* decode into the field list structure */
            if ((ret = fieldList.Decode(dIter, null)) == CodecReturnCode.SUCCESS)
            {
                /* decode each field entry in list */
                while ((ret = fEntry.Decode(dIter)) != CodecReturnCode.END_OF_CONTAINER)
                {
                    if (ret == CodecReturnCode.SUCCESS)
                    {
                        /* decode each field entry info */
                        /* look up type in field dictionary and call correct primitive decode function */

                        /* get dictionary entry */
                        if (!dictionariesLoadedInfo_isInitialized)
                        {
                            return TransportReturnCode.FAILURE;
                        }
                        else
                            dictionaryEntry = dictionary.Entry(fEntry.FieldId);

                        // return if no entry found
                        if (dictionaryEntry == null)
                        {
                            Console.Write("\tFid {0} not found in dictionary\n", fEntry.FieldId);
                            return TransportReturnCode.SUCCESS;
                        }

                        // print out fid name
                        Console.Write("\t" + fEntry.FieldId + "/" + dictionaryEntry.GetAcronym().ToString() + ": ");

                        // decode and print out fid value
                        dataType = dictionaryEntry.GetRwfType();

                        switch (dataType)
                        {
                            case DataTypes.UINT:
                                ret = fidUIntValue.Decode(dIter);
                                if (ret == CodecReturnCode.SUCCESS)
                                {
                                    Console.Write("{0}\n", fidUIntValue.ToLong());
                                }
                                else if (ret != CodecReturnCode.BLANK_DATA)
                                {
                                    Console.WriteLine("DecodeUInt() failed: <" + ret.GetAsString() + ">");
                                    return (TransportReturnCode)ret;
                                }
                                break;
                            case DataTypes.INT:
                                ret = fidIntValue.Decode(dIter);
                                if (ret == CodecReturnCode.SUCCESS)
                                {
                                    Console.Write("{0}\n", fidIntValue.ToLong());
                                }
                                else if (ret != CodecReturnCode.BLANK_DATA)
                                {
                                    Console.WriteLine("DecodeInt() failed: <" + ret.GetAsString() + ">");

                                    return (TransportReturnCode)ret;
                                }
                                break;
                            case DataTypes.FLOAT:
                                ret = fidFloatValue.Decode(dIter);
                                if (ret == CodecReturnCode.SUCCESS)
                                {
                                    Console.Write("{0}\n", fidFloatValue.ToFloat());
                                }
                                else if (ret != CodecReturnCode.BLANK_DATA)
                                {
                                    Console.WriteLine("DecodeFloat() failed: <" + ret.GetAsString() + ">");

                                    return (TransportReturnCode)ret;
                                }
                                break;
                            case DataTypes.DOUBLE:
                                ret = fidDoubleValue.Decode(dIter);
                                if (ret == CodecReturnCode.SUCCESS)
                                {
                                    Console.Write("{0}\n", fidDoubleValue.ToDouble());
                                }
                                else if (ret != CodecReturnCode.BLANK_DATA)
                                {
                                    Console.WriteLine("DecodeDouble() failed: <" + ret.GetAsString() + ">");

                                    return (TransportReturnCode)ret;
                                }
                                break;
                            case DataTypes.REAL:
                                ret = fidRealValue.Decode(dIter);
                                if (ret == CodecReturnCode.SUCCESS)
                                {
                                    Console.Write("{0}\n", fidRealValue.ToString());
                                }
                                else if (ret != CodecReturnCode.BLANK_DATA)
                                {
                                    Console.WriteLine("DecodeReal() failed: <" + ret.GetAsString() + ">");

                                    return (TransportReturnCode)ret;
                                }
                                break;
                            case DataTypes.ENUM:
                                ret = fidEnumValue.Decode(dIter);
                                if (ret == CodecReturnCode.SUCCESS)
                                {
                                    IEnumType enumType = dictionary.EntryEnumType(dictionaryEntry, fidEnumValue);

                                    if (enumType == null)
                                    {
                                        Console.Write("{0}\n", fidEnumValue.ToInt());
                                    }
                                    else
                                    {
                                        Console.WriteLine(enumType.Display.ToString() + "(" + fidEnumValue.ToInt() + ")");
                                    }
                                }
                                else if (ret != CodecReturnCode.BLANK_DATA)
                                {
                                    Console.WriteLine("DecodeEnum() failed: <" + ret.GetAsString() + ">");

                                    return (TransportReturnCode)ret;
                                }
                                break;
                            case DataTypes.DATE:
                                ret = fidDateValue.Decode(dIter);
                                if (ret == CodecReturnCode.SUCCESS)
                                {
                                    Console.Write("{0}\n", fidDateValue.ToString());

                                }
                                else if (ret != CodecReturnCode.BLANK_DATA)
                                {
                                    Console.WriteLine("DecodeDate() failed: <" + ret.GetAsString() + ">");

                                    return (TransportReturnCode)ret;
                                }
                                break;
                            case DataTypes.TIME:
                                ret = fidTimeValue.Decode(dIter);
                                if (ret == CodecReturnCode.SUCCESS)
                                {
                                    Console.Write("{0}\n", fidTimeValue.ToString());
                                }
                                else if (ret != CodecReturnCode.BLANK_DATA)
                                {
                                    Console.WriteLine("DecodeTime() failed: <" + ret.GetAsString() + ">");

                                    return (TransportReturnCode)ret;
                                }
                                break;
                            case DataTypes.DATETIME:
                                ret = fidDateTimeValue.Decode(dIter);
                                if (ret == CodecReturnCode.SUCCESS)
                                {
                                    Console.Write("{0}\n", fidDateTimeValue.ToString());
                                }
                                else if (ret != CodecReturnCode.BLANK_DATA)
                                {
                                    Console.WriteLine("DecodeDateTime() failed: <" + ret.GetAsString() + ">");
                                    return (TransportReturnCode)ret;
                                }
                                break;
                            case DataTypes.QOS:
                                ret = fidQosValue.Decode(dIter);
                                if (ret == CodecReturnCode.SUCCESS)
                                {
                                    Console.Write("{0}\n", fidQosValue.ToString());
                                }
                                else if (ret != CodecReturnCode.BLANK_DATA)
                                {
                                    Console.WriteLine("DecodeQos() failed: <" + ret.GetAsString() + ">");

                                    return (TransportReturnCode)ret;
                                }
                                break;
                            case DataTypes.STATE:
                                ret = fidStateValue.Decode(dIter);
                                if (ret == CodecReturnCode.SUCCESS)
                                {
                                    Console.Write("{0}\n", fidStateValue.ToString());
                                }
                                else if (ret != CodecReturnCode.BLANK_DATA)
                                {
                                    Console.WriteLine("DecodeState() failed: <" + ret.GetAsString() + ">");

                                    return (TransportReturnCode)ret;
                                }
                                break;
                            // For an example of array decoding, see
                            // FieldListCodec.exampleDecode()
                            case DataTypes.ARRAY:
                                break;
                            case DataTypes.BUFFER:
                            case DataTypes.ASCII_STRING:
                            case DataTypes.UTF8_STRING:
                            case DataTypes.RMTES_STRING:
                                if (fEntry.EncodedData.Length > 0)
                                {
                                    Console.Write("{0}\n", fEntry.EncodedData.ToString());
                                }
                                else
                                {
                                    ret = CodecReturnCode.BLANK_DATA;
                                }
                                break;
                            default:
                                Console.Write("Unsupported data type ( {0}" + DataTypes.ToString(dataType) + ")");
                                break;
                        }
                        if (ret == CodecReturnCode.BLANK_DATA)
                        {
                            Console.WriteLine("<blank data>");
                        }
                    }
                    else
                    {
                        /* decoding failure tends to be unrecoverable */
                        Console.Write("Error ({0}) (errno: {1}) encountered with DecodeFieldEntry.\n",
                            ret.GetAsInfo(), ret);

                        return (TransportReturnCode)ret;

                    }
                }
            }
            else
            {
                /* decoding failure tends to be unrecoverable */
                Console.Write("Error ({0}) (errno: {1}) encountered with DecodeFieldList.\n",
                    ret.GetAsInfo(), ret);

                return (TransportReturnCode)ret;
            }

            return TransportReturnCode.SUCCESS;
        }

        /// Close market price item stream.
        ///
        /// <param name="chnl">the channel</param>
        /// <param name="maxFragmentSize">the max fragment size</param>
        ///
        /// <returns>status code</returns>
        public static TransportReturnCode CloseMarketPriceItemStream(IChannel chnl, int maxFragmentSize)
        {
            Error error = new();
            /* get a buffer for the item close */
            ITransportBuffer? msgBuf;
            ICloseMsg closeMessage = (ICloseMsg)new Msg();
            CodecReturnCode ret;

            /* Iterator used for encoding throughout the application - we can clear it and reuse it instead of recreating it */
            EncodeIterator encIter = new();

            /* Obtains a non-packable buffer of the requested size from the ETA Transport
             * guaranteed buffer pool to write into for the Login request.
             *
             * For an outbound TransportBuffer (specifically one gotten from
             * Channel.getBuffer(), like this one) length() initially does indeed indicate
             * the number of bytes available, but when the ByteBuffer's position is after
             * the start (because the application has written something, i.e. encoded some
             * or all of a Login request message), it actually returns the number of bytes
             * between the start & current positions (basically, the number of bytes
             * encoded thus far).
             */

            /* EtaGetBuffer() is the utility function that does 2-pass (more robust) getting non-packable buffer. */
            if ((msgBuf = EtaGetBuffer(chnl, maxFragmentSize, error)) == null) /* first check Error */
            {
                /* Connection should be closed, return failure */
                /* Closes channel, cleans up and exits the application. */
                return TransportReturnCode.FAILURE;
            }

            /* if a buffer is returned, we can populate and write, encode an Msg into the buffer */

            /* Encodes the Dictionary request. */

            closeMessage.Clear();

            /* clear encode iterator for reuse - this should be used to achieve the best performance
             * while clearing the iterator. */
            encIter.Clear();

            /* set version information of the connection on the encode iterator so proper versioning
             * can be performed */

            /* set the buffer on an EncodeIterator */
            if ((ret = encIter.SetBufferAndRWFVersion(msgBuf, chnl.MajorVersion, chnl.MinorVersion)) < CodecReturnCode.SUCCESS)
            {
                chnl.ReleaseBuffer(msgBuf, out error);
                Console.Write("\nSetEncodeIteratorBuffer() failed with return code: {0}\n", ret);
                /* Closes channel, cleans up and exits the application. */
                return TransportReturnCode.FAILURE;
            }

            /* encode item close */
            closeMessage.MsgClass = MsgClasses.CLOSE;
            closeMessage.StreamId = MARKETPRICE_ITEM_STREAM_ID;
            closeMessage.DomainType = (int)DomainType.MARKET_PRICE;
            closeMessage.ContainerType = DataTypes.NO_DATA;

            /* encode message */

            /* Since there is no payload, no need for Init/Complete as everything is in the msg header */
            /* Functions without a suffix of Init or Complete (e.g. EncodeMsg) perform encoding
             * within a single call, typically used for encoding simple types like Integer or
             * incorporating previously encoded data (referred to as pre-encoded data).
             */
            ret = closeMessage.Encode(encIter);
            if (ret < CodecReturnCode.SUCCESS)
            {
                chnl.ReleaseBuffer(msgBuf, out error);
                Console.WriteLine("encodeMarketPriceClose(): Failed <" + ret.GetAsString() + ">");
                return TransportReturnCode.FAILURE;
            }

            /* send Market Price item close */
            if ((ret = (CodecReturnCode)SendMessage(chnl, msgBuf)) < CodecReturnCode.SUCCESS)
            {
                /* Closes channel, cleans up and exits the application. */
                return TransportReturnCode.FAILURE;
            }
            else if (ret > CodecReturnCode.SUCCESS)
            {
                /* There is still data left to flush, leave our write notification enabled so we get
                 * called again.
                 *
                 * If everything wasn't flushed, it usually indicates that the TCP output buffer
                 * cannot accept more yet
                 */

                /* flush needs to be done by application */
            }

            return (TransportReturnCode)ret;
        }
    }
}
