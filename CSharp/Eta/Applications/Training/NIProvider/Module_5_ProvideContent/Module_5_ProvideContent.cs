/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2022 Refinitiv. All rights reserved.              --
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
************************************************************************
* ETA NI Provider Training Module 1c: Reading and Writing Data
************************************************************************
* Summary:
*
* In this module, when a client or server Channel.State is
* ACTIVE, it is possible for an application to receive
* data from the connection. Similarly, when a client or server
* Channel.State is ACTIVE, it is possible for an
* application to write data to the connection. Writing involves a several
* step process.
*
* Detailed Descriptions:
*
* When a client or server Channel.State is ACTIVE, it is possible for an
* application to receive data from the connection. The arrival of this
* information is often announced by the I/O notification mechanism that the
* Channel.socketId is registered with.
*
* The ETA Transport reads information from the network as a byte stream, after
* which it determines Buffer boundaries and returns each buffer one by one.
*
* When a client or server Channel.state is ACTIVE, it is
* possible for an application to write data to the connection. Writing
* involves a several step process. Because the ETA Transport provides
* efficient buffer management, the user is required to obtain a buffer
* from the ETA Transport buffer pool. This can be the guaranteed output
* buffer pool associated with an Channel.
*
* After a buffer is acquired, the user can populate the Buffer.Data and set the
* Buffer.Length to the number of bytes referred to by data.
*
* If queued information cannot be passed to the network, a function is provided
* to allow the application to continue attempts to flush data to the
* connection.
*
* An I/O notification mechanism can be used to help with determining when the
* network is able to accept additional bytes for writing. The ETA Transport can
* continue to queue data, even if the network is unable to write.
*
* Command line usage:
*
* NIProvMod1c.exe
* (runs with a default set of parameters (-h localhost -p 14003 -i "" -r 300))
*
* or
*
* NIProvMod1c.exe [-h <SrvrHostname>] [-p <SrvrPortNo>] [-i <InterfaceName>] [-r <Running Time>]
* (runs with specified set of parameters, all parameters are optional)
*
* Pressing the CTRL+C buttons terminates the program.
*
************************************************************************
* ETA NI Provider Training Module 2: Log in
************************************************************************
* Summary:
* In this module, applications authenticate with one another using the Login
* domain model. An OMM NIP must register with the system using a Login request
* prior to providing any content. Because this is done in an interactive manner,
* the NIP should assign a streamId with a positive value which the ADH will
* reference when sending its response.
*
* Detailed Descriptions:
*
* After receiving a Login request, the ADH determines whether the NIP is
* permissioned to access the system. The ADH sends a Login response, indicating
* to the NIP whether the ADH grants it access.
*
* a) If the application is denied, the ADH closes the Login stream and the NI
*    provider application cannot perform any additional communication.
*
* b) If the application gains access to the ADH, the Login response informs the
*    application of this. The NI provider must now provide a Source Directory.
*
* Content is encoded and decoded using the ETA Message Package and the ETA
* Data Package.
*
* Command line usage:
*
* NIProvMod2.exe
* (runs with a default set of parameters (-h localhost -p 14003 -i "" -r 300 -s DIRECT_FEED))
*
* or
*
* NIProvMod2.exe [-h <SrvrHostname>] [-p <SrvrPortNo>] [-i <InterfaceName>] [-r <Running Time>]
* (runs with specified set of parameters, all parameters are optional)
*
* Pressing the CTRL+C buttons terminates the program.
*
************************************************************************
* ETA NI Provider Training Module 3: Provide Source Directory Information
************************************************************************
*
* Summary:
*
* In this module, OMM NIP application provides Source Directory information.
*
* The Source Directory domain model conveys information about all available
* services in the system. After completing the Login process, an OMM NIP must
* provide a Source Directory refresh.
*
* Detailed Descriptions:
*
* The Source Directory domain model conveys information about all
* available services in the system. After completing the Login process,
* an OMM NIP must provide a Source Directory refresh indicating:
*
* a) Service, service state, QoS, and capability information associated with the
*    NIP
*
* b) Supported domain types and any item group information associated with the
*    service.
*
* At a minimum, Refinitiv recommends that the NIP send the Info,
* State, and Group filters for the Source Directory. Because this is provider
* instantiated, the NIP should use a streamId with a negative value.
*
* a) The Source Directory Info filter contains service name and serviceId
*    information for all available services, though NIPs typically provide data
*    on only one service.
*
* b) The Source Directory State filter contains status information for service.
*    This informs the ADH whether the service is Up and available or Down and
*    unavailable.
*
* c) The Source Directory Group filter conveys item group status information,
*    including information about group states as well as the merging of groups.
*    For additional information about item groups, refer to ETAC Developer
*    Guide.
*
* Content is encoded and decoded using the ETA Message Package and the ETA
* Data Package.
*
* Command line usage:
*
* NIProvMod3.exe
* (runs with a default set of parameters (-h localhost -p 14003 -i "" -r 300 -s DIRECT_FEED -id 1))
*
* or
*
* NIProvMod3.exe [-h <SrvrHostname>] [-p <SrvrPortNo>] [-i <InterfaceName>] [-r <Running Time>] [-s <Service Name>] [-id <Service ID>]
* (runs with specified set of parameters, all parameters are optional)
*
* Pressing the CTRL+C buttons terminates the program.
*
************************************************************************
* ETA NI Provider Training Module 4: Load Dictionary Information
************************************************************************
* Summary:
* Dictionaries may be available locally in a file for an OMM NIP appliation. In
* this Training example, the OMM NIP will use dictionaries that are available
* locally in a file.
*
* Detailed Descriptions:
* Some data requires the use of a dictionary for encoding or decoding. This
* dictionary typically defines type and formatting information and directs
* the application as to how to encode or decode specific pieces of information.
* Content that uses the FieldList type requires the use of a field dictionary
* (usually the Refinitiv RDMFieldDictionary, though it could also be a
* user-defined or modified field dictionary).
*
* Dictionaries may be available locally in a file for an OMM NIP appliation. In
* this Training example, the OMM NIP will use dictionaries that are available
* locally in a file.
*
* Command line usage:
*
* NIProvMod4.exe
* (runs with a default set of parameters (-h localhost -p 14003 -i "" -r 300 -s DIRECT_FEED -id 1))
*
* or
*
* NIProvMod4.exe [-h <SrvrHostname>] [-p <SrvrPortNo>] [-i <InterfaceName>] [-r <Running Time>] [-s <Service Name>] [-id <Service ID>]
* (runs with specified set of parameters, all parameters are optional)
*
* Pressing the CTRL+C buttons terminates the program.
*
*
************************************************************************
* ETA NI Provider Training Module 5: Provide Content
************************************************************************
* Summary:
* In this module, after providing a Source Directory, the OMM NIP application can
* begin pushing content to the ADH. In this simple example, we just show functions
* for sending 1 MP(MarketPrice) domain type Item refresh, update, and
* close status message(s) to an ADH.
*
* Detailed Descriptions:
* After providing a Source Directory, the NIP application can begin pushing content
* to the ADH. Each unique information stream should begin with an RefreshMsg,
* conveying all necessary identification information for the content. Because the
* provider instantiates this information, a negative value streamId should be used
* for all streams. The initial identifying refresh can be followed by other status
* or update messages. Some ADH functionality, such as cache rebuilding, may require
* that NIP applications publish the message key on all RefreshMsgs. See the
* component specific documentation for more information.
*
* Some components may require that NIP applications publish the msgKey in UpdateMsgs.
* To avoid component or transport migration issues, NIP applications may want to always
* include this information.
*
* Content is encoded and decoded using the ETA Message Package and the ETA
* Data Package.
*
* Command line usage:
*
* NIProvMod5.exe
* (runs with a default set of parameters (-h localhost -p 14003 -i "" -r 300 -s DIRECT_FEED -id 1 -mp TRI))
*
* or
*
* NIProvMod5.exe [-h <SrvrHostname>] [-p <SrvrPortNo>] [-i <InterfaceName>] [-r <Running Time>] [-s <Service Name>] [-mp <Market Price Item Name>]
* (runs with specified set of parameters, all parameters are optional)
*
* Pressing the CTRL+C buttons terminates the program.
*
*/

using System.Net.Sockets;

using Refinitiv.Common.Interfaces;

using Refinitiv.Eta.Codec;
using Refinitiv.Eta.Example.Common;
using Refinitiv.Eta.Rdm;
using Refinitiv.Eta.Transports;
using Refinitiv.Eta.Transports.Interfaces;

using Buffer = Refinitiv.Eta.Codec.Buffer;
using DateTime = System.DateTime;
using Directory = Refinitiv.Eta.Rdm.Directory;

namespace Refinitiv.Eta.Training.NiProvider
{
    public class Module_5_ProvideContent
    {
        #region Global Variables
        static List<Socket> readSocketList = new();
        static List<Socket> writeSocketList = new();

        /* For this simple training app, only a single channel/connection is used for the
         * entire life of this app. */
        static IChannel? channel;

        /// <summary>dictionary file name</summary>
        const string FIELD_DICTIONARY_FILE_NAME = "RDMFieldDictionary";

        /// <summary>enum table file name</summary>
        const string ENUM_TYPE_DICTIONARY_FILE_NAME = "enumtype.def";

        static bool marketPriceItemInfo_isRefreshComplete;
        static string marketPriceItemInfo_itemName = String.Empty;
        static MarketPriceItem marketPriceItemInfo_itemData = new();
        const int MARKETPRICE_ITEM_STREAM_ID_START = -2;

        static bool opWrite = false;

        #endregion

        public static void Main(string[] args)
        {
            #region Declaring variables

            int serviceId = 1;

            TransportReturnCode retCode;
            /* Create error to keep track of any errors that may occur in Transport methods */
            Error error;

            /* Create and populate connect options to specify connection preferences */
            ConnectOptions cOpts = new();

            /* InProgInfo Information for the In Progress Connection State */
            InProgInfo inProgInfo = new();

            /* Create ChannelInfo */
            ChannelInfo channelInfo = new();

            int maxMsgSize; /* the requested size of the buffer for GetBuffer function to obtain from the guaranteed/shared buffer pool. */
            ITransportBuffer msgBuf;

            /* default service name is "DIRECT_FEED" used in source directory handler */
            string serviceName = "DIRECT_FEED";

            /* default item name is "TRI" used in market price item response handler */
            string itemName = "TRI";

            /* use default runTime of 300 seconds */
            TimeSpan runTime = TimeSpan.FromSeconds(300);

            /* Iterator used for encoding throughout the application - we can clear it and reuse it instead of recreating it */
            EncodeIterator encodeIter = new();

            /* Iterator used for decoding throughout the application - we can clear it and reuse it instead of recreating it */
            DecodeIterator decodeIter = new();

            /* In this app, we are only interested in using 2 dictionaries:
             * - Refinitiv Field Dictionary (RDMFieldDictionary) and
             * - Enumerated Types Dictionaries (enumtype.def)
             *
             * Dictionaries may be available locally in a file for an OMM NIP appliation. In
             * this Training example, the OMM NIP will use dictionaries that are available
             * locally in a file.

             * Some data requires the use of a dictionary for encoding or decoding. This
             * dictionary typically defines type and formatting information and directs
             * the application as to how to encode or decode specific pieces of information.
             * Content that uses the FieldList type requires the use of a field dictionary
             * (usually the Refinitiv RDMFieldDictionary, though it could also be a
             * user-defined or modified field dictionary).
             */

            /* data dictionary */
            DataDictionary dataDictionary = new();

            string errTxt = string.Empty;
            Buffer errorText = new();
            errorText.Data(errTxt);

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
                            Console.WriteLine("Usage: {0} or\n{0} [-h <SrvrHostname>] [-p <SrvrPortNo>] [-i <InterfaceName>] [-r <runTime>] [-s <ServiceName>] [-id <ServiceId>]",
                                System.AppDomain.CurrentDomain.FriendlyName);

                            Environment.Exit((int)TransportReturnCode.FAILURE);
                        }
                    }
                    else if ((args[i].Equals("-s")) == true)
                    {
                        i += 2;
                        serviceName = args[i - 1];
                    }
                    else if ((args[i].Equals("-id")) == true)
                    {
                        i += 2;

                        if (!Int32.TryParse(args[i - 1], out serviceId))
                        {
                            Console.WriteLine("Error: Could not parse serviceId: {0}\n", args[i - 1]);
                            Console.WriteLine("Usage: {0} or\n{0} [-h <SrvrHostname>] [-p <SrvrPortNo>] [-i <InterfaceName>] [-r <runTime>] [-s <ServiceName>] [-id <ServiceId>]",
                                System.AppDomain.CurrentDomain.FriendlyName);

                            Environment.Exit((int)TransportReturnCode.FAILURE);

                        }
                    }
                    else if ((args[i].Equals("-mp")) == true)
                    {
                        i += 2;
                        itemName = args[i - 1];
                    }
                    else
                    {
                        Console.Write("Error: Unrecognized option: {0}\n\n", args[i]);
                        Console.WriteLine("Usage: {0} or\n{0} [-h <SrvrHostname>] [-p <SrvrPortNo>] [-i <InterfaceName>] [-r <runTime>] [-s <ServiceName>] [-id <ServiceId>]",
                            System.AppDomain.CurrentDomain.FriendlyName);
                        Environment.Exit((int)TransportReturnCode.FAILURE);
                    }
                }
            }

            bool isLoginSuccessful = false;

            DateTime currentTime = DateTime.Now;
            DateTime etaRuntime = currentTime + runTime;

            #endregion

            #region Initialize Market Item

            /* Initialize item handler - initialize/reset market price item information */
            marketPriceItemInfo_itemName = itemName;
            marketPriceItemInfo_isRefreshComplete = false;

            /* Initializes market price item fields. */
            marketPriceItemInfo_itemData.InitFields();
            // isInUse = true;
            // ASK_TIME.localTime();
            // setSALTIME();
            // PERATIO = 5.00;

            // marketPriceItem.RDNDISPLAY = 100;
            // marketPriceItem.RDN_EXCHID = 155;
            // tempBuffer.data = (char *)"05/17/2013";
            // tempBuffer.length = (UInt32)strlen("05/17/2013");
            // DateStringToDate(&marketPriceItem.DIVPAYDATE, &tempBuffer);
            // marketPriceItem.TRDPRC_1 = 1.00;
            // marketPriceItem.BID = 0.99;
            // marketPriceItem.ASK = 1.03;
            // marketPriceItem.ACVOL_1 = 100000;
            // marketPriceItem.NETCHNG_1 = 2.15;
            // DateTimeLocalTime(&marketPriceItem.ASK_TIME);

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

            #region Load Dictionaries
            /*********************************************************
             * For performance considerations, it is recommended to first load field
             * and enumerated dictionaries from local files, if they exist, at the
             * earlier stage of the consumer applications.
             *
             * When loading from local files, ETA offers several utility functions
             * to load and manage a properly-formatted field dictionary and enum
             * type dictionary.
             *
             * Only make Market Price item request after both dictionaries are
             * successfully loaded from files. If at least one of the dictionaries
             * fails to get loaded properly, it will continue the code path of
             * downloading the failed loaded dictionary or both dictionaries (if
             * neither exists in run-time path or loaded properly) from provider
             *********************************************************/

            /* clear the DataDictionary dictionary before first use/load
             * This should be done prior to the first call of a dictionary loading function, if the initializer is not used.
             */
            dataDictionary.Clear();

            /* load field dictionary from file - adds data from a Field Dictionary file to the DataDictionary */

            if (dataDictionary.LoadFieldDictionary(FIELD_DICTIONARY_FILE_NAME, out var dataError) < 0)
            {
                Console.WriteLine("\nUnable to load field dictionary: {0}.\n\tError Text: {1}", FIELD_DICTIONARY_FILE_NAME, dataError.Text);
                Environment.Exit((int)TransportReturnCode.FAILURE);
            }
            else
            {
                Console.Write("Successfully loaded field dictionary from local file.\n\n");
            }

            /* load enumerated dictionary from file - adds data from an Enumerated Types Dictionary file to the DataDictionary */

            if (dataDictionary.LoadEnumTypeDictionary(ENUM_TYPE_DICTIONARY_FILE_NAME, out var enumError) < 0)
            {
                Console.WriteLine("\nUnable to load enum type dictionary: {0}.\n\tError Text: {1}", ENUM_TYPE_DICTIONARY_FILE_NAME, enumError.Text);
                Environment.Exit((int)TransportReturnCode.FAILURE);
            }
            else
            {
                Console.Write("Successfully loaded enum type dictionary from local file.\n\n");
            }

            Console.Write("ETA NIP application has successfully loaded both dictionaries from local files.\n\n");

            #endregion

            #region Connection Setup
            /******************************************************************************************************************
             * CONNECTION SETUP - USING Connect()
             ******************************************************************************************************************/
            /*********************************************************
             * Client/NIProv Application Life Cycle Major Step 2: Connect using
             * Connect (OS connection establishment handshake) Connect call
             * Establishes an outbound connection, which can leverage standard
             * sockets, HTTP, or HTTPS. Returns an Channel that represents the
             * connection to the user. In the event of an error, NULL is returned
             * and additional information can be found in the Error structure.
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

            /* maxMsgSize is the requested size of GetBuffer function. In this application, we set maxMsgSize to
             * be equal to maxFragmentSize from the channel info. maxFragmentSize from the channel info is the maximum
             * packable size, that is, the max fragment size before fragmentation and reassembly is necessary.
             * If the requested size is larger than the maxFragmentSize, the transport will create and return the buffer
             * to the user. When written, this buffer will be fragmented by the Write function.
             * Because of some additional book keeping required when packing, the application must specify whether
             * a buffer should be 'packable' when calling GetBuffer.
             * For performance purposes, an application is not permitted to request a buffer larger than maxFragmentSize
             * and have the buffer be 'packable.'
             */
            maxMsgSize = channelInfo.MaxFragmentSize; /* This is the max fragment size before fragmentation and reassembly is necessary. */

            #endregion

            #region Initialize ping management

            /* Initialize ping management handler */
            InitPingManagement(channel);

            /* clear encode iterator for initialization/use - this should be used to achieve the best performance while clearing the iterator. */
            encodeIter.Clear();

            /* Send Login request message */
            if ((retCode = SendLoginRequest(channel, maxMsgSize, encodeIter)) > TransportReturnCode.SUCCESS)
            {
                /* There is still data left to flush, leave our write notification enabled so we get called again.
                 * If everything wasn't flushed, it usually indicates that the TCP output buffer cannot accept more yet
                 */

                /* set write fd if there's still other data queued */
                /* flush is done by application */
                opWrite = true;
            }
            else if (retCode < TransportReturnCode.SUCCESS)
            {
                Console.WriteLine("Error ({0}) (errno: {1}) encountered with Init Channel fd={2}. Error Text: {3}", error.ErrorId, error.SysError, channelFDValue, error.Text);
                CloseChannelCleanUpAndExit(channel, error, TransportReturnCode.FAILURE);
            }

            /*track of simulating price update*/
            DateTime publishtime = DateTime.MinValue;

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
                    PerformSocketSelect(1000 * 1000, opWrite);
                    if (publishtime < DateTime.Now)
                    {
                        if (isLoginSuccessful && readSocketList.Count == 0)
                        {

                            /*
                             * Updates the item that's currently in use to simulate the Market Price movement.
                             */
                            marketPriceItemInfo_itemData.UpdateFields();
                            // TRDPRC_1 += 0.01;
                            // BID += 0.01;
                            // ASK += 0.01;
                            //
                            // ASK_TIME.localTime();
                            // PERATIO += 0.01;
                            // setSALTIME();
                            // DateTimeLocalTime(&marketPriceItem.ASK_TIME);
                            SendMarketPriceItemResponse(channel, maxMsgSize, encodeIter, serviceId, dataDictionary);
                        }
                        publishtime = DateTime.Now + TimeSpan.FromSeconds(1);
                    }

                    /* If our channel has been updated */
                    if (readSocketList.Count > 0)
                    {
                        /* reading data from channel via Read/Exception FD */

                        /* When a client Channel.state is ACTIVE, it is possible for an application to receive
                         * data from the connection.
                         *
                         * The arrival of this information is often announced by the I/O notification
                         * mechanism that the Channel.socketId is registered with.
                         *
                         * The ETA Transport reads information from the network as a byte stream, after which
                         * it determines Buffer boundaries and returns each buffer one by one.
                         */

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

                                /* No need to clear the message before we decode into it. ETA Decoding populates all message members (and that is true for any
                                 * decoding with ETA, you never need to clear anything but the iterator)
                                 */
                                /* We have data to process */

                                /* Create message to represent buffer data */
                                Msg msg = new();

                                /* This decodeIter.Clear() clear iterator function should be used to achieve
                                 * the best performance while clearing the iterator. */
                                /* Clears members necessary for decoding and readies the iterator for reuse.
                                 *
                                 * You must clear DecodeIterator before decoding content. For performance
                                 * purposes, only those members required for proper functionality are cleared.
                                 */
                                decodeIter.Clear();

                                /* Set the RWF version to decode with this iterator */
                                CodecReturnCode codeCode = decodeIter.SetBufferAndRWFVersion(msgBuf, channel.MajorVersion, channel.MinorVersion);

                                /* Associates the DecodeIterator with the Buffer from which to decode. */
                                if (codeCode != CodecReturnCode.SUCCESS)
                                {
                                    Console.WriteLine("\nSetDecodeIteratorBuffer() failed with return code: {0}", codeCode);
                                    /* Closes channel, cleans up and exits the application. */
                                    CloseChannelCleanUpAndExit(channel, error, TransportReturnCode.FAILURE);
                                }

                                /* decode contents into the Msg structure */

                                if ((codeCode = msg.Decode(decodeIter)) != CodecReturnCode.SUCCESS)
                                {
                                    Console.WriteLine("DecodeMsg(): Error ({0}) (errno: {1}): {2}", error.ErrorId, error.SysError, error.Text);
                                    CloseChannelCleanUpAndExit(channel, error, TransportReturnCode.FAILURE);
                                }

                                switch (msg.DomainType)
                                {
                                    /* (1) Login Message */
                                    case (int)DomainType.LOGIN:
                                        {
                                            if (ProcessLoginResponse(msg, decodeIter) != TransportReturnCode.SUCCESS)
                                            {
                                                /* Login Failed and the application is denied - Could be one of the following 3 possibilities:
                                                 *
                                                 * - CLOSED_RECOVER (Stream State): (3) Closed, the applications may attempt to re-open the stream later
                                                 *   (can occur via either an RefreshMsg or an StatusMsg), OR
                                                 *
                                                 * - CLOSED (Stream State): (4) Closed (indicates that the data is not available on this service/connection
                                                 *   and is not likely to become available), OR
                                                 *
                                                 * - SUSPECT (Data State): (2) Data is Suspect (similar to a stale data state, indicates that the health of
                                                 *	 some or all data associated with the stream is out of date or cannot be confirmed that it is current)
                                                 */

                                                /* Closes channel, cleans up and exits the application. */
                                                CloseChannelCleanUpAndExit(channel, error, TransportReturnCode.FAILURE);
                                            }
                                            else
                                            {
                                                isLoginSuccessful = true;

                                                Console.Write("ETA NI Provider application is granted access and has logged in successfully.\n\n");
                                                /* The Source Directory domain model conveys information about all available services in the system. After completing the
                                                 * Login process, an OMM NIP must provide a Source Directory refresh message.
                                                 */
                                                Console.Write("ETA NI Provider application is providing a Source Directory refresh message.\n\n");
                                                SendSourceDirectoryResponse(channel, maxMsgSize, encodeIter, serviceName, serviceId);
                                            }

                                        }
                                        break;
                                    default:
                                        {
                                            Console.WriteLine("Unhandled Domain Type: {0}", msg.DomainType);
                                        }
                                        break;
                                }

                                /* Process data and update ping monitor since data was received */
                                /* set flag for server message received */
                                receivedServerMsg = true;
                                Console.Write("Ping message has been received successfully from the server due to data message ...\n\n");
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
                    }

                    /* An I/O notification mechanism can be used to indicate when the operating system can
                     * accept more data for output.
                     *
                     * Flush function is called because of a write file descriptor alert
                     */
                    if (writeSocketList.Count > 0)
                    {

                        /* Flush */
                        retCode = TransportReturnCode.FAILURE;

                        /* this section of code was called because of a write file descriptor alert */
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
                                default:
                                    {
                                        Console.WriteLine("Error ({0}) (errno: {1}) encountered with Init Channel fd={2}. Error Text: {3}",
                                                          error.ErrorId, error.SysError, channelFDValue, error.Text);
                                        CloseChannelCleanUpAndExit(channel, error, TransportReturnCode.FAILURE);
                                    }
                                    break;
                            }
                        }
                    }

                    /* Processing ping management handler */
                    if ((retCode = ProcessPingManagementHandler(channel, error)) > TransportReturnCode.SUCCESS)
                    {
                        /* There is still data left to flush, leave our write notification enabled so we get called again.
                         * If everything wasn't flushed, it usually indicates that the TCP output buffer cannot accept more yet
                         */

                        /* set write fd if there's still other data queued */
                        /* flush is done by application */
                        opWrite = true;
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
                        /* Closes all streams for the NI Provider after run-time has elapsed. */

                        /* send close status messages to all item streams */
                        SendItemCloseStatusMsg(channel, maxMsgSize, encodeIter);

                        /* Note that closing Login stream will automatically close all other streams at the provider */
                        if ((retCode = CloseLoginStream(channel, maxMsgSize, encodeIter)) < TransportReturnCode.SUCCESS) /* (retval > TransportReturnCode.SUCCESS) or (retval < _RET_SUCCESS) */
                        {
                            /* When you close login, we want to make a best effort to get this across the network as it will gracefully
                             * close all open streams. If this cannot be flushed or failed, this application will just close the connection
                             * for simplicity.
                             */

                            /* Closes channel, cleans up and exits the application. */
                            CloseChannelCleanUpAndExit(channel, error, TransportReturnCode.FAILURE);
                        }

                        /* flush before exiting */
                        if (writeSocketList.Count > 0)
                        {
                            retCode = (TransportReturnCode)1;
                            while (retCode > TransportReturnCode.SUCCESS)
                            {
                                retCode = channel.Flush(out error);
                            }
                            if (retCode < TransportReturnCode.SUCCESS)
                            {
                                Console.WriteLine("Flush() failed with return code {0} - <{1}>", retCode, error.Text);
                            }
                        }

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

        #region Sending message


        /// <summary>Sends a message buffer to a channel.</summary>
        ///
        /// <param name="etaChannel">The channel to send the message buffer to</param>
        /// <param name="msgBuf">The msgBuf to be sent</param>
        public static TransportReturnCode SendMessage(IChannel channel, ITransportBuffer msgBuf)
        {
            Error error;
            TransportReturnCode retCode;

            WriteArgs writeArgs = new()
            {
                Flags = WriteFlags.NO_FLAGS
            };

            /* send the request */

            /*********************************************************
             * Client/NIProv Application Life Cycle Major Step 4: Write using Writer
             * Writer performs any writing or queuing of data. This function expects
             * the Channel to be in the active state and the buffer to be properly
             * populated, where length reflects the actual number of bytes used.
             * This function allows for several modifications to be specified for
             * this call. Here we use WriteFlags.NO_FLAGS. For more information on
             * other flag enumeration such as WriteFlags.DO_NOT_COMPRESS or
             * WriteFlags.DIRECT_SOCKET_WRITE, see the ETA C developers guide for
             * Write Flag Enumeration Values supported by ETA Transport.
             *
             * The ETA Transport also supports writing data at different priority
             * levels. The application can pass in two integer values used for
             * reporting information about the number of bytes that will be written.
             * The uncompressedBytesWritten parameter will return the number of
             * bytes to be written, including any transport header overhead but not
             * taking into account any compression. The bytesWritten parameter will
             * return the number of bytes to be written, including any transport
             * header overhead and taking into account any compression. If
             * compression is disabled, uncompressedBytesWritten and bytesWritten
             * should match. The number of bytes saved through the compression
             * process can be calculated by (bytesWritten -
             * uncompressedBytesWritten). Note: Before passing a buffer to Write, it
             * is required that the application set length to the number of bytes
             * actually used. This ensures that only the required bytes are written
             * to the network.
             *********************************************************/

            /* Now write the data - keep track of ETA Transport return code - Because positive values indicate
             * bytes left to write, some negative transport layer return codes still indicate success
             */

            /* this example writes buffer as high priority and no write modification flags */
            if ((retCode = channel.Write(msgBuf, writeArgs, out error)) == TransportReturnCode.WRITE_CALL_AGAIN)
            {
                /* (-10) Transport Success: Write is fragmenting the buffer and needs to be called again with
                 *       the same buffer. This indicates that Write was unable to send all fragments with the
                 *       current call and must continue fragmenting
                 */

                /* Large buffer is being split by transport, but out of output buffers. Schedule a call to
                 * Flush and then call the Write function again with this same exact buffer to continue the
                 * fragmentation process.
                 *
                 * Only release the buffer if not passing it to Write again. */

                /* call flush and write again - breaking out if the return code is something other than
                 * TransportReturnCode.WRITE_CALL_AGAIN (write call again) */
                while (retCode == TransportReturnCode.WRITE_CALL_AGAIN)
                {
                    /* Schedule a call to Flush */
                    if ((retCode = channel.Flush(out error)) < TransportReturnCode.SUCCESS)
                    {
                        Console.WriteLine("Flush has failed with return code {0} - <{1}>", retCode, error.Text);
                    }
                    /* call the Write function again with this same exact buffer to continue the fragmentation process. */
                    retCode = channel.Write(msgBuf, writeArgs, out error);
                }
            }

            /* set write fd if there's still data queued */
            if (retCode > TransportReturnCode.SUCCESS)
            {
                /* The write was successful and there is more data queued in ETA Transport. The Flush function
                 * should be used to continue attempting to flush data to the connection. ETA will release
                 * buffer.
                 */

                /* flush needs to be done by application */
            }
            else
            {
                /* Handle return codes appropriately, not all return values are failure conditions */
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

                                /* Set write fd if flush failed */
                                /* Flush needs to be done by application */

                                /* Channel is still open, but Channel.Write() tried to flush internally and failed.
                                 * Return positive value so the caller knows there's bytes to flush.
                                 */
                                return TransportReturnCode.SUCCESS + 1;
                            }
                            break;
                        }
                    case TransportReturnCode.FAILURE:
                    default:
                        {
                            Console.WriteLine("Error ({0}) (errno: {1}) encountered with Write : {2}", error.ErrorId, error.SysError, error.Text);
                            channel.ReleaseBuffer(msgBuf, out error);
                            return TransportReturnCode.FAILURE;
                        }
                }
            }

            return retCode;
        }

        #endregion

        #region Login management

        const int LOGIN_STREAM_ID = 1;

        /// <summary>Send Login request message to a channel.</summary>
        ///
        /// <remarks>
        /// This consists of getting a message buffer, setting the login request information,
        /// encoding the login request, and sending the login request to the server. A Login request
        /// message is encoded and sent by OMM consumer and OMM non-interactive provider
        /// applications. This message registers a user with the system. After receiving a
        /// successful Login response, applications can then begin consuming or providing additional
        /// content. An OMM provider can use the Login request information to authenticate users
        /// with DACS.
        /// </remarks>
        ///
        /// <param name="etaChannel">The channel to send the Login request message buffer to</param>
        /// <param name="maxMsgSize">the requested size of the buffer for GetBuffer function to obtain from the guaranteed/shared buffer pool.</param>
        /// <param name="encodeIter">The encode iterator</param>
        public static TransportReturnCode SendLoginRequest(IChannel channel, int maxFragmentSize, EncodeIterator encIter)
        {
            TransportReturnCode retCode;
            Error error;
            ITransportBuffer? msgBuf;

            /* Populate and encode a requestMsg */
            IRequestMsg reqMsg = (IRequestMsg)new Msg();

            ElementList elementList = new();
            ElementEntry elementEntry = new();

            Buffer applicationId = new();
            Buffer applicationName = new();
            UInt applicationRole = new();

            String userName = "put userName here";
            Buffer userNameBuf = new();

            elementList.Clear();
            elementEntry.Clear();

            /* etaGetBuffer() is the utility function that does 2-pass (more robust) getting non-packable buffer. */
            if ((msgBuf = EtaGetBuffer(channel, maxFragmentSize, out error)) == null)
            {
                return TransportReturnCode.FAILURE;
            }

            /* if a buffer is returned, we can populate and write, encode an Msg into the buffer */

            /* Encodes the Login request. */

            reqMsg.Clear();
            /* clear encode iterator for reuse - this should be used to achieve the best performance while clearing the iterator. */
            encIter.Clear();

            /* set version information of the connection on the encode iterator so proper versioning can be performed */

            CodecReturnCode codeCode;
            /* set the buffer on an EncodeIterator */
            if ((codeCode = encIter.SetBufferAndRWFVersion(msgBuf, channel.MajorVersion, channel.MinorVersion)) < CodecReturnCode.SUCCESS)
            {
                channel.ReleaseBuffer(msgBuf, out error);
                Console.WriteLine("\nSetEncodeIteratorBuffer() failed for Login Request with return code: {0}", codeCode);
                /* Closes channel, cleans up and exits the application. */
                return TransportReturnCode.FAILURE;
            }

            /* set-up message */
            reqMsg.MsgClass = MsgClasses.REQUEST;/* (1) Request Message */
            /* StreamId */
            reqMsg.StreamId = LOGIN_STREAM_ID;
            reqMsg.DomainType = (int)DomainType.LOGIN; /* (1) Login Message */
            reqMsg.ContainerType = DataTypes.NO_DATA;/* (128) No Data */
            /* The initial Login request must be streaming (i.e., a RequestMsgFlags.STREAMING flag is required). */
            reqMsg.Flags = RequestMsgFlags.STREAMING;

            /* set msgKey members */
            reqMsg.MsgKey.Flags = MsgKeyFlags.HAS_ATTRIB | MsgKeyFlags.HAS_NAME_TYPE | MsgKeyFlags.HAS_NAME;

            /* Username */
            userNameBuf.Data(userName);

            reqMsg.MsgKey.Name.Data(Environment.GetEnvironmentVariable("user.name") ?? "Unknown");

            reqMsg.MsgKey.NameType = (int)Login.UserIdTypes.NAME;/* (1) Name */
            /* (133) Element List container type, used to represent content containing element name,
             *       dataType, and value triples. */
            reqMsg.MsgKey.AttribContainerType = DataTypes.ELEMENT_LIST;

            /* encode message */

            /* since our msgKey has opaque that we want to encode, we need to use EncodeMsgInit */
            /* Msg.EncodeInit should return and inform us to encode our key opaque */
            if ((codeCode = reqMsg.EncodeInit(encIter, 0)) < CodecReturnCode.SUCCESS)
            {
                channel.ReleaseBuffer(msgBuf, out error);
                Console.WriteLine("reqMsg.encodeInit() failed for Login Request with return code: {0}", codeCode);
                /* Closes channel, cleans up and exits the application. */
                return TransportReturnCode.FAILURE;
            }

            /* encode our msgKey opaque */

            /* encode the element list */
            elementList.Clear();

            /* (0x08) The ElementList contains standard encoded content (e.g. not set defined).  */
            elementList.Flags = ElementListFlags.HAS_STANDARD_DATA;

            /* Begins encoding of an ElementList. */
            if ((codeCode = elementList.EncodeInit(encIter, null, 0)) < CodecReturnCode.SUCCESS)
            {
                channel.ReleaseBuffer(msgBuf, out error);
                Console.WriteLine("elementList.encodeInit() failed for Login Request with return code: {0}", codeCode);
                /* Closes channel, cleans up and exits the application. */
                return TransportReturnCode.FAILURE;
            }

            /* ApplicationId */
            applicationId.Data("256");
            elementEntry.DataType = DataTypes.ASCII_STRING;
            elementEntry.Name = ElementNames.APPID;

            if ((codeCode = elementEntry.Encode(encIter, applicationId)) < CodecReturnCode.SUCCESS)
            {
                channel.ReleaseBuffer(msgBuf, out error);
                Console.WriteLine("elementEntry.Encode() failed for Login Request with return code: {0}", codeCode);
                /* Closes channel, cleans up and exits the application. */
                return TransportReturnCode.FAILURE;
            }

            /* ApplicationName */
            applicationName.Data("ETA NI provider Training");

            elementEntry.DataType = DataTypes.ASCII_STRING;
            elementEntry.Name = ElementNames.APPNAME;
            if ((codeCode = elementEntry.Encode(encIter, applicationName)) < CodecReturnCode.SUCCESS)
            {
                channel.ReleaseBuffer(msgBuf, out error);
                Console.WriteLine("elementEntry.Encode() failed for Login Request with return code: {0}", codeCode);
                /* Closes channel, cleans up and exits the application. */
                return TransportReturnCode.FAILURE;
            }

            /* Role */
            elementEntry.DataType = DataTypes.UINT;
            elementEntry.Name = ElementNames.ROLE;

            /* (1) Application logs in as a provider */
            applicationRole.Value(Login.RoleTypes.PROV);

            if ((codeCode = elementEntry.Encode(encIter, applicationRole)) < CodecReturnCode.SUCCESS)
            {
                channel.ReleaseBuffer(msgBuf, out error);
                Console.WriteLine("elementEntry.Encode() failed for Login Request with return code: {0}", codeCode);
                /* Closes channel, cleans up and exits the application. */
                return TransportReturnCode.FAILURE;
            }

            /* complete encode element list */
            if ((codeCode = elementList.EncodeComplete(encIter, true)) < CodecReturnCode.SUCCESS)
            {
                channel.ReleaseBuffer(msgBuf, out error);
                Console.WriteLine("elementList.EncodeComplete() failed for Login Request with return code: {0}", codeCode);
                /* Closes channel, cleans up and exits the application. */
                return TransportReturnCode.FAILURE;
            }

            /* complete encode key */
            /* Msg.EncodeKeyAttribComplete finishes our key opaque, so it should return and indicate
             * for us to encode our container/msg payload
             */
            if ((codeCode = reqMsg.EncodeKeyAttribComplete(encIter, true)) < CodecReturnCode.SUCCESS)
            {
                channel.ReleaseBuffer(msgBuf, out error);
                Console.WriteLine("reqMsg.encodeKeyAttribComplete() failed for Login Request with return code: {0}", codeCode);
                /* Closes channel, cleans up and exits the application. */
                return TransportReturnCode.FAILURE;
            }
            /* complete encode message */
            if ((codeCode = reqMsg.EncodeComplete(encIter, true)) < CodecReturnCode.SUCCESS)
            {
                channel.ReleaseBuffer(msgBuf, out error);
                Console.WriteLine("reqMsg.EncodeComplete() failed for Login Request with return code: {0}", codeCode);
                /* Closes channel, cleans up and exits the application. */
                return TransportReturnCode.FAILURE;
            }

            /* send login request */
            if ((retCode = SendMessage(channel, msgBuf)) < TransportReturnCode.SUCCESS)
            {
                return TransportReturnCode.FAILURE;
            }
            else if (retCode > TransportReturnCode.SUCCESS)
            {
                /* There is still data left to flush, leave our write notification enabled so we get called again.
                 * If everything wasn't flushed, it usually indicates that the TCP output buffer cannot accept more yet
                 */

                /* set write flag if there's still other data queued */
                /* flush needs to be done by application */
            }

            return retCode;
        }


        /// <summary>Processes a login response. This consists of decoding the response.</summary>
        ///
        /// <param name="msg">The partially decoded message</param>
        /// <param name="decodeIter">The decode iterator</param>
        public static TransportReturnCode ProcessLoginResponse(Msg msg, DecodeIterator decIter)
        {
            CodecReturnCode codeCode;
            State pState;

            string tempData = "guess";
            Buffer tempBuf = new();

            tempBuf.Data(tempData);

            /* Switch cases depending on message class */
            switch (msg.MsgClass)
            {
                case MsgClasses.REFRESH:
                    {
                        IMsgKey key;

                        ElementList elementList = new();
                        ElementEntry elementEntry = new();

                        /* check stream id */
                        Console.WriteLine("\nReceived Login Refresh Msg with Stream Id {0}", msg.StreamId);

                        /* check if it's a solicited refresh */
                        if ((msg.Flags & RefreshMsgFlags.SOLICITED) != 0)
                        {
                            Console.WriteLine("\nThe refresh msg is a solicited refresh (sent as a response to a request).");
                        }
                        else
                        {
                            Console.WriteLine("A refresh sent to inform a NI provider of an upstream change in information (i.e., an unsolicited refresh).");
                        }

                        /* get key */
                        key = msg.MsgKey;

                        /* decode key opaque data */
                        if ((codeCode = msg.DecodeKeyAttrib(decIter, key)) != CodecReturnCode.SUCCESS)
                        {
                            Console.WriteLine("DecodeMsgKeyAttrib() failed with return code: {0}", codeCode);
                            return TransportReturnCode.FAILURE;
                        }

                        /* decode element list */
                        if ((codeCode = elementList.Decode(decIter, null)) == CodecReturnCode.SUCCESS)
                        {
                            /* decode each element entry in list */
                            while ((codeCode = elementEntry.Decode(decIter)) != CodecReturnCode.END_OF_CONTAINER)
                            {
                                if (codeCode == CodecReturnCode.SUCCESS)
                                {
                                    /* get login response information */

                                    /* Currently, ADH/Infra handling of login response was simpler than ADS
                                     * handling - The Only Received Login Response from ADH is ApplicationId
                                     * in the current implementation of ADH.  In some cases, a lot of things
                                     * don't apply (like SingleOpen, Support*, etc) as these are consumer
                                     * based behaviors so ADH does not advertise them.
                                     *
                                     * Also, likely many defaults are being relied on from ADH, while ADS may
                                     * be sending them even though default.
                                     */

                                    /* ApplicationId */
                                    if (elementEntry.Name.Equals(ElementNames.APPID))
                                    {
                                        Console.WriteLine("\tReceived Login Response for ApplicationId: {0}",
                                            elementEntry.EncodedData.ToString());
                                    }
                                    /* ApplicationName */
                                    else if (elementEntry.Name.Equals(ElementNames.APPNAME))
                                    {
                                        Console.WriteLine("\tReceived Login Response for ApplicationName: {0}",
                                            elementEntry.EncodedData.ToString());
                                    }
                                    /* Position */
                                    else if (elementEntry.Name.Equals(ElementNames.POSITION))
                                    {
                                        Console.WriteLine("\tReceived Login Response for Position: {0}",
                                            elementEntry.EncodedData.ToString());
                                    }
                                }
                                else
                                {
                                    Console.WriteLine("DecodeElementEntry() failed with return code: {0}", codeCode);
                                    return TransportReturnCode.FAILURE;
                                }
                            }
                        }
                        else
                        {
                            Console.WriteLine("DecodeElementList() failed with return code: {0}", codeCode);
                            return TransportReturnCode.FAILURE;
                        }

                        /* get Username */
                        if (key != null)
                        {
                            Console.WriteLine("\nReceived Login Response for ApplicationId: {0}", key.Name.ToString());
                        }
                        else
                        {
                            Console.WriteLine("\nReceived Login Response for ApplicationId: Unknown");
                        }

                        /* get state information */
                        pState = ((IRefreshMsg)msg).State;
                        Console.Write("{0}\n\n", pState.ToString());

                        /* check if login okay and is solicited */
                        if (((msg.Flags & RefreshMsgFlags.SOLICITED) != 0)
                            && pState.StreamState() == StreamStates.OPEN
                            && pState.DataState() == DataStates.OK)
                        {
                            Console.WriteLine("Login stream is OK and solicited");
                        }
                        else /* handle error cases */
                        {
                            if (pState.StreamState() == StreamStates.CLOSED_RECOVER)
                            {
                                /* Stream State is (3) Closed, the applications may attempt to re-open the
                                 * stream later (can occur via either an RefreshMsg or an StatusMsg) */

                                Console.WriteLine("Login stream is closed recover");
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
                case MsgClasses.STATUS:/* (3) Status Message */
                    {
                        Console.Write("Received Login StatusMsg\n");
                        /* (0x020) Indicates that this StatusMsg has stream or group state information, contained in StatusMsg.State. */
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
                            else if (pState.StreamState() == StreamStates.OPEN
                                && pState.DataState() == DataStates.SUSPECT)
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
                case MsgClasses.UPDATE: /* (4) Update Message */
                    {
                        Console.Write("Received Login Update\n");
                    }
                    break;
                case MsgClasses.CLOSE: /* (5) Close Message */
                    {
                        Console.Write("Received Login Close\n");
                        return TransportReturnCode.FAILURE;
                    }
                default: /* Error handling */
                    {
                        Console.WriteLine("Received Unhandled Login Msg Class: {0}", msg.MsgClass);
                        return TransportReturnCode.FAILURE;
                    }
            }

            return TransportReturnCode.SUCCESS;
        }


        /// <summary>
        /// Close the Login stream. Note that closing Login stream will automatically close all
        /// other streams at the provider.
        /// </summary>
        ///
        /// <remarks>
        /// A Login close message is encoded and sent by OMM NIP applications. This message allows a
        /// NIP to log out of the system. Closing a Login stream is equivalent to a 'Close All' type
        /// of message, where all open streams are closed (thus all other streams associated with
        /// the user are closed).
        /// </remarks>
        ///
        /// <param name="etaChannel">The channel to send the Login close message buffer to</param>
        /// <param name="maxMsgSize">the requested size of the buffer for GetBuffer function to obtain from the guaranteed/shared buffer pool.</param>
        /// <param name="encodeIter">The encode iterator</param>
        public static TransportReturnCode CloseLoginStream(IChannel channel, int maxMsgSize, EncodeIterator encIter)
        {
            TransportReturnCode retCode;
            CodecReturnCode codeCode;
            Error error = new();
            ITransportBuffer? msgBuf;
            ICloseMsg closeMsg = (ICloseMsg)new Msg();

            /* Get a buffer of the channel max fragment size */
            if ((msgBuf = EtaGetBuffer(channel, maxMsgSize, out error)) == null)
            {
                /* Connection should be closed, return failure */
                /* Closes channel, cleans up and exits the application. */

                return TransportReturnCode.FAILURE;
            }

            /* clear encode iterator for reuse - this should be used to achieve the best performance while clearing the iterator. */
            encIter.Clear();
            /* set version information of the connection on the encode iterator so proper versioning can be performed */
            /* set the buffer on an EncodeIterator */

            if ((codeCode = encIter.SetBufferAndRWFVersion(msgBuf, channel.MajorVersion, channel.MinorVersion)) < CodecReturnCode.SUCCESS)
            {
                channel.ReleaseBuffer(msgBuf, out error);
                Console.WriteLine("\nSetEncodeIteratorBuffer() failed with return code: {0}", codeCode);
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
            /* Functions without a suffix of Init or Complete (e.g. EncodeMsg) perform encoding within a single call,
             * typically used for encoding simple types like Integer or incorporating previously encoded data
             * (referred to as pre-encoded data).
             */

            if ((codeCode = closeMsg.Encode(encIter)) < CodecReturnCode.SUCCESS)
            {
                channel.ReleaseBuffer(msgBuf, out error);
                Console.WriteLine("Msg.Encode() failed with return code: {0}", codeCode);
                return TransportReturnCode.FAILURE;
            }

            /* send login close */

            if ((retCode = SendMessage(channel, msgBuf)) < TransportReturnCode.SUCCESS)
            {
                /* login close fails */
                /* Closes channel, cleans up and exits the application. */

                return TransportReturnCode.FAILURE;
            }
            else if (retCode > TransportReturnCode.SUCCESS)
            {
                /* There is still data left to flush, leave our write notification enabled so we get called again.
                 * If everything wasn't flushed, it usually indicates that the TCP output buffer cannot accept more yet
                 */

                /* set write flag if there's still other data queued */
                /* flush needs to be done by application */
            }

            return retCode;
        }


        /// <summary>
        /// Utility function that does 2-pass (more robust) getting non-packable buffer.
        /// Also, it simplifies the example codes and make the codes more readable.
        /// </summary>
        public static ITransportBuffer? EtaGetBuffer(IChannel channel, int size, out Error error)
        {
            TransportReturnCode retCode;

            ITransportBuffer msgBuf;

            /* First check error */
            if ((msgBuf = channel.GetBuffer(size, false, out error)) == null)
            {
                if (error.ErrorId != TransportReturnCode.NO_BUFFERS)
                {
                    /* Check to see if this is just out of buffers or if it's unrecoverable */

                    /* it's unrecoverable Error */
                    Console.WriteLine("Error ({0}) (errno: {1}) encountered with Channel.GetBuffer. Error Text: {2}",
                        error.ErrorId, error.SysError, error.Text);
                    /* Connection should be closed, return failure */
                    /* Closes channel, cleans up and exits the application. */

                    return null;
                }

                /* (-4) Transport Failure: There are no buffers available from the buffer pool, returned from GetBuffer.
                 * Use Ioctl to increase pool size or use Flush to flush data and return buffers to pool.
                 */

                /* The Flush function could be used to attempt to free buffers back to the pool */
                /* Flush and obtain buffer again */
                retCode = channel.Flush(out error);
                if (retCode < TransportReturnCode.SUCCESS)
                {
                    Console.WriteLine("Channel.Flush() failed with return code {0} - <{1}>", retCode, error.Text);
                    /* Closes channel, cleans up and exits the application. */

                    return null;
                }

                if ((msgBuf = channel.GetBuffer(size, false, out error)) == null)
                {
                    Console.Write("Error ({0}) (errno: {1}) encountered with Channel.getBuffer. Error Text: {2}",
                        error.ErrorId, error.SysError, error.Text);
                    /* Closes channel, cleans up and exits the application. */

                    return null;
                }
            }
            /* return  buffer to be filled in with valid memory */
            return msgBuf;
        }

        #endregion

        #region Send directory response


        private const int SRCDIR_STREAM_ID = -1;


        /// <summary>
        /// Send Source Directory response to a channel.
        /// </summary>
        ///
        /// <remarks>This consists of getting a message buffer, setting the source directory
        /// response information, encoding the source directory response, and sending the source directory response to
        /// the ADH server. OMM NIP application provides Source Directory information. The Source Directory domain model conveys
        /// information about all available services in the system. After completing the Login process, an OMM NIP must
        /// provide a Source Directory refresh.
        /// </remarks>
        ///
        /// <param name="channel">The channel to send the Source Directory response message buffer to</param>
        /// <param name="maxMsgSize">the requested size of the buffer for GetBuffer function to obtain from the guaranteed/shared buffer pool.</param>
        /// <param name="encodeIter">The encode iterator</param>
        /// <param name="serviceName">The service name specified by the OMM NIP application (Optional to set)</param>
        /// <param name="serviceId">the serviceId specified by the OMM NIP application (Optional to set)</param>
        public static TransportReturnCode SendSourceDirectoryResponse(IChannel channel, int maxMsgSize, EncodeIterator encodeIter, string serviceName, int serviceId)
        {
            CodecReturnCode codeCode;
            Error error;
            ITransportBuffer? msgBuf;
            UInt tmpUInt = new();
            /* Populate and encode a refreshMsg */
            IRefreshMsg refreshMsg = (IRefreshMsg)new Msg();

            /* The refresh flags of the source directory response */
            int refreshFlags = 0;

            /* The refresh key with filter flags that indicate the filter entries to include */
            MsgKey refreshKey = new();
            int streamId;

            Map map = new();
            MapEntry mapEntry = new();
            FilterList sourceDirectoryFilterList = new();
            string stateText;

            FilterEntry filterListItem = new();
            ElementEntry element = new();
            ElementList elementList = new();
            Codec.Array array = new();
            ArrayEntry arrayEntry = new();

            /* etaGetBuffer() is the utility function that does 2-pass (more robust) getting non-packable buffer. */
            if ((msgBuf = EtaGetBuffer(channel, maxMsgSize, out error)) == null) /* first check Error */
            {
                /* Connection should be closed, return failure */
                /* Closes channel, cleans up and exits the application. */
                return TransportReturnCode.FAILURE;
            }

            /* if a buffer is returned, we can populate and write, encode an Msg into the buffer */

            /* Encodes the Source Directory refresh. */

            refreshMsg.Clear();

            /* provide source directory response information */

            /* set refresh flags */
            /* The content of a Source Directory Refresh message is expected to be atomic and contained in a single part,
             * therefore RefreshMsgFlags.REFRESH_COMPLETE should be set.
             */

            /* (0x0008) The RefreshMsg has a message key, contained in RefreshMsg.MsgBase.MsgKey. */
            /* (0x0040) Indicates that this is the final part of a refresh. This flag should be set
             *          on both single-part response messages, as well as the final message in a
             *          multi-part response message sequence. */
            /* (0x0100) Indicates that any cached header or payload information associated with the
             *          RefreshMsg's item stream should be cleared. */
            refreshFlags |= RefreshMsgFlags.HAS_MSG_KEY;
            refreshFlags |= RefreshMsgFlags.REFRESH_COMPLETE;
            refreshFlags |= RefreshMsgFlags.CLEAR_CACHE;

            /* set filter flags */
            /* At a minimum, Refinitiv recommends that the NIP send the Info, State, and Group filters for the Source Directory. */
            refreshKey.Filter = Directory.ServiceFilterFlags.INFO
                | Directory.ServiceFilterFlags.STATE
                | Directory.ServiceFilterFlags.LOAD
                | Directory.ServiceFilterFlags.LINK;

            /* StreamId */
            streamId = SRCDIR_STREAM_ID;

            /* clear encode iterator for reuse - this should be used to achieve the best performance while clearing the iterator. */
            encodeIter.Clear();

            /* set version information of the connection on the encode iterator so proper versioning can be performed */
            /* set the buffer on an EncodeIterator */
            if ((codeCode = encodeIter.SetBufferAndRWFVersion(msgBuf, channel.MajorVersion, channel.MinorVersion)) < CodecReturnCode.SUCCESS)
            {
                channel.ReleaseBuffer(msgBuf, out error);
                Console.WriteLine("\nSetEncodeIteratorBuffer() failed with return code: {0}", codeCode);
                /* Closes channel, cleans up and exits the application. */
                return TransportReturnCode.FAILURE;
            }

            /* set-up message */
            refreshMsg.MsgClass = MsgClasses.REFRESH; /* (2) Refresh Message */
            refreshMsg.DomainType = (int)DomainType.SOURCE; /* (4) Source Message */
            /* (137) Map container type, used to represent primitive type key - container type paired entries.   */
            refreshMsg.ContainerType = DataTypes.MAP;

            refreshMsg.Flags = refreshFlags;
            refreshMsg.State.StreamState(StreamStates.OPEN);
            refreshMsg.State.DataState(DataStates.OK);
            refreshMsg.State.Code(StateCodes.NONE);
            stateText = "Source Directory Refresh Completed";
            refreshMsg.State.Text().Data(stateText);

            /* set members in msgKey */
            refreshMsg.MsgKey.Flags = MsgKeyFlags.HAS_FILTER;
            refreshMsg.MsgKey.Filter = refreshKey.Filter;

            /* StreamId */
            refreshMsg.StreamId = streamId;

            /* encode message - populate message and encode it */
            if ((codeCode = refreshMsg.EncodeInit(encodeIter, 0)) < CodecReturnCode.SUCCESS)
            {
                channel.ReleaseBuffer(msgBuf, out error);
                Console.WriteLine("EncodeMsgInit() failed with return code: {0}", codeCode);
                /* Closes channel, cleans up and exits the application. */
                return (TransportReturnCode)codeCode;
            }

            /* encode map */
            map.KeyPrimitiveType = DataTypes.UINT;
            map.ContainerType = DataTypes.FILTER_LIST;

            /**
             * Begins encoding of an MapEntry, where any payload is encoded after this call using
             * the appropriate container type encode functions, as specified by Map.ContainerType.
             *
             * Begins encoding of an MapEntry
             *
             * Typical use:
             *
             * 1. Call EncodeMapInit()
             *
             * 2. If Map contains set definitions that are not pre-encoded, call appropriate set
             *    definition encode functions, followed by EncodeMapSetDefsComplete()
             *
             * 3. If Map contains summary data that is not pre-encoded, call appropriate summary
             *    data container encoders, followed by EncodeMapSummaryDataComplete()
             *
             * 4. To encode entries, call EncodeMapEntry() or
             *    EncodeMapEntryInit()..EncodeMapEntryComplete() for each MapEntry
             *
             * 5. Call EncodeMapComplete() when all entries are completed
             */
            if ((codeCode = map.EncodeInit(encodeIter, 0, 0)) < CodecReturnCode.SUCCESS)
            {
                channel.ReleaseBuffer(msgBuf, out error);
                Console.WriteLine("EncodeMapInit() failed with return code: {0}", codeCode);
                /* Closes channel, cleans up and exits the application. */
                return (TransportReturnCode)codeCode;
            }

            /* encode map entry */
            mapEntry.Action = MapEntryActions.ADD;
            tmpUInt.Value(serviceId);
            if ((codeCode = mapEntry.EncodeInit(encodeIter, tmpUInt, 0)) < CodecReturnCode.SUCCESS)
            {
                channel.ReleaseBuffer(msgBuf, out error);
                Console.WriteLine("EncodeMapEntry() failed with return code: {0}", codeCode);
                /* Closes channel, cleans up and exits the application. */
                return (TransportReturnCode)codeCode;
            }

            /* encode filter list */
            sourceDirectoryFilterList.ContainerType = DataTypes.ELEMENT_LIST;

            /**
             * Begin encoding process for FilterList container type.
             *
             * Begins encoding of an FilterList
             *
             * Typical use:
             *
             * 1. Call EncodeFilterListInit()
             *
             * 2. To encode entries, call EncodeFilterEntry() or
             *    EncodeFilterEntryInit()..EncodeFilterEntryComplete() for each FilterEntry
             *
             * 3. Call EncodeFilterListComplete() when all entries are completed
             */
            if ((codeCode = sourceDirectoryFilterList.EncodeInit(encodeIter)) < CodecReturnCode.SUCCESS)
            {
                channel.ReleaseBuffer(msgBuf, out error);
                Console.WriteLine("EncodeFilterListInit() failed with return code: {0}", codeCode);
                /* Closes channel, cleans up and exits the application. */
                return (TransportReturnCode)codeCode;
            }

            /* encode filter list items -
             * for each filter list, element name use default values if not set
             */

            /* (0x00000001) Source Info Filter Mask */
            if ((refreshKey.Filter & Directory.ServiceFilterFlags.INFO) != 0)
            {
                /* Encodes the service's general information. */

                List<long> capabilitiesList = new();
                List<Qos> qosList = new();
                Buffer tempBuffer = new();

                filterListItem.Clear();
                element.Clear();
                elementList.Clear();
                array.Clear();

                /* encode filter list item */
                filterListItem.Id = Directory.ServiceFilterIds.INFO; /* (1) Service Info Filter ID */
                filterListItem.Action = FilterEntryActions.SET;
                if ((codeCode = filterListItem.EncodeInit(encodeIter, 0)) < CodecReturnCode.SUCCESS)
                {
                    channel.ReleaseBuffer(msgBuf, out error);
                    Console.WriteLine("EncodeFilterEntryInit() failed with return code: {0}", codeCode);
                    /* Closes channel, cleans up and exits the application. */
                    return (TransportReturnCode)codeCode;
                }

                /* encode the element list */
                /* (0x08) The ElementList contains standard encoded content (e.g. not set defined).  */
                elementList.Flags = ElementListFlags.HAS_STANDARD_DATA;
                if ((codeCode = elementList.EncodeInit(encodeIter, null, 0)) < CodecReturnCode.SUCCESS)
                {
                    channel.ReleaseBuffer(msgBuf, out error);
                    Console.WriteLine("EncodeElementListInit() failed with return code: {0}", codeCode);
                    /* Closes channel, cleans up and exits the application. */
                    return (TransportReturnCode)codeCode;
                }

                /* ServiceName */
                tempBuffer.Data(serviceName);
                element.DataType = DataTypes.ASCII_STRING;
                element.Name = ElementNames.NAME;
                if ((codeCode = element.Encode(encodeIter, tempBuffer)) < CodecReturnCode.SUCCESS)
                {
                    channel.ReleaseBuffer(msgBuf, out error);
                    Console.WriteLine("EncodeElementEntry() failed with return code: {0}", codeCode);
                    /* Closes channel, cleans up and exits the application. */
                    return (TransportReturnCode)codeCode;
                }

                /* Capabilities */
                element.DataType = DataTypes.ARRAY;
                element.Name = ElementNames.CAPABILITIES;
                if ((codeCode = element.EncodeInit(encodeIter, 0)) < CodecReturnCode.SUCCESS)
                {
                    channel.ReleaseBuffer(msgBuf, out error);
                    Console.WriteLine("EncodeElementEntryInit() failed with return code: {0}", codeCode);
                    /* Closes channel, cleans up and exits the application. */
                    return (TransportReturnCode)codeCode;
                }
                array.Clear();
                /*  Set Array.itemLength to 1, as each domainType uses only one byte. */
                array.ItemLength = 1;
                array.PrimitiveType = DataTypes.UINT;
                if ((codeCode = array.EncodeInit(encodeIter)) < CodecReturnCode.SUCCESS)
                {
                    channel.ReleaseBuffer(msgBuf, out error);
                    Console.WriteLine("EncodeArrayInit() failed with return code: {0}", codeCode);
                    /* Closes channel, cleans up and exits the application. */
                    return (TransportReturnCode)codeCode;
                }

                /* Lists the domains which this service can provide.*/
                capabilitiesList.Add((long)DomainType.DICTIONARY); /* (5) Dictionary Message */
                capabilitiesList.Add((long)DomainType.MARKET_PRICE); /* (6) Market Price Message */
                capabilitiesList.Add((long)DomainType.MARKET_BY_ORDER); /* (7) Market by Order/Order Book Message */
                capabilitiesList.Add((long)DomainType.SYMBOL_LIST); /* (10) Symbol List Messages */
                capabilitiesList.Add((long)DomainType.YIELD_CURVE); /* (22) Yield Curve */

                /* break out of decoding array items when predetermined max capabilities (10) reached */
                foreach (long capability in capabilitiesList)
                {
                    tmpUInt.Value(capability);
                    codeCode = arrayEntry.Encode(encodeIter, tmpUInt);
                    if ((codeCode) < CodecReturnCode.SUCCESS)
                    {
                        channel.ReleaseBuffer(msgBuf, out error);
                        Console.WriteLine("EncodeArrayEntry() failed with return code: {0}", codeCode);
                        /* Closes channel, cleans up and exits the application. */
                        return (TransportReturnCode)codeCode;
                    }
                }

                if ((codeCode = array.EncodeComplete(encodeIter, true)) < CodecReturnCode.SUCCESS)
                {
                    channel.ReleaseBuffer(msgBuf, out error);
                    Console.WriteLine("EncodeArrayComplete() failed with return code: {0}", codeCode);
                    /* Closes channel, cleans up and exits the application. */
                    return (TransportReturnCode)codeCode;
                }

                if ((codeCode = element.EncodeComplete(encodeIter, true)) < CodecReturnCode.SUCCESS)
                {
                    channel.ReleaseBuffer(msgBuf, out error);
                    Console.WriteLine("EncodeElementEntryComplete() failed with return code: {0}", codeCode);
                    /* Closes channel, cleans up and exits the application. */
                    return (TransportReturnCode)codeCode;
                }

                /* DictionariesProvided */
                element.DataType = DataTypes.ARRAY;
                element.Name = ElementNames.DICTIONARIES_PROVIDED;
                if ((codeCode = element.EncodeInit(encodeIter, 0)) < CodecReturnCode.SUCCESS)
                {
                    channel.ReleaseBuffer(msgBuf, out error);
                    Console.WriteLine("EncodeElementEntryInit() failed with return code: {0}", codeCode);
                    /* Closes channel, cleans up and exits the application. */
                    return (TransportReturnCode)codeCode;
                }
                array.Clear();

                /* If itemLength is set to 0, entries are variable length and each encoded entry can have a different length. */
                array.PrimitiveType = DataTypes.ASCII_STRING;
                array.ItemLength = 0;
                if ((codeCode = array.EncodeInit(encodeIter)) < CodecReturnCode.SUCCESS)
                {
                    channel.ReleaseBuffer(msgBuf, out error);
                    Console.WriteLine("EncodeArrayInit() failed with return code: {0}", codeCode);
                    /* Closes channel, cleans up and exits the application. */
                    return (TransportReturnCode)codeCode;
                }
                tempBuffer.Data("RWFFld");
                if ((codeCode = arrayEntry.Encode(encodeIter, tempBuffer)) < CodecReturnCode.SUCCESS)
                {
                    channel.ReleaseBuffer(msgBuf, out error);
                    Console.WriteLine("EncodeArrayEntry() failed with return code: {0}", codeCode);
                    /* Closes channel, cleans up and exits the application. */
                    return (TransportReturnCode)codeCode;
                }
                tempBuffer.Data("RWFEnum");
                if ((codeCode = arrayEntry.Encode(encodeIter, tempBuffer)) < CodecReturnCode.SUCCESS)
                {
                    channel.ReleaseBuffer(msgBuf, out error);
                    Console.WriteLine("EncodeArrayEntry() failed with return code: {0}", codeCode);
                    /* Closes channel, cleans up and exits the application. */
                    return (TransportReturnCode)codeCode;
                }
                if ((codeCode = array.EncodeComplete(encodeIter, true)) < CodecReturnCode.SUCCESS)
                {
                    channel.ReleaseBuffer(msgBuf, out error);
                    Console.WriteLine("EncodeArrayComplete() failed with return code: {0}", codeCode);
                    /* Closes channel, cleans up and exits the application. */
                    return (TransportReturnCode)codeCode;
                }
                if ((codeCode = element.EncodeComplete(encodeIter, true)) < CodecReturnCode.SUCCESS)
                {
                    channel.ReleaseBuffer(msgBuf, out error);
                    Console.WriteLine("EncodeElementEntryComplete() failed with return code: {0}", codeCode);
                    /* Closes channel, cleans up and exits the application. */
                    return (TransportReturnCode)codeCode;
                }

                /* DictionariesUsed */
                element.Name = ElementNames.DICTIONARIES_USED;
                element.DataType = DataTypes.ARRAY;
                if ((codeCode = element.EncodeInit(encodeIter, 0)) < CodecReturnCode.SUCCESS)
                {
                    channel.ReleaseBuffer(msgBuf, out error);
                    Console.WriteLine("EncodeElementEntryInit() failed with return code: {0}", codeCode);
                    /* Closes channel, cleans up and exits the application. */
                    return (TransportReturnCode)codeCode;
                }
                array.Clear();
                /* If itemLength is set to 0, entries are variable length and each encoded entry can have a different length. */
                array.PrimitiveType = DataTypes.ASCII_STRING;
                array.ItemLength = 0;

                if ((codeCode = array.EncodeInit(encodeIter)) < CodecReturnCode.SUCCESS)
                {
                    channel.ReleaseBuffer(msgBuf, out error);
                    Console.WriteLine("EncodeArrayInit() failed with return code: {0}", codeCode);
                    /* Closes channel, cleans up and exits the application. */
                    return (TransportReturnCode)codeCode;
                }
                tempBuffer.Data("RWFFld");
                if ((codeCode = arrayEntry.Encode(encodeIter, tempBuffer)) < CodecReturnCode.SUCCESS)
                {
                    channel.ReleaseBuffer(msgBuf, out error);
                    Console.WriteLine("EncodeArrayEntry() failed with return code: {0}", codeCode);
                    /* Closes channel, cleans up and exits the application. */
                    return (TransportReturnCode)codeCode;
                }
                tempBuffer.Data("RWFEnum");
                if ((codeCode = arrayEntry.Encode(encodeIter, tempBuffer)) < CodecReturnCode.SUCCESS)
                {
                    channel.ReleaseBuffer(msgBuf, out error);
                    Console.WriteLine("EncodeArrayEntry() failed with return code: {0}", codeCode);
                    /* Closes channel, cleans up and exits the application. */
                    return (TransportReturnCode)codeCode;
                }
                if ((codeCode = array.EncodeComplete(encodeIter, true)) < CodecReturnCode.SUCCESS)
                {
                    channel.ReleaseBuffer(msgBuf, out error);
                    Console.WriteLine("EncodeArrayComplete() failed with return code: {0}", codeCode);
                    /* Closes channel, cleans up and exits the application. */
                    return (TransportReturnCode)codeCode;
                }
                if ((codeCode = element.EncodeComplete(encodeIter, true)) < CodecReturnCode.SUCCESS)
                {
                    channel.ReleaseBuffer(msgBuf, out error);
                    Console.WriteLine("EncodeElementEntryComplete() failed with return code: {0}", codeCode);
                    /* Closes channel, cleans up and exits the application. */
                    return (TransportReturnCode)codeCode;
                }

                /* QoS */
                Qos qos = new();
                qos.IsDynamic = false;
                /* Rate is Tick By Tick, indicates every change to information is conveyed */
                qos.Rate(QosRates.TICK_BY_TICK);
                /* Timeliness is Realtime, indicates information is updated as soon as new information becomes available */
                qos.Timeliness(QosTimeliness.REALTIME);
                qosList.Add(qos);
                element.Name = ElementNames.QOS;
                element.DataType = DataTypes.ARRAY;

                if ((codeCode = element.EncodeInit(encodeIter, 0)) < CodecReturnCode.SUCCESS)
                {
                    channel.ReleaseBuffer(msgBuf, out error);
                    Console.WriteLine("EncodeElementEntryInit() failed with return code: {0}", codeCode);
                    /* Closes channel, cleans up and exits the application. */
                    return (TransportReturnCode)codeCode;
                }
                array.Clear();
                /* If itemLength is set to 0, entries are variable length and each encoded entry can have a different length. */
                array.ItemLength = 0;
                array.PrimitiveType = DataTypes.QOS;

                /**
                 *  Perform array item encoding (item can only be simple primitive type such as Int,
                 *  Real, or Date and not another Array or container type)
                 *
                 *  Encodes entries in an Array.
                 *
                 *  Typical use:
                 *
                 *  1. Call EncodeArrayInit()
                 *
                 *  2. Call EncodeArrayEntry() for each item in the array
                 *
                 *  3. Call EncodeArrayComplete()
                 *
                 * Only one of EncBuffer or Data should be supplied.
                 */
                if ((codeCode = array.EncodeInit(encodeIter)) < CodecReturnCode.SUCCESS)
                {
                    channel.ReleaseBuffer(msgBuf, out error);
                    Console.WriteLine("EncodeArrayInit() failed with return code: {0}", codeCode);
                    /* Closes channel, cleans up and exits the application. */
                    return (TransportReturnCode)codeCode;
                }

                if ((codeCode = arrayEntry.Encode(encodeIter, qosList[0])) < CodecReturnCode.SUCCESS)
                {
                    channel.ReleaseBuffer(msgBuf, out error);
                    Console.WriteLine("EncodeArrayEntry() failed with return code: {0}", codeCode);
                    /* Closes channel, cleans up and exits the application. */
                    return (TransportReturnCode)codeCode;
                }

                if ((codeCode = array.EncodeComplete(encodeIter, true)) < CodecReturnCode.SUCCESS)
                {
                    channel.ReleaseBuffer(msgBuf, out error);
                    Console.WriteLine("EncodeArrayComplete() failed with return code: {0}", codeCode);
                    /* Closes channel, cleans up and exits the application. */
                    return (TransportReturnCode)codeCode;
                }

                if ((codeCode = element.EncodeComplete(encodeIter, true)) < CodecReturnCode.SUCCESS)
                {
                    channel.ReleaseBuffer(msgBuf, out error);
                    Console.WriteLine("EncodeElementEntryComplete() failed with return code: {0}", codeCode);
                    /* Closes channel, cleans up and exits the application. */
                    return (TransportReturnCode)codeCode;
                }

                /* complete encode element list */
                if ((codeCode = elementList.EncodeComplete(encodeIter, true)) < CodecReturnCode.SUCCESS)
                {
                    channel.ReleaseBuffer(msgBuf, out error);
                    Console.WriteLine("EncodeElementListComplete() failed with return code: {0}", codeCode);
                    /* Closes channel, cleans up and exits the application. */
                    return (TransportReturnCode)codeCode;
                }

                /* complete encode filter list item */
                if ((codeCode = filterListItem.EncodeComplete(encodeIter, true)) < CodecReturnCode.SUCCESS)
                {
                    channel.ReleaseBuffer(msgBuf, out error);
                    Console.WriteLine("EncodeFilterEntryComplete() failed with return code: {0}", codeCode);
                    /* Closes channel, cleans up and exits the application. */
                    return (TransportReturnCode)codeCode;
                }
            }

            /* (0x00000002) Source State Filter Mask */
            if ((refreshKey.Filter & Directory.ServiceFilterFlags.STATE) != 0)
            {
                /* Encodes the service's state information. */

                int serviceState;
                int acceptingRequests;

                /* Specifies a status change to apply to all items provided by this service.
                 * It is equivalent to sending an StatusMsg to each item.
                 */
                State status = new();

                filterListItem.Clear();
                element.Clear();
                elementList.Clear();

                /* encode filter list item */
                filterListItem.Id = Directory.ServiceFilterIds.STATE; /* (2) Source State Filter ID */
                filterListItem.Action = FilterEntryActions.SET;
                if ((codeCode = filterListItem.EncodeInit(encodeIter, 0)) < CodecReturnCode.SUCCESS)
                {
                    channel.ReleaseBuffer(msgBuf, out error);
                    Console.WriteLine("EncodeFilterEntryInit() failed with return code: {0}", codeCode);
                    /* Closes channel, cleans up and exits the application. */
                    return (TransportReturnCode)codeCode;
                }

                /* encode the element list */
                /* (0x08) The ElementList contains standard encoded content (e.g. not set defined).  */
                elementList.Flags = ElementListFlags.HAS_STANDARD_DATA;
                if ((codeCode = elementList.EncodeInit(encodeIter, null, 0)) < CodecReturnCode.SUCCESS)
                {
                    channel.ReleaseBuffer(msgBuf, out error);
                    Console.WriteLine("EncodeElementListInit() failed with return code: {0}", codeCode);
                    /* Closes channel, cleans up and exits the application. */
                    return (TransportReturnCode)codeCode;
                }

                /* ServiceState */
                element.Name = ElementNames.SVC_STATE;
                element.DataType = DataTypes.UINT;
                /* Indicates whether the original provider of the data is available to respond to new requests. */
                serviceState = Directory.ServiceStates.UP; /* Service is Up */
                tmpUInt.Value(serviceState);
                if ((codeCode = element.Encode(encodeIter, tmpUInt)) < CodecReturnCode.SUCCESS)
                {
                    channel.ReleaseBuffer(msgBuf, out error);
                    Console.WriteLine("EncodeElementEntry() failed with return code: {0}", codeCode);
                    /* Closes channel, cleans up and exits the application. */
                    return (TransportReturnCode)codeCode;
                }

                /* AcceptingRequests */
                element.DataType = DataTypes.UINT;
                element.Name = ElementNames.ACCEPTING_REQS;
                /* Indicates whether the immediate provider can accept new requests and/or handle
                 * reissue requests on already open streams. */
                acceptingRequests = 1; /* 1: Yes */
                tmpUInt.Value(acceptingRequests);

                if ((codeCode = element.Encode(encodeIter, tmpUInt)) < CodecReturnCode.SUCCESS)
                {
                    channel.ReleaseBuffer(msgBuf, out error);
                    Console.WriteLine("EncodeElementEntry() failed with return code: {0}", codeCode);
                    /* Closes channel, cleans up and exits the application. */
                    return (TransportReturnCode)codeCode;
                }

                /* Status */
                element.DataType = DataTypes.STATE;
                element.Name = ElementNames.STATUS;

                /* The Status element can change the state of items provided by this service.
                 * Prior to changing a service status, Refinitiv recommends that you issue item or group
                 * status messages to update item states.
                 */
                status.StreamState(StreamStates.OPEN);
                status.DataState(DataStates.OK);
                status.Code(StateCodes.NONE);
                status.Text().Data("OK");
                if ((codeCode = element.Encode(encodeIter, status)) < CodecReturnCode.SUCCESS)
                {
                    channel.ReleaseBuffer(msgBuf, out error);
                    Console.WriteLine("EncodeElementEntry() failed with return code: {0}", codeCode);
                    /* Closes channel, cleans up and exits the application. */
                    return (TransportReturnCode)codeCode;
                }

                /* complete encode element list */
                if ((codeCode = elementList.EncodeComplete(encodeIter, true)) < CodecReturnCode.SUCCESS)
                {
                    channel.ReleaseBuffer(msgBuf, out error);
                    Console.WriteLine("EncodeElementListComplete() failed with return code: {0}", codeCode);
                    /* Closes channel, cleans up and exits the application. */
                    return (TransportReturnCode)codeCode;
                }

                /* complete encode filter list item */
                if ((codeCode = filterListItem.EncodeComplete(encodeIter, true)) < CodecReturnCode.SUCCESS)
                {
                    channel.ReleaseBuffer(msgBuf, out error);
                    Console.WriteLine("EncodeFilterEntryComplete() failed with return code: {0}", codeCode);
                    /* Closes channel, cleans up and exits the application. */
                    return (TransportReturnCode)codeCode;
                }
            }

            /* complete encode filter list */
            if ((codeCode = sourceDirectoryFilterList.EncodeComplete(encodeIter, true)) < CodecReturnCode.SUCCESS)
            {
                channel.ReleaseBuffer(msgBuf, out error);
                Console.WriteLine("EncodeFilterListComplete() failed with return code: {0}", codeCode);
                /* Closes channel, cleans up and exits the application. */
                return (TransportReturnCode)codeCode;
            }

            /* complete encode map entry */
            if ((codeCode = mapEntry.EncodeComplete(encodeIter, true)) < CodecReturnCode.SUCCESS)
            {
                channel.ReleaseBuffer(msgBuf, out error);
                Console.WriteLine("EncodeMapEntryComplete() failed with return code: {0}", codeCode);
                /* Closes channel, cleans up and exits the application. */
                return (TransportReturnCode)codeCode;
            }

            /* complete encode map */
            if ((codeCode = map.EncodeComplete(encodeIter, true)) < CodecReturnCode.SUCCESS)
            {
                channel.ReleaseBuffer(msgBuf, out error);
                Console.WriteLine("EncodeMapComplete() failed with return code: {0}", codeCode);
                /* Closes channel, cleans up and exits the application. */
                return (TransportReturnCode)codeCode;
            }

            /* complete encode message */
            if ((codeCode = refreshMsg.EncodeComplete(encodeIter, true)) < CodecReturnCode.SUCCESS)
            {
                channel.ReleaseBuffer(msgBuf, out error);
                Console.WriteLine("EncodeMsgComplete() failed with return code: {0}", codeCode);
                /* Closes channel, cleans up and exits the application. */
                return (TransportReturnCode)codeCode;
            }

            TransportReturnCode retVal;
            /* send source directory response */
            if ((retVal = SendMessage(channel, msgBuf)) < TransportReturnCode.SUCCESS)
            {
                /* Closes channel, cleans up and exits the application. */
                return TransportReturnCode.FAILURE;
            }
            else if (retVal > TransportReturnCode.SUCCESS)
            {
                /* There is still data left to flush, leave our write notification enabled so we get called again.
                 * If everything wasn't flushed, it usually indicates that the TCP output buffer cannot accept more yet
                 */

                /* set write fd if there's still other data queued */
                /* flush needs to be done by application */
            }

            return (TransportReturnCode)codeCode;
        }

        #endregion

        #region Handle Market Item Response

        static Real tempReal = new();
        static UInt tempUInt = new();
        static FieldList fieldList = new();
        static FieldEntry fieldEntry = new();


        /// <summary>Send just 1 Market Price item response message to a channel.</summary>
        ///
        /// <remarks>
        /// This consists of getting a message buffer, encoding the Market Price item response, and sending
        /// the item response to the server. Each unique information stream should begin with an RefreshMsg,
        /// conveying all necessary identification information for the content. Because the provider
        /// instantiates this information, a negative value streamId should be used for all streams. The
        /// initial identifying refresh can be followed by other status or update messages.
        /// </remarks>
        ///
        /// <param name="etaChannel">The channel to send the item response message buffer to</param>
        /// <param name="maxMsgSize">the requested size of the buffer for GetBuffer function to obtain from the guaranteed/shared buffer pool.</param>
        /// <param name="encodeIter">The encode iterator</param>
        /// <param name="marketPriceItemInfo">The market price item information</param>
        /// <param name="serviceId">The service id of the market price response</param>
        /// <param name="dataDictionary">The dictionary used for encoding</param>
        public static TransportReturnCode SendMarketPriceItemResponse(IChannel channel, int maxMsgSize, EncodeIterator encodeIter, int serviceId, DataDictionary dictionary)
        {
            CodecReturnCode codeCode;
            TransportReturnCode retVal;
            Error error;
            ITransportBuffer? msgBuf;

            /* Populate and encode a refreshMsg and a updateMsg */
            IRefreshMsg refreshMsg = (IRefreshMsg)new Msg();
            IUpdateMsg updateMsg = (IUpdateMsg)new Msg();

            Msg msgBase;
            Msg msg;

            string? stateText = null;
            string? errTxt = null;
            Buffer errorText = new();
            errorText.Data(errTxt);

            IDictionaryEntry dictionaryEntry;
            MarketPriceItem mpItem = new();

            /* In this simple example, we are sending just 1 Market Price item response message to a channel. */

            /* EtaGetBuffer() is the utility function that does 2-pass (more robust) getting non-packable buffer. */
            if ((msgBuf = EtaGetBuffer(channel, maxMsgSize, out error)) == null) /* first check Error */
            {
                /* Connection should be closed, return failure */
                /* Closes channel, cleans up and exits the application. */
                return TransportReturnCode.FAILURE;
            }

            /* if a buffer is returned, we can populate and write, encode an Msg into the buffer */

            /* Encodes the Market Price item response msg. */

            refreshMsg.Clear();
            updateMsg.Clear();

            /* clear encode iterator for reuse - this should be used to achieve the best performance while clearing the iterator. */
            encodeIter.Clear();

            /* set version information of the connection on the encode iterator so proper versioning can be performed */
            /* set the buffer on an EncodeIterator */
            codeCode = encodeIter.SetBufferAndRWFVersion(msgBuf, channel.MajorVersion, channel.MinorVersion);
            if ((codeCode) < CodecReturnCode.SUCCESS)
            {
                channel.ReleaseBuffer(msgBuf, out error);
                Console.WriteLine("\nSetEncodeIteratorBuffer() failed with return code: {0}", codeCode);
                /* Closes channel, cleans up and exits the application. */
                return TransportReturnCode.FAILURE;
            }

            mpItem = marketPriceItemInfo_itemData;

            /* set-up message */
            /* set message depending on whether refresh or update */

            /* After providing a Source Directory, the NIP application can begin pushing content to the
             * ADH. Each unique information stream should begin with an RefreshMsg, conveying all necessary
             * identification information for the content.
             *
             * Because the provider instantiates this information, a negative value streamId should be used
             * for all streams.
             *
             * The initial identifying refresh can be followed by other status or update messages.
             *
             * Some ADH functionality, such as cache rebuilding, may require that NIP applications publish the
             * message key on all RefreshMsgs. See the component specific documentation for more
             * information. */

            if (!marketPriceItemInfo_isRefreshComplete) /* this is a refresh message */
            {
                msgBase = (Msg)refreshMsg;
                msgBase.MsgClass = MsgClasses.REFRESH;

                /* streaming */
                /* (1) Stream is open (typically implies that information will be streaming, as information
                 * changes updated information will be sent on the stream, after final RefreshMsg or
                 * StatusMsg) */
                refreshMsg.State.StreamState(StreamStates.OPEN);
                refreshMsg.State.DataState(DataStates.OK);
                refreshMsg.State.Code(StateCodes.NONE);

                /* for non-interactive providers, this RefreshMsg is not a solicited response to a consumer's request. */

                /* (0x0008) The RefreshMsg has a message key, contained in RefreshMsg.MsgKey. */
                /* (0x0040) Indicates that this is the final part of a refresh. This flag should be set on
                 *          both single-part response messages, as well as the final message in a multi-part
                 *          response message sequence. */
                /* (0x0080) The RefreshMsg has quality of service information, contained in RefreshMsg.Qos. */
                /* (0x0100) Indicates that any cached header or payload information associated with the
                 *          RefreshMsg's item stream should be cleared. */
                refreshMsg.ApplyHasMsgKey();
                refreshMsg.ApplyRefreshComplete();
                refreshMsg.ApplyHasQos();
                refreshMsg.ApplyClearCache();

                stateText = "Item Refresh Completed";
                refreshMsg.State.Text().Data(stateText);
                msgBase.MsgKey.Flags = MsgKeyFlags.HAS_SERVICE_ID
                        | MsgKeyFlags.HAS_NAME
                        | MsgKeyFlags.HAS_NAME_TYPE;

                /* ServiceId */
                msgBase.MsgKey.ServiceId = serviceId;

                /* Item Name */
                msgBase.MsgKey.Name.Data(marketPriceItemInfo_itemName);
                /* (1) Reuters Instrument Code */
                msgBase.MsgKey.NameType = InstrumentNameTypes.RIC;

                /* Qos */
                refreshMsg.Qos.IsDynamic = false;
                /* Rate is Tick By Tick, indicates every change to information is conveyed */
                refreshMsg.Qos.Rate(QosRates.TICK_BY_TICK);
                /* Timeliness is Realtime, indicates information is updated as soon as new information becomes available */
                refreshMsg.Qos.Timeliness(QosTimeliness.REALTIME);
                msg = (Msg)refreshMsg;

                Console.Write("ETA NI Provider application is providing a MarketPrice Item refresh message.\n\n");
            }
            else /* this is an update message */
            {
                msgBase = (Msg)updateMsg;
                msgBase.MsgClass = MsgClasses.UPDATE; /* (4) Update Message */

                /* include msg key in updates for non-interactive provider streams */
                /* because the provider instantiates this information, a negative value streamId should be used for all streams. */
                updateMsg.Flags = UpdateMsgFlags.HAS_MSG_KEY;
                msgBase.MsgKey.Flags = MsgKeyFlags.HAS_SERVICE_ID
                        | MsgKeyFlags.HAS_NAME
                        | MsgKeyFlags.HAS_NAME_TYPE;

                /* ServiceId */
                msgBase.MsgKey.ServiceId = serviceId;
                /* Itemname */
                msgBase.MsgKey.Name.Data(marketPriceItemInfo_itemName);
                msgBase.MsgKey.NameType = InstrumentNameTypes.RIC;

                msg = (Msg)updateMsg;

                Console.Write("ETA NI Provider application is providing a MarketPrice Item update message.\n\n");
            }

            msgBase.DomainType = (int)DomainType.MARKET_PRICE; /* (6) Market Price Message */
            msgBase.ContainerType = DataTypes.FIELD_LIST; /* (132) Field List container type, used to represent content using fieldID - value pair data.  <BR>*/

            /* StreamId */
            msgBase.StreamId = MARKETPRICE_ITEM_STREAM_ID_START; /* negative value streamId should be used for all streams. */

            /* encode message - populate message and encode it */
            if ((codeCode = msg.EncodeInit(encodeIter, 0)) < CodecReturnCode.SUCCESS)
            {
                channel.ReleaseBuffer(msgBuf, out error);
                Console.WriteLine("EncodeMsgInit() failed with return code: {0}", codeCode);
                /* Closes channel, cleans up and exits the application. */
                return TransportReturnCode.FAILURE;
            }

            /* encode field list */
            /* (0x08) The FieldList contains standard encoded content (e.g. not set defined).  */
            fieldList.Flags = FieldListFlags.HAS_STANDARD_DATA;

            /**
             * Begin encoding process for FieldList container type.
             *
             * Begins encoding of an FieldList
             *
             * Typical use:
             *
             * 1. Call EncodeFieldListInit()
             *
             * 2. To encode entries, call EncodeFieldEntry() or
             *    EncodeFieldEntryInit()..EncodeFieldEntryComplete() for each FieldEntry
             *
             * 3. Call EncodeFieldListComplete() when all entries are completed
             */
            if ((codeCode = fieldList.EncodeInit(encodeIter, null, 0)) < CodecReturnCode.SUCCESS)
            {
                channel.ReleaseBuffer(msgBuf, out error);
                Console.WriteLine("EncodeFieldListInit() failed with return code: {0}", codeCode);
                /* Closes channel, cleans up and exits the application. */
                return TransportReturnCode.FAILURE;
            }

            /* encode fields */
            /* if refresh, encode refresh fields */
            if (!marketPriceItemInfo_isRefreshComplete)
            {
                /* RDNDISPLAY */
                fieldEntry.Clear();
                dictionaryEntry = dictionary.Entry(MarketPriceItem.RDNDISPLAY_FID);
                if (dictionaryEntry != null)
                {
                    fieldEntry.FieldId = MarketPriceItem.RDNDISPLAY_FID;
                    fieldEntry.DataType = dictionaryEntry.RwfType;
                    tempUInt.Value(mpItem.RDNDISPLAY);

                    if ((codeCode = fieldEntry.Encode(encodeIter, tempUInt)) < CodecReturnCode.SUCCESS)
                    {
                        channel.ReleaseBuffer(msgBuf, out error);
                        Console.WriteLine("EncodeFieldEntry() failed with return code: {0}", codeCode);
                        /* Closes channel, cleans up and exits the application. */
                        return TransportReturnCode.FAILURE;
                    }
                }
                /* RDN_EXCHID */
                fieldEntry.Clear();
                dictionaryEntry = dictionary.Entry(MarketPriceItem.RDN_EXCHID_FID);
                if (dictionaryEntry != null)
                {
                    fieldEntry.FieldId = MarketPriceItem.RDN_EXCHID_FID;
                    fieldEntry.DataType = dictionaryEntry.RwfType;
                    Codec.Enum enumValue = new();
                    enumValue.Value(mpItem.RDN_EXCHID);
                    if ((codeCode = fieldEntry.Encode(encodeIter, enumValue)) < CodecReturnCode.SUCCESS)
                    {
                        channel.ReleaseBuffer(msgBuf, out error);
                        Console.WriteLine("EncodeFieldEntry() failed with return code: {0}", codeCode);
                        /* Closes channel, cleans up and exits the application. */
                        return TransportReturnCode.FAILURE;
                    }
                }
                /* DIVPAYDATE */
                fieldEntry.Clear();
                dictionaryEntry = dictionary.Entry(MarketPriceItem.DIVPAYDATE_FID);
                if (dictionaryEntry != null)
                {
                    fieldEntry.FieldId = MarketPriceItem.DIVPAYDATE_FID;
                    fieldEntry.DataType = dictionaryEntry.RwfType;
                    if ((codeCode = fieldEntry.Encode(encodeIter, mpItem.DIVPAYDATE)) < CodecReturnCode.SUCCESS)
                    {
                        channel.ReleaseBuffer(msgBuf, out error);
                        Console.WriteLine("EncodeFieldEntry() failed with return code: {0}", codeCode);
                        /* Closes channel, cleans up and exits the application. */
                        return TransportReturnCode.FAILURE;
                    }
                }
            }
            /* TRDPRC_1 */
            fieldEntry.Clear();
            dictionaryEntry = dictionary.Entry(MarketPriceItem.TRDPRC_1_FID);
            if (dictionaryEntry != null)
            {
                fieldEntry.FieldId = MarketPriceItem.TRDPRC_1_FID;
                fieldEntry.DataType = dictionaryEntry.RwfType;
                tempReal.Clear();
                tempReal.Value(mpItem.TRDPRC_1, RealHints.EXPONENT_2);
                if ((codeCode = fieldEntry.Encode(encodeIter, tempReal)) < CodecReturnCode.SUCCESS)
                {
                    channel.ReleaseBuffer(msgBuf, out error);
                    Console.WriteLine("EncodeFieldEntry() failed with return code: {0}", codeCode);
                    /* Closes channel, cleans up and exits the application. */
                    return TransportReturnCode.FAILURE;
                }
            }
            /* BID */
            fieldEntry.Clear();
            dictionaryEntry = dictionary.Entry(MarketPriceItem.BID_FID);
            if (dictionaryEntry != null)
            {
                fieldEntry.FieldId = MarketPriceItem.BID_FID;
                fieldEntry.DataType = dictionaryEntry.RwfType;
                tempReal.Clear();
                tempReal.Value(mpItem.BID, RealHints.EXPONENT_2);
                if ((codeCode = fieldEntry.Encode(encodeIter, tempReal)) < CodecReturnCode.SUCCESS)
                {
                    channel.ReleaseBuffer(msgBuf, out error);
                    Console.WriteLine("EncodeFieldEntry() failed with return code: {0}", codeCode);
                    /* Closes channel, cleans up and exits the application. */
                    return TransportReturnCode.FAILURE;
                }
            }
            /* ASK */
            fieldEntry.Clear();
            dictionaryEntry = dictionary.Entry(MarketPriceItem.ASK_FID);
            if (dictionaryEntry != null)
            {
                fieldEntry.FieldId = MarketPriceItem.ASK_FID;
                fieldEntry.DataType = dictionaryEntry.RwfType;
                tempReal.Clear();
                tempReal.Value(mpItem.ASK, RealHints.EXPONENT_2);

                if ((codeCode = fieldEntry.Encode(encodeIter, tempReal)) < CodecReturnCode.SUCCESS)
                {
                    channel.ReleaseBuffer(msgBuf, out error);
                    Console.WriteLine("EncodeFieldEntry() failed with return code: {0}", codeCode);
                    /* Closes channel, cleans up and exits the application. */
                    return TransportReturnCode.FAILURE;
                }
            }
            /* ACVOL_1 */
            fieldEntry.Clear();
            dictionaryEntry = dictionary.Entry(MarketPriceItem.ACVOL_1_FID);
            if (dictionaryEntry != null)
            {
                fieldEntry.FieldId = MarketPriceItem.ACVOL_1_FID;
                fieldEntry.DataType = dictionaryEntry.RwfType;
                tempReal.Clear();
                tempReal.Value(mpItem.ACVOL_1, RealHints.EXPONENT_2);

                if ((codeCode = fieldEntry.Encode(encodeIter, tempReal)) < CodecReturnCode.SUCCESS)
                {
                    channel.ReleaseBuffer(msgBuf, out error);
                    Console.WriteLine("EncodeFieldEntry() failed with return code: {0}", codeCode);
                    /* Closes channel, cleans up and exits the application. */
                    return TransportReturnCode.FAILURE;
                }
            }
            /* NETCHNG_1 */
            fieldEntry.Clear();
            dictionaryEntry = dictionary.Entry(MarketPriceItem.NETCHNG_1_FID);
            if (dictionaryEntry != null)
            {
                fieldEntry.FieldId = MarketPriceItem.NETCHNG_1_FID;
                fieldEntry.DataType = dictionaryEntry.RwfType;
                tempReal.Clear();
                tempReal.Value(mpItem.NETCHNG_1, RealHints.EXPONENT_2);
                if ((codeCode = fieldEntry.Encode(encodeIter, tempReal)) < CodecReturnCode.SUCCESS)
                {
                    channel.ReleaseBuffer(msgBuf, out error);
                    Console.WriteLine("EncodeFieldEntry() failed with return code: {0}", codeCode);
                    /* Closes channel, cleans up and exits the application. */
                    return TransportReturnCode.FAILURE;
                }
            }
            /* ASK_TIME */
            fieldEntry.Clear();
            dictionaryEntry = dictionary.Entry(MarketPriceItem.ASK_TIME_FID);
            if (dictionaryEntry != null)
            {
                fieldEntry.FieldId = MarketPriceItem.ASK_TIME_FID;
                fieldEntry.DataType = dictionaryEntry.RwfType;
                if ((codeCode = fieldEntry.Encode(encodeIter, mpItem.ASK_TIME.Time())) < CodecReturnCode.SUCCESS)
                {
                    channel.ReleaseBuffer(msgBuf, out error);
                    Console.WriteLine("EncodeFieldEntry() failed with return code: {0}", codeCode);
                    /* Closes channel, cleans up and exits the application. */
                    return TransportReturnCode.FAILURE;
                }
            }

            /* complete encode field list */
            if ((codeCode = fieldList.EncodeComplete(encodeIter, true)) < CodecReturnCode.SUCCESS)
            {
                channel.ReleaseBuffer(msgBuf, out error);
                Console.WriteLine("EncodeFieldListComplete() failed with return code: {0}", codeCode);
                /* Closes channel, cleans up and exits the application. */
                return TransportReturnCode.FAILURE;
            }

            /* complete encode message */
            if ((codeCode = msg.EncodeComplete(encodeIter, true)) < CodecReturnCode.SUCCESS)
            {
                channel.ReleaseBuffer(msgBuf, out error);
                Console.WriteLine("EncodeMsgComplete() failed with return code: {0}", codeCode);
                /* Closes channel, cleans up and exits the application. */
                return TransportReturnCode.FAILURE;
            }

            /* set refresh complete flag if this is a refresh */
            if (marketPriceItemInfo_isRefreshComplete == false)
            {
                marketPriceItemInfo_isRefreshComplete = true;
            }

            /* send Market Price item request */
            if ((retVal = SendMessage(channel, msgBuf)) < TransportReturnCode.SUCCESS)
            {
                /* Closes channel, cleans up and exits the application. */
                return retVal;
            }
            else if (retVal > TransportReturnCode.SUCCESS)
            {
                /* There is still data left to flush, leave our write notification enabled so we get called again.
                 * If everything wasn't flushed, it usually indicates that the TCP output buffer cannot accept more yet
                 */

                /* set write fd if there's still other data queued */
                /* flush needs to be done by application */
                opWrite = true;
            }
            else
            {
                /* Message was sent, disable write notificaitons */
                opWrite = false;
            }

            return retVal;
        }


        /// <summary>Sends the item close status message for a channel.</summary>
        ///
        /// <param name="etaChannel">The channel to send close status message to</param>
        /// <param name="maxMsgSize">the requested size of the buffer for GetBuffer function to obtain from the guaranteed/shared buffer pool.</param>
        /// <param name="encodeIter">The encode iterator</param>
        /// <param name="marketPriceItemInfo">The market price item information</param>
        public static TransportReturnCode SendItemCloseStatusMsg(IChannel channel, int maxMsgSize, EncodeIterator encodeIter)
        {
            CodecReturnCode codeCode;
            TransportReturnCode retval;
            Error error;
            ITransportBuffer? msgBuf;

            /* Provider uses StatusMsg to send the item close status and to close the stream. */
            IStatusMsg msg = (IStatusMsg)new Msg();

            String stateText;

            /* EtaGetBuffer() is the utility function that does 2-pass (more robust) getting non-packable buffer. */
            if ((msgBuf = EtaGetBuffer(channel, maxMsgSize, out error)) == null) /* first check Error */
            {
                /* Connection should be closed, return failure */
                /* Closes channel, cleans up and exits the application. */
                return TransportReturnCode.FAILURE;
            }

            /* if a buffer is returned, we can populate and write, encode an Msg into the buffer */

            /* Encodes the item close status. */

            msg.Clear();

            /* clear encode iterator for reuse - this should be used to achieve the best performance while clearing the iterator. */
            encodeIter.Clear();
            /* set version information of the connection on the encode iterator so proper versioning can be performed */
            /* set the buffer on an EncodeIterator */
            codeCode = encodeIter.SetBufferAndRWFVersion(msgBuf, channel.MajorVersion, channel.MinorVersion);

            if (codeCode < CodecReturnCode.SUCCESS)
            {
                channel.ReleaseBuffer(msgBuf, out error);
                Console.WriteLine("\nSetEncodeIteratorBuffer() failed with return code: {0}", codeCode);
                /* Closes channel, cleans up and exits the application. */
                return TransportReturnCode.FAILURE;
            }

            /* set-up message */
            msg.MsgClass = MsgClasses.STATUS; /* (3) Status Message */
            msg.StreamId = MARKETPRICE_ITEM_STREAM_ID_START;
            msg.DomainType = (int)DomainType.MARKET_PRICE; /* (6) Market Price Message */
            /* No payload associated with this close status message */
            msg.ContainerType = DataTypes.NO_DATA; /* (128) No Data */

            /* (0x020) Indicates that this StatusMsg has stream or group state information, contained in StatusMsg.state.  */
            msg.Flags = StatusMsgFlags.HAS_STATE;
            /* (4) Closed (indicates that the data is not available on this service/connection and is not likely to become available) */
            msg.State.StreamState(StreamStates.CLOSED);
            /* (2) Data is Suspect (similar to a stale data state, indicates that the health of some or all data associated with the stream is out of date or cannot be confirmed that it is current ) */
            msg.State.DataState(DataStates.SUSPECT);
            msg.State.Code(StateCodes.NONE);
            stateText = $"Item stream closed for item: {marketPriceItemInfo_itemName}";
            msg.State.Text().Data(stateText);

            /* encode message */

            /* Since there is no payload, no need for Init/Complete as everything is in the msg header */
            /* Functions without a suffix of Init or Complete (e.g. EncodeMsg) perform encoding within a single call,
             * typically used for encoding simple types like Integer or incorporating previously encoded data
             * (referred to as pre-encoded data).
             */
            if ((codeCode = msg.Encode(encodeIter)) < CodecReturnCode.SUCCESS)
            {
                channel.ReleaseBuffer(msgBuf, out error);
                Console.WriteLine("EncodeMsg() failed with return code: {0}", codeCode);
                return TransportReturnCode.FAILURE;
            }

            /* send item close status */
            if ((retval = SendMessage(channel, msgBuf)) < TransportReturnCode.SUCCESS)
            {
                /* item close status fails */
                /* Closes channel, cleans up and exits the application. */
                return TransportReturnCode.FAILURE;
            }
            else if (retval > TransportReturnCode.SUCCESS)
            {
                /* There is still data left to flush */

                /* If the item close status doesn't flush, just close channel and exit the app. When you send item close status msg,
                 * we want to make a best effort to get this across the network as it will gracefully close the open item
                 * stream. If this cannot be flushed, this application will just close the connection for simplicity.
                 */

                /* Closes channel, cleans up and exits the application. */
            }

            return retval;
        }

        #endregion
    }
}
