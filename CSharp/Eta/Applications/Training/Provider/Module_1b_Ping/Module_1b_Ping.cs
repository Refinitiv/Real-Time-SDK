/*
 *|-------------------------------------------------------------------------------
 *| This source code is provided under the Apache 2.0 license and is provided   --
 *| AS IS with no warranty or guarantee of fit for purpose.  See the project's  --
 *| LICENSE.md for details.                                                     --
 *| Copyright (C) 2022-2023 Refinitiv. All rights reserved.                          --
 *|-------------------------------------------------------------------------------
 */

/*****************************************************************************************
 * This is the ETA Interactive Provider Training series of the ETA Training Suite
 * applications. The purpose of this application is to show step-by-step
 * training how to build a ETA OMM Interactive Provider using the ETA Transport layer.
 *
 * Main C# source file for the ETA Interactive Provider Training application. It is a
 * single-threaded client application.
 *
 *****************************************************************************************
 * ETA Interactive Provider Training Module 1a: Establish network communication
 *****************************************************************************************
 *
 * Summary:
 *
 * An OMM Interactive Provider application opens a listening socket on a well-known
 * port allowing OMM consumer applications to connect. Once connected, consumers
 * can request data from the Interactive Provider.
 *
 * In this module, the OMM Interactive Provider application opens a listening socket
 * on a well-known port allowing OMM consumer applications to connect.
 *
 * Detailed Descriptions:
 *
 * The first step of any ETA Interactive Provider application is to establish
 * a listening socket, usually on a well-known port so that consumer applications
 * can easily connect. The provider uses the Transport.Bind() method to open the port
 * and listen for incoming connection attempts.
 *
 * Whenever an OMM consumer application attempts to connect, the provider uses
 * the IServer.Accept() method to begin the connection initialization process.
 *
 * For this simple training app, the interactive provider only supports a single client.
 *
 * Command line usage:
 *
 * ProvMod1a.exe
 * (runs with a default set of parameters (-p 14002))
 *
 * or
 *
 * ProvMod1a.exe [-p <SrvrPortNo>]
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
 *
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
 * ProvMod1b.exe
 * (runs with a default set of parameters (-p 14002 -r 300))
 *
 * or
 *
 * ProvMod1b.exe [-p <SrvrPortNo>] [-r <Running Time>]
 * (runs with specified set of parameters, all parameters are optional)
 *
 * Pressing the CTRL+C buttons terminates the program.
 *****************************************************************************************/

using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Net.Sockets;

using LSEG.Eta.Common;

using LSEG.Eta.Transports;


namespace LSEG.Eta.Training.Provider
{
    public class Module_1b_Ping
    {
        #region Global Variables

        static List<Socket> readSocketList = new();
        static List<Socket> writeSocketList = new();

        /* For this simple training app, the interactive provider only supports a single client. If the
         * consumer disconnects, the interactive provider would simply exit.
         *
         * If you want the provider to support multiple client sessions at the same time, you need to
         * implement support for multiple client sessions feature similar to what Provider example is
         * doing.
         */
        /* Create a channel to keep track of connection */
        static IChannel? channel = null;

        /* Max waiting time */
        static TimeSpan timeOut = TimeSpan.FromSeconds(60);

        #endregion

        /// <summary>The main method.</summary>
        public static void Main(string[] args)
        {
            #region Declare Variables

            /* Create a server to eventually accept connection requests */
            IServer? server;

            /* Create initialization arguments (InitArgs) to specify any initialization options for Transport */
            InitArgs initArgs = new();

            /* Create error to keep track of any errors that may occur in Transport methods */
            Error error = new();

            /* Create channel info as a holder */
            ChannelInfo channelInfo = new();

            ITransportBuffer? msgBuf;

            /* Create accept options to specify any options for accepting */
            AcceptOptions acceptOpts = new() { NakMount = false };

            /* Create initialization progress info (InProgInfo) to keep track of channel initialization with Channel.Init() */
            InProgInfo inProgInfo = new();

            TransportReturnCode retCode;

            /* the default option parameters */
            /* server is running on port number 14002 */
            string srvrPortNo = "14002";

            /* use default runTime of 300 seconds */
            TimeSpan runTime = TimeSpan.FromSeconds(300);
            #endregion

            /* User specifies options such as address, port, and interface from the command line.
             * User can have the flexibility of specifying any or all of the parameters in any order.
             */
            if (args.Length > 0)
            {
                int i = 0;

                while (i < args.Length)
                {
                    if ((args[i].Equals("-p")) == true)
                    {
                        i += 2;
                        srvrPortNo = args[i - 1];
                    }
                    else if ((args[i].Equals("-r")) == true)
                    {
                        i += 2;
                        if (Int64.TryParse(args[i - 1], out var runTimeSeconds))
                        {
                            runTime = TimeSpan.FromSeconds(runTimeSeconds);
                        }
                        else
                        {
                            Console.WriteLine("Error: Could not parse runTime: {0}\n", args[i - 1]);
                            Console.WriteLine("Usage: {0} or\n{0} [-p <SrvrPortNo>] [-r <runTime>]",
                                System.AppDomain.CurrentDomain.FriendlyName);
                            Environment.Exit((int)TransportReturnCode.FAILURE);
                        }
                    }
                    else
                    {
                        Console.WriteLine("Error: Unrecognized option: {0}\n", args[i]);
                        Console.WriteLine("Usage: {0} or\n{0} [-p <SrvrPortNo>] [-r <runTime>]", System.AppDomain.CurrentDomain.FriendlyName);
                        Environment.Exit((int)TransportReturnCode.FAILURE);
                    }
                }
            }

            /**************************************************************************************************
             * INITIALIZATION
             **************************************************************************************************/
            /*********************************************************
             * Server/Provider Application Life Cycle Major Step 1: Initialize ETA
             * Transport using Initialize The first ETA Transport function that an
             * application should call. This creates and initializes internal memory
             * and structures, as well as performing any bootstrapping for
             * underlying dependencies. The Initialize function also allows the user
             * to specify the locking model they want applied to the ETA Transport.
             *********************************************************/
            if (Transport.Initialize(initArgs, out error) != TransportReturnCode.SUCCESS)
            {
                Console.WriteLine("Error ({0}) (errno: {1}) {2}",
                    error.ErrorId, error.SysError, error.Text);
                Environment.Exit((int)TransportReturnCode.FAILURE);
            }

            System.DateTime currentTime = System.DateTime.Now;
            System.DateTime etaRuntime = currentTime + runTime;

            /* populate bind options, then pass to Bind function - ETA Transport should
             * already be initialized */
            BindOptions bindOpts = new BindOptions()
            {
                /* server is running on default port number 14002 */
                ServiceName = srvrPortNo,
                /* servers desired ping timeout is 60 seconds, pings should be sent every 20 */
                PingTimeout = 60,
                /* min acceptable ping timeout is 30 seconds, pings should be sent every 10 */
                MinPingTimeout = 30,

                /* set up buffering, configure for shared and guaranteed pools */
                GuaranteedOutputBuffers = 1000,
                MaxOutputBuffers = 2000,
                SharedPoolSize = 50000,
                SharedPoolLock = true,

                /* perform non-blocking I/O */
                ServerBlocking = false,
                /* perform non-blocking I/O */
                ChannelIsBlocking = false,
                /* server does not desire compression for this connection */
                CompressionType = CompressionType.NONE,

                /* populate version and protocol with RWF information (found in Iterators.h) or protocol specific info */
                /* Protocol type definition for RWF */
                ProtocolType = (Transports.ProtocolType)Codec.Codec.ProtocolType(),
                MajorVersion = Codec.Codec.MajorVersion(),
                MinorVersion = Codec.Codec.MinorVersion()
            };
            #region Bind Server and Accept Client

            /**************************************************************************************************
             * Bind and receive a server
             **************************************************************************************************/
            /*********************************************************
             * Server/Provider Application Life Cycle Major Step 2: Create listening
             * socket using Bind Establishes a listening socket connection, which
             * supports connections from standard socket users.
             *
             * Returns an Server that represents the listening socket connection to
             * the user. In the event of an error, NULL is returned and additional
             * information can be found in the Error structure. Options are passed
             * in via an BindOptions structure. Once a listening socket is
             * established, this Server can begin accepting connections.
             *********************************************************/

            /* Bind ETA server */
            if ((server = Transport.Bind(bindOpts, out error)) == null)
            {
                Console.WriteLine("Error ({0}) (errno: {1}) encountered with Bind. Error Text: {2}",
                                  error.ErrorId, error.SysError, error.Text);
                /* End application, uninitialize to clean up first */
                Transport.Uninitialize();
                Environment.Exit((int)TransportReturnCode.FAILURE);
            }

            long clientChannelFDValue = -1;
            long etaServerFDValue = server.Socket.Handle.ToInt64();
            Console.WriteLine("Server IPC descriptor = {0} bound on port {1}", etaServerFDValue, server.PortNumber);

            /**************************************************************************************************
             * MAIN LOOP - Listen for connection requests until we accept a client
             **************************************************************************************************/

            List<Socket> serverWaitList = new() { server.Socket };

            /* Main Loop #1 for detecting incoming client connections. */
            while (channel == null)
            {
                /* Wait for any I/O notification updates in the channel for our specified amount of seconds (e.g. 60 sec.)*/
                try
                {
                    if (serverWaitList.Count == 0)
                        serverWaitList.Add(server.Socket);

                    Socket.Select(serverWaitList, null, null, (int)(timeOut.TotalMilliseconds * 1000));

                    if (serverWaitList.Count > 0)
                    {
                        /**************************************************************************************************
                         * Step 3) Accept the connection request
                         **************************************************************************************************/
                        /*********************************************************
                         * Server/Provider Application Life Cycle Major Step 3: Accept
                         * connection using Accept This step is performed per connected
                         * client/connection/channel Uses the Server that represents the
                         * listening socket connection and begins the accepting process
                         * for an incoming connection request.
                         *
                         * Returns an Channel that represents the client connection. In
                         * the event of an error, NULL is returned and additional
                         * information can be found in the Error structure. The Accept
                         * function can also begin the rejection process for a connection
                         * through the use of the AcceptOptions structure. Once a
                         * connection is established and transitions to the ACTIVE state,
                         * this Channel can be used for other transport operations.
                         *********************************************************/

                        /* An OMM Provider application can begin the connection accepting or rejecting process by using the Accept function */
                        if ((channel = server.Accept(acceptOpts, out error)) == null)
                        {
                            Console.WriteLine("Error ({0}) (errno: {1}) encountered with Init Channel fd={2}. Error Text: {3}",
                                              error.ErrorId, error.SysError, clientChannelFDValue, error.Text);
                            /* End application, uninitialize to clean up first */
                            Transport.Uninitialize();
                            Environment.Exit((int)TransportReturnCode.FAILURE);
                        }
                        else
                        {
                            /* For this simple training app, the interactive provider only supports one client session from the consumer. */
                            clientChannelFDValue = channel.Socket.Handle.ToInt64();
                            Console.WriteLine("\nServer fd = {0}: New client on Channel fd={1}",
                                etaServerFDValue, clientChannelFDValue);
                            /* client is accepted, channel is no longer null, exit the while Main Loop #1*/
                        }
                    }
                }
                catch (IOException e)
                {
                    Console.WriteLine($"Exception {e.Message}");
                    CloseChannelServerCleanUpAndExit(channel, server, TransportReturnCode.FAILURE);
                }
            }
            #endregion

            #region Initialize Channel Connection

            /*****************************************************************************************
             * Loop 2) Keep calling Channel.Init() until it has properly connected
             * us to the server
             *****************************************************************************************/
            /* Main Loop #2 for getting connection active and successful completion of the initialization process
             * Currently, the main loop would exit if an error condition is triggered
             */
            while (channel.State != ChannelState.ACTIVE)
            {
                /* Wait for any I/O notification updates in the channel for our specified amt of seconds (e.g. 60 sec.)*/
                try
                {
                    /* please note that channel might have its underlying Socket changed during initialization */
                    PerformSocketSelect((int)(timeOut.TotalMilliseconds * 1000));

                    /* If our channel has not updated, we must have timed out */
                    if (readSocketList.Count == 0)
                    {
                        Console.WriteLine("Channel initialization has timed out.");
                        CloseChannelServerCleanUpAndExit(channel, server, TransportReturnCode.FAILURE);
                    }
                    else
                    {
                        /****************************************************************************
                         * Step 4) Call Channel.Init() to progress channel initialization further.
                         * This method is called multiple times throughout the Loop 2, as it makes
                         * more progress towards channel initialization.
                         ***************************************************************************/

                        /* Internally, the ETA initialization process includes several actions. The
                         * initialization includes any necessary ETA connection handshake exchanges.
                         * Compression, ping timeout, and versioning related negotiations  also take
                         * place during the initialization process.
                         *
                         * This process involves exchanging several messages across the connection,
                         * and once all message exchanges have completed the Channel.State will transition.
                         *
                         * If the connection is accepted and all types of negotiations completed properly, the
                         * Channel state will become ACTIVE. If the connection is rejected, either due to some
                         * kind of negotiation failure or because an Server rejected the connection by setting
                         * NakMount to true, the Channel state will become CLOSED.
                         *
                         * Note:
                         *
                         * For both client and server channels, more than one call to InitChannel can be
                         * required to complete the channel initialization process.
                         */
                        if ((retCode = channel.Init(inProgInfo, out error)) < TransportReturnCode.SUCCESS)
                        {
                            Console.WriteLine("Error ({0}) (errno: {1}) encountered with Init Channel fd={2}. Error Text: {3}",
                                    error.ErrorId, error.SysError, clientChannelFDValue, error.Text);
                            CloseChannelServerCleanUpAndExit(channel, server, TransportReturnCode.FAILURE);
                        }

                        /* Handle return code appropriately */
                        switch (retCode)
                        {
                            case TransportReturnCode.CHAN_INIT_IN_PROGRESS:
                                {
                                    /* Initialization is still in progress, check the InProgInfo for additional information */
                                    if (inProgInfo.Flags == InProgFlags.SCKT_CHNL_CHANGE)
                                    {
                                        /* The InitChannel function requires the use of an additional
                                         * parameter, a InProgInfo structure.
                                         *
                                         * Under certain circumstances, the initialization process may be
                                         * required to create new or additional underlying connections.
                                         *
                                         * SCKT_CHNL_CHANGE indicates that a socketId change has occurred as a
                                         * result of this call. The previous socketId has been stored in
                                         * InProgInfo.OldSocket so it can be unregistered with the I/O
                                         * notification mechanism.
                                         *
                                         * The new socketId has been stored in InProgInfo.NewSocket so it can
                                         * be registered with the I/O notification mechanism. The channel
                                         * initialization is still in progress and subsequent calls to
                                         * InitChannel are required to complete it.
                                         */
                                        clientChannelFDValue = channel.Socket.Handle.ToInt64();
                                        Console.WriteLine("\nChannel In Progress - New FD: {0}    OLD {1}",
                                            inProgInfo.NewSocket.Handle.ToInt64(), inProgInfo.OldSocket.Handle.ToInt64());
                                        readSocketList.Clear();
                                    }
                                    else
                                    {
                                        Console.WriteLine($"\nChannel {clientChannelFDValue} In Progress...");
                                    }
                                }
                                break;
                            /* channel connection becomes active!
                             * Once a connection is established and transitions to the ACTIVE state,
                             * this Channel can be used for other transport operations.
                             */
                            case TransportReturnCode.SUCCESS:
                                {
                                    Console.WriteLine("Channel on fd {0} is now active - reading and writing can begin.",
                                        clientChannelFDValue);
                                    /*********************************************************
                                     * Connection is now active. The Channel can be
                                     * used for all additional transport
                                     * functionality (e.g. reading, writing) now
                                     * that the state transitions to ACTIVE
                                     *********************************************************/

                                    /* Populate information from channel */
                                    if ((retCode = channel.Info(channelInfo, out error)) != TransportReturnCode.SUCCESS)
                                    {
                                        Console.WriteLine("Error ({0}) (errno: {1}) encountered with channel.Info. Error Text: {2}",
                                            error.ErrorId, error.SysError, error.Text);
                                        /* Connection should be closed, return failure */
                                        /* Closes channel, closes server, cleans up and exits the application. */
                                        CloseChannelServerCleanUpAndExit(channel, server, TransportReturnCode.FAILURE);
                                    }

                                    /* Print out basic channel info */
                                    Console.WriteLine("\nChannel {0} active. Channel Info:\n"
                                                      + "Max Fragment Size:           {1}\n"
                                                      + "Output Buffers:              {2} Max, {3} Guaranteed\n"
                                                      + "Input Buffers:               {4}\n"
                                                      + "Send/Receive Buffer Sizes:   {5}/{6}\n"
                                                      + "Ping Timeout:                {7}",
                                                      clientChannelFDValue,
                                                      /*  This is the max fragment size before fragmentation and reassembly is necessary. */
                                                      channelInfo.MaxFragmentSize,
                                                      /* This is the maximum number of output buffers available to the channel. */
                                                      channelInfo.MaxOutputBuffers,
                                                      /*  This is the guaranteed number of output buffers available to the channel. */
                                                      channelInfo.GuaranteedOutputBuffers,
                                                      /*  This is the number of input buffers available to the channel. */
                                                      channelInfo.NumInputBuffers,
                                                      /*  This is the systems Send Buffer size. This reports the systems
                                                       *  send buffer size respective to the transport type being used
                                                       *  (TCP, UDP, etc) */
                                                      channelInfo.SysSendBufSize,
                                                      /*  This is the systems Receive Buffer size. This reports the
                                                       *  systems receive buffer size respective to the transport type
                                                       *  being used (TCP, UDP, etc) */
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

                                    Console.Write("\n\n");
                                    /* do not allow new client to connect  */
                                    /* For this simple training app, the interactive provider only supports a single
                                     * client. Once a client successfully connects, we call CloseServer function to
                                     * close the listening socket associated with the Server. The connected Channels
                                     * will remain open. This allows the established connection to continue to send and
                                     * receive data, while preventing new clients from connecting.
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

                                    /* clean up server using IServer.Close call.
                                     *
                                     * If a server is being shut down, the IServer.Close function should be used to
                                     * close the listening socket and perform any necessary cleanup. All currently
                                     * connected Channels will remain open. This allows applications to continue to send
                                     * and receive data, while preventing new applications from connecting. The server
                                     * has the option of calling IChannel.Close to shut down any currently connected
                                     * applications.
                                     *
                                     * When shutting down the ETA Transport, the application should release any
                                     * unwritten pool buffers.
                                     *
                                     * The listening socket can be closed by calling IServer.Close. This prevents any
                                     * new connection attempts.  If shutting down connections for all connected clients,
                                     * the provider should call IChannel.Close for each connection client.
                                     */
                                    if ((server != null)
                                        && (server.Close(out error) < TransportReturnCode.SUCCESS))
                                    {
                                        Console.WriteLine("Error ({0}) (errno: {1}) encountered with CloseServer.  Error Text : {2}",
                                            error.ErrorId, error.SysError, error.Text);

                                        /* End application, uninitialize to clean up first */
                                        Transport.Uninitialize();
                                        Environment.Exit((int)TransportReturnCode.FAILURE);
                                    }

                                    /*set server to be null*/
                                    server = null;
                                }
                                break;
                            default: /* Error handling */
                                {
                                    Console.WriteLine("Bad return value fd={0}: <{1}>", clientChannelFDValue, retCode);
                                    /* Closes channel, closes server, cleans up and exits the application. */
                                    CloseChannelServerCleanUpAndExit(channel, server, TransportReturnCode.FAILURE);
                                }
                                break;
                        }
                    }
                }
                catch (IOException e1)
                {
                    Console.WriteLine($"Exception {e1.Message}");
                    CloseChannelServerCleanUpAndExit(channel, server, TransportReturnCode.FAILURE);
                }
            }

            /* Initialize ping management handler */
            InitPingManagement(channel);
            #endregion

            #region Check Ping Operations

            /****************************************************
             * Loop 3) Check ping operations until runtime ends *
             ****************************************************/
            /* Here were are using a new Main loop #3. An alternative design would be to combine this Main
             * loop #3 (message processing) with the other 2 earlier Main loops, namely, Main Loop #1
             * (detecting incoming client connections), and Main Loop #2 (getting connection active and
             * successful completion of the initialization process) as a single provider Main Loop.  Some
             * bookkeeping would be required for that approach.
             */

            /* Main Loop #3 for message processing (reading data, writing data, and ping management, etc.)
             * The only way to exit this Main loop is when an error condition is triggered or after
             * a predetermined run-time has elapsed.
             */
            bool opWrite = false;

            while (true)
            {
                try
                {
                    PerformSocketSelect(1000 * 1000, opWrite);

                    /* If our channel has been updated */
                    if (readSocketList.Count > 0)
                    {
                        /* Initialize to a positive value for retCode in case we have more data that is available to read */
                        retCode = (TransportReturnCode)1;

                        /******************************************************
                         * Loop 4) Read and decode for all buffers in channel *
                         ******************************************************/
                        while (retCode > TransportReturnCode.SUCCESS)
                        {
                            /* Create read arguments (ReadArgs) to keep track of the statuses of any reads */
                            ReadArgs readArgs = new();

                            /**************************************************
                             * Step 5) Read message from channel into buffer *
                             **************************************************/
                            /*********************************************************
                             * Server/Provider Application Life Cycle Major Step 5:
                             * Read using Read Read provides the user with data
                             * received from the connection. This function expects the
                             * Channel to be in the active state.  When data is
                             * available, an Buffer referring to the information is
                             * returned, which is valid until the next call to Read. A
                             * return code parameter passed into the function is used
                             * to convey error information as well as communicate
                             * whether there is additional information to read. An I/O
                             * notification mechanism may not inform the user of this
                             * additional information as it has already been read from
                             * the socket and is contained in the Read input buffer.
                             *********************************************************/
                            msgBuf = channel.Read(readArgs, out error);

                            if (msgBuf == null)
                            {
                                /* Deduce an action from the return code of Channel.Read() */
                                retCode = readArgs.ReadRetVal;
                                /* keep track of the return values from read so data is not stranded in the input buffer.
                                 * Handle return codes appropriately, not all return values are failure conditions
                                 */
                                switch (retCode)
                                {
                                    /* Acknowledge that a ping has been received */
                                    case TransportReturnCode.READ_PING:
                                        /* Update ping monitor */
                                        /* set flag for server message received */
                                        receivedClientMsg = true;
                                        Console.Write("Ping message has been received successfully from the client due to ping message ...\n\n");
                                        break;

                                    /* Switch to a new channel if required */
                                    case TransportReturnCode.READ_FD_CHANGE:

                                        long oldChannelFDValue = clientChannelFDValue;
                                        clientChannelFDValue = channel.Socket.Handle.ToInt64();
                                        Console.WriteLine("\nChannel In Progress - New FD: {0}   Old FD: {1}",
                                                          clientChannelFDValue, oldChannelFDValue);
                                        break;

                                    case TransportReturnCode.READ_WOULD_BLOCK:
                                        /* Nothing to read */
                                        break;
                                    case TransportReturnCode.READ_IN_PROGRESS:
                                        /* Reading from multiple threads */
                                        break;
                                    case TransportReturnCode.INIT_NOT_INITIALIZED:
                                    case TransportReturnCode.FAILURE:
                                    default: /* Error handling */
                                        if (retCode < 0)
                                        {
                                            Console.WriteLine("Error ({0}) (errno: {1}) encountered with Read Error Text: {2}",
                                                              error.ErrorId, error.SysError, error.Text);
                                            CloseChannelServerCleanUpAndExit(channel, server, TransportReturnCode.FAILURE);
                                        }
                                        break;
                                }
                            }
                            /* In this training example we will not encounter any messages containing data */
                            else
                            {
                                Console.WriteLine("\nMessage Received!  Message length is {0} bytes.",
                                                  msgBuf.Length());
                            }
                        }
                    }

                    /* Processing ping management handler */
                    if ((retCode = ProcessPingManagementHandler(channel)) > TransportReturnCode.SUCCESS)
                    {
                        /* There is still data left to flush, leave our write notification enabled so we get called again.
                         * If everything wasn't flushed, it usually indicates that the TCP output buffer cannot accept more yet
                         */

                        /* set write if there's still other data queued */
                        /* flush is done by application */
                        opWrite = true;
                    }
                    else if (retCode < TransportReturnCode.SUCCESS)
                    {
                        /* Closes channel, cleans up and exits the application. */
                        CloseChannelServerCleanUpAndExit(channel, server, TransportReturnCode.FAILURE);
                    }

                    /* get current time */
                    currentTime = System.DateTime.Now;

                    /* If the runtime has expired */
                    if (currentTime >= etaRuntime)
                    {
                        /* Closes all streams for the Interactive Provider after run-time has elapsed in our
                         * simple Interactive Provider example.
                         *
                         * If the provider application must shut down, it can either leave consumer
                         * connections intact or shut them down. If the provider decides to close consumer
                         * connections, the provider should send an StatusMsg on each connection's Login
                         * stream closing the stream.
                         *
                         * At this point, the consumer should assume that its other open streams are also closed.
                         */

                        Console.WriteLine("ETA Server run-time has expired...\n");
                        CloseChannelServerCleanUpAndExit(channel, server, TransportReturnCode.SUCCESS);
                    }
                }
                catch (IOException e1)
                {
                    Console.WriteLine($"Exception {e1.Message}");
                    CloseChannelServerCleanUpAndExit(channel, server, TransportReturnCode.FAILURE);
                }
            }
            #endregion
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

        /// <summary>Closes channel and selector and exits application.</summary>
        ///
        /// <param name="channel">the channel</param>
        /// <param name="server"> the server</param>
        /// <param name="selector"> the selector</param>
        /// <param name="code"> the code</param>
        public static void CloseChannelServerCleanUpAndExit(IChannel? channel, IServer? server, TransportReturnCode code)
        {
            bool isClosedAndClean = true;
            Error? error = null;

            /*********************************************************
             * Server/Provider Application Life Cycle Major Step 6: Close connection
             * using IChannel.Close (OS connection release handshake) IChannel.Close
             * closes the server based Channel.
             *
             * This will release any pool based resources back to their respective pools,
             * close the connection, and perform any additional necessary cleanup. When
             * shutting down the ETA Transport, the application should release all
             * unwritten pool buffers.  Calling IChannel.Close terminates the connection
             * for each connection client.
             *********************************************************/
            if ((channel != null))
            {
                isClosedAndClean = channel.Close(out error) >= TransportReturnCode.SUCCESS;
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

            /* clean up server using Close call.
             *
             * If a server is being shut down, the IServer.Close function should be used to close the
             * listening socket and perform any necessary cleanup. All currently connected Channels will
             * remain open. This allows applications to continue to send and receive data, while preventing
             * new applications from connecting. The server has the option of calling IChannel.Close to shut
             * down any currently connected applications.
             *
             * When shutting down the ETA Transport, the application should release any unwritten pool
             * buffers.  The listening socket can be closed by calling IServer.Close. This prevents any new
             * connection attempts.  If shutting down connections for all connected clients, the provider
             * should call IChannel.Close for each connection client.
            */
            if ((server != null))
            {
                isClosedAndClean &= server.Close(out error) >= TransportReturnCode.SUCCESS;
            }

            /*********************************************************
             * Server/Provider Application Life Cycle Major Step 8: Uninitialize ETA
             * Transport using Uninitialize The last ETA Transport function that an
             * application should call. This uninitialized internal data structures
             * and deletes any allocated memory.
             *********************************************************/
            /* All ETA Transport use is complete, must uninitialize.
             * The uninitialization process allows for any heap allocated memory to be cleaned up properly.
             */
            Transport.Uninitialize();

            if (isClosedAndClean)
            {
                Console.WriteLine("Provider application has closed channel and has cleaned up successfully.");
            }
            else
            {
                Console.WriteLine("Error ({0}) (errno: {1}): {2}",
                    error?.ErrorId, error?.SysError, error?.Text);
            }

            /* For applications that do not exit due to errors/exceptions such as:
             * Exits the application if the run-time has expired.
             */
            if (code == TransportReturnCode.SUCCESS)
            {
                Console.WriteLine("\nETA Interactive Provider Training Application successfully ended.");
            }

            /* End application */
            Environment.Exit(0);
        }

        #region Ping Management

        /*********************************************************
         * Initializes the ping times for etaChannel. etaChannel - The channel for
         * ping management info initialization
         *********************************************************/
        /* server ping timeout */
        static TimeSpan pingTimeoutServer;
        /* client ping timeout */
        static TimeSpan pingTimeoutClient;
        /* time client should receive next message/ping from server */
        static System.DateTime nextReceivePingTime;
        /* time to send next ping from client */
        static System.DateTime nextSendPingTime;
        /* flag for server message received */
        static bool receivedClientMsg;

        /// <summary>Inits the ping management.</summary>
        ///
        /// <param name="channel">the channel</param>
        public static void InitPingManagement(IChannel channel)
        {
            /* get current time */
            System.DateTime currentTime = System.DateTime.Now;

            /* set ping timeout for server and client */
            /* Applications are able to configure their desired pingTimeout values, where the ping timeout is
             * the point at which a connection can be terminated due to inactivity. Heartbeat messages are
             * typically sent every one-third of the pingTimeout, ensuring that heartbeats are exchanged prior
             * to a timeout occurring. This can be useful for detecting loss of connection prior to any kind
             * of network or operating system notification that may occur.
             */
            pingTimeoutServer = TimeSpan.FromSeconds(channel.PingTimeOut / 3);
            pingTimeoutClient = TimeSpan.FromSeconds(channel.PingTimeOut);

            /* set time to send next ping to remote connection */
            nextSendPingTime = currentTime + pingTimeoutServer;

            /* set time should receive next ping from remote connection */
            nextReceivePingTime = currentTime + pingTimeoutClient;

            receivedClientMsg = false;
        }


        /// <summary>
        /// Processing ping management handler etaChannel - The channel for ping
        /// management processing
        /// </summary>
        ///
        /// <param name="channel">the channel</param>
        /// <param name="selector">the selector</param>
        /// <returns>whether operation has been successful</returns>
        public static TransportReturnCode ProcessPingManagementHandler(IChannel channel)
        {
            /* Handles the ping processing for etaChannel. Sends a ping to the client if the next send ping
             * time has arrived and checks if a ping has been received from the client within the next receive
             * ping time.
             */
            TransportReturnCode retval = TransportReturnCode.SUCCESS;
            Error error = new();

            /* get current time */
            System.DateTime currentTime = System.DateTime.Now;

            /* handle server pings */
            if (currentTime >= nextSendPingTime)
            {
                /* send ping to client */
                /*********************************************************
                 * Server/Provider Application Life Cycle Major Step 5: Ping using
                 * Ping Attempts to write a heartbeat message on the connection.
                 * This function expects the Channel to be in the active state. If
                 * an application calls the Ping function while there are other
                 * bytes queued for output, the ETA Transport layer will suppress
                 * the heartbeat message and attempt to flush bytes to the network
                 * on the user's behalf.
                 *********************************************************/

                /* Ping use - this demonstrates sending of heartbeats */
                if ((retval = channel.Ping(out error)) > TransportReturnCode.SUCCESS)
                {
                    /* Indicates that queued data was sent as a heartbeat and there is still information
                     * internally queued by the transport.
                     *
                     * The Flush function must be called to continue attempting to pass the queued bytes to
                     * the connection. This information may still be queued because there is not sufficient
                     * space in the connections output buffer.  An I/O notification mechanism can be used to
                     * indicate when the socketId has write availability.
                     *
                     * There is still data left to flush, leave our write notification enabled so we get
                     * called again.  If everything wasn't flushed, it usually indicates that the TCP output
                     * buffer cannot accept more yet
                     */

                    /* flush needs to be done by application */
                }
                else
                {
                    switch (retval)
                    {
                        case TransportReturnCode.SUCCESS:
                            {
                                /* Ping message has been sent successfully */
                                Console.WriteLine("Ping message has been sent successfully to the client ...\n");
                            }
                            break;
                        case TransportReturnCode.FAILURE: /* fall through to default. */
                        default: /* Error handling */
                            {
                                Console.WriteLine("Error ({0}) (errno: {1}) encountered with Ping(). Error Text: {2}",
                                                  error.ErrorId, error.SysError, error.Text);
                                /* Closes channel/connection, cleans up and exits the application. */
                                return TransportReturnCode.FAILURE;
                            }
                    }
                }

                /* set time to send next ping from server */
                nextSendPingTime = currentTime + pingTimeoutServer;
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
                    nextReceivePingTime = currentTime + pingTimeoutClient;
                }
                else /* lost contact with server */
                {
                    /* Lost contact with remote (connection) */
                    error.Text = "Lost contact with client...";
                    Console.WriteLine("Error ({0}) (errno: {1}) {2}",
                                      error.ErrorId, error.SysError, error.Text);

                    /* Closes channel/connection, cleans up and exits the application. */
                    return TransportReturnCode.FAILURE;
                }
            }
            return retval;
        }
        #endregion
    }
}
