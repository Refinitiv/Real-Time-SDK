﻿/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2022-2023 Refinitiv. All rights reserved.              --
 *|-----------------------------------------------------------------------------
 */

/**
* This is the ETA NI Provider Training series of the ETA Training Suite
* applications. The purpose of this application is to show step-by-step
* training how to build a ETA OMM NI Provider using the ETA Transport layer.
*
* Main c source file for the ETA NI Provider Training application. It is a
* single-threaded client application.
*
************************************************************************
* ETA NI Provider Training Module 1a: Establish network communication
************************************************************************
* Summary:
* A Non-Interactive Provider (NIP) writes a provider application that
* connects to Refinitiv Real-Time Distribution System and sends a specific
* set (non-interactive) of information (services, domains, and capabilities).
* NIPs act like clients in a client-server relationship. Multiple NIPs can
* connect to the same Refinitiv Real-Time Distribution System and publish
* the same items and content.
*
* In this module, the OMM NIP application initializes the ETA Transport
* and establish a connection to an ADH server. Once connected, an OMM NIP
* can publish information into the ADH cache without needing to handle
* requests for the information. The ADH can cache the information and
* along with other Refinitiv Real-Time Distribution System components,
* provide the information to any NIProvider applications that indicate interest.
*
* Detailed Descriptions:
* The first step of any ETA NIP application is to establish network
* communication with an ADH server. To do so, the OMM NIP typically creates
* an outbound connection to the well-known hostname and port of an ADH.
* The OMM NIP uses the Connect function to initiate the connection
* process and then performs connection initialization processes as needed.
*
* Command line usage:
*
* NIProvMod1a.exe
* (runs with a default set of parameters (-h localhost -p 14003 -i ""))
*
* or
*
* NIProvMod1a.exe [-h <SrvrHostname>] [-p <SrvrPortNo>] [-i <InterfaceName>]
* (runs with specified set of parameters, all parameters are optional)
*
* Pressing the CTRL+C buttons terminates the program.
*
************************************************************************
* ETA NI Provider Training Module 1b: Ping (heartbeat) Management
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
* NIProvMod1b.exe
* (runs with a default set of parameters (-h localhost -p 14003 -i "" -r 300))
*
* or
*
* NIProvMod1b.exe [-h <SrvrHostname>] [-p <SrvrPortNo>] [-i <InterfaceName>] [-r <Running Time>]
* (runs with specified set of parameters, all parameters are optional)
*
* Pressing the CTRL+C buttons terminates the program.
*
*/

using System.Net.Sockets;
using LSEG.Eta.Common;
using LSEG.Eta.Codec;
using LSEG.Eta.Transports;

using DateTime = System.DateTime;


namespace LSEG.Eta.Training.NiProvider
{
    public class Module_1b_Connect
    {

        #region Global Variables
        static List<Socket> readSocketList = new();
        static List<Socket> writeSocketList = new();

        /* For this simple training app, only a single channel/connection is used for the
         * entire life of this app. */
        static IChannel? channel;

        #endregion

        public static void Main(string[] args)
        {
            #region Declaring variables

            TransportReturnCode retCode;

            /* Create error to keep track of any errors that may occur in Transport methods */
            Error error;

            /* Create and populate connect options to specify connection preferences */
            ConnectOptions cOpts = new();

            /* InProgInfo Information for the In Progress Connection State */
            InProgInfo inProgInfo = new();

            /* Create ChannelInfo */
            ChannelInfo channelInfo = new();

            ITransportBuffer msgBuf;

            /* use default runTime of 300 seconds */
            TimeSpan runTime = TimeSpan.FromSeconds(300);

            /* Create initialization arguments (InitArgs) to specify any initialization options for Transport */
            InitArgs initArgs = new();

            /* the default option parameters */
            /* connect to server running on same machine */
            string srvrHostname = "localhost";
            /* server is running on port number 14003 */
            string srvrPortNo = "14003";
            /* use default NIC network interface card to bind to for all inbound and outbound data */
            string interfaceName = "";

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
                        if (Int64.TryParse(args[i - 1], out var runTimeSeconds))
                        {
                            runTime = TimeSpan.FromSeconds(runTimeSeconds);
                        }
                        else
                        {
                            Console.WriteLine("Error: Could not parse runTime: {0}\n", args[i - 1]);
                            Console.WriteLine("Usage: {0} or\n{0} [-h <SrvrHostname>] [-p <SrvrPortNo>] [-i <InterfaceName>] [-r <runTime>]",
                                System.AppDomain.CurrentDomain.FriendlyName);

                            Environment.Exit((int)TransportReturnCode.FAILURE);
                        }
                    }
                    else
                    {
                        Console.Write("Error: Unrecognized option: {0}\n\n", args[i]);
                        Console.WriteLine("Usage: {0} or\n{0} [-h <SrvrHostname>] [-p <SrvrPortNo>] [-i <InterfaceName>] [-r <runTime>]",
                            System.AppDomain.CurrentDomain.FriendlyName);
                        Environment.Exit((int)TransportReturnCode.FAILURE);
                    }
                }
            }

            DateTime currentTime = DateTime.Now;
            DateTime etaRuntime = currentTime + runTime;

            #endregion

            #region Initialization

            /******************************************************************************************************************
             * INITIALIZATION - USING Initialize()
             ******************************************************************************************************************/
            /*********************************************************
             * Client/NIProv Application Life Cycle Major Step 1: Initialize ETA
             * Transport using Initialize The first ETA Transport function that an
             * application should call. This creates and initializes internal memory
             * and structures, as well as performing any bootstrapping for
             * underlying dependencies. The Initialize function also allows the user
             * to specify the locking model they want applied to the ETA Transport.
             *********************************************************/

            if (Transport.Initialize(initArgs, out error) != TransportReturnCode.SUCCESS)
            {
                Console.WriteLine("Error ({0}) (errno: {1}): {2}", error.ErrorId, error.SysError, error.Text);
                Environment.Exit((int)TransportReturnCode.FAILURE);
            }

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

            /* populate version and protocol with RWF information  or protocol specific info */
            cOpts.ProtocolType = (Transports.ProtocolType)Codec.Codec.ProtocolType(); /* Protocol type definition for RWF */
            cOpts.MajorVersion = Codec.Codec.MajorVersion();
            cOpts.MinorVersion = Codec.Codec.MinorVersion();

            #endregion

            #region Connection Setup
            /******************************************************************************************************************
             * CONNECTION SETUP - USING Connect()
             ******************************************************************************************************************/
            /*********************************************************
             * Client/NIProv Application Life Cycle Major Step 2: Connect using
             * Connect (OS connection establishment handshake) Connect call
             * Establishes an outbound connection, which can leverage standard
             * sockets. Returns an Channel that represents the connection to the
             * user. In the event of an error, NULL is returned and additional
             * information can be found in the Error structure. Connection
             * options are passed in via an ConnectOptions structure.
             *********************************************************/

            if ((channel = Transport.Connect(cOpts, out error)) == null)
            {
                Console.WriteLine("Error ({0}) (errno: {1}) encountered with Connect. Error Text : {2}",
                    error.ErrorId, error.SysError, error.Text);

                /* End application, uninitialize to clean up first */
                Transport.Uninitialize();
                Environment.Exit((int)TransportReturnCode.FAILURE);
            }

            long channelFDValue = channel.Socket.Handle.ToInt64();
            Console.WriteLine("Channel IPC descriptor = {0}", channelFDValue);

            /* Connection was successful, add socketId to I/O notification mechanism and initialize connection */

            #endregion

            #region Main loop

            /******************************************************************************************************************
             * MAIN LOOP TO SEE IF RESPONSE RECEIVED FROM PROVIDER
             ******************************************************************************************************************/
            /* Main loop for getting connection active and successful completion of the initialization process
             * The loop calls select() to wait for notification
             * Currently, the main loop would exit if an error condition is triggered or
             * Channel.state transitions to ACTIVE.
             */

            /* Max waiting time */
            TimeSpan timeOutSeconds = TimeSpan.FromSeconds(60);

            /* Writeability in Socket.Select means that the socket is connected. Will be
             * set to false in the end of the cycle once the connection is
             * established.  */
            bool opConnect = true;
            while (channel.State != ChannelState.ACTIVE)
            {
                try
                {
                    PerformSocketSelect((int)timeOutSeconds.TotalMilliseconds * 1000, opWrite: opConnect);

                    /* If our channel has not updated, we must have timed out */
                    if (readSocketList.Count == 0 && writeSocketList.Count == 0)
                    {
                        Console.WriteLine("Channel initialization has timed out.");
                        CloseChannelCleanUpAndExit(channel, error, TransportReturnCode.FAILURE);
                    }
                    else
                    {
                        /* Received a response from the provider. */

                        /****************************************************************************
                         * Step 3) Call Channel.init() to progress channel
                         * initialization further. * * This method is called
                         * multiple times throughout the Loop 1, as it makes *
                         * more progress towards channel initialization. *
                         ***************************************************************************/
                        if ((retCode = channel.Init(inProgInfo, out error)) < TransportReturnCode.SUCCESS)
                        {
                            Console.WriteLine("Error ({0}) (errno: {1}) encountered with Init Channel fd={2}. Error Text: {3}",
                                              error.ErrorId, error.SysError, channelFDValue, error.Text);
                            CloseChannelCleanUpAndExit(channel, error, TransportReturnCode.FAILURE);
                        }

                        /* Handle return code appropriately */
                        switch (retCode)
                        {
                            /* (2)  Transport Success: Channel initialization is In progress, returned from InitChannel. */
                            case TransportReturnCode.CHAN_INIT_IN_PROGRESS:
                                {
                                    /* Initialization is still in progress, check the InProgInfo for additional information */
                                    if (inProgInfo.Flags == InProgFlags.SCKT_CHNL_CHANGE)
                                    {
                                        /* The InitChannel function requires the use of an additional parameter, a InProgInfo structure.
                                         *
                                         * Under certain circumstances, the initialization process may be
                                         * required to create new or additional underlying connections.  If
                                         * this occurs, the application is required to unregister the previous
                                         * socketId and register the new socketId with the I/O notification
                                         * mechanism being used. When this occurs, the information is conveyed
                                         * by the InProgInfo and the InProgFlags.
                                         *
                                         * SCKT_CHNL_CHANGE indicates that a socketId change has occurred as a
                                         * result of this call. The previous socketId has been stored in
                                         * InProgInfo.oldSocket so it can be unregistered with the I/O
                                         * notification mechanism.
                                         *
                                         * The new socketId has been stored in InProgInfo.NewSocket so it can
                                         * be registered with the I/O notification mechanism. The channel
                                         * initialization is still in progress and subsequent calls to
                                         * InitChannel are required to complete it.
                                         */
                                        long oldChannelFDValue = channelFDValue;
                                        channelFDValue = channel.Socket.Handle.ToInt64();
                                        Console.WriteLine("\nChannel In Progress - New FD: {0}   Old FD: {1}",
                                                          channelFDValue, oldChannelFDValue);
                                    }
                                    else
                                    {
                                        Console.WriteLine("Channel {0} in progress...", channelFDValue);
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
                                                      channelFDValue);

                                    /*********************************************************
                                     * Connection is now active. The Channel can be
                                     * used for all additional transport
                                     * functionality (e.g. reading, writing) now
                                     * that the state transitions to ACTIVE
                                     *********************************************************/

                                    /* After channel is active, use ETA Transport utility function
                                     * GetChannelInfo to query Channel negotiated parameters and settings and
                                     * retrieve all current settings. This includes maxFragmentSize and
                                     * negotiated compression information as well as many other values.
                                     */

                                    /* Populate information from channel */
                                    if ((retCode = channel.Info(channelInfo, out error)) != TransportReturnCode.SUCCESS)
                                    {
                                        Console.WriteLine("Error ({0}) (errno: {1}) encountered with GetChannelInfo. Error Text: {2}",
                                                      error.ErrorId, error.SysError, error.Text);
                                        /* Connection should be closed, return failure */
                                        CloseChannelCleanUpAndExit(channel, error, TransportReturnCode.FAILURE);
                                    }

                                    /* Print out basic channel info */
                                    Console.Write("\nChannel {0} active. Channel Info:\n"
                                                  + "Max Fragment Size:           {1}\n"
                                                  + "Output Buffers:              {2} Max, {3} Guaranteed\n"
                                                  + "Input Buffers:               {4}\n"
                                                  + "Send/Receive Buffer Sizes:   {5}/{6}\n"
                                                  + "Ping Timeout:                {7}\n",
                                                  channelFDValue,
                                                  /*  This is the max fragment size before fragmentation and reassembly is necessary. */
                                                  channelInfo.MaxFragmentSize,
                                                  /* This is the maximum number of output buffers available to the channel. */
                                                  channelInfo.MaxOutputBuffers,
                                                  /*  This is the guaranteed number of output buffers available to the channel. */
                                                  channelInfo.GuaranteedOutputBuffers,
                                                  /*  This is the number of input buffers available to the channel. */
                                                  channelInfo.NumInputBuffers,
                                                  /*  This is the systems Send Buffer size. This reports the
                                                   *  systems send buffer size respective to the transport
                                                   *  type being used (TCP, UDP, etc) */
                                                  channelInfo.SysSendBufSize,
                                                  /*  This is the systems Receive Buffer size. This reports
                                                   *  the systems receive buffer size respective to the
                                                   *  transport type being used (TCP, UDP, etc) */
                                                  channelInfo.SysRecvBufSize,
                                                  /* This is the value of the negotiated ping timeout */
                                                  channelInfo.PingTimeout);

                                    Console.Write("Connected component version: ");

                                    if (channelInfo.ComponentInfoList.Count == 0)
                                    {
                                        Console.Write("(No component info)");
                                    }
                                    else
                                    {
                                        Console.WriteLine(string.Join(", ",
                                            channelInfo.ComponentInfoList.Select(ci => ci.ComponentVersion)));
                                    }
                                }
                                break;
                            default:
                                {
                                    Console.WriteLine("Bad return value fd={0} <{1}>", channelFDValue, retCode);
                                    CloseChannelCleanUpAndExit(channel, error, TransportReturnCode.FAILURE);
                                }
                                break;
                        }
                    }
                    opConnect = !channel.Socket.Connected;
                }
                catch (Exception e1)
                {
                    Console.Error.WriteLine("Error: {0}", e1.Message);
                    CloseChannelCleanUpAndExit(channel, error, TransportReturnCode.FAILURE);
                }
            }

            #endregion

            #region Initialize ping management

            /* Initialize ping management handler */
            InitPingManagement(channel);
            /*****************************************************************************************************************
             * SECOND MAIN LOOP TO CONNECTION ACTIVE - KEEP LISTEINING FOR INCOMING
             * DATA
             ******************************************************************************************************************/
            /* Here were are using a new Main loop. An alternative design would be to combine this Main loop with
             * the Main loop for getting connection active. Some bookkeeping would be required for that approach.
             */

            /* Main loop for message processing (reading data, writing data, and ping management, etc.)  The
             * loop calls Select() to wait for notification
             *
             * Currently, the only way to exit this Main loop is when an error condition is triggered or after
             * a predetermined run-time has elapsed.
             */
            while (true)
            {
                try
                {
                    /* Wait 1 seconds for any I/O notification updates in the channel */
                    PerformSocketSelect(1000 * 1000, false);

                    /* If our channel has been updated */
                    if (readSocketList.Count > 0)
                    {
                        /* reading data from channel via Read/Exception FD */

                        /* initialize to a positive value for Read call in case we have more data that is available to read */
                        retCode = (TransportReturnCode)((int)TransportReturnCode.SUCCESS + 1);

                        /******************************************************
                         * Loop 4) Read and decode for all buffers in channel *
                         ******************************************************/
                        while (retCode > TransportReturnCode.SUCCESS)
                        {
                            /* Create read arguments (ReadArgs) to keep track of the statuses of any reads */
                            ReadArgs readArgs = new();

                            /* There is more data to read and process and I/O notification may not trigger for it
                             * Either schedule another call to read or loop on read until retCode == TransportReturnCode.SUCCESS
                             * and there is no data left in internal input buffer
                             */

                            /*********************************************************
                             * Read using channel.Read() Read provides the user with
                             * data received from the connection. This function
                             * expects the Channel to be in the active state.
                             *
                             * When data is available, an Buffer referring to the
                             * information is returned, which is valid until the next
                             * call to Read.
                             *
                             * A return code parameter passed into the function is
                             * used to convey error information as well as communicate
                             * whether there is additional information to read.
                             *
                             * An I/O notification mechanism may not inform the user
                             * of this additional information as it has already been
                             * read from the socket and is contained in the Read input
                             * buffer.
                             *********************************************************/

                            if ((msgBuf = channel.Read(readArgs, out error)) != null)
                            {
                                /* if a buffer is returned, we have data to process and code is success */
                                /* Processes a response from the channel/connection.
                                 *
                                 * This consists of performing a high level decode of the message and then
                                 * calling the applicable specific function for further processing.
                                 */
                            }
                            else
                            {

                                /* Deduce an action from the return code of Channel.read() */
                                retCode = readArgs.ReadRetVal;
                                switch (retCode)
                                {
                                    /* (-13) Transport Success: Read has received a ping message. There is no buffer in this case. */
                                    /* Acknowledge that a ping has been received */

                                    case TransportReturnCode.READ_PING:
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
                                    case TransportReturnCode.READ_FD_CHANGE:
                                        {
                                            /* File descriptor changed, typically due to tunneling keep-alive */
                                            long oldChannelFDValue = channelFDValue;
                                            channelFDValue = channel.Socket.Handle.ToInt64();
                                            Console.WriteLine("\nChannel In Progress - New FD: {0}   Old FD: {1}", channelFDValue, oldChannelFDValue);
                                        }
                                        break;
                                    /* (-11) Transport Success: Reading was blocked by the OS. Typically
                                     *       indicates that there are no bytes available to read, returned
                                     *       from Read.
                                     */
                                    case TransportReturnCode.READ_WOULD_BLOCK: /* Nothing to read */
                                        break;
                                    case TransportReturnCode.READ_IN_PROGRESS:/* fall through to default. */
                                    case TransportReturnCode.INIT_NOT_INITIALIZED:
                                    case TransportReturnCode.FAILURE:
                                        Console.WriteLine("Error ({0}) (errno: {1}) {2}", error.ErrorId, error.SysError, error.Text);
                                        CloseChannelCleanUpAndExit(channel, error, TransportReturnCode.FAILURE);
                                        break;

                                    default: /* Error handling */
                                        {
                                            if (retCode < 0)
                                            {
                                                Console.WriteLine("Error ({0}) (errno: {1}) encountered with Read. Error Text: {2}", error.ErrorId, error.SysError, error.Text);
                                                CloseChannelCleanUpAndExit(channel, error, TransportReturnCode.FAILURE);
                                            }
                                        }
                                        break;
                                }
                            }
                        }

                        /* An I/O notification mechanism can be used to indicate when the operating system can
                         * accept more data for output.
                         * 
                         * Flush function is called because of a write file descriptor alert
                         */
                    }

                    /* Processing ping management handler */
                    if ((retCode = ProcessPingManagementHandler(channel, error)) > TransportReturnCode.SUCCESS)
                    {
                        /* There is still data left to flush, leave our write notification enabled so we get called again.
                         * If everything wasn't flushed, it usually indicates that the TCP output buffer cannot accept more yet
                         */

                        /* set write fd if there's still other data queued */
                        /* flush is done by application */
                    }
                    else if (retCode < TransportReturnCode.SUCCESS)
                    {
                        /* Closes channel, cleans up and exits the application. */
                        CloseChannelCleanUpAndExit(channel, error, TransportReturnCode.FAILURE);
                    }

                    /* get current time */
                    currentTime = DateTime.Now;

                    /* Handles the run-time for the ETA NI Provider application. Here we exit the application after a predetermined time to run */
                    /* If the runtime has expired */
                    if (currentTime >= etaRuntime)
                    {
                        Console.Write("ETA Client run-time has expired...\n\n");
                        CloseChannelCleanUpAndExit(channel, error, TransportReturnCode.SUCCESS);
                    }
                }
                catch (Exception e1)
                {
                    Console.WriteLine("Error: {0}", e1.Message);
                }
            }

            #endregion
        }

        private static void PerformSocketSelect(int timeOutUs = 1000 * 1000, bool opWrite = true)
        {
            readSocketList.Clear();
            writeSocketList.Clear();

            if (channel != null)
                readSocketList.Add(channel.Socket);
            if (channel != null && opWrite)
                writeSocketList.Add(channel.Socket);

            Socket.Select(readSocketList, writeSocketList, null, timeOutUs);
        }

        /// <summary>Closes channel, cleans up and exits the application.</summary>
        ///
        /// <param name="etaChannel">The channel to be closed</param>
        /// <param name="code">if exit due to errors/exceptions</param>
        public static void CloseChannelCleanUpAndExit(IChannel? channel, Error error, TransportReturnCode code)
        {
            bool isClosedAndClean = true;

            /*********************************************************
             * Client/NIProv Application Life Cycle Major Step 5: Close connection
             * using CloseChannel (OS connection release handshake) CloseChannel
             * closes the client based Channel. This will release any pool based
             * resources back to their respective pools, close the connection, and
             * perform any additional necessary cleanup. When shutting down the
             * Transport, the application should release all unwritten pool buffers.
             * Calling CloseChannel terminates the connection to the ADH.
             *********************************************************/
            if ((channel != null))
            {
                isClosedAndClean = channel.Close(out error) >= TransportReturnCode.SUCCESS;
            }

            /*********************************************************
             * Client/NIProv Application Life Cycle Major Step 6: Uninitialize ETA
             * Transport using Uninitialize The last ETA Transport function that an
             * application should call. This uninitializes internal data structures
             * and deletes any allocated memory.
             *********************************************************/

            /* All ETA Transport use is complete, must uninitialize.
             * The uninitialization process allows for any heap allocated memory to be cleaned up properly.
             */
            Transport.Uninitialize();

            if (isClosedAndClean)
            {
                Console.WriteLine("NIProvider application has closed channel and has cleaned up successfully.");
            }
            else
            {
                Console.WriteLine("Error ({0}) (errno: {1}) encountered with CloseChannel. Error Text: {2}",
                    error.ErrorId, error.SysError, error.Text);
            }

            /* For applications that do not exit due to errors/exceptions such as:
             * Exits the application if the run-time has expired.
             */
            if (code == (int)TransportReturnCode.SUCCESS)
                Console.WriteLine("\nETA NI Provider Training application successfully ended.");

            /* End application */
            Environment.Exit(0);
        }

        #region Ping management
        /*
         * Initializes the ping times for etaChannel.
         * etaChannel - The channel for ping management info initialization
         */
        static TimeSpan pingTimeoutServer; /* server ping timeout */
        static TimeSpan pingTimeoutClient; /* client ping timeout */
        static DateTime nextReceivePingTime; /* time client should receive next message/ping from server */
        static DateTime nextSendPingTime; /* time to send next ping from client */
        static bool receivedServerMsg; /* flag for server message received */

        public static void InitPingManagement(IChannel channel)
        {
            /* set ping timeout for local and remote pings */
            pingTimeoutClient = TimeSpan.FromSeconds(channel.PingTimeOut / 3);
            pingTimeoutServer = TimeSpan.FromSeconds(channel.PingTimeOut);

            /* set time to send next ping to remote connection */
            nextSendPingTime = DateTime.Now + pingTimeoutClient;

            /* set time should receive next ping from remote connection */
            nextReceivePingTime = DateTime.Now + pingTimeoutServer * 1000;
        }


        /// <summary>Processing ping management handler</summary>
        ///
        /// <param name="etaChannel">The channel for ping management processing</param>
        public static TransportReturnCode ProcessPingManagementHandler(IChannel channel, Error error)
        {
            /* Handles the ping processing for etaChannel. Sends a ping to the server if the next send ping
             * time has arrived and checks if a ping has been received from the server within the next receive
             * ping time.
             */
            TransportReturnCode retval = TransportReturnCode.SUCCESS;

            /* get current time */
            DateTime currentTime = DateTime.Now;

            /* handle client pings */
            if (currentTime >= nextSendPingTime)
            {
                /* send ping to server */
                /*********************************************************
                 * Client/NIProv Application Life Cycle Major Step 4: Ping using
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
                     * space in the connections output buffer.
                     *
                     * An I/O notification mechanism can be used to indicate when the socketId has write
                     * availability.
                     *
                     * There is still data left to flush, leave our write notification enabled so we get
                     * called again.
                     *
                     * If everything wasn't flushed, it usually indicates that the TCP output buffer cannot
                     * accept more yet
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
                                Console.Write("Ping message has been sent successfully to the server ...\n\n");
                            }
                            break;
                        case TransportReturnCode.FAILURE: /* fall through to default. */
                        default: /* Error handling */
                            {
                                Console.WriteLine("Error ({0}) (errno: {1}) encountered with Ping(). Error Text:{2}",
                                    error.ErrorId, error.SysError, error.Text);
                                CloseChannelCleanUpAndExit(channel, error, TransportReturnCode.FAILURE);
                                /* Closes channel/connection, cleans up and exits the application. */
                                return TransportReturnCode.FAILURE;
                            }
                    }
                }

                /* set time to send next ping from client */
                nextSendPingTime = currentTime + pingTimeoutClient;
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
                    nextReceivePingTime = currentTime + pingTimeoutServer;
                }
                else /* lost contact with server */
                {
                    /* Lost contact with remote (connection) */
                    error.Text = "Lost contact with connection...";
                    Console.WriteLine("Error ({0}) (errno: {1}) {2}", error.ErrorId, error.SysError, error.Text);
                    CloseChannelCleanUpAndExit(channel, error, TransportReturnCode.FAILURE);
                }
            }
            return retval;
        }

        #endregion
    }
}
