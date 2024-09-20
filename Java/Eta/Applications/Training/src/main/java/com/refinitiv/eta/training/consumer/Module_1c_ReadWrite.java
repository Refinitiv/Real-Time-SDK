/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2019-2022 LSEG. All rights reserved.     
 *|-----------------------------------------------------------------------------
 */

/*********************************************************************************
 * This is the ETA Consumer Training series of the ETA Training Suite
 * applications. The purpose of this application is to show step-by-step 
 * training how to build a ETA OMM Consumer using the ETA Transport layer.
 *
 * Main Java source file for the ETA Consumer Training application. It is a 
 * single-threaded client application.
 *
 *********************************************************************************
 * ETA Consumer Training Module 1a: Establish network communication
 *********************************************************************************
 * Summary:
 * In this module, the application initializes the ETA Transport and 
 * connects the client. An OMM consumer application can establish a 
 * connection to other OMM Interactive Provider applications, including 
 * LSEG Real-Time Distribution Systems, Data Feed Direct,
 * and LSEG Real-Time. 
 *
 * Detailed Descriptions:
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
 *
 * Command line usage:
 *
 * ./gradlew runconsumermod1a
 * (runs with a default set of parameters (-h localhost -p 14002 -i ""))
 * or
 *
 * ./gradlew runconsumermod1a -PcommandLineArgs="[-h <SrvrHostname>] [-p <SrvrPortNo>] [-i <InterfaceName>]"
 * (runs with specified set of parameters, all parameters are optional)
 *
 * Pressing the CTRL+C buttons terminates the program.
 *
 *********************************************************************************
 * ETA Consumer Training Module 1b: Ping (heartbeat) Management
 *********************************************************************************
 * Summary:
 * Ping or heartbeat messages indicate the continued presence of an application. 
 * After the consumer connection is active, ping messages must be exchanged. 
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
 *
 * Command line usage:
 * ./gradlew runconsumermod1b
 * (runs with a default set of parameters (-h localhost -p 14002 -i "" -r 300))
 *
 * or
 *
 * ./gradlew runconsumermod1b -PcommandLineArgs="[-h <SrvrHostname>] [-p <SrvrPortNo>] [-i <InterfaceName>] [-r <Running Time>]"
 * (runs with specified set of parameters, all parameters are optional)
 *
 * Pressing the CTRL+C buttons terminates the program.
 * 
 *********************************************************************************
 * ETA Consumer Training Module 1c: Reading and Writing Data
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
 * ./gradlew runconsumermod1c
 * (runs with a default set of parameters (-h localhost -p 14002 -i "" -r 300))
 *
 * or
 *
 * ./gradlew runconsumermod1c -PcommandLineArgs="[-h <SrvrHostname>] [-p <SrvrPortNo>] [-i <InterfaceName>] [-r <Running Time>]"
 * (runs with specified set of parameters, all parameters are optional)
 *
 * Pressing the CTRL+C buttons terminates the program.
 *******************************************************************************/

package com.refinitiv.eta.training.consumer;

import java.io.IOException;
import java.nio.channels.ClosedChannelException;
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
import com.refinitiv.eta.transport.Transport;
import com.refinitiv.eta.transport.TransportFactory;
import com.refinitiv.eta.transport.TransportReturnCodes;
import com.refinitiv.eta.transport.ReadArgs;
import com.refinitiv.eta.codec.CodecFactory;
import com.refinitiv.eta.codec.CodecReturnCodes;
import com.refinitiv.eta.codec.DecodeIterator;
import com.refinitiv.eta.codec.Msg;
import com.refinitiv.eta.rdm.DomainTypes;
import com.refinitiv.eta.transport.TransportBuffer;

public class Module_1c_ReadWrite
{

    public static void main(String[] args)
    {
        /* For this simple training app, only a single channel/connection is used for the entire life of this app. */

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

        /* Create ChannelInfo */
        ChannelInfo channelInfo = TransportFactory.createChannelInfo();

        TransportBuffer msgBuf = null;

        long currentTime = 0;
        long etaRuntime = 0;
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
        /* server is running on port number 14002 */
        String srvrPortNo = "14002";
        /* use default NIC network interface card to bind to for all inbound and outbound data */
        String interfaceName = "";
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
                else
                {
                    System.out.printf("Error: Unrecognized option: %s\n\n", args[i]);
                    System.out.printf("Usage: %s or\n%s [-h <SrvrHostname>] [-p <SrvrPortNo>] [-i <InterfaceName>] [-r <runTime>] \n", args[0], args[0]);
                    System.exit(TransportReturnCodes.FAILURE);
                }
            }
        }

        /**************************************************************************************************
         * INITIALIZATION
         **************************************************************************************************/
        /*********************************************************
         * Client/Consumer Application Life cycle Major Step 1: Initialize ETA
         * Transport using Initialize The first ETA Transport function that an
         * application should call. This creates and initializes internal memory
         * and structures, as well as performing any bootstrapping for
         * underlying dependencies. The Initialize function also allows the user
         * to specify the locking model they want applied to the ETA Transport.
         *********************************************************/
        if (Transport.initialize(initArgs, error) != TransportReturnCodes.SUCCESS)
        {
            System.out.printf("Error (%d) (errno: %d): %s\n", error.errorId(), error.sysError(), error.text());
            System.exit(TransportReturnCodes.FAILURE);
        }
        currentTime = System.currentTimeMillis();
        etaRuntime = currentTime + runTime * 1000;

        /* Set connect options */
        /* populate connect options, then pass to Connect function -
         * ETA Transport should already be initialized
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

        /**************************************************************************************************
         * Connect to Server and receive a channel
         **************************************************************************************************/
        /*********************************************************
         * Client/Consumer Application life cycle Major Step 2: Connect using
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

        /**************************************************************************************************
         * MAIN LOOP - Keep calling Channel.init() until it has properly
         * connected us to the server
         **************************************************************************************************/
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
                    System.out.printf("Channel initialization has timed out.");
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
                        /* Internally, the ETA initialization process includes several actions. The initialization includes
                         * any necessary ETA connection handshake exchanges, including any HTTP or HTTPS negotiation.
                         * Compression, ping timeout, and versioning related negotiations also take place during the
                         * initialization process. This process involves exchanging several messages across the connection,
                         * and once all message exchanges have completed the Channel.state() will transition. If the connection
                         * is accepted and all types of negotiations completed properly, the Channel.state() will become
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
                            System.out.printf("Error (%d) (errno: %d) encountered with Init Channel fd=%d. Error Text: %s\n", error.errorId(), error.sysError(), channelFDValue, error.text());
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
                                    final int oldChannelFDValue = channelFDValue;
                                    channelFDValue = TrainingModuleUtils.getFDValueOfSelectableChannel(channel.selectableChannel());
                                    System.out.printf("\nChannel In Progress - New FD: %d   Old FD: %d\n", channelFDValue, oldChannelFDValue);
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
                                    System.out.printf("Channel %d in progress...\n", channelFDValue);
                                }
                            }
                                break;
                            /* channel connection becomes active!
                             * Once a connection is established and transitions to the ChannelState.ACTIVE state,
                             * this Channel can be used for other transport operations.
                             */
                            case TransportReturnCodes.SUCCESS:
                            {
                                System.out.printf("Channel on fd %d is now active - reading and writing can begin.\n", channelFDValue);

                                /* Populate information from channel */
                                if ((retCode = channel.info(channelInfo, error)) != TransportReturnCodes.SUCCESS)
                                {
                                    System.out.printf("Error (%d) (errno: %d) encountered with Init Channel fd=%d. Error Text: %s\n", error.errorId(), error.sysError(), channelFDValue, error.text());
                                    closeChannelCleanUpAndExit(channel, selector, TransportReturnCodes.FAILURE);
                                }

                                /* Print out basic channel info */
                                System.out.printf("\nChannel %d active. Channel Info:\n" + "Max Fragment Size:           %d\n" + "Output Buffers:              %d Max, %d Guaranteed\n" + "Input Buffers:               %d\n" + "Send/Receive Buffer Sizes:   %d/%d\n" + "Ping Timeout:                %d\n",
                                                      channelFDValue,
                                                      channelInfo.maxFragmentSize(), /*  This is the max fragment size before fragmentation and reassembly is necessary. */
                                                      channelInfo.maxOutputBuffers(), /*  This is the maximum number of output buffers available to the channel. */
                                                      channelInfo.guaranteedOutputBuffers(), /*  This is the guaranteed number of output buffers available to the channel. */
                                                      channelInfo.numInputBuffers(), /*  This is the number of input buffers available to the channel. */
                                                      channelInfo.sysSendBufSize(), /*  This is the systems Send Buffer size. This reports the systems send buffer size respective to the transport type being used (TCP, UDP, etc) */
                                                      channelInfo.sysRecvBufSize(), /*  This is the systems Receive Buffer size. This reports the systems receive buffer size respective to the transport type being used (TCP, UDP, etc) */
                                                      channelInfo.pingTimeout()); /*  This is the value of the negotiated ping timeout */

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
                            default: /* Error handling */
                            {
                                System.out.printf("Bad return value fd=%d: <%s>\n", channelFDValue, TransportReturnCodes.toString(retCode));
                                /* Closes channel, closes server, cleans up and exits the application. */
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

        /****************************************************
         * Loop 2) Check ping operations until runtime ends *
         ****************************************************/
        /* Here were are using a new Main loop. An alternative design would be to combine this Main loop with
         * the Main loop for getting connection active. Some bookkeeping would be required for that approach.
         */
        /* Add two functions at the end compared to module_1a:initPingManagementHandler() and
        * processPingManagementHandler() */

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
                                    closeChannelCleanUpAndExit(channel, selector, TransportReturnCodes.FAILURE);
                                }
                                /* decode contents into the Msg structure */
                                if ((retCode = msg.decode(decodeIter)) != CodecReturnCodes.SUCCESS)
                                {
                                    System.out.printf("Error (%d) (errno: %d) encountered with Init Channel fd=%d. Error Text: %s\n", error.errorId(), error.sysError(), channelFDValue, error.text());
                                    /* Closes channel, cleans up and exits the application. */
                                    closeChannelCleanUpAndExit(channel, selector, TransportReturnCodes.FAILURE);
                                }
                                /*After acquiring data in buffer, we can check what kind of information it is. It may be login message, update
                                *or other info. This part will be fulfilled in latter examples like module of login and source directory. */

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
                                        /* Update ping monitor */
                                        /* set flag for server message received */
                                        receivedServerMsg = true;
                                        System.out.printf("Ping message has been received successfully from the server due to ping message ...\n\n");
                                        break;

                                    /* Switch to a new channel if required */
                                    case TransportReturnCodes.READ_FD_CHANGE:
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
                                    case TransportReturnCodes.FAILURE:/* fall through to default. */
                                    default: /* Error handling */

                                        if (retCode < 0)
                                        {
                                            System.out.printf("Error (%d) (errno: %d) encountered with Read. Error Text: %s\n", error.errorId(), error.sysError(), error.text());
                                            /* Closes channel/connection, cleans up and exits the application. */
                                            closeChannelCleanUpAndExit(channel, selector, TransportReturnCodes.FAILURE);
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
                                        e.printStackTrace();
                                    }
                                }
                                    break;
                                case TransportReturnCodes.FAILURE:
                                default:
                                {
                                    System.out.printf("Error (%d) (errno: %d) encountered with Init Channel fd=%d. Error Text: %s\n", error.errorId(), error.sysError(), channelFDValue, error.text());
                                    closeChannelCleanUpAndExit(channel, selector, TransportReturnCodes.FAILURE);
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

                /* Handles the run-time for the ETA Consumer application. Here we exit the application after a predetermined time to run */
                /* If the runtime has expired */
                if (System.currentTimeMillis() >= etaRuntime)
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
                    System.out.printf("ETA Client run-time has expired...\n");
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
        boolean isClosedAndClean = true;
        Error error = TransportFactory.createError();
        /*********************************************************
         * Client/Consumer Application Life cycle Major Step 5: Close connection
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

        if ((channel != null)) {
            isClosedAndClean = channel.close(error) >= TransportReturnCodes.SUCCESS;
        }

        /*********************************************************
         * Client/Consumer Application Life cycle Major Step 6: Uninitialize ETA
         * Transport using Uninitialize The last ETA Transport function that an
         * application should call. This uninitializes internal data structures
         * and deletes any allocated memory.
         *********************************************************/

        /* All ETA Transport use is complete, must uninitialize.
         * The uninitialization process allows for any heap allocated memory to be cleaned up properly.
         */
        Transport.uninitialize();

        if (isClosedAndClean) {
            System.out.println("Consumer application has closed channel and has cleaned up successfully.");
        } else {
            System.out.printf("Error (%d) (errno: %d): %s\n", error.errorId(), error.sysError(), error.text());
        }

        if (code == TransportReturnCodes.SUCCESS)
        {
            System.out.printf("\nETA Consumer Training Application successfully ended.\n");
        }

        System.exit(0);
    }

    /*
     * Initializes the ping times for etaChannel.
     * etaChannel - The channel for ping management info initialization
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
     * etaChannel - The channel for ping management processing
     */
    public static int processPingManagementHandler(Channel channel)
    {
        /* Handles the ping processing for channel. Sends a ping to the server if the next send ping time has arrived and
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
             * Client/Consumer Application Life Cycle Major Step 4: Attempts to
             * write a heartbeat message on the connection. This function
             * expects the Channel to be in the active state. If an application
             * calls the Ping function while there are other bytes queued for
             * output, the ETA Transport layer will suppress the heartbeat
             * message and attempt to flush bytes to the network on the user's
             * behalf.
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
                return TransportReturnCodes.FAILURE;

            }
        }
        return retval;
    }

}
