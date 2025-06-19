/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2023-2024 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
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
 * LSEG Real-Time Distribution Systems, Data Feed Direct,
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
 *******************************************************************************/

using System;
using System.Collections.Generic;
using System.Linq;
using System.Net.Sockets;

using LSEG.Eta.Transports;
using LSEG.Eta.Codec;
using LSEG.Eta.Rdm;
using LSEG.Eta.Common;

namespace LSEG.Eta.Training.Consumer
{
    public class Module_1c_ReadWrite
    {
        #region Global Variables

        static List<Socket> readSocketList = new();
        static List<Socket> writeSocketList = new();

        /* A channel to keep track of connection */
        static IChannel? channel;

        static long channelFDValue = -1;

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

            /* the default option parameters */
            /* connect to server running on same machine */
            string srvrHostname = "localhost";
            /* server is running on port number 14003 */
            string srvrPortNo = "14002";
            /* use default NIC network interface card to bind to for all inbound and outbound data */
            string interfaceName = "";

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
                    else
                    {
                        Console.Write("Error: Unrecognized option: {0}\n\n", args[i]);
                        Console.Write("Usage: {0} or\n{0} [-h <SrvrHostname>] [-p <SrvrPortNo>] [-i <InterfaceName>] [-r <runTime>] \n",
                            System.AppDomain.CurrentDomain.FriendlyName);
                        Environment.Exit((int)TransportReturnCode.FAILURE);
                    }
                }
            }

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
                            CloseChannelCleanUpAndExit(channel, TransportReturnCode.FAILURE);
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
                                            CloseChannelCleanUpAndExit(channel, TransportReturnCode.FAILURE);
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
                                        CloseChannelCleanUpAndExit(channel, TransportReturnCode.FAILURE);
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
                    CloseChannelCleanUpAndExit(channel, TransportReturnCode.FAILURE);
                }
            }

            /* Initialize ping management handler */
            InitPingManagement(channel);

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
                                    CloseChannelCleanUpAndExit(channel, TransportReturnCode.FAILURE);
                                }
                                /* decode contents into the Msg structure */
                                if ((ret = msg.Decode(decodeIter)) != CodecReturnCode.SUCCESS)
                                {
                                    Console.Write("Error ({0}) (errno: {1}) encountered with DecodeMsg.\n",
                                        ret.GetAsInfo(), ret);
                                    /* Closes channel, cleans up and exits the application. */
                                    CloseChannelCleanUpAndExit(channel, TransportReturnCode.FAILURE);
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
                                            /* Skip login in this Read & Write example.
                                             * Logging procedure shown in ConsMod2 Login */
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
                                    case TransportReturnCode.FAILURE:/* fall through to default. */
                                    default: /* Error handling */

                                        if (retCode < 0)
                                        {
                                            Console.Write("Error ({0}) (errno: {1}) encountered with Read. Error Text: {2}\n",
                                                error.ErrorId, error.SysError, error.Text);
                                            /* Closes channel/connection, cleans up and exits the application. */
                                            CloseChannelCleanUpAndExit(channel, TransportReturnCode.FAILURE);
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
                                    CloseChannelCleanUpAndExit(channel, TransportReturnCode.FAILURE);
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
                        if ((retCode = channel.Flush(out error)) >= TransportReturnCode.SUCCESS)
                        {
                            /* retval == TransportReturnCode.SUCCESS - All was flushed.
                            /* retval == TransportReturnCode.SUCCESS - There is still data left to flush.
                            * If everything wasn't flushed, it usually indicates that the TCP output buffer cannot accept more yet
                            */
                        }
                        else
                        {
                            Console.Write("Channel failed to flush \n");
                            CloseChannelCleanUpAndExit(channel, TransportReturnCode.FAILURE);
                        }
                    }
                    else if (retCode < TransportReturnCode.SUCCESS)
                    {
                        /* Closes channel, cleans up and exits the application. */
                        CloseChannelCleanUpAndExit(channel, TransportReturnCode.FAILURE);
                    }

                    /* get current time */
                    currentTime = System.DateTime.Now;

                    /* Handles the run-time for the ETA Consumer application. Here we exit the application after a predetermined time to run */
                    /* If the runtime has expired */
                    if (currentTime >= etaRuntime)
                    {
                        /* Flush before exiting */
                        if (channel.Socket.Poll(1000000, System.Net.Sockets.SelectMode.SelectWrite))
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

                        Console.Write("ETA Client run-time has expired...\n");
                        CloseChannelCleanUpAndExit(channel, TransportReturnCode.SUCCESS);
                    }
                }
                catch (Exception e1)
                {
                    Console.WriteLine("Exception: {0}\nStack trace:\n{1}", e1.Message, e1.StackTrace);
                    CloseChannelCleanUpAndExit(channel, TransportReturnCode.FAILURE);
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
        public static void CloseChannelCleanUpAndExit(IChannel channel, TransportReturnCode code)
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
    }
}
