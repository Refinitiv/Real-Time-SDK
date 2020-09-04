/*
 *|-------------------------------------------------------------------------------
 *| This source code is provided under the Apache 2.0 license and is provided	--
 *| AS IS with no warranty or guarantee of fit for purpose.  See the project's 	--
 *| LICENSE.md for details.														--
 *| Copyright (C) 2019 Refinitiv. All rights reserved.						--
 *|-------------------------------------------------------------------------------
 */

/*****************************************************************************************
 * This is the UPA Interactive Provider Training series of the UPA Training Suite
 * applications. The purpose of this application is to show step-by-step 
 * training how to build a UPA OMM Interactive Provider using the UPA Transport layer.
 *
 * Main Java source file for the UPA Interactive Provider Training application. It is a 
 * single-threaded client application.
 *
 *****************************************************************************************
 * UPA Interactive Provider Training Module 1a: Establish network communication
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
 * The first step of any UPA Interactive Provider application is to establish
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
 * UPA Interactive Provider Training Module 1b: Ping (heartbeat) Management
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
 * UPA Interactive Provider Training Module 1c: Reading and Writing Data
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
 *****************************************************************************************/

package com.thomsonreuters.upa.training.provider;

import java.io.IOException;
import java.nio.channels.ClosedChannelException;
import java.nio.channels.SelectionKey;
import java.nio.channels.Selector;
import java.util.Iterator;
import java.util.Set;

import com.thomsonreuters.upa.codec.Codec;
import com.thomsonreuters.upa.training.common.TrainingModuleUtils;
import com.thomsonreuters.upa.transport.AcceptOptions;
import com.thomsonreuters.upa.transport.BindOptions;
import com.thomsonreuters.upa.transport.Channel;
import com.thomsonreuters.upa.transport.ChannelInfo;
import com.thomsonreuters.upa.transport.ChannelState;
import com.thomsonreuters.upa.transport.CompressionTypes;
import com.thomsonreuters.upa.transport.Error;
import com.thomsonreuters.upa.transport.InProgFlags;
import com.thomsonreuters.upa.transport.InProgInfo;
import com.thomsonreuters.upa.transport.InitArgs;
import com.thomsonreuters.upa.transport.Server;
import com.thomsonreuters.upa.transport.Transport;
import com.thomsonreuters.upa.transport.TransportFactory;
import com.thomsonreuters.upa.transport.TransportReturnCodes;
import com.thomsonreuters.upa.transport.ReadArgs;
import com.thomsonreuters.upa.codec.CodecFactory;
import com.thomsonreuters.upa.codec.CodecReturnCodes;
import com.thomsonreuters.upa.codec.DecodeIterator;
import com.thomsonreuters.upa.codec.Msg;
import com.thomsonreuters.upa.rdm.DomainTypes;
import com.thomsonreuters.upa.transport.TransportBuffer;

/**
 * The Class Module_1c_ReadWrite.
 */
public class Module_1c_ReadWrite
{

    /**
     * The main method.
     *
     * @param args the arguments
     */
    public static void main(String[] args)
    {
        /**************************************************************************************************
         * DECLARING VARIABLES
         **************************************************************************************************/
        /* Create a server to eventually accept connection requests */
        Server upaSrvr = null;

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
        long upaRuntime = 0;
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
         * Server/Provider Application Life Cycle Major Step 1: Initialize UPA
         * Transport using Initialize The first UPA Transport function that an
         * application should call. This creates and initializes internal memory
         * and structures, as well as performing any bootstrapping for
         * underlying dependencies. The Initialize function also allows the user
         * to specify the locking model they want applied to the UPA Transport.
         *********************************************************/

        if (Transport.initialize(initArgs, error) != TransportReturnCodes.SUCCESS)
        {
            System.out.printf("Error (%d) (errno: %d) %s\n", error.errorId(), error.sysError(), error.text());
            System.exit(TransportReturnCodes.FAILURE);
        }

        currentTime = System.currentTimeMillis();
        upaRuntime = currentTime + runTime * 1000;

        /* populate bind options, then pass to Bind function -
         * UPA Transport should already be initialized
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

        /* populate version and protocol with RWF information (found in Iterators.h) or protocol specific info */
        bindOpts.protocolType(Codec.protocolType()); /* Protocol type definition for RWF */
        bindOpts.majorVersion(Codec.majorVersion());
        bindOpts.minorVersion(Codec.minorVersion());
        /**************************************************************************************************
         * Bind and receive a server
         **************************************************************************************************/
        /* Bind UPA server */
        if ((upaSrvr = Transport.bind(bindOpts, error)) == null)
        {
            System.out.printf("Error (%d) (errno: %d) encountered with Bind. Error Text: %s\n", error.errorId(), error.sysError(), error.text());
            /* End application, uninitialize to clean up first */
            Transport.uninitialize();
            System.exit(TransportReturnCodes.FAILURE);
        }

        int clientChannelFDValue = -1;
        final int upaServerFDValue = TrainingModuleUtils.getFDValueOfSelectableChannel(upaSrvr.selectableChannel());
        System.out.printf("Server IPC descriptor = %d bound on port %d\n", upaServerFDValue, upaSrvr.portNumber());

        opMask |= SelectionKey.OP_ACCEPT;

        /* Register our channel to the selector and watch for I/O notifications specified by our operation mask */
        try
        {
            selector = Selector.open();
            upaSrvr.selectableChannel().register(selector, opMask, upaSrvr);
        }
        catch (Exception e)
        {
            System.out.printf("Exception %s\n", e.getMessage());
            closeChannelServerCleanUpAndExit(channel, upaSrvr, selector, TransportReturnCodes.FAILURE);
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
                    key = upaSrvr.selectableChannel().keyFor(selector);
                    if (key.isAcceptable())
                    {
                        acceptOpts.nakMount(false);

                        /*****************************************
                         * Step 3) Accept the connection request *
                         *****************************************/
                        /*********************************************************
                         * Server/Provider Application Life Cycle Major Step 3:
                         * Accept connection using Accept This step is performed
                         * per connected client/connection/channel Uses the
                         * Server that represents the listening socket
                         * connection and begins the accepting process for an
                         * incoming connection request. Returns an Channel that
                         * represents the client connection. In the event of an
                         * error, null is returned and additional information
                         * can be found in the Error structure. The Accept
                         * function can also begin the rejection process for a
                         * connection through the use of the AcceptOptions
                         * structure. Once a connection is established and
                         * transitions to the ACTIVE state, this Channel can be
                         * used for other transport operations.
                         *********************************************************/
                        /* An OMM Provider application can begin the connection accepting or rejecting process by using the Accept function */
                        if ((channel = upaSrvr.accept(acceptOpts, error)) == null)
                        {
                            System.out.printf("Error (%d) (errno: %d) encountered with Init Channel fd=%d. Error Text: %s\n", error.errorId(), error.sysError(), clientChannelFDValue, error.text());
                            Transport.uninitialize();
                        }

                        else
                        {
                            /* For this simple training app, the interactive provider only supports one client session from the consumer. */
                             clientChannelFDValue = TrainingModuleUtils.getFDValueOfSelectableChannel(channel.selectableChannel());
                            System.out.printf("\nServer fd = %d: New client on Channel fd=%d\n", upaServerFDValue, clientChannelFDValue);
                            /*set clientAccepted to be TRUE and exit the while Main Loop #1*/
                            clientAccepted = true;
                        }
                    }
                }
            }
            catch (IOException e)
            {
                System.out.printf("Exception %s\n", e.getMessage());
                closeChannelServerCleanUpAndExit(channel, upaSrvr, selector, TransportReturnCodes.FAILURE);
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
                closeChannelServerCleanUpAndExit(channel, upaSrvr, selector, TransportReturnCodes.FAILURE);
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
                    closeChannelServerCleanUpAndExit(channel, upaSrvr, selector, TransportReturnCodes.FAILURE);
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
                        /* Internally, the UPA initialization process includes several actions. The initialization includes
                         * any necessary UPA connection handshake exchanges, including any HTTP or HTTPS negotiation.
                         * Compression, ping timeout, and versioning related negotiations also take place during the
                         * initialization process. This process involves exchanging several messages across the connection,
                         * and once all message exchanges have completed the Channel.state will transition. If the connection
                         * is accepted and all types of negotiations completed properly, the Channel state will become
                         * ACTIVE. If the connection is rejected, either due to some kind of negotiation failure
                         * or because an Server rejected the connection by setting nakMount to true, the Channel state
                         * will become CLOSED.
                         *
                         * Note:
                         * For both client and server channels, more than one call to InitChannel can be required to complete
                         * the channel initialization process.
                         */
                        if ((retCode = channel.init(inProgInfo, error)) < TransportReturnCodes.SUCCESS)
                        {
                            System.out.printf("Error (%d) (errno: %d) encountered with Init Channel fd=%d. Error Text: %s\n", error.errorId(), error.sysError(), clientChannelFDValue, error.text());
                            closeChannelServerCleanUpAndExit(channel, upaSrvr, selector, TransportReturnCodes.FAILURE);
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
                                        closeChannelServerCleanUpAndExit(channel, upaSrvr, selector, TransportReturnCodes.FAILURE);
                                    }
                                    try
                                    {
                                        channel.selectableChannel().register(selector, opMask, channel);
                                    }
                                    catch (Exception e)
                                    {
                                        System.out.printf("Exception: %s\n", e.getMessage());
                                        closeChannelServerCleanUpAndExit(channel, upaSrvr, selector, TransportReturnCodes.FAILURE);
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

                                /* After channel is active, use UPA Transport utility function GetChannelInfo to query Channel negotiated
                                 * parameters and settings and retrieve all current settings. This includes maxFragmentSize and negotiated
                                 * compression information as well as many other values.
                                 */

                                /* Populate information from channel */
                                if ((retCode = channel.info(channelInfo, error)) != TransportReturnCodes.SUCCESS)
                                {
                                    System.out.printf("Error (%d) (errno: %d) encountered with channel.info. Error Text: %s\n", error.errorId(), error.sysError(), error.text());
                                    /* Connection should be closed, return failure */
                                    /* Closes channel, closes server, cleans up and exits the application. */
                                    closeChannelServerCleanUpAndExit(channel, upaSrvr, selector, TransportReturnCodes.FAILURE);
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
                                 * Server/Provider Application Life Cycle Major
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
                                 * When shutting down the UPA Transport, the application should release any unwritten pool buffers.
                                 * The listening socket can be closed by calling CloseServer. This prevents any new connection attempts.
                                 * If shutting down connections for all connected clients, the provider should call CloseChannel for each connection client.
                                */
                                if ((upaSrvr == null) & (upaSrvr.close(error) < TransportReturnCodes.SUCCESS))
                                {
                                    System.out.printf("Error (%d) (errno: %d) encountered with CloseServer.  Error Text : %s\n", error.errorId(), error.sysError(), error.text());

                                    /* End application, uninitialize to clean up first */
                                    Transport.uninitialize();
                                    System.exit(TransportReturnCodes.FAILURE);
                                }

                                /*set upaSrvr to be null*/
                                upaSrvr = null;
                            }
                                break;
                            default: /* Error handling */
                            {
                                System.out.printf("Bad return value fd=%d: <%s>\n", clientChannelFDValue, TransportReturnCodes.toString(retCode));
                                closeChannelServerCleanUpAndExit(channel, upaSrvr, selector, TransportReturnCodes.FAILURE);
                            }
                                break;
                        }
                    }
                }
            }
            catch (IOException e1)
            {
                System.out.printf("Exception %s\n", e1.getMessage());
                closeChannelServerCleanUpAndExit(channel, upaSrvr, selector, TransportReturnCodes.FAILURE);
            }
        }

        /* Keep track of READ notifications */
        opMask |= SelectionKey.OP_READ;

        /* Initialize ping management handler */
        initPingManagement(channel);

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
                closeChannelServerCleanUpAndExit(channel, upaSrvr, selector, TransportReturnCodes.FAILURE);
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

                                /* No need to clear the message before we decode into it. UPA Decoding populates all message members (and that is true for any
                                 * decoding with UPA, you never need to clear anything but the iterator)
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
                                    closeChannelServerCleanUpAndExit(channel, upaSrvr, selector, TransportReturnCodes.FAILURE);
                                }
                                /******************************************
                                 * Step 6) Decode buffer message *
                                 ******************************************/
                                /* decode contents into the Msg structure */
                                if ((retCode = msg.decode(decodeIter)) != CodecReturnCodes.SUCCESS)
                                {
                                    System.out.printf("Error (%d) (errno: %d) encountered with Init Channel fd=%d. Error Text: %s\n", error.errorId(), error.sysError(), clientChannelFDValue, error.text());
                                    /* Closes channel, closes server, cleans up and exits the application. */
                                    closeChannelServerCleanUpAndExit(channel, upaSrvr, selector, TransportReturnCodes.FAILURE);
                                }

                                /* Deduce an action based on the domain type of the message */
                                switch (msg.domainType())
                                {
                                    /* (1) Login Message */
                                    case DomainTypes.LOGIN:
                                    {

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
                                            closeChannelServerCleanUpAndExit(channel, upaSrvr, selector, TransportReturnCodes.FAILURE);
                                        }
                                        try
                                        {
                                            channel.selectableChannel().register(selector, opMask, channel);
                                        }
                                        catch (Exception e)
                                        {
                                            System.out.printf("Exception %s\n", e.getMessage());
                                            closeChannelServerCleanUpAndExit(channel, upaSrvr, selector, TransportReturnCodes.FAILURE);
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
                                    default: /* Error handling */
                                        if (retCode < 0)
                                        {
                                            System.out.printf("Error (%d) (errno: %d) encountered with Read Error Text: %s\n", error.errorId(), error.sysError(), error.text());
                                            closeChannelServerCleanUpAndExit(channel, upaSrvr, selector, TransportReturnCodes.FAILURE);
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
                                    closeChannelServerCleanUpAndExit(channel, upaSrvr, selector, TransportReturnCodes.FAILURE);
                                }
                            }
                        }
                    }
                }
                /* Processing ping management handler */
                if ((retCode = processPingManagementHandler(channel, selector)) > TransportReturnCodes.SUCCESS)
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
                    closeChannelServerCleanUpAndExit(channel, upaSrvr, selector, TransportReturnCodes.FAILURE);
                }

                /* get current time */
                currentTime = System.currentTimeMillis();

                /* If the runtime has expired */
                if (System.currentTimeMillis() >= upaRuntime)
                {
                    /* Closes all streams for the Interactive Provider after run-time has elapsed in our simple Interactive Provider example.
                     * If the provider application must shut down, it can either leave consumer connections intact or shut them down. If the provider
                     * decides to close consumer connections, the provider should send an StatusMsg on each connection's Login stream closing the stream.
                     * At this point, the consumer should assume that its other open streams are also closed.
                     */
                    /* flush before exiting */
                    key = channel.selectableChannel().keyFor(selector);
                    if (key.isWritable())
                    {
                        retCode = 1;

                        /******************************************************
                         * Loop 5) Flush all remaining buffers to channel *
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
                    System.out.printf("UPA Server run-time has expired...\n");
                    closeChannelServerCleanUpAndExit(channel, upaSrvr, selector, TransportReturnCodes.SUCCESS);
                }
            }
            catch (IOException e1)
            {
                System.out.printf("Exception %s\n", e1.getMessage());
                closeChannelServerCleanUpAndExit(channel, upaSrvr, selector, TransportReturnCodes.FAILURE);
            }
        }
    }

    /**
     * *******************************************************
     * Closes channel and selector and exits application. * channel - Channel to
     * be closed * selector - Selector to be closed * error - tracks error info
     * code - if exit is due to errors/exceptions *
     * *******************************************************
     *
     * @param channel the channel
     * @param server the server
     * @param selector the selector
     * @param code the code
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
         * Server/Provider Application Life Cycle Major Step 6: Close connection
         * using CloseChannel (OS connection release handshake) CloseChannel
         * closes the server based Channel. This will release any pool based
         * resources back to their respective pools, close the connection, and
         * perform any additional necessary cleanup. When shutting down the UPA
         * Transport, the application should release all unwritten pool buffers.
         * Calling CloseChannel terminates the connection for each connection
         * client.
         *********************************************************/
        if ((channel != null)) {
            isClosedAndClean = channel.close(error) >= TransportReturnCodes.SUCCESS;
        }

        /*********************************************************
         * Server/Provider Application Life Cycle Major Step 7: Closes a
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
         * When shutting down the UPA Transport, the application should release any unwritten pool buffers.
         * The listening socket can be closed by calling CloseServer. This prevents any new connection attempts.
         * If shutting down connections for all connected clients, the provider should call CloseChannel for each connection client.
        */
        if ((server != null)) {
            isClosedAndClean &= server.close(error) >= TransportReturnCodes.SUCCESS;
        }

        /*********************************************************
         * Server/Provider Application Life Cycle Major Step 8: Uninitialize UPA
         * Transport using Uninitialize The last UPA Transport function that an
         * application should call. This uninitialized internal data structures
         * and deletes any allocated memory.
         *********************************************************/
        /* All UPA Transport use is complete, must uninitialize.
         * The uninitialization process allows for any heap allocated memory to be cleaned up properly.
         */
        Transport.uninitialize();

        if (isClosedAndClean) {
            System.out.println("Provider application has closed channel and has cleaned up successfully.");
        } else {
            System.out.printf("Error (%d) (errno: %d): %s\n", error.errorId(), error.sysError(), error.text());
        }

        /* For applications that do not exit due to errors/exceptions such as:
         * Exits the application if the run-time has expired.
         */
        if (code == TransportReturnCodes.SUCCESS)
        {
            System.out.printf("\nUPA Interactive Provider Training Application successfully ended.\n");
        }

        /* End application */
        System.exit(0);
    }

    /*********************************************************
     * Initializes the ping times for upaChannel. upaChannel - The channel for
     * ping management info initialization
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
     * Processing ping management handler upaChannel - The channel for ping
     * management processing
     * *******************************************************.
     *
     * @param channel the channel
     * @param selector the selector
     * @return the int
     */
    public static int processPingManagementHandler(Channel channel, Selector selector)
    {
        /* Handles the ping processing for upaChannel. Sends a ping to the client if the next send ping time has arrived and
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
             * Server/Provider Application Life Cycle Major Step 5: Ping using
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
}
