
/*
 *|-------------------------------------------------------------------------------
 *| This source code is provided under the Apache 2.0 license and is provided	--
 *| AS IS with no warranty or guarantee of fit for purpose.  See the project's 	--
 *| LICENSE.md for details.														--
 *| Copyright (C) 2019 Refinitiv. All rights reserved.						--
 *|-------------------------------------------------------------------------------
 */

/**
 * This is the UPA NI Provider Training series of the UPA Training Suite
 * applications. The purpose of this application is to show step-by-step 
 * training how to build a UPA OMM NI Provider using the UPA Transport layer.
 *
 * Main c source file for the UPA NI Provider Training application. It is a 
 * single-threaded client application.
 *
 ************************************************************************
 * UPA NI Provider Training Module 1a: Establish network communication
 ************************************************************************
 * Summary:
 * A Non-Interactive Provider (NIP) writes a provider application that 
 * connects to TREP-RT and sends a specific set (non-interactive) of 
 * information (services, domains, and capabilities). NIPs act like 
 * clients in a client-server relationship. Multiple NIPs can connect 
 * to the same TREP-RT and publish the same items and content. 
 * 
 * In this module, the OMM NIP application initializes the UPA Transport 
 * and establish a connection to an ADH server. Once connected, an OMM NIP 
 * can publish information into the ADH cache without needing to handle 
 * requests for the information. The ADH can cache the information and 
 * along with other Refinitiv Real-Time Distribution System components,
 * provide the information to any NIProvider applications that indicate interest.
 *
 * Detailed Descriptions:
 * The first step of any UPA NIP application is to establish network 
 * communication with an ADH server. To do so, the OMM NIP typically creates 
 * an outbound connection to the well-known hostname and port of an ADH. 
 * The OMM NIP uses the Connect function to initiate the connection 
 * process and then performs connection initialization processes as needed.
 *
 * Command line usage:
 *
 * ./gradlew runniprovidermod1a
 * (runs with a default set of parameters (-h localhost -p 14003 -i ""))
 *
 * or
 *
 * ./gradlew runniprovidermod1a -PcommandLineArgs="[-h <SrvrHostname>] [-p <SrvrPortNo>] [-i <InterfaceName>]
 * (runs with specified set of parameters, all parameters are optional)
 *
 * Pressing the CTRL+C buttons terminates the program.
 *
 ************************************************************************
 * UPA NI Provider Training Module 1b: Ping (heartbeat) Management
 ************************************************************************
 * Summary:
 * In this module, after establishing a connection, ping messages might 
 * need to be exchanged. The negotiated ping timeout is available via 
 * the Channel. If ping heartbeats are not sent or received within 
 * the expected time frame, the connection can be terminated. Refinitiv 
 * recommends sending ping messages at intervals one-third the 
 * size of the ping timeout.
 *
 * Detailed Descriptions:
 * Ping or heartbeat messages are used to indicate the continued presence of 
 * an application. These are typically only required when no other information 
 * is being exchanged. For example, there may be long periods of time that 
 * elapse between requests made from an OMM NIP application to ADH Infrastructure.
 * In this situation, the NIP would send periodic heartbeat messages to inform 
 * the ADH Infrastructure that it is still alive.
 *
 * Command line usage:
 *
 * ./gradlew runniprovidermod1b
 * (runs with a default set of parameters (-h localhost -p 14003 -i "" -r 300))
 *
 * or
 *
 * ./gradlew runniprovidermod1b -PcommandLineArgs="[-h <SrvrHostname>] [-p <SrvrPortNo>] [-i <InterfaceName>] [-r <Running Time>]"
 * (runs with specified set of parameters, all parameters are optional)
 *
 * Pressing the CTRL+C buttons terminates the program.
 */

package com.refinitiv.eta.training.niprovider;

import java.io.IOException;
import java.nio.channels.SelectionKey;
import java.nio.channels.Selector;
import java.util.Iterator;
import java.util.Set;

import com.refinitiv.eta.codec.Codec;
import com.refinitiv.eta.training.common.TrainingModuleUtils;
import com.refinitiv.eta.transport.Channel;
import com.refinitiv.eta.transport.ChannelInfo;
import com.refinitiv.eta.transport.ChannelState;
import com.refinitiv.eta.transport.CompressionTypes;
import com.refinitiv.eta.transport.ConnectOptions;
import com.refinitiv.eta.transport.ConnectionTypes;
import com.refinitiv.eta.transport.Error;
import com.refinitiv.eta.transport.InProgFlags;
import com.refinitiv.eta.transport.InProgInfo;
import com.refinitiv.eta.transport.InitArgs;
import com.refinitiv.eta.transport.ReadArgs;
import com.refinitiv.eta.transport.Transport;
import com.refinitiv.eta.transport.TransportBuffer;
import com.refinitiv.eta.transport.TransportFactory;
import com.refinitiv.eta.transport.TransportReturnCodes;

public class Module_1b_Ping
{

    public static void main(String[] args)
    {

        /******************************************************************************************************************
         * DECLARING VARIABLES
         ******************************************************************************************************************/
        /* For this simple training app, only a single channel/connection is used for the entire life of this app. */
        Channel channel = null;

        /* Create an I/O notification mechanism (a Selector in our case) for our channel */
        Selector selector = null;
        /* Create a bit mask to specify what I/O notification operations to keep track of (e.g. READ and CONNECT)*/
        short opMask = 0;

        int retCode;
        /* Create error to keep track of any errors that may occur in Transport methods */
        Error error = TransportFactory.createError();

        /* Create and populate connect options to specify connection preferences */
        ConnectOptions cOpts = TransportFactory.createConnectOptions();

        /* InProgInfo Information for the In Progress Connection State */
        InProgInfo inProgInfo = TransportFactory.createInProgInfo();

        /*  ChannelInfo*/
        ChannelInfo channelInfo = TransportFactory.createChannelInfo();

        TransportBuffer msgBuf = null;

        long currentTime = 0;
        long upaRuntime = 0;
        long runTime = 0;

        /* Create initialization arguments (InitArgs) to specify any initialization options for Transport */
        InitArgs initArgs = TransportFactory.createInitArgs();

        /* the default option parameters */
        /* connect to server running on same machine */
        String srvrHostname = "localhost";
        /* server is running on port number 14003 */
        String srvrPortNo = "14003";
        /* use default NIC network interface card to bind to for all inbound and outbound data */
        String interfaceName = "";
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
                else
                {
                    System.out.printf("Error: Unrecognized option: %s\n\n", args[i]);
                    System.out.printf("Usage: %s or\n%s [ -h <SrvrHostname>] [-p <SrvrPortNo>] [-i <InterfaceName>] [-r <runTime>] \n", args[0], args[0]);
                    System.exit(TransportReturnCodes.FAILURE);
                }
            }
        }

        /******************************************************************************************************************
         * INITIALIZATION - USING Initialize()
         ******************************************************************************************************************/
        /*********************************************************
         * Client/NIProv Application Life Cycle Major Step 1: Initialize UPA
         * Transport using Initialize The first UPA Transport function that an
         * application should call. This creates and initializes internal memory
         * and structures, as well as performing any boot strapping for
         * underlying dependencies. The Initialize function also allows the user
         * to specify the locking model they want applied to the UPA Transport.
         *********************************************************/

        if (Transport.initialize(initArgs, error) != TransportReturnCodes.SUCCESS)
        {
            System.out.printf("Error (%d) (errno: %d): %s\n", error.errorId(), error.sysError(), error.text());
            System.exit(TransportReturnCodes.FAILURE);
        }

        currentTime = System.currentTimeMillis();
        upaRuntime = currentTime + runTime * 1000;
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

        /* populate version and protocol with RWF information or protocol specific info */
        cOpts.protocolType(Codec.protocolType()); /* Protocol type definition for RWF */

        cOpts.majorVersion(Codec.majorVersion());
        cOpts.minorVersion(Codec.minorVersion());

        /******************************************************************************************************************
         * CONNECTION SETUP - USING Connect()
         ******************************************************************************************************************/
        /*********************************************************
         * Client/NIProv Application Lief cycle Major Step 2: Connect using
         * Connect (OS connection establishment handshake) Connect call
         * Establishes an outbound connection, which can leverage standard
         * sockets, HTTP, or HTTPS. Returns an Channel that represents the
         * connection to the user. In the event of an error, NULL is returned
         * and additional information can be found in the Error structure.
         * Connection options are passed in via an ConnectOptions structure.
         *********************************************************/

        if ((channel = Transport.connect(cOpts, error)) == null)
        {
            System.out.printf("Error (%d) (errno: %d) encountered with Connect. Error Text : %s\n", error.errorId(), error.sysError(), error.text());

            /* End application, uninitialize to clean up first */
            Transport.uninitialize();
            System.exit(TransportReturnCodes.FAILURE);
        }

        int channelFDValue = TrainingModuleUtils.getFDValueOfSelectableChannel(channel.selectableChannel());
        System.out.printf("Channel IPC descriptor = %d\n", channelFDValue);

        /* Connection was successful, add socketId to I/O notification mechanism and initialize connection */
        /* Typical Maskset use, this may vary depending on the I/O notification mechanism the application is using */
        opMask |= SelectionKey.OP_READ;
        opMask |= SelectionKey.OP_CONNECT;
        opMask |= SelectionKey.OP_WRITE;
        if (!cOpts.blocking())
        {
            if (opMask == 0)
                opMask |= SelectionKey.OP_WRITE;
        }

        try
        {
            selector = Selector.open();
        }
        catch (Exception e)
        {
            System.out.printf("Exception %s\n", e.getMessage());
            closeChannelCleanUpAndExit(channel, selector, error, TransportReturnCodes.FAILURE);
        }

        SelectionKey key = null;
        Set<SelectionKey> selectedKeys = null;
        Iterator<SelectionKey> keyIter = null;

        /******************************************************************************************************************
         * MAIN LOOP TO SEE IF RESPONSE RECEIVED FROM PROVIDER
         ******************************************************************************************************************/
        /* Main loop for getting connection active and successful completion of the initialization process
         * The loop calls select() to wait for notification
         * Currently, the main loop would exit if an error condition is triggered or
         * Channel.state transitions to ACTIVE.
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
                closeChannelCleanUpAndExit(channel, selector, error, TransportReturnCodes.FAILURE);
            }

            /* Max waiting time */
            final int timeOutSeconds = 60;

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
                    closeChannelCleanUpAndExit(channel, selector, error, TransportReturnCodes.FAILURE);
                }
                else
                {
                    /* Received a response from the provider. */

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
                        if ((retCode = channel.init(inProgInfo, error)) < TransportReturnCodes.SUCCESS)
                        {
                            System.out.printf("Error (%d) (errno: %d) encountered with Init Channel fd=%d. Error Text: %s\n", error.errorId(), error.sysError(), channelFDValue, error.text());
                            closeChannelCleanUpAndExit(channel, selector, error, TransportReturnCodes.FAILURE);
                        }

                        /* Handle return code appropriately */
                        switch (retCode)
                        {
                            /* (2)  Transport Success: Channel initialization is In progress, returned from InitChannel. */
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
                                    opMask = SelectionKey.OP_READ | SelectionKey.OP_CONNECT;
                                    final int oldChannelFDValue = channelFDValue;
                                    channelFDValue = TrainingModuleUtils.getFDValueOfSelectableChannel(channel.selectableChannel());
                                    System.out.printf("\nChannel In Progress - New FD: %d   Old FD: %d\n", channelFDValue, oldChannelFDValue);

                                    /* File descriptor has changed, unregister old and register new */

                                    try
                                    {
                                        key = inProgInfo.oldSelectableChannel().keyFor(selector);
                                        key.cancel();
                                    }
                                    catch (Exception e)
                                    {
                                        System.out.printf("Exception %s\n", e.getMessage());
                                        closeChannelCleanUpAndExit(channel, selector, error, TransportReturnCodes.FAILURE);
                                    }
                                    try
                                    {
                                        channel.selectableChannel().register(selector, opMask, channel);
                                    }
                                    catch (Exception e)
                                    {
                                        System.out.printf("Exception %s\n", e.getMessage());
                                        closeChannelCleanUpAndExit(channel, selector, error, TransportReturnCodes.FAILURE);
                                    }
                                }
                                else
                                {
                                    System.out.printf("Channel %d in progress...\n", channelFDValue);
                                }
                            }
                                break;

                            /* channel connection becomes active!
                             * Once a connection is established and transitions to the ACTIVE state,
                             * this Channel can be used for other transport operations.
                             */
                            case TransportReturnCodes.SUCCESS:
                            {
                                System.out.printf("Channel on fd %d is now active - reading and writing can begin.\n", channelFDValue);

                                /*********************************************************
                                 * Connection is now active. The Channel can be
                                 * used for all additional transport
                                 * functionality (e.g. reading, writing) now
                                 * that the state transitions to
                                 * _CH_STATE_ACTIVE
                                 *********************************************************/

                                /* After channel is active, use UPA Transport utility function GetChannelInfo to query Channel negotiated
                                 * parameters and settings and retrieve all current settings. This includes maxFragmentSize and negotiated
                                 * compression information as well as many other values.
                                 */

                                /* Populate information from channel */
                                if ((retCode = channel.info(channelInfo, error)) != TransportReturnCodes.SUCCESS)
                                {
                                    System.out.printf("Error (%d) (errno: %d) encountered with GetChannelInfo. Error Text: %s\n", error.errorId(), error.sysError(), error.text());
                                    /* Connection should be closed, return failure */
                                    closeChannelCleanUpAndExit(channel, selector, error, TransportReturnCodes.FAILURE);
                                }

                                /* Print out basic channel info */
                                System.out.printf("\nChannel %d active. Channel Info:\n" + "Max Fragment Size:           %d\n" + "Output Buffers:              %d Max, %d Guaranteed\n" + "Input Buffers:               %d\n" + "Send/Receive Buffer Sizes:   %d/%d\n" + "Ping Timeout:                %d\n",
                                        channelFDValue,
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
                            }
                                break;
                            default:
                            {
                                System.out.printf("Bad return value fd=%d <%d>\n", channelFDValue, retCode);
                                closeChannelCleanUpAndExit(channel, selector, error, TransportReturnCodes.FAILURE);
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
        /*****************************************************************************************************************
         * SECOND MAIN LOOP TO CONNECTION ACTIVE - KEEP LISTEINING FOR INCOMING
         * DATA
         ******************************************************************************************************************/
        /* Here were are using a new Main loop. An alternative design would be to combine this Main loop with
         * the Main loop for getting connection active. Some bookkeeping would be required for that approach.
         */

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
                closeChannelCleanUpAndExit(channel, selector, error, TransportReturnCodes.FAILURE);
            }

            /* Wait 1 seconds for any I/O notification updates in the channel */
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
                        /* reading data from channel via Read/Exception FD */

                        retCode = 1; /* initialize to a positive value for Read call in case we have more data that is available to read */

                        /******************************************************
                         * Loop 4) Read and decode for all buffers in channel *
                         ******************************************************/
                        while (retCode > TransportReturnCodes.SUCCESS)
                        {
                            /* Create read arguments (ReadArgs) to keep track of the statuses of any reads */
                            ReadArgs readArgs = TransportFactory.createReadArgs();

                            /* There is more data to read and process and I/O notification may not trigger for it
                             * Either schedule another call to read or loop on read until retCode == TransportReturnCodes.SUCCESS
                             * and there is no data left in internal input buffer
                             */

                            /*********************************************************
                             * Read using channel.read() Read provides the user
                             * with data received from the connection. This
                             * function expects the Channel to be in the active
                             * state. When data is available, an Buffer
                             * referring to the information is returned, which
                             * is valid until the next call to Read. A return
                             * code parameter passed into the function is used
                             * to convey error information as well as
                             * communicate whether there is additional
                             * information to read. An I/O notification
                             * mechanism may not inform the user of this
                             * additional information as it has already been
                             * read from the socket and is contained in the Read
                             * input buffer.
                             *********************************************************/

                            if ((msgBuf = channel.read(readArgs, error)) != null)
                            {
                                /* if a buffer is returned, we have data to process and code is success */
                                msgBuf.length();
                                /* Processes a response from the channel/connection. This consists of performing a high level decode of the message and then
                                 * calling the applicable specific function for further processing.
                                 */
                            }
                            else
                            {

                                /* Deduce an action from the return code of Channel.read() */
                                retCode = readArgs.readRetVal();
                                switch (retCode)
                                {
                                    /* (-13) Transport Success: Read has received a ping message. There is no buffer in this case. */
                                    /* Acknowledge that a ping has been received */

                                    case TransportReturnCodes.READ_PING:
                                    {
                                        /* Update ping monitor */
                                        /* set flag for server message received */
                                        receivedServerMsg = true;
                                    }
                                        break;

                                    /* (-14) Transport Success: Read received an FD change event. The application should unregister the oldSocketId and
                                     * register the socketId with its notifier
                                     */
                                    /* Switch to a new channel if required */
                                    case TransportReturnCodes.READ_FD_CHANGE:
                                    {
                                        /* File descriptor changed, typically due to tunneling keep-alive */
                                        /* Unregister old socketId and register new socketId */
                                        opMask = SelectionKey.OP_READ;
                                        final int oldChannelFDValue = channelFDValue;
                                        channelFDValue = TrainingModuleUtils.getFDValueOfSelectableChannel(channel.selectableChannel());
                                        System.out.printf("\nChannel In Progress - New FD: %d   Old FD: %d\n", channelFDValue, oldChannelFDValue);
                                        try
                                        {
                                            key = channel.selectableChannel().keyFor(selector);
                                            key.cancel();
                                        }
                                        catch (Exception e)
                                        {
                                            System.out.printf("Exception %s\n", e.getMessage());
                                            closeChannelCleanUpAndExit(channel, selector, error, TransportReturnCodes.FAILURE);
                                        }
                                        try
                                        {
                                            channel.selectableChannel().register(selector, opMask, channel);
                                        }
                                        catch (Exception e)
                                        {
                                            System.out.printf("Exception %s\n", e.getMessage());
                                            closeChannelCleanUpAndExit(channel, selector, error, TransportReturnCodes.FAILURE);
                                        }
                                    }
                                        break;
                                    /* (-11) Transport Success: Reading was blocked by the OS. Typically indicates that there are no bytes available to read,
                                     * returned from Read.
                                     */
                                    case TransportReturnCodes.READ_WOULD_BLOCK: /* Nothing to read */
                                        break;
                                    case TransportReturnCodes.READ_IN_PROGRESS:/* fall through to default. */
                                    case TransportReturnCodes.INIT_NOT_INITIALIZED:
                                    case TransportReturnCodes.FAILURE:
                                        System.out.printf("Error (%d) (errno: %d) %s\n", error.errorId(), error.sysError(), error.text());
                                        closeChannelCleanUpAndExit(channel, selector, error, TransportReturnCodes.FAILURE);
                                        break;

                                    default: /* Error handling */
                                    {

                                        if (retCode < 0)
                                        {
                                            System.out.printf("Error (%d) (errno: %d) encountered with Read. Error Text: %s\n", error.errorId(), error.sysError(), error.text());
                                            closeChannelCleanUpAndExit(channel, selector, error, TransportReturnCodes.FAILURE);
                                        }
                                    }
                                        break;
                                }
                            }
                        }
                    }

                    /* An I/O notification mechanism can be used to indicate when the operating system can accept more data for output.
                     * Flush function is called because of a write file descriptor alert
                     */
                    key = channel.selectableChannel().keyFor(selector);
                    if (key.isWritable())
                    {
                        /* Flush */
                    }
                }

                /* Processing ping management handler */
                if ((retCode = processPingManagementHandler(channel, error, opMask, selector)) > TransportReturnCodes.SUCCESS)
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
                    closeChannelCleanUpAndExit(channel, selector, error, TransportReturnCodes.FAILURE);
                }

                /* get current time */
                currentTime = System.currentTimeMillis();

                /* Handles the run-time for the UPA NI Provider application. Here we exit the application after a predetermined time to run */
                /* If the runtime has expired */
                if (System.currentTimeMillis() >= upaRuntime)
                {
                    System.out.printf("UPA Client run-time has expired...\n\n");
                    closeChannelCleanUpAndExit(channel, selector, error, TransportReturnCodes.SUCCESS);
                }
            }
            catch (IOException e1)
            {
                e1.printStackTrace();
            }
        }
    }

    /*
     * Closes channel, cleans up and exits the application.
     * upaChannel - The channel to be closed
     * code - if exit due to errors/exceptions
     */
    public static void closeChannelCleanUpAndExit(Channel channel, Selector selector, Error error, int code)
    {
        boolean isClosedAndClean = true;
        try
        {
            selector.close();
        }
        catch (Exception e)
        {
            System.out.printf("Exception %s\n", e.getMessage());
        }

        /*********************************************************
         * Client/NIProv Application Life Cycle Major Step 5: Close connection
         * using CloseChannel (OS connection release handshake) CloseChannel
         * closes the client based Channel. This will release any pool based
         * resources back to their respective pools, close the connection, and
         * perform any additional necessary cleanup. When shutting down the
         * Transport, the application should release all unwritten pool buffers.
         * Calling CloseChannel terminates the connection to the ADH.
         *********************************************************/

        if ((channel != null)) {
            isClosedAndClean = channel.close(error) >= TransportReturnCodes.SUCCESS;
        }

        /*********************************************************
         * Client/NIProv Application Life Cycle Major Step 6: Uninitialize UPA
         * Transport using Uninitialize The last UPA Transport function that an
         * application should call. This uninitializes internal data structures
         * and deletes any allocated memory.
         *********************************************************/

        /* All UPA Transport use is complete, must uninitialize.
         * The uninitialization process allows for any heap allocated memory to be cleaned up properly.
         */
        Transport.uninitialize();

        if (isClosedAndClean) {
            System.out.println("NIProvider application has closed channel and has cleaned up successfully.");
        } else {
            System.out.printf("Error (%d) (errno: %d) encountered with CloseChannel. Error Text: %s\n", error.errorId(), error.sysError(), error.text());
        }

        /* For applications that do not exit due to errors/exceptions such as:
         * Exits the application if the run-time has expired.
         */
        if (code == TransportReturnCodes.SUCCESS)
            System.out.printf("\nUPA NI Provider Training application successfully ended.\n");

        /* End application */
        System.exit(0);

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
        /* set ping timeout for local and remote pings */
        pingTimeoutClient = channel.pingTimeout() / 3;
        pingTimeoutServer = channel.pingTimeout();

        /* set time to send next ping to remote connection */
        nextSendPingTime = System.currentTimeMillis() + pingTimeoutClient * 1000;

        /* set time should receive next ping from remote connection */
        nextReceivePingTime = System.currentTimeMillis() + pingTimeoutServer * 1000;
    }

    /*
     * Processing ping management handler
     * upaChannel - The channel for ping management processing
     */
    public static int processPingManagementHandler(Channel channel, Error error, Short opMask, Selector selector)
    {
        /* Handles the ping processing for upaChannel. Sends a ping to the server if the next send ping time has arrived and
         * checks if a ping has been received from the server within the next receive ping time.
         */
        int retval = TransportReturnCodes.SUCCESS;

        /* get current time */
        long currentTime = System.currentTimeMillis();

        /* handle client pings */
        if (currentTime >= nextSendPingTime)
        {
            /* send ping to server */
            /*********************************************************
             * Client/NIProv Application Life Cycle Major Step 4: Ping using
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
                        closeChannelCleanUpAndExit(channel, selector, error, TransportReturnCodes.FAILURE);
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
                closeChannelCleanUpAndExit(channel, selector, error, TransportReturnCodes.FAILURE);

            }
        }
        return retval;
    }
}
