/*|-----------------------------------------------------------------------------
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
*/

using System;
using System.Collections.Generic;
using System.Linq;
using System.Net.Sockets;

using LSEG.Eta.Transports;


namespace LSEG.Eta.Training.NiProvider
{
    public class Module_1a_Connect
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
                    else
                    {
                        Console.Write("Error: Unrecognized option: {0}\n\n", args[i]);
                        Console.WriteLine("Usage: {0} or\n{0} [-h <SrvrHostname>] [-p <SrvrPortNo>] [-i <InterfaceName>]",
                            System.AppDomain.CurrentDomain.FriendlyName);
                        Environment.Exit((int)TransportReturnCode.FAILURE);
                    }
                }
            }

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
             * sockets. Returns an Channel that represents the connection to 
             * the user. In the event of an error, NULL is returned and 
             * additional information can be found in the Error structure.
             * Connection options are passed in via an ConnectOptions structure.
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
    }
}
