/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2019-2022 Refinitiv. All rights reserved.         --
 *|-----------------------------------------------------------------------------
 */

/*****************************************************************************************
 * This is the ETA Interactive Provider Training series of the ETA Training Suite
 * applications. The purpose of this application is to show step-by-step 
 * training how to build a ETA OMM Interactive Provider using the ETA Transport layer.
 *
 * Main Java source file for the ETA Interactive Provider Training application. It is a 
 * single-threaded client application.
 *
 *****************************************************************************************
 * ETA Interactive Provider Training Module 1a: Establish network communication
 *****************************************************************************************
 * Summary:
 * An OMM Interactive Provider application opens a listening socket on a well-known 
 * port allowing OMM consumer applications to connect. Once connected, consumers 
 * can request data from the Interactive Provider.
 *
 * In this module, the OMM Interactive Provider application opens a listening socket
 * on a well-known port allowing OMM consumer applications to connect.
 *
 * Detailed Descriptions:
 * The first step of any ETA Interactive Provider application is to establish
 * a listening socket, usually on a well-known port so that consumer applications
 * can easily connect. The provider uses the Transport.bind() method to open the port
 * and listen for incoming connection attempts.
 * Whenever an OMM consumer application attempts to connect, the provider uses
 * the Server.accept() method to begin the connection initialization process.
 *
 * For this simple training app, the interactive provider only supports a single client.
 *
 * Command line usage:
 *
 * ./gradlew runprovidermod1a
 * (runs with a default set of parameters (-h localhost -p 14002 ))
 *
 * or
 *
 * ./gradlew runprovidermod1a -PcommandLineArgs="[-h <SrvrHostname>] [-p <SrvrPortNo>]"
 * (runs with specified set of parameters, all parameters are optional)
 *
 * Pressing the CTRL+C buttons terminates the program.
 *
 *****************************************************************************************
 * ETA Interactive Provider Training Module 1b: Ping (heartbeat) Management
 *****************************************************************************************
 * Summary:
 * In this module, after establishing a connection, ping messages might
 * need to be exchanged. The negotiated ping timeout is available via
 * the Channel. If ping heartbeats are not sent or received within
 * the expected time frame, the connection can be terminated. Refinitiv
 * recommends sending ping messages at intervals one-third the
 * size of the ping timeout.
 *
 * Detailed Descriptions:
 * Once the connection is active, the consumer and provider applications
 * might need to exchange ping messages. A negotiated ping timeout is available
 * via Channel corresponding to each connection (this value might differ on
 * a per-connection basis). A connection can be terminated if ping heartbeats
 * are not sent or received within the expected time frame. Refinitiv
 * recommends sending ping messages at intervals one-third the size of the ping timeout.
 * Ping or heartbeat messages are used to indicate the continued presence of
 * an application. These are typically only required when no other information is
 * being exchanged. Because the provider application is likely sending more frequent
 * information, providing updates on any streams the consumer has requested,
 * it may not need to send heartbeats as the other data is sufficient to announce
 * its continued presence. It is the responsibility of each connection to manage
 * the sending and receiving of heartbeat messages.
 *
 * Command line usage:
 *
 * ./gradlew runprovidermod1b
 * (runs with a default set of parameters (-p 14002 -r 300))
 *
 * or
 *
 * ./gradlew runprovidermod1b -PcommandLineArgs="[-p <SrvrPortNo>] [-r <Running Time>]"
 * (runs with specified set of parameters, all parameters are optional)
 *
 * Pressing the CTRL+C buttons terminates the program.
 *
 *****************************************************************************************
 * ETA Interactive Provider Training Module 1c: Reading and Writing Data
 *****************************************************************************************
 * Summary:
 * In this module, when a client or server Channel.state() is
 * ChannelState.ACTIVE, it is possible for an application to receive
 * data from the connection. Similarly, when a client or server
 * Channel.state() is ChannelState.ACTIVE, it is possible for an
 * application to write data to the connection. Writing involves a several
 * step process.
 *
 * Detailed Descriptions:
 * When a client or server Channel.state() is ChannelState.ACTIVE, it is
 * possible for an application to receive data from the connection. The
 * arrival of this information is often announced by the I/O notification
 * mechanism that the Channel.scktChannel() is registered with. The ETA
 * Transport reads information from the network as a byte stream, after
 * which it determines buffer boundaries and returns each buffer one by
 * one.
 *
 * When a client or server Channel.state() is ChannelState.ACTIVE, it is
 * possible for an application to write data to the connection. Writing
 * involves a several step process. Because the ETA Transport provides
 * efficient buffer management, the user is required to obtain a buffer
 * from the ETA Transport buffer pool. This can be the guaranteed output
 * buffer pool associated with a Channel. After a buffer is acquired,
 * the user can populate the Buffer.data and set the Buffer.length
 * to the number of bytes referred to by data. If queued information cannot
 * be passed to the network, a function is provided to allow the application
 * to continue attempts to flush data to the connection. An I/O notification
 * mechanism can be used to help with determining when the network is able
 * to accept additional bytes for writing. The ETA Transport can continue to
 * queue data, even if the network is unable to write.
 *
 * Command line usage:
 *
 * ./gradlew runprovidermod1c
 * (runs with a default set of parameters (-p 14002 -r 300))
 *
 * or
 *
 * ./gradlew runprovidermod1c -PcommandLineArgs="[-p <SrvrPortNo>] [-r <Running Time>]"
 * (runs with specified set of parameters, all parameters are optional)
 *
 * Pressing the CTRL+C buttons terminates the program.
 * 
 *****************************************************************************************
 * ETA Interactive Provider Training Module 2: Perform/Handle Login Process
 *****************************************************************************************
 * Summary:
 * Applications authenticate with one another using the Login domain model. 
 * An OMM Interactive Provider must handle the consumer's Login request messages 
 * and supply appropriate responses.
 * 
 * In this module, after receiving a Login request, the Interactive Provider 
 * can perform any necessary authentication and permissioning.
 *
 * Detailed Descriptions:
 * After receiving a Login request, the Interactive Provider can perform any 
 * necessary authentication and permissioning.
 *
 * a) If the Interactive Provider grants access, it should send an RefreshMsg 
 * to convey that the user successfully connected. This message should indicate 
 * the feature set supported by the provider application.
 * b) If the Interactive Provider denies access, it should send an StatusMsg, 
 * closing the connection and informing the user of the reason for denial.
 *
 * The login handler for this simple Interactive Provider application only allows
 * one login stream per channel. It provides functions for processing login requests
 * from consumers and sending back the responses. Functions for sending login request
 * reject/close status messages, initializing the login handler, and closing login streams 
 * are also provided.
 *
 * Also please note for simple training app, the interactive provider only supports 
 * one client session from the consumer, that is, only supports one channel/client connection.
 *
 * Content is encoded and decoded using the ETA Message Package and the ETA 
 * Data Package.
 *
 * Command line usage:
 *
 * ./gradlew runprovidermod2
 * (runs with a default set of parameters (-p 14002 -r 300))
 *
 * or
 *
 * ./gradlew runprovidermod2 -PcommandLineArgs="[-p <SrvrPortNo>] [-r <Running Time>]"
 * (runs with specified set of parameters, all parameters are optional)
 *
 * Pressing the CTRL+C buttons terminates the program.
 *****************************************************************************************/

package com.refinitiv.eta.training.provider;

import java.io.IOException;
import java.net.InetAddress;
import java.net.UnknownHostException;
import java.nio.channels.ClosedChannelException;
import java.nio.channels.SelectionKey;
import java.nio.channels.Selector;
import java.util.Iterator;
import java.util.Set;

import com.refinitiv.eta.codec.Codec;
import com.refinitiv.eta.training.common.TrainingModuleUtils;
import com.refinitiv.eta.transport.AcceptOptions;
import com.refinitiv.eta.transport.BindOptions;
import com.refinitiv.eta.transport.Channel;
import com.refinitiv.eta.transport.ChannelInfo;
import com.refinitiv.eta.transport.ChannelState;
import com.refinitiv.eta.transport.CompressionTypes;
import com.refinitiv.eta.transport.Error;
import com.refinitiv.eta.transport.InProgFlags;
import com.refinitiv.eta.transport.InProgInfo;
import com.refinitiv.eta.transport.InitArgs;
import com.refinitiv.eta.transport.Server;
import com.refinitiv.eta.transport.Transport;
import com.refinitiv.eta.transport.TransportFactory;
import com.refinitiv.eta.transport.TransportReturnCodes;
import com.refinitiv.eta.transport.ReadArgs;
import com.refinitiv.eta.codec.Buffer;
import com.refinitiv.eta.codec.CodecFactory;
import com.refinitiv.eta.codec.CodecReturnCodes;
import com.refinitiv.eta.codec.DataStates;
import com.refinitiv.eta.codec.DataTypes;
import com.refinitiv.eta.codec.DecodeIterator;
import com.refinitiv.eta.codec.ElementEntry;
import com.refinitiv.eta.codec.ElementList;
import com.refinitiv.eta.codec.ElementListFlags;
import com.refinitiv.eta.codec.EncodeIterator;
import com.refinitiv.eta.codec.Msg;
import com.refinitiv.eta.codec.MsgClasses;
import com.refinitiv.eta.codec.MsgKey;
import com.refinitiv.eta.codec.MsgKeyFlags;
import com.refinitiv.eta.codec.RefreshMsg;
import com.refinitiv.eta.codec.RefreshMsgFlags;
import com.refinitiv.eta.codec.StateCodes;
import com.refinitiv.eta.codec.StatusMsg;
import com.refinitiv.eta.codec.StatusMsgFlags;
import com.refinitiv.eta.codec.StreamStates;
import com.refinitiv.eta.codec.UInt;
import com.refinitiv.eta.rdm.DomainTypes;
import com.refinitiv.eta.rdm.ElementNames;
import com.refinitiv.eta.rdm.Login;
import com.refinitiv.eta.transport.TransportBuffer;
import com.refinitiv.eta.transport.WriteArgs;
import com.refinitiv.eta.transport.WriteFlags;

/**
 * The Class Module_2_Login.
 */
public class Module_2_Login
{

    public static boolean loginRequestInfo_IsInUse;
    public static int loginRequestInfo_StreamId;
    public static String loginRequestInfo_Username;
    public static String loginRequestInfo_ApplicationId;
    public static String loginRequestInfo_ApplicationName;
    public static String loginRequestInfo_Position;
    public static String loginRequestInfo_Password;
    public static String loginRequestInfo_InstanceId;
    public static String loginRequestInfo_Role;

    public static int etaServerFDValue;
    public static int clientChannelFDValue;

    /**
     * The Enum LoginRejectReason.
     */
    public static enum LoginRejectReason
    {
        MAX_LOGIN_REQUESTS_REACHED, NO_USER_NAME_IN_REQUEST
    }

    /**
     * The main method.
     *
     * @param args the arguments
     */
    @SuppressWarnings("fallthrough")
	public static void main(String[] args)
    {
        /**************************************************************************************************
         * DECLARING VARIABLES
         **************************************************************************************************/
        /* Create a server to eventually accept connection requests */
        Server server = null;

        boolean clientAccepted = false;

        /* Create initialization arguments (InitArgs) to specify any initialization options for Transport */
        InitArgs initArgs = TransportFactory.createInitArgs();

        /* Create error to keep track of any errors that may occur in Transport methods */
        Error error = TransportFactory.createError();

        /* For this simple training app, the interactive provider only supports a single client. If the consumer disconnects,
         * the interactive provider would simply exit.
         *
         * If you want the provider to support multiple client sessions at the same time, you need to implement support
         * for multiple client sessions feature similar to what Provider example is doing.
         */
        /* Create a channel to keep track of connection */
        Channel channel = null;

        /* Create channel info as a holder */
        ChannelInfo channelInfo = TransportFactory.createChannelInfo();

        TransportBuffer msgBuf = null;

        long currentTime = 0;
        long etaRuntime = 0;
        long runTime = 0;

        /* Create decode iterator to decode the contents of the buffer */
        DecodeIterator decodeIter = CodecFactory.createDecodeIterator();

        /* Create an I/O notification mechanism (a Selector in our case) for our channel */
        Selector selector = null;

        /* Create and populate binding options to specify listening socket preferences */
        BindOptions bindOpts = TransportFactory.createBindOptions();

        /* Create accept options to specify any options for accepting */
        AcceptOptions acceptOpts = TransportFactory.createAcceptOptions();

        /* Create initialization progress info (InProgInfo) to keep track of channel initialization with Channel.init() */
        InProgInfo inProgInfo = TransportFactory.createInProgInfo();

        /* Create a bit mask to specify what I/O notification operations to keep track of (e.g. ACCEPT)*/
        short opMask = 0;

        /* Max waiting time */
        final int timeOutSeconds = 60;

        int retCode;

        /* the default option parameters */
        /* server is running on port number 14002 */
        String srvrPortNo = "14002";
        /* use default runTime of 300 seconds */
        runTime = 300;

        /* User specifies options such as address, port, and interface from the command line.
         * User can have the flexibility of specifying any or all of the parameters in any order.
         */
        if (args.length > 0)
        {
            int i = 0;

            while (i < args.length)
            {

                if ((args[i].equals("-p")) == true)
                {
                    i += 2;
                    srvrPortNo = args[i - 1];
                }
                else if ((args[i].equals("-r")) == true)
                {
                    i += 2;
                    runTime = Integer.parseInt(args[i - 1]);
                }
                else
                {
                    System.out.printf("Error: Unrecognized option: %s\n\n", args[i]);
                    System.out.printf("Usage: %s or\n%s [-p <SrvrPortNo>] [-r <runTime>]\n", args[0], args[0]);
                    System.exit(TransportReturnCodes.FAILURE);
                }
            }
        }

        /**************************************************************************************************
         * INITIALIZATION
         **************************************************************************************************/
        /*********************************************************
         * Server/Provider Application Life cycle Major Step 1: Initialize ETA
         * Transport using Initialize The first ETA Transport function that an
         * application should call. This creates and initializes internal memory
         * and structures, as well as performing any bootstrapping for
         * underlying dependencies. The Initialize function also allows the user
         * to specify the locking model they want applied to the ETA Transport.
         *********************************************************/

        if (Transport.initialize(initArgs, error) != TransportReturnCodes.SUCCESS)
        {
            System.out.printf("Error (%d) (errno: %d) %s\n", error.errorId(), error.sysError(), error.text());
            System.exit(TransportReturnCodes.FAILURE);
        }

        currentTime = System.currentTimeMillis();
        etaRuntime = currentTime + runTime * 1000;

        /* populate bind options, then pass to Bind function -
         * ETA Transport should already be initialized
         */
        /* Set bind options */
        bindOpts.serviceName(srvrPortNo); /* server is running on default port number 14002 */
        bindOpts.pingTimeout(60); /* servers desired ping timeout is 60 seconds, pings should be sent every 20 */
        bindOpts.minPingTimeout(30); /* min acceptable ping timeout is 30 seconds, pings should be sent every 10 */

        /* set up buffering, configure for shared and guaranteed pools */
        bindOpts.guaranteedOutputBuffers(1000);
        bindOpts.maxOutputBuffers(2000);
        bindOpts.sharedPoolSize(50000);
        bindOpts.sharedPoolLock(true);

        bindOpts.serverBlocking(false); /* perform non-blocking I/O */
        bindOpts.channelsBlocking(false); /* perform non-blocking I/O */
        bindOpts.compressionType(CompressionTypes.NONE); /* server does not desire compression for this connection */

        /* populate version and protocol with RWF information or protocol specific info */
        bindOpts.protocolType(Codec.protocolType()); /* Protocol type definition for RWF */
        bindOpts.majorVersion(Codec.majorVersion());
        bindOpts.minorVersion(Codec.minorVersion());
        /**************************************************************************************************
         * Bind and receive a server
         **************************************************************************************************/
        /* Bind ETA server */
        if ((server = Transport.bind(bindOpts, error)) == null)
        {
            System.out.printf("Error (%d) (errno: %d) encountered with Bind. Error Text: %s\n", error.errorId(), error.sysError(), error.text());
            /* End application, uninitialize to clean up first */
            Transport.uninitialize();
            System.exit(TransportReturnCodes.FAILURE);
        }

        etaServerFDValue = TrainingModuleUtils.getFDValueOfSelectableChannel(server.selectableChannel());
        System.out.printf("Server IPC descriptor = %d bound on port %d\n", etaServerFDValue, server.portNumber());

        opMask |= SelectionKey.OP_ACCEPT;

        /* Register our channel to the selector and watch for I/O notifications specified by our operation mask */
        try
        {
            selector = Selector.open();
            server.selectableChannel().register(selector, opMask, server);
        }
        catch (Exception e)
        {
            System.out.printf("Exception %s\n", e.getMessage());
            closeChannelServerCleanUpAndExit(channel, server, selector, TransportReturnCodes.FAILURE);
        }

        SelectionKey key = null;
        Set<SelectionKey> selectedKeys = null;
        Iterator<SelectionKey> keyIter = null;

        /**************************************************************************************************
         * MAIN LOOP - Listen for connection requests until we accept a client
         **************************************************************************************************/
        /* Main Loop #1 for detecting incoming client connections. */
        while (!clientAccepted)
        {
            /* Wait for any I/O notification updates in the channel for our specified amt of seconds (e.g. 60 sec.)*/
            try
            {
                selector.select(timeOutSeconds * 1000);
                /* Create an iterator from the selector's updated channels*/
                selectedKeys = selector.selectedKeys();
                keyIter = selectedKeys.iterator();

                /* If our channel has been updated */
                if (keyIter.hasNext())
                {
                    /* Check if channel is ACCEPT-able */
                    key = server.selectableChannel().keyFor(selector);
                    if (key.isAcceptable())
                    {
                        acceptOpts.nakMount(false);

                        /*****************************************
                         * Step 3) Accept the connection request *
                         *****************************************/
                        /*********************************************************
                         * Server/Provider Application Life cycle Major Step 3:
                         * Accept connection using Accept This step is performed
                         * per connected client/connection/channel Uses the
                         * Server that represents the listening socket
                         * connection and begins the accepting process for an
                         * incoming connection request. Returns an Channel that
                         * represents the client connection. In the event of an
                         * error, NULL is returned and additional information
                         * can be found in the Error structure. The Accept
                         * function can also begin the rejection process for a
                         * connection through the use of the AcceptOptions
                         * structure. Once a connection is established and
                         * transitions to the _CH_STATE_ACTIVE state, this
                         * Channel can be used for other transport operations.
                         *********************************************************/
                        /* An OMM Provider application can begin the connection accepting or rejecting process by using the Accept function */
                        if ((channel = server.accept(acceptOpts, error)) == null)
                        {
                            System.out.printf("Error (%d) (errno: %d) encountered with Init Channel fd=%d. Error Text: %s\n", error.errorId(), error.sysError(), clientChannelFDValue, error.text());
                            Transport.uninitialize();
                        }

                        else
                        {
                            /* For this simple training app, the interactive provider only supports one client session from the consumer. */
                            clientChannelFDValue = TrainingModuleUtils.getFDValueOfSelectableChannel(channel.selectableChannel());
                            System.out.printf("\nServer fd = %d: New client on Channel fd=%d\n", etaServerFDValue, clientChannelFDValue);
                            /*set clientAccepted to be TRUE and exit the while Main Loop #1*/
                            clientAccepted = true;
                        }
                    }
                }
            }
            catch (IOException e)
            {
                System.out.printf("Exception %s\n", e.getMessage());
                closeChannelServerCleanUpAndExit(channel, server, selector, TransportReturnCodes.FAILURE);
            }
        }

        /* Keep track of READ notifications */
        opMask = SelectionKey.OP_READ;

        /*****************************************************************************************
         * Loop 2) Keep calling Channel.init() until it has properly connected
         * us to the server *
         *****************************************************************************************/
        /* Main Loop #2 for getting connection active and successful completion of the initialization process
         * Currently, the main loop would exit if an error condition is triggered 
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
                System.out.printf("Exception: %s\n", e.getMessage());
                closeChannelServerCleanUpAndExit(channel, server, selector, TransportReturnCodes.FAILURE);
            }

            /* Wait for any I/O notification updates in the channel for our specified amt of seconds (e.g. 60 sec.)*/
            try
            {
                selector.select(timeOutSeconds * 1000);
                /* Create an iterator from the selector's updated channels*/
                selectedKeys = selector.selectedKeys();
                keyIter = selectedKeys.iterator();

                /* If our channel has not updated, we must have timed out */
                if (!keyIter.hasNext())
                {
                    System.out.printf("Channel initialization has timed out.\n");
                    closeChannelServerCleanUpAndExit(channel, server, selector, TransportReturnCodes.FAILURE);
                }
                else
                {
                    /* Check if channel is READ-able */
                    key = channel.selectableChannel().keyFor(selector);
                    if (key.isReadable())
                    {
                        /****************************************************************************
                         * Step 4) Call Channel.init() to progress channel
                         * initialization further. * * This method is called
                         * multiple times throughout the Loop 2, as it makes *
                         * more progress towards channel initialization. *
                         ***************************************************************************/
                        /* Internally, the ETA initialization process includes several actions. The initialization includes
                         * any necessary ETA connection handshake exchanges, including any HTTP or HTTPS negotiation.
                         * Compression, ping timeout, and versioning related negotiations also take place during the
                         * initialization process. This process involves exchanging several messages across the connection,
                         * and once all message exchanges have completed the Channel.state will transition. If the connection
                         * is accepted and all types of negotiations completed properly, the Channel state will become
                         * ACTIVE. If the connection is rejected, either due to some kind of negotiation failure
                         * or because an Server rejected the connection by setting nakMount to TRUE, the Channel state
                         * will become CLOSED.
                         *
                         * Note:
                         * For both client and server channels, more than one call to InitChannel can be required to complete
                         * the channel initialization process.
                         */
                        if ((retCode = channel.init(inProgInfo, error)) < TransportReturnCodes.SUCCESS)
                        {
                            System.out.printf("Error (%d) (errno: %d) encountered with Init Channel fd=%d. Error Text: %s\n", error.errorId(), error.sysError(), clientChannelFDValue, error.text());
                            closeChannelServerCleanUpAndExit(channel, server, selector, TransportReturnCodes.FAILURE);
                        }

                        /* Handle return code appropriately */
                        switch (retCode)
                        {
                            case TransportReturnCodes.CHAN_INIT_IN_PROGRESS:
                            {
                                /* Initialization is still in progress, check the InProgInfo for additional information */
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
                                    opMask = SelectionKey.OP_READ;
                                    final int oldChannelFDValue = clientChannelFDValue;
                                    clientChannelFDValue = TrainingModuleUtils.getFDValueOfSelectableChannel(channel.selectableChannel());
                                    System.out.printf("\nChannel In Progress - New FD: %d   Old FD: %d\n", clientChannelFDValue, oldChannelFDValue);
                                    try
                                    {
                                        key = inProgInfo.oldSelectableChannel().keyFor(selector);
                                        key.cancel();
                                    }
                                    catch (Exception e)
                                    {
                                        System.out.printf("Exception: %s\n", e.getMessage());
                                        closeChannelServerCleanUpAndExit(channel, server, selector, TransportReturnCodes.FAILURE);
                                    }
                                    try
                                    {
                                        channel.selectableChannel().register(selector, opMask, channel);
                                    }
                                    catch (Exception e)
                                    {
                                        System.out.printf("Exception: %s\n", e.getMessage());
                                        closeChannelServerCleanUpAndExit(channel, server, selector, TransportReturnCodes.FAILURE);
                                    }
                                }
                                else
                                {
                                    System.out.printf("Channel %d in progress...\n", clientChannelFDValue);
                                }
                            }
                                break;
                            /* channel connection becomes active!
                             * Once a connection is established and transitions to the ACTIVE state,
                             * this Channel can be used for other transport operations.
                             */
                            case TransportReturnCodes.SUCCESS:
                            {
                                System.out.printf("Channel on fd %d is now active - reading and writing can begin.\n", clientChannelFDValue);
                                /*********************************************************
                                 * Connection is now active. The Channel can be
                                 * used for all additional transport
                                 * functionality (e.g. reading, writing) now
                                 * that the state transitions to ACTIVE
                                 *********************************************************/

                                /* Populate information from channel */
                                if ((retCode = channel.info(channelInfo, error)) != TransportReturnCodes.SUCCESS)
                                {
                                    System.out.printf("Error (%d) (errno: %d) encountered with channel.info. Error Text: %s\n", error.errorId(), error.sysError(), error.text());
                                    /* Connection should be closed, return failure */
                                    /* Closes channel, closes server, cleans up and exits the application. */
                                    closeChannelServerCleanUpAndExit(channel, server, selector, TransportReturnCodes.FAILURE);
                                }

                                /* Print out basic channel info */
                                System.out.printf("\nChannel %d active. Channel Info:\n" + "Max Fragment Size:           %d\n" + "Output Buffers:              %d Max, %d Guaranteed\n" + "Input Buffers:               %d\n" + "Send/Receive Buffer Sizes:   %d/%d\n" + "Ping Timeout:                %d\n",
                                                  clientChannelFDValue,
                                                  channelInfo.maxFragmentSize(), /*  This is the max fragment size before fragmentation and reassembly is necessary. */
                                                  channelInfo.maxOutputBuffers(), /* This is the maximum number of output buffers available to the channel. */
                                                  channelInfo.guaranteedOutputBuffers(), /*  This is the guaranteed number of output buffers available to the channel. */
                                                  channelInfo.numInputBuffers(), /*  This is the number of input buffers available to the channel. */
                                                  channelInfo.sysSendBufSize(), /*  This is the systems Send Buffer size. This reports the systems send buffer size respective to the transport type being used (TCP, UDP, etc) */
                                                  channelInfo.sysRecvBufSize(), /*  This is the systems Receive Buffer size. This reports the systems receive buffer size respective to the transport type being used (TCP, UDP, etc) */
                                                  channelInfo.pingTimeout()); /* This is the value of the negotiated ping timeout */

                                System.out.printf("Connected component version: ");
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

                                System.out.printf("\n\n");

                                /*********************************************************
                                 * Server/Provider Application Life cycle Major
                                 * Step 7: Closes a listening socket associated
                                 * with an Server. This will release any pool
                                 * based resources back to their respective
                                 * pools, close the listening socket, and
                                 * perform any additional necessary cleanup. Any
                                 * established connections will remain open,
                                 * allowing for continued information exchange.
                                 *********************************************************/

                                /* clean up server using CloseServer call.
                                 * If a server is being shut down, the CloseServer function should be used to close the listening socket and perform
                                 * any necessary cleanup. All currently connected Channels will remain open. This allows applications to continue
                                 * to send and receive data, while preventing new applications from connecting. The server has the option of calling
                                 * CloseChannel to shut down any currently connected applications.
                                 * When shutting down the ETA Transport, the application should release any unwritten pool buffers.
                                 * The listening socket can be closed by calling CloseServer. This prevents any new connection attempts.
                                 * If shutting down connections for all connected clients, the provider should call CloseChannel for each connection client.
                                */
                                if ((server == null) & (server.close(error) < TransportReturnCodes.SUCCESS))
                                {
                                    System.out.printf("Error (%d) (errno: %d) encountered with CloseServer.  Error Text : %s\n", error.errorId(), error.sysError(), error.text());

                                    /* End application, uninitialize to clean up first */
                                    Transport.uninitialize();
                                    System.exit(TransportReturnCodes.FAILURE);
                                }

                                /*set server to be null*/
                                server = null;
                            }
                                break;
                            default: /* Error handling */
                            {
                                System.out.printf("Bad return value fd=%d: <%s>\n", clientChannelFDValue, TransportReturnCodes.toString(retCode));
                                closeChannelServerCleanUpAndExit(channel, server, selector, TransportReturnCodes.FAILURE);
                            }
                                break;
                        }
                    }
                }
            }
            catch (IOException e1)
            {
                System.out.printf("Exception %s\n", e1.getMessage());
                closeChannelServerCleanUpAndExit(channel, server, selector, TransportReturnCodes.FAILURE);
            }
        }

        /* Keep track of READ notifications */
        opMask |= SelectionKey.OP_READ;

        /* Initialize ping management handler */
        initPingManagement(channel);
        /* Clears the login request information */
        clearLoginRequestInfo();

        /****************************************************
         * Loop 3) Check ping operations until runtime ends *
         ****************************************************/
        /* Here were are using a new Main loop #3. An alternative design would be to combine this Main loop #3 (message processing)
         * with the other 2 earlier Main loops, namely, Main Loop #1 (detecting incoming client connections), and
         * Main Loop #2 (getting connection active and successful completion of the initialization process) as a single provider Main Loop.
         * Some bookkeeping would be required for that approach.
         */

        /* Main Loop #3 for message processing (reading data, writing data, and ping management, etc.)
         * The only way to exit this Main loop is when an error condition is triggered or after
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
                closeChannelServerCleanUpAndExit(channel, server, selector, TransportReturnCodes.FAILURE);
            }

            /* Wait 1 second for any I/O notification updates in the channel */
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
                         * Loop 4) Read and decode for all buffers in channel *
                         ******************************************************/
                        while (retCode > TransportReturnCodes.SUCCESS)
                        {
                            /* Create read arguments (ReadArgs) to keep track of the statuses of any reads */
                            ReadArgs readArgs = TransportFactory.createReadArgs();

                            /**************************************************
                             * Step 5) Read message from channel into buffer *
                             **************************************************/
                            msgBuf = channel.read(readArgs, error);

                            if (msgBuf != null)
                            {
                                /* if a buffer is returned, we have data to process and code is success */

                                /* Processes a response from the channel/connection. This consists of performing a high level decode of the message and then
                                 * calling the applicable specific function for further processing.
                                 */

                                /* No need to clear the message before we decode into it. ETA Decoding populates all message members (and that is true for any
                                 * decoding with ETA, you never need to clear anything but the iterator)
                                 */
                                /* We have data to process */

                                /* Create message to represent buffer data */
                                Msg msg = CodecFactory.createMsg();

                                /* This ClearDecodeIterator clear iterator function should be used to achieve the best performance while clearing the iterator. */
                                /* Clears members necessary for decoding and readies the iterator for reuse. You must clear DecodeIterator
                                 * before decoding content. For performance purposes, only those members required for proper functionality are cleared.
                                 */
                                decodeIter.clear();
                                /* Set the RWF version to decode with this iterator */
                                int ret = decodeIter.setBufferAndRWFVersion(msgBuf, channel.majorVersion(), channel.minorVersion());
                                /* Associates the DecodeIterator with the Buffer from which to decode. */
                                if ((ret) != CodecReturnCodes.SUCCESS)
                                {
                                    System.out.printf("\nSetDecodeIteratorBuffer() failed with return code: %d\n", ret);
                                    /* Closes channel, closes server, cleans up and exits the application. */
                                    closeChannelServerCleanUpAndExit(channel, server, selector, TransportReturnCodes.FAILURE);
                                }
                                /******************************************
                                 * Step 6) Decode buffer message *
                                 ******************************************/
                                /* decode contents into the Msg structure */
                                if ((retCode = msg.decode(decodeIter)) != CodecReturnCodes.SUCCESS)
                                {
                                    System.out.printf("Error (%d) (errno: %d) encountered with Init Channel fd=%d. Error Text: %s\n", error.errorId(), error.sysError(), clientChannelFDValue, error.text());
                                    /* Closes channel, closes server, cleans up and exits the application. */
                                    closeChannelServerCleanUpAndExit(channel, server, selector, TransportReturnCodes.FAILURE);
                                }

                                /* Deduce an action based on the domain type of the message */
                                switch (msg.domainType())
                                {
                                    /* (1) Login Message */
                                    case DomainTypes.LOGIN:
                                    {
                                        try
                                        {
                                            if ((retCode = processLoginRequest(channel, msg, decodeIter, error)) > TransportReturnCodes.SUCCESS)
                                            {
                                                /* There is still data left to flush, leave our write notification enabled so we get called again.
                                                 * If everything wasn't flushed, it usually indicates that the TCP output buffer cannot accept more yet 
                                                 */

                                                /* set write flag if there's still other data queued */
                                                /* flush is done by application */
                                                opMask |= SelectionKey.OP_WRITE;
                                            }
                                            else if (retCode < TransportReturnCodes.SUCCESS)
                                            {
                                                clearLoginRequestInfo();
                                                closeChannelServerCleanUpAndExit(channel, server, selector, TransportReturnCodes.FAILURE);
                                            }
                                        }
                                        catch (UnknownHostException e)
                                        {
                                            e.printStackTrace();
                                        }
                                    }
                                        break;
                                    default: /* Error handling */
                                    {
                                        System.out.printf("Unhandled Domain Type: %d\n", msg.domainType());
                                    }
                                        break;
                                }

                                /* Acknowledge that a ping has been received */
                                /* set flag for server message received */
                                receivedClientMsg = true;
                                System.out.printf("Ping message has been received successfully from the client due to data message ...\n\n");
                            }
                            else
                            {
                                /* Deduce an action from the return code of Channel.read() */
                                retCode = readArgs.readRetVal();
                                /* keep track of the return values from read so data is not stranded in the input buffer.
                                 * Handle return codes appropriately, not all return values are failure conditions
                                 */
                                switch (retCode)
                                {
                                    /* Acknowledge that a ping has been received */
                                    case TransportReturnCodes.READ_PING:
                                        /* Update ping monitor */
                                        /* set flag for server message received */
                                        receivedClientMsg = true;
                                        System.out.printf("Ping message has been received successfully from the client due to ping message ...\n\n");
                                        break;

                                    /* Switch to a new channel if required */
                                    case TransportReturnCodes.READ_FD_CHANGE:
                                        opMask = SelectionKey.OP_READ;
                                        final int oldChannelFDValue = clientChannelFDValue;
                                        clientChannelFDValue = TrainingModuleUtils.getFDValueOfSelectableChannel(channel.selectableChannel());
                                        System.out.printf("\nChannel In Progress - New FD: %d   Old FD: %d\n", clientChannelFDValue, oldChannelFDValue);
                                        try
                                        {
                                            key = channel.selectableChannel().keyFor(selector);
                                            key.cancel();
                                        }
                                        catch (Exception e)
                                        {
                                            System.out.printf("Exception %s\n", e.getMessage());
                                            closeChannelServerCleanUpAndExit(channel, server, selector, TransportReturnCodes.FAILURE);
                                        }
                                        try
                                        {
                                            channel.selectableChannel().register(selector, opMask, channel);
                                        }
                                        catch (Exception e)
                                        {
                                            System.out.printf("Exception %s\n", e.getMessage());
                                            closeChannelServerCleanUpAndExit(channel, server, selector, TransportReturnCodes.FAILURE);
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
                                        System.out.printf("Error (%d) (errno: %d) channelInactive Error Text: %s\n", error.errorId(), error.sysError(), error.text());
                                    default: /* Error handling */
                                        if (retCode < 0)
                                        {
                                            System.out.printf("Error (%d) (errno: %d) encountered with Read Error Text: %s\n", error.errorId(), error.sysError(), error.text());
                                            closeChannelServerCleanUpAndExit(channel, server, selector, TransportReturnCodes.FAILURE);
                                        }
                                        break;
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
                                        System.out.printf("Exception %s\n", e.getMessage());
                                        System.exit(TransportReturnCodes.FAILURE);
                                    }
                                }
                                    break;
                                case TransportReturnCodes.FAILURE:
                                default: /* Error handling */
                                {
                                    System.out.printf("Error (%d) (errno: %d) encountered with Init Channel fd=%d. Error Text: %s\n", error.errorId(), error.sysError(), clientChannelFDValue, error.text());
                                    /* Connection should be closed, return failure */
                                    /* Closes channel/connection, cleans up and exits the application. */
                                    closeChannelServerCleanUpAndExit(channel, server, selector, TransportReturnCodes.FAILURE);
                                }
                            }
                        }
                    }
                }

                /* Processing ping management handler */
                if ((retCode = processPingManagementHandler(channel)) > TransportReturnCodes.SUCCESS)
                {
                    /* There is still data left to flush, leave our write notification enabled so we get called again.
                     * If everything wasn't flushed, it usually indicates that the TCP output buffer cannot accept more yet
                     */

                    /* set write if there's still other data queued */
                    /* flush is done by application */
                    opMask |= SelectionKey.OP_WRITE;
                }
                else if (retCode < TransportReturnCodes.SUCCESS)
                {
                    /* Closes channel, cleans up and exits the application. */
                    closeChannelServerCleanUpAndExit(channel, server, selector, TransportReturnCodes.FAILURE);
                }

                /* get current time */
                currentTime = System.currentTimeMillis();

                /* If the runtime has expired */
                if (System.currentTimeMillis() >= etaRuntime)
                { /* Closes all streams for the Interactive Provider after run-time has elapsed in our simple Interactive Provider example.
                   * If the provider application must shut down, it can either leave consumer connections intact or shut them down.
                   */

                    System.out.printf("ETA Server run-time has expired...\n\n");
                    closeChannelServerCleanUpAndExit(channel, server, selector, TransportReturnCodes.SUCCESS);
                }
            }
            catch (IOException e1)
            {
                System.out.printf("Exception %s\n", e1.getMessage());
                closeChannelServerCleanUpAndExit(channel, server, selector, TransportReturnCodes.FAILURE);
            }

        }
    }

    /**
     * *******************************************************
     * Closes channel and selector and exits application. *
     *
     * @param channel - Channel to be closed *
     * @param server - server to be closed *
     * @param selector - Selector to be closed
     * @param code - if exit is due to errors/exceptions *
     * *******************************************************
     */
    public static void closeChannelServerCleanUpAndExit(Channel channel, Server server, Selector selector, int code)
    {
        boolean isClosedAndClean = true;
        Error error = TransportFactory.createError();
        try
        {
            selector.close();
        }
        catch (Exception e)
        {
            System.out.printf("Exception %s\n", e.getMessage());
        }

        /*********************************************************
         * Server/Provider Application Life cycle Major Step 6: Close connection
         * using CloseChannel (OS connection release handshake) CloseChannel
         * closes the server based Channel. This will release any pool based
         * resources back to their respective pools, close the connection, and
         * perform any additional necessary cleanup. When shutting down the ETA
         * Transport, the application should release all unwritten pool buffers.
         * Calling CloseChannel terminates the connection for each connection
         * client.
         *********************************************************/
        if ((channel != null)) {
            isClosedAndClean = channel.close(error) >= TransportReturnCodes.SUCCESS;
        }

        /*********************************************************
         * Server/Provider Application Life cycle Major Step 7: Closes a
         * listening socket associated with an Server. This will release any
         * pool based resources back to their respective pools, close the
         * listening socket, and perform any additional necessary cleanup. Any
         * established connections will remain open, allowing for continued
         * information exchange. If desired, the server can use CloseChannel to
         * shutdown any remaining connections.
         *********************************************************/
        /* clean up server using CloseServer call.
         * If a server is being shut down, the CloseServer function should be used to close the listening socket and perform
         * any necessary cleanup. All currently connected Channels will remain open. This allows applications to continue
         * to send and receive data, while preventing new applications from connecting. The server has the option of calling
         * CloseChannel to shut down any currently connected applications.
         * When shutting down the ETA Transport, the application should release any unwritten pool buffers.
         * The listening socket can be closed by calling CloseServer. This prevents any new connection attempts.
         * If shutting down connections for all connected clients, the provider should call CloseChannel for each connection client.
        */
        if ((server != null)) {
            isClosedAndClean &= server.close(error) >= TransportReturnCodes.SUCCESS;
        }

        /*********************************************************
         * Server/Provider Application Life cycle Major Step 8: Uninitialize ETA
         * Transport using Uninitialize The last ETA Transport function that an
         * application should call. This uninitialized internal data structures
         * and deletes any allocated memory.
         *********************************************************/
        /* All ETA Transport use is complete, must uninitialize.
         * The uninitialization process allows for any heap allocated memory to be cleaned up properly.
         */
        Transport.uninitialize();

        if (isClosedAndClean) {
            System.out.println("Provider application has closed channel and has cleaned up successfully.");
        } else {
            System.out.printf("Error (%d) (errno: %d) encountered with Init Channel fd=%d. Error Text: %s\n",
                error.errorId(), error.sysError(), clientChannelFDValue, error.text());
        }

        /* For applications that do not exit due to errors/exceptions such as:
         * Exits the application if the run-time has expired.
         */
        if (code == TransportReturnCodes.SUCCESS)
        {
            System.out.printf("\nETA Interactive Provider Training Application successfully ended.\n");
        }

        /* End application */
        System.exit(0);
    }

    /*********************************************************
     * Initializes the ping times for etaChannel.
     * 
     * @param Channel - The channel for ping management info initialization
     *********************************************************/
    static int pingTimeoutServer; /* server ping timeout */
    static int pingTimeoutClient; /* client ping timeout */
    static long nextReceivePingTime; /* time client should receive next message/ping from server */
    static long nextSendPingTime; /* time to send next ping from client */
    static boolean receivedClientMsg; /* flag for server message received */

    /**
     * Inits the ping management.
     *
     * @param channel the channel
     */
    public static void initPingManagement(Channel channel)
    {
        /* get current time */
        long currentTime = System.currentTimeMillis();

        /* set ping timeout for server and client */
        /* Applications are able to configure their desired pingTimeout values, where the ping timeout is the point at which a connection
         * can be terminated due to inactivity. Heartbeat messages are typically sent every one-third of the pingTimeout, ensuring that
         * heartbeats are exchanged prior to a timeout occurring. This can be useful for detecting loss of connection prior to any kind of
         * network or operating system notification that may occur.
         */
        pingTimeoutServer = channel.pingTimeout() / 3;
        pingTimeoutClient = channel.pingTimeout();

        /* set time to send next ping to remote connection */
        nextSendPingTime = currentTime + pingTimeoutServer * 1000;

        /* set time should receive next ping from remote connection */
        nextReceivePingTime = currentTime + pingTimeoutClient * 1000;

        receivedClientMsg = false;

    }

    /**
     * *******************************************************
     * Processing ping management handler.
     *
     * @param channel - The channel for ping management processing
     * *******************************************************
     * @return the int
     */
    public static int processPingManagementHandler(Channel channel)
    {
        /* Handles the ping processing for etaChannel. Sends a ping to the client if the next send ping time has arrived and
         * checks if a ping has been received from the client within the next receive ping time.
         */
        int retval = TransportReturnCodes.SUCCESS;
        Error error = TransportFactory.createError();

        /* get current time */
        long currentTime = System.currentTimeMillis();

        /* handle server pings */
        if (currentTime >= nextSendPingTime)
        {
            /* send ping to client */
            /*********************************************************
             * Server/Provider Application life cycle Major Step 5: Ping using
             * Ping Attempts to write a heartbeat message on the connection.
             * This function expects the Channel to be in the active state. If
             * an application calls the Ping function while there are other
             * bytes queued for output, the ETA Transport layer will suppress
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
                        System.out.printf("Ping message has been sent successfully to the client ...\n\n");
                    }
                        break;
                    case TransportReturnCodes.FAILURE: /* fall through to default. */
                    default: /* Error handling */
                    {
                        System.out.printf("Error (%d) (errno: %d) encountered with Ping(). Error Text:%s\n", error.errorId(), error.sysError(), error.text());
                        /* Closes channel/connection, cleans up and exits the application. */
                        return TransportReturnCodes.FAILURE;
                    }
                }
            }

            /* set time to send next ping from server */
            nextSendPingTime = currentTime + (pingTimeoutServer * 1000);
        }

        /* handle server pings - an application should determine if data or pings have been received,
         * if not application should determine if pingTimeout has elapsed, and if so connection should be closed
         */
        if (currentTime >= nextReceivePingTime)
        {
            /* Check if received message from remote (connection) since last time */
            if (receivedClientMsg)
            {
                /* Reset flag for remote message received */
                receivedClientMsg = false;

                /* Set time should receive next message/ping from remote (connection)  */
                nextReceivePingTime = currentTime + (pingTimeoutClient * 1000);
            }
            else /* lost contact with server */
            {
                /* Lost contact with remote (connection) */
                error.text("Lost contact with client...\n");
                System.out.printf("Error (%d) (errno: %d) %s\n", error.errorId(), error.sysError(), error.text());
                /* Closes channel/connection, cleans up and exits the application. */
                return TransportReturnCodes.FAILURE;

            }
        }
        return retval;
    }

    /**
     * ************************************************************
     * Sends a message buffer to the channel *.
     *
     * @param channel - the Channel to send the message buffer to *
     * @param msgBuf - the buffer to be sent *
     * @return status code *
     * ************************************************************
     */
    public static int sendMessage(Channel channel, TransportBuffer msgBuf)
    {
        int retCode = 0;
        Error error = TransportFactory.createError();
        WriteArgs writeArgs = TransportFactory.createWriteArgs();
        writeArgs.flags(WriteFlags.NO_FLAGS);
        /* send the request */

        /*********************************************************
         * Server/Provider Application Life cycle Major Step 5: Write using
         * Writer Writer performs any writing or queuing of data. This function
         * expects the Channel to be in the active state and the buffer to be
         * properly populated, where length reflects the actual number of bytes
         * used. This function allows for several modifications to be specified
         * for this call. Here we use WriteFlags.NO_FLAGS. For more information
         * on other flag enumeration such as WriteFlags.DO_NOT_COMPRESS or
         * WriteFlags.DIRECT_SOCKET_WRITE, see the ETA C developers guide for
         * Write Flag Enumeration Values supported by ETA Transport.
         *
         * The ETA Transport also supports writing data at different priority
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

        /* Now write the data - keep track of ETA Transport return code -
         * Because positive values indicate bytes left to write, some negative transport layer return codes still indicate success
         */

        if ((retCode = channel.write(msgBuf, writeArgs, error)) == TransportReturnCodes.WRITE_CALL_AGAIN)
        {
            /* (-10) Transport Success: Write is fragmenting the buffer and needs to be called again with the same buffer. This indicates that Write was
             * unable to send all fragments with the current call and must continue fragmenting
             */

            /* Large buffer is being split by transport, but out of output buffers. Schedule a call to Flush and then call the Write function again with
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
            /* The write was successful and there is more data queued in ETA Transport. The Channel.flush() method should be used to continue attempting to flush data 
             * to the connection. ETA will release buffer.
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
                    /* Continue with next operations. ETA will release buffer.*/
                }
                    break;
                case TransportReturnCodes.NO_BUFFERS:
                {
                    channel.releaseBuffer(msgBuf, error);
                }
                    break;
                case TransportReturnCodes.WRITE_FLUSH_FAILED:
                {
                    if (channel.state() == ChannelState.CLOSED)
                    {
                        /* Channel is Closed - This is terminal. Treat as error, and buffer must be released - fall through to default. */
                    }
                    else
                    {
                        /* Channel.write() internally attempted to flush data to the connection but was blocked. This is not a failure and the user should not release their buffer."; 
                        /* Successful write call, data is queued. The Channel.flush() method should be used to continue attempting to flush data to the connection. */

                        /* Set write flag if flush failed */
                        /* Flush needs to be done by application */

                        /* Channel is still open, but Channel.write() tried to flush internally and failed. 
                         * Return positive value so the caller knows there's bytes to flush. 
                         */
                        return TransportReturnCodes.SUCCESS + 1;
                    }
                    break;
                }
                case TransportReturnCodes.FAILURE:
                default:
                {
                    System.out.printf("Error (%d) (errno: %d) encountered with Init Channel fd=%d. Error Text: %s\n", error.errorId(), error.sysError(), clientChannelFDValue, error.text());
                    channel.releaseBuffer(msgBuf, error);
                    return TransportReturnCodes.FAILURE;
                }
            }
        }

        return retCode;
    }

    /**
     * ************************************************************
     * Processes a login request *.
     *
     * @param channel - the Channel of connection *
     * @param msg - the partially decoded message *
     * @param decIter - the decode iterator *
     * @param error - tracks error info *
     * @return status code *
     * ************************************************************
     * @throws UnknownHostException the unknown host exception
     */
    public static int processLoginRequest(Channel channel, Msg msg, DecodeIterator decIter, Error error) throws UnknownHostException
    {
        MsgKey requestKey = null;

        int retCode;

        ElementList elementList = CodecFactory.createElementList();
        ElementEntry element = CodecFactory.createElementEntry();

        /* Switch cases depending on message class */
        switch (msg.msgClass())
        {
            case MsgClasses.REQUEST:
            { /* get request message key - retrieve the MsgKey structure from the provided decoded message structure */
                requestKey = msg.msgKey();

                /* check if key has user name */
                /* user name is only login user type accepted by this application (user name is the default type) */
                if (((requestKey.flags() & MsgKeyFlags.HAS_NAME) == 0) || (((requestKey.flags() & MsgKeyFlags.HAS_NAME_TYPE) != 0) && (requestKey.nameType() != Login.UserIdTypes.NAME)))
                {
                    if ((retCode = sendLoginRequestRejectStatusMsg(channel, msg.streamId(), LoginRejectReason.NO_USER_NAME_IN_REQUEST)) != TransportReturnCodes.SUCCESS)
                    {
                        return TransportReturnCodes.FAILURE;
                    }
                    break;
                }

                if (loginRequestInfo_IsInUse && (loginRequestInfo_StreamId != msg.streamId()))
                {
                    if ((retCode = sendLoginRequestRejectStatusMsg(channel, msg.streamId(), LoginRejectReason.MAX_LOGIN_REQUESTS_REACHED)) != TransportReturnCodes.SUCCESS)
                    {
                        return TransportReturnCodes.FAILURE;
                    }
                    break;
                }

                /* decode login request */

                /* get StreamId */
                loginRequestInfo_StreamId = msg.streamId();
                loginRequestInfo_IsInUse = true;

                /* get Username */
                if (requestKey.name().data() == null)
                {
                    loginRequestInfo_Username = null;
                }
                else
                {
                    loginRequestInfo_Username = requestKey.name().toString();
                }

                /* decode key opaque data */

                /**
                 * @brief Allows the user to continue decoding of any message
                 *        key attributes with the same \ref DecodeIterator used
                 *        when calling DecodeMsg
                 *
                 *        Typical use:<BR>
                 *        1. Call DecodeMsg()<BR>
                 *        2. If there are any message key attributes and the
                 *        application wishes to decode them using the same \ref
                 *        DecodeIterator, call decodeKeyAttrib() and continue
                 *        decoding using the appropriate container type decode
                 *        functions, as indicated by
                 *        MsgKey::attribContainerType<BR>
                 *        3. If payload is present and the application wishes to
                 *        decode it, use the appropriate decode functions, as
                 *        specified in \ref MsgBase::containerType<BR>
                 */
                if ((retCode = msg.decodeKeyAttrib(decIter, requestKey)) != CodecReturnCodes.SUCCESS)
                {
                    return TransportReturnCodes.FAILURE;
                }

                /* decode element list */

                /**
                 * @brief Decodes an ElementList container
                 *
                 *        Typical use:<BR>
                 *        1. Call elementList.decode()<BR>
                 *        2. Call ElementEntry.decode until error or
                 *        ::END_OF_CONTAINER is returned.<BR>
                 */
                if ((retCode = elementList.decode(decIter, null)) == CodecReturnCodes.SUCCESS)
                {
                    /* decode each element entry in list */
                    while ((retCode = element.decode(decIter)) != CodecReturnCodes.END_OF_CONTAINER)
                    {
                        /* get login request information */

                        if (retCode == CodecReturnCodes.SUCCESS)
                        {
                            /* ApplicationId */
                            if (element.name().equals(ElementNames.APPID))
                            {
                                loginRequestInfo_ApplicationId = element.encodedData().toString();
                            }
                            /* ApplicationName */
                            else if (element.name().equals(ElementNames.APPNAME))
                            {
                                loginRequestInfo_ApplicationName = element.encodedData().toString();
                            }
                            /* Position */
                            else if (element.name().equals(ElementNames.POSITION))
                            {
                                loginRequestInfo_Position = element.encodedData().toString();
                            }
                            /* Password */
                            else if (element.name().equals(ElementNames.PASSWORD))
                            {
                                loginRequestInfo_Password = element.encodedData().toString();
                            }
                            /* InstanceId */
                            else if (element.name().equals(ElementNames.INST_ID))
                            {
                                loginRequestInfo_InstanceId = element.encodedData().toString();
                            }
                            /* Role */
                            else if (element.name().equals(ElementNames.ROLE))
                            {
                                loginRequestInfo_Role = element.encodedData().toString();
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

                System.out.printf("Received Login Request for Username: %s\n", loginRequestInfo_Username);
                /* send login response */
                if ((retCode = sendLoginResponse(channel)) != TransportReturnCodes.SUCCESS)
                {
                    return retCode;
                }
            }
                break;

            case MsgClasses.CLOSE:
            {
                System.out.printf("Received Login Close for StreamId %d\n", msg.streamId());
                /* close login stream */
                closeLoginStream(msg.streamId());
            }
                break;

            default:
            {
                System.out.printf("Received Unhandled Login Msg Class: %d\n", msg.msgClass());
                return TransportReturnCodes.FAILURE;
            }
        }

        return TransportReturnCodes.SUCCESS;
    }

    /**************************************************************
     * Sends the login refresh response to channel *
     * 
     * @param channel - the Channel of connection *
     **************************************************************/
    static UInt supportBatchRequests = CodecFactory.createUInt();

    /**
     * Send login response.
     *
     * @param channel the channel
     * @return the int
     * @throws UnknownHostException the unknown host exception
     */
    public static int sendLoginResponse(Channel channel) throws UnknownHostException
    {
        int retCode;
        TransportBuffer msgBuf = null;
        Error error = TransportFactory.createError();
        /* Populate and encode a refreshMsg */
        RefreshMsg refreshMsg = (RefreshMsg)CodecFactory.createMsg();

        /* ETA provides clear functions for its structures as well as static initializers. 
         * These functions are tuned to be efficient and avoid initializing unnecessary
         * structure members, and allow for optimal structure use and reuse. In general, Refinitiv recommends that
         * you use the clear functions over static initializers, because the clear functions are more efficient.
         */
        /* Iterator used for encoding throughout the application - we can clear it and reuse it instead of recreating it */
        EncodeIterator encIter = CodecFactory.createEncodeIterator(); /* the encode iterator is created (typically stack allocated)  */

        ElementList elementList = CodecFactory.createElementList();
        ElementEntry elementEntry = CodecFactory.createElementEntry();

        Buffer applicationId = CodecFactory.createBuffer();
        Buffer applicationName = CodecFactory.createBuffer();
        Buffer applicationPosition = CodecFactory.createBuffer();

        elementList.clear();
        elementEntry.clear();

        /* Create channel info as a holder */
        ChannelInfo channelInfo = TransportFactory.createChannelInfo();

        /* Populate information from channel */
        if ((retCode = channel.info(channelInfo, error)) != TransportReturnCodes.SUCCESS)
        {
            return TransportReturnCodes.FAILURE;
        }

        /* Get a buffer of the channel max fragment size */

        /* etaGetBuffer() is the utility function that does 2-pass (more robust) getting non-packable buffer. */
        if ((msgBuf = etaGetBuffer(channel, channelInfo.maxFragmentSize(), error)) == null)
        {
            /* Connection should be closed, return failure */
            /* Closes channel, closes server, cleans up and exits the application. */
            return TransportReturnCodes.FAILURE;
        }

        /* clear encode iterator for reuse - this should be used to achieve the best performance while clearing the iterator. */
        encIter.clear();
        /* set version information of the connection on the encode iterator so proper versioning can be performed */
        /* set the buffer on an EncodeIterator */
        retCode = encIter.setBufferAndRWFVersion(msgBuf, channel.majorVersion(), channel.minorVersion());
        if ((retCode) < TransportReturnCodes.SUCCESS)
        {
            channel.releaseBuffer(msgBuf, error);
            System.out.printf("\nSetEncodeIteratorBuffer() failed with return code: %d\n", retCode);
            /* Closes channel, closes server, cleans up and exits the application. */
            return TransportReturnCodes.FAILURE;
        }

        /* initialize refresh message */

        refreshMsg.clear();
        /* provide login refresh response information */

        /* set refresh flags */

        /* set-up message */
        refreshMsg.msgClass(MsgClasses.REFRESH);/* (2) Refresh Message */
        refreshMsg.domainType(DomainTypes.LOGIN);/* (1) Login Message */
        refreshMsg.containerType(DataTypes.NO_DATA);/* (128) No Data <BR>*/
        /* (0x0008) The RefreshMsg has a message key, contained in \ref RefreshMsg::msgBase::msgKey. */
        /* (0x0020) Indicates that this RefreshMsg is a solicited response to a consumer's request. */
        /* (0x0040) Indicates that this is the final part of a refresh. This flag should be set on both single-part response messages, as well as the final message in a multi-part response message sequence. */
        /* (0x0100) Indicates that any cached header or payload information associated with the RefreshMsg's item stream should be cleared. */
        refreshMsg.flags(RefreshMsgFlags.HAS_MSG_KEY | RefreshMsgFlags.SOLICITED | RefreshMsgFlags.REFRESH_COMPLETE | RefreshMsgFlags.CLEAR_CACHE);
        /* (1) Stream is open (typically implies that information will be streaming, as information changes updated information will be sent on the stream, after final RefreshMsg or StatusMsg) */
        refreshMsg.state().streamState(StreamStates.OPEN);
        refreshMsg.state().dataState(DataStates.OK);
        refreshMsg.state().code(StateCodes.NONE);

        String stateText = "Login accepted by host ";
        String hostName = null;

        if ((hostName = InetAddress.getLocalHost().getHostName()) != null)
        {
            hostName = "localhost";
        }

        stateText += hostName;

        refreshMsg.state().text().data(stateText);
        /* provide login response information */

        /* StreamId - just set the Login response stream id info to be the same as the Login request stream id info */
        refreshMsg.streamId(loginRequestInfo_StreamId);
        /* set msgKey members */
        /* (0x0020) This MsgKey has additional attribute information, contained in \ref MsgKey::encAttrib. The container type of the attribute information is contained in \ref MsgKey::attribContainerType. */
        /* (0x0004) This MsgKey has a nameType enumeration, contained in \ref MsgKey::nameType. */
        /* (0x0002) This MsgKey has a name buffer, contained in \ref MsgKey::name.  */
        refreshMsg.msgKey().flags(MsgKeyFlags.HAS_ATTRIB | MsgKeyFlags.HAS_NAME_TYPE | MsgKeyFlags.HAS_NAME);

        /* Username */
        refreshMsg.msgKey().name().data(loginRequestInfo_Username);
        refreshMsg.msgKey().nameType(Login.UserIdTypes.NAME);
        refreshMsg.msgKey().attribContainerType(DataTypes.ELEMENT_LIST);

        /* encode message */

        /* since our msgKey has opaque that we want to encode, we need to use refreshMsg.encode */

        /* refreshMsg.encode should return and inform us to encode our key opaque */

        /**
         * @brief Begin encoding process for an Msg.
         *
         *        Begins encoding of an Msg<BR>
         *        Typical use:<BR>
         *        1. Populate desired members on the Msg<BR>
         *        2. Call refreshMsg.encodeInit() to begin message encoding<BR>
         *        3. If the Msg requires any message key attributes, but they
         *        are not pre-encoded and populated on the MsgKey::encAttrib,
         *        the refreshMsg.encodeInit() function will return
         *        ::ENCODE_MSG_KEY_OPAQUE. Call appropriate encode functions, as
         *        indicated by MsgKey::attribContainerType. When attribute
         *        encoding is completed, followed with
         *        refreshMsg.encodeMsgKeyAttribComplete() to continue with
         *        message encoding<BR>
         *        4. If the Msg requires any extended header information, but it
         *        is not pre-encoded and populated in the extendedHeader \ref
         *        Buffer, the refreshMsg.encodeInit() (or when also encoding
         *        attributes, the EncodeMsgKeyAttribComplete()) function will
         *        return ::ENCODE_EXTENDED_HEADER. Call any necessary extended
         *        header encoding functions; when completed call
         *        EncodeExtendedHeaderComplete() to continue with message
         *        encoding<BR>
         *        5. If the Msg requires any payload, but it is not pre-encoded
         *        and populated in the \ref encDataBody, the
         *        refreshMsg.encodeInit() (or when encoding message key
         *        attributes or extended header,
         *        refreshMsg.encodeKeyAttribComplete() or
         *        refreshMsg.encodeExtendedHeaderComplete() ) function will
         *        return ::ENCODE_CONTAINER. Call appropriate payload encode
         *        functions, as indicated by \ref ::containerType. If no payload
         *        is required or it is provided as pre-encoded, this function
         *        will return ::SUCCESS<BR>
         *        6. Call EncodeMsgComplete() when all content is completed<BR>
         */
        if ((retCode = refreshMsg.encodeInit(encIter, 0)) < CodecReturnCodes.SUCCESS)
        {
            channel.releaseBuffer(msgBuf, error);
            System.out.printf("\nEncodeMsgInit failed with return code: %d\n", retCode);
            /* Closes channel, closes server, cleans up and exits the application. */
            return TransportReturnCodes.FAILURE;
        }
        /* encode our msgKey opaque */

        /* encode the element list */
        elementList.flags(ElementListFlags.HAS_STANDARD_DATA);/* (0x08) The ElementList contains standard encoded content (e.g. not set defined). */

        /**
         * @brief Begin encoding process for ElementList container type.
         *
         *        Begins encoding of an ElementList<BR>
         *        Typical use:<BR>
         *        1. Call elementList.encodeInit()<BR>
         *        2. To encode entries, call elementEntry.encode()
         *        orelementEntry.encodeInit()..elementEntry.encodeComplete() for
         *        each elementEntry<BR>
         *        3. Call elementList.encodeComplete() when all entries are
         *        completed<BR>
         */
        if ((retCode = elementList.encodeInit(encIter, null, 0)) < CodecReturnCodes.SUCCESS)
        {
            channel.releaseBuffer(msgBuf, error);
            System.out.printf("\nEncodeElementListInit failed with return code: %d\n", retCode);
            /* Closes channel, closes server, cleans up and exits the application. */
            return TransportReturnCodes.FAILURE;
        }
        /* ApplicationId */
        applicationId.data("256");
        elementEntry.dataType(DataTypes.ASCII_STRING);
        elementEntry.name(ElementNames.APPID);
        if ((retCode = elementEntry.encode(encIter, applicationId)) < CodecReturnCodes.SUCCESS)
        {
            channel.releaseBuffer(msgBuf, error);
            System.out.printf("\nEncodeElementEntry failed with return code: %d\n", retCode);
            /* Closes channel, closes server, cleans up and exits the application. */
            return TransportReturnCodes.FAILURE;
        }

        /* ApplicationName */
        applicationName.data("ETA Provider Training");
        elementEntry.dataType(DataTypes.ASCII_STRING);
        elementEntry.name(ElementNames.APPNAME);

        if ((retCode = elementEntry.encode(encIter, applicationName)) < CodecReturnCodes.SUCCESS)
        {
            channel.releaseBuffer(msgBuf, error);
            System.out.printf("\nEncodeElementEntry failed with return code: %d\n", retCode);
            /* Closes channel, closes server, cleans up and exits the application. */
            return TransportReturnCodes.FAILURE;
        }

        /* Position - just set the Login response position info to be the same as the Login request position info */
        applicationPosition.data(loginRequestInfo_Position);
        elementEntry.dataType(DataTypes.ASCII_STRING);
        elementEntry.name(ElementNames.POSITION);

        if ((retCode = elementEntry.encode(encIter, applicationPosition)) < CodecReturnCodes.SUCCESS)
        {
            channel.releaseBuffer(msgBuf, error);
            System.out.printf("\nEncodeElementEntry failed with return code: %d\n", retCode);
            /* Closes channel, closes server, cleans up and exits the application. */
            return TransportReturnCodes.FAILURE;
        }

        /* SupportBatchRequests */
        elementEntry.dataType(DataTypes.UINT);
        elementEntry.name(ElementNames.SUPPORT_BATCH);

        if ((retCode = elementEntry.encode(encIter, supportBatchRequests)) < CodecReturnCodes.SUCCESS)
        {
            channel.releaseBuffer(msgBuf, error);
            System.out.printf("\nEncodeElementEntry failed with return code: %d\n", retCode);
            /* Closes channel, closes server, cleans up and exits the application. */
            return TransportReturnCodes.FAILURE;
        }
        /* complete encode element list - Completes encoding of an ElementList */
        if ((retCode = elementList.encodeComplete(encIter, true)) < CodecReturnCodes.SUCCESS)
        {
            channel.releaseBuffer(msgBuf, error);
            System.out.printf("\nEncodeElementListComplete failed with return code: %d\n", retCode);
            /* Closes channel, closes server, cleans up and exits the application. */
            return TransportReturnCodes.FAILURE;
        }
        /* complete encode key */

        /* EncodeMsgKeyAttribComplete finishes our key opaque, so it should return and indicate
           for us to encode our container/msg payload */
        if ((retCode = refreshMsg.encodeKeyAttribComplete(encIter, true)) < CodecReturnCodes.SUCCESS)
        {
            channel.releaseBuffer(msgBuf, error);
            System.out.printf("\nencodeKeyAttribComplete failed with return code: %d\n", retCode);
            /* Closes channel, closes server, cleans up and exits the application. */
            return TransportReturnCodes.FAILURE;
        }

        /* complete encode message */
        if ((retCode = refreshMsg.encodeComplete(encIter, true)) < CodecReturnCodes.SUCCESS)
        {
            channel.releaseBuffer(msgBuf, error);
            System.out.printf("\nencodeComplete failed with return code: %d\n", retCode);
            /* Closes channel, closes server, cleans up and exits the application. */
            return TransportReturnCodes.FAILURE;
        }

        /* send login response */
        if ((retCode = sendMessage(channel, msgBuf)) < TransportReturnCodes.SUCCESS)
        {
            /* Closes channel, closes server, cleans up and exits the application. */
            return TransportReturnCodes.FAILURE;
        }
        else if (retCode > TransportReturnCodes.SUCCESS)
        {
            /* There is still data left to flush, leave our write notification enabled so we get called again.
             * If everything wasn't flushed, it usually indicates that the TCP output buffer cannot accept more yet 
             */

            /* set write flags if there's still other data queued */
            /* flush needs to be done by application */
        }

        return retCode;
    }

    /**
     * ********************************************************************
     * Sends the login request reject status message for a channel *.
     *
     * @param streamId - the stream id to close the login for *
     * ********************************************************************
     */
    public static void closeLoginStream(int streamId)
    {
        /* find original request information associated with streamId */
        if (loginRequestInfo_StreamId == streamId)
        {
            System.out.printf("Closing login stream id %d with user name: %s\n", loginRequestInfo_StreamId, loginRequestInfo_Username);
            /* Clears the original login request information */
            clearLoginRequestInfo();
        }
    }

    /**
     * ************************************************************
     * Performs two time pass to obtain buffer *.
     *
     * @param channel - the Channel of connection *
     * @param size - size of requested buffer *
     * @param error - tracks error info *
     * @return obtained buffer *
     * ************************************************************
     */
    public static TransportBuffer etaGetBuffer(Channel channel, int size, Error error)
    {
        int retCode;
        TransportBuffer msgBuf = null;

        /* First check error */
        if ((msgBuf = channel.getBuffer(size, false, error)) == null)
        {
            /* Check to see if this is just out of buffers or if it's unrecoverable */
            if (error.errorId() != TransportReturnCodes.NO_BUFFERS)
            {
                /* Connection should be closed, return failure */
                /* Closes channel, closes server, cleans up and exits the application. */
                System.out.printf("Error (%d) (errno: %d) encountered with Channel.getBuffer. Error Text: %s\n", error.errorId(), error.sysError(), error.text());
                return null;
            }

            /* (-4) Transport Failure: There are no buffers available from the buffer pool, returned from GetBuffer.
             * This can happen if the reader isn't keeping up and/or we have a lot of write threads in multithreaded apps.
             * Use Ioctl to increase pool size or use Flush to flush data and return buffers to pool.
             */

            /* Flush and obtain buffer again */
            retCode = channel.flush(error);
            if (retCode < TransportReturnCodes.SUCCESS)
            {
                System.out.printf("Channel.flush() failed with return code %d - <%s>\n", retCode, error.text());
                return null;
            }

            /* call GetBuffer again to see if it works now after Flush */
            if ((msgBuf = channel.getBuffer(size, false, error)) == null)
            {
                System.out.printf("Error (%d) (errno: %d) encountered with Channel.getBuffer. Error Text: %s\n", error.errorId(), error.sysError(), error.text());
                return null;
            }

        }

        /* return  buffer to be filled in with valid memory */
        return msgBuf;
    }

    /**
     * ********************************************************************
     * Sends the login request reject status message for a channel *.
     *
     * @param channel - the Channel of connection *
     * @param streamId - the stream id of the login request reject status *
     * @param reason - the reason for the reject *
     * @return status code *
     * ********************************************************************
     * @throws UnknownHostException the unknown host exception
     */
    public static int sendLoginRequestRejectStatusMsg(Channel channel, int streamId, LoginRejectReason reason) throws UnknownHostException
    {
        int retCode;
        TransportBuffer msgBuf = null;
        Error error = TransportFactory.createError();

        /* ETA provides clear functions for its structuresas well as static initializers.These functions are tuned to be efficient and avoid initializing unnecessary
         * structure members, and allow for optimal structure use and reuse. In general, Refinitiv recommends that
         * you use the clear functions over static initializers, because the clear functions are more efficient.
         */
        /* Iterator used for encoding throughout the application - we can clear it and reuse it instead of recreating it */
        EncodeIterator encIter = CodecFactory.createEncodeIterator(); /* the encode iterator is created (typically stack allocated)  */

        /* Create channel info as a holder */
        ChannelInfo channelInfo = TransportFactory.createChannelInfo();

        /* Populate information from channel */
        if ((retCode = channel.info(channelInfo, error)) != TransportReturnCodes.SUCCESS)
        {
            return TransportReturnCodes.FAILURE;
        }
        /* Obtains a non-packable buffer of the requested size from the ETA Transport guaranteed buffer pool to write into for the Login request Reject Status Msg.
         * When the Buffer is returned, the length member indicates the number of bytes available in the buffer (this should match the amount
         * the application requested). When populating, it is required that the application set length to the number of bytes actually used.
         * This ensures that only the required bytes are written to the network.
         */
        /* etaGetBuffer() is the utility function that does 2-pass (more robust) getting non-packable buffer. */
        if ((msgBuf = etaGetBuffer(channel, channelInfo.maxFragmentSize(), error)) == null)
        {
            /* Connection should be closed, return failure */
            /* Closes channel, closes server, cleans up and exits the application. */
            return TransportReturnCodes.FAILURE;
        }

        encIter.clear();
        /* set version information of the connection on the encode iterator so proper versioning can be performed */
        /* set the buffer on an EncodeIterator */
        retCode = encIter.setBufferAndRWFVersion(msgBuf, channel.majorVersion(), channel.minorVersion());
        if ((retCode) < TransportReturnCodes.SUCCESS)
        {
            channel.releaseBuffer(msgBuf, error);
            System.out.printf("\nSetEncodeIteratorBuffer() failed with return code: %d\n", retCode);
            /* Closes channel, closes server, cleans up and exits the application. */
            return TransportReturnCodes.FAILURE;
        }

        /* Create and initialize status message */
        StatusMsg statusMsg = (StatusMsg)CodecFactory.createMsg();

        /* if a buffer is returned, we can populate and write, encode an Msg into the buffer */

        /* Encodes the login request reject status. */

        /* On larger structures, like messages, the clear functions tend to outperform the static initializer. It is recommended to use
         * the clear function when initializing any messages.
         */
        statusMsg.clear();
        /* set-up message */
        statusMsg.msgClass(MsgClasses.STATUS);/* (3) Status Message */
        statusMsg.streamId(streamId);
        statusMsg.domainType(DomainTypes.LOGIN);/* (1) Login Message */
        /* No payload associated with this close status message */
        statusMsg.containerType(DataTypes.NO_DATA);/* (128) No Data <BR>*/
        /* (0x020) Indicates that this StatusMsg has stream or group state information, contained in StatusMsg::state.  */
        statusMsg.flags(StatusMsgFlags.HAS_STATE);
        /* (3) Closed, the applications may attempt to re-open the stream later (can occur via either an RefreshMsg or an StatusMsg) */
        statusMsg.state().streamState(StreamStates.CLOSED_RECOVER);
        /* (2) Data is Suspect (similar to a stale data state, indicates that the health of some or all data associated with the stream is out of date or cannot be confirmed that it is current ) */
        statusMsg.state().dataState(DataStates.SUSPECT);
        statusMsg.state().code(StateCodes.NONE);

        /* Switch cases depending on login reject reason */
        switch (reason)
        {
            case MAX_LOGIN_REQUESTS_REACHED:
            {
                /* (13) Too many items open (indicates that a request cannot be processed because there are too many other streams already open) */
                statusMsg.state().code(StateCodes.TOO_MANY_ITEMS);

                String stateText = "Login request rejected for stream id " + streamId + " - max request count reached";

                statusMsg.state().text().data(stateText);
            }
                break;
            case NO_USER_NAME_IN_REQUEST:
            {
                /* (5) Usage Error (indicates an invalid usage within the system) */
                statusMsg.state().code(StateCodes.USAGE_ERROR);

                String stateText = "Login request rejected for stream id " + streamId + " - request does not contain user name";
                statusMsg.state().text().data(stateText);
            }
                break;
            default:
                break;
        }
        /* encode message */

        /* Since there is no payload, no need for Init/Complete as everything is in the msg header */
        /* Functions without a suffix of Init or Complete (e.g. EncodeMsg) perform encoding within a single call,
         * typically used for encoding simple types like Integer or incorporating previously encoded data
         * (referred to as pre-encoded data).
         */
        if ((retCode = statusMsg.encode(encIter)) < CodecReturnCodes.SUCCESS)
        {
            channel.releaseBuffer(msgBuf, error);
            System.out.printf("\nEncodeMsg() failed with return code: %d\n", retCode);
            /* Closes channel, closes server, cleans up and exits the application. */
            return TransportReturnCodes.FAILURE;
        }

        /* send login request reject status */
        if ((retCode = sendMessage(channel, msgBuf)) < TransportReturnCodes.SUCCESS)
        {
            /* send login request reject status fails */
            /* Closes channel, closes server, cleans up and exits the application. */
            return TransportReturnCodes.FAILURE;
        }
        else if (retCode > TransportReturnCodes.SUCCESS)
        {
            /* There is still data left to flush */

            /* If the login request reject status doesn't flush, just close channel and exit the app. When you send login request reject status msg, 
             * we want to make a best effort to get this across the network as it will gracefully close the open login 
             * stream. If this cannot be flushed, this application will just close the connection for simplicity.
             */

            /* Closes channel, closes server, cleans up and exits the application. */
        }

        return retCode;
    }

    /**
     * **************************************
     * Clears the login request info values *
     * **************************************.
     */
    public static void clearLoginRequestInfo()
    {
        loginRequestInfo_IsInUse = false;
        loginRequestInfo_StreamId = 0;
        loginRequestInfo_Username = "Unknown";
        loginRequestInfo_ApplicationId = "Unknown";
        loginRequestInfo_ApplicationName = "Unknown";
        loginRequestInfo_Position = "Unknown";
        loginRequestInfo_Password = "Unknown";
        loginRequestInfo_InstanceId = "Unknown";
        loginRequestInfo_Role = "Unknown";
    }
}
