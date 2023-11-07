/*
 *|-------------------------------------------------------------------------------
 *| This source code is provided under the Apache 2.0 license and is provided   --
 *| AS IS with no warranty or guarantee of fit for purpose.  See the project's  --
 *| LICENSE.md for details.                                                     --
 *| Copyright (C) 2022-2023 Refinitiv. All rights reserved.                          --
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
 * Refinitiv Real-Time Distribution Systems, Refinitiv Data Feed Direct,
 * and Refinitiv Real-Time.
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
 *********************************************************************************/

using System;
using System.Collections.Generic;
using System.Linq;
using System.Net.Sockets;

using LSEG.Eta.Transports;
using LSEG.Eta.Common;
using LSEG.Eta.Codec;
using LSEG.Eta.Rdm;

using Buffer = LSEG.Eta.Codec.Buffer;

namespace LSEG.Eta.Training.Consumer
{
    public class Module_2_Login
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
                CloseChannelCleanUpAndExit(channel, TransportReturnCode.FAILURE);
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
                                                CloseChannelCleanUpAndExit(channel, TransportReturnCode.FAILURE);
                                            }
                                            else
                                            {
                                                Console.Write("ETA Consumer application is granted access and has logged in successfully.\n\n");
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
                        /* Closes all streams for the consumer after run-time has elapsed. */

                        /* Close Login stream */
                        /* Note that closing Login stream will automatically close all other streams at the provider */
                        if ((retCode = CloseLoginStream(channel, channelInfo.MaxFragmentSize)) < TransportReturnCode.SUCCESS)
                        {
                            /* When you close login, we want to make a best effort to get this across the network as it
                             * will gracefully close all open streams. If this cannot be flushed or failed, this
                             * application will just close the connection for simplicity.
                             */

                            /* Closes channel, cleans up and exits the application. */
                            CloseChannelCleanUpAndExit(channel, TransportReturnCode.FAILURE);
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
    }
}
