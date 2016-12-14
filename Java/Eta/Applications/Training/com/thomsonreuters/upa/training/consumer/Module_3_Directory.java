/*********************************************************************************
 * This is the UPA Consumer Training series of the UPA Training Suite
 * applications. The purpose of this application is to show step-by-step 
 * training how to build a UPA OMM Consumer using the UPA Transport layer.
 *
 * Main Java source file for the UPA Consumer Training application. It is a 
 * single-threaded client application.
 *
 *********************************************************************************
 * UPA Consumer Training Module 1a: Establish network communication
 *********************************************************************************
 * Summary:
 * In this module, the application initializes the UPA Transport and 
 * connects the client. An OMM consumer application can establish a 
 * connection to other OMM Interactive Provider applications, including 
 * the Enterprise Platform, Data Feed Direct, and Elektron.
 *
 * Detailed Descriptions:
 * The first step of any UPA consumer application is to establish a 
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
 * UPA Consumer Training Module 1b: Ping (heartbeat) Management
 *********************************************************************************
 * Summary:
 * Ping or heartbeat messages indicate the continued presence of an application. 
 * After the consumer's connection is active, ping messages must be exchanged. 
 * The negotiated ping timeout is retrieved using the Channel.pingTimeout() method. 
 * The connection will be terminated if ping heartbeats are not sent or received 
 * within the expected time frame.
 *
 * Detailed Descriptions:
 * Ping or heartbeat messages are used to indicate the continued presence of 
 * an application. These are typically only required when no other information 
 * is being exchanged. For example, there may be long periods of time that 
 * elapse between requests made from an OMM consumer application. In this 
 * situation, the consumer would send periodic heartbeat messages to inform 
 * the providing application that it is still alive. Because the provider 
 * application is likely sending more frequent information, providing updates 
 * on any streams the consumer has requested, it may not need to send 
 * heartbeats as the other data is sufficient to announce its continued 
 * presence. It is the responsibility of each connection to manage the sending
 * and receiving of heartbeat messages.
 * 
 *********************************************************************************
 * UPA Consumer Training Module 1c: Reading and Writing Data
 *********************************************************************************
 * Summary:
 * When channel initialization is complete, the state of the channel 
 * Channel.state() is TransportReturnCodes.ACTIVE, and applications can send 
 * and receive data.
 *
 * Detailed Descriptions:
 * When a client or server Channel.state() is ChannelState.ACTIVE, it is 
 * possible for an application to receive data from the connection. The 
 * arrival of this information is often announced by the I/O notification 
 * mechanism that the Channel.scktChannel() is registered with. The UPA 
 * Transport reads information from the network as a byte stream, after 
 * which it determines buffer boundaries and returns each buffer one by 
 * one.
 * 
 * When a client or server Channel.state() is ChannelState.ACTIVE, it is 
 * possible for an application to write data to the connection. Writing 
 * involves a several step process. Because the UPA Transport provides 
 * efficient buffer management, the user is required to obtain a buffer 
 * from the UPA Transport buffer pool. This can be the guaranteed output 
 * buffer pool associated with a Channel. After a buffer is acquired, 
 * the user can populate the Buffer.data and set the Buffer.length 
 * to the number of bytes referred to by data. If queued information cannot 
 * be passed to the network, a function is provided to allow the application 
 * to continue attempts to flush data to the connection. An I/O notification
 * mechanism can be used to help with determining when the network is able 
 * to accept additional bytes for writing. The UPA Transport can continue to
 * queue data, even if the network is unable to write. 
 * 
 *********************************************************************************
 * UPA Consumer Training Module 2: Log in
 *********************************************************************************
 * Summary:
 * Applications authenticate using the Login domain model. An OMM consumer must 
 * authenticate with a provider using a Login request prior to issuing any other 
 * requests or opening any other streams. After receiving a Login request, an 
 * Interactive Provider determines whether a user is permissioned to access the 
 * system. The Interactive Provider sends back a Login response, indicating to 
 * the consumer whether access is granted.
 *
 * Detailed Descriptions:
 * After receiving a Login request, an Interactive Provider determines whether 
 * a user is permissioned to access the system. The Interactive Provider sends 
 * back a Login response, indicating to the consumer whether access is granted.
 * 
 * a) If the application is denied, the Login stream is closed, and the 
 * consumer application cannot send additional requests.
 * b) If the application is granted access, the Login response contains 
 * information about available features, such as Posting, Pause and Resume, 
 * and the use of Dynamic Views. The consumer application can use this 
 * information to tailor its interaction with the provider.
 *
 * Content is encoded and decoded using the UPA Message Package and the UPA 
 * Data Package.
 *
 ************************************************************************
 * UPA Consumer Training Module 3: Obtain Source Directory
 ************************************************************************
 * Summary:
 * The Source Directory domain model conveys information about all available 
 * services in the system. An OMM consumer typically requests a Source 
 * Directory to retrieve information about available services and their 
 * capabilities. This includes information about supported domain types, the 
 * service's state, the quality of service (QoS), and any item group 
 * information associated with the service.
 *
 * Detailed Descriptions:
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
 * Content is encoded and decoded using the UPA Message Package and the UPA 
 * Data Package.
 *
 *********************************************************************************/

package com.thomsonreuters.upa.training.consumer;

import java.io.IOException;

import java.nio.channels.ClosedChannelException;
import java.nio.channels.SelectionKey;
import java.nio.channels.Selector;
import java.util.ArrayList;
import java.util.Iterator;
import java.util.List;
import java.util.Set;

import com.thomsonreuters.upa.codec.Codec;
import com.thomsonreuters.upa.transport.Channel;
import com.thomsonreuters.upa.transport.ChannelInfo;
import com.thomsonreuters.upa.transport.ChannelState;
import com.thomsonreuters.upa.transport.CompressionTypes;
import com.thomsonreuters.upa.transport.ConnectOptions;
import com.thomsonreuters.upa.transport.ConnectionTypes;
import com.thomsonreuters.upa.transport.Error;
import com.thomsonreuters.upa.transport.InProgFlags;
import com.thomsonreuters.upa.transport.InProgInfo;
import com.thomsonreuters.upa.transport.InitArgs;
import com.thomsonreuters.upa.transport.Transport;
import com.thomsonreuters.upa.transport.TransportFactory;
import com.thomsonreuters.upa.transport.TransportReturnCodes;
import com.thomsonreuters.upa.transport.ReadArgs;
import com.thomsonreuters.upa.codec.Array;
import com.thomsonreuters.upa.codec.ArrayEntry;
import com.thomsonreuters.upa.codec.Buffer;
import com.thomsonreuters.upa.codec.CloseMsg;
import com.thomsonreuters.upa.codec.CodecFactory;
import com.thomsonreuters.upa.codec.CodecReturnCodes;
import com.thomsonreuters.upa.codec.DataStates;
import com.thomsonreuters.upa.codec.DataTypes;
import com.thomsonreuters.upa.codec.DecodeIterator;
import com.thomsonreuters.upa.codec.ElementEntry;
import com.thomsonreuters.upa.codec.ElementList;
import com.thomsonreuters.upa.codec.ElementListFlags;
import com.thomsonreuters.upa.codec.EncodeIterator;
import com.thomsonreuters.upa.codec.FilterEntry;
import com.thomsonreuters.upa.codec.FilterEntryFlags;
import com.thomsonreuters.upa.codec.FilterList;
import com.thomsonreuters.upa.codec.Map;
import com.thomsonreuters.upa.codec.MapEntry;
import com.thomsonreuters.upa.codec.MapFlags;
import com.thomsonreuters.upa.codec.Msg;
import com.thomsonreuters.upa.codec.MsgClasses;
import com.thomsonreuters.upa.codec.MsgKey;
import com.thomsonreuters.upa.codec.MsgKeyFlags;
import com.thomsonreuters.upa.codec.Qos;
import com.thomsonreuters.upa.codec.QosRates;
import com.thomsonreuters.upa.codec.QosTimeliness;
import com.thomsonreuters.upa.codec.RefreshMsg;
import com.thomsonreuters.upa.codec.RefreshMsgFlags;
import com.thomsonreuters.upa.codec.RequestMsg;
import com.thomsonreuters.upa.codec.RequestMsgFlags;
import com.thomsonreuters.upa.codec.State;
import com.thomsonreuters.upa.codec.StatusMsg;
import com.thomsonreuters.upa.codec.StatusMsgFlags;
import com.thomsonreuters.upa.codec.StreamStates;
import com.thomsonreuters.upa.codec.UInt;
import com.thomsonreuters.upa.codec.UpdateMsg;
import com.thomsonreuters.upa.rdm.Directory;
import com.thomsonreuters.upa.rdm.DomainTypes;
import com.thomsonreuters.upa.rdm.ElementNames;
import com.thomsonreuters.upa.rdm.Login;
import com.thomsonreuters.upa.rdm.UpdateEventTypes;
import com.thomsonreuters.upa.transport.TransportBuffer;
import com.thomsonreuters.upa.transport.WriteArgs;
import com.thomsonreuters.upa.transport.WriteFlags;

public class Module_3_Directory
{
    static UInt serviceDiscoveryInfo_serviceId = CodecFactory.createUInt();

    static boolean serviceDiscoveryInfo_serviceNameFound = false;
    static boolean serviceDiscoveryInfo_upalDMTDictionarySupported = false;
    static boolean serviceDiscoveryInfo_upaDMTMarketPriceSupported = false;
    static boolean serviceDiscoveryInfo_RDMFieldDictionaryProvided = false;
    static boolean serviceDiscoveryInfo_enumtypeProvided = false;
    static int serviceDiscoveryInfo_ServiceState;
    static int serviceDiscoveryInfo_AcceptingRequests;
    static List<Qos> serviceDiscoveryInfo_qoS = new ArrayList<Qos>();

    static String serviceDiscoveryInfo_serviceName;

    /* dictionary file name  */
    final static String fieldDictionaryFileName = "RDMFieldDictionary";
    /* dictionary download name */
    final static String dictionaryDownloadName = "RWFFld";

    /* enum table file name */
    final static String enumTypeDictionaryFileName = "enumtype.def";
    /* enum table download name */
    final static String enumTableDownloadName = "RWFEnum";

    public static void main(String[] args)
    {
        /**************************************************************************************************
         * DECLARING VARIABLES
         **************************************************************************************************/
        int retCode;

        /* Create error to keep track of any errors that may occur in Transport methods */
        Error error = TransportFactory.createError();

        /* Create and populate connect options to specify connection preferences */
        ConnectOptions cOpts = TransportFactory.createConnectOptions();

        /* InProgInfo Information for the In Progress Connection State */
        InProgInfo inProgInfo = TransportFactory.createInProgInfo();

        /*  Channel Info returned by GetChannelInfo call */
        ChannelInfo channelInfo = TransportFactory.createChannelInfo();

        TransportBuffer msgBuf = null;

        long currentTime = 0;
        long upaRuntime = 0;
        long runTime = 0;

        /* Iterator used for decoding throughout the application - we can clear it and reuse it instead of recreating it */
        DecodeIterator decodeIter = CodecFactory.createDecodeIterator(); /* the decode iterator is created (typically stack allocated)  */

        /* Create a channel to keep track of connection */
        Channel channel = null;

        /* Create an I/O notification mechanism (a Selector in our case) for our channel */
        Selector selector = null;

        /* Create initialization arguments (InitArgs) to specify any initialization options for Transport */
        InitArgs initArgs = TransportFactory.createInitArgs();

        /* the default option parameters */
        /* connect to server running on same machine */
        String srvrHostname = "localhost";
        /* server is running on port number 14003 */
        String srvrPortNo = "14002";
        /* use default NIC network interface card to bind to for all inbound and outbound data */
        String interfaceName = "";
        /* default service name is "DIRECT_FEED" used in source directory handler */
        String serviceName = "DIRECT_FEED";
        /* use default runTime of 300 seconds */
        runTime = 300;
        /* Create a bit mask to specify what I/O notification operations to keep track of (e.g. READ and CONNECT)*/
        short opMask = 0;

        /* User specifies options such as address, port, and interface from the command line.
         * User can have the flexibility of specifying any or all of the parameters in any order.
         */

        if (args.length > 0)
        {
            int i = 0;

            while (i < args.length)
            {
                if ((args[i].equals("-h")) == true)
                {
                    i += 2;
                    srvrHostname = args[i - 1];
                }
                else if ((args[i].equals("-p")) == true)
                {
                    i += 2;
                    srvrPortNo = args[i - 1];
                }
                else if ((args[i].equals("-i")) == true)
                {
                    i += 2;
                    interfaceName = args[i - 1];
                }
                else if ((args[i].equals("-r")) == true)
                {
                    i += 2;
                    runTime = Integer.parseInt(args[i - 1]);
                }
                else if ((args[i].equals("-s")) == true)
                {
                    i += 2;
                    serviceName = args[i - 1];
                }
                else
                {
                    System.out.printf("Error: Unrecognized option: %s\n\n", args[i]);
                    System.out.printf("Usage: %s or\n%s [-h <SrvrHostname>] [-p <SrvrPortNo>] [-i <InterfaceName>] [-r <runTime>] [-s <ServiceName>]\n", args[0], args[0]);
                    System.exit(TransportReturnCodes.FAILURE);
                }
            }
        }

        /*************************************
         * Step 1) Initialize Transport *
         *************************************/
        /*********************************************************
         * Client/Consumer Application Lifecycle Major Step 1: Initialize UPA
         * Transport using Initialize The first UPA Transport function that an
         * application should call. This creates and initializes internal memory
         * and structures, as well as performing any bootstrapping for
         * underlying dependencies. The Initialize function also allows the user
         * to specify the locking model they want applied to the UPA Transport.
         *********************************************************/
        if (Transport.initialize(initArgs, error) != TransportReturnCodes.SUCCESS)
        {
            System.out.printf("Error (%d) (errno: %d): %s", error.errorId(), error.sysError(), error.text());
            System.exit(TransportReturnCodes.FAILURE);
        }

        currentTime = System.currentTimeMillis();
        upaRuntime = currentTime + runTime * 1000;

        /* Set connect options */
        /* populate connect options, then pass to Connect function -
         * UPA Transport should already be initialized
         */
        /* use standard socket connection */
        cOpts.connectionType(ConnectionTypes.SOCKET); /* (0) Channel is a standard TCP socket connection type */
        cOpts.unifiedNetworkInfo().address(srvrHostname);
        cOpts.unifiedNetworkInfo().serviceName(srvrPortNo);
        cOpts.unifiedNetworkInfo().interfaceName(interfaceName);
        cOpts.blocking(false);
        cOpts.pingTimeout(60);
        cOpts.compressionType(CompressionTypes.NONE);

        cOpts.protocolType(Codec.protocolType());
        cOpts.majorVersion(Codec.majorVersion());
        cOpts.minorVersion(Codec.minorVersion());

        /*****************************************************
         * Step 2) Connect to Server and receive a channel *
         *****************************************************/
        /*********************************************************
         * Client/Consumer Application Life Cycle Major Step 2: Connect using
         * Connect (OS connection establishment handshake) Connect call
         * Establishes an outbound connection, which can leverage standard
         * sockets, HTTP, or HTTPS. Returns an Channel that represents the
         * connection to the user. In the event of an error, NULL is returned
         * and additional information can be found in the Error structure.
         * Connection options are passed in via an ConnectOptions structure.
         *********************************************************/
        if ((channel = Transport.connect(cOpts, error)) == null)
        {
            System.out.printf("Error (%d) (errno: %d): %s\n", error.errorId(), error.sysError(), error.text());
            Transport.uninitialize();
            System.exit(TransportReturnCodes.FAILURE);
        }

        opMask |= SelectionKey.OP_READ;
        opMask |= SelectionKey.OP_CONNECT;

        try
        {
            selector = Selector.open();
        }
        catch (Exception e)
        {
            System.out.printf("Exception %s\n", e.getMessage());
            closeChannelCleanUpAndExit(channel, selector, TransportReturnCodes.FAILURE);
        }

        SelectionKey key = null;
        Set<SelectionKey> selectedKeys = null;
        Iterator<SelectionKey> keyIter = null;

        /*****************************************************************************************
         * Loop 1) Keep calling Channel.init() until it has properly connected
         * us to the server *
         *****************************************************************************************/
        /* Main loop for getting connection active and successful completion of the initialization process
         * The loop calls select() to wait for notification
         * Currently, the main loop would exit if an error condition is triggered or
         * Channel.state() transitions to ChannelState.ACTIVE.
         */

        /*
         *If we want a non-blocking read call to the selector, we use select before read as read is a blocking call but select is not
         *If we want a blocking read call to the selector, such that we want to wait till we get a message, we should use read without select.
         *In the program below we will use select(), as it is non-blocking
         */
        while (channel.state() != ChannelState.ACTIVE)
        {
            /* Register our channel to the selector and watch for I/O notifications specified by our operation mask */
            try
            {
                channel.selectableChannel().register(selector, opMask, channel);
            }
            catch (Exception e)
            {
                System.out.printf("Exception %s\n", e.getMessage());
                closeChannelCleanUpAndExit(channel, selector, TransportReturnCodes.FAILURE);
            }

            /* Wait 1 second for any I/O notification updates in the channel */
            try
            {
                selector.select(1000);
                /* Create an iterator from the selector's updated channels*/
                selectedKeys = selector.selectedKeys();
                keyIter = selectedKeys.iterator();

                /* If our channel has not updated, we must have timed out */
                if (!keyIter.hasNext())
                {
                    System.out.printf("Channel initialization has timed out.\n");
                    closeChannelCleanUpAndExit(channel, selector, TransportReturnCodes.FAILURE);
                }
                else
                {
                    /* Check if channel is READ-able or CONNECT-able */
                    key = channel.selectableChannel().keyFor(selector);
                    if (key.isReadable() || key.isConnectable())
                    {
                        /****************************************************************************
                         * Step 3) Call Channel.init() to progress channel
                         * initialization further. * * This method is called
                         * multiple times throughout the Loop 1, as it makes *
                         * more progress towards channel initialization. *
                         ***************************************************************************/
                        /* Internally, the UPA initialization process includes several actions. The initialization includes
                         * any necessary UPA connection handshake exchanges, including any HTTP or HTTPS negotiation.
                         * Compression, ping timeout, and versioning related negotiations also take place during the
                         * initialization process. This process involves exchanging several messages across the connection,
                         * and once all message exchanges have completed the Channel.state() will transition. If the connection
                         * is accepted and all types of negotiations completed properly, the Channel.state will become
                         * ChannelState.ACTIVE. If the connection is rejected, either due to some kind of negotiation failure
                         * or because an Server rejected the connection by setting nakMount to true, the Channel.state()
                         * will become ChannelState.CLOSED.
                         *
                         * Note:
                         * For both client and server channels, more than one call to InitChannel can be required to complete
                         * the channel initialization process.
                         */
                        if ((retCode = channel.init(inProgInfo, error)) < TransportReturnCodes.SUCCESS)
                        {
                            System.out.printf("Error (%d) (errno: %d): %s\n", error.errorId(), error.sysError(), error.text());
                            closeChannelCleanUpAndExit(channel, selector, TransportReturnCodes.FAILURE);
                        }

                        /* Deduce an action from return code of Channel.init() */
                        switch (retCode)
                        {
                            case TransportReturnCodes.CHAN_INIT_IN_PROGRESS:
                            {
                                /* Switch to a new channel if required */
                                if (inProgInfo.flags() == InProgFlags.SCKT_CHNL_CHANGE)
                                {
                                    /* The InitChannel function requires the use of an additional parameter, a InProgInfo structure.
                                     * Under certain circumstances, the initialization process may be required to create new or additional underlying connections.
                                     * If this occurs, the application is required to unregister the previous socketId and register the new socketId with
                                     * the I/O notification mechanism being used. When this occurs, the information is conveyed by the InProgInfo and the InProgFlags.
                                     *
                                     * SCKT_CHNL_CHANGE indicates that a socketId change has occurred as a result of this call. The previous socketId has been
                                     * stored in InProgInfo.oldSocket so it can be unregistered with the I/O notification mechanism.
                                     * The new socketId has been stored in InProgInfo.newSocket so it can be registered with the
                                     * I/O notification mechanism. The channel initialization is still in progress and subsequent calls
                                     * to InitChannel are required to complete it.
                                     */
                                    opMask = SelectionKey.OP_READ | SelectionKey.OP_CONNECT;
                                    System.out.printf("Channel switch, NEW: %d OLD %d\n", channel.selectableChannel(), channel.oldSelectableChannel());
                                    try
                                    {
                                        key = inProgInfo.oldSelectableChannel().keyFor(selector);
                                        key.cancel();
                                    }
                                    catch (Exception e)
                                    {
                                        System.out.printf("Exception %s\n", e.getMessage());
                                        closeChannelCleanUpAndExit(channel, selector, TransportReturnCodes.FAILURE);
                                    }
                                    try
                                    {
                                        channel.selectableChannel().register(selector, opMask, channel);
                                    }
                                    catch (Exception e)
                                    {
                                        System.out.printf("Exception %s\n", e.getMessage());
                                        closeChannelCleanUpAndExit(channel, selector, TransportReturnCodes.FAILURE);
                                    }
                                }
                                else
                                {
                                    System.out.printf("Channel init in progress...\n");
                                }
                            }
                                break;
                            /* channel connection becomes active!
                             * Once a connection is established and transitions to the ChannelState.ACTIVE state,
                             * this Channel can be used for other transport operations.
                             */
                            case TransportReturnCodes.SUCCESS:
                            {
                                System.out.printf("Channel is now active!  Reading and writing can begin!\n");

                                /* Populate information from channel */
                                if ((retCode = channel.info(channelInfo, error)) != TransportReturnCodes.SUCCESS)
                                {
                                    System.out.printf("Error (%d) (errno: %d): %s\n", error.errorId(), error.sysError(), error.text());
                                    closeChannelCleanUpAndExit(channel, selector, TransportReturnCodes.FAILURE);
                                }

                                /* Print out basic channel info */
                                System.out.printf("\nChannel Info:\n" + "Max Fragment Size:         %d\n" + "Max Output Buffers:        %d, %d  Guaranteed\n" + "Input Buffers:             %d\n" + "Send/Receive Buffer Sizes: %d/%d\n" + "Ping Timeout:              %d\n",
                                                  channelInfo.maxFragmentSize(), /*  This is the max fragment size before fragmentation and reassembly is necessary. */
                                                  channelInfo.maxOutputBuffers(), /*  This is the maximum number of output buffers available to the channel. */
                                                  channelInfo.guaranteedOutputBuffers(), /*  This is the guaranteed number of output buffers available to the channel. */
                                                  channelInfo.numInputBuffers(), /*  This is the number of input buffers available to the channel. */
                                                  channelInfo.sysSendBufSize(), /*  This is the systems Send Buffer size. This reports the systems send buffer size respective to the transport type being used (TCP, UDP, etc) */
                                                  channelInfo.sysRecvBufSize(), /*  This is the systems Receive Buffer size. This reports the systems receive buffer size respective to the transport type being used (TCP, UDP, etc) */
                                                  channelInfo.pingTimeout()); /*  This is the value of the negotiated ping timeout */

                                int count = channelInfo.componentInfo().size();
                                if (count == 0)
                                    System.out.printf("(No component info)");
                                else
                                {
                                    for (int i = 0; i < count; ++i)
                                    {
                                        System.out.println(channelInfo.componentInfo().get(i).componentVersion());
                                        if (i < count - 1)
                                            System.out.printf(", ");
                                    }
                                }

                            }
                                break;
                            default:
                            {
                                System.out.printf("Unexpected return value\n");
                                closeChannelCleanUpAndExit(channel, selector, TransportReturnCodes.FAILURE);
                            }
                                break;
                        }
                    }
                }
            }
            catch (IOException e1)
            {
                e1.printStackTrace();
            }
        }

        /* Initialize ping management handler */
        initPingManagement(channel);

        /* Keep track of READ notifications */
        opMask |= SelectionKey.OP_READ;

        if ((retCode = sendLoginRequest(channel, channelInfo.maxFragmentSize())) > TransportReturnCodes.SUCCESS)
        {
            opMask |= SelectionKey.OP_WRITE;
        }
        else if (retCode < TransportReturnCodes.SUCCESS)
        {
            System.out.printf("Error (%d) (errno: %d): %s\n", error.errorId(), error.sysError(), error.text());
            closeChannelCleanUpAndExit(channel, selector, TransportReturnCodes.FAILURE);
        }

        /****************************************************
         * Loop 2) Check ping operations until runtime ends *
         ****************************************************/
        /* Here were are using a new Main loop. An alternative design would be to combine this Main loop with
         * the Main loop for getting connection active. Some bookkeeping would be required for that approach.
         */
        /* Add two functions at the end compared to module_1a:initPingManagement() and
        * processPingManagement() */

        /* Main loop for message processing (reading data, writing data, and ping management, etc.)
         * The loop calls select() to wait for notification
         * Currently, the only way to exit this Main loop is when an error condition is triggered or after
         * a predetermined run-time has elapsed.
         */
        while (true)
        {
            /* Register our channel to the selector and watch for I/O notifications specified by our operation mask */
            try
            {
                channel.selectableChannel().register(selector, opMask, channel);
            }
            catch (Exception e)
            {
                System.out.printf("Exception %s\n", e.getMessage());
                closeChannelCleanUpAndExit(channel, selector, TransportReturnCodes.FAILURE);
            }

            /* Wait for any I/O notification updates in the channel */
            try
            {
                selector.select(1000);
                selectedKeys = selector.selectedKeys();
                keyIter = selectedKeys.iterator();

                /* If our channel has been updated */
                if (keyIter.hasNext())
                {
                    /* Check if channel is READ-able */
                    key = channel.selectableChannel().keyFor(selector);
                    if (key.isReadable())
                    {
                        /* Initialize to a positive value for retCode in case we have more data that is available to read */
                        retCode = 1;

                        /******************************************************
                         * Loop 3) Read and decode for all buffers in channel *
                         ******************************************************/
                        while (retCode > TransportReturnCodes.SUCCESS)
                        {
                            /* Create read arguments (ReadArgs) to keep track of the statuses of any reads */
                            ReadArgs readArgs = TransportFactory.createReadArgs();

                            /**************************************************
                             * Step 4) Read message from channel into buffer *
                             **************************************************/
                            /*********************************************************
                             * Client/Consumer Application Life Cycle Major Step
                             * 4: Read using Read The examples for executing
                             * read and write will be shown in module_1c. Read
                             * provides the user with data received from the
                             * connection. This function expects the Channel to
                             * be in the active state. When data is available,
                             * an Buffer referring to the information is
                             * returned, which is valid until the next call to
                             * Read. A return code parameter passed into the
                             * function is used to convey error information as
                             * well as communicate whether there is additional
                             * information to read. An I/O notification
                             * mechanism may not inform the user of this
                             * additional information as it has already been
                             * read from the socket and is contained in the Read
                             * input buffer.
                             *********************************************************/
                            msgBuf = channel.read(readArgs, error);

                            if (msgBuf != null)
                            {
                                /* if a buffer is returned, we have data to process and code is success */

                                /* Processes a response from the channel/connection. This consists of performing a high level decode of the message and then
                                 * calling the applicable specific function for further processing.
                                 */

                                /* No need to clear the message before we decode into it. UPA Decoding populates all message members (and that is true for any
                                 * decoding with UPA, you never need to clear anything but the iterator)
                                 */
                                /* We have data to process */
                                /* Create message to represent buffer data */
                                Msg msg = CodecFactory.createMsg();

                                /* This decodeIter.clear() clear iterator function should be used to achieve the best performance while clearing the iterator. */
                                /* Clears members necessary for decoding and readies the iterator for reuse. You must clear DecodeIterator
                                 * before decoding content. For performance purposes, only those members required for proper functionality are cleared.
                                 */
                                decodeIter.clear();
                                /* Set the RWF version to decode with this iterator */
                                /* Associates the DecodeIterator with the Buffer from which to decode. */
                                if ((retCode = decodeIter.setBufferAndRWFVersion(msgBuf, channel.majorVersion(), channel.minorVersion())) != TransportReturnCodes.SUCCESS)
                                {
                                    System.out.printf("\nSetDecodeIteratorBuffer() failed with return code: %d\n", retCode);
                                    /* Closes channel, cleans up and exits the application. */
                                    closeChannelCleanUpAndExit(channel, selector, TransportReturnCodes.FAILURE);
                                }
                                /* decode contents into the Msg structure */
                                if ((retCode = msg.decode(decodeIter)) != CodecReturnCodes.SUCCESS)
                                {
                                    System.out.printf("Error (%d) (errno: %d): %s\n", error.errorId(), error.sysError(), error.text());
                                    closeChannelCleanUpAndExit(channel, selector, TransportReturnCodes.FAILURE);
                                }
                                /*After acquiring data in buffer, we can check what kind of information it is. It may be login message, update
                                *or other info. This part will be fulfilled in latter examples like module of login and source directory. */

                                /* Deduce an action based on the domain type of the message */
                                switch (msg.domainType())
                                {
                                    case DomainTypes.LOGIN:
                                    {
                                        /* (1) Login Message */
                                        /* main difference with last module_1c_read_and_write
                                        * add sendMessage(), sendLoginRequest(),processLoginResponse(), closeLoginStream()
                                        * and upaGetBuffer() functions at the end.*/
                                        if (processLoginResponse(msg, decodeIter) != TransportReturnCodes.SUCCESS)
                                        {
                                            /* Login Failed and the application is denied - Could be one of the following 3 possibilities:
                                             *
                                             * - CLOSED_RECOVER (Stream State): (3) Closed, the applications may attempt to re-open the stream later
                                             *   (can occur via either an RefreshMsg or an StatusMsg), OR
                                             *
                                             * - CLOSED (Stream State): (4) Closed (indicates that the data is not available on this service/connection
                                             *   and is not likely to become available), OR
                                             *
                                             * - SUSPECT (Data State): (2) Data is Suspect (similar to a stale data state, indicates that the health of
                                             *	 some or all data associated with the stream is out of date or cannot be confirmed that it is current)
                                             */

                                            /* Closes channel, cleans up and exits the application. */
                                            closeChannelCleanUpAndExit(channel, selector, TransportReturnCodes.FAILURE);
                                        }
                                        else
                                        {
                                            System.out.printf("UPA Consumer application is granted access and has logged in successfully.\n\n");
                                            serviceDiscoveryInfo_serviceName = serviceName;
                                            /* After it is granted access, those common daily activities and requesting could be done here.
                                             * But first we need the source directory.*/
                                            /* Send Source Directory request message */

                                            if ((retCode = sendSourceDirectoryRequest(channel, channelInfo.maxFragmentSize())) > TransportReturnCodes.SUCCESS)
                                            {
                                                /* There is still data left to flush, leave our write notification enabled so we get called again.
                                                 * If everything wasn't flushed, it usually indicates that the TCP output buffer cannot accept more yet
                                                 */
                                                /* set write fd if there's still other data queued */
                                                /* flush is done by application */
                                            }
                                            else if (retCode < TransportReturnCodes.SUCCESS)
                                            {
                                                /* Closes channel, cleans up and exits the application. */
                                                closeChannelCleanUpAndExit(channel, selector, TransportReturnCodes.FAILURE);
                                            }

                                        }
                                    }
                                        break;
                                    /* (4) Source Message */
                                    case DomainTypes.SOURCE:
                                    {
                                        if (processSourceDirectoryResponse(channel, msg, error, decodeIter, selector) != TransportReturnCodes.SUCCESS)
                                        {
                                            /* Closes channel, cleans up and exits the application. */
                                            closeChannelCleanUpAndExit(channel, selector, TransportReturnCodes.FAILURE);
                                        }
                                        else
                                        {
                                            System.out.printf("UPA Consumer application has successfully received source directory information.\n\n");
                                        }

                                        /* exit app if service name entered by user cannot be found */
                                        if (!serviceDiscoveryInfo_serviceNameFound)
                                        {
                                            System.out.printf("\nSource directory response does not contain service name: %s. Exit app.\n", serviceName);
                                            /* Closes channel, cleans up and exits the application. */
                                            closeChannelCleanUpAndExit(channel, selector, TransportReturnCodes.FAILURE);
                                        }

                                        /* exit app if service we care about is NOT up and accepting requests */
                                        if ((serviceDiscoveryInfo_ServiceState != Directory.ServiceStates.UP) || (serviceDiscoveryInfo_AcceptingRequests == 0))
                                        {
                                            System.out.printf("\nService name: %s is NOT up and accepting requests. Exit app.\n", serviceName);
                                            /* Closes channel, cleans up and exits the application. */
                                            closeChannelCleanUpAndExit(channel, selector, TransportReturnCodes.FAILURE);
                                        }
                                    }
                                        break;
                                    default:
                                    {
                                        System.out.printf("Unhandled Domain Type: %d\n", msg.domainType());
                                    }
                                        break;
                                }

                                /* Acknowledge that a ping has been received */
                                receivedServerMsg = true;
                                System.out.printf("Ping message has been received successfully from the server due to data message ...\n\n");
                            }
                            else
                            {
                                /* Deduce an action from the return code of Channel.read() */
                                retCode = readArgs.readRetVal();
                                switch (retCode)
                                {
                                    /* Acknowledge that a ping has been received */
                                    case TransportReturnCodes.READ_PING:
                                        receivedServerMsg = true;
                                        System.out.printf("Ping message has been received successfully from the server due to ping message ...\n\n");
                                        break;

                                    /* Switch to a new channel if required */
                                    case TransportReturnCodes.READ_FD_CHANGE:
                                        opMask = SelectionKey.OP_READ;
                                        System.out.printf("Channel switch, NEW: %d OLD %d\n", channel.selectableChannel(), channel.oldSelectableChannel());
                                        try
                                        {
                                            key = channel.selectableChannel().keyFor(selector);
                                            key.cancel();
                                        }
                                        catch (Exception e)
                                        {
                                            System.out.printf("Exception %s\n", e.getMessage());
                                            closeChannelCleanUpAndExit(channel, selector, TransportReturnCodes.FAILURE);
                                        }
                                        try
                                        {
                                            channel.selectableChannel().register(selector, opMask, channel);
                                        }
                                        catch (Exception e)
                                        {
                                            System.out.printf("Exception %s\n", e.getMessage());
                                            closeChannelCleanUpAndExit(channel, selector, TransportReturnCodes.FAILURE);
                                        }
                                        break;

                                    case TransportReturnCodes.READ_WOULD_BLOCK:
                                        /* Nothing to read */
                                        break;
                                    case TransportReturnCodes.READ_IN_PROGRESS:
                                        /* Reading from multiple threads */
                                        break;
                                    case TransportReturnCodes.INIT_NOT_INITIALIZED:
                                    case TransportReturnCodes.FAILURE:
                                        System.out.printf("Error (%d) (errno: %d) %s\n", error.errorId(), error.sysError(), error.text());
                                        closeChannelCleanUpAndExit(channel, selector, TransportReturnCodes.FAILURE);
                                    default:
                                        if (retCode < 0)
                                        {
                                            System.out.printf("Error (%d) (errno: %d) encountered with Read. Error Text: %s\n", error.errorId(), error.sysError(), error.text());
                                            closeChannelCleanUpAndExit(channel, selector, TransportReturnCodes.FAILURE);
                                        }
                                }
                            }
                        }
                    }

                    key = channel.selectableChannel().keyFor(selector);
                    if (key.isWritable())
                    {
                        retCode = TransportReturnCodes.FAILURE;

                        if ((retCode = channel.flush(error)) > TransportReturnCodes.SUCCESS)
                        {
                            /* There is still data left to flush, leave our write notification enabled so we get called again.
                             * If everything wasn't flushed, it usually indicates that the TCP output buffer cannot accept more yet 
                             */
                        }
                        else
                        {
                            switch (retCode)
                            {
                                case TransportReturnCodes.SUCCESS:
                                {
                                    /* Everything has been flushed, no data is left to send - unset/clear write fd notification */
                                    key = channel.selectableChannel().keyFor(selector);
                                    try
                                    {
                                        channel.selectableChannel().register(selector, key.interestOps() - SelectionKey.OP_WRITE, channel);
                                    }
                                    catch (ClosedChannelException e)
                                    {
                                        e.printStackTrace();
                                    }
                                }
                                    break;
                                case TransportReturnCodes.FAILURE:
                                default:
                                {
                                    System.out.printf("Error (%d) (errno: %d): %s\n", error.errorId(), error.sysError(), error.text());
                                    closeChannelCleanUpAndExit(channel, selector, TransportReturnCodes.FAILURE);
                                }
                            }
                        }
                    }
                }

                /* Processing ping management handler */
                if ((retCode = processPingManagementHandler(channel, opMask, selector)) > TransportReturnCodes.SUCCESS)
                {
                    /* There is still data left to flush, leave our write notification enabled so we get called again.
                     * If everything wasn't flushed, it usually indicates that the TCP output buffer cannot accept more yet
                     */

                    /* set write fd if there's still other data queued */
                    /* flush is done by application */
                    opMask |= SelectionKey.OP_WRITE;
                }
                else if (retCode < TransportReturnCodes.SUCCESS)
                {
                    /* Closes channel, cleans up and exits the application. */
                    closeChannelCleanUpAndExit(channel, selector, TransportReturnCodes.FAILURE);
                }

                /* get current time */
                currentTime = System.currentTimeMillis();

                /* If the runtime has expired */
                if (System.currentTimeMillis() >= upaRuntime)
                {
                    if ((retCode = closeLoginStream(channel, channelInfo.maxFragmentSize())) != TransportReturnCodes.SUCCESS)
                    {
                        /* When you close login, we want to make a best effort to get this across the network as it will gracefully
                         * close all open streams. If this cannot be flushed or failed, this application will just close the connection
                         * for simplicity.
                         */

                        /* Closes channel, cleans up and exits the application. */
                        closeChannelCleanUpAndExit(channel, selector, TransportReturnCodes.FAILURE);
                    }
                    else if (retCode > TransportReturnCodes.SUCCESS)
                    {
                        /* Flush before exiting */
                        key = channel.selectableChannel().keyFor(selector);
                        if (key.isWritable())
                        {
                            retCode = 1;

                            /******************************************************
                             * Loop 4) Flush all remaining buffers to channel *
                             ******************************************************/
                            while (retCode > TransportReturnCodes.SUCCESS)
                            {
                                retCode = channel.flush(error);
                            }
                            if (retCode < TransportReturnCodes.SUCCESS)
                            {
                                System.out.printf("Flush has failed with return code %d - <%s>\n", retCode, error.text());
                            }
                        }
                    }

                    System.out.printf("UPA Client run-time has expired...\n\n");
                    closeChannelCleanUpAndExit(channel, selector, TransportReturnCodes.SUCCESS);
                }
            }
            catch (IOException e1)
            {
                e1.printStackTrace();
            }
        }
    }

    /*********************************************************
     * Closes channel and selector and exits application. *
     * 
     * @param channel - Channel to be closed *
     * @param selector - Selector to be closed *
     * @param code - if exit is due to errors/exceptions *
     *********************************************************/
    public static void closeChannelCleanUpAndExit(Channel channel, Selector selector, int code)
    {
        Error error = TransportFactory.createError();
        /*********************************************************
         * Client/Consumer Application Lifecycle Major Step 5: Close connection
         * using CloseChannel (OS connection release handshake) CloseChannel
         * closes the client based Channel. This will release any pool based
         * resources back to their respective pools, close the connection, and
         * perform any additional necessary cleanup.
         *********************************************************/

        try
        {
            selector.close();
        }
        catch (Exception e)
        {
            System.out.printf("Exception %s\n", e.getMessage());
        }

        if ((channel != null) && channel.close(error) < TransportReturnCodes.SUCCESS)
        {
            System.out.printf("Error (%d) (errno: %d): %s\n", error.errorId(), error.sysError(), error.text());
        }

        /*********************************************************
         * Client/Consumer Application Lifecycle Major Step 6: Uninitialize UPA
         * Transport using Uninitialize The last UPA Transport function that an
         * application should call. This uninitializes internal data structures
         * and deletes any allocated memory.
         *********************************************************/

        /* All UPA Transport use is complete, must uninitialize.
         * The uninitialization process allows for any heap allocated memory to be cleaned up properly.
         */
        Transport.uninitialize();

        if (code == TransportReturnCodes.SUCCESS)
        {
            System.out.printf("\nUPA Consumer Training Application successfully ended.\n");
        }

        System.exit(code);
    }

    /*
     * Initializes the ping times for upaChannel.
     * upaChannel - The channel for ping management info initialization
     */
    static int pingTimeoutServer; /* server ping timeout */
    static int pingTimeoutClient; /* client ping timeout */
    static long nextReceivePingTime; /* time client should receive next message/ping from server */
    static long nextSendPingTime; /* time to send next ping from client */
    static boolean receivedServerMsg; /* flag for server message received */

    public static void initPingManagement(Channel channel)
    {
        /* get current time */
        long currentTime = System.currentTimeMillis();

        /* set ping timeout for local and remote pings */
        pingTimeoutClient = channel.pingTimeout() / 3;
        pingTimeoutServer = channel.pingTimeout();

        /* set time to send next ping to remote connection */
        nextSendPingTime = currentTime + pingTimeoutClient * 1000;

        /* set time should receive next ping from remote connection */
        nextReceivePingTime = currentTime + pingTimeoutServer * 1000;
    }

    /*
     * Processing ping management handler
     * upaChannel - The channel for ping management processing
     */
    public static int processPingManagementHandler(Channel channel, Short opMask, Selector selector)
    {
        /* Handles the ping processing for upaChannel. Sends a ping to the server if the next send ping time has arrived and
         * checks if a ping has been received from the server within the next receive ping time.
         */
        int retval = TransportReturnCodes.SUCCESS;
        Error error = TransportFactory.createError();

        /* get current time */
        long currentTime = System.currentTimeMillis();

        /* handle client pings */
        if (currentTime >= nextSendPingTime)
        {
            /* send ping to server */
            /*********************************************************
             * Client/Consumer Application Life Cycle Major Step 4: Ping using
             * Ping Attempts to write a heartbeat message on the connection.
             * This function expects the Channel to be in the active state. If
             * an application calls the Ping function while there are other
             * bytes queued for output, the UPA Transport layer will suppress
             * the heartbeat message and attempt to flush bytes to the network
             * on the user's behalf.
             *********************************************************/

            /* Ping use - this demonstrates sending of heartbeats */
            if ((retval = channel.ping(error)) > TransportReturnCodes.SUCCESS)
            {
                /* Indicates that queued data was sent as a heartbeat and there is still information internally queued by the transport.
                 * The Flush function must be called to continue attempting to pass the queued bytes to the connection. This information may
                 * still be queued because there is not sufficient space in the connections output buffer.
                 * An I/O notification mechanism can be used to indicate when the socketId has write availability.
                 *
                 * There is still data left to flush, leave our write notification enabled so we get called again.
                 * If everything wasn't flushed, it usually indicates that the TCP output buffer cannot accept more yet
                 */

                /* flush needs to be done by application */
            }
            else
            {
                switch (retval)
                {
                    case TransportReturnCodes.SUCCESS:
                    {
                        /* Ping message has been sent successfully */
                        System.out.printf("Ping message has been sent successfully to the server ...\n\n");
                    }
                        break;
                    case TransportReturnCodes.FAILURE: /* fall through to default. */
                    default: /* Error handling */
                    {
                        System.out.printf("Error (%d) (errno: %d) encountered with Ping(). Error Text:%s\n", error.errorId(), error.sysError(), error.text());
                        closeChannelCleanUpAndExit(channel, selector, TransportReturnCodes.FAILURE);
                        /* Closes channel/connection, cleans up and exits the application. */
                        return TransportReturnCodes.FAILURE;
                    }
                }
            }

            /* set time to send next ping from client */
            nextSendPingTime = currentTime + (pingTimeoutClient * 1000);
        }

        /* handle server pings - an application should determine if data or pings have been received,
         * if not application should determine if pingTimeout has elapsed, and if so connection should be closed
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
                /* Lost contact with remote (connection) */
                error.text("Lost contact with connection...\n");
                System.out.printf("Error (%d) (errno: %d) %s\n", error.errorId(), error.sysError(), error.text());
                closeChannelCleanUpAndExit(channel, selector, TransportReturnCodes.FAILURE);

            }
        }
        return retval;
    }

    /**************************************************************
     * Sends a message buffer to the channel *
     * 
     * @param channel - the Channel to send the message buffer to *
     * @param msgBuf - the buffer to be sent *
     * @return status code *
     **************************************************************/
    public static int sendMessage(Channel channel, TransportBuffer msgBuf)
    {
        Error error = TransportFactory.createError();
        int retCode = 0;
        WriteArgs writeArgs = TransportFactory.createWriteArgs();

        writeArgs.flags(WriteFlags.NO_FLAGS);

        /* send the request */

        /*********************************************************
         * Client/Consumer Application Life Cycle Major Step 4: Write using
         * Writer Writer performs any writing or queuing of data. This function
         * expects the channel to be in the active state and the buffer to be
         * properly populated, where length reflects the actual number of bytes
         * used. This function allows for several modifications to be specified
         * for this call. Here we use WriteFlags.NO_FLAGS. For more information
         * on other flag enumeration such as WriteFlags.DO_NOT_COMPRESS or
         * WriteFlags.DIRECT_SOCKET_WRITE, see the UPA C developers guide for
         * Write Flag Enumeration Values supported by UPA Transport.
         *
         * The UPA Transport also supports writing data at different priority
         * levels. The application can pass in two integer values used for
         * reporting information about the number of bytes that will be written.
         * The uncompressedBytesWritten parameter will return the number of
         * bytes to be written, including any transport header overhead but not
         * taking into account any compression. The bytesWritten parameter will
         * return the number of bytes to be written, including any transport
         * header overhead and taking into account any compression. If
         * compression is disabled, uncompressedBytesWritten and bytesWritten
         * should match. The number of bytes saved through the compression
         * process can be calculated by (bytesWritten -
         * uncompressedBytesWritten). Note: Before passing a buffer to Write, it
         * is required that the application set length to the number of bytes
         * actually used. This ensures that only the required bytes are written
         * to the network.
         *********************************************************/

        /* Now write the data - keep track of UPA Transport return code -
         * Because positive values indicate bytes left to write, some negative transport layer return codes still indicate success
         */

        /* this example writes buffer as high priority and no write modification flags */
        if ((retCode = channel.write(msgBuf, writeArgs, error)) == TransportReturnCodes.WRITE_CALL_AGAIN)
        {
            /* (-10) Transport Success: channel.write is fragmenting the buffer and needs to be called again with the same buffer. This indicates that Write was
             * unable to send all fragments with the current call and must continue fragmenting
             */

            /* Large buffer is being split by transport, but out of output buffers. Schedule a call to channel.flush() and then call the channel.write function again with
             * this same exact buffer to continue the fragmentation process. Only release the buffer if not passing it to Write again. */

            /* call flush and write again - breaking out if the return code is something other than TransportReturnCodes.WRITE_CALL_AGAIN (write call again) */
            while (retCode == TransportReturnCodes.WRITE_CALL_AGAIN)
            {
                if ((retCode = channel.flush(error)) < TransportReturnCodes.SUCCESS)
                {
                    System.out.printf("Flush has failed with return code %d - <%s>\n", retCode, error.text());
                }
                retCode = channel.write(msgBuf, writeArgs, error);
            }

        }

        if (retCode > TransportReturnCodes.SUCCESS)
        {
            /* The write was successful and there is more data queued in UPA Transport. The Channel.flush() method should be used to continue attempting to flush data 
             * to the connection. UPA will release buffer.
             */

            /* Flush needs to be done by application */
        }
        else
        {
            switch (retCode)
            {
                case TransportReturnCodes.SUCCESS:
                {
                    /* Successful write and all data has been passed to the connection */
                    /* Continue with next operations. UPA will release buffer.*/
                }
                    break;
                case TransportReturnCodes.WRITE_FLUSH_FAILED:
                {
                    if (channel.state() == ChannelState.CLOSED)
                    {
                        /* Channel is Closed - This is terminal. Treat as and buffer must be released - fall through to default. */
                    }
                    else
                    {
                        /* Channel.write() internally attempted to flush data to the connection but was blocked. This is not a failure and the user should not release their buffer."; 
                        /* Successful write call, data is queued. The Channel.flush() method should be used to continue attempting to flush data to the connection. */

                        /* Set write fd if flush failed */
                        /* Flush needs to be done by application */

                        /* Channel is still open, but Channel.write() tried to flush internally and failed. 
                         * Return positive value so the caller knows there's bytes to flush. 
                         */
                        return TransportReturnCodes.SUCCESS + 1;
                    }
                }
                case TransportReturnCodes.NO_BUFFERS:
                {
                    channel.releaseBuffer(msgBuf, error);
                }
                    break;
                case TransportReturnCodes.FAILURE:
                default:
                {
                    System.out.printf("Error (%d) (errno: %d): %s\n", error.errorId(), error.sysError(), error.text());
                    channel.releaseBuffer(msgBuf, error);
                    return TransportReturnCodes.FAILURE;
                }
            }
        }

        return retCode;
    }

    final public static int LOGIN_STREAM_ID = 1;
    
    /**************************************************************
     * Sends login request message to a channel *
     * 
     * @param channel - the Channel of connection *
     * @param maxFragmentSize - the maximum fragment size before fragmentation *
     * @return status code *
     **************************************************************/

    public static int sendLoginRequest(Channel channel, int maxFragmentSize)
    {
        int retCode;
        Error error = TransportFactory.createError();
        TransportBuffer msgBuf = null;

        /* Populate and encode a requestMsg */
        RequestMsg reqMsg = (RequestMsg)CodecFactory.createMsg();

        /* UPA provides clear functions for its structures (e.g., encodeIterator.clear()). 
         * These functions are tuned to be efficient 
         * and avoid initializing unnecessary
         * structure members, and allow for optimal structure use and reuse. 
         */
        /* Iterator used for encoding throughout the application - we can clear it and reuse it instead of recreating it */
        EncodeIterator encIter = CodecFactory.createEncodeIterator();/* the encode iterator is created (typically stack allocated)  */

        ElementList elementList = CodecFactory.createElementList();
        ElementEntry elementEntry = CodecFactory.createElementEntry();

        Buffer applicationId = CodecFactory.createBuffer();
        Buffer applicationName = CodecFactory.createBuffer();
        UInt applicationRole = CodecFactory.createUInt();

        elementList.clear();
        elementEntry.clear();

        String userName = "put userName here";
        Buffer userNameBuf = CodecFactory.createBuffer();

        /* Get a buffer of the channel max fragment size */
        /* Obtains a non-packable buffer of the requested size from the UPA Transport guaranteed buffer pool to write into for the Login request.
         * For an outbound TransportBuffer (specifically one gotten from Channel.getBuffer(), like this one) length() initially does indeed indicate the number of bytes available, 
         * but when the ByteBuffer's position is after the start 
         * (because the application has written something, i.e. encoded some or all of a Login request message), 
         * it actually returns the number of bytes between the start & current positions 
         * (basically, the number of bytes encoded thus far).
         */
        /* upaGetBuffer() is the utility function that does 2-pass (more robust) getting non-packable buffer. */
        if ((msgBuf = upaGetBuffer(channel, maxFragmentSize, error)) == null)
        {
            /* Connection should be closed, return failure */
            /* Closes channel, cleans up and exits the application. */
            return TransportReturnCodes.FAILURE;
        }
        /* if a buffer is returned, we can populate and write, encode an Msg into the buffer */

        /* Encodes the Login request. */

        reqMsg.clear();

        /* clear encode iterator for reuse - this should be used to achieve the best performance while clearing the iterator. */
        encIter.clear();
        /* set version information of the connection on the encode iterator so proper versioning can be performed */
        /* set the buffer on an EncodeIterator */
        if ((retCode = encIter.setBufferAndRWFVersion(msgBuf, channel.majorVersion(), channel.minorVersion())) < TransportReturnCodes.SUCCESS)
        {
            channel.releaseBuffer(msgBuf, error);
            System.out.printf("\nSetEncodeIteratorBuffer() failed for Login Request with return code: %d\n", retCode);
            /* Closes channel, cleans up and exits the application. */
            return TransportReturnCodes.FAILURE;
        }
        /* Create and initialize request message */
        reqMsg.msgClass(MsgClasses.REQUEST); /* (1) Request Message */
        reqMsg.streamId(LOGIN_STREAM_ID); /* StreamId */

        reqMsg.domainType(DomainTypes.LOGIN); /* (1) Login Message */
        reqMsg.containerType(DataTypes.NO_DATA); /* (128) No Data */
        /* The initial Login request must be streaming (i.e., a RequestMsgFlags.STREAMING flag is required). */
        reqMsg.flags(RequestMsgFlags.STREAMING);
        /* set msgKey members */
        reqMsg.msgKey().flags(MsgKeyFlags.HAS_ATTRIB | MsgKeyFlags.HAS_NAME_TYPE | MsgKeyFlags.HAS_NAME);

        /* Username */
        userNameBuf.data(userName);

        if ((userName = System.getProperty("user.name")) != null)
        {
            reqMsg.msgKey().name().data(userName);
        }
        else
        {
            reqMsg.msgKey().name().data("Unknown");
        }

        reqMsg.msgKey().nameType(Login.UserIdTypes.NAME); /* (1) Name */
        /* (133) Element List container type, used to represent content containing element name, dataType, and value triples.    <BR>*/
        reqMsg.msgKey().attribContainerType(DataTypes.ELEMENT_LIST);
        /* encode message */

        /* since our msgKey has opaque that we want to encode, we need to use encodeInit */
        /* encodeInit should return and inform us to encode our key opaque */
        if ((retCode = reqMsg.encodeInit(encIter, 0)) < CodecReturnCodes.SUCCESS)
        {
            return TransportReturnCodes.FAILURE;
        }
        /* encode our msgKey opaque */

        /* encode the element list */
        elementList.clear();

        elementList.flags(ElementListFlags.HAS_STANDARD_DATA);/* (0x08) The ElementList contains standard encoded content (e.g. not set defined).  */

        /* Begins encoding of an ElementList. */

        if ((retCode = elementList.encodeInit(encIter, null, 0)) < CodecReturnCodes.SUCCESS)
        {
            return TransportReturnCodes.FAILURE;
        }
        /* ApplicationId */

        applicationId.data("256");

        elementEntry.dataType(DataTypes.ASCII_STRING);
        elementEntry.name(ElementNames.APPID);

        if ((retCode = elementEntry.encode(encIter, applicationId)) < CodecReturnCodes.SUCCESS)
        {
            channel.releaseBuffer(msgBuf, error);
            System.out.printf("elementEntry.encode() failed for Login Request with return code: %d\n", retCode);
            /* Closes channel, cleans up and exits the application. */
            return TransportReturnCodes.FAILURE;
        }
        /* ApplicationName */
        applicationName.data("UPA Consumer Training");

        elementEntry.dataType(DataTypes.ASCII_STRING);
        elementEntry.name(ElementNames.APPNAME);

        if ((retCode = elementEntry.encode(encIter, applicationName)) < CodecReturnCodes.SUCCESS)
        {
            channel.releaseBuffer(msgBuf, error);
            System.out.printf("elementEntry.encode() failed for Login Request with return code: %d\n", retCode);
            /* Closes channel, cleans up and exits the application. */
            return TransportReturnCodes.FAILURE;
        }

        /* Role */
        elementEntry.dataType(DataTypes.UINT);
        elementEntry.name(ElementNames.ROLE);

        /* (1) Application logs in as a provider */
        applicationRole.value(Login.RoleTypes.CONS);

        if ((retCode = elementEntry.encode(encIter, applicationRole)) < CodecReturnCodes.SUCCESS)
        {
            channel.releaseBuffer(msgBuf, error);
            System.out.printf("elementEntry.encode() failed for Login Request with return code: %d\n", retCode);
            /* Closes channel, cleans up and exits the application. */
            return TransportReturnCodes.FAILURE;
        }

        /* complete encode element list */
        if ((retCode = elementList.encodeComplete(encIter, true)) < CodecReturnCodes.SUCCESS)
        {
            channel.releaseBuffer(msgBuf, error);
            System.out.printf("elementList.encodeComplete() failed for Login Request with return code: %d\n", retCode);
            /* Closes channel, cleans up and exits the application. */
            return TransportReturnCodes.FAILURE;
        }
        /* complete encode key */
        /* EncodeMsgKeyAttribComplete finishes our key opaque, so it should return and indicate
         * for us to encode our container/msg payload
         */
        if ((retCode = reqMsg.encodeKeyAttribComplete(encIter, true)) < CodecReturnCodes.SUCCESS)
        {
            channel.releaseBuffer(msgBuf, error);
            System.out.printf("reqMsg.encodeKeyAttribComplete() failed for Login Request with return code: %d\n", retCode);
            /* Closes channel, cleans up and exits the application. */
            return TransportReturnCodes.FAILURE;
        }
        /* complete encode message */
        if ((retCode = reqMsg.encodeComplete(encIter, true)) < CodecReturnCodes.SUCCESS)
        {
            channel.releaseBuffer(msgBuf, error);
            System.out.printf("reqMsg.encodeComplete() failed for Login Request with return code: %d\n", retCode);
            /* Closes channel, cleans up and exits the application. */
            return TransportReturnCodes.FAILURE;
        }

        /* send login request */
        if ((retCode = sendMessage(channel, msgBuf)) < TransportReturnCodes.SUCCESS)
        {
            return TransportReturnCodes.FAILURE;
        }
        else if (retCode > TransportReturnCodes.SUCCESS)
        {
            /* There is still data left to flush, leave our write notification enabled so we get called again.
             * If everything wasn't flushed, it usually indicates that the TCP output buffer cannot accept more yet 
             */

            /* set write flag if there's still other data queued */
            /* flush needs to be done by application */
        }

        return retCode;
    }

    /**************************************************************
     * Processes a login response *
     * 
     * @param msg - the partially decoded message *
     * @param decIter - the decode iterator *
     * @return status code *
     **************************************************************/
    public static int processLoginResponse(Msg msg, DecodeIterator decIter)
    {
        int retCode;
        State pState = CodecFactory.createState();

        String tempData = null;
        Buffer tempBuf = CodecFactory.createBuffer();

        tempBuf.data(tempData);

        /* Switch cases depending on message class */
        switch (msg.msgClass())
        {
            case MsgClasses.REFRESH:
            {
                MsgKey key = CodecFactory.createMsgKey();

                ElementList elementList = CodecFactory.createElementList();
                ElementEntry elementEntry = CodecFactory.createElementEntry();

                /* check stream id */
                System.out.printf("Received Login Refresh Msg with Stream Id %d\n", msg.streamId());

                /* check if it's a solicited refresh */
                if ((msg.flags() & RefreshMsgFlags.SOLICITED) != 0)
                {
                    System.out.printf("The refresh msg is a solicited refresh (sent as a response to a request).\n");
                }
                else
                {
                    System.out.printf("A refresh sent to inform a consumer of an upstream change in information (i.e., an unsolicited refresh).\n");
                }

                /* get key */
                key = msg.msgKey();

                /* decode key opaque data */
                if ((retCode = msg.decodeKeyAttrib(decIter, key)) != CodecReturnCodes.SUCCESS)
                {
                    return TransportReturnCodes.FAILURE;
                }

                /* decode element list */
                if ((retCode = elementList.decode(decIter, null)) == CodecReturnCodes.SUCCESS)
                {
                    /* decode each element entry in list */
                    while ((retCode = elementEntry.decode(decIter)) != CodecReturnCodes.END_OF_CONTAINER)
                    {
                        if (retCode == CodecReturnCodes.SUCCESS)
                        {
                            /* get login response information */
                            /* ApplicationId */
                            if (elementEntry.name().equals(ElementNames.APPID))
                            {
                                System.out.printf("\tReceived Login Response for ApplicationId: %s\n", elementEntry.encodedData().toString());
                            }
                            /* ApplicationName */
                            else if (elementEntry.name().equals(ElementNames.APPNAME))
                            {
                                System.out.printf("\tReceived Login Response for ApplicationName: %s\n", elementEntry.encodedData().toString());
                            }
                            /* Position */
                            else if (elementEntry.name().equals(ElementNames.POSITION))
                            {
                                System.out.printf("\tReceived Login Response for Position: %s\n", elementEntry.encodedData().toString());
                            }
                        }
                        else
                        {
                            return TransportReturnCodes.FAILURE;
                        }
                    }
                }
                else
                {
                    return TransportReturnCodes.FAILURE;
                }
                /* get Username */
                if (key != null)
                {
                    System.out.printf("Recieved Login Response for Username: %s\n", key.name().toString());
                }
                else
                {
                    System.out.printf("Recieved Login Response for Username: Unknown\n");
                }

                /* get state information */
                pState = ((RefreshMsg)msg).state();
                System.out.printf("%s\n\n", pState.toString());

                /* check if login okay and is solicited */
                if (((msg.flags() & RefreshMsgFlags.SOLICITED) != 0) && (pState.streamState() == StreamStates.OPEN && pState.dataState() == DataStates.OK))
                {
                    System.out.printf("Login stream is OK and solicited\n");
                }
                else /* handle error cases */
                {
                    if (pState.streamState() == StreamStates.CLOSED_RECOVER)
                    {
                        /* Stream State is (3) Closed, the applications may attempt to re-open the stream later (can occur via either an RefreshMsg or an StatusMsg) */
                        System.out.printf("Login stream is closed recover\n");
                        return TransportReturnCodes.FAILURE;
                    }
                    else if (pState.streamState() == StreamStates.CLOSED)
                    {
                        /* Stream State is (4) Closed (indicates that the data is not available on this service/connection and is not likely to become available) */
                        System.out.printf("Login attempt failed (stream closed)\n");
                        return TransportReturnCodes.FAILURE;
                    }
                    else if (pState.streamState() == StreamStates.OPEN && pState.dataState() == DataStates.SUSPECT)
                    {
                        /* Stream State is (1) Open (typically implies that information will be streaming, as information changes updated
                         * information will be sent on the stream, after final RefreshMsg or StatusMsg)
                         *
                         * Data State is (2) Data is Suspect (similar to a stale data state, indicates that the health of some or all data associated with the stream
                         * is out of date or cannot be confirmed that it is current )
                         */
                        System.out.printf("Login stream is suspect\n");
                        return TransportReturnCodes.FAILURE;
                    }
                }
            }
                break;
            case MsgClasses.STATUS:/* (3) Status Message */
            {
                System.out.printf("Received Login StatusMsg\n");
                if ((msg.flags() & StatusMsgFlags.HAS_STATE) != 0)
                {
                    /* get state information */
                    pState = ((StatusMsg)msg).state();
                    System.out.printf("%s\n\n", pState.toString());

                    /* handle error cases */
                    if (pState.streamState() == StreamStates.CLOSED_RECOVER)
                    {
                        /* Stream State is (3) Closed, the applications may attempt to re-open the stream later (can occur via either an RefreshMsg or an StatusMsg) */
                        System.out.printf("Login stream is closed recover\n");
                        return TransportReturnCodes.FAILURE;
                    }
                    else if (pState.streamState() == StreamStates.CLOSED)
                    {
                        /* Stream State is (4) Closed (indicates that the data is not available on this service/connection and is not likely to become available) */
                        System.out.printf("Login attempt failed (stream closed)\n");
                        return TransportReturnCodes.FAILURE;
                    }
                    else if (pState.streamState() == StreamStates.OPEN && pState.dataState() == DataStates.SUSPECT)
                    {
                        /* Stream State is (1) Open (typically implies that information will be streaming, as information changes updated
                         * information will be sent on the stream, after final RefreshMsg or StatusMsg)
                         *
                         * Data State is (2) Data is Suspect (similar to a stale data state, indicates that the health of some or all data associated with the stream
                         * is out of date or cannot be confirmed that it is current )
                         */
                        System.out.printf("Login stream is suspect\n");
                        return TransportReturnCodes.FAILURE;
                    }
                }
            }
                break;
            case MsgClasses.UPDATE:/* (4) Update Message */
            {
                System.out.printf("Received Login Update\n");
            }
                break;
            default:/* Error handling */
            {
                System.out.printf("Received Unhandled Login Msg Class: %d\n", msg.msgClass());
                return TransportReturnCodes.FAILURE;
            }
        }

        return TransportReturnCodes.SUCCESS;
    }

    /**************************************************************
     * Close the login stream *
     * 
     * @param channel - the Channel of connection *
     * @param maxFragmentSize - the maximum fragment size before fragmentation *
     * @return status code *
     **************************************************************/
    public static int closeLoginStream(Channel channel, int maxFragmentSize)
    {
        int retCode;
        Error error = TransportFactory.createError();
        TransportBuffer msgBuf = null;

        /* Consumer uses CloseMsg to indicate no further interest in an item stream and to close the stream. */
        CloseMsg closeMsg = (CloseMsg)CodecFactory.createMsg();

        /* Get a buffer of the channel max fragment size */
        /* UPA provides clear functions for its structures (e.g., encodeIterator.clear()). 
         * These functions are tuned to be efficient 
         * and avoid initializing unnecessary
         * structure members, and allow for optimal structure use and reuse. 
         */
        /* Iterator used for encoding throughout the application - we can clear it and reuse it instead of recreating it */
        EncodeIterator encIter = CodecFactory.createEncodeIterator(); /* the encode iterator is created (typically stack allocated)  */

        /* Obtains a non-packable buffer of the requested size from the UPA Transport guaranteed buffer pool to write into for the Login request.
         * For an outbound TransportBuffer (specifically one gotten from Channel.getBuffer(), like this one) length() initially does indeed indicate the number of bytes available, but when the ByteBuffer's position is after the start 
         * (because the application has written something, i.e. encoded some or all of a Login request message), 
         * it actually returns the number of bytes between the start & current positions 
         * (basically, the number of bytes encoded thus far).
         */
        /* upaGetBuffer() is the utility function that does 2-pass (more robust) getting non-packable buffer. */
        if ((msgBuf = upaGetBuffer(channel, maxFragmentSize, error)) == null)
        {
            return TransportReturnCodes.FAILURE;
        }

        /* if a buffer is returned, we can populate and write, encode an Msg into the buffer */

        /* Encodes the Login close. */

        closeMsg.clear();
        /* clear encode iterator for reuse - this should be used to achieve the best performance while clearing the iterator. */
        encIter.clear();
        /* set version information of the connection on the encode iterator so proper versioning can be performed */
        /* set the buffer on an EncodeIterator */

        if ((retCode = encIter.setBufferAndRWFVersion(msgBuf, channel.majorVersion(), channel.minorVersion())) < TransportReturnCodes.SUCCESS)
        {
            channel.releaseBuffer(msgBuf, error);
            System.out.printf("\nSetEncodeIteratorBuffer() failed with return code: %d\n", retCode);
            /* Closes channel, cleans up and exits the application. */
            return TransportReturnCodes.FAILURE;
        }

        /* Create and initialize close message */
        closeMsg.msgClass(MsgClasses.CLOSE); /* (5) Close Message */
        closeMsg.streamId(LOGIN_STREAM_ID);
        closeMsg.domainType(DomainTypes.LOGIN); /* (1) Login Message */
        /* No payload associated with this close message */
        closeMsg.containerType(DataTypes.NO_DATA); /* (128) No Data <BR>*/
        /* encode message */

        /* Since there is no payload, no need for Init/Complete as everything is in the msg header */
        /* Functions without a suffix of Init or Complete (e.g. EncodeMsg) perform encoding within a single call,
         * typically used for encoding simple types like Integer or incorporating previously encoded data
         * (referred to as pre-encoded data).
         */

        if ((retCode = closeMsg.encode(encIter)) < CodecReturnCodes.SUCCESS)
        {
            channel.releaseBuffer(msgBuf, error);
            System.out.printf("Msg.encode() failed with return code: %d\n", retCode);
            return TransportReturnCodes.FAILURE;
        }
        /* send login close */
        if ((retCode = sendMessage(channel, msgBuf)) < TransportReturnCodes.SUCCESS)
        {
            /* login close fails */
            /* Closes channel, cleans up and exits the application. */
            return TransportReturnCodes.FAILURE;
        }
        else if (retCode > TransportReturnCodes.SUCCESS)
        {
            /* There is still data left to flush, leave our write notification enabled so we get called again.
             * If everything wasn't flushed, it usually indicates that the TCP output buffer cannot accept more yet 
             */

            /* set write flag if there's still other data queued */
            /* flush needs to be done by application */
        }

        return retCode;
    }

    /**************************************************************
     * Performs two time pass to obtain buffer *
     * 
     * @param channel - the Channel of connection *
     * @param size - size of requested buffer *
     * @param error - tracks error info *
     * @return obtained buffer *
     **************************************************************/
    public static TransportBuffer upaGetBuffer(Channel channel, int size, Error error)
    {
        int retCode;

        TransportBuffer msgBuf = null;
        /* Obtains a non-packable buffer of the requested size from the UPA Transport guaranteed buffer pool to write into for any request Msg.
         * When the Buffer is returned, the length member indicates the number of bytes available in the buffer (this should match the amount
         * the application requested). When populating, it is required that the application set length to the number of bytes actually used.
         * This ensures that only the required bytes are written to the network.
         */

        /**
         * @brief Retrieves a Buffer for use
         *
         *        Typical use: <BR>
         *        This is called when a buffer is needed to write data to.
         *        Generally, the user will populate the Buffer structure and
         *        then pass it to the Write function.
         *
         * @param chnl Channel who requests the buffer
         * @param size Size of the requested buffer
         * @param packedBuffer Set to true if you plan on packing multiple
         *            messages into the same buffer
         * @param error Error, to be populated in event of an error
         * @return Buffer buffer to be filled in with valid memory
         * @see ReturnCodes
         */
        /* First check error */
        if ((msgBuf = channel.getBuffer(size, false, error)) == null)
        {
            if (error.errorId() != TransportReturnCodes.NO_BUFFERS)
            {
                /* Check to see if this is just out of buffers or if it's unrecoverable */

                /* it's unrecoverable Error */
                System.out.printf("Error (%d) (errno: %d) encountered with Channel.getBuffer. Error Text: %s\n", error.errorId(), error.sysError(), error.text());
                /* Connection should be closed, return failure */
                /* Closes channel, cleans up and exits the application. */

                return null;
            }

            /* (-4) Transport Failure: There are no buffers available from the buffer pool, returned from GetBuffer.
             * Use Ioctl to increase pool size or use Flush to flush data and return buffers to pool.
             */

            /* The Flush function could be used to attempt to free buffers back to the pool */

            /* Flush and obtain buffer again */
            retCode = channel.flush(error);
            if (retCode < TransportReturnCodes.SUCCESS)
            {
                System.out.printf("Channel.flush() failed with return code %d - <%s>\n", retCode, error.text());
                /* Closes channel, cleans up and exits the application. */
                return null;
            }

            if ((msgBuf = channel.getBuffer(size, false, error)) == null)
            {
                System.out.printf("Error (%d) (errno: %d) encountered with Channel.getBuffer. Error Text: %s\n", error.errorId(), error.sysError(), error.text());
                return null;
            }
        }
        /* return  buffer to be filled in with valid memory */
        return msgBuf;
    }

    final static int SRCDIR_STREAM_ID = 2;

    public static int sendSourceDirectoryRequest(Channel channel, int maxMsgSize)
    {
        int ret;
        Error error = TransportFactory.createError();
        TransportBuffer msgBuf = null;

        EncodeIterator encIter = CodecFactory.createEncodeIterator();

        RequestMsg requestMsg = (RequestMsg)CodecFactory.createMsg();

        /* get a buffer for the source directory request */
        if ((msgBuf = upaGetBuffer(channel, maxMsgSize, error)) == null)
        {
            /* Connection should be closed, return failure */
            /* Closes channel, cleans up and exits the application. */

            return TransportReturnCodes.FAILURE;
        }

        requestMsg.clear();

        encIter.clear();
        if ((ret = encIter.setBufferAndRWFVersion(msgBuf, channel.majorVersion(), channel.minorVersion())) < TransportReturnCodes.SUCCESS)
        {
            channel.releaseBuffer(msgBuf, error);
            System.out.printf("\nSetEncodeIteratorBuffer() failed with return code: %d\n", ret);
            /* Closes channel, cleans up and exits the application. */
            return TransportReturnCodes.FAILURE;
        }

        /*encode source directory request*/
        requestMsg.msgClass(MsgClasses.REQUEST);
        requestMsg.streamId(SRCDIR_STREAM_ID);
        requestMsg.domainType(DomainTypes.SOURCE);
        requestMsg.containerType(DataTypes.NO_DATA);
        /* RequestMsgFlags.STREAMING flag set. When the flag is set, the request is known as a "streaming" request, meaning
        * that the refresh will be followed by updates.
        */
        requestMsg.flags(RequestMsgFlags.STREAMING | RequestMsgFlags.HAS_PRIORITY);

        /* A consumer can request information about all services by omitting serviceId information
         * If the consumer wishes to receive information about all services, the consumer should not specify a serviceId
         * (i.e., the consumer should not set MsgKeyFlags.HAS_SERVICE_ID).
         */

        /* set members in msgKey */
        /* (0x0008) This MsgKey has a filter, contained in \ref MsgKey::filter.  */
        requestMsg.msgKey().flags(MsgKeyFlags.HAS_FILTER);

        /* Because the Source Directory domain uses an FilterList, a consumer can indicate the specific source related
         * information in which it is interested via a msgKey.filter. Each bit-value represented in the filter corresponds
         * to an information set that can be provided in response messages.
         * Thomson Reuters recommends that a consumer application minimally request Info, State, and Group filters for the
         * Source Directory:
         * - The Info filter contains the service name and serviceId data for all available services. When an appropriate
         *   service is discovered by the OMM Consumer, the serviceId associated with the service is used on subsequent
         *   requests to that service.
         * - The State filter contains status data for the service. Such data informs the Consumer whether the service is
         *   up (and available) or down (and unavailable).
         * - The Group filter conveys any item group status information, including group states and as regards the merging
         *   of groups if applicable.
         */
        requestMsg.msgKey().filter(Directory.ServiceFilterFlags.INFO | Directory.ServiceFilterFlags.STATE | Directory.ServiceFilterFlags.GROUP);

        ret = requestMsg.encode(encIter);
        if (ret != CodecReturnCodes.SUCCESS)
        {
            error.text("encodeDirectoryRequest(): Failed <" + CodecReturnCodes.toString(ret) + ">");
            return ret;
        }
        System.out.println("Send Directory Request");

        /* send source directory request */
        if ((ret = sendMessage(channel, msgBuf)) < TransportReturnCodes.SUCCESS)
        {
            /* Closes channel, cleans up and exits the application. */
            return TransportReturnCodes.FAILURE;
        }
        else if (ret > TransportReturnCodes.SUCCESS)
        {
            /* There is still data left to flush, leave our write notification enabled so we get called again.
             * If everything wasn't flushed, it usually indicates that the TCP output buffer cannot accept more yet
             */

            /* set write fd if there's still other data queued */
            /* flush needs to be done by application */
        }
        return ret;
    }

    public static int processSourceDirectoryResponse(Channel chnl, Msg msg, Error error, DecodeIterator dIter, Selector selector)
    {
        int retval;
        State pState = CodecFactory.createState();

        String tempData = null;
        Buffer tempBuffer = CodecFactory.createBuffer();

        Map map = CodecFactory.createMap();
        MapEntry mapEntry = CodecFactory.createMapEntry();
        FilterList filterList = CodecFactory.createFilterList();

        FilterEntry filterEntry = CodecFactory.createFilterEntry();
        ElementEntry elementEntry = CodecFactory.createElementEntry();
        ElementList elementList = CodecFactory.createElementList();
        Array array = CodecFactory.createArray();
        ArrayEntry arrayEntry = CodecFactory.createArrayEntry();

        int arrayCount = 0;

        int serviceCount = 0;
        /* this keeps track of which service we are actually interested in, that is,
         * when we run into the serviceName requested by the application
         */
        int foundServiceIndex = -1;

        String serviceName = "DIRECT_FEED";
        /* create primitive value to have key decoded into */
        UInt serviceId = CodecFactory.createUInt();

        UInt capabilities = CodecFactory.createUInt();

        String dictionariesProvided;

        Qos qoS = CodecFactory.createQos();
        List<Qos> QosBuf = new ArrayList<Qos>();
        boolean foundQoS = false;

        /* The ServiceState and AcceptingRequests elements in the State filter entry work together
         * to indicate the ability of a particular service to provide data:
         */
        UInt serviceState = CodecFactory.createUInt();
        UInt acceptingRequests = CodecFactory.createUInt();

        State serviceStatus = CodecFactory.createState();

        tempBuffer.data(tempData);

        RefreshMsg refreshMsg = (RefreshMsg)msg;
        StatusMsg statusMsg = (StatusMsg)msg;

        /* Switch cases depending on message class */
        switch (msg.msgClass())
        {
            case MsgClasses.REFRESH:
            case MsgClasses.UPDATE:
            {

                /* decode source directory refresh or update */

                if (msg.msgClass() == MsgClasses.REFRESH)
                    System.out.printf("\nReceived Source Directory Refresh\n");
                else /* (4) Update Message */
                {
                    System.out.printf("\nReceived Source Directory Update\n");

                    /* When displaying update information, we should also display the updateType information. */
                    System.out.printf("UPDATE TYPE: %s\n", UpdateEventTypes.toString(((UpdateMsg)msg).updateType()));
                }

                /* decode contents into the map structure */
                if ((retval = map.decode(dIter)) < CodecReturnCodes.SUCCESS)
                {
                    /* decoding failure tends to be unrecoverable */
                    System.out.printf("Error (%d) (errno: %d)encountered with DecodeMap. Error Text: %s", error.errorId(), error.sysError(), error.text());
                    return retval;
                }

                /* @brief The primitive type associated with each MapEntry key. */
                /* source directory refresh or update key data type must be DataTypes.UINT */
                if (map.keyPrimitiveType() != DataTypes.UINT)
                {
                    System.out.printf("Map has incorrect keyPrimitiveType");
                    return CodecReturnCodes.FAILURE;
                }

                /* if summary data is present, invoking decoder for that type (instead of DecodeEntry)
                 * indicates to UPA that user wants to decode summary data
                 */
                if ((map.flags() & MapFlags.HAS_SUMMARY_DATA) != 0)
                {
                    /* Map summary data is present. Its type should be that of Map.containerType */
                    System.out.printf("summary data is present. Its type should be that of Map.containerType\n");
                    /* Continue decoding ... */
                }
                else
                {
                    System.out.printf("\nMap summary data is NOT present.\n");
                }

                while ((retval = mapEntry.decode(dIter, serviceId)) != CodecReturnCodes.END_OF_CONTAINER)
                {
                    /* break out of decoding when predetermined max services (15) reached */
                    if (serviceCount == 15)
                    {
                        /* The decoding process typically runs until the end of each container, indicated by CodecReturnCodes.END_OF_CONTAINER.
                         * This FinishDecodeEntries() function sets the application to skip remaining entries in a container and continue
                         * the decoding process. This function will skip past remaining entries in the container and perform necessary
                         * synchronization between the content and iterator so that decoding can continue.
                         */
                        dIter.finishDecodeEntries();
                        System.out.printf("processSourceDirectoryResponse() maxServices limit reached - more services in message than memory can support\n");
                        break;
                    }

                    if (retval != CodecReturnCodes.SUCCESS && retval != CodecReturnCodes.BLANK_DATA)
                    {
                        /* decoding failure tends to be unrecoverable */
                        System.out.printf("Error (%d) (errno: %d)encountered with DecodeMapEntry. Error Text: %s", error.errorId(), error.sysError(), error.text());
                        return retval;
                    }
                    else
                    {
                        if (msg.msgClass() == MsgClasses.REFRESH)
                            System.out.printf("\nReceived Source Directory Refresh for Decoded Service Id:%d", serviceId.toLong());
                        else /* (4) Update Message */
                            System.out.printf("\nReceived Source Directory Update for Decoded Service Id: %d", serviceId.toLong());

                        /* if this is the current serviceId we are interested in */
                        if ((serviceId.equals(serviceDiscoveryInfo_serviceId)) && (serviceDiscoveryInfo_serviceNameFound == true))
                        {
                            /* this is the current serviceId we are interested in and requested by the UPA Consumer application */
                            System.out.printf(" (%s)\n", serviceDiscoveryInfo_serviceName);
                        }

                        /* decode contents into the filter list structure */
                        if ((retval = filterList.decode(dIter)) < CodecReturnCodes.SUCCESS)
                        {
                            /* decoding failure tends to be unrecoverable */
                            System.out.printf("Error (%d) (errno: %d)encountered with DecodeFilterList. Error Text: %s", error.errorId(), error.sysError(), error.text());
                            return retval;
                        }

                        /* decode each filter entry until there are no more left.
                         * Decodes an FilterEntry. This function expects the same DecodeIterator that was used with DecodeFilterList.
                         * This populates encData with an encoded entry.
                         */
                        while ((retval = filterEntry.decode(dIter)) != CodecReturnCodes.END_OF_CONTAINER)
                        {
                            if (retval < CodecReturnCodes.SUCCESS)
                            {
                                /* decoding failure tends to be unrecoverable */
                                System.out.printf("Error (%d) (errno: %d)encountered with DecodeFilterEntry. Error Text: %s", error.errorId(), error.sysError(), error.text());
                                return retval;
                            }

                            else
                            {
                                /* Show how to use ContainerType.
                                 * After DecodeFilterEntry function returns, the FilterList.containerType (or FilterEntry.containerType if present)
                                 * can invoke the correct contained type's decode functions.
                                 * if filterEntry.containerType is present, switch on that; otherwise switch on filterList.containerType
                                 */
                                int cType;
                                if ((filterEntry.flags() & FilterEntryFlags.HAS_CONTAINER_TYPE) != 0)
                                    cType = filterEntry.containerType();
                                else
                                    cType = filterList.containerType();
                                switch (cType)
                                {
                                    /* (137) Map container type, used to represent primitive type key - container type paired entries.   <BR>*/
                                    case DataTypes.MAP:
                                    {
                                        /* Continue decoding map entries. call DecodeMap function
                                         * printf("DecodeFilterEntry: Continue decoding map entries.\n");
                                         */
                                    }
                                        break;
                                    /* (133) Element List container type, used to represent content containing element name, dataType, and value triples.    <BR>*/
                                    case DataTypes.ELEMENT_LIST:
                                    {
                                        /* For Source Directory response, we actually know it is DataTypes.ELEMENT_LIST up-front
                                         * See code below in the next switch block: we are calling DecodeElementList to
                                         * continue decoding element entries
                                         */

                                        /* Continue decoding element entries. call DecodeElementList function
                                         * printf("DecodeFilterEntry: Continue decoding element entries.\n");
                                         */
                                    }
                                        break;
                                    default: /* Error handling */
                                    {
                                        System.out.printf("\nUnkonwn ContainerType: %d\n", cType);
                                        return CodecReturnCodes.FAILURE;
                                    }
                                }
                                /* decode source directory response information */
                                switch (filterEntry.id())
                                {
                                    case Directory.ServiceFilterIds.INFO: /* (1) Service Info Filter ID */
                                    {
                                        System.out.printf("\nDecoding Service Info Filter ID FilterListEntry\n");

                                        /* decode element list - third parameter is 0 because we do not have set definitions in this example */
                                        if ((retval = elementList.decode(dIter, null)) < CodecReturnCodes.SUCCESS)
                                        {
                                            /* decoding failure tends to be unrecoverable */
                                            System.out.printf("Error (%d) (errno: %d)encountered with DecodeElementList. Error Text: %s", error.errorId(), error.sysError(), error.text());
                                            return retval;
                                        }
                                        /* decode element list elements */
                                        while ((retval = elementEntry.decode(dIter)) != CodecReturnCodes.END_OF_CONTAINER)
                                        {
                                            if (retval < CodecReturnCodes.SUCCESS)
                                            {
                                                /* decoding failure tends to be unrecoverable */
                                                System.out.printf("Error (%d) (errno: %d)encountered with DecodeElementEntry. Error Text: %s", error.errorId(), error.sysError(), error.text());
                                                return retval;
                                            }
                                            else
                                            {
                                                /* get service general information */

                                                /* Name */
                                                if (elementEntry.name().equals(ElementNames.NAME))
                                                {
                                                    if (msg.msgClass() == MsgClasses.REFRESH)
                                                        System.out.printf("\tReceived Source Directory Refresh for ServiceName: %s\n", elementEntry.encodedData().toString());
                                                    else /* (4) Update Message */
                                                        System.out.printf("\tReceived Source Directory Update for ServiceName: %s\n", elementEntry.encodedData().toString());

                                                    /* When an appropriate service is discovered by the OMM consumer, it uses the serviceId associated with the service
                                                     * on all subsequent requests to that service. Check if service name received in response matches that entered by user
                                                     * if it does, store the service id
                                                     */
                                                    serviceName = elementEntry.encodedData().toString();

                                                    if (serviceName.equals(serviceDiscoveryInfo_serviceName))
                                                    {
                                                        /* serviceName requested by the application is FOUND */
                                                        foundServiceIndex = serviceCount;

                                                        System.out.printf("\tService name: %s (%d) is discovered by the OMM consumer. \n", serviceName, serviceId.toLong());
                                                        serviceDiscoveryInfo_serviceId = serviceId;
                                                        serviceDiscoveryInfo_serviceNameFound = true;
                                                    }
                                                }
                                                /* Capabilities */
                                                else if (elementEntry.name().equals(ElementNames.CAPABILITIES))
                                                {
                                                    /* decode into the array structure header */
                                                    if ((retval = array.decode(dIter)) < CodecReturnCodes.SUCCESS)
                                                    {
                                                        /* decoding failure tends to be unrecoverable */
                                                        System.out.printf("Error %s (%d) encountered with DecodeArray. Error Text: %s\n", error.errorId(), error.sysError(), error.text());
                                                        return retval;
                                                    }
                                                    while ((retval = arrayEntry.decode(dIter)) != CodecReturnCodes.END_OF_CONTAINER)
                                                    {
                                                        /* break out of decoding array items when predetermined max capabilities (10) reached */
                                                        if (arrayCount == 10)
                                                        {
                                                            /* The decoding process typically runs until the end of each container, indicated by CodecReturnCodes.END_OF_CONTAINER.
                                                             * This FinishDecodeEntries function sets the application to skip remaining entries in a container and continue
                                                             * the decoding process. This function will skip past remaining entries in the container and perform necessary
                                                             * synchronization between the content and iterator so that decoding can continue.
                                                             */
                                                            dIter.finishDecodeEntries();
                                                            break;
                                                        }

                                                        if (retval == CodecReturnCodes.SUCCESS)
                                                        {
                                                            retval = capabilities.decode(dIter);
                                                            if (retval != CodecReturnCodes.SUCCESS && retval != CodecReturnCodes.BLANK_DATA)
                                                            {
                                                                System.out.printf("Error %s (%d) encountered with DecodeUInt(). Error Text: %s\n", error.errorId(), error.sysError(), error.text());
                                                                return retval;
                                                            }
                                                            if (msg.msgClass() == MsgClasses.REFRESH)
                                                                System.out.printf("\tReceived Source Directory Refresh for Decoded Capabilities[%d]: %d\n", arrayCount, capabilities.toLong());
                                                            else /* (4) Update Message */
                                                                System.out.printf("\tReceived Source Directory Update for Decoded Capabilities[%d]: %d\n", arrayCount, capabilities.toLong());

                                                            /* if advertising Dictionary domain type is supported */
                                                            if (capabilities.toLong() == DomainTypes.DICTIONARY)
                                                            {
                                                                System.out.printf("\tDICTIONARY domain type is supported.\n");
                                                                serviceDiscoveryInfo_upalDMTDictionarySupported = true;
                                                            }

                                                            /* if advertising MarketPrice domain type is supported */
                                                            if (capabilities.toLong() == DomainTypes.MARKET_PRICE)
                                                            {
                                                                System.out.printf("\tMARKET_PRICE domain type is supported.\n");
                                                                serviceDiscoveryInfo_upaDMTMarketPriceSupported = true;
                                                            }

                                                        }
                                                        else if (retval != CodecReturnCodes.BLANK_DATA)
                                                        {
                                                            /* decoding failure tends to be unrecoverable */
                                                            System.out.printf("Error %s (%d) encountered with DecodeArrayEntry. Error Text: %s\n", error.errorId(), error.sysError(), error.text());
                                                            return retval;
                                                        }
                                                        arrayCount++;
                                                    }
                                                    arrayCount = 0;
                                                }
                                                /* DictionariesProvided */
                                                else if (elementEntry.name().equals(ElementNames.DICTIONARIES_PROVIDED))
                                                {
                                                    /* decode into the array structure header */
                                                    if ((retval = array.decode(dIter)) < CodecReturnCodes.SUCCESS)
                                                    {
                                                        /* decoding failure tends to be unrecoverable */
                                                        System.out.printf("Error %s (%d) encountered with DecodeArray. Error Text: %s\n", error.errorId(), error.sysError(), error.text());
                                                        return retval;
                                                    }
                                                    while ((retval = arrayEntry.decode(dIter)) != CodecReturnCodes.END_OF_CONTAINER)
                                                    {
                                                        /* break out of decoding array items when predetermined max dictionaries (5) reached */
                                                        if (arrayCount == 5)
                                                        {
                                                            /* The decoding process typically runs until the end of each container, indicated by CodecReturnCodes.END_OF_CONTAINER.
                                                             * This FinishDecodeEntries function sets the application to skip remaining entries in a container and continue
                                                             * the decoding process. This function will skip past remaining entries in the container and perform necessary
                                                             * synchronization between the content and iterator so that decoding can continue.
                                                             */
                                                            dIter.finishDecodeEntries();
                                                            break;
                                                        }

                                                        if (retval == CodecReturnCodes.SUCCESS)
                                                        {
                                                            if (msg.msgClass() == MsgClasses.REFRESH)
                                                                System.out.printf("\tReceived Source Directory Refresh for DictionariesProvided[%d]: %s\n", arrayCount, arrayEntry.encodedData().toString());
                                                            else /* (4) Update Message */
                                                                System.out.printf("\tReceived Source Directory Update for DictionariesProvided[%d]: %s\n", arrayCount, arrayEntry.encodedData().toString());

                                                            /* DictionariesProvided provide the dictionaries that are available for downloading */
                                                            /* Our training UPA Consumer app only cares about RDMFieldDictionary and enumtype.def */
                                                            dictionariesProvided = arrayEntry.encodedData().toString();

                                                            if (dictionariesProvided.equals(dictionaryDownloadName))
                                                            {
                                                                /* dictionary RDMFieldDictionary is available for downloading */
                                                                System.out.printf("\tDictionary Provided: %s with filename: %s \n", dictionariesProvided, fieldDictionaryFileName);
                                                                serviceDiscoveryInfo_RDMFieldDictionaryProvided = true;
                                                            }
                                                            else if (dictionariesProvided.equals(enumTableDownloadName))
                                                            {
                                                                /* dictionary enumtype.def is available for downloading */
                                                                System.out.printf("\tDictionary Provided: %s with filename: %s \n", enumTableDownloadName, enumTypeDictionaryFileName);
                                                                serviceDiscoveryInfo_enumtypeProvided = true;
                                                            }
                                                        }
                                                        else if (retval != CodecReturnCodes.BLANK_DATA)
                                                        {
                                                            /* decoding failure tends to be unrecoverable */
                                                            System.out.printf("Error %s (%d) encountered with DecodeArrayEntry. Error Text: %s\n", error.errorId(), error.sysError(), error.text());
                                                            return retval;
                                                        }
                                                        arrayCount++;
                                                    }
                                                    arrayCount = 0;
                                                }

                                                /* QoS */
                                                else if (elementEntry.name().equals(ElementNames.QOS))
                                                {
                                                    foundQoS = true;
                                                    /* decode into the array structure header */
                                                    if ((retval = array.decode(dIter)) < CodecReturnCodes.SUCCESS)
                                                    {
                                                        /* decoding failure tends to be unrecoverable */
                                                        System.out.printf("Error %s (%d) encountered with DecodeArray. Error Text: %s\n", error.errorId(), error.sysError(), error.text());
                                                        return retval;
                                                    }
                                                    while ((retval = arrayEntry.decode(dIter)) != CodecReturnCodes.END_OF_CONTAINER)
                                                    {
                                                        /* break out of decoding array items when predetermined max QOS (5) reached */
                                                        if (arrayCount == 5)
                                                        {
                                                            /* The decoding process typically runs until the end of each container, indicated by CodecReturnCodes.END_OF_CONTAINER.
                                                             * This FinishDecodeEntries function sets the application to skip remaining entries in a container and continue
                                                             * the decoding process. This function will skip past remaining entries in the container and perform necessary
                                                             * synchronization between the content and iterator so that decoding can continue.
                                                             */
                                                            dIter.finishDecodeEntries();
                                                            break;
                                                        }

                                                        if (retval == CodecReturnCodes.SUCCESS)
                                                        {
                                                            /* Obtain QoS information such as data timeliness (e.g. real time) and rate (e.g. tick-by-tick). */
                                                            retval = qoS.decode(dIter);
                                                            if (retval != CodecReturnCodes.SUCCESS && retval != CodecReturnCodes.BLANK_DATA)
                                                            {
                                                                System.out.printf("DecodeQos() failed with return code: %d\n", retval);
                                                                return retval;
                                                            }
                                                            else
                                                            {

                                                                QosBuf.add(qoS);
                                                                System.out.printf("\tReceived %s\n", QosBuf.get(0));
                                                            }
                                                        }
                                                        else if (retval != CodecReturnCodes.BLANK_DATA)
                                                        {
                                                            /* decoding failure tends to be unrecoverable */
                                                            System.out.printf("Error %s (%d) encountered with DecodeArrayEntry. Error Text: %s\n", error.errorId(), error.sysError(), error.text());
                                                            return retval;
                                                        }
                                                        arrayCount++;
                                                    }
                                                    arrayCount = 0;

                                                    /* if this is the serviceName that is requested by the application */
                                                    if (serviceCount == foundServiceIndex)
                                                    {
                                                        /* Need to store the Source Directory QoS information */
                                                        serviceDiscoveryInfo_qoS.add(qoS);
                                                    }
                                                }
                                            }
                                        }

                                        /* if QoS was not send in the directory refresh message set it to the default values */
                                        if (!foundQoS)
                                        {
                                            System.out.printf("\tNot Received Source Directory Refresh for QoS\n");
                                            System.out.printf("\tSet default QoS: Realtime/TickByTick/Static\n");

                                            for (arrayCount = 0; arrayCount < 5; arrayCount++)
                                            {
                                                serviceDiscoveryInfo_qoS.get(arrayCount).timeliness(QosTimeliness.REALTIME);
                                                serviceDiscoveryInfo_qoS.get(arrayCount).rate(QosRates.TICK_BY_TICK);
                                                serviceDiscoveryInfo_qoS.get(arrayCount).dynamic(false);
                                            }
                                        }
                                    }
                                        break;
                                    case Directory.ServiceFilterIds.STATE: /* (2) Source State Filter ID */
                                    {
                                        System.out.printf("\nDecoding Source State Filter ID FilterListEntry\n");

                                        /* decode element list - third parameter is 0 because we do not have set definitions in this example */
                                        if ((retval = elementList.decode(dIter, null)) < CodecReturnCodes.SUCCESS)
                                        {
                                            /* decoding failure tends to be unrecoverable */
                                            System.out.printf("Error %s (%d) encountered with DecodeElementList. Error Text: %s\n", error.errorId(), error.sysError(), error.text());
                                            return retval;
                                        }

                                        /* decode element list elements */
                                        while ((retval = elementEntry.decode(dIter)) != CodecReturnCodes.END_OF_CONTAINER)
                                        {
                                            if (retval < CodecReturnCodes.SUCCESS)
                                            {
                                                /* decoding failure tends to be unrecoverable */
                                                System.out.printf("Error %s (%d) encountered with DecodeElementEntry. Error Text: %s\n", error.errorId(), error.sysError(), error.text());
                                                return retval;
                                            }
                                            else
                                            {
                                                /* get service state information */

                                                /* The ServiceState and AcceptingRequests elements in the State filter entry work together to indicate the ability of
                                                 * a particular service to provide data:
                                                 * - ServiceState indicates whether the source of the data is accepting requests.
                                                 * - AcceptingRequests indicates whether the immediate upstream provider (the provider to which the consumer is directly connected)
                                                 *   can accept new requests and/or process reissue requests on already open streams.
                                                 */

                                                /* for our training app, for the service we are intersted in to be considered really up, both ServiceState has to be Up (1), and
                                                 * AcceptingRequests has to be Yes (1). That way, New requests and reissue requests can be successfully processed.
                                                 */

                                                /* ServiceState - Up(1), Down (0) */
                                                /* RDM_DIRECTORY_SERVICE_STATE_DOWN -  (0) Service state down */
                                                /* RDM_DIRECTORY_SERVICE_STATE_UP -    (1) Service state up */
                                                if (elementEntry.name().equals(ElementNames.SVC_STATE))
                                                {

                                                    retval = serviceState.decode(dIter);
                                                    if (retval != CodecReturnCodes.SUCCESS && retval != CodecReturnCodes.BLANK_DATA)
                                                    {
                                                        System.out.printf("Error %s (%d) encountered with State.Decode(). Error Text: %s\n", error.errorId(), error.sysError(), error.text());
                                                        return retval;
                                                    }
                                                    if (msg.msgClass() == MsgClasses.REFRESH)
                                                        System.out.printf("\tReceived Source Directory Refresh for Decoded ServiceState: %d\n", serviceState.toLong());
                                                    else /* (4) Update Message */
                                                        System.out.printf("\tReceived Source Directory Update for Decoded ServiceState: %d\n", serviceState.toLong());

                                                    /* if this is the serviceName that is requested by the application */
                                                    if (serviceCount == foundServiceIndex)
                                                    {
                                                        /* Need to track that service we care about is up */
                                                        serviceDiscoveryInfo_ServiceState = (int)serviceState.toLong();
                                                    }
                                                }

                                                /* AcceptingRequests - Yes (1), No (0) */
                                                else if (elementEntry.name().equals(ElementNames.ACCEPTING_REQS))
                                                {
                                                    retval = acceptingRequests.decode(dIter);
                                                    if (retval != CodecReturnCodes.SUCCESS && retval != CodecReturnCodes.BLANK_DATA)
                                                    {
                                                        System.out.printf("Error %s (%d) encountered with DecodeUInt(). Error Text: %s\n", error.errorId(), error.sysError(), error.text());
                                                        return retval;
                                                    }
                                                    if (msg.msgClass() == MsgClasses.REFRESH)
                                                        System.out.printf("\tReceived Source Directory Refresh for Decoded AcceptingRequests: %d\n", acceptingRequests.toLong());
                                                    else /* (4) Update Message */
                                                        System.out.printf("\tReceived Source Directory Update for Decoded AcceptingRequests: %d\n", acceptingRequests.toLong());

                                                    /* if this is the serviceName that is requested by the application */
                                                    if (serviceCount == foundServiceIndex)
                                                    {
                                                        /* Need to track that service we care about is accepting requests */
                                                        serviceDiscoveryInfo_AcceptingRequests = (int)acceptingRequests.toLong();
                                                    }
                                                }

                                                /* Status */
                                                else if (elementEntry.name().equals(ElementNames.STATUS))
                                                {
                                                    retval = serviceStatus.decode(dIter);
                                                    if (retval != CodecReturnCodes.SUCCESS && retval != CodecReturnCodes.BLANK_DATA)
                                                    {
                                                        System.out.printf("Error decoding State.\n");
                                                        return retval;
                                                    }
                                                    if (msg.msgClass() == MsgClasses.REFRESH)
                                                        System.out.printf("\tReceived Source Directory Refresh for Decoded State: %d %d %d %s\n", serviceStatus.streamState(), serviceStatus.dataState(), serviceStatus.code(), serviceStatus.text().toString());
                                                    else /* (4) Update Message */
                                                        System.out.printf("\tReceived Source Directory Update for Decoded State: %d %d %d %s\n", serviceStatus.streamState(), serviceStatus.dataState(), serviceStatus.code(), serviceStatus.text().toString());

                                                    tempBuffer.data(serviceStatus.toString());
                                                    System.out.printf("%s\n\n", tempBuffer.toString());
                                                }
                                            }
                                        }

                                    }
                                        break;
                                    case Directory.ServiceFilterIds.GROUP: /* (3) Source Group Filter ID */
                                    {
                                        System.out.printf("\nDecoding Source Group Filter ID FilterListEntry\n");

                                        /* decode element list - third parameter is 0 because we do not have set definitions in this example */
                                        if ((retval = elementList.decode(dIter, null)) < CodecReturnCodes.SUCCESS)
                                        {
                                            /* decoding failure tends to be unrecoverable */
                                            System.out.printf("Error %s (%d) encountered with DecodeElementList. Error Text: %s\n", error.errorId(), error.sysError(), error.text());
                                            return retval;
                                        }

                                        /* decode element list elements */
                                        while ((retval = elementEntry.decode(dIter)) != CodecReturnCodes.END_OF_CONTAINER)
                                        {
                                            if (retval < CodecReturnCodes.SUCCESS)
                                            {
                                                /* decoding failure tends to be unrecoverable */
                                                System.out.printf("Error %s (%d) encountered with DecodeElementEntry. Error Text: %s\n", error.errorId(), error.sysError(), error.text());
                                                return retval;
                                            }
                                            else
                                            {
                                                /* get service group information */

                                                /* Group */
                                                if (elementEntry.name().equals(ElementNames.GROUP))
                                                {
                                                    if (msg.msgClass() == MsgClasses.REFRESH)
                                                        System.out.printf("\tReceived Source Directory Refresh for Group: %s\n", elementEntry.encodedData().data());
                                                    else /* (4) Update Message */
                                                        System.out.printf("\tReceived Source Directory Update for Group: %s\n", elementEntry.encodedData().data());
                                                }
                                            }
                                        }
                                    }
                                        break;
                                    case Directory.ServiceFilterIds.LOAD: /* (4) Source Load Filter ID */
                                    {
                                        System.out.printf("\nDecoding Source Load Filter ID FilterListEntry not supported in this app\n");
                                    }
                                        break;
                                    case Directory.ServiceFilterIds.DATA: /* (5) Source Data Filter ID */
                                    {
                                        System.out.printf("\nDecoding Source Data Filter ID FilterListEntry not supported in this app\n");
                                    }
                                        break;
                                    case Directory.ServiceFilterIds.LINK: /* (6) Communication Link Filter ID */
                                    {
                                        System.out.printf("\nDecoding Communication Link Filter ID FilterListEntry not supported in this app\n");
                                    }
                                        break;
                                    default: /* Error handling */
                                    {
                                        System.out.printf("\nUnkonwn FilterListEntry filterID: %d\n", filterEntry.id());
                                        return CodecReturnCodes.FAILURE;
                                    }
                                }
                            }
                        }
                    }
                    serviceCount++;
                }

                if (msg.msgClass() == MsgClasses.REFRESH)
                {
                    pState = refreshMsg.state();
                    System.out.printf("	%s\n\n", pState.toString());
                }
            }
                break;

            case MsgClasses.STATUS:
                System.out.printf("\nReceived Source Directory StatusMsg\n");
                if ((statusMsg.flags() & StatusMsgFlags.HAS_STATE) != 0)
                {
                    pState = statusMsg.state();
                    System.out.printf("	%s\n\n", pState.toString());
                }
                break;

            case MsgClasses.CLOSE: /* (5) Close Message */
            {
                System.out.printf("\nReceived Source Directory Close\n");
            }
                break;
            default:
            {
                error.text("Received Unhandled Source Directory Msg Class: " + msg.msgClass());
                retval = CodecReturnCodes.FAILURE;
            }
                break;
        }

        return CodecReturnCodes.SUCCESS;
    }

}
