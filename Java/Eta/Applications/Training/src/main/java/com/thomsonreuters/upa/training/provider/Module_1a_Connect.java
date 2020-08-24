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
 * (runs with a default set of parameters (-p 14002))
 *
 * or
 *
 * ./gradlew runprovidermod1a -PcommandLineArgs="[-p <SrvrPortNo>]"
 * (runs with specified set of parameters, all parameters are optional)
 *
 * Pressing the CTRL+C buttons terminates the program.
 *****************************************************************************************/

package com.thomsonreuters.upa.training.provider;

import java.io.IOException;
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

/**
 * The Class Module_1a_Connect.
 */
public class Module_1a_Connect
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
                else
                {
                    System.out.printf("Error: Unrecognized option: %s\n\n", args[i]);
                    System.out.printf("Usage: %s or\n%s [-p <SrvrPortNo>]\n", args[0], args[0]);
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
            /* End application */
            System.exit(TransportReturnCodes.FAILURE);
        }

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
        /*********************************************************
         * Server/Provider Application Life Cycle Major Step 2: Create listening
         * socket using Bind Establishes a listening socket connection, which
         * supports connections from standard socket and HTTP Connect users.
         * Returns an Server that represents the listening socket connection to
         * the user. In the event of an error, NULL is returned and additional
         * information can be found in the Error structure. Options are passed
         * in via an BindOptions structure. Once a listening socket is
         * established, this Server can begin accepting connections.
         *********************************************************/

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
        /* Main Loop #1 for detecting incoming client connections.  */
        while (!clientAccepted)
        {
            /* Wait for any I/O notification updates in the channel for our specified amount of seconds (e.g. 60 sec.)*/
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

                        /**************************************************************************************************
                         * Step 3) Accept the connection request
                         **************************************************************************************************/
                        /*********************************************************
                         * Server/Provider Application Life Cycle Major Step 3:
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
                        if ((channel = upaSrvr.accept(acceptOpts, error)) == null)
                        {
                            System.out.printf("Error (%d) (errno: %d) encountered with Init Channel fd=%d. Error Text: %s\n", error.errorId(), error.sysError(), clientChannelFDValue, error.text());

                            /* End application, uninitialize to clean up first */
                            Transport.uninitialize();
                            System.exit(TransportReturnCodes.FAILURE);

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
                e.printStackTrace();
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
                         * or because an Server rejected the connection by setting nakMount to TRUE, the Channel state
                         * will become CLOSED.
                         *
                         * Note:
                         * For both client and server channels, more than one call to InitChannel can be required to complete
                         * the channel initialization process.
                         */

                        if ((retCode = channel.init(inProgInfo, error)) < TransportReturnCodes.SUCCESS)
                        {
                            System.out.printf("Error (%d) (errno: %d) encountered with InitChannel %d. Error Text: %s\n", error.errorId(), error.sysError(), clientChannelFDValue, error.text());
                            closeChannelServerCleanUpAndExit(channel, upaSrvr, selector, TransportReturnCodes.FAILURE);
                            break;
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
                                    clientChannelFDValue = TrainingModuleUtils.getFDValueOfSelectableChannel(channel.selectableChannel());
                                    int oldClientChannelFDValue = clientChannelFDValue;
                                    System.out.printf("\nChannel In Progress - New FD: %d    OLD %d\n", clientChannelFDValue, oldClientChannelFDValue);
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
                                    System.out.printf("\nChannel %d In Progress...\n", clientChannelFDValue);
                                }
                            }
                                break;
                            /* channel connection becomes active!
                             * Once a connection is established and transitions to the _CH_STATE_ACTIVE state,
                             * this Channel can be used for other transport operations.
                             */
                            case TransportReturnCodes.SUCCESS:
                            {
                                System.out.printf("\nChannel on fd %d is now active - reading and writing can begin.\n", clientChannelFDValue);
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

                                /* do not allow new client to connect  */

                                /* For this simple training app, the interactive provider only supports a single client. Once a client
                                 * successfully connects, we call CloseServer function to close the listening socket associated with the
                                 * Server. The connected Channels will remain open. This allows the established connection to continue
                                 * to send and receive data, while preventing new clients from connecting.
                                 */

                                /* clean up server */
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
                                /* Closes channel, closes server, cleans up and exits the application. */
                                closeChannelServerCleanUpAndExit(channel, upaSrvr, selector, TransportReturnCodes.FAILURE);
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
        if ((channel != null) && (channel.close(error) < TransportReturnCodes.SUCCESS))
        {
            System.out.printf("Error (%d) (errno: %d): %s\n", error.errorId(), error.sysError(), error.text());
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
        if ((server != null) && server.close(error) < TransportReturnCodes.SUCCESS)
        {
            System.out.printf("Error (%d) (errno: %d): %s\n", error.errorId(), error.sysError(), error.text());
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

        /* For applications that do not exit due to errors/exceptions such as:
         * Exits the application if the run-time has expired.
         */
        if (code == TransportReturnCodes.SUCCESS)
        {
            System.out.printf("\nUPA Interactive Provider Training Application successfully ended.\n");
        }

        /* End application */
        System.exit(code);
    }
}