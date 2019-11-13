/*
 *|-------------------------------------------------------------------------------
 *| This source code is provided under the Apache 2.0 license and is provided	--
 *| AS IS with no warranty or guarantee of fit for purpose.  See the project's 	--
 *| LICENSE.md for details.														--
 *| Copyright (C) 2019 Refinitiv. All rights reserved.						--
 *|-------------------------------------------------------------------------------
 */

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
 *********************************************************************************/

package com.thomsonreuters.upa.training.consumer;

import java.io.IOException;
import java.nio.channels.SelectionKey;
import java.nio.channels.Selector;
import java.util.Iterator;
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

public class Module_1a_Connect
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
                else
                {
                    System.out.printf("Error: Unrecognized option: %s\n\n", args[i]);
                    System.out.printf("Usage: %s or\n%s [-h <SrvrHostname>] [-p <SrvrPortNo>] [-i <InterfaceName>] \n", args[0], args[0]);
                    System.exit(TransportReturnCodes.FAILURE);
                }
            }
        }
        /**************************************************************************************************
         * INITIALIZATION
         **************************************************************************************************/
        /*********************************************************
         * Client/Consumer Application Life Cycle Major Step 1: Initialize UPA
         * Transport using Initialize The first UPA Transport function that an
         * application should call. This creates and initializes internal memory
         * and structures, as well as performing any bootstrapping for
         * underlying dependencies. The Initialize function also allows the user
         * to specify the locking model they want applied to the UPA Transport.
         *********************************************************/
        if (Transport.initialize(initArgs, error) != TransportReturnCodes.SUCCESS)
        {
            System.out.printf("Error (%d) (errno: %d): %s\n", error.errorId(), error.sysError(), error.text());
            System.exit(TransportReturnCodes.FAILURE);
        }

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

        /* populate version and protocol with RWF information or protocol specific info */
        cOpts.protocolType(Codec.protocolType()); /* Protocol type definition for RWF */
        cOpts.majorVersion(Codec.majorVersion());
        cOpts.minorVersion(Codec.minorVersion());

        /**************************************************************************************************
         * Connect to Server and receive a channel
         **************************************************************************************************/
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
            System.out.printf("Error (%d) (errno: %d) encountered with Connect. Error Text : %s\n", error.errorId(), error.sysError(), error.text());
            /* End application, uninitialize to clean up first */
            Transport.uninitialize();
            System.exit(TransportReturnCodes.FAILURE);
        }
        else
        {

            System.out.printf("Channel successfully connected to server.\n");
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

        /**************************************************************************************************
         * MAIN LOOP - Keep calling Channel.init() until it has properly
         * connected us to the server
         **************************************************************************************************/
        /* Main loop for getting connection active and successful completion of the initialization process
         * The loop calls select() to wait for notification
         * Currently, the main loop would exit if an error condition is triggered or
         * Channel.state() transitions to ChannelState.ACTIVE.
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
                            System.out.printf("Error (%d) (errno: %d): %s\n", error.errorId(), error.sysError(), error.text());
                            closeChannelCleanUpAndExit(channel, selector, TransportReturnCodes.FAILURE);
                        }
                        else
                        {
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
                                default: /* Error handling */
                                {
                                    System.out.printf("Unexpected init return code: %d\n", retCode);
                                    /* Closes channel, closes server, cleans up and exits the application. */
                                    closeChannelCleanUpAndExit(channel, selector, TransportReturnCodes.FAILURE);
                                }
                                    break;
                            }
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
         * Client/Consumer Application Life Cycle Major Step 5: Close connection
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
         * Client/Consumer Application Life Cycle Major Step 6: Uninitialize UPA
         * Transport using Uninitialize The last UPA Transport function that an
         * application should call. This uninitializes internal data structures
         * and deletes any allocated memory.
         *********************************************************/

        /*
         * All UPA Transport use is complete, must uninitialize. The
         * uninitialization process allows for any heap allocated memory to be
         * cleaned up properly.
         */
        Transport.uninitialize();

        if (code == TransportReturnCodes.SUCCESS)
        {
            System.out.printf("\nUPA Consumer Training Application successfully ended.\n");
        }

        System.exit(code);
    }
}