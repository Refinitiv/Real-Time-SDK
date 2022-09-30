/*
 *|-------------------------------------------------------------------------------
 *| This source code is provided under the Apache 2.0 license and is provided   --
 *| AS IS with no warranty or guarantee of fit for purpose.  See the project's  --
 *| LICENSE.md for details.                                                     --
 *| Copyright (C) 2022 Refinitiv. All rights reserved.                          --
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
 */

using System.Net.Sockets;

using Refinitiv.Eta.Transports;
using Refinitiv.Eta.Transports.Interfaces;
using Refinitiv.Eta.Example.Common;


namespace Refinitiv.Eta.Training.Consumer
{
    public class Module_1a_Connect
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
                    else
                    {
                        Console.Write("Error: Unrecognized option: {0}\n\n", args[i]);
                        Console.Write("Usage: {0} or\n{0} [-h <SrvrHostname>] [-p <SrvrPortNo>] [-i <InterfaceName>] \n",
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

            /* Set connect options
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
             * outbound connection, which can leverage standard sockets, HTTP, or HTTPS.
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
                         * exchanges, including any HTTP or HTTPS negotiation.  Compression, ping
                         * timeout, and versioning related negotiations also take place during the
                         * initialization process.
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
                    Console.Write("Exception. Stack trace: {0}\n", e1.StackTrace);
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
    }
}
