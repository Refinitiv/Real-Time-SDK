/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2023-2024 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
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
 *
 * In this module, after establishing a connection, ping messages might
 * need to be exchanged. The negotiated ping timeout is available via
 * the Channel. If ping heartbeats are not sent or received within
 * the expected time frame, the connection can be terminated. LSEG
 * recommends sending ping messages at intervals one-third the
 * size of the ping timeout.
 *
 * Detailed Descriptions:
 *
 * Once the connection is active, the consumer and provider applications
 * might need to exchange ping messages. A negotiated ping timeout is available
 * via Channel corresponding to each connection (this value might differ on
 * a per-connection basis). A connection can be terminated if ping heartbeats
 * are not sent or received within the expected time frame. LSEG
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
 *
 *****************************************************************************************
 * ETA Interactive Provider Training Module 1c: Reading and Writing Data
 *****************************************************************************************
 *
 * Summary:
 *
 * In this module, when a client or server Channel.State() is
 * ChannelState.ACTIVE, it is possible for an application to receive
 * data from the connection. Similarly, when a client or server
 * Channel.State() is ChannelState.ACTIVE, it is possible for an
 * application to write data to the connection. Writing involves a several
 * step process.
 *
 * Detailed Descriptions:
 *
 * When a client or server Channel.State() is ChannelState.ACTIVE, it is
 * possible for an application to receive data from the connection. The
 * arrival of this information is often announced by the I/O notification
 * mechanism that the Channel.scktChannel() is registered with. The ETA
 * Transport reads information from the network as a byte stream, after
 * which it determines buffer boundaries and returns each buffer one by
 * one.
 *
 * When a client or server Channel.State() is ChannelState.ACTIVE, it is
 * possible for an application to write data to the connection. Writing
 * involves a several step process. Because the ETA Transport provides
 * efficient buffer management, the user is required to obtain a buffer
 * from the ETA Transport buffer pool. This can be the guaranteed output
 * buffer pool associated with a Channel. After a buffer is acquired,
 * the user can populate the Buffer.data and set the Buffer.Length
 * to the number of bytes referred to by data. If queued information cannot
 * be passed to the network, a function is provided to allow the application
 * to continue attempts to flush data to the connection. An I/O notification
 * mechanism can be used to help with determining when the network is able
 * to accept additional bytes for writing. The ETA Transport can continue to
 * queue data, even if the network is unable to write.
 *
 * Command line usage:
 *
 * ProvMod1c.exe
 * (runs with a default set of parameters (-p 14002 -r 300))
 *
 * or
 *
 * ProvMod1c.exe [-p <SrvrPortNo>] [-r <Running Time>]
 * (runs with specified set of parameters, all parameters are optional)
 *
 * Pressing the CTRL+C buttons terminates the program.
 *
 *****************************************************************************************
 * ETA Interactive Provider Training Module 2: Perform/Handle Login Process
 *****************************************************************************************
 * Summary:
 *
 * Applications authenticate with one another using the Login domain model.
 * An OMM Interactive Provider must handle the consumer's Login request messages
 * and supply appropriate responses.
 *
 * In this module, after receiving a Login request, the Interactive Provider
 * can perform any necessary authentication and permissioning.
 *
 * Detailed Descriptions:
 *
 * After receiving a Login request, the Interactive Provider can perform any
 * necessary authentication and permissioning.
 *
 * a) If the Interactive Provider grants access, it should send an RefreshMsg to
 *    convey that the user successfully connected. This message should indicate the
 *    feature set supported by the provider application.
 *
 * b) If the Interactive Provider denies access, it should send an StatusMsg, closing
 *    the connection and informing the user of the reason for denial.
 *
 * The login handler for this simple Interactive Provider application only allows one
 * login stream per channel. It provides functions for processing login requests from
 * consumers and sending back the responses. Functions for sending login request
 * reject/close status messages, initializing the login handler, and closing login
 * streams are also provided.
 *
 * Also please note for simple training app, the interactive provider only supports
 * one client session from the consumer, that is, only supports one channel/client
 * connection.
 *
 * Content is encoded and decoded using the ETA Message Package and the ETA Data
 * Package.
 *
 * Command line usage:
 *
 * ProvMod2.exe
 * (runs with a default set of parameters (-p 14002 -r 300))
 *
 * or
 *
 * ProvMod2.exe [-p <SrvrPortNo>] [-r <Running Time>]
 * (runs with specified set of parameters, all parameters are optional)
 *
 * Pressing the CTRL+C buttons terminates the program.
 *****************************************************************************************/

using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Net;
using System.Net.Sockets;

using LSEG.Eta.Common;

using LSEG.Eta.Codec;
using LSEG.Eta.Rdm;
using LSEG.Eta.Transports;


namespace LSEG.Eta.Training.Provider
{
    ///<summary>
    /// The Enum LoginRejectReason.
    /// </summary>
    public enum LoginRejectReason
    {
        MAX_LOGIN_REQUESTS_REACHED, NO_USER_NAME_IN_REQUEST
    }

    /// <summary>The Class Module_2_Login.</summary>
    public class Module_2_Login
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

        static bool loginRequestInfo_IsInUse;
        static int loginRequestInfo_StreamId;
        static string loginRequestInfo_Username = string.Empty;
        static string loginRequestInfo_ApplicationId = string.Empty;
        static string loginRequestInfo_ApplicationName = string.Empty;
        static string loginRequestInfo_Position = string.Empty;
        static string loginRequestInfo_Password = string.Empty;
        static string loginRequestInfo_InstanceId = string.Empty;
        static string loginRequestInfo_Role = string.Empty;

        static long etaServerFDValue;
        static long clientChannelFDValue;
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

            /* Create decode iterator to decode the contents of the buffer */
            DecodeIterator decodeIter = new();

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
            /* Bind ETA server */
            if ((server = Transport.Bind(bindOpts, out error)) == null)
            {
                Console.WriteLine("Error ({0}) (errno: {1}) encountered with Bind. Error Text: {2}",
                                  error.ErrorId, error.SysError, error.Text);
                /* End application, uninitialize to clean up first */
                Transport.Uninitialize();
                Environment.Exit((int)TransportReturnCode.FAILURE);
            }

            clientChannelFDValue = -1;
            etaServerFDValue = server.Socket.Handle.ToInt64();
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
            /* Clears the login request information */
            ClearLoginRequestInfo();
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
                    if (readSocketList.Count > 0 || writeSocketList.Count > 0)
                    {
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
                                msgBuf = channel.Read(readArgs, out error);

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
                                    Msg msg = new();

                                    /* This ClearDecodeIterator clear iterator function should be used to achieve the best performance while clearing the iterator. */
                                    /* Clears members necessary for decoding and readies the iterator for reuse. You must clear DecodeIterator
                                     * before decoding content. For performance purposes, only those members required for proper functionality are cleared.
                                     */
                                    decodeIter.Clear();
                                    /* Set the RWF version to decode with this iterator */
                                    CodecReturnCode ret = decodeIter.SetBufferAndRWFVersion(msgBuf, channel.MajorVersion, channel.MinorVersion);

                                    /* Associates the DecodeIterator with the Buffer from which to decode. */
                                    if ((ret) != CodecReturnCode.SUCCESS)
                                    {
                                        Console.WriteLine("\nSetDecodeIteratorBuffer() failed with return code: {0}", ret);
                                        /* Closes channel, closes server, cleans up and exits the application. */
                                        CloseChannelServerCleanUpAndExit(channel, server, TransportReturnCode.FAILURE);
                                    }
                                    /******************************************
                                     * Step 6) Decode buffer message *
                                     ******************************************/
                                    /* decode contents into the Msg structure */
                                    if ((ret = msg.Decode(decodeIter)) != CodecReturnCode.SUCCESS)
                                    {
                                        Console.WriteLine("Error ({0}) (errno: {1}) encountered with Init Channel fd={2}. Error Text: {3}",
                                            error.ErrorId, error.SysError, clientChannelFDValue, error.Text);
                                        /* Closes channel, closes server, cleans up and exits the application. */
                                        CloseChannelServerCleanUpAndExit(channel, server, TransportReturnCode.FAILURE);
                                    }

                                    /* Deduce an action based on the domain type of the message */
                                    switch (msg.DomainType)
                                    {
                                        /* (1) Login Message */
                                        case (int)DomainType.LOGIN:
                                            {
                                                try
                                                {
                                                    if ((retCode = ProcessLoginRequest(channel, msg, decodeIter)) > TransportReturnCode.SUCCESS)
                                                    {
                                                        /* There is still data left to flush, leave our write notification enabled so we get called again.
                                                         * If everything wasn't flushed, it usually indicates that the TCP output buffer cannot accept more yet
                                                         */

                                                        /* set write flag if there's still other data queued */
                                                        /* flush is done by application */
                                                        opWrite = true;
                                                    }
                                                    else if (retCode < TransportReturnCode.SUCCESS)
                                                    {
                                                        ClearLoginRequestInfo();
                                                        CloseChannelServerCleanUpAndExit(channel, server, TransportReturnCode.FAILURE);
                                                    }
                                                }
                                                catch (SocketException e)
                                                {
                                                    Console.WriteLine($"Error: {e.Message}");
                                                }
                                            }
                                            break;
                                        default: /* Error handling */
                                            {
                                                Console.WriteLine("Unhandled Domain Type: {0}", msg.DomainType);
                                            }
                                            break;
                                    }

                                    /* Acknowledge that a ping has been received */
                                    /* set flag for server message received */
                                    receivedClientMsg = true;
                                    Console.Write("Ping message has been received successfully from the client due to data message ...\n\n");
                                }
                                else
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
                                            Console.WriteLine("Error ({0}) (errno: {1}) channelInactive Error Text: {2}",
                                                              error.ErrorId, error.SysError, error.Text);
                                            CloseChannelServerCleanUpAndExit(channel, server, TransportReturnCode.FAILURE);
                                            break;

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
                            }
                        }

                        if (writeSocketList.Count > 0)
                        {
                            retCode = TransportReturnCode.FAILURE;

                            if ((retCode = channel.Flush(out error)) > TransportReturnCode.SUCCESS)
                            {
                                /* There is still data left to flush, leave our write notification enabled so we get called again.
                                 * If everything wasn't flushed, it usually indicates that the TCP output buffer cannot accept more yet
                                 */
                            }
                            else
                            {
                                switch (retCode)
                                {
                                    case TransportReturnCode.SUCCESS:
                                        {
                                            /* Everything has been flushed, no data is left to send - unset/clear write fd notification */
                                            opWrite = false;
                                            writeSocketList.Clear();
                                        }
                                        break;
                                    case TransportReturnCode.FAILURE:
                                    default: /* Error handling */
                                        {
                                            Console.WriteLine("Error ({0}) (errno: {1}) encountered with Init Channel fd={2}. Error Text: {3}",
                                                              error.ErrorId, error.SysError, clientChannelFDValue, error.Text);
                                            /* Connection should be closed, return failure */
                                            /* Closes channel/connection, cleans up and exits the application. */
                                            CloseChannelServerCleanUpAndExit(channel, server, TransportReturnCode.FAILURE);
                                        }
                                        break;
                                }
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
                        Console.Write("ETA Server run-time has expired...\n\n");
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
             * Transport using Uninitialize.
             *
             * The last ETA Transport function that an application should call. This
             * uninitialized internal data structures and deletes any allocated memory.
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
                    error ??= new Error();
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

        /// <summary>Sends a message buffer to the channel.</summary>
        ///
        /// <param name="channel">the Channel to send the message buffer to</param>
        /// <param name="msgBuf">the buffer to be sent</param>
        /// <returns>status code</returns>
        public static TransportReturnCode SendMessage(IChannel channel, ITransportBuffer msgBuf)
        {
            TransportReturnCode retCode = 0;
            Error error = new();

            WriteArgs writeArgs = new();
            writeArgs.Flags = WriteFlags.NO_FLAGS;

            /* send the request */

            /*********************************************************
             * Server/Provider Application Life cycle Major Step 5: Write using Writer.
             *
             * Writer performs any writing or queuing of data. This function expects the
             * Channel to be in the active state and the buffer to be properly populated,
             * where length reflects the actual number of bytes used. This function allows
             * for several modifications to be specified for this call. Here we use
             * WriteFlags.NO_FLAGS.
             *
             * For more information on other flag enumeration such as
             * WriteFlags.DO_NOT_COMPRESS or WriteFlags.DIRECT_SOCKET_WRITE, see the ETA C
             * developers guide for Write Flag Enumeration Values supported by ETA
             * Transport.
             *
             * The ETA Transport also supports writing data at different priority
             * levels. The application can pass in two integer values used for reporting
             * information about the number of bytes that will be written.
             *
             * The uncompressedBytesWritten parameter will return the number of bytes to
             * be written, including any transport header overhead but not taking into
             * account any compression.
             *
             * The bytesWritten parameter will return the number of bytes to be written,
             * including any transport header overhead and taking into account any
             * compression. If compression is disabled, uncompressedBytesWritten and
             * bytesWritten should match.
             *
             * The number of bytes saved through the compression process can be calculated
             * by (bytesWritten - uncompressedBytesWritten).
             *
             * Note: Before passing a buffer to Write, it is required that the application
             * set length to the number of bytes actually used. This ensures that only the
             * required bytes are written to the network.
             *********************************************************/

            /* Now write the data - keep track of ETA Transport return code - Because
             * positive values indicate bytes left to write, some negative transport layer
             * return codes still indicate success
             */
            if ((retCode = channel.Write(msgBuf, writeArgs, out error)) == TransportReturnCode.WRITE_CALL_AGAIN)
            {
                /* (-10) Transport Success: Write is fragmenting the buffer and needs to be called again with
                 * the same buffer. This indicates that Write was unable to send all fragments with the
                 * current call and must continue fragmenting
                 */

                /* Large buffer is being split by transport, but out of output buffers. Schedule a call to
                 * Flush and then call the Write function again with this same exact buffer to continue the
                 * fragmentation process. Only release the buffer if not passing it to Write again. */

                /* call Flush and Write again - breaking out if the return code is something other than
                 * TransportReturnCode.WRITE_CALL_AGAIN (write call again) */
                while (retCode == TransportReturnCode.WRITE_CALL_AGAIN)
                {
                    if ((retCode = channel.Flush(out error)) < TransportReturnCode.SUCCESS)
                    {
                        Console.WriteLine("Flush has failed with return code {0} - <{1}>",
                            retCode, error.Text);
                    }
                    retCode = channel.Write(msgBuf, writeArgs, out error);
                }

            }

            if (retCode > TransportReturnCode.SUCCESS)
            {
                /* The write was successful and there is more data queued in ETA Transport. The
                 * Channel.flush() method should be used to continue attempting to flush data to the
                 * connection. ETA will release buffer.
                 */


                /* Flush needs to be done by application */

                if ((retCode = channel.Flush(out error)) >= TransportReturnCode.SUCCESS)
                {
                    /* retval == TransportReturnCode.SUCCESS - All was flushed.
                    /* retval == TransportReturnCode.SUCCESS - There is still data left to flush.
                    *
                    * If everything wasn't flushed, it usually indicates that the TCP output buffer cannot
                    * accept more yet
                    */
                }
                else
                {
                    Console.Write("Channel failed to flush \n");
                    return TransportReturnCode.FAILURE;
                }
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
                                /* Channel is Closed - This is terminal. Treat as error, and buffer must be
                                 * released - fall through to default. */
                            }
                            else
                            {
                                /* Channel.Write() internally attempted to flush data to the connection but
                                 * was blocked. This is not a failure and the user should not release their
                                 * buffer. */

                                /* Successful write call, data is queued. The Channel.Flush() method should be
                                 * used to continue attempting to flush data to the connection. */

                                /* Set write flag if flush failed */
                                /* Flush needs to be done by application */

                                /* Channel is still open, but Channel.write() tried to flush internally and failed.
                                 * Return positive value so the caller knows there's bytes to flush.
                                 */

                                if ((retCode = channel.Flush(out error)) >= TransportReturnCode.SUCCESS)
                                {
                                    /* retval == TransportReturnCode.SUCCESS - All was flushed.
                                    /* retval == TransportReturnCode.SUCCESS - There is still data left to flush.
                                    *
                                    * If everything wasn't flushed, it usually indicates that the TCP output
                                    * buffer cannot accept more yet
                                    */
                                }
                                else
                                {
                                    Console.Write("Channel failed to flush \n");
                                    return TransportReturnCode.FAILURE;
                                }

                                return TransportReturnCode.SUCCESS + 1;
                            }
                            break;
                        }
                    case TransportReturnCode.FAILURE:
                    default:
                        {
                            Console.WriteLine("Error ({0}) (errno: {1}) encountered with Init Channel fd={2}. Error Text: {3}",
                                error.ErrorId, error.SysError, clientChannelFDValue, error.Text);
                            channel.ReleaseBuffer(msgBuf, out error);
                            return TransportReturnCode.FAILURE;
                        }
                }
            }

            return retCode;
        }

        #region Login Request

        /// <summary>Processes a login request.</summary>
        ///
        /// <param name="channel">the Channel of connection</param>
        /// <param name="msg">the partially decoded message</param>
        /// <param name="decIter">the decode iterator</param>
        /// <param name="error">tracks error info</param>
        /// <returns>status code</returns>
        public static TransportReturnCode ProcessLoginRequest(IChannel channel, Msg msg, DecodeIterator decIter)
        {
            IMsgKey? requestKey = null;

            TransportReturnCode retCode;

            ElementList elementList = new();
            ElementEntry element = new();

            /* Switch cases depending on message class */
            switch (msg.MsgClass)
            {
                case MsgClasses.REQUEST:
                    {
                        /* get request message key - retrieve the MsgKey structure from the provided decoded message structure */
                        requestKey = msg.MsgKey;

                        /* check if key has user name */
                        /* user name is only login user type accepted by this application (user name is the default type) */
                        if (((requestKey.Flags & MsgKeyFlags.HAS_NAME) == 0)
                            || (((requestKey.Flags & MsgKeyFlags.HAS_NAME_TYPE) != 0)
                                && (requestKey.NameType != (int)Login.UserIdTypes.NAME)))
                        {
                            if ((retCode = SendLoginRequestRejectStatusMsg(channel, msg.StreamId, LoginRejectReason.NO_USER_NAME_IN_REQUEST)) != TransportReturnCode.SUCCESS)
                            {
                                return TransportReturnCode.FAILURE;
                            }
                            break;
                        }

                        if (loginRequestInfo_IsInUse && (loginRequestInfo_StreamId != msg.StreamId))
                        {
                            if ((retCode = SendLoginRequestRejectStatusMsg(channel, msg.StreamId, LoginRejectReason.MAX_LOGIN_REQUESTS_REACHED)) != TransportReturnCode.SUCCESS)
                            {
                                return TransportReturnCode.FAILURE;
                            }
                            break;
                        }

                        /* decode login request */

                        /* get StreamId */
                        loginRequestInfo_StreamId = msg.StreamId;
                        loginRequestInfo_IsInUse = true;

                        /* get Username */
                        if (requestKey.Name.Data() == null)
                        {
                            loginRequestInfo_Username = string.Empty;
                        }
                        else
                        {
                            loginRequestInfo_Username = requestKey.Name.ToString();
                        }

                        /* decode key opaque data */
                        CodecReturnCode codecCode;

                        /*
                         * Allows the user to continue decoding of any message key attributes with the same
                         * DecodeIterator used when calling DecodeMsg
                         *
                         * Typical use:
                         *
                         * 1. Call DecodeMsg()
                         *
                         * 2. If there are any message key attributes and the application wishes to decode
                         *    them using the same DecodeIterator, call DecodeKeyAttrib() and continue decoding
                         *    using the appropriate container type decode functions, as indicated by
                         *    MsgKey.AttribContainerType
                         *
                         * 3. If payload is present and the application wishes to decode it, use the
                         *    appropriate decode functions, as specified in MsgBase.DomainType
                         */
                        if ((codecCode = msg.DecodeKeyAttrib(decIter, requestKey)) != CodecReturnCode.SUCCESS)
                        {
                            return TransportReturnCode.FAILURE;
                        }

                        /* decode element list */

                        /*
                         * Decodes an ElementList container
                         *
                         * Typical use:
                         * 1. Call elementList.Decode()
                         *
                         * 2. Call ElementEntry.Decode until error or END_OF_CONTAINER is returned.
                         */
                        if ((codecCode = elementList.Decode(decIter, null)) == CodecReturnCode.SUCCESS)
                        {
                            /* decode each element entry in list */
                            while ((codecCode = element.Decode(decIter)) != CodecReturnCode.END_OF_CONTAINER)
                            {
                                /* get login request information */
                                if (codecCode == CodecReturnCode.SUCCESS)
                                {
                                    /* ApplicationId */
                                    if (element.Name.Equals(ElementNames.APPID))
                                    {
                                        loginRequestInfo_ApplicationId = element.EncodedData.ToString();
                                    }
                                    /* ApplicationName */
                                    else if (element.Name.Equals(ElementNames.APPNAME))
                                    {
                                        loginRequestInfo_ApplicationName = element.EncodedData.ToString();
                                    }
                                    /* Position */
                                    else if (element.Name.Equals(ElementNames.POSITION))
                                    {
                                        loginRequestInfo_Position = element.EncodedData.ToString();
                                    }
                                    /* Password */
                                    else if (element.Name.Equals(ElementNames.PASSWORD))
                                    {
                                        loginRequestInfo_Password = element.EncodedData.ToString();
                                    }
                                    /* InstanceId */
                                    else if (element.Name.Equals(ElementNames.INST_ID))
                                    {
                                        loginRequestInfo_InstanceId = element.EncodedData.ToString();
                                    }
                                    /* Role */
                                    else if (element.Name.Equals(ElementNames.ROLE))
                                    {
                                        loginRequestInfo_Role = element.EncodedData.ToString();
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

                        Console.WriteLine("Received Login Request for Username: {0}", loginRequestInfo_Username);
                        /* send login response */
                        if ((retCode = SendLoginResponse(channel)) != TransportReturnCode.SUCCESS)
                        {
                            return retCode;
                        }
                    }
                    break;

                case MsgClasses.CLOSE:
                    {
                        Console.WriteLine("Received Login Close for StreamId {0}", msg.StreamId);
                        /* close login stream */
                        CloseLoginStream(msg.StreamId);
                    }
                    break;

                default:
                    {
                        Console.WriteLine("Received Unhandled Login Msg Class: {0}", msg.MsgClass);
                        return TransportReturnCode.FAILURE;
                    }
            }

            return TransportReturnCode.SUCCESS;
        }

        /// <summary>Send login response.</summary>
        ///
        /// <param name="channel">the channel</param>
        /// <returns>status code</returns>
        public static TransportReturnCode SendLoginResponse(IChannel channel)
        {
            CodecReturnCode retCode = CodecReturnCode.SUCCESS;
            ITransportBuffer? msgBuf;
            Error error = new();
            /* Populate and encode a refreshMsg */
            IRefreshMsg refreshMsg = (IRefreshMsg)new Msg();

            /* ETA provides clear functions for its structures as well as static initializers.
             *
             * These functions are tuned to be efficient and avoid initializing unnecessary
             * structure members, and allow for optimal structure use and reuse. In general,
             * LSEG recommends that you use the clear functions over static initializers,
             * because the clear functions are more efficient.
             */
            /* Iterator used for encoding throughout the application - we can clear it and reuse it instead of recreating it */
            EncodeIterator encIter = new();

            ElementList elementList = new();
            ElementEntry elementEntry = new();

            Codec.Buffer applicationId = new();
            Codec.Buffer applicationName = new();
            Codec.Buffer applicationPosition = new();
            UInt supportBatchRequests = new();

            elementList.Clear();
            elementEntry.Clear();

            /* Create channel info as a holder */
            ChannelInfo channelInfo = new();

            /* Populate information from channel */
            if (channel.Info(channelInfo, out error) != TransportReturnCode.SUCCESS)
            {
                return TransportReturnCode.FAILURE;
            }

            /* Get a buffer of the channel max fragment size */

            /* Obtains a non-packable buffer of the requested size from the ETA Transport guaranteed
             * buffer pool to write into for the Login response.
             *
             * When the Buffer is returned, the length member indicates the number of bytes
             * available in the buffer (this should match the amount the application requested).
             *
             * When populating, it is required that the application set length to the number of
             * bytes actually used.  This ensures that only the required bytes are written to the
             * network.
             */

            /* etaGetBuffer() is the utility function that does 2-pass (more robust) getting non-packable buffer. */
            if ((msgBuf = EtaGetBuffer(channel, channelInfo.MaxFragmentSize, out error)) == null)
            {
                /* Connection should be closed, return failure */
                /* Closes channel, closes server, cleans up and exits the application. */
                return TransportReturnCode.FAILURE;
            }

            /* clear encode iterator for reuse - this should be used to achieve the best performance
             * while clearing the iterator. */
            encIter.Clear();

            /* set version information of the connection on the encode iterator so proper versioning can be performed */
            /* set the buffer on an EncodeIterator */
            retCode = encIter.SetBufferAndRWFVersion(msgBuf, channel.MajorVersion, channel.MinorVersion);
            if (retCode < CodecReturnCode.SUCCESS)
            {
                channel.ReleaseBuffer(msgBuf, out error);
                Console.WriteLine("\nSetEncodeIteratorBuffer() failed with return code: {0}", retCode);
                /* Closes channel, closes server, cleans up and exits the application. */
                return TransportReturnCode.FAILURE;
            }

            /* initialize refresh message */

            /* On larger structures, like messages, the clear functions tend to outperform the
             * static initializer. It is recommended to use the clear function when initializing any
             * messages.
             */
            refreshMsg.Clear();

            /* provide login refresh response information */

            /* set refresh flags */

            /* set-up message */
            refreshMsg.MsgClass = MsgClasses.REFRESH;/* (2) Refresh Message */
            refreshMsg.DomainType = (int)DomainType.LOGIN;/* (1) Login Message */
            refreshMsg.ContainerType = DataTypes.NO_DATA;/* (128) No Data <BR>*/

            /* (0x0008) The RefreshMsg has a message key, contained in RefreshMsg.MsgBase.MsgKey. */
            /* (0x0020) Indicates that this RefreshMsg is a solicited response to a consumer's request. */
            /* (0x0040) Indicates that this is the final part of a refresh. This flag should be set
             *          on both single-part response messages, as well as the final message in a multi-part
             *          response message sequence. */
            /* (0x0100) Indicates that any cached header or payload information associated with the
             *          RefreshMsg's item stream should be cleared. */
            refreshMsg.Flags = RefreshMsgFlags.HAS_MSG_KEY
                | RefreshMsgFlags.SOLICITED
                | RefreshMsgFlags.REFRESH_COMPLETE
                | RefreshMsgFlags.CLEAR_CACHE;
            /* (1) Stream is open (typically implies that information will be streaming, as
             *     information changes updated information will be sent on the stream, after final
             *     RefreshMsg or StatusMsg) */
            refreshMsg.State.StreamState(StreamStates.OPEN);
            refreshMsg.State.DataState(DataStates.OK);
            refreshMsg.State.Code(StateCodes.NONE);

            string stateText = "Login accepted by host ";
            string hostName = Dns.GetHostName();

            if (!string.IsNullOrEmpty(hostName))
            {
                hostName = "localhost";
            }

            stateText += hostName;

            refreshMsg.State.Text().Data(stateText);
            /* provide login response information */

            /* StreamId - just set the Login response stream id info to be the same as the Login
             * request stream id info */
            refreshMsg.StreamId = loginRequestInfo_StreamId;
            /* set msgKey members */
            /* (0x0020) This MsgKey has additional attribute information, contained in
             *          MsgKey.EncodedAttrib. The container type of the attribute information is
             *          contained in MsgKey.AttribContainerType. */
            /* (0x0004) This MsgKey has a nameType enumeration, contained in MsgKey.NameType. */
            /* (0x0002) This MsgKey has a name buffer, contained in MsgKey.Name.  */
            refreshMsg.MsgKey.Flags = MsgKeyFlags.HAS_ATTRIB | MsgKeyFlags.HAS_NAME_TYPE | MsgKeyFlags.HAS_NAME;

            /* Username */
            refreshMsg.MsgKey.Name.Data(loginRequestInfo_Username);
            refreshMsg.MsgKey.NameType = (int)Login.UserIdTypes.NAME;
            refreshMsg.MsgKey.AttribContainerType = DataTypes.ELEMENT_LIST;

            /* encode message */

            /* since our msgKey has opaque that we want to encode, we need to use refreshMsg.encode */

            /* refreshMsg.encode should return and inform us to encode our key opaque */

            /**
             * Begin encoding process for an Msg.
             *
             * Begins encoding of an Msg
             * Typical use:
             *
             * 1. Populate desired members on the Msg
             *
             * 2. Call refreshMsg.EncodeInit() to begin message encoding
             *
             * 3. If the Msg requires any message key attributes, but they are not pre-encoded and
             *    populated on the MsgKey.EncodedAttrib, the refreshMsg.EncodeInit() function will
             *    return ENCODE_MSG_KEY_OPAQUE. Call appropriate encode functions, as indicated by
             *    MsgKey.AttribContainerType. When attribute encoding is completed, followed with
             *    refreshMsg.EncodeMsgKeyAttribComplete() to continue with message encoding
             *
             * 4. If the Msg requires any extended header information, but it is not pre-encoded and
             *    populated in the extendedHeader Buffer, the refreshMsg.EncodeInit() (or when also
             *    encoding attributes, the EncodeMsgKeyAttribComplete()) function will return
             *    ENCODE_EXTENDED_HEADER. Call any necessary extended header encoding functions;
             *    when completed call EncodeExtendedHeaderComplete() to continue with message
             *    encoding
             *
             * 5. If the Msg requires any payload, but it is not pre-encoded and populated in the
             *    EncodedDataBody, the refreshMsg.EncodeInit() (or when encoding message key
             *    attributes or extended header, refreshMsg.EncodeKeyAttribComplete() or
             *    refreshMsg.EncodeExtendedHeaderComplete()) function will return
             *    ENCODE_CONTAINER. Call appropriate payload encode functions, as indicated by
             *    ContainerType. If no payload is required or it is provided as pre-encoded, this
             *    function will return SUCCESS
             *
             * 6. Call EncodeMsgComplete() when all content is completed
             */
            if ((retCode = refreshMsg.EncodeInit(encIter, 0)) < CodecReturnCode.SUCCESS)
            {
                channel.ReleaseBuffer(msgBuf, out error);
                Console.WriteLine("\nEncodeMsgInit failed with return code: {0}", retCode);
                /* Closes channel, closes server, cleans up and exits the application. */
                return TransportReturnCode.FAILURE;
            }
            /* encode our msgKey opaque */

            /* encode the element list */
            /* (0x08) The ElementList contains standard encoded content (e.g. not set defined). */
            elementList.Flags = ElementListFlags.HAS_STANDARD_DATA;

            /**
             * Begin encoding process for ElementList container type.
             *
             * Begins encoding of an ElementList
             * Typical use:
             *
             * 1. Call elementList.EncodeInit()
             *
             * 2. To encode entries, call elementEntry.Encode()
             * orelementEntry.EncodeInit()..elementEntry.EncodeComplete() for
             * each elementEntry
             *
             * 3. Call elementList.EncodeComplete() when all entries are
             * completed
             */
            if ((retCode = elementList.EncodeInit(encIter, null, 0)) < CodecReturnCode.SUCCESS)
            {
                channel.ReleaseBuffer(msgBuf, out error);
                Console.WriteLine("\nEncodeElementListInit failed with return code: {0}", retCode);
                /* Closes channel, closes server, cleans up and exits the application. */
                return TransportReturnCode.FAILURE;
            }

            /* ApplicationId */
            applicationId.Data("256");
            elementEntry.DataType = DataTypes.ASCII_STRING;
            elementEntry.Name = ElementNames.APPID;

            if ((retCode = elementEntry.Encode(encIter, applicationId)) < CodecReturnCode.SUCCESS)
            {
                channel.ReleaseBuffer(msgBuf, out error);
                Console.WriteLine("\nEncodeElementEntry failed with return code: {0}", retCode);
                /* Closes channel, closes server, cleans up and exits the application. */
                return TransportReturnCode.FAILURE;
            }

            /* ApplicationName */
            applicationName.Data("ETA Provider Training");
            elementEntry.DataType = DataTypes.ASCII_STRING;
            elementEntry.Name = ElementNames.APPNAME;

            if ((retCode = elementEntry.Encode(encIter, applicationName)) < CodecReturnCode.SUCCESS)
            {
                channel.ReleaseBuffer(msgBuf, out error);
                Console.WriteLine("\nEncodeElementEntry failed with return code: {0}", retCode);
                /* Closes channel, closes server, cleans up and exits the application. */
                return TransportReturnCode.FAILURE;
            }

            /* Position - just set the Login response position info to be the same as the Login request position info */
            applicationPosition.Data(loginRequestInfo_Position);
            elementEntry.DataType = DataTypes.ASCII_STRING;
            elementEntry.Name = ElementNames.POSITION;

            if ((retCode = elementEntry.Encode(encIter, applicationPosition)) < CodecReturnCode.SUCCESS)
            {
                channel.ReleaseBuffer(msgBuf, out error);
                Console.WriteLine("\nEncodeElementEntry failed with return code: {0}", retCode);
                /* Closes channel, closes server, cleans up and exits the application. */
                return TransportReturnCode.FAILURE;
            }

            /* SupportBatchRequests */
            elementEntry.DataType = DataTypes.UINT;
            elementEntry.Name = ElementNames.SUPPORT_BATCH;

            if ((retCode = elementEntry.Encode(encIter, supportBatchRequests)) < CodecReturnCode.SUCCESS)
            {
                channel.ReleaseBuffer(msgBuf, out error);
                Console.WriteLine("\nEncodeElementEntry failed with return code: {0}", retCode);
                /* Closes channel, closes server, cleans up and exits the application. */
                return TransportReturnCode.FAILURE;
            }

            /* complete encode element list - Completes encoding of an ElementList */
            if ((retCode = elementList.EncodeComplete(encIter, true)) < CodecReturnCode.SUCCESS)
            {
                channel.ReleaseBuffer(msgBuf, out error);
                Console.WriteLine("\nEncodeElementListComplete failed with return code: {0}", retCode);
                /* Closes channel, closes server, cleans up and exits the application. */
                return TransportReturnCode.FAILURE;
            }
            /* complete encode key */

            /* EncodeMsgKeyAttribComplete finishes our key opaque, so it should return and indicate
             * for us to encode our container/msg payload */
            if ((retCode = refreshMsg.EncodeKeyAttribComplete(encIter, true)) < CodecReturnCode.SUCCESS)
            {
                channel.ReleaseBuffer(msgBuf, out error);
                Console.WriteLine("\nencodeKeyAttribComplete failed with return code: {0}", retCode);
                /* Closes channel, closes server, cleans up and exits the application. */
                return TransportReturnCode.FAILURE;
            }

            /* complete encode message */
            if ((retCode = refreshMsg.EncodeComplete(encIter, true)) < CodecReturnCode.SUCCESS)
            {
                channel.ReleaseBuffer(msgBuf, out error);
                Console.WriteLine("\nencodeComplete failed with return code: {0}", retCode);
                /* Closes channel, closes server, cleans up and exits the application. */
                return TransportReturnCode.FAILURE;
            }

            /* send login response */
            if ((SendMessage(channel, msgBuf)) < TransportReturnCode.SUCCESS)
            {
                /* Closes channel, closes server, cleans up and exits the application. */
                return TransportReturnCode.FAILURE;
            }
            else
            {
                /* There is still data left to flush, leave our write notification enabled so we get called again.
                 * If everything wasn't flushed, it usually indicates that the TCP output buffer cannot accept more yet
                 */

                /* set write flags if there's still other data queued */
                /* flush needs to be done by application */
            }

            return TransportReturnCode.SUCCESS;
        }

        /// <summary>Sends the login request reject status message for a channel.</summary>
        ///
        /// <param name="streamId">the stream id to close the login for</param>
        public static void CloseLoginStream(int streamId)
        {
            /* find original request information associated with streamId */
            if (loginRequestInfo_StreamId == streamId)
            {
                Console.WriteLine("Closing login stream id {0} with user name: {1}",
                    loginRequestInfo_StreamId, loginRequestInfo_Username);
                /* Clears the original login request information */
                ClearLoginRequestInfo();
            }
        }

        /// <summary>Performs two time pass to obtain buffer.</summary>
        ///
        /// <param name="channel">the Channel of connection</param>
        /// <param name="size">size of requested buffer</param>
        /// <param name="error">tracks error info</param>
        /// <returns>obtained buffer</returns>
        public static ITransportBuffer? EtaGetBuffer(IChannel channel, int size, out Error error)
        {
            TransportReturnCode retCode;
            ITransportBuffer msgBuf;

            /* First check error */
            if ((msgBuf = channel.GetBuffer(size, false, out error)) == null)
            {
                /* Check to see if this is just out of buffers or if it's unrecoverable */
                if (error.ErrorId != TransportReturnCode.NO_BUFFERS)
                {
                    /* Connection should be closed, return failure */
                    /* Closes channel, closes server, cleans up and exits the application. */
                    Console.WriteLine("Error ({0}) (errno: {1}) encountered with Channel.GetBuffer. Error Text: {2}",
                        error.ErrorId, error.SysError, error.Text);
                    return null;
                }

                /* (-4) Transport Failure: There are no buffers available from the buffer pool, returned from GetBuffer.
                 *
                 * This can happen if the reader isn't keeping up and/or we have a lot of write threads in multithreaded apps.
                 * Use Ioctl to increase pool size or use Flush to flush data and return buffers to pool.
                 */

                /* Flush and obtain buffer again */
                retCode = channel.Flush(out error);
                if (retCode < TransportReturnCode.SUCCESS)
                {
                    Console.WriteLine("Channel.flush() failed with return code {0} - <{1}>",
                        retCode, error.Text);
                    return null;
                }

                /* call GetBuffer again to see if it works now after Flush */
                if ((msgBuf = channel.GetBuffer(size, false, out error)) == null)
                {
                    Console.WriteLine("Error ({0}) (errno: {1}) encountered with Channel.getBuffer. Error Text: {2}",
                        error.ErrorId, error.SysError, error.Text);
                    return null;
                }
            }

            /* return  buffer to be filled in with valid memory */
            return msgBuf;
        }

        /// <summary>Sends the login request reject status message for a channel.</summary>
        ///
        /// <param name="channel">the Channel of connection</param>
        /// <param name="streamId">the stream id of the login request reject status</param>
        /// <param name="reason">the reason for the reject</param>
        /// <returns>status code</returns>
        public static TransportReturnCode SendLoginRequestRejectStatusMsg(IChannel channel, int streamId, LoginRejectReason reason)
        {
            TransportReturnCode retCode;
            ITransportBuffer? msgBuf;
            Error error = new();

            /* ETA provides clear functions for its structuresas well as static
             * initializers. These functions are tuned to be efficient and avoid
             * initializing unnecessary structure members, and allow for optimal structure
             * use and reuse.
             *
             * In general, LSEG recommends that you use the Clear() functions over
             * constructing new instances, because the clear functions are more efficient.
             */
            /* Iterator used for encoding throughout the application - we can clear it and
             * reuse it instead of recreating it */
            EncodeIterator encIter = new();

            /* Create channel info as a holder */
            ChannelInfo channelInfo = new();

            /* Populate information from channel */
            if ((retCode = channel.Info(channelInfo, out error)) != TransportReturnCode.SUCCESS)
            {
                return TransportReturnCode.FAILURE;
            }
            /* Obtains a non-packable buffer of the requested size from the ETA Transport
             * guaranteed buffer pool to write into for the Login request Reject Status
             * Msg.
             *
             * When the Buffer is returned, the length member indicates the number of
             * bytes available in the buffer (this should match the amount the application
             * requested).
             *
             * When populating, it is required that the application set length to the
             * number of bytes actually used.
             *
             * This ensures that only the required bytes are written to the network.
             */
            /* EtaGetBuffer() is the utility function that does 2-pass (more robust) getting non-packable buffer. */
            if ((msgBuf = EtaGetBuffer(channel, channelInfo.MaxFragmentSize, out error)) == null)
            {
                /* Connection should be closed, return failure */
                /* Closes channel, closes server, cleans up and exits the application. */
                return TransportReturnCode.FAILURE;
            }

            encIter.Clear();
            /* set version information of the connection on the encode iterator so proper versioning can be performed */
            /* set the buffer on an EncodeIterator */
            CodecReturnCode codecCode = encIter.SetBufferAndRWFVersion(msgBuf, channel.MajorVersion, channel.MinorVersion);
            if ((codecCode) < CodecReturnCode.SUCCESS)
            {
                channel.ReleaseBuffer(msgBuf, out error);
                Console.WriteLine("\nSetEncodeIteratorBuffer() failed with return code: {0}", retCode);
                /* Closes channel, closes server, cleans up and exits the application. */
                return TransportReturnCode.FAILURE;
            }

            /* Create and initialize status message */
            IStatusMsg statusMsg = new Msg();

            /* if a buffer is returned, we can populate and write, encode an Msg into the buffer */

            /* Encodes the login request reject status. */

            /* On larger structures, like messages, the Clear functions tend to outperform
             * instantiation. It is recommended to use the Clear function when
             * initializing any messages.
             */
            statusMsg.Clear();
            /* set-up message */
            statusMsg.MsgClass = MsgClasses.STATUS;/* (3) Status Message */
            statusMsg.StreamId = streamId;
            statusMsg.DomainType = (int)DomainType.LOGIN;/* (1) Login Message */
            /* No payload associated with this close status message */
            statusMsg.ContainerType = DataTypes.NO_DATA;/* (128) No Data <BR>*/
            /* (0x020) Indicates that this StatusMsg has stream or group state
             *         information, contained in IStatusMsg.State.  */
            statusMsg.Flags = StatusMsgFlags.HAS_STATE;
            /* (3) Closed, the applications may attempt to re-open the stream later (can
             *     occur via either an RefreshMsg or an StatusMsg) */
            statusMsg.State.StreamState(StreamStates.CLOSED_RECOVER);
            /* (2) Data is Suspect (similar to a stale data state, indicates that the
             *     health of some or all data associated with the stream is out of date or
             *     cannot be confirmed that it is current ) */
            statusMsg.State.DataState(DataStates.SUSPECT);
            statusMsg.State.Code(StateCodes.NONE);

            /* Switch cases depending on login reject reason */
            switch (reason)
            {
                case LoginRejectReason.MAX_LOGIN_REQUESTS_REACHED:
                    {
                        /* (13) Too many items open (indicates that a request cannot be processed because
                         *      there are too many other streams already open) */
                        statusMsg.State.Code(StateCodes.TOO_MANY_ITEMS);

                        string stateText = $"Login request rejected for stream id {streamId} - max request count reached";

                        statusMsg.State.Text().Data(stateText);
                    }
                    break;
                case LoginRejectReason.NO_USER_NAME_IN_REQUEST:
                    {
                        /* (5) Usage Error (indicates an invalid usage within the system) */
                        statusMsg.State.Code(StateCodes.USAGE_ERROR);

                        string stateText = $"Login request rejected for stream id {streamId} - request does not contain user name";
                        statusMsg.State.Text().Data(stateText);
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
            if ((codecCode = statusMsg.Encode(encIter)) < CodecReturnCode.SUCCESS)
            {
                channel.ReleaseBuffer(msgBuf, out error);
                Console.WriteLine("\nEncodeMsg() failed with return code: {0}", codecCode);
                /* Closes channel, closes server, cleans up and exits the application. */
                return TransportReturnCode.FAILURE;
            }

            /* send login request reject status */
            if ((retCode = SendMessage(channel, msgBuf)) < TransportReturnCode.SUCCESS)
            {
                /* send login request reject status fails */
                /* Closes channel, closes server, cleans up and exits the application. */
                return TransportReturnCode.FAILURE;
            }
            else if (retCode > TransportReturnCode.SUCCESS)
            {
                /* There is still data left to flush */

                /* If the login request reject status doesn't flush, just close channel and exit the app.
                 *
                 * When you send login request reject status msg, we want to make a best effort to get this
                 * across the network as it will gracefully close the open login stream.
                 *
                 * If this cannot be flushed, this application will just close the connection for simplicity.
                 */

                /* Closes channel, closes server, cleans up and exits the application. */
            }

            return retCode;
        }

        /// <summary>Clears the login request info values</summary>
        public static void ClearLoginRequestInfo()
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

        #endregion
    }
}
