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
 *****************************************************************************************/


using System.Net.Sockets;
using LSEG.Eta.Codec;
using LSEG.Eta.Transports;

namespace LSEG.Eta.Training.Provider
{
    public class Module_1a_Connect
    {
        #region Global Variables

        static List<Socket> readSocketList = new();

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

            /* For this simple training app, the interactive provider only supports a single client. If the
             * consumer disconnects, the interactive provider would simply exit.
             *
             * If you want the provider to support multiple client sessions at the same time, you need to
             * implement support for multiple client sessions feature similar to what Provider example is
             * doing.
             */

            /* Create accept options to specify any options for accepting */
            AcceptOptions acceptOpts = new() { NakMount = false };

            /* Create initialization progress info (InProgInfo) to keep track of channel initialization with Channel.Init() */
            InProgInfo inProgInfo = new();

            TransportReturnCode retCode;

            /* the default option parameters */
            /* server is running on port number 14002 */
            string srvrPortNo = "14002";

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
                    else
                    {
                        Console.WriteLine("Error: Unrecognized option: {0}\n", args[i]);
                        Console.WriteLine("Usage: {0} or\n{0} [-p <SrvrPortNo>]", System.AppDomain.CurrentDomain.FriendlyName);
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
                    PerformSocketSelect();

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
                         * initialization includes any necessary ETA connection handshake exchanges, including
                         * any HTTP or HTTPS negotiation.  Compression, ping timeout, and versioning related
                         * negotiations also take place during the initialization process.
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
                            break;
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

                                    /* After channel is active, use ETA Transport utility function GetChannelInfo to
                                     * query Channel negotiated parameters and settings and retrieve all current
                                     * settings. This includes maxFragmentSize and negotiated compression information as
                                     * well as many other values. */

                                    /* Populate information from channel */

                                    /* Create channel info as a holder */
                                    ChannelInfo channelInfo = new ChannelInfo();

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

                                    /* clean up server using CloseServer call.
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
                                    Console.WriteLine("Bad return value fd={0}: <{1}>",
                                        clientChannelFDValue, retCode);
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
            #endregion
        }

        private static void PerformSocketSelect()
        {
            readSocketList.Clear();
            if (channel != null)
                readSocketList.Add(channel.Socket);

            Socket.Select(readSocketList, null, null, (int)(timeOut.TotalMilliseconds * 1000));
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
    }
}
